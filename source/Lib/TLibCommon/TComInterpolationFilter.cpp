//
//  TComInterpolationFilter.cpp
//  HM
//
//  Created by Frank Bossen on 4/11/11.
//  Copyright 2011 NTT DOCOMO, Inc. All rights reserved.
//

/**
 * \file
 * \brief Implementation of TComInterpolationFilter class
 */

// ====================================================================================================================
// Includes
// ====================================================================================================================

#include "TComRom.h"
#include "TComInterpolationFilter.h"
#include <assert.h>
#if GENERIC_IF

// ====================================================================================================================
// Tables
// ====================================================================================================================

const Short TComInterpolationFilter::m_lumaFilter[4][NTAPS_LUMA] =
{
  {  0, 0,   0, 64,  0,   0, 0,  0 },
#if IF_MOT
  { -1, 3,  -8, 60, 14,  -6, 3, -1 },
  { -1, 4, -11, 40, 40, -11, 4, -1 },
  { -1, 3,  -6, 14, 60,  -8, 3, -1 }
#else
  { -1, 4, -10, 57, 19,  -7, 3, -1 },
  { -1, 4, -11, 40, 40, -11, 4, -1 },
  { -1, 3,  -7, 19, 57, -10, 4, -1 }
#endif
};

const Short TComInterpolationFilter::m_chromaFilter[8][NTAPS_CHROMA] =
{
  {  0, 64,  0,  0 },
  { -3, 60,  8, -1 },
  { -4, 54, 16, -2 },
  { -5, 46, 27, -4 },
  { -4, 36, 36, -4 },
  { -4, 27, 46, -5 },
  { -2, 16, 54, -4 },
  { -1,  8, 60, -3 }
};

#if IF_LUMA_4TAP
const Short TComInterpolationFilter::m_lumaFilterShort[4][NTAPS_LUMA_SHORT] =
{
  {  0, 64,  0,  0 },
  { -4, 54, 16, -2 },
  { -4, 36, 36, -4 },
  { -2, 16, 54, -4 },
};
#endif

#if IF_CHROMA_2TAP
const Short TComInterpolationFilter::m_chromaFilterShort[8][NTAPS_CHROMA_SHORT] =
{
  { 64,  0 },
  { 56,  8 },
  { 48, 16 },
  { 40, 24 },
  { 32, 32 },
  { 24, 40 },
  { 16, 48 },
  {  8, 56 }
};
#endif

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

/**
 * \brief Apply unit FIR filter to a block of samples
 *
 * \param src        Pointer to source samples
 * \param srcStride  Stride of source samples
 * \param dst        Pointer to destination samples
 * \param dstStride  Stride of destination samples
 * \param width      Width of block
 * \param height     Height of block
 * \param isFirst    Flag indicating whether it is the first filtering operation
 * \param isLast     Flag indicating whether it is the last filtering operation
 */
Void TComInterpolationFilter::filterCopy(const Short *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast)
{
  Int row, col;
  
  if ( isFirst == isLast )
  {
    for (row = 0; row < height; row++)
    {
      for (col = 0; col < width; col++)
      {
        dst[col] = src[col];
      }
      
      src += srcStride;
      dst += dstStride;
    }              
  }
  else if ( isFirst )
  {
    Int shift = IF_INTERNAL_PREC - ( g_uiBitDepth + g_uiBitIncrement );
    
    for (row = 0; row < height; row++)
    {
      for (col = 0; col < width; col++)
      {
        Short val = src[col] << shift;
        dst[col] = val - (Short)IF_INTERNAL_OFFS;
      }
      
      src += srcStride;
      dst += dstStride;
    }          
  }
  else
  {
    Int shift = IF_INTERNAL_PREC - ( g_uiBitDepth + g_uiBitIncrement );
    Short offset = IF_INTERNAL_OFFS + (1 << (shift - 1));
    Short maxVal = g_uiIBDI_MAX;
    Short minVal = 0;
    for (row = 0; row < height; row++)
    {
      for (col = 0; col < width; col++)
      {
        Short val = src[ col ];
        val = ( val + offset ) >> shift;
        if (val < minVal) val = minVal;
        if (val > maxVal) val = maxVal;
        dst[col] = val;
      }
      
      src += srcStride;
      dst += dstStride;
    }              
  }
}

