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

/** \file     TComRom.h
    \brief    global variables & functions (header)
*/

#ifndef __TCOMROM__
#define __TCOMROM__

#include "CommonDef.h"

#include<stdio.h>
#include<iostream>

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
#if QC_MDCS
Void         initSigLastScan(UInt* pBuffZ, UInt* pBuffH, UInt* pBuffV, Int iWidth, Int iHeight, Int iDepth);
#endif //QC_MDCS

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

extern       UInt g_auiPUOffset[4];

#if E243_CORE_TRANSFORMS
#define QUANT_IQUANT_SHIFT    20 // Q(QP%6) * IQ(QP%6) = 2^20
#define QUANT_SHIFT           14 // Q(4) = 2^14
#define SCALE_BITS            15 // Inherited from TMuC, pressumably for fractional bit estimates in RDOQ
#define MAX_TR_DYNAMIC_RANGE  15 // Maximum transform dynamic range (excluding sign bit)

#define SHIFT_INV_1ST          7 // Shift after first inverse transform stage
#define SHIFT_INV_2ND         12 // Shift after second inverse transform stage

extern UInt g_auiQ[6];             // Q(QP%6)  
extern UInt g_auiIQ[6];            // IQ(QP%6)
extern const short g_aiT4[4][4];
extern const short g_aiT8[8][8];
extern const short g_aiT16[16][16];
extern const short g_aiT32[32][32];



#endif

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
#if QC_MDCS
extern       UInt*  g_auiSigLastScan[3][ MAX_CU_DEPTH ];  // raster index from scanning index (zigzag, hor, ver)
#endif //QC_MDCS
#if PCP_SIGMAP_SIMPLE_LAST
extern       UInt   g_uiCtxXYOffset[ MAX_CU_DEPTH ];      //!< context offset for last pos coding
extern       UInt   g_uiCtxXY      [ 31 ];                //!< context mapping for last pos coding
#endif

#if E253
extern const UInt   g_auiGoRiceRange[4];                  //!< maximum value coded with Rice codes
extern const UInt   g_auiGoRicePrefixLen[4];              //!< prefix length for each maximum value
extern const UInt   g_aauiGoRiceUpdate[4][16];            //!< parameter update rules for Rice codes
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

#if !CAVLC_COEF_LRG_BLK
extern const UInt    g_auiLPTableE8[8][128];
extern const UInt    g_auiLPTableD8[8][128];
#endif
extern const UInt    g_auiLPTableE4[3][32];
extern const UInt    g_auiLPTableD4[3][32];
extern const UInt    g_auiLastPosVlcIndex[10];
extern const UInt    g_auiLastPosVlcNum[10][17];
#if RUNLEVEL_TABLE_CUT
extern const UInt    g_auiLumaRun8x8[28][29];
#else
extern const UInt    g_auiLumaRun8x8[29][2][64];
#endif

#if LCEC_INTRA_MODE
extern const UInt    g_auiIntraModeTableD17[16];
extern const UInt    g_auiIntraModeTableE17[16];
extern const UInt    g_auiIntraModeTableD34[33];
extern const UInt    g_auiIntraModeTableE34[33];
#endif

#if QC_MOD_LCEC
extern const UInt    g_auiVlcTable8x8Inter[29];
extern const UInt    g_auiVlcTable8x8Intra[29];
#else
extern const UInt    g_auiVlcTable8x8[28];
#endif
#if RUNLEVEL_TABLE_CUT 
extern const UInt    g_acstructLumaRun8x8[28][29];
#else
extern const LastCoeffStruct g_acstructLumaRun8x8[29][127];
#endif

#if CAVLC_COEF_LRG_BLK
extern const UInt   g_auiVlcTable16x16Intra[29];
extern const UInt   g_auiVlcTable16x16Inter[29];
#endif

#if LCEC_INTRA_MODE
extern const UInt huff17_2[2][17];
extern const UInt lengthHuff17_2[2][17];
extern const UInt huff34_2[2][34];
extern const UInt lengthHuff34_2[2][34];
#endif

#if QC_MOD_LCEC
#if CAVLC_COEF_LRG_BLK
extern const UInt   *g_pLumaRunTr14x4[5]; 
extern const UInt   *g_pLumaRunTr18x8[5]; 
#else
extern const UInt    g_auiLumaRunTr14x4[5][15];
extern const UInt    g_auiLumaRunTr18x8[5][29];
#endif
#endif

