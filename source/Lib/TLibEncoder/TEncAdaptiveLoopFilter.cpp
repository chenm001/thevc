/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
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

/** \file     TEncAdaptiveLoopFilter.cpp
    \brief    estimation part of adaptive loop filter class
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

#if HHI_ALF
// ====================================================================================================================
// Constructor / destructor
// ====================================================================================================================

TEncAdaptiveLoopFilter::TEncAdaptiveLoopFilter()
{
  m_ppdAlfCorr = NULL;
  m_pdDoubleAlfCoeff = NULL;
  m_puiCUHorizontalCorr = NULL;
  m_puiCUVerticalCorr = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  m_pcBestAlfParam = NULL;
  m_pcTempAlfParam = NULL;
  m_pcPicYuvBest = NULL;
  m_pcPicYuvTmp = NULL;
  m_dMinCost = MAX_DOUBLE ;
  m_pcCoderFixedBits = NULL;
  m_bUseSBACRD = false;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  pcPic           picture (TComPic) pointer
    \param  pcEntropyCoder  entropy coder class
 */
Void TEncAdaptiveLoopFilter::startALFEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder , TEncSbac*** pppcRDSbacCoder, TEncSbac* pcRDGoOnSbacCoder )
{
  m_pcPic = pcPic;
  m_pcEntropyCoder = pcEntropyCoder;
  m_pcCoderFixedBits = new TComBitCounter;
  m_pcEntropyCoder->setBitstream( m_pcCoderFixedBits );
  m_pppcRDSbacCoder = pppcRDSbacCoder;
  m_pcRDGoOnSbacCoder = pcRDGoOnSbacCoder;
  if( pcRDGoOnSbacCoder )
    m_bUseSBACRD = true;

  m_bALFSeparateQt = pcPic->getSlice()->getSPS()->getALFSeparateQt();
  m_bALFSymmetry   = pcPic->getSlice()->getSPS()->getALFSymmetry();
  m_iALFMinLength  = pcPic->getSlice()->getSPS()->getALFMinLength();
  m_iALFMaxLength  = pcPic->getSlice()->getSPS()->getALFMaxLength();

  assert( m_iALFMinLength >= ALF_MIN_LENGTH       );
  assert( m_iALFMaxLength <= ALF_MAX_VERT_LENGTH  );
  assert( m_iALFMaxLength <= ALF_MAX_HORIZ_LENGTH );


  m_eSliceType = pcPic->getSlice()->getSliceType();
  m_iPicNalReferenceIdc = (pcPic->getSlice()->isReferenced() ? 1 :0);

  m_uiNumSCUInCU = m_pcPic->getNumPartInCU();
  m_uiSCUWidth = (m_pcPic->getMinCUWidth()<<1);
  m_uiSCUHeight = (m_pcPic->getMinCUHeight()<<1);

  xInitParam();
  Int iWidth  = pcPic->getPicYuvOrg()->getWidth();
  Int iHeight = pcPic->getPicYuvOrg()->getHeight();

  m_pcPicYuvTmp = new TComPicYuv();
  m_pcPicYuvTmp->create(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);

  m_pcPicYuvFiltered = new TComPicYuv();
  m_pcPicYuvFiltered->create(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);

  m_pcPicYuvBest = pcPic->getPicYuvPred();

  m_pcBestAlfParam = new ALFParam;
  m_pcTempAlfParam = new ALFParam;
  allocALFParam(m_pcBestAlfParam);
  allocALFParam(m_pcTempAlfParam);
}


Void TEncAdaptiveLoopFilter::endALFEnc()
{
  xUninitParam();

  m_pcPicYuvFiltered->destroy() ;
  delete m_pcPicYuvFiltered ;

  m_pcPicYuvTmp->destroy();
  delete m_pcPicYuvTmp ;
//  m_pcPicYuvTmp = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  delete m_pcCoderFixedBits;

  freeALFParam(m_pcBestAlfParam);
  freeALFParam(m_pcTempAlfParam);
  delete m_pcBestAlfParam ;
  delete m_pcTempAlfParam;
}

