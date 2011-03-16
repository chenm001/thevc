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

/** \file     TVideoIOBits.cpp
    \brief    bitstream file I/O class
*/

#include <cstdlib>
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

/**
 \param pchFile    file name string
 \param bWriteMode file open mode
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

/**
 \param  rpcBitstream    bitstream class pointer
 \retval                 true if EOF is reached
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

/** \param  pcBitstream   bitstream class pointer
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

////////////////////////////////////////////////////////////////////

Void TVideoIOBitsStartCode::openBits( char* pchFile, Bool bWriteMode )
{
  if ( bWriteMode )
  {
    m_cHandle.open( pchFile, ios::binary | ios::out );
    
    if( m_cHandle.fail() )
    {
      printf("\nfailed to write Bitstream file\n");
      exit(0);
    }
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
    m_ucFastLookupBuf = NULL;
#endif
  }
  else
  {
    m_cHandle.open( pchFile, ios::binary | ios::in );
    
    if( m_cHandle.fail() )
    {
      printf("\nfailed to read Bitstream file\n");
      exit(0);
    }
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
    m_cHandle.seekg( 0, ios_base::end );
    m_ulBitstreamLength = m_cHandle.tellg();
    m_ucFastLookupBuf   = new UChar [AD_HOC_SLICES_BUF_SIZE];
    m_iCurFileLocation  = 0;
    m_iNextFileLocation = 0;
    m_iBufBytesLeft     = 0;
    m_cHandle.seekg( 0, ios_base::beg );
#endif 
  }
  
  return;
}

Void TVideoIOBitsStartCode::closeBits()
{
  m_cHandle.close();
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
  if (m_ucFastLookupBuf!=NULL) delete [] m_ucFastLookupBuf;
#endif 
}

/**
 \param  rpcBitstream    bitstream class pointer
 \retval                 true if EOF is reached
 */
Bool TVideoIOBitsStartCode::readBits( TComBitstream*& rpcBitstream )
{
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
  m_bLastSliceEncounteredInPicture = true;
#endif
  rpcBitstream->rewindStreamPacket();
  
  // check end-of-file
  if ( m_cHandle.eof() ) return true;
  
  UInt uiPacketSize = 0;
  if( 0 != xFindNextStartCode( uiPacketSize, reinterpret_cast<UChar*>(rpcBitstream->getBuffer()) ) )
  {
    return true;
  }
  
#if AD_HOC_SLICES
  rpcBitstream->setFirstSliceEncounteredInPicture( m_bFirstSliceEncounteredInPicture );
  rpcBitstream->setLastSliceEncounteredInPicture ( m_bLastSliceEncounteredInPicture  );
#endif
  // initialize parsing process
  rpcBitstream->initParsingConvertPayloadToRBSP( uiPacketSize );
  return false;
}

