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

/** \file     TComPicYuv.cpp
    \brief    picture YUV buffer class
*/

#include <cstdlib>
#include <assert.h>
#include <memory.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include "TComPicYuv.h"

TComPicYuv::TComPicYuv()
{
  m_apiPicBufY      = NULL;   // Buffer (including margin)
  m_apiPicBufU      = NULL;
  m_apiPicBufV      = NULL;
  
  m_piPicOrgY       = NULL;    // m_apiPicBufY + m_iMarginLuma*getStride() + m_iMarginLuma
  m_piPicOrgU       = NULL;
  m_piPicOrgV       = NULL;
  
  m_bIsBorderExtended = false;
}

TComPicYuv::~TComPicYuv()
{
}

Void TComPicYuv::create( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
  m_iPicWidth       = iPicWidth;
  m_iPicHeight      = iPicHeight;
  
  // --> After config finished!
  m_iCuWidth        = uiMaxCUWidth;
  m_iCuHeight       = uiMaxCUHeight;
  
  m_iNumCuInWidth   = m_iPicWidth / m_iCuWidth;
  m_iNumCuInWidth  += ( m_iPicWidth % m_iCuWidth ) ? 1 : 0;
  
  m_iBaseUnitWidth  = uiMaxCUWidth  >> uiMaxCUDepth;
  m_iBaseUnitHeight = uiMaxCUHeight >> uiMaxCUDepth;
  
  m_iLumaMarginX    = g_uiMaxCUWidth  + 12; // up to 12-tap DIF
  m_iLumaMarginY    = g_uiMaxCUHeight + 12; // up to 12-tap DIF
  
  m_iChromaMarginX  = m_iLumaMarginX>>1;
  m_iChromaMarginY  = m_iLumaMarginY>>1;
  
  m_apiPicBufY      = (Pel*)xMalloc( Pel, ( m_iPicWidth       + (m_iLumaMarginX  <<1)) * ( m_iPicHeight       + (m_iLumaMarginY  <<1)));
  m_apiPicBufU      = (Pel*)xMalloc( Pel, ((m_iPicWidth >> 1) + (m_iChromaMarginX<<1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY<<1)));
  m_apiPicBufV      = (Pel*)xMalloc( Pel, ((m_iPicWidth >> 1) + (m_iChromaMarginX<<1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY<<1)));
  
  m_piPicOrgY       = m_apiPicBufY + m_iLumaMarginY   * getStride()  + m_iLumaMarginX;
  m_piPicOrgU       = m_apiPicBufU + m_iChromaMarginY * getCStride() + m_iChromaMarginX;
  m_piPicOrgV       = m_apiPicBufV + m_iChromaMarginY * getCStride() + m_iChromaMarginX;
  
  m_bIsBorderExtended = false;
  
  return;
}

Void TComPicYuv::destroy()
{
  m_piPicOrgY       = NULL;
  m_piPicOrgU       = NULL;
  m_piPicOrgV       = NULL;
  
  if( m_apiPicBufY ){ xFree( m_apiPicBufY );    m_apiPicBufY = NULL; }
  if( m_apiPicBufU ){ xFree( m_apiPicBufU );    m_apiPicBufU = NULL; }
  if( m_apiPicBufV ){ xFree( m_apiPicBufV );    m_apiPicBufV = NULL; }
}

Void TComPicYuv::createLuma( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
  m_iPicWidth       = iPicWidth;
  m_iPicHeight      = iPicHeight;
  
  // --> After config finished!
  m_iCuWidth        = uiMaxCUWidth;
  m_iCuHeight       = uiMaxCUHeight;
  
  m_iNumCuInWidth   = m_iPicWidth / m_iCuWidth;
  m_iNumCuInWidth  += ( m_iPicWidth % m_iCuWidth ) ? 1 : 0;
  
  m_iBaseUnitWidth  = uiMaxCUWidth  >> uiMaxCUDepth;
  m_iBaseUnitHeight = uiMaxCUHeight >> uiMaxCUDepth;
  
  m_iLumaMarginX    = g_uiMaxCUWidth  + 12; // up to 12-tap DIF
  m_iLumaMarginY    = g_uiMaxCUHeight + 12; // up to 12-tap DIF
  
  m_apiPicBufY      = (Pel*)xMalloc( Pel, ( m_iPicWidth       + (m_iLumaMarginX  <<1)) * ( m_iPicHeight       + (m_iLumaMarginY  <<1)));
  m_piPicOrgY       = m_apiPicBufY + m_iLumaMarginY   * getStride()  + m_iLumaMarginX;
  
  return;
}

Void TComPicYuv::destroyLuma()
{
  m_piPicOrgY       = NULL;
  
  if( m_apiPicBufY ){ xFree( m_apiPicBufY );    m_apiPicBufY = NULL; }
}

