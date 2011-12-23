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

#if MULTI_LEVEL_SIGNIFICANCE
typedef struct
{
  Int    iNNZbeforePos0;
  Double d64CodedLevelandDist; // distortion and level cost only
  Double d64UncodedDist;    // all zero coded block distortion
  Double d64SigCost;
  Double d64SigCost_0;
} coeffGroupRDStats;
#endif

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_CHROMA                 1           ///< use of RDOQ in chroma

// ====================================================================================================================
// Tables
// ====================================================================================================================

// RDOQ parameter

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
  if ( m_pcEstBitsSbac )
  {
    delete m_pcEstBitsSbac;
  }
}

/// Including Chroma QP Parameter setting
Void TComTrQuant::setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType)
{
  iQP = max( min( iQP, 51 ), 0 );
  
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
#if IT_CLIPPING
      tmp[i*uiTrSize+j] = Clip3(-32768, 32767, (iSum + add_1st)>>shift_1st); // Clipping is normative
#else
      tmp[i*uiTrSize+j] = (iSum + add_1st)>>shift_1st;
#endif
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
#if IT_CLIPPING
      block[i*uiStride+j] = Clip3(-32768, 32767, (iSum + add_2nd)>>shift_2nd); // Clipping is non-normative
#else
      block[i*uiStride+j] = (iSum + add_2nd)>>shift_2nd;
#endif
    }
  }
}

#else //MATRIX_MULT

/** 4x4 forward transform implemented using partial butterfly structure (1D)
 *  \param src   input data (residual)
 *  \param dst   output data (transform coefficients)
 *  \param shift specifies right shift after 1D transform
 */
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

#if NSQT
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
#endif

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

#if IT_CLIPPING
    block[i][0] = Clip3( -32768, 32767, ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][1] = Clip3( -32768, 32767, ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][2] = Clip3( -32768, 32767, ( 74 * (tmp[0][i] - tmp[2][i]  + tmp[3][i])      + rnd_factor ) >> shift );
    block[i][3] = Clip3( -32768, 32767, ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift );
#else
    block[i][0] =  ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift;
    block[i][1] =  ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift;
    block[i][2] =  ( 74 * (tmp[0][i] - tmp[2][i]  + tmp[3][i])      + rnd_factor ) >> shift;
    block[i][3] =  ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift;
#endif
  }
}
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
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode])// Check for DCT or DST
  {
    fastForwardDst(block,tmp,shift_1st); // Forward DST BY FAST ALGORITHM, block input, tmp output
  }
  else  
  {
    partialButterfly4(block,tmp,shift_1st);
  }

  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )   // Check for DCT or DST
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
#if IT_CLIPPING
    dst[j][0] = Clip3( -32768, 32767, (E[0] + O[0] + add)>>shift );
    dst[j][1] = Clip3( -32768, 32767, (E[1] + O[1] + add)>>shift );
    dst[j][2] = Clip3( -32768, 32767, (E[1] - O[1] + add)>>shift );
    dst[j][3] = Clip3( -32768, 32767, (E[0] - O[0] + add)>>shift );
#else
    dst[j][0] = (E[0] + O[0] + add)>>shift;
    dst[j][1] = (E[1] + O[1] + add)>>shift;
    dst[j][2] = (E[1] - O[1] + add)>>shift;
    dst[j][3] = (E[0] - O[0] + add)>>shift;
#endif
  }
}

#if NSQT
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
#if IT_CLIPPING
    dst[0] = Clip3( -32768, 32767, (E[0] + O[0] + add)>>shift );
    dst[1] = Clip3( -32768, 32767, (E[1] + O[1] + add)>>shift );
    dst[2] = Clip3( -32768, 32767, (E[1] - O[1] + add)>>shift );
    dst[3] = Clip3( -32768, 32767, (E[0] - O[0] + add)>>shift );
#else
    dst[0] = (E[0] + O[0] + add)>>shift;
    dst[1] = (E[1] + O[1] + add)>>shift;
    dst[2] = (E[1] - O[1] + add)>>shift;
    dst[3] = (E[0] - O[0] + add)>>shift;
#endif
            
    src   ++;
    dst += 4;
  }
}
#endif

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
  
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Vert[uiMode] )    // Check for DCT or DST
  {
    fastInverseDst(coeff,tmp,shift_1st);    // Inverse DST by FAST Algorithm, coeff input, tmp output
  }
  else
  {
    partialButterflyInverse4(coeff,tmp,shift_1st);    
  } 
  if (uiMode != REG_DCT && g_aucDCTDSTMode_Hor[uiMode] )    // Check for DCT or DST
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

#if NSQT
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
#endif

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
#if IT_CLIPPING
      dst[j][k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+4] = Clip3( -32768, 32767, (E[3-k] - O[3-k] + add)>>shift );
#else
      dst[j][k] = (E[k] + O[k] + add)>>shift;
      dst[j][k+4] = (E[3-k] - O[3-k] + add)>>shift;
#endif
    }        
  }
}

#if NSQT
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
#if IT_CLIPPING
      dst[ k   ] = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[ k+4 ] = Clip3( -32768, 32767, (E[3-k] - O[3-k] + add)>>shift );
#else
      dst[ k   ] = (E[k] + O[k] + add)>>shift;
      dst[ k+4 ] = (E[3-k] - O[3-k] + add)>>shift;
#endif
    }   
    src ++;
    dst += 8;
  }
}
#endif

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

#if NSQT
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
#endif

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
#if IT_CLIPPING
      dst[j][k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+8] = Clip3( -32768, 32767, (E[7-k] - O[7-k] + add)>>shift );
#else
      dst[j][k] = (E[k] + O[k] + add)>>shift;
      dst[j][k+8] = (E[7-k] - O[7-k] + add)>>shift;
#endif
    }        
  }
}

#if NSQT
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
#if IT_CLIPPING
      dst[k]   = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[k+8] = Clip3( -32768, 32767, (E[7-k] - O[7-k] + add)>>shift );
#else
      dst[k] = (E[k] + O[k] + add)>>shift;
      dst[k+8] = (E[7-k] - O[7-k] + add)>>shift;
#endif
    }   
    src ++; 
    dst += 16;
  }
}
#endif

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

#if NSQT
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
#endif

/** 32x32 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 */
void xTr32(short block[32][32],short coeff[32][32])
{
  int shift_1st = 4; // log2(32) - 1 + g_uiBitIncrement
  int shift_2nd = 11;// log2(32) + 6
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
#if IT_CLIPPING
      dst[j][k]    = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[j][k+16] = Clip3( -32768, 32767, (E[15-k] - O[15-k] + add)>>shift );
#else
      dst[j][k] = (E[k] + O[k] + add)>>shift;
      dst[j][k+16] = (E[15-k] - O[15-k] + add)>>shift;
#endif
    }        
  }
}

#if NSQT
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
#if IT_CLIPPING
      dst[k]    = Clip3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      dst[k+16] = Clip3( -32768, 32767, (E[15-k] - O[15-k] + add)>>shift );
#else
      dst[ k    ] = (E[k] + O[k] + add)>>shift;
      dst[ k+16 ] = (E[15-k] - O[15-k] + add)>>shift;
#endif
    }
    src ++;
    dst += 32;
  }
}
#endif

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

#if NSQT
/** MxN forward transform (2D)
*  \param block input data (residual)
*  \param coeff output data (transform coefficients)
*  \param iWidth input data (width of transform)
*  \param iHeight input data (height of transform)
*/
void xTrMxN(short *block,short *coeff, int iWidth, int iHeight)
{
  int shift_1st = g_aucConvertToBit[iWidth]  + 1;                    // log2(iWidth) - 1 + g_uiBitIncrement
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
#if NSQT_TX_ORDER
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
#endif
}
/** MxN inverse transform (2D)
*  \param coeff input data (transform coefficients)
*  \param block output data (residual)
*  \param iWidth input data (width of transform)
*  \param iHeight input data (height of transform)
*/
void xITrMxN(short *coeff,short *block, int iWidth, int iHeight)
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
#if NSQT_TX_ORDER
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
#endif
}
#endif

