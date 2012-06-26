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

/** \file     TEncEntropy.h
    \brief    entropy encoder class (header)
*/

#ifndef __TENCENTROPY__
#define __TENCENTROPY__

#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComDataCU.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/ContextModel.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComAdaptiveLoopFilter.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"

class TEncSbac;
class TEncCavlc;
class SEI;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// entropy encoder pure class
class TEncEntropyIf
{
public:
#if !AHG6_ALF_OPTION2
  virtual Bool getAlfCtrl()                = 0;
  virtual UInt getMaxAlfCtrlDepth()                = 0;
  virtual Void setAlfCtrl(Bool bAlfCtrl)                = 0;
  virtual Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth)                = 0;
#endif  
  virtual Void  resetEntropy          ()                = 0;
  virtual Void  determineCabacInitIdx ()                = 0;
  virtual Void  setBitstream          ( TComBitIf* p )  = 0;
  virtual Void  setSlice              ( TComSlice* p )  = 0;
  virtual Void  resetBits             ()                = 0;
  virtual Void  resetCoeffCost        ()                = 0;
  virtual UInt  getNumberOfWrittenBits()                = 0;
  virtual UInt  getCoeffCost          ()                = 0;

#if VPS_INTEGRATION
  virtual Void  codeVPS                 ( TComVPS* pcVPS )                                      = 0;
#endif
  virtual Void  codeSPS                 ( TComSPS* pcSPS )                                      = 0;
  virtual Void  codePPS                 ( TComPPS* pcPPS )                                      = 0;
  virtual void codeSEI(const SEI&) = 0;
  virtual Void  codeSliceHeader         ( TComSlice* pcSlice )                                  = 0;
#if !REMOVE_TILE_MARKERS
  virtual Void codeTileMarkerFlag      ( TComSlice* pcSlice )                                  = 0;
#endif

  virtual Void  codeTilesWPPEntryPoint  ( TComSlice* pSlice )     = 0;
  virtual Void  codeTerminatingBit      ( UInt uilsLast )                                       = 0;
  virtual Void  codeSliceFinish         ()                                                      = 0;
  virtual Void  codeFlush               ()                                                      = 0;
  virtual Void  encodeStart             ()                                                      = 0;
#if !AHG6_ALF_OPTION2
  virtual Void codeAlfCtrlDepth() = 0;
#endif
  virtual Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList ) = 0;
  virtual Void codeScalingList   ( TComScalingList* scalingList )      = 0;
  
public:
#if AHG6_ALF_OPTION2
  virtual Void codeAlfCtrlFlag   ( Int compIdx, UInt code ) = 0;
#else
  virtual Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
#endif
  virtual Void codeApsExtensionFlag () = 0;
  
#if CU_LEVEL_TRANSQUANT_BYPASS
  virtual Void codeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
#endif
  virtual Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  
  virtual Void codeIPCMInfo      ( TComDataCU* pcCU, UInt uiAbsPartIdx, Int numIPCM, Bool firstIPCMFlag) = 0;

  virtual Void codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx ) = 0;
  virtual Void codeQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth ) = 0;
  virtual Void codeQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
#if INTRAMODE_BYPASSGROUP
  virtual Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU ) = 0;
#else
  virtual Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
#endif  
  
  virtual Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )      = 0;
  virtual Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )      = 0;
  virtual Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType ) = 0;
#if INTRA_TRANSFORMSKIP
  virtual Void codeTransformSkipFlags ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt width, UInt height, UInt uiDepth, TextType eTType ) = 0;
#endif
#if AHG6_ALF_OPTION2  
  virtual Void codeAlfParam      (ALFParam* alfParam) = 0;
#else
  virtual Void codeAlfFlag          ( UInt uiCode ) = 0;
  virtual Void codeAlfUvlc          ( UInt uiCode ) = 0;
  virtual Void codeAlfSvlc          ( Int   iCode ) = 0;
  virtual Void codeAlfFixedLengthIdx( UInt idx, UInt numFilterSetsInBuffer) = 0;
  virtual Void codeAPSAlflag(UInt uiCode) = 0;
