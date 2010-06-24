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

/** \file     TDecCAVLC.cpp
    \brief    CAVLC decoder class
*/

#include "TDecCAVLC.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCavlc::TDecCavlc()
{
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TDecCavlc::~TDecCavlc()
{

}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecCavlc::parsePPS(TComPPS* pcPPS)
{
#if HHI_NAL_UNIT_SYNTAX
  UInt  uiCode;

  xReadCode ( 2, uiCode ); //NalRefIdc
  xReadCode ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCode ( 5, uiCode ); assert( NAL_UNIT_PPS == uiCode);//NalUnitType
#endif
  return;
}

Void TDecCavlc::parseSPS(TComSPS* pcSPS)
{
  UInt  uiCode;
#if HHI_NAL_UNIT_SYNTAX
  xReadCode ( 2, uiCode ); //NalRefIdc
  xReadCode ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCode ( 5, uiCode ); assert( NAL_UNIT_SPS == uiCode);//NalUnitType
#endif
  // Structure
  xReadUvlc ( uiCode ); pcSPS->setWidth       ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setHeight      ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadX        ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadY        ( uiCode    );

  xReadUvlc ( uiCode ); pcSPS->setMaxCUWidth  ( uiCode    ); g_uiMaxCUWidth  = uiCode;
  xReadUvlc ( uiCode ); pcSPS->setMaxCUHeight ( uiCode    ); g_uiMaxCUHeight = uiCode;
  xReadUvlc ( uiCode ); pcSPS->setMaxCUDepth  ( uiCode+1  ); g_uiMaxCUDepth  = uiCode + 1;

  // Transform
  xReadUvlc ( uiCode ); pcSPS->setMinTrDepth  ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setMaxTrDepth  ( uiCode    );

#if HHI_RQT
  xReadFlag( uiCode ); pcSPS->setQuadtreeTUFlag( uiCode != 0 );
  if( pcSPS->getQuadtreeTUFlag() )
  {
    xReadUvlc( uiCode ); pcSPS->setQuadtreeTULog2MinSize( uiCode + 2 );
    if( pcSPS->getQuadtreeTULog2MinSize() < 6 )
    {
      xReadUvlc( uiCode ); pcSPS->setQuadtreeTULog2MaxSize( uiCode + pcSPS->getQuadtreeTULog2MinSize() );
    }
  }
#endif  // Max transform size
  xReadUvlc ( uiCode ); pcSPS->setMaxTrSize   ( (uiCode == 0) ? 2 : (1<<(uiCode+1)) );

  // Tool on/off
  xReadFlag( uiCode ); pcSPS->setUseALF ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseDQP ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseWPG ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseLDC ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseQBO ( uiCode ? true : false );

#if HHI_ALLOW_CIP_SWITCH
  xReadFlag( uiCode ); pcSPS->setUseCIP ( uiCode ? true : false ); // BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
  xReadFlag( uiCode ); pcSPS->setUseROT ( uiCode ? true : false ); // BB:
#endif
#if HHI_AIS
  xReadFlag( uiCode ); pcSPS->setUseAIS ( uiCode ? true : false ); // BB:
#endif
#if HHI_MRG
  xReadFlag( uiCode ); pcSPS->setUseMRG ( uiCode ? true : false ); // SOPH:
#endif
#if HHI_IMVP
  xReadFlag( uiCode ); pcSPS->setUseIMP ( uiCode ? true : false ); // SOPH:
#endif
  // number of taps for DIF
  xReadUvlc( uiCode ); pcSPS->setDIFTap ( (uiCode+2)<<1 );  // 4, 6, 8, 10, 12

  // AMVP mode for each depth (AM_NONE or AM_EXPL)
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xReadFlag( uiCode );
    pcSPS->setAMVPMode( i, (AMVP_MODE)uiCode );
  }

  // Bit-depth information
  xReadUvlc( uiCode ); pcSPS->setBitDepth     ( uiCode+8 ); g_uiBitDepth     = uiCode + 8;
  xReadUvlc( uiCode ); pcSPS->setBitIncrement ( uiCode   ); g_uiBitIncrement = uiCode;

  xReadCode( 8, uiCode ); pcSPS->setBalancedCPUs( uiCode );

  g_uiBASE_MAX  = ((1<<(g_uiBitDepth))-1);

#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX  = g_uiBASE_MAX << g_uiBitIncrement;
#else
  g_uiIBDI_MAX  = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);
#endif

  g_uiAddCUDepth = 0;
  if( ((g_uiMaxCUWidth>>(g_uiMaxCUDepth-1)) > pcSPS->getMaxTrSize()) )
  {
    while( (g_uiMaxCUWidth>>(g_uiMaxCUDepth-1)) > (pcSPS->getMaxTrSize()<<g_uiAddCUDepth) ) g_uiAddCUDepth++;
  }
  g_uiMaxCUDepth += g_uiAddCUDepth;
  g_uiAddCUDepth++;

  return;
}

