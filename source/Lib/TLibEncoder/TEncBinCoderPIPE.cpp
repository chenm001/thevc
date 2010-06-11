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

/** \file     TEncBinCoderPIPE.cpp
    \brief    binary entropy encoder for PIPE
*/

#include "TEncBinCoderPIPE.h"


#define INVALID  UInt(-1)



V2VEncoder::V2VEncoder( const UInt uiPIPEId, CodewordBuffer& rcCodewordBuffer )
: m_uiPIPEId            ( uiPIPEId )
, m_paucStateTransition ( TEncPIPETables::sm_StateTransition[ m_uiPIPEId ] )
, m_pauiCodeword        ( TEncPIPETables::sm_Codeword       [ m_uiPIPEId ] )
, m_rcCodewordBuffer    ( rcCodewordBuffer )
{
}

Void
V2VEncoder::start()
{
  m_uiState = 0;
}

Void
V2VEncoder::pushBack( UInt uiBin )
{
  if( m_uiState == 0 )
  {
    m_rcCodewordBuffer.reserve( m_uiPIPEId );
  }
  m_uiState     = m_paucStateTransition[ ( m_uiState << 2 ) + uiBin ];
  UInt64 ui64CW = m_pauiCodeword[ m_uiState ];
  if( ui64CW )
  {
    m_rcCodewordBuffer.write( m_uiPIPEId, UInt( ui64CW >> 6 ), UInt( ui64CW & 63 ) );
    m_uiState = 0;
  }
}

Void
V2VEncoder::flush()
{
  if( m_uiState )
  {
    UInt64 ui64CW = m_pauiCodeword[ m_paucStateTransition [ ( m_uiState << 2 ) + 2 ] ];
    m_rcCodewordBuffer.write( m_uiPIPEId, UInt( ui64CW >> 6 ), UInt( ui64CW & 63 ) );
    m_uiState = 0;
  }
}





CodewordBuffer::CodewordBuffer()
: m_pcTComBitIf       ( 0 )
, m_uiBufferSize      ( INIT_CODEWORD_BUFFER_SIZE )
, m_pacCodewordBuffer ( new CBufEntry [ m_uiBufferSize ] )
#if PIPE_LOW_DELAY_OPTION
, m_papcV2VEncoders   ( 0 )
#endif
{
}

CodewordBuffer::~CodewordBuffer()
{
  delete [] m_pacCodewordBuffer;
}

#if PIPE_LOW_DELAY_OPTION
Void
CodewordBuffer::initDelay( V2VEncoder** papcV2VEncoder, UInt uiMaxDelayInBits )
{
  m_papcV2VEncoders             = papcV2VEncoder;
  m_uiMaxOverallCodewordLength  = 0;
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_auiMaxCodewordLength    [ uiIdx ] = TEncPIPETables::sm_MaxTableSize[ uiIdx ];
    if( m_auiMaxCodewordLength[ uiIdx ] > m_uiMaxOverallCodewordLength )
    {
      m_uiMaxOverallCodewordLength      = m_auiMaxCodewordLength[ uiIdx ];
    }
  }
  m_uiMaxDelayInBits            = ( uiMaxDelayInBits ? uiMaxDelayInBits : UInt(-1) );
  assert( m_uiMaxDelayInBits   >= m_uiMaxOverallCodewordLength );
}
#endif

Void
CodewordBuffer::start()
{
  m_uiNextWriteIdx                          = 0;
  m_uiNextReserveIdx                        = 0;
  m_uiNumFreeEntries                        = m_uiBufferSize;
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_auiReservedIdBuffer[ uiIdx ]          = INVALID;
  }
  for( UInt uiIdx = 0; uiIdx < m_uiBufferSize; uiIdx++ )
  {
    m_pacCodewordBuffer[ uiIdx ].ucCWLength = 0;
  }
#if PIPE_LOW_DELAY_OPTION
  m_uiMaxDelayBitCounter                    = 0;
#endif
}

Void
CodewordBuffer::finish()
{
  m_pcTComBitIf->write( 1, 1 );
}

Void
CodewordBuffer::reserve( UInt uiPIPEId )
{
#if PIPE_LOW_DELAY_OPTION // low-delay encoder flush
  UInt uiMaxBitCounter = m_uiMaxDelayInBits - m_auiMaxCodewordLength[ uiPIPEId ];
  while( m_uiMaxDelayBitCounter > uiMaxBitCounter )
  {
    m_papcV2VEncoders[ m_pacCodewordBuffer[ m_uiNextWriteIdx ].ucPIPEId ]->flush();
  }
#endif
  if( m_uiNumFreeEntries == 0 )
  {
    xIncreaseBufferSize();
  }
  m_pacCodewordBuffer   [ m_uiNextReserveIdx ].ucPIPEId = uiPIPEId;
  m_auiReservedIdBuffer [ uiPIPEId           ]          = m_uiNextReserveIdx++;
  if( m_uiNextReserveIdx == m_uiBufferSize )
  {
    m_uiNextReserveIdx = 0;
  }
  m_uiNumFreeEntries--;
#if PIPE_LOW_DELAY_OPTION
  m_uiMaxDelayBitCounter += m_auiMaxCodewordLength[ uiPIPEId ];
#endif
}

Void
CodewordBuffer::write( UInt uiPIPEId, UInt uiValue, UInt uiLength )
{
  UInt uiCurrIdx = m_auiReservedIdBuffer[ uiPIPEId ];
  m_auiReservedIdBuffer[ uiPIPEId  ]            = INVALID;
  m_pacCodewordBuffer  [ uiCurrIdx ].ucCWLength = uiLength;
  m_pacCodewordBuffer  [ uiCurrIdx ].uiCWValue  = uiValue;
  if( uiCurrIdx == m_uiNextWriteIdx )
  {
    xWrite();
  }
}

