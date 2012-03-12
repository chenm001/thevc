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

/** \file     TComTrQuant.cpp
    \brief    transform and quantization class
*/

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "TComTrQuant.h"
#include "TComPic.h"
#include "ContextTables.h"

#if (CHEN_TV)
extern FILE *fp_tv;
extern int do_print;
extern void tPrintMatrix16( FILE *fp, char *name, Pel *P, UInt uiStride, Int iSize );
extern void tPrintMatrix32( FILE *fp, char *name, Int *P, UInt uiStride, Int iSize );
#endif

typedef struct
{
  Int    iNNZbeforePos0;
  Double d64CodedLevelandDist; // distortion and level cost only
  Double d64UncodedDist;    // all zero coded block distortion
  Double d64SigCost;
  Double d64SigCost_0;
} coeffGroupRDStats;

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_CHROMA                 1           ///< use of RDOQ in chroma

// ====================================================================================================================
// Tables
// ====================================================================================================================

// ====================================================================================================================
// Qp class member functions
// ====================================================================================================================

QpParam::QpParam()
{
}

// ====================================================================================================================
// TComTrQuant class member functions
// ====================================================================================================================

TComTrQuant::TComTrQuant()
{
  m_cQP.clear();
  
  // allocate temporary buffers
  m_plTempCoeff  = new Int[ MAX_CU_SIZE*MAX_CU_SIZE ];
}

TComTrQuant::~TComTrQuant()
{
  // delete temporary buffers
  if ( m_plTempCoeff )
  {
    delete [] m_plTempCoeff;
    m_plTempCoeff = NULL;
  }
}

/// Including Chroma QP Parameter setting
Void TComTrQuant::setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType)
{
  iQP = Clip3( MIN_QP, MAX_QP, iQP );
  
  if(eTxtType != TEXT_LUMA) //Chroma
  {
    iQP  = g_aucChromaScale[ iQP ];
  }
  
  m_cQP.setQpParam( iQP, bLowpass, eSliceType );
}

#if MATRIX_MULT
/** NxN forward transform (2D) using brute force matrix multiplication (3 nested loops)
 *  \param block pointer to input data (residual)
 *  \param coeff pointer to output data (transform coefficients)
 *  \param uiStride stride of input data
 *  \param uiTrSize transform size (uiTrSize x uiTrSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr(Pel *block, Int *coeff, UInt uiStride, UInt uiTrSize, UInt uiMode)
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
  else
  {
    assert(0);
  }

  int shift_1st = uiLog2TrSize - 1; // log2(N) - 1 + g_uiBitIncrement

  int add_1st = 1<<(shift_1st-1);
  int shift_2nd = uiLog2TrSize + 6;
  int add_2nd = 1<<(shift_2nd-1);

  /* Horizontal transform */

  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])
    {
      iT  =  g_as_DST_MAT_4[0];
    }
  }
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
void xITr(Int *coeff, Pel *block, UInt uiStride, UInt uiTrSize, UInt uiMode)
{
  int i,j,k,iSum;
  Int tmp[32*32];
  const short *iT;
  
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
  else
  {
    assert(0);
  }
  
  int shift_1st = SHIFT_INV_1ST;
  int add_1st = 1<<(shift_1st-1);  
  int shift_2nd = SHIFT_INV_2ND;
  int add_2nd = 1<<(shift_2nd-1);
  if (uiTrSize==4)
  {
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] ) // Check for DCT or DST
    {
      iT  =  g_as_DST_MAT_4[0];
    }
  }
  
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
      tmp[i*uiTrSize+j] = Clip3(-32768, 32767, (iSum + add_1st)>>shift_1st); // Clipping is normative
    }
  }   
  
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
      block[i*uiStride+j] = Clip3(-32768, 32767, (iSum + add_2nd)>>shift_2nd); // Clipping is non-normative
    }
  }
}

#else //MATRIX_MULT

/** 4x4 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
#if !UNIFIED_TRANSFORM
void partialButterfly4(short src[4][4],short dst[4][4],int shift)
{
  int j;  
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* E and O */
    E[0] = src[j][0] + src[j][3];
    O[0] = src[j][0] - src[j][3];
    E[1] = src[j][1] + src[j][2];
    O[1] = src[j][1] - src[j][2];

    dst[0][j] = (g_aiT4[0][0]*E[0] + g_aiT4[0][1]*E[1] + add)>>shift;
    dst[2][j] = (g_aiT4[2][0]*E[0] + g_aiT4[2][1]*E[1] + add)>>shift;
    dst[1][j] = (g_aiT4[1][0]*O[0] + g_aiT4[1][1]*O[1] + add)>>shift;
    dst[3][j] = (g_aiT4[3][0]*O[0] + g_aiT4[3][1]*O[1] + add)>>shift;
  }
}
#endif

void partialButterfly4(short *src,short *dst,int shift, int line)
{
  int j;  
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {    
    /* E and O */
    E[0] = src[0] + src[3];
    O[0] = src[0] - src[3];
    E[1] = src[1] + src[2];
    O[1] = src[1] - src[2];

    dst[0] = (g_aiT4[0][0]*E[0] + g_aiT4[0][1]*E[1] + add)>>shift;
    dst[2*line] = (g_aiT4[2][0]*E[0] + g_aiT4[2][1]*E[1] + add)>>shift;
    dst[line] = (g_aiT4[1][0]*O[0] + g_aiT4[1][1]*O[1] + add)>>shift;
    dst[3*line] = (g_aiT4[3][0]*O[0] + g_aiT4[3][1]*O[1] + add)>>shift;

    src += 4;
    dst ++;
  }
}

// Fast DST Algorithm. Full matrix multiplication for DST and Fast DST algorithm 
// give identical results
#if UNIFIED_TRANSFORM
void fastForwardDst(short *block,short *coeff,int shift)  // input block, output coeff
#else
void fastForwardDst(short block[4][4],short coeff[4][4],int shift)  // input block, output coeff
#endif
{
  int i, c[4];
  int rnd_factor = 1<<(shift-1);
  for (i=0; i<4; i++)
  {
    // Intermediate Variables
#if UNIFIED_TRANSFORM
    c[0] = block[4*i+0] + block[4*i+3];
    c[1] = block[4*i+1] + block[4*i+3];
    c[2] = block[4*i+0] - block[4*i+1];
    c[3] = 74* block[4*i+2];

    coeff[   i] =  ( 29 * c[0] + 55 * c[1]         + c[3]               + rnd_factor ) >> shift;
    coeff[ 4+i] =  ( 74 * (block[4*i+0]+ block[4*i+1] - block[4*i+3])   + rnd_factor ) >> shift;
    coeff[ 8+i] =  ( 29 * c[2] + 55 * c[0]         - c[3]               + rnd_factor ) >> shift;
    coeff[12+i] =  ( 55 * c[2] - 29 * c[1]         + c[3]               + rnd_factor ) >> shift;
#else
    c[0] = block[i][0] + block[i][3];
    c[1] = block[i][1] + block[i][3];
    c[2] = block[i][0] - block[i][1];
    c[3] = 74* block[i][2];
    
    coeff[0][i] =  ( 29 * c[0] + 55 * c[1]         + c[3]               + rnd_factor ) >> shift;
    coeff[1][i] =  ( 74 * (block[i][0]+ block[i][1] - block[i][3])      + rnd_factor ) >> shift;
    coeff[2][i] =  ( 29 * c[2] + 55 * c[0]         - c[3]               + rnd_factor ) >> shift;
    coeff[3][i] =  ( 55 * c[2] - 29 * c[1]         + c[3]               + rnd_factor ) >> shift;
#endif
  }
}

