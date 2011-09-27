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

/** \file     TComSampleAdaptiveOffset.cpp
    \brief    sample adaptive offset class
*/

#include "TComSampleAdaptiveOffset.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Tables
// ====================================================================================================================

TComSampleAdaptiveOffset::TComSampleAdaptiveOffset()
{
  m_pClipTable = NULL;
  m_pClipTableBase = NULL;
  m_ppLumaTableBo0 = NULL;
  m_ppLumaTableBo1 = NULL;

  m_iUpBuff1 = NULL;
  m_iUpBuff2 = NULL;
  m_iUpBufft = NULL;
  ipSwap = NULL;
  m_bIsFineSliceCu = NULL;
  m_pcPicYuvMap = NULL;

  m_pTmpU1 = NULL;
  m_pTmpU2 = NULL;
  m_pTmpL1 = NULL;
  m_pTmpL2 = NULL;
  m_iLcuPartIdx = NULL;
}

TComSampleAdaptiveOffset::~TComSampleAdaptiveOffset()
{

}

const Int TComSampleAdaptiveOffset::m_aiNumPartsInRow[5] =
{
  1,   //level 0
  2,   //level 1
  4,   //level 2
  8,   //level 3
  16   //level 4
};

const Int TComSampleAdaptiveOffset::m_aiNumPartsLevel[5] =
{
  1,   //level 0
  4,   //level 1
  16,  //level 2
  64,  //level 3
  256  //level 4
};

const Int TComSampleAdaptiveOffset::m_aiNumCulPartsLevel[5] =
{
  1,   //level 0
  5,   //level 1
  21,  //level 2
  85,  //level 3
  341, //level 4
};

const UInt TComSampleAdaptiveOffset::m_auiEoTable[9] =
{
  1, //0    
  2, //1   
  0, //2
  3, //3
  4, //4
  0, //5  
  0, //6  
  0, //7 
  0
};

const UInt TComSampleAdaptiveOffset::m_iWeightSao[MAX_NUM_SAO_TYPE] =
{
  2,
  2,
  2,
  2,
  1,
  1
};

const UInt TComSampleAdaptiveOffset::m_auiEoTable2D[9] =
{
  1, //0    
  2, //1   
  3, //2
  0, //3
  0, //4
  0, //5  
  4, //6  
  5, //7 
  6
};

Int TComSampleAdaptiveOffset::m_iNumClass[MAX_NUM_SAO_TYPE] =
{
  SAO_EO_LEN,
  SAO_EO_LEN,
  SAO_EO_LEN,
  SAO_EO_LEN,
  SAO_BO_LEN,
  SAO_BO_LEN
};

UInt TComSampleAdaptiveOffset::m_uiMaxDepth = SAO_MAX_DEPTH;


/** convert Level Row Col to Idx
 * \param   level,  row,  col
 */
Int  TComSampleAdaptiveOffset::convertLevelRowCol2Idx(int level, int row, int col)
{
  Int idx;
  if (level == 0)
  {
    idx = 0;
  }
  else if (level == 1)
  {
    idx = 1 + row*2 + col;
  }
  else if (level == 2)
  {
    idx = 5 + row*4 + col;
  }
  else if (level == 3)
  {
    idx = 21 + row*8 + col;
  }
  else // (level == 4)
  {
    idx = 85 + row*16 + col;
  }
  return idx;
}
/** convert quadtree Idx to Level, Row, and Col
 * \param  idx,  *level,  *row,  *col
 */
void TComSampleAdaptiveOffset::convertIdx2LevelRowCol(int idx, int *level, int *row, int *col)
{
  if (idx == 0)
  {
    *level = 0;
    *row = 0;
    *col = 0;
  }
  else if (idx>=1 && idx<=4)
  {
    *level = 1;
    *row = (idx-1) / 2;
    *col = (idx-1) % 2;
  }
  else if (idx>=5 && idx<=20)
  {
    *level = 2;
    *row = (idx-5) / 4;
    *col = (idx-5) % 4;
  }
  else if (idx>=21 && idx<=84)
  {
    *level = 3;
    *row = (idx-21) / 8;
    *col = (idx-21) % 8;
  }
  else // (idx>=85 && idx<=340)
  {
    *level = 4;
    *row = (idx-85) / 16;
    *col = (idx-85) % 16;
  }
}
/** create SampleAdaptiveOffset memory.
 * \param 
 */
Void TComSampleAdaptiveOffset::create( UInt uiSourceWidth, UInt uiSourceHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth)
{
  m_iPicWidth  = uiSourceWidth;
  m_iPicHeight = uiSourceHeight;

  m_uiMaxCUWidth  = uiMaxCUWidth;
  m_uiMaxCUHeight = uiMaxCUHeight;

  m_iNumCuInWidth  = m_iPicWidth / m_uiMaxCUWidth;
  m_iNumCuInWidth += ( m_iPicWidth % m_uiMaxCUWidth ) ? 1 : 0;

  m_iNumCuInHeight  = m_iPicHeight / m_uiMaxCUHeight;
  m_iNumCuInHeight += ( m_iPicHeight % m_uiMaxCUHeight ) ? 1 : 0;

  Int iMaxSplitLevelHeight = (Int)(logf((float)m_iNumCuInHeight)/logf(2.0));
  Int iMaxSplitLevelWidth  = (Int)(logf((float)m_iNumCuInWidth )/logf(2.0));

  m_uiMaxSplitLevel = (iMaxSplitLevelHeight < iMaxSplitLevelWidth)?(iMaxSplitLevelHeight):(iMaxSplitLevelWidth);
  m_uiMaxSplitLevel = (m_uiMaxSplitLevel< m_uiMaxDepth)?(m_uiMaxSplitLevel):(m_uiMaxDepth);
  m_iNumTotalParts  = m_aiNumCulPartsLevel[m_uiMaxSplitLevel];

  UInt auiTable[2][LUMA_GROUP_NUM] = 
  {{0, 0, 0, 0, 0, 0, 0, 0,
  1, 2, 3, 4, 5, 6, 7, 8,
  9,10,11,12,13,14,15,16,
  0, 0, 0, 0, 0, 0, 0, 0},

  {1, 2, 3, 4, 5, 6, 7, 8,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  9,10,11,12,13,14,15,16}};

  UInt uiInternalBitDepth = g_uiBitDepth+g_uiBitIncrement;
  UInt uiPixelRange = 1<<uiInternalBitDepth;
  UInt uiBoRangeShift = uiInternalBitDepth - SAO_BO_BITS;

  m_ppLumaTableBo0 = new Pel [uiPixelRange];
  m_ppLumaTableBo1 = new Pel [uiPixelRange];
  for (Int k2=0; k2<uiPixelRange; k2++)
  {
    m_ppLumaTableBo0[k2] = auiTable[0][k2>>uiBoRangeShift];
    m_ppLumaTableBo1[k2] = auiTable[1][k2>>uiBoRangeShift];
  }

  m_iUpBuff1 = new Int[m_iPicWidth+2];
  m_iUpBuff2 = new Int[m_iPicWidth+2];
  m_iUpBufft = new Int[m_iPicWidth+2];

  m_iUpBuff1++;
  m_iUpBuff2++;
  m_iUpBufft++;
  Pel i;

  UInt uiMaxY  = g_uiIBDI_MAX;
  UInt uiMinY  = 0;

  Int iCRangeExt = uiMaxY>>1;

  m_pClipTableBase = new Pel[uiMaxY+2*iCRangeExt];
  m_iOffsetBo      = new Int[uiMaxY+2*iCRangeExt];

  for(i=0;i<(uiMinY+iCRangeExt);i++)
  {
    m_pClipTableBase[i] = uiMinY;
  }

  for(i=uiMinY+iCRangeExt;i<(uiMaxY+  iCRangeExt);i++)
  {
    m_pClipTableBase[i] = i-iCRangeExt;
  }

  for(i=uiMaxY+iCRangeExt;i<(uiMaxY+2*iCRangeExt);i++)
  {
    m_pClipTableBase[i] = uiMaxY;
  }

  m_pClipTable = &(m_pClipTableBase[iCRangeExt]);

#if SAO_FGS_NIF
  m_pcPicYuvMap = new TComPicYuv;
  m_pcPicYuvMap->create( m_iPicWidth, m_iPicHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, uiMaxCUDepth );
  m_bIsFineSliceCu = new Bool [m_iNumCuInWidth*m_iNumCuInHeight];
#endif

#if SAO_CROSS_LCU_BOUNDARIES
  m_iLcuPartIdx = new Int [m_iNumCuInHeight*m_iNumCuInWidth];
  m_pTmpL1 = new Pel [m_uiMaxCUHeight+1];
  m_pTmpL2 = new Pel [m_uiMaxCUHeight+1];
  m_pTmpU1 = new Pel [m_iPicWidth];
  m_pTmpU2 = new Pel [m_iPicWidth];
#endif
}

