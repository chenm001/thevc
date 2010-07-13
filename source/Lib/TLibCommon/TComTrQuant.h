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

/** \file     TComTrQuant.h
    \brief    transform and quantization class (header)
*/

#ifndef __TCOMTRQUANT__
#define __TCOMTRQUANT__

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_MAX_PREC_COEFF     25          ///< max. precision of coefficient in RDOQ
#define QP_BITS                 15

// AQO Parameter
#define QOFFSET_BITS            15
#define QOFFSET_BITS_LTR        9

// LTR Butterfly Paramter
#define ECore16Shift            10
#define DCore16Shift            10
#define ECore32Shift            10
#define DCore32Shift            10
#define ECore64Shift            9
#define DCore64Shift            9

#define DenShift16              6
#define DenShift32              8
#define DenShift64              10

// ====================================================================================================================
// Type definition
// ====================================================================================================================

#if HHI_TRANSFORM_CODING
typedef struct
{
  Int significantBits[16][2];
  Int lastBits[16][2];
  Int greaterOneBits[6][2][5][2];
  Int greaterOneState[5];
  Int blockCbpBits[4][2];
  Int scanZigzag[2];            ///< flag for zigzag scan
  Int scanNonZigzag[2];         ///< flag for non zigzag scan
} estBitsSbacStruct;
#else
typedef struct
{
  Int significantBits[16][2];
  Int lastBits[16][2];
  Int greaterOneBits[2][5][2];
  Int greaterOneState[5];
  Int blockCbpBits[4][2];
  Int scanZigzag[2];            ///< flag for zigzag scan
  Int scanNonZigzag[2];         ///< flag for non zigzag scan
} estBitsSbacStruct;
#endif

typedef struct
{
  Int level[3];
  Int pre_level;
  Int coeff_ctr;
#if QC_MDDT
  Int64 levelDouble;
#else
  Long levelDouble;
#endif
  Double errLevel[3];
  Int noLevels;
  Long levelQ;
  Bool lowerInt;
} levelDataStruct;

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

  Int m_iAdd2x2;
  Int m_iAdd4x4;
  Int m_iAdd8x8;
  Int m_iAdd16x16;
  Int m_iAdd32x32;
  Int m_iAdd64x64;
  Int m_iAddNxN;
private:
  Int m_aiAdd2x2[MAX_QP+1][3];
  Int m_aiAdd4x4[MAX_QP+1][3];
  Int m_aiAdd8x8[MAX_QP+1][3];
  Int m_aiAdd16x16[MAX_QP+1][3];
  Int m_aiAdd32x32[MAX_QP+1][3];
  Int m_aiAdd64x64[MAX_QP+1][3];
  Int m_aiAddNxN[MAX_QP+1][3];
public:
  Int m_iBits;

  Void initOffsetParam(Int iStartQP = MIN_QP, Int iEndQP = MAX_QP );
  Void setQOffset( Int iQP, SliceType eSliceType )
  {
    m_iAdd2x2 = m_aiAdd2x2[iQP][eSliceType];
    m_iAdd4x4 = m_aiAdd4x4[iQP][eSliceType];
    m_iAdd8x8 = m_aiAdd8x8[iQP][eSliceType];
    m_iAdd16x16 = m_aiAdd16x16[iQP][eSliceType];
    m_iAdd32x32 = m_aiAdd32x32[iQP][eSliceType];
    m_iAdd64x64 = m_aiAdd64x64[iQP][eSliceType];
    m_iAddNxN = m_aiAddNxN[iQP][eSliceType];
  }

  Void setQpParam( Int iQP, Bool bLowpass, SliceType eSliceType, Bool bEnc )
  {
    assert ( iQP >= MIN_QP && iQP <= MAX_QP );
    m_iQP   = iQP;

    m_iPer  = (iQP + 6*g_uiBitIncrement)/6;
    m_iRem  = (iQP + 6*g_uiBitIncrement)%6;

    m_iBits = QP_BITS + m_iPer;

    if ( bEnc )
    {
      setQOffset(iQP, eSliceType);
    }
  }

  Void clear()
  {
    m_iQP   = 0;
    m_iPer  = 0;
    m_iRem  = 0;
    m_iBits = 0;
  }


  const Int per()   const { return m_iPer; }
  const Int rem()   const { return m_iRem; }
  const Int bits()  const { return m_iBits; }

  Int qp() {return m_iQP;}
}; // END CLASS DEFINITION QpParam