#if UNIFIED_TRANSFORM
void fastInverseDst(short *tmp,short *block,int shift)  // input tmp, output block
#else
void fastInverseDst(short tmp[4][4],short block[4][4],int shift)  // input tmp, output block
#endif
{
  int i, c[4];
  int rnd_factor = 1<<(shift-1);
  for (i=0; i<4; i++)
  {  
    // Intermediate Variables
#if UNIFIED_TRANSFORM
    c[0] = tmp[  i] + tmp[ 8+i];
    c[1] = tmp[8+i] + tmp[12+i];
    c[2] = tmp[  i] - tmp[12+i];
    c[3] = 74* tmp[4+i];

    block[4*i+0] = Clip3( -32768, 32767, ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[4*i+1] = Clip3( -32768, 32767, ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[4*i+2] = Clip3( -32768, 32767, ( 74 * (tmp[i] - tmp[8+i]  + tmp[12+i])      + rnd_factor ) >> shift );
    block[4*i+3] = Clip3( -32768, 32767, ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift );
#else
    c[0] = tmp[0][i] + tmp[2][i];
    c[1] = tmp[2][i] + tmp[3][i];
    c[2] = tmp[0][i] - tmp[3][i];
    c[3] = 74* tmp[1][i];

    block[i][0] = Clip3( -32768, 32767, ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][1] = Clip3( -32768, 32767, ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][2] = Clip3( -32768, 32767, ( 74 * (tmp[0][i] - tmp[2][i]  + tmp[3][i])      + rnd_factor ) >> shift );
    block[i][3] = Clip3( -32768, 32767, ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift );
#endif
  }
}
#if !UNIFIED_TRANSFORM
/** 4x4 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr4(short block[4][4],short coeff[4][4],UInt uiMode)
{
  int shift_1st = 1; // log2(4) - 1 + g_uiBitIncrement
  int shift_2nd = 8;                    // log2(4) + 6
  short tmp[4][4]; 
#if LOGI_INTRA_NAME_3MPM
  if (uiMode != REG_DCT && (!uiMode || (uiMode>=2 && uiMode <= 25)))    // Check for DCT or DST
#else
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])// Check for DCT or DST
#endif
  {
    fastForwardDst(block,tmp,shift_1st); // Forward DST BY FAST ALGORITHM, block input, tmp output
  }
  else  
  {
    partialButterfly4(block,tmp,shift_1st);
  }

#if LOGI_INTRA_NAME_3MPM
  if (uiMode != REG_DCT && (!uiMode || (uiMode>=11 && uiMode <= 34)))    // Check for DCT or DST
#else
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )   // Check for DCT or DST
#endif
  {
    fastForwardDst(tmp,coeff,shift_2nd); // Forward DST BY FAST ALGORITHM, tmp input, coeff output
  }
  else  
  {
    partialButterfly4(tmp,coeff,shift_2nd);
  }   
}

/** 4x4 inverse transform implemented using partial butterfly structure (1D)
 *  \param src   input data (transform coefficients)
 *  \param dst   output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse4(short src[4][4],short dst[4][4],int shift)
{
  int j;    
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<4; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */    
    O[0] = g_aiT4[1][0]*src[1][j] + g_aiT4[3][0]*src[3][j];
    O[1] = g_aiT4[1][1]*src[1][j] + g_aiT4[3][1]*src[3][j];
    E[0] = g_aiT4[0][0]*src[0][j] + g_aiT4[2][0]*src[2][j];
    E[1] = g_aiT4[0][1]*src[0][j] + g_aiT4[2][1]*src[2][j];
    
    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    dst[j][0] = Clip3( -32768, 32767, (E[0] + O[0] + add)>>shift );
    dst[j][1] = Clip3( -32768, 32767, (E[1] + O[1] + add)>>shift );
    dst[j][2] = Clip3( -32768, 32767, (E[1] - O[1] + add)>>shift );
    dst[j][3] = Clip3( -32768, 32767, (E[0] - O[0] + add)>>shift );
  }
}
#endif

void partialButterflyInverse4(short *src,short *dst,int shift, int line)
{
  int j;    
  int E[2],O[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */    
    O[0] = g_aiT4[1][0]*src[line] + g_aiT4[3][0]*src[3*line];
    O[1] = g_aiT4[1][1]*src[line] + g_aiT4[3][1]*src[3*line];
    E[0] = g_aiT4[0][0]*src[0] + g_aiT4[2][0]*src[2*line];
    E[1] = g_aiT4[0][1]*src[0] + g_aiT4[2][1]*src[2*line];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    dst[0] = Clip3( -32768, 32767, (E[0] + O[0] + add)>>shift );
    dst[1] = Clip3( -32768, 32767, (E[1] + O[1] + add)>>shift );
    dst[2] = Clip3( -32768, 32767, (E[1] - O[1] + add)>>shift );
    dst[3] = Clip3( -32768, 32767, (E[0] - O[0] + add)>>shift );
            
    src   ++;
    dst += 4;
  }
}

#if !UNIFIED_TRANSFORM
/** 4x4 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr4(short coeff[4][4],short block[4][4], UInt uiMode)
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  short tmp[4][4];
  
#if LOGI_INTRA_NAME_3MPM
  if (uiMode != REG_DCT && (!uiMode || (uiMode>=11 && uiMode <= 34)))    // Check for DCT or DST
#else
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )    // Check for DCT or DST
#endif
  {
    fastInverseDst(coeff,tmp,shift_1st);    // Inverse DST by FAST Algorithm, coeff input, tmp output
  }
  else
  {
    partialButterflyInverse4(coeff,tmp,shift_1st);    
  } 
#if LOGI_INTRA_NAME_3MPM
  if (uiMode != REG_DCT && (!uiMode || (uiMode>=2 && uiMode <= 25)))    // Check for DCT or DST
#else
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode] )    // Check for DCT or DST
#endif
  {
    fastInverseDst(tmp,block,shift_2nd); // Inverse DST by FAST Algorithm, tmp input, coeff output
  }
  else
  {
    partialButterflyInverse4(tmp,block,shift_2nd);
  }   
}

/** 8x8 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly8(short src[8][8],short dst[8][8],int shift)
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
      E[k] = src[j][k] + src[j][7-k];
      O[k] = src[j][k] - src[j][7-k];
    }    
    /* EE and EO */
    EE[0] = E[0] + E[3];    
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    dst[0][j] = (g_aiT8[0][0]*EE[0] + g_aiT8[0][1]*EE[1] + add)>>shift;
    dst[4][j] = (g_aiT8[4][0]*EE[0] + g_aiT8[4][1]*EE[1] + add)>>shift; 
    dst[2][j] = (g_aiT8[2][0]*EO[0] + g_aiT8[2][1]*EO[1] + add)>>shift;
    dst[6][j] = (g_aiT8[6][0]*EO[0] + g_aiT8[6][1]*EO[1] + add)>>shift; 

    dst[1][j] = (g_aiT8[1][0]*O[0] + g_aiT8[1][1]*O[1] + g_aiT8[1][2]*O[2] + g_aiT8[1][3]*O[3] + add)>>shift;
    dst[3][j] = (g_aiT8[3][0]*O[0] + g_aiT8[3][1]*O[1] + g_aiT8[3][2]*O[2] + g_aiT8[3][3]*O[3] + add)>>shift;
    dst[5][j] = (g_aiT8[5][0]*O[0] + g_aiT8[5][1]*O[1] + g_aiT8[5][2]*O[2] + g_aiT8[5][3]*O[3] + add)>>shift;
    dst[7][j] = (g_aiT8[7][0]*O[0] + g_aiT8[7][1]*O[1] + g_aiT8[7][2]*O[2] + g_aiT8[7][3]*O[3] + add)>>shift;
  }
}
#endif

