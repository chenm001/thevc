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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "TEncCavlc.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
  m_bRunLengthCoding  = false;   //  m_bRunLengthCoding  = !rcSliceHeader.isIntra();
  m_uiCoeffCost       = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;

  m_bAdaptFlag        = true;    // adaptive VLC table
}

TEncCavlc::~TEncCavlc()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy()
{
  m_bRunLengthCoding = ! m_pcSlice->isIntra();
  m_uiRun = 0;
  ::memcpy(m_uiLPTableE8, g_auiLPTableE8, 10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableD8, g_auiLPTableD8, 10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableE4, g_auiLPTableE4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLPTableD4, g_auiLPTableD4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLastPosVlcIndex, g_auiLastPosVlcIndex, 10*sizeof(UInt));
}

Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_PPS, 5);
#endif
  return;
}

Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_SPS, 5);
#endif
  // Structure
  xWriteUvlc  ( pcSPS->getWidth () );
  xWriteUvlc  ( pcSPS->getHeight() );

  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );

  xWriteUvlc  ( pcSPS->getMaxCUWidth ()   );
  xWriteUvlc  ( pcSPS->getMaxCUHeight()   );
  xWriteUvlc  ( pcSPS->getMaxCUDepth ()-1 ); //xWriteUvlc ( pcSPS->getMaxCUDepth ()-g_uiAddCUDepth );

  // Transform
  xWriteUvlc  ( pcSPS->getMinTrDepth()   );
  xWriteUvlc  ( pcSPS->getMaxTrDepth()   );

#if HHI_RQT
  xWriteFlag( pcSPS->getQuadtreeTUFlag() );
  if( pcSPS->getQuadtreeTUFlag() )
  {
    xWriteUvlc( pcSPS->getQuadtreeTULog2MinSize() - 2 );
    if( pcSPS->getQuadtreeTULog2MinSize() < 6 )
    {
      xWriteUvlc( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize() );
    }
  }
#endif

  // Max transform size
  xWriteUvlc  ( pcSPS->getMaxTrSize() == 2 ? 0 : g_aucConvertToBit[pcSPS->getMaxTrSize()]+1 );

  // Tools
  xWriteFlag  ( (pcSPS->getUseALF ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseDQP ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseWPG () || pcSPS->getUseWPO ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseQBO ()) ? 1 : 0 );
#ifdef QC_AMVRES
  xWriteFlag  ( (pcSPS->getUseAMVRes ()) ? 1 : 0 );
#endif
#if HHI_ALLOW_CIP_SWITCH
  xWriteFlag  ( (pcSPS->getUseCIP ()) ? 1 : 0 ); // BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
	xWriteFlag	( (pcSPS->getUseROT ()) ? 1 : 0 ); // BB:
#endif
#if HHI_AIS
  xWriteFlag  ( (pcSPS->getUseAIS ()) ? 1 : 0 ); // BB:
#endif
#if HHI_MRG
  xWriteFlag  ( (pcSPS->getUseMRG ()) ? 1 : 0 ); // SOPH:
#endif
#if HHI_IMVP
   xWriteFlag ( (pcSPS->getUseIMP ()) ? 1 : 0 ); // SOPH:
#endif

  // write number of taps for DIF
  xWriteUvlc  ( (pcSPS->getDIFTap ()>>1)-2 ); // 4, 6, 8, 10, 12

  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }

  // Bit-depth information
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
  xWriteUvlc( pcSPS->getBitIncrement() );

  xWriteCode( pcSPS->getBalancedCPUs(), 8);
}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
#if HHI_NAL_UNIT_SYNTAX
  // here someone can add an appropriated NalRefIdc type 
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_CODED_SLICE, 5);
#endif
  xWriteCode  (pcSlice->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  xWriteUvlc  (pcSlice->getSliceType() );
  xWriteSvlc  (pcSlice->getSliceQp() );
  
  xWriteFlag  (pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 ? 1 : 0);
  if( pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 )
  {
    xWriteFlag( pcSlice->getSymbolMode() > 1 ? 1 : 0 );
    if( pcSlice->getSymbolMode() )
    {
      xWriteFlag( pcSlice->getMultiCodeword() ? 1 : 0 );
    }
    if( pcSlice->getSymbolMode() == 2 && ! pcSlice->getMultiCodeword() )
    {
      xWriteUvlc( pcSlice->getMaxPIPEDelay() >> 6 );
    }
  }
  else
  {
    xWriteFlag( pcSlice->getSymbolMode() > 0 ? 1 : 0 );
  }

  if (!pcSlice->isIntra())
  {
    xWriteFlag  (pcSlice->isReferenced() ? 1 : 0);
#ifdef ROUNDING_CONTROL
	xWriteFlag  (pcSlice->isRounding() ? 1 : 0);
#endif
  }

  xWriteFlag  (pcSlice->getLoopFilterDisable());

  if (!pcSlice->isIntra())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_0 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_0), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
  }
  if (pcSlice->isInterB())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_1 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_1), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
  }

  xWriteFlag  (pcSlice->getDRBFlag() ? 1 : 0 );
  if ( !pcSlice->getDRBFlag() )
  {
    xWriteCode  (pcSlice->getERBIndex(), 2);
  }

  if (!pcSlice->isIntra())   // weighted prediction information
  {
    Int  iNumPredDir = pcSlice->isInterP() ? 1 : 2;

    if (pcSlice->getSPS()->getUseWPG() || pcSlice->getSPS()->getUseWPO())
    {
      for (Int n=0; n<iNumPredDir; n++)
      {
        RefPicList eRefPicList = (RefPicList)n;

        UInt uiWpMode =  pcSlice->getWPmode(eRefPicList);
        xWriteCode  (uiWpMode, 1 );

        if (uiWpMode)
        {
          EFF_MODE eEffMode = (pcSlice->getSPS()->getUseWPG()? EFF_WP_SO : EFF_WP_O);
          UInt uiWeight,uiOffset;

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 0)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 0));
          xWriteUvlc( uiOffset );

#if GRF_WP_CHROMA
          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 1)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 1));
          xWriteUvlc( uiOffset );

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 2)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 2));
          xWriteUvlc( uiOffset );
