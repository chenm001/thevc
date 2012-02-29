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
  initScalingList();
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
  destroyScalingList();
}

#if ADAPTIVE_QP_SELECTION
Void TComTrQuant::storeSliceQpNext(TComSlice* pcSlice)
{
  Int qpBase = pcSlice->getSliceQpBase();
  Int sliceQpused = pcSlice->getSliceQp();
  Int sliceQpnext;
  Double alpha = qpBase < 17 ? 0.5 : 1;
  
  Int cnt=0;
  for(int u=1; u<=LEVEL_RANGE; u++)
  { 
    cnt += m_sliceNsamples[u] ;
  }

  if( !m_bUseRDOQ )
  {
    sliceQpused = qpBase;
    alpha = 0.5;
  }

  if( cnt > 120 )
  {
    Double sum = 0;
    Int k = 0;
    for(Int u=1; u<LEVEL_RANGE; u++)
    {
      sum += u*m_sliceSumC[u];
      k += u*u*m_sliceNsamples[u];
    }

    Int v;
    Double q[MAX_QP+1] ;
    for(v=0; v<=MAX_QP; v++)
    {
      q[v] = (Double)(g_invQuantScales[v%6] * (1<<(v/6)))/64 ;
    }

    Double qnext = sum/k * q[sliceQpused] / (1<<ARL_C_PRECISION);

    for(v=0; v<MAX_QP; v++)
    {
      if(qnext < alpha * q[v] + (1 - alpha) * q[v+1] )
      {
        break;
      }
    }
    sliceQpnext = Clip3(sliceQpused - 3, sliceQpused + 3, v);
  }
  else
  {
    sliceQpnext = sliceQpused;
  }

  m_qpDelta[qpBase] = sliceQpnext - qpBase; 
}

Void TComTrQuant::initSliceQpDelta()
{
  for(Int qp=0; qp<=MAX_QP; qp++)
  {
    m_qpDelta[qp] = qp < 17 ? 0 : 1;
  }
}

Void TComTrQuant::clearSliceARLCnt()
{ 
  memset(m_sliceSumC, 0, sizeof(Double)*(LEVEL_RANGE+1));
  memset(m_sliceNsamples, 0, sizeof(Int)*(LEVEL_RANGE+1));
}
#endif


#if H0736_AVC_STYLE_QP_RANGE
/** Set qP for Quantization.
 * \param qpy QPy
 * \param bLowpass
 * \param eSliceType
 * \param eTxtType
 * \param qpBdOffset
 * \param chromaQPOffset
 *
 * return void  
 */
Void TComTrQuant::setQPforQuant( Int qpy, Bool bLowpass, SliceType eSliceType, TextType eTxtType, Int qpBdOffset, Int chromaQPOffset)
{
  Int qpScaled;

  if(eTxtType == TEXT_LUMA)
  {
    qpScaled = qpy + qpBdOffset;
  }
  else
  {
    qpScaled = Clip3( -qpBdOffset, 51, qpy + chromaQPOffset );

    if(qpScaled < 0)
    {
      qpScaled = qpScaled + qpBdOffset;
    }
    else
    {
      qpScaled = g_aucChromaScale[ Clip3(0, 51, qpScaled) ] + qpBdOffset;
    }
  }
  m_cQP.setQpParam( qpScaled, bLowpass, eSliceType );
}
#else
/// Including Chroma QP Parameter setting
Void TComTrQuant::setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType, Int Shift)
{
  iQP = Clip3( MIN_QP, MAX_QP, iQP + Shift );
  
  if(eTxtType != TEXT_LUMA) //Chroma
  {
    iQP  = g_aucChromaScale[ iQP ];
  }
  
  m_cQP.setQpParam( iQP, bLowpass, eSliceType );
}
#endif

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

#if FULL_NBIT
  int shift_1st = uiLog2TrSize - 1 + g_uiBitDepth - 8; // log2(N) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = uiLog2TrSize - 1 + g_uiBitIncrement; // log2(N) - 1 + g_uiBitIncrement
#endif

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
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
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

    block[i][0] = Clip3( -32768, 32767, ( 29 * c[0] + 55 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][1] = Clip3( -32768, 32767, ( 55 * c[2] - 29 * c[1]     + c[3]               + rnd_factor ) >> shift );
    block[i][2] = Clip3( -32768, 32767, ( 74 * (tmp[0][i] - tmp[2][i]  + tmp[3][i])      + rnd_factor ) >> shift );
    block[i][3] = Clip3( -32768, 32767, ( 55 * c[0] + 29 * c[2]     - c[3]               + rnd_factor ) >> shift );
  }
}
/** 4x4 forward transform (2D)
 *  \param block input data (residual)
 *  \param coeff output data (transform coefficients)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xTr4(short block[4][4],short coeff[4][4],UInt uiMode)
{
#if FULL_NBIT
  int shift_1st = 1 + g_uiBitDepth - 8; // log2(4) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = 1 + g_uiBitIncrement; // log2(4) - 1 + g_uiBitIncrement
#endif
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

/** 4x4 inverse transform (2D)
 *  \param coeff input data (transform coefficients)
 *  \param block output data (residual)
 *  \param uiMode is Intra Prediction mode used in Mode-Dependent DCT/DST only
 */
