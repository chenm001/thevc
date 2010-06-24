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

// ====================================================================================================================
// Tables
// ====================================================================================================================

#if TENTM_DEBLOCKING_FILTER

const UChar tctable_8x8[56] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,9,10,10,11,11,12,12,13,13,14,14
};

const UChar betatable_8x8[52] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

#else
const UChar ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
const UChar  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
const UChar CLIP_TAB[52][5]  =
{
  { 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 1, 1, 1, 1},
  { 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 2, 3, 3},
  { 0, 1, 2, 3, 3},{ 0, 2, 2, 3, 3},{ 0, 2, 2, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 3, 3, 5, 5},{ 0, 3, 4, 6, 6},{ 0, 3, 4, 6, 6},
  { 0, 4, 5, 7, 7},{ 0, 4, 5, 8, 8},{ 0, 4, 6, 9, 9},{ 0, 5, 7,10,10},{ 0, 6, 8,11,11},{ 0, 6, 8,13,13},{ 0, 7,10,14,14},{ 0, 8,11,16,16},
  { 0, 9,12,18,18},{ 0,10,13,20,20},{ 0,11,15,23,23},{ 0,13,17,25,25}
};
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComLoopFilter::TComLoopFilter()
#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
: m_uiNumPartitions( 0 )
#endif
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
#if !TENTM_DEBLOCKING_FILTER
  m_iAlphaOffset                  = iAlphaOffset;
  m_iBetaOffset                   = iBetaOffset;
#endif
}

#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
Void TComLoopFilter::create( UInt uiMaxCUDepth )
{
  m_uiNumPartitions = 1 << ( uiMaxCUDepth<<1 );
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      m_aapucBS       [uiDir][uiPlane] = new UChar[m_uiNumPartitions];
      m_aapbEdgeFilter[uiDir][uiPlane] = new Bool [m_uiNumPartitions];
    }
}

Void TComLoopFilter::destroy()
{
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      delete [] m_aapucBS       [uiDir][uiPlane];
      delete [] m_aapbEdgeFilter[uiDir][uiPlane];
    }
}
#endif

/**
    - call deblocking function for every CU
    .
    \param  pcPic   picture class (TComPic) pointer
 */
Void TComLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;

  // for every CU
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
    for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
      for( Int iPlane = 0; iPlane < 3; iPlane++ )
      {
        ::memset( m_aapucBS       [iDir][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
        assert( 0 == false );
        ::memset( m_aapbEdgeFilter[iDir][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
      }
#endif
    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0 );
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TComLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
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
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1 );
    }
    return;
  }

  xSetLoopfilterParam( pcCU, uiAbsZorderIdx );

#if PLANAR_INTRA
  Bool bVerDone = false;
  Bool bHorDone = false;

  if ( pcCU->getPlanarInfo( uiAbsZorderIdx, PLANAR_FLAG) )
  {
    // Check the vertical (left) border
    if (m_stLFCUParam.bLeftEdge )
    {
      UInt        uiPartP;
      TComDataCU* pcCUP = pcCU->getPULeft(uiPartP, uiAbsZorderIdx);

      if ( pcCUP->getPlanarInfo( uiPartP, PLANAR_FLAG) && (pcCU->getHeight(uiAbsZorderIdx) == pcCUP->getHeight(uiPartP) ) )
      {
        xEdgeFilterPlanarIntra( pcCU, uiAbsZorderIdx, EDGE_VER );
        bVerDone = true;
      }
    }

    // Check the horizontal (top) border
    if (m_stLFCUParam.bTopEdge )
    {
      UInt        uiPartP;
      TComDataCU* pcCUP = pcCU->getPUAbove(uiPartP, uiAbsZorderIdx);

      if ( pcCUP->getPlanarInfo( uiPartP, PLANAR_FLAG)  && (pcCU->getWidth(uiAbsZorderIdx) == pcCUP->getWidth(uiPartP) ))
      {
        xEdgeFilterPlanarIntra( pcCU, uiAbsZorderIdx, EDGE_HOR );
        bHorDone = true;
      }
    }
  }

  if ( bVerDone && bHorDone )
    return;
