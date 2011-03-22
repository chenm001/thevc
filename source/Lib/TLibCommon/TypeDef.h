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

/** \file     TypeDef.h
    \brief    Define basic types, new types and enumerations
*/

#ifndef _TYPEDEF__
#define _TYPEDEF__

////////////////////////////
// HHI defines section start
////////////////////////////
#define HHI_DISABLE_INTER_NxN_SPLIT       1           ///< TN: disable redundant use of pu-mode NxN for CTBs larger 8x8 (inter only)
#define HHI_RMP_SWITCH                    0

// HHI tools
#define HHI_INTERP_FILTER                 1           ///< HL: interpolation filter
#define HHI_MRG                           1           ///< SOPH: inter partition merging
#define HHI_AMVP_OFF                      0           ///< SOPH: Advanced Motion Vector Predictor deactivated
#define HHI_RQT_FORCE_SPLIT_NxN           0           ///< MSHK: force split flags of residual quadtree for NxN PUs such that transform blocks are guaranteed to not span NxN PUs
#define HHI_RQT_FORCE_SPLIT_RECT          0           ///< MSHK: force split flags of residual quadtree for rectangular PUs such that transform blocks are guaranteed to not span rectangular PUs
#define HHI_RQT_INTRA_SPEEDUP             1 // tests one best mode with full rqt
#define HHI_RQT_INTRA_SPEEDUP_MOD         0 // tests two best modes with full rqt

#if HHI_MRG
#define PART_MRG		                      1            // If the number of partitions is two and size > 8, only merging mode is enabled for the first partition & do not code merge_flag for the first partition
#endif

#if HHI_RQT_INTRA_SPEEDUP_MOD && !HHI_RQT_INTRA_SPEEDUP
#error
#endif

#if ( HHI_RQT_FORCE_SPLIT_NxN || HHI_RQT_FORCE_SPLIT_RECT)
#define HHI_RQT_FORCE_SPLIT_ACC2_PU       1
#else
#define HHI_RQT_FORCE_SPLIT_ACC2_PU       0
#endif

//////////////////////////
// HHI defines section end
//////////////////////////


////////////////////////////
// TEN defines section start
////////////////////////////

#define TEN_DIRECTIONAL_INTERP            0           ///< AF: interpolation filter

#define LCEC_STAT                         0           // LCEC - support for LCEC bitusage statistics
//////////////////////////
// TEN defines section end
//////////////////////////


//////////////////////////////
// Nokia defines section start
//////////////////////////////

#define HIGH_ACCURACY_BI                  1          // High precision bi-prediction JCTVC-D321
#define FIX_ISSUE_125                     1

//////////////////////////////
// Nokia defines section end
//////////////////////////////


/////////////////////////////////
// QUALCOMM defines section start
/////////////////////////////////

#define LCEC_CBP_YUV_ROOT                 1           // enable VLC phase-2 CBP root coding under RQT
#define QC_BLK_CBP                        1           // block level CBP coding, to be enabled only when LCEC_CBP_YUV_ROOT is enabled
#if LCEC_CBP_YUV_ROOT==0 && QC_BLK_CBP
#error
#endif

#define QC_MOD_LCEC                       1           // JCTVC-D374: modified LCEC coeff. coding
#define LCEC_INTRA_MODE                   1           // JCTVC-D366: improved luma intra mode coding
#define QC_MOD_LCEC_RDOQ                  1           // JCTVC-D374: improved RDOQ
#define QC_LCEC_INTER_MODE                1
#define QC_MDIS                           1           // JCTVC-D282: enable mode dependent intra smoothing
#define QC_MDCS                           1           // JCTVC-D393: mode dependent coefficients coding 
#define LCEC_CBP_YUV_ROOT_RDFIX           1           // JCTVC-D375: RD fix associated with LCEC_CBP_YUV_ROOT

#define ENABLE_FORCECOEFF0  0

/* Rounding control */
#define ROUNDING_CONTROL_BIPRED ///< From JCTVC-B074
#define TRANS_PRECISION_EXT     ///< From JCTVC-B074

///////////////////////////////
// QUALCOMM defines section end
///////////////////////////////

///////////////////////////////
// SAMSUNG defines section start
///////////////////////////////
#define HHI_RQT_DISABLE_SUB                   0           ///< disabling subtree whose node size is smaller than partition size

#if HHI_MRG
#define SAMSUNG_MRG_SKIP_DIRECT               1           ///< enabling of skip and direct when mrg is on
#endif

#define HHI_DISABLE_SCAN                      0           ///< disable adaptive scan

#define FAST_UDI_MAX_RDMODE_NUM               10          ///< maximum number of RD comparison in fast-UDI estimation loop 

