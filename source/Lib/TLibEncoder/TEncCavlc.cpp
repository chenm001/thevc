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

/** \file			TEncCavlc.cpp
    \brief		CAVLC encoder class
*/

#include "TEncCavlc.h"

//-- baekeun.lee@samsung.com
#define RUNBEFORE_NUM				7
#define MAX_VALUE           0xdead

//-- baekeun.lee@samsung.com
const UChar g_aucCbpIntra[48] =
{
   3, 29, 30, 17,
  31, 18, 37,  8,
  32, 38, 19,  9,
  20, 10, 11,  2,
  16, 33, 34, 21,
  35, 22, 39,  4,
  36, 40, 23,  5,
  24,  6,  7,  1,
  41, 42, 43, 25,
  44, 26, 46, 12,
  45, 47, 27, 13,
  28, 14, 15,  0
};

const UChar g_aucCbpInter[48] =
{
   0,  2,  3,  7,
   4,  8, 17, 13,
   5, 18,  9, 14,
  10, 15, 16, 11,
   1, 32, 33, 36,
  34, 37, 44, 40,
  35, 45, 38, 41,
  39, 42, 43, 19,
   6, 24, 25, 20,
  26, 21, 46, 28,
  27, 47, 22, 29,
  23, 30, 31, 12
};
//--

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
  m_bRunLengthCoding  = false;   //  m_bRunLengthCoding  = !rcSliceHeader.isIntra();
  m_uiCoeffCost       = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TEncCavlc::~TEncCavlc()
{

}

Void TEncCavlc::resetEntropy()
{
  m_bRunLengthCoding = ! m_pcSlice->isIntra();
  m_uiRun = 0;
}

Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
  // Structure
	xWriteUvlc  ( pcSPS->getWidth () );
	xWriteUvlc  ( pcSPS->getHeight() );

  xWriteUvlc  ( pcSPS->getPad (0) );
	xWriteUvlc  ( pcSPS->getPad (1) );

  xWriteUvlc	( pcSPS->getMaxCUWidth ()   );
  xWriteUvlc	( pcSPS->getMaxCUHeight()   );
  xWriteUvlc	( pcSPS->getMaxCUDepth ()-1 ); //xWriteUvlc	( pcSPS->getMaxCUDepth ()-g_uiAddCUDepth );

  // Transform
  xWriteUvlc	( pcSPS->getMinTrDepth()   );
  xWriteUvlc	( pcSPS->getMaxTrDepth()   );

	// Max transform size
  xWriteUvlc  ( pcSPS->getMaxTrSize() == 2 ? 0 : g_aucConvertToBit[pcSPS->getMaxTrSize()]+1 );

	// Tools
	xWriteFlag	( (pcSPS->getUseALF ()) ? 1 : 0 );
	xWriteFlag	( (pcSPS->getUseDQP ()) ? 1 : 0 );
	xWriteFlag	( (pcSPS->getUseWPG () || pcSPS->getUseWPO ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseQBO ()) ? 1 : 0 );

	// write number of taps for DIF
  xWriteUvlc  ( (pcSPS->getDIFTap ()>>1)-2 );	// 4, 6, 8, 10, 12

	// AMVP mode for each depth
	for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
	{
		xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
	}

	// Bit-depth information
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
  xWriteUvlc( pcSPS->getBitIncrement() );
}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
  xWriteCode  (pcSlice->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  xWriteUvlc  (pcSlice->getSliceType() );
  xWriteSvlc  (pcSlice->getSliceQp() );
  xWriteFlag  (pcSlice->getSymbolMode() ? 1 : 0);

  if (!pcSlice->isIntra())
    xWriteFlag  (pcSlice->isReferenced() ? 1 : 0);

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

#if AMVP_NEIGH_COL
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    xWriteFlag( pcSlice->getColDir() );
  }
#endif
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
  //assert (0);
  return;
}

Void TEncCavlc::codeSliceFinish ()
{
  if ( m_bRunLengthCoding && m_uiRun)
  {
    xWriteUvlc(m_uiRun);
  }
}


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

Void TEncCavlc::xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes )
{
  m_pcBitIf->write( g_aucCodeTableTO4[uiTrailingOnes][uiCoeffCount],
                    g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount] );

  return;
}

Void TEncCavlc::xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiVal;
  UInt uiSize;

  if( 3 == uiLastCoeffCount )
  {
    UInt uiBits = 3;
    if( uiCoeffCount )
    {
      uiBits = (uiCoeffCount-1)<<2 | uiTrailingOnes;
    }
    m_pcBitIf->write ( uiBits, 6 );

    uiVal   = uiBits;
    uiSize  = 6;
  }
  else
  {
    m_pcBitIf->write( g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount],
                      g_aucLenTableTO16 [uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] );

    uiVal   = g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
    uiSize  = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
  }

  return;
}

