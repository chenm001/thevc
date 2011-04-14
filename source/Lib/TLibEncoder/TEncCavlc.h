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

/** \file     TEncCavlc.h
    \brief    CAVLC encoder class (header)
*/

#ifndef __TENCCAVLC__
#define __TENCCAVLC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComBitStream.h"
#include "TEncEntropy.h"
#if QC_MOD_LCEC
#include "../TLibCommon/TComRom.h"
#endif

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CAVLC encoder class
class TEncCavlc : public TEncEntropyIf
{
private:
  Bool m_bAdaptFlag;
  
  
public:
  TEncCavlc();
  virtual ~TEncCavlc();
  
protected:
  TComBitIf*    m_pcBitIf;
  TComSlice*    m_pcSlice;
  UInt          m_uiCoeffCost;
  Bool          m_bRunLengthCoding;
  UInt          m_uiRun;
  Bool          m_bAlfCtrl;
  UInt          m_uiMaxAlfCtrlDepth;
  UInt          m_uiLPTableE4[3][32];
  UInt          m_uiLPTableD4[3][32];
#if !CAVLC_COEF_LRG_BLK
  UInt          m_uiLPTableE8[10][128];
  UInt          m_uiLPTableD8[10][128];
#endif
  UInt          m_uiLastPosVlcIndex[10];
  
#if LCEC_INTRA_MODE
 #if MTK_DCM_MPM
  UInt          m_uiIntraModeTableD17[2][16];
  UInt          m_uiIntraModeTableE17[2][16];

  UInt          m_uiIntraModeTableD34[2][33];
  UInt          m_uiIntraModeTableE34[2][33];
#else
  UInt          m_uiIntraModeTableD17[16];
  UInt          m_uiIntraModeTableE17[16];

  UInt          m_uiIntraModeTableD34[33];
  UInt          m_uiIntraModeTableE34[33];
#endif
#endif
  
#if CAVLC_RQT_CBP
  UInt          m_uiCBP_YUV_TableE[4][8];
  UInt          m_uiCBP_YUV_TableD[4][8];
  UInt          m_uiCBP_YS_TableE[2][4];
  UInt          m_uiCBP_YS_TableD[2][4];
  UInt          m_uiCBP_YCS_TableE[2][8];
  UInt          m_uiCBP_YCS_TableD[2][8];
  UInt          m_uiCBP_4Y_TableE[2][15];
  UInt          m_uiCBP_4Y_TableD[2][15];
  UInt          m_uiCBP_4Y_VlcIdx;
#else
  UInt          m_uiCBPTableE[2][8];
  UInt          m_uiCBPTableD[2][8];
  UInt          m_uiBlkCBPTableE[2][15];
  UInt          m_uiBlkCBPTableD[2][15];
  UInt          m_uiCbpVlcIdx[2];
  UInt          m_uiBlkCbpVlcIdx;
#endif


  
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
  UInt          m_uiMI1TableE[9];
  UInt          m_uiMI1TableD[9];
#else
  UInt          m_uiMI1TableE[8];
  UInt          m_uiMI1TableD[8];
#endif
  UInt          m_uiMI2TableE[15];
  UInt          m_uiMI2TableD[15];
  
  UInt          m_uiMITableVlcIdx;
#if QC_LCEC_INTER_MODE
  UInt          m_uiSplitTableE[4][7];
  UInt          m_uiSplitTableD[4][7];
#endif

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
  UChar         m_ucCBP_YUV_TableCounter[4][4];
  UChar         m_ucCBP_4Y_TableCounter[2][2];
  UChar         m_ucCBP_YS_TableCounter[2][3];
  UChar         m_ucCBP_YCS_TableCounter[2][4];
  UChar         m_ucCBP_YUV_TableCounterSum[4];
  UChar         m_ucCBP_4Y_TableCounterSum[2];
  UChar         m_ucCBP_YS_TableCounterSum[2];
  UChar         m_ucCBP_YCS_TableCounterSum[2];
#else
  UChar         m_ucCBFTableCounter    [2][4];
  UChar         m_ucBlkCBPTableCounter [2][2];
  UChar         m_ucCBFTableCounterSum[2];
  UChar         m_ucBlkCBPTableCounterSum[2];
#endif
  UChar         m_ucMI1TableCounter       [4];
  UChar         m_ucSplitTableCounter  [4][4];

