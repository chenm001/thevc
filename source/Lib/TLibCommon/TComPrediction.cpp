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

/** \file			TComPrediction.cpp
    \brief		prediction class
*/

#include <memory.h>
#include "TComPrediction.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define CCCP_DOWN_FILTER_TAP				2						///< downsampling filter tap for luma to chroma size conversion
#define CCCP_SEG_ITE_NUM            1						///< iteration number for segmentation
#define CCCP_UPDATE_ONLINE          1						///< usage of updated prediction pixels for next pixel prediction

// ====================================================================================================================
// Constructor / destructor / initialize
// ====================================================================================================================

TComPrediction::TComPrediction()
{
  m_piYuvExt = NULL;

  m_puiComponentCost        = NULL;
  m_puiComponentCostMem        = NULL;
  m_iMvPredHor = 0;
  m_iMvPredVer = 0;
  m_uiLambda = 0;
  m_uiMaxTemplateSearchRange	= 0;
  m_dLastQp = 0;

  for( Int n = 0; n < AMVP_MAX_NUM_CANDS; n++)
  {
    m_auiMVPIdxCost[n] = xGetSymbolBits( n );
  }

// CCCP
  Int iExt = CCCP_BOUNDARY_EXT << 1;
  m_iSegStride = ( g_uiMaxCUWidth + iExt ) >> 1;
  m_iLumaRecStride = ( g_uiMaxCUWidth + iExt );

  m_pLumaRecBuffer        = new Pel  [ m_iLumaRecStride * m_iLumaRecStride];
  m_pHorDsLumaRecBuffer   = new Pel  [ m_iLumaRecStride * m_iLumaRecStride >> 1 ];
  m_pDsLumaRecBuffer      = new Pel  [ m_iLumaRecStride * m_iLumaRecStride >> 2 ];
  m_pSegBuffer            = new Pel  [ m_iSegStride * m_iSegStride ];

  m_bHAM = false;
}

TComPrediction::~TComPrediction()
{
  m_cYuvExt.destroy();

	delete[] m_piYuvExt;

  m_acYuvPred[0].destroy();
  m_acYuvPred[1].destroy();

  m_acYuvPredTemp.destroy();

  m_cTemplate.destroy();
  m_cTemplateCand.destroy();

  xUninitRateDistortionModel();

	// CCCP
  if( m_pSegBuffer )
    delete [] m_pSegBuffer;

  if (m_pLumaRecBuffer)
    delete [] m_pLumaRecBuffer;

  if( m_pHorDsLumaRecBuffer )
    delete [] m_pHorDsLumaRecBuffer;

  if( m_pDsLumaRecBuffer )
    delete [] m_pDsLumaRecBuffer;
}

Void TComPrediction::initTempBuff()
{
  if( m_piYuvExt == NULL )
  {
    m_iYuvExtHeight  = ((g_uiMaxCUHeight + 2) << 4);
    m_iYuvExtStride = ((g_uiMaxCUWidth  + 12) << 4);

		m_cYuvExt.create( m_iYuvExtStride, m_iYuvExtHeight );
    m_piYuvExt = new Int[ m_iYuvExtStride * m_iYuvExtHeight ];

    // new structure
    m_acYuvPred[0] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );
    m_acYuvPred[1] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );

    m_acYuvPredTemp.create( g_uiMaxCUWidth, g_uiMaxCUHeight );
  }

  if (m_cTemplate.getLumaAddr() == NULL)
  {
    m_cTemplate.create( g_uiMaxCUWidth + AMVP_MAX_TEMP_SIZE, g_uiMaxCUHeight + AMVP_MAX_TEMP_SIZE );
  }

  if (m_cTemplateCand.getLumaAddr() == NULL)
  {
    m_cTemplateCand.create( g_uiMaxCUWidth + AMVP_MAX_TEMP_SIZE, g_uiMaxCUHeight + AMVP_MAX_TEMP_SIZE );
  }

  m_iDIFHalfTap		= ( m_iDIFTap  >> 1 );
	m_iDIFHalfTapC	= ( m_iDIFTapC >> 1 );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComPrediction::initRateDistortionModel( Double dQp, UInt uiSearchLimit )
{
  if( uiSearchLimit != m_uiMaxTemplateSearchRange )
  {
    xUninitRateDistortionModel();

    m_uiMaxTemplateSearchRange = uiSearchLimit;

    Int iHalfSearchLimit = (uiSearchLimit+1)>>1;

    m_puiComponentCostMem = new UInt[iHalfSearchLimit*2];

    m_puiComponentCost = m_puiComponentCostMem + iHalfSearchLimit;

    for( Int n = -iHalfSearchLimit; n < iHalfSearchLimit; n++)
    {
      m_puiComponentCost[n] = xGetComponentBits( n );
    }
  }

  if ( dQp != m_dLastQp )
  {
    m_dLastQp = dQp;
    Double dLambda = 0.85 * pow( 2.0, Min( 52.0, dQp ) / 3.0 - 4.0 );
//SAD    m_uiLambda = (UInt)floor(65536.0 * sqrt( dLambda ));
    m_uiLambda = (UInt)floor(dLambda + 0.5);
  }
}

Void TComPrediction::xUninitRateDistortionModel()
{
  if( NULL != m_puiComponentCostMem )
  {
    delete [] m_puiComponentCostMem;
    m_puiComponentCostMem = NULL;
  }
}

