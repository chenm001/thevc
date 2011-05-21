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

/** \file     TEncBinCoderCABAC.cpp
    \brief    binary entropy encoder of CABAC
*/

#include "TEncBinCoderCABAC.h"


TEncBinCABAC::TEncBinCABAC()
: m_pcTComBitIf( 0 )
, m_bBinCountingEnabled(0)
{
}

TEncBinCABAC::~TEncBinCABAC()
{
}

Void
TEncBinCABAC::init( TComBitIf* pcTComBitIf )
{
  m_pcTComBitIf = pcTComBitIf;
}

Void
TEncBinCABAC::uninit()
{
  m_pcTComBitIf = 0;
}

Void
TEncBinCABAC::start()
{
  m_uiLow           = 0;
  m_uiRange         = 510;
  m_uiBitsToFollow  = 0;
  m_uiByte          = 0;
  m_uiBitsLeft      = 9;
}

Void
TEncBinCABAC::finish()
{
  xWriteBitAndBitsToFollow( ( m_uiLow >> 9 ) & 1 );
  xWriteBit               ( ( m_uiLow >> 8 ) & 1 );
  // xWriteBit               ( 1 ); // stop bit, already written in TEncGOP::compressGOP
  if( 8 - m_uiBitsLeft != 0 )
  {
    m_pcTComBitIf->write  ( m_uiByte, 8 - m_uiBitsLeft );
  }
}

#if E057_INTRA_PCM
/** Reset BAC register and counter values.
 * \returns Void
 */
Void TEncBinCABAC::resetBac()
{
  m_uiLow           = 0;
  m_uiRange         = 510;
  m_uiBitsToFollow  = 0;
  m_uiByte          = 0;
  m_uiBitsLeft      = 9;
}

/** Encode PCM alignment zero bits.
 * \returns Void
 */
Void TEncBinCABAC::encodePCMAlignBits()
{
  xWriteBitAndBitsToFollow( ( m_uiLow >> 9 ) & 1 );
  xWriteBit               ( ( m_uiLow >> 8 ) & 1 );
  xWriteBit               ( 1 ); // stop bit

  if( 8 - m_uiBitsLeft != 0 )
  {
    m_pcTComBitIf->write  ( m_uiByte, 8 - m_uiBitsLeft );
    m_pcTComBitIf->writeAlignZero(); // pcm align zero
    m_uiBitsLeft  = 8;
    m_uiByte      = 0;
  }
}

/** Write a PCM code.
 * \param uiCode code value
 * \param uiLength code bit-depth
 * \returns Void
 */
Void  TEncBinCABAC::xWritePCMCode(UInt uiCode, UInt uiLength)
{
  m_pcTComBitIf->write  (uiCode, uiLength);
}
#endif

Void
TEncBinCABAC::copyState( TEncBinIf* pcTEncBinIf )
{
  TEncBinCABAC* pcTEncBinCABAC = pcTEncBinIf->getTEncBinCABAC();
  m_uiLow           = pcTEncBinCABAC->m_uiLow;
  m_uiRange         = pcTEncBinCABAC->m_uiRange;
  m_uiBitsToFollow  = pcTEncBinCABAC->m_uiBitsToFollow;
  m_uiByte          = pcTEncBinCABAC->m_uiByte;
  m_uiBitsLeft      = pcTEncBinCABAC->m_uiBitsLeft;
}

Void  
TEncBinCABAC::resetBits()
{
  m_uiLow          &= 255;
  m_uiBitsToFollow  = 0;
  m_uiByte          = 0;
  m_uiBitsLeft      = 9;
}

UInt
TEncBinCABAC::getNumWrittenBits()
{
  return m_pcTComBitIf->getNumberOfWrittenBits() + 8 + m_uiBitsToFollow - m_uiBitsLeft + 1;
}

Void
TEncBinCABAC::encodeBin( UInt uiBin, ContextModel &rcCtxModel )
{
  {
    DTRACE_CABAC_V( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tstate=" )
    DTRACE_CABAC_V( ( rcCtxModel.getState() << 1 ) + rcCtxModel.getMps() )
    DTRACE_CABAC_T( "\tsymbol=" )
    DTRACE_CABAC_V( uiBin )
    DTRACE_CABAC_T( "\n" )
  }
  if (m_bBinCountingEnabled) 
  {
    m_uiBinsCoded++;
  }
  UInt  uiLPS   = TComCABACTables::sm_aucLPSTable[ rcCtxModel.getState() ][ ( m_uiRange >> 6 ) & 3 ];
  m_uiRange    -= uiLPS;
  if( uiBin != rcCtxModel.getMps() )
  {
    m_uiLow    += m_uiRange;
    m_uiRange   = uiLPS;
    rcCtxModel.updateLPS();
  }
  else
  {
    rcCtxModel.updateMPS();
  }
  while( m_uiRange < 256 )
  {
    if( m_uiLow >= 512 )
    {
      xWriteBitAndBitsToFollow( 1 );
      m_uiLow -= 512;
    }
    else if( m_uiLow < 256 )
    {
      xWriteBitAndBitsToFollow( 0 );
    }
    else
    {
      m_uiBitsToFollow++;
      m_uiLow         -= 256;
    }
    m_uiLow   <<= 1;
    m_uiRange <<= 1;
  }
}

Void
TEncBinCABAC::encodeBinEP( UInt uiBin )
{
  {
    DTRACE_CABAC_V( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tEPsymbol=" )
    DTRACE_CABAC_V( uiBin )
    DTRACE_CABAC_T( "\n" )
  }
  if (m_bBinCountingEnabled)
  {
    m_uiBinsCoded++;
  }
  m_uiLow <<= 1;
  if( uiBin )
  {
    m_uiLow += m_uiRange;
  }
  if( m_uiLow >= 1024 )
  {
    xWriteBitAndBitsToFollow( 1 );
    m_uiLow -= 1024;
  }
  else if( m_uiLow < 512 )
  {
    xWriteBitAndBitsToFollow( 0 );
  }
  else
  {
    m_uiBitsToFollow++;
    m_uiLow         -= 512;
  }
}

Void
TEncBinCABAC::encodeBinTrm( UInt uiBin )
{
  if (m_bBinCountingEnabled)
  {
    m_uiBinsCoded++;
  }
  m_uiRange -= 2;
  if( uiBin )
  {
    m_uiLow  += m_uiRange;
    m_uiRange = 2;
  }
  while( m_uiRange < 256 )
  {
    if( m_uiLow >= 512 )
    {
      xWriteBitAndBitsToFollow( 1 );
      m_uiLow -= 512;
    }
    else if( m_uiLow < 256 )
    {
      xWriteBitAndBitsToFollow( 0 );
    }
    else
    {
      m_uiBitsToFollow++;
      m_uiLow         -= 256;
    }
    m_uiLow   <<= 1;
    m_uiRange <<= 1;
  }
}

Void  
TEncBinCABAC::xWriteBit( UInt uiBit )
{
  m_uiByte += m_uiByte + uiBit;
  if( ! --m_uiBitsLeft )
  {
    m_pcTComBitIf->write( m_uiByte, 8 );
    m_uiBitsLeft  = 8;
    m_uiByte      = 0;
  }
}

Void  
TEncBinCABAC::xWriteBitAndBitsToFollow( UInt uiBit )
{
  xWriteBit( uiBit );
  uiBit = 1 - uiBit;
  while( m_uiBitsToFollow > 0 )
  {
    m_uiBitsToFollow--;
    xWriteBit( uiBit );
  }
}