Void TDecCavlc::parseSliceHeader (TComSlice*& rpcSlice)
{
  UInt  uiCode;
  Int   iCode;
#if HHI_NAL_UNIT_SYNTAX
  xReadCode ( 2, uiCode ); //NalRefIdc
  xReadCode ( 1, uiCode ); assert( 0 == uiCode); // zero bit
  xReadCode ( 5, uiCode ); assert( NAL_UNIT_CODED_SLICE == uiCode);//NalUnitType
#endif
  xReadCode (10, uiCode);  rpcSlice->setPOC              (uiCode);             // 9 == SPS->Log2MaxFrameNum()
  xReadUvlc (   uiCode);  rpcSlice->setSliceType        ((SliceType)uiCode);
  xReadSvlc (    iCode);  rpcSlice->setSliceQp          (iCode);

  xReadFlag ( uiCode );
  if( uiCode )
  {
    xReadFlag ( uiCode );
    uiCode++;
    rpcSlice->setSymbolMode( uiCode );
    xReadFlag( uiCode );
    rpcSlice->setMultiCodeword( uiCode == 1 );
    if( rpcSlice->getSymbolMode() == 2 && ! rpcSlice->getMultiCodeword() )
    {
      xReadUvlc( uiCode );
      rpcSlice->setMaxPIPEDelay( uiCode << 6 );
    }
  }
  else
  {
    xReadFlag ( uiCode );
    rpcSlice->setSymbolMode( uiCode ? 3 : 0 );
  }

  if (!rpcSlice->isIntra())
    xReadFlag (   uiCode);
  else
    uiCode = 1;

  rpcSlice->setReferenced       (uiCode ? true : false);

  xReadFlag (   uiCode);  rpcSlice->setLoopFilterDisable(uiCode ? 1 : 0);

  if (!rpcSlice->isIntra())
  {
    xReadCode (3, uiCode);  rpcSlice->setNumRefIdx      (REF_PIC_LIST_0, uiCode);
  }
  else
  {
    rpcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
  }
  if (rpcSlice->isInterB())
  {
    xReadCode (3, uiCode);  rpcSlice->setNumRefIdx      (REF_PIC_LIST_1, uiCode);
  }
  else
  {
    rpcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
  }

  xReadFlag (uiCode);     rpcSlice->setDRBFlag          (uiCode ? 1 : 0);
  if ( !rpcSlice->getDRBFlag() )
  {
    xReadCode(2, uiCode); rpcSlice->setERBIndex( (ERBIndex)uiCode );    assert (uiCode == ERB_NONE || uiCode == ERB_LTR);
  }

  if (!rpcSlice->isIntra())
  {
    Int  iNumPredDir = rpcSlice->isInterP() ? 1 : 2;

    if (rpcSlice->getSPS()->getUseWPG())
    {
      for (Int n=0; n<iNumPredDir; n++)
      {
        RefPicList eRefPicList = (RefPicList)n;

        xReadCode (1, uiCode);
        rpcSlice->setWPmode(eRefPicList, uiCode);

        if (rpcSlice->getWPmode(eRefPicList))
        {
          rpcSlice->addEffectMode(eRefPicList, EFF_WP_SO);
#if !GRF_WP_CHROMA
          rpcSlice->initWPParam(eRefPicList, EFF_WP_SO, 1);
          rpcSlice->initWPParam(eRefPicList, EFF_WP_SO, 2);
#endif
          UInt uiTemp;
          Int iWeight, iOffset;

          xReadUvlc(uiTemp);
          iWeight = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          iWeight=iWeight+32;
          rpcSlice->setWPWeight(eRefPicList, EFF_WP_SO,0,iWeight);

          xReadUvlc(uiTemp);
          iOffset = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          rpcSlice->setWPOffset(eRefPicList, EFF_WP_SO,0,iOffset);

#if GRF_WP_CHROMA
          xReadUvlc(uiTemp);
          iWeight = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          iWeight=iWeight+32;
          rpcSlice->setWPWeight(eRefPicList, EFF_WP_SO,1,iWeight);

          xReadUvlc(uiTemp);
          iOffset = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          rpcSlice->setWPOffset(eRefPicList, EFF_WP_SO,1,iOffset);

          xReadUvlc(uiTemp);
          iWeight = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          iWeight=iWeight+32;
          rpcSlice->setWPWeight(eRefPicList, EFF_WP_SO,2,iWeight);

          xReadUvlc(uiTemp);
          iOffset = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
          rpcSlice->setWPOffset(eRefPicList, EFF_WP_SO,2,iOffset);
#endif
        }
      }
    }
  }

#if HHI_INTERP_FILTER
  xReadUvlc( uiCode ); rpcSlice->setInterpFilterType( uiCode );
#endif

#if AMVP_NEIGH_COL
  if ( rpcSlice->getSliceType() == B_SLICE )
  {
    xReadFlag (uiCode);
    rpcSlice->setColDir(uiCode);
  }
#endif

  return;
}

Void TDecCavlc::resetEntropy          (TComSlice* pcSlice)
{
  m_bRunLengthCoding = ! pcSlice->isIntra();
  m_uiRun = 0;

  ::memcpy(m_uiLPTableD8,        g_auiLPTableD8,        10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableD4,        g_auiLPTableD4,        3*32*sizeof(UInt));
  ::memcpy(m_uiLastPosVlcIndex, g_auiLastPosVlcIndex, 10*sizeof(UInt));
}

