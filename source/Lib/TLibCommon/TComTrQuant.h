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

/** \file     TComTrQuant.h
    \brief    transform and quantization class (header)
*/

#ifndef __TCOMTRQUANT__
#define __TCOMTRQUANT__

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"
#include "ContextTables.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define QP_BITS                 15

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
#if SIMPLE_CONTEXT_SIG
  Int significantBits[NUM_SIG_FLAG_CTX][2];
#else
  Int significantBits[16][2];
#endif
#if PCP_SIGMAP_SIMPLE_LAST
  Int lastXBits[32];
  Int lastYBits[32];
#else  
  Int lastBits[16][2];
#endif
  Int m_greaterOneBits[NUM_ONE_FLAG_CTX][2];
  Int m_levelAbsBits[NUM_ABS_FLAG_CTX][2];

  Int blockCbpBits[3*NUM_QT_CBF_CTX][2];
  Int blockRootCbpBits[4][2];
  Int scanZigzag[2];            ///< flag for zigzag scan
  Int scanNonZigzag[2];         ///< flag for non zigzag scan
} estBitsSbacStruct;

typedef struct
{
  Int level[4];
  Int pre_level;
  Int coeff_ctr;
  Long levelDouble;
  Double errLevel[4];
  Int noLevels;
  Long levelQ;
  Bool lowerInt;
  UInt quantInd;
} levelDataStruct;

typedef struct
{
  Int run;
  Int maxrun;
  Int nextLev;
  Int nexLevelVal;
} quantLevelStruct;



class TEncCavlc;

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
    m_iRem  = (iQP + 6*g_uiBitIncrement)%6;
    
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
  Void init                 ( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Int iSymbolMode = 0, UInt *aTable4 = NULL, UInt *aTable8 = NULL, UInt *aTableLastPosVlcIndex=NULL, Bool bUseRDOQ = false,  Bool bEnc = false );
  
  // transform & inverse transform functions
  Void transformNxN         ( TComDataCU* pcCU, Pel*   pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight,
                             UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx );
#if INTRA_DST_TYPE_7
  Void invtransformNxN      (TextType eText, UInt uiMode,Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight);
#else
  Void invtransformNxN      ( Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight );
#endif
  Void invRecurTransformNxN ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr,   UInt uiStride, UInt uiWidth, UInt uiHeight,
                             UInt uiMaxTrMode,  UInt uiTrMode, TCoeff* rpcCoeff );
  
  // Misc functions
  Void setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType);
  Void setLambda(Double dLambda) { m_dLambda = dLambda;}
  Void setRDOQOffset( UInt uiRDOQOffset ) { m_uiRDOQOffset = uiRDOQOffset; }
  
  estBitsSbacStruct* m_pcEstBitsSbac;
  
  static UInt     getSigCtxInc     ( TCoeff*                         pcCoeff,
                                     const UInt                      uiPosX,
                                     const UInt                      uiPosY,
                                     const UInt                      uiLog2BlkSize,
                                     const UInt                      uiStride );
#if !PCP_SIGMAP_SIMPLE_LAST
  static UInt     getLastCtxInc    ( const UInt                      uiPosX,
                                     const UInt                      uiPosY,
                                     const UInt                      uiLog2BlkSize );
#endif
protected:
  Long*    m_plTempCoeff;
  
  QpParam  m_cQP;
  Double   m_dLambda;
  UInt     m_uiRDOQOffset;
  UInt     m_uiMaxTrSize;
  Bool     m_bEnc;
  Bool     m_bUseRDOQ;
  
  UInt     *m_uiLPTableE8;
  UInt     *m_uiLPTableE4;
  Int      m_iSymbolMode;
  UInt     *m_uiLastPosVlcIndex;
  
private:
  // forward Transform
#if INTRA_DST_TYPE_7
  Void xT   ( UInt uiMode,Pel* pResidual, UInt uiStride, Long* plCoeff, Int iSize );
#else
  Void xT   ( Pel* pResidual, UInt uiStride, Long* plCoeff, Int iSize );
