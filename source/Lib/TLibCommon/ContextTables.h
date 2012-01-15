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

/** \file     ContextTables.h
    \brief    Defines constants and tables for SBAC
    \todo     number of context models is not matched to actual use, should be fixed
*/

#ifndef __CONTEXTTABLES__
#define __CONTEXTTABLES__

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define MAX_NUM_CTX_MOD             512       ///< maximum number of supported contexts

#define NUM_SPLIT_FLAG_CTX            3       ///< number of context models for split flag
#define NUM_SKIP_FLAG_CTX             3       ///< number of context models for skip flag

#define NUM_MERGE_FLAG_EXT_CTX        1       ///< number of context models for merge flag of merge extended
#define NUM_MERGE_IDX_EXT_CTX         4       ///< number of context models for merge index of merge extended

#define NUM_ALF_CTRL_FLAG_CTX         1       ///< number of context models for ALF control flag
#if PREDTYPE_CLEANUP
#define NUM_PART_SIZE_CTX             4       ///< number of context models for partition size
#else
#define NUM_PART_SIZE_CTX             5       ///< number of context models for partition size
#endif
#if AMP
#define NUM_CU_X_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_CU_Y_POS_CTX              2       ///< number of context models for partition size (AMP)
#endif
#if PREDTYPE_CLEANUP
#define NUM_PRED_MODE_CTX             1       ///< number of context models for prediction mode
#else
#define NUM_PRED_MODE_CTX             2       ///< number of context models for prediction mode
#endif

#if BYPASS_FOR_INTRA_MODE
#define NUM_ADI_CTX                   1       ///< number of context models for intra prediction
#else
#define NUM_ADI_CTX                   3       ///< number of context models for intra prediction
#endif

#define NUM_CHROMA_PRED_CTX           2       ///< number of context models for intra prediction (chroma)
#define NUM_INTER_DIR_CTX             4       ///< number of context models for inter prediction direction
#define NUM_MV_RES_CTX                2       ///< number of context models for motion vector difference

#define NUM_REF_NO_CTX                4       ///< number of context models for reference index
#define NUM_TRANS_SUBDIV_FLAG_CTX     10      ///< number of context models for transform subdivision flags
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF
#define NUM_DELTA_QP_CTX              3       ///< number of context models for dQP

#if MULTI_LEVEL_SIGNIFICANCE
#define NUM_SIG_CG_FLAG_CTX           2       ///< number of context models for MULTI_LEVEL_SIGNIFICANCE
#endif
#if SIGMAP_CTX_RED
#define NUM_SIG_FLAG_CTX              27      ///< number of context models for sig flag
#define NUM_SIG_FLAG_CTX_LUMA         27      ///< number of context models for luma sig flag
#define NUM_SIG_FLAG_CTX_CHROMA       21      ///< number of context models for chroma sig flag
#else
#define NUM_SIG_FLAG_CTX              44      ///< number of context models for sig flag
#endif
#if MODIFIED_LAST_XY_CODING
#define NUM_CTX_LAST_FLAG_XY          18      ///< number of context models for last coefficient position
#else
#define NUM_CTX_LAST_FLAG_XY          19      ///< number of context models for PCP last flag
#endif

#if COEFF_CTX_RED
#define NUM_ONE_FLAG_CTX              24      ///< number of context models for greater than one
#define NUM_ABS_FLAG_CTX              18      ///< number of context models for magnitude
#if COEFF_CTXSET_RED
#define NUM_ONE_FLAG_CTX_LUMA         24      ///< number of context models for greater than one of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than one of chroma
#define NUM_ABS_FLAG_CTX_LUMA         18      ///< number of context models for magnitude of luma
#define NUM_ABS_FLAG_CTX_CHROMA        6      ///< number of context models for magnitude of chroma
#endif
#else
#define NUM_ONE_FLAG_CTX              30      ///< number of context models for greater than one
#define NUM_ABS_FLAG_CTX              30      ///< number of context models for magnitude
#if COEFF_CTXSET_RED
#define NUM_ONE_FLAG_CTX_LUMA         30      ///< number of context models for greater than one of luma
#define NUM_ONE_FLAG_CTX_CHROMA       10      ///< number of context models for greater than one of chroma
#define NUM_ABS_FLAG_CTX_LUMA         30      ///< number of context models for magnitude of luma
#define NUM_ABS_FLAG_CTX_CHROMA       10      ///< number of context models for magnitude of chroma
#endif
#endif

#define NUM_MVP_IDX_CTX               2       ///< number of context models for MVP index

#define NUM_ALF_FLAG_CTX              1       ///< number of context models for ALF flag
#define NUM_ALF_UVLC_CTX              2       ///< number of context models for ALF UVLC (filter length)
#define NUM_ALF_SVLC_CTX              3       ///< number of context models for ALF SVLC (filter coeff.)

#if SAO
#define NUM_SAO_FLAG_CTX              1       ///< number of context models for SAO flag
#define NUM_SAO_UVLC_CTX              2       ///< number of context models for SAO UVLC
#define NUM_SAO_SVLC_CTX              3       ///< number of context models for SAO SVLC
#endif

#if G633_8BIT_INIT
#define CNU                          119      ///< dummy initialization value for unused context models 'Context model Not Used'
#endif

// ====================================================================================================================
// Tables
// ====================================================================================================================


#if G633_8BIT_INIT
// initial probability for split flag
static const UChar
INIT_SPLIT_FLAG[3][NUM_SPLIT_FLAG_CTX] =
{
  {
     87,  74, 107,
    
  },
  {
     84, 103, 105,
    
  },
  {
     84, 103, 105,
    
  },
};

// initial probability for skip flag
static const UChar
INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX] =
{
  {
    CNU, CNU, CNU,
    
  },
  {
    165, 168, 154,
    
  },
  {
    165, 168, 154,
    
  },
};

// initial probability for skip flag
static const UChar
INIT_ALF_CTRL_FLAG[3][NUM_ALF_CTRL_FLAG_CTX] =
{
  {
    153,
    
  },
  {
     87,
    
  },
  {
    135,
    
  },
};

// initial probability for merge flag
static const UChar
INIT_MERGE_FLAG_EXT[3][NUM_MERGE_FLAG_EXT_CTX] =
{
  {
    CNU,
    
  },
  {
    72,
    
  },
  {
    119,
    
  },
};

static const UChar
INIT_MERGE_IDX_EXT[3][NUM_MERGE_IDX_EXT_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    100,  86, 102, 133,
    
  },
  {
    116,  87, 119, 103,
    
  },
};

// initial probability for PU size
static const UChar
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] =
{
#if PREDTYPE_CLEANUP
  {
    167, CNU, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
#else
  {
    152, CNU, CNU, CNU, CNU,
    
  },
  {
    134,  87,  95, CNU, CNU,
    
  },
  {
    118, 102, 107,  86, CNU,
    
  },
#endif
};

#if AMP
// initial probability for AMP split position (X)
static const UChar
INIT_CU_X_POS[3][NUM_CU_X_POS_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    119, 103,
    
  },
  {
    119, 103,
    
  },
};

// initial probability for AMP split position (Y)
static const UChar
INIT_CU_Y_POS[3][NUM_CU_Y_POS_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    119, 119,
    
  },
  {
    119, 103,
    
  },
};
#endif

// initial probability for prediction mode
static const UChar
INIT_PRED_MODE[3][NUM_PRED_MODE_CTX] =
{
#if PREDTYPE_CLEANUP 
  {
    CNU,
    
  },
  {
    114,
    
  },
  {
    98,
    
  },
#else
  {
    CNU, CNU,
    
  },
  {
    CNU, 113,
    
  },
  {
    CNU, CNU,
    
  },
#endif
};

// initial probability for intra direction of luma
static const UChar
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX] =
{
#if BYPASS_FOR_INTRA_MODE
  {
    167,
    
  },
  {
    119,
    
  },
  {
    150,
    
  },
#else
  {
    136, 134, 119,
    
  },
  {
    119, 119,  87,
    
  },
  {
    119, 119,  87,
    
  },
#endif
};

// initial probability for intra direction of chroma
static const UChar
INIT_CHROMA_PRED_MODE[3][NUM_CHROMA_PRED_CTX] =
{
  {
    53, 103,
    
  },
  {
    85,  87,
    
  },
  {
    101,  87,
    
  },
};

// initial probability for temporal direction
static const UChar
INIT_INTER_DIR[3][NUM_INTER_DIR_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    41,  39,  38,  36,
    
  },
};

// initial probability for motion vector difference
static const UChar
INIT_MVD[3][NUM_MV_RES_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    120, 166,
    
  },
  {
    135, 166,
    
  },
};

// initial probability for reference frame index
static const UChar
INIT_REF_PIC[3][NUM_REF_NO_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    102, 118, 103, CNU,
    
  },
  {
    118, 118, 134, CNU,
    
  },
};

// initial probability for dQP
static const UChar
INIT_DQP[3][NUM_DELTA_QP_CTX] =
{
  {
    CNU, CNU, CNU, 
    
  },
  {
    CNU, CNU, CNU, 
    
  },
  {
    CNU, CNU, CNU, 
    
  },
};

static const UChar
#if CHROMA_CBF_CTX_REDUCTION
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX] =
#else
INIT_QT_CBF[3][3*NUM_QT_CBF_CTX] =
#endif
{
  {
     73,  74, CNU, CNU, CNU,
#if CHROMA_CBF_CTX_REDUCTION==0
     54,  70, 117, CNU, CNU,
#endif
     55,  86, 133, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
#if CHROMA_CBF_CTX_REDUCTION==0
     82,  99, 117, CNU, CNU,
#endif
    114,  84, 117, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
#if CHROMA_CBF_CTX_REDUCTION==0
     82,  52, 117, CNU, CNU,
#endif
    114,  68, 117, CNU, CNU,
    
  },
};

static const UChar
INIT_QT_ROOT_CBF[3][NUM_QT_ROOT_CBF_CTX] =
{
  {
    CNU,
    
  },
  {
    39,
    
  },
  {
    39,
    
  },
};

