/* ====================================================================================================================

	The copyright in this software is being made available under the License included below.
	This software may be subject to other third party and 	contributor rights, including patent rights, and no such
	rights are granted under this license.

	Copyright (c) 2010, SAMSUNG ELECTRONICS CO., LTD. and BRITISH BROADCASTING CORPORATION
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted only for
	the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
	promoting such standards. The following conditions are required to be met:

		* Redistributions of source code must retain the above copyright notice, this list of conditions and
		  the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
		  the following disclaimer in the documentation and/or other materials provided with the distribution.
		* Neither the name of SAMSUNG ELECTRONICS CO., LTD. nor the name of the BRITISH BROADCASTING CORPORATION
		  may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * ====================================================================================================================
*/

/** \file			TEncAdaptiveLoopFilter.cpp
    \brief		estimation part of adaptive loop filter class
*/

#include "TEncAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define ALF_NUM_OF_REDESIGN 3

// ====================================================================================================================
// Tables
// ====================================================================================================================

const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray9x9[81] =
{
   0,  1,  2,  3,  4,  5,  6,  7,  8,
   9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26,
  27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40, 39, 38, 37, 36,
  35, 34, 33, 32, 31, 30, 29, 28, 27,
  26, 25, 24, 23, 22, 21, 20, 19, 18,
  17, 16, 15, 14, 13, 12, 11, 10,  9,
   8,  7,  6,  5,  4,  3,  2,  1,  0
};
const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray7x7[49] =
{
   0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 10, 11, 12, 13,
  14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 23, 22, 21,
  20, 19, 18, 17, 16, 15, 14,
  13, 12, 11, 10,  9,  8,  7,
   6,  5,  4,  3,  2,  1,  0,
};
const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray5x5[25] =
{
   0,  1,  2,  3,  4,
   5,  6,  7,  8,  9,
  10, 11, 12, 11, 10,
   9,  8,  7,  6,  5,
   4,  3,  2,  1,  0,
};

// ====================================================================================================================
// Constructor / destructor
// ====================================================================================================================

TEncAdaptiveLoopFilter::TEncAdaptiveLoopFilter()
{
  m_ppdAlfCorr = NULL;
  m_pdDoubleAlfCoeff = NULL;
  m_puiCUCorr = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  m_pcBestAlfParam = NULL;
  m_pcTempAlfParam = NULL;
  m_pcPicYuvBest = NULL;
  m_pcPicYuvTmp = NULL;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param	pcPic						picture (TComPic) pointer
		\param	pcEntropyCoder	entropy coder class
 */
Void TEncAdaptiveLoopFilter::startALFEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder )
{
  m_pcPic = pcPic;
  m_pcEntropyCoder = pcEntropyCoder;

  m_eSliceType = pcPic->getSlice()->getSliceType();
  m_iPicNalReferenceIdc = (pcPic->getSlice()->isReferenced() ? 1 :0);

  m_uiNumSCUInCU = m_pcPic->getNumPartInCU();
  m_uiSCUWidth = (m_pcPic->getMinCUWidth()<<1);
  m_uiSCUHeight = (m_pcPic->getMinCUHeight()<<1);

  xInitParam();
  xCreateTmpAlfCtrlFlags();

  Int iWidth = pcPic->getPicYuvOrg()->getWidth();
  Int iHeight = pcPic->getPicYuvOrg()->getHeight();

  m_pcPicYuvTmp = new TComPicYuv();
  m_pcPicYuvTmp->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  m_pcPicYuvBest = pcPic->getPicYuvPred();

  m_pcBestAlfParam = new ALFParam;
  m_pcTempAlfParam = new ALFParam;
  allocALFParam(m_pcBestAlfParam);
  allocALFParam(m_pcTempAlfParam);
}

Void TEncAdaptiveLoopFilter::endALFEnc()
{
  xUninitParam();
  xDestroyTmpAlfCtrlFlags();

  m_pcPicYuvTmp->destroyLuma();
  m_pcPicYuvTmp = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;

  freeALFParam(m_pcBestAlfParam);
  freeALFParam(m_pcTempAlfParam);
}