UInt TComPrediction::xGetComponentBits( Int iVal )
{
  iVal = ( iVal <= 0) ? -iVal<<1 : (iVal<<1)-1;
  UInt uiLength = 1;
  UInt uiTemp = ++iVal;

  assert ( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  return uiLength;
}

UInt TComPrediction::xGetSymbolBits( Int iVal )
{
  UInt uiBits = 0;
  Int iNum = AMVP_MAX_NUM_CANDS;
  if ( iNum > 1 )
  {
    if ( iVal == 0 )
    {
      uiBits = 1;
    }
    else
    {
      uiBits = 1;

      for (int i = 0; i < (iVal-1); i++)
      {
        uiBits += 1;
      }
      if ( ( iVal+1 ) != iNum )
      {
        uiBits += 1;
      }
    }
  }

  return uiBits;
}

UInt  TComPrediction::xCalcRdCost( UInt uiBits, UInt uiDistortion )
{
  Double dRdCost = 0.0;
  Double dLambda = m_uiLambda;
  dRdCost = ((Double)uiDistortion + (Double)((Int)(uiBits * dLambda+.5)>>16));
  return (UInt)floor(dRdCost);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// new structure
Void TComPrediction::predIntraLumaNxN( TComPattern* pcTComPattern, UInt iPredMode, Pel*& rpPred, UInt uiStride, UInt iWidth, UInt iHeight, UChar indexMPI )
{
  Pel* pDst = rpPred;
  Int  iDstStride = uiStride;

  if ( iWidth > 8 || iWidth < 4 )  // Intra prediction with 4 directions for large block (>8)
  {
    Pel* pSrc = pcTComPattern->getROIY();
    Int  iSrcStride = pcTComPattern->getPatternLStride();

    switch ( iPredMode )
    {
    case 0: // INTRA_VERT
      xPredIntraVert( pSrc, iSrcStride, pDst, iDstStride, iWidth, iHeight );
      break;
    case 1: // INTRA_HOR
      xPredIntraHor( pSrc, iSrcStride, pDst, iDstStride, iWidth, iHeight );
      break;
    case 2: // INTRA_DC
      xPredIntraDC( pcTComPattern, pSrc, iSrcStride, pDst, iDstStride, iWidth, iHeight );
      break;
    case 3: // INTRA_Plane
      xPredIntraPlane( pSrc, iSrcStride, pDst, iDstStride, iWidth, iHeight );
      break;
      // TM_INTRA
	case 33:  //TM
    {
	  Pel* pSrcRescuer = pcTComPattern->getROIY();
	  Int  iSrcStride = pcTComPattern->getPatternLStride();
	  if (iWidth<16)  pSrcRescuer = pcTComPattern->getROIYBlk( 0 );

	  if (pcTComPattern->isAboveAvailable() || pcTComPattern->isLeftAvailable())
    	  xPredIntraTM ( pSrcRescuer, iSrcStride, pDst, iDstStride, iWidth, iHeight, pcTComPattern );
	  else
		xPredIntraDC( pcTComPattern, pSrc, iSrcStride, pDst, iDstStride, iWidth, iHeight );
	  break;
    }
    }

		if ( m_bUseMPI )
		{
			if( pcTComPattern->isAllAboveLeftAvailable() )
			{
        xMPI_POST_PROCESSING( pSrc, iSrcStride, rpPred, uiStride, iWidth, iHeight, indexMPI);
			}
		}
  }
  else if ( iWidth == 8 )      // Intra prediction with 9 directions for 8x8 (<8)
  {
    switch ( iPredMode )
    {
    case 0: // INTRA_VERT
      xPredIntra8Vert( pcTComPattern, pDst, iDstStride );
      break;
    case 1: // INTRA_HOR
      xPredIntra8Hor( pcTComPattern, pDst, iDstStride );
      break;
    case 2: // INTRA_DC
      xPredIntra8DC( pcTComPattern, pDst, iDstStride );
      break;
    case 3: // INTRA_DIAG_DOWN_LEFT
      xPredIntra8DiagDownLeft( pcTComPattern, pDst, iDstStride );
      break;
    case 4: // INTRA_DIAG_DOWN_RIGHT
      xPredIntra8DiagDownRight( pcTComPattern, pDst, iDstStride );
      break;
    case 5: // INTRA_VERT_RIGHT
      xPredIntra8VertRight( pcTComPattern, pDst, iDstStride );
      break;
    case 6: // INTRA_HOR_DOWN
      xPredIntra8HorDown( pcTComPattern, pDst, iDstStride );
      break;
    case 7: // INTRA_VERT_LEFT
      xPredIntra8VertLeft( pcTComPattern, pDst, iDstStride );
      break;
    case 8: // INTRA_HOR_UP
      xPredIntra8HorUp( pcTComPattern, pDst, iDstStride );
      break;
      // TM_INTRA
	case 33:  //TM
    {
	  Pel* pSrcRescuer = pcTComPattern->getROIY();
	  Int  iSrcStride = pcTComPattern->getPatternLStride();
	  if (iWidth<16)  pSrcRescuer = pcTComPattern->getROIYBlk( 0 );

	  if (pcTComPattern->isAboveAvailable() || pcTComPattern->isLeftAvailable())
    	  xPredIntraTM ( pSrcRescuer, iSrcStride, pDst, iDstStride, iWidth, iHeight, pcTComPattern );
	  else
		  xPredIntra8DC( pcTComPattern, pDst, iDstStride );
	  break;
    }
    }

		if ( m_bUseMPI )
		{
			if(pcTComPattern->isAllAboveLeftAvailable()   )
			{
				Pel* pSrcRescuer = pcTComPattern->getROIYBlk( 0 );
				Int  iSrcStride  = pcTComPattern->getPatternLStride();
				Int  iDstStride  = uiStride;
				Pel* pDstRescuer = rpPred;

				xMPI_POST_PROCESSING( pSrcRescuer, iSrcStride, pDstRescuer, iDstStride, iWidth, iHeight, indexMPI );
			}
		}
  }
  else                         // Intra prediction with 9 directions for 4x4
  {
    switch ( iPredMode )
    {
    case 0: // INTRA_VERT
      xPredIntra4Vert( pcTComPattern, pDst, iDstStride );
      break;
    case 1: // INTRA_HOR
      xPredIntra4Hor( pcTComPattern, pDst, iDstStride );
      break;
    case 2: // INTRA_DC
      xPredIntra4DC( pcTComPattern, pDst, iDstStride );
      break;
    case 3: // INTRA_DIAG_DOWN_LEFT
      xPredIntra4DiagDownLeft( pcTComPattern, pDst, iDstStride );
      break;
    case 4: // INTRA_DIAG_DOWN_RIGHT
      xPredIntra4DiagDownRight( pcTComPattern, pDst, iDstStride );
      break;
    case 5: // INTRA_VERT_RIGHT
      xPredIntra4VertRight( pcTComPattern, pDst, iDstStride );
      break;
    case 6: // INTRA_HOR_DOWN
      xPredIntra4HorDown( pcTComPattern, pDst, iDstStride );
      break;
    case 7: // INTRA_VERT_LEFT
      xPredIntra4VertLeft( pcTComPattern, pDst, iDstStride );
      break;
    case 8: // INTRA_HOR_UP
      xPredIntra4HorUp( pcTComPattern, pDst, iDstStride );
      break;
      // TM_INTRA
	case 33:  //TM
    {
	  Pel* pSrcRescuer = pcTComPattern->getROIY();
	  Int  iSrcStride = pcTComPattern->getPatternLStride();
	  if (iWidth<16)  pSrcRescuer = pcTComPattern->getROIYBlk( 0 );

	  if (pcTComPattern->isAboveAvailable() || pcTComPattern->isLeftAvailable())
    	  xPredIntraTM ( pSrcRescuer, iSrcStride, pDst, iDstStride, iWidth, iHeight, pcTComPattern );
	  else
		  xPredIntra4DC( pcTComPattern, pDst, iDstStride );
	  break;
    }
    }

		if ( m_bUseMPI )
		{
			if( pcTComPattern->isAllAboveLeftAvailable()   )
			{
				Pel* pSrcRescuer = pcTComPattern->getROIY();
				Int  iSrcStride  = pcTComPattern->getPatternLStride();
				Int  iDstStride  = uiStride;
				Pel* pDstRescuer = rpPred;

				xMPI_POST_PROCESSING( pSrcRescuer, iSrcStride, pDstRescuer, iDstStride, iWidth, iHeight,indexMPI );
			}
		}
  }
}
Bool TComPrediction::checkIntraLumaAvailableADI( TComDataCU* pcCU, UInt uiDirMode, UInt uiWidth, Bool bAbove, Bool bLeft )
{
  UInt ui = uiWidth;
  Int	 iIntraSizeIdx = 0;
  Bool bDirFlag = true;

  while( true )
  {
    if( ui & 0x1 )
    {
      break;
    }

    ui >>= 1;
    iIntraSizeIdx++;
  }
  iIntraSizeIdx--;

  assert( iIntraSizeIdx >= 0 );

  if( iIntraSizeIdx > 6 )
    return false;

  uiDirMode = g_aucIntraModeOrder[ iIntraSizeIdx ][ uiDirMode ];

  switch( uiDirMode )
  {
  case 0:  // Vertical
      return true;
  case 1:  // Horizontal
      return true;
  case 2:  // DC
      return true;
  case 3:  // Plane
      return true;
  case 32:  // bi
      return true;
// TM_INTRA
  case 33:  // TM
      return true;
  }

  if( bDirFlag )
  {
    return true;
  }

  return false;
}
Bool TComPrediction::checkIntraLumaAvailable( TComPattern* pcTComPattern, UInt iPredMode, UInt iWidth, UInt iHeight )
{
  if ( iWidth > 8 || iWidth < 4 )  // Intra prediction with 4 directions for large block (>8)
  {
    switch ( iPredMode )
    {
    case 0: // INTRA_VERT
      if ( pcTComPattern->isAboveAvailable() )
        return true;
      break;
    case 1: // INTRA_HOR
      if ( pcTComPattern->isLeftAvailable() )
        return true;
      break;
    case 2: // INTRA_DC
      return true;
    case 3: // INTRA_Plane
      if ( pcTComPattern->isAllAboveLeftAvailable() )
        return true;
      break;
    }
  }
  else
  {
    switch ( iPredMode )
    {
    case 0: // INTRA_VERT
      if ( pcTComPattern->isAboveAvailable() )
        return true;
      break;
    case 1: // INTRA_HOR
      if ( pcTComPattern->isLeftAvailable() )
        return true;
      break;
    case 2: // INTRA_DC
      return true;
      break;
    case 3: // INTRA_DIAG_DOWN_LEFT
      if ( pcTComPattern->isAboveAvailable() )
        return true;
      break;
    case 4: // INTRA_DIAG_DOWN_RIGHT
      if ( pcTComPattern->isAllAboveLeftAvailable() )
        return true;
      break;
    case 5: // INTRA_VERT_RIGHT
      if ( pcTComPattern->isAllAboveLeftAvailable() )
        return true;
      break;
    case 6: // INTRA_HOR_DOWN
      if ( pcTComPattern->isAllAboveLeftAvailable() )
        return true;
      break;
    case 7: // INTRA_VERT_LEFT
      if ( pcTComPattern->isAboveAvailable() )
        return true;
      break;
    case 8: // INTRA_HOR_UP
      if ( pcTComPattern->isLeftAvailable() )
        return true;
      break;
    }
  }
  return false;
}

Void TComPrediction::predIntraChromaNxN( TComPattern* pcTComPattern, Pel* piSrc, UInt uiSrcStride, UInt iPredMode, Pel* piPred, Int  iDstStride, UInt iWidth, UInt iHeight )
{
  Pel* pDst       = piPred;
  Pel* pSrc       = piSrc;

  switch ( iPredMode )
  {
  case 0: // INTRA_Chroma_DC
    xPredIntraDC( pcTComPattern, pSrc, uiSrcStride, pDst, iDstStride, iWidth, iHeight );
    break;
  case 1: // INTRA_Chroma_Hor
    xPredIntraHor( pSrc, uiSrcStride, pDst, iDstStride, iWidth, iHeight );
    break;
  case 2: // INTRA_Chroma_Ver
    xPredIntraVert( pSrc, uiSrcStride, pDst, iDstStride, iWidth, iHeight );
    break;
  case 3: // INTRA_Chroma_PLANE
    xPredIntraPlane( pSrc, uiSrcStride, pDst, iDstStride, iWidth, iHeight );
    break;
  }
}

Bool TComPrediction::CheckIntraChromaNxN( TComPattern* pcTComPattern, UInt iPredMode )
{
  switch ( iPredMode )
  {
  case 0: // INTRA_Chroma_DC
    return true;
  case 1: // INTRA_Chroma_Hor
    if ( pcTComPattern->isLeftAvailable() )
      return true;
    break;
  case 2: // INTRA_Chroma_Ver
    if ( pcTComPattern->isAboveAvailable() )
      return true;
    break;
  case 3: // INTRA_Chroma_PLANE
    if ( pcTComPattern->isAllAboveLeftAvailable() )
      return true;
    break;
  }
  return false;
}


//ADI_CHROMA

Void TComPrediction::predIntraChromaAdi(TComPattern* pcTComPattern,Int* piSrc, UInt uiDirMode, Pel* piPred, Int  iPredStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft ,Pel* piSrcOld, UInt uiSrcStrideOld )
{
  Bool  ret        = false;
  Bool  bDirFlag   = true;
  Int   iDstStride = iPredStride;
  Int   i, j;
  Pel  *pDst     ;
  Pel  *ptrDst   ;
  Int   sx, sy, sw;
  UInt  uixyflag   = 0; // 0 for x axis 1 for y axis
  Int   iDx, iDy, iXx, iYx, iXy, iYy, iX, iY, iTempDx, iTempDy;
  Int   iXn, iYn;
  Bool  bCenter;


  Int* ptrSrc;
  Int* ptrQSrc;

  Int	iIntraSizeIdx;

  UInt ui = iWidth;
  iIntraSizeIdx = 0;

  while( true )
  {
    if( ui & 0x1 )
    {
      break;
    }

    ui >>= 1;
    iIntraSizeIdx++;
  }
  iIntraSizeIdx--;

  assert( iIntraSizeIdx >= 0 );

  if( ( iIntraSizeIdx < 0 ) )
  {
    return;
  }

  UInt dirmodeold=uiDirMode;

  if( iIntraSizeIdx > 6 )
    return;

  pDst = piPred;
  ptrSrc = piSrc;


  ptrDst   = pDst;
  ptrQSrc = ptrSrc;

  sy = 1;
  sx = 1;
  sw = 2*iWidth+1;

  ptrQSrc += sw*(sy)+sx;

  for(i=0;i<iHeight; i++)
	{
    // ::memset( ptrQSrc, 0x00, iWidth*sizeof(Short) );
    ::memset( ptrDst,  0x00, iWidth*sizeof(Short) );

    ptrQSrc += sw;
    ptrDst  += iDstStride;
	}

  iDx = 1;
  iDy = 1;

  ptrDst = pDst;
  switch( uiDirMode )
  {
  case 0:  // Vertical
    {
      xPredIntraVertAdi( ptrSrc+sw+1, sw, ptrDst, iDstStride, iWidth, iHeight );
      bDirFlag = false;
      ret      = true;
      break;
    }
  case 1:  // Horizontal
    {

      xPredIntraHorAdi( ptrSrc+sw+1, sw, ptrDst, iDstStride, iWidth, iHeight );

      bDirFlag = false;
      ret      = true;
      break;
    }
  case 2:  // DC
    {
      xPredIntraDCAdi( ptrSrc+sw+1, sw, ptrDst, iDstStride, iWidth, iHeight, bAbove,  bLeft );
      bDirFlag = false;
      ret      = true;
      break;
    }
  case 3:  // Plane
    {
      xPredIntraPlaneAdi( ptrSrc+sw+1,sw, ptrDst, iDstStride, iWidth, iHeight );
      bDirFlag = false;
      ret      = true;
      break;
    }
// TM_INTRA
	case 33:  //TM
    {
	  if (pcTComPattern->isAboveAvailable() || pcTComPattern->isLeftAvailable())
    	  xPredIntraTM ( piSrcOld, uiSrcStrideOld, ptrDst, iDstStride, iWidth, iHeight, pcTComPattern );
	  else
		xPredIntraDCAdi( ptrSrc+sw+1, sw, ptrDst, iDstStride, iWidth, iHeight, bAbove,  bLeft );

	  bDirFlag = false;
	  break;
    }
  case 32:  // bi
    {
      xPredIntraBiAdi ( ptrSrc+sw+1, sw, ptrDst, iDstStride, iWidth, iHeight );
      bDirFlag = false;
      ret      = true;
      break;
    }
  default:
    {
      iDx      = g_aucDirDx[ uiDirMode-4 ];
      iDy      = g_aucDirDy[ uiDirMode-4 ];
      uixyflag = g_aucXYflg[ uiDirMode   ];
      break;
    }
  }


  if( !bDirFlag )
    return;


  if( bDirFlag )
  {
    ret=true;
    for( i = 0; i < iHeight; i++ )
    {
      for( j = 0; j < iWidth; j++ )
      {
        Bool bSetFlag1=false;
        Bool bSetFlag2=false;

        if ( ( (i+iDy) >= 0 ) && ( (i+iDy) < iHeight ) && ( (j+iDx) >= 0 ) && ( (j+iDx) < iWidth ) )
          if ( pDst[ (i+iDy)*iDstStride+(j+iDx) ] > 0 )
            bSetFlag1=true;

        if( !bSetFlag1 )
          if ( ( (i-iDy) >= 0 ) && ( (i-iDy) < iHeight ) && ( (j-iDx) >= 0 ) && ( (j-iDx) < iWidth ) )
            if (pDst[ (i-iDy)*iDstStride+(j-iDx) ]>0)
              bSetFlag2=true;

        if( bSetFlag1 )
        {
          pDst[ i*iDstStride+j ] = pDst[ (i+iDy)*iDstStride+(j+iDx) ];
        }
        else if(bSetFlag2)
        {
          pDst[ i*iDstStride+j ] = pDst[ (i-iDy)*iDstStride+(j-iDx) ];
        }
        else
        {
          if ( iDx*iDy < 0 )   // select context pixel based on uixyflag
          {
            if ( uixyflag == 0 )
            {
              iTempDy = i-(-1);

              // case xline
              iTempDx =getContextPixel(uiDirMode,0,iTempDy, bCenter);

              iX      = j+iTempDx;
              iY      = -1;
            }
            else
            {
              iTempDx = j-(-1);

              // case yline
              iTempDy=getContextPixel(uiDirMode,1, iTempDx, bCenter);

              iX=-1;
              iY=i+iTempDy;
            }
          }
          else  // select context pixel based on the distance
          {
            Bool bCenterx, bCentery;
            iTempDy = i-(-1);

            iTempDx= getContextPixel( uiDirMode, 0, iTempDy, bCenterx);
            iTempDx=-iTempDx;

            iXx     = j+iTempDx;
            iYx     = -1;

            iTempDx = j-(-1);
            iTempDy = getContextPixel( uiDirMode, 1, iTempDx, bCentery );
            iTempDy=-iTempDy;

            iXy     = -1;
            iYy     = i+iTempDy;

            if( iYy < -1 )
            {
              iX=iXx;
              iY=iYx;
              bCenter=bCenterx;
            }
            else
            {
              iX=iXy;
              iY=iYy;
              bCenter=bCentery;
            }
          }

          if( iY == -1 )
          {
            if(iX<-1)
              iX = -1;
            else if( iX >= 2*iWidth )
              iX = 2*iWidth-1;
          }
          else if( iX == -1 )
          {
            if( iY < -1 )
              iY = -1;
            else if( iY >= 2*iHeight )
              iY = 2*iHeight-1;
          }

          if (bCenter)
            pDst[i*iDstStride+j] = ptrSrc[(sy+iY)*sw+sx+iX];
          else {
            if( iY == -1 )
            {
              if ( iDx*iDy < 0 )
                iXn=iX+1;
              else
                iXn=iX-1;
              if(iXn<-1)
                iXn = -1;
              else if( iXn >= 2*iWidth )
                iXn = 2*iWidth-1;

              pDst[i*iDstStride+j] = (ptrSrc[(sy+iY)*sw+sx+iX]+ptrSrc[(sy+iY)*sw+sx+iXn]+1)>>1;
            }
            else if( iX == -1 )
            {
              if ( iDx*iDy < 0 )
                iYn=iY+1;
              else
                iYn=iY-1;
              if( iYn < -1 )
                iYn = -1;
              else if( iYn >= 2*iHeight )
                iYn = 2*iHeight-1;

              pDst[i*iDstStride+j] = (ptrSrc[(sy+iY)*sw+sx+iX]+ptrSrc[(sy+iYn)*sw+sx+iX]+1)>>1;
            }
          }
        }
      }
    } // i, j loop
    ret=true;
  }  //  if (bDirFlag)


  return;
}



//<- ADI_CHROMA



Void TComPrediction::xPredIntraVert( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Pel* pDst = rpDst;
  ::memcpy( pDst, pSrc - iSrcStride, sizeof(Pel)*iWidth );
  for ( Int i = 1; i < iHeight; i++ )
  {
    pDst += iDstStride;
    ::memcpy( pDst, pDst - iDstStride, sizeof(Pel)*iWidth );
  }

  return;
}

Void TComPrediction::xPredIntraHor( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Pel* pDst = rpDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    pDst[0] = pSrc[-1];
    for ( Int x = 1; x < iWidth; x++ )
    {
      pDst[x] = pDst[x-1];
    }
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
  return;
}

Void TComPrediction::xPredIntraDC( TComPattern* pcTComPattern, Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Int   x, y;
  Int   iDCValue = 0;
  Pel*  pSrcTemp = pSrc;
  Pel*  pDst = rpDst;

  if ( pcTComPattern->isLeftAvailable() ) // left neighboring sample is available
  {
    x = -1;
    for ( y = 0; y < iHeight; y++ )
    {
      iDCValue += pSrc[x];
      pSrc += iSrcStride;
    }

    pSrc = pSrcTemp - iSrcStride;
    if ( pcTComPattern->isAboveAvailable() ) // if left and above neighboring sample are available
    {
      for ( x = 0; x < iWidth; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += iWidth;
      iDCValue /= iWidth + iHeight;
    }
    else
    {
      iDCValue += iHeight / 2;
      iDCValue /= iHeight;
    }
  }
  else
  {
    pSrc = pSrcTemp - iSrcStride;
    if ( pcTComPattern->isAboveAvailable() ) // if above neighboring sample is available
    {
      for ( x = 0; x < iWidth; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += iWidth / 2;
      iDCValue /= iWidth;
    }
    else
    {
      iDCValue = (1<<(g_uiBitDepth+g_uiBitIncrement-1));
    }
  }

  for ( y = 0; y < iHeight; y++ )
  {
    for ( x = 0; x < iWidth; x++ )
    {
      pDst[x] = iDCValue;
    }
    pDst += iDstStride;
  }

  return;
}

Void TComPrediction::xPredIntraPlane( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt uiWidth, UInt uiHeight )
{
  Int iH = 0;
  Int iV = 0;
  Int iWidth  = uiWidth;
  Int iHeight = uiHeight;
  Int iA, iB, iC;
  Int x, y;
  Int iW2 = iWidth>>1;
  Int iH2 = iHeight>>1;
  Int iWeight = 0;
  Int iTmp;

  Pel* pSrcTemp = pSrc;
  Pel* pDst     = rpDst;

  pSrc += iW2 - iSrcStride - 1;
  for ( x = 1; x < iW2+1; x++ )
  {
    iH += x * (pSrc[x] - pSrc[-x]);
  }

  pSrc += (iSrcStride * iH2) - iW2;
  for( x = iSrcStride, y = 1; y < iH2; y++, x += iSrcStride)
  {
    iV += y * (pSrc[x] - pSrc[-x]);
  }

  pSrc    = pSrcTemp;
  iWeight = iW2 * ( iW2 + 1 ) * ( (iW2<<1) + 1 );
  iA = ( pSrc[(iHeight-1)*iSrcStride - 1] + pSrc[iWidth-1 - iSrcStride] ) << 4;
  iB = ( 3*(iH<<5) + (iWeight>>1) ) / iWeight;
  iC = ( 3*(iV<<5) + (iWeight>>1) ) / iWeight;

  iTmp = iA - iH2*iC - iW2*iB + 16;
  for ( y = 0; y < iHeight; y++ )
  {
    Int iTmp2 = iTmp;
    for ( x = 0; x < iWidth; x++ )
    {
      pDst[x] = Clip( iTmp2 >> 5 );
      iTmp2 += iB;
    }
    iTmp += iC;

    pDst += iDstStride;
  }
  return;
}

Void TComPrediction::xPredIntraVertAdi( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Pel* pDst = rpDst;
  Int* rpSrc = pSrc - iSrcStride;
  Int i =0;
  for (i=0;i<iWidth;i++)
  {
    pDst[i]=(Pel)rpSrc[i];
  }

  for ( i = 1; i < iHeight; i++ )
  {
    pDst += iDstStride;
    ::memcpy( pDst, pDst - iDstStride, sizeof(Pel)*iWidth );
  }
  return;
}

Void TComPrediction::xPredIntraHorAdi( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Pel* pDst = rpDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    pDst[0] = pSrc[-1];
    for ( Int x = 1; x < iWidth; x++ )
    {
      pDst[x] = pDst[x-1];
    }
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
  return;
}

Void TComPrediction::xPredIntraTM( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight, TComPattern* pcTComPattern)
{
	static Char av[ MAX_CU_SIZE*2 ][ MAX_CU_SIZE*3 ];
	static Pel  f [ MAX_CU_SIZE*2 ][ MAX_CU_SIZE*3 ];
	Pel*			  pTmp;

	Int		x, y;
	Int		xtr, ytr;
	Int		xtr_s, xtr_e, ytr_s, ytr_e;

	Int		fcost;
	Int		best_fcost[2], best_xtr[2], best_ytr[2], best2_idx, cnt;

	Int W=(Int)iWidth;
	Int H=(Int)iHeight;
	Int Wmax=2*W;

	if ( pcTComPattern->isAboveRightAvailable() )
	{
		Wmax+=W;
	}

	Int l=3;

	Pel* rpDst_temp = rpDst;

	// initialize the region according to its actual size
	for ( y=H-l; y<2*H; y++ )
	{
		::memset (&f [y][W-l], 0, sizeof(Pel )*(Wmax-W+l) );
		::memset (&av[y][W-l], 0, sizeof(Char)*(Wmax-W+l) );
	}

	/// --- f -----///
	if (pcTComPattern->isAboveLeftAvailable())
	{
		pTmp = &pSrc [(0-W)+(0-H)*iSrcStride];
		for ( y = 0; y < H; y++)
		{
			::memcpy( &f [y], pTmp, sizeof(Pel )*Wmax );
			::memset( &av[y], 1,    sizeof(Char)*Wmax );
			pTmp += iSrcStride;
		}
		for ( y = H; y < 2*H; y++)
		{
			::memcpy( &f [y], pTmp, sizeof(Pel )*W );
			::memset( &av[y], 1,    sizeof(Char)*W );
			pTmp += iSrcStride;
		}
	}
	else if (pcTComPattern->isLeftAvailable())
	{
		pTmp = &pSrc [(0-W)+(H-H)*iSrcStride];
		for ( y = H; y < 2*H; y++)
		{
			::memcpy( &f [y], pTmp, sizeof(Pel )*W );
			::memset( &av[y], 1,    sizeof(Char)*W );
			pTmp += iSrcStride;
		}
	}
	else if (pcTComPattern->isAboveAvailable())
	{
		pTmp = &pSrc [(W-W)+(0-H)*iSrcStride];
		x    = Wmax-W;
		for ( y = 0; y < H; y++)
		{
			::memcpy( &f [y][W], pTmp, sizeof(Pel )*x );
			::memset( &av[y][W], 1,    sizeof(Char)*x );
			pTmp += iSrcStride;
		}
	}

	for ( y = H; y < 2*H; y++)
	{
		ytr_s = Max(y-l,1); ytr_e = Min(2*H, y+l);
		for ( x = W; x < 2*W; x++)
		{
			// get range
			xtr_s = Max(x-l,1); xtr_e = Min(Wmax,x+l);

			// initialize variables
			best_fcost[0] = best_fcost[1] = MAX_INT;
			best_xtr  [0] = best_xtr  [1] = x;
			best_ytr  [0] = best_ytr  [1] = y;
			best2_idx     = 0;
			cnt           = 0;

			// for searching range
			for ( ytr = ytr_s; ytr < ytr_e; ytr++)
			{
				for ( xtr = xtr_s; xtr < xtr_e; xtr++)
				{
					if (av[ytr][xtr] && av[ytr-1][xtr] && av[ytr][xtr-1] && av[ytr-1][xtr-1] )
					{
						fcost = ( abs( f[y][x-1]-f[ytr][xtr-1] ) + abs( f[y-1][x-1]-f[ytr-1][xtr-1] ) + abs( f[y-1][x]-f[ytr-1][xtr] )) << 7;
						fcost +=((xtr-x)*(xtr-x)+(ytr-y)*(ytr-y));

						// compared to second-best element
						if ( fcost < best_fcost[best2_idx] )
						{
							best_fcost[best2_idx] = fcost;
							best_xtr  [best2_idx] = xtr;
							best_ytr  [best2_idx] = ytr;

#if TMI_AVERAGE
							cnt                   ++;

							// update second-best element index
							if ( best_fcost[best2_idx] < best_fcost[1-best2_idx] )
							{
								best2_idx = 1-best2_idx;
							}
#endif

						}
					}
				}
			}

			// check number of total candidates
#if TMI_AVERAGE
			if ( cnt < 2 )
			{
				rpDst[x-W]=f[y][x]=( f[best_ytr[0]][best_xtr[0]] );
			}
			else
			{
				rpDst[x-W]=f[y][x]=( f[best_ytr[0]][best_xtr[0]] + f[best_ytr[1]][best_xtr[1]] + 1 ) >> 1;
			}
#else
			rpDst[x-W]=f[y][x]=( f[best_ytr[0]][best_xtr[0]] );
#endif

			// mark "processed"
			av[y][x]=true;
		}
		rpDst += iDstStride;
	}

	return;
}

Void TComPrediction::xPredIntraDCAdi( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight,Bool bAboveAvail, Bool bLeftAvail )
{
  Int   x, y;
  Int   iDCValue = 0;
  Int*  pSrcTemp = pSrc;
  Pel*  pDst = rpDst;

  if (bLeftAvail ) // left neighboring sample is available
  {
    x = -1;
    for ( y = 0; y < iHeight; y++ )
    {
      iDCValue += pSrc[x];
      pSrc += iSrcStride;
    }

    pSrc = pSrcTemp - iSrcStride;
    if (bAboveAvail ) // if left and above neighboring sample are available
    {
      for ( x = 0; x < iWidth; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += iWidth;
      iDCValue /= iWidth + iHeight;
    }
    else
    {
      iDCValue += iHeight / 2;
      iDCValue /= iHeight;
    }
  }
  else
  {
    pSrc = pSrcTemp - iSrcStride;
    if (bAboveAvail) // if above neighboring sample is available
    {
      for ( x = 0; x < iWidth; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += iWidth / 2;
      iDCValue /= iWidth;
    }
    else
    {
      iDCValue = (1<<( g_uiBitDepth + g_uiBitIncrement - 1 ) );
    }
  }

  for ( y = 0; y < iHeight; y++ )
  {
    for ( x = 0; x < iWidth; x++ )
    {
      pDst[x] = iDCValue;
    }
    pDst += iDstStride;
  }

  return;
}

Void TComPrediction::xPredIntraPlaneAdi(Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt uiWidth, UInt uiHeight )
{
  Int iH = 0;
  Int iV = 0;
  Int iWidth  = uiWidth;
  Int iHeight = uiHeight;
  Int iA, iB, iC;
  Int x, y;
  Int iW2 = iWidth>>1;
  Int iH2 = iHeight>>1;
  Int iWeight = 0;
  Int iTmp;

  Int* pSrcTemp = pSrc;
  Pel* pDst     = rpDst;

  pSrc += iW2 - iSrcStride - 1;
  for ( x = 1; x < iW2+1; x++ )
  {
    iH += x * (pSrc[x] - pSrc[-x]);
  }

  pSrc += (iSrcStride * iH2) - iW2;
  for( x = iSrcStride, y = 1; y < iH2; y++, x += iSrcStride)
  {
    iV += y * (pSrc[x] - pSrc[-x]);
  }

  pSrc    = pSrcTemp;
  iWeight = iW2 * ( iW2 + 1 ) * ( (iW2<<1) + 1 );
  iA = ( pSrc[(iHeight-1)*iSrcStride - 1] + pSrc[iWidth-1 - iSrcStride] ) << 4;
  iB = ( 3*(iH<<5) + (iWeight>>1) ) / iWeight;
  iC = ( 3*(iV<<5) + (iWeight>>1) ) / iWeight;

  iTmp = iA - iH2*iC - iW2*iB + 16;
  for ( y = 0; y < iHeight; y++ )
  {
    Int iTmp2 = iTmp;
    for ( x = 0; x < iWidth; x++ )
    {
      pDst[x] = Clip( iTmp2 >> 5 );
      iTmp2 += iB;
    }
    iTmp += iC;

    pDst += iDstStride;
  }
  return;
}



Void TComPrediction::xPredIntraBiAdi( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight )
{
  Int x, y;
  Int iW2 = iWidth>>1;
  Int iH2 = iHeight>>1;

  Int* pCorner =  pSrc - iSrcStride; // pCorner[x] - points 0...iHeight-1 along x axis
  Int* pDown = pSrc + (iHeight-1) * iSrcStride; // pDown[-1] - left down pixel

  Int a = (pCorner[iWidth-1] * iHeight + pDown[-1] * iWidth + iW2 + iH2) / ( iWidth  + iHeight) ;

  Int ky = (a - pCorner[iWidth-1]); // the slope along vertical right
  Int kx = (a - pDown[-1]);         // the slope along horizontal down

  Pel* pDst = rpDst;

  for ( y = 0; y < iHeight; y++)
  {
    // intermediate line
    Int* pY = pSrc + y * iSrcStride; // pY[-1] - left point
    Int pRight;
    if(ky>=0)
      pRight = pCorner[iWidth-1] + ((ky * (y + 1)+iH2) /iHeight);
    else
      pRight = pCorner[iWidth-1] + ((ky * (y + 1)-iH2) /iHeight);

    Int kxIntermediate = (pRight - pY[-1]);

    for ( x = 0; x < iWidth; x++)
      {
        Int xVal ;
        if (kxIntermediate>=0) xVal = pY[-1] + ((kxIntermediate * (x + 1)+iW2) /iWidth);
        else xVal = pY[-1] + ((kxIntermediate * (x + 1)-iW2) /iWidth);
        Int pxDown ;
        if (kx>=0)           pxDown = pDown[-1] + ((kx * (x + 1)+iW2) /iWidth );
        else                 pxDown = pDown[-1] + ((kx * (x + 1)-iW2) /iWidth );

        Int kyIntermediate = (pxDown - pCorner[x]);
        Int yVal ;
        if (kyIntermediate>=0)           yVal = pCorner[x] + ((kyIntermediate * (y + 1)+iH2) /iHeight);
        else                             yVal = pCorner[x] + ((kyIntermediate * (y + 1)-iH2) /iHeight);

        if ((xVal * (Int)iHeight + yVal * (Int)iWidth)>=0)
           pDst[x] = Clip((xVal * (Int)iHeight+ yVal * (Int)iWidth + iW2 + iH2) / ( (Int)iWidth  + (Int)iHeight));
        else
           pDst[x] = Clip((xVal * (Int)iHeight+ yVal * (Int)iWidth - iW2 - iH2) / ( (Int)iWidth  + (Int)iHeight));
      }
    pDst += iDstStride;
  }
  return;
}


Int TComPrediction::getContextPixel(UInt uiDirMode,UInt uiXYflag,Int iTempD, Bool& bCenter)
{
  Int iDxt,iDyt,iDx,iDy,iTempm, iTempr,iTempDn;

  iTempDn=0;

  iTempm=0;
  iTempr=0;

  iDxt=g_aucDirDx[uiDirMode-4];
  iDyt=g_aucDirDy[uiDirMode-4];

  iDx=abs(g_aucDirDx[uiDirMode-4]);
  iDy=abs(g_aucDirDy[uiDirMode-4]);


  if (uiXYflag==0)
  {
    iTempDn=iDx*iTempD/iDy;
    if ((iDx*iTempD)%iDy==0) {bCenter=true;     }
    else { bCenter=false;                           }
  }
  else
  {
    iTempDn=iDy*iTempD/iDx;

    if ((iDy*iTempD)%iDx==0) { bCenter=true;   }
    else {  bCenter=false;                            }
  }

  return iTempDn;
}


Void TComPrediction::predIntraLumaNxNAdi(TComPattern* pcTComPattern,UInt uiDirMode, Pel*& rpPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft, UChar indexMPI )
{
  Bool  bDirFlag   = true;
  Int   i, j;
  Pel  *pDst       = rpPred;
  Pel  *ptrDst     = pDst;
  Int   sx, sy, sw, iWidth2;
  UInt  uixyflag   = 0; // 0 for x axis 1 for y axis
  Int   iDx, iDy, iXx, iYx, iXy, iYy, iX, iY, iTempDx, iTempDy;
  Int   iXn, iYn;
  Bool  bCenter;

  Int* ptrSrc;
  Int* ptrQSrc;

  Int	iIntraSizeIdx;

  UInt ui = iWidth;
  iIntraSizeIdx = 0;
  iWidth2 = iWidth<<1;

  while( true )
  {
    if( ui & 0x1 )
    {
      break;
    }

    ui >>= 1;
    iIntraSizeIdx++;
  }
  iIntraSizeIdx--;

  assert( iIntraSizeIdx >= 0 );

  if( iIntraSizeIdx > 6 )
    return;

  UInt dirmodeold=uiDirMode;
  // TM_INTRA
  if (pcCU->getSlice()->getSPS()->getUseTMI() && uiDirMode==33)
	  ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
  else
  {
  uiDirMode = g_aucIntraModeOrder[ iIntraSizeIdx ][ uiDirMode ];

  if( g_aucIntraFilter[iIntraSizeIdx][uiDirMode] == 0 ){
    ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
  }
  else if( g_aucIntraFilter[iIntraSizeIdx][uiDirMode] == 1 ){
    ptrSrc = pcTComPattern->getAdiFilteredBuf1( iWidth, iHeight, m_piYuvExt );
  }
  else
  {
    ptrSrc = pcTComPattern->getAdiFilteredBuf2( iWidth, iHeight, m_piYuvExt );
  }
  }

  ptrQSrc = ptrSrc;

  sy = 1;
  sx = 1;
  sw = iWidth2+1;

  ptrQSrc += sw*(sy)+sx;

  for(i=0;i<iHeight; i++)
	{
    // ::memset( ptrQSrc, 0x00, iWidth*sizeof(Short) );
    ::memset( ptrDst,  0x00, iWidth*sizeof(Short) );

    ptrQSrc += sw;
    ptrDst  += uiStride;
	}

  iDx = 1;
  iDy = 1;

  ptrDst = pDst;
  switch( uiDirMode )
  {
  case 0:  // Vertical
    {
      xPredIntraVertAdi( ptrSrc+sw+1, sw, ptrDst, uiStride, iWidth, iHeight );
      bDirFlag = false;
      break;
    }
  case 1:  // Horizontal
    {
      xPredIntraHorAdi( ptrSrc+sw+1, sw, ptrDst, uiStride, iWidth, iHeight );
      bDirFlag = false;
      break;
    }
  case 2:  // DC
    {
      xPredIntraDCAdi( ptrSrc+sw+1, sw, ptrDst, uiStride, iWidth, iHeight, bAbove,  bLeft );
      bDirFlag = false;
      break;
    }
  case 3:  // Plane
    {
      xPredIntraPlaneAdi( ptrSrc+sw+1,sw, ptrDst, uiStride, iWidth, iHeight );
      bDirFlag = false;
      break;
    }
  case 32:  // bi
    {
      xPredIntraBiAdi ( ptrSrc+sw+1, sw, ptrDst, uiStride, iWidth, iHeight );
      bDirFlag = false;
      break;
    }
// TM_INTRA
	case 33:  //TM
    {
	  Pel* pSrcRescuer = pcTComPattern->getROIY();
	  Int  iSrcStride = pcTComPattern->getPatternLStride();
	  if (iWidth<16)  pSrcRescuer = pcTComPattern->getROIYBlk( 0 );

	  if (pcTComPattern->isAboveAvailable() || pcTComPattern->isLeftAvailable())
    	  xPredIntraTM ( pSrcRescuer, iSrcStride, ptrDst, uiStride, iWidth, iHeight, pcTComPattern );
	  else
		xPredIntraDCAdi( ptrSrc+sw+1, sw, ptrDst, uiStride, iWidth, iHeight, bAbove,  bLeft );

	  bDirFlag = false;
	  break;
    }
  default:
    {
      iDx      = g_aucDirDx[ uiDirMode-4 ];
      iDy      = g_aucDirDy[ uiDirMode-4 ];
      uixyflag = g_aucXYflg[ uiDirMode   ];
      break;
    }
  }

  if( bDirFlag )
  {
    Int iStride;
    Int iDyP, iDyN;
    Int iHeight2 = iHeight<<1;
    Int iDxy = iDx*iDy;
    for( i = 0, iDyP = iDy, iDyN = -iDy, iStride = 0; i < iHeight; i++, iStride+=uiStride, iDyP++, iDyN++ )
    {
      Int iDxP, iDxN;
      Int iStrideJ;
      Int iDyPStride = iDyP*uiStride;
      Int iDyNStride = iDyN*uiStride;
      for( j = 0, iDxP = iDx, iDxN = -iDx, iStrideJ = iStride; j < iWidth; j++, iDxP++, iDxN++, iStrideJ++ )
      {
        Bool bSetFlag1=false;
        Bool bSetFlag2=false;

        if ( ( iDyP >= 0 ) && ( iDyP < iHeight ) && ( iDxP >= 0 ) && ( iDxP < iWidth ) )
          if ( pDst[ iDyPStride+iDxP ] > 0 )
            bSetFlag1=true;

        if( !bSetFlag1 )
          if ( ( iDyN >= 0 ) && ( iDyN < iHeight ) && ( iDxN >= 0 ) && ( iDxN < iWidth ) )
            if (pDst[ iDyNStride+iDxN ]>0)
              bSetFlag2=true;

        if( bSetFlag1 )
        {
          pDst[ iStrideJ ] = pDst[ iDyPStride+iDxP ];
        }
        else if(bSetFlag2)
        {
          pDst[ iStrideJ ] = pDst[ iDyNStride+iDxN ];
        }
        else
        {
          if ( iDxy < 0 )   // select context pixel based on uixyflag
          {
            if ( uixyflag == 0 )
            {
              iTempDy = i-(-1);

              // case xline
              iTempDx = getContextPixel(uiDirMode,0,iTempDy, bCenter);

              iX = j+iTempDx;
              iY = -1;
            }
            else
            {
              iTempDx = j-(-1);

              // case yline
              iTempDy = getContextPixel(uiDirMode,1, iTempDx, bCenter);

              iX = -1;
              iY = i+iTempDy;
            }
          }
          else  // select context pixel based on the distance
          {
            Bool bCenterx, bCentery;
            iTempDy = i-(-1);

            iTempDx = getContextPixel( uiDirMode, 0, iTempDy, bCenterx);
            iTempDx = -iTempDx;

            iXx     = j+iTempDx;
            iYx     = -1;

            iTempDx = j-(-1);
            iTempDy = getContextPixel( uiDirMode, 1, iTempDx, bCentery );
            iTempDy = -iTempDy;

            iXy     = -1;
            iYy     = i+iTempDy;

            if( iYy < -1 )
            {
              iX=iXx;
              iY=iYx;
              bCenter=bCenterx;
            }
            else
            {
              iX=iXy;
              iY=iYy;
              bCenter=bCentery;
            }
          }

          if( iY == -1 )
          {
            if(iX<-1)
              iX = -1;
            else if( iX >= iWidth2 )
              iX = iWidth2-1;
          }
          else if( iX == -1 )
          {
            if( iY < -1 )
              iY = -1;
            else if( iY >= iHeight2 )
              iY = iHeight2-1;
          }

          if (bCenter)
            pDst[iStrideJ] = ptrSrc[(sy+iY)*sw+sx+iX];
          else
          {
            if( iY == -1 )
            {
              if ( iDxy < 0 )
                iXn=iX+1;
              else
                iXn=iX-1;
              if(iXn<-1)
                iXn = -1;
              else if( iXn >= iWidth2 )
                iXn = iWidth2-1;

              Int iTemp = (sy+iY)*sw+sx;

              pDst[iStrideJ] = (ptrSrc[iTemp+iX]+ptrSrc[iTemp+iXn]+1)>>1;
            }
            else if( iX == -1 )
            {
              if ( iDxy < 0 )
                iYn=iY+1;
              else
                iYn=iY-1;
              if( iYn < -1 )
                iYn = -1;
              else if( iYn >= iHeight2 )
                iYn = iHeight2-1;

              Int iTemp = sx+iX;

              pDst[iStrideJ] = (ptrSrc[(sy+iY)*sw+iTemp]+ptrSrc[(sy+iYn)*sw+iTemp]+1)>>1;
            }
          }
        }
      }
    } // i, j loop
  }  //  if (bDirFlag)

	if ( m_bUseMPI )
	{
		if( pcTComPattern->isAboveLeftAvailable() )
		{
			Pel* pSrcRescuer = pcTComPattern->getROIY();
			Int  iSrcStride  = pcTComPattern->getPatternLStride();

			xMPI_POST_PROCESSING( pSrcRescuer, iSrcStride, rpPred, uiStride, iWidth, iHeight, indexMPI );
		}
	}
}

Void TComPrediction::xMPI_POST_PROCESSING( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight, UChar indexMPI )
{
  Pel* pDst = rpDst;
  static Int f[128][128], df[128][128];
	Int x, y;
  Int iDstStride2, iSrcStride2;
  switch (indexMPI)
  {
  case 0:
    {}
    break;
  case 1:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      for ( y = 1; y < iHeight-1; y++ )
      {
        for ( x = 1; x < iWidth; x++ )
          pDst[x] =(pDst[x]+pDst[x-1]+pDst[x-iDstStride]+pDst[x+iDstStride]+2)>>2;

        pDst += iDstStride;
      }

      for ( x = 1; x < iWidth; x++ )
		    pDst[x] =( (pDst[x]<<1)+pDst[x-1]+pDst[x-iDstStride]+2)>>2;
    }
    break;
  case 2:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;


      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      //inner pixels processing
      for ( y = 1; y < iHeight; y++ )
      {
        for ( x = 1; x < iWidth; x++ )
          pDst[x] =(pDst[x]+pDst[x-1]+pDst[x-iDstStride]+pDst[x-iDstStride-1]+2)>>2;

        pDst += iDstStride;
      }
    }
    break;
  case 3:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      //inner pixels processing
      for ( y = 1; y < iHeight; y++ )
      {
        for ( x = 1; x < iWidth-1; x++ )
          pDst[x] =( (pDst[x]<<1)+pDst[x+1]+pDst[x-iDstStride]+2)>>2;

        pDst[x] = ( (pDst[x]<<1)+pDst[x]+pDst[x-iDstStride]+2)>>2;

        pDst += iDstStride;
      }
    }
    break;
  case 4:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      //inner pixels processing
      for ( y = 1; y < iHeight; y++ )
      {
        for ( x = 1; x < iWidth; x++ )
          pDst[x] = ( (pDst[x]<<1)+pDst[x-1]+pDst[x-iDstStride]+2)>>2;

        pDst += iDstStride;
      }
    }
    break;
  case 5:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      //inner pixels processing
      for ( y = 1; y < iHeight-1; y++ )
      {
        for ( x = 1; x < iWidth-1; x++ )
          pDst[x] =(pDst[x]+pDst[x+1]+pDst[x-iDstStride]+pDst[x+iDstStride]+2)>>2;

        pDst[x] =( (pDst[x]<<1)+pDst[x-iDstStride]+pDst[x+iDstStride]+2)>>2;

        pDst += iDstStride;
      }

      for ( x = 1; x < iWidth-1; x++ )
        pDst[x] = ( (pDst[x]<<1)+pDst[x+1]+pDst[x-iDstStride]+2)>>2;

      pDst[x] =( (pDst[x]<<1)+pDst[x]+pDst[x-iDstStride]+2)>>2;
    }
    break;
  case 6:
    {
      // boundary pixels processing
      pDst[0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      pDst += iDstStride;
      //inner pixels processing
      for ( y = 1; y < iHeight-1; y++ )
      {
        for ( x = 1; x < iWidth; x++ )
          pDst[x] =(pDst[x]+pDst[x-1]+pDst[x-iDstStride]+pDst[x+iDstStride]+2)>>2;

        pDst += iDstStride;
      }

      for ( x = 1; x < iWidth; x++ )
        pDst[x] =( (pDst[x]<<1)+pDst[x]+pDst[x-1]+pDst[x-iDstStride]+2)>>2;
    }
    break;
  case 7:
    {
      // boundary pixels processing
      f[0][0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        f[0][x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst[x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        f[y][0] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst[iDstStride2-iDstStride] +2) >>2;

      //inner pixels processing
      for ( y = 1, iDstStride2 = iDstStride; y < iHeight; y++, iDstStride2+=iDstStride )
      {
        for ( x = 1; x < iWidth; x++ )
          f[y][x] =( (pDst[x+iDstStride2]<<1)+pDst[x-1+iDstStride2]+pDst[x-iDstStride+iDstStride2]+2)>>2;
      }

      for ( y = 0; y < iHeight; y++ )
      {
        for ( x = 0; x < iWidth; x++ )
          pDst[x] = f[y][x];

        pDst += iDstStride;
      }
    }
    break;
  case 8:
    {
      // boundary pixels processing
      f[0][0] =Median(pDst[0],pSrc[-1],pSrc[-iSrcStride]);

      for ( x = 1; x < iWidth; x++ )
        f[0][x] =Median(pDst[x],pDst[x-1],pSrc[x-iSrcStride]);

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
      {
        f[y][0] =Median(pDst[iDstStride2],pSrc[iSrcStride2],pDst[-iDstStride+iDstStride2]);

        //inner pixels processing
        for ( x = 1; x < iWidth; x++ )
          f[y][x] =Median(pDst[x+iDstStride2],pDst[x-1+iDstStride2],pDst[x-iDstStride+iDstStride2]);
      }

      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2] = f[y][x];
    }
    break;
  case 9:
    {
      // boundary pixels processing
      f[0][0] = ( (pDst[0]<<1) + pSrc[-iSrcStride] + pSrc[-1] +2) >>2;

      for ( x = 1; x < iWidth; x++ )
        f[0][x] = ( (pDst[x]<<1) + pSrc[x-iSrcStride] + pDst [x-1] +2) >>2;

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
      {
        f[y][0] = ( (pDst[iDstStride2]<<1) + pSrc[iSrcStride2] + pDst [iDstStride2-iDstStride] +2) >>2;

        //inner pixels processing
        for ( x = 1; x < iWidth; x++ )
          f[y][x] =( (pDst[x+iDstStride2]<<1)+pDst[x-1+iDstStride2]+pDst[x-iDstStride+iDstStride2]+2)>>2;
      }

      for ( y = 1, iDstStride2 = iDstStride; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2]= f[y][x] ;
    }
    break;
  case 10:
    {
      Int alfa = (1<<(g_uiBitIncrement + 4)), alfa2 = (alfa>>1), iterMax = 5;
      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          f[y][x] = pDst[x+iDstStride2];

      for (Int iter=0; iter<iterMax; iter++)
      {
        ////periodic boundary condition
        df[0][0] = - (f[0][0]<<2) + f[1][0]+ pSrc[-iSrcStride]+ f[0][1]+ pSrc[-1];

        for ( x = 1; x < iWidth-1; x++ )
          df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x+1]+ f[0][x-1];

        df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ pSrc[-1]+ f[0][x-1];

        for ( y = 0, iSrcStride2 = -1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
          df[y][0] = -(f[y][0]<<2) + f[y+1][0]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        df[y][0] = -(f[y][0]<<2) + pSrc[-iSrcStride]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        for ( y = 1, iSrcStride2 = iSrcStride-1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
        {
          for (x = 1; x < iWidth-1; x++ )
            df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

          df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ pSrc[iSrcStride2]+ f[y][x-1];
        }

        for ( x = 1; x < iWidth-1; x++ )
          df[y][x] = -(f[y][x]<<2) + pSrc[x-iSrcStride]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

        df[y][x] = -(f[y][x]<<2) + pSrc[x-iSrcStride]+ f[y-1][x]+ pSrc[iSrcStride2]+ f[y][x-1];

        for ( y = 0; y < iHeight; y++ )
          for ( x = 0; x < iWidth; x++ )
          {
            Int iTmp = f[y][x]*alfa + df[y][x];
            if( iTmp>=0 ) f[y][x] = (iTmp + alfa2)/alfa;
            else          f[y][x] = (iTmp - alfa2)/alfa;
          }
      }

      for ( y = 0, iDstStride2 = iDstStride; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2] = f[y][x];
    }
    break;
  case 11:
    {
      Int alfa = (1<<(g_uiBitIncrement + 3)), alfa2 = (alfa>>1), iterMax = 3;
      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
	         f[y][x] = pDst[x+iDstStride2];

      for (Int iter=0; iter<iterMax; iter++)
      {
        ////periodic boundary condition
        df[0][0] = -(f[0][0]<<2) + f[1][0]+ pSrc[-iSrcStride]+ f[0][1]+ pSrc[-1];

        for ( x = 1; x < iWidth-1; x++ )
          df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x+1]+ f[0][x-1];

        df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ pSrc[-1]+ f[0][x-1];


        for ( y = 0, iSrcStride2 = -1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
          df[y][0] = -(f[y][0]<<2) + f[y+1][0]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        df[y][0] = -(f[y][0]<<2) + pSrc[-iSrcStride]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        for ( y = 1, iSrcStride2 = iSrcStride-1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
        {
          for (x = 1; x < iWidth-1; x++ )
            df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

          df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ pSrc[iSrcStride2]+ f[y][x-1];
        }

        for ( x = 1; x < iWidth-1; x++ )
          df[y][x] = -(f[y][x]<<2) + pSrc[x-iSrcStride]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

        df[y][x] = -(f[y][x]<<2) + pSrc[x-iSrcStride]+ f[y-1][x]+ pSrc[iSrcStride2]+ f[y][x-1];

        for ( y = 0; y < iHeight; y++ )
          for ( x = 0; x < iWidth; x++ )
          {
            Int iTmp = f[y][x] *alfa+ df[y][x];
            if( iTmp>=0 )  f[y][x] = (iTmp + alfa2)/alfa;
            else           f[y][x] = (iTmp - alfa2)/alfa;
          }
      }

      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2] = f[y][x];
    }
    break;

  case 12:
    {
      Int alfa = (1<<(g_uiBitIncrement + 4)), alfa2 = (alfa>>1), iterMax = 5;
      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
      {
        for ( x = 0; x < iWidth; x++ )
          f[y][x] = pDst[x+iDstStride2];
      }

      for (Int iter=0; iter<iterMax; iter++)
      {
        ////Neumann boundary condition
        df[0][0] = -(f[0][0]<<2) + f[1][0]+ pSrc[-iSrcStride]+ f[0][1]+ pSrc[-1];

        for ( x = 1; x < iWidth-1; x++ )
          df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x+1]+ f[0][x-1];

        df[0][x] = -(f[0][x]<<2) + f[0][x] + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x-1];

        for ( y = 0, iSrcStride2 = -1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
          df[y][0] = -(f[y][0]<<2) + f[y][0] + f[y+1][0]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        df[y][0] = -(f[y][0]<<2) + f[y][0] + f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        for ( y = 1; y < iHeight-1; y++ )
        {
          for (x = 1; x < iWidth-1; x++ )
            df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

          df[y][x] = -(f[y][x]<<2) + f[y][x] + f[y+1][x]+ f[y-1][x]+ f[y][x-1];
        }

        for ( x = 1; x < iWidth-1; x++ )
          df[y][x] = -(f[y][x]<<2) + f[y][x] + f[y-1][x]+ f[y][x+1]+ f[y][x-1];

        df[y][x] = -(f[y][x]<<1) + f[y-1][x] + f[y][x-1];

        for ( y = 0; y < iHeight; y++ )
          for ( x = 0; x < iWidth; x++ )
          {
            Int iTmp = f[y][x]*alfa + df[y][x];
            if( iTmp>=0 )  f[y][x] = (iTmp + alfa2)/alfa;
            else           f[y][x] = (iTmp - alfa2)/alfa;
          }
      }

      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2] = f[y][x];
    }
    break;
  case 13:
    {
      Int alfa = (1<<(g_uiBitIncrement + 3)), alfa2 = (alfa>>1), iterMax = 3;
      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
      {
        for ( x = 0; x < iWidth; x++ )
          f[y][x] = pDst[x+iDstStride2];
      }

      for (Int iter=0; iter<iterMax; iter++)
      {
        ////Neumann boundary condition
        df[0][0] = -(f[0][0]<<2) + f[1][0]+ pSrc[-iSrcStride]+ f[0][1]+ pSrc[-1];

        for ( x = 1; x < iWidth-1; x++ )
          df[0][x] = -(f[0][x]<<2) + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x+1]+ f[0][x-1];

        df[0][x] = -(f[0][x]<<2) + f[0][x] + f[1][x]+ pSrc[x-iSrcStride]+ f[0][x-1];

        for ( y = 0, iSrcStride2 = -1; y < iHeight-1; y++, iSrcStride2+=iSrcStride )
          df[y][0] = -(f[y][0]<<2) + f[y+1][0]+ f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        df[y][0] = -(f[y][0]<<2) + f[y][0] + f[y-1][0]+ f[y][1]+ pSrc[iSrcStride2];

        for ( y = 1; y < iHeight-1; y++ )
        {
          for (x = 1; x < iWidth-1; x++ )
            df[y][x] = -(f[y][x]<<2) + f[y+1][x]+ f[y-1][x]+ f[y][x+1]+ f[y][x-1];

          df[y][x] = -(f[y][x]<<2) + f[y][x] + f[y+1][x]+ f[y-1][x]+ f[y][x-1];
        }

        for ( x = 1; x < iWidth-1; x++ )
          df[y][x] = -(f[y][x]<<2) + f[y][x] + f[y-1][x] + f[y][x+1] + f[y][x-1];

        df[y][x] = -(f[y][x]<<1) + f[y-1][x] + f[y][x-1];

        for ( y = 0; y < iHeight; y++ )
          for ( x = 0; x < iWidth; x++ )
          {
            Int iTmp = f[y][x]*alfa + df[y][x];
            if( iTmp>=0)  f[y][x] = (iTmp + alfa2)/alfa;
            else          f[y][x] = (iTmp - alfa2)/alfa;
          }
      }

      for ( y = 0, iDstStride2 = 0; y < iHeight; y++, iDstStride2+=iDstStride )
        for ( x = 0; x < iWidth; x++ )
          pDst[x+iDstStride2] = f[y][x];
    }
    break;
  }
  return;
}

Void TComPrediction::xFilterHorPels( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& pDst )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( iBlkIdx );
  Int  iSrcStride = pcTComPattern->getPatternLStride();

  pSrc -= iSrcStride;
  if ( pcTComPattern->isAboveLeftAvailable() )
  {
    pDst[0] = (pSrc[-1] + 2*pSrc[0] + pSrc[1] + 2) >> 2;
  }
  else
  {
    pDst[0] = (3*pSrc[0] + pSrc[1] + 2) >> 2;
  }

  for ( Int i = 1; i < 7; i++ )
  {
    pDst[i] = (pSrc[i-1] + 2*pSrc[i] + pSrc[i+1] + 2) >> 2;
  }

  if ( ! pcTComPattern->isAboveRightAvailable() )
  {
    pDst[7] = (pSrc[6] + 3*pSrc[7] + 2) >> 2;

    for ( Int i = 8; i < 16; i++ )
    {
      pDst[i] = pSrc[7];
    }
  }
  else
  {
    pDst[7] = (pSrc[6] + 2*pSrc[7] + pSrc[8] + 2) >> 2;

    for ( Int i = 8; i < 15; i++ )
    {
      pDst[i] = (pSrc[i-1] + 2*pSrc[i] + pSrc[i+1] + 2) >> 2;
    }

    pDst[15] = (pSrc[14] + 3*pSrc[15] + 2) >> 2;
  }
}

