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

/** \file			TEncCavlc.h
    \brief		CAVLC encoder class (header)
*/

#ifndef __TENCCAVLC__
#define __TENCCAVLC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComBitStream.h"
#include "TEncEntropy.h"

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CAVLC encoder class
class TEncCavlc : public TEncEntropyIf
{
public:
  TEncCavlc();
  virtual ~TEncCavlc();

  //  Virtual list for SBAC/CAVLC
  Void  resetEntropy          ();
  Void  setBitstream          ( TComBitIf* p )  { m_pcBitIf = p;  }
  Void  setSlice              ( TComSlice* p )  { m_pcSlice = p;  }
  Bool getAlfCtrl() {return m_bAlfCtrl;}
  UInt getMaxAlfCtrlDepth() {return m_uiMaxAlfCtrlDepth;}
  Void setAlfCtrl(Bool bAlfCtrl) {m_bAlfCtrl = bAlfCtrl;}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth;}
  Void  resetBits             ()                { m_pcBitIf->resetBits(); }
  Void  resetCoeffCost        ()                { m_uiCoeffCost = 0;  }
  UInt  getNumberOfWrittenBits()                { return  m_pcBitIf->getNumberOfWrittenBits();  }
  UInt  getCoeffCost          ()                { return  m_uiCoeffCost;  }

	Void  codeSPS									( TComSPS* pcSPS );
  Void  codeSliceHeader         ( TComSlice* pcSlice );

  Void  codeTerminatingBit      ( UInt uilsLast );
  Void  codeSliceFinish         ();

  Void  codeExtremeValue( Int iMinVal, Int iMaxVal, Int iExtremeType ) ;
  // EXCBand
  Void  codeCorrBandExType(Int iCorVal, Int iBandNumber);

  Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void codeAlfFlag       ( UInt uiCode );
  Void codeAlfUvlc       ( UInt uiCode );
  Void codeAlfSvlc       ( Int   iCode );
  Void codeAlfCtrlDepth();
protected:
  Void  xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart );

  Void  xWriteCode      ( UInt uiCode, UInt uiLength );
  Void  xWriteUvlc      ( UInt uiCode );
  Void  xWriteSvlc      ( Int iCode );
  Void  xWriteFlag      ( UInt uiCode );

  UInt  xConvertToUInt        ( Int iValue )    {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }

  Void  xWriteTrailingOnes4   ( UInt uiCoeffCount, UInt uiTrailingOnes );
  Void  xWriteTrailingOnes16  ( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes );
  Void  xWriteRunLevel        ( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun );
  Void  xWriteLevelVLC0       ( Int iLevel );
  Void  xWriteLevelVLCN       ( Int iLevel, UInt uiVlcLength );
  Void  xWriteTotalRun4       ( UInt uiVlcPos, UInt uiTotalRun );
  Void  xWriteTotalRun16      ( UInt uiVlcPos, UInt uiTotalRun );
  Void  xWriteRun             ( UInt uiVlcPos, UInt uiRun  );

  Void	xWriteExGolombLevel   (UInt uiLevel);
  Void	xWriteEpExGolomb      (UInt uiLevel, UInt uiCount );

protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;
  UInt          m_uiCoeffCost;
  Bool          m_bRunLengthCoding;
  UInt          m_uiRun;
  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;

public:
  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeAlfCtrlFlag	   ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codeIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx );

  Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );

  Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD = false );

  Void codeMPIindex( TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );
  Void codeCIPflag ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD );

  Void estBit             (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType);
  Void codeULTUsedFlag( TComDataCU* pcCU, UInt uiAbsPartIdx );
};

#endif // !defined(AFX_TENCCAVLC_H__EE8A0B30_945B_4169_B290_24D3AD52296F__INCLUDED_)
