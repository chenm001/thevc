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

/** \file			TEncSearch.cpp
    \brief		encoder search class
*/

#include "../TLibCommon/TypeDef.h"
#include "../TLibCommon/TComMotionInfo.h"
#include "TEncSearch.h"

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

static UInt s_auiDFilter[9] =
{
	0, 1, 0,
	2, 3, 2,
	0, 1, 0
};

TEncSearch::TEncSearch()
{
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
}

void TEncSearch::init(	TEncCfg*			pcEncCfg,
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
	m_pcEncCfg						 = pcEncCfg;
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

  initTempBuff();

	m_pTempPel = new Pel[g_uiMaxCUWidth*g_uiMaxCUHeight];

  m_iDIFTap2 = (m_iDIFTap << 1);
}

#if FASTME_SMOOTHER_MV
#define FIRSTSEARCHSTOP			1
#else
#define FIRSTSEARCHSTOP			0
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

  // distortion
  uiSad = m_cDistParam.DistFunc( &m_cDistParam );

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

Void TEncSearch::xEncodeIntraTexture( TComDataCU* pcCU,
                                      TComPattern* pcPattern,
                                      TComYuv* pcOrgYuv,
                                      TComYuv*& rpcResiYuv,
                                      TComYuv*& rpcPredYuv,
                                      TComYuv*& rpcRecoYuv,
                                      UInt uiMode,
                                      UInt uiTU,
                                      UInt uiPU,
                                      UInt uiPartDepth,
                                      UInt uiPartOffset,
                                      UInt uiCoeffOffset,
                                      UInt uiWidth,
                                      UInt uiHeight )
{
  TCoeff* pCoeff = pcCU->getCoeffY()  + uiCoeffOffset;
  TCoeff* pCoef  = pCoeff;

  Pel* pResi = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
  Pel* pPred = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
  Pel* pReco = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
  UInt uiStride = rpcResiYuv->getStride();

  pcCU->clearCbf(uiPartOffset, TEXT_LUMA,     pcCU->getTotalNumPart()>>(uiPartDepth<<1));

  rpcResiYuv->subtractLuma( pcOrgYuv, rpcPredYuv, uiPU, uiWidth );

  UChar indexROT = pcCU->getROTindex(0);

  xRecurTransformNxNIntra( pcCU, uiPartOffset, pResi, pPred, pReco, 0, uiStride, uiWidth, uiHeight, uiTU, uiPartDepth, pCoef, TEXT_LUMA, indexROT );
  pcCU->setCuCbfLuma( uiPartOffset, uiTU, uiPartDepth );
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

    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, m_piYuvExt,m_iYuvExtStride,m_iYuvExtHeight,bAboveAvail,bLeftAvail);

    Int*   pPatChr;

    if (eText==TEXT_CHROMA_U)
      pPatChr=  pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_piYuvExt );
    else // (eText==TEXT_CHROMA_V)
      pPatChr=  pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_piYuvExt );

    predIntraChromaAdi( pcCU->getPattern(),pPatChr,uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );

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
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
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

Void TEncSearch::xRecurIntraLumaSearchADI( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiMode, UInt uiWidth, UInt uiHeight, UInt uiMaxDepth, UInt uiCurrDepth, Bool bAbove, Bool bLeft, Bool bSmallTrs)
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
    }
    else {
      bAboveAvail=  bAbove;
      bLeftAvail=bLeft;
    }
    predIntraLumaAdi( pcCU->getPattern(), uiMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );

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

    m_pcTrQuant->transformNxN( pcCU, piResi, uiStride, piCoeff, uiWidth, uiHeight, uiAbsSum, TEXT_LUMA, uiAbsPartIdx, indexROT );

    if ( uiAbsSum )
    {
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
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
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiWidth; pPred = piPred+uiWidth; pResi = piResi+uiWidth; pReco = piReco+uiWidth;
    piCoeff += uiCoeffOffset;
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false, true);
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset; pPred = piPred+uiPelOffset; pResi = piResi+uiPelOffset; pReco = piReco+uiPelOffset;
    piCoeff += uiCoeffOffset;
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false,true);
    uiAbsPartIdx += uiPartOffset;
    pOrg = piOrg+uiPelOffset+uiWidth; pPred = piPred+uiPelOffset+uiWidth; pResi = piResi+uiPelOffset+uiWidth; pReco = piReco+uiPelOffset+uiWidth;
    piCoeff += uiCoeffOffset;
    xRecurIntraLumaSearchADI( pcCU, uiAbsPartIdx, pOrg, pPred, pResi, pReco, uiStride, piCoeff, uiMode, uiWidth, uiHeight, uiMaxDepth, uiCurrDepth, false, false, true);
  }
}