#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
Int TVideoIOBitsStartCode::findNextStartCodeFastLookup( TComBitstream*& rpcBitstream )
{
  m_bLastSliceEncounteredInPicture = true;
  if (m_iBufBytesLeft<8)
  {
    // put some data in the buffer
    m_cHandle.seekg( m_iNextFileLocation, ios_base::beg );
    m_cHandle.seekg(-m_iBufBytesLeft, ios_base::cur);
    if ( m_ulBitstreamLength < AD_HOC_SLICES_BUF_SIZE )
    {
      m_iBufBytesLeft = m_ulBitstreamLength;
    }
    else
    {
      m_iBufBytesLeft = AD_HOC_SLICES_BUF_SIZE;
    }
    if (m_iBufBytesLeft < 8) 
      return -1;
    m_cHandle.read( reinterpret_cast<char*>(m_ucFastLookupBuf), m_iBufBytesLeft );
    m_ulBitstreamLength -= m_iBufBytesLeft;
    m_ucCurBufPtr        = m_ucFastLookupBuf;
    m_iNextFileLocation  = m_cHandle.tellg();
  }

  
  assert( 0 == m_ucCurBufPtr[0] );
  assert( 0 == m_ucCurBufPtr[1] );
  assert( 0 == m_ucCurBufPtr[2] );
  assert( 1 == m_ucCurBufPtr[3] );
  m_ucCurBufPtr      += 4;
  m_iCurFileLocation += 4;
  m_iBufBytesLeft    -= 4;

#if SHARP_ENTROPY_SLICE
  m_bEntropySlice = false; 
#endif

  // Peek ahead and check if the new NALU contains a slice with a POC different from one encountered previously
  UInt uiCurPOC = UInt(-1);
  Int iNalUnitType = (m_ucCurBufPtr[0] & 0x1F); 
  if ( iNalUnitType == NAL_UNIT_CODED_SLICE || iNalUnitType == NAL_UNIT_CODED_SLICE_IDR || iNalUnitType == NAL_UNIT_CODED_SLICE_CDR )
  {
#if SHARP_ENTROPY_SLICE 
    if ( (m_ucCurBufPtr[1] & 0x80) != 0 ) 
    {
      // Entropy slice detected
      m_bEntropySlice = true;
    }
    else
    {
      uiCurPOC   = m_ucCurBufPtr[1] & 0x7F;
      uiCurPOC <<= 3;
      uiCurPOC |= ((m_ucCurBufPtr[2] & 0xE0) >> 5);
      m_ucCurBufPtr      += 3;
      m_iCurFileLocation += 3;
      m_iBufBytesLeft    -= 3;      
    }
  }

  if (!m_bEntropySlice && m_uiLastPOC!=uiCurPOC)
  {
    m_bFirstSliceEncounteredInPicture = true;
    m_uiLastPOC = uiCurPOC;
  }
  else
  {
    m_bFirstSliceEncounteredInPicture = false;
    uiCurPOC = m_uiLastPOC;
  }
#else
    uiCurPOC   = m_ucCurBufPtr[1] & 0xFF;
    uiCurPOC <<= 2;
    uiCurPOC  |= ((m_ucCurBufPtr[2] & 0xC0) >> 6);
    m_ucCurBufPtr      += 3;
    m_iCurFileLocation += 3;
    m_iBufBytesLeft    -= 3;
  }
  m_uiLastPOC         = uiCurPOC;
  if (m_uiLastPOC!=uiCurPOC)
  {
    m_bFirstSliceEncounteredInPicture = true;
  }
  else
  {
    m_bFirstSliceEncounteredInPicture = false;
  }