Void  TEncCavlc::xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun )
{
  if( 0 == uiCoeffCnt )
  {
    return;
  }

  if( uiTrailingOnes )
  {
    UInt uiBits = 0;
    Int n = uiTrailingOnes-1;
    for( UInt k = uiCoeffCnt; k > uiCoeffCnt-uiTrailingOnes; k--, n--)
    {
      if( aiLevelRun[k-1] < 0)
      {
        uiBits |= 1<<n;
      }
    }

    m_pcBitIf->write( uiBits, uiTrailingOnes );
  }

  Int iHighLevel = ( uiCoeffCnt > 3 && uiTrailingOnes == 3) ? 0 : 1;
  Int iVlcTable  = ( uiCoeffCnt > 10 && uiTrailingOnes < 3) ? 1 : 0;

  for( Int k = uiCoeffCnt - 1 - uiTrailingOnes; k >= 0; k--)
  {
    Int iLevel;
    iLevel = aiLevelRun[k];

    UInt uiAbsLevel = (UInt)abs(iLevel);

    if( iHighLevel )
    {
      iLevel -= ( iLevel > 0 ) ? 1 : -1;
	    iHighLevel = 0;
    }

    if( iVlcTable == 0 )
    {
	    xWriteLevelVLC0( iLevel );
    }
    else
    {
	    xWriteLevelVLCN( iLevel, iVlcTable );
    }

    // update VLC table
    if( uiAbsLevel > g_auiIncVlc[ iVlcTable ] )
    {
      iVlcTable++;
    }

    if( k == Int(uiCoeffCnt - 1 - uiTrailingOnes) && uiAbsLevel > 3)
    {
      iVlcTable = 2;
    }

  }

  //ROFRS( uiCoeffCnt < uiMaxCoeffs, Err::m_nOK );
  if (uiCoeffCnt >= uiMaxCoeffs)
  {
    return;
  }


  iVlcTable = uiCoeffCnt-1;
  if( uiMaxCoeffs <= 4 )
  {
    xWriteTotalRun4( iVlcTable, uiTotalRun );
  }
  else
  {
    xWriteTotalRun16( iVlcTable, uiTotalRun );
  }

  // decode run before each coefficient
  uiCoeffCnt--;
  if( uiTotalRun > 0 && uiCoeffCnt > 0)
  {
    do
    {
      iVlcTable = (( uiTotalRun > RUNBEFORE_NUM) ? RUNBEFORE_NUM : uiTotalRun) - 1;
      UInt uiRun = aiLevelRun[uiCoeffCnt+0x10];

      xWriteRun( iVlcTable, uiRun );

      uiTotalRun -= uiRun;
      uiCoeffCnt--;
    } while( uiTotalRun != 0 && uiCoeffCnt != 0);
  }

  return;
}

Void	TEncCavlc::xWriteExGolombLevel		(UInt	uiLevel)
{
	if( uiLevel )
  {
    xWriteFlag(1);
    UInt uiCount = 0;
    Bool bNoExGo = (uiLevel < 13);

    while( --uiLevel && ++uiCount < 13 )
    {
      xWriteFlag(1);
    }
    if( bNoExGo )
    {
      xWriteFlag(0);
    }
    else
    {
      xWriteEpExGolomb( uiLevel, 0 );
    }
  }
  else
  {
    xWriteFlag( 0);
  }

  return;
}

Void	TEncCavlc::xWriteEpExGolomb(UInt uiLevel, UInt uiCount ){

	while( uiLevel >= (UInt)(1<<uiCount) )
  {
    xWriteFlag( 1 );
    uiLevel -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteFlag( 0 );
  while( uiCount-- )
  {
    xWriteFlag( (uiLevel>>uiCount) & 1 );
  }

	return;

}

Void TEncCavlc::xWriteLevelVLC0( Int iLevel )
{

  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;

  UInt uiBits;

  if( 8 > uiLevel )
  {
    uiBits   = 1;
    uiLength = 2 * uiLevel - 1 + uiSign;
  }
  else if( 16 > uiLevel )
  {
    uiBits   = 2*uiLevel + uiSign;
    uiLength = 15 + 4;
  }
  else
  {
    uiBits   = 0x1000-32 + (uiLevel<<1) + uiSign;
    uiLength = 16 + 12;
  }


  m_pcBitIf->write( uiBits, uiLength );

  return;
}


Void TEncCavlc::xWriteLevelVLCN( Int iLevel, UInt uiVlcLength )
{
  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;
  UInt uiBits;

  UInt uiShift = uiVlcLength-1;
  UInt uiEscapeCode = (0xf<<uiShift)+1;

  if( uiLevel < uiEscapeCode )
  {
    uiLevel--;
	  uiLength = (uiLevel>>uiShift) + uiVlcLength + 1;
    uiLevel &= ~((0xffffffff)<<uiShift);
	  uiBits   = (2<<uiShift) | 2*uiLevel | uiSign;
  }
  else
  {
	  uiLength = 28;
	  uiBits   = 0x1000 + 2*(uiLevel-uiEscapeCode) + uiSign;
  }

  m_pcBitIf->write( uiBits, uiLength );

  return;
}

Void TEncCavlc::xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun )
{
  m_pcBitIf->write( g_aucCodeTableTZ4[uiVlcPos][uiTotalRun],
                    g_aucLenTableTZ4 [uiVlcPos][uiTotalRun] );

  return;
}


