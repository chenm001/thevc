/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  ? *
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

/** \file     TComLoopFilter.cpp
    \brief    deblocking filter
*/

#include "TComLoopFilter.h"
#include "TComSlice.h"
#include "TComMv.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define   EDGE_VER    0
#define   EDGE_HOR    1
#define   QpUV(iQpY)  ( g_aucChromaScale[ Max( Min( (iQpY), MAX_QP ), MIN_QP ) ] )


#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
#define   DECIDE_FILTER                0
#define   EXECUTE_FILTER               1
#define   DECIDE_AND_EXECUTE_FILTER    2
#endif



// ====================================================================================================================
// Tables
// ====================================================================================================================

const UChar tctable_8x8[56] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,9,10,10,11,11,12,12,13,13,14,14
};

const UChar betatable_8x8[52] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComLoopFilter::TComLoopFilter()
: m_uiNumPartitions( 0 )
{
  m_uiDisableDeblockingFilterIdc = 0;
}

TComLoopFilter::~TComLoopFilter()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComLoopFilter::setCfg( UInt uiDisableDblkIdc, Int iAlphaOffset, Int iBetaOffset)
{
  m_uiDisableDeblockingFilterIdc  = uiDisableDblkIdc;
}

Void TComLoopFilter::create( UInt uiMaxCUDepth )
{
  m_uiNumPartitions = 1 << ( uiMaxCUDepth<<1 );
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
  {
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      m_aapucBS       [uiDir][uiPlane] = new UChar[m_uiNumPartitions];
      m_aapbEdgeFilter[uiDir][uiPlane] = new Bool [m_uiNumPartitions];
    }
  }
}

Void TComLoopFilter::destroy()
{
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
  {
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      delete [] m_aapucBS       [uiDir][uiPlane];
      delete [] m_aapbEdgeFilter[uiDir][uiPlane];
    }
  }
}

/**
 - call deblocking function for every CU
 .
 \param  pcPic   picture class (TComPic) pointer
 */
Void TComLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;
  
#if PARALLEL_MERGED_DEBLK
  pcPic->getPicYuvRec()->copyToPicLuma(pcPic->getPicYuvDeblkBuf());

  // Horizontal filtering
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    for( Int iPlane = 0; iPlane < 3; iPlane++ )
    {
      ::memset( m_aapucBS       [EDGE_VER][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
      assert( 0 == false );
      ::memset( m_aapbEdgeFilter[EDGE_VER][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
    }

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0, EDGE_VER );
  }

  // Vertical filtering
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    for( Int iPlane = 0; iPlane < 3; iPlane++ )
    {
      ::memset( m_aapucBS       [EDGE_HOR][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
      assert( 0 == false );
      ::memset( m_aapbEdgeFilter[EDGE_HOR][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
    }

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0, EDGE_HOR );
  }
#else
  // for every CU
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    
    for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
    {
      for( Int iPlane = 0; iPlane < 3; iPlane++ )
      {
        ::memset( m_aapucBS       [iDir][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
        assert( 0 == false );
        ::memset( m_aapbEdgeFilter[iDir][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
      }
    }
    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0 );
  }
#endif
}


// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#if PARALLEL_MERGED_DEBLK
/**
 - Deblocking filter process in CU-based (the same function as conventional's)
 .
 \param Edge          the direction of the edge in block boundary (horizonta/vertical), which is added newly
*/
Void TComLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int Edge )
#else
Void TComLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
#endif
{
  TComPic* pcPic     = pcCU->getPic();
  UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts   = uiCurNumParts>>2;
  
  if( pcCU->getDepth(uiAbsZorderIdx) > uiDepth )
  {
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#if PARALLEL_MERGED_DEBLK
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1, Edge );
#else
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1 );
#endif
    }
    return;
  }
  
  xSetLoopfilterParam( pcCU, uiAbsZorderIdx );
  
  xSetEdgefilterTU   ( pcCU, uiAbsZorderIdx, uiDepth );
  xSetEdgefilterPU   ( pcCU, uiAbsZorderIdx );
  
#if PARALLEL_MERGED_DEBLK
  Int iDir = Edge;
#else
  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
#endif
  {
    for( UInt uiPartIdx = uiAbsZorderIdx; uiPartIdx < uiAbsZorderIdx + uiCurNumParts; uiPartIdx++ )
    {
      if ( m_aapbEdgeFilter[iDir][0][uiPartIdx] )
      {
        xGetBoundaryStrengthSingle ( pcCU, uiAbsZorderIdx, iDir, uiPartIdx );
      }
    }
  }
  
  UInt uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1 ;
  
  UInt uiSizeInPU = pcPic->getNumPartInWidth()>>(uiDepth);
  
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge, DECIDE_FILTER);//Decide horizontal filter
  }
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, iEdge, DECIDE_AND_EXECUTE_FILTER);//Decide vertical filter
    if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
    {
      xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, iEdge );
    }
  }  
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge, EXECUTE_FILTER );//Execute horizontal filter
    if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
    {
      xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge );
    }
  }  
