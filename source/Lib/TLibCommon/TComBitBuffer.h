/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, FRAUNHOFER HHI
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of FRAUNHOFER HHI
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

/** \file     TComBitBuffer.h
    \brief    bit/bin buffer class (header)
*/

#ifndef __TCOM_BIT_BUFFER__
#define __TCOM_BIT_BUFFER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDef.h"
#include <assert.h>
#include <string.h>


#define STD_BIT_BUFFER_ALLOC_SIZE  16384


class TComBitBuffer
{
public:
  TComBitBuffer ();
  ~TComBitBuffer();

  __inline Void reset()
  {
    m_uiWriteElemIdx    = 0;
    m_uiWriteAvailBits  = 32;
    m_uiReadElemIdx     = 0;
    m_uiReadAvailBits   = 32;
    m_puiBuffer[ 0 ]    = 0;
  }

  __inline Void insertBit( UInt uiBit )
  {
    if( m_uiWriteAvailBits > 1 )
    {
      m_uiWriteAvailBits--;
      m_puiBuffer[ m_uiWriteElemIdx ] |= ( uiBit << m_uiWriteAvailBits );
    }
    else
    {
      m_puiBuffer[ m_uiWriteElemIdx ] |= uiBit;
      m_uiWriteElemIdx++;
      m_uiWriteAvailBits = 32;
      if( m_uiWriteElemIdx == m_uiBufferSize )
      {
        xIncreaseBufferSize();
      }
      m_puiBuffer[ m_uiWriteElemIdx ]  = 0;
    }
  }

  __inline Void insertBits( UInt uiVal, UInt uiNumBits )
  {
    if( m_uiWriteAvailBits > uiNumBits )
    {
      m_uiWriteAvailBits -= uiNumBits;
      m_puiBuffer[ m_uiWriteElemIdx ] |= ( uiVal << m_uiWriteAvailBits );
    }
    else
    {
      UInt uiRemBits = uiNumBits - m_uiWriteAvailBits;
      m_puiBuffer[ m_uiWriteElemIdx ] |= ( uiVal >> uiRemBits );
      m_uiWriteAvailBits = 32;
      m_uiWriteElemIdx++;
      if( m_uiWriteElemIdx == m_uiBufferSize )
      {
        xIncreaseBufferSize();
      }
      m_puiBuffer[ m_uiWriteElemIdx ]  = 0;
      if( uiRemBits )
      {
        uiVal &= ( 1U << uiRemBits ) - 1;
        m_uiWriteAvailBits -= uiRemBits;
        m_puiBuffer[ m_uiWriteElemIdx ] |= ( uiVal << m_uiWriteAvailBits );
      }
    }
  }

  __inline UInt getWrittenBits()
  {
    return ( m_uiWriteElemIdx << 5 ) + 32 - m_uiWriteAvailBits;
  }

  __inline UInt getReadBits()
  {
    return ( m_uiReadElemIdx << 5 ) + 32 - m_uiReadAvailBits;
  }

  __inline UInt numAvailableBits()
  {
    return ( ( m_uiWriteElemIdx - m_uiReadElemIdx ) << 5 ) + ( m_uiReadAvailBits - m_uiWriteAvailBits ); 
  }
  
  __inline UInt removeBit()
  {
    assert( numAvailableBits() >= 1 );
    if( m_uiReadAvailBits > 1 )
    {
      m_uiReadAvailBits--;
      return ( ( m_puiBuffer[ m_uiReadElemIdx ] >> m_uiReadAvailBits ) & 1 );
    }
    else
    {
      m_uiReadAvailBits = 32;
      return ( m_puiBuffer[ m_uiReadElemIdx++ ] & 1 );
    }
  }

  __inline UInt removeBits( UInt uiNumBits )
  {
    assert( numAvailableBits() >= uiNumBits );
    if( m_uiReadAvailBits > uiNumBits )
    {
      m_uiReadAvailBits -= uiNumBits;
      return ( ( m_puiBuffer[ m_uiReadElemIdx ] >> m_uiReadAvailBits ) & ( ( 1U << uiNumBits ) - 1 ) );
    }
    else
    {
      UInt  uiRemBits   = uiNumBits - m_uiReadAvailBits;
      UInt  uiVal       = ( m_uiReadAvailBits < 32 ? ( m_puiBuffer[ m_uiReadElemIdx ] & ( ( 1U << m_uiReadAvailBits ) - 1 ) ) : m_puiBuffer[ m_uiReadElemIdx ] );
      m_uiReadAvailBits = 32;
      m_uiReadElemIdx++;
      if( uiRemBits )
      {
        m_uiReadAvailBits -= uiNumBits;
        uiVal <<= uiRemBits;
        uiVal  |= ( ( m_puiBuffer[ m_uiReadElemIdx ] >> m_uiReadAvailBits ) & ( ( 1U << uiRemBits ) - 1 ) );
      }
      return uiVal;
    }
  }

private:
  Void  xIncreaseBufferSize();

private:
  UInt* m_puiBuffer;
  UInt  m_uiBufferSize;
  UInt  m_uiWriteElemIdx;
  UInt  m_uiWriteAvailBits;
  UInt  m_uiReadElemIdx;
  UInt  m_uiReadAvailBits;
};


#endif

