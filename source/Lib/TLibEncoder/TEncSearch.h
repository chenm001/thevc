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

/** \file     TEncSearch.h
    \brief    encoder search class (header)
*/

#ifndef __TENCSEARCH__
#define __TENCSEARCH__

// Include files
#include "../TLibCommon/TComYuv.h"
#include "../TLibCommon/TComMotionInfo.h"
#include "../TLibCommon/TComPattern.h"
#include "../TLibCommon/TComPredFilter.h"
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TComTrQuant.h"
#include "../TLibCommon/TComPic.h"
#include "TEncEntropy.h"
#include "TEncSbac.h"
#include "TEncCfg.h"

#if HHI_INTERP_FILTER
#include "../TLibCommon/TComPredFilterMOMS.h"
#endif

class TEncCu;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder search class
class TEncSearch : public TComPrediction
{
#if HHI_RQT
private:
  TCoeff**        m_ppcQTTempCoeffY;
  TCoeff**        m_ppcQTTempCoeffCb;
  TCoeff**        m_ppcQTTempCoeffCr;
  TCoeff*         m_pcQTTempCoeffY;
  TCoeff*         m_pcQTTempCoeffCb;
  TCoeff*         m_pcQTTempCoeffCr;
  UChar*          m_puhQTTempTrIdx;
  UChar*          m_puhQTTempCbf[3];

  TComYuv*        m_pcQTTempTComYuv;
#endif
protected:
  // interface to option
  TEncCfg*        m_pcEncCfg;

  // interface to classes
  TComTrQuant*    m_pcTrQuant;
  TComRdCost*     m_pcRdCost;
  TEncEntropy*    m_pcEntropyCoder;

  // ME parameters
  Int             m_iSearchRange;
  Int             m_iFastSearch;
  Int             m_aaiAdaptSR[2][33];
  TComMv          m_cSrchRngLT;
  TComMv          m_cSrchRngRB;
  TComMv          m_acMvPredictors[3];

  // RD computation
  TEncSbac***     m_pppcRDSbacCoder;
  TEncSbac*       m_pcRDGoOnSbacCoder;
  Bool            m_bUseSBACRD;
  DistParam       m_cDistParam;

  // Misc.
  Pel*            m_pTempPel;
  UInt*           m_puiDFilter;
  Int             m_iDIFTap2;
  Int             m_iMaxDeltaQP;

  // AMVP cost computation
  // UInt            m_auiMVPIdxCost[AMVP_MAX_NUM_CANDS+1][AMVP_MAX_NUM_CANDS];
  UInt            m_auiMVPIdxCost[AMVP_MAX_NUM_CANDS+1][AMVP_MAX_NUM_CANDS+1]; //th array bounds

#if HHI_IMVP
  MvPredMeasure   m_cMvPredMeasure;
#endif

public:
  TEncSearch();
  virtual ~TEncSearch();

  Void init(  TEncCfg*      pcEncCfg,
              TComTrQuant*  pcTrQuant,
              Int           iSearchRange,
              Int           iFastSearch,
              Int           iMaxDeltaQP,
              TEncEntropy*  pcEntropyCoder,
              TComRdCost*   pcRdCost,
              TEncSbac***   pppcRDSbacCoder,
              TEncSbac*     pcRDGoOnSbacCoder );

protected:

  /// sub-function for motion vector refinement used in fractional-pel accuracy
#ifdef ROUNDING_CONTROL_BIPRED
  UInt  xPatternRefinement_Bi( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac, Pel* pRefY2, Bool bRound );
#ifdef QC_AMVRES
#if HHI_INTERP_FILTER
  UInt xPatternRefinementHAM_MOMS_Bi    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth, InterpFilterType ePFilt,TComMv* PredMv, Pel* pRefY2, Bool bRound );
#endif
  UInt xPatternRefinementHAM_DIF_Bi    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth,TComMv* predMV, Pel* pRefY2, Bool bRound );