#if MODIFIED_LAST_XY_CODING
static const UChar
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =
{
  {
    72,  72,  71,  72, 104,  89,  71,  88,  89,  59,  73,  86,  89, 106,  60,  59,  43,  55,
    54,  70,  53,  53,  87,  71,  69,  54,  88,  73,  72,  53, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    57,  72,  71,  72,  57,  72, 102,  88,  73,  73,  72, 102, 103,  73,  89,  73,  57,  87,
    54,  70,  54, 101,  71,  55,  70, 116, 103,  72,  72, 119, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    88,  72,  71,  72,  57,  72, 102,  88,  73,  73,  72, 118, 103,  73,  89,  73,  57,  87,
    54,  70,  69,  85,  71,  55,  70,  85, 103,  72,  72, 119, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
};
#else
static const UChar
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =
{
  {
    166, 182, 152, 166, 134, 150, 181, 164, 164, 164, 179, 164, 179, 178, 178, 162, 162, 178, 178,
    184, 184, 170, 170, 167, 136, 169, 150, 150, 134, 134, 165, CNU, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    181, 151, 167, 166, 181, 182, 119, 165, 181, 181, 165, 134, 120, 119, 165, 165, 180, 163, 163,
    169, 184, 185, 169, 167, 184, 107, 120, 183, 182, 182, 117, CNU, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    165, 182, 167, 166, 166, 182, 119, 165, 181, 181, 165, 165, 104, 119, 165, 165, 180, 163, 178,
    184, 184, 185, 153, 183, 168, 106, 120, 167, 182, 182, 117, CNU, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
};
#endif

#if MULTI_LEVEL_SIGNIFICANCE
static const UChar
INIT_SIG_CG_FLAG[3][2 * NUM_SIG_CG_FLAG_CTX] = 
{
  {
    83, 122,
    98, 121,
    
  },
  {
    99, 120,
    67, 119,
    
  },
  {
    99, 120,
    67, 119,
    
  },
};
#endif

#if SIGMAP_CTX_RED
static const UChar
INIT_SIG_FLAG_LUMA[3][NUM_SIG_FLAG_CTX_LUMA] =
{
  {
    74,  73,  88,  72,  72,  55,  71,  54,  71,  88, 103,  71,  53,  87, 134,  86,  84,  70,  68,  89,  90,  84,  88,  74, 130, 118,  88,
    
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53, 118, 134, 118, 101,  68,  85, 101, 116, 100,  68,  67, 136, 168, 147, 150, 120, 115, 118, 119,
    
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53,  71, 103, 118, 101,  68,  85, 101, 116, 116,  68,  67, 152, 168, 147, 150, 120, 115, 118, 119,
    
  },
};

static const UChar
INIT_SIG_FLAG_CHROMA[3][NUM_SIG_FLAG_CTX_CHROMA] =
{
  {
    120,  87, 149,  70,  52, 118, 133, 116, 114, 129, 132, 162, 115,  51, 115,  66, 120,  74, 115,  87,  89,
    
  },
  {
    136, 102,  70,  53,  67, 117, 102, 117, 115, 114,  84, 115,  99, 100,  83, 114, 152, 168, 131, 150, 120,
    
  },
  {
    136, 102,  86,  84,  67, 117, 102, 117, 115,  99, 100, 115,  99, 100,  83, 114, 152, 152, 131, 150, 120,
    
  },
};
#else
#if MULTI_LEVEL_SIGNIFICANCE
static const UChar
INIT_SIG_FLAG[3][2 * NUM_SIG_FLAG_CTX] =
{  
  // I-pic
  {
    // Luma 4x4
    106,  89,  88,  55,
     73,  88,  72,  55,
     56,  56,  88,  71,
     38,  54,  71,
    // Luma 8x8
     73, 119, 118,  68,
    119, 119, 118,  68,
     86, 102, 118,  84,
     52,  83,  69,  69,
    // Luma 16x16 & 32x32, 1st 3 coeffs
    107,  58,  73,
    // Luma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Luma 32x32
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 4x4
    154, 105, 119,  52,
     88, 119, 119,  53,
     86,  71,  87,  70,
     51,  36, 118,
    // Chroma 8x8
    120, 118, 131, 130,
     70, 133, 148, 146,
     52,  84,  70,  68,
     49, 130,  83, 151,
    // Chroma 16x16 & 32x32, 1st 3 coeffs
    122, 103, 119,
    // Chroma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 32x32
    CNU, CNU, CNU, CNU, CNU,
  },
  // P-pic
  {
    // Luma 4x4
    169, 120, 103, 102,
    103, 103, 103,  87,
    102,  87, 103, 104,
     53,  70,  87,
    // Luma 8x8
    167,  87, 102,  53,
    117, 102, 102,  85,
     84, 100,  70,  86,
     67,  99, 117, 102,
    // Luma 16x16 & 32x32, 1st 3 coeffs
    169, 104, 119,
    // Luma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Luma 32x32
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 4x4
    154, 104,  87,  52,
     87, 118, 102,  53,
     69,  85, 135, 183,
     67,  68, 134,
    // Chroma 8x8
    167, 118, 116,  83,
     68, 132, 116,  99,
     83, 131, 133,  84,
    146, 163, 117, 165,
    // Chroma 16x16 & 32x32, 1st 3 coeffs
    185, 120, 103,
    // Chroma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 32x32
    CNU, CNU, CNU, CNU, CNU,
  },
  // B-pic
  {
    // Luma 4x4
    169, 136, 103, 102,
    103, 103, 103,  87,
    102,  87, 104, 104,
     53,  70,  87,
    // Luma 8x8
    120,  87, 102,  68,
     86, 118, 118,  85,
     84,  85, 102,  70,
     67,  68,  70, 102,
    // Luma 16x16 & 32x32, 1st 3 coeffs
    169, 120, 119,
    // Luma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Luma 32x32
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 4x4
    170, 120,  87,  53,
     87, 118, 118,  53,
     53,  70, 135, 183,
     67,  52, 102,
    // Chroma 8x8
    120, 118, 116, 131,
     84,  85,  84,  68,
     83, 115, 117, 117,
    114, 115, 149, 117,
    // Chroma 16x16 & 32x32, 1st 3 
    170, 120, 103,
    // Chroma 16x16
    CNU, CNU, CNU, CNU, CNU,
    // Chroma 32x32
    CNU, CNU, CNU, CNU, CNU,
  }
};
#else
static const UChar
INIT_SIG_FLAG[3][2 * NUM_SIG_FLAG_CTX] =
{
  {
    // Luma 4x4
    106,  89,  88,  55,
     73,  88,  72,  55,
     56,  56,  88,  71,
     38,  54,  71,
    // Luma 8x8
     73, 119, 118,  68,
    119, 119, 118,  68,
     86, 102, 118,  84,
     52,  83,  69,  69,
    // Luma 16x16
    107,  58,  73,  68,
    119, 104,  89, 122,
     98, 134, 119, 104,
     89,
    // Chroma 4x4
    154, 105, 119,  52,
     88, 119, 119,  53,
     86,  71,  87,  70,
     51,  36, 118,
    // Chroma 8x8
    120, 118, 131, 130,
     70, 133, 148, 146,
     52,  84,  70,  68,
     49, 130,  83, 151,
    // Chroma 16x16
    122, 103, 119,  83,
    119, 103, 136, 153,
    113, 134, 119, 103,
    136,
    
  },
  {
    // Luma 4x4
    169, 120, 103, 102,
    103, 103, 103,  87,
    102,  87, 103, 104,
     53,  70,  87,
    // Luma 8x8
    167,  87, 102,  53,
    117, 102, 102,  85,
     84, 100,  70,  86,
     67,  99, 117, 102,
    // Luma 16x16
    169, 104, 119, 162,
    119, 135, 120, 121,
    146, 118,  87, 135,
    120,
    // Chroma 4x4
    154, 104,  87,  52,
     87, 118, 102,  53,
     69,  85, 135, 183,
     67,  68, 134,
    // Chroma 8x8
    167, 118, 116,  83,
     68, 132, 116,  99,
     83, 131, 133,  84,
    146, 163, 117, 165,
    // Chroma 16x16
    185, 120, 103, 132,
    119, 135, 104, 152,
    146, 102, 118, 119,
    135,
    
  },
  {
    // Luma 4x4
    169, 136, 103, 102,
    103, 103, 103,  87,
    102,  87, 104, 104,
     53,  70,  87,
    // Luma 8x8
    120,  87, 102,  68,
     86, 118, 118,  85,
     84,  85, 102,  70,
     67,  68,  70, 102,
    // Luma 16x16
    169, 120, 119, 131,
    119, 135, 120, 137,
    146, 118, 119, 135,
    120,
    // Chroma 4x4
    170, 120,  87,  53,
     87, 118, 118,  53,
     53,  70, 135, 183,
     67,  52, 102,
    // Chroma 8x8
    120, 118, 116, 131,
     84,  85,  84,  68,
     83, 115, 117, 117,
    114, 115, 149, 117,
    // Chroma 16x16
    170, 120, 103, 132,
    135, 135, 120, 121,
    146, 118, 134, 119,
    167,
    
  },
};
#endif
#endif

#if COEFF_CTXSET_RED
#if COEFF_CTX_RED
static const UChar
INIT_ONE_FLAG_LUMA[3][NUM_ONE_FLAG_CTX_LUMA] =
{
  {
    104,  68, 116,  86, 104, 132,  86,  87, 105, 134,  87, 103, 102,  66, 114,  68,  87,  84, 100, 101,  72,  69, 101,  86,
    
  },
  {
    119, 179, 179, 164, 119,  85, 117, 149, 136, 103, 103, 103, 133,  98, 114, 115, 118,  99, 115, 116,  87, 100,  85, 117,
    
  },
  {
    119, 179, 148, 164, 119,  85, 117, 149, 136,  87, 103, 103, 133,  98, 114, 115, 118,  99, 115, 100,  87,  84,  85,  85,
    
  },
};

static const UChar
INIT_ONE_FLAG_CHROMA[3][NUM_ONE_FLAG_CTX_CHROMA] =
{
  {
    104, 130, 147, 149, 104, 196, 100, 165,
    
  },
  {
    135, 146, 147, 164, 119, 148, 116, 133,
    
  },
  {
    135, 177, 147, 164, 119, 132, 148, 149,
    
  },
};

static const UChar
INIT_ABS_FLAG_LUMA[3][NUM_ABS_FLAG_CTX_LUMA] =
{
  {
    86, 103,  73, 102, 103,  73, 103,  88,  89, 115, 117, 103, 117, 118, 103, 102, 103,  72,
    
  },
  {
    84, 102,  88, 117, 118, 104, 103, 119, 136,  83, 116, 118, 100, 117,  87,  85,  86, 103,
    
  },
  {
    84, 102,  88, 117, 118, 104,  87, 119, 136,  83, 116, 118,  84, 117,  87,  69,  86,  87,
    
  },
};

static const UChar
INIT_ABS_FLAG_CHROMA[3][NUM_ABS_FLAG_CTX_CHROMA] =
{
  {
    101, 103, 104, 101, 167, 121,
    
  },
  {
    84, 118, 120, 117, 150, 120,
    
  },
  {
    84, 118, 120, 117, 150, 120,
    
  },
};
#else
static const UChar
INIT_ONE_FLAG_LUMA[3][NUM_ONE_FLAG_CTX_LUMA] =
{
  {
    104,  99,  84,  69, 102,
    103, 117,  86, 102, 118,
    105, 119, 103, 103, 119,
     85,  66,  98, 114, 115,
    102, 101, 101, 148, 132,
    119, 134, 134, 134, 134,
  },
  {
    119, 180, 179, 179, 164,
    119, 117, 164, 164, 133,
    152, 119, 135, 119, 103,
    116, 146,  98,  98, 114,
    102, 147, 116, 147, 116,
    135, 118, 118, 134, 118,
  },
  {
    119, 196, 179, 179, 164,
    119, 117, 117, 133, 149,
    121, 119, 103, 103, 119,
    132, 146, 114,  98,  83,
    118, 132, 116, 116, 132,
    119, 134, 118, 134, 134,
  }
};

static const UChar
INIT_ONE_FLAG_CHROMA[3][NUM_ONE_FLAG_CTX_CHROMA] =
{
  {
    104,  99, 115, 164, 118,
    120,  85, 117, 117, 134,
  },
  {
    135, 178, 178, 179, 164,
    103, 179, 179, 115, 132,
  },
  {
    151, 178, 178, 179, 164,
    135, 148, 148, 164, 133,
  }
};

static const UChar
INIT_ABS_FLAG_LUMA[3][NUM_ABS_FLAG_CTX_LUMA] =
{
  {
    101, 119,  88, 120,  74,
    102, 134, 119,  88,  74,
    119,  88,  88, 104,  90,
     67, 116, 101, 102,  71,
    117, 102, 102, 118, 119,
    134, 119, 119, 119,  88,
  },
  {
     52,  54,  87, 135, 105,
     85,  86, 134, 119, 168,
    119, 119, 119, 135, 121,
     82, 115,  85, 102,  87,
     84,  85, 117, 118, 119,
    118, 134,  87, 119, 103,
  },
  {
     52,  70, 119, 135, 105,
    117, 102, 119, 119, 121,
    119, 119, 135, 104, 137,
     98,  68,  85, 102, 118,
    116,  85,  86, 118, 119,
    118,  87, 119, 119, 135,
  }
};

static const UChar
INIT_ABS_FLAG_CHROMA[3][NUM_ABS_FLAG_CTX_CHROMA] =
{
  {
    101, 134, 104,  73, 122,
    165, 150, 103, 120, 169,
  },
  {
     83, 102, 151, 136, 169,
     85, 118, 103,  72, 106,
  },
  {
     99, 118, 151, 136, 169,
    133, 102, 103, 120, 137,
  }
};
#endif
#else
#if COEFF_CTX_RED
static const UChar
INIT_ONE_FLAG[3][2*NUM_ONE_FLAG_CTX] =
{
  {
    104,  99,  84,  69,
    103, 117,  86, 102,
    105, 119, 103, 103,
     85,  66,  98, 114,
    102, 101, 101, 148,
    119, 134, 134, 134,
    104,  99, 115, 164,
    120,  85, 117, 117,
    153,  71, 119,  71,
    132,  48, 113, 113,
    134, 147, 148, 116,
    119, 166, 101, 134,
    
  },
  {
    119, 180, 179, 179,
    119, 117, 164, 164,
    152, 119, 135, 119,
    116, 146,  98,  98,
    102, 147, 116, 147,
    135, 118, 118, 134,
    135, 178, 178, 179,
    103, 179, 179, 115,
     40,  71, 151,  22,
    164, 176,  81,  97,
     69,  83, 180,  99,
     71,  18,   2,   5,
    
  },
  {
    119, 196, 179, 179,
    119, 117, 117, 133,
    121, 119, 103, 103,
    132, 146, 114,  98,
    118, 132, 116, 116,
    119, 134, 118, 134,
    151, 178, 178, 179,
    135, 148, 148, 164,
    137,  86, 135,  72,
    116,  97,  97,  97,
     85, 147, 180, 100,
     54,  20,  20,   4,
    
  },
};

static const UChar
INIT_ABS_FLAG[3][2*NUM_ABS_FLAG_CTX] =
{
  {
    101, 119,  88,
    102, 134, 119,
    119,  88,  88,
     67, 116, 101,
    117, 102, 102,
    134, 119, 119,
    101, 134, 104,
    165, 150, 103,
    166, 151, 152,
    114, 163, 148,
    148, 165, 165,
    134, 119, 119,
    
  },
  {
     52,  54,  87,
     85,  86, 134,
    119, 119, 119,
     82, 115,  85,
     84,  85, 117,
    118, 134,  87,
     83, 102, 151,
     85, 118, 103,
     87, 118, 104,
     98, 100,  36,
     84,  37, 149,
      1,  67,  70,
    
  },
  {
     52,  70, 119,
    117, 102, 119,
    119, 119, 135,
     98,  68,  85,
    116,  85,  86,
    118,  87, 119,
     99, 118, 151,
    133, 102, 103,
    103,  55,  87,
     98,  83, 117,
     84, 133,  69,
    181,  85,  37,
    
  },
};
#else
static const UChar
INIT_ONE_FLAG[3][2*NUM_ONE_FLAG_CTX] =
{
  {
    104,  99,  84,  69, 102, 103, 117,  86, 102, 118, 105, 119, 103, 103, 119,  85,  66,  98, 114, 115, 102, 101, 101, 148, 132, 119, 134, 134, 134, 134,
    104,  99, 115, 164, 118, 120,  85, 117, 117, 134, 153,  71, 119,  71, 119, 132,  48, 113, 113, 114, 134, 147, 148, 116, 164, 119, 166, 101, 134, 166,
    
  },
  {
    119, 180, 179, 179, 164, 119, 117, 164, 164, 133, 152, 119, 135, 119, 103, 116, 146,  98,  98, 114, 102, 147, 116, 147, 116, 135, 118, 118, 134, 118,
    135, 178, 178, 179, 164, 103, 179, 179, 115, 132,  40,  71, 151,  22,   4, 164, 176,  81,  97, 161,  69,  83, 180,  99,  83,  71,  18,   2,   5,   4,
    
  },
  {
    119, 196, 179, 179, 164, 119, 117, 117, 133, 149, 121, 119, 103, 103, 119, 132, 146, 114,  98,  83, 118, 132, 116, 116, 132, 119, 134, 118, 134, 134,
    151, 178, 178, 179, 164, 135, 148, 148, 164, 133, 137,  86, 135,  72,  69, 116,  97,  97,  97, 114,  85, 147, 180, 100, 100,  54,  20,  20,   4,   4,
    
  },
};

static const UChar
INIT_ABS_FLAG[3][2*NUM_ABS_FLAG_CTX] =
{
  {
    101, 119,  88, 120,  74, 102, 134, 119,  88,  74, 119,  88,  88, 104,  90,  67, 116, 101, 102,  71, 117, 102, 102, 118, 119, 134, 119, 119, 119,  88,
    101, 134, 104,  73, 122, 165, 150, 103, 120, 169, 166, 151, 152, 168, 138, 114, 163, 148, 149, 150, 148, 165, 165, 166, 183, 134, 119, 119, 183, 167,
    
  },
  {
     52,  54,  87, 135, 105,  85,  86, 134, 119, 168, 119, 119, 119, 135, 121,  82, 115,  85, 102,  87,  84,  85, 117, 118, 119, 118, 134,  87, 119, 103,
     83, 102, 151, 136, 169,  85, 118, 103,  72, 106,  87, 118, 104,  23, 106,  98, 100,  36, 185, 199,  84,  37, 149, 182,   5,   1,  67,  70, 121,  73,
    
  },
  {
     52,  70, 119, 135, 105, 117, 102, 119, 119, 121, 119, 119, 135, 104, 137,  98,  68,  85, 102, 118, 116,  85,  86, 118, 119, 118,  87, 119, 119, 135,
     99, 118, 151, 136, 169, 133, 102, 103, 120, 137, 103,  55,  87, 104,  73,  98,  83, 117, 199, 232,  84, 133,  69, 150,  70, 181,  85,  37, 103, 199,
    
  },
};
#endif
#endif

// initial probability for motion vector predictor index
static const UChar
INIT_MVP_IDX[3][NUM_MVP_IDX_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    134, CNU,
    
  },
  {
    134, CNU,
    
  },
};

// initial probability for ALF flag
static const UChar
INIT_ALF_FLAG[3][NUM_ALF_FLAG_CTX] =
{
  {
    118,
    
  },
  {
    102,
    
  },
  {
    102,
    
  },
};

// initial probability for ALF side information (unsigned)
static const UChar
INIT_ALF_UVLC[3][NUM_ALF_UVLC_CTX] =
{
  {
    120, 119,
    
  },
  {
    119, 119,
    
  },
  {
    119, 119,
    
  },
};

// initial probability for ALF side information (signed)
static const UChar
INIT_ALF_SVLC[3][NUM_ALF_SVLC_CTX] =
{
  {
    139, 119, 124,
    
  },
  {
     90, 119, 140,
    
  },
  {
     90, 119, 124,
    
  },
};

#if SAO
// initial probability for SAO flag
static const UChar
INIT_SAO_FLAG[3][NUM_SAO_FLAG_CTX] =
{
  {
    119,
    
  },
  {
    102,
    
  },
  {
    102,
    
  },
};

// initial probability for SAO side information (unsigned)
static const UChar
INIT_SAO_UVLC[3][NUM_SAO_UVLC_CTX] =
{
  {
     61, 104,
    
  },
  {
    168, 120,
    
  },
  {
    184, 120,
    
  },
};

// initial probability for SAO side information (signed)
static const UChar
INIT_SAO_SVLC[3][NUM_SAO_SVLC_CTX] =
{
  {
    171, 119, 199,
    
  },
  {
    169, 119, 151,
    
  },
  {
    169, 119, 151,
    
  },
};
#endif

static const UChar
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX] =
{
  {
    CNU, 162, 148, 100, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    CNU,  71,  86,  55, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    CNU, 102,  86,  86, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
};

//! \}

#else

// initial probability for split flag
static const Short
INIT_SPLIT_FLAG[3][NUM_SPLIT_FLAG_CTX][2] =
{
  {
    {   -7,   68 }, {  -10,   87 }, {  -10,  105 }
  },
  {
    {  -14,   71 }, {   -6,   73 }, {   -6,   91 }
  },
  {
    {  -14,   71 }, {   -7,   74 }, {  -10,   92 }
  }
};

// initial probability for skip flag
static const Short
INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }
  }
};

