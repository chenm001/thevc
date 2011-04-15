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

/** \file     TComTrQuant.cpp
    \brief    transform and quantization class
*/

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "TComTrQuant.h"
#include "TComPic.h"
#include "ContextTables.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_CHROMA                 1           ///< use of RDOQ in chroma

#define DQ_BITS                     6
#define Q_BITS_8                    16
#define SIGN_BITS                   1

// ====================================================================================================================
// Tables
// ====================================================================================================================

// RDOQ parameter
Int entropyBits[128]=
{
  895,    943,    994,   1048,   1105,   1165,   1228,   1294,
  1364,   1439,   1517,   1599,   1686,   1778,   1875,   1978,
  2086,   2200,   2321,   2448,   2583,   2725,   2876,   3034,
  3202,   3380,   3568,   3767,   3977,   4199,   4435,   4684,
  4948,   5228,   5525,   5840,   6173,   6527,   6903,   7303,
  7727,   8178,   8658,   9169,   9714,  10294,  10914,  11575,
  12282,  13038,  13849,  14717,  15650,  16653,  17734,  18899,
  20159,  21523,  23005,  24617,  26378,  28306,  30426,  32768,
  32768,  35232,  37696,  40159,  42623,  45087,  47551,  50015,
  52479,  54942,  57406,  59870,  62334,  64798,  67262,  69725,
  72189,  74653,  77117,  79581,  82044,  84508,  86972,  89436,
  91900,  94363,  96827,  99291, 101755, 104219, 106683, 109146,
  111610, 114074, 116538, 119002, 121465, 123929, 126393, 128857,
  131321, 133785, 136248, 138712, 141176, 143640, 146104, 148568,
  151031, 153495, 155959, 158423, 160887, 163351, 165814, 168278,
  170742, 173207, 175669, 178134, 180598, 183061, 185525, 187989
};

static const Int estErr4x4[6][4][4]=
{
  {
    {25600, 27040, 25600, 27040},
    {27040, 25600, 27040, 25600},
    {25600, 27040, 25600, 27040},
    {27040, 25600, 27040, 25600}
  },
  {
    {30976, 31360, 30976, 31360},
    {31360, 32400, 31360, 32400},
    {30976, 31360, 30976, 31360},
    {31360, 32400, 31360, 32400}
  },
  {
    {43264, 40960, 43264, 40960},
    {40960, 40000, 40960, 40000},
    {43264, 40960, 43264, 40960},
    {40960, 40000, 40960, 40000}
  },
  {
    {50176, 51840, 50176, 51840},
    {51840, 52900, 51840, 52900},
    {50176, 51840, 50176, 51840},
    {51840, 52900, 51840, 52900}
  },
  {
    {65536, 64000, 65536, 64000},
    {64000, 62500, 64000, 62500},
    {65536, 64000, 65536, 64000},
    {64000, 62500, 64000, 62500}
  },
  {
    {82944, 84640, 82944, 84640},
    {84640, 84100, 84640, 84100},
    {82944, 84640, 82944, 84640},
    {84640, 84100, 84640, 84100}
  }
};

static const Int estErr8x8[6][8][8]={
  {
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201}
  },
  {
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770}
  },
  {
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652}
  },
  {
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156}
  },
  {
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116}
  },
  {
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376}
  }
};

#if QC_MOD_LCEC
#if CAVLC_COEF_LRG_BLK
  static const int VLClength[14][128] = {
#else
  static const int VLClength[13][128] = {
#endif
    { 1, 2, 3, 4, 5, 6, 7, 9, 9,11,11,11,11,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,10,10,10,10,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16},
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,63,64,64,65,65},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,24,24,24,24,25,25,25,25,26,26,26,26,27,27,27,27,28,28,28,28,29,29,29,29,30,30,30,30,31,31,31,31,32,32,32,32,33,33,33,33,34,34,34,34},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19},
    { 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13},
    { 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15},

    { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
#if CAVLC_COEF_LRG_BLK
    ,{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12}
#endif
  };
#endif
static const Int estErr16x16[6] = { 25329, 30580, 42563, 49296, 64244, 82293 };
static const Int estErr32x32[6] = { 25351, 30674, 42843, 49687, 64898, 82136 };

// ====================================================================================================================
// Qp class member functions
// ====================================================================================================================

QpParam::QpParam()
{
}

Void QpParam::initOffsetParam( Int iStartQP, Int iEndQP )
{
  Int iDefaultOffset;
  Int iDefaultOffset_LTR;
  
  Int iPer;
  
  for (Int iQP = iStartQP; iQP <= iEndQP; iQP++)
  {
    for (UInt uiSliceType = 0; uiSliceType < 3; uiSliceType++)
    {
      Int k =  (iQP + 6*g_uiBitIncrement)/6;
#if FULL_NBIT
      k += g_uiBitDepth - 8;
#endif
      
      Bool bLowPass = (uiSliceType == 0);
      iDefaultOffset = (bLowPass? 10922 : 5462);
      
      bLowPass = (uiSliceType == 0);
      iDefaultOffset_LTR = (bLowPass? 170 : 86);
      
      iPer = QP_BITS + k - QOFFSET_BITS;
      m_aiAdd2x2[iQP][uiSliceType] = iDefaultOffset << iPer;
      m_aiAdd4x4[iQP][uiSliceType] = iDefaultOffset << iPer;
      
      iPer = QP_BITS + 1 + k - QOFFSET_BITS;
      m_aiAdd8x8[iQP][uiSliceType] = iDefaultOffset << iPer;
      
      iPer = ECore16Shift + k - QOFFSET_BITS_LTR;
      m_aiAdd16x16[iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
      
      iPer = ECore32Shift + k - QOFFSET_BITS_LTR;
      m_aiAdd32x32[iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
    }
  }
}

// ====================================================================================================================
// TComTrQuant class member functions
// ====================================================================================================================

TComTrQuant::TComTrQuant()
{
  m_cQP.clear();
  
  // allocate temporary buffers
  m_plTempCoeff  = new Long[ MAX_CU_SIZE*MAX_CU_SIZE ];
  
  // allocate bit estimation class  (for RDOQ)
  m_pcEstBitsSbac = new estBitsSbacStruct;
}

TComTrQuant::~TComTrQuant()
{
  // delete temporary buffers
  if ( m_plTempCoeff )
  {
    delete [] m_plTempCoeff;
    m_plTempCoeff = NULL;
  }
  
  // delete bit estimation class
  if ( m_pcEstBitsSbac ) delete m_pcEstBitsSbac;
}

/// Including Chroma QP Parameter setting
Void TComTrQuant::setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType)
{
  iQP = Max( Min( iQP, 51 ), 0 );
  
  if(eTxtType != TEXT_LUMA) //Chroma
  {
    iQP  = g_aucChromaScale[ iQP ];
  }
  
  m_cQP.setQpParam( iQP, bLowpass, eSliceType, m_bEnc );
}

#if E243_CORE_TRANSFORMS

#if MATRIX_MULT
/** NxN forward transform (2D) using brute force matrix multiplication (3 nested loops)
 *  \param block pointer to input data (residual)
 *  \param coeff pointer to output data (transform coefficients)
 *  \param uiStride stride of input data
 *  \param uiTrSize transform size (uiTrSize x uiTrSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
void xTr(Pel *block, Long *coeff, UInt uiStride, UInt uiTrSize, UInt uiMode)
#else
void xTr(Pel *block, Long *coeff, UInt uiStride, UInt uiTrSize)
#endif
{
  Int i,j,k,iSum;
  Int tmp[32*32];
  const short *iT;
  UInt uiLog2TrSize = g_aucConvertToBit[ uiTrSize ] + 2;

  if (uiTrSize==4)
  {
    iT  = g_aiT4[0];
  }
  else if (uiTrSize==8)
  {
    iT = g_aiT8[0];
  }
  else if (uiTrSize==16)
  {
    iT = g_aiT16[0];
  }
  else if (uiTrSize==32)
  {
    iT = g_aiT32[0];
  }
  else{
    assert(0);
  }

#if FULL_NBIT
  int shift_1st = uiLog2TrSize - 1 + g_uiBitDepth - 8; // log2(N) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = uiLog2TrSize - 1 + g_uiBitIncrement; // log2(N) - 1 + g_uiBitIncrement
#endif

  int add_1st = 1<<(shift_1st-1);
  int shift_2nd = uiLog2TrSize + 6;
  int add_2nd = 1<<(shift_2nd-1);

  /* Horizontal transform */

#if INTRA_DST_TYPE_7
  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])
    {
      iT  =  g_as_DST_MAT_4[0];
    }
  }
#endif
  for (i=0; i<uiTrSize; i++)
  {
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {
        iSum += iT[i*uiTrSize+k]*block[j*uiStride+k];
      }
      tmp[i*uiTrSize+j] = (iSum + add_1st)>>shift_1st;
    }
  }
/* Vertical transform */
#if INTRA_DST_TYPE_7
  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode])
    {
      iT  =  g_as_DST_MAT_4[0];
    }
    else
    {
      iT  = g_aiT4[0];
    }
  }
 #endif
  for (i=0; i<uiTrSize; i++)
  {                 
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {
        iSum += iT[i*uiTrSize+k]*tmp[j*uiTrSize+k];        
      }
      coeff[i*uiTrSize+j] = (iSum + add_2nd)>>shift_2nd; 
    }
  }  
}

/** NxN inverse transform (2D) using brute force matrix multiplication (3 nested loops)
 *  \param coeff pointer to input data (transform coefficients)
 *  \param block pointer to output data (residual)
 *  \param uiStride stride of output data
 *  \param uiTrSize transform size (uiTrSize x uiTrSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
void xITr(Long *coeff, Pel *block, UInt uiStride, UInt uiTrSize, UInt uiMode)
#else
void xITr(Long *coeff, Pel *block, UInt uiStride, UInt uiTrSize)
#endif
{
  int i,j,k,iSum;
  Int tmp[32*32];
  const short *iT;
  UInt uiLog2TrSize = g_aucConvertToBit[ uiTrSize ] + 2;
  if (uiTrSize==4)
  {
    iT  = g_aiT4[0];
  }
  else if (uiTrSize==8)
  {
    iT = g_aiT8[0];
  }
  else if (uiTrSize==16)
  {
    iT = g_aiT16[0];
  }
  else if (uiTrSize==32)
  {
    iT = g_aiT32[0];
  }
  else{
    assert(0);
  }
  int shift_1st = SHIFT_INV_1ST;
  int add_1st = 1<<(shift_1st-1);  
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
  int add_2nd = 1<<(shift_2nd-1);
#if INTRA_DST_TYPE_7
  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] ) // Check for DCT or DST
    {
      iT  =  g_as_DST_MAT_4[0];
    }
  }
#endif
  /* Horizontal transform */
  for (i=0; i<uiTrSize; i++)
  {    
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {        
        iSum += iT[k*uiTrSize+i]*coeff[k*uiTrSize+j]; 
      }
      tmp[i*uiTrSize+j] = (iSum + add_1st)>>shift_1st;
    }
  }   
#if INTRA_DST_TYPE_7
  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode] )   // Check for DCT or DST
    {
      iT  =  g_as_DST_MAT_4[0];
    }
    else  
    {
      iT  = g_aiT4[0];
    }
  }
#endif
  /* Vertical transform */
  for (i=0; i<uiTrSize; i++)
  {   
    for (j=0; j<uiTrSize; j++)
    {
      iSum = 0;
      for (k=0; k<uiTrSize; k++)
      {        
        iSum += iT[k*uiTrSize+j]*tmp[i*uiTrSize+k];
      }
      block[i*uiStride+j] = (iSum + add_2nd)>>shift_2nd;
    }
  }
}

#else //MATRIX_MULT

/** 4x4 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly4(short block[4][4],short coeff[4][4],int shift)
{
  int j;  
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* E and O */
    E[0] = block[j][0] + block[j][3];
    O[0] = block[j][0] - block[j][3];
    E[1] = block[j][1] + block[j][2];
    O[1] = block[j][1] - block[j][2];

    coeff[0][j] = (g_aiT4[0][0]*E[0] + g_aiT4[0][1]*E[1] + add)>>shift;
    coeff[2][j] = (g_aiT4[2][0]*E[0] + g_aiT4[2][1]*E[1] + add)>>shift;
    coeff[1][j] = (g_aiT4[1][0]*O[0] + g_aiT4[1][1]*O[1] + add)>>shift;
    coeff[3][j] = (g_aiT4[3][0]*O[0] + g_aiT4[3][1]*O[1] + add)>>shift;
  }
}

#if INTRA_DST_TYPE_7
// Fast DST Algorithm. Full matrix multiplication for DST and Fast DST algorithm 
// give identical results
void fastForwardDst(short block[4][4],short coeff[4][4],int shift)  // input block, output coeff
{
  int i, c[4];
  int rnd_factor = 1<<(shift-1);
  for (i=0; i<4; i++)
  {
    // Intermediate Variables
    c[0] = block[i][0] + block[i][3];
    c[1] = block[i][1] + block[i][3];
    c[2] = block[i][0] - block[i][1];
    c[3] = 74* block[i][2];
    
    coeff[0][i] =  ( 29 * c[0] + 55 * c[1]         + c[3]               + rnd_factor ) >> shift;
    coeff[1][i] =  ( 74 * (block[i][0]+ block[i][1] - block[i][3])      + rnd_factor ) >> shift;
    coeff[2][i] =  ( 29 * c[2] + 55 * c[0]         - c[3]               + rnd_factor ) >> shift;
    coeff[3][i] =  ( 55 * c[2] - 29 * c[1]         + c[3]               + rnd_factor ) >> shift;
  }
}
void fastInverseDst(short tmp[4][4],short block[4][4],int shift)  // input tmp, output block
{
  int i, c[4];
  int rnd_factor = 1<<(shift-1);
  for (i=0; i<4; i++)
  {  
    // Intermediate Variables
    c[0] = tmp[0][i] + tmp[2][i];
    c[1] = tmp[2][i] + tmp[3][i];
    c[2] = tmp[0][i] - tmp[3][i];
    c[3] = 74* tmp[1][i];

    block[i][0] =  ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift;
    block[i][1] =  ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift;
    block[i][2] =  ( 74 * (tmp[0][i] - tmp[2][i]  + tmp[3][i])      + rnd_factor ) >> shift;
    block[i][3] =  ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift;
  }
}
#endif 
/** 4x4 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
void xTr4(short block[4][4],short coeff[4][4],UInt uiMode)
#else
void xTr4(short block[4][4],short coeff[4][4])
#endif
{
#if FULL_NBIT
  int shift_1st = 1 + g_uiBitDepth - 8; // log2(4) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = 1 + g_uiBitIncrement; // log2(4) - 1 + g_uiBitIncrement
#endif
  int shift_2nd = 8;                    // log2(4) + 6
  short tmp[4][4]; 
#if INTRA_DST_TYPE_7
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])// Check for DCT or DST
  {
    fastForwardDst(block,tmp,shift_1st); // Forward DST BY FAST ALGORITHM, block input, tmp output
  }
  else  
  {
    partialButterfly4(block,tmp,shift_1st);
  }
#else
  partialButterfly4(block,tmp,shift_1st);
#endif

#if INTRA_DST_TYPE_7
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )   // Check for DCT or DST
  {
    fastForwardDst(tmp,coeff,shift_2nd); // Forward DST BY FAST ALGORITHM, tmp input, coeff output
  }
  else  
  {
    partialButterfly4(tmp,coeff,shift_2nd);
  }   
#else
  partialButterfly4(tmp,coeff,shift_2nd);
#endif
}

/** 4x4 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse4(short tmp[4][4],short block[4][4],int shift)
{
  int j;    
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */    
    O[0] = g_aiT4[1][0]*tmp[1][j] + g_aiT4[3][0]*tmp[3][j];
    O[1] = g_aiT4[1][1]*tmp[1][j] + g_aiT4[3][1]*tmp[3][j];
    E[0] = g_aiT4[0][0]*tmp[0][j] + g_aiT4[2][0]*tmp[2][j];
    E[1] = g_aiT4[0][1]*tmp[0][j] + g_aiT4[2][1]*tmp[2][j];
    
    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    block[j][0] = (E[0] + O[0] + add)>>shift;
    block[j][1] = (E[1] + O[1] + add)>>shift;
    block[j][2] = (E[1] - O[1] + add)>>shift;
    block[j][3] = (E[0] - O[0] + add)>>shift;
  }
}

/** 4x4 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
void xITr4(short coeff[4][4],short block[4][4], UInt uiMode)
#else
void xITr4(short coeff[4][4],short block[4][4])
#endif
{
  int shift_1st = SHIFT_INV_1ST;
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
  short tmp[4][4];
  
#if INTRA_DST_TYPE_7
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )    // Check for DCT or DST
  {
    fastInverseDst(coeff,tmp,shift_1st);    // Inverse DST by FAST Algorithm, coeff input, tmp output
  }
  else
  {
    partialButterflyInverse4(coeff,tmp,shift_1st);    
  } 
#else
  partialButterflyInverse4(coeff,tmp,shift_1st);
#endif
#if INTRA_DST_TYPE_7
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode] )    // Check for DCT or DST
  {
    fastInverseDst(tmp,block,shift_2nd); // Inverse DST by FAST Algorithm, tmp input, coeff output
  }
  else
  {
    partialButterflyInverse4(tmp,block,shift_2nd);
  }   
#else
   partialButterflyInverse4(tmp,block,shift_2nd);
#endif
}

/** 8x8 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly8(short block[8][8],short coeff[8][8],int shift)
{
  int j,k;  
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<8; j++)
  {    
    /* E and O*/
    for (k=0;k<4;k++)
    {
      E[k] = block[j][k] + block[j][7-k];
      O[k] = block[j][k] - block[j][7-k];
    }    
    /* EE and EO */
    EE[0] = E[0] + E[3];    
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    coeff[0][j] = (g_aiT8[0][0]*EE[0] + g_aiT8[0][1]*EE[1] + add)>>shift;
    coeff[4][j] = (g_aiT8[4][0]*EE[0] + g_aiT8[4][1]*EE[1] + add)>>shift; 
    coeff[2][j] = (g_aiT8[2][0]*EO[0] + g_aiT8[2][1]*EO[1] + add)>>shift;
    coeff[6][j] = (g_aiT8[6][0]*EO[0] + g_aiT8[6][1]*EO[1] + add)>>shift; 

    coeff[1][j] = (g_aiT8[1][0]*O[0] + g_aiT8[1][1]*O[1] + g_aiT8[1][2]*O[2] + g_aiT8[1][3]*O[3] + add)>>shift;
    coeff[3][j] = (g_aiT8[3][0]*O[0] + g_aiT8[3][1]*O[1] + g_aiT8[3][2]*O[2] + g_aiT8[3][3]*O[3] + add)>>shift;
    coeff[5][j] = (g_aiT8[5][0]*O[0] + g_aiT8[5][1]*O[1] + g_aiT8[5][2]*O[2] + g_aiT8[5][3]*O[3] + add)>>shift;
    coeff[7][j] = (g_aiT8[7][0]*O[0] + g_aiT8[7][1]*O[1] + g_aiT8[7][2]*O[2] + g_aiT8[7][3]*O[3] + add)>>shift;
  }
}

/** 8x8 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff  output data (transform coefficients)
 */
void xTr8(short block[8][8],short coeff[8][8])
{
#if FULL_NBIT
  int shift_1st = 2 + g_uiBitDepth - 8; // log2(8) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = 2 + g_uiBitIncrement; // log2(8) - 1 + g_uiBitIncrement
#endif
  int shift_2nd = 9;                    // log2(8) + 6
  short tmp[8][8]; 

  partialButterfly8(block,tmp,shift_1st);
  partialButterfly8(tmp,coeff,shift_2nd);
}

/** 8x8 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse8(short tmp[8][8],short block[8][8],int shift)
{
  int j,k;    
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<8; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<4;k++)
    {
      O[k] = g_aiT8[ 1][k]*tmp[ 1][j] + g_aiT8[ 3][k]*tmp[ 3][j] + g_aiT8[ 5][k]*tmp[ 5][j] + g_aiT8[ 7][k]*tmp[ 7][j];
    }
   
    EO[0] = g_aiT8[2][0]*tmp[2][j] + g_aiT8[6][0]*tmp[6][j];
    EO[1] = g_aiT8[2][1]*tmp[2][j] + g_aiT8[6][1]*tmp[6][j];
    EE[0] = g_aiT8[0][0]*tmp[0][j] + g_aiT8[4][0]*tmp[4][j];
    EE[1] = g_aiT8[0][1]*tmp[0][j] + g_aiT8[4][1]*tmp[4][j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      block[j][k] = (E[k] + O[k] + add)>>shift;
      block[j][k+4] = (E[3-k] - O[3-k] + add)>>shift;
    }        
  }
}

/** 8x8 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr8(short coeff[8][8],short block[8][8])
{
  int shift_1st = SHIFT_INV_1ST;
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
  short tmp[8][8];
  
  partialButterflyInverse8(coeff,tmp,shift_1st);
  partialButterflyInverse8(tmp,block,shift_2nd);
}

/** 16x16 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly16(short block[16][16],short coeff[16][16],int shift)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<16; j++)
  {    
    /* E and O*/
    for (k=0;k<8;k++)
    {
      E[k] = block[j][k] + block[j][15-k];
      O[k] = block[j][k] - block[j][15-k];
    } 
    /* EE and EO */
    for (k=0;k<4;k++)
    {
      EE[k] = E[k] + E[7-k];
      EO[k] = E[k] - E[7-k];
    }
    /* EEE and EEO */
    EEE[0] = EE[0] + EE[3];    
    EEO[0] = EE[0] - EE[3];
    EEE[1] = EE[1] + EE[2];
    EEO[1] = EE[1] - EE[2];

    coeff[ 0][j] = (g_aiT16[ 0][0]*EEE[0] + g_aiT16[ 0][1]*EEE[1] + add)>>shift;        
    coeff[ 8][j] = (g_aiT16[ 8][0]*EEE[0] + g_aiT16[ 8][1]*EEE[1] + add)>>shift;    
    coeff[ 4][j] = (g_aiT16[ 4][0]*EEO[0] + g_aiT16[ 4][1]*EEO[1] + add)>>shift;        
    coeff[12][j] = (g_aiT16[12][0]*EEO[0] + g_aiT16[12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      coeff[k][j] = (g_aiT16[k][0]*EO[0] + g_aiT16[k][1]*EO[1] + g_aiT16[k][2]*EO[2] + g_aiT16[k][3]*EO[3] + add)>>shift;      
    }
    
    for (k=1;k<16;k+=2)
    {
      coeff[k][j] = (g_aiT16[k][0]*O[0] + g_aiT16[k][1]*O[1] + g_aiT16[k][2]*O[2] + g_aiT16[k][3]*O[3] + 
                     g_aiT16[k][4]*O[4] + g_aiT16[k][5]*O[5] + g_aiT16[k][6]*O[6] + g_aiT16[k][7]*O[7] + add)>>shift;
    }

  }
}

/** 16x16 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr16(short block[16][16],short coeff[16][16])
{
 #if FULL_NBIT
  int shift_1st = 3 + g_uiBitDepth - 8; // log2(16) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = 3 + g_uiBitIncrement; // log2(16) - 1 + g_uiBitIncrement
#endif
  int shift_2nd = 10;                   // log2(16) + 6
  short tmp[16][16]; 

  partialButterfly16(block,tmp,shift_1st);
  partialButterfly16(tmp,coeff,shift_2nd);
}

/** 16x16 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse16(short tmp[16][16],short block[16][16],int shift)
{
  int j,k;  
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<16; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<8;k++)
    {
      O[k] = g_aiT16[ 1][k]*tmp[ 1][j] + g_aiT16[ 3][k]*tmp[ 3][j] + g_aiT16[ 5][k]*tmp[ 5][j] + g_aiT16[ 7][k]*tmp[ 7][j] + 
             g_aiT16[ 9][k]*tmp[ 9][j] + g_aiT16[11][k]*tmp[11][j] + g_aiT16[13][k]*tmp[13][j] + g_aiT16[15][k]*tmp[15][j];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[ 2][k]*tmp[ 2][j] + g_aiT16[ 6][k]*tmp[ 6][j] + g_aiT16[10][k]*tmp[10][j] + g_aiT16[14][k]*tmp[14][j];
    }
    EEO[0] = g_aiT16[4][0]*tmp[4][j] + g_aiT16[12][0]*tmp[12][j];
    EEE[0] = g_aiT16[0][0]*tmp[0][j] + g_aiT16[ 8][0]*tmp[ 8][j];
    EEO[1] = g_aiT16[4][1]*tmp[4][j] + g_aiT16[12][1]*tmp[12][j];
    EEE[1] = g_aiT16[0][1]*tmp[0][j] + g_aiT16[ 8][1]*tmp[ 8][j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    for (k=0;k<2;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+2] = EEE[1-k] - EEO[1-k];
    }    
    for (k=0;k<4;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+4] = EE[3-k] - EO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      block[j][k] = (E[k] + O[k] + add)>>shift;
      block[j][k+8] = (E[7-k] - O[7-k] + add)>>shift;
    }        
  }
}

/** 16x16 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr16(short coeff[16][16],short block[16][16])
{
  int shift_1st = SHIFT_INV_1ST;
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
  short tmp[16][16];
  
  partialButterflyInverse16(coeff,tmp,shift_1st);
  partialButterflyInverse16(tmp,block,shift_2nd);
}

/** 32x32 forward transform implemented using partial butterfly structure (1D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly32(short block[32][32],short coeff[32][32],int shift)
{
  int j,k;
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<32; j++)
  {    
    /* E and O*/
    for (k=0;k<16;k++)
    {
      E[k] = block[j][k] + block[j][31-k];
      O[k] = block[j][k] - block[j][31-k];
    } 
    /* EE and EO */
    for (k=0;k<8;k++)
    {
      EE[k] = E[k] + E[15-k];
      EO[k] = E[k] - E[15-k];
    }
    /* EEE and EEO */
    for (k=0;k<4;k++)
    {
      EEE[k] = EE[k] + EE[7-k];
      EEO[k] = EE[k] - EE[7-k];
    }
    /* EEEE and EEEO */
    EEEE[0] = EEE[0] + EEE[3];    
    EEEO[0] = EEE[0] - EEE[3];
    EEEE[1] = EEE[1] + EEE[2];
    EEEO[1] = EEE[1] - EEE[2];

    coeff[ 0][j] = (g_aiT32[ 0][0]*EEEE[0] + g_aiT32[ 0][1]*EEEE[1] + add)>>shift;
    coeff[16][j] = (g_aiT32[16][0]*EEEE[0] + g_aiT32[16][1]*EEEE[1] + add)>>shift;
    coeff[ 8][j] = (g_aiT32[ 8][0]*EEEO[0] + g_aiT32[ 8][1]*EEEO[1] + add)>>shift; 
    coeff[24][j] = (g_aiT32[24][0]*EEEO[0] + g_aiT32[24][1]*EEEO[1] + add)>>shift;
    for (k=4;k<32;k+=8)
    {
      coeff[k][j] = (g_aiT32[k][0]*EEO[0] + g_aiT32[k][1]*EEO[1] + g_aiT32[k][2]*EEO[2] + g_aiT32[k][3]*EEO[3] + add)>>shift;
    }       
    for (k=2;k<32;k+=4)
    {
      coeff[k][j] = (g_aiT32[k][0]*EO[0] + g_aiT32[k][1]*EO[1] + g_aiT32[k][2]*EO[2] + g_aiT32[k][3]*EO[3] + 
                     g_aiT32[k][4]*EO[4] + g_aiT32[k][5]*EO[5] + g_aiT32[k][6]*EO[6] + g_aiT32[k][7]*EO[7] + add)>>shift;
    }       
    for (k=1;k<32;k+=2)
    {
      coeff[k][j] = (g_aiT32[k][ 0]*O[ 0] + g_aiT32[k][ 1]*O[ 1] + g_aiT32[k][ 2]*O[ 2] + g_aiT32[k][ 3]*O[ 3] + 
                     g_aiT32[k][ 4]*O[ 4] + g_aiT32[k][ 5]*O[ 5] + g_aiT32[k][ 6]*O[ 6] + g_aiT32[k][ 7]*O[ 7] +
                     g_aiT32[k][ 8]*O[ 8] + g_aiT32[k][ 9]*O[ 9] + g_aiT32[k][10]*O[10] + g_aiT32[k][11]*O[11] + 
                     g_aiT32[k][12]*O[12] + g_aiT32[k][13]*O[13] + g_aiT32[k][14]*O[14] + g_aiT32[k][15]*O[15] + add)>>shift;
    }
  }
}