void partialButterfly8(short *src,short *dst,int shift, int line)
{
  int j,k;  
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {  
    /* E and O*/
    for (k=0;k<4;k++)
    {
      E[k] = src[k] + src[7-k];
      O[k] = src[k] - src[7-k];
    }    
    /* EE and EO */
    EE[0] = E[0] + E[3];    
    EO[0] = E[0] - E[3];
    EE[1] = E[1] + E[2];
    EO[1] = E[1] - E[2];

    dst[0] = (g_aiT8[0][0]*EE[0] + g_aiT8[0][1]*EE[1] + add)>>shift;
    dst[4*line] = (g_aiT8[4][0]*EE[0] + g_aiT8[4][1]*EE[1] + add)>>shift; 
    dst[2*line] = (g_aiT8[2][0]*EO[0] + g_aiT8[2][1]*EO[1] + add)>>shift;
    dst[6*line] = (g_aiT8[6][0]*EO[0] + g_aiT8[6][1]*EO[1] + add)>>shift; 

    dst[line] = (g_aiT8[1][0]*O[0] + g_aiT8[1][1]*O[1] + g_aiT8[1][2]*O[2] + g_aiT8[1][3]*O[3] + add)>>shift;
    dst[3*line] = (g_aiT8[3][0]*O[0] + g_aiT8[3][1]*O[1] + g_aiT8[3][2]*O[2] + g_aiT8[3][3]*O[3] + add)>>shift;
    dst[5*line] = (g_aiT8[5][0]*O[0] + g_aiT8[5][1]*O[1] + g_aiT8[5][2]*O[2] + g_aiT8[5][3]*O[3] + add)>>shift;
    dst[7*line] = (g_aiT8[7][0]*O[0] + g_aiT8[7][1]*O[1] + g_aiT8[7][2]*O[2] + g_aiT8[7][3]*O[3] + add)>>shift;

    src += 8;
    dst ++;
  }
}

#if !UNIFIED_TRANSFORM
/** 8x8 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff  output data (transform coefficients)
 */
void xTr8(short block[8][8],short coeff[8][8])
{
  int shift_1st = 2; // log2(8) - 1 + g_uiBitIncrement
  int shift_2nd = 9;                    // log2(8) + 6
  short tmp[8][8]; 

  partialButterfly8(block,tmp,shift_1st);
  partialButterfly8(tmp,coeff,shift_2nd);
}

/** 8x8 inverse transform implemented using partial butterfly structure (1D)
 *  \param src   input data (transform coefficients)
 *  \param dst   output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse8(short src[8][8],short dst[8][8],int shift)
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
      O[k] = g_aiT8[ 1][k]*src[ 1][j] + g_aiT8[ 3][k]*src[ 3][j] + g_aiT8[ 5][k]*src[ 5][j] + g_aiT8[ 7][k]*src[ 7][j];
    }
   
    EO[0] = g_aiT8[2][0]*src[2][j] + g_aiT8[6][0]*src[6][j];
    EO[1] = g_aiT8[2][1]*src[2][j] + g_aiT8[6][1]*src[6][j];
    EE[0] = g_aiT8[0][0]*src[0][j] + g_aiT8[4][0]*src[4][j];
    EE[1] = g_aiT8[0][1]*src[0][j] + g_aiT8[4][1]*src[4][j];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      dst[j][k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+4] = Clip3( -32768, 32767, (E[3-k] - O[3-k] + add)>>shift );
    }        
  }
}
#endif

void partialButterflyInverse8(short *src,short *dst,int shift, int line)
{
  int j,k;    
  int E[4],O[4];
  int EE[2],EO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++) 
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<4;k++)
    {
      O[k] = g_aiT8[ 1][k]*src[line] + g_aiT8[ 3][k]*src[3*line] + g_aiT8[ 5][k]*src[5*line] + g_aiT8[ 7][k]*src[7*line];
    }

    EO[0] = g_aiT8[2][0]*src[ 2*line ] + g_aiT8[6][0]*src[ 6*line ];
    EO[1] = g_aiT8[2][1]*src[ 2*line ] + g_aiT8[6][1]*src[ 6*line ];
    EE[0] = g_aiT8[0][0]*src[ 0      ] + g_aiT8[4][0]*src[ 4*line ];
    EE[1] = g_aiT8[0][1]*src[ 0      ] + g_aiT8[4][1]*src[ 4*line ];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */ 
    E[0] = EE[0] + EO[0];
    E[3] = EE[0] - EO[0];
    E[1] = EE[1] + EO[1];
    E[2] = EE[1] - EO[1];
    for (k=0;k<4;k++)
    {
      dst[ k   ] = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[ k+4 ] = Clip3( -32768, 32767, (E[3-k] - O[3-k] + add)>>shift );
    }   
    src ++;
    dst += 8;
  }
}

#if !UNIFIED_TRANSFORM
/** 8x8 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr8(short coeff[8][8],short block[8][8])
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  short tmp[8][8];
  
  partialButterflyInverse8(coeff,tmp,shift_1st);
  partialButterflyInverse8(tmp,block,shift_2nd);
}

/** 16x16 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly16(short src[16][16],short dst[16][16],int shift)
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
      E[k] = src[j][k] + src[j][15-k];
      O[k] = src[j][k] - src[j][15-k];
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

    dst[ 0][j] = (g_aiT16[ 0][0]*EEE[0] + g_aiT16[ 0][1]*EEE[1] + add)>>shift;        
    dst[ 8][j] = (g_aiT16[ 8][0]*EEE[0] + g_aiT16[ 8][1]*EEE[1] + add)>>shift;    
    dst[ 4][j] = (g_aiT16[ 4][0]*EEO[0] + g_aiT16[ 4][1]*EEO[1] + add)>>shift;        
    dst[12][j] = (g_aiT16[12][0]*EEO[0] + g_aiT16[12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      dst[k][j] = (g_aiT16[k][0]*EO[0] + g_aiT16[k][1]*EO[1] + g_aiT16[k][2]*EO[2] + g_aiT16[k][3]*EO[3] + add)>>shift;      
    }
    
    for (k=1;k<16;k+=2)
    {
      dst[k][j] = (g_aiT16[k][0]*O[0] + g_aiT16[k][1]*O[1] + g_aiT16[k][2]*O[2] + g_aiT16[k][3]*O[3] + 
                     g_aiT16[k][4]*O[4] + g_aiT16[k][5]*O[5] + g_aiT16[k][6]*O[6] + g_aiT16[k][7]*O[7] + add)>>shift;
    }

  }
}
#endif

void partialButterfly16(short *src,short *dst,int shift, int line)
{
  int j,k;
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++) 
  {    
    /* E and O*/
    for (k=0;k<8;k++)
    {
      E[k] = src[k] + src[15-k];
      O[k] = src[k] - src[15-k];
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

    dst[ 0      ] = (g_aiT16[ 0][0]*EEE[0] + g_aiT16[ 0][1]*EEE[1] + add)>>shift;        
    dst[ 8*line ] = (g_aiT16[ 8][0]*EEE[0] + g_aiT16[ 8][1]*EEE[1] + add)>>shift;    
    dst[ 4*line ] = (g_aiT16[ 4][0]*EEO[0] + g_aiT16[ 4][1]*EEO[1] + add)>>shift;        
    dst[ 12*line] = (g_aiT16[12][0]*EEO[0] + g_aiT16[12][1]*EEO[1] + add)>>shift;

    for (k=2;k<16;k+=4)
    {
      dst[ k*line ] = (g_aiT16[k][0]*EO[0] + g_aiT16[k][1]*EO[1] + g_aiT16[k][2]*EO[2] + g_aiT16[k][3]*EO[3] + add)>>shift;      
    }

    for (k=1;k<16;k+=2)
    {
      dst[ k*line ] = (g_aiT16[k][0]*O[0] + g_aiT16[k][1]*O[1] + g_aiT16[k][2]*O[2] + g_aiT16[k][3]*O[3] + 
        g_aiT16[k][4]*O[4] + g_aiT16[k][5]*O[5] + g_aiT16[k][6]*O[6] + g_aiT16[k][7]*O[7] + add)>>shift;
    }

    src += 16;
    dst ++; 

  }
}