// initial probability for skip flag
static const Short
INIT_ALF_CTRL_FLAG[3][NUM_ALF_CTRL_FLAG_CTX][2] =
{
  {
    {    0,   64 }
  },
  {
    {    0,   64 }
  },
  {
    {    0,   64 }
  }
};

// initial probability for merge flag
static const Short
INIT_MERGE_FLAG_EXT[3][NUM_MERGE_FLAG_EXT_CTX][2] =
{
  {
    {    0,   64 }
  },
  {
    {    0,   64 }
  },
  {
    {    0,   64 }
  }
};

static const Short
INIT_MERGE_IDX_EXT[3][NUM_MERGE_IDX_EXT_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    1,   65 }, {    6,   42 }, {   -7,   75 }, {   -4,   72 }
  }
};

// initial probability for PU size
static const Short
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX][2] =
{
#if PREDTYPE_CLEANUP
  {
    {    0,   73 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {   -1,   64 }, {   -3,   63 }, {    6,   78 }, {    0,   64 }
  },
  {
    {    6,   50 }, {   -1,   56 }, {   13,   53 }, {  -11,   76 }
  }
#else
  {
    {    0,   73 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, 
    {    0,   64 }
  },
  {
    {   -1,   64 }, {   -3,   63 }, {    6,   78 }, {    0,   64 }, 
    {    0,   64 }
  },
  {
    {    6,   50 }, {   -1,   56 }, {   13,   53 }, {  -11,   76 }, 
    {  -11,   70 }
  }
#endif
};

#if AMP
// initial probability for AMP split position (X)
static const Short
INIT_CU_X_POS[3][NUM_CU_X_POS_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {   -1,   59 }, {    0,   63 }
  },
  {
    {   -1,   55 }, {   -3,   67 }
  }
};

// initial probability for AMP split position (Y)
static const Short
INIT_CU_Y_POS[3][NUM_CU_Y_POS_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   57 }, {   -2,   66 }
  },
  {
    {   -3,   61 }, {   -3,   66 }
  }
};
#endif