Void TComPrediction::xFilterVerPels( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& pDst )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( iBlkIdx );
  Int  iSrcStride = pcTComPattern->getPatternLStride();

  if ( pcTComPattern->isAboveLeftAvailable() )
  {
    pDst[0] = (pSrc[-iSrcStride - 1] + 2*pSrc[-1] + pSrc[iSrcStride - 1] + 2) >> 2;
  }
  else
  {
    pDst[0] = (3*pSrc[-1] + pSrc[iSrcStride - 1] + 2) >> 2;
  }

  for ( Int i = 1; i < 7; i++ )
  {
    pDst[i] = (pSrc[(i-1)*iSrcStride - 1] + 2*pSrc[i*iSrcStride - 1] + pSrc[(i+1)*iSrcStride - 1] + 2) >> 2;
  }

  pDst[7] = (pSrc[6*iSrcStride - 1] + 3*pSrc[7*iSrcStride - 1] + 2) >> 2;
}

Void TComPrediction::xFilterXPels( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& rpDst )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( iBlkIdx );
  Int  iSrcStride = pcTComPattern->getPatternLStride();

  if ( pcTComPattern->isAboveLeftAvailable() )
  {
    if ( pcTComPattern->isAboveAvailable() && pcTComPattern->isLeftAvailable() )
      rpDst[0] = (pSrc[-iSrcStride] + 2*pSrc[-iSrcStride - 1] + pSrc[-1] + 2) >> 2;
    else
      fprintf(stdout, "Error in xFilterXPles()\n");
  }
  else
  {
    fprintf(stdout, "Error in xFilterXPles()\n");
  }
}

