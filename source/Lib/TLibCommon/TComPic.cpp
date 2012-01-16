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

/** \file     TComPic.cpp
    \brief    picture class
*/

#include "TComPic.h"
#include "SEI.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComPic::TComPic()
{
  m_uiTLayer          = 0;

  m_apcPicSym         = NULL;
  m_apcPicYuv[0]      = NULL;
  m_apcPicYuv[1]      = NULL;
  m_pcPicYuvPred      = NULL;
  m_pcPicYuvResi      = NULL;
#if G1002_RPS
  m_bIsLongTerm       = false;
#endif
  m_bReconstructed    = false;
#if NO_TMVP_MARKING
  m_usedForTMVP      = true;
#endif
#if G1002_RPS && G1002_IDR_POC_ZERO_BUGFIX
  m_bNeededForOutput  = false;
#endif
#if NONCROSS_TILE_IN_LOOP_FILTERING
  m_piSliceSUMap      = NULL;
#endif

}

TComPic::~TComPic()
{
}

Void TComPic::create( Int iWidth, Int iHeight, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth, Bool bIsVirtual )
{
  m_apcPicSym     = new TComPicSym;  m_apcPicSym   ->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  if (!bIsVirtual)
  {
    m_apcPicYuv[0]  = new TComPicYuv;  m_apcPicYuv[0]->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  }
  m_apcPicYuv[1]  = new TComPicYuv;  m_apcPicYuv[1]->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  
  /* there are no SEI messages associated with this picture initially */
  m_SEIs = NULL;
#if G1002_RPS
  m_bUsedByCurr = false;
#endif
  return;
}

Void TComPic::destroy()
{
  if (m_apcPicSym)
  {
    m_apcPicSym->destroy();
    delete m_apcPicSym;
    m_apcPicSym = NULL;
  }
  
  if (m_apcPicYuv[0])
  {
    m_apcPicYuv[0]->destroy();
    delete m_apcPicYuv[0];
    m_apcPicYuv[0]  = NULL;
  }
  
  if (m_apcPicYuv[1])
  {
    m_apcPicYuv[1]->destroy();
    delete m_apcPicYuv[1];
    m_apcPicYuv[1]  = NULL;
  }
  
  delete m_SEIs;
}