extern const UInt    g_auiCBPTableE[2][8];
extern const UInt    g_auiCBPTableD[2][8];
extern const UInt    g_auiCbpVlcNum[2][8];

extern const UInt    g_auiBlkCBPTableE[2][15];
extern const UInt    g_auiBlkCBPTableD[2][15];
extern const UInt    g_auiBlkCbpVlcNum[15];

extern const UInt g_auiMI1TableE[8];
extern const UInt g_auiMI1TableD[8];
extern const UInt g_auiMI2TableE[15];
extern const UInt g_auiMI2TableD[15];
extern const UInt g_auiMITableVlcNum[15];

extern const UInt g_auiMI1TableENoL1[8];
extern const UInt g_auiMI1TableDNoL1[8];
extern const UInt g_auiMI2TableENoL1[15];
extern const UInt g_auiMI2TableDNoL1[15];

#if MS_LCEC_ONE_FRAME
extern const UInt g_auiMI1TableEOnly1Ref[8];
extern const UInt g_auiMI1TableDOnly1Ref[8];
extern const UInt g_auiMI1TableEOnly1RefNoL1[8];
extern const UInt g_auiMI1TableDOnly1RefNoL1[8];
#endif
#if QC_LCEC_INTER_MODE
extern const UInt g_auiInterModeTableE[4][7];
extern const UInt g_auiInterModeTableD[4][7];
#endif
// ====================================================================================================================
// ADI table
// ====================================================================================================================

extern const UChar  g_aucIntraModeNumFast[7];

// ====================================================================================================================
// Angular Intra table
// ====================================================================================================================

extern const UChar g_aucIntraModeNumAng[7];
extern const UChar g_aucIntraModeBitsAng[7];
extern const UChar g_aucAngModeMapping[4][34];
#if ADD_PLANAR_MODE
extern const UChar g_aucAngIntraModeOrder[NUM_INTRA_MODE];
#else
extern const UChar g_aucAngIntraModeOrder[34];
#endif

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
#if QC_MOD_LCEC
__inline UInt xRunLevelInd(Int lev, Int run, Int maxrun, UInt lrg1Pos)
{
  UInt cn;

  if ( lrg1Pos > 0 )
  {
    if ( lev == 0 )
    {
      if ( run < lrg1Pos )
        cn = run;
      else
        cn = (run << 1) - lrg1Pos + 1;
    }
    else
    {
      if ( run > (maxrun - (Int)lrg1Pos + 1) )
        cn = maxrun + run + 2 ;  
      else
        cn = lrg1Pos + (run << 1);
    }
  }
  else
  {
    cn = (run << 1);
    if ( lev == 0 && run <= maxrun )
    { 
      cn++;
    }
  }
  return(cn);
}

#if RUNLEVEL_TABLE_CUT
/** Function for deriving codeword index in CAVLC run-level coding 
 * \param lev a value indicating coefficient level greater than one or not
 * \param run length of run
 * \param maxrun maximum length of run for a given coefficient location
 * \returns the codeword index
 * This function derives codeword index in CAVLC run-level coding .
 */
__inline UInt xRunLevelIndInter(Int lev, Int run, Int maxrun)
{
  UInt cn;
  
  if (maxrun < 28)
  {
    if (lev == 0)
    {
      cn = g_auiLumaRun8x8[maxrun][run];
    }
    else
    {
      cn = maxrun + g_auiLumaRun8x8[maxrun][run] + 1;  
    }
  }
  else
  {
    if (lev == 0)
    {
      cn = run;
    }
    else
    {
      cn = maxrun + run + 2;
    }
  }

  return(cn);
}
#endif
#endif


#if QC_MOD_LCEC_RDOQ
__inline UInt xLeadingZeros(UInt uiCode)
{
  UInt uiCount = 0;
  Int iDone = 0;
  
  if (uiCode)
  {
    while (!iDone)
    {
      uiCode >>= 1;
      if (!uiCode) iDone = 1;
      else uiCount++;
    }
  }
  return uiCount;
}
#endif

extern       Char   g_aucConvertToBit  [ MAX_CU_SIZE+1 ];   // from width to log2(width)-2