/** 32x32 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr32(short block[32][32],short coeff[32][32])
{
 #if FULL_NBIT
  int shift_1st = 4 + g_uiBitDepth - 8; // log2(32) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = 4 + g_uiBitIncrement; // log2(32) - 1 + g_uiBitIncrement
#endif
  int shift_2nd = 11;                   // log2(32) + 6
  short tmp[32][32]; 

  partialButterfly32(block,tmp,shift_1st);
  partialButterfly32(tmp,coeff,shift_2nd);
}

/** 32x32 inverse transform implemented using partial butterfly structure (1D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse32(short tmp[32][32],short block[32][32],int shift)
{
  int j,k;  
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<32; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<16;k++)
    {
      O[k] = g_aiT32[ 1][k]*tmp[ 1][j] + g_aiT32[ 3][k]*tmp[ 3][j] + g_aiT32[ 5][k]*tmp[ 5][j] + g_aiT32[ 7][k]*tmp[ 7][j] + 
             g_aiT32[ 9][k]*tmp[ 9][j] + g_aiT32[11][k]*tmp[11][j] + g_aiT32[13][k]*tmp[13][j] + g_aiT32[15][k]*tmp[15][j] + 
             g_aiT32[17][k]*tmp[17][j] + g_aiT32[19][k]*tmp[19][j] + g_aiT32[21][k]*tmp[21][j] + g_aiT32[23][k]*tmp[23][j] + 
             g_aiT32[25][k]*tmp[25][j] + g_aiT32[27][k]*tmp[27][j] + g_aiT32[29][k]*tmp[29][j] + g_aiT32[31][k]*tmp[31][j];
    }
    for (k=0;k<8;k++)
    {
      EO[k] = g_aiT32[ 2][k]*tmp[ 2][j] + g_aiT32[ 6][k]*tmp[ 6][j] + g_aiT32[10][k]*tmp[10][j] + g_aiT32[14][k]*tmp[14][j] + 
              g_aiT32[18][k]*tmp[18][j] + g_aiT32[22][k]*tmp[22][j] + g_aiT32[26][k]*tmp[26][j] + g_aiT32[30][k]*tmp[30][j];
    }
    for (k=0;k<4;k++)
    {
      EEO[k] = g_aiT32[4][k]*tmp[4][j] + g_aiT32[12][k]*tmp[12][j] + g_aiT32[20][k]*tmp[20][j] + g_aiT32[28][k]*tmp[28][j];
    }
    EEEO[0] = g_aiT32[8][0]*tmp[8][j] + g_aiT32[24][0]*tmp[24][j];
    EEEO[1] = g_aiT32[8][1]*tmp[8][j] + g_aiT32[24][1]*tmp[24][j];
    EEEE[0] = g_aiT32[0][0]*tmp[0][j] + g_aiT32[16][0]*tmp[16][j];    
    EEEE[1] = g_aiT32[0][1]*tmp[0][j] + g_aiT32[16][1]*tmp[16][j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    EEE[0] = EEEE[0] + EEEO[0];
    EEE[3] = EEEE[0] - EEEO[0];
    EEE[1] = EEEE[1] + EEEO[1];
    EEE[2] = EEEE[1] - EEEO[1];    
    for (k=0;k<4;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+4] = EEE[3-k] - EEO[3-k];
    }    
    for (k=0;k<8;k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+8] = EE[7-k] - EO[7-k];
    }    
    for (k=0;k<16;k++)
    {
      block[j][k] = (E[k] + O[k] + add)>>shift;
      block[j][k+16] = (E[15-k] - O[15-k] + add)>>shift;
    }        
  }
}

/** 32x32 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr32(short coeff[32][32],short block[32][32])
{
  int shift_1st = SHIFT_INV_1ST;
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
  short tmp[32][32];
  
  partialButterflyInverse32(coeff,tmp,shift_1st);
  partialButterflyInverse32(tmp,block,shift_2nd);
}
#endif //MATRIX_MULT
#else //E243_CORE_TRANSFORMS

Void TComTrQuant::xT32( Pel* pSrc, UInt uiStride, Long* pDes )
{
  Int x, y;
  Long aaiTemp[32][32];
  
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B20, B21, B22, B23, B24, B25, B26, B27;
  Long C0, C1, C2, C3, C4, C5, C6, C7, C10, C11, C12, C13, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31;
  Long D0, D1, D2, D3, D5, D6, D8, D9, D10, D11, D12, D13, D14, D15, D18, D19, D20, D21, D26, D27, D28, D29;
  Long E4, E5, E6, E7, E9, E10, E13, E14, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F17, F18, F21, F22, F25, F26, F29, F30;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift32x32-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    A0  = (pSrc[0] + pSrc[31])<<uiBitDepthIncrease;
    A31 = (pSrc[0] - pSrc[31])<<uiBitDepthIncrease;
    A1  = (pSrc[1] + pSrc[30])<<uiBitDepthIncrease;
    A30 = (pSrc[1] - pSrc[30])<<uiBitDepthIncrease;
    A2  = (pSrc[2] + pSrc[29])<<uiBitDepthIncrease;
    A29 = (pSrc[2] - pSrc[29])<<uiBitDepthIncrease;
    A3  = (pSrc[3] + pSrc[28])<<uiBitDepthIncrease;
    A28 = (pSrc[3] - pSrc[28])<<uiBitDepthIncrease;
    A4  = (pSrc[4] + pSrc[27])<<uiBitDepthIncrease;
    A27 = (pSrc[4] - pSrc[27])<<uiBitDepthIncrease;
    A5  = (pSrc[5] + pSrc[26])<<uiBitDepthIncrease;
    A26 = (pSrc[5] - pSrc[26])<<uiBitDepthIncrease;
    A6  = (pSrc[6] + pSrc[25])<<uiBitDepthIncrease;
    A25 = (pSrc[6] - pSrc[25])<<uiBitDepthIncrease;
    A7  = (pSrc[7] + pSrc[24])<<uiBitDepthIncrease;
    A24 = (pSrc[7] - pSrc[24])<<uiBitDepthIncrease;
    A8  = (pSrc[8] + pSrc[23])<<uiBitDepthIncrease;
    A23 = (pSrc[8] - pSrc[23])<<uiBitDepthIncrease;
    A9  = (pSrc[9] + pSrc[22])<<uiBitDepthIncrease;
    A22 = (pSrc[9] - pSrc[22])<<uiBitDepthIncrease;
    A10 = (pSrc[10] + pSrc[21])<<uiBitDepthIncrease;
    A21 = (pSrc[10] - pSrc[21])<<uiBitDepthIncrease;
    A11 = (pSrc[11] + pSrc[20])<<uiBitDepthIncrease;
    A20 = (pSrc[11] - pSrc[20])<<uiBitDepthIncrease;
    A12 = (pSrc[12] + pSrc[19])<<uiBitDepthIncrease;
    A19 = (pSrc[12] - pSrc[19])<<uiBitDepthIncrease;
    A13 = (pSrc[13] + pSrc[18])<<uiBitDepthIncrease;
    A18 = (pSrc[13] - pSrc[18])<<uiBitDepthIncrease;
    A14 = (pSrc[14] + pSrc[17])<<uiBitDepthIncrease;
    A17 = (pSrc[14] - pSrc[17])<<uiBitDepthIncrease;
    A15 = (pSrc[15] + pSrc[16])<<uiBitDepthIncrease;
    A16 = (pSrc[15] - pSrc[16])<<uiBitDepthIncrease;
#else
    A0 = pSrc[0] + pSrc[31];
    A31 = pSrc[0] - pSrc[31];
    A1 = pSrc[1] + pSrc[30];
    A30 = pSrc[1] - pSrc[30];
    A2 = pSrc[2] + pSrc[29];
    A29 = pSrc[2] - pSrc[29];
    A3 = pSrc[3] + pSrc[28];
    A28 = pSrc[3] - pSrc[28];
    A4 = pSrc[4] + pSrc[27];
    A27 = pSrc[4] - pSrc[27];
    A5 = pSrc[5] + pSrc[26];
    A26 = pSrc[5] - pSrc[26];
    A6 = pSrc[6] + pSrc[25];
    A25 = pSrc[6] - pSrc[25];
    A7 = pSrc[7] + pSrc[24];
    A24 = pSrc[7] - pSrc[24];
    A8 = pSrc[8] + pSrc[23];
    A23 = pSrc[8] - pSrc[23];
    A9 = pSrc[9] + pSrc[22];
    A22 = pSrc[9] - pSrc[22];
    A10 = pSrc[10] + pSrc[21];
    A21 = pSrc[10] - pSrc[21];
    A11 = pSrc[11] + pSrc[20];
    A20 = pSrc[11] - pSrc[20];
    A12 = pSrc[12] + pSrc[19];
    A19 = pSrc[12] - pSrc[19];
    A13 = pSrc[13] + pSrc[18];
    A18 = pSrc[13] - pSrc[18];
    A14 = pSrc[14] + pSrc[17];
    A17 = pSrc[14] - pSrc[17];
    A15 = pSrc[15] + pSrc[16];
    A16 = pSrc[15] - pSrc[16];
#endif
    B0 = A0 + A15;
    B15 = A0 - A15;
    B1 = A1 + A14;
    B14 = A1 - A14;
    B2 = A2 + A13;
    B13 = A2 - A13;
    B3 = A3 + A12;
    B12 = A3 - A12;
    B4 = A4 + A11;
    B11 = A4 - A11;
    B5 = A5 + A10;
    B10 = A5 - A10;
    B6 = A6 + A9;
    B9 = A6 - A9;
    B7 = A7 + A8;
    B8 = A7 - A8;
    B20 = xTrRound( 181 * ( A27 - A20 ) , DenShift32);
    B27 = xTrRound( 181 * ( A27 + A20 ) , DenShift32);
    B21 = xTrRound( 181 * ( A26 - A21 ) , DenShift32);
    B26 = xTrRound( 181 * ( A26 + A21 ) , DenShift32);
    B22 = xTrRound( 181 * ( A25 - A22 ) , DenShift32);
    B25 = xTrRound( 181 * ( A25 + A22 ) , DenShift32);
    B23 = xTrRound( 181 * ( A24 - A23 ) , DenShift32);
    B24 = xTrRound( 181 * ( A24 + A23 ) , DenShift32);;
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 181 * ( B13 - B10 ) , DenShift32);
    C13 = xTrRound( 181 * ( B13 + B10 ) , DenShift32);
    C11 = xTrRound( 181 * ( B12 - B11 ) , DenShift32);
    C12 = xTrRound( 181 * ( B12 + B11 ) , DenShift32);
    C16 = A16 + B23;
    C23 = A16 - B23;
    C24 = A31 - B24;
    C31 = A31 + B24;
    C17 = A17 + B22;
    C22 = A17 - B22;
    C25 = A30 - B25;
    C30 = A30 + B25;
    C18 = A18 + B21;
    C21 = A18 - B21;
    C26 = A29 - B26;
    C29 = A29 + B26;
    C19 = A19 + B20;
    C20 = A19 - B20;
    C27 = A28 - B27;
    C28 = A28 + B27;
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 181 * ( C6 - C5 ) , DenShift32);
    D6 = xTrRound( 181 * ( C6 + C5 ) , DenShift32);
    D18 = xTrRound( 97 * C29 - 236 * C18 , DenShift32);
    D20 = xTrRound(  - 236 * C27 - 97 * C20 , DenShift32);
    D26 = xTrRound(  - 236 * C21 + 97 * C26 , DenShift32);
    D28 = xTrRound( 97 * C19 + 236 * C28 , DenShift32);
    D19 = xTrRound( 97 * C28 - 236 * C19 , DenShift32);
    D21 = xTrRound(  - 236 * C26 - 97 * C21 , DenShift32);
    D27 = xTrRound(  - 236 * C20 + 97 * C27 , DenShift32);
    D29 = xTrRound( 97 * C18 + 236 * C29 , DenShift32);;
    
    aaiTemp[0][y] = xTrRound( 181 * ( D0 + D1 ) , DenShift32);
    aaiTemp[16][y] = xTrRound( 181 * ( D0 - D1 ) , DenShift32);
    aaiTemp[8][y] = xTrRound( 236 * D3 + 97 * D2 , DenShift32);
    aaiTemp[24][y] = xTrRound( 97 * D3 - 236 * D2 , DenShift32);
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 97 * D14 - 236 * D9 , DenShift32);
    E10 = xTrRound(  - 236 * D13 - 97 * D10 , DenShift32);
    E13 = xTrRound( 97 * D13 - 236 * D10 , DenShift32);
    E14 = xTrRound( 236 * D14 + 97 * D9 , DenShift32);
    E16 = C16 + D19;
    E19 = C16 - D19;
    E20 = C23 - D20;
    E23 = C23 + D20;
    E24 = C24 + D27;
    E27 = C24 - D27;
    E28 = C31 - D28;
    E31 = C31 + D28;
    E17 = C17 + D18;
    E18 = C17 - D18;
    E21 = C22 - D21;
    E22 = C22 + D21;
    E25 = C25 + D26;
    E26 = C25 - D26;
    E29 = C30 - D29;
    E30 = C30 + D29;
    
    aaiTemp[4][y] = xTrRound( 49 * E4 + 251 * E7 , DenShift32);
    aaiTemp[20][y] = xTrRound( 212 * E5 + 142 * E6 , DenShift32);
    aaiTemp[12][y] = xTrRound( 212 * E6 - 142 * E5 , DenShift32);
    aaiTemp[28][y] = xTrRound( 49 * E7 - 251 * E4 , DenShift32);
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    F17 = xTrRound( 49 * E30 - 251 * E17 , DenShift32);
    F18 = xTrRound(  - 251 * E29 - 49 * E18 , DenShift32);
    F21 = xTrRound( 212 * E26 - 142 * E21 , DenShift32);
    F22 = xTrRound(  - 142 * E25 - 212 * E22 , DenShift32);
    F25 = xTrRound( 212 * E25 - 142 * E22 , DenShift32);
    F26 = xTrRound( 142 * E26 + 212 * E21 , DenShift32);
    F29 = xTrRound( 49 * E29 - 251 * E18 , DenShift32);
    F30 = xTrRound( 251 * E30 + 49 * E17 , DenShift32);;
    
    aaiTemp[2][y] = xTrRound( 25 * F8 + 254 * F15 , DenShift32);
    aaiTemp[18][y] = xTrRound( 197 * F9 + 162 * F14 , DenShift32);
    aaiTemp[10][y] = xTrRound( 120 * F10 + 225 * F13 , DenShift32);
    aaiTemp[26][y] = xTrRound( 244 * F11 + 74 * F12 , DenShift32);
    aaiTemp[6][y] = xTrRound( 244 * F12 - 74 * F11 , DenShift32);
    aaiTemp[22][y] = xTrRound( 120 * F13 - 225 * F10 , DenShift32);
    aaiTemp[14][y] = xTrRound( 197 * F14 - 162 * F9 , DenShift32);
    aaiTemp[30][y] = xTrRound( 25 * F15 - 254 * F8 , DenShift32);
    G16 = E16 + F17;
    G17 = E16 - F17;
    G18 = E19 - F18;
    G19 = E19 + F18;
    G20 = E20 + F21;
    G21 = E20 - F21;
    G22 = E23 - F22;
    G23 = E23 + F22;
    G24 = E24 + F25;
    G25 = E24 - F25;
    G26 = E27 - F26;
    G27 = E27 + F26;
    G28 = E28 + F29;
    G29 = E28 - F29;
    G30 = E31 - F30;
    G31 = E31 + F30;
    
    aaiTemp[1][y] = xTrRound( 12 * G16 + 255 * G31 , DenShift32);
    aaiTemp[17][y] = xTrRound( 189 * G17 + 171 * G30 , DenShift32);
    aaiTemp[9][y] = xTrRound( 109 * G18 + 231 * G29 , DenShift32);
    aaiTemp[25][y] = xTrRound( 241 * G19 + 86 * G28 , DenShift32);
    aaiTemp[5][y] = xTrRound( 62 * G20 + 248 * G27 , DenShift32);
    aaiTemp[21][y] = xTrRound( 219 * G21 + 131 * G26 , DenShift32);
    aaiTemp[13][y] = xTrRound( 152 * G22 + 205 * G25 , DenShift32);
    aaiTemp[29][y] = xTrRound( 253 * G23 + 37 * G24 , DenShift32);
    aaiTemp[3][y] = xTrRound( 253 * G24 - 37 * G23 , DenShift32);
    aaiTemp[19][y] = xTrRound( 152 * G25 - 205 * G22 , DenShift32);
    aaiTemp[11][y] = xTrRound( 219 * G26 - 131 * G21 , DenShift32);
    aaiTemp[27][y] = xTrRound( 62 * G27 - 248 * G20 , DenShift32);
    aaiTemp[7][y] = xTrRound( 241 * G28 - 86 * G19 , DenShift32);
    aaiTemp[23][y] = xTrRound( 109 * G29 - 231 * G18 , DenShift32);
    aaiTemp[15][y] = xTrRound( 189 * G30 - 171 * G17 , DenShift32);
    aaiTemp[31][y] = xTrRound( 12 * G31 - 255 * G16 , DenShift32);
    
    pSrc += uiStride;
  }
  
  for( x=0 ; x<32 ; x++, pDes++ )
  {
    A0 = aaiTemp[x][0] + aaiTemp[x][31];
    A31 = aaiTemp[x][0] - aaiTemp[x][31];
    A1 = aaiTemp[x][1] + aaiTemp[x][30];
    A30 = aaiTemp[x][1] - aaiTemp[x][30];
    A2 = aaiTemp[x][2] + aaiTemp[x][29];
    A29 = aaiTemp[x][2] - aaiTemp[x][29];
    A3 = aaiTemp[x][3] + aaiTemp[x][28];
    A28 = aaiTemp[x][3] - aaiTemp[x][28];
    A4 = aaiTemp[x][4] + aaiTemp[x][27];
    A27 = aaiTemp[x][4] - aaiTemp[x][27];
    A5 = aaiTemp[x][5] + aaiTemp[x][26];
    A26 = aaiTemp[x][5] - aaiTemp[x][26];
    A6 = aaiTemp[x][6] + aaiTemp[x][25];
    A25 = aaiTemp[x][6] - aaiTemp[x][25];
    A7 = aaiTemp[x][7] + aaiTemp[x][24];
    A24 = aaiTemp[x][7] - aaiTemp[x][24];
    A8 = aaiTemp[x][8] + aaiTemp[x][23];
    A23 = aaiTemp[x][8] - aaiTemp[x][23];
    A9 = aaiTemp[x][9] + aaiTemp[x][22];
    A22 = aaiTemp[x][9] - aaiTemp[x][22];
    A10 = aaiTemp[x][10] + aaiTemp[x][21];
    A21 = aaiTemp[x][10] - aaiTemp[x][21];
    A11 = aaiTemp[x][11] + aaiTemp[x][20];
    A20 = aaiTemp[x][11] - aaiTemp[x][20];
    A12 = aaiTemp[x][12] + aaiTemp[x][19];
    A19 = aaiTemp[x][12] - aaiTemp[x][19];
    A13 = aaiTemp[x][13] + aaiTemp[x][18];
    A18 = aaiTemp[x][13] - aaiTemp[x][18];
    A14 = aaiTemp[x][14] + aaiTemp[x][17];
    A17 = aaiTemp[x][14] - aaiTemp[x][17];
    A15 = aaiTemp[x][15] + aaiTemp[x][16];
    A16 = aaiTemp[x][15] - aaiTemp[x][16];
    
    B0 = A0 + A15;
    B15 = A0 - A15;
    B1 = A1 + A14;
    B14 = A1 - A14;
    B2 = A2 + A13;
    B13 = A2 - A13;
    B3 = A3 + A12;
    B12 = A3 - A12;
    B4 = A4 + A11;
    B11 = A4 - A11;
    B5 = A5 + A10;
    B10 = A5 - A10;
    B6 = A6 + A9;
    B9 = A6 - A9;
    B7 = A7 + A8;
    B8 = A7 - A8;
    B20 = xTrRound( 181 * ( A27 - A20 ) , DenShift32);
    B27 = xTrRound( 181 * ( A27 + A20 ) , DenShift32);
    B21 = xTrRound( 181 * ( A26 - A21 ) , DenShift32);
    B26 = xTrRound( 181 * ( A26 + A21 ) , DenShift32);
    B22 = xTrRound( 181 * ( A25 - A22 ) , DenShift32);
    B25 = xTrRound( 181 * ( A25 + A22 ) , DenShift32);
    B23 = xTrRound( 181 * ( A24 - A23 ) , DenShift32);
    B24 = xTrRound( 181 * ( A24 + A23 ) , DenShift32);;
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 181 * ( B13 - B10 ) , DenShift32);
    C13 = xTrRound( 181 * ( B13 + B10 ) , DenShift32);
    C11 = xTrRound( 181 * ( B12 - B11 ) , DenShift32);
    C12 = xTrRound( 181 * ( B12 + B11 ) , DenShift32);
    C16 = A16 + B23;
    C23 = A16 - B23;
    C24 = A31 - B24;
    C31 = A31 + B24;
    C17 = A17 + B22;
    C22 = A17 - B22;
    C25 = A30 - B25;
    C30 = A30 + B25;
    C18 = A18 + B21;
    C21 = A18 - B21;
    C26 = A29 - B26;
    C29 = A29 + B26;
    C19 = A19 + B20;
    C20 = A19 - B20;
    C27 = A28 - B27;
    C28 = A28 + B27;
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 181 * ( C6 - C5 ) , DenShift32);
    D6 = xTrRound( 181 * ( C6 + C5 ) , DenShift32);
    D18 = xTrRound( 97 * C29 - 236 * C18 , DenShift32);
    D20 = xTrRound(  - 236 * C27 - 97 * C20 , DenShift32);
    D26 = xTrRound(  - 236 * C21 + 97 * C26 , DenShift32);
    D28 = xTrRound( 97 * C19 + 236 * C28 , DenShift32);
    D19 = xTrRound( 97 * C28 - 236 * C19 , DenShift32);
    D21 = xTrRound(  - 236 * C26 - 97 * C21 , DenShift32);
    D27 = xTrRound(  - 236 * C20 + 97 * C27 , DenShift32);
    D29 = xTrRound( 97 * C18 + 236 * C29 , DenShift32);;
    
    pDes[0] = xTrRound( 181 * ( D0 + D1 ) , DenShift32);
    pDes[512] = xTrRound( 181 * ( D0 - D1 ) , DenShift32);
    pDes[256] = xTrRound( 236 * D3 + 97 * D2 , DenShift32);
    pDes[768] = xTrRound( 97 * D3 - 236 * D2 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[0]   = (pDes[0]  +offset)>>uiBitDepthIncrease;
    pDes[512] = (pDes[512]+offset)>>uiBitDepthIncrease;
    pDes[256] = (pDes[256]+offset)>>uiBitDepthIncrease;
    pDes[768] = (pDes[768]+offset)>>uiBitDepthIncrease;
#endif
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 97 * D14 - 236 * D9 , DenShift32);
    E10 = xTrRound(  - 236 * D13 - 97 * D10 , DenShift32);
    E13 = xTrRound( 97 * D13 - 236 * D10 , DenShift32);
    E14 = xTrRound( 236 * D14 + 97 * D9 , DenShift32);
    E16 = C16 + D19;
    E19 = C16 - D19;
    E20 = C23 - D20;
    E23 = C23 + D20;
    E24 = C24 + D27;
    E27 = C24 - D27;
    E28 = C31 - D28;
    E31 = C31 + D28;
    E17 = C17 + D18;
    E18 = C17 - D18;
    E21 = C22 - D21;
    E22 = C22 + D21;
    E25 = C25 + D26;
    E26 = C25 - D26;
    E29 = C30 - D29;
    E30 = C30 + D29;
    
    pDes[128] = xTrRound( 49 * E4 + 251 * E7 , DenShift32);
    pDes[640] = xTrRound( 212 * E5 + 142 * E6 , DenShift32);
    pDes[384] = xTrRound( 212 * E6 - 142 * E5 , DenShift32);
    pDes[896] = xTrRound( 49 * E7 - 251 * E4 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[128] = (pDes[128]+offset)>>uiBitDepthIncrease;
    pDes[640] = (pDes[640]+offset)>>uiBitDepthIncrease;
    pDes[384] = (pDes[384]+offset)>>uiBitDepthIncrease;
    pDes[896] = (pDes[896]+offset)>>uiBitDepthIncrease;
#endif
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    F17 = xTrRound( 49 * E30 - 251 * E17 , DenShift32);
    F18 = xTrRound(  - 251 * E29 - 49 * E18 , DenShift32);
    F21 = xTrRound( 212 * E26 - 142 * E21 , DenShift32);
    F22 = xTrRound(  - 142 * E25 - 212 * E22 , DenShift32);
    F25 = xTrRound( 212 * E25 - 142 * E22 , DenShift32);
    F26 = xTrRound( 142 * E26 + 212 * E21 , DenShift32);
    F29 = xTrRound( 49 * E29 - 251 * E18 , DenShift32);
    F30 = xTrRound( 251 * E30 + 49 * E17 , DenShift32);;
    
    pDes[64] = xTrRound( 25 * F8 + 254 * F15 , DenShift32);
    pDes[576] = xTrRound( 197 * F9 + 162 * F14 , DenShift32);
    pDes[320] = xTrRound( 120 * F10 + 225 * F13 , DenShift32);
    pDes[832] = xTrRound( 244 * F11 + 74 * F12 , DenShift32);
    pDes[192] = xTrRound( 244 * F12 - 74 * F11 , DenShift32);
    pDes[704] = xTrRound( 120 * F13 - 225 * F10 , DenShift32);
    pDes[448] = xTrRound( 197 * F14 - 162 * F9 , DenShift32);
    pDes[960] = xTrRound( 25 * F15 - 254 * F8 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[64]  = (pDes[64] +offset)>>uiBitDepthIncrease;
    pDes[576] = (pDes[576]+offset)>>uiBitDepthIncrease;
    pDes[320] = (pDes[320]+offset)>>uiBitDepthIncrease;
    pDes[832] = (pDes[832]+offset)>>uiBitDepthIncrease;
    pDes[192] = (pDes[192]+offset)>>uiBitDepthIncrease;
    pDes[704] = (pDes[704]+offset)>>uiBitDepthIncrease;
    pDes[448] = (pDes[448]+offset)>>uiBitDepthIncrease;
    pDes[960] = (pDes[960]+offset)>>uiBitDepthIncrease;
#endif
    G16 = E16 + F17;
    G17 = E16 - F17;
    G18 = E19 - F18;
    G19 = E19 + F18;
    G20 = E20 + F21;
    G21 = E20 - F21;
    G22 = E23 - F22;
    G23 = E23 + F22;
    G24 = E24 + F25;
    G25 = E24 - F25;
    G26 = E27 - F26;
    G27 = E27 + F26;
    G28 = E28 + F29;
    G29 = E28 - F29;
    G30 = E31 - F30;
    G31 = E31 + F30;
    
    pDes[32] = xTrRound( 12 * G16 + 255 * G31 , DenShift32);
    pDes[544] = xTrRound( 189 * G17 + 171 * G30 , DenShift32);
    pDes[288] = xTrRound( 109 * G18 + 231 * G29 , DenShift32);
    pDes[800] = xTrRound( 241 * G19 + 86 * G28 , DenShift32);
    pDes[160] = xTrRound( 62 * G20 + 248 * G27 , DenShift32);
    pDes[672] = xTrRound( 219 * G21 + 131 * G26 , DenShift32);
    pDes[416] = xTrRound( 152 * G22 + 205 * G25 , DenShift32);
    pDes[928] = xTrRound( 253 * G23 + 37 * G24 , DenShift32);
    pDes[96] = xTrRound( 253 * G24 - 37 * G23 , DenShift32);
    pDes[608] = xTrRound( 152 * G25 - 205 * G22 , DenShift32);
    pDes[352] = xTrRound( 219 * G26 - 131 * G21 , DenShift32);
    pDes[864] = xTrRound( 62 * G27 - 248 * G20 , DenShift32);
    pDes[224] = xTrRound( 241 * G28 - 86 * G19 , DenShift32);
    pDes[736] = xTrRound( 109 * G29 - 231 * G18 , DenShift32);
    pDes[480] = xTrRound( 189 * G30 - 171 * G17 , DenShift32);
    pDes[992] = xTrRound( 12 * G31 - 255 * G16 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[32]  = (pDes[32] +offset)>>uiBitDepthIncrease;
    pDes[544] = (pDes[544]+offset)>>uiBitDepthIncrease;
    pDes[288] = (pDes[288]+offset)>>uiBitDepthIncrease;
    pDes[800] = (pDes[800]+offset)>>uiBitDepthIncrease;
    pDes[160] = (pDes[160]+offset)>>uiBitDepthIncrease;
    pDes[672] = (pDes[672]+offset)>>uiBitDepthIncrease;
    pDes[416] = (pDes[416]+offset)>>uiBitDepthIncrease;
    pDes[928] = (pDes[928]+offset)>>uiBitDepthIncrease;
    pDes[96]  = (pDes[96] +offset)>>uiBitDepthIncrease;
    pDes[608] = (pDes[608]+offset)>>uiBitDepthIncrease;
    pDes[352] = (pDes[352]+offset)>>uiBitDepthIncrease;
    pDes[864] = (pDes[864]+offset)>>uiBitDepthIncrease;
    pDes[224] = (pDes[224]+offset)>>uiBitDepthIncrease;
    pDes[736] = (pDes[736]+offset)>>uiBitDepthIncrease;
    pDes[480] = (pDes[480]+offset)>>uiBitDepthIncrease;
    pDes[992] = (pDes[992]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xT16( Pel* pSrc, UInt uiStride, Long* pDes )
{
  Int x, y;
  
  Long aaiTemp[16][16];
  Long B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15;
  Long C0, C1, C2, C3, C4, C5, C6, C7, C10, C11, C12, C13;
  Long D0, D1, D2, D3, D5, D6, D8, D9, D10, D11, D12, D13, D14, D15;
  Long E4, E5, E6, E7, E9, E10, E13, E14;
  Long F8, F9, F10, F11, F12, F13, F14, F15;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift16x16-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  
  //--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    B0  = (pSrc[0] + pSrc[15])<<uiBitDepthIncrease;
    B15 = (pSrc[0] - pSrc[15])<<uiBitDepthIncrease;
    B1  = (pSrc[1] + pSrc[14])<<uiBitDepthIncrease;
    B14 = (pSrc[1] - pSrc[14])<<uiBitDepthIncrease;
    B2  = (pSrc[2] + pSrc[13])<<uiBitDepthIncrease;
    B13 = (pSrc[2] - pSrc[13])<<uiBitDepthIncrease;
    B3  = (pSrc[3] + pSrc[12])<<uiBitDepthIncrease;
    B12 = (pSrc[3] - pSrc[12])<<uiBitDepthIncrease;
    B4  = (pSrc[4] + pSrc[11])<<uiBitDepthIncrease;
    B11 = (pSrc[4] - pSrc[11])<<uiBitDepthIncrease;
    B5  = (pSrc[5] + pSrc[10])<<uiBitDepthIncrease;
    B10 = (pSrc[5] - pSrc[10])<<uiBitDepthIncrease;
    B6  = (pSrc[6] + pSrc[9])<<uiBitDepthIncrease;
    B9  = (pSrc[6] - pSrc[9])<<uiBitDepthIncrease;
    B7  = (pSrc[7] + pSrc[8])<<uiBitDepthIncrease;
    B8  = (pSrc[7] - pSrc[8])<<uiBitDepthIncrease;
#else
    B0 = pSrc[0] + pSrc[15];
    B15 = pSrc[0] - pSrc[15];
    B1 = pSrc[1] + pSrc[14];
    B14 = pSrc[1] - pSrc[14];
    B2 = pSrc[2] + pSrc[13];
    B13 = pSrc[2] - pSrc[13];
    B3 = pSrc[3] + pSrc[12];
    B12 = pSrc[3] - pSrc[12];
    B4 = pSrc[4] + pSrc[11];
    B11 = pSrc[4] - pSrc[11];
    B5 = pSrc[5] + pSrc[10];
    B10 = pSrc[5] - pSrc[10];
    B6 = pSrc[6] + pSrc[9];
    B9 = pSrc[6] - pSrc[9];
    B7 = pSrc[7] + pSrc[8];
    B8 = pSrc[7] - pSrc[8];
#endif
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 45 * ( B13 - B10 ) , DenShift16);
    C13 = xTrRound( 45 * ( B13 + B10 ) , DenShift16);
    C11 = xTrRound( 45 * ( B12 - B11 ) , DenShift16);
    C12 = xTrRound( 45 * ( B12 + B11 ) , DenShift16);
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 45 * ( C6 - C5 ) , DenShift16);
    D6 = xTrRound( 45 * ( C6 + C5 ) , DenShift16);
    
    aaiTemp[0][y] = xTrRound( 45 * ( D0 + D1 ) , DenShift16);
    aaiTemp[8][y] = xTrRound( 45 * ( D0 - D1 ) , DenShift16);
    aaiTemp[4][y] = xTrRound( 59 * D3 + 24 * D2 , DenShift16);
    aaiTemp[12][y] = xTrRound( 24 * D3 - 59 * D2 , DenShift16);
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 24 * D14 - 59 * D9 , DenShift16);
    E10 = xTrRound(  -59 * D13 - 24 * D10 , DenShift16);
    E13 = xTrRound( 24 * D13 - 59 * D10 , DenShift16);
    E14 = xTrRound( 59 * D14 + 24 * D9 , DenShift16);
    
    aaiTemp[2][y] = xTrRound( 12 * E4 + 62 * E7 , DenShift16);
    aaiTemp[10][y] = xTrRound( 53 * E5 + 35 * E6 , DenShift16);
    aaiTemp[6][y] = xTrRound( 53 * E6 - 35 * E5 , DenShift16);
    aaiTemp[14][y] = xTrRound( 12 * E7 - 62 * E4 , DenShift16);
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    
    aaiTemp[1][y] = xTrRound( 6 * F8 + 63 * F15 , DenShift16);
    aaiTemp[9][y] = xTrRound( 49 * F9 + 40 * F14 , DenShift16);
    aaiTemp[5][y] = xTrRound( 30 * F10 + 56 * F13 , DenShift16);
    aaiTemp[13][y] = xTrRound( 61 * F11 + 18 * F12 , DenShift16);
    aaiTemp[3][y] = xTrRound( 61 * F12 - 18 * F11 , DenShift16);
    aaiTemp[11][y] = xTrRound( 30 * F13 - 56 * F10 , DenShift16);
    aaiTemp[7][y] = xTrRound( 49 * F14 - 40 * F9 , DenShift16);
    aaiTemp[15][y] = xTrRound( 6 * F15 - 63 * F8 , DenShift16);
    
    pSrc += uiStride;
  }
  
  for( x=0 ; x<16 ; x++, pDes++ )
  {
    B0 = aaiTemp[x][0] + aaiTemp[x][15];
    B15 = aaiTemp[x][0] - aaiTemp[x][15];
    B1 = aaiTemp[x][1] + aaiTemp[x][14];
    B14 = aaiTemp[x][1] - aaiTemp[x][14];
    B2 = aaiTemp[x][2] + aaiTemp[x][13];
    B13 = aaiTemp[x][2] - aaiTemp[x][13];
    B3 = aaiTemp[x][3] + aaiTemp[x][12];
    B12 = aaiTemp[x][3] - aaiTemp[x][12];
    B4 = aaiTemp[x][4] + aaiTemp[x][11];
    B11 = aaiTemp[x][4] - aaiTemp[x][11];
    B5 = aaiTemp[x][5] + aaiTemp[x][10];
    B10 = aaiTemp[x][5] - aaiTemp[x][10];
    B6 = aaiTemp[x][6] + aaiTemp[x][9];
    B9 = aaiTemp[x][6] - aaiTemp[x][9];
    B7 = aaiTemp[x][7] + aaiTemp[x][8];
    B8 = aaiTemp[x][7] - aaiTemp[x][8];
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 45 * ( B13 - B10 ) , DenShift16);
    C13 = xTrRound( 45 * ( B13 + B10 ) , DenShift16);
    C11 = xTrRound( 45 * ( B12 - B11 ) , DenShift16);
    C12 = xTrRound( 45 * ( B12 + B11 ) , DenShift16);
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 45 * ( C6 - C5 ) , DenShift16);
    D6 = xTrRound( 45 * ( C6 + C5 ) , DenShift16);
    
    pDes[0] = xTrRound( 45 * ( D0 + D1 ) , DenShift16);
    pDes[128] = xTrRound( 45 * ( D0 - D1 ) , DenShift16);
    pDes[64] = xTrRound( 59 * D3 + 24 * D2 , DenShift16);
    pDes[192] = xTrRound( 24 * D3 - 59 * D2 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[0  ] = (pDes[0  ]+offset)>>uiBitDepthIncrease;
    pDes[128] = (pDes[128]+offset)>>uiBitDepthIncrease;
    pDes[64 ] = (pDes[64 ]+offset)>>uiBitDepthIncrease;
    pDes[192] = (pDes[192]+offset)>>uiBitDepthIncrease;
#endif
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 24 * D14 - 59 * D9 , DenShift16);
    E10 = xTrRound(  -59 * D13 - 24 * D10 , DenShift16);
    E13 = xTrRound( 24 * D13 - 59 * D10 , DenShift16);
    E14 = xTrRound( 59 * D14 + 24 * D9 , DenShift16);
    
    pDes[32] = xTrRound( 12 * E4 + 62 * E7 , DenShift16);
    pDes[160] = xTrRound( 53 * E5 + 35 * E6 , DenShift16);
    pDes[96] = xTrRound( 53 * E6 - 35 * E5 , DenShift16);
    pDes[224] = xTrRound( 12 * E7 - 62 * E4 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[32]  = (pDes[32] +offset)>>uiBitDepthIncrease;
    pDes[160] = (pDes[160]+offset)>>uiBitDepthIncrease;
    pDes[96]  = (pDes[96] +offset)>>uiBitDepthIncrease;
    pDes[224] = (pDes[224]+offset)>>uiBitDepthIncrease;
#endif
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    
    pDes[16] = xTrRound( 6 * F8 + 63 * F15 , DenShift16);
    pDes[144] = xTrRound( 49 * F9 + 40 * F14 , DenShift16);
    pDes[80] = xTrRound( 30 * F10 + 56 * F13 , DenShift16);
    pDes[208] = xTrRound( 61 * F11 + 18 * F12 , DenShift16);
    pDes[48] = xTrRound( 61 * F12 - 18 * F11 , DenShift16);
    pDes[176] = xTrRound( 30 * F13 - 56 * F10 , DenShift16);
    pDes[112] = xTrRound( 49 * F14 - 40 * F9 , DenShift16);
    pDes[240] = xTrRound( 6 * F15 - 63 * F8 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[16]  = (pDes[16] +offset)>>uiBitDepthIncrease;
    pDes[144] = (pDes[144]+offset)>>uiBitDepthIncrease;
    pDes[80]  = (pDes[80] +offset)>>uiBitDepthIncrease;
    pDes[208] = (pDes[208]+offset)>>uiBitDepthIncrease;
    pDes[48]  = (pDes[48] +offset)>>uiBitDepthIncrease;
    pDes[176] = (pDes[176]+offset)>>uiBitDepthIncrease;
    pDes[112] = (pDes[112]+offset)>>uiBitDepthIncrease;
    pDes[240] = (pDes[240]+offset)>>uiBitDepthIncrease;
#endif
  }
}
#endif //E243_CORE_TRANSFORMS

#if QC_MOD_LCEC_RDOQ
UInt TComTrQuant::xCountVlcBits(UInt uiTableNumber, UInt uiCodeNumber)
{
  UInt uiLength = 0;
  UInt uiCode = 0;

  if ( uiCodeNumber < 128 )
  {
    uiLength=VLClength[uiTableNumber][uiCodeNumber];
  }
  else
  {
    if ( uiTableNumber < 10 )
    {
      if ( uiTableNumber < 5 )
      {
        uiCode = uiCodeNumber - (6 * (1 << uiTableNumber)) + (1 << uiTableNumber);
        uiLength = ( 6 - uiTableNumber ) + 1 + 2 * xLeadingZeros(uiCode);
      }
      else if ( uiTableNumber < 8 )
      {
        uiLength = 1 + (uiTableNumber - 4) + (uiCodeNumber >> (uiTableNumber - 4));
      }
      else if ( uiTableNumber == 9 )
      {
        uiLength = 5 + ((uiCodeNumber + 5) >> 4);
      }
    }
    else
    {
      if ( uiTableNumber == 10 )
      {
        uiCode = uiCodeNumber + 1;
        uiLength = 1 + 2 * xLeadingZeros(uiCode);
      }
#if CAVLC_COEF_LRG_BLK
      else if (uiTableNumber == 12)
      {
        uiLength = 7+(uiCodeNumber>>6);
      }
      else if(uiTableNumber == 13)
      {
        uiLength = 5+(uiCodeNumber>>4);
      }
#endif
    }
  }
  return uiLength;
}


#if CAVLC_COEF_LRG_BLK
Int TComTrQuant::bitCountRDOQ(Int coeff, Int pos, Int nTab, Int lastCoeffFlag,Int levelMode,Int run, Int maxrun, Int vlc_adaptive, Int N, 
                              UInt uiTr1, Int iSum_big_coef, Int iBlockType, TComDataCU* pcCU, const UInt **pLumaRunTr1)
#else
Int TComTrQuant::bitCountRDOQ(Int coeff, Int pos, Int nTab, Int lastCoeffFlag,Int levelMode,Int run, Int maxrun, Int vlc_adaptive, Int N, 
                              UInt uiTr1, Int iSum_big_coef, Int iBlockType, TComDataCU* pcCU)
#endif
{
  UInt cn, n, level, lev;
  Int vlc,x,cx,vlcNum,bits;
  static const int vlcTable4[3] = {2,2,2};             // Y4x4I,Y4x4P,Y4x4B,

  Int sign = coeff < 0 ? 1 : 0;

  if ( N==4 )
  {
    n = Max(0, nTab - 2);
  }
  else
  {
    n = nTab;
  }

  UInt uiModZeroCoding = 0;
  const UInt *uiVlcTableTemp;

#if CAVLC_COEF_LRG_BLK
  uiModZeroCoding = (m_uiRDOQOffset==1 || N>8)? 1:0;
  int tmprun = Min(maxrun,28);

  if( N<=8 )
  {
    uiVlcTableTemp = (nTab==2 || nTab==5)? g_auiVlcTable8x8Intra:g_auiVlcTable8x8Inter;
  }
  else
  {
    uiVlcTableTemp = (nTab==5)? g_auiVlcTable16x16Intra:g_auiVlcTable16x16Inter;
  }
#else
  if( nTab == 2 || nTab == 5 ){
    uiVlcTableTemp = g_auiVlcTable8x8Intra;
  }
  else
  {
    uiVlcTableTemp = g_auiVlcTable8x8Inter;
  }
#endif

  level = abs(coeff);
  lev = (level == 1) ? 0 : 1;

  if ( level )
  {
    if ( lastCoeffFlag == 1 )
    {     
      x = pos + (level == 1 ? 0 : N * N);
      if( N == 4 )
      {
        cx = m_uiLPTableE4[(n << 5) + x];
        vlcNum = vlcTable4[n];
      }
      else{
#if CAVLC_COEF_LRG_BLK
        cx = xLastLevelInd(lev, pos, N);
#else
        cx = m_uiLPTableE8[(n << 7) + x];
#endif
        vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
      }
      bits=xCountVlcBits( vlcNum, cx );

      if ( level > 1 )
      {
        bits += xCountVlcBits( 0, 2 * (level - 2) + sign );
      }
      else
      {
        bits++;
      }

    }
    else{ // Level !=0  && lastCoeffFlag==0

      if ( !levelMode ){   
#if CAVLC_COEF_LRG_BLK
          if(nTab == 2 || nTab == 5)
          {
            cn = xRunLevelInd(lev, run, maxrun, pLumaRunTr1[uiTr1][tmprun]);
          }
          else
          {
            cn = xRunLevelIndInter(lev, run, maxrun);
          }
          vlc = uiVlcTableTemp[tmprun];
#else
        if ( N == 4 ){
          // 4x4
          if ( nTab == 2 ){
            cn = xRunLevelInd(lev, run, maxrun, g_auiLumaRunTr14x4[uiTr1][maxrun]);
          }
          else{
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(lev, run, maxrun);
#else
            cn = g_auiLumaRun8x8[maxrun][lev][run];
#endif
          }
          vlc = uiVlcTableTemp[maxrun];
        }
        else {
          // 8x8
          if(nTab == 2 || nTab == 5)
          {
            cn = xRunLevelInd(lev, run, maxrun, g_auiLumaRunTr18x8[uiTr1][Min(maxrun,28)]);
          }
          else
          {
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(lev, run, maxrun);
#else
            cn = g_auiLumaRun8x8[Min(maxrun,28)][lev][run];
#endif
          }
          vlc = uiVlcTableTemp[Min(maxrun,28)];
        }
#endif
        bits = xCountVlcBits( vlc, cn );
        if ( level > 1 ){
          bits += xCountVlcBits( 0, 2 * (level - 2) + sign );
        }
        else{
          bits++;
        } 

      }
      else{ // Level !=0  && lastCoeffFlag==0 && levelMode
        bits = (xCountVlcBits( vlc_adaptive, level ) + 1);
      }
    }
  }
  else{

    if (levelMode){
      bits=xCountVlcBits( vlc_adaptive, level );
    }
    else{                        
      if ( pos == 0 && lastCoeffFlag == 0){  

#if CAVLC_COEF_LRG_BLK
        vlc = uiVlcTableTemp[tmprun];
        if(nTab == 2 || nTab == 5)
        {
          cn = xRunLevelInd(0, run + 1, maxrun, pLumaRunTr1[uiTr1][tmprun]);
        }
        else
        {
          cn = xRunLevelIndInter(0, run + 1, maxrun);
        }
#else
        if ( N == 4 ){
          // 4x4
          vlc = uiVlcTableTemp[maxrun];
          if ( nTab == 2 ){
            cn = xRunLevelInd(0, run + 1, maxrun, g_auiLumaRunTr14x4[uiTr1][maxrun]);
          }
          else{
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(0, run+1, maxrun);
#else
            cn = g_auiLumaRun8x8[maxrun][0][run + 1];
#endif
          }
        }
        else{
          // 8x8
          vlc = uiVlcTableTemp[Min(maxrun, 28)];
          if(nTab == 2 || nTab == 5){
            cn = xRunLevelInd(0, run + 1, maxrun, g_auiLumaRunTr18x8[uiTr1][Min(maxrun, 28)]);
          }
          else{
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(0, run+1, maxrun);
#else
            cn = g_auiLumaRun8x8[Min(maxrun, 28)][0][run + 1];
#endif
          }
        }
#endif
        bits=xCountVlcBits( vlc, cn );
      }
      else{
        bits = 0;

        if ( pos > 0 && uiModZeroCoding == 1 ){

          Int iSum_big_coefTemp, levelModeTemp, maxrunTemp;
          UInt uiTr1Temp;

          if ( lastCoeffFlag == 0 ){

#if CAVLC_COEF_LRG_BLK
            vlc = uiVlcTableTemp[tmprun];
            if(nTab == 2 || nTab == 5)
            {
              cn = xRunLevelInd(0, run + 1, maxrun, pLumaRunTr1[uiTr1][tmprun]);
            }
            else
            {
              cn = xRunLevelIndInter(0, run + 1, maxrun);
            }
#else
            if ( N == 4 ){
              // 4x4
              vlc = uiVlcTableTemp[maxrun];
              if ( nTab == 2 ){
                cn = xRunLevelInd(0, run + 1, maxrun, g_auiLumaRunTr14x4[uiTr1][maxrun]);
              }
              else{
#if RUNLEVEL_TABLE_CUT
                cn = xRunLevelIndInter(0, run+1, maxrun);
#else
                cn = g_auiLumaRun8x8[maxrun][0][run + 1];
#endif
              }
            }
            else{
              // 8x8
              vlc = uiVlcTableTemp[Min(maxrun, 28)];
              if(nTab == 2 || nTab == 5){
                cn = xRunLevelInd(0, run + 1, maxrun, g_auiLumaRunTr18x8[uiTr1][Min(maxrun, 28)]);
              }
              else{
#if RUNLEVEL_TABLE_CUT
                cn = xRunLevelIndInter(0, run+1, maxrun);
#else
                cn = g_auiLumaRun8x8[Min(maxrun,28)][0][run + 1];
#endif
              }
            }
#endif
          }
          else{

            x = (pos - 1);
            if( N == 4 )
            {
              cn = m_uiLPTableE4[(n << 5) + x];
              vlc = vlcTable4[n];
            }
            else{
#if CAVLC_COEF_LRG_BLK
              cn = xLastLevelInd(lev, pos, N);
#else
              cn = m_uiLPTableE8[(n << 7) + x];
#endif
              vlc = g_auiLastPosVlcNum[n][Min(16, m_uiLastPosVlcIndex[n])];
            }
          }
          bits+=xCountVlcBits( vlc, cn );

          // Next coeff is 1 with run=0

          iSum_big_coefTemp = iSum_big_coef;
          levelModeTemp = levelMode;
          Int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};

          if ( N > 4 ){ 
            if ( level > 1 ){
              iSum_big_coefTemp += level;
              if ((N * N - pos - 1) > switch_thr[iBlockType] || iSum_big_coefTemp > 2) levelModeTemp = 1;
            }
          }
          else{
            if ( level > 1 ) levelModeTemp = 1;
          }

          if ( levelModeTemp == 1 ){
            bits-=xCountVlcBits( vlc_adaptive, 1);
          }
          else{
            maxrunTemp = pos - 1;
            uiTr1Temp = uiTr1;

            if ( uiTr1Temp > 0 && level < 2 ){
              uiTr1Temp++;
              uiTr1Temp = Min(MAX_TR1, uiTr1Temp);
            }
            else{
              uiTr1Temp=0;
            }

#if CAVLC_COEF_LRG_BLK
            vlc = uiVlcTableTemp[Min(maxrunTemp,28)];
            if(nTab == 2 || nTab == 5)
            {
              cn = xRunLevelInd(0, 0, maxrunTemp, pLumaRunTr1[uiTr1Temp][Min(maxrunTemp,28)]);
            }
            else
            {
              cn = xRunLevelIndInter(0, 0, maxrunTemp);
            }
#else
            if ( N == 4 ){
              // 4x4
              vlc = uiVlcTableTemp[maxrunTemp];
              if ( nTab == 2 ){
                cn = xRunLevelInd(0, 0, maxrunTemp, g_auiLumaRunTr14x4[uiTr1Temp][maxrunTemp]);
              }
              else{
#if RUNLEVEL_TABLE_CUT
                cn = xRunLevelIndInter(0, 0, maxrunTemp);
#else
                cn = g_auiLumaRun8x8[maxrunTemp][0][0];
#endif
              }
            }
            else{
              // 8x8
              vlc = uiVlcTableTemp[Min(maxrunTemp,28)];
              if(nTab == 2 || nTab == 5){
                cn = xRunLevelInd(0, 0, maxrunTemp, g_auiLumaRunTr18x8[uiTr1Temp][Min(maxrunTemp,28)]);
              }
              else{
#if RUNLEVEL_TABLE_CUT
                cn = xRunLevelIndInter(0, 0, maxrunTemp);
#else
                cn = g_auiLumaRun8x8[Min(maxrunTemp,28)][0][0];
#endif
              }
            }
#endif
            bits -= xCountVlcBits( vlc, cn );
          }
        } // if ( pos > 0 && uiModZeroCoding == 1 ){

      } 
    }
  }
  return bits;
}

Int TComTrQuant::xCodeCoeffCountBitsLast(TCoeff* scoeff, levelDataStruct* levelData, Int nTab, UInt N)
{
  Int i, prevCoeffInd, lastPosMin, iRate;
  Int done,last_pos;
#if CAVLC_COEF_LRG_BLK
  Int run_done, maxrun,run, bitsLast[1024], bitsRun[1024], bitsLastPrev;
  quantLevelStruct quantCoeffInfo[1024];
#else
  Int run_done, maxrun,run, bitsLast[256], bitsRun[256], bitsLastPrev;
  quantLevelStruct quantCoeffInfo[256];
#endif
  UInt last_pos_init, bitsLevel, sign, lev, cn, vlc, uiBitShift=15, uiNoCoeff=N*N, absLevel;
  Int n;
  double lagrMin, lagr, lagrPrev;
  UInt uiLumaRunNoTr14x4[15]={2, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2};
  UInt uiLumaRunNoTr18x8[29]={2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 9, 13};


  if ( N == 4 ){
    n = Max(0, nTab - 2);
  }
  else
  {
    n = nTab;
  }

  /* Do the last coefficient first */
  i = uiNoCoeff - 1;
  done = 0;

  while ( !done && i >= 0 )
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i--;
    }
  }
  if (i == -1)
  {
    return(-1);
  }

  last_pos = last_pos_init = i;
  prevCoeffInd = i;

  i--;

  if ( i >= 0 ){

    /* Go into run mode */
    run_done = 0;
    while ( !run_done )
    {
      maxrun = i;

      run = 0;
      done = 0;
      while ( !done )
      {
        if ( !scoeff[i] )
        {
          run++;
        }
        else
        {
          quantCoeffInfo[prevCoeffInd].run=run;
          quantCoeffInfo[prevCoeffInd].maxrun=maxrun;
          quantCoeffInfo[prevCoeffInd].nextLev=(abs(scoeff[i]) == 1) ? 0 : 1;
          quantCoeffInfo[prevCoeffInd].nexLevelVal=scoeff[i];

          prevCoeffInd = i;

          run = 0;
          done = 1;
        }
        if (i == 0)
        {
          quantCoeffInfo[prevCoeffInd].run=run;
          quantCoeffInfo[prevCoeffInd].maxrun=maxrun;
          quantCoeffInfo[prevCoeffInd].nextLev=0;
          quantCoeffInfo[prevCoeffInd].nexLevelVal=0;

          done = 1;
          run_done = 1;
        }
        i--;
      }
    }
  }

