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

/** \file     TDecCAVLC.h
    \brief    CAVLC decoder class (header)
*/

#ifndef __TDECCAVLC__
#define __TDECCAVLC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TDecEntropy.h"

//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class SEImessages;

/// CAVLC decoder class
class TDecCavlc : public TDecEntropyIf
{
public:
  TDecCavlc();
  virtual ~TDecCavlc();
  
protected:
  Void  xReadCode             (UInt   uiLength, UInt& ruiCode);
  Void  xReadUvlc             (UInt&  ruiVal);
  Void  xReadSvlc             (Int&   riVal);
  Void  xReadFlag             (UInt&  ruiCode);
  Void  xReadEpExGolomb       ( UInt& ruiSymbol, UInt uiCount );
  Void  xReadExGolombLevel    ( UInt& ruiSymbol );
  Void  xReadUnaryMaxSymbol   ( UInt& ruiSymbol, UInt uiMaxSymbol );
#if ENC_DEC_TRACE
  Void  xReadCodeTr           (UInt  length, UInt& rValue, const Char *pSymbolName);
  Void  xReadUvlcTr           (              UInt& rValue, const Char *pSymbolName);
  Void  xReadSvlcTr           (               Int& rValue, const Char *pSymbolName);
  Void  xReadFlagTr           (              UInt& rValue, const Char *pSymbolName);
#endif
  
  Void  xReadPCMAlignZero     ();

  UInt  xGetBit             ();
  Int   xReadVlc            ( Int n );
  Void  xParseCoeff         ( TCoeff* scoeff, Int blockType, Int blSize
                            , Int isIntra
                            );
  Void  xRunLevelIndInv     (LastCoeffStruct *combo, Int maxrun, UInt lrg1Pos, UInt cn);
  Void  xRunLevelIndInterInv(LastCoeffStruct *combo, Int maxrun, UInt cn, UInt scale);
  
#if G1002_RPS
#if INTER_RPS_PREDICTION
  void  parseShortTermRefPicSet            (TComPPS* pcPPS, TComReferencePictureSet* pcRPS, Int idx);
#else
  Void  parseShortTermRefPicSet            (TComPPS* pcPPS, TComReferencePictureSet* pcRPS);
#endif
#endif
private:
  TComInputBitstream*   m_pcBitstream;
  UInt                  m_uiCoeffCost;
  Bool                  m_bRunLengthCoding;
  UInt                  m_uiRun;
  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;
#if FINE_GRANULARITY_SLICES
  Int           m_iSliceGranularity; //!< slice granularity
#endif
  UInt                      m_uiLPTableD4[3][32];
  UInt                      m_uiLastPosVlcIndex[10];
  
  UInt                      m_uiIntraModeTableD17[17];
  UInt                      m_uiIntraModeTableD34[34];
#if AMP
  UInt                      m_uiSplitTableD[4][11];
#else
  UInt                      m_uiSplitTableD[4][7];
#endif
  UInt                      m_uiCBP_YUV_TableD[4][8];
  UInt                      m_uiCBP_YS_TableD[2][4];
  UInt                      m_uiCBP_YCS_TableD[2][8];
  UInt                      m_uiCBP_4Y_TableD[2][15];
  UInt                      m_uiCBP_4Y_VlcIdx;


  
  Int                   m_iRefFrame0[1000];
  Int                   m_iRefFrame1[1000];
  Bool                  m_bMVres0[1000];
  Bool                  m_bMVres1[1000];
  UInt                  m_uiMI1TableD[9];
  UInt                  m_uiMI2TableD[15]; 
  UInt                  m_uiMITableVlcIdx;

  UChar         m_ucCBP_YUV_TableCounter[4][4];
  UChar         m_ucCBP_4Y_TableCounter[2][2];
  UChar         m_ucCBP_YS_TableCounter[2][3];
  UChar         m_ucCBP_YCS_TableCounter[2][4];
  UChar         m_ucCBP_YUV_TableCounterSum[4];
  UChar         m_ucCBP_4Y_TableCounterSum[2];
  UChar         m_ucCBP_YS_TableCounterSum[2];
  UChar         m_ucCBP_YCS_TableCounterSum[2];