#else  
#if !PARALLEL_MERGED_DEBLK
  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
#endif
  {
    for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
    {
      xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
      if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
        xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
    }
  }
#endif  
}

Void TComLoopFilter::xSetEdgefilterMultiple( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdgeIdx, Bool bValue )
{
  const UInt uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
  const UInt uiHeightInBaseUnits = pcCU->getPic()->getNumPartInHeight() >> uiDepth;
  const UInt uiNumElem = iDir == 0 ? uiHeightInBaseUnits : uiWidthInBaseUnits;
  assert( uiNumElem > 0 );
  for( UInt ui = 0; ui < uiNumElem; ui++ )
  {
    const UInt uiBsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdgeIdx, ui );
    m_aapbEdgeFilter[iDir][0][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][1][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][2][uiBsIdx] = bValue;
  }
}

Void TComLoopFilter::xSetEdgefilterTU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
{
  if( pcCU->getTransformIdx( uiAbsZorderIdx ) + pcCU->getDepth( uiAbsZorderIdx) > uiDepth )
  {
    const UInt uiCurNumParts = pcCU->getPic()->getNumPartInCU() >> (uiDepth<<1);
    const UInt uiQNumParts   = uiCurNumParts>>2;
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      xSetEdgefilterTU( pcCU, uiAbsZorderIdx, uiDepth+1 );
    }
    return;
  }
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0, m_stLFCUParam.bInternalEdge );
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0, m_stLFCUParam.bInternalEdge );
}

Void TComLoopFilter::xSetEdgefilterPU( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  const UInt uiDepth = pcCU->getDepth( uiAbsZorderIdx );
  const UInt uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
  const UInt uiHeightInBaseUnits = pcCU->getPic()->getNumPartInHeight() >> uiDepth;
  const UInt uiHWidthInBaseUnits  = uiWidthInBaseUnits  >> 1;
  const UInt uiHHeightInBaseUnits = uiHeightInBaseUnits >> 1;
  
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0, m_stLFCUParam.bLeftEdge );
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0, m_stLFCUParam.bTopEdge );
  
  switch ( pcCU->getPartitionSize( uiAbsZorderIdx ) )
  {
    case SIZE_2Nx2N:
    {
      break;
    }
    case SIZE_2NxN:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
    case SIZE_Nx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiHWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
    case SIZE_NxN:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiHWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
    default:
    {
      assert(0);
      break;
    }
  }
}


Void TComLoopFilter::xSetLoopfilterParam( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  UInt uiX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];
  UInt uiY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];
  
#if MTK_NONCROSS_INLOOP_FILTER
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
#endif

  m_stLFCUParam.bInternalEdge = m_uiDisableDeblockingFilterIdc ? false : true ;
  
  if ( (uiX == 0) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bLeftEdge = false;
  else
    m_stLFCUParam.bLeftEdge = true;
#if MTK_NONCROSS_INLOOP_FILTER
  if ( m_stLFCUParam.bLeftEdge )
  {
    pcTempCU = pcCU->getPULeft( uiTempPartIdx, uiAbsZorderIdx, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false );
    if ( pcTempCU )
      m_stLFCUParam.bLeftEdge = true;
    else
      m_stLFCUParam.bLeftEdge = false;
  }
#endif
  
  if ( (uiY == 0 ) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bTopEdge = false;
  else
    m_stLFCUParam.bTopEdge = true;
#if MTK_NONCROSS_INLOOP_FILTER
  if ( m_stLFCUParam.bTopEdge )
  {
    pcTempCU = pcCU->getPUAbove( uiTempPartIdx, uiAbsZorderIdx, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false );
    if ( pcTempCU )
      m_stLFCUParam.bTopEdge = true;
    else
      m_stLFCUParam.bTopEdge = false;
  }
#endif
}

Void TComLoopFilter::xGetBoundaryStrengthSingle ( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, UInt uiAbsPartIdx )
{
  const UInt uiHWidth  = pcCU->getWidth( uiAbsZorderIdx ) >> 1;
  const UInt uiHHeight = pcCU->getHeight( uiAbsZorderIdx ) >> 1;
  const bool bAtCUBoundary = iDir == EDGE_VER ? g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] == g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]]
  : g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] == g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]];
  const bool bAtCUHalf     = iDir == EDGE_VER ? ( g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] + uiHWidth ) == g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]]
  : ( g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] + uiHHeight ) == g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]];
  TComSlice* const pcSlice = pcCU->getSlice();
  
  const UInt uiPartQ = uiAbsPartIdx;
  TComDataCU* const pcCUQ = pcCU;
  
  UInt uiPartP;
  TComDataCU* pcCUP;
  UInt uiBs;
  