// initial probability for prediction mode
static const Short
INIT_PRED_MODE[3][NUM_PRED_MODE_CTX][2] =
{
#if PREDTYPE_CLEANUP 
  {
    {    0,   64 }
  },
  {
    {  -25,   89 }
  },
  {  
    {    0,   64 }
  }
#else
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {  -25,   89 }
  },
  {
    {    0,   64 }, {    0,   64 }
  }
#endif
};

// initial probability for intra direction of luma
static const Short
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX][2] =
{
#if BYPASS_FOR_INTRA_MODE
  {
    {    2,   54 }
  },
  {
    {    0,   50 }
  },
  {
    {    0,   51 }
  }
#else
  {
    {    2,   54 }, {  -3,   65  }, {   -3,   65 }
  },
  {
    {    0,   50 }, {  -2,   61  }, {   -2,   61 }
  },
  {
    {    0,   51 }, {  1,   55   }, {    1,   55 }
  }
#endif
};

// initial probability for intra direction of chroma
static const Short
INIT_CHROMA_PRED_MODE[3][NUM_CHROMA_PRED_CTX][2] =
{
  {
    {  0,   64 }, {   0,   64 }
  },
  {
    {  0,   64 }, {   0,   64 }
  },
  {
    {  0,   64 }, {   0,   64 }
  }
};

// initial probability for temporal direction
static const Short
INIT_INTER_DIR[3][4][2] =
{
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {   -2,   58 }, {   -5,   70 }, {   -9,   85 }, {    1,   61 }
  }
};

// initial probability for motion vector difference
static const
Short INIT_MVD[3][NUM_MV_RES_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }
  }
};

// initial probability for reference frame index
static const Short
INIT_REF_PIC[3][NUM_REF_NO_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {   -6,   59 }, {  -17,   96 }, {    1,   59 }, {    0,   64 }
  },
  {
    {   -9,   55 }, {  -12,   86 }, {  -18,   55 }, {    0,   64 }
  }
};

// initial probability for dQP
static const Short
INIT_DQP[3][NUM_DELTA_QP_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, 
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, 
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, 
  }
};

static const Short
#if CHROMA_CBF_CTX_REDUCTION
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX][2] =
#else
INIT_QT_CBF[3][3*NUM_QT_CBF_CTX][2] =
#endif
{
  {
    {  -22,  116 }, {   -5,   75 }, {  -16,  112 }, {  -16,  111 }, {  -32,  165 },
#if CHROMA_CBF_CTX_REDUCTION==0
    {  -35,  116 }, {  -12,   61 }, {   -9,   73 }, {  -10,   75 }, {  -14,   96 }, 
#endif
    {  -29,  104 }, {  -12,   59 }, {   -5,   65 }, {   -6,   67 }, {  -11,   90 }
  },
  {
    {  -18,   98 }, {  -41,  120 }, {  -29,  117 }, {  -23,  108 }, {  -35,  143 },
#if CHROMA_CBF_CTX_REDUCTION==0
    {  -46,  114 }, {  -42,  119 }, {  -11,   74 }, {  -19,   90 }, {  -42,  139 }, 
#endif
    {  -43,  107 }, {  -41,  118 }, {  -17,   86 }, {  -25,  101 }, {  -14,   91 }
  },
  {
    {  -11,   80 }, {  -32,   83 }, {  -19,   89 }, {  -16,   85 }, {  -19,  102 },
#if CHROMA_CBF_CTX_REDUCTION==0
    {  -22,   52 }, {  -48,  123 }, {   -7,   68 }, {  -37,  121 }, {  -58,  164 }, 
#endif
    {  -19,   45 }, {  -48,  123 }, {  -21,   94 }, {   -9,   73 }, {  -42,  138 }
  }
};
static const Short
INIT_QT_ROOT_CBF[3][NUM_QT_ROOT_CBF_CTX][2] =
{
  {
    {    0,   64 }
  },
  {
    {  -22,   85 }
  },
  {
    {  -36,  103 }
  }
};

#if MODIFIED_LAST_XY_CODING
static const Short
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY][2] =
{ 
  {
    {  -12,  92  },  {  -10,  87  },  {  -10,  80  },  {  -13,  92  },
    {  -9,  87  },   {  -7,  86  },   {  -4,  67  },   {  -12,  92  },
    {  -15,  106  }, {  -15,  109  }, {  -14,  99  },  {  -11,  78  },
    {  -16,  103  }, {  -17,  117  }, {  -22,  131  }, {  -21,  123  },
    {  -19,  112  }, {  -23,  102  }, {  -37,  118  }, {  -35,  113  },
    {  -53,  132  }, {  -31,  106  }, {  -13,  89  },  {  -21,  99  },
    {  0,  64  },    {  -35,  125  }, {  -22,  110  }, {  -18,  104  },
    {  -14,  96  },  {  0,  64  },    {  0,  64  },    {  0,  64  },
    {  0,  64  },    {  0,  64  },    {  0,  64  },    {  0,  64  }
  },
  {
    {  -9,  81  },   {  -14,  86  },  {  -25,  90  },  {  -10,  88  },
    {  -17,  99  },  {  -18,  90  },  {  -30,  103  }, {  -3,  74  },
    {  -14,  99  },  {  -23,  114  }, {  -17,  95  },  {  -24,  87  },
    {  -12,  83  },  {  -8,  82  },   {  -11,  97  },  {  -10,  98  },
    {  -27,  122  }, {  -57,  148  }, {  -21,  91  },  {  -22,  90  },
    {  -35,  100  }, {  -11,  75  },  {  -22,  100  }, {  -27,  110  },
    {  0,  64  },    {  3,  45  },    {  -14,  93  },  {  -23,  110  },
    {  -32,  138  }, {  0,  64  },    {  0,  64  },    {  0,  64  },
    {  0,  64  },    {  0,  64  },    {  0,  64  },    {  0,  64  }
  },
  {
    {  -9,  81  },   {  -14,  86  },  {  -25,  90  },  {  -10,  88  },
    {  -17,  99  },  {  -18,  90  },  {  -30,  103  }, {  -3,  74  },
    {  -14,  99  },  {  -23,  114  }, {  -17,  95  },  {  -24,  87  },
    {  -12,  83  },  {  -8,  82  },   {  -11,  97  },  {  -10,  98  },
    {  -27,  122  }, {  -57,  148  }, {  -21,  91  },  {  -22,  90  },
    {  -35,  100  }, {  -11,  75  },  {  -22,  100  }, {  -27,  110  },
    {  0,  64  },    {  3,  45  },    {  -14,  93  },  {  -23,  110  },
    {  -32,  138  }, {  0,  64  },    {  0,  64  },    {  0,  64  },
    {  0,  64  },    {  0,  64  },    {  0,  64  },    {  0,  64  }
  }
};
#else
static const Short
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY][2] =
{
  {
    {   14,   25 }, {   10,   38 }, {   12,   41 },
    {   22,   25 }, {   10,   37 }, {    9,   40 },
    {   17,   19 }, {   15,   21 }, {   16,   18 }, {   17,   16 }, {   18,   13 }, {   16,   18 },
    {   23,   -2 }, {   20,   -2 }, {   24,   -9 }, {   24,   -7 }, {   18,   10 }, {    9,   24 }, {   22,   -7 },
    {   37,    9 }, {   35,   14 }, {   53,   -5 },
    {   31,   21 }, {   13,   39 }, {   21,   28 },
    {   35,    2 }, {   22,   17 }, {   18,   23 }, {    9,   40 }, {   14,   32 }, {    7,   44 },
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {   12,   35 }, {   12,   40 }, {   13,   47 },
    {   21,   16 }, {   24,   10 }, {   23,   15 },
    {   13,   34 }, {   22,   10 }, {   29,   10 }, {   29,   -8 }, {   27,   -5 }, {   20,    7 },
    {   25,   13 }, {   30,   -8 }, {   35,  -20 }, {   37,  -29 }, {   42,  -41 }, {   42,  -45 }, {   43,  -46 },

    {   21,   36 }, {   22,   37 }, {   35,   27 },
    {   11,   53 }, {   22,   27 }, {   27,   18 },
    {   -3,   83 }, {   14,   35 }, {   23,   18 }, {   27,    3 }, {   32,  -11 }, {   17,   11 },
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {   12,   35 }, {   12,   40 }, {   13,   47 },
    {   21,   16 }, {   24,   10 }, {   23,   15 },
    {   13,   34 }, {   22,   10 }, {   29,   10 }, {   29,   -8 }, {   27,   -5 }, {   20,    7 },
    {   25,   13 }, {   30,   -8 }, {   35,  -20 }, {   37,  -29 }, {   42,  -41 }, {   42,  -45 }, {   43,  -46 },

    {   21,   36 }, {   22,   37 }, {   35,   27 },
    {   11,   53 }, {   22,   27 }, {   27,   18 },
    {   -3,   83 }, {   14,   35 }, {   23,   18 }, {   27,    3 }, {   32,  -11 }, {   17,   11 },
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  }
};
#endif