#endif

  Int  iNextStartCodeBytes = 0;
  UInt uiZeros             = 0;
  while( true )
  {
    UChar ucByte = (*m_ucCurBufPtr);
    m_ucCurBufPtr++; m_iBufBytesLeft--; m_iCurFileLocation++;
    if ( m_iBufBytesLeft==0 )
    {
      if ( m_ulBitstreamLength==0 ) 
      {
        iNextStartCodeBytes = 0;
        m_bLastSliceEncounteredInPicture = true;
        m_iCurFileLocation = -1;
        break;   
      }    

      // put some data in the buffer
      m_cHandle.seekg( m_iNextFileLocation, ios_base::beg );
      if ( m_ulBitstreamLength < AD_HOC_SLICES_BUF_SIZE )
      {
        m_iBufBytesLeft = m_ulBitstreamLength;
      }
      else
      {
        m_iBufBytesLeft = AD_HOC_SLICES_BUF_SIZE;
      }
      m_cHandle.read( reinterpret_cast<char*>(m_ucFastLookupBuf), m_iBufBytesLeft );
      m_iNextFileLocation  = m_cHandle.tellg();
      m_ulBitstreamLength -= m_iBufBytesLeft;
      m_ucCurBufPtr        = m_ucFastLookupBuf;      
    }
    if( 1 < ucByte )
    {
      uiZeros = 0;
    }
    else if( 0 == ucByte )
    {
      uiZeros++;
    }
    else if( uiZeros > 2)
    {
      iNextStartCodeBytes = 3 + 1;
      // look ahead if it is not end of stream
      Int iLookaheadBytesRead = 0;
      if ( !(m_iBufBytesLeft==0 && m_ulBitstreamLength==0) )
      {
        if (m_iBufBytesLeft<3)
        {
          // include the start code read so far into the buffer in case a move backwards is required
          m_iNextFileLocation -= 5;
          m_ulBitstreamLength += 5;
          
          // put some data in the buffer
          m_cHandle.seekg( m_iNextFileLocation, ios_base::beg );
          if ( m_ulBitstreamLength < AD_HOC_SLICES_BUF_SIZE )
          {
            m_iBufBytesLeft = m_ulBitstreamLength;
          }
          else
          {
            m_iBufBytesLeft = AD_HOC_SLICES_BUF_SIZE;
          }
          if (m_iBufBytesLeft < 3)
          {          
            m_bLastSliceEncounteredInPicture = true;
            return -1;
          }
          m_cHandle.read( reinterpret_cast<char*>(m_ucFastLookupBuf), m_iBufBytesLeft );
          m_ulBitstreamLength -= m_iBufBytesLeft;
          m_ucCurBufPtr        = m_ucFastLookupBuf;
          m_iNextFileLocation  = m_cHandle.tellg();
          assert(m_ucCurBufPtr[0] == 0);
          assert(m_ucCurBufPtr[1] == 0);
          assert(m_ucCurBufPtr[2] == 0);
          assert(m_ucCurBufPtr[3] != 0);
          m_ucCurBufPtr       += 4;
          m_iBufBytesLeft     -= 4;                              
        }       
        ucByte = (*m_ucCurBufPtr);
        m_ucCurBufPtr++; m_iBufBytesLeft--; m_iCurFileLocation++; iLookaheadBytesRead++;
        iNalUnitType = (ucByte & 0x1F); 
        if ( iNalUnitType == NAL_UNIT_CODED_SLICE || iNalUnitType == NAL_UNIT_CODED_SLICE_IDR || iNalUnitType == NAL_UNIT_CODED_SLICE_CDR )
        {
          ucByte = (*m_ucCurBufPtr);
          m_ucCurBufPtr++; m_iBufBytesLeft--; m_iCurFileLocation++;
          if ( m_iBufBytesLeft==0 && m_ulBitstreamLength==0 ) return -1;  
          iLookaheadBytesRead++;
#if SHARP_ENTROPY_SLICE
          if ( (ucByte & 0x80) != 0 ) 
          {
            // Entropy slice detected
            m_bLastSliceEncounteredInPicture = false;
            m_ucCurBufPtr      -= iLookaheadBytesRead;
            m_iCurFileLocation -= iLookaheadBytesRead;
            m_iBufBytesLeft    += iLookaheadBytesRead; 
            break;
          }
          else
          {
            UInt uiReadPOC   = ucByte & 0x7F;
            uiReadPOC <<= 3;
            ucByte = (*m_ucCurBufPtr);
            m_ucCurBufPtr++; m_iBufBytesLeft--; m_iCurFileLocation++;
            if ( m_iBufBytesLeft==0 && m_ulBitstreamLength==0 ) return -1;  
            iLookaheadBytesRead++;
            uiReadPOC |= ((ucByte & 0xE0) >> 5);
#else
          UInt uiReadPOC   = ucByte & 0xFF;
          uiReadPOC <<= 2;
          ucByte = (*m_ucCurBufPtr);
          m_ucCurBufPtr++; m_iBufBytesLeft--; m_iCurFileLocation++;
          if ( m_iBufBytesLeft==0 && m_ulBitstreamLength==0 ) return -1;  
          iLookaheadBytesRead++;
          uiReadPOC |= ((ucByte & 0xC0) >> 6);
#endif
          if ( uiReadPOC == uiCurPOC )
          {
            m_bLastSliceEncounteredInPicture = false;
            m_ucCurBufPtr      -= iLookaheadBytesRead;
            m_iCurFileLocation -= iLookaheadBytesRead;
            m_iBufBytesLeft    += iLookaheadBytesRead;            
            break;
          }
          else
          {
            m_bLastSliceEncounteredInPicture = true; // a new POC detected so signal that this is the last Slice/NALU for current picture.
          }
#if SHARP_ENTROPY_SLICE
          }
#endif
        }
      }
      if (iNextStartCodeBytes!=0)
      {
        m_ucCurBufPtr      -= iLookaheadBytesRead;
        m_iCurFileLocation -= iLookaheadBytesRead;
        m_iBufBytesLeft    += iLookaheadBytesRead;
        break;
      }
    }
    else
    {
      uiZeros = 0;
    }
  }

  m_ucCurBufPtr       -= iNextStartCodeBytes;
  m_iCurFileLocation  -= iNextStartCodeBytes;
  m_iBufBytesLeft     += iNextStartCodeBytes;  

  rpcBitstream->setFirstSliceEncounteredInPicture( m_bFirstSliceEncounteredInPicture );
  rpcBitstream->setLastSliceEncounteredInPicture ( m_bLastSliceEncounteredInPicture  );

  return m_iCurFileLocation;
}
#endif