#endif //MATRIX_MULT

Void TComTrQuant::xQuant(TComDataCU* pcCU, Int* pSrc, TCoeff* pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  Int*   piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
  Int   iAdd = 0;
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
      xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
  }
  else
  {
#if NSQT 
    Bool bNonSqureFlag = ( iWidth != iHeight );
    UInt uiNonSqureScanTableIdx = 0;
    if( bNonSqureFlag )
    {
      UInt uiWidthBit  = g_aucConvertToBit[ iWidth ] + 2;
      UInt uiHeightBit = g_aucConvertToBit[ iHeight ] + 2;
#if NSQT_TX_ORDER
      uiNonSqureScanTableIdx = ( iWidth * iHeight ) == 64 ? 2 * ( iHeight > iWidth ) : 2 * ( iHeight > iWidth ) + 1;
#else
      uiNonSqureScanTableIdx = ( iWidth * iHeight ) == 64 ? 0 : 1;
#endif
      iWidth  = 1 << ( ( uiWidthBit + uiHeightBit) >> 1 );
      iHeight = iWidth;
    }    
#endif

    UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;
    UInt uiQ = g_quantScales[m_cQP.rem()];

    UInt uiBitDepth = 8;
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

    iAdd = (pcCU->getSlice()->getSliceType()==I_SLICE ? 171 : 85) << (iQBits-9);

    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
      Int iLevel;
      Int  iSign;
      UInt uiBlockPos = n;
      iLevel  = piCoef[uiBlockPos];
      iSign   = (iLevel < 0 ? -1: 1);      

      iLevel = (abs(iLevel) * uiQ + iAdd ) >> iQBits;
      uiAcSum += iLevel;
      iLevel *= iSign;        
      piQCoef[uiBlockPos] = iLevel;
#if LEVEL_LIMIT
      piQCoef[uiBlockPos] = Clip3(-32768,32767,piQCoef[uiBlockPos]);
#endif
    } // for n
  } //if RDOQ
  //return;

}

Void TComTrQuant::xDeQuant( const TCoeff* pSrc, Int* pDes, Int iWidth, Int iHeight )
{
  
  const TCoeff* piQCoef   = pSrc;
  Int*   piCoef    = pDes;
#if NSQT
  if( iWidth != iHeight )
  {
    UInt uiWidthBit  = g_aucConvertToBit[ iWidth ]  + 2;
    UInt uiHeightBit = g_aucConvertToBit[ iHeight ] + 2;
    iWidth  = 1 << ( ( uiWidthBit + uiHeightBit) >> 1 );
    iHeight = iWidth;
  }    
#endif

  if ( iWidth > (Int)m_uiMaxTrSize )
  {
    iWidth  = m_uiMaxTrSize;
    iHeight = m_uiMaxTrSize;
  }
  
  Int iShift,iAdd,iCoeffQ;
  UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;

  UInt uiBitDepth = 8;
  UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize; 
  iShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
  iAdd = 1 << (iShift-1);
  Int scale = g_invQuantScales[m_cQP.m_iRem] << m_cQP.m_iPer;

  for( Int n = 0; n < iWidth*iHeight; n++ )
  {
    iCoeffQ = ( piQCoef[n] * scale + iAdd ) >> iShift;
    piCoef[n] = Clip3(-32768,32767,iCoeffQ);
  } 
}

Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Bool bUseRDOQ,  Bool bEnc )
{
  m_uiMaxTrSize  = uiMaxTrSize;
  m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;
}

Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff* rpcCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
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

#if NSQT
  xT( uiMode, pcResidual, uiStride, m_plTempCoeff, uiWidth, uiHeight );
#else
  xT( uiMode, pcResidual, uiStride, m_plTempCoeff, uiWidth );
#endif
  xQuant( pcCU, m_plTempCoeff, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
}


Void TComTrQuant::invtransformNxN( TextType eText,UInt uiMode, Pel* rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight);
#if NSQT
  xIT( uiMode, m_plTempCoeff, rpcResidual, uiStride, uiWidth, uiHeight );
#else
  xIT( uiMode, m_plTempCoeff, rpcResidual, uiStride, uiWidth);
#endif
}

Void TComTrQuant::invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel* rpcResidual, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff )
{
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
  {
    return;
  }
  
  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
  const UInt uiStopTrMode = eTxt == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
#if NSQT
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
#endif
  
  assert(1); // as long as quadtrees are not used for residual transform
  
  if( uiTrMode == uiStopTrMode )
  {
    UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
#if NSQT
    UInt uiTrModeC    = uiTrMode;
#endif
    if( eTxt != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
      {
        return;
      }
      uiWidth  <<= 1;
      uiHeight <<= 1;
#if NSQT
      uiTrModeC--;
#endif
    }
    Pel* pResi = rpcResidual + uiAddr;
#if NSQT
    if( ( eTxt == TEXT_LUMA && pcCU->useNonSquareTrans( uiTrMode ) ) || ( eTxt != TEXT_LUMA && pcCU->useNonSquareTrans( uiTrModeC ) ) )
    {
#if AMP
      UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )? uiWidth >> 1 : uiWidth << 1;
      UInt uiTrHeight = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )? uiHeight << 1 : uiHeight >> 1;
#else
      UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N )? uiWidth >> 1 : uiWidth << 1;
      UInt uiTrHeight = ( ePartSize == SIZE_Nx2N )? uiHeight << 1 : uiHeight >> 1;
#endif

      if( uiWidth == 4 )
      {
        uiTrWidth = uiTrHeight = 4;
      }

#if NSQT_DIAG_SCAN
      uiWidth = uiTrWidth;
      uiHeight = uiTrHeight;
#else
      if( uiTrWidth != uiTrHeight )
      {
#if !NSQT_MOD
        TCoeff  orgCoeff[ 256 ];
#if NSQT_TX_ORDER
        UInt uiNonSqureScanTableIdx = ( uiTrWidth * uiTrHeight ) == 64 ? 2 * ( uiTrHeight > uiTrWidth ) : 2 * ( uiTrHeight > uiTrWidth ) + 1;
#else
        UInt uiNonSqureScanTableIdx = ( uiTrWidth * uiTrHeight ) == 64 ? 0 : 1;
#endif
        memcpy( &orgCoeff[0], rpcCoeff, uiWidth * uiHeight * sizeof( TCoeff ) ); 
        for( UInt uiScanPos = 0; uiScanPos < uiWidth * uiHeight; uiScanPos++ )
        {
          UInt uiBlkPos = g_auiNonSquareSigLastScan[ uiNonSqureScanTableIdx ][ uiScanPos ];
          rpcCoeff[ uiBlkPos ] = orgCoeff[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiScanPos ] ];
        }
#endif
        uiWidth  = uiTrWidth;
        uiHeight = uiTrHeight;
      }
#endif
    }
