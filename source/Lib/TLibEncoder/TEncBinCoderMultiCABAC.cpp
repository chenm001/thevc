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

/** \file     TEncBinCoderMultiCABAC.cpp
    \brief    binary entropy encoder for multi-partition CABAC
*/

#include "TEncBinCoderMultiCABAC.h"


TEncBinMultiCABAC::TEncBinMultiCABAC()
: m_pacStat2Idx( TComCABACTables::sm_State2Idx )
{
}

TEncBinMultiCABAC::~TEncBinMultiCABAC()
{
}

Void
TEncBinMultiCABAC::init( TComBitIf* pcTComBitIf )
{
  m_pcTComBitIf = pcTComBitIf;
}

Void
TEncBinMultiCABAC::uninit()
{
}

Void
TEncBinMultiCABAC::start()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_acBinBuffer[ uiIdx ].reset();
  }
}

Void
TEncBinMultiCABAC::finish()
{
  //===== write codewords into bit buffer (except partition 0) =====
  UInt  auiWrittenBits[ NUM_V2V_CODERS ];
  UInt  uiNumOverallBits = 0;
  m_cBitBuffer.reset();
  for( UInt uiIdx = NUM_V2V_CODERS - 1; uiIdx > 0; uiIdx-- )
  {
    xEncode( uiIdx, auiWrittenBits[ uiIdx ] );
    uiNumOverallBits += auiWrittenBits[ uiIdx ];
  }

  //===== write partition sizes =====
  xEncodePartSize( m_pcTComBitIf, auiWrittenBits[ NUM_V2V_CODERS - 1 ] );
  for( UInt uiIdx = NUM_V2V_CODERS - 2; uiIdx > 0; uiIdx-- )
  {
    Int   iDiff   = Int( auiWrittenBits[ uiIdx ] ) - Int( auiWrittenBits[ uiIdx + 1 ] );
    UInt  uiCode  = ( iDiff < 0 ? ( UInt( -iDiff ) << 1 ) - 1 : UInt( iDiff ) << 1 );
    xEncodePartSize( m_pcTComBitIf, uiCode );
  }

  //===== write data of partitions 11-1 to bitstream =====
  while( m_cBitBuffer.numAvailableBits() >= 32 )
  {
    UInt uiVal = m_cBitBuffer.removeBits( 32 );
    m_pcTComBitIf->write( uiVal, 32 );
  }
  if( m_cBitBuffer.numAvailableBits() )
  {
    UInt uiNum = m_cBitBuffer.numAvailableBits();
    UInt uiVal = m_cBitBuffer.removeBits( uiNum );
    m_pcTComBitIf->write( uiVal, uiNum );
  }

  //===== write data of partitions 0 to bitstream =====
  TComBitBuffer& rcBinBuffer = m_acBinBuffer[ 0 ];
  while( rcBinBuffer.numAvailableBits() >= 32 )
  {
    UInt uiVal = rcBinBuffer.removeBits( 32 );
    m_pcTComBitIf->write( uiVal, 32 );
  }
  if( rcBinBuffer.numAvailableBits() )
  {
    UInt uiNum = rcBinBuffer.numAvailableBits();
    UInt uiVal = rcBinBuffer.removeBits( uiNum );
    m_pcTComBitIf->write( uiVal, uiNum );
  }

  //===== write stop bit =====
  m_pcTComBitIf->write( 1, 1 );
}

Void
TEncBinMultiCABAC::copyState( TEncBinIf* pcTEncBinIf )
{
  assert(0);
}

Void  
TEncBinMultiCABAC::resetBits()
{
  assert(0); 
}

UInt
TEncBinMultiCABAC::getNumWrittenBits()
{
  assert(0);
  return 0;
}

Void
TEncBinMultiCABAC::encodeBin( UInt uiBin, ContextModel &rcCtxModel )
{
  UInt uiPIPEId = m_pacStat2Idx[ rcCtxModel.getState() ];
  if( uiBin != rcCtxModel.getMps() )
  {
    m_acBinBuffer[ uiPIPEId ].insertBit( 1 );
    rcCtxModel.updateLPS();
  }
  else
  {
    m_acBinBuffer[ uiPIPEId ].insertBit( 0 );
    rcCtxModel.updateMPS();
  }
}

Void
TEncBinMultiCABAC::encodeBinEP( UInt uiBin )
{
  m_acBinBuffer[ 0 ].insertBit( uiBin );
}