int TVideoIOBitsStartCode::xFindNextStartCode(UInt& ruiPacketSize, UChar* pucBuffer)
{
  UInt uiDummy = 0;
  m_cHandle.read( reinterpret_cast<char*>(&uiDummy), 3 );
  if ( m_cHandle.eof() ) return -1;
  assert( 0 == uiDummy );
  
  m_cHandle.read( reinterpret_cast<char*>(&uiDummy), 1 );
  if ( m_cHandle.eof() ) return -1;
  assert( 1 == uiDummy );
  
#if AD_HOC_SLICES 
  UInt uiCurPOC = UInt(-1);
  UChar ucDummyByte;

#if SHARP_ENTROPY_SLICE
  m_bEntropySlice = false; 
#endif

  // Peek ahead and check if the new NALU contains a slice with a POC different from one encountered previously
  Int  iMovedForwardBytes = 0;
  m_cHandle.read( reinterpret_cast<char*>(&ucDummyByte), 1 );
  if ( m_cHandle.eof() ) return -1;  
  iMovedForwardBytes++;
  Int iNalUnitType = (ucDummyByte & 0x1F); 
  if ( iNalUnitType == NAL_UNIT_CODED_SLICE || iNalUnitType == NAL_UNIT_CODED_SLICE_IDR || iNalUnitType == NAL_UNIT_CODED_SLICE_CDR )
  {
    m_cHandle.read( reinterpret_cast<char*>(&ucDummyByte), 1 );
    if ( m_cHandle.eof() ) return -1;  
    iMovedForwardBytes++;
#if SHARP_ENTROPY_SLICE 
    if ( (ucDummyByte & 0x80) != 0 ) 
    {
      // Entropy slice detected
      m_bEntropySlice = true;
    }
    else
    {
      uiCurPOC        = ucDummyByte & 0x7F;
      uiCurPOC      <<= 3;
      m_cHandle.read( reinterpret_cast<char*>(&ucDummyByte), 1 );
      if ( m_cHandle.eof() ) return -1;  
      iMovedForwardBytes++;
      uiCurPOC |= ((ucDummyByte & 0xE0) >> 5);
    }
  }
  m_cHandle.seekg(-iMovedForwardBytes, ios_base::cur);

  if (!m_bEntropySlice && m_uiLastPOC!=uiCurPOC)
  {
    m_bFirstSliceEncounteredInPicture = true;
    m_uiLastPOC = uiCurPOC;
  }
  else
  {
    m_bFirstSliceEncounteredInPicture = false;
    uiCurPOC = m_uiLastPOC;
  }
#else
    uiCurPOC   = ucDummyByte & 0xFF;
    uiCurPOC <<= 2;
    m_cHandle.read( reinterpret_cast<char*>(&ucDummyByte), 1 );
    if ( m_cHandle.eof() ) return -1;  
    iMovedForwardBytes++;
    uiCurPOC |= ((ucDummyByte & 0xC0) >> 6);
  }
  m_cHandle.seekg(-iMovedForwardBytes, ios_base::cur);

  if (m_uiLastPOC!=uiCurPOC)
  {
    m_bFirstSliceEncounteredInPicture = true;
  }
  else
  {
    m_bFirstSliceEncounteredInPicture = false;
  }
  m_uiLastPOC = uiCurPOC;
