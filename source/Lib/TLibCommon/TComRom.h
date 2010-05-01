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

/** \file			TComRom.h
    \brief		global variables & functions (header)
*/

#ifndef __TCOMROM__
#define __TCOMROM__

#include "CommonDef.h"

// ====================================================================================================================
// Macros
// ====================================================================================================================

#define			MAX_CU_DEPTH						7														// log2(LCUSize)
#define			MAX_CU_SIZE							(1<<(MAX_CU_DEPTH))					// maximum allowable size of CU
#define			MIN_PU_SIZE							4
#define			MAX_NUM_SPU_W						(MAX_CU_SIZE/MIN_PU_SIZE)		// maximum number of SPU in horizontal line

// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

Void				 initROM();
Void				 destroyROM();
Void				 initFrameScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight );

// ====================================================================================================================
// Data structure related table & variable
// ====================================================================================================================

// flexible conversion from relative to absolute index
extern       UInt		g_auiZscanToRaster[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt		g_auiRasterToZscan[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initZscanToRaster ( Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx );
Void         initRasterToZscan ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth					);

// conversion of partition index to picture pel position
extern       UInt		g_auiRasterToPelX[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt		g_auiRasterToPelY[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initRasterToPelXY ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth );

// global variable (LCU width/height, max. CU depth)
extern       UInt g_uiMaxCUWidth;
extern       UInt g_uiMaxCUHeight;
extern       UInt g_uiMaxCUDepth;
extern       UInt g_uiAddCUDepth;

// ====================================================================================================================
// Quantization & DeQuantization
// ====================================================================================================================

extern       UInt		g_aiQuantCoef4			[6];
extern			 Int		g_aiDequantCoef4		[6];
extern       UInt		g_aiQuantCoef				[6][16];
extern			 Int		g_aiDequantCoef			[6][16];
extern			 Int		g_aiDequantCoef64		[6][64];
extern       UInt		g_aiQuantCoef64			[6][64];
extern       UInt		g_aiQuantCoef256		[6][256];
extern       UInt		g_aiDeQuantCoef256	[6][256];
extern       UInt		g_aiQuantCoef1024		[6][1024];
extern       UInt		g_aiDeQuantCoef1024	[6][1024];
extern       UInt		g_aiQuantCoef4096		[6];
extern       UInt		g_aiDeQuantCoef4096	[6];

// ====================================================================================================================
// Luma QP to Chroma QP mapping
// ====================================================================================================================

extern const UChar	g_aucChromaScale      [52];

// ====================================================================================================================
// Scanning order & context mapping table
// ====================================================================================================================

extern			 UInt*	g_auiFrameScanXY[ MAX_CU_DEPTH  ];		// raster index			from scanning index
extern			 UInt*	g_auiFrameScanX [ MAX_CU_DEPTH  ];		// raster index (x) from scanning index
extern			 UInt*	g_auiFrameScanY [ MAX_CU_DEPTH  ];		// raster index (y) from scanning index
extern       UInt		g_auiAntiScan8[64];										// 2D context mapping for coefficients

// ====================================================================================================================
// CAVLC table
// ====================================================================================================================

extern const UInt		g_auiIncVlc[];
extern const UChar	g_aucCodeTableTO16[3][4][17];
extern const UChar	g_aucLenTableTO16 [3][4][17];
extern const UChar	g_aucCodeTable3[7][15];
extern const UChar	g_aucLenTable3 [7][15];
extern const UChar	g_aucCodeTableTZ4[3][4];
extern const UChar	g_aucLenTableTZ4 [3][4];
extern const UChar	g_aucCodeTableTZ16[15][16];
extern const UChar	g_aucLenTableTZ16 [15][16];
extern const UChar	g_aucCodeTableTO4[4][5];
extern const UChar	g_aucLenTableTO4 [4][5];
extern const UChar	g_aucACTab[6];
extern const UChar	g_aucFrameBits[32];

// ====================================================================================================================
// ADI table
// ====================================================================================================================

extern const UChar	g_aucIntraFilter[7][40];
extern const UChar	g_aucIntraModeOrder[7][40];
extern const UChar	g_aucIntraModeConv[7][40];
extern const UChar	g_aucIntraModeConvInv[7][9];
extern const UChar	g_aucIntraModeNum[7];
extern const UChar	g_aucIntraModeBits[7];
extern const UChar	g_aucIntraModeNumFast[7];
extern const UChar	g_aucIntraAvail[40][2];
extern const UChar	g_aucXYflg[40];
extern const Char		g_aucDirDx[32];
extern const Char		g_aucDirDy[32];
extern const UChar	g_aucIntraModeConvC[7][40];
extern const UChar	g_aucIntraModeConvInvC[7][9];
extern const UChar	g_aucIntraModeNumC[7];
extern const UChar	g_aucIntraModeBitsC[7];

// ====================================================================================================================
// ROT table
// ====================================================================================================================

extern const Long		ROT_MATRIX		 [27][96];
extern const Long		ROT_MATRIX_S[2][27][96];

// ====================================================================================================================
// Bit-depth
// ====================================================================================================================

extern       UInt g_uiBitDepth;
extern       UInt g_uiBitIncrement;
extern       UInt g_uiIBDI_MAX;
extern       UInt g_uiBASE_MAX;

// ====================================================================================================================
// Texture type to integer mapping
// ====================================================================================================================

extern const UChar g_aucConvertTxtTypeToIdx[4];

// ====================================================================================================================
// Misc.
// ====================================================================================================================

extern			 Char		g_aucConvertToBit  [ MAX_CU_SIZE+1 ];		// from width to log2(width)-2

#endif  //__TCOMROM__