#if MTK_NONCROSS_INLOOP_FILTER
  //-- Calculate Block Index
  if (iDir == EDGE_VER)
  {
    pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false);
  }
  else  // (iDir == EDGE_HOR)
  {
    pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false);
  }
#else
  //-- Calculate Block Index
  if (iDir == EDGE_VER)
  {
    pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ, false, false);
  }
  else  // (iDir == EDGE_HOR)
  {
    pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, false, false);
  }
#endif
  
  //-- Set BS for Intra MB : BS = 4 or 3
  if ( pcCUP->isIntra(uiPartP) || pcCUQ->isIntra(uiPartQ) )
  {
    uiBs = bAtCUBoundary ? 4 : 3;   // Intra MB && MB boundary
  }
  
  //-- Set BS for not Intra MB : BS = 2 or 1 or 0
  if ( !pcCUP->isIntra(uiPartP) && !pcCUQ->isIntra(uiPartQ) )
  {
    if ( pcCUQ->getCbf( uiPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(uiPartQ)) != 0 || pcCUP->getCbf( uiPartP, TEXT_LUMA, pcCUP->getTransformIdx(uiPartP) ) != 0)
    {
      uiBs = 2;
    }
    else
    {
      if (pcSlice->isInterB())
      {
        Int iRefIdx;
        Int *piRefP0, *piRefP1, *piRefQ0, *piRefQ1;
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
        piRefP0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartP);
        piRefP1 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
        piRefQ0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartQ);
        piRefQ1 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx);
        
        
        TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
        TComMv pcMvP1 = pcCUP->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartP);
        TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);
        TComMv pcMvQ1 = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartQ);
        
        if ( ((piRefP0==piRefQ0)&&(piRefP1==piRefQ1)) || ((piRefP0==piRefQ1)&&(piRefP1==piRefQ0)) )
        {
          uiBs = 0;
          if ( piRefP0 != piRefP1 )   // Different L0 & L1
          {
            if ( piRefP0 == piRefQ0 )
            {
              pcMvP0 -= pcMvQ0;   pcMvP1 -= pcMvQ1;
              uiBs = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
              (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
            }
            else
            {
              pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
              uiBs = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
              (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
            }
          }
          else    // Same L0 & L1
          {
            TComMv pcMvSub0 = pcMvP0 - pcMvQ0;
            TComMv pcMvSub1 = pcMvP1 - pcMvQ1;
            pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
            uiBs = ( (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
                    (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4) ) &&
            ( (pcMvSub0.getAbsHor() >= 4) | (pcMvSub0.getAbsVer() >= 4) |
             (pcMvSub1.getAbsHor() >= 4) | (pcMvSub1.getAbsVer() >= 4) );
          }
        }
        else // for all different Ref_Idx
        {
          uiBs = 1;
        }
      }
      else  // pcSlice->isInterP()
      {
        Int iRefIdx;
        Int *piRefP0, *piRefQ0;
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
        piRefP0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
        piRefQ0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
        TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);
        
        pcMvP0 -= pcMvQ0;
        uiBs = (piRefP0 != piRefQ0) | (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4);
      }
    }   // enf of "if( one of BCBP == 0 )"
  }   // enf of "if( not Intra )"
  
  m_aapucBS[iDir][0][uiAbsPartIdx] = uiBs;
  if ( bAtCUBoundary || bAtCUHalf )
  {
    m_aapucBS[iDir][1][uiAbsPartIdx] = uiBs;
    m_aapucBS[iDir][2][uiAbsPartIdx] = uiBs;
  }
}