/// transform and quantization class
class TComTrQuant
{
public:
  TComTrQuant();
  ~TComTrQuant();

  // initialize class
#if HHI_ALLOW_ROT_SWITCH
  Void init                 ( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Bool bUseROT, Bool bUseRDOQ = false, Bool bEnc = false );
#else
  Void init                 ( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Bool bUseRDOQ = false, Bool bEnc = false );
#endif

  // transform & inverse transform functions
  Void transformNxN         ( TComDataCU* pcCU, Pel*   pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight,
                              UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT = 0 );
#if QC_MDDT
  Void invtransformNxN      ( TextType eText, UInt uiMode, Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight,
															UChar indexROT = 0 );
#else
  Void invtransformNxN      ( Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight,
                              UChar indexROT = 0 );
#endif
  Void invRecurTransformNxN ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr,   UInt uiStride, UInt uiWidth, UInt uiHeight,
                              UInt uiMaxTrMode,  UInt uiTrMode, TCoeff* rpcCoeff, Int indexROT = 0);

  // ROT functions
  Void RotTransform4I     ( Long* matrix, UChar index   );
  Void InvRotTransform4I  ( Long* matrix, UChar index   );
  Void RotTransformLI2      ( Long* matrix, UChar index, UInt uiWidth );
  Void InvRotTransformLI2 ( Long* matrix, UChar index   );

  // Misc functions
  Void setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType);
  Void setLambda(Double dLambda) { m_dLambda = dLambda;}

  estBitsSbacStruct* m_pcEstBitsSbac;

#if HHI_TRANSFORM_CODING
  static UInt     getSigCtxInc     ( TCoeff*                         pcCoeff,
                                     const UInt                      uiPosX,
                                     const UInt                      uiPosY,
                                     const UInt                      uiLog2BlkSize,
                                     const UInt                      uiStride,
                                     const bool                      bDownLeft );
  static UInt     getLastCtxInc    ( const UInt                      uiPosX,
                                     const UInt                      uiPosY,
                                     const UInt                      uiLog2BlkSize );
#else
  Void precalculateUnaryExpGolombLevel();
  Int m_aiPrecalcUnaryLevelTab[128][RDOQ_MAX_PREC_COEFF];
#endif


#if QC_MDDT
  Bool     m_bQT;
#endif
protected:
  Long*    m_plTempCoeff;
  UInt*    m_puiQuantMtx;

  QpParam  m_cQP;
  Double   m_dLambda;

  UInt     m_uiMaxTrSize;
#if HHI_ALLOW_ROT_SWITCH
	Bool		 m_bUseROT;
#endif
  Bool     m_bEnc;
  Bool     m_bUseRDOQ;

private:
  // forward Transform
#if QC_MDDT
#if ROT_CHECK 
  Bool getUseMDDT (UInt uiMode, UChar indexROT)         { return ( (uiMode != REG_DCT && uiMode != 2 && indexROT == 0) ); }
#else
  Bool getUseMDDT (UInt uiMode, UChar indexROT)         { return ( uiMode != REG_DCT && uiMode != 2 ); }
