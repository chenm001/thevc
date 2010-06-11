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

/** \file     TEncBinCoderPIPE.h
    \brief    binary entropy encoder for PIPE
*/

#ifndef __TENC_BIN_CODER_PIPE__
#define __TENC_BIN_CODER_PIPE__

#include "TEncBinCoder.h"
#include "TEncPIPETables.h"


#define INIT_CODEWORD_BUFFER_SIZE   16384 // should be large enough 

class CodewordBuffer;


class V2VEncoder
{
public:
  V2VEncoder        ( const UInt uiPIPEId, CodewordBuffer& rcCodewordBuffer );
  ~V2VEncoder       ()  {}

  Void  start       ();
  Void  pushBack    ( UInt uiBin );
  Void  flush       ();

private:
  const UInt      m_uiPIPEId;
  const UChar*    m_paucStateTransition;
  const UInt64*   m_pauiCodeword;
  CodewordBuffer& m_rcCodewordBuffer;
  UInt            m_uiState;
};



class CBufEntry
{
public:
  UInt  uiCWValue;
  UChar ucCWLength;
  UChar ucPIPEId;
};



class CodewordBuffer
{
public:
  CodewordBuffer    ();
  ~CodewordBuffer   ();

  Void  setBitStream( TComBitIf* pcTComBitIf )    { m_pcTComBitIf = pcTComBitIf; }
#if PIPE_LOW_DELAY_OPTION
  Void  initDelay   ( V2VEncoder** papcV2VEncoder, UInt uiMaxDelayInBits );
#endif
  
  Void  start       ();
  Void  finish      ();

  Void  reserve     ( UInt uiPIPEId );
  Void  write       ( UInt uiPIPEId, UInt uiValue, UInt uiLength );

private:
  Void  xIncreaseBufferSize ();
  Void  xWrite              ();

private:
  TComBitIf*    m_pcTComBitIf;
  UInt          m_uiNextWriteIdx;
  UInt          m_uiNextReserveIdx;
  UInt          m_uiBufferSize;       // just for checking buffer size
  UInt          m_uiNumFreeEntries;   // just for checking buffer size
  UInt          m_auiReservedIdBuffer [NUM_V2V_CODERS];
  CBufEntry*    m_pacCodewordBuffer;
#if PIPE_LOW_DELAY_OPTION
  V2VEncoder**  m_papcV2VEncoders;
  UInt          m_auiMaxCodewordLength[NUM_V2V_CODERS];
  UInt          m_uiMaxOverallCodewordLength;
  UInt          m_uiMaxDelayInBits;
  UInt          m_uiMaxDelayBitCounter;
#endif
};





class TEncBinPIPE : public TEncBinIf
{
public:
  TEncBinPIPE ();
  ~TEncBinPIPE();

  Void  initDelay         ( UInt       uiMaxDelayInBits );
  Void  init              ( TComBitIf* pcTComBitIf );
  Void  uninit            ();

  Void  start             ();
  Void  finish            ();
  Void  copyState         ( TEncBinIf* pcTEncBinIf );

  Void  resetBits         ();
  UInt  getNumWrittenBits ();

  Void  encodeBin         ( UInt  uiBin,  ContextModel& rcCtxModel );
  Void  encodeBinEP       ( UInt  uiBin                            );
  Void  encodeBinTrm      ( UInt  uiBin                            );

private:
  const UInt*         m_pacStat2Idx;
  CodewordBuffer      m_cCodewordBuffer;
  V2VEncoder*         m_apcV2VEncoders[ NUM_V2V_CODERS ];
};


#endif