Void TComPrediction::xPredIntra8Vert( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[16];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[0];
  Pel* pDst = rpPred;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );

  ::memcpy( pDst, pFilteredHorSrc, sizeof(Pel)*8 );
  for ( Int i = 1; i < 8; i++ )
  {
    pDst += iStride;
    ::memcpy( pDst, pDst - iStride, sizeof(Pel)*8 );
  }

  return;
}

Void TComPrediction::xPredIntra8Hor( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredVerPels[8];
  Pel* pFilteredVerSrc = &aiFilteredVerPels[0];
  Pel* pDst = rpPred;

  xFilterVerPels( pcTComPattern, 0, pFilteredVerSrc );

  for ( Int y = 0; y < 8; y++ )
  {
    pDst[0] = pFilteredVerSrc[y];
    for ( Int x = 1; x < 8; x++ )
    {
      pDst[x] = pDst[x-1];
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8DC( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredPels[16];
  Pel* pFilteredSrc = &aiFilteredPels[0];
  Pel* pDst = rpPred;

  Int  x, y;
  Int  iDCValue = 0;

  if ( pcTComPattern->isLeftAvailable() ) // left neighboring sample is available
  {
    xFilterVerPels( pcTComPattern, 0, pFilteredSrc );

    for ( y = 0; y < 8; y++ )
    {
      iDCValue += pFilteredSrc[y];
    }

    if ( pcTComPattern->isAboveAvailable() ) // if left and above neighboring sample are available
    {
      xFilterHorPels( pcTComPattern, 0, pFilteredSrc );

      for ( x = 0; x < 8; x++ )
      {
        iDCValue += pFilteredSrc[x];
      }

      iDCValue += 8;
      iDCValue >>= 4;
    }
    else
    {
      iDCValue += 4;
      iDCValue >>= 3;
    }
  }
  else
  {
    if ( pcTComPattern->isAboveAvailable() ) // if above neighboring sample is available
    {
      xFilterHorPels( pcTComPattern, 0, pFilteredSrc );

      for ( x = 0; x < 8; x++ )
      {
        iDCValue += pFilteredSrc[x];
      }

      iDCValue += 4;
      iDCValue >>= 3;
    }
    else
    {
      iDCValue = (1<<(g_uiBitDepth+g_uiBitIncrement-1));
    }
  }

  for ( y = 0; y < 8; y++ )
  {
    for ( x = 0; x < 8; x++ )
    {
      pDst[x] = iDCValue;
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8DiagDownLeft( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[17];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[0];
  Pel* pDst = rpPred;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      pDst[x] = (pFilteredHorSrc[x+y] + 2*pFilteredHorSrc[x+y+1] + pFilteredHorSrc[x+y+2] + 2) >> 2;
    }
    pDst += iStride;
  }
  pDst[7-iStride] = (pFilteredHorSrc[14] + 3*pFilteredHorSrc[15] + 2) >> 2;

  return;
}

Void TComPrediction::xPredIntra8DiagDownRight( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[17];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[1];
  Pel  aiFilteredVerPels[9];
  Pel* pFilteredVerSrc = &aiFilteredVerPels[1];
  Pel* pFilteredXSrc = &aiFilteredHorPels[0];

  Pel* pDst = rpPred;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );
  xFilterVerPels( pcTComPattern, 0, pFilteredVerSrc );
  xFilterXPels  ( pcTComPattern, 0, pFilteredXSrc   );
  aiFilteredVerPels[0] = aiFilteredHorPels[0];

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      if ( x > y )
      {
        pDst[x] = (pFilteredHorSrc[x-y-2] + 2*pFilteredHorSrc[x-y-1] + pFilteredHorSrc[x-y] + 2) >> 2;
      }
      else if ( x < y )
      {
        pDst[x] = (pFilteredVerSrc[y-x-2] + 2*pFilteredVerSrc[y-x-1] + pFilteredVerSrc[y-x] + 2) >> 2;
      }
      else
      {
        pDst[x] = (pFilteredHorSrc[0] + 2*pFilteredXSrc[0] + pFilteredVerSrc[0] + 2) >> 2;
      }
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8VertRight( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[17];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[1];
  Pel  aiFilteredVerPels[9];
  Pel* pFilteredVerSrc = &aiFilteredVerPels[1];
  Pel* pFilteredXSrc = &aiFilteredHorPels[0];

  Pel* pDst = rpPred;

  Int  zVR;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );
  xFilterVerPels( pcTComPattern, 0, pFilteredVerSrc );
  xFilterXPels  ( pcTComPattern, 0, pFilteredXSrc   );
  aiFilteredVerPels[0] = aiFilteredHorPels[0];

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      zVR = 2*x - y;
      if ( zVR >= 0 && (zVR & 0x1) == 0 )
      {
        pDst[x] = (pFilteredHorSrc[x-(y>>1)-1] + pFilteredHorSrc[x-(y>>1)] + 1) >> 1;
      }
      else if ( zVR >= 0 && (zVR & 0x1) == 1 )
      {
        pDst[x] = (pFilteredHorSrc[x-(y>>1)-2] + 2*pFilteredHorSrc[x-(y>>1)-1] + pFilteredHorSrc[x-(y>>1)] + 2) >> 2;
      }
      else if ( zVR == -1 )
      {
        pDst[x] = (pFilteredVerSrc[0] + 2*pFilteredXSrc[0] + pFilteredHorSrc[0] + 2) >> 2;
      }
      else
      {
        pDst[x] = (pFilteredVerSrc[y-2*x-1] + 2*pFilteredVerSrc[y-2*x-2] + pFilteredVerSrc[y-2*x-3] + 2) >> 2;
      }
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8HorDown( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[17];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[1];
  Pel  aiFilteredVerPels[9];
  Pel* pFilteredVerSrc = &aiFilteredVerPels[1];
  Pel* pFilteredXSrc = &aiFilteredHorPels[0];

  Pel* pDst = rpPred;

  Int  zHD;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );
  xFilterVerPels( pcTComPattern, 0, pFilteredVerSrc );
  xFilterXPels  ( pcTComPattern, 0, pFilteredXSrc   );
  aiFilteredVerPels[0] = aiFilteredHorPels[0];

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      zHD = 2*y - x;
      if ( zHD >= 0 && (zHD & 0x1) == 0 )
      {
        pDst[x] = (pFilteredVerSrc[y-(x>>1)-1] + pFilteredVerSrc[y-(x>>1)] + 1) >> 1;
      }
      else if ( zHD >= 0 && (zHD & 0x1) == 1 )
      {
        pDst[x] = (pFilteredVerSrc[y-(x>>1)-2] + 2*pFilteredVerSrc[y-(x>>1)-1] + pFilteredVerSrc[y-(x>>1)] + 2) >> 2;
      }
      else if ( zHD == -1 )
      {
        pDst[x] = (pFilteredVerSrc[0] + 2*pFilteredXSrc[0] + pFilteredHorSrc[0] + 2) >> 2;
      }
      else
      {
        pDst[x] = (pFilteredHorSrc[x-2*y-1] + 2*pFilteredHorSrc[x-2*y-2] + pFilteredHorSrc[x-2*y-3] + 2) >> 2;
      }
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8VertLeft( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredHorPels[16];
  Pel* pFilteredHorSrc = &aiFilteredHorPels[0];
  Pel* pDst = rpPred;

  xFilterHorPels( pcTComPattern, 0, pFilteredHorSrc );

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      if ( (y & 0x1) == 0 )
      {
        pDst[x] = (pFilteredHorSrc[x+(y>>1)] + pFilteredHorSrc[x+(y>>1)+1] + 1) >> 1;
      }
      else
      {
        pDst[x] = (pFilteredHorSrc[x+(y>>1)] + 2*pFilteredHorSrc[x+(y>>1)+1] + pFilteredHorSrc[x+(y>>1)+2] + 2) >> 2;
      }
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra8HorUp( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel  aiFilteredVerPels[8];
  Pel* pFilteredVerSrc = &aiFilteredVerPels[0];
  Pel* pDst = rpPred;

  Int  zHU;

  xFilterVerPels( pcTComPattern, 0, pFilteredVerSrc );

  for ( Int y = 0; y < 8; y++ )
  {
    for ( Int x = 0; x < 8; x++ )
    {
      zHU = x + 2*y;
      if ( zHU < 13 && (zHU & 0x1) == 0 )
      {
        pDst[x] = (pFilteredVerSrc[y+(x>>1)] + pFilteredVerSrc[y+(x>>1)+1] + 1) >> 1;
      }
      else if ( zHU <13 && (zHU & 0x1) == 1 )
      {
        pDst[x] = (pFilteredVerSrc[y+(x>>1)] + 2*pFilteredVerSrc[y+(x>>1)+1] + pFilteredVerSrc[y+(x>>1)+2] + 2) >> 2;
      }
      else if ( zHU == 13 )
      {
        pDst[x] = (pFilteredVerSrc[6] + 3*pFilteredVerSrc[7] + 2) >> 2;
      }
      else
      {
        pDst[x] = pFilteredVerSrc[7];
      }
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra4Vert( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  ::memcpy( pDst, pSrc - iSrcStride, sizeof(Pel)*4 );
  for ( Int i = 1; i < 4; i++ )
  {
    pDst += iStride;
    ::memcpy( pDst, pDst - iStride, sizeof(Pel)*4 );
  }

  return;
}

Void TComPrediction::xPredIntra4Hor( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  for ( Int y = 0; y < 4; y++ )
  {
    pDst[0] = pSrc[-1];
    for ( Int x = 1; x < 4; x++ )
    {
      pDst[x] = pDst[x-1];
    }
    pSrc += iSrcStride;
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra4DC( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  Int  x, y;
  Int  iDCValue = 0;

  if ( pcTComPattern->isLeftAvailable() ) // left neighboring sample is available
  {
    for ( y = 0; y < 4; y++ )
    {
      iDCValue += pSrc[-1];
      pSrc += iSrcStride;
    }

    pSrc = pcTComPattern->getROIYBlk( 0 ) - iSrcStride;
    if ( pcTComPattern->isAboveAvailable() ) // if left and above neighboring sample are available
    {
      for ( x = 0; x < 4; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += 4;
      iDCValue >>= 3;
    }
    else
    {
      iDCValue += 2;
      iDCValue >>= 2;
    }
  }
  else
  {
    pSrc = pcTComPattern->getROIYBlk( 0 ) - iSrcStride;
    if ( pcTComPattern->isAboveAvailable() ) // if above neighboring sample is available
    {
      for ( x = 0; x < 4; x++ )
      {
        iDCValue += pSrc[x];
      }

      iDCValue += 2;
      iDCValue >>= 2;
    }
    else
    {
      iDCValue = (1<<(g_uiBitDepth+g_uiBitIncrement-1));
    }
  }

  for ( y = 0; y < 4; y++ )
  {
    for ( x = 0; x < 4; x++ )
    {
      pDst[x] = iDCValue;
    }
    pDst += iStride;
  }

  return;
}

Void TComPrediction::xPredIntra4DiagDownLeft( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride;

  if ( pcTComPattern->isAboveRightAvailable() )
  {
    pDst[0] = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
    pDst[1] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
    pDst[2] = (pSrc[2] + 2*pSrc[3] + pSrc[4] + 2) >> 2;
    pDst[3] = (pSrc[3] + 2*pSrc[4] + pSrc[5] + 2) >> 2;

                                                pDst[iStride + 0] = pDst[1];
                          pDst[2*iStride + 0] = pDst[iStride + 1] = pDst[2];
    pDst[3*iStride + 0] = pDst[2*iStride + 1] = pDst[iStride + 2] = pDst[3];
    pDst[3*iStride + 1] = pDst[2*iStride + 2] = pDst[iStride + 3] = (pSrc[4] + 2*pSrc[5] + pSrc[6] + 2) >> 2;
    pDst[3*iStride + 2] = pDst[2*iStride + 3]                     = (pSrc[5] + 2*pSrc[6] + pSrc[7] + 2) >> 2;
    pDst[3*iStride + 3]                                           = (pSrc[6] + 2*pSrc[7] + pSrc[7] + 2) >> 2;
  }
  else
  {
    pDst[0] = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
    pDst[1] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
    pDst[2] = (pSrc[2] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
    pDst[3] = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;

                                                pDst[iStride + 0] = pDst[1];
                          pDst[2*iStride + 0] = pDst[iStride + 1] = pDst[2];
    pDst[3*iStride + 0] = pDst[2*iStride + 1] = pDst[iStride + 2] = pDst[3];
    pDst[3*iStride + 1] = pDst[2*iStride + 2] = pDst[iStride + 3] = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
    pDst[3*iStride + 2] = pDst[2*iStride + 3]                     = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
    pDst[3*iStride + 3]                                           = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
  }

  return;
}

Void TComPrediction::xPredIntra4DiagDownRight( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride + 1;
  pDst[0] = (pSrc[iSrcStride] + 2*pSrc[0] + pSrc[1] + 2) >> 2;
  pDst[1] = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
  pDst[2] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
  pDst[3] = (pSrc[2] + 2*pSrc[3] + pSrc[4] + 2) >> 2;

                                              pDst[iStride + 3] = pDst[2];
                        pDst[2*iStride + 3] = pDst[iStride + 2] = pDst[1];
  pDst[3*iStride + 3] = pDst[2*iStride + 2] = pDst[iStride + 1] = pDst[0];
  pDst[3*iStride + 2] = pDst[2*iStride + 1] = pDst[iStride + 0] = (pSrc[2*iSrcStride] + 2*pSrc[  iSrcStride] + pSrc[0] + 2) >> 2;;
  pDst[3*iStride + 1] = pDst[2*iStride + 0]                     = (pSrc[3*iSrcStride] + 2*pSrc[2*iSrcStride] + pSrc[iSrcStride] + 2) >> 2;
  pDst[3*iStride + 0]                                           = (pSrc[4*iSrcStride] + 2*pSrc[3*iSrcStride] + pSrc[2*iSrcStride] + 2) >> 2;

  return;
}

Void TComPrediction::xPredIntra4VertRight( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride + 1;
  pDst[0] = pDst[2*iStride + 1] = (pSrc[0] + pSrc[1] + 1) >> 1;
  pDst[1] = pDst[2*iStride + 2] = (pSrc[1] + pSrc[2] + 1) >> 1;
  pDst[2] = pDst[2*iStride + 3] = (pSrc[2] + pSrc[3] + 1) >> 1;
  pDst[3]                       = (pSrc[3] + pSrc[4] + 1) >> 1;

  pDst[iStride + 0] = pDst[3*iStride + 1] = (pSrc[iSrcStride] + 2*pSrc[0] + pSrc[1] + 2) >> 2;
  pDst[iStride + 1] = pDst[3*iStride + 2] = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
  pDst[iStride + 2] = pDst[3*iStride + 3] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
  pDst[iStride + 3]                       = (pSrc[2] + 2*pSrc[3] + pSrc[4] + 2) >> 2;

  pDst[2*iStride] = (pSrc[0] + 2*pSrc[iSrcStride] + pSrc[2*iSrcStride] + 2) >> 2;
  pDst[3*iStride] = (pSrc[iSrcStride] + 2*pSrc[2*iSrcStride] + pSrc[3*iSrcStride] + 2) >> 2;

  return;
}

Void TComPrediction::xPredIntra4HorDown( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride + 1;
  pDst[0] = pDst[iStride + 2] = (pSrc[0] + pSrc[iSrcStride] + 1) >> 1;
  pDst[1] = pDst[iStride + 3] = (pSrc[iSrcStride] + 2*pSrc[0] + pSrc[1] + 2) >> 2;
  pDst[2]                     = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
  pDst[3]                     = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;

  pDst[iStride + 1]   = pDst[2*iStride + 3]                       = (pSrc[0] + 2*pSrc[iSrcStride] + pSrc[2*iSrcStride] + 2) >> 2;
  pDst[iStride + 0]   = pDst[2*iStride + 2]                       = (pSrc[iSrcStride] + pSrc[2*iSrcStride] + 1) >> 1;
                        pDst[2*iStride + 1] = pDst[3*iStride + 3] = (pSrc[iSrcStride] + 2*pSrc[2*iSrcStride] + pSrc[3*iSrcStride] + 2) >> 2;
                        pDst[2*iStride + 0] = pDst[3*iStride + 2] = (pSrc[2*iSrcStride] + pSrc[3*iSrcStride] + 1) >> 1;
                                              pDst[3*iStride + 1] = (pSrc[2*iSrcStride] + 2*pSrc[3*iSrcStride] + pSrc[4*iSrcStride] + 2) >> 2;
                                              pDst[3*iStride + 0] = (pSrc[3*iSrcStride] + pSrc[4*iSrcStride] + 1) >> 1;

  return;
}

Void TComPrediction::xPredIntra4VertLeft( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride;

  if ( pcTComPattern->isAboveRightAvailable() )
  {
    pDst[0]                       = (pSrc[0] + pSrc[1] + 1) >> 1;
    pDst[1] = pDst[2*iStride + 0] = (pSrc[1] + pSrc[2] + 1) >> 1;
    pDst[2] = pDst[2*iStride + 1] = (pSrc[2] + pSrc[3] + 1) >> 1;
    pDst[3] = pDst[2*iStride + 2] = (pSrc[3] + pSrc[4] + 1) >> 1;
              pDst[2*iStride + 3] = (pSrc[4] + pSrc[5] + 1) >> 1;

    pDst[iStride + 0]                       = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
    pDst[iStride + 1] = pDst[3*iStride + 0] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
    pDst[iStride + 2] = pDst[3*iStride + 1] = (pSrc[2] + 2*pSrc[3] + pSrc[4] + 2) >> 2;
    pDst[iStride + 3] = pDst[3*iStride + 2] = (pSrc[3] + 2*pSrc[4] + pSrc[5] + 2) >> 2;
                        pDst[3*iStride + 3] = (pSrc[4] + 2*pSrc[5] + pSrc[6] + 2) >> 2;
  }
  else
  {
    pDst[0]                       = (pSrc[0] + pSrc[1] + 1) >> 1;
    pDst[1] = pDst[2*iStride + 0] = (pSrc[1] + pSrc[2] + 1) >> 1;
    pDst[2] = pDst[2*iStride + 1] = (pSrc[2] + pSrc[3] + 1) >> 1;
    pDst[3] = pDst[2*iStride + 2] = (pSrc[3] + pSrc[3] + 1) >> 1;
              pDst[2*iStride + 3] = (pSrc[3] + pSrc[3] + 1) >> 1;

    pDst[iStride + 0]                       = (pSrc[0] + 2*pSrc[1] + pSrc[2] + 2) >> 2;
    pDst[iStride + 1] = pDst[3*iStride + 0] = (pSrc[1] + 2*pSrc[2] + pSrc[3] + 2) >> 2;
    pDst[iStride + 2] = pDst[3*iStride + 1] = (pSrc[2] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
    pDst[iStride + 3] = pDst[3*iStride + 2] = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
                        pDst[3*iStride + 3] = (pSrc[3] + 2*pSrc[3] + pSrc[3] + 2) >> 2;
  }

  return;
}

Void TComPrediction::xPredIntra4HorUp( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride )
{
  Pel* pSrc = pcTComPattern->getROIYBlk( 0 );
  Int  iSrcStride = pcTComPattern->getPatternLStride();
  Pel* pDst = rpPred;

  pSrc -= iSrcStride + 1;
  pDst[0]                     = (pSrc[iSrcStride] + pSrc[2*iSrcStride] + 1) >> 1;
  pDst[1]                     = (pSrc[iSrcStride] + 2*pSrc[2*iSrcStride] + pSrc[3*iSrcStride] + 2) >> 2;
  pDst[2] = pDst[iStride + 0] = (pSrc[2*iSrcStride] + pSrc[3*iSrcStride] + 1) >> 1;
  pDst[3] = pDst[iStride + 1] = (pSrc[2*iSrcStride] + 2*pSrc[3*iSrcStride] + pSrc[4*iSrcStride] + 2) >> 2;
            pDst[iStride + 2] = (pSrc[3*iSrcStride] + pSrc[4*iSrcStride] + 1) >> 1;
            pDst[iStride + 3] = (pSrc[3*iSrcStride] + 2*pSrc[4*iSrcStride] + pSrc[4*iSrcStride] + 2) >> 2;

  pDst[2*iStride + 0] = pDst[iStride + 2];
  pDst[2*iStride + 1] = pDst[iStride + 3];
  pDst[2*iStride + 2] =
  pDst[2*iStride + 3] =

  pDst[3*iStride + 0] =
  pDst[3*iStride + 1] =
  pDst[3*iStride + 2] =
  pDst[3*iStride + 3] = pSrc[4*iSrcStride];

  return;
}

Void TComPrediction::motionCompensation ( TComDataCU* pcCU, TComYuv* pcYuvPred, RefPicList eRefPicList, Int iPartIdx )
{
  Int         iWidth;
  Int         iHeight;
  UInt        uiPartAddr;

  if ( iPartIdx >= 0 )
  {
    if (pcCU->getSlice()->getSPS()->getUseSHV())
    	{

      int iVirtualPartIdx;
      PartSize ePartSize = pcCU->getPartitionSize(pcCU->getFirstPartIdx(iPartIdx, 0));
      if (pcCU->isNonRectPart(iPartIdx))
      {
        int iVirtualPartIdxArray[4][3] = { {1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2} };

        for (Int i = 0; i < 3; i++ )
        {
          iVirtualPartIdx = iVirtualPartIdxArray[ ePartSize - SIZE_SHV_LT ][i];

          pcCU->getLeftTopIdxSizeVirt( iVirtualPartIdx, uiPartAddr, iWidth, iHeight );
          if ( eRefPicList != REF_PIC_LIST_X )
          {
            xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
          }
          else
          {
            xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
          }
        }
      }
      else
      {
        if (ePartSize == SIZE_SHV_LT || ePartSize == SIZE_SHV_RT || ePartSize == SIZE_SHV_LB || ePartSize == SIZE_SHV_RB )
        {
          iVirtualPartIdx = ePartSize - SIZE_SHV_LT;
        }
        else
          iVirtualPartIdx = iPartIdx;
        pcCU->getLeftTopIdxSizeVirt( iVirtualPartIdx, uiPartAddr, iWidth, iHeight );
        if ( eRefPicList != REF_PIC_LIST_X )
        {
          xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
        }
        else
        {
          xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
        }
      }
    }
    else
    {
      pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
      if ( eRefPicList != REF_PIC_LIST_X )
      {
        xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
      }
      else
      {
        xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
      }
    }
    return;
  }

  if (pcCU->getSlice()->getSPS()->getUseSHV())
  {
    for ( iPartIdx = 0; iPartIdx < pcCU->getNumVirtPartInter(); iPartIdx++ )
    {
      pcCU->getLeftTopIdxSizeVirt( iPartIdx, uiPartAddr, iWidth, iHeight );
			if ( eRefPicList != REF_PIC_LIST_X )
			{
        xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
			}
			else
			{
        xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
			}
		}
  }
  else
  {
    for ( iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++ )
    {
      pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
      pcCU->getLeftTopIdxSizeVirt( iPartIdx, uiPartAddr, iWidth, iHeight );
			if ( eRefPicList != REF_PIC_LIST_X )
			{
        xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
			}
			else
			{
        xPredInterBi	(pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
			}
		}
  }
  return;
}

Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  Int         iRefIdx     = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );           assert (iRefIdx >= 0);
  TComMv      cMv         = pcCU->getCUMvField( eRefPicList )->getMv( uiPartAddr );
  TComPicYuv* pcPicYuvRef = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();

  pcCU->clipMv(cMv);

  xPredInterLumaBlk   ( pcCU, pcPicYuvRef, uiPartAddr, &cMv, iWidth, iHeight, eRefPicList, rpcYuvPred, iPartIdx );
  xPredInterChromaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
}

Void TComPrediction::xPredInterBi ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  TComYuv* pcMbYuv;
  Int      iRefIdx[2] = {-1, -1};

  for ( Int iRefList = 0; iRefList < 2; iRefList++ )
  {
    RefPicList eRefPicList = (iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    iRefIdx[iRefList] = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );

    if ( iRefIdx[iRefList] < 0 )  { continue; }

    assert( iRefIdx[iRefList] < pcCU->getSlice()->getNumRefIdx(eRefPicList) );

    pcMbYuv = &m_acYuvPred[iRefList];
    xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcMbYuv, iPartIdx );
  }

  xWeightedAverage( pcCU, &m_acYuvPred[0], &m_acYuvPred[1], iRefIdx[0], iRefIdx[1], uiPartAddr, iWidth, iHeight, rpcYuvPred );
}

Void TComPrediction::getMvPredTemplateMatch( TComDataCU* pcCU, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred, Int iTemplSize, Int iSearchRange, Bool bFilled )
{
  Int i;
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();

  TComMv  cBestMv;
  Int     iBestIdx = 0;
  TComMv  cZeroMv;
  TComMv  cMvPred;
  UInt  uiBestDist = MAX_INT;

  UInt uiPartAddr = 0;
  UInt uiFirstPartAddr = 0;
  Int iRoiWidth, iRoiHeight;

  pcCU->getPartIndexAndSize( uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );
  if (pcCU->getSlice()->getSPS()->getUseSHV())
  {
    uiFirstPartAddr = pcCU->getFirstPartIdx( uiPartIdx, 0);
  }

  assert( pcCU->getAMVPMode(uiPartAddr) == AM_NONE || pcCU->getAMVPMode(uiPartAddr) == AM_EXPL || pcCU->getAMVPMode(uiPartAddr) == AM_IMPL );

  //-- Fill the MV Candidates
  if (!bFilled)
  {
    if (pcCU->getSlice()->getSPS()->getUseSHV())
      pcCU->fillMvpCand( uiPartIdx, uiFirstPartAddr, eRefPicList, iRefIdx, pcAMVPInfo );
    else
      pcCU->fillMvpCand( uiPartIdx, uiPartAddr, eRefPicList, iRefIdx, pcAMVPInfo );
  }

  //-- initialize Mvp index & Mvp
  iBestIdx = 0;
  cBestMv  = pcAMVPInfo->m_acMvCand[0];

  if( pcCU->getAMVPMode(uiPartAddr) == AM_NONE || (pcAMVPInfo->iN <= 1 && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    rcMvPred = cBestMv;

    if (pcCU->getSlice()->getSPS()->getUseSHV())
    {
      pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiFirstPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiFirstPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    }
    else
    {
      pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
      pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    }
    return;
  }

  if (pcCU->getAMVPMode(uiPartAddr) == AM_EXPL && bFilled) //decoder
  {
    assert(pcCU->getMVPIdx(eRefPicList,pcCU->getFirstPartIdx(uiPartIdx, 0)) >= 0);
    rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,pcCU->getFirstPartIdx(uiPartIdx, 0))];

    if (pcCU->getSlice()->getSPS()->getUseSHV())
    {
      assert(pcCU->getMVPIdx(eRefPicList,uiFirstPartAddr) >= 0);
      rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,uiFirstPartAddr)];
    }
    else
    {
      assert(pcCU->getMVPIdx(eRefPicList,uiPartAddr) >= 0);
      rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,uiPartAddr)];
    }
    return;
  }

  if (iTemplSize < 1)	iTemplSize = 1;

  if (pcCU->getAMVPMode(uiPartAddr) == AM_EXPL)
  {
    m_cTemplate.clear();
    m_cTemplateCand.clear();

    //-- Get the Template
    xGetCurrTemplate( pcCU, uiPartAddr, &m_cTemplate, iRoiWidth, iRoiHeight );

    //-- Check Minimum Dist.
    for ( i = 0 ; i < pcAMVPInfo->iN; i++)
    {
      UInt uiTmpDist;
      TComMv cTmpMv;

      xCurrTemplateMatchingProcess( pcCU, uiPartIdx, &m_cTemplate, &m_cTemplateCand, pcAMVPInfo->m_acMvCand[i], i, cTmpMv, eRefPicList, iRefIdx, iRoiWidth, iRoiHeight, iSearchRange, uiTmpDist, uiBestDist);

      if ( uiBestDist > uiTmpDist )
      {
        uiBestDist = uiTmpDist;
        cBestMv   = pcAMVPInfo->m_acMvCand[i];
        iBestIdx  = i;
      }
    }
  }
  else //(pcCU->getAMVPMode(uiPartAddr) == AM_IMPL)
  {
    m_cTemplate.clear();
    m_cTemplateCand.clear();

    //-- Get the Template
    xGetNeighTemplate( pcCU, uiPartAddr, &m_cTemplate, iTemplSize, iRoiWidth, iRoiHeight );

    //-- Check Minimum Dist.
    for ( i = 0 ; i < pcAMVPInfo->iN; i++)
    {
      UInt uiTmpDist;
      TComMv cTmpMv;

      xNeighTemplateMatchingProcess( pcCU, uiPartAddr, &m_cTemplate, &m_cTemplateCand, pcAMVPInfo->m_acMvCand[i], cTmpMv, eRefPicList, iRefIdx, iTemplSize, iRoiWidth, iRoiHeight, iSearchRange, uiTmpDist, uiBestDist);

      if ( uiBestDist > uiTmpDist )
      {
        uiBestDist = uiTmpDist;
        cBestMv   = cTmpMv;
        iBestIdx  = i;
      }
    }
  }

   if ( pcCU->getSlice()->getUseMVAC() )
    pcCU->restrictMvpAccuracy(cBestMv);

  // Setting Best MVP
  rcMvPred = cBestMv;
  if (pcCU->getSlice()->getSPS()->getUseSHV())
  {
    pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiFirstPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiFirstPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
  }
  else
  {
    pcCU->setMVPIdxSubParts( iBestIdx, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
  }
  return;
}

Void TComPrediction::xGetNeighTemplate( TComDataCU* pcCU, UInt uiPartAddr, TComYuv* pcTemplate, Int iTemplSize, Int iSizeX, Int iSizeY )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel*    pSrc        = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
  Int iSrcStride    = pcPicYuvRec->getStride();

  Pel* pDst  = pcTemplate->getLumaAddr();
  Int iDstStride     = pcTemplate->getStride();

  pSrc -= (iTemplSize + iSrcStride*iTemplSize );

  Int i;

  for( i=0; i < iTemplSize; i++ )
  {
    memcpy( pDst, pSrc, sizeof(Pel)*(iSizeX+iTemplSize) );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }

  for( i=0; i < iSizeY; i++ )
  {
    memcpy( pDst, pSrc, sizeof(Pel)*(iTemplSize) );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
}

UInt TComPrediction::xCalcDistNeighTemplate( TComYuv* pcTemplate, TComYuv* pcTemplateCand, Int iTemplSize, Int iSizeX, Int iSizeY, UInt uiThresDist )
{
  Int i,j;
  UInt uiDist = 0;
  Pel* iOrigTempl = pcTemplate->getLumaAddr();
  Pel* iPredTempl = pcTemplateCand->getLumaAddr();
  Int  iStride = pcTemplate->getStride();

  // Upper template
  for( i = 0; i < iTemplSize; i++)
  {
    for( j = 0; j < iSizeX+iTemplSize; j++ )
    {
      uiDist += abs( iOrigTempl[j] - iPredTempl[j] );

      if( uiDist > uiThresDist )
        return uiDist;
    }
    iOrigTempl += iStride;
    iPredTempl += iStride;
  }

  // Left template
  for( i = 0; i < iSizeY; i++)
  {
    for( j = 0; j < iTemplSize; j++ )
    {
      uiDist += abs( iOrigTempl[j] - iPredTempl[j] );

      if( uiDist > uiThresDist )
        return uiDist;
    }
    iOrigTempl += iStride;
    iPredTempl += iStride;
  }

  return uiDist;
}

Void TComPrediction::xNeighTemplateMatchingProcess( TComDataCU*     pcCU, UInt uiPartAddr,
                                              TComYuv* pcTemplate,
                                              TComYuv* pcTemplateCand,
                                              TComMv      cMvCand,
                                              TComMv&      rcMatchedMv,
                                              RefPicList  eRefPicList,
                                              Int         iRefIdx,
                                              Int iTemplSize, Int iSizeX, Int iSizeY,
                                              Int         iSearchRange,
                                              UInt&     uiDist,
                                              UInt     uiThresDist )
{
  TComMv cMvTmp;
  TComMv cMvBestMatch;

  //-- reset Mvp for synchronization with template pixel position
  cMvTmp.set( cMvCand.getHor() - iTemplSize*4, cMvCand.getVer() - iTemplSize*4 );
  cMvTmp.setD( cMvCand.getHorD() - iTemplSize*4, cMvCand.getVerD() - iTemplSize*4);

  cMvBestMatch = cMvTmp;

  UInt uiDistTmp = MAX_INT;
  UInt uiDistBest = uiThresDist + 1;

  Int iDelta = 0;
  Int iDir = 0;
  Int iOffset = -1;

  while ( iDelta < (iSearchRange+1) )
  {
    if (iDir)
    {
      iDelta++;
    }

    iDir = 1-iDir;

    for (Int i = (iDelta? 1 : 0); i <= iDelta; i++)
    {
      if (iDelta == 0)
      {
        iOffset = 0;
      }
      else
      {
        iOffset = ( ( iDelta % 2 ) ? 1 : -1);
      }

      if (iDir)
      {
        cMvTmp.setVer(cMvTmp.getVer() + iOffset);
      }
      else
      {
        cMvTmp.setHor(cMvTmp.getHor() + iOffset);
      }

      if ( pcCU->getSlice()->getUseMVAC() )
      {
        if (cMvTmp.getHor() % 2 || cMvTmp.getVer() % 2)
          continue;
      }

      // check boundary range for search_range
      Int iPosX = (cMvTmp.getHor()/4) + pcCU->getCUPelX();
      Int iPosY = (cMvTmp.getVer()/4) + pcCU->getCUPelY();

      Int iBoundMaxX = pcCU->getSlice()->getSPS()->getWidth()  - (iSizeX + iTemplSize +1);
      Int iBoundMaxY = pcCU->getSlice()->getSPS()->getHeight() - (iSizeY + iTemplSize +1);

      if( iPosX < 0 || iPosX > iBoundMaxX || iPosY < 0 || iPosY > iBoundMaxY )
      {
        continue;
      }

      TComSlice*  pcSlice     = pcCU->getSlice();
      TComPicYuv* pcPicYuvRef = pcSlice->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();

      // prediction pattern
      xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvTmp, iTemplSize, iSizeY+iTemplSize, eRefPicList, pcTemplateCand, 0, 0 );
      xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvTmp, iSizeX+iTemplSize, iTemplSize, eRefPicList, pcTemplateCand, 0, 0 );

      // calc distortion
      uiDistTmp = xCalcDistNeighTemplate( pcTemplate, pcTemplateCand, iTemplSize, iSizeX, iSizeY, uiThresDist );

      if (uiDistTmp < uiDistBest)
      {
        uiDistBest = uiDistTmp;
        uiThresDist = uiDistBest;
        cMvBestMatch = cMvTmp;
      }
    }
  }

  rcMatchedMv.set( cMvBestMatch.getHor() + iTemplSize*4, cMvBestMatch.getVer() + iTemplSize*4 );
  rcMatchedMv.setD( cMvBestMatch.getHorD() + iTemplSize*4, cMvBestMatch.getVerD() + iTemplSize*4 );
  uiDist = uiDistBest;
}

Void TComPrediction::xGetCurrTemplate( TComDataCU* pcCU, UInt uiPartAddr, TComYuv* pcTemplate, Int iSizeX, Int iSizeY )
{
  TComPicYuv* pcPicYuvOrg = pcCU->getPic()->getPicYuvOrg();
  Pel*        pSrc        = pcPicYuvOrg->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr );
  Int         iSrcStride  = pcPicYuvOrg->getStride();

  Pel*        pDst        = pcTemplate->getLumaAddr();
  Int         iDstStride  = pcTemplate->getStride();

  Int i;

  for( i=0; i < iSizeY; i++ )
  {
    memcpy( pDst, pSrc, sizeof(Pel)*(iSizeX) );
    pSrc += iSrcStride;
    pDst += iDstStride;
  }
}

UInt TComPrediction::xCalcDistCurrTemplate( TComYuv* pcTemplate, TComYuv* pcTemplateCand, UInt uiPartAddr, Int iSizeX, Int iSizeY, UInt uiThresDist )
{
  Int i,j;
  UInt uiDist = 0;

  Pel* iOrigTempl = pcTemplate->getLumaAddr(uiPartAddr);
  Pel* iPredTempl = pcTemplateCand->getLumaAddr(uiPartAddr);

  Int  iStride = pcTemplate->getStride();
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( i = 0; i < iSizeY; i++)
  {
    for( j = 0; j < iSizeX; j++ )
    {
      iTemp = iOrigTempl[j] - iPredTempl[j];
      uiDist += ( iTemp * iTemp ) >> uiShift;

      if( uiDist > uiThresDist )
        return uiDist;
    }
    iOrigTempl += iStride;
    iPredTempl += iStride;
  }

  return uiDist;
}

Void TComPrediction::xCurrTemplateMatchingProcess( TComDataCU*     pcCU,
                                                  UInt uiPartIdx,
                                              TComYuv* pcTemplate,
                                              TComYuv* pcTemplateCand,
                                              TComMv      cMvCand,
                                              Int iMVPIdx, //--T0430
                                              TComMv&      rcMatchedMv,
                                              RefPicList  eRefPicList,
                                              Int         iRefIdx,
                                              Int iSizeX, Int iSizeY,
                                              Int         iSearchRange,
                                              UInt&     uiDist,
                                              UInt     uiThresDist )
{
  TComMv cMvTmp;
  TComMv cMvBestMatch;

  //-- reset Mvp for synchronization with template pixel position
  cMvTmp = cMvCand;
  cMvBestMatch = cMvTmp;

  xSetPredictor(cMvCand); //--T0430

  UInt uiDistTmp  = MAX_INT;
  UInt uiDistBest = uiThresDist + 1;

  Int  iDelta		=  0;
  Int  iDir			=  0;
  Int  iOffset	= -1;
  UInt uiShift	=  0;

  while ( iDelta < (iSearchRange+1) )
  {
    if (iDir)
    {
      iDelta++;
    }

    iDir = 1-iDir;

    for (Int i = (iDelta? 1 : 0); i <= iDelta; i++)
    {
      if (iDelta == 0)
      {
        iOffset = 0;
      }
      else
      {
        iOffset = ( ( iDelta % 2 ) ? 1 : -1);
      }

      if (iDir)
      {
        cMvTmp.setVer(cMvTmp.getVer() + iOffset);
      }
      else
      {
        cMvTmp.setHor(cMvTmp.getHor() + iOffset);
      }

      if ( pcCU->getSlice()->getUseMVAC() )
      {
        if (cMvTmp.getHor() % 2 || cMvTmp.getVer() % 2)
          continue;
      }

      // check boundary range for search_range
      Int iPosX = (cMvTmp.getHor()/4) + pcCU->getCUPelX();
      Int iPosY = (cMvTmp.getVer()/4) + pcCU->getCUPelY();

      Int iBoundMaxX = pcCU->getSlice()->getSPS()->getWidth()  - (iSizeX + +1);
      Int iBoundMaxY = pcCU->getSlice()->getSPS()->getHeight() - (iSizeY + +1);

      if( iPosX < 0 || iPosX > iBoundMaxX || iPosY < 0 || iPosY > iBoundMaxY )
      {
        continue;
      }

      TComSlice*  pcSlice     = pcCU->getSlice();
      TComPicYuv* pcPicYuvRef = pcSlice->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();

      if (pcCU->isNonRectPart(uiPartIdx))
      {
        int iVirtualPartIdxArray[4][3] = { {1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2} };
        PartSize ePartSize = pcCU->getPartitionSize(0);
        uiDistTmp = 0;
        for (Int i = 0; i < 3; i++ )
        {
          UInt uiPartAddr;
          Int iVirtualPartIdx = iVirtualPartIdxArray[ ePartSize - SIZE_SHV_LT ][i];

          pcCU->getLeftTopIdxSizeVirt( iVirtualPartIdx, uiPartAddr, iSizeX, iSizeY);

          // prediction pattern
          xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvTmp, iSizeX, iSizeY, eRefPicList, pcTemplateCand, uiPartIdx );
          // calc distortion
          uiDistTmp += xCalcDistCurrTemplate( pcTemplate, pcTemplateCand, uiPartAddr, iSizeX, iSizeY, uiThresDist );
        }
      }
      else
      {
        UInt uiPartAddr = pcCU->getFirstPartIdx(uiPartIdx, 0);
        // prediction pattern
        xPredInterLumaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMvTmp, iSizeX, iSizeY, eRefPicList, pcTemplateCand, uiPartIdx, 0 );
        // calc distortion
        uiDistTmp = xCalcDistCurrTemplate( pcTemplate, pcTemplateCand, 0, iSizeX, iSizeY, uiThresDist );
      }

      uiDistTmp = xCalcRdCost(xGetMvBits(cMvTmp.getHor(), cMvTmp.getVer(), uiShift) + xGetMVPIdxBits(iMVPIdx), uiDistTmp);

      if (uiDistTmp < uiDistBest)
      {
        uiDistBest = uiDistTmp;
        uiThresDist = uiDistBest;
        cMvBestMatch = cMvTmp;
      }
    }
  }

  rcMatchedMv = cMvBestMatch;
  uiDist = uiDistBest;
}

Void  TComPrediction::xPredInterLumaBlk_NIF ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, Int iDstPartAddr )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

	if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
	{
		if (iDstPartAddr >= 0)
		{
			uiPartAddr = iDstPartAddr;
		}
	}

  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );

  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstY, piRefY, sizeof(Pel)*iWidth);
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

  //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();

  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer (piRefY - 6,  iRefStride, 1, iWidth + 12, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor (piExtY + 6,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }

  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
	  {
		  xCTI_FilterQuarter0Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
    if ( ixFrac == 3)
	  {
		  xCTI_FilterQuarter1Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
	  {
		  xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
		  xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
    if ( ixFrac == 3)
	  {
		  xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
		  xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
  }

  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
	  if( iyFrac == 1 )
	  {
		  xCTI_FilterQuarter0Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
	  if( iyFrac == 3 )
	  {
		  xCTI_FilterQuarter1Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
		  return;
	  }
  }

  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }

  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
}
//--

Void  TComPrediction:: xQuantRemainder(double alfa1, double alfa2, Int& dMVx, Int& dMVy)
{
  double MaxVal =  1.0*HAM_RANGE;
  double MinVal = -1.0*HAM_RANGE;

  if(  alfa1 > MaxVal ) alfa1 = MaxVal;
  if(  alfa1 < MinVal ) alfa1 = MinVal;
  if(  alfa2 > MaxVal ) alfa2 = MaxVal;
  if(  alfa2 < MinVal ) alfa2 = MinVal;

  if (alfa1>0) dMVx = (1.0*int (alfa1+0.5));
  else         dMVx = (1.0*int (alfa1-0.5));
  if (alfa2>0) dMVy = (1.0*int (alfa2+0.5));
  else         dMVy = (1.0*int (alfa2-0.5));
  dMVx = dMVx%HAM_ANC_ACC;
  dMVy = dMVy%HAM_ANC_ACC;
}

Void  TComPrediction:: xHAM_FilterC(Pel*  piRefY, Int iRefStride,Pel*  piDstY,Int iDstStride,
                                   Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstY, piRefY, sizeof(Pel)*iWidth);
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHorC (piRefY, iRefStride,  iWidth, iHeight, iDstStride,  piDstY, iMVxFrac );
    return;
  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVerC (piRefY, iRefStride,  iWidth, iHeight, iDstStride,  piDstY, iMVyFrac );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtY     = m_piYuvExt;

  xCTI_Filter2DVerC (piRefY - m_iDIFHalfTapC,  iRefStride,  iWidth + m_iDIFTapC, iHeight, iExtStride,  piExtY,  iMVyFrac );
  xCTI_Filter2DHorC (piExtY + m_iDIFHalfTapC,  iExtStride,  iWidth             , iHeight, iDstStride,  piDstY , iMVxFrac );
}

Void  TComPrediction::xHAM_Filter(Pel*  piRefY, Int iRefStride,Pel*  piDstY,Int iDstStride,
                                  Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstY, piRefY, sizeof(Pel)*iWidth);
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHor (piRefY, iRefStride,  iWidth, iHeight, iDstStride,  piDstY, iMVxFrac );
    return;
  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVer (piRefY, iRefStride,  iWidth, iHeight, iDstStride,  piDstY, iMVyFrac );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtY     = m_piYuvExt;

  xCTI_Filter2DVer (piRefY - m_iDIFHalfTap,  iRefStride,  iWidth + m_iDIFTap, iHeight, iExtStride,  piExtY,  iMVyFrac );
  xCTI_Filter2DHor (piExtY + m_iDIFHalfTap,  iExtStride,  iWidth            , iHeight, iDstStride,  piDstY , iMVxFrac );
}
Void  TComPrediction::xPredInterLumaBlkHMVEstimateApply ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr,
														 TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv,
														 Int iDstPartAddr)
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

  if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
  {
    if (iDstPartAddr >= 0)
    {
      uiPartAddr = iDstPartAddr;
    }
  }

  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  Int   dMVx = 0,  dMVy =0;

  piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  piRefY = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Pel* pOrg = pcCU->getPic()->getPicYuvOrg()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU());
  Int iOrgStride  = pcCU->getPic()->getPicYuvOrg()->getStride();

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtY     = m_piYuvExt;

  if( pcCU->isSkip(uiPartAddr) || !pcCU->getSlice()->getUseHAM() )
  {
    xHAM_Filter(piRefY, iRefStride, piDstY, iDstStride, iWidth, iHeight, HAM_ANC_ACC*iyFrac, HAM_ANC_ACC*ixFrac);
  }
  else
	{
#if HAM_BLACKBOX
    Int i = 0, j=0;

    Pel* GRADY = new Pel [iWidth*iHeight];
    Pel* GRADX = new Pel [iWidth*iHeight];

    Pel* pGradY = GRADY;
    Pel* pGradX = GRADX;
    Pel* piDst  = piDstY;

    Int iyFracTemp = HAM_ANC_ACC*iyFrac +1 ;
    xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +1, HAM_ANC_ACC* ixFrac);

    for (j=0;j<iHeight; j++)
    {
      memcpy(pGradY, piDst, sizeof(Pel)*iWidth);
      pGradY += iWidth;
      piDst  += iDstStride;
    }

    Int ixFracTemp = HAM_ANC_ACC*ixFrac +1 ;
    xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac, HAM_ANC_ACC* ixFrac + 1);

    piDst  = piDstY;
    for (j=0;j<iHeight; j++)
    {
      memcpy(pGradX, piDst, sizeof(Pel)*iWidth);
      pGradX += iWidth;
      piDst  += iDstStride;
    }

    xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac, HAM_ANC_ACC* ixFrac);

    double s1 = 0.0,s2 = 0.0, s3 = 0.0, s4 = 0.0, s5 = 0.0;
    Pel iGradX, iGradY;

    pGradY = GRADY;
    pGradX = GRADX;
    piDst  = piDstY;
    for( j=0;j< iHeight;j++)
    {
      for( i=0;i<iWidth;i++)
      {
        Int iTmp = pOrg[i] - piDst[i];
        iGradX = pGradX[i] - piDst[i];
        iGradY = pGradY[i] - piDst[i];

        s1+= (iTmp*iGradX);
        s2+= (iGradX*iGradX);
        s3+= (iTmp*iGradY);
        s4+= (iGradY*iGradY);
        s5+= (iGradX*iGradY);
      }
      piDst  += iDstStride;
      pOrg   += iOrgStride;
      pGradY += iWidth;
      pGradX += iWidth;
    }

    double  alfa1 = (s1*s4-s3*s5)/(s2*s4-s5*s5+0.0001);
    double  alfa2 = (s3*s2-s1*s5)/(s2*s4-s5*s5+0.0001);

    xQuantRemainder( alfa1,  alfa2,  dMVx,  dMVy);

    delete [] GRADY;
    delete [] GRADX;
