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

/** \file     TComPattern.cpp
    \brief    neighbouring pixel access classes
*/

#include "TComPic.h"
#include "TComPattern.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Tables
// ====================================================================================================================

const UChar g_aaucAvailableBlkMask[16][8] =
{
  // 4x4 block neighbor availability      // MB neighbor availability
  {0x0,0x0,0x0,0x0,  0x0,0x8,0x0,0x08  }, // L, A, AL, AR   <== WTF (blkIdx < 8)
  {0x1,0x0,0x0,0x0,  0x1,0x8,0x0,0x08  }, //    A, AL, AR
  {0xA,0xE,0xE,0x6,  0x0,0x8,0x0,0x08  }, // L,    AL, AR
  {0xB,0xE,0xE,0x6,  0x1,0x8,0x0,0x08  }, //       AL, AR
  
  {0x4,0x0,0x0,0x0,  0x0,0x8,0x0,0x08  }, // L, A,     AR
  {0x5,0x0,0x0,0x0,  0x1,0x8,0x0,0x08  }, //    A,     AR
  {0xE,0xE,0xE,0x6,  0x0,0x8,0x0,0x08  }, // L,        AR
  {0xF,0xE,0xE,0x6,  0x1,0x8,0x0,0x08  }, //           AR
  
  {0x0,0x0,0x0,0x8,  0x0,0x8,0x0,0x08  }, // L, A, AL       <== WTF (blkIdx < 8 || blkIdx >= 8)
  {0x1,0x0,0x0,0x8,  0x1,0x8,0x0,0x08  }, //    A, AL
  {0xA,0xE,0xE,0xE,  0x0,0x8,0x0,0x08  }, // L,    AL
  {0xB,0xE,0xE,0xE,  0x1,0x8,0x0,0x08  }, //       AL
  
  {0x4,0x0,0x0,0x8,  0x0,0x8,0x0,0x08  }, // L, A,
  {0x5,0x0,0x0,0x8,  0x1,0x8,0x0,0x08  }, //    A,
  {0xE,0xE,0xE,0xE,  0x0,0x8,0x0,0x08  }, // L,
  {0xF,0xE,0xE,0xE,  0x1,0x8,0x0,0x08  }  //
};

// ====================================================================================================================
// Public member functions (TComPatternParam)
// ====================================================================================================================

/** \param  piTexture     pixel data
 \param  iRoiWidth     pattern width
 \param  iRoiHeight    pattern height
 \param  iStride       buffer stride
 \param  iOffsetLeft   neighbour offset (left)
 \param  iOffsetRight  neighbour offset (right)
 \param  iOffsetAbove  neighbour offset (above)
 \param  iOffsetBottom neighbour offset (bottom)
 */
Void TComPatternParam::setPatternParamPel ( Pel* piTexture,
                                           Int iRoiWidth,
                                           Int iRoiHeight,
                                           Int iStride,
                                           Int iOffsetLeft,
                                           Int iOffsetRight,
                                           Int iOffsetAbove,
                                           Int iOffsetBottom )
{
  m_piPatternOrigin = piTexture;
  m_iROIWidth       = iRoiWidth;
  m_iROIHeight      = iRoiHeight;
  m_iPatternStride  = iStride;
  m_iOffsetLeft     = iOffsetLeft;
  m_iOffsetAbove    = iOffsetAbove;
  m_iOffsetRight    = iOffsetRight;
  m_iOffsetBottom   = iOffsetBottom;
}

/**
 \param  pcCU          CU data structure
 \param  iComp         component index (0=Y, 1=Cb, 2=Cr)
 \param  iRoiWidth     pattern width
 \param  iRoiHeight    pattern height
 \param  iStride       buffer stride
 \param  iOffsetLeft   neighbour offset (left)
 \param  iOffsetRight  neighbour offset (right)
 \param  iOffsetAbove  neighbour offset (above)
 \param  iOffsetBottom neighbour offset (bottom)
 \param  uiPartDepth   CU depth
 \param  uiAbsPartIdx  part index
 */
Void TComPatternParam::setPatternParamCU( TComDataCU* pcCU,
                                         UChar       iComp,
                                         UChar       iRoiWidth,
                                         UChar       iRoiHeight,
                                         Int         iOffsetLeft,
                                         Int         iOffsetRight,
                                         Int         iOffsetAbove,
                                         Int         iOffsetBottom,
                                         UInt        uiPartDepth,
                                         UInt        uiAbsPartIdx )
{
  m_iOffsetLeft   = iOffsetLeft;
  m_iOffsetRight  = iOffsetRight;
  m_iOffsetAbove  = iOffsetAbove;
  m_iOffsetBottom = iOffsetBottom;
  
  m_iROIWidth     = iRoiWidth;
  m_iROIHeight    = iRoiHeight;
  
  UInt uiAbsZorderIdx = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  
  if ( iComp == 0 )
  {
    m_iPatternStride  = pcCU->getPic()->getStride();
    m_piPatternOrigin = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiAbsZorderIdx) - m_iOffsetAbove * m_iPatternStride - m_iOffsetLeft;
  }
  else
  {
    m_iPatternStride = pcCU->getPic()->getCStride();
    if ( iComp == 1 )
      m_piPatternOrigin = pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), uiAbsZorderIdx) - m_iOffsetAbove * m_iPatternStride - m_iOffsetLeft;
    else
      m_piPatternOrigin = pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), uiAbsZorderIdx) - m_iOffsetAbove * m_iPatternStride - m_iOffsetLeft;
  }
}

// ====================================================================================================================
// Public member functions (TComPattern)
// ====================================================================================================================

Void TComPattern::initPattern ( Pel* piY,
                               Pel* piCb,
                               Pel* piCr,
                               Int iRoiWidth,
                               Int iRoiHeight,
                               Int iStride,
                               Int iOffsetLeft,
                               Int iOffsetRight,
                               Int iOffsetAbove,
                               Int iOffsetBottom )
{
  m_cPatternY. setPatternParamPel( piY,  iRoiWidth,      iRoiHeight,      iStride,      iOffsetLeft,      iOffsetRight,      iOffsetAbove,      iOffsetBottom );
  m_cPatternCb.setPatternParamPel( piCb, iRoiWidth >> 1, iRoiHeight >> 1, iStride >> 1, iOffsetLeft >> 1, iOffsetRight >> 1, iOffsetAbove >> 1, iOffsetBottom >> 1 );
  m_cPatternCr.setPatternParamPel( piCr, iRoiWidth >> 1, iRoiHeight >> 1, iStride >> 1, iOffsetLeft >> 1, iOffsetRight >> 1, iOffsetAbove >> 1, iOffsetBottom >> 1 );
  
  return;
}

