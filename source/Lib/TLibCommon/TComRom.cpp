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

/** \file     TComRom.cpp
    \brief    global variables & functions
*/

#include "TComRom.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
// ====================================================================================================================
// Initialize / destroy functions
// ====================================================================================================================

//! \ingroup TLibCommon
//! \{

// initialize ROM variables
Void initROM()
{
  Int i, c;
  
  // g_aucConvertToBit[ x ]: log2(x/4), if x=4 -> 0, x=8 -> 1, x=16 -> 2, ...
  ::memset( g_aucConvertToBit,   -1, sizeof( g_aucConvertToBit ) );
  c=0;
  for ( i=4; i<MAX_CU_SIZE; i*=2 )
  {
    g_aucConvertToBit[ i ] = c;
    c++;
  }
  g_aucConvertToBit[ i ] = c;
  
  // g_auiFrameScanXY[ g_aucConvertToBit[ transformSize ] ]: zigzag scan array for transformSize
  c=2;
  for ( i=0; i<MAX_CU_DEPTH; i++ )
  {
    g_auiFrameScanXY[ i ] = new UInt[ c*c ];
    g_auiFrameScanX [ i ] = new UInt[ c*c ];
    g_auiFrameScanY [ i ] = new UInt[ c*c ];
    initFrameScanXY( g_auiFrameScanXY[i], g_auiFrameScanX[i], g_auiFrameScanY[i], c, c );
    g_auiSigLastScan[0][i] = new UInt[ c*c ];
    g_auiSigLastScan[1][i] = new UInt[ c*c ];
    g_auiSigLastScan[2][i] = new UInt[ c*c ];
#if DIAG_SCAN
    g_auiSigLastScan[3][i] = new UInt[ c*c ];
    initSigLastScan( g_auiSigLastScan[0][i], g_auiSigLastScan[1][i], g_auiSigLastScan[2][i], g_auiSigLastScan[3][i], c, c, i);
#else
    initSigLastScan( g_auiSigLastScan[0][i], g_auiSigLastScan[1][i], g_auiSigLastScan[2][i], c, c, i);
#endif

    c <<= 1;
  }  

#if NSQT
  UInt uiWidth[ 2 ]  = { 16, 32 };
  UInt uiHeight[ 2 ] = { 4,  8  };
  for ( i = 0; i < 2; i++ )
  {
    UInt uiW = uiWidth[ i ];
    UInt uiH = uiHeight[ i ];
    g_auiNonSquareSigLastScan[ i ] = new UInt[ uiW * uiH ];
    initNonSquareSigLastScan( g_auiNonSquareSigLastScan[ i ], uiW, uiH);
  }
#endif
}

Void destroyROM()
{
  Int i;
  
  for ( i=0; i<MAX_CU_DEPTH; i++ )
  {
    delete[] g_auiFrameScanXY[i];
    delete[] g_auiFrameScanX [i];
    delete[] g_auiFrameScanY [i];
    delete[] g_auiSigLastScan[0][i];
    delete[] g_auiSigLastScan[1][i];
    delete[] g_auiSigLastScan[2][i];
#if DIAG_SCAN
    delete[] g_auiSigLastScan[3][i];
#endif
  }
}

// ====================================================================================================================
// Data structure related table & variable
// ====================================================================================================================

UInt g_uiMaxCUWidth  = MAX_CU_SIZE;
UInt g_uiMaxCUHeight = MAX_CU_SIZE;
UInt g_uiMaxCUDepth  = MAX_CU_DEPTH;
UInt g_uiAddCUDepth  = 0;