/**
 * \brief Apply FIR filter to a block of samples
 *
 * \tparam N          Number of taps
 * \tparam isVertical Flag indicating filtering along vertical direction
 * \tparam isFirst    Flag indicating whether it is the first filtering operation
 * \tparam isLast     Flag indicating whether it is the last filtering operation
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  coeff      Pointer to filter taps
 */
template<int N, bool isVertical, bool isFirst, bool isLast>
Void TComInterpolationFilter::filter(Short const *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Short const *coeff)
{
  Int row, col;
  
  Short c[8];
  c[0] = coeff[0];
  c[1] = coeff[1];
  if ( N >= 4 )
  {
    c[2] = coeff[2];
    c[3] = coeff[3];
  }
  if ( N >= 6 )
  {
    c[4] = coeff[4];
    c[5] = coeff[5];
  }
  if ( N == 8 )
  {
    c[6] = coeff[6];
    c[7] = coeff[7];
  }
  
  Int cStride = ( isVertical ) ? srcStride : 1;
  src -= ( N/2 - 1 ) * cStride;

  Int offset;
  Short maxVal;
  Int headRoom = IF_INTERNAL_PREC - (g_uiBitDepth + g_uiBitIncrement);
  Int shift = IF_FILTER_PREC;
  if ( isLast )
  {
    shift += (isFirst) ? 0 : headRoom;
    offset = 1 << (shift - 1);
    offset += (isFirst) ? 0 : IF_INTERNAL_OFFS << IF_FILTER_PREC;
    maxVal = g_uiIBDI_MAX;
  }
  else
  {
    shift -= (isFirst) ? headRoom : 0;
    offset = (isFirst) ? -IF_INTERNAL_OFFS << shift : 0;
    maxVal = 0;
  }
  
  for (row = 0; row < height; row++)
  {
    for (col = 0; col < width; col++)
    {
      Int sum;
      
      sum  = src[ col + 0 * cStride] * c[0];
      sum += src[ col + 1 * cStride] * c[1];
      if ( N >= 4 )
      {
        sum += src[ col + 2 * cStride] * c[2];
        sum += src[ col + 3 * cStride] * c[3];
      }
      if ( N >= 6 )
      {
        sum += src[ col + 4 * cStride] * c[4];
        sum += src[ col + 5 * cStride] * c[5];
      }
      if ( N == 8 )
      {
        sum += src[ col + 6 * cStride] * c[6];
        sum += src[ col + 7 * cStride] * c[7];        
      }
      
      Short val = ( sum + offset ) >> shift;
      if ( isLast )
      {
        val = ( val < 0 ) ? 0 : val;
        val = ( val > maxVal ) ? maxVal : val;        
      }
      dst[col] = val;
    }
    
    src += srcStride;
    dst += dstStride;
  }    
}

/**
 * \brief Filter a block of samples (horizontal)
 *
 * \tparam N          Number of taps
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  coeff      Pointer to filter taps
 */
template<int N>
Void TComInterpolationFilter::filterHor(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isLast, Short const *coeff)
{
  if ( isLast )
  {
    filter<N, false, true, true>(src, srcStride, dst, dstStride, width, height, coeff);
  }
  else
  {
    filter<N, false, true, false>(src, srcStride, dst, dstStride, width, height, coeff);
  }
}

/**
 * \brief Filter a block of samples (vertical)
 *
 * \tparam N          Number of taps
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  isFirst    Flag indicating whether it is the first filtering operation
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  coeff      Pointer to filter taps
 */
