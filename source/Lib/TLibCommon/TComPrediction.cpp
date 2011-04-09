/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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

#if LM_CHROMA  
  if( m_pLumaRecBuffer )
    delete [] m_pLumaRecBuffer;  
#endif
}

Void TComPrediction::initTempBuff()
{
  if( m_piYuvExt == NULL )
  {
    m_iYuvExtHeight  = ((g_uiMaxCUHeight + 2) << 4);
    m_iYuvExtStride = ((g_uiMaxCUWidth  + 8) << 4);
    m_cYuvExt.create( m_iYuvExtStride, m_iYuvExtHeight );
    m_piYuvExt = new Int[ m_iYuvExtStride * m_iYuvExtHeight ];

    // new structure
    m_acYuvPred[0] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );
    m_acYuvPred[1] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );

    m_cYuvPredTemp.create( g_uiMaxCUWidth, g_uiMaxCUHeight );
  }

#if LM_CHROMA                      
  m_iLumaRecStride =  (g_uiMaxCUWidth>>1) + 1;
  m_pLumaRecBuffer = new Pel[ m_iLumaRecStride * m_iLumaRecStride ];

  for( Int i = 1; i < 66; i++ )
    m_uiaShift[i-1] = ( (1 << 15) + i/2 ) / i;
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// Function for calculating DC value of the reference samples used in Intra prediction
Pel TComPrediction::predIntraGetPredValDC( Int* pSrc, Int iSrcStride, UInt iWidth, UInt iHeight, Bool bAbove, Bool bLeft )
{
  Int iInd, iSum = 0;
  Pel pDcVal;

  if (bAbove)
  {
    for (iInd = 0;iInd < iWidth;iInd++)
      iSum += pSrc[iInd-iSrcStride];
  }
  if (bLeft)
  {
    for (iInd = 0;iInd < iHeight;iInd++)
      iSum += pSrc[iInd*iSrcStride-1];
  }

  if (bAbove && bLeft)
    pDcVal = (iSum + iWidth) / (iWidth + iHeight);
  else if (bAbove)
    pDcVal = (iSum + iWidth/2) / iWidth;
  else if (bLeft)
    pDcVal = (iSum + iHeight/2) / iHeight;
  else
    pDcVal = pSrc[-1]; // Default DC value already calculated and placed in the prediction array if no neighbors are available

  return pDcVal;
}

// Function for deriving the angular Intra predictions

/** Function for deriving the simplified angular intra predictions.
 * \param pSrc pointer to reconstructed sample array
 * \param srcStride the stride of the reconstructed sample array
 * \param rpDst reference to pointer for the prediction sample array
 * \param dstStride the stride of the prediction sample array
 * \param width the width of the block
 * \param height the height of the block
 * \param dirMode the intra prediction mode index
 * \param blkAboveAvailable boolean indication if the block above is available
 * \param blkLeftAvailable boolean indication if the block to the left is available
 *
 * This function derives the prediction samples for the angular mode based on the prediction direction indicated by
 * the prediction mode index. The prediction direction is given by the displacement of the bottom row of the block and
 * the reference row above the block in the case of vertical prediction or displacement of the rightmost column
 * of the block and reference column left from the block in the case of the horizontal prediction. The displacement
 * is signalled at 1/32 pixel accuracy. When projection of the predicted pixel falls inbetween reference samples,
 * the predicted value for the pixel is linearly interpolated from the reference samples. All reference samples are taken
 * from the extended main reference.
 */