Void TDecCavlc::parseTerminatingBit( UInt& ruiBit )
{
  xReadFlag( ruiBit );
}

Void TDecCavlc::parseCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadFlag( uiSymbol );
  pcCU->setCIPflagSubParts( (UChar)uiSymbol, uiAbsPartIdx, uiDepth );
}

Void TDecCavlc::parseROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt  uiSymbol;
  UInt  indexROT = 0;
  Int    dictSize = ROT_DICT;

  switch (dictSize)
  {
  case 9:
    {
      xReadFlag( uiSymbol );
      if ( !uiSymbol )
      {
        xReadFlag( uiSymbol );
        indexROT  = uiSymbol;
        xReadFlag( uiSymbol );
        indexROT |= uiSymbol << 1;
        xReadFlag( uiSymbol );
        indexROT |= uiSymbol << 2;
        indexROT++;
      }
    }
    break;
  case 4:
    {
      xReadFlag( uiSymbol );
      indexROT  = uiSymbol;
      xReadFlag( uiSymbol );
      indexROT |= uiSymbol << 1;
    }
    break;
  case 2:
    {
      xReadFlag( uiSymbol );
      if ( !uiSymbol ) indexROT =1;
    }
    break;
  case 5:
    {
      xReadFlag( uiSymbol );
      if ( !uiSymbol )
      {
        xReadFlag( uiSymbol );
        indexROT  = uiSymbol;
        xReadFlag( uiSymbol );
        indexROT |= uiSymbol << 1;
        indexROT++;
      }
    }
    break;
  case 1:
    {
    }
    break;
  }
  pcCU->setROTindexSubParts( indexROT, uiAbsPartIdx, uiDepth );

  return;
}

Void TDecCavlc::parseAlfCtrlDepth              ( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, g_uiMaxCUDepth-1);
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecCavlc::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (!m_bAlfCtrl)
    return;

  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  UInt uiSymbol;
  xReadFlag( uiSymbol );

  if (uiDepth > m_uiMaxAlfCtrlDepth)
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth);
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

Void TDecCavlc::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG
  if ( pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    return;
  }
#endif

  if( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  UInt uiSymbol;
  xReadFlag( uiSymbol );

  if( uiSymbol )
  {
    pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );

    TComMv cZeroMv(0,0);
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );

    pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
    pcCU->setCbfSubParts  ( 0, 0, 0, uiAbsPartIdx, uiDepth );

    if ( pcCU->getSlice()->isInterP() )
    {
      pcCU->setInterDirSubParts( 1, uiAbsPartIdx, 0, uiDepth );

      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
    else
    {
      pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );

      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
  }
}

Void TDecCavlc::parseMVPIdx      ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, iMVPNum-1);
  riMVPIdx = uiSymbol;
}

Void TDecCavlc::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return ;
  }

  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );

  return ;
}

Void TDecCavlc::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif

  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    return ;
  }

  UInt uiSymbol, uiMode = 0;
  PartSize eMode;

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    xReadFlag( uiSymbol );
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;
  }
  else
  {
    UInt uiMaxNumBits = 3;
    for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
    {
      xReadFlag( uiSymbol );
      if ( uiSymbol )
      {
        break;
      }
      uiMode++;
    }

    eMode = (PartSize) uiMode;

    if (pcCU->getSlice()->isInterB() && uiMode == 3)
    {
      xReadFlag( uiSymbol );
      if (uiSymbol == 0)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        xReadFlag( uiSymbol );
        if (uiSymbol == 0)
          eMode = SIZE_2Nx2N;
      }
    }

    if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
    {
      if (eMode == SIZE_2NxN)
      {
        xReadFlag(uiSymbol);
        if (uiSymbol == 0)
        {
          xReadFlag(uiSymbol);
          eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
        xReadFlag(uiSymbol);
        if (uiSymbol == 0)
        {
          xReadFlag(uiSymbol);
          eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
        }
      }
    }
  }

  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );

  UInt uiTrLevel = 0;

  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
  {
    if( pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1+uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
  }
}

Void TDecCavlc::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    pcCU->setPredModeSubParts( MODE_INTER, uiAbsPartIdx, uiDepth );
    return;
  }
#endif

  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return ;
  }

  UInt uiSymbol;
  Int  iPredMode = MODE_INTER;

#if HHI_MRG
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    xReadFlag( uiSymbol );
    if ( uiSymbol == 0 )
    {
      iPredMode = MODE_SKIP;
    }
  }
#else
    xReadFlag( uiSymbol );
    if ( uiSymbol == 0 )
    {
      iPredMode = MODE_SKIP;
    }
#endif

  if ( pcCU->getSlice()->isInterB() )
  {
    pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
    return;
  }

  if ( iPredMode != MODE_SKIP )
  {
    xReadFlag( uiSymbol );
    iPredMode += uiSymbol;
  }

  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}

#if PLANAR_INTRA
// Temporary VLC function
UInt TDecCavlc::xParsePlanarVlc( )
{
  Bool bDone    = false;
  UInt uiZeroes = 0;
  UInt uiBit;
  UInt uiCodeword;

  while (!bDone)
  {
    xReadFlag( uiBit );

    if ( uiBit )
    {
      xReadFlag( uiCodeword );
      bDone = true;
    }
    else
      uiZeroes++;
  }

  return ( ( uiZeroes << 1 ) + uiCodeword );
}

