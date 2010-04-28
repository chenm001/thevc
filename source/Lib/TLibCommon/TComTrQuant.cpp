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

/** \file			TComTrQuant.cpp
    \brief		transform and quantization class
*/

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "TComTrQuant.h"
#include "ContextTables.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define LTR_Q_OFFSET_I              (1.0/3.0)		///< LTR quantization offset of I slice
#define LTR_Q_OFFSET_P              (1.0/6.0)		///< LTR quantization offset of P slice
#define LTR_Q_OFFSET_B              (1.0/6.0)		///< LTR quantization offset of B slice
#define RDOQ_CHROMA                 1						///< use of RDOQ in chroma
#define RDOQ_ROT_IDX0_ONLY          0						///< use of RDOQ with ROT

#define DQ_BITS											6
#define Q_BITS_8										16
#define SIGN_BITS										1

// ====================================================================================================================
// Tables
// ====================================================================================================================

UChar stateMappingTable[113]=
// Modified state mapping table
{0, 16, 31, 43, 54, 62, 62, 62, 62, 62, 62, 62, 62, 62, 0, 7, 13, 19, 25, 30, 35, 40, 45, 49, 53, 57, 60, 62, 62, 62, 62, 62, 62, 62, 62, 62, 0, 4, 8, 12, 16, 20, 23, 27, 30, 33, 37, 40, 42, 45, 48, 50, 53, 55, 57, 59, 61, 62, 62, 62, 62, 62, 62, 62, 0, 3, 6, 9, 12, 15, 18, 20, 23, 25, 28, 30, 32, 35, 37, 39, 1, 3, 6, 8, 10, 12, 15, 17, 1, 3, 5, 7, 9, 11, 13, 1, 3, 4, 6, 8, 9, 2, 3, 5, 6, 1, 2, 4, 1, 2, 0, 1, 0};

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

