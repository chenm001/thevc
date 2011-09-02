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

/** \file     TDecSlice.cpp
    \brief    slice decoder class
*/

#include "TDecSlice.h"

//! \ingroup TLibDecoder
//! \{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSlice::TDecSlice()
{
#if OL_USE_WPP
  m_pcBufferSbacDecoders = NULL;
  m_pcBufferBinCABACs    = NULL;
#endif
}

TDecSlice::~TDecSlice()
{
}

Void TDecSlice::create( TComSlice* pcSlice, Int iWidth, Int iHeight, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth )
{
}

Void TDecSlice::destroy()
{
#if OL_USE_WPP
  if ( m_pcBufferSbacDecoders )
  {
  delete[] m_pcBufferSbacDecoders;
  m_pcBufferSbacDecoders = NULL;
  }
  if ( m_pcBufferBinCABACs )
  {
  delete[] m_pcBufferBinCABACs;
  m_pcBufferBinCABACs = NULL;
  }
#endif
}

Void TDecSlice::init(TDecEntropy* pcEntropyDecoder, TDecCu* pcCuDecoder)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcCuDecoder       = pcCuDecoder;
#if OL_USE_WPP
  // Want one per column of tiles, ideally.
  m_pcBufferSbacDecoders = new TDecSbac    [OL_MAX_TILES];  
  m_pcBufferBinCABACs  = new TDecBinCABAC[OL_MAX_TILES];
  for (UInt ui = 0; ui < OL_MAX_TILES; ui++)
    m_pcBufferSbacDecoders[ui].init(&m_pcBufferBinCABACs[ui]);
  
#endif
}

Void TDecSlice::decompressSlice(TComInputBitstream* pcBitstream, TComPic*& rpcPic)
#if OL_USE_WPP
Void TDecSlice::decompressSlice(TComBitstream* pcBitstream, TComBitstream** ppcSubstreams, TComPic*& rpcPic, TDecSbac* pcSbacDecoder, TDecSbac* pcSbacDecoders)
#else
//Void TDecSlice::decompressSlice(TComBitstream* pcBitstream, TComPic*& rpcPic)
#endif
{
  TComDataCU* pcCU;
  UInt        uiIsLast = 0;
#if FINE_GRANULARITY_SLICES
#if TILES
  Int   iStartCUEncOrder = max(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr()/rpcPic->getNumPartInCU(), rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr()/rpcPic->getNumPartInCU());
  Int   iStartCUAddr = rpcPic->getPicSym()->getCUOrderMap(iStartCUEncOrder);
#else
  Int   iStartCUAddr = max(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr()/rpcPic->getNumPartInCU(), rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr()/rpcPic->getNumPartInCU());
#endif
#else
#if TILES
  Int   iStartCUAddr;
  if(rpcPic->getPicSym()->getInverseCUOrderMap(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr()) > 
     rpcPic->getPicSym()->getInverseCUOrderMap(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr()))
  {
    iStartCUAddr = rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr();
  }
  else
  {
    iStartCUAddr = rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr();
  }
#else
  Int   iStartCUAddr = max(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr(), rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr());
#endif
#endif

  // decoder don't need prediction & residual frame buffer
  rpcPic->setPicYuvPred( 0 );
  rpcPic->setPicYuvResi( 0 );
  
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( rpcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );

#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif

#if OL_USE_WPP
  TComSlice*  pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());
  UInt iSymbolMode    = pcSlice->getSymbolMode();

  if( iSymbolMode )
  {
  //m_pcBufferSbacDecoders[0].load(pcSbacDecoder);  //save init. state
    for (UInt ui = 0; ui < OL_MAX_TILES; ui++)
      m_pcBufferSbacDecoders[ui].load(pcSbacDecoder);
  }

  UInt uiWidthInLCUs  = rpcPic->getPicSym()->getFrameWidthInCU();
  UInt uiHeightInLCUs = rpcPic->getPicSym()->getFrameHeightInCU();
  UInt uiCol=0, uiLin=0, uiSubStrm=0;