  UChar         m_ucSplitTableCounterSum[4];
  UChar         m_ucMI1TableCounterSum;
#endif

  Void  xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart );
  
  Void  xWriteCode            ( UInt uiCode, UInt uiLength );
  Void  xWriteUvlc            ( UInt uiCode );
  Void  xWriteSvlc            ( Int iCode   );
  Void  xWriteFlag            ( UInt uiCode );
  Void  xWriteEpExGolomb      ( UInt uiSymbol, UInt uiCount );
  Void  xWriteExGolombLevel    ( UInt uiSymbol );
  Void  xWriteUnaryMaxSymbol  ( UInt uiSymbol, UInt uiMaxSymbol );
#if !QC_MOD_LCEC_RDOQ
  UInt  xLeadingZeros         ( UInt uiCode );
#endif
  Void  xWriteVlc             ( UInt uiTableNumber, UInt uiCodeNumber );

#if CAVLC_COEF_LRG_BLK
  Void  xCodeCoeff             ( TCoeff* scoeff, Int n, Int blSize);
#else
  Void  xCodeCoeff4x4          ( TCoeff* scoeff, Int iTableNumber );
  Void  xCodeCoeff8x8          ( TCoeff* scoeff, Int iTableNumber );
#endif
  
  UInt  xConvertToUInt        ( Int iValue ) {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }
  
public:
  
  Void  resetEntropy          ();

#if !CAVLC_COEF_LRG_BLK
  UInt* GetLP8Table();
#endif
  UInt* GetLP4Table();
#if QC_MOD_LCEC
  UInt* GetLastPosVlcIndexTable();
#endif
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
  
  Void  codeNALUnitHeader       ( NalUnitType eNalUnitType, NalRefIdc eNalRefIdc, UInt TemporalId = 0, Bool bOutputFlag = true );
  
  Void  codeSPS                 ( TComSPS* pcSPS );
  Void  codePPS                 ( TComPPS* pcPPS );
  void codeSEI(const SEI&);
  Void  codeSliceHeader         ( TComSlice* pcSlice );
  Void  codeTerminatingBit      ( UInt uilsLast );
  Void  codeSliceFinish         ();
  
  Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeAlfFlag       ( UInt uiCode );
  Void codeAlfUvlc       ( UInt uiCode );
  Void codeAlfSvlc       ( Int   iCode );
  Void codeAlfCtrlDepth();
#if MTK_SAO
  Void codeAoFlag       ( UInt uiCode );
  Void codeAoUvlc       ( UInt uiCode );
  Void codeAoSvlc       ( Int   iCode );
#endif
  Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if TSB_ALF_HEADER
  Void codeAlfFlagNum    ( UInt uiCode, UInt minValue );
  Void codeAlfCtrlFlag   ( UInt uiSymbol );
#endif
#if QC_LCEC_INTER_MODE
  Void codeInterModeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiEncMode );
#endif  
  Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx );
  Void codeQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList );
  
  Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx );
#if CAVLC_RQT_CBP
  Void codeCbfTrdiv      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  UInt xGetFlagPattern   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
#endif
  Void codeCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void codeBlockCbf      ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiQPartNum, Bool bRD = false);
  
  Void codeCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD = false );
  
  Void estBit             (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType);
  
  Bool  getAdaptFlag          ()          { return m_bAdaptFlag; }
  Void  setAdaptFlag          ( Bool b )  { m_bAdaptFlag = b;     }
};

#endif // !defined(AFX_TENCCAVLC_H__EE8A0B30_945B_4169_B290_24D3AD52296F__INCLUDED_)