#if !UNIFIED_TRANSFORM
/** 16x16 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr16(short block[16][16],short coeff[16][16])
{
  int shift_1st = 3; // log2(16) - 1 + g_uiBitIncrement
  int shift_2nd = 10;                   // log2(16) + 6
  short tmp[16][16]; 

  partialButterfly16(block,tmp,shift_1st);
  partialButterfly16(tmp,coeff,shift_2nd);
}

/** 16x16 inverse transform implemented using partial butterfly structure (1D)
 *  \param src   input data (transform coefficients)
 *  \param dst   output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse16(short src[16][16],short dst[16][16],int shift)
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
      O[k] = g_aiT16[ 1][k]*src[ 1][j] + g_aiT16[ 3][k]*src[ 3][j] + g_aiT16[ 5][k]*src[ 5][j] + g_aiT16[ 7][k]*src[ 7][j] + 
             g_aiT16[ 9][k]*src[ 9][j] + g_aiT16[11][k]*src[11][j] + g_aiT16[13][k]*src[13][j] + g_aiT16[15][k]*src[15][j];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[ 2][k]*src[ 2][j] + g_aiT16[ 6][k]*src[ 6][j] + g_aiT16[10][k]*src[10][j] + g_aiT16[14][k]*src[14][j];
    }
    EEO[0] = g_aiT16[4][0]*src[4][j] + g_aiT16[12][0]*src[12][j];
    EEE[0] = g_aiT16[0][0]*src[0][j] + g_aiT16[ 8][0]*src[ 8][j];
    EEO[1] = g_aiT16[4][1]*src[4][j] + g_aiT16[12][1]*src[12][j];
    EEE[1] = g_aiT16[0][1]*src[0][j] + g_aiT16[ 8][1]*src[ 8][j];

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
      dst[j][k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+8] = Clip3( -32768, 32767, (E[7-k] - O[7-k] + add)>>shift );
    }        
  }
}
#endif

void partialButterflyInverse16(short *src,short *dst,int shift, int line)
{
  int j,k;  
  int E[8],O[8];
  int EE[4],EO[4];
  int EEE[2],EEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<8;k++)
    {
      O[k] = g_aiT16[ 1][k]*src[ line] + g_aiT16[ 3][k]*src[ 3*line] + g_aiT16[ 5][k]*src[ 5*line] + g_aiT16[ 7][k]*src[ 7*line] + 
        g_aiT16[ 9][k]*src[ 9*line] + g_aiT16[11][k]*src[11*line] + g_aiT16[13][k]*src[13*line] + g_aiT16[15][k]*src[15*line];
    }
    for (k=0;k<4;k++)
    {
      EO[k] = g_aiT16[ 2][k]*src[ 2*line] + g_aiT16[ 6][k]*src[ 6*line] + g_aiT16[10][k]*src[10*line] + g_aiT16[14][k]*src[14*line];
    }
    EEO[0] = g_aiT16[4][0]*src[ 4*line ] + g_aiT16[12][0]*src[ 12*line ];
    EEE[0] = g_aiT16[0][0]*src[ 0      ] + g_aiT16[ 8][0]*src[ 8*line  ];
    EEO[1] = g_aiT16[4][1]*src[ 4*line ] + g_aiT16[12][1]*src[ 12*line ];
    EEE[1] = g_aiT16[0][1]*src[ 0      ] + g_aiT16[ 8][1]*src[ 8*line  ];

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
      dst[k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[k+8] = Clip3( -32768, 32767, (E[7-k] - O[7-k] + add)>>shift );
    }   
    src ++; 
    dst += 16;
  }
}

#if !UNIFIED_TRANSFORM
/** 16x16 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr16(short coeff[16][16],short block[16][16])
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  short tmp[16][16];
  
  partialButterflyInverse16(coeff,tmp,shift_1st);
  partialButterflyInverse16(tmp,block,shift_2nd);
}

/** 32x32 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterfly32(short src[32][32],short dst[32][32],int shift)
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
      E[k] = src[j][k] + src[j][31-k];
      O[k] = src[j][k] - src[j][31-k];
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

    dst[ 0][j] = (g_aiT32[ 0][0]*EEEE[0] + g_aiT32[ 0][1]*EEEE[1] + add)>>shift;
    dst[16][j] = (g_aiT32[16][0]*EEEE[0] + g_aiT32[16][1]*EEEE[1] + add)>>shift;
    dst[ 8][j] = (g_aiT32[ 8][0]*EEEO[0] + g_aiT32[ 8][1]*EEEO[1] + add)>>shift; 
    dst[24][j] = (g_aiT32[24][0]*EEEO[0] + g_aiT32[24][1]*EEEO[1] + add)>>shift;
    for (k=4;k<32;k+=8)
    {
      dst[k][j] = (g_aiT32[k][0]*EEO[0] + g_aiT32[k][1]*EEO[1] + g_aiT32[k][2]*EEO[2] + g_aiT32[k][3]*EEO[3] + add)>>shift;
    }       
    for (k=2;k<32;k+=4)
    {
      dst[k][j] = (g_aiT32[k][0]*EO[0] + g_aiT32[k][1]*EO[1] + g_aiT32[k][2]*EO[2] + g_aiT32[k][3]*EO[3] + 
                     g_aiT32[k][4]*EO[4] + g_aiT32[k][5]*EO[5] + g_aiT32[k][6]*EO[6] + g_aiT32[k][7]*EO[7] + add)>>shift;
    }       
    for (k=1;k<32;k+=2)
    {
      dst[k][j] = (g_aiT32[k][ 0]*O[ 0] + g_aiT32[k][ 1]*O[ 1] + g_aiT32[k][ 2]*O[ 2] + g_aiT32[k][ 3]*O[ 3] + 
                     g_aiT32[k][ 4]*O[ 4] + g_aiT32[k][ 5]*O[ 5] + g_aiT32[k][ 6]*O[ 6] + g_aiT32[k][ 7]*O[ 7] +
                     g_aiT32[k][ 8]*O[ 8] + g_aiT32[k][ 9]*O[ 9] + g_aiT32[k][10]*O[10] + g_aiT32[k][11]*O[11] + 
                     g_aiT32[k][12]*O[12] + g_aiT32[k][13]*O[13] + g_aiT32[k][14]*O[14] + g_aiT32[k][15]*O[15] + add)>>shift;
    }
  }
}
#endif

void partialButterfly32(short *src,short *dst,int shift, int line)
{
  int j,k;
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {    
    /* E and O*/
    for (k=0;k<16;k++)
    {
      E[k] = src[k] + src[31-k];
      O[k] = src[k] - src[31-k];
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

    dst[ 0       ] = (g_aiT32[ 0][0]*EEEE[0] + g_aiT32[ 0][1]*EEEE[1] + add)>>shift;
    dst[ 16*line ] = (g_aiT32[16][0]*EEEE[0] + g_aiT32[16][1]*EEEE[1] + add)>>shift;
    dst[ 8*line  ] = (g_aiT32[ 8][0]*EEEO[0] + g_aiT32[ 8][1]*EEEO[1] + add)>>shift; 
    dst[ 24*line ] = (g_aiT32[24][0]*EEEO[0] + g_aiT32[24][1]*EEEO[1] + add)>>shift;
    for (k=4;k<32;k+=8)
    {
      dst[ k*line ] = (g_aiT32[k][0]*EEO[0] + g_aiT32[k][1]*EEO[1] + g_aiT32[k][2]*EEO[2] + g_aiT32[k][3]*EEO[3] + add)>>shift;
    }       
    for (k=2;k<32;k+=4)
    {
      dst[ k*line ] = (g_aiT32[k][0]*EO[0] + g_aiT32[k][1]*EO[1] + g_aiT32[k][2]*EO[2] + g_aiT32[k][3]*EO[3] + 
        g_aiT32[k][4]*EO[4] + g_aiT32[k][5]*EO[5] + g_aiT32[k][6]*EO[6] + g_aiT32[k][7]*EO[7] + add)>>shift;
    }       
    for (k=1;k<32;k+=2)
    {
      dst[ k*line ] = (g_aiT32[k][ 0]*O[ 0] + g_aiT32[k][ 1]*O[ 1] + g_aiT32[k][ 2]*O[ 2] + g_aiT32[k][ 3]*O[ 3] + 
        g_aiT32[k][ 4]*O[ 4] + g_aiT32[k][ 5]*O[ 5] + g_aiT32[k][ 6]*O[ 6] + g_aiT32[k][ 7]*O[ 7] +
        g_aiT32[k][ 8]*O[ 8] + g_aiT32[k][ 9]*O[ 9] + g_aiT32[k][10]*O[10] + g_aiT32[k][11]*O[11] + 
        g_aiT32[k][12]*O[12] + g_aiT32[k][13]*O[13] + g_aiT32[k][14]*O[14] + g_aiT32[k][15]*O[15] + add)>>shift;
    }
    src += 32;
    dst ++;
  }
}