Void
TEncBinMultiCABAC::encodeBinTrm( UInt uiBin )
{
  m_acBinBuffer[ NUM_V2V_CODERS - 1 ].insertBit( uiBin );
}


Void
TEncBinMultiCABAC::xEncode( UInt uiIdx, UInt& ruiWrittenBits )
{
  TComBitBuffer&  rcBinBuffer = m_acBinBuffer[ uiIdx ];
  const UChar*    pucLPSTable = TComCABACTables::sm_aucLPSTable[ TComCABACTables::sm_Idx2State[ uiIdx ] ];
  const UInt      uiStartBits = m_cBitBuffer.getWrittenBits();
  if( rcBinBuffer.numAvailableBits() )
  {
    UInt  uiLPS       = 0;
    UInt  uiLow       = 0;
    UInt  uiRange     = 510;
    m_uiBitsToFollow  = 0;
    m_uiByte          = 0;
    m_uiBitsLeft      = 9;
    // encode CABAC bins
    while( rcBinBuffer.numAvailableBits() )
    {
      uiLPS     = pucLPSTable[ ( uiRange >> 6 ) & 3 ];
      uiRange  -= uiLPS;
      if( rcBinBuffer.removeBit() )
      {
        uiLow  += uiRange;
        uiRange = uiLPS;
      }
      while( uiRange < 256 )
      {
        if( uiLow >= 512 )
        {
          xWriteBitAndBitsToFollow( 1 );
          uiLow -= 512;
        }
        else if( uiLow < 256 )
        {
          xWriteBitAndBitsToFollow( 0 );
        }
        else
        {
          m_uiBitsToFollow++;
          uiLow -= 256;
        }
        uiLow   <<= 1;
        uiRange <<= 1;
      }
    }
    // terminate codeword 
    UInt uiHigh = uiLow + uiRange;
    if(  uiHigh > 512 )
    {
      xWriteBitAndBitsToFollow( 1 );
      uiHigh -= 512;
    }
    else
    {
      xWriteBitAndBitsToFollow( 0 );
    }
    if( uiHigh > 256 )
    {
      xWriteBit( 1 );
    }
    if( 8 - m_uiBitsLeft != 0 )
    {
      m_cBitBuffer.insertBits( m_uiByte, 8 - m_uiBitsLeft );
    }
  }
  ruiWrittenBits = m_cBitBuffer.getWrittenBits() - uiStartBits;
}

Void
TEncBinMultiCABAC::xWriteBitAndBitsToFollow( UInt uiBit )
{
  xWriteBit( uiBit );
  uiBit = 1 - uiBit;
  while( m_uiBitsToFollow > 0 )
  {
    m_uiBitsToFollow--;
    xWriteBit( uiBit );
  }
}

Void
TEncBinMultiCABAC::xWriteBit( UInt uiBit )
{
  m_uiByte += m_uiByte + uiBit;
  if( ! --m_uiBitsLeft )
  {
    m_cBitBuffer.insertBits( m_uiByte, 8 );
    m_uiBitsLeft  = 8;
    m_uiByte      = 0;
  }
}

Void
TEncBinMultiCABAC::xEncodePartSize( TComBitIf* pcTComBitIf, UInt uiSize )
{
  if( uiSize < 128 )
  {
    pcTComBitIf->write( uiSize << 1, 8 );
    return;
  }
  uiSize -= 128;
  if( uiSize < 16384 )
  {
    uiSize = ( uiSize << 2 ) | 1;
    pcTComBitIf->write( uiSize & 255, 8 );
    pcTComBitIf->write( uiSize >>  8, 8 );
    return;
  }
  uiSize -= 16384;
  if( uiSize < 2097152 )
  {
    uiSize = ( uiSize << 3 ) | 3;
    pcTComBitIf->write( uiSize & 255, 8 );
    uiSize >>= 8;
    pcTComBitIf->write( uiSize & 255, 8 );
    pcTComBitIf->write( uiSize >>  8, 8 );
    return;
  }
  uiSize -= 2097152;
  if( uiSize >= 536870912 )
  {
    assert(0);
  }
  uiSize = ( uiSize << 3 ) | 7;
  pcTComBitIf->write( uiSize & 255, 8 );
  uiSize >>= 8;
  pcTComBitIf->write( uiSize & 255, 8 );
  uiSize >>= 8;
  pcTComBitIf->write( uiSize & 255, 8 );
  pcTComBitIf->write( uiSize >>  8, 8 );
}
