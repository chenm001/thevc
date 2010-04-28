/* ====================================================================================================================

	The copyright in this software is being made available under the License included below.
	This software may be subject to other third party and 	contributor rights, including patent rights, and no such
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

/** \file			TVideoIOBits.cpp
    \brief		bitstream file I/O class
*/

#include <cstdlib>
//#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "TVideoIOBits.h"

using namespace std;

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param pchFile		file name string
    \param bWriteMode	file open mode
 */
Void TVideoIOBits::openBits( char* pchFile, Bool bWriteMode )
{
  if ( bWriteMode )
  {
    m_cHandle.open( pchFile, ios::binary | ios::out );

    if( m_cHandle.fail() )
    {
      printf("\nfailed to write Bitstream file\n");
      exit(0);
    }
  }
  else
  {
    m_cHandle.open( pchFile, ios::binary | ios::in );

    if( m_cHandle.fail() )
    {
      printf("\nfailed to read Bitstream file\n");
      exit(0);
    }
  }

  return;
}

Void TVideoIOBits::closeBits()
{
  m_cHandle.close();
}

/** \param	rpcBitstream		bitstream class pointer
    \retval									true if EOF is reached
 */
Bool TVideoIOBits::readBits( TComBitstream*& rpcBitstream )
{
  UInt  uiBytes = 0;

  rpcBitstream->rewindStreamPacket();

	// check end-of-file
  if ( m_cHandle.eof() ) return true;

	// read 32-bit packet size
  m_cHandle.read( reinterpret_cast<char*>(&uiBytes), sizeof (Int) );

  // kolya
  if ( m_cHandle.eof() ) return true; //additional insertion to avoid over-reading, for <fstream>

	// read packet data
  m_cHandle.read(  reinterpret_cast<char*>(rpcBitstream->getBuffer()), uiBytes );

	// initialize parsing process
  rpcBitstream->initParsing ( uiBytes );

  assert (uiBytes >= 4);

  return false;
}

/** \param	pcBitstream		bitstream class pointer
 */
Void TVideoIOBits::writeBits( TComBitstream* pcBitstream )
{
  UInt*  plBuff  = pcBitstream->getStartStream();
  UInt   uiBytes = pcBitstream->getNumberOfWrittenBits() >> 3;

	// write 32-bit packet size
  m_cHandle.write( reinterpret_cast<char*>(&uiBytes ), sizeof(UInt) );

	// write packet data
  m_cHandle.write( reinterpret_cast<char*>(plBuff   ), uiBytes      );
}