/** destroy SampleAdaptiveOffset memory.
 * \param 
 */
Void TComSampleAdaptiveOffset::destroy()
{
  if (m_pClipTableBase)
  {
    delete [] m_pClipTableBase; m_pClipTableBase = NULL;
  }
  if (m_iOffsetBo)
  {
    delete [] m_iOffsetBo; m_iOffsetBo = NULL;
  }
  if (m_ppLumaTableBo0)
  {
    delete[] m_ppLumaTableBo0; m_ppLumaTableBo0 = NULL;
  }
  if (m_ppLumaTableBo1)
  {
    delete[] m_ppLumaTableBo1; m_ppLumaTableBo1 = NULL;
  }

  m_iUpBuff1--;
  m_iUpBuff2--;
  m_iUpBufft--;

  if (m_iUpBuff1)
  {
    delete [] m_iUpBuff1; m_iUpBuff1 = NULL;
  }
  if (m_iUpBuff2)
  {
    delete [] m_iUpBuff2; m_iUpBuff2 = NULL;
  }
  if (m_iUpBufft)
  {
    delete [] m_iUpBufft; m_iUpBufft = NULL;
  }
#if SAO_FGS_NIF
  if (m_pcPicYuvMap)
  {
    m_pcPicYuvMap->destroy();
    delete m_pcPicYuvMap; m_pcPicYuvMap = NULL;
  }
  if (m_bIsFineSliceCu)
  {
    delete [] m_bIsFineSliceCu; m_bIsFineSliceCu = NULL;
  }
#endif
#if SAO_CROSS_LCU_BOUNDARIES
  if (m_pTmpL1)
  {
    delete [] m_pTmpL1; m_pTmpL1 = NULL;
  }
  if (m_pTmpL2)
  {
    delete [] m_pTmpL2; m_pTmpL2 = NULL;
  }
  if (m_pTmpU1)
  {
    delete [] m_pTmpU1; m_pTmpU1 = NULL;
  }
  if (m_pTmpU2)
  {
    delete [] m_pTmpU2; m_pTmpU2 = NULL;
  }
  if(m_iLcuPartIdx)
  {
    delete []m_iLcuPartIdx; m_iLcuPartIdx = NULL;
  }
#endif
}

/** allocate memory for SAO parameters
 * \param    *pcSaoParam
 */
Void TComSampleAdaptiveOffset::allocSaoParam(SAOParam *pcSaoParam)
{
  pcSaoParam->iMaxSplitLevel = m_uiMaxSplitLevel;
  pcSaoParam->psSaoPart[0] = new SAOQTPart[ m_aiNumCulPartsLevel[pcSaoParam->iMaxSplitLevel] ];
  initSAOParam(pcSaoParam, 0, 0, 0, -1, 0, m_iNumCuInWidth-1,  0, m_iNumCuInHeight-1,0);
#if SAO_CHROMA
  pcSaoParam->psSaoPart[1] = new SAOQTPart[ m_aiNumCulPartsLevel[pcSaoParam->iMaxSplitLevel] ];
  pcSaoParam->psSaoPart[2] = new SAOQTPart[ m_aiNumCulPartsLevel[pcSaoParam->iMaxSplitLevel] ];
  initSAOParam(pcSaoParam, 0, 0, 0, -1, 0, m_iNumCuInWidth-1,  0, m_iNumCuInHeight-1,1);
  initSAOParam(pcSaoParam, 0, 0, 0, -1, 0, m_iNumCuInWidth-1,  0, m_iNumCuInHeight-1,2);
#endif
  for(Int j=0;j<MAX_NUM_SAO_TYPE;j++)
  {
    pcSaoParam->iNumClass[j] = m_iNumClass[j];
  }
}

/** initialize SAO parameters
 * \param    *pcSaoParam,  iPartLevel,  iPartRow,  iPartCol,  iParentPartIdx,  StartCUX,  EndCUX,  StartCUY,  EndCUY,  iYCbCr
 */