Void TEncSearch::predIntraLumaAdiSearch( TComDataCU* pcCU, TComYuv* pcOrgYuv, TComYuv*& rpcPredYuv, TComYuv*& rpcResiYuv, TComYuv*& rpcRecoYuv )
{
  UInt   uiDepth        = pcCU->getDepth(0);
  UInt   uiNumPU        = pcCU->getNumPartInter();
  UInt   uiPartDepth    = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
  UInt   uiWidth        = pcCU->getWidth(0) >> uiPartDepth;
  UInt   uiHeight       = pcCU->getHeight(0)>> uiPartDepth;
  UInt   uiCoeffSize    = uiWidth*uiHeight;

  UInt   uiWidthBit;

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

  Int    iMindQp        = 0;
  Int    iMaxdQp        = 0;
  Int    idQp;
  Int    iBestdQp				= 0;

  UInt   uiPartOffset;
  UInt   uiCoeffOffset;

  Pel*    pOrg;
  Pel*    pResi;
  Pel*    pReco;
  Pel*    pPred;
  TCoeff* pCoeff;

  UInt    uiStride  = rpcPredYuv->getStride();

  TComPattern* pcPattern  = pcCU->getPattern();

  pcCU->setTrIdxSubParts( uiMaxTrDepth, 0, uiDepth );
  uiWidthBit = pcCU->getIntraSizeIdx(0);

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

      uiMaxMode          = g_aucIntraModeNum    [uiWidthBit];
      UInt uiMaxModeFast = g_aucIntraModeNumFast[uiWidthBit];

      pOrg     = pcOrgYuv->getLumaAddr  (uiPU, uiWidth);
      pResi    = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
      pReco    = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
      pPred    = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
      pCoeff   = pcCU->getCoeffY()  + uiCoeffOffset;

      dPUBestCost = MAX_DOUBLE;

      UInt uiBestSad    = MAX_UINT;
      UInt iBestPreMode = 0;

      for ( uiMode = uiMaxModeFast; uiMode < uiMaxMode; uiMode++ )
      {
        UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiMode];

        if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
          continue;

        predIntraLumaAdi( pcPattern, uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );

        Pel* piOrgY  = pcOrgYuv  ->getLumaAddr(uiPU, uiWidth);
        Pel* piPreY  = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
        UInt iStride = pcOrgYuv  ->getStride  ();

        // use hadamard transform here
        UInt uiSad   = m_pcRdCost->calcHAD( piOrgY, iStride, piPreY, iStride, uiWidth, uiHeight );
        if ( uiSad < uiBestSad )
        {
          uiBestSad		 = uiSad;
          iBestPreMode = uiMode;
        }
      }

      UInt uiRdModeList[10];
      UInt uiNewMaxMode;
      for (Int i=0;i<10;i++) uiRdModeList[i]=i;

      if (uiMaxModeFast>=uiMaxMode)
      {
        uiNewMaxMode=uiMaxMode;
      }
      else
      {
        uiNewMaxMode=uiMaxModeFast+1;
        uiRdModeList[uiMaxModeFast]=iBestPreMode;
      }

      bAboveAvail = false;
      bLeftAvail  = false;
      pcPattern->initPattern( pcCU, uiPartDepth, uiPartOffset );
      pcPattern->initAdiPattern(pcCU, uiPartOffset, uiPartDepth, m_piYuvExt, m_iYuvExtStride, m_iYuvExtHeight, bAboveAvail, bLeftAvail);

      for ( uiMode = uiMinMode; uiMode < uiNewMaxMode; uiMode++ )
      {
        UInt uiOrgMode = uiRdModeList[uiMode];
        UInt uiNewMode = g_aucIntraModeOrder[uiWidthBit][uiOrgMode];

        if ( ( g_aucIntraAvail[uiNewMode][0] && (!bAboveAvail) ) || ( g_aucIntraAvail[uiNewMode][1] && (!bLeftAvail) ) )
          continue;

        uiBits = 0;
        pcCU->setLumaIntraDirSubParts( uiOrgMode, uiPartOffset, uiPartDepth+uiDepth );

        if(m_bUseSBACRD)
        {
          if( uiPU )
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
          else
            m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }

        xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiOrgMode, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail, (uiMaxTrDepth>uiPartDepth)? 1:0);
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
          uiPUBestBits       = uiBits;
          uiPUBestDistortion = uiDistortion;
          dPUBestCost        = dCost;

          if( m_bUseSBACRD )
            m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[uiNextDepth][CI_TEMP_BEST] );
        }
      } // Mode loop

      uiPUMode[uiPU]   = uiPUBestMode;
      uiPUBits        += uiPUBestBits;
      uiPUDistortion  += uiPUBestDistortion;
      dPUCost         += dPUBestCost;

      pOrg   = pcOrgYuv->getLumaAddr(uiPU, uiWidth);
      pResi  = rpcResiYuv->getLumaAddr(uiPU, uiWidth);
      pPred  = rpcPredYuv->getLumaAddr(uiPU, uiWidth);
      pReco  = rpcRecoYuv->getLumaAddr(uiPU, uiWidth);
      pCoeff = pcCU->getCoeffY()  + uiCoeffOffset;

      pcCU->setLumaIntraDirSubParts( uiPUBestMode,    uiPartOffset, uiPartDepth+uiDepth );

      if(m_bUseSBACRD)
      {
        if( uiPU )
          m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiNextDepth][CI_NEXT_BEST]);
        else
          m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
      }

      xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiPUBestMode, uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0  );
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
    pcCU->setLumaIntraDirSubParts( uiBestMode[uiPU],    uiPartOffset, uiPartDepth+uiDepth );

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


    xRecurIntraLumaSearchADI( pcCU, uiPartOffset, pOrg, pPred, pResi, pReco, uiStride, pCoeff, uiBestMode[uiPU], uiWidth, uiHeight, uiMaxTrDepth, uiPartDepth, bAboveAvail,bLeftAvail,(uiMaxTrDepth>uiPartDepth)? 1:0);
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
  uiModeList[4]=g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir(0)];

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
  Int					  aaiMvpIdxBi[2][33];

  Int						aaiMvpIdx[2][33];
  Int						aaiMvpNum[2][33];

  AMVPInfo aacAMVPInfo[2][33];

  Int           iRefIdx[2];
  Int           iRefIdxBi[2];

  UInt          uiPartAddr;
  Int           iRoiWidth, iRoiHeight;

  UInt          uiMbBits[3] = {1, 1, 0};

  UInt          uiLastMode = 0;
	Int						iRefStart, iRefEnd;

  PartSize      ePartSize = pcCU->getPartitionSize( 0 );

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
				uiBitsTemp = uiMbBits[iRefList];
				if ( pcCU->getSlice()->getNumRefIdx(eRefPicList) > 1 )
				{
					uiBitsTemp += iRefIdxTemp+1;
					if ( iRefIdxTemp == pcCU->getSlice()->getNumRefIdx(eRefPicList)-1 ) uiBitsTemp--;
				}
				xEstimateMvPredAMVP( pcCU, pcOrgYuv, iPartIdx, eRefPicList, iRefIdxTemp, cMvPred[iRefList][iRefIdxTemp]);

        aaiMvpIdx[iRefList][iRefIdxTemp] = pcCU->getMVPIdx(eRefPicList, uiPartAddr);
        aaiMvpNum[iRefList][iRefIdxTemp] = pcCU->getMVPNum(eRefPicList, uiPartAddr);

        uiBitsTemp += m_auiMVPIdxCost[aaiMvpIdx[iRefList][iRefIdxTemp]][aaiMvpNum[iRefList][iRefIdxTemp]];