static const Int estErr16x16[6] = { 25329, 30580, 42563, 49296, 64244, 82293 };
static const Int estErr32x32[6] = { 25351, 30674, 42843, 49687, 64898, 82136 };
static const Int estErr64x64[6] = { 102400, 123904, 173056, 200704, 262144, 331776 };

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
  for (UInt uiSliceType = 0; uiSliceType < 3; uiSliceType++)
  {
    Int k =  (iQP + 6*g_uiBitIncrement)/6;

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

    iPer = ECore64Shift + k - QOFFSET_BITS_LTR;
    m_aiAdd64x64[iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
    m_aiAddNxN  [iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
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

	// allocate bit estimation class	(for RDOQ)
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

Void TComTrQuant::xT64  ( Pel* pSrc, UInt uiStride, Long* pDes )
{
  Int x, y;
  static Long aaiTemp[64][64];

  Long O0, O1, O2, O3, O4, O5, O6, O7, O8, O9, O10, O11, O12, O13, O14, O15, O16, O17, O18, O19, O20, O21, O22, O23, O24, O25, O26, O27, O28, O29, O30, O31, O32;
  Long O33, O34, O35, O36, O37, O38, O39, O40, O41, O42, O43, O44, O45, O46, O47, O48, O49, O50, O51, O52, O53, O54, O55, O56, O57, O58, O59, O60, O61, O62, O63;
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31;
  Long A40, A41, A42, A43, A44, A45, A46, A47, A48, A49, A50, A51, A52, A53, A54, A55;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B20, B21, B22, B23, B24, B25, B26, B27, B32;
  Long B33, B34, B35, B36, B37, B38, B39, B40, B41, B42, B43, B44, B45, B46, B47, B48, B49, B50, B51, B52, B53, B54, B55, B56, B57, B58, B59, B60, B61, B62, B63;
  Long C0, C1, C2, C3, C4, C5, C6, C7, C10, C11, C12, C13, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31;
  Long C36, C37, C38, C39, C40, C41, C42, C43, C52, C53, C54, C55, C56, C57, C58, C59;
  Long D0, D1, D2, D3, D5, D6, D8, D9, D10, D11, D12, D13, D14, D15, D18, D19, D20, D21, D26, D27, D28, D29, D32;
  Long D33, D34, D35, D36, D37, D38, D39, D40, D41, D42, D43, D44, D45, D46, D47, D48, D49, D50, D51, D52, D53, D54, D55, D56, D57, D58, D59, D60, D61, D62, D63;
  Long E4, E5, E6, E7, E9, E10, E13, E14, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31;
  Long E34, E35, E36, E37, E42, E43, E44, E45, E50, E51, E52, E53, E58, E59, E60, E61;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F17, F18, F21, F22, F25, F26, F29, F30, F32;
  Long F33, F34, F35, F36, F37, F38, F39, F40, F41, F42, F43, F44, F45, F46, F47, F48, F49, F50, F51, F52, F53, F54, F55, F56, F57, F58, F59, F60, F61, F62, F63;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31;
  Long G33, G34, G37, G38, G41, G42, G45, G46, G49, G50, G53, G54, G57, G58, G61, G62;
  Long H32;
  Long H33, H34, H35, H36, H37, H38, H39, H40, H41, H42, H43, H44, H45, H46, H47, H48, H49, H50, H51, H52, H53, H54, H55, H56, H57, H58, H59, H60, H61, H62, H63;

  for( y=0 ; y<64 ; y++ )
  {
    O0 = pSrc[0]+pSrc[63];
    O63 = pSrc[0]-pSrc[63];
    O1 = pSrc[1]+pSrc[62];
    O62 = pSrc[1]-pSrc[62];
    O2 = pSrc[2]+pSrc[61];
    O61 = pSrc[2]-pSrc[61];
    O3 = pSrc[3]+pSrc[60];
    O60 = pSrc[3]-pSrc[60];
    O4 = pSrc[4]+pSrc[59];
    O59 = pSrc[4]-pSrc[59];
    O5 = pSrc[5]+pSrc[58];
    O58 = pSrc[5]-pSrc[58];
    O6 = pSrc[6]+pSrc[57];
    O57 = pSrc[6]-pSrc[57];
    O7 = pSrc[7]+pSrc[56];
    O56 = pSrc[7]-pSrc[56];
    O8 = pSrc[8]+pSrc[55];
    O55 = pSrc[8]-pSrc[55];
    O9 = pSrc[9]+pSrc[54];
    O54 = pSrc[9]-pSrc[54];
    O10 = pSrc[10]+pSrc[53];
    O53 = pSrc[10]-pSrc[53];
    O11 = pSrc[11]+pSrc[52];
    O52 = pSrc[11]-pSrc[52];
    O12 = pSrc[12]+pSrc[51];
    O51 = pSrc[12]-pSrc[51];
    O13 = pSrc[13]+pSrc[50];
    O50 = pSrc[13]-pSrc[50];
    O14 = pSrc[14]+pSrc[49];
    O49 = pSrc[14]-pSrc[49];
    O15 = pSrc[15]+pSrc[48];
    O48 = pSrc[15]-pSrc[48];
    O16 = pSrc[16]+pSrc[47];
    O47 = pSrc[16]-pSrc[47];
    O17 = pSrc[17]+pSrc[46];
    O46 = pSrc[17]-pSrc[46];
    O18 = pSrc[18]+pSrc[45];
    O45 = pSrc[18]-pSrc[45];
    O19 = pSrc[19]+pSrc[44];
    O44 = pSrc[19]-pSrc[44];
    O20 = pSrc[20]+pSrc[43];
    O43 = pSrc[20]-pSrc[43];
    O21 = pSrc[21]+pSrc[42];
    O42 = pSrc[21]-pSrc[42];
    O22 = pSrc[22]+pSrc[41];
    O41 = pSrc[22]-pSrc[41];
    O23 = pSrc[23]+pSrc[40];
    O40 = pSrc[23]-pSrc[40];
    O24 = pSrc[24]+pSrc[39];
    O39 = pSrc[24]-pSrc[39];
    O25 = pSrc[25]+pSrc[38];
    O38 = pSrc[25]-pSrc[38];
    O26 = pSrc[26]+pSrc[37];
    O37 = pSrc[26]-pSrc[37];
    O27 = pSrc[27]+pSrc[36];
    O36 = pSrc[27]-pSrc[36];
    O28 = pSrc[28]+pSrc[35];
    O35 = pSrc[28]-pSrc[35];
    O29 = pSrc[29]+pSrc[34];
    O34 = pSrc[29]-pSrc[34];
    O30 = pSrc[30]+pSrc[33];
    O33 = pSrc[30]-pSrc[33];
    O31 = pSrc[31]+pSrc[32];
    O32 = pSrc[31]-pSrc[32];

    A0 = O0+O31;
    A31 = O0-O31;
    A1 = O1+O30;
    A30 = O1-O30;
    A2 = O2+O29;
    A29 = O2-O29;
    A3 = O3+O28;
    A28 = O3-O28;
    A4 = O4+O27;
    A27 = O4-O27;
    A5 = O5+O26;
    A26 = O5-O26;
    A6 = O6+O25;
    A25 = O6-O25;
    A7 = O7+O24;
    A24 = O7-O24;
    A8 = O8+O23;
    A23 = O8-O23;
    A9 = O9+O22;
    A22 = O9-O22;
    A10 = O10+O21;
    A21 = O10-O21;
    A11 = O11+O20;
    A20 = O11-O20;
    A12 = O12+O19;
    A19 = O12-O19;
    A13 = O13+O18;
    A18 = O13-O18;
    A14 = O14+O17;
    A17 = O14-O17;
    A15 = O15+O16;
    A16 = O15-O16;
    A40 = xTrRound(724*(O55-O40), DenShift64);
    A55 = xTrRound(724*(O55+O40), DenShift64);
    A41 = xTrRound(724*(O54-O41), DenShift64);
    A54 = xTrRound(724*(O54+O41), DenShift64);
    A42 = xTrRound(724*(O53-O42), DenShift64);
    A53 = xTrRound(724*(O53+O42), DenShift64);
    A43 = xTrRound(724*(O52-O43), DenShift64);
    A52 = xTrRound(724*(O52+O43), DenShift64);
    A44 = xTrRound(724*(O51-O44), DenShift64);
    A51 = xTrRound(724*(O51+O44), DenShift64);
    A45 = xTrRound(724*(O50-O45), DenShift64);
    A50 = xTrRound(724*(O50+O45), DenShift64);
    A46 = xTrRound(724*(O49-O46), DenShift64);
    A49 = xTrRound(724*(O49+O46), DenShift64);
    A47 = xTrRound(724*(O48-O47), DenShift64);
    A48 = xTrRound(724*(O48+O47), DenShift64);

    B0 = A0+A15;
    B15 = A0-A15;
    B1 = A1+A14;
    B14 = A1-A14;
    B2 = A2+A13;
    B13 = A2-A13;
    B3 = A3+A12;
    B12 = A3-A12;
    B4 = A4+A11;
    B11 = A4-A11;
    B5 = A5+A10;
    B10 = A5-A10;
    B6 = A6+A9;
    B9 = A6-A9;
    B7 = A7+A8;
    B8 = A7-A8;
    B20 = xTrRound(724*(A27-A20), DenShift64);
    B27 = xTrRound(724*(A27+A20), DenShift64);
    B21 = xTrRound(724*(A26-A21), DenShift64);
    B26 = xTrRound(724*(A26+A21), DenShift64);
    B22 = xTrRound(724*(A25-A22), DenShift64);
    B25 = xTrRound(724*(A25+A22), DenShift64);
    B23 = xTrRound(724*(A24-A23), DenShift64);
    B24 = xTrRound(724*(A24+A23), DenShift64);
    B32 = O32+A47;
    B47 = O32-A47;
    B48 = O63-A48;
    B63 = O63+A48;
    B33 = O33+A46;
    B46 = O33-A46;
    B49 = O62-A49;
    B62 = O62+A49;
    B34 = O34+A45;
    B45 = O34-A45;
    B50 = O61-A50;
    B61 = O61+A50;
    B35 = O35+A44;
    B44 = O35-A44;
    B51 = O60-A51;
    B60 = O60+A51;
    B36 = O36+A43;
    B43 = O36-A43;
    B52 = O59-A52;
    B59 = O59+A52;
    B37 = O37+A42;
    B42 = O37-A42;
    B53 = O58-A53;
    B58 = O58+A53;
    B38 = O38+A41;
    B41 = O38-A41;
    B54 = O57-A54;
    B57 = O57+A54;
    B39 = O39+A40;
    B40 = O39-A40;
    B55 = O56-A55;
    B56 = O56+A55;

    C0 = B0+B7;
    C7 = B0-B7;
    C1 = B1+B6;
    C6 = B1-B6;
    C2 = B2+B5;
    C5 = B2-B5;
    C3 = B3+B4;
    C4 = B3-B4;
    C10 = xTrRound(724*(B13-B10), DenShift64);
    C13 = xTrRound(724*(B13+B10), DenShift64);
    C11 = xTrRound(724*(B12-B11), DenShift64);
    C12 = xTrRound(724*(B12+B11), DenShift64);
    C16 = A16+B23;
    C23 = A16-B23;
    C24 = A31-B24;
    C31 = A31+B24;
    C17 = A17+B22;
    C22 = A17-B22;
    C25 = A30-B25;
    C30 = A30+B25;
    C18 = A18+B21;
    C21 = A18-B21;
    C26 = A29-B26;
    C29 = A29+B26;
    C19 = A19+B20;
    C20 = A19-B20;
    C27 = A28-B27;
    C28 = A28+B27;
    C36 = xTrRound(392*B59-946*B36, DenShift64);
    C40 = xTrRound(-946*B55-392*B40, DenShift64);
    C52 = xTrRound(-946*B43+392*B52, DenShift64);
    C56 = xTrRound(392*B39+946*B56, DenShift64);
    C37 = xTrRound(392*B58-946*B37, DenShift64);
    C41 = xTrRound(-946*B54-392*B41, DenShift64);
    C53 = xTrRound(-946*B42+392*B53, DenShift64);
    C57 = xTrRound(392*B38+946*B57, DenShift64);
    C38 = xTrRound(392*B57-946*B38, DenShift64);
    C42 = xTrRound(-946*B53-392*B42, DenShift64);
    C54 = xTrRound(-946*B41+392*B54, DenShift64);
    C58 = xTrRound(392*B37+946*B58, DenShift64);
    C39 = xTrRound(392*B56-946*B39, DenShift64);
    C43 = xTrRound(-946*B52-392*B43, DenShift64);
    C55 = xTrRound(-946*B40+392*B55, DenShift64);
    C59 = xTrRound(392*B36+946*B59, DenShift64);

    D0 = C0+C3;
    D3 = C0-C3;
    D8 = B8+C11;
    D11 = B8-C11;
    D12 = B15-C12;
    D15 = B15+C12;
    D1 = C1+C2;
    D2 = C1-C2;
    D9 = B9+C10;
    D10 = B9-C10;
    D13 = B14-C13;
    D14 = B14+C13;
    D5 = xTrRound(724*(C6-C5), DenShift64);
    D6 = xTrRound(724*(C6+C5), DenShift64);
    D18 = xTrRound(392*C29-946*C18, DenShift64);
    D20 = xTrRound(-946*C27-392*C20, DenShift64);
    D26 = xTrRound(-946*C21+392*C26, DenShift64);
    D28 = xTrRound(392*C19+946*C28, DenShift64);
    D19 = xTrRound(392*C28-946*C19, DenShift64);
    D21 = xTrRound(-946*C26-392*C21, DenShift64);
    D27 = xTrRound(-946*C20+392*C27, DenShift64);
    D29 = xTrRound(392*C18+946*C29, DenShift64);
    D32 = B32+C39;
    D39 = B32-C39;
    D40 = B47-C40;
    D47 = B47+C40;
    D48 = B48+C55;
    D55 = B48-C55;
    D56 = B63-C56;
    D63 = B63+C56;
    D33 = B33+C38;
    D38 = B33-C38;
    D41 = B46-C41;
    D46 = B46+C41;
    D49 = B49+C54;
    D54 = B49-C54;
    D57 = B62-C57;
    D62 = B62+C57;
    D34 = B34+C37;
    D37 = B34-C37;
    D42 = B45-C42;
    D45 = B45+C42;
    D50 = B50+C53;
    D53 = B50-C53;
    D58 = B61-C58;
    D61 = B61+C58;
    D35 = B35+C36;
    D36 = B35-C36;
    D43 = B44-C43;
    D44 = B44+C43;
    D51 = B51+C52;
    D52 = B51-C52;
    D59 = B60-C59;
    D60 = B60+C59;

    aaiTemp[0 ][y] = xTrRound(724*(D0+D1), DenShift64);
    aaiTemp[32 ][y] = xTrRound(724*(D0-D1), DenShift64);
    aaiTemp[16 ][y] = xTrRound(946*D3+392*D2, DenShift64);
    aaiTemp[48 ][y] = xTrRound(392*D3-946*D2, DenShift64);
    E4 = C4+D5;
    E5 = C4-D5;
    E6 = C7-D6;
    E7 = C7+D6;
    E9 = xTrRound(392*D14-946*D9, DenShift64);
    E10 = xTrRound(-946*D13-392*D10, DenShift64);
    E13 = xTrRound(392*D13-946*D10, DenShift64);
    E14 = xTrRound(946*D14+392*D9, DenShift64);
    D15 = D15;
    E16 = C16+D19;
    E19 = C16-D19;
    E20 = C23-D20;
    E23 = C23+D20;
    E24 = C24+D27;
    E27 = C24-D27;
    E28 = C31-D28;
    E31 = C31+D28;
    E17 = C17+D18;
    E18 = C17-D18;
    E21 = C22-D21;
    E22 = C22+D21;
    E25 = C25+D26;
    E26 = C25-D26;
    E29 = C30-D29;
    E30 = C30+D29;
    E34 = xTrRound(200*D61-1004*D34, DenShift64);
    E35 = xTrRound(200*D60-1004*D35, DenShift64);
    E36 = xTrRound(-1004*D59-200*D36, DenShift64);
    E37 = xTrRound(-1004*D58-200*D37, DenShift64);
    E42 = xTrRound(851*D53-569*D42, DenShift64);
    E43 = xTrRound(851*D52-569*D43, DenShift64);
    E44 = xTrRound(-569*D51-851*D44, DenShift64);
    E45 = xTrRound(-569*D50-851*D45, DenShift64);
    E50 = xTrRound(851*D50-569*D45, DenShift64);
    E51 = xTrRound(851*D51-569*D44, DenShift64);
    E52 = xTrRound(569*D52+851*D43, DenShift64);
    E53 = xTrRound(569*D53+851*D42, DenShift64);
    E58 = xTrRound(200*D58-1004*D37, DenShift64);
    E59 = xTrRound(200*D59-1004*D36, DenShift64);
    E60 = xTrRound(1004*D60+200*D35, DenShift64);
    E61 = xTrRound(1004*D61+200*D34, DenShift64);

    aaiTemp[8 ][y] = xTrRound(200*E4+1004*E7, DenShift64);
    aaiTemp[40 ][y] = xTrRound(851*E5+569*E6, DenShift64);
    aaiTemp[24 ][y] = xTrRound(851*E6-569*E5, DenShift64);
    aaiTemp[56 ][y] = xTrRound(200*E7-1004*E4, DenShift64);
    F8 = D8+E9;
    F9 = D8-E9;
    F10 = D11-E10;
    F11 = D11+E10;
    F12 = D12+E13;
    F13 = D12-E13;
    F14 = D15-E14;
    F15 = D15+E14;
    F17 = xTrRound(200*E30-1004*E17, DenShift64);
    F18 = xTrRound(-1004*E29-200*E18, DenShift64);
    F21 = xTrRound(851*E26-569*E21, DenShift64);
    F22 = xTrRound(-569*E25-851*E22, DenShift64);
    F25 = xTrRound(851*E25-569*E22, DenShift64);
    F26 = xTrRound(569*E26+851*E21, DenShift64);
    F29 = xTrRound(200*E29-1004*E18, DenShift64);
    F30 = xTrRound(1004*E30+200*E17, DenShift64);
    F32 = D32+E35;
    F33 = D33+E34;
    F34 = D33-E34;
    F35 = D32-E35;
    F36 = D39-E36;
    F37 = D38-E37;
    F38 = D38+E37;
    F39 = D39+E36;
    F40 = D40+E43;
    F41 = D41+E42;
    F42 = D41-E42;
    F43 = D40-E43;
    F44 = D47-E44;
    F45 = D46-E45;
    F46 = D46+E45;
    F47 = D47+E44;
    F48 = D48+E51;
    F49 = D49+E50;
    F50 = D49-E50;
    F51 = D48-E51;
    F52 = D55-E52;
    F53 = D54-E53;
    F54 = D54+E53;
    F55 = D55+E52;
    F56 = D56+E59;
    F57 = D57+E58;
    F58 = D57-E58;
    F59 = D56-E59;
    F60 = D63-E60;
    F61 = D62-E61;
    F62 = D62+E61;
    F63 = D63+E60;

    aaiTemp[4 ][y] = xTrRound(100*F8+1019*F15, DenShift64);
    aaiTemp[36 ][y] = xTrRound(792*F9+650*F14, DenShift64);
    aaiTemp[20 ][y] = xTrRound(483*F10+903*F13, DenShift64);
    aaiTemp[52 ][y] = xTrRound(980*F11+297*F12, DenShift64);
    aaiTemp[12 ][y] = xTrRound(980*F12-297*F11, DenShift64);
    aaiTemp[44 ][y] = xTrRound(483*F13-903*F10, DenShift64);
    aaiTemp[28 ][y] = xTrRound(792*F14-650*F9, DenShift64);
    aaiTemp[60 ][y] = xTrRound(100*F15-1019*F8, DenShift64);
    G16 = E16+F17;
    G17 = E16-F17;
    G18 = E19-F18;
    G19 = E19+F18;
    G20 = E20+F21;
    G21 = E20-F21;
    G22 = E23-F22;
    G23 = E23+F22;
    G24 = E24+F25;
    G25 = E24-F25;
    G26 = E27-F26;
    G27 = E27+F26;
    G28 = E28+F29;
    G29 = E28-F29;
    G30 = E31-F30;
    G31 = E31+F30;
    G33 = xTrRound(100*F62-1019*F33, DenShift64);
    G34 = xTrRound(-1019*F61-100*F34, DenShift64);
    G37 = xTrRound(792*F58-650*F37, DenShift64);
    G38 = xTrRound(-650*F57-792*F38, DenShift64);
    G41 = xTrRound(483*F54-903*F41, DenShift64);
    G42 = xTrRound(-903*F53-483*F42, DenShift64);
    G45 = xTrRound(980*F50-297*F45, DenShift64);
    G46 = xTrRound(-297*F49-980*F46, DenShift64);
    G49 = xTrRound(980*F49-297*F46, DenShift64);
    G50 = xTrRound(297*F50+980*F45, DenShift64);
    G53 = xTrRound(483*F53-903*F42, DenShift64);
    G54 = xTrRound(903*F54+483*F41, DenShift64);
    G57 = xTrRound(792*F57-650*F38, DenShift64);
    G58 = xTrRound(650*F58+792*F37, DenShift64);
    G61 = xTrRound(100*F61-1019*F34, DenShift64);
    G62 = xTrRound(1019*F62+100*F33, DenShift64);

    aaiTemp[2 ][y] = xTrRound(50*G16+1023*G31, DenShift64);
    aaiTemp[34 ][y] = xTrRound(759*G17+688*G30, DenShift64);
    aaiTemp[18 ][y] = xTrRound(438*G18+926*G29, DenShift64);
    aaiTemp[50 ][y] = xTrRound(964*G19+345*G28, DenShift64);
    aaiTemp[10 ][y] = xTrRound(249*G20+993*G27, DenShift64);
    aaiTemp[42 ][y] = xTrRound(878*G21+526*G26, DenShift64);
    aaiTemp[26 ][y] = xTrRound(610*G22+822*G25, DenShift64);
    aaiTemp[58 ][y] = xTrRound(1013*G23+150*G24, DenShift64);
    aaiTemp[6 ][y] = xTrRound(1013*G24-150*G23, DenShift64);
    aaiTemp[38 ][y] = xTrRound(610*G25-822*G22, DenShift64);
    aaiTemp[22 ][y] = xTrRound(878*G26-526*G21, DenShift64);
    aaiTemp[54 ][y] = xTrRound(249*G27-993*G20, DenShift64);
    aaiTemp[14 ][y] = xTrRound(964*G28-345*G19, DenShift64);
    aaiTemp[46 ][y] = xTrRound(438*G29-926*G18, DenShift64);
    aaiTemp[30 ][y] = xTrRound(759*G30-688*G17, DenShift64);
    aaiTemp[62 ][y] = xTrRound(50*G31-1023*G16, DenShift64);
    H32 = F32+G33;
    H33 = F32-G33;
    H34 = F35-G34;
    H35 = F35+G34;
    H36 = F36+G37;
    H37 = F36-G37;
    H38 = F39-G38;
    H39 = F39+G38;
    H40 = F40+G41;
    H41 = F40-G41;
    H42 = F43-G42;
    H43 = F43+G42;
    H44 = F44+G45;
    H45 = F44-G45;
    H46 = F47-G46;
    H47 = F47+G46;
    H48 = F48+G49;
    H49 = F48-G49;
    H50 = F51-G50;
    H51 = F51+G50;
    H52 = F52+G53;
    H53 = F52-G53;
    H54 = F55-G54;
    H55 = F55+G54;
    H56 = F56+G57;
    H57 = F56-G57;
    H58 = F59-G58;
    H59 = F59+G58;
    H60 = F60+G61;
    H61 = F60-G61;
    H62 = F63-G62;
    H63 = F63+G62;

    aaiTemp[1 ][y] = xTrRound(25*H32+1024*H63, DenShift64);
    aaiTemp[33 ][y] = xTrRound(742*H33+706*H62, DenShift64);
    aaiTemp[17 ][y] = xTrRound(415*H34+936*H61, DenShift64);
    aaiTemp[49 ][y] = xTrRound(955*H35+369*H60, DenShift64);
    aaiTemp[9 ][y] = xTrRound(224*H36+999*H59, DenShift64);
    aaiTemp[41 ][y] = xTrRound(865*H37+548*H58, DenShift64);
    aaiTemp[25 ][y] = xTrRound(590*H38+837*H57, DenShift64);
    aaiTemp[57 ][y] = xTrRound(1009*H39+175*H56, DenShift64);
    aaiTemp[5 ][y] = xTrRound(125*H40+1016*H55, DenShift64);
    aaiTemp[37 ][y] = xTrRound(807*H41+630*H54, DenShift64);
    aaiTemp[21 ][y] = xTrRound(505*H42+891*H53, DenShift64);
    aaiTemp[53 ][y] = xTrRound(987*H43+273*H52, DenShift64);
    aaiTemp[13 ][y] = xTrRound(321*H44+972*H51, DenShift64);
    aaiTemp[45 ][y] = xTrRound(915*H45+460*H50, DenShift64);
    aaiTemp[29 ][y] = xTrRound(669*H46+775*H49, DenShift64);
    aaiTemp[61 ][y] = xTrRound(1021*H47+75*H48, DenShift64);
    aaiTemp[3 ][y] = xTrRound(1021*H48-75*H47, DenShift64);
    aaiTemp[35 ][y] = xTrRound(669*H49-775*H46, DenShift64);
    aaiTemp[19 ][y] = xTrRound(915*H50-460*H45, DenShift64);
    aaiTemp[51 ][y] = xTrRound(321*H51-972*H44, DenShift64);
    aaiTemp[11 ][y] = xTrRound(987*H52-273*H43, DenShift64);
    aaiTemp[43 ][y] = xTrRound(505*H53-891*H42, DenShift64);
    aaiTemp[27 ][y] = xTrRound(807*H54-630*H41, DenShift64);
    aaiTemp[59 ][y] = xTrRound(125*H55-1016*H40, DenShift64);
    aaiTemp[7 ][y] = xTrRound(1009*H56-175*H39, DenShift64);
    aaiTemp[39 ][y] = xTrRound(590*H57-837*H38, DenShift64);
    aaiTemp[23 ][y] = xTrRound(865*H58-548*H37, DenShift64);
    aaiTemp[55 ][y] = xTrRound(224*H59-999*H36, DenShift64);
    aaiTemp[15 ][y] = xTrRound(955*H60-369*H35, DenShift64);
    aaiTemp[47 ][y] = xTrRound(415*H61-936*H34, DenShift64);
    aaiTemp[31 ][y] = xTrRound(742*H62-706*H33, DenShift64);
    aaiTemp[63 ][y] = xTrRound(25*H63-1024*H32, DenShift64);

    pSrc += uiStride;
  }

  for( x=0 ; x<64 ; x++, pDes++ )
  {
    O0 = aaiTemp[x][0]+aaiTemp[x][63];
    O63 = aaiTemp[x][0]-aaiTemp[x][63];
    O1 = aaiTemp[x][1]+aaiTemp[x][62];
    O62 = aaiTemp[x][1]-aaiTemp[x][62];
    O2 = aaiTemp[x][2]+aaiTemp[x][61];
    O61 = aaiTemp[x][2]-aaiTemp[x][61];
    O3 = aaiTemp[x][3]+aaiTemp[x][60];
    O60 = aaiTemp[x][3]-aaiTemp[x][60];
    O4 = aaiTemp[x][4]+aaiTemp[x][59];
    O59 = aaiTemp[x][4]-aaiTemp[x][59];
    O5 = aaiTemp[x][5]+aaiTemp[x][58];
    O58 = aaiTemp[x][5]-aaiTemp[x][58];
    O6 = aaiTemp[x][6]+aaiTemp[x][57];
    O57 = aaiTemp[x][6]-aaiTemp[x][57];
    O7 = aaiTemp[x][7]+aaiTemp[x][56];
    O56 = aaiTemp[x][7]-aaiTemp[x][56];
    O8 = aaiTemp[x][8]+aaiTemp[x][55];
    O55 = aaiTemp[x][8]-aaiTemp[x][55];
    O9 = aaiTemp[x][9]+aaiTemp[x][54];
    O54 = aaiTemp[x][9]-aaiTemp[x][54];
    O10 = aaiTemp[x][10]+aaiTemp[x][53];
    O53 = aaiTemp[x][10]-aaiTemp[x][53];
    O11 = aaiTemp[x][11]+aaiTemp[x][52];
    O52 = aaiTemp[x][11]-aaiTemp[x][52];
    O12 = aaiTemp[x][12]+aaiTemp[x][51];
    O51 = aaiTemp[x][12]-aaiTemp[x][51];
    O13 = aaiTemp[x][13]+aaiTemp[x][50];
    O50 = aaiTemp[x][13]-aaiTemp[x][50];
    O14 = aaiTemp[x][14]+aaiTemp[x][49];
    O49 = aaiTemp[x][14]-aaiTemp[x][49];
    O15 = aaiTemp[x][15]+aaiTemp[x][48];
    O48 = aaiTemp[x][15]-aaiTemp[x][48];
    O16 = aaiTemp[x][16]+aaiTemp[x][47];
    O47 = aaiTemp[x][16]-aaiTemp[x][47];
    O17 = aaiTemp[x][17]+aaiTemp[x][46];
    O46 = aaiTemp[x][17]-aaiTemp[x][46];
    O18 = aaiTemp[x][18]+aaiTemp[x][45];
    O45 = aaiTemp[x][18]-aaiTemp[x][45];
    O19 = aaiTemp[x][19]+aaiTemp[x][44];
    O44 = aaiTemp[x][19]-aaiTemp[x][44];
    O20 = aaiTemp[x][20]+aaiTemp[x][43];
    O43 = aaiTemp[x][20]-aaiTemp[x][43];
    O21 = aaiTemp[x][21]+aaiTemp[x][42];
    O42 = aaiTemp[x][21]-aaiTemp[x][42];
    O22 = aaiTemp[x][22]+aaiTemp[x][41];
    O41 = aaiTemp[x][22]-aaiTemp[x][41];
    O23 = aaiTemp[x][23]+aaiTemp[x][40];
    O40 = aaiTemp[x][23]-aaiTemp[x][40];
    O24 = aaiTemp[x][24]+aaiTemp[x][39];
    O39 = aaiTemp[x][24]-aaiTemp[x][39];
    O25 = aaiTemp[x][25]+aaiTemp[x][38];
    O38 = aaiTemp[x][25]-aaiTemp[x][38];
    O26 = aaiTemp[x][26]+aaiTemp[x][37];
    O37 = aaiTemp[x][26]-aaiTemp[x][37];
    O27 = aaiTemp[x][27]+aaiTemp[x][36];
    O36 = aaiTemp[x][27]-aaiTemp[x][36];
    O28 = aaiTemp[x][28]+aaiTemp[x][35];
    O35 = aaiTemp[x][28]-aaiTemp[x][35];
    O29 = aaiTemp[x][29]+aaiTemp[x][34];
    O34 = aaiTemp[x][29]-aaiTemp[x][34];
    O30 = aaiTemp[x][30]+aaiTemp[x][33];
    O33 = aaiTemp[x][30]-aaiTemp[x][33];
    O31 = aaiTemp[x][31]+aaiTemp[x][32];
    O32 = aaiTemp[x][31]-aaiTemp[x][32];

    A0 = O0+O31;
    A31 = O0-O31;
    A1 = O1+O30;
    A30 = O1-O30;
    A2 = O2+O29;
    A29 = O2-O29;
    A3 = O3+O28;
    A28 = O3-O28;
    A4 = O4+O27;
    A27 = O4-O27;
    A5 = O5+O26;
    A26 = O5-O26;
    A6 = O6+O25;
    A25 = O6-O25;
    A7 = O7+O24;
    A24 = O7-O24;
    A8 = O8+O23;
    A23 = O8-O23;
    A9 = O9+O22;
    A22 = O9-O22;
    A10 = O10+O21;
    A21 = O10-O21;
    A11 = O11+O20;
    A20 = O11-O20;
    A12 = O12+O19;
    A19 = O12-O19;
    A13 = O13+O18;
    A18 = O13-O18;
    A14 = O14+O17;
    A17 = O14-O17;
    A15 = O15+O16;
    A16 = O15-O16;
    A40 = xTrRound(724*(O55-O40), DenShift64);
    A55 = xTrRound(724*(O55+O40), DenShift64);
    A41 = xTrRound(724*(O54-O41), DenShift64);
    A54 = xTrRound(724*(O54+O41), DenShift64);
    A42 = xTrRound(724*(O53-O42), DenShift64);
    A53 = xTrRound(724*(O53+O42), DenShift64);
    A43 = xTrRound(724*(O52-O43), DenShift64);
    A52 = xTrRound(724*(O52+O43), DenShift64);
    A44 = xTrRound(724*(O51-O44), DenShift64);
    A51 = xTrRound(724*(O51+O44), DenShift64);
    A45 = xTrRound(724*(O50-O45), DenShift64);
    A50 = xTrRound(724*(O50+O45), DenShift64);
    A46 = xTrRound(724*(O49-O46), DenShift64);
    A49 = xTrRound(724*(O49+O46), DenShift64);
    A47 = xTrRound(724*(O48-O47), DenShift64);
    A48 = xTrRound(724*(O48+O47), DenShift64);

    B0 = A0+A15;
    B15 = A0-A15;
    B1 = A1+A14;
    B14 = A1-A14;
    B2 = A2+A13;
    B13 = A2-A13;
    B3 = A3+A12;
    B12 = A3-A12;
    B4 = A4+A11;
    B11 = A4-A11;
    B5 = A5+A10;
    B10 = A5-A10;
    B6 = A6+A9;
    B9 = A6-A9;
    B7 = A7+A8;
    B8 = A7-A8;
    B20 = xTrRound(724*(A27-A20), DenShift64);
    B27 = xTrRound(724*(A27+A20), DenShift64);
    B21 = xTrRound(724*(A26-A21), DenShift64);
    B26 = xTrRound(724*(A26+A21), DenShift64);
    B22 = xTrRound(724*(A25-A22), DenShift64);
    B25 = xTrRound(724*(A25+A22), DenShift64);
    B23 = xTrRound(724*(A24-A23), DenShift64);
    B24 = xTrRound(724*(A24+A23), DenShift64);
    B32 = O32+A47;
    B47 = O32-A47;
    B48 = O63-A48;
    B63 = O63+A48;
    B33 = O33+A46;
    B46 = O33-A46;
    B49 = O62-A49;
    B62 = O62+A49;
    B34 = O34+A45;
    B45 = O34-A45;
    B50 = O61-A50;
    B61 = O61+A50;
    B35 = O35+A44;
    B44 = O35-A44;
    B51 = O60-A51;
    B60 = O60+A51;
    B36 = O36+A43;
    B43 = O36-A43;
    B52 = O59-A52;
    B59 = O59+A52;
    B37 = O37+A42;
    B42 = O37-A42;
    B53 = O58-A53;
    B58 = O58+A53;
    B38 = O38+A41;
    B41 = O38-A41;
    B54 = O57-A54;
    B57 = O57+A54;
    B39 = O39+A40;
    B40 = O39-A40;
    B55 = O56-A55;
    B56 = O56+A55;

    C0 = B0+B7;
    C7 = B0-B7;
    C1 = B1+B6;
    C6 = B1-B6;
    C2 = B2+B5;
    C5 = B2-B5;
    C3 = B3+B4;
    C4 = B3-B4;
    C10 = xTrRound(724*(B13-B10), DenShift64);
    C13 = xTrRound(724*(B13+B10), DenShift64);
    C11 = xTrRound(724*(B12-B11), DenShift64);
    C12 = xTrRound(724*(B12+B11), DenShift64);
    C16 = A16+B23;
    C23 = A16-B23;
    C24 = A31-B24;
    C31 = A31+B24;
    C17 = A17+B22;
    C22 = A17-B22;
    C25 = A30-B25;
    C30 = A30+B25;
    C18 = A18+B21;
    C21 = A18-B21;
    C26 = A29-B26;
    C29 = A29+B26;
    C19 = A19+B20;
    C20 = A19-B20;
    C27 = A28-B27;
    C28 = A28+B27;
    C36 = xTrRound(392*B59-946*B36, DenShift64);
    C40 = xTrRound(-946*B55-392*B40, DenShift64);
    C52 = xTrRound(-946*B43+392*B52, DenShift64);
    C56 = xTrRound(392*B39+946*B56, DenShift64);
    C37 = xTrRound(392*B58-946*B37, DenShift64);
    C41 = xTrRound(-946*B54-392*B41, DenShift64);
    C53 = xTrRound(-946*B42+392*B53, DenShift64);
    C57 = xTrRound(392*B38+946*B57, DenShift64);
    C38 = xTrRound(392*B57-946*B38, DenShift64);
    C42 = xTrRound(-946*B53-392*B42, DenShift64);
    C54 = xTrRound(-946*B41+392*B54, DenShift64);
    C58 = xTrRound(392*B37+946*B58, DenShift64);
    C39 = xTrRound(392*B56-946*B39, DenShift64);
    C43 = xTrRound(-946*B52-392*B43, DenShift64);
    C55 = xTrRound(-946*B40+392*B55, DenShift64);
    C59 = xTrRound(392*B36+946*B59, DenShift64);

    D0 = C0+C3;
    D3 = C0-C3;
    D8 = B8+C11;
    D11 = B8-C11;
    D12 = B15-C12;
    D15 = B15+C12;
    D1 = C1+C2;
    D2 = C1-C2;
    D9 = B9+C10;
    D10 = B9-C10;
    D13 = B14-C13;
    D14 = B14+C13;
    D5 = xTrRound(724*(C6-C5), DenShift64);
    D6 = xTrRound(724*(C6+C5), DenShift64);
    D18 = xTrRound(392*C29-946*C18, DenShift64);
    D20 = xTrRound(-946*C27-392*C20, DenShift64);
    D26 = xTrRound(-946*C21+392*C26, DenShift64);
    D28 = xTrRound(392*C19+946*C28, DenShift64);
    D19 = xTrRound(392*C28-946*C19, DenShift64);
    D21 = xTrRound(-946*C26-392*C21, DenShift64);
    D27 = xTrRound(-946*C20+392*C27, DenShift64);
    D29 = xTrRound(392*C18+946*C29, DenShift64);
    D32 = B32+C39;
    D39 = B32-C39;
    D40 = B47-C40;
    D47 = B47+C40;
    D48 = B48+C55;
    D55 = B48-C55;
    D56 = B63-C56;
    D63 = B63+C56;
    D33 = B33+C38;
    D38 = B33-C38;
    D41 = B46-C41;
    D46 = B46+C41;
    D49 = B49+C54;
    D54 = B49-C54;
    D57 = B62-C57;
    D62 = B62+C57;
    D34 = B34+C37;
    D37 = B34-C37;
    D42 = B45-C42;
    D45 = B45+C42;
    D50 = B50+C53;
    D53 = B50-C53;
    D58 = B61-C58;
    D61 = B61+C58;
    D35 = B35+C36;
    D36 = B35-C36;
    D43 = B44-C43;
    D44 = B44+C43;
    D51 = B51+C52;
    D52 = B51-C52;
    D59 = B60-C59;
    D60 = B60+C59;

    pDes[0] = xTrRound(724*(D0+D1), DenShift64);
    pDes[2048] = xTrRound(724*(D0-D1), DenShift64);
    pDes[1024] = xTrRound(946*D3+392*D2, DenShift64);
    pDes[3072] = xTrRound(392*D3-946*D2, DenShift64);
    E4 = C4+D5;
    E5 = C4-D5;
    E6 = C7-D6;
    E7 = C7+D6;
    E9 = xTrRound(392*D14-946*D9, DenShift64);
    E10 = xTrRound(-946*D13-392*D10, DenShift64);
    E13 = xTrRound(392*D13-946*D10, DenShift64);
    E14 = xTrRound(946*D14+392*D9, DenShift64);
    D15 = D15;
    E16 = C16+D19;
    E19 = C16-D19;
    E20 = C23-D20;
    E23 = C23+D20;
    E24 = C24+D27;
    E27 = C24-D27;
    E28 = C31-D28;
    E31 = C31+D28;
    E17 = C17+D18;
    E18 = C17-D18;
    E21 = C22-D21;
    E22 = C22+D21;
    E25 = C25+D26;
    E26 = C25-D26;
    E29 = C30-D29;
    E30 = C30+D29;
    E34 = xTrRound(200*D61-1004*D34, DenShift64);
    E35 = xTrRound(200*D60-1004*D35, DenShift64);
    E36 = xTrRound(-1004*D59-200*D36, DenShift64);
    E37 = xTrRound(-1004*D58-200*D37, DenShift64);
    E42 = xTrRound(851*D53-569*D42, DenShift64);
    E43 = xTrRound(851*D52-569*D43, DenShift64);
    E44 = xTrRound(-569*D51-851*D44, DenShift64);
    E45 = xTrRound(-569*D50-851*D45, DenShift64);
    E50 = xTrRound(851*D50-569*D45, DenShift64);
    E51 = xTrRound(851*D51-569*D44, DenShift64);
    E52 = xTrRound(569*D52+851*D43, DenShift64);
    E53 = xTrRound(569*D53+851*D42, DenShift64);
    E58 = xTrRound(200*D58-1004*D37, DenShift64);
    E59 = xTrRound(200*D59-1004*D36, DenShift64);
    E60 = xTrRound(1004*D60+200*D35, DenShift64);
    E61 = xTrRound(1004*D61+200*D34, DenShift64);

    pDes[512] = xTrRound(200*E4+1004*E7, DenShift64);
    pDes[2560] = xTrRound(851*E5+569*E6, DenShift64);
    pDes[1536] = xTrRound(851*E6-569*E5, DenShift64);
    pDes[3584] = xTrRound(200*E7-1004*E4, DenShift64);
    F8 = D8+E9;
    F9 = D8-E9;
    F10 = D11-E10;
    F11 = D11+E10;
    F12 = D12+E13;
    F13 = D12-E13;
    F14 = D15-E14;
    F15 = D15+E14;
    F17 = xTrRound(200*E30-1004*E17, DenShift64);
    F18 = xTrRound(-1004*E29-200*E18, DenShift64);
    F21 = xTrRound(851*E26-569*E21, DenShift64);
    F22 = xTrRound(-569*E25-851*E22, DenShift64);
    F25 = xTrRound(851*E25-569*E22, DenShift64);
    F26 = xTrRound(569*E26+851*E21, DenShift64);
    F29 = xTrRound(200*E29-1004*E18, DenShift64);
    F30 = xTrRound(1004*E30+200*E17, DenShift64);
    F32 = D32+E35;
    F33 = D33+E34;
    F34 = D33-E34;
    F35 = D32-E35;
    F36 = D39-E36;
    F37 = D38-E37;
    F38 = D38+E37;
    F39 = D39+E36;
    F40 = D40+E43;
    F41 = D41+E42;
    F42 = D41-E42;
    F43 = D40-E43;
    F44 = D47-E44;
    F45 = D46-E45;
    F46 = D46+E45;
    F47 = D47+E44;
    F48 = D48+E51;
    F49 = D49+E50;
    F50 = D49-E50;
    F51 = D48-E51;
    F52 = D55-E52;
    F53 = D54-E53;
    F54 = D54+E53;
    F55 = D55+E52;
    F56 = D56+E59;
    F57 = D57+E58;
    F58 = D57-E58;
    F59 = D56-E59;
    F60 = D63-E60;
    F61 = D62-E61;
    F62 = D62+E61;
    F63 = D63+E60;

    pDes[256] = xTrRound(100*F8+1019*F15, DenShift64);
    pDes[2304] = xTrRound(792*F9+650*F14, DenShift64);
    pDes[1280] = xTrRound(483*F10+903*F13, DenShift64);
    pDes[3328] = xTrRound(980*F11+297*F12, DenShift64);
    pDes[768] = xTrRound(980*F12-297*F11, DenShift64);
    pDes[2816] = xTrRound(483*F13-903*F10, DenShift64);
    pDes[1792] = xTrRound(792*F14-650*F9, DenShift64);
    pDes[3840] = xTrRound(100*F15-1019*F8, DenShift64);
    G16 = E16+F17;
    G17 = E16-F17;
    G18 = E19-F18;
    G19 = E19+F18;
    G20 = E20+F21;
    G21 = E20-F21;
    G22 = E23-F22;
    G23 = E23+F22;
    G24 = E24+F25;
    G25 = E24-F25;
    G26 = E27-F26;
    G27 = E27+F26;
    G28 = E28+F29;
    G29 = E28-F29;
    G30 = E31-F30;
    G31 = E31+F30;
    G33 = xTrRound(100*F62-1019*F33, DenShift64);
    G34 = xTrRound(-1019*F61-100*F34, DenShift64);
    G37 = xTrRound(792*F58-650*F37, DenShift64);
    G38 = xTrRound(-650*F57-792*F38, DenShift64);
    G41 = xTrRound(483*F54-903*F41, DenShift64);
    G42 = xTrRound(-903*F53-483*F42, DenShift64);
    G45 = xTrRound(980*F50-297*F45, DenShift64);
    G46 = xTrRound(-297*F49-980*F46, DenShift64);
    G49 = xTrRound(980*F49-297*F46, DenShift64);
    G50 = xTrRound(297*F50+980*F45, DenShift64);
    G53 = xTrRound(483*F53-903*F42, DenShift64);
    G54 = xTrRound(903*F54+483*F41, DenShift64);
    G57 = xTrRound(792*F57-650*F38, DenShift64);
    G58 = xTrRound(650*F58+792*F37, DenShift64);
    G61 = xTrRound(100*F61-1019*F34, DenShift64);
    G62 = xTrRound(1019*F62+100*F33, DenShift64);

    pDes[128] = xTrRound(50*G16+1023*G31, DenShift64);
    pDes[2176] = xTrRound(759*G17+688*G30, DenShift64);
    pDes[1152] = xTrRound(438*G18+926*G29, DenShift64);
    pDes[3200] = xTrRound(964*G19+345*G28, DenShift64);
    pDes[640] = xTrRound(249*G20+993*G27, DenShift64);
    pDes[2688] = xTrRound(878*G21+526*G26, DenShift64);
    pDes[1664] = xTrRound(610*G22+822*G25, DenShift64);
    pDes[3712] = xTrRound(1013*G23+150*G24, DenShift64);
    pDes[384] = xTrRound(1013*G24-150*G23, DenShift64);
    pDes[2432] = xTrRound(610*G25-822*G22, DenShift64);
    pDes[1408] = xTrRound(878*G26-526*G21, DenShift64);
    pDes[3456] = xTrRound(249*G27-993*G20, DenShift64);
    pDes[896] = xTrRound(964*G28-345*G19, DenShift64);
    pDes[2944] = xTrRound(438*G29-926*G18, DenShift64);
    pDes[1920] = xTrRound(759*G30-688*G17, DenShift64);
    pDes[3968] = xTrRound(50*G31-1023*G16, DenShift64);
    H32 = F32+G33;
    H33 = F32-G33;
    H34 = F35-G34;
    H35 = F35+G34;
    H36 = F36+G37;
    H37 = F36-G37;
    H38 = F39-G38;
    H39 = F39+G38;
    H40 = F40+G41;
    H41 = F40-G41;
    H42 = F43-G42;
    H43 = F43+G42;
    H44 = F44+G45;
    H45 = F44-G45;
    H46 = F47-G46;
    H47 = F47+G46;
    H48 = F48+G49;
    H49 = F48-G49;
    H50 = F51-G50;
    H51 = F51+G50;
    H52 = F52+G53;
    H53 = F52-G53;
    H54 = F55-G54;
    H55 = F55+G54;
    H56 = F56+G57;
    H57 = F56-G57;
    H58 = F59-G58;
    H59 = F59+G58;
    H60 = F60+G61;
    H61 = F60-G61;
    H62 = F63-G62;
    H63 = F63+G62;

    pDes[64] = xTrRound(25*H32+1024*H63, DenShift64);
    pDes[2112] = xTrRound(742*H33+706*H62, DenShift64);
    pDes[1088] = xTrRound(415*H34+936*H61, DenShift64);
    pDes[3136] = xTrRound(955*H35+369*H60, DenShift64);
    pDes[576] = xTrRound(224*H36+999*H59, DenShift64);
    pDes[2624] = xTrRound(865*H37+548*H58, DenShift64);
    pDes[1600] = xTrRound(590*H38+837*H57, DenShift64);
    pDes[3648] = xTrRound(1009*H39+175*H56, DenShift64);
    pDes[320] = xTrRound(125*H40+1016*H55, DenShift64);
    pDes[2368] = xTrRound(807*H41+630*H54, DenShift64);
    pDes[1344] = xTrRound(505*H42+891*H53, DenShift64);
    pDes[3392] = xTrRound(987*H43+273*H52, DenShift64);
    pDes[832] = xTrRound(321*H44+972*H51, DenShift64);
    pDes[2880] = xTrRound(915*H45+460*H50, DenShift64);
    pDes[1856] = xTrRound(669*H46+775*H49, DenShift64);
    pDes[3904] = xTrRound(1021*H47+75*H48, DenShift64);
    pDes[192] = xTrRound(1021*H48-75*H47, DenShift64);
    pDes[2240] = xTrRound(669*H49-775*H46, DenShift64);
    pDes[1216] = xTrRound(915*H50-460*H45, DenShift64);
    pDes[3264] = xTrRound(321*H51-972*H44, DenShift64);
    pDes[704] = xTrRound(987*H52-273*H43, DenShift64);
    pDes[2752] = xTrRound(505*H53-891*H42, DenShift64);
    pDes[1728] = xTrRound(807*H54-630*H41, DenShift64);
    pDes[3776] = xTrRound(125*H55-1016*H40, DenShift64);
    pDes[448] = xTrRound(1009*H56-175*H39, DenShift64);
    pDes[2496] = xTrRound(590*H57-837*H38, DenShift64);
    pDes[1472] = xTrRound(865*H58-548*H37, DenShift64);
    pDes[3520] = xTrRound(224*H59-999*H36, DenShift64);
    pDes[960] = xTrRound(955*H60-369*H35, DenShift64);
    pDes[3008] = xTrRound(415*H61-936*H34, DenShift64);
    pDes[1984] = xTrRound(742*H62-706*H33, DenShift64);
    pDes[4032] = xTrRound(25*H63-1024*H32, DenShift64);

  }
}

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

//--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
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

//--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
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
  }
}

Void TComTrQuant::xQuantLTR  (TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  UInt* piQuantCoef = NULL;
  Int   iNewBits    = 0;
  Int   iAdd;
  Bool  bLogical    = false;

	Long*		piCoef		= pSrc;
	TCoeff* piQCoef		= pDes;

	if ( iWidth > (Int)m_uiMaxTrSize )
	{
		iWidth  = m_uiMaxTrSize;
		iHeight = m_uiMaxTrSize;
    bLogical = true;
	}

  switch(iWidth)
  {
  case 2:
    {
      m_puiQuantMtx = &g_aiQuantCoef4[m_cQP.m_iRem];
      xQuant2x2(piCoef, piQCoef, uiAcSum, indexROT );
      return;
    }
  case 4:
    {
      m_puiQuantMtx = &g_aiQuantCoef[m_cQP.m_iRem][0];
      xQuant4x4(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx, indexROT );
      return;
    }
  case 8:
    {
      m_puiQuantMtx = &g_aiQuantCoef64[m_cQP.m_iRem][0];
      xQuant8x8(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx, indexROT );
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
  case 64:
    {
      piQuantCoef = g_aiQuantCoef4096;	 // To save the memory for g_aiQuantCoef4096
      iNewBits = ECore64Shift + m_cQP.per();
      iAdd = bLogical ? m_cQP.m_iAddNxN : m_cQP.m_iAdd64x64;
      break;
    }
  }

	if ( indexROT )
	{
		Int x, x2, y, y2, y3;
		static Long ROT_DOMAIN[64];

		for( y = 0, y2 = 0, y3 = 0; y < 8; y++, y2+=8, y3+=iWidth )
		{
			for(  x = 0, x2=y3; x < 8; x++, x2++ )
			{
				if ( iWidth == 64) ROT_DOMAIN[x+y2] = piCoef [x2] * piQuantCoef[m_cQP.rem()];
				else							 ROT_DOMAIN[x+y2] = piCoef [x2] * piQuantCoef[x2];
			}
		}

		RotTransformLI2(ROT_DOMAIN, indexROT);

		for( y = 0, y2 = 0, y3 = 0; y < 8; y++, y2+=8, y3+=iWidth )
		{
			::memcpy( piCoef+y3, ROT_DOMAIN+y2 , sizeof(Long)*8 );
		}
	}

  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) && (!RDOQ_ROT_IDX0_ONLY || indexROT == 0) )
  {
    xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx, indexROT );
  }
  else
  {
    for( Int n = 0; n < iWidth*iHeight; n++ )
		{
      Long iLevel;
      Int  iSign;
      if ( indexROT )
      {
        iLevel  = (Long) piCoef[n];
        iSign   = (iLevel < 0 ? -1: 1);

				if ( ( n/iWidth ) < 8 && ( n%iWidth ) < 8 )
				{
					iLevel = abs( iLevel );
				}
        else
        {
          if ( iWidth == 64 ) iLevel = abs( iLevel ) * piQuantCoef[m_cQP.rem()];
          else								iLevel = abs( iLevel ) * piQuantCoef[n];
        }
      }
      else
      {
        iLevel  = (Long) piCoef[n];
        iSign   = (iLevel < 0 ? -1: 1);
        if ( iWidth == 64 )	iLevel = abs( iLevel ) * piQuantCoef[m_cQP.rem()];
        else								iLevel = abs( iLevel ) * piQuantCoef[n];
      }

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
  }
}