#if MULTI_LEVEL_SIGNIFICANCE
static const Short
INIT_SIG_CG_FLAG[3][2 * NUM_SIG_CG_FLAG_CTX][2] = 
{
  //I-pic
  {
    //Luma significant_coeff_group_flag
    {  -6,  63}, {  14,  58},
    //Chroma significant_coeff_group_flag
    { -25,  83}, {  -8,  83},
  },
  //P-pic
  {
    //Luma significant_coeff_group_flag
    { -12,  66}, {  12,  56}, 
    //Chroma significant_coeff_group_flag
    { -17,  69}, {  -3,  78},
  },
  //B-pic
  {
    //Luma significant_coeff_group_flag
    {  -7,  63}, {  13,  57},
    //Chroma significant_coeff_group_flag
    { -23,  80}, {  -8,  83}
  }
};
#endif

#if SIGMAP_CTX_RED
static const Short
INIT_SIG_FLAG_LUMA[3][NUM_SIG_FLAG_CTX_LUMA][2] =
{
  {
    // Luma 4x4
    { -11, 103 }, { -10, 94 }, { -11, 93 }, { -11, 92 },
    { -9, 87 },   { -14, 89 }, { -13, 91 }, { -19, 94 },
    { -10, 89 },
    // Luma 8x8
    { -8, 90 }, { -4, 72 }, { 0, 58 }, { -6, 54 },
    { -8, 80 }, { -4, 70 }, { 3, 54 }, { -3, 55 },
    { -8, 72 }, { -17, 79 }, { -8, 90 },
    // Luma 16x16 & 32X32
    { -15, 119 }, { -4,  49 },{ -2,  72 },{ -15, 112 },
    { -4,  28  }, { -4,  72 },{ -10, 96 }
  },
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -8, 77 }, { -8, 75 },
    { -6, 72 }, { -19, 89 }, { -10, 73 }, { -20, 88 },
    { -4, 69 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 },
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -4, 52 },
    { -16, 72 }, { -24, 88 }, { 2, 61 },
    // Luma 16x16 & 32X32
    {  0,  78 }, {  2, 31 }, { 0,  65 }, { -9, 93 },
    { -4,  20 }, {  1, 57 }, { -1, 72 }
  },
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -8, 77 }, { -8, 75 },
    { -6, 72 }, { -19, 89 }, { -10, 73 }, { -20, 88 },
    { -4, 69 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 },
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -4, 52 },
    { -16, 72 }, { -24, 88 }, { 2, 61 },
    // Luma 16x16 & 32X32
    {  0, 78 }, { 2, 31 }, {  0, 65 }, { -9, 93 },
    { -4, 20 }, { 1, 57 }, { -1, 72 }
  }
};

static const Short
INIT_SIG_FLAG_CHROMA[3][NUM_SIG_FLAG_CTX_CHROMA][2] =
{
  {
    // Chroma 4x4
    { 34, 22 }, { 8, 52 },  { -11, 83 }, { -14, 83 },
    { -59, 149 }, { 60, -35 },
    // Chroma 8x8
    { 14, 48 }, { 6, 50 }, { -30, 103 }, { -35, 74 },
    { -7, 68 }, { 14, 36 }, { 9, 43 }, { 7, 22 },
    { -37, 104 }, { -37, 80 }, { 14, 48 },
    // Chroma 16x16 & 32X32
    { 15, 59 }, { -9, 38 }, { 7, 49 }, { 13, 47 }
  },
  {
    // Chroma 4x4
    { 41, 3 }, { 13, 34 }, { 16, 33 }, { -24, 84 },
    { 4, 43 }, { 6, 52 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 },
    { 3, 36 }, { 25, -6 }, { 27, -2 }, { 11, 36 },
    { -30, 81 }, { -56, 143 }, { 18, 31 },
    // Chroma 16x16 & 32X32
   { 28,  29 }, {  0,  0  }, {  5,  49 }, { 20,  32 }
  },
  {
    // Chroma 4x4
    { 41, 3 }, { 13, 34 }, { 16, 33 }, { -24, 84 },
    { 4, 43 }, { 6, 52 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 },
    { 3, 36 }, { 25, -6 }, { 27, -2 }, { 11, 36 },
    { -30, 81 }, { -56, 143 }, { 18, 31 },
    // Chroma 16x16 & 32X32
    { 28, 29 }, { 0,   0 }, { 5,  49 }, { 20, 32 }
  }
};
#else
#if MULTI_LEVEL_SIGNIFICANCE
static const Short
INIT_SIG_FLAG[3][2 * NUM_SIG_FLAG_CTX][2] =
{  
  // I-pic
  {
    // Luma 4x4
    { -11, 103 }, { -10, 94 }, { -9, 87 }, { -14, 89 }, 
    { -11, 93 }, { -11, 92 }, { -11, 91 }, { -17, 97 }, 
    { -13, 91 }, { -12, 89 }, { -10, 89 }, { -19, 101 }, 
    { -19, 94 }, { -16, 93 }, { -14, 93 },
    // Luma 8x8
    { -8, 90 }, { -4, 72 }, { 0, 58 }, { -6, 54 }, 
    { -8, 80 }, { -4, 70 }, { 1, 57 }, { -4, 54 }, 
    { -8, 72 }, { -3, 63 }, { 3, 54 }, { -3, 55 }, 
    { -17, 79 }, { -5, 59 }, { -1, 56 }, { -1, 53 }, 
    // Luma 16x16 & 32x32, 1st 3 coeffs
    { -15, 119 }, { -14, 104 }, { -15, 106 },     
    // Luma 16x16
    {  -4,  48}, {  -1,  61}, {  -3,  73}, {  -6,  86}, { -12, 106}, 
    // Luma 32x32
    {   5,  24}, {   2,  49}, {  -4,  71}, {  -5,  78}, { -10,  96}, 
    // Chroma 4x4
    { 34, 22 }, { 19, 41 }, { -11, 83 }, { -58, 144 }, 
    { 6, 58 }, { 8, 52 }, { 1, 64 }, { -59, 149 }, 
    { -14, 83 }, { 19, 30 }, { 60, -35 }, { 1, 56 }, 
    { -50, 124 }, { -53, 137 }, { 7, 47 },
    // Chroma 8x8
    { 14, 48 }, { 6, 50 }, { -30, 103 }, { -35, 74 }, 
    { -7, 68 }, { 14, 36 }, { 8, 43 }, { -25, 63 }, 
    { -37, 104 }, { -21, 83 }, { 9, 43 }, { 7, 22 }, 
    { -37, 80 }, { 2, 24 }, { -36, 103 }, { 1, 57 }, 
    // Chroma 16x16 & 32x32, 1st 3 coeffs
    { 15, 59 }, { 7, 56 }, { 5, 57 }, 
    // Chroma 16x16
    {  -7,  51}, {   0,  56}, {   1,  64}, {  -1,  74}, {  -3,  85}, 
    // Chroma 32x32
    { -11,  52}, {   0,  45}, {   0,  56}, {  -1,  64}, {  -3,  74}, 
  },
  // P-pic
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -6, 72 }, { -19, 89 }, 
    { -8, 77 }, { -8, 75 }, { -9, 76 }, { -17, 88 }, 
    { -10, 73 }, { -10, 75 }, { -4, 69 }, { -22, 100 }, 
    { -20, 88 }, { -19, 89 }, { -9, 75 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 }, 
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -8, 55 }, 
    { -16, 72 }, { -10, 62 }, { -1, 49 }, { -4, 52 }, 
    { -24, 88 }, { -20, 84 }, { -10, 70 }, { -3, 59 }, 
    // Luma 16x16 & 32x32, 1st 3 coeffs
    { 0, 78 }, { 0, 66 }, { -3, 68 }, 
    // Luma 16x16
    {  -4,  47}, {   1,  57}, {  -2,  72}, {  -6,  86}, { -15, 113}, 
    // Luma 32x32
    {   2,  22}, {   8,  37}, {   0,  62}, {  -5,  77}, { -10,  93}, 
    // Chroma 4x4
    { 41, 3 }, { 17, 37 }, { 16, 33 }, { -27, 89 }, 
    { 0, 57 }, { 13, 34 }, { 75, -70 }, { 4, 43 }, 
    { -24, 84 }, { -11, 68 }, { 6, 52 }, { 20, 40 }, 
    { -29, 79 }, { -50, 124 }, { 12, 38 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 }, 
    { 3, 36 }, { 25, -6 }, { 21, -3 }, { 37, -34 }, 
    { -30, 81 }, { 36, -36 }, { 27, -2 }, { 11, 36 }, 
    { -56, 143 }, { 20, 3 }, { 16, 19 }, { 19, 46 }, 
    // Chroma 16x16 & 32x32, 1st 3 coeffs
    { 28, 29 }, { 16, 35 }, { 11, 39 }, 
    // Chroma 16x16
    {  -8,  51}, {  -7,  66}, {  -2,  65}, {  -6,  78}, {   9,  60}, 
    // Chroma 32x32
    {  -9,  50}, {  -4,  54}, {  -3,  62}, {  -2,  66}, {  -3,  73}, 
  },
  // B-pic
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -6, 72 }, { -19, 89 }, 
    { -8, 77 }, { -8, 75 }, { -9, 76 }, { -17, 88 }, 
    { -10, 73 }, { -10, 75 }, { -4, 69 }, { -22, 100 }, 
    { -20, 88 }, { -19, 89 }, { -9, 75 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 }, 
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -8, 55 }, 
    { -16, 72 }, { -10, 62 }, { -1, 49 }, { -4, 52 }, 
    { -24, 88 }, { -20, 84 }, { -10, 70 }, { -3, 59 }, 
    // Luma 16x16
    { 0, 78 }, { 0, 66 }, { -3, 68 }, 
    // Luma 16x16
    {  -4,  49}, {  -1,  61}, {  -2,  71}, {  -5,  84}, { -12, 106}, 
    // Luma 32x32
    {   4,  26}, {   3,  47}, {  -4,  68}, {  -6,  81}, {  -9,  94}, 
    // Chroma 4x4
    { 41, 3 }, { 17, 37 }, { 16, 33 }, { -27, 89 }, 
    { 0, 57 }, { 13, 34 }, { 75, -70 }, { 4, 43 }, 
    { -24, 84 }, { -11, 68 }, { 6, 52 }, { 20, 40 }, 
    { -29, 79 }, { -50, 124 }, { 12, 38 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 }, 
    { 3, 36 }, { 25, -6 }, { 21, -3 }, { 37, -34 }, 
    { -30, 81 }, { 36, -36 }, { 27, -2 }, { 11, 36 }, 
    { -56, 143 }, { 20, 3 }, { 16, 19 }, { 19, 46 }, 
    // Chroma 16x16 & 32x32, 1st 3 
    { 28, 29 }, { 16, 35 }, { 11, 39 }, 
    // Chroma 16x16
    {  -8,  53}, {  -1,  58}, {   0,  64}, {   1,  70}, {  -2,  82}, 
    // Chroma 32x32
    { -11,  52}, {   0,  46}, {  -1,  59}, {   0,  63}, {   0,  69}
  }
};
#else
static const Short
INIT_SIG_FLAG[3][2 * NUM_SIG_FLAG_CTX][2] =
{
  {
    // Luma 4x4
    { -11, 103 }, { -10, 94 }, { -9, 87 }, { -14, 89 }, 
    { -11, 93 }, { -11, 92 }, { -11, 91 }, { -17, 97 }, 
    { -13, 91 }, { -12, 89 }, { -10, 89 }, { -19, 101 }, 
    { -19, 94 }, { -16, 93 }, { -14, 93 },
    // Luma 8x8
    { -8, 90 }, { -4, 72 }, { 0, 58 }, { -6, 54 }, 
    { -8, 80 }, { -4, 70 }, { 1, 57 }, { -4, 54 }, 
    { -8, 72 }, { -3, 63 }, { 3, 54 }, { -3, 55 }, 
    { -17, 79 }, { -5, 59 }, { -1, 56 }, { -1, 53 }, 
    // Luma 16x16
    { -15, 119 }, { -14, 104 }, { -15, 106 }, { -4, 49 }, 
    { 0, 62 }, { -2, 72 }, { -7, 88 }, { -15, 112 }, 
    { -4, 28 }, { 1, 54 }, { -4, 72 }, { -7, 82 }, 
    { -10, 96 },
    // Chroma 4x4
    { 34, 22 }, { 19, 41 }, { -11, 83 }, { -58, 144 }, 
    { 6, 58 }, { 8, 52 }, { 1, 64 }, { -59, 149 }, 
    { -14, 83 }, { 19, 30 }, { 60, -35 }, { 1, 56 }, 
    { -50, 124 }, { -53, 137 }, { 7, 47 },
    // Chroma 8x8
    { 14, 48 }, { 6, 50 }, { -30, 103 }, { -35, 74 }, 
    { -7, 68 }, { 14, 36 }, { 8, 43 }, { -25, 63 }, 
    { -37, 104 }, { -21, 83 }, { 9, 43 }, { 7, 22 }, 
    { -37, 80 }, { 2, 24 }, { -36, 103 }, { 1, 57 }, 
    // Chroma 16x16
    { 15, 59 }, { 7, 56 }, { 5, 57 }, { 14, 11 }, 
    { 10, 45 }, { 7, 53 }, { 5, 61 }, { 11, 59 }, 
    { -9, 38 }, { 5, 46 }, { 7, 49 }, { 10, 48 }, 
    { 13, 47 } 
  },
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -6, 72 }, { -19, 89 }, 
    { -8, 77 }, { -8, 75 }, { -9, 76 }, { -17, 88 }, 
    { -10, 73 }, { -10, 75 }, { -4, 69 }, { -22, 100 }, 
    { -20, 88 }, { -19, 89 }, { -9, 75 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 }, 
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -8, 55 }, 
    { -16, 72 }, { -10, 62 }, { -1, 49 }, { -4, 52 }, 
    { -24, 88 }, { -20, 84 }, { -10, 70 }, { -3, 59 }, 
    // Luma 16x16
    { 0, 78 }, { 0, 66 }, { -3, 68 }, { 2, 31 }, 
    { 3, 53 }, { 0, 65 }, { -3, 74 }, { -9, 93 }, 
    { -4, 20 }, { 4, 44 }, { 1, 57 }, { 0, 65 }, 
    { -1, 72 }, 
    // Chroma 4x4
    { 41, 3 }, { 17, 37 }, { 16, 33 }, { -27, 89 }, 
    { 0, 57 }, { 13, 34 }, { 75, -70 }, { 4, 43 }, 
    { -24, 84 }, { -11, 68 }, { 6, 52 }, { 20, 40 }, 
    { -29, 79 }, { -50, 124 }, { 12, 38 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 }, 
    { 3, 36 }, { 25, -6 }, { 21, -3 }, { 37, -34 }, 
    { -30, 81 }, { 36, -36 }, { 27, -2 }, { 11, 36 }, 
    { -56, 143 }, { 20, 3 }, { 16, 19 }, { 19, 46 }, 
    // Chroma 16x16
    { 28, 29 }, { 16, 35 }, { 11, 39 }, { 26, -18 }, 
    { 10, 44 }, { 4, 58 }, { -2, 71 }, { -11, 94 }, 
    { 0, 0 }, { 5, 45 }, { 5, 49 }, { 9, 45 }, 
    { 20, 32 }
  },
  {
    // Luma 4x4
    { -2, 74 }, { -5, 76 }, { -6, 72 }, { -19, 89 }, 
    { -8, 77 }, { -8, 75 }, { -9, 76 }, { -17, 88 }, 
    { -10, 73 }, { -10, 75 }, { -4, 69 }, { -22, 100 }, 
    { -20, 88 }, { -19, 89 }, { -9, 75 },
    // Luma 8x8
    { 2, 61 }, { -1, 56 }, { -2, 51 }, { -15, 67 }, 
    { -9, 66 }, { -5, 58 }, { -1, 49 }, { -8, 55 }, 
    { -16, 72 }, { -10, 62 }, { -1, 49 }, { -4, 52 }, 
    { -24, 88 }, { -20, 84 }, { -10, 70 }, { -3, 59 }, 
    // Luma 16x16
    { 0, 78 }, { 0, 66 }, { -3, 68 }, { 2, 31 }, 
    { 3, 53 }, { 0, 65 }, { -3, 74 }, { -9, 93 }, 
    { -4, 20 }, { 4, 44 }, { 1, 57 }, { 0, 65 }, 
    { -1, 72 }, 
    // Chroma 4x4
    { 41, 3 }, { 17, 37 }, { 16, 33 }, { -27, 89 }, 
    { 0, 57 }, { 13, 34 }, { 75, -70 }, { 4, 43 }, 
    { -24, 84 }, { -11, 68 }, { 6, 52 }, { 20, 40 }, 
    { -29, 79 }, { -50, 124 }, { 12, 38 },
    // Chroma 8x8
    { 18, 31 }, { 22, 11 }, { 13, 10 }, { -32, 58 }, 
    { 3, 36 }, { 25, -6 }, { 21, -3 }, { 37, -34 }, 
    { -30, 81 }, { 36, -36 }, { 27, -2 }, { 11, 36 }, 
    { -56, 143 }, { 20, 3 }, { 16, 19 }, { 19, 46 }, 
    // Chroma 16x16
    { 28, 29 }, { 16, 35 }, { 11, 39 }, { 26, -18 }, 
    { 10, 44 }, { 4, 58 }, { -2, 71 }, { -11, 94 }, 
    { 0, 0 }, { 5, 45 }, { 5, 49 }, { 9, 45 }, 
    { 20, 32 }
  }
};
#endif
#endif

