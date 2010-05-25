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

/** \file			TEncSbac.h
    \brief		SBAC encoder class (header)
*/

#ifndef __TENCSBAC__
#define __TENCSBAC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/TComBitStream.h"
#include "../TLibCommon/ContextTables.h"
#include "../TLibCommon/SbacTables.h"
#include "../TLibCommon/SbacContextModel.h"
#include "TEncEntropy.h"

#include "../TLibCommon/SbacContextModel3DBuffer.h"

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

private:
  Void  xWriteSymbol         ( UInt uiSymbol, SbacContextModel& rcSCModel );
  Void  xWriteUnarySymbol    ( UInt uiSymbol, SbacContextModel* pcSCModel, Int iOffset );
  Void  xWriteEPSymbol       ( UInt uiSymbol );
  Void  xWriteUnaryMaxSymbol ( UInt uiSymbol, SbacContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol );
  Void  xWriteEpExGolomb     ( UInt uiSymbol, UInt uiCount );
  Void  xWriteExGolombLevel  ( UInt uiSymbol, SbacContextModel& rcSCModel  );
  Void  xWriteTerminatingBit ( UInt uiBit );

  Void  xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart );

  __inline Void xPutByte( UChar ucByte);
  __inline Void xCarryPropagate( UInt& ruiCode );

  Void  xWriteMvd            ( Int iMvd, UInt uiAbsSum, UInt uiCtx );
  Void  xWriteExGolombMvd    ( UInt uiSymbol, SbacContextModel* pcSCModel, UInt uiMaxBin );

  Void  xCopyFrom						 ( TEncSbac* pSrc );
	UInt	xGetCTXIdxFromWidth	 ( Int iWidth );

protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;

	Bool	m_bAlfCtrl;
  UInt  m_uiRange;
  UInt  m_uiCode;
  UInt  m_uiCodeBits;
  UInt  m_uiStackedFFs;
  UInt  m_uiStackedZeros;
  UChar m_ucPendingByte;
  Bool  m_bIsPendingByte;

  UInt	m_uiCoeffCost;
  UInt	m_uiMaxAlfCtrlDepth;

public:
  Void  resetEntropy						();
  Void  setBitstream						( TComBitIf* p )  { m_pcBitIf = p; }
  Void  setSlice								( TComSlice* p )  { m_pcSlice = p; }

  Void  resetCoeffCost					()	{ m_uiCoeffCost = 0;			}
  UInt  getCoeffCost						()  { return  m_uiCoeffCost;  }

  Bool	getAlfCtrl							() { return m_bAlfCtrl; }
  UInt	getMaxAlfCtrlDepth			() { return m_uiMaxAlfCtrlDepth; }
  Void	setAlfCtrl							(	Bool bAlfCtrl ) { m_bAlfCtrl = bAlfCtrl; }
  Void	setMaxAlfCtrlDepth			( UInt uiMaxAlfCtrlDepth ) { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }

	// -------------------------------------------------------------------------------------------------------------------
	// for syntax writing
	// -------------------------------------------------------------------------------------------------------------------

	Void  codeSPS									( TComSPS* pcSPS );
  Void  codeSliceHeader         ( TComSlice* pcSlice );
  Void  codeTerminatingBit      ( UInt uilsLast );
  Void  codeSliceFinish         ();

  Void	codeAlfFlag							( UInt uiCode );
  Void	codeAlfUvlc							( UInt uiCode );
  Void	codeAlfSvlc							( Int  uiCode );
  Void  codeAlfCtrlDepth				();

  Void	codeAlfCtrlFlag					( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void	codeSkipFlag						( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void	codeSplitFlag						( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void	codeMVPIdx							( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void	codePartSize						( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void	codePredMode						( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void	codeTransformIdx				( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void	codeIntraDirLumaAdi			( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void	codeIntraDirChroma			( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void	codeInterDir						( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void	codeRefFrmIdx						( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void	codeMvd									( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void	codeDeltaQP							( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void	codeCbf									( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void	codeCoeffNxN						( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD = false );

  Void	codeROTindex						( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );
  Void	codeCIPflag							( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );

	// -------------------------------------------------------------------------------------------------------------------
	// for RD-optimizatioon
	// -------------------------------------------------------------------------------------------------------------------

  Void  load													( TEncSbac* pScr);
  Void  store													( TEncSbac* pDest);
  Void  resetBits											() { m_uiCode &= 0x7FFFF; m_uiCodeBits = 11; m_ucPendingByte = 0; m_bIsPendingByte = false; m_uiStackedFFs = 0; m_uiStackedZeros = 0;  m_pcBitIf->resetBits(); }
  UInt  getNumberOfWrittenBits				() { return  m_pcBitIf->getNumberOfWrittenBits() + 8 * (m_uiStackedZeros + m_uiStackedFFs) + 8 * (m_bIsPendingByte ? 1 : 0) + 8 - m_uiCodeBits + 3; }
  Void	estBit                        ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void	estCBFBit                     ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void	estSignificantMapBit          ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
  Void	estSignificantCoefficientsBit ( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType );
	__inline Int  biari_no_bits					( Short symbol, SbacContextModel& rcSCModel );
  Int  biari_state										( Short symbol, SbacContextModel& rcSCModel );

private:
  UInt m_uiLastQp;
  SbacContextModel3DBuffer m_cCUSplitFlagSCModel;

  SbacContextModel3DBuffer m_cCUSkipFlagSCModel;
  SbacContextModel3DBuffer m_cCUPartSizeSCModel;
  SbacContextModel3DBuffer m_cCUPredModeSCModel;
  SbacContextModel3DBuffer m_cCUAlfCtrlFlagSCModel;
  SbacContextModel3DBuffer m_cCUIntraPredSCModel;
  SbacContextModel3DBuffer m_cCUChromaPredSCModel;
  SbacContextModel3DBuffer m_cCUDeltaQpSCModel;
  SbacContextModel3DBuffer m_cCUInterDirSCModel;
  SbacContextModel3DBuffer m_cCURefPicSCModel;
  SbacContextModel3DBuffer m_cCUMvdSCModel;
  SbacContextModel3DBuffer m_cCUCbfSCModel;
  SbacContextModel3DBuffer m_cCUTransIdxSCModel;

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

#endif // !defined(AFX_TENCSBAC_H__DDA7CDC4_EDE3_4015_9D32_2156249C82AA__INCLUDED_)