Void TComPrediction::xPredIntraAng( Int* pSrc, Int srcStride, Pel*& rpDst, Int dstStride, UInt width, UInt height, UInt dirMode, Bool blkAboveAvailable, Bool blkLeftAvailable )
{
  Int k,l;
  Int blkSize        = width;
  Pel* pDst          = rpDst;

  // Map the mode index to main prediction direction and angle
  Bool modeDC        = dirMode == 0;
  Bool modeVer       = !modeDC && (dirMode < 18);
  Bool modeHor       = !modeDC && !modeVer;
  Int intraPredAngle = modeVer ? dirMode - 9 : modeHor ? dirMode - 25 : 0;
  Int absAng         = abs(intraPredAngle);
  Int signAng        = intraPredAngle < 0 ? -1 : 1;

  // Set bitshifts and scale the angle parameter to block size
  Int angTable[9]    = {0,    2,    5,   9,  13,  17,  21,  26,  32};
  Int invAngTable[9] = {0, 4096, 1638, 910, 630, 482, 390, 315, 256}; // (256 * 32) / Angle
  Int invAngle       = invAngTable[absAng];
  absAng             = angTable[absAng];
  intraPredAngle     = signAng * absAng;

  // Do the DC prediction
  if (modeDC)
  {
    Pel dcval = predIntraGetPredValDC(pSrc, srcStride, width, height, blkAboveAvailable, blkLeftAvailable);

    for (k=0;k<blkSize;k++)
    {
      for (l=0;l<blkSize;l++)
      {
        pDst[k*dstStride+l] = dcval;
      }
    }
  }

  // Do angular predictions
  else
  {
    Pel* refMain;
    Pel* refSide;
    Pel  refAbove[2*MAX_CU_SIZE+1];
    Pel  refLeft[2*MAX_CU_SIZE+1];

    // Initialise the Main and Left reference array.
    if (intraPredAngle < 0)
    {
      for (k=0;k<blkSize+1;k++)
      {
        refAbove[k+blkSize-1] = pSrc[k-srcStride-1];
      }
      for (k=0;k<blkSize+1;k++)
      {
        refLeft[k+blkSize-1] = pSrc[(k-1)*srcStride-1];
      }
      refMain = (modeVer ? refAbove : refLeft) + (blkSize-1);
      refSide = (modeVer ? refLeft : refAbove) + (blkSize-1);

      // Extend the Main reference to the left.
      Int invAngleSum    = 128;       // rounding for (shift by 8)
      for (k=-1; k>blkSize*intraPredAngle>>5; k--)
      {
        invAngleSum += invAngle;
        refMain[k] = refSide[invAngleSum>>8];
      }
    }
    else
    {
      for (k=0;k<2*blkSize+1;k++)
      {
        refAbove[k] = pSrc[k-srcStride-1];
      }
      for (k=0;k<2*blkSize+1;k++)
      {
        refLeft[k] = pSrc[(k-1)*srcStride-1];
      }
      refMain = modeVer ? refAbove : refLeft;
    }

    if (intraPredAngle == 0)
    {
      for (k=0;k<blkSize;k++)
      {
        for (l=0;l<blkSize;l++)
        {
          pDst[k*dstStride+l] = refMain[l+1];
        }
      }
    }
    else
    {
      Int deltaPos=0;
      Int deltaInt;
      Int deltaFract;
      Int refMainIndex;

      for (k=0;k<blkSize;k++)
      {
        deltaPos += intraPredAngle;
        deltaInt   = deltaPos >> 5;
        deltaFract = deltaPos & (32 - 1);

        if (deltaFract)
        {
          // Do linear filtering
          for (l=0;l<blkSize;l++)
          {
            refMainIndex        = l+deltaInt+1;
            pDst[k*dstStride+l] = (Pel) ( ((32-deltaFract)*refMain[refMainIndex]+deltaFract*refMain[refMainIndex+1]+16) >> 5 );
          }
        }
        else
        {
          // Just copy the integer samples
          for (l=0;l<blkSize;l++)
          {
            pDst[k*dstStride+l] = refMain[l+deltaInt+1];
          }
        }
      }
    }

    // Flip the block if this is the horizontal mode
    if (modeHor)
    {
      Pel  tmp;
      for (k=0;k<blkSize-1;k++)
      {
        for (l=k+1;l<blkSize;l++)
        {
          tmp                 = pDst[k*dstStride+l];
          pDst[k*dstStride+l] = pDst[l*dstStride+k];
          pDst[l*dstStride+k] = tmp;
        }
      }
    }
  }
}

