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

/** \file     TComAdaptiveLoopFilter.h
    \brief    adaptive loop filter class (header)
*/

#ifndef __TCOMADAPTIVELOOPFILTER__
#define __TCOMADAPTIVELOOPFILTER__

#include "TComPic.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#if HHI_ALF
#define ALF_MAX_NUM_TAP       12                                         ///< maximum number of filter taps (9x9)
#define ALF_MAX_VERT_LENGTH   9
#define ALF_MAX_HORIZ_LENGTH  9
#define ALF_FILT_FOR_CHROMA   2                                         ///< number of filter used for chroma planes
#define ALF_MAX_NUM_COEF      12                                         ///< maximum number of filter coefficients
#define ALF_NUM_BIT_SHIFT     8                                         ///< bit shift parameter for quantization of ALF param.
#else
#define ALF_MAX_NUM_TAP       9                                       ///< maximum number of filter taps (9x9)
#define ALF_MIN_NUM_TAP       5                                       ///< minimum number of filter taps (5x5)
#define ALF_MAX_NUM_TAP_C     5                                       ///< number of filter taps for chroma (5x5)
#define ALF_MAX_NUM_COEF      42                                      ///< maximum number of filter coefficients
#define ALF_MIN_NUM_COEF      14                                      ///< minimum number of filter coefficients
#define ALF_MAX_NUM_COEF_C    14                                      ///< number of filter taps for chroma
#define ALF_NUM_BIT_SHIFT     8                                       ///< bit shift parameter for quantization of ALF param.
#define ALF_ROUND_OFFSET      ( 1 << ( ALF_NUM_BIT_SHIFT - 1 ) )      ///< rounding offset for ALF quantization
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if HHI_ALF
/// adaptive loop filter class
class TComAdaptiveLoopFilter
{
protected:

  // temporary picture buffer
  TComPicYuv*   m_pcTempPicYuv;                                         ///< temporary picture buffer for ALF processing


  // ------------------------------------------------------------------------------------------------------------------
  // For luma component
  // ------------------------------------------------------------------------------------------------------------------

  /// ALF for luma component
  Void  xALFLuma          ( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicRest );
  Void  xALFFilter        ( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicRest, Int iPlane);

  /// sub function: CU-adaptive ALF process for luma
  Void  xCUAdaptive       ( TComPicSym*  pcPicSym, AlfFilter& rcFilter, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void  xSubCUAdaptive    ( TComPicSym* pcQuadTree, TComDataCU*  pcCU    , AlfFilter& rcFilter, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt uiAbsPartIdx, UInt uiDepth);
  Void  xCopyDecToRestCUs ( TComPicSym*  pcQuadTree, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest);
  Void  xCopyDecToRestCU  ( TComPicSym* pcQuadTree, TComDataCU*  pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest);
  /// sub function: non-adaptive ALF process for luma
  Void  xApplyFrame       ( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, AlfFilter& rcFilter );
  Void  xApplyFrame       ( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, AlfFilter& rcFilter, Int iPlane );

  Void  xApplyFilter      ( Pel* pDec, Pel* pRest, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride, AlfFilter& rcFilter );
  Void  xApplyFilter      ( Pel* pDec, Pel* pRest, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride, AlfFilter& rcFilter,Int iOffsetX, Int iOffsetY, Int iMaxX, Int iMaxY );
  // ------------------------------------------------------------------------------------------------------------------
  // For chroma component
  // ------------------------------------------------------------------------------------------------------------------

  /// ALF for chroma component
  Void  xALFChroma      ( ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );

  /// sub function: non-adaptive ALF process for chroma
  Void  xFrameChroma    ( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, ALFParam* pcAlfParam , Int iColor );

  Void xFillAlfFilterInitParam ( AlfFilter& rcFilter , Int iLength , Int iSymmetry );

public:
  TComAdaptiveLoopFilter();
  virtual ~TComAdaptiveLoopFilter() {}


  // initialize & destory temporary buffer
  Void  create                  ( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void  destroy                 ();

  // alloc & free & set functions
  Void  allocALFParam           ( ALFParam* pAlfParam );
  Void  freeALFParam            ( ALFParam* pAlfParam );
  Void  copyALFParam            ( ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam );
  Void  destroyQuadTree         ( ALFParam* pcAlfParam );
  Void  copyALFFilter           ( AlfFilter& rDesAlfFilter, AlfFilter& rSrcAlfFilter) ;

  // predict filter coefficients
  Void  xPredictALFCoeff        ( ALFParam* pAlfParam) ;
  Void  predictALFCoeff         ( ALFParam* pAlfParam );                  ///< prediction of luma ALF coefficients
  Void  predictALFCoeffChroma   ( ALFParam* pAlfParam );                  ///< prediction of chroma ALF coefficients

  // interface function
  Void  ALFProcess              ( TComPic* pcPic, ALFParam* pcAlfParam ); ///< interface function for ALF process
};
#else
/// adaptive loop filter class
class TComAdaptiveLoopFilter
{
protected:
  // quantized filter coefficients
  static const	Int m_aiSymmetricMag9x9[41];														///< quantization scaling factor for 9x9 filter
  static const	Int m_aiSymmetricMag7x7[25];														///< quantization scaling factor for 7x7 filter
  static const	Int m_aiSymmetricMag5x5[13];														///< quantization scaling factor for 5x5 filter

	// temporary picture buffer
	TComPicYuv*		m_pcTempPicYuv;																					///< temporary picture buffer for ALF processing

	// ------------------------------------------------------------------------------------------------------------------
	// For luma component
	// ------------------------------------------------------------------------------------------------------------------

	/// ALF for luma component
  Void	xALFLuma				( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );

	/// sub function: CU-adaptive ALF process for luma
  Void	xCUAdaptive			( TComPic*	  pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void	xSubCUAdaptive	( TComDataCU* pcCU,  ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest,
													UInt uiAbsPartIdx, UInt uiDepth );
  Void	xSubCUFilter		( Pel* pDec, Pel* pRest, Int *qh, Int iTap, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride );

	/// sub function: non-adaptive ALF process for luma
  Void	xFrame					( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap );

	// ------------------------------------------------------------------------------------------------------------------
	// For chroma component
	// ------------------------------------------------------------------------------------------------------------------

	/// ALF for chroma component
  Void	xALFChroma			( ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );

	/// sub function: non-adaptive ALF process for chroma
  Void	xFrameChroma		( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor );

public:
	TComAdaptiveLoopFilter();
	virtual ~TComAdaptiveLoopFilter() {}

	// initialize & destory temporary buffer
	Void	create									( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
	Void	destroy									();

	// alloc & free & set functions
  Void	allocALFParam						( ALFParam* pAlfParam );
  Void	freeALFParam						( ALFParam* pAlfParam );
  Void	copyALFParam						( ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam );

	// predict filter coefficients
  Void	predictALFCoeff					( ALFParam* pAlfParam );									///< prediction of luma ALF coefficients
  Void	predictALFCoeffChroma		( ALFParam* pAlfParam );									///< prediction of chroma ALF coefficients

	// interface function
  Void	ALFProcess							( TComPic* pcPic, ALFParam* pcAlfParam );	///< interface function for ALF process
};
#endif
#endif
