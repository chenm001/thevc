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

/** \file     TEncAdaptiveLoopFilter.h
    \brief    estimation part of adaptive loop filter class (header)
*/

#ifndef __TENCADAPTIVELOOPFILTER__
#define __TENCADAPTIVELOOPFILTER__

#include "../TLibCommon/TComAdaptiveLoopFilter.h"
#include "../TLibCommon/TComPic.h"

#include "TEncEntropy.h"
#include "TEncSbac.h"
#include "../TLibCommon/TComBitCounter.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if HHI_ALF
/// estimation part of adaptive loop filter class
class TEncAdaptiveLoopFilter : public TComAdaptiveLoopFilter
{
private:

  Double**          m_ppdAlfCorr;
  Double*           m_pdDoubleAlfCoeff;

  UInt****          m_puiCUHorizontalCorr;
  UInt****          m_puiCUVerticalCorr;

  SliceType         m_eSliceType;
  Int               m_iPicNalReferenceIdc;

  Double            m_dLambdaLuma;
  Double            m_dLambdaChroma;

  TEncEntropy*      m_pcEntropyCoder;
  TEncSbac***       m_pppcRDSbacCoder;
  TEncSbac*         m_pcRDGoOnSbacCoder;
  TComBitIf*        m_pcCoderFixedBits;
  Bool              m_bUseSBACRD;

  TComPic*          m_pcPic;
  ALFParam*         m_pcBestAlfParam;
  ALFParam*         m_pcTempAlfParam;

  TComPicYuv*       m_pcPicYuvBest;
  TComPicYuv*       m_pcPicYuvTmp;
  TComPicYuv*       m_pcPicYuvFiltered;

  UInt64            m_uiMinRate;
  UInt64            m_uiMinDist;
  Double            m_dMinCost;

  UInt              m_uiNumSCUInCU;
  UInt              m_uiSCUWidth;
  UInt              m_uiSCUHeight;

  //parameters
  Bool              m_bALFSeparateQt;
  Bool              m_bALFSymmetry;
  Int               m_iALFMinLength;
  Int               m_iALFMaxLength;


private:
  // init / uninit internal variables
  Void xInitParam      ();
  Void xUninitParam    ();