#endif
#endif

  UInt  xPatternRefinement( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac );

#ifdef QC_AMVRES
#if HHI_INTERP_FILTER
  UInt xPatternRefinementHAM_MOMS    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth, InterpFilterType ePFilt,TComMv* PredMv);
#endif
  UInt xPatternRefinementHAM_DIF    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, TComMv& rcMvFrac ,UInt  uiDistBest_onefourth,TComMv* predMV);
#endif
#if ANG_INTRA
  Bool predIntraLumaDirAvailable( UInt uiMode, UInt uiWidthBit, Bool angIntraEnabled, Bool bAboveAvail, Bool bLeftAvail);
#endif

#if PLANAR_INTRA
  Void xIntraPlanarRecon( TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piOrg, Pel* piPred, Pel* piResi, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiWidth, UInt uiHeight, UInt uiCurrDepth, TextType eText );
#endif

  typedef struct
  {
    Pel*  piRefY;
    Int   iYStride;
    Int   iBestX;
    Int   iBestY;
    UInt  uiBestRound;
    UInt  uiBestDistance;
    UInt  uiBestSad;
    UChar ucPointNr;
  } IntTZSearchStruct;

  // sub-functions for ME
  __inline Void xTZSearchHelp         ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStruct, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance );
  __inline Void xTZ2PointSearch       ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB );
  __inline Void xTZ8PointSquareSearch ( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist );
  __inline Void xTZ8PointDiamondSearch( TComPattern* pcPatternKey, IntTZSearchStruct& rcStrukt, TComMv* pcMvSrchRngLT, TComMv* pcMvSrchRngRB, const Int iStartX, const Int iStartY, const Int iDist );

public:

  /// encoder estimation - intra prediction (luma)
  Void predIntraLumaAdiSearch   ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv,
                                  TComYuv*&   rpcResiYuv,
                                  TComYuv*&   rpcRecoYuv );

#if PLANAR_INTRA
  /// encoder estimation - planar intra prediction (luma & chroma)
  Void predIntraPlanarSearch    ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv,
                                  TComYuv*&   rpcResiYuv,
                                  TComYuv*&   rpcRecoYuv );
#endif

  /// encoder estimation - intra prediction (chroma)
  Void predIntraChromaAdiSearch ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv,
                                  TComYuv*&   rpcResiYuv,
                                  TComYuv*&   rpcRecoYuv,
                                  UInt        uiChromaTrMode );

#if HHI_RQT_INTRA
  Void  preestChromaPredMode    ( TComDataCU* pcCU, 
                                  TComYuv*    pcOrgYuv, 
                                  TComYuv*    pcPredYuv );
  Void  estIntraPredQT          ( TComDataCU* pcCU, 
                                  TComYuv*    pcOrgYuv, 
                                  TComYuv*    pcPredYuv, 
                                  TComYuv*    pcResiYuv, 
                                  TComYuv*    pcRecoYuv,
                                  UInt&       ruiDistC,
                                  Bool        bLumaOnly );
  Void  estIntraPredChromaQT    ( TComDataCU* pcCU, 
                                  TComYuv*    pcOrgYuv, 
                                  TComYuv*    pcPredYuv, 
                                  TComYuv*    pcResiYuv, 
                                  TComYuv*    pcRecoYuv,
                                  UInt        uiPreCalcDistC );
#endif


  /// encoder estimation - inter prediction (non-skip)
  Void predInterSearch          ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv,
                                  TComYuv*&   rpcResiYuv,
                                  TComYuv*&   rpcRecoYuv,
                                  Bool        bUseRes = false );

  /// encoder estimation - intra prediction (skip)
  Void predInterSkipSearch      ( TComDataCU* pcCU,
                                  TComYuv*    pcOrgYuv,
                                  TComYuv*&   rpcPredYuv,
                                  TComYuv*&   rpcResiYuv,
                                  TComYuv*&   rpcRecoYuv );

