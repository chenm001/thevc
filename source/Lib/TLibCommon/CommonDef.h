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

/** \file     CommonDef.h
    \brief    Defines constants, macros and tool parameters
*/

#ifndef __COMMONDEF__
#define __COMMONDEF__

// this pragma can be used for turning off "signed and unsigned mismatch"
#if _MSC_VER > 1000
#pragma warning( disable : 4018 )
#endif // _MSC_VER > 1000
#include "TypeDef.h"
#include "TComRom.h"

// ====================================================================================================================
// Version information
// ====================================================================================================================

#define NV_VERSION        "0.6"                 ///< Current software version

// ====================================================================================================================
// Platform information
// ====================================================================================================================

#ifdef __GNUC__
  #define NVM_COMPILEDBY  "[GCC %d.%d.%d]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
  #ifdef __IA64__
    #define NVM_ONARCH    "[on 64-bit] "
  #else
    #define NVM_ONARCH    "[on 32-bit] "
  #endif
#endif

#ifdef __INTEL_COMPILER
  #define NVM_COMPILEDBY  "[ICC %d]", __INTEL_COMPILER
#elif  _MSC_VER
  #define NVM_COMPILEDBY  "[VS %d]", _MSC_VER
#endif

#ifndef NVM_COMPILEDBY
  #define NVM_COMPILEDBY "[Unk-CXX]"
#endif

#ifdef _WIN32
  #define NVM_ONOS        "[Windows]"
#elif  __linux
  #define NVM_ONOS        "[Linux]"
#elif  __CYGWIN__
  #define NVM_ONOS        "[Cygwin]"
#elif __APPLE__
  #define NVM_ONOS        "[Mac OS X]"
#else
 #define NVM_ONOS "[Unk-OS]"
#endif

#define NVM_BITS          "[%d bit] ", (sizeof(void*) == 8 ? 64 : 32) ///< used for checking 64-bit O/S

#ifndef NULL
#define NULL              0
#endif

// ====================================================================================================================
// Common constants
// ====================================================================================================================

#define _SUMMARY_OUT_               0           ///< print-out PSNR results of all slices to summary.txt
#define _SUMMARY_PIC_               0           ///< print-out PSNR results for each slice type to summary.txt

#define MAX_REF_PIC_NUM             64
#define MAX_GOP                     64          ///< max. value of hierarchical GOP size

#define MAX_NUM_REF                 4           ///< max. value of multiple reference frames

#define MAX_UINT                    0xFFFFFFFFU ///< max. value of unsigned 32-bit integer
#define MAX_INT                     2147483647  ///< max. value of signed 32-bit integer
#define MAX_DOUBLE                  1.7e+308    ///< max. value of double-type value

#define MIN_QP                      0
#define MAX_QP                      51

#define NOT_VALID                   -1

// ====================================================================================================================
// Macro functions
// ====================================================================================================================

#define Max(x, y)                   ((x)>(y)?(x):(y))                                                 ///< max of (x, y)
#define Min(x, y)                   ((x)<(y)?(x):(y))                                                 ///< min of (x, y)
#define Median(a,b,c)               ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b)) ///< 3-point median
#define Clip(x)                     ( Min(g_uiIBDI_MAX, Max( 0, (x)) ) )                              ///< clip with bit-depth range
#define ClipMax(x)                  ( Min(g_uiBASE_MAX, x            ) )                              ///< clip with max. value
#define Clip3( MinVal, MaxVal, a)   ( ((a)<(MinVal)) ? (MinVal) : (((a)>(MaxVal)) ? (MaxVal) :(a)) )  ///< general min/max clip

#define DATA_ALIGN                  1                                                                 ///< use 32-bit aligned malloc/free
#if     DATA_ALIGN && _WIN32 && ( _MSC_VER > 1300 )
#define xMalloc( type, len )        _aligned_malloc( sizeof(type)*(len), 32 )
#define xFree( ptr )                _aligned_free  ( ptr )
#else
#define xMalloc( type, len )        malloc   ( sizeof(type)*(len) )
#define xFree( ptr )                free     ( ptr )
#endif

// ====================================================================================================================
// Bug fixes
// ====================================================================================================================

#define ALF_FIX                     1           ///< very rarely ALF estimation makes divide-by-zero error

// ====================================================================================================================
// Coding tool configuration
// ====================================================================================================================


#ifdef QC_AMVRES
#define AMVRES_ACC										8 					    ///< MV accuracy for AMVRES
#define AMVRES_ACC_IDX_OFFSET				            (-1)
#endif


// GRF: generated reference frame
#define GRF_MAX_NUM_EFF             2           ///< maximum number of effects
#define GRF_MAX_NUM_WEFF            2           ///< maximum number of wp effects
#define GRF_WP_CHROMA               1           ///< weighted prediction of chroma

// ROT: rotational transform
#define ROT_DICT                    5           ///< intra ROT dictionary size (1, 2, 4, 5, 9)
#define ROT_DICT_INTER              1           ///< inter ROT dictionary size (1, 2, 4, 5, 9)
#define ROT_TRY_NONZERO_CBP         1           ///< try ROT (in encoder) for non-zero cbp case only

