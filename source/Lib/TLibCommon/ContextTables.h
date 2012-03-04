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
#if MRG_IDX_CTX_RED
#define NUM_MERGE_IDX_EXT_CTX         1       ///< number of context models for merge index of merge extended
#else
#define NUM_MERGE_IDX_EXT_CTX         4       ///< number of context models for merge index of merge extended
#endif

#define NUM_ALF_CTRL_FLAG_CTX         1       ///< number of context models for ALF control flag
#define NUM_PART_SIZE_CTX             4       ///< number of context models for partition size
#if AMP_CTX
#define NUM_CU_AMP_CTX                1       ///< number of context models for partition size (AMP)
#else
#define NUM_CU_X_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_CU_Y_POS_CTX              2       ///< number of context models for partition size (AMP)
#endif
#define NUM_PRED_MODE_CTX             1       ///< number of context models for prediction mode

#define NUM_ADI_CTX                   1       ///< number of context models for intra prediction

#define NUM_CHROMA_PRED_CTX           2       ///< number of context models for intra prediction (chroma)
#define NUM_INTER_DIR_CTX             4       ///< number of context models for inter prediction direction
#define NUM_MV_RES_CTX                2       ///< number of context models for motion vector difference

#define NUM_REF_NO_CTX                4       ///< number of context models for reference index
#define NUM_TRANS_SUBDIV_FLAG_CTX     10      ///< number of context models for transform subdivision flags
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF
#define NUM_DELTA_QP_CTX              3       ///< number of context models for dQP

#define NUM_SIG_CG_FLAG_CTX           2       ///< number of context models for MULTI_LEVEL_SIGNIFICANCE

#define NUM_SIG_FLAG_CTX              48      ///< number of context models for sig flag

#define NUM_SIG_FLAG_CTX_LUMA         27      ///< number of context models for luma sig flag
#define NUM_SIG_FLAG_CTX_CHROMA       21      ///< number of context models for chroma sig flag
#if LAST_CTX_REDUCTION
#define NUM_CTX_LAST_FLAG_XY          15      ///< number of context models for last coefficient position
#else
#define NUM_CTX_LAST_FLAG_XY          18      ///< number of context models for last coefficient position
#endif

#if LEVEL_CTX_LUMA_RED
#define NUM_ONE_FLAG_CTX              24      ///< number of context models for greater than 1 flag
#define NUM_ONE_FLAG_CTX_LUMA         16      ///< number of context models for greater than 1 flag of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than 1 flag of chroma
#if RESTRICT_GR1GR2FLAG_NUMBER
#define NUM_ABS_FLAG_CTX               6      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA          4      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        2      ///< number of context models for greater than 2 flag of chroma
#else
#define NUM_ABS_FLAG_CTX              18      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA         12      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        6      ///< number of context models for greater than 2 flag of chroma
#endif
#else
#define NUM_ONE_FLAG_CTX              32      ///< number of context models for greater than 1 flag
#define NUM_ONE_FLAG_CTX_LUMA         24      ///< number of context models for greater than 1 flag of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than 1 flag of chroma
#if RESTRICT_GR1GR2FLAG_NUMBER
#define NUM_ABS_FLAG_CTX               8      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA          6      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        2      ///< number of context models for greater than 2 flag of chroma
#else
#define NUM_ABS_FLAG_CTX              24      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA         18      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        6      ///< number of context models for greater than 2 flag of chroma
#endif
#endif

#define NUM_MVP_IDX_CTX               2       ///< number of context models for MVP index

#define NUM_ALF_FLAG_CTX              1       ///< number of context models for ALF flag
#define NUM_ALF_UVLC_CTX              2       ///< number of context models for ALF UVLC (filter length)
#define NUM_ALF_SVLC_CTX              3       ///< number of context models for ALF SVLC (filter coeff.)