Void TComPrediction::predIntraLumaAng(TComPattern* pcTComPattern, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft )
{
  Pel *pDst = piPred;
  Int *ptrSrc;

  // only assign variable in debug mode
#ifndef NDEBUG
  // get intra direction
  Int iIntraSizeIdx = g_aucConvertToBit[ iWidth ] + 1;

  assert( iIntraSizeIdx >= 1 ); //   4x  4
  assert( iIntraSizeIdx <= 6 ); // 128x128
  assert( iWidth == iHeight  );
#endif //NDEBUG

#if QC_MDIS
  ptrSrc = pcTComPattern->getPredictorPtr( uiDirMode, g_aucConvertToBit[ iWidth ] + 1, iWidth, iHeight, m_piYuvExt );
#else
  ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
#endif //QC_MDIS

  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;

#if ADD_PLANAR_MODE
  if ( uiDirMode == PLANAR_IDX )
  {
#if REFERENCE_SAMPLE_PADDING
    xPredIntraPlanar( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
#else
    xPredIntraPlanar( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, bAbove, bLeft );
#endif
    return;
  }
#endif

  // get converted direction
  uiDirMode = g_aucAngIntraModeOrder[ uiDirMode ];

  // Create the prediction
  xPredIntraAng( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode, bAbove,  bLeft );

#if MN_DC_PRED_FILTER
  if ((uiDirMode == 0) && pcTComPattern->getDCPredFilterFlag())
    xDCPredFiltering( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight);
#endif
}

// Angular chroma
Void TComPrediction::predIntraChromaAng( TComPattern* pcTComPattern, Int* piSrc, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAbove, Bool bLeft )
{
  Pel *pDst = piPred;
  Int *ptrSrc = piSrc;

  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;

#if ADD_PLANAR_MODE
  if ( uiDirMode == PLANAR_IDX )
  {
#if REFERENCE_SAMPLE_PADDING
    xPredIntraPlanar( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight );
#else
    xPredIntraPlanar( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, bAbove, bLeft );
#endif
    return;
  }
#endif

  // get converted direction
  uiDirMode = g_aucAngIntraModeOrder[ uiDirMode ];

  // Create the prediction
  xPredIntraAng( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode, bAbove,  bLeft );
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

#if HIGH_ACCURACY_BI
Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx, Bool bi )
#else
Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx )
#endif
{
  Int         iRefIdx     = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );           assert (iRefIdx >= 0);
  TComMv      cMv         = pcCU->getCUMvField( eRefPicList )->getMv( uiPartAddr );
  pcCU->clipMv(cMv);
#if HIGH_ACCURACY_BI
  if(!bi)
  {
    xPredInterLumaBlk ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
  else
  {
    xPredInterLumaBlk_ha  ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
#else
  xPredInterLumaBlk       ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
#endif

#if HIGH_ACCURACY_BI
  if (!bi)
  {
    xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
  else
  {
    xPredInterChromaBlk_ha ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
#else
  xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
#endif
}

Void TComPrediction::xPredInterBi ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  TComYuv* pcMbYuv;
  Int      iRefIdx[2] = {-1, -1};

  for ( Int iRefList = 0; iRefList < 2; iRefList++ )
  {
    RefPicList eRefPicList = (iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    iRefIdx[iRefList] = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );

    if ( iRefIdx[iRefList] < 0 )
    {
      continue;
    }

    assert( iRefIdx[iRefList] < pcCU->getSlice()->getNumRefIdx(eRefPicList) );

    pcMbYuv = &m_acYuvPred[iRefList];
#if HIGH_ACCURACY_BI
    if( pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiPartAddr ) >= 0 && pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiPartAddr ) >= 0 )
      xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcMbYuv, iPartIdx, true );
    else
      xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcMbYuv, iPartIdx );
#else
    xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcMbYuv, iPartIdx );
#endif
  }

  xWeightedAverage( pcCU, &m_acYuvPred[0], &m_acYuvPred[1], iRefIdx[0], iRefIdx[1], uiPartAddr, iWidth, iHeight, rpcYuvPred );
}

#if HIGH_ACCURACY_BI

Void  TComPrediction::xPredInterLumaBlk_ha( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();

  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;

  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
    UInt shiftNum = 14-g_uiBitDepth-g_uiBitIncrement;
  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      for(Int x=0; x<iWidth; x++)
        piDstY[x] = piRefY[x]<<shiftNum;
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

  //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor_ha ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer_ha ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();

  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }

  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY -3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }

  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }

  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY -3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }

  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
}

#endif

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

    xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
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
      xCTI_FilterHalfVer (piRefY -3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
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
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY -3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }

  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
}

