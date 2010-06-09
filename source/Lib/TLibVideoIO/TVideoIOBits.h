/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, SAMSUNG ELECTRONICS CO., LTD. and BRITISH BROADCASTING CORPORATION
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of SAMSUNG ELECTRONICS CO., LTD. nor the name of the BRITISH BROADCASTING CORPORATION
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

/** \file     TVideoIOBits.h
    \brief    bitstream file I/O class (header)
*/

#ifndef __TVIDEOIOBITS__
#define __TVIDEOIOBITS__

#include <stdio.h>
#include <fstream>
#include <iostream>
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComBitStream.h"

using namespace std;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// bitstream file I/O class
class TVideoIOBits
{
private:
  fstream   m_cHandle;                                      ///< file handle

public:
  TVideoIOBits()            {}
  virtual ~TVideoIOBits()   {}

  Void openBits   ( char* pchFile,  Bool bWriteMode );      ///< open or create file
  Void closeBits  ();                                       ///< close file

  Bool readBits   ( TComBitstream*& rpcBitstream    );      ///< read  one packet from file
  Void writeBits  ( TComBitstream*  pcBitstream     );      ///< write one packet to   file

};

/// bitstream file I/O class
class TVideoIOBitsStartCode
{
private:
  fstream   m_cHandle;                                      ///< file handle

public:
  TVideoIOBitsStartCode()            {}
  virtual ~TVideoIOBitsStartCode()   {}

  Void openBits   ( char* pchFile,  Bool bWriteMode );      ///< open or create file
  Void closeBits  ();                                       ///< close file

  Bool readBits   ( TComBitstream*& rpcBitstream    );      ///< read  one packet from file
  Void writeBits  ( TComBitstream*  pcBitstream     );      ///< write one packet to   file

private:
  int xFindNextStartCode(UInt& ruiPacketSize, UChar* pucBuffer); ///< get packet size and number of startcode bytes and seeks to the packet's start position

};

#endif // __TVIDEOIOBITS__