#if CAVLC_COEF_LRG_BLK
/** Function for deriving codeword index in coding last significant position and level information.
 * \param lev a value indicating coefficient level greater than one or not
 * \param last_pos last significant coefficient position
 * \param N block size
 * \returns the codeword index
 * This function derives codeword index in coding last significant position and level information in CAVLC. 
 */
__inline UInt xLastLevelInd(Int lev, Int last_pos, Int N)
{
  UInt cx;
  UInt uiConvBit = g_aucConvertToBit[N]+2;

  if (lev==0)
  {
    cx = ((last_pos + (last_pos>>uiConvBit))>>uiConvBit)+last_pos;
  }
  else
  {
    if (last_pos<N)
    {
      cx = (last_pos+1)<<uiConvBit;
    }
    else
    {
      cx = (0x01<<(uiConvBit<<1)) + last_pos;
    }
  }
  return(cx);
}

__inline void xLastLevelIndInv(Int& lev, Int& last_pos, Int N, UInt cx)
{
  UInt uiConvBit = g_aucConvertToBit[N]+2;
  Int N2 = 0x01<<(uiConvBit<<1);

  if(cx <= N2+N)
  {
    if(cx && (cx&(N-1))==0)
    {
      lev = 1;
      last_pos = (cx>>uiConvBit)-1;
    }
    else
    {
      lev = 0;
      last_pos = cx - (cx>>uiConvBit);
    }
  }
  else
  {
    lev = 1;
    last_pos = cx - N2;
  }
}
#endif


#if CHROMA_CODEWORD_SWITCH 
extern const UChar ChromaMapping[2][5];
#endif

#if ADD_PLANAR_MODE
__inline Void mapPlanartoDC( UChar& curDir ) { curDir = (curDir == PLANAR_IDX) ? 2 : curDir; }
__inline Void mapPlanartoDC(  UInt& curDir ) { curDir = (curDir == PLANAR_IDX) ? 2 : curDir; }
__inline Void mapPlanartoDC(   Int& curDir ) { curDir = (curDir == PLANAR_IDX) ? 2 : curDir; }
#endif

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
#if CAVLC_COUNTER_ADAPT
/** Function for codeword adaptation
 * \param uiCodeIdx codeword index of the syntax element being coded
 * \param pucTableCounter pointer to counter array
 * \param rucTableCounterSum sum counter
 * \param puiTableD pointer to table mapping codeword index to syntax element value
 * \param puiTableE pointer to table mapping syntax element value to codeword index
 * \param uiCounterNum number of counters
 * \returns
 * This function performs codeword adaptation.
*/
__inline Void adaptCodeword( UInt uiCodeIdx, UChar * pucTableCounter, UChar & rucTableCounterSum, UInt * puiTableD, UInt * puiTableE, UInt uiCounterNum )
{
  Bool bSwapping  = false;
  UInt uiCodeword = puiTableD [uiCodeIdx];

  UInt uiPrevCodeIdx   = (uiCodeIdx >= 1)? uiCodeIdx - 1 : 0;
  UInt uiPrevCodeword  = puiTableD[uiPrevCodeIdx];

  if ( uiCodeIdx < uiCounterNum ) 
  {
    pucTableCounter [uiCodeIdx] ++;

    if (pucTableCounter[uiCodeIdx] >= pucTableCounter[uiPrevCodeIdx])
    {
      bSwapping = true;
      UChar ucTempCounter             = pucTableCounter[uiCodeIdx];
      pucTableCounter[uiCodeIdx]      = pucTableCounter[uiPrevCodeIdx];
      pucTableCounter[uiPrevCodeIdx]  = ucTempCounter;
    }

    if ( rucTableCounterSum >= 15 )
    {
      rucTableCounterSum = 0;
      for (UInt uiIdx = 0; uiIdx < uiCounterNum; uiIdx++)
      {
        pucTableCounter[uiIdx] >>= 1;
      }
    }
    else
    {
      rucTableCounterSum ++;
    }
  }
  else
  {
    bSwapping = true;
  }

  if ( bSwapping )
  {
    puiTableD[uiPrevCodeIdx] = uiCodeword;
    puiTableD[uiCodeIdx    ] = uiPrevCodeword;

    if  (puiTableE != NULL)
    {
      puiTableE[uiCodeword]     = uiPrevCodeIdx;
      puiTableE[uiPrevCodeword] = uiCodeIdx;
    }
  }
}

#endif//
#endif  //__TCOMROM__