#endif

#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
  xSetEdgefilterTU   ( pcCU, uiAbsZorderIdx, uiDepth );
  xSetEdgefilterPU   ( pcCU, uiAbsZorderIdx );

  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
    for( UInt uiPartIdx = uiAbsZorderIdx; uiPartIdx < uiAbsZorderIdx + uiCurNumParts; uiPartIdx++ )
    {
      if ( m_aapbEdgeFilter[iDir][0][uiPartIdx] )
      {
        xGetBoundaryStrengthSingle ( pcCU, uiAbsZorderIdx, iDir, uiPartIdx );
      }
    }
  }
#endif

#if HHI_DEBLOCKING_FILTER
  for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  for( UInt uiPartIdx = uiAbsZorderIdx; uiPartIdx < uiAbsZorderIdx + uiCurNumParts; uiPartIdx++ )
  {
#if PLANAR_INTRA
    if ( ( iDir == EDGE_VER && bVerDone ) || ( iDir == EDGE_HOR && bHorDone ) )
      continue;
#endif
    xEdgeFilterLumaSingle  ( pcCU, uiPartIdx, iDir );
    xEdgeFilterChromaSingle( pcCU, uiPartIdx, iDir );
  }
#elif TENTM_DEBLOCKING_FILTER
  UInt uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1 ;

  UInt uiSizeInPU = pcPic->getNumPartInWidth()>>(uiDepth);

  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
#if PLANAR_INTRA
    if ( ( iDir == EDGE_VER && bVerDone ) || ( iDir == EDGE_HOR && bHorDone ) )
      continue;
#endif
    for ( Int iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
    {
      xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
      if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
        xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
    }
  }
#else
  xSetEdgefilter     ( pcCU, uiAbsZorderIdx );

  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
#if PLANAR_INTRA
    if ( ( iDir == EDGE_VER && bVerDone ) || ( iDir == EDGE_HOR && bHorDone ) )
      continue;
#endif
    for ( Int iEdge = 0; iEdge < 4; iEdge++ )
    {
      if ( m_stLFCUParam.bLumaEdgeFilter[iDir][iEdge] )
      {
        xGetBoundaryStrength ( pcCU, uiAbsZorderIdx, iDir, iEdge, uiDepth );
        if (m_stLFCUParam.iBsEdgeSum[iDir][iEdge] != 0)
        {
          xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, iDir, iEdge );
          xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, iDir, iEdge );
        }
      }
    }
  }
#endif
}

#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
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
  const UInt uiQWidthInBaseUnits  = uiWidthInBaseUnits  >> 2;
  const UInt uiQHeightInBaseUnits = uiHeightInBaseUnits >> 2;

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
  case SIZE_2NxnU:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiQHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_2NxnD:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHeightInBaseUnits - uiQHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_nLx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiQWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_nRx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiWidthInBaseUnits - uiQWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  default:
    {
      assert(0);
      break;
    }
}
}

#if PLANAR_INTRA
Void TComLoopFilter::xPelFilterPlanarIntra( Pel* piSrc, Int iOffset, Int iBlkSize )
{
  Int a1,a2,bitShift;
  Int blkSizeHalf    = iBlkSize >> 1;
  Int blkSizeQuarter = iBlkSize >> 2;
  Int k;

  switch (iBlkSize)
  {
  case 2:   return;
  case 4:   bitShift = 1; break;
  case 8:   bitShift = 2; break;
  case 16:  bitShift = 3; break;
  case 32:  bitShift = 4; break;
  case 64:  bitShift = 5; break;
  case 128: bitShift = 6; break;
  default: assert( iBlkSize ); break;
  }

  a1 = piSrc[-blkSizeQuarter*iOffset];
  a2 = piSrc[ blkSizeQuarter*iOffset];

  for(k=1;k<blkSizeHalf;k++)
  {
      piSrc[(-blkSizeQuarter+k)*iOffset] = (a1*(blkSizeHalf-k) + a2*k + blkSizeQuarter) >> bitShift;
  }
}

