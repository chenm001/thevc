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

/** \file     TEncSbac.h
    \brief    Context-adaptive entropy encoder class (header)
*/

#ifndef __TENCSBAC__
#define __TENCSBAC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/ContextModel.h"
#include "../TLibCommon/ContextModel3DBuffer.h"
#include "TEncEntropy.h"
#include "TEncBinCoder.h"
#include "TEncBinCoderCABAC.h"
#include "TEncBinCoderMultiCABAC.h"
#include "TEncBinCoderPIPE.h"
#include "TEncBinCoderMultiPIPE.h"
#include "TEncBinCoderV2VwLB.h"

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC encoder class
class TEncSbac : public TEncEntropyIf
{
public:
  TEncSbac();
  virtual ~TEncSbac();

  Void  init                  ( TEncBinIf* p )  { m_pcBinIf = p; }
  Void  uninit                ()                { m_pcBinIf = 0; }

  //  Virtual list
  Void  resetEntropy          ();
  Void  setBitstream          ( TComBitIf* p )  { m_pcBitIf = p; m_pcBinIf->init( p ); }
  Void  setSlice              ( TComSlice* p )  { m_pcSlice = p;                       }
  Bool getAlfCtrl() {return m_bAlfCtrl;}
  UInt getMaxAlfCtrlDepth() {return m_uiMaxAlfCtrlDepth;}
  Void setAlfCtrl(Bool bAlfCtrl) {m_bAlfCtrl = bAlfCtrl;}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth;}


  // SBAC RD
  Void  resetCoeffCost        ()                { m_uiCoeffCost = 0;  }
  UInt  getCoeffCost          ()                { return  m_uiCoeffCost;  }

  Void  load                          ( TEncSbac* pScr);
  Void  store                          ( TEncSbac* pDest);
  Void  resetBits             ()                { m_pcBinIf->resetBits(); m_pcBitIf->resetBits(); }
  UInt  getNumberOfWrittenBits()                { return m_pcBinIf->getNumWrittenBits(); }
  //--SBAC RD


  Void  codeSPS                 ( TComSPS* pcSPS );
  Void  codePPS                 ( TComPPS* pcPPS );
  Void  codeSliceHeader         ( TComSlice* pcSlice );
  Void  codeTerminatingBit      ( UInt uilsLast );
  Void  codeSliceFinish         ();

  Void  codeAlfFlag       ( UInt uiCode );
  Void  codeAlfUvlc       ( UInt uiCode );
  Void  codeAlfSvlc       ( Int  uiCode );
  Void  codeAlfCtrlDepth  ();
private:
  Void  xWriteUnarySymbol    ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xWriteUnaryMaxSymbol ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xWriteEpExGolomb     ( UInt uiSymbol, UInt uiCount );
  Void  xWriteExGolombLevel  ( UInt uiSymbol, ContextModel& rcSCModel  );
  Void  xWriteTerminatingBit ( UInt uiBit );

  Void  xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart );

  __inline Void xPutByte( UChar ucByte);
  __inline Void xCarryPropagate( UInt& ruiCode );

  Void  xWriteMvd            ( Int iMvd, UInt uiAbsSum, UInt uiCtx );
  Void  xWriteExGolombMvd    ( UInt uiSymbol, ContextModel* pcSCModel, UInt uiMaxBin );

  Void  xCopyFrom            ( TEncSbac* pSrc );
  UInt  xGetCTXIdxFromWidth  ( Int iWidth );

#if PLANAR_INTRA
  Void xPutPlanarBins( Int n, Int cn );
  Void xCodePlanarDelta( TComDataCU* pcCU, UInt uiAbsPartIdx , Int iDelta );
#endif

protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;
  TEncBinIf*    m_pcBinIf;
  Bool m_bAlfCtrl;

  //SBAC RD
  UInt                  m_uiCoeffCost;

  // Adaptive loop filter
  UInt m_uiMaxAlfCtrlDepth;
  //--Adaptive loop filter

public:
  Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx );

#if HHI_ALF
  Void codeAlfCoeff      ( Int iCoeff, Int iLength, Int iPos );
  Void codeAlfDc         ( Int iDc    );
  Void codeAlfQTCtrlFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeAlfQTSplitFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth );
#endif

  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if HHI_MRG
  Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if PLANAR_INTRA
  Void codePlanarInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif

#if HHI_RQT
  Void codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx );
  Void codeQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
#endif
  Void codeTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codeIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if ANG_INTRA
  Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif

#if HHI_AIS
  Void codeIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif

  Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD = false );

  Void codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );
  Void codeCIPflag ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );

  // -------------------------------------------------------------------------------------------------------------------
  // for RD-optimizatioon
  // -------------------------------------------------------------------------------------------------------------------

  Void estBit                        ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void estCBFBit                     ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void estSignificantMapBit          ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void estSignificantCoefficientsBit ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );

  __inline Int  biari_no_bits        ( Short symbol, ContextModel& rcSCModel );
  Int  biari_state                   ( Short symbol, ContextModel& rcSCModel );

private:
  UInt m_uiLastQp;
  ContextModel3DBuffer m_cCUSplitFlagSCModel;

  ContextModel3DBuffer m_cCUSkipFlagSCModel;
#if HHI_MRG
  ContextModel3DBuffer m_cCUMergeFlagSCModel;
  ContextModel3DBuffer m_cCUMergeIndexSCModel;
#endif
  ContextModel3DBuffer m_cCUPartSizeSCModel;
  ContextModel3DBuffer m_cCUPredModeSCModel;
  ContextModel3DBuffer m_cCUAlfCtrlFlagSCModel;
  ContextModel3DBuffer m_cCUIntraPredSCModel;
#if HHI_AIS
  ContextModel3DBuffer m_cCUIntraFiltFlagSCModel;
#endif
  ContextModel3DBuffer m_cCUChromaPredSCModel;
  ContextModel3DBuffer m_cCUDeltaQpSCModel;
  ContextModel3DBuffer m_cCUInterDirSCModel;
  ContextModel3DBuffer m_cCURefPicSCModel;
  ContextModel3DBuffer m_cCUMvdSCModel;
  ContextModel3DBuffer m_cCUCbfSCModel;
#if HHI_RQT
  ContextModel3DBuffer m_cCUQtCbfSCModel;
  ContextModel3DBuffer m_cCUTransSubdivFlagSCModel;
#endif
  ContextModel3DBuffer m_cCUTransIdxSCModel;

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

#endif // !defined(AFX_TENCSBAC_H__DDA7CDC4_EDE3_4015_9D32_2156249C82AA__INCLUDED_)