UInt g_auiZscanToRaster [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToZscan [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToPelX  [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_auiRasterToPelY  [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, };
UInt g_motionRefer   [ MAX_NUM_SPU_W*MAX_NUM_SPU_W ] = { 0, }; 

#if AMP
UInt g_auiPUOffset[8] = { 0, 8, 4, 4, 2, 10, 1, 5};
#else
UInt g_auiPUOffset[4] = { 0, 8, 4, 4 };
#endif

Void initZscanToRaster ( Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx )
{
  Int iStride = 1 << ( iMaxDepth - 1 );
  
  if ( iDepth == iMaxDepth )
  {
    rpuiCurrIdx[0] = uiStartVal;
    rpuiCurrIdx++;
  }
  else
  {
    Int iStep = iStride >> iDepth;
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal,                     rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep,               rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep*iStride,       rpuiCurrIdx );
    initZscanToRaster( iMaxDepth, iDepth+1, uiStartVal+iStep*iStride+iStep, rpuiCurrIdx );
  }
}

Void initRasterToZscan ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth )
{
  UInt  uiMinCUWidth  = uiMaxCUWidth  >> ( uiMaxDepth - 1 );
  UInt  uiMinCUHeight = uiMaxCUHeight >> ( uiMaxDepth - 1 );
  
  UInt  uiNumPartInWidth  = (UInt)uiMaxCUWidth  / uiMinCUWidth;
  UInt  uiNumPartInHeight = (UInt)uiMaxCUHeight / uiMinCUHeight;
  
  for ( UInt i = 0; i < uiNumPartInWidth*uiNumPartInHeight; i++ )
  {
    g_auiRasterToZscan[ g_auiZscanToRaster[i] ] = i;
  }
}

/** generate motion data compression mapping table
* \param uiMaxCUWidth, width of LCU
* \param uiMaxCUHeight, hight of LCU
* \param uiMaxDepth, max depth of LCU
* \returns Void
*/
Void initMotionReferIdx ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth )
{
  Int  minCUWidth  = (Int)uiMaxCUWidth  >> ( (Int)uiMaxDepth - 1 );
  Int  minCUHeight = (Int)uiMaxCUHeight >> ( (Int)uiMaxDepth - 1 );

  Int  numPartInWidth  = (Int)uiMaxCUWidth  / (Int)minCUWidth;
  Int  numPartInHeight = (Int)uiMaxCUHeight / (Int)minCUHeight;

  for ( Int i = 0; i < numPartInWidth*numPartInHeight; i++ )
  {
    g_motionRefer[i] = i;
  }

  Int compressionNum = 2;

  for ( Int i = numPartInWidth*(numPartInHeight-1); i < numPartInWidth*numPartInHeight; i += compressionNum*2)
  {
    for ( Int j = 1; j < compressionNum; j++ )
    {
      g_motionRefer[g_auiRasterToZscan[i+j]] = g_auiRasterToZscan[i];
    }
  }

  for ( Int i = numPartInWidth*(numPartInHeight-1)+compressionNum*2-1; i < numPartInWidth*numPartInHeight; i += compressionNum*2)
  {
    for ( Int j = 1; j < compressionNum; j++ )
    {
      g_motionRefer[g_auiRasterToZscan[i-j]] = g_auiRasterToZscan[i];
    }
  }
}

Void initRasterToPelXY ( UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth )
{
  UInt    i;
  
  UInt* uiTempX = &g_auiRasterToPelX[0];
  UInt* uiTempY = &g_auiRasterToPelY[0];
  
  UInt  uiMinCUWidth  = uiMaxCUWidth  >> ( uiMaxDepth - 1 );
  UInt  uiMinCUHeight = uiMaxCUHeight >> ( uiMaxDepth - 1 );
  
  UInt  uiNumPartInWidth  = uiMaxCUWidth  / uiMinCUWidth;
  UInt  uiNumPartInHeight = uiMaxCUHeight / uiMinCUHeight;
  
  uiTempX[0] = 0; uiTempX++;
  for ( i = 1; i < uiNumPartInWidth; i++ )
  {
    uiTempX[0] = uiTempX[-1] + uiMinCUWidth; uiTempX++;
  }
  for ( i = 1; i < uiNumPartInHeight; i++ )
  {
    memcpy(uiTempX, uiTempX-uiNumPartInWidth, sizeof(UInt)*uiNumPartInWidth);
    uiTempX += uiNumPartInWidth;
  }
  
  for ( i = 1; i < uiNumPartInWidth*uiNumPartInHeight; i++ )
  {
    uiTempY[i] = ( i / uiNumPartInWidth ) * uiMinCUWidth;
  }
};


Int g_quantScales[6] =
{
  26214,23302,20560,18396,16384,14564
};    

Int g_invQuantScales[6] =
{
  40,45,51,57,64,72
};

const short g_aiT4[4][4] =
{
  { 64, 64, 64, 64},
  { 83, 36,-36,-83},
  { 64,-64,-64, 64},
  { 36,-83, 83,-36}
};

const short g_aiT8[8][8] =
{
  { 64, 64, 64, 64, 64, 64, 64, 64},
  { 89, 75, 50, 18,-18,-50,-75,-89},
  { 83, 36,-36,-83,-83,-36, 36, 83},
  { 75,-18,-89,-50, 50, 89, 18,-75},
  { 64,-64,-64, 64, 64,-64,-64, 64},
  { 50,-89, 18, 75,-75,-18, 89,-50},
  { 36,-83, 83,-36,-36, 83,-83, 36},
  { 18,-50, 75,-89, 89,-75, 50,-18}
};