Void TComTrQuant::xDeQuantLTR( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight, UChar indexROT )
{
  UInt* piDeQuantCoef = NULL;

	TCoeff* piQCoef		= pSrc;
	Long*		piCoef		= pDes;

	if ( iWidth > (Int)m_uiMaxTrSize )
	{
		iWidth  = m_uiMaxTrSize;
		iHeight = m_uiMaxTrSize;
	}

  switch(iWidth)
  {
  case 2:
    {
      xDeQuant2x2( piQCoef, piCoef, indexROT );
      return;
    }
  case 4:
    {
      xDeQuant4x4( piQCoef, piCoef, indexROT );
      return;
    }
  case 8:
    {
      xDeQuant8x8( piQCoef, piCoef, indexROT );
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
			else								iDeScale = piDeQuantCoef[n];
			piCoef[n] = (Long) (iLevel*iDeScale) << m_cQP.per();
		}
		else
		{
			piCoef [n] = 0;
		}
	}

	if ( indexROT )
	{
		Int y,y2, y3;
		static Long ROT_DOMAIN[64];

		for( y = 0, y2 = 0, y3 = 0; y < 8; y++, y2+=8, y3+=iWidth )
		{
			::memcpy( ROT_DOMAIN+y2, piCoef+y3, sizeof(Long)*8 );
		}

		InvRotTransformLI2(ROT_DOMAIN, indexROT);

		for( y = 0, y2 = 0, y3 = 0; y < 8; y++, y2+=8, y3+=iWidth )
		{
			::memcpy( piCoef+y3, ROT_DOMAIN+y2 , sizeof(Long)*8 );
		}
	}
}

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

//--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
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
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;

    D0 = xTrRound( 45 * ( pSrc[0] + pSrc[8] ) , DenShift16);
    D1 = xTrRound( 45 * ( pSrc[0] - pSrc[8] ) , DenShift16);
    D2 = xTrRound( 24 * pSrc[4] - 59 * pSrc[12] , DenShift16);
    D3 = xTrRound( 59 * pSrc[4] + 24 * pSrc[12] , DenShift16);
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

//--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
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

    E4 = xTrRound( 49 * pSrc[4] - 251 * pSrc[28], DenShift32);
    E5 = xTrRound( 212 * pSrc[20] - 142 * pSrc[12], DenShift32);
    E6 = xTrRound( 212 * pSrc[12] + 142 * pSrc[20], DenShift32);
    E7 = xTrRound( 49 * pSrc[28] + 251 * pSrc[4], DenShift32);
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

    D0 = xTrRound( 181 * ( pSrc[0] + pSrc[16] ), DenShift32);
    D1 = xTrRound( 181 * ( pSrc[0] - pSrc[16] ), DenShift32);
    D2 = xTrRound( 97 * pSrc[8] - 236 * pSrc[24], DenShift32);
    D3 = xTrRound( 236 * pSrc[8] + 97 * pSrc[24], DenShift32);
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
  }
}