Void TComPattern::initPattern( TComDataCU* pcCU, UInt uiPartDepth, UInt uiAbsPartIdx )
{
  Int   uiOffsetLeft  = 0;
  Int   uiOffsetRight = 0;
  Int   uiOffsetAbove = 0;
  
  TComPic* pcPic         = pcCU->getPic();
  UChar uiWidth          = pcCU->getWidth (0)>>uiPartDepth;
  UChar uiHeight         = pcCU->getHeight(0)>>uiPartDepth;
  
  UInt  uiAbsZorderIdx   = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  UInt  uiCurrPicPelX    = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
  UInt  uiCurrPicPelY    = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
  
  if( uiCurrPicPelX != 0 )
    uiOffsetLeft = 1;
  if( uiCurrPicPelY != 0 )
  {
    UInt uiNumPartInWidth = ( uiWidth/pcPic->getMinCUWidth() );
    uiOffsetAbove = 1;
    
    if( uiCurrPicPelX + uiWidth < pcCU->getSlice()->getSPS()->getWidth() )
    {
      if( ( g_auiZscanToRaster[uiAbsZorderIdx] + uiNumPartInWidth ) % pcPic->getNumPartInWidth() ) // Not CU boundary
      {
        if( g_auiRasterToZscan[ (Int)g_auiZscanToRaster[uiAbsZorderIdx] - (Int)pcPic->getNumPartInWidth() + (Int)uiNumPartInWidth ] < uiAbsZorderIdx )
          uiOffsetRight = 1;
      }
      else // if it is CU boundary
      {
        if( g_auiZscanToRaster[uiAbsZorderIdx] < pcPic->getNumPartInWidth() && (uiCurrPicPelX+uiWidth) < pcPic->getPicYuvRec()->getWidth() ) // first line
        {
          uiOffsetRight = 1;
        }
      }
    }
  }
  
#if LM_CHROMA && !LM_CHROMA_TICKET156
  m_bLeftAvailable = uiCurrPicPelX > 0 ? true : false;
  m_bAboveAvailable = uiCurrPicPelY > 0 ? true : false;
#endif

  m_cPatternY .setPatternParamCU( pcCU, 0, uiWidth,      uiHeight,      uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
  m_cPatternCb.setPatternParamCU( pcCU, 1, uiWidth >> 1, uiHeight >> 1, uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
  m_cPatternCr.setPatternParamCU( pcCU, 2, uiWidth >> 1, uiHeight >> 1, uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
}

#if LM_CHROMA_TICKET156
Void TComPattern::initAdiPattern( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piAdiBuf, Int iOrgBufStride, Int iOrgBufHeight, Bool& bAbove, Bool& bLeft, UInt uiExt )
#else
Void TComPattern::initAdiPattern( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piAdiBuf, Int iOrgBufStride, Int iOrgBufHeight, Bool& bAbove, Bool& bLeft )
#endif
{
  Pel*  piRoiOrigin;
  Int*  piAdiTemp;
  UInt  uiCuWidth   = pcCU->getWidth(0) >> uiPartDepth;
  UInt  uiCuHeight  = pcCU->getHeight(0)>> uiPartDepth;
  UInt  uiCuWidth2  = uiCuWidth<<1;
  UInt  uiCuHeight2 = uiCuHeight<<1;
  UInt  uiWidth;
  UInt  uiHeight;
  Int   iPicStride = pcCU->getPic()->getStride();
  Int   i;
#if REFERENCE_SAMPLE_PADDING
  Int   iUnitSize = 0;
  Int   iNumUnitsInCu = 0;
  Int   iTotalUnits = 0;
  Bool* bNeighborFlags;
  Int   iNumIntraNeighbor = 0;
#else  // REFERENCE_SAMPLE_PADDING
  Pel*  piRoiTemp;
  Bool  bAboveFlag      = false;
  Bool  bAboveRightFlag = false;
  Bool  bLeftFlag       = false;
  Bool  bBelowLeftFlag  = false;
  Bool  bAboveLeftFlag  = false;
#endif // REFERENCE_SAMPLE_PADDING
  
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB, uiPartDum;
  
  pcCU->deriveLeftRightTopIdxAdi( uiPartIdxLT, uiPartIdxRT, uiZorderIdxInPart, uiPartDepth );
  pcCU->deriveLeftBottomIdxAdi  ( uiPartIdxLB,              uiZorderIdxInPart, uiPartDepth );
  
#if CONSTRAINED_INTRA_PRED
  if ( pcCU->getSlice()->getPPS()->getConstrainedIntraPred() )
  {
#if REFERENCE_SAMPLE_PADDING
    iUnitSize      = g_uiMaxCUWidth >> g_uiMaxCUDepth;
    iNumUnitsInCu  = uiCuWidth / iUnitSize;
    iTotalUnits    = (iNumUnitsInCu << 2) + 1;
    bNeighborFlags = new Bool[iTotalUnits];

    bNeighborFlags[iNumUnitsInCu*2] = isAboveLeftAvailableForCIP( pcCU, uiPartIdxLT );
    iNumIntraNeighbor  += (Int)(bNeighborFlags[iNumUnitsInCu*2]);
    iNumIntraNeighbor  += isAboveAvailableForCIP     ( pcCU, uiPartIdxLT, uiPartIdxRT, bNeighborFlags+(iNumUnitsInCu*2)+1 );
    iNumIntraNeighbor  += isAboveRightAvailableForCIP( pcCU, uiPartIdxLT, uiPartIdxRT, bNeighborFlags+(iNumUnitsInCu*3)+1 );
    iNumIntraNeighbor  += isLeftAvailableForCIP      ( pcCU, uiPartIdxLT, uiPartIdxLB, bNeighborFlags+(iNumUnitsInCu*2)-1 );
    iNumIntraNeighbor  += isBelowLeftAvailableForCIP ( pcCU, uiPartIdxLT, uiPartIdxLB, bNeighborFlags+ iNumUnitsInCu   -1 );
#else // REFERENCE_SAMPLE_PADDING
    bAboveFlag      = isAboveAvailableForCIP     ( pcCU, uiPartIdxLT, uiPartIdxRT );
    bAboveRightFlag = isAboveRightAvailableForCIP( pcCU, uiPartIdxLT, uiPartIdxRT );
    bLeftFlag       = isLeftAvailableForCIP      ( pcCU, uiPartIdxLT, uiPartIdxLB );
    bBelowLeftFlag  = isBelowLeftAvailableForCIP ( pcCU, uiPartIdxLT, uiPartIdxLB );
    bAboveLeftFlag  = isAboveLeftAvailableForCIP ( pcCU, uiPartIdxLT );
#endif // REFERENCE_SAMPLE_PADDING
  }
  else
  {
#if REFERENCE_SAMPLE_PADDING
    iUnitSize     = uiCuWidth;
    iNumUnitsInCu = 1;
    iTotalUnits   = 5;
    bNeighborFlags = new Bool[iTotalUnits];

    ::memset(bNeighborFlags, false, sizeof(Bool)*iTotalUnits);
    if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[3] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, 1, true, false ) ) { bNeighborFlags[4] = true; iNumIntraNeighbor++; }
    if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[1] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, 1, true, false ) ) { bNeighborFlags[0] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[2] = true; iNumIntraNeighbor++; }
#if MN_DC_PRED_FILTER
    m_bAboveFlagForDCFilt = bNeighborFlags[3];
    m_bLeftFlagForDCFilt  = bNeighborFlags[1];
#endif
#else // REFERENCE_SAMPLE_PADDING
    if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT,    true, false ) ) bAboveFlag      = true;
    if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, 1, true, false ) ) bAboveRightFlag = true;
    if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT,    true, false ) ) bLeftFlag       = true;
    if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, 1, true, false ) ) bBelowLeftFlag  = true;
    if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT,    true, false ) ) bAboveLeftFlag  = true;
#if MN_DC_PRED_FILTER
    m_bAboveFlagForDCFilt = bAboveFlag;
    m_bLeftFlagForDCFilt  = bLeftFlag;
#endif
#endif // REFERENCE_SAMPLE_PADDING
  }
#else //CONSTRAINED_INTRA_PRED
#if REFERENCE_SAMPLE_PADDING
  iUnitSize     = uiCuWidth;
  iNumUnitsInCu = 1;
  iTotalUnits   = 5;
  bNeighborFlags = new Bool[iTotalUnits];

  ::memset(bNeighborFlags, false, sizeof(Bool)*iTotalUnits);
  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[3] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, true, false ) ) { bNeighborFlags[4] = true; iNumIntraNeighbor++; }
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[1] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, true, false ) ) { bNeighborFlags[0] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[2] = true; iNumIntraNeighbor++; }
#if MN_DC_PRED_FILTER
  m_bAboveFlagForDCFilt = bNeighborFlags[3];
  m_bLeftFlagForDCFilt  = bNeighborFlags[1];