#endif

  Void xQuantKLT( Long* pSrcCoeff, TCoeff*& pDstCoeff, UInt& uiAbsSum, UInt uiWidth, UInt uiHeight);
  Void xT4_klt	( UInt uiMode, Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT8_klt	( UInt uiMode, Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT		( TextType eText, UInt uiMode, Pel* pResidual, UInt uiStride, Long* plCoeff, Int iSize, Int indexROT );
#else
  Void xT   ( Pel* pResidual, UInt uiStride, Long* plCoeff, Int iSize );
#endif
  Void xT2  ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT4  ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT8  ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT16 ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT32 ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT64 ( Pel* pResidual, UInt uiStride, Long* plCoeff );

  // quantization
  Void xQuant     ( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
  Void xQuantLTR  ( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
  Void xQuant2x2  ( Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, UChar indexROT );
  Void xQuant4x4  ( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
  Void xQuant8x8  ( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );

  // RDOQ functions
#if HHI_TRANSFORM_CODING
  Void           xRateDistOptQuant ( TComDataCU*                     pcCU,
                                     Long*                           plSrcCoeff,
                                     TCoeff*&                        piDstCoeff,
                                     UInt                            uiWidth,
                                     UInt                            uiHeight,
                                     UInt&                           uiAbsSum,
                                     TextType                        eTType,
                                     UInt                            uiAbsPartIdx,
                                     UChar                           ucIndexROT    );
  __inline UInt  xGetCodedLevel    ( Double&                         rd64UncodedCost,
                                     Double&                         rd64CodedCost,
#if QC_MDDT
                                     Int64                           lLevelDouble,
#else
                                     Long                            lLevelDouble,
#endif
                                     UInt                            uiMaxAbsLevel,
                                     bool                            bLastScanPos,
                                     UShort                          ui16CtxNumSig,
                                     UShort                          ui16CtxNumOne,
                                     UShort                          ui16CtxNumAbs,
                                     Int                             iQBits,
                                     Double                          dTemp,
                                     UChar                           ucIndexROT,
                                     UShort                          ui16CtxBase   ) const;
  __inline Double xGetICRateCost   ( UInt                            uiAbsLevel,
                                     bool                            bLastScanPos,
                                     UShort                          ui16CtxNumSig,
                                     UShort                          ui16CtxNumOne,
                                     UShort                          ui16CtxNumAbs,
                                     UShort                          ui16CtxBase   ) const;
  __inline Double xGetICost        ( Double                          dRate         ) const; 
  __inline Double xGetIEPRate      (                                               ) const;
#else
  Void xRateDistOptQuant ( TComDataCU* pcCU, Long* pSrcCoef, TCoeff*& pDstCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
#if QC_MDDT//ADAPTIVE_SCAN
  Double xEst_writeRunLevel_SBAC ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiMode, levelDataStruct* levelData, Int* levelTabMin, TextType eTType, Double lambda, Int& kInit, Int kStop, Int noCoeff, Int estCBP, UInt uiWidth, UInt uiHeight, UInt uiDepth, UChar indexROT );
#else  
  Double xEst_writeRunLevel_SBAC ( levelDataStruct* levelData, Int* levelTabMin, TextType eTType, Double lambda, Int& kInit, Int kStop, Int noCoeff, Int estCBP, UInt uiWidth, UInt uiHeight, UInt uiDepth );
#endif
  Int  xEst_write_and_store_CBP_block_bit ( TComDataCU* pcCU, TextType eTType );
  Int  est_unary_exp_golomb_level_encode (UInt symbol, Int ctx, TextType eTType, UInt uiDepth);
  Int  est_exp_golomb_encode_eq_prob (UInt symbol);
  Int  est_unary_exp_golomb_level_bits( UInt symbol, Int bits0, Int bits1);
#endif


  __inline Int          xRound   ( Int i )   { return ((i)+(1<<5))>>6; }
  __inline static Long  xTrRound ( Long i, UInt uiShift ) { return ((i)>>uiShift); }

  // dequantization
#if QC_MDDT
  Void xDeQuant			( TextType eText, UInt uiMode, TCoeff* pSrc,			Long*& pDes,			 Int iWidth, Int iHeight, UChar indexROT );
  Void xDeQuantLTR		( TextType eText, UInt uiMode, TCoeff* pSrc,			Long*&  pDes,			 Int iWidth, Int iHeight, UChar indexROT );
#else
  Void xDeQuant         ( TCoeff* pSrc,     Long*& pDes,       Int iWidth, Int iHeight, UChar indexROT );
  Void xDeQuantLTR      ( TCoeff* pSrc,     Long*&  pDes,      Int iWidth, Int iHeight, UChar indexROT );
#endif
  Void xDeQuant2x2      ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );
  Void xDeQuant4x4      ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );
  Void xDeQuant8x8      ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );

  // inverse transform
#if QC_MDDT
  __inline Int		xRound_klt	 ( Int64 i )   { return (Int)(((i)+(1<<19))>>20); }//KLTprec+6=14+6=20 //precision increase
  Void xDeQuant_klt ( Int size, TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );
  Void xIT4_klt		( UInt ipmode, Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT8_klt		( UInt ipmode, Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT	        ( TextType eText, UInt uiMode, Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize, UChar indexROT );
#else
  Void xIT    ( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize );
#endif
  Void xIT2   ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT4   ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT8   ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT16  ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT32  ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT64  ( Long* plCoef, Pel* pResidual, UInt uiStride );

};// END CLASS DEFINITION TComTrQuant


#endif // __TCOMTRQUANT__