Int TDecCavlc::xParsePlanarDelta( TextType ttText )
{
  /* Planar quantization
  Y        qY              cW
  0-3   :  0,1,2,3         0-3
  4-15  :  4,6,8..14       4-9
  16-63 : 18,22,26..62    10-21
  64-.. : 68,76...        22-
  */
  UInt bDeltaNegative = 0;
  Int  iDelta         = xParsePlanarVlc();

  if( iDelta > 21 )
    iDelta = ( ( iDelta - 14 ) << 3 ) + 4;
  else if( iDelta > 9 )
    iDelta = ( ( iDelta - 6 ) << 2 ) + 2;
  else if( iDelta > 3 )
    iDelta = ( iDelta - 2 ) << 1;

  if( iDelta > 0 )
  {
    xReadFlag( bDeltaNegative );

    if( bDeltaNegative )
      iDelta = -iDelta;
  }

  return iDelta;
}

Void TDecCavlc::parsePlanarInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadFlag( uiSymbol );

  if ( uiSymbol )
  {
    Int iPlanarFlag   = 1;
    Int iPlanarDeltaY = xParsePlanarDelta( TEXT_LUMA );
    Int iPlanarDeltaU = 0;
    Int iPlanarDeltaV = 0;

    xReadFlag( uiSymbol );

    if ( !uiSymbol )
    {
      iPlanarDeltaU = xParsePlanarDelta( TEXT_CHROMA_U );
      iPlanarDeltaV = xParsePlanarDelta( TEXT_CHROMA_V );
    }

    pcCU->setPartSizeSubParts  ( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts      ( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    pcCU->setPlanarInfoSubParts( iPlanarFlag, iPlanarDeltaY, iPlanarDeltaU, iPlanarDeltaV, uiAbsPartIdx, uiDepth );
  }
}
#endif

#if ANG_INTRA
Void TDecCavlc::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );

  xReadFlag( uiSymbol );

  if ( uiSymbol )
    uiIPredMode = iMostProbable;
  else{
    xReadFlag( uiSymbol ); uiIPredMode  = uiSymbol;
    xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 1;
    xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 2;
    xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 3;
    xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 4;

    if (uiIPredMode == 31){ // Escape coding for the last two modes
      xReadFlag( uiSymbol );
      uiIPredMode = uiSymbol ? 32 : 31;
    }

    if (uiIPredMode >= iMostProbable)
      uiIPredMode++;
  }

  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );
}
#endif

Void TDecCavlc::parseIntraDirLumaAdi  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);

  xReadFlag( uiSymbol );

  if ( !uiSymbol )
  {
    xReadFlag( uiSymbol );
    uiIPredMode  = uiSymbol;
    xReadFlag( uiSymbol );
    uiIPredMode |= uiSymbol << 1;
    if (g_aucIntraModeBits[iIntraIdx]>=4)
    {
      xReadFlag( uiSymbol );
      uiIPredMode |= uiSymbol << 2;
      if (g_aucIntraModeBits[iIntraIdx]>=5)
      {
        xReadFlag( uiSymbol );
        uiIPredMode |= uiSymbol << 3;
        if (g_aucIntraModeBits[iIntraIdx]>=6)
        {
          xReadFlag( uiSymbol );
          uiIPredMode |= uiSymbol << 4;
        }
      }
    }
    uiIPredMode = pcCU->revertIntraDirLumaAdi( pcCU,uiAbsPartIdx, (Int)( uiIPredMode  ) );
  }
  else
  {
    uiIPredMode  = pcCU->revertIntraDirLumaAdi( pcCU,uiAbsPartIdx, -1 );
  }
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );

  return;
}


Void TDecCavlc::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadFlag( uiSymbol );

  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, 3 );
    uiSymbol++;
  }

  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );

  return ;
}

Void TDecCavlc::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadFlag( uiSymbol );

  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
  else
  {
    xReadFlag( uiSymbol );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecCavlc::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;

  xReadFlag ( uiSymbol );
  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );

    uiSymbol++;
  }
  riRefFrmIdx = uiSymbol;
  return;
}

Void TDecCavlc::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  Int iHor, iVer;
  UInt uiAbsPartIdxL, uiAbsPartIdxA;
  Int iHorPred, iVerPred;

  TComDataCU* pcCUL   = pcCU->getPULeft ( uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx );

  TComCUMvField* pcCUMvFieldL = ( pcCUL == NULL || pcCUL->isIntra( uiAbsPartIdxL ) ) ? NULL : pcCUL->getCUMvField( eRefList );
  TComCUMvField* pcCUMvFieldA = ( pcCUA == NULL || pcCUA->isIntra( uiAbsPartIdxA ) ) ? NULL : pcCUA->getCUMvField( eRefList );

  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
    ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
    ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );

  TComMv cTmpMv( 0, 0 );
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  xReadSvlc( iHor );
  xReadSvlc( iVer );

  // set mvd
  TComMv cMv( iHor, iVer );
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  return;
}