#endif
#else // REFERENCE_SAMPLE_PADDING
  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT, true, false ) ) bAboveFlag      = true;
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, true, false ) ) bAboveRightFlag = true;
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT, true, false ) ) bLeftFlag       = true;
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, true, false ) ) bBelowLeftFlag  = true;
  if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT, true, false ) ) bAboveLeftFlag  = true;
#if MN_DC_PRED_FILTER
  m_bAboveFlagForDCFilt = bAboveFlag;
  m_bLeftFlagForDCFilt  = bLeftFlag;
#endif
#endif // REFERENCE_SAMPLE_PADDING
#endif //CONSTRAINED_INTRA_PRED

#if MN_DC_PRED_FILTER
  m_bDCPredFilterFlag = (m_bAboveFlagForDCFilt && m_bLeftFlagForDCFilt) ? true : false;
#endif
  
#if REFERENCE_SAMPLE_PADDING
  bAbove = true;
  bLeft  = true;
#else // REFERENCE_SAMPLE_PADDING
  bAbove = bAboveFlag;
  bLeft  = bLeftFlag;

  Int iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );
#endif // REFERENCE_SAMPLE_PADDING

  uiWidth=uiCuWidth2+1;
  uiHeight=uiCuHeight2+1;
  
  if (((uiWidth<<2)>iOrgBufStride)||((uiHeight<<2)>iOrgBufHeight))
    return;
  
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  piAdiTemp   = piAdiBuf;

#if REFERENCE_SAMPLE_PADDING
#if LM_CHROMA_TICKET156
  if( uiExt == 2 )
    fill2ReferenceSamples_LM ( pcCU, piRoiOrigin, piAdiTemp, bNeighborFlags, iNumIntraNeighbor, iUnitSize, iNumUnitsInCu, iTotalUnits, uiCuWidth, uiCuHeight, uiWidth, uiHeight, iPicStride);
  else if( uiExt == 1 )
#endif
  fillReferenceSamples ( pcCU, piRoiOrigin, piAdiTemp, bNeighborFlags, iNumIntraNeighbor, iUnitSize, iNumUnitsInCu, iTotalUnits, uiCuWidth, uiCuHeight, uiWidth, uiHeight, iPicStride);

  delete [] bNeighborFlags;
  bNeighborFlags = NULL;

#if LM_CHROMA_TICKET156
  if( uiExt == 2 )
  {
    return;
  }
#endif

#else // REFERENCE_SAMPLE_PADDING
  //BB: fill border with DC value - needed if( bAboveFlag=false || bLeftFlag=false )
  for (i=0;i<uiWidth;i++)
    piAdiTemp[i]=iDCValue;
  for (i=0;i<uiHeight;i++) //BB: can start from 1?
    piAdiTemp[i*uiWidth]=iDCValue;
  
  piRoiTemp=piRoiOrigin; //BB: not needed?
  
  if (bAboveFlag)
  {
    // BB: fill top border with rec. samples
    piRoiTemp=piRoiOrigin-iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piAdiTemp[1+i]=piRoiTemp[i];
    // BB: fill top right border with rec. samples
    if (bAboveRightFlag)
    {
      piRoiTemp=piRoiOrigin-iPicStride+uiCuWidth;
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piRoiTemp[i];
    }
    // BB: fill top right border with last sample from top
    else
    {
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piAdiTemp[uiCuWidth];
    }
  }
  
  if (bAboveLeftFlag)
  {
    piRoiTemp=piRoiOrigin-iPicStride-1;
    piAdiTemp[0]=piRoiTemp[0];
  }

  if (bLeftFlag)
  {
    // BB: fill left border with rec. samples
    piRoiTemp=piRoiOrigin-1;
    for (i=0;i<uiCuHeight;i++)
    {
      piAdiTemp[(1+i)*uiWidth]=piRoiTemp[0];
      piRoiTemp+=iPicStride;
    }
    // BB: fill below left border with rec. samples
    if (bBelowLeftFlag)
    {
      for (i=0;i<uiCuHeight;i++)
      {
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piRoiTemp[0];
        piRoiTemp+=iPicStride;
      }
    }
    // BB: fill below left border with last sample from left
    else
    {
      for (i=0;i<uiCuHeight;i++)
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piAdiTemp[(uiCuHeight)*uiWidth];
    }
  }
#endif // REFERENCE_SAMPLE_PADDING
  
#if QC_MDIS
  // generate filtered intra prediction samples
  Int iBufSize = uiCuHeight2 + uiCuWidth2 + 1;  // left and left above border + above and above right border + top left corner = length of 3. filter buffer

  UInt uiWH = uiWidth * uiHeight;               // number of elements in one buffer

  Int* piFilteredBuf1 = piAdiBuf + uiWH;        // 1. filter buffer
  Int* piFilteredBuf2 = piFilteredBuf1 + uiWH;  // 2. filter buffer
  Int* piFilterBuf = piFilteredBuf2 + uiWH;     // buffer for 2. filtering (sequential)
  Int* piFilterBufN = piFilterBuf + iBufSize;   // buffer for 1. filtering (sequential)

  Int l = 0;
  // left border from bottom to top
  for (i = 0; i < uiCuHeight2; i++)
    piFilterBuf[l++] = piAdiTemp[uiWidth * (uiCuHeight2 - i)];
  // top left corner
  piFilterBuf[l++] = piAdiTemp[0];
  // above border from left to right
  for (i=0; i < uiCuWidth2; i++)
    piFilterBuf[l++] = piAdiTemp[1 + i];

  // 1. filtering with [1 2 1]
  piFilterBufN[0] = piFilterBuf[0];
  piFilterBufN[iBufSize - 1] = piFilterBuf[iBufSize - 1];
  for (i = 1; i < iBufSize - 1; i++)
    piFilterBufN[i] = (piFilterBuf[i - 1] + 2 * piFilterBuf[i]+piFilterBuf[i + 1] + 2) >> 2;

  // fill 1. filter buffer with filtered values
  l=0;
  for (i = 0; i < uiCuHeight2; i++)
    piFilteredBuf1[uiWidth * (uiCuHeight2 - i)] = piFilterBufN[l++];
  piFilteredBuf1[0] = piFilterBufN[l++];
  for (i = 0; i < uiCuWidth2; i++)
    piFilteredBuf1[1 + i] = piFilterBufN[l++];

#if !MN_MDIS_SIMPLIFICATION
  // 2. filtering with [1 2 1]
  piFilterBuf[0] = piFilterBufN[0];                   
  piFilterBuf[iBufSize - 1] = piFilterBufN[iBufSize - 1];
  for (i = 1; i < iBufSize - 1; i++)
    piFilterBuf[i] = (piFilterBufN[i - 1] + 2 * piFilterBufN[i] + piFilterBufN[i + 1] + 2) >> 2;

  // fill 2. filter buffer with filtered values
  l=0;
  for (i = 0; i < uiCuHeight2; i++)
    piFilteredBuf2[uiWidth * (uiCuHeight2 - i)] = piFilterBuf[l++];
  piFilteredBuf2[0] = piFilterBuf[l++];
  for (i = 0; i < uiCuWidth2; i++)
    piFilteredBuf2[1 + i] = piFilterBuf[l++];
#endif
#endif //QC_MDIS
  
}

