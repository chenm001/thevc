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

/** \file			TComTrQuant.h
    \brief		transform and quantization class (header)
*/

#ifndef __TCOMTRQUANT__
#define __TCOMTRQUANT__

#include "CommonDef.h"
#include "TComYuv.h"
#include "TComDataCU.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_MAX_PREC_COEFF			25					///< max. precision of coefficient in RDOQ
#define QP_BITS									15

// AQO Parameter
#define QOFFSET_BITS						15
#define QOFFSET_BITS_LTR				9

// LTR Butterfly Paramter
#define ECore16Shift						10
#define DCore16Shift						10
#define ECore32Shift						10
#define DCore32Shift						10
#define ECore64Shift						9
#define DCore64Shift						9

#define DenShift16							6
#define DenShift32							8
#define DenShift64							10

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
  Int significantBits[16][2];
  Int lastBits[16][2];
  Int greaterOneBits[2][5][2];
  Int greaterOneState[5];
  Int blockCbpBits[4][2];
  Int scanZigzag[2];						///< flag for zigzag scan
  Int scanNonZigzag[2];					///< flag for non zigzag scan
} estBitsSbacStruct;

typedef struct
{
  Int level[3];
  Int pre_level;
  Int coeff_ctr;
  Long levelDouble;
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

  Void initOffsetParam(Int iStartQP = MIN_QP, Int iEndQP = MAX_QP, Bool bUseJMCFG = false );
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
  Void init									( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Bool bUseADI, Bool bUseROT, Bool bUseLCT, Bool bUseACS, Bool bUseJMCFG = false, Bool bUseRDOQ = false, Bool bEnc = false );

	// transform & inverse transform functions
  Void transformNxN					( TComDataCU* pcCU, Pel*   pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight,
                              UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT = 0 );

  Void invtransformNxN			( Pel*& rpcResidual, UInt uiStride, TCoeff*   pcCoeff, UInt uiWidth, UInt uiHeight,
															UChar indexROT = 0 );

  Void invRecurTransformNxN	( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr,		UInt uiStride, UInt uiWidth, UInt uiHeight,
															UInt uiMaxTrMode,  UInt uiTrMode, TCoeff* rpcCoeff, Int indexROT = 0);
public:
  Void setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType);

  Void RotTransform4I     ( Long* matrix, UChar index		);
  Void InvRotTransform4I  ( Long* matrix, UChar index		);
  Void RotTransformLI2    ( Long* matrix, UChar index		);
  Void InvRotTransformLI2 ( Long* matrix, UChar index		);

  Void precalculateUnaryExpGolombLevel();
  Void setLambda(Double dLambda) { m_dLambda = dLambda;}

  estBitsSbacStruct* m_pcEstBitsSbac;
  Int m_aiPrecalcUnaryLevelTab[128][RDOQ_MAX_PREC_COEFF];

protected:
  Long*    m_plTempCoeff;
  UInt*    m_puiQuantMtx;

  QpParam  m_cQP;
  Int      m_iLTRChromaQP;
  Double   m_dLTROffset;

	UInt		 m_uiMaxTrSize;
  Bool     m_bUseADI;
	Bool		 m_bUseROT;
  Bool     m_bUseJMCFG;
	Bool		 m_bEnc;

  Bool     m_bUseRDOQ;
  Bool     m_bUseACS;
  Bool     m_bUseLCT;

  Double	 m_dLambda;

private:
  // Forward Transform
  Void (*xT16) (Pel* , UInt , Long* );
  Void (*xT32) (Pel* , UInt , Long* );

	Void xT  ( Pel* pResidual, UInt uiStride, Long* plCoeff, Int iSize );
  Void xT2 ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT4 ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT8 ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  static Void xT16_Chen ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  static Void xT16_Loeffler_Lifting ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  static Void xT32_Chen( Pel* pResidual, UInt uiStride, Long* plCoeff );
  static Void xT32_Loeffler_Lifting ( Pel* pResidual, UInt uiStride, Long* plCoeff );
  Void xT64( Pel* pResidual, UInt uiStride, Long* plCoeff );

  Void xQuant( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
  Void xDeQuant( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight, UChar indexROT );
  Void xUniQuantLTR  ( TComDataCU* pcCU, Long*  pSrc,   TCoeff*&  pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT  );
  Void xQuant2x2  ( Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, UChar indexROT );
  Void xQuant4x4  ( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );
  Void xQuant8x8  ( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );

  // ACS
  Void xRateDistOptQuant ( TComDataCU* pcCU, Long* pSrcCoef, TCoeff*& pDstCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx, UChar indexROT );

  // ACS
  Double xEst_writeRunLevel_SBAC ( levelDataStruct* levelData, Int* levelTabMin, TextType eTType, Double lambda, Int& kInit, Int kStop, Int noCoeff, Int estCBP, UInt uiWidth, UInt uiHeight, UInt uiDepth, UInt uiScanIdx );
  Int  xEst_write_and_store_CBP_block_bit ( TComDataCU* pcCU, TextType eTType );
  Int  est_unary_exp_golomb_level_encode (UInt symbol, Int ctx, TextType eTType, UInt uiDepth);
  Int  est_exp_golomb_encode_eq_prob (UInt symbol);
  Int  est_unary_exp_golomb_level_bits( UInt symbol, Int bits0, Int bits1);

#if IQC_ROUND_OFF
  __inline Int  xRound ( Int i )   { return ((i)+i/25+(1<<5))>>6; }
#else
  __inline Int  xRound ( Int i )   { return ((i)+(1<<5))>>6; }
#endif

  __inline static Long  xTrRound ( Long i, UInt uiShift ) { return ((i)>>uiShift); }

  // Backward Transform
  Void xDeQuant2x2  ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );
  Void xDeQuant4x4  ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );
  Void xDeQuant8x8  ( TCoeff* pSrcCoef, Long*& rplDstCoef, UChar indexROT );

  Void xUniDeQuantLTR		( TCoeff* pSrc, Double*& pfDes, Int iWidth, Int iHeight );
  Void xUniDeQuantLTR		( TCoeff* pSrc, Long*&	 pDes,	Int iWidth, Int iHeight, UChar indexROT );

  Void (*xIT16) (Long* , Pel* , UInt );
  Void (*xIT32) (Long* , Pel* , UInt );

	Void xIT  ( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize );
  Void xIT2 ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT4 ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT8 ( Long* plCoef, Pel* pResidual, UInt uiStride );
  static Void xIT16_Chen( Long* plCoef, Pel* pResidual, UInt uiStride );
  static Void xIT16_Loeffler_Lifting ( Long* plCoef, Pel* pResidual, UInt uiStride );
  static Void xIT32_Chen ( Long* plCoef, Pel* pResidual, UInt uiStride );
  static Void xIT32_Loeffler_Lifting ( Long* plCoef, Pel* pResidual, UInt uiStride );
  Void xIT64( Long* plCoef, Pel* pResidual, UInt uiStride );
};// END CLASS DEFINITION TComTrQuant


#endif // __TCOMTRQUANT__