#define NUM_SAO_FLAG_CTX              1       ///< number of context models for SAO flag
#define NUM_SAO_UVLC_CTX              2       ///< number of context models for SAO UVLC
#define NUM_SAO_SVLC_CTX              3       ///< number of context models for SAO SVLC
#if SAO_UNIT_INTERLEAVING
#define NUM_SAO_RUN_CTX               3       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_MERGE_LEFT_FLAG_CTX   3       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_MERGE_UP_FLAG_CTX     1       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_TYPE_IDX_CTX          2       ///< number of context models for AO SVLC (filter coeff.)
#endif
#if CABAC_LINEAR_INIT
#define CNU                          154      ///< dummy initialization value for unused context models 'Context model Not Used'
#else
#define CNU                          119      ///< dummy initialization value for unused context models 'Context model Not Used'
#endif

// ====================================================================================================================
// Tables
// ====================================================================================================================

// initial probability for split flag
#if CABAC_LINEAR_INIT
static const UChar 
INIT_SPLIT_FLAG[3][NUM_SPLIT_FLAG_CTX] =  
{
  { 139,  141,  157, }, 
  { 107,  139,  126, }, 
  { 107,  139,  126, }, 
};

static const UChar 
INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX] =  
{
  { CNU,  CNU,  CNU, }, 
  { 197,  185,  201, }, 
  { 197,  185,  201, }, 
};

static const UChar 
INIT_ALF_CTRL_FLAG[3][NUM_ALF_CTRL_FLAG_CTX] = 
{
  { 200, }, 
  { 139, }, 
  { 169, }, 
};

static const UChar 
INIT_MERGE_FLAG_EXT[3][NUM_MERGE_FLAG_EXT_CTX] = 
{
  { CNU, }, 
  { 110, }, 
  { 154, }, 
};

static const UChar 
INIT_MERGE_IDX_EXT[3][NUM_MERGE_IDX_EXT_CTX] =  
{
#if MRG_IDX_CTX_RED
  { CNU, }, 
  { 122, }, 
  { 137, }, 
#else
  { CNU,  CNU,  CNU,  CNU, }, 
  { 122,  138,  153,  182, }, 
  { 137,  139,  154,  139, }, 
#endif
};

static const UChar 
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] =  
{
  { 184,  CNU,  CNU,  CNU, }, 
  { 154,  139,  CNU,  CNU, }, 
  { 154,  139,  CNU,  CNU, }, 
};

#if AMP_CTX
static const UChar 
INIT_CU_AMP_POS[3][NUM_CU_AMP_CTX] =  
{
  { CNU, }, 
  { 154, }, 
  { 154, }, 
};
#else
static const UChar 
INIT_CU_X_POS[3][NUM_CU_X_POS_CTX] =  
{
  { CNU,  CNU, }, 
  { 154,  139, }, 
  { 154,  139, }, 
};

static const UChar 
INIT_CU_Y_POS[3][NUM_CU_Y_POS_CTX] =  
{
  { CNU,  CNU, }, 
  { 154,  154, }, 
  { 154,  139, }, 
};
#endif

static const UChar 
INIT_PRED_MODE[3][NUM_PRED_MODE_CTX] = 
{
  { CNU, }, 
  { 149, }, 
  { 134, }, 
};

static const UChar 
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX] = 
{
  { 184, }, 
  { 154, }, 
  { 183, }, 
};

static const UChar 
INIT_CHROMA_PRED_MODE[3][NUM_CHROMA_PRED_CTX] = 
{
  {  63,  139, }, 
  { 152,  139, }, 
  { 152,  139, }, 
};

static const UChar 
INIT_INTER_DIR[3][NUM_INTER_DIR_CTX] = 
{
  { CNU,  CNU,  CNU,  CNU, }, 
  { CNU,  CNU,  CNU,  CNU, }, 
  {  95,   79,   63,   31, }, 
};

static const UChar 
INIT_MVD[3][NUM_MV_RES_CTX] =  
{
  { CNU,  CNU, }, 
  { 140,  198, }, 
  { 169,  198, }, 
};

static const UChar 
INIT_REF_PIC[3][NUM_REF_NO_CTX] =  
{
  { CNU,  CNU,  CNU,  CNU, }, 
  { 153,  153,  139,  CNU, }, 
  { 153,  153,  168,  CNU, }, 
};

