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
#define NUM_PART_SIZE_CTX             5       ///< number of context models for partition size
#if AMP
#define NUM_CU_X_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_CU_Y_POS_CTX              2       ///< number of context models for partition size (AMP)
#endif
#define NUM_PRED_MODE_CTX             2       ///< number of context models for prediction mode
#define NUM_ADI_CTX                   3       ///< number of context models for intra prediction

#define NUM_CHROMA_PRED_CTX           2       ///< number of context models for intra prediction (chroma)
#define NUM_INTER_DIR_CTX             4       ///< number of context models for inter prediction direction
#define NUM_MV_RES_CTX                2       ///< number of context models for motion vector difference

#define NUM_REF_NO_CTX                4       ///< number of context models for reference index
#define NUM_TRANS_SUBDIV_FLAG_CTX     10      ///< number of context models for transform subdivision flags
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF
#define NUM_DELTA_QP_CTX              4       ///< number of context models for dQP

#define NUM_SIG_FLAG_CTX              44      ///< number of context models for sig flag
#define NUM_CTX_LAST_FLAG_XY          19      ///< number of context models for PCP last flag
#define NUM_ONE_FLAG_CTX              30      ///< number of context models for greater than one
#define NUM_ABS_FLAG_CTX              30      ///< number of context models for magnitude

#define NUM_MVP_IDX_CTX               2       ///< number of context models for MVP index

#define NUM_ALF_FLAG_CTX              1       ///< number of context models for ALF flag
#define NUM_ALF_UVLC_CTX              2       ///< number of context models for ALF UVLC (filter length)
#define NUM_ALF_SVLC_CTX              3       ///< number of context models for ALF SVLC (filter coeff.)

#if SAO
#define NUM_SAO_FLAG_CTX              1       ///< number of context models for SAO flag
#define NUM_SAO_UVLC_CTX              2       ///< number of context models for SAO UVLC
#define NUM_SAO_SVLC_CTX              3       ///< number of context models for SAO SVLC
#endif

// ====================================================================================================================
// Tables
// ====================================================================================================================

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
  {
    {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {  -25,   89 }
  },
  {
    {    0,   64 }, {    0,   64 }
  }
};

// initial probability for intra direction of luma
static const Short
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX][2] =
{
  {
    {    2,   54 }, {  -3,   65  }, {   -3,   65 }
  },
  {
    {    0,   50 }, {  -2,   61  }, {   -2,   61 }
  },
  {
    {    0,   51 }, {  1,   55   }, {    1,   55 }
  }
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
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  },
  {
    {    0,   64 }, {    0,   64 }, {    0,   64 }, {    0,   64 }
  }
};

static const Short
INIT_QT_CBF[3][3*NUM_QT_CBF_CTX][2] =
{
  {
    {  -22,  116 }, {   -5,   75 }, {  -16,  112 }, {  -16,  111 }, {  -32,  165 },
    {  -35,  116 }, {  -12,   61 }, {   -9,   73 }, {  -10,   75 }, {  -14,   96 }, 
    {  -29,  104 }, {  -12,   59 }, {   -5,   65 }, {   -6,   67 }, {  -11,   90 }
  },
  {
    {  -18,   98 }, {  -41,  120 }, {  -29,  117 }, {  -23,  108 }, {  -35,  143 },
    {  -46,  114 }, {  -42,  119 }, {  -11,   74 }, {  -19,   90 }, {  -42,  139 }, 
    {  -43,  107 }, {  -41,  118 }, {  -17,   86 }, {  -25,  101 }, {  -14,   91 }
  },
  {
    {  -11,   80 }, {  -32,   83 }, {  -19,   89 }, {  -16,   85 }, {  -19,  102 },
    {  -22,   52 }, {  -48,  123 }, {   -7,   68 }, {  -37,  121 }, {  -58,  164 }, 
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

