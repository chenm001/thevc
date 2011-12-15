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
  Void  setBitstream           ( TComBitIf* p )  { m_pcBitIf = p; m_pcBinIf->init( p ); }
  Void  setSlice               ( TComSlice* p )  { m_pcSlice = p;                       }
  
  Bool  getAlfCtrl             ()                         { return m_bAlfCtrl;          }
  UInt  getMaxAlfCtrlDepth     ()                         { return m_uiMaxAlfCtrlDepth; }
  Void  setAlfCtrl             ( Bool bAlfCtrl          ) { m_bAlfCtrl          = bAlfCtrl;          }
  Void  setMaxAlfCtrlDepth     ( UInt uiMaxAlfCtrlDepth ) { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }
  
  // SBAC RD
  Void  resetCoeffCost         ()                { m_uiCoeffCost = 0;  }
  UInt  getCoeffCost           ()                { return  m_uiCoeffCost;  }
  
  Void  load                   ( TEncSbac* pScr  );
  Void  loadIntraDirModeLuma   ( TEncSbac* pScr  );
  Void  store                  ( TEncSbac* pDest );
#if OL_USE_WPP
  Void  loadContexts           ( TEncSbac* pScr  );
#endif
  Void  resetBits              ()                { m_pcBinIf->resetBits(); m_pcBitIf->resetBits(); }
  UInt  getNumberOfWrittenBits ()                { return m_pcBinIf->getNumWrittenBits(); }
  //--SBAC RD

  Void  codeSPS                 ( TComSPS* pcSPS     );
  Void  codePPS                 ( TComPPS* pcPPS     );
  void codeSEI(const SEI&);
  Void  codeSliceHeader         ( TComSlice* pcSlice );
#if G220_PURE_VLC_SAO_ALF
#if TILES_DECODER
  Void codeTileMarkerFlag(TComSlice* pcSlice) {printf("Not supported\n"); assert(0); exit(1);}
#endif
#endif
#if OL_USE_WPP
  Void  codeSliceHeaderSubstreamTable( TComSlice* pcSlice );
#endif
  Void  codeTerminatingBit      ( UInt uilsLast      );
  Void  codeSliceFinish         ();
#if OL_FLUSH
  Void  codeFlush               ();
  Void  encodeStart             ();
#endif
  
  Void  codeAlfFlag       ( UInt uiCode );
  Void  codeAlfUvlc       ( UInt uiCode );
  Void  codeAlfSvlc       ( Int  uiCode );
  Void  codeAlfCtrlDepth  ();

  /// Code number of ALF CU control flags
  Void codeAlfFlagNum        ( UInt uiCode, UInt minValue, Int iDepth);

  Void codeAlfCtrlFlag       ( UInt uiSymbol );
#if SAO
  Void  codeSaoFlag       ( UInt uiCode );
  Void  codeSaoUvlc       ( UInt uiCode );
  Void  codeSaoSvlc       ( Int  uiCode );
#endif