#endif
        }
      }
    }
  }

#if HHI_INTERP_FILTER
  xWriteUvlc  ( pcSlice->getInterpFilterType() );
#endif

#if AMVP_NEIGH_COL
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    xWriteFlag( pcSlice->getColDir() );
  }
#endif
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
  xWriteFlag( uilsLast );
}

Void TEncCavlc::codeSliceFinish ()
{
  if ( m_bRunLengthCoding && m_uiRun)
  {
    xWriteUvlc(m_uiRun);
  }
}

// CIP
Void TEncCavlc::codeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int CIPflag = pcCU->getCIPflag   ( uiAbsPartIdx );

  xWriteFlag( CIPflag );
}

Void TEncCavlc::codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int indexROT = pcCU->getROTindex( uiAbsPartIdx );
  Int dictSize = ROT_DICT;

  switch (dictSize)
  {
   case 9:
    {
      xWriteFlag( indexROT> 0 ? 0 : 1 );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteFlag( ( indexROT & 0x01 )        );
        xWriteFlag( ( indexROT & 0x02 )        );
        xWriteFlag( ( indexROT & 0x04 ) >> 2  );
      }
    }
    break;
  case 4:
    {
      xWriteFlag( ( indexROT & 0x01 )        );
      xWriteFlag( ( indexROT & 0x02 ) >> 1  );
    }
    break;
  case 2:
    {
      xWriteFlag( ( indexROT> 0 ? 0 : 1 ) );
    }
    break;
  case 5:
    {
      xWriteFlag( ( indexROT> 0 ? 0 : 1 ) );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteFlag( ( indexROT & 0x01 )        );
        xWriteFlag( ( indexROT & 0x02 ) >> 1  );
      }
    }
    break;
  case 1:
    {
    }
    break;
  }
  return;
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum    = pcCU->getMVPNum(eRefList, uiAbsPartIdx);

  xWriteUnaryMaxSymbol(iSymbol, iNum-1);
}

Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );

  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteFlag( 0 );
    xWriteFlag( 0 );
    xWriteFlag( 0 );
    xWriteFlag( 0 );
    xWriteFlag( (eSize == SIZE_2Nx2N? 0 : 1) );
    return;
  }

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteFlag( eSize == SIZE_2Nx2N? 1 : 0 );
    return;
  }

  switch(eSize)
  {
  case SIZE_2Nx2N:
    {
      xWriteFlag( 1 );
      break;
    }
  case SIZE_2NxN:
  case SIZE_2NxnU:
  case SIZE_2NxnD:
    {
      xWriteFlag( 0 );
      xWriteFlag( 1 );

      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_2NxN)
        {
          xWriteFlag( 1 );
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( (eSize == SIZE_2NxnU? 0: 1) );
        }
      }
      break;
    }
  case SIZE_Nx2N:
  case SIZE_nLx2N:
  case SIZE_nRx2N:
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
      xWriteFlag( 1 );

      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_Nx2N)
        {
          xWriteFlag( 1 );
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( (eSize == SIZE_nLx2N? 0: 1) );
        }
      }
      break;
    }
  case SIZE_NxN:
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
      xWriteFlag( 0 );

      if (pcCU->getSlice()->isInterB())
      {
        xWriteFlag( 1 );
      }
      break;
    }
  default:
    {
      assert(0);
    }
  }
}

Void TEncCavlc::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );

#if HHI_MRG
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    xWriteFlag( iPredMode == MODE_SKIP ? 0 : 1 );
  }
#else
  xWriteFlag( iPredMode == MODE_SKIP ? 0 : 1 );
#endif

  if ( pcCU->getSlice()->isInterB() )
  {
    return;
  }

  if ( iPredMode != MODE_SKIP )
  {
    xWriteFlag( iPredMode == MODE_INTER ? 0 : 1 );
  }
}

#if HHI_MRG
Void TEncCavlc::codeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeIndex( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
}
#endif

#if HHI_ALF
Void TEncCavlc::codeAlfQTCtrlFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
      return;

    if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
    {
      return;
    }
    UInt uiSymbol = pcCU->getAlfCtrlFlag(uiAbsPartIdx);
    xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfQTSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth )
{
  if( uiDepth >= g_uiMaxCUDepth - g_uiAddCUDepth )
     return;

   UInt uiFlag = ( pcCU->getDepth( uiAbsPartIdx ) == uiDepth ) ? 0 : 1;
   xWriteFlag( uiFlag );
}



