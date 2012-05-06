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
#define NUM_MERGE_IDX_EXT_CTX         1       ///< number of context models for merge index of merge extended

#define NUM_ALF_CTRL_FLAG_CTX         1       ///< number of context models for ALF control flag
#define NUM_PART_SIZE_CTX             4       ///< number of context models for partition size
#define NUM_CU_AMP_CTX                1       ///< number of context models for partition size (AMP)
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
#define NUM_CTX_LAST_FLAG_XY          15      ///< number of context models for last coefficient position

#define NUM_ONE_FLAG_CTX              24      ///< number of context models for greater than 1 flag
#define NUM_ONE_FLAG_CTX_LUMA         16      ///< number of context models for greater than 1 flag of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than 1 flag of chroma
#define NUM_ABS_FLAG_CTX               6      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA          4      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        2      ///< number of context models for greater than 2 flag of chroma

#define NUM_MVP_IDX_CTX               2       ///< number of context models for MVP index

#define NUM_ALF_FLAG_CTX              1       ///< number of context models for ALF flag
#define NUM_ALF_UVLC_CTX              2       ///< number of context models for ALF UVLC (filter length)
#define NUM_ALF_SVLC_CTX              3       ///< number of context models for ALF SVLC (filter coeff.)

#define NUM_SAO_FLAG_CTX              1       ///< number of context models for SAO flag
#define NUM_SAO_UVLC_CTX              2       ///< number of context models for SAO UVLC
#define NUM_SAO_SVLC_CTX              3       ///< number of context models for SAO SVLC
#define NUM_SAO_RUN_CTX               3       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_MERGE_LEFT_FLAG_CTX   3       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_MERGE_UP_FLAG_CTX     1       ///< number of context models for AO SVLC (filter coeff.)
#define NUM_SAO_TYPE_IDX_CTX          2       ///< number of context models for AO SVLC (filter coeff.)
#define CNU                          154      ///< dummy initialization value for unused context models 'Context model Not Used'

// ====================================================================================================================
// Tables
// ====================================================================================================================

// initial probability for split flag
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
  { CNU, }, 
  { 122, }, 
  { 137, }, 
};

static const UChar 
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] =  
{
  { 184,  CNU,  CNU,  CNU, }, 
  { 154,  139,  CNU,  CNU, }, 
  { 154,  139,  CNU,  CNU, }, 
};

static const UChar 
INIT_CU_AMP_POS[3][NUM_CU_AMP_CTX] =  
{
  { CNU, }, 
  { 154, }, 
  { 154, }, 
};

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
  {  95,   79,   63,   31, }, 
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

static const UChar 
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] = 
{
  { 140,   92,  137,  138,  140,  152,  138,  139,  153,   74,  149,   92,  139,  107,  122,  152,  140,  179,  166,  182,  140,  227,  122,  197, }, 
  { 154,  196,  196,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  137,  169,  194,  166,  167,  154,  167,  137,  182, }, 
  { 154,  196,  167,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  122,  169,  208,  166,  167,  154,  152,  167,  182, }, 
};

static const UChar 
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =  
{
  { 138,  153,  136,  167,  152,  152, }, 
  { 107,  167,   91,  122,  107,  167, }, 
  { 107,  167,   91,  107,  107,  167, }, 
};

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

static const UChar 
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX] = 
{
{ CNU,  224,  167,  122,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
{ CNU,  124,  138,   94,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
{ CNU,  153,  138,  138,  CNU,  CNU,  CNU,  CNU,  CNU,  CNU, }, 
};
//! \}


#endif