#if TILES
  Int iBreakDep;
  UInt uiTileCol;
  UInt uiTileStartLCU;
  UInt uiSliceStartLCU;
  UInt uiTileLCUX;
  UInt uiTileLCUY;
#endif
#endif


#if TILES
  for( Int iCUAddr = iStartCUAddr; !uiIsLast && iCUAddr < rpcPic->getNumCUsInFrame(); iCUAddr = rpcPic->getPicSym()->xCalculateNxtCUAddr(iCUAddr) )
#else
  // for all CUs in slice
  UInt  uiLastCUAddr = iStartCUAddr;
  for( Int iCUAddr = iStartCUAddr; !uiIsLast && iCUAddr < rpcPic->getNumCUsInFrame(); iCUAddr++, uiLastCUAddr++ )
#endif
  {
    pcCU = rpcPic->getCU( iCUAddr );
    pcCU->initCU( rpcPic, iCUAddr );

#if OL_USE_WPP
  if( iSymbolMode )
  {
#if TILES
    iBreakDep = rpcPic->getPicSym()->getTileBoundaryIndependenceIdr();
    uiTileCol = rpcPic->getPicSym()->getTileIdxMap(iCUAddr) % (rpcPic->getPicSym()->getNumColumnsMinus1()+1); // what column of tiles are we in?
    uiTileStartLCU = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr();
    uiTileLCUX = uiTileStartLCU % uiWidthInLCUs;
    uiTileLCUY = uiTileStartLCU / uiWidthInLCUs;
    uiSliceStartLCU = pcSlice->getSliceCurStartCUAddr();
    uiCol     = iCUAddr % uiWidthInLCUs;
    uiLin     = iCUAddr / uiWidthInLCUs;
    uiSubStrm = OL_TILE_SUBSTREAMS ? rpcPic->getPicSym()->getTileIdxMap(iCUAddr)*OL_NUM_SUBSTREAMS+uiLin%OL_NUM_SUBSTREAMS : uiLin%OL_NUM_SUBSTREAMS;
    m_pcEntropyDecoder->setBitstream( ppcSubstreams[uiSubStrm] );
    // Synchronize cabac probabilities with upper-right LCU
    TComDataCU *pcCUTR = pcCU->getCUAboveRight();
    //printf("y=%d x=%d\n", uiLin, uiCol);
    if( (true/*bEnforceSliceRestriction*/ &&
        (pcCUTR==NULL || pcCUTR->getSlice()==NULL || rpcPic->getPicSym()->getInverseCUOrderMap( pcCUTR->getAddr()) < rpcPic->getPicSym()->getTempInverseCUOrderMap(uiSliceStartLCU) ||
        (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(iCUAddr) ))) ||
        (true/*bEnforceEntropySliceRestriction*/ &&
        (pcCUTR==NULL || pcCUTR->getSlice()==NULL || rpcPic->getPicSym()->getInverseCUOrderMap( pcCUTR->getAddr()) < rpcPic->getPicSym()->getTempInverseCUOrderMap(pcSlice->getEntropySliceCurStartCUAddr())
||
        (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(iCUAddr) )))
      )
    {
      // TR not available.
      //printf("TR of %d not avail\n", pcCU->getAddr());
    }
    else
    {
      // TR is available, do we use it?
      if ( uiCol == uiTileLCUX )
      {
        //printf("y=%d x=%d inherit %d\n", uiLin, uiCol, uiTileCol);
        pcSbacDecoders[uiSubStrm].loadContexts( &m_pcBufferSbacDecoders[uiTileCol] );
      }
    }
#else
    uiCol     = iCUAddr % uiWidthInLCUs;
    uiLin     = iCUAddr / uiWidthInLCUs;
    uiSubStrm = uiLin   % OL_NUM_SUBSTREAMS;    //index of {substream/entropy coder}
     
    m_pcEntropyDecoder->setBitstream( ppcSubstreams[uiSubStrm] );
    // Synchronize cabac probabilities with upper-right LCU
    TComDataCU *pcCUTR = pcCU->getCUAboveRight();
    if (pcCUTR != NULL
        && pcCUTR->getSlice() != NULL
        && pcCUTR->getAddr() >= pcSlice->getSliceCurStartCUAddr()
        && pcCUTR->getAddr() >= pcSlice->getEntropySliceCurStartCUAddr())
    {
      // TR is available, do we use it?
      if ( !uiCol && iCUAddr )  pcSbacDecoders[uiSubStrm].loadContexts( &m_pcBufferSbacDecoders[0] );
    }
#endif

    pcSbacDecoder->load(&pcSbacDecoders[uiSubStrm]);  //this load is used to simplify the code (avoid to change all the call to pcSbacDecoders)
  }
#endif
#if TILES
    TComSlice *pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());