#if AMVP_BUFFERCOMPRESS
Void TComPic::compressMotion()
{
  TComPicSym* pPicSym = getPicSym(); 
  for ( UInt uiCUAddr = 0; uiCUAddr < pPicSym->getFrameHeightInCU()*pPicSym->getFrameWidthInCU(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pPicSym->getCU(uiCUAddr);
    pcCU->compressMV(); 
  } 
}
#endif

#if NONCROSS_TILE_IN_LOOP_FILTERING
Void TComPic::createNonDBFilterInfo(UInt* puiSliceStartAddress, Int iNumSlices, Int iSliceGranularityDepth
                                    ,Bool bNDBFilterCrossSliceBoundary
                                    ,Int iNumTiles
                                    ,Bool bNDBFilterCrossTileBoundary)
{
  UInt uiMaxNumSUInLCU = getNumPartInCU();
  UInt uiNumLCUInPic   = getNumCUsInFrame();
  UInt uiPicWidth      = getCU(0)->getSlice()->getSPS()->getWidth();
  UInt uiPicHeight     = getCU(0)->getSlice()->getSPS()->getHeight();
  Int  iNumLCUsInPicWidth = getFrameWidthInCU();
  Int  iNumLCUsInPicHeight= getFrameHeightInCU();
  UInt uiMaxNumSUInLCUWidth = getNumPartInWidth();
  UInt uiMAxNumSUInLCUHeight= getNumPartInHeight();

  m_bIndependentSliceBoundaryForNDBFilter = (bNDBFilterCrossSliceBoundary)?(false):((iNumSlices > 1)?(true):(false)) ;
  m_iSliceGranularityForNDBFilter = iSliceGranularityDepth;
  m_bIndependentTileBoundaryForNDBFilter  = (bNDBFilterCrossTileBoundary)?(false) :((iNumTiles > 1)?(true):(false));

  m_pbValidSlice = new Bool[iNumSlices];
  for(Int s=0; s< iNumSlices; s++)
  {
    m_pbValidSlice[s] = true;
  }

  if( puiSliceStartAddress == NULL || (iNumSlices == 1 && iNumTiles == 1) )
  {
    return;
  }
  m_piSliceSUMap = new Int[uiMaxNumSUInLCU * uiNumLCUInPic];

  //initialization
  for(UInt i=0; i< (uiMaxNumSUInLCU * uiNumLCUInPic); i++ )
  {
    m_piSliceSUMap[i] = -1;
  }
  for( UInt uiCUAddr = 0; uiCUAddr < uiNumLCUInPic ; uiCUAddr++ )
  {
    TComDataCU* pcCU = getCU( uiCUAddr );
    pcCU->setSliceSUMap(m_piSliceSUMap + (uiCUAddr* uiMaxNumSUInLCU)); 
    pcCU->getNDBFilterBlocks()->clear();
  }
  m_vSliceCUDataLink.clear();

  m_vSliceCUDataLink.resize(iNumSlices);

  UInt uiStartAddr, uiEndAddr, uiFirstCUInStartLCU, uiStartLCU, uiEndLCU, uiLastCUInEndLCU, uiAddr;
  UInt uiLPelX, uiTPelY, uiLCUX, uiLCUY;
  UInt uiCurrSU;
  UInt uiStartSU, uiEndSU;

  for(Int s=0; s< iNumSlices; s++)
  {
    //1st step: decide the real start address
#if FINE_GRANULARITY_SLICES
    uiStartAddr = puiSliceStartAddress[s];
    uiEndAddr   = puiSliceStartAddress[s+1] -1;
#else
    uiStartAddr = (puiSliceStartAddress[s]*uiMaxNumSUInLCU);
    uiEndAddr   = (puiSliceStartAddress[s+1]*uiMaxNumSUInLCU) -1;
#endif

    uiStartLCU            = uiStartAddr / uiMaxNumSUInLCU;
    uiFirstCUInStartLCU   = uiStartAddr % uiMaxNumSUInLCU;

    uiEndLCU              = uiEndAddr   / uiMaxNumSUInLCU;
    uiLastCUInEndLCU      = uiEndAddr   % uiMaxNumSUInLCU;   

#if TILES
    uiAddr = m_apcPicSym->getCUOrderMap(uiStartLCU);
#else
    uiAddr = uiStartLCU;
#endif

    uiLCUX      = getCU(uiAddr)->getCUPelX();
    uiLCUY      = getCU(uiAddr)->getCUPelY();
    uiLPelX     = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiFirstCUInStartLCU] ];
    uiTPelY     = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiFirstCUInStartLCU] ];
    uiCurrSU    = uiFirstCUInStartLCU;

    Bool bMoveToNextLCU = false;
    Bool bSliceInOneLCU = (uiStartLCU == uiEndLCU);

    while(!( uiLPelX < uiPicWidth ) || !( uiTPelY < uiPicHeight ))
    {
      uiCurrSU ++;

      if(bSliceInOneLCU)
      {
        if(uiCurrSU > uiLastCUInEndLCU)
        {
          m_pbValidSlice[s] = false;
          break;
        }
      }

      if(uiCurrSU >= uiMaxNumSUInLCU )
      {
        bMoveToNextLCU = true;
        break;
      }

      uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
      uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

    }


    if(!m_pbValidSlice[s])
    {
      continue;
    }

    if(uiCurrSU != uiFirstCUInStartLCU)
    {
      if(!bMoveToNextLCU)
      {
        uiFirstCUInStartLCU = uiCurrSU;
      }
      else
      {
        uiStartLCU++;
        uiFirstCUInStartLCU = 0;
        assert( uiStartLCU < getNumCUsInFrame());
      }
      assert(uiStartLCU*uiMaxNumSUInLCU + uiFirstCUInStartLCU < uiEndAddr);
    }


    //2nd step: assign NonDBFilterInfo to each processing block
    for(UInt i= uiStartLCU; i <= uiEndLCU; i++)
    {
      uiStartSU = (i == uiStartLCU)?(uiFirstCUInStartLCU):(0);
      uiEndSU   = (i == uiEndLCU  )?(uiLastCUInEndLCU   ):(uiMaxNumSUInLCU -1);

#if TILES
      uiAddr = m_apcPicSym->getCUOrderMap(i);
      Int iTileID= m_apcPicSym->getTileIdxMap(uiAddr);
#else
      uiAddr = i;
#endif

      TComDataCU* pcCU = getCU(uiAddr);
      m_vSliceCUDataLink[s].push_back(pcCU);

#if TILES
      createNonDBFilterInfoLCU(iTileID, s, pcCU, uiStartSU, uiEndSU, m_iSliceGranularityForNDBFilter, uiPicWidth, uiPicHeight);
#else
      createNonDBFilterInfoLCU(s, pcCU, uiStartSU, uiEndSU, m_iSliceGranularityForNDBFilter, uiPicWidth, uiPicHeight);
#endif
    }
  }

  //step 3: border availability
  for(Int s=0; s< iNumSlices; s++)
  {
    if(!m_pbValidSlice[s])
    {
      continue;
    }

    for(Int i=0; i< m_vSliceCUDataLink[s].size(); i++)
    {
      TComDataCU* pcCU = m_vSliceCUDataLink[s][i];
      uiAddr = pcCU->getAddr();

      Int iTileID= m_apcPicSym->getTileIdxMap(uiAddr);
      Bool bTopTileBoundary = false, bDownTileBoundary= false, bLeftTileBoundary= false, bRightTileBoundary= false;

      if(m_bIndependentTileBoundaryForNDBFilter)
      {
        //left
        if( uiAddr % iNumLCUsInPicWidth != 0)
        {
          bLeftTileBoundary = ( m_apcPicSym->getTileIdxMap(uiAddr -1) != iTileID )?true:false;
        }
        //right
        if( (uiAddr % iNumLCUsInPicWidth) != (iNumLCUsInPicWidth -1) )
        {
          bRightTileBoundary = ( m_apcPicSym->getTileIdxMap(uiAddr +1) != iTileID)?true:false;
        }
        //top
        if( uiAddr >= iNumLCUsInPicWidth)
        {
          bTopTileBoundary = (m_apcPicSym->getTileIdxMap(uiAddr - iNumLCUsInPicWidth) !=  iTileID )?true:false;
        }
        //down
        if( uiAddr + iNumLCUsInPicWidth < uiNumLCUInPic )
        {
          bDownTileBoundary = (m_apcPicSym->getTileIdxMap(uiAddr + iNumLCUsInPicWidth) != iTileID)?true:false;
        }

      }

      pcCU->setNDBFilterBlockBorderAvailability(iNumLCUsInPicWidth, iNumLCUsInPicHeight, uiMaxNumSUInLCUWidth, uiMAxNumSUInLCUHeight,uiPicWidth, uiPicHeight
        ,m_bIndependentSliceBoundaryForNDBFilter
        ,bTopTileBoundary, bDownTileBoundary, bLeftTileBoundary, bRightTileBoundary
        ,m_bIndependentTileBoundaryForNDBFilter);

    }

  }

  if( m_bIndependentSliceBoundaryForNDBFilter || m_bIndependentTileBoundaryForNDBFilter)
  {
    m_pcNDBFilterYuvTmp = new TComPicYuv();
    m_pcNDBFilterYuvTmp->create(uiPicWidth, uiPicHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  }

}