Void TComLoopFilter::xEdgeFilterPlanarIntra( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel*        piSrc       = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );

  Int   iStride   = pcPicYuvRec->getStride();
  Int   iOffset, iSrcStep;
  Int   iNumStep;

  // Luminance parameters
  if (iDir == EDGE_VER)
  {
    iOffset  = 1;
    iSrcStep = iStride;
    iNumStep = pcCU->getHeight(uiAbsZorderIdx);
  }
  else
  {
    iOffset  = iStride;
    iSrcStep = 1;
    iNumStep = pcCU->getWidth(uiAbsZorderIdx);
  }

  // Filter Y component
  for ( UInt uiStep = 0; uiStep < iNumStep; uiStep++ )
  {
    xPelFilterPlanarIntra( piSrc+iSrcStep*uiStep, iOffset, iNumStep );
  }

  // Chrominance parameters
  iStride = pcPicYuvRec->getCStride();

  if (iDir == EDGE_VER)
  {
    iOffset  = 1;
    iSrcStep = iStride;
    iNumStep = pcCU->getHeight(uiAbsZorderIdx) >> 1;
  }
  else
  {
    iOffset  = iStride;
    iSrcStep = 1;
    iNumStep = pcCU->getWidth(uiAbsZorderIdx) >> 1;
  }

  // Filter U component
  piSrc = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );

  for ( UInt uiStep = 0; uiStep < iNumStep; uiStep++ )
  {
    xPelFilterPlanarIntra( piSrc + iSrcStep*uiStep, iOffset, iNumStep );
  }

  // Filter V component
  piSrc = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );

  for ( UInt uiStep = 0; uiStep < iNumStep; uiStep++ )
  {
    xPelFilterPlanarIntra( piSrc + iSrcStep*uiStep, iOffset, iNumStep );
  }
}
#endif
#else

Void TComLoopFilter::xSetEdgefilter( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  switch ( pcCU->getPartitionSize( uiAbsZorderIdx ) )
  {
  case SIZE_2Nx2N:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_2NxN:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_Nx2N:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_NxN:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_2NxnU:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_2NxnD:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = m_stLFCUParam.bInternalEdge;
      break;
    }
  case SIZE_nLx2N:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = false;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  case SIZE_nRx2N:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][0] = m_stLFCUParam.bLeftEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = m_stLFCUParam.bInternalEdge;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][0] = m_stLFCUParam.bTopEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = false;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = false;
      break;
    }
  default:
    {
      assert(0);
      break;
    }
  }

  switch ( pcCU->getTransformIdx( uiAbsZorderIdx ) )
  {
  case 0:
    {
      //follows to prediction boundary??
      break;
    }
  case 1:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = m_stLFCUParam.bInternalEdge;
      break;
    }
  case 2:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = m_stLFCUParam.bInternalEdge;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = m_stLFCUParam.bInternalEdge;
      break;
    }
  default:
    {
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_VER][3] = m_stLFCUParam.bInternalEdge;

      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][1] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][2] = m_stLFCUParam.bInternalEdge;
      m_stLFCUParam.bLumaEdgeFilter[EDGE_HOR][3] = m_stLFCUParam.bInternalEdge;
      break;
    }
  }
}
#endif

Void TComLoopFilter::xSetLoopfilterParam( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  UInt uiX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];
  UInt uiY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];

  m_stLFCUParam.bInternalEdge = m_uiDisableDeblockingFilterIdc ? false : true ;

  if ( (uiX == 0) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bLeftEdge = false;
  else
    m_stLFCUParam.bLeftEdge = true;

  if ( (uiY == 0 ) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bTopEdge = false;
  else
    m_stLFCUParam.bTopEdge = true;
}

#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
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

  //-- Calculate Block Index
  if (iDir == EDGE_VER)
  {
    pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ);
  }
  else  // (iDir == EDGE_HOR)
  {
    pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ);
  }

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
#endif

