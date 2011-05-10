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

/** \file     TComBitStream.h
    \brief    class for handling bitstream (header)
*/

#ifndef __COMBITSTREAM__
#define __COMBITSTREAM__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include "CommonDef.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// pure virtual class for basic bit handling
class TComBitIf
{
public:
  virtual Void        writeAlignOne         () {};
#if E057_INTRA_PCM
  virtual Void        writeAlignZero        () {};
#endif
  virtual Void        write                 ( UInt uiBits, UInt uiNumberOfBits )  = 0;
  virtual Void        resetBits             ()                                    = 0;
  virtual UInt getNumberOfWrittenBits() const = 0;
  virtual ~TComBitIf() {}
};

/**
 * Model of a writable bitstream that accumulates bits to produce a
 * bytestream.
 */
class TComOutputBitstream : public TComBitIf
{
  /**
   * FIFO for storage of bytes.  Use:
   *  - fifo.push_back(x) to append words
   *  - fifo.clear() to empty the FIFO
   *  - &fifo.front() to get a pointer to the data array.
   *    NB, this pointer is only valid until the next push_back()/clear()
   */
  std::vector<uint8_t> *m_fifo;

  unsigned int m_num_held_bits; /// number of bits not flushed to bytestream.
  unsigned char m_held_bits; /// the bits held and not flushed to bytestream.
                             /// this value is always msb-aligned, bigendian.

  UInt        m_uiBitsWritten;
  
  UInt        *m_auiSliceByteLocation, m_uiSliceCount;  // used to skip over slice start codes in initParsingConvertPayloadToRBSP()
  UInt        m_uiSliceProcessed;
  
public:
  // create / destroy
  TComOutputBitstream();
  ~TComOutputBitstream();

  Void        create          ( UInt uiSizeInBytes );
  Void        destroy         ();
  
  // interface for encoding
  /**
   * append @uiNumberOfBits@ least significant bits of @uiBits@ to
   * the current bitstream
   */
  Void        write           ( UInt uiBits, UInt uiNumberOfBits );

  /** insert one bits until the bitstream is byte-aligned */
  Void        writeAlignOne   ();

  /** insert zero bits until the bitstream is byte-aligned */
  Void        writeAlignZero  ();

  Void        convertRBSPToPayload( UInt uiStartPos = 0);
  UInt        getSliceProcessed                ()       { return m_uiSliceProcessed;                }
  Void        setSliceProcessed                (UInt u) { m_uiSliceProcessed                = u;    }
  
  // interface for slice start-code positioning at encoder
  UInt        getSliceCount                    ()                            { return m_uiSliceCount;                     }
  UInt        getSliceByteLocation             ( UInt uiIdx )                { return m_auiSliceByteLocation[ uiIdx ];    }
  Void        setSliceCount                    ( UInt uiCount )              { m_uiSliceCount = uiCount;                  }
  Void        setSliceByteLocation             ( UInt uiIdx, UInt uiCount )  { m_auiSliceByteLocation[ uiIdx ] = uiCount; }

  // memory allocation / deallocation interface for "slice location" bookkeeping
  Void        allocateMemoryForSliceLocations       ( UInt uiMaxNumOfSlices );
  Void        freeMemoryAllocatedForSliceLocations  ();

  /** this function should never be called */
  void resetBits() { assert(0); }

  // utility functions

  /**
   * Return a pointer to the start of the byte-stream buffer.
   * Pointer is valid until the next write/flush/reset call.
   * NB, data is arranged such that subsequent bytes in the
   * bytestream are stored in ascending addresses.
   */
  char* getByteStream() const;

  /**
   * Return the number of valid bytes available from  getByteStream()
   */
  unsigned int getByteStreamLength();

  /**
   * Reset all internal state.
   */
  Void clear();

  /**
   * returns the number of bits that need to be written to
   * achieve byte alignment.
   */
  Int getNumBitsUntilByteAligned() { return (8 - m_num_held_bits) & 0x7; }

  /**
   * Return the number of bits that have been written since the
   * last resetBits()/clear() call.  Unless convertRBSPToPayload()
   * has been called more recently, whereby the number of bits
   * is the total number of bits that getByteStreamLength() would return.
   */
  UInt getNumberOfWrittenBits() const { return  m_uiBitsWritten; }

  /**
   * Flush any partially written bytes.  This has the same effect as
   * calling writeAlignZero().
   */
  Void        flushBuffer();

  void insertAt(const TComOutputBitstream& src, unsigned pos);
};

/**
 * Model of an input bitstream that extracts bits from a predefined
 * bytestream.
 */
class TComInputBitstream
{
protected:
  UInt*       m_apulStreamPacketBegin;
  UInt*       m_pulStreamPacket;
  UInt        m_uiBufSize;

  Int         m_iValidBits;

  UInt        m_ulCurrentBits;

  UInt        m_uiDWordsLeft;
  UInt        m_uiBitsLeft;
  UInt        m_uiNextBits;

  // read one word
  __inline Void xReadNextWord ();

public:
  // create / destroy
  Void        create          ( UInt uiSizeInBytes );
  Void        destroy         ();

  // interface for decoding
  Void        initParsingConvertPayloadToRBSP( const UInt uiBytesRead );
  Void        initParsing     ( UInt uiNumBytes );
#if LCEC_INTRA_MODE || QC_LCEC_INTER_MODE
  Void        pseudoRead      ( UInt uiNumberOfBits, UInt& ruiBits );
#endif
  Void        read            ( UInt uiNumberOfBits, UInt& ruiBits );

  // Peek at bits in word-storage. Used in determining if we have completed reading of current bitstream and therefore slice in LCEC.
  UInt        peekBits (UInt uiBits) { return( m_ulCurrentBits >> (32 - uiBits));  }

  // reset internal status
  Void        resetBits       ()
  {
    m_iValidBits = 32;
    m_ulCurrentBits = 0;
  }

  // utility functions
  unsigned read(unsigned numberOfBits) { UInt tmp; read(numberOfBits, tmp); return tmp; }
  UInt* getStartStream() const { return m_apulStreamPacketBegin; }
  UInt*       getBuffer()               { return  m_pulStreamPacket;                    }
  Int         getNumBitsUntilByteAligned() { return m_iValidBits & (0x7); }
  Void        setModeSbac()             { m_uiBitsLeft = 8*((m_uiBitsLeft+7)/8);        } // stop bit + trailing stuffing bits
  Void        rewindStreamPacket()      { m_pulStreamPacket = m_apulStreamPacketBegin;  }
  UInt        getNumBitsLeft()             { return  m_uiBitsLeft;                         }
};

#endif