void xITr4(short coeff[4][4],short block[4][4], UInt uiMode)
{
  int shift_1st = SHIFT_INV_1ST;
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif
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

/** MxN forward transform (2D)
*  \param block input data (residual)
*  \param coeff output data (transform coefficients)
*  \param iWidth input data (width of transform)
*  \param iHeight input data (height of transform)
*/
void xTrMxN(short *block,short *coeff, int iWidth, int iHeight)
{
#if FULL_NBIT
  int shift_1st = g_aucConvertToBit[iWidth]  + 1 + g_uiBitDepth - 8; // log2(iWidth) - 1 + g_uiBitDepth - 8
#else
  int shift_1st = g_aucConvertToBit[iWidth]  + 1 + g_uiBitIncrement; // log2(iWidth) - 1 + g_uiBitIncrement
#endif
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
#if FULL_NBIT
  int shift_2nd = SHIFT_INV_2ND - ((short)g_uiBitDepth - 8);
#else
  int shift_2nd = SHIFT_INV_2ND - g_uiBitIncrement;
#endif

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
#if ADAPTIVE_QP_SELECTION
                          Int*&       pArlDes,
#endif
                          Int         iWidth, 
                          Int         iHeight, 
                          UInt&       uiAcSum, 
                          TextType    eTType, 
                          UInt        uiAbsPartIdx )
{
  Int*   piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
#if ADAPTIVE_QP_SELECTION
  Int*   piArlCCoef = pArlDes;
#endif
  Int   iAdd = 0;
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
#if ADAPTIVE_QP_SELECTION
    xRateDistOptQuant( pcCU, piCoef, pDes, pArlDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
#else
    xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
#endif
  }
  else
  {
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

#if ADAPTIVE_QP_SELECTION
    QpParam cQpBase;
    Int iQpBase = pcCU->getSlice()->getSliceQpBase();

#if H0736_AVC_STYLE_QP_RANGE
    Int qpScaled;
    Int qpBDOffset = (eTType == TEXT_LUMA)? pcCU->getSlice()->getSPS()->getQpBDOffsetY() : pcCU->getSlice()->getSPS()->getQpBDOffsetC();

    if(eTType == TEXT_LUMA)
    {
      qpScaled = iQpBase + qpBDOffset;
    }
    else
    {
      qpScaled = Clip3( -qpBDOffset, 51, iQpBase);

      if(qpScaled < 0)
      {
        qpScaled = qpScaled +  qpBDOffset;
      }
      else
      {
        qpScaled = g_aucChromaScale[ Clip3(0, 51, qpScaled) ] + qpBDOffset;
      }
    }
    cQpBase.setQpParam(qpScaled, false, pcCU->getSlice()->getSliceType());
#else
    if(eTType != TEXT_LUMA)
    {
      iQpBase = g_aucChromaScale[iQpBase];
    }
    cQpBase.setQpParam(iQpBase, false, pcCU->getSlice()->getSliceType());
#endif
#endif

    Bool bNonSqureFlag = ( iWidth != iHeight );
    UInt dir           = SCALING_LIST_SQT;
    if( bNonSqureFlag )
    {
      dir = ( iWidth < iHeight )?  SCALING_LIST_VER: SCALING_LIST_HOR;
      UInt uiWidthBit  = g_aucConvertToBit[ iWidth ] + 2;
      UInt uiHeightBit = g_aucConvertToBit[ iHeight ] + 2;
      iWidth  = 1 << ( ( uiWidthBit + uiHeightBit) >> 1 );
      iHeight = iWidth;
    }    

    UInt uiLog2TrSize = g_aucConvertToBit[ iWidth ] + 2;
    Int scalingListType = (pcCU->isIntra(uiAbsPartIdx) ? 0 : 3) + g_eTTable[(Int)eTType];
    assert(scalingListType < 6);
    Int *piQuantCoeff = 0;
    piQuantCoeff = getQuantCoeff(scalingListType,m_cQP.m_iRem,uiLog2TrSize-2, dir);

#if FULL_NBIT
    UInt uiBitDepth = g_uiBitDepth;
#else
    UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;
#endif
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits

    iAdd = (pcCU->getSlice()->getSliceType()==I_SLICE ? 171 : 85) << (iQBits-9);

#if ADAPTIVE_QP_SELECTION
    iQBits = QUANT_SHIFT + cQpBase.m_iPer + iTransformShift;
    iAdd = (pcCU->getSlice()->getSliceType()==I_SLICE ? 171 : 85) << (iQBits-9);
    Int iQBitsC = QUANT_SHIFT + cQpBase.m_iPer + iTransformShift - ARL_C_PRECISION;  
    Int iAddC   = 1 << (iQBitsC-1);
#endif

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

#if ADAPTIVE_QP_SELECTION
      Int64 tmpLevel = (Int64)abs(iLevel) * piQuantCoeff[uiBlockPos];
      if( m_bUseAdaptQpSelect )
      {
        piArlCCoef[uiBlockPos] = (Int)((tmpLevel + iAddC ) >> iQBitsC);
      }
      iLevel = (Int)((tmpLevel + iAdd ) >> iQBits);
#if MULTIBITS_DATA_HIDING
      deltaU[uiBlockPos] = (Int)((tmpLevel - (iLevel<<iQBits) )>> qBits8);
#endif
#else
      iLevel = ((Int64)abs(iLevel) * piQuantCoeff[uiBlockPos] + iAdd ) >> iQBits;
#if MULTIBITS_DATA_HIDING
      deltaU[uiBlockPos] = (Int)( ((Int64)abs(iLevel) * piQuantCoeff[uiBlockPos] - (iLevel<<iQBits) )>> qBits8 );
#endif
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
  } //if RDOQ
  //return;

}

Void TComTrQuant::xDeQuant( const TCoeff* pSrc, Int* pDes, Int iWidth, Int iHeight, Int scalingListType )
{
  
  const TCoeff* piQCoef   = pSrc;
  Int*   piCoef    = pDes;
  UInt dir          = SCALING_LIST_SQT;
  if( iWidth != iHeight )
  {
    dir          = ( iWidth < iHeight )? SCALING_LIST_VER: SCALING_LIST_HOR;
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

#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;
#endif
  UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize; 
  iShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;

#if DEQUAN_CLIPPING
  TCoeff iClipQCoef;
  const Int iBitRange = min( 15, ( Int )( 12 + uiLog2TrSize + uiBitDepth - m_cQP.m_iPer) );
  const Int iLevelLimit = 1 << iBitRange;
#endif

  if(getUseScalingList())
  {
    iShift += 4;
    if(iShift > m_cQP.m_iPer)
    {
      iAdd = 1 << (iShift - m_cQP.m_iPer - 1);
    }
    else
    {
      iAdd = 0;
    }
    Int *piDequantCoef = getDequantCoeff(scalingListType,m_cQP.m_iRem,uiLog2TrSize-2,dir);

    if(iShift > m_cQP.m_iPer)
    {
      for( Int n = 0; n < iWidth*iHeight; n++ )
      {
#if DEQUAN_CLIPPING
        iClipQCoef = Clip3( -32768, 32767, piQCoef[n] );
        iCoeffQ = ((iClipQCoef * piDequantCoef[n]) + iAdd ) >> (iShift -  m_cQP.m_iPer);
#else
        iCoeffQ = ((piQCoef[n] * piDequantCoef[n]) + iAdd ) >> (iShift -  m_cQP.m_iPer);
#endif
        piCoef[n] = Clip3(-32768,32767,iCoeffQ);
      }
    }
    else
    {
      for( Int n = 0; n < iWidth*iHeight; n++ )
      {
#if DEQUAN_CLIPPING
        iClipQCoef = Clip3( -iLevelLimit, iLevelLimit - 1, piQCoef[n] );
        iCoeffQ = (iClipQCoef * piDequantCoef[n]) << (m_cQP.m_iPer - iShift);
#else
        iCoeffQ = (piQCoef[n] * piDequantCoef[n]) << (m_cQP.m_iPer - iShift);
#endif
        piCoef[n] = Clip3(-32768,32767,iCoeffQ);
      }
    }
  }
  else
  {
    iAdd = 1 << (iShift-1);
    Int scale = g_invQuantScales[m_cQP.m_iRem] << m_cQP.m_iPer;

    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
#if DEQUAN_CLIPPING
      iClipQCoef = Clip3( -32768, 32767, piQCoef[n] );
      iCoeffQ = ( iClipQCoef * scale + iAdd ) >> iShift;
#else
      iCoeffQ = ( piQCoef[n] * scale + iAdd ) >> iShift;
#endif
      piCoef[n] = Clip3(-32768,32767,iCoeffQ);
    }
  }
}

Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Int iSymbolMode, UInt *aTableLP4, UInt *aTableLP8, UInt *aTableLastPosVlcIndex,
                       Bool bUseRDOQ,  Bool bEnc
#if ADAPTIVE_QP_SELECTION
                       , Bool bUseAdaptQpSelect
#endif
                       )
{
  m_uiMaxTrSize  = uiMaxTrSize;
  m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;
#if ADAPTIVE_QP_SELECTION
  m_bUseAdaptQpSelect = bUseAdaptQpSelect;
#endif
}