Void TEncCavlc::codeAlfCoeff( Int iCoeff, Int iLength, Int iPos)
{
  if ( iCoeff == 0)
    xWriteFlag( 0 );
  else
  {
      xWriteFlag( 1 );

    if ( iCoeff > 0 )
      xWriteFlag( 0 );
    else
    {
      xWriteFlag( 1 );
      iCoeff = -iCoeff ;
    }
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

    xWriteEpExGolomb( iCoeff , iM );
  }

}




Void TEncCavlc::codeAlfDc( Int iDc    )
{
  if ( iDc == 0 )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
      // write sign
    if ( iDc > 0 )
    {
      xWriteFlag( 0 ) ;
    }
    else
    {
      xWriteFlag( 1 ) ;
      iDc = -iDc;
    }
    xWriteEpExGolomb( iDc , 9 );
  }
}
#endif

#if HHI_AIS
Void TEncCavlc::codeIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Bool bFlag = pcCU->getLumaIntraFiltFlag( uiAbsPartIdx );
  xWriteFlag( (UInt)bFlag );
}
#endif

Void TEncCavlc::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
    return;

  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;

  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfCtrlDepth()
{
  if (!m_bAlfCtrl)
    return;

  UInt uiDepth = m_uiMaxAlfCtrlDepth;

  xWriteUnaryMaxSymbol(uiDepth, g_uiMaxCUDepth-1);
}

Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;

  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;

  xWriteFlag( uiCurrSplitFlag );
  return;
}

#if HHI_RQT
Void TEncCavlc::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType, uiTrDepth );
  xWriteFlag( uiCbf );
}
#endif

Void TEncCavlc::codeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiTrLevel = 0;

  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

  UInt uiMinTrDepth = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel;
  UInt uiMaxTrDepth = pcCU->getSlice()->getSPS()->getMaxTrDepth() + uiTrLevel;

  if ( uiMinTrDepth == uiMaxTrDepth )
  {
    return;
  }

  UInt uiSymbol = pcCU->getTransformIdx(uiAbsPartIdx) - uiMinTrDepth;

  xWriteFlag( uiSymbol ? 1 : 0 );

  if ( !uiSymbol )
  {
    return;
  }

  if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N && uiMinTrDepth == 0 && uiMaxTrDepth == 1 && uiSymbol)
  {
    return;
  }

  Int  iCount = 1;
  uiSymbol--;
  while( ++iCount <= (Int)( uiMaxTrDepth - uiMinTrDepth ) )
  {
    xWriteFlag( uiSymbol ? 1 : 0 );
    if ( uiSymbol == 0 )
    {
      return;
    }
    uiSymbol--;
  }

  return;
}

#if PLANAR_INTRA
// Temporary VLC function
Void TEncCavlc::xPutPlanarVlc( Int n, Int cn )
{
  UInt tmp  = 1<<(n-4);
  UInt code = tmp+cn%tmp;
  UInt len  = 1+(n-4)+(cn>>(n-4));

  xWriteCode( code, len );
}

Void TEncCavlc::xCodePlanarDelta( TComDataCU* pcCU, UInt uiAbsPartIdx , Int iDelta )
{
  /* Planar quantization
  Y        qY              cW
  0-3   :  0,1,2,3         0-3
  4-15  :  4,6,8..14       4-9
  16-63 : 18,22,26..62    10-21
  64-.. : 68,76...        22-
  */
  Bool bDeltaNegative = iDelta < 0 ? true : false;
  UInt uiDeltaAbs     = abs(iDelta);

  if( uiDeltaAbs < 4 )
    xPutPlanarVlc( 5, uiDeltaAbs );
  else if( uiDeltaAbs < 16 )
    xPutPlanarVlc( 5, (uiDeltaAbs>>1)+2 );
  else if( uiDeltaAbs < 64)
    xPutPlanarVlc( 5, (uiDeltaAbs>>2)+6 );
  else
    xPutPlanarVlc( 5, (uiDeltaAbs>>3)+14 );

  if(uiDeltaAbs > 0)
    xWriteFlag( bDeltaNegative ? 1 : 0 );

}

Void TEncCavlc::codePlanarInfo( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (pcCU->isIntra( uiAbsPartIdx ))
  {
    UInt uiPlanar = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_FLAG);

    xWriteFlag( uiPlanar );

    if ( uiPlanar )
    {
      // Planar delta for Y
      xCodePlanarDelta( pcCU, uiAbsPartIdx, pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAY) );

      // Planar delta for U and V
      Int  iPlanarDeltaU = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAU);
      Int  iPlanarDeltaV = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAV);

      xWriteFlag( ( iPlanarDeltaU == 0 && iPlanarDeltaV == 0 ) ? 1 : 0 );

      if ( iPlanarDeltaU != 0 || iPlanarDeltaV != 0 )
      {
        xCodePlanarDelta( pcCU, uiAbsPartIdx, iPlanarDeltaU );
        xCodePlanarDelta( pcCU, uiAbsPartIdx, iPlanarDeltaV );
      }
    }
  }
}
#endif