#define SAMSUNG_FAST_UDI                      1           ///< improved mode decision for UDI (JCTVC-C207)
#if     SAMSUNG_FAST_UDI                           
#define SAMSUNG_FAST_UDI_MODESET              0           ///< 0: {9,9,4,4,5} (default) and 1: {9,9,9,9,5} for {4x4,8x8,16x16,32x32,64x64} 
#endif

#define DCTIF_8_6_LUMA                        1
#define DCTIF_4_6_CHROMA                      1
#if DCTIF_8_6_LUMA && TEN_DIRECTIONAL_INTERP
#error TEN_DIRECTIONAL_INTERP should be disabled if DCTIF_8_6_LUMA is enabled
#endif

#define ZERO_MVD_EST											0           // Zero Mvd Estimation in normal mode
///////////////////////////////
// SAMSUNG defines section end
///////////////////////////////

///////////////////////////////
// DOCOMO defines section start
///////////////////////////////
#define DCM_RDCOST_TEMP_FIX //Enables temporary bug fixes to RD cost computation
#define FIX_D235 1 // Fixes an issue with CABAC state management in intra mode search

#define DCM_DECODING_REFRESH              1           ///< enable/disable decoding refresh (IDR and CDR)
#if DCM_DECODING_REFRESH
#define DCM_SKIP_DECODING_FRAMES          1           ///< enable/disable the random access by the decoder
#endif

#define DCM_SIMPLIFIED_MVP                1           ///< enable/disable the simplified motoin vector prediction(D231)

#define DCM_COMB_LIST                  1           ///< Use of combined list for uni-prediction in B-slices


///////////////////////////////
// DOCOMO defines section end
///////////////////////////////

////////////////////////////////
// TOSHIBA defines section start
////////////////////////////////
#define TSB_ALF_HEADER                 1           // Send ALF ON/OFF flag in slice header
#define IBDI_DISTORTION                0           ///< enable/disable SSE modification when IBDI is used (JCTVC-D152)
#define FIXED_ROUNDING_FRAME_MEMORY    0           ///< enable/disable fixed rounding to 8-bitdepth of frame memory when IBDI is used  
////////////////////////////////
// TOSHIBA defines section end
////////////////////////////////

#define BUGFIX102 1 // Do not code terminating bit when using LCEC
#define BUGFIX118 1 // Fixes an issue related to ALF and all-0 filters

////////////////////////////////
// MICROSOFT&USTC defines section start
////////////////////////////////
#define MS_NO_BACK_PRED_IN_B0           1           // disable backward prediction when list1 == list0, and disable list1 search, JCTVC-C278
#define MS_LAST_CBF                     1           // last cbf handling, JCTVC-C277
#define MS_LCEC_ONE_FRAME		        1           // change the initial table in LCEC when there is up to one reference frame in each list, JCTVC-D141
#define MS_LCEC_LOOKUP_TABLE_MAX_VALUE  1           // use the information of the max position in the lookup table, JCTVC-D141
#define MS_LCEC_LOOKUP_TABLE_EXCEPTION  1           // deal with the case when the number of reference frames is greater than 2, JCTVC-D141
#define MS_LCEC_UNI_EXCEPTION_THRES     1           // for GPB case, uni-prediction, > MS_LCEC_UNI_EXCEPTION_THRES is exception
////////////////////////////////
// MICROSOFT&USTC defines section end
////////////////////////////////

////////////////////////////////
// MediaTek defines section start
////////////////////////////////
#define MTK_DISABLE_INTRA_NxN_SPLIT       1           ///< Disable use of PUs-mode NxN for CUs larger 8x8 (intra only)
#define MTK_NONCROSS_INLOOP_FILTER        1           ///< Allow non-cross-slice-boundary in-loop filtering, including DB & ALF (JCTVC-D128)
////////////////////////////////
// MediaTek defines section end
////////////////////////////////

#define FT_TCTR 1
#define FT_TCTR_MERGE 1
#define PANASONIC_AMVPTEMPORALEXT 1
#define PANASONIC_MERGETEMPORALEXT 1
#define FAST_UDI_USE_MPM 1
#define SONY_SIG_CTX 1
#define SNY_DQP                          1           ///< SONY's proposal on syntax change of dQP (JCT-VC D258)

#define AMVP_BUFFERCOMPRESS                   1     // motion vector buffer compression
#define AMVP_DECIMATION_FACTOR                4

#define TI_ALF_MAX_VSIZE_7 1

#define CHROMA_CODEWORD 1                             ///< enable new intra chroma mode encoding by setting to 1. setting to 0 should yield same results as TMuC 0.9
#define CHROMA_CODEWORD_SWITCH  1                     ///< Switch the places of the last two codewords 

#define FULL_NBIT 0 ///< When enabled, does not use g_uiBitIncrement anymore to support > 8 bit data