Void TComPattern::initAdiPatternChroma( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piAdiBuf, Int iOrgBufStride, Int iOrgBufHeight, Bool& bAbove, Bool& bLeft )
{
  Pel*  piRoiOrigin;
  Int*  piAdiTemp;
  UInt  uiCuWidth  = pcCU->getWidth (0) >> uiPartDepth;
  UInt  uiCuHeight = pcCU->getHeight(0) >> uiPartDepth;
  UInt  uiWidth;
  UInt  uiHeight;
  Int   iPicStride = pcCU->getPic()->getCStride();
#if REFERENCE_SAMPLE_PADDING
  Int   iUnitSize = 0;
  Int   iNumUnitsInCu = 0;
  Int   iTotalUnits = 0;
  Bool* bNeighborFlags;
  Int   iNumIntraNeighbor = 0;
#else // REFERENCE_SAMPLE_PADDING
  Pel*  piRoiTemp;
  Int   i;
  Bool  bAboveFlag=false;
  Bool  bAboveRightFlag=false;
  Bool  bLeftFlag=false;
  Bool  bBelowLeftFlag=false;
  Bool  bAboveLeftFlag    = false;
#endif // REFERENCE_SAMPLE_PADDING
  
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB,uiPartDum;
  
  pcCU->deriveLeftRightTopIdxAdi( uiPartIdxLT, uiPartIdxRT, uiZorderIdxInPart, uiPartDepth );
  pcCU->deriveLeftBottomIdxAdi  ( uiPartIdxLB,              uiZorderIdxInPart, uiPartDepth );
  
#if CONSTRAINED_INTRA_PRED
  if ( pcCU->getSlice()->getPPS()->getConstrainedIntraPred() )
  {
#if REFERENCE_SAMPLE_PADDING
    iUnitSize      = (g_uiMaxCUWidth >> g_uiMaxCUDepth) >> 1; // for chroma
    iNumUnitsInCu  = (uiCuWidth / iUnitSize) >> 1;            // for chroma
    iTotalUnits    = (iNumUnitsInCu << 2) + 1;
    bNeighborFlags = new Bool[iTotalUnits];

    bNeighborFlags[iNumUnitsInCu*2] = isAboveLeftAvailableForCIP( pcCU, uiPartIdxLT );
    iNumIntraNeighbor  += (Int)(bNeighborFlags[iNumUnitsInCu*2]);
    iNumIntraNeighbor  += isAboveAvailableForCIP     ( pcCU, uiPartIdxLT, uiPartIdxRT, bNeighborFlags+(iNumUnitsInCu*2)+1 );
    iNumIntraNeighbor  += isAboveRightAvailableForCIP( pcCU, uiPartIdxLT, uiPartIdxRT, bNeighborFlags+(iNumUnitsInCu*3)+1 );
    iNumIntraNeighbor  += isLeftAvailableForCIP      ( pcCU, uiPartIdxLT, uiPartIdxLB, bNeighborFlags+(iNumUnitsInCu*2)-1 );
    iNumIntraNeighbor  += isBelowLeftAvailableForCIP ( pcCU, uiPartIdxLT, uiPartIdxLB, bNeighborFlags+ iNumUnitsInCu   -1 );
#else // REFERENCE_SAMPLE_PADDING
    bAboveFlag      = isAboveAvailableForCIP     ( pcCU, uiPartIdxLT, uiPartIdxRT );
    bAboveRightFlag = isAboveRightAvailableForCIP( pcCU, uiPartIdxLT, uiPartIdxRT );
    bLeftFlag       = isLeftAvailableForCIP      ( pcCU, uiPartIdxLT, uiPartIdxLB );
    bBelowLeftFlag  = isBelowLeftAvailableForCIP ( pcCU, uiPartIdxLT, uiPartIdxLB );
    bAboveLeftFlag  = isAboveLeftAvailableForCIP ( pcCU, uiPartIdxLT );
#endif // REFERENCE_SAMPLE_PADDING
  }
  else
  {
#if REFERENCE_SAMPLE_PADDING
    iUnitSize     = uiCuWidth >> 1;
    iNumUnitsInCu = 1;
    iTotalUnits   = 5;
    bNeighborFlags = new Bool[iTotalUnits];

    ::memset(bNeighborFlags, false, sizeof(Bool)*iTotalUnits);
    if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[3] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, 1, true, false ) ) { bNeighborFlags[4] = true; iNumIntraNeighbor++; }
    if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[1] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, 1, true, false ) ) { bNeighborFlags[0] = true; iNumIntraNeighbor++; }
    if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT,    true, false ) ) { bNeighborFlags[2] = true; iNumIntraNeighbor++; }
#else // REFERENCE_SAMPLE_PADDING
    if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT,    true, false ) ) bAboveFlag      = true;
    if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, 1, true, false ) ) bAboveRightFlag = true;
    if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT,    true, false ) ) bLeftFlag       = true;
    if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, 1, true, false ) ) bBelowLeftFlag  = true;
    if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT,    true, false ) ) bAboveLeftFlag  = true;
#endif // REFERENCE_SAMPLE_PADDING
  }
#else //CONSTRAINED_INTRA_PRED
#if REFERENCE_SAMPLE_PADDING
  iUnitSize     = uiCuWidth >> 1;
  iNumUnitsInCu = 1;
  iTotalUnits   = 5;
  bNeighborFlags = new Bool[iTotalUnits];

  ::memset(bNeighborFlags, false, sizeof(Bool)*iTotalUnits);
  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[3] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, true, false ) ) { bNeighborFlags[4] = true; iNumIntraNeighbor++; }
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[1] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, true, false ) ) { bNeighborFlags[0] = true; iNumIntraNeighbor++; }
  if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT, true, false ) ) { bNeighborFlags[2] = true; iNumIntraNeighbor++; }
#else // REFERENCE_SAMPLE_PADDING
  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT, true, false ) ) bAboveFlag      = true;
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT, true, false ) ) bAboveRightFlag = true;
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT, true, false ) ) bLeftFlag       = true;
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB, true, false ) ) bBelowLeftFlag  = true;
  if( pcCU->getPUAboveLeft    ( uiPartDum,             uiPartIdxLT, true, false ) ) bAboveLeftFlag  = true;
#endif // REFERENCE_SAMPLE_PADDING
#endif //CONSTRAINED_INTRA_PRED
  
#if REFERENCE_SAMPLE_PADDING
  bAbove = true;
  bLeft  = true;
#else // REFERENCE_SAMPLE_PADDING
  bAbove = bAboveFlag;
  bLeft  = bLeftFlag;

  Int iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );

#endif // REFERENCE_SAMPLE_PADDING
  
  

  uiCuWidth=uiCuWidth>>1;  // for chroma
  uiCuHeight=uiCuHeight>>1;  // for chroma
  
  uiWidth=uiCuWidth*2+1;
  uiHeight=uiCuHeight*2+1;
  
  if ((4*uiWidth>iOrgBufStride)||(4*uiHeight>iOrgBufHeight))
    return;
  
  // get Cb pattern
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  piAdiTemp   = piAdiBuf;

#if REFERENCE_SAMPLE_PADDING
  fillReferenceSamples ( pcCU, piRoiOrigin, piAdiTemp, bNeighborFlags, iNumIntraNeighbor, iUnitSize, iNumUnitsInCu, iTotalUnits, uiCuWidth, uiCuHeight, uiWidth, uiHeight, iPicStride);