static const UChar 
INIT_DQP[3][NUM_DELTA_QP_CTX] = 
{
  { 154,  154,  154, }, 
  { 154,  154,  154, }, 
  { 154,  154,  154, }, 
};

static const UChar 
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX] =  
{
  { 111,  141,  CNU,  CNU,  CNU,   94,  138,  182,  CNU,  CNU, }, 
  { 153,  111,  CNU,  CNU,  CNU,  149,  107,  167,  CNU,  CNU, }, 
  { 153,  111,  CNU,  CNU,  CNU,  149,   92,  167,  CNU,  CNU, }, 
};

static const UChar 
INIT_QT_ROOT_CBF[3][NUM_QT_ROOT_CBF_CTX] = 
{
  { CNU, }, 
  {  79, }, 
  {  79, }, 
};

#if LAST_CTX_REDUCTION
static const UChar 
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =  
{
  { 110,  110,  124,  110,  140,  111,  125,  111,  127,  111,  111,  156,  127,  127,  111, 
    108,  123,   63,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, 
  }, 
  { 125,  110,   94,  110,  125,  110,  125,  111,  111,  110,  139,  111,  111,  111,  125,  
    108,  123,  108,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,
  }, 
  { 125,  110,  124,  110,  125,  110,  125,  111,  111,  110,  139,  111,  111,  111,  125, 
    108,  123,   93,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, 
  }, 
};
#else
static const UChar 
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =  
{
  { 110,  110,  124,  110,  140,  111,  124,  125,  111,  127,  111,  138,  111,  156,  127,  127,  111,   94,
    108,  123,   63,   63,  139,  124,   93,  108,  125,  111,  110,   63,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, 
  }, 
  { 125,  110,  124,  110,  125,  110,  153,  125,  111,  111,  110,  153,  139,  111,  111,  111,  125,  139, 
    108,  123,  108,  152,  124,   94,  123,  137,  139,  110,  110,  154,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, 
  }, 
  { 125,  110,  124,  110,  125,  110,  153,  125,  111,  111,  110,  153,  139,  111,  111,  111,  125,  139,  
    108,  123,   93,  152,  124,   94,  123,  152,  139,  110,  110,  154,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, 
  }, 
};
#endif

static const UChar 
INIT_SIG_CG_FLAG[3][2 * NUM_SIG_CG_FLAG_CTX] =  
{
  {  91,  171,  
    134,  141, 
  }, 
  { 121,  140, 
    61,  154, 
  }, 
  { 121,  140,  
    61,  154, 
  }, 
};

static const UChar 
INIT_SIG_FLAG[3][NUM_SIG_FLAG_CTX] = 
{
  { 141,  111,  125,  110,  110,   94,  124,  108,  124,  125,  139,  124,   63,  139,  168,  138,  107,  123,   92,  111,  141,  107,  125,  141,  179,  153,  125,  140,  139,  182,  123,   47,  153,  182,  137,  149,  192,  152,  224,  136,   31,  136,   74,  140,  141,  136,  139,  111, }, 
  { 170,  154,  139,  153,  139,  123,  123,   63,  153,  168,  153,  152,   92,  152,  152,  137,  122,   92,   61,  155,  185,  166,  183,  140,  136,  153,  154,  155,  153,  123,   63,   61,  167,  153,  167,  136,  149,  107,  136,  121,  122,   91,  149,  170,  185,  151,  183,  140, }, 
  { 170,  154,  139,  153,  139,  123,  123,   63,  124,  139,  153,  152,   92,  152,  152,  137,  137,   92,   61,  170,  185,  166,  183,  140,  136,  153,  154,  155,  153,  138,  107,   61,  167,  153,  167,  136,  121,  122,  136,  121,  122,   91,  149,  170,  170,  151,  183,  140, }, 
};

#if LEVEL_CTX_LUMA_RED
static const UChar 
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] = 
{
  { 140,   92,  137,  138,  140,  152,  138,  139,  153,   74,  149,   92,  139,  107,  122,  152,  140,  179,  166,  182,  140,  227,  122,  197, }, 
  { 154,  196,  196,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  137,  169,  194,  166,  167,  154,  167,  137,  182, }, 
  { 154,  196,  167,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  122,  169,  208,  166,  167,  154,  152,  167,  182, }, 
};