#if HIGH_ACCURACY_BI
Void TComPrediction::xPredInterChromaBlk_ha( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
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
  UInt    uiCWidth  = iWidth  >> 1;
  UInt    uiCHeight = iHeight >> 1;

  xDCTIF_FilterC_ha(piRefCb, iRefStride,piDstCb,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  xDCTIF_FilterC_ha(piRefCr, iRefStride,piDstCr,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  return;
}
#endif

//--
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
  UInt    uiCWidth  = iWidth  >> 1;
  UInt    uiCHeight = iHeight >> 1;

  xDCTIF_FilterC(piRefCb, iRefStride,piDstCb,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  xDCTIF_FilterC(piRefCr, iRefStride,piDstCr,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  return;
}

Void  TComPrediction::xDCTIF_FilterC ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,
                                       Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  // Integer point
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstC, piRefC, sizeof(Pel)*iWidth);
      piDstC += iDstStride;
      piRefC += iRefStride;
    }
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHorC (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVxFrac );
    return;
  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVerC (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVyFrac );
    return;
}

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtC     = m_piYuvExt;

  xCTI_Filter2DVerC (piRefC - 1,  iRefStride,  iWidth + 3, iHeight, iExtStride,  piExtC, iMVyFrac );
  xCTI_Filter2DHorC (piExtC + 1,  iExtStride,  iWidth             , iHeight, iDstStride,  piDstC, iMVxFrac );
}

#if HIGH_ACCURACY_BI

Void  TComPrediction::xDCTIF_FilterC_ha ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,
                                       Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  UInt    shiftNumOrg = 6 - g_uiBitIncrement + 8 - g_uiBitDepth;
  // Integer point
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for (Int y = 0; y < iHeight; y++ )
    {
      for(Int x=0; x<iWidth; x++)
      {
        piDstC[x] = (piRefC[x]<<shiftNumOrg);
      }
      piDstC += iDstStride;
      piRefC += iRefStride;
    }
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHorC_ha (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVxFrac );
    return;

  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVerC_ha (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVyFrac );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtC     = m_piYuvExt;

  xCTI_Filter2DVerC (piRefC - 1,  iRefStride,  iWidth + 3, iHeight, iExtStride,  piExtC, iMVyFrac );
  xCTI_Filter2DHorC_ha (piExtC + 1,  iExtStride,  iWidth , iHeight, iDstStride,  piDstC, iMVxFrac );
  return;

}

#endif


Void TComPrediction::xWeightedAverage( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, Int iRefIdx0, Int iRefIdx1, UInt uiPartIdx, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst )
{
  if( iRefIdx0 >= 0 && iRefIdx1 >= 0 )
  {
#ifdef ROUNDING_CONTROL_BIPRED
    rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight, pcCU->getSlice()->isRounding());
#else
    rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight );
#endif
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

#if ADD_PLANAR_MODE
/** Function for deriving planar intra prediction.
 * \param pSrc pointer to reconstructed sample array
 * \param srcStride the stride of the reconstructed sample array
 * \param rpDst reference to pointer for the prediction sample array
 * \param dstStride the stride of the prediction sample array
 * \param width the width of the block
 * \param height the height of the block
 * \param blkAboveAvailable boolean indication if the block above is available
 * \param blkLeftAvailable boolean indication if the block to the left is available
 *
 * This function derives the prediction samples for planar mode (intra coding).
 */
#if REFERENCE_SAMPLE_PADDING
Void TComPrediction::xPredIntraPlanar( Int* pSrc, Int srcStride, Pel*& rpDst, Int dstStride, UInt width, UInt height )
#else
Void TComPrediction::xPredIntraPlanar( Int* pSrc, Int srcStride, Pel*& rpDst, Int dstStride, UInt width, UInt height, Bool blkAboveAvailable, Bool blkLeftAvailable )
#endif
{
  assert(width == height);

  Int k, l, bottomLeft, topRight;
  Int horPred;
  Int leftColumn[MAX_CU_SIZE], topRow[MAX_CU_SIZE], bottomRow[MAX_CU_SIZE], rightColumn[MAX_CU_SIZE];
  UInt blkSize = width;
  UInt offset2D = width;
  UInt shift1D = g_aucConvertToBit[ width ] + 2;
  UInt shift2D = shift1D + 1;

  // Get left and above reference column and row
#if REFERENCE_SAMPLE_PADDING
  for(k=0;k<blkSize;k++)
  {
    topRow[k] = pSrc[k-srcStride];
    leftColumn[k] = pSrc[k*srcStride-1];
  }
#else
  if (!blkAboveAvailable && !blkLeftAvailable)
  {
    for(k=0;k<blkSize;k++)
    {
      leftColumn[k] = topRow[k] = ( 1 << ( g_uiBitDepth + g_uiBitIncrement - 1 ) );
    }
  }
  else
  {
    if(blkAboveAvailable)
    {
      for(k=0;k<blkSize;k++)
      {
        topRow[k] = pSrc[k-srcStride];
      }
    }
    else
    {
      Int leftSample = pSrc[-1];
      for(k=0;k<blkSize;k++)
      {
        topRow[k] = leftSample;
      }
    }
    if(blkLeftAvailable)
    {
      for(k=0;k<blkSize;k++)
      {
        leftColumn[k] = pSrc[k*srcStride-1];
      }
    }
    else
    {
      Int aboveSample = pSrc[-srcStride];
      for(k=0;k<blkSize;k++)
      {
        leftColumn[k] = aboveSample;
      }
    }
  }
#endif

  // Prepare intermediate variables used in interpolation
  bottomLeft = leftColumn[blkSize-1];
  topRight   = topRow[blkSize-1];
  for (k=0;k<blkSize;k++)
  {
    bottomRow[k]   = bottomLeft - topRow[k];
    rightColumn[k] = topRight   - leftColumn[k];
    topRow[k]      <<= shift1D;
    leftColumn[k]  <<= shift1D;
  }

  // Generate prediction signal
  for (k=0;k<blkSize;k++)
  {
    horPred = leftColumn[k] + offset2D;
    for (l=0;l<blkSize;l++)
    {
      horPred += rightColumn[k];
      topRow[l] += bottomRow[l];
      rpDst[k*dstStride+l] = ( (horPred + topRow[l]) >> shift2D );
    }
  }
}
#endif