Pel*  TComPicYuv::getLumaAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgY + iCuY*m_iCuHeight*getStride() + iCuX*m_iCuWidth );
}

Pel*  TComPicYuv::getLumaAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = iCuY*m_iCuHeight*getStride() + iCuX*m_iCuWidth;
  
  Int iCuSizeInBases = m_iCuWidth / m_iBaseUnitWidth;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx % iCuSizeInBases;
  Int iBaseY         = iRastPartIdx / iCuSizeInBases;
  Int iOffsetBase    = iBaseY*m_iBaseUnitHeight*getStride() + iBaseX*m_iBaseUnitWidth;
  
  return (m_piPicOrgY + iOffsetCu + iOffsetBase);
}

Pel*  TComPicYuv::getCbAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgU + ( ( iCuY*m_iCuHeight*getCStride() + iCuX*m_iCuWidth )>>1 ) );
}

Pel*  TComPicYuv::getCbAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = iCuY*m_iCuHeight*getCStride() + iCuX*m_iCuWidth;
  
  Int iCuSizeInBases = m_iCuWidth / m_iBaseUnitWidth;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx % iCuSizeInBases;
  Int iBaseY         = iRastPartIdx / iCuSizeInBases;
  Int iOffsetBase    = iBaseY*m_iBaseUnitHeight*getCStride() + iBaseX*m_iBaseUnitWidth;
  
  return (m_piPicOrgU + ( ( iOffsetCu + iOffsetBase)>>1 ) );
}

Pel*  TComPicYuv::getCrAddr( int iCuAddr )
{
  Int iCuX = iCuAddr % m_iNumCuInWidth;
  Int iCuY = iCuAddr / m_iNumCuInWidth;
  
  return ( m_piPicOrgV + ( ( iCuY*m_iCuHeight*getCStride() + iCuX*m_iCuWidth )>>1 ) );
}

Pel*  TComPicYuv::getCrAddr( Int iCuAddr, Int uiAbsZorderIdx )
{
  Int iCuX           = iCuAddr % m_iNumCuInWidth;
  Int iCuY           = iCuAddr / m_iNumCuInWidth;
  Int iOffsetCu      = iCuY*m_iCuHeight*getCStride() + iCuX*m_iCuWidth;
  
  Int iCuSizeInBases = m_iCuWidth / m_iBaseUnitWidth;
  Int iRastPartIdx   = g_auiZscanToRaster[uiAbsZorderIdx];
  Int iBaseX         = iRastPartIdx % iCuSizeInBases;
  Int iBaseY         = iRastPartIdx / iCuSizeInBases;
  Int iOffsetBase    = iBaseY*m_iBaseUnitHeight*getCStride() + iBaseX*m_iBaseUnitWidth;
  
  return (m_piPicOrgV + ( ( iOffsetCu + iOffsetBase)>>1 ) );
}

Void  TComPicYuv::copyToPic (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufY(), m_apiPicBufY, sizeof (Pel) * ( m_iPicWidth       + (m_iLumaMarginX   << 1)) * ( m_iPicHeight       + (m_iLumaMarginY   << 1)) );
  ::memcpy ( pcPicYuvDst->getBufU(), m_apiPicBufU, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
  ::memcpy ( pcPicYuvDst->getBufV(), m_apiPicBufV, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
  return;
}

Void  TComPicYuv::copyToPicLuma (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufY(), m_apiPicBufY, sizeof (Pel) * ( m_iPicWidth       + (m_iLumaMarginX   << 1)) * ( m_iPicHeight       + (m_iLumaMarginY   << 1)) );
  return;
}

Void  TComPicYuv::copyToPicCb (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufU(), m_apiPicBufU, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
  return;
}

