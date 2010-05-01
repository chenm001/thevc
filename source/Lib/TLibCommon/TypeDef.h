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

/** \file     TypeDef.h
    \brief    Define basic types, new types and enumerations
*/

#ifndef _TYPEDEF__
#define _TYPEDEF__

// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================

typedef       void								Void;
typedef       bool								Bool;

typedef       char								Char;
typedef       unsigned char				UChar;
typedef       short								Short;
typedef       unsigned short			UShort;
typedef       int									Int;
typedef       unsigned int				UInt;
typedef       long								Long;
typedef       unsigned long				ULong;
typedef       double							Double;

// ====================================================================================================================
// 64-bit integer type
// ====================================================================================================================

#ifdef _MSC_VER
typedef       __int64							Int64;

#if _MSC_VER <= 1200 // MS VC6
typedef       __int64							UInt64;		// MS VC6 does not support unsigned __int64 to double conversion
#else
typedef       unsigned __int64		UInt64;
#endif

#else

typedef       long long						Int64;
typedef       unsigned long long  UInt64;

#endif

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef       UChar           Pxl;				///< 8-bit pixel type
typedef       Short           Pel;				///< 16-bit pixel type
typedef       Int             TCoeff;			///< transform coefficient

/// parameters for adaptive loop filter
typedef struct _AlfParam
{
  Int alf_flag;														///< indicates use of ALF
  Int cu_control_flag;										///< coding unit based control flag
  Int chroma_idc;													///< indicates use of ALF for chroma
  Int tap;																///< number of filter taps
  Int num_coeff;													///< number of filter coefficients
  Int *coeff;															///< filter coefficient array
  Int tap_chroma;													///< number of filter taps (chroma)
  Int num_coeff_chroma;										///< number of filter coefficients (chroma)
  Int *coeff_chroma;											///< filter coefficient array (chroma)
} ALFParam;

/// parameters for deblocking filter
typedef struct _LFCUParam
{
  Bool bInternalEdge;											///< indicates internal edge
  Bool bLeftEdge;													///< indicates left edge
  Bool bTopEdge;													///< indicates top edge
  Bool bLumaEdgeFilter[2][4];							///< array of luma edge decisions
  Int  iBsEdgeSum[2][4];									///< array of Bs edge sum values
} LFCUParam;

// ====================================================================================================================
// Enumeration
// ====================================================================================================================

/// supported slice type
enum SliceType
{
  I_SLICE,
  P_SLICE,
  B_SLICE
};

/// supported partition shape
enum PartSize
{
  SIZE_2Nx2N,						///< symmetric motion partition,  2Nx2N
  SIZE_2NxN,						///< symmetric motion partition,  2Nx N
  SIZE_Nx2N,						///< symmetric motion partition,   Nx2N
  SIZE_NxN,							///< symmetric motion partition,   Nx N

  SIZE_2NxnU,						///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
  SIZE_2NxnD,						///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
  SIZE_nLx2N,						///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
  SIZE_nRx2N,						///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N

  SIZE_NONE = 15
};

/// supported prediction type
enum PredMode
{
  MODE_SKIP,						///< SKIP mode
  MODE_INTER,						///< inter-prediction mode
  MODE_INTRA,						///< intra-prediction mode

  MODE_NONE = 15
};

/// texture component type
enum TextType
{
  TEXT_LUMA,						///< luma
  TEXT_CHROMA,					///< chroma (U+V)
  TEXT_CHROMA_U,				///< chroma U
  TEXT_CHROMA_V,				///< chroma V

  TEXT_NONE = 15
};

/// reference list index
enum RefPicList
{
  REF_PIC_LIST_0 = 0,		///< reference list 0
  REF_PIC_LIST_1 = 1,		///< reference list 1
  REF_PIC_LIST_X = 100	///< special mark
};

/// distortion function index
enum DFunc
{
  DF_DEFAULT  = 0,
	DF_SSE      = 1,			///< general size SSE
  DF_SSE4     = 2,      ///<   4xM SSE
  DF_SSE8     = 3,      ///<   8xM SSE
  DF_SSE16    = 4,      ///<  16xM SSE
  DF_SSE32    = 5,      ///<  32xM SSE
  DF_SSE64    = 6,      ///<  64xM SSE
	DF_SSE16N		= 7,			///< 16NxM SSE

	DF_SAD      = 8,			///< general size SAD
  DF_SAD4     = 9,      ///<   4xM SAD
  DF_SAD8     = 10,     ///<   8xM SAD
  DF_SAD16    = 11,     ///<  16xM SAD
  DF_SAD32    = 12,     ///<  32xM SAD
  DF_SAD64    = 13,     ///<  64xM SAD
	DF_SAD16N	  = 14,			///< 16NxM SAD

	DF_SADS     = 15,			///< general size SAD with step
  DF_SADS4    = 16,     ///<   4xM SAD with step
  DF_SADS8    = 17,     ///<   8xM SAD with step
  DF_SADS16   = 18,     ///<  16xM SAD with step
  DF_SADS32   = 19,     ///<  32xM SAD with step
  DF_SADS64   = 20,     ///<  64xM SAD with step
	DF_SADS16N	= 21,     ///< 16NxM SAD with step

	DF_HADS     = 22,			///< general size Hadamard with step
  DF_HADS4    = 23,     ///<   4xM HAD with step
  DF_HADS8    = 24,     ///<   8xM HAD with step
  DF_HADS16   = 25,     ///<  16xM HAD with step
  DF_HADS32   = 26,     ///<  32xM HAD with step
  DF_HADS64   = 27,     ///<  64xM HAD with step
	DF_HADS16N	= 28,     ///< 16NxM HAD with step

  DF_SSE_FRAME = 33			///< Frame-based SSE
};

/// index for reference type
enum  ERBIndex
{
  ERB_NONE    = 0,			///< normal case
  ERB_LTR     = 1				///< long-term reference
};

/// index for SBAC based RD optimization
enum CI_IDX
{
  CI_CURR_BEST = 0,			///< best mode index
  CI_NEXT_BEST,					///< next best index
  CI_TEMP_BEST,					///< temporal index
  CI_CHROMA_INTRA,			///< chroma intra index
  CI_NUM,								///< total number
};

/// motion vector predictor direction used in AMVP
enum MVP_DIR
{
  MD_LEFT = 0,					///< MVP of left block
  MD_ABOVE,							///< MVP of above block
  MD_ABOVE_RIGHT,				///< MVP of above right block
  MD_BELOW_LEFT,				///< MVP of below left block
  MD_ABOVE_LEFT					///< MVP of above left block
};

/// motion vector prediction mode used in AMVP
enum AMVP_MODE
{
  AM_NONE = 0,					///< no AMVP mode
  AM_EXPL,							///< explicit signalling of motion vector index
};

/// effect mode used in GRF
enum EFF_MODE
{
  EFF_WP_SO = 0,				///< weighted prediction (scale+offset)
  EFF_WP_O,							///< weighted prediction (offset)
  EFF_NONE
};

#endif
