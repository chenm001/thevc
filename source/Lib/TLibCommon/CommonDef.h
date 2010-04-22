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

/** \file			CommonDef.h
    \brief		Defines constants, macros and tool parameters
*/

#ifndef __COMMONDEF__
#define __COMMONDEF__

// wjhan@note: this pragma can be used for turning off "signed and unsigned mismatch"
#pragma warning( disable : 4018 )

#include "TypeDef.h"
#include "TComRom.h"

// ====================================================================================================================
// Version information
// ====================================================================================================================

#define NV_VERSION				"NVM_v2.0.0.7"	///< Current software version

// ====================================================================================================================
// Platform information
// ====================================================================================================================

#ifdef __GNUC__
  #define NVM_COMPILEDBY	"[GCC %d.%d.%d]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
  #ifdef __IA64__
    #define NVM_ONARCH		"[on 64-bit] "
  #else
    #define NVM_ONARCH		"[on 32-bit] "
  #endif
#endif

#ifdef __INTEL_COMPILER
  #define NVM_COMPILEDBY	"[ICC %d]", __INTEL_COMPILER
#elif  _MSC_VER
  #define NVM_COMPILEDBY	"[VS %d]", _MSC_VER
#endif

#ifdef _WIN32
  #define NVM_ONOS				"[Windows]"
#elif  __linux
  #define NVM_ONOS				"[Linux]"
#elif  __CYGWIN__
  #define NVM_ONOS				"[Cygwin]"
#endif

#define NVM_BITS					"[%d bit] ", (sizeof(void*) == 8 ? 64 : 32)	///< used for checking 64-bit O/S

// ====================================================================================================================
// Common constants
// ====================================================================================================================

#define _SUMMARY_OUT_								1						///< print-out PSNR results of all slices to summary.txt
#define _SUMMARY_PIC_								1						///< print-out PSNR results for each slice type to summary.txt

#define MAX_REF_PIC_NUM							64
#define MAX_GOP											64					///< max. value of hierarchical GOP size

#define MAX_NUM_REF								  4						///< max. value of multiple reference frames

#define MAX_UINT										0xFFFFFFFFU	///< max. value of unsigned 32-bit integer
#define MAX_INT											2147483647	///< max. value of signed 32-bit integer
#define MAX_DOUBLE									1.7e+308		///< max. value of double-type value

#define MIN_QP											0
#define MAX_QP											51

#define	NOT_VALID										-1

// ====================================================================================================================
// Macro functions
// ====================================================================================================================

#define Max(x, y)										((x)>(y)?(x):(y))																									///< max of (x, y)
#define Min(x, y)										((x)<(y)?(x):(y))																									///< min of (x, y)
#define Median(a,b,c)								((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b))	///< 3-point median
#define Clip(x)                     ( Min(g_uiIBDI_MAX, Max( 0, (x)) ) )															///< clip with bit-depth range
#define ClipMax(x)									( Min(g_uiBASE_MAX, x            ) )															///< clip with max. value
#define Clip3( MinVal, MaxVal, a)   ( ((a)<(MinVal)) ? (MinVal) : (((a)>(MaxVal)) ? (MaxVal) :(a)) )	///< general min/max clip

#define DATA_ALIGN									1																																	///< use 32-bit aligned malloc/free
#if     DATA_ALIGN && _WIN32 && ( _MSC_VER > 1300 )
#define xMalloc( type, len )        _aligned_malloc( sizeof(type)*(len), 32 )
#define xFree( ptr )								_aligned_free  ( ptr )
#else
#define xMalloc( type, len )        malloc   ( sizeof(type)*(len) )
#define xFree( ptr )								free		 ( ptr )
#endif

// ====================================================================================================================
// Bug fixes
// ====================================================================================================================

#define CIP_FIX											1						///< CIPflag context model is not initialized properly
#define ALF_FIX											1						///< very rarely ALF estimation makes divide-by-zero error

// ====================================================================================================================
// Coding tool configuration
// ====================================================================================================================

// GRF: generated reference frame
#define GRF_MAX_NUM_EFF							2  					///< maximum number of effects
#define GRF_MAX_NUM_WEFF   					4  					///< maximum number of wp effects
#define GRF_WP_CHROMA								1						///< weighted prediction of chroma

// GMC: global motion compensation
#define GMC_SIX_TAP									0						///< six tap filter used for half-pel generation of GMC
#define GMC_AFF_ACC									2						///< affine accuracy : default = 2 (2: Half, 4: Quarter pel encoding for parameters)
#define GMC_WRF_ACC									3						///< warping accuracy: default = 3, max = 3 (3: 1/16 pel)

// IQC: inverse quantization correction
#define IQC_ROUND_OFF								0						///< inverse quantization correction for 4x4 and 8x8
#define IQC_ROUND_OFF_L             0						///< inverse quantization correction for sizes greater than 8x8

// ROT: rotational transform
#define ROT_DICT										5						///< intra ROT dictionary size (1, 2, 4, 5, 9)
#define ROT_DICT_INTER		        	1						///< inter ROT dictionary size (1, 2, 4, 5, 9)
#define ROT_TRY_NONZERO_CBP         1						///< try ROT (in encoder) for non-zero cbp case only

// MPI: multi-parameter intra
#define MPI_DICT										2						///< MPI dictionary size (1, 2, 4, 5) / 1 = no search/no overhead

