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

/** \file     TComPrediction.cpp
    \brief    prediction class
*/

#include <memory.h>
#include "TComPrediction.h"

// ====================================================================================================================
// Constructor / destructor / initialize
// ====================================================================================================================

TComPrediction::TComPrediction()
{
  m_piYuvExt = NULL;
}

TComPrediction::~TComPrediction()
{
  m_cYuvExt.destroy();

  delete[] m_piYuvExt;

  m_acYuvPred[0].destroy();
  m_acYuvPred[1].destroy();

  m_cYuvPredTemp.destroy();
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

    m_cYuvPredTemp.create( g_uiMaxCUWidth, g_uiMaxCUHeight );

#if HHI_INTERP_FILTER
    TComPredFilterMOMS::initTempBuff();
#endif  
  }

  m_iDIFHalfTap   = ( m_iDIFTap  >> 1 );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// ADI luma
#if HHI_AIS
Void TComPrediction::predIntraLumaAdi(TComPattern* pcTComPattern, UInt uiDirMode, Bool bSmoothing, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft )
#else
Void TComPrediction::predIntraLumaAdi(TComPattern* pcTComPattern, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft )
#endif
{
  Pel  *pDst = piPred;
  Int*  ptrSrc;

  // get intra direction
  Int   iIntraSizeIdx = g_aucConvertToBit[ iWidth ] + 1;

  assert( iIntraSizeIdx >= 1 );   //   4x  4
  assert( iIntraSizeIdx <= 6 );   // 128x128

  // get converted direction
  uiDirMode = g_aucIntraModeOrder[ iIntraSizeIdx ][ uiDirMode ];

  // get buffer
#if HHI_AIS
  UChar ucIntraFilter = g_aucIntraFilter[iIntraSizeIdx][uiDirMode];
  if ( !bSmoothing )
  {
    if ( pcCU->getSlice()->getSPS()->getUseAIS() )
      ucIntraFilter = ucIntraFilter ? 0 : 1;
    else 
      ucIntraFilter = 0;
  }
  if( ucIntraFilter == 0 )
  {
    ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
  }
  else if( ucIntraFilter == 1 )
  {
    ptrSrc = pcTComPattern->getAdiFilteredBuf1( iWidth, iHeight, m_piYuvExt );
  }
  else
  {
    ptrSrc = pcTComPattern->getAdiFilteredBuf2( iWidth, iHeight, m_piYuvExt );
  }
#else
  if( g_aucIntraFilter[iIntraSizeIdx][uiDirMode] == 0 )
  {
    ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
  }
  else if( g_aucIntraFilter[iIntraSizeIdx][uiDirMode] == 1 )
  {
    ptrSrc = pcTComPattern->getAdiFilteredBuf1( iWidth, iHeight, m_piYuvExt );
  }
  else
  {
    ptrSrc = pcTComPattern->getAdiFilteredBuf2( iWidth, iHeight, m_piYuvExt );
  }
#endif

  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;

  // for each case
  switch( uiDirMode )
  {
  case 0:  // Vertical
    {
      xPredIntraVertAdi   ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 1:  // Horizontal
    {
      xPredIntraHorAdi    ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 2:  // DC
    {
      xPredIntraDCAdi     ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, bAbove,  bLeft );
      return;
    }
  case 3:  // Plane
    {
      xPredIntraPlaneAdi  ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 32:  // bi
    {
      xPredIntraBiAdi     ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  default:  // angle
    {
      xPredIntraAngleAdi  ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode );
      break;
    }
  }
}

// ADI chroma
Void TComPrediction::predIntraChromaAdi( TComPattern* pcTComPattern, Int* piSrc, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAbove, Bool bLeft )
{
  Pel  *pDst   = piPred;
  Int*  ptrSrc = piSrc;

  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;

  // for each case
  switch( uiDirMode )
  {
  case 0:  // Vertical
    {
      xPredIntraVertAdi   ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 1:  // Horizontal
    {
      xPredIntraHorAdi    ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 2:  // DC
    {
      xPredIntraDCAdi     ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, bAbove,  bLeft );
      return;
    }
  case 3:  // Plane
    {
      xPredIntraPlaneAdi  ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  case 32:  // bi
    {
      xPredIntraBiAdi     ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
      return;
    }
  default:  // angle
    {
      xPredIntraAngleAdi  ( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode );
      break;
    }
  }
}

Void TComPrediction::xPredIntraAngleAdi( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight, UInt uiDirMode )
{
  Int   iDx      = g_aucDirDx[ uiDirMode-4 ];
  Int   iDy      = g_aucDirDy[ uiDirMode-4 ];
  Int    iDxy        = iDx*iDy;
  UInt  uixyflag = g_aucXYflg[ uiDirMode   ];
  Pel*  pDst     = rpDst;
  Int*  ptrSrc   = pSrc;
  Int    iWidth2M1    = ( iWidth  << 1 ) - 1;
  Int    iHeight2M1  = ( iHeight << 1 ) - 1;

  Int   iX, iY, iXn, iYn, iYy;
  Int    iTemp;
  Bool  bCenter = false, bCenterTemp;
  Int   i, j;

  // compute step
  if ( iDxy < 0 ) iDxy = -1;
  else            iDxy = +1;

  // for all pixels inside PU
  for( i = 0; i < iHeight; i++ )
      {
    for( j = 0; j < iWidth; j++ )
      {
        if ( iDxy < 0 )   // select context pixel based on uixyflag
        {
          if ( uixyflag == 0 )
          {
            // case xline
          iTemp = xGetContextPixel( uiDirMode, 0, i-(-1), bCenter );

          iX = Clip3( -1, iWidth2M1, j+iTemp );
            iY = -1;
          }
          else
          {
            // case yline
          iTemp = xGetContextPixel( uiDirMode, 1, j-(-1), bCenter );

            iX = -1;
          iY = Clip3( -1, iHeight2M1, i+iTemp );
          }
        }
        else  // select context pixel based on the distance
        {
        // case yline
        iTemp = xGetContextPixel( uiDirMode, 1, j-(-1), bCenterTemp );
        iYy   = i-iTemp;

          if( iYy < -1 )
          {
          // case xline
          iTemp = xGetContextPixel( uiDirMode, 0, i-(-1), bCenter );
          iX    = Clip3( -1, iWidth2M1, j-iTemp );
          iY    = -1;
          }
          else
          {
            iX = -1;
          iY=Clip3( -1, iHeight2M1, iYy );
          bCenter=bCenterTemp;
        }
        }

      // copy
        if (bCenter)
      {
        pDst[ j ] = ptrSrc[ iY*iSrcStride+iX ];
      }
      // averaging
        else
        {
          if( iY == -1 )
          {
          iXn = Clip3( -1, iWidth2M1, iX-iDxy );
          pDst[ j ] = ( ptrSrc[ iY*iSrcStride+iX ] + ptrSrc[ iY*iSrcStride+iXn ] + 1 ) >> 1;
          }
            else
        if( iX == -1 )
        {
          iYn = Clip3( -1, iHeight2M1, iY-iDxy );
          pDst[ j ] = ( ptrSrc[ iY*iSrcStride+iX ] + ptrSrc[ iYn*iSrcStride+iX ] + 1 ) >> 1;
        }
      }
    }
    pDst += iDstStride;
  }
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

__inline Int TComPrediction::xGetContextPixel( UInt uiDirMode, UInt uiXYflag, Int iTempD, Bool& bCenter )
{
  Int iDx = abs( g_aucDirDx[uiDirMode-4] );
  Int iDy = abs( g_aucDirDy[uiDirMode-4] );
  Int iTempDn;

  if (uiXYflag==0)
  {
    iTempDn=iDx*iTempD/iDy;
    bCenter = ( iDx*iTempD ) % iDy == 0;
  }
  else
  {
    iTempDn=iDy*iTempD/iDx;
    bCenter = ( iDy*iTempD ) % iDx == 0;
  }

  return iTempDn;
}

Void TComPrediction::motionCompensation ( TComDataCU* pcCU, TComYuv* pcYuvPred, RefPicList eRefPicList, Int iPartIdx )
{
  Int         iWidth;
  Int         iHeight;
  UInt        uiPartAddr;

  if ( iPartIdx >= 0 )
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
    return;
  }

  for ( iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++ )
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

#if HHI_INTERP_FILTER
Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  Int         iRefIdx     = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );           assert (iRefIdx >= 0);
  TComMv      cMv         = pcCU->getCUMvField( eRefPicList )->getMv( uiPartAddr );

  InterpFilterType ePFilt = (InterpFilterType)pcCU->getSlice()->getInterpFilterType();

  pcCU->clipMv(cMv);

  switch ( ePFilt )
  {
  case IPF_HHI_4TAP_MOMS:
  case IPF_HHI_6TAP_MOMS:
    predInterLumaBlkMOMS    ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRecFilt(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred, ePFilt );
    predInterChromaBlkMOMS  ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRecFilt(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred, ePFilt );
    break;
  default:
    xPredInterLumaBlk       ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
    xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
}
#else
Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  Int         iRefIdx     = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );           assert (iRefIdx >= 0);
  TComMv      cMv         = pcCU->getCUMvField( eRefPicList )->getMv( uiPartAddr );
  TComPicYuv* pcPicYuvRef = pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec();

  pcCU->clipMv(cMv);

  xPredInterLumaBlk   ( pcCU, pcPicYuvRef, uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  xPredInterChromaBlk ( pcCU, pcPicYuvRef, uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
}
#endif

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

Void  TComPrediction::xPredInterLumaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

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

      piRefCbP1     = piRefCb     + 1;
      piNextRefCbP1 = piNextRefCb + 1;
      piRefCrP1     = piRefCr     + 1;
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

// AMVP
Void TComPrediction::getMvPredAMVP( TComDataCU* pcCU, UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();

  if( pcCU->getAMVPMode(uiPartAddr) == AM_NONE || (pcAMVPInfo->iN <= 1 && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    rcMvPred = pcAMVPInfo->m_acMvCand[0];

    pcCU->setMVPIdxSubParts( 0, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    return;
  }

  assert(pcCU->getMVPIdx(eRefPicList,uiPartAddr) >= 0);
  rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,uiPartAddr)];
  return;
}

#if HHI_IMVP
Void TComPrediction::getMvPredIMVP( TComDataCU* pcSubCU, UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefList, Int iRefIdx, TComCUMvField* pcSubCUMvField, TComMv& rcMvPred )
{
  TComMv cZeroMv( 0, 0 );
  TComMv cMvd     = cZeroMv;

  int iMvPredY = 0;
  int iMvPredX = 0;

  cMvd = pcSubCUMvField->getMvd( uiPartAddr );
  iMvPredY = rcMvPred.getVer() + cMvd.getVer();

  pcSubCU->getMvPredXDep( eRefList, uiPartIdx, iRefIdx, iMvPredY, iMvPredX );

  rcMvPred.setHor( iMvPredX );
}
#endif

// CIP
Void TComPrediction::recIntraLumaCIP( TComPattern* pcTComPattern, Pel* pPred, Pel* pResi, Pel* pReco, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAboveAvail, Bool bLeftAvail )
{
  Int*  ptrSrc;
  Int   sw, iWidth2;
  Int   x, y;
  Pel   iTemp;

  // obtain source
  ptrSrc  = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );

  // obtain source stride
  iWidth2 = iWidth<<1;
  sw = iWidth2+1;
  ptrSrc += sw+1;

  // compute DC value for non-availability
  Int   iDC  = 0;
  Int   iCnt = 0;

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

