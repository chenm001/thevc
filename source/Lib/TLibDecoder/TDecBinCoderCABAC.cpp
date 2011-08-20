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

/** \file     TDecBinCoderCABAC.cpp
    \brief    binary entropy decoder of CABAC
*/

#include "TDecBinCoderCABAC.h"


TDecBinCABAC::TDecBinCABAC()
: m_pcTComBitstream( 0 )
{
}

TDecBinCABAC::~TDecBinCABAC()
{
}

Void
TDecBinCABAC::init( TComInputBitstream* pcTComBitstream )
{
  m_pcTComBitstream = pcTComBitstream;
}

Void
TDecBinCABAC::uninit()
{
  m_pcTComBitstream = 0;
}

Void
TDecBinCABAC::start()
{
  m_uiRange    = 510;
  m_uiValue    = 0;
  for( UInt ui = 0; ui < 9; ui++ )
  {
    xReadBit( m_uiValue );
  }
}

Void
TDecBinCABAC::finish()
{
}

Void
TDecBinCABAC::decodeBin( UInt& ruiBin, ContextModel &rcCtxModel )
{
  UInt  uiLPS   = TComCABACTables::sm_aucLPSTable[ rcCtxModel.getState() ][ ( m_uiRange >> 6 ) & 3 ];
  m_uiRange    -= uiLPS;
  if( m_uiValue < m_uiRange )
  {
    ruiBin      = rcCtxModel.getMps();
    rcCtxModel.updateMPS();
  }
  else
  {
    m_uiValue  -= m_uiRange;
    m_uiRange   = uiLPS;
    ruiBin      = 1 - rcCtxModel.getMps();
    rcCtxModel.updateLPS();
  }
  while( m_uiRange < 256 )
  {
    m_uiRange  += m_uiRange;
    xReadBit( m_uiValue );
  }
}

Void
TDecBinCABAC::decodeBinEP( UInt& ruiBin )
{
  xReadBit( m_uiValue );
  if( m_uiValue >= m_uiRange )
  {
    ruiBin      = 1;
    m_uiValue  -= m_uiRange;
  }
  else
  {
    ruiBin      = 0;
  }
}

Void
TDecBinCABAC::decodeBinTrm( UInt& ruiBin )
{
  m_uiRange -= 2;
  if( m_uiValue >= m_uiRange )
  {
    ruiBin = 1;
  }
  else
  {
    ruiBin = 0;
    while( m_uiRange < 256 )
    {
      m_uiRange += m_uiRange;
      xReadBit( m_uiValue );
    }
  }
}

Void  
TDecBinCABAC::xReadBit( UInt& ruiVal )
{
  UInt uiBit = 0;
  m_pcTComBitstream->read( 1, uiBit );
  ruiVal  = ( ruiVal << 1 ) | uiBit;
}

#if E057_INTRA_PCM
/** Reset BAC register values.
 * \returns Void
 */
Void TDecBinCABAC::resetBac()
{
  m_uiRange    = 510;
  m_uiValue    = 0;

  for( UInt ui = 0; ui < 9; ui++ )
  {
    xReadBit( m_uiValue );
  }
}
/** Decode PCM alignment zero bits.
 * \returns Void
 */
Void TDecBinCABAC::decodePCMAlignBits()
{
  Int iNum = m_pcTComBitstream->getNumBitsUntilByteAligned();

  for( UInt ui = 0; ui < iNum ; ui++ )
  {
    UInt uiBit = 0;

    m_pcTComBitstream->read( 1, uiBit );
  }
}

/** Read a PCM code.
 * \param uiLength code bit-depth
 * \param ruiCode pointer to PCM code value
 * \returns Void
 */
Void  TDecBinCABAC::xReadPCMCode(UInt uiLength, UInt& ruiCode)
{
  assert ( uiLength > 0 );
  m_pcTComBitstream->read (uiLength, ruiCode);
}
#endif
