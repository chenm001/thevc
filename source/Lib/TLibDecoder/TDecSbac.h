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

/** \file			TDecSbac.h
    \brief		SBAC decoder class (header)
*/

#ifndef __TDECSBAC__
#define __TDECSBAC__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "TDecEntropy.h"

#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/SbacTables.h"
#include "../TLibCommon/SbacContextModel.h"
#include "../TLibCommon/SbacContextModel3DBuffer.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC decoder class
class TDecSbac : public TDecEntropyIf
{
public:
  TDecSbac();
  virtual ~TDecSbac();

  Void  resetEntropy              (TComSlice* pcSlice);
  Void  setBitstream              (TComBitstream* p)        { m_pcBitstream = p;  }
  Void  preloadSbac              ();
  Void setAlfCtrl(Bool bAlfCtrl) {m_bAlfCtrl = bAlfCtrl;}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth;}

	Void  parseSPS									(TComSPS* pcSPS) {}
  Void  parseSliceHeader          (TComSlice*& rpcSlice)    {}
  Void  parseTerminatingBit       ( UInt& ruiBit );

  //  for debugging
  UInt  getRange()                { return  m_uiRange;  }
  UInt  getValue()                { return  m_uiValue;  }
  //--

  Void parseMVPIdx				( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );

  Void parseAlfFlag				( UInt& ruiVal						);
  Void parseAlfUvlc				( UInt& ruiVal						);
  Void parseAlfSvlc				( Int&  riVal							);
  Void parseAlfCtrlDepth 	( UInt& ruiAlfCtrlDepth		);

private:
  __inline Void  xReadSymbol         ( UInt& ruiSymbol, SbacContextModel& rcSCModel );

  Void  xReadUnarySymbol    ( UInt& ruiSymbol, SbacContextModel* pcSCModel, Int iOffset );
  Void  xReadEpSymbol       ( UInt& ruiSymbol );
  Void  xReadUnaryMaxSymbol ( UInt& ruiSymbol, SbacContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xReadEpExGolomb     ( UInt& ruiSymbol, UInt uiCount );
  Void  xReadExGolombLevel  ( UInt& ruiSymbol, SbacContextModel& rcSCModel  );
  Void  xReadTerminatingBit ( UInt& ruiBit );

  __inline Void xReadBit( UInt& ruiValue );

  Void  xReadMvd            ( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx );
  Void  xReadExGolombMvd    ( UInt& ruiSymbol, SbacContextModel* pcSCModel, UInt uiMaxBin );

private:
  TComBitstream*    m_pcBitstream;
  UInt              m_uiRange;
  UInt              m_uiValue;
  UInt              m_uiWord;
  UInt              m_uiBitsLeft;
  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;

public:
  Void parseAlfCtrlFlag	   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void parseInterDir      ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseRefFrmIdx     ( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  Void parseMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList );

  Void parseTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void parseCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );

  Void parseROTindex      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCIPflag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

private:
  UInt m_uiLastQp;

  SbacContextModel3DBuffer m_cCUSkipFlagSCModel;
  SbacContextModel3DBuffer m_cCUSplitFlagSCModel;
  SbacContextModel3DBuffer m_cCUAlfCtrlFlagSCModel;
  SbacContextModel3DBuffer m_cCUPartSizeSCModel;
  SbacContextModel3DBuffer m_cCUPredModeSCModel;

  SbacContextModel3DBuffer m_cCUIntraPredSCModel;
  SbacContextModel3DBuffer m_cCUChromaPredSCModel;
  SbacContextModel3DBuffer m_cCUInterDirSCModel;
  SbacContextModel3DBuffer m_cCURefPicSCModel;
  SbacContextModel3DBuffer m_cCUMvdSCModel;

  SbacContextModel3DBuffer m_cCUTransIdxSCModel;
  SbacContextModel3DBuffer m_cCUDeltaQpSCModel;
  SbacContextModel3DBuffer m_cCUCbfSCModel;

  SbacContextModel3DBuffer m_cCUMapSCModel;
  SbacContextModel3DBuffer m_cCULastSCModel;
  SbacContextModel3DBuffer m_cCUOneSCModel;
  SbacContextModel3DBuffer m_cCUAbsSCModel;

  SbacContextModel3DBuffer m_cMVPIdxSCModel;

  SbacContextModel3DBuffer m_cCUROTindexSCModel;
  SbacContextModel3DBuffer m_cCUCIPflagCCModel;

	SbacContextModel3DBuffer m_cALFFlagSCModel;
	SbacContextModel3DBuffer m_cALFUvlcSCModel;
	SbacContextModel3DBuffer m_cALFSvlcSCModel;
  SbacContextModel3DBuffer m_cCUXPosiSCModel;
  SbacContextModel3DBuffer m_cCUYPosiSCModel;
};

#endif // !defined(AFX_TDECSBAC_H__CFCAAA19_8110_47F4_9A16_810C4B5499D5__INCLUDED_)