Void TDecCavlc::parseTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiTrLevel = 0;

  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

  UInt uiMinTrDepth = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel;
  UInt uiMaxTrDepth = pcCU->getSlice()->getSPS()->getMaxTrDepth() + uiTrLevel;

  if ( uiMinTrDepth == uiMaxTrDepth )
  {
    pcCU->setTrIdxSubParts( uiMinTrDepth, uiAbsPartIdx, uiDepth );
    return;
  }

  UInt uiTrIdx;
  xReadFlag( uiTrIdx );

  if ( !uiTrIdx )
  {
    uiTrIdx = uiTrIdx + uiMinTrDepth;
    pcCU->setTrIdxSubParts( uiTrIdx, uiAbsPartIdx, uiDepth );
    return;
  }

  if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N && uiMinTrDepth == 0 && uiMaxTrDepth == 1)
  {
    uiTrIdx++;

    ///Maybe unnecessary///
    UInt      uiWidth      = pcCU->getWidth ( uiAbsPartIdx );
    while((uiWidth>>uiTrIdx) < (g_uiMaxCUWidth>>g_uiMaxCUDepth)) uiTrIdx--;
    ////////////////////////

    uiTrIdx = uiTrIdx + uiMinTrDepth;
    pcCU->setTrIdxSubParts( uiTrIdx, uiAbsPartIdx, uiDepth );
    return;
  }

  UInt uiSymbol;
  Int  iCount = 1;
  while( ++iCount <= (Int)( uiMaxTrDepth - uiMinTrDepth ) )
  {
    xReadFlag( uiSymbol );
    if ( uiSymbol == 0 )
    {
      uiTrIdx = uiTrIdx + uiMinTrDepth;
      pcCU->setTrIdxSubParts( uiTrIdx, uiAbsPartIdx, uiDepth );
      return;
    }
    uiTrIdx += uiSymbol;
  }

  uiTrIdx = uiTrIdx + uiMinTrDepth;

  pcCU->setTrIdxSubParts( uiTrIdx, uiAbsPartIdx, uiDepth );

  return ;
}

Void TDecCavlc::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiDQp;
  Int  iDQp;

  xReadFlag( uiDQp );

  if ( uiDQp == 0 )
  {
    uiDQp = pcCU->getSlice()->getSliceQp();
  }
  else
  {
    xReadSvlc( iDQp );
    uiDQp = pcCU->getSlice()->getSliceQp() + iDQp;
  }

  pcCU->setQPSubParts( uiDQp, uiAbsPartIdx, uiDepth );
}

Void TDecCavlc::parseCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
#if HHI_RQT
  if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
  {
#if HHI_RQT_INTRA
    return;
#else
    if( !pcCU->isIntra( uiAbsPartIdx ) )
    {
      return;
    }
#endif
  }
#endif
  UInt uiSymbol;
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );

  xReadFlag( uiSymbol );
  pcCU->setCbfSubParts( uiCbf | ( uiSymbol << uiTrDepth ), eType, uiAbsPartIdx, uiDepth );

  return;
}

Void TDecCavlc::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{

  if ( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;

  // point to coefficient
  TCoeff* piCoeff = pcCoef;

  // initialize scan
  const UInt*  pucScan;

  UInt uiConvBit = g_aucConvertToBit[ uiWidth    ];
#if HHI_RQT
  pucScan        = g_auiFrameScanXY  [ uiConvBit + 1 ];
#else
  pucScan        = g_auiFrameScanXY  [ uiConvBit ];
#endif

  UInt uiScanning, uiInterleaving, uiIsCoded;

  TCoeff scoeff[64];
  Int iBlockType;
#if HHI_RQT
  if( uiSize == 2*2 )
  {
    // hack: re-use 4x4 coding
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();
    xParseCoeff4x4( scoeff, iBlockType );

    for (uiScanning=0; uiScanning<4; uiScanning++)
    {
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[15-uiScanning];
    }
  }
  else if ( uiSize == 4*4 )
#else
  if ( uiSize == 4*4 )
#endif
  {
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();
    xParseCoeff4x4( scoeff, iBlockType );

    for (uiScanning=0; uiScanning<16; uiScanning++)
    {
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[15-uiScanning];
    }
  }
  else if ( uiSize == 8*8 )
  {
    iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
    xParseCoeff8x8( scoeff, iBlockType );

    for (uiScanning=0; uiScanning<64; uiScanning++)
    {
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[63-uiScanning];
    }

  }
  else
  {
    for (uiInterleaving=0; uiInterleaving<uiSize/64; uiInterleaving++)
    {
      xReadFlag( uiIsCoded );
      if ( !uiIsCoded )
      {
        for (uiScanning=0; uiScanning<64; uiScanning++)
        {
          piCoeff[ pucScan[ (uiSize/64) * uiScanning + uiInterleaving ] ] = 0;
        }
      }
      else
      {
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
        xParseCoeff8x8( scoeff, iBlockType );

        for (uiScanning=0; uiScanning<64; uiScanning++)
        {
          piCoeff[ pucScan[ (uiSize/64) * uiScanning + uiInterleaving ] ] = scoeff[63-uiScanning];
        }

      }
    }
  }

  return ;
}

#if HHI_RQT
Void TDecCavlc::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  xReadFlag( ruiSubdivFlag );
}