#else
    Int  iDistortionBest = MAX_INT;
    Int  iDistortion = 0;  dMVx =0; dMVy = 0;

    for( Int i = -HAM_RANGE ; i <= HAM_RANGE ; i++ )
		  for( Int j = -HAM_RANGE; j <= HAM_RANGE ; j++ )
		  {
        xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +j , HAM_ANC_ACC* ixFrac + i);

        iDistortion = 0;
        for(Int k=0;k<iWidth;k++)
          for(Int m=0;m<iHeight;m++)
            iDistortion+=abs(pOrg[k+m*iOrgStride]-piDstY[k+m*iDstStride]);

        if (iDistortionBest>iDistortion)
        {
          iDistortionBest=iDistortion;
          dMVx = i;
          dMVy = j;
        }
      }
#endif
  }
  pcMv->setD(dMVx,  dMVy);
  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC*iyFrac+dMVy, HAM_ANC_ACC*ixFrac+dMVx);
}
Void  TComPrediction::xPredInterLumaBlkHMVApplyME ( Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv,
                                                   Int iWidth, Int iHeight, Pel* piOrg,Int iStrideOrg, Int dMVx, Int dMVy)
{
  Int 		iOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iSrcStride;
  Pel*		piRefY		 = piSrcY + iOffset;
  Int iRefStride = iSrcStride;

  Int 		ixFrac	= pcMv->getHor() & 0x3;
  Int 		iyFrac	= pcMv->getVer() & 0x3;
  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +dMVy  , HAM_ANC_ACC* ixFrac+dMVx);

  pcMv->setD(dMVx,  dMVy);
}
Void  TComPrediction::xPredInterLumaBlkHMVEstimateApplyME ( Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv,
															  Int iWidth, Int iHeight, Pel* piOrg,Int iStrideOrg, Int dMVx, Int dMVy)
{
	Int 		iOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iSrcStride;
	Pel*		piRefY		 = piSrcY + iOffset;
	Int iRefStride = iSrcStride;

	Int 		ixFrac	= pcMv->getHor() & 0x3;
	Int 		iyFrac	= pcMv->getVer() & 0x3;

  Int i = 0, j=0;
  Pel* GRADY = new Pel [iWidth*iHeight];
  Pel* GRADX = new Pel [iWidth*iHeight];

  Pel* pGradY = GRADY;
  Pel* pGradX = GRADX;
  Pel* piDst  = piDstY;

  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +1, HAM_ANC_ACC* ixFrac);

  for (j=0;j<iHeight; j++)
  {
    memcpy(pGradY, piDst, sizeof(Pel)*iWidth);
    pGradY += iWidth;
    piDst  += iDstStride;
  }

  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac, HAM_ANC_ACC* ixFrac + 1);

  piDst  = piDstY;
  for (j=0;j<iHeight; j++)
  {
    memcpy(pGradX, piDst, sizeof(Pel)*iWidth);
    pGradX += iWidth;
    piDst  += iDstStride;
  }

  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac , HAM_ANC_ACC* ixFrac);

	Pel* pOrg = piOrg;
	Int     iOrgStride = iStrideOrg;

  double s1 = 0.0,s2 = 0.0, s3 = 0.0, s4 = 0.0, s5 = 0.0;
  Pel iGradX, iGradY;

  pGradY = GRADY;
  pGradX = GRADX;
  piDst  = piDstY;
  for( j=0;j< iHeight;j++)
  {
   for( i=0;i<iWidth;i++)
   {
     Int iTmp = pOrg[i] - piDst[i];
     iGradX = pGradX[i] - piDst[i];
     iGradY = pGradY[i] - piDst[i];

     s1+= (iTmp*iGradX);
     s2+= (iGradX*iGradX);
     s3+= (iTmp*iGradY);
     s4+= (iGradY*iGradY);
     s5+= (iGradX*iGradY);
   }
   piDst  += iDstStride;
   pOrg   += iOrgStride;
   pGradY += iWidth;
   pGradX += iWidth;
  }

  delete [] GRADY;		 	 delete [] GRADX;


  double  alfa1=(s1*s4-s3*s5)/(s2*s4-s5*s5+0.0001);
  double  alfa2=(s3*s2-s1*s5)/(s2*s4-s5*s5+0.0001);

  xQuantRemainder( alfa1,  alfa2,  dMVx,  dMVy);

  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +dMVy  , HAM_ANC_ACC* ixFrac+dMVx);

  pcMv->setD(dMVx,  dMVy);
}