Void TComTrQuant::transformNxN( TComDataCU* pcCU, 
                                Pel*        pcResidual, 
                                UInt        uiStride, 
                                TCoeff*     rpcCoeff, 
#if ADAPTIVE_QP_SELECTION
                                Int*&       rpcArlCoeff, 
#endif
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
  xQuant( pcCU, m_plTempCoeff, rpcCoeff,
#if ADAPTIVE_QP_SELECTION
       rpcArlCoeff,
#endif
       uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
}


Void TComTrQuant::invtransformNxN( TextType eText,UInt uiMode, Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight, Int scalingListType )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight, scalingListType);
  xIT( uiMode, m_plTempCoeff, rpcResidual, uiStride, uiWidth, uiHeight );
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
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  
  assert(1); // as long as quadtrees are not used for residual transform
  
  if( uiTrMode == uiStopTrMode )
  {
    UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
    UInt uiTrModeC    = uiTrMode;
    if( eTxt != TEXT_LUMA && uiLog2TrSize == 2 )
    {
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
      {
        return;
      }
      uiWidth  <<= 1;
      uiHeight <<= 1;
      uiTrModeC--;
    }
    Pel* pResi = rpcResidual + uiAddr;
    if( ( eTxt == TEXT_LUMA && pcCU->useNonSquareTrans( uiTrMode ) ) || ( eTxt != TEXT_LUMA && pcCU->useNonSquareTrans( uiTrModeC ) ) )
    {
      UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )? uiWidth >> 1 : uiWidth << 1;
      UInt uiTrHeight = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )? uiHeight << 1 : uiHeight >> 1;

      if( uiWidth == 4 )
      {
        uiTrWidth = uiTrHeight = 4;
      }

      uiWidth = uiTrWidth;
      uiHeight = uiTrHeight;
    }
    Int scalingListType = (pcCU->isIntra(uiAbsPartIdx) ? 0 : 3) + g_eTTable[(Int)eTxt];
    assert(scalingListType < 6);
    invtransformNxN( eTxt, REG_DCT, pResi, uiStride, rpcCoeff, uiWidth, uiHeight, scalingListType );
  }
  else
  {
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
    if( pcCU->useNonSquareTrans( uiTrMode ) && ! ( ( uiWidth == 4 && uiTrMode == 1 ) || 
      ( eTxt != TEXT_LUMA && uiTrMode > 1 && ( ( 1 << pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() ) >> 2 ) == 4 ) ) )
    {
      UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
      if( uiTrMode == 1 || ( uiTrMode == 2 && ( uiWidth == 4 || uiLog2TrSize == ( pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 1 ) ) ) )
      {
        if( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N )
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
        UInt uiTrWidth  = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N ) ? uiWidth >> 1 : uiWidth << 1;
        UInt uiTrHeight = ( ePartSize == SIZE_Nx2N || ePartSize == SIZE_nLx2N || ePartSize == SIZE_nRx2N ) ? uiWidth << 1 : uiWidth >> 1;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                                   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrWidth                       , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrHeight*uiStride             , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
        invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiTrHeight*uiStride+uiTrWidth   , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
      }
    }
    else
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
  Int j,k;
  Int iSize = iWidth; 
  if( iWidth != iHeight)
  {
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
    {
      for (j = 0; j < iHeight; j++)
      {    
        memcpy( block + j * iWidth, piBlkResi + j * uiStride, iWidth * sizeof( short ) );      
      }
    }
    xTrMxN( block, coeff, iWidth, iHeight );
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      psCoeff[ j ] = coeff[ j ];
    }
    return ;
  }
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
  Int j,k;
  Int iSize = iWidth; 
  if( iWidth != iHeight )
  {
    short block[ 64 * 64 ];
    short coeff[ 64 * 64 ];
    for ( j = 0; j < iHeight * iWidth; j++ )
    {    
      coeff[j] = (short)plCoef[j];
    }
    xITrMxN( coeff, block, iWidth, iHeight );
    {
      for ( j = 0; j < iHeight; j++ )
      {    
        memcpy( pResidual + j * uiStride, block + j * iWidth, iWidth * sizeof(short) );      
      }
    }
    return ;
  }
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
#if ADAPTIVE_QP_SELECTION
                                                      Int*&                           piArlDstCoeff,
#endif
                                                      UInt                            uiWidth,
                                                      UInt                            uiHeight,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Int    iQBits      = m_cQP.m_iBits;
  Double dTemp       = 0;
  
  UInt dir         = SCALING_LIST_SQT;
  UInt uiLog2TrSize = g_aucConvertToBit[ uiWidth ] + 2;
  Int uiQ = g_quantScales[m_cQP.rem()];
  if (uiWidth != uiHeight)
  {
    uiLog2TrSize += (uiWidth > uiHeight) ? -1 : 1;
    dir            = ( uiWidth < uiHeight )?  SCALING_LIST_VER: SCALING_LIST_HOR;
  }
  
#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;  
#endif
  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform
  UInt       uiGoRiceParam       = 0;
  Double     d64BlockUncodedCost = 0;
  const UInt uiLog2BlkSize       = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt uiMaxNumCoeff       = uiWidth * uiHeight;
  Int scalingListType = (pcCU->isIntra(uiAbsPartIdx) ? 0 : 3) + g_eTTable[(Int)eTType];
  assert(scalingListType < 6);
 
  iQBits = QUANT_SHIFT + m_cQP.m_iPer + iTransformShift;                   // Right shift of non-RDOQ quantizer;  level = (coeff*uiQ + offset)>>q_bits
  double dErrScale   = 0;
  double *pdErrScaleOrg = getErrScaleCoeff(scalingListType,uiLog2TrSize-2,m_cQP.m_iRem,dir);
  Int *piQCoefOrg = getQuantCoeff(scalingListType,m_cQP.m_iRem,uiLog2TrSize-2,dir);
  Int *piQCoef = piQCoefOrg;
  double *pdErrScale = pdErrScaleOrg;
#if ADAPTIVE_QP_SELECTION
  Int iQBitsC = iQBits - ARL_C_PRECISION;
  Int iAddC =  1 << (iQBitsC-1);
#endif
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
  if (uiScanIdx == SCAN_ZIGZAG)
  {
    // Map value zigzag to diagonal scan
    uiScanIdx = SCAN_DIAG;
  }
  Int blockType = uiLog2BlkSize;
  if (uiWidth != uiHeight)
  {
    uiScanIdx = SCAN_DIAG;
    blockType = 4;
  }
  
#if ADAPTIVE_QP_SELECTION
  memset(piArlDstCoeff, 0, sizeof(Int) *  uiMaxNumCoeff);
#endif

  Double pdCostCoeff [ 32 * 32 ];
  Double pdCostSig   [ 32 * 32 ];
  Double pdCostCoeff0[ 32 * 32 ];
  ::memset( pdCostCoeff, 0, sizeof(Double) *  uiMaxNumCoeff );
  ::memset( pdCostSig,   0, sizeof(Double) *  uiMaxNumCoeff );
#if MULTIBITS_DATA_HIDING
  Int rateIncUp   [ 32 * 32 ];
  Int rateIncDown [ 32 * 32 ];
  Int sigRateDelta[ 32 * 32 ];
  Int deltaU      [ 32 * 32 ];
  ::memset( rateIncUp,    0, sizeof(Int) *  uiMaxNumCoeff );
  ::memset( rateIncDown,  0, sizeof(Int) *  uiMaxNumCoeff );
  ::memset( sigRateDelta, 0, sizeof(Int) *  uiMaxNumCoeff );
  ::memset( deltaU,       0, sizeof(Int) *  uiMaxNumCoeff );
#endif

  const UInt * scanCG;
  if (uiWidth == uiHeight)
  {
    scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize > 3 ? uiLog2BlkSize-2-1 : 0  ];
#if MULTILEVEL_SIGMAP_EXT
    if( uiLog2BlkSize == 3 )
    {
      scanCG = g_sigLastScan8x8[ uiScanIdx ];
    }
    else if( uiLog2BlkSize == 5 )
    {
      scanCG = g_sigLastScanCG32x32;
    }
#endif
  }
  else
  {
    scanCG = g_sigCGScanNSQT[ uiLog2BlkSize - 2 ];
  }
  const UInt uiCGSize = (1 << MLS_CG_SIZE);         // 16
  Double pdCostCoeffGroupSig[ MLS_GRP_NUM ];
  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
  UInt uiNumBlkSide = uiWidth / MLS_CG_SIZE;
  Int iCGLastScanPos = -1;

  UInt    uiCtxSet            = 0;
  Int     c1                  = 1;
  Int     c2                  = 0;
  UInt    uiNumOne            = 0;
  Double  d64BaseCost         = 0;
  Int     iLastScanPos        = -1;
  dTemp                       = dErrScale;

#if RESTRICT_GR1GR2FLAG_NUMBER
  UInt    c1Idx     = 0;
  UInt    c2Idx     = 0;
  Int     baseLevel;
#endif

  const UInt * scan;
  if (uiWidth == uiHeight)
  {
    scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlkSize - 1 ];    
  }
  else
  {
    scan = g_sigScanNSQT[ uiLog2BlkSize - 2 ];
  }