#if HHI_MRG
  /// encoder estimation - inter prediction (merge)
  Void predInterMergeSearch      ( TComDataCU* pcCU,
                                   TComYuv*    pcOrgYuv,
                                   TComYuv*&   rpcPredYuv,
                                   TComYuv*&   rpcResiYuv,
                                   TComYuv*&   rpcRecoYuv,
                                   TComMvField cMFieldNeighbourToTest[2],
                                   UChar uhInterDirNeighbourToTest);
#endif

  /// encode residual and compute rd-cost for inter mode
  Void encodeResAndCalcRdInterCU( TComDataCU* pcCU,
                                  TComYuv*    pcYuvOrg,
                                  TComYuv*    pcYuvPred,
                                  TComYuv*&   rpcYuvResi,
#if HHI_RQT
                                  TComYuv*&   rpcYuvResiBest,
#endif
                                  TComYuv*&   rpcYuvRec,
                                  Bool        bSkipRes );

  /// set ME search range
  Void setAdaptiveSearchRange   ( Int iDir, Int iRefIdx, Int iSearchRange) { m_aaiAdaptSR[iDir][iRefIdx] = iSearchRange; }

protected:

  // -------------------------------------------------------------------------------------------------------------------
  // Intra search (ADI & CIP)
  // -------------------------------------------------------------------------------------------------------------------

#if HHI_RQT_INTRA
  Void  xEncSubdivCbfQT           ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    Bool         bLuma,
                                    Bool         bChroma );
  Void  xEncCoeffQT               ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    TextType     eTextType,
                                    Bool         bRealCoeff );
  Void  xEncIntraHeader           ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    Bool         bLuma,
                                    Bool         bChroma );
  UInt  xGetIntraBitsQT           ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    Bool         bLuma,
                                    Bool         bChroma,
                                    Bool         bRealCoeff );

  Void  xIntraCodingLumaBlk       ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    TComYuv*     pcOrgYuv, 
                                    TComYuv*     pcPredYuv, 
                                    TComYuv*     pcResiYuv, 
                                    UInt&        ruiDist );
  Void  xIntraCodingChromaBlk     ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    TComYuv*     pcOrgYuv, 
                                    TComYuv*     pcPredYuv, 
                                    TComYuv*     pcResiYuv, 
                                    UInt&        ruiDist,
                                    UInt         uiChromaId );
  Void  xRecurIntraCodingQT       ( TComDataCU*  pcCU, 
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx, 
                                    Bool         bLumaOnly,
                                    TComYuv*     pcOrgYuv, 
                                    TComYuv*     pcPredYuv, 
                                    TComYuv*     pcResiYuv, 
                                    UInt&        ruiDistY,
                                    UInt&        ruiDistC,
                                    Double&      dRDCost );
  Void  xSetIntraResultQT         ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    Bool         bLumaOnly,
                                    TComYuv*     pcRecoYuv );

  Void  xRecurIntraChromaCodingQT ( TComDataCU*  pcCU, 
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx, 
                                    TComYuv*     pcOrgYuv, 
                                    TComYuv*     pcPredYuv, 
                                    TComYuv*     pcResiYuv, 
                                    UInt&        ruiDist );
  Void  xSetIntraResultChromaQT   ( TComDataCU*  pcCU,
                                    UInt         uiTrDepth,
                                    UInt         uiAbsPartIdx,
                                    TComYuv*     pcRecoYuv );