#endif
    invtransformNxN( eTxt, REG_DCT, pResi, uiStride, rpcCoeff, uiWidth, uiHeight );
  }
  else
  {
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
#if NSQT
    if( pcCU->useNonSquareTrans( uiTrMode ) && ! ( ( uiWidth == 4 && uiTrMode == 1 ) || 
      ( eTxt != TEXT_LUMA && uiTrMode > 1 && ( ( 1 << pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() ) >> 2 ) == 4 ) ) )
    {
      UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
      if( uiTrMode == 1 || ( uiTrMode == 2 && ( uiWidth == 4 || uiLog2TrSize == ( pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 1 ) ) ) )
      {
#if AMP
        if( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )
#else
        if( ePartSize == SIZE_Nx2N )
#endif
        {
          uiAddrOffset = ( uiTrMode == 1 || ( uiLog2TrSize == ( pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 1 ) && uiTrMode == 2 ) ) ? uiWidth >> 1 : uiAddrOffset;
        }
        else
        {
          uiAddrOffset = ( uiTrMode == 1 || ( uiLog2TrSize == ( pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 1 ) && uiTrMode == 2 ) ) ? ( uiWidth >> 1 ) * uiStride : uiWidth;  
        }

        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                    , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset     , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + 2*uiAddrOffset   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + 3*uiAddrOffset   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
      }
      else
      {
#if AMP
        UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N ) ? uiWidth >> 1 : uiWidth << 1;
        UInt uiTrHeight = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N ) ? uiWidth << 1 : uiWidth >> 1;
#else
        UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N ) ? uiWidth >> 1 : uiWidth << 1;
        UInt uiTrHeight = ( ePartSize == SIZE_Nx2N ) ? uiWidth << 1 : uiWidth >> 1;
#endif
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                                   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrWidth                       , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrHeight*uiStride             , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrHeight*uiStride+uiTrWidth   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
      }
    }
    else
#endif
    {
      invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                         , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
      invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth               , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
      invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
      invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
    }
  }
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
#if NSQT
Void TComTrQuant::xT( UInt uiMode, Pel* piBlkResi, UInt uiStride, Int* psCoeff, Int iWidth, Int iHeight )
#else
Void TComTrQuant::xT( UInt uiMode, Pel* piBlkResi, UInt uiStride, Int* psCoeff, Int iSize )
#endif
{
#if MATRIX_MULT  
#if NSQT
  Int iSize = iWidth; 
  if( iWidth != iHeight)
  {
    xTrMxN( piBlkResi, psCoeff, uiStride, (UInt)iWidth, (UInt)iHeight );
    return;
  }
#endif
  xTr(piBlkResi,psCoeff,uiStride,(UInt)iSize,uiMode);
#else
  Int j,k;
#if NSQT
  Int iSize = iWidth; 
  if( iWidth != iHeight)
  {
#if !NSQT_TX_ORDER
    Int iMaxSize = max( iWidth , iHeight);
    Int iMinSize = min( iWidth , iHeight);    
#endif
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
#if !NSQT_TX_ORDER
    if( iWidth > iHeight)
#endif
    {
      for (j = 0; j < iHeight; j++)
      {    
        memcpy( block + j * iWidth, piBlkResi + j * uiStride, iWidth * sizeof( short ) );      
      }
    }
#if !NSQT_TX_ORDER
    else
    {
      for ( j = 0; j < iHeight; j ++)
      {    
        for ( k = 0; k < iWidth; k ++)
        {     
          block[ k * iHeight + j ] =  piBlkResi[ k ];
        }  
        piBlkResi += uiStride;
      } 
    }
    xTrMxN( block, coeff, iMaxSize, iMinSize );
#else
    xTrMxN( block, coeff, iWidth, iHeight );
#endif
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      psCoeff[ j ] = coeff[ j ];
    }
    return ;
  }
#endif
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
}