#if !MULTILEVEL_SIGMAP_EXT
  if (blockType < 4)
  {
  for( Int iScanPos = (Int) uiMaxNumCoeff-1; iScanPos >= 0; iScanPos-- )
  {
    //===== quantization =====
    UInt    uiBlkPos          = scan[iScanPos];
    // set coeff
    uiQ  = piQCoef[uiBlkPos];
    dTemp = pdErrScale[uiBlkPos];
    Int lLevelDouble          = plSrcCoeff[ uiBlkPos ];
    lLevelDouble              = (Int)min<Int64>(((Int64)abs(lLevelDouble) * uiQ), MAX_INT-(1 << (iQBits - 1)));
#if ADAPTIVE_QP_SELECTION
    if( m_bUseAdaptQpSelect )
    {
      piArlDstCoeff[uiBlkPos]   = (Int)(( lLevelDouble + iAddC) >> iQBitsC );
    }
#endif
    UInt uiMaxAbsLevel        = (lLevelDouble + (1 << (iQBits - 1))) >> iQBits;
    uiMaxAbsLevel=plSrcCoeff[ uiBlkPos ]>=0 ? min<UInt>(uiMaxAbsLevel,32767): min<UInt>(uiMaxAbsLevel,32768);
    Double dErr               = Double( lLevelDouble );
    pdCostCoeff0[ iScanPos ]  = dErr * dErr * dTemp;
    d64BlockUncodedCost      += pdCostCoeff0[ iScanPos ];
    piDstCoeff[ uiBlkPos ]    = uiMaxAbsLevel;

    if ( uiMaxAbsLevel > 0 && iLastScanPos < 0 )
    {
      iLastScanPos            = iScanPos;
#if LEVEL_CTX_LUMA_RED
      uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 2;
#else
      uiCtxSet                = iScanPos < SCAN_SET_SIZE ? 0 : 3;
      uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#endif
    }    

    if ( iLastScanPos >= 0 )
    {
      //===== coefficient level estimation =====
      UInt  uiLevel;
      UInt  uiOneCtx         = 4 * uiCtxSet + c1;
#if RESTRICT_GR1GR2FLAG_NUMBER
      UInt  uiAbsCtx         = uiCtxSet + c2;
#else
      UInt  uiAbsCtx         = 3 * uiCtxSet + c2;
#endif

      if( iScanPos == iLastScanPos )
      {
#if RESTRICT_GR1GR2FLAG_NUMBER
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx, iQBits, dTemp, 1 );
#else
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, iQBits, dTemp, 1 );
#endif
      }
      else
      {
        UInt   uiPosY        = uiBlkPos >> uiLog2BlkSize;
        UInt   uiPosX        = uiBlkPos - ( uiPosY << uiLog2BlkSize );
        UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#if RESTRICT_GR1GR2FLAG_NUMBER
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx, iQBits, dTemp, 0 );
#else
        uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, iQBits, dTemp, 0 );
#endif
#if MULTIBITS_DATA_HIDING
        sigRateDelta[ uiBlkPos ] = m_pcEstBitsSbac->significantBits[ uiCtxSig ][ 1 ] - m_pcEstBitsSbac->significantBits[ uiCtxSig ][ 0 ];
#endif
      }
#if MULTIBITS_DATA_HIDING
      deltaU[ uiBlkPos ]        = (lLevelDouble - ((Int)uiLevel << iQBits)) >> (iQBits-8);
      if( uiLevel > 0 )
      {
#if RESTRICT_GR1GR2FLAG_NUMBER   
        Int rateNow = xGetICRate( uiLevel, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx );
        rateIncUp   [ uiBlkPos ] = xGetICRate( uiLevel+1, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx ) - rateNow;
        rateIncDown [ uiBlkPos ] = xGetICRate( uiLevel-1, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx ) - rateNow;
#else  
        Int rateNow = xGetICRate( uiLevel, uiOneCtx, uiAbsCtx, uiGoRiceParam );
        rateIncUp   [ uiBlkPos ] = xGetICRate( uiLevel+1, uiOneCtx, uiAbsCtx, uiGoRiceParam ) - rateNow;
        rateIncDown [ uiBlkPos ] = xGetICRate( uiLevel-1, uiOneCtx, uiAbsCtx, uiGoRiceParam ) - rateNow;
#endif
      }
      else // uiLevel == 0
      {
        rateIncUp   [ uiBlkPos ] = m_pcEstBitsSbac->m_greaterOneBits[ uiOneCtx ][ 0 ];
      }
#endif
      piDstCoeff[ uiBlkPos ] = uiLevel;
      d64BaseCost           += pdCostCoeff [ iScanPos ];

#if RESTRICT_GR1GR2FLAG_NUMBER
      baseLevel = (c1Idx < C1FLAG_NUMBER) ? (2 + (c2Idx < C2FLAG_NUMBER)) : 1;
      if( uiLevel >= baseLevel )
      {
#if EIGHT_BITS_RICE_CODE
        uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - baseLevel, 23 ) ];
#else
        uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - baseLevel, 15 ) ];
#endif
      }
      if ( uiLevel >= 1)
      {
        c1Idx ++;
      }
#endif

      //===== update bin model =====
      if( uiLevel > 1 )
      {
        c1 = 0; 
        c2 += (c2 < 2);
        uiNumOne++;
#if RESTRICT_GR1GR2FLAG_NUMBER
        c2Idx ++;
#else
        if( uiLevel > 2 )
        {
#if EIGHT_BITS_RICE_CODE
          uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 23 ) ];
#else
          uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 15 ) ];
#endif
        }