#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge, Int iDecideExecute )
#else
Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge  )
#endif
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrc = piSrc;
#if PARALLEL_MERGED_DEBLK
  TComPicYuv* pcPicYuvJudge = pcCU->getPic()->getPicYuvDeblkBuf();
  Pel* piSrcJudge    = pcPicYuvJudge->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrcJudge = piSrcJudge;
#endif

  Int  iStride = pcPicYuvRec->getStride();
  Int  iQP = pcCU->getQP( uiAbsZorderIdx );
  UInt uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;
  
  UInt  uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt  PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1;
  UInt  uiBlocksInPart = uiPelsInPart / DEBLOCK_SMALLEST_BLOCK ? uiPelsInPart / DEBLOCK_SMALLEST_BLOCK : 1;
  UInt  uiBsAbsIdx, uiBs;
  Int   iOffset, iSrcStep;
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  Pel*  piTmpSrc1;
  UInt* piDecisions_D;
  UInt* piDecisions_Sample;
#endif  
  if (iDir == EDGE_VER)
  {
    iOffset = 1;
    iSrcStep = iStride;
    piTmpSrc += iEdge*uiPelsInPart;
#if PARALLEL_MERGED_DEBLK
    piTmpSrcJudge += iEdge*uiPelsInPart;
#endif
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset = iStride;
    iSrcStep = 1;
    piTmpSrc += iEdge*uiPelsInPart*iStride;
#if PARALLEL_MERGED_DEBLK
    piTmpSrcJudge += iEdge*uiPelsInPart*iStride;
#endif
  }
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {
    
    uiBs = 0;
    for (UInt iIdxInside = 0; iIdxInside<PartIdxIncr; iIdxInside++)
    {
      uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx+iIdxInside);
      if (uiBs < m_aapucBS[iDir][0][uiBsAbsIdx])
      {
        uiBs = m_aapucBS[iDir][0][uiBsAbsIdx];
      }
    }
    
    Int iBitdepthScale = (1<<(g_uiBitIncrement+g_uiBitDepth-8));
    
    UInt uiTcOffset = (uiBs>2)?4:0;
    
    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );
    Int iIndexB = Clip3(0, MAX_QP, iQP );
    
    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
    if (iDecideExecute==DECIDE_AND_EXECUTE_FILTER && uiBs)
    {
      Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
      
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
        Int iTmp=iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK;
        Int iD = xCalcD( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcD( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
        
        if (iD < iBeta)
        {
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
            xPelFilterLuma( piTmpSrc+iSrcStep*(iTmp+i), iOffset, iD, iBeta, iTc);
          }
        }
      }      
    }
    else if (iDecideExecute==DECIDE_FILTER && uiBs)
    {
      Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
      
      piDecisions_D      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_D     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piDecisions_Sample =  iIdx*uiPelsInPart                         + m_decisions_Sample[(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
        Int iTmp=iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK;
        Int iD = xCalcD( piTmpSrc+(iTmp+2), iOffset) + xCalcD( piTmpSrc+(iTmp+5), iOffset);
        piTmpSrc1 = piTmpSrc+iTmp;
        
        if (iD < iBeta)
        {
          *piDecisions_D++=1;
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
            *piDecisions_Sample++ = xPelFilterLumaDecision( piTmpSrc1++, iOffset, iD, iBeta, iTc);
          }
        }
        else
        {
          *piDecisions_D++=0;
          piDecisions_Sample+=DEBLOCK_SMALLEST_BLOCK;
        }
      }
    }
    else if ( uiBs ) // EXECUTE_FILTER
    {
      piDecisions_D      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_D     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piDecisions_Sample =  iIdx*uiPelsInPart                         + m_decisions_Sample[(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piTmpSrc1          = piTmpSrc+iIdx*uiPelsInPart;
      
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
        if ( *piDecisions_D++ )
        {
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
            xPelFilterLumaExecution( piTmpSrc1++, iOffset, iTc, *piDecisions_Sample++  );
          }
        }
        else
        {
          piDecisions_Sample+=DEBLOCK_SMALLEST_BLOCK;
          piTmpSrc1+=DEBLOCK_SMALLEST_BLOCK;
        }
      }
    }