#endif

  Void xRecurIntraLumaSearchADI   ( TComDataCU* pcCU,
                                    UInt        uiAbsPartIdx,
                                    Pel*        piOrg,
                                    Pel*        piPred,
                                    Pel*        piResi,
                                    Pel*        piReco,
                                    UInt        uiStride,
                                    TCoeff*     piCoeff,
                                    UInt        uiMode,
#if HHI_AIS
                                    Bool        bSmoothing,
#endif
                                    UInt        uiWidth,
                                    UInt        uiHeight,
                                    UInt        uiMaxDepth,
                                    UInt        uiCurrDepth,
                                    Bool        bAbove,
                                    Bool        bLeft,
                                    Bool        bSmallTrs );

  Void xRecurIntraChromaSearchADI ( TComDataCU* pcCU,
                                    UInt        uiAbsPartIdx,
                                    Pel*        piOrg,
                                    Pel*        piPred,
                                    Pel*        piResi,
                                    Pel*        piReco,
                                    UInt        uiStride,
                                    TCoeff*     piCoeff,
                                    UInt        uiMode,
                                    UInt        uiWidth,
                                    UInt        uiHeight,
                                    UInt        uiMaxDepth,
                                    UInt        uiCurrDepth,
                                    TextType    eText );

  Void xPredIntraLumaNxNCIPEnc    ( TComPattern*  pcTComPattern,
                                    Pel*          pOrig,
                                    Pel*          pPredCL,
                                    UInt          uiStride,
                                    Pel*          pPred,
                                    UInt          uiPredStride,
                                    Int           iWidth,
                                    Int           iHeight,
                                    TComDataCU*   pcCU,
                                    Bool          bAboveAvail,
                                    Bool          bLeftAvail );

  // -------------------------------------------------------------------------------------------------------------------
  // Inter search (AMP)
  // -------------------------------------------------------------------------------------------------------------------

  Void xEstimateMvPredAMVP        ( TComDataCU* pcCU,
                                    TComYuv*    pcOrgYuv,
                                    UInt        uiPartIdx,
                                    RefPicList  eRefPicList,
                                    Int         iRefIdx,
                                    TComMv&     rcMvPred,
                                    Bool        bFilled = false );

  Void xCheckBestMVP              ( TComDataCU* pcCU,
                                    RefPicList  eRefPicList,
                                    TComMv      cMv,
                                    TComMv&     rcMvPred,
                                    Int&        riMVPIdx,
                                    UInt&       ruiBits,
                                    UInt&       ruiCost );

#ifdef QC_AMVRES
  Void xCheckBestMVP_onefourth							( TComDataCU* pcCU,
																		RefPicList	eRefPicList,
																		TComMv			cMv,
																		TComMv&			rcMvPred,
																		Int&				riMVPIdx,
																		UInt&				ruiBits,
																		UInt&				ruiCost );
#endif

  UInt xGetTemplateCost           ( TComDataCU* pcCU,
                                    UInt        uiPartIdx,
                                    UInt        uiPartAddr,
                                    TComYuv*    pcOrgYuv,
                                    TComYuv*    pcTemplateCand,
                                    TComMv      cMvCand,
                                    Int         iMVPIdx,
                                    Int         iMVPNum,
                                    RefPicList  eRefPicList,
                                    Int         iRefIdx,
                                    Int         iSizeX,
                                    Int         iSizeY );

  Void xCopyAMVPInfo              ( AMVPInfo*   pSrc, AMVPInfo* pDst );
  UInt xGetMvpIdxBits             ( Int iIdx, Int iNum );
  Void xGetBlkBits                ( PartSize  eCUMode, Bool bPSlice, Int iPartIdx,  UInt uiLastMode, UInt uiBlkBit[3]);

