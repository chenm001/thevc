/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
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

/** \file     TComRom.h
    \brief    global variables & functions (header)
*/

#ifndef __TCOMROM__
#define __TCOMROM__

#include "CommonDef.h"

#if HHI_RQT
#include<stdio.h>
#include<iostream>
#endif

// ====================================================================================================================
// Macros
// ====================================================================================================================

#define     MAX_CU_DEPTH            7                           // log2(LCUSize)
#define     MAX_CU_SIZE             (1<<(MAX_CU_DEPTH))         // maximum allowable size of CU
#define     MIN_PU_SIZE             4
#define     MAX_NUM_SPU_W           (MAX_CU_SIZE/MIN_PU_SIZE)   // maximum number of SPU in horizontal line

// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

Void         initROM();
Void         destroyROM();
Void         initFrameScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight );

#if HHI_TRANSFORM_CODING
Void         initSigLastScanPattern( UInt* puiScanPattern, const UInt uiLog2BlockSize, const bool bDownLeft );
#endif

// ====================================================================================================================
// Data structure related table & variable
// ====================================================================================================================

// flexible conversion from relative to absolute index
extern       UInt   g_auiZscanToRaster[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt   g_auiRasterToZscan[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initZscanToRaster ( Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx );
Void         initRasterToZscan ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth         );

// conversion of partition index to picture pel position
extern       UInt   g_auiRasterToPelX[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];
extern       UInt   g_auiRasterToPelY[ MAX_NUM_SPU_W*MAX_NUM_SPU_W ];

Void         initRasterToPelXY ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth );

// global variable (LCU width/height, max. CU depth)
extern       UInt g_uiMaxCUWidth;
extern       UInt g_uiMaxCUHeight;
extern       UInt g_uiMaxCUDepth;
extern       UInt g_uiAddCUDepth;

// ====================================================================================================================
// Quantization & DeQuantization
// ====================================================================================================================

extern       UInt   g_aiQuantCoef4      [6];
extern       Int    g_aiDequantCoef4    [6];
extern       UInt   g_aiQuantCoef       [6][16];
extern       Int    g_aiDequantCoef     [6][16];
extern       Int    g_aiDequantCoef64   [6][64];
extern       UInt   g_aiQuantCoef64     [6][64];
extern       UInt   g_aiQuantCoef256    [6][256];
extern       UInt   g_aiDeQuantCoef256  [6][256];
extern       UInt   g_aiQuantCoef1024   [6][1024];
extern       UInt   g_aiDeQuantCoef1024 [6][1024];
extern       UInt   g_aiQuantCoef4096   [6];
extern       UInt   g_aiDeQuantCoef4096 [6];

// ====================================================================================================================
// Luma QP to Chroma QP mapping
// ====================================================================================================================

extern const UChar  g_aucChromaScale      [52];

// ====================================================================================================================
// Scanning order & context mapping table
// ====================================================================================================================

extern       UInt*  g_auiFrameScanXY[ MAX_CU_DEPTH  ];    // raster index     from scanning index
extern       UInt*  g_auiFrameScanX [ MAX_CU_DEPTH  ];    // raster index (x) from scanning index
extern       UInt*  g_auiFrameScanY [ MAX_CU_DEPTH  ];    // raster index (y) from scanning index
extern       UInt   g_auiAntiScan8[64];                   // 2D context mapping for coefficients

#if QC_MDDT//ADAPTIVE_SCAN
extern  UInt *scanOrder4x4[9];
extern  UInt *scanOrder4x4X[9];
extern  UInt *scanOrder4x4Y[9];
extern  UInt *scanOrder8x8[9];
extern  UInt *scanOrder8x8X[9];
extern  UInt *scanOrder8x8Y[9];
extern  UInt *scanStats4x4[9];
extern  UInt *scanStats8x8[9];

extern  UInt *scanOrder16x16[NUM_SCANS_16x16];
extern  UInt *scanOrder16x16X[NUM_SCANS_16x16];
extern  UInt *scanOrder16x16Y[NUM_SCANS_16x16];
extern  UInt *scanStats16x16[NUM_SCANS_16x16];

extern  UInt *scanOrder32x32[NUM_SCANS_32x32];
extern  UInt *scanOrder32x32X[NUM_SCANS_32x32];
extern  UInt *scanOrder32x32Y[NUM_SCANS_32x32];
extern  UInt *scanStats32x32[NUM_SCANS_32x32];

extern  UInt *scanOrder64x64[NUM_SCANS_64x64];
extern  UInt *scanOrder64x64X[NUM_SCANS_64x64];
extern  UInt *scanOrder64x64Y[NUM_SCANS_64x64];
extern  UInt *scanStats64x64[NUM_SCANS_64x64];

extern  int  update4x4Count[9];
extern  int  update8x8Count[9];

extern Int g_aiDequantCoef_klt[6][16];
extern UInt g_aiQuantCoef_klt[6][16] ;
extern Int g_aiDequantCoef64_klt[6][64];
extern UInt g_aiQuantCoef64_klt[6][64];
extern const char LUT16x16[5][33];
extern const char LUT32x32[5][33];
extern const char LUT64x64[5][5];