Void  TComPrediction::xPredInterLumaBlkHMVApply ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv,
                                                 Int iWidth, Int iHeight, TComYuv*& rpcYuv, Int iDstPartAddr )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

  if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
  {
    if (iDstPartAddr >= 0)
    {
      uiPartAddr = iDstPartAddr;
    }
  }

  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  Int  dMVx = 0, dMVy =0;

  if( !pcCU->isSkip(uiPartAddr) && pcCU->getSlice()->getUseHAM() )
  {
    dMVx = pcMv->getHorD();
    dMVy = pcMv->getVerD();
  }
  xHAM_Filter(piRefY, iRefStride,piDstY,iDstStride,iWidth,iHeight,HAM_ANC_ACC* iyFrac +dMVy  , HAM_ANC_ACC* ixFrac+dMVx);
}

Void  TComPrediction::xPredInterLumaBlk ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuv, Int iPartIdx, Int iDstPartAddr )
{
  if ( pcCU->getSlice()->getSPS()->getUseDIF() )
  {
    if( m_bHAM && !pcCU->getSlice()->getUseHME() )
    {
      xPredInterLumaBlkHMVEstimateApply ( pcCU, pcPicYuvRef,  uiPartAddr,  pcMv,  iWidth,  iHeight,  rpcYuv,  iDstPartAddr );
      pcCU->getCUMvField(eRefPicList)->setAllMv( *pcMv, pcCU->getPartitionSize(uiPartAddr), uiPartAddr, iPartIdx, 0 );
    }

    if( pcCU->getSlice()->getUseHAM() )
      xPredInterLumaBlkHMVApply( pcCU, pcPicYuvRef,  uiPartAddr,  pcMv,  iWidth,  iHeight,  rpcYuv,  iDstPartAddr );
    else
      xPredInterLumaBlk_NIF( pcCU, pcPicYuvRef, uiPartAddr, pcMv, iWidth, iHeight, rpcYuv, iDstPartAddr );

  	return;
  }

  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

	if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
	{
		if (iDstPartAddr >= 0)
		{
			uiPartAddr = iDstPartAddr;
		}
	}

  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );

  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstY, piRefY, sizeof(Pel)*iWidth);
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

  //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xFilterHalfHorRf1 ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xFilterHalfVerRf1 ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();

  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {
    xFilterHalfVerRf0 (piRefY - 2,  iRefStride, 1, iWidth + 5, iHeight, iExtStride, 1, piExtY );
    xFilterHalfHorRf2 (piExtY + 2,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }

  Int*  piY0;
  Int*  piY1;

  Pel*  piY0Pel;

  //  Quater-pel horizontal
  if ( iyFrac == 0 && (ixFrac == 1 || ixFrac == 3) )
  {
    xFilterHalfHorRf1 ( piRefY, iRefStride, 1, iWidth, iHeight, iExtStride, 1, piExtY );
    xFilterBilinear( piRefY + (ixFrac >> 1), iRefStride, 1, piExtY, iExtStride, 1,iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  if ( iyFrac == 2 && (ixFrac == 1 || ixFrac == 3) )
  {
    //  Half-pel interpolation + NO NORM. : vertical
    piY0Pel = piRefY - iRefStride - 3;
    piY1 = piExtY + 2 * iExtStride;
    xFilterHalfVerRf0(piY0Pel, iRefStride,     1, iWidth + 6, iHeight + 1,      4 * iExtStride, 4, piY1);

    //  Half-pel interpolation : center
    piY0 = piExtY + 2 * iExtStride + 8;
    piY1 = piY0 + 2;
    xFilterHalfHorRf2(piY0, 4 * iExtStride, 4, iWidth + 1, iHeight + 1,      4 * iExtStride, 4, piY1);

    //  Half-pel NORM. : vertical
    piY1 = piExtY + 2 * iExtStride;
    xRoundOffHalfVerRf1(iWidth + 6, iHeight + 1,			4 * iExtStride, 4, piY1);

    piY0  = piExtY + (3 + iExtStride) * 4 + iExtStride * 2;
    piY1  = piY0 + 2;
    piY0 += (4 * (ixFrac >> 1));
    xFilterBilinear( piY0, 4*iExtStride, 4, piY1, 4*iExtStride, 4, iWidth, iHeight, iDstStride, 1, piDstY );

    return;
  }

  //  Quater-pel vertical
  if ( ixFrac == 0 && (iyFrac == 1 || iyFrac == 3) )
  {
    xFilterHalfVerRf1 ( piRefY, iRefStride, 1, iWidth, iHeight, iExtStride, 1, piExtY );
    xFilterBilinear( piRefY + iRefStride * (iyFrac >> 1), iRefStride, 1, piExtY, iExtStride, 1,iWidth, iHeight, iDstStride, 1, piDstY );

    return;
  }

  if ( ixFrac == 2 && (iyFrac == 1 || iyFrac == 3) )
  {
    //  Half-pel interpolation : horizontal
    piY0Pel = piRefY - 1 - iRefStride;
    piY1    = piExtY + (2 << 2) + 2;
    xFilterHalfHorRf1         (piY0Pel, iRefStride, 1, iWidth + 1, iHeight + 2,  4 * iExtStride, 4, piY1);

    //  Half-pel interpolation + NO NORM. : vertical
    piY0Pel = piRefY - iRefStride - 3;
    piY1    = piExtY + 2 * iExtStride;
    xFilterHalfVerRf0         (piY0Pel, iRefStride,     1, iWidth + 6, iHeight + 1,      4 * iExtStride, 4, piY1);

    //  Half-pel interpolation : center
    piY0 = piExtY + 2 * iExtStride + 8;
    piY1 = piY0 + 2;
    xFilterHalfHorRf2         (piY0, 4 * iExtStride, 4, iWidth + 1, iHeight + 1,      4 * iExtStride, 4, piY1);

    piY0  = piExtY + (3 + iExtStride) * 4 + 2;
    piY1  = piY0 + 2 * iExtStride;
    piY0 += (4 * iExtStride * (iyFrac >> 1));
    xFilterBilinear( piY0, 4*iExtStride, 4, piY1, 4*iExtStride, 4, iWidth, iHeight, iDstStride, 1, piDstY );

    return;
  }

  //  Quater-pel center
  //  Half-pel interpolation : vertical
  piY0Pel = piRefY - iRefStride - 3;
  piY1 = piExtY + 2 * iExtStride;
  xFilterHalfVerRf1         (piY0Pel, iRefStride, 1, iWidth + 6, iHeight + 1,      4 * iExtStride, 4, piY1);

  //  Half-pel interpolation : horizontal
  piY0Pel = piRefY - 1 - iRefStride;
  piY1 = piExtY + (2 << 2) + 2;
  xFilterHalfHorRf1         (piY0Pel, iRefStride, 1, iWidth + 1, iHeight + 2,  4 * iExtStride, 4, piY1);

  piY0 = piExtY + (3 + iExtStride) * 4;
  piY1 = piY0 + 2*iExtStride + (ixFrac >> 1) * 4;
  piY0 = piY0 + 2            + (iyFrac >> 1) * 4 * iExtStride;
  xFilterBilinear(piY0, iExtStride*4, 4, piY1, iExtStride*4, 4, iWidth, iHeight, iDstStride, 1, piDstY);
}

Void TComPrediction::xPredInterChromaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
	Int     iRefStride  = pcPicYuvRef->getCStride();
	Int     iDstStride  = rpcYuv->getCStride();

	Int     iRefOffset  = (pcMv->getHor() >> 3) + (pcMv->getVer() >> 3) * iRefStride;

	Pel*    piRefCb     = pcPicYuvRef->getCbAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
	Pel*    piRefCr     = pcPicYuvRef->getCrAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

	Pel* piDstCb = rpcYuv->getCbAddr( uiPartAddr );
	Pel* piDstCr = rpcYuv->getCrAddr( uiPartAddr );

	Int     ixFrac  = pcMv->getHor() & 0x7;
	Int     iyFrac  = pcMv->getVer() & 0x7;

	Int     x, y;
	UInt    uiCWidth  = iWidth  >> 1;
	UInt    uiCHeight = iHeight >> 1;

	// HAM for chroma
	if ( pcCU->getSlice()->getSPS()->getDIFTapC() > 2 )
	{
		Int  dMVx = 0, dMVy =0;

		if( !pcCU->isSkip(uiPartAddr) && pcCU->getSlice()->getUseHAM() )
		{
			dMVx = pcMv->getHorD();
			dMVy = pcMv->getVerD();
		}

		//////////////////1///////////////////////
		Int iFracYapplay = HAM_ANC_ACC* iyFrac +dMVy;
		if (iFracYapplay >0) iFracYapplay = (iFracYapplay + 1)/2;
		else iFracYapplay = (iFracYapplay - 1)/2;
		Int iFracXapplay = HAM_ANC_ACC* ixFrac +dMVx;
		if (iFracXapplay >0) iFracXapplay = (iFracXapplay + 1)/2;
		else iFracXapplay = (iFracXapplay - 1)/2;

		xHAM_FilterC(piRefCb, iRefStride,piDstCb,iDstStride,uiCWidth,uiCHeight,  iFracYapplay, iFracXapplay);
		xHAM_FilterC(piRefCr, iRefStride,piDstCr,iDstStride,uiCWidth,uiCHeight,  iFracYapplay ,iFracXapplay);
		return;
	}

	// Integer point
	if ( ixFrac == 0 && iyFrac == 0 )
	{
		for ( y = 0; y < uiCHeight; y++ )
		{
			::memcpy(piDstCb, piRefCb, sizeof(Pel)*uiCWidth);
			::memcpy(piDstCr, piRefCr, sizeof(Pel)*uiCWidth);
			piDstCb += iDstStride;
			piDstCr += iDstStride;
			piRefCb += iRefStride;
			piRefCr += iRefStride;
		}
		return;
	}

	// Horizontal point
	if ( iyFrac == 0 )
	{
		Pel* piRefCbP1;
		Pel* piRefCrP1;

		if ( ixFrac == 4 )
		{
			for ( y = 0; y < uiCHeight; y++ )
			{
				piRefCbP1= piRefCb + 1;
				piRefCrP1= piRefCr + 1;

				for ( x = 0; x < uiCWidth; x++ )
				{
					piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + 1) >> 1;
					piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + 1) >> 1;
				}
				piDstCb += iDstStride;
				piDstCr += iDstStride;
				piRefCb += iRefStride;
				piRefCr += iRefStride;
			}
		}
		else
		{
			for ( y = 0; y < uiCHeight; y++ )
			{
				piRefCbP1= piRefCb + 1;
				piRefCrP1= piRefCr + 1;

				for ( x = 0; x < uiCWidth; x++ )
				{
					piDstCb[x] = ( ( piRefCb[x] << 3 ) + ixFrac * ( piRefCbP1[x] - piRefCb[x] ) + 4 ) >> 3;
					piDstCr[x] = ( ( piRefCr[x] << 3 ) + ixFrac * ( piRefCrP1[x] - piRefCr[x] ) + 4 ) >> 3;
				}
				piDstCb += iDstStride;
				piDstCr += iDstStride;
				piRefCb += iRefStride;
				piRefCr += iRefStride;
			}
		}
		return;
	}

	// Vertical point
	if ( ixFrac == 0 )
	{
		Pel* piNextRefCb;
		Pel* piNextRefCr;

		if (iyFrac == 4)
		{
			for ( y = 0; y < uiCHeight; y++ )
			{
				piNextRefCb = piRefCb + iRefStride;
				piNextRefCr = piRefCr + iRefStride;
				for ( x = 0; x < uiCWidth; x++ )
				{
					piDstCb[x] = (piRefCb[x] + piNextRefCb[x] + 1) >> 1;
					piDstCr[x] = (piRefCr[x] + piNextRefCr[x] + 1) >> 1;
				}
				piDstCb += iDstStride;
				piDstCr += iDstStride;
				piRefCb += iRefStride;
				piRefCr += iRefStride;
			}
		}
		else
		{
			for ( y = 0; y < uiCHeight; y++ )
			{
				piNextRefCb = piRefCb + iRefStride;
				piNextRefCr = piRefCr + iRefStride;
				for ( x = 0; x < uiCWidth; x++ )
				{
					piDstCb[x] = ( ( piRefCb[x] << 3 ) + iyFrac * ( piNextRefCb[x] - piRefCb[x] ) + 4) >> 3;
					piDstCr[x] = ( ( piRefCr[x] << 3 ) + iyFrac * ( piNextRefCr[x] - piRefCr[x] ) + 4) >> 3;
				}
				piDstCb += iDstStride;
				piDstCr += iDstStride;
				piRefCb += iRefStride;
				piRefCr += iRefStride;
			}
		}
		return;
	}

	// Center point
	{
		Pel* piNextRefCb;
		Pel* piNextRefCr;

		Pel* piRefCbP1;
		Pel* piNextRefCbP1;
		Pel* piRefCrP1;
		Pel* piNextRefCrP1;

		if (ixFrac == 4 && iyFrac == 4)
		{
			for ( y = 0; y < uiCHeight; y++ )
			{
				piNextRefCb = piRefCb + iRefStride;
				piNextRefCr = piRefCr + iRefStride;

				piRefCbP1= piRefCb + 1;
				piNextRefCbP1 = piNextRefCb + 1;
				piRefCrP1= piRefCr + 1;
				piNextRefCrP1 = piNextRefCr + 1;

				for ( x = 0; x < uiCWidth; x++ )
				{
					piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + piNextRefCb[x] + piNextRefCbP1[x] + 2) >> 2;
					piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + piNextRefCr[x] + piNextRefCrP1[x] + 2) >> 2;
				}
				piDstCb += iDstStride;
				piDstCr += iDstStride;
				piRefCb += iRefStride;
				piRefCr += iRefStride;
			}
			return;
		}

		Int aCb, bCb, cCb;
		Int aCr, bCr, cCr;

		for ( y = 0; y < uiCHeight; y++ )
		{
			piNextRefCb = piRefCb + iRefStride;
			piNextRefCr = piRefCr + iRefStride;

			aCb = ( piRefCb[0] << 3 ) + iyFrac * ( piNextRefCb[0] - piRefCb[0] ); aCb *= ( 8 - ixFrac );
			aCr = ( piRefCr[0] << 3 ) + iyFrac * ( piNextRefCr[0] - piRefCr[0] ); aCr *= ( 8 - ixFrac );

			piRefCbP1			= piRefCb			+ 1;
			piNextRefCbP1 = piNextRefCb + 1;
			piRefCrP1			= piRefCr			+ 1;
			piNextRefCrP1 = piNextRefCr + 1;

			for ( x = 0; x < uiCWidth; x++ )
			{
				bCb = ( piRefCbP1[x] << 3 ) + iyFrac * ( piNextRefCbP1[x] - piRefCbP1[x] );
				bCr = ( piRefCrP1[x] << 3 ) + iyFrac * ( piNextRefCrP1[x] - piRefCrP1[x] );

				cCb = ixFrac     * bCb;
				cCr = ixFrac     * bCr;

				piDstCb[x] = ( aCb + cCb + 32 ) >> 6;
				piDstCr[x] = ( aCr + cCr + 32 ) >> 6;

				aCb = (bCb<<3) - cCb;
				aCr = (bCr<<3) - cCr;
			}
			piDstCb += iDstStride;
			piDstCr += iDstStride;
			piRefCb += iRefStride;
			piRefCr += iRefStride;
		}
	}
}