#else
    Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
    
    
    for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
    {
      if ( uiBs )
      {
#if PARALLEL_MERGED_DEBLK
        Int iD = xCalcD( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcD( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
#else  
        Int iD = xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
#endif
        if (iD < iBeta)
        {
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
#if PARALLEL_MERGED_DEBLK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i));
#else           
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc );
#endif
          }
        }
      }
    }
#endif
  }
}


Void TComLoopFilter::xEdgeFilterChroma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
  
  Int   iQP = QpUV((Int) pcCU->getQP( uiAbsZorderIdx ));
  UInt  uiPelsInPartChroma = g_uiMaxCUWidth >> (g_uiMaxCUDepth+1);
  
  Int   iOffset, iSrcStep;
  
  const UInt uiLCUWidthInBaseUnits = pcCU->getPic()->getNumPartInWidth();
  
  // Vertical Position
  UInt uiEdgeNumInLCUVert = g_auiZscanToRaster[uiAbsZorderIdx]%uiLCUWidthInBaseUnits + iEdge;
  UInt uiEdgeNumInLCUHor = g_auiZscanToRaster[uiAbsZorderIdx]/uiLCUWidthInBaseUnits + iEdge;
  
  if ( ( (uiEdgeNumInLCUVert%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&&(iDir==0) ) || ( (uiEdgeNumInLCUHor%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&& iDir ) )
  {
    return;
  }
  
  UInt  uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;
  
  UInt  uiBsAbsIdx;
  UChar ucBs;
  
  Pel* piTmpSrcCb = piSrcCb;
  Pel* piTmpSrcCr = piSrcCr;
  
  
  if (iDir == EDGE_VER)
  {
    iOffset   = 1;
    iSrcStep  = iStride;
    piTmpSrcCb += iEdge*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*uiPelsInPartChroma;
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset   = iStride;
    iSrcStep  = 1;
    piTmpSrcCb += iEdge*iStride*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*iStride*uiPelsInPartChroma;
  }
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx++ )
  {
    ucBs = 0;
    
    uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);
    ucBs = m_aapucBS[iDir][0][uiBsAbsIdx];
    
    Int iBitdepthScale = (1<<(g_uiBitIncrement+g_uiBitDepth-8));
    
    UInt uiTcOffset = (ucBs>2)?4:0;
    
    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );
    
    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;
    
    if ( ucBs > 2)
    {
      for ( UInt uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
      {
        xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc );
        xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc );
      }
    }
  }
}

#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
/**
 - Decision for one line/column for the luminance component to use strong or weak deblocking
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param d             d value
 \param beta          beta value
 \param tc            tc value

 \returns decision to use strong or weak deblocking
 */
__inline Int TComLoopFilter::xPelFilterLumaDecision( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc)
{
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  
  if ( ((abs(piSrc[-iOffset*4]-m3) + abs(piSrc[ iOffset*3]-m4)) < (beta>>3)) && (d<(beta>>2)) && ( abs(m3-m4) < ((tc*5+1)>>1)) ) //strong filtering
  {
    return 1;
  }
  else //weak filtering
  {
    return 0;
  }
}


/**
 - Strong deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param m0            sample value
 \param m1            sample value
 \param m2            sample value
 \param m3            sample value
 \param m4            sample value
 \param m5            sample value
 \param m6            sample value
 \param m7            sample value
 */