/** Wrapper function between HM interface and core NxN inverse transform (2D) 
 *  \param plCoef input data (transform coefficients)
 *  \param pResidual output data (residual)
 *  \param uiStride stride of input residual data
 *  \param iSize transform size (iSize x iSize)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
#if NSQT
Void TComTrQuant::xIT( UInt uiMode, Int* plCoef, Pel* pResidual, UInt uiStride, Int iWidth, Int iHeight )
#else
Void TComTrQuant::xIT( UInt uiMode, Int* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
#endif
{
#if MATRIX_MULT  
#if NSQT
  Int iSize = iWidth;
  if( iWidth != iHeight )
  {
    xITrMxN( plCoef, pResidual, uiStride, (UInt)iWidth, (UInt)iHeight );
    return;
  }
#endif
  xITr(plCoef,pResidual,uiStride,(UInt)iSize,uiMode);
#else
  Int j,k;
#if NSQT
  Int iSize = iWidth; 
  if( iWidth != iHeight )
  {
#if !NSQT_TX_ORDER
    Int iMaxSize = max( iWidth , iHeight);
    Int iMinSize = min( iWidth , iHeight);
#endif
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      coeff[j] = (short)plCoef[j];
    }
#if NSQT_TX_ORDER
    xITrMxN( coeff, block, iWidth, iHeight );
#else
    xITrMxN( coeff, block, iMaxSize, iMinSize );
    if( iWidth > iHeight )
#endif
    {
      for ( j = 0; j < iHeight; j++ )
      {    
        memcpy( pResidual + j * uiStride, block + j * iWidth, iWidth * sizeof(short) );      
      }
    }
#if !NSQT_TX_ORDER
    else
    {
      for ( j = 0; j < iHeight; j++ )
      {    
        for ( k = 0; k < iWidth; k++ )
        {     
          pResidual[k] = block[k * iHeight + j];
        }  
        pResidual += uiStride;
      } 
    }
#endif
    return ;
  }
#endif
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
}
 
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
                                                      Int*                            plSrcCoeff,
                                                      TCoeff*                         piDstCoeff,
                                                      UInt                            uiWidth,
                                                      UInt                            uiHeight,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Int    iQBits      = m_cQP.m_iBits;
  Double dTemp       = 0;
  
#if NSQT && !NSQT_DIAG_SCAN
  Bool bNonSqureFlag = ( uiWidth != uiHeight );
  UInt uiNonSqureScanTableIdx = 0;
  if( bNonSqureFlag )
  {
    UInt uiWidthBit  = g_aucConvertToBit[ uiWidth ] + 2;
    UInt uiHeightBit = g_aucConvertToBit[ uiHeight ] + 2;
#if NSQT_TX_ORDER
    uiNonSqureScanTableIdx = ( uiWidth * uiHeight ) == 64 ? 2 * ( uiHeight > uiWidth ) : 2 * ( uiHeight > uiWidth ) + 1;
#else
    uiNonSqureScanTableIdx = ( uiWidth * uiHeight ) == 64 ? 0 : 1;
#endif
    uiWidth  = 1 << ( ( uiWidthBit + uiHeightBit ) >> 1 );
    uiHeight = uiWidth;
  }    
#endif
  UInt uiLog2TrSize = g_aucConvertToBit[ uiWidth ] + 2;
  Int uiQ = g_quantScales[m_cQP.rem()];
#if NSQT_DIAG_SCAN
  if (uiWidth != uiHeight)
  {
    uiLog2TrSize += (uiWidth > uiHeight) ? -1 : 1;
  }
#endif
  
  UInt uiBitDepth = 8;
  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
  double dErrScale = (double)(1<<SCALE_BITS);                              // Compensate for scaling of bitcount in Lagrange cost function
  dErrScale = dErrScale*pow(2.0,-2.0*iTransformShift);                     // Compensate for scaling through forward transform
  dErrScale = dErrScale/(double)(uiQ*uiQ);                                 // Compensate for qp-dependent multiplier applied before calculating the Lagrange cost function
  dErrScale = dErrScale;                                                   // Compensate for Lagrange multiplier that is tuned towards 8-bit input

  iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                   // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

  UInt       uiGoRiceParam       = 0;
  Double     d64BlockUncodedCost = 0;
  const UInt uiLog2BlkSize       = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt uiMaxNumCoeff       = uiWidth * uiHeight;
#if DIAG_SCAN
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
  uiScanIdx = ( uiScanIdx == SCAN_ZIGZAG ) ? SCAN_DIAG : uiScanIdx; // Map value zigzag to diagonal scan
#else
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif
#if NSQT_DIAG_SCAN
  Int blockType = uiLog2BlkSize;
  if (uiWidth != uiHeight)
  {
    uiScanIdx = SCAN_DIAG;
    blockType = 4;
  }
#endif
#if NSQT && !NSQT_DIAG_SCAN
  static Int  orgSrcCoeff[ 256 ];
  if( bNonSqureFlag )
  {
    memcpy( &orgSrcCoeff[ 0 ], plSrcCoeff, uiMaxNumCoeff * sizeof( Int ) );
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
      UInt uiBlkPos = g_auiNonSquareSigLastScan[ uiNonSqureScanTableIdx ][ uiScanPos ];
      plSrcCoeff[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiScanPos ] ] = orgSrcCoeff[ uiBlkPos ]; 
    }
  }
#endif
  
  Double pdCostCoeff [ 32 * 32 ];
  Double pdCostSig   [ 32 * 32 ];
  Double pdCostCoeff0[ 32 * 32 ];
  ::memset( pdCostCoeff, 0, sizeof(Double) *  uiMaxNumCoeff );
  ::memset( pdCostSig,   0, sizeof(Double) *  uiMaxNumCoeff );

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  const UInt * scanCG;
  if (uiWidth == uiHeight)
  {
    scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize > 3 ? uiLog2BlkSize-2-1 : 0  ];
  }
  else
  {
    scanCG = g_sigCGScanNSQT[ uiLog2BlkSize - 2 ];
  }
#else
  const UInt * const scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize > 3 ? uiLog2BlkSize-2-1 : 0  ];
#endif
  const UInt uiCGSize = (1 << MLS_CG_SIZE);         // 16
  Double pdCostCoeffGroupSig[ MLS_GRP_NUM ];
  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
  UInt uiNumBlkSide = uiWidth / MLS_CG_SIZE;
  Int iCGLastScanPos = -1;
#endif

  UInt    uiCtxSet            = 0;
  Int     c1                  = 1;
  Int     c2                  = 0;
  UInt    uiNumOne            = 0;
  Double  d64BaseCost         = 0;
  Int     iLastScanPos        = -1;
  dTemp                       = dErrScale;
#if NSQT_DIAG_SCAN
  const UInt * scan;
  if (uiWidth == uiHeight)
  {
    scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize - 1 ];    
  }
  else
  {
    scan = g_sigScanNSQT[ uiLog2BlkSize - 2 ];
  }
#else
  const UInt * const scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize - 1 ];
#endif
  
#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  if (blockType < 4)
#else
  if (uiLog2BlkSize < 4)
#endif
  {
#endif
  for( Int iScanPos = (Int) uiMaxNumCoeff-1; iScanPos >= 0; iScanPos-- )
  {
    //===== quantization =====
    UInt    uiBlkPos          = scan[iScanPos];
    Int lLevelDouble          = plSrcCoeff[ uiBlkPos ];
    lLevelDouble              = abs(lLevelDouble * uiQ);     
    UInt uiMaxAbsLevel        = (lLevelDouble + (1 << (iQBits - 1))) >> iQBits;
#if LEVEL_LIMIT
    uiMaxAbsLevel=plSrcCoeff[ uiBlkPos ]>=0 ? min<UInt>(uiMaxAbsLevel,32767): min<UInt>(uiMaxAbsLevel,32768);
#endif
    Double dErr               = Double( lLevelDouble );
    pdCostCoeff0[ iScanPos ]  = dErr * dErr * dTemp;
    d64BlockUncodedCost      += pdCostCoeff0[ iScanPos ];
    piDstCoeff[ uiBlkPos ]    = uiMaxAbsLevel;

    if ( uiMaxAbsLevel > 0 && iLastScanPos < 0 )
    {
      iLastScanPos            = iScanPos;
      uiCtxSet                = iScanPos < SCAN_SET_SIZE ? 0 : 3;
#if COEFF_CTXSET_RED
      uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#else
      uiCtxSet                = iScanPos < SCAN_SET_SIZE ? 0 : 3;
#endif
    }    

    if ( iLastScanPos >= 0 )
    {
      //===== coefficient level estimation =====
      UInt  uiLevel;
#if COEFF_CTX_RED
      UInt  uiOneCtx         = 4 * uiCtxSet + c1;
      UInt  uiAbsCtx         = 3 * uiCtxSet + c2;
#else
      UInt  uiOneCtx         = 5 * uiCtxSet + c1;
      UInt  uiAbsCtx         = 5 * uiCtxSet + c2;
#endif
      if( iScanPos == iLastScanPos )
      {
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, iQBits, dTemp, 1 );
      }
      else
      {
        UInt   uiPosY        = uiBlkPos >> uiLog2BlkSize;
        UInt   uiPosX        = uiBlkPos - ( uiPosY << uiLog2BlkSize );
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
        UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
        UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
        UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth, eTType );
#else
        UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth );
#endif
#endif
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, iQBits, dTemp, 0 );
      }

      piDstCoeff[ uiBlkPos ] = uiLevel;
      d64BaseCost           += pdCostCoeff [ iScanPos ];

      //===== update bin model =====
      if( uiLevel > 1 )
      {
        c1 = 0; 
#if COEFF_CTX_RED
        c2 += (c2 < 2);
#else
        c2 += (c2 < 4);
#endif
        uiNumOne++;
        if( uiLevel > 2 )
        {
          uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 15 ) ];
        }
      }
#if COEFF_CTX_RED
      else if( (c1 < 3) && (c1 > 0) && uiLevel)
#else
      else if( (c1 & 3) && uiLevel )
#endif
      {
        c1++;
      }

      //===== context set update =====
      if( ( iScanPos % SCAN_SET_SIZE == 0 ) && ( iScanPos > 0 ) )
      {
        c1                = 1;
        c2                = 0;
        uiGoRiceParam     = 0;
#if COEFF_CTXSET_RED
        uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#else
        uiCtxSet          = iScanPos == SCAN_SET_SIZE ? 0 : 3;
#endif
        if( uiNumOne > 0 )
        {
          uiCtxSet++;
#if COEFF_CTXSET_RED
          if(eTType==TEXT_LUMA)
          {
#endif
          if( uiNumOne > 3 )
          {
            uiCtxSet++;
          }
#if COEFF_CTXSET_RED
          }
#endif
        }
        uiNumOne    >>= 1;
      }
    }
    else
    {
      d64BaseCost    += pdCostCoeff0[ iScanPos ];
    }
  }
#if MULTI_LEVEL_SIGNIFICANCE
  }
  else //(uiLog2BlkSize > 3), for 16x16 and 32x32 TU
  {      
    ::memset( pdCostCoeffGroupSig,   0, sizeof(Double) * MLS_GRP_NUM );
    ::memset( uiSigCoeffGroupFlag,   0, sizeof(UInt) * MLS_GRP_NUM );

#if NSQT_DIAG_SCAN
    UInt uiCGNum = uiWidth * uiHeight >> MLS_CG_SIZE;
#else
    UInt uiCGNum = uiNumBlkSide * uiNumBlkSide;
#endif
    Int iScanPos;
    coeffGroupRDStats rdStats;     

    for (Int iCGScanPos = uiCGNum-1; iCGScanPos >= 0; iCGScanPos--)
    {
      UInt uiCGBlkPos = scanCG[ iCGScanPos ];
      UInt uiCGPosY   = uiCGBlkPos / uiNumBlkSide;
      UInt uiCGPosX   = uiCGBlkPos - (uiCGPosY * uiNumBlkSide);
      ::memset( &rdStats, 0, sizeof (coeffGroupRDStats));
        
      for (Int iScanPosinCG = uiCGSize-1; iScanPosinCG >= 0; iScanPosinCG--)
      {
        iScanPos = iCGScanPos*uiCGSize + iScanPosinCG;
        //===== quantization =====
        UInt    uiBlkPos          = scan[iScanPos];
        Int lLevelDouble          = plSrcCoeff[ uiBlkPos ];
        lLevelDouble              = abs(lLevelDouble * uiQ);     
        UInt uiMaxAbsLevel        = (lLevelDouble + (1 << (iQBits - 1))) >> iQBits;

        Double dErr               = Double( lLevelDouble );
        pdCostCoeff0[ iScanPos ]  = dErr * dErr * dTemp;
        d64BlockUncodedCost      += pdCostCoeff0[ iScanPos ];
        piDstCoeff[ uiBlkPos ]    = uiMaxAbsLevel;

        if ( uiMaxAbsLevel > 0 && iLastScanPos < 0 )
        {
          iLastScanPos            = iScanPos;
#if COEFF_CTXSET_RED
          uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#else
          uiCtxSet                = iScanPos < SCAN_SET_SIZE ? 0 : 3;
#endif
          iCGLastScanPos          = iCGScanPos;
        }

        if ( iLastScanPos >= 0 )
        {
          //===== coefficient level estimation =====
          UInt  uiLevel;
#if COEFF_CTX_RED
          UInt  uiOneCtx         = 4 * uiCtxSet + c1;
          UInt  uiAbsCtx         = 3 * uiCtxSet + c2;
#else
          UInt  uiOneCtx         = 5 * uiCtxSet + c1;
          UInt  uiAbsCtx         = 5 * uiCtxSet + c2;
#endif
          if( iScanPos == iLastScanPos )
          {
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], 
                                                   lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   iQBits, dTemp, 1 );
          }
          else
          {
            UInt   uiPosY        = uiBlkPos >> uiLog2BlkSize;
            UInt   uiPosX        = uiBlkPos - ( uiPosY << uiLog2BlkSize );
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
            UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
            UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
            UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth, eTType );
#else
            UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth );
#endif
#endif
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ],
                                                   lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   iQBits, dTemp, 0 );
          }

          piDstCoeff[ uiBlkPos ] = uiLevel;
          d64BaseCost           += pdCostCoeff [ iScanPos ];

          //===== update bin model =====
          if( uiLevel > 1 )
          {
            c1 = 0; 
#if COEFF_CTX_RED
            c2 += (c2 < 2);
#else
            c2 += (c2 < 4);
#endif
            uiNumOne++;
            if( uiLevel > 2 )
            {
              uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 15 ) ];
            }
          }
#if COEFF_CTX_RED
          else if( (c1 < 3) && (c1 > 0) && uiLevel)
#else
          else if( (c1 & 3) && uiLevel )
#endif
          {
            c1++;
          }

          //===== context set update =====
          if( ( iScanPos % SCAN_SET_SIZE == 0 ) && ( iScanPos > 0 ) )
          {
            c1                = 1;
            c2                = 0;
            uiGoRiceParam     = 0;
#if COEFF_CTXSET_RED
            uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#else
            uiCtxSet          = iScanPos == SCAN_SET_SIZE ? 0 : 3;
#endif
            if( uiNumOne > 0 )
            {
              uiCtxSet++;
#if COEFF_CTXSET_RED
              if(eTType==TEXT_LUMA)
              {
#endif
                if( uiNumOne > 3 )
                {
                  uiCtxSet++;
                }
#if COEFF_CTXSET_RED
              }
#endif
            }
            uiNumOne    >>= 1;
          }
        }
        else
        {
          d64BaseCost    += pdCostCoeff0[ iScanPos ];
        }
        rdStats.d64SigCost += pdCostSig[ iScanPos ];
        if (iScanPosinCG == 0 )
        {
          rdStats.d64SigCost_0 = pdCostSig[ iScanPos ];
        }
        if (piDstCoeff[ uiBlkPos ] )
        {
          uiSigCoeffGroupFlag[ uiCGBlkPos ] = 1;
          rdStats.d64CodedLevelandDist += pdCostCoeff[ iScanPos ] - pdCostSig[ iScanPos ];
          rdStats.d64UncodedDist += pdCostCoeff0[ iScanPos ];
          if ( iScanPosinCG != 0 )
          {
            rdStats.iNNZbeforePos0++;
          }
        }
      } //end for (iScanPosinCG)

      if (iCGLastScanPos >= 0) 
      {
#if NSQT_DIAG_SCAN
        if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight ) && (iCGScanPos != 0) )
#else
        if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiLog2BlkSize ) && (iCGScanPos != 0) )
#endif
        {
          if (uiSigCoeffGroupFlag[ uiCGBlkPos ] == 0)
          {
#if NSQT_DIAG_SCAN
            UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight);
#else
            UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiLog2BlkSize);
#endif
            d64BaseCost += xGetRateSigCoeffGroup(0, uiCtxSig) - rdStats.d64SigCost;;  
            pdCostCoeffGroupSig[ iCGScanPos ] = xGetRateSigCoeffGroup(0, uiCtxSig);  
          } 
          else
          {
            if (iCGScanPos < iCGLastScanPos) //skip the last coefficient group, which will be handled together with last position below.
            {
              if ( rdStats.iNNZbeforePos0 == 0 ) 
              {
                d64BaseCost -= rdStats.d64SigCost_0;
                rdStats.d64SigCost -= rdStats.d64SigCost_0;
              }
              // rd-cost if SigCoeffGroupFlag = 0, initialization
              Double d64CostZeroCG = d64BaseCost;

              // add SigCoeffGroupFlag cost to total cost
#if NSQT_DIAG_SCAN
              UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight);
#else
              UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiLog2BlkSize);
#endif
              if (iCGScanPos < iCGLastScanPos)
              {
                d64BaseCost  += xGetRateSigCoeffGroup(1, uiCtxSig); 
                d64CostZeroCG += xGetRateSigCoeffGroup(0, uiCtxSig);  
                pdCostCoeffGroupSig[ iCGScanPos ] = xGetRateSigCoeffGroup(1, uiCtxSig); 
              }

              // try to convert the current coeff group from non-zero to all-zero
              d64CostZeroCG += rdStats.d64UncodedDist;  // distortion for resetting non-zero levels to zero levels
              d64CostZeroCG -= rdStats.d64CodedLevelandDist;   // distortion and level cost for keeping all non-zero levels
              d64CostZeroCG -= rdStats.d64SigCost;     // sig cost for all coeffs, including zero levels and non-zerl levels

              // if we can save cost, change this block to all-zero block
              if ( d64CostZeroCG < d64BaseCost )      
              {
                uiSigCoeffGroupFlag[ uiCGBlkPos ] = 0;
                d64BaseCost = d64CostZeroCG;
                if (iCGScanPos < iCGLastScanPos)
                {
                  pdCostCoeffGroupSig[ iCGScanPos ] = xGetRateSigCoeffGroup(0, uiCtxSig); 
                }
                // reset coeffs to 0 in this block                
                for (Int iScanPosinCG = uiCGSize-1; iScanPosinCG >= 0; iScanPosinCG--)
                {
                  iScanPos      = iCGScanPos*uiCGSize + iScanPosinCG;
                  UInt uiBlkPos = scan[ iScanPos ];

                  if (piDstCoeff[ uiBlkPos ])
                  {
                    piDstCoeff [ uiBlkPos ] = 0;
                    pdCostCoeff[ iScanPos ] = pdCostCoeff0[ iScanPos ];
                    pdCostSig  [ iScanPos ] = 0;
                  }
                }
              } // end if ( d64CostAllZeros < d64BaseCost )      
            }
          } // end if if (uiSigCoeffGroupFlag[ uiCGBlkPos ] == 0)
        }
        else // if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY ) && (uiCGScanPos != 0) && (uiSigCoeffGroupFlag[ uiCGBlkPos ] != 0) )
        {
          uiSigCoeffGroupFlag[ uiCGBlkPos ] = 1;
        } // end if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY ) && (uiCGScanPos != 0) && (uiSigCoeffGroupFlag[ uiCGBlkPos ] != 0) )
      }
    } //end for (iCGScanPos)
  }
#endif  // #if MULTI_LEVEL_SIGNIFICANCE 

  //===== estimate last position =====
  if ( iLastScanPos < 0 )
  {
    return;
  }

  Double  d64BestCost         = 0;
  Int     ui16CtxCbf          = 0;
  Int     iBestLastIdxP1      = 0;
  if( !pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && pcCU->getTransformIdx( uiAbsPartIdx ) == 0 )
  {
    ui16CtxCbf   = 0;
    d64BestCost  = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockRootCbpBits[ ui16CtxCbf ][ 0 ] );
    d64BaseCost += xGetICost( m_pcEstBitsSbac->blockRootCbpBits[ ui16CtxCbf ][ 1 ] );
  }
  else
  {
    ui16CtxCbf   = pcCU->getCtxQtCbf( uiAbsPartIdx, eTType, pcCU->getTransformIdx( uiAbsPartIdx ) );
#if CHROMA_CBF_CTX_REDUCTION
    ui16CtxCbf   = ( eTType ? TEXT_CHROMA : eTType ) * NUM_QT_CBF_CTX + ui16CtxCbf;
#else
    ui16CtxCbf   = ( eTType ? eTType - 1 : eTType ) * NUM_QT_CBF_CTX + ui16CtxCbf;
#endif
    d64BestCost  = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 0 ] );
    d64BaseCost += xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 1 ] );
  }

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  if (blockType < 4)
#else
  if (uiLog2BlkSize < 4)
#endif
  {
#endif
  for( Int iScanPos = iLastScanPos; iScanPos >= 0; iScanPos-- )
  {
    UInt   uiBlkPos     = scan[iScanPos];
    if( piDstCoeff[ uiBlkPos ] )
    {
      UInt   uiPosY       = uiBlkPos >> uiLog2BlkSize;
      UInt   uiPosX       = uiBlkPos - ( uiPosY << uiLog2BlkSize );
      Double d64CostLast= uiScanIdx == SCAN_VER ? xGetRateLast( uiPosY, uiPosX, uiWidth ) : xGetRateLast( uiPosX, uiPosY, uiWidth );
      Double totalCost = d64BaseCost + d64CostLast - pdCostSig[ iScanPos ];
      if( totalCost < d64BestCost )
      {
        iBestLastIdxP1  = iScanPos + 1;
        d64BestCost     = totalCost;
      }
      if( piDstCoeff[ uiBlkPos ] > 1 )
      {
        break;
      }
      d64BaseCost      -= pdCostCoeff[ iScanPos ];
      d64BaseCost      += pdCostCoeff0[ iScanPos ];
    }
    else
    {
      d64BaseCost      -= pdCostSig[ iScanPos ];
    }
  }
#if MULTI_LEVEL_SIGNIFICANCE
  }
  else //if (uiLog2BlkSize < 4)
  {
    Bool bFoundLast = false;
    for (Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos--)
    {
      UInt uiCGBlkPos = scanCG[ iCGScanPos ];

      d64BaseCost -= pdCostCoeffGroupSig [ iCGScanPos ]; 
      if (uiSigCoeffGroupFlag[ uiCGBlkPos ])
      {     
        for (Int iScanPosinCG = uiCGSize-1; iScanPosinCG >= 0; iScanPosinCG--)
        {
          Int iScanPos = iCGScanPos*uiCGSize + iScanPosinCG;
          if (iScanPos > iLastScanPos) continue;
          UInt   uiBlkPos     = scan[iScanPos];

          if( piDstCoeff[ uiBlkPos ] )
          {
            UInt   uiPosY       = uiBlkPos >> uiLog2BlkSize;
            UInt   uiPosX       = uiBlkPos - ( uiPosY << uiLog2BlkSize );

            Double d64CostLast= uiScanIdx == SCAN_VER ? xGetRateLast( uiPosY, uiPosX, uiWidth ) : xGetRateLast( uiPosX, uiPosY, uiWidth );
            Double totalCost = d64BaseCost + d64CostLast - pdCostSig[ iScanPos ];

            if( totalCost < d64BestCost )
            {
              iBestLastIdxP1  = iScanPos + 1;
              d64BestCost     = totalCost;
            }
            if( piDstCoeff[ uiBlkPos ] > 1 )
            {
              bFoundLast = true;
              break;
            }
            d64BaseCost      -= pdCostCoeff[ iScanPos ];
            d64BaseCost      += pdCostCoeff0[ iScanPos ];
          }
          else
          {
            d64BaseCost      -= pdCostSig[ iScanPos ];
          }
        } //end for 
        if (bFoundLast)
        {
          break;
        }
      } // end if (uiSigCoeffGroupFlag[ uiCGBlkPos ])
    } // end for 
  } //if (uiLog2BlkSize < 4)
#endif

  for ( Int scanPos = 0; scanPos < iBestLastIdxP1; scanPos++ )
  {
    Int blkPos = scan[ scanPos ];
    Int level  = piDstCoeff[ blkPos ];
    uiAbsSum += level;
    piDstCoeff[ blkPos ] = ( plSrcCoeff[ blkPos ] < 0 ) ? -level : level;
  }
  
  //===== clean uncoded coefficients =====
  for ( Int scanPos = iBestLastIdxP1; scanPos <= iLastScanPos; scanPos++ )
  {
    piDstCoeff[ scan[ scanPos ] ] = 0;
  }

#if NSQT && !NSQT_DIAG_SCAN
  static TCoeff dstCoeff[ 256 ];
  if( bNonSqureFlag )
  {
    memcpy( plSrcCoeff, &orgSrcCoeff[ 0 ], uiMaxNumCoeff * sizeof( Int ) );

    memcpy( &dstCoeff[ 0 ], piDstCoeff, uiMaxNumCoeff * sizeof( TCoeff ) );        
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
      UInt uiBlkPos = g_auiNonSquareSigLastScan[ uiNonSqureScanTableIdx ][ uiScanPos ];
      piDstCoeff[ uiBlkPos ] = dstCoeff[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiScanPos ] ];
    }        
  }
#endif
}

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
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
                                   Int uiStride, Int height, Int eTType )
#else
                                   Int uiStride, Int height )
#endif
#else
#if SIGMAP_CTX_RED
                                    const UInt                      uiStride,
                                    const UInt                      eTType )
#else
                                    const UInt                      uiStride )
#endif
#endif
{
  if ( uiLog2BlkSize == 2)
  {
#if SIGMAP_CTX_RED
    const UChar CTX_IND_MAP_4x4[30] =
    {
      //LUMA map
      0, 1, 4, 5,
      2, 3, 4, 5,
      6, 6, 8, 8,
      7, 7, 8,
      //CHROMA map
      0, 1, 2, 4,
      1, 1, 2, 4,
      3, 3, 5, 5,
      4, 4, 5
    };

    if (eTType == TEXT_LUMA)
    {
      return CTX_IND_MAP_4x4[(uiPosY << 2) + uiPosX];
    }
    else
    {
      return CTX_IND_MAP_4x4[15 + (uiPosY << 2) + uiPosX];
    }
#else
    return 4 * uiPosY + uiPosX;
#endif
  }
  
  if ( uiLog2BlkSize == 3 )
  {
#if SIGMAP_CTX_RED
    const UInt map8x8[] = { 0,  1,  2,  3,
                            4,  5,  6,  3,
                            8,  6,  6,  7,
                            9,  9,  7,  7  };
    UInt uiOffset = (eTType == TEXT_LUMA) ? 9 : 6;

    if ( uiPosX + uiPosY == 0 )
    {
      return uiOffset + 10;
    }
    return uiOffset + map8x8[4 * (uiPosY >> 1) + (uiPosX >> 1)];
#else
    return 15 + 4 * (uiPosY >> 1) + (uiPosX >> 1);
#endif
  }

#if SIGMAP_CTX_RED
  UInt uiOffset = (eTType == TEXT_LUMA) ? 20 : 17;
  if( uiPosX + uiPosY == 0 )
  {
    return uiOffset;
  }
#else
  if( uiPosX + uiPosY < 2 )
  {
    return 31 + 2 * uiPosY + uiPosX;
  }
#endif
  
#if NSQT_DIAG_SCAN
  const TCoeff *pData = pcCoeff + uiPosX + uiPosY * uiStride;
#else
  const TCoeff *pData = pcCoeff + uiPosX + (uiPosY << uiLog2BlkSize);
#endif
  
  Int iStride = uiStride;
#if SIGMAP_CTX_RED
#if NSQT_DIAG_SCAN
  Int thred = std::max(height, iStride) >> 2;
#else
  UInt thred = 1 << (uiLog2BlkSize-2);
#endif
#endif
  
#if !NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
  if(eTType==TEXT_LUMA && uiPosX + uiPosY < thred)
#else
  if( uiPosX + uiPosY < 5 )
#endif
  {
#if SUBBLOCK_SCAN
    UInt cnt = (pData[1] != 0) + (pData[2] != 0) + (pData[2*iStride] != 0) + (pData[iStride+1] != 0);
    if( ( ( uiPosX & 3 ) || ( uiPosY & 3 ) ) && ( ( (uiPosX+1) & 3 ) || ( (uiPosY+2) & 3 ) ) )
    {
      cnt += pData[iStride] != 0;
    }
#else
    UInt cnt = (pData[1] != 0) + (pData[2] != 0) + (pData[iStride] != 0) + (pData[2*iStride] != 0) + (pData[iStride+1] != 0);
#endif  
#if SIGMAP_CTX_RED
    cnt=(cnt+1)>>1;
    return uiOffset + 1 + min<UInt>( 2, cnt );
#else
    return 31 + 3 + min<UInt>( 4, cnt );
#endif
  }
#endif
  
  UInt uiWidthM1   = uiStride - 1;
  UInt cnt = 0;
  if( uiPosX < uiWidthM1 )
  {
    cnt += pData[1] != 0;
#if NSQT_DIAG_SCAN
    if( uiPosY < height - 1 )
#else
    if( uiPosY < uiWidthM1 )
#endif
    {
      cnt += pData[iStride+1] != 0;
    }
    if( uiPosX < uiWidthM1 - 1 )
    {
      cnt += pData[2] != 0;
    }
  }
#if NSQT_DIAG_SCAN
  if ( uiPosY < height - 1 )
#else
  if ( uiPosY < uiWidthM1 )
#endif
  {
#if SUBBLOCK_SCAN
  if( ( ( uiPosX & 3 ) || ( uiPosY & 3 ) ) && ( ( (uiPosX+1) & 3 ) || ( (uiPosY+2) & 3 ) ) )
  {
    cnt += pData[iStride] != 0;
  }
#else
    cnt += pData[iStride] != 0;
#endif
#if NSQT_DIAG_SCAN
    if ( uiPosY < height - 2 && cnt < 4 )
#else
    if ( uiPosY < uiWidthM1 - 1 && cnt < 4 )
#endif
    {
      cnt += pData[2*iStride] != 0;
    }
  }

#if SIGMAP_CTX_RED
  cnt=(cnt+1)>>1;
#if NSQT_DIAG_SCAN
  return (( eTType == TEXT_LUMA && uiPosX + uiPosY >= thred ) ? uiOffset + 4 : uiOffset + 1) + cnt;
#else
  if(eTType==TEXT_LUMA)
  {
    return uiOffset + 4 + cnt;
  }
  else
  {
    return uiOffset + 1 + cnt;
  }
#endif
#else
#if NSQT_DIAG_SCAN
  return (( uiPosX + uiPosY < 5 ) ? 31 + 3 : 31 + 8) + cnt;
#else
  return 31 + 8 + cnt;
#endif
#endif
}

/** Get the best level in RD sense
 * \param rd64CodedCost reference to coded cost
 * \param rd64CodedCost0 reference to cost when coefficient is 0
 * \param rd64CodedCostSig reference to cost of significant coefficient
 * \param lLevelDouble reference to unscaled quantized level
 * \param uiMaxAbsLevel scaled quantized level
 * \param ui16CtxNumSig current ctxInc for coeff_abs_significant_flag
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_greater2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param ui16AbsGoRice current Rice parameter for coeff_abs_level_minus3
 * \param iQBits quantization step size
 * \param dTemp correction factor
 * \param bLast indicates if the coefficient is the last significant
 * \returns best quantized transform level for given scan position
 * This method calculates the best quantized transform level for a given scan position.
 */