Void TComPrediction::xWeightedAverage( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, Int iRefIdx0, Int iRefIdx1, UInt uiPartIdx, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst )
{
  if( iRefIdx0 >= 0 && iRefIdx1 >= 0 )
  {
    rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight );
  }
  else if ( iRefIdx0 >= 0 && iRefIdx1 <  0 )
  {
    pcYuvSrc0->copyPartToPartYuv( rpcYuvDst, uiPartIdx, iWidth, iHeight );
  }
  else if ( iRefIdx0 <  0 && iRefIdx1 >= 0 )
  {
    pcYuvSrc1->copyPartToPartYuv( rpcYuvDst, uiPartIdx, iWidth, iHeight );
  }
  else
  {
    assert (0);
  }
}

Void TComPrediction::calcMotionVector( TComDataCU*  pcCU )
{
  PartSize  ePartSize   = pcCU->getPartitionSize( 0 );
  Int       iNumPredDir = pcCU->getSlice()->isInterP() ? 1 : 2;
  UInt      uiPartAddr;

  for ( UInt uiPartIdx = 0; uiPartIdx < pcCU->getNumPartInter(); uiPartIdx++ )
  {
    uiPartAddr = pcCU->getFirstPartIdx( uiPartIdx, 0 );

    if ( pcCU->isSkip( 0 ) ) // direct
    {
      pcCU->getMvPredSkip( pcCU->getSlice()->getSliceType() == B_SLICE );
      continue;
    }

    for ( Int iRefList = 0; iRefList < iNumPredDir; iRefList++ )
    {
      RefPicList     eRefPicList = ( iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
      TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefPicList );

      Int            iRefIdx     = pcCUMvField->getRefIdx( uiPartAddr );
      TComMv         cMv( 0, 0 );

      if ( iRefIdx >= 0 )
      {
        pcCU->getMvPred( ePartSize, uiPartIdx, eRefPicList, iRefIdx, cMv );
        cMv += pcCU->getCUMvField( eRefPicList )->getMvd( uiPartAddr );
      }
      cMv.setHorD( pcCU->getCUMvField( eRefPicList )->getMv(uiPartAddr).getHorD());
      cMv.setVerD( pcCU->getCUMvField( eRefPicList )->getMv(uiPartAddr).getVerD());
      pcCUMvField->setAllMv( cMv, ePartSize, uiPartAddr, uiPartIdx, 0 );
    }
  }
  return;
}

//--



// --> CCCP
Void TComPrediction::xPelsSet(  Pel* pcDst, Pel cValue, UInt uiNum )
{
  for ( UInt ui = 0; ui < uiNum; ui++ )
  {
    pcDst[ui] = cValue;
  }
}


Void TComPrediction::xGetRecPixels( TComPattern* pcPattern, Pel* pRecSrc, Int iRecSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt )
{
  Pel* pSrc = pRecSrc;
  Pel* pDst = pDst0;

  Int uiWidth = uiWidth0;
  Int uiHeight = uiHeight0;

  Int x, y;

  if ( uiExt == 2)
  {
    {
      // inner block and right extension
      for( y = 0; y < uiHeight; y++ )
      {
        memcpy( pDst, pSrc, sizeof(Pel) * uiWidth );
        pDst[uiWidth] = pDst[uiWidth+1] = pDst[ uiWidth - 1 ];
        pDst += iDstStride;
        pSrc += iRecSrcStride;
      }

      // left extension
      if( pcPattern->isLeftAvailable() )
      {
        pSrc = pRecSrc - uiExt;
        pDst = pDst0 - uiExt;

        for( y = 0; y < uiHeight; y++ )
        {
          pDst[0] = pSrc[0];
          pDst[1] = pSrc[1];
          pDst += iDstStride;
          pSrc += iRecSrcStride;
        }
      }
      else
      {
        pSrc = pDst0;
        pDst = pDst0 - uiExt;

        for( y = 0; y < uiHeight; y++ )
        {
          pDst[0] = pDst[1] = pSrc[0];
          pDst += iDstStride;
          pSrc += iDstStride;
        }
      }

      // bottom extension
      pDst = pDst0 + uiHeight * iDstStride - uiExt;
      pSrc = pDst - iDstStride;
      uiWidth = uiWidth0 + ( uiExt << 1 );

      for( x = 0; x < uiWidth; x++ )
      {
        pDst[ x ] = pDst[ iDstStride + x ] = pSrc[ x ];
      }

      // upper extension
      if( pcPattern->isAboveAvailable() )
      {
        pSrc = pRecSrc - uiExt * iRecSrcStride;
        pDst = pDst0 - uiExt * iDstStride;

        uiWidth = uiWidth0;

        for( y = 0; y < uiExt; y++ )
        {
          memcpy( pDst, pSrc, sizeof(Pel) * uiWidth );
          pDst[uiWidth] = pDst[uiWidth+1] = pDst[ uiWidth - 1 ];
          pDst += iDstStride;
          pSrc += iRecSrcStride;
        }
      }
      else
      {
        pSrc = pDst0;
        pDst = pDst0 - uiExt * iDstStride;
        uiWidth = uiWidth0 + uiExt;

        for( x = 0; x < uiWidth; x++ )
        {
          pDst[ x ] = pDst[ iDstStride + x ] = pSrc[ x ];
        }
      }

      // upper right extension -->
      if( pcPattern->isAboveRightAvailable() )
      {
        pSrc = pRecSrc - uiExt * iRecSrcStride + uiWidth0;
        pDst = pDst0 - uiExt * iDstStride + uiWidth0;
        for( y = 0; y < uiExt; y++ )
        {
          pDst[0] = pSrc[0];
          pDst[1] = pSrc[1];
          pDst += iDstStride;
          pSrc += iRecSrcStride;
        }
      }

      // left above extension
      if( pcPattern->isAboveLeftAvailable() )
      {
        pSrc = pRecSrc - (uiExt*iRecSrcStride + uiExt);
        pDst = pDst0 - (uiExt*iDstStride + uiExt);

        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];

        pDst[iDstStride    ] = pSrc[iRecSrcStride];
        pDst[iDstStride + 1] = pSrc[iRecSrcStride+1];
      }
      else
      {
        pSrc = pDst0;
        pDst = pDst0 - (uiExt*iDstStride + uiExt);

        pDst[0] = pDst[1] = pSrc[0];
        pDst[iDstStride] = pDst[iDstStride+1] = pSrc[0];
      }
    }
  }
  else
  {
    // inner block and right extension
    for( y = 0; y < uiHeight; y++ )
    {
      memcpy( pDst, pSrc, sizeof(Pel) * uiWidth );
      xPelsSet( pDst + uiWidth, pDst[ uiWidth - 1 ], uiExt );
      pDst += iDstStride;
      pSrc += iRecSrcStride;
    }

    // left extension
    if( pcPattern->isLeftAvailable() )
    {
      pSrc = pRecSrc - uiExt;
      pDst = pDst0 - uiExt;

      for( y = 0; y < uiHeight; y++ )
      {
        memcpy( pDst, pSrc, sizeof(Pel) * uiExt );
        pDst += iDstStride;
        pSrc += iRecSrcStride;
      }
    }
    else
    {
      pSrc = pDst0;
      pDst = pDst0 - uiExt;

      for( y = 0; y < uiHeight; y++ )
      {
        xPelsSet( pDst, pSrc[0], uiExt );
        pDst += iDstStride;
        pSrc += iDstStride;
      }
    }

    // bottom extension
    pDst = pDst0 + uiHeight * iDstStride - uiExt;
    pSrc = pDst - iDstStride;
    uiWidth = uiWidth0 + ( uiExt << 1 );

    for( x = 0; x < uiWidth; x++ )
    {
      for( y = 0; y < uiExt; y++ )
        pDst[ y * iDstStride + x ] = pSrc[ x ];
    }

    // upper extension
    if( pcPattern->isAboveAvailable() )
    {
      pSrc = pRecSrc - uiExt * iRecSrcStride;
      pDst = pDst0 - uiExt * iDstStride;

      uiWidth = uiWidth0;

      for( y = 0; y < uiExt; y++ )
      {
        memcpy( pDst, pSrc, sizeof(Pel) * uiWidth );
        xPelsSet( pDst + uiWidth, pDst[ uiWidth - 1 ], uiExt );
        pDst += iDstStride;
        pSrc += iRecSrcStride;
      }
    }
    else
    {
      pSrc = pDst0;
      pDst = pDst0 - uiExt * iDstStride;
      uiWidth = uiWidth0 + uiExt;

      for( x = 0; x < uiWidth; x++ )
        for( y = 0; y < uiExt; y++ )
          pDst[ y * iDstStride + x ] = pSrc[ x ];
    }

    // upper right extension -->
    if( pcPattern->isAboveRightAvailable() )
    {
      pSrc = pRecSrc - uiExt * iRecSrcStride + uiWidth0;
      pDst = pDst0 - uiExt * iDstStride + uiWidth0;
      for( y = 0; y < uiExt; y++ )
      {
        memcpy( pDst, pSrc, sizeof( Pel ) * uiExt );
        pDst += iDstStride;
        pSrc += iRecSrcStride;
      }
    }

    // left above extension
    if( pcPattern->isAboveLeftAvailable() )
    {
      pSrc = pRecSrc - (uiExt*iRecSrcStride + uiExt);
      pDst = pDst0 - (uiExt*iDstStride + uiExt);

      for( Int i = 0; i < uiExt; i++ )
      {
        memcpy( pDst, pSrc, sizeof(Pel) * uiExt );
        pDst += iDstStride;
        pSrc += iRecSrcStride;
      }
    }
    else
    {
      pSrc = pDst0;
      pDst = pDst0 - (uiExt*iDstStride + uiExt);

      for( y = 0; y < uiExt; y++ )
      {
        xPelsSet( pDst, pSrc[0], uiExt );
        pDst += iDstStride;
      }
    }
  }
}

Void TComPrediction::xSegmentation( Pel* pSrc, Int iSrcStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt )
{
  Pel *pSrc0 = pSrc, *pSeg = m_pSegBuffer;
  Int iSegStride = m_iSegStride;

  Int x, y;
  Int iMean, iMean0, iMean1, iMeanDefault = 128<<g_uiBitIncrement;
  Int iPelNum0, iPelNum1;

  UInt uiWidth  = uiWidth0  + 2*uiExt;
  UInt uiHeight = uiHeight0 + 2*uiExt;
  UInt uiSize = uiHeight*uiWidth;

  iMean = 0;
  for ( y = 0; y < uiHeight ; y++ )
  {
    for ( x = 0 ; x < uiWidth ; x ++ )
    {
      iMean += pSrc0[x];
    }
    pSrc0 += iSrcStride;
  }
  iMean = ((iMean<<1) + uiSize)/(uiSize<<1);

  pSrc0 = pSrc;
  for ( y = 0; y < uiHeight ; y++ )
  {
    for ( x = 0 ; x < uiWidth ; x ++ )
    {
      pSeg[x] = !(pSrc0[x] <= iMean);
    }
    pSrc0 += iSrcStride;
    pSeg += iSegStride;
  }

  pSrc0 = pSrc;
  pSeg = m_pSegBuffer;
  for(Int ite = 0 ; ite < CCCP_SEG_ITE_NUM; ite++)
  {
    iMean0 = iMean1 = 0;
    iPelNum0 = iPelNum1 = 0;

    for ( y = 0; y < uiHeight; y++ )
    {
      for ( x = 0; x < uiWidth; x ++ )
      {
        if ( pSeg[x] == 0 )
        {
          iMean0 += pSrc0[x];
          iPelNum0 ++;
        }
        else
        {
          iMean1 += pSrc0[x];
        }
      }
      pSrc0 += iSrcStride;
      pSeg += iSegStride;
    }

    iPelNum1 = uiSize - iPelNum0;

    if( iPelNum0 )
      iMean0 = ( ( iMean0 << 1 ) + iPelNum0 ) / ( iPelNum0 << 1 );
    else
      iMean0 = iMeanDefault;

    if( iPelNum1 )
      iMean1 = ( ( iMean1 << 1 ) + iPelNum1 ) / ( iPelNum1 << 1 );
    else
      iMean1 = iMeanDefault;

    pSrc0 = pSrc;
    pSeg = m_pSegBuffer;
    for ( y = 0; y < uiHeight; y++ )
    {
      for ( x = 0; x < uiWidth; x ++ )
      {
        pSeg[x] = !( abs(pSrc0[x] - iMean0) </*=*/ abs(pSrc0[x] - iMean1) ); //Vadim
      }
      pSrc0 += iSrcStride;
      pSeg += iSegStride;
    }
  }// ite
}

Void TComPrediction::xSegmentation( Pel* pSrc, Int iStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt, Int iPartNum )
{
  Int i;

  if (iPartNum == 2)
  {
    xSegmentation( pSrc, iStride, uiWidth0, uiHeight0, uiExt );
    return;
  }

  Int iSegStride = m_iSegStride;

  Int x, y;
  Int *piThreshold = new Int[iPartNum+1];
  Int *piMean      = new Int[iPartNum];

  piThreshold[0]         = 0;
  piThreshold[iPartNum] = 255 << g_uiBitIncrement;

  UInt uiWidth  = uiWidth0  + 2*uiExt;
  UInt uiHeight = uiHeight0 + 2*uiExt;

  xGetThresholds( pSrc, iStride, piThreshold, piThreshold[0], piThreshold[iPartNum], 0, iPartNum, uiWidth, uiHeight );

  for( i = 0; i < iPartNum; i++ )
  {
    for ( y = 0; y < uiHeight; y++ )
    {
      for ( x = 0 ; x < uiWidth; x ++ )
      {
        Pel temp = pSrc[y*iStride + x];
        if( temp == piThreshold[0] && i == 0 )
          m_pSegBuffer[y*iSegStride + x] = 0;
        else if( temp > piThreshold[i] && temp <= piThreshold[i+1] )
          m_pSegBuffer[y*iSegStride + x] = i;
      }
    }
  }

  for(Int ite = 0 ; ite < CCCP_SEG_ITE_NUM; ite++)
  {
    for( i = 0; i < iPartNum; i++ )
    {
      Int iCount = 0, iMean = 0;

      for ( y = 0; y < uiHeight ; y++ )
      {
        for ( x = 0 ; x < uiWidth ; x ++ )
        {
          if( m_pSegBuffer[y*iSegStride + x] == i )
          {
            iMean += pSrc[y*iStride + x];
            iCount++;
          }
        }
      }

      if( iCount )
        piMean[i] = ( ( iMean << 1 ) + iCount ) / ( iCount << 1 );
      else
        piMean[i] = 128 << g_uiBitIncrement;
    } // i

    for( i = 0; i < iPartNum; i++ )
    {
      for ( y = 0; y < uiHeight; y++ )
      {
        for ( x = 0; x < uiWidth; x ++ )
        {
          Pel temp = pSrc[y*iStride + x];
          for( Int j = i + 1; j < iPartNum; j++ )
          {
            //if( i == j )
            //  continue;

            if ( abs( temp - piMean[i] ) < abs( temp - piMean[j] ) )
              m_pSegBuffer[y*iSegStride + x] = i;
            else
              m_pSegBuffer[y*iSegStride + x] = j;
          }
        }
      }
    }

  }// ite

  delete[] piThreshold;
  delete[] piMean;
}

Void TComPrediction::xGetThresholds( Pel* pSrc, Int iStride, Int* piThreshold, Int iMinPel, Int iMaxPel, Int iShift, Int iPartNum, UInt uiWidth, UInt uiHeight )
{
  Int iIdx = iPartNum >> 1;

  Int iMean = 0, iCount = 0;
  for ( Int y = 0; y < uiHeight; y++ )
  {
    for ( Int x = 0; x < uiWidth; x ++ )
    {
      Pel temp = pSrc[y*iStride + x];
      if( temp >= iMinPel && temp < iMaxPel )
      {
        iMean += temp;
        iCount++;
      }
    }
  }

  if( iCount )
    piThreshold[ iShift + iIdx ] = ( ( iMean << 1 ) + iCount ) / ( iCount << 1 );
  else
    piThreshold[ iShift + iIdx ] = ( iMinPel + iMaxPel + 1 ) >> 1;

  if( iIdx == 1 )
    return;

  xGetThresholds( pSrc, iStride, piThreshold, iMinPel, iMean, 0, iIdx, uiWidth, uiHeight );
  xGetThresholds( pSrc, iStride, piThreshold, iMean, iMaxPel, iIdx, iIdx, uiWidth, uiHeight );
}

