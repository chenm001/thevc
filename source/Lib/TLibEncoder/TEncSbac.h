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

/** \file     TEncSbac.h
    \brief    Context-adaptive entropy encoder class (header)
*/

#ifndef __TENCSBAC__
#define __TENCSBAC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/ContextTables.h"
#include "TLibCommon/ContextModel.h"
#include "TLibCommon/ContextModel3DBuffer.h"
#include "TEncEntropy.h"
#include "TEncBinCoder.h"
#include "TEncBinCoderCABAC.h"
#if FAST_BIT_EST
#include "TEncBinCoderCABACCounter.h"
#endif

class TEncTop;

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC encoder class
class TEncSbac : public TEncEntropyIf
{
public:
  TEncSbac();
  virtual ~TEncSbac();
  
  Void  init                   ( TEncBinIf* p )  { m_pcBinIf = p; }
  Void  uninit                 ()                { m_pcBinIf = 0; }
  
  //  Virtual list
  Void  resetEntropy           ();
  Void  determineCabacInitIdx  ();
  Void  setBitstream           ( TComBitIf* p )  { m_pcBitIf = p; m_pcBinIf->init( p ); }
  Void  setSlice               ( TComSlice* p )  { m_pcSlice = p;                       }
#if !AHG6_ALF_OPTION2
  Bool  getAlfCtrl             ()                         { return m_bAlfCtrl;          }
  UInt  getMaxAlfCtrlDepth     ()                         { return m_uiMaxAlfCtrlDepth; }
  Void  setAlfCtrl             ( Bool bAlfCtrl          ) { m_bAlfCtrl          = bAlfCtrl;          }
  Void  setMaxAlfCtrlDepth     ( UInt uiMaxAlfCtrlDepth ) { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }
#endif  
  // SBAC RD
  Void  resetCoeffCost         ()                { m_uiCoeffCost = 0;  }
  UInt  getCoeffCost           ()                { return  m_uiCoeffCost;  }
  
  Void  load                   ( TEncSbac* pScr  );
  Void  loadIntraDirModeLuma   ( TEncSbac* pScr  );
  Void  store                  ( TEncSbac* pDest );
  Void  loadContexts           ( TEncSbac* pScr  );
  Void  resetBits              ()                { m_pcBinIf->resetBits(); m_pcBitIf->resetBits(); }
  UInt  getNumberOfWrittenBits ()                { return m_pcBinIf->getNumWrittenBits(); }
  //--SBAC RD

#if VPS_INTEGRATION
  Void  codeVPS                 ( TComVPS* pcVPS );
#endif
  Void  codeSPS                 ( TComSPS* pcSPS     );
  Void  codePPS                 ( TComPPS* pcPPS     );
  void codeSEI(const SEI&);
  Void  codeSliceHeader         ( TComSlice* pcSlice );
#if !REMOVE_TILE_MARKERS
  Void codeTileMarkerFlag(TComSlice* pcSlice) {printf("Not supported\n"); assert(0); exit(1);}
#endif
  Void  codeTilesWPPEntryPoint( TComSlice* pSlice );
  Void  codeTerminatingBit      ( UInt uilsLast      );
  Void  codeSliceFinish         ();
  Void  codeFlush               ();
  Void  encodeStart             ();
#if AHG6_ALF_OPTION2
  Void codeAlfParam(ALFParam* alfParam){printf("Not supported\n"); assert(0); exit(1);}
  Void codeAlfCtrlFlag( Int compIdx, UInt code );
#else
  Void  codeAlfFlag       ( UInt uiCode );
  Void  codeAlfUvlc       ( UInt uiCode );
  Void  codeAlfSvlc       ( Int  uiCode );
  Void  codeAlfCtrlDepth  ();
  Void codeAPSAlflag(UInt uiCode) {assert (0);  return;}
  Void codeAlfFixedLengthIdx( UInt idx, UInt maxValue){ assert (0);  return;}