#endif
      }
      else if( (c1 < 3) && (c1 > 0) && uiLevel)
      {
        c1++;
      }

      //===== context set update =====
      if( ( iScanPos % SCAN_SET_SIZE == 0 ) && ( iScanPos > 0 ) )
      {
        c1                = 1;
        c2                = 0;
        uiGoRiceParam     = 0;

#if RESTRICT_GR1GR2FLAG_NUMBER
        c1Idx   = 0;
        c2Idx   = 0; 
#endif
#if LEVEL_CTX_LUMA_RED
        uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 2;
#else
        uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#endif
        if( uiNumOne > 0 )
        {
          uiCtxSet++;
#if !LEVEL_CTX_LUMA_RED
          if(uiNumOne > 3 && eTType==TEXT_LUMA)
          {
            uiCtxSet++;
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
  }
  else //(uiLog2BlkSize > 3), for 16x16 and 32x32 TU
  {      
#endif
    ::memset( pdCostCoeffGroupSig,   0, sizeof(Double) * MLS_GRP_NUM );
    ::memset( uiSigCoeffGroupFlag,   0, sizeof(UInt) * MLS_GRP_NUM );

    UInt uiCGNum = uiWidth * uiHeight >> MLS_CG_SIZE;
    Int iScanPos;
    coeffGroupRDStats rdStats;     

    for (Int iCGScanPos = uiCGNum-1; iCGScanPos >= 0; iCGScanPos--)
    {
      UInt uiCGBlkPos = scanCG[ iCGScanPos ];
      UInt uiCGPosY   = uiCGBlkPos / uiNumBlkSide;
      UInt uiCGPosX   = uiCGBlkPos - (uiCGPosY * uiNumBlkSide);
#if MULTILEVEL_SIGMAP_EXT
      if( uiWidth == 8 && uiHeight == 8 && (uiScanIdx == SCAN_HOR || uiScanIdx == SCAN_VER) )
      {
        uiCGPosY = (uiScanIdx == SCAN_HOR ? uiCGBlkPos : 0);
        uiCGPosX = (uiScanIdx == SCAN_VER ? uiCGBlkPos : 0);
      }
#endif
      ::memset( &rdStats, 0, sizeof (coeffGroupRDStats));
        
      for (Int iScanPosinCG = uiCGSize-1; iScanPosinCG >= 0; iScanPosinCG--)
      {
        iScanPos = iCGScanPos*uiCGSize + iScanPosinCG;
        //===== quantization =====
        UInt    uiBlkPos          = scan[iScanPos];
        // set coeff
        uiQ  = piQCoef[uiBlkPos];
        dTemp = pdErrScale[uiBlkPos];
        Int lLevelDouble          = plSrcCoeff[ uiBlkPos ];
        lLevelDouble              = (Int)min<Int64>((Int64)abs((Int)lLevelDouble) * uiQ , MAX_INT - (1 << (iQBits - 1)));
#if ADAPTIVE_QP_SELECTION
        if( m_bUseAdaptQpSelect )
        {
          piArlDstCoeff[uiBlkPos]   = (Int)(( lLevelDouble + iAddC) >> iQBitsC );
        }
#endif
        UInt uiMaxAbsLevel        = (lLevelDouble + (1 << (iQBits - 1))) >> iQBits;

        Double dErr               = Double( lLevelDouble );
        pdCostCoeff0[ iScanPos ]  = dErr * dErr * dTemp;
        d64BlockUncodedCost      += pdCostCoeff0[ iScanPos ];
        piDstCoeff[ uiBlkPos ]    = uiMaxAbsLevel;

        if ( uiMaxAbsLevel > 0 && iLastScanPos < 0 )
        {
          iLastScanPos            = iScanPos;
#if LEVEL_CTX_LUMA_RED
          uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 2;
#else
          uiCtxSet                = (iScanPos < SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#endif
          iCGLastScanPos          = iCGScanPos;
        }

        if ( iLastScanPos >= 0 )
        {
          //===== coefficient level estimation =====
          UInt  uiLevel;
          UInt  uiOneCtx         = 4 * uiCtxSet + c1;
#if RESTRICT_GR1GR2FLAG_NUMBER
          UInt  uiAbsCtx         = uiCtxSet + c2;
#else
          UInt  uiAbsCtx         = 3 * uiCtxSet + c2;
#endif

          if( iScanPos == iLastScanPos )
          {
#if RESTRICT_GR1GR2FLAG_NUMBER  
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], 
                                                   lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   c1Idx, c2Idx, iQBits, dTemp, 1 );
#else
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ], 
                                                   lLevelDouble, uiMaxAbsLevel, 0, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   iQBits, dTemp, 1 );
#endif
          }
          else
          {
            UInt   uiPosY        = uiBlkPos >> uiLog2BlkSize;
            UInt   uiPosX        = uiBlkPos - ( uiPosY << uiLog2BlkSize );
            UShort uiCtxSig      = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#if RESTRICT_GR1GR2FLAG_NUMBER
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ],
                                                   lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   c1Idx, c2Idx, iQBits, dTemp, 0 );
#else
            uiLevel              = xGetCodedLevel( pdCostCoeff[ iScanPos ], pdCostCoeff0[ iScanPos ], pdCostSig[ iScanPos ],
                                                   lLevelDouble, uiMaxAbsLevel, uiCtxSig, uiOneCtx, uiAbsCtx, uiGoRiceParam, 
                                                   iQBits, dTemp, 0 );
#endif
#if MULTIBITS_DATA_HIDING
            sigRateDelta[ uiBlkPos ] = m_pcEstBitsSbac->significantBits[ uiCtxSig ][ 1 ] - m_pcEstBitsSbac->significantBits[ uiCtxSig ][ 0 ];
#endif
          }
#if MULTIBITS_DATA_HIDING
          deltaU[ uiBlkPos ]        = (lLevelDouble - ((Int)uiLevel << iQBits)) >> (iQBits-8);
          if( uiLevel > 0 )
          {
#if RESTRICT_GR1GR2FLAG_NUMBER   
            Int rateNow = xGetICRate( uiLevel, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx );
            rateIncUp   [ uiBlkPos ] = xGetICRate( uiLevel+1, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx ) - rateNow;
            rateIncDown [ uiBlkPos ] = xGetICRate( uiLevel-1, uiOneCtx, uiAbsCtx, uiGoRiceParam, c1Idx, c2Idx ) - rateNow;
#else
            Int rateNow = xGetICRate( uiLevel, uiOneCtx, uiAbsCtx, uiGoRiceParam );
            rateIncUp   [ uiBlkPos ] = xGetICRate( uiLevel+1, uiOneCtx, uiAbsCtx, uiGoRiceParam ) - rateNow;
            rateIncDown [ uiBlkPos ] = xGetICRate( uiLevel-1, uiOneCtx, uiAbsCtx, uiGoRiceParam ) - rateNow;
#endif
          }
          else // uiLevel == 0
          {
            rateIncUp   [ uiBlkPos ] = m_pcEstBitsSbac->m_greaterOneBits[ uiOneCtx ][ 0 ];
          }
#endif
          piDstCoeff[ uiBlkPos ] = uiLevel;
          d64BaseCost           += pdCostCoeff [ iScanPos ];


#if RESTRICT_GR1GR2FLAG_NUMBER
          baseLevel = (c1Idx < C1FLAG_NUMBER) ? (2 + (c2Idx < C2FLAG_NUMBER)) : 1;
          if( uiLevel >= baseLevel )
          {
#if EIGHT_BITS_RICE_CODE
            uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - baseLevel , 23 ) ];
#else
            uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - baseLevel, 15 ) ];
#endif
          }
          if ( uiLevel >= 1)
          {
            c1Idx ++;
          }
#endif

          //===== update bin model =====
          if( uiLevel > 1 )
          {
            c1 = 0; 
            c2 += (c2 < 2);
            uiNumOne++;
#if RESTRICT_GR1GR2FLAG_NUMBER
            c2Idx ++;
#else
            if( uiLevel > 2 )
            {
#if EIGHT_BITS_RICE_CODE
              uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 23 ) ];
#else
              uiGoRiceParam = g_aauiGoRiceUpdate[ uiGoRiceParam ][ min<UInt>( uiLevel - 3, 15 ) ];
#endif
            }
#endif
          }
          else if( (c1 < 3) && (c1 > 0) && uiLevel)
          {
            c1++;
          }

          //===== context set update =====
          if( ( iScanPos % SCAN_SET_SIZE == 0 ) && ( iScanPos > 0 ) )
          {
            c1                = 1;
            c2                = 0;
            uiGoRiceParam     = 0;

#if RESTRICT_GR1GR2FLAG_NUMBER
            c1Idx   = 0;
            c2Idx   = 0; 
#endif
#if LEVEL_CTX_LUMA_RED
            uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 2;
#else
            uiCtxSet          = (iScanPos == SCAN_SET_SIZE || eTType!=TEXT_LUMA) ? 0 : 3;
#endif
            if( uiNumOne > 0 )
            {
              uiCtxSet++;
#if !LEVEL_CTX_LUMA_RED
              if( uiNumOne > 3 && eTType==TEXT_LUMA)
              {
                uiCtxSet++;
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
#if REMOVE_INFER_SIGGRP
        if( iCGScanPos )
#else
#if MULTILEVEL_SIGMAP_EXT
        if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiScanIdx, uiWidth, uiHeight ) && (iCGScanPos != 0) )
#else
        if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight ) && (iCGScanPos != 0) )
#endif
#endif
        {
          if (uiSigCoeffGroupFlag[ uiCGBlkPos ] == 0)
          {
#if MULTILEVEL_SIGMAP_EXT
            UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiScanIdx, uiWidth, uiHeight);
#else
            UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight);
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
#if MULTILEVEL_SIGMAP_EXT
              UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiScanIdx, uiWidth, uiHeight);
#else
              UInt  uiCtxSig = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY, uiWidth, uiHeight);
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
#if REMOVE_INFER_SIGGRP
        else
        {
          uiSigCoeffGroupFlag[ uiCGBlkPos ] = 1;
        }
#else
        else // if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY ) && (uiCGScanPos != 0) && (uiSigCoeffGroupFlag[ uiCGBlkPos ] != 0) )
        {
          uiSigCoeffGroupFlag[ uiCGBlkPos ] = 1;
        } // end if ( !bothCGNeighboursOne( uiSigCoeffGroupFlag, uiCGPosX, uiCGPosY ) && (uiCGScanPos != 0) && (uiSigCoeffGroupFlag[ uiCGBlkPos ] != 0) )
#endif 
      }
    } //end for (iCGScanPos)
#if !MULTILEVEL_SIGMAP_EXT
  }
#endif

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
    ui16CtxCbf   = ( eTType ? TEXT_CHROMA : eTType ) * NUM_QT_CBF_CTX + ui16CtxCbf;
    d64BestCost  = d64BlockUncodedCost + xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 0 ] );
    d64BaseCost += xGetICost( m_pcEstBitsSbac->blockCbpBits[ ui16CtxCbf ][ 1 ] );
  }