Void TComSampleAdaptiveOffset::initSAOParam(SAOParam *pcSaoParam, Int iPartLevel, Int iPartRow, Int iPartCol, Int iParentPartIdx, Int StartCUX, Int EndCUX, Int StartCUY, Int EndCUY, Int iYCbCr)
{
  Int j;
  Int iPartIdx = convertLevelRowCol2Idx(iPartLevel, iPartRow, iPartCol);

  SAOQTPart* pSaoPart;

  pSaoPart = &(pcSaoParam->psSaoPart[iYCbCr][iPartIdx]);

  pSaoPart->PartIdx   = iPartIdx;
  pSaoPart->PartLevel = iPartLevel;
  pSaoPart->PartRow   = iPartRow;
  pSaoPart->PartCol   = iPartCol;

  pSaoPart->StartCUX  = StartCUX;
  pSaoPart->EndCUX    = EndCUX;
  pSaoPart->StartCUY  = StartCUY;
  pSaoPart->EndCUY    = EndCUY;

  pSaoPart->UpPartIdx = iParentPartIdx;

  pSaoPart->bEnableFlag =  0;
  pSaoPart->iBestType   = -1;
  pSaoPart->iLength     =  0;

  for (j=0;j<MAX_NUM_SAO_CLASS;j++)
  {
    pSaoPart->iOffset[j] = 0;
  }

  if(pSaoPart->PartLevel != m_uiMaxSplitLevel)
  {
    Int DownLevel    = (iPartLevel+1 );
    Int DownRowStart = (iPartRow << 1);
    Int DownColStart = (iPartCol << 1);

    Int iDownRowIdx, iDownColIdx;
    Int NumCUWidth,  NumCUHeight;
    Int NumCULeft;
    Int NumCUTop;

    Int DownStartCUX, DownStartCUY;
    Int DownEndCUX, DownEndCUY;

    NumCUWidth  = EndCUX - StartCUX +1;
    NumCUHeight = EndCUY - StartCUY +1;
    NumCULeft   = (NumCUWidth  >> 1);
    NumCUTop    = (NumCUHeight >> 1);

    DownStartCUX= StartCUX;
    DownEndCUX  = DownStartCUX + NumCULeft - 1;
    DownStartCUY= StartCUY;
    DownEndCUY  = DownStartCUY + NumCUTop  - 1;
    iDownRowIdx = DownRowStart + 0;
    iDownColIdx = DownColStart + 0;

    pSaoPart->DownPartsIdx[0]= convertLevelRowCol2Idx(DownLevel, iDownRowIdx, iDownColIdx);

    initSAOParam(pcSaoParam, DownLevel, iDownRowIdx, iDownColIdx, iPartIdx, DownStartCUX, DownEndCUX, DownStartCUY, DownEndCUY, iYCbCr);

    DownStartCUX = StartCUX + NumCULeft;
    DownEndCUX   = EndCUX;
    DownStartCUY = StartCUY;
    DownEndCUY   = DownStartCUY + NumCUTop -1;
    iDownRowIdx  = DownRowStart + 0;
    iDownColIdx  = DownColStart + 1;

    pSaoPart->DownPartsIdx[1] = convertLevelRowCol2Idx(DownLevel, iDownRowIdx, iDownColIdx);

    initSAOParam(pcSaoParam, DownLevel, iDownRowIdx, iDownColIdx, iPartIdx,  DownStartCUX, DownEndCUX, DownStartCUY, DownEndCUY, iYCbCr);

    DownStartCUX = StartCUX;
    DownEndCUX   = DownStartCUX + NumCULeft -1;
    DownStartCUY = StartCUY + NumCUTop;
    DownEndCUY   = EndCUY;
    iDownRowIdx  = DownRowStart + 1;
    iDownColIdx  = DownColStart + 0;

    pSaoPart->DownPartsIdx[2] = convertLevelRowCol2Idx(DownLevel, iDownRowIdx, iDownColIdx);

    initSAOParam(pcSaoParam, DownLevel, iDownRowIdx, iDownColIdx, iPartIdx, DownStartCUX, DownEndCUX, DownStartCUY, DownEndCUY, iYCbCr);

    DownStartCUX = StartCUX+ NumCULeft;
    DownEndCUX   = EndCUX;
    DownStartCUY = StartCUY + NumCUTop;
    DownEndCUY   = EndCUY;
    iDownRowIdx  = DownRowStart + 1;
    iDownColIdx  = DownColStart + 1;

    pSaoPart->DownPartsIdx[3] = convertLevelRowCol2Idx(DownLevel, iDownRowIdx, iDownColIdx);

    initSAOParam(pcSaoParam, DownLevel, iDownRowIdx, iDownColIdx, iPartIdx,DownStartCUX, DownEndCUX, DownStartCUY, DownEndCUY, iYCbCr);
  }
  else
  {
    pSaoPart->DownPartsIdx[0]=pSaoPart->DownPartsIdx[1]= pSaoPart->DownPartsIdx[2]= pSaoPart->DownPartsIdx[3]= -1; 
  }
}

/** free memory of SAO parameters
 * \param   pcSaoParam
 */
Void TComSampleAdaptiveOffset::freeSaoParam(SAOParam *pcSaoParam)
{
  delete [] pcSaoParam->psSaoPart[0];
  delete [] pcSaoParam->psSaoPart[1];
  delete [] pcSaoParam->psSaoPart[2];
} 

/** reset SAO parameters
 * \param   pcSaoParam
 */
Void TComSampleAdaptiveOffset::resetSAOParam(SAOParam *pcSaoParam)
{
#if SAO_CHROMA
  Int iNumComponet = 3;
#else
  Int iNumComponet = 1;
#endif
  for(Int c=0; c<iNumComponet; c++)
  {
    pcSaoParam->bSaoFlag[c] = 0;
    for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
    {
      pcSaoParam->psSaoPart[c][i].bEnableFlag   =  0;
      pcSaoParam->psSaoPart[c][i].iBestType     = -1;
      pcSaoParam->psSaoPart[c][i].iLength       =  0;
      pcSaoParam->psSaoPart[c][i].bSplit        = false; 
      pcSaoParam->psSaoPart[c][i].bProcessed    = false;
      pcSaoParam->psSaoPart[c][i].dMinCost      = MAX_DOUBLE;
      pcSaoParam->psSaoPart[c][i].iMinDist      = MAX_INT;
      pcSaoParam->psSaoPart[c][i].iMinRate      = MAX_INT;
      for (Int j=0;j<MAX_NUM_SAO_CLASS;j++)
      {
        pcSaoParam->psSaoPart[c][i].iOffset[j] = 0;
        pcSaoParam->psSaoPart[c][i].iOffset[j] = 0;
        pcSaoParam->psSaoPart[c][i].iOffset[j] = 0;
      }
    }
  }
}

/** get the sign of input variable
 * \param   x
 */
inline int xSign(int x)
{
  return ((x >> 31) | ((int)( (((unsigned int) -x)) >> 31)));
}

#if SAO_FGS_NIF
/** create Slice Map
 * \param   iSliceIdx, uiStartAddr, uiEndAddr
 */