#if GPB_SIMPLE_UNI
				if ( pcCU->getSlice()->getSPS()->getUseLDC() )
				{
					if ( iRefList && iRefIdxTemp != iRefIdx[0] )
					{
						uiCostTemp = MAX_UINT;
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

        if ( pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
        {
          xCopyAMVPInfo(pcCU->getCUMvField(eRefPicList)->getAMVPInfo(), &aacAMVPInfo[iRefList][iRefIdxTemp]);
          xCheckBestMVP(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPred[iRefList][iRefIdxTemp], aaiMvpIdx[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
        }

        if ( uiCostTemp < uiCost[iRefList] )
        {
          uiCost[iRefList] = uiCostTemp;
          uiBits[iRefList] = uiBitsTemp; // storing for bi-prediction

          // set motion
          cMv[iRefList]     = cMvTemp[iRefList][iRefIdxTemp];
          iRefIdx[iRefList] = iRefIdxTemp;
          pcCU->getCUMvField(eRefPicList)->setAllMvField( cMv[iRefList], iRefIdx[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );

          // storing list 1 prediction signal for iterative bi-directional prediction
          if ( eRefPicList == REF_PIC_LIST_1 )
          {
            TComYuv*  pcYuvPred = &m_acYuvPred[iRefList];
            motionCompensation ( pcCU, pcYuvPred, eRefPicList, iPartIdx );
          }
        }
      }
    }

    //  Bi-directional prediction
    if ( pcCU->getSlice()->isInterB() )
    {
      cMvBi[0] = cMv[0];            cMvBi[1] = cMv[1];
      iRefIdxBi[0] = iRefIdx[0];    iRefIdxBi[1] = iRefIdx[1];

      ::memcpy(cMvPredBi, cMvPred, sizeof(cMvPred));
      ::memcpy(aaiMvpIdxBi, aaiMvpIdx, sizeof(aaiMvpIdx));

      UInt uiMotBits[2] = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
      uiBits[2] = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

      for ( Int iIter = 0; iIter < 4; iIter++ )
      {
        Int         iRefList    = iIter % 2;
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

				for ( Int iRefIdxTemp = iRefStart; iRefIdxTemp <= iRefEnd; iRefIdxTemp++ )
        {
					uiBitsTemp = uiMbBits[2] + uiMotBits[1-iRefList];
					if ( pcCU->getSlice()->getNumRefIdx(eRefPicList) > 1 )
					{
						uiBitsTemp += iRefIdxTemp+1;
						if ( iRefIdxTemp == pcCU->getSlice()->getNumRefIdx(eRefPicList)-1 ) uiBitsTemp--;
					}

					uiBitsTemp += m_auiMVPIdxCost[aaiMvpIdxBi[iRefList][iRefIdxTemp]][aaiMvpNum[iRefList][iRefIdxTemp]];

					// call ME
          xMotionEstimation ( pcCU, pcOrgYuv, iPartIdx, eRefPicList, &cMvPredBi[iRefList][iRefIdxTemp], iRefIdxTemp, cMvTemp[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp, true );

          if ( pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
          {
            xCopyAMVPInfo(&aacAMVPInfo[iRefList][iRefIdxTemp], pcCU->getCUMvField(eRefPicList)->getAMVPInfo());
            xCheckBestMVP(pcCU, eRefPicList, cMvTemp[iRefList][iRefIdxTemp], cMvPredBi[iRefList][iRefIdxTemp], aaiMvpIdxBi[iRefList][iRefIdxTemp], uiBitsTemp, uiCostTemp);
          }

          if ( uiCostTemp < uiCostBi )
          {
            bChanged = true;

            cMvBi[iRefList]     = cMvTemp[iRefList][iRefIdxTemp];
            iRefIdxBi[iRefList] = iRefIdxTemp;

            uiCostBi            = uiCostTemp;
            uiMotBits[iRefList] = uiBitsTemp - uiMbBits[2] - uiMotBits[1-iRefList];
            uiBits[2]           = uiBitsTemp;

            //  Set motion
            pcCU->getCUMvField( eRefPicList )->setAllMvField( cMvBi[iRefList], iRefIdxBi[iRefList], ePartSize, uiPartAddr, iPartIdx, 0 );

            TComYuv* pcYuvPred = &m_acYuvPred[iRefList];
            motionCompensation( pcCU, pcYuvPred, eRefPicList, iPartIdx );
          }
        } // for loop-iRefIdxTemp

        if ( !bChanged )
				{
					if ( uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1] && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL )
					{
						xCopyAMVPInfo(&aacAMVPInfo[0][iRefIdxBi[0]], pcCU->getCUMvField(REF_PIC_LIST_0)->getAMVPInfo());
						xCheckBestMVP(pcCU, REF_PIC_LIST_0, cMvBi[0], cMvPredBi[0][iRefIdxBi[0]], aaiMvpIdxBi[0][iRefIdxBi[0]], uiBits[2], uiCostBi);

						xCopyAMVPInfo(&aacAMVPInfo[1][iRefIdxBi[1]], pcCU->getCUMvField(REF_PIC_LIST_1)->getAMVPInfo());
						xCheckBestMVP(pcCU, REF_PIC_LIST_1, cMvBi[1], cMvPredBi[1][iRefIdxBi[1]], aaiMvpIdxBi[1][iRefIdxBi[1]], uiBits[2], uiCostBi);
					}
					break;
				}
      } // for loop-iter
    } // if (B_SLICE)

    //  Clear Motion Field
    pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
    pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMvZero, NOT_VALID, ePartSize, uiPartAddr, iPartIdx, 0 );
    pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( cMvZero,            ePartSize, uiPartAddr, iPartIdx, 0 );
    pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( cMvZero,            ePartSize, uiPartAddr, iPartIdx, 0 );

    pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));

    // Set Motion Field_
    if ( uiCostBi <= uiCost[0] && uiCostBi <= uiCost[1])
    {
      uiLastMode = 2;

      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMvBi[0], iRefIdxBi[0], ePartSize, uiPartAddr, iPartIdx, 0 );
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMvBi[1], iRefIdxBi[1], ePartSize, uiPartAddr, iPartIdx, 0 );

      TempMv = cMvBi[0] - cMvPredBi[0][iRefIdxBi[0]];
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
			if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdxBi[0]]))
      {
        aaiMvpIdxBi[0][iRefIdxBi[0]] = pcCU->searchMVPIdx(cMvPredBi[0][iRefIdxBi[0]], &aacAMVPInfo[0][iRefIdxBi[0]]);
        aaiMvpNum[0][iRefIdxBi[0]] = aacAMVPInfo[0][iRefIdxBi[0]].iN;
      }

      TempMv = cMvBi[1] - cMvPredBi[1][iRefIdxBi[1]];
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
			if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdxBi[1]]))
      {
        aaiMvpIdxBi[1][iRefIdxBi[1]] = pcCU->searchMVPIdx(cMvPredBi[1][iRefIdxBi[1]], &aacAMVPInfo[1][iRefIdxBi[1]]);
				aaiMvpNum[1][iRefIdxBi[1]] = aacAMVPInfo[1][iRefIdxBi[1]].iN;
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
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvField( cMv[0],   iRefIdx[0],   ePartSize, uiPartAddr, iPartIdx, 0 );
      TempMv = cMv[0] - cMvPred[0][iRefIdx[0]];
      pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
			if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[0][iRefIdx[0]]))
      {
        aaiMvpIdx[0][iRefIdx[0]] = pcCU->searchMVPIdx(cMvPred[0][iRefIdx[0]], &aacAMVPInfo[0][iRefIdx[0]]);
  		  aaiMvpNum[0][iRefIdx[0]] = aacAMVPInfo[0][iRefIdx[0]].iN;
      }

      pcCU->setInterDirSubParts( 1, uiPartAddr, iPartIdx, pcCU->getDepth(0) );

			pcCU->setMVPIdxSubParts( aaiMvpIdx[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[0][iRefIdx[0]], REF_PIC_LIST_0, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    }
    else
    {
      uiLastMode = 1;
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvField( cMv[1],   iRefIdx[1],   ePartSize, uiPartAddr, iPartIdx, 0 );
      TempMv = cMv[1] - cMvPred[1][iRefIdx[1]];
      pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMvd    ( TempMv,                 ePartSize, uiPartAddr, iPartIdx, 0 );
			if (pcCU->clearMVPCand(TempMv, &aacAMVPInfo[1][iRefIdx[1]]))
			{
				aaiMvpIdx[1][iRefIdx[1]] = pcCU->searchMVPIdx(cMvPred[1][iRefIdx[1]], &aacAMVPInfo[1][iRefIdx[1]]);
				aaiMvpNum[1][iRefIdx[1]] = aacAMVPInfo[1][iRefIdx[1]].iN;
			}

      pcCU->setInterDirSubParts( 2, uiPartAddr, iPartIdx, pcCU->getDepth(0) );

      pcCU->setMVPIdxSubParts( aaiMvpIdx[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( aaiMvpNum[1][iRefIdx[1]], REF_PIC_LIST_1, uiPartAddr, iPartIdx, pcCU->getDepth(uiPartAddr));
    }

    //  MC
    motionCompensation ( pcCU, rpcPredYuv, REF_PIC_LIST_X, iPartIdx );
  } //  end of for ( Int iPartIdx = 0; iPartIdx < iNumPart; iPartIdx++ )

  return;
}

// AMVP
Void TEncSearch::xEstimateMvPredAMVP( TComDataCU* pcCU, TComYuv* pcOrgYuv, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred, Bool bFilled )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();

  TComMv  cBestMv;
  Int     iBestIdx = 0;
  TComMv  cZeroMv;
  TComMv  cMvPred;
  UInt		uiBestCost = MAX_INT;
  UInt		uiPartAddr = 0;
  Int			iRoiWidth, iRoiHeight;
  Int			i;

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

Void TEncSearch::xCheckBestMVP ( TComDataCU* pcCU, RefPicList eRefPicList, TComMv cMv, TComMv& rcMvPred, Int& riMVPIdx, UInt& ruiBits, UInt& ruiCost )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();

  assert(pcAMVPInfo->m_acMvCand[riMVPIdx] == rcMvPred);
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

    m_pcRdCost->setPredictor( pcAMVPInfo->m_acMvCand[iMVPIdx] );
    Int iMvBits = m_pcRdCost->getBits(cMv.getHor(), cMv.getVer());

    if (iMvBits < iBestMvBits)
    {
      iBestMvBits = iMvBits;
      iBestMVPIdx = iMVPIdx;
    }
  }

  if (iBestMVPIdx != riMVPIdx)	//if changed
  {
    rcMvPred = pcAMVPInfo->m_acMvCand[iBestMVPIdx];

    iOrgMvBits  += m_auiMVPIdxCost[riMVPIdx][pcAMVPInfo->iN];
    iBestMvBits += m_auiMVPIdxCost[iBestMVPIdx][pcAMVPInfo->iN];

    riMVPIdx = iBestMVPIdx;
    UInt uiOrgBits = ruiBits;
    ruiBits = uiOrgBits - iOrgMvBits + iBestMvBits;
    ruiCost = ruiCost - m_pcRdCost->getCost( uiOrgBits )	+ m_pcRdCost->getCost( ruiBits );
  }
}

