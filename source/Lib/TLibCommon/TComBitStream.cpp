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

/** \file     TComBitStream.cpp
    \brief    class for handling bitstream
*/

#include <stdint.h>
#include <vector>
#include "TComBitStream.h"
#include <string.h>
#include <memory.h>

using namespace std;

static unsigned int xSwap ( unsigned int ui )
{
  // heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
  return ui;
#else
  UInt ul2;

  ul2  = ui>>24;
  ul2 |= (ui>>8) & 0x0000ff00;
  ul2 |= (ui<<8) & 0x00ff0000;
  ul2 |= ui<<24;

  return ul2;
#endif
}

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComOutputBitstream::TComOutputBitstream()
{
  m_fifo = new vector<uint8_t>;
  clear();
}

TComOutputBitstream::~TComOutputBitstream()
{
  delete m_fifo;
}

TComInputBitstream::TComInputBitstream(vector<uint8_t>* buf)
{
  m_fifo = buf;
  m_fifo_idx = 0;
  m_uiBitsLeft = 0;

  m_iValidBits      = 32;

  m_ulCurrentBits   = 0;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

char* TComOutputBitstream::getByteStream() const
{
  return (char*) &m_fifo->front();
}

unsigned int TComOutputBitstream::getByteStreamLength()
{
  return m_fifo->size();
}

void TComOutputBitstream::clear()
{
  m_fifo->clear();
  m_held_bits = 0;
  m_num_held_bits = 0;
  m_uiBitsWritten = 0;
}

Void TComOutputBitstream::write   ( UInt uiBits, UInt uiNumberOfBits )
{
  assert( uiNumberOfBits <= 32 );
  m_uiBitsWritten += uiNumberOfBits;

  /* any modulo 8 remainder of num_total_bits cannot be written this time,
   * and will be held until next time. */
  unsigned num_total_bits = uiNumberOfBits + m_num_held_bits;
  unsigned next_num_held_bits = num_total_bits % 8;

  /* form a byte aligned word (write_bits), by concatenating any held bits
   * with the new bits, discarding the bits that will form the next_held_bits.
   * eg: H = held bits, V = n new bits        /---- next_held_bits
   * len(H)=7, len(V)=1: ... ---- HHHH HHHV . 0000 0000, next_num_held_bits=0
   * len(H)=7, len(V)=2: ... ---- HHHH HHHV . V000 0000, next_num_held_bits=1
   * if total_bits < 8, the value of v_ is not used */
  unsigned char next_held_bits = uiBits << (8 - next_num_held_bits);

  if (!(num_total_bits >> 3)) {
    /* insufficient bits accumulated to write out, append new_held_bits to
     * current held_bits */
    /* NB, this requires that v only contains 0 in bit positions {31..n} */
    m_held_bits |= next_held_bits;
    m_num_held_bits = next_num_held_bits;
    return;
  }

  /* topword serves to justify held_bits to align with the msb of uiBits */
  unsigned topword = (uiNumberOfBits - next_num_held_bits) & ~((1 << 3) -1);
  unsigned int write_bits = (m_held_bits << topword) | (uiBits >> next_num_held_bits);

  switch (num_total_bits >> 3) {
  case 4: m_fifo->push_back(write_bits >> 24);
  case 3: m_fifo->push_back(write_bits >> 16);
  case 2: m_fifo->push_back(write_bits >> 8);
  case 1: m_fifo->push_back(write_bits);
  }

  m_held_bits = next_held_bits;
  m_num_held_bits = next_num_held_bits;
}

Void TComOutputBitstream::writeAlignOne()
{
  unsigned int num_bits = getNumBitsUntilByteAligned();
  write((1 << num_bits) - 1, num_bits);
  return;
}

Void TComOutputBitstream::writeAlignZero()
{
  if (0 == m_num_held_bits)
    return;
  m_fifo->push_back(m_held_bits);
  m_uiBitsWritten += getNumBitsUntilByteAligned();
  m_held_bits = 0;
  m_num_held_bits = 0;
}

char* TComInputBitstream::getBuffer()
{
  return (char*) &m_fifo->front();
}

Void TComInputBitstream::initParsing ( UInt uiNumBytes )
{
  m_ulCurrentBits     = 0xdeaddead;
  m_uiNextBits        = 0xdeaddead;
  m_iValidBits        = 0;
  
  m_fifo_idx = 0;
  m_uiBitsLeft        = uiNumBytes << 3;
  
  m_iValidBits        = -32;
  
  xReadNextWord();
  xReadNextWord();
}

#if LCEC_INTRA_MODE || QC_LCEC_INTER_MODE
Void TComInputBitstream::pseudoRead ( UInt uiNumberOfBits, UInt& ruiBits )
{
  UInt ui_right_shift;

  // check the number_of_bits parameter matches the range
  assert( uiNumberOfBits <= 32 );

  if( uiNumberOfBits > m_uiBitsLeft )
  {
    //assert (0);
  }

  Int  iValidBits = m_iValidBits - uiNumberOfBits;
  UInt ulCurrentBits=m_ulCurrentBits;
  UInt uiNextBits= m_uiNextBits;
  if( 0 <= iValidBits )
  {
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = 32 - uiNumberOfBits ;
    // mask out the value
    ruiBits  = ulCurrentBits >> ui_right_shift;
    //prepare for next access
    ulCurrentBits = ulCurrentBits << uiNumberOfBits;
  }
  else
  {
    // mask out the value in the current word
    ruiBits = ulCurrentBits;
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = iValidBits + uiNumberOfBits ;
    // mask out the value in the next word
    ruiBits |= uiNextBits >> ui_right_shift;
    ruiBits >>= 32 - uiNumberOfBits;
    uiNextBits <<=  -iValidBits;
  }

  // check the current word for being empty
  //-- baekeun.lee@samsung.com
  if ( 0 < m_iValidBits)
  {
    return ;
  }
}
#endif


Void TComInputBitstream::read (UInt uiNumberOfBits, UInt& ruiBits)
{
  UInt ui_right_shift;
  
  // check the number_of_bits parameter matches the range
  assert( uiNumberOfBits <= 32 );
  
  if( uiNumberOfBits > m_uiBitsLeft )
  {
    assert (0);
  }
  
  m_uiBitsLeft -= uiNumberOfBits;
  m_iValidBits -= uiNumberOfBits;
  
  if( 0 <= m_iValidBits )
  {
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = 32 - uiNumberOfBits ;
    
    // mask out the value
    ruiBits  = m_ulCurrentBits >> ui_right_shift;
    
    //prepare for next access
    m_ulCurrentBits = m_ulCurrentBits << uiNumberOfBits;
  }
  else
  {
    // mask out the value in the current word
    ruiBits = m_ulCurrentBits;
    
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = m_iValidBits + uiNumberOfBits ;
    
    // mask out the value in the next word
    ruiBits |= m_uiNextBits >> ui_right_shift;
    
    ruiBits >>= 32 - uiNumberOfBits;
    
    m_uiNextBits <<=  -m_iValidBits;
  }
  
  // check the current word for being empty
  //-- baekeun.lee@samsung.com
  if ( 0 < m_iValidBits)
  {
    return ;
  }
  xReadNextWord();
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

__inline Void TComInputBitstream::xReadNextWord()
{
  m_ulCurrentBits = m_uiNextBits;
  m_iValidBits += 32;
  
  // chech if there are bytes left in the packet
  if( m_fifo_idx + 4 <= m_fifo->size() )
  {
    // read 32 bit from the packet
    m_uiNextBits = (*m_fifo)[m_fifo_idx+0] << 24
                 | (*m_fifo)[m_fifo_idx+1] << 16
                 | (*m_fifo)[m_fifo_idx+2] << 8
                 | (*m_fifo)[m_fifo_idx+3];
    m_fifo_idx += 4;
  }
  else
  {
    Int iBytesLeft  = ((Int)m_uiBitsLeft - m_iValidBits+7) >> 3;
    m_uiNextBits  = 0;
    
    if( iBytesLeft > 0)
    {
      for( Int iByte = 0; iByte < iBytesLeft; iByte++ )
      {
        m_uiNextBits <<= 8;
        m_uiNextBits += (*m_fifo)[m_fifo_idx++];
      }
      m_uiNextBits <<= (4-iBytesLeft)<<3;
    }
  }
}

Void TComInputBitstream::initParsingConvertPayloadToRBSP( const UInt uiBytesRead )
{
  UInt uiZeroCount    = 0;
  UInt uiReadOffset   = 0;
  UInt uiWriteOffset  = 0;
  const char* pucRead = getBuffer();
  char* pucWrite      = getBuffer();
  
  for( ; uiReadOffset < uiBytesRead; uiReadOffset++ )
  {
    if( 2 == uiZeroCount && 0x03 == pucRead[uiReadOffset] )
    {
      uiReadOffset++;
      uiZeroCount = 0;
      if (uiReadOffset>=uiBytesRead)
      {
        break;
      }
    }
    
    pucWrite[uiWriteOffset++] = pucRead[uiReadOffset];
    
    if( 0x00 == pucRead[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }
  
  // th just clear the remaining bits in the buffer
  for( UInt ui = uiWriteOffset; ui < uiBytesRead; ui++)
  {
    pucWrite[ui] = 0;
  }
  
  m_fifo->resize(uiWriteOffset);
  initParsing( uiWriteOffset );
}

/**
 * insert the contents of the bytealigned (and flushed) bitstream @src
 * into @this at byte position @pos.
 */
void TComOutputBitstream::insertAt(const TComOutputBitstream& src, unsigned pos)
{
  unsigned src_bits = src.getNumberOfWrittenBits();
  assert(0 == src_bits % 8);

  vector<uint8_t>::iterator at = this->m_fifo->begin() + pos;
  this->m_fifo->insert(at, src.m_fifo->begin(), src.m_fifo->end());

  /* update state */
  this->m_uiBitsWritten += src_bits;
}
