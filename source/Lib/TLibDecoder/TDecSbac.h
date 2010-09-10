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

/** \file     TDecSbac.h
    \brief    SBAC decoder class (header)
*/

#ifndef __TDECSBAC__
#define __TDECSBAC__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "TDecEntropy.h"
#include "TDecBinCoder.h"
#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/ContextModel3DBuffer.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC decoder class
class TDecSbac : public TDecEntropyIf
{
public:
  TDecSbac();
  virtual ~TDecSbac();

  Void  init                      ( TDecBinIf* p )    { m_pcTDecBinIf = p; }
  Void  uninit                    ()                  { m_pcTDecBinIf = 0; }

  Void  resetEntropy              (TComSlice* pcSlice);
  Void  setBitstream              (TComBitstream* p)        { m_pcBitstream = p; m_pcTDecBinIf->init( p ); }

  Void setAlfCtrl(Bool bAlfCtrl) {m_bAlfCtrl = bAlfCtrl;}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth;}

  Void  parseSPS                  (TComSPS* pcSPS) {}
  Void  parsePPS                  (TComPPS* pcPPS) {}
  Void  parseSliceHeader          (TComSlice*& rpcSlice)    {}
  Void  parseTerminatingBit       ( UInt& ruiBit );
#ifdef QC_SIFO
  Void  parseSwitched_Filters      (TComSlice*& rpcSlice, TComPrediction* m_cPrediction)   {}
#endif
  Void parseMVPIdx        ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
#ifdef DCM_PBIC
  Void parseICPIdx        ( TComDataCU* pcCU, Int& riICPIdx, Int iICPNum, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  Void parseAlfFlag       ( UInt& ruiVal            );
  Void parseAlfUvlc       ( UInt& ruiVal            );
  Void parseAlfSvlc       ( Int&  riVal             );
  Void parseAlfCtrlDepth  ( UInt& ruiAlfCtrlDepth   );

private:
  Void  xReadUnarySymbol    ( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xReadUnaryMaxSymbol ( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xReadEpExGolomb     ( UInt& ruiSymbol, UInt uiCount );
  Void  xReadExGolombLevel  ( UInt& ruiSymbol, ContextModel& rcSCModel  );

  Void  xReadMvd            ( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx );
#ifdef DCM_PBIC
  Void  xReadMvdNZ          ( Int& riMvdComp, UInt uiCtx );
  Void  xReadIcdNZ          ( Int& riIcdComp, UInt uiCtx );
  Void  xReadExGolombIcd    ( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiMaxBin );
#endif

  Void  xReadExGolombMvd    ( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiMaxBin );
#ifdef QC_AMVRES
  Void xReadMvResFlag ( Int& riVal, UInt uiCtx );
#ifdef DCM_PBIC
  Bool xParseMvResFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList );
#endif
#endif
#if PLANAR_INTRA
  UInt xParsePlanarBins( );
  Int  xParsePlanarDelta( TextType ttText );
#endif

private:
  TComBitstream*    m_pcBitstream;
  TDecBinIf*        m_pcTDecBinIf;

  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;

public:
  Void parseAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

#if HHI_ALF
  Void parseAlfCoeff      ( Int& riCoeff, Int iLength, Int iPos                                );
  Void parseAlfDc         ( Int& riDc                                                          );
  Void parseAlfQTCtrlFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseAlfQTSplitFlag( TComDataCU* pcCU ,UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth );
#endif

  Void parseSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if HHI_MRG
#if HHI_MRG_PU
  Void parseMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx );
  Void parseMergeIndex    ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth );
#else
  Void parseMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
#endif
  Void parsePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if ANG_INTRA
  Void parseIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif

#if HHI_AIS
  Void parseIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  Void parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseInterDir      ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseRefFrmIdx     ( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  Void parseMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList );
#ifdef DCM_PBIC
  Void parseMvdIcd        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList );
  Int  parseZTree         ( TComZeroTree* pcZTree, ContextModel *pcCtxModel);
  ContextModel* getZTreeCtx ( Int iIdx );