Void TComTrQuant::xIT64 ( Long* pSrc, Pel* pDes, UInt uiStride )
{
  Int x, y;
  static Long aaiTemp[64][64];

  Long O0, O1, O2, O3, O4, O5, O6, O7, O8, O9, O10, O11, O12, O13, O14, O15, O16, O17, O18, O19, O20, O21, O22, O23, O24, O25, O26, O27, O28, O29, O30, O31;
  Long O40, O41, O42, O43, O44, O45, O46, O47, O48, O49, O50, O51, O52, O53, O54, O55;
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A20, A21, A22, A23, A24, A25, A26, A27, A32;
  Long A33, A34, A35, A36, A37, A38, A39, A40, A41, A42, A43, A44, A45, A46, A47, A48, A49, A50, A51, A52, A53, A54, A55, A56, A57, A58, A59, A60, A61, A62, A63;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B10, B11, B12, B13, B16, B17, B18, B19, B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31;
  Long B36, B37, B38, B39, B40, B41, B42, B43, B52, B53, B54, B55, B56, B57, B58, B59;
  Long C0, C1, C2, C3, C5, C6, C8, C9, C10, C11, C12, C13, C14, C15, C18, C19, C20, C21, C26, C27, C28, C29, C32;
  Long C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, C50, C51, C52, C53, C54, C55, C56, C57, C58, C59, C60, C61, C62, C63;
  Long D0, D1, D2, D3, D4, D5, D6, D7, D9, D10, D13, D14, D16, D17, D18, D19, D20, D21, D22, D23, D24, D25, D26, D27, D28, D29, D30, D31;
  Long D34, D35, D36, D37, D42, D43, D44, D45, D50, D51, D52, D53, D58, D59, D60, D61;
  Long E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E17, E18, E21, E22, E25, E26, E29, E30, E32;
  Long E33, E34, E35, E36, E37, E38, E39, E40, E41, E42, E43, E44, E45, E46, E47, E48, E49, E50, E51, E52, E53, E54, E55, E56, E57, E58, E59, E60, E61, E62, E63;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31;
  Long F33, F34, F37, F38, F41, F42, F45, F46, F49, F50, F53, F54, F57, F58, F61, F62;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31, G32;
  Long G33, G34, G35, G36, G37, G38, G39, G40, G41, G42, G43, G44, G45, G46, G47, G48, G49, G50, G51, G52, G53, G54, G55, G56, G57, G58, G59, G60, G61, G62, G63;
  Long H32;
  Long H33, H34, H35, H36, H37, H38, H39, H40, H41, H42, H43, H44, H45, H46, H47, H48, H49, H50, H51, H52, H53, H54, H55, H56, H57, H58, H59, H60, H61, H62, H63;

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
  UInt uiStride32 = uiStride31 + uiStride;
  UInt uiStride33 = uiStride32 + uiStride;
  UInt uiStride34 = uiStride33 + uiStride;
  UInt uiStride35 = uiStride34 + uiStride;
  UInt uiStride36 = uiStride35 + uiStride;
  UInt uiStride37 = uiStride36 + uiStride;
  UInt uiStride38 = uiStride37 + uiStride;
  UInt uiStride39 = uiStride38 + uiStride;
  UInt uiStride40 = uiStride39 + uiStride;
  UInt uiStride41 = uiStride40 + uiStride;
  UInt uiStride42 = uiStride41 + uiStride;
  UInt uiStride43 = uiStride42 + uiStride;
  UInt uiStride44 = uiStride43 + uiStride;
  UInt uiStride45 = uiStride44 + uiStride;
  UInt uiStride46 = uiStride45 + uiStride;
  UInt uiStride47 = uiStride46 + uiStride;
  UInt uiStride48 = uiStride47 + uiStride;
  UInt uiStride49 = uiStride48 + uiStride;
  UInt uiStride50 = uiStride49 + uiStride;
  UInt uiStride51 = uiStride50 + uiStride;
  UInt uiStride52 = uiStride51 + uiStride;
  UInt uiStride53 = uiStride52 + uiStride;
  UInt uiStride54 = uiStride53 + uiStride;
  UInt uiStride55 = uiStride54 + uiStride;
  UInt uiStride56 = uiStride55 + uiStride;
  UInt uiStride57 = uiStride56 + uiStride;
  UInt uiStride58 = uiStride57 + uiStride;
  UInt uiStride59 = uiStride58 + uiStride;
  UInt uiStride60 = uiStride59 + uiStride;
  UInt uiStride61 = uiStride60 + uiStride;
  UInt uiStride62 = uiStride61 + uiStride;
  UInt uiStride63 = uiStride62 + uiStride;

  for( y=0 ; y<64 ; y++ )
  {
    H32 = xTrRound(25*pSrc[1]-1024*pSrc[63], DenShift64);
    H33 = xTrRound(742*pSrc[33]-706*pSrc[31], DenShift64);
    H34 = xTrRound(415*pSrc[17]-936*pSrc[47], DenShift64);
    H35 = xTrRound(955*pSrc[49]-369*pSrc[15], DenShift64);
    H36 = xTrRound(224*pSrc[9]-999*pSrc[55], DenShift64);
    H37 = xTrRound(865*pSrc[41]-548*pSrc[23], DenShift64);
    H38 = xTrRound(590*pSrc[25]-837*pSrc[39], DenShift64);
    H39 = xTrRound(1009*pSrc[57]-175*pSrc[7], DenShift64);
    H40 = xTrRound(125*pSrc[5]-1016*pSrc[59], DenShift64);
    H41 = xTrRound(807*pSrc[37]-630*pSrc[27], DenShift64);
    H42 = xTrRound(505*pSrc[21]-891*pSrc[43], DenShift64);
    H43 = xTrRound(987*pSrc[53]-273*pSrc[11], DenShift64);
    H44 = xTrRound(321*pSrc[13]-972*pSrc[51], DenShift64);
    H45 = xTrRound(915*pSrc[45]-460*pSrc[19], DenShift64);
    H46 = xTrRound(669*pSrc[29]-775*pSrc[35], DenShift64);
    H47 = xTrRound(1021*pSrc[61]-75*pSrc[3], DenShift64);
    H48 = xTrRound(1021*pSrc[3]+75*pSrc[61], DenShift64);
    H49 = xTrRound(669*pSrc[35]+775*pSrc[29], DenShift64);
    H50 = xTrRound(915*pSrc[19]+460*pSrc[45], DenShift64);
    H51 = xTrRound(321*pSrc[51]+972*pSrc[13], DenShift64);
    H52 = xTrRound(987*pSrc[11]+273*pSrc[53], DenShift64);
    H53 = xTrRound(505*pSrc[43]+891*pSrc[21], DenShift64);
    H54 = xTrRound(807*pSrc[27]+630*pSrc[37], DenShift64);
    H55 = xTrRound(125*pSrc[59]+1016*pSrc[5], DenShift64);
    H56 = xTrRound(1009*pSrc[7]+175*pSrc[57], DenShift64);
    H57 = xTrRound(590*pSrc[39]+837*pSrc[25], DenShift64);
    H58 = xTrRound(865*pSrc[23]+548*pSrc[41], DenShift64);
    H59 = xTrRound(224*pSrc[55]+999*pSrc[9], DenShift64);
    H60 = xTrRound(955*pSrc[15]+369*pSrc[49], DenShift64);
    H61 = xTrRound(415*pSrc[47]+936*pSrc[17], DenShift64);
    H62 = xTrRound(742*pSrc[31]+706*pSrc[33], DenShift64);
    H63 = xTrRound(25*pSrc[63]+1024*pSrc[1], DenShift64);

    G16 = xTrRound(50*pSrc[2]-1023*pSrc[62], DenShift64);
    G17 = xTrRound(759*pSrc[34]-688*pSrc[30], DenShift64);
    G18 = xTrRound(438*pSrc[18]-926*pSrc[46], DenShift64);
    G19 = xTrRound(964*pSrc[50]-345*pSrc[14], DenShift64);
    G20 = xTrRound(249*pSrc[10]-993*pSrc[54], DenShift64);
    G21 = xTrRound(878*pSrc[42]-526*pSrc[22], DenShift64);
    G22 = xTrRound(610*pSrc[26]-822*pSrc[38], DenShift64);
    G23 = xTrRound(1013*pSrc[58]-150*pSrc[6], DenShift64);
    G24 = xTrRound(1013*pSrc[6]+150*pSrc[58], DenShift64);
    G25 = xTrRound(610*pSrc[38]+822*pSrc[26], DenShift64);
    G26 = xTrRound(878*pSrc[22]+526*pSrc[42], DenShift64);
    G27 = xTrRound(249*pSrc[54]+993*pSrc[10], DenShift64);
    G28 = xTrRound(964*pSrc[14]+345*pSrc[50], DenShift64);
    G29 = xTrRound(438*pSrc[46]+926*pSrc[18], DenShift64);
    G30 = xTrRound(759*pSrc[30]+688*pSrc[34], DenShift64);
    G31 = xTrRound(50*pSrc[62]+1023*pSrc[2], DenShift64);
    G32 = H32+H33;
    G33 = H32-H33;
    G34 = H35-H34;
    G35 = H35+H34;
    G36 = H36+H37;
    G37 = H36-H37;
    G38 = H39-H38;
    G39 = H39+H38;
    G40 = H40+H41;
    G41 = H40-H41;
    G42 = H43-H42;
    G43 = H43+H42;
    G44 = H44+H45;
    G45 = H44-H45;
    G46 = H47-H46;
    G47 = H47+H46;
    G48 = H48+H49;
    G49 = H48-H49;
    G50 = H51-H50;
    G51 = H51+H50;
    G52 = H52+H53;
    G53 = H52-H53;
    G54 = H55-H54;
    G55 = H55+H54;
    G56 = H56+H57;
    G57 = H56-H57;
    G58 = H59-H58;
    G59 = H59+H58;
    G60 = H60+H61;
    G61 = H60-H61;
    G62 = H63-H62;
    G63 = H63+H62;

    F8 = xTrRound(100*pSrc[4]-1019*pSrc[60], DenShift64);
    F9 = xTrRound(792*pSrc[36]-650*pSrc[28], DenShift64);
    F10 = xTrRound(483*pSrc[20]-903*pSrc[44], DenShift64);
    F11 = xTrRound(980*pSrc[52]-297*pSrc[12], DenShift64);
    F12 = xTrRound(980*pSrc[12]+297*pSrc[52], DenShift64);
    F13 = xTrRound(483*pSrc[44]+903*pSrc[20], DenShift64);
    F14 = xTrRound(792*pSrc[28]+650*pSrc[36], DenShift64);
    F15 = xTrRound(100*pSrc[60]+1019*pSrc[4], DenShift64);
    F16 = G16+G17;
    F17 = G16-G17;
    F18 = G19-G18;
    F19 = G19+G18;
    F20 = G20+G21;
    F21 = G20-G21;
    F22 = G23-G22;
    F23 = G23+G22;
    F24 = G24+G25;
    F25 = G24-G25;
    F26 = G27-G26;
    F27 = G27+G26;
    F28 = G28+G29;
    F29 = G28-G29;
    F30 = G31-G30;
    F31 = G31+G30;
    F33 = xTrRound(100*G62-1019*G33, DenShift64);
    F34 = xTrRound(-1019*G61-100*G34, DenShift64);
    F37 = xTrRound(792*G58-650*G37, DenShift64);
    F38 = xTrRound(-650*G57-792*G38, DenShift64);
    F41 = xTrRound(483*G54-903*G41, DenShift64);
    F42 = xTrRound(-903*G53-483*G42, DenShift64);
    F45 = xTrRound(980*G50-297*G45, DenShift64);
    F46 = xTrRound(-297*G49-980*G46, DenShift64);
    F49 = xTrRound(980*G49-297*G46, DenShift64);
    F50 = xTrRound(297*G50+980*G45, DenShift64);
    F53 = xTrRound(483*G53-903*G42, DenShift64);
    F54 = xTrRound(903*G54+483*G41, DenShift64);
    F57 = xTrRound(792*G57-650*G38, DenShift64);
    F58 = xTrRound(650*G58+792*G37, DenShift64);
    F61 = xTrRound(100*G61-1019*G34, DenShift64);
    F62 = xTrRound(1019*G62+100*G33, DenShift64);

    E4 = xTrRound(200*pSrc[8]-1004*pSrc[56], DenShift64);
    E5 = xTrRound(851*pSrc[40]-569*pSrc[24], DenShift64);
    E6 = xTrRound(851*pSrc[24]+569*pSrc[40], DenShift64);
    E7 = xTrRound(200*pSrc[56]+1004*pSrc[8], DenShift64);
    E8 = F8+F9;
    E9 = F8-F9;
    E10 = F11-F10;
    E11 = F11+F10;
    E12 = F12+F13;
    E13 = F12-F13;
    E14 = F15-F14;
    E15 = F15+F14;
    E17 = xTrRound(200*F30-1004*F17, DenShift64);
    E18 = xTrRound(-1004*F29-200*F18, DenShift64);
    E21 = xTrRound(851*F26-569*F21, DenShift64);
    E22 = xTrRound(-569*F25-851*F22, DenShift64);
    E25 = xTrRound(851*F25-569*F22, DenShift64);
    E26 = xTrRound(569*F26+851*F21, DenShift64);
    E29 = xTrRound(200*F29-1004*F18, DenShift64);
    E30 = xTrRound(1004*F30+200*F17, DenShift64);
    E32 = G32+G35;
    E33 = F33+F34;
    E34 = F33-F34;
    E35 = G32-G35;
    E36 = G39-G36;
    E37 = F38-F37;
    E38 = F38+F37;
    E39 = G39+G36;
    E40 = G40+G43;
    E41 = F41+F42;
    E42 = F41-F42;
    E43 = G40-G43;
    E44 = G47-G44;
    E45 = F46-F45;
    E46 = F46+F45;
    E47 = G47+G44;
    E48 = G48+G51;
    E49 = F49+F50;
    E50 = F49-F50;
    E51 = G48-G51;
    E52 = G55-G52;
    E53 = F54-F53;
    E54 = F54+F53;
    E55 = G55+G52;
    E56 = G56+G59;
    E57 = F57+F58;
    E58 = F57-F58;
    E59 = G56-G59;
    E60 = G63-G60;
    E61 = F62-F61;
    E62 = F62+F61;
    E63 = G63+G60;

    D0 = xTrRound(724*(pSrc[0]+pSrc[32]), DenShift64);
    D1 = xTrRound(724*(pSrc[0]-pSrc[32]), DenShift64);
    D2 = xTrRound(392*pSrc[16]-946*pSrc[48], DenShift64);
    D3 = xTrRound(946*pSrc[16]+392*pSrc[48], DenShift64);
    D4 = E4+E5;
    D5 = E4-E5;
    D6 = E7-E6;
    D7 = E7+E6;
    D9 = xTrRound(392*E14-946*E9, DenShift64);
    D10 = xTrRound(-946*E13-392*E10, DenShift64);
    D13 = xTrRound(392*E13-946*E10, DenShift64);
    D14 = xTrRound(946*E14+392*E9, DenShift64);
    D16 = F16+F19;
    D19 = F16-F19;
    D20 = F23-F20;
    D23 = F23+F20;
    D24 = F24+F27;
    D27 = F24-F27;
    D28 = F31-F28;
    D31 = F31+F28;
    D17 = E17+E18;
    D18 = E17-E18;
    D21 = E22-E21;
    D22 = E22+E21;
    D25 = E25+E26;
    D26 = E25-E26;
    D29 = E30-E29;
    D30 = E30+E29;
    D34 = xTrRound(200*E61-1004*E34, DenShift64);
    D35 = xTrRound(200*E60-1004*E35, DenShift64);
    D36 = xTrRound(-1004*E59-200*E36, DenShift64);
    D37 = xTrRound(-1004*E58-200*E37, DenShift64);
    D42 = xTrRound(851*E53-569*E42, DenShift64);
    D43 = xTrRound(851*E52-569*E43, DenShift64);
    D44 = xTrRound(-569*E51-851*E44, DenShift64);
    D45 = xTrRound(-569*E50-851*E45, DenShift64);
    D50 = xTrRound(851*E50-569*E45, DenShift64);
    D51 = xTrRound(851*E51-569*E44, DenShift64);
    D52 = xTrRound(569*E52+851*E43, DenShift64);
    D53 = xTrRound(569*E53+851*E42, DenShift64);
    D58 = xTrRound(200*E58-1004*E37, DenShift64);
    D59 = xTrRound(200*E59-1004*E36, DenShift64);
    D60 = xTrRound(1004*E60+200*E35, DenShift64);
    D61 = xTrRound(1004*E61+200*E34, DenShift64);

    C0 = D0+D3;
    C3 = D0-D3;
    C8 = E8+E11;
    C11 = E8-E11;
    C12 = E15-E12;
    C15 = E15+E12;
    C1 = D1+D2;
    C2 = D1-D2;
    C9 = D9+D10;
    C10 = D9-D10;
    C13 = D14-D13;
    C14 = D14+D13;
    C5 = xTrRound(724*(D6-D5), DenShift64);
    C6 = xTrRound(724*(D6+D5), DenShift64);
    C18 = xTrRound(392*D29-946*D18, DenShift64);
    C20 = xTrRound(-946*D27-392*D20, DenShift64);
    C26 = xTrRound(-946*D21+392*D26, DenShift64);
    C28 = xTrRound(392*D19+946*D28, DenShift64);
    C19 = xTrRound(392*D28-946*D19, DenShift64);
    C21 = xTrRound(-946*D26-392*D21, DenShift64);
    C27 = xTrRound(-946*D20+392*D27, DenShift64);
    C29 = xTrRound(392*D18+946*D29, DenShift64);
    C32 = E32+E39;
    C39 = E32-E39;
    C40 = E47-E40;
    C47 = E47+E40;
    C48 = E48+E55;
    C55 = E48-E55;
    C56 = E63-E56;
    C63 = E63+E56;
    C33 = E33+E38;
    C38 = E33-E38;
    C41 = E46-E41;
    C46 = E46+E41;
    C49 = E49+E54;
    C54 = E49-E54;
    C57 = E62-E57;
    C62 = E62+E57;
    C34 = D34+D37;
    C37 = D34-D37;
    C42 = D45-D42;
    C45 = D45+D42;
    C50 = D50+D53;
    C53 = D50-D53;
    C58 = D61-D58;
    C61 = D61+D58;
    C35 = D35+D36;
    C36 = D35-D36;
    C43 = D44-D43;
    C44 = D44+D43;
    C51 = D51+D52;
    C52 = D51-D52;
    C59 = D60-D59;
    C60 = D60+D59;

    B0 = C0+D7;
    B7 = C0-D7;
    B1 = C1+C6;
    B6 = C1-C6;
    B2 = C2+C5;
    B5 = C2-C5;
    B3 = C3+D4;
    B4 = C3-D4;
    B10 = xTrRound(724*(C13-C10), DenShift64);
    B13 = xTrRound(724*(C13+C10), DenShift64);
    B11 = xTrRound(724*(C12-C11), DenShift64);
    B12 = xTrRound(724*(C12+C11), DenShift64);
    B16 = D16+D23;
    B23 = D16-D23;
    B24 = D31-D24;
    B31 = D31+D24;
    B17 = D17+D22;
    B22 = D17-D22;
    B25 = D30-D25;
    B30 = D30+D25;
    B18 = C18+C21;
    B21 = C18-C21;
    B26 = C29-C26;
    B29 = C29+C26;
    B19 = C19+C20;
    B20 = C19-C20;
    B27 = C28-C27;
    B28 = C28+C27;
    B36 = xTrRound(392*C59-946*C36, DenShift64);
    B40 = xTrRound(-946*C55-392*C40, DenShift64);
    B52 = xTrRound(-946*C43+392*C52, DenShift64);
    B56 = xTrRound(392*C39+946*C56, DenShift64);
    B37 = xTrRound(392*C58-946*C37, DenShift64);
    B41 = xTrRound(-946*C54-392*C41, DenShift64);
    B53 = xTrRound(-946*C42+392*C53, DenShift64);
    B57 = xTrRound(392*C38+946*C57, DenShift64);
    B38 = xTrRound(392*C57-946*C38, DenShift64);
    B42 = xTrRound(-946*C53-392*C42, DenShift64);
    B54 = xTrRound(-946*C41+392*C54, DenShift64);
    B58 = xTrRound(392*C37+946*C58, DenShift64);
    B39 = xTrRound(392*C56-946*C39, DenShift64);
    B43 = xTrRound(-946*C52-392*C43, DenShift64);
    B55 = xTrRound(-946*C40+392*C55, DenShift64);
    B59 = xTrRound(392*C36+946*C59, DenShift64);

    A0 = B0+C15;
    A15 = B0-C15;
    A1 = B1+C14;
    A14 = B1-C14;
    A2 = B2+B13;
    A13 = B2-B13;
    A3 = B3+B12;
    A12 = B3-B12;
    A4 = B4+B11;
    A11 = B4-B11;
    A5 = B5+B10;
    A10 = B5-B10;
    A6 = B6+C9;
    A9 = B6-C9;
    A7 = B7+C8;
    A8 = B7-C8;
    A20 = xTrRound(724*(B27-B20), DenShift64);
    A27 = xTrRound(724*(B27+B20), DenShift64);
    A21 = xTrRound(724*(B26-B21), DenShift64);
    A26 = xTrRound(724*(B26+B21), DenShift64);
    A22 = xTrRound(724*(B25-B22), DenShift64);
    A25 = xTrRound(724*(B25+B22), DenShift64);
    A23 = xTrRound(724*(B24-B23), DenShift64);
    A24 = xTrRound(724*(B24+B23), DenShift64);
    A32 = C32+C47;
    A47 = C32-C47;
    A48 = C63-C48;
    A63 = C63+C48;
    A33 = C33+C46;
    A46 = C33-C46;
    A49 = C62-C49;
    A62 = C62+C49;
    A34 = C34+C45;
    A45 = C34-C45;
    A50 = C61-C50;
    A61 = C61+C50;
    A35 = C35+C44;
    A44 = C35-C44;
    A51 = C60-C51;
    A60 = C60+C51;
    A36 = B36+B43;
    A43 = B36-B43;
    A52 = B59-B52;
    A59 = B59+B52;
    A37 = B37+B42;
    A42 = B37-B42;
    A53 = B58-B53;
    A58 = B58+B53;
    A38 = B38+B41;
    A41 = B38-B41;
    A54 = B57-B54;
    A57 = B57+B54;
    A39 = B39+B40;
    A40 = B39-B40;
    A55 = B56-B55;
    A56 = B56+B55;

    O0 = A0+B31;
    O31 = A0-B31;
    O1 = A1+B30;
    O30 = A1-B30;
    O2 = A2+B29;
    O29 = A2-B29;
    O3 = A3+B28;
    O28 = A3-B28;
    O4 = A4+A27;
    O27 = A4-A27;
    O5 = A5+A26;
    O26 = A5-A26;
    O6 = A6+A25;
    O25 = A6-A25;
    O7 = A7+A24;
    O24 = A7-A24;
    O8 = A8+A23;
    O23 = A8-A23;
    O9 = A9+A22;
    O22 = A9-A22;
    O10 = A10+A21;
    O21 = A10-A21;
    O11 = A11+A20;
    O20 = A11-A20;
    O12 = A12+B19;
    O19 = A12-B19;
    O13 = A13+B18;
    O18 = A13-B18;
    O14 = A14+B17;
    O17 = A14-B17;
    O15 = A15+B16;
    O16 = A15-B16;
    O40 = xTrRound(724*(A55-A40), DenShift64);
    O55 = xTrRound(724*(A55+A40), DenShift64);
    O41 = xTrRound(724*(A54-A41), DenShift64);
    O54 = xTrRound(724*(A54+A41), DenShift64);
    O42 = xTrRound(724*(A53-A42), DenShift64);
    O53 = xTrRound(724*(A53+A42), DenShift64);
    O43 = xTrRound(724*(A52-A43), DenShift64);
    O52 = xTrRound(724*(A52+A43), DenShift64);
    O44 = xTrRound(724*(A51-A44), DenShift64);
    O51 = xTrRound(724*(A51+A44), DenShift64);
    O45 = xTrRound(724*(A50-A45), DenShift64);
    O50 = xTrRound(724*(A50+A45), DenShift64);
    O46 = xTrRound(724*(A49-A46), DenShift64);
    O49 = xTrRound(724*(A49+A46), DenShift64);
    O47 = xTrRound(724*(A48-A47), DenShift64);
    O48 = xTrRound(724*(A48+A47), DenShift64);

    aaiTemp[0][y] = O0+A63;
    aaiTemp[63][y] = O0-A63;
    aaiTemp[1][y] = O1+A62;
    aaiTemp[62][y] = O1-A62;
    aaiTemp[2][y] = O2+A61;
    aaiTemp[61][y] = O2-A61;
    aaiTemp[3][y] = O3+A60;
    aaiTemp[60][y] = O3-A60;
    aaiTemp[4][y] = O4+A59;
    aaiTemp[59][y] = O4-A59;
    aaiTemp[5][y] = O5+A58;
    aaiTemp[58][y] = O5-A58;
    aaiTemp[6][y] = O6+A57;
    aaiTemp[57][y] = O6-A57;
    aaiTemp[7][y] = O7+A56;
    aaiTemp[56][y] = O7-A56;
    aaiTemp[8][y] = O8+O55;
    aaiTemp[55][y] = O8-O55;
    aaiTemp[9][y] = O9+O54;
    aaiTemp[54][y] = O9-O54;
    aaiTemp[10][y] = O10+O53;
    aaiTemp[53][y] = O10-O53;
    aaiTemp[11][y] = O11+O52;
    aaiTemp[52][y] = O11-O52;
    aaiTemp[12][y] = O12+O51;
    aaiTemp[51][y] = O12-O51;
    aaiTemp[13][y] = O13+O50;
    aaiTemp[50][y] = O13-O50;
    aaiTemp[14][y] = O14+O49;
    aaiTemp[49][y] = O14-O49;
    aaiTemp[15][y] = O15+O48;
    aaiTemp[48][y] = O15-O48;
    aaiTemp[16][y] = O16+O47;
    aaiTemp[47][y] = O16-O47;
    aaiTemp[17][y] = O17+O46;
    aaiTemp[46][y] = O17-O46;
    aaiTemp[18][y] = O18+O45;
    aaiTemp[45][y] = O18-O45;
    aaiTemp[19][y] = O19+O44;
    aaiTemp[44][y] = O19-O44;
    aaiTemp[20][y] = O20+O43;
    aaiTemp[43][y] = O20-O43;
    aaiTemp[21][y] = O21+O42;
    aaiTemp[42][y] = O21-O42;
    aaiTemp[22][y] = O22+O41;
    aaiTemp[41][y] = O22-O41;
    aaiTemp[23][y] = O23+O40;
    aaiTemp[40][y] = O23-O40;
    aaiTemp[24][y] = O24+A39;
    aaiTemp[39][y] = O24-A39;
    aaiTemp[25][y] = O25+A38;
    aaiTemp[38][y] = O25-A38;
    aaiTemp[26][y] = O26+A37;
    aaiTemp[37][y] = O26-A37;
    aaiTemp[27][y] = O27+A36;
    aaiTemp[36][y] = O27-A36;
    aaiTemp[28][y] = O28+A35;
    aaiTemp[35][y] = O28-A35;
    aaiTemp[29][y] = O29+A34;
    aaiTemp[34][y] = O29-A34;
    aaiTemp[30][y] = O30+A33;
    aaiTemp[33][y] = O30-A33;
    aaiTemp[31][y] = O31+A32;
    aaiTemp[32][y] = O31-A32;

    pSrc += 64;
  }

  for( x=0 ; x<64 ; x++, pDes++ )
  {
    H32 = xTrRound(25*aaiTemp[x][1]-1024*aaiTemp[x][63], DenShift64);
    H33 = xTrRound(742*aaiTemp[x][33]-706*aaiTemp[x][31], DenShift64);
    H34 = xTrRound(415*aaiTemp[x][17]-936*aaiTemp[x][47], DenShift64);
    H35 = xTrRound(955*aaiTemp[x][49]-369*aaiTemp[x][15], DenShift64);
    H36 = xTrRound(224*aaiTemp[x][9]-999*aaiTemp[x][55], DenShift64);
    H37 = xTrRound(865*aaiTemp[x][41]-548*aaiTemp[x][23], DenShift64);
    H38 = xTrRound(590*aaiTemp[x][25]-837*aaiTemp[x][39], DenShift64);
    H39 = xTrRound(1009*aaiTemp[x][57]-175*aaiTemp[x][7], DenShift64);
    H40 = xTrRound(125*aaiTemp[x][5]-1016*aaiTemp[x][59], DenShift64);
    H41 = xTrRound(807*aaiTemp[x][37]-630*aaiTemp[x][27], DenShift64);
    H42 = xTrRound(505*aaiTemp[x][21]-891*aaiTemp[x][43], DenShift64);
    H43 = xTrRound(987*aaiTemp[x][53]-273*aaiTemp[x][11], DenShift64);
    H44 = xTrRound(321*aaiTemp[x][13]-972*aaiTemp[x][51], DenShift64);
    H45 = xTrRound(915*aaiTemp[x][45]-460*aaiTemp[x][19], DenShift64);
    H46 = xTrRound(669*aaiTemp[x][29]-775*aaiTemp[x][35], DenShift64);
    H47 = xTrRound(1021*aaiTemp[x][61]-75*aaiTemp[x][3], DenShift64);
    H48 = xTrRound(1021*aaiTemp[x][3]+75*aaiTemp[x][61], DenShift64);
    H49 = xTrRound(669*aaiTemp[x][35]+775*aaiTemp[x][29], DenShift64);
    H50 = xTrRound(915*aaiTemp[x][19]+460*aaiTemp[x][45], DenShift64);
    H51 = xTrRound(321*aaiTemp[x][51]+972*aaiTemp[x][13], DenShift64);
    H52 = xTrRound(987*aaiTemp[x][11]+273*aaiTemp[x][53], DenShift64);
    H53 = xTrRound(505*aaiTemp[x][43]+891*aaiTemp[x][21], DenShift64);
    H54 = xTrRound(807*aaiTemp[x][27]+630*aaiTemp[x][37], DenShift64);
    H55 = xTrRound(125*aaiTemp[x][59]+1016*aaiTemp[x][5], DenShift64);
    H56 = xTrRound(1009*aaiTemp[x][7]+175*aaiTemp[x][57], DenShift64);
    H57 = xTrRound(590*aaiTemp[x][39]+837*aaiTemp[x][25], DenShift64);
    H58 = xTrRound(865*aaiTemp[x][23]+548*aaiTemp[x][41], DenShift64);
    H59 = xTrRound(224*aaiTemp[x][55]+999*aaiTemp[x][9], DenShift64);
    H60 = xTrRound(955*aaiTemp[x][15]+369*aaiTemp[x][49], DenShift64);
    H61 = xTrRound(415*aaiTemp[x][47]+936*aaiTemp[x][17], DenShift64);
    H62 = xTrRound(742*aaiTemp[x][31]+706*aaiTemp[x][33], DenShift64);
    H63 = xTrRound(25*aaiTemp[x][63]+1024*aaiTemp[x][1], DenShift64);

    G16 = xTrRound(50*aaiTemp[x][2]-1023*aaiTemp[x][62], DenShift64);
    G17 = xTrRound(759*aaiTemp[x][34]-688*aaiTemp[x][30], DenShift64);
    G18 = xTrRound(438*aaiTemp[x][18]-926*aaiTemp[x][46], DenShift64);
    G19 = xTrRound(964*aaiTemp[x][50]-345*aaiTemp[x][14], DenShift64);
    G20 = xTrRound(249*aaiTemp[x][10]-993*aaiTemp[x][54], DenShift64);
    G21 = xTrRound(878*aaiTemp[x][42]-526*aaiTemp[x][22], DenShift64);
    G22 = xTrRound(610*aaiTemp[x][26]-822*aaiTemp[x][38], DenShift64);
    G23 = xTrRound(1013*aaiTemp[x][58]-150*aaiTemp[x][6], DenShift64);
    G24 = xTrRound(1013*aaiTemp[x][6]+150*aaiTemp[x][58], DenShift64);
    G25 = xTrRound(610*aaiTemp[x][38]+822*aaiTemp[x][26], DenShift64);
    G26 = xTrRound(878*aaiTemp[x][22]+526*aaiTemp[x][42], DenShift64);
    G27 = xTrRound(249*aaiTemp[x][54]+993*aaiTemp[x][10], DenShift64);
    G28 = xTrRound(964*aaiTemp[x][14]+345*aaiTemp[x][50], DenShift64);
    G29 = xTrRound(438*aaiTemp[x][46]+926*aaiTemp[x][18], DenShift64);
    G30 = xTrRound(759*aaiTemp[x][30]+688*aaiTemp[x][34], DenShift64);
    G31 = xTrRound(50*aaiTemp[x][62]+1023*aaiTemp[x][2], DenShift64);
    G32 = H32+H33;
    G33 = H32-H33;
    G34 = H35-H34;
    G35 = H35+H34;
    G36 = H36+H37;
    G37 = H36-H37;
    G38 = H39-H38;
    G39 = H39+H38;
    G40 = H40+H41;
    G41 = H40-H41;
    G42 = H43-H42;
    G43 = H43+H42;
    G44 = H44+H45;
    G45 = H44-H45;
    G46 = H47-H46;
    G47 = H47+H46;
    G48 = H48+H49;
    G49 = H48-H49;
    G50 = H51-H50;
    G51 = H51+H50;
    G52 = H52+H53;
    G53 = H52-H53;
    G54 = H55-H54;
    G55 = H55+H54;
    G56 = H56+H57;
    G57 = H56-H57;
    G58 = H59-H58;
    G59 = H59+H58;
    G60 = H60+H61;
    G61 = H60-H61;
    G62 = H63-H62;
    G63 = H63+H62;

    F8 = xTrRound(100*aaiTemp[x][4]-1019*aaiTemp[x][60], DenShift64);
    F9 = xTrRound(792*aaiTemp[x][36]-650*aaiTemp[x][28], DenShift64);
    F10 = xTrRound(483*aaiTemp[x][20]-903*aaiTemp[x][44], DenShift64);
    F11 = xTrRound(980*aaiTemp[x][52]-297*aaiTemp[x][12], DenShift64);
    F12 = xTrRound(980*aaiTemp[x][12]+297*aaiTemp[x][52], DenShift64);
    F13 = xTrRound(483*aaiTemp[x][44]+903*aaiTemp[x][20], DenShift64);
    F14 = xTrRound(792*aaiTemp[x][28]+650*aaiTemp[x][36], DenShift64);
    F15 = xTrRound(100*aaiTemp[x][60]+1019*aaiTemp[x][4], DenShift64);
    F16 = G16+G17;
    F17 = G16-G17;
    F18 = G19-G18;
    F19 = G19+G18;
    F20 = G20+G21;
    F21 = G20-G21;
    F22 = G23-G22;
    F23 = G23+G22;
    F24 = G24+G25;
    F25 = G24-G25;
    F26 = G27-G26;
    F27 = G27+G26;
    F28 = G28+G29;
    F29 = G28-G29;
    F30 = G31-G30;
    F31 = G31+G30;
    F33 = xTrRound(100*G62-1019*G33, DenShift64);
    F34 = xTrRound(-1019*G61-100*G34, DenShift64);
    F37 = xTrRound(792*G58-650*G37, DenShift64);
    F38 = xTrRound(-650*G57-792*G38, DenShift64);
    F41 = xTrRound(483*G54-903*G41, DenShift64);
    F42 = xTrRound(-903*G53-483*G42, DenShift64);
    F45 = xTrRound(980*G50-297*G45, DenShift64);
    F46 = xTrRound(-297*G49-980*G46, DenShift64);
    F49 = xTrRound(980*G49-297*G46, DenShift64);
    F50 = xTrRound(297*G50+980*G45, DenShift64);
    F53 = xTrRound(483*G53-903*G42, DenShift64);
    F54 = xTrRound(903*G54+483*G41, DenShift64);
    F57 = xTrRound(792*G57-650*G38, DenShift64);
    F58 = xTrRound(650*G58+792*G37, DenShift64);
    F61 = xTrRound(100*G61-1019*G34, DenShift64);
    F62 = xTrRound(1019*G62+100*G33, DenShift64);

    E4 = xTrRound(200*aaiTemp[x][8]-1004*aaiTemp[x][56], DenShift64);
    E5 = xTrRound(851*aaiTemp[x][40]-569*aaiTemp[x][24], DenShift64);
    E6 = xTrRound(851*aaiTemp[x][24]+569*aaiTemp[x][40], DenShift64);
    E7 = xTrRound(200*aaiTemp[x][56]+1004*aaiTemp[x][8], DenShift64);
    E8 = F8+F9;
    E9 = F8-F9;
    E10 = F11-F10;
    E11 = F11+F10;
    E12 = F12+F13;
    E13 = F12-F13;
    E14 = F15-F14;
    E15 = F15+F14;
    E17 = xTrRound(200*F30-1004*F17, DenShift64);
    E18 = xTrRound(-1004*F29-200*F18, DenShift64);
    E21 = xTrRound(851*F26-569*F21, DenShift64);
    E22 = xTrRound(-569*F25-851*F22, DenShift64);
    E25 = xTrRound(851*F25-569*F22, DenShift64);
    E26 = xTrRound(569*F26+851*F21, DenShift64);
    E29 = xTrRound(200*F29-1004*F18, DenShift64);
    E30 = xTrRound(1004*F30+200*F17, DenShift64);
    E32 = G32+G35;
    E33 = F33+F34;
    E34 = F33-F34;
    E35 = G32-G35;
    E36 = G39-G36;
    E37 = F38-F37;
    E38 = F38+F37;
    E39 = G39+G36;
    E40 = G40+G43;
    E41 = F41+F42;
    E42 = F41-F42;
    E43 = G40-G43;
    E44 = G47-G44;
    E45 = F46-F45;
    E46 = F46+F45;
    E47 = G47+G44;
    E48 = G48+G51;
    E49 = F49+F50;
    E50 = F49-F50;
    E51 = G48-G51;
    E52 = G55-G52;
    E53 = F54-F53;
    E54 = F54+F53;
    E55 = G55+G52;
    E56 = G56+G59;
    E57 = F57+F58;
    E58 = F57-F58;
    E59 = G56-G59;
    E60 = G63-G60;
    E61 = F62-F61;
    E62 = F62+F61;
    E63 = G63+G60;

    D0 = xTrRound(724*(aaiTemp[x][0]+aaiTemp[x][32]), DenShift64);
    D1 = xTrRound(724*(aaiTemp[x][0]-aaiTemp[x][32]), DenShift64);
    D2 = xTrRound(392*aaiTemp[x][16]-946*aaiTemp[x][48], DenShift64);
    D3 = xTrRound(946*aaiTemp[x][16]+392*aaiTemp[x][48], DenShift64);
    D4 = E4+E5;
    D5 = E4-E5;
    D6 = E7-E6;
    D7 = E7+E6;
    D9 = xTrRound(392*E14-946*E9, DenShift64);
    D10 = xTrRound(-946*E13-392*E10, DenShift64);
    D13 = xTrRound(392*E13-946*E10, DenShift64);
    D14 = xTrRound(946*E14+392*E9, DenShift64);
    D16 = F16+F19;
    D19 = F16-F19;
    D20 = F23-F20;
    D23 = F23+F20;
    D24 = F24+F27;
    D27 = F24-F27;
    D28 = F31-F28;
    D31 = F31+F28;
    D17 = E17+E18;
    D18 = E17-E18;
    D21 = E22-E21;
    D22 = E22+E21;
    D25 = E25+E26;
    D26 = E25-E26;
    D29 = E30-E29;
    D30 = E30+E29;
    D34 = xTrRound(200*E61-1004*E34, DenShift64);
    D35 = xTrRound(200*E60-1004*E35, DenShift64);
    D36 = xTrRound(-1004*E59-200*E36, DenShift64);
    D37 = xTrRound(-1004*E58-200*E37, DenShift64);
    D42 = xTrRound(851*E53-569*E42, DenShift64);
    D43 = xTrRound(851*E52-569*E43, DenShift64);
    D44 = xTrRound(-569*E51-851*E44, DenShift64);
    D45 = xTrRound(-569*E50-851*E45, DenShift64);
    D50 = xTrRound(851*E50-569*E45, DenShift64);
    D51 = xTrRound(851*E51-569*E44, DenShift64);
    D52 = xTrRound(569*E52+851*E43, DenShift64);
    D53 = xTrRound(569*E53+851*E42, DenShift64);
    D58 = xTrRound(200*E58-1004*E37, DenShift64);
    D59 = xTrRound(200*E59-1004*E36, DenShift64);
    D60 = xTrRound(1004*E60+200*E35, DenShift64);
    D61 = xTrRound(1004*E61+200*E34, DenShift64);

    C0 = D0+D3;
    C3 = D0-D3;
    C8 = E8+E11;
    C11 = E8-E11;
    C12 = E15-E12;
    C15 = E15+E12;
    C1 = D1+D2;
    C2 = D1-D2;
    C9 = D9+D10;
    C10 = D9-D10;
    C13 = D14-D13;
    C14 = D14+D13;
    C5 = xTrRound(724*(D6-D5), DenShift64);
    C6 = xTrRound(724*(D6+D5), DenShift64);
    C18 = xTrRound(392*D29-946*D18, DenShift64);
    C20 = xTrRound(-946*D27-392*D20, DenShift64);
    C26 = xTrRound(-946*D21+392*D26, DenShift64);
    C28 = xTrRound(392*D19+946*D28, DenShift64);
    C19 = xTrRound(392*D28-946*D19, DenShift64);
    C21 = xTrRound(-946*D26-392*D21, DenShift64);
    C27 = xTrRound(-946*D20+392*D27, DenShift64);
    C29 = xTrRound(392*D18+946*D29, DenShift64);
    C32 = E32+E39;
    C39 = E32-E39;
    C40 = E47-E40;
    C47 = E47+E40;
    C48 = E48+E55;
    C55 = E48-E55;
    C56 = E63-E56;
    C63 = E63+E56;
    C33 = E33+E38;
    C38 = E33-E38;
    C41 = E46-E41;
    C46 = E46+E41;
    C49 = E49+E54;
    C54 = E49-E54;
    C57 = E62-E57;
    C62 = E62+E57;
    C34 = D34+D37;
    C37 = D34-D37;
    C42 = D45-D42;
    C45 = D45+D42;
    C50 = D50+D53;
    C53 = D50-D53;
    C58 = D61-D58;
    C61 = D61+D58;
    C35 = D35+D36;
    C36 = D35-D36;
    C43 = D44-D43;
    C44 = D44+D43;
    C51 = D51+D52;
    C52 = D51-D52;
    C59 = D60-D59;
    C60 = D60+D59;

    B0 = C0+D7;
    B7 = C0-D7;
    B1 = C1+C6;
    B6 = C1-C6;
    B2 = C2+C5;
    B5 = C2-C5;
    B3 = C3+D4;
    B4 = C3-D4;
    B10 = xTrRound(724*(C13-C10), DenShift64);
    B13 = xTrRound(724*(C13+C10), DenShift64);
    B11 = xTrRound(724*(C12-C11), DenShift64);
    B12 = xTrRound(724*(C12+C11), DenShift64);
    B16 = D16+D23;
    B23 = D16-D23;
    B24 = D31-D24;
    B31 = D31+D24;
    B17 = D17+D22;
    B22 = D17-D22;
    B25 = D30-D25;
    B30 = D30+D25;
    B18 = C18+C21;
    B21 = C18-C21;
    B26 = C29-C26;
    B29 = C29+C26;
    B19 = C19+C20;
    B20 = C19-C20;
    B27 = C28-C27;
    B28 = C28+C27;
    B36 = xTrRound(392*C59-946*C36, DenShift64);
    B40 = xTrRound(-946*C55-392*C40, DenShift64);
    B52 = xTrRound(-946*C43+392*C52, DenShift64);
    B56 = xTrRound(392*C39+946*C56, DenShift64);
    B37 = xTrRound(392*C58-946*C37, DenShift64);
    B41 = xTrRound(-946*C54-392*C41, DenShift64);
    B53 = xTrRound(-946*C42+392*C53, DenShift64);
    B57 = xTrRound(392*C38+946*C57, DenShift64);
    B38 = xTrRound(392*C57-946*C38, DenShift64);
    B42 = xTrRound(-946*C53-392*C42, DenShift64);
    B54 = xTrRound(-946*C41+392*C54, DenShift64);
    B58 = xTrRound(392*C37+946*C58, DenShift64);
    B39 = xTrRound(392*C56-946*C39, DenShift64);
    B43 = xTrRound(-946*C52-392*C43, DenShift64);
    B55 = xTrRound(-946*C40+392*C55, DenShift64);
    B59 = xTrRound(392*C36+946*C59, DenShift64);

    A0 = B0+C15;
    A15 = B0-C15;
    A1 = B1+C14;
    A14 = B1-C14;
    A2 = B2+B13;
    A13 = B2-B13;
    A3 = B3+B12;
    A12 = B3-B12;
    A4 = B4+B11;
    A11 = B4-B11;
    A5 = B5+B10;
    A10 = B5-B10;
    A6 = B6+C9;
    A9 = B6-C9;
    A7 = B7+C8;
    A8 = B7-C8;
    A20 = xTrRound(724*(B27-B20), DenShift64);
    A27 = xTrRound(724*(B27+B20), DenShift64);
    A21 = xTrRound(724*(B26-B21), DenShift64);
    A26 = xTrRound(724*(B26+B21), DenShift64);
    A22 = xTrRound(724*(B25-B22), DenShift64);
    A25 = xTrRound(724*(B25+B22), DenShift64);
    A23 = xTrRound(724*(B24-B23), DenShift64);
    A24 = xTrRound(724*(B24+B23), DenShift64);
    A32 = C32+C47;
    A47 = C32-C47;
    A48 = C63-C48;
    A63 = C63+C48;
    A33 = C33+C46;
    A46 = C33-C46;
    A49 = C62-C49;
    A62 = C62+C49;
    A34 = C34+C45;
    A45 = C34-C45;
    A50 = C61-C50;
    A61 = C61+C50;
    A35 = C35+C44;
    A44 = C35-C44;
    A51 = C60-C51;
    A60 = C60+C51;
    A36 = B36+B43;
    A43 = B36-B43;
    A52 = B59-B52;
    A59 = B59+B52;
    A37 = B37+B42;
    A42 = B37-B42;
    A53 = B58-B53;
    A58 = B58+B53;
    A38 = B38+B41;
    A41 = B38-B41;
    A54 = B57-B54;
    A57 = B57+B54;
    A39 = B39+B40;
    A40 = B39-B40;
    A55 = B56-B55;
    A56 = B56+B55;

    O0 = A0+B31;
    O31 = A0-B31;
    O1 = A1+B30;
    O30 = A1-B30;
    O2 = A2+B29;
    O29 = A2-B29;
    O3 = A3+B28;
    O28 = A3-B28;
    O4 = A4+A27;
    O27 = A4-A27;
    O5 = A5+A26;
    O26 = A5-A26;
    O6 = A6+A25;
    O25 = A6-A25;
    O7 = A7+A24;
    O24 = A7-A24;
    O8 = A8+A23;
    O23 = A8-A23;
    O9 = A9+A22;
    O22 = A9-A22;
    O10 = A10+A21;
    O21 = A10-A21;
    O11 = A11+A20;
    O20 = A11-A20;
    O12 = A12+B19;
    O19 = A12-B19;
    O13 = A13+B18;
    O18 = A13-B18;
    O14 = A14+B17;
    O17 = A14-B17;
    O15 = A15+B16;
    O16 = A15-B16;
    O40 = xTrRound(724*(A55-A40), DenShift64);
    O55 = xTrRound(724*(A55+A40), DenShift64);
    O41 = xTrRound(724*(A54-A41), DenShift64);
    O54 = xTrRound(724*(A54+A41), DenShift64);
    O42 = xTrRound(724*(A53-A42), DenShift64);
    O53 = xTrRound(724*(A53+A42), DenShift64);
    O43 = xTrRound(724*(A52-A43), DenShift64);
    O52 = xTrRound(724*(A52+A43), DenShift64);
    O44 = xTrRound(724*(A51-A44), DenShift64);
    O51 = xTrRound(724*(A51+A44), DenShift64);
    O45 = xTrRound(724*(A50-A45), DenShift64);
    O50 = xTrRound(724*(A50+A45), DenShift64);
    O46 = xTrRound(724*(A49-A46), DenShift64);
    O49 = xTrRound(724*(A49+A46), DenShift64);
    O47 = xTrRound(724*(A48-A47), DenShift64);
    O48 = xTrRound(724*(A48+A47), DenShift64);

    pDes[         0] = (Pel)xTrRound( O0+A63, DCore64Shift);
    pDes[uiStride  ] = (Pel)xTrRound( O1+A62, DCore64Shift);
    pDes[uiStride2 ] = (Pel)xTrRound( O2+A61, DCore64Shift);
    pDes[uiStride3 ] = (Pel)xTrRound( O3+A60, DCore64Shift);
    pDes[uiStride4 ] = (Pel)xTrRound( O4+A59, DCore64Shift);
    pDes[uiStride5 ] = (Pel)xTrRound( O5+A58, DCore64Shift);
    pDes[uiStride6 ] = (Pel)xTrRound( O6+A57, DCore64Shift);
    pDes[uiStride7 ] = (Pel)xTrRound( O7+A56, DCore64Shift);
    pDes[uiStride8 ] = (Pel)xTrRound( O8+O55, DCore64Shift);
    pDes[uiStride9 ] = (Pel)xTrRound( O9+O54, DCore64Shift);
    pDes[uiStride10] = (Pel)xTrRound( O10+O53, DCore64Shift);
    pDes[uiStride11] = (Pel)xTrRound( O11+O52, DCore64Shift);
    pDes[uiStride12] = (Pel)xTrRound( O12+O51, DCore64Shift);
    pDes[uiStride13] = (Pel)xTrRound( O13+O50, DCore64Shift);
    pDes[uiStride14] = (Pel)xTrRound( O14+O49, DCore64Shift);
    pDes[uiStride15] = (Pel)xTrRound( O15+O48, DCore64Shift);
    pDes[uiStride16] = (Pel)xTrRound( O16+O47, DCore64Shift);
    pDes[uiStride17] = (Pel)xTrRound( O17+O46, DCore64Shift);
    pDes[uiStride18] = (Pel)xTrRound( O18+O45, DCore64Shift);
    pDes[uiStride19] = (Pel)xTrRound( O19+O44, DCore64Shift);
    pDes[uiStride20] = (Pel)xTrRound( O20+O43, DCore64Shift);
    pDes[uiStride21] = (Pel)xTrRound( O21+O42, DCore64Shift);
    pDes[uiStride22] = (Pel)xTrRound( O22+O41, DCore64Shift);
    pDes[uiStride23] = (Pel)xTrRound( O23+O40, DCore64Shift);
    pDes[uiStride24] = (Pel)xTrRound( O24+A39, DCore64Shift);
    pDes[uiStride25] = (Pel)xTrRound( O25+A38, DCore64Shift);
    pDes[uiStride26] = (Pel)xTrRound( O26+A37, DCore64Shift);
    pDes[uiStride27] = (Pel)xTrRound( O27+A36, DCore64Shift);
    pDes[uiStride28] = (Pel)xTrRound( O28+A35, DCore64Shift);
    pDes[uiStride29] = (Pel)xTrRound( O29+A34, DCore64Shift);
    pDes[uiStride30] = (Pel)xTrRound( O30+A33, DCore64Shift);
    pDes[uiStride31] = (Pel)xTrRound( O31+A32, DCore64Shift);
    pDes[uiStride32] = (Pel)xTrRound( O31-A32, DCore64Shift);
    pDes[uiStride33] = (Pel)xTrRound( O30-A33, DCore64Shift);
    pDes[uiStride34] = (Pel)xTrRound( O29-A34, DCore64Shift);
    pDes[uiStride35] = (Pel)xTrRound( O28-A35, DCore64Shift);
    pDes[uiStride36] = (Pel)xTrRound( O27-A36, DCore64Shift);
    pDes[uiStride37] = (Pel)xTrRound( O26-A37, DCore64Shift);
    pDes[uiStride38] = (Pel)xTrRound( O25-A38, DCore64Shift);
    pDes[uiStride39] = (Pel)xTrRound( O24-A39, DCore64Shift);
    pDes[uiStride40] = (Pel)xTrRound( O23-O40, DCore64Shift);
    pDes[uiStride41] = (Pel)xTrRound( O22-O41, DCore64Shift);
    pDes[uiStride42] = (Pel)xTrRound( O21-O42, DCore64Shift);
    pDes[uiStride43] = (Pel)xTrRound( O20-O43, DCore64Shift);
    pDes[uiStride44] = (Pel)xTrRound( O19-O44, DCore64Shift);
    pDes[uiStride45] = (Pel)xTrRound( O18-O45, DCore64Shift);
    pDes[uiStride46] = (Pel)xTrRound( O17-O46, DCore64Shift);
    pDes[uiStride47] = (Pel)xTrRound( O16-O47, DCore64Shift);
    pDes[uiStride48] = (Pel)xTrRound( O15-O48, DCore64Shift);
    pDes[uiStride49] = (Pel)xTrRound( O14-O49, DCore64Shift);
    pDes[uiStride50] = (Pel)xTrRound( O13-O50, DCore64Shift);
    pDes[uiStride51] = (Pel)xTrRound( O12-O51, DCore64Shift);
    pDes[uiStride52] = (Pel)xTrRound( O11-O52, DCore64Shift);
    pDes[uiStride53] = (Pel)xTrRound( O10-O53, DCore64Shift);
    pDes[uiStride54] = (Pel)xTrRound( O9-O54, DCore64Shift);
    pDes[uiStride55] = (Pel)xTrRound( O8-O55, DCore64Shift);
    pDes[uiStride56] = (Pel)xTrRound( O7-A56, DCore64Shift);
    pDes[uiStride57] = (Pel)xTrRound( O6-A57, DCore64Shift);
    pDes[uiStride58] = (Pel)xTrRound( O5-A58, DCore64Shift);
    pDes[uiStride59] = (Pel)xTrRound( O4-A59, DCore64Shift);
    pDes[uiStride60] = (Pel)xTrRound( O3-A60, DCore64Shift);
    pDes[uiStride61] = (Pel)xTrRound( O2-A61, DCore64Shift);
    pDes[uiStride62] = (Pel)xTrRound( O1-A62, DCore64Shift);
    pDes[uiStride63] = (Pel)xTrRound( O0-A63, DCore64Shift);
  }
}

Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Bool bUseRDOQ, Bool bEnc )
{
	m_uiMaxTrSize	 = uiMaxTrSize;
	m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;

	if ( m_bEnc )
	{
		m_cQP.initOffsetParam( MIN_QP, MAX_QP );
	}
}