#endif
#endif
  Int iNextStartCodeBytes = 0;
  Int iBytesRead = 0;
  UInt uiZeros = 0;
  while( true )
  {
    UChar ucByte = 0;
    m_cHandle.read( reinterpret_cast<char*>(&ucByte), 1 );
    if ( m_cHandle.eof() )
    {
      iNextStartCodeBytes = 0;
#if AD_HOC_SLICES 
      m_bLastSliceEncounteredInPicture = true;
#endif
      break;
    }
    pucBuffer[iBytesRead++] = ucByte;
    if( 1 < ucByte )
    {
      uiZeros = 0;
    }
    else if( 0 == ucByte )
    {
      uiZeros++;
    }
    else if( uiZeros > 2)
    {
      iNextStartCodeBytes = 3 + 1;
#if AD_HOC_SLICES
      // look ahead if it is not end of stream
      Int iLookaheadBytesRead = 0;
      if ( !m_cHandle.eof() )
      {
        m_cHandle.read( reinterpret_cast<char*>(&ucByte), 1 );
        iLookaheadBytesRead++;
        iNalUnitType = (ucByte & 0x1F); 
        if ( iNalUnitType == NAL_UNIT_CODED_SLICE || iNalUnitType == NAL_UNIT_CODED_SLICE_IDR || iNalUnitType == NAL_UNIT_CODED_SLICE_CDR )
        {
          m_cHandle.read( reinterpret_cast<char*>(&ucByte), 1 );
          if ( m_cHandle.eof() ) return -1;  
          iLookaheadBytesRead++;
#if SHARP_ENTROPY_SLICE
          if ( (ucByte & 0x80) != 0 ) 
          {
            // Entropy slice detected
            m_bLastSliceEncounteredInPicture = false;
            m_cHandle.seekg(-iLookaheadBytesRead, ios_base::cur);
            break;
          }
          else
          {
            UInt uiReadPOC   = ucByte & 0x7F;
            uiReadPOC <<= 3;
            m_cHandle.read( reinterpret_cast<char*>(&ucByte), 1 );
            if ( m_cHandle.eof() ) return -1;  
            iLookaheadBytesRead++;
            uiReadPOC |= ((ucByte & 0xE0) >> 5);
#else
          UInt uiReadPOC   = ucByte & 0xFF;
          uiReadPOC <<= 2;
          m_cHandle.read( reinterpret_cast<char*>(&ucByte), 1 );
          if ( m_cHandle.eof() ) return -1;  
          iLookaheadBytesRead++;
          uiReadPOC |= ((ucByte & 0xC0) >> 6);
#endif
          if ( uiReadPOC == uiCurPOC )
          {
            m_bLastSliceEncounteredInPicture = false;
            m_cHandle.seekg(-iLookaheadBytesRead, ios_base::cur);
            break;
          }
          else
          {
            m_bLastSliceEncounteredInPicture = true; // a new POC detected so signal that this is the last Slice/NALU for current picture.
          }
#if SHARP_ENTROPY_SLICE
          }
#endif
        }
      }
      if (iNextStartCodeBytes!=0)
      {
        m_cHandle.seekg(-iLookaheadBytesRead, ios_base::cur);
        break;
      }
#else
      break;
#endif
    }
    else
    {
      uiZeros = 0;
    }
  }
  
  ruiPacketSize = iBytesRead - iNextStartCodeBytes;
  
  m_cHandle.seekg( -iNextStartCodeBytes, ios::cur );
  return 0;
}

/**
 \param  pcBitstream   bitstream class pointer
 */
Void TVideoIOBitsStartCode::writeBits( TComBitstream* pcBitstream )
{
  UInt*  plBuff  = pcBitstream->getStartStream();
  UInt   uiBytes = pcBitstream->getNumberOfWrittenBits() >> 3;
  Char   ucZero = 0;
  Char   ucOne = 1;
  
  // write 32-bit packet size
  m_cHandle.write( &ucZero , sizeof(Char) );
  m_cHandle.write( &ucZero , sizeof(Char) );
  m_cHandle.write( &ucZero , sizeof(Char) );
  m_cHandle.write( &ucOne  , sizeof(Char) );
  
  // write packet data
  m_cHandle.write( reinterpret_cast<char*>(plBuff   ), uiBytes      );
}