Void
CodewordBuffer::xIncreaseBufferSize()
{
  UInt              uiNewFreeEntries      = m_uiNumFreeEntries  + m_uiBufferSize;
  UInt              uiNewBufferSize       = m_uiBufferSize      + m_uiBufferSize;
  UInt              uiNewNextWriteIdx     = 0;
  UInt              uiNewNextReservedIdx  = m_uiBufferSize      - m_uiNumFreeEntries;
  CBufEntry*        pacNewCodewordBuffer  = new CBufEntry       [ uiNewBufferSize ];
  for( UInt uiSrcIdx = m_uiNextWriteIdx, uiDesIdx = uiNewNextWriteIdx; uiDesIdx < uiNewNextReservedIdx; uiDesIdx++, uiSrcIdx = ( uiSrcIdx + 1 ) % m_uiBufferSize )
  {
    pacNewCodewordBuffer[ uiDesIdx ] = m_pacCodewordBuffer[ uiSrcIdx ];
    if( pacNewCodewordBuffer[ uiDesIdx ].ucCWLength == 0 )
    {
      m_auiReservedIdBuffer[ pacNewCodewordBuffer[ uiDesIdx ].ucPIPEId ] = uiDesIdx;
    }
  }
  for( UInt uiDesIdx = uiNewNextReservedIdx; uiDesIdx < uiNewBufferSize; uiDesIdx++ )
  {
    pacNewCodewordBuffer[ uiDesIdx ].ucCWLength = 0;
  }
  delete [] m_pacCodewordBuffer;
  m_uiNextWriteIdx    = uiNewNextWriteIdx;
  m_uiNextReserveIdx  = uiNewNextReservedIdx;
  m_uiBufferSize      = uiNewBufferSize;
  m_uiNumFreeEntries  = uiNewFreeEntries;
  m_pacCodewordBuffer = pacNewCodewordBuffer;
}

Void
CodewordBuffer::xWrite()
{
  while( m_pacCodewordBuffer[ m_uiNextWriteIdx ].ucCWLength )
  {
    CBufEntry&  rcEntry     = m_pacCodewordBuffer[ m_uiNextWriteIdx ];
    m_pcTComBitIf->write( rcEntry.uiCWValue, rcEntry.ucCWLength );
#if PIPE_LOW_DELAY_OPTION
    m_uiMaxDelayBitCounter -= m_auiMaxCodewordLength[ rcEntry.ucPIPEId ];
#endif
    rcEntry.ucCWLength      = 0;
    m_uiNextWriteIdx++;
    if( m_uiNextWriteIdx   == m_uiBufferSize )
    {
      m_uiNextWriteIdx      = 0;
    }
    m_uiNumFreeEntries++;
  }
}





TEncBinPIPE::TEncBinPIPE()
: m_pacStat2Idx     ( TEncPIPETables::sm_State2Idx )
, m_cCodewordBuffer ()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VEncoders[ uiIdx ] = new V2VEncoder( uiIdx, m_cCodewordBuffer );
  }
#if PIPE_LOW_DELAY_OPTION
  m_cCodewordBuffer.initDelay( m_apcV2VEncoders, 0 );
#endif
}

TEncBinPIPE::~TEncBinPIPE()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    delete m_apcV2VEncoders[ uiIdx ];
  }
}

Void
TEncBinPIPE::initDelay( UInt uiMaxDelayInBits )
{
#if PIPE_LOW_DELAY_OPTION
  m_cCodewordBuffer.initDelay( m_apcV2VEncoders, uiMaxDelayInBits );
#endif
}

Void
TEncBinPIPE::init( TComBitIf* pcTComBitIf )
{
  m_cCodewordBuffer.setBitStream( pcTComBitIf );
}

Void
TEncBinPIPE::uninit()
{
}

Void
TEncBinPIPE::start()
{
  m_cCodewordBuffer.start();
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VEncoders[ uiIdx ]->start();
  }
}

Void
TEncBinPIPE::finish()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VEncoders[ uiIdx ]->flush();
  }
  m_cCodewordBuffer.finish();
}

Void
TEncBinPIPE::copyState( TEncBinIf* pcTEncBinIf )
{
  assert(0);
}

Void  
TEncBinPIPE::resetBits()
{
  assert(0); 
}

UInt
TEncBinPIPE::getNumWrittenBits()
{
  assert(0);
  return 0;
}

Void
TEncBinPIPE::encodeBin( UInt uiBin, ContextModel &rcCtxModel )
{
#if HHI_RQT
  {
    DTRACE_CABAC_V( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tstate=" )
    DTRACE_CABAC_V( ( rcCtxModel.getState() << 1 ) + rcCtxModel.getMps() )
    DTRACE_CABAC_T( "\tsymbol=" )
    DTRACE_CABAC_V( uiBin )
    DTRACE_CABAC_T( "\n" )
  }
#endif
  UInt uiPIPEId = m_pacStat2Idx[ rcCtxModel.getState() ];
  if( uiBin != rcCtxModel.getMps() )
  {
    m_apcV2VEncoders[ uiPIPEId ]->pushBack( 1 );
    rcCtxModel.updateLPS();
  }
  else
  {
    m_apcV2VEncoders[ uiPIPEId ]->pushBack( 0 );
    rcCtxModel.updateMPS();
  }
}

Void
TEncBinPIPE::encodeBinEP( UInt uiBin )
{
  m_apcV2VEncoders[ 0 ]->pushBack( uiBin );
}


Void
TEncBinPIPE::encodeBinTrm( UInt uiBin )
{
  m_apcV2VEncoders[ NUM_V2V_CODERS - 1 ]->pushBack( uiBin );
}

