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

//! \ingroup TLibCommon
//! \{
#define FAST_DECISION_FOR_MRG_RD_COST  1 ////< H0178: Fast Decision for Merge 2Nx2N RDCost

#define REMOVE_DIV_OPERATION      1 ///< H0238: Simplified intra horizontal and vertical filtering
#define LOGI_INTRA_NAME_3MPM      1  ///< H0407: logical Intra mode naming (sequential angular mode numbering) and 3 MPM mode coding

#define LEVEL_CTX_LUMA_RED        1  ///<H0130: Luma level context reduction
#define REMOVE_INFER_SIGGRP       1  ///<H0131: Remove inferred significant_coeff_group_flag

#define SET_MERGE_TMVP_REFIDX     1  ///< H0278/H0199: Setting the merge TMVP refidx to 0 for the non-first partition

#define MULTILEVEL_SIGMAP_EXT     1  ///< H0526: multi-level significance map extended to smaller TUs
#define MULTIBITS_DATA_HIDING     1  ///< H0481: multiple sign bit hiding

#define DEQUANT_CLIPPING           1  ///< H0312/H0541: transformed coefficients clipping before de-quantization

#define REMOVE_NON_SCALED         1 ///< H0164/H0250: Removal of non-scaled merge candidate
#define MRG_IDX_CTX_RED           1 ///< H0251: Merge index context reduction
#define SIMP_MRG_PRUN             1 ///< H0252: simplification of merge pruning process

#define AMVP_PRUNING_SIMPLIFICATION         1     ///H0316: simplify the pruning process of AMVP by exempting the temporal candidate
#define AMVP_ZERO_CHECKING_REMOVAL          1     ///H0239/H0316: remove zero motion vector checking of AMVP

#define H0111_MVD_L1_ZERO         1  ///< H0111: modification of bi-prediction
#define DISABLING_CLIP_FOR_BIPREDME         1  ///< Ticket #175
  
#define CLIPSCALEDMVP               1  ///< H0216: Clipping scaled MV to 16 bit

#define UNIFIED_TRANSFORM_TREE      1   ///< H0123: unified tree structure for TU

#define SIGMAP_CTX_SUBBLOCK       1 ///< H0290: 4x4 sub-block based region for significant_flag context selection

#define SIGMAP_CONST_AT_HIGH_FREQUENCY      1      ///< H0095 method2.1: const significance map at high freaquency

#define LAST_CTX_REDUCTION        1  ///< H0537/H514: contexts reduction for last position coding

#define AMP_CTX                   1 ///<H0545: context reduction for asymmetric partition

#define RESTRICT_GR1GR2FLAG_NUMBER    1 ///< H0554: Throughput improvement of CABAC coefficients level coding
#if RESTRICT_GR1GR2FLAG_NUMBER    // 
#define C1FLAG_NUMBER               8 // maximum number of largerThan1 flag coded in one chunk :  16 in HM5
#define C2FLAG_NUMBER               1 // maximum number of largerThan2 flag coded in one chunk:  16 in HM5 
#endif 

#define EIGHT_BITS_RICE_CODE        1 ///< H0498 : 8 bits rice codes

#define SAO_UNIT_INTERLEAVING      1   ///< H0273
#define REMOVE_SAO_LCU_ENC_CONSTRAINTS_1 0  ///< disable the encoder constraint that does not test SAO/BO mode for chroma in interleaved mode
#define REMOVE_SAO_LCU_ENC_CONSTRAINTS_2 0  ///< disable the encoder constraint that reduce the range of SAO/EO for chroma in interleaved mode
#define REMOVE_SAO_LCU_ENC_CONSTRAINTS_3 0  ///< disable the encoder constraint that conditionally disable SAO for chroma for entire slice in interleaved mode

#define ALF_SINGLE_FILTER_SHAPE    1     //< !!! H0068: Single filter type : 9x7 cross + 3x3 square

#define PARAMSET_VLC_CLEANUP               1      ///< followup to G220: Simplify parameter set code

#define ALF_16_BA_GROUPS        1     ///< H0409 16 BA groups
#define LCU_SYNTAX_ALF          1     ///< H0274 LCU-syntax ALF
#define ALF_CHROMA_COEF_PRED_HARMONIZATION 1 ///< H0483: ALF chroma coeff pred harmonization

#define CABAC_LINEAR_INIT       1     ///< H0535 : linear CABAC initialization

#define MAX_NUM_SPS                32
#define MAX_NUM_PPS                256
#define MAX_NUM_APS                32         //< !!!KS: number not defined in WD yet

#define MRG_MAX_NUM_CANDS_SIGNALED         5   //<G091: value of maxNumMergeCand signaled in slice header 

