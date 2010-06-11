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

/** \file     TDecBinCoderPIPE.h
    \brief    binary entropy decoder for PIPE
*/

#ifndef __TDEC_BIN_CODER_PIPE__
#define __TDEC_BIN_CODER_PIPE__

#include "TDecBinCoder.h"
#include "TDecPIPETables.h"



class V2VDecoder
{
public:
#if PIPE_LOW_DELAY_OPTION
  V2VDecoder( const UInt uiPIPEId, UInt* puiMaxDelayBitCounter );
#else
  V2VDecoder( const UInt uiPIPEId );
#endif
  ~V2VDecoder() {}

#if PIPE_LOW_DELAY_OPTION
  Void  initDelay   ( UInt uiMaxDelayInBits );
#endif
  Void  setBitstream( TComBitstream* pcTComBitstream ) { m_pcTComBitstream = pcTComBitstream; }
  
  Void  start       ();
  UInt  getNextBin  ();

private:
  Void  xReadNextCodeword ();

private:
  const UInt      m_uiPIPEId;
  const UChar*    m_paucStateTransition;
  const UInt64*   m_pauiCodeword;
  TComBitstream*  m_pcTComBitstream;
  UInt            m_uiValue;
  UInt            m_uiLength;
#if PIPE_LOW_DELAY_OPTION
  UInt            m_uiMaxCodewordLength;
  UInt            m_uiMaxDelayInBits;
  UInt            m_uiMaxBitCounterValue;
  UInt*           m_puiMaxDelayBitCounter;
#endif
};


class TDecBinPIPE : public TDecBinIf
{
public:
  TDecBinPIPE ();
  ~TDecBinPIPE();

  Void  initDelay         ( UInt           uiMaxDelayInBits );
  Void  init              ( TComBitstream* pcTComBitstream );
  Void  uninit            ();

  Void  start             ();
  Void  finish            ();

  Void  decodeBin         ( UInt& ruiBin, ContextModel& rcCtxModel );
  Void  decodeBinEP       ( UInt& ruiBin                           );
  Void  decodeBinTrm      ( UInt& ruiBin                           );

private:
  const UInt*   m_pacState2Idx;
  V2VDecoder*   m_apcV2VDecoders[ NUM_V2V_CODERS ];
#if PIPE_LOW_DELAY_OPTION
  UInt          m_uiMaxDelayBitCounter;
#endif
};


#endif

