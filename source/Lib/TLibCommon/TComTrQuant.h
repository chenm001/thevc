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

/** \file     TComTrQuant.h
    \brief    transform and quantization class (header)
*/

#ifndef __TCOMTRQUANT__
#define __TCOMTRQUANT__

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"
#include "ContextTables.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define QP_BITS                 15

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
#if MULTI_LEVEL_SIGNIFICANCE
  Int significantCoeffGroupBits[NUM_SIG_CG_FLAG_CTX][2];
#endif
  Int significantBits[NUM_SIG_FLAG_CTX][2];
  Int lastXBits[32];
  Int lastYBits[32];
  Int m_greaterOneBits[NUM_ONE_FLAG_CTX][2];
  Int m_levelAbsBits[NUM_ABS_FLAG_CTX][2];

  Int blockCbpBits[3*NUM_QT_CBF_CTX][2];
  Int blockRootCbpBits[4][2];
  Int scanZigzag[2];            ///< flag for zigzag scan
  Int scanNonZigzag[2];         ///< flag for non zigzag scan
} estBitsSbacStruct;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// QP class
class QpParam
{
public:
  QpParam();
  
  Int m_iQP;
  Int m_iPer;
  Int m_iRem;
  
public:
  Int m_iBits;
    
  Void setQpParam( Int iQP, Bool bLowpass, SliceType eSliceType )
  {
    assert ( iQP >= MIN_QP && iQP <= MAX_QP );
    m_iQP   = iQP;
    
    m_iPer  = (iQP + 6*g_uiBitIncrement)/6;
#if FULL_NBIT
    m_iPer += g_uiBitDepth - 8;
#endif
    m_iRem  = iQP % 6;
    
    m_iBits = QP_BITS + m_iPer;
  }
  
  Void clear()
  {
    m_iQP   = 0;
    m_iPer  = 0;
    m_iRem  = 0;
    m_iBits = 0;
  }
  
  
  Int per()   const { return m_iPer; }
  Int rem()   const { return m_iRem; }
  Int bits()  const { return m_iBits; }
  
  Int qp() {return m_iQP;}
}; // END CLASS DEFINITION QpParam

/// transform and quantization class
class TComTrQuant
{
public:
  TComTrQuant();
  ~TComTrQuant();
  
  // initialize class
  Void init                 ( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, UInt *aTable4 = NULL, UInt *aTable8 = NULL, UInt *aTableLastPosVlcIndex=NULL, Bool bEnc = false
#if ADAPTIVE_QP_SELECTION
                       , Bool bUseAdaptQpSelect = false
#endif    
    );
  
  // transform & inverse transform functions
  Void transformNxN( TComDataCU* pcCU, 
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
                     UInt        uiAbsPartIdx );
  Void invtransformNxN      (TextType eText, UInt uiMode,Pel* rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight);
  Void invRecurTransformNxN ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel* rpcResidual, UInt uiAddr,   UInt uiStride, UInt uiWidth, UInt uiHeight,
                             UInt uiMaxTrMode,  UInt uiTrMode, TCoeff* rpcCoeff );
  
  // Misc functions
#if G509_CHROMA_QP_OFFSET
  Void setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType, Int Shift);
#else
  Void setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType);
#endif
#if RDOQ_CHROMA_LAMBDA 
  Void setLambda(Double dLambdaLuma, Double dLambdaChroma) { m_dLambdaLuma = dLambdaLuma; m_dLambdaChroma = dLambdaChroma; }
  Void selectLambda(TextType eTType) { m_dLambda = (eTType == TEXT_LUMA) ? m_dLambdaLuma : m_dLambdaChroma; }
#else
  Void setLambda(Double dLambda) { m_dLambda = dLambda;}
#endif
  
  static Int      getSigCtxInc     ( TCoeff*                         pcCoeff,
                                     Int                             posX,
                                     Int                             posY,
                                     Int                             blockType,
                                     Int                             width
#if NSQT_DIAG_SCAN
                                    ,Int                             height
#endif
#if SIGMAP_CTX_RED
                                    ,TextType                        textureType
#endif
                                    );
#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  static UInt getSigCoeffGroupCtxInc  ( const UInt*                   uiSigCoeffGroupFlag,
                                       const UInt                       uiCGPosX,
                                       const UInt                       uiCGPosY,
                                       Int width, Int height);
  
  static Bool bothCGNeighboursOne  ( const UInt*                      uiSigCoeffGroupFlag,
                                    const UInt                       uiCGPosX,
                                    const UInt                       uiCGPosY,
                                    Int width, Int height);
#else
  static UInt getSigCoeffGroupCtxInc  ( const UInt*                   uiSigCoeffGroupFlag,
                                     const UInt                       uiCGPosX,
                                     const UInt                       uiCGPosY,
                                     const UInt                       uiLog2BlockSize);

  static Bool bothCGNeighboursOne  ( const UInt*                      uiSigCoeffGroupFlag,
                                     const UInt                       uiCGPosX,
                                     const UInt                       uiCGPosY,
                                     const UInt                       uiLog2BlockSize);
#endif
#endif
#if ADAPTIVE_QP_SELECTION
  Void    initSliceQpDelta() ;
  Void    storeSliceQpNext(TComSlice* pcSlice);
  Void    clearSliceARLCnt();
  Int     getQpDelta(Int qp) { return m_qpDelta[qp]; } 
  Int*    getSliceNSamples(){ return m_sliceNsamples ;} 
  Double* getSliceSumC()    { return m_sliceSumC; }
#endif
protected:
#if ADAPTIVE_QP_SELECTION
  Int     m_qpDelta[MAX_QP+1]; 
  Int     m_sliceNsamples[LEVEL_RANGE+1];  
  Double  m_sliceSumC[LEVEL_RANGE+1] ;  
#endif
  Int*    m_plTempCoeff;
  
  QpParam  m_cQP;
#if RDOQ_CHROMA_LAMBDA
  Double   m_dLambdaLuma;
  Double   m_dLambdaChroma;
#endif
  Double   m_dLambda;
  UInt     m_uiMaxTrSize;
  Bool     m_bEnc;
#if ADAPTIVE_QP_SELECTION
  Bool     m_bUseAdaptQpSelect;
#endif

private:
  // forward Transform
#if NSQT
  Void xT   ( UInt uiMode,Pel* pResidual, UInt uiStride, Int* plCoeff, Int iWidth, Int iHeight );
#else
  Void xT   ( UInt uiMode,Pel* pResidual, UInt uiStride, Int* plCoeff, Int iSize );
#endif
  
  // quantization
  Void xQuant( TComDataCU* pcCU, 
               Int*        pSrc, 
               TCoeff*     pDes, 
#if ADAPTIVE_QP_SELECTION
               Int*&       pArlDes,
#endif
               Int         iWidth, 
               Int         iHeight, 
               UInt&       uiAcSum, 
               TextType    eTType, 
               UInt        uiAbsPartIdx );

  // dequantization
  Void xDeQuant( const TCoeff* pSrc,     Int* pDes,       Int iWidth, Int iHeight );
  
  // inverse transform
#if NSQT
  Void xIT    ( UInt uiMode, Int* plCoef, Pel* pResidual, UInt uiStride, Int iWidth, Int iHeight );
#else
  Void xIT    ( UInt uiMode, Int* plCoef, Pel* pResidual, UInt uiStride, Int iSize );
#endif
  
};// END CLASS DEFINITION TComTrQuant

//! \}

#endif // __TCOMTRQUANT__