Void TComTrQuant::xQuant( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  xQuantLTR(pcCU, pSrc, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx, indexROT );
}
Void TComTrQuant::xDeQuant( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight, UChar indexROT )
{
  xDeQuantLTR( pSrc, pDes, iWidth, iHeight, indexROT );
}

Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  uiAbsSum = 0;

  assert( (pcCU->getSlice()->getSPS()->getMaxTrSize() >= uiWidth) );

  xT( pcResidual, uiStride, m_plTempCoeff, uiWidth );
  xQuant( pcCU, m_plTempCoeff, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx, indexROT );
}

Void TComTrQuant::invtransformNxN( Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight, UChar indexROT )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight, indexROT );
  xIT( m_plTempCoeff, rpcResidual, uiStride, uiWidth );
}

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

  for( Int i = 0; i < 8; i++, piBlkResi += uiStride )
  {
    Int ai1 [8];
    Int ai2 [8];

    ai1[0] = piBlkResi[0] + piBlkResi[7];
    ai1[1] = piBlkResi[1] + piBlkResi[6];
    ai1[2] = piBlkResi[2] + piBlkResi[5];
    ai1[3] = piBlkResi[3] + piBlkResi[4];

    ai1[4] = piBlkResi[0] - piBlkResi[7];
    ai1[5] = piBlkResi[1] - piBlkResi[6];
    ai1[6] = piBlkResi[2] - piBlkResi[5];
    ai1[7] = piBlkResi[3] - piBlkResi[4];

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
  }
}

Void TComTrQuant::xQuant2x2( Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, UChar indexROT )
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
Void TComTrQuant::xQuant4x4( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  for( Int i=0; i<16; i++ )
    plSrcCoef[i] *= m_puiQuantMtx[i];

	if ( indexROT )
	{
		RotTransform4I( plSrcCoef, indexROT );
	}

  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) && (!RDOQ_ROT_IDX0_ONLY || indexROT == 0 ) )
  {
    xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 4, 4, uiAbsSum, eTType, uiAbsPartIdx, indexROT);
  }
  else
  {
    for( Int n = 0; n < 16; n++ )
    {
      Int iLevel, iSign;

      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) ;

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

Void TComTrQuant::xQuant8x8( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  for( Int i=0; i<64; i++ )
    plSrcCoef[i] *= m_puiQuantMtx[i];

	if ( indexROT )
	{
		RotTransformLI2( plSrcCoef, indexROT );
	}

  Int iBit = m_cQP.m_iBits + 1;

  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) && (!RDOQ_ROT_IDX0_ONLY || indexROT == 0 ) )
  {
    xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 8, 8, uiAbsSum, eTType, uiAbsPartIdx, indexROT);
  }
  else
  {

    for( Int n = 0; n < 64; n++ )
    {
      Int iLevel, iSign;

      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) ;

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

    ai1[0] = pi[0] + pi[4];
    ai1[2] = pi[0] - pi[4];

    ai1[4] = (pi[2]>>1) -  pi[6];
    ai1[6] =  pi[2]     + (pi[6]>>1);

    ai1[1] = pi[5] - pi[3] - pi[7] - (pi[7]>>1);
    ai1[3] = pi[1] + pi[7] - pi[3] - (pi[3]>>1);;
    ai1[5] = pi[7] - pi[1] + pi[5] + (pi[5]>>1);
    ai1[7] = pi[3] + pi[5] + pi[1] + (pi[1]>>1);

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
  }
}

Void TComTrQuant::xDeQuant2x2( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT )
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

Void TComTrQuant::xDeQuant4x4( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT )
{
  Int iLevel;
  Int iDeScale;

	if ( !indexROT )
	{
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
	else
	{
    for( Int n=0; n<16; n++)
    {
      if( 0 != pSrcCoef[n] )
        rplDstCoef[n] = pSrcCoef[n] * ( 1 << m_cQP.m_iPer );
      else
        rplDstCoef[n] = 0;
    }

    InvRotTransform4I( rplDstCoef, indexROT );
	}
}
Void TComTrQuant::xDeQuant8x8( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT )
{
  Int iLevel;
  Int iDeScale;

  Int iAdd = ( 1 << 5 ) >> m_cQP.m_iPer;

  if ( !indexROT )
	{

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
  else
  {
		for( Int n = 0; n < 64; n++ )
		{
			iLevel  = pSrcCoef[n];

			if( 0 != iLevel )
			{
				iDeScale = 1;
				rplDstCoef[n]   = ( (iLevel*iDeScale*16 + iAdd) << m_cQP.m_iPer ) >> 6;
			}
			else
			{
				rplDstCoef[n] = 0;
			}
		}

    InvRotTransformLI2( rplDstCoef, indexROT );

    for( Int i=0; i<64; i++ )
    {
      if( rplDstCoef[i] != 0 )
        rplDstCoef[i] *= g_aiDequantCoef64[m_cQP.m_iRem][i];
    }
	}
}

Void TComTrQuant::invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff, Int indexROT )
{
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
      return;

  if ( uiTrMode == uiMaxTrMode )
  {
    Pel* pResi = rpcResidual + uiAddr;
    invtransformNxN( pResi, uiStride, rpcCoeff, uiWidth, uiHeight, indexROT );
  }
  else
  {
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                         , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, indexROT ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth               , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, indexROT ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, indexROT ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff, indexROT );
  }
}