Void TDecCavlc::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setCbfSubParts( uiSymbol << uiTrDepth, eType, uiAbsPartIdx, uiDepth );
}
#endif

Void TDecCavlc::parseAlfFlag (UInt& ruiVal)
{
  xReadFlag( ruiVal );
}

Void TDecCavlc::parseAlfUvlc (UInt& ruiVal)
{
  xReadUvlc( ruiVal );
}

Void TDecCavlc::parseAlfSvlc (Int&  riVal)
{
  xReadSvlc( riVal );
}

#if HHI_ALF
Void TDecCavlc::parseAlfQTCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (!m_bAlfCtrl)
    return;

  UInt uiSymbol;
  xReadFlag( uiSymbol );

  if (uiDepth > m_uiMaxAlfCtrlDepth)
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth);
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

Void TDecCavlc::parseAlfQTSplitFlag  ( TComDataCU* pcCU ,UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth )
{
  //if( uiDepth >= uiMaxDepth )
  if( uiDepth >= g_uiMaxCUDepth-g_uiAddCUDepth ) // fix HS
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return;
  }

  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );

  return;
}

Void TDecCavlc::parseAlfDc( Int& riDc    )
{
  UInt uiCode;
  UInt iSign ;

  xReadFlag( uiCode );
  if ( uiCode == 0)
  {
    riDc = 0 ;
    return ;
  }

  xReadFlag( uiCode );
  if ( uiCode == 0 )  iSign =  1;
  else                iSign = -1;

  xReadEpExGolomb(uiCode, 9);
  riDc = uiCode*iSign ;
}

Void TDecCavlc::parseAlfCoeff( Int& riCoeff, Int iLength, Int iPos )
{
  UInt uiCode;
  UInt iSign ;

  xReadFlag( uiCode ) ;
  if ( uiCode == 0)
  {
    riCoeff = 0 ;
    return ;
  }

  xReadFlag( uiCode );
  if ( uiCode == 0 )  iSign =  1;
  else                iSign = -1;

  Int iM =4;
  if(iLength==3)
  {
    if(iPos == 0)
      iM = 4 ;
    else if(iPos == 1)
      iM = 1 ;
    else if(iPos == 2)
      iM = 2 ;
  }
  else if(iLength==5)
  {
    if(iPos == 0)
      iM = 3 ;
    else if(iPos == 1)
      iM = 5 ;
    else if(iPos == 2)
      iM = 1 ;
    else if(iPos == 3)
      iM = 3 ;
    else if(iPos == 4)
      iM = 2 ;
  }
  else if(iLength==7)
  {
    if(iPos == 0)
      iM = 3 ;
    else if(iPos == 1)
      iM = 4 ;
    else if(iPos == 2)
      iM = 5;
    else if(iPos == 3)
      iM = 1 ;
    else if(iPos == 4)
      iM = 3 ;
    else if(iPos == 5)
     iM = 3 ;
    else if(iPos == 6)
     iM = 2 ;
  }
  else if(iLength==9)
  {
    if(iPos == 0)
      iM = 2 ;
    else if(iPos == 1)
      iM = 4 ;
    else if(iPos == 2)
      iM = 4 ;
    else if(iPos == 3)
      iM = 5 ;
    else if(iPos == 4)
      iM = 1 ;
    else if(iPos == 5)
     iM = 3 ;
    else if(iPos == 6)
     iM = 3 ;
    else if(iPos == 7)
    iM = 3 ;
   else if(iPos == 8)
    iM = 2 ;
  }

  xReadEpExGolomb(uiCode, iM);
  riCoeff = uiCode*iSign ;
}
#endif

#if HHI_MRG
Void TDecCavlc::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setMergeFlag( uiAbsPartIdx, uiSymbol ? true : false );
}

Void TDecCavlc::parseMergeIndex ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiMergeIndex;
  xReadFlag( uiMergeIndex );
  pcCU->setMergeIndex( uiAbsPartIdx, uiMergeIndex );
}
#endif

#if HHI_AIS
Void TDecCavlc::parseIntraFiltFlagLumaAdi ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

  //pcCU->setLumaIntraFiltFlag( uiAbsPartIdx, uiSymbol ? true : false );
  pcCU->setLumaIntraFiltFlagSubParts( uiSymbol != 0, uiAbsPartIdx, uiDepth ); // fix HS
}
#endif

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TDecCavlc::xReadCode (UInt uiLength, UInt& ruiCode)
{
  assert ( uiLength > 0 );
  m_pcBitstream->read (uiLength, ruiCode);
}

Void TDecCavlc::xReadUvlc( UInt& ruiVal)
{
  UInt uiVal = 0;
  UInt uiCode = 0;
  UInt uiLength;
  m_pcBitstream->read( 1, uiCode );

  if( 0 == uiCode )
  {
    uiLength = 0;

    while( ! ( uiCode & 1 ))
    {
      m_pcBitstream->read( 1, uiCode );
      uiLength++;
    }

    m_pcBitstream->read( uiLength, uiVal );

    uiVal += (1 << uiLength)-1;
  }

  ruiVal = uiVal;
}