#if RESTRICT_GR1GR2FLAG_NUMBER
static const UChar 
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =  
{
  { 138,  153,  136,  167,  152,  152, }, 
  { 107,  167,   91,  122,  107,  167, }, 
  { 107,  167,   91,  107,  107,  167, }, 
};
#else
static const UChar 
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =  
{
  { 138,  139,  111,  153,  139,  111,  136,  167,  139,  167,  153,  139,  152,  139,  140,  152,  184,  141, }, 
  { 107,  153,  125,  167,  153,  140,   91,  137,  153,  122,  167,  139,  107,  153,  140,  167,  183,  140, }, 
  { 107,  153,  125,  167,  153,  140,   91,  137,  153,  107,  167,  139,  107,  153,  140,  167,  183,  140, }, 
};
#endif
#else
static const UChar 
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] = 
{
  { 140,   92,  137,  138,  140,  152,  138,  139,  126,  168,  139,  139,  153,   74,  149,   92,  139,  107,  122,  152,  110,   93,  152,  138,  140,  179,  166,  182,  140,  227,  122,  197, }, 
  { 154,  196,  196,  167,  154,  152,  167,  182,  155,  139,  139,  139,  182,  134,  149,  136,  153,  121,  136,  137,  139,  122,  152,  167,  169,  194,  166,  167,  154,  167,  137,  182, }, 
  { 154,  196,  167,  167,  154,  152,  167,  182,  155,  139,  139,  139,  182,  134,  149,  136,  153,  121,  136,  122,  139,  107,  152,  152,  169,  208,  166,  167,  154,  152,  167,  182, }, 
};

#if RESTRICT_GR1GR2FLAG_NUMBER
static const UChar 
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =  
{
  { 138,  153,  139,  136,  167,  153,  152,  152, }, 
  { 107,  167,  139,   91,  122,  152,  107,  167, }, 
  { 107,  167,  139,   91,  107,   93,  107,  167, }, 
};
#else
static const UChar 
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =  
{
  { 138,  139,  111,  153,  139,  111,  139,  125,  111,  136,  167,  139,  167,  153,  139,  153,  139,  110,  152,  139,  140,  152,  184,  141, }, 
  { 107,  153,  125,  167,  153,  140,  139,  154,  155,   91,  137,  153,  122,  167,  139,  152,  138,  139,  107,  153,  140,  167,  183,  140, }, 
  { 107,  153,  125,  167,  153,  140,  139,  154,  155,   91,  137,  153,  107,  167,  139,   93,  138,  139,  107,  153,  140,  167,  183,  140, }, 
};
#endif
#endif

static const UChar 
INIT_MVP_IDX[3][NUM_MVP_IDX_CTX] =  
{
  { CNU,  CNU, }, 
  { 168,  CNU, }, 
  { 168,  CNU, }, 
};

static const UChar 
INIT_ALF_FLAG[3][NUM_ALF_FLAG_CTX] = 
{
  { 153, }, 
  { 153, }, 
  { 153, }, 
};

static const UChar 
INIT_ALF_UVLC[3][NUM_ALF_UVLC_CTX] = 
{
  { 140,  154, }, 
  { 154,  154, }, 
  { 154,  154, }, 
};

static const UChar 
INIT_ALF_SVLC[3][NUM_ALF_SVLC_CTX] =  
{
  { 187,  154,  159, }, 
  { 141,  154,  189, }, 
  { 141,  154,  159, }, 
};

static const UChar 
INIT_SAO_FLAG[3][NUM_SAO_FLAG_CTX] =  
{
  { 154, }, 
  { 153, }, 
  { 153, }, 
};

static const UChar 
INIT_SAO_UVLC[3][NUM_SAO_UVLC_CTX] =  
{
  { 143,  140, }, 
  { 185,  140, }, 
  { 200,  140, }, 
};

static const UChar 
INIT_SAO_SVLC[3][NUM_SAO_SVLC_CTX] = 
{
  { 247,  154,  244, }, 
  { 215,  154,  169, }, 
  { 215,  154,  169, }, 
};