Void TComPrediction::xGetSegmentPrediction( TComPattern* pcPattern, Pel* pSrc0, Int iSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth, UInt uiHeight, UInt uiExt0, Int iPartNum )
{
  Pel  *pSrc, *pDst;
  Pel  *pSeg;

  Int  iSegStride = m_iSegStride;
  Pel* pSeg0 = m_pSegBuffer + uiExt0 * iSegStride + uiExt0 ;

  Int i, j;

  Int piPel[16];
  Int piCount[16];

  UInt uiExt = uiExt0;

  memset( piPel,   0, sizeof( Int ) * iPartNum );
  memset( piCount, 0, sizeof( Int ) * iPartNum );

  // get pixel values for each segmentation part -->
  Int iIdx = 0;

  if (uiExt == 1)
  {
    if( pcPattern->isAboveAvailable() )
    {
      pSrc = pSrc0 - iSrcStride;
      pSeg = pSeg0 - iSegStride;

      for( j = 0; j < uiWidth; j++ )
      {
        iIdx = pSeg[j];
        piPel[iIdx] += pSrc[j];
        piCount[iIdx]++;
      }
    }

    if( pcPattern->isLeftAvailable() )
    {
      pSrc = pSrc0 - uiExt;
      pSeg = pSeg0 - uiExt;

      for( i = 0; i < uiHeight; i++ )
      {
        iIdx = pSeg[0];
        piPel[iIdx] += pSrc[0];
        piCount[iIdx]++;

        pSrc += iSrcStride;
        pSeg += iSegStride;
      }
    }

    if( pcPattern->isAboveLeftAvailable() )
    {
      pSrc = pSrc0 - iSrcStride - uiExt;
      pSeg = pSeg0 - iSegStride - uiExt;

      iIdx = pSeg[0];
      piPel[iIdx] += pSrc[0];
      piCount[iIdx]++;
    }

    if( pcPattern->isAboveRightAvailable() )
    {
      pSrc = pSrc0 - iSrcStride + uiWidth;
      pSeg = pSeg0 - iSegStride + uiWidth;

      iIdx = pSeg[0];
      piPel[iIdx] += pSrc[0];
      piCount[iIdx]++;
    }


    for( Int iNum = 0; iNum < iPartNum; iNum++ )
    {
      iIdx = piCount[iNum];
      if( iIdx )
        piPel[iNum] =  ( ( piPel[iNum] << 1 ) + iIdx ) / ( iIdx << 1 ) ;
      else
        piPel[iNum] = 128 << g_uiBitIncrement; // should be reconsidered
    } // iNum

    // <-- end of get pixel values for each segmentation part

    // get prediction -->
    uiExt = uiExt0;

    // fill entire segmentation by boundary pixel values -->
    //left above part
    pSeg = m_pSegBuffer;
    if( pcPattern->isAboveLeftAvailable() )
    {
      pSrc = pSrc0 - iSrcStride - uiExt;
      pSeg[0] = pSrc[0];
    }
    else
    {
      pSeg[0] = piPel[ pSeg[0] ];
    }

    // above part
    pSeg = m_pSegBuffer + uiExt;
    if( pcPattern->isAboveAvailable() )
    {
      pSrc = pSrc0 - iSrcStride;

      memcpy( pSeg, pSrc, sizeof(Pel) * uiWidth );
    }
    else
    {
      for( j = 0; j < uiWidth; j++ )
        pSeg[j] = piPel[ pSeg[j] ];
    }

    // above right part
    pSeg = m_pSegBuffer + uiExt + uiWidth;
    if( pcPattern->isAboveRightAvailable() )
    {
      pSrc = pSrc0 - iSrcStride + uiWidth;
      pSeg[0] = pSrc[0];
    }
    else
    {
      pSeg[0] = piPel[ pSeg[0] ];
    }

    // left part
    pSeg = pSeg0 - uiExt;
    if( pcPattern->isLeftAvailable() )
    {
      pSrc = pSrc0 - uiExt;
      for( i = 0; i < uiHeight; i++ )
      {
        pSeg[0] = pSrc[0];
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }

      pSeg[0] = piPel[ pSeg[0] ];
    }
    else
    {
      for( i = 0; i < uiHeight + uiExt; i++ )
      {
        pSeg[0] = piPel[ pSeg[0] ];
        pSeg += iSegStride;
      }
    }

    // inner, bottom and right parts
    pSeg = pSeg0;
    for( i = 0; i < uiHeight + uiExt; i++ )
    {
      for( j = 0; j < uiWidth + uiExt; j++ )
        pSeg[j] = piPel[ pSeg[j] ];
      pSeg += iSegStride;
    }
  }
  else
  {
    if( pcPattern->isAboveAvailable() )
    {
      pSrc = pSrc0 - iSrcStride * uiExt;
      pSeg = pSeg0 - iSegStride * uiExt;

      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiWidth; j++ )
        {
          iIdx = pSeg[j];
          piPel[iIdx] += pSrc[j];
          piCount[iIdx]++;
        }
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }
    }

    if( pcPattern->isLeftAvailable() )
    {
      pSrc = pSrc0 - uiExt;
      pSeg = pSeg0 - uiExt;

      for( i = 0; i < uiHeight; i++ )
      {
        for( j = 0; j < uiExt; j++ )
        {
          iIdx = pSeg[j];
          piPel[iIdx] += pSrc[j];
          piCount[iIdx]++;
        }

        pSrc += iSrcStride;
        pSeg += iSegStride;
      }
    }

    if( pcPattern->isAboveLeftAvailable() )
    {
      pSrc = pSrc0 - iSrcStride * uiExt - uiExt;
      pSeg = pSeg0 - iSegStride * uiExt - uiExt;

      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiExt; j++ )
        {
          iIdx = pSeg[j];
          piPel[iIdx] += pSrc[j];
          piCount[iIdx]++;
        }
        pSrc += iSrcStride;
        pSeg += iSegStride;
      }
    }

    if( pcPattern->isAboveRightAvailable() )
    {
      pSrc = pSrc0 - iSrcStride + uiWidth;
      pSeg = pSeg0 - iSegStride + uiWidth;

      for( j = 0; j < uiExt; j++ )
      {
        iIdx = pSeg[j];
        piPel[iIdx] += pSrc[j];
        piCount[iIdx]++;
      }
    }


    for( Int iNum = 0; iNum < iPartNum; iNum++ )
    {
      iIdx = piCount[iNum];
      if( iIdx )
        piPel[iNum] =  ( ( piPel[iNum] << 1 ) + iIdx ) / ( iIdx << 1 ) ;
      else
        piPel[iNum] = 128 << g_uiBitIncrement; // should be reconsidered
    } // iNum

    // <-- end of get pixel values for each segmentation part

    // get prediction -->

    uiExt = uiExt0;
    // fill entire segmentation by boundary pixel values -->
    //left above part
    pSeg = m_pSegBuffer;
    if( pcPattern->isAboveLeftAvailable() )
    {
      pSrc = pSrc0 - iSrcStride * uiExt - uiExt;
      for( i = 0; i < uiExt; i++ )
      {
        memcpy( pSeg, pSrc, sizeof(Pel) * uiExt );
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }
    }
    else
    {
      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiExt; j++ )
          pSeg[j] = piPel[ pSeg[j] ];
        pSeg += iSegStride;
      }
    }

    // above part
    pSeg = m_pSegBuffer + uiExt;
    if( pcPattern->isAboveAvailable() )
    {
      pSrc = pSrc0 - iSrcStride * uiExt;
      for( i = 0; i < uiExt; i++ )
      {
        memcpy( pSeg, pSrc, sizeof(Pel) * uiWidth );
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }
    }
    else
    {
      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiWidth; j++ )
          pSeg[j] = piPel[ pSeg[j] ];
        pSeg += iSegStride;
      }
    }

    // above right part
    pSeg = m_pSegBuffer + uiExt + uiWidth;
    if( pcPattern->isAboveRightAvailable() )
    {
      pSrc = pSrc0 - iSrcStride * uiExt + uiWidth;
      for( i = 0; i < uiExt; i++ )
      {
        memcpy( pSeg, pSrc, sizeof(Pel) * uiExt );
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }
    }
    else
    {
      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiExt; j++ )
          pSeg[j] = piPel[ pSeg[j] ];
        pSeg += iSegStride;
      }
    }

    // left part
    pSeg = pSeg0 - uiExt;
    if( pcPattern->isLeftAvailable() )
    {
      pSrc = pSrc0 - uiExt;
      for( i = 0; i < uiHeight; i++ )
      {
        memcpy( pSeg, pSrc, sizeof(Pel) * uiExt );
        pSeg += iSegStride;
        pSrc += iSrcStride;
      }
      for( i = 0; i < uiExt; i++ )
      {
        for( j = 0; j < uiExt; j++ )
          pSeg[j] = piPel[ pSeg[j] ];
        pSeg += iSegStride;
      }
    }
    else
    {
      for( i = 0; i < uiHeight + uiExt; i++ )
      {
        for( j = 0; j < uiExt; j++ )
          pSeg[j] = piPel[ pSeg[j] ];
        pSeg += iSegStride;
      }
    }

    // inner, bottom and right parts
    pSeg = pSeg0;
    for( i = 0; i < uiHeight + uiExt; i++ )
    {
      for( j = 0; j < uiWidth + uiExt; j++ )
        pSeg[j] = piPel[ pSeg[j] ];
      pSeg += iSegStride;
    }
  }

  pDst = pDst0;
  pSeg = pSeg0;

  for( i = 0; i < uiHeight; i++ )
  {
    for( j = 0; j < uiWidth; j++ )
    {
      // nine point averaging
      Pel* pTemp = pSeg - iSegStride + j;
      Int iTemp = 0;
      iTemp  = pTemp[-1] + pTemp[0] + pTemp[1];

      pTemp += iSegStride;
      iTemp += pTemp[-1] + pTemp[0] + pTemp[1];

      pTemp += iSegStride;
      iTemp += pTemp[-1] + pTemp[0] + pTemp[1];

      pDst[j] =  ( (iTemp<<1) + 9 ) / 18 ; // Vadim

#if CCCP_UPDATE_ONLINE
      pSeg[j] = pDst[j];
#endif
    }

    pDst += iDstStride;
    pSeg += iSegStride;
  }


  // <-- end of get prediction

}
/*
Void TComPrediction::predCCCPIntraChroma( TComPattern* pcPattern, TComYuv*& rpcPredYuv, UInt uiCWidth, UInt uiCHeight, UInt uiCExt )
{
  Int iPartNum = 2;

  UInt uiWidth  = uiCWidth << 1;
  UInt uiHeight = uiCHeight << 1;
  UInt uiExt    = uiCExt << 1;

  // fill extended chroma buffer by reconstructed pixels with best temporal CU and outside top CU data
  xGetRecPixels( pcPattern, pcPattern->getROIY(), pcPattern->getPatternLStride(), m_pLumaRecBuffer + uiExt*m_iLumaRecStride + uiExt, m_iLumaRecStride, uiWidth, uiHeight, uiExt );

  // fill chroma segmentation buffer by downnsampled reconstructed chroma
  xDownsampling( uiWidth, uiHeight, uiExt );

  // get Cb segmentation map
  xSegmentation( m_pDsLumaRecBuffer, m_iLumaRecStride/2, uiCWidth, uiCHeight, uiCExt, iPartNum );

  // copy seg buffer to upsbuffer temporally
  for ( int i = 0; i < uiCHeight + 2*uiCExt; i++)
  {
    memcpy(m_pDsLumaRecBuffer+i*m_iSegStride, m_pSegBuffer+i*m_iSegStride, sizeof(Pel)*(uiCWidth+ 2*uiCExt) );
  }

  // get Cb prediction based on segmentation
  xGetSegmentPrediction( pcPattern, pcPattern->getROICb(), pcPattern->getPatternCStride(), rpcPredYuv->getCbAddr(), rpcPredYuv->getCStride(), uiCWidth, uiCHeight, uiCExt, iPartNum );

  // get Cr segmentation map
  //  xSegmentation( m_pRecBuffer, m_iRecBufferStride, uiCWidth, uiCHeight, uiCExt, iPartNum );
  // copy back upsbuffer to seg buffer
  for ( int i = 0; i < uiCHeight + 2*uiCExt; i++)
  {
    memcpy(m_pSegBuffer+i*m_iSegStride, m_pDsLumaRecBuffer+i*m_iSegStride, sizeof(Pel)*(uiCWidth+ 2*uiCExt) );
  }

  // get Cr prediction based on segmentation
  xGetSegmentPrediction( pcPattern, pcPattern->getROICr(), pcPattern->getPatternCStride(), rpcPredYuv->getCrAddr(), rpcPredYuv->getCStride(), uiCWidth, uiCHeight, uiCExt, iPartNum );

}*/

Void TComPrediction::predCCCPIntraChroma( TComPattern* pcPattern, Pel* piSrc, UInt uiSrcStride, Pel* pPred, UInt uiPredStride, UInt uiCWidth, UInt uiCHeight, UInt uiCExt )
{
  Int iPartNum = 2;

  UInt uiWidth  = uiCWidth << 1;
  UInt uiHeight = uiCHeight << 1;
  UInt uiExt    = uiCExt << 1;

  // fill extended chroma buffer by reconstructed pixels with best temporal CU and outside top CU data
  xGetRecPixels( pcPattern, pcPattern->getROIY(), pcPattern->getPatternLStride(), m_pLumaRecBuffer + uiExt*m_iLumaRecStride + uiExt, m_iLumaRecStride, uiWidth, uiHeight, uiExt );

  // fill chroma segmentation buffer by downnsampled reconstructed chroma
  xDownsampling( uiWidth, uiHeight, uiExt );

  // get Cb segmentation map
  xSegmentation( m_pDsLumaRecBuffer, m_iLumaRecStride/2, uiCWidth, uiCHeight, uiCExt, iPartNum );

  xGetSegmentPrediction( pcPattern, piSrc, uiSrcStride, pPred, uiPredStride, uiCWidth, uiCHeight, uiCExt, iPartNum );

}

Void TComPrediction::xDownsampling( UInt uiWidth, UInt uiHeight, UInt uiExt )
{
  uiWidth  += ( uiExt << 1 );
  uiHeight += ( uiExt << 1 );

  UInt uiCWidth  = uiWidth >> 1;
  UInt uiCHeight = uiHeight >> 1;

  // horizontal downsampling
  Int  iSrcStride = m_iLumaRecStride;
  Pel* pSrc = m_pLumaRecBuffer;

  Int  iDstStride = m_iLumaRecStride/2;
  Pel* pDst = m_pHorDsLumaRecBuffer;

  for( Int i = 0; i < uiHeight; i++ )
  {
#if CCCP_DOWN_FILTER_TAP == 1
    for( Int j = 0; j < uiCWidth ; j++)
      pDst[j] = pSrc[j<<1];
#elif CCCP_DOWN_FILTER_TAP == 2
    for( Int j = 0, jj = j << 1; j < uiCWidth ; j++, jj = j << 1)
      pDst[j] = (pSrc[jj] + pSrc[jj + 1]) >> 1;
#elif CCCP_DOWN_FILTER_TAP == 4
    pDst[0] = ( 16 * pSrc[0] + 11 * pSrc[1] + 5 * pSrc[2] + 16 ) >> 5;

    for( Int j = 1, jj = j << 1; j < uiCWidth - 1; j++, jj = j << 1 )
      pDst[j] = ( 5 * pSrc[jj - 1] + 11 * pSrc[jj] + 11 * pSrc[jj + 1] + 5 * pSrc[jj + 2] + 16 ) >> 5;

    pDst[uiCWidth - 1] = ( 16 * pSrc[uiWidth - 1] + 11 * pSrc[uiWidth - 2] + 5 * pSrc[uiWidth - 3] + 16 ) >> 5;
#elif
    printf("unsupported down sample type");
    assert(0);
#endif
    pDst += iDstStride;
    pSrc += iSrcStride;
  }


  // vertical downsampling
  iSrcStride = m_iLumaRecStride/2;
  pSrc = m_pHorDsLumaRecBuffer;

  iDstStride = m_iLumaRecStride/2;
  pDst = m_pDsLumaRecBuffer;

  for( Int j = 0; j < uiCWidth; j++ )
  {
#if CCCP_DOWN_FILTER_TAP == 1
    for( Int i = 0, ii = i << 1; i < uiCHeight; i++, ii = i << 1 )
      pDst[i * iDstStride + j] = pSrc[ii * iSrcStride + j];
#elif CCCP_DOWN_FILTER_TAP == 2
    for( Int i = 0, ii = i << 1; i < uiCHeight; i++, ii = i << 1 )
      pDst[i * iDstStride + j] = (pSrc[ii * iSrcStride + j] + pSrc[(ii+1) * iSrcStride + j]) >> 1;

#elif CCCP_DOWN_FILTER_TAP == 4
    pDst[j] = ( ( 16 * pSrc[j] + 11 * pSrc[iSrcStride + j] + 5 * pSrc[2 * iSrcStride + j] + 16 ) >> 5 );

    for( Int i = 1, ii = i << 1; i < uiCHeight - 1; i++, ii = i << 1 )
      pDst[i * iDstStride + j] = ( ( 5 * pSrc[( ii - 1 ) * iSrcStride + j] + 11 * pSrc[ii * iSrcStride + j] +
      11 * pSrc[( ii + 1 ) * iSrcStride + j] + 5 * pSrc[( ii + 2 ) * iSrcStride + j] + 16 ) >> 5 );

    Int i = uiHeight - 1;
    pDst[( uiCHeight - 1 ) * iDstStride + j] = ( ( 16 * pSrc[i * iSrcStride + j] + 11 * pSrc[( i - 1 ) * iSrcStride + j] + 5 * pSrc[( i - 2 ) * iSrcStride + j] + 16 ) >> 5 );
#elif
    printf("unsupported down sample type");
    assert(0);
#endif
  }
}

// <-- CCCP

// CIP
Void TComPrediction::recIntraLumaNxNCIP( TComPattern* pcTComPattern, Pel* pPred, Pel* pResi, Pel* pReco, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAboveAvail, Bool bLeftAvail )
{
  Int*	ptrSrc;
	Int   sw, iWidth2;
  Int   x, y;
	Pel		iTemp;

	// obtain source
	ptrSrc	= pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );

	// obtain source stride
  iWidth2 = iWidth<<1;
  sw = iWidth2+1;
	ptrSrc += sw+1;

	// compute DC value for non-availability
	Int	  iDC  = 0;
	Int		iCnt = 0;

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
		iTemp = CIP_PRED( ptrSrc[-1], ptrSrc[-sw], ptrSrc[-1-sw] );
		pReco[0] = Clip( CIP_WSUM( pPred[0], iTemp, CIP_WEIGHT ) + pResi[0] );
	}
	else
	if ( bAboveAvail )
	{
		iTemp = CIP_PRED( iDC, ptrSrc[-sw], iDC );
		pReco[0] = Clip( CIP_WSUM( pPred[0], iTemp, CIP_WEIGHT ) + pResi[0] );
	}
	else
	if ( bLeftAvail )
	{
		iTemp = CIP_PRED( ptrSrc[-1], iDC, iDC );
		pReco[0] = Clip( CIP_WSUM( pPred[0], iTemp, CIP_WEIGHT ) + pResi[0] );
	}
	else
	{
		iTemp = iDC;
		pReco[0] = Clip( CIP_WSUM( pPred[0], iTemp, CIP_WEIGHT ) + pResi[0] );
	}

	// update prediction for top side
	if ( bAboveAvail )
	{
		for ( x=1; x<iWidth; x++ )
		{
			iTemp = CIP_PRED( pReco[x-1], ptrSrc[x-sw], ptrSrc[x-1-sw] );
			pReco[ x ] = Clip( CIP_WSUM( pPred[x], iTemp, CIP_WEIGHT ) + pResi[x] );
		}
	}
	else
	{
		for ( x=1; x<iWidth; x++ )
		{
			iTemp = CIP_PRED( pReco[x-1], iDC, iDC );
			pReco[ x ] = Clip( CIP_WSUM( pPred[x], iTemp, CIP_WEIGHT ) + pResi[x] );
		}
	}

	// update prediction for left side
	if ( bLeftAvail )
	{
		for ( y=1; y<iHeight; y++ )
		{
			iTemp = CIP_PRED( ptrSrc[-1+y*sw], pReco[(y-1)*uiStride], ptrSrc[-1+(y-1)*sw] );
			pReco[ y*uiStride ] = Clip( CIP_WSUM( pPred[ y*uiStride ], iTemp, CIP_WEIGHT ) + pResi[ y*uiStride ] );
		}
	}
	else
	{
		for ( y=1; y<iHeight; y++ )
		{
			iTemp = CIP_PRED( iDC, pReco[(y-1)*uiStride], iDC );
			pReco[ y*uiStride ] = Clip( CIP_WSUM( pPred[ y*uiStride ], iTemp, CIP_WEIGHT ) + pResi[ y*uiStride ] );
		}
	}

	// update prediction for inner region
	for ( y=1; y<iHeight; y++ )
	{
		for ( x=1; x<iWidth; x++ )
		{
			iTemp = CIP_PRED( pReco[ (x-1)+y*uiStride], pReco[ x+(y-1)*uiStride ], pReco[ (x-1)+(y-1)*uiStride ] );
			pReco[ x+y*uiStride ] = Clip( CIP_WSUM( pPred[ x+y*uiStride ], iTemp, CIP_WEIGHT ) + pResi[ x+y*uiStride ] );
		}
	}
}