#else // REFERENCE_SAMPLE_PADDING
  for (i=0;i<uiWidth;i++)
    piAdiTemp[i]=iDCValue;
  for (i=0;i<uiHeight;i++)
    piAdiTemp[i*uiWidth]=iDCValue;
  
  piRoiTemp=piRoiOrigin;
  
  if (bAboveFlag)
  {
    piRoiTemp=piRoiOrigin-iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piAdiTemp[1+i]=piRoiTemp[i];
    if (bAboveRightFlag)
    {
      piRoiTemp=piRoiOrigin-iPicStride+uiCuWidth;
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piRoiTemp[i];
    }
    else
    {
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piAdiTemp[uiCuWidth];
    }
  }
  
  if (bAboveLeftFlag)
  {
    piRoiTemp=piRoiOrigin-iPicStride-1;
    piAdiTemp[0]=piRoiTemp[0];
  }

  if (bLeftFlag)
  {
    piRoiTemp=piRoiOrigin-1;
    for (i=0;i<uiCuHeight;i++)
    {
      piAdiTemp[(1+i)*uiWidth]=piRoiTemp[0];
      piRoiTemp+=iPicStride;
    }
    if (bBelowLeftFlag)
    {
      for (i=0;i<uiCuHeight;i++)
      {
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piRoiTemp[0];
        piRoiTemp+=iPicStride;
      }
    }
    else
    {
      for (i=0;i<uiCuHeight;i++)
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piAdiTemp[(uiCuHeight)*uiWidth];
    }
  }
#endif // REFERENCE_SAMPLE_PADDING
  
  // get Cr pattern
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  piAdiTemp   = piAdiBuf+uiWidth*uiHeight;
  
#if REFERENCE_SAMPLE_PADDING
  fillReferenceSamples ( pcCU, piRoiOrigin, piAdiTemp, bNeighborFlags, iNumIntraNeighbor, iUnitSize, iNumUnitsInCu, iTotalUnits, uiCuWidth, uiCuHeight, uiWidth, uiHeight, iPicStride);
  delete [] bNeighborFlags;
  bNeighborFlags = NULL;
#else // REFERENCE_SAMPLE_PADDING
  for (i=0;i<uiWidth;i++)
    piAdiTemp[i]=iDCValue;
  for (i=0;i<uiHeight;i++)
    piAdiTemp[i*uiWidth]=iDCValue;
  
  piRoiTemp=piRoiOrigin;
  
  if (bAboveFlag)
  {
    piRoiTemp=piRoiOrigin-iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piAdiTemp[1+i]=piRoiTemp[i];
    if (bAboveRightFlag)
    {
      piRoiTemp=piRoiOrigin-iPicStride+uiCuWidth;
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piRoiTemp[i];
    }
    else
    {
      for (i=0;i<uiCuWidth;i++)
        piAdiTemp[1+uiCuWidth+i]=piAdiTemp[uiCuWidth];
    }
  }

  if (bAboveLeftFlag)
  {
    piRoiTemp=piRoiOrigin-iPicStride-1;
    piAdiTemp[0]=piRoiTemp[0];
  }

  if (bLeftFlag)
  {
    piRoiTemp=piRoiOrigin-1;
    for (i=0;i<uiCuHeight;i++)
    {
      piAdiTemp[(1+i)*uiWidth]=piRoiTemp[0];
      piRoiTemp+=iPicStride;
    }
    if (bBelowLeftFlag)
    {
      for (i=0;i<uiCuHeight;i++)
      {
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piRoiTemp[0];
        piRoiTemp+=iPicStride;
      }
    }
    else
    {
      for (i=0;i<uiCuHeight;i++)
        piAdiTemp[(1+uiCuHeight+i)*uiWidth]=piAdiTemp[(uiCuHeight)*uiWidth];
    }
  }
#endif // REFERENCE_SAMPLE_PADDING
}

#if REFERENCE_SAMPLE_PADDING
Void TComPattern::fillReferenceSamples( TComDataCU* pcCU, Pel* piRoiOrigin, Int* piAdiTemp, Bool* bNeighborFlags, Int iNumIntraNeighbor, Int iUnitSize, Int iNumUnitsInCu, Int iTotalUnits, UInt uiCuWidth, UInt uiCuHeight, UInt uiWidth, UInt uiHeight, Int iPicStride)
{
  Pel* piRoiTemp;
  Int  i, j;
  Int  iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );

  if (iNumIntraNeighbor == 0)
  {
    // Fill border with DC value
    for (i=0; i<uiWidth; i++)
      piAdiTemp[i] = iDCValue;
    for (i=1; i<uiHeight; i++)
      piAdiTemp[i*uiWidth] = iDCValue;
  }
  else if (iNumIntraNeighbor == iTotalUnits)
  {
    // Fill top-left border with rec. samples
    piRoiTemp = piRoiOrigin - iPicStride - 1;
    piAdiTemp[0] = piRoiTemp[0];

    // Fill left border with rec. samples
    piRoiTemp = piRoiOrigin - 1;
    for (i=0; i<uiCuHeight; i++)
    {
      piAdiTemp[(1+i)*uiWidth] = piRoiTemp[0];
      piRoiTemp += iPicStride;
    }

    // Fill below left border with rec. samples
    for (i=0; i<uiCuHeight; i++)
    {
      piAdiTemp[(1+uiCuHeight+i)*uiWidth] = piRoiTemp[0];
      piRoiTemp += iPicStride;
    }

    // Fill top border with rec. samples
    piRoiTemp = piRoiOrigin - iPicStride;
    for (i=0; i<uiCuWidth; i++)
      piAdiTemp[1+i] = piRoiTemp[i];

    // Fill top right border with rec. samples
    piRoiTemp = piRoiOrigin - iPicStride + uiCuWidth;
    for (i=0; i<uiCuWidth; i++)
      piAdiTemp[1+uiCuWidth+i] = piRoiTemp[i];
  }
  else // reference samples are partially available
  {
    Int  iNumUnits2 = iNumUnitsInCu<<1;
    Int  iTotalSamples = iTotalUnits*iUnitSize;
    Pel  *piAdiLine = new Pel[iTotalSamples];
    Pel  *piAdiLineTemp; 
    Bool *pbNeighborFlags;
    Int  iPrev, iNext, iCurr;
    Pel  piRef = 0;

    // Initialize
    for (i=0; i<iTotalSamples; i++)
      piAdiLine[i] = iDCValue;

    // Fill top-left sample
    piRoiTemp = piRoiOrigin - iPicStride - 1;
    piAdiLineTemp = piAdiLine + (iNumUnits2*iUnitSize);
    pbNeighborFlags = bNeighborFlags + iNumUnits2;
    if (*pbNeighborFlags)
    {
      piAdiLineTemp[0] = piRoiTemp[0];
      for (i=1; i<iUnitSize; i++)
        piAdiLineTemp[i] = piAdiLineTemp[0];
    }

    // Fill left & below-left samples
    piRoiTemp += iPicStride;
    piAdiLineTemp--;
    pbNeighborFlags--;
    for (j=0; j<iNumUnits2; j++)
    {
      if (*pbNeighborFlags)
      {
        for (i=0; i<iUnitSize; i++)
          piAdiLineTemp[-i] = piRoiTemp[i*iPicStride];
      }
      piRoiTemp += iUnitSize*iPicStride;
      piAdiLineTemp -= iUnitSize;
      pbNeighborFlags--;
    }

    // Fill above & above-right samples
    piRoiTemp = piRoiOrigin - iPicStride;
    piAdiLineTemp = piAdiLine + ((iNumUnits2+1)*iUnitSize);
    pbNeighborFlags = bNeighborFlags + iNumUnits2 + 1;
    for (j=0; j<iNumUnits2; j++)
    {
      if (*pbNeighborFlags)
      {
        for (i=0; i<iUnitSize; i++)
          piAdiLineTemp[i] = piRoiTemp[i];
      }
      piRoiTemp += iUnitSize;
      piAdiLineTemp += iUnitSize;
      pbNeighborFlags++;
    }

    // Pad reference samples when necessary
    iPrev = -1;
    iCurr = 0;
    iNext = 1;
    piAdiLineTemp = piAdiLine;
    while (iCurr < iTotalUnits)
    {
      if (bNeighborFlags[iCurr])
      {
        // Move on to next block if current unit is available
        piAdiLineTemp += iUnitSize;
        iPrev++;
        iCurr++;
        iNext++;
      }
      else
      {
        // Interpolate from nearest samples if current unit is not available
        
        while (iNext < iTotalUnits && !bNeighborFlags[iNext])
          iNext++;

        if (iPrev >= 0 && iNext < iTotalUnits)
          piRef = (piAdiLine[iCurr*iUnitSize-1] + piAdiLine[iNext*iUnitSize] + 1) >> 1;
        else if (iPrev >= 0)
          piRef = piAdiLine[iCurr*iUnitSize-1];
        else if (iNext < iTotalUnits)
          piRef = piAdiLine[iNext*iUnitSize];
        else
          printf("\nERROR! No valid samples to interpolate.\n");

        // Pad unavailable samples with new value
        while (iCurr < iNext)
        {
          for (i=0; i<iUnitSize; i++)
            piAdiLineTemp[i] = piRef;
          piAdiLineTemp += iUnitSize;
          iPrev++;
          iCurr++;
        }
        iNext++;
      }
    }

    // Copy processed samples
    piAdiLineTemp = piAdiLine + uiHeight + iUnitSize - 2;
    for (i=0; i<uiWidth; i++)
      piAdiTemp[i] = piAdiLineTemp[i];
    piAdiLineTemp = piAdiLine + uiHeight - 1;
    for (i=1; i<uiHeight; i++)
      piAdiTemp[i*uiWidth] = piAdiLineTemp[-i];

    delete [] piAdiLine;
    piAdiLine = NULL;
  }
}