__inline UInt TComTrQuant::xGetCodedLevel ( Double&                         rd64CodedCost,
                                            Double&                         rd64CodedCost0,
                                            Double&                         rd64CodedCostSig,
                                            Int                             lLevelDouble,
                                            UInt                            uiMaxAbsLevel,
                                            UShort                          ui16CtxNumSig,
                                            UShort                          ui16CtxNumOne,
                                            UShort                          ui16CtxNumAbs,
                                            UShort                          ui16AbsGoRice,
                                            Int                             iQBits,
                                            Double                          dTemp,
                                            Bool                            bLast        ) const
{
  Double dCurrCostSig   = 0; 
  UInt   uiBestAbsLevel = 0;
  
  if( !bLast && uiMaxAbsLevel < 3 )
  {
    rd64CodedCostSig    = xGetRateSigCoef( 0, ui16CtxNumSig ); 
    rd64CodedCost       = rd64CodedCost0 + rd64CodedCostSig;
    if( uiMaxAbsLevel == 0 )
    {
      return uiBestAbsLevel;
    }
  }
  else
  {
    rd64CodedCost       = MAX_DOUBLE;
  }

  if( !bLast )
  {
    dCurrCostSig        = xGetRateSigCoef( 1, ui16CtxNumSig );
  }

  UInt uiMinAbsLevel    = ( uiMaxAbsLevel > 1 ? uiMaxAbsLevel - 1 : 1 );
  for( Int uiAbsLevel  = uiMaxAbsLevel; uiAbsLevel >= uiMinAbsLevel ; uiAbsLevel-- )
  {
    Double dErr         = Double( lLevelDouble  - ( uiAbsLevel << iQBits ) );
    Double dCurrCost    = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice );
    dCurrCost          += dCurrCostSig;

    if( dCurrCost < rd64CodedCost )
    {
      uiBestAbsLevel    = uiAbsLevel;
      rd64CodedCost     = dCurrCost;
      rd64CodedCostSig  = dCurrCostSig;
    }
  }

  return uiBestAbsLevel;
}