// ROT_SHIFT_ONLY
Void
TComTrQuant::InvRotTransform4I(  Long* matrix, UChar index )
{
  Int t[8][16];
  Int r[2],m[6];
  Int itemp = 0;

  Int n = 0;
  for (n=0; n<13; n+=4)
  {
    t[0][n]=0;   t[1][n]=0;  t[2][n]=0;

    if (matrix[n]>=0) {m[0] = matrix[n]>>13;		m[1]=matrix[n]-(m[0]<<13);}
    else              {m[0] = (abs(matrix[n]))>>13;	m[1]=matrix[n]+(m[0]<<13);  m[0]=-m[0];}
    r[0] = ROT_MATRIX_S[0][index][16]; r[1] = ROT_MATRIX_S[1][index][16];
    t[0][n] += r[0]*m[0]; t[1][n] +=r[0]*m[1]+m[0]*r[1]; t[2][n]  += m[1]*r[1];

    if (matrix[n+1]>=0) {m[2] = matrix[n+1]>>13;		m[3]=matrix[n+1]-(m[2]<<13);}
    else                {m[2] = (abs(matrix[n+1]))>>13;	m[3]=matrix[n+1]+(m[2]<<13);  m[2]=-m[2];}
    r[0] = ROT_MATRIX_S[0][index][20];   r[1] = ROT_MATRIX_S[1][index][20];
    t[0][n] += r[0]*m[2]; t[1][n] +=r[0]*m[3]+m[2]*r[1]; t[2][n]  += m[3]*r[1];

    if (matrix[n+2]>=0) {m[4] = matrix[n+2]>>13;		m[5]=matrix[n+2]-(m[4]<<13);}
    else              {m[4] = (abs(matrix[n+2]))>>13;	m[5]=matrix[n+2]+(m[4]<<13);  m[4]=-m[4];}
    r[0] = ROT_MATRIX_S[0][index][24];   r[1] = ROT_MATRIX_S[1][index][24];
    t[0][n] += r[0]*m[4]; t[1][n] +=r[0]*m[5]+m[4]*r[1]; t[2][n]  += m[5]*r[1];


    t[0][n+1]=0;   t[1][n+1]=0;  t[2][n+1]=0;

    r[0] = ROT_MATRIX_S[0][index][17];   r[1] = ROT_MATRIX_S[1][index][17];
    t[0][n+1] += r[0]*m[0]; t[1][n+1] +=r[0]*m[1]+m[0]*r[1]; t[2][n+1]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][21];   r[1] = ROT_MATRIX_S[1][index][21];
    t[0][n+1] += r[0]*m[2]; t[1][n+1] +=r[0]*m[3]+m[2]*r[1]; t[2][n+1]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][25];   r[1] = ROT_MATRIX_S[1][index][25];
    t[0][n+1] += r[0]*m[4]; t[1][n+1] +=r[0]*m[5]+m[4]*r[1]; t[2][n+1]  += m[5]*r[1];


    t[0][n+2]=0;   t[1][n+2]=0;  t[2][n+2]=0;

    r[0] = ROT_MATRIX_S[0][index][18];   r[1] = ROT_MATRIX_S[1][index][18];
    t[0][n+2] += r[0]*m[0]; t[1][n+2] +=r[0]*m[1]+m[0]*r[1]; t[2][n+2]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][22];   r[1] = ROT_MATRIX_S[1][index][22];
    t[0][n+2] += r[0]*m[2]; t[1][n+2] +=r[0]*m[3]+m[2]*r[1]; t[2][n+2]  += m[3]*r[1];



    r[0] = ROT_MATRIX_S[0][index][26];   r[1] = ROT_MATRIX_S[1][index][26];
    t[0][n+2] += r[0]*m[4]; t[1][n+2] +=r[0]*m[5]+m[4]*r[1]; t[2][n+2]  += m[5]*r[1];


    t[0][n+3] = matrix[n+3]; t[1][n+3] = 0; t[2][n+3] = 0;
  }
  Int res[4];
  for (n=0; n<4; n++)
  {
    res[0]=0;res[1]=0;res[2]=0;

    if (t[0][n]>=0)   {t[3][n] = t[0][n]>>13;		    t[4][n]=t[0][n]-(t[3][n]<<13);}
    else              {t[3][n] = (abs(t[0][n]))>>13;	t[4][n]=t[0][n]+(t[3][n]<<13);  t[3][n]=-t[3][n];}

    if (t[1][n]>=0)   {t[5][n] = t[1][n]>>13;		    t[6][n]=t[1][n]-(t[5][n]<<13);}
    else              {t[5][n] = (abs(t[1][n]))>>13;	t[6][n]=t[1][n]+(t[5][n]<<13);  t[5][n]=-t[5][n];}

    if (t[2][n]>=0)   {t[7][n] = t[2][n]>>13;		   }
    else              {t[7][n] = (abs(t[2][n]))>>13;	t[7][n]=-t[7][n];}

    if (ROT_MATRIX[index][0]*g_aiDequantCoef[m_cQP.m_iRem][n]>=0)
    {r[0] = (ROT_MATRIX[index][0]*g_aiDequantCoef[m_cQP.m_iRem][n])>>13;		    r[1]=(ROT_MATRIX[index][0]*g_aiDequantCoef[m_cQP.m_iRem][n])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][0]*g_aiDequantCoef[m_cQP.m_iRem][n]))>>13;	r[1]=(ROT_MATRIX[index][0]*g_aiDequantCoef[m_cQP.m_iRem][n])+(r[0]<<13);  r[0]=-r[0];}


    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    if (t[0][n+4]>=0)   {t[3][n+4] = t[0][n+4]>>13;		    t[4][n+4]=t[0][n+4]-(t[3][n+4]<<13);}
    else              {t[3][n+4] = (abs(t[0][n+4]))>>13;	t[4][n+4]=t[0][n+4]+(t[3][n+4]<<13);  t[3][n+4]=-t[3][n+4];}

    if (t[1][n+4]>=0)   {t[5][n+4] = t[1][n+4]>>13;		    t[6][n+4]=t[1][n+4]-(t[5][n+4]<<13);}
    else              {t[5][n+4] = (abs(t[1][n+4]))>>13;	t[6][n+4]=t[1][n+4]+(t[5][n+4]<<13);  t[5][n+4]=-t[5][n+4];}

    if (t[2][n+4]>=0)   {t[7][n+4] = t[2][n+4]>>13;		   }
    else              {t[7][n+4] = (abs(t[2][n+4]))>>13;	t[7][n+4]=-t[7][n+4];}

    if (ROT_MATRIX[index][4]*g_aiDequantCoef[m_cQP.m_iRem][n]>=0)
    {r[0] = (ROT_MATRIX[index][4]*g_aiDequantCoef[m_cQP.m_iRem][n])>>13;		    r[1]=(ROT_MATRIX[index][4]*g_aiDequantCoef[m_cQP.m_iRem][n])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][4]*g_aiDequantCoef[m_cQP.m_iRem][n]))>>13;	r[1]=(ROT_MATRIX[index][4]*g_aiDequantCoef[m_cQP.m_iRem][n])+(r[0]<<13);  r[0]=-r[0];}


    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];

    if (t[0][n+8]>=0)   {t[3][n+8] = t[0][n+8]>>13;		    t[4][n+8]=t[0][n+8]-(t[3][n+8]<<13);}
    else              {t[3][n+8] = (abs(t[0][n+8]))>>13;	t[4][n+8]=t[0][n+8]+(t[3][n+8]<<13);  t[3][n+8]=-t[3][n+8];}

    if (t[1][n+8]>=0)   {t[5][n+8] = t[1][n+8]>>13;		    t[6][n+8]=t[1][n+8]-(t[5][n+8]<<13);}
    else              {t[5][n+8] = (abs(t[1][n+8]))>>13;	t[6][n+8]=t[1][n+8]+(t[5][n+8]<<13);  t[5][n+8]=-t[5][n+8];}

    if (t[2][n+8]>=0)   {t[7][n+8] = t[2][n+8]>>13;		   }
    else              {t[7][n+8] = (abs(t[2][n+8]))>>13;	t[7][n+8]=-t[7][n+8];}

    if (ROT_MATRIX[index][8]*g_aiDequantCoef[m_cQP.m_iRem][n]>=0)
    {r[0] = (ROT_MATRIX[index][8]*g_aiDequantCoef[m_cQP.m_iRem][n])>>13;
    r[1]=(ROT_MATRIX[index][8]*g_aiDequantCoef[m_cQP.m_iRem][n])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][8]*g_aiDequantCoef[m_cQP.m_iRem][n]))>>13;
    r[1]=(ROT_MATRIX[index][8]*g_aiDequantCoef[m_cQP.m_iRem][n])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n] += itemp;

    res[0]=0;res[1]=0;res[2]=0;

    if (ROT_MATRIX[index][1]*g_aiDequantCoef[m_cQP.m_iRem][n+4]>=0)
    {r[0] = (ROT_MATRIX[index][1]*g_aiDequantCoef[m_cQP.m_iRem][n+4])>>13;		    r[1]=(ROT_MATRIX[index][1]*g_aiDequantCoef[m_cQP.m_iRem][n+4])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][1]*g_aiDequantCoef[m_cQP.m_iRem][n+4]))>>13;	r[1]=(ROT_MATRIX[index][1]*g_aiDequantCoef[m_cQP.m_iRem][n+4])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];

    if (ROT_MATRIX[index][5]*g_aiDequantCoef[m_cQP.m_iRem][n+4]>=0)
    {r[0] = (ROT_MATRIX[index][5]*g_aiDequantCoef[m_cQP.m_iRem][n+4])>>13;		    r[1]=(ROT_MATRIX[index][5]*g_aiDequantCoef[m_cQP.m_iRem][n+4])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][5]*g_aiDequantCoef[m_cQP.m_iRem][n+4]))>>13;	r[1]=(ROT_MATRIX[index][5]*g_aiDequantCoef[m_cQP.m_iRem][n+4])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];

    if (ROT_MATRIX[index][9]*g_aiDequantCoef[m_cQP.m_iRem][n+4]>=0)
    {r[0] = (ROT_MATRIX[index][9]*g_aiDequantCoef[m_cQP.m_iRem][n+4])>>13;		    r[1]=(ROT_MATRIX[index][9]*g_aiDequantCoef[m_cQP.m_iRem][n+4])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][9]*g_aiDequantCoef[m_cQP.m_iRem][n+4]))>>13;	r[1]=(ROT_MATRIX[index][9]*g_aiDequantCoef[m_cQP.m_iRem][n+4])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+4] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+4] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+4] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+4] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    if (ROT_MATRIX[index][2]*g_aiDequantCoef[m_cQP.m_iRem][n+8]>=0)
    {r[0] = (ROT_MATRIX[index][2]*g_aiDequantCoef[m_cQP.m_iRem][n+8])>>13;		    r[1]=(ROT_MATRIX[index][2]*g_aiDequantCoef[m_cQP.m_iRem][n+8])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][2]*g_aiDequantCoef[m_cQP.m_iRem][n+8]))>>13;	r[1]=(ROT_MATRIX[index][2]*g_aiDequantCoef[m_cQP.m_iRem][n+8])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];

    if (ROT_MATRIX[index][6]*g_aiDequantCoef[m_cQP.m_iRem][n+8]>=0)
    {r[0] = (ROT_MATRIX[index][6]*g_aiDequantCoef[m_cQP.m_iRem][n+8])>>13;		    r[1]=(ROT_MATRIX[index][6]*g_aiDequantCoef[m_cQP.m_iRem][n+8])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][6]*g_aiDequantCoef[m_cQP.m_iRem][n+8]))>>13;	r[1]=(ROT_MATRIX[index][6]*g_aiDequantCoef[m_cQP.m_iRem][n+8])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];

    if (ROT_MATRIX[index][10]*g_aiDequantCoef[m_cQP.m_iRem][n+8]>=0)
    {r[0] = (ROT_MATRIX[index][10]*g_aiDequantCoef[m_cQP.m_iRem][n+8])>>13;		    r[1]=(ROT_MATRIX[index][10]*g_aiDequantCoef[m_cQP.m_iRem][n+8])-(r[0]<<13);}
    else
    {r[0] = (abs(ROT_MATRIX[index][10]*g_aiDequantCoef[m_cQP.m_iRem][n+8]))>>13;	r[1]=(ROT_MATRIX[index][10]*g_aiDequantCoef[m_cQP.m_iRem][n+8])+(r[0]<<13);  r[0]=-r[0];}

    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+8] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+8] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+8] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+8] += itemp;

    res[0]=t[0][n+12]*g_aiDequantCoef[m_cQP.m_iRem][n+12];
    res[1]=t[1][n+12]*g_aiDequantCoef[m_cQP.m_iRem][n+12];
    res[2]=t[2][n+12]*g_aiDequantCoef[m_cQP.m_iRem][n+12];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+12] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+12] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+12] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+12] += itemp;
  }
}
Void
TComTrQuant::RotTransform4I( Long* matrix, UChar index )
{
  if (index==0) return;

  Int m[6],r[2],t[8][16];
  Int itemp = 0;
  Int n = 0;

  for (n=0; n<4; n++)
  {
    t[0][n]=0;   t[1][n]=0;  t[2][n]=0;

    if (matrix[n]>=0) {m[0] = matrix[n]>>13;		m[1]=matrix[n]-(m[0]<<13);}
    else              {m[0] = (abs(matrix[n]))>>13;	m[1]=matrix[n]+(m[0]<<13);  m[0]=-m[0];}
    r[0] = ROT_MATRIX_S[0][index][0]; r[1] = ROT_MATRIX_S[1][index][0];
    t[0][n] += r[0]*m[0]; t[1][n] +=r[0]*m[1]+m[0]*r[1]; t[2][n]  += m[1]*r[1];

    if (matrix[n+4]>=0) {m[2] = matrix[n+4]>>13;		m[3]=matrix[n+4]-(m[2]<<13);}
    else                {m[2] = (abs(matrix[n+4]))>>13;	m[3]=matrix[n+4]+(m[2]<<13);  m[2]=-m[2];}
    r[0] = ROT_MATRIX_S[0][index][1]; r[1] = ROT_MATRIX_S[1][index][1];
    t[0][n] += r[0]*m[2]; t[1][n] +=r[0]*m[3]+m[2]*r[1]; t[2][n]  += m[3]*r[1];

    if (matrix[n+8]>=0) {m[4] = matrix[n+8]>>13;		m[5]=matrix[n+8]-(m[4]<<13);}
    else                {m[4] = (abs(matrix[n+8]))>>13;	m[5]=matrix[n+8]+(m[4]<<13);  m[4]=-m[4];}
    r[0] = ROT_MATRIX_S[0][index][2]; r[1] = ROT_MATRIX_S[1][index][2];
    t[0][n] += r[0]*m[4]; t[1][n] +=r[0]*m[5]+m[4]*r[1]; t[2][n]  += m[5]*r[1];


    t[0][n+4]=0;   t[1][n+4]=0;  t[2][n+4]=0;

    r[0] = ROT_MATRIX_S[0][index][4]; r[1] = ROT_MATRIX_S[1][index][4];
    t[0][n+4] += r[0]*m[0]; t[1][n+4] +=r[0]*m[1]+m[0]*r[1]; t[2][n+4]  += m[1]*r[1];



    r[0] = ROT_MATRIX_S[0][index][5]; r[1] = ROT_MATRIX_S[1][index][5];
    t[0][n+4] += r[0]*m[2]; t[1][n+4] +=r[0]*m[3]+m[2]*r[1]; t[2][n+4]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][6]; r[1] = ROT_MATRIX_S[1][index][6];
    t[0][n+4] += r[0]*m[4]; t[1][n+4] +=r[0]*m[5]+m[4]*r[1]; t[2][n+4]  += m[5]*r[1];


    t[0][n+8]=0;   t[1][n+8]=0;  t[2][n+8]=0;

    r[0] = ROT_MATRIX_S[0][index][8]; r[1] = ROT_MATRIX_S[1][index][8];
    t[0][n+8] += r[0]*m[0]; t[1][n+8] +=r[0]*m[1]+m[0]*r[1]; t[2][n+8]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][9]; r[1] = ROT_MATRIX_S[1][index][9];
    t[0][n+8] += r[0]*m[2]; t[1][n+8] +=r[0]*m[3]+m[2]*r[1]; t[2][n+8]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][10]; r[1] = ROT_MATRIX_S[1][index][10];
    t[0][n+8] += r[0]*m[4]; t[1][n+8] +=r[0]*m[5]+m[4]*r[1]; t[2][n+8]  += m[5]*r[1];

    t[0][n+12] = matrix[n+12]; t[1][n+12] = 0; t[2][n+12] = 0;
  }

  Int res[4];
  for (n=0; n<13; n+=4)
  {
    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    if (t[0][n]>=0)   {t[3][n] = t[0][n]>>13;		    t[4][n]=t[0][n]-(t[3][n]<<13);}
    else              {t[3][n] = (abs(t[0][n]))>>13;	t[4][n]=t[0][n]+(t[3][n]<<13);  t[3][n]=-t[3][n];}

    if (t[1][n]>=0)   {t[5][n] = t[1][n]>>13;		    t[6][n]=t[1][n]-(t[5][n]<<13);}
    else              {t[5][n] = (abs(t[1][n]))>>13;	t[6][n]=t[1][n]+(t[5][n]<<13);  t[5][n]=-t[5][n];}

    if (t[2][n]>=0)   {t[7][n] = t[2][n]>>13;		   }
    else              {t[7][n] = (abs(t[2][n]))>>13;	t[7][n]=-t[7][n];}
    r[0] = ROT_MATRIX_S[0][index][16]; r[1] = ROT_MATRIX_S[1][index][16];

    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    if (t[0][n+1]>=0)   {t[3][n+1] = t[0][n+1]>>13;		    t[4][n+1]=t[0][n+1]-(t[3][n+1]<<13);}
    else              {t[3][n+1] = (abs(t[0][n+1]))>>13;	t[4][n+1]=t[0][n+1]+(t[3][n+1]<<13);  t[3][n+1]=-t[3][n+1];}

    if (t[1][n+1]>=0)   {t[5][n+1] = t[1][n+1]>>13;		    t[6][n+1]=t[1][n+1]-(t[5][n+1]<<13);}
    else              {t[5][n+1] = (abs(t[1][n+1]))>>13;	t[6][n+1]=t[1][n+1]+(t[5][n+1]<<13);  t[5][n+1]=-t[5][n+1];}

    if (t[2][n+1]>=0)   {t[7][n+1] = t[2][n+1]>>13;		   }
    else              {t[7][n+1] = (abs(t[2][n+1]))>>13;	t[7][n+1]=-t[7][n+1];}
    r[0] = ROT_MATRIX_S[0][index][17]; r[1] = ROT_MATRIX_S[1][index][17];


    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];

    if (t[0][n+2]>=0)   {t[3][n+2] = t[0][n+2]>>13;		    t[4][n+2]=t[0][n+2]-(t[3][n+2]<<13);}
    else              {t[3][n+2] = (abs(t[0][n+2]))>>13;	t[4][n+2]=t[0][n+2]+(t[3][n+2]<<13);  t[3][n+2]=-t[3][n+2];}

    if (t[1][n+2]>=0)   {t[5][n+2] = t[1][n+2]>>13;		    t[6][n+2]=t[1][n+2]-(t[5][n+2]<<13);}
    else              {t[5][n+2] = (abs(t[1][n+2]))>>13;	t[6][n+2]=t[1][n+2]+(t[5][n+2]<<13);  t[5][n+2]=-t[5][n+2];}

    if (t[2][n+2]>=0)   {t[7][n+2] = t[2][n+2]>>13;		   }
    else              {t[7][n+2] = (abs(t[2][n+2]))>>13;	t[7][n+2]=-t[7][n+2];}
    r[0] = ROT_MATRIX_S[0][index][18]; r[1] = ROT_MATRIX_S[1][index][18];


    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n] += itemp;


    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][20]; r[1] = ROT_MATRIX_S[1][index][20];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    r[0] = ROT_MATRIX_S[0][index][21]; r[1] = ROT_MATRIX_S[1][index][21];
    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];

    r[0] = ROT_MATRIX_S[0][index][22]; r[1] = ROT_MATRIX_S[1][index][22];
    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];


    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+1] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+1] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+1] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+1] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][24]; r[1] = ROT_MATRIX_S[1][index][24];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    r[0] = ROT_MATRIX_S[0][index][25]; r[1] = ROT_MATRIX_S[1][index][25];
    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];

    r[0] = ROT_MATRIX_S[0][index][26]; r[1] = ROT_MATRIX_S[1][index][26];
    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];


    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+2] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+2] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+2] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+2] += itemp;

    res[0]=t[0][n+3];res[1]=t[1][n+3];res[2]=t[2][n+3];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+3] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+3] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+3] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+3] += itemp;
  }
}
Void
TComTrQuant::RotTransformLI2( Long* matrix, UChar index )
{
  if (index==0) return;
  Int m[6],r[2],t[8][64];
  Int itemp = 0;

  Int n = 0;

  for (n=0; n<8; n++)
  {

    t[0][n]=0;   t[1][n]=0;  t[2][n]=0;
    if (matrix[n]>=0) {m[0] = matrix[n]>>13;		m[1]=matrix[n]-(m[0]<<13);}
    else              {m[0] = (abs(matrix[n]))>>13;	m[1]=matrix[n]+(m[0]<<13);  m[0]=-m[0];}
    r[0] = ROT_MATRIX_S[0][index][32]; r[1] = ROT_MATRIX_S[1][index][32];
    t[0][n] += r[0]*m[0]; t[1][n] +=r[0]*m[1]+m[0]*r[1]; t[2][n]  += m[1]*r[1];

    if (matrix[n+8]>=0) {m[2] = matrix[n+8]>>13;		m[3]=matrix[n+8]-(m[2]<<13);}
    else              {m[2] = (abs(matrix[n+8]))>>13;	m[3]=matrix[n+8]+(m[2]<<13);  m[2]=-m[2];}
    r[0] = ROT_MATRIX_S[0][index][33]; r[1] = ROT_MATRIX_S[1][index][33];
    t[0][n] += r[0]*m[2]; t[1][n] +=r[0]*m[3]+m[2]*r[1]; t[2][n]  += m[3]*r[1];

    if (matrix[n+16]>=0) {m[4] = matrix[n+16]>>13;		m[5]=matrix[n+16]-(m[4]<<13);}
    else              {m[4] = (abs(matrix[n+16]))>>13;	m[5]=matrix[n+16]+(m[4]<<13);  m[4]=-m[4];}
    r[0] = ROT_MATRIX_S[0][index][34]; r[1] = ROT_MATRIX_S[1][index][34];
    t[0][n] += r[0]*m[4]; t[1][n] +=r[0]*m[5]+m[4]*r[1]; t[2][n]  += m[5]*r[1];

    t[0][n+8]=0;   t[1][n+8]=0;  t[2][n+8]=0;

    r[0] = ROT_MATRIX_S[0][index][36]; r[1] = ROT_MATRIX_S[1][index][36];
    t[0][n+8] += r[0]*m[0]; t[1][n+8] +=r[0]*m[1]+m[0]*r[1]; t[2][n+8]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][37]; r[1] = ROT_MATRIX_S[1][index][37];
    t[0][n+8] += r[0]*m[2]; t[1][n+8] +=r[0]*m[3]+m[2]*r[1]; t[2][n+8]  += m[3]*r[1];
    r[0] = ROT_MATRIX_S[0][index][38]; r[1] = ROT_MATRIX_S[1][index][38];
    t[0][n+8] += r[0]*m[4]; t[1][n+8] +=r[0]*m[5]+m[4]*r[1]; t[2][n+8]  += m[5]*r[1];

    t[0][n+16]=0;   t[1][n+16]=0;  t[2][n+16]=0;

    r[0] = ROT_MATRIX_S[0][index][40]; r[1] = ROT_MATRIX_S[1][index][40];
    t[0][n+16] += r[0]*m[0]; t[1][n+16] +=r[0]*m[1]+m[0]*r[1]; t[2][n+16]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][41]; r[1] = ROT_MATRIX_S[1][index][41];
    t[0][n+16] += r[0]*m[2]; t[1][n+16] +=r[0]*m[3]+m[2]*r[1]; t[2][n+16]  += m[3]*r[1];
    r[0] = ROT_MATRIX_S[0][index][42]; r[1] = ROT_MATRIX_S[1][index][42];
    t[0][n+16] += r[0]*m[4]; t[1][n+16] +=r[0]*m[5]+m[4]*r[1]; t[2][n+16]  += m[5]*r[1];


    t[0][n+24]=0;   t[1][n+24]=0;  t[2][n+24]=0;

    if (matrix[n+24]>=0) {m[0] = matrix[n+24]>>13;		m[1]=matrix[n+24]-(m[0]<<13);}
    else              {m[0] = (abs(matrix[n+24]))>>13;	m[1]=matrix[n+24]+(m[0]<<13);  m[0]=-m[0];}

    r[0] = ROT_MATRIX_S[0][index][64]; r[1] = ROT_MATRIX_S[1][index][64];
    t[0][n+24] += r[0]*m[0]; t[1][n+24] +=r[0]*m[1]+m[0]*r[1]; t[2][n+24]  += m[1]*r[1];

    if (matrix[n+32]>=0) {m[2] = matrix[n+32]>>13;		m[3]=matrix[n+32]-(m[2]<<13);}
    else              {m[2] = (abs(matrix[n+32]))>>13;	m[3]=matrix[n+32]+(m[2]<<13);  m[2]=-m[2];}

    r[0] = ROT_MATRIX_S[0][index][65]; r[1] = ROT_MATRIX_S[1][index][65];
    t[0][n+24] += r[0]*m[2]; t[1][n+24] +=r[0]*m[3]+m[2]*r[1]; t[2][n+24]  += m[3]*r[1];

    if (matrix[n+40]>=0) {m[4] = matrix[n+40]>>13;		m[5]=matrix[n+40]-(m[4]<<13);}
    else              {m[4] = (abs(matrix[n+40]))>>13;	m[5]=matrix[n+40]+(m[4]<<13);  m[4]=-m[4];}

    r[0] = ROT_MATRIX_S[0][index][66]; r[1] = ROT_MATRIX_S[1][index][66];
    t[0][n+24] += r[0]*m[4]; t[1][n+24] +=r[0]*m[5]+m[4]*r[1]; t[2][n+24]  += m[5]*r[1];

    t[0][n+32]=0;   t[1][n+32]=0;  t[2][n+32]=0;

    r[0] = ROT_MATRIX_S[0][index][68]; r[1] = ROT_MATRIX_S[1][index][68];
    t[0][n+32] += r[0]*m[0]; t[1][n+32] +=r[0]*m[1]+m[0]*r[1]; t[2][n+32]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][69]; r[1] = ROT_MATRIX_S[1][index][69];
    t[0][n+32] += r[0]*m[2]; t[1][n+32] +=r[0]*m[3]+m[2]*r[1]; t[2][n+32]  += m[3]*r[1];


    r[0] = ROT_MATRIX_S[0][index][70]; r[1] = ROT_MATRIX_S[1][index][70];
    t[0][n+32] += r[0]*m[4]; t[1][n+32] +=r[0]*m[5]+m[4]*r[1]; t[2][n+32]  += m[5]*r[1];


    t[0][n+40]=0;   t[1][n+40]=0;  t[2][n+40]=0;
    r[0] = ROT_MATRIX_S[0][index][72]; r[1] = ROT_MATRIX_S[1][index][72];
    t[0][n+40] += r[0]*m[0]; t[1][n+40] +=r[0]*m[1]+m[0]*r[1]; t[2][n+40]  += m[1]*r[1];



    r[0] = ROT_MATRIX_S[0][index][73]; r[1] = ROT_MATRIX_S[1][index][73];
    t[0][n+40] += r[0]*m[2]; t[1][n+40] +=r[0]*m[3]+m[2]*r[1]; t[2][n+40]  += m[3]*r[1];



    r[0] = ROT_MATRIX_S[0][index][74]; r[1] = ROT_MATRIX_S[1][index][74];
    t[0][n+40] += r[0]*m[4]; t[1][n+40] +=r[0]*m[5]+m[4]*r[1]; t[2][n+40]  += m[5]*r[1];


    t[0][n+48] = matrix[n+48]; t[1][n+48] = 0; t[2][n+48] = 0;
    t[0][n+56] = matrix[n+56]; t[1][n+56] = 0; t[2][n+56] = 0;
  }

  Int res[4];

  for (n=0; n<64; n+=8)
  {
    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    if (t[0][n]>=0)   {t[3][n] = t[0][n]>>13;		    t[4][n]=t[0][n]-(t[3][n]<<13);}
    else              {t[3][n] = (abs(t[0][n]))>>13;	t[4][n]=t[0][n]+(t[3][n]<<13);  t[3][n]=-t[3][n];}

    if (t[1][n]>=0)   {t[5][n] = t[1][n]>>13;		    t[6][n]=t[1][n]-(t[5][n]<<13);}
    else              {t[5][n] = (abs(t[1][n]))>>13;	t[6][n]=t[1][n]+(t[5][n]<<13);  t[5][n]=-t[5][n];}

    if (t[2][n]>=0)   {t[7][n] = t[2][n]>>13;		   }
    else              {t[7][n] = (abs(t[2][n]))>>13;	t[7][n]=-t[7][n];}

    r[0] = ROT_MATRIX_S[0][index][48]; r[1] = ROT_MATRIX_S[1][index][48];

    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];

    if (t[0][n+1]>=0)   {t[3][n+1] = t[0][n+1]>>13;		    t[4][n+1]=t[0][n+1]-(t[3][n+1]<<13);}
    else              {t[3][n+1] = (abs(t[0][n+1]))>>13;	t[4][n+1]=t[0][n+1]+(t[3][n+1]<<13);  t[3][n+1]=-t[3][n+1];}

    if (t[1][n+1]>=0)   {t[5][n+1] = t[1][n+1]>>13;		    t[6][n+1]=t[1][n+1]-(t[5][n+1]<<13);}
    else              {t[5][n+1] = (abs(t[1][n+1]))>>13;	t[6][n+1]=t[1][n+1]+(t[5][n+1]<<13);  t[5][n+1]=-t[5][n+1];}

    if (t[2][n+1]>=0)   {t[7][n+1] = t[2][n+1]>>13;		   }
    else              {t[7][n+1] = (abs(t[2][n+1]))>>13;	t[7][n+1]=-t[7][n+1];}

    r[0] = ROT_MATRIX_S[0][index][49]; r[1] = ROT_MATRIX_S[1][index][49];

    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];

    if (t[0][n+2]>=0)   {t[3][n+2] = t[0][n+2]>>13;		    t[4][n+2]=t[0][n+2]-(t[3][n+2]<<13);}
    else              {t[3][n+2] = (abs(t[0][n+2]))>>13;	t[4][n+2]=t[0][n+2]+(t[3][n+2]<<13);  t[3][n+2]=-t[3][n+2];}

    if (t[1][n+2]>=0)   {t[5][n+2] = t[1][n+2]>>13;		    t[6][n+2]=t[1][n+2]-(t[5][n+2]<<13);}
    else              {t[5][n+2] = (abs(t[1][n+2]))>>13;	t[6][n+2]=t[1][n+2]+(t[5][n+2]<<13);  t[5][n+2]=-t[5][n+2];}

    if (t[2][n+2]>=0)   {t[7][n+2] = t[2][n+2]>>13;		   }
    else              {t[7][n+2] = (abs(t[2][n+2]))>>13;	t[7][n+2]=-t[7][n+2];}

    r[0] = ROT_MATRIX_S[0][index][50]; r[1] = ROT_MATRIX_S[1][index][50];
    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][52]; r[1] = ROT_MATRIX_S[1][index][52];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    r[0] = ROT_MATRIX_S[0][index][53]; r[1] = ROT_MATRIX_S[1][index][53];
    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];


    r[0] = ROT_MATRIX_S[0][index][54]; r[1] = ROT_MATRIX_S[1][index][54];
    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+1] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+1] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+1] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+1] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][56]; r[1] = ROT_MATRIX_S[1][index][56];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    r[0] = ROT_MATRIX_S[0][index][57]; r[1] = ROT_MATRIX_S[1][index][57];
    res[0] += r[0]*t[3][n+1];  res[1] +=r[0]*t[4][n+1]+r[1]*t[3][n+1]+r[0]*t[5][n+1]; res[2] +=r[1]*t[4][n+1]+r[0]*t[6][n+1]+r[1]*t[5][n+1]+r[0]*t[7][n+1];

    r[0] = ROT_MATRIX_S[0][index][58]; r[1] = ROT_MATRIX_S[1][index][58];
    res[0] += r[0]*t[3][n+2];  res[1] +=r[0]*t[4][n+2]+r[1]*t[3][n+2]+r[0]*t[5][n+2]; res[2] +=r[1]*t[4][n+2]+r[0]*t[6][n+2]+r[1]*t[5][n+2]+r[0]*t[7][n+2];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+2] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+2] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+2] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+2] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    if (t[0][n+3]>=0)   {t[3][n+3] = t[0][n+3]>>13;		    t[4][n+3]=t[0][n+3]-(t[3][n+3]<<13);}
    else              {t[3][n+3] = (abs(t[0][n+3]))>>13;	t[4][n+3]=t[0][n+3]+(t[3][n+3]<<13);  t[3][n+3]=-t[3][n+3];}

    if (t[1][n+3]>=0)   {t[5][n+3] = t[1][n+3]>>13;		    t[6][n+3]=t[1][n+3]-(t[5][n+3]<<13);}
    else              {t[5][n+3] = (abs(t[1][n+3]))>>13;	t[6][n+3]=t[1][n+3]+(t[5][n+3]<<13);  t[5][n+3]=-t[5][n+3];}

    if (t[2][n+3]>=0)   {t[7][n+3] = t[2][n+3]>>13;		   }
    else              {t[7][n+3] = (abs(t[2][n+3]))>>13;	t[7][n+3]=-t[7][n+3];}

    r[0] = ROT_MATRIX_S[0][index][80]; r[1] = ROT_MATRIX_S[1][index][80];

    res[0] += r[0]*t[3][n+3];  res[1] +=r[0]*t[4][n+3]+r[1]*t[3][n+3]+r[0]*t[5][n+3]; res[2] +=r[1]*t[4][n+3]+r[0]*t[6][n+3]+r[1]*t[5][n+3]+r[0]*t[7][n+3];


    if (t[0][n+4]>=0)   {t[3][n+4] = t[0][n+4]>>13;		    t[4][n+4]=t[0][n+4]-(t[3][n+4]<<13);}
    else              {t[3][n+4] = (abs(t[0][n+4]))>>13;	t[4][n+4]=t[0][n+4]+(t[3][n+4]<<13);  t[3][n+4]=-t[3][n+4];}

    if (t[1][n+4]>=0)   {t[5][n+4] = t[1][n+4]>>13;		    t[6][n+4]=t[1][n+4]-(t[5][n+4]<<13);}
    else              {t[5][n+4] = (abs(t[1][n+4]))>>13;	t[6][n+4]=t[1][n+4]+(t[5][n+4]<<13);  t[5][n+4]=-t[5][n+4];}

    if (t[2][n+4]>=0)   {t[7][n+4] = t[2][n+4]>>13;		   }
    else              {t[7][n+4] = (abs(t[2][n+4]))>>13;	t[7][n+4]=-t[7][n+4];}

    r[0] = ROT_MATRIX_S[0][index][81]; r[1] = ROT_MATRIX_S[1][index][81];

    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];


    if (t[0][n+5]>=0)   {t[3][n+5] = t[0][n+5]>>13;		    t[4][n+5]=t[0][n+5]-(t[3][n+5]<<13);}
    else              {t[3][n+5] = (abs(t[0][n+5]))>>13;	t[4][n+5]=t[0][n+5]+(t[3][n+5]<<13);  t[3][n+5]=-t[3][n+5];}

    if (t[1][n+5]>=0)   {t[5][n+5] = t[1][n+5]>>13;		    t[6][n+5]=t[1][n+5]-(t[5][n+5]<<13);}
    else              {t[5][n+5] = (abs(t[1][n+5]))>>13;	t[6][n+5]=t[1][n+5]+(t[5][n+5]<<13);  t[5][n+5]=-t[5][n+5];}

    if (t[2][n+5]>=0)   {t[7][n+5] = t[2][n+5]>>13;		   }
    else              {t[7][n+5] = (abs(t[2][n+5]))>>13;	t[7][n+5]=-t[7][n+5];}

    r[0] = ROT_MATRIX_S[0][index][82]; r[1] = ROT_MATRIX_S[1][index][82];
    res[0] += r[0]*t[3][n+5];  res[1] +=r[0]*t[4][n+5]+r[1]*t[3][n+5]+r[0]*t[5][n+5]; res[2] +=r[1]*t[4][n+5]+r[0]*t[6][n+5]+r[1]*t[5][n+5]+r[0]*t[7][n+5];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+3] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+3] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+3] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+3] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][84]; r[1] = ROT_MATRIX_S[1][index][84];
    res[0] += r[0]*t[3][n+3];  res[1] +=r[0]*t[4][n+3]+r[1]*t[3][n+3]+r[0]*t[5][n+3]; res[2] +=r[1]*t[4][n+3]+r[0]*t[6][n+3]+r[1]*t[5][n+3]+r[0]*t[7][n+3];

    r[0] = ROT_MATRIX_S[0][index][85]; r[1] = ROT_MATRIX_S[1][index][85];
    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];

    r[0] = ROT_MATRIX_S[0][index][86]; r[1] = ROT_MATRIX_S[1][index][86];
    res[0] += r[0]*t[3][n+5];  res[1] +=r[0]*t[4][n+5]+r[1]*t[3][n+5]+r[0]*t[5][n+5]; res[2] +=r[1]*t[4][n+5]+r[0]*t[6][n+5]+r[1]*t[5][n+5]+r[0]*t[7][n+5];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+4] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+4] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+4] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+4] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][88]; r[1] = ROT_MATRIX_S[1][index][88];
    res[0] += r[0]*t[3][n+3];  res[1] +=r[0]*t[4][n+3]+r[1]*t[3][n+3]+r[0]*t[5][n+3]; res[2] +=r[1]*t[4][n+3]+r[0]*t[6][n+3]+r[1]*t[5][n+3]+r[0]*t[7][n+3];

    r[0] = ROT_MATRIX_S[0][index][89]; r[1] = ROT_MATRIX_S[1][index][89];
    res[0] += r[0]*t[3][n+4];  res[1] +=r[0]*t[4][n+4]+r[1]*t[3][n+4]+r[0]*t[5][n+4]; res[2] +=r[1]*t[4][n+4]+r[0]*t[6][n+4]+r[1]*t[5][n+4]+r[0]*t[7][n+4];

    r[0] = ROT_MATRIX_S[0][index][90]; r[1] = ROT_MATRIX_S[1][index][90];
    res[0] += r[0]*t[3][n+5];  res[1] +=r[0]*t[4][n+5]+r[1]*t[3][n+5]+r[0]*t[5][n+5]; res[2] +=r[1]*t[4][n+5]+r[0]*t[6][n+5]+r[1]*t[5][n+5]+r[0]*t[7][n+5];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+5] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+5] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+5] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+5] += itemp;

    res[0]=t[0][n+6];res[1]=t[1][n+6];res[2]=t[2][n+6];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+6] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+6] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+6] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+6] += itemp;

    res[0]=t[0][n+7];res[1]=t[1][n+7];res[2]=t[2][n+7];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+7] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+7] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+7] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+7] += itemp;
  }
}
Void
TComTrQuant::InvRotTransformLI2( Long* matrix , UChar index)
{
  if (index==0) return;

  Int m[6],r[2],t[8][64];
  Int itemp = 0;

  Int n = 0;

  for (n=0; n<64; n+=8)
  {
    t[0][n]=0;   t[1][n]=0;  t[2][n]=0;
    if (matrix[n]>=0) {m[0] = matrix[n]>>13;		m[1]=matrix[n]-(m[0]<<13);}
    else              {m[0] = (abs(matrix[n]))>>13;	m[1]=matrix[n]+(m[0]<<13);  m[0]=-m[0];}

    r[0] = ROT_MATRIX_S[0][index][48]; r[1] = ROT_MATRIX_S[1][index][48];
    t[0][n] += r[0]*m[0]; t[1][n] +=r[0]*m[1]+m[0]*r[1]; t[2][n]  += m[1]*r[1];


    if (matrix[n+1]>=0) {m[2] = matrix[n+1]>>13;		m[3]=matrix[n+1]-(m[2]<<13);}
    else              {m[2] = (abs(matrix[n+1]))>>13;	m[3]=matrix[n+1]+(m[2]<<13);  m[2]=-m[2];}

    r[0] = ROT_MATRIX_S[0][index][52]; r[1] = ROT_MATRIX_S[1][index][52];
    t[0][n] += r[0]*m[2]; t[1][n] +=r[0]*m[3]+m[2]*r[1]; t[2][n]  += m[3]*r[1];

    if (matrix[n+2]>=0) {m[4] = matrix[n+2]>>13;		m[5]=matrix[n+2]-(m[4]<<13);}
    else              {m[4] = (abs(matrix[n+2]))>>13;	m[5]=matrix[n+2]+(m[4]<<13);  m[4]=-m[4];}

    r[0] = ROT_MATRIX_S[0][index][56]; r[1] = ROT_MATRIX_S[1][index][56];
    t[0][n] += r[0]*m[4]; t[1][n] +=r[0]*m[5]+m[4]*r[1]; t[2][n]  += m[5]*r[1];


    t[0][n+1]=0;   t[1][n+1]=0;  t[2][n+1]=0;


    r[0] = ROT_MATRIX_S[0][index][49]; r[1] = ROT_MATRIX_S[1][index][49];
    t[0][n+1] += r[0]*m[0]; t[1][n+1] +=r[0]*m[1]+m[0]*r[1]; t[2][n+1]  += m[1]*r[1];



    r[0] = ROT_MATRIX_S[0][index][53]; r[1] = ROT_MATRIX_S[1][index][53];
    t[0][n+1] += r[0]*m[2]; t[1][n+1] +=r[0]*m[3]+m[2]*r[1]; t[2][n+1]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][57]; r[1] = ROT_MATRIX_S[1][index][57];
    t[0][n+1] += r[0]*m[4]; t[1][n+1] +=r[0]*m[5]+m[4]*r[1]; t[2][n+1]  += m[5]*r[1];


    t[0][n+2]=0;   t[1][n+2]=0;  t[2][n+2]=0;

    r[0] = ROT_MATRIX_S[0][index][50]; r[1] = ROT_MATRIX_S[1][index][50];
    t[0][n+2] += r[0]*m[0]; t[1][n+2] +=r[0]*m[1]+m[0]*r[1]; t[2][n+2]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][54]; r[1] = ROT_MATRIX_S[1][index][54];
    t[0][n+2] += r[0]*m[2]; t[1][n+2] +=r[0]*m[3]+m[2]*r[1]; t[2][n+2]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][58]; r[1] = ROT_MATRIX_S[1][index][58];
    t[0][n+2] += r[0]*m[4]; t[1][n+2] +=r[0]*m[5]+m[4]*r[1]; t[2][n+2]  += m[5]*r[1];


    t[0][n+3]=0;   t[1][n+3]=0;  t[2][n+3]=0;

    if (matrix[n+3]>=0) {m[0] = matrix[n+3]>>13;		m[1]=matrix[n+3]-(m[0]<<13);}
    else               {m[0] = (abs(matrix[n+3]))>>13;	m[1]=matrix[n+3]+(m[0]<<13);  m[0]=-m[0];}

    r[0] = ROT_MATRIX_S[0][index][80]; r[1] = ROT_MATRIX_S[1][index][80];
    t[0][n+3] += r[0]*m[0]; t[1][n+3] +=r[0]*m[1]+m[0]*r[1]; t[2][n+3]  += m[1]*r[1];

    if (matrix[n+4]>=0) {m[2] = matrix[n+4]>>13;		m[3]=matrix[n+4]-(m[2]<<13);}
    else              {m[2] = (abs(matrix[n+4]))>>13;	m[3]=matrix[n+4]+(m[2]<<13);  m[2]=-m[2];}

    r[0] = ROT_MATRIX_S[0][index][84]; r[1] = ROT_MATRIX_S[1][index][84];
    t[0][n+3] += r[0]*m[2]; t[1][n+3] +=r[0]*m[3]+m[2]*r[1]; t[2][n+3]  += m[3]*r[1];

    if (matrix[n+5]>=0) {m[4] = matrix[n+5]>>13;		m[5]=matrix[n+5]-(m[4]<<13);}
    else              {m[4] = (abs(matrix[n+5]))>>13;	m[5]=matrix[n+5]+(m[4]<<13);  m[4]=-m[4];}

    r[0] = ROT_MATRIX_S[0][index][88]; r[1] = ROT_MATRIX_S[1][index][88];
    t[0][n+3] += r[0]*m[4]; t[1][n+3] +=r[0]*m[5]+m[4]*r[1]; t[2][n+3]  += m[5]*r[1];


    t[0][n+4]=0;   t[1][n+4]=0;  t[2][n+4]=0;

    r[0] = ROT_MATRIX_S[0][index][81]; r[1] = ROT_MATRIX_S[1][index][81];
    t[0][n+4] += r[0]*m[0]; t[1][n+4] +=r[0]*m[1]+m[0]*r[1]; t[2][n+4]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][85]; r[1] = ROT_MATRIX_S[1][index][85];
    t[0][n+4] += r[0]*m[2]; t[1][n+4] +=r[0]*m[3]+m[2]*r[1]; t[2][n+4]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][89]; r[1] = ROT_MATRIX_S[1][index][89];
    t[0][n+4] += r[0]*m[4]; t[1][n+4] +=r[0]*m[5]+m[4]*r[1]; t[2][n+4]  += m[5]*r[1];


    t[0][n+5]=0;   t[1][n+5]=0;  t[2][n+5]=0;

    r[0] = ROT_MATRIX_S[0][index][82]; r[1] = ROT_MATRIX_S[1][index][82];
    t[0][n+5] += r[0]*m[0]; t[1][n+5] +=r[0]*m[1]+m[0]*r[1]; t[2][n+5]  += m[1]*r[1];

    r[0] = ROT_MATRIX_S[0][index][86]; r[1] = ROT_MATRIX_S[1][index][86];
    t[0][n+5] += r[0]*m[2]; t[1][n+5] +=r[0]*m[3]+m[2]*r[1]; t[2][n+5]  += m[3]*r[1];

    r[0] = ROT_MATRIX_S[0][index][90]; r[1] = ROT_MATRIX_S[1][index][90];
    t[0][n+5] += r[0]*m[4]; t[1][n+5] +=r[0]*m[5]+m[4]*r[1]; t[2][n+5]  += m[5]*r[1];

    t[0][n+6] = matrix[n+6]; t[1][n+6] = 0; t[2][n+6] = 0;
    t[0][n+7] = matrix[n+7]; t[1][n+7] = 0; t[2][n+7] = 0;
  }

  Int res[4];
  for (n=0; n<8; n++)
  {
    res[0]=0;res[1]=0;res[2]=0;

    if (t[0][n]>=0)   {t[3][n] = t[0][n]>>13;		    t[4][n]=t[0][n]-(t[3][n]<<13);}
    else              {t[3][n] = (abs(t[0][n]))>>13;	t[4][n]=t[0][n]+(t[3][n]<<13);  t[3][n]=-t[3][n];}

    if (t[1][n]>=0)   {t[5][n] = t[1][n]>>13;		    t[6][n]=t[1][n]-(t[5][n]<<13);}
    else              {t[5][n] = (abs(t[1][n]))>>13;	t[6][n]=t[1][n]+(t[5][n]<<13);  t[5][n]=-t[5][n];}

    if (t[2][n]>=0)   {t[7][n] = t[2][n]>>13;		   }
    else              {t[7][n] = (abs(t[2][n]))>>13;	t[7][n]=-t[7][n];}

    r[0] = ROT_MATRIX_S[0][index][32]; r[1] = ROT_MATRIX_S[1][index][32];

    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    if (t[0][n+8]>=0)   {t[3][n+8] = t[0][n+8]>>13;		    t[4][n+8]=t[0][n+8]-(t[3][n+8]<<13);}
    else              {t[3][n+8] = (abs(t[0][n+8]))>>13;	t[4][n+8]=t[0][n+8]+(t[3][n+8]<<13);  t[3][n+8]=-t[3][n+8];}

    if (t[1][n+8]>=0)   {t[5][n+8] = t[1][n+8]>>13;		    t[6][n+8]=t[1][n+8]-(t[5][n+8]<<13);}
    else              {t[5][n+8] = (abs(t[1][n+8]))>>13;	t[6][n+8]=t[1][n+8]+(t[5][n+8]<<13);  t[5][n+8]=-t[5][n+8];}

    if (t[2][n+8]>=0)   {t[7][n+8] = t[2][n+8]>>13;		   }
    else              {t[7][n+8] = (abs(t[2][n+8]))>>13;	t[7][n+8]=-t[7][n+8];}

    r[0] = ROT_MATRIX_S[0][index][36]; r[1] = ROT_MATRIX_S[1][index][36];

    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];


    if (t[0][n+16]>=0)   {t[3][n+16] = t[0][n+16]>>13;		    t[4][n+16]=t[0][n+16]-(t[3][n+16]<<13);}
    else              {t[3][n+16] = (abs(t[0][n+16]))>>13;	t[4][n+16]=t[0][n+16]+(t[3][n+16]<<13);  t[3][n+16]=-t[3][n+16];}

    if (t[1][n+16]>=0)   {t[5][n+16] = t[1][n+16]>>13;		    t[6][n+16]=t[1][n+16]-(t[5][n+16]<<13);}
    else              {t[5][n+16] = (abs(t[1][n+16]))>>13;	t[6][n+16]=t[1][n+16]+(t[5][n+16]<<13);  t[5][n+16]=-t[5][n+16];}

    if (t[2][n+16]>=0)   {t[7][n+16] = t[2][n+16]>>13;		   }
    else              {t[7][n+16] = (abs(t[2][n+16]))>>13;	t[7][n+16]=-t[7][n+16];}

    r[0] = ROT_MATRIX_S[0][index][40]; r[1] = ROT_MATRIX_S[1][index][40];

    res[0] += r[0]*t[3][n+16];  res[1] +=r[0]*t[4][n+16]+r[1]*t[3][n+16]+r[0]*t[5][n+16]; res[2] +=r[1]*t[4][n+16]+r[0]*t[6][n+16]+r[1]*t[5][n+16]+r[0]*t[7][n+16];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n] += itemp;

    res[0]=0;res[1]=0;res[2]=0;


    r[0] = ROT_MATRIX_S[0][index][33]; r[1] = ROT_MATRIX_S[1][index][33];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];


    r[0] = ROT_MATRIX_S[0][index][37]; r[1] = ROT_MATRIX_S[1][index][37];
    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];

    r[0] = ROT_MATRIX_S[0][index][41]; r[1] = ROT_MATRIX_S[1][index][41];
    res[0] += r[0]*t[3][n+16];  res[1] +=r[0]*t[4][n+16]+r[1]*t[3][n+16]+r[0]*t[5][n+16]; res[2] +=r[1]*t[4][n+16]+r[0]*t[6][n+16]+r[1]*t[5][n+16]+r[0]*t[7][n+16];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+8] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+8] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+8] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+8] += itemp;

    res[0]=0;res[1]=0;res[2]=0;res[3]=0;

    r[0] = ROT_MATRIX_S[0][index][34]; r[1] = ROT_MATRIX_S[1][index][34];
    res[0] += r[0]*t[3][n];  res[1] +=r[0]*t[4][n]+r[1]*t[3][n]+r[0]*t[5][n]; res[2] +=r[1]*t[4][n]+r[0]*t[6][n]+r[1]*t[5][n]+r[0]*t[7][n];

    r[0] = ROT_MATRIX_S[0][index][38]; r[1] = ROT_MATRIX_S[1][index][38];
    res[0] += r[0]*t[3][n+8];  res[1] +=r[0]*t[4][n+8]+r[1]*t[3][n+8]+r[0]*t[5][n+8]; res[2] +=r[1]*t[4][n+8]+r[0]*t[6][n+8]+r[1]*t[5][n+8]+r[0]*t[7][n+8];

    r[0] = ROT_MATRIX_S[0][index][42]; r[1] = ROT_MATRIX_S[1][index][42];
    res[0] += r[0]*t[3][n+16];  res[1] +=r[0]*t[4][n+16]+r[1]*t[3][n+16]+r[0]*t[5][n+16]; res[2] +=r[1]*t[4][n+16]+r[0]*t[6][n+16]+r[1]*t[5][n+16]+r[0]*t[7][n+16];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+16] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+16] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+16] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+16] += itemp;

    res[0]=0;res[1]=0;res[2]=0;

    if (t[0][n+24]>=0)   {t[3][n+24] = t[0][n+24]>>13;		    t[4][n+24]=t[0][n+24]-(t[3][n+24]<<13);}
    else              {t[3][n+24] = (abs(t[0][n+24]))>>13;	t[4][n+24]=t[0][n+24]+(t[3][n+24]<<13);  t[3][n+24]=-t[3][n+24];}

    if (t[1][n+24]>=0)   {t[5][n+24] = t[1][n+24]>>13;		    t[6][n+24]=t[1][n+24]-(t[5][n+24]<<13);}
    else              {t[5][n+24] = (abs(t[1][n+24]))>>13;	t[6][n+24]=t[1][n+24]+(t[5][n+24]<<13);  t[5][n+24]=-t[5][n+24];}

    if (t[2][n+24]>=0)   {t[7][n+24] = t[2][n+24]>>13;		   }
    else              {t[7][n+24] = (abs(t[2][n+24]))>>13;	t[7][n+24]=-t[7][n+24];}

    r[0] = ROT_MATRIX_S[0][index][64]; r[1] = ROT_MATRIX_S[1][index][64];

    res[0] += r[0]*t[3][n+24];  res[1] +=r[0]*t[4][n+24]+r[1]*t[3][n+24]+r[0]*t[5][n+24]; res[2] +=r[1]*t[4][n+24]+r[0]*t[6][n+24]+r[1]*t[5][n+24]+r[0]*t[7][n+24];


    if (t[0][n+32]>=0)   {t[3][n+32] = t[0][n+32]>>13;		    t[4][n+32]=t[0][n+32]-(t[3][n+32]<<13);}
    else              {t[3][n+32] = (abs(t[0][n+32]))>>13;	t[4][n+32]=t[0][n+32]+(t[3][n+32]<<13);  t[3][n+32]=-t[3][n+32];}

    if (t[1][n+32]>=0)   {t[5][n+32] = t[1][n+32]>>13;		    t[6][n+32]=t[1][n+32]-(t[5][n+32]<<13);}
    else              {t[5][n+32] = (abs(t[1][n+32]))>>13;	t[6][n+32]=t[1][n+32]+(t[5][n+32]<<13);  t[5][n+32]=-t[5][n+32];}

    if (t[2][n+32]>=0)   {t[7][n+32] = t[2][n+32]>>13;		   }
    else              {t[7][n+32] = (abs(t[2][n+32]))>>13;	t[7][n+32]=-t[7][n+32];}

    r[0] = ROT_MATRIX_S[0][index][68]; r[1] = ROT_MATRIX_S[1][index][68];

    res[0] += r[0]*t[3][n+32];  res[1] +=r[0]*t[4][n+32]+r[1]*t[3][n+32]+r[0]*t[5][n+32]; res[2] +=r[1]*t[4][n+32]+r[0]*t[6][n+32]+r[1]*t[5][n+32]+r[0]*t[7][n+32];

    if (t[0][n+40]>=0)   {t[3][n+40] = t[0][n+40]>>13;		    t[4][n+40]=t[0][n+40]-(t[3][n+40]<<13);}
    else              {t[3][n+40] = (abs(t[0][n+40]))>>13;	t[4][n+40]=t[0][n+40]+(t[3][n+40]<<13);  t[3][n+40]=-t[3][n+40];}

    if (t[1][n+40]>=0)   {t[5][n+40] = t[1][n+40]>>13;		    t[6][n+40]=t[1][n+40]-(t[5][n+40]<<13);}
    else              {t[5][n+40] = (abs(t[1][n+40]))>>13;	t[6][n+40]=t[1][n+40]+(t[5][n+40]<<13);  t[5][n+40]=-t[5][n+40];}

    if (t[2][n+40]>=0)   {t[7][n+40] = t[2][n+40]>>13;		   }
    else              {t[7][n+40] = (abs(t[2][n+40]))>>13;	t[7][n+40]=-t[7][n+40];}

    r[0] = ROT_MATRIX_S[0][index][72]; r[1] = ROT_MATRIX_S[1][index][72];

    res[0] += r[0]*t[3][n+40];  res[1] +=r[0]*t[4][n+40]+r[1]*t[3][n+40]+r[0]*t[5][n+40]; res[2] +=r[1]*t[4][n+40]+r[0]*t[6][n+40]+r[1]*t[5][n+40]+r[0]*t[7][n+40];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+24] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+24] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+24] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+24] += itemp;

    res[0]=0;res[1]=0;res[2]=0;

    r[0] = ROT_MATRIX_S[0][index][65]; r[1] = ROT_MATRIX_S[1][index][65];
    res[0] += r[0]*t[3][n+24];  res[1] +=r[0]*t[4][n+24]+r[1]*t[3][n+24]+r[0]*t[5][n+24]; res[2] +=r[1]*t[4][n+24]+r[0]*t[6][n+24]+r[1]*t[5][n+24]+r[0]*t[7][n+24];


    r[0] = ROT_MATRIX_S[0][index][69]; r[1] = ROT_MATRIX_S[1][index][69];
    res[0] += r[0]*t[3][n+32];  res[1] +=r[0]*t[4][n+32]+r[1]*t[3][n+32]+r[0]*t[5][n+32]; res[2] +=r[1]*t[4][n+32]+r[0]*t[6][n+32]+r[1]*t[5][n+32]+r[0]*t[7][n+32];

    r[0] = ROT_MATRIX_S[0][index][73]; r[1] = ROT_MATRIX_S[1][index][73];
    res[0] += r[0]*t[3][n+40];  res[1] +=r[0]*t[4][n+40]+r[1]*t[3][n+40]+r[0]*t[5][n+40]; res[2] +=r[1]*t[4][n+40]+r[0]*t[6][n+40]+r[1]*t[5][n+40]+r[0]*t[7][n+40];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+32] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+32] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+32] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+32] += itemp;

    res[0]=0;res[1]=0;res[2]=0;

    r[0] = ROT_MATRIX_S[0][index][66]; r[1] = ROT_MATRIX_S[1][index][66];
    res[0] += r[0]*t[3][n+24];  res[1] +=r[0]*t[4][n+24]+r[1]*t[3][n+24]+r[0]*t[5][n+24]; res[2] +=r[1]*t[4][n+24]+r[0]*t[6][n+24]+r[1]*t[5][n+24]+r[0]*t[7][n+24];


    r[0] = ROT_MATRIX_S[0][index][70]; r[1] = ROT_MATRIX_S[1][index][70];
    res[0] += r[0]*t[3][n+32];  res[1] +=r[0]*t[4][n+32]+r[1]*t[3][n+32]+r[0]*t[5][n+32]; res[2] +=r[1]*t[4][n+32]+r[0]*t[6][n+32]+r[1]*t[5][n+32]+r[0]*t[7][n+32];

    r[0] = ROT_MATRIX_S[0][index][74]; r[1] = ROT_MATRIX_S[1][index][74];
    res[0] += r[0]*t[3][n+40];  res[1] +=r[0]*t[4][n+40]+r[1]*t[3][n+40]+r[0]*t[5][n+40]; res[2] +=r[1]*t[4][n+40]+r[0]*t[6][n+40]+r[1]*t[5][n+40]+r[0]*t[7][n+40];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+40] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+40] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+40] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+40] += itemp;

    res[0] =t[0][n+48];res[1]=t[1][n+48] ;res[2]=t[2][n+48];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);


    matrix[n+48] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+48] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+48] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+48] += itemp;


    res[0] =t[0][n+56];res[1]=t[1][n+56] ;res[2]=t[2][n+56];

    if (res[1]>=0) {res[3]=res[1]>>13;res[3]=res[1]-(res[3]<<13);}
    else {res[3]=(abs(res[1]))>>13;res[3]=res[1]+(res[3]<<13);}
    res[3]=res[3]<<13;

    if (res[2]>=0) {itemp=res[2]>>26;itemp=res[2]-(itemp<<26);}
    else {itemp=(abs(res[2]))>>26;itemp=res[2]+(itemp<<26);}

    res[3]+=itemp;
    if (res[3]>=0) res[3]+=(1<<25);
    else res[3]-=(1<<25);

    matrix[n+56] = res[0] ;
    if (res[1]>=0) {itemp=res[1]>>13;}
    else {itemp=(abs(res[1]))>>13;itemp=-itemp;}
    matrix[n+56] += itemp;
    if (res[2]>=0) {itemp=res[2]>>26;}
    else {itemp=(abs(res[2]))>>26;itemp=-itemp;}
    matrix[n+56] += itemp;
    if (res[3]>=0) {itemp=res[3]>>26;}
    else {itemp=(abs(res[3]))>>26;itemp=-itemp;}
    matrix[n+56] += itemp;
  }
}