#define WEIGHTED_CHROMA_DISTORTION  1   ///< F386: weighting of chroma for RDO
#define RDOQ_CHROMA_LAMBDA          1   ///< F386: weighting of chroma for RDOQ
#define ALF_CHROMA_LAMBDA           1   ///< F386: weighting of chroma for ALF
#define SAO_CHROMA_LAMBDA           1   ///< F386: weighting of chroma for SAO

#define MIN_SCAN_POS_CROSS          4

#define FAST_BIT_EST                1   ///< G763: Table-based bit estimation for CABAC

#define G519_TU_AMP_NSQT_HARMONIZATION  1   ///< G519: Harmonization of implicit TU, AMP and NSQT

#define MLS_GRP_NUM                         64     ///< G644 : Max number of coefficient groups, max(16, 64)
#define MLS_CG_SIZE                         4      ///< G644 : Coefficient group size of 4x4

#define ADAPTIVE_QP_SELECTION               1      ///< G382: Adaptive reconstruction levels, non-normative part for adaptive QP selection
#if ADAPTIVE_QP_SELECTION
#define ARL_C_PRECISION                     7      ///< G382: 7-bit arithmetic precision
#define LEVEL_RANGE                         30     ///< G382: max coefficient level in statistics collection
#endif


#define CHROMA_MODE_CODING                   1     //H0326/H0475 : 2-length fixed, bypass coding for chroma intra prediction mode

#define NSQT_LFFIX                           1     ///< Bug fix related to NSQT and deblocking filter
#define NS_HAD                               1

#define APS_BITS_FOR_SAO_BYTE_LENGTH 12           
#define APS_BITS_FOR_ALF_BYTE_LENGTH 8

#define H0736_AVC_STYLE_QP_RANGE             1    ///< H0736: AVC style qp range and wrapping.
#define H0204_QP_PREDICTION                  1    ///< H0204: improved QP prediction

#define HHI_RQT_INTRA_SPEEDUP             1           ///< tests one best mode with full rqt
#define HHI_RQT_INTRA_SPEEDUP_MOD         0           ///< tests two best modes with full rqt

#define BURST_IPCM                        1           ///< H0051: Burst IPCM

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

#define RVM_VCEGAM10_M 4

#define PLANAR_IDX             0
#if LOGI_INTRA_NAME_3MPM
#define VER_IDX                26                    // index for intra VERTICAL   mode
#define HOR_IDX                10                    // index for intra HORIZONTAL mode
#define DC_IDX                 1                     // index for intra DC mode
#else
#define DC_IDX                 3                     // index for intra DC mode
#endif
#define NUM_CHROMA_MODE        6                     // total number of chroma modes
#define DM_CHROMA_IDX          36                    // chroma mode index for derived from luma intra mode


#define FAST_UDI_USE_MPM 1

#define RDO_WITHOUT_DQP_BITS              0           ///< Disable counting dQP bits in RDO-based mode decision

#define FULL_NBIT 0 ///< When enabled, does not use g_uiBitIncrement anymore to support > 8 bit data

#define AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE      1          ///< OPTION IDENTIFIER. mode==1 -> Limit maximum number of largest coding tree blocks in a slice
#define AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE    2          ///< OPTION IDENTIFIER. mode==2 -> Limit maximum number of bins/bits in a slice

// Entropy slice options
#define SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE            1          ///< OPTION IDENTIFIER. Limit maximum number of largest coding tree blocks in an entropy slice
#define SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE         2          ///< OPTION IDENTIFIER. Limit maximum number of bins/bits in an entropy slice

#define LOG2_MAX_NUM_COLUMNS_MINUS1        7
#define LOG2_MAX_NUM_ROWS_MINUS1           7
#define LOG2_MAX_COLUMN_WIDTH              13
#define LOG2_MAX_ROW_HEIGHT                13

#define MAX_MARKER_PER_NALU                 1000

#define MATRIX_MULT                             0   // Brute force matrix multiplication instead of partial butterfly

#define REG_DCT 65535

#define AMP_SAD                               1           ///< dedicated SAD functions for AMP
#define AMP_ENC_SPEEDUP                       1           ///< encoder only speed-up by AMP mode skipping
#if AMP_ENC_SPEEDUP
#define AMP_MRG                               1           ///< encoder only force merge for AMP partition (no motion search for AMP)
#endif

#define SCALING_LIST_OUTPUT_RESULT    0 //JCTVC-G880/JCTVC-G1016 quantization matrices
#define SCALING_LIST                  1 //JCTVC-H0230/H0461/H0237

#define DEFAULT_DC                    1 // JCTVC-H0242

#define RPS_IN_SPS                    1 // Adopted during discussion of JCTVC-H0423

#define H0412_REF_PIC_LIST_RESTRICTION 1

#define H0566_TLA                     1

#define H0567_DPB_PARAMETERS_PER_TEMPORAL_LAYER 1

