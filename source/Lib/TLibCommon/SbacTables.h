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

/** \file			SbacTables.h
    \brief		SBAC common probability definition (header)
*/

#ifndef __SBACTABLES__
#define __SBACTABLES__

#include "CommonDef.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

/// macro to pack four probability parameters into one word
#define _V(a,b,c,d) ( ((UInt)a << 16) | ((UInt)c << 8) | ((UInt)d << 7) | b )

// ====================================================================================================================
// Tables
// ====================================================================================================================

/// quantized probability definition for SBAC
const UInt g_auiStandardProbFsm[113] =
{
/*
 * Index, Qe_Value, Next_Index_LPS, Next_Index_MPS, Switch_MPS
 */
/*   0 */  _V( 0x3F57,   1,   1, 1 ),
/*   1 */  _V( 0x1B1E,  14,   2, 0 ),
/*   2 */  _V( 0x0D09,  16,   3, 0 ),
/*   3 */  _V( 0x06D0,  18,   4, 0 ),
/*   4 */  _V( 0x03EC,  20,   5, 0 ),
/*   5 */  _V( 0x028D,  23,   6, 0 ),
/*   6 */  _V( 0x01E4,  25,   7, 0 ),
/*   7 */  _V( 0x0193,  28,   8, 0 ),
/*   8 */  _V( 0x016B,  30,   9, 0 ),
/*   9 */  _V( 0x0158,  33,  10, 0 ),
/*  10 */  _V( 0x0150,  35,  11, 0 ),
/*  11 */  _V( 0x014A,   9,  12, 0 ),
/*  12 */  _V( 0x0148,  10,  13, 0 ),
/*  13 */  _V( 0x0147,  12,  13, 0 ),
/*  14 */  _V( 0x3F9A,  15,  15, 1 ),
/*  15 */  _V( 0x2CC4,  36,  16, 0 ),
/*  16 */  _V( 0x203A,  38,  17, 0 ),
/*  17 */  _V( 0x17A6,  39,  18, 0 ),
/*  18 */  _V( 0x119D,  40,  19, 0 ),
/*  19 */  _V( 0x0D55,  42,  20, 0 ),
/*  20 */  _V( 0x0A2F,  43,  21, 0 ),
/*  21 */  _V( 0x07E8,  45,  22, 0 ),
/*  22 */  _V( 0x0639,  46,  23, 0 ),
/*  23 */  _V( 0x04F7,  48,  24, 0 ),
/*  24 */  _V( 0x040B,  49,  25, 0 ),
/*  25 */  _V( 0x035A,  51,  26, 0 ),
/*  26 */  _V( 0x02D3,  52,  27, 0 ),
/*  27 */  _V( 0x0270,  54,  28, 0 ),
/*  28 */  _V( 0x0225,  56,  29, 0 ),
/*  29 */  _V( 0x01EF,  57,  30, 0 ),
/*  30 */  _V( 0x01C5,  59,  31, 0 ),
/*  31 */  _V( 0x01A5,  60,  32, 0 ),
/*  32 */  _V( 0x018E,  62,  33, 0 ),
/*  33 */  _V( 0x017C,  63,  34, 0 ),
/*  34 */  _V( 0x016F,  32,  35, 0 ),
/*  35 */  _V( 0x0165,  33,   9, 0 ),
/*  36 */  _V( 0x3FDE,  37,  37, 1 ),
/*  37 */  _V( 0x3311,  64,  38, 0 ),
/*  38 */  _V( 0x2941,  65,  39, 0 ),
/*  39 */  _V( 0x219B,  67,  40, 0 ),
/*  40 */  _V( 0x1B87,  68,  41, 0 ),
/*  41 */  _V( 0x16C3,  69,  42, 0 ),
/*  42 */  _V( 0x12F2,  70,  43, 0 ),
/*  43 */  _V( 0x0FCD,  72,  44, 0 ),
/*  44 */  _V( 0x0D4D,  73,  45, 0 ),
/*  45 */  _V( 0x0B3A,  74,  46, 0 ),
/*  46 */  _V( 0x0987,  75,  47, 0 ),
/*  47 */  _V( 0x0824,  77,  48, 0 ),
/*  48 */  _V( 0x070B,  78,  49, 0 ),
/*  49 */  _V( 0x061D,  79,  50, 0 ),
/*  50 */  _V( 0x0545,  48,  51, 0 ),
/*  51 */  _V( 0x04A1,  50,  52, 0 ),
/*  52 */  _V( 0x0412,  50,  53, 0 ),
/*  53 */  _V( 0x039C,  51,  54, 0 ),
/*  54 */  _V( 0x0339,  52,  55, 0 ),
/*  55 */  _V( 0x02E6,  53,  56, 0 ),
/*  56 */  _V( 0x02A1,  54,  57, 0 ),
/*  57 */  _V( 0x0267,  55,  58, 0 ),
/*  58 */  _V( 0x0239,  56,  59, 0 ),
/*  59 */  _V( 0x0210,  57,  60, 0 ),
/*  60 */  _V( 0x01EF,  58,  61, 0 ),
/*  61 */  _V( 0x01D2,  59,  62, 0 ),
/*  62 */  _V( 0x01BC,  61,  63, 0 ),
/*  63 */  _V( 0x01A9,  61,  32, 0 ),
/*  64 */  _V( 0x3FFF,  65,  65, 1 ),
/*  65 */  _V( 0x3651,  80,  66, 0 ),
/*  66 */  _V( 0x2E29,  81,  67, 0 ),
/*  67 */  _V( 0x27BC,  82,  68, 0 ),
/*  68 */  _V( 0x2244,  83,  69, 0 ),
/*  69 */  _V( 0x1DAC,  84,  70, 0 ),
/*  70 */  _V( 0x19B5,  86,  71, 0 ),
/*  71 */  _V( 0x1689,  87,  72, 0 ),
/*  72 */  _V( 0x13A3,  87,  73, 0 ),
/*  73 */  _V( 0x1153,  72,  74, 0 ),
/*  74 */  _V( 0x0F25,  72,  75, 0 ),
/*  75 */  _V( 0x0D67,  74,  76, 0 ),
/*  76 */  _V( 0x0BE5,  74,  77, 0 ),
/*  77 */  _V( 0x0A72,  75,  78, 0 ),
/*  78 */  _V( 0x0957,  77,  79, 0 ),
/*  79 */  _V( 0x0855,  77,  48, 0 ),
/*  80 */  _V( 0x3E04,  80,  81, 1 ),
/*  81 */  _V( 0x3662,  88,  82, 0 ),
/*  82 */  _V( 0x2FCD,  89,  83, 0 ),
/*  83 */  _V( 0x2A81,  90,  84, 0 ),
/*  84 */  _V( 0x25BA,  91,  85, 0 ),
/*  85 */  _V( 0x216C,  92,  86, 0 ),
/*  86 */  _V( 0x1DED,  93,  87, 0 ),
/*  87 */  _V( 0x1AD1,  86,  71, 0 ),
/*  88 */  _V( 0x3C1E,  88,  89, 1 ),
/*  89 */  _V( 0x3612,  95,  90, 0 ),
/*  90 */  _V( 0x30B1,  96,  91, 0 ),
/*  91 */  _V( 0x2C11,  97,  92, 0 ),
/*  92 */  _V( 0x27F0,  99,  93, 0 ),
/*  93 */  _V( 0x2432,  99,  94, 0 ),
/*  94 */  _V( 0x2104,  93,  86, 0 ),
/*  95 */  _V( 0x3CF5,  95,  96, 1 ),
/*  96 */  _V( 0x37DF, 101,  97, 0 ),
/*  97 */  _V( 0x32CA, 102,  98, 0 ),
/*  98 */  _V( 0x2E99, 103,  99, 0 ),
/*  99 */  _V( 0x2AC3, 104, 100, 0 ),
/* 100 */  _V( 0x2768,  99,  93, 0 ),
/* 101 */  _V( 0x39E1, 105, 102, 0 ),
/* 102 */  _V( 0x35A9, 106, 103, 0 ),
/* 103 */  _V( 0x31A4, 107, 104, 0 ),
/* 104 */  _V( 0x2E4B, 103,  99, 0 ),
/* 105 */  _V( 0x3C9C, 105, 106, 1 ),
/* 106 */  _V( 0x38FE, 108, 107, 0 ),
/* 107 */  _V( 0x3549, 109, 103, 0 ),
/* 108 */  _V( 0x3C39, 110, 109, 0 ),
/* 109 */  _V( 0x3896, 111, 107, 0 ),
/* 110 */  _V( 0x3F4E, 110, 111, 1 ),
/* 111 */  _V( 0x3BE8, 112, 109, 0 ),
/* 112 */  _V( 0x3F34, 112, 111, 1 )
};

/// SBAC shift parameters for LPS
const UInt g_auiShiftParameters[4][2] = {{3,4},{2,3},{1,3},{0,16}};

#endif