#endif
  /// set slice granularity
  virtual Void setSliceGranularity(Int iSliceGranularity) = 0;

  /// get slice granularity
  virtual Int  getSliceGranularity()                      = 0;
#if !AHG6_ALF_OPTION2
  virtual Void codeAlfCtrlFlag      ( UInt uiSymbol ) = 0;
#endif
#if !SAO_CODE_CLEAN_UP
  virtual Void codeSaoFlag          ( UInt uiCode ) = 0;
  virtual Void codeSaoUvlc          ( UInt uiCode ) = 0;
#endif
#if SAO_OFFSET_MAG_SIGN_SPLIT
  virtual Void codeSAOSign          ( UInt code   ) = 0;
#endif
#if SAO_TRUNCATED_U
  virtual Void codeSaoMaxUvlc       ( UInt code, UInt maxSymbol ) = 0;
#endif
#if !(SAO_OFFSET_MAG_SIGN_SPLIT && SAO_RDO_FIX)
  virtual Void codeSaoSvlc          ( Int   iCode ) = 0;
#endif
#if !SAO_CODE_CLEAN_UP
  virtual Void codeSaoRun          ( UInt   uiCode, UInt uiMaxValue  ) = 0;
#endif
  virtual Void codeSaoMergeLeft    ( UInt   uiCode, UInt uiCompIdx  ) = 0;
  virtual Void codeSaoMergeUp      ( UInt   uiCode) = 0;
  virtual Void codeSaoTypeIdx      ( UInt   uiCode) = 0;
  virtual Void codeSaoUflc         ( UInt   uiCode) = 0;
  virtual Void estBit               (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType) = 0;
  
  virtual Void updateContextTables ( SliceType eSliceType, Int iQp, Bool bExecuteFinish )   = 0;
  virtual Void updateContextTables ( SliceType eSliceType, Int iQp )   = 0;
#if !REMOVE_TILE_MARKERS
  virtual Void writeTileMarker             ( UInt uiTileIdx, UInt uiBitsUsed ) = 0;
#endif

  virtual Void codeAPSInitInfo  (TComAPS* pcAPS)= 0;
  virtual Void codeFinish       (Bool bEnd)= 0;

  virtual Void codeDFFlag (UInt uiCode, const Char *pSymbolName) = 0;
  virtual Void codeDFSvlc (Int iCode, const Char *pSymbolName)   = 0;

  virtual ~TEncEntropyIf() {}

};

/// entropy encoder class
class TEncEntropy
{
private:
  UInt    m_uiBakAbsPartIdx;
  UInt    m_uiBakChromaOffset;
  UInt    m_bakAbsPartIdxCU;

public:
  Void    setEntropyCoder           ( TEncEntropyIf* e, TComSlice* pcSlice );
  Void    setBitstream              ( TComBitIf* p )          { m_pcEntropyCoderIf->setBitstream(p);  }
  Void    resetBits                 ()                        { m_pcEntropyCoderIf->resetBits();      }
  Void    resetCoeffCost            ()                        { m_pcEntropyCoderIf->resetCoeffCost(); }
  UInt    getNumberOfWrittenBits    ()                        { return m_pcEntropyCoderIf->getNumberOfWrittenBits(); }
  UInt    getCoeffCost              ()                        { return  m_pcEntropyCoderIf->getCoeffCost(); }
  Void    resetEntropy              ()                        { m_pcEntropyCoderIf->resetEntropy();  }
  Void    determineCabacInitIdx     ()                        { m_pcEntropyCoderIf->determineCabacInitIdx(); }
  
