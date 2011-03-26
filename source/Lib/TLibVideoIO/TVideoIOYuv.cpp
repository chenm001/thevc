/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.   
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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

/**
 * Perform division with rounding of all pixels in #img by
 * \f$ 2^{#shiftbits} \f$. All pixels are clipped to [minval, maxval]
 *
 * @param stride  distance between vertically adjacent pixels of #img.
 * @param width   width of active area in #img.
 * @param height  height of active area in #img.
 * @param minval  minimum clipping value
 * @param maxval  maximum clipping value
 */
static void invScalePlane(Pel* img, unsigned int stride, unsigned int width, unsigned int height,
                       unsigned int shiftbits, Pel minval, Pel maxval)
{
  Pel offset = 1 << (shiftbits-1);
  for (unsigned int y = 0; y < height; y++)
  {
    for (unsigned int x = 0; x < width; x++)
    {
      Pel val = (img[x] + offset) >> shiftbits;
      img[x] = Clip3(minval, maxval, val);
    }
    img += stride;
  }
}

/**
 * Multiply all pixels in #img by \f$ 2^{#shiftbits} \f$.
 *
 * @param stride  distance between vertically adjacent pixels of #img.
 * @param width   width of active area in #img.
 * @param height  height of active area in #img.
 */
static void scalePlane(Pel* img, unsigned int stride, unsigned int width, unsigned int height,
                       unsigned int shiftbits)
{
  for (unsigned int y = 0; y < height; y++)
  {
    for (unsigned int x = 0; x < width; x++)
    {
      img[x] <<= shiftbits;
    }
    img += stride;
  }
}

/**
 * Scale all pixels in #img depending upon sign of #shiftbits by a factor of
 * \f$ 2^{#shiftbits} \f$.
 *
 * @param stride  distance between vertically adjacent pixels of #img.
 * @param width   width of active area in #img.
 * @param height  height of active area in #img.
 * @param shiftbits if zero, no operation performed
 *                  if > 0, multiply by \f$ 2^{#shiftbits} \f$, see scalePlane()
 *                  if < 0, divide and round by \f$ 2^{#shiftbits} \f$ and clip,
 *                          see invScalePlane().
 * @param minval  minimum clipping value when dividing.
 * @param maxval  maximum clipping value when dividing.
 */