#if COEFF_CTXSET_RED
#if COEFF_CTX_RED
static const Short
INIT_ONE_FLAG_LUMA[3][NUM_ONE_FLAG_CTX_LUMA][2] =
{
  {
    { -9, 84 }, { -16, 60 }, { -11, 59 }, { -9, 61 }, 
    { -5, 79 }, { 0, 47 }, { -5, 66 }, { -7, 70 }, 
    { -7, 92 }, { -8, 79 }, { -4, 74 }, { -4, 75 }, 
    { -6, 62 }, { -12, 39 }, { -12, 48 }, { -13, 53 },  
    { -3, 63 }, { -2, 47 }, { -6, 59 }, { -8, 62 }, 
    { -5, 78 }, { -12, 85 }, { -11, 79 }, { -9, 75 } 
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 68 }, { 3, 39 }, { 1, 47 }, { 4, 41 },  
    { 2, 72 }, { -2, 63 }, { -2, 66 }, { 2, 59 },
    { 2, 43 }, { -6, 24 }, { -4, 25 }, { -4, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { -3, 70 }, { 3, 52 }, { 4, 48 }, { 5, 47 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 68 }, { 3, 39 }, { 1, 47 }, { 4, 41 },
    { 2, 72 }, { -2, 63 }, { -2, 66 }, { 2, 59 }, 
    { 2, 43 }, { -6, 24 }, { -4, 25 }, { -4, 27 },
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { -3, 70 }, { 3, 52 }, { 4, 48 }, { 5, 47 }
  }
};
static const Short
INIT_ONE_FLAG_CHROMA[3][NUM_ONE_FLAG_CTX_CHROMA][2] =
{
  {
    { 3, 62 }, { 20, -11 }, { 16, 9 }, { 16, 19 }, 
    { 4, 64 }, { 2, 44 }, { 1, 53 }, { 3, 51 }
  },
  {
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { -28, 114 }, { -35, 97 }, { 13, 19 }, { 9, 24 }  
  },
  {
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { -28, 114 }, { -35, 97 }, { 13, 19 }, { 9, 24 } 
  }
};

