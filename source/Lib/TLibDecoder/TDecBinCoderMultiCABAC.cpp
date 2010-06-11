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

/** \file     TDecBinCoderMultiCABAC.cpp
    \brief    binary entropy decoder for multi-partition CABAC
*/

#include "TDecBinCoderMultiCABAC.h"


TDecBinMultiCABAC::TDecBinMultiCABAC()
: m_pacState2Idx( TComCABACTables::sm_State2Idx )
{
}

TDecBinMultiCABAC::~TDecBinMultiCABAC()
{
}

Void
TDecBinMultiCABAC::init( TComBitstream* pcTComBitstream )
{
  m_pcTComBitstream = pcTComBitstream;
}


Void
TDecBinMultiCABAC::uninit()
{
}


Void
TDecBinMultiCABAC::start()
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
TDecBinMultiCABAC::finish()
{
}


Void
TDecBinMultiCABAC::decodeBin( UInt& ruiBin, ContextModel &rcCtxModel )
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
TDecBinMultiCABAC::decodeBinEP( UInt& ruiBin )
{
  ruiBin = m_acBinBuffer[ 0 ].removeBit();
}


Void
TDecBinMultiCABAC::decodeBinTrm( UInt& ruiBin )
{
  ruiBin = m_acBinBuffer[ NUM_V2V_CODERS - 1 ].removeBit();
}


Void
TDecBinMultiCABAC::xDecode( UInt uiIdx, UInt uiBits )
{
  assert( m_pcTComBitstream->getBitsLeft() >= uiBits );
  TComBitBuffer&  rcBinBuffer       = m_acBinBuffer[ uiIdx ];
  const UChar*    pucLPSTable       = TComCABACTables::sm_aucLPSTable[ TComCABACTables::sm_Idx2State[ uiIdx ] ];
  const UInt      uiTargetBitsLeft  = m_pcTComBitstream->getBitsLeft() - uiBits;
  rcBinBuffer.reset();
  if( uiBits )
  {
    Int   iBitsToRead = Int(uiBits) + 10;
    UInt  uiRange     = 510;
    UInt  uiValue     = 0;
    UInt  uiLPS       = 0;
    for( UInt ui = 0; ui < 9; ui++ )
    {
      xReadBit( uiValue, uiTargetBitsLeft );
      iBitsToRead--;
    }
    while( iBitsToRead > 0 )
    {
      uiLPS     = pucLPSTable[ ( uiRange >> 6 ) & 3 ];
      uiRange  -= uiLPS;
      if( uiValue < uiRange )
      {
        rcBinBuffer.insertBit( 0 );
      }
      else
      {
        rcBinBuffer.insertBit( 1 );
        uiValue-= uiRange;
        uiRange = uiLPS;
      }
      while( uiRange < 256 )
      {
        uiRange += uiRange;
        xReadBit( uiValue, uiTargetBitsLeft );
        iBitsToRead--;
      }
    }
    assert( m_pcTComBitstream->getBitsLeft() == uiTargetBitsLeft ); // otherwise something wrong
  }
}


Void
TDecBinMultiCABAC::xDecodeEP()
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

Void
TDecBinMultiCABAC::xReadBit( UInt& ruiVal, UInt uiTargetBitsLeft )
{
  if( m_pcTComBitstream->getBitsLeft() <= uiTargetBitsLeft )
  {
    ruiVal = ( ruiVal << 1 );
    return;
  }
  UInt uiBit = 0;
  m_pcTComBitstream->read( 1, uiBit );
  ruiVal = ( ruiVal << 1 ) | uiBit;
}


UInt
TDecBinMultiCABAC::xDecodePartSize( TComBitstream* pcTComBitstream )
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