#if HHI_IMVP
  Void xEstimateMvPredIMVP        ( TComDataCU*     pcCU,
                                    UInt            uiPartIdx,
                                    RefPicList      eRefPicList,
                                    Int             iRefIdx,
                                    TComMv&         rcMvPred,
                                    MvPredMeasure&  rcMvPredMeasure );
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // motion estimation
  // -------------------------------------------------------------------------------------------------------------------

  Void xMotionEstimation          ( TComDataCU*   pcCU,
                                    TComYuv*      pcYuvOrg,
                                    Int           iPartIdx,
                                    RefPicList    eRefPicList,
                                    TComMv*       pcMvPred,
                                    Int           iRefIdxPred,
                                    TComMv&       rcMv,
                                    UInt&         ruiBits,
                                    UInt&         ruiCost,
                                    Bool          bBi = false  );

  Void xTZSearch                  ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
                                    UInt&         ruiSAD );

  Void xSetSearchRange            ( TComDataCU*   pcCU,
                                    TComMv&       cMvPred,
                                    Int           iSrchRng,
                                    TComMv&       rcMvSrchRngLT,
                                    TComMv&       rcMvSrchRngRB );

  Void xPatternSearchFast         ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
                                    UInt&         ruiSAD );

#ifdef ROUNDING_CONTROL_BIPRED
  Void xPatternSearch_Bi             ( TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
                                    UInt&         ruiSAD,
				    Pel*		  pcRefY2,
				    Bool		  bRound);

  Void xPatternSearchFracDIF_Bi      ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
  									UInt&    	  ruiCost,
#ifdef QC_AMVRES																		
									TComMv         *PredMv, 
									Int            iRefIdxPred, 
#endif
									Pel*		  pcRefY2,
									Bool		  bRound);


#ifdef QC_SIFO
  Void xPatternSearchFracDIF_QC_Bi   ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
									UInt&					ruiCost, 
#ifdef QC_AMVRES																		
									TComMv *PredMv, 
									Int iRefIdxPred, 
#endif
									Pel*		  pcRefY2,
									Bool		  bRound	
									);
#endif
  UInt xPatternRefinementMC_Bi    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac,Pel* pcRefY2,	Bool bRound);
#if TEN_DIRECTIONAL_INTERP
  Void xPatternSearchFracDIF_TEN_Bi   ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
                                    UInt&         ruiCost, 
									Pel*		  pcRefY2,
									Bool		  bRound	
									);
#endif
#if HHI_INTERP_FILTER
  Void xPatternSearchFracMOMS_Bi     ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
                                    UInt&         ruiCost,
                                    InterpFilterType ePFilt, 
#ifdef QC_AMVRES
									TComMv *PredMv,
   				                    Int iRefIdxPred, 
#endif			
									Pel*		  pcRefY2,
									Bool		  bRound	
  				    );
#endif
#endif

  Void xPatternSearch             ( TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvSrchRngLT,
                                    TComMv*       pcMvSrchRngRB,
                                    TComMv&       rcMv,
                                    UInt&         ruiSAD );

  Void xPatternSearchFracDIF      ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
																		UInt&					ruiCost 
#ifdef QC_AMVRES																		
																		,TComMv *PredMv 
																		, Int iRefIdxPred 
#endif
																		);
#ifdef QC_SIFO
  Void xPatternSearchFracDIF_QC   ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
																		UInt&					ruiCost 
#ifdef QC_AMVRES																		
																		,TComMv *PredMv 
																		, Int iRefIdxPred 
#endif
																		);
  
  Void xAddSubFullPelOffset    ( TComPattern* pcPatternKey, Int iOffset, Bool Add);
  UInt xPatternRefinementMC    ( TComPattern* pcPatternKey, Pel* piRef, Int iRefStride, Int iIntStep, Int iFrac, TComMv& rcMvFrac);
  Void xExtDIFUpSamplingH_QC   ( TComPattern*  pcPattern, TComYuv* pcYuvExt);
#endif

  Void xExtDIFUpSamplingH         ( TComPattern*  pcPattern, TComYuv* pcYuvExt  );

  Void xExtDIFUpSamplingQ         ( TComPattern*  pcPatternKey,
                                    Pel*          piDst,
                                    Int           iDstStride,
                                    Pel*          piSrcPel,
                                    Int           iSrcPelStride,
                                    Int*          piSrc,
                                    Int           iSrcStride,
                                    UInt          uiFilter  );