  // create/destroy/copy/set functions of ALF control flags
  Void xCreateTmpAlfCtrlFlags   ( TComPicSym* pcQuadTree );
  Void xDestroyTmpAlfCtrlFlags  ( TComPicSym* pcQuadTree );
  Void xCopyTmpAlfCtrlFlagsTo   ( TComPicSym* pcQuadTree );
  Void xCopyTmpAlfCtrlFlagsFrom ( TComPicSym* pcQuadTree );
  Void xSetCUAlfCtrlFlags       ( UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest,TComPicSym* pcQuadTree, UInt64& ruiDist, UInt64& ruiRate );
  Void xSetCUAlfCtrlFlag        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist );
  Void xSetCUAlfCtrlFlag        ( TComPicSym* pcQuadTree,TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiResultDist,  UInt64* auiFixedBitsCurrBest, UInt64* auiFixedBitsNextBest );

  // functions related to correlation computation
  Void xReadOrCalcCorrFromCUs           ( TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr );
  Void xReadOrCalcCorrFromFUs           ( TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr );
  Void xReadOrCalcCorrFromFU            ( TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr, AlfFilter& rcFilter, UInt**** ppdAlfCorr );
  Void xCalcALFCoeff                    ( AlfFilter& rcFilter );
  Void xCalcCorrelationFunc             ( Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);
  Void xCalcCorrelationFuncBlock        ( Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);
  Void xCalcStoredCorrelationFuncBlock  ( Pel* pOrg, Pel* pCmp, UInt** ppuiCorr, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);

  Void xEstimateCorr                    ( Pel* pOrig, Pel* pDec, Int iWidth, Int iHeight, Int iOrigStride , Int iDecStride, AlfFilter& rcFilter , double** ppdCorr , Int iOffsetX , Int iOffsetY, Int iMaxX , Int iMaxY );
  Void xEstimateCorr                    ( Pel* pOrig, Pel* pDec, Int iWidth, Int iHeight, Int iOrigStride , Int iDecStride, AlfFilter& rcFilter , UInt** ppuiCorr , Int iOffsetX , Int iOffsetY, Int iMaxX , Int iMaxY );
  Void xEstimateCorrCU                  ( TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiSCUIndx, UChar uhDepth, AlfFilter& rcFilter, double** pdCUCorr  );
  Void xEstimateCorrCU                  ( TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiSCUIndx, UChar uhDepth, AlfFilter& rcFilter, UInt** puiCUCorr  );
  Void xEstimateCorrFU                  (TComPicYuv* pcPicOrg , TComPicYuv* pcPicDec, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, AlfFilter& rcFilter, double** pdCUCorr  );

  // functions related to filtering
  Void xFilterCoefQuickSort     ( Double *coef_data, Int *coef_num, Int upper, Int lower );
  Void xQuantFilterCoef         ( Double* adDoubleCoeffs, AlfFilter& rcFilter, Int iBit_depth );
  Void xClearFilterCoefInt      (  AlfFilter& rcFilter );

  Void xReDesignFilterCoeff     ( TComPicSym* pcQuadTree, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, AlfFilter& rcFilter, UInt**** ppuiAlfCorr, Bool bReadCorr);
  Void xFilteringFrameLuma      ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Bool bStoreCorr );
  Void xFilteringFrameChroma    ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );

  Void xEstimateFrameFilterLuma ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, AlfFilter& rcFilter , Bool bStoreCorr , UInt**** puiCUCorr );
  Void xEstimateFrameFilter     ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, AlfFilter& rcFilter , Bool bStoreCorr , UInt**** puiCUCorr, Int iPlane ) ;

  Void  xPredictALFCoeff        ( ALFParam* pAlfParam) ;
  Void  xPredictALFCoeff        ( ALFParam* pAlfParam, Int iPlane) ;

  Void xCheckFilterReuse        (TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, ALFParam* pcFilterParam, Double& rcMinCost, Int iPlane) ;
  // distortion / misc functions
  UInt64 xCalcSSD               ( Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride );
  Void   xCalcRDCost            ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost, Int iPlane );
  Void   xCalcRDCostChroma      ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost );
  Void   xCalcRDCost            ( ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost );
  Int    xGauss                 ( Double **a, Int N );

protected:
  /// test ALF for luma
  Void xEncALFFullFrameLuma     ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost );
  Void xEncALFFullFrame         ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost, Int iPlane );
  /// test CU-based partition
  Void xCUAdaptiveControl       ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost );
  Void xCUAdaptiveControl       ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicFiltered, ALFParam* pcAlfParam, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost ) ;
  /// test various filter taps
  Void xFilterTapDecision       ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost, Int iPlane );
  Void xFilterTapDecision       (TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicFiltered, ALFParam* pcAlfParams, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost, Int iPlane) ;

  /// do ALF for chroma
  Void xEncALFChroma            ( UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits );
public:
  TEncAdaptiveLoopFilter          ();
  virtual ~TEncAdaptiveLoopFilter () {}

  /// allocate temporal memory
  Void startALFEnc(TComPic* pcPic, TEncEntropy* pcEntropyCoder , TEncSbac*** pppcRDSbacCoder,  TEncSbac* pcRDGoOnSbacCoder);

  /// destroy temporal memory
  Void endALFEnc();

  /// estimate ALF parameters
  Void ALFProcess(ALFParam* pcAlfParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth );


  // encode ALF QuadTree
  Void encodeQuadTree          ( ALFParam*    pAlfParam     , TEncEntropy* pcEntropyCoder, UInt uiMaxAlfCtrlDepth );
  Void encodeFUAlfCtrlFlags    ( TEncEntropy* pcEntropyCoder, TComPicSym*  pcQuadTree    , UInt uiMaxDepth        );
  Void encodeFUAlfCtrlFlag     ( TEncEntropy* pcEntropyCoder, TComPicSym*  pcQuadTree    , TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth );

};
#else
/// estimation part of adaptive loop filter class
class TEncAdaptiveLoopFilter : public TComAdaptiveLoopFilter
{
private:
  static const Int	m_aiSymmetricArray9x9[81];					///< scan index for 9x9 filter
  static const Int	m_aiSymmetricArray7x7[49];					///< scan index for 7x7 filter
  static const Int	m_aiSymmetricArray5x5[25];					///< scan index for 5x5 filter

