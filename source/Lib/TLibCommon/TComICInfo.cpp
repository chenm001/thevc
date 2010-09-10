/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, NTT DOCOMO, INC. and DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of NTT DOCOMO, INC. nor the name of DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
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

/** \file			TComICInfo.cpp
    \brief		IC parameter information handling classes
*/

#include <memory.h>
#include "TComICInfo.h"
#include "assert.h"
#include <stdlib.h>

#ifdef DCM_PBIC
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Create / destroy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUIcField::create( UInt uiNumPartition )
{
  m_pcIc     = ( TComIc* )xMalloc( TComIc, uiNumPartition );
  m_pcIcd    = ( TComIc* )xMalloc( TComIc, uiNumPartition );

  m_uiNumPartition = uiNumPartition;
}

Void TComCUIcField::destroy()
{
  if( m_pcIc )
  {
    xFree( m_pcIc );     m_pcIc     = NULL;
  }
  if( m_pcIcd )
  {
    xFree( m_pcIcd );    m_pcIcd    = NULL;
  }
}

// --------------------------------------------------------------------------------------------------------------------
// Clear / copy
// --------------------------------------------------------------------------------------------------------------------

Void TComCUIcField::clearIc( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);

  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcIc[ i ].reset();
  }
}

Void TComCUIcField::clearIcd( Int iPartAddr, UInt uiDepth )
{
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);

  for ( Int i = iNumPartition - 1; i >= 0; i-- )
  {
    m_pcIcd[ i ].reset();
  }
}

Void TComCUIcField::clearIcField()
{
  for ( Int i = m_uiNumPartition - 1; i >= 0; i-- )
  {
    m_pcIc    [ i ].reset();
    m_pcIcd   [ i ].reset();
  }
}

Void TComCUIcField::copyFrom( TComCUIcField* pcCUIcFieldSrc, Int iNumPartSrc, Int iPartAddrDst )
{
  Int iSizeInTComIc = sizeof( TComIc ) * iNumPartSrc;

  memcpy( m_pcIc     + iPartAddrDst, pcCUIcFieldSrc->getIc(),     iSizeInTComIc );
  memcpy( m_pcIcd    + iPartAddrDst, pcCUIcFieldSrc->getIcd(),    iSizeInTComIc ); 
}

Void TComCUIcField::copyTo( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst )
{
  Int iSizeInTComIc = sizeof( TComIc ) * m_uiNumPartition;

  memcpy( pcCUIcFieldDst->getIc()     + iPartAddrDst, m_pcIc,     iSizeInTComIc );
  memcpy( pcCUIcFieldDst->getIcd()    + iPartAddrDst, m_pcIcd,    iSizeInTComIc );
}

Void TComCUIcField::copyTo( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst, UInt uiOffset, UInt uiNumPart )
{
  Int iSizeInTComIc = sizeof( TComIc ) * uiNumPart;
  Int iOffset = uiOffset + iPartAddrDst;

  memcpy( pcCUIcFieldDst->getIc()     + iOffset, m_pcIc     + uiOffset, iSizeInTComIc );
  memcpy( pcCUIcFieldDst->getIcd()    + iOffset, m_pcIcd    + uiOffset, iSizeInTComIc ); 
}

Void TComCUIcField::copyIcTo( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst )
{
  memcpy( pcCUIcFieldDst->getIc() + iPartAddrDst, m_pcIc, sizeof( TComIc ) * m_uiNumPartition );
}

// --------------------------------------------------------------------------------------------------------------------
// Set
// --------------------------------------------------------------------------------------------------------------------