#if LM_CHROMA
/** Function for deriving chroma LM intra prediction.
 * \param pcPattern pointer to neighbouring pixel access pattern
 * \param pSrc pointer to reconstructed chroma sample array
 * \param pPred pointer for the prediction sample array
 * \param uiPredStride the stride of the prediction sample array
 * \param uiCWidth the width of the chroma block
 * \param uiCHeight the height of the chroma block
 * \param uiChromaId boolean indication of chroma component

 \ This function derives the prediction samples for chroma LM mode (chroma intra coding)
 */
Void TComPrediction::predLMIntraChroma( TComPattern* pcPattern, Int* piSrc, Pel* pPred, UInt uiPredStride, UInt uiCWidth, UInt uiCHeight, UInt uiChromaId )
{
  UInt uiWidth  = uiCWidth << 1;
  UInt uiHeight = uiCHeight << 1;

  if (uiChromaId == 0)
    xGetRecPixels( pcPattern, pcPattern->getROIY(), pcPattern->getPatternLStride(), m_pLumaRecBuffer + m_iLumaRecStride + 1, m_iLumaRecStride, uiWidth, uiHeight );

  xGetLLSPrediction( pcPattern, piSrc+uiWidth+2, uiWidth+1, pPred, uiPredStride, uiCWidth, uiCHeight, 1 );  
}

/** Function for deriving downsampled luma sample of current chroma block and its above, left causal pixel
 * \param pcPattern pointer to neighbouring pixel access pattern
 * \param pRecSrc pointer to reconstructed luma sample array
 * \param iRecSrcStride the stride of reconstructed luma sample array
 * \param pDst0 pointer to downsampled luma sample array
 * \param iDstStride the stride of downsampled luma sample array
 * \param uiWidth0 the width of the luma block
 * \param uiHeight0 the height of the luma block

 \ This function derives downsampled luma sample of current chroma block and its above, left causal pixel
 */

Void TComPrediction::xGetRecPixels( TComPattern* pcPattern, Pel* pRecSrc, Int iRecSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth0, UInt uiHeight0 )
{
  Pel* pSrc = pRecSrc;
  Pel* pDst = pDst0;

  Int uiCWidth = uiWidth0/2;
  Int uiCHeight = uiHeight0/2;

  if( pcPattern->isLeftAvailable() )
  {
    pSrc = pSrc - 2;
    pDst = pDst - 1;

    uiCWidth += 1;
  }

  if( pcPattern->isAboveAvailable() )
  {
    pSrc = pSrc - 2*iRecSrcStride;
    pDst = pDst - iDstStride;

    uiCHeight += 1;
  }

  for( Int j = 0; j < uiCHeight; j++ )
    {
      for( Int i = 0, ii = i << 1; i < uiCWidth; i++, ii = i << 1 )
        pDst[i] = (pSrc[ii] + pSrc[ii + iRecSrcStride]) >> 1;

      pDst += iDstStride;
      pSrc += iRecSrcStride*2;
    }  
}