  Double**					m_ppdAlfCorr;
  Double*						m_pdDoubleAlfCoeff;
  UInt****					m_puiCUCorr;

  SliceType					m_eSliceType;
  Int								m_iPicNalReferenceIdc;

  Double						m_dLambdaLuma;
  Double						m_dLambdaChroma;

  TEncEntropy*			m_pcEntropyCoder;

  TComPic*					m_pcPic;
  ALFParam*					m_pcBestAlfParam;
  ALFParam*					m_pcTempAlfParam;

  TComPicYuv*				m_pcPicYuvBest;
  TComPicYuv*				m_pcPicYuvTmp;

  UInt							m_uiNumSCUInCU;
  UInt							m_uiSCUWidth;
  UInt							m_uiSCUHeight;

private:
	// init / uninit internal variables
  Void xInitParam 	   ();
  Void xUninitParam	   ();

	// create/destroy/copy/set functions of ALF control flags
  Void xCreateTmpAlfCtrlFlags		();
  Void xDestroyTmpAlfCtrlFlags	();
  Void xCopyTmpAlfCtrlFlagsTo		();
  Void xCopyTmpAlfCtrlFlagsFrom	();
  Void xSetCUAlfCtrlFlags				( UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist );
  Void xSetCUAlfCtrlFlag				( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist );

	// encoder ALF control flags
	Void xEncodeCUAlfCtrlFlags	();
  Void xEncodeCUAlfCtrlFlag		( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);

	// functions related to correlation computation
	Void xReadOrCalcCorrFromCUs						( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr );
  Void xCalcALFCoeff										( ALFParam* pAlfParam );
  Void xCalcCorrelationFunc							( Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);
  Void xCalcCorrelationFuncBlock				( Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);
  Void xCalcStoredCorrelationFuncBlock	( Pel* pOrg, Pel* pCmp, UInt** ppuiCorr, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride);

	// functions related to filtering
	Void xFilterCoefQuickSort		( Double *coef_data, Int *coef_num, Int upper, Int lower );
  Void xQuantFilterCoef				( Double* h, Int* qh, Int tap, int bit_depth );
  Void xClearFilterCoefInt		( Int* qh, Int N );
  Void xReDesignFilterCoeff		( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, Bool bReadCorr );
  Void xCopyDecToRestCUs			( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xCopyDecToRestCU				( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xFilteringFrameLuma		( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Bool bStoreCorr );
  Void xFilteringFrameChroma	( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );

	// distortion / misc functions
  UInt64 xCalcSSD							( Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride );
  Void	 xCalcRDCost					( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost );
  Void   xCalcRDCostChroma		( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost );
  Void   xCalcRDCost					( ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost );
  Int		 xGauss								( Double **a, Int N );

protected:
	/// test ALF for luma
  Void xEncALFLuma						( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost );

	/// test CU-based partition
  Void xCUAdaptiveControl			( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost );

	/// test various filter taps
  Void xFilterTapDecision			( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost);

	/// do ALF for chroma
	Void xEncALFChroma					( UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits );
public:
  TEncAdaptiveLoopFilter					();
	virtual ~TEncAdaptiveLoopFilter	() {}

	/// allocate temporal memory
  Void startALFEnc(TComPic* pcPic, TEncEntropy* pcEntropyCoder);

	/// destroy temporal memory
  Void endALFEnc();

	/// estimate ALF parameters
  Void ALFProcess(ALFParam* pcAlfParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth );
};
#endif
#endif