#if ANG_INTRA
Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );

  if (uiDir == iMostProbable)
    xWriteFlag( 1 );
  else{
    xWriteFlag( 0 );
    uiDir = uiDir > iMostProbable ? uiDir - 1 : uiDir;
    if (uiDir < 31){ // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
      xWriteFlag( uiDir & 0x01 ? 1 : 0 );
      xWriteFlag( uiDir & 0x02 ? 1 : 0 );
      xWriteFlag( uiDir & 0x04 ? 1 : 0 );
      xWriteFlag( uiDir & 0x08 ? 1 : 0 );
      xWriteFlag( uiDir & 0x10 ? 1 : 0 );
    }
    else{
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( uiDir == 32 ? 1 : 0 );
    }
  }
}
#endif

Void TEncCavlc::codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iIntraDirLuma = pcCU->convertIntraDirLumaAdi( pcCU, uiAbsPartIdx );
  Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);

  xWriteFlag( iIntraDirLuma >= 0 ? 0 : 1 );

  if (iIntraDirLuma >= 0)
  {
    xWriteFlag((iIntraDirLuma & 0x01));

    xWriteFlag((iIntraDirLuma & 0x02) >> 1);

    if (g_aucIntraModeBits[iIntraIdx] >= 4)
    {
      xWriteFlag((iIntraDirLuma & 0x04) >> 2);

      if (g_aucIntraModeBits[iIntraIdx] >= 5)
      {
        xWriteFlag((iIntraDirLuma & 0x08) >> 3);

        if (g_aucIntraModeBits[iIntraIdx] >= 6)
        {
          xWriteFlag((iIntraDirLuma & 0x10) >> 4);
        }
      }
    }
  }
  return;
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );

  if ( 0 == uiIntraDirChroma )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
    xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, 3 );
  }

  return;
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  uiInterDir--;

  xWriteFlag( ( uiInterDir == 2 ? 1 : 0 ));

  if ( uiInterDir < 2 )
  {
    xWriteFlag( uiInterDir );
  }

  return;
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );

#ifdef QC_AMVRES
  if(pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
	  Bool Mvres = !pcCU->getCUMvField( eRefList )->getMv( uiAbsPartIdx ).isHAM();
   
	  if (iRefFrame == 0 )
	  {
		  if (Mvres)
			 xWriteFlag(0);
		  else
		  {
			 xWriteFlag(1);
			 xWriteFlag(0);
		  }
	  }
	  else
	  {
		  xWriteFlag(1);
		  xWriteFlag(1);
		  if (pcCU->getSlice()->getNumRefIdx( eRefList )>2)
	 		  xWriteUnaryMaxSymbol( iRefFrame-1, pcCU->getSlice()->getNumRefIdx( eRefList )-2);
	  }
  }
  else
  {
	  xWriteFlag( ( iRefFrame == 0 ? 0 : 1 ) );
	  if ( iRefFrame > 0 )
	  {
		xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
	  }
  }
#else

  xWriteFlag( ( iRefFrame == 0 ? 0 : 1 ) );

  if ( iRefFrame > 0 )
  {
    xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
  }
#endif
  return;
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();

  UInt uiAbsPartIdxL, uiAbsPartIdxA;
  Int iHorPred, iVerPred;

  TComDataCU* pcCUL   = pcCU->getPULeft ( uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx );

  TComCUMvField* pcCUMvFieldL = ( pcCUL == NULL || pcCUL->isIntra( uiAbsPartIdxL ) ) ? NULL : pcCUL->getCUMvField( eRefList );
  TComCUMvField* pcCUMvFieldA = ( pcCUA == NULL || pcCUA->isIntra( uiAbsPartIdxA ) ) ? NULL : pcCUA->getCUMvField( eRefList );
#ifdef QC_AMVRES
  if(pcCU->getSlice()->getSPS()->getUseAMVRes()&& (!pcCUMvField->getMv ( uiAbsPartIdx ).isHAM()) )
  {
	  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor()>>1) ) +
				 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor()>>1) );
	  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer()>>1) ) +
				 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer()>>1) );
      xWriteSvlc( iHor/2);
      xWriteSvlc( iVer/2);
	  return;
  }
#endif
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
       ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
       ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );

  xWriteSvlc( iHor );
  xWriteSvlc( iVer );

  return;
}

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getSlice()->getSliceQp();

  if ( iDQp == 0 )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
    xWriteSvlc( iDQp );
  }

  return;
}

Void TEncCavlc::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
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

  UInt uiCbf = pcCU->getCbf   ( uiAbsPartIdx, eType, uiTrDepth );

  xWriteFlag( uiCbf );

  return;
}

#if QC_MDDT
Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, UInt uiMode, Bool bRD )
#else
Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
#endif
{
  if ( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() ) {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;

  // point to coefficient
  TCoeff* piCoeff = pcCoef;
  UInt uiNumSig = 0;
  UInt uiScanning;
#if !QC_MDDT
  UInt uiInterleaving;
#endif

  // compute number of significant coefficients
  UInt  uiPart = 0;
  xCheckCoeff(piCoeff, uiWidth, 0, uiNumSig, uiPart );

  if ( bRD ) {
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }

  if ( uiNumSig == 0 ) {
    return;
  }

  // initialize scan
  const UInt*  pucScan;
#if NEWVLC
  //UInt uiConvBit = g_aucConvertToBit[ Min(8,uiWidth)    ];
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : Min(8,uiWidth)    ];
#else
  UInt uiConvBit = g_aucConvertToBit[ uiWidth    ];
#endif
#if HHI_RQT
  pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];
#else
  pucScan         = g_auiFrameScanXY  [ uiConvBit ];