#if HHI_DEBLOCKING_FILTER
Void TComLoopFilter::xEdgeFilterLumaSingle( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel*        piSrc       = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );

  Int   iStride   = pcPicYuvRec->getStride();
  Int   iQP = pcCU->getQP( uiAbsZorderIdx );
  Int   iOffset, iSrcStep;
  UInt  uiNumStep;

  if (iDir == EDGE_VER)
  {
    iOffset = 1;
    iSrcStep = iStride;
    uiNumStep = g_uiMaxCUHeight >> g_uiMaxCUDepth;
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset = iStride;
    iSrcStep = 1;
    uiNumStep = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  }

  const UChar ucBs = m_aapucBS[iDir][0][uiAbsZorderIdx];

  if ( ucBs )
  {
    for ( UInt uiStep = 0; uiStep < uiNumStep; uiStep++ )
    {
      xPelFilterLuma( piSrc+iSrcStep*uiStep, iOffset, ucBs, iQP );
    }
  }
}

Void TComLoopFilter::xEdgeFilterChromaSingle( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );

  Int   iQP       = QpUV((Int) pcCU->getQP( uiAbsZorderIdx ));

  UInt  uiNumStep;
  Int   iOffset, iSrcStep;

  if (iDir == EDGE_VER)
  {
    iOffset   = 1;
    iSrcStep  = iStride;
    uiNumStep = g_uiMaxCUHeight >> ( 1 + g_uiMaxCUDepth );
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset   = iStride;
    iSrcStep  = 1;
    uiNumStep = g_uiMaxCUWidth >> ( 1 + g_uiMaxCUDepth );
  }

  const UChar ucBsCb = m_aapucBS[iDir][1][uiAbsZorderIdx];
  const UChar ucBsCr = m_aapucBS[iDir][2][uiAbsZorderIdx];

  if ( ucBsCb )
  {
    for ( UInt uiStep = 0; uiStep < uiNumStep; uiStep++ )
    {
      xPelFilterChroma( piSrcCb + iSrcStep*uiStep, iOffset, ucBsCb, iQP );
    }
  }

  if ( ucBsCr )
  {
    for ( UInt uiStep = 0; uiStep < uiNumStep; uiStep++ )
    {
      xPelFilterChroma( piSrcCr + iSrcStep*uiStep, iOffset, ucBsCr, iQP );
    }
  }
}

#elif TENTM_DEBLOCKING_FILTER
Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge  )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrc = piSrc;

  Int  iStride = pcPicYuvRec->getStride();
  Int  iQP = pcCU->getQP( uiAbsZorderIdx );
  UInt uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;

  UInt  uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt  PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1;
  UInt  uiBlocksInPart = uiPelsInPart / DEBLOCK_SMALLEST_BLOCK ? uiPelsInPart / DEBLOCK_SMALLEST_BLOCK : 1;
  UInt  uiBsAbsIdx, uiBs;
  Int   iOffset, iSrcStep;

  if (iDir == EDGE_VER)
  {
    iOffset = 1;
    iSrcStep = iStride;
    piTmpSrc += iEdge*uiPelsInPart;
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset = iStride;
    iSrcStep = 1;
    piTmpSrc += iEdge*uiPelsInPart*iStride;
  }

  for ( Int iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {

    uiBs = 0;
    for (Int iIdxInside = 0; iIdxInside<PartIdxIncr; iIdxInside++)
    {
      uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx+iIdxInside);
      if (uiBs < m_aapucBS[iDir][0][uiBsAbsIdx])
      {
        uiBs = m_aapucBS[iDir][0][uiBsAbsIdx];
      }
    }

    Int iBitdepthScale = (1<<g_uiBitIncrement);

    UInt uiTcOffset = (uiBs>2)?4:0;

    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );
    Int iIndexB = Clip3(0, MAX_QP, iQP );

    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;
    Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;

    for (Int iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
    {
      if ( uiBs )
      {
        Int iD = xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
        if (iD < iBeta)
        {
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc );
          }
        }
      }
    }
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
  UInt  uiBlocksInPart = uiPelsInPartChroma / DEBLOCK_SMALLEST_BLOCK ? uiPelsInPartChroma / DEBLOCK_SMALLEST_BLOCK : 1;

  Int   iOffset, iSrcStep;

  const UInt uiLCUWidthInBaseUnits = pcCU->getPic()->getNumPartInWidth();