#if CAVLC_COEF_LRG_BLK
  const UInt *vlcTableIntra = (N<=8)? g_auiVlcTable8x8Intra:g_auiVlcTable16x16Intra;
  const UInt *vlcTableInter = (N<=8)? g_auiVlcTable8x8Inter:g_auiVlcTable16x16Inter;
  const UInt *pLumaRunTr1 = (N==4)? uiLumaRunNoTr14x4:uiLumaRunNoTr18x8;
#endif
  for (i = last_pos_init; i >= 0; i--){

    if (scoeff[i]){

      bitsLast[i] = bitsRun[i] = 0; 

      last_pos = i;
      {
        int x,cx,vlcNum;
        int vlcTable[3] = {2,2,2};

        bitsLevel=0;
        absLevel = abs(scoeff[i]);
        sign = (scoeff[i] < 0) ? 1 : 0;
        lev = (absLevel == 1) ? 0 : 1;

        if (absLevel > 1)
        {
          bitsLevel=xCountVlcBits( 0, 2*(absLevel-2)+sign );
        }
        else
        {
          bitsLevel++;
        }

        x = uiNoCoeff*lev + last_pos;

        if ( uiNoCoeff == 16 ){
          cx = m_uiLPTableE4[n * 32 + x];
          vlcNum = vlcTable[n];
        }
        else {
#if CAVLC_COEF_LRG_BLK
          cx = xLastLevelInd(lev, last_pos, N);
#else
          cx = m_uiLPTableE8[n * 128 + x];
#endif
          vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
        }
        bitsLast[i]=bitsLevel + xCountVlcBits( vlcNum, cx);
      }

      bitsRun[i]=0;

      if ( i > 0 )
      {
        bitsLevel = 0;
        if ( quantCoeffInfo[i].nexLevelVal != 0 ){
          absLevel = abs(quantCoeffInfo[i].nexLevelVal);
          sign = (quantCoeffInfo[i].nexLevelVal < 0) ? 1 : 0;
          lev = (absLevel == 1) ? 0 : 1;

          if (absLevel > 1)
          {
            bitsLevel = xCountVlcBits( 0, 2 * (absLevel - 2) + sign );
          }
          else
          {
            bitsLevel++;
          }
        }

        bitsRun[i] = bitsLevel;
        run = quantCoeffInfo[i].run;
        maxrun = quantCoeffInfo[i].maxrun;

#if CAVLC_COEF_LRG_BLK 
        Int tmprun = Min(maxrun,28);
        if(nTab == 2 || nTab == 5)
        {
          vlc = vlcTableIntra[tmprun]; 
          cn = xRunLevelInd(quantCoeffInfo[i].nextLev, run, maxrun, pLumaRunTr1[tmprun]);
        }
        else
        {
          vlc = vlcTableInter[tmprun]; 
          cn = xRunLevelIndInter(quantCoeffInfo[i].nextLev, run, maxrun);
        }
        bitsRun[i] += xCountVlcBits( vlc, cn );
#else
        if ( uiNoCoeff == 16 )
        {
          if ( nTab == 2 ){
            vlc = g_auiVlcTable8x8Intra[maxrun];        
            cn = xRunLevelInd(quantCoeffInfo[i].nextLev, run, maxrun, uiLumaRunNoTr14x4[maxrun]);
          }
          else{
            vlc = g_auiVlcTable8x8Inter[maxrun];           
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(quantCoeffInfo[i].nextLev, run, maxrun);
#else
            cn = g_auiLumaRun8x8[maxrun][quantCoeffInfo[i].nextLev][run];
#endif
          }
          bitsRun[i] += xCountVlcBits( vlc, cn );
        }
        else
        {
          if(nTab == 2 || nTab == 5){
            vlc = g_auiVlcTable8x8Intra[Min(maxrun,28)];          
            cn = xRunLevelInd(quantCoeffInfo[i].nextLev, run, maxrun, uiLumaRunNoTr18x8[min(maxrun,28)]);
          }
          else{
            vlc = g_auiVlcTable8x8Inter[Min(maxrun,28)];           
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(quantCoeffInfo[i].nextLev, run, maxrun); 
#else
            cn = g_auiLumaRun8x8[min(maxrun,28)][quantCoeffInfo[i].nextLev][run];
#endif
          }
          bitsRun[i] += xCountVlcBits( vlc, cn );
        }
#endif
      }
    }
  }

  lagrMin=0; lastPosMin=-1; 
  for (i=0; i<uiNoCoeff; i++){
    if ( scoeff[i] != 0 ){
      lagrMin += levelData[i].errLevel[0];
    }
  }

  UInt first=1; 

  bitsLastPrev=0; lagrPrev=lagrMin;
  for (i=0; i<uiNoCoeff; i++){
    if (scoeff[i]){
      iRate = (bitsRun[i] + bitsLast[i] - bitsLastPrev) << uiBitShift;
      lagr = lagrPrev-levelData[i].errLevel[0] + levelData[i].errLevel[levelData[i].quantInd] + m_dLambda*iRate;
      bitsLastPrev = bitsLast[i];
      lagrPrev = lagr;

      if ( lagr < lagrMin || abs(scoeff[i]) > 1 || first == 1){
        lagrMin = lagr;
        lastPosMin =i;
        first = 0;
      }
    }
  }

  return(lastPosMin);
}    
#else
#if QC_MOD_LCEC
Int TComTrQuant::bitCount_LCEC(Int k,Int pos,Int nTab, Int lpflag,Int levelMode,Int run, Int maxrun, Int vlc_adaptive, Int N, UInt uiTr1)
#else
Int TComTrQuant::bitCount_LCEC(Int k,Int pos,Int n,Int lpflag,Int levelMode,Int run,Int maxrun,Int vlc_adaptive,Int N)
#endif
{
  UInt cn;
  int vlc,x,cx,vlcNum,bits,temp;
#if QC_MOD_LCEC == 0
  static const int vlctable_8x8[28] = {8,0,0,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
  static const int vlcTable8[8] = {10,10,3,3,3,4,4,4}; // U,V,Y8x8I,Y8x8P,Y8x8B,Y16x16I,Y16x16P,Y16x16B
#endif
  static const int vlcTable4[3] = {2,2,2};             // Y4x4I,Y4x4P,Y4x4B,
#if QC_MOD_LCEC == 0
  static const int VLClength[11][128] =
  {
    { 1, 2, 3, 4, 5, 6, 7, 9, 9,11,11,11,11,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,10,10,10,10,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16},
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,63,64,64,65,65},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,24,24,24,24,25,25,25,25,26,26,26,26,27,27,27,27,28,28,28,28,29,29,29,29,30,30,30,30,31,31,31,31,32,32,32,32,33,33,33,33,34,34,34,34},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19},
    { 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13},
    { 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15}
  };