#if !UNIFIED_TRANSFORM
/** 32x32 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr32(short block[32][32],short coeff[32][32])
{
  int shift_1st = 4; // log2(32) - 1 + g_uiBitIncrement
  int shift_2nd = 11;                   // log2(32) + 6
  short tmp[32][32]; 

  partialButterfly32(block,tmp,shift_1st);
  partialButterfly32(tmp,coeff,shift_2nd);
}

/** 32x32 inverse transform implemented using partial butterfly structure (1D)
 *  \param src   input data (transform coefficients)
 *  \param dst   output data (residual)
 *  \param shift specifies right shift after 1D transform
 */
void partialButterflyInverse32(short src[32][32],short dst[32][32],int shift)
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
      O[k] = g_aiT32[ 1][k]*src[ 1][j] + g_aiT32[ 3][k]*src[ 3][j] + g_aiT32[ 5][k]*src[ 5][j] + g_aiT32[ 7][k]*src[ 7][j] + 
             g_aiT32[ 9][k]*src[ 9][j] + g_aiT32[11][k]*src[11][j] + g_aiT32[13][k]*src[13][j] + g_aiT32[15][k]*src[15][j] + 
             g_aiT32[17][k]*src[17][j] + g_aiT32[19][k]*src[19][j] + g_aiT32[21][k]*src[21][j] + g_aiT32[23][k]*src[23][j] + 
             g_aiT32[25][k]*src[25][j] + g_aiT32[27][k]*src[27][j] + g_aiT32[29][k]*src[29][j] + g_aiT32[31][k]*src[31][j];
    }
    for (k=0;k<8;k++)
    {
      EO[k] = g_aiT32[ 2][k]*src[ 2][j] + g_aiT32[ 6][k]*src[ 6][j] + g_aiT32[10][k]*src[10][j] + g_aiT32[14][k]*src[14][j] + 
              g_aiT32[18][k]*src[18][j] + g_aiT32[22][k]*src[22][j] + g_aiT32[26][k]*src[26][j] + g_aiT32[30][k]*src[30][j];
    }
    for (k=0;k<4;k++)
    {
      EEO[k] = g_aiT32[4][k]*src[4][j] + g_aiT32[12][k]*src[12][j] + g_aiT32[20][k]*src[20][j] + g_aiT32[28][k]*src[28][j];
    }
    EEEO[0] = g_aiT32[8][0]*src[8][j] + g_aiT32[24][0]*src[24][j];
    EEEO[1] = g_aiT32[8][1]*src[8][j] + g_aiT32[24][1]*src[24][j];
    EEEE[0] = g_aiT32[0][0]*src[0][j] + g_aiT32[16][0]*src[16][j];    
    EEEE[1] = g_aiT32[0][1]*src[0][j] + g_aiT32[16][1]*src[16][j];

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
      dst[j][k]    = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+16] = Clip3( -32768, 32767, (E[15-k] - O[15-k] + add)>>shift );
    }        
  }
}
#endif

void partialButterflyInverse32(short *src,short *dst,int shift, int line)
{
  int j,k;  
  int E[16],O[16];
  int EE[8],EO[8];
  int EEE[4],EEO[4];
  int EEEE[2],EEEO[2];
  int add = 1<<(shift-1);

  for (j=0; j<line; j++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for (k=0;k<16;k++)
    {
      O[k] = g_aiT32[ 1][k]*src[ line  ] + g_aiT32[ 3][k]*src[ 3*line  ] + g_aiT32[ 5][k]*src[ 5*line  ] + g_aiT32[ 7][k]*src[ 7*line  ] + 
        g_aiT32[ 9][k]*src[ 9*line  ] + g_aiT32[11][k]*src[ 11*line ] + g_aiT32[13][k]*src[ 13*line ] + g_aiT32[15][k]*src[ 15*line ] + 
        g_aiT32[17][k]*src[ 17*line ] + g_aiT32[19][k]*src[ 19*line ] + g_aiT32[21][k]*src[ 21*line ] + g_aiT32[23][k]*src[ 23*line ] + 
        g_aiT32[25][k]*src[ 25*line ] + g_aiT32[27][k]*src[ 27*line ] + g_aiT32[29][k]*src[ 29*line ] + g_aiT32[31][k]*src[ 31*line ];
    }
    for (k=0;k<8;k++)
    {
      EO[k] = g_aiT32[ 2][k]*src[ 2*line  ] + g_aiT32[ 6][k]*src[ 6*line  ] + g_aiT32[10][k]*src[ 10*line ] + g_aiT32[14][k]*src[ 14*line ] + 
        g_aiT32[18][k]*src[ 18*line ] + g_aiT32[22][k]*src[ 22*line ] + g_aiT32[26][k]*src[ 26*line ] + g_aiT32[30][k]*src[ 30*line ];
    }
    for (k=0;k<4;k++)
    {
      EEO[k] = g_aiT32[4][k]*src[ 4*line ] + g_aiT32[12][k]*src[ 12*line ] + g_aiT32[20][k]*src[ 20*line ] + g_aiT32[28][k]*src[ 28*line ];
    }
    EEEO[0] = g_aiT32[8][0]*src[ 8*line ] + g_aiT32[24][0]*src[ 24*line ];
    EEEO[1] = g_aiT32[8][1]*src[ 8*line ] + g_aiT32[24][1]*src[ 24*line ];
    EEEE[0] = g_aiT32[0][0]*src[ 0      ] + g_aiT32[16][0]*src[ 16*line ];    
    EEEE[1] = g_aiT32[0][1]*src[ 0      ] + g_aiT32[16][1]*src[ 16*line ];

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
      dst[k]    = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[k+16] = Clip3( -32768, 32767, (E[15-k] - O[15-k] + add)>>shift );
    }
    src ++;
    dst += 32;
  }
}

#if !UNIFIED_TRANSFORM
/** 32x32 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 */
void xITr32(short coeff[32][32],short block[32][32])
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;
  short tmp[32][32];
  
  partialButterflyInverse32(coeff,tmp,shift_1st);
  partialButterflyInverse32(tmp,block,shift_2nd);
}
#endif

/** MxN forward transform (2D)
*  \param block input data (residual)
*  \param coeff output data (transform coefficients)
*  \param iWidth input data (width of transform)
*  \param iHeight input data (height of transform)
*/
#if UNIFIED_TRANSFORM
void xTrMxN(short *block,short *coeff, int iWidth, int iHeight, UInt uiMode)
#else
void xTrMxN(short *block,short *coeff, int iWidth, int iHeight)
#endif
{
  int shift_1st = g_aucConvertToBit[iWidth]  + 1; // log2(iWidth) - 1 + g_uiBitIncrement
  int shift_2nd = g_aucConvertToBit[iHeight]  + 8;                   // log2(iHeight) + 6

  short tmp[ 64 * 64 ];

  if( iWidth == 16 && iHeight == 4)
  {
    partialButterfly16( block, tmp, shift_1st, iHeight );
    partialButterfly4( tmp, coeff, shift_2nd, iWidth );
  }
  else if( iWidth == 32 && iHeight == 8 )
  {
    partialButterfly32( block, tmp, shift_1st, iHeight );
    partialButterfly8( tmp, coeff, shift_2nd, iWidth );
  }
  else if( iWidth == 4 && iHeight == 16)
  {
    partialButterfly4( block, tmp, shift_1st, iHeight );
    partialButterfly16( tmp, coeff, shift_2nd, iWidth );
  }
  else if( iWidth == 8 && iHeight == 32 )
  {
    partialButterfly8( block, tmp, shift_1st, iHeight );
    partialButterfly32( tmp, coeff, shift_2nd, iWidth );
  }
#if UNIFIED_TRANSFORM
  else if( iWidth == 4 && iHeight == 4)
  {
#if LOGI_INTRA_NAME_3MPM
    if (uiMode != REG_DCT && (!uiMode || (uiMode>=2 && uiMode <= 25)))    // Check for DCT or DST
#else
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])// Check for DCT or DST
#endif
    {
      fastForwardDst(block,tmp,shift_1st); // Forward DST BY FAST ALGORITHM, block input, tmp output
    }
    else  
    {
      partialButterfly4(block, tmp, shift_1st, iHeight);
    }