// Vertical Position
  UInt uiEdgeNumInLCUVert = g_auiZscanToRaster[uiAbsZorderIdx]%uiLCUWidthInBaseUnits + iEdge;
  UInt uiEdgeNumInLCUHor = g_auiZscanToRaster[uiAbsZorderIdx]/uiLCUWidthInBaseUnits + iEdge;

  if ( (uiEdgeNumInLCUVert%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma)) || (uiEdgeNumInLCUHor%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma)) )
    return;

  UInt  uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;

  UInt  PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPartChroma ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPartChroma : 1;
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

  for ( Int iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {
    ucBs = 0;

    for (Int iIdxInside = 0; iIdxInside<PartIdxIncr; iIdxInside++)
    {
      uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx+iIdxInside);
      if (ucBs < m_aapucBS[iDir][0][uiBsAbsIdx])
      {
        ucBs = m_aapucBS[iDir][0][uiBsAbsIdx];
      }
    }

    Int iBitdepthScale = (1<<g_uiBitIncrement);

    UInt uiTcOffset = (ucBs>2)?4:0;

    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );

    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;

    for (Int iBlkIdx = 0; iBlkIdx < uiBlocksInPart; iBlkIdx ++)
    {

      if ( ucBs > 2)
      {
        for ( UInt uiStep = 0; uiStep < DEBLOCK_SMALLEST_BLOCK; uiStep++ )
        {
          xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma+iBlkIdx*DEBLOCK_SMALLEST_BLOCK), iOffset, iTc );
          xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma+iBlkIdx*DEBLOCK_SMALLEST_BLOCK), iOffset, iTc );
        }
      }
    }
  }
}


