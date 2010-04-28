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

/** \file			TComLoopFilter.cpp
    \brief		deblocking filter
*/

#include "TComLoopFilter.h"
#include "TComSlice.h"
#include "TComMv.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define		EDGE_VER		0
#define		EDGE_HOR		1
#define   QpUV(iQpY)	( g_aucChromaScale[ Max( Min( (iQpY), MAX_QP ), MIN_QP ) ] )

// ====================================================================================================================
// Tables
// ====================================================================================================================

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

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComLoopFilter::TComLoopFilter()
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
  m_iAlphaOffset                  = iAlphaOffset;
  m_iBetaOffset                   = iBetaOffset;
}

/**
    - call deblocking function for every CU
		.
		\param	pcPic		picture class (TComPic) pointer
 */
Void TComLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;

	// for every CU
	for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

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
  xSetEdgefilter     ( pcCU, uiAbsZorderIdx );

  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
  {
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
}

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
	Pel*				piSrc				= pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );

	Int		iStride		= pcPicYuvRec->getStride();
	Int		iQWidth		= pcCU->getWidth(uiAbsZorderIdx)>>2;
	Int		iQHeight	= pcCU->getHeight(uiAbsZorderIdx)>>2;
	Int		iQP = pcCU->getQP( uiAbsZorderIdx );
	UInt	uiBsIdx;
	Int		iOffset, iSrcStep;
	UInt	uiNumStep;

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

	Int		iQWidth		= pcCU->getWidth(uiAbsZorderIdx)>>3;
	Int		iQHeight	= pcCU->getHeight(uiAbsZorderIdx)>>3;
	Int		iQStride	= iQHeight*iStride;
	Int		iQP				= QpUV((Int) pcCU->getQP( uiAbsZorderIdx ));

	UInt	uiNumStep;
	UInt	uiBsIdx;
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