UInt TEncSearch::xGetTemplateCost( TComDataCU* pcCU,
                                        UInt				uiPartIdx,
                                        UInt			uiPartAddr,
                                        TComYuv*		pcOrgYuv,
                                        TComYuv*		pcTemplateCand,
                                        TComMv      cMvCand,
                                        Int					iMVPIdx,
                                        Int			iMVPNum,
                                        RefPicList  eRefPicList,
                                        Int         iRefIdx,
                                        Int					iSizeX,
                                        Int					iSizeY)
{
  UInt uiCost  = MAX_INT;

  TComPicYuv* pcPicYuvRef = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();

  // prediction pattern
  xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvCand, iSizeX, iSizeY, pcTemplateCand );

  // calc distortion
  uiCost = m_pcRdCost->getDistPart( pcTemplateCand->getLumaAddr(uiPartAddr), pcTemplateCand->getStride(), pcOrgYuv->getLumaAddr(uiPartAddr), pcOrgYuv->getStride(), iSizeX, iSizeY, DF_SAD );
  uiCost = (UInt) m_pcRdCost->calcRdCost( m_auiMVPIdxCost[iMVPIdx][iMVPNum], uiCost, false, DF_SAD );
  return uiCost;
}

Void TEncSearch::xMotionEstimation( TComDataCU* pcCU, TComYuv* pcYuvOrg, Int iPartIdx, RefPicList eRefPicList, TComMv* pcMvPred, Int iRefIdxPred, TComMv& rcMv, UInt& ruiBits, UInt& ruiCost, Bool bBi  )
{
  UInt          uiPartAddr;
  Int           iRoiWidth;
  Int           iRoiHeight;

  TComMv        cMvHalf, cMvQter;
  TComMv        cMvSrchRngLT;
  TComMv        cMvSrchRngRB;

  TComYuv*  pcYuv = pcYuvOrg;

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
    pcYuv->removeHighFreq( pcYuvOther, uiPartAddr, iRoiWidth, iRoiHeight );

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

  if ( bBi )  xSetSearchRange   ( pcCU, rcMv   , iSrchRng, cMvSrchRngLT, cMvSrchRngRB );
  else        xSetSearchRange   ( pcCU, cMvPred, iSrchRng, cMvSrchRngLT, cMvSrchRngRB );

  m_pcRdCost->getMotionCost	( 1, 0 );
  m_pcRdCost->setPredictor	( *pcMvPred );
  m_pcRdCost->setCostScale	( 2 );

  //  Do integer search
  if ( !m_iFastSearch || bBi )
  {
    xPatternSearch      ( pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
  }
  else
  {
    rcMv = *pcMvPred;
    xPatternSearchFast  ( pcCU, pcPatternKey, piRefY, iRefStride, &cMvSrchRngLT, &cMvSrchRngRB, rcMv, ruiCost );
  }

  m_pcRdCost->getMotionCost( 1, 0 );
  m_pcRdCost->setCostScale ( 1 );

  xPatternSearchFrac        ( pcCU, pcPatternKey, piRefY, iRefStride, &rcMv, cMvHalf, cMvQter, ruiCost );
  m_pcRdCost->setCostScale( 0 );

  rcMv <<= 2;
  rcMv += (cMvHalf <<= 1);
  rcMv +=  cMvQter;

  UInt uiMvBits = m_pcRdCost->getBits( rcMv.getHor(), rcMv.getVer() );

  ruiBits      += uiMvBits;
  ruiCost       = (UInt)( floor( fWeight * ( (Double)ruiCost - (Double)m_pcRdCost->getCost( uiMvBits ) ) ) + (Double)m_pcRdCost->getCost( ruiBits ) );
}

Void TEncSearch::xSetSearchRange ( TComDataCU* pcCU, TComMv& cMvPred, Int iSrchRng, TComMv& rcMvSrchRngLT, TComMv& rcMvSrchRngRB )
{
  pcCU->clipMv( cMvPred );

  rcMvSrchRngLT.setHor( cMvPred.getHor() - (iSrchRng << 2) );
  rcMvSrchRngLT.setVer( cMvPred.getVer() - (iSrchRng << 2) );

  rcMvSrchRngRB.setHor( cMvPred.getHor() + (iSrchRng << 2) );
  rcMvSrchRngRB.setVer( cMvPred.getVer() + (iSrchRng << 2) );

  pcCU->clipMv        ( rcMvSrchRngLT );
  pcCU->clipMv        ( rcMvSrchRngRB );

  rcMvSrchRngLT >>= 2;
  rcMvSrchRngRB >>= 2;
}

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
  rcMv >>= 2;

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
      cMv >>= 2;
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

Void TEncSearch::xPatternSearchFrac( TComDataCU* pcCU, TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, TComMv* pcMvInt, TComMv& rcMvHalf, TComMv& rcMvQter, UInt& ruiCost )
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
  Pel*	piRef;
  iRefStride	= m_cYuvExt.getStride();

  //	Half-pel refinement
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
}

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

    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
		pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
		pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cZeroMv, NOT_VALID, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
		pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary

		pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, 0, 0, pcCU->getDepth(0));
		pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, 0, 0, pcCU->getDepth(0));

    //  Motion compensation
    motionCompensation ( pcCU, rpcPredYuv, REF_PIC_LIST_0 );
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
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 ); //unnecessary
    }

    if (pcCU->getInterDir(0)!=1)
    {
      xEstimateMvPredAMVP( pcCU, pcOrgYuv, 0, REF_PIC_LIST_1, 0, cMv, (pcCU->getCUMvField( REF_PIC_LIST_1 )->getAMVPInfo()->iN > 0?  true:false) );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMv, 0, SIZE_2Nx2N, 0, 0, 0 );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, 0, 0, 0 );  //unnecessary
    }

    motionCompensation ( pcCU, rpcPredYuv );
  }
  else
  {
    assert( 0 );
  }

  return;
}