#if TILES
Void TComPic::createNonDBFilterInfoLCU(Int iTileID, Int iSliceID, TComDataCU* pcCU, UInt uiStartSU, UInt uiEndSU, Int iSliceGranularyDepth, UInt uiPicWidth, UInt uiPicHeight)
#else
Void TComPic::createNonDBFilterInfoLCU(Int iSliceID, TComDataCU* pcCU, UInt uiStartSU, UInt uiEndSU, Int iSliceGranularyDepth, UInt uiPicWidth, UInt uiPicHeight)
#endif
{
  UInt uiLCUX          = pcCU->getCUPelX();
  UInt uiLCUY          = pcCU->getCUPelY();
  Int* piCUSliceMap    = pcCU->getSliceSUMap();
  UInt uiMaxNumSUInLCU = getNumPartInCU();
  UInt uiMaxNumSUInSGU = uiMaxNumSUInLCU >> (iSliceGranularyDepth << 1);
  UInt uiMaxNumSUInLCUWidth = getNumPartInWidth();
  UInt uiLPelX, uiTPelY;
  UInt uiCurrSU;
  UInt uiNumSGU;


  //get the number of valid NBFilterBLock
  uiCurrSU   = uiStartSU;
  uiNumSGU = 0;
  while(uiCurrSU <= uiEndSU)
  {
    uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
    uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

    while(!( uiLPelX < uiPicWidth ) || !( uiTPelY < uiPicHeight ))
    {
      uiCurrSU += uiMaxNumSUInSGU;
      if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > uiEndSU)
      {
        break;
      }
      uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
      uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];
    }

    if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > uiEndSU)
    {
      break;
    }

    NDBFBlockInfo NDBFBlock;

