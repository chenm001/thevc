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
    
    if( uiCurrPicPelX + uiWidth < pcPic->getSlice()->getSPS()->getWidth() )
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
  
  m_cPatternY .setPatternParamCU( pcCU, 0, uiWidth,      uiHeight,      uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
  m_cPatternCb.setPatternParamCU( pcCU, 1, uiWidth >> 1, uiHeight >> 1, uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
  m_cPatternCr.setPatternParamCU( pcCU, 2, uiWidth >> 1, uiHeight >> 1, uiOffsetLeft, uiOffsetRight, uiOffsetAbove, 0, uiPartDepth, uiAbsPartIdx );
}

Void TComPattern::initAdiPattern( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piAdiBuf, Int iOrgBufStride, Int iOrgBufHeight, Bool& bAbove, Bool& bLeft )
{
  Pel*  piRoiOrigin;
  Pel*  piRoiTemp;
  Int*  piAdiTemp;
  UInt  uiCuWidth   = pcCU->getWidth(0) >> uiPartDepth;
  UInt  uiCuHeight  = pcCU->getHeight(0)>> uiPartDepth;
  UInt  uiCuWidth2  = uiCuWidth<<1;
  UInt  uiCuHeight2 = uiCuHeight<<1;
  UInt  uiWidth;
  UInt  uiHeight;
  Int   iPicStride = pcCU->getPic()->getStride();
  Int   i;
  Int   iCuAddr;
  Bool  bAboveFlag      = false;
  Bool  bAboveRightFlag = false;
  Bool  bLeftFlag       = false;
  Bool  bBelowLeftFlag  = false;
  
  iCuAddr = pcCU->getAddr();
  
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB, uiPartDum;
  
  pcCU->deriveLeftRightTopIdxAdi( uiPartIdxLT, uiPartIdxRT, uiZorderIdxInPart, uiPartDepth );
  pcCU->deriveLeftBottomIdxAdi  ( uiPartIdxLB,              uiZorderIdxInPart, uiPartDepth );
  
  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT ) ) bAboveFlag      = true;
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT ) ) bAboveRightFlag = true;
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT ) ) bLeftFlag       = true;
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB ) ) bBelowLeftFlag  = true;
  
  bAbove = bAboveFlag;
  bLeft  = bLeftFlag;
  
  uiWidth=uiCuWidth2+1;
  uiHeight=uiCuHeight2+1;
  
  if (((uiWidth<<2)>iOrgBufStride)||((uiHeight<<2)>iOrgBufHeight))
    return;
  
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  
  piAdiTemp   = piAdiBuf;
  Int iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );
  
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
    // BB: fill top left border corner with rec. sample
    if (bLeftFlag)//BB: why left not top left?
    {
      piRoiTemp=piRoiOrigin-iPicStride-1;
      piAdiTemp[0]=piRoiTemp[0];
    }
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

#endif //QC_MDIS
  
}

Void TComPattern::initAdiPatternChroma( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piAdiBuf, Int iOrgBufStride, Int iOrgBufHeight, Bool& bAbove, Bool& bLeft )
{
  Pel*  piRoiOrigin;
  Pel*  piRoiTemp;
  Int*  piAdiTemp;
  UInt  uiCuWidth  = pcCU->getWidth (0) >> uiPartDepth;
  UInt  uiCuHeight = pcCU->getHeight(0) >> uiPartDepth;
  UInt  uiWidth;
  UInt  uiHeight;
  Int   iPicStride = pcCU->getPic()->getCStride();
  Int   i;
  Int   iCuAddr;
  Bool  bAboveFlag=false;
  Bool  bAboveRightFlag=false;
  Bool  bLeftFlag=false;
  Bool  bBelowLeftFlag=false;
  
  iCuAddr = pcCU->getAddr();
  
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB,uiPartDum;
  
  pcCU->deriveLeftRightTopIdxAdi( uiPartIdxLT, uiPartIdxRT, uiZorderIdxInPart, uiPartDepth );
  pcCU->deriveLeftBottomIdxAdi  ( uiPartIdxLB,              uiZorderIdxInPart, uiPartDepth );
  
  if( pcCU->getPUAbove     ( uiPartDum, uiPartIdxLT ) ) bAboveFlag      = true;
  if( pcCU->getPUAboveRightAdi( uiPartDum,uiCuWidth, uiPartIdxRT ) ) bAboveRightFlag = true;
  if( pcCU->getPULeft      ( uiPartDum, uiPartIdxLT ) ) bLeftFlag       = true;
  if( pcCU->getPUBelowLeftAdi (uiPartDum, uiCuHeight, uiPartIdxLB ) ) bBelowLeftFlag  = true;
  
  bAbove = bAboveFlag;
  bLeft  = bLeftFlag;
  
  uiCuWidth=uiCuWidth>>1;  // for chroma
  uiCuHeight=uiCuHeight>>1;  // for chroma
  
  uiWidth=uiCuWidth*2+1;
  uiHeight=uiCuHeight*2+1;
  
  if ((4*uiWidth>iOrgBufStride)||(4*uiHeight>iOrgBufHeight))  return;
  
  Int iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );
  
  // get Cb pattern
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  piAdiTemp   = piAdiBuf;
  
  
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
    if (bLeftFlag)
    {
      piRoiTemp=piRoiOrigin-iPicStride-1;
      piAdiTemp[0]=piRoiTemp[0];
    }
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
  
  // get Cr pattern
  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);
  piAdiTemp   = piAdiBuf+uiWidth*uiHeight;
  
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
    if (bLeftFlag)
    {
      piRoiTemp=piRoiOrigin-iPicStride-1;
      piAdiTemp[0]=piRoiTemp[0];
    }
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
}

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

  Int* piSrc;
  UChar ucFiltIdx = g_aucIntraFilter[uiWidthBits][uiDirMode];

  assert( ucFiltIdx >= 0 && ucFiltIdx <= 2 );

  piSrc = getAdiOrgBuf( iCuWidth, iCuHeight, piAdiBuf );

  if ( ucFiltIdx )
  {
    piSrc += (((iCuWidth << 1) + 1) * ((iCuHeight << 1) + 1) << (ucFiltIdx - 1));
  }

  return piSrc;
}
#endif //QC_MDIS