static void scalePlane(Pel* img, unsigned int stride, unsigned int width, unsigned int height,
                       int shiftbits, Pel minval, Pel maxval)
{
  if (shiftbits == 0)
  {
    return;
  }

  if (shiftbits > 0)
  {
    scalePlane(img, stride, width, height, shiftbits);
  }
  else
  {
    invScalePlane(img, stride, width, height, -shiftbits, minval, maxval);
  }
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 * Open file for reading/writing Y'CbCr frames.
 *
 * Frames read/written have bitdepth #fileBitDepth, and are automatically
 * formatted as 8 or 16 bit word values (see TVideoIOYuv::write()).
 *
 * Image data read or written is converted to/from #internalBitDepth
 * (See scalePlane(), TVideoIOYuv::read() and TVideoIOYuv::write() for
 * further details).
 *
 * \param pchFile          file name string
 * \param bWriteMode       file open mode: true=read, false=write
 * \param fileBitDepth     bit-depth of input/output file data.
 * \param internalBitDepth bit-depth to scale image data to/from when reading/writing.
 */
Void TVideoIOYuv::open( char* pchFile, Bool bWriteMode, unsigned int fileBitDepth, unsigned int internalBitDepth )
{
  m_bitdepthShift = internalBitDepth - fileBitDepth;
  m_fileBitdepth = fileBitDepth;

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
 * Read \f$ #width * #height \f$ pixels from #fd into #dst, optionally
 * padding the left and right edges by edge-extension.  Input may be
 * either 8bit or 16bit little-endian lsb-aligned words.
 *
 * @param dst     destination image
 * @param is16bit true if input file carries > 8bit data, false otherwise.
 * @param stride  distance between vertically adjacent pixels of #dst.
 * @param width   width of active area in #dst.
 * @param height  height of active area in #dst.
 * @param pad_x   length of horizontal padding.
 * @param pad_y   length of vertical padding.
 */
static void readPlane(Pel* dst, istream& fd, bool is16bit,
                      unsigned int stride,
                      unsigned int width, unsigned int height,
                      unsigned int pad_x, unsigned int pad_y)
{
  int read_len = width * (is16bit ? 2 : 1);
  unsigned char *buf = new unsigned char[read_len];
  for (int y = 0; y < height; y++)
  {
    fd.read(reinterpret_cast<char*>(buf), read_len);
    if (!is16bit) {
      for (int x = 0; x < width; x++) {
        dst[x] = buf[x];
      }
    }
    else {
      for (int x = 0; x < width; x++) {
        dst[x] = (buf[2*x+1] << 8) | buf[2*x];
      }
    }

    for (int x = width; x < width + pad_x; x++)
    {
      dst[x] = dst[width - 1];
    }
    dst += stride;
  }
  for (int y = height; y < height + pad_y; y++)
  {
    for (int x = width; x < width + pad_x; x++)
    {
      dst[x] = dst[x - stride];
    }
    dst += stride;
  }
  delete[] buf;
}

/**
 * Write \f$ #width * #height \f$ pixels info #fd from #src.
 *
 * @param src     source image
 * @param is16bit true if input file carries > 8bit data, false otherwise.
 * @param stride  distance between vertically adjacent pixels of #src.
 * @param width   width of active area in #src.
 * @param height  height of active area in #src.
 */
static void writePlane(ostream& fd, Pel* src, bool is16bit,
                       unsigned int stride,
                       unsigned int width, unsigned int height)
{
  int write_len = width * (is16bit ? 2 : 1);
  unsigned char *buf = new unsigned char[write_len];
  for (int y = 0; y < height; y++)
  {
    if (!is16bit) 
    {
      for (int x = 0; x < width; x++) 
      {
        buf[x] = (unsigned char) src[x];
      }
    }
    else 
    {
      for (int x = 0; x < width; x++) 
      {
        buf[2*x] = src[x] & 0xff;
        buf[2*x+1] = (src[x] >> 8) & 0xff;
      }
    }

    fd.write(reinterpret_cast<char*>(buf), write_len);
    src += stride;
  }
  delete[] buf;
}

/**
 * Read one Y'CbCr frame, performing any required input scaling to change
 * from the bitdepth of the input file to the internal bit-depth.
 *
 * If a bit-depth reduction is requried, and internalBitdepth >= 8, then
 * the input file is assumed to be ITU-R BT.601/709 compliant, and the
 * resulting data is clipped to the appropriate legal range, as if the
 * file had been provided at the lower-bitdepth compliant to Rec601/709.
 *
 \param rpcPicYuv      input picture YUV buffer class pointer
 \param aiPad[2]       source padding size, aiPad[0] = horizontal, aiPad[1] = vertical
 */
Void TVideoIOYuv::read ( TComPicYuv*&  rpcPicYuv, Int aiPad[2] )
{
  // check end-of-file
  if ( isEof() ) return;
  
  Int   iStride = rpcPicYuv->getStride();
  
  // compute actual YUV width & height excluding padding size
  unsigned int pad_h = aiPad[0];
  unsigned int pad_v = aiPad[1];
  unsigned int width_full = rpcPicYuv->getWidth();
  unsigned int height_full = rpcPicYuv->getHeight();
  unsigned int width  = width_full - pad_h;
  unsigned int height = height_full - pad_v;
  bool is16bit = m_fileBitdepth > 8;

  int desired_bitdepth = m_fileBitdepth + m_bitdepthShift;
  Pel minval = 0;
  Pel maxval = (1 << desired_bitdepth) - 1;
#if CLIP_TO_709_RANGE
  if (m_bitdepthShift < 0 && desired_bitdepth >= 8)
  {
    /* ITU-R BT.709 compliant clipping for converting say 10b to 8b */
    minval = 1 << (desired_bitdepth - 8);
    maxval = (0xff << (desired_bitdepth - 8)) -1;
  }
#endif
  
  readPlane(rpcPicYuv->getLumaAddr(), m_cHandle, is16bit, iStride, width, height, pad_h, pad_v);
  scalePlane(rpcPicYuv->getLumaAddr(), iStride, width_full, height_full, m_bitdepthShift, minval, maxval);

  iStride >>= 1;
  width_full >>= 1;
  height_full >>= 1;
  width >>= 1;
  height >>= 1;
  pad_h >>= 1;
  pad_v >>= 1;

  readPlane(rpcPicYuv->getCbAddr(), m_cHandle, is16bit, iStride, width, height, pad_h, pad_v);
  scalePlane(rpcPicYuv->getCbAddr(), iStride, width_full, height_full, m_bitdepthShift, minval, maxval);

  readPlane(rpcPicYuv->getCrAddr(), m_cHandle, is16bit, iStride, width, height, pad_h, pad_v);
  scalePlane(rpcPicYuv->getCrAddr(), iStride, width_full, height_full, m_bitdepthShift, minval, maxval);
}

/**
 * Write one Y'CbCr frame. No bit-depth conversion is performed, #pcPicYuv is
 * assumed to be at TVideoIO::m_fileBitdepth depth.
 *
 \param pcPicYuv     input picture YUV buffer class pointer
 \param aiPad[2]     source padding size, aiPad[0] = horizontal, aiPad[1] = vertical
 */
Void TVideoIOYuv::write( TComPicYuv* pcPicYuv, Int aiPad[2] )
{
  // compute actual YUV frame size excluding padding size
  Int   iStride = pcPicYuv->getStride();
  unsigned int width  = pcPicYuv->getWidth() - aiPad[0];
  unsigned int height = pcPicYuv->getHeight() - aiPad[1];
  bool is16bit = m_fileBitdepth > 8;
  TComPicYuv *dstPicYuv = NULL;

  if (m_bitdepthShift != 0)
  {
    dstPicYuv = new TComPicYuv;
    dstPicYuv->create( pcPicYuv->getWidth(), pcPicYuv->getHeight(), 1, 1, 0 );
    pcPicYuv->copyToPic(dstPicYuv);

    Pel minval = 0;
    Pel maxval = (1 << m_fileBitdepth) - 1;
#if CLIP_TO_709_RANGE
    if (-m_bitdepthShift < 0 && m_fileBitdepth >= 8)
    {
      /* ITU-R BT.709 compliant clipping for converting say 10b to 8b */
      minval = 1 << (m_fileBitdepth - 8);
      maxval = (0xff << (m_fileBitdepth - 8)) -1;
    }
#endif
    scalePlane(dstPicYuv->getLumaAddr(), dstPicYuv->getStride(), dstPicYuv->getWidth(), dstPicYuv->getHeight(), -m_bitdepthShift, minval, maxval);
    scalePlane(dstPicYuv->getCbAddr(), dstPicYuv->getCStride(), dstPicYuv->getWidth()>>1, dstPicYuv->getHeight()>>1, -m_bitdepthShift, minval, maxval);
    scalePlane(dstPicYuv->getCrAddr(), dstPicYuv->getCStride(), dstPicYuv->getWidth()>>1, dstPicYuv->getHeight()>>1, -m_bitdepthShift, minval, maxval);
  }
  else
  {
    dstPicYuv = pcPicYuv;
  }
  
  writePlane(m_cHandle, dstPicYuv->getLumaAddr(), is16bit, iStride, width, height);

  width >>= 1;
  height >>= 1;
  iStride >>= 1;
  writePlane(m_cHandle, dstPicYuv->getCbAddr(), is16bit, iStride, width, height);
  writePlane(m_cHandle, dstPicYuv->getCrAddr(), is16bit, iStride, width, height);
  
  if (m_bitdepthShift != 0)
  {
    dstPicYuv->destroy();
    delete dstPicYuv;
  }  
}