#endif
  int sign = k<0 ? 1 : 0;
  
  k = abs(k);
#if QC_MOD_LCEC
  Int n;
  Int lev = (k>1 ? 1 : 0);
  if (N==4)
    n=max(0,nTab-2);
  else
    n=nTab;
#endif
  if (N != 4 && N!= 8)
  {
    FATAL_ERROR_0("unsupported block size in bitCount_LCEC()" , -1 );
  }
  if (k)
  {
    if (lpflag==1)
    {                       
      x = pos + (k==1 ? 0 : N*N);
      if (N==8)
      {
        cx = m_uiLPTableE8[n*128+x];
#if QC_MOD_LCEC
        vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
#else
        vlcNum = vlcTable8[n];
#endif
      }
      else // (N==4)
      {
        cx = m_uiLPTableE4[n*32+x];
        vlcNum = vlcTable4[n];
      }
      bits = VLClength[vlcNum][cx];
      if (k>1)
      {
        temp = 2*(k-2)+sign;
        if(temp > 127)
          temp = 127;
        bits += VLClength[0][temp];
      }
      else
        bits += 1;                                     
    }
    else
    {
      if (!levelMode)
      {                                                    
#if QC_MOD_LCEC
        const UInt *p_auiLumaRunTr1 = g_auiLumaRunTr14x4[uiTr1];
        UInt ui_maxrun = maxrun;
        if (N==8)
        { 
          p_auiLumaRunTr1 = g_auiLumaRunTr18x8[uiTr1];
          ui_maxrun =min(maxrun,28);
        }
        if(nTab == 2 || nTab == 5)
        {
          cn = xRunLevelInd(lev, run, maxrun, p_auiLumaRunTr1[ui_maxrun]);
          vlc = g_auiVlcTable8x8Intra[ui_maxrun];
        }
        else
        {
          cn = g_auiLumaRun8x8[ui_maxrun][lev][run];
          vlc = g_auiVlcTable8x8Inter[ui_maxrun];
        }
#else                                      
        if (maxrun > 27)
        {
          cn = g_auiLumaRun8x8[28][k>1 ? 1 : 0][run];
        }
        else
        {
          cn = g_auiLumaRun8x8[maxrun][k>1 ? 1 : 0][run];
        }
        vlc = (maxrun>27) ? 3 : vlctable_8x8[maxrun];
#endif
        bits = VLClength[vlc][cn];
        if (k>1)
        {
          temp = 2*(k-2)+sign;
          if(temp > 127)
            temp = 127;
          bits += VLClength[0][temp];
        }
        else
          bits += 1;  
        
      }
      else
      {
        if(k > 127)
          k = 127;
        bits = VLClength[vlc_adaptive][k] + 1;
      }
    }
  }
  else
  {
    if (levelMode)
      bits = VLClength[vlc_adaptive][k];
    else
    {                        
      if (pos==0 && lpflag==0)
      {  
#if QC_MOD_LCEC
        const UInt *p_auiLumaRunTr1 = g_auiLumaRunTr14x4[uiTr1];
        UInt ui_maxrun = maxrun;
        if (N==8)
        { 
           p_auiLumaRunTr1 = g_auiLumaRunTr18x8[uiTr1];
           ui_maxrun =min(maxrun,28);
        }
        if(nTab == 2 || nTab == 5)
        {
          cn = xRunLevelInd(0, run+1, maxrun, p_auiLumaRunTr1[ui_maxrun]);
          vlc = g_auiVlcTable8x8Intra[ui_maxrun];
        }
        else
        {
          cn = g_auiLumaRun8x8[ui_maxrun][0][run+1];
          vlc = g_auiVlcTable8x8Inter[ui_maxrun];
        }
#else   
        if (maxrun > 27)
        {
          cn = g_auiLumaRun8x8[28][0][run+1];
        }
        else
        {
          cn = g_auiLumaRun8x8[maxrun][0][run+1];
        }
        vlc = (maxrun>27) ? 3 : vlctable_8x8[maxrun];
#endif
        bits = VLClength[vlc][cn];
      }
      else
        bits = 0;
    }
  }
  return bits;
}
#endif
#if QC_MOD_LCEC_RDOQ
static levelDataStruct slevelData  [ MAX_CU_SIZE*MAX_CU_SIZE ];
Void TComTrQuant::xRateDistOptQuant_LCEC(TComDataCU* pcCU, Long* pSrcCoeff, TCoeff*& pDstCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, 
                                         UInt uiAbsPartIdx )
{
  Int     i, j;
  Int     iShift = 0;
  Int     qp_rem, q_bits;
  Double  err, lagr, lagrMin;
  Double  normFact = 0.0;
  Double  OneOverNormFact = 0.0;
  Double  fTemp = 0.0;
  Int     iQuantCoeff;
  
#if E243_CORE_TRANSFORMS
  Int     iShiftQBits, iSign, iRate, lastPosMin, iBlockType;
  UInt    uiBitShift = SCALE_BITS, uiScanPos, levelInd;
  Int     levelBest, iLevel;
#else
  Bool    bExt8x8Flag = false;
  Int     iShiftQBits, iSign, iRate, lastPosMin, iBlockType;
  UInt    uiBitShift = 15, uiScanPos, levelInd;
  Int     levelBest, iLevel, iAdd;
#endif

  levelDataStruct* levelData = &slevelData[0];

  Int     iPos, iScanning;

#if CAVLC_COEF_LRG_BLK
  static TCoeff sQuantCoeff[1024];
#else
  static TCoeff sQuantCoeff[256];
#endif

  qp_rem    = m_cQP.m_iRem;
  q_bits    = m_cQP.m_iBits;

  UInt noCoeff=(uiWidth < 8 ? 16 : 64);
#if CAVLC_COEF_LRG_BLK
  UInt maxBlSize = (eTType==TEXT_LUMA)? 32:8;
  UInt uiBlSize = Min(uiWidth,maxBlSize);
  noCoeff = uiBlSize*uiBlSize;
#endif

#if E243_CORE_TRANSFORMS 
  UInt uiLog2TrSize = g_aucConvertToBit[ uiWidth ] + 2;
  UInt uiQ = g_auiQ[m_cQP.rem()];

#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;
#endif
  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
  double dErrScale = (double)(1<<uiBitShift);                              // Compensate for scaling of bitcount in Lagrange cost function
  dErrScale = dErrScale*pow(2.0,-2.0*iTransformShift);                     // Compensate for scaling through forward transform
  dErrScale = dErrScale/(double)(uiQ*uiQ);                                 // Compensate for qp-dependent multiplier applied before calculating the Lagrange cost function
  dErrScale = dErrScale/(double)(1<<(2*g_uiBitIncrement));                   // Compensate for Lagrange multiplier that is tuned towards 8-bit input

  q_bits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                   // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

  iShift = uiLog2TrSize;
  if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
  {
    iBlockType = eTType-2;
  }
  else
  {
    iBlockType = (uiWidth < 16 ? 2 : 5) + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
  }
#else
  if ( uiWidth == 4 && uiHeight == 4 )
  {
    normFact = pow(2., (2 * DQ_BITS + 19));
    if ( g_uiBitIncrement ) normFact *= 1 << (2 * g_uiBitIncrement);
    m_puiQuantMtx = &g_aiQuantCoef  [m_cQP.m_iRem][0];
    iShift = 2;
    iAdd = m_cQP.m_iAdd4x4;

    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
  }
  else
    if ( uiWidth == 8 && uiHeight == 8 )
    {
      q_bits++;
      normFact = pow(2., (2 * Q_BITS_8 + 9));
      if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
      m_puiQuantMtx = &g_aiQuantCoef64[m_cQP.m_iRem][0];
      iShift = 3;
      iAdd = m_cQP.m_iAdd8x8;

      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    }
    else
      if ( uiWidth == 16 && uiHeight == 16 )
      {
        q_bits = ECore16Shift + m_cQP.per();
        normFact = pow(2., 21);
        if ( g_uiBitIncrement ) normFact *= 1 << (2 * g_uiBitIncrement);
        fTemp = estErr16x16[qp_rem] / normFact;

        m_puiQuantMtx = ( &g_aiQuantCoef256[m_cQP.m_iRem][0] );
        iShift = 4;
        iAdd = m_cQP.m_iAdd16x16;
        bExt8x8Flag = true;

        if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
          iBlockType = eTType-2;
        else
          iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
      }
      else
        if ( uiWidth == 32 && uiHeight == 32 )
        {
          q_bits = ECore32Shift + m_cQP.per();
          normFact = pow(2., 21);
          if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
          fTemp = estErr32x32[qp_rem]/normFact;

          m_puiQuantMtx = ( &g_aiQuantCoef1024[m_cQP.m_iRem][0] );
          iShift = 5;
          iAdd = m_cQP.m_iAdd32x32;
          bExt8x8Flag = true;

          if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
            iBlockType = eTType-2;
          else
            iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
        }
        else
        {
          iBlockType =0;

          assert(0);
        }
#endif //E243_CORE_TRANSFORMS
        memset(&pDstCoeff[0],0,uiWidth*uiHeight*sizeof(TCoeff)); 

        iShiftQBits = (1 <<( q_bits - 1));


#if QC_MDCS
#if CAVLC_COEF_LRG_BLK
        UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : uiBlSize   ] + 2;
#else
        UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : Min(8,uiWidth)    ] + 2;
#endif
        const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#else
        const UInt* pucScan;
        if( !pcCU->isIntra(uiAbsPartIdx )){
#if CAVLC_COEF_LRG_BLK
          pucScan = g_auiFrameScanXY[ g_aucConvertToBit[ uiBlSize ] + 1];
#else
          pucScan = g_auiFrameScanXY[ g_aucConvertToBit[ Min(uiWidth, 8) ] + 1];
#endif
        }
        else{
          pucScan = g_auiFrameScanXY[ g_aucConvertToBit[ uiWidth ] + 1];
        }
#endif //QC_MDCS



        OneOverNormFact = 1.0 / normFact;

        UInt uiShift_local = iShift;
        UInt uiRes_local = (uiWidth - 1);
        UInt uiWidth_local = uiShift_local;

#if CAVLC_COEF_LRG_BLK
        if(!pcCU->isIntra(uiAbsPartIdx) && uiWidth > maxBlSize)
        {
          uiShift_local = g_aucConvertToBit[ maxBlSize ] + 2;;
          uiRes_local = maxBlSize - 1; 
        }
#else
        if( !pcCU->isIntra(uiAbsPartIdx) && uiWidth >= 16 )
        {
          uiShift_local = 3;
          uiRes_local = 7; 
        }
#endif

        Int iAddRDOQ = 0;
#if E243_CORE_TRANSFORMS
        /* Code below is consistent with JCTVC-E243 but could preferably be replaced with iAddRDOQ = 171 << (q_bits-9); */
        if (q_bits>=15)
        {
          iAddRDOQ = (uiWidth<16 ? 10922 : 10880) << (q_bits-15);
        }
        else
        {
          iAddRDOQ = (uiWidth<16 ? 10922 : 10880) >> (15-q_bits);
        }
#else
        {
          UInt uiSliceType = 0;

          Int iQP=pcCU->getSlice()->getSliceQp();
          if( eTType != TEXT_LUMA ) //Chroma
          {
            iQP  = g_aucChromaScale[ iQP ];
          }

          Int iDefaultOffset;
          Int iDefaultOffset_LTR;
          Int iPer;
          Int k =  (iQP + 6 * g_uiBitIncrement) / 6;
#if FULL_NBIT
          k += g_uiBitDepth - 8;
#endif
          Bool bLowPass = (uiSliceType == 0);
          iDefaultOffset = (bLowPass ? 10922 : 5462);

          bLowPass = (uiSliceType == 0);
          iDefaultOffset_LTR = (bLowPass? 170 : 86);

          if ( uiWidth == 4 && uiWidth == 4 ){
            iPer = QP_BITS + k - QOFFSET_BITS;
            iAddRDOQ = iDefaultOffset << iPer;
          }
          else if ( uiWidth == 8 && uiWidth == 8){
            iPer = QP_BITS + k + 1 - QOFFSET_BITS;
            iAddRDOQ = iDefaultOffset << iPer;
          }
          else if ( uiWidth == 16 && uiHeight == 16 ){
            iPer = ECore16Shift + k - QOFFSET_BITS_LTR;
            iAddRDOQ = iDefaultOffset_LTR << iPer;
          }
          else if ( uiWidth == 32 && uiHeight == 32 ){
            iPer = ECore32Shift + k - QOFFSET_BITS_LTR;
            iAddRDOQ = iDefaultOffset_LTR << iPer;
          }
        }
#endif
        if (m_uiRDOQOffset==1)
          iAddRDOQ=iShiftQBits;

        for (iScanning=noCoeff-1; iScanning>=0; iScanning--) 
        {
#if QC_MDCS
          iPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][iScanning];
#else
          iPos = pucScan[iScanning];
#endif //QC_MDCS
          j = iPos >> uiShift_local;
          i = iPos &  uiRes_local;
          iPos = (j << uiWidth_local) + i;

          levelDataStruct *psLevelData = &levelData[iScanning];
#if E243_CORE_TRANSFORMS
          psLevelData->levelDouble = abs(pSrcCoeff[iPos]) * uiQ;          
          iQuantCoeff = (Int)((psLevelData->levelDouble + iAddRDOQ) >> q_bits);

          psLevelData->levelQ   = (Long)( psLevelData->levelDouble >> q_bits );
          psLevelData->lowerInt = ( ( psLevelData->levelDouble - (psLevelData->levelQ << q_bits) ) < iShiftQBits ) ? true : false;

          iSign = pSrcCoeff[iPos] < 0 ? -1 : 1;
          sQuantCoeff[iScanning] = iQuantCoeff*iSign;

          fTemp = dErrScale;
#else
          psLevelData->levelDouble = abs( pSrcCoeff[iPos] * (Long) m_puiQuantMtx[iPos]);
          iQuantCoeff=(Int)((psLevelData->levelDouble +iAddRDOQ) >> q_bits);

          psLevelData->levelQ   = ( psLevelData->levelDouble >> q_bits );
          psLevelData->lowerInt = ( ( psLevelData->levelDouble - (psLevelData->levelQ << q_bits) ) < iShiftQBits ) ? true : false;

          iSign = pSrcCoeff[iPos] < 0 ? -1 : 1;
          sQuantCoeff[iScanning] = iQuantCoeff*iSign;

          if      ( uiWidth == 4 ) fTemp = estErr4x4[qp_rem][i][j] * OneOverNormFact;
          else if ( uiWidth == 8 ) fTemp = estErr8x8[qp_rem][i][j] * OneOverNormFact;
#endif
          psLevelData->level[0] = 0;
          err = (Double)(psLevelData->levelDouble);
          psLevelData->errLevel[0] = err * err * fTemp;

          if ( !psLevelData->levelQ )
          {
            if ( psLevelData->lowerInt )
            {
              psLevelData->noLevels = 1;
            }
            else
            {
              psLevelData->level[1] = 1;
              psLevelData->noLevels = 2;
            }
            if (iQuantCoeff==0)
              psLevelData->quantInd=0;
            else
              psLevelData->quantInd=1;

          }
          else if ( psLevelData->lowerInt )
          {
            psLevelData->level[1] = psLevelData->levelQ;
            psLevelData->noLevels = 2;

            if ( psLevelData->levelQ > 1 ){
              psLevelData->noLevels++;
              psLevelData->level[2] = 1;
            }

            psLevelData->quantInd = 1;
          }
          else
          {
            psLevelData->level[1] = psLevelData->levelQ;
            psLevelData->level[2] = psLevelData->levelQ + 1;
            psLevelData->noLevels = 3;

            if ( psLevelData->levelQ > 1 ){
              psLevelData->noLevels++;
              psLevelData->level[3] = 1;
            }
            if ( iQuantCoeff == psLevelData->level[1] )
              psLevelData->quantInd = 1;
            else
              psLevelData->quantInd = 2;
          }

          for ( levelInd = 1; levelInd < psLevelData->noLevels; levelInd++ ){
            err = (Double)((psLevelData->level[levelInd] << q_bits) - psLevelData->levelDouble);
            psLevelData->errLevel[levelInd] = err * err * fTemp;
            psLevelData->level[levelInd] *= iSign;
          }
        }

#if CAVLC_COEF_LRG_BLK==0
        UInt uiNum;
        if ( uiWidth == 4 )
          uiNum = 4;
        else
          uiNum = 8;
#endif

        // Last Position
#if CAVLC_COEF_LRG_BLK
        lastPosMin = xCodeCoeffCountBitsLast(sQuantCoeff, levelData, iBlockType, uiBlSize);
#else
        lastPosMin = xCodeCoeffCountBitsLast(sQuantCoeff, levelData, iBlockType, uiNum);
#endif
        memset(&sQuantCoeff[lastPosMin+1],0,sizeof(TCoeff) * (noCoeff - (lastPosMin + 1)));


        Int  iLpFlag = 1; 
        Int  iLevelMode = 0;
        Int  iRun = 0;
        Int  iVlc_adaptive = 0;
        Int  iMaxrun = 0;
        Int  iSum_big_coef = 0;


        UInt uiTr1=0;
        UInt absBestLevel;

        Int atable[5] = {4,6,14,28,0xfffffff};
        Int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};

        Int levelIndBest, iRateMin=0, levelStart;
        Double lagrCoded=0, lagrNotCoded=0;
#if CAVLC_COEF_LRG_BLK
        const UInt **pLumaRunTr1 = (uiWidth==4)? g_pLumaRunTr14x4:g_pLumaRunTr18x8;
        UInt coeffBlkSize = (uiWidth==4)? 4:(noCoeff==64)? 8:(noCoeff==256)? 16:32;
#endif

        for (iScanning = lastPosMin; iScanning>=0; iScanning--){
          uiScanPos = iScanning;
          levelStart = (iScanning == lastPosMin) ? 1 : 0;

          sQuantCoeff[uiScanPos] = levelBest = 0;
          levelDataStruct *psLevelData = &levelData[uiScanPos];
          if ( psLevelData->noLevels >1 || iScanning == 0 ){

            lagrMin = 0; iRateMin = 0;
            for (levelInd = levelStart; levelInd < psLevelData->noLevels; levelInd++){

              lagr = psLevelData->errLevel[levelInd];
              iLevel=psLevelData->level[levelInd];

#if CAVLC_COEF_LRG_BLK
              iRate = bitCountRDOQ(iLevel,uiScanPos,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,
                coeffBlkSize,uiTr1, iSum_big_coef, iBlockType, pcCU, pLumaRunTr1)<<uiBitShift;
#else
              if ( uiWidth == 4 ){
                iRate = bitCountRDOQ(iLevel,uiScanPos,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4,uiTr1, 
                  iSum_big_coef,iBlockType, pcCU)<<uiBitShift;
              }
              else{
                iRate = bitCountRDOQ(iLevel,uiScanPos,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8,uiTr1, 
                  iSum_big_coef, iBlockType, pcCU) << uiBitShift;
              }
#endif
              lagr += m_dLambda * iRate; 

              if ( lagr < lagrMin || levelInd == levelStart){
                lagrMin = lagr;
                iRateMin = iRate;
                levelBest = iLevel;
                levelIndBest = levelInd;
              }
            }
          }

          if ( levelBest != 0 ){
            lagrCoded += lagrMin;
            lagrNotCoded += psLevelData->errLevel[0];
          }
          if ( uiScanPos == 0 && levelBest == 0 ){
            lagrCoded += m_dLambda * iRateMin;
          }

          sQuantCoeff[uiScanPos] = levelBest;

          absBestLevel = abs(levelBest);
          if ( levelBest != 0 ){  

            if ( uiWidth > 4 ){ 
              if ( !iLpFlag && absBestLevel > 1 ){
                iSum_big_coef += absBestLevel;
                if ((noCoeff - uiScanPos - 1) > switch_thr[iBlockType] || iSum_big_coef > 2) iLevelMode = 1; 
              }
            }
            else{
              if ( absBestLevel > 1 ) iLevelMode = 1;
            }

            if ( iLpFlag == 1 )
            {
              uiTr1 = (absBestLevel > 1) ? 0 : 1;
            }
            else
            {
              if ( uiTr1 == 0 || absBestLevel >= 2 )
              { 
                uiTr1 = 0;
              }
              else if ( uiTr1 < MAX_TR1 )
              {
                uiTr1++;
              }
            }
            iMaxrun = iScanning - 1;
            iLpFlag = 0;
            iRun = 0;
            if ( iLevelMode && (absBestLevel > atable[iVlc_adaptive])) iVlc_adaptive++;        
          }
          else
          {
            iRun += 1;         
          }
        }

        if (lastPosMin >= 0 && lagrCoded > lagrNotCoded){
          for (iScanning = lastPosMin; iScanning>=0; iScanning--){
            sQuantCoeff[iScanning] = 0;
          }
        }

#if CAVLC_COEF_LRG_BLK 
        if ((!pcCU->isIntra(uiAbsPartIdx) && uiWidth > maxBlSize))
        {
          for (iScanning=noCoeff-1; iScanning>=0; iScanning--) 
          {
#if QC_MDCS
            iPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][iScanning];
#else
            iPos = pucScan[iScanning];
#endif //QC_MDCS
            j = iPos >>  (g_aucConvertToBit[ maxBlSize ] + 2);
            i = iPos & (maxBlSize-1);
            iPos = (j<<(g_aucConvertToBit[ uiWidth ] + 2))+i;
            pDstCoeff[iPos] = sQuantCoeff[iScanning];
            uiAbsSum += abs(sQuantCoeff[iScanning]);
          }
        }
#else
        if ((!pcCU->isIntra(uiAbsPartIdx) && uiWidth >= 16))
        {
          for (iScanning = noCoeff - 1; iScanning >= 0; iScanning--) 
          {
#if QC_MDCS
            iPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][iScanning];
#else
            iPos = pucScan[iScanning];
#endif //QC_MDCS
            j = iPos >> 3;
            i = iPos & 0x7;
            iPos = uiWidth * j + i;
            pDstCoeff[iPos] = sQuantCoeff[iScanning];
            uiAbsSum += abs(sQuantCoeff[iScanning]);
          }
        }
#endif
        else
        {
          for (iScanning = noCoeff - 1; iScanning >= 0; iScanning--) 
          {
#if QC_MDCS
            iPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][iScanning];
#else
            iPos = pucScan[iScanning];
#endif //QC_MDCS
            pDstCoeff[iPos] = sQuantCoeff[iScanning];
            uiAbsSum += abs(sQuantCoeff[iScanning]);
          }
        }
}
#else
Void TComTrQuant::xRateDistOptQuant_LCEC             ( TComDataCU*                     pcCU,
                                                      Long*                           plSrcCoeff,
                                                      TCoeff*&                        piDstCoeff,
                                                      UInt                            uiWidth,
                                                      UInt                            uiHeight,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Int iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,iSum_big_coef,iSign;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int switch_thr[8] = {49,49,0,49,49,0,49,49};
  
  const UInt* pucScan;
  UInt* piQuantCoef = NULL;
  UInt uiBlkPos,uiPosY,uiPosX,uiLog2BlkSize,uiConvBit,uiLevel,uiMaxLevel,uiMinLevel,uiAbsLevel,uiBestAbsLevel,uiBitShift;
  Int iScanning,iQpRem,iBlockType,iRate;
  Int  iQBits      = m_cQP.m_iBits;
  Int64 lLevelDouble;
  Double dErr,dTemp=0,dNormFactor,rd64UncodedCost,rd64CodedCost,dCurrCost;
  
  uiBitShift = 15;
  iQpRem = m_cQP.m_iRem;
  
  Bool bExt8x8Flag = false;
  uiLog2BlkSize = g_aucConvertToBit[ uiWidth ] + 2; 
  uiConvBit = g_aucConvertToBit[ uiWidth ];
  
#if QC_MDCS
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif //QC_MDCS
  
  if (uiWidth == 4)
  {
#if QC_MOD_LCEC
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#else
    iBlockType = 0 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
#endif
    iQBits = m_cQP.m_iBits;                 
    dNormFactor = pow(2., (2*DQ_BITS+19));
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    piQuantCoef = ( g_aiQuantCoef[m_cQP.rem()] );
  }
  else if (uiWidth == 8)
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    iQBits = m_cQP.m_iBits + 1;                 
    dNormFactor = pow(2., (2*Q_BITS_8+9)); 
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    piQuantCoef = ( g_aiQuantCoef64[m_cQP.rem()] );
  }
  else
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
      iBlockType = eTType-2;
    else
      iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    
    if(!pcCU->isIntra(uiAbsPartIdx))
    {
      uiLog2BlkSize = g_aucConvertToBit[ 8 ] + 2; 
      uiConvBit = g_aucConvertToBit[ 8 ];
    }
    dNormFactor = pow(2., 21);
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    
    bExt8x8Flag = true;
    
    if ( uiWidth == 16)
    {
      piQuantCoef = ( g_aiQuantCoef256[m_cQP.rem()] );
      iQBits = ECore16Shift + m_cQP.per();     
      dTemp = estErr16x16[iQpRem]/dNormFactor;
    }
    else if ( uiWidth == 32)
    {
      piQuantCoef = ( g_aiQuantCoef1024[m_cQP.rem()] );
      iQBits = ECore32Shift + m_cQP.per();
      dTemp = estErr32x32[iQpRem]/dNormFactor;
    }
    else
    {
      assert(0);
    }
    memset(&piDstCoeff[0],0,uiWidth*uiHeight*sizeof(TCoeff)); 
  }
  
  pucScan = g_auiFrameScanXY [ uiConvBit + 1 ];
  
  iLpFlag = 1;  // shawn note: last position flag
  iLevelMode = 0;
  iRun = 0;
  iVlc_adaptive = 0;
  iMaxrun = 0;
  iSum_big_coef = 0;
#if QC_MOD_LCEC
  UInt uiTr1=0;
#endif
  
  for (iScanning=(uiWidth<8 ? 15 : 63); iScanning>=0; iScanning--) 
  {            
#if QC_MDCS
    uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][iScanning];
#else
    uiBlkPos = pucScan[iScanning];
#endif //QC_MDCS
    uiPosY   = uiBlkPos >> uiLog2BlkSize;
    uiPosX   = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    
    if (uiWidth==4)
      dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor; 
    else if(uiWidth==8)
      dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if(!pcCU->isIntra(uiAbsPartIdx))
      uiBlkPos = uiWidth*uiPosY+uiPosX;
    
    lLevelDouble = abs(plSrcCoeff[uiBlkPos]);
    
    lLevelDouble = lLevelDouble * (Int64) ( uiWidth == 64? piQuantCoef[m_cQP.rem()]: piQuantCoef[uiBlkPos] );
    
    iSign = plSrcCoeff[uiBlkPos]<0 ? -1 : 1;
    
    
    uiLevel = (UInt)(lLevelDouble  >> iQBits);      
    uiMaxLevel = uiLevel + 1;
    uiMinLevel = Max(1,(Int)uiLevel - 2);
    
    uiBestAbsLevel = 0;
#if QC_MOD_LCEC
    if (uiWidth==4)
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4,uiTr1)<<uiBitShift; 
    else 
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8,uiTr1)<<uiBitShift; 
#else
    if (uiWidth==4)
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4)<<uiBitShift; 
    else 
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8)<<uiBitShift; 
#endif
    
    dErr = Double( lLevelDouble );
    rd64UncodedCost = dErr * dErr * dTemp;
    rd64CodedCost   = rd64UncodedCost + xGetICost( iRate ); 
    for(uiAbsLevel = uiMinLevel; uiAbsLevel <= uiMaxLevel ; uiAbsLevel++ ) 
    {
#if QC_MOD_LCEC
      if (uiWidth==4)
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4,uiTr1)<<uiBitShift; 
      else 
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8,uiTr1)<<uiBitShift; 
#else
      if (uiWidth==4)
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4)<<uiBitShift; 
      else 
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8)<<uiBitShift; 
#endif
      dErr = Double( lLevelDouble  - (((Int64)uiAbsLevel) << iQBits ) );
      rd64UncodedCost = dErr * dErr * dTemp;
      dCurrCost = rd64UncodedCost + xGetICost( iRate ); 
      if( dCurrCost < rd64CodedCost )
      {         
        uiBestAbsLevel  = uiAbsLevel;
        rd64CodedCost   = dCurrCost;
      }
    }
    
    
    if (uiBestAbsLevel)
    {                  
      if (uiWidth>4)
      { 
        if (!iLpFlag && uiBestAbsLevel > 1)
        {
          iSum_big_coef += uiBestAbsLevel;
          if ((63-iScanning) > switch_thr[iBlockType] || iSum_big_coef > 2)
            iLevelMode = 1;
        }
      }
      else
      {
        if (uiBestAbsLevel>1)
          iLevelMode = 1;
      }
#if QC_MOD_LCEC
      if (iLpFlag==1){
        if (uiBestAbsLevel>1){
          uiTr1=0;
        }
        else{
          uiTr1=1;
        }
      }
      else{
        if (uiTr1>0 && uiBestAbsLevel<2){
          uiTr1++;
          uiTr1=Min(MAX_TR1,uiTr1);
        }
        else{
          uiTr1=0;
        }
      }
#endif
      iMaxrun = iScanning-1;
      iLpFlag = 0;
      iRun = 0;
      if (iLevelMode && (uiBestAbsLevel > atable[iVlc_adaptive]))
        iVlc_adaptive++;                    
    }
    else
    {
      iRun += 1;         
    }
    
    uiAbsSum += uiBestAbsLevel;
    piDstCoeff[uiBlkPos] = iSign*uiBestAbsLevel;
  } // for uiScanning
} 
#endif

