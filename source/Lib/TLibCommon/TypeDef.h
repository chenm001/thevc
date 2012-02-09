/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TypeDef.h
    \brief    Define basic types, new types and enumerations
*/

#ifndef _TYPEDEF__
#define _TYPEDEF__

#define CLEANUP_CTXINIT           1
//! \ingroup TLibCommon
//! \{

#define PADDING_INTRA             1 ///< G812: padding from bottom left, copy previous pixel instead of averaging
#define COEFF_CTX_RED             1 ///< G121: reduce max value of c1 and c2

#define MRG_MAX_NUM_CANDS_SIGNALED         5   //<G091: value of maxNumMergeCand signaled in slice header 

#define WEIGHT_PRED_IMP           1 ///< high-precision offset for weighted bipred G065
#define PLANAR_IS_DEFAULT         1 ///< default to planar if neighbor not available G119
#define VER_HOR_FILTER            1 ///< F172: intra ver/hor prediction filter
#define WP_IMPROVED_SYNTAX        1 ///< improved weighted prediction syntax to remove redundancy G441

#define REMOVE_INTRA_LINE_BUFFER  1 ///< G145: intra line buffer removal

#define WEIGHTED_CHROMA_DISTORTION  1   ///< F386: weighting of chroma for RDO
#define RDOQ_CHROMA_LAMBDA          1   ///< F386: weighting of chroma for RDOQ
#define ALF_CHROMA_LAMBDA           1   ///< F386: weighting of chroma for ALF
#define SAO_CHROMA_LAMBDA           1   ///< F386: weighting of chroma for SAO

#define UNI_BI_IDENTICAL_MOTION   1 ///< chaning bi-prediction to uni-prediction for identical motion G415/G438

#define G216_ALF_MERGE_FLAG_FIX     1   ///< G216: bug fixed: removing 15th merge flag for BA mode
#define G212_CROSS9x9_VB            1   ///< G212: Cross9x9 filter shape and virtual boundary processing for ALF
#define G610_ALF_K_BIT_FIX          1   ///< G610: bug fixed: removing extra alf_golomb_index_bit for cross-shaped filter
#if G610_ALF_K_BIT_FIX
#if G212_CROSS9x9_VB
#define MIN_SCAN_POS_CROSS          4
#else
#define MIN_SCAN_POS_CROSS          5
#endif
#endif
#define G214_ALF_CONSTRAINED_COEFF  1   ///< G214: Constrained ALF coefficient value
#define G215_ALF_NUM_FILTER         1   ///< G215: the number of filters in one picture, encoder only
#define ALF_DC_OFFSET_REMOVAL       1   ///< G445: Remove DC offset for ALF

#define SAO_RDO_OFFSET              1   ///< G915: Considering rate-distortion-cost in optimal offset calculation for SAO
#define G1023_FIX_NPASS_ALF         1   ///< G1023: Improved ALF N-pass encoding

#define FAST_BIT_EST                1   ///< G763: Table-based bit estimation for CABAC
#define G633_8BIT_INIT              1   ///< G633: Context model initialization method using 8 bit initialization values

#define G519_TU_AMP_NSQT_HARMONIZATION  1   ///< G519: Harmonization of implicit TU, AMP and NSQT

////////////////////////////
// JCT-VC G start
////////////////////////////

#define UNIFIED_SCAN_PASSES                 1      ///< G320 : Unified scan passes for transform coefficient coding
#define SUBBLOCK_SCAN                       1      ///< G323 : 4x4 sub-block based scan for large blocks
#if SUBBLOCK_SCAN
#define MULTI_LEVEL_SIGNIFICANCE            1      ///< G644 : Multi-level significance map for large TUs
#endif
#if MULTI_LEVEL_SIGNIFICANCE
#define MLS_GRP_NUM                         64     ///< G644 : Max number of coefficient groups, max(16, 64)
#define MLS_CG_SIZE                         4      ///< G644 : Coefficient group size of 4x4
#endif
#define CHROMA_CBF_CTX_REDUCTION            1      ///< G718 : Sharing contexts for cbf_cb and cbf_cr

#define COEFF_CTXSET_RED                    1      ///< G783 : reduce level context set of chroma
#define ALF_SAO_SLICE_FLAGS                 1      ///< G566 : Re-insert ALF and SAO flags in the slice header

#define NONCROSS_TILE_IN_LOOP_FILTERING     1      ///< G194 : Non-cross-tiles loop filtering