template<int N>
Void TComInterpolationFilter::filterVer(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast, Short const *coeff)
{
  if ( isFirst && isLast )
  {
    filter<N, true, true, true>(src, srcStride, dst, dstStride, width, height, coeff);
  }
  else if ( isFirst && !isLast )
  {
    filter<N, true, true, false>(src, srcStride, dst, dstStride, width, height, coeff);
  }
  else if ( !isFirst && isLast )
  {
    filter<N, true, false, true>(src, srcStride, dst, dstStride, width, height, coeff);
  }
  else
  {
    filter<N, true, false, false>(src, srcStride, dst, dstStride, width, height, coeff);    
  }      
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 * \brief Filter a block of luma samples (horizontal)
 *
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  frac       Fractional sample offset
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  isSmallBlock Flag indicating whether an alternate filter should be used
 */
Void TComInterpolationFilter::filterHorLuma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isLast, Bool isSmallBlock )
{
  assert(frac >= 0 && frac < 4);
  
  if ( frac == 0 )
  {
    filterCopy( src, srcStride, dst, dstStride, width, height, true, isLast );
  }
#if IF_LUMA_4TAP
  else if ( isSmallBlock)
  {
    filterHor<NTAPS_LUMA_SHORT>(src, srcStride, dst, dstStride, width, height, isLast, m_lumaFilterShort[frac]);
  }
#endif
  else
  {
    filterHor<NTAPS_LUMA>(src, srcStride, dst, dstStride, width, height, isLast, m_lumaFilter[frac]);
  }
}

/**
 * \brief Filter a block of luma samples (vertical)
 *
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  frac       Fractional sample offset
 * \param  isFirst    Flag indicating whether it is the first filtering operation
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  isSmallBlock Flag indicating whether an alternate filter should be used
 */
Void TComInterpolationFilter::filterVerLuma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast, Bool isSmallBlock )
{
  assert(frac >= 0 && frac < 4);
  
  if ( frac == 0 )
  {
    filterCopy( src, srcStride, dst, dstStride, width, height, isFirst, isLast );
  }
#if IF_LUMA_4TAP
  else if ( isSmallBlock )
  {
    filterVer<NTAPS_LUMA_SHORT>(src, srcStride, dst, dstStride, width, height, isFirst, isLast, m_lumaFilterShort[frac]);
  }
#endif
  else
  {
    filterVer<NTAPS_LUMA>(src, srcStride, dst, dstStride, width, height, isFirst, isLast, m_lumaFilter[frac]);    
  }
}

/**
 * \brief Filter a block of chroma samples (horizontal)
 *
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  frac       Fractional sample offset
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  isSmallBlock Flag indicating whether an alternate filter should be used
 */
Void TComInterpolationFilter::filterHorChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isLast, Bool isSmallBlock )
{
  assert(frac >= 0 && frac < 8);
  
  if ( frac == 0 )
  {
    filterCopy( src, srcStride, dst, dstStride, width, height, true, isLast );
  }
#if IF_CHROMA_2TAP
  else if ( isSmallBlock )
  {
    filterHor<NTAPS_CHROMA_SHORT>(src, srcStride, dst, dstStride, width, height, isLast, m_chromaFilterShort[frac]);
  }
#endif
  {
    filterHor<NTAPS_CHROMA>(src, srcStride, dst, dstStride, width, height, isLast, m_chromaFilter[frac]);
  }
}

/**
 * \brief Filter a block of chroma samples (vertical)
 *
 * \param  src        Pointer to source samples
 * \param  srcStride  Stride of source samples
 * \param  dst        Pointer to destination samples
 * \param  dstStride  Stride of destination samples
 * \param  width      Width of block
 * \param  height     Height of block
 * \param  frac       Fractional sample offset
 * \param  isFirst    Flag indicating whether it is the first filtering operation
 * \param  isLast     Flag indicating whether it is the last filtering operation
 * \param  isSmallBlock Flag indicating whether an alternate filter should be used
 */
Void TComInterpolationFilter::filterVerChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast, Bool isSmallBlock )
{
  assert(frac >= 0 && frac < 8);
  
  if ( frac == 0 )
  {
    filterCopy( src, srcStride, dst, dstStride, width, height, isFirst, isLast );
  }
#if IF_CHROMA_2TAP
  else if ( isSmallBlock )
  {
    filterVer<NTAPS_CHROMA_SHORT>(src, srcStride, dst, dstStride, width, height, isFirst, isLast, m_chromaFilterShort[frac]);
  }
#endif
  else
  {
    filterVer<NTAPS_CHROMA>(src, srcStride, dst, dstStride, width, height, isFirst, isLast, m_chromaFilter[frac]);    
  }
}

#endif