Void TComTrQuant::xQuantLTR  (TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  Long*   piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
  Int   iAdd = 0;
  
#if E243_CORE_TRANSFORMS
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
  }
  else
  {
    const UInt*  pucScan;
    UInt uiConvBit = g_aucConvertToBit[ iWidth ];
    pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];

    UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;
    UInt uiQ = g_auiQ[m_cQP.rem()];

#if FULL_NBIT
    UInt uiBitDepth = g_uiBitDepth;
#else
    UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;
#endif
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

    iAdd = (pcCU->getSlice()->getSliceType()==I_SLICE ? 171 : 85) << (iQBits-9);

    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
      Long iLevel;
      Int  iSign;
      UInt uiBlockPos = pucScan[n]; 
      iLevel  = (Long) piCoef[uiBlockPos];
      iSign   = (iLevel < 0 ? -1: 1);      

      iLevel = (abs(iLevel) * uiQ + iAdd ) >> iQBits;
#if CAVLC_COEF_LRG_BLK
      if (m_iSymbolMode == 0 && n>=64 && eTType != TEXT_LUMA)
      {
        iLevel = 0;
      }
#else
      if (m_iSymbolMode == 0 && iWidth>8)
      {
        /* Two methods of limiting number of encoded coefficients to 8x8 for intra and inter respectively */
        if (pcCU->isIntra( uiAbsPartIdx ))
        {
          if(n>=64) iLevel = 0;
        }
        else
        {
          if ((uiBlockPos%iWidth)>=8 || (uiBlockPos/iWidth)>=8) iLevel = 0;
        }
      }
#endif
      uiAcSum += iLevel;
      iLevel *= iSign;        
      piQCoef[uiBlockPos] = iLevel;
    } // for n
  } //if RDOQ
  //return;

#else //E243_CORE_TRANSFORMS

  UInt* piQuantCoef = NULL;
  Int   iNewBits    = 0;
  switch(iWidth)
  {
    case 2:
    {
      m_puiQuantMtx = &g_aiQuantCoef4[m_cQP.m_iRem];
      xQuant2x2(piCoef, piQCoef, uiAcSum );
      return;
    }
    case 4:
    {
      m_puiQuantMtx = &g_aiQuantCoef[m_cQP.m_iRem][0];
      xQuant4x4(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx );
      return;
    }
    case 8:
    {
      m_puiQuantMtx = &g_aiQuantCoef64[m_cQP.m_iRem][0];
      xQuant8x8(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx );
      return;
    }
    case 16:
    {
      piQuantCoef = ( g_aiQuantCoef256[m_cQP.rem()] );
      iNewBits = ECore16Shift + m_cQP.per();
      iAdd = m_cQP.m_iAdd16x16;
      break;
    }
    case 32:
    {
      piQuantCoef = ( g_aiQuantCoef1024[m_cQP.rem()] );
      iNewBits = ECore32Shift + m_cQP.per();
      iAdd = m_cQP.m_iAdd32x32;
      break;
    }
    default:
      assert(0);
      break;
  }
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
  }
  else
  {
    UInt uiAcSum_init = uiAcSum;
    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
      Long iLevel;
      Int  iSign;
      iLevel  = (Long) piCoef[n];
      iSign   = (iLevel < 0 ? -1: 1);
      if ( iWidth == 64 ) iLevel = abs( iLevel ) * piQuantCoef[m_cQP.rem()];
      else                iLevel = abs( iLevel ) * piQuantCoef[n];
      
      if (!pcCU->isIntra( uiAbsPartIdx ) && (m_iSymbolMode == 0) && ((n%iWidth)>=8 || (n/iWidth)>=8))
        iLevel = 0;
      else
        iLevel = ( iLevel + iAdd ) >> iNewBits;
      
      if( 0 != iLevel )
      {
        uiAcSum += iLevel;
        iLevel    *= iSign;
        piQCoef[n] = iLevel;
      }
      else
      {
        piQCoef[n] = 0;
      }
    }
    
    const UInt*  pucScan;
    if(pcCU->isIntra( uiAbsPartIdx ) && m_iSymbolMode == 0 && iWidth >= 16)
    {
      UInt uiConvBit = g_aucConvertToBit[ iWidth ];
      pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];
#if CAVLC_COEF_LRG_BLK
      UInt noCoeff = (eTType == TEXT_LUMA)? (iWidth*iHeight):64;

      for( Int n = noCoeff; n < iWidth*iHeight; n++ )
      {
        piQCoef[ pucScan[ n ] ] = 0;
      }
      uiAcSum = uiAcSum_init;
      for( Int n = 0; n < noCoeff; n++ )
      {
        uiAcSum += abs(piQCoef[ pucScan[ n ] ]);
      }
#else
      for( Int n = 64; n < iWidth*iHeight; n++ )
      {
        piQCoef[ pucScan[ n ] ] = 0;
      }
      
      uiAcSum = uiAcSum_init;
      
      for( Int n = 0; n < 64; n++ )
      {
        uiAcSum += abs(piQCoef[ pucScan[ n ] ]);
      }
#endif
    }
  }
#endif
}

Void TComTrQuant::xDeQuantLTR( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight )
{
  
  TCoeff* piQCoef   = pSrc;
  Long*   piCoef    = pDes;
  
  if ( iWidth > (Int)m_uiMaxTrSize )
  {
    iWidth  = m_uiMaxTrSize;
    iHeight = m_uiMaxTrSize;
  }
  
#if E243_CORE_TRANSFORMS
  Int iShift,iAdd,iCoeffQ;
  UInt uiQ;
  UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;

#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;
#endif
  UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize; 
  iShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
  iAdd = 1 << (iShift-1);
  uiQ = g_auiIQ[m_cQP.m_iRem];

  for( Int n = 0; n < iWidth*iHeight; n++ )
  {
    iCoeffQ = ((piQCoef[n]*(Int)uiQ << m_cQP.m_iPer)+iAdd)>>iShift;
    piCoef[n] = Clip3(-32768,32767,iCoeffQ);
  } 
#else
  UInt* piDeQuantCoef = NULL;
  switch(iWidth)
  {
    case 2:
    {
      xDeQuant2x2( piQCoef, piCoef );
      return;
    }
    case 4:
    {
      xDeQuant4x4( piQCoef, piCoef );
      return;
    }
    case 8:
    {
      xDeQuant8x8( piQCoef, piCoef );
      return;
    }
    case 16:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef256[m_cQP.rem()] );
      break;
    }
    case 32:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef1024[m_cQP.rem()] );
      break;
    }
    case 64:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef4096 ); // To save the memory for g_aiDeQuantCoef4096
      break;
    }
  }
  
  Int iLevel;
  Int iDeScale;
  
  for( Int n = 0; n < iWidth*iHeight; n++ )
  {
    iLevel  = piQCoef[n];
    
    if( 0 != iLevel )
    {
      if ( iWidth == 64 ) iDeScale = piDeQuantCoef[m_cQP.rem()];
      else                iDeScale = piDeQuantCoef[n];
      piCoef[n] = (Long) (iLevel*iDeScale) << m_cQP.per();
    }
    else
    {
      piCoef [n] = 0;
    }
  }
#endif
}
#if !E243_CORE_TRANSFORMS
Void TComTrQuant::xIT16( Long* pSrc, Pel* pDes, UInt uiStride )
{
  Int x, y;
  Long aaiTemp[16][16];
  
  Long B0, B1, B2, B3, B4, B5, B6, B7, B10, B11, B12, B13;
  Long C0, C1, C2, C3, C5, C6, C8, C9, C10, C11, C12, C13, C14, C15;
  Long D0, D1, D2, D3, D4, D5, D6, D7, D9, D10, D13, D14;
  Long E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15;
  Long F8, F9, F10, F11, F12, F13, F14, F15;
  
  UInt uiStride2  = uiStride<<1;
  UInt uiStride3  = uiStride2  + uiStride;
  UInt uiStride4  = uiStride3  + uiStride;
  UInt uiStride5  = uiStride4  + uiStride;
  UInt uiStride6  = uiStride5  + uiStride;
  UInt uiStride7  = uiStride6  + uiStride;
  UInt uiStride8  = uiStride7  + uiStride;
  UInt uiStride9  = uiStride8  + uiStride;
  UInt uiStride10 = uiStride9  + uiStride;
  UInt uiStride11 = uiStride10 + uiStride;
  UInt uiStride12 = uiStride11 + uiStride;
  UInt uiStride13 = uiStride12 + uiStride;
  UInt uiStride14 = uiStride13 + uiStride;
  UInt uiStride15 = uiStride14 + uiStride;
#ifdef TRANS_PRECISION_EXT
#if FULL_NBIT
  Int uiBitDepthIncrease=g_iShift16x16-g_uiBitIncrement-g_uiBitDepth+8;
#else
  Int uiBitDepthIncrease=g_iShift16x16-g_uiBitIncrement;
#endif
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    Long     ai0[16];
    ai0[0] =  pSrc[0]<<uiBitDepthIncrease;
    ai0[1] =  pSrc[1]<<uiBitDepthIncrease;
    ai0[2] =  pSrc[2]<<uiBitDepthIncrease;
    ai0[3] =  pSrc[3]<<uiBitDepthIncrease;
    ai0[4] =  pSrc[4]<<uiBitDepthIncrease;
    ai0[5] =  pSrc[5]<<uiBitDepthIncrease;
    ai0[6] =  pSrc[6]<<uiBitDepthIncrease;
    ai0[7] =  pSrc[7]<<uiBitDepthIncrease;
    ai0[8 ] =  pSrc[8 ]<<uiBitDepthIncrease;
    ai0[9 ] =  pSrc[9 ]<<uiBitDepthIncrease;
    ai0[10] =  pSrc[10]<<uiBitDepthIncrease;
    ai0[11] =  pSrc[11]<<uiBitDepthIncrease;
    ai0[12] =  pSrc[12]<<uiBitDepthIncrease;
    ai0[13] =  pSrc[13]<<uiBitDepthIncrease;
    ai0[14] =  pSrc[14]<<uiBitDepthIncrease;
    ai0[15] =  pSrc[15]<<uiBitDepthIncrease;
    F8 = xTrRound( 6 * ai0[1] - 63 * ai0[15] , DenShift16);
    F9 = xTrRound( 49 * ai0[9] - 40 * ai0[7] , DenShift16);
    F10 = xTrRound( 30 * ai0[5] - 56 * ai0[11] , DenShift16);
    F11 = xTrRound( 61 * ai0[13] - 18 * ai0[3] , DenShift16);
    F12 = xTrRound( 61 * ai0[3] + 18 * ai0[13] , DenShift16);
    F13 = xTrRound( 30 * ai0[11] + 56 * ai0[5] , DenShift16);
    F14 = xTrRound( 49 * ai0[7] + 40 * ai0[9] , DenShift16);
    F15 = xTrRound( 6 * ai0[15] + 63 * ai0[1] , DenShift16);
    
    E4 = xTrRound( 12 * ai0[2] - 62 * ai0[14] , DenShift16);
    E5 = xTrRound( 53 * ai0[10] - 35 * ai0[6] , DenShift16);
    E6 = xTrRound( 53 * ai0[6] + 35 * ai0[10] , DenShift16);
    E7 = xTrRound( 12 * ai0[14] + 62 * ai0[2] , DenShift16);
#else
    F8 = xTrRound( 6 * pSrc[1] - 63 * pSrc[15] , DenShift16);
    F9 = xTrRound( 49 * pSrc[9] - 40 * pSrc[7] , DenShift16);
    F10 = xTrRound( 30 * pSrc[5] - 56 * pSrc[11] , DenShift16);
    F11 = xTrRound( 61 * pSrc[13] - 18 * pSrc[3] , DenShift16);
    F12 = xTrRound( 61 * pSrc[3] + 18 * pSrc[13] , DenShift16);
    F13 = xTrRound( 30 * pSrc[11] + 56 * pSrc[5] , DenShift16);
    F14 = xTrRound( 49 * pSrc[7] + 40 * pSrc[9] , DenShift16);
    F15 = xTrRound( 6 * pSrc[15] + 63 * pSrc[1] , DenShift16);
    
    E4 = xTrRound( 12 * pSrc[2] - 62 * pSrc[14] , DenShift16);
    E5 = xTrRound( 53 * pSrc[10] - 35 * pSrc[6] , DenShift16);
    E6 = xTrRound( 53 * pSrc[6] + 35 * pSrc[10] , DenShift16);
    E7 = xTrRound( 12 * pSrc[14] + 62 * pSrc[2] , DenShift16);
#endif
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
#ifdef TRANS_PRECISION_EXT
    D0 = xTrRound( 45 * ( ai0[0] + ai0[8] ) , DenShift16);
    D1 = xTrRound( 45 * ( ai0[0] - ai0[8] ) , DenShift16);
    D2 = xTrRound( 24 * ai0[4] - 59 * ai0[12] , DenShift16);
    D3 = xTrRound( 59 * ai0[4] + 24 * ai0[12] , DenShift16);
#else
    D0 = xTrRound( 45 * ( pSrc[0] + pSrc[8] ) , DenShift16);
    D1 = xTrRound( 45 * ( pSrc[0] - pSrc[8] ) , DenShift16);
    D2 = xTrRound( 24 * pSrc[4] - 59 * pSrc[12] , DenShift16);
    D3 = xTrRound( 59 * pSrc[4] + 24 * pSrc[12] , DenShift16);
#endif
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 24 * E14 - 59 * E9 , DenShift16);
    D10 = xTrRound(  - 59 * E13 - 24 * E10 , DenShift16);
    D13 = xTrRound( 24 * E13 - 59 * E10 , DenShift16);
    D14 = xTrRound( 59 * E14 + 24 * E9 , DenShift16);
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 45 * ( D6 - D5 ) , DenShift16);
    C6 = xTrRound( 45 * ( D6 + D5 ) , DenShift16);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 45 * ( C13 - C10 ) , DenShift16);
    B13 = xTrRound( 45 * ( C13 + C10 ) , DenShift16);
    B11 = xTrRound( 45 * ( C12 - C11 ) , DenShift16);
    B12 = xTrRound( 45 * ( C12 + C11 ) , DenShift16);
    
    aaiTemp[0][y] = B0 + C15;
    aaiTemp[15][y] = B0 - C15;
    aaiTemp[1][y] = B1 + C14;
    aaiTemp[14][y] = B1 - C14;
    aaiTemp[2][y] = B2 + B13;
    aaiTemp[13][y] = B2 - B13;
    aaiTemp[3][y] = B3 + B12;
    aaiTemp[12][y] = B3 - B12;
    aaiTemp[4][y] = B4 + B11;
    aaiTemp[11][y] = B4 - B11;
    aaiTemp[5][y] = B5 + B10;
    aaiTemp[10][y] = B5 - B10;
    aaiTemp[6][y] = B6 + C9;
    aaiTemp[9][y] = B6 - C9;
    aaiTemp[7][y] = B7 + C8;
    aaiTemp[8][y] = B7 - C8;
    
    pSrc += 16;
  }
  
  for( x=0 ; x<16 ; x++, pDes++ )
  {
    F8 = xTrRound( 6 * aaiTemp[x][1] - 63 * aaiTemp[x][15] , DenShift16);
    F9 = xTrRound( 49 * aaiTemp[x][9] - 40 * aaiTemp[x][7] , DenShift16);
    F10 = xTrRound( 30 * aaiTemp[x][5] - 56 * aaiTemp[x][11] , DenShift16);
    F11 = xTrRound( 61 * aaiTemp[x][13] - 18 * aaiTemp[x][3] , DenShift16);
    F12 = xTrRound( 61 * aaiTemp[x][3] + 18 * aaiTemp[x][13] , DenShift16);
    F13 = xTrRound( 30 * aaiTemp[x][11] + 56 * aaiTemp[x][5] , DenShift16);
    F14 = xTrRound( 49 * aaiTemp[x][7] + 40 * aaiTemp[x][9] , DenShift16);
    F15 = xTrRound( 6 * aaiTemp[x][15] + 63 * aaiTemp[x][1] , DenShift16);
    
    E4 = xTrRound( 12 * aaiTemp[x][2] - 62 * aaiTemp[x][14] , DenShift16);
    E5 = xTrRound( 53 * aaiTemp[x][10] - 35 * aaiTemp[x][6] , DenShift16);
    E6 = xTrRound( 53 * aaiTemp[x][6] + 35 * aaiTemp[x][10] , DenShift16);
    E7 = xTrRound( 12 * aaiTemp[x][14] + 62 * aaiTemp[x][2] , DenShift16);
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    
    D0 = xTrRound( 45 * ( aaiTemp[x][0] + aaiTemp[x][8] ) , DenShift16);
    D1 = xTrRound( 45 * ( aaiTemp[x][0] - aaiTemp[x][8] ) , DenShift16);
    D2 = xTrRound( 24 * aaiTemp[x][4] - 59 * aaiTemp[x][12] , DenShift16);
    D3 = xTrRound( 59 * aaiTemp[x][4] + 24 * aaiTemp[x][12] , DenShift16);
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 24 * E14 - 59 * E9 , DenShift16);
    D10 = xTrRound(  - 59 * E13 - 24 * E10 , DenShift16);
    D13 = xTrRound( 24 * E13 - 59 * E10 , DenShift16);
    D14 = xTrRound( 59 * E14 + 24 * E9 , DenShift16);
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 45 * ( D6 - D5 ) , DenShift16);
    C6 = xTrRound( 45 * ( D6 + D5 ) , DenShift16);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 45 * ( C13 - C10 ) , DenShift16);
    B13 = xTrRound( 45 * ( C13 + C10 ) , DenShift16);
    B11 = xTrRound( 45 * ( C12 - C11 ) , DenShift16);
    B12 = xTrRound( 45 * ( C12 + C11 ) , DenShift16);
    
    pDes[          0] = (Pel)xTrRound(B0 + C15, DCore16Shift);
    pDes[uiStride15] = (Pel)xTrRound(B0 - C15, DCore16Shift);
    pDes[uiStride  ] = (Pel)xTrRound(B1 + C14, DCore16Shift);
    pDes[uiStride14] = (Pel)xTrRound(B1 - C14, DCore16Shift);
    pDes[uiStride2 ] = (Pel)xTrRound(B2 + B13, DCore16Shift);
    pDes[uiStride13] = (Pel)xTrRound(B2 - B13, DCore16Shift);
    pDes[uiStride3 ] = (Pel)xTrRound(B3 + B12, DCore16Shift);
    pDes[uiStride12] = (Pel)xTrRound(B3 - B12, DCore16Shift);
    pDes[uiStride4 ] = (Pel)xTrRound(B4 + B11, DCore16Shift);
    pDes[uiStride11] = (Pel)xTrRound(B4 - B11, DCore16Shift);
    pDes[uiStride5 ] = (Pel)xTrRound(B5 + B10, DCore16Shift);
    pDes[uiStride10] = (Pel)xTrRound(B5 - B10, DCore16Shift);
    pDes[uiStride6 ] = (Pel)xTrRound(B6 + C9, DCore16Shift);
    pDes[uiStride9 ] = (Pel)xTrRound(B6 - C9, DCore16Shift);
    pDes[uiStride7 ] = (Pel)xTrRound(B7 + C8, DCore16Shift);
    pDes[uiStride8 ] = (Pel)xTrRound(B7 - C8, DCore16Shift);
