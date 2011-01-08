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

/** \file     TVideoIOYuv.cpp
    \brief    YUV file I/O class
*/

#include <cstdlib>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "TVideoIOYuv.h"

using namespace std;

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 \param pchFile    file name string
 \param bWriteMode file open mode
 */
Void TVideoIOYuv::open( char* pchFile, Bool bWriteMode )
{
  if ( bWriteMode )
  {
    m_cHandle.open( pchFile, ios::binary | ios::out );
    
    if( m_cHandle.fail() )
    {
      printf("\nfailed to write reconstructed YUV file\n");
      exit(0);
    }
  }
  else
  {
    m_cHandle.open( pchFile, ios::binary | ios::in );
    
    if( m_cHandle.fail() )
    {
      printf("\nfailed to open Input YUV file\n");
      exit(0);
    }
  }
  
  return;
}

Void TVideoIOYuv::close()
{
  m_cHandle.close();
}

Bool TVideoIOYuv::isEof()
{
  return m_cHandle.eof();
}

/**
 \param rpcPicYuv      input picture YUV buffer class pointer
 \param aiPad[2]       source padding size, aiPad[0] = horizontal, aiPad[1] = vertical
 */
Void TVideoIOYuv::read ( TComPicYuv*&  rpcPicYuv, Int aiPad[2] )
{
  // check end-of-file
  if ( isEof() ) return;
  
  Int   x, y;
  Int   iWidth, iHeight;
  Int   iStride = rpcPicYuv->getStride();
  
  // compute actual YUV width & height excluding padding size
  iWidth  = rpcPicYuv->getWidth () - aiPad[0];
  iHeight = rpcPicYuv->getHeight() - aiPad[1];
  
  // allocate 8-bit buffer
  Pxl*  apuchBuf = new Pxl[iWidth];
  
  // Y
  Pel*  pDst = rpcPicYuv->getLumaAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    m_cHandle.read( reinterpret_cast<char*>(apuchBuf), sizeof (Pxl) * iWidth );
    for ( x = 0; x < iWidth; x++ ) pDst[x] = (Pel)apuchBuf[x];
    
    // horizontal-right padding
    for ( x = iWidth; x < rpcPicYuv->getWidth(); x++ ) pDst[x] = pDst[x-1];
    pDst += iStride;
  }
  
  // vertial-bottom padding
  for ( y = iHeight; y < rpcPicYuv->getHeight(); y++ )
  {
    for ( x = 0; x < rpcPicYuv->getWidth(); x++ ) pDst[x] = pDst[-iStride+x];
    pDst += iStride;
  }
  
  iWidth   >>= 1;
  iHeight  >>= 1;
  iStride  >>= 1;
  
  //  U
  pDst = rpcPicYuv->getCbAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    m_cHandle.read( reinterpret_cast<char*>(apuchBuf), sizeof (Pxl) * iWidth );
    for ( x = 0; x < iWidth; x++ ) pDst[x] = (Pel)apuchBuf[x];
    
    // horizontal-right padding
    for ( x = iWidth; x < (rpcPicYuv->getWidth()>>1); x++ ) pDst[x] = pDst[x-1];
    pDst += iStride;
  }
  
  // vertical-bottom padding
  for ( y = iHeight; y < (rpcPicYuv->getHeight()>>1); y++ )
  {
    for ( x = 0; x < (rpcPicYuv->getWidth()>>1); x++ ) pDst[x] = pDst[-iStride+x];
    pDst += iStride;
  }
  
  //  V
  pDst = rpcPicYuv->getCrAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    m_cHandle.read( reinterpret_cast<char*>(apuchBuf), sizeof (Pxl) * iWidth );
    for ( x = 0; x < iWidth; x++ ) pDst[x] = (Pel)apuchBuf[x];
    
    // horizontal-right padding
    for ( x = iWidth; x < (rpcPicYuv->getWidth()>>1); x++ ) pDst[x] = pDst[x-1];
    pDst += iStride;
  }
  
  // vertical-bottom padding
  for ( y = iHeight; y < (rpcPicYuv->getHeight()>>1); y++ )
  {
    for ( x = 0; x < (rpcPicYuv->getWidth()>>1); x++ ) pDst[x] = pDst[-iStride+x];
    pDst += iStride;
  }
  
  delete [] apuchBuf;
  
  return;
}

/** \param pcPicYuv     input picture YUV buffer class pointer
 \param aiPad[2]     source padding size, aiPad[0] = horizontal, aiPad[1] = vertical
 */
Void TVideoIOYuv::write( TComPicYuv* pcPicYuv, Int aiPad[2] )
{
  Int   x, y;
  
  // compute actual YUV frame size excluding padding size
  Int   iWidth  = pcPicYuv->getWidth () - aiPad[0];
  Int   iHeight = pcPicYuv->getHeight() - aiPad[1];
  Int   iStride = pcPicYuv->getStride();
  
  // allocate 8-bit buffer
  Pxl*  apuchBuf = new Pxl[iWidth];
  
  //  Y
  Pel*  pSrc = pcPicYuv->getLumaAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    for ( x = 0; x < iWidth; x++ ) apuchBuf[x] = (Pxl)pSrc[x];
    m_cHandle.write(  reinterpret_cast<char*>(apuchBuf), sizeof(Pxl) * iWidth );
    pSrc += iStride;
  }
  
  iWidth   >>= 1;
  iHeight  >>= 1;
  iStride  >>= 1;
  
  //  U
  pSrc = pcPicYuv->getCbAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    for ( x = 0; x < iWidth; x++ ) apuchBuf[x] = (Pxl)pSrc[x];
    m_cHandle.write( reinterpret_cast<char*>(apuchBuf), sizeof(Pxl) * iWidth );
    pSrc += iStride;
  }
  
  //  V
  pSrc = pcPicYuv->getCrAddr();
  for ( y = 0; y < iHeight; y++ )
  {
    for ( x = 0; x < iWidth; x++ ) apuchBuf[x] = (Pxl)pSrc[x];
    m_cHandle.write( reinterpret_cast<char*>(apuchBuf), sizeof(Pxl) * iWidth );
    pSrc += iStride;
  }
  
  delete [] apuchBuf;
}