#if LOGI_INTRA_NAME_3MPM
    if (uiMode != REG_DCT && (!uiMode || (uiMode>=11 && uiMode <= 34)))    // Check for DCT or DST
#else
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )   // Check for DCT or DST
#endif
    {
      fastForwardDst(tmp,coeff,shift_2nd); // Forward DST BY FAST ALGORITHM, tmp input, coeff output
    }
    else  
    {
      partialButterfly4(tmp, coeff, shift_2nd, iWidth);
    }   
  }
  else if( iWidth == 8 && iHeight == 8)
  {
    partialButterfly8( block, tmp, shift_1st, iHeight );
    partialButterfly8( tmp, coeff, shift_2nd, iWidth );
  }
  else if( iWidth == 16 && iHeight == 16)
  {
    partialButterfly16( block, tmp, shift_1st, iHeight );
    partialButterfly16( tmp, coeff, shift_2nd, iWidth );
  }
  else if( iWidth == 32 && iHeight == 32)
  {
    partialButterfly32( block, tmp, shift_1st, iHeight );
    partialButterfly32( tmp, coeff, shift_2nd, iWidth );
  }
#endif
}
/** MxN inverse transform (2D)
*  \param coeff input data (transform coefficients)
*  \param block output data (residual)
*  \param iWidth input data (width of transform)
*  \param iHeight input data (height of transform)
*/
#if UNIFIED_TRANSFORM
void xITrMxN(short *coeff,short *block, int iWidth, int iHeight, UInt uiMode)
#else
void xITrMxN(short *coeff,short *block, int iWidth, int iHeight)
#endif
{
  int shift_1st = SHIFT_INV_1ST;
  int shift_2nd = SHIFT_INV_2ND;

  short tmp[ 64*64];
  if( iWidth == 16 && iHeight == 4)
  {
    partialButterflyInverse4(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse16(tmp,block,shift_2nd,iHeight);
  }
  else if( iWidth == 32 && iHeight == 8)
  {
    partialButterflyInverse8(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse32(tmp,block,shift_2nd,iHeight);
  }
  else if( iWidth == 4 && iHeight == 16)
  {
    partialButterflyInverse16(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse4(tmp,block,shift_2nd,iHeight);
  }
  else if( iWidth == 8 && iHeight == 32)
  {
    partialButterflyInverse32(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse8(tmp,block,shift_2nd,iHeight);
  }
#if UNIFIED_TRANSFORM
  else if( iWidth == 4 && iHeight == 4)
  {
#if LOGI_INTRA_NAME_3MPM
    if (uiMode != REG_DCT && (!uiMode || (uiMode>=11 && uiMode <= 34)))    // Check for DCT or DST
#else
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )    // Check for DCT or DST
#endif
    {
      fastInverseDst(coeff,tmp,shift_1st);    // Inverse DST by FAST Algorithm, coeff input, tmp output
    }
    else
    {
      partialButterflyInverse4(coeff,tmp,shift_1st,iWidth);    
    } 
#if LOGI_INTRA_NAME_3MPM
    if (uiMode != REG_DCT && (!uiMode || (uiMode>=2 && uiMode <= 25)))    // Check for DCT or DST
#else
    if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode] )    // Check for DCT or DST
#endif
    {
      fastInverseDst(tmp,block,shift_2nd); // Inverse DST by FAST Algorithm, tmp input, coeff output
    }
    else
    {
      partialButterflyInverse4(tmp,block,shift_2nd,iHeight);
    }   
  }
  else if( iWidth == 8 && iHeight == 8)
  {
    partialButterflyInverse8(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse8(tmp,block,shift_2nd,iHeight);
  }
  else if( iWidth == 16 && iHeight == 16)
  {
    partialButterflyInverse16(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse16(tmp,block,shift_2nd,iHeight);
  }
  else if( iWidth == 32 && iHeight == 32)
  {
    partialButterflyInverse32(coeff,tmp,shift_1st,iWidth);
    partialButterflyInverse32(tmp,block,shift_2nd,iHeight);
  }
#endif
}

#endif //MATRIX_MULT

#if MULTIBITS_DATA_HIDING
// To minimize the distortion only. No rate is considered. 
Void TComTrQuant::signBitHidingHDQ( TComDataCU* pcCU, TCoeff* pQCoef, TCoeff* pCoef, UInt const *scan, Int* deltaU, Int width, Int height )
{
  Int tsig = pcCU->getSlice()->getPPS()->getTSIG() ;
  Int lastCG = -1;
  Int absSum = 0 ;
  Int n ;

  for( Int subSet = (width*height-1) >> LOG2_SCAN_SET_SIZE; subSet >= 0; subSet-- )
  {
    Int  subPos     = subSet << LOG2_SCAN_SET_SIZE;
    Int  firstNZPosInCG=SCAN_SET_SIZE , lastNZPosInCG=-1 ;
    absSum = 0 ;

    for(n = SCAN_SET_SIZE-1; n >= 0; --n )
    {
      if( pQCoef[ scan[ n + subPos ]] )
      {
        lastNZPosInCG = n;
        break;
      }
    }

    for(n = 0; n <SCAN_SET_SIZE; n++ )
    {
      if( pQCoef[ scan[ n + subPos ]] )
      {
        firstNZPosInCG = n;
        break;
      }
    }

    for(n = firstNZPosInCG; n <=lastNZPosInCG; n++ )
    {
      absSum += pQCoef[ scan[ n + subPos ]];
    }

    if(lastNZPosInCG>=0 && lastCG==-1) 
    {
      lastCG = 1 ; 
    }
    
    if( lastNZPosInCG-firstNZPosInCG>=tsig )
    {
      UInt signbit = (pQCoef[scan[subPos+firstNZPosInCG]]>0?0:1) ;
      if( signbit!=(absSum&0x1) )  //compare signbit with sum_parity
      {
        Int minCostInc = MAX_INT,  minPos =-1, finalChange=0, curCost=MAX_INT, curChange=0;
        
        for( n = (lastCG==1?lastNZPosInCG:SCAN_SET_SIZE-1) ; n >= 0; --n )
        {
          UInt blkPos   = scan[ n+subPos ];
          if(pQCoef[ blkPos ] != 0 )
          {
            if(deltaU[blkPos]>0)
            {
              curCost = - deltaU[blkPos]; 
              curChange=1 ;
            }
            else 
            {
              //curChange =-1;
              if(n==firstNZPosInCG && abs(pQCoef[blkPos])==1)
              {
                curCost=MAX_INT ; 
              }
              else
              {
                curCost = deltaU[blkPos]; 
                curChange =-1;
              }
            }
          }
          else
          {
            if(n<firstNZPosInCG)
            {
              UInt thisSignBit = (pCoef[blkPos]>=0?0:1);
              if(thisSignBit != signbit )
              {
                curCost = MAX_INT;
              }
              else
              { 
                curCost = - (deltaU[blkPos])  ;
                curChange = 1 ;
              }
            }
            else
            {
              curCost = - (deltaU[blkPos])  ;
              curChange = 1 ;
            }
          }

          if( curCost<minCostInc)
          {
            minCostInc = curCost ;
            finalChange = curChange ;
            minPos = blkPos ;
          }
        } //CG loop

        if(pQCoef[minPos] == 32767 || pQCoef[minPos] == -32768)
        {
          finalChange = -1;
        }

        if(pCoef[minPos]>=0)
        {
          pQCoef[minPos] += finalChange ; 
        }
        else 
        { 
          pQCoef[minPos] -= finalChange ;
        }  
      } // Hide
    }
    if(lastCG==1) 
    {
      lastCG=0 ;
    }
  } // TU loop

  return;
}
#endif

Void TComTrQuant::xQuant( TComDataCU* pcCU, 
                          Int*        pSrc, 
                          TCoeff*     pDes, 
                          Int         iWidth, 
                          Int         iHeight, 
                          UInt&       uiAcSum, 
                          TextType    eTType, 
                          UInt        uiAbsPartIdx )
{
  Int*   piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
  Int   iAdd = 0;
  
#if MULTIBITS_DATA_HIDING
    const UInt   log2BlockSize   = g_aucConvertToBit[ iWidth ] + 2;

    UInt scanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, iWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
    if (scanIdx == SCAN_ZIGZAG)
    {
      scanIdx = SCAN_DIAG;
    }

    if (iWidth != iHeight)
    {
      scanIdx = SCAN_DIAG;
    }

    const UInt * scan;
    if (iWidth == iHeight)
    {
      scan = g_auiSigLastScan[ scanIdx ][ log2BlockSize - 1 ];
    }
    else
    {
      scan = g_sigScanNSQT[ log2BlockSize - 2 ];
    }

    Int deltaU[32*32] ;
#endif

    Bool bNonSqureFlag = ( iWidth != iHeight );
    if( bNonSqureFlag )
    {
      UInt uiWidthBit  = g_aucConvertToBit[ iWidth ] + 2;
      UInt uiHeightBit = g_aucConvertToBit[ iHeight ] + 2;
      iWidth  = 1 << ( ( uiWidthBit + uiHeightBit) >> 1 );
      iHeight = iWidth;
    }    

    UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;
    UInt uiQ = g_quantScales[m_cQP.rem()];

    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

    iAdd = (pcCU->getSlice()->getSliceType()==I_SLICE ? 171 : 85) << (iQBits-9);

#if MULTIBITS_DATA_HIDING
    Int qBits8 = iQBits-8;
#endif
    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
      Int iLevel;
      Int  iSign;
      UInt uiBlockPos = n;
      iLevel  = piCoef[uiBlockPos];
      iSign   = (iLevel < 0 ? -1: 1);      

      Int64 tmpLevel = (Int64)abs(iLevel) * uiQ;
      iLevel = (Int)((tmpLevel + iAdd ) >> iQBits);
#if MULTIBITS_DATA_HIDING
      deltaU[uiBlockPos] = (Int)((tmpLevel - (iLevel<<iQBits) )>> qBits8);
#endif
      uiAcSum += iLevel;
      iLevel *= iSign;        
      piQCoef[uiBlockPos] = Clip3( -32768, 32767, iLevel );
    } // for n
#if MULTIBITS_DATA_HIDING
    if( pcCU->getSlice()->getPPS()->getSignHideFlag() )
    {
      if(uiAcSum>=2)
      {
        signBitHidingHDQ( pcCU, piQCoef, piCoef, scan, deltaU, iWidth, iHeight ) ;
      }
    }
#endif
}

Void TComTrQuant::xDeQuant( const TCoeff* pSrc, Int* pDes, Int iWidth, Int iHeight )
{
  
  const TCoeff* piQCoef   = pSrc;
  Int*   piCoef    = pDes;
  if( iWidth != iHeight )
  {
    UInt uiWidthBit  = g_aucConvertToBit[ iWidth ]  + 2;
    UInt uiHeightBit = g_aucConvertToBit[ iHeight ] + 2;
    iWidth  = 1 << ( ( uiWidthBit + uiHeightBit) >> 1 );
    iHeight = iWidth;
  }    

  if ( iWidth > (Int)m_uiMaxTrSize )
  {
    iWidth  = m_uiMaxTrSize;
    iHeight = m_uiMaxTrSize;
  }
  
  Int iShift,iAdd,iCoeffQ;
  UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;

  UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize; 
  iShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
  iAdd = 1 << (iShift-1);
  Int scale = g_invQuantScales[m_cQP.m_iRem] << m_cQP.m_iPer;

#if DEQUANT_CLIPPING
  TCoeff clipQCoef;
#endif

    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
#if DEQUANT_CLIPPING
      clipQCoef = Clip3( -32768, 32767, piQCoef[n] );
      iCoeffQ = ( clipQCoef * scale + iAdd ) >> iShift;
#else
      iCoeffQ = ( piQCoef[n] * scale + iAdd ) >> iShift;
#endif
      piCoef[n] = Clip3(-32768,32767,iCoeffQ);
    }
}

Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, UInt *aTableLP4, UInt *aTableLP8, UInt *aTableLastPosVlcIndex,
                        Bool bEnc
                       )
{
  m_uiMaxTrSize  = uiMaxTrSize;
  m_bEnc         = bEnc;
}