extern Bool g_bUpdateStats;


extern const Short kltRow4x4[9][4][4];
extern const Short kltCol4x4[9][4][4];
extern const Short kltRow8x8[9][8][8];
extern const Short kltCol8x8[9][8][8];


#endif
#if HHI_TRANSFORM_CODING
extern       UInt*  g_auiSigLastScan[ MAX_CU_DEPTH  ][ 2 ];
#endif

// ====================================================================================================================
// CAVLC table
// ====================================================================================================================

extern const UChar  g_aucCodeTable3[7][15];
extern const UChar  g_aucLenTable3 [7][15];
extern const UChar  g_aucCodeTableTZ4[3][4];
extern const UChar  g_aucLenTableTZ4 [3][4];
extern const UChar  g_aucCodeTableTZ16[15][16];
extern const UChar  g_aucLenTableTZ16 [15][16];
extern const UChar  g_aucCodeTableTO4[4][5];
extern const UChar  g_aucLenTableTO4 [4][5];
extern const UChar  g_aucACTab[6];
extern const UChar  g_aucFrameBits[32];

extern const UInt    g_auiLPTableE8[10][128];
extern const UInt    g_auiLPTableD8[10][128];
extern const UInt    g_auiLPTableE4[3][32];
extern const UInt    g_auiLPTableD4[3][32];
extern const UInt    g_auiLastPosVlcIndex[10];
extern const UInt    g_auiLastPosVlcNum[10][17];
extern const UInt    g_auiLumaRun8x8[29][2][64];
extern const UInt    g_auiVlcTable8x8[28];
extern const LastCoeffStruct g_acstructLumaRun8x8[29][127];

// ====================================================================================================================
// ADI table
// ====================================================================================================================

extern const UChar  g_aucIntraFilter[7][40];
extern const UChar  g_aucIntraModeOrder[7][40];
extern const UChar  g_aucIntraModeConv[7][40];
extern const UChar  g_aucIntraModeConvInv[7][9];
extern const UChar  g_aucIntraModeNum[7];
extern const UChar  g_aucIntraModeBits[7];
extern const UChar  g_aucIntraModeNumFast[7];
extern const UChar  g_aucIntraAvail[40][2];
extern const UChar  g_aucXYflg[40];
extern const Char   g_aucDirDx[32];
extern const Char   g_aucDirDy[32];
extern const UChar  g_aucIntraModeConvC[7][40];
extern const UChar  g_aucIntraModeConvInvC[7][9];
extern const UChar  g_aucIntraModeNumC[7];
extern const UChar  g_aucIntraModeBitsC[7];

#if ANG_INTRA
// ====================================================================================================================
// Angular Intra table
// ====================================================================================================================

extern const UChar g_aucAngIntraModeOrder[34];
#endif

#if QC_MDDT
extern const UChar g_aucAngIntra9Mode[34];
#if ANG_INTRA==2
extern const UChar g_aucIntra9Mode[34];
#else
extern const UChar g_aucIntra9Mode[33];
#endif
#endif

// ====================================================================================================================
// ROT table
// ====================================================================================================================

extern const Int    g_FWD_ROT_MATRIX_4[4][18];
extern const Int    g_FWD_ROT_MATRIX_8[4][36];
extern const Int    g_INV_ROT_MATRIX_4[4][18];
extern const Int    g_INV_ROT_MATRIX_8[4][36];
extern const Int    g_auiROTFwdShift[5];

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

extern       Char   g_aucConvertToBit  [ MAX_CU_SIZE+1 ];   // from width to log2(width)-2

#if HHI_RQT

__inline UInt gCeilLog2( const UInt uiVal )
{
  if( uiVal <= MAX_CU_SIZE )
  {
    return g_aucConvertToBit[uiVal]+2;
  }
  UInt uiTmp = uiVal-1;
  UInt uiRet = 0;

  while( uiTmp != 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}


#define ENC_DEC_TRACE 0


#if ENC_DEC_TRACE
extern FILE*  g_hTrace;
extern Bool   g_bJustDoIt;
extern const Bool g_bEncDecTraceEnable;
extern const Bool g_bEncDecTraceDisable;
extern UInt64 g_nSymbolCounter;

#define COUNTER_START    1
#define COUNTER_END      0 //( UInt64(1) << 63 )

#define DTRACE_CABAC_F(x)     if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, "%f", x );
#define DTRACE_CABAC_V(x)     if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, "%d", x );
#define DTRACE_CABAC_T(x)     if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, "%s", x );
#define DTRACE_CABAC_X(x)     if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, "%x", x );
#define DTRACE_CABAC_R( x,y ) if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, x,    y );
#define DTRACE_CABAC_N        if ( ( g_nSymbolCounter >= COUNTER_START && g_nSymbolCounter <= COUNTER_END )|| g_bJustDoIt ) fprintf( g_hTrace, "\n"    );

#else

#define DTRACE_CABAC_F(x)
#define DTRACE_CABAC_V(x)
#define DTRACE_CABAC_T(x)
#define DTRACE_CABAC_X(x)
#define DTRACE_CABAC_R( x,y )
#define DTRACE_CABAC_N

#endif

#endif

#endif  //__TCOMROM__