#if !MULTILEVEL_SIGMAP_EXT
  if (blockType < 4)
  {
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
  }
  else //if (uiLog2BlkSize < 4)
  {
#endif
    Bool bFoundLast = false;
    for (Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos--)
    {
      UInt uiCGBlkPos = scanCG[ iCGScanPos ];

      d64BaseCost -= pdCostCoeffGroupSig [ iCGScanPos ]; 
      if (uiSigCoeffGroupFlag[ uiCGBlkPos ])
      {     
        for (Int iScanPosinCG = uiCGSize-1; iScanPosinCG >= 0; iScanPosinCG--)
        {
#if MULTILEVEL_SIGMAP_EXT
          iScanPos = iCGScanPos*uiCGSize + iScanPosinCG;
#else
          Int iScanPos = iCGScanPos*uiCGSize + iScanPosinCG;
#endif
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
#if !MULTILEVEL_SIGMAP_EXT
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

#if MULTIBITS_DATA_HIDING
  if( pcCU->getSlice()->getPPS()->getSignHideFlag() && uiAbsSum>=2)
  {
    Int rdFactor = (Int)((Double)(g_invQuantScales[m_cQP.rem()]*g_invQuantScales[m_cQP.rem()]<<(2*m_cQP.m_iPer))/m_dLambda/16 + 0.5) ;

    Int tsig = pcCU->getSlice()->getPPS()->getTSIG() ;

    Int lastCG = -1;
    Int absSum = 0 ;
    Int n ;

    for( Int subSet = (uiWidth*uiHeight-1) >> LOG2_SCAN_SET_SIZE; subSet >= 0; subSet-- )
    {
      Int  subPos     = subSet << LOG2_SCAN_SET_SIZE;
      Int  firstNZPosInCG=SCAN_SET_SIZE , lastNZPosInCG=-1 ;
      absSum = 0 ;

      for(n = SCAN_SET_SIZE-1; n >= 0; --n )
      {
        if( piDstCoeff[ scan[ n + subPos ]] )
        {
          lastNZPosInCG = n;
          break;
        }
      }

      for(n = 0; n <SCAN_SET_SIZE; n++ )
      {
        if( piDstCoeff[ scan[ n + subPos ]] )
        {
          firstNZPosInCG = n;
          break;
        }
      }

      for(n = firstNZPosInCG; n <=lastNZPosInCG; n++ )
      {
        absSum += piDstCoeff[ scan[ n + subPos ]];
      }

      if(lastNZPosInCG>=0 && lastCG==-1) lastCG =1 ; 
      
      if( lastNZPosInCG-firstNZPosInCG>=tsig )
      {
        UInt signbit = (piDstCoeff[scan[subPos+firstNZPosInCG]]>0?0:1);
        if( signbit!=(absSum&0x1) )  // hide but need tune
        {
          // calculate the cost 
          Int minCostInc = MAX_INT,  minPos =-1, finalChange=0, curCost=MAX_INT, curChange=0;

          for( n = (lastCG==1?lastNZPosInCG:SCAN_SET_SIZE-1) ; n >= 0; --n )
          {
            UInt uiBlkPos   = scan[ n + subPos ];
            if(piDstCoeff[ uiBlkPos ] != 0 )
            {
              Int costUp   = rdFactor * ( - deltaU[uiBlkPos] ) + rateIncUp[uiBlkPos] ;
              Int costDown = rdFactor * (   deltaU[uiBlkPos] ) + rateIncDown[uiBlkPos] 
                -   ( abs(piDstCoeff[uiBlkPos])==1?((1<<15)+sigRateDelta[uiBlkPos]):0 );

              if(lastCG==1 && lastNZPosInCG==n && abs(piDstCoeff[uiBlkPos])==1)
              {
                costDown -= (4<<15) ;
              }

              if(costUp<costDown)
              {  
                curCost = costUp;
                curChange =  1 ;
              }
              else               
              {
                curChange = -1 ;
                if(n==firstNZPosInCG && abs(piDstCoeff[uiBlkPos])==1)
                {
                  curCost = MAX_INT ;
                }
                else
                {
                  curCost = costDown ; 
                }
              }
            }
            else
            {
              curCost = rdFactor * ( - (abs(deltaU[uiBlkPos])) ) + (1<<15) + rateIncUp[uiBlkPos] + sigRateDelta[uiBlkPos] ; 
              curChange = 1 ;

              if(n<firstNZPosInCG)
              {
                UInt thissignbit = (plSrcCoeff[uiBlkPos]>=0?0:1);
                if(thissignbit != signbit )
                {
                  curCost = MAX_INT;
                }
              }
            }

            if( curCost<minCostInc)
            {
              minCostInc = curCost ;
              finalChange = curChange ;
              minPos = uiBlkPos ;
            }
          }

          if(piQCoef[minPos] == 32767 || piQCoef[minPos] == -32768)
          {
            finalChange = -1;
          }

          if(plSrcCoeff[minPos]>=0)
          {
            piDstCoeff[minPos] += finalChange ;
          }
          else
          {
            piDstCoeff[minPos] -= finalChange ; 
          }          
        }
      }

      if(lastCG==1)
      {
        lastCG=0 ;  
      }
    }
  }
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
#if RESTRICT_GR1GR2FLAG_NUMBER
                                            UInt                            c1Idx,
                                            UInt                            c2Idx,
#endif
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
#if RESTRICT_GR1GR2FLAG_NUMBER
    Double dCurrCost    = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice, c1Idx, c2Idx );
#else
    Double dCurrCost    = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, ui16CtxNumOne, ui16CtxNumAbs, ui16AbsGoRice );
#endif
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
#if RESTRICT_GR1GR2FLAG_NUMBER
                                            ,  UInt                            c1Idx,
                                               UInt                            c2Idx
#endif
                                               ) const
{
  Double iRate = xGetIEPRate();
#if RESTRICT_GR1GR2FLAG_NUMBER
  UInt baseLevel  =  (c1Idx < C1FLAG_NUMBER)? (2 + (c2Idx < C2FLAG_NUMBER)) : 1;

  if ( uiAbsLevel >= baseLevel )
  {
    UInt uiSymbol     = uiAbsLevel - baseLevel;
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

    if (c1Idx < C1FLAG_NUMBER)
    {
      iRate += m_pcEstBitsSbac->m_greaterOneBits[ ui16CtxNumOne ][ 1 ];

      if (c2Idx < C2FLAG_NUMBER)
      {
        iRate += m_pcEstBitsSbac->m_levelAbsBits[ ui16CtxNumAbs ][ 1 ];
      }
    }
  }
  else
#endif
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
#if RESTRICT_GR1GR2FLAG_NUMBER
    assert (0);
#else
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
#endif
  }
  return xGetICost( iRate );
}

#if MULTIBITS_DATA_HIDING
__inline Int TComTrQuant::xGetICRate  ( UInt                            uiAbsLevel,
                                       UShort                          ui16CtxNumOne,
                                       UShort                          ui16CtxNumAbs,
                                       UShort                          ui16AbsGoRice
#if RESTRICT_GR1GR2FLAG_NUMBER
                                     , UInt                            c1Idx,
                                       UInt                            c2Idx
#endif
                                       ) const
{
  Int iRate = 0;
#if RESTRICT_GR1GR2FLAG_NUMBER
  UInt baseLevel  =  (c1Idx < C1FLAG_NUMBER)? (2 + (c2Idx < C2FLAG_NUMBER)) : 1;

  if ( uiAbsLevel >= baseLevel )
  {
    UInt uiSymbol     = uiAbsLevel - baseLevel;
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

    if (c1Idx < C1FLAG_NUMBER)
    {
      iRate += m_pcEstBitsSbac->m_greaterOneBits[ ui16CtxNumOne ][ 1 ];

      if (c2Idx < C2FLAG_NUMBER)
      {
        iRate += m_pcEstBitsSbac->m_levelAbsBits[ ui16CtxNumAbs ][ 1 ];
      }
    }
  }
  else
#endif
  if( uiAbsLevel == 0 )
  {
    return 0;
  }
  else if( uiAbsLevel == 1 )
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
#if RESTRICT_GR1GR2FLAG_NUMBER
    assert(0);
#else
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
#endif
  }
  return iRate;
}
#endif

