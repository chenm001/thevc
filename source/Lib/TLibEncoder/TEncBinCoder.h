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

/** \file     TEncBinCoder.h
    \brief    binary entropy encoder interface
*/

#ifndef __TENC_BIN_CODER__
#define __TENC_BIN_CODER__

#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/TComBitStream.h"


class TEncBinCABAC;


class TEncBinIf
{
public:
  virtual Void  init              ( TComBitIf* pcTComBitIf )                  = 0;
  virtual Void  uninit            ()                                          = 0;

  virtual Void  start             ()                                          = 0;
  virtual Void  finish            ()                                          = 0;
  virtual Void  copyState         ( TEncBinIf* pcTEncBinIf )                  = 0;    

  virtual Void  resetBits         ()                                          = 0;
  virtual UInt  getNumWrittenBits ()                                          = 0;

  virtual Void  encodeBin         ( UInt  uiBin,  ContextModel& rcCtxModel )  = 0;
  virtual Void  encodeBinEP       ( UInt  uiBin                            )  = 0;
  virtual Void  encodeBinTrm      ( UInt  uiBin                            )  = 0;

  virtual TEncBinCABAC*   getTEncBinCABAC   ()  { return 0; }
  
  virtual ~TEncBinIf() {}
};


#endif