/** \param	pcAlfParam					ALF parameter
		\param	dLambda							lambda value for RD cost computation
		\retval	ruiDist							distortion
		\retval	ruiBits							required bits
		\retval	ruiMaxAlfCtrlDepth	optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth )
{
  Int tap, num_coef;

  // set global variables
  tap         = ALF_MAX_NUM_TAP;
  num_coef    = (tap*tap+1)>>1;
  num_coef    = num_coef + 1; // DC offset

  // set lambda
	m_dLambdaLuma   = dLambda;
  m_dLambdaChroma = dLambda;

  TComPicYuv* pcPicOrg = m_pcPic->getPicYuvOrg();

  // extend image for filtering
  TComPicYuv* pcPicYuvRec    = m_pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;

  pcPicYuvRec->copyToPic(pcPicYuvExtRec);
	pcPicYuvExtRec->setBorderExtension( false );
  pcPicYuvExtRec->extendPicBorder   ();

  // set min cost
  UInt64 uiMinRate = MAX_INT;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;

  UInt64  uiOrigRate;
  UInt64  uiOrigDist;
  Double	dOrigCost;

  // calc original cost
  xCalcRDCost( pcPicOrg, pcPicYuvRec, NULL, uiOrigRate, uiOrigDist, dOrigCost );
  m_pcBestAlfParam->alf_flag = 0;
  m_pcBestAlfParam->cu_control_flag = 0;

  // initialize temp_alfps
  m_pcTempAlfParam->alf_flag        = 1;
  m_pcTempAlfParam->tap							= tap;
  m_pcTempAlfParam->num_coeff				= num_coef;
  m_pcTempAlfParam->chroma_idc      = 0;
  m_pcTempAlfParam->cu_control_flag = 0;

  // adaptive in-loop wiener filtering
  xEncALFLuma( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );

  // cu-based filter on/off control
  xCUAdaptiveControl( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );

  // adaptive tap-length
  xFilterTapDecision( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );

	// compute RD cost
  xCalcRDCost( pcPicOrg, pcPicYuvRec, m_pcBestAlfParam, uiMinRate, uiMinDist, dMinCost );

	// compare RD cost to non-ALF case
  if( dMinCost < dOrigCost )
  {
    m_pcBestAlfParam->alf_flag = 1;

    ruiBits = uiMinRate;
    ruiDist = uiMinDist;
  }
  else
  {
    m_pcBestAlfParam->alf_flag				= 0;
    m_pcBestAlfParam->cu_control_flag = 0;

		uiMinRate = uiOrigRate;
    uiMinDist = uiOrigDist;
    dMinCost = dMinCost;

    m_pcEntropyCoder->setAlfCtrl(false);
    pcPicYuvExtRec->copyToPicLuma(pcPicYuvRec);

    ruiBits = uiOrigRate;
    ruiDist = uiOrigDist;
  }

	// if ALF works
  if( m_pcBestAlfParam->alf_flag )
  {
		// predict ALF coefficients
    predictALFCoeff( m_pcBestAlfParam );

    // do additional ALF process for chroma
    xEncALFChroma( uiMinRate, pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, ruiDist, ruiBits );
  }

	// copy to best storage
  copyALFParam(pcAlfParam, m_pcBestAlfParam);

	// store best depth
  ruiMaxAlfCtrlDepth = m_pcEntropyCoder->getMaxAlfCtrlDepth();
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncAdaptiveLoopFilter::xEncALFLuma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
  UInt64  uiRate;
  UInt64  uiDist;
  Double dCost;

  Int tap, num_coef;

  tap                 = ALF_MIN_NUM_TAP;
  m_pcTempAlfParam->tap = tap;
  num_coef            = (tap*tap+1)>>1;
  num_coef            = num_coef + 1; // DC offset
  m_pcTempAlfParam->num_coeff = num_coef;

  xFilteringFrameLuma(pcPicOrg, pcPicDec, pcPicRest, true);
  xCalcRDCost(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost);

  if( dCost < rdMinCost)
  {
    ruiMinRate = uiRate;
    ruiMinDist = uiDist;
    rdMinCost = dCost;
    copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
  }
}

Void TEncAdaptiveLoopFilter::xEncALFChroma( UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits )
{
  // restriction for non-referenced B-slice
  if (m_eSliceType == B_SLICE && m_iPicNalReferenceIdc == 0)
  {
    return;
  }

  Int tap, num_coef;

  // set global variables
  tap         = ALF_MAX_NUM_TAP_C;
  num_coef    = (tap*tap+1)>>1;
  num_coef    = num_coef + 1; // DC offset

  // set min cost
  UInt64 uiMinRate = uiLumaRate;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;

  // calc original cost
  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
  xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiMinRate, uiMinDist, dMinCost);

  // initialize temp_alfps
  m_pcTempAlfParam->chroma_idc = 3;
  m_pcTempAlfParam->tap_chroma       = tap;
  m_pcTempAlfParam->num_coeff_chroma = num_coef;

  // Adaptive in-loop wiener filtering for chroma
  xFilteringFrameChroma(pcPicOrg, pcPicDec, pcPicRest);

  // filter on/off decision for chroma
  Int iCWidth = (pcPicOrg->getWidth()>>1);
  Int iCHeight = (pcPicOrg->getHeight()>>1);
  Int iCStride = pcPicOrg->getCStride();
  UInt64 uiFiltDistCb = xCalcSSD(pcPicOrg->getCbAddr(), pcPicRest->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiFiltDistCr = xCalcSSD(pcPicOrg->getCrAddr(), pcPicRest->getCrAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCb = xCalcSSD(pcPicOrg->getCbAddr(), pcPicDec->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCr = xCalcSSD(pcPicOrg->getCrAddr(), pcPicDec->getCrAddr(), iCWidth, iCHeight, iCStride);

  m_pcTempAlfParam->chroma_idc = 0;
  if(uiOrgDistCb > uiFiltDistCb)
    m_pcTempAlfParam->chroma_idc += 2;
  if(uiOrgDistCr  > uiFiltDistCr )
    m_pcTempAlfParam->chroma_idc += 1;

  if(m_pcTempAlfParam->chroma_idc)
  {
    if(m_pcTempAlfParam->chroma_idc!=3)
    {
      // chroma filter re-design
      xFilteringFrameChroma(pcPicOrg, pcPicDec, pcPicRest);
    }

    UInt64 uiRate, uiDist;
    Double dCost;
    xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost);

    if( dCost < dMinCost )
    {
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      predictALFCoeffChroma(m_pcBestAlfParam);

      ruiBits += uiRate;
      ruiDist += uiDist;
    }
    else
    {
      m_pcBestAlfParam->chroma_idc = 0;

      if((m_pcTempAlfParam->chroma_idc>>1)&0x01)
        pcPicDec->copyToPicCb(pcPicRest);
      if(m_pcTempAlfParam->chroma_idc&0x01)
        pcPicDec->copyToPicCr(pcPicRest);

      ruiBits += uiMinRate;
      ruiDist += uiMinDist;
    }
  }
  else
  {
    m_pcBestAlfParam->chroma_idc = 0;

    ruiBits += uiMinRate;
    ruiDist += uiMinDist;

		pcPicDec->copyToPicCb(pcPicRest);
    pcPicDec->copyToPicCr(pcPicRest);
  }
}

Void TEncAdaptiveLoopFilter::xCUAdaptiveControl(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
  m_pcEntropyCoder->setAlfCtrl(true);

  UInt uiBestDepth = 0;

  ALFParam cFrmAlfParam;
  allocALFParam(&cFrmAlfParam);
  copyALFParam(&cFrmAlfParam, m_pcBestAlfParam);

  for (UInt uiDepth = 0; uiDepth < g_uiMaxCUDepth; uiDepth++)
  {
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
    pcPicRest->copyToPicLuma(m_pcPicYuvTmp);
    copyALFParam(m_pcTempAlfParam, &cFrmAlfParam);
    m_pcTempAlfParam->cu_control_flag = 1;

    for (UInt uiRD = 0; uiRD <= ALF_NUM_OF_REDESIGN; uiRD++)
    {
      if (uiRD)
      {
        // re-design filter coefficients
        xReDesignFilterCoeff(pcPicOrg, pcPicDec, true);
        xFrame(pcPicDec, m_pcPicYuvTmp, m_pcTempAlfParam->coeff, m_pcTempAlfParam->tap);
      }

      UInt64 uiRate, uiDist;
      Double dCost;
      xSetCUAlfCtrlFlags(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist);

			// compute RD cost
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost);

      if (dCost < rdMinCost)
      {
        uiBestDepth = uiDepth;
        rdMinCost = dCost;
        ruiMinDist = uiDist;
        ruiMinRate = uiRate;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        xCopyTmpAlfCtrlFlagsFrom();
      }
    }
  }

  if (m_pcBestAlfParam->cu_control_flag)
  {
    m_pcEntropyCoder->setAlfCtrl(true);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiBestDepth);
    xCopyTmpAlfCtrlFlagsTo();
    m_pcPicYuvBest->copyToPicLuma(pcPicRest);
    xCopyDecToRestCUs(pcPicDec, pcPicRest);
  }
  else
  {
    m_pcEntropyCoder->setAlfCtrl(false);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(0);
  }
  freeALFParam(&cFrmAlfParam);
}

Void TEncAdaptiveLoopFilter::xFilterTapDecision(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
  // restriction for non-referenced B-slice
  if (m_eSliceType == B_SLICE && m_iPicNalReferenceIdc == 0)
  {
    return;
  }

  UInt64 uiRate, uiDist;
  Double dCost;

  if (m_pcBestAlfParam->cu_control_flag)
  {
    xCopyTmpAlfCtrlFlagsFrom();
  }

  Bool bChanged = false;
  for (Int iTap = ALF_MIN_NUM_TAP; iTap <= ALF_MAX_NUM_TAP; iTap += 2)
  {
    copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
    m_pcTempAlfParam->tap = iTap;
    m_pcTempAlfParam->num_coeff = ((iTap*iTap+1)>>1) + 1;
    if (m_pcTempAlfParam->cu_control_flag)
    {
      xReDesignFilterCoeff(pcPicOrg, pcPicDec, false);
      xFrame(pcPicDec, m_pcPicYuvTmp, m_pcTempAlfParam->coeff, m_pcTempAlfParam->tap);
      xSetCUAlfCtrlFlags(m_pcEntropyCoder->getMaxAlfCtrlDepth(), pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist);

			// compute RD cost
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost);
    }
    else
    {
      xFilteringFrameLuma(pcPicOrg, pcPicDec, m_pcPicYuvTmp, false);
      xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, m_pcTempAlfParam, uiRate, uiDist, dCost);
    }

    if (dCost < rdMinCost)
    {
      rdMinCost = dCost;
      ruiMinDist = uiDist;
      ruiMinRate = uiRate;
      m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      bChanged = true;
      if (m_pcTempAlfParam->cu_control_flag)
      {
        xCopyTmpAlfCtrlFlagsFrom();
      }
    }
  }

  if (m_pcBestAlfParam->cu_control_flag)
  {
    xCopyTmpAlfCtrlFlagsTo();
    if (bChanged)
    {
      m_pcPicYuvBest->copyToPicLuma(pcPicRest);
      xCopyDecToRestCUs(pcPicDec, pcPicRest);
    }
  }
  else if (m_pcBestAlfParam->tap > ALF_MIN_NUM_TAP)
  {
    m_pcPicYuvBest->copyToPicLuma(pcPicRest);
  }

  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Void TEncAdaptiveLoopFilter::xInitParam()
{
  Int i, j, k, l;

  if (m_ppdAlfCorr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorr[i][j] = 0;
      }
    }
  }
  else
  {
    m_ppdAlfCorr = new Double*[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_ppdAlfCorr[i] = new Double[ALF_MAX_NUM_COEF+1];
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorr[i][j] = 0;
      }
    }
  }

  if (m_puiCUCorr != NULL)
  {
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        for (k = 0; k < ALF_MIN_NUM_COEF; k++)
        {
          for (l = 0; l< ALF_MIN_NUM_COEF+1; l++)
          {
            m_puiCUCorr[i][j][k][l] = 0;
          }
        }
      }
    }
  }
  else
  {
    m_puiCUCorr = new UInt***[m_pcPic->getNumCUsInFrame()];
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      m_puiCUCorr[i] = new UInt**[m_uiNumSCUInCU];
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        m_puiCUCorr[i][j] = new UInt*[ALF_MIN_NUM_COEF];
        for (k = 0; k < ALF_MIN_NUM_COEF; k++)
        {
          m_puiCUCorr[i][j][k] = new UInt[ALF_MIN_NUM_COEF+1];
          for (l = 0; l< ALF_MIN_NUM_COEF+1; l++)
          {
            m_puiCUCorr[i][j][k][l] = 0;
          }
        }
      }
    }
  }

  if (m_pdDoubleAlfCoeff != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_pdDoubleAlfCoeff[i] = 0;
    }
  }
  else
  {
    m_pdDoubleAlfCoeff = new Double[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_pdDoubleAlfCoeff[i] = 0;
    }
  }
}

Void TEncAdaptiveLoopFilter::xUninitParam()
{
  Int i, j, k;

  if (m_ppdAlfCorr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      delete[] m_ppdAlfCorr[i];
      m_ppdAlfCorr[i] = NULL;
    }
    delete[] m_ppdAlfCorr;
    m_ppdAlfCorr = NULL;
  }

  if (m_puiCUCorr != NULL)
  {
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        for (k = 0; k < ALF_MIN_NUM_COEF; k++)
        {
          delete[] m_puiCUCorr[i][j][k];
          m_puiCUCorr[i][j][k] = NULL;
        }
        delete[] m_puiCUCorr[i][j];
        m_puiCUCorr[i][j] = NULL;
      }
      delete[] m_puiCUCorr[i];
      m_puiCUCorr[i] = NULL;
    }
    delete[] m_puiCUCorr;
    m_puiCUCorr = NULL;
  }

  if (m_pdDoubleAlfCoeff != NULL)
  {
    delete[] m_pdDoubleAlfCoeff;
    m_pdDoubleAlfCoeff = NULL;
  }
}

Void TEncAdaptiveLoopFilter::xCreateTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->createTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xDestroyTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->destroyTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsTo()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagFromTmp();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsFrom()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagToTmp();
  }
}

Void TEncAdaptiveLoopFilter::xReadOrCalcCorrFromCUs(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr)
{
  Int N = m_pcTempAlfParam->num_coeff;
  Int tap = m_pcTempAlfParam->tap;

  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    for (UInt uiIdx = 0; uiIdx < pcCU->getTotalNumPart(); uiIdx+=4)
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

      if (uiLPelX >= pcPicOrg->getWidth() || uiTPelY >= pcPicOrg->getHeight())
      {
        continue;
      }

      if (pcCU->getAlfCtrlFlag(uiIdx))
      {
        if (bReadCorr)
        {
          for(Int j=0; j<N; j++)
          {
            for(Int k=j; k<N+1; k++)
            {
              m_ppdAlfCorr[j][k] += (Double) m_puiCUCorr[uiCUAddr][(uiIdx>>2)][j][k-j];
            }
          }
        }
        else
        {
          Pel* pOrg = pcPicOrg->getLumaAddr(uiCUAddr, uiIdx);
          Pel* pCmp = pcPicDec->getLumaAddr(uiCUAddr, uiIdx);

          xCalcCorrelationFuncBlock(pOrg, pCmp, tap, m_uiSCUWidth, m_uiSCUHeight, pcPicOrg->getStride(), pcPicDec->getStride());
        }
      }
    }
  }
}

Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xEncodeCUAlfCtrlFlag(pcCU, 0, 0);
  }
}

Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xEncodeCUAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }

  m_pcEntropyCoder->encodeAlfCtrlFlag(pcCU, uiAbsPartIdx);
}

Void TEncAdaptiveLoopFilter::xCalcALFCoeff( ALFParam* pAlfParam )
{
  Int iErrCode;

  Int    *qh;

  Int tap			= pAlfParam->tap;
  Int N				= pAlfParam->num_coeff;
  Double* h   = m_pdDoubleAlfCoeff;
  qh					= pAlfParam->coeff;

  iErrCode = xGauss(m_ppdAlfCorr, N);

  if(iErrCode)
  {
    xClearFilterCoefInt(pAlfParam->coeff, N);
  }
  else
  {
    for(Int i=0; i<N; i++)
      h[i] = m_ppdAlfCorr[i][N];
    xQuantFilterCoef(h, pAlfParam->coeff, tap, g_uiBitDepth + g_uiBitIncrement);
  }
}

Void TEncAdaptiveLoopFilter::xCalcStoredCorrelationFuncBlock(Pel* pOrg, Pel* pCmp, UInt** ppuiCorr, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride)
{
  Int N      = (iTap*iTap+1)>>1;
  Int offset = iTap>>1;

  const Int* pFiltPos;

  switch(iTap)
  {
  case 5:
    pFiltPos = m_aiSymmetricArray5x5;
    break;
  case 7:
    pFiltPos = m_aiSymmetricArray7x7;
    break;
  case 9:
    pFiltPos = m_aiSymmetricArray9x9;
    break;
  default:
		pFiltPos = m_aiSymmetricArray9x9;
    assert(0);
    break;
  }

  UInt* pTerm = new UInt[N];

  Int i, j;

  for (Int y = 0; y < iHeight; y++)
  {
    for (Int x = 0; x < iWidth; x++)
    {
      i = 0;
      ::memset(pTerm, 0, sizeof(UInt)*N);
      for(Int yy=y-offset; yy<=y+offset; yy++)
      {
        for(Int xx=x-offset; xx<=x+offset; xx++)
        {
          pTerm[pFiltPos[i]] += (UInt) pCmp[xx + yy*iCmpStride];
          i++;
        }
      }

      for(j=0; j<N; j++)
      {
        ppuiCorr[j][0] += pTerm[j]*pTerm[j];
        for(i=j+1; i<N; i++)
          ppuiCorr[j][i-j] += pTerm[j]*pTerm[i];

        // DC offset
        ppuiCorr[j][N-j]   += pTerm[j];
        ppuiCorr[j][N-j+1] += (UInt) pOrg[x+y*iOrgStride]*pTerm[j];
      }
      // DC offset
      ppuiCorr[N][0] += 1;
      ppuiCorr[N][1] += (UInt) pOrg[x+y*iOrgStride];
    }
  }

  delete[] pTerm;
  pTerm = NULL;
}

Void TEncAdaptiveLoopFilter::xCalcCorrelationFunc(Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride)
{
  //Patch should be extended before this point................
  //ext_offset  = tap>>1;

  Int N      = (iTap*iTap+1)>>1;
  Int offset = iTap>>1;

  const Int* pFiltPos;

  switch(iTap)
  {
  case 5:
    pFiltPos = m_aiSymmetricArray5x5;
    break;
  case 7:
    pFiltPos = m_aiSymmetricArray7x7;
    break;
  case 9:
    pFiltPos = m_aiSymmetricArray9x9;
    break;
  default:
		pFiltPos = m_aiSymmetricArray9x9;
    assert(0);
    break;
  }

  Pel* pTerm = new Pel[N];

  Int i, j;

  for (Int y = 0; y < iHeight; y++)
  {
    for (Int x = 0; x < iWidth; x++)
    {
      i = 0;
      ::memset(pTerm, 0, sizeof(Pel)*N);
      for(Int yy=y-offset; yy<=y+offset; yy++)
      {
        for(Int xx=x-offset; xx<=x+offset; xx++)
        {
          pTerm[pFiltPos[i]] += pCmp[xx + yy*iCmpStride];
          i++;
        }
      }

      for(j=0; j<N; j++)
      {
        m_ppdAlfCorr[j][j] += pTerm[j]*pTerm[j];
        for(i=j+1; i<N; i++)
          m_ppdAlfCorr[j][i] += pTerm[j]*pTerm[i];

        // DC offset
        m_ppdAlfCorr[j][N]   += pTerm[j];
        m_ppdAlfCorr[j][N+1] += pOrg[x+y*iOrgStride]*pTerm[j];
      }
      // DC offset
      for(i=0; i<N; i++)
        m_ppdAlfCorr[N][i] += pTerm[i];
      m_ppdAlfCorr[N][N]   += 1;
      m_ppdAlfCorr[N][N+1] += pOrg[x+y*iOrgStride];
    }
  }

  for(j=0; j<N-1; j++)
    for(i=j+1; i<N; i++)
      m_ppdAlfCorr[i][j] = m_ppdAlfCorr[j][i];

  delete[] pTerm;
  pTerm = NULL;
}

Void TEncAdaptiveLoopFilter::xCalcCorrelationFuncBlock(Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride)
{
  //Patch should be extended before this point................
  //ext_offset  = tap>>1;

  Int N      = (iTap*iTap+1)>>1;
  Int offset = iTap>>1;

  const Int* pFiltPos;

  switch(iTap)
  {
  case 5:
    pFiltPos = m_aiSymmetricArray5x5;
    break;
  case 7:
    pFiltPos = m_aiSymmetricArray7x7;
    break;
  case 9:
    pFiltPos = m_aiSymmetricArray9x9;
    break;
  default:
		pFiltPos = m_aiSymmetricArray9x9;
    assert(0);
    break;
  }

  Pel* pTerm = new Pel[N];

  Int i, j;

  for (Int y = 0; y < iHeight; y++)
  {
    for (Int x = 0; x < iWidth; x++)
    {
      i = 0;
      ::memset(pTerm, 0, sizeof(Pel)*N);
      for(Int yy=y-offset; yy<=y+offset; yy++)
      {
        for(Int xx=x-offset; xx<=x+offset; xx++)
        {
          pTerm[pFiltPos[i]] += pCmp[xx + yy*iCmpStride];
          i++;
        }
      }

      for(j=0; j<N; j++)
      {
        m_ppdAlfCorr[j][j] += pTerm[j]*pTerm[j];
        for(i=j+1; i<N; i++)
          m_ppdAlfCorr[j][i] += pTerm[j]*pTerm[i];

        // DC offset
        m_ppdAlfCorr[j][N]   += pTerm[j];
        m_ppdAlfCorr[j][N+1] += pOrg[x+y*iOrgStride]*pTerm[j];
      }
      // DC offset
      for(i=0; i<N; i++)
        m_ppdAlfCorr[N][i] += pTerm[i];
      m_ppdAlfCorr[N][N]   += 1;
      m_ppdAlfCorr[N][N+1] += pOrg[x+y*iOrgStride];
    }
  }

  delete[] pTerm;
  pTerm = NULL;
}

UInt64 TEncAdaptiveLoopFilter::xCalcSSD(Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride )
{
  UInt64 uiSSD = 0;
  Int x, y;

  UInt uiShift = g_uiBitIncrement<<1;
  Int iTemp;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pOrg[x] - pCmp[x]; uiSSD += ( iTemp * iTemp ) >> uiShift;
    }
    pOrg += iStride;
    pCmp += iStride;
  }

	return uiSSD;;
}

Int TEncAdaptiveLoopFilter::xGauss(Double **a, Int N)
{
  Int i, j, k;
  Double t;

#if ALF_FIX
  for(k=0; k<N; k++)
  {
    if (a[k][k] <0.000001)
        return 1;
  }
#endif

  for(k=0; k<N-1; k++)
  {
    for(i=k+1;i<N; i++)
    {
      t=a[i][k]/a[k][k];
      for(j=k+1; j<=N; j++)
      {
        a[i][j] -= t * a[k][j];
        if(i==j && fabs(a[i][j])<0.000001) return 1;
      }
    }
  }
  for(i=N-1; i>=0; i--)
  {
    t = a[i][N];
    for(j=i+1; j<N; j++)
      t -= a[i][j] * a[j][N];
    a[i][N] = t / a[i][i];
  }
  return 0;
}

Void TEncAdaptiveLoopFilter::xFilterCoefQuickSort( Double *coef_data, Int *coef_num, Int upper, Int lower )
{
  Double mid, tmp_data;
  Int i, j, tmp_num;

  i = upper;
  j = lower;
  mid = coef_data[(lower+upper)>>1];
  do
  {
    while( coef_data[i] < mid ) i++;
    while( mid < coef_data[j] ) j--;
    if( i <= j )
    {
      tmp_data = coef_data[i];
      tmp_num  = coef_num[i];
      coef_data[i] = coef_data[j];
      coef_num[i]  = coef_num[j];
      coef_data[j] = tmp_data;
      coef_num[j]  = tmp_num;
      i++;
      j--;
    }
  } while( i <= j );
  if ( upper < j ) xFilterCoefQuickSort(coef_data, coef_num, upper, j);
  if ( i < lower ) xFilterCoefQuickSort(coef_data, coef_num, i, lower);
}

Void TEncAdaptiveLoopFilter::xQuantFilterCoef(Double* h, Int* qh, Int tap, int bit_depth)
{
  Int i, N;
  Int max_value, min_value;
  Double dbl_total_gain;
  Int total_gain, q_total_gain;
  Int upper, lower;
  Double *dh;
  Int    *nc;
  const Int    *pFiltMag;

  switch(tap)
  {
  case 5:
    pFiltMag = m_aiSymmetricMag5x5;
    break;
  case 7:
    pFiltMag = m_aiSymmetricMag7x7;
    break;
  case 9:
    pFiltMag = m_aiSymmetricMag9x9;
    break;
  default:
		pFiltMag = m_aiSymmetricMag9x9;
    assert(0);
    break;
  }

  N = (tap*tap+1)>>1;

  dh = new Double[N];
  nc = new Int[N];

  max_value =   (1<<(1+ALF_NUM_BIT_SHIFT))-1;
  min_value = 0-(1<<(1+ALF_NUM_BIT_SHIFT));

  dbl_total_gain=0.0;
  q_total_gain=0;
  for(i=0; i<N; i++)
  {
    if(h[i]>=0.0)
      qh[i] =  (Int)( h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
    else
      qh[i] = -(Int)(-h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);

    dh[i] = (Double)qh[i]/(Double)(1<<ALF_NUM_BIT_SHIFT) - h[i];
    dh[i]*=pFiltMag[i];
    dbl_total_gain += h[i]*pFiltMag[i];
    q_total_gain   += qh[i]*pFiltMag[i];
    nc[i] = i;
  }

  // modification of quantized filter coefficients
  total_gain = (Int)(dbl_total_gain*(1<<ALF_NUM_BIT_SHIFT)+0.5);

  if( q_total_gain != total_gain )
  {
    xFilterCoefQuickSort(dh, nc, 0, N-1);
    if( q_total_gain > total_gain )
    {
      upper = N-1;
      while( q_total_gain > total_gain+1 )
      {
        i = nc[upper%N];
        qh[i]--;
        q_total_gain -= pFiltMag[i];
        upper--;
      }
      if( q_total_gain == total_gain+1 )
      {
        if(dh[N-1]>0)
          qh[N-1]--;
        else
        {
          i=nc[upper%N];
          qh[i]--;
          qh[N-1]++;
        }
      }
    }
    else if( q_total_gain < total_gain )
    {
      lower = 0;
      while( q_total_gain < total_gain-1 )
      {
        i=nc[lower%N];
        qh[i]++;
        q_total_gain += pFiltMag[i];
        lower++;
      }
      if( q_total_gain == total_gain-1 )
      {
        if(dh[N-1]<0)
          qh[N-1]++;
        else
        {
          i=nc[lower%N];
          qh[i]++;
          qh[N-1]--;
        }
      }
    }
  }

  // set of filter coefficients
  for(i=0; i<N; i++)
  {
    qh[i] = Max(min_value,Min(max_value, qh[i]));
  }

  // DC offset
//  max_value = Min(  (1<<(3+Max(img_bitdepth_luma,img_bitdepth_chroma)))-1, (1<<14)-1);
//  min_value = Max( -(1<<(3+Max(img_bitdepth_luma,img_bitdepth_chroma))),  -(1<<14)  );
  max_value = Min(  (1<<(3+g_uiBitDepth + g_uiBitIncrement))-1, (1<<14)-1);
  min_value = Max( -(1<<(3+g_uiBitDepth + g_uiBitIncrement)),  -(1<<14)  );

  qh[N] =  (h[N]>=0.0)? (Int)( h[N]*(1<<(ALF_NUM_BIT_SHIFT-bit_depth+8)) + 0.5) : -(Int)(-h[N]*(1<<(ALF_NUM_BIT_SHIFT-bit_depth+8)) + 0.5);
  qh[N] = Max(min_value,Min(max_value, qh[N]));

  delete[] dh;
  dh = NULL;

  delete[] nc;
  nc = NULL;
}

Void TEncAdaptiveLoopFilter::xClearFilterCoefInt(Int* qh, Int N)
{
	// clear
  memset( qh, 0, sizeof( Int ) * N );

  // center pos
  qh[N-2]  = 1<<ALF_NUM_BIT_SHIFT;
}

Void TEncAdaptiveLoopFilter::xCalcRDCost(ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost)
{
  if(pAlfParam != NULL)
  {
    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF];

    memcpy(piTmpCoef, pAlfParam->coeff, sizeof(Int)*pAlfParam->num_coeff);

    predictALFCoeff(pAlfParam);

    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);

    if(pAlfParam->cu_control_flag)
    {
      xEncodeCUAlfCtrlFlags();
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
    memcpy(pAlfParam->coeff, piTmpCoef, sizeof(int)*pAlfParam->num_coeff);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
  }
  else
  {
    ruiRate = 1;
  }

  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(uiDist);
}

Void TEncAdaptiveLoopFilter::xCalcRDCost(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost)
{
  if(pAlfParam != NULL)
  {
    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF];

    memcpy(piTmpCoef, pAlfParam->coeff, sizeof(Int)*pAlfParam->num_coeff);

    predictALFCoeff(pAlfParam);

    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);

    if(pAlfParam->cu_control_flag)
    {
      xEncodeCUAlfCtrlFlags();
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
    memcpy(pAlfParam->coeff, piTmpCoef, sizeof(int)*pAlfParam->num_coeff);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
  }
  else
  {
    ruiRate = 1;
  }

  ruiDist     = xCalcSSD(pcPicOrg->getLumaAddr(), pcPicCmp->getLumaAddr(), pcPicOrg->getWidth(), pcPicOrg->getHeight(), pcPicOrg->getStride());
  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(ruiDist);
}

Void TEncAdaptiveLoopFilter::xCalcRDCostChroma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost)
{
  if(pAlfParam->chroma_idc)
  {
    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF_C];

    memcpy(piTmpCoef, pAlfParam->coeff_chroma, sizeof(Int)*pAlfParam->num_coeff_chroma);

    predictALFCoeffChroma(pAlfParam);

    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);

    if(pAlfParam->cu_control_flag)
    {
      xEncodeCUAlfCtrlFlags();
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
    memcpy(pAlfParam->coeff_chroma, piTmpCoef, sizeof(int)*pAlfParam->num_coeff_chroma);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
  }
  ruiDist = 0;
  ruiDist += xCalcSSD(pcPicOrg->getCbAddr(), pcPicCmp->getCbAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  ruiDist += xCalcSSD(pcPicOrg->getCrAddr(), pcPicCmp->getCrAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  rdCost  = (Double)(ruiRate) * m_dLambdaChroma + (Double)(ruiDist);
}

Void TEncAdaptiveLoopFilter::xFilteringFrameLuma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Bool bStoreCorr)
{
  Int    i, tap, N, err_code;
  Int* qh;
  Int    j, k;
  UInt** ppuiBlkCorr;

  tap  = m_pcTempAlfParam->tap;
  N    = m_pcTempAlfParam->num_coeff;
  qh   = m_pcTempAlfParam->coeff;

  // initialize correlation
  for(i=0; i<N; i++)
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(N+1));

  if(bStoreCorr)
  {
    // store correlation per minimum size cu
    for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
    for( UInt uiIdx = 0; uiIdx < m_pcPic->getNumPartInCU() ; uiIdx+=4 )
    {
      TComDataCU* pcCU = m_pcPic->getCU(uiCUAddr);
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

      if (uiLPelX >= pcPicOrg->getWidth() || uiTPelY >= pcPicOrg->getHeight())
      {
        continue;
      }
      ppuiBlkCorr = m_puiCUCorr[uiCUAddr][(uiIdx>>2)];

      for(j=0; j<N; j++)
        memset(ppuiBlkCorr[j], 0, sizeof(UInt)*(N+1-j));

      Pel* pOrg = pcPicOrg->getLumaAddr(uiCUAddr, uiIdx);
      Pel* pCmp = pcPicDec->getLumaAddr(uiCUAddr, uiIdx);
      xCalcStoredCorrelationFuncBlock(pOrg, pCmp, ppuiBlkCorr, tap, m_uiSCUWidth, m_uiSCUHeight, pcPicOrg->getStride(), pcPicDec->getStride());

      for(j=0; j<N; j++)
        for(k=j; k<N+1; k++)
          m_ppdAlfCorr[j][k] += (Double) ppuiBlkCorr[j][k-j];
    }
    for(j=0; j<N-1; j++)
      for(k=j+1; k<N; k++)
        m_ppdAlfCorr[k][j] = m_ppdAlfCorr[j][k];
  }
  else
  {
    Pel* pOrg = pcPicOrg->getLumaAddr();
    Pel* pCmp = pcPicDec->getLumaAddr();

    xCalcCorrelationFunc(pOrg, pCmp, tap, pcPicOrg->getWidth(), pcPicOrg->getHeight(), pcPicOrg->getStride(), pcPicDec->getStride());
  }

  err_code = xGauss(m_ppdAlfCorr, N);

  if(err_code)
  {
    xClearFilterCoefInt(qh, N);
  }
  else
  {
    for(i=0; i<N; i++)
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][N];

    xQuantFilterCoef(m_pdDoubleAlfCoeff, qh, tap, g_uiBitDepth + g_uiBitIncrement);
  }

  xFrame(pcPicDec, pcPicRest, qh, tap);
}

Void TEncAdaptiveLoopFilter::xFilteringFrameChroma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Int    i, tap, N, err_code;
  Int* qh;

  tap  = m_pcTempAlfParam->tap_chroma;
  N    = m_pcTempAlfParam->num_coeff_chroma;
  qh   = m_pcTempAlfParam->coeff_chroma;

  // initialize correlation
  for(i=0; i<N; i++)
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(N+1));

  if ((m_pcTempAlfParam->chroma_idc>>1)&0x01)
  {
    Pel* pOrg = pcPicOrg->getCbAddr();
    Pel* pCmp = pcPicDec->getCbAddr();

    xCalcCorrelationFunc(pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
  }
  if ((m_pcTempAlfParam->chroma_idc)&0x01)
  {
    Pel* pOrg = pcPicOrg->getCrAddr();
    Pel* pCmp = pcPicDec->getCrAddr();

    xCalcCorrelationFunc(pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
  }

  err_code = xGauss(m_ppdAlfCorr, N);

  if(err_code)
  {
    xClearFilterCoefInt(qh, N);
  }
  else
  {
    for(i=0; i<N; i++)
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][N];

    xQuantFilterCoef(m_pdDoubleAlfCoeff, qh, tap, g_uiBitDepth + g_uiBitIncrement);
  }


  if ((m_pcTempAlfParam->chroma_idc>>1)&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, qh, tap, 0);
  }
  if ((m_pcTempAlfParam->chroma_idc)&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, qh, tap, 1);
  }

  if(m_pcTempAlfParam->chroma_idc<3)
  {
    if(m_pcTempAlfParam->chroma_idc==1)
    {
      pcPicDec->copyToPicCb(pcPicRest);
    }
    if(m_pcTempAlfParam->chroma_idc==2)
    {
      pcPicDec->copyToPicCr(pcPicRest);
    }
  }

}

Void TEncAdaptiveLoopFilter::xReDesignFilterCoeff(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr)
{
  Int i, j, k;

  Int tap = m_pcTempAlfParam->tap;
  Int N = m_pcTempAlfParam->num_coeff;
  Int* qh = m_pcTempAlfParam->coeff;
  // initialize correlation
  for(i=0; i<N; i++)
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(N+1));

  xReadOrCalcCorrFromCUs(pcPicOrg, pcPicDec, bReadCorr);

  for(j=0; j<N-1; j++)
    for(k=j+1; k<N; k++)
      m_ppdAlfCorr[k][j] = m_ppdAlfCorr[j][k];

  Int err_code = xGauss(m_ppdAlfCorr, N);

  if(err_code)
  {
    xClearFilterCoefInt(qh, N);
  }
  else
  {
    for(i=0; i<N; i++)
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][N];

    xQuantFilterCoef(m_pdDoubleAlfCoeff, qh, tap, g_uiBitDepth + g_uiBitIncrement);
  }
}

Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlags(UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist)
{
  ruiDist = 0;

  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xSetCUAlfCtrlFlag(pcCU, 0, 0, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist);
  }
}

Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSetCUAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth+1, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist);
    }
    return;
  }

  if( uiDepth > uiAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    return;
  }

  UInt uiCUAddr = pcCU->getAddr();
  UInt64 uiRecSSD = 0;
  UInt64 uiFiltSSD = 0;

  Int iWidth;
  Int iHeight;
  UInt uiSetDepth;

  if (uiDepth > uiAlfCtrlDepth && pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    iWidth = g_uiMaxCUWidth >> uiAlfCtrlDepth;
    iHeight = g_uiMaxCUHeight >> uiAlfCtrlDepth;

    uiRPelX   = uiLPelX + iWidth  - 1;
    uiBPelY   = uiTPelY + iHeight - 1;

    if( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() )
    {
      iWidth = pcCU->getSlice()->getSPS()->getWidth() - uiLPelX;
    }

    if( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() )
    {
      iHeight = pcCU->getSlice()->getSPS()->getHeight() - uiTPelY;
    }

    uiSetDepth = uiAlfCtrlDepth;
  }
  else
  {
    iWidth = pcCU->getWidth(uiAbsPartIdx);
    iHeight = pcCU->getHeight(uiAbsPartIdx);
    uiSetDepth = uiDepth;
  }

  Pel* pOrg = pcPicOrg->getLumaAddr(uiCUAddr, uiAbsPartIdx);
  Pel* pRec = pcPicDec->getLumaAddr(uiCUAddr, uiAbsPartIdx);
  Pel* pFilt = pcPicRest->getLumaAddr(uiCUAddr, uiAbsPartIdx);

  uiRecSSD  += xCalcSSD( pOrg, pRec,  iWidth, iHeight, pcPicOrg->getStride() );
  uiFiltSSD += xCalcSSD( pOrg, pFilt, iWidth, iHeight, pcPicOrg->getStride() );

  if (uiFiltSSD < uiRecSSD)
  {
    ruiDist += uiFiltSSD;
    pcCU->setAlfCtrlFlagSubParts(1, uiAbsPartIdx, uiSetDepth);
  }
  else
  {
    ruiDist += uiRecSSD;
    pcCU->setAlfCtrlFlagSubParts(0, uiAbsPartIdx, uiSetDepth);
  }
}

Void TEncAdaptiveLoopFilter::xCopyDecToRestCUs(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xCopyDecToRestCU(pcCU, 0, 0, pcPicDec, pcPicRest);
  }
}

Void TEncAdaptiveLoopFilter::xCopyDecToRestCU(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xCopyDecToRestCU(pcCU, uiAbsPartIdx, uiDepth+1, pcPicDec, pcPicRest);
    }
    return;
  }

  if (!pcCU->getAlfCtrlFlag(uiAbsPartIdx))
  {
    UInt uiCUAddr = pcCU->getAddr();

    Int iWidth = pcCU->getWidth(uiAbsPartIdx);
    Int iHeight = pcCU->getHeight(uiAbsPartIdx);

    Pel* pRec = pcPicDec->getLumaAddr(uiCUAddr, uiAbsPartIdx);
    Pel* pFilt = pcPicRest->getLumaAddr(uiCUAddr, uiAbsPartIdx);

    Int iRecStride = pcPicDec->getStride();
    Int iFiltStride = pcPicRest->getStride();

    for (Int y = 0; y < iHeight; y++)
    {
      for (Int x = 0; x < iWidth; x++)
      {
        pFilt[x] = pRec[x];
      }
      pRec += iRecStride;
      pFilt += iFiltStride;
    }
  }
}