/** Function for deriving the positon of first non-zero binary bit of a value
 * \param x input value
 \ This function derives the positon of first non-zero binary bit of a value
 */
Int GetMSB( UInt x )
{
#if 1
  Int iMSB = 0, bits = ( sizeof( Int ) << 3 ), y = 1;

  while( x > 1 )
  {
    bits >>= 1;
    y = x >> bits;

    if( y )
    {
      x = y;
      iMSB += bits;
    }
  }

  iMSB+=y;

#else

  Int iMSB = 0;
  while( x > 0 )
  {
    x >>= 1;
    iMSB++;
  }
#endif

  return iMSB;
}

/** Function for deriving LM intra prediction.
 * \param pcPattern pointer to neighbouring pixel access pattern
 * \param pSrc0 pointer to reconstructed chroma sample array
 * \param iSrcStride the stride of reconstructed chroma sample array
 * \param pDst0 reference to pointer for the prediction sample array
 * \param iDstStride the stride of the prediction sample array
 * \param uiWidth the width of the chroma block
 * \param uiHeight the height of the chroma block
 * \param uiExt0 line number of neiggboirng pixels for calculating LM model parameter, default value is 1

 \ This function derives the prediction samples for chroma LM mode (chroma intra coding)
 */
Void TComPrediction::xGetLLSPrediction( TComPattern* pcPattern, Int* pSrc0, Int iSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth, UInt uiHeight, UInt uiExt0 )
{

  Pel  *pDst, *pLuma;
  Int  *pSrc;

  Int  iLumaStride = m_iLumaRecStride;
  Pel* pLuma0 = m_pLumaRecBuffer + uiExt0 * iLumaStride + uiExt0;

  Int i, j, iCountShift = 0;

  UInt uiExt = uiExt0;

  // LLS parameters estimation -->

  Int x = 0, y = 0, xx = 0, xy = 0;

  if( pcPattern->isAboveAvailable() )
  {
    pSrc  = pSrc0  - iSrcStride;
    pLuma = pLuma0 - iLumaStride;

    for( j = 0; j < uiWidth; j++ )
    {
      x += pLuma[j];
      y += pSrc[j];
      xx += pLuma[j] * pLuma[j];
      xy += pLuma[j] * pSrc[j];
    }
    iCountShift += g_aucConvertToBit[ uiWidth ] + 2;
  }

  if( pcPattern->isLeftAvailable() )
  {
    pSrc  = pSrc0 - uiExt;
    pLuma = pLuma0 - uiExt;

    for( i = 0; i < uiHeight; i++ )
    {
      x += pLuma[0];
      y += pSrc[0];
      xx += pLuma[0] * pLuma[0];
      xy += pLuma[0] * pSrc[0];

      pSrc  += iSrcStride;
      pLuma += iLumaStride;
    }
    iCountShift += iCountShift > 0 ? 1 : ( g_aucConvertToBit[ uiWidth ] + 2 );
  }

  Int iBitdepth = ( ( g_uiBitDepth + g_uiBitIncrement ) + g_aucConvertToBit[ uiWidth ] + 3 ) * 2;
  Int iTempShift = Max( ( iBitdepth - 31 + 1) / 2, 0);

  if(iTempShift > 0)
  {
    x  = ( x +  ( 1 << ( iTempShift - 1 ) ) ) >> iTempShift;
    y  = ( y +  ( 1 << ( iTempShift - 1 ) ) ) >> iTempShift;
    xx = ( xx + ( 1 << ( iTempShift - 1 ) ) ) >> iTempShift;
    xy = ( xy + ( 1 << ( iTempShift - 1 ) ) ) >> iTempShift;
    iCountShift -= iTempShift;
  }

  Int a, b, iShift = 13;

  if( iCountShift == 0 )
  {
    a = 0;
    b = 128 << g_uiBitIncrement;
    iShift = 0;
  }
  else
  {
    Int a1 = ( xy << iCountShift ) - y * x;
    Int a2 = ( xx << iCountShift ) - x * x;              

    if( a2 == 0 || a1 == 0 )
    {
      a = 0;
      b = ( y + ( 1 << ( iCountShift - 1 ) ) )>> iCountShift;
      iShift = 0;
    }
    else
    {
      const Int iShiftA2 = 6;
      const Int iShiftA1 = 15;
      const Int iAccuracyShift = 15;

      Int iScaleShiftA2 = 0;
      Int iScaleShiftA1 = 0;
      Int a1s = a1;
      Int a2s = a2;

      iScaleShiftA1 = GetMSB( abs( a1 ) ) - iShiftA1;
      iScaleShiftA2 = GetMSB( abs( a2 ) ) - iShiftA2;  

      if( iScaleShiftA1 < 0 )
        iScaleShiftA1 = 0;

      if( iScaleShiftA2 < 0 )
        iScaleShiftA2 = 0;

      Int iScaleShiftA = iScaleShiftA2 + iAccuracyShift - iShift - iScaleShiftA1;

      a2s = a2 >> iScaleShiftA2;

      a1s = a1 >> iScaleShiftA1;

      a = a1s * m_uiaShift[ abs( a2s ) ];
      
      if( iScaleShiftA < 0 )
        a = a << -iScaleShiftA;
      else
        a = a >> iScaleShiftA;

      if( a > ( 1 << 15 ) - 1 )
        a = ( 1 << 15 ) - 1;
      else if( a < -( 1 << 15 ) )
        a = -( 1 << 15 );

      b = (  y - ( ( a * x ) >> iShift ) + ( 1 << ( iCountShift - 1 ) ) ) >> iCountShift;
    }
  }   

  // <-- end of LLS parameters estimation

  // get prediction -->
  uiExt = uiExt0;
  pLuma = pLuma0;
  pDst = pDst0;

  for( i = 0; i < uiHeight; i++ )
  {
    for( j = 0; j < uiWidth; j++ )
      pDst[j] = Clip( ( ( a * pLuma[j] ) >> iShift ) + b );

    pDst  += iDstStride;
    pLuma += iLumaStride;
  }
  // <-- end of get prediction

}
#endif