#if SAO_UNIT_INTERLEAVING 
static const UChar 
INIT_SAO_MERGE_LEFT_FLAG[3][NUM_SAO_MERGE_LEFT_FLAG_CTX] = 
{
  { 153,  153,  153, }, 
  { 153,  153,  153, }, 
  { 153,  153,  153, }, 
};

static const UChar 
INIT_SAO_MERGE_UP_FLAG[3][NUM_SAO_MERGE_UP_FLAG_CTX] = 
{
  { 175, }, 
  { 153, }, 
  { 153, }, 
};

static const UChar 
INIT_SAO_TYPE_IDX[3][NUM_SAO_TYPE_IDX_CTX] = 
{
  { 160,  140, }, 
  { 185,  140, }, 
  { 200,  140, }, 
};
#endif

static const UChar 
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX] = 
{
{ CNU,  224,  167,  122,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
{ CNU,  124,  138,   94,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
{ CNU,  153,  138,  138,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
};
#else
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
#if MRG_IDX_CTX_RED
  {
    CNU,
    
  },
  {
    100,
    
  },
  {
    116,
  },
#else
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    100,  86, 102, 133,
    
  },
  {
    116,  87, 119, 103,
    
  },
#endif
};

// initial probability for PU size
static const UChar
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] =
{
  {
    167, CNU, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
};
#if AMP_CTX
static const UChar
INIT_CU_AMP_POS[3][NUM_CU_AMP_CTX] =
{
  {
    CNU, 
  },
  {
    119, 
  },
  {
    119, 
  },
};
#else
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
  {
    CNU,
    
  },
  {
    114,
    
  },
  {
    98,
    
  },
};

// initial probability for intra direction of luma
static const UChar
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX] =
{
  {
    167,
    
  },
  {
    119,
    
  },
  {
    150,
    
  },
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
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX] =
{
  {
     73,  74, CNU, CNU, CNU,
     55,  86, 133, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
    114,  84, 117, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
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

#if LAST_CTX_REDUCTION
static const UChar
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =
{
  {
    72,  72,  71,  72, 104,  89,  88,  89,  59,  73,  89, 106,  60,  59,  43,   
    54,  70,  53,  CNU, CNU, CNU,  CNU, CNU, CNU,  CNU, CNU, CNU, CNU, CNU, CNU,
  },
  {
    57,  72,  55,  72,  57,  72,   88,  73,  73,  72,  103,  73,  89,  73,  57,  
    54,  70,  54,  CNU, CNU, CNU,  CNU, CNU, CNU,  CNU, CNU, CNU, CNU, CNU, CNU,
  },
  {
    88,  72,  71,  72,  57,  72,  88,  73,  73,  72,   103,  73,  89,  73,  57,   
    54,  70,  69,   CNU, CNU, CNU,  CNU, CNU, CNU,  CNU, CNU, CNU, CNU, CNU, CNU,
  },
};
#else
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
#endif

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

static const UChar
INIT_SIG_FLAG[3][NUM_SIG_FLAG_CTX] =
{
  {
    74,  73,  88,  72,  72,  55,  71,  54,  71,  88, 103,  71,  53,  87, 134,  86,  84,  70,  68,  89,  90,  84,  88,  74, 130, 118,  88,
    120,  87, 149,  70,  52, 118, 133, 116, 114, 129, 132, 162, 115,  51, 115,  66, 120,  74, 115,  87,  89,
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53, 118, 134, 118, 101,  68,  85, 101, 116, 100,  68,  67, 136, 168, 147, 150, 120, 115, 118, 119,
    136, 102,  70,  53,  67, 117, 102, 117, 115, 114,  84, 115,  99, 100,  83, 114, 152, 168, 131, 150, 120,
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53,  71, 103, 118, 101,  68,  85, 101, 116, 116,  68,  67, 152, 168, 147, 150, 120, 115, 118, 119,
    136, 102,  86,  84,  67, 117, 102, 117, 115,  99, 100, 115,  99, 100,  83, 114, 152, 152, 131, 150, 120,
  },
};

#if LEVEL_CTX_LUMA_RED
static const UChar
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] =
{
  {
    104,  68, 116,  86, 104, 132,  86,  87, 102,  66, 114,  68,  87,  84, 100, 101, 
      104, 130, 147, 149, 104, 196, 100, 165,
  },
  {
    119, 179, 179, 164, 119,  85, 117, 149, 133,  98, 114, 115, 118,  99, 115, 116,
      135, 146, 147, 164, 119, 148, 116, 133,
  },
  {
    119, 179, 148, 164, 119,  85, 117, 149, 133,  98, 114, 115, 118,  99, 115, 100,
      135, 177, 147, 164, 119, 132, 148, 149,
  },
};

#if RESTRICT_GR1GR2FLAG_NUMBER
static const UChar
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =
{
  {
    86, 102, 115, 117, 101, 101,
  },
  {
    84, 117, 83, 100, 84, 117,
  },
  {
    84, 117, 83,  84, 84, 117,
  },
};
#else
static const UChar
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =
{
  {
    86, 103,  73, 102, 103,  73, 115, 117, 103, 117, 118, 103,
      101, 103, 104, 101, 167, 121,
  },
  {
    84, 102,  88, 117, 118, 104, 83, 116, 118, 100, 117,  87,
      84, 118, 120, 117, 150, 120,
    },
    {
      84, 102,  88, 117, 118, 104, 83, 116, 118,  84, 117,  87,
        84, 118, 120, 117, 150, 120,
    },
};
#endif
#else
static const UChar
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] =
{
  {
    104,  68, 116,  86, 104, 132,  86,  87, 105, 134,  87, 103, 102,  66, 114,  68,  87,  84, 100, 101,  72,  69, 101,  86,
    104, 130, 147, 149, 104, 196, 100, 165,
  },
  {
    119, 179, 179, 164, 119,  85, 117, 149, 136, 103, 103, 103, 133,  98, 114, 115, 118,  99, 115, 116,  87, 100,  85, 117,
    135, 146, 147, 164, 119, 148, 116, 133,
  },
  {
    119, 179, 148, 164, 119,  85, 117, 149, 136,  87, 103, 103, 133,  98, 114, 115, 118,  99, 115, 100,  87,  84,  85,  85,
    135, 177, 147, 164, 119, 132, 148, 149,
  },
};