__inline Double TComTrQuant::xGetRateSigCoeffGroup  ( UShort                    uiSignificanceCoeffGroup,
                                                UShort                          ui16CtxNumSig ) const
{
  return xGetICost( m_pcEstBitsSbac->significantCoeffGroupBits[ ui16CtxNumSig ][ uiSignificanceCoeffGroup ] );
}

/** Calculates the cost of signaling the last significant coefficient in the block
 * \param uiPosX X coordinate of the last significant coefficient
 * \param uiPosY Y coordinate of the last significant coefficient
 * \returns cost of last significant coefficient
 */
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
/** set quantized matrix coefficient for encode
 * \param scalingList quantaized matrix address
 */
Void TComTrQuant::setScalingList(TComScalingList *scalingList)
{
  UInt size,list;
  UInt qp;

  for(size=0;size<SCALING_LIST_SIZE_NUM;size++)
  {
    for(list = 0; list < g_scalingListNum[size]; list++)
    {
      for(qp=0;qp<SCALING_LIST_REM_NUM;qp++)
      {
        xSetScalingListEnc(scalingList,list,size,qp);
        xSetScalingListDec(scalingList,list,size,qp);
        setErrScaleCoeff(list,size,qp,SCALING_LIST_SQT);
        if(size == SCALING_LIST_32x32 || size == SCALING_LIST_16x16)
        {
          setErrScaleCoeff(list,size-1,qp,SCALING_LIST_HOR);
          setErrScaleCoeff(list,size-1,qp,SCALING_LIST_VER);
        }
      }
    }
  }
}
/** set quantized matrix coefficient for decode
 * \param scalingList quantaized matrix address
 */
Void TComTrQuant::setScalingListDec(TComScalingList *scalingList)
{
  UInt size,list;
  UInt qp;

  for(size=0;size<SCALING_LIST_SIZE_NUM;size++)
  {
    for(list = 0; list < g_scalingListNum[size]; list++)
    {
      for(qp=0;qp<SCALING_LIST_REM_NUM;qp++)
      {
        xSetScalingListDec(scalingList,list,size,qp);
      }
    }
  }
}
/** set error scale coefficients
 * \param list List ID
 * \param uiSize Size
 * \param uiQP Quantization parameter
 */
Void TComTrQuant::setErrScaleCoeff(UInt list,UInt size, UInt qp, UInt dir)
{

  UInt uiLog2TrSize = g_aucConvertToBit[ g_scalingListSizeX[size] ] + 2;
#if FULL_NBIT
  UInt uiBitDepth = g_uiBitDepth;
#else
  UInt uiBitDepth = g_uiBitDepth + g_uiBitIncrement;  
#endif

  Int iTransformShift = MAX_TR_DYNAMIC_RANGE - uiBitDepth - uiLog2TrSize;  // Represents scaling through forward transform

  UInt i,uiMaxNumCoeff = g_scalingListSize[size];
  Int *piQuantcoeff;
  double *pdErrScale;
  piQuantcoeff   = getQuantCoeff(list, qp,size,dir);
  pdErrScale     = getErrScaleCoeff(list, size, qp,dir);

  double dErrScale = (double)(1<<SCALE_BITS);                              // Compensate for scaling of bitcount in Lagrange cost function
  dErrScale = dErrScale*pow(2.0,-2.0*iTransformShift);                     // Compensate for scaling through forward transform
  for(i=0;i<uiMaxNumCoeff;i++)
  {
    pdErrScale[i] =  dErrScale/(double)piQuantcoeff[i]/(double)piQuantcoeff[i]/(double)(1<<(2*g_uiBitIncrement));
  }
}

/** set quantized matrix coefficient for encode
 * \param scalingList quantaized matrix address
 * \param listId List index
 * \param sizeId size index
 * \param uiQP Quantization parameter
 */
Void TComTrQuant::xSetScalingListEnc(TComScalingList *scalingList, UInt listId, UInt sizeId, UInt qp)
{
  UInt width = g_scalingListSizeX[sizeId];
  UInt height = g_scalingListSizeX[sizeId];
#if SCALING_LIST
  UInt ratio = g_scalingListSizeX[sizeId]/min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]);
#endif
  Int *quantcoeff;
  Int *coeff = scalingList->getScalingListAddress(sizeId,listId);
  quantcoeff   = getQuantCoeff(listId, qp, sizeId, SCALING_LIST_SQT);

#if SCALING_LIST
  processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height,width,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
  processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height,width,1,(Int)g_scalingListSizeX[sizeId],0);
#endif

  if(sizeId == SCALING_LIST_32x32 || sizeId == SCALING_LIST_16x16) //for NSQT
  {
    quantcoeff   = getQuantCoeff(listId, qp, sizeId-1,SCALING_LIST_VER);
#if SCALING_LIST
    processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height,width>>2,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
    processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height,width>>2,1,(Int)g_scalingListSizeX[sizeId],0);
#endif

    quantcoeff   = getQuantCoeff(listId, qp, sizeId-1,SCALING_LIST_HOR);
#if SCALING_LIST
    processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height>>2,width,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
    processScalingListEnc(coeff,quantcoeff,g_quantScales[qp]<<4,height>>2,width,1,(Int)g_scalingListSizeX[sizeId],0);
#endif
  }
}
/** set quantized matrix coefficient for decode
 * \param scalingList quantaized matrix address
 * \param list List index
 * \param size size index
 * \param uiQP Quantization parameter
 */
Void TComTrQuant::xSetScalingListDec(TComScalingList *scalingList, UInt listId, UInt sizeId, UInt qp)
{
  UInt width = g_scalingListSizeX[sizeId];
  UInt height = g_scalingListSizeX[sizeId];
#if SCALING_LIST
  UInt ratio = g_scalingListSizeX[sizeId]/min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]);
#endif
  Int *dequantcoeff;
  Int *coeff = scalingList->getScalingListAddress(sizeId,listId);

  dequantcoeff = getDequantCoeff(listId, qp, sizeId,SCALING_LIST_SQT);
#if SCALING_LIST
  processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height,width,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
  processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height,width,1,(Int)g_scalingListSizeX[sizeId],0);
#endif

  if(sizeId == SCALING_LIST_32x32 || sizeId == SCALING_LIST_16x16)
  {
    dequantcoeff   = getDequantCoeff(listId, qp, sizeId-1,SCALING_LIST_VER);
#if SCALING_LIST
    processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height,width>>2,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
    processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height,width>>2,1,(Int)g_scalingListSizeX[sizeId],0);
#endif

    dequantcoeff   = getDequantCoeff(listId, qp, sizeId-1,SCALING_LIST_HOR);

#if SCALING_LIST
    processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height>>2,width,ratio,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),scalingList->getScalingListDC(sizeId,listId));
#else
    processScalingListDec(coeff,dequantcoeff,g_invQuantScales[qp],height>>2,width,1,min(MAX_MATRIX_SIZE_NUM,(Int)g_scalingListSizeX[sizeId]),0);
#endif
  }
}

/** set flat matrix value to quantized coefficient
 */
Void TComTrQuant::setFlatScalingList()
{
  UInt size,list;
  UInt qp;

  for(size=0;size<SCALING_LIST_SIZE_NUM;size++)
  {
    for(list = 0; list <  g_scalingListNum[size]; list++)
    {
      for(qp=0;qp<SCALING_LIST_REM_NUM;qp++)
      {
        xsetFlatScalingList(list,size,qp);
        setErrScaleCoeff(list,size,qp,SCALING_LIST_SQT);
        if(size == SCALING_LIST_32x32 || size == SCALING_LIST_16x16)
        {
          setErrScaleCoeff(list,size-1,qp,SCALING_LIST_HOR);
          setErrScaleCoeff(list,size-1,qp,SCALING_LIST_VER);
        }
      }
    }
  }
}

