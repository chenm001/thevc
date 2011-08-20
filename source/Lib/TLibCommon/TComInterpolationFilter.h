//
//  TComInterpolationFilter.h
//  HM
//
//  Created by Frank Bossen on 4/11/11.
//  Copyright 2011 NTT DOCOMO, Inc. All rights reserved.
//

/**
 * \file
 * \brief Declaration of TComInterpolationFilter class
 */

#ifndef __HM_TCOMINTERPOLATIONFILTER_H__
#define __HM_TCOMINTERPOLATIONFILTER_H__

#include "TypeDef.h"

#if GENERIC_IF

#define NTAPS_LUMA        8 ///< Number of taps for luma
#define NTAPS_CHROMA      4 ///< Number of taps for chroma
#define IF_INTERNAL_PREC 14 ///< Number of bits for internal precision
#define IF_FILTER_PREC    6 ///< Log2 of sum of filter taps
#define IF_INTERNAL_OFFS (1<<(IF_INTERNAL_PREC-1)) ///< Offset used internally

#if IF_LUMA_4TAP
#define NTAPS_LUMA_SHORT 4
#endif
#if IF_CHROMA_2TAP
#define NTAPS_CHROMA_SHORT 2
#endif

/**
 * \brief Interpolation filter class
 */
class TComInterpolationFilter
{
  static const Short m_lumaFilter[4][NTAPS_LUMA];     ///< Luma filter taps
  static const Short m_chromaFilter[8][NTAPS_CHROMA]; ///< Chroma filter taps
#if IF_LUMA_4TAP
  static const Short m_lumaFilterShort[4][NTAPS_LUMA_SHORT];
#endif
#if IF_CHROMA_2TAP
  static const Short m_chromaFilterShort[8][NTAPS_CHROMA_SHORT];
#endif
  
  static Void filterCopy(const Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast);
  
  template<int N, bool isVertical, bool isFirst, bool isLast>
  static Void filter(Pel const *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Short const *coeff);

  template<int N>
  static Void filterHor(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height,               Bool isLast, Short const *coeff);
  template<int N>
  static Void filterVer(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Bool isFirst, Bool isLast, Short const *coeff);

public:
  TComInterpolationFilter() {}
  ~TComInterpolationFilter() {}

  Void filterHorLuma  (Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac,               Bool isLast, Bool isSmallBlock );
  Void filterVerLuma  (Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast, Bool isSmallBlock );
  Void filterHorChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac,               Bool isLast, Bool isSmallBlock );
  Void filterVerChroma(Pel *src, Int srcStride, Short *dst, Int dstStride, Int width, Int height, Int frac, Bool isFirst, Bool isLast, Bool isSmallBlock );
};

#endif // GENERIC_IF
#endif