Void  TComPicYuv::copyToPicCr (TComPicYuv*  pcPicYuvDst)
{
  assert( m_iPicWidth  == pcPicYuvDst->getWidth()  );
  assert( m_iPicHeight == pcPicYuvDst->getHeight() );
  
  ::memcpy ( pcPicYuvDst->getBufV(), m_apiPicBufV, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
  return;
}


Void TComPicYuv::getLumaMinMax( Int *pMin, Int *pMax )
{
  Pel*  piY   = getLumaAddr();
  Int   iMin  = (1<<(g_uiBitDepth))-1;
  Int   iMax  = 0;
  Int   x, y;
  
  for ( y = 0; y < m_iPicHeight; y++ )
  {
    for ( x = 0; x < m_iPicWidth; x++ )
    {
      if ( piY[x] < iMin ) iMin = piY[x];
      if ( piY[x] > iMax ) iMax = piY[x];
    }
    piY += getStride();
  }
  
  *pMin = iMin;
  *pMax = iMax;
}

Void TComPicYuv::extendPicBorder ()
{
  if ( m_bIsBorderExtended ) return;
  
  xExtendPicCompBorder( getLumaAddr(), getStride(),  getWidth(),      getHeight(),      m_iLumaMarginX,   m_iLumaMarginY   );
  xExtendPicCompBorder( getCbAddr()  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMarginX, m_iChromaMarginY );
  xExtendPicCompBorder( getCrAddr()  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMarginX, m_iChromaMarginY );
  
  m_bIsBorderExtended = true;
}

Void TComPicYuv::xExtendPicCompBorder  (Pel* piTxt, Int iStride, Int iWidth, Int iHeight, Int iMarginX, Int iMarginY)
{
  Int   x, y;
  Pel*  pi;
  
  pi = piTxt;
  for ( y = 0; y < iHeight; y++)
  {
    for ( x = 0; x < iMarginX; x++ )
    {
      pi[ -iMarginX + x ] = pi[0];
      pi[    iWidth + x ] = pi[iWidth-1];
    }
    pi += iStride;
  }
  
  pi -= (iStride + iMarginX);
  for ( y = 0; y < iMarginY; y++ )
  {
    ::memcpy( pi + (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
  }
  
  pi -= ((iHeight-1) * iStride);
  for ( y = 0; y < iMarginY; y++ )
  {
    ::memcpy( pi - (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
  }
}


Void TComPicYuv::dump (char* pFileName, Bool bAdd)
{
  FILE* pFile;
  if (!bAdd)
  {
    pFile = fopen (pFileName, "wb");
  }
  else
  {
    pFile = fopen (pFileName, "ab");
  }
  
  Int     shift = g_uiBitIncrement;
  Int     offset = (shift>0)?(1<<(shift-1)):0;
  
  Int   x, y;
  UChar uc;
  
  Pel*  piY   = getLumaAddr();
  Pel*  piCb  = getCbAddr();
  Pel*  piCr  = getCrAddr();
  
  Pel  iMax = ((1<<(g_uiBitDepth))-1);
  
  for ( y = 0; y < m_iPicHeight; y++ )
  {
    for ( x = 0; x < m_iPicWidth; x++ )
    {
      uc = (UChar)Clip3(0, iMax, (piY[x]+offset)>>shift);
      
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piY += getStride();
  }
  
  for ( y = 0; y < m_iPicHeight >> 1; y++ )
  {
    for ( x = 0; x < m_iPicWidth >> 1; x++ )
    {
      uc = (UChar)Clip3(0, iMax, (piCb[x]+offset)>>shift);
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piCb += getCStride();
  }
  
  for ( y = 0; y < m_iPicHeight >> 1; y++ )
  {
    for ( x = 0; x < m_iPicWidth >> 1; x++ )
    {
      uc = (UChar)Clip3(0, iMax, (piCr[x]+offset)>>shift);
      fwrite( &uc, sizeof(UChar), 1, pFile );
    }
    piCr += getCStride();
  }
  
  fclose(pFile);
}

#if FIXED_ROUNDING_FRAME_MEMORY
Void TComPicYuv::xFixedRoundingPic()
{
  Int   x, y;
  Pel*  pRec    = getLumaAddr();
  Int   iStride = getStride();
  Int   iWidth  = getWidth();
  Int   iHeight = getHeight();
#if FULL_NBIT
  Int   iOffset  = ((g_uiBitDepth-8)>0)?(1<<(g_uiBitDepth-8-1)):0;
  Int   iMask   = (~0<<(g_uiBitDepth-8));
  Int   iMaxBdi = g_uiBASE_MAX<<(g_uiBitDepth-8);
#else
  Int   iOffset  = (g_uiBitIncrement>0)?(1<<(g_uiBitIncrement-1)):0;
  Int   iMask   = (~0<<g_uiBitIncrement);
  Int   iMaxBdi = g_uiBASE_MAX<<g_uiBitIncrement;
#endif

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_NOCLIP_RANGE
      pRec[x] = ( pRec[x] + iOffset ) & iMask;
#else
      pRec[x] = ( pRec[x]+iOffset>iMaxBdi)? iMaxBdi : ((pRec[x]+iOffset) & iMask);
#endif
    }
    pRec += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pRec  = getCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_NOCLIP_RANGE
      pRec[x] = ( pRec[x] + iOffset ) & iMask;
#else
      pRec[x] = ( pRec[x]+iOffset>iMaxBdi)? iMaxBdi : ((pRec[x]+iOffset) & iMask);
#endif
    }
    pRec += iStride;
  }

  pRec  = getCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_NOCLIP_RANGE
      pRec[x] = ( pRec[x] + iOffset ) & iMask;
#else
      pRec[x] = ( pRec[x]+iOffset>iMaxBdi)? iMaxBdi : ((pRec[x]+iOffset) & iMask);
#endif
    }
    pRec += iStride;
  }
}
#endif