// AMVP: advanced motion vector prediction
#define AMVP_NEIGH_COL              1           ///< use of colocated MB in AMVP
#define AMVP_MAX_NUM_CANDS          5           ///< max number of final candidates

// CIP: combined intra prediction
#define CIP_ADAPTIVE                1                                                       ///< adaptive use of CIP
#define CIP_BITS                    5                                                       ///< CIP accuracy
#define CIP_WEIGHT                  16                                                      ///< weighting factor of CIP
#define CIP_MAX                     ( 1<<(CIP_BITS  ) )                                     ///< max value of CIP
#define CIP_OFFSET                  ( 1<<(CIP_BITS-1) )                                     ///< rounding offset of CIP
#define CIP_PRED(A, B, C)           (((A)*3 + (B)*3 + (C)*2 + 4)/8)                         ///< CIP weighting function
#define CIP_WSUM(A, B, W)           (((A)*(W) + (CIP_MAX-(W))*(B) + CIP_OFFSET)>>CIP_BITS)  ///< weighted average of CIP

// Reference memory management
#define DYN_REF_FREE                0           ///< dynamic free of reference memories

// Explicit temporal layer QP offset
#define MAX_TLAYER                  4           ///< max number of temporal layer
#define HB_LAMBDA_FOR_LDC           1           ///< use of B-style lambda for non-key pictures in low-delay mode

// Fast estimation of generalized B in low-delay mode
#define GPB_SIMPLE                  1           ///< Simple GPB mode
#if     GPB_SIMPLE
#define GPB_SIMPLE_UNI              1           ///< Simple mode for uni-direction
#endif

// Fast ME using smoother MV assumption
#define FASTME_SMOOTHER_MV          1           ///< reduce ME time using faster option

// Adaptive search range depending on POC difference
#define ADAPT_SR_SCALE              1           ///< division factor for adaptive search range

// IBDI range restriction for skipping clip
#define IBDI_NOCLIP_RANGE           1           ///< restrict max. value after IBDI to skip clip

// entropy coding
#define PIPE_LOW_DELAY_OPTION       1           ///< enable/disable low-delay buffer control for PIPE
#define NUM_V2V_CODERS              12          ///< number of V2V coders for PIPE

// ALF: Adaptive Loop Filter
#define ALF_MIN_LENGTH              3

// VLC texture coding
#define VLC_SIG_RUN                  1            ///< run-coding of sigmap

// Early-skip threshold (encoder)
#define EARLY_SKIP_THRES            1.50        ///< if RD < thres*avg[BestSkipRD]


/* Rounding control */
#define ROUNDING_CONTROL

const int g_iShift8x8    = 7;
const int g_iShift16x16  = 6;
const int g_iShift32x32  = 5;
const int g_iShift64x64  = 4;

/* End of Rounding control */

enum NalRefIdc
{
  NAL_REF_IDC_PRIORITY_LOWEST = 0,
  NAL_REF_IDC_PRIORITY_LOW,
  NAL_REF_IDC_PRIORITY_HIGH,
  NAL_REF_IDC_PRIORITY_HIGHEST
};

enum NalUnitType
{
  NAL_UNIT_UNSPECIFIED_0 = 0,
  NAL_UNIT_CODED_SLICE,
  NAL_UNIT_CODED_SLICE_DATAPART_A,
  NAL_UNIT_CODED_SLICE_DATAPART_B,
  NAL_UNIT_CODED_SLICE_DATAPART_C,
  NAL_UNIT_CODED_SLICE_IDR,
  NAL_UNIT_SEI,
  NAL_UNIT_SPS,
  NAL_UNIT_PPS,
  NAL_UNIT_ACCESS_UNIT_DELIMITER,
  NAL_UNIT_END_OF_SEQUENCE,
  NAL_UNIT_END_OF_STREAM,
  NAL_UNIT_FILLER_DATA,
  NAL_UNIT_RESERVED_13,
  NAL_UNIT_RESERVED_14,
  NAL_UNIT_RESERVED_15,
  NAL_UNIT_RESERVED_16,
  NAL_UNIT_RESERVED_17,
  NAL_UNIT_RESERVED_18,
  NAL_UNIT_RESERVED_19,
  NAL_UNIT_RESERVED_20,
  NAL_UNIT_RESERVED_21,
  NAL_UNIT_RESERVED_22,
  NAL_UNIT_RESERVED_23,
  NAL_UNIT_UNSPECIFIED_24,
  NAL_UNIT_UNSPECIFIED_25,
  NAL_UNIT_UNSPECIFIED_26,
  NAL_UNIT_UNSPECIFIED_27,
  NAL_UNIT_UNSPECIFIED_28,
  NAL_UNIT_UNSPECIFIED_29,
  NAL_UNIT_UNSPECIFIED_30,
  NAL_UNIT_UNSPECIFIED_31,
  NAL_UNIT_INVALID,
};


#if HHI_ALF
typedef _AlfParamHHI ALFParam;
#define ALF_MIN_LENGTH                    3           ///< MS: mimimum allowed filter length
#define ALF_DC_CONSIDERED                 0           ///< MS: consider dc offset in estimation of coefficients
#else
typedef _AlfParam    ALFParam;
#endif

#endif // end of #ifndef  __COMMONDEF__