#endif

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
  UInt *scanStats;
  int indexROT ;
  if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && (uiWidth == 4 || uiWidth == 8 || uiWidth == 16 || uiWidth == 32 || uiWidth == 64))//!bRD &&  eTType == TEXT_LUMA)
  {
	indexROT = pcCU->getROTindex(uiAbsPartIdx);
	int scan_index;
    if(uiWidth == 4)// && uiMode<=8&&indexROT == 0)
    {
      UInt uiPredMode = g_aucIntra9Mode[uiMode];
       pucScan = scanOrder4x4[uiPredMode]; //pucScanX = scanOrder4x4X[ipredmode]; pucScanY = scanOrder4x4Y[ipredmode];

	   if(g_bUpdateStats)
       {
         scanStats = scanStats4x4[uiPredMode]; update4x4Count[uiPredMode]++;
       }
    }
    else if(uiWidth == 8)// && uiMode<=8 && indexROT == 0)
    {
      UInt uiPredMode = ((1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth) ? g_aucIntra9Mode[uiMode] : g_aucAngIntra9Mode[uiMode];
      pucScan = scanOrder8x8[uiPredMode]; //pucScanX = scanOrder8x8X[ipredmode]; pucScanY = scanOrder8x8Y[ipredmode];

	   if(g_bUpdateStats)
      {
        scanStats = scanStats8x8[uiPredMode]; update8x8Count[uiPredMode]++;
      }
    }
	else if(uiWidth == 16)
	{
		scan_index = LUT16x16[indexROT][uiMode];
		pucScan = scanOrder16x16[scan_index]; //pucScanX = scanOrder16x16X[scan_index]; pucScanY = scanOrder16x16Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats16x16[scan_index];
		}
    }
    else if(uiWidth == 32)
    {
		scan_index = LUT32x32[indexROT][uiMode];
		pucScan = scanOrder32x32[scan_index]; //pucScanX = scanOrder32x32X[scan_index]; pucScanY = scanOrder32x32Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats32x32[scan_index];
		}
    }
    else if(uiWidth == 64)
    {
		scan_index = LUT64x64[indexROT][uiMode];
		pucScan = scanOrder64x64[scan_index]; //pucScanX = scanOrder64x64X[scan_index]; pucScanY = scanOrder64x64Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats64x64[scan_index];
		}
    }
    else
    {
      //printf("uiWidth = %d is not supported!\n", uiWidth);
      //exit(1);
    }
  }
#endif

  TCoeff scoeff[64];
  Int iBlockType;
#if !QC_MDDT
  UInt uiNumSigInterleaved;
#endif
#if NEWVLC
  UInt uiCodeDCCoef = 0;
  TCoeff dcCoeff = 0;
  if (pcCU->isIntra(uiAbsPartIdx))
  {
    UInt uiAbsPartIdxL, uiAbsPartIdxA;
    TComDataCU* pcCUL   = pcCU->getPULeft (uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    TComDataCU* pcCUA   = pcCU->getPUAbove(uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    if (pcCUL == NULL && pcCUA == NULL)
    {
#if 1
      uiCodeDCCoef = 1;
      xWriteVlc((eTType == TEXT_LUMA ? 3 : 1) , abs(piCoeff[0]));
      if (piCoeff[0] != 0)
      {
        UInt sign = (piCoeff[0] < 0) ? 1 : 0;
        xWriteFlag(sign);
      }
      dcCoeff = piCoeff[0];
      piCoeff[0] = 1;
#else
        xWriteFlag(1);
#endif
    }
  }
#endif

#if HHI_RQT
  if( uiSize == 2*2 )
  {
    // hack: re-use 4x4 coding
    ::memset( scoeff, 0, 16*sizeof(TCoeff) );
    for (uiScanning=0; uiScanning<4; uiScanning++)
    {
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
    }
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();

    xCodeCoeff4x4( scoeff, iBlockType );
  }
  else if ( uiSize == 4*4 )
#else
  if ( uiSize == 4*4 )
#endif
  {
    for (uiScanning=0; uiScanning<16; uiScanning++)
    {
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
      if(scoeff[15-uiScanning])
      {
        if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA)// && (uiWidth == 4 && uiMode<=8&&indexROT == 0))
        {
          scanStats[uiScanning]++;
        }
      }
#endif
    }
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();

    xCodeCoeff4x4( scoeff, iBlockType );
  }
  else if ( uiSize == 8*8 )
  {
    for (uiScanning=0; uiScanning<64; uiScanning++)
    {
      scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
      if(scoeff[63-uiScanning])
      {
        if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA)// && (uiWidth == 8 && uiMode<=8 && indexROT == 0))
        {
          scanStats[uiScanning]++;
        }
      }
#endif
    }
#if NEWVLC
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
#endif
    iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );

    xCodeCoeff8x8( scoeff, iBlockType );
  }
  else
  {
#if NEWVLC
    if(!pcCU->isIntra( uiAbsPartIdx ))
    {
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        scoeff[63-uiScanning] = piCoeff[(pucScan[uiScanning]/8)*uiWidth + (pucScan[uiScanning]%8)];      
	    }
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
      return;
    }    
#endif