// AMVP: advanced motion vector prediction
#define AMVP_NEIGH_ALL							0						///< whether use all neighbor MVs or only three typical MVs in AMVP
#define AMVP_NEIGH_COL							1						///< use of colocated MB in AMVP
#define AMVP_TEMP_SR								0						///< default AMVP template search range
#define AMVP_TEMP_SIZE							1						///< default size of AMVP template
#define AMVP_MAX_TEMP_SIZE					4						///< max size of AMVP template
#define AMVP_MAX_TEMP_SR						16					///< max size of AMVP template search area

#if AMVP_NEIGH_ALL
#define AMVP_MAX_NUM_CANDS					70
#else
  #if AMVP_NEIGH_COL
	#define AMVP_MAX_NUM_CANDS				5
  #else
	#define AMVP_MAX_NUM_CANDS				4
  #endif
#endif

// EXC: extreme correction
#define EXC_CORR_LIMIT              1						///< correlation value limit for EXC
#define EXC_NO_EC_BSLICE            4						///< block size limit for EXC in B-slice

// EXB: band extreme correction
#define EXB_NB											16					///< number of bands in EXB
#define EXB_BITS										4						///< 2^EXB_BITS = EXB_NB
#define EXB_CUT											1

// CCCP: color-component-correlation-based prediction
#define CCCP_ADI_MODE								2						///< ADI mode index assigned for CCCP
#define CCCP_MODE										0						///< mode index assigned for CCCP
#define CCCP_BOUNDARY_EXT						2						///< block boundary extension size of neighboring pixels, should be even, default = 2

// TMI: template matching intra
#define TMI_AVERAGE									1						///< use of average mode in template matching

// HAM: high-accuracy motion
#define HAM_ACC											12					///< MV accuracy for HAM
#define HAM_ANC_ACC									(HAM_ACC>>2)///< anchor MV accuracy for HAM
#define HAM_ZEROMV_REP							1						///< mix Hor and Ver
#define HAM_BLACKBOX								1						///< 1: determines optimal Additional MV, 0: tries 9 possible additioanl MV
#define HAM_OVH_OPT									0						///< overhead optimization for (-1, 0, 1) case
#define HAM_RANGE										1						///< range value for overhead of HAM

// CADR: context-adaptive dynamic range
#define CADR_BITS										8																												///< base bit-depth in CADR
#define CADR_OFFSET									( 1 << (   CADR_BITS-1 ) )															///< rounding offset of CADR
#define CADR_FWD_L(x, A, B)					(  (((x)-(A))<<CADR_BITS) / (B) )												///< forward conversion of CADR (luma)
#define CADR_FWD_C(x, A, B)					(  (((x)-(A))<<CADR_BITS) / (B) + (A) )									///< forward conversion of CADR (chroma)
#define CADR_INV_L(y, A, B)					(  (((y)     *(B)+CADR_OFFSET )>>CADR_BITS) + (A) )			///< inverse conversion of CADR (luma)
#define CADR_INV_C(y, A, B)					( ((((y)-(A))*(B)+0           )>>CADR_BITS) + (A) )			///< inverse conversion of CADR (chroma)
#define CADR_SCALE(x, B)						(  ( (x)                       <<CADR_BITS) / (B) )			///< scaling function of CADR
#define CADR_DESCALE(y, B)					(  (((y)     *(B)+CADR_OFFSET )>>CADR_BITS)       )			///< descaling function of CADR for SAD
#define CADR_DESCALE2(y, B)					(   ((y)*(B)*(B)/(1<<(2*CADR_BITS)))              )			///< descaling function of CADR for SSE
#define CADR_DIST										1																												///< modify distortion function with CADR
#define CADR_DERIVE_BT709						0																												///< try to follow BT.709

// CIP: combined intra prediction
#define CIP_ADAPTIVE								1																												///< adaptive use of CIP
#define CIP_BITS										5																												///< CIP accuracy
#define CIP_WEIGHT									16																											///< weighting factor of CIP
#define CIP_MAX											( 1<<(CIP_BITS  ) )																			///< max value of CIP
#define CIP_OFFSET									( 1<<(CIP_BITS-1) )																			///< rounding offset of CIP
#define CIP_PRED(A, B, C)           (((A)*2 + (B)*2 + (C)*2 + 3)/6)													///< CIP weighting function
#define CIP_WSUM(A, B, W)						(((A)*(W) + (CIP_MAX-(W))*(B) + CIP_OFFSET)>>CIP_BITS)	///< weighted average of CIP

// Reference memory management
#define DYN_REF_FREE                0						///< dynamic free of reference memories

// Explicit temporal layer QP offset
#define MAX_TLAYER									4						///< max number of temporal layer
#define HB_LAMBDA_FOR_LDC						1						///< use of B-style lambda for non-key pictures in low-delay mode

// Fast estimation of generalized B in low-delay mode
#define GPB_SIMPLE									1						///< Simple GPB mode
#if     GPB_SIMPLE
#define GPB_SIMPLE_UNI							1						///< Simple mode for uni-direction
#endif

// Fast ME using smoother MV assumption
#define FASTME_SMOOTHER_MV					1						///< reduce ME time using faster option

// Adaptive search range depending on POC difference
#define ADAPT_SR_SCALE							1 					///< division factor for adaptive search range

#endif // end of #ifndef	__COMMONDEF__