// ------------------------------------------------------------------------------------------------
// Logical transform
// ------------------------------------------------------------------------------------------------

Void TComTrQuant::xT( Pel* piBlkResi, UInt uiStride, Long* psCoeff, Int iSize )
{
	switch( iSize )
	{
		case  2: xT2 ( piBlkResi, uiStride, psCoeff ); break;
		case  4: xT4 ( piBlkResi, uiStride, psCoeff ); break;
		case  8: xT8 ( piBlkResi, uiStride, psCoeff ); break;
		case 16: xT16( piBlkResi, uiStride, psCoeff ); break;
		case 32: xT32( piBlkResi, uiStride, psCoeff ); break;
		case 64: xT64( piBlkResi, uiStride, psCoeff ); break;
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
		case 64: xIT64( plCoef, pResidual, uiStride ); break;
		default: assert(0); break;
	}
}

// temporal buffer for speed
static levelDataStruct slevelData		[ MAX_CU_SIZE*MAX_CU_SIZE ];
static Int             slevelRDOQ[2][ MAX_CU_SIZE*MAX_CU_SIZE ];

// RDOQ
Void TComTrQuant::xRateDistOptQuant( TComDataCU* pcCU, Long* pSrcCoeff, TCoeff*& pDstCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT )
{
  Int			i, j, coeff_ctr;
  Int			kStart = 0, kStop = 0, noCoeff, estBits;
	Int			iShift;
  Int			qp_rem, q_bits;
  Int			iMaxNumCoeff = uiWidth*uiHeight;
	Double	err, normFact, fTemp;
  Int			iQuantCoef;
	Bool		b64Flag		  = false;
	Bool		bExt8x8Flag = false;
	Int			iShiftQBits;

  levelDataStruct* levelData = &slevelData[0];
  Int*		piLevelRDOQTemp = slevelRDOQ[0];
  Int*		piLevelRDOQBest = slevelRDOQ[1];
  Int*    pTemp;

	Int			iBestStop = kStop, iPos;
  Double	dBestCost = MAX_DOUBLE, dCost = MAX_DOUBLE;

  qp_rem    = m_cQP.m_iRem;
  q_bits    = m_cQP.m_iBits;

	// Step #1: compute scale
  if ( uiWidth == 4 && uiHeight == 4 )
  {
    normFact = pow(2., (2*DQ_BITS+19));
		if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
    m_puiQuantMtx = &g_aiQuantCoef  [m_cQP.m_iRem][0];
		iShift = 2;
  }
  else
	if ( uiWidth == 8 && uiHeight == 8 )
  {
    q_bits++;
    normFact = pow(2., (2*Q_BITS_8+9));
		if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
    m_puiQuantMtx = &g_aiQuantCoef64[m_cQP.m_iRem][0];
		iShift = 3;
  }
  else
	if ( uiWidth == 16 && uiHeight == 16 )
  {
    q_bits = ECore16Shift + m_cQP.per();
    normFact = pow(2., 21);
		if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
		fTemp = estErr16x16[qp_rem]/normFact;

    m_puiQuantMtx = ( &g_aiQuantCoef256[m_cQP.m_iRem][0] );
		iShift = 4;
		bExt8x8Flag = true;
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
		bExt8x8Flag = true;
  }
  else
	if ( uiWidth == 64 && uiHeight == 64 )
  {
    q_bits = ECore64Shift + m_cQP.per();
    normFact = pow(2., 21);
		if ( g_uiBitIncrement ) normFact *= 1<<(2*g_uiBitIncrement);
		fTemp = estErr64x64[qp_rem]/normFact;

    iQuantCoef = g_aiQuantCoef4096[m_cQP.m_iRem];
		iShift = 6;
		b64Flag = true;
		bExt8x8Flag = true;
  }
  else
  {
    assert(0);
  }

	iShiftQBits = (1 <<( q_bits - 1));

  const UInt* pucScan = g_auiFrameScanXY[ g_aucConvertToBit[ uiWidth ] ];

  // Step #2: compute level
  for( iPos = 0; iPos < iMaxNumCoeff; iPos++ )
  {
    j = iPos >> iShift;
    i = iPos % uiWidth;

    if ( bExt8x8Flag )
    {
      if ( ( j < 8 ) && ( i < 8 ) && indexROT )
        levelData[iPos].levelDouble = abs( pSrcCoeff[iPos] );
      else
        levelData[iPos].levelDouble = abs( pSrcCoeff[iPos] * (Long)( b64Flag ? iQuantCoef : m_puiQuantMtx[iPos] ) );
    }
    else
		{
      levelData[iPos].levelDouble = abs( pSrcCoeff[iPos] );
		}

    levelData[iPos].levelQ		= ( levelData[iPos].levelDouble >> q_bits );
    levelData[iPos].lowerInt = ( ( levelData[iPos].levelDouble - (levelData[iPos].levelQ << q_bits) ) < iShiftQBits ) ? true : false;
  }

  noCoeff = 0;

  // Step #2.1: compute kStop
  for( coeff_ctr = 0; coeff_ctr < iMaxNumCoeff; coeff_ctr++ )
  {
    iPos = pucScan[coeff_ctr];
    if ( !levelData[iPos].levelQ )
    {
      if ( !levelData[iPos].lowerInt ) kStop = coeff_ctr;
    }
    else
    {
      kStop = coeff_ctr;
      if ( !levelData[iPos].lowerInt ) kStart = coeff_ctr;
    }
  }

  // Step #3: compute errLevel
  for( coeff_ctr = 0; coeff_ctr < kStop + 1; coeff_ctr++ )
  {
    iPos = pucScan[coeff_ctr];

    j = iPos >> iShift;
    i = iPos % uiWidth;

    levelData[iPos].level[0] = 0;

    if      ( uiWidth == 4 ) fTemp = estErr4x4[qp_rem][i][j]/normFact;
    else if ( uiWidth == 8 ) fTemp = estErr8x8[qp_rem][i][j]/normFact;

    if ( !levelData[iPos].levelQ )
    {
      if ( levelData[iPos].lowerInt )
      {
        levelData[iPos].noLevels = 1;

        err = (Double)(levelData[iPos].levelDouble);
        levelData[iPos].errLevel[0] = err*err*fTemp;
      }
      else
      {
        levelData[iPos].level[1] = 1;
        levelData[iPos].noLevels = 2;
        noCoeff++;

        err = (Double)(levelData[iPos].levelDouble);
        levelData[iPos].errLevel[0] = err*err*fTemp;
        err = (Double)((levelData[iPos].level[1]<<q_bits) - levelData[iPos].levelDouble);
        levelData[iPos].errLevel[1] = err*err*fTemp;
      }
    }
    else if ( levelData[iPos].lowerInt )
    {
      levelData[iPos].level[1] = levelData[iPos].levelQ;
      levelData[iPos].noLevels = 2;
      noCoeff++;

      err = (Double)(levelData[iPos].levelDouble);
      levelData[iPos].errLevel[0] = err*err*fTemp;
      err = (Double)((levelData[iPos].level[1]<<q_bits) - levelData[iPos].levelDouble);
      levelData[iPos].errLevel[1] = err*err*fTemp;
    }
    else
    {
      levelData[iPos].level[1] = levelData[iPos].levelQ;
      levelData[iPos].level[2] = levelData[iPos].levelQ + 1;
      levelData[iPos].noLevels = 3;
      noCoeff++;

      err = (Double)(levelData[iPos].levelDouble);
      levelData[iPos].errLevel[0] = err*err*fTemp;
      err = (Double)((levelData[iPos].level[1]<<q_bits) - levelData[iPos].levelDouble);
      levelData[iPos].errLevel[1] = err*err*fTemp;
      err = (Double)((levelData[iPos].level[2]<<q_bits) - levelData[iPos].levelDouble);
      levelData[iPos].errLevel[2] = err*err*fTemp;
    }
  }

  for (coeff_ctr = kStop+1; coeff_ctr < iMaxNumCoeff; coeff_ctr++ )
  {
    iPos = pucScan[coeff_ctr];

    j = iPos >> iShift;
    i = iPos % uiWidth;

    levelData[iPos].level[0] = 0;

    if      ( uiWidth == 4 ) fTemp = estErr4x4[qp_rem][i][j]/normFact;
    else if ( uiWidth == 8 ) fTemp = estErr8x8[qp_rem][i][j]/normFact;

    levelData[iPos].noLevels = 1;

    err = (Double)(levelData[iPos].levelDouble);
    levelData[iPos].errLevel[0] = err*err*fTemp;
  }

  estBits = xEst_write_and_store_CBP_block_bit ( pcCU, eTType);

  Int kStartEst = kStart;

  dCost = xEst_writeRunLevel_SBAC (levelData, piLevelRDOQTemp, ( eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA ), m_dLambda, kStartEst, kStop, noCoeff, estBits, uiWidth, uiHeight, pcCU->getDepth(0) + pcCU->getTransformIdx(0) );

  if( dCost < dBestCost )
  {
    dBestCost = dCost;
    pTemp = piLevelRDOQBest;
    piLevelRDOQBest = piLevelRDOQTemp;
    piLevelRDOQTemp = pTemp;
    iBestStop = kStop;
  }

  // restore coefficients including sign information
  ::memset( pDstCoeff, 0, sizeof(TCoeff)*iMaxNumCoeff );

  for (coeff_ctr = 0; coeff_ctr < iBestStop+1; coeff_ctr++ )
  {
    iPos = pucScan[coeff_ctr];
    if ( piLevelRDOQBest[iPos] )
    {
      uiAbsSum += piLevelRDOQBest[iPos];
      pDstCoeff[iPos] = pSrcCoeff[iPos] < 0 ? -piLevelRDOQBest[iPos] : piLevelRDOQBest[iPos];
    }
  }
}

