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

/** \file     TDecEntropy.h
    \brief    entropy decoder class (header)
*/

#ifndef __TDECENTROPY__
#define __TDECENTROPY__

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/TComSlice.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPrediction.h"
#include "../TLibCommon/TComAdaptiveLoopFilter.h"
#ifdef DCM_PBIC
#include "../TLibCommon/ContextModel.h"
#endif

class TDecSbac;
class TDecCavlc;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// entropy decoder pure class
class TDecEntropyIf
{
public:
  //  Virtual list for SBAC/CAVLC
  virtual Void setAlfCtrl(Bool bAlfCtrl)  = 0;
  virtual Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth)  = 0;

  virtual Void  resetEntropy          (TComSlice* pcSlice)                = 0;
  virtual Void  setBitstream          ( TComBitstream* p )  = 0;

  virtual Void  parseSPS                  ( TComSPS* pcSPS )                                      = 0;
  virtual Void  parsePPS                  ( TComPPS* pcPPS )                                      = 0;
  virtual Void  parseSliceHeader          ( TComSlice*& rpcSlice )                                = 0;
  virtual Void  parseTerminatingBit       ( UInt& ruilsLast )                                     = 0;
#ifdef QC_SIFO
  virtual Void  parseSwitched_Filters      (TComSlice*& rpcSlice, TComPrediction* m_cPrediction)                                 = 0;
#endif

  virtual Void parseMVPIdx      ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;
#ifdef DCM_PBIC
  virtual Void parseICPIdx      ( TComDataCU* pcCU, Int& riICPIdx, Int iICPNum, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#endif
  
public:
  virtual Void parseSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#if HHI_MRG
#if HHI_MRG_PU
  virtual Void parseMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx ) = 0;
  virtual Void parseMergeIndex    ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#else
  virtual Void parseMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#endif
#endif
  virtual Void parsePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parsePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;

  virtual Void parseIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#if ANG_INTRA
  virtual Void parseIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#endif

#if HHI_AIS
  virtual Void parseIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#endif

  virtual Void parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;

  virtual Void parseInterDir      ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseRefFrmIdx     ( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;
  virtual Void parseMvd           ( TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;

#ifdef DCM_PBIC
  virtual Void parseMvdIcd        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;
  virtual ContextModel* getZTreeCtx ( Int iIdx ) = 0;
#endif

#if HHI_RQT
  virtual Void parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize ) = 0;
  virtual Void parseQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth ) = 0;
#if HHI_RQT_ROOT
  virtual Void parseQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf ) = 0;
#endif
#endif

  virtual Void parseTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth ) = 0;

  virtual Void parseROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseCIPflag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#if PLANAR_INTRA
  virtual Void parsePlanarInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#endif

  virtual Void parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType ) = 0;

  virtual Void parseAlfFlag       ( UInt& ruiVal           ) = 0;
  virtual Void parseAlfUvlc       ( UInt& ruiVal           ) = 0;
  virtual Void parseAlfSvlc       ( Int&  riVal            ) = 0;
  virtual Void parseAlfCtrlDepth  ( UInt& ruiAlfCtrlDepth  ) = 0;
  virtual Void parseAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
#if TSB_ALF_HEADER
  virtual Void parseAlfFlagNum    ( UInt& ruiVal, UInt minValue, UInt depth ) = 0;
  virtual Void parseAlfCtrlFlag   ( UInt &ruiAlfCtrlFlag ) = 0;
#endif

#if HHI_ALF
  virtual Void parseAlfCoeff      ( Int&  riCoeff, Int iLength, Int iPos                               ) = 0;
  virtual Void parseAlfDc         ( Int&  riDc                                                         ) = 0;
  virtual Void parseAlfQTCtrlFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseAlfQTSplitFlag( TComDataCU* pcCU ,UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth ) = 0;
#endif
};

/// entropy decoder class
class TDecEntropy
{
private:
  TDecEntropyIf*  m_pcEntropyDecoderIf;
  TComPrediction* m_pcPrediction;

public:
  Void init (TComPrediction* p) {m_pcPrediction = p;}
  Void decodeMVPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList, TComDataCU* pcSubCU );