private:
  Void  xWriteUnarySymbol    ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset );
  Void  xWriteUnaryMaxSymbol ( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xWriteEpExGolomb     ( UInt uiSymbol, UInt uiCount );
  Void  xWriteGoRiceExGolomb ( UInt uiSymbol, UInt &ruiGoRiceParam );
  Void  xWriteTerminatingBit ( UInt uiBit );
  
  Void  xCopyFrom            ( TEncSbac* pSrc );
#if OL_USE_WPP
  Void  xCopyContextsFrom    ( TEncSbac* pSrc );  
#endif
  
#if F747_APS
  Void codeAPSInitInfo(TComAPS* pcAPS) {printf("Not supported in codeAPSInitInfo()\n"); assert(0); exit(1);}
  Void codeFinish     (Bool bEnd)      { m_pcBinIf->encodeFlush(bEnd); }  //<! flush bits when CABAC termination
#endif

protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;
  TEncBinIf*    m_pcBinIf;
  Bool          m_bAlfCtrl;
  
  //SBAC RD
  UInt          m_uiCoeffCost;
  
  // Adaptive loop filter
  UInt          m_uiMaxAlfCtrlDepth;
#if FINE_GRANULARITY_SLICES
  Int           m_iSliceGranularity; //!< slice granularity
#endif
  //--Adaptive loop filter
  
public:

#if FINE_GRANULARITY_SLICES
  /// set slice granularity
  Void setSliceGranularity(Int iSliceGranularity)  {m_iSliceGranularity = iSliceGranularity;}

  /// get slice granularity
  Int  getSliceGranularity()                       {return m_iSliceGranularity;             }
#endif
  Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codeMVPIdx        ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  
  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeIPCMInfo      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeTransformSubdivFlag ( UInt uiSymbol, UInt uiCtx );
  Void codeQtCbf               ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeQtRootCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeIntraDirLumaAng     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeIntraDirChroma      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir            ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  
  Void codeDeltaQP             ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeCbf                 ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth ) {}
  Void codeBlockCbf            ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiQPartNum, Bool bRD = false ) {}
  
  Void codeCbfTrdiv      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) {}
  UInt xGetFlagPattern   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) { return 0; }
  Void codeLastSignificantXY ( UInt uiPosX, UInt uiPosY, Int width, Int height, TextType eTType, UInt uiScanIdx );
  Void codeCoeffNxN            ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );
  
  // -------------------------------------------------------------------------------------------------------------------
  // for RD-optimizatioon
  // -------------------------------------------------------------------------------------------------------------------
  
#if NSQT_DIAG_SCAN
  Void estBit               (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType);
#else
  Void estBit                        ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
#endif
  Void estCBFBit                     ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
#if MULTI_LEVEL_SIGNIFICANCE
  Void estSignificantCoeffGroupMapBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
#endif
#if NSQT_DIAG_SCAN
  Void estSignificantMapBit          ( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType );
#else
  Void estSignificantMapBit          ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
#endif
  Void estSignificantCoefficientsBit ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  

#if TILES
  Void updateContextTables           ( SliceType eSliceType, Int iQp, Bool bExecuteFinish=true  );
  Void updateContextTables           ( SliceType eSliceType, Int iQp  ) { this->updateContextTables( eSliceType, iQp, true); };
#if TILES_DECODER
  Void writeTileMarker               ( UInt uiTileIdx, UInt uiBitsUsed );
#endif
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
  
#if MULTI_LEVEL_SIGNIFICANCE
  ContextModel3DBuffer m_cCUSigCoeffGroupSCModel;
#endif
#if SIGMAP_CTX_RED
  ContextModel3DBuffer m_cCUSigSCModelLuma;
  ContextModel3DBuffer m_cCUSigSCModelChroma;
#else
  ContextModel3DBuffer m_cCUSigSCModel;
#endif
  ContextModel3DBuffer m_cCuCtxLastX;
  ContextModel3DBuffer m_cCuCtxLastY;
#if COEFF_CTXSET_RED
  ContextModel3DBuffer m_cCUOneSCModelLuma;
  ContextModel3DBuffer m_cCUOneSCModelChroma;
  ContextModel3DBuffer m_cCUAbsSCModelLuma;
  ContextModel3DBuffer m_cCUAbsSCModelChroma;
#else
  ContextModel3DBuffer m_cCUOneSCModel;
  ContextModel3DBuffer m_cCUAbsSCModel;
#endif
  
  ContextModel3DBuffer m_cMVPIdxSCModel;
  
  ContextModel3DBuffer m_cALFFlagSCModel;
  ContextModel3DBuffer m_cALFUvlcSCModel;
  ContextModel3DBuffer m_cALFSvlcSCModel;
#if AMP
  ContextModel3DBuffer m_cCUXPosiSCModel;
  ContextModel3DBuffer m_cCUYPosiSCModel;
#endif
#if SAO
  ContextModel3DBuffer m_cSaoFlagSCModel;
  ContextModel3DBuffer m_cSaoUvlcSCModel;
  ContextModel3DBuffer m_cSaoSvlcSCModel;
#endif

};

//! \}

#endif // !defined(AFX_TENCSBAC_H__DDA7CDC4_EDE3_4015_9D32_2156249C82AA__INCLUDED_)