#ifdef TRANS_PRECISION_EXT
    pDes[        0 ] =  (pDes[        0 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride  ] =  (pDes[uiStride  ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride2 ] =  (pDes[uiStride2 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride3 ] =  (pDes[uiStride3 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride4 ] =  (pDes[uiStride4 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride5 ] =  (pDes[uiStride5 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride6 ] =  (pDes[uiStride6 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride7 ] =  (pDes[uiStride7 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride8 ] =  (pDes[uiStride8 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride9 ] =  (pDes[uiStride9 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride10] =  (pDes[uiStride10]+offset)>>uiBitDepthIncrease;
    pDes[uiStride11] =  (pDes[uiStride11]+offset)>>uiBitDepthIncrease;
    pDes[uiStride12] =  (pDes[uiStride12]+offset)>>uiBitDepthIncrease;
    pDes[uiStride13] =  (pDes[uiStride13]+offset)>>uiBitDepthIncrease;
    pDes[uiStride14] =  (pDes[uiStride14]+offset)>>uiBitDepthIncrease;
    pDes[uiStride15] =  (pDes[uiStride15]+offset)>>uiBitDepthIncrease;
#endif
    
  }
}

Void TComTrQuant::xIT32( Long* pSrc, Pel* pDes, UInt uiStride )
{
  Int x, y;
  Long aaiTemp[32][32];
  
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A20, A21, A22, A23, A24, A25, A26, A27;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B10, B11, B12, B13, B16, B17, B18, B19, B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31;
  Long C0, C1, C2, C3, C5, C6, C8, C9, C10, C11, C12, C13, C14, C15, C18, C19, C20, C21, C26, C27, C28, C29;
  Long D0, D1, D2, D3, D4, D5, D6, D7, D9, D10, D13, D14, D16, D17, D18, D19, D20, D21, D22, D23, D24, D25, D26, D27, D28, D29, D30, D31;
  Long E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E17, E18, E21, E22, E25, E26, E29, E30;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31;
  
  UInt uiStride2  = uiStride<<1;
  UInt uiStride3  = uiStride2  + uiStride;
  UInt uiStride4  = uiStride3  + uiStride;
  UInt uiStride5  = uiStride4  + uiStride;
  UInt uiStride6  = uiStride5  + uiStride;
  UInt uiStride7  = uiStride6  + uiStride;
  UInt uiStride8  = uiStride7  + uiStride;
  UInt uiStride9  = uiStride8  + uiStride;
  UInt uiStride10 = uiStride9  + uiStride;
  UInt uiStride11 = uiStride10 + uiStride;
  UInt uiStride12 = uiStride11 + uiStride;
  UInt uiStride13 = uiStride12 + uiStride;
  UInt uiStride14 = uiStride13 + uiStride;
  UInt uiStride15 = uiStride14 + uiStride;
  UInt uiStride16 = uiStride15 + uiStride;
  UInt uiStride17 = uiStride16 + uiStride;
  UInt uiStride18 = uiStride17 + uiStride;
  UInt uiStride19 = uiStride18 + uiStride;
  UInt uiStride20 = uiStride19 + uiStride;
  UInt uiStride21 = uiStride20 + uiStride;
  UInt uiStride22 = uiStride21 + uiStride;
  UInt uiStride23 = uiStride22 + uiStride;
  UInt uiStride24 = uiStride23 + uiStride;
  UInt uiStride25 = uiStride24 + uiStride;
  UInt uiStride26 = uiStride25 + uiStride;
  UInt uiStride27 = uiStride26 + uiStride;
  UInt uiStride28 = uiStride27 + uiStride;
  UInt uiStride29 = uiStride28 + uiStride;
  UInt uiStride30 = uiStride29 + uiStride;
  UInt uiStride31 = uiStride30 + uiStride;
#ifdef TRANS_PRECISION_EXT
#if FULL_NBIT
  Int uiBitDepthIncrease=g_iShift32x32-g_uiBitIncrement-g_uiBitDepth+8;
#else
  Int uiBitDepthIncrease=g_iShift32x32-g_uiBitIncrement;
#endif
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    Long     ai0[32];
    ai0[0] =  pSrc[0]<<uiBitDepthIncrease;
    ai0[1] =  pSrc[1]<<uiBitDepthIncrease;
    ai0[2] =  pSrc[2]<<uiBitDepthIncrease;
    ai0[3] =  pSrc[3]<<uiBitDepthIncrease;
    ai0[4] =  pSrc[4]<<uiBitDepthIncrease;
    ai0[5] =  pSrc[5]<<uiBitDepthIncrease;
    ai0[6] =  pSrc[6]<<uiBitDepthIncrease;
    ai0[7] =  pSrc[7]<<uiBitDepthIncrease;
    ai0[8 ] =  pSrc[8 ]<<uiBitDepthIncrease;
    ai0[9 ] =  pSrc[9 ]<<uiBitDepthIncrease;
    ai0[10] =  pSrc[10]<<uiBitDepthIncrease;
    ai0[11] =  pSrc[11]<<uiBitDepthIncrease;
    ai0[12] =  pSrc[12]<<uiBitDepthIncrease;
    ai0[13] =  pSrc[13]<<uiBitDepthIncrease;
    ai0[14] =  pSrc[14]<<uiBitDepthIncrease;
    ai0[15] =  pSrc[15]<<uiBitDepthIncrease;
    ai0[16] =  pSrc[16]<<uiBitDepthIncrease;
    ai0[17] =  pSrc[17]<<uiBitDepthIncrease;
    ai0[18] =  pSrc[18]<<uiBitDepthIncrease;
    ai0[19] =  pSrc[19]<<uiBitDepthIncrease;
    ai0[20] =  pSrc[20]<<uiBitDepthIncrease;
    ai0[21] =  pSrc[21]<<uiBitDepthIncrease;
    ai0[22] =  pSrc[22]<<uiBitDepthIncrease;
    ai0[23] =  pSrc[23]<<uiBitDepthIncrease;
    ai0[24] =  pSrc[24]<<uiBitDepthIncrease;
    ai0[25] =  pSrc[25]<<uiBitDepthIncrease;
    ai0[26] =  pSrc[26]<<uiBitDepthIncrease;
    ai0[27] =  pSrc[27]<<uiBitDepthIncrease;
    ai0[28] =  pSrc[28]<<uiBitDepthIncrease;
    ai0[29] =  pSrc[29]<<uiBitDepthIncrease;
    ai0[30] =  pSrc[30]<<uiBitDepthIncrease;
    ai0[31] =  pSrc[31]<<uiBitDepthIncrease;
    G16 = xTrRound(  12 * ai0[1]  - 255 * ai0[31], DenShift32);
    G17 = xTrRound( 189 * ai0[17] - 171 * ai0[15], DenShift32);
    G18 = xTrRound( 109 * ai0[9]  - 231 * ai0[23], DenShift32);
    G19 = xTrRound( 241 * ai0[25] -  86 * ai0[7], DenShift32);
    G20 = xTrRound(  62 * ai0[5]  - 248 * ai0[27], DenShift32);
    G21 = xTrRound( 219 * ai0[21] - 131 * ai0[11], DenShift32);
    G22 = xTrRound( 152 * ai0[13] - 205 * ai0[19], DenShift32);
    G23 = xTrRound( 253 * ai0[29] -  37 * ai0[3], DenShift32);
    G24 = xTrRound( 253 * ai0[3]  +  37 * ai0[29], DenShift32);
    G25 = xTrRound( 152 * ai0[19] + 205 * ai0[13], DenShift32);
    G26 = xTrRound( 219 * ai0[11] + 131 * ai0[21], DenShift32);
    G27 = xTrRound(  62 * ai0[27] + 248 * ai0[5], DenShift32);
    G28 = xTrRound( 241 * ai0[7]  +  86 * ai0[25], DenShift32);
    G29 = xTrRound( 109 * ai0[23] + 231 * ai0[9], DenShift32);
    G30 = xTrRound( 189 * ai0[15] + 171 * ai0[17], DenShift32);
    G31 = xTrRound(  12 * ai0[31] + 255 * ai0[1], DenShift32);
    
    F8  = xTrRound(  25 * ai0[2]  - 254 * ai0[30], DenShift32);
    F9  = xTrRound( 197 * ai0[18] - 162 * ai0[14], DenShift32);
    F10 = xTrRound( 120 * ai0[10] - 225 * ai0[22], DenShift32);
    F11 = xTrRound( 244 * ai0[26] -  74 * ai0[6], DenShift32);
    F12 = xTrRound( 244 * ai0[6]  +  74 * ai0[26], DenShift32);
    F13 = xTrRound( 120 * ai0[22] + 225 * ai0[10], DenShift32);
    F14 = xTrRound( 197 * ai0[14] + 162 * ai0[18], DenShift32);
    F15 = xTrRound(  25 * ai0[30] + 254 * ai0[2], DenShift32);
#else
    G16 = xTrRound( 12 * pSrc[1] - 255 * pSrc[31], DenShift32);
    G17 = xTrRound( 189 * pSrc[17] - 171 * pSrc[15], DenShift32);
    G18 = xTrRound( 109 * pSrc[9] - 231 * pSrc[23], DenShift32);
    G19 = xTrRound( 241 * pSrc[25] - 86 * pSrc[7], DenShift32);
    G20 = xTrRound( 62 * pSrc[5] - 248 * pSrc[27], DenShift32);
    G21 = xTrRound( 219 * pSrc[21] - 131 * pSrc[11], DenShift32);
    G22 = xTrRound( 152 * pSrc[13] - 205 * pSrc[19], DenShift32);
    G23 = xTrRound( 253 * pSrc[29] - 37 * pSrc[3], DenShift32);
    G24 = xTrRound( 253 * pSrc[3] + 37 * pSrc[29], DenShift32);
    G25 = xTrRound( 152 * pSrc[19] + 205 * pSrc[13], DenShift32);
    G26 = xTrRound( 219 * pSrc[11] + 131 * pSrc[21], DenShift32);
    G27 = xTrRound( 62 * pSrc[27] + 248 * pSrc[5], DenShift32);
    G28 = xTrRound( 241 * pSrc[7] + 86 * pSrc[25], DenShift32);
    G29 = xTrRound( 109 * pSrc[23] + 231 * pSrc[9], DenShift32);
    G30 = xTrRound( 189 * pSrc[15] + 171 * pSrc[17], DenShift32);
    G31 = xTrRound( 12 * pSrc[31] + 255 * pSrc[1], DenShift32);
    
    F8 = xTrRound( 25 * pSrc[2] - 254 * pSrc[30], DenShift32);
    F9 = xTrRound( 197 * pSrc[18] - 162 * pSrc[14], DenShift32);
    F10 = xTrRound( 120 * pSrc[10] - 225 * pSrc[22], DenShift32);
    F11 = xTrRound( 244 * pSrc[26] - 74 * pSrc[6], DenShift32);
    F12 = xTrRound( 244 * pSrc[6] + 74 * pSrc[26], DenShift32);
    F13 = xTrRound( 120 * pSrc[22] + 225 * pSrc[10], DenShift32);
    F14 = xTrRound( 197 * pSrc[14] + 162 * pSrc[18], DenShift32);
    F15 = xTrRound( 25 * pSrc[30] + 254 * pSrc[2], DenShift32);
#endif
    F16 = G16 + G17;
    F17 = G16 - G17;
    F18 = G19 - G18;
    F19 = G19 + G18;
    F20 = G20 + G21;
    F21 = G20 - G21;
    F22 = G23 - G22;
    F23 = G23 + G22;
    F24 = G24 + G25;
    F25 = G24 - G25;
    F26 = G27 - G26;
    F27 = G27 + G26;
    F28 = G28 + G29;
    F29 = G28 - G29;
    F30 = G31 - G30;
    F31 = G31 + G30;
#ifdef TRANS_PRECISION_EXT
    E4 = xTrRound( 49 * ai0[4] - 251 * ai0[28], DenShift32);
    E5 = xTrRound( 212 * ai0[20] - 142 * ai0[12], DenShift32);
    E6 = xTrRound( 212 * ai0[12] + 142 * ai0[20], DenShift32);
    E7 = xTrRound( 49 * ai0[28] + 251 * ai0[4], DenShift32);
#else
    E4 = xTrRound( 49 * pSrc[4] - 251 * pSrc[28], DenShift32);
    E5 = xTrRound( 212 * pSrc[20] - 142 * pSrc[12], DenShift32);
    E6 = xTrRound( 212 * pSrc[12] + 142 * pSrc[20], DenShift32);
    E7 = xTrRound( 49 * pSrc[28] + 251 * pSrc[4], DenShift32);
#endif
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    E17 = xTrRound( 49 * F30 - 251 * F17, DenShift32);
    E18 = xTrRound(  - 251 * F29 - 49 * F18, DenShift32);
    E21 = xTrRound( 212 * F26 - 142 * F21, DenShift32);
    E22 = xTrRound(  - 142 * F25 - 212 * F22, DenShift32);
    E25 = xTrRound( 212 * F25 - 142 * F22, DenShift32);
    E26 = xTrRound( 142 * F26 + 212 * F21, DenShift32);
    E29 = xTrRound( 49 * F29 - 251 * F18, DenShift32);
    E30 = xTrRound( 251 * F30 + 49 * F17, DenShift32);
#ifdef TRANS_PRECISION_EXT
    D0 = xTrRound( 181 * ( ai0[0] + ai0[16] ), DenShift32);
    D1 = xTrRound( 181 * ( ai0[0] - ai0[16] ), DenShift32);
    D2 = xTrRound( 97 * ai0[8] - 236 * ai0[24], DenShift32);
    D3 = xTrRound( 236 * ai0[8] + 97 * ai0[24], DenShift32);
#else
    D0 = xTrRound( 181 * ( pSrc[0] + pSrc[16] ), DenShift32);
    D1 = xTrRound( 181 * ( pSrc[0] - pSrc[16] ), DenShift32);
    D2 = xTrRound( 97 * pSrc[8] - 236 * pSrc[24], DenShift32);
    D3 = xTrRound( 236 * pSrc[8] + 97 * pSrc[24], DenShift32);
#endif
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 97 * E14 - 236 * E9, DenShift32);
    D10 = xTrRound(  - 236 * E13 - 97 * E10, DenShift32);
    D13 = xTrRound( 97 * E13 - 236 * E10, DenShift32);
    D14 = xTrRound( 236 * E14 + 97 * E9, DenShift32);
    D16 = F16 + F19;
    D19 = F16 - F19;
    D20 = F23 - F20;
    D23 = F23 + F20;
    D24 = F24 + F27;
    D27 = F24 - F27;
    D28 = F31 - F28;
    D31 = F31 + F28;
    D17 = E17 + E18;
    D18 = E17 - E18;
    D21 = E22 - E21;
    D22 = E22 + E21;
    D25 = E25 + E26;
    D26 = E25 - E26;
    D29 = E30 - E29;
    D30 = E30 + E29;
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 181 * ( D6 - D5 ), DenShift32);
    C6 = xTrRound( 181 * ( D6 + D5 ), DenShift32);
    C18 = xTrRound( 97 * D29 - 236 * D18, DenShift32);
    C20 = xTrRound(  - 236 * D27 - 97 * D20, DenShift32);
    C26 = xTrRound(  - 236 * D21 + 97 * D26, DenShift32);
    C28 = xTrRound( 97 * D19 + 236 * D28, DenShift32);
    C19 = xTrRound( 97 * D28 - 236 * D19, DenShift32);
    C21 = xTrRound(  - 236 * D26 - 97 * D21, DenShift32);
    C27 = xTrRound(  - 236 * D20 + 97 * D27, DenShift32);
    C29 = xTrRound( 97 * D18 + 236 * D29, DenShift32);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 181 * ( C13 - C10 ), DenShift32);
    B13 = xTrRound( 181 * ( C13 + C10 ), DenShift32);
    B11 = xTrRound( 181 * ( C12 - C11 ), DenShift32);
    B12 = xTrRound( 181 * ( C12 + C11 ), DenShift32);
    B16 = D16 + D23;
    B23 = D16 - D23;
    B24 = D31 - D24;
    B31 = D31 + D24;
    B17 = D17 + D22;
    B22 = D17 - D22;
    B25 = D30 - D25;
    B30 = D30 + D25;
    B18 = C18 + C21;
    B21 = C18 - C21;
    B26 = C29 - C26;
    B29 = C29 + C26;
    B19 = C19 + C20;
    B20 = C19 - C20;
    B27 = C28 - C27;
    B28 = C28 + C27;
    
    A0 = B0 + C15;
    A15 = B0 - C15;
    A1 = B1 + C14;
    A14 = B1 - C14;
    A2 = B2 + B13;
    A13 = B2 - B13;
    A3 = B3 + B12;
    A12 = B3 - B12;
    A4 = B4 + B11;
    A11 = B4 - B11;
    A5 = B5 + B10;
    A10 = B5 - B10;
    A6 = B6 + C9;
    A9 = B6 - C9;
    A7 = B7 + C8;
    A8 = B7 - C8;
    A20 = xTrRound( 181 * ( B27 - B20 ), DenShift32);
    A27 = xTrRound( 181 * ( B27 + B20 ), DenShift32);
    A21 = xTrRound( 181 * ( B26 - B21 ), DenShift32);
    A26 = xTrRound( 181 * ( B26 + B21 ), DenShift32);
    A22 = xTrRound( 181 * ( B25 - B22 ), DenShift32);
    A25 = xTrRound( 181 * ( B25 + B22 ), DenShift32);
    A23 = xTrRound( 181 * ( B24 - B23 ), DenShift32);
    A24 = xTrRound( 181 * ( B24 + B23 ), DenShift32);
    
    aaiTemp[0][y] = A0 + B31;
    aaiTemp[31][y] = A0 - B31;
    aaiTemp[1][y] = A1 + B30;
    aaiTemp[30][y] = A1 - B30;
    aaiTemp[2][y] = A2 + B29;
    aaiTemp[29][y] = A2 - B29;
    aaiTemp[3][y] = A3 + B28;
    aaiTemp[28][y] = A3 - B28;
    aaiTemp[4][y] = A4 + A27;
    aaiTemp[27][y] = A4 - A27;
    aaiTemp[5][y] = A5 + A26;
    aaiTemp[26][y] = A5 - A26;
    aaiTemp[6][y] = A6 + A25;
    aaiTemp[25][y] = A6 - A25;
    aaiTemp[7][y] = A7 + A24;
    aaiTemp[24][y] = A7 - A24;
    aaiTemp[8][y] = A8 + A23;
    aaiTemp[23][y] = A8 - A23;
    aaiTemp[9][y] = A9 + A22;
    aaiTemp[22][y] = A9 - A22;
    aaiTemp[10][y] = A10 + A21;
    aaiTemp[21][y] = A10 - A21;
    aaiTemp[11][y] = A11 + A20;
    aaiTemp[20][y] = A11 - A20;
    aaiTemp[12][y] = A12 + B19;
    aaiTemp[19][y] = A12 - B19;
    aaiTemp[13][y] = A13 + B18;
    aaiTemp[18][y] = A13 - B18;
    aaiTemp[14][y] = A14 + B17;
    aaiTemp[17][y] = A14 - B17;
    aaiTemp[15][y] = A15 + B16;
    aaiTemp[16][y] = A15 - B16;
    
    pSrc += 32;
  }
  
  for( x=0 ; x<32 ; x++, pDes++ )
  {
    G16 = xTrRound( 12 * aaiTemp[x][1] - 255 * aaiTemp[x][31], DenShift32);
    G17 = xTrRound( 189 * aaiTemp[x][17] - 171 * aaiTemp[x][15], DenShift32);
    G18 = xTrRound( 109 * aaiTemp[x][9] - 231 * aaiTemp[x][23], DenShift32);
    G19 = xTrRound( 241 * aaiTemp[x][25] - 86 * aaiTemp[x][7], DenShift32);
    G20 = xTrRound( 62 * aaiTemp[x][5] - 248 * aaiTemp[x][27], DenShift32);
    G21 = xTrRound( 219 * aaiTemp[x][21] - 131 * aaiTemp[x][11], DenShift32);
    G22 = xTrRound( 152 * aaiTemp[x][13] - 205 * aaiTemp[x][19], DenShift32);
    G23 = xTrRound( 253 * aaiTemp[x][29] - 37 * aaiTemp[x][3], DenShift32);
    G24 = xTrRound( 253 * aaiTemp[x][3] + 37 * aaiTemp[x][29], DenShift32);
    G25 = xTrRound( 152 * aaiTemp[x][19] + 205 * aaiTemp[x][13], DenShift32);
    G26 = xTrRound( 219 * aaiTemp[x][11] + 131 * aaiTemp[x][21], DenShift32);
    G27 = xTrRound( 62 * aaiTemp[x][27] + 248 * aaiTemp[x][5], DenShift32);
    G28 = xTrRound( 241 * aaiTemp[x][7] + 86 * aaiTemp[x][25], DenShift32);
    G29 = xTrRound( 109 * aaiTemp[x][23] + 231 * aaiTemp[x][9], DenShift32);
    G30 = xTrRound( 189 * aaiTemp[x][15] + 171 * aaiTemp[x][17], DenShift32);
    G31 = xTrRound( 12 * aaiTemp[x][31] + 255 * aaiTemp[x][1], DenShift32);
    
    F8 = xTrRound( 25 * aaiTemp[x][2] - 254 * aaiTemp[x][30], DenShift32);
    F9 = xTrRound( 197 * aaiTemp[x][18] - 162 * aaiTemp[x][14], DenShift32);
    F10 = xTrRound( 120 * aaiTemp[x][10] - 225 * aaiTemp[x][22], DenShift32);
    F11 = xTrRound( 244 * aaiTemp[x][26] - 74 * aaiTemp[x][6], DenShift32);
    F12 = xTrRound( 244 * aaiTemp[x][6] + 74 * aaiTemp[x][26], DenShift32);
    F13 = xTrRound( 120 * aaiTemp[x][22] + 225 * aaiTemp[x][10], DenShift32);
    F14 = xTrRound( 197 * aaiTemp[x][14] + 162 * aaiTemp[x][18], DenShift32);
    F15 = xTrRound( 25 * aaiTemp[x][30] + 254 * aaiTemp[x][2], DenShift32);
    F16 = G16 + G17;
    F17 = G16 - G17;
    F18 = G19 - G18;
    F19 = G19 + G18;
    F20 = G20 + G21;
    F21 = G20 - G21;
    F22 = G23 - G22;
    F23 = G23 + G22;
    F24 = G24 + G25;
    F25 = G24 - G25;
    F26 = G27 - G26;
    F27 = G27 + G26;
    F28 = G28 + G29;
    F29 = G28 - G29;
    F30 = G31 - G30;
    F31 = G31 + G30;
    
    E4 = xTrRound( 49 * aaiTemp[x][4] - 251 * aaiTemp[x][28], DenShift32);
    E5 = xTrRound( 212 * aaiTemp[x][20] - 142 * aaiTemp[x][12], DenShift32);
    E6 = xTrRound( 212 * aaiTemp[x][12] + 142 * aaiTemp[x][20], DenShift32);
    E7 = xTrRound( 49 * aaiTemp[x][28] + 251 * aaiTemp[x][4], DenShift32);
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    E17 = xTrRound( 49 * F30 - 251 * F17, DenShift32);
    E18 = xTrRound(  - 251 * F29 - 49 * F18, DenShift32);
    E21 = xTrRound( 212 * F26 - 142 * F21, DenShift32);
    E22 = xTrRound(  - 142 * F25 - 212 * F22, DenShift32);
    E25 = xTrRound( 212 * F25 - 142 * F22, DenShift32);
    E26 = xTrRound( 142 * F26 + 212 * F21, DenShift32);
    E29 = xTrRound( 49 * F29 - 251 * F18, DenShift32);
    E30 = xTrRound( 251 * F30 + 49 * F17, DenShift32);
    
    D0 = xTrRound( 181 * ( aaiTemp[x][0] + aaiTemp[x][16] ), DenShift32);
    D1 = xTrRound( 181 * ( aaiTemp[x][0] - aaiTemp[x][16] ), DenShift32);
    D2 = xTrRound( 97 * aaiTemp[x][8] - 236 * aaiTemp[x][24], DenShift32);
    D3 = xTrRound( 236 * aaiTemp[x][8] + 97 * aaiTemp[x][24], DenShift32);
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 97 * E14 - 236 * E9, DenShift32);
    D10 = xTrRound(  - 236 * E13 - 97 * E10, DenShift32);
    D13 = xTrRound( 97 * E13 - 236 * E10, DenShift32);
    D14 = xTrRound( 236 * E14 + 97 * E9, DenShift32);
    D16 = F16 + F19;
    D19 = F16 - F19;
    D20 = F23 - F20;
    D23 = F23 + F20;
    D24 = F24 + F27;
    D27 = F24 - F27;
    D28 = F31 - F28;
    D31 = F31 + F28;
    D17 = E17 + E18;
    D18 = E17 - E18;
    D21 = E22 - E21;
    D22 = E22 + E21;
    D25 = E25 + E26;
    D26 = E25 - E26;
    D29 = E30 - E29;
    D30 = E30 + E29;
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 181 * ( D6 - D5 ), DenShift32);
    C6 = xTrRound( 181 * ( D6 + D5 ), DenShift32);
    C18 = xTrRound( 97 * D29 - 236 * D18, DenShift32);
    C20 = xTrRound(  - 236 * D27 - 97 * D20, DenShift32);
    C26 = xTrRound(  - 236 * D21 + 97 * D26, DenShift32);
    C28 = xTrRound( 97 * D19 + 236 * D28, DenShift32);
    C19 = xTrRound( 97 * D28 - 236 * D19, DenShift32);
    C21 = xTrRound(  - 236 * D26 - 97 * D21, DenShift32);
    C27 = xTrRound(  - 236 * D20 + 97 * D27, DenShift32);
    C29 = xTrRound( 97 * D18 + 236 * D29, DenShift32);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 181 * ( C13 - C10 ), DenShift32);
    B13 = xTrRound( 181 * ( C13 + C10 ), DenShift32);
    B11 = xTrRound( 181 * ( C12 - C11 ), DenShift32);
    B12 = xTrRound( 181 * ( C12 + C11 ), DenShift32);
    B16 = D16 + D23;
    B23 = D16 - D23;
    B24 = D31 - D24;
    B31 = D31 + D24;
    B17 = D17 + D22;
    B22 = D17 - D22;
    B25 = D30 - D25;
    B30 = D30 + D25;
    B18 = C18 + C21;
    B21 = C18 - C21;
    B26 = C29 - C26;
    B29 = C29 + C26;
    B19 = C19 + C20;
    B20 = C19 - C20;
    B27 = C28 - C27;
    B28 = C28 + C27;
    
    A0 = B0 + C15;
    A15 = B0 - C15;
    A1 = B1 + C14;
    A14 = B1 - C14;
    A2 = B2 + B13;
    A13 = B2 - B13;
    A3 = B3 + B12;
    A12 = B3 - B12;
    A4 = B4 + B11;
    A11 = B4 - B11;
    A5 = B5 + B10;
    A10 = B5 - B10;
    A6 = B6 + C9;
    A9 = B6 - C9;
    A7 = B7 + C8;
    A8 = B7 - C8;
    A20 = xTrRound( 181 * ( B27 - B20 ), DenShift32);
    A27 = xTrRound( 181 * ( B27 + B20 ), DenShift32);
    A21 = xTrRound( 181 * ( B26 - B21 ), DenShift32);
    A26 = xTrRound( 181 * ( B26 + B21 ), DenShift32);
    A22 = xTrRound( 181 * ( B25 - B22 ), DenShift32);
    A25 = xTrRound( 181 * ( B25 + B22 ), DenShift32);
    A23 = xTrRound( 181 * ( B24 - B23 ), DenShift32);
    A24 = xTrRound( 181 * ( B24 + B23 ), DenShift32);
    
    pDes[         0] = (Pel)xTrRound( A0 + B31 , DCore32Shift);
    pDes[uiStride31] = (Pel)xTrRound( A0 - B31 , DCore32Shift);
    pDes[uiStride  ] = (Pel)xTrRound( A1 + B30 , DCore32Shift);
    pDes[uiStride30] = (Pel)xTrRound( A1 - B30 , DCore32Shift);
    pDes[uiStride2 ] = (Pel)xTrRound( A2 + B29 , DCore32Shift);
    pDes[uiStride29] = (Pel)xTrRound( A2 - B29 , DCore32Shift);
    pDes[uiStride3 ] = (Pel)xTrRound( A3 + B28 , DCore32Shift);
    pDes[uiStride28] = (Pel)xTrRound( A3 - B28 , DCore32Shift);
    pDes[uiStride4 ] = (Pel)xTrRound( A4 + A27 , DCore32Shift);
    pDes[uiStride27] = (Pel)xTrRound( A4 - A27 , DCore32Shift);
    pDes[uiStride5 ] = (Pel)xTrRound( A5 + A26 , DCore32Shift);
    pDes[uiStride26] = (Pel)xTrRound( A5 - A26 , DCore32Shift);
    pDes[uiStride6 ] = (Pel)xTrRound( A6 + A25 , DCore32Shift);
    pDes[uiStride25] = (Pel)xTrRound( A6 - A25 , DCore32Shift);
    pDes[uiStride7 ] = (Pel)xTrRound( A7 + A24 , DCore32Shift);
    pDes[uiStride24] = (Pel)xTrRound( A7 - A24 , DCore32Shift);
    pDes[uiStride8 ] = (Pel)xTrRound( A8 + A23 , DCore32Shift);
    pDes[uiStride23] = (Pel)xTrRound( A8 - A23 , DCore32Shift);
    pDes[uiStride9 ] = (Pel)xTrRound( A9 + A22 , DCore32Shift);
    pDes[uiStride22] = (Pel)xTrRound( A9 - A22 , DCore32Shift);
    pDes[uiStride10] = (Pel)xTrRound( A10 + A21 , DCore32Shift);
    pDes[uiStride21] = (Pel)xTrRound( A10 - A21 , DCore32Shift);
    pDes[uiStride11] = (Pel)xTrRound( A11 + A20 , DCore32Shift);
    pDes[uiStride20] = (Pel)xTrRound( A11 - A20 , DCore32Shift);
    pDes[uiStride12] = (Pel)xTrRound( A12 + B19 , DCore32Shift);
    pDes[uiStride19] = (Pel)xTrRound( A12 - B19 , DCore32Shift);
    pDes[uiStride13] = (Pel)xTrRound( A13 + B18 , DCore32Shift);
    pDes[uiStride18] = (Pel)xTrRound( A13 - B18 , DCore32Shift);
    pDes[uiStride14] = (Pel)xTrRound( A14 + B17 , DCore32Shift);
    pDes[uiStride17] = (Pel)xTrRound( A14 - B17 , DCore32Shift);
    pDes[uiStride15] = (Pel)xTrRound( A15 + B16 , DCore32Shift);
    pDes[uiStride16] = (Pel)xTrRound( A15 - B16 , DCore32Shift);
    
#ifdef TRANS_PRECISION_EXT
    pDes[        0 ] =  (pDes[        0 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride  ] =  (pDes[uiStride  ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride2 ] =  (pDes[uiStride2 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride3 ] =  (pDes[uiStride3 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride4 ] =  (pDes[uiStride4 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride5 ] =  (pDes[uiStride5 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride6 ] =  (pDes[uiStride6 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride7 ] =  (pDes[uiStride7 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride8 ] =  (pDes[uiStride8 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride9 ] =  (pDes[uiStride9 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride10] =  (pDes[uiStride10]+offset)>>uiBitDepthIncrease;
    pDes[uiStride11] =  (pDes[uiStride11]+offset)>>uiBitDepthIncrease;
    pDes[uiStride12] =  (pDes[uiStride12]+offset)>>uiBitDepthIncrease;
    pDes[uiStride13] =  (pDes[uiStride13]+offset)>>uiBitDepthIncrease;
    pDes[uiStride14] =  (pDes[uiStride14]+offset)>>uiBitDepthIncrease;
    pDes[uiStride15] =  (pDes[uiStride15]+offset)>>uiBitDepthIncrease;
    pDes[uiStride16] =  (pDes[uiStride16]+offset)>>uiBitDepthIncrease;
    pDes[uiStride17] =  (pDes[uiStride17]+offset)>>uiBitDepthIncrease;
    pDes[uiStride18] =  (pDes[uiStride18]+offset)>>uiBitDepthIncrease;
    pDes[uiStride19] =  (pDes[uiStride19]+offset)>>uiBitDepthIncrease;
    pDes[uiStride20] =  (pDes[uiStride20]+offset)>>uiBitDepthIncrease;
    pDes[uiStride21] =  (pDes[uiStride21]+offset)>>uiBitDepthIncrease;
    pDes[uiStride22] =  (pDes[uiStride22]+offset)>>uiBitDepthIncrease;
    pDes[uiStride23] =  (pDes[uiStride23]+offset)>>uiBitDepthIncrease;
    pDes[uiStride24] =  (pDes[uiStride24]+offset)>>uiBitDepthIncrease;
    pDes[uiStride25] =  (pDes[uiStride25]+offset)>>uiBitDepthIncrease;
    pDes[uiStride26] =  (pDes[uiStride26]+offset)>>uiBitDepthIncrease;
    pDes[uiStride27] =  (pDes[uiStride27]+offset)>>uiBitDepthIncrease;
    pDes[uiStride28] =  (pDes[uiStride28]+offset)>>uiBitDepthIncrease;
    pDes[uiStride29] =  (pDes[uiStride29]+offset)>>uiBitDepthIncrease;
    pDes[uiStride30] =  (pDes[uiStride30]+offset)>>uiBitDepthIncrease;
    pDes[uiStride31] =  (pDes[uiStride31]+offset)>>uiBitDepthIncrease;
#endif
  }
}
#endif //!E243_CORE_TRANSFORMS
#if QC_MOD_LCEC
Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Int iSymbolMode, UInt *aTableLP4, UInt *aTableLP8, UInt *aTableLastPosVlcIndex,
                       Bool bUseRDOQ,  Bool bEnc )
#else 
Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Int iSymbolMode, UInt *aTableLP4, UInt *aTableLP8, Bool bUseRDOQ,  Bool bEnc )
#endif
{
  m_uiMaxTrSize  = uiMaxTrSize;
  m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;
  m_uiLPTableE8 = aTableLP8;
  m_uiLPTableE4 = aTableLP4;
#if QC_MOD_LCEC
  m_uiLastPosVlcIndex=aTableLastPosVlcIndex;
#endif
  m_iSymbolMode = iSymbolMode;
  
  if ( m_bEnc )
  {
    m_cQP.initOffsetParam( MIN_QP, MAX_QP );
  }
}

Void TComTrQuant::xQuant( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  xQuantLTR(pcCU, pSrc, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
}

Void TComTrQuant::xDeQuant( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight )
{
  xDeQuantLTR( pSrc, pDes, iWidth, iHeight );
}

#if INTRA_DST_TYPE_7
Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  UInt uiMode;  //luma intra pred
  if(eTType == TEXT_LUMA && pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
    uiMode = pcCU->getLumaIntraDir( uiAbsPartIdx );
  else
    uiMode = REG_DCT;

  uiAbsSum = 0;
  assert( (pcCU->getSlice()->getSPS()->getMaxTrSize() >= uiWidth) );

  xT( uiMode, pcResidual, uiStride, m_plTempCoeff, uiWidth );
  xQuant( pcCU, m_plTempCoeff, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
}
#else
Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  uiAbsSum = 0;
  
  assert( (pcCU->getSlice()->getSPS()->getMaxTrSize() >= uiWidth) );
  
  xT( pcResidual, uiStride, m_plTempCoeff, uiWidth );
  xQuant( pcCU, m_plTempCoeff, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
}
#endif


#if INTRA_DST_TYPE_7
Void TComTrQuant::invtransformNxN( TextType eText,UInt uiMode, Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight);
  xIT( uiMode, m_plTempCoeff, rpcResidual, uiStride, uiWidth);
}
#else
Void TComTrQuant::invtransformNxN( Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight);
  xIT( m_plTempCoeff, rpcResidual, uiStride, uiWidth );
}
#endif

#if !E243_CORE_TRANSFORMS
Void TComTrQuant::xT2( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int itmp1, itmp2;
  
  itmp1 = piBlkResi[0] + piBlkResi[uiStride  ];
  itmp2 = piBlkResi[1] + piBlkResi[uiStride+1];
  
  psCoeff[0] = itmp1 + itmp2;
  psCoeff[1] = itmp1 - itmp2;
  
  itmp1 = piBlkResi[0] - piBlkResi[uiStride  ];
  itmp2 = piBlkResi[1] - piBlkResi[uiStride+1];
  
  psCoeff[2] = itmp1 + itmp2;
  psCoeff[3] = itmp1 - itmp2;
}

Void TComTrQuant::xT4( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  
  for( Int y = 0; y < 4; y++ )
  {
    tmp1 = piBlkResi[0] + piBlkResi[3];
    tmp2 = piBlkResi[1] + piBlkResi[2];
    
    aai[0][y] = tmp1 + tmp2;
    aai[2][y] = tmp1 - tmp2;
    
    tmp1 = piBlkResi[0] - piBlkResi[3];
    tmp2 = piBlkResi[1] - piBlkResi[2];
    
    aai[1][y] = tmp1 * 2 + tmp2 ;
    aai[3][y] = tmp1  - tmp2 * 2;
    piBlkResi += uiStride;
  }
  
  for( Int x = 0; x < 4; x++, psCoeff++ )
  {
    tmp1 = aai[x][0] + aai[x][3];
    tmp2 = aai[x][1] + aai[x][2];
    
    psCoeff[0] = tmp1 + tmp2;
    psCoeff[8] = tmp1 - tmp2;
    
    tmp1 = aai[x][0] - aai[x][3];
    tmp2 = aai[x][1] - aai[x][2];
    
    psCoeff[4]  = tmp1 * 2 + tmp2;
    psCoeff[12] = tmp1 - tmp2 * 2;
  }
}

Void TComTrQuant::xT8( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int aai[8][8];
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift8x8-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  for( Int i = 0; i < 8; i++, piBlkResi += uiStride )
  {
    Int ai1 [8];
    Int ai2 [8];
#ifdef TRANS_PRECISION_EXT
    ai1[0] = (piBlkResi[0] + piBlkResi[7])<<uiBitDepthIncrease;
    ai1[1] = (piBlkResi[1] + piBlkResi[6])<<uiBitDepthIncrease;
    ai1[2] = (piBlkResi[2] + piBlkResi[5])<<uiBitDepthIncrease;
    ai1[3] = (piBlkResi[3] + piBlkResi[4])<<uiBitDepthIncrease;
    
    ai1[4] = (piBlkResi[0] - piBlkResi[7])<<uiBitDepthIncrease;
    ai1[5] = (piBlkResi[1] - piBlkResi[6])<<uiBitDepthIncrease;
    ai1[6] = (piBlkResi[2] - piBlkResi[5])<<uiBitDepthIncrease;
    ai1[7] = (piBlkResi[3] - piBlkResi[4])<<uiBitDepthIncrease;
#else
    ai1[0] = piBlkResi[0] + piBlkResi[7];
    ai1[1] = piBlkResi[1] + piBlkResi[6];
    ai1[2] = piBlkResi[2] + piBlkResi[5];
    ai1[3] = piBlkResi[3] + piBlkResi[4];
    
    ai1[4] = piBlkResi[0] - piBlkResi[7];
    ai1[5] = piBlkResi[1] - piBlkResi[6];
    ai1[6] = piBlkResi[2] - piBlkResi[5];
    ai1[7] = piBlkResi[3] - piBlkResi[4];
#endif
    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);
    
    aai[0][i] =  ai2[0]     +  ai2[1];
    aai[2][i] =  ai2[2]     + (ai2[3]>>1);
    aai[4][i] =  ai2[0]     -  ai2[1];
    aai[6][i] = (ai2[2]>>1) -  ai2[3];
    
    aai[1][i] =  ai2[4]     + (ai2[7]>>2);
    aai[3][i] =  ai2[5]     + (ai2[6]>>2);
    aai[5][i] =  ai2[6]     - (ai2[5]>>2);
    aai[7][i] = (ai2[4]>>2) -  ai2[7];
  }
  
  // vertical transform
  for( Int n = 0; n < 8; n++, psCoeff++)
  {
    Int ai1[8];
    Int ai2[8];
    
    ai1[0] = aai[n][0] + aai[n][7];
    ai1[1] = aai[n][1] + aai[n][6];
    ai1[2] = aai[n][2] + aai[n][5];
    ai1[3] = aai[n][3] + aai[n][4];
    ai1[4] = aai[n][0] - aai[n][7];
    ai1[5] = aai[n][1] - aai[n][6];
    ai1[6] = aai[n][2] - aai[n][5];
    ai1[7] = aai[n][3] - aai[n][4];
    
    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);
    
    psCoeff[ 0] =  ai2[0]     +  ai2[1];
    psCoeff[16] =  ai2[2]     + (ai2[3]>>1);
    psCoeff[32] =  ai2[0]     -  ai2[1];
    psCoeff[48] = (ai2[2]>>1) -  ai2[3];
    
    psCoeff[ 8] =  ai2[4]     + (ai2[7]>>2);
    psCoeff[24] =  ai2[5]     + (ai2[6]>>2);
    psCoeff[40] =  ai2[6]     - (ai2[5]>>2);
    psCoeff[56] = (ai2[4]>>2) -  ai2[7];
#ifdef TRANS_PRECISION_EXT
    psCoeff[ 0] =  (psCoeff[ 0]+offset)>>uiBitDepthIncrease;
    psCoeff[16] =  (psCoeff[16]+offset)>>uiBitDepthIncrease;
    psCoeff[32] =  (psCoeff[32]+offset)>>uiBitDepthIncrease;
    psCoeff[48] =  (psCoeff[48]+offset)>>uiBitDepthIncrease;
    
    psCoeff[ 8] =  (psCoeff[ 8]+offset)>>uiBitDepthIncrease;
    psCoeff[24] =  (psCoeff[24]+offset)>>uiBitDepthIncrease;
    psCoeff[40] =  (psCoeff[40]+offset)>>uiBitDepthIncrease;
    psCoeff[56] =  (psCoeff[56]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xQuant2x2( Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum )
{
  Int iLevel;
  Int iSign;
  Int iOffset = 1<<(6+m_cQP.per());
  Int iBits   = m_cQP.per()+7;
  
  iSign       = plSrcCoef[0]>>31;
  iLevel      = abs(plSrcCoef[0]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[0] = iLevel;
  }
  else
  {
    pDstCoef[0] = 0;
  }
  
  iSign       = plSrcCoef[1]>>31;
  iLevel      = abs(plSrcCoef[1]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[1] = iLevel;
  }
  else
  {
    pDstCoef[1] = 0;
  }
  
  iSign       = plSrcCoef[2]>>31;
  iLevel      = abs(plSrcCoef[2]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[2] = iLevel;
  }
  else
  {
    pDstCoef[2] = 0;
  }
  
  iSign       = plSrcCoef[3]>>31;
  iLevel      = abs(plSrcCoef[3]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[3] = iLevel;
  }
  else
  {
    pDstCoef[3] = 0;
  }
}

Void TComTrQuant::xQuant4x4( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, plSrcCoef, pDstCoef, 4, 4, uiAbsSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 4, 4, uiAbsSum, eTType, uiAbsPartIdx );
  }
  else
  {
    for( Int n = 0; n < 16; n++ )
    {
      Int iLevel, iSign;
      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) * m_puiQuantMtx[n];
      
      iLevel      = ( iLevel + m_cQP.m_iAdd4x4 ) >> m_cQP.m_iBits;
      
      if( 0 != iLevel )
      {
        iSign     >>= 31;
        uiAbsSum   += iLevel;
        iLevel     ^= iSign;
        iLevel     -= iSign;
        pDstCoef[n] = iLevel;
      }
      else
      {
        pDstCoef [n] = 0;
      }
    }
  }
}

Void TComTrQuant::xQuant8x8( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  Int iBit = m_cQP.m_iBits + 1;
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, plSrcCoef, pDstCoef, 8, 8, uiAbsSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 8, 8, uiAbsSum, eTType, uiAbsPartIdx );
  }
  else
  {
    for( Int n = 0; n < 64; n++ )
    {
      Int iLevel, iSign;
      
      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) * m_puiQuantMtx[n];
      
      iLevel      = ( iLevel + m_cQP.m_iAdd8x8 ) >> iBit;
      
      if( 0 != iLevel )
      {
        iSign     >>= 31;
        uiAbsSum   += iLevel;
        iLevel     ^= iSign;
        iLevel     -= iSign;
        pDstCoef[n] = iLevel;
      }
      else
      {
        pDstCoef [n] = 0;
      }
    }
  }
}

Void TComTrQuant::xIT2( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Int itemp, itmp1, itmp2;
  Int iSign;
  UInt uiBits = 5;
  UInt uiOffset = 1<<(uiBits-1);
  
  itmp1 = plCoef[0] + plCoef[2];
  itmp2 = plCoef[1] + plCoef[3];
  
  itemp = itmp1 + itmp2;
  iSign = itemp>>31;
  pResidual[0] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[0] ^= iSign;
  pResidual[0] -= iSign;
  
  itemp = itmp1 - itmp2;
  iSign = itemp>>31;
  pResidual[1] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[1] ^= iSign;
  pResidual[1] -= iSign;
  
  itmp1 = plCoef[0] - plCoef[2];
  itmp2 = plCoef[1] - plCoef[3];
  
  itemp = itmp1 + itmp2;
  iSign = itemp>>31;
  pResidual[uiStride] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[uiStride] ^= iSign;
  pResidual[uiStride] -= iSign;
  
  itemp = itmp1 - itmp2;
  iSign = itemp>>31;
  pResidual[uiStride+1] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[uiStride+1] ^= iSign;
  pResidual[uiStride+1] -= iSign;
}

Void TComTrQuant::xIT4( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;
  Int uiStride2=(uiStride<<1);
  Int uiStride3=uiStride2 + uiStride;
  
  for( x = 0; x < 4; x++, plCoef+=4 )
  {
    tmp1 = plCoef[0] + plCoef[2];
    tmp2 = (plCoef[3]>>1) + plCoef[1];
    
    aai[0][x] = tmp1 + tmp2;
    aai[3][x] = tmp1 - tmp2;
    
    tmp1 = plCoef[0] - plCoef[2];
    tmp2 = (plCoef[1]>>1) - plCoef[3];
    
    aai[1][x] = tmp1 + tmp2;
    aai[2][x] = tmp1 - tmp2;
  }
  
  for( y = 0; y < 4; y++, pResidual ++ )
  {
    tmp1 =  aai[y][0] + aai[y][2];
    tmp2 = (aai[y][3]>>1) + aai[y][1];
    
    pResidual[0]         =  xRound( tmp1 + tmp2);
    pResidual[uiStride3] =  xRound( tmp1 - tmp2);
    
    tmp1 =  aai[y][0] - aai[y][2];
    tmp2 = (aai[y][1]>>1) - aai[y][3];
    
    pResidual[uiStride]  =  xRound( tmp1 + tmp2);
    pResidual[uiStride2] =  xRound( tmp1 - tmp2);
  }
}

Void TComTrQuant::xIT8( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Long aai[8][8];
  Int n;
#ifdef TRANS_PRECISION_EXT
#if FULL_NBIT
  Int uiBitDepthIncrease=g_iShift8x8-g_uiBitIncrement-g_uiBitDepth+8;
#else
  Int uiBitDepthIncrease=g_iShift8x8-g_uiBitIncrement;
#endif
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  UInt uiStride2 = uiStride<<1;
  UInt uiStride3 = uiStride2 + uiStride;
  UInt uiStride4 = uiStride3 + uiStride;
  UInt uiStride5 = uiStride4 + uiStride;
  UInt uiStride6 = uiStride5 + uiStride;
  UInt uiStride7 = uiStride6 + uiStride;
  
  for( n = 0; n < 8; n++ )
  {
    Long* pi = plCoef + (n<<3);
    Long     ai1[8];
    Long     ai2[8];
#ifdef TRANS_PRECISION_EXT
    Long     ai0[8];
    ai0[0] =  pi[0]<<uiBitDepthIncrease;
    ai0[1] =  pi[1]<<uiBitDepthIncrease;
    ai0[2] =  pi[2]<<uiBitDepthIncrease;
    ai0[3] =  pi[3]<<uiBitDepthIncrease;
    ai0[4] =  pi[4]<<uiBitDepthIncrease;
    ai0[5] =  pi[5]<<uiBitDepthIncrease;
    ai0[6] =  pi[6]<<uiBitDepthIncrease;
    ai0[7] =  pi[7]<<uiBitDepthIncrease;
    ai1[0] = ai0[0] + ai0[4];
    ai1[2] = ai0[0] - ai0[4];
    
    ai1[4] = (ai0[2]>>1) -  ai0[6];
    ai1[6] =  ai0[2]     + (ai0[6]>>1);
    
    ai1[1] = ai0[5] - ai0[3] - ai0[7] - (ai0[7]>>1);
    ai1[3] = ai0[1] + ai0[7] - ai0[3] - (ai0[3]>>1);;
    ai1[5] = ai0[7] - ai0[1] + ai0[5] + (ai0[5]>>1);
    ai1[7] = ai0[3] + ai0[5] + ai0[1] + (ai0[1]>>1);
#else
    ai1[0] = pi[0] + pi[4];
    ai1[2] = pi[0] - pi[4];
    
    ai1[4] = (pi[2]>>1) -  pi[6];
    ai1[6] =  pi[2]     + (pi[6]>>1);
    
    ai1[1] = pi[5] - pi[3] - pi[7] - (pi[7]>>1);
    ai1[3] = pi[1] + pi[7] - pi[3] - (pi[3]>>1);;
    ai1[5] = pi[7] - pi[1] + pi[5] + (pi[5]>>1);
    ai1[7] = pi[3] + pi[5] + pi[1] + (pi[1]>>1);
#endif
    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];
    
    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];
    
    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);
    
    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];
    
    aai[n][0] = ai2[0] + ai2[7];
    aai[n][1] = ai2[2] + ai2[5];
    aai[n][2] = ai2[4] + ai2[3];
    aai[n][3] = ai2[6] + ai2[1];
    aai[n][4] = ai2[6] - ai2[1];
    aai[n][5] = ai2[4] - ai2[3];
    aai[n][6] = ai2[2] - ai2[5];
    aai[n][7] = ai2[0] - ai2[7];
  }
  
  for( n = 0; n < 8; n++, pResidual++ )
  {
    Int ai1[8];
    Int ai2[8];
    
    ai1[0] =  aai[0][n]     +  aai[4][n];
    ai1[1] =  aai[5][n]     -  aai[3][n]     - aai[7][n] - (aai[7][n]>>1);
    ai1[2] =  aai[0][n]     -  aai[4][n];
    ai1[3] =  aai[1][n]     +  aai[7][n]     - aai[3][n] - (aai[3][n]>>1);
    ai1[4] = (aai[2][n]>>1) -  aai[6][n];
    ai1[5] =  aai[7][n]     -  aai[1][n]     + aai[5][n] + (aai[5][n]>>1);
    ai1[6] =  aai[2][n]     + (aai[6][n]>>1);
    ai1[7] =  aai[3][n]     +  aai[5][n]     + aai[1][n] + (aai[1][n]>>1);
    
    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];
    
    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];
    
    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);
    
    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];
    
    pResidual[        0] = xRound( ai2[0] + ai2[7] );
    pResidual[uiStride ] = xRound( ai2[2] + ai2[5] );
    pResidual[uiStride2] = xRound( ai2[4] + ai2[3] );
    pResidual[uiStride3] = xRound( ai2[6] + ai2[1] );
    pResidual[uiStride4] = xRound( ai2[6] - ai2[1] );
    pResidual[uiStride5] = xRound( ai2[4] - ai2[3] );
    pResidual[uiStride6] = xRound( ai2[2] - ai2[5] );
    pResidual[uiStride7] = xRound( ai2[0] - ai2[7] );
    