#if QC_MDDT
    if(pcCU->isIntra( uiAbsPartIdx ))
    {
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        if(scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ])
        {
          if(g_bUpdateStats && eTType == TEXT_LUMA )
            scanStats[ uiScanning ]++;
        }
      }

      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
    }
#else
    for (uiInterleaving=0; uiInterleaving<uiSize/64; uiInterleaving++)
    {
      uiNumSigInterleaved = 0;
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        scoeff[63-uiScanning] = piCoeff[ pucScan[ (uiSize/64) * uiScanning + uiInterleaving ] ];

        if ( scoeff[63-uiScanning] )
        {
          uiNumSigInterleaved++;
        }
      }
      if ( uiNumSigInterleaved )
      {
        xWriteFlag( 1 );
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
        xCodeCoeff8x8( scoeff, iBlockType );
      }
      else
      {
        xWriteFlag( 0 );
      }
    }
#endif // QC_MDDT
//#endif
  }

#if NEWVLC
  if (uiCodeDCCoef == 1)
  {
    piCoeff[0] = dcCoeff;
  }
#endif

}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{
  xWriteFlag( uiCode );
}

Void TEncCavlc::codeAlfUvlc( UInt uiCode )
{
  xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
  xWriteSvlc( iCode );
}

Void TEncCavlc::estBit( estBitsSbacStruct* pcEstBitsCabac, UInt uiCTXIdx, TextType eTType )
{
#if !NEWVLC
  assert(0);
#endif
  // printf("error : no VLC mode support in this version\n");
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncCavlc::xWriteCode     ( UInt uiCode, UInt uiLength )
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncCavlc::xWriteUvlc     ( UInt uiCode )
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;

  assert ( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncCavlc::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;

  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
}

Void TEncCavlc::xWriteFlag( UInt uiCode )
{
  m_pcBitIf->write( uiCode, 1 );
}

Void TEncCavlc::xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart )
{
  UInt ui = uiSize>>uiDepth;
  if( uiPart == 0 )
  {
    if( ui <= 4 )
    {
      UInt x, y;
      TCoeff* pCeoff = pcCoef;
      for( y=0 ; y<ui ; y++ )
      {
        for( x=0 ; x<ui ; x++ )
        {
          if( pCeoff[x] != 0 )
          {
            uiNumofCoeff++;
          }
        }
        pCeoff += uiSize;
      }
    }
    else
    {
      xCheckCoeff( pcCoef,                            uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //1st Part
      xCheckCoeff( pcCoef             + (ui>>1),      uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //2nd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize,           uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //3rd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize + (ui>>1), uiSize, uiDepth+1, uiNumofCoeff, uiPart );           //4th Part
    }
  }
  else
  {
    UInt x, y;
    TCoeff* pCeoff = pcCoef;
    for( y=0 ; y<ui ; y++ )
    {
      for( x=0 ; x<ui ; x++ )
      {
        if( pCeoff[x] != 0 )
        {
          uiNumofCoeff++;
        }
      }
      pCeoff += uiSize;
    }
  }
}

Void TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
{
  xWriteFlag( uiSymbol ? 1 : 0 );

  if ( uiSymbol == 0 )
  {
    return;
  }

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    xWriteFlag( 1 );
  }
  if( bCodeLast )
  {
    xWriteFlag( 0 );
  }

  return;
}

Void TEncCavlc::xWriteExGolombLevel( UInt uiSymbol )
{
  if( uiSymbol )
  {
    xWriteFlag( 1 );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);

    while( --uiSymbol && ++uiCount < 13 )
    {
      xWriteFlag( 1 );
    }
    if( bNoExGo )
    {
      xWriteFlag( 0 );
    }
    else
    {
      xWriteEpExGolomb( uiSymbol, 0 );
    }
  }
  else
  {
    xWriteFlag( 0 );
  }

  return;
}

Void TEncCavlc::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xWriteFlag( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteFlag( 0 );
  while( uiCount-- )
  {
    xWriteFlag( (uiSymbol>>uiCount) & 1 );
  }

  return;
}

UInt TEncCavlc::xLeadingZeros(UInt uiCode)
{
  UInt uiCount = 0;
  Int iDone = 0;

  if (uiCode)
  {
    while (!iDone)
    {
      uiCode >>= 1;
      if (!uiCode) iDone = 1;
      else uiCount++;
    }
  }
  return uiCount;
}