#if OL_USE_WPP && !OL_TILE_SUBSTREAMS
    if ( uiCol   == uiTileLCUX && uiLin >= uiTileLCUY && uiLin-uiTileLCUY < OL_NUM_SUBSTREAMS && // 1st in tile for this ss.
         iCUAddr != (uiLin%OL_NUM_SUBSTREAMS)*uiWidthInLCUs && // !1st in frame for this ss.
         iCUAddr/uiWidthInLCUs - iStartCUAddr/uiWidthInLCUs > 0 && iCUAddr/uiWidthInLCUs - iStartCUAddr/uiWidthInLCUs < OL_NUM_SUBSTREAMS && // 1st in slice for ss
         rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()) // independant.
#else
    if ( iCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr() && // 1st in tile.
         iCUAddr!=0 && iCUAddr!=iStartCUAddr && rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()) // !1st in frame && !1st in slice && tile-independant
#endif
    {
      if (pcSlice->getSymbolMode())
      {
#if !OL_USE_WPP
          /*m_pcEntropyDecoder->updateContextTables( pcSlice->getSliceType(), pcSlice->getSliceQp() );*/
#elif OL_TILE_SUBSTREAMS
      // We're crossing into another tile, tiles are independent.
      // There's normally a reset at that point.  If we have "substreams per frame" we leave the original reset.  If
      // we have "substreams per tile" our substreams have already been terminated, and the reset is no longer required.
#else
        {
          printf("addr %d crossed to start of next tile\n", iCUAddr);
          m_pcEntropyDecoder->updateContextTables( pcSlice->getSliceType(), pcSlice->getSliceQp() );
          if (uiLin-uiTileLCUY > 0) // !1st line in tile -- inherit our TR.  The updateContextTables has otherwise reset it.
          {
            printf("(inheriting TR)\n");
            //pcSbacDecoders[uiSubStrm].loadContexts( &m_pcBufferSbacDecoders[uiTileCol] );
            pcSbacDecoder->loadContexts(&m_pcBufferSbacDecoders[uiTileCol]);
          }
        }
#endif
      }
      else
      {
        /*m_pcEntropyDecoder->resetEntropy( pcSlice );*/
      }
    }
#endif

#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif

    m_pcCuDecoder->decodeCU     ( pcCU, uiIsLast );
    m_pcCuDecoder->decompressCU ( pcCU );
    
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif
#if OL_USE_WPP
  if( iSymbolMode )
  {
     pcSbacDecoders[uiSubStrm].load(pcSbacDecoder);

     //Store probabilties of second LCU in line into buffer
     // !!!
#if TILES
    if (uiCol == uiTileLCUX+OL_SHIFT)
    {
      m_pcBufferSbacDecoders[uiTileCol].loadContexts( &pcSbacDecoders[uiSubStrm] );
    }
#else
    if (uiCol == OL_SHIFT)
    {
      m_pcBufferSbacDecoders[0].loadContexts( &pcSbacDecoders[uiSubStrm] );
    }
#endif

  }
#endif
  }
}
//! \}