#ifdef TRANS_PRECISION_EXT
    pResidual[        0] =  (pResidual[        0]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride ] =  (pResidual[uiStride ]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride2] =  (pResidual[uiStride2]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride3] =  (pResidual[uiStride3]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride4] =  (pResidual[uiStride4]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride5] =  (pResidual[uiStride5]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride6] =  (pResidual[uiStride6]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride7] =  (pResidual[uiStride7]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xDeQuant2x2( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iDeScale = g_aiDequantCoef4[m_cQP.m_iRem];
  
  if( pSrcCoef[0] != 0 )
  {
    rplDstCoef[0] = pSrcCoef[0]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[0] = 0;
  }
  
  if( pSrcCoef[1] != 0 )
  {
    rplDstCoef[1] = pSrcCoef[1]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[1] = 0;
  }
  
  if( pSrcCoef[2] != 0 )
  {
    rplDstCoef[2] = pSrcCoef[2]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[2] = 0;
  }
  
  if( pSrcCoef[3] != 0 )
  {
    rplDstCoef[3] = pSrcCoef[3]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[3] = 0;
  }
}

Void TComTrQuant::xDeQuant4x4( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iLevel;
  Int iDeScale;
  
  for( Int n = 0; n < 16; n++ )
  {
    iLevel  = pSrcCoef[n];
    
    if( 0 != iLevel )
    {
      iDeScale = g_aiDequantCoef[m_cQP.m_iRem][n];
      
      rplDstCoef[n] = iLevel*iDeScale << m_cQP.m_iPer;
    }
    else
    {
      rplDstCoef[n] = 0;
    }
  }
}

Void TComTrQuant::xDeQuant8x8( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iLevel;
  Int iDeScale;
  
  Int iAdd = ( 1 << 5 ) >> m_cQP.m_iPer;
  
  for( Int n = 0; n < 64; n++ )
  {
    iLevel  = pSrcCoef[n];
    
    if( 0 != iLevel )
    {
      iDeScale = g_aiDequantCoef64[m_cQP.m_iRem][n];
      rplDstCoef[n]   = ( (iLevel*iDeScale*16 + iAdd) << m_cQP.m_iPer ) >> 6;
    }
    else
    {
      rplDstCoef[n] = 0;
    }
  }
}
#endif //!E243_CORE_TRANSFORMS

Void TComTrQuant::invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff )
{
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
    return;
  
  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
  const UInt uiStopTrMode = eTxt == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
  
  assert(1); // as long as quadtrees are not used for residual transform
  
  if( uiTrMode == uiStopTrMode )
  {
    UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
    if( eTxt != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
      {
        return;
      }
      uiWidth  <<= 1;
      uiHeight <<= 1;
    }
    Pel* pResi = rpcResidual + uiAddr;
#if INTRA_DST_TYPE_7
    invtransformNxN( eTxt, REG_DCT, pResi, uiStride, rpcCoeff, uiWidth, uiHeight );
#else
    invtransformNxN( pResi, uiStride, rpcCoeff, uiWidth, uiHeight );
#endif
  }
  else
  {
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                         , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth               , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
  }
}

// ------------------------------------------------------------------------------------------------
// Logical transform
// ------------------------------------------------------------------------------------------------

#if E243_CORE_TRANSFORMS
/** Wrapper function between HM interface and core NxN forward transform (2D) 
 *  \param piBlkResi input data (residual)
 *  \param psCoeff output data (transform coefficients)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
Void TComTrQuant::xT( UInt uiMode, Pel* piBlkResi, UInt uiStride, Long* psCoeff, Int iSize )
#else
Void TComTrQuant::xT( Pel* piBlkResi, UInt uiStride, Long* psCoeff, Int iSize )
#endif
{
#if MATRIX_MULT  
#if INTRA_DST_TYPE_7
  xTr(piBlkResi,psCoeff,uiStride,(UInt)iSize,uiMode);
#else
  xTr(piBlkResi,psCoeff,uiStride,(UInt)iSize);
#endif
#else
  Int j,k;
  if (iSize==4)
  {   
    short block[4][4];   
    short coeff[4][4];
    for (j=0; j<4; j++)
    {    
      memcpy(block[j],piBlkResi+j*uiStride,4*sizeof(short));      
    }
#if INTRA_DST_TYPE_7
    xTr4(block,coeff,uiMode);
#else
    xTr4(block,coeff);     
#endif
    for (j=0; j<4; j++)
    {    
      for (k=0; k<4; k++)
      {        
        psCoeff[j*4+k] = coeff[j][k];
      }    
    }    
  }
  else if (iSize==8)
  {
    short block[8][8];
    short coeff[8][8];

    for (j=0; j<8; j++)
    {    
      memcpy(block[j],piBlkResi+j*uiStride,8*sizeof(short));
    }

    xTr8(block,coeff);       
    for (j=0; j<8; j++)
    {    
      for (k=0; k<8; k++)
      {        
        psCoeff[j*8+k] = coeff[j][k];
      }    
    }
  }
  else if (iSize==16)
  {   
    short block[16][16];
    short coeff[16][16];

    for (j=0; j<16; j++)
    {    
      memcpy(block[j],piBlkResi+j*uiStride,16*sizeof(short));
    }
    xTr16(block,coeff);       
    for (j=0; j<16; j++)
    {    
      for (k=0; k<16; k++)
      {        
        psCoeff[j*16+k] = coeff[j][k];
      }    
    }
  }
  else if (iSize==32)
  {   
    short block[32][32];
    short coeff[32][32];

    for (j=0; j<32; j++)
    {    
      memcpy(block[j],piBlkResi+j*uiStride,32*sizeof(short));
    }
    xTr32(block,coeff);       
    for (j=0; j<32; j++)
    {    
      for (k=0; k<32; k++)
      {        
        psCoeff[j*32+k] = coeff[j][k];
      }    
    }
  }
#endif  
}

/** Wrapper function between HM interface and core NxN inverse transform (2D) 
 *  \param plCoef input data (transform coefficients)
 *  \param pResidual output data (residual)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if INTRA_DST_TYPE_7
Void TComTrQuant::xIT( UInt uiMode, Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
#else
Void TComTrQuant::xIT( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
#endif
{
#if MATRIX_MULT  
#if INTRA_DST_TYPE_7
  xITr(plCoef,pResidual,uiStride,(UInt)iSize,uiMode);
#else
  xITr(plCoef,pResidual,uiStride,(UInt)iSize);
#endif
#else
  Int j,k;
  if (iSize==4)
  {    
    short block[4][4];
    short coeff[4][4];

    for (j=0; j<4; j++)
    {    
      for (k=0; k<4; k++)
      {        
        coeff[j][k] = (short)plCoef[j*4+k];
      }    
    }
#if INTRA_DST_TYPE_7
    xITr4(coeff,block,uiMode);
#else
    xITr4(coeff,block);       
#endif
    for (j=0; j<4; j++)
    {    
      memcpy(pResidual+j*uiStride,block[j],4*sizeof(short));
    }    
  }
  else if (iSize==8)
  {
    short block[8][8];
    short coeff[8][8];

    for (j=0; j<8; j++)
    {    
      for (k=0; k<8; k++)
      {        
        coeff[j][k] = (short)plCoef[j*8+k];
      }    
    }
    xITr8(coeff,block);       
    for (j=0; j<8; j++)
    {    
      memcpy(pResidual+j*uiStride,block[j],8*sizeof(short));
    }
  }
  else if (iSize==16)
  {
    short block[16][16];
    short coeff[16][16];

    for (j=0; j<16; j++)
    {    
      for (k=0; k<16; k++)
      {        
        coeff[j][k] = (short)plCoef[j*16+k];
      }    
    }
    xITr16(coeff,block);       
    for (j=0; j<16; j++)
    {    
      memcpy(pResidual+j*uiStride,block[j],16*sizeof(short));
    }
  }

  else if (iSize==32)
  {
    short block[32][32];
    short coeff[32][32];

    for (j=0; j<32; j++)
    {    
      for (k=0; k<32; k++)
      {        
        coeff[j][k] = (short)plCoef[j*32+k];
      }    
    }
    xITr32(coeff,block);       
    for (j=0; j<32; j++)
    {    
      memcpy(pResidual+j*uiStride,block[j],32*sizeof(short));
    }   
  }
#endif  
}
#else

Void TComTrQuant::xT( Pel* piBlkResi, UInt uiStride, Long* psCoeff, Int iSize )
{
  switch( iSize )
  {
    case  2: xT2 ( piBlkResi, uiStride, psCoeff ); break;
    case  4: xT4 ( piBlkResi, uiStride, psCoeff ); break;
    case  8: xT8 ( piBlkResi, uiStride, psCoeff ); break;
    case 16: xT16( piBlkResi, uiStride, psCoeff ); break;
    case 32: xT32( piBlkResi, uiStride, psCoeff ); break;
    default: assert(0); break;
  }
}

Void TComTrQuant::xIT( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
{
  switch( iSize )
  {
    case  2: xIT2 ( plCoef, pResidual, uiStride ); break;
    case  4: xIT4 ( plCoef, pResidual, uiStride ); break;
    case  8: xIT8 ( plCoef, pResidual, uiStride ); break;
    case 16: xIT16( plCoef, pResidual, uiStride ); break;
    case 32: xIT32( plCoef, pResidual, uiStride ); break;
    default: assert(0); break;
  }
}
#endif //E243_CORE_TRANSFORMS
#if QC_MDCS
UInt TComTrQuant::getCurrLineNum(UInt uiScanIdx, UInt uiPosX, UInt uiPosY)
{
      UInt uiLineNum = 0;

      switch (uiScanIdx)
      {
        case SCAN_ZIGZAG:
          uiLineNum = uiPosY + uiPosX;
          break;
        case SCAN_HOR:
          uiLineNum = uiPosY;
          break;
        case SCAN_VER:
          uiLineNum = uiPosX;
          break;
      }

      return uiLineNum;
}
#endif

/** RDOQ with CABAC
 * \param pcCU pointer to coding unit structure
 * \param plSrcCoeff pointer to input buffer
 * \param piDstCoeff reference to pointer to output buffer
 * \param uiWidth block width
 * \param uiHeight block height
 * \param uiAbsSum reference to absolute sum of quantized transform coefficient
 * \param eTType plane type / luminance or chrominance
 * \param uiAbsPartIdx absolute partition index
 * \returns Void
 * Rate distortion optimized quantization for entropy
 * coding engines using probability models like CABAC
 */
Void TComTrQuant::xRateDistOptQuant                 ( TComDataCU*                     pcCU,
                                                      Long*                           plSrcCoeff,
                                                      TCoeff*&                        piDstCoeff,
                                                      UInt                            uiWidth,
                                                      UInt                            uiHeight,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Int    iQBits      = m_cQP.m_iBits;
  Double dTemp       = 0;
  
#if E243_CORE_TRANSFORMS
  UInt uiLog2TrSize = g_aucConvertToBit[ uiWidth ] + 2;
  UInt uiQ = g_auiQ[m_cQP.rem()];

#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;  
#endif
  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
  double dErrScale = (double)(1<<SCALE_BITS);                              // Compensate for scaling of bitcount in Lagrange cost function
  dErrScale = dErrScale*pow(2.0,-2.0*iTransformShift);                     // Compensate for scaling through forward transform
  dErrScale = dErrScale/(double)(uiQ*uiQ);                                 // Compensate for qp-dependent multiplier applied before calculating the Lagrange cost function
  dErrScale = dErrScale/(double)(1<<(2*g_uiBitIncrement));                   // Compensate for Lagrange multiplier that is tuned towards 8-bit input

  iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                   // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits
#else
  Bool   b64Flag     = false;
  Int    iQuantCoeff = 0;
  Int    iQpRem      = m_cQP.m_iRem;
  Bool   bExt8x8Flag = false;
  Double dNormFactor = 0;
  if( uiWidth == 4 && uiHeight == 4 )
  {
    dNormFactor = pow( 2., ( 2 * DQ_BITS + 19 ) );
    if( g_uiBitIncrement )
    {
      dNormFactor *=  1 << ( 2 * g_uiBitIncrement );
    }
    m_puiQuantMtx = &g_aiQuantCoef[ m_cQP.m_iRem ][ 0 ];
  }
  else if( uiWidth == 8 && uiHeight == 8 )
  {
    iQBits++;
    dNormFactor = pow( 2., ( 2 * Q_BITS_8 + 9 ) );
    if( g_uiBitIncrement )
    {
      dNormFactor *= 1 << ( 2 * g_uiBitIncrement );
    }
    m_puiQuantMtx = &g_aiQuantCoef64[ m_cQP.m_iRem ][ 0 ];
  }
  else if( uiWidth == 16 && uiHeight == 16 )
  {
    iQBits = ECore16Shift + m_cQP.per();
    dNormFactor = pow( 2., 21 );
    if( g_uiBitIncrement )
    {
      dNormFactor *=  1 << ( 2 * g_uiBitIncrement );
    }
    dTemp = estErr16x16[ iQpRem ] / dNormFactor;
    m_puiQuantMtx = ( &g_aiQuantCoef256[ m_cQP.m_iRem ][ 0 ] );
    bExt8x8Flag = true;
  }
  else if ( uiWidth == 32 && uiHeight == 32 )
  {
    iQBits = ECore32Shift + m_cQP.per();
    dNormFactor = pow( 2., 21 );
    if( g_uiBitIncrement )
    {
      dNormFactor *= 1 << ( 2 * g_uiBitIncrement );
    }
    dTemp = estErr32x32[ iQpRem ] / dNormFactor;
    m_puiQuantMtx = ( &g_aiQuantCoef1024[ m_cQP.m_iRem ][ 0 ] );
    bExt8x8Flag = true;
  }
  else
  {
    assert( 0 );
  }
#endif

#if E253
  UInt       uiGoRiceParam       = 0;
#endif
#if PCP_SIGMAP_SIMPLE_LAST
  UInt       uiLastScanPos       = 0;
#else
  UInt       uiMaxLineNum        = 0;
#endif
  Double     d64BlockUncodedCost = 0;
  const UInt uiLog2BlkSize       = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt uiMaxNumCoeff       = 1 << ( uiLog2BlkSize << 1 );
  const UInt uiNum4x4Blk         = max<UInt>( 1, uiMaxNumCoeff >> 4 );
#if QC_MDCS
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif //QC_MDCS
  
  Int  piCoeff      [ MAX_CU_SIZE * MAX_CU_SIZE ];
  Long plLevelDouble[ MAX_CU_SIZE * MAX_CU_SIZE ];
#if E253
  UInt puiEstParams [ 16384 ];

  ::memset( piDstCoeff,    0, sizeof(TCoeff) *   uiMaxNumCoeff        );
  ::memset( piCoeff,       0, sizeof(Int)    *   uiMaxNumCoeff        );
  ::memset( plLevelDouble, 0, sizeof(Long)   *   uiMaxNumCoeff        );
  ::memset( puiEstParams,  0, sizeof(UInt)   * ( uiMaxNumCoeff << 2 ) );

  UInt *puiOneCtx    = puiEstParams;
  UInt *puiAbsCtx    = puiEstParams +   uiMaxNumCoeff;
  UInt *puiAbsGoRice = puiEstParams + ( uiMaxNumCoeff << 1 );
  UInt *puiBaseCtx   = puiEstParams + ( uiMaxNumCoeff << 1 ) + uiMaxNumCoeff;
#else
  UInt puiOneCtx    [ MAX_CU_SIZE * MAX_CU_SIZE ];
  UInt puiAbsCtx    [ MAX_CU_SIZE * MAX_CU_SIZE ];
  UInt puiBaseCtx   [ ( MAX_CU_SIZE * MAX_CU_SIZE ) >> 4 ];
  
  
  ::memset( piDstCoeff,    0, sizeof(TCoeff) *   uiMaxNumCoeff        );
  ::memset( piCoeff,       0, sizeof(Int)    *   uiMaxNumCoeff        );
  ::memset( plLevelDouble, 0, sizeof(Long)   *   uiMaxNumCoeff        );
  ::memset( puiOneCtx,     0, sizeof(UInt)   *   uiMaxNumCoeff        );
  ::memset( puiAbsCtx,     0, sizeof(UInt)   *   uiMaxNumCoeff        );
  ::memset( puiBaseCtx,    0, sizeof(UInt)   * ( uiMaxNumCoeff >> 4 ) );
#endif

  //===== quantization =====
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
#if PCP_SIGMAP_SIMPLE_LAST
    UInt    uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanPos];  
#else
    UInt    uiBlkPos = g_auiFrameScanXY[ uiLog2BlkSize-1 ][ uiScanPos ];
#endif

    Long lLevelDouble = plSrcCoeff[ uiBlkPos ];

#if E243_CORE_TRANSFORMS
    dTemp = dErrScale;  
    lLevelDouble = abs(lLevelDouble * (Long)uiQ);   
