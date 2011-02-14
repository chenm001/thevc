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

/** \file     TComMotionInfo.cpp
    \brief    motion information handling classes
*/

#include <memory.h>
#include "TComMotionInfo.h"
#include "assert.h"
#include <stdlib.h>

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Create / destroy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::create( UInt uiNumPartition )
{
  m_pcMv     = ( TComMv* )xMalloc( TComMv, uiNumPartition );
  m_pcMvd    = ( TComMv* )xMalloc( TComMv, uiNumPartition );
  m_piRefIdx = (    Int* )xMalloc( Int,    uiNumPartition );
  
  m_uiNumPartition = uiNumPartition;
}

Void TComCUMvField::destroy()
{
  if( m_pcMv )
  {
    xFree( m_pcMv );     m_pcMv     = NULL;
  }
  if( m_pcMvd )
  {
    xFree( m_pcMvd );    m_pcMvd    = NULL;
  }
  if( m_piRefIdx )
  {
    xFree( m_piRefIdx ); m_piRefIdx = NULL;
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Clear / copy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::clearMv( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcMv[ i ].setZero();
  }
}

Void TComCUMvField::clearMvd( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcMvd[ i ].setZero();
  }
}

Void TComCUMvField::clearMvField()
{
  for ( Int i = m_uiNumPartition - 1; i >= 0; i-- )
  {
    m_pcMv    [ i ].setZero();
    m_pcMvd   [ i ].setZero();
    m_piRefIdx[ i ] = NOT_VALID;
  }
}

Void TComCUMvField::copyFrom( TComCUMvField* pcCUMvFieldSrc, Int iNumPartSrc, Int iPartAddrDst )
{
  Int iSizeInTComMv = sizeof( TComMv ) * iNumPartSrc;
  
  memcpy( m_pcMv     + iPartAddrDst, pcCUMvFieldSrc->getMv(),     iSizeInTComMv );
  memcpy( m_pcMvd    + iPartAddrDst, pcCUMvFieldSrc->getMvd(),    iSizeInTComMv );
  memcpy( m_piRefIdx + iPartAddrDst, pcCUMvFieldSrc->getRefIdx(), sizeof( Int ) * iNumPartSrc );
}

Void TComCUMvField::copyTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst )
{
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;
  
  memcpy( pcCUMvFieldDst->getMv()     + iPartAddrDst, m_pcMv,     iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getMvd()    + iPartAddrDst, m_pcMvd,    iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getRefIdx() + iPartAddrDst, m_piRefIdx, sizeof( Int ) * m_uiNumPartition );
}

Void TComCUMvField::copyTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst, UInt uiOffset, UInt uiNumPart )
{
  Int iSizeInTComMv = sizeof( TComMv ) * uiNumPart;
  Int iOffset = uiOffset + iPartAddrDst;
  
  memcpy( pcCUMvFieldDst->getMv()     + iOffset, m_pcMv     + uiOffset, iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getMvd()    + iOffset, m_pcMvd    + uiOffset, iSizeInTComMv );
  memcpy( pcCUMvFieldDst->getRefIdx() + iOffset, m_piRefIdx + uiOffset, sizeof( Int ) * uiNumPart );
}

Void TComCUMvField::copyMvTo( TComCUMvField* pcCUMvFieldDst, Int iPartAddrDst )
{
  memcpy( pcCUMvFieldDst->getMv() + iPartAddrDst, m_pcMv, sizeof( TComMv ) * m_uiNumPartition );
}

// --------------------------------------------------------------------------------------------------------------------
// Set
// --------------------------------------------------------------------------------------------------------------------

Void TComCUMvField::setAllMv( TComMv& rcMv, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComMv* pcMv = m_pcMv + iPartAddr;
  register TComMv cMv = rcMv;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  switch( eCUMode )
  {
    case SIZE_2Nx2N:
      for ( i = iNumPartition - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
    case SIZE_2NxN:
      for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
    case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
        pcMv[ i + uiOffset ] = cMv;
      }
      break;
    }
    case SIZE_NxN:
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMv[ i ] = cMv;
      }
      break;
    default:
      assert(0);
      break;
  }
}

Void TComCUMvField::setAllMvd( TComMv& rcMvd, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComMv* pcMvd = m_pcMvd + iPartAddr;
  register TComMv cMvd = rcMvd;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  switch( eCUMode )
  {
    case SIZE_2Nx2N:
      for ( i = iNumPartition - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
    case SIZE_2NxN:
      for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
    case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
        pcMvd[ i + uiOffset ] = cMvd;
      }
      break;
    }
    case SIZE_NxN:
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcMvd[ i ] = cMvd;
      }
      break;
    default:
      assert(0);
      break;
  }
}

Void TComCUMvField::setAllRefIdx ( Int iRefIdx, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  Int* piRefIdx = m_piRefIdx + iPartAddr;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);
  
  switch( eCUMode )
  {
    case SIZE_2Nx2N:
      for ( i = iNumPartition - 1; i >= 0; i-- )
      {
        piRefIdx[ i ] = iRefIdx;
      }
      break;
    case SIZE_2NxN:
      for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
      {
        piRefIdx[ i ] = iRefIdx;
      }
      break;
    case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        piRefIdx[ i ] = iRefIdx;
        piRefIdx[ i + uiOffset ] = iRefIdx;
      }
      break;
    }
    case SIZE_NxN:
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        piRefIdx[ i ] = iRefIdx;
      }
      break;
    default:
      assert(0);
      break;
  }
}

Void TComCUMvField::setAllMvField ( TComMv& rcMv, Int iRefIdx, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  setAllMv( rcMv, eCUMode, iPartAddr, iPartIdx, uiDepth);
  setAllRefIdx(iRefIdx, eCUMode,iPartAddr,iPartIdx,uiDepth);
  return;
}

#if AMVP_BUFFERCOMPRESS
Void TComCUMvField::compress(PredMode* pePredMode)
{
  Int N = AMVP_DECIMATION_FACTOR;
  for ( Int uiPartIdx = 0; uiPartIdx <m_uiNumPartition; uiPartIdx+=(N*N) )
  {
    Int  jj = uiPartIdx+N*N;
    
    TComMv cMv(0,0); 
    if (pePredMode[uiPartIdx]!=MODE_INTRA) cMv = m_pcMv[ uiPartIdx ]; 
    for ( Int i = jj-1; i >= uiPartIdx; i-- )
    {
      m_pcMv[ i ] = cMv;
    }
  }
} 
#endif 
