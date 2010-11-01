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

/** \file     TEncSearch.cpp
 \brief    encoder search class
 */

#include "../TLibCommon/TypeDef.h"
#include "../TLibCommon/TComMotionInfo.h"
#include "TEncSearch.h"

#ifdef ROUNDING_CONTROL_BIPRED
#ifndef ROUNDING_CONTROL_BIPRED_FIX
__inline Pel  xClip  (Pel x )      { return ( (x < 0) ? 0 : (x > (Pel)g_uiIBDI_MAX) ? (Pel)g_uiIBDI_MAX : x ); }
#endif
#endif

#ifdef DCM_PBIC
extern Int entropyBits[128];
#endif

static TComMv s_acMvRefineH[9] =
{
  TComMv(  0,  0 ), // 0
  TComMv(  0, -1 ), // 1
  TComMv(  0,  1 ), // 2
  TComMv( -1,  0 ), // 3
  TComMv(  1,  0 ), // 4
  TComMv( -1, -1 ), // 5
  TComMv(  1, -1 ), // 6
  TComMv( -1,  1 ), // 7
  TComMv(  1,  1 )  // 8
};


#ifdef QC_SIFO
static TComMv s_acMvRefineQ[16] =
{
  TComMv(  0,  0 ), // 0
  TComMv(  0, -1 ), // 1
  TComMv(  0,  1 ), // 2
  TComMv( -1, -1 ), // 5
  TComMv(  1, -1 ), // 6
  TComMv( -1,  0 ), // 3
  TComMv(  1,  0 ), // 4
  TComMv( -1,  1 ), // 7
  TComMv(  1,  1 ), // 8
  TComMv( -1, -2 ), // 9
  TComMv(  0, -2 ), // 10
  TComMv(  1, -2 ), // 11
  TComMv( -2, -1 ), // 12
  TComMv( -2,  0 ), // 13
  TComMv( -2,  1 ), // 14
  TComMv( -2, -2 )  // 15
};
#else
static TComMv s_acMvRefineQ[9] =
{
  TComMv(  0,  0 ), // 0
  TComMv(  0, -1 ), // 1
  TComMv(  0,  1 ), // 2
  TComMv( -1, -1 ), // 5
  TComMv(  1, -1 ), // 6
  TComMv( -1,  0 ), // 3
  TComMv(  1,  0 ), // 4
  TComMv( -1,  1 ), // 7
  TComMv(  1,  1 )  // 8
};
#endif

static UInt s_auiDFilter[9] =
{
  0, 1, 0,
  2, 3, 2,
  0, 1, 0
};

TEncSearch::TEncSearch()
{
#if HHI_RQT
  m_ppcQTTempCoeffY  = NULL;
  m_ppcQTTempCoeffCb = NULL;
  m_ppcQTTempCoeffCr = NULL;
  m_pcQTTempCoeffY   = NULL;
  m_pcQTTempCoeffCb  = NULL;
  m_pcQTTempCoeffCr  = NULL;
  m_puhQTTempTrIdx   = NULL;
  m_puhQTTempCbf[0] = m_puhQTTempCbf[1] = m_puhQTTempCbf[2] = NULL;
  m_pcQTTempTComYuv  = NULL;
#endif
  m_pcEncCfg = NULL;
  m_pcEntropyCoder = NULL;
  m_pTempPel = NULL;
}

TEncSearch::~TEncSearch()
{
  if ( m_pTempPel )
  {
    delete [] m_pTempPel;
    m_pTempPel = NULL;
  }
#if HHI_RQT
  const UInt uiNumLayersAllocated = m_pcEncCfg->getQuadtreeTULog2MaxSize()-m_pcEncCfg->getQuadtreeTULog2MinSize()+1;
  for( UInt ui = 0; ui < uiNumLayersAllocated; ++ui )
  {
    delete[] m_ppcQTTempCoeffY[ui];
    delete[] m_ppcQTTempCoeffCb[ui];
    delete[] m_ppcQTTempCoeffCr[ui];
    m_pcQTTempTComYuv[ui].destroy();
  }
  delete[] m_ppcQTTempCoeffY;
  delete[] m_ppcQTTempCoeffCb;
  delete[] m_ppcQTTempCoeffCr;
  delete[] m_pcQTTempCoeffY;
  delete[] m_pcQTTempCoeffCb;
  delete[] m_pcQTTempCoeffCr;
  delete[] m_puhQTTempTrIdx;
  delete[] m_puhQTTempCbf[0];
  delete[] m_puhQTTempCbf[1];
  delete[] m_puhQTTempCbf[2];
  delete[] m_pcQTTempTComYuv;
#endif
}

void TEncSearch::init(  TEncCfg*      pcEncCfg,
                      TComTrQuant*  pcTrQuant,
                      Int           iSearchRange,
                      Int           iFastSearch,
                      Int           iMaxDeltaQP,
                      TEncEntropy*  pcEntropyCoder,
                      TComRdCost*   pcRdCost,
                      TEncSbac*** pppcRDSbacCoder,
                      TEncSbac*   pcRDGoOnSbacCoder
                      )
{
  m_pcEncCfg             = pcEncCfg;
  m_pcTrQuant            = pcTrQuant;
  m_iSearchRange         = iSearchRange;
  m_iFastSearch          = iFastSearch;
  m_iMaxDeltaQP          = iMaxDeltaQP;
  m_pcEntropyCoder       = pcEntropyCoder;
  m_pcRdCost             = pcRdCost;
  
  m_pppcRDSbacCoder     = pppcRDSbacCoder;
  m_pcRDGoOnSbacCoder   = pcRDGoOnSbacCoder;
  
  m_bUseSBACRD          = pppcRDSbacCoder ? true : false;
  
  for (Int iDir = 0; iDir < 2; iDir++)
    for (Int iRefIdx = 0; iRefIdx < 33; iRefIdx++)
    {
      m_aaiAdaptSR[iDir][iRefIdx] = iSearchRange;
    }
  
  m_puiDFilter = s_auiDFilter + 4;
  
  // initialize motion cost
  m_pcRdCost->initRateDistortionModel( m_iSearchRange << 2 );
  
  for( Int iNum = 0; iNum < AMVP_MAX_NUM_CANDS+1; iNum++)
    for( Int iIdx = 0; iIdx < AMVP_MAX_NUM_CANDS; iIdx++)
    {
      if (iIdx < iNum)
        m_auiMVPIdxCost[iIdx][iNum] = xGetMvpIdxBits(iIdx, iNum);
      else
        m_auiMVPIdxCost[iIdx][iNum] = MAX_INT;
    }
  
#ifdef DCM_PBIC
    for( Int iNum = 0; iNum < AICP_MAX_NUM_CANDS+1; iNum++)
      for( Int iIdx = 0; iIdx < AICP_MAX_NUM_CANDS; iIdx++)
      {
        if (iIdx < iNum)
          m_auiICPIdxCost[iIdx][iNum] = xGetIcpIdxBits(iIdx, iNum);
        else
          m_auiICPIdxCost[iIdx][iNum] = MAX_INT;
      }
#endif

  initTempBuff();
  
  m_pTempPel = new Pel[g_uiMaxCUWidth*g_uiMaxCUHeight];
  
  m_iDIFTap2 = (m_iDIFTap << 1);
  
#if HHI_RQT
  const UInt uiNumLayersToAllocate = pcEncCfg->getQuadtreeTULog2MaxSize()-pcEncCfg->getQuadtreeTULog2MinSize()+1;
  m_ppcQTTempCoeffY  = new TCoeff*[uiNumLayersToAllocate];
  m_ppcQTTempCoeffCb = new TCoeff*[uiNumLayersToAllocate];
  m_ppcQTTempCoeffCr = new TCoeff*[uiNumLayersToAllocate];
  m_pcQTTempCoeffY   = new TCoeff [g_uiMaxCUWidth*g_uiMaxCUHeight   ];
  m_pcQTTempCoeffCb  = new TCoeff [g_uiMaxCUWidth*g_uiMaxCUHeight>>2];
  m_pcQTTempCoeffCr  = new TCoeff [g_uiMaxCUWidth*g_uiMaxCUHeight>>2];
  
  const UInt uiNumPartitions = 1<<(g_uiMaxCUDepth<<1);
  m_puhQTTempTrIdx   = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[0]  = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[1]  = new UChar  [uiNumPartitions];
  m_puhQTTempCbf[2]  = new UChar  [uiNumPartitions];
  m_pcQTTempTComYuv  = new TComYuv[uiNumLayersToAllocate];
  for( UInt ui = 0; ui < uiNumLayersToAllocate; ++ui )
  {
    m_ppcQTTempCoeffY[ui]  = new TCoeff[g_uiMaxCUWidth*g_uiMaxCUHeight   ];
    m_ppcQTTempCoeffCb[ui] = new TCoeff[g_uiMaxCUWidth*g_uiMaxCUHeight>>2];
    m_ppcQTTempCoeffCr[ui] = new TCoeff[g_uiMaxCUWidth*g_uiMaxCUHeight>>2];
    m_pcQTTempTComYuv[ui].create( g_uiMaxCUWidth, g_uiMaxCUHeight );
  }
#endif
}

#if FASTME_SMOOTHER_MV
#define FIRSTSEARCHSTOP     1
#else
#define FIRSTSEARCHSTOP     0
#endif

#define TZ_SEARCH_CONFIGURATION                                                                                 \
const Int  iRaster                  = 3;  /* TZ soll von aussen ?ergeben werden */                            \
const Bool bTestOtherPredictedMV    = 1;                                                                      \
const Bool bTestZeroVector          = 1;                                                                      \
const Bool bTestZeroVectorStart     = 0;                                                                      \
const Bool bTestZeroVectorStop      = 0;                                                                      \
const Bool bFirstSearchDiamond      = 1;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bFirstSearchStop         = FIRSTSEARCHSTOP;                                                        \
const UInt uiFirstSearchRounds      = 3;  /* first search stop X rounds after best match (must be >=1) */     \
const Bool bEnableRasterSearch      = 1;                                                                      \
const Bool bAlwaysRasterSearch      = 0;  /* ===== 1: BETTER but factor 2 slower ===== */                     \
const Bool bRasterRefinementEnable  = 0;  /* enable either raster refinement or star refinement */            \
const Bool bRasterRefinementDiamond = 0;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bStarRefinementEnable    = 1;  /* enable either star refinement or raster refinement */            \
const Bool bStarRefinementDiamond   = 1;  /* 1 = xTZ8PointDiamondSearch   0 = xTZ8PointSquareSearch */        \
const Bool bStarRefinementStop      = 0;                                                                      \
const UInt uiStarRefinementRounds   = 2;  /* star refinement stop X rounds after best match (must be >=1) */  \


__inline Void TEncSearch::xTZSearchHelp( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance )
{
  UInt  uiSad;
  
  Pel*  piRefSrch;
  
  piRefSrch = rcStruct.piRefY + iSearchY * rcStruct.iYStride + iSearchX;
  
  //-- jclee for using the SAD function pointer
  m_pcRdCost->setDistParam( pcPatternKey, piRefSrch, rcStruct.iYStride,  m_cDistParam );
  
  // fast encoder decision: use subsampled SAD when rows > 8 for integer ME
  if ( m_pcEncCfg->getUseFastEnc() )
  {
    if ( m_cDistParam.iRows > 8 )
    {
      m_cDistParam.iSubShift = 1;
    }
  }
  
  // distortion
  uiSad = m_cDistParam.DistFunc( &m_cDistParam );
  
#if HHI_IMVP
  if ( m_pcEncCfg->getUseIMP() )
  {
#ifdef QC_AMVRES
    TComMv cMvPred;
    if(m_pcEncCfg->getUseAMVRes())
    {
      cMvPred = m_cMvPredMeasure.getMVPred( (iSearchX<<3) , (iSearchY<<3) );
      cMvPred.scale_down();
    }
    else
      cMvPred = m_cMvPredMeasure.getMVPred( (iSearchX<<2) , (iSearchY<<2) );
    
    m_pcRdCost->setPredictor( cMvPred );
#else
    TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iSearchX<<2) , (iSearchY<<2) );
    m_pcRdCost->setPredictor( cMvPred );
#endif
  }
#endif
  // motion cost
  uiSad += m_pcRdCost->getCost( iSearchX, iSearchY );
  
  if( uiSad < rcStruct.uiBestSad )
  {
    rcStruct.uiBestSad      = uiSad;
    rcStruct.iBestX         = iSearchX;
    rcStruct.iBestY         = iSearchY;
    rcStruct.uiBestDistance = uiDistance;
    rcStruct.uiBestRound    = 0;
    rcStruct.ucPointNr      = ucPointNr;
  }
}

__inline Void TEncSearch::xTZ2PointSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 2 point search,                   //   1 2 3
  // check only the 2 untested points  //   4 0 5
  // around the start point            //   6 7 8
  Int iStartX = rcStruct.iBestX;
  Int iStartY = rcStruct.iBestY;
  switch( rcStruct.ucPointNr )
  {
    case 1:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY, 0, 2 );
      }
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY - 1, 0, 2 );
      }
    }
      break;
    case 2:
    {
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        if ( (iStartX - 1) >= iSrchRngHorLeft )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY - 1, 0, 2 );
        }
        if ( (iStartX + 1) <= iSrchRngHorRight )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY - 1, 0, 2 );
        }
      }
    }
      break;
    case 3:
    {
      if ( (iStartY - 1) >= iSrchRngVerTop )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY - 1, 0, 2 );
      }
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY, 0, 2 );
      }
    }
      break;
    case 4:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        if ( (iStartY + 1) <= iSrchRngVerBottom )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY + 1, 0, 2 );
        }
        if ( (iStartY - 1) >= iSrchRngVerTop )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY - 1, 0, 2 );
        }
      }
    }
      break;
    case 5:
    {
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        if ( (iStartY - 1) >= iSrchRngVerTop )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY - 1, 0, 2 );
        }
        if ( (iStartY + 1) <= iSrchRngVerBottom )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY + 1, 0, 2 );
        }
      }
    }
      break;
    case 6:
    {
      if ( (iStartX - 1) >= iSrchRngHorLeft )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY , 0, 2 );
      }
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY + 1, 0, 2 );
      }
    }
      break;
    case 7:
    {
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        if ( (iStartX - 1) >= iSrchRngHorLeft )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX - 1, iStartY + 1, 0, 2 );
        }
        if ( (iStartX + 1) <= iSrchRngHorRight )
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY + 1, 0, 2 );
        }
      }
    }
      break;
    case 8:
    {
      if ( (iStartX + 1) <= iSrchRngHorRight )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX + 1, iStartY, 0, 2 );
      }
      if ( (iStartY + 1) <= iSrchRngVerBottom )
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iStartY + 1, 0, 2 );
      }
    }
      break;
    default:
    {
      assert( false );
    }
      break;
  } // switch( rcStruct.ucPointNr )
}

__inline Void TEncSearch::xTZ8PointSquareSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 8 point search,                   //   1 2 3
  // search around the start point     //   4 0 5
  // with the required  distance       //   6 7 8
  assert( iDist != 0 );
  const Int iTop        = iStartY - iDist;
  const Int iBottom     = iStartY + iDist;
  const Int iLeft       = iStartX - iDist;
  const Int iRight      = iStartX + iDist;
  rcStruct.uiBestRound += 1;
  
  if ( iTop >= iSrchRngVerTop ) // check top
  {
    if ( iLeft >= iSrchRngHorLeft ) // check top left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iTop, 1, iDist );
    }
    // top middle
    xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
    
    if ( iRight <= iSrchRngHorRight ) // check top right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iTop, 3, iDist );
    }
  } // check top
  if ( iLeft >= iSrchRngHorLeft ) // check middle left
  {
    xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
  }
  if ( iRight <= iSrchRngHorRight ) // check middle right
  {
    xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
  }
  if ( iBottom <= iSrchRngVerBottom ) // check bottom
  {
    if ( iLeft >= iSrchRngHorLeft ) // check bottom left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iBottom, 6, iDist );
    }
    // check bottom middle
    xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
    
    if ( iRight <= iSrchRngHorRight ) // check bottom right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iBottom, 8, iDist );
    }
  } // check bottom
}

__inline Void TEncSearch::xTZ8PointDiamondSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  // 8 point search,                   //   1 2 3
  // search around the start point     //   4 0 5
  // with the required  distance       //   6 7 8
  assert ( iDist != 0 );
  const Int iTop        = iStartY - iDist;
  const Int iBottom     = iStartY + iDist;
  const Int iLeft       = iStartX - iDist;
  const Int iRight      = iStartX + iDist;
  rcStruct.uiBestRound += 1;
  
  if ( iDist == 1 ) // iDist == 1
  {
    if ( iTop >= iSrchRngVerTop ) // check top
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
    }
    if ( iLeft >= iSrchRngHorLeft ) // check middle left
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
    }
    if ( iRight <= iSrchRngHorRight ) // check middle right
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
    }
    if ( iBottom <= iSrchRngVerBottom ) // check bottom
    {
      xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
    }
  }
  else // if (iDist != 1)
  {
    if ( iDist <= 8 )
    {
      const Int iTop_2      = iStartY - (iDist>>1);
      const Int iBottom_2   = iStartY + (iDist>>1);
      const Int iLeft_2     = iStartX - (iDist>>1);
      const Int iRight_2    = iStartX + (iDist>>1);
      
      if (  iTop >= iSrchRngVerTop && iLeft >= iSrchRngHorLeft &&
          iRight <= iSrchRngHorRight && iBottom <= iSrchRngVerBottom ) // check border
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX,  iTop,      2, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2,  iTop_2,    1, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iTop_2,    3, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft,    iStartY,   4, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight,   iStartY,   5, iDist    );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2,  iBottom_2, 6, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iBottom_2, 8, iDist>>1 );
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX,  iBottom,   7, iDist    );
      }
      else // check border
      {
        if ( iTop >= iSrchRngVerTop ) // check top
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 2, iDist );
        }
        if ( iTop_2 >= iSrchRngVerTop ) // check half top
        {
          if ( iLeft_2 >= iSrchRngHorLeft ) // check half left
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2, iTop_2, 1, (iDist>>1) );
          }
          if ( iRight_2 <= iSrchRngHorRight ) // check half right
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iTop_2, 3, (iDist>>1) );
          }
        } // check half top
        if ( iLeft >= iSrchRngHorLeft ) // check left
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 4, iDist );
        }
        if ( iRight <= iSrchRngHorRight ) // check right
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 5, iDist );
        }
        if ( iBottom_2 <= iSrchRngVerBottom ) // check half bottom
        {
          if ( iLeft_2 >= iSrchRngHorLeft ) // check half left
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iLeft_2, iBottom_2, 6, (iDist>>1) );
          }
          if ( iRight_2 <= iSrchRngHorRight ) // check half right
          {
            xTZSearchHelp( pcPatternKey, rcStruct, iRight_2, iBottom_2, 8, (iDist>>1) );
          }
        } // check half bottom
        if ( iBottom <= iSrchRngVerBottom ) // check bottom
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 7, iDist );
        }
      } // check border
    }
    else // iDist > 8
    {
      if ( iTop >= iSrchRngVerTop && iLeft >= iSrchRngHorLeft &&
          iRight <= iSrchRngHorRight && iBottom <= iSrchRngVerBottom ) // check border
      {
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop,    0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iLeft,   iStartY, 0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iRight,  iStartY, 0, iDist );
        xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 0, iDist );
        for ( Int index = 1; index < 4; index++ )
        {
          Int iPosYT = iTop    + ((iDist>>2) * index);
          Int iPosYB = iBottom - ((iDist>>2) * index);
          Int iPosXL = iStartX - ((iDist>>2) * index);
          Int iPosXR = iStartX + ((iDist>>2) * index);
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYT, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYT, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYB, 0, iDist );
          xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYB, 0, iDist );
        }
      }
      else // check border
      {
        if ( iTop >= iSrchRngVerTop ) // check top
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iTop, 0, iDist );
        }
        if ( iLeft >= iSrchRngHorLeft ) // check left
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iLeft, iStartY, 0, iDist );
        }
        if ( iRight <= iSrchRngHorRight ) // check right
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iRight, iStartY, 0, iDist );
        }
        if ( iBottom <= iSrchRngVerBottom ) // check bottom
        {
          xTZSearchHelp( pcPatternKey, rcStruct, iStartX, iBottom, 0, iDist );
        }
        for ( Int index = 1; index < 4; index++ )
        {
          Int iPosYT = iTop    + ((iDist>>2) * index);
          Int iPosYB = iBottom - ((iDist>>2) * index);
          Int iPosXL = iStartX - ((iDist>>2) * index);
          Int iPosXR = iStartX + ((iDist>>2) * index);
          
          if ( iPosYT >= iSrchRngVerTop ) // check top
          {
            if ( iPosXL >= iSrchRngHorLeft ) // check left
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYT, 0, iDist );
            }
            if ( iPosXR <= iSrchRngHorRight ) // check right
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYT, 0, iDist );
            }
          } // check top
          if ( iPosYB <= iSrchRngVerBottom ) // check bottom
          {
            if ( iPosXL >= iSrchRngHorLeft ) // check left
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXL, iPosYB, 0, iDist );
            }
            if ( iPosXR <= iSrchRngHorRight ) // check right
            {
              xTZSearchHelp( pcPatternKey, rcStruct, iPosXR, iPosYB, 0, iDist );
            }
          } // check bottom
        } // for ...
      } // check border
    } // iDist <= 8
  } // iDist == 1
}

#ifdef ROUNDING_CONTROL_BIPRED
UInt TEncSearch::xPatternRefinement_Bi    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac, Pel* pcRef2, Bool bRound )
{
  UInt  uiDist;
  UInt  uiDistBest  = MAX_UINT;
  UInt  uiDirecBest = 0;
  
  Pel*  piRefPos;
  
  m_pcRdCost->setDistParam_Bi( pcPatternKey, piRef, iRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);
  
  for (UInt i = 0; i < 9; i++)
  {
    TComMv cMvTest = pcMvRefine[i];
    cMvTest += rcMvFrac;
    piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * iFrac;
    m_cDistParam.pCur = piRefPos;
    uiDist = m_cDistParam.DistFuncRnd( &m_cDistParam, pcRef2, bRound );
#if HHI_IMVP
    if ( m_pcEncCfg->getUseIMP() )
    {
#ifdef QC_AMVRES
      TComMv cMvPred;
      if(m_pcEncCfg->getUseAMVRes())
      {
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<iFrac, cMvTest.getVer()<<iFrac);
        cMvPred.scale_down();
      }
      else
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      
      m_pcRdCost->setPredictor( cMvPred );
#else
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      m_pcRdCost->setPredictor( cMvPred );
#endif
    }
#endif
    uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer() );
    
    if ( uiDist < uiDistBest )
    {
      uiDistBest  = uiDist;
      uiDirecBest = i;
    }
  }
  
  rcMvFrac = pcMvRefine[uiDirecBest];
  
  return uiDistBest;
}
#ifdef QC_AMVRES
#if HHI_INTERP_FILTER
UInt TEncSearch::xPatternRefinementHAM_MOMS_Bi    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth, InterpFilterType ePFilt,TComMv* PredMv, Pel* pcRef2, Bool bRound )
{
  UInt  uiDist;
  UInt  uiDistBest  = uiDistBest_onefourth;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  m_pcRdCost->setDistParam_Bi( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv cMvQ;
  
  rcMvFrac <<=1;
  TComMv cMvHAM;
  
  for (Int dMVx = -1; dMVx <= 1; dMVx++)
  {
    for (Int dMVy = -1; dMVy <= 1; dMVy++)
    {
      if (dMVy!=0 || dMVx!=0)
      {
        TComMv cMvTest = rcMvFrac;
#if HHI_IMVP
        if ( m_pcEncCfg->getUseIMP() )
        {
          TComMv cMvPred;
          cMvPred = m_cMvPredMeasure.getMVPred(cMvTest.getHor()+dMVx, cMvTest.getVer()+dMVy);
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
#endif
          m_pcRdCost->setPredictor(*PredMv);
        
        predInterLumaBlkHAM_ME_MOMS(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvTest, pcPatternKey->getROIYWidth(), 
                                    pcPatternKey->getROIYHeight(), ePFilt,  dMVx,  dMVy);
		uiDist = m_cDistParam.DistFuncRnd( &m_cDistParam, pcRef2, bRound );
        uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer(),1 );
        if ( uiDist < uiDistBest )
        {
          cMvHAM.set(dMVx,dMVy);
          uiDistBest  = uiDist;
        }
      }
    }
  }
  rcMvFrac = cMvHAM;
  
  return uiDistBest;
}
#endif

UInt TEncSearch::xPatternRefinementHAM_DIF_Bi( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth,TComMv* PredMv, Pel* pcRef2, Bool bRound )
{
  UInt  uiDist;
  UInt  uiDistBest  = uiDistBest_onefourth;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  
  m_pcRdCost->setDistParam_Bi( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME());
  
  TComMv cMvQ;
  
  rcMvFrac <<=1;
  TComMv cMvHAM;
  for (Int dMVx = -1; dMVx <= 1; dMVx++)
  {
    for (Int dMVy = -1; dMVy <= 1; dMVy++)
    {
      if (dMVy!=0 || dMVx!=0)
      {
        TComMv cMvTest = rcMvFrac;
#if HHI_IMVP
        if ( m_pcEncCfg->getUseIMP() )
        {
          TComMv cMvPred;
          cMvPred = m_cMvPredMeasure.getMVPred(cMvTest.getHor()+dMVx, cMvTest.getVer()+dMVy);
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
#endif
          m_pcRdCost->setPredictor(*PredMv);
        
        xPredInterLumaBlkHMVME(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvTest, pcPatternKey->getROIYWidth(), 
                               pcPatternKey->getROIYHeight(),   dMVx,  dMVy);
        uiDist = m_cDistParam.DistFuncRnd( &m_cDistParam,pcRef2 ,bRound );
        uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer(),1 );
        if ( uiDist < uiDistBest )
        {
          cMvHAM.set(dMVx,dMVy);
          uiDistBest  = uiDist;
        }
      }
    }
  }
  rcMvFrac = cMvHAM;
  
  return uiDistBest;
}
#endif
#endif




#ifdef QC_AMVRES
#if HHI_INTERP_FILTER
UInt TEncSearch::xPatternRefinementHAM_MOMS    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth, InterpFilterType ePFilt,TComMv* PredMv)
{
  UInt  uiDist;
  UInt  uiDistBest  = uiDistBest_onefourth;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  
  m_pcRdCost->setDistParam( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv cMvQ;
  
  rcMvFrac <<=1;
  TComMv cMvHAM;
  
  for (Int dMVx = -1; dMVx <= 1; dMVx++)
  {
    for (Int dMVy = -1; dMVy <= 1; dMVy++)
    {
      if (dMVy!=0 || dMVx!=0)
      {
        TComMv cMvTest = rcMvFrac;
#if HHI_IMVP
        if ( m_pcEncCfg->getUseIMP() )
        {
          TComMv cMvPred;
          cMvPred = m_cMvPredMeasure.getMVPred(cMvTest.getHor()+dMVx, cMvTest.getVer()+dMVy);
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
#endif
          m_pcRdCost->setPredictor(*PredMv);
        
        predInterLumaBlkHAM_ME_MOMS(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvTest, pcPatternKey->getROIYWidth(), 
                                    pcPatternKey->getROIYHeight(), ePFilt,  dMVx,  dMVy);
        uiDist = m_cDistParam.DistFunc( &m_cDistParam );
        uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer(),1 );
        if ( uiDist < uiDistBest )
        {
          cMvHAM.set(dMVx,dMVy);
          uiDistBest  = uiDist;
        }
      }
    }
  }
  rcMvFrac = cMvHAM;
  
  return uiDistBest;
}
#endif

UInt TEncSearch::xPatternRefinementHAM_DIF( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth,TComMv* PredMv)
{
  UInt  uiDist;
  UInt  uiDistBest  = uiDistBest_onefourth;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  
  m_pcRdCost->setDistParam( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv cMvQ;
  
  rcMvFrac <<=1;
  TComMv cMvHAM;
  for (Int dMVx = -1; dMVx <= 1; dMVx++)
  {
    for (Int dMVy = -1; dMVy <= 1; dMVy++)
    {
      if (dMVy!=0 || dMVx!=0)
      {
        TComMv cMvTest = rcMvFrac;
#if HHI_IMVP
        if ( m_pcEncCfg->getUseIMP() )
        {
          TComMv cMvPred;
          cMvPred = m_cMvPredMeasure.getMVPred(cMvTest.getHor()+dMVx, cMvTest.getVer()+dMVy);
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
#endif
          m_pcRdCost->setPredictor(*PredMv);
        
        xPredInterLumaBlkHMVME(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvTest, pcPatternKey->getROIYWidth(), 
                               pcPatternKey->getROIYHeight(),   dMVx,  dMVy);
        uiDist = m_cDistParam.DistFunc( &m_cDistParam );
        uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer(),1 );
        if ( uiDist < uiDistBest )
        {
          cMvHAM.set(dMVx,dMVy);
          uiDistBest  = uiDist;
        }
      }
    }
  }
  rcMvFrac = cMvHAM;
  
  return uiDistBest;
}
#endif

//<--

UInt TEncSearch::xPatternRefinement    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac )
{
  UInt  uiDist;
  UInt  uiDistBest  = MAX_UINT;
  UInt  uiDirecBest = 0;
  
  Pel*  piRefPos;
  m_pcRdCost->setDistParam( pcPatternKey, piRef, iRefStride, iIntStep, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);
  
  for (UInt i = 0; i < 9; i++)
  {
    TComMv cMvTest = pcMvRefine[i];
    cMvTest += rcMvFrac;
    piRefPos = piRef + (pcMvRefine[i].getHor() + iRefStride * pcMvRefine[i].getVer()) * iFrac;
    m_cDistParam.pCur = piRefPos;
    uiDist = m_cDistParam.DistFunc( &m_cDistParam );
#if HHI_IMVP
    if ( m_pcEncCfg->getUseIMP() )
    {
#ifdef QC_AMVRES
      TComMv cMvPred;
      if(m_pcEncCfg->getUseAMVRes())
      {
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<iFrac, cMvTest.getVer()<<iFrac);
        cMvPred.scale_down();
      }
      else
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      
      m_pcRdCost->setPredictor( cMvPred );
#else
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      m_pcRdCost->setPredictor( cMvPred );
#endif
    }
#endif
    uiDist += m_pcRdCost->getCost( cMvTest.getHor(), cMvTest.getVer() );
    
    if ( uiDist < uiDistBest )
    {
      uiDistBest  = uiDist;
      uiDirecBest = i;
    }
  }
  
  rcMvFrac = pcMvRefine[uiDirecBest];
  
  return uiDistBest;
}


Void TEncSearch::xRecurIntraChromaSearchADI( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiMode, UInt uiWidth, UInt uiHeight, UInt uiMaxDepth, UInt uiCurrDepth, TextType eText )
{
  UInt uiCoeffOffset = uiWidth*uiHeight;
  if( uiMaxDepth == uiCurrDepth )
  {
    UInt uiX, uiY;
    Pel* pOrg  = piOrg;
    Pel* pPred = piPred;
    Pel* pResi = piResi;
    Pel* pReco = piReco;
    Pel* pRecoPic;
    if( eText == TEXT_CHROMA_U)
      pRecoPic= pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiAbsPartIdx);
    else
      pRecoPic= pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiAbsPartIdx);
    
    UInt uiReconStride = pcCU->getPic()->getPicYuvRec()->getCStride();
    UInt uiAbsSum = 0;
    
    if (m_pcEncCfg->getUseRDOQ())
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, uiWidth, eText );
    
    pcCU->getPattern()->initPattern( pcCU, uiCurrDepth, uiAbsPartIdx );
    
    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;
    
#if HHI_RQT_INTRA
    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, uiCurrDepth, m_piYuvExt,m_iYuvExtStride,m_iYuvExtHeight,bAboveAvail,bLeftAvail);
#else
    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, m_piYuvExt,m_iYuvExtStride,m_iYuvExtHeight,bAboveAvail,bLeftAvail);
#endif
    
    Int*   pPatChr;
    
    if (eText==TEXT_CHROMA_U)
      pPatChr=  pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_piYuvExt );
    else // (eText==TEXT_CHROMA_V)
      pPatChr=  pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_piYuvExt );
    
#if ANG_INTRA
    if ( pcCU->angIntraEnabledPredPart( uiAbsPartIdx ) )
      predIntraChromaAng( pcCU->getPattern(),pPatChr,uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    else
      predIntraChromaAdi( pcCU->getPattern(),pPatChr,uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
    predIntraChromaAdi( pcCU->getPattern(),pPatChr,uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
    
    // Make residual from prediction. (MUST BE FIXED FOR EACH TRANSFORM UNIT PREDICTION)
    UChar indexROT = pcCU->getROTindex(0);
    
    // Get Residual
    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        pResi[uiX] = pOrg[uiX] - pPred[uiX];
      }
      pOrg  += uiStride;
      pResi += uiStride;
      pPred += uiStride;
    }
    
    pPred = piPred;
    pResi = piResi;
    
    m_pcTrQuant->transformNxN( pcCU, pResi, uiStride, piCoeff, uiWidth, uiHeight, uiAbsSum, eText, uiAbsPartIdx, indexROT );
    
    if ( uiAbsSum )
    {
#if QC_MDDT
      m_pcTrQuant->invtransformNxN( eText, REG_DCT, pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
#else
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
#endif
    }
    else
    {
      memset(piCoeff,  0, sizeof(TCoeff)*uiCoeffOffset);
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        memset(pResi, 0, sizeof(Pel)*uiWidth);
        pResi += uiStride;
      }
    }
    
    m_pcEntropyCoder->encodeCoeffNxN( pcCU, piCoeff, uiAbsPartIdx, uiWidth, uiHeight, pcCU->getDepth(0)+uiCurrDepth, eText, true );
    
    pPred = piPred;
    pResi = piResi;
    
    // Reconstruction
    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco   [uiX] = Clip(pPred[uiX] + pResi[uiX]);
        pRecoPic[uiX] = pReco[uiX];
      }
      pReco    += uiStride;
      pResi    += uiStride;
      pPred    += uiStride;
      pRecoPic += uiReconStride;
    }
  }
  else
  {
    uiCurrDepth++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiPartOffset  = pcCU->getTotalNumPart()>>(uiCurrDepth<<1);
    UInt uiPelOffset   = uiHeight* uiStride;
    uiCoeffOffset >>= 2;
    Pel* pOrg  = piOrg;
    Pel* pResi = piResi;
    Pel* pReco = piReco;
    Pel* pPred = piPred;
    
    xRecurIntraChromaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, eText );
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiWidth; pPred = piPred+uiWidth; pResi = piResi+uiWidth; pReco = piReco+uiWidth;
    piCoeff += uiCoeffOffset;
    xRecurIntraChromaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, eText );
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset; pPred = piPred+uiPelOffset; pResi = piResi+uiPelOffset; pReco = piReco+uiPelOffset;
    piCoeff += uiCoeffOffset;
    xRecurIntraChromaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, eText );
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset+uiWidth; pPred = piPred+uiPelOffset+uiWidth; pResi = piResi+uiPelOffset+uiWidth; pReco = piReco+uiPelOffset+uiWidth;
    piCoeff += uiCoeffOffset;
    xRecurIntraChromaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, eText );
  }
}

// temp buffer for CIP
static Pel iPredOL[ MAX_CU_SIZE*MAX_CU_SIZE ];

#if HHI_AIS
Void TEncSearch::xRecurIntraLumaSearchADI( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiMode, Bool bSmoothing, UInt uiWidth, UInt uiHeight, UInt uiMaxDepth, UInt uiCurrDepth, Bool bAbove, Bool bLeft, Bool bSmallTrs)
#else
Void TEncSearch::xRecurIntraLumaSearchADI( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiMode, UInt uiWidth, UInt uiHeight, UInt uiMaxDepth, UInt uiCurrDepth, Bool bAbove, Bool bLeft, Bool bSmallTrs)
#endif
{
  UInt uiCoeffOffset = uiWidth*uiHeight;
  if( uiMaxDepth == uiCurrDepth )
  {
    UInt uiX, uiY;
    UInt uiZorder = pcCU->getZorderIdxInCU()+uiAbsPartIdx;
    Pel* pOrg  = piOrg;
    Pel* pPred = piPred;
    Pel* pResi = piResi;
    Pel* pReco = piReco;
    Pel* pRecoPic = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZorder);
    UInt uiReconStride = pcCU->getPic()->getPicYuvRec()->getStride();
    UInt uiAbsSum = 0;
    
    if (m_pcEncCfg->getUseRDOQ())
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, uiWidth, TEXT_LUMA );
    
    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;
    
    if (bSmallTrs){
      pcCU->getPattern()->initPattern   ( pcCU, uiCurrDepth, uiAbsPartIdx );
      pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiCurrDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
#ifdef EDGE_BASED_PREDICTION
      if(getEdgeBasedPred()->get_edge_prediction_enable())
        getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiAbsPartIdx, uiCurrDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
    }
    else {
      bAboveAvail=  bAbove;
      bLeftAvail=bLeft;
    }
    
#if ANG_INTRA
    if ( pcCU->angIntraEnabledPredPart( uiAbsPartIdx ) )
#if HHI_AIS
      predIntraLumaAng( pcCU->getPattern(), uiMode, bSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
    predIntraLumaAng( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
    else
#if HHI_AIS
      predIntraLumaAdi( pcCU->getPattern(), uiMode, bSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
    predIntraLumaAdi( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
    
#else // ANG_INTRA
    
#if HHI_AIS
    predIntraLumaAdi( pcCU->getPattern(), uiMode, bSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
    predIntraLumaAdi( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
#endif // ANG_INTRA
    
    // CIP
    if ( pcCU->getCIPflag( uiAbsPartIdx ) )
    {
      // Prediction
      xPredIntraLumaNxNCIPEnc( pcCU->getPattern(), piOrg, piPred, uiStride, iPredOL, uiWidth, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
      
      // Get Residual
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          pResi[uiX] = pOrg[uiX] - CIP_WSUM( pPred[uiX], iPredOL[ uiX+uiY*uiWidth ], CIP_WEIGHT );
        }
        pOrg  += uiStride;
        pResi += uiStride;
        pPred += uiStride;
      }
    }
    else
    {
      // Get Residual
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          pResi[uiX] = pOrg[uiX] - pPred[uiX];
        }
        pOrg  += uiStride;
        pResi += uiStride;
        pPred += uiStride;
      }
    }
    
    pPred = piPred;
    pResi = piResi;
    
    UChar indexROT = pcCU->getROTindex(0);
#if DISABLE_ROT_LUMA_4x4_8x8
    if (uiWidth < 16) indexROT = 0;
#endif
    m_pcTrQuant->transformNxN( pcCU, piResi, uiStride, piCoeff, uiWidth, uiHeight, uiAbsSum, TEXT_LUMA, uiAbsPartIdx, indexROT );
    
    if ( uiAbsSum )
    {
#if QC_MDDT
      m_pcTrQuant->m_bQT = (1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth;
      
      m_pcTrQuant->invtransformNxN( TEXT_LUMA, uiMode, pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
#else
#if DISABLE_ROT_LUMA_4x4_8x8
      if (uiWidth < 16) indexROT = 0;
#endif
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
#endif
    }
    else
    {
      memset(piCoeff,  0, sizeof(TCoeff)*uiCoeffOffset);
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        memset(pResi, 0, sizeof(Pel)*uiWidth);
        pResi += uiStride;
      }
    }
#if QC_MDDT//ADAPTIVE_SCAN
    g_bUpdateStats = false;
#endif
    m_pcEntropyCoder->encodeCoeffNxN( pcCU, piCoeff, uiAbsPartIdx, uiWidth, uiHeight, pcCU->getDepth( 0 )+uiCurrDepth, TEXT_LUMA, true );
    
    pPred = piPred;
    pResi = piResi;
    
    // Reconstruction
    if ( pcCU->getCIPflag( uiAbsPartIdx ) )
    {
      recIntraLumaCIP( pcCU->getPattern(), piPred, piResi, piReco, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
      
      // update to picture
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          pRecoPic[uiX] = pReco[uiX];
        }
        pReco += uiStride;
        pRecoPic += uiReconStride;
      }
    }
    else
    {
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          pReco   [uiX] = Clip(pPred[uiX] + pResi[uiX]);
          pRecoPic[uiX] = pReco[uiX];
        }
        pReco += uiStride;
        pResi += uiStride;
        pPred += uiStride;
        pRecoPic += uiReconStride;
      }
    }
  }
  else
  {
    uiCurrDepth++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    uiCoeffOffset >>= 2;
    UInt uiPartOffset  = pcCU->getTotalNumPart()>>(uiCurrDepth<<1);
    UInt uiPelOffset   = uiHeight*uiStride;
    Pel* pOrg  = piOrg;
    Pel* pResi = piResi;
    Pel* pReco = piReco;
    Pel* pPred = piPred;
#if HHI_AIS
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, bSmoothing, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#else
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#endif
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiWidth; pPred = piPred+uiWidth; pResi = piResi+uiWidth; pReco = piReco+uiWidth;
    piCoeff += uiCoeffOffset;
#if HHI_AIS
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, bSmoothing, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false, true);
#else
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#endif
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset; pPred = piPred+uiPelOffset; pResi = piResi+uiPelOffset; pReco = piReco+uiPelOffset;
    piCoeff += uiCoeffOffset;
#if HHI_AIS
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, bSmoothing, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#else
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#endif
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset+uiWidth; pPred = piPred+uiPelOffset+uiWidth; pResi = piResi+uiPelOffset+uiWidth; pReco = piReco+uiPelOffset+uiWidth;
    piCoeff += uiCoeffOffset;
#if HHI_AIS
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, bSmoothing, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false, true);
#else
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
#endif
  }
}


#if HHI_RQT_INTRA

Void
TEncSearch::xEncSubdivCbfQT( TComDataCU*  pcCU,
                            UInt         uiTrDepth,
                            UInt         uiAbsPartIdx,
                            Bool         bLuma,
                            Bool         bChroma )
{
  UInt  uiFullDepth     = pcCU->getDepth(0) + uiTrDepth;
  UInt  uiTrMode        = pcCU->getTransformIdx( uiAbsPartIdx );
  UInt  uiSubdiv        = ( uiTrMode > uiTrDepth ? 1 : 0 );
  UInt  uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()] + 2 - uiFullDepth;
  
  if( pcCU->getPredictionMode(0) == MODE_INTRA && pcCU->getPartitionSize(0) == SIZE_NxN && uiTrDepth == 0 )
  {
    assert( uiSubdiv );
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    assert( uiSubdiv );
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( !uiSubdiv );
  }
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    assert( !uiSubdiv );
  }
#endif  
  else
  {
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#else 
    assert( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
#endif
    if( bLuma )
    {
      m_pcEntropyCoder->encodeTransformSubdivFlag( uiSubdiv, uiFullDepth );
    }
  }
  
  if( uiSubdiv )
  {
    UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xEncSubdivCbfQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiQPartNum, bLuma, bChroma );
    }
    return;
  }
  
#if LCEC_CBP_YUV_ROOT
  if(pcCU->getSlice()->getSymbolMode())
  {
#endif
  //===== Cbfs =====
  if( bLuma )
  {
    m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA,     uiTrMode );
  }
  if( bChroma )
  {
    Bool bCodeChroma = true;
    if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( uiTrDepth > 0 );
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth - 1 ) << 1 );
      bCodeChroma  = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    }
    if( bCodeChroma )
    {
      m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
      m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
    }
  }
#if LCEC_CBP_YUV_ROOT
  }
#endif
}


Void
TEncSearch::xEncCoeffQT( TComDataCU*  pcCU,
                        UInt         uiTrDepth,
                        UInt         uiAbsPartIdx,
                        TextType     eTextType,
                        Bool         bRealCoeff )
{
  UInt  uiFullDepth     = pcCU->getDepth(0) + uiTrDepth;
  UInt  uiTrMode        = pcCU->getTransformIdx( uiAbsPartIdx );
  UInt  uiSubdiv        = ( uiTrMode > uiTrDepth ? 1 : 0 );
  UInt  uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()] + 2 - uiFullDepth;
  UInt  uiChroma        = ( eTextType != TEXT_LUMA ? 1 : 0 );
  
  if( uiSubdiv )
  {
    UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
#if LCEC_CBP_YUV_ROOT
    if ( pcCU->getSlice()->getSymbolMode() || pcCU->getCbf( uiAbsPartIdx, eTextType, uiTrDepth ))
    {
#endif
#if LCEC_CBP_YUV_ROOT
    if(pcCU->getSlice()->getSymbolMode() == 0)
    {
      if( eTextType == TEXT_LUMA || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
        m_pcEntropyCoder->m_pcEntropyCoderIf->codeBlockCbf(pcCU, uiAbsPartIdx, eTextType, uiTrDepth + 1, uiQPartNum, true);
    }
#endif
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xEncCoeffQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiQPartNum, eTextType, bRealCoeff );
    }
#if LCEC_CBP_YUV_ROOT
    }
#endif
    return;
  }
  
  if( eTextType != TEXT_LUMA && uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( uiTrDepth > 0 );
    uiTrDepth--;
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth ) << 1 );
    Bool bFirstQ = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    if( !bFirstQ )
    {
      return;
    }
  }
  
  //===== coefficients =====
  UInt    uiWidth         = pcCU->getWidth  ( 0 ) >> ( uiTrDepth + uiChroma );
  UInt    uiHeight        = pcCU->getHeight ( 0 ) >> ( uiTrDepth + uiChroma );
  UInt    uiCoeffOffset   = ( pcCU->getPic()->getMinCUWidth() * pcCU->getPic()->getMinCUHeight() * uiAbsPartIdx ) >> ( uiChroma << 1 );
  UInt    uiQTLayer       = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrafoSize;
  TCoeff* pcCoeff         = 0;
  switch( eTextType )
  {
    case TEXT_LUMA:     pcCoeff = ( bRealCoeff ? pcCU->getCoeffY () : m_ppcQTTempCoeffY [uiQTLayer] );  break;
    case TEXT_CHROMA_U: pcCoeff = ( bRealCoeff ? pcCU->getCoeffCb() : m_ppcQTTempCoeffCb[uiQTLayer] );  break;
    case TEXT_CHROMA_V: pcCoeff = ( bRealCoeff ? pcCU->getCoeffCr() : m_ppcQTTempCoeffCr[uiQTLayer] );  break;
    default:            assert(0);
  }
  pcCoeff += uiCoeffOffset;
  
#if QC_MDDT//ADAPTIVE_SCAN
  g_bUpdateStats = false;
#endif
  m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiFullDepth, eTextType, false );
}


Void
TEncSearch::xEncIntraHeader( TComDataCU*  pcCU,
                            UInt         uiTrDepth,
                            UInt         uiAbsPartIdx,
                            Bool         bLuma,
                            Bool         bChroma )
{
  if( bLuma )
  {
    // CU header
    if( uiAbsPartIdx == 0 )
    {
      if( !pcCU->getSlice()->isIntra() )
      {
        m_pcEntropyCoder->encodeSkipFlag( pcCU, 0, true );
#if HHI_MRG && !HHI_MRG_PU
        m_pcEntropyCoder->encodeMergeInfo( pcCU, 0, true );
#endif
        m_pcEntropyCoder->encodePredMode( pcCU, 0, true );
      }
      
#if PLANAR_INTRA
      m_pcEntropyCoder->encodePlanarInfo( pcCU, 0, true );
      
      if ( pcCU->getPlanarInfo(0, PLANAR_FLAG) )
        return;
#endif
      
      m_pcEntropyCoder  ->encodePartSize( pcCU, 0, pcCU->getDepth(0), true );
    }
    // luma prediction mode
    if( pcCU->getPartitionSize(0) == SIZE_2Nx2N )
    {
      if( uiAbsPartIdx == 0 )
      {
        m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, 0 );
#if HHI_AIS
        m_pcEntropyCoder->encodeIntraFiltFlagLuma( pcCU, 0 );
#endif
      }
    }
    else
    {
      UInt uiQNumParts = pcCU->getTotalNumPart() >> 2;
      if( uiTrDepth == 0 )
      {
        assert( uiAbsPartIdx == 0 );
        for( UInt uiPart = 0; uiPart < 4; uiPart++ )
        {
          m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, uiPart * uiQNumParts );
        }
#if HHI_AIS
        for( UInt uiPart = 0; uiPart < 4; uiPart++ )
        {
          m_pcEntropyCoder->encodeIntraFiltFlagLuma( pcCU, uiPart * uiQNumParts );
        }
#endif
      }
      else if( ( uiAbsPartIdx % uiQNumParts ) == 0 )
      {
        m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, uiAbsPartIdx );
#if HHI_AIS
        m_pcEntropyCoder->encodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx );
#endif
      }
    }
  }
  if( bChroma )
  {
    // chroma prediction mode
    if( uiAbsPartIdx == 0 )
    {
      m_pcEntropyCoder->encodeIntraDirModeChroma( pcCU, 0, true );
    }
  }
}


UInt
TEncSearch::xGetIntraBitsQT( TComDataCU*  pcCU,
                            UInt         uiTrDepth,
                            UInt         uiAbsPartIdx,
                            Bool         bLuma,
                            Bool         bChroma,
                            Bool         bRealCoeff /* just for test */ )
{
  m_pcEntropyCoder->resetBits();
  xEncIntraHeader ( pcCU, uiTrDepth, uiAbsPartIdx, bLuma, bChroma );
  xEncSubdivCbfQT ( pcCU, uiTrDepth, uiAbsPartIdx, bLuma, bChroma );

#if LCEC_CBP_YUV_ROOT
  if (pcCU->getSlice()->getSymbolMode()==0 && uiTrDepth == 0)
  {
    if(bLuma && bChroma)
    {
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0 );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
      {
        UInt   uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
        return uiBits;
      }
    }
    else if(bLuma)
    {
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeQtCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0 );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0)
      {
        UInt   uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
        return uiBits;
      }
    }
    else if(bChroma)
    {
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeQtCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0 );
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeQtCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0 );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0 )
      {
        UInt   uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
        return uiBits;
      }
    }
  }
#endif

  if( bLuma )
  {
    xEncCoeffQT   ( pcCU, uiTrDepth, uiAbsPartIdx, TEXT_LUMA,      bRealCoeff );
  }
  if( bChroma )
  {
    xEncCoeffQT   ( pcCU, uiTrDepth, uiAbsPartIdx, TEXT_CHROMA_U,  bRealCoeff );
    xEncCoeffQT   ( pcCU, uiTrDepth, uiAbsPartIdx, TEXT_CHROMA_V,  bRealCoeff );
  }
  UInt   uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
  return uiBits;
}



Void
TEncSearch::xIntraCodingLumaBlk( TComDataCU* pcCU,
                                UInt        uiTrDepth,
                                UInt        uiAbsPartIdx,
                                TComYuv*    pcOrgYuv, 
                                TComYuv*    pcPredYuv, 
                                TComYuv*    pcResiYuv, 
                                UInt&       ruiDist )
{
#if HHI_AIS
  Bool    bIntraSmoothing   = pcCU     ->getLumaIntraFiltFlag( uiAbsPartIdx );
#endif
  UInt    uiLumaPredMode    = pcCU     ->getLumaIntraDir     ( uiAbsPartIdx );
  UInt    uiFullDepth       = pcCU     ->getDepth   ( 0 )  + uiTrDepth;
  UInt    uiWidth           = pcCU     ->getWidth   ( 0 ) >> uiTrDepth;
  UInt    uiHeight          = pcCU     ->getHeight  ( 0 ) >> uiTrDepth;
  UInt    uiStride          = pcOrgYuv ->getStride  ();
  Pel*    piOrg             = pcOrgYuv ->getLumaAddr( uiAbsPartIdx );
  Pel*    piPred            = pcPredYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piResi            = pcResiYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piReco            = pcPredYuv->getLumaAddr( uiAbsPartIdx );
  
  UInt    uiLog2TrSize      = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  UInt    uiQTLayer         = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
  UInt    uiNumCoeffPerInc  = pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
  TCoeff* pcCoeff           = m_ppcQTTempCoeffY[ uiQTLayer ] + uiNumCoeffPerInc * uiAbsPartIdx;
  Pel*    piRecQt           = m_pcQTTempTComYuv[ uiQTLayer ].getLumaAddr( uiAbsPartIdx );
  UInt    uiRecQtStride     = m_pcQTTempTComYuv[ uiQTLayer ].getStride  ();
  
  UInt    uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*    piRecIPred        = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiZOrder );
  UInt    uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getStride  ();
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern   ( pcCU, uiTrDepth, uiAbsPartIdx );
  pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiTrDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail );
#ifdef EDGE_BASED_PREDICTION
  if(getEdgeBasedPred()->get_edge_prediction_enable())
    getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiAbsPartIdx, uiTrDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
  
  //===== get prediction signal =====
#if ANG_INTRA
  if ( pcCU->angIntraEnabledPredPart( uiAbsPartIdx ) )
#if HHI_AIS
    predIntraLumaAng( pcCU->getPattern(), uiLumaPredMode, bIntraSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
  predIntraLumaAng( pcCU->getPattern(), uiLumaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
  else
#if HHI_AIS
    predIntraLumaAdi( pcCU->getPattern(), uiLumaPredMode, bIntraSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
  predIntraLumaAdi( pcCU->getPattern(), uiLumaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
  
#else // ANG_INTRA
  
#if HHI_AIS
  predIntraLumaAdi( pcCU->getPattern(), uiLumaPredMode, bIntraSmoothing, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
  predIntraLumaAdi( pcCU->getPattern(), uiLumaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
#endif // ANG_INTRA
  
  //===== get residual signal =====
  if( pcCU->getCIPflag( uiAbsPartIdx ) )
  {
    // CIP
    Pel aiPredOL[ MAX_CU_SIZE*MAX_CU_SIZE ];
    xPredIntraLumaNxNCIPEnc( pcCU->getPattern(), piOrg, piPred, uiStride, aiPredOL, uiWidth, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    
    // get residual
    Pel*  pOrg    = piOrg;
    Pel*  pPred   = piPred;
    Pel*  pResi   = piResi;
    Pel*  pPredOL = aiPredOL;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pResi[ uiX ] = pOrg[ uiX ] - CIP_WSUM( pPred[ uiX ], pPredOL[ uiX ], CIP_WEIGHT );
      }
      pOrg    += uiStride;
      pPred   += uiStride;
      pResi   += uiStride;
      pPredOL += uiWidth;
    }
  }
  else
  {
    // get residual
    Pel*  pOrg    = piOrg;
    Pel*  pPred   = piPred;
    Pel*  pResi   = piResi;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pResi[ uiX ] = pOrg[ uiX ] - pPred[ uiX ];
      }
      pOrg  += uiStride;
      pResi += uiStride;
      pPred += uiStride;
    }
  }
  
  //===== transform and quantization =====
  //--- init rate estimation arrays for RDOQ ---
  if( m_pcEncCfg->getUseRDOQ() )
  {
    m_pcEntropyCoder->estimateBit( m_pcTrQuant->m_pcEstBitsSbac, uiWidth, TEXT_LUMA );
  }
  //--- transform and quantization ---
  UInt uiAbsSum = 0;
  pcCU       ->setTrIdxSubParts ( uiTrDepth, uiAbsPartIdx, uiFullDepth );
  m_pcTrQuant->setQPforQuant    ( pcCU->getQP( 0 ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
#if DISABLE_ROT_LUMA_4x4_8x8
  m_pcTrQuant->transformNxN     ( pcCU, piResi, uiStride, pcCoeff, uiWidth, uiHeight, uiAbsSum, TEXT_LUMA, uiAbsPartIdx, uiWidth > 8 ? pcCU->getROTindex( 0 ): 0  );
#else
  m_pcTrQuant->transformNxN     ( pcCU, piResi, uiStride, pcCoeff, uiWidth, uiHeight, uiAbsSum, TEXT_LUMA, uiAbsPartIdx, pcCU->getROTindex( 0 ) );
#endif
  //--- set coded block flag ---
  pcCU->setCbfSubParts          ( ( uiAbsSum ? 1 : 0 ) << uiTrDepth, TEXT_LUMA, uiAbsPartIdx, uiFullDepth );
  //--- inverse transform ---
  if( uiAbsSum )
  {
#if QC_MDDT
    m_pcTrQuant->m_bQT = (1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth;
    
    m_pcTrQuant->invtransformNxN( TEXT_LUMA, pcCU->getLumaIntraDir( uiAbsPartIdx ), piResi, uiStride, pcCoeff, uiWidth, uiHeight, pcCU->getROTindex( 0 ) );
#else
#if DISABLE_ROT_LUMA_4x4_8x8
    m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight, uiWidth > 8 ? pcCU->getROTindex( 0 ): 0 );
#else
    m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight, pcCU->getROTindex( 0 ) );
#endif
#endif
  }
  else
  {
    Pel* pResi = piResi;
    memset( pcCoeff, 0, sizeof( TCoeff ) * uiWidth * uiHeight );
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      memset( pResi, 0, sizeof( Pel ) * uiWidth );
      pResi += uiStride;
    }
  }
  
  //===== reconstruction =====
  if( pcCU->getCIPflag( uiAbsPartIdx ) )
  {
    recIntraLumaCIP( pcCU->getPattern(), piPred, piResi, piReco, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    
    Pel* pReco      = piReco;
    Pel* pRecQt     = piRecQt;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pRecQt   [ uiX ] = pReco[ uiX ];
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pReco     += uiStride;
      pRecQt    += uiRecQtStride;
      pRecIPred += uiRecIPredStride;
    }
  }
  else
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecQt     = piRecQt;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecQt   [ uiX ] = pReco[ uiX ];
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecQt    += uiRecQtStride;
      pRecIPred += uiRecIPredStride;
    }
  }
  
  //===== update distortion =====
  ruiDist += m_pcRdCost->getDistPart( piReco, uiStride, piOrg, uiStride, uiWidth, uiHeight );
}


Void
TEncSearch::xIntraCodingChromaBlk( TComDataCU* pcCU,
                                  UInt        uiTrDepth,
                                  UInt        uiAbsPartIdx,
                                  TComYuv*    pcOrgYuv, 
                                  TComYuv*    pcPredYuv, 
                                  TComYuv*    pcResiYuv, 
                                  UInt&       ruiDist,
                                  UInt        uiChromaId )
{
  UInt uiOrgTrDepth = uiTrDepth;
  UInt uiFullDepth  = pcCU->getDepth( 0 ) + uiTrDepth;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( uiTrDepth > 0 );
    uiTrDepth--;
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth ) << 1 );
    Bool bFirstQ = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    if( !bFirstQ )
    {
      return;
    }
  }
  
  TextType  eText             = ( uiChromaId > 0 ? TEXT_CHROMA_V : TEXT_CHROMA_U );
  UInt      uiChromaPredMode  = pcCU     ->getChromaIntraDir( uiAbsPartIdx );
  UInt      uiWidth           = pcCU     ->getWidth   ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiHeight          = pcCU     ->getHeight  ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiStride          = pcOrgYuv ->getCStride ();
  Pel*      piOrg             = ( uiChromaId > 0 ? pcOrgYuv ->getCrAddr( uiAbsPartIdx ) : pcOrgYuv ->getCbAddr( uiAbsPartIdx ) );
  Pel*      piPred            = ( uiChromaId > 0 ? pcPredYuv->getCrAddr( uiAbsPartIdx ) : pcPredYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piResi            = ( uiChromaId > 0 ? pcResiYuv->getCrAddr( uiAbsPartIdx ) : pcResiYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piReco            = ( uiChromaId > 0 ? pcPredYuv->getCrAddr( uiAbsPartIdx ) : pcPredYuv->getCbAddr( uiAbsPartIdx ) );
  
  UInt      uiQTLayer         = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
  UInt      uiNumCoeffPerInc  = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 ) ) >> 2;
  TCoeff*   pcCoeff           = ( uiChromaId > 0 ? m_ppcQTTempCoeffCr[ uiQTLayer ] : m_ppcQTTempCoeffCb[ uiQTLayer ] ) + uiNumCoeffPerInc * uiAbsPartIdx;
  Pel*      piRecQt           = ( uiChromaId > 0 ? m_pcQTTempTComYuv[ uiQTLayer ].getCrAddr( uiAbsPartIdx ) : m_pcQTTempTComYuv[ uiQTLayer ].getCbAddr( uiAbsPartIdx ) );
  UInt      uiRecQtStride     = m_pcQTTempTComYuv[ uiQTLayer ].getCStride();
  
  UInt      uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*      piRecIPred        = ( uiChromaId > 0 ? pcCU->getPic()->getPicYuvRec()->getCrAddr( pcCU->getAddr(), uiZOrder ) : pcCU->getPic()->getPicYuvRec()->getCbAddr( pcCU->getAddr(), uiZOrder ) );
  UInt      uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getCStride();
  
  //===== update chroma mode =====
  if( uiChromaPredMode == 4 )
#if ANG_INTRA
  {
    UInt    uiIntraIdx        = pcCU->getIntraSizeIdx( 0 );
    uiChromaPredMode          = pcCU->angIntraEnabledPredPart( 0 ) ? pcCU->getLumaIntraDir( 0 ) : g_aucIntraModeOrder[ uiIntraIdx ][ pcCU->getLumaIntraDir( 0 ) ];
  }
#else
  {
    UInt    uiIntraIdx        = pcCU->getIntraSizeIdx( 0 );
    uiChromaPredMode          = g_aucIntraModeOrder[ uiIntraIdx ][ pcCU->getLumaIntraDir( 0 ) ];
  }
#endif
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern         ( pcCU, uiTrDepth, uiAbsPartIdx );
  pcCU->getPattern()->initAdiPatternChroma( pcCU, uiAbsPartIdx, uiTrDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail );
  Int*  pPatChroma  = ( uiChromaId > 0 ? pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_piYuvExt ) : pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_piYuvExt ) );
  
  //===== get prediction signal =====
#if ANG_INTRA
  if ( pcCU->angIntraEnabledPredPart( 0 ) )
    predIntraChromaAng( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
  else
    predIntraChromaAdi( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
  predIntraChromaAdi( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
  
  //===== get residual signal =====
  {
    // get residual
    Pel*  pOrg    = piOrg;
    Pel*  pPred   = piPred;
    Pel*  pResi   = piResi;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pResi[ uiX ] = pOrg[ uiX ] - pPred[ uiX ];
      }
      pOrg  += uiStride;
      pResi += uiStride;
      pPred += uiStride;
    }
  }
  
  //===== transform and quantization =====
  {
    //--- init rate estimation arrays for RDOQ ---
    if( m_pcEncCfg->getUseRDOQ() )
    {
      m_pcEntropyCoder->estimateBit( m_pcTrQuant->m_pcEstBitsSbac, uiWidth, eText );
    }
    //--- transform and quantization ---
    UInt uiAbsSum = 0;
    m_pcTrQuant->setQPforQuant     ( pcCU->getQP( 0 ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
    m_pcTrQuant->transformNxN      ( pcCU, piResi, uiStride, pcCoeff, uiWidth, uiHeight, uiAbsSum, eText, uiAbsPartIdx, pcCU->getROTindex( 0 ) );
    //--- set coded block flag ---
    pcCU->setCbfSubParts           ( ( uiAbsSum ? 1 : 0 ) << uiOrgTrDepth, eText, uiAbsPartIdx, pcCU->getDepth(0) + uiTrDepth );
    //--- inverse transform ---
    if( uiAbsSum )
    {
#if QC_MDDT
      m_pcTrQuant->invtransformNxN( TEXT_CHROMA, REG_DCT, piResi, uiStride, pcCoeff, uiWidth, uiHeight, pcCU->getROTindex( 0 ) );
#else
      m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight, pcCU->getROTindex( 0 ) );
#endif
    }
    else
    {
      Pel* pResi = piResi;
      memset( pcCoeff, 0, sizeof( TCoeff ) * uiWidth * uiHeight );
      for( UInt uiY = 0; uiY < uiHeight; uiY++ )
      {
        memset( pResi, 0, sizeof( Pel ) * uiWidth );
        pResi += uiStride;
      }
    }
  }
  
  //===== reconstruction =====
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecQt     = piRecQt;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecQt   [ uiX ] = pReco[ uiX ];
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecQt    += uiRecQtStride;
      pRecIPred += uiRecIPredStride;
    }
  }
  
  //===== update distortion =====
  ruiDist += m_pcRdCost->getDistPart( piReco, uiStride, piOrg, uiStride, uiWidth, uiHeight );
}



Void 
TEncSearch::xRecurIntraCodingQT( TComDataCU*  pcCU, 
                                UInt         uiTrDepth,
                                UInt         uiAbsPartIdx, 
                                Bool         bLumaOnly,
                                TComYuv*     pcOrgYuv, 
                                TComYuv*     pcPredYuv, 
                                TComYuv*     pcResiYuv, 
                                UInt&        ruiDistY,
                                UInt&        ruiDistC,
#if HHI_RQT_INTRA_SPEEDUP
                                Bool         bCheckFirst,
#endif
                                Double&      dRDCost )
{
  UInt    uiFullDepth   = pcCU->getDepth( 0 ) +  uiTrDepth;
  UInt    uiLog2TrSize  = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  Bool    bCheckFull    = ( uiLog2TrSize  <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() );
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  Bool    bCheckSplit   = ( uiLog2TrSize  >  pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#else
  Bool    bCheckSplit   = ( uiLog2TrSize  >  pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
#endif

#if HHI_RQT_INTRA_SPEEDUP
  if( bCheckFirst && bCheckFull )
  {
    bCheckSplit = false;
  }
#endif
  Double  dSingleCost   = MAX_DOUBLE;
  UInt    uiSingleDistY = 0;
  UInt    uiSingleDistC = 0;
  UInt    uiSingleCbfY  = 0;
  UInt    uiSingleCbfU  = 0;
  UInt    uiSingleCbfV  = 0;
  
  if( bCheckFull )
  {
    //----- store original entropy coding status -----
    if( m_bUseSBACRD && bCheckSplit )
    {
      m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_ROOT ] );
    }
    //----- code luma block with given intra prediction mode and store Cbf-----
    dSingleCost   = 0.0;
    xIntraCodingLumaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcOrgYuv, pcPredYuv, pcResiYuv, uiSingleDistY ); 
    if( bCheckSplit )
    {
      uiSingleCbfY = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrDepth );
    }
    //----- code chroma blocks with given intra prediction mode and store Cbf-----
    if( !bLumaOnly )
    {
      xIntraCodingChromaBlk ( pcCU, uiTrDepth, uiAbsPartIdx, pcOrgYuv, pcPredYuv, pcResiYuv, uiSingleDistC, 0 ); 
      xIntraCodingChromaBlk ( pcCU, uiTrDepth, uiAbsPartIdx, pcOrgYuv, pcPredYuv, pcResiYuv, uiSingleDistC, 1 ); 
      if( bCheckSplit )
      {
        uiSingleCbfU = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
        uiSingleCbfV = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
      }
    }
    //----- determine rate and r-d cost -----
    UInt uiSingleBits = xGetIntraBitsQT( pcCU, uiTrDepth, uiAbsPartIdx, true, !bLumaOnly, false );
    dSingleCost       = m_pcRdCost->calcRdCost( uiSingleBits, uiSingleDistY + uiSingleDistC );
  }
  
  if( bCheckSplit )
  {
    //----- store full entropy coding status, load original entropy coding status -----
    if( m_bUseSBACRD )
    {
      if( bCheckFull )
      {
        m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_TEST ] );
        m_pcRDGoOnSbacCoder->load ( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_ROOT ] );
      }
      else
      {
        m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_ROOT ] );
      }
    }
    //----- code splitted block -----
    Double  dSplitCost      = 0.0;
    UInt    uiSplitDistY    = 0;
    UInt    uiSplitDistC    = 0;
    UInt    uiQPartsDiv     = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    UInt    uiAbsPartIdxSub = uiAbsPartIdx;
    for( UInt uiPart = 0; uiPart < 4; uiPart++, uiAbsPartIdxSub += uiQPartsDiv )
    {
#if HHI_RQT_INTRA_SPEEDUP
      xRecurIntraCodingQT( pcCU, uiTrDepth + 1, uiAbsPartIdxSub, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiSplitDistY, uiSplitDistC, bCheckFirst, dSplitCost );
#else
      xRecurIntraCodingQT( pcCU, uiTrDepth + 1, uiAbsPartIdxSub, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiSplitDistY, uiSplitDistC, dSplitCost );
#endif
    }
    //----- restore context states -----
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load ( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_ROOT ] );
    }
    //----- determine rate and r-d cost -----
    UInt uiSplitBits = xGetIntraBitsQT( pcCU, uiTrDepth, uiAbsPartIdx, true, !bLumaOnly, false );
    dSplitCost       = m_pcRdCost->calcRdCost( uiSplitBits, uiSplitDistY + uiSplitDistC );
    
    //===== compare and set best =====
    if( dSplitCost < dSingleCost )
    {
      //--- set luma Cbf values ---
      UInt  uiSplitCbfY = 0;
      uiAbsPartIdxSub   = uiAbsPartIdx;
      for( UInt uiPart  = 0; uiPart < 4; uiPart++, uiAbsPartIdxSub += uiQPartsDiv )
      {
        uiSplitCbfY |= pcCU->getCbf( uiAbsPartIdxSub, TEXT_LUMA, uiTrDepth + 1 );
      }
      for( UInt uiOffs = 0; uiOffs < 4 * uiQPartsDiv; uiOffs++ )
      {
        pcCU->getCbf( TEXT_LUMA )[ uiAbsPartIdx + uiOffs ] |= ( uiSplitCbfY << uiTrDepth );
      }
      //--- set chroma Cbf values ---
      if( !bLumaOnly )
      {
        UInt  uiSplitCbfU = 0;
        UInt  uiSplitCbfV = 0;
        uiAbsPartIdxSub   = uiAbsPartIdx;
        for( UInt uiPart  = 0; uiPart < 4; uiPart++, uiAbsPartIdxSub += uiQPartsDiv )
        {
          uiSplitCbfU |= pcCU->getCbf( uiAbsPartIdxSub, TEXT_CHROMA_U, uiTrDepth + 1 );
          uiSplitCbfV |= pcCU->getCbf( uiAbsPartIdxSub, TEXT_CHROMA_V, uiTrDepth + 1 );
        }
        for( UInt uiOffs = 0; uiOffs < 4 * uiQPartsDiv; uiOffs++ )
        {
          pcCU->getCbf( TEXT_CHROMA_U )[ uiAbsPartIdx + uiOffs ] |= ( uiSplitCbfU << uiTrDepth );
          pcCU->getCbf( TEXT_CHROMA_V )[ uiAbsPartIdx + uiOffs ] |= ( uiSplitCbfV << uiTrDepth );
        }
      }
      //--- update cost ---
      ruiDistY += uiSplitDistY;
      ruiDistC += uiSplitDistC;
      dRDCost  += dSplitCost;
      return;
    }
    //----- set entropy coding status -----
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load ( m_pppcRDSbacCoder[ uiFullDepth ][ CI_QT_TRAFO_TEST ] );
    }
  }
  
  if( bCheckSplit )
  {
    //--- set transform index and Cbf values ---
    pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiFullDepth );
    pcCU->setCbfSubParts  ( uiSingleCbfY << uiTrDepth, TEXT_LUMA, uiAbsPartIdx, uiFullDepth );
    if( !bLumaOnly )
    {
      pcCU->setCbfSubParts( uiSingleCbfU << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiFullDepth );
      pcCU->setCbfSubParts( uiSingleCbfV << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiFullDepth );
    }
    
    //--- set reconstruction for next intra prediction blocks ---
    UInt  uiWidth     = pcCU->getWidth ( 0 ) >> uiTrDepth;
    UInt  uiHeight    = pcCU->getHeight( 0 ) >> uiTrDepth;
    UInt  uiQTLayer   = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    UInt  uiZOrder    = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
    Pel*  piSrc       = m_pcQTTempTComYuv[ uiQTLayer ].getLumaAddr( uiAbsPartIdx );
    UInt  uiSrcStride = m_pcQTTempTComYuv[ uiQTLayer ].getStride  ();
    Pel*  piDes       = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiZOrder );
    UInt  uiDesStride = pcCU->getPic()->getPicYuvRec()->getStride  ();
    for( UInt uiY = 0; uiY < uiHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        piDes[ uiX ] = piSrc[ uiX ];
      }
    }
    if( !bLumaOnly )
    {
      uiWidth   >>= 1;
      uiHeight  >>= 1;
      piSrc       = m_pcQTTempTComYuv[ uiQTLayer ].getCbAddr  ( uiAbsPartIdx );
      uiSrcStride = m_pcQTTempTComYuv[ uiQTLayer ].getCStride ();
      piDes       = pcCU->getPic()->getPicYuvRec()->getCbAddr ( pcCU->getAddr(), uiZOrder );
      uiDesStride = pcCU->getPic()->getPicYuvRec()->getCStride();
      for( UInt uiY = 0; uiY < uiHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
      {
        for( UInt uiX = 0; uiX < uiWidth; uiX++ )
        {
          piDes[ uiX ] = piSrc[ uiX ];
        }
      }
      piSrc       = m_pcQTTempTComYuv[ uiQTLayer ].getCrAddr  ( uiAbsPartIdx );
      piDes       = pcCU->getPic()->getPicYuvRec()->getCrAddr ( pcCU->getAddr(), uiZOrder );
      for( UInt uiY = 0; uiY < uiHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
      {
        for( UInt uiX = 0; uiX < uiWidth; uiX++ )
        {
          piDes[ uiX ] = piSrc[ uiX ];
        }
      }
    }
  }
  ruiDistY += uiSingleDistY;
  ruiDistC += uiSingleDistC;
  dRDCost  += dSingleCost;
}


Void
TEncSearch::xSetIntraResultQT( TComDataCU* pcCU,
                              UInt        uiTrDepth,
                              UInt        uiAbsPartIdx,
                              Bool        bLumaOnly,
                              TComYuv*    pcRecoYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if(  uiTrMode == uiTrDepth )
  {
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
    UInt uiQTLayer    = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    
    Bool bSkipChroma  = false;
    Bool bChromaSame  = false;
    if( !bLumaOnly && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( uiTrDepth > 0 );
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth - 1 ) << 1 );
      bSkipChroma  = ( ( uiAbsPartIdx % uiQPDiv ) != 0 );
      bChromaSame  = true;
    }
    
    //===== copy transform coefficients =====
    UInt uiNumCoeffY    = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( uiFullDepth << 1 );
    UInt uiNumCoeffIncY = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
    TCoeff* pcCoeffSrcY = m_ppcQTTempCoeffY [ uiQTLayer ] + ( uiNumCoeffIncY * uiAbsPartIdx );
    TCoeff* pcCoeffDstY = pcCU->getCoeffY ()              + ( uiNumCoeffIncY * uiAbsPartIdx );
    ::memcpy( pcCoeffDstY, pcCoeffSrcY, sizeof( TCoeff ) * uiNumCoeffY );
    if( !bLumaOnly && !bSkipChroma )
    {
      UInt uiNumCoeffC    = ( bChromaSame ? uiNumCoeffY    : uiNumCoeffY    >> 2 );
      UInt uiNumCoeffIncC = uiNumCoeffIncY >> 2;
      TCoeff* pcCoeffSrcU = m_ppcQTTempCoeffCb[ uiQTLayer ] + ( uiNumCoeffIncC * uiAbsPartIdx );
      TCoeff* pcCoeffSrcV = m_ppcQTTempCoeffCr[ uiQTLayer ] + ( uiNumCoeffIncC * uiAbsPartIdx );
      TCoeff* pcCoeffDstU = pcCU->getCoeffCb()              + ( uiNumCoeffIncC * uiAbsPartIdx );
      TCoeff* pcCoeffDstV = pcCU->getCoeffCr()              + ( uiNumCoeffIncC * uiAbsPartIdx );
      ::memcpy( pcCoeffDstU, pcCoeffSrcU, sizeof( TCoeff ) * uiNumCoeffC );
      ::memcpy( pcCoeffDstV, pcCoeffSrcV, sizeof( TCoeff ) * uiNumCoeffC );
    }
    
    //===== copy reconstruction =====
    m_pcQTTempTComYuv[ uiQTLayer ].copyPartToPartLuma( pcRecoYuv, uiAbsPartIdx, 1 << uiLog2TrSize, 1 << uiLog2TrSize );
    if( !bLumaOnly && !bSkipChroma )
    {
      UInt uiLog2TrSizeChroma = ( bChromaSame ? uiLog2TrSize : uiLog2TrSize - 1 );
      m_pcQTTempTComYuv[ uiQTLayer ].copyPartToPartChroma( pcRecoYuv, uiAbsPartIdx, 1 << uiLog2TrSizeChroma, 1 << uiLog2TrSizeChroma );
    }
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xSetIntraResultQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, bLumaOnly, pcRecoYuv );
    }
  }
}



Void 
TEncSearch::xRecurIntraChromaCodingQT( TComDataCU*  pcCU, 
                                      UInt         uiTrDepth,
                                      UInt         uiAbsPartIdx, 
                                      TComYuv*     pcOrgYuv, 
                                      TComYuv*     pcPredYuv, 
                                      TComYuv*     pcResiYuv, 
                                      UInt&        ruiDist )
{
  UInt uiFullDepth = pcCU->getDepth( 0 ) +  uiTrDepth;
  UInt uiTrMode    = pcCU->getTransformIdx( uiAbsPartIdx );
  if(  uiTrMode == uiTrDepth )
  {
    xIntraCodingChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcOrgYuv, pcPredYuv, pcResiYuv, ruiDist, 0 ); 
    xIntraCodingChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcOrgYuv, pcPredYuv, pcResiYuv, ruiDist, 1 ); 
  }
  else
  {
    UInt uiSplitCbfU     = 0;
    UInt uiSplitCbfV     = 0;
    UInt uiQPartsDiv     = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    UInt uiAbsPartIdxSub = uiAbsPartIdx;
    for( UInt uiPart = 0; uiPart < 4; uiPart++, uiAbsPartIdxSub += uiQPartsDiv )
    {
      xRecurIntraChromaCodingQT( pcCU, uiTrDepth + 1, uiAbsPartIdxSub, pcOrgYuv, pcPredYuv, pcResiYuv, ruiDist );
      uiSplitCbfU |= pcCU->getCbf( uiAbsPartIdxSub, TEXT_CHROMA_U, uiTrDepth + 1 );
      uiSplitCbfV |= pcCU->getCbf( uiAbsPartIdxSub, TEXT_CHROMA_V, uiTrDepth + 1 );
    }
    for( UInt uiOffs = 0; uiOffs < 4 * uiQPartsDiv; uiOffs++ )
    {
      pcCU->getCbf( TEXT_CHROMA_U )[ uiAbsPartIdx + uiOffs ] |= ( uiSplitCbfU << uiTrDepth );
      pcCU->getCbf( TEXT_CHROMA_V )[ uiAbsPartIdx + uiOffs ] |= ( uiSplitCbfV << uiTrDepth );
    }
  }
}

Void
TEncSearch::xSetIntraResultChromaQT( TComDataCU* pcCU,
                                    UInt        uiTrDepth,
                                    UInt        uiAbsPartIdx,
                                    TComYuv*    pcRecoYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if(  uiTrMode == uiTrDepth )
  {
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
    UInt uiQTLayer    = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    
    Bool bChromaSame  = false;
    if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( uiTrDepth > 0 );
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
      {
        return;
      }
      bChromaSame     = true;
    }
    
    //===== copy transform coefficients =====
    UInt uiNumCoeffC    = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( uiFullDepth << 1 );
    if( !bChromaSame )
    {
      uiNumCoeffC     >>= 2;
    }
    UInt uiNumCoeffIncC = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 ) + 2 );
    TCoeff* pcCoeffSrcU = m_ppcQTTempCoeffCb[ uiQTLayer ] + ( uiNumCoeffIncC * uiAbsPartIdx );
    TCoeff* pcCoeffSrcV = m_ppcQTTempCoeffCr[ uiQTLayer ] + ( uiNumCoeffIncC * uiAbsPartIdx );
    TCoeff* pcCoeffDstU = pcCU->getCoeffCb()              + ( uiNumCoeffIncC * uiAbsPartIdx );
    TCoeff* pcCoeffDstV = pcCU->getCoeffCr()              + ( uiNumCoeffIncC * uiAbsPartIdx );
    ::memcpy( pcCoeffDstU, pcCoeffSrcU, sizeof( TCoeff ) * uiNumCoeffC );
    ::memcpy( pcCoeffDstV, pcCoeffSrcV, sizeof( TCoeff ) * uiNumCoeffC );
    
    //===== copy reconstruction =====
    UInt uiLog2TrSizeChroma = ( bChromaSame ? uiLog2TrSize : uiLog2TrSize - 1 );
    m_pcQTTempTComYuv[ uiQTLayer ].copyPartToPartChroma( pcRecoYuv, uiAbsPartIdx, 1 << uiLog2TrSizeChroma, 1 << uiLog2TrSizeChroma );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xSetIntraResultChromaQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv );
    }
  }
}


Void 
TEncSearch::preestChromaPredMode( TComDataCU* pcCU, 
                                 TComYuv*    pcOrgYuv, 
                                 TComYuv*    pcPredYuv )
{
  UInt  uiWidth     = pcCU->getWidth ( 0 ) >> 1;
  UInt  uiHeight    = pcCU->getHeight( 0 ) >> 1;
  UInt  uiStride    = pcOrgYuv ->getCStride();
  Pel*  piOrgU      = pcOrgYuv ->getCbAddr ( 0 );
  Pel*  piOrgV      = pcOrgYuv ->getCrAddr ( 0 );
  Pel*  piPredU     = pcPredYuv->getCbAddr ( 0 );
  Pel*  piPredV     = pcPredYuv->getCrAddr ( 0 );
  
  //===== init pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern         ( pcCU, 0, 0 );
  pcCU->getPattern()->initAdiPatternChroma( pcCU, 0, 0, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail );
  Int*  pPatChromaU = pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_piYuvExt );
  Int*  pPatChromaV = pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_piYuvExt );
  
  //===== get best prediction modes (using SAD) =====
  UInt  uiMinMode   = 0;
  UInt  uiMaxMode   = 4;
  UInt  uiBestMode  = MAX_UINT;
  UInt  uiMinSAD    = MAX_UINT;
  for( UInt uiMode  = uiMinMode; uiMode < uiMaxMode; uiMode++ )
  {
    //--- get prediction ---
#if ANG_INTRA
    if( pcCU->angIntraEnabledPredPart( 0 ) ){
      predIntraChromaAng( pcCU->getPattern(), pPatChromaU, uiMode, piPredU, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
      predIntraChromaAng( pcCU->getPattern(), pPatChromaV, uiMode, piPredV, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    }
    else{
      predIntraChromaAdi( pcCU->getPattern(), pPatChromaU, uiMode, piPredU, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
      predIntraChromaAdi( pcCU->getPattern(), pPatChromaV, uiMode, piPredV, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    }
#else
    predIntraChromaAdi( pcCU->getPattern(), pPatChromaU, uiMode, piPredU, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    predIntraChromaAdi( pcCU->getPattern(), pPatChromaV, uiMode, piPredV, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
    
    //--- get SAD ---
    UInt  uiSAD  = m_pcRdCost->calcHAD( piOrgU, uiStride, piPredU, uiStride, uiWidth, uiHeight );
    uiSAD       += m_pcRdCost->calcHAD( piOrgV, uiStride, piPredV, uiStride, uiWidth, uiHeight );
    //--- check ---
    if( uiSAD < uiMinSAD )
    {
      uiMinSAD   = uiSAD;
      uiBestMode = uiMode;
    }
  }
  
  //===== set chroma pred mode =====
  pcCU->setChromIntraDirSubParts( uiBestMode, 0, pcCU->getDepth( 0 ) );
}

Void 
TEncSearch::estIntraPredQT( TComDataCU* pcCU, 
                           TComYuv*    pcOrgYuv, 
                           TComYuv*    pcPredYuv, 
                           TComYuv*    pcResiYuv, 
                           TComYuv*    pcRecoYuv,
                           UInt&       ruiDistC,
                           Bool        bLumaOnly )
{
  UInt    uiDepth        = pcCU->getDepth(0);
  UInt    uiNumPU        = pcCU->getNumPartInter();
  UInt    uiInitTrDepth  = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
  UInt    uiWidth        = pcCU->getWidth (0) >> uiInitTrDepth;
  UInt    uiHeight       = pcCU->getHeight(0) >> uiInitTrDepth;
  UInt    uiQNumParts    = pcCU->getTotalNumPart() >> 2;
  UInt    uiWidthBit     = pcCU->getIntraSizeIdx(0);
  UInt    uiOverallDistY = 0;
  UInt    uiOverallDistC = 0;
#if HHI_AIS
  Bool    bAISEnabled    = pcCU->getSlice()->getSPS()->getUseAIS();
  Bool    bDefaultIS     = ( bAISEnabled ? true : DEFAULT_IS );
#endif
#if ANG_INTRA
  Bool    angIntraEnabled= pcCU->angIntraEnabledPredPart( 0 );
#endif
#if SAMSUNG_FAST_UDI
  UInt    CandNum;
  UInt    CandModeList[ FAST_UDI_MAX_RDMODE_NUM ];
  Double  CandCostList[ FAST_UDI_MAX_RDMODE_NUM ];
  UInt    uiFastCandNum=g_aucIntraModeNumFast[ uiWidthBit ];
#endif
  
  //===== set QP and clear Cbf =====
  pcCU->setQPSubParts( pcCU->getSlice()->getSliceQp(), 0, uiDepth );
  
  //===== loop over partitions =====
  UInt uiPartOffset = 0;
  for( UInt uiPU = 0; uiPU < uiNumPU; uiPU++, uiPartOffset += uiQNumParts )
  {
    //===== init pattern for luma prediction =====
    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;
    pcCU->getPattern()->initPattern   ( pcCU, uiInitTrDepth, uiPartOffset );
    pcCU->getPattern()->initAdiPattern( pcCU, uiPartOffset, uiInitTrDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail );
#ifdef EDGE_BASED_PREDICTION
    if(getEdgeBasedPred()->get_edge_prediction_enable())
      getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiPartOffset, uiInitTrDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
    
    //===== determine set of modes to be tested (using prediction signal only) =====
#if ANG_INTRA
#if UNIFIED_DIRECTIONAL_INTRA
    UInt uiMaxMode     = angIntraEnabled ? g_aucIntraModeNumAng[uiWidthBit] : g_aucIntraModeNum[uiWidthBit];
#else
    UInt uiMaxMode     = angIntraEnabled ? 34 : g_aucIntraModeNum[uiWidthBit];
#endif
#else
    UInt uiMaxMode     = g_aucIntraModeNum    [ uiWidthBit ];
#endif
    UInt uiMaxModeFast = g_aucIntraModeNumFast[ uiWidthBit ];
    Pel* piOrg         = pcOrgYuv ->getLumaAddr( uiPU, uiWidth );
    Pel* piPred        = pcPredYuv->getLumaAddr( uiPU, uiWidth );
    UInt uiStride      = pcPredYuv->getStride();

#if SAMSUNG_FAST_UDI
    if ( uiFastCandNum != uiMaxMode ) uiMaxModeFast = 0;
    for( Int i=0; i < uiFastCandNum; i++ ) 
    {
      CandCostList[ i ] = MAX_DOUBLE;
    }
	  CandNum = 0;
#else
    UInt uiBestSad     = MAX_UINT;
    UInt iBestPreMode  = 0;
#endif

    for( UInt uiMode = uiMaxModeFast; uiMode < uiMaxMode; uiMode++ )
    {
#if ANG_INTRA
      if ( !predIntraLumaDirAvailable( uiMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
        continue;
      
      if ( angIntraEnabled ){
#if HHI_AIS
        predIntraLumaAng( pcCU->getPattern(), uiMode, bDefaultIS, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
        predIntraLumaAng( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
      }
      else{
#if HHI_AIS
        predIntraLumaAdi( pcCU->getPattern(), uiMode, bDefaultIS, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
        predIntraLumaAdi( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
      }
      
#else // ANG_INTRA
      
      UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiMode];
      if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
      {
        continue;
      }
#if HHI_AIS
      predIntraLumaAdi( pcCU->getPattern(), uiMode, bDefaultIS, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
      predIntraLumaAdi( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
#endif // ANG_INTRA
      
      // use hadamard transform here
      UInt uiSad = m_pcRdCost->calcHAD( piOrg, uiStride, piPred, uiStride, uiWidth, uiHeight );

#if SAMSUNG_FAST_UDI
	    UInt   iModeBits = xModeBitsIntra( pcCU, uiMode, uiPU, uiPartOffset, uiDepth, uiInitTrDepth );
	    Double dModeRate = m_pcRdCost->getLambda();
	    Double uiCost    = (Double)uiSad+(Double)iModeBits*sqrt( dModeRate );

	    CandNum += xUpdateCandList( uiMode, uiCost, uiFastCandNum, CandModeList, CandCostList );
#else  
      if ( uiSad < uiBestSad )
      {
        uiBestSad    = uiSad;
        iBestPreMode = uiMode;
      }
#endif
    }
    UInt uiRdModeList[FAST_UDI_MAX_RDMODE_NUM];
    UInt uiNewMaxMode;
    UInt uiMinMode = 0;

#if SAMSUNG_FAST_UDI
    if(uiFastCandNum!=uiMaxMode)
    {
      uiNewMaxMode = Min( uiFastCandNum, CandNum );
      for( Int i=0; i < uiNewMaxMode; i++) uiRdModeList[i] = CandModeList[i];
    }
    else
    {
      uiNewMaxMode = uiMaxMode;
      for( Int i=0; i < uiNewMaxMode; i++) uiRdModeList[i] = i;
    }
#else

	for( Int i = 0; i < FAST_UDI_MAX_RDMODE_NUM; i++ ) uiRdModeList[ i ] = i;
	if( uiMaxModeFast >= uiMaxMode )
		uiNewMaxMode = uiMaxMode;
    else
    {
      uiNewMaxMode = uiMaxModeFast + 1;
      uiRdModeList[uiMaxModeFast] = iBestPreMode;
    }
    
#endif
    
    //===== check modes (using r-d costs) =====
#if HHI_RQT_INTRA_SPEEDUP_MOD
    UInt   uiSecondBestMode  = MAX_UINT;
#if HHI_AIS
    Bool   bSecondBestISMode = bDefaultIS;
#endif
    Double dSecondBestPUCost = MAX_DOUBLE;
#endif

#if HHI_AIS
    Bool    bBestISMode   = bDefaultIS;
#endif
    UInt    uiBestPUMode  = 0;
    UInt    uiBestPUDistY = 0;
    UInt    uiBestPUDistC = 0;
    Double  dBestPUCost   = MAX_DOUBLE;
    for( UInt uiMode = uiMinMode; uiMode < uiNewMaxMode; uiMode++ )
    {
      // set luma prediction mode
      UInt uiOrgMode = uiRdModeList[uiMode];
      
#if ANG_INTRA
      if ( !predIntraLumaDirAvailable( uiOrgMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
        continue;
#else
      UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiOrgMode];
      if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
      {
        continue;
      }
#endif
      
      pcCU->setLumaIntraDirSubParts ( uiOrgMode, uiPartOffset, uiDepth + uiInitTrDepth );
      
#if HHI_AIS
      // set intra smoothing mode
      pcCU->setLumaIntraFiltFlagSubParts( bBestISMode, uiPartOffset, uiDepth + uiInitTrDepth );
#endif
      
      // set context models
      if( m_bUseSBACRD )
      {
        if( uiPU )  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
        else        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth  ][CI_CURR_BEST] );
      }
      
      // determine residual for partition
      UInt   uiPUDistY = 0;
      UInt   uiPUDistC = 0;
      Double dPUCost   = 0.0;
#if HHI_RQT_INTRA_SPEEDUP
      xRecurIntraCodingQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiPUDistY, uiPUDistC, true, dPUCost );
#else
      xRecurIntraCodingQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiPUDistY, uiPUDistC, dPUCost );
#endif
      
      // check r-d cost
      if( dPUCost < dBestPUCost )
      {
#if HHI_RQT_INTRA_SPEEDUP_MOD
        uiSecondBestMode  = uiBestPUMode;
        dSecondBestPUCost = dBestPUCost;
#endif
        uiBestPUMode  = uiOrgMode;
        uiBestPUDistY = uiPUDistY;
        uiBestPUDistC = uiPUDistC;
        dBestPUCost   = dPUCost;
        
        xSetIntraResultQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcRecoYuv );
        
        UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth(0) + uiInitTrDepth ) << 1 );
        ::memcpy( m_puhQTTempTrIdx,  pcCU->getTransformIdx()       + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[0], pcCU->getCbf( TEXT_LUMA     ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[1], pcCU->getCbf( TEXT_CHROMA_U ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[2], pcCU->getCbf( TEXT_CHROMA_V ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        
        if( m_bUseSBACRD )
        {
          m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
        }
      }
#if HHI_RQT_INTRA_SPEEDUP_MOD
      else if( dPUCost < dSecondBestPUCost )
      {
        uiSecondBestMode  = uiOrgMode;
        dSecondBestPUCost = dPUCost;
      }
#endif
    } // Mode loop
    
    
#if HHI_AIS
    //===== test all or selected modes with modified intra smoothing (execpt intra DC) =====
    if( bAISEnabled )
    {
      Bool bTestISMode = !bBestISMode;
      for( UInt uiMode = uiMinMode; uiMode < uiNewMaxMode; uiMode++ )
      {
        if( uiRdModeList[uiMode] == 2 )
        {
          continue;
        }
#if AIS_TEST_BEST
        if( uiRdModeList[uiMode] != uiBestPUMode )
        {
          continue;
        }
#endif
        
        // set luma prediction mode
        UInt uiOrgMode = uiRdModeList[uiMode];
        
#if ANG_INTRA
        if ( !predIntraLumaDirAvailable( uiOrgMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
          continue;
#else
        UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiOrgMode];
        if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
        {
          continue;
        }
#endif
        
        pcCU->setLumaIntraDirSubParts ( uiOrgMode, uiPartOffset, uiDepth + uiInitTrDepth );
        
        // set intra smoothing mode
        pcCU->setLumaIntraFiltFlagSubParts( bTestISMode, uiPartOffset, uiDepth + uiInitTrDepth );
        
        // set context models
        if( m_bUseSBACRD )
        {
          if( uiPU )  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
          else        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth  ][CI_CURR_BEST] );
        }
        
        // determine residual for partition
        UInt   uiPUDistY = 0;
        UInt   uiPUDistC = 0;
        Double dPUCost   = 0.0;
#if HHI_RQT_INTRA_SPEEDUP
        xRecurIntraCodingQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiPUDistY, uiPUDistC, true, dPUCost );
#else
        xRecurIntraCodingQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiPUDistY, uiPUDistC, dPUCost );
#endif
        // check r-d cost
        if( dPUCost < dBestPUCost )
        {
#if HHI_RQT_INTRA_SPEEDUP_MOD
          uiSecondBestMode  = uiBestPUMode;
          bSecondBestISMode = bBestISMode;
          dSecondBestPUCost = dBestPUCost;
#endif
          bBestISMode   = bTestISMode;
          uiBestPUMode  = uiOrgMode;
          uiBestPUDistY = uiPUDistY;
          uiBestPUDistC = uiPUDistC;
          dBestPUCost   = dPUCost;
          
          xSetIntraResultQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcRecoYuv );
          
          UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth(0) + uiInitTrDepth ) << 1 );
          ::memcpy( m_puhQTTempTrIdx,  pcCU->getTransformIdx()       + uiPartOffset, uiQPartNum * sizeof( UChar ) );
          ::memcpy( m_puhQTTempCbf[0], pcCU->getCbf( TEXT_LUMA     ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
          ::memcpy( m_puhQTTempCbf[1], pcCU->getCbf( TEXT_CHROMA_U ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
          ::memcpy( m_puhQTTempCbf[2], pcCU->getCbf( TEXT_CHROMA_V ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
          
          if( m_bUseSBACRD )
          {
            m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
          }
        }
#if HHI_RQT_INTRA_SPEEDUP_MOD
        else if( dPUCost < dSecondBestPUCost )
        {
          uiSecondBestMode  = uiOrgMode;
          bSecondBestISMode = bTestISMode;
          dSecondBestPUCost = dPUCost;
        }
#endif
      } // Mode loop
    } // AIS enabled
#endif
    
#if HHI_RQT_INTRA_SPEEDUP
#if HHI_RQT_INTRA_SPEEDUP_MOD
    for( UInt ui =0; ui < 2; ++ui )
#endif
    {
#if HHI_RQT_INTRA_SPEEDUP_MOD
#if HHI_AIS
      Bool bTestISMode = ui ? bSecondBestISMode : bBestISMode;
#endif
      UInt uiOrgMode   = ui ? uiSecondBestMode  : uiBestPUMode;
      if( uiOrgMode == MAX_UINT )
      {
        break;
      }
#else
#if HHI_AIS
      Bool bTestISMode = bBestISMode;
#endif
      UInt uiOrgMode = uiBestPUMode;
#endif

#if ANG_INTRA
      if ( !predIntraLumaDirAvailable( uiOrgMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
        continue;
#else
      UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiOrgMode];
      if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
      {
        continue;
      }
#endif
      
      pcCU->setLumaIntraDirSubParts ( uiOrgMode, uiPartOffset, uiDepth + uiInitTrDepth );
      
#if HHI_AIS
      // set intra smoothing mode
      pcCU->setLumaIntraFiltFlagSubParts( bTestISMode, uiPartOffset, uiDepth + uiInitTrDepth );
#endif
      
      // set context models
      if( m_bUseSBACRD )
      {
        if( uiPU )  m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
        else        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth  ][CI_CURR_BEST] );
      }
      
      // determine residual for partition
      UInt   uiPUDistY = 0;
      UInt   uiPUDistC = 0;
      Double dPUCost   = 0.0;
      xRecurIntraCodingQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcOrgYuv, pcPredYuv, pcResiYuv, uiPUDistY, uiPUDistC, false, dPUCost );
      
      // check r-d cost
      if( dPUCost < dBestPUCost )
      {
#if HHI_AIS
        bBestISMode   = bTestISMode;
#endif
        uiBestPUMode  = uiOrgMode;
        uiBestPUDistY = uiPUDistY;
        uiBestPUDistC = uiPUDistC;
        dBestPUCost   = dPUCost;
        
        xSetIntraResultQT( pcCU, uiInitTrDepth, uiPartOffset, bLumaOnly, pcRecoYuv );
        
        UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth(0) + uiInitTrDepth ) << 1 );
        ::memcpy( m_puhQTTempTrIdx,  pcCU->getTransformIdx()       + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[0], pcCU->getCbf( TEXT_LUMA     ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[1], pcCU->getCbf( TEXT_CHROMA_U ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        ::memcpy( m_puhQTTempCbf[2], pcCU->getCbf( TEXT_CHROMA_V ) + uiPartOffset, uiQPartNum * sizeof( UChar ) );
        
        if( m_bUseSBACRD )
        {
          m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
        }
      }
    } // Mode loop
#endif

    //--- update overall distortion ---
    uiOverallDistY += uiBestPUDistY;
    uiOverallDistC += uiBestPUDistC;
    
    //--- update transform index and cbf ---
    UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth(0) + uiInitTrDepth ) << 1 );
    ::memcpy( pcCU->getTransformIdx()       + uiPartOffset, m_puhQTTempTrIdx,  uiQPartNum * sizeof( UChar ) );
    ::memcpy( pcCU->getCbf( TEXT_LUMA     ) + uiPartOffset, m_puhQTTempCbf[0], uiQPartNum * sizeof( UChar ) );
    ::memcpy( pcCU->getCbf( TEXT_CHROMA_U ) + uiPartOffset, m_puhQTTempCbf[1], uiQPartNum * sizeof( UChar ) );
    ::memcpy( pcCU->getCbf( TEXT_CHROMA_V ) + uiPartOffset, m_puhQTTempCbf[2], uiQPartNum * sizeof( UChar ) );
    
    //--- set reconstruction for next intra prediction blocks ---
    if( uiPU != uiNumPU - 1 )
    {
      Bool bSkipChroma  = false;
      Bool bChromaSame  = false;
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> ( pcCU->getDepth(0) + uiInitTrDepth ) ] + 2;
      if( !bLumaOnly && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        assert( uiInitTrDepth  > 0 );
        bSkipChroma  = ( uiPU != 0 );
        bChromaSame  = true;
      }
      
      UInt    uiCompWidth   = pcCU->getWidth ( 0 ) >> uiInitTrDepth;
      UInt    uiCompHeight  = pcCU->getHeight( 0 ) >> uiInitTrDepth;
      UInt    uiZOrder      = pcCU->getZorderIdxInCU() + uiPartOffset;
      Pel*    piDes         = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiZOrder );
      UInt    uiDesStride   = pcCU->getPic()->getPicYuvRec()->getStride();
      Pel*    piSrc         = pcRecoYuv->getLumaAddr( uiPartOffset );
      UInt    uiSrcStride   = pcRecoYuv->getStride();
      for( UInt uiY = 0; uiY < uiCompHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
      {
        for( UInt uiX = 0; uiX < uiCompWidth; uiX++ )
        {
          piDes[ uiX ] = piSrc[ uiX ];
        }
      }
      if( !bLumaOnly && !bSkipChroma )
      {
        if( !bChromaSame )
        {
          uiCompWidth   >>= 1;
          uiCompHeight  >>= 1;
        }
        piDes         = pcCU->getPic()->getPicYuvRec()->getCbAddr( pcCU->getAddr(), uiZOrder );
        uiDesStride   = pcCU->getPic()->getPicYuvRec()->getCStride();
        piSrc         = pcRecoYuv->getCbAddr( uiPartOffset );
        uiSrcStride   = pcRecoYuv->getCStride();
        for( UInt uiY = 0; uiY < uiCompHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
        {
          for( UInt uiX = 0; uiX < uiCompWidth; uiX++ )
          {
            piDes[ uiX ] = piSrc[ uiX ];
          }
        }
        piDes         = pcCU->getPic()->getPicYuvRec()->getCrAddr( pcCU->getAddr(), uiZOrder );
        piSrc         = pcRecoYuv->getCrAddr( uiPartOffset );
        for( UInt uiY = 0; uiY < uiCompHeight; uiY++, piSrc += uiSrcStride, piDes += uiDesStride )
        {
          for( UInt uiX = 0; uiX < uiCompWidth; uiX++ )
          {
            piDes[ uiX ] = piSrc[ uiX ];
          }
        }
      }
    }
    
    //=== update PU data ====
#if HHI_AIS
    pcCU->setLumaIntraFiltFlagSubParts( bBestISMode,  uiPartOffset, uiDepth + uiInitTrDepth );
#endif
    pcCU->setLumaIntraDirSubParts     ( uiBestPUMode, uiPartOffset, uiDepth + uiInitTrDepth );
    pcCU->copyToPic                   ( uiDepth, uiPU, uiInitTrDepth );
  } // PU loop
  
  
  if( uiNumPU > 1 )
  { // set Cbf for all blocks
    UInt uiCombCbfY = 0;
    UInt uiCombCbfU = 0;
    UInt uiCombCbfV = 0;
    UInt uiPartIdx  = 0;
    for( UInt uiPart = 0; uiPart < 4; uiPart++, uiPartIdx += uiQNumParts )
    {
      uiCombCbfY |= pcCU->getCbf( uiPartIdx, TEXT_LUMA,     1 );
      uiCombCbfU |= pcCU->getCbf( uiPartIdx, TEXT_CHROMA_U, 1 );
      uiCombCbfV |= pcCU->getCbf( uiPartIdx, TEXT_CHROMA_V, 1 );
    }
    for( UInt uiOffs = 0; uiOffs < 4 * uiQNumParts; uiOffs++ )
    {
      pcCU->getCbf( TEXT_LUMA     )[ uiOffs ] |= uiCombCbfY;
      pcCU->getCbf( TEXT_CHROMA_U )[ uiOffs ] |= uiCombCbfU;
      pcCU->getCbf( TEXT_CHROMA_V )[ uiOffs ] |= uiCombCbfV;
    }
  }
  
  //===== reset context models =====
  if(m_bUseSBACRD)
  {
    m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
  }
  
  //===== set distortion (rate and r-d costs are determined later) =====
  ruiDistC                   = uiOverallDistC;
  pcCU->getTotalDistortion() = uiOverallDistY + uiOverallDistC;
}



Void 
TEncSearch::estIntraPredChromaQT( TComDataCU* pcCU, 
                                 TComYuv*    pcOrgYuv, 
                                 TComYuv*    pcPredYuv, 
                                 TComYuv*    pcResiYuv, 
                                 TComYuv*    pcRecoYuv,
                                 UInt        uiPreCalcDistC )
{
  UInt    uiDepth     = pcCU->getDepth(0);
  UInt    uiBestMode  = 0;
  UInt    uiBestDist  = 0;
  Double  dBestCost   = MAX_DOUBLE;
  
  //----- init mode list -----
  Int   iIntraIdx = pcCU->getIntraSizeIdx(0);
  UInt  uiModeList[5];
  for( Int i = 0; i < 4; i++ )
  {
    uiModeList[i] = i;
  }
  
#if ANG_INTRA
  uiModeList[4]   = pcCU->angIntraEnabledPredPart( 0 ) ? pcCU->getLumaIntraDir(0) : g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir(0)];
#else
  uiModeList[4]   = g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir(0)];
#endif
  
  UInt  uiMinMode = 0;
  UInt  uiMaxMode = ( uiModeList[4] >= 4 ? 5 : 4 );
  
  //----- check chroma modes -----
  for( UInt uiMode = uiMinMode; uiMode < uiMaxMode; uiMode++ )
  {
    //----- restore context models -----
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST] );
    }
    
    //----- chroma coding -----
    UInt    uiDist = 0;
    pcCU->setChromIntraDirSubParts  ( uiMode, 0, uiDepth );
    xRecurIntraChromaCodingQT       ( pcCU,   0, 0, pcOrgYuv, pcPredYuv, pcResiYuv, uiDist );
    UInt    uiBits = xGetIntraBitsQT( pcCU,   0, 0, false, true, false );
    Double  dCost  = m_pcRdCost->calcRdCost( uiBits, uiDist );
    
    //----- compare -----
    if( dCost < dBestCost )
    {
      dBestCost   = dCost;
      uiBestDist  = uiDist;
      uiBestMode  = uiMode;
      UInt  uiQPN = pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 );
      xSetIntraResultChromaQT( pcCU, 0, 0, pcRecoYuv );
      ::memcpy( m_puhQTTempCbf[1], pcCU->getCbf( TEXT_CHROMA_U ), uiQPN * sizeof( UChar ) );
      ::memcpy( m_puhQTTempCbf[2], pcCU->getCbf( TEXT_CHROMA_V ), uiQPN * sizeof( UChar ) );
    }
  }
  
  //----- set data -----
  UInt  uiQPN = pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 );
  ::memcpy( pcCU->getCbf( TEXT_CHROMA_U ), m_puhQTTempCbf[1], uiQPN * sizeof( UChar ) );
  ::memcpy( pcCU->getCbf( TEXT_CHROMA_V ), m_puhQTTempCbf[2], uiQPN * sizeof( UChar ) );
  pcCU->setChromIntraDirSubParts( uiBestMode, 0, uiDepth );
  pcCU->getTotalDistortion      () += uiBestDist - uiPreCalcDistC;
  
  //----- restore context models -----
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST] );
  }
}


#endif

#if ANG_INTRA
Bool TEncSearch::predIntraLumaDirAvailable( UInt uiMode, UInt uiWidthBit, Bool angIntraEnabled, Bool bAboveAvail, Bool bLeftAvail)
{
  Bool bDirAvailable = true;
  UInt uiNewMode     = angIntraEnabled ? g_aucAngIntraModeOrder[uiMode] : g_aucIntraModeOrder[uiWidthBit][uiMode];
  
  if ( angIntraEnabled ){
    if ( uiNewMode > 0 && ( (!bAboveAvail) && uiNewMode < 18 ) || ( (!bLeftAvail) && uiNewMode > 17 ) )
      bDirAvailable = false;
  }
  else{
    if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
      bDirAvailable = false;
  }
  
  return bDirAvailable;
}
#endif

#if PLANAR_INTRA
Void TEncSearch::xIntraPlanarRecon( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiWidth, UInt uiHeight, UInt uiCurrDepth, TextType eText )
{
  UInt uiX, uiY;
  UInt uiReconStride;
  Int  iSample;
  
  Pel* pOrg  = piOrg;
  Pel* pPred = piPred;
  Pel* pResi = piResi;
  Pel* pReco = piReco;
  Pel* pRecoPic;
  Int* pPat;
  
  pcCU->getPattern()->initPattern( pcCU, uiCurrDepth, uiAbsPartIdx );
  
  Bool bAboveAvail = false;
  Bool bLeftAvail  = false;
  
  if( eText == TEXT_LUMA)
  {
    pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiCurrDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
    
    pPat          = pcCU->getPattern()->getAdiOrgBuf( uiWidth, uiHeight, m_piYuvExt );
    uiReconStride = pcCU->getPic()->getPicYuvRec()->getStride();
    pRecoPic      = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiAbsPartIdx);
  }
  else
  {
    
#if HHI_RQT_INTRA
    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, 0, m_piYuvExt,m_iYuvExtStride,m_iYuvExtHeight,bAboveAvail,bLeftAvail);
#else
    pcCU->getPattern()->initAdiPatternChroma(pcCU, uiAbsPartIdx, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
#endif
    
    uiReconStride = pcCU->getPic()->getPicYuvRec()->getCStride();
    
    if( eText == TEXT_CHROMA_U )
    {
      pRecoPic = pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiAbsPartIdx);
      pPat     = pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_piYuvExt );
    }
    else
    {
      pRecoPic = pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiAbsPartIdx);
      pPat     = pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_piYuvExt );
    }
  }
  
  // Get the sample value for the bottom-right corner
  iSample = ( piOrg[(uiHeight-1)*uiStride + uiWidth - 1] +
             piOrg[(uiHeight-2)*uiStride + uiWidth - 1] +
             piOrg[(uiHeight-1)*uiStride + uiWidth - 2] +
             piOrg[(uiHeight-2)*uiStride + uiWidth - 2] + 2 ) >> 2;
  
  // Get prediction for the bottom-right sample value
  Int iPredBufStride = ( uiWidth<<1 ) + 1;
  Int iSamplePred    = predIntraGetPredValDC(pPat+iPredBufStride+1, iPredBufStride, uiWidth, uiHeight, bAboveAvail, bLeftAvail );
  Int iDelta         = iSample - iSamplePred;
  Int iSign          = iDelta < 0 ? -1 : 1;
  iDelta             = abs(iDelta) >> g_uiBitIncrement;
  
  // Quantize the difference value
  if( iDelta < 4 )
    iDelta = iDelta;
  else if( iDelta < 16 )
    iDelta = (iDelta>>1)<<1;
  else if( iDelta < 64 )
    iDelta = ((iDelta>>2)<<2)+2;
  else
    iDelta = ((iDelta>>3)<<3)+4;
  
  iDelta *= iSign;
  
  // Intermediate value to be passed to entropy coding (would be better to have here a continuous index instead and avoid replicating quantization steps in the entropy coder)
  if( eText == TEXT_LUMA)
    pcCU->setPlanarInfo( uiAbsPartIdx, PLANAR_DELTAY, iDelta );
  else if( eText == TEXT_CHROMA_U)
    pcCU->setPlanarInfo( uiAbsPartIdx, PLANAR_DELTAU, iDelta );
  else
    pcCU->setPlanarInfo( uiAbsPartIdx, PLANAR_DELTAV, iDelta );
  
  // Reconstructed sample value
  iDelta  = iDelta << g_uiBitIncrement;
  iSample = iDelta + iSamplePred;
  
  predIntraPlanar( pPat, iSample, pPred, uiStride, uiWidth, uiHeight, bAboveAvail, bLeftAvail );
  
  // Make residual from prediction. (MUST BE FIXED FOR EACH TRANSFORM UNIT PREDICTION)
  for( uiY = 0; uiY < uiHeight; uiY++ )
  {
    for( uiX = 0; uiX < uiWidth; uiX++ )
    {
      pResi[uiX] = pOrg[uiX] - pPred[uiX];
    }
    pOrg  += uiStride;
    pResi += uiStride;
    pPred += uiStride;
  }
  
  pPred = piPred;
  pResi = piResi;
  
  //  m_pcTrQuant->transformNxN( pcCU, pResi, uiStride, piCoeff, uiWidth, uiHeight, uiAbsSum, eText, uiAbsPartIdx, indexROT );
  
  for( uiY = 0; uiY < uiHeight; uiY++ )
  {
    memset(pResi, 0, sizeof(Pel)*uiWidth);
    pResi += uiStride;
  }
  
  pPred = piPred;
  pResi = piResi;
  
  // Reconstruction
  for( uiY = 0; uiY < uiHeight; uiY++ )
  {
    for( uiX = 0; uiX < uiWidth; uiX++ )
    {
      pReco   [uiX] = Clip(pPred[uiX]);
      pRecoPic[uiX] = pReco[uiX];
    }
    pReco    += uiStride;
    pResi    += uiStride;
    pPred    += uiStride;
    pRecoPic += uiReconStride;
  }
}

/// encoder estimation - planar intra prediction (luma & chroma)
Void TEncSearch::predIntraPlanarSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv )
{
  UInt   uiDepth        = pcCU->getDepth(0);
  UInt   uiWidth        = pcCU->getWidth(0);
  UInt   uiHeight       = pcCU->getHeight(0);
  UInt   uiStride       = rpcPredYuv->getStride();
  UInt   uiStrideC      = rpcPredYuv->getCStride();
  UInt   uiWidthC       = uiWidth  >> 1;
  UInt   uiHeightC      = uiHeight >> 1;
  UInt   uiBits;
  UInt   uiDistortion;
  
  Double dCost;
  
  Pel*    pOrig;
  Pel*    pResi;
  Pel*    pReco;
  Pel*    pPred;
  TCoeff* pCoeff;
  
  // Planar for Luminance
  pOrig    = pcOrgYuv->getLumaAddr(0, uiWidth);
  pResi    = rpcResiYuv->getLumaAddr(0, uiWidth);
  pPred    = rpcPredYuv->getLumaAddr(0, uiWidth);
  pReco    = rpcRecoYuv->getLumaAddr(0, uiWidth);
  pCoeff   = pcCU->getCoeffY();
  
  xIntraPlanarRecon( pcCU, 0, pOrig, pPred, pResi, pReco, uiStride, pCoeff, uiWidth, uiHeight, 0, TEXT_LUMA );
  
  uiDistortion  = m_pcRdCost->getDistPart( pReco, uiStride, pOrig, uiStride, uiWidth, uiHeight );
  
  // Planar for U
  pOrig    = pcOrgYuv->getCbAddr();
  pResi    = rpcResiYuv->getCbAddr();
  pPred    = rpcPredYuv->getCbAddr();
  pReco    = rpcRecoYuv->getCbAddr();
  pCoeff   = pcCU->getCoeffCb();
  
  xIntraPlanarRecon( pcCU, 0, pOrig, pPred, pResi, pReco, uiStrideC, pCoeff, uiWidthC, uiHeightC, 0, TEXT_CHROMA_U );
  
  uiDistortion += m_pcRdCost->getDistPart( pReco, uiStrideC, pOrig, uiStrideC, uiWidthC, uiHeightC );
  
  // Planar for V
  pOrig    = pcOrgYuv->getCrAddr();
  pResi    = rpcResiYuv->getCrAddr();
  pPred    = rpcPredYuv->getCrAddr();
  pReco    = rpcRecoYuv->getCrAddr();
  pCoeff   = pcCU->getCoeffCr();
  
  xIntraPlanarRecon( pcCU, 0, pOrig, pPred, pResi, pReco, uiStrideC, pCoeff, uiWidthC, uiHeightC, 0, TEXT_CHROMA_V );
  
  uiDistortion += m_pcRdCost->getDistPart( pReco, uiStrideC, pOrig, uiStrideC, uiWidthC, uiHeightC );
  
  xAddSymbolBitsIntra( pcCU, pCoeff, 0, 0, 0, 1, 0, 0, uiWidth, uiHeight, uiBits );
  
  dCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
  
  if(m_bUseSBACRD)
    m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
  
  pcCU->getTotalBits()       = uiBits;
  pcCU->getTotalCost()       = dCost;
  pcCU->getTotalDistortion() = uiDistortion;
  
  pcCU->copyToPic(uiDepth, 0, 0);
  
}
#endif




Void TEncSearch::predIntraLumaAdiSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv )
{
  UInt   uiDepth        = pcCU->getDepth(0);
  UInt   uiNumPU        = pcCU->getNumPartInter();
  UInt   uiPartDepth    = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
  UInt   uiWidth        = pcCU->getWidth(0) >> uiPartDepth;
  UInt   uiHeight       = pcCU->getHeight(0)>> uiPartDepth;
  UInt   uiCoeffSize    = uiWidth*uiHeight;
  
  UInt   uiWidthBit     = pcCU->getIntraSizeIdx(0);
  
  UInt   uiNextDepth    = uiDepth + 1;
  
  UInt   uiQNumParts    = pcCU->getTotalNumPart()>>2;
  UInt   uiPU;
  
  UInt uiBestBits       = 0;
  UInt uiPUBestBits     = 0;
  UInt uiBits;
  
  UInt uiBestDistortion   = 0;
  UInt uiPUBestDistortion = 0;
  UInt uiDistortion;
  
  Double dBestCost      = MAX_DOUBLE;
  Double dPUBestCost    = MAX_DOUBLE;
  Double dCost;
  
  UInt uiTrLevel = 0;
  
  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(0)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
  
  UInt uiMaxTrDepth;
  
  uiMaxTrDepth   = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel + (pcCU->getPartitionSize(0) == SIZE_NxN ? 1 : 0);
  
  UInt   uiMinMode      = 0;
  UInt   uiMaxMode      = 1;
  UInt   uiPUBestMode   = 2;
  UInt   uiMode;
  UInt   uiPUMode[4];
  UInt   uiBestMode[4];
  
#if HHI_AIS
  // BB: AIS adaptive intra smoothing (filtering)
  Bool   bUseAIS = pcCU->getSlice()->getSPS()->getUseAIS();
  Bool   bPUBestFilt = bUseAIS ? true : DEFAULT_IS;
  Bool   bPUCurrFilt = bPUBestFilt;
  Bool   bPUFilt[4];    // BB: best per PU
  Bool   bBestFilt[4];  // BB: best per PU in dQp loop (currently the same)
  Double dPUNoFiltCost;
  UInt   uiPUNoFiltBits;
  UInt   uiPUNoFiltDistortion;
#endif
  
  Int    iMindQp        = 0;
  Int    iMaxdQp        = 0;
  Int    idQp;
  Int    iBestdQp       = 0;
  
  UInt   uiPartOffset;
  UInt   uiCoeffOffset;
  
  Pel*    pOrg;
  Pel*    pResi;
  Pel*    pReco;
  Pel*    pPred;
  TCoeff* pCoeff;
  
  UInt    uiStride  = rpcPredYuv->getStride();
  
  TComPattern* pcPattern  = pcCU->getPattern();
  
#if ANG_INTRA
  Bool angIntraEnabled    = pcCU->angIntraEnabledPredPart( 0 );
#endif
  
#if SAMSUNG_FAST_UDI
  UInt    CandNum;
  UInt    CandModeList[ FAST_UDI_MAX_RDMODE_NUM ];
  Double  CandCostList[ FAST_UDI_MAX_RDMODE_NUM ];
  UInt    uiFastCandNum=g_aucIntraModeNumFast[ uiWidthBit ];
#endif
  
  pcCU->setTrIdxSubParts( uiMaxTrDepth, 0, uiDepth );
  
  for ( idQp = iMindQp; idQp <= iMaxdQp; idQp++ )
  {
    UInt uiPUBits = 0;
    UInt uiPUDistortion = 0;
    Double dPUCost = 0;
    
    pcCU->clearCbf(0, TEXT_LUMA, pcCU->getTotalNumPart());
    
    // Set Qp for quantization.
    m_pcTrQuant->setQPforQuant( pcCU->getSlice()->getSliceQp()+idQp,  !pcCU->getSlice()->getDepth() , pcCU->getSlice()->getSliceType(), TEXT_LUMA );
    pcCU->setQPSubParts( pcCU->getSlice()->getSliceQp()+idQp, 0, uiDepth );
    
    uiPartOffset  = 0;
    uiCoeffOffset = 0;
    for( uiPU = 0; uiPU < uiNumPU; uiPU++ )
    {
      pcPattern->initPattern( pcCU, uiPartDepth, uiPartOffset );
      
      // ADI ADDED
      Bool bAboveAvail = false;
      Bool bLeftAvail  = false;
      
      pcPattern->initAdiPattern(pcCU, uiPartOffset, uiPartDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
#ifdef EDGE_BASED_PREDICTION
      if(getEdgeBasedPred()->get_edge_prediction_enable())
        getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiPartOffset, uiPartDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
      
#if ANG_INTRA
#if UNIFIED_DIRECTIONAL_INTRA
      uiMaxMode          = angIntraEnabled ? g_aucIntraModeNumAng[uiWidthBit] : g_aucIntraModeNum[uiWidthBit];
#else
      uiMaxMode          = angIntraEnabled ? 34 : g_aucIntraModeNum[uiWidthBit];
#endif
#else
      uiMaxMode          = g_aucIntraModeNum    [uiWidthBit];
#endif
      UInt uiMaxModeFast = g_aucIntraModeNumFast[uiWidthBit];
      
      pOrg     = pcOrgYuv->getLumaAddr  (uiPU, uiWidth);
      pResi    = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
      pReco    = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
      pPred    = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
      pCoeff   = pcCU->getCoeffY()  + uiCoeffOffset;
      
      dPUBestCost = MAX_DOUBLE;
      
      
#if SAMSUNG_FAST_UDI
      if ( uiFastCandNum != uiMaxMode )
      {
        uiMaxModeFast = 0;
      }
      for ( Int i=0; i<uiFastCandNum; i++)
      {
        CandCostList[i]=MAX_DOUBLE;
      }
      CandNum=0;
#else
      UInt uiBestSad    = MAX_UINT;
      UInt iBestPreMode = 0;
#endif
      
      for ( uiMode = uiMaxModeFast; uiMode < uiMaxMode; uiMode++ )
      {
#if ANG_INTRA
        if ( !predIntraLumaDirAvailable( uiMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
          continue;
        
        if ( angIntraEnabled ){
#if HHI_AIS
          predIntraLumaAng( pcPattern, uiMode, bPUCurrFilt, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
          predIntraLumaAng( pcPattern, uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
        }
        else{
#if HHI_AIS
          predIntraLumaAdi( pcPattern, uiMode, bPUCurrFilt, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
          predIntraLumaAdi( pcPattern, uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
        }
        
#else // ANG_INTRA
        UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiMode];
        
        if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
          continue;
        
#if HHI_AIS
        predIntraLumaAdi( pcPattern, uiMode, bPUCurrFilt, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#else
        predIntraLumaAdi( pcPattern, uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif
#endif // ANG_INTRA
        
        Pel* piOrgY  = pcOrgYuv  ->getLumaAddr(uiPU, uiWidth);
        Pel* piPreY  = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
        UInt iStride = pcOrgYuv  ->getStride  ();
        
        // use hadamard transform here
        UInt uiSad   = m_pcRdCost->calcHAD( piOrgY, iStride, piPreY, iStride, uiWidth, uiHeight );

#if SAMSUNG_FAST_UDI
        UInt   iModeBits = xModeBitsIntra( pcCU, uiMode, uiPU, uiPartOffset, uiDepth, uiPartDepth );
        Double dModeRate = m_pcRdCost->getLambda();
        Double uiCost    = (Double)uiSad+(Double)iModeBits*sqrt(dModeRate);

        CandNum += xUpdateCandList( uiMode, uiCost, uiFastCandNum, CandModeList, CandCostList );
#else  
        if ( uiSad < uiBestSad )
        {
          uiBestSad    = uiSad;
          iBestPreMode = uiMode;
        }
#endif
      }
      
      UInt uiRdModeList[FAST_UDI_MAX_RDMODE_NUM];
      UInt uiNewMaxMode;
      
#if SAMSUNG_FAST_UDI
      if( uiFastCandNum!=uiMaxMode )
      {
        uiNewMaxMode=Min( uiFastCandNum, CandNum );
        for ( Int i=0; i<uiNewMaxMode; i++)
        {
          uiRdModeList[i] = CandModeList[i];
        }
      }
      else
      {
        uiNewMaxMode=uiMaxMode;
        for ( Int i=0; i<uiNewMaxMode; i++ )
        {
          uiRdModeList[i] = i;
        }
      }
#else

	  for( Int i = 0; i < FAST_UDI_MAX_RDMODE_NUM; i++ ) uiRdModeList[ i ] = i;
	  if( uiMaxModeFast >= uiMaxMode )
		  uiNewMaxMode = uiMaxMode;
      else
      {
        uiNewMaxMode=uiMaxModeFast+1;
        uiRdModeList[uiMaxModeFast]=iBestPreMode;
      }
      
#endif
      
      bAboveAvail = false;
      bLeftAvail  = false;
      pcPattern->initPattern( pcCU, uiPartDepth, uiPartOffset );
      pcPattern->initAdiPattern(pcCU, uiPartOffset, uiPartDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
#ifdef EDGE_BASED_PREDICTION
      if(getEdgeBasedPred()->get_edge_prediction_enable())
        getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiPartOffset, uiPartDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
      
      for ( uiMode = uiMinMode; uiMode < uiNewMaxMode; uiMode++ )
      {
        UInt uiOrgMode = uiRdModeList[uiMode];
        
#if ANG_INTRA
        if ( !predIntraLumaDirAvailable( uiOrgMode, uiWidthBit, angIntraEnabled, bAboveAvail, bLeftAvail ) )
          continue;
#else
        UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiOrgMode];
        
        if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
          continue;
#endif
        
        uiBits = 0;
        pcCU->setLumaIntraDirSubParts     ( uiOrgMode,   uiPartOffset, uiPartDepth+uiDepth );
        
#if HHI_AIS
        bPUCurrFilt    = bUseAIS ? true : DEFAULT_IS;
        pcCU->setLumaIntraFiltFlagSubParts( bPUCurrFilt, uiPartOffset, uiPartDepth+uiDepth );
#endif
        
        if(m_bUseSBACRD)
        {
          if( uiPU )
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
          else
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        
#if HHI_AIS
        xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiOrgMode, bPUCurrFilt, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail, (uiMaxTrDepth>uiPartDepth)? 1:0);
#else
        xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiOrgMode, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail, (uiMaxTrDepth>uiPartDepth)? 1:0);
#endif
        pcCU->setCuCbfLuma( uiPartOffset, uiMaxTrDepth, uiPartDepth );
        
        
        uiDistortion = m_pcRdCost->getDistPart( pReco, uiStride, pcOrgYuv->getLumaAddr(uiPU, uiWidth), uiStride, uiWidth, uiHeight );
        
        if(m_bUseSBACRD)
        {
          if( uiPU )
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
          else
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        
        xAddSymbolBitsIntra( pcCU, pCoeff, uiPU, uiQNumParts, uiPartDepth, 1, uiMaxTrDepth, uiPartDepth, uiWidth, uiHeight, uiBits );
        
        dCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
        
        if( dCost < dPUBestCost )
        {
          uiPUBestMode       = uiOrgMode;
#if HHI_AIS
          bPUBestFilt        = bPUCurrFilt;
#endif
          uiPUBestBits       = uiBits;
          uiPUBestDistortion = uiDistortion;
          dPUBestCost        = dCost;
          
          if( m_bUseSBACRD )
            m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
        }
        
#if HHI_AIS
#if !AIS_TEST_BEST
        // BB: test every mode without filtering (execpt intra DC)
        ////////////////////////////////////////////////////////////////////////////
        if ( bUseAIS && (uiOrgMode != 2) )
        {
          dPUNoFiltCost        = MAX_DOUBLE;
          uiPUNoFiltBits       = 0;
          uiPUNoFiltDistortion = 0;
          bPUCurrFilt          = false;
          
          pcCU->setLumaIntraDirSubParts     ( uiOrgMode,   uiPartOffset, uiPartDepth+uiDepth );
          pcCU->setLumaIntraFiltFlagSubParts( bPUCurrFilt, uiPartOffset, uiPartDepth+uiDepth );
          
          if(m_bUseSBACRD)
          {
            if( uiPU )
              m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
            else
              m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
          }
          
          xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiOrgMode, bPUCurrFilt, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail, (uiMaxTrDepth>uiPartDepth)? 1:0);
          pcCU->setCuCbfLuma( uiPartOffset, uiMaxTrDepth, uiPartDepth );
          
          
          uiPUNoFiltDistortion = m_pcRdCost->getDistPart( pReco, uiStride, pcOrgYuv->getLumaAddr(uiPU, uiWidth), uiStride, uiWidth, uiHeight );
          
          if(m_bUseSBACRD)
          {
            if( uiPU )
              m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
            else
              m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
          }
          
          xAddSymbolBitsIntra( pcCU, pCoeff, uiPU, uiQNumParts, uiPartDepth, 1, uiMaxTrDepth, uiPartDepth, uiWidth, uiHeight, uiPUNoFiltBits );
          
          dPUNoFiltCost = m_pcRdCost->calcRdCost( uiPUNoFiltBits, uiPUNoFiltDistortion );
          
          if( dPUNoFiltCost < dPUBestCost )
          {
            uiPUBestMode       = uiOrgMode;
            bPUBestFilt        = bPUCurrFilt;
            uiPUBestBits       = uiPUNoFiltBits;
            uiPUBestDistortion = uiPUNoFiltDistortion;
            dPUBestCost        = dPUNoFiltCost;
            
            if( m_bUseSBACRD )
              m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
          }
        }
        ////////////////////////////////////////////////////////////////////////////
#endif
        
#endif
      } // Mode loop
      
#if HHI_AIS
#if AIS_TEST_BEST
      // BB: test best mode without filtering (execpt intra DC)
      ////////////////////////////////////////////////////////////////////////////
      if ( bUseAIS && (uiPUBestMode != 2) )
      {
        dPUNoFiltCost        = MAX_DOUBLE;
        uiPUNoFiltBits       = 0;
        uiPUNoFiltDistortion = 0;
        bPUCurrFilt          = false;
        
        pcCU->setLumaIntraDirSubParts     ( uiPUBestMode,   uiPartOffset, uiPartDepth+uiDepth );
        pcCU->setLumaIntraFiltFlagSubParts( bPUCurrFilt, uiPartOffset, uiPartDepth+uiDepth );
        
        if(m_bUseSBACRD)
        {
          if( uiPU )
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
          else
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        
        xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiPUBestMode, bPUCurrFilt, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail, (uiMaxTrDepth>uiPartDepth)? 1:0);
        pcCU->setCuCbfLuma( uiPartOffset, uiMaxTrDepth, uiPartDepth );
        
        
        uiPUNoFiltDistortion = m_pcRdCost->getDistPart( pReco, uiStride, pcOrgYuv->getLumaAddr(uiPU, uiWidth), uiStride, uiWidth, uiHeight );
        
        if(m_bUseSBACRD)
        {
          if( uiPU )
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
          else
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        
        xAddSymbolBitsIntra( pcCU, pCoeff, uiPU, uiQNumParts, uiPartDepth, 1, uiMaxTrDepth, uiPartDepth, uiWidth, uiHeight, uiPUNoFiltBits );
        
        dPUNoFiltCost = m_pcRdCost->calcRdCost( uiPUNoFiltBits, uiPUNoFiltDistortion );
        
        if( dPUNoFiltCost < dPUBestCost )
        {
          bPUBestFilt        = bPUCurrFilt;
          uiPUBestBits       = uiPUNoFiltBits;
          uiPUBestDistortion = uiPUNoFiltDistortion;
          dPUBestCost        = dPUNoFiltCost;
          
          if( m_bUseSBACRD )
            m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
        }
        pcCU->setLumaIntraFiltFlagSubParts( bPUBestFilt,     uiPartOffset, uiPartDepth+uiDepth );
      }
      ////////////////////////////////////////////////////////////////////////////
#endif
      
      bPUFilt[uiPU]       = bPUBestFilt;
#endif
      uiPUMode[uiPU]      = uiPUBestMode;
      uiPUBits           += uiPUBestBits;
      uiPUDistortion     += uiPUBestDistortion;
      dPUCost            += dPUBestCost;
      
      pOrg   = pcOrgYuv->getLumaAddr(uiPU, uiWidth);
      pResi  = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
      pPred  = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
      pReco  = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
      pCoeff = pcCU->getCoeffY()  + uiCoeffOffset;
      
      pcCU->setLumaIntraDirSubParts     ( uiPUBestMode,    uiPartOffset, uiPartDepth+uiDepth );
#if HHI_AIS
      pcCU->setLumaIntraFiltFlagSubParts( bPUBestFilt,     uiPartOffset, uiPartDepth+uiDepth );
#endif
      
      if(m_bUseSBACRD)
      {
        if( uiPU )
          m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
        else
          m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
      }
      
#if HHI_AIS
      xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiPUBestMode, bPUBestFilt, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0  );
#else
      xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiPUBestMode, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0  );
#endif
      pcCU->setCuCbfLuma( uiPartOffset, uiMaxTrDepth, uiPartDepth );
      
      pcCU->copyToPic(uiDepth, uiPU, uiPartDepth);
      
      if( m_bUseSBACRD )
        m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST]->store( m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST] );
      
      uiPartOffset  += uiQNumParts;
      uiCoeffOffset += uiCoeffSize;
    } // PU loop
    
    
    pcCU->setCuCbfLuma( 0, uiMaxTrDepth, 0);
    
    if( dPUCost < dBestCost )
    {
      uiBestMode[0] = uiPUMode[0]; uiBestMode[1] = uiPUMode[1]; uiBestMode[2] = uiPUMode[2]; uiBestMode[3] = uiPUMode[3];
#if HHI_AIS
      bBestFilt[0]  =  bPUFilt[0];  bBestFilt[1] =  bPUFilt[1];  bBestFilt[2] =  bPUFilt[2];  bBestFilt[3] =  bPUFilt[3];
#endif
      uiBestDistortion = uiPUDistortion;
      iBestdQp   = idQp;
      dBestCost  = dPUCost;
      uiBestBits = uiPUBits;
      
      if(m_bUseSBACRD)
      {
        if ( uiMaxTrDepth < uiPartDepth )
        {
          m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
        }
        else
        {
          m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
        }
      }
    }
  } // dQp loop
  
  
  //Finalize Intra Coding
  pcCU->getTotalBits()       = uiBestBits;
  pcCU->getTotalCost()       = dBestCost;
  pcCU->getTotalDistortion() = uiBestDistortion;
  
  if(m_bUseSBACRD)
    m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
  
  pcCU->setQPSubParts( pcCU->getSlice()->getSliceQp()+iBestdQp, 0, uiDepth );
  pcCU->setTrIdxSubParts( uiMaxTrDepth, 0, uiDepth );
  uiWidthBit = pcCU->getIntraSizeIdx(0);
  
  uiPartOffset  = 0;
  uiCoeffOffset = 0;
  
  for( uiPU = 0; uiPU < uiNumPU; uiPU++ )
  {
    pcCU->setLumaIntraDirSubParts     ( uiBestMode[uiPU],     uiPartOffset, uiPartDepth+uiDepth );
#if HHI_AIS
    pcCU->setLumaIntraFiltFlagSubParts( bBestFilt[uiPU],      uiPartOffset, uiPartDepth+uiDepth );
#endif
    
    pOrg     = pcOrgYuv->getLumaAddr(uiPU, uiWidth);
    pResi    = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
    pPred    = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
    pReco    = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
    uiStride = rpcPredYuv->getStride();
    pCoeff   = pcCU->getCoeffY()  + uiCoeffOffset;
    
    pcPattern->initPattern( pcCU, uiPartDepth, uiPartOffset );
    
    // ADI ADDED
    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;
    
    pcPattern->initAdiPattern(pcCU, uiPartOffset, uiPartDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);
#ifdef EDGE_BASED_PREDICTION
    if(getEdgeBasedPred()->get_edge_prediction_enable())
      getEdgeBasedPred()->initEdgeBasedBuffer(pcCU, uiPartOffset, uiPartDepth, m_piYExtEdgeBased);
#endif //EDGE_BASED_PREDICTION
    
    
#if HHI_AIS
    xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiBestMode[uiPU], bBestFilt[uiPU], uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0);
#else
    xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiBestMode[uiPU], uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0);
#endif
    pcCU->setCuCbfLuma( uiPartOffset, uiMaxTrDepth, uiPartDepth );
    
    pcCU->copyToPic(uiDepth, uiPU, uiPartDepth);
    
    uiPartOffset  += uiQNumParts;
    uiCoeffOffset += uiCoeffSize;
  }
}

Void TEncSearch::predIntraChromaAdiSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv, UInt uiChromaTrMode )
{
  UInt    uiBestBits       = 0;
  UInt    uiBestDistortion = 0;
  UInt    uiDistortion     = 0;
  
  Double  fBestCost        = MAX_DOUBLE;
  Double  fCost            = 0;
  
  UInt    uiWidth          = pcCU->getWidth(0) >>1;
  UInt    uiHeight         = pcCU->getHeight(0)>>1;
  
  // Mode
  UInt    uiMinMode        = 0;
  UInt    uiMaxMode        = 4;
  UInt    uiBestMode       = 0;
  UInt    uiMode;
  
  TComPattern* pcPattern = pcCU->getPattern();
  
  // Buffer pointers for each transform unit
  Pel*    pOrigCb = pcOrgYuv  ->getCbAddr();
  Pel*    pResiCb = rpcResiYuv->getCbAddr();
  Pel*    pRecoCb = rpcRecoYuv->getCbAddr();
  Pel*    pPredCb = rpcPredYuv->getCbAddr();
  TCoeff* pCoefCb = pcCU      ->getCoeffCb();
  
  Pel*    pOrigCr = pcOrgYuv  ->getCrAddr();
  Pel*    pResiCr = rpcResiYuv->getCrAddr();
  Pel*    pRecoCr = rpcRecoYuv->getCrAddr();
  Pel*    pPredCr = rpcPredYuv->getCrAddr();
  TCoeff* pCoefCr = pcCU      ->getCoeffCr();
  UInt    uiStride = pcOrgYuv ->getCStride();
  
  // Set Qp for quantization.
  m_pcTrQuant->setQPforQuant( pcCU->getQP(0),  !pcCU->getSlice()->getDepth() , pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
  
  if (m_pcEncCfg->getUseRDOQ())
  {
    if( m_bUseSBACRD )
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CHROMA_INTRA] );
    m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, uiWidth, TEXT_CHROMA);
  }
  
  pcPattern->initPattern( pcCU, 0, 0 );
  
  Int  iIntraIdx      = pcCU->getIntraSizeIdx(0);
  
  UInt    uiModeList[5];
  for (Int i=0;i<4;i++)
    uiModeList[i]=i;
  
#if ANG_INTRA
  uiModeList[4] = pcCU->angIntraEnabledPredPart( 0 ) ? pcCU->getLumaIntraDir( 0 ) : g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir( 0 )];
#else
  uiModeList[4] = g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir(0)];
#endif
  
  if (uiModeList[4]>=4) uiMaxMode=5;
  
  // Prediction Mode loop
  for ( uiMode = uiMinMode; uiMode < uiMaxMode; uiMode++ )
  {
    pcCU->setChromIntraDirSubParts(  uiMode, 0, pcCU->getDepth(0) ); // for RD cost
    
    pcCU->clearCbf(0, TEXT_CHROMA_U, pcCU->getTotalNumPart());
    pcCU->clearCbf(0, TEXT_CHROMA_V, pcCU->getTotalNumPart());
    
    xRecurIntraChromaSearchADI( pcCU, 0,  pOrigCb, pPredCb, pResiCb, pRecoCb, uiStride, pCoefCb, uiModeList[uiMode], uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_U);
    xRecurIntraChromaSearchADI( pcCU, 0,  pOrigCr, pPredCr, pResiCr, pRecoCr, uiStride, pCoefCr, uiModeList[uiMode], uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_V);
    
    pcCU->setCuCbfChroma( 0, uiChromaTrMode );
    
    // For RD compare
    uiDistortion  = m_pcRdCost->getDistPart( pRecoCb, uiStride, pOrigCb, uiStride, uiWidth, uiHeight );
    uiDistortion += m_pcRdCost->getDistPart( pRecoCr, uiStride, pOrigCr, uiStride, uiWidth, uiHeight );
    
    // load Going on CI from the CI_CHROMA
    if( m_bUseSBACRD )
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CHROMA_INTRA] );
    
    // Entropy coding
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeIntraDirModeChroma( pcCU, 0, true );
    
    m_pcEntropyCoder->encodeCbf( pcCU, 0, TEXT_CHROMA_U, 0 );
    m_pcEntropyCoder->encodeCoeff(pcCU, pCoefCb, 0, pcCU->getDepth(0), uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_U);
    
    m_pcEntropyCoder->encodeCbf( pcCU, 0, TEXT_CHROMA_V, 0 );
    m_pcEntropyCoder->encodeCoeff(pcCU, pCoefCr, 0, pcCU->getDepth(0), uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_V);
    UInt uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
    
    // Calculate RD cost
    fCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
    
    // Choose Best RD mode
    if( fCost < fBestCost )
    {
      // Keep best_dir, best Qp, best transform index, best_distortion, best_bit, best_RDcost
      uiBestBits       = uiBits;
      uiBestDistortion = uiDistortion;
      uiBestMode       = uiMode;
      fBestCost        = fCost;
      
      // store Going on CI from the Temp best buffer
      if( m_bUseSBACRD )
        m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST] );
    }
  } // End of Mode loop
  
  // Set bests
  pcCU->getTotalBits()       += uiBestBits;
  pcCU->getTotalCost()       += fBestCost;
  pcCU->getTotalDistortion() += uiBestDistortion;
  pcCU->setChromIntraDirSubParts( uiBestMode, 0, pcCU->getDepth( 0 ) );
  
  pcCU->clearCbf(0, TEXT_CHROMA_U, pcCU->getTotalNumPart());
  pcCU->clearCbf(0, TEXT_CHROMA_V, pcCU->getTotalNumPart());
  
  // Best prediction (MUST BE FIXED FOR EACH TRANSFORM UNIT PREDICTION)
  xRecurIntraChromaSearchADI( pcCU, 0,  pOrigCb, pPredCb, pResiCb, pRecoCb, uiStride, pCoefCb, uiModeList[uiBestMode], uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_U);
  xRecurIntraChromaSearchADI( pcCU, 0,  pOrigCr, pPredCr, pResiCr, pRecoCr, uiStride, pCoefCr, uiModeList[uiBestMode], uiWidth, uiHeight, uiChromaTrMode, 0, TEXT_CHROMA_V);
  
  pcCU->setCuCbfChroma( 0, uiChromaTrMode );
}

#if HHI_MRG_PU
#ifdef DCM_PBIC
Void TEncSearch::xMergeEstimation( TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, Int* piRefIdxPred, TComMv* pcMvTemp, TComIc& rcIcTemp, UInt& uiInterDir, UInt& uiMergeIndex, UInt& ruiCost )
#else
Void TEncSearch::xMergeEstimation( TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, Int* piRefIdxPred, TComMv* pcMvTemp, UInt& uiInterDir, UInt& uiMergeIndex, UInt& ruiCost )
#endif
{
  UInt        uiNeighbourInfos = 0;
  TComMvField  cMFieldNeighbours[4]; // 0: above ref_list 0, above ref list 1, left ref list 0, left ref list 1
#ifdef DCM_PBIC
  TComIc cIcNeighbours[2]; //above, left
#endif
  UChar uhInterDirNeighbours[2];
  uhInterDirNeighbours[0] = 0;
  uhInterDirNeighbours[1] = 0;
  
  UInt uiCostTemp = MAX_UINT;
  UInt uiBitsTemp = 0;
  ruiCost = MAX_UINT;

  Int iNumPredDir = pcCU->getSlice()->isInterP() ? 1 : 2;

  UInt uiAbsPartIdx = 0;
  Int iWidth = 0;
  Int iHeight = 0;
  pcCU->getPartIndexAndSize( iPartIdx, uiAbsPartIdx, iWidth, iHeight );
#ifdef DCM_PBIC
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMFieldNeighbours, cIcNeighbours, uhInterDirNeighbours, uiNeighbourInfos );
#else
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfos );
#endif

  UInt uiNeighbourIdx = 0; // 0: top, 1: left
  if ( uiNeighbourInfos == 2 )
  {
    // test left parameters first
    uiNeighbourIdx = 1;
  }

  TComYuv cYuvPred;
  cYuvPred.create( pcYuvOrg->getWidth(), pcYuvOrg->getHeight() );

  while ( uiNeighbourInfos > 0 && uiNeighbourIdx < 2 )
  {
    uiCostTemp = MAX_UINT;    
    uiBitsTemp = 0;

    PartSize ePartSize = pcCU->getPartitionSize( 0 );

#ifdef QC_AMVRES
    if (pcCU->getSlice()->getSPS()->getUseAMVRes())
    {
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField_AMVRes( cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField_AMVRes( cMFieldNeighbours[1 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[1 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
    }
    else
    {
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMFieldNeighbours[1 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[1 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
    }
#else
    pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
    pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMFieldNeighbours[1 + 2*uiNeighbourIdx].getMv(), cMFieldNeighbours[1 + 2*uiNeighbourIdx].getRefIdx(), ePartSize, uiAbsPartIdx, iPartIdx, 0 );
#endif

#ifdef DCM_PBIC
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      RefPicList eRefList = (uhInterDirNeighbours[uiNeighbourIdx] == 3) ? REF_PIC_LIST_X : ( (uhInterDirNeighbours[uiNeighbourIdx] == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
      cIcNeighbours[uiNeighbourIdx].computeScaleOffset( eRefList );
      pcCU->getCUIcField()->setAllIcField( cIcNeighbours[uiNeighbourIdx], ePartSize, uiAbsPartIdx, iPartIdx, 0 );
    }
#endif

    motionCompensation( pcCU, &cYuvPred, REF_PIC_LIST_X, iPartIdx );

    // both blocks have the size of the CU but we only cycle over the current PU.
    Pel* pOrgY = pcYuvOrg->getLumaAddr( uiAbsPartIdx );
    Pel* pPredY = cYuvPred.getLumaAddr( uiAbsPartIdx );
    UInt uiSAD = 0;
    for ( UInt uiY = 0; uiY < iHeight; uiY++ )
    {
      for ( UInt uiX = 0; uiX < iWidth; uiX++ )
      {
        uiSAD += abs( pOrgY[uiX] - pPredY[uiX] );
      }
      pOrgY += pcYuvOrg->getStride();
      pPredY += cYuvPred.getStride();
    }

    uiSAD >>= g_uiBitIncrement;

    // Merge signalization
    if ( uiNeighbourInfos == 3 )
    {
      uiBitsTemp = 2;
    }
    else
    {
      uiBitsTemp = 1;
    }

    m_pcRdCost->getMotionCost( 0, 0 ); // choose lambda;

    uiCostTemp = (UInt)( floor(  (Double)uiSAD + (Double)m_pcRdCost->getCost( uiBitsTemp ) ) );


    if ( iNumPredDir == 1 ) // P slice
    {
      if ( uiCostTemp < ruiCost )
      {
        ruiCost = uiCostTemp;
        pcMvTemp[0]    = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv();
#ifdef DCM_PBIC
        rcIcTemp       = cIcNeighbours[uiNeighbourIdx];
#endif
        piRefIdxPred[0] = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx();
        uiInterDir = 1;
        uiMergeIndex = uiNeighbourIdx;
      }
    }
    else // B slice
    {
      // Ref List 0
      if ( uhInterDirNeighbours[uiNeighbourIdx] == 1 )
      {
        if ( uiCostTemp < ruiCost )
        {
          ruiCost = uiCostTemp;
          pcMvTemp[0]    = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv();
#ifdef DCM_PBIC
          rcIcTemp       = cIcNeighbours[uiNeighbourIdx];
#endif
          piRefIdxPred[0] = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx();
          uiInterDir = 1;
          uiMergeIndex = uiNeighbourIdx;
        }
      }
      // Ref List 1
      else if ( uhInterDirNeighbours[uiNeighbourIdx] == 2 )
      {
        if ( uiCostTemp < ruiCost )
        {
          ruiCost = uiCostTemp;
          pcMvTemp[1]    = cMFieldNeighbours[1 + 2*uiNeighbourIdx].getMv();
#ifdef DCM_PBIC
          rcIcTemp       = cIcNeighbours[uiNeighbourIdx];
#endif
          piRefIdxPred[1] = cMFieldNeighbours[1 + 2*uiNeighbourIdx].getRefIdx();
          uiInterDir = 2;
          uiMergeIndex = uiNeighbourIdx;
        }
      } 
      // Both of the List 0&1
      else 
      {
        if ( uiCostTemp < ruiCost )
        {
          ruiCost = uiCostTemp;
          pcMvTemp[0]    = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getMv();
          pcMvTemp[1]    = cMFieldNeighbours[1 + 2*uiNeighbourIdx].getMv();
#ifdef DCM_PBIC
          rcIcTemp       = cIcNeighbours[uiNeighbourIdx];
#endif
          piRefIdxPred[0] = cMFieldNeighbours[0 + 2*uiNeighbourIdx].getRefIdx();
          piRefIdxPred[1] = cMFieldNeighbours[1 + 2*uiNeighbourIdx].getRefIdx();
          uiInterDir = 3;
          uiMergeIndex = uiNeighbourIdx;
        }
      }
    }
    if ( uiNeighbourInfos != 3 )
    {
      // there is only one merge candidate
      // only one candidate has to be tested.
      break;
    }
    else
    {
      uiNeighbourIdx++;
    }
  }

  cYuvPred.destroy();
}
#endif

Void TEncSearch::predInterSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv, Bool bUseRes )
{
  m_acYuvPred[0].clear();
  m_acYuvPred[1].clear();
  m_cYuvPredTemp.clear();
  rpcPredYuv->clear();
  
  if ( !bUseRes )
  {
    rpcResiYuv->clear();
  }
  
  rpcRecoYuv->clear();
  
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;
  
  TComMv        cMvZero;
  TComMv        TempMv; //kolya
  
  TComMv        cMv[2];
  TComMv        cMvBi[2];
  TComMv        cMvTemp[2][33];
  
  Int           iNumPart    = pcCU->getNumPartInter();
  Int           iNumPredDir = pcCU->getSlice()->isInterP() ? 1 : 2;
  
  TComMv        cMvPred[2][33];
  
  TComMv        cMvPredBi[2][33];
  Int           aaiMvpIdxBi[2][33];
#if HHI_IMVP
  TComMv        cMvPredBiTemp[2][33];
  MvPredMeasure cMvPredMeasure[2][33];
#endif
  
  Int           aaiMvpIdx[2][33];
  Int           aaiMvpNum[2][33];
  
  AMVPInfo aacAMVPInfo[2][33];
  
  Int           iRefIdx[2];
  Int           iRefIdxBi[2];
  
  UInt          uiPartAddr;
  Int           iRoiWidth, iRoiHeight;
  
  UInt          uiMbBits[3] = {1, 1, 0};
  
  UInt          uiLastMode = 0;
  Int           iRefStart, iRefEnd;
  
  PartSize      ePartSize = pcCU->getPartitionSize( 0 );
  
#if HHI_MRG_PU
  // Merge tools
  UInt uiMergeInterDir = 0;
  Int iMergeRefIdx[2];
  TComMv cMergeMv[2];
#ifdef DCM_PBIC
  TComIc cMergeIc;
#endif
  Int iRefBestList= 0;
  UInt uiMergeCost = MAX_UINT;
  UInt uiMergeIndex = 0;
#endif

  for ( Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++ )
  {
    UInt          uiCost[2] = { MAX_UINT, MAX_UINT };
    UInt          uiCostBi  =   MAX_UINT;
    UInt          uiCostTemp;
    
    UInt          uiBits[3];
    UInt          uiBitsTemp;
    
    xGetBlkBits( ePartSize, pcCU->getSlice()->isInterP(), iPartIdx, uiLastMode, uiMbBits);
    
    pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );
    
    //  Uni-directional prediction
    for ( Int iRefList = 0; iRefList < iNumPredDir; iRefList++ )
    {
      RefPicList  eRefPicList = ( iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
      
      for ( Int iRefIdxTemp = 0; iRefIdxTemp < pcCU->getSlice()->getNumRefIdx(eRefPicList); iRefIdxTemp++ )
      {
#ifdef QC_SIFO
        setCurrList(iRefList);
        setCurrRefFrame(iRefIdxTemp);
#endif
        uiBitsTemp = uiMbBits[iRefList];
        if ( pcCU->getSlice()->getNumRefIdx(eRefPicList) > 1 )
        {
          uiBitsTemp += iRefIdxTemp+1;
          if ( iRefIdxTemp == pcCU->getSlice()->getNumRefIdx(eRefPicList)-1 ) uiBitsTemp--;
        }
        xEstimateMvPredAMVP( pcCU, pcOrgYuv, iPartIdx, eRefPicList, iRefIdxTemp, cMvPred[iRefList][iRefIdxTemp]);
#if HHI_IMVP
        if ( pcCU->getSlice()->getSPS()->getUseIMP() )
        {
          xEstimateMvPredIMVP( pcCU, iPartIdx, eRefPicList, iRefIdxTemp, cMvPred[iRefList][iRefIdxTemp], cMvPredMeasure[iRefList][iRefIdxTemp] );
          m_cMvPredMeasure = cMvPredMeasure[iRefList][iRefIdxTemp];
        }
#endif
        aaiMvpIdx[iRefList][iRefIdxTemp] = pcCU->getMVPIdx(eRefPicList, uiPartAddr);
        aaiMvpNum[iRefList][iRefIdxTemp] = pcCU->getMVPNum(eRefPicList, uiPartAddr);
        
        uiBitsTemp += m_auiMVPIdxCost[aaiMvpIdx[iRefList][iRefIdxTemp]][aaiMvpNum[iRefList][iRefIdxTemp]];
        
#if GPB_SIMPLE_UNI
        if ( pcCU->getSlice()->getSPS()->getUseLDC() )
        {
#if MS_NO_BACK_PRED_IN_B0
          if ( iRefList && ( iRefIdxTemp != iRefIdx[0] || pcCU->getSlice()->getNoBackPredFlag() ) )
#else
          if ( iRefList && iRefIdxTemp != iRefIdx[0] )
#endif
          {
            uiCostTemp = MAX_UINT;
#ifdef QC_AMVRES
#if HHI_IMVP
            //if ( pcCU->getSlice()->getSPS()->getUseIMP() && pcCU->getSlice()->getSPS()->getUseAMVRes())
            if ( pcCU->getSlice()->getSPS()->getUseIMP())
              cMvPred[iRefList][iRefIdxTemp] = m_cMvPredMeasure.getMVPred( cMvTemp[iRefList][iRefIdxTemp].getHor(), cMvTemp[iRefList][iRefIdxTemp].getVer() );
#endif
#endif
#if MS_NO_BACK_PRED_IN_B0
            if ( pcCU->getSlice()->getNoBackPredFlag() )
            {
              cMvTemp[1][iRefIdxTemp] = cMvTemp[0][iRefIdxTemp];
              //xMotionEstimationForL1 ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPred[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp );
            }
#endif
          }
          else
          {
            xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPred[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp );
          }
        }
        else
        {
          xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPred[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp );
        }
#else
        xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPred[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp );
#endif
        xCopyAMVPInfo(pcCU->getCUMvField(eRefPicList)->getAMVPInfo(), &aacAMVPInfo[iRefList][iRefIdxTemp]); // must always be done ( also when AMVP_MODE = AM_NONE )
        if ( pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
        {          
#ifdef QC_AMVRES
          if( !cMvTemp[iRefList][iRefIdxTemp].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
            xCheckBestMVP_onefourth(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPred[iRefList][iRefIdxTemp], aaiMvpIdx[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
          else
#endif
            xCheckBestMVP(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPred[iRefList][iRefIdxTemp], aaiMvpIdx[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
#if HHI_IMVP
          if ( pcCU->getSlice()->getSPS()->getUseIMP() )
          {
            cMvPredMeasure[iRefList][iRefIdxTemp] = m_cMvPredMeasure;
          }
#endif
        }
#if MS_NO_BACK_PRED_IN_B0
        if ( ( iRefList == 0 && uiCostTemp < uiCost[iRefList] ) ||
             ( iRefList == 1 &&  pcCU->getSlice()->getNoBackPredFlag() && iRefIdxTemp == iRefIdx[0] ) ||
             ( iRefList == 1 && !pcCU->getSlice()->getNoBackPredFlag() && uiCostTemp < uiCost[iRefList] ) )
#else
        if ( uiCostTemp < uiCost[iRefList] )
#endif
        {
          uiCost[iRefList] = uiCostTemp;
          uiBits[iRefList] = uiBitsTemp; // storing for bi-prediction
          
          // set motion
          cMv[iRefList]     = cMvTemp[iRefList][iRefIdxTemp];
          iRefIdx[iRefList] = iRefIdxTemp;
#ifdef QC_AMVRES
          if (pcCU->getSlice()->getSPS()->getUseAMVRes())
            pcCU->getCUMvField(eRefPicList)->setAllMvField_AMVRes( cMv[iRefList], iRefIdx[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );
          else
#endif
            pcCU->getCUMvField(eRefPicList)->setAllMvField( cMv[iRefList], iRefIdx[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );
          
          // storing list 1 prediction signal for iterative bi-directional prediction
          if ( eRefPicList == REF_PIC_LIST_1 )
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefList];
            motionCompensation ( pcCU, pcYuvPred, eRefPicList, iPartIdx );
          }
#if MS_NO_BACK_PRED_IN_B0
          if ( pcCU->getSlice()->getNoBackPredFlag() && eRefPicList == REF_PIC_LIST_0 )
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefList];
            motionCompensation ( pcCU, pcYuvPred, eRefPicList, iPartIdx );
          }
#endif
        }
      }
    }
    
    //  Bi-directional prediction
    if ( pcCU->getSlice()->isInterB() )
    {

      cMvBi[0] = cMv[0];            cMvBi[1] = cMv[1];
      iRefIdxBi[0] = iRefIdx[0];    iRefIdxBi[1] = iRefIdx[1];
      
      ::memcpy(cMvPredBi, cMvPred, sizeof(cMvPred));
#if HHI_IMVP
      ::memcpy(cMvPredBiTemp, cMvPred, sizeof(cMvPred));
#endif
      ::memcpy(aaiMvpIdxBi, aaiMvpIdx, sizeof(aaiMvpIdx));
      
      UInt uiMotBits[2] = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
      uiBits[2] = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];
      
      // 4-times iteration (default)
      Int iNumIter = 4;
      
      // fast encoder setting: only one iteration
      if ( m_pcEncCfg->getUseFastEnc() )
      {
        iNumIter = 1;
      }
      
      for ( Int iIter = 0; iIter < iNumIter; iIter++ )
      {

        Int         iRefList    = iIter % 2;
#if MS_NO_BACK_PRED_IN_B0
        if ( pcCU->getSlice()->getSPS()->getUseLDC() && m_pcEncCfg->getUseFastEnc() && pcCU->getSlice()->getNoBackPredFlag() )
        {
          iRefList = 1;
        }
#endif
        RefPicList  eRefPicList = ( iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
        
        Bool bChanged = false;
        
#if GPB_SIMPLE
        if ( pcCU->getSlice()->getSPS()->getUseLDC() && iRefList )
        {
          iRefStart = iRefIdxBi[1-iRefList];
          iRefEnd   = iRefIdxBi[1-iRefList];
        }
        else
        {
          iRefStart = 0;
          iRefEnd   = pcCU->getSlice()->getNumRefIdx(eRefPicList)-1;
        }
#else
        iRefStart = 0;
        iRefEnd   = pcCU->getSlice()->getNumRefIdx(eRefPicList)-1;
#endif

#if MS_NO_BACK_PRED_IN_B0
        if ( pcCU->getSlice()->getSPS()->getUseLDC() && m_pcEncCfg->getUseFastEnc() && pcCU->getSlice()->getNoBackPredFlag() )
        {
          iRefStart = 0;
          iRefEnd   = pcCU->getSlice()->getNumRefIdx(eRefPicList)-1;
        }
#endif
        
        for ( Int iRefIdxTemp = iRefStart; iRefIdxTemp <= iRefEnd; iRefIdxTemp++ )
        {
          uiBitsTemp = uiMbBits[2] + uiMotBits[1-iRefList];
          if ( pcCU->getSlice()->getNumRefIdx(eRefPicList) > 1 )
          {
            uiBitsTemp += iRefIdxTemp+1;
            if ( iRefIdxTemp == pcCU->getSlice()->getNumRefIdx(eRefPicList)-1 ) uiBitsTemp--;
          }
          
          uiBitsTemp += m_auiMVPIdxCost[aaiMvpIdxBi[iRefList][iRefIdxTemp]][aaiMvpNum[iRefList][iRefIdxTemp]];
#ifdef QC_SIFO
          setCurrList(iRefList);
          setCurrRefFrame(iRefIdxTemp);
#endif
          // call ME
#if HHI_IMVP
          if ( pcCU->getSlice()->getSPS()->getUseIMP() )
          {
            m_cMvPredMeasure = cMvPredMeasure[iRefList][iRefIdxTemp];
          }
          xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPredBiTemp[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp, true );
#else
          xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPredBi[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp, true );
#endif
          if ( pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
          {
            xCopyAMVPInfo(&aacAMVPInfo[iRefList][iRefIdxTemp], pcCU->getCUMvField(eRefPicList)->getAMVPInfo());
#if HHI_IMVP
#ifdef QC_AMVRES
            if( !cMvTemp[iRefList][iRefIdxTemp].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
              xCheckBestMVP_onefourth(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPredBiTemp[iRefList][iRefIdxTemp], aaiMvpIdxBi[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
            else
#endif 
              xCheckBestMVP(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPredBiTemp[iRefList][iRefIdxTemp], aaiMvpIdxBi[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
            if ( pcCU->getSlice()->getSPS()->getUseIMP() )
            {
              cMvPredMeasure[iRefList][iRefIdxTemp] = m_cMvPredMeasure;
              cMvPredBi[iRefList][iRefIdxTemp].setVer( cMvPredBiTemp[iRefList][iRefIdxTemp].getVer());
            }
            else
            {
              cMvPredBi[iRefList][iRefIdxTemp] =  cMvPredBiTemp[iRefList][iRefIdxTemp];
            }
#else
#ifdef QC_AMVRES
            if( !cMvTemp[iRefList][iRefIdxTemp].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
              xCheckBestMVP_onefourth(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPredBi[iRefList][iRefIdxTemp], aaiMvpIdxBi[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
            else
#endif  
              xCheckBestMVP(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPredBi[iRefList][iRefIdxTemp], aaiMvpIdxBi[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
#endif
          }
          
          if ( uiCostTemp < uiCostBi )
          {
            bChanged = true;
            
            cMvBi[iRefList]     = cMvTemp[iRefList][iRefIdxTemp];
#if HHI_IMVP
            if ( pcCU->getSlice()->getSPS()->getUseIMP() )
            {
              cMvPredBi[iRefList][iRefIdxTemp].setHor( cMvPredBiTemp[iRefList][iRefIdxTemp].getHor() ); 
            }
#endif          
            iRefIdxBi[iRefList] = iRefIdxTemp;
            
            uiCostBi            = uiCostTemp;
            uiMotBits[iRefList] = uiBitsTemp - uiMbBits[2] - uiMotBits[1-iRefList];
            uiBits[2]           = uiBitsTemp;
            
            //  Set motion
#ifdef QC_AMVRES
            if (pcCU->getSlice()->getSPS()->getUseAMVRes())
              pcCU->getCUMvField(eRefPicList)->setAllMvField_AMVRes( cMvBi[iRefList], iRefIdxBi[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );
            else
#endif
              pcCU->getCUMvField( eRefPicList )->setAllMvField( cMvBi[iRefList], iRefIdxBi[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );
            
            TComYuv* pcYuvPred = &m_acYuvPred[iRefList];
            motionCompensation( pcCU, pcYuvPred, eRefPicList, iPartIdx );
          }
        } // for loop-iRefIdxTemp
        
        if ( !bChanged )
        {
          if ( uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1] && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
          {
#if HHI_IMVP
            if ( pcCU->getSlice()->getSPS()->getUseIMP() )
            {
              m_cMvPredMeasure = cMvPredMeasure[0][iRefIdxBi[0]];
            }
#endif
            xCopyAMVPInfo(&aacAMVPInfo[0][iRefIdxBi[0]], pcCU->getCUMvField(REF_PIC_LIST_0)->getAMVPInfo());
#ifdef QC_AMVRES
            if( !cMvBi[0].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
              xCheckBestMVP_onefourth(pcCU, REF_PIC_LIST_0, cMvBi[0], cMvPredBi[0][iRefIdxBi[0]], aaiMvpIdxBi[0][iRefIdxBi[0]], uiBits[2], uiCostBi);
            else
#endif             
              xCheckBestMVP(pcCU, REF_PIC_LIST_0, cMvBi[0], cMvPredBi[0][iRefIdxBi[0]], aaiMvpIdxBi[0][iRefIdxBi[0]], uiBits[2], uiCostBi);
#if HHI_IMVP
            if ( pcCU->getSlice()->getSPS()->getUseIMP() )
            {
              cMvPredMeasure[0][iRefIdxBi[0]] = m_cMvPredMeasure;
              m_cMvPredMeasure = cMvPredMeasure[1][iRefIdxBi[1]];
            }  
#endif
            xCopyAMVPInfo(&aacAMVPInfo[1][iRefIdxBi[1]], pcCU->getCUMvField(REF_PIC_LIST_1)->getAMVPInfo());
#ifdef QC_AMVRES
            if( !cMvBi[1].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
              xCheckBestMVP_onefourth(pcCU, REF_PIC_LIST_1, cMvBi[1], cMvPredBi[1][iRefIdxBi[1]], aaiMvpIdxBi[1][iRefIdxBi[1]], uiBits[2], uiCostBi);
            else
#endif             
              xCheckBestMVP(pcCU, REF_PIC_LIST_1, cMvBi[1], cMvPredBi[1][iRefIdxBi[1]], aaiMvpIdxBi[1][iRefIdxBi[1]], uiBits[2], uiCostBi);
#if HHI_IMVP
            if ( pcCU->getSlice()->getSPS()->getUseIMP() )
            {
              cMvPredMeasure[1][iRefIdxBi[1]] = m_cMvPredMeasure;
            }
#endif
          }
          break;
        }
      } // for loop-iter
    } // if (B_SLICE)
    
#if HHI_MRG_PU
    pcCU->setMergeFlagSubParts( false, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));  
    pcCU->setMergeIndexSubParts( 0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));

    UInt uiBestCost = MAX_INT;
    UInt uiMergeCost = MAX_INT;
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
#ifdef DCM_PBIC
      xMergeEstimation( pcCU, pcOrgYuv, iPartIdx, iMergeRefIdx, cMergeMv , cMergeIc, uiMergeInterDir, uiMergeIndex, uiMergeCost );
#else
      xMergeEstimation( pcCU, pcOrgYuv, iPartIdx, iMergeRefIdx, cMergeMv , uiMergeInterDir, uiMergeIndex, uiMergeCost );
#endif

      if( pcCU->getSlice()->isInterP() )
      {
        if ( uiMergeCost < uiCost[0] )
        {
          uiCost[ 0 ] = uiMergeCost;
          // set motion
          cMv[0] = cMergeMv[0];
          cMvPred[0][iMergeRefIdx[0]] = cMergeMv[0];
          iRefIdx[0] = iMergeRefIdx[0];
          
          aaiMvpIdx[0][iRefIdx[0]] = 0;
          aaiMvpNum[0][iRefIdx[0]] = 0;
          
          pcCU->setMergeFlagSubParts( true, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
          pcCU->setMergeIndexSubParts( uiMergeIndex, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));   
        }
      }
      else // Slice B
      {
        if ( uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1] )
        {
          uiBestCost = uiCostBi;
        }
        else if ( uiCost[0] <= uiCost[1] )
        {
          uiBestCost = uiCost[0];
        }
        else
        {
          uiBestCost = uiCost[1];
        }
        if ( uiMergeCost < uiBestCost )
        {
          if ( uiMergeInterDir == 3 )
          {
            uiCostBi = uiMergeCost;
            // set motion
            cMvBi[0]      = cMergeMv[0];
            cMvPredBi[0][iMergeRefIdx[0]] = cMergeMv[0];
            cMvBi[1]      = cMergeMv[1];
            cMvPredBi[1][iMergeRefIdx[1]]  = cMergeMv[1];
            iRefIdxBi[0] = iMergeRefIdx[0];
            iRefIdxBi[1] = iMergeRefIdx[1];

            aaiMvpIdxBi[0][iRefIdxBi[0]] = 0;
            aaiMvpNum[0][iRefIdxBi[0]] = 0; 
            aaiMvpIdxBi[1][iRefIdxBi[1]] = 0;
            aaiMvpNum[1][iRefIdxBi[1]] = 0; 

            pcCU->setMergeFlagSubParts( true, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
            pcCU->setMergeIndexSubParts( uiMergeIndex, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));   
          } 
          else
          {
            uiCost[ uiMergeInterDir-1 ] = uiMergeCost;
            // set motion
            cMv[uiMergeInterDir-1] = cMergeMv[uiMergeInterDir-1];
            cMvPred[uiMergeInterDir-1][iMergeRefIdx[uiMergeInterDir-1]] = cMergeMv[uiMergeInterDir-1];
            iRefIdx[uiMergeInterDir-1] = iMergeRefIdx[ uiMergeInterDir-1 ];
            
            aaiMvpIdx[uiMergeInterDir-1][iRefIdx[uiMergeInterDir-1]] = 0;
            aaiMvpNum[uiMergeInterDir-1][iRefIdx[uiMergeInterDir-1]] = 0;

            pcCU->setMergeFlagSubParts( true, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
            pcCU->setMergeIndexSubParts( uiMergeIndex, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));   
          }
        }
      }
    }
#endif

    //  Clear Motion Field
#ifdef QC_AMVRES
    if (pcCU->getSlice()->getSPS()->getUseAMVRes())
    {
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField_AMVRes( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField_AMVRes( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
    }
    else
#endif
    {
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
    }
    pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( cMvZero,            ePartSize, uiPartAddr, iPartIdx, 0 );
    pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( cMvZero,            ePartSize, uiPartAddr, iPartIdx, 0 );
    
    pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    
    // Set Motion Field_
#if MS_NO_BACK_PRED_IN_B0
    if ( pcCU->getSlice()->getNoBackPredFlag() )
    {
      uiCost[1] = MAX_UINT;
    }
#endif
    if ( uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1])
    {
      uiLastMode = 2;
#ifdef QC_AMVRES
      if (pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField_AMVRes( cMvBi[0], iRefIdxBi[0], ePartSize, uiPartAddr, iPartIdx, 0 );
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField_AMVRes( cMvBi[1], iRefIdxBi[1], ePartSize, uiPartAddr, iPartIdx, 0 );
      }
      else
#endif
      {
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMvBi[0], iRefIdxBi[0], ePartSize, uiPartAddr, iPartIdx, 0 );
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMvBi[1], iRefIdxBi[1], ePartSize, uiPartAddr, iPartIdx, 0 );
      }
#ifdef QC_AMVRES
      if( !cMvBi[0].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes() )
      {
        TempMv.set((cMvBi[0].getHor()/2-cMvPredBi[0][iRefIdxBi[0]].getHor()/2)*2,(cMvBi[0].getVer()/2-cMvPredBi[0][iRefIdxBi[0]].getVer()/2)*2 ) ;
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP 
        if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[0][iRefIdxBi[0]], REF_PIC_LIST_0, uiPartAddr, iRefIdxBi[0] )) 
#else			
          if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[0][iRefIdxBi[0]]))
#endif
          {
            aaiMvpIdxBi[0][iRefIdxBi[0]] = pcCU->searchMVPIdx_one_fourth(cMvPredBi[0][iRefIdxBi[0]], &aacAMVPInfo[0][iRefIdxBi[0]]);
            aaiMvpNum[0][iRefIdxBi[0]] = aacAMVPInfo[0][iRefIdxBi[0]].iN;
          }
      }
      else
#endif
      {
        TempMv = cMvBi[0] - cMvPredBi[0][iRefIdxBi[0]];
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if ( pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdxBi[0]], REF_PIC_LIST_0, uiPartAddr, iRefIdxBi[0] )) 
#else
          if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdxBi[0]]))
#endif
          {
            aaiMvpIdxBi[0][iRefIdxBi[0]] = pcCU->searchMVPIdx(cMvPredBi[0][iRefIdxBi[0]], &aacAMVPInfo[0][iRefIdxBi[0]]);
            aaiMvpNum[0][iRefIdxBi[0]] = aacAMVPInfo[0][iRefIdxBi[0]].iN;
          }
      }
#ifdef QC_AMVRES
      if( !cMvBi[1].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        TempMv.set((cMvBi[1].getHor()/2-cMvPredBi[1][iRefIdxBi[1]].getHor()/2)*2,(cMvBi[1].getVer()/2-cMvPredBi[1][iRefIdxBi[1]].getVer()/2) *2 ) ;
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[1][iRefIdxBi[1]], REF_PIC_LIST_1, uiPartAddr, iRefIdxBi[1] ) )
#else			
          if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[1][iRefIdxBi[1]]))
#endif
          {
            aaiMvpIdxBi[1][iRefIdxBi[1]] = pcCU->searchMVPIdx_one_fourth(cMvPredBi[1][iRefIdxBi[1]], &aacAMVPInfo[1][iRefIdxBi[1]]);
            aaiMvpNum[1][iRefIdxBi[1]] = aacAMVPInfo[1][iRefIdxBi[1]].iN;
          }
      }
      else
#endif
      {
        TempMv = cMvBi[1] - cMvPredBi[1][iRefIdxBi[1]];
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if ( pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdxBi[1]], REF_PIC_LIST_1, uiPartAddr, iRefIdxBi[1] ) )
#else
          if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdxBi[1]]))
#endif
          {
            aaiMvpIdxBi[1][iRefIdxBi[1]] = pcCU->searchMVPIdx(cMvPredBi[1][iRefIdxBi[1]], &aacAMVPInfo[1][iRefIdxBi[1]]);
            aaiMvpNum[1][iRefIdxBi[1]] = aacAMVPInfo[1][iRefIdxBi[1]].iN;
          }
      }
      
      pcCU->setInterDirSubParts( 3, uiPartAddr, iPartIdx, pcCU->getDepth(0) );
      
      pcCU->setMVPIdxSubParts( aaiMvpIdxBi[0][iRefIdxBi[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[0][iRefIdxBi[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPIdxSubParts( aaiMvpIdxBi[1][iRefIdxBi[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[1][iRefIdxBi[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    }
    else if ( uiCost[0] <= uiCost[1] )
    {
      uiLastMode = 0;
#ifdef QC_AMVRES 
      if (pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField_AMVRes( cMv[0], iRefIdx[0], ePartSize, uiPartAddr, iPartIdx, 0 );
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMVRes      ( false , ePartSize, uiPartAddr, iPartIdx, 0 );
      }
      else
#endif
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMv[0],   iRefIdx[0],   ePartSize, uiPartAddr, iPartIdx, 0 );
      
#ifdef QC_AMVRES
      if( !cMv[0].isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        TempMv.set(((cMv[0].getHor()/2)-(cMvPred[0][iRefIdx[0]].getHor()/2))*2,((cMv[0].getVer()/2)-(cMvPred[0][iRefIdx[0]].getVer()/2))*2 ) ;
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iRefIdx[0] ) )
#else		  
          if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[0][iRefIdx[0]]))
#endif
          {
            aaiMvpIdx[0][iRefIdx[0]] = pcCU->searchMVPIdx_one_fourth(cMvPred[0][iRefIdx[0]], &aacAMVPInfo[0][iRefIdx[0]]);
            aaiMvpNum[0][iRefIdx[0]] = aacAMVPInfo[0][iRefIdx[0]].iN;
          }
      }
      else
#endif
      {
        TempMv = cMv[0] - cMvPred[0][iRefIdx[0]];
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if ( pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iRefIdx[0] ) )
#else
          if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdx[0]]))
#endif
          {
            aaiMvpIdx[0][iRefIdx[0]] = pcCU->searchMVPIdx(cMvPred[0][iRefIdx[0]], &aacAMVPInfo[0][iRefIdx[0]]);
            aaiMvpNum[0][iRefIdx[0]] = aacAMVPInfo[0][iRefIdx[0]].iN;
          }
      }
      pcCU->setInterDirSubParts( 1, uiPartAddr, iPartIdx, pcCU->getDepth(0) );
      
      pcCU->setMVPIdxSubParts( aaiMvpIdx[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    }
    else
    {
      uiLastMode = 1;
#ifdef QC_AMVRES
      if (pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMVRes      ( false , ePartSize, uiPartAddr, iPartIdx, 0 );
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField_AMVRes( cMv[1], iRefIdx[1], ePartSize, uiPartAddr, iPartIdx, 0 );
      }
      else
#endif
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMv[1],   iRefIdx[1],   ePartSize, uiPartAddr, iPartIdx, 0 );
      
#ifdef QC_AMVRES
      if( !cMv[1].isHAM() && pcCU->getSlice()->getSPS()->getUseAMVRes())
      {
        TempMv.set(((cMv[1].getHor()/2)-(cMvPred[1][iRefIdx[1]].getHor()/2))*2,((cMv[1].getVer()/2)-(cMvPred[1][iRefIdx[1]].getVer()/2))*2 ) ;
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iRefIdx[1] ) )
#else			
          if (pcCU->clearMVPCand_one_fourth(TempMv, &aacAMVPInfo[1][iRefIdx[1]]))
#endif
          {
            aaiMvpIdx[1][iRefIdx[1]] = pcCU->searchMVPIdx_one_fourth(cMvPred[1][iRefIdx[1]], &aacAMVPInfo[1][iRefIdx[1]]);
            aaiMvpNum[1][iRefIdx[1]] = aacAMVPInfo[1][iRefIdx[1]].iN;
          }
      }
      else
#endif
      {
        TempMv = cMv[1] - cMvPred[1][iRefIdx[1]];
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
#if HHI_IMVP
        if ( pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iRefIdx[1] ) )
#else
          if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdx[1]]))
#endif
          {
            aaiMvpIdx[1][iRefIdx[1]] = pcCU->searchMVPIdx(cMvPred[1][iRefIdx[1]], &aacAMVPInfo[1][iRefIdx[1]]);
            aaiMvpNum[1][iRefIdx[1]] = aacAMVPInfo[1][iRefIdx[1]].iN;
          }
      }
      pcCU->setInterDirSubParts( 2, uiPartAddr, iPartIdx, pcCU->getDepth(0) );
      
      pcCU->setMVPIdxSubParts( aaiMvpIdx[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    }
    
#if HHI_MRG_PU && defined(DCM_PBIC)
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      if (pcCU->getMergeFlag(uiPartAddr) == false)
        cMergeIc.reset();
      pcCU->getCUIcField()->setAllIc( cMergeIc, ePartSize, uiPartAddr, iPartIdx, 0 );
    }
#endif

    //  MC
    motionCompensation ( pcCU, rpcPredYuv, REF_PIC_LIST_X, iPartIdx );

#ifdef DCM_PBIC
#if HHI_MRG_PU
    if (pcCU->getSlice()->getSPS()->getUseIC() && (pcCU->getMergeFlag(uiPartAddr) == false))
#else
    if (pcCU->getSlice()->getSPS()->getUseIC())
#endif
    {
      predICompSearch(pcCU, pcOrgYuv, rpcPredYuv, iPartIdx, uiLastMode);
    }
#endif

  } //  end of for ( Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++ )
  
  return;
}

#if HHI_IMVP
Void TEncSearch::xEstimateMvPredIMVP( TComDataCU* pcCU, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred, MvPredMeasure&  rcMvPredMeasure )
{  
  Int iMvHor = 0;
  
  pcCU->getYThresXPredLists( uiPartIdx, eRefPicList, iRefIdx, rcMvPredMeasure.m_iYOrigin, rcMvPredMeasure.m_aiYThreshold, rcMvPredMeasure.m_aiXOrigin ) ;   
  rcMvPredMeasure.m_iYOrigin = rcMvPred.getVer();
  pcCU->getMvPredXDep( eRefPicList, uiPartIdx, iRefIdx, rcMvPredMeasure.m_iYOrigin, iMvHor );
  rcMvPred.setHor( iMvHor );
}
#endif

// AMVP
Void TEncSearch::xEstimateMvPredAMVP( TComDataCU* pcCU, TComYuv* pcOrgYuv, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred, Bool bFilled )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();
  
  TComMv  cBestMv;
  Int     iBestIdx = 0;
  TComMv  cZeroMv;
  TComMv  cMvPred;
  UInt    uiBestCost = MAX_INT;
  UInt    uiPartAddr = 0;
  Int     iRoiWidth, iRoiHeight;
  Int     i;
  
  pcCU->getPartIndexAndSize( uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );
  // Fill the MV Candidates
  if (!bFilled)
  {
    pcCU->fillMvpCand( uiPartIdx, uiPartAddr, eRefPicList, iRefIdx, pcAMVPInfo );
  }
  
  // initialize Mvp index & Mvp
  iBestIdx = 0;
  cBestMv  = pcAMVPInfo->m_acMvCand[0];
  
  if( pcCU->getAMVPMode(uiPartAddr) == AM_NONE || (pcAMVPInfo->iN <= 1 && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    rcMvPred = cBestMv;
    
    pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    return;
  }
  
  if (pcCU->getAMVPMode(uiPartAddr) == AM_EXPL && bFilled)
  {
    assert(pcCU->getMVPIdx(eRefPicList,uiPartAddr) >= 0);
    rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,uiPartAddr)];
    return;
  }
  
  if (pcCU->getAMVPMode(uiPartAddr) == AM_EXPL)
  {
    m_cYuvPredTemp.clear();
    
    //-- Check Minimum Cost.
    for ( i = 0 ; i < pcAMVPInfo->iN; i++)
    {
      UInt uiTmpCost;
      
      uiTmpCost = xGetTemplateCost( pcCU, uiPartIdx, uiPartAddr, pcOrgYuv, &m_cYuvPredTemp, pcAMVPInfo->m_acMvCand[i], i, pcAMVPInfo->iN, eRefPicList, iRefIdx, iRoiWidth, iRoiHeight);
      
      if ( uiBestCost > uiTmpCost )
      {
        uiBestCost = uiTmpCost;
        cBestMv   = pcAMVPInfo->m_acMvCand[i];
        iBestIdx  = i;
      }
    }
    
    m_cYuvPredTemp.clear();
  }
  
  // Setting Best MVP
  rcMvPred = cBestMv;
  pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
  pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
  return;
}

UInt TEncSearch::xGetMvpIdxBits(Int iIdx, Int iNum)
{
  assert(iIdx >= 0 && iNum >= 0 && iIdx < iNum);
  
  if (iNum == 1)
    return 0;
  
  UInt uiLength = 1;
  Int iTemp = iIdx;
  if ( iTemp == 0 )
  {
    return uiLength;
  }
  
  Bool bCodeLast = ( iNum-1 > iTemp );
  
  uiLength += (iTemp-1);
  
  if( bCodeLast )
  {
    uiLength++;
  }
  
  return uiLength;
}

Void TEncSearch::xGetBlkBits( PartSize eCUMode, Bool bPSlice, Int iPartIdx, UInt uiLastMode, UInt uiBlkBit[3])
{
  if ( eCUMode == SIZE_2Nx2N )
  {
    uiBlkBit[0] = (! bPSlice) ? 3 : 1;
    uiBlkBit[1] = 3;
    uiBlkBit[2] = 5;
  }
  else if ( (eCUMode == SIZE_2NxN || eCUMode == SIZE_2NxnU) || eCUMode == SIZE_2NxnD )
  {
    UInt aauiMbBits[2][3][3] = { { {0,0,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7,5,7}, {9-3,9-3,9-3} } };
    if ( bPSlice )
    {
      uiBlkBit[0] = 3;
      uiBlkBit[1] = 0;
      uiBlkBit[2] = 0;
    }
    else
    {
      ::memcpy( uiBlkBit, aauiMbBits[iPartIdx][uiLastMode], 3*sizeof(UInt) );
    }
  }
  else if ( (eCUMode == SIZE_Nx2N || eCUMode == SIZE_nLx2N) || eCUMode == SIZE_nRx2N )
  {
    UInt aauiMbBits[2][3][3] = { { {0,2,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7-2,7-2,9-2}, {9-3,9-3,9-3} } };
    if ( bPSlice )
    {
      uiBlkBit[0] = 3;
      uiBlkBit[1] = 0;
      uiBlkBit[2] = 0;
    }
    else
    {
      ::memcpy( uiBlkBit, aauiMbBits[iPartIdx][uiLastMode], 3*sizeof(UInt) );
    }
  }
  else if ( eCUMode == SIZE_NxN )
  {
    uiBlkBit[0] = (! bPSlice) ? 3 : 1;
    uiBlkBit[1] = 3;
    uiBlkBit[2] = 5;
  }
  else
  {
    printf("Wrong!\n");
    assert( 0 );
  }
}

Void TEncSearch::xCopyAMVPInfo (AMVPInfo* pSrc, AMVPInfo* pDst)
{
  pDst->iN = pSrc->iN;
  for (Int i = 0; i < pSrc->iN; i++)
  {
    pDst->m_acMvCand[i] = pSrc->m_acMvCand[i];
  }
}
#ifdef QC_AMVRES 
Void TEncSearch::xCheckBestMVP_onefourth ( TComDataCU* pcCU, RefPicList eRefPicList, TComMv cMv, TComMv& rcMvPred, Int& riMVPIdx, UInt& ruiBits, UInt& ruiCost )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();
  
#if HHI_IMVP
  TComMv cMvPredIMP;
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    cMvPredIMP = m_cMvPredMeasure.getMVPred( cMv.getHor(), cMv.getVer() );
    assert( pcAMVPInfo->m_acMvCand[riMVPIdx].getVer() == rcMvPred.getVer() );
    assert( cMvPredIMP.getHor() == rcMvPred.getHor() );
  }
  else
  {
    assert(pcAMVPInfo->m_acMvCand[riMVPIdx] == rcMvPred);
  }
#else
  assert(pcAMVPInfo->m_acMvCand[riMVPIdx] == rcMvPred);
#endif
  if (pcAMVPInfo->iN < 2) return;
  
  m_pcRdCost->getMotionCost( 1, 0 );
  m_pcRdCost->setCostScale ( 0    );
  
  Int iBestMVPIdx = riMVPIdx;
  
  TComMv rcMvPred_round=rcMvPred,cMv_round=cMv;
  
  rcMvPred_round.scale_down();
  cMv_round.scale_down();
  m_pcRdCost->setPredictor( rcMvPred_round );
  
  
  Int iOrgMvBits  = m_pcRdCost->getBits(cMv_round.getHor(), cMv_round.getVer());
  Int iBestMvBits = iOrgMvBits;
  
  for (Int iMVPIdx = 0; iMVPIdx < pcAMVPInfo->iN; iMVPIdx++)
  {
    if (iMVPIdx == riMVPIdx) continue;
    
#if HHI_IMVP
    if ( pcCU->getSlice()->getSPS()->getUseIMP() )
    {
      TComMv rcMvPred_round_curr= rcMvPred;
      rcMvPred_round_curr.setVer( pcAMVPInfo->m_acMvCand[iMVPIdx].getVer() );
      rcMvPred_round_curr.scale_down();
      m_pcRdCost->setPredictor( rcMvPred_round_curr);
    }
    else
    {
      TComMv rcMvPred_round_curr=pcAMVPInfo->m_acMvCand[iMVPIdx];
      rcMvPred_round_curr.scale_down();
      m_pcRdCost->setPredictor( rcMvPred_round_curr);
    }
#else
    TComMv rcMvPred_round_curr=pcAMVPInfo->m_acMvCand[iMVPIdx];
    rcMvPred_round_curr.scale_down();
    m_pcRdCost->setPredictor( rcMvPred_round_curr);
#endif
    Int iMvBits = m_pcRdCost->getBits(cMv_round.getHor(), cMv_round.getVer());
    
    if (iMvBits < iBestMvBits)
    {
      iBestMvBits = iMvBits;
      iBestMVPIdx = iMVPIdx;
    }
  }
  
  if (iBestMVPIdx != riMVPIdx)	//if changed
  {
#if HHI_IMVP
    if ( pcCU->getSlice()->getSPS()->getUseIMP() )
    {
      m_cMvPredMeasure.m_iYOrigin = pcAMVPInfo->m_acMvCand[iBestMVPIdx].getVer();
      rcMvPred.setVer( pcAMVPInfo->m_acMvCand[iBestMVPIdx].getVer() );
      rcMvPred.setHor( cMvPredIMP.getHor() );
    }
    else
    {
      rcMvPred = pcAMVPInfo->m_acMvCand[iBestMVPIdx];
    }
#else
    rcMvPred = pcAMVPInfo->m_acMvCand[iBestMVPIdx];
#endif
    iOrgMvBits  += m_auiMVPIdxCost[riMVPIdx][pcAMVPInfo->iN];
    iBestMvBits += m_auiMVPIdxCost[iBestMVPIdx][pcAMVPInfo->iN];
    
    riMVPIdx = iBestMVPIdx;
    UInt uiOrgBits = ruiBits;
    ruiBits = uiOrgBits - iOrgMvBits + iBestMvBits;
    ruiCost = (ruiCost - m_pcRdCost->getCost( uiOrgBits ))	+ m_pcRdCost->getCost( ruiBits );
  }
}
#endif 
Void TEncSearch::xCheckBestMVP ( TComDataCU* pcCU, RefPicList eRefPicList, TComMv cMv, TComMv& rcMvPred, Int& riMVPIdx, UInt& ruiBits, UInt& ruiCost )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();
  
#if HHI_IMVP
  TComMv cMvPredIMP;
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    cMvPredIMP = m_cMvPredMeasure.getMVPred( cMv.getHor(), cMv.getVer() );
    assert( pcAMVPInfo->m_acMvCand[riMVPIdx].getVer() == rcMvPred.getVer() );
    assert( cMvPredIMP.getHor() == rcMvPred.getHor() );
  }
  else
  {
    assert(pcAMVPInfo->m_acMvCand[riMVPIdx] == rcMvPred);
  }
#else
  assert(pcAMVPInfo->m_acMvCand[riMVPIdx] == rcMvPred);
#endif
  
  if (pcAMVPInfo->iN < 2) return;
  
  m_pcRdCost->getMotionCost( 1, 0 );
  m_pcRdCost->setCostScale ( 0    );
  
  Int iBestMVPIdx = riMVPIdx;
  
  m_pcRdCost->setPredictor( rcMvPred );
  Int iOrgMvBits  = m_pcRdCost->getBits(cMv.getHor(), cMv.getVer());
  Int iBestMvBits = iOrgMvBits;
  
  for (Int iMVPIdx = 0; iMVPIdx < pcAMVPInfo->iN; iMVPIdx++)
  {
    if (iMVPIdx == riMVPIdx) continue;
    
#if HHI_IMVP
    if ( pcCU->getSlice()->getSPS()->getUseIMP() )
    {
      cMvPredIMP.setVer( pcAMVPInfo->m_acMvCand[iMVPIdx].getVer() );
      m_pcRdCost->setPredictor( cMvPredIMP );
    }
    else
    {
      m_pcRdCost->setPredictor( pcAMVPInfo->m_acMvCand[iMVPIdx] );
    }
#else
    m_pcRdCost->setPredictor( pcAMVPInfo->m_acMvCand[iMVPIdx] );
#endif
    
    Int iMvBits = m_pcRdCost->getBits(cMv.getHor(), cMv.getVer());
    
    if (iMvBits < iBestMvBits)
    {
      iBestMvBits = iMvBits;
      iBestMVPIdx = iMVPIdx;
    }
  }
  
  if (iBestMVPIdx != riMVPIdx)  //if changed
  {
#if HHI_IMVP
    if ( pcCU->getSlice()->getSPS()->getUseIMP() )
    {
      m_cMvPredMeasure.m_iYOrigin = pcAMVPInfo->m_acMvCand[iBestMVPIdx].getVer();
      rcMvPred.setVer( pcAMVPInfo->m_acMvCand[iBestMVPIdx].getVer() );
      rcMvPred.setHor( cMvPredIMP.getHor() );
    }
    else
    {
      rcMvPred = pcAMVPInfo->m_acMvCand[iBestMVPIdx];
    }
#else
    rcMvPred = pcAMVPInfo->m_acMvCand[iBestMVPIdx];
#endif
    
    iOrgMvBits  += m_auiMVPIdxCost[riMVPIdx][pcAMVPInfo->iN];
    iBestMvBits += m_auiMVPIdxCost[iBestMVPIdx][pcAMVPInfo->iN];
    
    riMVPIdx = iBestMVPIdx;
    UInt uiOrgBits = ruiBits;
    ruiBits = uiOrgBits - iOrgMvBits + iBestMvBits;
    ruiCost = (ruiCost - m_pcRdCost->getCost( uiOrgBits ))  + m_pcRdCost->getCost( ruiBits );
  }
}

UInt TEncSearch::xGetTemplateCost( TComDataCU* pcCU,
                                  UInt        uiPartIdx,
                                  UInt      uiPartAddr,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*    pcTemplateCand,
                                  TComMv      cMvCand,
                                  Int         iMVPIdx,
                                  Int     iMVPNum,
                                  RefPicList  eRefPicList,
                                  Int         iRefIdx,
                                  Int         iSizeX,
                                  Int         iSizeY)
{
  UInt uiCost  = MAX_INT;
  
  TComPicYuv* pcPicYuvRef = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();
  
  // prediction pattern
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
    xPredInterLumaBlkHMV( pcCU, pcPicYuvRef, uiPartAddr, &cMvCand, iSizeX, iSizeY, pcTemplateCand );
  else
#endif
    xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvCand, iSizeX, iSizeY, pcTemplateCand );
  
  // calc distortion
  uiCost = m_pcRdCost->getDistPart( pcTemplateCand->getLumaAddr(uiPartAddr), pcTemplateCand->getStride(), pcOrgYuv->getLumaAddr(uiPartAddr), pcOrgYuv->getStride(), iSizeX, iSizeY, DF_SAD );
  uiCost = (UInt) m_pcRdCost->calcRdCost( m_auiMVPIdxCost[iMVPIdx][iMVPNum], uiCost, false, DF_SAD );
  return uiCost;
}

#ifdef DCM_PBIC
UInt TEncSearch::xGetTemplateCostIC   ( TComDataCU* pcCU,
                                        UInt        uiPartIdx,
                                        UInt        uiPartAddr,
                                        TComYuv*    pcOrgYuv,
                                        TComIc      cIcCand,
                                        Int         iICPIdx,
                                        Int         iICPNum,
                                        RefPicList  eRefPicList,
                                        Int         iSizeX,
                                        Int         iSizeY)
{
  UInt uiCost  = MAX_INT;

  //Apply IC
  if (eRefPicList == REF_PIC_LIST_X )
  {
    Int iRefIdx[2];
    iRefIdx[0] = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiPartAddr );
    iRefIdx[1] = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiPartAddr );

    xPredICompLumaBlk  ( &cIcCand, iSizeX   , iSizeY   , m_acYuvTempIC[0].getStride() , 1, m_acYuvTempIC[0].getLumaAddr(uiPartAddr), m_acYuvPred[0].getStride() , 1, m_acYuvPred[0].getLumaAddr(uiPartAddr), REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCbAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCbAddr(uiPartAddr)  , REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCrAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCrAddr(uiPartAddr)  , REF_PIC_LIST_0 );

    xPredICompLumaBlk  ( &cIcCand, iSizeX   , iSizeY   , m_acYuvTempIC[1].getStride() , 1, m_acYuvTempIC[1].getLumaAddr(uiPartAddr), m_acYuvPred[1].getStride() , 1, m_acYuvPred[1].getLumaAddr(uiPartAddr), REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCbAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCbAddr(uiPartAddr)  , REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCrAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCrAddr(uiPartAddr)  , REF_PIC_LIST_1 );

    TComYuv* pcYuvPred = m_acYuvTempIC;
    xWeightedAverage( pcCU, m_acYuvTempIC, m_acYuvTempIC+1, iRefIdx[0], iRefIdx[1], uiPartAddr, iSizeX, iSizeY, pcYuvPred);
  }
  else
  {
    xPredICompLumaBlk  ( &cIcCand, iSizeX   , iSizeY   , m_acYuvTempIC->getStride() , 1, m_acYuvTempIC->getLumaAddr(uiPartAddr), m_acYuvPred[eRefPicList].getStride() , 1, m_acYuvPred[eRefPicList].getLumaAddr(uiPartAddr), eRefPicList );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC->getCStride(), 1, m_acYuvTempIC->getCbAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCbAddr(uiPartAddr)  , eRefPicList );
    xPredICompChromaBlk( &cIcCand, iSizeX>>1, iSizeY>>1, m_acYuvTempIC->getCStride(), 1, m_acYuvTempIC->getCrAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCrAddr(uiPartAddr)  , eRefPicList );
  }

  // calc distortion
  DFunc eDFunc = (m_pcEncCfg->getUseHADME() == true) ? DF_HADS : DF_SADS;
  uiCost = m_pcRdCost->getDistPart( m_acYuvTempIC->getLumaAddr(uiPartAddr), m_acYuvTempIC->getStride(), pcOrgYuv->getLumaAddr(uiPartAddr), pcOrgYuv->getStride(), iSizeX, iSizeY, eDFunc );
  uiCost += UInt( m_pcRdCost->getDistPart( m_acYuvTempIC->getCbAddr(uiPartAddr), m_acYuvTempIC->getCStride(),pcOrgYuv->getCbAddr(uiPartAddr),pcOrgYuv->getCStride(), iSizeX>>1, iSizeY>>1, eDFunc)
                + m_pcRdCost->getDistPart( m_acYuvTempIC->getCrAddr(uiPartAddr), m_acYuvTempIC->getCStride(),pcOrgYuv->getCrAddr(uiPartAddr),pcOrgYuv->getCStride(), iSizeX>>1, iSizeY>>1, eDFunc));
  uiCost =  (UInt) ( uiCost / 1.5 );

  uiCost = (UInt) m_pcRdCost->calcRdCost( m_auiICPIdxCost[iICPIdx][iICPNum], uiCost, false, DF_SAD );

	return uiCost;
}
#endif

#if MS_NO_BACK_PRED_IN_B0
Void TEncSearch::xMotionEstimationForL1( TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPicList eRefPicList, TComMv* pcMvPred, Int iRefIdxPred, TComMv& rcMv, UInt& ruiBits, UInt& ruiCost, Bool bBi  )
{

  TComMv        cMvHalf, cMvQter;
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;




  TComMv      cMvPred = *pcMvPred;

#ifdef QC_AMVRES
  TComMv      cMvPred_onefourth;
#endif


  m_pcRdCost->setCostScale( 0 );
#ifdef QC_AMVRES 

  UInt uiMvBits;

#if HHI_IMVP
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    *pcMvPred = m_cMvPredMeasure.getMVPred( rcMv.getHor(), rcMv.getVer() ) ;
  }
#endif

  if (!rcMv.isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    cMvPred_onefourth=*pcMvPred;
    cMvPred_onefourth.scale_down();
    m_pcRdCost->setPredictor	( cMvPred_onefourth );
    uiMvBits = m_pcRdCost->getBits( rcMv.getHor()/2, rcMv.getVer()/2 );
  }
  else
  {
    m_pcRdCost->setPredictor	( *pcMvPred );
    uiMvBits = m_pcRdCost->getBits( rcMv.getHor(), rcMv.getVer())+pcCU->getSlice()->getSPS()->getUseAMVRes();
  }
#else
  rcMv <<= 2;
  rcMv += (cMvHalf <<= 1);
  rcMv +=  cMvQter;

#if HHI_IMVP
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    *pcMvPred = m_cMvPredMeasure.getMVPred( rcMv.getHor(), rcMv.getVer() ) ;
    m_pcRdCost->setPredictor(*pcMvPred); 
  }
#endif

  UInt uiMvBits = m_pcRdCost->getBits( rcMv.getHor(), rcMv.getVer() );
#endif

  ruiBits      += uiMvBits;
 
}

#endif

Void TEncSearch::xMotionEstimation( TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPicList eRefPicList, TComMv* pcMvPred, Int iRefIdxPred, TComMv& rcMv, UInt& ruiBits, UInt& ruiCost, Bool bBi  )
{
  UInt          uiPartAddr;
  Int           iRoiWidth;
  Int           iRoiHeight;
  
  TComMv        cMvHalf, cMvQter;
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;
  
  TComYuv*  pcYuv = pcYuvOrg;
#ifdef ROUNDING_CONTROL_BIPRED
  Pel			pRefBufY[16384];  // 128x128
#endif
#ifdef QC_SIFO
  Int iList = (eRefPicList==REF_PIC_LIST_0)? 0 : 1;
  TComMv cMv_start = rcMv;
  Int FullpelOffset;
#endif
  m_iSearchRange = m_aaiAdaptSR[eRefPicList][iRefIdxPred];
  
  Int           iSrchRng      = ( bBi ? 8 : m_iSearchRange );
  TComPattern*  pcPatternKey  = pcCU->getPattern        ();
  
  Double        fWeight       = 1.0;
  
  pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );
  
  if ( bBi )
  {
    TComYuv*  pcYuvOther = &m_acYuvPred[1-(Int)eRefPicList];
    pcYuv                = &m_cYuvPredTemp;
    
    pcYuvOrg->copyPartToPartYuv( pcYuv, uiPartAddr, iRoiWidth, iRoiHeight );
    
#ifdef ROUNDING_CONTROL_BIPRED
	Int y;
	//Int x;
	Pel *pRefY = pcYuvOther->getLumaAddr(uiPartAddr);
	Int iRefStride = pcYuvOther->getStride();
    
	// copy the MC block into pRefBufY
	for( y = 0; y < iRoiHeight; y++)
	{
      memcpy(pRefBufY+y*iRoiWidth,pRefY,sizeof(Pel)*iRoiWidth);
      pRefY += iRefStride;
	}
#else
    pcYuv->removeHighFreq( pcYuvOther, uiPartAddr, iRoiWidth, iRoiHeight );
#endif
    
    fWeight = 0.5;
  }
  
  //  Search key pattern initialization
  pcPatternKey->initPattern( pcYuv->getLumaAddr( uiPartAddr ),
                            pcYuv->getCbAddr  ( uiPartAddr ),
                            pcYuv->getCrAddr  ( uiPartAddr ),
                            iRoiWidth,
                            iRoiHeight,
                            pcYuv->getStride(),
                            0, 0, 0, 0 );
  
  Pel*        piRefY      = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
  Int         iRefStride  = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRec()->getStride();
  
  TComMv      cMvPred = *pcMvPred;
  
#ifdef QC_AMVRES
  TComMv      cMvPred_onefourth;
#endif
  
  if ( bBi )  xSetSearchRange   ( pcCU, rcMv   , iSrchRng, cMvSrchRngLT, cMvSrchRngRB );
  else        xSetSearchRange   ( pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB );
  
  m_pcRdCost->getMotionCost ( 1, 0 );
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    cMvPred_onefourth = *pcMvPred;
    cMvPred_onefourth.scale_down();
    m_pcRdCost->setPredictor	( cMvPred_onefourth );
  }
  else
#endif
    m_pcRdCost->setPredictor  ( *pcMvPred );
  m_pcRdCost->setCostScale  ( 2 );
  
  //  Do integer search
#ifdef ROUNDING_CONTROL_BIPRED
  if( bBi ) 
  {
	xPatternSearch_Bi      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost, pRefBufY, pcCU->getSlice()->isRounding() );
  } 
  else
  {
    if ( !m_iFastSearch)
    {
      xPatternSearch      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
    }
    else
    {
      rcMv = *pcMvPred;
      xPatternSearchFast  ( pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
    }
  }
#else
  if ( !m_iFastSearch || bBi )
  {
    xPatternSearch      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
  }
  else
  {
    rcMv = *pcMvPred;
    xPatternSearchFast  ( pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
  }
#endif
  
#ifdef QC_SIFO
  UInt NumMEOffsets = getNum_Offset_FullpelME(iList); 
  if(pcCU->getSlice()->getUseSIFO() && NumMEOffsets>0 && iRefIdxPred==0)
  {
    for(UInt MEloop = 0; MEloop < NumMEOffsets; MEloop++)
    {
      UInt uiCostTemp = MAX_UINT;
      TComMv cMv_temp = cMv_start;  
      FullpelOffset = getOffset_FullpelME(iList,MEloop);
      xAddSubFullPelOffset(pcPatternKey, FullpelOffset, 1);
      //  Do integer search
#ifdef ROUNDING_CONTROL_BIPRED
	  if( bBi ) 
	  {
#if BUGFIX48
		xPatternSearch_Bi      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, cMv_temp, uiCostTemp, pRefBufY, pcCU->getSlice()->isRounding() );
#else
		xPatternSearch_Bi      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost, pRefBufY, pcCU->getSlice()->isRounding() );
#endif
	  }
	  else
	  {
        if (!m_iFastSearch)
        {
          xPatternSearch      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, cMv_temp, uiCostTemp );
        }
        else
        {
          cMv_temp = *pcMvPred;
          xPatternSearchFast  ( pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, cMv_temp, uiCostTemp );
        }
	  }
#else
      if ( !m_iFastSearch || bBi )
      {
        xPatternSearch      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, cMv_temp, uiCostTemp );
      }
      else
      {
        cMv_temp = *pcMvPred;
        xPatternSearchFast  ( pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, cMv_temp, uiCostTemp );
      }
#endif
      xAddSubFullPelOffset(pcPatternKey, FullpelOffset, 0);
      
      if(uiCostTemp < ruiCost)
      {
        rcMv = cMv_temp;
        ruiCost = uiCostTemp;
      }
    }
  }
#endif
  
  
  m_pcRdCost->getMotionCost( 1, 0 );
  m_pcRdCost->setCostScale ( 1 );
  
#ifdef ROUNDING_CONTROL_BIPRED
  if( bBi ) 
  {
	Bool bRound =  pcCU->getSlice()->isRounding() ;
#if HHI_INTERP_FILTER
    InterpFilterType ePFilt = (InterpFilterType)pcCU->getSlice()->getInterpFilterType();
    switch ( ePFilt )
    {
#ifdef QC_AMVRES
#ifdef QC_SIFO
      case IPF_QC_SIFO:
        xPatternSearchFracDIF_QC_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred, pRefBufY, bRound);
        break;
#endif
#if TEN_DIRECTIONAL_INTERP
      case IPF_TEN_DIF:
        xPatternSearchFracDIF_TEN_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost , pRefBufY, bRound);
        break;
#endif
      case IPF_HHI_4TAP_MOMS:
      case IPF_HHI_6TAP_MOMS:
        piRefY = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRecFilt()->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
        xPatternSearchFracMOMS_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, ePFilt ,pcMvPred,iRefIdxPred, pRefBufY, bRound);
        break;
      default:
        xPatternSearchFracDIF_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred, pRefBufY, bRound);  
#else
#ifdef QC_SIFO
      case IPF_QC_SIFO:
        xPatternSearchFracDIF_QC_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost , pRefBufY, bRound);
        break;
#endif
#if TEN_DIRECTIONAL_INTERP
      case IPF_TEN_DIF:
        xPatternSearchFracDIF_TEN_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost , pRefBufY, bRound);
        break;
#endif
      case IPF_HHI_4TAP_MOMS:
      case IPF_HHI_6TAP_MOMS:
        piRefY = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRecFilt()->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
        xPatternSearchFracMOMS_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, ePFilt , pRefBufY, bRound);
        break;
      default:
        xPatternSearchFracDIF_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, pRefBufY, bRound );
#endif
    }
#else
#ifdef QC_AMVRES
    xPatternSearchFracDIF_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred, pRefBufY, bRound);
#else
    xPatternSearchFracDIF_Bi( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, pRefBufY, bRound );
#endif
#endif
  }
  else
#endif
  {
#if HHI_INTERP_FILTER
    InterpFilterType ePFilt = (InterpFilterType)pcCU->getSlice()->getInterpFilterType();
    switch ( ePFilt )
    {
#ifdef QC_AMVRES
#ifdef QC_SIFO
      case IPF_QC_SIFO:
        xPatternSearchFracDIF_QC( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred);
        break;
#endif
#if TEN_DIRECTIONAL_INTERP
      case IPF_TEN_DIF:
        xPatternSearchFracDIF_TEN( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
        break;
#endif
      case IPF_HHI_4TAP_MOMS:
      case IPF_HHI_6TAP_MOMS:
        piRefY = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRecFilt()->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
        xPatternSearchFracMOMS( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, ePFilt ,pcMvPred,iRefIdxPred);
        break;
      default:
        xPatternSearchFracDIF( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred);
#else
#ifdef QC_SIFO
      case IPF_QC_SIFO:
        xPatternSearchFracDIF_QC( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
        break;
#endif
#if TEN_DIRECTIONAL_INTERP
      case IPF_TEN_DIF:
        xPatternSearchFracDIF_TEN( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
        break;
#endif
      case IPF_HHI_4TAP_MOMS:
      case IPF_HHI_6TAP_MOMS:
        piRefY = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdxPred )->getPicYuvRecFilt()->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
        xPatternSearchFracMOMS( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost, ePFilt );
        break;
      default:
        xPatternSearchFracDIF( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
#endif
    }
#else
#ifdef QC_AMVRES
    xPatternSearchFracDIF( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost,pcMvPred,iRefIdxPred);
#else
    xPatternSearchFracDIF( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
#endif
#endif
  }
  
  
  
  m_pcRdCost->setCostScale( 0 );
#ifdef QC_AMVRES 
  rcMv <<= (2+pcCU->getSlice()->getSPS()->getUseAMVRes());
  rcMv += (cMvHalf <<= (1+pcCU->getSlice()->getSPS()->getUseAMVRes()));
  rcMv +=  cMvQter;
  UInt uiMvBits;
  
#if HHI_IMVP
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    *pcMvPred = m_cMvPredMeasure.getMVPred( rcMv.getHor(), rcMv.getVer() ) ;
  }
#endif
  
  if (!rcMv.isHAM()&& pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    cMvPred_onefourth=*pcMvPred;
    cMvPred_onefourth.scale_down();
    m_pcRdCost->setPredictor	( cMvPred_onefourth );
    uiMvBits = m_pcRdCost->getBits( rcMv.getHor()/2, rcMv.getVer()/2 );
  }
  else
  {
    m_pcRdCost->setPredictor	( *pcMvPred );
    uiMvBits = m_pcRdCost->getBits( rcMv.getHor(), rcMv.getVer())+pcCU->getSlice()->getSPS()->getUseAMVRes();
  }
#else
  rcMv <<= 2;
  rcMv += (cMvHalf <<= 1);
  rcMv +=  cMvQter;
  
#if HHI_IMVP
  if ( pcCU->getSlice()->getSPS()->getUseIMP() )
  {
    *pcMvPred = m_cMvPredMeasure.getMVPred( rcMv.getHor(), rcMv.getVer() ) ;
    m_pcRdCost->setPredictor(*pcMvPred); 
  }
#endif
  
  UInt uiMvBits = m_pcRdCost->getBits( rcMv.getHor(), rcMv.getVer() );
#endif
  
  ruiBits      += uiMvBits;
  ruiCost       = (UInt)( floor( fWeight * ( (Double)ruiCost - (Double)m_pcRdCost->getCost( uiMvBits ) ) ) + (Double)m_pcRdCost->getCost( ruiBits ) );
}


Void TEncSearch::xSetSearchRange ( TComDataCU* pcCU, TComMv& cMvPred, Int iSrchRng, TComMv& rcMvSrchRngLT, TComMv& rcMvSrchRngRB )
{
#ifdef QC_AMVRES
  Int  iMvShift = (pcCU->getSlice()->getSPS()->getUseAMVRes())?3:2;
#else
  Int  iMvShift = 2;
#endif
  pcCU->clipMv( cMvPred );
  
  rcMvSrchRngLT.setHor( cMvPred.getHor() - (iSrchRng << iMvShift) );
  rcMvSrchRngLT.setVer( cMvPred.getVer() - (iSrchRng << iMvShift) );
  
  rcMvSrchRngRB.setHor( cMvPred.getHor() + (iSrchRng << iMvShift) );
  rcMvSrchRngRB.setVer( cMvPred.getVer() + (iSrchRng << iMvShift) );
  
  pcCU->clipMv        ( rcMvSrchRngLT );
  pcCU->clipMv        ( rcMvSrchRngRB );
  
  rcMvSrchRngLT >>= iMvShift;
  rcMvSrchRngRB >>= iMvShift;
}



#ifdef ROUNDING_CONTROL_BIPRED
Void TEncSearch::xPatternSearch_Bi( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD, Pel* pcRefY2, Bool bRound )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  UInt  uiSad;
  UInt  uiSadBest         = MAX_UINT;
  Int   iBestX = 0;
  Int   iBestY = 0;
  
  Pel*  piRefSrch;
  
  //-- jclee for using the SAD function pointer
  m_pcRdCost->setDistParam_Bi( pcPatternKey, piRefY, iRefStride,  m_cDistParam);
  
  // fast encoder decision: use subsampled SAD for integer ME
  if ( m_pcEncCfg->getUseFastEnc() )
  {
    if ( m_cDistParam.iRows > 8 )
    {
      m_cDistParam.iSubShift = 1;
    }
  }
  
  piRefY += (iSrchRngVerTop * iRefStride);
  for ( Int y = iSrchRngVerTop; y <= iSrchRngVerBottom; y++ )
  {
    for ( Int x = iSrchRngHorLeft; x <= iSrchRngHorRight; x++ )
    {
      //  find min. distortion position
      piRefSrch = piRefY + x;
      m_cDistParam.pCur = piRefSrch;
      uiSad = m_cDistParam.DistFuncRnd( &m_cDistParam, pcRefY2, bRound );
      
      // motion cost
#if HHI_IMVP
      if ( m_pcEncCfg->getUseIMP() )
      {
#ifdef QC_AMVRES
        if (m_pcEncCfg->getUseAMVRes() )
        {
          TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<3) , (y<<3) );
          cMvPred.scale_down();
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
        {
          TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<2) , (y<<2) );
          m_pcRdCost->setPredictor( cMvPred );
        }
#else
        TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<2) , (y<<2) );
        m_pcRdCost->setPredictor( cMvPred );
#endif
      }
#endif
      uiSad += m_pcRdCost->getCost( x, y );
      
      if ( uiSad < uiSadBest )
      {
        uiSadBest = uiSad;
        iBestX    = x;
        iBestY    = y;
      }
    }
    piRefY += iRefStride;
  }
  
  rcMv.set( iBestX, iBestY );
#if HHI_IMVP
  if ( m_pcEncCfg->getUseIMP() )
  {
#ifdef QC_AMVRES
    if (m_pcEncCfg->getUseAMVRes() )
    {
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<3) , (iBestY<<3) );
      cMvPred.scale_down();
      m_pcRdCost->setPredictor( cMvPred );
    }
    else
    {
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<2) , (iBestY<<2) );
      m_pcRdCost->setPredictor( cMvPred );
    }
#else
    TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<2) , (iBestY<<2) );
    m_pcRdCost->setPredictor( cMvPred );
#endif
  }
#endif
  ruiSAD = uiSadBest - m_pcRdCost->getCost( iBestX, iBestY );
  return;
}
#endif

Void TEncSearch::xPatternSearch( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  UInt  uiSad;
  UInt  uiSadBest         = MAX_UINT;
  Int   iBestX = 0;
  Int   iBestY = 0;
  
  Pel*  piRefSrch;
  
  //-- jclee for using the SAD function pointer
  m_pcRdCost->setDistParam( pcPatternKey, piRefY, iRefStride,  m_cDistParam );
  
  // fast encoder decision: use subsampled SAD for integer ME
  if ( m_pcEncCfg->getUseFastEnc() )
  {
    if ( m_cDistParam.iRows > 8 )
    {
      m_cDistParam.iSubShift = 1;
    }
  }
  
  piRefY += (iSrchRngVerTop * iRefStride);
  for ( Int y = iSrchRngVerTop; y <= iSrchRngVerBottom; y++ )
  {
    for ( Int x = iSrchRngHorLeft; x <= iSrchRngHorRight; x++ )
    {
      //  find min. distortion position
      piRefSrch = piRefY + x;
      m_cDistParam.pCur = piRefSrch;
      uiSad = m_cDistParam.DistFunc( &m_cDistParam );
      
      // motion cost
#if HHI_IMVP
      if ( m_pcEncCfg->getUseIMP() )
      {
#ifdef QC_AMVRES
        if (m_pcEncCfg->getUseAMVRes() )
        {
          TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<3) , (y<<3) );
          cMvPred.scale_down();
          m_pcRdCost->setPredictor( cMvPred );
        }
        else
        {
          TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<2) , (y<<2) );
          m_pcRdCost->setPredictor( cMvPred );
        }
#else
        TComMv cMvPred = m_cMvPredMeasure.getMVPred( (x<<2) , (y<<2) );
        m_pcRdCost->setPredictor( cMvPred );
#endif
      }
#endif
      
      uiSad += m_pcRdCost->getCost( x, y );
      
      if ( uiSad < uiSadBest )
      {
        uiSadBest = uiSad;
        iBestX    = x;
        iBestY    = y;
      }
    }
    piRefY += iRefStride;
  }
  
  rcMv.set( iBestX, iBestY );
  
#if HHI_IMVP
  if ( m_pcEncCfg->getUseIMP() )
  {
#ifdef QC_AMVRES
    if (m_pcEncCfg->getUseAMVRes() )
    {
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<3) , (iBestY<<3) );
      cMvPred.scale_down();
      m_pcRdCost->setPredictor( cMvPred );
    }
    else
    {
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<2) , (iBestY<<2) );
      m_pcRdCost->setPredictor( cMvPred );
    }
#else
    TComMv cMvPred = m_cMvPredMeasure.getMVPred( (iBestX<<2) , (iBestY<<2) );
    m_pcRdCost->setPredictor( cMvPred );
#endif
  }
#endif
  ruiSAD = uiSadBest - m_pcRdCost->getCost( iBestX, iBestY );
  return;
}

Void TEncSearch::xPatternSearchFast( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD )
{
  pcCU->getMvPredLeft       ( m_acMvPredictors[0] );
  pcCU->getMvPredAbove      ( m_acMvPredictors[1] );
  pcCU->getMvPredAboveRight ( m_acMvPredictors[2] );
  
  switch ( m_iFastSearch )
  {
    case 1:
      xTZSearch( pcCU, pcPatternKey, piRefY, iRefStride, pcMvSrchRngLT, pcMvSrchRngRB, rcMv, ruiSAD );
      break;
      
    default:
      break;
  }
}

Void TEncSearch::xTZSearch( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, TComMv& rcMv, UInt& ruiSAD )
{
  Int   iSrchRngHorLeft   = pcMvSrchRngLT->getHor();
  Int   iSrchRngHorRight  = pcMvSrchRngRB->getHor();
  Int   iSrchRngVerTop    = pcMvSrchRngLT->getVer();
  Int   iSrchRngVerBottom = pcMvSrchRngRB->getVer();
  
  TZ_SEARCH_CONFIGURATION
  
  UInt uiSearchRange = m_iSearchRange;
  pcCU->clipMv( rcMv );
#ifdef QC_AMVRES
  rcMv >>= (2+pcCU->getSlice()->getSPS()->getUseAMVRes());
#else
  rcMv >>= 2;
#endif
  // init TZSearchStruct
  IntTZSearchStruct cStruct;
  cStruct.iYStride    = iRefStride;
  cStruct.piRefY      = piRefY;
  cStruct.uiBestSad   = MAX_UINT;
  
  // set rcMv (Median predictor) as start point and as best point
  xTZSearchHelp( pcPatternKey, cStruct, rcMv.getHor(), rcMv.getVer(), 0, 0 );
  
  // test whether one of PRED_A, PRED_B, PRED_C MV is better start point than Median predictor
  if ( bTestOtherPredictedMV )
  {
    for ( UInt index = 0; index < 3; index++ )
    {
      TComMv cMv = m_acMvPredictors[index];
      pcCU->clipMv( cMv );
#ifdef QC_AMVRES
      cMv >>= (2+pcCU->getSlice()->getSPS()->getUseAMVRes());
#else
      cMv >>= 2;
#endif
      xTZSearchHelp( pcPatternKey, cStruct, cMv.getHor(), cMv.getVer(), 0, 0 );
    }
  }
  
  // test whether zero Mv is better start point than Median predictor
  if ( bTestZeroVector )
  {
    xTZSearchHelp( pcPatternKey, cStruct, 0, 0, 0, 0 );
  }
  
  // start search
  Int  iDist = 0;
  Int  iStartX = cStruct.iBestX;
  Int  iStartY = cStruct.iBestY;
  
  // first search
  for ( iDist = 1; iDist <= (Int)uiSearchRange; iDist*=2 )
  {
    if ( bFirstSearchDiamond == 1 )
    {
      xTZ8PointDiamondSearch ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
    }
    else
    {
      xTZ8PointSquareSearch  ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
    }
    
    if ( bFirstSearchStop && ( cStruct.uiBestRound >= uiFirstSearchRounds ) ) // stop criterion
    {
      break;
    }
  }
  
  // test whether zero Mv is a better start point than Median predictor
  if ( bTestZeroVectorStart && ((cStruct.iBestX != 0) || (cStruct.iBestY != 0)) )
  {
    xTZSearchHelp( pcPatternKey, cStruct, 0, 0, 0, 0 );
    if ( (cStruct.iBestX == 0) && (cStruct.iBestY == 0) )
    {
      // test its neighborhood
      for ( iDist = 1; iDist <= (Int)uiSearchRange; iDist*=2 )
      {
        xTZ8PointDiamondSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, 0, 0, iDist );
        if ( bTestZeroVectorStop && (cStruct.uiBestRound > 0) ) // stop criterion
        {
          break;
        }
      }
    }
  }
  
  // calculate only 2 missing points instead 8 points if cStruct.uiBestDistance == 1
  if ( cStruct.uiBestDistance == 1 )
  {
    cStruct.uiBestDistance = 0;
    xTZ2PointSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB );
  }
  
  // raster search if distance is too big
  if ( bEnableRasterSearch && ( ((Int)(cStruct.uiBestDistance) > iRaster) || bAlwaysRasterSearch ) )
  {
    cStruct.uiBestDistance = iRaster;
    for ( iStartY = iSrchRngVerTop; iStartY <= iSrchRngVerBottom; iStartY += iRaster )
    {
      for ( iStartX = iSrchRngHorLeft; iStartX <= iSrchRngHorRight; iStartX += iRaster )
      {
        xTZSearchHelp( pcPatternKey, cStruct, iStartX, iStartY, 0, iRaster );
      }
    }
  }
  
  // raster refinement
  if ( bRasterRefinementEnable && cStruct.uiBestDistance > 0 )
  {
    while ( cStruct.uiBestDistance > 0 )
    {
      iStartX = cStruct.iBestX;
      iStartY = cStruct.iBestY;
      if ( cStruct.uiBestDistance > 1 )
      {
        iDist = cStruct.uiBestDistance >>= 1;
        if ( bRasterRefinementDiamond == 1 )
        {
          xTZ8PointDiamondSearch ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
        else
        {
          xTZ8PointSquareSearch  ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
      }
      
      // calculate only 2 missing points instead 8 points if cStruct.uiBestDistance == 1
      if ( cStruct.uiBestDistance == 1 )
      {
        cStruct.uiBestDistance = 0;
        if ( cStruct.ucPointNr != 0 )
        {
          xTZ2PointSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB );
        }
      }
    }
  }
  
  // start refinement
  if ( bStarRefinementEnable && cStruct.uiBestDistance > 0 )
  {
    while ( cStruct.uiBestDistance > 0 )
    {
      iStartX = cStruct.iBestX;
      iStartY = cStruct.iBestY;
      cStruct.uiBestDistance = 0;
      cStruct.ucPointNr = 0;
      for ( iDist = 1; iDist < (Int)uiSearchRange + 1; iDist*=2 )
      {
        if ( bStarRefinementDiamond == 1 )
        {
          xTZ8PointDiamondSearch ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
        else
        {
          xTZ8PointSquareSearch  ( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB, iStartX, iStartY, iDist );
        }
        if ( bStarRefinementStop && (cStruct.uiBestRound >= uiStarRefinementRounds) ) // stop criterion
        {
          break;
        }
      }
      
      // calculate only 2 missing points instead 8 points if cStrukt.uiBestDistance == 1
      if ( cStruct.uiBestDistance == 1 )
      {
        cStruct.uiBestDistance = 0;
        if ( cStruct.ucPointNr != 0 )
        {
          xTZ2PointSearch( pcPatternKey, cStruct, pcMvSrchRngLT, pcMvSrchRngRB );
        }
      }
    }
  }
  
  // write out best match
  rcMv.set( cStruct.iBestX, cStruct.iBestY );
  ruiSAD = cStruct.uiBestSad - m_pcRdCost->getCost( cStruct.iBestX, cStruct.iBestY );
}


#ifdef ROUNDING_CONTROL_BIPRED
#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracDIF_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost,TComMv *PredMv, Int iRefIdxPred , Pel* piRefY2, Bool bRound )
#else
Void TEncSearch::xPatternSearchFracDIF_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost, Pel* piRefY2, Bool bRound )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
#ifdef QC_AMVRES
  Int iRefStride_HAM	= iRefStride;
#endif
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + m_iDIFHalfTap) << 2);
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf, piRefY2, bRound );
  
  m_pcRdCost->setCostScale( 0 );
  
  //  Quater-pel refinement
  Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
  Int*  piSrc    = m_piYuvExt  + ((m_iYuvExtStride + m_iDIFHalfTap) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
  piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
  xExtDIFUpSamplingQ ( pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor()+rcMvHalf.getVer()*3] );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter, piRefY2, bRound );
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_DIF_Bi( pcPatternKey, piRef, iRefStride_HAM, 4, rcMv_HAM,ruiCost ,PredMv, piRefY2, bRound );
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}

#ifdef QC_SIFO
#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracDIF_QC_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost,TComMv *PredMv, Int iRefIdxPred , Pel* piRefY2, Bool bRound )
#else
Void TEncSearch::xPatternSearchFracDIF_QC_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost , Pel* piRefY2, Bool bRound )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
  Pel* piRef_temp = piRefY; 
  Int iRefStride_temp = iRefStride;
#ifdef QC_AMVRES
  Int iRefStride_HAM	= iRefStride;
#endif
  
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH_QC ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + m_iDIFHalfTap) << 2);
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf, piRefY2, bRound );
  //  Quater-pel refinement on the fly
  m_pcRdCost->setCostScale( 0 ); 
  rcMvQter  = *pcMvInt;  rcMvQter <<= 1;    
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost  = xPatternRefinementMC_Bi( pcPatternKey, piRef_temp, iRefStride_temp, 4, 1, rcMvQter, piRefY2, bRound );
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_DIF_Bi( pcPatternKey, piRef, iRefStride_HAM, 4, rcMv_HAM,ruiCost ,PredMv, piRefY2, bRound  );
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}
UInt TEncSearch::xPatternRefinementMC_Bi  ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac, Pel* piRefY2, Bool bRound  )
{
  UInt  uiDist;
  UInt  uiDistBest  = MAX_UINT;
  UInt  uiDirecBest = 0;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  
  m_pcRdCost->setDistParam_Bi( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);
  
  UInt i = 0;
  Int search_pos=16;
  
  for ( i = 0; i < search_pos; i++)
  {
    TComMv cMvTest = pcMvRefine[i];
    cMvTest += rcMvFrac;
    
    TComMv cMvQter = cMvTest;
    cMvQter <<= (iFrac-1);
    Pel* piOrg   = m_cDistParam.pOrg;
    Int iStrideOrg = m_cDistParam.iStrideOrg;
    
    xPredInterLumaBlk_SIFOApplyME(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvQter, pcPatternKey->getROIYWidth(), 
                                  pcPatternKey->getROIYHeight(), piOrg, iStrideOrg,  0,  0);
    
    uiDist = m_cDistParam.DistFuncRnd( &m_cDistParam,piRefY2, bRound );
    
#if HHI_IMVP
    if ( m_pcEncCfg->getUseIMP() )
    {
#ifdef QC_AMVRES
      TComMv cMvPred;
      if(m_pcEncCfg->getUseAMVRes())
      {
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<iFrac, cMvTest.getVer()<<iFrac);
        cMvPred.scale_down();
      }
      else
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      
      m_pcRdCost->setPredictor( cMvPred );
#else
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      m_pcRdCost->setPredictor( cMvPred );
#endif
    }
#endif
    
    uiDist += m_pcRdCost->getCost( cMvQter.getHor(), cMvQter.getVer() );
    
    if ( uiDist < uiDistBest )
    {
      uiDistBest  = uiDist;
      uiDirecBest = i;
    }
  }
  
  rcMvFrac = pcMvRefine[uiDirecBest];
  return uiDistBest;
}
#endif


#if TEN_DIRECTIONAL_INTERP
Void TEncSearch::xPatternSearchFracDIF_TEN_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost , Pel* piRefY2, Bool bRound  )
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH_TEN ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + 0) << 2);
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf  , piRefY2,  bRound   );
  
  m_pcRdCost->setCostScale( 0 );
  
  //  Quater-pel refinement
  Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
  Int*  piSrc    = m_piYuvExt  + ((m_iYuvExtStride + m_iDIFHalfTap) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
  piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
  xExtDIFUpSamplingQ_TEN ( pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor()+rcMvHalf.getVer()*3] );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter , piRefY2, bRound  );
}
#endif

#if HHI_INTERP_FILTER
#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracMOMS_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost, InterpFilterType ePFilt ,TComMv *PredMv , Int iRefIdxPred , Pel* piRefY2, Bool bRound)
#else
Void TEncSearch::xPatternSearchFracMOMS_Bi( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost, InterpFilterType ePFilt , Pel* piRefY2, Bool bRound)
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  
  Int   iBufStride  = TComPredFilterMOMS::getTmpStride();
  Pel*  piBuf       = TComPredFilterMOMS::getTmpLumaAddr() + ((iBufStride + 1) << 2);
  
  //  Half-pel refinement
  m_pcRdCost->setCostScale( 1 );
  TComPredFilterMOMS::setFiltType( ePFilt );
  TComPredFilterMOMS::extMOMSUpSamplingH ( &cPatternRoi );
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piBuf, iBufStride, 4, 2, rcMvHalf  , piRefY2, bRound );
  
  //  Quarter-pel refinement
  m_pcRdCost->setCostScale( 0 );
  TComPredFilterMOMS::extMOMSUpSamplingQ ( &cPatternRoi, rcMvHalf, piBuf, iBufStride );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  piBuf += (rcMvHalf.getHor() << 1) + iBufStride * (rcMvHalf.getVer() << 1);
  ruiCost = xPatternRefinement_Bi( pcPatternKey, piBuf, iBufStride, 4, 1, rcMvQter , piRefY2, bRound);
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred  !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      Pel* piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_MOMS_Bi( pcPatternKey, piRef, iRefStride, 4, rcMv_HAM,ruiCost,ePFilt ,PredMv ,  piRefY2,  bRound);
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}
#endif
#endif

#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracDIF( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost,TComMv *PredMv, Int iRefIdxPred )
#else
Void TEncSearch::xPatternSearchFracDIF( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
#ifdef QC_AMVRES
  Int iRefStride_HAM	= iRefStride;
#endif
  
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + m_iDIFHalfTap) << 2);
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf   );
  
  m_pcRdCost->setCostScale( 0 );
  
  //  Quater-pel refinement
  Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
  Int*  piSrc    = m_piYuvExt  + ((m_iYuvExtStride + m_iDIFHalfTap) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
  piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
  xExtDIFUpSamplingQ ( pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor()+rcMvHalf.getVer()*3] );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter );
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_DIF( pcPatternKey, piRef, iRefStride_HAM, 4, rcMv_HAM,ruiCost ,PredMv );
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}


#ifdef QC_SIFO
#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracDIF_QC( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost,TComMv *PredMv, Int iRefIdxPred )
#else
Void TEncSearch::xPatternSearchFracDIF_QC( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
  Pel* piRef_temp = piRefY; 
  Int iRefStride_temp = iRefStride;
#ifdef QC_AMVRES
  Int iRefStride_HAM	= iRefStride;
#endif
  
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH_QC ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + m_iDIFHalfTap) << 2);
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf   );
  //  Quater-pel refinement on the fly
  m_pcRdCost->setCostScale( 0 ); 
  rcMvQter  = *pcMvInt;  rcMvQter <<= 1;    
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost  = xPatternRefinementMC( pcPatternKey, piRef_temp, iRefStride_temp, 4, 1, rcMvQter);
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_DIF( pcPatternKey, piRef, iRefStride_HAM, 4, rcMv_HAM,ruiCost ,PredMv );
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}

Void TEncSearch::xExtDIFUpSamplingH_QC ( TComPattern* pcPattern, TComYuv* pcYuvExt)
{
  Int   x, y;
  Int   iWidth      = pcPattern->getROIYWidth();
  Int   iHeight     = pcPattern->getROIYHeight();
  Int   iPatStride  = pcPattern->getPatternLStride();
  Int   iExtStride  = pcYuvExt ->getStride();
  Int*  piSrcY;
  Int*  piDstY;
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  Int List = getCurrList();
  Int RefFrame = getCurrRefFrame();
  Int Offset=0; Offset = getSIFOOffset(List,0,RefFrame);
  
  //  Copy integer-pel
  piSrcYPel = pcPattern->getROIY() - m_iDIFHalfTap - iPatStride;
  piDstY    = m_piYuvExt;//pcYuvExt->getLumaAddr();
  piDstYPel = pcYuvExt->getLumaAddr();
  for ( y = 0; y < iHeight + 2; y++ )
  {
    for ( x = 0; x < iWidth + m_iDIFTap; x++ )
    {
      piDstYPel[x << 2] = Clip(piSrcYPel[x] + Offset);
    }
    piSrcYPel +=  iPatStride;
    piDstY    += (m_iYuvExtStride << 2);
    piDstYPel += (iExtStride      << 2);
  }
  
  Int i,filterH,filterV,filterC;
  i=2;	filterH = getTabFilters(i,getSIFOFilter(i),0);
  i=8;	filterV = getTabFilters(i,getSIFOFilter(i),0);
  i=10;	filterC = getSIFOFilter(i);
  
  Int OffsetH=0;  OffsetH = getSIFOOffset(List, 2,RefFrame);
  Int OffsetV=0;  OffsetV = getSIFOOffset(List, 8,RefFrame);
  Int OffsetC=0;  OffsetC = getSIFOOffset(List,10,RefFrame);
  
  //vertical
  piSrcYPel = pcPattern->getROIY()    - iPatStride - m_iDIFHalfTap;
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1);
  xCTI_FilterHalfVer     (piSrcYPel, iPatStride,     1, iWidth + m_iDIFTap, iHeight + 1, iExtStride<<2, 4, piDstYPel, filterV, OffsetV);
  
  //horizontal
  piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1;
  piDstYPel = pcYuvExt->getLumaAddr() + m_iDIFTap2 - 2;
  xCTI_FilterHalfHor (piSrcYPel, iPatStride,     1,  iWidth + 1, iHeight + 1,  iExtStride<<2, 4, piDstYPel, filterH, OffsetH);
  
  //center
#if USE_DIAGONAL_FILT==1
#if SIFO_DIF_COMPATIBILITY==1
  UInt DIF_filter_position = getNum_SIFOFilters() - getNum_AvailableFilters();
  if(filterC>=DIF_filter_position && m_iDIFTap==6)
  {
    piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1;
    piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1) - 2;
    xCTI_FilterDIF_TEN (piSrcYPel, iPatStride, 1,  iWidth + 1, iHeight + 1, iExtStride<<2, 4, piDstYPel, 2, 2, filterC-DIF_filter_position);
    if(OffsetC)
    {
      for (Int y = 0; y < iHeight + 1; y++)
      {
        for ( Int x = 0; x < iWidth + 1; x++)
          piDstYPel[x*4] = Clip( piDstYPel[x*4] + OffsetC);
        piDstYPel += (iExtStride<<2);
      }
    }
    return;
  }
#else
  if(filterC==5 && m_iDIFTap==6)
  {
    piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1;
    piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1) - 2;
    xCTI_FilterDIF_TEN (piSrcYPel, iPatStride, 1,  iWidth + 1, iHeight + 1, iExtStride<<2, 4, piDstYPel, 2, 2);
    if(OffsetC)
    {
      for (Int y = 0; y < iHeight + 1; y++)
      {
        for ( Int x = 0; x < iWidth + 1; x++)
          piDstYPel[x*4] = Clip( piDstYPel[x*4] + OffsetC);
        piDstYPel += (iExtStride<<2);
      }
    }
    return;
  }
#endif
#endif
  i=10;
  Int filterC_0 = getTabFilters(i,getSIFOFilter(i),0);
  Int filterC_1 = getTabFilters(i,getSIFOFilter(i),1);
  piSrcYPel = pcPattern->getROIY()    - iPatStride - m_iDIFHalfTap;
  piDstY    = m_piYuvExt              + (m_iYuvExtStride<<1);
  xCTI_FilterHalfVer     (piSrcYPel, iPatStride,     1, iWidth + m_iDIFTap, iHeight + 1, m_iYuvExtStride<<2, 4, piDstY, filterC_0, 0);
  piSrcY    = m_piYuvExt              + (m_iYuvExtStride<<1) + ((m_iDIFHalfTap-1) << 2);
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1)      + m_iDIFTap2 - 2;
  xCTI_FilterHalfHor       (piSrcY, m_iYuvExtStride<<2, 4, iWidth + 1, iHeight + 1,iExtStride<<2, 4, piDstYPel, filterC_1, OffsetC);
  
}

UInt TEncSearch::xPatternRefinementMC  ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac )
{
  UInt  uiDist;
  UInt  uiDistBest  = MAX_UINT;
  UInt  uiDirecBest = 0;
  
  Pel* piLumaExt = m_cYuvExt.getLumaAddr();
  Int iYuvExtStride = m_cYuvExt.getStride();
  
  m_pcRdCost->setDistParam( pcPatternKey, piLumaExt, iYuvExtStride, 1, m_cDistParam, m_pcEncCfg->getUseHADME() );
  
  TComMv* pcMvRefine = (iFrac == 2 ? s_acMvRefineH : s_acMvRefineQ);
  
  UInt i = 0;
  Int search_pos=16;
  
  for ( i = 0; i < search_pos; i++)
  {
    TComMv cMvTest = pcMvRefine[i];
    cMvTest += rcMvFrac;
    
    TComMv cMvQter = cMvTest;
    cMvQter <<= (iFrac-1);
    Pel* piOrg   = m_cDistParam.pOrg;
    Int iStrideOrg = m_cDistParam.iStrideOrg;
    
    xPredInterLumaBlk_SIFOApplyME(piRef, iRefStride, piLumaExt, iYuvExtStride, &cMvQter, pcPatternKey->getROIYWidth(), 
                                  pcPatternKey->getROIYHeight(), piOrg, iStrideOrg,  0,  0);
    
    uiDist = m_cDistParam.DistFunc( &m_cDistParam );
    
#if HHI_IMVP
    if ( m_pcEncCfg->getUseIMP() )
    {
#ifdef QC_AMVRES
      TComMv cMvPred;
      if(m_pcEncCfg->getUseAMVRes())
      {
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<iFrac, cMvTest.getVer()<<iFrac);
        cMvPred.scale_down();
      }
      else
        cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      
      m_pcRdCost->setPredictor( cMvPred );
#else
      TComMv cMvPred = m_cMvPredMeasure.getMVPred( cMvTest.getHor()<<(iFrac-1), cMvTest.getVer()<<(iFrac-1) );
      m_pcRdCost->setPredictor( cMvPred );
#endif
    }
#endif
    
    uiDist += m_pcRdCost->getCost( cMvQter.getHor(), cMvQter.getVer() );
    
    if ( uiDist < uiDistBest )
    {
      uiDistBest  = uiDist;
      uiDirecBest = i;
    }
  }
  
  rcMvFrac = pcMvRefine[uiDirecBest];
  return uiDistBest;
}

Void TEncSearch::xAddSubFullPelOffset( TComPattern* pcPatternKey, Int iOffset, Bool Add)
{//SAD = [Org - (Ref+Offset)] = (Org - Offset - Ref);  ........(Org - Offset) is done here...
  if(Add) 
    iOffset = -iOffset; 
  
  Pel *piOrgY = pcPatternKey->getROIY();
  for(Int h = 0; h < pcPatternKey->getROIYHeight(); h++)
  {
    for(Int w = 0; w < pcPatternKey->getROIYWidth(); w++)
    {
      piOrgY[w] = piOrgY[w] + iOffset;      
    }
    piOrgY += pcPatternKey->getPatternLStride();
  }
}

#endif  //QC_SIFO


#if TEN_DIRECTIONAL_INTERP
Void TEncSearch::xPatternSearchFracDIF_TEN( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost )
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  Pel*  piRef;
  iRefStride  = m_cYuvExt.getStride();
  
  //  Half-pel refinement
  xExtDIFUpSamplingH_TEN ( &cPatternRoi, &m_cYuvExt );
  piRef = m_cYuvExt.getLumaAddr() + ((iRefStride + 0) << 2);
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 2, rcMvHalf   );
  
  m_pcRdCost->setCostScale( 0 );
  
  //  Quater-pel refinement
  Pel*  piSrcPel = cPatternRoi.getROIY() + (rcMvHalf.getHor() >> 1) + cPatternRoi.getPatternLStride() * (rcMvHalf.getVer() >> 1);
  Int*  piSrc    = m_piYuvExt  + ((m_iYuvExtStride + m_iDIFHalfTap) << 2) + (rcMvHalf.getHor() << 1) + m_iYuvExtStride * (rcMvHalf.getVer() << 1);
  piRef += (rcMvHalf.getHor() << 1) + iRefStride * (rcMvHalf.getVer() << 1);
  xExtDIFUpSamplingQ_TEN ( pcPatternKey, piRef, iRefStride, piSrcPel, cPatternRoi.getPatternLStride(), piSrc, m_iYuvExtStride, m_puiDFilter[rcMvHalf.getHor()+rcMvHalf.getVer()*3] );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  ruiCost = xPatternRefinement( pcPatternKey, piRef, iRefStride, 4, 1, rcMvQter );
}
#endif

#if HHI_INTERP_FILTER
#ifdef QC_AMVRES
Void TEncSearch::xPatternSearchFracMOMS( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost, InterpFilterType ePFilt ,TComMv *PredMv , Int iRefIdxPred )
#else
Void TEncSearch::xPatternSearchFracMOMS( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost, InterpFilterType ePFilt )
#endif
{
  //  Reference pattern initialization (integer scale)
  TComPattern cPatternRoi;
  Int         iOffset    = pcMvInt->getHor() + pcMvInt->getVer() * iRefStride;
  cPatternRoi.initPattern( piRefY +  iOffset,
                          NULL,
                          NULL,
                          pcPatternKey->getROIYWidth(),
                          pcPatternKey->getROIYHeight(),
                          iRefStride,
                          0, 0, 0, 0 );
  
  Int   iBufStride  = TComPredFilterMOMS::getTmpStride();
  Pel*  piBuf       = TComPredFilterMOMS::getTmpLumaAddr() + ((iBufStride + 1) << 2);
  
  //  Half-pel refinement
  m_pcRdCost->setCostScale( 1 );
  TComPredFilterMOMS::setFiltType( ePFilt );
  TComPredFilterMOMS::extMOMSUpSamplingH ( &cPatternRoi );
  
  rcMvHalf = *pcMvInt;   rcMvHalf <<= 1;    // for mv-cost
  ruiCost = xPatternRefinement( pcPatternKey, piBuf, iBufStride, 4, 2, rcMvHalf   );
  
  //  Quarter-pel refinement
  m_pcRdCost->setCostScale( 0 );
  TComPredFilterMOMS::extMOMSUpSamplingQ ( &cPatternRoi, rcMvHalf, piBuf, iBufStride );
  
  rcMvQter = *pcMvInt;   rcMvQter <<= 1;    // for mv-cost
  rcMvQter += rcMvHalf;  rcMvQter <<= 1;
  piBuf += (rcMvHalf.getHor() << 1) + iBufStride * (rcMvHalf.getVer() << 1);
  ruiCost = xPatternRefinement( pcPatternKey, piBuf, iBufStride, 4, 1, rcMvQter );
  
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    if (pcCU->getSlice()->getSymbolMode()==0 && iRefIdxPred  !=0 )
    {
      rcMvQter <<=1;
    }
    else
    {
      Pel* piRef = piRefY; 
      TComMv rcMv_HAM=*pcMvInt; rcMv_HAM<<= 1;  
      rcMv_HAM += rcMvHalf;  rcMv_HAM <<= 1;
      rcMv_HAM += rcMvQter; 
      ruiCost   = xPatternRefinementHAM_MOMS( pcPatternKey, piRef, iRefStride, 4, rcMv_HAM,ruiCost,ePFilt ,PredMv );
      rcMvQter <<=1;
      rcMvQter+=rcMv_HAM;
    }
  }
#endif
}
#endif

Void TEncSearch::predInterSkipSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv )
{
  SliceType eSliceType = pcCU->getSlice()->getSliceType();
  if ( eSliceType == I_SLICE )
    return;
  
  if ( eSliceType == P_SLICE && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
  {
    pcCU->setInterDirSubParts( 1, 0, 0, pcCU->getDepth( 0 ) );
    
    TComMv cMv;
    TComMv cZeroMv;
    xEstimateMvPredAMVP( pcCU, pcOrgYuv, 0, REF_PIC_LIST_0, 0, cMv, (pcCU->getCUMvField( REF_PIC_LIST_0 )->getAMVPInfo()->iN > 0?  true:false) );
#ifdef QC_AMVRES
    if (pcCU->getSlice()->getSPS()->getUseAMVRes())
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField_AMVRes( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
    else
#endif
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
#ifdef QC_AMVRES
    if (pcCU->getSlice()->getSPS()->getUseAMVRes())
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField_AMVRes( cZeroMv, NOT_VALID, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
    else
#endif
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cZeroMv, NOT_VALID, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
    
    pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, 0, 0, pcCU->getDepth(0));
    pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, 0, 0, pcCU->getDepth(0));
    
    //  Motion compensation
#ifdef DCM_PBIC
    motionCompensation ( pcCU, rpcPredYuv );
#else
    motionCompensation ( pcCU, rpcPredYuv, REF_PIC_LIST_0 );
#endif

#ifdef DCM_PBIC 
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      TComIc cIc;

      xEstimateIcPredAICP( pcCU, pcOrgYuv, 0, REF_PIC_LIST_0, cIc, false );

      cIc.computeScaleOffset( REF_PIC_LIST_0 );
      pcCU->getCUIcField()->setAllIcField( cIc, SIZE_2Nx2N, 0, 0, 0 );

      motionCompensation ( pcCU, rpcPredYuv );
    }
#endif 
  }
  else if ( eSliceType == B_SLICE &&
           pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 &&
           pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0  )
  {
    TComMv cMv;
    TComMv cZeroMv;
    
    if (pcCU->getInterDir(0)!=2)
    {
      xEstimateMvPredAMVP( pcCU, pcOrgYuv, 0, REF_PIC_LIST_0, 0, cMv, (pcCU->getCUMvField( REF_PIC_LIST_0 )->getAMVPInfo()->iN > 0?  true:false) );
#ifdef QC_AMVRES
      if (pcCU->getSlice()->getSPS()->getUseAMVRes())
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField_AMVRes( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );  
      else
#endif      
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 ); //unnecessary
    }
    
    if (pcCU->getInterDir(0)!=1)
    {
      xEstimateMvPredAMVP( pcCU, pcOrgYuv, 0, REF_PIC_LIST_1, 0, cMv, (pcCU->getCUMvField( REF_PIC_LIST_1 )->getAMVPInfo()->iN > 0?  true:false) );
#ifdef QC_AMVRES
      if (pcCU->getSlice()->getSPS()->getUseAMVRes())
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField_AMVRes( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
      else
#endif      
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
    }
    
    motionCompensation ( pcCU, rpcPredYuv );

#ifdef DCM_PBIC 
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      TComIc cIc;
      UChar uhInterDir = pcCU->getInterDir(0);
      RefPicList eRefList = (uhInterDir == 3) ? REF_PIC_LIST_X : ( (uhInterDir == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);;

      xEstimateIcPredAICP( pcCU, pcOrgYuv, 0, eRefList, cIc, false );

      cIc.computeScaleOffset( eRefList );
      pcCU->getCUIcField()->setAllIcField( cIc,SIZE_2Nx2N, 0, 0, 0 );
      
      motionCompensation ( pcCU, rpcPredYuv );
    }
#endif
  }
  else
  {
    assert( 0 );
  }
  
  return;
}

#if HHI_MRG
#ifdef DCM_PBIC
Void TEncSearch::predInterMergeSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv,  TComMvField cMvFieldNeighbourToTest[2], TComIc cIcNeighbourToTest, UChar uhInterDirNeighbourToTest )
#else
Void TEncSearch::predInterMergeSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv,  TComMvField cMvFieldNeighbourToTest[2], UChar uhInterDirNeighbourToTest )
#endif
{
  SliceType eSliceType = pcCU->getSlice()->getSliceType();
  if ( eSliceType == I_SLICE )
    return;
  
  UChar uhDepth = pcCU->getDepth( 0 );
  pcCU->setInterDirSubParts( uhInterDirNeighbourToTest, 0, 0, uhDepth );
#ifdef QC_AMVRES
  if (pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField_AMVRes( cMvFieldNeighbourToTest[ 0 ].getMv(), cMvFieldNeighbourToTest[ 0 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField_AMVRes( cMvFieldNeighbourToTest[ 1 ].getMv(), cMvFieldNeighbourToTest[ 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
  }
  else
  {
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbourToTest[ 0 ].getMv(), cMvFieldNeighbourToTest[ 0 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbourToTest[ 1 ].getMv(), cMvFieldNeighbourToTest[ 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
  }
#else
  pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbourToTest[ 0 ].getMv(), cMvFieldNeighbourToTest[ 0 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
  pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbourToTest[ 1 ].getMv(), cMvFieldNeighbourToTest[ 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
#endif

#ifdef DCM_PBIC
  if (pcCU->getSlice()->getSPS()->getUseIC())
  {
    RefPicList eRefList = (uhInterDirNeighbourToTest == 3) ? REF_PIC_LIST_X : ( (uhInterDirNeighbourToTest == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);;
    cIcNeighbourToTest.computeScaleOffset( eRefList ); 
    pcCU->getCUIcField()->setAllIcField( cIcNeighbourToTest, SIZE_2Nx2N, 0, 0, 0 );
  }
#endif

  motionCompensation ( pcCU, rpcPredYuv );
  
  return;
}
#endif

#if HHI_RQT
Void TEncSearch::encodeResAndCalcRdInterCU( TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvResiBest, TComYuv*& rpcYuvRec, Bool bSkipRes )
#else
Void TEncSearch::encodeResAndCalcRdInterCU( TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvRec, Bool bSkipRes )
#endif
{
  if ( pcCU->isIntra(0) )
  {
    return;
  }
  
  PredMode  ePredMode    = pcCU->getPredictionMode( 0 );
  Bool      bHighPass    = pcCU->getSlice()->getDepth() ? true : false;
  UInt      uiBits       = 0, uiBitsBest = 0;
  UInt      uiDistortion = 0, uiDistortionBest = 0;
  
  UInt      uiWidth      = pcCU->getWidth ( 0 );
  UInt      uiHeight     = pcCU->getHeight( 0 );
  
  //  No residual coding : SKIP mode
  if ( ePredMode == MODE_SKIP && bSkipRes )
  {
    rpcYuvResi->clear();
    
    pcYuvPred->copyToPartYuv( rpcYuvRec, 0 );
    
    uiDistortion = m_pcRdCost->getDistPart( rpcYuvRec->getLumaAddr(), rpcYuvRec->getStride(),  pcYuvOrg->getLumaAddr(), pcYuvOrg->getStride(),  uiWidth,      uiHeight      )
    + m_pcRdCost->getDistPart( rpcYuvRec->getCbAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 )
    + m_pcRdCost->getDistPart( rpcYuvRec->getCrAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 );
    
    if( m_bUseSBACRD )
      m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);
    
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSkipFlag(pcCU, 0, true);
    
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, 0, REF_PIC_LIST_0);
    }
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, 0, REF_PIC_LIST_1);
    }
    
#ifdef DCM_PBIC
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      m_pcEntropyCoder->encodeICPIdx( pcCU, 0 );
    }
#endif

    uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
    pcCU->getTotalBits()       = uiBits;
    pcCU->getTotalDistortion() = uiDistortion;
    pcCU->getTotalCost()       = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
    
    if( m_bUseSBACRD )
      m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
    
    pcCU->setCbfSubParts( 0, 0, 0, 0, pcCU->getDepth( 0 ) );
    pcCU->setTrIdxSubParts( 0, 0, pcCU->getDepth(0) );
    
    return;
  }
  
  //  Residual coding.
  UInt    uiQp, uiQpBest = 0, uiQpMin, uiQpMax;
  Double  dCost, dCostBest = MAX_DOUBLE;
  
  UInt uiTrMode, uiBestTrMode = 0;
  
  UInt uiTrLevel = 0;
  if( (pcCU->getWidth(0) > pcCU->getSlice()->getSPS()->getMaxTrSize()) )
  {
    while( pcCU->getWidth(0) > (pcCU->getSlice()->getSPS()->getMaxTrSize()<<uiTrLevel) ) uiTrLevel++;
  }
  UInt uiMinTrMode = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel;
  UInt uiMaxTrMode = pcCU->getSlice()->getSPS()->getMaxTrDepth() + uiTrLevel;
  
  Bool bSpecial = false;
  
  if (pcCU->getPartitionSize(0) >= SIZE_2NxnU && pcCU->getPartitionSize(0) <= SIZE_nRx2N && uiMinTrMode == 0 && uiMaxTrMode == 1)
  {
    uiMaxTrMode++;
    bSpecial = true;
  }
  
  while((uiWidth>>uiMaxTrMode) < (g_uiMaxCUWidth>>g_uiMaxCUDepth)) uiMaxTrMode--;
  
  uiQpMin      = bHighPass ? Min( MAX_QP, Max( MIN_QP, pcCU->getQP(0) - m_iMaxDeltaQP ) ) : pcCU->getQP( 0 );
  uiQpMax      = bHighPass ? Min( MAX_QP, Max( MIN_QP, pcCU->getQP(0) + m_iMaxDeltaQP ) ) : pcCU->getQP( 0 );
  
#if HHI_RQT
    rpcYuvResi->subtract( pcYuvOrg, pcYuvPred, 0, uiWidth );
#endif
  for ( uiQp = uiQpMin; uiQp <= uiQpMax; uiQp++ )
  {
#if HHI_RQT
    pcCU->setQPSubParts( uiQp, 0, pcCU->getDepth(0) );
    dCost = 0.;
    uiBits = 0;
    uiDistortion = 0;
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_CURR_BEST ] );
    }
    
#if HHI_RQT_ROOT
    UInt uiZeroDistortion = 0;
    xEstimateResidualQT( pcCU, 0, 0, rpcYuvResi,  pcCU->getDepth(0), dCost, uiBits, uiDistortion, &uiZeroDistortion );

    double dZeroCost = m_pcRdCost->calcRdCost( 0, uiZeroDistortion );
    if ( dZeroCost < dCost )
    {
      dCost        = dZeroCost;
      uiBits       = 0;
      uiDistortion = uiZeroDistortion;

      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1);
      ::memset( pcCU->getTransformIdx()      , 0, uiQPartNum * sizeof(UChar) );
      ::memset( pcCU->getCbf( TEXT_LUMA )    , 0, uiQPartNum * sizeof(UChar) );
      ::memset( pcCU->getCbf( TEXT_CHROMA_U ), 0, uiQPartNum * sizeof(UChar) );
      ::memset( pcCU->getCbf( TEXT_CHROMA_V ), 0, uiQPartNum * sizeof(UChar) );
      ::memset( pcCU->getCoeffY()            , 0, uiWidth * uiHeight * sizeof( TCoeff )      );
      ::memset( pcCU->getCoeffCb()           , 0, uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
      ::memset( pcCU->getCoeffCr()           , 0, uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
    }
    else
    {
      xSetResidualQTData( pcCU, 0, NULL, pcCU->getDepth(0), false );
    }
#else
#if HHI_RQT_FORCE_SPLIT_ACC2_PU
    xEstimateResidualQT( pcCU, 0, 0, rpcYuvResi,  pcCU->getDepth(0), dCost, uiBits, uiDistortion );
#else
    xEstimateResidualQT( pcCU, 0, rpcYuvResi,  pcCU->getDepth(0), dCost, uiBits, uiDistortion );
#endif
    xSetResidualQTData( pcCU, 0, NULL, pcCU->getDepth(0), false );
#endif

    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
    }
#if 0 // check
    {
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeCoeff( pcCU, 0, pcCU->getDepth(0), pcCU->getWidth(0), pcCU->getHeight(0) );
      const UInt uiBitsForCoeff = m_pcEntropyCoder->getNumberOfWrittenBits();
      if( m_bUseSBACRD )
      {
        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
      }
      if( uiBitsForCoeff != uiBits )
        assert( 0 );
    }
#endif
    uiBits = 0;
    {
      TComYuv *pDummy = NULL;
      xAddSymbolBitsInter( pcCU, 0, 0, uiBits, pDummy, NULL, pDummy );
    }
    
    
    Double dExactCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
    dCost = dExactCost;
    
    if ( dCost < dCostBest )
    {
#if HHI_RQT_ROOT
      if ( !pcCU->getQtRootCbf( 0 ) )
      {
        rpcYuvResiBest->clear();
      }
      else
      {
        xSetResidualQTData( pcCU, 0, rpcYuvResiBest, pcCU->getDepth(0), true );
      }
#else
      xSetResidualQTData( pcCU, 0, rpcYuvResiBest, pcCU->getDepth(0), true );
#endif
      
      if( uiQpMin != uiQpMax && uiQp != uiQpMax )
      {
        const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1);
        ::memcpy( m_puhQTTempTrIdx, pcCU->getTransformIdx(),        uiQPartNum * sizeof(UChar) );
        ::memcpy( m_puhQTTempCbf[0], pcCU->getCbf( TEXT_LUMA ),     uiQPartNum * sizeof(UChar) );
        ::memcpy( m_puhQTTempCbf[1], pcCU->getCbf( TEXT_CHROMA_U ), uiQPartNum * sizeof(UChar) );
        ::memcpy( m_puhQTTempCbf[2], pcCU->getCbf( TEXT_CHROMA_V ), uiQPartNum * sizeof(UChar) );
        ::memcpy( m_pcQTTempCoeffY,  pcCU->getCoeffY(),  uiWidth * uiHeight * sizeof( TCoeff )      );
        ::memcpy( m_pcQTTempCoeffCb, pcCU->getCoeffCb(), uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
        ::memcpy( m_pcQTTempCoeffCr, pcCU->getCoeffCr(), uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
      }
      uiBitsBest       = uiBits;
      uiDistortionBest = uiDistortion;
      dCostBest        = dCost;
      uiQpBest         = uiQp;
      
      if( m_bUseSBACRD )
      {
        m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_TEMP_BEST ] );
      }
    }
#else
    for ( uiTrMode = uiMinTrMode; uiTrMode <= uiMaxTrMode; uiTrMode++ )
    {
      if (bSpecial && uiMaxTrMode == 2 && uiTrMode == 1)
        continue;
      
      pcCU->setTrIdxSubParts( uiTrMode, 0, pcCU->getDepth(0) );
      
      // coefficient clearing is needed in the loop
      ::memset( pcCU->getCoeffY(), 0, sizeof(TCoeff) * uiWidth * uiHeight );
      
      rpcYuvResi->subtract      ( pcYuvOrg, pcYuvPred, 0, uiWidth );
      
      if( m_bUseSBACRD )
        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
      
      xEncodeInterTexture( pcCU, uiQp, bHighPass,  rpcYuvResi, uiTrMode );
      rpcYuvRec->addClip ( pcYuvPred,              rpcYuvResi, 0,      uiWidth  );
      
      uiDistortion = m_pcRdCost->getDistPart( rpcYuvRec->getLumaAddr(), rpcYuvRec->getStride(),  pcYuvOrg->getLumaAddr(), pcYuvOrg->getStride(),  uiWidth,      uiHeight      )
      + m_pcRdCost->getDistPart( rpcYuvRec->getCbAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 )
      + m_pcRdCost->getDistPart( rpcYuvRec->getCrAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 );
      
      // here is code for subtract bcbp if cbp == 0
      
      uiBits = 0;
      
      if( m_bUseSBACRD )
      {
        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
      }
      
      xAddSymbolBitsInter( pcCU, uiQp, uiTrMode, uiBits, rpcYuvRec, pcYuvPred, rpcYuvResi );
      dCost = m_pcRdCost->calcRdCost( uiBits, uiDistortion );
      
      if ( dCost < dCostBest )
      {
        uiBitsBest       = uiBits;
        uiDistortionBest = uiDistortion;
        dCostBest        = dCost;
        uiQpBest         = uiQp;
        uiBestTrMode     = uiTrMode;
        
        if( m_bUseSBACRD )
          m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_TEMP_BEST]);
      }
    }
//#if HHI_RQT
//    }
#endif
  }
  
  assert ( dCostBest != MAX_DOUBLE );
  
#if HHI_RQT
//  if( ! pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
//  {
//#endif
#else
  rpcYuvResi->subtract      ( pcYuvOrg, pcYuvPred, 0, uiWidth );
  
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
  }
    
//#if HHI_RQT
//    pcCU->setTrIdxSubParts( uiBestTrMode, 0, pcCU->getDepth(0) );
//#endif
  xEncodeInterTexture( pcCU, uiQpBest, bHighPass, rpcYuvResi, uiBestTrMode );
  
  if( (pcCU->getCbf(0, TEXT_LUMA, 0) == 0) &&
     (pcCU->getCbf(0, TEXT_CHROMA_U, 0) == 0) &&
     (pcCU->getCbf(0, TEXT_CHROMA_V, 0) == 0) )
//#if HHI_RQT
//    {
//      if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
//      {
//        assert( 0 ); // obsolete
//        const UInt uiLog2CUSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - pcCU->getDepth(0);
//        const UInt uiLog2MaxTUSize = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize();
//        uiBestTrMode = uiLog2MaxTUSize >= uiLog2CUSize ? 0 : uiLog2CUSize - uiLog2MaxTUSize;
//      }
//      else
//      {
//#endif
    uiBestTrMode = 0;
//#if HHI_RQT
//      }
//    }
//  }
// else
#endif
#if HHI_RQT
  if( uiQpMin != uiQpMax && uiQpBest != uiQpMax )
  {
    if( m_bUseSBACRD )
    {
      assert( 0 ); // check
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ pcCU->getDepth( 0 ) ][ CI_TEMP_BEST ] );
    }
    // copy best cbf and trIdx to pcCU
    const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (pcCU->getDepth(0) << 1);
    ::memcpy( pcCU->getTransformIdx(),       m_puhQTTempTrIdx,  uiQPartNum * sizeof(UChar) );
    ::memcpy( pcCU->getCbf( TEXT_LUMA ),     m_puhQTTempCbf[0], uiQPartNum * sizeof(UChar) );
    ::memcpy( pcCU->getCbf( TEXT_CHROMA_U ), m_puhQTTempCbf[1], uiQPartNum * sizeof(UChar) );
    ::memcpy( pcCU->getCbf( TEXT_CHROMA_V ), m_puhQTTempCbf[2], uiQPartNum * sizeof(UChar) );
    ::memcpy( pcCU->getCoeffY(),  m_pcQTTempCoeffY,  uiWidth * uiHeight * sizeof( TCoeff )      );
    ::memcpy( pcCU->getCoeffCb(), m_pcQTTempCoeffCb, uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
    ::memcpy( pcCU->getCoeffCr(), m_pcQTTempCoeffCr, uiWidth * uiHeight * sizeof( TCoeff ) >> 2 );
  }
  rpcYuvRec->addClip ( pcYuvPred, rpcYuvResiBest, 0, uiWidth );
#else
  rpcYuvRec->addClip ( pcYuvPred,     rpcYuvResi, 0, uiWidth  );
#endif
  
#if HHI_RQT
  // update with clipped distortion and cost (qp estimation loop uses unclipped values)
  uiDistortionBest = m_pcRdCost->getDistPart( rpcYuvRec->getLumaAddr(), rpcYuvRec->getStride(),  pcYuvOrg->getLumaAddr(), pcYuvOrg->getStride(),  uiWidth,      uiHeight      )
  + m_pcRdCost->getDistPart( rpcYuvRec->getCbAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCbAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 )
  + m_pcRdCost->getDistPart( rpcYuvRec->getCrAddr(),   rpcYuvRec->getCStride(), pcYuvOrg->getCrAddr(),   pcYuvOrg->getCStride(), uiWidth >> 1, uiHeight >> 1 );
  dCostBest = m_pcRdCost->calcRdCost( uiBitsBest, uiDistortionBest );
#endif
  
  pcCU->getTotalBits()       = uiBitsBest;
  pcCU->getTotalDistortion() = uiDistortionBest;
  pcCU->getTotalCost()       = dCostBest;
  
  if ( pcCU->isSkipped(0) )
  {
    uiBestTrMode = 0;
    pcCU->setCbfSubParts( 0, 0, 0, 0, pcCU->getDepth( 0 ) );
  }
  
#if HHI_RQT
#else
    pcCU->setTrIdxSubParts( uiBestTrMode, 0, pcCU->getDepth(0) );
#endif
  pcCU->setQPSubParts( uiQpBest, 0, pcCU->getDepth(0) );
}

#if HHI_RQT
#if HHI_RQT_ROOT
      Void TEncSearch::xEstimateResidualQT( TComDataCU* pcCU, UInt uiQuadrant, UInt uiAbsPartIdx, TComYuv* pcResi, const UInt uiDepth, Double &rdCost, UInt &ruiBits, UInt &ruiDist, UInt *puiZeroDist )
#elif HHI_RQT_FORCE_SPLIT_ACC2_PU
      Void TEncSearch::xEstimateResidualQT( TComDataCU* pcCU, UInt uiQuadrant, UInt uiAbsPartIdx, TComYuv* pcResi, const UInt uiDepth, Double &rdCost, UInt &ruiBits, UInt &ruiDist )
#else
      Void TEncSearch::xEstimateResidualQT( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcResi, const UInt uiDepth, Double &rdCost, UInt &ruiBits, UInt &ruiDist )
#endif
{
  const UInt uiTrMode = uiDepth - pcCU->getDepth( 0 );
  
  assert( pcCU->getDepth( 0 ) == pcCU->getDepth( uiAbsPartIdx ) );
  const UInt uiLog2TrSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth]+2;

#if HHI_RQT_FORCE_SPLIT_ACC2_PU
#if HHI_RQT_FORCE_SPLIT_NxN
  const Bool bNxNOK = pcCU->getPartitionSize( 0 ) == SIZE_NxN && uiTrMode > 0;
#else
  const Bool bNxNOK = pcCU->getPartitionSize( 0 ) == SIZE_NxN;
#endif
#if HHI_RQT_FORCE_SPLIT_RECT
  const Bool bSymmetricOK  = pcCU->getPartitionSize( 0 ) >= SIZE_2NxN  && pcCU->getPartitionSize( 0 ) < SIZE_NxN   && uiTrMode > 0;
#else
  const Bool bSymmetricOK  = pcCU->getPartitionSize( 0 ) >= SIZE_2NxN  && pcCU->getPartitionSize( 0 ) < SIZE_NxN;
#endif
#if HHI_RQT_FORCE_SPLIT_ASYM
  const Bool bAsymmetricOK = pcCU->getPartitionSize( 0 ) >= SIZE_2NxnU && pcCU->getPartitionSize( 0 ) <= SIZE_nRx2N && uiTrMode > 1;
  const Bool b2NxnUOK      = pcCU->getPartitionSize( 0 ) == SIZE_2NxnU && uiQuadrant > 1      && uiTrMode == 1;
  const Bool b2NxnDOK      = pcCU->getPartitionSize( 0 ) == SIZE_2NxnD && uiQuadrant < 2      && uiTrMode == 1;
  const Bool bnLx2NOK      = pcCU->getPartitionSize( 0 ) == SIZE_nLx2N &&  ( uiQuadrant & 1 ) && uiTrMode == 1;
  const Bool bnRx2NOK      = pcCU->getPartitionSize( 0 ) == SIZE_nRx2N && !( uiQuadrant & 1 ) && uiTrMode == 1;
  const Bool bNoForceSplit = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N || bSymmetricOK || bAsymmetricOK ||
                                                            b2NxnUOK || b2NxnDOK || bnLx2NOK || bnRx2NOK || bNxNOK;
#else
  const Bool bAsymmetricOK = pcCU->getPartitionSize( 0 ) >= SIZE_2NxnU && pcCU->getPartitionSize( 0 ) <= SIZE_nRx2N;
  const Bool bNoForceSplit = pcCU->getPartitionSize( 0 ) == SIZE_2Nx2N || bNxNOK || bSymmetricOK || bAsymmetricOK;
#endif
  const Bool bCheckFull    = bNoForceSplit && ( uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() );
#else
  const Bool bCheckFull    = ( uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() );
#endif

#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  const Bool bCheckSplit  = ( uiLog2TrSize >  pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#else
  const Bool bCheckSplit   = ( uiLog2TrSize >  pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
#endif
  
  assert( bCheckFull || bCheckSplit );
  
  Bool  bCodeChroma   = true;
  UInt  uiTrModeC     = uiTrMode;
  UInt  uiLog2TrSizeC = uiLog2TrSize-1;
  if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiLog2TrSizeC++;
    uiTrModeC    --;
    UInt  uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrModeC ) << 1 );
    bCodeChroma   = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
  }
  
  const UInt uiSetCbf = 1 << uiTrMode;
  // code full block
  Double dSingleCost = MAX_DOUBLE;
  UInt uiSingleBits = 0;
  UInt uiSingleDist = 0;
  UInt uiAbsSumY = 0, uiAbsSumU = 0, uiAbsSumV = 0;
  
  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_ROOT ] );
  }
  
  if( bCheckFull )
  {
    assert( pcCU->getROTindex( 0 ) == pcCU->getROTindex( uiAbsPartIdx ) );
    const UInt uiNumCoeffPerAbsPartIdxIncrement = pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
    const UInt uiQTTempAccessLayer = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    TCoeff *pcCoeffCurrY = m_ppcQTTempCoeffY [uiQTTempAccessLayer] +  uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx;
    TCoeff *pcCoeffCurrU = m_ppcQTTempCoeffCb[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
    TCoeff *pcCoeffCurrV = m_ppcQTTempCoeffCr[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
    
    pcCU->setTrIdxSubParts( uiDepth - pcCU->getDepth( 0 ), uiAbsPartIdx, uiDepth );
    if (m_pcEncCfg->getUseRDOQ())
    {
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, 1<< uiLog2TrSize, TEXT_LUMA );
    }
    m_pcTrQuant->setQPforQuant( pcCU->getQP( 0 ), false, pcCU->getSlice()->getSliceType(), TEXT_LUMA );
#if DISABLE_ROT_LUMA_4x4_8x8
    m_pcTrQuant->transformNxN( pcCU, pcResi->getLumaAddr( uiAbsPartIdx ), pcResi->getStride (), pcCoeffCurrY, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    uiAbsSumY, TEXT_LUMA,     uiAbsPartIdx, (1<< uiLog2TrSize) > 8 ? pcCU->getROTindex( 0 ) : 0 );
#else
    m_pcTrQuant->transformNxN( pcCU, pcResi->getLumaAddr( uiAbsPartIdx ), pcResi->getStride (), pcCoeffCurrY, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    uiAbsSumY, TEXT_LUMA,     uiAbsPartIdx, pcCU->getROTindex( 0 ) );
#endif
    
    pcCU->setCbfSubParts( uiAbsSumY ? uiSetCbf : 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    
    if( bCodeChroma )
    {
      if (m_pcEncCfg->getUseRDOQ())
      {
        m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, 1<<uiLog2TrSizeC, TEXT_CHROMA );
      }
      m_pcTrQuant->setQPforQuant( pcCU->getQP( 0 ), false, pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
      m_pcTrQuant->transformNxN( pcCU, pcResi->getCbAddr( uiAbsPartIdx ), pcResi->getCStride(), pcCoeffCurrU, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiAbsSumU, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getROTindex( 0 ) );
      m_pcTrQuant->transformNxN( pcCU, pcResi->getCrAddr( uiAbsPartIdx ), pcResi->getCStride(), pcCoeffCurrV, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiAbsSumV, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getROTindex( 0 ) );
      pcCU->setCbfSubParts( uiAbsSumU ? uiSetCbf : 0, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
      pcCU->setCbfSubParts( uiAbsSumV ? uiSetCbf : 0, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
    }
    
    m_pcEntropyCoder->resetBits();
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode()==0 && uiTrMode == 0)
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0 );
#endif

#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode())
#endif
    m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA,     uiTrMode );

    m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrY, uiAbsPartIdx, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    uiDepth, TEXT_LUMA,     false );
    const UInt uiSingleBitsY = m_pcEntropyCoder->getNumberOfWrittenBits();
    
    UInt uiSingleBitsU = 0;
    UInt uiSingleBitsV = 0;
    if( bCodeChroma )
    {
#if LCEC_CBP_YUV_ROOT
      if (pcCU->getSlice()->getSymbolMode())
#endif
      m_pcEntropyCoder->encodeQtCbf   ( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrMode );
      m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrU, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_U, false );
      uiSingleBitsU = m_pcEntropyCoder->getNumberOfWrittenBits() - uiSingleBitsY;
      
#if LCEC_CBP_YUV_ROOT
      if (pcCU->getSlice()->getSymbolMode())
#endif
      m_pcEntropyCoder->encodeQtCbf   ( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrMode );
      m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrV, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_V, false );
      uiSingleBitsV = m_pcEntropyCoder->getNumberOfWrittenBits() - ( uiSingleBitsY + uiSingleBitsU );
    }
    
    const UInt uiNumSamplesLuma = 1 << (uiLog2TrSize<<1);
    const UInt uiNumSamplesChro = 1 << (uiLog2TrSizeC<<1);
    
    ::memset( m_pTempPel, 0, sizeof( Pel ) * uiNumSamplesLuma ); // not necessary needed for inside of recursion (only at the beginning)
    
    UInt uiDistY = m_pcRdCost->getDistPart( m_pTempPel, 1<< uiLog2TrSize, pcResi->getLumaAddr( uiAbsPartIdx ), pcResi->getStride(), 1<< uiLog2TrSize, 1<< uiLog2TrSize ); // initialized with zero residual destortion
#if HHI_RQT_ROOT
    if ( puiZeroDist )
    {
      *puiZeroDist += uiDistY;
    }
#endif
    if( uiAbsSumY )
    {
      Pel *pcResiCurrY = m_pcQTTempTComYuv[uiQTTempAccessLayer].getLumaAddr( uiAbsPartIdx );
      m_pcTrQuant->setQPforQuant( pcCU->getQP( 0 ), false, pcCU->getSlice()->getSliceType(), TEXT_LUMA );
#if QC_MDDT
      m_pcTrQuant->invtransformNxN( TEXT_LUMA, REG_DCT, pcResiCurrY, m_pcQTTempTComYuv[uiQTTempAccessLayer].getStride(),  pcCoeffCurrY, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    pcCU->getROTindex( 0 ) );//this is for inter mode only
#else
#if DISABLE_ROT_LUMA_4x4_8x8
      m_pcTrQuant->invtransformNxN( pcResiCurrY, m_pcQTTempTComYuv[uiQTTempAccessLayer].getStride(),  pcCoeffCurrY, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    (1<< uiLog2TrSize) > 8 ? pcCU->getROTindex( 0 ) : 0 );
#else
      m_pcTrQuant->invtransformNxN( pcResiCurrY, m_pcQTTempTComYuv[uiQTTempAccessLayer].getStride(),  pcCoeffCurrY, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    pcCU->getROTindex( 0 ) );
#endif
#endif
      const UInt uiNonzeroDistY = m_pcRdCost->getDistPart( m_pcQTTempTComYuv[uiQTTempAccessLayer].getLumaAddr( uiAbsPartIdx ), m_pcQTTempTComYuv[uiQTTempAccessLayer].getStride(),
                                                          pcResi->getLumaAddr( uiAbsPartIdx ), pcResi->getStride(), 1<< uiLog2TrSize,    1<< uiLog2TrSize );
      const Double dSingleCostY = m_pcRdCost->calcRdCost( uiSingleBitsY, uiNonzeroDistY );
      const Double dNullCostY   = m_pcRdCost->calcRdCost( 0, uiDistY );
      if( dNullCostY < dSingleCostY )
      {
        uiAbsSumY = 0;
        ::memset( pcCoeffCurrY, 0, sizeof( TCoeff ) * uiNumSamplesLuma );
      }
      else
      {
        uiDistY = uiNonzeroDistY;
      }
    }
    
    if( !uiAbsSumY )
    {
      Pel *pcPtr =  m_pcQTTempTComYuv[uiQTTempAccessLayer].getLumaAddr( uiAbsPartIdx );
      const UInt uiStride = m_pcQTTempTComYuv[uiQTTempAccessLayer].getStride();
      for( UInt uiY = 0; uiY < 1<< uiLog2TrSize; ++uiY )
      {
        ::memset( pcPtr, 0, sizeof(Pel) << uiLog2TrSize );
        pcPtr += uiStride;
      }
    }
    
    UInt uiDistU = 0;
    UInt uiDistV = 0;
    if( bCodeChroma )
    {
      uiDistU = m_pcRdCost->getDistPart( m_pTempPel, 1<<uiLog2TrSizeC, pcResi->getCbAddr( uiAbsPartIdx ), pcResi->getCStride(), 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC ); // initialized with zero residual destortion
#if HHI_RQT_ROOT
      if ( puiZeroDist )
      {
        *puiZeroDist += uiDistU;
      }
#endif
      if( uiAbsSumU )
      {
        Pel *pcResiCurrU = m_pcQTTempTComYuv[uiQTTempAccessLayer].getCbAddr( uiAbsPartIdx );
        m_pcTrQuant->setQPforQuant( pcCU->getQP( 0 ), false, pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
#if QC_MDDT
        m_pcTrQuant->invtransformNxN( TEXT_CHROMA, REG_DCT, pcResiCurrU, m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(), pcCoeffCurrU, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, pcCU->getROTindex( 0 ) );
#else
        m_pcTrQuant->invtransformNxN( pcResiCurrU, m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(), pcCoeffCurrU, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, pcCU->getROTindex( 0 ) );
#endif
        const UInt uiNonzeroDistU = m_pcRdCost->getDistPart( m_pcQTTempTComYuv[uiQTTempAccessLayer].getCbAddr( uiAbsPartIdx ), m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(),
                                                            pcResi->getCbAddr( uiAbsPartIdx ), pcResi->getCStride(), 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC );
        const Double dSingleCostU = m_pcRdCost->calcRdCost( uiSingleBitsU, uiNonzeroDistU );
        const Double dNullCostU   = m_pcRdCost->calcRdCost( 0, uiDistU );
        if( dNullCostU < dSingleCostU )
        {
          uiAbsSumU = 0;
          ::memset( pcCoeffCurrU, 0, sizeof( TCoeff ) * uiNumSamplesChro );
        }
        else
        {
          uiDistU = uiNonzeroDistU;
        }
      }
      if( !uiAbsSumU )
      {
        Pel *pcPtr =  m_pcQTTempTComYuv[uiQTTempAccessLayer].getCbAddr( uiAbsPartIdx );
        const UInt uiStride = m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride();
        for( UInt uiY = 0; uiY < 1<<uiLog2TrSizeC; ++uiY )
        {
          ::memset( pcPtr, 0, sizeof(Pel) << uiLog2TrSizeC );
          pcPtr += uiStride;
        }
      }
      
      uiDistV = m_pcRdCost->getDistPart( m_pTempPel, 1<<uiLog2TrSizeC, pcResi->getCrAddr( uiAbsPartIdx ), pcResi->getCStride(), 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC ); // initialized with zero residual destortion
#if HHI_RQT_ROOT
      if ( puiZeroDist )
      {
        *puiZeroDist += uiDistV;
      }
#endif
      if( uiAbsSumV )
      {
        Pel *pcResiCurrV = m_pcQTTempTComYuv[uiQTTempAccessLayer].getCrAddr  ( uiAbsPartIdx );
        if( !uiAbsSumU )
        {
          m_pcTrQuant->setQPforQuant( pcCU->getQP( 0 ), false, pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
        }
#if QC_MDDT
        m_pcTrQuant->invtransformNxN( TEXT_CHROMA, REG_DCT, pcResiCurrV, m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(), pcCoeffCurrV, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, pcCU->getROTindex( 0 ) );
#else
        m_pcTrQuant->invtransformNxN( pcResiCurrV, m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(), pcCoeffCurrV, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, pcCU->getROTindex( 0 ) );
#endif
        const UInt uiNonzeroDistV = m_pcRdCost->getDistPart( m_pcQTTempTComYuv[uiQTTempAccessLayer].getCrAddr( uiAbsPartIdx ), m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride(),
                                                            pcResi->getCrAddr( uiAbsPartIdx ), pcResi->getCStride(), 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC );
        const Double dSingleCostV = m_pcRdCost->calcRdCost( uiSingleBitsV, uiNonzeroDistV );
        const Double dNullCostV   = m_pcRdCost->calcRdCost( 0, uiDistV );
        if( dNullCostV < dSingleCostV )
        {
          uiAbsSumV = 0;
          ::memset( pcCoeffCurrV, 0, sizeof( TCoeff ) * uiNumSamplesChro );
        }
        else
        {
          uiDistV = uiNonzeroDistV;
        }
      }
      if( !uiAbsSumV )
      {
        Pel *pcPtr =  m_pcQTTempTComYuv[uiQTTempAccessLayer].getCrAddr( uiAbsPartIdx );
        const UInt uiStride = m_pcQTTempTComYuv[uiQTTempAccessLayer].getCStride();
        for( UInt uiY = 0; uiY < 1<<uiLog2TrSizeC; ++uiY )
        {
          ::memset( pcPtr, 0, sizeof(Pel) << uiLog2TrSizeC );
          pcPtr += uiStride;
        }
      }
    }
    pcCU->setCbfSubParts( uiAbsSumY ? uiSetCbf : 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    if( bCodeChroma )
    {
      pcCU->setCbfSubParts( uiAbsSumU ? uiSetCbf : 0, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
      pcCU->setCbfSubParts( uiAbsSumV ? uiSetCbf : 0, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
    }
    
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_ROOT ] );
    }
    
    m_pcEntropyCoder->resetBits();
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
    if( uiLog2TrSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
#else	
    if( uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
#endif
    {
      m_pcEntropyCoder->encodeTransformSubdivFlag( 0, uiDepth );
    }
    
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode())
    {
#endif
#if HHI_RQT_CHROMA_CBF_MOD
    if( bCodeChroma )
    {
      m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrMode );
      m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrMode );
    }
#endif
    
    m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA,     uiTrMode );
    
#if HHI_RQT_CHROMA_CBF_MOD
    if( 0 )
#endif
      if( bCodeChroma )
      {
        m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrMode );
        m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrMode );
      }
#if LCEC_CBP_YUV_ROOT
    }
#endif

#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode()==0 && uiTrMode == 0)
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0 );
#endif

    m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrY, uiAbsPartIdx, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    uiDepth, TEXT_LUMA,     false );
    
    if( bCodeChroma )
    {
      m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrU, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_U, false );
      m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrV, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_V, false );
    }
    
    uiSingleBits = m_pcEntropyCoder->getNumberOfWrittenBits();
    
    uiSingleDist = uiDistY + uiDistU + uiDistV;
    dSingleCost = m_pcRdCost->calcRdCost( uiSingleBits, uiSingleDist );
  }  
 
  // code sub-blocks
  if( bCheckSplit )
  {
    if( m_bUseSBACRD && bCheckFull )
    {
      m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_TEST ] );
      m_pcRDGoOnSbacCoder->load ( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_ROOT ] );
    }
    UInt uiSubdivDist = 0;
    UInt uiSubdivBits = 0;
    Double dSubdivCost = 0.0;
    
    const UInt uiQPartNumSubdiv = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1 ) << 1);
    for( UInt ui = 0; ui < 4; ++ui )
    {
#if HHI_RQT_ROOT
      xEstimateResidualQT( pcCU, ui, uiAbsPartIdx + ui * uiQPartNumSubdiv, pcResi, uiDepth + 1, dSubdivCost, uiSubdivBits, uiSubdivDist, bCheckFull ? NULL : puiZeroDist );
#elif HHI_RQT_FORCE_SPLIT_ACC2_PU
      xEstimateResidualQT( pcCU, ui, uiAbsPartIdx + ui * uiQPartNumSubdiv, pcResi, uiDepth + 1, dSubdivCost, uiSubdivBits, uiSubdivDist );
#else
      xEstimateResidualQT( pcCU, uiAbsPartIdx + ui * uiQPartNumSubdiv, pcResi, uiDepth + 1, dSubdivCost, uiSubdivBits, uiSubdivDist );
#endif
    }

    UInt uiYCbf = 0;
    UInt uiUCbf = 0;
    UInt uiVCbf = 0;
    for( UInt ui = 0; ui < 4; ++ui )
    {
      uiYCbf |= pcCU->getCbf( uiAbsPartIdx + ui * uiQPartNumSubdiv, TEXT_LUMA,     uiTrMode + 1 );
      uiUCbf |= pcCU->getCbf( uiAbsPartIdx + ui * uiQPartNumSubdiv, TEXT_CHROMA_U, uiTrMode + 1 );
      uiVCbf |= pcCU->getCbf( uiAbsPartIdx + ui * uiQPartNumSubdiv, TEXT_CHROMA_V, uiTrMode + 1 );
    }
    for( UInt ui = 0; ui < 4 * uiQPartNumSubdiv; ++ui )
    {
      pcCU->getCbf( TEXT_LUMA     )[uiAbsPartIdx + ui] |= uiYCbf << uiTrMode;
      pcCU->getCbf( TEXT_CHROMA_U )[uiAbsPartIdx + ui] |= uiUCbf << uiTrMode;
      pcCU->getCbf( TEXT_CHROMA_V )[uiAbsPartIdx + ui] |= uiVCbf << uiTrMode;
    }
    
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_ROOT ] );
    }
    m_pcEntropyCoder->resetBits();
    
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode()==0 && uiTrMode == 0)
    {
      m_pcEntropyCoder->m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0 );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)
         || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0))
      {
        xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, true,  TEXT_LUMA );
        xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_LUMA );
        xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_U );
        xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_V );
      }
    }
    else
    {
      xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, true,  TEXT_LUMA );
      xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_LUMA );
      xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_U );
      xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_V );
    }
#else
    xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, true,  TEXT_LUMA );
    xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_LUMA );
    xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_U );
    xEncodeResidualQT( pcCU, uiAbsPartIdx, uiDepth, false, TEXT_CHROMA_V );
#endif
    
    uiSubdivBits = m_pcEntropyCoder->getNumberOfWrittenBits();
    dSubdivCost  = m_pcRdCost->calcRdCost( uiSubdivBits, uiSubdivDist );
    
#if MS_LAST_CBF
    if( uiYCbf || uiUCbf || uiVCbf || !bCheckFull )
#endif
    if( dSubdivCost < dSingleCost )
    {
      rdCost += dSubdivCost;
      ruiBits += uiSubdivBits;
      ruiDist += uiSubdivDist;
      return;
    }
    assert( bCheckFull );
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[ uiDepth ][ CI_QT_TRAFO_TEST ] );
    }
  }
  rdCost += dSingleCost;
  ruiBits += uiSingleBits;
  ruiDist += uiSingleDist;
  
  pcCU->setTrIdxSubParts( uiTrMode, uiAbsPartIdx, uiDepth );
  
  pcCU->setCbfSubParts( uiAbsSumY ? uiSetCbf : 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
  if( bCodeChroma )
  {
    pcCU->setCbfSubParts( uiAbsSumU ? uiSetCbf : 0, TEXT_CHROMA_U, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
    pcCU->setCbfSubParts( uiAbsSumV ? uiSetCbf : 0, TEXT_CHROMA_V, uiAbsPartIdx, pcCU->getDepth(0)+uiTrModeC );
  }
}

Void TEncSearch::xEncodeResidualQT( TComDataCU* pcCU, UInt uiAbsPartIdx, const UInt uiDepth, Bool bSubdivAndCbf, TextType eType )
{
  assert( pcCU->getDepth( 0 ) == pcCU->getDepth( uiAbsPartIdx ) );
  const UInt uiCurrTrMode = uiDepth - pcCU->getDepth( 0 );
  const UInt uiTrMode = pcCU->getTransformIdx( uiAbsPartIdx );
  
  const Bool bSubdiv = uiCurrTrMode != uiTrMode;
  
  const UInt uiLog2TrSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth]+2;
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  if( bSubdivAndCbf && uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && uiLog2TrSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
#else
  if( bSubdivAndCbf && uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
#endif
  {
    m_pcEntropyCoder->encodeTransformSubdivFlag( bSubdiv, uiDepth );
  }
  
#if LCEC_CBP_YUV_ROOT
  if (pcCU->getSlice()->getSymbolMode())
  {
#endif
#if HHI_RQT_CHROMA_CBF_MOD
  assert( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA );
  if( bSubdivAndCbf && uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    const Bool bFirstCbfOfCU = uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiCurrTrMode == 0;
    if( bFirstCbfOfCU || uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiCurrTrMode - 1 ) )
      {
        m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiCurrTrMode );
      }
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiCurrTrMode - 1 ) )
      {
        m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiCurrTrMode );
      }
    }
    else if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiCurrTrMode ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiCurrTrMode - 1 ) );
      assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiCurrTrMode ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiCurrTrMode - 1 ) );
    }
  }
#endif
#if LCEC_CBP_YUV_ROOT
  }
#endif
  
  if( !bSubdiv )
  {
    const UInt uiNumCoeffPerAbsPartIdxIncrement = pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
    assert( 16 == uiNumCoeffPerAbsPartIdxIncrement ); // check
    const UInt uiQTTempAccessLayer = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    TCoeff *pcCoeffCurrY = m_ppcQTTempCoeffY [uiQTTempAccessLayer] +  uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx;
    TCoeff *pcCoeffCurrU = m_ppcQTTempCoeffCb[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
    TCoeff *pcCoeffCurrV = m_ppcQTTempCoeffCr[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
    
    Bool  bCodeChroma   = true;
    UInt  uiTrModeC     = uiTrMode;
    UInt  uiLog2TrSizeC = uiLog2TrSize-1;
    if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      uiLog2TrSizeC++;
      uiTrModeC    --;
      UInt  uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrModeC ) << 1 );
      bCodeChroma   = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    }
    
    if( bSubdivAndCbf )
    {
#if LCEC_CBP_YUV_ROOT
      if (pcCU->getSlice()->getSymbolMode())
      {
#endif
      m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA,     uiTrMode );
      
#if HHI_RQT_CHROMA_CBF_MOD
      if( 0 ) // do only if intra -> never
#endif
        if( bCodeChroma )
        {
          m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrMode );
          m_pcEntropyCoder->encodeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrMode );
        }
#if LCEC_CBP_YUV_ROOT
      }
#endif
    }
    else
    {
      if( eType == TEXT_LUMA     && pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA,     uiTrMode ) )
      {
        m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrY, uiAbsPartIdx, 1<< uiLog2TrSize,    1<< uiLog2TrSize,    uiDepth, TEXT_LUMA,     false );
      }
      if( bCodeChroma )
      {
        if( eType == TEXT_CHROMA_U && pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrMode ) )
        {
          m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrU, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_U, false );
        }
        if( eType == TEXT_CHROMA_V && pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrMode ) )
        {
          m_pcEntropyCoder->encodeCoeffNxN( pcCU, pcCoeffCurrV, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC, uiDepth, TEXT_CHROMA_V, false );
        }
      }
    }
  }
  else
  {
    if( bSubdivAndCbf || pcCU->getCbf( uiAbsPartIdx, eType, uiCurrTrMode ) )
    {
      const UInt uiQPartNumSubdiv = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1 ) << 1);
#if LCEC_CBP_YUV_ROOT
      if(pcCU->getSlice()->getSymbolMode() == 0)
      {
        if( eType == TEXT_LUMA || uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
          m_pcEntropyCoder->m_pcEntropyCoderIf->codeBlockCbf(pcCU, uiAbsPartIdx, eType, uiCurrTrMode + 1, uiQPartNumSubdiv, true);
      }
#endif
      for( UInt ui = 0; ui < 4; ++ui )
      {
        xEncodeResidualQT( pcCU, uiAbsPartIdx + ui * uiQPartNumSubdiv, uiDepth + 1, bSubdivAndCbf, eType );
      }
    }
  }
}

Void TEncSearch::xSetResidualQTData( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcResi, UInt uiDepth, Bool bSpatial )
{
  assert( pcCU->getDepth( 0 ) == pcCU->getDepth( uiAbsPartIdx ) );
  const UInt uiCurrTrMode = uiDepth - pcCU->getDepth( 0 );
  const UInt uiTrMode = pcCU->getTransformIdx( uiAbsPartIdx );
  
  if( uiCurrTrMode == uiTrMode )
  {
    const UInt uiLog2TrSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth]+2;
    const UInt uiQTTempAccessLayer = pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - uiLog2TrSize;
    
    Bool  bCodeChroma   = true;
    UInt  uiTrModeC     = uiTrMode;
    UInt  uiLog2TrSizeC = uiLog2TrSize-1;
    if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      uiLog2TrSizeC++;
      uiTrModeC    --;
      UInt  uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrModeC ) << 1 );
      bCodeChroma   = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    }
    
    if( bSpatial )
    {
      m_pcQTTempTComYuv[uiQTTempAccessLayer].copyPartToPartLuma    ( pcResi, uiAbsPartIdx, 1<<uiLog2TrSize , 1<<uiLog2TrSize  );
      if( bCodeChroma )
      {
        m_pcQTTempTComYuv[uiQTTempAccessLayer].copyPartToPartChroma( pcResi, uiAbsPartIdx, 1<<uiLog2TrSizeC, 1<<uiLog2TrSizeC );
      }
    }
    else
    {
      UInt    uiNumCoeffPerAbsPartIdxIncrement = pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
      UInt    uiNumCoeffY = ( 1 << ( uiLog2TrSize << 1 ) );
      TCoeff* pcCoeffSrcY = m_ppcQTTempCoeffY [uiQTTempAccessLayer] +  uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx;
      TCoeff* pcCoeffDstY = pcCU->getCoeffY() + uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx;
      ::memcpy( pcCoeffDstY, pcCoeffSrcY, sizeof( TCoeff ) * uiNumCoeffY );
      if( bCodeChroma )
      {
        UInt    uiNumCoeffC = ( 1 << ( uiLog2TrSizeC << 1 ) );
        TCoeff* pcCoeffSrcU = m_ppcQTTempCoeffCb[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
        TCoeff* pcCoeffSrcV = m_ppcQTTempCoeffCr[uiQTTempAccessLayer] + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
        TCoeff* pcCoeffDstU = pcCU->getCoeffCb() + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
        TCoeff* pcCoeffDstV = pcCU->getCoeffCr() + (uiNumCoeffPerAbsPartIdxIncrement * uiAbsPartIdx>>2);
        ::memcpy( pcCoeffDstU, pcCoeffSrcU, sizeof( TCoeff ) * uiNumCoeffC );
        ::memcpy( pcCoeffDstV, pcCoeffSrcV, sizeof( TCoeff ) * uiNumCoeffC );
      }
    }
  }
  else
  {
    const UInt uiQPartNumSubdiv = pcCU->getPic()->getNumPartInCU() >> ((uiDepth + 1 ) << 1);
    for( UInt ui = 0; ui < 4; ++ui )
    {
      xSetResidualQTData( pcCU, uiAbsPartIdx + ui * uiQPartNumSubdiv, pcResi, uiDepth + 1, bSpatial );
    }
  }
}
#endif

Void TEncSearch::xEncodeInterTexture ( TComDataCU*& rpcCU, UInt uiQp, Bool bHighPass, TComYuv*& rpcYuv, UInt uiTrMode )
{
  UInt    uiWidth, uiHeight, uiCWidth, uiCHeight, uiLumaTrMode, uiChromaTrMode;
  TCoeff* piCoeff = rpcCU->getCoeffY();
  Pel*    pResi;
  
  uiWidth    = rpcCU->getWidth ( 0 );
  uiHeight   = rpcCU->getHeight( 0 );
  uiCWidth   = uiWidth >>1;
  uiCHeight  = uiHeight>>1;
  
  rpcCU->convertTransIdx( 0, uiTrMode, uiLumaTrMode, uiChromaTrMode );
  
  m_pcTrQuant->setQPforQuant( uiQp, !bHighPass, rpcCU->getSlice()->getSliceType(), TEXT_LUMA );
  
  rpcCU->clearCbf(0, TEXT_LUMA,     rpcCU->getTotalNumPart());
  rpcCU->clearCbf(0, TEXT_CHROMA_U, rpcCU->getTotalNumPart());
  rpcCU->clearCbf(0, TEXT_CHROMA_V, rpcCU->getTotalNumPart());
  
  UChar indexROT =     rpcCU->getROTindex(0);
  
  // Luma   Y
  piCoeff = rpcCU->getCoeffY();
  
  xRecurTransformNxN( rpcCU, 0, rpcYuv->getLumaAddr(), 0, rpcYuv->getStride(), uiWidth, uiHeight, uiLumaTrMode, 0, piCoeff, TEXT_LUMA, indexROT );
  rpcCU->setCuCbfLuma( 0, uiLumaTrMode );
  piCoeff = rpcCU->getCoeffY(); pResi = rpcYuv->getLumaAddr();
  rpcYuv->clearLuma();
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_LUMA, pResi, 0, rpcYuv->getStride(), uiWidth, uiHeight, uiLumaTrMode, 0, piCoeff, indexROT );
  
  m_pcTrQuant->setQPforQuant( uiQp, !bHighPass, rpcCU->getSlice()->getSliceType(), TEXT_CHROMA );
  
  // Chroma Cb
  piCoeff = rpcCU->getCoeffCb();
  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCbAddr(), 0, rpcYuv->getCStride(), uiCWidth, uiCHeight, uiChromaTrMode, 0, piCoeff, TEXT_CHROMA_U, indexROT );
  rpcCU->setCuCbfChromaUV( 0, uiChromaTrMode, TEXT_CHROMA_U );
  piCoeff = rpcCU->getCoeffCb(); pResi = rpcYuv->getCbAddr();
  rpcYuv->clearChromaUV(0);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_U, pResi, 0, rpcYuv->getCStride(), uiCWidth, uiCHeight, uiChromaTrMode, 0, piCoeff, indexROT );
  
  // Chroma Cr
  piCoeff = rpcCU->getCoeffCr();
  xRecurTransformNxN( rpcCU, 0, rpcYuv->getCrAddr(), 0, rpcYuv->getCStride(), uiCWidth, uiCHeight, uiChromaTrMode, 0, piCoeff, TEXT_CHROMA_V, indexROT );
  rpcCU->setCuCbfChromaUV( 0, uiChromaTrMode, TEXT_CHROMA_V );
  piCoeff = rpcCU->getCoeffCr(); pResi = rpcYuv->getCrAddr();
  rpcYuv->clearChromaUV(1);
  m_pcTrQuant->invRecurTransformNxN( rpcCU, 0, TEXT_CHROMA_V, pResi, 0, rpcYuv->getCStride(), uiCWidth, uiCHeight, uiChromaTrMode, 0, piCoeff, indexROT );
}

Void TEncSearch::xRecurTransformNxN( TComDataCU* rpcCU, UInt uiAbsPartIdx, Pel* pcResidual, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff*& rpcCoeff, TextType eType, Int indexROT )
{
  if ( uiTrMode == uiMaxTrMode )
  {
    UInt uiAbsSum;
    UInt uiCoeffOffset = uiWidth*uiHeight;
    
    if (m_pcEncCfg->getUseRDOQ())
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, uiWidth, eType );
    
    m_pcTrQuant->transformNxN( rpcCU, pcResidual + uiAddr, uiStride, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eType, uiAbsPartIdx, indexROT );
    
    if ( !rpcCU->getSlice()->isIntra() )
    {
      if ( uiAbsSum )
      {
        UInt uiBits, uiDist, uiDistCC;
        Double fCost, fCostCC;
        
        Pel* pcResidualRec = m_pTempPel;
        
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeCoeffNxN( rpcCU, rpcCoeff, uiAbsPartIdx, uiWidth, uiHeight, rpcCU->getDepth( 0 ) + uiTrMode, eType, true );
        uiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
#if QC_MDDT
        m_pcTrQuant->invtransformNxN( eType, REG_DCT, pcResidualRec, uiStride, rpcCoeff, uiWidth, uiHeight, indexROT);
#else
        m_pcTrQuant->invtransformNxN( pcResidualRec, uiStride, rpcCoeff, uiWidth, uiHeight, indexROT);
#endif
        //-- distortion when the coefficients are not cleared
        uiDist   = m_pcRdCost->getDistPart(pcResidualRec, uiStride, pcResidual+uiAddr, uiStride, uiWidth, uiHeight);
        
        //-- distortion when the coefficients are cleared
        memset(pcResidualRec, 0, sizeof(Pel)*uiHeight*uiStride);
        uiDistCC = m_pcRdCost->getDistPart(pcResidualRec, uiStride, pcResidual+uiAddr, uiStride, uiWidth, uiHeight);
        
        fCost   = m_pcRdCost->calcRdCost(uiBits, uiDist);
        fCostCC = m_pcRdCost->calcRdCost(0, uiDistCC);
        
        if ( fCostCC < fCost )
        {
          uiAbsSum = 0;
          memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
          rpcCU->setCbfSubParts( 0x00, eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
        }
      }
      else
      {
        rpcCU->setCbfSubParts( 0x00, eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
        memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
      }
    }
    else
    {
      if ( uiAbsSum )
      {
        rpcCU->setCbfSubParts( 1 << (uiTrMode), eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
      }
      else
      {
        rpcCU->setCbfSubParts( 0x00, eType, uiAbsPartIdx, rpcCU->getDepth( 0 ) + uiTrMode );
        memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
      }
    }
    
    rpcCoeff += uiCoeffOffset;
  }
  else
  {
    uiTrMode++;
    uiWidth  = uiWidth  >> 1;
    uiHeight = uiHeight >> 1;
    UInt uiQPartNum = rpcCU->getPic()->getNumPartInCU() >> ( ( rpcCU->getDepth(0)+uiTrMode ) << 1 );
    UInt uiAddrOffset = uiHeight * uiStride;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr                         , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiWidth               , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxN( rpcCU, uiAbsPartIdx, pcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
  }
}

#if SAMSUNG_FAST_UDI
UInt TEncSearch::xModeBitsIntra( TComDataCU* pcCU, UInt uiMode, UInt uiPU, UInt uiPartOffset, UInt uiDepth, UInt uiInitTrDepth )
{
  if( m_bUseSBACRD )
  {
    if ( uiPU ) m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth+1][CI_NEXT_BEST] );
    else        m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[uiDepth  ][CI_CURR_BEST] );
  }
  pcCU->setLumaIntraDirSubParts ( uiMode, uiPartOffset, uiDepth + uiInitTrDepth );

  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, uiPartOffset);
  
  return m_pcEntropyCoder->getNumberOfWrittenBits();
}

UInt TEncSearch::xUpdateCandList( UInt uiMode, Double uiCost, UInt uiFastCandNum, UInt * CandModeList, Double * CandCostList )
{
	UInt i;
	UInt shift=0;
	
  while ( shift<uiFastCandNum && uiCost<CandCostList[ uiFastCandNum-1-shift ] ) shift++;

	if( shift!=0 )
	{
		for(i=1; i<shift; i++)
		{
			CandModeList[ uiFastCandNum-i ] = CandModeList[ uiFastCandNum-1-i ];
			CandCostList[ uiFastCandNum-i ] = CandCostList[ uiFastCandNum-1-i ];
		}
		CandModeList[ uiFastCandNum-shift ] = uiMode;
		CandCostList[ uiFastCandNum-shift ] = uiCost;
    return 1;
	}

  return 0;
}
#endif

Void  TEncSearch::xAddSymbolBitsIntra( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiNumPart, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiWidth, UInt uiHeight, UInt& ruiBits )
{
  UInt uiPartOffset = uiPU*uiQNumPart;
  m_pcEntropyCoder->resetBits();
  
  if( uiPU==0 )
  {
    if( !pcCU->getSlice()->isIntra() )
    {
      m_pcEntropyCoder->encodeSkipFlag( pcCU, 0, true );
#if HHI_MRG && !HHI_MRG_PU
      m_pcEntropyCoder->encodeMergeInfo( pcCU, 0, true );      
#endif
      m_pcEntropyCoder->encodePredMode( pcCU, 0, true );
    }
    
#if PLANAR_INTRA
    if( pcCU->isIntra( 0 ) )
    {
      m_pcEntropyCoder->encodePlanarInfo( pcCU, 0, true );
      
      if ( pcCU->getPlanarInfo(0, PLANAR_FLAG) )
      {
        ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
        return;
      }
    }
#endif
    
    //BugFix for 1603
    m_pcEntropyCoder->encodePartSize( pcCU, 0, pcCU->getDepth(0), true );
    
    for( UInt ui = 0; ui < uiNumPart; ui++ )
    {
      m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, (uiPU+ui)*uiQNumPart );
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      m_pcEntropyCoder->encodeIntraFiltFlagLuma( pcCU, (uiPU+ui)*uiQNumPart );
#endif
    }
  }
  else
  {
    for( UInt ui = 0; ui < uiNumPart; ui++ )
    {
      m_pcEntropyCoder->encodeIntraDirModeLuma ( pcCU, (uiPU+ui)*uiQNumPart );
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      m_pcEntropyCoder->encodeIntraFiltFlagLuma( pcCU, (uiPU+ui)*uiQNumPart );
#endif
    }
  }
  
  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_LUMA, uiTrDepth );
#if QC_MDDT//ADAPTIVE_SCAN//USE_INTRA_MDDT_bRD //setting bRD to be true changes the results
  g_bUpdateStats = false;
  m_pcEntropyCoder->encodeCoeff( pcCU, pCoeff, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiWidth, uiHeight, uiMaxTrDepth, uiTrDepth, TEXT_LUMA );
#else
  m_pcEntropyCoder->encodeCoeff( pcCU, pCoeff, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiWidth, uiHeight, uiMaxTrDepth, uiTrDepth, TEXT_LUMA );
#endif
  ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
}

Void  TEncSearch::xAddSymbolBitsInter( TComDataCU* pcCU, UInt uiQp, UInt uiTrMode, UInt& ruiBits, TComYuv*& rpcYuvRec, TComYuv*pcYuvPred, TComYuv*& rpcYuvResi )
{
  if ( pcCU->isSkipped( 0 ) )
  {
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSkipFlag(pcCU, 0, true);
    ruiBits = m_pcEntropyCoder->getNumberOfWrittenBits();
    
    m_pcEntropyCoder->resetBits();
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, 0, REF_PIC_LIST_0);
    }
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, 0, REF_PIC_LIST_1);
    }
#ifdef DCM_PBIC
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      m_pcEntropyCoder->encodeICPIdx( pcCU, 0 );
    }
#endif
    ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  else
  {
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSkipFlag ( pcCU, 0, true );
#if HHI_MRG && !HHI_MRG_PU
    m_pcEntropyCoder->encodeMergeInfo( pcCU, 0, true );
#endif
    m_pcEntropyCoder->encodePredMode( pcCU, 0, true );
    m_pcEntropyCoder->encodePartSize( pcCU, 0, pcCU->getDepth(0), true );
    m_pcEntropyCoder->encodePredInfo( pcCU, 0, true );
    m_pcEntropyCoder->encodeCoeff   ( pcCU, 0, pcCU->getDepth(0), pcCU->getWidth(0), pcCU->getHeight(0) );
    
#if !USLESS_TR_CODE
    // ROT index
    if ( pcCU->getSlice()->getSPS()->getUseROT() )
    {
      m_pcEntropyCoder->encodeROTindex( pcCU, 0, pcCU->getDepth(0) );
    }
#endif
    ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  }
}


#ifdef DCM_PBIC
Int TEncSearch::xEstMvdIcdBits( TComDataCU *pcCU, TComMv* pcMvd, TComIc* pcIcd, RefPicList eRefList )
{
  Int iZeroPatt = 0;
  Int iIcParam[3];
  Int iCost;
  ContextModel* pcCtxModel = NULL;
  TComZeroTree* pcZTree;

  if( m_bUseSBACRD )
    m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST]);

  // AMVRES flag is not counted here as it would be a common cost to all candidates. Mvd is already expected to scaled accordingly.

  // Identify the non-zero components
  if (eRefList == REF_PIC_LIST_X)
  {
    iZeroPatt |= ( pcMvd[REF_PIC_LIST_0].getHor() == 0 ) ? 0 : 1;
    iZeroPatt |= ( pcMvd[REF_PIC_LIST_0].getVer() == 0 ) ? 0 : 2;
    iZeroPatt |= ( pcMvd[REF_PIC_LIST_1].getHor() == 0 ) ? 0 : 4;
    iZeroPatt |= ( pcMvd[REF_PIC_LIST_1].getVer() == 0 ) ? 0 : 8;

    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      pcIcd->getIcParam( iIcParam[0], iIcParam[1], iIcParam[2] );
      iZeroPatt |= ( iIcParam[0] == 0 ) ? 0 : 16;
      iZeroPatt |= ( iIcParam[1] == 0 ) ? 0 : 32;
      iZeroPatt |= ( iIcParam[2] == 0 ) ? 0 : 64;

      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDICDBI);
      pcCtxModel = m_pcEntropyCoder->m_pcEntropyCoderIf->getZTreeCtx(IDX_ZTREE_MVDICDBI);
    }
    else
    {
      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDBI);
      pcCtxModel = m_pcEntropyCoder->m_pcEntropyCoderIf->getZTreeCtx(IDX_ZTREE_MVDBI);
    }
  } 
  else
  {
    iZeroPatt |= ( pcMvd[eRefList].getHor() == 0 ) ? 0 : 1;
    iZeroPatt |= ( pcMvd[eRefList].getVer() == 0 ) ? 0 : 2;

    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      pcIcd->getIcParam( iIcParam[0], iIcParam[1], iIcParam[2] );
      iZeroPatt |= ( iIcParam[0] == 0 ) ? 0 : 4;
      assert ( iIcParam[1] == 0 );
      iZeroPatt |= ( iIcParam[2] == 0 ) ? 0 : 8;

      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDICDUNI);
      pcCtxModel = m_pcEntropyCoder->m_pcEntropyCoderIf->getZTreeCtx(IDX_ZTREE_MVDICDUNI);
    }
    else
    {
      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDUNI);
      pcCtxModel = m_pcEntropyCoder->m_pcEntropyCoderIf->getZTreeCtx(IDX_ZTREE_MVDUNI);
    }
  }

  // Cost to encode zeroflag and zerotree (if necessary)
  iCost = xcostBin( (iZeroPatt==0), m_pcEntropyCoder->m_pcEntropyCoderIf->getZTreeCtx(IDX_ZEROFLAG) );

  if (iZeroPatt != 0)
  {
    pcZTree->updateVal(iZeroPatt);
    iCost += xcostZTree( pcZTree, pcZTree->m_pcRoot, pcCtxModel );
  }
  iCost = ((iCost + (1<<14)) >> 15);

  //Cost to encode the non-zero components
  if ( (eRefList == REF_PIC_LIST_X) || (eRefList == REF_PIC_LIST_0) )
  {
    iCost += ( xGetMvdBits( pcMvd[REF_PIC_LIST_0] ) - 2 );
  }
  if ( (eRefList == REF_PIC_LIST_X) || (eRefList == REF_PIC_LIST_1) )
  {
    iCost += ( xGetMvdBits( pcMvd[REF_PIC_LIST_1] ) - 2 );
  }

  if (pcCU->getSlice()->getSPS()->getUseIC())
  {
    iCost += ( xGetIcdBits( *pcIcd ) - 3 );
  }

  return iCost;
}

Int TEncSearch::xcostZTree( TComZeroTree* pcZTree, TComZTNode* pcZTNode, ContextModel *pcCtxModel )
{
  Int iVal, iLval, iRval;
  Int iCost;

  iCost = 0;
  if (pcZTNode->IsLeaf() == false)
  {
    iLval = pcZTNode->m_pcLeft->m_iVal;
    iRval = pcZTNode->m_pcRight->m_iVal;

    iVal = iLval & iRval;
    iCost += xcostBin( iVal, (pcCtxModel == NULL) ? NULL : (pcCtxModel + 2*pcZTNode->m_id) );

    if (iVal == 0)
      iCost += xcostBin( iLval, (pcCtxModel == NULL) ? NULL : (pcCtxModel + 2*pcZTNode->m_id + 1) );

    if (iLval != 0)
      iCost += xcostZTree( pcZTree,  pcZTNode->m_pcLeft, pcCtxModel);
    if (iRval != 0)
      iCost += xcostZTree( pcZTree, pcZTNode->m_pcRight, pcCtxModel);
  }
  return iCost;
}

Int TEncSearch::xcostBin( UInt uiBin, ContextModel *pcCtxModel )
{
  if (pcCtxModel == NULL)
    return 32768;

  uiBin = (uiBin != 0);
  Int iState = (uiBin == pcCtxModel->getMps() ) ? 64 + pcCtxModel->getState() : 63 - pcCtxModel->getState();

  return entropyBits[127-iState];
}
#endif

Void TEncSearch::xExtDIFUpSamplingH ( TComPattern* pcPattern, TComYuv* pcYuvExt  )
{
  Int   x, y;
  
  Int   iWidth      = pcPattern->getROIYWidth();
  Int   iHeight     = pcPattern->getROIYHeight();
  
  Int   iPatStride  = pcPattern->getPatternLStride();
  Int   iExtStride  = pcYuvExt ->getStride();
  
  Int*  piSrcY;
  Int*  piDstY;
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  //  Copy integer-pel
  piSrcYPel = pcPattern->getROIY() - m_iDIFHalfTap - iPatStride;
  piDstY    = m_piYuvExt;//pcYuvExt->getLumaAddr();
  piDstYPel = pcYuvExt->getLumaAddr();
  for ( y = 0; y < iHeight + 2; y++ )
  {
    for ( x = 0; x < iWidth + m_iDIFTap; x++ )
    {
      piDstYPel[x << 2] = piSrcYPel[x];
    }
    piSrcYPel +=  iPatStride;
    piDstY    += (m_iYuvExtStride << 2);
    piDstYPel += (iExtStride      << 2);
  }
  
  //  Half-pel NORM. : vertical
  piSrcYPel = pcPattern->getROIY()    - iPatStride - m_iDIFHalfTap;
  piDstY    = m_piYuvExt              + (m_iYuvExtStride<<1);
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1);
  xCTI_FilterHalfVer     (piSrcYPel, iPatStride,     1, iWidth + m_iDIFTap, iHeight + 1, m_iYuvExtStride<<2, 4, piDstY, iExtStride<<2, piDstYPel);
  
  //  Half-pel interpolation : horizontal
  piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1;
  piDstYPel = pcYuvExt->getLumaAddr() + m_iDIFTap2 - 2;
  xCTI_FilterHalfHor (piSrcYPel, iPatStride,     1,  iWidth + 1, iHeight + 1,  iExtStride<<2, 4, piDstYPel);
  
  //  Half-pel interpolation : center
  piSrcY    = m_piYuvExt              + (m_iYuvExtStride<<1) + ((m_iDIFHalfTap-1) << 2);
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1)      + m_iDIFTap2 - 2;
  xCTI_FilterHalfHor       (piSrcY, m_iYuvExtStride<<2, 4, iWidth + 1, iHeight + 1,iExtStride<<2, 4, piDstYPel);
  
}

Void TEncSearch::xExtDIFUpSamplingQ   ( TComPattern* pcPatternKey, Pel* piDst, Int iDstStride, Pel* piSrcPel, Int iSrcPelStride, Int* piSrc, Int iSrcStride, UInt uiFilter )
{
  Int   x, y;
  
  Int   iWidth      = pcPatternKey->getROIYWidth();
  Int   iHeight     = pcPatternKey->getROIYHeight();
  
  Int*  piSrcY;
  Int*  piDstY;
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  Int iSrcStride4 = (iSrcStride<<2);
  Int iDstStride4 = (iDstStride<<2);
  
  switch (uiFilter)
  {
    case 0:
    {
      //  Quater-pel interpolation : vertical
      piSrcYPel = piSrcPel - m_iDIFHalfTap + 1;
      piDstY    = piSrc - m_iDIFTap2 + 2 - iSrcStride;
      xCTI_FilterQuarter0Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap - 1, iHeight, iSrcStride4, 4, piDstY);
      
      piSrcYPel = piSrcPel - m_iDIFHalfTap + 1;
      piDstY    = piSrc - m_iDIFTap2 + 2 + iSrcStride;
      xCTI_FilterQuarter1Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap - 1, iHeight, iSrcStride4, 4, piDstY);
      
      // Above three pixels
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst-1 - iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst   - iDstStride;;
      xCTI_FilterHalfHor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst+1 - iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Middle two pixels
      piSrcY    = piSrc-2;
      piDstYPel = piDst-1;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2;
      piDstYPel = piDst+1;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Below three pixels
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst-1 + iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst   + iDstStride;;
      xCTI_FilterHalfHor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst+1 + iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      break;
    }
    case 1:
    {
      //  Quater-pel interpolation : vertical
      piSrcYPel = piSrcPel - m_iDIFHalfTap;
      piDstY    = piSrc-m_iDIFTap2 - iSrcStride;
      xCTI_FilterQuarter0Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap, iHeight, iSrcStride4, 4, piDstY);
      
      piSrcYPel = piSrcPel - m_iDIFHalfTap;
      piDstY    = piSrc-m_iDIFTap2 + iSrcStride;
      xCTI_FilterQuarter1Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap, iHeight, iSrcStride4, 4, piDstY);
      
      // Left three pixels
      piSrcY    = piSrc-4 - iSrcStride;
      piDstYPel = piDst-1 - iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-4;
      piDstYPel = piDst-1;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-4 + iSrcStride;
      piDstYPel = piDst-1 + iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Middle two pixels
      piSrcY    = piSrc - iSrcStride;
      piDstYPel = piDst - iDstStride;
      Int iSrcStride2 = (iSrcStride<<1);
      Int iDstStride2 = (iDstStride<<1);
      
      for (y=0; y < iHeight*2; y++)
      {
        for (x=0; x < iWidth; x++)
        {
          piDstYPel[x*4] = Clip( (piSrcY[x*4] +  128) >>  8 );
        }
        piSrcY+=iSrcStride2;
        piDstYPel+=iDstStride2;
      }
      
      // Right three pixels
      piSrcY    = piSrc   - iSrcStride;
      piDstYPel = piDst+1 - iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc;
      piDstYPel = piDst+1;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc   + iSrcStride;
      piDstYPel = piDst+1 + iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      
      // Right three pixels
      piSrcY    = piSrc   - iSrcStride;
      piDstYPel = piDst+1 - iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc;
      piDstYPel = piDst+1;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc   + iSrcStride;
      piDstYPel = piDst+1 + iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Middle two pixels
      piSrcYPel = piSrcPel;
      piDstYPel = piDst - iDstStride;
      xCTI_FilterQuarter0Ver(piSrcYPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcYPel = piSrcPel;
      piDstYPel = piDst + iDstStride;
      xCTI_FilterQuarter1Ver(piSrcYPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      break;
    }
    case 2:
    {
      //  Quater-pel interpolation : vertical
      piSrcYPel = piSrcPel - m_iDIFHalfTap + 1 - iSrcPelStride;;
      piDstY    = piSrc - m_iDIFTap2 + 2 - iSrcStride;
      xCTI_FilterQuarter1Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap - 1, iHeight, iSrcStride4, 4, piDstY);
      
      piSrcYPel = piSrcPel - m_iDIFHalfTap + 1;
      piDstY    = piSrc - m_iDIFTap2 + 2 + iSrcStride;
      xCTI_FilterQuarter0Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap - 1, iHeight, iSrcStride4, 4, piDstY);
      
      // Above three pixels
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst-1 - iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst   - iDstStride;;
      xCTI_FilterHalfHor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 - iSrcStride;
      piDstYPel = piDst+1 - iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Middle two pixels
      piDstYPel = piDst - 1;
      xCTI_FilterQuarter0Hor(piSrcPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piDstYPel = piDst + 1;
      xCTI_FilterQuarter1Hor(piSrcPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Below three pixels
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst-1 + iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst   + iDstStride;;
      xCTI_FilterHalfHor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-2 + iSrcStride;
      piDstYPel = piDst+1 + iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      break;
    }
    case 3:
    {
      //  Quater-pel interpolation : vertical
      piSrcYPel = piSrcPel-m_iDIFHalfTap - iSrcPelStride;
      piDstY    = piSrc-m_iDIFTap2 - iSrcStride;
      xCTI_FilterQuarter1Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap, iHeight, iSrcStride4, 4, piDstY);
      
      piSrcYPel = piSrcPel-m_iDIFHalfTap;
      piDstY    = piSrc-m_iDIFTap2 + iSrcStride;
      xCTI_FilterQuarter0Ver(piSrcYPel, iSrcPelStride, 1, iWidth + m_iDIFTap, iHeight, iSrcStride4, 4, piDstY);
      
      // Left three pixels
      piSrcY    = piSrc-4 - iSrcStride;
      piDstYPel = piDst-1 - iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcYPel = piSrcPel-1;
      piDstYPel = piDst-1;
      xCTI_FilterQuarter1Hor(piSrcYPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc-4 + iSrcStride;
      piDstYPel = piDst-1 + iDstStride;
      xCTI_FilterQuarter1Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      // Middle two pixels
      piSrcY    = piSrc - iSrcStride;
      piDstYPel = piDst - iDstStride;
      Int iSrcStride2 = (iSrcStride<<1);
      Int iDstStride2 = (iDstStride<<1);
      
      for (y=0; y < iHeight*2; y++)
      {
        for (x=0; x < iWidth; x++)
        {
          piDstYPel[x*4] = Clip( (piSrcY[x*4] + 128) >>  8 );
        }
        piSrcY+=iSrcStride2;
        piDstYPel+=iDstStride2;
      }
      
      // Right three pixels
      piSrcY    = piSrc   - iSrcStride;
      piDstYPel = piDst+1 - iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piDstYPel = piDst+1;
      xCTI_FilterQuarter0Hor(piSrcPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      piSrcY    = piSrc   + iSrcStride;
      piDstYPel = piDst+1 + iDstStride;
      xCTI_FilterQuarter0Hor(piSrcY, iSrcStride4, 4, iWidth, iHeight, iDstStride4, 4, piDstYPel);
      
      break;
    }
    default:
    {
      assert(0);
    }
  }
}

#if TEN_DIRECTIONAL_INTERP
Void TEncSearch::xExtDIFUpSamplingH_TEN ( TComPattern* pcPattern, TComYuv* pcYuvExt  )
{
  Int   x, y;
  
  Int   iWidth      = pcPattern->getROIYWidth();
  Int   iHeight     = pcPattern->getROIYHeight();
  
  Int   iPatStride  = pcPattern->getPatternLStride();
  Int   iExtStride  = pcYuvExt ->getStride();
  
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  //  Copy integer-pel
  piSrcYPel = pcPattern->getROIY();
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<2);
  for ( y = 0; y < iHeight + 1; y++ )
  {
    for ( x = 0; x < iWidth; x++ )
    {
      piDstYPel[x << 2] = piSrcYPel[x];
    }
    piSrcYPel +=  iPatStride;
    piDstYPel += (iExtStride      << 2);
  }
  
  //  Half-pel NORM. : vertical
  piSrcYPel = pcPattern->getROIY()    - iPatStride;
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1);
  xCTI_FilterDIF_TEN (piSrcYPel, iPatStride, 1, iWidth, iHeight + 1, iExtStride<<2, 4, piDstYPel, 2, 0);
  
  //  Half-pel interpolation : horizontal
  piSrcYPel = pcPattern->getROIY() - 1;
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<2) - 2;
  xCTI_FilterDIF_TEN (piSrcYPel, iPatStride, 1, iWidth + 1, iHeight, iExtStride<<2, 4, piDstYPel, 0, 2);
  
  
  //  Half-pel interpolation : center
  piSrcYPel = pcPattern->getROIY()   -  iPatStride - 1;
  piDstYPel = pcYuvExt->getLumaAddr() + (iExtStride<<1) - 2;
  xCTI_FilterDIF_TEN (piSrcYPel, iPatStride, 1,  iWidth + 1, iHeight + 1, iExtStride<<2, 4, piDstYPel, 2, 2);
}

Void TEncSearch::xExtDIFUpSamplingQ_TEN   ( TComPattern* pcPatternKey, Pel* piDst, Int iDstStride, Pel* piSrcPel, Int iSrcPelStride, Int* piSrc, Int iSrcStride, UInt uiFilter )
{
  Int   iWidth      = pcPatternKey->getROIYWidth();
  Int   iHeight     = pcPatternKey->getROIYHeight();
  
  Pel*  piDstYPel;
  Pel*  piSrcYPel;
  
  Int iDstStride4 = (iDstStride<<2);
  
  Int i,j,i0,j0;
  
  /* Position of quarter pixel search center relative to integer pixel grid in units of quarter pixels */
  Int ioff = 2 - (uiFilter&2);
  Int joff = 2 - 2*(uiFilter&1);
  
  /* Loop over quarter pixel search candidates,
   (i0,j0) is relative to quarter pixel search center
   (i,j) is relative to the closest upper left integer pixel */
  for (i0 = -1; i0 < 2; i0++){
    i = (i0 + ioff)&3;
    for (j0 = -1; j0 < 2; j0++){
      j = (j0 + joff)&3;
      piSrcYPel = piSrcPel - ((j-j0)>>2) - ((i-i0)>>2)*iSrcPelStride;
      piDstYPel = piDst + j0 + i0*iDstStride;
      if (i0 || j0)
        xCTI_FilterDIF_TEN (piSrcYPel, iSrcPelStride, 1, iWidth, iHeight, iDstStride4, 4, piDstYPel, i, j);
    }
  }
}
#endif

Void TEncSearch::xPredIntraLumaNxNCIPEnc( TComPattern* pcTComPattern, Pel* pOrig, Pel* pPredCL, UInt uiStride, Pel* pPred, UInt uiPredStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAboveAvail, Bool bLeftAvail )
{
  Int*  ptrSrc;
  Int   sw, iWidth2;
  Int   x, y;
  
  // obtain source
  ptrSrc  = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
  
  // obtain source stride
  iWidth2 = iWidth<<1;
  sw = iWidth2+1;
  ptrSrc += sw+1;
  
  // compute DC value for non-availability
  Int  iDC  = 0;
  Int  iCnt = 0;
  
  if ( !bAboveAvail && !bLeftAvail )
  {
    iDC = 128 << g_uiBitIncrement;
  }
  else
    if ( !bAboveAvail )
    {
      for ( y=0; y<iHeight; y++ )
      {
        iDC  += ptrSrc[-1+y*sw];
        iCnt ++;
      }
      iDC = ( iDC + iCnt/2 ) / iCnt;
    }
    else
      if ( !bLeftAvail )
      {
        for ( x=0; x<iWidth; x++ )
        {
          iDC  += ptrSrc[x+(-1)*sw];
          iCnt ++;
        }
        iDC = ( iDC + iCnt/2 ) / iCnt;
      }
      else
      {
        for ( y=0; y<iHeight; y++ )
        {
          iDC  += ptrSrc[-1+y*sw];
          iCnt ++;
        }
        for ( x=0; x<iWidth; x++ )
        {
          iDC  += ptrSrc[x+(-1)*sw];
          iCnt ++;
        }
        iDC = ( iDC + iCnt/2 ) / iCnt;
      }
  
  // update prediction for left-top corner
  if ( bAboveAvail && bLeftAvail )
  {
    pPred[ 0 ] = CIP_PRED( ptrSrc[-1], ptrSrc[-sw], ptrSrc[-1-sw] );
  }
  else
    if ( bAboveAvail )
    {
      pPred[ 0 ] = CIP_PRED( iDC, ptrSrc[-sw], iDC );
    }
    else
      if ( bLeftAvail )
      {
        pPred[ 0 ] = CIP_PRED( ptrSrc[-1], iDC, iDC );
      }
      else
      {
        pPred[ 0 ] = iDC;
      }
  
  // update prediction for top side
  if ( bAboveAvail )
  {
    for ( x=1; x<iWidth; x++ )
    {
      pPred[ x ] = CIP_PRED( pOrig[x-1], ptrSrc[x-sw], ptrSrc[x-1-sw] );
    }
  }
  else
  {
    for ( x=1; x<iWidth; x++ )
    {
      pPred[ x ] = CIP_PRED( pOrig[x-1], iDC, iDC );
    }
  }
  
  // update prediction for left side
  if ( bLeftAvail )
  {
    for ( y=1; y<iHeight; y++ )
    {
      pPred[ y*uiPredStride ] = CIP_PRED( ptrSrc[-1+y*sw], pOrig[(y-1)*uiStride], ptrSrc[-1+(y-1)*sw] );
    }
  }
  else
  {
    for ( y=1; y<iHeight; y++ )
    {
      pPred[ y*uiPredStride ] = CIP_PRED( iDC, pOrig[(y-1)*uiStride], iDC );
    }
  }
  
  // update prediction for inner region
  for ( y=1; y<iHeight; y++ )
  {
    for ( x=1; x<iWidth; x++ )
    {
      pPred[ x+y*uiPredStride ] = CIP_PRED( pOrig[ (x-1)+y*uiStride], pOrig[ x+(y-1)*uiStride ], pOrig[ (x-1)+(y-1)*uiStride ] );
    }
  }
}

#ifdef DCM_PBIC
Void TEncSearch::xEstimateIcPredAICP( TComDataCU* pcCU, TComYuv* pcOrgYuv, UInt uiPartIdx, RefPicList  eRefPicList, TComIc& rcIcPred, Bool bFilled )
{
  AICPInfo* pcAICPInfo = pcCU->getCUIcField()->getAICPInfo();

  TComIc  cBestIc;
  Int     iBestIdx = 0;
  TComIc  cZeroIc;
  TComIc  cIcTemp;
  UInt    uiBestCost = MAX_INT;
  UInt    uiPartAddr = 0;
  Int     iRoiWidth, iRoiHeight;
  Int     i;
  
  pcCU->getPartIndexAndSize( uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );

  // Fill the IC Candidates
  if (!bFilled)
  {
    pcCU->fillICPCand( uiPartIdx, uiPartAddr, pcAICPInfo );
  }

  // initialize Icp index & Icp
  iBestIdx = 0;
	cBestIc=pcAICPInfo->m_acIcCand[0];

	if( pcAICPInfo->iN <= 1 )
  {
    rcIcPred = cBestIc;

	  pcCU->setICPIdxSubParts( iBestIdx, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr) );
    pcCU->setICPNumSubParts( pcAICPInfo->iN, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    return;
  }

  if ( bFilled )
  {
    assert(pcCU->getICPIdx(uiPartAddr) >= 0);
    rcIcPred = pcAICPInfo->m_acIcCand[pcCU->getICPIdx(uiPartAddr)];
    return;
  }

  //-- Check Minimum Cost.
  for ( i = 0 ; i < pcAICPInfo->iN; i++)
  {
    UInt uiTmpCost;
    cIcTemp = pcAICPInfo->m_acIcCand[i];
		cIcTemp.computeScaleOffset( eRefPicList);

    uiTmpCost = xGetTemplateCostIC( pcCU, uiPartIdx, uiPartAddr, pcOrgYuv, cIcTemp, i, pcAICPInfo->iN, eRefPicList, iRoiWidth, iRoiHeight);
    if ( uiBestCost > uiTmpCost )
    {
      uiBestCost = uiTmpCost;
      cBestIc    = pcAICPInfo->m_acIcCand[i];
      iBestIdx   = i;
    }
  }

  // Setting Best ICP
	rcIcPred=cBestIc;

  pcCU->setICPIdxSubParts( iBestIdx, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr) );
  pcCU->setICPNumSubParts( pcAICPInfo->iN, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
 
  return;
	
}

UInt TEncSearch::xGetIcpIdxBits(Int iIdx, Int iNum)
{
  assert(iIdx >= 0 && iNum >= 0 && iIdx < iNum);

  if (iNum == 1)
    return 0;

  UInt uiLength = 1;
  Int iTemp = iIdx;
  if ( iTemp == 0 )
  {
    return uiLength;
  }

  Bool bCodeLast = ( iNum-1 > iTemp );

  uiLength += (iTemp-1);

  if( bCodeLast )
  {
    uiLength++;
  }

  return uiLength;
}

Void  TEncSearch::predICompSearch( TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, Int iPartIdx, Int uiLastMode )
{
  Int   iRoiWidth;
  Int   iRoiHeight;
  Int   iIcpIdxCur;
  Int   iIcpIdxBest;

  UInt  uiPartAddr;
  UInt  uiCost;
  UInt  uiCostBest = MAX_UINT;
  
  TComIc cIc; 
  TComIc cIcd;
  TComIc cIcLMSlist[2];
  TComIc cIcPred;
  TComIc ZeroIc;
  TComIc cIc_test;

  RefPicList eRefPicList;

  PartSize ePartSize = pcCU->getPartitionSize( 0 );
  pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );

  m_acYuvTempIC[0].clear();
  m_acYuvTempIC[1].clear();

  if( uiLastMode==0 || uiLastMode==1 )
  {
    eRefPicList = (uiLastMode ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    Calc_ICParam_LMS_Uni( pcYuvOrg, &m_acYuvPred[eRefPicList], uiPartAddr, iRoiHeight, iRoiWidth, cIcLMSlist, eRefPicList);
  }
  else
  {
    eRefPicList = REF_PIC_LIST_X;
    Calc_ICParam_LMS_Bi( pcYuvOrg, &m_acYuvPred[0], &m_acYuvPred[1], uiPartAddr, iRoiHeight, iRoiWidth, cIcLMSlist );
  }

  AICPInfo* pcAICPInfo = pcCU->getCUIcField()->getAICPInfo();
  pcCU->fillICPCand( iPartIdx, uiPartAddr, pcAICPInfo );

  // Test LMS candidates
  for(Int iLMSIdx = 0; iLMSIdx < 2; iLMSIdx++)
  {
    cIc_test = cIcLMSlist[iLMSIdx];
    uiCost = MAX_UINT;
    xSearchBestICParam(&cIc_test, iIcpIdxCur,  0,  0,  0, iPartIdx, pcCU, pcYuvOrg, eRefPicList, uiCost); //Just test the candidate - no search
    if ( uiCost < uiCostBest )
    {
      uiCostBest  = uiCost;
      cIc         = cIc_test;
      iIcpIdxBest = iIcpIdxCur;
    }
  }

  // Test every predictor as a candidate
  for(Int iIcpIdxTemp = 0; iIcpIdxTemp < pcAICPInfo->iN; iIcpIdxTemp++)
  {
    cIc_test = pcAICPInfo->m_acIcCand[iIcpIdxTemp];
    uiCost = MAX_UINT;
    xSearchBestICParam(&cIc_test, iIcpIdxCur,  0,  0,  0, iPartIdx, pcCU, pcYuvOrg, eRefPicList, uiCost); //Just test the candidate - no search
    if ( uiCost < uiCostBest )
    {
      uiCostBest  = uiCost;
      cIc         = cIc_test;
      iIcpIdxBest = iIcpIdxCur;
    }
  }

  if (uiCostBest == MAX_UINT)
  {
    cIc.reset();
    iIcpIdxBest = 0;
  }

  cIc.computeScaleOffset( eRefPicList );
  pcCU->getCUIcField()->setAllIcField( cIc, ePartSize, uiPartAddr, iPartIdx, 0);

  cIcPred = pcAICPInfo->m_acIcCand[iIcpIdxBest];

  cIcd.copyIcParam(cIc);
  cIcd.subIcParamPred(cIcPred);
  pcCU->getCUIcField()->setAllIcd    ( cIcd, ePartSize, uiPartAddr, iPartIdx, 0);

  pcCU->setICPIdxSubParts( iIcpIdxBest, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
  pcCU->setICPNumSubParts( pcAICPInfo->iN, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));

  //Apply the best IC
  if (eRefPicList != REF_PIC_LIST_X)
  {
    xPredICompLumaBlk  ( &cIc, iRoiWidth   , iRoiHeight   , pcYuvPred->getStride() , 1, pcYuvPred->getLumaAddr(uiPartAddr), m_acYuvPred[eRefPicList].getStride() , 1, m_acYuvPred[eRefPicList].getLumaAddr(uiPartAddr), eRefPicList );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, pcYuvPred->getCStride(), 1, pcYuvPred->getCbAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCbAddr(uiPartAddr)  , eRefPicList );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, pcYuvPred->getCStride(), 1, pcYuvPred->getCrAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCrAddr(uiPartAddr)  , eRefPicList );
  }
  else
  {
    Int iRefIdx[2];
    iRefIdx[0] = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiPartAddr );
    iRefIdx[1] = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiPartAddr );

    xPredICompLumaBlk  ( &cIc, iRoiWidth   , iRoiHeight   , m_acYuvTempIC[0].getStride() , 1, m_acYuvTempIC[0].getLumaAddr(uiPartAddr), m_acYuvPred[0].getStride() , 1, m_acYuvPred[0].getLumaAddr(uiPartAddr), REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCbAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCbAddr(uiPartAddr)  , REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCrAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCrAddr(uiPartAddr)  , REF_PIC_LIST_0 );

    xPredICompLumaBlk  ( &cIc, iRoiWidth   , iRoiHeight   , m_acYuvTempIC[1].getStride() , 1, m_acYuvTempIC[1].getLumaAddr(uiPartAddr), m_acYuvPred[1].getStride() , 1, m_acYuvPred[1].getLumaAddr(uiPartAddr), REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCbAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCbAddr(uiPartAddr)  , REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIc, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCrAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCrAddr(uiPartAddr)  , REF_PIC_LIST_1 );

    xWeightedAverage ( pcCU, m_acYuvTempIC, m_acYuvTempIC+1, iRefIdx[0], iRefIdx[1], uiPartAddr, iRoiWidth, iRoiHeight, pcYuvPred);
  }

}

Bool TEncSearch::xSearchBestICParam( TComIc* cIcSearchCenter, Int& riIcpIdx, Int SearchRng0, Int SearchRng1, Int SearchRng2, UInt uiPartIdx, TComDataCU* pcCU, TComYuv* pcYuvOrg, RefPicList eRefPicList, UInt& ruiCostBest, Bool bFlagReset)
{ 
  Int i,j,k;
  UInt  uiCost;

  TComIc cIcRefine;
  TComIc cIcTest;
  Int iPram0RefineBest  = 0; 
  Int iPram1RefineBest  = 0;
  Int iPram2RefineBest  = 0;
  Int iIcpIdx           = 0;
  Int iIcpIdxBest       = 0;
  Bool bNewBest = false;

  ruiCostBest = MAX_UINT;
  Int aiCurParam[3], aiCenParam[3];

  assert (IC_SCALE_PREC >= 1);

  if (bFlagReset == true)
  {
    return false;
  }

  cIcSearchCenter->getIcParam(aiCenParam[0], aiCenParam[1], aiCenParam[2]);

  if (eRefPicList != REF_PIC_LIST_X) { SearchRng1 = 0; }

  for( i = -SearchRng0; i <= SearchRng0; i++)
  {
    for( j = -SearchRng1; j <= SearchRng1; j++)
    {
      for( k = -SearchRng2; k <= SearchRng2; k++)
      {
        aiCurParam[0] = aiCenParam[0] + i;
        aiCurParam[1] = aiCenParam[1] + j;
        aiCurParam[2] = aiCenParam[2] + k;

        cIcTest.setIcParam(aiCurParam[0], aiCurParam[1], aiCurParam[2]);
        cIcTest.computeScaleOffset( eRefPicList );
        uiCost = xCalculateCostAfterICPred(cIcTest, iIcpIdx, pcCU, pcYuvOrg, uiPartIdx, eRefPicList);

        if ( uiCost < ruiCostBest )
        {
          ruiCostBest      = uiCost;
          iPram0RefineBest = i;
          iPram1RefineBest = j;
          iPram2RefineBest = k;
          iIcpIdxBest      = iIcpIdx;
          bNewBest         = true;
        }
      }
    }
  }
  //Update cIcSearchCenter
  if (bNewBest == true)
  {
    cIcRefine.setIcParam(iPram0RefineBest,iPram1RefineBest,iPram2RefineBest);
    cIcSearchCenter->addIcParamDiff(cIcRefine);
    cIcSearchCenter->computeScaleOffset(eRefPicList);
    riIcpIdx = iIcpIdxBest;
  }

  return bNewBest;
}

UInt TEncSearch::xCalculateCostAfterICPred( TComIc cIcCand, Int& riIcpIdx, TComDataCU* pcCU, TComYuv* pcYuvOrg, UInt uiPartIdx, RefPicList eRefPicList )
{
  Int    iRoiWidth;
  Int    iRoiHeight;
  UInt   uiPartAddr;
  TComIc cIcPred;
  TComIc cIcd;

  UInt   uiCost;
  UInt   IcpBits;
  UInt   IcBits;

  TComMv acMvd[2];

  pcCU->getPartIndexAndSize( uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );

  AICPInfo* pcAICPInfo = pcCU->getCUIcField()->getAICPInfo();
  Int iIcpIdx;
  Int iIcpNum = pcAICPInfo->iN;
  UInt IcBitsBest = MAX_UINT;
  for (iIcpIdx = 0; iIcpIdx < pcAICPInfo->iN; iIcpIdx++)
  {

    cIcPred = pcAICPInfo->m_acIcCand[iIcpIdx];

    IcpBits = m_auiICPIdxCost[iIcpIdx][iIcpNum];

    // compute ICD
    cIcd.copyIcParam(cIcCand);
    cIcd.subIcParamPred(cIcPred);

    // add the estimated cost of coding MVD & ICD
    if (eRefPicList != REF_PIC_LIST_X)
    {
      acMvd[eRefPicList] = pcCU->getCUMvField( eRefPicList )->getMvd   ( uiPartAddr );
    }
    else
    {
      acMvd[0]   = pcCU->getCUMvField( REF_PIC_LIST_0 )->getMvd   ( uiPartAddr );
      acMvd[1]   = pcCU->getCUMvField( REF_PIC_LIST_1 )->getMvd   ( uiPartAddr );
    }
#ifdef QC_AMVRES
    if ( pcCU->getSlice()->getSPS()->getUseAMVRes() )
    {
      if (( (eRefPicList == REF_PIC_LIST_0) || (eRefPicList == REF_PIC_LIST_X) ) && ( !(pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv( uiPartAddr ).isHAM()) ))
        acMvd[REF_PIC_LIST_0].scale_down();
      if (( (eRefPicList == REF_PIC_LIST_1) || (eRefPicList == REF_PIC_LIST_X) ) && ( !(pcCU->getCUMvField( REF_PIC_LIST_1 )->getMv( uiPartAddr ).isHAM()) ))
        acMvd[REF_PIC_LIST_1].scale_down();
    }
#endif
    IcBits = IcpBits + (UInt)xEstMvdIcdBits( pcCU, acMvd, &cIcd, eRefPicList );

    if (cIcCand.isequalIcParam(cIcPred))
    {
      riIcpIdx = iIcpIdx;
      IcBitsBest = IcBits;
      break;
    }
    if (IcBits < IcBitsBest)
    {
      riIcpIdx = iIcpIdx;
      IcBitsBest = IcBits;
    }
  }
  iIcpIdx = riIcpIdx;
  IcBits = IcBitsBest;

  if (eRefPicList != REF_PIC_LIST_X)
  {
    xPredICompLumaBlk  ( &cIcCand, iRoiWidth   , iRoiHeight   , m_acYuvTempIC->getStride() , 1, m_acYuvTempIC->getLumaAddr(uiPartAddr), m_acYuvPred[eRefPicList].getStride() , 1, m_acYuvPred[eRefPicList].getLumaAddr(uiPartAddr), eRefPicList );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC->getCStride(), 1, m_acYuvTempIC->getCbAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCbAddr(uiPartAddr)  , eRefPicList );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC->getCStride(), 1, m_acYuvTempIC->getCrAddr(uiPartAddr)  , m_acYuvPred[eRefPicList].getCStride(), 1, m_acYuvPred[eRefPicList].getCrAddr(uiPartAddr)  , eRefPicList );
  }
  else
  {
    Int iRefIdx[2];
    iRefIdx[0] = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiPartAddr );
    iRefIdx[1] = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiPartAddr );

    xPredICompLumaBlk  ( &cIcCand, iRoiWidth   , iRoiHeight   , m_acYuvTempIC[0].getStride() , 1, m_acYuvTempIC[0].getLumaAddr(uiPartAddr), m_acYuvPred[0].getStride() , 1, m_acYuvPred[0].getLumaAddr(uiPartAddr), REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCbAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCbAddr(uiPartAddr)  , REF_PIC_LIST_0 );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[0].getCStride(), 1, m_acYuvTempIC[0].getCrAddr(uiPartAddr)  , m_acYuvPred[0].getCStride(), 1, m_acYuvPred[0].getCrAddr(uiPartAddr)  , REF_PIC_LIST_0 );

    xPredICompLumaBlk  ( &cIcCand, iRoiWidth   , iRoiHeight   , m_acYuvTempIC[1].getStride() , 1, m_acYuvTempIC[1].getLumaAddr(uiPartAddr), m_acYuvPred[1].getStride() , 1, m_acYuvPred[1].getLumaAddr(uiPartAddr), REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCbAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCbAddr(uiPartAddr)  , REF_PIC_LIST_1 );
    xPredICompChromaBlk( &cIcCand, iRoiWidth>>1, iRoiHeight>>1, m_acYuvTempIC[1].getCStride(), 1, m_acYuvTempIC[1].getCrAddr(uiPartAddr)  , m_acYuvPred[1].getCStride(), 1, m_acYuvPred[1].getCrAddr(uiPartAddr)  , REF_PIC_LIST_1 );

    TComYuv* pcYuvPred = m_acYuvTempIC;
    xWeightedAverage( pcCU, m_acYuvTempIC, m_acYuvTempIC+1, iRefIdx[0], iRefIdx[1], uiPartAddr, iRoiWidth, iRoiHeight, pcYuvPred);
  }

  // calc distortion ( HADSAD + bitcost )
  DFunc eDFunc = (m_pcEncCfg->getUseHADME() == true) ? DF_HADS : DF_SADS;
  uiCost = m_pcRdCost->getDistPart(  m_acYuvTempIC->getLumaAddr(uiPartAddr),  m_acYuvTempIC->getStride(), pcYuvOrg->getLumaAddr(uiPartAddr), pcYuvOrg->getStride(), iRoiWidth, iRoiHeight, eDFunc );
  uiCost += UInt(m_pcRdCost->getDistPart( m_acYuvTempIC->getCbAddr(uiPartAddr), m_acYuvTempIC->getCStride(),pcYuvOrg->getCbAddr(uiPartAddr),pcYuvOrg->getCStride(), iRoiWidth>>1, iRoiHeight>>1, eDFunc)
              +  m_pcRdCost->getDistPart( m_acYuvTempIC->getCrAddr(uiPartAddr), m_acYuvTempIC->getCStride(),pcYuvOrg->getCrAddr(uiPartAddr),pcYuvOrg->getCStride(), iRoiWidth>>1, iRoiHeight>>1, eDFunc));
  uiCost = (UInt) ( uiCost / 1.5 );

  UInt uiCostTmp = uiCost;
  uiCost = (UInt) m_pcRdCost->calcRdCost( IcBits, uiCost, false, DF_SAD );
  assert(uiCost >= uiCostTmp);

  return uiCost;
}

Double TEncSearch::xComputeSum( TComYuv* pcYuvSrc, UInt uiPartAddr, Int iWidth, Int iHeight, Int YCbCr )
{
  Int x, y;
  Double dbSum=0.0;
  Pel* pSrc; 
  Int iStride;

  if(YCbCr==0){
	  pSrc   = pcYuvSrc->getLumaAddr(uiPartAddr);
	  iStride = pcYuvSrc->getStride();
 }
  if(YCbCr==1){
		pSrc   = pcYuvSrc->getCbAddr(uiPartAddr);
		iStride = pcYuvSrc->getCStride();
	}
	if(YCbCr==2){
		pSrc   = pcYuvSrc->getCrAddr(uiPartAddr);
		iStride = pcYuvSrc->getCStride();
	}

  for (y=0; y<iHeight; y++)
  {
    for (x=0; x<iWidth; x++)
    {
      dbSum += (Double)(pSrc[x]);
    }
    pSrc += iStride;  
	}

  return dbSum;
}

Double TEncSearch::xComputeSquareSum( TComYuv* pcYuvSrc, UInt uiPartAddr, Int iWidth, Int iHeight, Int YCbCr )
{
  Int x, y;
  Double dbSquareSum=0.0;
  Pel* pSrc; 
  Int iStride;

  if(YCbCr==0){
	  pSrc    = pcYuvSrc->getLumaAddr(uiPartAddr);
	  iStride = pcYuvSrc->getStride();
 }
  if(YCbCr==1){
		pSrc    = pcYuvSrc->getCbAddr(uiPartAddr);
		iStride = pcYuvSrc->getCStride();
	}
	if(YCbCr==2){
		pSrc    = pcYuvSrc->getCrAddr(uiPartAddr);
		iStride = pcYuvSrc->getCStride();
	}

  for (y=0; y<iHeight; y++)
  {
    for (x=0; x<iWidth; x++)
    {
			dbSquareSum += (((Double)pSrc[x])*pSrc[x]);

    }
		pSrc += iStride;
  }
  return dbSquareSum;
}

Double TEncSearch::xComputeSumMult( TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, UInt uiPartAddr, Int iWidth, Int iHeight, Int YCbCr )
{
  Int x, y;
  Double dbSumMult=0.0;
	Pel* pSrc0;
	Pel* pSrc1;
	Int iStride0,iStride1;

 if(YCbCr==0){
    pSrc0    = pcYuvSrc0->getLumaAddr(uiPartAddr);
    iStride0 = pcYuvSrc0->getStride();
 
	  pSrc1    = pcYuvSrc1->getLumaAddr(uiPartAddr);
	  iStride1 = pcYuvSrc1->getStride();
 }
 if(YCbCr==1){
    pSrc0   = pcYuvSrc0->getCbAddr(uiPartAddr);
    iStride0 = pcYuvSrc0->getCStride();

    pSrc1   = pcYuvSrc1->getCbAddr(uiPartAddr);
    iStride1 = pcYuvSrc1->getCStride();
 }
 if(YCbCr==2){
		pSrc0   = pcYuvSrc0->getCrAddr(uiPartAddr);
		iStride0 = pcYuvSrc0->getCStride();

		pSrc1   = pcYuvSrc1->getCrAddr(uiPartAddr);
		iStride1 = pcYuvSrc1->getCStride();
	}

  for (y=0; y<iHeight; y++)
  {
    for (x=0; x<iWidth; x++)
    {
			dbSumMult += (((Double)pSrc0[x])*pSrc1[x]);
    }
    pSrc0 += iStride0;
	  pSrc1 += iStride1;
		
  }
  return dbSumMult;
}

Void TEncSearch::Calc_ICParam_LMS_Uni( TComYuv* pcYuvOrg, TComYuv* pcYuvPred, UInt uiPartAddr, Int iHeight, Int iWidth, TComIc* rcIc, RefPicList eRefPicList )
{
  Double dbScale0;
  Double dbOffset;

  Int iParam0;
  Int iParam2;
  Int iDCoffset = (128 << g_uiBitIncrement);
  Int iRound = (g_uiBitIncrement>0) ? (1<<(g_uiBitIncrement-1)) : 0;

  Double dbOrgSum          = xComputeSum(pcYuvOrg ,uiPartAddr,iWidth,iHeight,0);          // sum(OrgY)
  Double dbPred0Sum        = xComputeSum(pcYuvPred,uiPartAddr,iWidth,iHeight,0);          // sum(PredY)
  Double dbPred0SquareSum  = xComputeSquareSum(pcYuvPred ,uiPartAddr,iWidth,iHeight,0); 
  Double dbPred0OrgSum     = xComputeSumMult(pcYuvPred ,pcYuvOrg ,uiPartAddr,iWidth,iHeight,0);
                         
  Double dbN                 = iWidth * iHeight; //N : d

  Double dbNchroma     = (iWidth>>1) * (iHeight>>1);
  Double dbOrgCbSum    = xComputeSum(pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbOrgCrSum    = xComputeSum(pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,2);
  Double dbPred0CbSum  = xComputeSum(pcYuvPred,uiPartAddr,iWidth>>1,iHeight>>1,1);    
  Double dbPred0CrSum  = xComputeSum(pcYuvPred,uiPartAddr,iWidth>>1,iHeight>>1,2); 

  Double dbPred0CbSquareSum = xComputeSquareSum(pcYuvPred ,uiPartAddr,iWidth>>1,iHeight>>1,1); 
  Double dbPred0CrSquareSum = xComputeSquareSum(pcYuvPred ,uiPartAddr,iWidth>>1,iHeight>>1,2); 

  Double dbPred0CbOrgCbSum  = xComputeSumMult(pcYuvPred ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbPred0CrOrgCrSum  = xComputeSumMult(pcYuvPred ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,2);

  dbPred0CbSquareSum -= iDCoffset * ( dbPred0CbSum + dbPred0CbSum - (dbNchroma * iDCoffset) );
  dbPred0CrSquareSum -= iDCoffset * ( dbPred0CrSum + dbPred0CrSum - (dbNchroma * iDCoffset) );
  dbPred0CbOrgCbSum  -= iDCoffset * ( dbPred0CbSum + dbOrgCbSum   - (dbNchroma * iDCoffset) );
  dbPred0CrOrgCrSum  -= iDCoffset * ( dbPred0CrSum + dbOrgCrSum   - (dbNchroma * iDCoffset) );

  dbPred0SquareSum += ( dbPred0CbSquareSum + dbPred0CrSquareSum);  // update to sum(predY^2)+ sum( predCb^2) + sum ( predCr^2) :a
  dbPred0OrgSum    += ( dbPred0CbOrgCbSum  + dbPred0CrOrgCrSum);  // update to sum(predY * OrgY) + sum(predCb * OrgCb) + sum(predCr * OrgCr)

  Double dbDet =  (dbPred0SquareSum * dbN) - (dbPred0Sum * dbPred0Sum) ;

  if( dbDet > 0 )
  {
    dbScale0 =((dbN * dbPred0OrgSum) - (dbPred0Sum  * dbOrgSum)) / dbDet;
    iParam0 = ICROUND( (dbScale0 - 1.0) * (1 << IC_SCALE_PREC) );

    dbScale0 = 1 + Double(iParam0) / (1 << IC_SCALE_PREC);
    dbOffset = ( dbOrgSum - dbPred0Sum*dbScale0 ) / dbN;
    iParam2 = ( ICROUND(dbOffset) + iRound ) >> g_uiBitIncrement;
  }
  else
  {
	  iParam0=0;
	  iParam2=0;
  }

  rcIc->setIcParam(iParam0,0,iParam2); 
  rcIc->computeScaleOffset( eRefPicList );

  // Scale only
  rcIc++;
  if (dbPred0SquareSum > 0)
  {
    dbScale0 = dbPred0OrgSum / dbPred0SquareSum;
    iParam0 = ICROUND( (dbScale0 - 1.0) * (1 << IC_SCALE_PREC) );
  }
  else
  {
    iParam0 = 0;
  }
  rcIc->setIcParam(iParam0, 0, 0);
  rcIc->computeScaleOffset( eRefPicList );

  return;
}

Void TEncSearch::Calc_ICParam_LMS_Bi( TComYuv* pcYuvOrg, TComYuv* pcYuvPred0, TComYuv* pcYuvPred1, UInt uiPartAddr, Int iHeight, Int iWidth, TComIc* rcIc )
{
  Double dbScale0;
  Double dbScale1;
  Double dbOffset;

  Int iParam0;
  Int iParam1;
  Int iParam2;
  Int iDCoffset = (128 << g_uiBitIncrement);
  Int iRound = (g_uiBitIncrement>0) ? (1<<(g_uiBitIncrement-1)) : 0;

  Double dbOrgSum   = xComputeSum(pcYuvOrg   ,uiPartAddr,iWidth,iHeight,0); // sum(OrgY)
  Double dbPred0Sum = xComputeSum(pcYuvPred0 ,uiPartAddr,iWidth,iHeight,0); //sum(pred0Y)
  Double dbPred1Sum = xComputeSum(pcYuvPred1 ,uiPartAddr,iWidth,iHeight,0); //sum(pred1Y)

  Double dbPred0SquareSum = xComputeSquareSum(pcYuvPred0,uiPartAddr,iWidth,iHeight,0);
  Double dbPred1SquareSum = xComputeSquareSum(pcYuvPred1,uiPartAddr,iWidth,iHeight,0);
  Double dbN             = iWidth * iHeight; //N

  Double dbPred0Pred1Sum  = xComputeSumMult(pcYuvPred0,pcYuvPred1,uiPartAddr,iWidth,iHeight,0);
  Double dbPred0OrgSum    = xComputeSumMult(pcYuvPred0,pcYuvOrg  ,uiPartAddr,iWidth,iHeight,0);
  Double dbPred1OrgSum    = xComputeSumMult(pcYuvPred1,pcYuvOrg  ,uiPartAddr,iWidth,iHeight,0);

  Double dbNchroma     = (iWidth>>1) * (iHeight>>1);
  Double dbOrgCbSum    = xComputeSum(pcYuvOrg  ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbOrgCrSum    = xComputeSum(pcYuvOrg  ,uiPartAddr,iWidth>>1,iHeight>>1,2);
  Double dbPred0CbSum  = xComputeSum(pcYuvPred0,uiPartAddr,iWidth>>1,iHeight>>1,1);    
  Double dbPred0CrSum  = xComputeSum(pcYuvPred0,uiPartAddr,iWidth>>1,iHeight>>1,2); 
  Double dbPred1CbSum  = xComputeSum(pcYuvPred1,uiPartAddr,iWidth>>1,iHeight>>1,1);    
  Double dbPred1CrSum  = xComputeSum(pcYuvPred1,uiPartAddr,iWidth>>1,iHeight>>1,2); 

  Double dbPred0CbSquareSum = xComputeSquareSum(pcYuvPred0,uiPartAddr,iWidth>>1,iHeight>>1,1); 
  Double dbPred0CrSquareSum = xComputeSquareSum(pcYuvPred0,uiPartAddr,iWidth>>1,iHeight>>1,2); 
  Double dbPred1CbSquareSum = xComputeSquareSum(pcYuvPred1,uiPartAddr,iWidth>>1,iHeight>>1,1); 
  Double dbPred1CrSquareSum = xComputeSquareSum(pcYuvPred1,uiPartAddr,iWidth>>1,iHeight>>1,2); 

  Double dbPred0CbOrgCbSum  = xComputeSumMult(pcYuvPred0 ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbPred0CrOrgCrSum  = xComputeSumMult(pcYuvPred0 ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,2);
  Double dbPred1CbOrgCbSum  = xComputeSumMult(pcYuvPred1 ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbPred1CrOrgCrSum  = xComputeSumMult(pcYuvPred1 ,pcYuvOrg ,uiPartAddr,iWidth>>1,iHeight>>1,2);

  Double dbPred0CbPred1CbSum  = xComputeSumMult(pcYuvPred0 ,pcYuvPred1 ,uiPartAddr,iWidth>>1,iHeight>>1,1);
  Double dbPred0CrPred1CrSum  = xComputeSumMult(pcYuvPred0 ,pcYuvPred1 ,uiPartAddr,iWidth>>1,iHeight>>1,2);

  dbPred0CbSquareSum -= iDCoffset * ( dbPred0CbSum + dbPred0CbSum - (dbNchroma * iDCoffset) );
  dbPred0CrSquareSum -= iDCoffset * ( dbPred0CrSum + dbPred0CrSum - (dbNchroma * iDCoffset) );
  dbPred1CbSquareSum -= iDCoffset * ( dbPred1CbSum + dbPred1CbSum - (dbNchroma * iDCoffset) );
  dbPred1CrSquareSum -= iDCoffset * ( dbPred1CrSum + dbPred1CrSum - (dbNchroma * iDCoffset) );

  dbPred0CbOrgCbSum  -= iDCoffset * ( dbPred0CbSum + dbOrgCbSum   - (dbNchroma * iDCoffset) );
  dbPred0CrOrgCrSum  -= iDCoffset * ( dbPred0CrSum + dbOrgCrSum   - (dbNchroma * iDCoffset) );
  dbPred1CbOrgCbSum  -= iDCoffset * ( dbPred1CbSum + dbOrgCbSum   - (dbNchroma * iDCoffset) );
  dbPred1CrOrgCrSum  -= iDCoffset * ( dbPred1CrSum + dbOrgCrSum   - (dbNchroma * iDCoffset) );
  dbPred0CbPred1CbSum -= iDCoffset * ( dbPred0CbSum + dbPred1CbSum   - (dbNchroma * iDCoffset) );
  dbPred0CrPred1CrSum -= iDCoffset * ( dbPred0CrSum + dbPred1CrSum   - (dbNchroma * iDCoffset) );

  dbPred0SquareSum += ( dbPred0CbSquareSum + dbPred0CrSquareSum);  // sum(pred0Y^2) + sum (pred0Cb^2) + sum ( pred0Cr^2)
  dbPred1SquareSum += ( dbPred1CbSquareSum + dbPred1CrSquareSum);  // sum(pred0Y^2) + sum (pred0Cb^2) + sum ( pred0Cr^2)
  dbPred0Pred1Sum  += ( dbPred0CbPred1CbSum + dbPred0CrPred1CrSum);// sum(pred0Y*pred1Y) + sum(pred0Cb*pred1Cb) + sum(pred0Cr*pred1Cr)
  dbPred0OrgSum    += ( dbPred0CbOrgCbSum + dbPred0CrOrgCrSum);    // sum(pred0Y* OrgY) + sum(pred0Cb * OrgCb) + sum(pred0Cr* OrgCr)
  dbPred1OrgSum    += ( dbPred1CbOrgCbSum + dbPred1CrOrgCrSum);    // sum(pred1Y* OrgY) + sum(pred1Cb * OrgCb) + sum(pred1Cr* OrgCr)

  Double dbA = (dbN * dbPred1SquareSum)  - (dbPred1Sum * dbPred1Sum);
  Double dbB = (dbPred0Sum * dbPred1Sum) - (dbN * dbPred0Pred1Sum);
  Double dbC = (dbPred1Sum * dbPred0Pred1Sum) -  (dbPred1SquareSum * dbPred0Sum);

  Double dbD = (dbN * dbPred0SquareSum) - (dbPred0Sum * dbPred0Sum);
  Double dbE = (dbPred0Pred1Sum * dbPred0Sum) - (dbPred1Sum * dbPred0SquareSum);
  Double dbF = (dbPred0SquareSum * dbPred1SquareSum) - (dbPred0Pred1Sum * dbPred0Pred1Sum);

  Double dbDet = (dbPred0SquareSum * dbA) + (dbPred0Pred1Sum * dbB) + (dbPred0Sum * dbC);

  if( dbDet > 0 )
  {
    dbScale0 = ((dbPred0OrgSum * dbA) + (dbPred1OrgSum * dbB) + (dbOrgSum * dbC))/dbDet;
    dbScale1 = ((dbPred0OrgSum * dbB) + (dbPred1OrgSum * dbD) + (dbOrgSum * dbE))/dbDet;
    iParam0 = ICROUND( (dbScale0 + dbScale1 - 1.0) * (1 <<  IC_SCALE_PREC   ) );
    iParam1 = ICROUND( (dbScale0 - dbScale1      ) * (1 << (IC_SCALE_PREC-1)) );

    dbScale0 = 0.5 * (1 + Double(iParam0 + (iParam1<<1)) / (1 << IC_SCALE_PREC));
    dbScale1 = 0.5 * (1 + Double(iParam0 - (iParam1<<1)) / (1 << IC_SCALE_PREC));
    dbOffset = ( dbOrgSum - (dbPred0Sum*dbScale0 + dbPred1Sum*dbScale1) ) / dbN;
    iParam2 = ( ICROUND(dbOffset) + iRound ) >> g_uiBitIncrement;
  }
  else
  {
	  iParam0=0;
	  iParam1=0;
	  iParam2=0;
  }

  rcIc->setIcParam(iParam0,iParam1,iParam2);
  rcIc->computeScaleOffset( REF_PIC_LIST_X  );

  // Scale only
  rcIc++;
  if (dbF > 0)
  {
    dbScale0 = ( (dbPred0OrgSum*dbPred1SquareSum) - (dbPred1OrgSum*dbPred0Pred1Sum) ) / dbF;
    dbScale1 = ( (dbPred1OrgSum*dbPred0SquareSum) - (dbPred0OrgSum*dbPred0Pred1Sum) ) / dbF;
    iParam0 = ICROUND( (dbScale0 + dbScale1 - 1.0) * (1 <<  IC_SCALE_PREC   ) );
    iParam1 = ICROUND( (dbScale0 - dbScale1      ) * (1 << (IC_SCALE_PREC-1)) );
  }
  else
  {
	  iParam0 = 0;
	  iParam1 = 0;
  }
  rcIc->setIcParam(iParam0, iParam1, 0);
  rcIc->computeScaleOffset( REF_PIC_LIST_X  );

  return;
}

UInt TEncSearch::xGetComponentBits(Int iVal)
{
  UInt uiLength = 1;
  UInt uiTemp   = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);

  assert ( uiTemp );

  while ( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  return uiLength;
}

UInt TEncSearch::xGetMvdBits(TComMv cMvd)
{
  return ( xGetComponentBits(cMvd.getHor()) + xGetComponentBits(cMvd.getVer()) );
}

UInt TEncSearch::xGetIcdBits(TComIc cIcd)
{
  Int aiParam[3];
  cIcd.getIcParam( aiParam[0], aiParam[1], aiParam[2] );
  return ( xGetComponentBits(aiParam[0]) + xGetComponentBits(aiParam[1]) + xGetComponentBits(aiParam[2]) );
}
#endif //#ifdef DCM_PBIC