#if RESTRICT_GR1GR2FLAG_NUMBER
static const UChar
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =
{
  {
    86, 102, 103, 115, 117, 102, 101, 101,
  },
  {
    84, 117, 103, 83, 100, 85, 84, 117, 
  },
  {
    84, 117, 87, 83, 84, 69, 84, 117,
  },
};
#else
static const UChar
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =
{
  {
    86, 103,  73, 102, 103,  73, 103,  88,  89, 115, 117, 103, 117, 118, 103, 102, 103,  72,
    101, 103, 104, 101, 167, 121,
  },
  {
    84, 102,  88, 117, 118, 104, 103, 119, 136,  83, 116, 118, 100, 117,  87,  85,  86, 103,
    84, 118, 120, 117, 150, 120,
  },
  {
    84, 102,  88, 117, 118, 104,  87, 119, 136,  83, 116, 118,  84, 117,  87,  69,  86,  87,
    84, 118, 120, 117, 150, 120,
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

#if SAO_UNIT_INTERLEAVING
static const UChar
INIT_SAO_MERGE_LEFT_FLAG[3][NUM_SAO_MERGE_LEFT_FLAG_CTX] =
{
  {
    118, 118, 118,
  },
  {
    102, 102, 102,
    },
    {
      102, 102, 102,
    },
};

static const UChar
INIT_SAO_MERGE_UP_FLAG[3][NUM_SAO_MERGE_UP_FLAG_CTX] =
{
  {
    109, 
  },
  {
    102,
  },
  {
    102,
  },
};
static const UChar
INIT_SAO_TYPE_IDX[3][NUM_SAO_TYPE_IDX_CTX] =
{
  {
    64, 104 
  },
  {
  168, 120
  },
  {
    184, 120
  },
};
static const Short
INIT_SAO_RUN[3][NUM_SAO_RUN_CTX][2] =
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
#endif
//! \}


#endif