#if TEN_DIRECTIONAL_INTERP
  Void xPatternSearchFracDIF_TEN   ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
                                    UInt&         ruiCost );

  Void xExtDIFUpSamplingH_TEN     ( TComPattern*  pcPattern, TComYuv* pcYuvExt  );
  Void xExtDIFUpSamplingQ_TEN     ( TComPattern*  pcPatternKey,
                                    Pel*          piDst,
                                    Int           iDstStride,
                                    Pel*          piSrcPel,
                                    Int           iSrcPelStride,
                                    Int*          piSrc,
                                    Int           iSrcStride,
                                    UInt          uiFilter  );
#endif

#if HHI_INTERP_FILTER
  Void xPatternSearchFracMOMS     ( TComDataCU*   pcCU,
                                    TComPattern*  pcPatternKey,
                                    Pel*          piRefY,
                                    Int           iRefStride,
                                    TComMv*       pcMvInt,
                                    TComMv&       rcMvHalf,
                                    TComMv&       rcMvQter,
                                    UInt&         ruiCost,
                                    InterpFilterType ePFilt 
#ifdef QC_AMVRES
									,TComMv *PredMv
									, Int iRefIdxPred 
#endif
									);
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // T & Q & Q-1 & T-1
  // -------------------------------------------------------------------------------------------------------------------

#if HHI_RQT
  Void xEncodeResidualQT( TComDataCU* pcCU, UInt uiAbsPartIdx, const UInt uiDepth, Bool bSubdivAndCbf, TextType eType );
#if HHI_RQT_ROOT
  Void xEstimateResidualQT( TComDataCU* pcCU, UInt uiQuadrant, UInt uiAbsPartIdx, TComYuv* pcResi, const UInt uiDepth, Double &rdCost, UInt &ruiBits, UInt &ruiDist, UInt *puiZeroDist );
#else
  Void xEstimateResidualQT( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcResi, const UInt uiDepth, Double &rdCost, UInt &ruiBits, UInt &ruiDist );
#endif
  Void xSetResidualQTData( TComDataCU* pcCU, UInt uiAbsPartIdx, TComYuv* pcResi, UInt uiDepth, Bool bSpatial );
#endif
  Void xEncodeInterTexture        ( TComDataCU*&  rpcCU, UInt uiQp, Bool bHighPass, TComYuv*& rpcYuv, UInt uiTrMode );
  Void xRecurTransformNxN         ( TComDataCU*   rpcCU,
                                    UInt          uiAbsPartIdx,
                                    Pel*          pcResidual,
                                    UInt          uiAddr,
                                    UInt          uiStride,
                                    UInt          uiWidth,
                                    UInt          uiHeight,
                                    UInt          uiMaxTrMode,
                                    UInt          uiTrMode,
                                    TCoeff*&      rpcCoeff,
                                    TextType      eType,
                                    Int           indexROT = 0 );

  // -------------------------------------------------------------------------------------------------------------------
  // compute symbol bits
  // -------------------------------------------------------------------------------------------------------------------

  Void xAddSymbolBitsInter        ( TComDataCU*   pcCU,
                                    UInt          uiQp,
                                    UInt          uiTrMode,
                                    UInt&         ruiBits,
                                    TComYuv*&     rpcYuvRec,
                                    TComYuv*      pcYuvPred,
                                    TComYuv*&     rpcYuvResi );

  Void xAddSymbolBitsIntra        ( TComDataCU*   pcCU,
                                    TCoeff*       pCoeff,
                                    UInt          uiPU,
                                    UInt          uiQNumPart,
                                    UInt          uiPartDepth,
                                    UInt          uiNumPart,
                                    UInt          uiMaxTrDepth,
                                    UInt          uiTrDepth,
                                    UInt          uiWidth,
                                    UInt          uiHeight,
                                    UInt&         ruiBits );

};// END CLASS DEFINITION TEncSearch


#endif // __TENCSEARCH__