Void TComSampleAdaptiveOffset::createSliceMap(UInt iSliceIdx, UInt uiStartAddr, UInt uiEndAddr)
{
  Pel* pMap;    
#if SAO_CHROMA
  Pel* pMapCb;
  Pel* pMapCr;
  Int  iStrideC = m_pcPicYuvMap->getCStride();
#endif
  Int  iStride = m_pcPicYuvMap->getStride();
  UInt uiLPelX;
  UInt uiTPelY;
  UInt uiStartLCU;
  UInt uiEndLCU;
  UInt uiFirstCUInStartLCU;
  UInt uiLastCUInEndLCU;
  UInt uiNumSUInLCU = m_pcPic->getNumPartInHeight()*m_pcPic->getNumPartInWidth();
  Int i,k,l;
  UInt uiMinCUWidth  = m_pcPic->getMinCUWidth(); 
  UInt uiMinCUHeight = m_pcPic->getMinCUHeight();
  UInt uiMaxCUWidth  = g_uiMaxCUWidth;
  UInt uiMaxCUHeight = g_uiMaxCUHeight;

  uiStartLCU          = uiStartAddr / uiNumSUInLCU;
  uiFirstCUInStartLCU = uiStartAddr % uiNumSUInLCU;
  uiEndLCU            = uiEndAddr   / uiNumSUInLCU;
  uiLastCUInEndLCU    = uiEndAddr   % uiNumSUInLCU;
  if (uiFirstCUInStartLCU!= 0)
  {
    m_bIsFineSliceCu[uiStartLCU] = 1;
  }
  if (uiLastCUInEndLCU!= uiNumSUInLCU-1)
  {
    m_bIsFineSliceCu[uiEndLCU] = 1;
  }

  for (Int iLcuIdx = uiStartLCU; iLcuIdx<= uiEndLCU; iLcuIdx++)
  {
    if (m_bIsFineSliceCu[iLcuIdx])
    {
#if TILES
      pMap = m_pcPicYuvMap->getLumaAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
#if SAO_CHROMA
      pMapCb = m_pcPicYuvMap->getCbAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
      pMapCr = m_pcPicYuvMap->getCrAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
#endif
#else
      pMap = m_pcPicYuvMap->getLumaAddr(iLcuIdx);
#if SAO_CHROMA
      pMapCb = m_pcPicYuvMap->getCbAddr(iLcuIdx);
      pMapCr = m_pcPicYuvMap->getCrAddr(iLcuIdx);
#endif
#endif
      UInt uiFirstCUInStartLCUTmp = (iLcuIdx == uiStartLCU) ? uiFirstCUInStartLCU : 0;
      UInt uiLastCUInEndLCUTmp    = (iLcuIdx == uiEndLCU  ) ? uiLastCUInEndLCU+1 : uiNumSUInLCU;
      assert (uiFirstCUInStartLCUTmp<uiLastCUInEndLCUTmp);

      for (i=uiFirstCUInStartLCUTmp;i<uiLastCUInEndLCUTmp;i++)
      {
        uiLPelX = g_auiRasterToPelX[ g_auiZscanToRaster[i] ];
        uiTPelY = g_auiRasterToPelY[ g_auiZscanToRaster[i] ];

        for (l=0;l<uiMinCUHeight;l++)
        {
          for (k=0;k<uiMinCUWidth;k++)
          {
            pMap[(uiLPelX+k)+(uiTPelY+l)*iStride] = iSliceIdx;
#if SAO_CHROMA
            pMapCb[((uiLPelX+k)>>1)+((uiTPelY+l)>>1)*iStrideC] = iSliceIdx;
            pMapCr[((uiLPelX+k)>>1)+((uiTPelY+l)>>1)*iStrideC] = iSliceIdx;
#endif
          }
        }
      } 
    }
    else
    {
#if TILES
      pMap = m_pcPicYuvMap->getLumaAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
#if SAO_CHROMA
      pMapCb = m_pcPicYuvMap->getCbAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
      pMapCr = m_pcPicYuvMap->getCrAddr(m_pcPic->getPicSym()->getCUOrderMap(iLcuIdx));
#endif
#else
      pMap = m_pcPicYuvMap->getLumaAddr(iLcuIdx);
#if SAO_CHROMA
      pMapCb = m_pcPicYuvMap->getCbAddr(iLcuIdx);
      pMapCr = m_pcPicYuvMap->getCrAddr(iLcuIdx);
#endif
#endif
      for (l=0;l<uiMaxCUHeight;l++)
      {
        for (k=0;k<uiMaxCUWidth;k++)
        {
          pMap[k+l*iStride] = iSliceIdx;
#if SAO_CHROMA
          pMapCb[(k>>1)+(l>>1)*iStrideC] = iSliceIdx;
          pMapCr[(k>>1)+(l>>1)*iStrideC] = iSliceIdx;
#endif
        }
      }
    }
  }
}

/** sample adaptive offset process for one LCU
 * \param   iAddr, iSaoType, iYCbCr
 */
Void TComSampleAdaptiveOffset::processSaoCu(Int iAddr, Int iSaoType, Int iYCbCr)
{
  if (getUseNIF())
  {
    processSaoCuMap( iAddr, iSaoType, iYCbCr);
  }
  else
  {
    processSaoCuOrg( iAddr, iSaoType, iYCbCr);
  }

}
/** sample adaptive offset process for one LCU non-crossing LCU boundary
 * \param   iAddr, iSaoType, iYCbCr
 */