Void TEncCavlc::xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun )
{
  m_pcBitIf->write( g_aucCodeTableTZ16[uiVlcPos][uiTotalRun],
                    g_aucLenTableTZ16 [uiVlcPos][uiTotalRun] );

  return;
}

Void TEncCavlc::xWriteRun( UInt uiVlcPos, UInt uiRun  )
{
  m_pcBitIf->write( g_aucCodeTable3[uiVlcPos][uiRun],
                    g_aucLenTable3 [uiVlcPos][uiRun] );

  return;
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum    = pcCU->getMVPNum( eRefList, uiAbsPartIdx );

  if ( iNum > 1 )
  {
    if ( iSymbol == 0 )
    {
      xWriteFlag(0);
    }
    else
    {
      xWriteFlag(1);

      for (int i = 0; i < (iSymbol-1); i++)
      {
        xWriteFlag(1);
      }
      if ( ( iSymbol+1 ) != iNum )
      {
        xWriteFlag(0);
      }
    }
  }
}

Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx )? 1 : 0;
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfCtrlDepth()
{
  if (!m_bAlfCtrl)
    return;

  UInt uiDepth = m_uiMaxAlfCtrlDepth;

  if ( uiDepth == 0 )
  {
    xWriteFlag(0);
  }
  else
  {
    xWriteFlag(1);

    for (int i = 0; i < (uiDepth-1); i++)
    {
      xWriteFlag(1);
    }
    if ( uiDepth < g_uiMaxCUDepth-1 )
    {
      xWriteFlag(0);
    }
  }
}

Void TEncCavlc::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
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

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;

  UInt uiFlag = ( pcCU->getDepth( uiAbsPartIdx ) == uiDepth ) ? 0 : 1;
  xWriteFlag( uiFlag );
}

Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

  UInt uiMostProbable = (UInt)(pcCU->getMostProbablePartSize( uiAbsPartIdx ));
  UInt uiSize         = (UInt)(pcCU->getPartitionSize( uiAbsPartIdx ));
  UInt uiSymbol       = ( uiMostProbable == uiSize ) ? 1 : 0;

  if( pcCU->getSlice()->isIntra() )		// for flexible size intra, delete this part
  {
    xWriteFlag( uiSymbol );           // Square size intra only : 2Nx2N, NxN
    return;
  }

  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );

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

      if ( pcCU->getSlice()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_2NxN)
        {
          xWriteFlag( 1 );
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( eSize == SIZE_2NxnU? 0: 1 );
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

      if ( pcCU->getSlice()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_Nx2N)
        {
          xWriteFlag( 1 );
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( eSize == SIZE_nLx2N? 0: 1);
        }
      }
      break;
    }
  case SIZE_NxN:
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
      xWriteFlag( 0 );
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
  Int iPredMode = pcCU->convertPredMode( uiAbsPartIdx );
  xWriteFlag( iPredMode >= 0 ? 0 : 1 );

  if ( iPredMode >= 0 )
  {
    xWriteFlag( iPredMode & 0x01 );
  }
}

Void TEncCavlc::codeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  xWriteUvlc( pcCU->getTransformIdx( uiAbsPartIdx ) );
}

Void TEncCavlc::codeIntraDirLuma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UChar uiMode = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if( uiMode == 2 ) // DC mode
  {
    xWriteFlag( 1 );
    return;
  }
  xWriteFlag( 0 );

  if ( uiMode > 2 )
    uiMode--;

  while(uiMode)
  {
    xWriteFlag( 0 );
    uiMode--;
  }
  xWriteFlag( 1 );
}