  Void codeAlfCtrlFlag       ( UInt uiSymbol );
#endif
  Void  codeApsExtensionFlag () { assert (0); return; };
#if !SAO_CODE_CLEAN_UP
  Void  codeSaoFlag       ( UInt uiCode );
  Void  codeSaoUvlc       ( UInt uiCode );
#endif
#if SAO_TRUNCATED_U
  Void  codeSaoMaxUvlc    ( UInt code, UInt maxSymbol );
#endif
#if !(SAO_OFFSET_MAG_SIGN_SPLIT && SAO_RDO_FIX)
  Void  codeSaoSvlc       ( Int  uiCode );
#endif
#if !SAO_CODE_CLEAN_UP
  Void  codeSaoRun        ( UInt  uiCode, UInt uiMaxValue  ) {;}
#endif
  Void  codeSaoMergeLeft  ( UInt  uiCode, UInt uiCompIdx );
  Void  codeSaoMergeUp    ( UInt  uiCode);
  Void  codeSaoTypeIdx    ( UInt  uiCode);
  Void  codeSaoUflc       ( UInt  uiCode);
#if SAO_OFFSET_MAG_SIGN_SPLIT
  Void  codeSAOSign       ( UInt  uiCode);  //<! code SAO offset sign
#endif
  Void  codeScalingList      ( TComScalingList* scalingList     ){ assert (0);  return;};

private:
  Void  xWriteUnarySymbol    ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xWriteUnaryMaxSymbol ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xWriteEpExGolomb     ( UInt uiSymbol, UInt uiCount );
#if COEF_REMAIN_BINARNIZATION
  Void  xWriteCoefRemainExGolomb ( UInt symbol, UInt &rParam );
#else
  Void  xWriteGoRiceExGolomb ( UInt uiSymbol, UInt &ruiGoRiceParam );
#endif
  Void  xWriteTerminatingBit ( UInt uiBit );
  
  Void  xCopyFrom            ( TEncSbac* pSrc );
  Void  xCopyContextsFrom    ( TEncSbac* pSrc );  
  
  Void codeAPSInitInfo(TComAPS* pcAPS) {printf("Not supported in codeAPSInitInfo()\n"); assert(0); exit(1);}
  Void codeFinish     (Bool bEnd)      { m_pcBinIf->encodeFlush(bEnd); }  //<! flush bits when CABAC termination
  Void codeDFFlag( UInt uiCode, const Char *pSymbolName )       {printf("Not supported in codeDFFlag()\n"); assert(0); exit(1);};
  Void codeDFSvlc( Int iCode, const Char *pSymbolName )         {printf("Not supported in codeDFSvlc()\n"); assert(0); exit(1);};

protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;
  TEncBinIf*    m_pcBinIf;
#if !AHG6_ALF_OPTION2
  Bool          m_bAlfCtrl;
#endif  
  //SBAC RD
  UInt          m_uiCoeffCost;

#if !AHG6_ALF_OPTION2
  // Adaptive loop filter
  UInt          m_uiMaxAlfCtrlDepth;
#endif
  Int           m_iSliceGranularity; //!< slice granularity
  //--Adaptive loop filter
  
public:

  /// set slice granularity
  Void setSliceGranularity(Int iSliceGranularity)  {m_iSliceGranularity = iSliceGranularity;}

  /// get slice granularity
  Int  getSliceGranularity()                       {return m_iSliceGranularity;             }
#if !AHG6_ALF_OPTION2
  Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
#if CU_LEVEL_TRANSQUANT_BYPASS
  Void codeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif
  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codeMVPIdx        ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  
  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeIPCMInfo      ( TComDataCU* pcCU, UInt uiAbsPartIdx, Int numIPCM, Bool firstIPCMFlag);
  Void codeTransformSubdivFlag ( UInt uiSymbol, UInt uiCtx );
  Void codeQtCbf               ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeQtRootCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if INTRAMODE_BYPASSGROUP
  Void codeIntraDirLumaAng     ( TComDataCU* pcCU, UInt absPartIdx, Bool isMultiple);
#else
  Void codeIntraDirLumaAng     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#endif 
  
  Void codeIntraDirChroma      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  
  Void codeDeltaQP             ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeLastSignificantXY ( UInt uiPosX, UInt uiPosY, Int width, Int height, TextType eTType, UInt uiScanIdx );
  Void codeCoeffNxN            ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );
