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

/** \file			TComAdaptiveLoopFilter.cpp
    \brief		adaptive loop filter class
*/

#include "TComAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ====================================================================================================================
// Tables
// ====================================================================================================================

// scaling factor for quantization of filter coefficients (9x9)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag9x9[41] =
{
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (7x7)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag7x7[25] =
{
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (5x5)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag5x5[13] =
{
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 1
};

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComAdaptiveLoopFilter::TComAdaptiveLoopFilter()
{
	m_pcTempPicYuv = NULL;
}

Void TComAdaptiveLoopFilter::create( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
	if ( !m_pcTempPicYuv )
	{
		m_pcTempPicYuv = new TComPicYuv;
		m_pcTempPicYuv->create( iPicWidth, iPicHeight, uiMaxCUWidth, uiMaxCUHeight, uiMaxCUDepth );
	}
}

Void TComAdaptiveLoopFilter::destroy()
{
	if ( m_pcTempPicYuv )
	{
		m_pcTempPicYuv->destroy();
		delete m_pcTempPicYuv;
	}
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// allocate / free / copy functions
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::allocALFParam(ALFParam* pAlfParam)
{
  pAlfParam->alf_flag = 0;

  pAlfParam->coeff				= new Int[ALF_MAX_NUM_COEF];
  pAlfParam->coeff_chroma = new Int[ALF_MAX_NUM_COEF_C];

  ::memset(pAlfParam->coeff,				0, sizeof(Int)*ALF_MAX_NUM_COEF		);
  ::memset(pAlfParam->coeff_chroma, 0, sizeof(Int)*ALF_MAX_NUM_COEF_C	);
}

Void TComAdaptiveLoopFilter::freeALFParam(ALFParam* pAlfParam)
{
  assert(pAlfParam != NULL);

  if (pAlfParam->coeff != NULL)
  {
    delete[] pAlfParam->coeff;
    pAlfParam->coeff = NULL;
  }

  if (pAlfParam->coeff_chroma != NULL)
  {
    delete[] pAlfParam->coeff_chroma;
    pAlfParam->coeff_chroma = NULL;
  }
}

Void TComAdaptiveLoopFilter::copyALFParam(ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam)
{
  pDesAlfParam->alf_flag = pSrcAlfParam->alf_flag;
  pDesAlfParam->cu_control_flag = pSrcAlfParam->cu_control_flag;
  pDesAlfParam->chroma_idc = pSrcAlfParam->chroma_idc;
  pDesAlfParam->tap = pSrcAlfParam->tap;
  pDesAlfParam->num_coeff = pSrcAlfParam->num_coeff;
  pDesAlfParam->tap_chroma = pSrcAlfParam->tap_chroma;
  pDesAlfParam->num_coeff_chroma = pSrcAlfParam->num_coeff_chroma;

  ::memcpy(pDesAlfParam->coeff, pSrcAlfParam->coeff, sizeof(Int)*ALF_MAX_NUM_COEF);
  ::memcpy(pDesAlfParam->coeff_chroma, pSrcAlfParam->coeff_chroma, sizeof(Int)*ALF_MAX_NUM_COEF_C);
}

// --------------------------------------------------------------------------------------------------------------------
// prediction of filter coefficients
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::predictALFCoeff( ALFParam* pAlfParam)
{
  Int i, sum, pred, tap, N;
  const Int* pFiltMag;

  tap = pAlfParam->tap;

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
    assert(0);
    break;
  }
  N = (tap*tap+1)>>1;
  sum=0;
  for(i=0; i<N-1;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-sum;
  pAlfParam->coeff[N-1]=pred-pAlfParam->coeff[N-1];
}

Void TComAdaptiveLoopFilter::predictALFCoeffChroma( ALFParam* pAlfParam )
{
  Int i, sum, pred, tap, N;
  const Int* pFiltMag;

  tap = pAlfParam->tap_chroma;
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
    assert(0);
    break;
  }
  N = (tap*tap+1)>>1;
  sum=0;
  for(i=0; i<N;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff_chroma[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-(sum-pAlfParam->coeff_chroma[N-1]);
  pAlfParam->coeff_chroma[N-1]=pred-pAlfParam->coeff_chroma[N-1];
}

// --------------------------------------------------------------------------------------------------------------------
// interface function for actual ALF process
// --------------------------------------------------------------------------------------------------------------------

/**
		\param	pcPic					picture (TComPic) class (input/output)
		\param	pcAlfParam		ALF parameter
		\todo   for temporal buffer, it uses residual picture buffer, which may not be safe. Make it be safer.
 */
Void TComAdaptiveLoopFilter::ALFProcess(TComPic* pcPic, ALFParam* pcAlfParam)
{
  if(!pcAlfParam->alf_flag)
  {
    return;
  }

  TComPicYuv* pcPicYuvRec    = pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;

  pcPicYuvRec		->copyToPic						( pcPicYuvExtRec );
	pcPicYuvExtRec->setBorderExtension	( false );
  pcPicYuvExtRec->extendPicBorder			();

	predictALFCoeff(pcAlfParam);
  xALFLuma(pcPic, pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);

  if(pcAlfParam->chroma_idc)
  {
		predictALFCoeffChroma(pcAlfParam);
		xALFChroma( pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// ALF for luma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFLuma(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if(pcAlfParam->cu_control_flag)
  {
		// block-adaptive ALF process
    xCUAdaptive(pcPic, pcAlfParam, pcPicDec, pcPicRest);
  }
  else
  {
		// non-adaptive ALF process
    xFrame(pcPicDec, pcPicRest, pcAlfParam->coeff, pcAlfParam->tap);
  }
}

Void TComAdaptiveLoopFilter::xCUAdaptive(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
	// for every CU, call CU-adaptive ALF process
  for( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    xSubCUAdaptive(pcCU, pcAlfParam, pcPicDec, pcPicRest, 0, 0);
  }
}

/**
		- For every sub-CU's, it is called recursively
		.
		\param	pcCU					CU data structure
		\param	pcAlfParam		ALF parameter
		\param  pcPicDec			picture before ALF
		\retval	pcPicRest			picture after  ALF
		\param  uiAbsPartIdx	current sub-CU position
		\param	uiDepth				current sub-CU depth
 */
Void TComAdaptiveLoopFilter::xSubCUAdaptive(TComDataCU* pcCU, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt uiAbsPartIdx, UInt uiDepth)
{
  TComPic* pcPic = pcCU->getPic();

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

	// check picture boundary
  if ( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

	// go to sub-CU?
  if ( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSubCUAdaptive(pcCU, pcAlfParam, pcPicDec, pcPicRest, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }

	// check CU-based flag
  if ( pcCU->getAlfCtrlFlag(uiAbsPartIdx) )
  {
    xSubCUFilter( pcPicDec->getLumaAddr(pcCU->getAddr(), uiAbsPartIdx), pcPicRest->getLumaAddr(pcCU->getAddr(), uiAbsPartIdx),
                       pcAlfParam->coeff, pcAlfParam->tap,
                       pcCU->getWidth(uiAbsPartIdx), pcCU->getHeight(uiAbsPartIdx),
                       pcPicDec->getStride(), pcPicRest->getStride() );
  }
}

/** \todo		filtering operation is not optimized as frame-based functions
 */
Void TComAdaptiveLoopFilter::xSubCUFilter( Pel* pDec, Pel* pRest, Int *qh, Int iTap, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride )
{
  Int i, x, y, value, N, offset;
  Pel PixSum[ALF_MAX_NUM_COEF];

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Pel* pTmpDec;

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  switch(iTap)
  {
  case 5:
    {
    pDec -= iDecStride*2;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 2;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[5] = pTmpDec[0];
        PixSum[6] = pTmpDec[1];
        PixSum[7] = pTmpDec[2];
        PixSum[8] = pTmpDec[3];
        PixSum[9] = pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[10] = pTmpDec[0];
        PixSum[11] = pTmpDec[1];
        PixSum[12] = pTmpDec[2];
        PixSum[11]+= pTmpDec[3];
        PixSum[10]+= pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[9]+= pTmpDec[0];
        PixSum[8]+= pTmpDec[1];
        PixSum[7]+= pTmpDec[2];
        PixSum[6]+= pTmpDec[3];
        PixSum[5]+= pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[4]+= pTmpDec[0];
        PixSum[3]+= pTmpDec[1];
        PixSum[2]+= pTmpDec[2];
        PixSum[1]+= pTmpDec[3];
        PixSum[0]+= pTmpDec[4];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = ( value + ALF_ROUND_OFFSET ) >> ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  case 7:
    {
    pDec -= iDecStride*3;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 3;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];
        PixSum[5] = pTmpDec[5];
        PixSum[6] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[7] = pTmpDec[0];
        PixSum[8] = pTmpDec[1];
        PixSum[9] = pTmpDec[2];
        PixSum[10] = pTmpDec[3];
        PixSum[11] = pTmpDec[4];
        PixSum[12] = pTmpDec[5];
        PixSum[13] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[14] = pTmpDec[0];
        PixSum[15] = pTmpDec[1];
        PixSum[16] = pTmpDec[2];
        PixSum[17] = pTmpDec[3];
        PixSum[18] = pTmpDec[4];
        PixSum[19] = pTmpDec[5];
        PixSum[20] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[21] = pTmpDec[0];
        PixSum[22] = pTmpDec[1];
        PixSum[23] = pTmpDec[2];
        PixSum[24] = pTmpDec[3];
        PixSum[23]+= pTmpDec[4];
        PixSum[22]+= pTmpDec[5];
        PixSum[21]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[20]+= pTmpDec[0];
        PixSum[19]+= pTmpDec[1];
        PixSum[18]+= pTmpDec[2];
        PixSum[17]+= pTmpDec[3];
        PixSum[16]+= pTmpDec[4];
        PixSum[15]+= pTmpDec[5];
        PixSum[14]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[13]+= pTmpDec[0];
        PixSum[12]+= pTmpDec[1];
        PixSum[11]+= pTmpDec[2];
        PixSum[10]+= pTmpDec[3];
        PixSum[9]+= pTmpDec[4];
        PixSum[8]+= pTmpDec[5];
        PixSum[7]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[6]+= pTmpDec[0];
        PixSum[5]+= pTmpDec[1];
        PixSum[4]+= pTmpDec[2];
        PixSum[3]+= pTmpDec[3];
        PixSum[2]+= pTmpDec[4];
        PixSum[1]+= pTmpDec[5];
        PixSum[0]+= pTmpDec[6];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  case 9:
    {
    pDec -= iDecStride*4;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 4;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];
        PixSum[5] = pTmpDec[5];
        PixSum[6] = pTmpDec[6];
        PixSum[7] = pTmpDec[7];
        PixSum[8] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[9] = pTmpDec[0];
        PixSum[10] = pTmpDec[1];
        PixSum[11] = pTmpDec[2];
        PixSum[12] = pTmpDec[3];
        PixSum[13] = pTmpDec[4];
        PixSum[14] = pTmpDec[5];
        PixSum[15] = pTmpDec[6];
        PixSum[16] = pTmpDec[7];
        PixSum[17] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[18] = pTmpDec[0];
        PixSum[19] = pTmpDec[1];
        PixSum[20] = pTmpDec[2];
        PixSum[21] = pTmpDec[3];
        PixSum[22] = pTmpDec[4];
        PixSum[23] = pTmpDec[5];
        PixSum[24] = pTmpDec[6];
        PixSum[25] = pTmpDec[7];
        PixSum[26] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[27] = pTmpDec[0];
        PixSum[28] = pTmpDec[1];
        PixSum[29] = pTmpDec[2];
        PixSum[30] = pTmpDec[3];
        PixSum[31] = pTmpDec[4];
        PixSum[32] = pTmpDec[5];
        PixSum[33] = pTmpDec[6];
        PixSum[34] = pTmpDec[7];
        PixSum[35] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[36] = pTmpDec[0];
        PixSum[37] = pTmpDec[1];
        PixSum[38] = pTmpDec[2];
        PixSum[39] = pTmpDec[3];
        PixSum[40] = pTmpDec[4];
        PixSum[39]+= pTmpDec[5];
        PixSum[38]+= pTmpDec[6];
        PixSum[37]+= pTmpDec[7];
        PixSum[36]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[35]+= pTmpDec[0];
        PixSum[34]+= pTmpDec[1];
        PixSum[33]+= pTmpDec[2];
        PixSum[32]+= pTmpDec[3];
        PixSum[31]+= pTmpDec[4];
        PixSum[30]+= pTmpDec[5];
        PixSum[29]+= pTmpDec[6];
        PixSum[28]+= pTmpDec[7];
        PixSum[27]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[26]+= pTmpDec[0];
        PixSum[25]+= pTmpDec[1];
        PixSum[24]+= pTmpDec[2];
        PixSum[23]+= pTmpDec[3];
        PixSum[22]+= pTmpDec[4];
        PixSum[21]+= pTmpDec[5];
        PixSum[20]+= pTmpDec[6];
        PixSum[19]+= pTmpDec[7];
        PixSum[18]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[17]+= pTmpDec[0];
        PixSum[16]+= pTmpDec[1];
        PixSum[15]+= pTmpDec[2];
        PixSum[14]+= pTmpDec[3];
        PixSum[13]+= pTmpDec[4];
        PixSum[12]+= pTmpDec[5];
        PixSum[11]+= pTmpDec[6];
        PixSum[10]+= pTmpDec[7];
        PixSum[9]+=  pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[8]+= pTmpDec[0];
        PixSum[7]+= pTmpDec[1];
        PixSum[6]+= pTmpDec[2];
        PixSum[5]+= pTmpDec[3];
        PixSum[4]+= pTmpDec[4];
        PixSum[3]+= pTmpDec[5];
        PixSum[2]+= pTmpDec[6];
        PixSum[1]+= pTmpDec[7];
        PixSum[0]+= pTmpDec[8];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  default:
    assert(0);
    break;
  }
}

Void TComAdaptiveLoopFilter::xFrame(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap)
{
  Int i, x, y, value, N, offset;
  Pel PixSum[ALF_MAX_NUM_COEF];

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Int iHeight = pcPicRest->getHeight();
  Int iWidth = pcPicRest->getWidth();

  Pel* pDec = pcPicDec->getLumaAddr();
  Int iDecStride = pcPicDec->getStride();

  Pel* pRest = pcPicRest->getLumaAddr();
  Int iRestStride = pcPicRest->getStride();

  Pel* pTmpDec1, *pTmpDec2;
  Pel* pTmpPixSum;

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  switch(iTap)
  {
  case 5:
    {
      Int iJump = iDecStride - 4;
      pDec -= iDecStride*2;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-2;
          pTmpDec2 = pTmpDec1+4+(4*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;


          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<13; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[13] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 7:
    {
      Int iJump = iDecStride - 6;
      pDec -= iDecStride*3;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-3;
          pTmpDec2 = pTmpDec1+6+(6*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<25; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[25] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 9:
    {
      Int iJump = iDecStride - 8;
      pDec -= iDecStride*4;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {

          pTmpDec1 = pDec+x-4;
          pTmpDec2 = pTmpDec1+8+(8*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<41; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[41] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  default:
    assert(0);
    break;
  }
}

// --------------------------------------------------------------------------------------------------------------------
// ALF for chroma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFChroma(ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if((pcAlfParam->chroma_idc>>1)&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 0);
  }

  if(pcAlfParam->chroma_idc&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 1);
  }
}

/** \param	pcPicDec			picture before ALF
		\param	pcPicRest			picture after  ALF
		\param	qh						filter coefficient
		\param	iTap					filter tap
		\param	iColor				0 for Cb and 1 for Cr
 */
Void TComAdaptiveLoopFilter::xFrameChroma( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor )
{
  Int i, x, y, value, N, offset;
  Pel PixSum[ALF_MAX_NUM_COEF_C];

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Int iHeight = pcPicRest->getHeight() >> 1;
  Int iWidth = pcPicRest->getWidth() >> 1;

  Pel* pDec;
  Int iDecStride = pcPicDec->getCStride();

  Pel* pRest;
  Int iRestStride = pcPicRest->getCStride();

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  if (iColor)
  {
    pDec = pcPicDec->getCrAddr();
    pRest = pcPicRest->getCrAddr();
  }
  else
  {
    pDec = pcPicDec->getCbAddr();
    pRest = pcPicRest->getCbAddr();
  }

  Pel* pTmpDec1, *pTmpDec2;
  Pel* pTmpPixSum;

  switch(iTap)
  {
  case 5:
    {
      Int iJump = iDecStride - 4;
      pDec -= iDecStride*2;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-2;
          pTmpDec2 = pTmpDec1+4+(4*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 7:
    {
      Int iJump = iDecStride - 6;
      pDec -= iDecStride*3;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-3;
          pTmpDec2 = pTmpDec1+6+(6*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 9:
    {
      Int iJump = iDecStride - 8;
      pDec -= iDecStride*4;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-4;
          pTmpDec2 = pTmpDec1+8+(8*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;


          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum =(*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  default:
    assert(0);
    break;
  }
}