#else 
    UInt    uiPosY   = uiBlkPos >> uiLog2BlkSize;
    UInt    uiPosX   = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    if      ( uiWidth == 4 ) dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if ( uiWidth == 8 ) dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    
    lLevelDouble = abs( lLevelDouble * ( Long )( b64Flag ? iQuantCoeff : m_puiQuantMtx[ uiBlkPos ] ) );
#endif    
    plLevelDouble[ uiBlkPos ] = lLevelDouble;
    UInt uiMaxAbsLevel = lLevelDouble >> iQBits;
    Bool bLowerInt     = ( ( lLevelDouble - Long( uiMaxAbsLevel << iQBits ) ) < Long( 1 << ( iQBits - 1 ) ) ) ? true : false;
    
    if( !bLowerInt )
    {
      uiMaxAbsLevel++;
    }
    
    Double dErr          = Double( lLevelDouble );
    d64BlockUncodedCost += dErr * dErr * dTemp;
    
    piCoeff[ uiBlkPos ] = plSrcCoeff[ uiBlkPos ] > 0 ? uiMaxAbsLevel : -Int( uiMaxAbsLevel );
    
    if ( uiMaxAbsLevel > 0 )
    {
#if PCP_SIGMAP_SIMPLE_LAST
      uiLastScanPos = uiScanPos;
#else
#if QC_MDCS
      UInt uiLineNum = getCurrLineNum(uiScanIdx, uiPosX, uiPosY);
#else
      UInt uiLineNum = uiPosY + uiPosX;
#endif //QC_MDCS
      
      if( uiLineNum > uiMaxLineNum )
      {
        uiMaxLineNum = uiLineNum;
      }
#endif
    }    
  }
  
#if PCP_SIGMAP_SIMPLE_LAST
  uiLastScanPos++;
#endif

  //===== estimate context models =====
  if ( uiNum4x4Blk > 1 )
  {
    Bool bFirstBlock  = true;
    UInt uiNumOne = 0;
    
    for( UInt uiSubBlk = 0; uiSubBlk < uiNum4x4Blk; uiSubBlk++ )
    {
      UInt uiCtxSet    = 0;
      UInt uiSubNumSig = 0;
      UInt uiSubPosX   = 0;
      UInt uiSubPosY   = 0;
#if E253
      uiGoRiceParam    = 0;
#endif

      uiSubPosX = g_auiFrameScanX[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
      uiSubPosY = g_auiFrameScanY[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
      
      Int* piCurr = &piCoeff[ uiSubPosX + uiSubPosY * uiWidth ];
      
      for( UInt uiY = 0; uiY < 4; uiY++ )
      {
        for( UInt uiX = 0; uiX < 4; uiX++ )
        {
          if( piCurr[ uiX ] )
          {
            uiSubNumSig++;
          }
        }
        piCurr += uiWidth;
      }
      
      if( uiSubNumSig > 0 )
      {
        Int c1 = 1;
        Int c2 = 0;
        UInt uiAbs  = 0;
        UInt uiSign = 0;
        
        if( bFirstBlock )
        {
          bFirstBlock = false;
          uiCtxSet = 5;
        }
        else
        {
          uiCtxSet = ( uiNumOne >> 2 ) + 1;
          uiNumOne = 0;
        }
        
        puiBaseCtx[ ( uiSubPosX >> 2 ) + ( uiSubPosY >> 2 ) * ( uiWidth >> 2 ) ] = uiCtxSet;
        
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = (uiSubPosY + uiPosY) * uiWidth + uiSubPosX + uiPosX;
          
          puiOneCtx[ uiIndex ] = min<UInt>( c1, 4 );
          puiAbsCtx[ uiIndex ] = min<UInt>( c2, 4 );
#if E253
          puiAbsGoRice[ uiIndex ] = uiGoRiceParam;
#endif
          
          if( piCoeff[ uiIndex ]  )
          {
            if( piCoeff[ uiIndex ] > 0) { uiAbs = static_cast<UInt>(  piCoeff[ uiIndex ] );  uiSign = 0; }
            else                        { uiAbs = static_cast<UInt>( -piCoeff[ uiIndex ] );  uiSign = 1; }
            
            UInt uiSymbol = uiAbs > 1 ? 1 : 0;
            
            if( uiSymbol )
            {
              c1 = 0; c2++;
              uiNumOne++;
#if E253
              if( uiAbs > 3 )
              {
                uiAbs -= 4;
                uiAbs  = min<UInt>( uiAbs, 15 );
                uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ uiAbs ];
              }
#endif
            }
            else if( c1 )
            {
              c1++;
            }
          }
        }
      }
    }
  }
  else
  {
    Int c1 = 1;
    Int c2 = 0;
    UInt uiAbs  = 0;
    UInt uiSign = 0;
    
    for ( UInt uiScanPos = 0; uiScanPos < uiWidth*uiHeight; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiWidth * uiHeight - uiScanPos - 1 ];
      
      puiOneCtx[ uiIndex ] = min<UInt>( c1, 4 );
      puiAbsCtx[ uiIndex ] = min<UInt>( c2, 4 );
#if E253
      puiAbsGoRice[ uiIndex ] = uiGoRiceParam;
#endif

      if( piCoeff[ uiIndex ]  )
      {
        if( piCoeff[ uiIndex ] > 0) { uiAbs = static_cast<UInt>(  piCoeff[ uiIndex ] );  uiSign = 0; }
        else                        { uiAbs = static_cast<UInt>( -piCoeff[ uiIndex ] );  uiSign = 1; }
        
        UInt uiSymbol = uiAbs > 1 ? 1 : 0;
        
        if( uiSymbol )
        {
          c1 = 0; c2++;
#if E253
          if( uiAbs > 3 )
          {
            uiAbs -= 4;
            uiAbs  = min<UInt>( uiAbs, 15 );
            uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ uiAbs ];
          }
#endif
        }
        else if( c1 )
        {
          c1++;
        }
      }
    }
  }
  
  Int     ui16CtxCbf        = 0;
  UInt    uiBestLastIdxP1   = 0;
  Double  d64BestCost       = 0;
  Double  d64BaseCost       = 0;
  Double  d64CodedCost      = 0;
  Double  d64UncodedCost    = 0;

  if( !pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && pcCU->getTransformIdx( uiAbsPartIdx ) == 0 )
  {
    ui16CtxCbf  = pcCU->getCtxQtRootCbf( uiAbsPartIdx );
    d64BestCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockRootCbpBits[ ui16CtxCbf ][ 0 ] );
    d64BaseCost = d64BestCost - xGetICost( m_pcEstBitsSbac->blockRootCbpBits[ ui16CtxCbf ][ 0 ] ) + xGetICost( m_pcEstBitsSbac->blockRootCbpBits[ ui16CtxCbf ][ 1 ] );
  }
  else
  {
    ui16CtxCbf  = pcCU->getCtxQtCbf( uiAbsPartIdx, eTType, pcCU->getTransformIdx( uiAbsPartIdx ) );
    ui16CtxCbf  = ( eTType ? eTType - 1 : eTType ) * NUM_QT_CBF_CTX + ui16CtxCbf;
    d64BestCost = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 0 ] );
    d64BaseCost = d64BestCost - xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 0 ] ) + xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 1 ] );
  }
  
#if PCP_SIGMAP_SIMPLE_LAST
  Double  d64CostLast        = 0;
  TCoeff  iLastCoeffLevel    = 0;
  UInt    uiBestNonZeroLevel = 0;
  UInt    uiBestLastBlkPos   = 0;

  for( UInt uiScanPos = 0; uiScanPos < uiLastScanPos; uiScanPos++ )
  {
    UInt   uiBlkPos     = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanPos];  
    UInt   uiPosY       = uiBlkPos >> uiLog2BlkSize;
    UInt   uiPosX       = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    UInt   uiCtxBase    = uiNum4x4Blk > 0 ? puiBaseCtx[ ( uiPosX >> 2 ) + ( uiPosY >> 2 ) * ( uiWidth >> 2 ) ] : 0;

#if E243_CORE_TRANSFORMS
    dTemp = dErrScale;
#else
    if      ( uiWidth == 4 ) dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if ( uiWidth == 8 ) dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
#endif

    UShort  uiCtxSig       = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth );
    UInt    uiMaxAbsLevel  = abs( piCoeff[ uiBlkPos ] );
#if E253
    UInt    uiLevel        = xGetCodedLevel( d64UncodedCost, d64CodedCost, d64CostLast, uiBestNonZeroLevel, plLevelDouble[ uiBlkPos ], uiMaxAbsLevel, uiCtxSig, puiOneCtx[ uiBlkPos ], puiAbsCtx[ uiBlkPos ], puiAbsGoRice[ uiBlkPos ], iQBits, dTemp, uiCtxBase );
#else
    UInt    uiLevel        = xGetCodedLevel( d64UncodedCost, d64CodedCost, d64CostLast, uiBestNonZeroLevel, plLevelDouble[ uiBlkPos ], uiMaxAbsLevel, uiCtxSig, puiOneCtx[ uiBlkPos ], puiAbsCtx[ uiBlkPos ], iQBits, dTemp, uiCtxBase );
#endif
    piDstCoeff[ uiBlkPos ] = plSrcCoeff[ uiBlkPos ] < 0 ? -Int( uiLevel ) : uiLevel;
    d64BaseCost           -= d64UncodedCost;

    if( uiBestNonZeroLevel != 0 )
    {
      d64CostLast        += d64BaseCost;
      d64CostLast        += uiScanIdx == SCAN_VER ? xGetRateLast( uiPosY, uiPosX ) : xGetRateLast( uiPosX, uiPosY );
      if( d64CostLast < d64BestCost )
      {
        d64BestCost       = d64CostLast;
        uiBestLastIdxP1   = uiScanPos + 1;
        uiBestLastBlkPos  = uiBlkPos;
        iLastCoeffLevel   = plSrcCoeff[ uiBlkPos ] < 0 ? -Int( uiBestNonZeroLevel ) : uiBestNonZeroLevel;
      }
    }
    d64BaseCost           += d64CodedCost;
  }
  if( uiBestLastBlkPos > 0)
  {
    piDstCoeff[ uiBestLastBlkPos ] = iLastCoeffLevel;
  }
#else
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
#if QC_MDCS
    UInt uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanPos];  
#else
    UInt   uiBlkPos     = g_auiFrameScanXY[ uiLog2BlkSize-1 ][ uiScanPos ];
#endif //QC_MDCS
    UInt   uiPosY       = uiBlkPos >> uiLog2BlkSize;
    UInt   uiPosX       = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    UInt   uiCtxBase    = uiNum4x4Blk > 0 ? puiBaseCtx[ ( uiPosX >> 2 ) + ( uiPosY >> 2 ) * ( uiWidth >> 2 ) ] : 0;
    
#if QC_MDCS
    UInt uiLineNum = getCurrLineNum(uiScanIdx, uiPosX, uiPosY);
    if ( uiLineNum > uiMaxLineNum )
    {
      break;
    }
#else 
    if( uiPosY + uiPosX > uiMaxLineNum )
    {
      break;
    }
#endif //QC_MDCS
    
    if      ( uiWidth == 4 ) dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if ( uiWidth == 8 ) dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    
    UShort  uiCtxSig                = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth );
    Bool    bLastScanPos            = ( uiScanPos == uiMaxNumCoeff - 1 );
#if E253
    UInt    uiLevel                 = xGetCodedLevel( d64UncodedCost, d64CodedCost, plLevelDouble[ uiBlkPos ], abs( piCoeff[ uiBlkPos ] ), bLastScanPos, uiCtxSig, puiOneCtx[ uiBlkPos ], puiAbsCtx[ uiBlkPos ], puiAbsGoRice[ uiBlkPos ], iQBits, dTemp, uiCtxBase );
#else
    UInt    uiLevel                 = xGetCodedLevel( d64UncodedCost, d64CodedCost, plLevelDouble[ uiBlkPos ], abs( piCoeff[ uiBlkPos ] ), bLastScanPos, uiCtxSig, puiOneCtx[ uiBlkPos ], puiAbsCtx[ uiBlkPos ], iQBits, dTemp, uiCtxBase );
#endif
    piDstCoeff[ uiBlkPos ]          = plSrcCoeff[ uiBlkPos ] < 0 ? -Int( uiLevel ) : uiLevel;
    d64BaseCost                    -= d64UncodedCost;
    d64BaseCost                    += d64CodedCost;
    
    if( uiLevel )
    {
      //----- check for last flag -----
      UShort  uiCtxLast             = getLastCtxInc( uiPosX, uiPosY, uiLog2BlkSize );
      Double  d64CostLastZero       = xGetICost( m_pcEstBitsSbac->lastBits[ uiCtxLast ][ 0 ] );
      Double  d64CostLastOne        = xGetICost( m_pcEstBitsSbac->lastBits[ uiCtxLast ][ 1 ] );
      Double  d64CurrIsLastCost     = d64BaseCost + d64CostLastOne;
      d64BaseCost                  += d64CostLastZero;
      
      if( d64CurrIsLastCost < d64BestCost )
      {
        d64BestCost       = d64CurrIsLastCost;
        uiBestLastIdxP1   = uiScanPos + 1;
      }
    }
  }
#endif
  
  //===== clean uncoded coefficients =====
  {
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
#if QC_MDCS
      UInt uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanPos];  
#else
      UInt uiBlkPos = g_auiFrameScanXY[ uiLog2BlkSize-1 ][ uiScanPos ];
#endif //QC_MDCS
      
      if( uiScanPos < uiBestLastIdxP1 )
      {
        uiAbsSum += abs( piDstCoeff[ uiBlkPos ] );
      }
      else
      {
        piDstCoeff[ uiBlkPos ] = 0;
      }      
    }
  }
}

#if !SONY_SIG_CTX
/** Context derivation process of coeff_abs_significant_flag
 * \param pcCoeff pointer to prior coded transform coefficients
 * \param uiPosX column of current scan position
 * \param uiPosY row of current scan position
 * \param uiLog2BlkSize log2 value of block size
 * \param uiStride stride of the block
 * \returns ctxInc for current scan position
 */
UInt TComTrQuant::getSigCtxInc    ( TCoeff*                         pcCoeff,
                                    const UInt                      uiPosX,
                                    const UInt                      uiPosY,
                                    const UInt                      uiLog2BlkSize,
                                    const UInt                      uiStride )
{
  UInt  uiCtxInc  = 0;
  UInt  uiSizeM1  = ( 1 << uiLog2BlkSize ) - 1;
  if( uiLog2BlkSize <= 3 )
  {
    UInt  uiShift = uiLog2BlkSize > 2 ? uiLog2BlkSize - 2 : 0;
    uiCtxInc      = ( ( uiPosY >> uiShift ) << 2 ) + ( uiPosX >> uiShift );
  }
  else if( uiPosX <= 1 && uiPosY <= 1 )
  {
    uiCtxInc            = ( uiPosY << 1 ) + uiPosX;
  }
  else if( uiPosY == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    UInt        uiCnt   = ( pData[         -1 ] ? 1 : 0 );
    uiCnt              += ( pData[         -2 ] ? 1 : 0 );
    uiCnt              += ( pData[ iStride -2 ] ? 1 : 0 );
    if( ! (uiPosX & 1) )
    {
      uiCnt            += ( pData[ iStride -1 ] ? 1 : 0 );
    }
    uiCtxInc            = 4 + ( ( uiCnt + 1 ) >> 1 );
  }
  else if( uiPosX == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[  -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[  -iStride2 ] ? 1 : 0 );
    uiCnt              += ( pData[ 1-iStride2 ] ? 1 : 0 );
    if( uiPosY & 1 )
    {
      uiCnt            += ( pData[ 1-iStride  ] ? 1 : 0 );
    }
    uiCtxInc            = 7 + ( ( uiCnt + 1 ) >> 1 );
  }
  else
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[    -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[ -1           ] ? 1 : 0 );
    uiCnt              += ( pData[ -1 -iStride  ] ? 1 : 0 );
    if( uiPosX > 1 )
    {
      uiCnt            += ( pData[ -2           ] ? 1 : 0 );
      uiCnt            += ( pData[ -2 -iStride  ] ? 1 : 0 );
      if( uiPosY < uiSizeM1 )
      {
        uiCnt          += ( pData[ -2 +iStride  ] ? 1 : 0 );
      }
    }
    if( uiPosY > 1 )
    {
      uiCnt            += ( pData[    -iStride2 ] ? 1 : 0 );
      uiCnt            += ( pData[ -1 -iStride2 ] ? 1 : 0 );
      if( uiPosX < uiSizeM1 )
      {
        uiCnt          += ( pData[  1 -iStride2 ] ? 1 : 0 );
      }
    }
    if( (uiPosX + uiPosY) & 1 )
    {
      if( uiPosX < uiSizeM1 )
      {
        uiCnt          += ( pData[  1 -iStride  ] ? 1 : 0 );
      }
    }
    else
    {
      if( uiPosY < uiSizeM1 )
      {
        uiCnt          += ( pData[ -1 +iStride  ] ? 1 : 0 );
      }
    }
    uiCtxInc      = 10 + min<UInt>( 4, ( uiCnt + 1 ) >> 1 );
  }
  return uiCtxInc;
}
#else
/** Context derivation process of coeff_abs_significant_flag
 * \param pcCoeff pointer to prior coded transform coefficients
 * \param uiPosX column of current scan position
 * \param uiPosY row of current scan position
 * \param uiLog2BlkSize log2 value of block size
 * \param uiStride stride of the block
 * \returns ctxInc for current scan position
 */
UInt TComTrQuant::getSigCtxInc    ( TCoeff*                         pcCoeff,
                                    const UInt                      uiPosX,
                                    const UInt                      uiPosY,
                                    const UInt                      uiLog2BlkSize,
                                    const UInt                      uiStride )
{
  UInt  uiCtxInc  = 0;
  
  if( uiLog2BlkSize <= 3 )
  {
    UInt  uiShift = uiLog2BlkSize > 2 ? uiLog2BlkSize - 2 : 0;
    uiCtxInc      = ( ( uiPosY >> uiShift ) << 2 ) + ( uiPosX >> uiShift );
  }
  else if( uiPosX <= 1 && uiPosY <= 1 )
  {
    uiCtxInc            = ( uiPosY << 1 ) + uiPosX;
  }
  else if( uiPosY == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    UInt        uiCnt   = ( pData[         -1 ] ? 1 : 0 );
    uiCnt              += ( pData[         -2 ] ? 1 : 0 );
    uiCtxInc            = 4 + uiCnt;
  }
  else if( uiPosX == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[  -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[  -iStride2 ] ? 1 : 0 );
    uiCtxInc            = 7 + uiCnt;
  }
  else
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[ -1 -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[    -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[ -1           ] ? 1 : 0 );
    if( uiPosX > 1 )
    {
      uiCnt          += ( pData[ -2           ] ? 1 : 0 );
    }
    if ( uiPosY > 1 )
    {
      uiCnt          += ( pData[    -iStride2 ] ? 1 : 0 );
    }
    uiCtxInc            = 10 + min<UInt>( 4, uiCnt);
  }
  return uiCtxInc;
}
#endif

#if !PCP_SIGMAP_SIMPLE_LAST
/** Context derivation of coeff_abs_last_significant_flag
 * \param uiPosX column of current scan position
 * \param uiPosY row of current scan position
 * \param uiLog2BlkSize log2 value of block size
 * \returns ctxInc for current scan position
 */
UInt TComTrQuant::getLastCtxInc   ( const UInt                      uiPosX,
                                    const UInt                      uiPosY,
                                    const UInt                      uiLog2BlkSize )
{
  if( uiLog2BlkSize <= 2 )
  {
    return ( uiPosY << 2 ) + uiPosX;
  }
  else
  {
    return ( uiPosX + uiPosY ) >> ( uiLog2BlkSize - 3 );
  }
}
#endif

#if E253 && PCP_SIGMAP_SIMPLE_LAST // only valid if both tools are enabled
/** Get the best level in RD sense
 * \param rd64UncodedCost reference to uncoded cost
 * \param rd64CodedCost reference to current coded cost
 * \param rd64CodedLastCost reference to coded cost of coefficient without the significance cost
 * \param uiBestNonZeroLevel !!! not available yet
 * \param lLevelDouble reference to unscaled quantized level
 * \param uiMaxAbsLevel scaled quantized level
 * \param ui16CtxNumSig current ctxInc for coeff_abs_significant_flag
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_greater2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param ui16AbsGoRice current Rice parameter for coeff_abs_level_minus3
 * \param iQBits quantization step size
 * \param dTemp correction factor
 * \param ui16CtxBase current global offset for coeff_abs_level_greater1 and coeff_abs_level_greater2
 * \returns best quantized transform level for given scan position
 * This method calculates the best quantized transform level for a given scan position.
 */
#else
/** Get the best level in RD sense
 * \param rd64UncodedCost reference to uncoded cost
 * \param rd64CodedCost reference to current coded cost
 * \param lLevelDouble reference to unscaled quantized level
 * \param uiMaxAbsLevel scaled quantized level
 * \param bLastScanPos last scan position
 * \param ui16CtxNumSig current ctxInc for coeff_abs_significant_flag
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_minus2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param iQBits quantization step size
 * \param dTemp correction factor
 * \param ui16CtxBase current global offset for coeff_abs_level_greater1 and coeff_abs_level_minus2
 * \returns best quantized transform level for given scan position
 * This method calculates the best quantized transform level for a given scan position.
 */
#endif
__inline UInt TComTrQuant::xGetCodedLevel  ( Double&                         rd64UncodedCost,
                                             Double&                         rd64CodedCost,
#if PCP_SIGMAP_SIMPLE_LAST
                                             Double&                         rd64CodedLastCost,
                                             UInt&                           ruiBestNonZeroLevel,
                                             Long                            lLevelDouble,
                                             UInt                            uiMaxAbsLevel,
#else
                                             Long                            lLevelDouble,
                                             UInt                            uiMaxAbsLevel,
                                             bool                            bLastScanPos,
#endif
                                             UShort                          ui16CtxNumSig,
                                             UShort                          ui16CtxNumOne,
                                             UShort                          ui16CtxNumAbs,
#if E253
                                             UShort                          ui16AbsGoRice,
#endif
                                             Int                             iQBits,
                                             Double                          dTemp,
                                             UShort                          ui16CtxBase   ) const
{
  UInt   uiBestAbsLevel = 0;
  Double dErr1          = Double( lLevelDouble );
  
  rd64UncodedCost = dErr1 * dErr1 * dTemp;
#if PCP_SIGMAP_SIMPLE_LAST
  rd64CodedCost   = rd64UncodedCost + xGetRateSigCoef( 0, ui16CtxNumSig );

  ruiBestNonZeroLevel   = 0;
  if( uiMaxAbsLevel )
  {
    UInt uiAbsLevel     = uiMaxAbsLevel;
    ruiBestNonZeroLevel = uiMaxAbsLevel;
    Double dErr         = Double( lLevelDouble - Long( uiAbsLevel << iQBits ) );
#if E253
    rd64CodedLastCost   = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice, ui16CtxBase );
#else
    rd64CodedLastCost   = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
#endif
  }
  else
  {
    return uiBestAbsLevel;
  }

  UInt uiAbsLevel = ( uiMaxAbsLevel > 1 ? uiMaxAbsLevel - 1 : 1 );
  if( uiAbsLevel != uiMaxAbsLevel )
  {
    Double dErr        = Double( lLevelDouble - Long( uiAbsLevel << iQBits ) );
#if E253
    Double dCurrCost   = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice, ui16CtxBase );
#else
    Double dCurrCost   = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
#endif
    if( dCurrCost < rd64CodedLastCost )
    {
      ruiBestNonZeroLevel = uiAbsLevel;
      rd64CodedLastCost   = dCurrCost;
    }  
  }

  Double dCurrCost = rd64CodedLastCost + xGetRateSigCoef( 1, ui16CtxNumSig );

  if( dCurrCost < rd64CodedCost )
  {
    uiBestAbsLevel  = ruiBestNonZeroLevel;
    rd64CodedCost   = dCurrCost;
  }
#else
#if E253
  rd64CodedCost   = rd64UncodedCost + xGetICRateCost( 0, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice, ui16CtxBase );
#else
  rd64CodedCost   = rd64UncodedCost + xGetICRateCost( 0, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
#endif

  UInt uiMinAbsLevel = ( uiMaxAbsLevel > 1 ? uiMaxAbsLevel - 1 : 1 );
  for( UInt uiAbsLevel = uiMaxAbsLevel; uiAbsLevel >= uiMinAbsLevel ; uiAbsLevel-- )
  {
    Double i64Delta  = Double( lLevelDouble  - Long( uiAbsLevel << iQBits ) );
    Double dErr      = Double( i64Delta );
#if E253
    Double dCurrCost = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice, ui16CtxBase );
#else
    Double dCurrCost = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
#endif

    if( dCurrCost < rd64CodedCost )
    {
      uiBestAbsLevel  = uiAbsLevel;
      rd64CodedCost   = dCurrCost;
    }
  }
#endif
  return uiBestAbsLevel;
}

#if E253 && PCP_SIGMAP_SIMPLE_LAST // only valid if both tools are enabled
/** Calculates the cost for specific absolute transform level
 * \param uiAbsLevel scaled quantized level
 * \param bLastScanPos last scan position
 * \param ui16CtxNumSig current ctxInc for coeff_abs_significant_flag
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_greater2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param ui16AbsGoRice Rice parameter for coeff_abs_level_minus3
 * \param ui16CtxBase current global offset for coeff_abs_level_greater1 and coeff_abs_level_greater2
 * \returns cost of given absolute transform level
 */
#endif
__inline Double TComTrQuant::xGetICRateCost  ( UInt                            uiAbsLevel,
#if !PCP_SIGMAP_SIMPLE_LAST
                                               Bool                            bLastScanPos,
                                               UShort                          ui16CtxNumSig,
#endif
                                               UShort                          ui16CtxNumOne,
                                               UShort                          ui16CtxNumAbs,
#if E253
                                               UShort                          ui16AbsGoRice,
#endif
                                               UShort                          ui16CtxBase   ) const
{
#if PCP_SIGMAP_SIMPLE_LAST
  Double iRate = xGetIEPRate();
#else
  if( uiAbsLevel == 0 )
  {
    Double iRate = 0;
    if( !bLastScanPos )
    {
      iRate += m_pcEstBitsSbac->significantBits[ ui16CtxNumSig ][ 0 ];
    }
    return xGetICost( iRate );
  }
  Double iRate = xGetIEPRate();
  if( !bLastScanPos )
  {
    iRate += m_pcEstBitsSbac->significantBits[ ui16CtxNumSig ][ 1 ];
  }
#endif
  if( uiAbsLevel == 1 )
  {
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 0 ];
  }
#if E253
  else if( uiAbsLevel == 2 )
  {
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 0 ];
  }
  else
  {
    UInt uiSymbol     = uiAbsLevel - 3;
    UInt uiMaxVlc     = g_auiGoRiceRange[ ui16AbsGoRice ];
    Bool bExpGolomb   = ( uiSymbol > uiMaxVlc );

    if( bExpGolomb )
    {
      uiAbsLevel  = uiSymbol - uiMaxVlc;
      int iEGS    = 1;  for( UInt uiMax = 2; uiAbsLevel >= uiMax; uiMax <<= 1, iEGS += 2 );
      iRate      += iEGS << 15;
      uiSymbol    = min<UInt>( uiSymbol, ( uiMaxVlc + 1 ) );
    }

    UShort ui16PrefLen = UShort( uiSymbol >> ui16AbsGoRice ) + 1;
    UShort ui16NumBins = min<UInt>( ui16PrefLen, g_auiGoRicePrefixLen[ ui16AbsGoRice ] ) + ui16AbsGoRice;

    iRate += ui16NumBins << 15;
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 1 ];
  }
#else
  else if( uiAbsLevel < 15 )
  {
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 0 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 1 ] * (int)( uiAbsLevel - 2 );
  }
  else
  {
    uiAbsLevel -= 14;
    int iEGS    = 1;  for( UInt uiMax = 2; uiAbsLevel >= uiMax; uiMax <<= 1, iEGS += 2 );
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 1 ] * 13;
    iRate += xGetIEPRate() * iEGS;
  }
#endif
  return xGetICost( iRate );
}

#if PCP_SIGMAP_SIMPLE_LAST
/** Calculates the cost of signaling the last significant coefficient in the block
 * \param uiPosX X coordinate of the last significant coefficient
 * \param uiPosY Y coordinate of the last significant coefficient
 * \returns cost of last significant coefficient
 */
__inline Double TComTrQuant::xGetRateLast   ( UInt                            uiPosX,
                                              UInt                            uiPosY ) const
{
  return xGetICost( m_pcEstBitsSbac->lastXBits[ uiPosX ] + m_pcEstBitsSbac->lastYBits[ uiPosY ] );
}

 /** Calculates the cost for specific absolute transform level
 * \param uiAbsLevel scaled quantized level
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_greater2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxBase current global offset for coeff_abs_level_greater1 and coeff_abs_level_greater2
 * \returns cost of given absolute transform level
 */
__inline Double TComTrQuant::xGetRateSigCoef  ( UShort                          uiSignificance,
                                                UShort                          ui16CtxNumSig ) const
{
  return xGetICost( m_pcEstBitsSbac->significantBits[ ui16CtxNumSig ][ uiSignificance ] );
}
#endif

/** Get the cost for a specific rate
 * \param dRate rate of a bit
 * \returns cost at the specific rate
 */
__inline Double TComTrQuant::xGetICost        ( Double                          dRate         ) const
{
  return m_dLambda * dRate;
}

/** Get the cost of an equal probable bit
 * \returns cost of equal probable bit
 */
__inline Double TComTrQuant::xGetIEPRate      (                                               ) const
{
  return 32768;
}