#define ADAPTIVE_QP_SELECTION               1      ///< G382: Adaptive reconstruction levels, non-normative part for adaptive QP selection
#if ADAPTIVE_QP_SELECTION
#define ARL_C_PRECISION                     7      ///< G382: 7-bit arithmetic precision
#define LEVEL_RANGE                         30     ///< G382: max coefficient level in statistics collection
#endif

#define G678_LAMBDA_ADJUSTMENT              1      ///< G678: lambda adjustment for rate-constrained test tool

////////////////////////////
// JCT-VC G end
////////////////////////////

////////////////////////////
// JCT-VC F start
////////////////////////////
#define NSQT_DIAG_SCAN                      1      ///< G1038: use diagonal and subblock scans for NSQT
#if NSQT_DIAG_SCAN && !(SUBBLOCK_SCAN)
#error
#endif
#define NS_HAD                               1

  #define APS_BITS_FOR_SAO_BYTE_LENGTH 12           
  #define APS_BITS_FOR_ALF_BYTE_LENGTH 8

////////////////////////////
// JCT-VC F end
////////////////////////////

#define HHI_RQT_INTRA_SPEEDUP             1           ///< tests one best mode with full rqt
#define HHI_RQT_INTRA_SPEEDUP_MOD         0           ///< tests two best modes with full rqt

#if HHI_RQT_INTRA_SPEEDUP_MOD && !HHI_RQT_INTRA_SPEEDUP
#error
#endif

#define VERBOSE_RATE 0 ///< Print additional rate information in encoder

#define AMVP_DECIMATION_FACTOR            4

#define SCAN_SET_SIZE                     16
#define LOG2_SCAN_SET_SIZE                4

#define FAST_UDI_MAX_RDMODE_NUM               35          ///< maximum number of RD comparison in fast-UDI estimation loop 

#define ZERO_MVD_EST                          0           ///< Zero Mvd Estimation in normal mode

#define NUM_INTRA_MODE 36
#define LM_CHROMA_IDX  35

#define IBDI_DISTORTION                0           ///< enable/disable SSE modification when IBDI is used (JCTVC-D152)
#define FIXED_ROUNDING_FRAME_MEMORY    0           ///< enable/disable fixed rounding to 8-bitdepth of frame memory when IBDI is used  

#define MS_LCEC_UNI_EXCEPTION_THRES     1           // for GPB case, uni-prediction, > MS_LCEC_UNI_EXCEPTION_THRES is exception

#define G1002_CRA_CHECK                 1
#define INTER_RPS_PREDICTION            1           // remove this once tested.
#define WRITE_BACK                      1           ///< Enable/disable the encoder to replace the deltaPOC and Used by current from the config file with the values derived by the refIdc parameter.
#define PRINT_RPS_INFO                  0           ///< Enable/disable the printing of bits used to send the RPS.
                                                    // using one nearest frame as reference frame, and the other frames are high quality (POC%4==0) frames (1+X)
                                                    // this should be done with encoder only decision
                                                    // but because of the absence of reference frame management, the related code was hard coded currently

#define OL_FLUSH 1          // Set to 1 to enable Wavefront Flush.
#define OL_FLUSH_ALIGN 0    // Align flush to byte boundary.  This preserves byte operations in CABAC (faster) but at the expense of an average
                            // of 4 bits per flush.
                            // Setting to 0 will slow cabac by an as yet unknown amount.
                            // This is here just to perform timing tests -- OL_FLUSH_ALIGN should be 0 for WPP.

#define RVM_VCEGAM10 1 // RVM model proposed in VCEG-AM10
#if RVM_VCEGAM10
#define RVM_VCEGAM10_M 4
#endif

#define PLANAR_IDX             0
#define DC_IDX                 3                     // index for intra DC mode
#define NUM_CHROMA_MODE        6                     // total number of chroma modes
#define DM_CHROMA_IDX          36                    // chroma mode index for derived from luma intra mode


#define FAST_UDI_USE_MPM 1

#define QP_ADAPTATION                     0           ///< Enable TM5Step3-like QP adaptation in encoder (JCTVC-D308/E215)
#if QP_ADAPTATION
#define RDO_WITHOUT_DQP_BITS              0           ///< Disable counting dQP bits in RDO-based mode decision
#endif

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

#define LOG2_MAX_NUM_COLUMNS_MINUS1        7
#define LOG2_MAX_NUM_ROWS_MINUS1           7
#define LOG2_MAX_COLUMN_WIDTH              13
#define LOG2_MAX_ROW_HEIGHT                13