/*!
 ****************************************************************************
 * \brief
 *    estimate SBAC CBP bits
 ****************************************************************************
 */
Int TComTrQuant::xEst_write_and_store_CBP_block_bit ( TComDataCU* pcCU, TextType eTType )
{
  Int estBits;
  UInt uiAbsPartIdx = 0;

  Int ctx = pcCU->getCtxCbf( uiAbsPartIdx, eTType, pcCU->getTransformIdx(0) );

  //===== encode symbol =====
  estBits = m_pcEstBitsSbac->blockCbpBits[3-ctx][0] - m_pcEstBitsSbac->blockCbpBits[3-ctx][1];

  return estBits;
}

/*!
 ****************************************************************************
 * \brief
 *    Rate distortion optimized trellis quantization
 ****************************************************************************
 */

// temporal buffer for speed
static Int slevelTab[ MAX_CU_SIZE*MAX_CU_SIZE ];

Double TComTrQuant::xEst_writeRunLevel_SBAC(levelDataStruct* levelData, Int* levelTabMin, TextType eTType, Double lambda, Int& kInit, Int kStop, Int noCoeff, Int estCBP, UInt uiWidth, UInt uiHeight, UInt uiDepth )
{
  Int    k, i, iPos;
  Int    estBits;
  Double lagr, lagrMin = 0, lagrTabMin, lagrTab;
  Int    c1 = 1, c2 = 0, c1Tab[3], c2Tab[3];
  Int    iBest;
  Int*   levelTab = &slevelTab[0];
  Int    ctx, greater_one, last, maxK;
  Double lagrAcc, lagrLastMin = 0, lagrLast;
  Int    kBest = 0, kStart, first;

  UInt uiCtxSize  = 64;
  const Int* pos2ctx_map  = pos2ctx_map8x8;
  const Int* pos2ctx_last = pos2ctx_last8x8;
  if ( uiWidth < 8 )
  {
    pos2ctx_map  = pos2ctx_nomap;
    pos2ctx_last = pos2ctx_nomap;
    uiCtxSize    = 16;
  }

  maxK = uiWidth*uiHeight;

	::memset( levelTabMin, 0, sizeof(Int)*maxK );

  UInt uiCtxSigMap, uiCtxLastBit;
  UInt uiConvBit = g_aucConvertToBit[ uiWidth ];
  const UInt*  pucScanX = g_auiFrameScanX[ uiConvBit ];
  const UInt*  pucScanY = g_auiFrameScanY[ uiConvBit ];
  const UInt*  pucScan = g_auiFrameScanXY[ uiConvBit ];

  if ( noCoeff > 0 )
  {
    if ( noCoeff > 1 )
    {
      kStart = kInit; kBest = 0; first = 1;

      lagrAcc = 0;
      for ( k = kStart; k <= kStop; k++ )
      {
        lagrAcc += levelData[pucScan[k]].errLevel[0];
      }

      if ( levelData[pucScan[kStart]].noLevels > 2 )
      {
        uiCtxLastBit = kStart * uiCtxSize / maxK; // map (iWidth*iHeight-2) to 62

        lagrAcc -= levelData[pucScan[kStart]].errLevel[0];
        lagrLastMin = lambda*(m_pcEstBitsSbac->lastBits[pos2ctx_last[uiCtxLastBit]][1] - m_pcEstBitsSbac->lastBits[pos2ctx_last[uiCtxLastBit]][0]) + lagrAcc;

        kBest = kStart;
        kStart = kStart + 1;
        first = 0;
      }

      for ( k = kStart; k <= kStop; k++ )
      {
        iPos = pucScan[k];
        if (uiCtxSize != maxK)
        {
          UInt uiXX, uiYY;
          uiXX = pucScanX[k]/(uiWidth >> 3  );
          uiYY = pucScanY[k]/(uiHeight >> 3 );

          uiCtxSigMap = g_auiAntiScan8[uiYY*8+uiXX];
        }
        else
          uiCtxSigMap = k * uiCtxSize / maxK; // map (iWidth*iHeight-2) to 62


        lagrMin = levelData[iPos].errLevel[0] + lambda*m_pcEstBitsSbac->significantBits[pos2ctx_map[uiCtxSigMap]][0];

        lagrAcc -= levelData[iPos].errLevel[0];
        if ( levelData[iPos].noLevels > 1 )
        {
          estBits = SIGN_BITS + m_pcEstBitsSbac->significantBits[pos2ctx_map[uiCtxSigMap]][1] + m_pcEstBitsSbac->greaterOneBits[0][4][0];

          uiCtxLastBit = kStart * uiCtxSize / maxK; // map (iWidth*iHeight-2) to 62

          lagrLast = levelData[iPos].errLevel[1] + lambda*(estBits + m_pcEstBitsSbac->lastBits[pos2ctx_last[uiCtxLastBit]][1]) + lagrAcc;
          lagr = levelData[iPos].errLevel[1] + lambda*(estBits + m_pcEstBitsSbac->lastBits[pos2ctx_last[uiCtxLastBit]][0]);

          lagrMin = (lagr < lagrMin) ? lagr : lagrMin;

          if ( lagrLast < lagrLastMin || first == 1 )
          {
            kBest = k;
            first = 0;
            lagrLastMin = lagrLast;
          }

        }
        lagrAcc += lagrMin;
      }

      kStart = kBest;
    }
    else
    {
      kStart = kStop;
    }

    lagrTabMin = 0;
    for ( k = 0; k <= kStart; k++ )
    {
      lagrTabMin += levelData[pucScan[k]].errLevel[0];
    }

    // Initial Lagrangian calculation
    lagrTab = 0;

    //////////////////////////

    lagrTabMin += (lambda*estCBP);
    iBest = 0; first = 1;
    for ( k = kStart; k >= 0; k-- )
    {
      iPos = pucScan[k];
      if (uiCtxSize != maxK)
      {
        UInt uiXX, uiYY;
        uiXX = pucScanX[k]/(uiWidth  >> 3);
        uiYY = pucScanY[k]/(uiHeight >> 3);

        uiCtxSigMap = g_auiAntiScan8[uiYY*8+uiXX];
      }
      else
        uiCtxSigMap = k * uiCtxSize / maxK;

      last = (k == kStart);
      if ( !last )
      {
        lagrMin = levelData[iPos].errLevel[0] + lambda*m_pcEstBitsSbac->significantBits[pos2ctx_map[uiCtxSigMap]][0];
        iBest = 0;
        first = 0;
      }

      for ( i = 1; i < levelData[iPos].noLevels; i++)
      {
        estBits = SIGN_BITS + m_pcEstBitsSbac->significantBits[pos2ctx_map[uiCtxSigMap]][1];

        uiCtxLastBit = k * uiCtxSize / maxK;

        estBits += m_pcEstBitsSbac->lastBits[pos2ctx_last[uiCtxLastBit]][last];

        // greater than 1
        greater_one = (levelData[iPos].level[i]>1);

        c1Tab[i] = c1;   c2Tab[i] = c2;

        ctx = Min (c1Tab[i], 4);
        estBits += m_pcEstBitsSbac->greaterOneBits[0][ctx][greater_one];

        // magnitude if greater than 1
        if (greater_one)
        {
          ctx = Min(c2Tab[i], 4);
          if ( (levelData[iPos].level[i]-2) < RDOQ_MAX_PREC_COEFF )
          {
            estBits += m_aiPrecalcUnaryLevelTab[m_pcEstBitsSbac->greaterOneState[ctx]][levelData[iPos].level[i]-2];
          }
          else
          {
            estBits += est_unary_exp_golomb_level_encode((UInt)levelData[iPos].level[i]-2, ctx, eTType, uiDepth);
          }

          c1Tab[i] = 0;
          c2Tab[i]++;
        }
        else if (c1Tab[i])
        {
          c1Tab[i]++;
        }

        lagr = levelData[iPos].errLevel[i] + lambda*estBits;
        if (lagr < lagrMin || first == 1)
        {
          iBest = i;
          lagrMin = lagr;
          first = 0;
        }
      }

      if ( iBest > 0 )
      {
        c1 = c1Tab[iBest]; c2 = c2Tab[iBest];
      }

      levelTab[iPos] = (Int)levelData[iPos].level[iBest];
      lagrTab += lagrMin;
    }
    ///////////////////////////////////

    if ( lagrTab < lagrTabMin )
    {
      for ( k = 0; k <= kStart; k++ )
      {
        iPos = pucScan[k];
        levelTabMin[iPos] = levelTab[iPos];
      }
      }
    }
  else
  {
    kStart = -1;
    lagrTab = lagrTabMin = lambda*estCBP;
  }
  lagrTabMin = Min( lagrTab, lagrTabMin );
  for ( k = kStart+1; k < maxK; k++ )
    lagrTabMin += levelData[pucScan[k]].errLevel[0];
  kInit = kStart;

  return lagrTabMin;

}

Void TComTrQuant::precalculateUnaryExpGolombLevel()
{
  Int state, ctx_state0, ctx_state1, estBits0, estBits1, symbol;

  for (state = 0; state <= 63; state++)
  {
    // symbol 0 is MPS
    ctx_state0 = 64 + state;
    estBits0 = entropyBits[127-ctx_state0];
    ctx_state1 = 63 - state;
    estBits1 = entropyBits[127-ctx_state1];

    for ( symbol = 0; symbol < RDOQ_MAX_PREC_COEFF; symbol++ )
    {
      m_aiPrecalcUnaryLevelTab[ctx_state0][symbol] = est_unary_exp_golomb_level_bits(symbol, estBits0, estBits1);

      // symbol 0 is LPS
      m_aiPrecalcUnaryLevelTab[ctx_state1][symbol] = est_unary_exp_golomb_level_bits(symbol, estBits1, estBits0);
    }
  }
}

Int TComTrQuant::est_unary_exp_golomb_level_bits( UInt symbol, Int bits0, Int bits1)
{
  UInt l, k;
  UInt exp_start = 13; // 15-2 : 0,1 level decision always sent
  Int estBits;

  if ( symbol == 0 )
  {
    return bits0;
  }
  else
  {
    estBits = bits1;
    l = symbol;
    k = 1;
    while (((--l)>0) && (++k <= exp_start))
    {
      estBits += bits1;
    }
    if (symbol < exp_start)
    {
      estBits += bits0;
    }
    else
    {
      estBits += est_exp_golomb_encode_eq_prob(symbol-exp_start);
    }
  }

  return estBits;
}

/*!
 ****************************************************************************
 * \brief
 *    estimate unary exp golomb bit cost
 ****************************************************************************
 */
Int TComTrQuant::est_unary_exp_golomb_level_encode (UInt symbol, Int ctx, TextType eTType, UInt uiDepth)
{
  UInt l, k;
  UInt exp_start = 13; // 15-2 : 0, 1 level decision always sent
  Int estBits;

  if ( symbol == 0 )
  {
    estBits = m_pcEstBitsSbac->greaterOneBits[1][ctx][0];
    return estBits;
  }
  else
  {
    estBits = m_pcEstBitsSbac->greaterOneBits[1][ctx][1];
    l = symbol;
    k = 1;
    while (((--l)>0) && (++k <= exp_start))
    {
      estBits += m_pcEstBitsSbac->greaterOneBits[1][ctx][1];
    }

    if (symbol < exp_start)
    {
      estBits += m_pcEstBitsSbac->greaterOneBits[1][ctx][0];
    }
    else
    {
      estBits += est_exp_golomb_encode_eq_prob(symbol-exp_start);
    }
  }
  return estBits;
}

/*!
 ****************************************************************************
 * \brief
 *    estimate exp golomb bit cost
 ****************************************************************************
 */
Int TComTrQuant::est_exp_golomb_encode_eq_prob (UInt symbol)
{
  Int k = 0, estBits = 0;

  while ( 1 )
  {
    if ( symbol >= (UInt)(1<<k))
    {
      estBits++;
      symbol = symbol - (1<<k);
      k++;
    }
    else
    {
      estBits++;
      while (k--)
      {
        estBits++;
      }
      break;
    }
  }
  return (estBits << 15);
}