__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc )
{

  Int d_strong, delta;

  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  Pel m6  = piSrc[ iOffset*2];
  Pel m1  = piSrc[-iOffset*3];
  Pel m7  = piSrc[ iOffset*3];
  Pel m0  = piSrc[-iOffset*4];

  d_strong = abs(m0-m3) + abs(m7-m4);

  if ( (d_strong < (beta>>3)) && (d<(beta>>2)) && ( abs(m3-m4) < ((tc*5+1)>>1)) ) //strong filtering
  {

    piSrc[-iOffset] = Clip(( m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3 );
    piSrc[0] = Clip(( m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3 );

    piSrc[-iOffset*2] = Clip(( m1 + m2 + m3 + m4 + 2)>>2);
    piSrc[ iOffset] = Clip(( m3 + m4 + m5 + m6 + 2)>>2);

    piSrc[-iOffset*3] = Clip(( 2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3);
    piSrc[ iOffset*2] = Clip(( m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3);

  }
  else
  {
    /* Weak filter */

    delta = Clip3(-tc, tc, ((13*(m4-m3) + 4*(m5-m2) - 5*(m6-m1)+16)>>5) );

    piSrc[-iOffset] = Clip(m3+delta);
    piSrc[0] = Clip(m4-delta);
    piSrc[-iOffset*2] = Clip(m2+delta/2);
    piSrc[ iOffset] = Clip(m5-delta/2);
  }
}

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
#else

Void TComLoopFilter::xGetBoundaryStrength( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiHNumParts      = uiCurNumParts>>1;
  UInt uiQNumParts      = uiCurNumParts>>2;

  UInt uiPartP, uiPartQ;
  UInt uiBsIdx; //index to store Bs for each transform boundary

  TComSlice* pcSlice = pcCU->getSlice();
  TComDataCU* pcCUQ = pcCU;
  TComDataCU* pcCUP;

  m_stLFCUParam.iBsEdgeSum[iDir][iEdge] = 0;

  for ( Int iIdx = 0; iIdx < 4; iIdx++ )
  {
    //-- Calculate Block Index
    if (iDir == EDGE_VER)
    {
      uiBsIdx = iEdge + (iIdx*4);

      uiPartQ = uiAbsZorderIdx;
      if (iEdge >> 1)
        uiPartQ += uiQNumParts;
      if (iIdx >> 1)
        uiPartQ += uiHNumParts;

      if (iEdge %2)
      {
        uiPartP = uiPartQ;
        pcCUP = pcCUQ;
      }
      else
      {
        pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ);
      }
    }
    else  // (iDir == EDGE_HOR)
    {
      uiBsIdx = iIdx + (iEdge*4);

      uiPartQ = uiAbsZorderIdx;
      if (iIdx >> 1)
        uiPartQ += uiQNumParts;
      if (iEdge >> 1)
        uiPartQ += uiHNumParts;

      if (iEdge %2)
      {
        uiPartP = uiPartQ;
        pcCUP = pcCUQ;
      }
      else
      {
        pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ);
      }
    }

    //-- Set BS for Intra MB : BS = 4 or 3
    if ( pcCUP->isIntra(uiPartP) || pcCUQ->isIntra(uiPartQ) )
    {
      m_aaucBS[iDir][uiBsIdx] = (iEdge == 0) ? 4 : 3;   // Intra MB && MB boundary
    }

    //-- Set BS for not Intra MB : BS = 2 or 1 or 0
    if ( !pcCUP->isIntra(uiPartP) && !pcCUQ->isIntra(uiPartQ) )
    {
      if ( pcCUQ->getCbf( uiPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(uiPartQ)) != 0 || pcCUP->getCbf( uiPartP, TEXT_LUMA, pcCUP->getTransformIdx(uiPartP) ) != 0)
      {
        m_aaucBS[iDir][uiBsIdx] = 2;
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
            m_aaucBS[iDir][uiBsIdx] = 0;

            if ( piRefP0 != piRefP1 )   // Different L0 & L1
            {
              if ( piRefP0 == piRefQ0 )
              {
                pcMvP0 -= pcMvQ0;   pcMvP1 -= pcMvQ1;
                m_aaucBS[iDir][uiBsIdx] = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
                                        (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
              }
              else
              {
                pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
                m_aaucBS[iDir][uiBsIdx] = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
                                        (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
              }
            }
            else    // Same L0 & L1
            {
              TComMv pcMvSub0 = pcMvP0 - pcMvQ0;
              TComMv pcMvSub1 = pcMvP1 - pcMvQ1;
              pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
              m_aaucBS[iDir][uiBsIdx] = ( (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
                                        (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4) ) &&
                                      ( (pcMvSub0.getAbsHor() >= 4) | (pcMvSub0.getAbsVer() >= 4) |
                                        (pcMvSub1.getAbsHor() >= 4) | (pcMvSub1.getAbsVer() >= 4) );
            }
          }
          else // for all different Ref_Idx
          {
            m_aaucBS[iDir][uiBsIdx] = 1;
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
          m_aaucBS[iDir][uiBsIdx] = (piRefP0 != piRefQ0) | (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4);
        }
      }   // enf of "if( one of BCBP == 0 )"
    }   // enf of "if( not Intra )"

    m_stLFCUParam.iBsEdgeSum[iDir][iEdge] += m_aaucBS[iDir][uiBsIdx];
  }   // end of "for( iIdx<4 )"
}

Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel*        piSrc       = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );

  Int   iStride   = pcPicYuvRec->getStride();
  Int   iQWidth   = pcCU->getWidth(uiAbsZorderIdx)>>2;
  Int   iQHeight  = pcCU->getHeight(uiAbsZorderIdx)>>2;
  Int   iQP = pcCU->getQP( uiAbsZorderIdx );
  UInt  uiBsIdx;
  Int   iOffset, iSrcStep;
  UInt  uiNumStep;

  for ( Int iIdx = 0; iIdx < 4; iIdx++ )
  {
    Pel* piTmpSrc = piSrc;

    if (iDir == EDGE_VER)
    {
      uiBsIdx = iEdge + (iIdx*4);
      iOffset = 1;
      iSrcStep = iStride;
      piTmpSrc += iEdge*iQWidth + iIdx*iQHeight*iStride;
      uiNumStep = iQHeight;
    }
    else  // (iDir == EDGE_HOR)
    {
      uiBsIdx = iIdx + (iEdge*4);
      iOffset = iStride;
      iSrcStep = 1;
      piTmpSrc += iIdx*iQWidth + iEdge*iQHeight*iStride;
      uiNumStep = iQWidth;
    }

    UChar ucBs = m_aaucBS[iDir][uiBsIdx];

    if ( ucBs )
    {
      for ( UInt uiStep = 0; uiStep < uiNumStep; uiStep++ )
      {
        xPelFilterLuma( piTmpSrc+iSrcStep*uiStep, iOffset, ucBs, iQP );
      }
    }
  }  // end of for (PelNum)
}

Void TComLoopFilter::xEdgeFilterChroma( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );

  Int   iQWidth   = pcCU->getWidth(uiAbsZorderIdx)>>3;
  Int   iQHeight  = pcCU->getHeight(uiAbsZorderIdx)>>3;
  Int   iQStride  = iQHeight*iStride;
  Int   iQP       = QpUV((Int) pcCU->getQP( uiAbsZorderIdx ));

  UInt  uiNumStep;
  UInt  uiBsIdx;
  Int   iOffset, iSrcStep;

  if ( iEdge % 2 != 0 )
    return;

  for ( Int iIdx = 0; iIdx < 4; iIdx++ )
  {
    Pel* piTmpSrcCb = piSrcCb;
    Pel* piTmpSrcCr = piSrcCr;

    //-- Calculate Block Index
    if (iDir == EDGE_VER)
    {
      uiBsIdx  = iEdge + (iIdx*4);
      iOffset   = 1;
      iSrcStep  = iStride;
      piTmpSrcCb += iEdge*iQWidth + iIdx*iQStride;
      piTmpSrcCr += iEdge*iQWidth + iIdx*iQStride;
      uiNumStep = iQHeight;
    }
    else  // (iDir == EDGE_HOR)
    {
      uiBsIdx  = iIdx + (iEdge*4);
      iOffset   = iStride;
      iSrcStep  = 1;
      piTmpSrcCb += iIdx*iQWidth + iEdge*iQStride;
      piTmpSrcCr += iIdx*iQWidth + iEdge*iQStride;
      uiNumStep = iQWidth;
    }

    UChar ucBs = m_aaucBS[iDir][uiBsIdx];

    if ( ucBs )
    {
      for ( UInt uiStep = 0; uiStep < uiNumStep; uiStep++ )
      {
        xPelFilterChroma( piTmpSrcCb + iSrcStep*uiStep, iOffset, ucBs, iQP );
        xPelFilterChroma( piTmpSrcCr + iSrcStep*uiStep, iOffset, ucBs, iQP );
      }
    }
  }  // end of for (iBlkIdx)
}

#endif

#if !TENTM_DEBLOCKING_FILTER
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, UChar ucBs, Int iQP )
{
  Int iBitdepthScale = (1<<g_uiBitIncrement);

  Int iIndexA = Clip3(0, MAX_QP, iQP + m_iAlphaOffset);
  Int iIndexB = Clip3(0, MAX_QP, iQP + m_iBetaOffset);

  UInt    uiAlpha = ALPHA_TABLE[iIndexA] * iBitdepthScale;
  UInt    uiBeta  =  BETA_TABLE[iIndexB] * iBitdepthScale;
  const UChar* pucClipTab = CLIP_TAB[iIndexA];

  Pel Q0  = piSrc[0];
  Pel P0  = piSrc[-iOffset];
  Pel Q1  = piSrc[ iOffset];
  Pel P1  = piSrc[-iOffset*2];
  Pel Q2  = piSrc[ iOffset*2];
  Pel P2  = piSrc[-iOffset*3];
  Pel Q3  = piSrc[ iOffset*3];
  Pel P3  = piSrc[-iOffset*4];

  Int aQ = 0;
  Int aP = 0;

  UInt uiAbsDelta = abs(Q0 - P0);

  if ( uiAbsDelta < uiAlpha )
  {
    Int C0 = pucClipTab[ucBs] * iBitdepthScale;
    if ( ((UInt)abs(Q0 - Q1) < uiBeta) && ((UInt)abs(P0 - P1) < uiBeta ) )   // filterSampleFlag
    {
      aQ = ( (UInt)abs(Q2 - Q0) < uiBeta ) ? 1 : 0;
      aP = ( (UInt)abs(P2 - P0) < uiBeta ) ? 1 : 0;

      if (ucBs == 4)    // for Intra strong filtering
      {
        UInt uiSmallGap = (uiAbsDelta < ((uiAlpha>>2) + 2));
        aQ &= uiSmallGap;
        aP &= uiSmallGap;
        piSrc[0]        = aQ ? ( P1 + ((Q1+ Q0 + P0) << 1) + Q2 + 4) >> 3 : ((Q1<<1) + Q0 + P1 + 2) >> 2;
        piSrc[-iOffset] = aP ? ( Q1 + ((P1+ P0 + Q0) << 1) + P2 + 4) >> 3 : ((P1<<1) + P0 + Q1 + 2) >> 2;

        piSrc[ iOffset  ] = aQ ? ( Q2 + Q1 + Q0 + P0 + 2) >> 2 : Q1;
        piSrc[-iOffset*2] = aP ? ( P2 + P1 + P0 + Q0 + 2) >> 2 : P1;

        piSrc[ iOffset*2] = aQ ? (((Q3 + Q2) << 1) + Q2 + Q1 + Q0 + P0 + 4) >> 3 : Q2;
        piSrc[-iOffset*3] = aP ? (((P3 + P2) << 1) + P2 + P1 + P0 + Q0 + 4) >> 3 : P2;
      }
      else    // normal filtering ( BS < 4)
      {
        Short tC     = (C0 + aP + aQ);
        Pel iDif  = Clip3(-tC, tC, (( ((Q0-P0) << 2) + (P1-Q1) + 4 ) >> 3) );
        piSrc[0]        = Clip( Q0 - iDif );
        piSrc[-iOffset] = Clip( P0 + iDif );

        if (aP)
          piSrc[-iOffset*2] += Clip3(-C0, C0, ( (P2 + ((P0 + Q0 + 1) >> 1) - ( P1 << 1) ) >> 1) );
        if (aQ)
          piSrc[ iOffset  ] += Clip3(-C0, C0, ( (Q2 + ((P0 + Q0 + 1) >> 1) - ( Q1 << 1) ) >> 1) );
      }
    }  // end of if ( filterSampleFlag )
  } // end of if ( uiAbsDelta < uiAlpha )
}