  Void    encodeSliceHeader         ( TComSlice* pcSlice );
#if !REMOVE_TILE_MARKERS
  Void    encodeTileMarkerFlag       (TComSlice* pcSlice) {m_pcEntropyCoderIf->codeTileMarkerFlag(pcSlice);}
#endif
  Void    encodeTilesWPPEntryPoint( TComSlice* pSlice );
  Void    encodeTerminatingBit      ( UInt uiIsLast );
  Void    encodeSliceFinish         ();
  Void    encodeFlush               ();
  Void    encodeStart               ();
#if AHG6_ALF_OPTION2
  Void encodeAlfParam(ALFParam* alfParam) {m_pcEntropyCoderIf->codeAlfParam(alfParam);}
#else
  Void encodeAlfFlag(UInt code) {m_pcEntropyCoderIf->codeAlfFlag(code);}
  Void encodeAlfStoredFilterSetIdx(UInt idx, UInt numFilterSetsInBuffer);
  Void encodeAlfFixedLengthRun(UInt run, UInt rx, UInt numLCUInWidth);
  Void encodeAlfParam(AlfParamSet* pAlfParamSet, Bool bSentInAPS = true, Int firstLCUAddr = 0, Bool alfAcrossSlice= true);
  Void encodeAlfParamSet(AlfParamSet* pAlfParamSet, Int numLCUInWidth, Int numLCU, Int firstLCUAddr, Bool alfAcrossSlice, Int startCompIdx, Int endCompIdx);
  Bool getAlfRepeatRowFlag(Int compIdx, AlfParamSet* pAlfParamSet, Int lcuIdx, Int lcuPos, Int startlcuPosX, Int endlcuPosX, Int numLCUInWidth);
  Int  getAlfRun(Int compIdx, AlfParamSet* pAlfParamSet, Int lcuIdxInSlice, Int lcuPos, Int startlcuPosX, Int endlcuPosX);
  Void encodeAPSAlfFlag(UInt code) {m_pcEntropyCoderIf->codeAPSAlflag(code);}
  Void encodeAlfParam(ALFParam* pAlfParam);
#endif
  TEncEntropyIf*      m_pcEntropyCoderIf;
  
public:
#if VPS_INTEGRATION
  Void encodeVPS               ( TComVPS* pcVPS);
#endif
  // SPS
  Void encodeSPS               ( TComSPS* pcSPS );
  Void encodePPS               ( TComPPS* pcPPS );
  void encodeSEI(const SEI&);
#if !AHG6_ALF_OPTION2
  Bool getAlfCtrl() {return m_pcEntropyCoderIf->getAlfCtrl();}
  UInt getMaxAlfCtrlDepth() {return m_pcEntropyCoderIf->getMaxAlfCtrlDepth();}
  Void setAlfCtrl(Bool bAlfCtrl) {m_pcEntropyCoderIf->setAlfCtrl(bAlfCtrl);}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_pcEntropyCoderIf->setMaxAlfCtrlDepth(uiMaxAlfCtrlDepth);}
#endif  
  Void encodeSplitFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
#if CU_LEVEL_TRANSQUANT_BYPASS
  Void encodeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#endif
  Void encodeSkipFlag          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePUWise       ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodeInterDirPU   ( TComDataCU* pcSubCU, UInt uiAbsPartIdx  );
  Void encodeRefFrmIdxPU  ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMvdPU        ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMVPIdxPU     ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx );
  Void encodeMergeIndex   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx, Bool bRD = false );
#if !AHG6_ALF_OPTION2
  Void encodeAlfCtrlFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#endif
  /// set slice granularity
  Void setSliceGranularity (Int iSliceGranularity) {m_pcEntropyCoderIf->setSliceGranularity(iSliceGranularity);}
#if AHG6_ALF_OPTION2
  /// encode ALF LCU control flag
  Void encodeAlfCtrlFlag( Int compIdx, UInt code ) {m_pcEntropyCoderIf->codeAlfCtrlFlag(compIdx, code);}
#else
  /// encode ALF CU control flag
  Void encodeAlfCtrlFlag(UInt uiFlag);
#endif  
  Void encodeApsExtensionFlag() {m_pcEntropyCoderIf->codeApsExtensionFlag();};
#if !AHG6_ALF_OPTION2
  Void encodeAlfCtrlParam(AlfCUCtrlInfo& cAlfParam, Int iNumCUsInPic);
#endif
  Void encodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
  Void encodeIPCMInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
#if INTRAMODE_BYPASSGROUP
  Void encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt absPartIdx, Bool isMultiplePU = false );