/////////////////////////////////
// AHG SLICES defines section start
/////////////////////////////////
#define AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE      1          ///< OPTION IDENTIFIER. mode==1 -> Limit maximum number of largest coding tree blocks in a slice
#define AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE    2          ///< OPTION IDENTIFIER. mode==2 -> Limit maximum number of bins/bits in a slice

// Entropy slice options
#define SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE            1          ///< OPTION IDENTIFIER. Limit maximum number of largest coding tree blocks in an entropy slice
#define SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE         2          ///< OPTION IDENTIFIER. Limit maximum number of bins/bits in an entropy slice
/////////////////////////////////
// AHG SLICES defines section end
/////////////////////////////////

/////////////////////////////////
// NEC defines section start
/////////////////////////////////

#define CONSTRAINED_INTRA_PRED            1           // JCTVC-D086: constrained intra prediction

/////////////////////////////////
// NEC defines section end
/////////////////////////////////

/////////////////////////////////
// MQT (MEDIATEK, QUALCOMM, TOSHIBA) defines section start
/////////////////////////////////
#define MQT_ALF_NPASS                       1
/////////////////////////////////
// MQT (MEDIATEK, QUALCOMM, TOSHIBA) defines section start
/////////////////////////////////

// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================

typedef       void                Void;
typedef       bool                Bool;

typedef       char                Char;
typedef       unsigned char       UChar;
typedef       short               Short;
typedef       unsigned short      UShort;
typedef       int                 Int;
typedef       unsigned int        UInt;
typedef       long                Long;
typedef       unsigned long       ULong;
typedef       double              Double;

// ====================================================================================================================
// 64-bit integer type
// ====================================================================================================================

#ifdef _MSC_VER
typedef       __int64             Int64;

#if _MSC_VER <= 1200 // MS VC6
typedef       __int64             UInt64;   // MS VC6 does not support unsigned __int64 to double conversion
#else
typedef       unsigned __int64    UInt64;
#endif

#else

typedef       long long           Int64;
typedef       unsigned long long  UInt64;

#endif

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef       UChar           Pxl;        ///< 8-bit pixel type
typedef       Short           Pel;        ///< 16-bit pixel type
typedef       Int             TCoeff;     ///< transform coefficient

/// parameters for adaptive loop filter
class TComPicSym;

struct _AlfParam
{
  Int alf_flag;                           ///< indicates use of ALF
  Int cu_control_flag;                    ///< coding unit based control flag
  Int chroma_idc;                         ///< indicates use of ALF for chroma
#if TI_ALF_MAX_VSIZE_7
  Int tap;                                ///< number of filter taps - horizontal
  Int tapV;                               ///< number of filter taps - vertical
#else
  Int tap;                                ///< number of filter taps
#endif
  Int num_coeff;                          ///< number of filter coefficients
  Int *coeff;                             ///< filter coefficient array
  Int tap_chroma;                         ///< number of filter taps (chroma)
  Int num_coeff_chroma;                   ///< number of filter coefficients (chroma)
  Int *coeff_chroma;                      ///< filter coefficient array (chroma)
  //CodeAux related
  Int realfiltNo;
  Int filtNo;
  Int filterPattern[16];
  Int startSecondFilter;
  Int noFilters;
  Int varIndTab[16];
  
  //Coeff send related
  Int filters_per_group_diff; //this can be updated using codedVarBins
  Int filters_per_group;
  Int codedVarBins[16]; 
  Int forceCoeff0;
  Int predMethod;
  Int **coeffmulti;
  Int minKStart;
  Int maxScanVal;
  Int kMinTab[42];
#if TSB_ALF_HEADER
  UInt num_alf_cu_flag;
  UInt num_cus_in_frame;
  UInt alf_max_depth;
  UInt *alf_cu_flag;
#endif
};

/// parameters for deblocking filter
typedef struct _LFCUParam
{
  Bool bInternalEdge;                     ///< indicates internal edge
  Bool bLeftEdge;                         ///< indicates left edge
  Bool bTopEdge;                          ///< indicates top edge
} LFCUParam;

/// parapeters for TENTM coefficient VLC
typedef struct _LastCoeffStruct
{
  int level;
  int last_pos;
} LastCoeffStruct;

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
  SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
  SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
  SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
  SIZE_NxN,             ///< symmetric motion partition,   Nx N
  
  SIZE_NONE = 15
};

/// supported prediction type
enum PredMode
{
  MODE_SKIP,            ///< SKIP mode
  MODE_INTER,           ///< inter-prediction mode
  MODE_INTRA,           ///< intra-prediction mode
  MODE_NONE = 15
};