/** set flat matrix value to quantized coefficient
 * \param list List ID
 * \param uiQP Quantization parameter
 * \param uiSize Size
 */
Void TComTrQuant::xsetFlatScalingList(UInt list, UInt size, UInt qp)
{
  UInt i,num = g_scalingListSize[size];
  UInt numDiv4 = num>>2;
  Int *quantcoeff;
  Int *dequantcoeff;
  Int quantScales = g_quantScales[qp];
  Int invQuantScales = g_invQuantScales[qp]<<4;

  quantcoeff   = getQuantCoeff(list, qp, size,SCALING_LIST_SQT);
  dequantcoeff = getDequantCoeff(list, qp, size,SCALING_LIST_SQT);

  for(i=0;i<num;i++)
  { 
    *quantcoeff++ = quantScales;
    *dequantcoeff++ = invQuantScales;
  }

  if(size == SCALING_LIST_32x32 || size == SCALING_LIST_16x16)
  {
    quantcoeff   = getQuantCoeff(list, qp, size-1, SCALING_LIST_HOR);
    dequantcoeff = getDequantCoeff(list, qp, size-1, SCALING_LIST_HOR);

    for(i=0;i<numDiv4;i++)
    {
      *quantcoeff++ = quantScales;
      *dequantcoeff++ = invQuantScales;
    }
    quantcoeff   = getQuantCoeff(list, qp, size-1 ,SCALING_LIST_VER);
    dequantcoeff = getDequantCoeff(list, qp, size-1 ,SCALING_LIST_VER);

    for(i=0;i<numDiv4;i++)
    {
      *quantcoeff++ = quantScales;
      *dequantcoeff++ = invQuantScales;
    }
  }
}

/** set quantized matrix coefficient for encode
 * \param coeff quantaized matrix address
 * \param quantcoeff quantaized matrix address
 * \param quantScales Q(QP%6)
 * \param height height
 * \param width width
 * \param ratio ratio for upscale
 * \param sizuNum matrix size
 * \param dc dc parameter
 */
Void TComTrQuant::processScalingListEnc( Int *coeff, Int *quantcoeff, Int quantScales, UInt height, UInt width, UInt ratio, Int sizuNum, UInt dc)
{
  Int nsqth = (height < width) ? 4: 1; //height ratio for NSQT
  Int nsqtw  = (width < height) ? 4: 1; //width ratio for NSQT
  for(UInt j=0;j<height;j++)
  {
    for(UInt i=0;i<width;i++)
    {
      quantcoeff[j*width + i] = quantScales / coeff[sizuNum * (j * nsqth / ratio) + i * nsqtw /ratio];
    }
  }
#if SCALING_LIST
  if(ratio > 1)
  {
    quantcoeff[0] = quantScales / dc;
  }
#endif
}
/** set quantized matrix coefficient for decode
 * \param coeff quantaized matrix address
 * \param dequantcoeff quantaized matrix address
 * \param invQuantScales IQ(QP%6))
 * \param height height
 * \param width width
 * \param ratio ratio for upscale
 * \param sizuNum matrix size
 * \param dc dc parameter
 */
Void TComTrQuant::processScalingListDec( Int *coeff, Int *dequantcoeff, Int invQuantScales, UInt height, UInt width, UInt ratio, Int sizuNum, UInt dc)
{
  Int nsqth = (height < width) ? 4: 1; //height ratio for NSQT
  Int nsqtw  = (width < height) ? 4: 1; //width ratio for NSQT
  for(UInt j=0;j<height;j++)
  {
    for(UInt i=0;i<width;i++)
    {
      dequantcoeff[j*width + i] = invQuantScales * coeff[sizuNum * (j * nsqth / ratio) + i * nsqtw /ratio];
    }
  }
#if SCALING_LIST
  if(ratio > 1)
  {
    dequantcoeff[0] = invQuantScales * dc;
  }
#endif
}

/** initialization process of scaling list array
 */
Void TComTrQuant::initScalingList()
{
  for(UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
  {
    for(UInt listId = 0; listId < g_scalingListNum[sizeId]; listId++)
    {
      for(UInt qp = 0; qp < SCALING_LIST_REM_NUM; qp++)
      {
        m_quantCoef   [sizeId][listId][qp][SCALING_LIST_SQT] = new Int [g_scalingListSize[sizeId]];
        m_dequantCoef [sizeId][listId][qp][SCALING_LIST_SQT] = new Int [g_scalingListSize[sizeId]];
        m_errScale    [sizeId][listId][qp][SCALING_LIST_SQT] = new double [g_scalingListSize[sizeId]];
        
        if(sizeId == SCALING_LIST_8x8 || (sizeId == SCALING_LIST_16x16 && listId < 2))
        {
          for(UInt dir = SCALING_LIST_VER; dir < SCALING_LIST_DIR_NUM; dir++)
          {
            m_quantCoef   [sizeId][listId][qp][dir] = new Int [g_scalingListSize[sizeId]];
            m_dequantCoef [sizeId][listId][qp][dir] = new Int [g_scalingListSize[sizeId]];
            m_errScale    [sizeId][listId][qp][dir] = new double [g_scalingListSize[sizeId]];
          }
        }
      }
    }
  }
  //copy for NSQT
  for(UInt qp = 0; qp < SCALING_LIST_REM_NUM; qp++)
  {
    for(UInt dir = SCALING_LIST_VER; dir < SCALING_LIST_DIR_NUM; dir++)
    {
      m_quantCoef   [SCALING_LIST_16x16][3][qp][dir] = m_quantCoef   [SCALING_LIST_16x16][1][qp][dir];
      m_dequantCoef [SCALING_LIST_16x16][3][qp][dir] = m_dequantCoef [SCALING_LIST_16x16][1][qp][dir];
      m_errScale    [SCALING_LIST_16x16][3][qp][dir] = m_errScale    [SCALING_LIST_16x16][1][qp][dir];
    }
    m_quantCoef   [SCALING_LIST_32x32][3][qp][SCALING_LIST_SQT] = m_quantCoef   [SCALING_LIST_32x32][1][qp][SCALING_LIST_SQT];
    m_dequantCoef [SCALING_LIST_32x32][3][qp][SCALING_LIST_SQT] = m_dequantCoef [SCALING_LIST_32x32][1][qp][SCALING_LIST_SQT];
    m_errScale    [SCALING_LIST_32x32][3][qp][SCALING_LIST_SQT] = m_errScale    [SCALING_LIST_32x32][1][qp][SCALING_LIST_SQT];
  }
}
/** destroy quantization matrix array
 */
Void TComTrQuant::destroyScalingList()
{
  for(UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
  {
    for(UInt listId = 0; listId < g_scalingListNum[sizeId]; listId++)
    {
      for(UInt qp = 0; qp < SCALING_LIST_REM_NUM; qp++)
      {
        if(m_quantCoef   [sizeId][listId][qp][SCALING_LIST_SQT]) delete [] m_quantCoef   [sizeId][listId][qp][SCALING_LIST_SQT];
        if(m_dequantCoef [sizeId][listId][qp][SCALING_LIST_SQT]) delete [] m_dequantCoef [sizeId][listId][qp][SCALING_LIST_SQT];
        if(m_errScale    [sizeId][listId][qp][SCALING_LIST_SQT]) delete [] m_errScale    [sizeId][listId][qp][SCALING_LIST_SQT];
        if(sizeId == SCALING_LIST_8x8 || (sizeId == SCALING_LIST_16x16 && listId < 2))
        {
          for(UInt dir = SCALING_LIST_VER; dir < SCALING_LIST_DIR_NUM; dir++)
          {
            if(m_quantCoef   [sizeId][listId][qp][dir]) delete [] m_quantCoef   [sizeId][listId][qp][dir];
            if(m_dequantCoef [sizeId][listId][qp][dir]) delete [] m_dequantCoef [sizeId][listId][qp][dir];
            if(m_errScale    [sizeId][listId][qp][dir]) delete [] m_errScale    [sizeId][listId][qp][dir];
          }
        }
      }
    }
  }
}

//! \}