Void TComSampleAdaptiveOffset::processSaoCuMap(Int iAddr, Int iSaoType, Int iYCbCr)
#if SAO_CROSS_LCU_BOUNDARIES
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  Pel* pRec;
  Pel* pMap;
  Int  iStride;
  Int  iLcuWidth  = m_uiMaxCUWidth;
  Int  iLcuHeight = m_uiMaxCUHeight;
  UInt uiLPelX    = pTmpCu->getCUPelX();
  UInt uiTPelY    = pTmpCu->getCUPelY();
  UInt uiRPelX;
  UInt uiBPelY;
  Int  iSignLeft;
  Int  iSignRight;
  Int  iSignDown;
  Int  iSignDown1;
  Int  iSignDown2;
  UInt uiEdgeType;
  Int iPicWidthTmp;
  Int iPicHeightTmp;
  Int iStartX;
  Int iStartY;
  Int iEndX;
  Int iEndY;
  Int iIsChroma = (iYCbCr!=0)? 1:0;
  Int iShift;
  Int iCuHeightTmp;
  Pel *pTmpLSwap;
  Pel *pTmpL;
  Pel *pTmpU;

  iPicWidthTmp  = m_iPicWidth  >> iIsChroma;
  iPicHeightTmp = m_iPicHeight >> iIsChroma;
  iLcuWidth     = iLcuWidth    >> iIsChroma;
  iLcuHeight    = iLcuHeight   >> iIsChroma;
  uiLPelX       = uiLPelX      >> iIsChroma;
  uiTPelY       = uiTPelY      >> iIsChroma;
  uiRPelX       = uiLPelX + iLcuWidth  ;
  uiBPelY       = uiTPelY + iLcuHeight ;
  uiRPelX       = uiRPelX > iPicWidthTmp  ? iPicWidthTmp  : uiRPelX;
  uiBPelY       = uiBPelY > iPicHeightTmp ? iPicHeightTmp : uiBPelY;
  iLcuWidth     = uiRPelX - uiLPelX;
  iLcuHeight    = uiBPelY - uiTPelY;

  if (iYCbCr == 0)
  {
    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    pMap       = m_pcPicYuvMap->getLumaAddr(iAddr);
    iStride    = m_pcPic->getStride();
  } 
  else if (iYCbCr == 1)
  {
    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    pMap       = m_pcPicYuvMap->getCbAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else 
  {
    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    pMap       = m_pcPicYuvMap->getCrAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }

  {
    iCuHeightTmp = (m_uiMaxCUHeight >> iIsChroma);
    iShift = (m_uiMaxCUWidth>> iIsChroma)-1;
    for (Int i=0;i<iCuHeightTmp+1;i++)
    {
      m_pTmpL2[i] = pRec[iShift];
      pRec += iStride;
    }
    pRec -= (iStride*(iCuHeightTmp+1));

    pTmpL = m_pTmpL1; 
    pTmpU = &(m_pTmpU1[uiLPelX]); 
  }

  switch (iSaoType)
  {
  case SAO_EO_0: // dir: -
    {
      iStartX = (uiLPelX == 0) ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;
      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft = xSign(pRec[iStartX] - pTmpL[y]);
        for (x=iStartX; x< iEndX; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;
          if (pMap[x-1] == pMap[x+1])
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        pRec += iStride;
        pMap += iStride;
      }
      break;
    }
  case SAO_EO_1: // dir: |
    {
      iStartY = (uiTPelY == 0) ? 1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;
      if (uiTPelY == 0)
      {
        pRec += iStride;
        pMap += iStride;
      }
      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  = xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType = iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;
          if (pMap[x-iStride] == pMap[x+iStride] && pMap[x] == pMap[x+iStride])
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        pRec += iStride;
        pMap += iStride;
      }
      break;
    }
  case SAO_EO_2: // dir: 135
    {
      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (uiTPelY == 0)
      {
        pRec += iStride;
        pMap += iStride;
      }

      for (x=iStartX; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x-1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        iSignDown2 = xSign(pRec[iStride+iStartX] - pTmpL[y]);
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          if ((pMap[x-iStride-1] == pMap[x+iStride+1]) && (pMap[x] == pMap[x-iStride-1]))
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        m_iUpBufft[iStartX] = iSignDown2;

        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pMap += iStride;
      }
      break;
    } 
  case SAO_EO_3: // dir: 45
    {
      iStartX = (uiLPelX == 0) ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ? 1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pRec += iStride;
        pMap += iStride;
      }

      for (x=iStartX-1; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x+1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        x=iStartX;
        iSignDown1      =  xSign(pRec[x] - pTmpL[y+1]) ;
        uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
        m_iUpBuff1[x-1] = -iSignDown1; 
        if ((pMap[x-iStride+1] == pMap[x+iStride-1]) && (pMap[x+iStride-1]==pMap[x]))
        {
          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        for (x=iStartX+1; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1] = -iSignDown1; 
          if ((pMap[x-iStride+1] == pMap[x+iStride-1]) && (pMap[x+iStride-1]==pMap[x]))
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        m_iUpBuff1[iEndX-1] = xSign(pRec[iEndX-1 + iStride] - pRec[iEndX]);

        pRec += iStride;
        pMap += iStride;
      } 
      break;
    }   
  case SAO_BO_0:
  case SAO_BO_1:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          pRec[x] = m_iOffsetBo[pRec[x]];
        }
        pRec += iStride;
      }
      break;
    }
  default: break;
  }
  {
    pTmpLSwap = m_pTmpL1;
    m_pTmpL1  = m_pTmpL2;
    m_pTmpL2  = pTmpLSwap;
  }
}
#else
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  Pel* pRec;
  Pel* pMap;    

  Int  iStride;
  Int  iLcuWidth  = m_uiMaxCUWidth;
  Int  iLcuHeight = m_uiMaxCUHeight;
  UInt uiLPelX    = pTmpCu->getCUPelX();
  UInt uiTPelY    = pTmpCu->getCUPelY();
  UInt uiRPelX;
  UInt uiBPelY;
  Int  iSignLeft;
  Int  iSignRight;
  Int  iSignDown;
  Int  iSignDown1;
  Int  iSignDown2;
  UInt uiEdgeType;
  Int iPicWidthTmp;
  Int iPicHeightTmp;

  Int iIsChroma = (iYCbCr!=0)? 1:0;

  iPicWidthTmp  = m_iPicWidth  >> iIsChroma;
  iPicHeightTmp = m_iPicHeight >> iIsChroma;
  iLcuWidth     = iLcuWidth    >> iIsChroma;
  iLcuHeight    = iLcuHeight   >> iIsChroma;
  uiLPelX       = uiLPelX      >> iIsChroma;
  uiTPelY       = uiTPelY      >> iIsChroma;
  uiRPelX       = uiLPelX + iLcuWidth  ;
  uiBPelY       = uiTPelY + iLcuHeight ;
  uiRPelX       = uiRPelX > iPicWidthTmp  ? iPicWidthTmp  : uiRPelX;
  uiBPelY       = uiBPelY > iPicHeightTmp ? iPicHeightTmp : uiBPelY;
  iLcuWidth     = uiRPelX - uiLPelX;
  iLcuHeight    = uiBPelY - uiTPelY;

  switch (iYCbCr)
  {
  case 0:
    {
      pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
      pMap       = m_pcPicYuvMap->getLumaAddr(iAddr);
      iStride    = m_pcPic->getStride();
      break;
    }
  case 1:
    {
      pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
      pMap       = m_pcPicYuvMap->getCbAddr(iAddr);
      iStride    = m_pcPic->getCStride();
      break;
    }
  case 2:
    {
      pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
      pMap       = m_pcPicYuvMap->getCrAddr(iAddr);
      iStride    = m_pcPic->getCStride();
      break;
    }
  default: break;
  }



  switch (iSaoType)
  {
  case SAO_EO_0:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft = xSign(pRec[1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;

          if (pMap[x-1] == pMap[x+1])
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        pRec += iStride;
        pMap += iStride;
      }
      break;
    }
  case SAO_EO_1:
    {
      pRec += iStride;
      pMap += iStride;
      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  = xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType = iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;

          if (pMap[x-iStride] != 0 && pMap[x+iStride] != 0)
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        pRec += iStride;
        pMap += iStride;
      }
      break;
    }
  case SAO_EO_2:
    {
      pRec += iStride;
      pMap += iStride;
      for (x=1; x<iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        iSignDown2 = xSign(pRec[iStride+1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          if (pMap[x-iStride-1] != 0 && pMap[x+iStride+1] != 0)
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        m_iUpBufft[1] = iSignDown2;

        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pMap += iStride;
      }
      break;
    } 
  case SAO_EO_3:
    {
      pRec += iStride;
      pMap += iStride;
      for (x=0; x<iLcuWidth-1; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1] = -iSignDown1; 
          if (pMap[x-iStride+1] != 0 && pMap[x+iStride-1] != 0)
          {
            pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
          }
        }
        m_iUpBuff1[iLcuWidth-2] = xSign(pRec[iLcuWidth-2 + iStride] - pRec[iLcuWidth-1]);

        pRec += iStride;
        pMap += iStride;
      } 
      break;
    }   
  case SAO_BO_0:
  case SAO_BO_1:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          pRec[x] = m_iOffsetBo[pRec[x]];
        }
        pRec += iStride;
      }
      break;
    }
  default: break;
  }
}
#endif



#endif

/** sample adaptive offset process for one LCU crossing LCU boundary
 * \param   iAddr, iSaoType, iYCbCr
 */