#if HHI_MRG
#if HHI_MRG_PU
  Void decodePUWise       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
  Void decodeInterDirPU   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx );
  Void decodeRefFrmIdxPU  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
  Void decodeMvdPU        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
  Void decodeMVPIdxPU     ( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
#ifdef DCM_PBIC
  Void decodeMvdIcdPU     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx );
  Void decodeICPIdxPU     ( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx );
#endif
#else
  Void decodeMergeInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
#endif
#endif
#ifdef DCM_PBIC
  Void decodeICPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
#endif

  Void    setEntropyDecoder           ( TDecEntropyIf* p );
  Void    setBitstream                ( TComBitstream* p )      { m_pcEntropyDecoderIf->setBitstream(p);                    }
  Void    resetEntropy                ( TComSlice* p)           { m_pcEntropyDecoderIf->resetEntropy(p);                    }

  Void    decodeSPS                   ( TComSPS* pcSPS     )    { m_pcEntropyDecoderIf->parseSPS(pcSPS);                    }
  Void    decodePPS                   ( TComPPS* pcPPS     )    { m_pcEntropyDecoderIf->parsePPS(pcPPS);                    }
  Void    decodeSliceHeader           ( TComSlice*& rpcSlice )  { m_pcEntropyDecoderIf->parseSliceHeader(rpcSlice);         }
  Void    decodeTerminatingBit        ( UInt& ruiIsLast )       { m_pcEntropyDecoderIf->parseTerminatingBit(ruiIsLast);     }
#ifdef QC_SIFO
  Void    decodeSwitched_Filters      ( TComSlice*& rpcSlice, TComPrediction* m_cPrediction )  { m_pcEntropyDecoderIf->parseSwitched_Filters(rpcSlice,m_cPrediction);		}
#endif

  // Adaptive Loop filter
#if HHI_ALF
  Void decodeAlfParam(ALFParam* pAlfParam , TComPic* pcPic );
#else
  Void decodeAlfParam(ALFParam* pAlfParam);
#endif
  //--Adaptive Loop filter

  TDecEntropyIf* getEntropyDecoder() { return m_pcEntropyDecoderIf; }

public:
  Void decodeSplitFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeSkipFlag          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if HHI_MRG
#if HHI_MRG_PU
  Void decodeMergeFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx );
  Void decodeMergeIndex        ( TComDataCU* pcSubCU, UInt uiPartIdx, UInt uiPartAddr, PartSize eCUMode, UInt uiDepth );
#else
  Void decodeMergeFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeMergeIndex        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
#endif
  Void decodeAlfCtrlFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if TSB_ALF_HEADER
  Void decodeAlfCtrlParam      ( ALFParam *pAlfParam );
#endif

#if HHI_ALF
  Void decodeAlfQuadTree       ( TComPicSym* pcQuadTree, UInt uiMaxDepth );
  Void decodeAlfQuadTreeNode   ( TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiabsPartIdx, UInt uiDepth, UInt uiMaxDepth);
#endif

  Void decodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void decodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
#if PLANAR_INTRA
  Void decodePlanarInfo        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  Void decodeROTIdx            ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeCIPflag           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#if HHI_AIS
  Void decodeIntraFiltFlagLuma ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
  Void decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeInterDir          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeRefFrmIdx         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  Void decodeMvd               ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
#ifdef DCM_PBIC
  Void decodeMvdIcd            ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
#endif

  Void decodeTransformIdx      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeQP                ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );



#if HHI_RQT
private:
  Void xDecodeTransformSubdiv  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx );
#endif

#ifdef QC_AMVRES
Void xDecodeMvRes(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList, Int &iRefFrmIdx, Int iParseRefFrmIdx, PartSize ePartSize,UInt PartIdx);
#endif
  Void xDecodeCoeff            ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType );
#if HHI_RQT
public:
#endif
  Void decodeCoeff             ( TComDataCU* pcCU                 , UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight );
#if QC_ALF
  Void decodeAux(ALFParam* pAlfParam);
  Void decodeFilt(ALFParam* pAlfParam);
  Void readFilterCodingParams(ALFParam* pAlfParam);
  Void readFilterCoeffs(ALFParam* pAlfParam);
  Void decodeFilterCoeff (ALFParam* pAlfParam);
  Int golombDecode(Int k);
#endif
};// END CLASS DEFINITION TDecEntropy


#endif // __TDECENTROPY__