Void TComCUIcField::setAllIc( TComIc& rcIc, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComIc* pcIc = m_pcIc + iPartAddr;
  register TComIc cIc = rcIc;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);

  switch( eCUMode )
  {
  case SIZE_2Nx2N:
    for ( i = iNumPartition - 1; i >= 0; i-- )
    {
      pcIc[ i ] = cIc;
    }
    break;
  case SIZE_2NxN:
    for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
    {
      pcIc[ i ] = cIc;
    }
    break;
  case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcIc[ i ] = cIc;
        pcIc[ i + uiOffset ] = cIc;
      }
      break;
    }
  case SIZE_NxN:
    for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
    {
      pcIc[ i ] = cIc;
    }
    break;
  case SIZE_2NxnU:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }
      }
      else
      {
        TComIc* pi  = pcIc;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cIc;
        }

        pi = pcIc + iCurrPartNumQ;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cIc;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIc;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cIc;
        }
        pi = pcIc + ( iNumPartition - iCurrPartNumQ );
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cIc;
        }
      }
      else
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + (iCurrPartNumQ<<1);
        TComIc* pi3 = pcIc + (iCurrPartNumQ>>1);
        TComIc* pi4 = pcIc + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);

        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
          pi3[i] = cIc;
          pi4[i] = cIc;
        }
      }
      else
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + (iCurrPartNumQ<<1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }

        pi  = pcIc + (iCurrPartNumQ>>1);
        pi2 = pcIc + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + (iCurrPartNumQ<<1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }

        pi  = pcIc + iCurrPartNumQ + (iCurrPartNumQ>>1);
        pi2 = pcIc + iNumPartition - iCurrPartNumQ + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
        }
      }
      else
      {
        TComIc* pi  = pcIc;
        TComIc* pi2 = pcIc + (iCurrPartNumQ>>1);
        TComIc* pi3 = pcIc + (iCurrPartNumQ<<1);
        TComIc* pi4 = pcIc + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIc;
          pi2[i] = cIc;
          pi3[i] = cIc;
          pi4[i] = cIc;
        }
      }
      break;
    }
  default:
    assert(0);
    break;
  }
}

Void TComCUIcField::setAllIcd( TComIc& rcIcd, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  Int i;
  TComIc* pcIcd = m_pcIcd + iPartAddr;
  register TComIc cIcd = rcIcd;
  Int iNumPartition = m_uiNumPartition >> (uiDepth<<1);

  switch( eCUMode )
  {
  case SIZE_2Nx2N:
    for ( i = iNumPartition - 1; i >= 0; i-- )
    {
      pcIcd[ i ] = cIcd;
    }
    break;
  case SIZE_2NxN:
    for ( i = ( iNumPartition >> 1 ) - 1; i >= 0; i-- )
    {
      pcIcd[ i ] = cIcd;
    }
    break;
  case SIZE_Nx2N:
    {
      UInt uiOffset = iNumPartition >> 1;
      for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
      {
        pcIcd[ i ] = cIcd;
        pcIcd[ i + uiOffset ] = cIcd;
      }
      break;
    }
  case SIZE_NxN:
    for ( i = ( iNumPartition >> 2 ) - 1; i >= 0; i-- )
    {
      pcIcd[ i ] = cIcd;
    }
    break;
  case SIZE_2NxnU:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }
      }
      else
      {
        TComIc* pi  = pcIcd;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cIcd;
        }

        pi = pcIcd + iCurrPartNumQ;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cIcd;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIcd;
        for (i = 0; i < ( (iCurrPartNumQ>>1) + (iCurrPartNumQ<<1) ); i++)
        {
          pi[i] = cIcd;
        }
        pi = pcIcd + ( iNumPartition - iCurrPartNumQ );
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi[i] = cIcd;
        }
      }
      else
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + iCurrPartNumQ;
        for (i = 0; i < (iCurrPartNumQ>>1); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + (iCurrPartNumQ<<1);
        TComIc* pi3 = pcIcd + (iCurrPartNumQ>>1);
        TComIc* pi4 = pcIcd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);

        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
          pi3[i] = cIcd;
          pi4[i] = cIcd;
        }
      }
      else
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + (iCurrPartNumQ<<1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }

        pi  = pcIcd + (iCurrPartNumQ>>1);
        pi2 = pcIcd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      Int iCurrPartNumQ = iNumPartition>>2;
      if( iPartIdx == 0 )
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + (iCurrPartNumQ<<1);
        for (i = 0; i < ( (iCurrPartNumQ>>2) + iCurrPartNumQ ); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }

        pi  = pcIcd + iCurrPartNumQ + (iCurrPartNumQ>>1);
        pi2 = pcIcd + iNumPartition - iCurrPartNumQ + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
        }
      }
      else
      {
        TComIc* pi  = pcIcd;
        TComIc* pi2 = pcIcd + (iCurrPartNumQ>>1);
        TComIc* pi3 = pcIcd + (iCurrPartNumQ<<1);
        TComIc* pi4 = pcIcd + (iCurrPartNumQ<<1) + (iCurrPartNumQ>>1);
        for (i = 0; i < (iCurrPartNumQ>>2); i++)
        {
          pi [i] = cIcd;
          pi2[i] = cIcd;
          pi3[i] = cIcd;
          pi4[i] = cIcd;
        }
      }
      break;
    }
  default:
    assert(0);
    break;
  }
}


Void TComCUIcField::setAllIcField ( TComIc& rcIc, PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth )
{
  setAllIc( rcIc, eCUMode, iPartAddr, iPartIdx, uiDepth);
  return;
}

#endif //#ifdef DCM_PBIC