#if SAO_FGS_NIF
Void TComSampleAdaptiveOffset::processSaoCuOrg(Int iAddr, Int iSaoType, Int iYCbCr)
#else
Void TComSampleAdaptiveOffset::processSaoCu(Int iAddr, Int iSaoType, Int iYCbCr)
#endif
#if SAO_CROSS_LCU_BOUNDARIES
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  Pel* pRec;
  Int  iStride;
  Int  iLcuWidth  = m_uiMaxCUWidth;
  Int  iLcuHeight = m_uiMaxCUHeight;
  UInt uiLPelX    = pTmpCu->getCUPelX();
  UInt uiTPelY    = pTmpCu->getCUPelY();
  UInt uiRPelX;
  UInt uiBPelY;
  Int  iSignLeft;
  Int  iSignRight;
  Int  iSignDown;
  Int  iSignDown1;
  Int  iSignDown2;
  UInt uiEdgeType;
  Int iPicWidthTmp;
  Int iPicHeightTmp;
  Int iStartX;
  Int iStartY;
  Int iEndX;
  Int iEndY;
  Int iIsChroma = (iYCbCr!=0)? 1:0;
  Int iShift;
  Int iCuHeightTmp;
  Pel *pTmpLSwap;
  Pel *pTmpL;
  Pel *pTmpU;

  iPicWidthTmp  = m_iPicWidth  >> iIsChroma;
  iPicHeightTmp = m_iPicHeight >> iIsChroma;
  iLcuWidth     = iLcuWidth    >> iIsChroma;
  iLcuHeight    = iLcuHeight   >> iIsChroma;
  uiLPelX       = uiLPelX      >> iIsChroma;
  uiTPelY       = uiTPelY      >> iIsChroma;
  uiRPelX       = uiLPelX + iLcuWidth  ;
  uiBPelY       = uiTPelY + iLcuHeight ;
  uiRPelX       = uiRPelX > iPicWidthTmp  ? iPicWidthTmp  : uiRPelX;
  uiBPelY       = uiBPelY > iPicHeightTmp ? iPicHeightTmp : uiBPelY;
  iLcuWidth     = uiRPelX - uiLPelX;
  iLcuHeight    = uiBPelY - uiTPelY;

  if (iYCbCr == 0)
  {
    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    iStride    = m_pcPic->getStride();
  } 
  else if (iYCbCr == 1)
  {
    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else 
  {
    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }

//   if (iSaoType!=SAO_BO_0 || iSaoType!=SAO_BO_1)
  {
    iCuHeightTmp = (m_uiMaxCUHeight >> iIsChroma);
    iShift = (m_uiMaxCUWidth>> iIsChroma)-1;
    for (Int i=0;i<iCuHeightTmp+1;i++)
    {
      m_pTmpL2[i] = pRec[iShift];
      pRec += iStride;
    }
    pRec -= (iStride*(iCuHeightTmp+1));

    pTmpL = m_pTmpL1; 
    pTmpU = &(m_pTmpU1[uiLPelX]); 
  }

  switch (iSaoType)
  {
  case SAO_EO_0: // dir: -
    {
      iStartX = (uiLPelX == 0) ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;
      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft = xSign(pRec[iStartX] - pTmpL[y]);
        for (x=iStartX; x< iEndX; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;

          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        pRec += iStride;
      }
      break;
    }
  case SAO_EO_1: // dir: |
    {
      iStartY = (uiTPelY == 0) ? 1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;
      if (uiTPelY == 0)
      {
        pRec += iStride;
      }
      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  = xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType = iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;

          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        pRec += iStride;
      }
      break;
    }
  case SAO_EO_2: // dir: 135
    {
      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (uiTPelY == 0)
      {
        pRec += iStride;
      }

      for (x=iStartX; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x-1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        iSignDown2 = xSign(pRec[iStride+iStartX] - pTmpL[y]);
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        m_iUpBufft[iStartX] = iSignDown2;

        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
      }
      break;
    } 
  case SAO_EO_3: // dir: 45
    {
      iStartX = (uiLPelX == 0) ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ? 1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pRec += iStride;
      }

      for (x=iStartX-1; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pTmpU[x+1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        x=iStartX;
        iSignDown1      =  xSign(pRec[x] - pTmpL[y+1]) ;
        uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
        m_iUpBuff1[x-1] = -iSignDown1; 
        pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        for (x=iStartX+1; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1] = -iSignDown1; 
          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        m_iUpBuff1[iEndX-1] = xSign(pRec[iEndX-1 + iStride] - pRec[iEndX]);

        pRec += iStride;
      } 
      break;
    }   
  case SAO_BO_0:
  case SAO_BO_1:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          pRec[x] = m_iOffsetBo[pRec[x]];
        }
        pRec += iStride;
      }
      break;
    }
  default: break;
  }
//   if (iSaoType!=SAO_BO_0 || iSaoType!=SAO_BO_1)
  {
    pTmpLSwap = m_pTmpL1;
    m_pTmpL1  = m_pTmpL2;
    m_pTmpL2  = pTmpLSwap;
  }
}
#else
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  Pel* pRec;
  Int  iStride;
  Int  iLcuWidth  = m_uiMaxCUWidth;
  Int  iLcuHeight = m_uiMaxCUHeight;
  UInt uiLPelX    = pTmpCu->getCUPelX();
  UInt uiTPelY    = pTmpCu->getCUPelY();
  UInt uiRPelX;
  UInt uiBPelY;
  Int  iSignLeft;
  Int  iSignRight;
  Int  iSignDown;
  Int  iSignDown1;
  Int  iSignDown2;
  UInt uiEdgeType;
  Int iPicWidthTmp;
  Int iPicHeightTmp;

  Int iIsChroma = (iYCbCr!=0)? 1:0;

  iPicWidthTmp  = m_iPicWidth  >> iIsChroma;
  iPicHeightTmp = m_iPicHeight >> iIsChroma;
  iLcuWidth     = iLcuWidth    >> iIsChroma;
  iLcuHeight    = iLcuHeight   >> iIsChroma;
  uiLPelX       = uiLPelX      >> iIsChroma;
  uiTPelY       = uiTPelY      >> iIsChroma;
  uiRPelX       = uiLPelX + iLcuWidth  ;
  uiBPelY       = uiTPelY + iLcuHeight ;
  uiRPelX       = uiRPelX > iPicWidthTmp  ? iPicWidthTmp  : uiRPelX;
  uiBPelY       = uiBPelY > iPicHeightTmp ? iPicHeightTmp : uiBPelY;
  iLcuWidth     = uiRPelX - uiLPelX;
  iLcuHeight    = uiBPelY - uiTPelY;

  switch (iYCbCr)
  {
  case 0:
    {
      pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
      pTmp       = m_pcPicYuvTmp->getLumaAddr(iAddr);
      iStride    = m_pcPic->getStride();
      break;
    }
  case 1:
    {
      pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
      pTmp       = m_pcPicYuvTmp->getCbAddr(iAddr);
      iStride    = m_pcPic->getCStride();
      break;
    }
  case 2:
    {
      pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
      pTmp       = m_pcPicYuvTmp->getCrAddr(iAddr);
      iStride    = m_pcPic->getCStride();
      break;
    }
  default: break;
  }

  switch (iSaoType)
  {
  case SAO_EO_0:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft = xSign(pRec[1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;

          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        pRec += iStride;
      }
      break;
    }
  case SAO_EO_1:
    {
      pRec += iStride;
      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  = xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType = iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;

          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        pRec += iStride;
      }
      break;
    }
  case SAO_EO_2:
    {
      pRec += iStride;
      for (x=1; x<iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        iSignDown2 = xSign(pRec[iStride+1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        m_iUpBufft[1] = iSignDown2;

        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
      }
      break;
    } 
  case SAO_EO_3:
    {
      pRec += iStride;
      for (x=0; x<iLcuWidth-1; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1] = -iSignDown1; 
          pRec[x] = m_pClipTable[pRec[x] + m_iOffsetEo[uiEdgeType]];
        }
        m_iUpBuff1[iLcuWidth-2] = xSign(pRec[iLcuWidth-2 + iStride] - pRec[iLcuWidth-1]);

        pRec += iStride;
      } 
      break;
    }   
  case SAO_BO_0:
  case SAO_BO_1:
    {
      for (y=0; y<iLcuHeight; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          pRec[x] = m_iOffsetBo[pRec[x]];
        }
        pRec += iStride;
      }
      break;
    }
  default: break;
  }
}
#endif
/** sample adaptive offset process for one partition
 * \param   *psQTPart,  uiPartIdx,  iYCbCr
 */