#if MN_DC_PRED_FILTER
/** Function for filtering intra DC predictor.
 * \param pSrc pointer to reconstructed sample array
 * \param iSrcStride the stride of the reconstructed sample array
 * \param rpDst reference to pointer for the prediction sample array
 * \param iDstStride the stride of the prediction sample array
 * \param iWidth the width of the block
 * \param iHeight the height of the block
 *
 * This function performs filtering left and top edges of the prediction samples for DC mode (intra coding).
 */
Void TComPrediction::xDCPredFiltering( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, Int iWidth, Int iHeight )
{
  Pel* pDst = rpDst;
  Int x, y, iDstStride2, iSrcStride2;
  Int iIntraSizeIdx = g_aucConvertToBit[ iWidth ] + 1;
  static const UChar g_aucDCPredFilter[7] = { 0, 3, 2, 1, 0, 0, 0};

  switch (g_aucDCPredFilter[iIntraSizeIdx])
  {
  case 0:
    {}
    break;
  case 1:
    {
      // boundary pixels processing
      pDst[0] = (Pel)((pSrc[-iSrcStride] + pSrc[-1] + 6 * pDst[0] + 4) >> 3);

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = (Pel)((pSrc[x - iSrcStride] + 7 * pDst[x] + 4) >> 3);

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = (Pel)((pSrc[iSrcStride2] + 7 * pDst[iDstStride2] + 4) >> 3);
    }
    break;
  case 2:
    {
      // boundary pixels processing
      pDst[0] = (Pel)((pSrc[-iSrcStride] + pSrc[-1] + 2 * pDst[0] + 2) >> 2);

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = (Pel)((pSrc[x - iSrcStride] + 3 * pDst[x] + 2) >> 2);

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = (Pel)((pSrc[iSrcStride2] + 3 * pDst[iDstStride2] + 2) >> 2);
    }
    break;
  case 3:
    {
      // boundary pixels processing
      pDst[0] = (Pel)((3 * (pSrc[-iSrcStride] + pSrc[-1]) + 2 * pDst[0] + 4) >> 3);

      for ( x = 1; x < iWidth; x++ )
        pDst[x] = (Pel)((3 * pSrc[x - iSrcStride] + 5 * pDst[x] + 4) >> 3);

      for ( y = 1, iDstStride2 = iDstStride, iSrcStride2 = iSrcStride-1; y < iHeight; y++, iDstStride2+=iDstStride, iSrcStride2+=iSrcStride )
        pDst[iDstStride2] = (Pel)((3 * pSrc[iSrcStride2] + 5 * pDst[iDstStride2] + 4) >> 3);
    }
    break;
  }

  return;
}
#endif