__inline Void TComLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset, UChar ucBs, Int iQP )
{
  Int iBitdepthScale = (1<<g_uiBitIncrement);

  Int iIndexA = Clip3(0, MAX_QP, iQP + m_iAlphaOffset);
  Int iIndexB = Clip3(0, MAX_QP, iQP + m_iBetaOffset);

  UInt    uiAlpha = ALPHA_TABLE[iIndexA] * iBitdepthScale;
  UInt    uiBeta  =  BETA_TABLE[iIndexB] * iBitdepthScale;
  const UChar* pucClipTab = CLIP_TAB[iIndexA];

  Pel Q0  = piSrc[0];
  Pel P0  = piSrc[-iOffset];
  Pel Q1  = piSrc[ iOffset];
  Pel P1  = piSrc[-iOffset*2];

  UInt uiAbsDelta = abs(Q0 - P0);

  if ( uiAbsDelta < uiAlpha )
  {
    Int C0 = pucClipTab[ucBs] * iBitdepthScale;
    if ( ((UInt)abs(Q0 - Q1) < uiBeta) && ((UInt)abs(P0 - P1) < uiBeta ) )  // filterSampleFlag
    {
      if (ucBs == 4)    // for Intra strong filtering
      {
        piSrc[0]        = ((Q1 <<1) + Q0 + P1 + 2) >> 2;
        piSrc[-iOffset] = ((P1 <<1) + P0 + Q1 + 2) >> 2;
      }
      else    // normal filtering ( BS < 4)
      {
        Short tC     = (C0 + 1);
        Pel iDelta  = Clip3(-tC, tC, (( ((Q0-P0) << 2) + (P1-Q1) + 4 ) >> 3) );
        piSrc[0]        = Clip( Q0 - iDelta );
        piSrc[-iOffset] = Clip( P0 + iDelta );
      }
    }  // end of if ( filterSampleFlag )
  } // end of if ( uiAbsDelta < uiAlpha )
}
#endif