#define MAX_MARKER_PER_NALU                 1000
#define TILES_LOW_LATENCY_CABAC_INI         1 // JCTVC-G197 = low latency CABAC initialization for dependent tiles

#define MATRIX_MULT                             0   // Brute force matrix multiplication instead of partial butterfly

#define REG_DCT 65535

#define AMP_SAD                               1           ///< dedicated SAD functions for AMP
#define AMP_ENC_SPEEDUP                       1           ///< encoder only speed-up by AMP mode skipping
#if AMP_ENC_SPEEDUP
#define AMP_MRG                               1           ///< encoder only force merge for AMP partition (no motion search for AMP)
#endif


#define SCALING_LIST                  1 //JCTVC-G880/JCTVC-G1016 quantization matrices
#define SCALING_LIST_OUTPUT_RESULT    0 //JCTVC-G880/JCTVC-G1016 quantization matrices

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

#define NUM_DOWN_PART 4
#define NUM_MAX_OFFSET  32

enum SAOTypeLen
{
  SAO_EO_LEN    = 4, 
  SAO_EO_LEN_2D = 6, 
  SAO_BO_LEN    = 16
};

enum SAOType
{
  SAO_EO_0 = 0, 
  SAO_EO_1,
  SAO_EO_2, 
  SAO_EO_3,
  SAO_BO_0,
  SAO_BO_1,
  MAX_NUM_SAO_TYPE
};

typedef struct _SaoQTPart
{
  Bool        bEnableFlag;
  Int         iBestType;
  Int         iLength;
  Int         iOffset[32];

  Int         StartCUX;
  Int         StartCUY;
  Int         EndCUX;
  Int         EndCUY;

  Int         PartIdx;
  Int         PartLevel;
  Int         PartCol;
  Int         PartRow;

  Int         DownPartsIdx[NUM_DOWN_PART];
  Int         UpPartIdx;

  Bool        bSplit;

  //---- encoder only start -----//
  Bool        bProcessed;
  Double      dMinCost;
  Int64       iMinDist;
  Int         iMinRate;
  //---- encoder only end -----//
} SAOQTPart;

struct SAOParam
{
  Bool       bSaoFlag[3];
  SAOQTPart* psSaoPart[3];
  Int        iMaxSplitLevel;
  Int        iNumClass[MAX_NUM_SAO_TYPE];
  ~SAOParam();
};

struct ALFParam
{
  Int alf_flag;                           ///< indicates use of ALF
  Int chroma_idc;                         ///< indicates use of ALF for chroma
  Int num_coeff;                          ///< number of filter coefficients
  Int filter_shape;
  Int filter_shape_chroma;
  Int num_coeff_chroma;                   ///< number of filter coefficients (chroma)
  Int *coeff_chroma;                      ///< filter coefficient array (chroma)
  Int *filterPattern;
  Int startSecondFilter;
  Int filters_per_group;
  Int predMethod;
  Int *nbSPred;
  Int **coeffmulti;
  Int minKStart;
  Int maxScanVal;
  Int kMinTab[42];

  Int alf_pcr_region_flag;
  ~ALFParam();
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

/// chroma formats (according to semantics of chroma_format_idc)
enum ChromaFormat
{
  CHROMA_400  = 0,
  CHROMA_420  = 1,
  CHROMA_422  = 2,
  CHROMA_444  = 3
};

/// supported partition shape
enum PartSize
{
  SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
  SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
  SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
  SIZE_NxN,             ///< symmetric motion partition,   Nx N
  SIZE_2NxnU,           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
  SIZE_2NxnD,           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
  SIZE_nLx2N,           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
  SIZE_nRx2N,           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N
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
  REF_PIC_LIST_C = 2,   ///< combined reference list for uni-prediction in B-Slices
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
  
#if AMP_SAD
  DF_SAD12    = 43,
  DF_SAD24    = 44,
  DF_SAD48    = 45,

  DF_SADS12   = 46,
  DF_SADS24   = 47,
  DF_SADS48   = 48,

  DF_SSE_FRAME = 50     ///< Frame-based SSE
#else
  DF_SSE_FRAME = 33     ///< Frame-based SSE
#endif
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

/// coefficient scanning type used in ACS
enum COEFF_SCAN_TYPE
{
  SCAN_ZIGZAG = 0,      ///< typical zigzag scan
  SCAN_HOR,             ///< horizontal first scan
  SCAN_VER,              ///< vertical first scan
  SCAN_DIAG              ///< up-right diagonal scan
};

//! \}

#endif