Void TEncSearch::encodeResAndCalcRdInterCU( TComDataCU* pcCU, TComYuv* pcYuvOrg, TComYuv* pcYuvPred, TComYuv*& rpcYuvResi, TComYuv*& rpcYuvRec, Bool bSkipRes )
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

  for ( uiQp = uiQpMin; uiQp <= uiQpMax; uiQp++ )
  {
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
  }

  assert ( dCostBest != MAX_DOUBLE );

  rpcYuvResi->subtract      ( pcYuvOrg, pcYuvPred, 0, uiWidth );

	if( m_bUseSBACRD )
	{
    m_pcRDGoOnSbacCoder->load( m_pppcRDSbacCoder[pcCU->getDepth(0)][CI_CURR_BEST] );
	}

  xEncodeInterTexture( pcCU, uiQpBest, bHighPass, rpcYuvResi, uiBestTrMode );

  if( (pcCU->getCbf(0, TEXT_LUMA, 0) == 0) &&
      (pcCU->getCbf(0, TEXT_CHROMA_U, 0) == 0) &&
      (pcCU->getCbf(0, TEXT_CHROMA_V, 0) == 0) )
      uiBestTrMode = 0;

  rpcYuvRec->addClip ( pcYuvPred,                 rpcYuvResi, 0,      uiWidth  );

  pcCU->getTotalBits()       = uiBitsBest;
  pcCU->getTotalDistortion() = uiDistortionBest;
  pcCU->getTotalCost()       = dCostBest;

  if ( pcCU->isSkipped(0) )
  {
    uiBestTrMode = 0;
    pcCU->setCbfSubParts( 0, 0, 0, 0, pcCU->getDepth( 0 ) );
  }

  pcCU->setTrIdxSubParts( uiBestTrMode, 0, pcCU->getDepth(0) );
  pcCU->setQPSubParts( uiQpBest, 0, pcCU->getDepth(0) );
}

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