Void TComSampleAdaptiveOffset::processSaoOnePart(SAOQTPart *psQTPart, UInt uiPartIdx, Int iYCbCr)
{
  int  i;
  UInt uiEdgeType, uiTypeIdx;
  Pel* ppLumaTable = NULL;
  SAOQTPart*  pOnePart= &(psQTPart[uiPartIdx]);

  static Int iOffset[LUMA_GROUP_NUM];
  Int LcuIdxX;
  Int LcuIdxY;
  Int iAddr;
  Int iFrameWidthInCU = m_pcPic->getFrameWidthInCU();

  if(pOnePart->bEnableFlag)
  {
    uiTypeIdx = pOnePart->iBestType;
    if (uiTypeIdx == SAO_BO_0 || uiTypeIdx == SAO_BO_1)
    {
      for (i=0;i<pOnePart->iLength;i++)
#if SAO_ACCURATE_OFFSET
        iOffset[i+1] = pOnePart->iOffset[i] << m_uiSaoBitIncrease;
#else
#if FULL_NBIT
        iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitDepth-8-m_uiSaoBitIncrease);
#else
        iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitIncrement-m_uiSaoBitIncrease);
#endif
#endif

      if (uiTypeIdx == SAO_BO_0 )
      {
        ppLumaTable = m_ppLumaTableBo0;
      }
      if (uiTypeIdx == SAO_BO_1 )
      {
        ppLumaTable = m_ppLumaTableBo1;
      }

#if FULL_NBIT
      for (i=0;i<(1<<(g_uiBitDepth));i++)
#else
      for (i=0;i<(1<<(g_uiBitIncrement+8));i++)
#endif
      {
        m_iOffsetBo[i] = m_pClipTable[i + iOffset[ppLumaTable[i]]];
      }

    }
    if (uiTypeIdx == SAO_EO_0 || uiTypeIdx == SAO_EO_1 || uiTypeIdx == SAO_EO_2 || uiTypeIdx == SAO_EO_3)
    {
      for (i=0;i<pOnePart->iLength;i++)
      {
#if SAO_ACCURATE_OFFSET
        iOffset[i+1] = pOnePart->iOffset[i] << m_uiSaoBitIncrease;
#else
#if FULL_NBIT
        iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitDepth-8-m_uiSaoBitIncrease);
#else
        iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitIncrement-m_uiSaoBitIncrease);
#endif
#endif

      }
      for (uiEdgeType=0;uiEdgeType<6;uiEdgeType++)
      {
        m_iOffsetEo[uiEdgeType]= iOffset[m_auiEoTable[uiEdgeType]];
      }
    }
    for (LcuIdxY = pOnePart->StartCUY; LcuIdxY<= pOnePart->EndCUY; LcuIdxY++)
    {
      for (LcuIdxX = pOnePart->StartCUX; LcuIdxX<= pOnePart->EndCUX; LcuIdxX++)
      {
        iAddr = LcuIdxY * iFrameWidthInCU + LcuIdxX;
        processSaoCu(iAddr, uiTypeIdx, iYCbCr);
      }
    }
  }
}

/** Process quadtree sample adaptive offset
 * \param  psQTPart, uiPartIdx, iYCbCr
 */
Void TComSampleAdaptiveOffset::processSaoQuadTree(SAOQTPart *psQTPart, UInt uiPartIdx, Int iYCbCr)
{
  SAOQTPart*  pSaoPart= &(psQTPart[uiPartIdx]);

#if SAO_CROSS_LCU_BOUNDARIES
  if (uiPartIdx == 0)
  {
    initTmpSaoQuadTree(psQTPart, iYCbCr);
    xSaoAllPart(psQTPart, iYCbCr);
    return;
  }
#endif

  if (!pSaoPart->bSplit)
  {
    if (pSaoPart->bEnableFlag)
    {
      processSaoOnePart(psQTPart, uiPartIdx, iYCbCr);
    }
    return;
  }

  if (pSaoPart->PartLevel < m_uiMaxSplitLevel)
  {
    processSaoQuadTree(psQTPart, pSaoPart->DownPartsIdx[0], iYCbCr);
    processSaoQuadTree(psQTPart, pSaoPart->DownPartsIdx[1], iYCbCr);
    processSaoQuadTree(psQTPart, pSaoPart->DownPartsIdx[2], iYCbCr);
    processSaoQuadTree(psQTPart, pSaoPart->DownPartsIdx[3], iYCbCr);
  }
}

#if SAO_CROSS_LCU_BOUNDARIES
/** run SAO processing in LCU order
 * \param *psQTPart,  iYCbCr 
 */
Void TComSampleAdaptiveOffset::xSaoAllPart(SAOQTPart *psQTPart, Int iYCbCr)
{
  int  i;
  UInt uiEdgeType;
  Pel* ppLumaTable = NULL;
  Int  iTypeIdx;

  static Int iOffset[LUMA_GROUP_NUM];
  Int LcuIdxX;
  Int LcuIdxY;
  Int iAddr;
  Int iFrameWidthInCU = m_pcPic->getFrameWidthInCU();
  Int iFrameHeightInCU = m_pcPic->getFrameHeightInCU();
  Int iPartIdx;
  Pel *pRec;
  Int iPicWidthTmp;
  Int iStride;
  Pel *pTmpUSwap;
  Int iIsChroma = (iYCbCr == 0) ? 0:1;

  SAOQTPart*  pOnePart;

  for (LcuIdxY = 0; LcuIdxY< iFrameHeightInCU; LcuIdxY++)
  { 
    iAddr = LcuIdxY * iFrameWidthInCU;
    if (iYCbCr == 0)
    {
      pRec  = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
      iStride = m_pcPic->getStride();
      iPicWidthTmp = m_iPicWidth;
    }
    else if (iYCbCr == 1)
    {
      pRec  = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
      iStride = m_pcPic->getCStride();
      iPicWidthTmp = m_iPicWidth>>1;
    }
    else
    {
      pRec  = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
      iStride = m_pcPic->getCStride();
      iPicWidthTmp = m_iPicWidth>>1;
    }

//     pRec += iStride*(m_uiMaxCUHeight-1);
    for (i=0;i<(m_uiMaxCUHeight>>iIsChroma)+1;i++)
    {
      m_pTmpL1[i] = pRec[0];
      pRec+=iStride;
    }
    pRec-=(iStride<<1);

    memcpy(m_pTmpU2, pRec, sizeof(Pel)*iPicWidthTmp);

    for (LcuIdxX = 0; LcuIdxX< iFrameWidthInCU; LcuIdxX++)
    {
      iAddr = LcuIdxY * iFrameWidthInCU + LcuIdxX;
      iPartIdx = m_iLcuPartIdx[iAddr];
      if (iPartIdx>=0)
      {
        pOnePart = &(psQTPart[iPartIdx]);
        iTypeIdx = pOnePart->iBestType;
        if (iTypeIdx>=0)
        {
          if (iTypeIdx == SAO_BO_0 || iTypeIdx == SAO_BO_1)
          {
            for (i=0;i<pOnePart->iLength;i++)
#if SAO_ACCURATE_OFFSET
              iOffset[i+1] = pOnePart->iOffset[i] << m_uiSaoBitIncrease;
#else
#if FULL_NBIT
              iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitDepth-8-m_uiSaoBitIncrease);
#else
              iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitIncrement-m_uiSaoBitIncrease);
#endif
#endif

            if (iTypeIdx == SAO_BO_0 )
            {
              ppLumaTable = m_ppLumaTableBo0;
            }
            if (iTypeIdx == SAO_BO_1 )
            {
              ppLumaTable = m_ppLumaTableBo1;
            }

#if FULL_NBIT
            for (i=0;i<(1<<(g_uiBitDepth));i++)
#else
            for (i=0;i<(1<<(g_uiBitIncrement+8));i++)
#endif
            {
              m_iOffsetBo[i] = m_pClipTable[i + iOffset[ppLumaTable[i]]];
            }

          }
          if (iTypeIdx == SAO_EO_0 || iTypeIdx == SAO_EO_1 || iTypeIdx == SAO_EO_2 || iTypeIdx == SAO_EO_3)
          {
            for (i=0;i<pOnePart->iLength;i++)
            {
#if SAO_ACCURATE_OFFSET
              iOffset[i+1] = pOnePart->iOffset[i] << m_uiSaoBitIncrease;
#else
#if FULL_NBIT
              iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitDepth-8-m_uiSaoBitIncrease);
#else
              iOffset[i+1] = pOnePart->iOffset[i] << (g_uiBitIncrement-m_uiSaoBitIncrease);
#endif
#endif
            }
            for (uiEdgeType=0;uiEdgeType<6;uiEdgeType++)
            {
              m_iOffsetEo[uiEdgeType]= iOffset[m_auiEoTable[uiEdgeType]];
            }
          }
          processSaoCu(iAddr, iTypeIdx, iYCbCr);
        }
      }
      else
      {
        if (LcuIdxX != (iFrameWidthInCU-1))
        {
          if( m_iLcuPartIdx[iAddr+1] >=0) 
          {
            if (iYCbCr == 0)
            {
              pRec  = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
              iStride = m_pcPic->getStride();
            }
            else if (iYCbCr == 1)
            {
              pRec  = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
              iStride = m_pcPic->getCStride();
            }
            else
            {
              pRec  = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
              iStride = m_pcPic->getCStride();
            }
            Int iWidthShift = m_uiMaxCUWidth>>iIsChroma;
            for (i=0;i<(m_uiMaxCUHeight>>iIsChroma)+1;i++)
            {
              m_pTmpL1[i] = pRec[iWidthShift-1];
              pRec+=iStride;
            }
          }
        }
      }
    }
    pTmpUSwap = m_pTmpU1;
    m_pTmpU1  = m_pTmpU2;
    m_pTmpU2  = pTmpUSwap;
  }
}

