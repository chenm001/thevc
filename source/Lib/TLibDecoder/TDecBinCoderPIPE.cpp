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

/** \file     TDecBinCoderPIPE.cpp
    \brief    binary entropy decoder for PIPE
*/

#include "TDecBinCoderPIPE.h"


#if PIPE_LOW_DELAY_OPTION
V2VDecoder::V2VDecoder( const UInt uiPIPEId, UInt* puiMaxDelayBitCounter )
#else
V2VDecoder::V2VDecoder( const UInt uiPIPEId )
#endif
: m_uiPIPEId              ( uiPIPEId )
, m_paucStateTransition   ( TDecPIPETables::sm_StateTransition[ m_uiPIPEId ] )
, m_pauiCodeword          ( TDecPIPETables::sm_Codeword       [ m_uiPIPEId ] )
, m_pcTComBitstream       ( 0 )
#if PIPE_LOW_DELAY_OPTION
, m_uiMaxCodewordLength   ( TDecPIPETables::sm_MaxTableSize   [ m_uiPIPEId ] )
, m_uiMaxDelayInBits      ( UInt(-1) >> 1 )
, m_puiMaxDelayBitCounter ( puiMaxDelayBitCounter )
#endif
{
}

#if PIPE_LOW_DELAY_OPTION
Void
V2VDecoder::initDelay( UInt uiMaxDelayInBits )
{
  m_uiMaxDelayInBits = ( uiMaxDelayInBits ? uiMaxDelayInBits : ( UInt(-1) >> 1 ) );
}
#endif

Void
V2VDecoder::start()
{
  m_uiValue               = 0;
  m_uiLength              = 0;
#if PIPE_LOW_DELAY_OPTION
  m_uiMaxBitCounterValue  = m_uiMaxDelayInBits;
#endif
}

UInt
V2VDecoder::getNextBin()
{
#if PIPE_LOW_DELAY_OPTION
  if( m_uiLength == 0 || *m_puiMaxDelayBitCounter > m_uiMaxBitCounterValue )
  {
    m_uiMaxBitCounterValue    = *m_puiMaxDelayBitCounter + m_uiMaxDelayInBits;
    *m_puiMaxDelayBitCounter += m_uiMaxCodewordLength;
    xReadNextCodeword();
  }
#else
  if( m_uiLength == 0 )
  {
    xReadNextCodeword();
  }
#endif
  m_uiLength--;
  return ( ( m_uiValue >> m_uiLength ) & 1 );
}

Void
V2VDecoder::xReadNextCodeword()
{
  UInt    uiBit   = 0;
  UInt    uiState = 0;
  UInt64  ui64CW  = 0;
  do
  {
    m_pcTComBitstream->read( 1, uiBit );
    uiState = m_paucStateTransition [ ( uiState << 2 ) + uiBit ];
    ui64CW  = m_pauiCodeword        [ uiState ];
  } while( ui64CW == 0 );
  m_uiValue   = UInt( ui64CW >> 6 );
  m_uiLength  = UInt( ui64CW & 63 );
}





TDecBinPIPE::TDecBinPIPE()
: m_pacState2Idx( TDecPIPETables::sm_State2Idx )
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
#if PIPE_LOW_DELAY_OPTION
    m_apcV2VDecoders[ uiIdx ] = new V2VDecoder( uiIdx, &m_uiMaxDelayBitCounter );
#else
    m_apcV2VDecoders[ uiIdx ] = new V2VDecoder( uiIdx );
#endif
  }
}

TDecBinPIPE::~TDecBinPIPE()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    delete m_apcV2VDecoders[ uiIdx ];
  }
}

Void
TDecBinPIPE::initDelay( UInt uiMaxDelayInBits )
{
#if PIPE_LOW_DELAY_OPTION
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VDecoders[ uiIdx ]->initDelay( uiMaxDelayInBits );
  }
#endif
}


Void
TDecBinPIPE::init( TComBitstream* pcTComBitstream )
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VDecoders[ uiIdx ]->setBitstream( pcTComBitstream );
  }
}


Void
TDecBinPIPE::uninit()
{
}


Void
TDecBinPIPE::start()
{
  for( UInt uiIdx = 0; uiIdx < NUM_V2V_CODERS; uiIdx++ )
  {
    m_apcV2VDecoders[ uiIdx ]->start();
  }
#if PIPE_LOW_DELAY_OPTION
  m_uiMaxDelayBitCounter = 0;
#endif
}


Void
TDecBinPIPE::finish()
{
}


Void
TDecBinPIPE::decodeBin( UInt& ruiBin, ContextModel &rcCtxModel )
{
  UInt uiPIPEId = m_pacState2Idx  [ rcCtxModel.getState() ];
  UInt uiBin    = m_apcV2VDecoders[ uiPIPEId ]->getNextBin();
  ruiBin        = ( uiBin ^ rcCtxModel.getMps() );
#if HHI_RQT
  {
    DTRACE_CABAC_V( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tstate=" )
    DTRACE_CABAC_V( ( rcCtxModel.getState() << 1 ) + rcCtxModel.getMps() )
    DTRACE_CABAC_T( "\tsymbol=" )
    DTRACE_CABAC_V( ruiBin )
    DTRACE_CABAC_T( "\n" )
  }
#endif
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
TDecBinPIPE::decodeBinEP( UInt& ruiBin )
{
  ruiBin = m_apcV2VDecoders[ 0 ]->getNextBin();
}


Void
TDecBinPIPE::decodeBinTrm( UInt& ruiBin )
{
  ruiBin = m_apcV2VDecoders[ NUM_V2V_CODERS - 1 ]->getNextBin();
}