Void TComTrQuant::transformNxN( TComDataCU* pcCU, 
                                Pel*        pcResidual, 
                                UInt        uiStride, 
                                TCoeff*     rpcCoeff, 
                                UInt        uiWidth, 
                                UInt        uiHeight, 
                                UInt&       uiAbsSum, 
                                TextType    eTType, 
                                UInt        uiAbsPartIdx )
{
  UInt uiMode;  //luma intra pred
  if(eTType == TEXT_LUMA && pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
  {
    uiMode = pcCU->getLumaIntraDir( uiAbsPartIdx );
  }
  else
  {
    uiMode = REG_DCT;
  }
  
  uiAbsSum = 0;
  assert( (pcCU->getSlice()->getSPS()->getMaxTrSize() >= uiWidth) );

  xT( uiMode, pcResidual, uiStride, m_plTempCoeff, uiWidth, uiHeight );
#if (CHEN_TV)
  if ( do_print ) {
      if ( uiMode != REG_DCT ) {
        tPrintMatrix32( fp_tv, "--- Trans ---\n", m_plTempCoeff, uiWidth, uiWidth);
      }
  }
#endif
  xQuant( pcCU, m_plTempCoeff, rpcCoeff,
       uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
#if (CHEN_TV)
  if ( do_print ) {
      if ( uiMode != REG_DCT ) {
        tPrintMatrix32( fp_tv, "--- Quant ---\n", rpcCoeff, uiWidth, uiWidth);
      }
  }
#endif
}


Void TComTrQuant::invtransformNxN( TextType eText,UInt uiMode, Pel* rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight);
#if (CHEN_TV)
  if ( do_print ) {
    if ( eText == TEXT_LUMA ) {
      tPrintMatrix32( fp_tv, "--- IQuant ---\n", m_plTempCoeff, uiWidth, uiWidth);
    }
  }
#endif
  xIT( uiMode, m_plTempCoeff, rpcResidual, uiStride, uiWidth, uiHeight );
#if (CHEN_TV)
  if ( do_print ) {
    if ( eText == TEXT_LUMA ) {
      tPrintMatrix16( fp_tv, "--- ITrans ---\n", rpcResidual, uiStride, uiWidth);
    }
  }
#endif
}

// ------------------------------------------------------------------------------------------------
// Logical transform
// ------------------------------------------------------------------------------------------------

/** Wrapper function between HM interface and core NxN forward transform (2D) 
 *  \param piBlkResi input data (residual)
 *  \param psCoeff output data (transform coefficients)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
Void TComTrQuant::xT( UInt uiMode, Pel* piBlkResi, UInt uiStride, Int* psCoeff, Int iWidth, Int iHeight )
{
#if MATRIX_MULT  
  Int iSize = iWidth; 
  if( iWidth != iHeight)
  {
    xTrMxN( piBlkResi, psCoeff, uiStride, (UInt)iWidth, (UInt)iHeight );
    return;
  }
  xTr(piBlkResi,psCoeff,uiStride,(UInt)iSize,uiMode);
#else
#if UNIFIED_TRANSFORM
  Int j;
#else
  Int iSize = iWidth; 
  if( iWidth != iHeight)
#endif
  {
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
    {
      for (j = 0; j < iHeight; j++)
      {    
        memcpy( block + j * iWidth, piBlkResi + j * uiStride, iWidth * sizeof( short ) );      
      }
    }
#if UNIFIED_TRANSFORM
    xTrMxN( block, coeff, iWidth, iHeight, uiMode );
#else
    xTrMxN( block, coeff, iWidth, iHeight );
#endif
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      psCoeff[ j ] = coeff[ j ];
    }
    return ;
  }
#if !UNIFIED_TRANSFORM
  if (iSize==4)
  {   
    short block[4][4];   
    short coeff[4][4];
    for (j=0; j<4; j++)
    {    
      memcpy(block[j],piBlkResi+j*uiStride,4*sizeof(short));      
    }
    xTr4(block,coeff,uiMode);
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
#endif  
}

/** Wrapper function between HM interface and core NxN inverse transform (2D) 
 *  \param plCoef input data (transform coefficients)
 *  \param pResidual output data (residual)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
Void TComTrQuant::xIT( UInt uiMode, Int* plCoef, Pel* pResidual, UInt uiStride, Int iWidth, Int iHeight )
{
#if MATRIX_MULT  
  Int iSize = iWidth;
  if( iWidth != iHeight )
  {
    xITrMxN( plCoef, pResidual, uiStride, (UInt)iWidth, (UInt)iHeight );
    return;
  }
  xITr(plCoef,pResidual,uiStride,(UInt)iSize,uiMode);
#else
#if UNIFIED_TRANSFORM
  Int j;
#else
  Int j,k;
  Int iSize = iWidth; 
  if( iWidth != iHeight )
#endif
  {
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      coeff[j] = (short)plCoef[j];
    }
#if UNIFIED_TRANSFORM
    xITrMxN( coeff, block, iWidth, iHeight, uiMode );
#else
    xITrMxN( coeff, block, iWidth, iHeight );
#endif
    {
      for ( j = 0; j < iHeight; j++ )
      {    
        memcpy( pResidual + j * uiStride, block + j * iWidth, iWidth * sizeof(short) );      
      }
    }
    return ;
  }
#if !UNIFIED_TRANSFORM
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
    xITr4(coeff,block,uiMode);
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
#endif  
}

/** Context derivation process of coeff_abs_significant_flag
 * \param pcCoeff pointer to prior coded transform coefficients
 * \param posX column of current scan position
 * \param posY row of current scan position
 * \param blockType log2 value of block size if square block, or 4 otherwise
 * \param width width of the block
 * \param height height of the block
 * \param textureType texture type (TEXT_LUMA...)
 * \returns ctxInc for current scan position
 */
Int TComTrQuant::getSigCtxInc    ( TCoeff*                         pcCoeff,
                                   Int                             posX,
                                   Int                             posY,
                                   Int                             blockType,
                                   Int                             width
                                  ,Int                             height
                                  ,TextType                        textureType
                                  )
{
  if ( blockType == 2 )
  {
    //LUMA map
    const Int ctxIndMap4x4Luma[15] =
    {
      0, 1, 4, 5,
      2, 3, 4, 5,
      6, 6, 8, 8,
      7, 7, 8
    };
    //CHROMA map
    const Int ctxIndMap4x4Chroma[15] =
    {
      0, 1, 2, 4,
      1, 1, 2, 4,
      3, 3, 5, 5,
      4, 4, 5
    };

    if (textureType == TEXT_LUMA)
    {
      return ctxIndMap4x4Luma[ 4 * posY + posX ];
    }
    else
    {
      return ctxIndMap4x4Chroma[ 4 * posY + posX ];
    }
  }
  
  if ( blockType == 3 )
  {
    const Int map8x8[16] =
    {
      0,  1,  2,  3,
      4,  5,  6,  3,
      8,  6,  6,  7,
      9,  9,  7,  7
    };
    
    Int offset = (textureType == TEXT_LUMA) ? 9 : 6;

    if ( posX + posY == 0 )
    {
      return offset + 10;
    }
    return offset + map8x8[4 * (posY >> 1) + (posX >> 1)];
  }

  Int offset = (textureType == TEXT_LUMA) ? 20 : 17;
  if( posX + posY == 0 )
  {
    return offset;
  }
#if SIGMAP_CONST_AT_HIGH_FREQUENCY
  Int thredHighFreq = 3*(std::max(width, height)>>4);
  if ((posX>>2) + (posY>>2) >= thredHighFreq)
  {
    return (textureType == TEXT_LUMA) ? 24 : 18;
  }
#endif
  
  const TCoeff *pData = pcCoeff + posX + posY * width;
  
#if !SIGMAP_CTX_SUBBLOCK
  Int thred = std::max(height, width) >> 2;
#endif
  
  Int cnt = 0;
  if( posX < width - 1 )
  {
    cnt += pData[1] != 0;
    if( posY < height - 1 )
    {
      cnt += pData[width+1] != 0;
    }
    if( posX < width - 2 )
    {
      cnt += pData[2] != 0;
    }
  }
  if ( posY < height - 1 )
  {
    if( ( ( posX & 3 ) || ( posY & 3 ) ) && ( ( (posX+1) & 3 ) || ( (posY+2) & 3 ) ) )
    {
      cnt += pData[width] != 0;
    }
    if ( posY < height - 2 && cnt < 4 )
    {
      cnt += pData[2*width] != 0;
    }
  }

  cnt = ( cnt + 1 ) >> 1;
#if SIGMAP_CTX_SUBBLOCK
  return (( textureType == TEXT_LUMA && ((posX>>2) + (posY>>2)) > 0 ) ? 4 : 1) + offset + cnt;
#else
  return (( textureType == TEXT_LUMA && posX + posY >= thred ) ? 4 : 1) + offset + cnt;
#endif
}

/** Context derivation process of coeff_abs_significant_flag
 * \param uiSigCoeffGroupFlag significance map of L1
 * \param uiBlkX column of current scan position
 * \param uiBlkY row of current scan position
 * \param uiLog2BlkSize log2 value of block size
 * \returns ctxInc for current scan position
 */
UInt TComTrQuant::getSigCoeffGroupCtxInc  ( const UInt*               uiSigCoeffGroupFlag,
                                           const UInt                      uiCGPosX,
                                           const UInt                      uiCGPosY,
#if MULTILEVEL_SIGMAP_EXT
                                           const UInt                      scanIdx,
#endif
                                           Int width, Int height)
{
  UInt uiRight = 0;
  UInt uiLower = 0;

  width >>= 2;
  height >>= 2;
#if MULTILEVEL_SIGMAP_EXT
  if( width == 2 && height == 2 ) // 8x8
  {
    if( scanIdx == SCAN_HOR )  
    {
      width = 1;
      height = 4;
    }
    else if( scanIdx == SCAN_VER )
    {
      width = 4;
      height = 1;
    }
  }
#endif
  if( uiCGPosX < width - 1 )
  {
    uiRight = (uiSigCoeffGroupFlag[ uiCGPosY * width + uiCGPosX + 1 ] != 0);
  }
  if (uiCGPosY < height - 1 )
  {
    uiLower = (uiSigCoeffGroupFlag[ (uiCGPosY  + 1 ) * width + uiCGPosX ] != 0);
  }
#if REMOVE_INFER_SIGGRP
  return (uiRight || uiLower);
#else
  return uiRight + uiLower;
#endif

}
#if !REMOVE_INFER_SIGGRP
// return 1 if both right neighbour and lower neighour are 1's
Bool TComTrQuant::bothCGNeighboursOne ( const UInt*                   uiSigCoeffGroupFlag,
                                       const UInt                      uiCGPosX,
                                       const UInt                      uiCGPosY, 
#if MULTILEVEL_SIGMAP_EXT
                                       const UInt                      scanIdx,
#endif
                                       Int width, Int height)
{
  UInt uiRight = 0;
  UInt uiLower = 0;

  width >>= 2;
  height >>= 2;
#if MULTILEVEL_SIGMAP_EXT
  if( width == 2 && height == 2 ) // 8x8
  {
    if( scanIdx == SCAN_HOR )  
    {
      width = 1;
      height = 4;
    }
    else if( scanIdx == SCAN_VER )
    {
      width = 4;
      height = 1;
    }
  }
#endif
  if( uiCGPosX < width - 1 )
  {
    uiRight = (uiSigCoeffGroupFlag[ uiCGPosY * width + uiCGPosX + 1 ] != 0);
  }
  if (uiCGPosY < height - 1 )
  {
    uiLower = (uiSigCoeffGroupFlag[ (uiCGPosY  + 1 ) * width + uiCGPosX ] != 0);
  }
  
  return (uiRight & uiLower);
}
#endif

//! \}