#if LM_CHROMA_TICKET156

/** Function for deriving the neighboring luma reference pixels which is specifically used for luma-based chroma intra prediction method.
  // In this funtioned, first two rows in output buffer correspond to two rows of above reference pixels, 
  // and next two rows correspond to two columns of left reference pixels
 */
Void TComPattern::fill2ReferenceSamples_LM( TComDataCU* pcCU, Pel* piRoiOrigin, Int* piAdiTemp, Bool* bNeighborFlags, Int iNumIntraNeighbor, Int iUnitSize, Int iNumUnitsInCu, Int iTotalUnits, UInt uiCuWidth, UInt uiCuHeight, UInt uiWidth, UInt uiHeight, Int iPicStride)
{
  Pel* piRoiTemp1, *piRoiTemp2;
  Int  i, j;
  Int  iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );
  
  Int iTempStride = Max( uiWidth, uiHeight );
  Int* piAdiTemp1 = piAdiTemp;
  Int* piAdiTemp2 = piAdiTemp1 + iTempStride;
  Int* piAdiTemp3 = piAdiTemp2 + iTempStride;
  Int* piAdiTemp4 = piAdiTemp3 + iTempStride;

  if (iNumIntraNeighbor == 0)
  {
    // Fill border with DC value
    for (i=0; i<uiWidth; i++)
    {
      piAdiTemp1[i] = iDCValue;
    }
    memcpy( piAdiTemp2, piAdiTemp1, sizeof( Int ) * iTempStride );
    memcpy( piAdiTemp3, piAdiTemp1, sizeof( Int ) * iTempStride );
    memcpy( piAdiTemp4, piAdiTemp1, sizeof( Int ) * iTempStride );
  }
  else if (iNumIntraNeighbor == iTotalUnits)
  {
    // Fill top-left border with rec. samples
    // actually two point shoud be taken, but one pixel is considered since for LM top-left in not needed
    // top-left reference can be removed at all
    piRoiTemp1 = piRoiOrigin - iPicStride * 2 - 1;
    piRoiTemp2 = piRoiOrigin - iPicStride - 1;
    piAdiTemp1[0] = piRoiTemp1[0];
    piAdiTemp2[0] = piRoiTemp2[0];
    
    // Fill left border with rec. samples
    piRoiTemp1 = piRoiOrigin - 2;
    piRoiTemp2 = piRoiOrigin - 1;
    for (i=0; i<uiCuHeight; i++)
    {
      piAdiTemp3[i] = piRoiTemp1[0];
      piAdiTemp4[i] = piRoiTemp2[0];
      piRoiTemp1 += iPicStride;
      piRoiTemp2 += iPicStride;
    }

    // Fill below left border with rec. samples
    piRoiTemp1 = piRoiOrigin + uiCuHeight - 2;
    piRoiTemp2 = piRoiOrigin + uiCuHeight - 1;
    for (i=0; i<uiCuHeight; i++)
    {
      piAdiTemp3[uiCuHeight + i] = piRoiTemp1[0];
      piAdiTemp4[uiCuHeight + i] = piRoiTemp2[0];
      piRoiTemp1 += iPicStride;
      piRoiTemp2 += iPicStride;
    }

    // Fill top border with rec. samples
    piRoiTemp1 = piRoiOrigin - iPicStride*2;
    piRoiTemp2 = piRoiOrigin - iPicStride;
    for (i=0; i<uiCuWidth; i++)
    {
      piAdiTemp1[1+i] = piRoiTemp1[i];
      piAdiTemp2[1+i] = piRoiTemp2[i];
    }

    // Fill top right border with rec. samples
    piRoiTemp1 = piRoiOrigin - iPicStride * 2 + uiCuWidth;
    piRoiTemp2 = piRoiOrigin - iPicStride + uiCuWidth;
    for (i=0; i<uiCuWidth; i++)
    {
      piAdiTemp1[1+uiCuWidth+i] = piRoiTemp1[i];
      piAdiTemp2[1+uiCuWidth+i] = piRoiTemp2[i];
    }
  }
  else // reference samples are partially available
  {
    Int  iNumUnits2 = iNumUnitsInCu<<1;
    Int  iTotalSamples = iTotalUnits*iUnitSize;
    Pel  *piAdiLine1 = new Pel[iTotalSamples];
    Pel  *piAdiLine2 = new Pel[iTotalSamples];
    Pel  *piAdiLineTemp1, *piAdiLineTemp2; 
    Bool *pbNeighborFlags;
    Int  iPrev, iNext, iCurr;
    Pel  piRef1 = 0, piRef2 = 0;

    // Initialize
    for (i=0; i<iTotalSamples; i++)
      piAdiLine1[i] = iDCValue;

    memcpy( piAdiLine2, piAdiLine1, sizeof( Pel ) * iTotalSamples );

    // Fill top-left sample
    piRoiTemp1 = piRoiOrigin - iPicStride * 2 - 1;
    piRoiTemp2 = piRoiOrigin - iPicStride - 1;
    piAdiLineTemp1 = piAdiLine1 + (iNumUnits2*iUnitSize);
    piAdiLineTemp2 = piAdiLine2 + (iNumUnits2*iUnitSize);
    pbNeighborFlags = bNeighborFlags + iNumUnits2;
    if (*pbNeighborFlags)
    {
      piAdiLineTemp1[0] = piRoiTemp1[0];
      piAdiLineTemp2[0] = piRoiTemp2[0];
      for (i=1; i<iUnitSize; i++)
      {
        piAdiLineTemp1[i] = piAdiLineTemp1[0];
        piAdiLineTemp2[i] = piAdiLineTemp2[0];
      }
    }

    // Fill left & below-left samples
    piRoiTemp1 = piRoiOrigin - 2;
    piRoiTemp2 = piRoiOrigin - 1;
    piAdiLineTemp1--;
    piAdiLineTemp2--;
    pbNeighborFlags--;
    for (j=0; j<iNumUnits2; j++)
    {
      if (*pbNeighborFlags)
      {
        for (i=0; i<iUnitSize; i++)
        {
          piAdiLineTemp1[-i] = piRoiTemp1[i*iPicStride];
          piAdiLineTemp2[-i] = piRoiTemp2[i*iPicStride];
        }
      }
      piRoiTemp1 += iUnitSize*iPicStride;
      piRoiTemp2 += iUnitSize*iPicStride;
      piAdiLineTemp1 -= iUnitSize;
      piAdiLineTemp2 -= iUnitSize;
      pbNeighborFlags--;
    }
    
    // Fill above & above-right samples
    piRoiTemp1 = piRoiOrigin - iPicStride * 2;
    piRoiTemp2 = piRoiOrigin - iPicStride;
    piAdiLineTemp1 = piAdiLine1 + ((iNumUnits2+1)*iUnitSize);
    piAdiLineTemp2 = piAdiLine2 + ((iNumUnits2+1)*iUnitSize);
    pbNeighborFlags = bNeighborFlags + iNumUnits2 + 1;
    for (j=0; j<iNumUnits2; j++)
    {
      if (*pbNeighborFlags)
      {
        for (i=0; i<iUnitSize; i++)
        {
          piAdiLineTemp1[i] = piRoiTemp1[i];
          piAdiLineTemp2[i] = piRoiTemp2[i];
        }
      }
      piRoiTemp1 += iUnitSize;
      piRoiTemp2 += iUnitSize;
      piAdiLineTemp1 += iUnitSize;
      piAdiLineTemp2 += iUnitSize;
      pbNeighborFlags++;
    }

    // Pad reference samples when necessary
    iPrev = -1;
    iCurr = 0;
    iNext = 1;
    piAdiLineTemp1 = piAdiLine1;
    piAdiLineTemp2 = piAdiLine2;
    while (iCurr < iTotalUnits)
    {
      if (bNeighborFlags[iCurr])
      {
        // Move on to next block if current unit is available
        piAdiLineTemp1 += iUnitSize;
        piAdiLineTemp2 += iUnitSize;
        iPrev++;
        iCurr++;
        iNext++;
      }
      else
      {
        // Interpolate from nearest samples if current unit is not available
        
        while (iNext < iTotalUnits && !bNeighborFlags[iNext])
          iNext++;

        if (iPrev >= 0 && iNext < iTotalUnits)
        {
          piRef1 = (piAdiLine1[iCurr*iUnitSize-1] + piAdiLine1[iNext*iUnitSize] + 1) >> 1;
          piRef2 = (piAdiLine2[iCurr*iUnitSize-1] + piAdiLine2[iNext*iUnitSize] + 1) >> 1;
        }
        else if (iPrev >= 0)
        {
          piRef1 = piAdiLine1[iCurr*iUnitSize-1];
          piRef2 = piAdiLine2[iCurr*iUnitSize-1];
        }
        else if (iNext < iTotalUnits)
        {
          piRef1 = piAdiLine1[iNext*iUnitSize];
          piRef2 = piAdiLine2[iNext*iUnitSize];
        }
        else
          printf("\nERROR! No valid samples to interpolate.\n");

        // Pad unavailable samples with new value
        while (iCurr < iNext)
        {
          for (i=0; i<iUnitSize; i++)
          {
            piAdiLineTemp1[i] = piRef1;
            piAdiLineTemp2[i] = piRef2;
          }
          piAdiLineTemp1 += iUnitSize;
          piAdiLineTemp2 += iUnitSize;
          iPrev++;
          iCurr++;
        }
        iNext++;
      }
    }

    // Copy processed samples
    piAdiLineTemp1 = piAdiLine1 + uiHeight + iUnitSize - 2;
    piAdiLineTemp2 = piAdiLine2 + uiHeight + iUnitSize - 2;
    for (i=0; i<uiWidth; i++)
    {
      piAdiTemp1[i] = piAdiLineTemp1[i];
      piAdiTemp2[i] = piAdiLineTemp2[i];
    }
    piAdiLineTemp1 = piAdiLine1 + uiHeight - 1;
    piAdiLineTemp2 = piAdiLine2 + uiHeight - 1;
    for (i=1; i<uiHeight; i++)
    {
      piAdiTemp3[i-1] = piAdiLineTemp1[-i];
      piAdiTemp4[i-1] = piAdiLineTemp2[-i];
    }

    delete [] piAdiLine1;
    piAdiLine1 = NULL;

    delete [] piAdiLine2;
    piAdiLine2 = NULL;
  }
}
#endif