#endif
  
  // quantization
  Void xQuant     ( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx );
  Void xQuantLTR  ( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx );

  // RDOQ functions

  Int            xCodeCoeffCountBitsLast(TCoeff* scoeff, levelDataStruct* levelData, Int nTab, UInt uiNoCoeff);
  UInt           xCountVlcBits(UInt uiTableNumber, UInt uiCodeNumber);
#if CAVLC_COEF_LRG_BLK
  Int            bitCountRDOQ(Int coeff, Int pos, Int nTab, Int lastCoeffFlag,Int levelMode,Int run, Int maxrun, Int vlc_adaptive, Int N, 
                              UInt uiTr1, Int iSum_big_coef, Int iBlockType, TComDataCU* pcCU, const UInt **pLumaRunTr1);
#else
  Int            bitCountRDOQ(Int coeff, Int pos, Int nTab, Int lastCoeffFlag,Int levelMode,Int run, Int maxrun, Int vlc_adaptive, Int N, 
                              UInt uiTr1, Int iSum_big_coef, Int iBlockType, TComDataCU* pcCU);
#endif
#if QC_MDCS
UInt             getCurrLineNum(UInt uiScanIdx, UInt uiPosX, UInt uiPosY);
#endif
  Void           xRateDistOptQuant_LCEC ( TComDataCU*                     pcCU,
                                          Long*                           plSrcCoeff,
                                          TCoeff*&                        piDstCoeff,
                                          UInt                            uiWidth,
                                          UInt                            uiHeight,
                                          UInt&                           uiAbsSum,
                                          TextType                        eTType,
                                          UInt                            uiAbsPartIdx );
  
  Void           xRateDistOptQuant ( TComDataCU*                     pcCU,
                                     Long*                           plSrcCoeff,
                                     TCoeff*&                        piDstCoeff,
                                     UInt                            uiWidth,
                                     UInt                            uiHeight,
                                     UInt&                           uiAbsSum,
                                     TextType                        eTType,
                                     UInt                            uiAbsPartIdx );
  __inline UInt  xGetCodedLevel    ( Double&                         rd64UncodedCost,
                                     Double&                         rd64CodedCost,
#if PCP_SIGMAP_SIMPLE_LAST
                                     Double&                         rd64CodedLastCost,
                                     UInt&                           ruiBestNonZeroLevel,
                                     Long                            lLevelDouble,
                                     UInt                            uiMaxAbsLevel,
#else
                                     Long                            lLevelDouble,
                                     UInt                            uiMaxAbsLevel,
                                     bool                            bLastScanPos,
#endif
                                     UShort                          ui16CtxNumSig,
                                     UShort                          ui16CtxNumOne,
                                     UShort                          ui16CtxNumAbs,
#if E253
                                     UShort                          ui16AbsGoRice,
#endif
                                     Int                             iQBits,
                                     Double                          dTemp
                                    ) const;
  __inline Double xGetICRateCost   ( UInt                            uiAbsLevel,
#if !PCP_SIGMAP_SIMPLE_LAST
                                     Bool                            bLastScanPos,
                                     UShort                          ui16CtxNumSig,
#endif
                                     UShort                          ui16CtxNumOne,
                                     UShort                          ui16CtxNumAbs
#if E253
                                     ,UShort                          ui16AbsGoRice
#endif
                                     ) const;
#if PCP_SIGMAP_SIMPLE_LAST
  __inline Double xGetRateLast     ( UInt                            uiPosX,
                                     UInt                            uiPosY        ) const;
  __inline Double xGetRateSigCoef (  UShort                          uiSignificance,
                                     UShort                          ui16CtxNumSig ) const;
#endif
  __inline Double xGetICost        ( Double                          dRate         ) const; 
  __inline Double xGetIEPRate      (                                               ) const;
  
  
  // dequantization
  Void xDeQuant         ( TCoeff* pSrc,     Long*& pDes,       Int iWidth, Int iHeight );
  Void xDeQuantLTR      ( TCoeff* pSrc,     Long*&  pDes,      Int iWidth, Int iHeight );
  
  // inverse transform
#if INTRA_DST_TYPE_7
  Void xIT    ( UInt uiMode, Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize );
#else
  Void xIT    ( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize );
#endif
  
};// END CLASS DEFINITION TComTrQuant


#endif // __TCOMTRQUANT__

