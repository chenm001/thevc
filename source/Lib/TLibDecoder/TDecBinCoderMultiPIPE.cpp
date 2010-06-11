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

/** \file     TDecBinCoderMultiPIPE.cpp
    \brief    binary entropy decoder for multi-partition PIPE
*/

#include "TDecBinCoderMultiPIPE.h"


TDecBinMultiPIPE::TDecBinMultiPIPE()
: m_pacState2Idx( TDecPIPETables::sm_State2Idx )
{
}

TDecBinMultiPIPE::~TDecBinMultiPIPE()
{
}

Void
TDecBinMultiPIPE::init( TComBitstream* pcTComBitstream )
{
  m_pcTComBitstream = pcTComBitstream;
}


Void
TDecBinMultiPIPE::uninit()
{
}


Void
TDecBinMultiPIPE::start()
{
  //===== read partition sizes =====
  UInt auiWrittenBits[ NUM_V2V_CODERS ];
  auiWrittenBits[ NUM_V2V_CODERS - 1 ] = xDecodePartSize( m_pcTComBitstream );
  for( UInt uiIdx = NUM_V2V_CODERS - 2; uiIdx > 0; uiIdx-- )
  {
    UInt  uiCode            = xDecodePartSize( m_pcTComBitstream );
    Int   iDiff             = ( uiCode & 1 ? -Int( ( uiCode + 1 ) >> 1 ) : Int( uiCode >> 1 ) );
    auiWrittenBits[ uiIdx ] = UInt( (Int)auiWrittenBits[ uiIdx + 1 ] + iDiff );
  }

  //===== fill bit buffers =====
  for( UInt uiIdx = NUM_V2V_CODERS - 1; uiIdx > 0; uiIdx-- )
  {
    xDecode( uiIdx, auiWrittenBits[ uiIdx ] );
  }
  xDecodeEP();
}


Void
TDecBinMultiPIPE::finish()
{
}


Void
TDecBinMultiPIPE::decodeBin( UInt& ruiBin, ContextModel &rcCtxModel )
{
  UInt uiPIPEId = m_pacState2Idx[ rcCtxModel.getState() ];
  UInt uiBin    = m_acBinBuffer [ uiPIPEId ].removeBit();
  ruiBin        = ( uiBin ^ rcCtxModel.getMps() );
  if(  uiBin )
  {
    rcCtxModel.updateLPS();
  }
  else
  {
    rcCtxModel.updateMPS();
  }
}


Void
TDecBinMultiPIPE::decodeBinEP( UInt& ruiBin )
{
  ruiBin = m_acBinBuffer[ 0 ].removeBit();
}


Void
TDecBinMultiPIPE::decodeBinTrm( UInt& ruiBin )
{
  ruiBin = m_acBinBuffer[ NUM_V2V_CODERS - 1 ].removeBit();
}


Void
TDecBinMultiPIPE::xDecode( UInt uiIdx, UInt uiBits )
{
  assert( m_pcTComBitstream->getBitsLeft() >= uiBits );
  TComBitBuffer&  rcBinBuffer       = m_acBinBuffer                     [ uiIdx ];
  const UChar*    paucStateTrans    = TDecPIPETables::sm_StateTransition[ uiIdx ];
  const UInt64*   paui64Codeword    = TDecPIPETables::sm_Codeword       [ uiIdx ];
  const UInt      uiTargetBitsLeft  = m_pcTComBitstream->getBitsLeft() - uiBits;
  rcBinBuffer.reset();
  while( m_pcTComBitstream->getBitsLeft() > uiTargetBitsLeft )
  {
    UInt    uiBit   = 0;
    UInt    uiState = 0;
    UInt64  ui64CW  = 0;
    do
    {
      m_pcTComBitstream->read( 1, uiBit );
      uiState = paucStateTrans[ ( uiState << 2 ) + uiBit ];
      ui64CW  = paui64Codeword[ uiState ];
    } while( ui64CW == 0 );
    rcBinBuffer.insertBits( UInt( ui64CW >> 6 ), UInt( ui64CW & 63 ) );
  }
  assert( m_pcTComBitstream->getBitsLeft() == uiTargetBitsLeft ); // otherwise something wrong
}


Void
TDecBinMultiPIPE::xDecodeEP()
{
  TComBitBuffer&  rcBinBuffer   = m_acBinBuffer[ 0 ];
  UInt            uiBitsToRead  = m_pcTComBitstream->getBitsLeft();
  UInt            uiVal         = 0;
  rcBinBuffer.reset();
  while( uiBitsToRead >= 32  )
  {
    m_pcTComBitstream->read( 32, uiVal );
    rcBinBuffer.insertBits ( uiVal, 32 );
    uiBitsToRead -= 32;
  }
  if( uiBitsToRead )
  {
    m_pcTComBitstream->read( uiBitsToRead, uiVal );
    rcBinBuffer.insertBits ( uiVal, uiBitsToRead );
  }
}

UInt
TDecBinMultiPIPE::xDecodePartSize( TComBitstream* pcTComBitstream )
{
  UInt uiCode = 0;
  UInt uiSize = 0;

  pcTComBitstream->read( 8, uiSize );
  if( ( uiSize & 1 ) == 0 )
  {
    return ( uiSize >> 1 );
  }
  uiSize >>= 1;

  pcTComBitstream->read( 8, uiCode );
  uiSize  += ( uiCode << 7 );
  if( ( uiSize & 1 ) == 0 )
  {
    return 128 + ( uiSize >> 1 );
  }
  uiSize >>= 1;

  pcTComBitstream->read( 8, uiCode );
  uiSize  += ( uiCode << 14 );
  if( ( uiSize & 1 ) == 0 )
  {
    return 16512 + ( uiSize >> 1 );
  }
  uiSize >>= 1;

  pcTComBitstream->read( 8, uiCode );
  uiSize  += ( uiCode << 21 );
  return 2113664 + uiSize;
}