#endif // REFERENCE_SAMPLE_PADDING

Int* TComPattern::getAdiOrgBuf( Int iCuWidth, Int iCuHeight, Int* piAdiBuf)
{
  return piAdiBuf;
}

Int* TComPattern::getAdiCbBuf( Int iCuWidth, Int iCuHeight, Int* piAdiBuf)
{
  return piAdiBuf;
}

Int* TComPattern::getAdiCrBuf(Int iCuWidth,Int iCuHeight, Int* piAdiBuf)
{
  return piAdiBuf+(iCuWidth*2+1)*(iCuHeight*2+1);
}

#if QC_MDIS
Int* TComPattern::getPredictorPtr ( UInt uiDirMode, UInt uiWidthBits, Int iCuWidth, Int iCuHeight, Int* piAdiBuf )
{
#if MN_MDIS_SIMPLIFICATION
  static const UChar g_aucIntraFilter[7][34] =
  {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //2x2
      {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //4x4
      {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //8x8
      {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //16x16
      {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //32x32
      {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //64x64
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  //128x128
  };
#else
  static const UChar g_aucIntraFilter[7][34] =
  {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //2x2
      {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //4x4
      {0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //8x8
      {0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //16x16
      {0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, //32x32
      {0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //64x64
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  //128x128
  };
#endif

  Int* piSrc;
#if ADD_PLANAR_MODE
  mapPlanartoDC( uiDirMode );
#endif
  UChar ucFiltIdx = g_aucIntraFilter[uiWidthBits][uiDirMode];

#if MN_MDIS_SIMPLIFICATION
  assert( ucFiltIdx <= 1 );
#else
  assert( ucFiltIdx <= 2 );
#endif

  piSrc = getAdiOrgBuf( iCuWidth, iCuHeight, piAdiBuf );

  if ( ucFiltIdx )
  {
#if MN_MDIS_SIMPLIFICATION
    piSrc += ((iCuWidth << 1) + 1) * ((iCuHeight << 1) + 1);
#else
    piSrc += (((iCuWidth << 1) + 1) * ((iCuHeight << 1) + 1) << (ucFiltIdx - 1));
#endif
  }

  return piSrc;
}
#endif //QC_MDIS

#if CONSTRAINED_INTRA_PRED
Bool TComPattern::isAboveLeftAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT )
{
  Bool bAboveLeftFlag;
  UInt uiPartAboveLeft;
  TComDataCU* pcCUAboveLeft = pcCU->getPUAboveLeft( uiPartAboveLeft, uiPartIdxLT, true, false );
  bAboveLeftFlag = ( pcCUAboveLeft && pcCUAboveLeft->getPredictionMode( uiPartAboveLeft ) == MODE_INTRA );
  return bAboveLeftFlag;
}

#if REFERENCE_SAMPLE_PADDING
Int TComPattern::isAboveAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxRT, Bool *bValidFlags )
#else
Bool TComPattern::isAboveAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxRT )
#endif
{
  const UInt uiRasterPartBegin = g_auiZscanToRaster[uiPartIdxLT];
  const UInt uiRasterPartEnd = g_auiZscanToRaster[uiPartIdxRT]+1;
  const UInt uiIdxStep = 1;
#if REFERENCE_SAMPLE_PADDING
  Bool *pbValidFlags = bValidFlags;
  Int iNumIntra = 0;
#else
  Bool bAboveFlag = true;
#endif
#if MN_DC_PRED_FILTER
  m_bAboveFlagForDCFilt = true;
#endif

  for ( UInt uiRasterPart = uiRasterPartBegin; uiRasterPart < uiRasterPartEnd; uiRasterPart += uiIdxStep )
  {
    UInt uiPartAbove;
    TComDataCU* pcCUAbove = pcCU->getPUAbove( uiPartAbove, g_auiRasterToZscan[uiRasterPart], true, false );
#if REFERENCE_SAMPLE_PADDING
    if ( pcCUAbove && pcCUAbove->getPredictionMode( uiPartAbove ) == MODE_INTRA )
    {
      iNumIntra++;
      *pbValidFlags = true;
    }
    else
    {
      *pbValidFlags = false;
#if MN_DC_PRED_FILTER
      m_bAboveFlagForDCFilt = false;
#endif
    }
    pbValidFlags++;
#else
    if ( !pcCUAbove || pcCUAbove->getPredictionMode( uiPartAbove ) != MODE_INTRA )
    {
      bAboveFlag = false;
#if MN_DC_PRED_FILTER
      m_bAboveFlagForDCFilt = false;
#endif
      break;
    }
#endif
  }
#if REFERENCE_SAMPLE_PADDING
  return iNumIntra;
#else
  return bAboveFlag;
#endif
}

#if REFERENCE_SAMPLE_PADDING
Int TComPattern::isLeftAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxLB, Bool *bValidFlags )
#else
Bool TComPattern::isLeftAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxLB )
#endif
{
  const UInt uiRasterPartBegin = g_auiZscanToRaster[uiPartIdxLT];
  const UInt uiRasterPartEnd = g_auiZscanToRaster[uiPartIdxLB]+1;
  const UInt uiIdxStep = pcCU->getPic()->getNumPartInWidth();
#if REFERENCE_SAMPLE_PADDING
  Bool *pbValidFlags = bValidFlags;
  Int iNumIntra = 0;
#else
  Bool bLeftFlag = true;
#endif
#if MN_DC_PRED_FILTER
  m_bLeftFlagForDCFilt = true;
#endif

  for ( UInt uiRasterPart = uiRasterPartBegin; uiRasterPart < uiRasterPartEnd; uiRasterPart += uiIdxStep )
  {
    UInt uiPartLeft;
    TComDataCU* pcCULeft = pcCU->getPULeft( uiPartLeft, g_auiRasterToZscan[uiRasterPart], true, false );
#if REFERENCE_SAMPLE_PADDING
    if ( pcCULeft && pcCULeft->getPredictionMode( uiPartLeft ) == MODE_INTRA )
    {
      iNumIntra++;
      *pbValidFlags = true;
    }
    else
    {
      *pbValidFlags = false;
#if MN_DC_PRED_FILTER
      m_bLeftFlagForDCFilt = false;
#endif
    }
    pbValidFlags--; // opposite direction
#else
    if ( !pcCULeft || pcCULeft->getPredictionMode( uiPartLeft ) != MODE_INTRA )
    {
      bLeftFlag = false;
#if MN_DC_PRED_FILTER
      m_bLeftFlagForDCFilt = false;
#endif
      break;
    }
#endif
  }

#if REFERENCE_SAMPLE_PADDING
  return iNumIntra;
#else
  return bLeftFlag;
#endif
}

#if REFERENCE_SAMPLE_PADDING
Int TComPattern::isAboveRightAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxRT, Bool *bValidFlags )
#else
Bool TComPattern::isAboveRightAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxRT )
#endif
{
  const UInt uiNumUnitsInPU = g_auiZscanToRaster[uiPartIdxRT] - g_auiZscanToRaster[uiPartIdxLT] + 1;
  const UInt uiPuWidth = uiNumUnitsInPU * pcCU->getPic()->getMinCUWidth();
#if REFERENCE_SAMPLE_PADDING
  Bool *pbValidFlags = bValidFlags;
  Int iNumIntra = 0;
#else
  Bool bAboveRightFlag = true;
#endif

  for ( UInt uiOffset = 1; uiOffset <= uiNumUnitsInPU; uiOffset++ )
  {
    UInt uiPartAboveRight;
    TComDataCU* pcCUAboveRight = pcCU->getPUAboveRightAdi( uiPartAboveRight, uiPuWidth, uiPartIdxRT, uiOffset, true, false );
#if REFERENCE_SAMPLE_PADDING
    if ( pcCUAboveRight && pcCUAboveRight->getPredictionMode( uiPartAboveRight ) == MODE_INTRA )
    {
      iNumIntra++;
      *pbValidFlags = true;
    }
    else
    {
      *pbValidFlags = false;
    }
    pbValidFlags++;
#else
    if ( !pcCUAboveRight || pcCUAboveRight->getPredictionMode( uiPartAboveRight ) != MODE_INTRA )
    {
      bAboveRightFlag = false;
      break;
    }
#endif
  }

#if REFERENCE_SAMPLE_PADDING
  return iNumIntra;
#else
  return bAboveRightFlag;
#endif
}

#if REFERENCE_SAMPLE_PADDING
Int TComPattern::isBelowLeftAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxLB, Bool *bValidFlags )
#else
Bool TComPattern::isBelowLeftAvailableForCIP( TComDataCU* pcCU, UInt uiPartIdxLT, UInt uiPartIdxLB )
#endif
{
  const UInt uiNumUnitsInPU = (g_auiZscanToRaster[uiPartIdxLB] - g_auiZscanToRaster[uiPartIdxLT]) / pcCU->getPic()->getNumPartInWidth() + 1;
  const UInt uiPuHeight = uiNumUnitsInPU * pcCU->getPic()->getMinCUHeight();
#if REFERENCE_SAMPLE_PADDING
  Bool *pbValidFlags = bValidFlags;
  Int iNumIntra = 0;
#else
  Bool bBelowLeftFlag = true;
#endif

  for ( UInt uiOffset = 1; uiOffset <= uiNumUnitsInPU; uiOffset++ )
  {
    UInt uiPartBelowLeft;
    TComDataCU* pcCUBelowLeft = pcCU->getPUBelowLeftAdi( uiPartBelowLeft, uiPuHeight, uiPartIdxLB, uiOffset, true, false );
#if REFERENCE_SAMPLE_PADDING
    if ( pcCUBelowLeft && pcCUBelowLeft->getPredictionMode( uiPartBelowLeft ) == MODE_INTRA )
    {
      iNumIntra++;
      *pbValidFlags = true;
    }
    else
    {
      *pbValidFlags = false;
    }
    pbValidFlags--; // opposite direction
#else
    if ( !pcCUBelowLeft || pcCUBelowLeft->getPredictionMode( uiPartBelowLeft ) != MODE_INTRA )
    {
      bBelowLeftFlag = false;
      break;
    }
#endif
  }

#if REFERENCE_SAMPLE_PADDING
  return iNumIntra;
#else
  return bBelowLeftFlag;
#endif
}
#endif //CONSTRAINED_INTRA_PRED