Void TEncCavlc::xWriteVlc(UInt uiTableNumber, UInt uiCodeNumber)
{
  assert( uiTableNumber<=10 );

  UInt uiTemp;
  UInt uiLength = 0;
  UInt uiCode = 0;

  if ( uiTableNumber < 5 )
  {
    if ((Int)uiCodeNumber < (6 * (1 << uiTableNumber)))
    {
      uiTemp = 1<<uiTableNumber;
      uiCode = uiTemp+uiCodeNumber%uiTemp;
      uiLength = 1+uiTableNumber+(uiCodeNumber>>uiTableNumber);
    }
    else
    {
      uiCode = uiCodeNumber - (6 * (1 << uiTableNumber)) + (1 << uiTableNumber);
      uiLength = (6-uiTableNumber)+1+2*xLeadingZeros(uiCode);
    }
  }
  else if (uiTableNumber < 8)
  {
    uiTemp = 1<<(uiTableNumber-4);
    uiCode = uiTemp+uiCodeNumber%uiTemp;
    uiLength = 1+(uiTableNumber-4)+(uiCodeNumber>>(uiTableNumber-4));
  }
  else if (uiTableNumber == 8)
  {
    assert( uiCodeNumber<=2 );
    if (uiCodeNumber == 0)
    {
      uiCode = 1;
      uiLength = 1;
    }
    else if (uiCodeNumber == 1)
    {
      uiCode = 1;
      uiLength = 2;
    }
    else if (uiCodeNumber == 2)
    {
      uiCode = 0;
      uiLength = 2;
    }
  }
  else if (uiTableNumber == 9)
  {
    if (uiCodeNumber == 0)
    {
      uiCode = 4;
      uiLength = 3;
    }
    else if (uiCodeNumber == 1)
    {
      uiCode = 10;
      uiLength = 4;
    }
    else if (uiCodeNumber == 2)
    {
      uiCode = 11;
      uiLength = 4;
    }
    else if (uiCodeNumber < 11)
    {
      uiCode = uiCodeNumber+21;
      uiLength = 5;
    }
    else
    {
      uiTemp = 1<<4;
      uiCode = uiTemp+(uiCodeNumber+5)%uiTemp;
      uiLength = 5+((uiCodeNumber+5)>>4);
    }
  }
  else if (uiTableNumber == 10)
  {
    uiCode = uiCodeNumber+1;
    uiLength = 1+2*xLeadingZeros(uiCode);
  }
  xWriteCode(uiCode, uiLength);
}

Void TEncCavlc::xCodeCoeff4x4(TCoeff* scoeff, Int n )
{
  Int i;
  UInt cn;
  Int level,vlc,sign,done,last_pos,start;
  Int run_done,maxrun,run,lev;
  Int tmprun, vlc_adaptive=0;
  static const int atable[5] = {4,6,14,28,0xfffffff};
  Int tmp;

  /* Do the last coefficient first */
  i = 0;
  done = 0;

  while (!done && i < 16)
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i++;
    }
  }
  if (i == 16)
  {
    return;
  }

  last_pos = 15-i;
  level = abs(scoeff[i]);
  lev = (level == 1) ? 0 : 1;

  {
    int x,y,cx,cy,vlcNum;
    int vlcTable[3] = {2,2,2};

    x = 16*lev + last_pos;
    cx = m_uiLPTableE4[n][x];
    vlcNum = vlcTable[n];

    xWriteVlc( vlcNum, cx );

    if ( m_bAdaptFlag )
    {
      cy = Max( 0, cx-1 );
      y = m_uiLPTableD4[n][cy];
      m_uiLPTableD4[n][cy] = x;
      m_uiLPTableD4[n][cx] = y;
      m_uiLPTableE4[n][x] = cy;
      m_uiLPTableE4[n][y] = cx;
    }
  }

  sign = (scoeff[i] < 0) ? 1 : 0;
  if (level > 1)
  {
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
  }
  i++;

  if (i < 16)
  {
    /* Go into run mode */
    run_done = 0;
    while (!run_done)
    {
      maxrun = 15-i;
      tmprun = maxrun;
      if (maxrun > 27)
      {
        vlc = 3;
        tmprun = 28;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }

      run = 0;
      done = 0;
      while (!done)
      {
        if (!scoeff[i])
        {
          run++;
        }
        else
        {
          level = abs(scoeff[i]);
          lev = (level == 1) ? 0 : 1;
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
          xWriteVlc( vlc, cn );

          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
            xWriteVlc( 0, 2*(level-2)+sign );
            run_done = 1;
          }
          else
          {
            xWriteFlag( sign );
          }

          run = 0;
          done = 1;
        }
        if (i == 15)
        {
          done = 1;
          run_done = 1;
          if (run)
          {
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
            xWriteVlc( vlc, cn );
          }
        }
        i++;
      }
    }
  }

  /* Code the rest in level mode */
  start = i;
  for ( i=start; i<16; i++ )
  {
    tmp = abs(scoeff[i]);
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
    }
    if ( tmp > atable[vlc_adaptive] )
    {
      vlc_adaptive++;
    }
  }
  return;
}