Void TDecCavlc::xReadSvlc( Int& riVal)
{
  UInt uiBits = 0;
  m_pcBitstream->read( 1, uiBits );
  if( 0 == uiBits )
  {
    UInt uiLength = 0;

    while( ! ( uiBits & 1 ))
    {
      m_pcBitstream->read( 1, uiBits );
      uiLength++;
    }

    m_pcBitstream->read( uiLength, uiBits );

    uiBits += (1 << uiLength);
    riVal = ( uiBits & 1) ? -(Int)(uiBits>>1) : (Int)(uiBits>>1);
  }
  else
  {
    riVal = 0;
  }
}

Void TDecCavlc::xReadFlag (UInt& ruiCode)
{
  m_pcBitstream->read( 1, ruiCode );
}

Void TDecCavlc::xReadUnaryMaxSymbol( UInt& ruiSymbol, UInt uiMaxSymbol )
{
  xReadFlag( ruiSymbol );

  if (ruiSymbol == 0 || uiMaxSymbol == 1)
  {
    return;
  }

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    xReadFlag( uiCont );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );

  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }

  ruiSymbol = uiSymbol;
}

Void TDecCavlc::xReadExGolombLevel( UInt& ruiSymbol )
{
  UInt uiSymbol ;
  UInt uiCount = 0;
  do
  {
    xReadFlag( uiSymbol );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 13));

  ruiSymbol = uiCount-1;

  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 0 );
    ruiSymbol += uiSymbol+1;
  }

  return;
}

Void TDecCavlc::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;


  while( uiBit )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount++;
  }

  uiCount--;
  while( uiCount-- )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount;
  }

  ruiSymbol = uiSymbol;

  return;
}

UInt TDecCavlc::xGetBit()
{
  UInt ruiCode;
  m_pcBitstream->read( 1, ruiCode );
  return ruiCode;
}

Int TDecCavlc::xReadVlc( Int n )
{
  assert( n>=0 && n<=10 );

  UInt zeroes=0, done=0, tmp;
  UInt cw, bit;
  UInt val = 0;
  UInt first;
  UInt lead = 0;

  if (n < 5)
  {
    while (!done && zeroes < 6)
    {
      xReadFlag( bit );
      if (bit)
      {
        if (n)
        {
          xReadCode( n, cw );
        }
        else
        {
          cw = 0;
        }
        done = 1;
      }
      else
      {
        zeroes++;
      }
    }
    if ( done )
    {
      val = (zeroes<<n)+cw;
    }
    else
    {
      lead = n;
      while (!done)
      {
        xReadFlag( first );
        if ( !first )
        {
          lead++;
        }
        else
        {
          if ( lead )
          {
            xReadCode( lead, tmp );
          }
          else
          {
            tmp = 0;
          }
          val = 6 * (1 << n) + (1 << lead) + tmp - (1 << n);
          done = 1;
        }
      }
    }
  }
  else if (n < 8)
  {
    while (!done)
    {
      xReadFlag( bit );
      if ( bit )
      {
        xReadCode( n-4, cw );
        done = 1;
      }
      else
      {
        zeroes++;
      }
    }
    val = (zeroes<<(n-4))+cw;
  }
  else if (n == 8)
  {
    if ( xGetBit() )
    {
      val = 0;
    }
    else if ( xGetBit() )
    {
      val = 1;
    }
    else
    {
      val = 2;
    }
  }
  else if (n == 9)
  {
    if ( xGetBit() )
    {
      if ( xGetBit() )
      {
        xReadCode(3, val);
        val += 3;
      }
      else if ( xGetBit() )
      {
        val = xGetBit() + 1;
      }
      else
      {
        val = 0;
      }
    }
    else
    {
      while (!done)
      {
        xReadFlag( bit );
        if ( bit )
        {
          xReadCode(4, cw);
          done = 1;
        }
        else
        {
          zeroes++;
        }
      }
      val = (zeroes<<4)+cw+11;
    }
  }
  else if (n == 10)
  {
    while (!done)
    {
      xReadFlag( first );
      if ( !first )
      {
        lead++;
      }
      else
      {
        if ( !lead )
        {
          val = 0;
        }
        else
        {
          xReadCode(lead, val);
          val += (1<<lead);
          val--;
        }
        done = 1;
      }
    }
  }
  return val;
}