#if TILES
    NDBFBlock.tileID  = iTileID;
#endif
    NDBFBlock.sliceID = iSliceID;
    NDBFBlock.posY    = uiTPelY;
    NDBFBlock.posX    = uiLPelX;
    NDBFBlock.startSU = uiCurrSU;

    UInt uiLastValidSU  = uiCurrSU;
    UInt uiIdx, uiLPelX_su, uiTPelY_su;
    for(uiIdx = uiCurrSU; uiIdx < uiCurrSU + uiMaxNumSUInSGU; uiIdx++)
    {
      if(uiIdx > uiEndSU)
      {
        break;        
      }
      uiLPelX_su   = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY_su   = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      if( !(uiLPelX_su < uiPicWidth ) || !( uiTPelY_su < uiPicHeight ))
      {
        continue;
      }
      piCUSliceMap[uiIdx] = iSliceID;
      uiLastValidSU = uiIdx;
    }
    NDBFBlock.endSU = uiLastValidSU;

    UInt rTLSU = g_auiZscanToRaster[ NDBFBlock.startSU ];
    UInt rBRSU = g_auiZscanToRaster[ NDBFBlock.endSU   ];
    NDBFBlock.widthSU  = (rBRSU % uiMaxNumSUInLCUWidth) - (rTLSU % uiMaxNumSUInLCUWidth)+ 1;
    NDBFBlock.heightSU = (UInt)(rBRSU / uiMaxNumSUInLCUWidth) - (UInt)(rTLSU / uiMaxNumSUInLCUWidth)+ 1;
    NDBFBlock.width    = NDBFBlock.widthSU  * getMinCUWidth();
    NDBFBlock.height   = NDBFBlock.heightSU * getMinCUHeight();

    pcCU->getNDBFilterBlocks()->push_back(NDBFBlock);

    uiCurrSU += uiMaxNumSUInSGU;
  }

}

Void TComPic::destroyNonDBFilterInfo()
{
  if(m_pbValidSlice != NULL)
  {
    delete[] m_pbValidSlice;
    m_pbValidSlice = NULL;
  }

  if(m_piSliceSUMap != NULL)
  {
    delete[] m_piSliceSUMap;
    m_piSliceSUMap = NULL;
  }
  for( UInt uiCUAddr = 0; uiCUAddr < getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = getCU( uiCUAddr );
    pcCU->getNDBFilterBlocks()->clear();
  }

  if( m_bIndependentSliceBoundaryForNDBFilter || m_bIndependentTileBoundaryForNDBFilter)
  {
    m_pcNDBFilterYuvTmp->destroy();
    delete m_pcNDBFilterYuvTmp;
    m_pcNDBFilterYuvTmp = NULL;
  }

}

#endif


//! \}