Void TEncSearch::xRecurTransformNxNIntra( TComDataCU* rpcCU, UInt uiAbsPartIdx, Pel* pcResidual, Pel* pcPrediction, Pel* piReconstruction, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff*& rpcCoeff, TextType eType, Int indexROT )
{
  if ( uiTrMode == uiMaxTrMode )
  {
    UInt uiX, uiY;
    UInt uiAbsSum = 0;
    Pel* piResi = pcResidual       + uiAddr;
    Pel* piPred = pcPrediction     + uiAddr;
    Pel* piReco = piReconstruction + uiAddr;
    Pel* pResi  = piResi;
    UInt uiCoeffOffset = uiWidth*uiHeight;

    UInt uiZorder    = rpcCU->getZorderIdxInCU()+uiAbsPartIdx;
    Pel* pPicReco    = rpcCU->getPic()->getPicYuvRec()->getLumaAddr(rpcCU->getAddr(), uiZorder);
    UInt uiPicStride = rpcCU->getPic()->getPicYuvRec()->getStride();

    if (m_pcEncCfg->getUseRDOQ())
      m_pcEntropyCoder->estimateBit(m_pcTrQuant->m_pcEstBitsSbac, uiWidth, eType );

    m_pcTrQuant->transformNxN( rpcCU, piResi, uiStride, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eType, uiAbsPartIdx, indexROT );

    if ( uiAbsSum )
    {
      m_pcTrQuant->invtransformNxN( pResi, uiStride, rpcCoeff, uiWidth, uiHeight, indexROT );
    }
    else
    {
      memset(rpcCoeff, 0, sizeof(TCoeff)*uiCoeffOffset);
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        memset(pResi, 0, sizeof(Pel)*uiWidth);
        pResi += uiStride;
      }
    }

    m_pcEntropyCoder->encodeCoeffNxN( rpcCU, rpcCoeff, uiAbsPartIdx, uiWidth, uiHeight, rpcCU->getDepth( 0 ) + uiTrMode, eType, true );

    pResi = piResi;

    // Reconstruction
    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        piReco  [uiX] = Clip(piPred[uiX] + pResi[uiX]);
        pPicReco[uiX] = piReco[uiX];
      }
      piReco   += uiStride;
      pResi    += uiStride;
      piPred   += uiStride;
      pPicReco += uiPicStride;
    }

    rpcCoeff += uiCoeffOffset;
  }
  else
  {
    uiTrMode++;
    uiWidth  = uiWidth  >> 1;
    uiHeight = uiHeight >> 1;
    UInt uiQPartNum    = rpcCU->getPic()->getNumPartInCU() >> ( ( rpcCU->getDepth(0)+uiTrMode ) << 1 );
    xRecurTransformNxNIntra( rpcCU, uiAbsPartIdx, pcResidual, pcPrediction, piReconstruction, uiAddr                                , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxNIntra( rpcCU, uiAbsPartIdx, pcResidual, pcPrediction, piReconstruction, uiAddr + uiWidth                      , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxNIntra( rpcCU, uiAbsPartIdx, pcResidual, pcPrediction, piReconstruction, uiAddr + uiHeight * uiStride          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
    uiAbsPartIdx += uiQPartNum;
    xRecurTransformNxNIntra( rpcCU, uiAbsPartIdx, pcResidual, pcPrediction, piReconstruction, uiAddr + uiHeight * uiStride + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, eType, indexROT );
  }
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

        m_pcTrQuant->invtransformNxN( pcResidualRec, uiStride, rpcCoeff, uiWidth, uiHeight, indexROT);

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

Void  TEncSearch::xAddSymbolBitsIntra( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiPU, UInt uiQNumPart, UInt uiPartDepth, UInt uiNumPart, UInt uiMaxTrDepth, UInt uiTrDepth, UInt uiWidth, UInt uiHeight, UInt& ruiBits )
{
  UInt uiPartOffset = uiPU*uiQNumPart;
  m_pcEntropyCoder->resetBits();

  if( uiPU==0 )
  {
    if( !pcCU->getSlice()->isIntra() )
    {
      m_pcEntropyCoder->encodeSkipFlag( pcCU, 0, true );
      m_pcEntropyCoder->encodePredMode( pcCU, 0, true );
    }

    //BugFix for 1603
    m_pcEntropyCoder->encodePartSize( pcCU, 0, pcCU->getDepth(0), true );

    for( UInt ui = 0; ui < uiNumPart; ui++ )
      m_pcEntropyCoder->encodeIntraDirModeLuma( pcCU, (uiPU+ui)*uiQNumPart );
  }
  else
  {
    for( UInt ui = 0; ui < uiNumPart; ui++ )
      m_pcEntropyCoder->encodeIntraDirModeLuma( pcCU, (uiPU+ui)*uiQNumPart );
  }

  m_pcEntropyCoder->encodeCbf( pcCU, uiPartOffset, TEXT_LUMA, uiTrDepth );
  m_pcEntropyCoder->encodeCoeff( pcCU, pCoeff, uiPartOffset, pcCU->getDepth(0)+uiPartDepth, uiWidth, uiHeight, uiMaxTrDepth, uiTrDepth, TEXT_LUMA );
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
    ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  else
  {
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSkipFlag( pcCU, 0, true );
    m_pcEntropyCoder->encodePredMode( pcCU, 0, true );
    m_pcEntropyCoder->encodePartSize( pcCU, 0, pcCU->getDepth(0), true );
    m_pcEntropyCoder->encodePredInfo( pcCU, 0, true );
    m_pcEntropyCoder->encodeCoeff		( pcCU, 0, pcCU->getDepth(0), pcCU->getWidth(0), pcCU->getHeight(0) );

    // ROT index
    m_pcEntropyCoder->encodeROTindex( pcCU, 0, pcCU->getDepth(0) );

    ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  }
}

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

Void TEncSearch::xExtDIFUpSamplingQ 	( TComPattern* pcPatternKey, Pel* piDst, Int iDstStride, Pel* piSrcPel, Int iSrcPelStride, Int* piSrc, Int iSrcStride, UInt uiFilter )
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

Void TEncSearch::xPredIntraLumaNxNCIPEnc( TComPattern* pcTComPattern, Pel* pOrig, Pel* pPredCL, UInt uiStride, Pel* pPred, UInt uiPredStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAboveAvail, Bool bLeftAvail )
{
  Int*	ptrSrc;
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