  UChar         m_ucMI1TableCounter[4];
  UChar         m_ucSplitTableCounter[4][4];
  UChar         m_ucSplitTableCounterSum[4];
  UChar         m_ucMI1TableCounterSum;

  
public:

#if F747_APS
  /// rest entropy coder by intial QP and IDC in CABAC
  Void  resetEntropy        (Int  iQp, Int iID) { printf("Not supported yet\n"); assert(0); exit(1);}
#endif
  Void  resetEntropy        ( TComSlice* pcSlice  );
  Void  setBitstream        ( TComInputBitstream* p )   { m_pcBitstream = p; }
  Void  setAlfCtrl          ( Bool bAlfCtrl )            { m_bAlfCtrl = bAlfCtrl; }
  Void  setMaxAlfCtrlDepth  ( UInt uiMaxAlfCtrlDepth )  { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }
#if FINE_GRANULARITY_SLICES
  /// set slice granularity
  Void setSliceGranularity(Int iSliceGranularity)  {m_iSliceGranularity = iSliceGranularity;}

  /// get slice granularity
  Int  getSliceGranularity()                       {return m_iSliceGranularity;             }
#endif
  Void  parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize );
  Void  parseQtCbf          ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void  parseQtRootCbf      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf );
  Void  parseAlfFlag        ( UInt& ruiVal );
  Void  parseAlfUvlc        ( UInt& ruiVal );
  Void  parseAlfSvlc        ( Int&  riVal  );
#if SAO
  Void  parseSaoFlag        ( UInt& ruiVal );
  Void  parseSaoUvlc        ( UInt& ruiVal );
  Void  parseSaoSvlc        ( Int&  riVal  );
#endif
  
  Void  parseSPS            ( TComSPS* pcSPS );
  Void  parsePPS            ( TComPPS* pcPPS);
  void parseSEI(SEImessages&);
  Void  parseSliceHeader    ( TComSlice*& rpcSlice );
#if G220_PURE_VLC_SAO_ALF
#if (TILES_DECODER || OL_USE_WPP)
  Void parseWPPTileInfoToSliceHeader(TComSlice*& rpcSlice);
#endif
#endif
  Void  parseTerminatingBit ( UInt& ruiBit );
  
  Void  parseMVPIdx         ( Int& riMVPIdx );
  
  Void  parseSkipFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseMergeFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx );
  Void parseMergeIndex      ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseSplitFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePartSize        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePredMode        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseIntraDirLumaAng ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseIntraDirChroma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseInterDir        ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseRefFrmIdx       ( TComDataCU* pcCU, Int& riRefFrmIdx,  UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  Void parseMvd             ( TComDataCU* pcCU, UInt uiAbsPartAddr,UInt uiPartIdx,    UInt uiDepth, RefPicList eRefList );
  
  Void parseDeltaQP         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCbfTrdiv        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiTrDepth, UInt uiDepth, UInt& uiSubdiv );
  UInt xGetFlagPattern      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth,  UInt& uiSubdiv );
  Void parseCbf             ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void parseBlockCbf        ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth, UInt uiQPartNum );
  Void parseCoeffNxN        ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );
  
  Void parseIPCMInfo        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);

  Void parseAlfCtrlDepth    ( UInt& ruiAlfCtrlDepth );
  Void parseAlfCtrlFlag     ( UInt &ruiAlfCtrlFlag );
#if TILES
#if TILES_DECODER
  Void readTileMarker     ( UInt& uiTileIdx, UInt uiBitsUsed );
#endif
  Void updateContextTables  ( SliceType eSliceType, Int iQp ) { return; }
#endif    
#if OL_FLUSH
  Void decodeFlush() {};
#endif

#if F747_APS
  /// parse APF flags before SAO and ALF parameters
  Void parseAPSInitInfo(TComAPS& cAPS);
#endif

#if WEIGHT_PRED
  Void xParsePredWeightTable ( TComSlice* pcSlice );
#endif
#if SCALING_LIST
  Void  parseScalingList               ( TComScalingList* scalingList );
  Void  xDecodeDPCMScalingListMatrix   ( TComScalingList *scalingList, Int* data, UInt sizeId, UInt listId);
  Void  xReadScalingListCode           ( TComScalingList *scalingList, Int* buf,  UInt sizeId, UInt listId);
#endif
};

//! \}

#endif // !defined(AFX_TDECCAVLC_H__9732DD64_59B0_4A41_B29E_1A5B18821EAD__INCLUDED_)