__inline Void TComLoopFilter::xPelFilterLumaStrong(Pel* piSrc, Int iOffset, Pel m0, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6, Pel m7)
{
  piSrc[-iOffset] = Clip(( m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3 );
  piSrc[0] = Clip(( m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3 );

  piSrc[-iOffset*2] = Clip(( m1 + m2 + m3 + m4 + 2)>>2);
  piSrc[ iOffset] = Clip(( m3 + m4 + m5 + m6 + 2)>>2);

  piSrc[-iOffset*3] = Clip(( 2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3);
  piSrc[ iOffset*2] = Clip(( m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3);  
}

/**
 - Weak deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param tc            tc value
 \param m1            sample value
 \param m2            sample value
 \param m3            sample value
 \param m4            sample value
 \param m5            sample value
 \param m6            sample value
 */
__inline Void TComLoopFilter::xPelFilterLumaWeak(Pel* piSrc, Int iOffset, Int tc, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6)
{
  Int delta = Clip3(-tc, tc, ((13*(m4-m3) + 4*(m5-m2) - 5*(m6-m1)+16)>>5) );

  piSrc[-iOffset] = Clip(m3+delta);
  piSrc[0] = Clip(m4-delta);
  piSrc[-iOffset*2] = Clip(m2+delta/2);
  piSrc[ iOffset] = Clip(m5-delta/2);
}
#endif

#if PARALLEL_MERGED_DEBLK
/**
 - Deblocking for the luminance component with strong or weak filter
 .
 \param piSrcJudge    pointer to picture data for decision
*/
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Pel* piSrcJudge)
#else
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc )
#endif
{
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  Int d_strong;
#else  
  Int d_strong, delta;
#endif
  
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  Pel m6  = piSrc[ iOffset*2];
  Pel m1  = piSrc[-iOffset*3];
  Pel m7  = piSrc[ iOffset*3];
  Pel m0  = piSrc[-iOffset*4];
#if PARALLEL_MERGED_DEBLK
  Pel m4j  = piSrcJudge[0];
  Pel m3j  = piSrcJudge[-iOffset];
  Pel m7j  = piSrcJudge[ iOffset*3];
  Pel m0j  = piSrcJudge[-iOffset*4];

  d_strong = abs(m0j-m3j) + abs(m7j-m4j);

  if ( (d_strong < (beta>>3)) && (d<(beta>>2)) && ( abs(m3j-m4j) < ((tc*5+1)>>1)) ) //strong filtering
#else
  d_strong = abs(m0-m3) + abs(m7-m4);
  
  if ( (d_strong < (beta>>3)) && (d<(beta>>2)) && ( abs(m3-m4) < ((tc*5+1)>>1)) ) //strong filtering
#endif
  {
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
    xPelFilterLumaStrong(piSrc, iOffset, m0, m1, m2, m3, m4, m5, m6, m7);
#else  
    piSrc[-iOffset] = Clip(( m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3 );
    piSrc[0] = Clip(( m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3 );
    
    piSrc[-iOffset*2] = Clip(( m1 + m2 + m3 + m4 + 2)>>2);
    piSrc[ iOffset] = Clip(( m3 + m4 + m5 + m6 + 2)>>2);
    
    piSrc[-iOffset*3] = Clip(( 2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3);
    piSrc[ iOffset*2] = Clip(( m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3);
#endif
  }
  else
  {
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
    xPelFilterLumaWeak(piSrc, iOffset, tc, m1, m2, m3, m4, m5, m6);
#else    
    /* Weak filter */
    delta = Clip3(-tc, tc, ((13*(m4-m3) + 4*(m5-m2) - 5*(m6-m1)+16)>>5) );
    
    piSrc[-iOffset] = Clip(m3+delta);
    piSrc[0] = Clip(m4-delta);
    piSrc[-iOffset*2] = Clip(m2+delta/2);
    piSrc[ iOffset] = Clip(m5-delta/2);
#endif
  }
}

#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
/**
 - Deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param tc            tc value
 \param strongFilter  indicator to use either strong or weak filter      
 */
__inline Void TComLoopFilter::xPelFilterLumaExecution( Pel* piSrc, Int iOffset, Int tc, Int strongFilter)
{  
  if (strongFilter) //strong filtering
  {
    xPelFilterLumaStrong(piSrc, iOffset, piSrc[-iOffset*4], piSrc[-iOffset*3], piSrc[-iOffset*2], piSrc[-iOffset], piSrc[0], piSrc[ iOffset], piSrc[ iOffset*2], piSrc[ iOffset*3]);
  }
  else //weak filtering
  {
    xPelFilterLumaWeak(piSrc, iOffset, tc, piSrc[-iOffset*3], piSrc[-iOffset*2], piSrc[-iOffset], piSrc[0], piSrc[ iOffset], piSrc[ iOffset*2]);
  }
}
#endif

__inline Void TComLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset, Int tc )
{
  int delta;
  
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  
  delta = Clip3(-tc,tc, (((( m4 - m3 ) << 2 ) + m2 - m5 + 4 ) >> 3) );
  piSrc[-iOffset] = Clip(m3+delta);
  piSrc[0] = Clip(m4-delta);
}


__inline Int TComLoopFilter::xCalcD( Pel* piSrc, Int iOffset)
{
  return abs( piSrc[-iOffset*3] - 2*piSrc[-iOffset*2] + piSrc[-iOffset] ) + abs( piSrc[0] - 2*piSrc[iOffset] + piSrc[iOffset*2] );
}