Void TDecCavlc::xParseCoeff4x4( TCoeff* scoeff, Int n )
{
  Int i;
  UInt sign;
  Int tmp;
  Int vlc,cn,this_pos;
  Int maxrun;
  Int last_position;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int vlc_adaptive=0;
  Int done;
  LastCoeffStruct combo;

  for (i = 0; i < 16; i++)
  {
    scoeff[i] = 0;
  }

  {
    /* Get the last nonzero coeff */
    Int x,y,cx,cy,vlcNum;
    Int vlcTable[8] = {2,2,2};

    /* Decode according to current LP table */
    vlcNum = vlcTable[n];

    tmp = xReadVlc( vlcNum );
    cn = m_uiLPTableD4[n][tmp];
    combo.level = (cn>15);
    combo.last_pos = cn&0x0f;

    /* Adapt LP table */
    cx = tmp;
    cy = Max( 0, cx-1 );
    x = cn;
    y = m_uiLPTableD4[n][cy];
    m_uiLPTableD4[n][cy] = x;
    m_uiLPTableD4[n][cx] = y;
  }

  if ( combo.level == 1 )
  {
    tmp = xReadVlc( 0 );
    sign = tmp&1;
    tmp = (tmp>>1)+2;
  }
  else
  {
    tmp = 1;
    xReadFlag( sign );
  }

  if ( sign )
  {
    tmp = -tmp;
  }

  last_position = combo.last_pos;
  this_pos = 15 - last_position;
  scoeff[this_pos] = tmp;
  i = this_pos;
  i++;

  done = 0;
  {
    while (!done && i < 16)
    {
      maxrun = 15-i;
      if (maxrun > 27)
      {
        maxrun = 28;
        vlc = 3;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }

      /* Go into run mode */
      cn = xReadVlc( vlc );
      combo = g_acstructLumaRun8x8[maxrun][cn];
      i += combo.last_pos;
      /* No sign for last zeroes */
      if (i < 16)
      {
        if (combo.level == 1)
        {
          tmp = xReadVlc( 0 );
          sign = tmp&1;
          tmp = (tmp>>1)+2;
          done = 1;
        }
        else
        {
          tmp = 1;
          xReadFlag( sign );
        }
        if ( sign )
        {
          tmp = -tmp;
        }
        scoeff[i] = tmp;
      }
      i++;
    }
  }
  if (i < 16)
  {
    /* Get the rest in level mode */
    while ( i < 16 )
    {
      tmp = xReadVlc( vlc_adaptive );
      if ( tmp > atable[vlc_adaptive] )
      {
        vlc_adaptive++;
      }
      if ( tmp )
      {
        xReadFlag( sign );
        if ( sign )
        {
          tmp = -tmp;
        }
      }
      scoeff[i] = tmp;
      i++;
    }
  }

  return;
}

Void TDecCavlc::xParseCoeff8x8(TCoeff* scoeff, int n)
{
  Int i;
  UInt sign;
  Int tmp;
  LastCoeffStruct combo;
  Int vlc,cn,this_pos;
  Int maxrun;
  Int last_position;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int vlc_adaptive=0;
  Int done;

  static const Int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
  Int sum_big_coef = 0;

  for (i = 0; i < 64; i++)
  {
    scoeff[i] = 0;
  }

  /* Get the last nonzero coeff */
  {
    Int x,y,cx,cy,vlcNum;

    /* Decode according to current LP table */
    // ADAPT_VLC_NUM
    vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
    tmp = xReadVlc( vlcNum );
    cn = m_uiLPTableD8[n][tmp];
    combo.level = (cn>63);
    combo.last_pos = cn&0x3f;

    /* Adapt LP table */
    cx = tmp;
    cy = Max(0,cx-1);
    x = cn;
    y = m_uiLPTableD8[n][cy];
    m_uiLPTableD8[n][cy] = x;
    m_uiLPTableD8[n][cx] = y;
    // ADAPT_VLC_NUM
    m_uiLastPosVlcIndex[n] += cx == m_uiLastPosVlcIndex[n] ? 0 : (cx < m_uiLastPosVlcIndex[n] ? -1 : 1);
  }

  if (combo.level == 1)
  {
    tmp = xReadVlc( 0 );
    sign = tmp&1;
    tmp = (tmp>>1)+2;
  }
  else
  {
    tmp = 1;
    xReadFlag( sign );
  }
  if ( sign )
  {
    tmp = -tmp;
  }

  last_position = combo.last_pos;
  this_pos = 63 - last_position;
  scoeff[this_pos] = tmp;
  i = this_pos;
  i++;

  done = 0;
  {
    while (!done && i < 64)
    {
      maxrun = 63-i;
      if (maxrun > 27)
      {
        maxrun = 28;
        vlc = 3;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }

      /* Go into run mode */
      cn = xReadVlc( vlc );
      combo = g_acstructLumaRun8x8[maxrun][cn];
      i += combo.last_pos;
      /* No sign for last zeroes */
      if (i < 64)
      {
        if (combo.level == 1)
        {
          tmp = xReadVlc( 0 );
          sign = tmp&1;
          tmp = (tmp>>1)+2;

          sum_big_coef += tmp;
          if (i > switch_thr[n] || sum_big_coef > 2)
          {
            done = 1;
          }
        }
        else
        {
          tmp = 1;
          xReadFlag( sign );
        }
        if ( sign )
        {
          tmp = -tmp;
        }
        scoeff[i] = tmp;
      }
      i++;
    }
  }
  if (i < 64)
  {
    /* Get the rest in level mode */
    while (i<64)
    {
      tmp = xReadVlc( vlc_adaptive );

      if (tmp>atable[vlc_adaptive])
      {
        vlc_adaptive++;
      }
      if (tmp)
      {
        xReadFlag( sign );
        if ( sign )
        {
          tmp = -tmp;
        }
      }
      scoeff[i] = tmp;
      i++;
    }
  }
  return;
}