const short g_aiT16[16][16] =
{
  { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
  { 90, 87, 80, 70, 57, 43, 25,  9, -9,-25,-43,-57,-70,-80,-87,-90},
  { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
  { 87, 57,  9,-43,-80,-90,-70,-25, 25, 70, 90, 80, 43, -9,-57,-87},
  { 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83},
  { 80,  9,-70,-87,-25, 57, 90, 43,-43,-90,-57, 25, 87, 70, -9,-80},
  { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
  { 70,-43,-87,  9, 90, 25,-80,-57, 57, 80,-25,-90, -9, 87, 43,-70},
  { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
  { 57,-80,-25, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 25, 80,-57},
  { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
  { 43,-90, 57, 25,-87, 70,  9,-80, 80, -9,-70, 87,-25,-57, 90,-43},
  { 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36},
  { 25,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-25},
  { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
  {  9,-25, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 25, -9}
};

const short g_aiT32[32][32] =
{
  { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
  { 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13,  4, -4,-13,-22,-31,-38,-46,-54,-61,-67,-73,-78,-82,-85,-88,-90,-90},
  { 90, 87, 80, 70, 57, 43, 25,  9, -9,-25,-43,-57,-70,-80,-87,-90,-90,-87,-80,-70,-57,-43,-25, -9,  9, 25, 43, 57, 70, 80, 87, 90},
  { 90, 82, 67, 46, 22, -4,-31,-54,-73,-85,-90,-88,-78,-61,-38,-13, 13, 38, 61, 78, 88, 90, 85, 73, 54, 31,  4,-22,-46,-67,-82,-90},
  { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89, 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
  { 88, 67, 31,-13,-54,-82,-90,-78,-46, -4, 38, 73, 90, 85, 61, 22,-22,-61,-85,-90,-73,-38,  4, 46, 78, 90, 82, 54, 13,-31,-67,-88},
  { 87, 57,  9,-43,-80,-90,-70,-25, 25, 70, 90, 80, 43, -9,-57,-87,-87,-57, -9, 43, 80, 90, 70, 25,-25,-70,-90,-80,-43,  9, 57, 87},
  { 85, 46,-13,-67,-90,-73,-22, 38, 82, 88, 54, -4,-61,-90,-78,-31, 31, 78, 90, 61,  4,-54,-88,-82,-38, 22, 73, 90, 67, 13,-46,-85},
  { 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83},
  { 82, 22,-54,-90,-61, 13, 78, 85, 31,-46,-90,-67,  4, 73, 88, 38,-38,-88,-73, -4, 67, 90, 46,-31,-85,-78,-13, 61, 90, 54,-22,-82},
  { 80,  9,-70,-87,-25, 57, 90, 43,-43,-90,-57, 25, 87, 70, -9,-80,-80, -9, 70, 87, 25,-57,-90,-43, 43, 90, 57,-25,-87,-70,  9, 80},
  { 78, -4,-82,-73, 13, 85, 67,-22,-88,-61, 31, 90, 54,-38,-90,-46, 46, 90, 38,-54,-90,-31, 61, 88, 22,-67,-85,-13, 73, 82,  4,-78},
  { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75, 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
  { 73,-31,-90,-22, 78, 67,-38,-90,-13, 82, 61,-46,-88, -4, 85, 54,-54,-85,  4, 88, 46,-61,-82, 13, 90, 38,-67,-78, 22, 90, 31,-73},
  { 70,-43,-87,  9, 90, 25,-80,-57, 57, 80,-25,-90, -9, 87, 43,-70,-70, 43, 87, -9,-90,-25, 80, 57,-57,-80, 25, 90,  9,-87,-43, 70},
  { 67,-54,-78, 38, 85,-22,-90,  4, 90, 13,-88,-31, 82, 46,-73,-61, 61, 73,-46,-82, 31, 88,-13,-90, -4, 90, 22,-85,-38, 78, 54,-67},
  { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
  { 61,-73,-46, 82, 31,-88,-13, 90, -4,-90, 22, 85,-38,-78, 54, 67,-67,-54, 78, 38,-85,-22, 90,  4,-90, 13, 88,-31,-82, 46, 73,-61},
  { 57,-80,-25, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 25, 80,-57,-57, 80, 25,-90,  9, 87,-43,-70, 70, 43,-87, -9, 90,-25,-80, 57},
  { 54,-85, -4, 88,-46,-61, 82, 13,-90, 38, 67,-78,-22, 90,-31,-73, 73, 31,-90, 22, 78,-67,-38, 90,-13,-82, 61, 46,-88,  4, 85,-54},
  { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50, 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
  { 46,-90, 38, 54,-90, 31, 61,-88, 22, 67,-85, 13, 73,-82,  4, 78,-78, -4, 82,-73,-13, 85,-67,-22, 88,-61,-31, 90,-54,-38, 90,-46},
  { 43,-90, 57, 25,-87, 70,  9,-80, 80, -9,-70, 87,-25,-57, 90,-43,-43, 90,-57,-25, 87,-70, -9, 80,-80,  9, 70,-87, 25, 57,-90, 43},
  { 38,-88, 73, -4,-67, 90,-46,-31, 85,-78, 13, 61,-90, 54, 22,-82, 82,-22,-54, 90,-61,-13, 78,-85, 31, 46,-90, 67,  4,-73, 88,-38},
  { 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36},
  { 31,-78, 90,-61,  4, 54,-88, 82,-38,-22, 73,-90, 67,-13,-46, 85,-85, 46, 13,-67, 90,-73, 22, 38,-82, 88,-54, -4, 61,-90, 78,-31},
  { 25,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-25,-25, 70,-90, 80,-43, -9, 57,-87, 87,-57,  9, 43,-80, 90,-70, 25},
  { 22,-61, 85,-90, 73,-38, -4, 46,-78, 90,-82, 54,-13,-31, 67,-88, 88,-67, 31, 13,-54, 82,-90, 78,-46,  4, 38,-73, 90,-85, 61,-22},
  { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18, 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
  { 13,-38, 61,-78, 88,-90, 85,-73, 54,-31,  4, 22,-46, 67,-82, 90,-90, 82,-67, 46,-22, -4, 31,-54, 73,-85, 90,-88, 78,-61, 38,-13},
  {  9,-25, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 25, -9, -9, 25,-43, 57,-70, 80,-87, 90,-90, 87,-80, 70,-57, 43,-25,  9},
  {  4,-13, 22,-31, 38,-46, 54,-61, 67,-73, 78,-82, 85,-88, 90,-90, 90,-90, 88,-85, 82,-78, 73,-67, 61,-54, 46,-38, 31,-22, 13, -4}
};

const UChar g_aucChromaScale[52]=
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
  12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
  28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
  37,38,38,38,39,39,39,39
};
// ====================================================================================================================
// TENTM VLC table
// ====================================================================================================================

#define M1 MAX_UINT

const Int atable[5] = {3,7,15,31,0xfffffff};
// Below table need to be optimized
const UInt g_auiCbpVlcNum[2][8] =
{
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

const UInt g_auiCBP_YUV_TableE[4][8] = 
{
  {2,5,6,7,0,3,4,1},
  {0,4,6,7,1,2,3,5},
  {2,5,6,7,0,3,4,1},
  {0,4,6,7,1,2,3,5}
};
const UInt g_auiCBP_YUV_TableD[4][8] = 
{
  {4,7,0,5,6,1,2,3},
  {0,4,5,6,1,7,2,3},
  {4,7,0,5,6,1,2,3},
  {0,4,5,6,1,7,2,3}
};
const UInt g_auiCBP_YS_TableE[2][4] = 
{
  {3,  2, 0, 1},
  {2, M1, 0, 1}
};

const UInt g_auiCBP_YS_TableD[2][4] = 
{
  {  2,  3,  1,  0},
  {  2,  3,  0, M1}
};

const UInt g_auiCBP_YC_TableE[2][4] =
{ 
  {  2,  1,  3,  0}, 
  {  0,  2,  1,  3}
};

const UInt g_auiCBP_YC_TableD[2][4] =
{ 
  {3,1,0,2}, 
  {0,2,1,3}
};

const UInt g_auiCBP_YCS_Table[2][8] = 
{
  {  0,  3,  9, 11,  8, 20, 42, 43},
  {  1,  1,  1,  1,  1,  1,  0, M1}
};

const UInt g_auiCBP_YCS_TableLen[2][8] = 
{
  {  1,  2,  4,  4,  4,  5,  6,  6},
  {  1,  2,  3,  4,  5,  6,  6,  0}
};

const UInt g_auiCBP_YCS_TableE[2][8] = 
{
  {4,  5,  6,  7,  1,  2,  0,  3},
  {0,  7,  5,  6,  2,  1,  4,  3}
};

const UInt g_auiCBP_YCS_TableD[2][8] = 
{
  {6,  4,  5,  7,  0,  1,  2,  3},
  {0,  5,  4,  7,  6,  2,  3, 1}
};

const UInt g_auiCBP_4Y_TableE[2][15] = 
{
  {14, 13, 10, 12,  9,  8,  4, 11,  7,  6,  3,  5,  2,  1,  0},
  { 0,  1,  4,  2,  5,  6, 10,  3,  7,  8, 11,  9, 12, 13, 14}
};

const UInt g_auiCBP_4Y_TableD[2][15] = 
{
  {14, 13, 12, 10, 6, 11, 9, 8, 5, 4, 2, 7, 3, 1, 0},
  {0, 1, 3, 7, 2, 4, 5, 8, 9, 11, 6, 10 ,12, 13, 14} 
};

const UInt g_auiCBP_4Y_VlcNum[15] = 
{
  1,  2,  2,  2, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
};

const UInt g_auiComMI1TableE[9] = {0,1,2,3,4,5,6,7,8};
const UInt g_auiComMI1TableD[9] = {0,1,2,3,4,5,6,7,8};

#if AMP
const UInt g_auiInterModeTableE[4][11] = {{0,1,2,3,4,5,6,7,8,9,10},{0,1,2,3,4,5,6,7,8,9,10},{0,1,2,3,4,5,6,7,8,9,10},{6,0,1,2,3,4,5,7,8,9,10}};
const UInt g_auiInterModeTableD[4][11] = {{0,1,2,3,4,5,6,7,8,9,10},{0,1,2,3,4,5,6,7,8,9,10},{0,1,2,3,4,5,6,7,8,9,10},{1,2,3,4,5,6,0,7,8,9,10}};
#else
const UInt g_auiInterModeTableE[4][7] = {{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{6,0,1,2,3,4,5}};
const UInt g_auiInterModeTableD[4][7] = {{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{1,2,3,4,5,6,0}};
#endif

// Below table need to be optimized
const UInt g_auiMITableVlcNum[15] = 
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


const UInt g_auiLPTableE4[3][32] =
{
  {0,1,2,3,5,4,7,6,9,11,14,8,16,15,10,13,12,17,18,19,25,23,20,22,28,26,29,24,30,31,27,21},  //4x4I
  {0,1,2,7,5,3,6,4,11,8,12,10,9,14,13,15,16,17,21,27,26,18,19,23,29,20,25,28,22,30,24,31},  //4x4P
  {0,1,2,7,5,3,6,4,11,8,12,10,9,14,13,15,16,17,21,27,26,18,19,23,29,20,25,28,22,30,24,31}   //4x4B
};

const UInt g_auiLPTableD4[3][32] =
{
  {0,1,2,3,5,4,7,6,11,8,14,9,16,15,10,13,12,17,18,19,22,31,23,21,27,20,25,30,24,26,28,29},  //4x4I
  {0,1,2,5,7,4,6,3,9,12,11,8,10,14,13,15,16,17,21,22,25,18,28,23,30,26,20,19,27,24,29,31},  //4x4P
  {0,1,2,5,7,4,6,3,9,12,11,8,10,14,13,15,16,17,21,22,25,18,28,23,30,26,20,19,27,24,29,31}   //4x4B
};

const UInt  g_auiIntraModeTableD17[17] = { 0, 3, 2, 15, 8, 11, 1, 10, 7, 4, 14, 9, 6, 5, 13, 12, 0 };
const UInt  g_auiIntraModeTableE17[17] = { 0, 6, 2, 1, 9, 13, 12, 8, 4, 11, 7, 5, 15, 14, 10, 3, 0 };
const UInt  g_auiIntraModeTableD34[34] = { 0, 2, 3, 29, 1, 8, 30, 21, 28, 16, 7, 15, 20, 31, 9, 11, 6, 4, 5, 12, 10, 14, 22, 19, 17, 27, 13, 18, 23, 26, 32, 24, 25, 0 };
const UInt  g_auiIntraModeTableE34[34] = { 0, 4, 1, 2, 17, 18, 16, 10, 5, 14, 20, 15, 19, 26, 21, 11, 9, 24, 27, 23, 12, 7, 22, 28, 31, 32, 29, 25, 8, 3, 6, 13, 30, 0 };

const UInt g_auiLastPosVlcIndex[10] = {0,0,0,0,0,0,0,0,0,0};

const UInt g_auiLastPosVlcNum[10][17] =
{
  {10,10,10,10, 2,2,2,7,9,9,9,9,9,4,4,4,4},
  {10,10,10,10,10,2,9,9,9,9,9,9,9,4,4,4,4},
  { 2, 2, 2, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 4,13},
  { 2, 2, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,13},
  { 2, 2, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,13},
  {10,10,10, 4, 4, 4, 4,12,12,12,12,12,12,12,12,12,12},
  {10,10,10,10, 4, 4,12,12,12,12,12,12,12,12,12,12,12},
  {10,10,10,10, 4, 4,12,12,12,12,12,12,12,12,12,12,12},
  { 2, 2, 2, 2, 7,7,7,7,7,7,7,7,7,7,7,7,4},
  { 2, 2, 2, 2, 7,7,7,7,7,7,7,7,7,7,7,7,4}
};

const UInt g_auiLumaRunTr14x4[5][15]=
{
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {2, 2, 2, 2, 3, 2, 2, 4, 4, 2, 4, 4, 2, 2, 2},
  {2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, M1},
  {1, 1, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, M1, M1},
  {1, 1, 1, 2, 1, 1, 2, 2, 0, 0, 2, 0, 0, M1, M1}
};
const UInt g_auiLumaRunTr18x8[2][29]=
{
  {2,2,1,2,2,2,2,4,2,2,2,2,2,4,4,6,4,4,4,4,4,4,6,6,4,4,4,6,6},
  {2,3,3,3,3,2,4,8,4,2,4,4,4,6,6,8,6,6,6,8,8,8,10,12,8,8,8,10,14}
};
const UInt g_auiLumaRunTr116x16[2][29]=
{
  {1,1,1,2,2,2,2,2,2,2,2,4,4,4,4,4,4,6,6,6,4,4,6,6,6,6,8,6,10},
  {1,3,2,3,4,4,4,6,6,6,6,6,8,14,10,8,8,10,10,16,12,10,10,12,12,12,28,16,22}
};

const UInt *g_pLumaRunTr14x4[5] =
{ 
  &g_auiLumaRunTr14x4[0][0], &g_auiLumaRunTr14x4[1][0], &g_auiLumaRunTr14x4[2][0], 
  &g_auiLumaRunTr14x4[3][0], &g_auiLumaRunTr14x4[4][0]
};

const UInt *g_pLumaRunTr18x8[2] = 
{ 
  &g_auiLumaRunTr18x8[0][0], &g_auiLumaRunTr18x8[1][0]
};
const UInt *g_pLumaRunTr116x16[2] = 
{ 
  &g_auiLumaRunTr116x16[0][0], &g_auiLumaRunTr116x16[1][0]
};

const UInt g_auiVlcTable8x8Inter[29]   = {8,0,0,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3};
const UInt g_auiVlcTable8x8Intra[29]   = {8,0,0,0,0,1,5,5,5,5,5,5,5,5,5,5,5,5,1,1,1,1,1,1,1,1,1,1,1}; 
const UInt g_auiVlcTable16x16Inter[29] = {8,0,1,1,1,1,2,2,2,2,2,2,2,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3};

const UInt huff17_2[18]       = { 1, 0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 31, 30, 0 };
const UInt lengthHuff17_2[18] = { 1, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 0 };
const UInt huff34_2[35]       = { 1, 0, 5, 4, 3, 2, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 127, 126, 0 };
const UInt lengthHuff34_2[35] = { 1, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 0 };

// Mode-Dependent DCT/DST 
const short g_as_DST_MAT_4 [4][4]=
{
  {29,   55,    74,   84},
  {74,   74,    0 ,  -74},
  {84,  -29,   -74,   55},
  {55,  -84,    74,  -29},
};
// Mapping each Unified Directional Intra prediction direction to DCT/DST transform 
// 0 implies use DCT, 1 implies DST
const UChar g_aucDCTDSTMode_Vert[NUM_INTRA_MODE] =
{ //0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33
  1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
};
const UChar g_aucDCTDSTMode_Hor[NUM_INTRA_MODE] =
{ //0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33
  1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0
};



// ====================================================================================================================
// ADI
// ====================================================================================================================

#if FAST_UDI_USE_MPM
const UChar g_aucIntraModeNumFast[7] =
{
  3,  //   2x2
  8,  //   4x4
  8,  //   8x8
  3,  //  16x16   
  3,  //  32x32   
  3,  //  64x64   
  3   // 128x128  
};
#else // FAST_UDI_USE_MPM
const UChar g_aucIntraModeNumFast[7] =
{
  3,  //   2x2
  9,  //   4x4
  9,  //   8x8
  4,  //  16x16   33
  4,  //  32x32   33
  5,  //  64x64   33
  4   // 128x128  33
};
#endif // FAST_UDI_USE_MPM

// chroma

const UChar g_aucConvertTxtTypeToIdx[4] = { 0, 1, 1, 2 };

// ====================================================================================================================
// Angular Intra prediction
// ====================================================================================================================

// g_aucAngIntraModeOrder
//   Indexing this array with the mode indicated in the bitstream
//   gives a logical index used in the prediction functions.
const UChar g_aucAngIntraModeOrder[NUM_INTRA_MODE] =
{     //  ModeOrder LogicalOrderInPredFunctions
  34, //  PLANAR_IDX PLANAR PLANAR
  9,  //  0 VER     DC
  25, //  1 HOR     VER-8 (diagonal from top-left to bottom-right = HOR-8)
  0,  //  2 DC      VER-7
  1,  //  4 VER-8   VER-6
  5,  //  5 VER-4   VER-5
  13, //  6 VER+4   VER-4
  17, //  7 VER+8   VER-3
  21, //  8 HOR-4   VER-2
  29, //  9 HOR+4   VER-1
  33, // 10 HOR+8   VER
  3,  // 11 VER-6   VER+1
  7,  // 12 VER-2   VER+2
  11, // 13 VER+2   VER+3
  15, // 14 VER+6   VER+4
  19, // 15 HOR-6   VER+5
  23, // 16 HOR-2   VER+6
  27, // 17 HOR+2   VER+7
  31, // 18 HOR+6   VER+8
  2,  // 19 VER-7   HOR-7
  4,  // 20 VER-5   HOR-6
  6,  // 21 VER-3   HOR-5
  8,  // 22 VER-1   HOR-4
  10, // 23 VER+1   HOR-3
  12, // 24 VER+3   HOR-2
  14, // 25 VER+5   HOR-1
  16, // 26 VER+7   HOR
  18, // 27 HOR-7   HOR+1
  20, // 28 HOR-5   HOR+2
  22, // 29 HOR-3   HOR+3
  24, // 30 HOR-1   HOR+4
  26, // 31 HOR+1   HOR+5
  28, // 32 HOR+3   HOR+6
  30, // 33 HOR+5   HOR+7
  32, // 34 HOR+7   HOR+8
  0, // LM_CHROMA_IDX 
};

const UChar g_aucIntraModeNumAng[7] =
{
  4,  //   2x2
  18,  //   4x4
  35,  //   8x8
  35,  //  16x16
  35,  //  32x32
  4,  //  64x64
  6   // 128x128
};

const UChar g_aucIntraModeBitsAng[7] =
{
  2,  //   2x2     3   1+1
  5,  //   4x4    17   4+1
  6,  //   8x8    34   5+esc
  6,  //  16x16   34   5+esc
  6,  //  32x32   34   5+esc
  2,  //  64x64    3   1+1
  3   // 128x128   5   2+1
};

const UChar g_aucAngModeMapping[4][35] = // intra mode conversion for most probable
{
  {3,  4,  3,  3,  5,  5,  5,  1,  1,  1,  1,  1,  1,  1,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3},        // conversion to 5 modes
  {3,  4,  4,  3,  5,  5,  5,  3,  1,  1,  1,  3,  6,  6,  6,  3,  7,  7,  4,  3,  8,  8,  8,  3,  2,  2,  2,  3,  9,  9,  9,  3,  3,  3},        // conversion to 9 modes
  {3,  4,  4,  11, 11, 5,  12, 12, 1,  1,  1,  13, 13, 6,  6,  14, 14, 7,  15, 15, 8,  8,  16, 16, 2,  2,  2,  17, 17, 9,  9,  3,  3,  10},       // conversion to 17 modes
  {3,  3,  3,  3,  3,  3,  3,  1,  1,  1,  1,  1,  1,  1,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3}         // conversion to 3 modes
};

// ====================================================================================================================
// Bit-depth
// ====================================================================================================================

UInt g_uiBitDepth     = 8;    // base bit-depth
UInt g_uiBitIncrement = 0;    // increments
UInt g_uiIBDI_MAX     = 255;  // max. value after  IBDI
UInt g_uiBASE_MAX     = 255;  // max. value before IBDI

#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
UInt g_uiPCMBitDepthLuma     = 8;    // PCM bit-depth
UInt g_uiPCMBitDepthChroma   = 8;    // PCM bit-depth
#endif

// ====================================================================================================================
// Misc.
// ====================================================================================================================

Char  g_aucConvertToBit  [ MAX_CU_SIZE+1 ];

#if ENC_DEC_TRACE
FILE*  g_hTrace = NULL;
const Bool g_bEncDecTraceEnable  = true;
const Bool g_bEncDecTraceDisable = false;
Bool   g_bJustDoIt = false;
UInt64 g_nSymbolCounter = 0;
#endif
// ====================================================================================================================
// Scanning order & context model mapping
// ====================================================================================================================

// scanning order table
UInt* g_auiFrameScanXY[ MAX_CU_DEPTH  ];
UInt* g_auiFrameScanX [ MAX_CU_DEPTH  ];
UInt* g_auiFrameScanY [ MAX_CU_DEPTH  ];
#if DIAG_SCAN
UInt* g_auiSigLastScan[4][ MAX_CU_DEPTH ];
#else
UInt* g_auiSigLastScan[3][ MAX_CU_DEPTH ];
#endif

#if NSQT
UInt* g_auiNonSquareSigLastScan[ 2 ];
#endif

const UInt g_uiLastCtx[ 32 ] =
{
  0, 1, 2, 2, // 4x4
  3, 4, 5, 5, // 8x8
  6, 7, 8, 9, 10, 10, 11, 11, // 16x16
  12, 13, 14, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18 // 32x32
};

// scanning order to 8x8 context model mapping table
UInt  g_auiAntiScan8  [64];

// Rice parameters for absolute transform levels
const UInt g_auiGoRiceRange[4] =
{
  7, 20, 42, 70
};

const UInt g_auiGoRicePrefixLen[4] =
{
#if CABAC_RICE_FIX
  8, 10, 10, 8
#else
  8, 10, 11, 8
#endif
};

const UInt g_aauiGoRiceUpdate[4][16] =
{
  {
    0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3
  },
  {
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3
  },
  {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3
  },
  {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
  }
};

// initialize g_auiFrameScanXY
Void initFrameScanXY( UInt* pBuff, UInt* pBuffX, UInt* pBuffY, Int iWidth, Int iHeight )
{
  Int x, y, c = 0;
  
  // starting point
  pBuffX[ c ] = 0;
  pBuffY[ c ] = 0;
  pBuff[ c++ ] = 0;
  
  // loop
  x=1; y=0;
  while (1)
  {
    // decrease loop
    while ( x>=0 )
    {
      if ( x >= 0 && x < iWidth && y >= 0 && y < iHeight )
      {
        pBuffX[ c ] = x;
        pBuffY[ c ] = y;
        pBuff[ c++ ] = x+y*iWidth;
      }
      x--; y++;
    }
    x=0;
    
    // increase loop
    while ( y>=0 )
    {
      if ( x >= 0 && x < iWidth && y >= 0 && y < iHeight )
      {
        pBuffX[ c ] = x;
        pBuffY[ c ] = y;
        pBuff[ c++ ] = x+y*iWidth;
      }
      x++; y--;
    }
    y=0;
    
    // termination condition
    if ( c >= iWidth*iHeight ) break;
  }
  
  // LTR_2D_CONTEXT_MAPPING
  if (iWidth == 8 && iHeight == 8)
  {
    for( c = 0; c < iWidth*iHeight; c++)
    {
      g_auiAntiScan8[pBuff[c]] = c;
    }
  }
}

#if DIAG_SCAN
Void initSigLastScan(UInt* pBuffZ, UInt* pBuffH, UInt* pBuffV, UInt* pBuffD, Int iWidth, Int iHeight, Int iDepth)
#else
Void initSigLastScan(UInt* pBuffZ, UInt* pBuffH, UInt* pBuffV, Int iWidth, Int iHeight, Int iDepth)
#endif
{
#if DIAG_SCAN
  const UInt  uiNumScanPos  = UInt( iWidth * iWidth );
  UInt        uiNextScanPos = 0;

  for( UInt uiScanLine = 0; uiNextScanPos < uiNumScanPos; uiScanLine++ )
  {
    int    iPrimDim  = int( uiScanLine );
    int    iScndDim  = 0;
    while( iPrimDim >= iWidth )
    {
      iScndDim++;
      iPrimDim--;
    }
    while( iPrimDim >= 0 && iScndDim < iWidth )
    {
      pBuffD[ uiNextScanPos ] = iPrimDim * iWidth + iScndDim ;
      uiNextScanPos++;
      iScndDim++;
      iPrimDim--;
    }
  }
#endif
  
  memcpy(pBuffZ, g_auiFrameScanXY[iDepth], sizeof(UInt)*iWidth*iHeight);

  UInt uiCnt = 0;
  for(Int iY=0; iY < iHeight; iY++)
  {
    for(Int iX=0; iX < iWidth; iX++)
    {
      pBuffH[uiCnt] = iY*iWidth + iX;
      uiCnt ++;
    }
  }

  uiCnt = 0;
  for(Int iX=0; iX < iWidth; iX++)
  {
    for(Int iY=0; iY < iHeight; iY++)
    {
      pBuffV[uiCnt] = iY*iWidth + iX;
      uiCnt ++;
    }
  }    
}

#if NSQT
Void initNonSquareSigLastScan(UInt* pBuffZ, UInt uiWidth, UInt uiHeight)
{

  Int x, y, c = 0;

  // starting point
  pBuffZ[ c++ ] = 0;

  // loop
  x=0; y=1;
  while (1)
  {
    // increase loop
    while ( y>=0 )
    {
      if ( x >= 0 && x < uiWidth && y >= 0 && y < uiHeight )
      {
        pBuffZ[ c++ ] = x + y * uiWidth;
      }
      x++;
      y--;
    }
    y=0;

    // decrease loop
    while ( x>=0 )
    {
      if ( x >= 0 && x < uiWidth && y >= 0 && y < uiHeight )
      {
        pBuffZ[ c++ ] = x + y * uiWidth;
      }
      x--;
      y++;
    }
    x=0;

    // termination condition
    if ( c >= uiWidth * uiHeight ) 
      break;
  }

}
#endif
//! \}