/** \param  pcAlfParam          ALF parameter
    \param  dLambda             lambda value for RD cost computation
    \retval ruiDist             distortion
    \retval ruiBits             required bits
    \retval ruiMaxAlfCtrlDepth  optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth )
{
  // set lambda
  m_dLambdaLuma   = dLambda;
  m_dLambdaChroma = dLambda;

  TComPicYuv* pcPicOrg             = m_pcPic->getPicYuvOrg();
  TComPicYuv* pcPicYuvRec          = m_pcPic->getPicYuvRec();

  TComPicYuv* pcPicYuvFiltered = new TComPicYuv ;
  pcPicYuvFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth ) ;

  // set min cost
  UInt64 uiMinRate = MAX_INT;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;

  UInt64  uiOrigRate;
  UInt64  uiOrigDist;
  Double  dOrigCost;

  // calc original cost
  xCalcRDCost( pcPicOrg, pcPicYuvRec, NULL, uiOrigRate, uiOrigDist, dOrigCost,0 );

  Int iInitSymmetry = m_bALFSymmetry ? 1 : 0;
  pcAlfParam->chroma_idc      = 0;
  pcAlfParam->cu_control_flag = 0;
  pcAlfParam->alf_flag        = 0;
  pcAlfParam->bSeparateQt      = m_bALFSeparateQt;
  xFillAlfFilterInitParam( pcAlfParam->acHorizontalAlfFilter[0], ALF_MAX_HORIZ_LENGTH, iInitSymmetry  );
  xFillAlfFilterInitParam( pcAlfParam->acVerticalAlfFilter  [0], ALF_MAX_VERT_LENGTH , iInitSymmetry  );
  for(Int i = 0; i<3; i++)
    pcAlfParam->aiPlaneFilterMapping[i] = i ;

  // adaptive in-loop wiener filtering
  Int iPlane = 0 ;

  // adaptive filter-lengths
  xFilterTapDecision( pcPicOrg, pcPicYuvRec, pcPicYuvFiltered, pcAlfParam, uiMinRate, uiMinDist, dMinCost, iPlane)  ;

  xCUAdaptiveControl( pcPicOrg, pcPicYuvRec, pcPicYuvFiltered, pcAlfParam, uiMinRate, uiMinDist, dMinCost ) ;


  if( dMinCost < dOrigCost )
  {
    pcPicYuvFiltered->copyToPicLuma( pcPicYuvRec );
  }
  // if ALF works
  if( pcAlfParam->alf_flag )
  {
    if( ALF_FILT_FOR_CHROMA ==2 )
    {
      for (iPlane=1;iPlane<3; iPlane++)
      {
        uiMinRate = MAX_INT;
        uiMinDist = MAX_INT;
        dMinCost  = MAX_DOUBLE;

        // calc original cost
        xCalcRDCost( pcPicOrg, pcPicYuvRec, pcAlfParam, uiOrigRate, uiOrigDist, dOrigCost, iPlane );

        xFillAlfFilterInitParam( pcAlfParam->acHorizontalAlfFilter[iPlane], ALF_MAX_HORIZ_LENGTH, iInitSymmetry );
        xFillAlfFilterInitParam( pcAlfParam->acVerticalAlfFilter  [iPlane], ALF_MAX_VERT_LENGTH , iInitSymmetry );
        xFilterTapDecision(pcPicOrg, pcPicYuvRec, pcPicYuvFiltered, pcAlfParam, uiMinRate, uiMinDist, dMinCost, iPlane)  ;
        xCheckFilterReuse(pcPicOrg, pcPicYuvRec, pcPicYuvFiltered, pcAlfParam, dMinCost, iPlane) ;

        if( dMinCost < dOrigCost )
        {
          if (iPlane == 1)
            pcPicYuvFiltered->copyToPicCb( pcPicYuvRec );
          else if (iPlane == 2)
            pcPicYuvFiltered->copyToPicCr( pcPicYuvRec );
        }
      }
    }
    // predict ALF coefficients
   xPredictALFCoeff( pcAlfParam );
  }

  // copy to best storage
  // store best depth
  ruiMaxAlfCtrlDepth = m_pcEntropyCoder->getMaxAlfCtrlDepth();
  pcPicYuvFiltered->destroy() ;
  delete pcPicYuvFiltered ;
}



// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
Void TEncAdaptiveLoopFilter::xPredictALFCoeff(ALFParam *pALFParam)
{
  for(Int i=0; i<ALF_FILT_FOR_CHROMA+1; i++)
  {
    if( (pALFParam->chroma_idc & i) || (i==0 ) )
    {
        xPredictALFCoeff(pALFParam, i);
    }
  }
}


Void TEncAdaptiveLoopFilter::xPredictALFCoeff(ALFParam *pALFParam, Int iPlane)
{
  Int iCenterPos = 0;
  Int iLength;
// horizontal
  iLength = pALFParam->acHorizontalAlfFilter[iPlane].iFilterLength;
  iCenterPos =  iLength >>1;

  Int iSum = 0 ;
  for (Int i = 0; i< pALFParam->acHorizontalAlfFilter[iPlane].iFilterLength; i++)
  {
    if(i!=iCenterPos)
      iSum += pALFParam->acHorizontalAlfFilter[iPlane].aiQuantFilterCoeffs[ pALFParam->acHorizontalAlfFilter[iPlane].aiTapCoeffMapping[i] ] ;
  }
  pALFParam->acHorizontalAlfFilter[iPlane].aiQuantFilterCoeffs[iCenterPos]-= ((1<<ALF_NUM_BIT_SHIFT) - iSum) ;
  if(pALFParam->acHorizontalAlfFilter[iPlane].iFilterSymmetry == 0)
  {
    for(Int j=0 ; j < iCenterPos  ; j++ )
        pALFParam->acHorizontalAlfFilter[iPlane].aiQuantFilterCoeffs[iLength-j-1] -= pALFParam->acHorizontalAlfFilter[iPlane].aiQuantFilterCoeffs[j];
  }
// vertical
  iLength = pALFParam->acVerticalAlfFilter[iPlane].iFilterLength;
  iCenterPos = iLength >> 1;

  iSum = 0 ;
  for (Int i = 0; i< pALFParam->acVerticalAlfFilter[iPlane].iFilterLength; i++)
  {
    if(i!=iCenterPos)
      iSum += pALFParam->acVerticalAlfFilter[iPlane].aiQuantFilterCoeffs[ pALFParam->acVerticalAlfFilter[iPlane].aiTapCoeffMapping[i] ] ;
  }
  pALFParam->acVerticalAlfFilter[iPlane].aiQuantFilterCoeffs[iCenterPos]-= ((1<<ALF_NUM_BIT_SHIFT) - iSum) ;

  if(pALFParam->acVerticalAlfFilter[iPlane].iFilterSymmetry == 0)
  {
    for(Int j=0 ; j < iCenterPos  ; j++ )
      pALFParam->acVerticalAlfFilter[iPlane].aiQuantFilterCoeffs[iLength-j-1] -= pALFParam->acVerticalAlfFilter[iPlane].aiQuantFilterCoeffs[j];
  }
}

Void TEncAdaptiveLoopFilter::xCheckFilterReuse(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicFiltered, ALFParam* pcFilterParam, Double& rdMinCost, Int iPlane)
{
  UInt64 uiRate   ;
  UInt64 uiDist   ;
  Double dCmpCost   = rdMinCost;
  
  TComPicYuv* pcTmpPicVerticallyFiltered = new TComPicYuv;
  TComPicYuv* pcTmpPicHorizontallyFiltered = new TComPicYuv;

  pcTmpPicVerticallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth) ;
  pcTmpPicHorizontallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth) ;

  ALFParam* pcTmpAlfParams = new ALFParam ;
  allocALFParam( pcTmpAlfParams );

  for(Int iPrevPlane=0; iPrevPlane<iPlane; iPrevPlane++)
  {
    if ( ( pcFilterParam->chroma_idc & iPrevPlane) || (iPrevPlane==0) )
    {
      // set filters of iPlane to filters of iPrevPlane in m_pcTempAlfParam
      copyALFParam( pcTmpAlfParams, pcFilterParam) ;
      pcTmpAlfParams->aiPlaneFilterMapping[iPlane] = iPrevPlane ;

      if( !( pcTmpAlfParams->chroma_idc & iPlane ))
        pcTmpAlfParams->chroma_idc += iPlane ;

      copyALFFilter(pcTmpAlfParams->acHorizontalAlfFilter[iPlane], pcFilterParam->acHorizontalAlfFilter[iPrevPlane]);
      copyALFFilter(pcTmpAlfParams->acVerticalAlfFilter[iPlane], pcFilterParam->acVerticalAlfFilter[iPrevPlane]);

      // check for every previous coded plane if the recent plane can be coded with a previous filter
      xApplyFrame( pcPicDec, pcTmpPicVerticallyFiltered, pcTmpAlfParams->acVerticalAlfFilter[iPlane], iPlane);
      xApplyFrame( pcTmpPicVerticallyFiltered , pcTmpPicHorizontallyFiltered, pcTmpAlfParams->acHorizontalAlfFilter[iPlane], iPlane);

      // check new rdcosts
      xCalcRDCost( pcPicOrg, pcTmpPicHorizontallyFiltered, pcTmpAlfParams, uiRate, uiDist, dCmpCost, iPlane );

      if(dCmpCost < rdMinCost )
      {
        rdMinCost = dCmpCost;
        copyALFParam(pcFilterParam, pcTmpAlfParams);
        if(iPlane == 1)
          pcTmpPicHorizontallyFiltered->copyToPicCb(pcPicFiltered);
        else if(iPlane  == 2)
          pcTmpPicHorizontallyFiltered->copyToPicCr(pcPicFiltered);
      }
    }
  }
  pcTmpPicVerticallyFiltered->destroy() ;
  pcTmpPicHorizontallyFiltered->destroy() ;
  delete pcTmpPicVerticallyFiltered ;
  delete pcTmpPicHorizontallyFiltered ;
  freeALFParam( pcTmpAlfParams );
  delete pcTmpAlfParams ;
}

Void TEncAdaptiveLoopFilter::xFilterTapDecision(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicFiltered, ALFParam* pcAlfParams, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost, Int iPlane)
{
  // restriction for non-referenced B-slice
  // if (m_eSliceType == B_SLICE && m_iPicNalReferenceIdc == 0)
  // {
  //   return;
  // }
  UInt64 uiTmpRate, uiTmpDist;
  Double dTmpCost;
  TComPicYuv* pcTmpPicVerticallyFiltered = new TComPicYuv;
  TComPicYuv* pcTmpPicHorizontallyFiltered = new TComPicYuv;
  ALFParam* pcTmpAlfParams = new ALFParam ;
  allocALFParam( pcTmpAlfParams );


  xCalcRDCost(pcPicOrg, pcPicDec, NULL , ruiMinRate, ruiMinDist, rdMinCost, iPlane );

  pcTmpPicVerticallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth) ;
  pcTmpPicHorizontallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth) ;

  pcPicDec->copyToPic( pcPicFiltered ) ;
  copyALFParam( pcTmpAlfParams, pcAlfParams );
  uiTmpRate = ruiMinRate ;
  uiTmpDist = ruiMinDist ;
  dTmpCost  = rdMinCost ;
  pcTmpAlfParams->alf_flag = 1 ;
  pcTmpAlfParams->chroma_idc += iPlane ;

  for (Int iLengthVert = m_iALFMaxLength ; iLengthVert >= m_iALFMinLength ; iLengthVert -= 2 )
  {
    xFillAlfFilterInitParam( pcTmpAlfParams->acVerticalAlfFilter[iPlane] , iLengthVert , pcTmpAlfParams->acVerticalAlfFilter[iPlane].iFilterSymmetry );
    xEstimateFrameFilter   ( pcPicOrg, pcPicDec      , pcTmpAlfParams->acVerticalAlfFilter[iPlane], false , m_puiCUVerticalCorr, iPlane   );
    xApplyFrame            ( pcPicDec, pcTmpPicVerticallyFiltered, pcTmpAlfParams->acVerticalAlfFilter[iPlane], iPlane );

    for (Int iLengthHoriz = m_iALFMaxLength ; iLengthHoriz >= m_iALFMinLength ; iLengthHoriz -= 2 )
    {
      xFillAlfFilterInitParam( pcTmpAlfParams->acHorizontalAlfFilter[iPlane] , iLengthHoriz , pcTmpAlfParams->acHorizontalAlfFilter[iPlane].iFilterSymmetry );
      xEstimateFrameFilter   ( pcPicOrg,        pcTmpPicVerticallyFiltered, pcTmpAlfParams->acHorizontalAlfFilter[iPlane], false , NULL, iPlane   );
      xApplyFrame            ( pcTmpPicVerticallyFiltered , pcTmpPicHorizontallyFiltered , pcTmpAlfParams->acHorizontalAlfFilter[iPlane], iPlane );

      xCalcRDCost(pcPicOrg, pcTmpPicHorizontallyFiltered, pcTmpAlfParams , uiTmpRate, uiTmpDist, dTmpCost, iPlane );
      if (dTmpCost < rdMinCost)
      {
        rdMinCost = dTmpCost;
        ruiMinDist = uiTmpDist;
        ruiMinRate = uiTmpRate;
        if (iPlane == 0)
          pcTmpPicHorizontallyFiltered->copyToPicLuma( pcPicFiltered   );
        else if (iPlane == 1)
          pcTmpPicHorizontallyFiltered->copyToPicCb  ( pcPicFiltered   );
        else if (iPlane == 2)
          pcTmpPicHorizontallyFiltered->copyToPicCr  ( pcPicFiltered   );
        copyALFParam( pcAlfParams, pcTmpAlfParams );
      }
    }
  }
  freeALFParam( pcTmpAlfParams );
  pcTmpPicHorizontallyFiltered->destroy();
  pcTmpPicVerticallyFiltered->destroy();
  delete pcTmpAlfParams;
  delete pcTmpPicHorizontallyFiltered ;
  delete pcTmpPicVerticallyFiltered ;

}


Void TEncAdaptiveLoopFilter::xCUAdaptiveControl( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicFiltered, ALFParam* pcAlfParam, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost )
{
  m_pcEntropyCoder->setAlfCtrl(true);
  UInt uiBestDepth = 0;

  ALFParam* pcTmpAlfParam = new ALFParam ;
  allocALFParam(pcTmpAlfParam);
  copyALFParam(pcTmpAlfParam, pcAlfParam);

  ALFParam* pcFrmAlfParam = new ALFParam ;
  allocALFParam(pcFrmAlfParam);
  copyALFParam(pcFrmAlfParam, pcAlfParam);

  TComPicYuv* pcFilteredFrm = new TComPicYuv();
  pcFilteredFrm->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  pcPicFiltered->copyToPic( pcFilteredFrm );

  TComPicYuv* pcTmpPicVerticallyFiltered = new TComPicYuv;
  pcTmpPicVerticallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  TComPicYuv* pcTmpPicHorizontallyFiltered = new TComPicYuv;
  pcTmpPicHorizontallyFiltered->create(pcPicOrg->getWidth(), pcPicOrg->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);

  TComPicSym* pcQuadTreeBest;
  TComPicSym* pcQuadTreeTmp;
  TComPicSym* pcQuadTreeHelp;
  if( m_bALFSeparateQt )
  {
    pcQuadTreeBest = new TComPicSym();
    pcQuadTreeBest->create( pcPicDec->getWidth(), pcPicDec->getHeight(),  g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
    pcQuadTreeTmp = new TComPicSym();
    pcQuadTreeTmp->create( pcPicDec->getWidth(), pcPicDec->getHeight(),  g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
    for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTreeBest->getNumberOfCUsInFrame(); uiCUAddr++ )
    {
      TComDataCU* pcCU = pcQuadTreeBest->getCU( uiCUAddr );
      pcCU->initCU( m_pcPic , uiCUAddr );
      pcCU = pcQuadTreeTmp->getCU( uiCUAddr );
      pcCU->initCU( m_pcPic , uiCUAddr );
    }
  }
  else
  {
    pcQuadTreeBest = m_pcPic->getPicSym();
    pcQuadTreeTmp  = m_pcPic->getPicSym();
    xCreateTmpAlfCtrlFlags( pcQuadTreeBest );
  }
  pcQuadTreeHelp = pcQuadTreeTmp;
  bool bUsedRedesign = false;
  for( UInt uiDepth = 0; uiDepth < g_uiMaxCUDepth; uiDepth++ )
  {
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
    copyALFParam( pcTmpAlfParam , pcFrmAlfParam );
    pcTmpAlfParam->cu_control_flag = 1;

    for( UInt uiRD = 0; uiRD <= ALF_NUM_OF_REDESIGN; uiRD++ )
    {
      UInt64 uiRate, uiDist, uiEstRate;
      Double dCost;
      if( uiRD )
      {
        // re-design filter coefficients
        xReDesignFilterCoeff( pcQuadTreeHelp, pcPicOrg , pcPicDec, pcTmpAlfParam->acVerticalAlfFilter[0] , m_puiCUVerticalCorr, false );
        xApplyFrame( pcPicDec , pcTmpPicVerticallyFiltered , pcTmpAlfParam->acVerticalAlfFilter  [0]);
        xReDesignFilterCoeff( pcQuadTreeHelp, pcPicOrg , pcTmpPicVerticallyFiltered, pcTmpAlfParam->acHorizontalAlfFilter[0] , m_puiCUHorizontalCorr, false);
        xApplyFrame( pcTmpPicVerticallyFiltered , pcTmpPicHorizontallyFiltered , pcTmpAlfParam->acHorizontalAlfFilter[0] );
        //xCUAdaptive( m_pcPic, cFrmAlfParam.acHorizontalAlfFilter[0], m_pcPicYuvTmp, pcPicRest);
        xSetCUAlfCtrlFlags(uiDepth, pcPicOrg, pcPicDec, pcTmpPicHorizontallyFiltered, pcQuadTreeTmp, uiDist, uiEstRate);
      }
      else
      {
        xSetCUAlfCtrlFlags(uiDepth, pcPicOrg, pcPicDec, pcFilteredFrm, pcQuadTreeTmp, uiDist, uiEstRate);
      }

      pcTmpAlfParam->pcQuadTree = pcQuadTreeTmp;
      xCalcRDCost( pcTmpAlfParam , uiRate, uiDist, dCost );

      pcQuadTreeHelp = pcQuadTreeTmp;
      if( dCost < rdMinCost )
      {
        pcQuadTreeHelp = pcQuadTreeBest;
        pcQuadTreeBest = pcQuadTreeTmp;
        pcQuadTreeTmp  = pcQuadTreeHelp;
        pcQuadTreeHelp = pcQuadTreeBest;
        uiBestDepth = uiDepth;
        rdMinCost   = dCost;
        ruiMinDist  = uiDist;
        ruiMinRate  = uiRate;
        bUsedRedesign = uiRD ? true : false ;
        if( uiRD )
        {
          pcTmpPicHorizontallyFiltered->copyToPicLuma( pcPicFiltered );
        }
        copyALFParam( pcAlfParam , pcTmpAlfParam );
        if( !m_bALFSeparateQt )
          xCopyTmpAlfCtrlFlagsFrom( pcQuadTreeBest );
      }
    }
  }

  if (pcAlfParam->cu_control_flag)
  {
    m_pcEntropyCoder->setAlfCtrl(true);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiBestDepth);
    if( !m_bALFSeparateQt)
    {
      xCopyTmpAlfCtrlFlagsTo( pcQuadTreeBest );
    }
    if( !bUsedRedesign )
    {
      pcFilteredFrm->copyToPicLuma( pcPicFiltered );
    }
    xCopyDecToRestCUs( pcQuadTreeBest , pcPicDec , pcPicFiltered  );
  }
  else
  {
    m_pcEntropyCoder->setAlfCtrl(false);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(0);
    if( m_bALFSeparateQt )
    {
      pcQuadTreeBest->destroy();
      delete pcQuadTreeBest;
    }
  }

  if( m_bALFSeparateQt)
  {
    pcQuadTreeTmp->destroy();
    delete pcQuadTreeTmp;
  }
  else
  {
    xDestroyTmpAlfCtrlFlags( pcQuadTreeBest );
  }

  pcFilteredFrm->destroy();
  delete pcFilteredFrm;

  //
  freeALFParam(pcTmpAlfParam);
  delete pcTmpAlfParam ;
  freeALFParam(pcFrmAlfParam);
  delete pcFrmAlfParam ;

  pcTmpPicVerticallyFiltered->destroy() ;
  delete pcTmpPicVerticallyFiltered ;
  pcTmpPicHorizontallyFiltered->destroy() ;
  delete pcTmpPicHorizontallyFiltered ;

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

  if (m_puiCUVerticalCorr != NULL)
  {
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        for (k = 0; k < ALF_MAX_NUM_COEF; k++)
        {
          for (l = 0; l< ALF_MAX_NUM_COEF+1; l++)
          {
            m_puiCUVerticalCorr[i][j][k][l] = 0;
          }
        }
      }
    }
  }
  else
  {
    m_puiCUVerticalCorr = new UInt***[m_pcPic->getNumCUsInFrame()];
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      m_puiCUVerticalCorr[i] = new UInt**[m_uiNumSCUInCU];
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        m_puiCUVerticalCorr[i][j] = new UInt*[ALF_MAX_NUM_COEF];
        for (k = 0; k < ALF_MAX_NUM_COEF; k++)
        {
          m_puiCUVerticalCorr[i][j][k] = new UInt[ALF_MAX_NUM_COEF+1];
          for (l = 0; l< ALF_MAX_NUM_COEF+1; l++)
          {
            m_puiCUVerticalCorr[i][j][k][l] = 0;
          }
        }
      }
    }
  }


  if (m_puiCUHorizontalCorr != NULL)
  {
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        for (k = 0; k < ALF_MAX_NUM_COEF; k++)
        {
          for (l = 0; l< ALF_MAX_NUM_COEF+1; l++)
          {
            m_puiCUHorizontalCorr[i][j][k][l] = 0;
          }
        }
      }
    }
  }
  else
  {
    m_puiCUHorizontalCorr = new UInt***[m_pcPic->getNumCUsInFrame()];
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      m_puiCUHorizontalCorr[i] = new UInt**[m_uiNumSCUInCU];
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        m_puiCUHorizontalCorr[i][j] = new UInt*[ALF_MAX_NUM_COEF];
        for (k = 0; k < ALF_MAX_NUM_COEF; k++)
        {
          m_puiCUHorizontalCorr[i][j][k] = new UInt[ALF_MAX_NUM_COEF+1];
          for (l = 0; l< ALF_MAX_NUM_COEF+1; l++)
          {
            m_puiCUHorizontalCorr[i][j][k][l] = 0;
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

  if (m_puiCUVerticalCorr != NULL)
  {
    for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
    {
      for (j = 0; j < m_uiNumSCUInCU; j++)
      {
        for (k = 0; k < ALF_MAX_NUM_COEF; k++)
        {
          delete[] m_puiCUVerticalCorr[i][j][k];
          m_puiCUVerticalCorr[i][j][k] = NULL;
        }
        delete[] m_puiCUVerticalCorr[i][j];
        m_puiCUVerticalCorr[i][j] = NULL;
      }
      delete[] m_puiCUVerticalCorr[i];
      m_puiCUVerticalCorr[i] = NULL;
    }
    delete[] m_puiCUVerticalCorr;
    m_puiCUVerticalCorr = NULL;
  }

  if (m_puiCUHorizontalCorr != NULL)
    {
      for (i = 0; i < m_pcPic->getNumCUsInFrame(); i++)
      {
        for (j = 0; j < m_uiNumSCUInCU; j++)
        {
          for (k = 0; k < ALF_MAX_NUM_COEF; k++)
          {
            delete[] m_puiCUHorizontalCorr[i][j][k];
            m_puiCUHorizontalCorr[i][j][k] = NULL;
          }
          delete[] m_puiCUHorizontalCorr[i][j];
          m_puiCUHorizontalCorr[i][j] = NULL;
        }
        delete[] m_puiCUHorizontalCorr[i];
        m_puiCUHorizontalCorr[i] = NULL;
      }
      delete[] m_puiCUHorizontalCorr;
      m_puiCUHorizontalCorr = NULL;
    }


  if (m_pdDoubleAlfCoeff != NULL)
  {
    delete[] m_pdDoubleAlfCoeff;
    m_pdDoubleAlfCoeff = NULL;
  }
}

Void TEncAdaptiveLoopFilter::xCreateTmpAlfCtrlFlags( TComPicSym* pcQuadTree )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    pcCU->createTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xDestroyTmpAlfCtrlFlags( TComPicSym* pcQuadTree )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    pcCU->destroyTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsTo( TComPicSym* pcQuadTree )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagFromTmp();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsFrom( TComPicSym* pcQuadTree )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagToTmp();
  }
}

Void TEncAdaptiveLoopFilter::xReadOrCalcCorrFromCUs(TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr )
{
  Int iNumCoeffs = rcFilter.iNumOfCoeffs;

  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
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
#if ALF_DC_CONSIDERED
          for(Int j=0 ; j < iNumCoeffs + 1; j++ )
          {
            for(Int k = j ; k < iNumCoeffs + 1 ; k++ )
            {
              m_ppdAlfCorr[k][j] += (Double) ppdAlfCorr[uiCUAddr][(uiIdx>>2)][k][j];
            }
            m_ppdAlfCorr[j][iNumCoeffs + 1 ] += ( Double ) ppdAlfCorr[uiCUAddr][(uiIdx>>2)][j][ iNumCoeffs + 1];
          }
#else
          for(Int j=0 ; j < iNumCoeffs; j++ )
          {
            for(Int k = j ; k < iNumCoeffs; k++ )
            {
              m_ppdAlfCorr[k][j] += (Double) ppdAlfCorr[uiCUAddr][(uiIdx>>2)][k][j];
            }
          }
#endif
        }
        else
        {
          xEstimateCorrCU( pcPicOrg , pcPicDec, pcCU, uiIdx, *pcCU->getDepth(), rcFilter, m_ppdAlfCorr );
        }
      }
    }
  }
}


Void TEncAdaptiveLoopFilter::xReadOrCalcCorrFromFUs(TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr )
{

  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    xReadOrCalcCorrFromFU( pcQuadTree, pcCU, 0, 0, pcPicOrg, pcPicDec, bReadCorr, rcFilter, ppdAlfCorr );
  }
}

Void TEncAdaptiveLoopFilter::xReadOrCalcCorrFromFU(TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr )
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
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xReadOrCalcCorrFromFU( pcQuadTree, pcCU, uiAbsPartIdx, uiDepth + 1, pcPicOrg, pcPicDec,bReadCorr, rcFilter, ppdAlfCorr );
    }
    return;
  }

  if ( pcCU->getAlfCtrlFlag( uiAbsPartIdx ) )
  {
    if (bReadCorr)
    {
      assert(0);
    }
    else
    {
      xEstimateCorrFU( pcPicOrg , pcPicDec, pcCU, uiAbsPartIdx, uiDepth, rcFilter, m_ppdAlfCorr );
    }
  }
}



Void TEncAdaptiveLoopFilter::encodeQuadTree( ALFParam* pAlfParam, TEncEntropy* pcEntropyCoder, UInt uiMaxAlfCtrlDepth )
{
  encodeFUAlfCtrlFlags( pcEntropyCoder, pAlfParam->pcQuadTree, uiMaxAlfCtrlDepth);
}

Void TEncAdaptiveLoopFilter::encodeFUAlfCtrlFlags(TEncEntropy* pcEntropyCoder, TComPicSym* pcQuadTree , UInt uiMaxDepth )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    encodeFUAlfCtrlFlag( pcEntropyCoder, pcQuadTree, pcCU, 0, 0 , uiMaxDepth);
  }
}

Void TEncAdaptiveLoopFilter::encodeFUAlfCtrlFlag(TEncEntropy* pcEntropyCoder, TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth )
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
  if( m_bALFSeparateQt )
  {
   pcEntropyCoder->encodeAlfQTSplitFlag( pcCU , uiAbsPartIdx, uiDepth, uiMaxDepth );
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || ( bBoundary && !m_bALFSeparateQt) )
  {
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        encodeFUAlfCtrlFlag(pcEntropyCoder, pcQuadTree, pcCU, uiAbsPartIdx, uiDepth+1 , uiMaxDepth );
    }
    return;
  }
  pcEntropyCoder->encodeAlfCtrlFlag(pcCU, uiAbsPartIdx, false, m_bALFSeparateQt );

}

Void TEncAdaptiveLoopFilter::xCalcALFCoeff( AlfFilter& rcFilter )
{
  Int iErrCode;
  Int iNumCoeffs  = rcFilter.iNumOfCoeffs;

  Double* h             = m_pdDoubleAlfCoeff;
  iErrCode = xGauss( m_ppdAlfCorr , iNumCoeffs );

  if(iErrCode)
  {
    xClearFilterCoefInt( rcFilter );
  }
  else
  {
    for(Int i = 0; i < iNumCoeffs ; i++)
      h[i] = m_ppdAlfCorr[i][iNumCoeffs];
    xQuantFilterCoef(h, rcFilter, g_uiBitDepth + g_uiBitIncrement);
  }
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

Void TEncAdaptiveLoopFilter::xQuantFilterCoef( Double* adDoubleCoeffs, AlfFilter& rcFilter, Int iBit_depth )
{
  Int i;
  const Int iNumCoeffs       = rcFilter.iNumOfCoeffs;
  Int iMaxValue = ( 1 << ( 1 + ALF_NUM_BIT_SHIFT ) ) - 1;
  Int iMinValue = 0 - ( 1 << ( 1 + ALF_NUM_BIT_SHIFT ) );

  Double dGainNotQuant;
  Int    iGainNotQuant;
  Int    iGainQuant;

  Int* aiQuantCoeffs = rcFilter.aiQuantFilterCoeffs;

  Double* adDelta   = new Double[ iNumCoeffs+1 ];
  Int*    aiPosIndx = new Int   [ iNumCoeffs+1 ];


  dGainNotQuant = 0.0;
  iGainQuant    = 0;

  for(i=0 ; i < iNumCoeffs ; i++)
  {
    if( adDoubleCoeffs[i] >= 0.0 )
      aiQuantCoeffs[i] =  (Int)( adDoubleCoeffs[i] * ( 1 << ALF_NUM_BIT_SHIFT ) +0.5 );
    else
      aiQuantCoeffs[i] = -(Int)(-adDoubleCoeffs[i] * ( 1 << ALF_NUM_BIT_SHIFT ) +0.5 );

    adDelta[i]  = (Double) aiQuantCoeffs[i] / (Double) (1<<ALF_NUM_BIT_SHIFT) - adDoubleCoeffs[i];
    adDelta[i] *= rcFilter.aiCoeffWeights[i];

    dGainNotQuant += adDoubleCoeffs[i] * rcFilter.aiCoeffWeights[i];
    iGainQuant    += aiQuantCoeffs [i] * rcFilter.aiCoeffWeights[i];
    aiPosIndx[i]   = i;
  }
#if ALF_DC_CONSIDERED
  // quant dc offset
  if( adDoubleCoeffs[iNumCoeffs] >= 0.0 )
    aiQuantCoeffs[iNumCoeffs] =  (Int)( adDoubleCoeffs[iNumCoeffs] * ( 1 << ALF_NUM_BIT_SHIFT ) +0.5 );
  else
    aiQuantCoeffs[iNumCoeffs] = -(Int)(-adDoubleCoeffs[iNumCoeffs] * ( 1 << ALF_NUM_BIT_SHIFT ) +0.5 );
#endif

  iGainNotQuant = (Int)( dGainNotQuant * ( 1 << ALF_NUM_BIT_SHIFT ) + 0.5 );


 // modification of quantized filter coefficients
  Int iUpper, iLower;
  if( iGainQuant != iGainNotQuant )
  {
    xFilterCoefQuickSort( adDelta , aiPosIndx, 0, iNumCoeffs - 1 );
    if( iGainQuant > iGainNotQuant )
    {
      iUpper = iNumCoeffs - 1;
      while( iGainQuant > iGainNotQuant )
      {
        i = aiPosIndx[ iUpper % iNumCoeffs ];
        aiQuantCoeffs[ i ]--;
        iGainQuant -= rcFilter.aiCoeffWeights[i];
        iUpper--;
      }
    }
    else if( iGainQuant < iGainNotQuant )
    {
      iLower = 0;
      while( iGainQuant < iGainNotQuant )
      {
        i = aiPosIndx[ iLower % iNumCoeffs ];
        aiQuantCoeffs[ i ]++;
        iGainQuant += rcFilter.aiCoeffWeights[i];
        iLower++;
      }
    }
  }
  // set of filter coefficients
  for( i=0; i < iNumCoeffs ; i++ )
  {
    aiQuantCoeffs[i] = Max( iMinValue , Min( iMaxValue , aiQuantCoeffs[i] ) );
  }

  delete[] adDelta;
  adDelta = NULL;

  delete[] aiPosIndx;
  aiPosIndx = NULL;
}

Void TEncAdaptiveLoopFilter::xClearFilterCoefInt( AlfFilter& rcFilter )
{
  // clear
  memset( rcFilter.aiQuantFilterCoeffs , 0, sizeof( Int ) * ( rcFilter.iNumOfCoeffs + 1 ) );
  rcFilter.bIsValid = false;
}


Void TEncAdaptiveLoopFilter::xCalcRDCost(ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost )
{
  if(pAlfParam != NULL)
  {
    ALFParam* pcTempAlfParams = new ALFParam ;
    allocALFParam( pcTempAlfParams );
    copyALFParam(pcTempAlfParams, pAlfParam);
    xPredictALFCoeff(pcTempAlfParams, 0 );
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();

    if(pAlfParam->cu_control_flag)
    {
      encodeFUAlfCtrlFlags( m_pcEntropyCoder, pAlfParam->pcQuadTree , m_pcEntropyCoder->getMaxAlfCtrlDepth() );
    }

    m_pcEntropyCoder->encodeAlfParam( pcTempAlfParams );
    freeALFParam( pcTempAlfParams );
    delete pcTempAlfParams ;
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  else
  {
    ruiRate = 1;
  }
  rdCost = (Double)(ruiRate) * m_dLambdaLuma + (Double)(uiDist);
}

Void TEncAdaptiveLoopFilter::xCalcRDCost(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost, Int iPlane)
{
  if(pAlfParam != NULL)
  {
    ALFParam* pcTempAlfParams = new ALFParam ;
    allocALFParam( pcTempAlfParams );
    copyALFParam(pcTempAlfParams, pAlfParam);
    xPredictALFCoeff(pcTempAlfParams, iPlane );
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pcTempAlfParams);
    freeALFParam( pcTempAlfParams );
    delete pcTempAlfParams ;

    if(pAlfParam->cu_control_flag)
    {
      encodeFUAlfCtrlFlags( m_pcEntropyCoder, pAlfParam->pcQuadTree, m_pcEntropyCoder->getMaxAlfCtrlDepth() );
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  else
  {
    ruiRate = 1;
  }
  if (iPlane == 0)
    ruiDist     = xCalcSSD(pcPicOrg->getLumaAddr(), pcPicCmp->getLumaAddr(), pcPicOrg->getWidth(), pcPicOrg->getHeight(), pcPicOrg->getStride());
  else if (iPlane == 1)
    ruiDist     = xCalcSSD(pcPicOrg->getCbAddr(), pcPicCmp->getCbAddr(), pcPicOrg->getWidth()>>1, pcPicOrg->getHeight()>>1, pcPicOrg->getCStride());
  else if (iPlane == 2)
      ruiDist     = xCalcSSD(pcPicOrg->getCrAddr(), pcPicCmp->getCrAddr(), pcPicOrg->getWidth()>>1, pcPicOrg->getHeight()>>1, pcPicOrg->getCStride());

  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(ruiDist);
}




Void TEncAdaptiveLoopFilter::xEstimateFrameFilter( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, AlfFilter& rcFilter , Bool bStoreCorr , UInt**** puiCUCorr, Int iPlane )
{
  Int  iNumCoeffs   = rcFilter.iNumOfCoeffs;
#if ALF_DC_CONSIDERED
  for(Int i = 0 ; i < iNumCoeffs + 1 ; i++ )
  {
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*( iNumCoeffs + 2 ) );
  }
#else
  for(Int i = 0 ; i < iNumCoeffs ; i++ )
  {
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*( iNumCoeffs + 1 ) );
  }
#endif
  if( bStoreCorr && (iPlane ==0 ))
  {
    UInt** ppuiBlkCorr;
    // store correlation per minimum size cu
    for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
    {
      for( UInt uiIdx = 0; uiIdx < m_pcPic->getNumPartInCU() ; uiIdx += 4 )
      {
        TComDataCU* pcCU = m_pcPic->getCU(uiCUAddr);
        UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiIdx ] ];
        UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiIdx ] ];

        if (uiLPelX >= pcPicOrg->getWidth() || uiTPelY >= pcPicOrg->getHeight())
        {
          continue;
        }
        ppuiBlkCorr  = puiCUCorr[uiCUAddr][(uiIdx>>2)];
#if ALF_DC_CONSIDERED
        for(Int i = 0 ; i < iNumCoeffs ; i++ )
        {
          memset( ppuiBlkCorr[i], 0, sizeof(UInt)*( iNumCoeffs + 1 - i) );
        }

        xEstimateCorrCU( pcPicOrg, pcPicDec, pcCU, uiIdx, *pcCU->getDepth(), rcFilter, ppuiBlkCorr );
        for(Int j=0 ; j < iNumCoeffs + 1; j++ )
        {
          for(Int k = j ; k < iNumCoeffs + 1 ; k++ )
          {
            m_ppdAlfCorr[k][j] += (Double) ppuiBlkCorr[ k ][ j ];
          }
          m_ppdAlfCorr[j][iNumCoeffs + 1 ] += ( Double ) ppuiBlkCorr[ j ][ iNumCoeffs + 1 ];
        }
#else
        for(Int i = 0 ; i < iNumCoeffs ; i++ )
        {
          memset( ppuiBlkCorr[i], 0, sizeof(UInt)*( iNumCoeffs - i) );
        }

        xEstimateCorrCU( pcPicOrg, pcPicDec, pcCU, uiIdx, *pcCU->getDepth(), rcFilter, ppuiBlkCorr );
        for(Int j=0 ; j < iNumCoeffs ; j++ )
        {
          for(Int k = j ; k < iNumCoeffs  ; k++ )
          {
            m_ppdAlfCorr[k][j] += (Double) ppuiBlkCorr[ k ][ j ];
          }
        }
#endif
      }
    }
  }
  else
  {
    Pel* pOrg = NULL;
    Pel* pCmp = NULL;
    Int iWidth = 0;
    Int iHeight = 0;
    Int iOrgStride = 0;
    Int iDecStride = 0;
    if (iPlane == 0)
    {
      pOrg        = pcPicOrg->getLumaAddr();
      pCmp        = pcPicDec->getLumaAddr();
      iWidth      = pcPicOrg->getWidth() ;
      iHeight     = pcPicOrg->getHeight() ;
      iOrgStride  = pcPicOrg->getStride() ;
      iDecStride  = pcPicDec->getStride() ;
    }
    else if (iPlane == 1)
    {
      pOrg        = pcPicOrg->getCbAddr();
      pCmp        = pcPicDec->getCbAddr();
      iWidth      = pcPicOrg->getWidth()>>1 ;
      iHeight     = pcPicOrg->getHeight()>>1 ;
      iOrgStride  = pcPicOrg->getCStride() ;
      iDecStride  = pcPicDec->getCStride() ;
    }
    if (iPlane == 2)
    {
      pOrg        = pcPicOrg->getCrAddr();
      pCmp        = pcPicDec->getCrAddr();
      iWidth      = pcPicOrg->getWidth()>>1 ;
      iHeight     = pcPicOrg->getHeight()>>1 ;
      iOrgStride  = pcPicOrg->getCStride() ;
      iDecStride  = pcPicDec->getCStride() ;
    }

    xEstimateCorr( pOrg, pCmp, iWidth, iHeight, iOrgStride, iDecStride, rcFilter , m_ppdAlfCorr , 0 , 0 , iWidth, iHeight );
  }
#if ALF_DC_CONSIDERED
  for( Int j=0 ; j < iNumCoeffs ; j++ )
    {
      for( Int k=j+1; k< iNumCoeffs + 1 ; k++)
      {
        m_ppdAlfCorr[j][k] = m_ppdAlfCorr[k][j];
      }
    }
  Int iErr_code = xGauss(m_ppdAlfCorr, iNumCoeffs + 1);
#else
  for( Int j=0 ; j < iNumCoeffs ; j++ )
  {
    for( Int k=j+1; k< iNumCoeffs  ; k++)
    {
      m_ppdAlfCorr[j][k] = m_ppdAlfCorr[k][j];
    }
  }
  Int iErr_code = xGauss(m_ppdAlfCorr, iNumCoeffs );
#endif
  if(iErr_code)
  {
    xClearFilterCoefInt( rcFilter );
  }
  else
  {
#if ALF_DC_CONSIDERED
    for(Int i = 0; i < iNumCoeffs+1 ; i++)
    {
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][iNumCoeffs+1];
    }
#else
    for(Int i = 0; i < iNumCoeffs ; i++)
    {
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][iNumCoeffs];
    }
#endif
    xQuantFilterCoef( m_pdDoubleAlfCoeff, rcFilter, g_uiBitDepth + g_uiBitIncrement);
    rcFilter.bIsValid = true;
  }
}


Void TEncAdaptiveLoopFilter::xReDesignFilterCoeff( TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, AlfFilter& rcFilter, UInt**** ppuiAlfCorr, Bool bReadCorr)
{

#if ALF_DC_CONSIDERED
  Int N = rcFilter.iNumOfCoeffs + 1;
#else
  Int N = rcFilter.iNumOfCoeffs;
#endif
  // initialize correlation

  for(Int i=0; i<N; i++)
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(N+1));

  if( m_bALFSeparateQt )
    xReadOrCalcCorrFromFUs( pcQuadTree, pcPicOrg, pcPicDec, bReadCorr, rcFilter, ppuiAlfCorr );
  else
    xReadOrCalcCorrFromCUs( pcQuadTree, pcPicOrg, pcPicDec, bReadCorr, rcFilter, ppuiAlfCorr );


  for( Int j=0 ; j < N - 1 ; j++ )
  {
    for( Int k=j+1; k< N ; k++)
    {
      m_ppdAlfCorr[j][k] = m_ppdAlfCorr[k][j];
    }
  }

  Int err_code = xGauss(m_ppdAlfCorr, N);

  if(err_code)
  {
    xClearFilterCoefInt( rcFilter );
  }
  else
  {
    for(Int i=0; i<N; i++)
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][N];

    xQuantFilterCoef(m_pdDoubleAlfCoeff, rcFilter, g_uiBitDepth + g_uiBitIncrement);
  }
}

Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlags(UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest,TComPicSym* pcQuadTree, UInt64& ruiDist, UInt64& ruiRate)
{
  ruiDist = 0;
  ruiRate = 0;
  if( m_bALFSeparateQt )
  {
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[0][CI_NEXT_BEST]);
    }
    UInt64* auiFixedBitsCurrBest;
    UInt64* auiFixedBitsNextBest;
    auiFixedBitsCurrBest = new UInt64[MAX_CU_DEPTH];
    auiFixedBitsNextBest = new UInt64[MAX_CU_DEPTH];
    ::memset( auiFixedBitsCurrBest , 0 , sizeof(UInt64) * MAX_CU_DEPTH );
    ::memset( auiFixedBitsNextBest , 0 , sizeof(UInt64) * MAX_CU_DEPTH );
    for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
    {
      TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[0][CI_CURR_BEST]->load( m_pppcRDSbacCoder[0][CI_NEXT_BEST]);
      }
      auiFixedBitsCurrBest[0] = auiFixedBitsNextBest[0];
      xSetCUAlfCtrlFlag( pcQuadTree, pcCU, 0, 0, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist, auiFixedBitsCurrBest, auiFixedBitsNextBest );
    }
    m_pcCoderFixedBits->resetBits();
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[0][CI_NEXT_BEST] );
    }
    ruiRate = auiFixedBitsNextBest[0] + m_pcEntropyCoder->getNumberOfWrittenBits() ;
    delete[] auiFixedBitsCurrBest;
    delete[] auiFixedBitsNextBest;
  }
  else
  {
    for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
    {
      TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
      xSetCUAlfCtrlFlag(pcCU, 0, 0, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist);
    }
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

Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlag(TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiResultDist, UInt64* auiFixedBitsCurrBest, UInt64* auiFixedBitsNextBest )
{
 
  pcCU->setWidth (uiAbsPartIdx, g_uiMaxCUWidth  >> uiDepth );
  pcCU->setHeight(uiAbsPartIdx, g_uiMaxCUHeight >> uiDepth );
  pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  m_pcCoderFixedBits->resetBits();
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
  }
  m_pcEntropyCoder->encodeAlfQTSplitFlag( pcCU , uiAbsPartIdx , uiDepth, uiAlfCtrlDepth, false );
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST] );
  }
  UInt uiFixedBitsNoSplit = m_pcCoderFixedBits->getNumberOfWrittenBits() +  UInt( auiFixedBitsCurrBest[ uiDepth ] );


  UInt64 uiSplitDist = 0;
  UInt64 uiSplitRate = 0;
  Double dSplitCost  = 0;
  if( ( uiDepth < uiAlfCtrlDepth ) )
  {
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    UInt uiNextPartIdx = uiAbsPartIdx;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiNextPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiNextPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiNextPartIdx] ];

      if ( 0 == uiPartUnitIdx) //initialize RD with previous depth buffer
      {
        m_pcCoderFixedBits->resetBits();
        if( m_bUseSBACRD )
        {
          m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        pcCU->setDepthSubParts( uiDepth + 1, uiAbsPartIdx );
        m_pcEntropyCoder->encodeAlfQTSplitFlag( pcCU , uiAbsPartIdx , uiDepth, uiAlfCtrlDepth, false );
        if( m_bUseSBACRD )
        {
          m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth + 1][CI_CURR_BEST]);
        }
        auiFixedBitsCurrBest[ uiDepth + 1 ] = auiFixedBitsCurrBest[ uiDepth ] + m_pcCoderFixedBits->getNumberOfWrittenBits();
      }
      else
      {
        if( m_bUseSBACRD )
        {
          m_pppcRDSbacCoder[ uiDepth + 1 ][CI_CURR_BEST]->load(m_pppcRDSbacCoder[ uiDepth + 1 ][CI_NEXT_BEST]);
        }
        auiFixedBitsCurrBest[ uiDepth + 1 ] = auiFixedBitsNextBest[ uiDepth + 1 ];
      }

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
      {
        xSetCUAlfCtrlFlag(pcQuadTree, pcCU, uiNextPartIdx, uiDepth+1, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, uiSplitDist, auiFixedBitsCurrBest, auiFixedBitsNextBest  );
      }
    }
  }

  if( uiDepth > uiAlfCtrlDepth )
  {
    return;
  }

  Int iWidth  = g_uiMaxCUWidth  >> uiDepth;
  Int iHeight = g_uiMaxCUHeight >> uiDepth;
  uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() )
  {
    iWidth = pcCU->getSlice()->getSPS()->getWidth() - uiLPelX;
  }
  if( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() )
  {
    iHeight = pcCU->getSlice()->getSPS()->getHeight() - uiTPelY;
  }

  Pel* pOrg  = pcPicOrg->getLumaAddr ( pcCU->getAddr() , uiAbsPartIdx );
  Pel* pRec  = pcPicDec->getLumaAddr ( pcCU->getAddr() , uiAbsPartIdx );
  Pel* pFilt = pcPicRest->getLumaAddr( pcCU->getAddr() , uiAbsPartIdx );

  UInt64 uiRecSSD   = xCalcSSD( pOrg, pRec,  iWidth, iHeight, pcPicOrg->getStride() );
  UInt64 uiFiltSSD  = xCalcSSD( pOrg, pFilt, iWidth, iHeight, pcPicOrg->getStride() );

  UInt64 uiNoSplitRate = 0;
  UInt64 uiNoSplitDist = 0;

  UInt uiPrevFlag = pcCU->getAlfCtrlFlag( uiAbsPartIdx);

  m_pcCoderFixedBits->resetBits();
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][CI_TEMP_BEST] );
  }
  if ( uiFiltSSD < uiRecSSD )
  {
    uiNoSplitDist = uiFiltSSD;
    pcCU->setAlfCtrlFlag( uiAbsPartIdx , 1 );
    m_pcEntropyCoder->encodeAlfCtrlFlag( pcCU , uiAbsPartIdx, false, true );
  }
  else
  {
    uiNoSplitDist = uiRecSSD;
    pcCU->setAlfCtrlFlag( uiAbsPartIdx , 0 );
    m_pcEntropyCoder->encodeAlfCtrlFlag( pcCU , uiAbsPartIdx, false, true );
  }
  uiFixedBitsNoSplit +=  m_pcCoderFixedBits->getNumberOfWrittenBits();
  m_pcCoderFixedBits->resetBits();
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
  }
  uiNoSplitRate = uiFixedBitsNoSplit + m_pcEntropyCoder->getNumberOfWrittenBits();
  Double dNoSplitCost  = (Double)( uiNoSplitRate ) * m_dLambdaLuma + (Double)(uiNoSplitDist);

  if( uiDepth < uiAlfCtrlDepth)
  {
    m_pcCoderFixedBits->resetBits();
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth + 1 ][CI_NEXT_BEST] );
    }
    uiSplitRate = m_pcEntropyCoder->getNumberOfWrittenBits() + auiFixedBitsNextBest[ uiDepth + 1 ];
    dSplitCost  = (Double)( uiSplitRate ) * m_dLambdaLuma + (Double)( uiSplitDist );

    if( dSplitCost < dNoSplitCost )
    {
      ruiResultDist += uiSplitDist;
      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[ uiDepth ][CI_NEXT_BEST]->load( m_pppcRDSbacCoder[ uiDepth + 1 ][CI_NEXT_BEST] );
      }
      auiFixedBitsNextBest[ uiDepth ] = auiFixedBitsNextBest[ uiDepth + 1 ];
      pcCU->setAlfCtrlFlag( uiAbsPartIdx , uiPrevFlag );
      return;
    }
  }

  ruiResultDist += uiNoSplitDist;
  if( m_bUseSBACRD )
  {
    m_pppcRDSbacCoder[ uiDepth ][CI_NEXT_BEST]->load( m_pppcRDSbacCoder[ uiDepth  ][CI_TEMP_BEST] );
  }
  auiFixedBitsNextBest[ uiDepth ] = uiFixedBitsNoSplit;

  if( bBoundary )
  {
    pcCU->setAlfCtrlFlagSubParts( pcCU->getAlfCtrlFlag( uiAbsPartIdx) , uiAbsPartIdx , uiDepth );
  }
  else
  {
    pcCU->setWidth (uiAbsPartIdx, iWidth  );
    pcCU->setHeight(uiAbsPartIdx, iHeight );
  }
  pcCU->setDepth( uiAbsPartIdx, uiDepth );
}



Void TEncAdaptiveLoopFilter::xEstimateCorrFU(TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiAbsPartIdx,UInt uiDepth, AlfFilter& rcFilter, double** pdCUCorr  )
{

  Int iLPelX    = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  Int iTPelY    = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  Int iFuWidth  = g_uiMaxCUWidth  >> uiDepth;
  Int iFuHeight = g_uiMaxCUHeight >> uiDepth;

  const Int iOffsetX = iLPelX - Max( 0 , iLPelX - rcFilter.iHorizontalOverlap );
  const Int iOffsetY = iTPelY - Max( 0 , iTPelY - rcFilter.iVerticalOverlap   );
  const Int iMaxX    = pcPicDec->getWidth() -  iLPelX + iOffsetX;
  const Int iMaxY    = pcPicDec->getHeight() - iTPelY + iOffsetY;

  Pel* pOrg = pcPicOrg->getLumaAddr( pcCU->getAddr(), uiAbsPartIdx );
  Pel* pCmp = pcPicDec->getLumaAddr( pcCU->getAddr(), uiAbsPartIdx );

  pCmp -= iOffsetX + pcPicDec->getStride() * iOffsetY;

   xEstimateCorr( pOrg, pCmp, iFuWidth, iFuHeight, pcPicOrg->getStride(), pcPicDec->getStride() , rcFilter , pdCUCorr, iOffsetX, iOffsetY, iMaxX , iMaxY );

}



Void TEncAdaptiveLoopFilter::xEstimateCorrCU(TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiSCUIndx, UChar uhDepth, AlfFilter& rcFilter, double** pdCUCorr  )
{
  Int iLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiSCUIndx ] ];
  Int iTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiSCUIndx ] ];

  const Int iOffsetX = iLPelX - Max( 0 , iLPelX - rcFilter.iHorizontalOverlap );
  const Int iOffsetY = iTPelY - Max( 0 , iTPelY - rcFilter.iVerticalOverlap   );
  const Int iMaxX    = pcPicDec->getWidth() -  iLPelX + iOffsetX;
  const Int iMaxY    = pcPicDec->getHeight() - iTPelY + iOffsetY;

  Pel* pOrg = pcPicOrg->getLumaAddr( pcCU->getAddr(), uiSCUIndx );
  Pel* pCmp = pcPicDec->getLumaAddr( pcCU->getAddr(), uiSCUIndx );

  pCmp -= iOffsetX + pcPicDec->getStride() * iOffsetY;
  xEstimateCorr( pOrg, pCmp, m_uiSCUWidth, m_uiSCUHeight, pcPicOrg->getStride(), pcPicDec->getStride() , rcFilter , pdCUCorr, iOffsetX, iOffsetY, iMaxX , iMaxY );
}


Void TEncAdaptiveLoopFilter::xEstimateCorrCU(TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiSCUIndx, UChar uhDepth, AlfFilter& rcFilter, UInt** puiCUCorr  )
{
  const Int iLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiSCUIndx ] ];
  const Int iTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiSCUIndx ] ];

  const Int iOffsetX = iLPelX - Max( 0 , iLPelX - rcFilter.iHorizontalOverlap );
  const Int iOffsetY = iTPelY - Max( 0 , iTPelY - rcFilter.iVerticalOverlap   );
  const Int iMaxX    = pcPicDec->getWidth() -  iLPelX + iOffsetX;
  const Int iMaxY    = pcPicDec->getHeight() - iTPelY + iOffsetY;

  Pel* pOrg = pcPicOrg->getLumaAddr( pcCU->getAddr(), uiSCUIndx );
  Pel* pCmp = pcPicDec->getLumaAddr( pcCU->getAddr(), uiSCUIndx );

  pCmp -= iOffsetX + pcPicDec->getStride() * iOffsetY;
  xEstimateCorr( pOrg, pCmp, m_uiSCUWidth, m_uiSCUHeight, pcPicOrg->getStride(), pcPicDec->getStride() , rcFilter , puiCUCorr, iOffsetX, iOffsetY, iMaxX , iMaxY );
}


void TEncAdaptiveLoopFilter::xEstimateCorr(  Pel* pOrig, Pel* pDec, Int iWidth, Int iHeight, Int iOrigStride , Int iDecStride, AlfFilter& rcFilter , double** ppdCorr , Int iOffsetX , Int iOffsetY, Int iMaxX , Int iMaxY )
{

  double adPosAccumulation[ ALF_MAX_NUM_COEF ];

  Int iPosIndx = 0;
  Int iPosX = 0;
  Int iPosY = 0;

  for (Int y = iOffsetY ; y < iHeight + iOffsetY ; y++ )
  {
    for (Int x = iOffsetX ; x < iWidth + iOffsetX ; x++ )
    {
      //accumulate values for each CoeffPosition
      memset( adPosAccumulation , 0 , sizeof( double ) * rcFilter.iNumOfCoeffs);

      iPosIndx = 0;
      if( rcFilter.bIsVertical )
      {
        for( Int yy = 0 ; yy < rcFilter.iFilterLength ; yy++ )
        {
            iPosY = std::max( 0 , y - rcFilter.iOverlap + yy );
            iPosY = std::min( iMaxY - 1 , iPosY );

            adPosAccumulation[ rcFilter.aiTapCoeffMapping[ iPosIndx ] ] += pDec[ x + iPosY * iDecStride ];
            iPosIndx++;
        }
      }
      else if( rcFilter.bIsHorizontal )
      {
        for( Int xx = 0 ; xx < rcFilter.iFilterLength ; xx++ )
        {
            iPosX = std::max( 0 , x - rcFilter.iOverlap + xx );
            iPosX = std::min( iMaxX - 1 , iPosX );

            adPosAccumulation[ rcFilter.aiTapCoeffMapping[ iPosIndx ] ] += pDec[ iPosX + y * iDecStride ];
            iPosIndx++;
        }
      }

      for (UInt j = 0; j < rcFilter.iNumOfCoeffs ; j++)
      {
        //autocorrelation of decoded Block
        ppdCorr[j][j] += adPosAccumulation[ j ] * adPosAccumulation[ j ];
        for (UInt i = j + 1 ; i < rcFilter.iNumOfCoeffs ; i++ )
        {
          ppdCorr[i][j] += adPosAccumulation[j] * adPosAccumulation[i];
        }
        //DC offset
#if ALF_DC_CONSIDERED
        ppdCorr[rcFilter.iNumOfCoeffs][j] += adPosAccumulation[j];

        //cross-correlation between Pixels of decoded Block at CoeffPositions and Pixel (x,y) from original Block
        ppdCorr[j][rcFilter.iNumOfCoeffs + 1] += pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride] * adPosAccumulation[j];
#else
        //cross-correlation between Pixels of decoded Block at CoeffPositions and Pixel (x,y) from original Block
        ppdCorr[j][rcFilter.iNumOfCoeffs ] += pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride] * adPosAccumulation[j];
#endif
      }
#if ALF_DC_CONSIDERED
      ppdCorr[rcFilter.iNumOfCoeffs][rcFilter.iNumOfCoeffs]     += 1;
      ppdCorr[rcFilter.iNumOfCoeffs][rcFilter.iNumOfCoeffs + 1] += pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride ];
#endif
    }
  }


}

void TEncAdaptiveLoopFilter::xEstimateCorr(  Pel* pOrig, Pel* pDec, Int iWidth, Int iHeight, Int iOrigStride , Int iDecStride, AlfFilter& rcFilter , UInt** ppuiCorr , Int iOffsetX , Int iOffsetY, Int iMaxX , Int iMaxY )
{

  double adPosAccumulation[ ALF_MAX_NUM_COEF ];


   Int iPosIndx = 0;
   Int iPosX = 0;
   Int iPosY = 0;


   for (Int y = iOffsetY ; y < iHeight + iOffsetY ; y++ )
   {
     for (Int x = iOffsetX ; x < iWidth + iOffsetX ; x++ )
     {
       //accumulate values for each CoeffPosition
       memset( adPosAccumulation , 0 , sizeof( double ) * rcFilter.iNumOfCoeffs);

       iPosIndx = 0;
       if( rcFilter.bIsVertical )
       {
         for( Int yy = 0 ; yy < rcFilter.iFilterLength ; yy++ )
         {
             iPosY = std::max( 0 , y - rcFilter.iOverlap + yy );
             iPosY = std::min( iMaxY - 1 , iPosY );

             adPosAccumulation[ rcFilter.aiTapCoeffMapping[ iPosIndx ] ] += pDec[ x + iPosY * iDecStride ];
             iPosIndx++;
         }

       }
       else if( rcFilter.bIsHorizontal )
       {
         for( Int xx = 0 ; xx < rcFilter.iFilterLength ; xx++ )
         {
             iPosX = std::max( 0 , x - rcFilter.iOverlap + xx );
             iPosX = std::min( iMaxX - 1 , iPosX );

             adPosAccumulation[ rcFilter.aiTapCoeffMapping[ iPosIndx ] ] += pDec[ iPosX + y * iDecStride ];
             iPosIndx++;
         }
       }

       for (UInt j = 0; j < rcFilter.iNumOfCoeffs ; j++)
       {
         //autocorrelation of decoded Block
         ppuiCorr[j][j] += (UInt)(adPosAccumulation[ j ] * adPosAccumulation[ j ]);
         for (UInt i = j + 1 ; i < rcFilter.iNumOfCoeffs ; i++ )
         {
           ppuiCorr[i][j] += (UInt)(adPosAccumulation[j] * adPosAccumulation[i]);
         }
         //DC offset
#if ALF_DC_CONSIDERED
         ppuiCorr[rcFilter.iNumOfCoeffs][j] += (UInt)adPosAccumulation[j];

         //cross-correlation between Pixels of decoded Block at CoeffPositions and Pixel (x,y) from original Block
         ppuiCorr[j][rcFilter.iNumOfCoeffs + 1] += (UInt)(pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride] * adPosAccumulation[j]);
#else
         //cross-correlation between Pixels of decoded Block at CoeffPositions and Pixel (x,y) from original Block
         ppuiCorr[j][rcFilter.iNumOfCoeffs    ] += (UInt)(pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride] * adPosAccumulation[j]);
#endif
       }
#if ALF_DC_CONSIDERED
       ppuiCorr[rcFilter.iNumOfCoeffs][rcFilter.iNumOfCoeffs]     += 1;
       ppuiCorr[rcFilter.iNumOfCoeffs][rcFilter.iNumOfCoeffs + 1] += pOrig[  ( x - iOffsetX ) + ( y - iOffsetY )  * iOrigStride ];
#endif
     }
   }
}
#else
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
#endif