/// texture component type
enum TextType
{
  TEXT_LUMA,            ///< luma
  TEXT_CHROMA,          ///< chroma (U+V)
  TEXT_CHROMA_U,        ///< chroma U
  TEXT_CHROMA_V,        ///< chroma V
  TEXT_ALL,             ///< Y+U+V
  TEXT_NONE = 15
};

/// reference list index
enum RefPicList
{
  REF_PIC_LIST_0 = 0,   ///< reference list 0
  REF_PIC_LIST_1 = 1,   ///< reference list 1
#if DCM_COMB_LIST
  REF_PIC_LIST_C = 2,   ///< combined reference list for uni-prediction in B-Slices
#endif
  REF_PIC_LIST_X = 100  ///< special mark
};

/// distortion function index
enum DFunc
{
  DF_DEFAULT  = 0,
  DF_SSE      = 1,      ///< general size SSE
  DF_SSE4     = 2,      ///<   4xM SSE
  DF_SSE8     = 3,      ///<   8xM SSE
  DF_SSE16    = 4,      ///<  16xM SSE
  DF_SSE32    = 5,      ///<  32xM SSE
  DF_SSE64    = 6,      ///<  64xM SSE
  DF_SSE16N   = 7,      ///< 16NxM SSE
  
  DF_SAD      = 8,      ///< general size SAD
  DF_SAD4     = 9,      ///<   4xM SAD
  DF_SAD8     = 10,     ///<   8xM SAD
  DF_SAD16    = 11,     ///<  16xM SAD
  DF_SAD32    = 12,     ///<  32xM SAD
  DF_SAD64    = 13,     ///<  64xM SAD
  DF_SAD16N   = 14,     ///< 16NxM SAD
  
  DF_SADS     = 15,     ///< general size SAD with step
  DF_SADS4    = 16,     ///<   4xM SAD with step
  DF_SADS8    = 17,     ///<   8xM SAD with step
  DF_SADS16   = 18,     ///<  16xM SAD with step
  DF_SADS32   = 19,     ///<  32xM SAD with step
  DF_SADS64   = 20,     ///<  64xM SAD with step
  DF_SADS16N  = 21,     ///< 16NxM SAD with step
  
  DF_HADS     = 22,     ///< general size Hadamard with step
  DF_HADS4    = 23,     ///<   4xM HAD with step
  DF_HADS8    = 24,     ///<   8xM HAD with step
  DF_HADS16   = 25,     ///<  16xM HAD with step
  DF_HADS32   = 26,     ///<  32xM HAD with step
  DF_HADS64   = 27,     ///<  64xM HAD with step
  DF_HADS16N  = 28,     ///< 16NxM HAD with step
  
  DF_SSE_FRAME = 33     ///< Frame-based SSE
};

/// index for reference type
enum  ERBIndex
{
  ERB_NONE    = 0,      ///< normal case
  ERB_LTR     = 1       ///< long-term reference
};

/// index for SBAC based RD optimization
enum CI_IDX
{
  CI_CURR_BEST = 0,     ///< best mode index
  CI_NEXT_BEST,         ///< next best index
  CI_TEMP_BEST,         ///< temporal index
  CI_CHROMA_INTRA,      ///< chroma intra index
  CI_QT_TRAFO_TEST,
  CI_QT_TRAFO_ROOT,
  CI_NUM,               ///< total number
};

/// motion vector predictor direction used in AMVP
enum MVP_DIR
{
  MD_LEFT = 0,          ///< MVP of left block
  MD_ABOVE,             ///< MVP of above block
  MD_ABOVE_RIGHT,       ///< MVP of above right block
  MD_BELOW_LEFT,        ///< MVP of below left block
  MD_ABOVE_LEFT         ///< MVP of above left block
};

/// motion vector prediction mode used in AMVP
enum AMVP_MODE
{
  AM_NONE = 0,          ///< no AMVP mode
  AM_EXPL,              ///< explicit signalling of motion vector index
};

/// interpolation filter type
enum InterpFilterType
{
  IPF_SAMSUNG_DIF_DEFAULT = 0,          ///< Samsung DCT-based filter
  IPF_HHI_4TAP_MOMS,                    ///< HHI 4-tap MOMS filter
  IPF_HHI_6TAP_MOMS,                    ///< HHI 6-tap MOMS filter
# if TEN_DIRECTIONAL_INTERP
  IPF_TEN_DIF                           ///< TEN directional filter
# else
  IPF_TEN_DIF_PLACEHOLDER               ///< Place holder to keep ordering if IPF_TEN_DIF not compiled-in
# endif
  ,IPF_LAST
};

#if QC_MDCS
/// coefficient scanning type used in ACS
enum COEFF_SCAN_TYPE
{
  SCAN_ZIGZAG = 0,			///< typical zigzag scan
  SCAN_HOR,							///< horizontal first scan
  SCAN_VER							///< vertical first scan
};
#endif //QC_MDCS

#endif