static const Short
INIT_ABS_FLAG_LUMA[3][NUM_ABS_FLAG_CTX_LUMA][2] =
{
  {
    { -11, 68 }, { -4, 67 }, { -4, 73 }, 
    { -6, 66 }, { -4, 68 }, { -5, 74 }, 
    { -7, 79 }, { -6, 81 }, { -7, 84 }, 
    { -9, 50 }, { -5, 56 }, { -4, 60 }, 
    { -2, 52 }, { -1, 54 }, { -1, 58 },
    { -6, 73 }, { -2, 66 }, { 0, 65 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 },
    { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -2, 63 }, { -3, 68 }, { 0, 64 }, 
    { -7, 43 }, { -1, 46 }, { -1, 51 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, 
    { -2, 58 }, { 4, 52 }, { 2, 57 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 },
    { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -2, 63 }, { -3, 68 }, { 0, 64 }, 
    { -7, 43 }, { -1, 46 }, { -1, 51 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, 
    { -2, 58 }, { 4, 52 }, { 2, 57 }
  }
};
static const Short
INIT_ABS_FLAG_CHROMA[3][NUM_ABS_FLAG_CTX_CHROMA][2] =
{
  {
    { -5, 53 }, { 1, 58 }, { 4, 61 },
    { 0, 55 }, { 2, 58 }, { 1, 63 }
  },
  {
    { 6, 31 }, { 2, 56 }, { 4, 63 }, 
    { -52, 136 }, { 3, 53 }, { -2, 64 }
  },
  {
    { 6, 31 }, { 2, 56 }, { 4, 63 },
    { -52, 136 }, { 3, 53 }, { -2, 64 }
  }
};
#else
static const Short
INIT_ONE_FLAG_LUMA[3][NUM_ONE_FLAG_CTX_LUMA][2] =
{
  {
    { -9, 84 }, { -16, 60 }, { -11, 59 }, { -9, 61 }, 
    { -7, 65 }, { -5, 79 }, { 0, 47 }, { -5, 66 }, 
    { -7, 70 }, { -5, 68 }, { -7, 92 }, { -8, 79 }, 
    { -4, 74 }, { -4, 75 }, { -3, 70 }, { -6, 62 }, 
    { -12, 39 }, { -12, 48 }, { -13, 53 }, { -11, 55 }, 
    { -3, 63 }, { -2, 47 }, { -6, 59 }, { -8, 62 }, 
    { -5, 58 }, { -5, 78 }, { -12, 85 }, { -11, 79 }, 
    { -9, 75 }, { -3, 64 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 47 }, { -1, 68 }, { 3, 39 }, { 1, 47 }, 
    { 4, 41 }, { 4, 42 }, { 2, 72 }, { -2, 63 }, 
    { -2, 66 }, { 2, 59 }, { 7, 48 }, { 2, 43 }, 
    { -6, 24 }, { -4, 25 }, { -4, 27 }, { -1, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { 5, 32 }, { -3, 70 }, { 3, 52 }, { 4, 48 }, 
    { 5, 47 }, { 3, 50 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 47 }, { -1, 68 }, { 3, 39 }, { 1, 47 }, 
    { 4, 41 }, { 4, 42 }, { 2, 72 }, { -2, 63 }, 
    { -2, 66 }, { 2, 59 }, { 7, 48 }, { 2, 43 }, 
    { -6, 24 }, { -4, 25 }, { -4, 27 }, { -1, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { 5, 32 }, { -3, 70 }, { 3, 52 }, { 4, 48 }, 
    { 5, 47 }, { 3, 50 }
  }
};
INIT_ONE_FLAG_CHROMA[3][NUM_ONE_FLAG_CTX_CHROMA][2] =
{
  {
    { 3, 62 }, { 20, -11 }, { 16, 9 }, { 16, 19 }, 
    { 9, 38 }, { 4, 64 }, { 2, 44 }, { 1, 53 }, 
    { 3, 51 }, { 2, 54 }
  },
  {
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { 13, 23 }, { -28, 114 }, { -35, 97 }, { 13, 19 }, 
    { 9, 24 }, { -3, 55 }
  },
  {
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { 13, 23 }, { -28, 114 }, { -35, 97 }, { 13, 19 }, 
    { 9, 24 }, { -3, 55 }
  }
};
static const Short
INIT_ABS_FLAG_LUMA[3][NUM_ABS_FLAG_CTX_LUMA][2] =
{
  {
    { -11, 68 }, { -4, 67 }, { -4, 73 }, { -3, 75 }, 
    { -5, 87 }, { -6, 66 }, { -4, 68 }, { -5, 74 }, 
    { -3, 73 }, { -3, 83 }, { -7, 79 }, { -6, 81 }, 
    { -7, 84 }, { -7, 85 }, { -8, 99 }, { -9, 50 }, 
    { -5, 56 }, { -4, 60 }, { -3, 62 }, { -6, 73 }, 
    { -2, 52 }, { -1, 54 }, { -1, 58 }, { 1, 55 }, 
    { -2, 65 }, { -6, 73 }, { -2, 66 }, { 0, 65 }, 
    { 0, 65 }, { -2, 75 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 }, { 1, 66 }, 
    { 2, 72 }, { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -1, 66 }, { -1, 78 }, { -2, 63 }, { -3, 68 }, 
    { 0, 64 }, { 5, 58 }, { 4, 72 }, { -7, 43 }, 
    { -1, 46 }, { -1, 51 }, { 0, 55 }, { 1, 56 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, { -10, 74 }, 
    { 2, 56 }, { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 2, 57 }, { -1, 67 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 }, { 1, 66 }, 
    { 2, 72 }, { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -1, 66 }, { -1, 78 }, { -2, 63 }, { -3, 68 }, 
    { 0, 64 }, { 5, 58 }, { 4, 72 }, { -7, 43 }, 
    { -1, 46 }, { -1, 51 }, { 0, 55 }, { 1, 56 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, { -10, 74 }, 
    { 2, 56 }, { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 2, 57 }, { -1, 67 }
  }
};
static const Short
INIT_ABS_FLAG_CHROMA[3][NUM_ABS_FLAG_CTX_CHROMA][2] =
{
  {
    { -5, 53 }, { 1, 58 }, { 4, 61 }, { 5, 64 }, 
    { 3, 78 }, { 0, 55 }, { 2, 58 }, { 1, 63 }, 
    { 4, 62 }, { 2, 78 }
  },
  {
    { 6, 31 }, { 2, 56 }, { 4, 63 }, { -1, 76 }, 
    { -4, 91 }, { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { -6, 80 }, { -26, 122 }
  },
  {
    { 6, 31 }, { 2, 56 }, { 4, 63 }, { -1, 76 }, 
    { -4, 91 }, { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { -6, 80 }, { -26, 122 }
  }
};
#endif
#else
#if COEFF_CTX_RED
static const Short
INIT_ONE_FLAG[3][2*NUM_ONE_FLAG_CTX][2] =
{
  {
    { -9, 84 }, { -16, 60 }, { -11, 59 }, { -9, 61 }, 
    { -5, 79 }, { 0, 47 }, { -5, 66 }, { -7, 70 }, 
    { -7, 92 }, { -8, 79 }, { -4, 74 }, { -4, 75 }, 
    { -6, 62 }, { -12, 39 }, { -12, 48 }, { -13, 53 },  
    { -3, 63 }, { -2, 47 }, { -6, 59 }, { -8, 62 }, 
    { -5, 78 }, { -12, 85 }, { -11, 79 }, { -9, 75 }, 
    { 3, 62 }, { 20, -11 }, { 16, 9 }, { 16, 19 }, 
    { 4, 64 }, { 2, 44 }, { 1, 53 }, { 3, 51 }, 
    { 10, 62 }, { -2, 60 }, { 1, 62 }, { 8, 53 },  
    { 0, 47 }, { 17, -20 }, { 14, -7 }, { 8, 8 }, 
    { 5, 46 }, { 11, 21 }, { 7, 32 }, { 1, 42 }, 
    { -3, 66 }, { -2, 55 }, { 11, 31 }, { -50, 138 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 68 }, { 3, 39 }, { 1, 47 }, { 4, 41 },  
    { 2, 72 }, { -2, 63 }, { -2, 66 }, { 2, 59 },
    { 2, 43 }, { -6, 24 }, { -4, 25 }, { -4, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { -3, 70 }, { 3, 52 }, { 4, 48 }, { 5, 47 }, 
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { -28, 114 }, { -35, 97 }, { 13, 19 }, { 9, 24 },  
    { -18, 107 }, { 9, 45 }, { 8, 36 }, { 17, 16 },  
    { -7, 62 }, { 11, 8 }, { -16, 54 }, { 23, -25 }, 
    { 7, 42 }, { -38, 104 }, { 15, 24 }, { 12, 27 }, 
    { 2, 60 }, { 27, 4 }, { 61, -73 }, { 61, -73 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 68 }, { 3, 39 }, { 1, 47 }, { 4, 41 },
    { 2, 72 }, { -2, 63 }, { -2, 66 }, { 2, 59 }, 
    { 2, 43 }, { -6, 24 }, { -4, 25 }, { -4, 27 },
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { -3, 70 }, { 3, 52 }, { 4, 48 }, { 5, 47 }, 
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { -28, 114 }, { -35, 97 }, { 13, 19 }, { 9, 24 },  
    { -18, 107 }, { 9, 45 }, { 8, 36 }, { 17, 16 },  
    { -7, 62 }, { 11, 8 }, { -16, 54 }, { 23, -25 },
    { 7, 42 }, { -38, 104 }, { 15, 24 }, { 12, 27 }, 
    { 2, 60 }, { 27, 4 }, { 61, -73 }, { 61, -73 }
  }
};
static const Short
INIT_ABS_FLAG[3][2*NUM_ABS_FLAG_CTX][2] =
{
  {
    { -11, 68 }, { -4, 67 }, { -4, 73 }, 
    { -6, 66 }, { -4, 68 }, { -5, 74 }, 
    { -7, 79 }, { -6, 81 }, { -7, 84 }, 
    { -9, 50 }, { -5, 56 }, { -4, 60 }, 
    { -2, 52 }, { -1, 54 }, { -1, 58 },
    { -6, 73 }, { -2, 66 }, { 0, 65 },
    { -5, 53 }, { 1, 58 }, { 4, 61 },
    { 0, 55 }, { 2, 58 }, { 1, 63 },
    { 5, 53 }, { 4, 62 }, { 2, 65 }, 
    { -11, 47 }, { -1, 43 }, { 2, 46 }, 
    { 5, 35 }, { 4, 46 }, { 6, 43 }, 
    { 5, 45 }, { -54, 149 }, { 1, 58 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 },
    { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -2, 63 }, { -3, 68 }, { 0, 64 }, 
    { -7, 43 }, { -1, 46 }, { -1, 51 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, 
    { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 6, 31 }, { 2, 56 }, { 4, 63 }, 
    { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { 3, 58 }, { -1, 66 }, { 4, 54 }, 
    { -51, 122 }, { -46, 123 }, { 13, 6 }, 
    { 18, 17 }, { 6, 45 }, { 33, 12 }, 
    { 61, -73 }, { 0, 64 }, { 0, 64 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 },
    { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -2, 63 }, { -3, 68 }, { 0, 64 }, 
    { -7, 43 }, { -1, 46 }, { -1, 51 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, 
    { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 6, 31 }, { 2, 56 }, { 4, 63 },
    { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { 3, 58 }, { -1, 66 }, { 4, 54 }, 
    { -51, 122 }, { -46, 123 }, { 13, 6 }, 
    { 18, 17 }, { 6, 45 }, { 33, 12 }, 
    { 61, -73 }, { 0, 64 }, { 0, 64 }
  }
};
#else
static const Short
INIT_ONE_FLAG[3][2*NUM_ONE_FLAG_CTX][2] =
{
  {
    { -9, 84 }, { -16, 60 }, { -11, 59 }, { -9, 61 }, 
    { -7, 65 }, { -5, 79 }, { 0, 47 }, { -5, 66 }, 
    { -7, 70 }, { -5, 68 }, { -7, 92 }, { -8, 79 }, 
    { -4, 74 }, { -4, 75 }, { -3, 70 }, { -6, 62 }, 
    { -12, 39 }, { -12, 48 }, { -13, 53 }, { -11, 55 }, 
    { -3, 63 }, { -2, 47 }, { -6, 59 }, { -8, 62 }, 
    { -5, 58 }, { -5, 78 }, { -12, 85 }, { -11, 79 }, 
    { -9, 75 }, { -3, 64 },
    { 3, 62 }, { 20, -11 }, { 16, 9 }, { 16, 19 }, 
    { 9, 38 }, { 4, 64 }, { 2, 44 }, { 1, 53 }, 
    { 3, 51 }, { 2, 54 }, { 10, 62 }, { -2, 60 }, 
    { 1, 62 }, { 8, 53 }, { 17, 36 }, { 0, 47 }, 
    { 17, -20 }, { 14, -7 }, { 8, 8 }, { 6, 19 }, 
    { 5, 46 }, { 11, 21 }, { 7, 32 }, { 1, 42 }, 
    { 9, 31 }, { -3, 66 }, { -2, 55 }, { 11, 31 }, 
    { -50, 138 }, { -8, 69 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 47 }, { -1, 68 }, { 3, 39 }, { 1, 47 }, 
    { 4, 41 }, { 4, 42 }, { 2, 72 }, { -2, 63 }, 
    { -2, 66 }, { 2, 59 }, { 7, 48 }, { 2, 43 }, 
    { -6, 24 }, { -4, 25 }, { -4, 27 }, { -1, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { 5, 32 }, { -3, 70 }, { 3, 52 }, { 4, 48 }, 
    { 5, 47 }, { 3, 50 },
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { 13, 23 }, { -28, 114 }, { -35, 97 }, { 13, 19 }, 
    { 9, 24 }, { -3, 55 }, { -18, 107 }, { 9, 45 }, 
    { 8, 36 }, { 17, 16 }, { -12, 100 }, { -7, 62 }, 
    { 11, 8 }, { -16, 54 }, { 23, -25 }, { 28, -27 }, 
    { 7, 42 }, { -38, 104 }, { 15, 24 }, { 12, 27 }, 
    { 11, 14 }, { 2, 60 }, { 27, 4 }, { 61, -73 }, 
    { 61, -73 }, { 24, 10 }
  },
  {
    { 0, 62 }, { 8, 21 }, { -2, 40 }, { -3, 43 }, 
    { -1, 47 }, { -1, 68 }, { 3, 39 }, { 1, 47 }, 
    { 4, 41 }, { 4, 42 }, { 2, 72 }, { -2, 63 }, 
    { -2, 66 }, { 2, 59 }, { 7, 48 }, { 2, 43 }, 
    { -6, 24 }, { -4, 25 }, { -4, 27 }, { -1, 27 }, 
    { 1, 54 }, { 7, 27 }, { 7, 31 }, { 9, 27 }, 
    { 5, 32 }, { -3, 70 }, { 3, 52 }, { 4, 48 }, 
    { 5, 47 }, { 3, 50 },
    { 7, 56 }, { 24, -14 }, { 17, 4 }, { 18, 9 }, 
    { 13, 23 }, { -28, 114 }, { -35, 97 }, { 13, 19 }, 
    { 9, 24 }, { -3, 55 }, { -18, 107 }, { 9, 45 }, 
    { 8, 36 }, { 17, 16 }, { -12, 100 }, { -7, 62 }, 
    { 11, 8 }, { -16, 54 }, { 23, -25 }, { 28, -27 }, 
    { 7, 42 }, { -38, 104 }, { 15, 24 }, { 12, 27 }, 
    { 11, 14 }, { 2, 60 }, { 27, 4 }, { 61, -73 }, 
    { 61, -73 }, { 24, 10 }
  }
};
static const Short
INIT_ABS_FLAG[3][2*NUM_ABS_FLAG_CTX][2] =
{
  {
    { -11, 68 }, { -4, 67 }, { -4, 73 }, { -3, 75 }, 
    { -5, 87 }, { -6, 66 }, { -4, 68 }, { -5, 74 }, 
    { -3, 73 }, { -3, 83 }, { -7, 79 }, { -6, 81 }, 
    { -7, 84 }, { -7, 85 }, { -8, 99 }, { -9, 50 }, 
    { -5, 56 }, { -4, 60 }, { -3, 62 }, { -6, 73 }, 
    { -2, 52 }, { -1, 54 }, { -1, 58 }, { 1, 55 }, 
    { -2, 65 }, { -6, 73 }, { -2, 66 }, { 0, 65 }, 
    { 0, 65 }, { -2, 75 }, 
    { -5, 53 }, { 1, 58 }, { 4, 61 }, { 5, 64 }, 
    { 3, 78 }, { 0, 55 }, { 2, 58 }, { 1, 63 }, 
    { 4, 62 }, { 2, 78 }, { 5, 53 }, { 4, 62 }, 
    { 2, 65 }, { 1, 68 }, { 4, 78 }, { -11, 47 }, 
    { -1, 43 }, { 2, 46 }, { 1, 51 }, { -2, 60 }, 
    { 5, 35 }, { 4, 46 }, { 6, 43 }, { 12, 36 }, 
    { 9, 44 }, { 5, 45 }, { -54, 149 }, { 1, 58 }, 
    { -4, 70 }, { -60, 169 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 }, { 1, 66 }, 
    { 2, 72 }, { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -1, 66 }, { -1, 78 }, { -2, 63 }, { -3, 68 }, 
    { 0, 64 }, { 5, 58 }, { 4, 72 }, { -7, 43 }, 
    { -1, 46 }, { -1, 51 }, { 0, 55 }, { 1, 56 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, { -10, 74 }, 
    { 2, 56 }, { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 2, 57 }, { -1, 67 },
    { 6, 31 }, { 2, 56 }, { 4, 63 }, { -1, 76 }, 
    { -4, 91 }, { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { -6, 80 }, { -26, 122 }, { 3, 58 }, { -1, 66 }, 
    { 4, 54 }, { -7, 79 }, { -26, 124 }, { -51, 122 }, 
    { -46, 123 }, { 13, 6 }, { 34, -22 }, { -28, 137 }, 
    { 18, 17 }, { 6, 45 }, { 33, 12 }, { -12, 102 }, 
    { 8, 47 }, { 61, -73 }, { 0, 64 }, { 0, 64 }, 
    { 61, -73 }, { 0, 64 }
  },
  {
    { -18, 71 }, { -7, 66 }, { -3, 68 }, { 1, 66 }, 
    { 2, 72 }, { -6, 59 }, { -4, 62 }, { -2, 64 }, 
    { -1, 66 }, { -1, 78 }, { -2, 63 }, { -3, 68 }, 
    { 0, 64 }, { 5, 58 }, { 4, 72 }, { -7, 43 }, 
    { -1, 46 }, { -1, 51 }, { 0, 55 }, { 1, 56 }, 
    { -4, 51 }, { -1, 54 }, { -5, 63 }, { -10, 74 }, 
    { 2, 56 }, { -2, 58 }, { 4, 52 }, { 2, 57 }, 
    { 2, 57 }, { -1, 67 },
    { 6, 31 }, { 2, 56 }, { 4, 63 }, { -1, 76 }, 
    { -4, 91 }, { -52, 136 }, { 3, 53 }, { -2, 64 }, 
    { -6, 80 }, { -26, 122 }, { 3, 58 }, { -1, 66 }, 
    { 4, 54 }, { -7, 79 }, { -26, 124 }, { -51, 122 }, 
    { -46, 123 }, { 13, 6 }, { 34, -22 }, { -28, 137 }, 
    { 18, 17 }, { 6, 45 }, { 33, 12 }, { -12, 102 }, 
    { 8, 47 }, { 61, -73 }, { 0, 64 }, { 0, 64 }, 
    { 61, -73 }, { 0, 64 }
  }
};
#endif
#endif

// initial probability for motion vector predictor index
static const Short
INIT_MVP_IDX[3][NUM_MVP_IDX_CTX][2] =
{
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }
  }
};

// initial probability for ALF flag
static const Short
INIT_ALF_FLAG[3][NUM_ALF_FLAG_CTX][2] =
{
  {
    {   50,  -48 }
  },
  {
    {   27,  -20 }
  },
  {
    {  -12,   68 }
  }
};

// initial probability for ALF side information (unsigned)
static const Short
INIT_ALF_UVLC[3][NUM_ALF_UVLC_CTX][2] =
{
  {
    {    1,   66 }, {   -3,   77 }
  },
  {
    {   -5,   75 }, {  -14,   94 }
  },
  {
    {   -5,   72 }, {  -30,  122 }
  }
};

// initial probability for ALF side information (signed)
static const Short
INIT_ALF_SVLC[3][NUM_ALF_SVLC_CTX][2] =
{
  {
    {   11,   57 }, {   -1,   62 }, {    0,   64 }
  },
  {
    {    6,   66 }, {   -1,   64 }, {    0,   64 }
  },
  {
    {    1,   73 }, {    2,   61 }, {    0,   64 }
  }
};
#if SAO
// initial probability for SAO flag
static const Short
INIT_SAO_FLAG[3][NUM_SAO_FLAG_CTX][2] =
{
  {
    {   50,  -48 }
  },
  {
    {   27,  -20 }
  },
  {
    {  -12,   68 }
  }
};

// initial probability for SAO side information (unsigned)
static const Short
INIT_SAO_UVLC[3][NUM_SAO_UVLC_CTX][2] =
{
  {
    {    1,   66 }, {   -3,   77 }
  },
  {
    {   -5,   75 }, {  -14,   94 }
  },
  {
    {   -5,   72 }, {  -30,  122 }
  }
};

// initial probability for SAO side information (signed)
static const Short
INIT_SAO_SVLC[3][NUM_SAO_SVLC_CTX][2] =
{
  {
    {   11,   57 }, {   -1,   62 }, {    0,   64 }
  },
  {
    {    6,   66 }, {   -1,   64 }, {    0,   64 }
  },
  {
    {    1,   73 }, {    2,   61 }, {    0,   64 }
  }
};
#endif

static const Short
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX][2] =
{
  {
    {    0,    0 }, {   12,   12 }, {   22,    4 }, {   -2,   49 }, 
    {    4,   46 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, 
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   13 }, {  -28,   89 }, {  -30,   99 }, {  -34,  106 }, 
    {  -19,   76 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, 
    {    0,   64 }, {    0,   64 }
  },
  {
    {  -11,   38 }, {  -31,   88 }, {  -42,  118 }, {  -47,  130 }, 
    {  -21,   73 }, {    0,   64 }, {    0,   64 }, {    0,   64 }, 
    {    0,   64 }, {    0,   64 }
  }
};

//! \}
#endif

#endif