/** initialize buffer for quadtree boundary
 * \param *psQTPart,  iYCbCr 
 */
Void TComSampleAdaptiveOffset::initTmpSaoQuadTree(SAOQTPart *psQTPart, Int iYCbCr)
{
  Pel *pRec;
  Int iStride;
  Int iPicWidthTmp;
  Int iPicHeightTmp;
  

  memset(m_iLcuPartIdx,-1, sizeof(Int)*m_iNumCuInWidth*m_iNumCuInHeight);
  convertSaoQt2Lcu(psQTPart, 0);

  if (iYCbCr == 0)
  {
    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr();
    iStride    = m_pcPic->getStride();
    iPicWidthTmp = m_iPicWidth;
    iPicHeightTmp = m_iPicHeight;
  } 
  else if (iYCbCr == 1)
  {
    pRec       = m_pcPic->getPicYuvRec()->getCbAddr();
    iStride    = m_pcPic->getCStride();
    iPicWidthTmp =  m_iPicWidth>>1;
    iPicHeightTmp = m_iPicHeight>>1;
  }
  else 
  {
    pRec       = m_pcPic->getPicYuvRec()->getCrAddr();
    iStride    = m_pcPic->getCStride();
    iPicWidthTmp =  m_iPicWidth>>1;
    iPicHeightTmp = m_iPicHeight>>1;
  }

  memcpy(m_pTmpU1, pRec, sizeof(Pel)*iPicWidthTmp);
}

/** recursive covert quadtree partition index to each LCU 
 * \param psQTPart, uiPartIdx  
 */
Void TComSampleAdaptiveOffset::convertSaoQt2Lcu(SAOQTPart *psQTPart,UInt uiPartIdx)
{

  SAOQTPart*  pSaoPart= &(psQTPart[uiPartIdx]);

  if (!pSaoPart->bSplit)
  {
    xSaoQt2Lcu(psQTPart, uiPartIdx);
    return;
  }

  if (pSaoPart->PartLevel < m_uiMaxSplitLevel)
  {
    convertSaoQt2Lcu(psQTPart, pSaoPart->DownPartsIdx[0]);
    convertSaoQt2Lcu(psQTPart, pSaoPart->DownPartsIdx[1]);
    convertSaoQt2Lcu(psQTPart, pSaoPart->DownPartsIdx[2]);
    convertSaoQt2Lcu(psQTPart, pSaoPart->DownPartsIdx[3]);
  }
}

/** assign quadtree partition index to each LCU 
 * \param psQTPart, uiPartIdx  
 */

Void TComSampleAdaptiveOffset::xSaoQt2Lcu(SAOQTPart *psQTPart,UInt uiPartIdx)
{
  Int LcuIdxX;
  Int LcuIdxY;
  Int iAddr;
  Int iFrameWidthInCU = m_iNumCuInWidth;

  for (LcuIdxY = psQTPart[uiPartIdx].StartCUY; LcuIdxY<= psQTPart[uiPartIdx].EndCUY; LcuIdxY++)
  {
    for (LcuIdxX = psQTPart[uiPartIdx].StartCUX; LcuIdxX<= psQTPart[uiPartIdx].EndCUX; LcuIdxX++)
    {
      iAddr = LcuIdxY * iFrameWidthInCU + LcuIdxX;
      if (psQTPart[uiPartIdx].bEnableFlag)
      {
        m_iLcuPartIdx[iAddr] = (Int)uiPartIdx; 
      }
      else
      {
        m_iLcuPartIdx[iAddr] = -1;
      } 
    }
  }
}

#endif

/** Sample adaptive offset process
 * \param pcPic, pcSaoParam  
 */
Void TComSampleAdaptiveOffset::SAOProcess(TComPic* pcPic, SAOParam* pcSaoParam)
{
  if (pcSaoParam->bSaoFlag[0])
  {
#if SAO_ACCURATE_OFFSET
#if FULL_NBIT
    m_uiSaoBitIncrease = g_uiBitDepth + (g_uiBitDepth-8) - min((Int)(g_uiBitDepth + (g_uiBitDepth-8)), 10);
#else
    m_uiSaoBitIncrease = g_uiBitDepth + g_uiBitIncrement - min((Int)(g_uiBitDepth + g_uiBitIncrement), 10);
#endif
#else
#if FULL_NBIT
    if (g_uiBitDepth-8>1)
#else
    if (g_uiBitIncrement>1)
#endif
    {
      m_uiSaoBitIncrease = 1;
    }
    else
    {
      m_uiSaoBitIncrease = 0;
    }
#endif
    m_pcPic = pcPic;

    Int iY  = 0;
    processSaoQuadTree( pcSaoParam->psSaoPart[iY], 0 , iY);

#if SAO_CHROMA
    Int iCb = 1;
    Int iCr = 2;
    if (pcSaoParam->bSaoFlag[iCb])
    {
      processSaoQuadTree( pcSaoParam->psSaoPart[iCb], 0 , iCb);
    }
    if (pcSaoParam->bSaoFlag[iCr])
    {
      processSaoQuadTree( pcSaoParam->psSaoPart[iCr], 0 , iCr);
    }
#endif
    m_pcPic = NULL;
  }
}

Pel* TComSampleAdaptiveOffset::getPicYuvAddr(TComPicYuv* pcPicYuv, Int iYCbCr, Int iAddr)
{
  switch (iYCbCr)
  {
  case 0:
    return pcPicYuv->getLumaAddr(iAddr);
    break;
  case 1:
    return pcPicYuv->getCbAddr(iAddr);
    break;
  case 2:
    return pcPicYuv->getCrAddr(iAddr);
    break;
  default:
    return NULL;
    break;
  }
}
//! \}