#endif

#if HHI_RQT
  Void parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize );
  Void parseQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
#if HHI_RQT_ROOT
  Void parseQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf );
#endif
#endif

  Void parseTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void parseCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );

  Void parseROTindex      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCIPflag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if PLANAR_INTRA
  Void parsePlanarInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif

private:
  UInt m_uiLastDQpNonZero;
  UInt m_uiLastQp;

  ContextModel3DBuffer m_cCUSkipFlagSCModel;
  ContextModel3DBuffer m_cCUSplitFlagSCModel;
#if HHI_MRG
  ContextModel3DBuffer m_cCUMergeFlagSCModel;
  ContextModel3DBuffer m_cCUMergeIndexSCModel;
#endif
  ContextModel3DBuffer m_cCUAlfCtrlFlagSCModel;
  ContextModel3DBuffer m_cCUPartSizeSCModel;
  ContextModel3DBuffer m_cCUPredModeSCModel;

  ContextModel3DBuffer m_cCUIntraPredSCModel;
#if HHI_AIS
  ContextModel3DBuffer m_cCUIntraFiltFlagSCModel;
#endif
  ContextModel3DBuffer m_cCUChromaPredSCModel;
  ContextModel3DBuffer m_cCUInterDirSCModel;
  ContextModel3DBuffer m_cCURefPicSCModel;
  ContextModel3DBuffer m_cCUMvdSCModel;
#ifdef QC_AMVRES 
  ContextModel3DBuffer m_cCUMvResCModel;
#endif
#ifdef DCM_PBIC
  ContextModel3DBuffer m_cCUIcdSCModel;
#endif

#if HHI_RQT
  ContextModel3DBuffer m_cCUTransSubdivFlagSCModel;
#if HHI_RQT_ROOT
  ContextModel3DBuffer m_cCUQtRootCbfSCModel;
#endif
#endif
  ContextModel3DBuffer m_cCUTransIdxSCModel;
  ContextModel3DBuffer m_cCUDeltaQpSCModel;
  ContextModel3DBuffer m_cCUCbfSCModel;

#if HHI_RQT
  ContextModel3DBuffer m_cCUQtCbfSCModel;
#endif

#if HHI_TRANSFORM_CODING
  ContextModel3DBuffer m_cCuCtxModSig;
  ContextModel3DBuffer m_cCuCtxModLast;
  ContextModel3DBuffer m_cCuCtxModAbsGreOne;
  ContextModel3DBuffer m_cCuCtxModCoeffLevelM1;
#else
  ContextModel3DBuffer m_cCUMapSCModel;
  ContextModel3DBuffer m_cCULastSCModel;
  ContextModel3DBuffer m_cCUOneSCModel;
  ContextModel3DBuffer m_cCUAbsSCModel;
#endif

  ContextModel3DBuffer m_cMVPIdxSCModel;
#ifdef DCM_PBIC
  ContextModel3DBuffer m_cICPIdxSCModel;
  ContextModel3DBuffer m_cZTreeMV0SCModel;
  ContextModel3DBuffer m_cZTreeMV1SCModel;
  ContextModel3DBuffer m_cZTreeMV2SCModel;
#endif

  ContextModel3DBuffer m_cCUROTindexSCModel;
  ContextModel3DBuffer m_cCUCIPflagCCModel;

#if HHI_ALF
  ContextModel3DBuffer m_cALFSplitFlagSCModel;
#endif
  ContextModel3DBuffer m_cALFFlagSCModel;
  ContextModel3DBuffer m_cALFUvlcSCModel;
  ContextModel3DBuffer m_cALFSvlcSCModel;
  ContextModel3DBuffer m_cCUXPosiSCModel;
  ContextModel3DBuffer m_cCUYPosiSCModel;
#if PLANAR_INTRA
  ContextModel3DBuffer m_cPlanarIntraSCModel;
#endif
};

#endif // !defined(AFX_TDECSBAC_H__CFCAAA19_8110_47F4_9A16_810C4B5499D5__INCLUDED_)