/** Calculates the cost for specific absolute transform level
 * \param uiAbsLevel scaled quantized level
 * \param ui16CtxNumOne current ctxInc for coeff_abs_level_greater1 (1st bin of coeff_abs_level_minus1 in AVC)
 * \param ui16CtxNumAbs current ctxInc for coeff_abs_level_greater2 (remaining bins of coeff_abs_level_minus1 in AVC)
 * \param ui16AbsGoRice Rice parameter for coeff_abs_level_minus3
 * \returns cost of given absolute transform level
 */
__inline Double TComTrQuant::xGetICRateCost  ( UInt                            uiAbsLevel,
                                               UShort                          ui16CtxNumOne,
                                               UShort                          ui16CtxNumAbs,
                                               UShort                          ui16AbsGoRice
                                              ) const
{
  Double iRate = xGetIEPRate();
  if( uiAbsLevel == 1 )
  {
    iRate += m_pcEstBitsSbac->m_greaterOneBits[ ui16CtxNumOne ][ 0 ];
  }
  else if( uiAbsLevel == 2 )
  {
    iRate += m_pcEstBitsSbac->m_greaterOneBits[ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->m_levelAbsBits[ ui16CtxNumAbs ][ 0 ];
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
    iRate += m_pcEstBitsSbac->m_greaterOneBits[ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->m_levelAbsBits[ ui16CtxNumAbs ][ 1 ];
  }
  return xGetICost( iRate );
}

#if MULTI_LEVEL_SIGNIFICANCE
__inline Double TComTrQuant::xGetRateSigCoeffGroup  ( UShort                    uiSignificanceCoeffGroup,
                                                UShort                          ui16CtxNumSig ) const
{
  return xGetICost( m_pcEstBitsSbac->significantCoeffGroupBits[ ui16CtxNumSig ][ uiSignificanceCoeffGroup ] );
}
#endif

/** Calculates the cost of signaling the last significant coefficient in the block
 * \param uiPosX X coordinate of the last significant coefficient
 * \param uiPosY Y coordinate of the last significant coefficient
 * \returns cost of last significant coefficient
 */
#if MODIFIED_LAST_XY_CODING
/*
 * \param uiWidth width of the transform unit (TU)
*/
__inline Double TComTrQuant::xGetRateLast   ( const UInt                      uiPosX,
                                              const UInt                      uiPosY,
                                              const UInt                      uiBlkWdth     ) const
{
  UInt uiCtxX   = g_uiGroupIdx[uiPosX];
  UInt uiCtxY   = g_uiGroupIdx[uiPosY];
  Double uiCost = m_pcEstBitsSbac->lastXBits[ uiCtxX ] + m_pcEstBitsSbac->lastYBits[ uiCtxY ];
  if( uiCtxX > 3 )
  {
    uiCost += xGetIEPRate() * ((uiCtxX-2)>>1);
  }
  if( uiCtxY > 3 )
  {
    uiCost += xGetIEPRate() * ((uiCtxY-2)>>1);
  }
  return xGetICost( uiCost );
}
#else
/*
 * \param uiWidth width of the transform unit (TU)
*/
__inline Double TComTrQuant::xGetRateLast   ( const UInt                      uiPosX,
                                              const UInt                      uiPosY,
                                              const UInt                      uiBlkWdth     ) const
{
  UInt uiCtxX              = uiPosX;
  UInt uiCtxY              = uiPosY;
  const UInt uiMinWidth    = min<UInt>( 4, uiBlkWdth );
  const UInt uiHalfWidth   = uiBlkWdth >> 1;
  const UInt uiLog2BlkSize = g_aucConvertToBit[ uiHalfWidth ] + 2;

  Double uiCost = 0;

  if( uiHalfWidth >= uiMinWidth )
  {
    if( uiPosX >= uiHalfWidth )
    {
      uiCost += xGetIEPRate() * uiLog2BlkSize;
      uiCtxX  = uiHalfWidth;
    }

    if( uiPosY >= uiHalfWidth )
    {
      uiCost += xGetIEPRate() * uiLog2BlkSize;
      uiCtxY  = uiHalfWidth;
    }
  }

  uiCost += m_pcEstBitsSbac->lastXBits[ uiCtxX ] + m_pcEstBitsSbac->lastYBits[ uiCtxY ];

  return xGetICost( uiCost );
}
#endif

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

#if MULTI_LEVEL_SIGNIFICANCE
/** Context derivation process of coeff_abs_significant_flag
 * \param uiSigCoeffGroupFlag significance map of L1
 * \param uiBlkX column of current scan position
 * \param uiBlkY row of current scan position
 * \param uiLog2BlkSize log2 value of block size
 * \returns ctxInc for current scan position
 */
#if NSQT_DIAG_SCAN
UInt TComTrQuant::getSigCoeffGroupCtxInc  ( const UInt*               uiSigCoeffGroupFlag,
                                           const UInt                      uiCGPosX,
                                           const UInt                      uiCGPosY,
                                           Int width, Int height)
#else
UInt TComTrQuant::getSigCoeffGroupCtxInc  ( const UInt*               uiSigCoeffGroupFlag,
                                      const UInt                      uiCGPosX,
                                      const UInt                      uiCGPosY,
                                      const UInt                      uiLog2BlockSize)
#endif
{
  UInt uiRight = 0;
  UInt uiLower = 0;

#if NSQT_DIAG_SCAN
  width >>= 2;
  height >>= 2;
  if( uiCGPosX < width - 1 )
  {
    uiRight = (uiSigCoeffGroupFlag[ uiCGPosY * width + uiCGPosX + 1 ] != 0);
  }
  if (uiCGPosY < height - 1 )
  {
    uiLower = (uiSigCoeffGroupFlag[ (uiCGPosY  + 1 ) * width + uiCGPosX ] != 0);
  }
#else
  if( uiLog2BlockSize == 4 )
  {
    if( uiCGPosX < 3 )
    {
      uiRight = (uiSigCoeffGroupFlag[ (uiCGPosY << 2) + uiCGPosX + 1 ] != 0);
    }
    if (uiCGPosY < 3 )
    {
      uiLower = (uiSigCoeffGroupFlag[ ((uiCGPosY  + 1 ) << 2) + uiCGPosX ] != 0);
    }
  }
  else
  {
    if( uiCGPosX < 7 )
    {
      uiRight = (uiSigCoeffGroupFlag[ (uiCGPosY << 3) + uiCGPosX + 1 ] != 0);
    }
    if (uiCGPosY < 7 )
    {
      uiLower = (uiSigCoeffGroupFlag[ ((uiCGPosY  + 1 ) << 3) + uiCGPosX ] != 0);
     }
  }
#endif
  return uiRight + uiLower;

}

// return 1 if both right neighbour and lower neighour are 1's
#if NSQT_DIAG_SCAN
Bool TComTrQuant::bothCGNeighboursOne ( const UInt*                   uiSigCoeffGroupFlag,
                                       const UInt                      uiCGPosX,
                                       const UInt                      uiCGPosY, 
                                       Int width, Int height)
#else
Bool TComTrQuant::bothCGNeighboursOne ( const UInt*                   uiSigCoeffGroupFlag,
                                      const UInt                      uiCGPosX,
                                      const UInt                      uiCGPosY, 
                                      const UInt                      uiLog2BlockSize)
#endif
{
  UInt uiRight = 0;
  UInt uiLower = 0;

#if NSQT_DIAG_SCAN
  width >>= 2;
  height >>= 2;
  if( uiCGPosX < width - 1 )
  {
    uiRight = (uiSigCoeffGroupFlag[ uiCGPosY * width + uiCGPosX + 1 ] != 0);
  }
  if (uiCGPosY < height - 1 )
  {
    uiLower = (uiSigCoeffGroupFlag[ (uiCGPosY  + 1 ) * width + uiCGPosX ] != 0);
  }
#else
  if( uiLog2BlockSize == 4 )
  {
    if( uiCGPosX < 3 )
    {
      uiRight = (uiSigCoeffGroupFlag[ (uiCGPosY << 2) + uiCGPosX + 1 ] != 0);
    }
    if (uiCGPosY < 3 )
    {
      uiLower = (uiSigCoeffGroupFlag[ ((uiCGPosY  + 1 ) << 2) + uiCGPosX ] != 0);
    }
  }
  else
  {
    if( uiCGPosX < 7 )
    {
      uiRight = (uiSigCoeffGroupFlag[ (uiCGPosY << 3) + uiCGPosX + 1 ] != 0);
    }
    if (uiCGPosY < 7 )
    {
      uiLower = (uiSigCoeffGroupFlag[ ((uiCGPosY  + 1 ) << 3) + uiCGPosX ] != 0);
    }
  }
#endif
  
  return (uiRight & uiLower);
}
#endif
//! \}