#define DBL_H0473_PART_1          1   //Deblocking filtering simplification
#define DBL_CONTROL               1   //PPS deblocking_filter_control_present_flag (JCTVC-H0398); condition for inherit params flag in SH (JCTVC-H0424)
#define DBL_STRONG_FILTER_CLIP    1   //Introduction of strong filter clipping in deblocking filter (JCTVC-H0275)

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

enum SAOTypeLen
{
  SAO_EO_LEN    = 4, 
#if SAO_UNIT_INTERLEAVING
  SAO_BO_LEN    = 4,
  SAO_MAX_BO_CLASSES = 32
#else
  SAO_BO_LEN    = 16
#endif
};

enum SAOType
{
  SAO_EO_0 = 0, 
  SAO_EO_1,
  SAO_EO_2, 
  SAO_EO_3,
#if SAO_UNIT_INTERLEAVING
  SAO_BO,
#else
  SAO_BO_0,
  SAO_BO_1,
#endif
  MAX_NUM_SAO_TYPE
};

typedef struct _SaoQTPart
{
#if !SAO_UNIT_INTERLEAVING
  Bool        bEnableFlag;
#endif
  Int         iBestType;
  Int         iLength;
#if SAO_UNIT_INTERLEAVING
  Int         bandPosition ;
  Int         iOffset[4];
#else
  Int         iOffset[32];
#endif
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

#if SAO_UNIT_INTERLEAVING
typedef struct _SaoLcuParam
{
  Bool       mergeUpFlag;
  Bool       mergeLeftFlag;
  Int        typeIdx;
  Int        bandPosition;
  Int        offset[4];
  Int        runDiff;
  Int        run;
  Int        partIdx;
  Int        partIdxTmp;
  Int        length;
} SaoLcuParam;
#endif

struct SAOParam
{
  Bool       bSaoFlag[3];
  SAOQTPart* psSaoPart[3];
  Int        iMaxSplitLevel;
  Int        iNumClass[MAX_NUM_SAO_TYPE];
#if SAO_UNIT_INTERLEAVING
  Bool         oneUnitFlag[3];
  SaoLcuParam* saoLcuParam[3];
  Int          numCuInHeight;
  Int          numCuInWidth;
#endif
  ~SAOParam();
};

struct ALFParam
{
  Int alf_flag;                           ///< indicates use of ALF
#if !LCU_SYNTAX_ALF
  Int chroma_idc;                         ///< indicates use of ALF for chroma
#endif
  Int num_coeff;                          ///< number of filter coefficients
  Int filter_shape;
#if !LCU_SYNTAX_ALF
  Int filter_shape_chroma;
  Int num_coeff_chroma;                   ///< number of filter coefficients (chroma)
  Int *coeff_chroma;                      ///< filter coefficient array (chroma)
#endif
  Int *filterPattern;
  Int startSecondFilter;
  Int filters_per_group;
  Int predMethod;
  Int *nbSPred;
  Int **coeffmulti;
  Int minKStart;
#if !LCU_SYNTAX_ALF
  Int maxScanVal;
  Int kMinTab[42];

  Int alf_pcr_region_flag;
  ~ALFParam();
#endif
#if LCU_SYNTAX_ALF
  Int componentID;
  Int* kMinTab;
  //constructor, operator
  ALFParam():componentID(-1){}
  ALFParam(Int cID){create(cID);}
  ALFParam(const ALFParam& src) {*this = src;}
  ~ALFParam(){destroy();}
  const ALFParam& operator= (const ALFParam& src);
private:
  Void create(Int cID);
  Void destroy();
  Void copy(const ALFParam& src);
#endif
};

#if LCU_SYNTAX_ALF
struct AlfUnitParam
{
  Int   mergeType;
  Bool  isEnabled;
  Bool  isNewFilt;
  Int   storedFiltIdx;
  ALFParam* alfFiltParam;
  //constructor, operator 
  AlfUnitParam();
  AlfUnitParam(const AlfUnitParam& src){ *this = src;}
  const AlfUnitParam& operator= (const AlfUnitParam& src);
  Bool operator == (const AlfUnitParam& cmp);
};

struct AlfParamSet
{
  Bool isEnabled[3];
  Bool isUniParam[3];
  Int  numLCUInWidth;
  Int  numLCUInHeight;
  Int  numLCU;
  AlfUnitParam* alfUnitParam[3];
  //constructor, operator 
  AlfParamSet(){create();}
  ~AlfParamSet(){destroy();}
  Void create(Int width =0, Int height=0, Int num=0);
  Void init();
  Void releaseALFParam();
  Void createALFParam();
private:
  Void destroy();
};
#endif



/// parameters for deblocking filter
typedef struct _LFCUParam
{
  Bool bInternalEdge;                     ///< indicates internal edge
  Bool bLeftEdge;                         ///< indicates left edge
  Bool bTopEdge;                          ///< indicates top edge
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