#else
  Void encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
  
  Void encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  
  Void encodeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx );
  Void encodeQtCbf             ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void encodeQtRootCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void encodeQP                ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void updateContextTables     ( SliceType eSliceType, Int iQp, Bool bExecuteFinish )   { m_pcEntropyCoderIf->updateContextTables( eSliceType, iQp, bExecuteFinish );     }
  Void updateContextTables     ( SliceType eSliceType, Int iQp )                        { m_pcEntropyCoderIf->updateContextTables( eSliceType, iQp, true );               }
#if !REMOVE_TILE_MARKERS
  Void writeTileMarker              ( UInt uiTileIdx, UInt uiBitsUsed ) { m_pcEntropyCoderIf->writeTileMarker( uiTileIdx, uiBitsUsed ); }
#endif

  Void encodeAPSInitInfo          (TComAPS* pcAPS) {m_pcEntropyCoderIf->codeAPSInitInfo(pcAPS);}
  Void encodeFinish               (Bool bEnd) {m_pcEntropyCoderIf->codeFinish(bEnd);}
  Void encodeScalingList       ( TComScalingList* scalingList );
#if !DBL_HL_SYNTAX
  Void encodeDFParams          (TComAPS* pcAPS);
#endif

private:
#if REMOVE_LASTTU_CBFDERIV
  Void xEncodeTransform        ( TComDataCU* pcCU,UInt offsetLumaOffset, UInt offsetChroma, UInt uiAbsPartIdx, UInt absTUPartIdx, UInt uiDepth, UInt width, UInt height, UInt uiTrIdx, UInt uiInnerQuadIdx,Bool& bCodeDQP );
#else
  Void xEncodeTransform        ( TComDataCU* pcCU,UInt offsetLumaOffset, UInt offsetChroma, UInt uiAbsPartIdx, UInt absTUPartIdx, UInt uiDepth, UInt width, UInt height, UInt uiTrIdx, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3, Bool& bCodeDQP );
#endif
public:
  Void encodeCoeff             ( TComDataCU* pcCU,                 UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP );
  
  Void encodeCoeffNxN         ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType );
  
  Void estimateBit             ( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType);
#if !AHG6_ALF_OPTION2
  // ALF-related
  Void codeAuxCountBit(ALFParam* pAlfParam, Int64* ruiRate);
  Void codeFiltCountBit(ALFParam* pAlfParam, Int64* ruiRate);
  Void codeAux (ALFParam* pAlfParam);
  Void codeFilt (ALFParam* pAlfParam);
  Int codeFilterCoeff(ALFParam* ALFp);
  Int writeFilterCodingParams(int minKStart, int minScanVal, int maxScanVal, int kMinTab[]);
  Int writeFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[], 
                        int **FilterCoeff, int kMinTab[]);
  Int golombEncode(int coeff, int k);
  Int lengthGolomb(int coeffVal, int k);
#endif
#if !SAO_REMOVE_APS
  Void    encodeSaoUnit(Int rx, Int ry, Int compIdx, SAOParam* saoParam, Int repeatedRow);
#endif
  Void    encodeSaoOffset(SaoLcuParam* saoLcuParam);
#if SAO_NO_MERGE_CROSS_SLICE_TILE
#if SAO_RDO_FIX
  Void    encodeSaoUnitInterleaving(Int compIdx, Bool saoFlag, Int rx, Int ry, SaoLcuParam* saoLcuParam, Int cuAddrInSlice, Int cuAddrUpInSlice, Int allowMergeLeft, Int allowMergeUp);
#else
  Void    encodeSaoUnitInterleaving(Int rx, Int ry, SAOParam* saoParam, TComDataCU* cu, Int cuAddrInSlice, Int cuAddrUpInSlice, Int allowMergeLeft, Int allowMergeUp);
#endif
#else
  Void    encodeSaoUnitInterleaving(Int rx, Int ry, SAOParam* saoParam, TComDataCU* cu, Int cuAddrInSlice, Int cuAddrUpInSlice, Bool lfCrossSliceBoundaryFlag);
#endif
#if !SAO_REMOVE_APS
  Void    encodeSaoParam         (TComAPS*  aps);
#endif
  static Int countNonZeroCoeffs( TCoeff* pcCoef, UInt uiSize );

};// END CLASS DEFINITION TEncEntropy

//! \}

#endif // __TENCENTROPY__