Void TEncCavlc::xCodeCoeff8x8( TCoeff* scoeff, Int n )
{
  int i;
  unsigned int cn;
  int level,vlc,sign,done,last_pos,start;
  int run_done,maxrun,run,lev;
  int tmprun,vlc_adaptive=0;
  static const int atable[5] = {4,6,14,28,0xfffffff};
  int tmp;

  static const int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
  int sum_big_coef = 0;


  /* Do the last coefficient first */
  i = 0;
  done = 0;
  while (!done && i < 64)
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i++;
    }
  }
  if (i == 64)
  {
    return;
  }

  last_pos = 63-i;
  level = abs(scoeff[i]);
  lev = (level == 1) ? 0 : 1;

  {
    int x,y,cx,cy,vlcNum;
    x = 64*lev + last_pos;
    cx = m_uiLPTableE8[n][x];
    // ADAPT_VLC_NUM
    vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
    xWriteVlc( vlcNum, cx );

    if ( m_bAdaptFlag )
    {
        // ADAPT_VLC_NUM
        m_uiLastPosVlcIndex[n] += cx == m_uiLastPosVlcIndex[n] ? 0 : (cx < m_uiLastPosVlcIndex[n] ? -1 : 1);
        cy = Max(0,cx-1);
        y = m_uiLPTableD8[n][cy];
        m_uiLPTableD8[n][cy] = x;
        m_uiLPTableD8[n][cx] = y;
        m_uiLPTableE8[n][x] = cy;
        m_uiLPTableE8[n][y] = cx;
    }
  }

  sign = (scoeff[i] < 0) ? 1 : 0;
  if (level > 1)
  {
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
  }
  i++;

  if (i < 64)
  {
    /* Go into run mode */
    run_done = 0;
    while ( !run_done )
    {
      maxrun = 63-i;
      tmprun = maxrun;
      if (maxrun > 27)
      {
        vlc = 3;
        tmprun = 28;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }

      run = 0;
      done = 0;
      while (!done)
      {
        if (!scoeff[i])
        {
          run++;
        }
        else
        {
          level = abs(scoeff[i]);
          lev = (level == 1) ? 0 : 1;
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
          xWriteVlc( vlc, cn );

          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
            xWriteVlc( 0, 2*(level-2)+sign );

            sum_big_coef += level;
            if (i > switch_thr[n] || sum_big_coef > 2)
            {
              run_done = 1;
            }
          }
          else
          {
            xWriteFlag( sign );
          }
          run = 0;
          done = 1;
        }
        if (i == 63)
        {
          done = 1;
          run_done = 1;
          if (run)
          {
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
            xWriteVlc( vlc, cn );
          }
        }
        i++;
      }
    }
  }

  /* Code the rest in level mode */
  start = i;
  for ( i=start; i<64; i++ )
  {
    tmp = abs(scoeff[i]);
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
    }
    if (tmp>atable[vlc_adaptive])
    {
      vlc_adaptive++;
    }
  }

  return;
}

#ifdef QC_SIFO

Void TEncCavlc::encodeSwitched_Filters(TComSlice* pcSlice,TComPrediction *m_cPrediction)
{
  if(pcSlice->getSliceType() != I_SLICE)
  {	
    UInt num_AVALABLE_FILTERS = m_cPrediction->getNum_AvailableFilters();
    UInt num_SIFO = m_cPrediction->getNum_SIFOFilters();

    Int bitsPerFilter=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); 
    Int bitsPer2Filters=(Int)ceil(log10((Double)num_SIFO)/log10((Double)2)); 
    Int sub_pos;
    //Int bestFilter = m_cPrediction->getBestFilter();

#ifdef QC_SIFO_PRED
    Int predict_filter_flag = pcSlice->getSPS()->getUseSIFO_Pred()? 1 : 0;
    Int predFiltP = m_cPrediction->getPredictFilterP();
    Int predFiltB = m_cPrediction->getPredictFilterB();

    if (predict_filter_flag && pcSlice->getSliceType()==P_SLICE && predFiltP<2)
      predict_filter_flag = 0;
    if (predict_filter_flag && pcSlice->getSliceType()==B_SLICE && predFiltB<2)
      predict_filter_flag = 0;

      xWriteFlag ( predict_filter_flag );
#endif

    if (pcSlice->getSliceType()==P_SLICE)
    {
      Int bestFilter = m_cPrediction->getBestFilter_P();
      Int predictFilterP = m_cPrediction->getPredictFilterP();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      if (predictFilterP < 2)
#endif
      {
        xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
          xWriteCode(bestFilter==1,1);
          if (bestFilter>1)
            xWriteCode(bestFilter-2,bitsPerFilter);
        }

        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
            else
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPer2Filters);
          }
        }

        if (bestFilter == 1)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
      }
      else
      {				// Predictive coding
        xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceP(sub_pos);
            xWriteCode(predict,1);
            if (predict==0)
            {
              if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
                xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
              else
                xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPer2Filters);
            }
          }
        }
      }
    }
    else  //B slice
    {
      Int bestFilter = m_cPrediction->getBestFilter_B();
      Int predictFilterB = m_cPrediction->getPredictFilterB();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      if (predictFilterB < 2)
#endif
      {
        xWriteCode(bestFilter==0,1);
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
        else
        {
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
      else
      {
        xWriteCode(bestFilter==0,1);
        if (bestFilter==0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceB(sub_pos);
            xWriteCode(predict,1);
            if (predict!=1)
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
        else
        {
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
    }
  }

//----encode Offsets
  if(pcSlice->getSliceType() != I_SLICE)
  {
    Int offset;
    Int listNo = (pcSlice->getSliceType() == B_SLICE)? 2: 1;
    for(Int list = 0; list < listNo; ++list) 
    {
      UInt nonzero = 0;
      nonzero = m_cPrediction->isOffsetZero(pcSlice, list);

      xWriteCode(nonzero,1);
      if(nonzero)
      {
        for(UInt frame = 0; frame < pcSlice->getNumRefIdx(RefPicList(list)); ++frame)
        {
          if(frame == 0)     
          {    
            for(UInt sub_pos = 0; sub_pos < 16; ++sub_pos)   
            {
              offset = m_cPrediction->getSubpelOffset(list,sub_pos);
              offset /= (1<<g_uiBitIncrement);
              xWriteSvlc(offset);
            }         
          }
          else              
          {
            offset = m_cPrediction->getFrameOffset(list,frame);
            offset /= (1<<g_uiBitIncrement);
            xWriteSvlc(offset);
          }
        }
      }
    }
  }

}
#endif