#if INTRA_TRANSFORMSKIP
  void codeTransformSkipFlags ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt width, UInt height, UInt uiDepth, TextType eTType );
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // for RD-optimizatioon
  // -------------------------------------------------------------------------------------------------------------------
  
  Void estBit               (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType);
  Void estCBFBit                     ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void estSignificantCoeffGroupMapBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void estSignificantMapBit          ( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType );
  Void estSignificantCoefficientsBit ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  
  Void updateContextTables           ( SliceType eSliceType, Int iQp, Bool bExecuteFinish=true  );
  Void updateContextTables           ( SliceType eSliceType, Int iQp  ) { this->updateContextTables( eSliceType, iQp, true); };
#if !REMOVE_TILE_MARKERS
  Void writeTileMarker               ( UInt uiTileIdx, UInt uiBitsUsed );
#endif
  
  TEncBinIf* getEncBinIf()  { return m_pcBinIf; }
private:
  UInt                 m_uiLastQp;
  
  ContextModel         m_contextModels[MAX_NUM_CTX_MOD];
  Int                  m_numContextModels;
  ContextModel3DBuffer m_cCUSplitFlagSCModel;
  ContextModel3DBuffer m_cCUSkipFlagSCModel;
  ContextModel3DBuffer m_cCUMergeFlagExtSCModel;
  ContextModel3DBuffer m_cCUMergeIdxExtSCModel;
  ContextModel3DBuffer m_cCUPartSizeSCModel;
  ContextModel3DBuffer m_cCUPredModeSCModel;
  ContextModel3DBuffer m_cCUAlfCtrlFlagSCModel;
  ContextModel3DBuffer m_cCUIntraPredSCModel;
  ContextModel3DBuffer m_cCUChromaPredSCModel;
  ContextModel3DBuffer m_cCUDeltaQpSCModel;
  ContextModel3DBuffer m_cCUInterDirSCModel;
  ContextModel3DBuffer m_cCURefPicSCModel;
  ContextModel3DBuffer m_cCUMvdSCModel;
  ContextModel3DBuffer m_cCUQtCbfSCModel;
  ContextModel3DBuffer m_cCUTransSubdivFlagSCModel;
  ContextModel3DBuffer m_cCUQtRootCbfSCModel;
  
  ContextModel3DBuffer m_cCUSigCoeffGroupSCModel;
  ContextModel3DBuffer m_cCUSigSCModel;
  ContextModel3DBuffer m_cCuCtxLastX;
  ContextModel3DBuffer m_cCuCtxLastY;
  ContextModel3DBuffer m_cCUOneSCModel;
  ContextModel3DBuffer m_cCUAbsSCModel;
  
  ContextModel3DBuffer m_cMVPIdxSCModel;
  
  ContextModel3DBuffer m_cALFFlagSCModel;
  ContextModel3DBuffer m_cALFUvlcSCModel;
  ContextModel3DBuffer m_cALFSvlcSCModel;
  ContextModel3DBuffer m_cCUAMPSCModel;
#if !SAO_CODE_CLEAN_UP
  ContextModel3DBuffer m_cSaoFlagSCModel;
#endif
  ContextModel3DBuffer m_cSaoUvlcSCModel;
#if !(SAO_OFFSET_MAG_SIGN_SPLIT && SAO_RDO_FIX)
  ContextModel3DBuffer m_cSaoSvlcSCModel;
#endif
  ContextModel3DBuffer m_cSaoMergeLeftSCModel;
  ContextModel3DBuffer m_cSaoMergeUpSCModel;
  ContextModel3DBuffer m_cSaoTypeIdxSCModel;
#if INTRA_TRANSFORMSKIP
  ContextModel3DBuffer m_cTransformSkipSCModel;
#endif
#if CU_LEVEL_TRANSQUANT_BYPASS
  ContextModel3DBuffer m_CUTransquantBypassFlagSCModel;
#endif
};

//! \}

#endif // !defined(AFX_TENCSBAC_H__DDA7CDC4_EDE3_4015_9D32_2156249C82AA__INCLUDED_)