Void TEncCavlc::codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UChar uiMode = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if( uiMode == 2 ) // DC mode
  {
    xWriteFlag( 1 );
    return;
  }
  xWriteFlag( 0 );

  if ( uiMode > 2 )
    uiMode--;

  while(uiMode)
  {
    xWriteFlag( 0 );
    uiMode--;
  }
  xWriteFlag( 1 );
}



Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiMode = pcCU->getChromaIntraDir( uiAbsPartIdx );

  xWriteUvlc(uiMode);
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  xWriteUvlc( (UInt)pcCU->getInterDir( uiAbsPartIdx ) - 1 );
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  UInt uiRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );

  Bool bWriteBit = ( 2 == pcCU->getSlice()->getNumRefIdx( eRefList ) );

  if ( bWriteBit )
  {
    xWriteCode( 1 - uiRefFrame, 1 );
  }
  else
  {
    xWriteUvlc( uiRefFrame );
  }

  return;
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
  UInt uiTemp;
  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();

  uiTemp = xConvertToUInt( iHor );
  xWriteUvlc( uiTemp );

  uiTemp = xConvertToUInt( iVer );
  xWriteUvlc( uiTemp );

  return;
}

Void TEncCavlc::codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  xWriteSvlc( pcCU->getQP(uiAbsPartIdx) - pcCU->getSlice()->getSliceQp());
}

Void TEncCavlc::codeCbf       ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType, uiTrDepth );
  xWriteFlag( uiCbf );
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

Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
{
  UInt uiSize   = uiWidth*uiHeight;

  if( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }

  // initialize scan
  const UInt*  pucScan;
  const UInt*  pucScanX;
  const UInt*  pucScanY;

  UInt uiConvBit = g_aucConvertToBit[ uiWidth ];

  switch(uiWidth)
  {
  case  2:
    {
      static UInt uiScanOrder [4] = {0, 1, 2, 3};
      static UInt uiScanOrderX[4] = {0, 1, 0, 1};
      static UInt uiScanOrderY[4] = {0, 0, 1, 1};
      pucScan  = &uiScanOrder [0];
      pucScanX = &uiScanOrderX[0];
      pucScanY = &uiScanOrderY[0];
      break;
    }
  case  4:
  case  8: pucScan = g_auiFrameScanXY[ uiConvBit ]; pucScanX = g_auiFrameScanX[uiConvBit]; pucScanY = g_auiFrameScanY[uiConvBit]; break;
  case 16:
  case 32:
  case 64:
  default: pucScan = g_auiFrameScanXY[ uiConvBit ]; pucScanX = g_auiFrameScanX[uiConvBit]; pucScanY = g_auiFrameScanY[uiConvBit]; break;
  }

	// point to coefficient
  TCoeff* piCoeff = pcCoef;
  UInt uiNumSig = 0;
  UInt ui;

	// compute number of significant coefficients
  UInt  uiPart = 0;
  xCheckCoeff(piCoeff, uiWidth, 0, uiNumSig, uiPart );

  if ( bRD )
	{
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
//     if ( pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA && pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN && eTType == TEXT_LUMA )
//     {
//       uiTempDepth = 1;
//     }
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
		}

  if ( uiNumSig == 0 )
		return;

	UInt uiCodedSig = 0;

  //----- encode significance map -----
	for( ui = 0; ui < (UInt)(uiSize-1); ui++ ) // if last coeff is reached, it has to be significant
	{
		UInt uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;
		xWriteFlag( uiSig );

		if( uiSig )
		{
			UInt uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
			xWriteFlag( uiLast );

			if( uiLast)
			{
				break;
			}
		}
	}

  //----- encode significant coefficients -----
  ui++;
  while( (ui--) != 0 )
  {
    UInt  uiAbs, uiSign;
    Int   iCoeff = piCoeff[ pucScan[ ui ] ];

    if( iCoeff )
    {
      if( iCoeff > 0) { uiAbs = static_cast<UInt>( iCoeff);  uiSign = 0; }
      else            { uiAbs = static_cast<UInt>(-iCoeff);  uiSign = 1; }

      UInt uiSymbol = uiAbs > 1 ? 1 : 0;
      xWriteFlag( uiSymbol );

      if( uiSymbol )
      {
        uiAbs -= 2;
        xWriteUvlc(uiAbs);
      }
      xWriteFlag( uiSign );
    }
  }
	return;
}

Void TEncCavlc::codeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  UChar uiMode = pcCU->getCIPflag( uiAbsPartIdx );
  xWriteFlag( uiMode ? 1 : 0 );
}

Void TEncCavlc::codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx , Bool bRD)
{
  UChar uiMode = pcCU->getROTindex( uiAbsPartIdx );
  xWriteUvlc( uiMode );
}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{
   xWriteFlag(uiCode);
}

Void TEncCavlc::codeAlfUvlc( UInt uiCode )
{
  xWriteUvlc(uiCode);
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
  xWriteSvlc(iCode);
}

Void TEncCavlc::estBit (estBitsSbacStruct* pcEstBitsCabac, UInt uiCTXIdx, TextType eTType)
{
	assert(0);
  // printf("error : no VLC mode support in this version\n");
  return;
}
