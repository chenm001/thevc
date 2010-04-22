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

/** \file			TDecCAVLC.cpp
    \brief		CAVLC decoder class
*/

#include "TDecCAVLC.h"

//-- baekeun.lee@samsung.com
#define RUNBEFORE_NUM  7

const UChar g_aucCbpIntra[48] =
{
   47, 31, 15,  0,
   23, 27, 29, 30,
    7, 11, 13, 14,
   39, 43, 45, 46,
   16,  3,  5, 10,
   12, 19, 21, 26,
   28, 35, 37, 42,
   44,  1,  2,  4,
    8, 17, 18, 20,
   24,  6,  9, 22,
   25, 32, 33, 34,
   36, 40, 38, 41
};


const UChar g_aucCbpInter[48] =
{
    0, 16,  1,  2,
    4,  8, 32,  3,
    5, 10, 12, 15,
   47,  7, 11, 13,
   14,  6,  9, 31,
   35, 37, 42, 44,
   33, 34, 36, 40,
   39, 43, 45, 46,
   17, 18, 20, 24,
   19, 21, 26, 28,
   23, 27, 29, 30,
   22, 25, 38, 41
};
//--

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecCavlc::TDecCavlc()
{
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TDecCavlc::~TDecCavlc()
{

}

Void TDecCavlc::parseSPS(TComSPS* pcSPS)
{
  UInt  uiCode;

	// Structure
	xReadUvlc ( uiCode ); pcSPS->setWidth				( uiCode		);
  xReadUvlc ( uiCode ); pcSPS->setHeight			( uiCode		);
  xReadUvlc ( uiCode ); pcSPS->setPadX			  ( uiCode		);
  xReadUvlc ( uiCode ); pcSPS->setPadY			  ( uiCode		);

  xReadUvlc ( uiCode ); pcSPS->setMaxCUWidth	( uiCode		); g_uiMaxCUWidth  = uiCode;
  xReadUvlc ( uiCode ); pcSPS->setMaxCUHeight	( uiCode		); g_uiMaxCUHeight = uiCode;
  xReadUvlc ( uiCode ); pcSPS->setMaxCUDepth	( uiCode+1	); g_uiMaxCUDepth  = uiCode + 1;

  // Transform
  xReadUvlc ( uiCode ); pcSPS->setMinTrDepth	( uiCode		);
  xReadUvlc ( uiCode ); pcSPS->setMaxTrDepth	( uiCode		);

	// Max transform size
  xReadUvlc	( uiCode ); pcSPS->setMaxTrSize   ( (uiCode == 0) ? 2 : (1<<(uiCode+1)) );

	// Tool on/off
  xReadFlag( uiCode ); pcSPS->setUseMVAC( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseADI ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseROT ( uiCode ? true : false );
	xReadFlag( uiCode ); // reserved
	xReadFlag( uiCode ); pcSPS->setUseMPI ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseAMVP( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseIMR ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseDIF ( uiCode ? true : false );
  xReadFlag( uiCode ); // reserved
	xReadFlag( uiCode ); pcSPS->setUseALF ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseDQP ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseRNG ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseAMP ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseSHV ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseWPG ( uiCode ? true : false );
	xReadFlag( uiCode ); pcSPS->setUseWPR ( uiCode ? true : false );
	xReadUvlc( uiCode );
  if (uiCode)
  {
    if (uiCode == 1)			pcSPS->setUseIME ( true );
    else if (uiCode == 2)	pcSPS->setUseAME ( true );
    else if (uiCode == 3)	pcSPS->setUsePME ( true );
    else								assert(0);
  }
  xReadFlag( uiCode ); pcSPS->setUseLOT ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseExC ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseCCP ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseTMI ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseLDC ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseCIP ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseLCT ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseQBO ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseACS ( uiCode ? true : false );

  xReadFlag( uiCode ); pcSPS->setUseHAP ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseHAB ( uiCode ? true : false );

	xReadUvlc( uiCode ); pcSPS->setDIFTap ( (uiCode+2)<<1 );	// 4, 6, 8, 10, 12
	xReadUvlc( uiCode ); pcSPS->setDIFTapC( (uiCode+1)<<1 );  // 2, 4, 6,  8, 10, 12

	xReadFlag( uiCode ); g_bUseCADR =     ( uiCode ? true : false );
	if ( g_bUseCADR )
	{
		xReadUvlc( uiCode ); g_iMinCADR = uiCode;
		xReadUvlc( uiCode ); g_iMaxCADR = (1<<CADR_BITS) - uiCode - 1;
		g_iRangeCADR = ( g_iMaxCADR - g_iMinCADR + 1 );
	}

  // Parameters
	if ( pcSPS->getUseAMVP() )
	{
		for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
		{
			xReadFlag( uiCode );
			pcSPS->setAMVPMode( i, (AMVP_MODE)uiCode );
		}
	}

  xReadUvlc( uiCode ); pcSPS->setBitDepth			( uiCode+8 ); g_uiBitDepth     = uiCode + 8;
  xReadUvlc( uiCode ); pcSPS->setBitIncrement	( uiCode	 ); g_uiBitIncrement = uiCode;
	g_uiIBDI_MAX = ((1<<(pcSPS->getBitDepth()+pcSPS->getBitIncrement()))-1);

  g_uiAddCUDepth = 0;
  if( !pcSPS->getUseLOT() && ((g_uiMaxCUWidth>>(g_uiMaxCUDepth-1)) > pcSPS->getMaxTrSize()) )
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

  xReadCode (10, uiCode);  rpcSlice->setPOC              (uiCode);             // 9 == SPS->Log2MaxFrameNum()
  xReadUvlc (   uiCode);  rpcSlice->setSliceType        ((SliceType)uiCode);
  xReadSvlc (    iCode);  rpcSlice->setSliceQp          (iCode);
  xReadFlag (   uiCode);  rpcSlice->setSymbolMode				(uiCode);

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

    if (rpcSlice->getSPS()->getUseWPR())
    {
      xReadCode (1, uiCode);
      rpcSlice->setRFmode(uiCode);
      if (rpcSlice->getRFmode())
      {
        for (Int n=0; n<iNumPredDir; n++)
        {
          RefPicList eRefPicList = (RefPicList)n;

          rpcSlice->addEffectMode(eRefPicList, EFF_WP_P1);
          rpcSlice->initWPAllParam(eRefPicList, EFF_WP_P1);

          rpcSlice->addEffectMode(eRefPicList, EFF_WP_M1);
          rpcSlice->initWPAllParam(eRefPicList, EFF_WP_M1);
        }
      }
    }

    if (rpcSlice->getSPS()->getUseIME() || rpcSlice->getSPS()->getUseAME() || rpcSlice->getSPS()->getUsePME())
    {
      for (Int n=0; n<iNumPredDir; n++)
      {
        RefPicList eRefPicList = (RefPicList)n;

        xReadFlag(uiCode);

        if (uiCode)
        {
          if (rpcSlice->getSPS()->getUseIME())
          {
            rpcSlice->setGMmode(eRefPicList, 1);
            rpcSlice->addEffectMode(eRefPicList, EFF_IM);
          }
          else if (rpcSlice->getSPS()->getUseAME())
          {
            rpcSlice->setGMmode(eRefPicList, 2);
            rpcSlice->addEffectMode(eRefPicList, EFF_AM);
          }
          else if (rpcSlice->getSPS()->getUsePME())
          {
            rpcSlice->setGMmode(eRefPicList, 3);
            rpcSlice->addEffectMode(eRefPicList, EFF_PM);
					}

          for (UInt ui=0; ui<rpcSlice->getNumOfSpritePoints(); ui++)
          {
            TrajPoint cTrajPoint;
            xReadSvlc (iCode);  cTrajPoint.x = iCode;
            xReadSvlc (iCode);  cTrajPoint.y = iCode;
            rpcSlice->setDiffTrajPoint(eRefPicList, ui, cTrajPoint);
          }
        }
        else
        {
          rpcSlice->setGMmode(eRefPicList, 0);
        }
      }
    }
  }

#if AMVP_NEIGH_COL
  if (rpcSlice->getSPS()->getUseAMVP() && rpcSlice->getSliceType() == B_SLICE)
  {
    xReadFlag (uiCode);
    rpcSlice->setColDir(uiCode);
  }
#endif

  if ( rpcSlice->getSPS()->getUseMVAC() && rpcSlice->isInterB() )
  {
    xReadFlag ( uiCode );
    rpcSlice->setUseMVAC( uiCode ? true : false );
  }

  if( ( rpcSlice->getSliceType() == P_SLICE && rpcSlice->getSPS()->getUseHAP() ) || ( rpcSlice->getSliceType() == B_SLICE && rpcSlice->getSPS()->getUseHAB() ) )
    rpcSlice->setUseHAM( true );
  else
    rpcSlice->setUseHAM( false );

	return;
}

Void TDecCavlc::resetEntropy          (TComSlice* pcSlice)
{
  m_bRunLengthCoding = ! pcSlice->isIntra();
  m_uiRun = 0;
}

Void	TDecCavlc::xGetRunLevel					(	Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt& uiTotalRun)
{
  if ( !uiCoeffCnt ) return ;

  if( uiTrailingOnes )
  {
    UInt uiBits;
    m_pcBitstream->read(uiTrailingOnes, uiBits);

    Int n = uiTrailingOnes-1;
    for( UInt k = uiCoeffCnt; k > uiCoeffCnt-uiTrailingOnes; k--, n--)
    {
      aiLevelRun[k-1] = (uiBits & (1<<n)) ? -1 : 1;
    }
  }

  UInt uiHighLevel = ( uiCoeffCnt > 3 && uiTrailingOnes == 3) ? 0 : 1;
  UInt uiVlcTable  = ( uiCoeffCnt > 10 && uiTrailingOnes < 3) ? 1 : 0;

  for( Int k = uiCoeffCnt - 1 - uiTrailingOnes; k >= 0; k--)
  {
    Int iLevel;

    if( uiVlcTable == 0 )
    {
      xReadLevelVLC0( iLevel );
    }
    else
    {
      xReadLevelVLCN( iLevel, uiVlcTable );
    }

    if( uiHighLevel )
    {
      iLevel += ( iLevel > 0 ) ? 1 : -1;
      uiHighLevel = 0;
    }
    aiLevelRun[k] = iLevel;

    UInt uiAbsLevel = (UInt)abs(iLevel);

    if( uiAbsLevel > g_auiIncVlc[ uiVlcTable ] )
    {
      uiVlcTable++;
    }

    if( k == Int(uiCoeffCnt - 1 - uiTrailingOnes) && uiAbsLevel > 3)
    {
      uiVlcTable = 2;
    }
  }

  if (uiCoeffCnt >= uiMaxCoeffs) return;

  uiVlcTable = uiCoeffCnt-1;
  if( uiMaxCoeffs <= 4 )
  {
    xReadTotalRun4( uiVlcTable, uiTotalRun );
  }
  else
  {
    xReadTotalRun16( uiVlcTable, uiTotalRun );
  }

  // decode run before each coefficient
  for ( UInt i = 0; i < uiCoeffCnt; i++ )
  {
    aiLevelRun[i + 0x10] = 0;
  }
  uiCoeffCnt--;
  UInt uiRunCount = uiTotalRun;
  if( uiRunCount > 0 && uiCoeffCnt > 0)
  {
    do
    {
      uiVlcTable = (( uiRunCount > RUNBEFORE_NUM) ? RUNBEFORE_NUM : uiRunCount) - 1;
      UInt uiRun;

      xReadRun( uiVlcTable, uiRun );
      aiLevelRun[uiCoeffCnt+0x10] = uiRun;

      uiRunCount -= uiRun;
      uiCoeffCnt--;
    } while( uiRunCount != 0 && uiCoeffCnt != 0);
  }

  return ;
}

Void TDecCavlc::xGetTrailingOnes4			( UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  xCodeFromBitstream2D( &g_aucCodeTableTO4[0][0], &g_aucLenTableTO4[0][0], 5, 4, uiCoeffCount, uiTrailingOnes ) ;
  return ;
}

Void TDecCavlc::xReadTotalRun4(	UInt& uiVlcPos, UInt& uiTotalRun )
{
  UInt uiTemp ;
  xCodeFromBitstream2D( &g_aucCodeTableTZ4[uiVlcPos][0], &g_aucLenTableTZ4[uiVlcPos][0], 4, 1, uiTotalRun, uiTemp );
  return ;
}

Void TDecCavlc::xReadTotalRun16( UInt uiVlcPos, UInt& uiTotalRun )
{
  UInt uiTemp;
  xCodeFromBitstream2D( &g_aucCodeTableTZ16[uiVlcPos][0], &g_aucLenTableTZ16[uiVlcPos][0], 16, 1, uiTotalRun, uiTemp );
  return ;
}

Void TDecCavlc::xGetTrailingOnes16( UInt uiLastCoeffCount, UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  if( 3 == uiLastCoeffCount )
  {
    UInt uiBits;
    m_pcBitstream->read(6, uiBits);

    uiTrailingOnes = ( uiBits & 0x3 );
    uiCoeffCount   = ( uiBits >>  2 );
    if ( !uiCoeffCount && uiTrailingOnes == 3 )
    {
      uiTrailingOnes = 0;
    }
    else
    {
      uiCoeffCount++;
    }
  }
  else
  {
    assert (uiLastCoeffCount < 3);
    xCodeFromBitstream2D( &g_aucCodeTableTO16[uiLastCoeffCount][0][0], &g_aucLenTableTO16[uiLastCoeffCount][0][0], 17, 4, uiCoeffCount, uiTrailingOnes ) ;
  }
  return ;
}

Void TDecCavlc::xReadRun( UInt uiVlcPos , UInt& uiRun )
{
  UInt uiTemp;
  xCodeFromBitstream2D( &g_aucCodeTable3[uiVlcPos][0], &g_aucLenTable3[uiVlcPos][0], 15, 1, uiRun, uiTemp ) ;
  return ;
}

Void TDecCavlc::xReadLevelVLC0( Int& iLevel )
{
  UInt uiLength = 0;
  UInt uiCode   = 0;
  UInt uiTemp = 0;
  UInt uiSign = 0;
  UInt uiLevel = 0;

  do
  {
    m_pcBitstream->read(1, uiTemp);
    uiLength++;
    uiCode = ( uiCode << 1 ) + uiTemp;
  } while ( uiCode == 0 );

  if ( uiLength < 15 )
  {
    uiSign  = (uiLength - 1) & 1;
    uiLevel = (uiLength - 1) / 2 + 1;
  }
  else if (uiLength == 15)
  {
    // escape code
    m_pcBitstream->read(4, uiTemp);
    uiCode = (uiCode << 4) | uiTemp;
    uiLength += 4;
    uiSign  = (uiCode & 1);
    uiLevel = ((uiCode >> 1) & 0x7) + 8;
  }
  else if (uiLength >= 16)
  {
    // escape code
    UInt uiAddBit = uiLength - 16;
    m_pcBitstream->read(uiLength-4, uiCode);
    uiLength -= 4;
    uiSign    = (uiCode & 1);
    uiLevel   = (uiCode >> 1) + (2048<<uiAddBit)+16-2048;
    uiCode   |= (1 << (uiLength)); // for display purpose only
    uiLength += uiAddBit + 16;
  }

  iLevel = uiSign ? -(Int)uiLevel : (Int)uiLevel;
  return ;
}


Void TDecCavlc::xReadLevelVLCN( Int& iLevel, UInt uiVlcLength )
{
  UInt uiTemp;
  UInt uiLength;
  UInt uiCode;
  UInt uiLevAbs;
  UInt uiSb;
  UInt uiSign;
  UInt uiAddBit;
  UInt uiOffset;

  UInt uiNumPrefix = 0;
  UInt uiShift     = uiVlcLength - 1;
  UInt uiEscape    = (15<<uiShift)+1;

  // read pre zeros
  do
  {
    m_pcBitstream->read(1,uiTemp);
    uiNumPrefix++;
  } while ( uiTemp == 0 );

  uiLength = uiNumPrefix;
  uiCode   = 1;
  uiNumPrefix--;

  if (uiNumPrefix < 15)
  {
    uiLevAbs = (uiNumPrefix<<uiShift) + 1;

    if ( uiVlcLength-1 )
    {
      m_pcBitstream->read(uiVlcLength-1,uiSb);
      uiCode = (uiCode << (uiVlcLength-1) )| uiSb;
      uiLevAbs += uiSb;
      uiLength += (uiVlcLength-1);

    }

    // read 1 bit -> sign
    m_pcBitstream->read(1,uiSign);
    uiCode = (uiCode << 1)| uiSign;
    uiLength++;
  }
  else // escape
  {
    uiAddBit = uiNumPrefix - 15;

    m_pcBitstream->read((11+uiAddBit), uiSb);
    uiCode = (uiCode << (11+uiAddBit) )| uiSb;

    uiLength += (11+uiAddBit);
    uiOffset  = (2048<<uiAddBit)+uiEscape-2048;
    uiLevAbs  = uiSb + uiOffset;

    // read 1 bit -> sign
    m_pcBitstream->read(1, uiSign);
    uiCode = (uiCode << 1)| uiSign;
    uiLength++;
  }

  iLevel = (uiSign) ? -(Int)uiLevAbs : (Int)uiLevAbs;

  return ;
}

Void TDecCavlc::xCodeFromBitstream2D(const UChar* aucCode, const UChar* aucLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 )
{
  const UChar *paucLenTab;
  const UChar *paucCodTab;
  UChar  uiLenRead = 0;
  UChar  uiCode    = 0;
  UChar  uiMaxLen  = 0;

  // Find maximum number of bits to read before generating error
  paucLenTab = aucLen;
  paucCodTab = aucCode;
  for (UInt j = 0; j < uiHeight; j++, paucLenTab += uiWidth, paucCodTab += uiWidth)
  {
    for ( UInt i = 0; i < uiWidth; i++ )
    {
      if ( paucLenTab[i] > uiMaxLen )
      {
        uiMaxLen = paucLenTab[i];
      }
    }
  }

  while ( uiLenRead < uiMaxLen )
  {
    // Read next bit
    UInt uiBit;
    m_pcBitstream->read(1, uiBit);
    uiCode = ( uiCode << 1 ) + uiBit;
    uiLenRead++;

    // Check for matches
    paucLenTab = aucLen;
    paucCodTab = aucCode;
    for (UInt j = 0; j < uiHeight; j++, paucLenTab += uiWidth, paucCodTab += uiWidth)
    {
      for (UInt i = 0; i < uiWidth; i++)
      {
        if ( (paucLenTab[i] == uiLenRead) && (paucCodTab[i] == uiCode) )
        {
          uiVal1 = i;
          uiVal2 = j;
          return ;
        }
      }
    }
  }
  return ;
}
//--

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

Void TDecCavlc::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiFlag;
  xReadFlag( uiFlag );

  pcCU->setPredictionMode( uiAbsPartIdx, MODE_SKIP );

  if( uiFlag )
  {
    pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );

		if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
		{
			TComMv cZeroMv(0,0);

      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
			pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
		}

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
  Int iVal = 0;
  UInt uiSymbol ;

  if( iMVPNum > 1 )
  {
    xReadFlag( uiSymbol );

    if( uiSymbol == 0 )
    {
      riMVPIdx	= iVal;
    }
    else
    {
      while ( uiSymbol == 1 )
      {
        iVal++;
        if ( iVal == iMVPNum-1 ) break;
        xReadFlag( uiSymbol );
      }
      riMVPIdx	= iVal;
    }
  }
  else
  {
    riMVPIdx	= iVal;
  }

  return;
}

Void TDecCavlc::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiFlag;
  xReadFlag( uiFlag );

  pcCU->setDepth( uiAbsPartIdx, uiDepth + uiFlag );
}

Void TDecCavlc::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

}

Void TDecCavlc::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

}

Void TDecCavlc::parseIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UChar uiMode = 0;
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

  if( uiSymbol ) // DC mode
  {
    pcCU->setLumaIntraDir( uiAbsPartIdx, 2 );
    return;
  }

  while( !uiSymbol )
  {
    xReadFlag( uiSymbol );
    uiMode++;
  }

  if( uiMode < 2 )
  {
    uiMode--;
  }

  pcCU->setLumaIntraDir( uiAbsPartIdx, uiMode );
}

Void TDecCavlc::parseIntraDirLumaAdi  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UChar uiMode = 0;
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

  if( uiSymbol ) // DC mode
  {
    pcCU->setLumaIntraDir( uiAbsPartIdx, 2 );
    return;
  }

  while( !uiSymbol )
  {
    xReadFlag( uiSymbol );
    uiMode++;
  }

  if( uiMode < 2 )
  {
    uiMode--;
  }

  pcCU->setLumaIntraDir( uiAbsPartIdx, uiMode );
}

Void TDecCavlc::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadUvlc(uiSymbol);

  pcCU->setChromaIntraDir( uiAbsPartIdx, uiSymbol );
}

Void TDecCavlc::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
}

Void TDecCavlc::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
}
Void TDecCavlc::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  //pcMb->getMbMvd
  Int iHor = 0 ; Int iVer = 0 ;
  UInt uiTemp = 0 ;
  xReadUvlc(uiTemp);
  iHor = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
  xReadUvlc(uiTemp);
  iVer = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);

  // MVAC
  if ( pcCU->getSlice()->getUseMVAC() )
  {
    UInt	uiHor;
    UInt	uiVer;

    Int	iSignHor;
    Int iSignVer;

    uiHor	= (UInt)abs(iHor);
    uiVer	=	(UInt)abs(iVer);

    iSignHor = iHor >= 0 ? 1 : -1;
    iSignVer = iVer >= 0 ? 1 : -1;

    uiHor <<=	1;
    uiVer <<=	1;

    iHor = uiHor * iSignHor;
    iVer = uiVer * iSignVer;
  }
  //--MVAC

  TComMv  cMv (iHor, iVer);

	pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  return;
}

Void TDecCavlc::parseTransformIdx  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiVal;
  xReadUvlc( uiVal );
  pcCU->setTransformIdx(uiAbsPartIdx, uiVal);
}

Void TDecCavlc::parseDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  Int uiVal;
  xReadSvlc( uiVal );
  pcCU->setQP( uiAbsPartIdx, pcCU->getSlice()->getSliceQp() + uiVal );
}

Void TDecCavlc::parseCbf           ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType, uiTrDepth);

  xReadFlag( uiSymbol );

  pcCU->setCbfSubParts( uiCbf | ( uiSymbol << uiTrDepth ), eType, uiAbsPartIdx, uiDepth + uiTrDepth );

  return;
}

Void TDecCavlc::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  UInt uiSize   = uiWidth*uiHeight;

  // initialize scan
	const UInt*  pucScan;
	pucScan = g_auiFrameScanXY[SCAN_ZIGZAG][ g_aucConvertToBit[ uiWidth ] ];

	// point to coefficient
  TCoeff* piCoeff = pcCoef;

  // read bcbp here
	UInt ui;
	xReadFlag( ui );
	if ( !ui ) return;

  //----- parse significance map -----
	for( ui = 0; ui < uiSize-1; ui++ ) // if last coeff is reached, it has to be significant
	{
		UInt uiSig, uiLast;

		xReadFlag( uiSig );

		piCoeff[ pucScan[ui] ] = uiSig;

		if( uiSig )
		{
			xReadFlag( uiLast );

			if( uiLast)
			{
				break;
			}
		}
	}

  //--- last coefficient must be significant if no last symbol was received ---
  if( ui == uiSize-1 )
  {
    piCoeff[pucScan[ui]] = 1;
  }

  ui++;
  while( (ui--) != 0 )
  {
    UInt  uiCoeff = piCoeff[ pucScan[ui] ];

    if( uiCoeff )
    {
      xReadFlag( uiCoeff );

      if( 1 == uiCoeff )
      {
        xReadUvlc( uiCoeff );
        uiCoeff += 2;
      }
      else
      {
        uiCoeff++;
      }

      UInt uiSign;
      xReadFlag( uiSign );

      piCoeff[ pucScan[ui] ] = ( uiSign ? -(TCoeff)uiCoeff : (TCoeff)uiCoeff );
    }
  }

  return;
}

Void TDecCavlc::parseMPIindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UChar uiMode = 0;
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

  uiMode = uiSymbol;

  pcCU->setMPIindex( uiAbsPartIdx, uiMode );
}

Void TDecCavlc::parseCIPflag  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

	pcCU->setCIPflagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
}

Void TDecCavlc::parseExtremeValue( Int& iMinVal, Int& iMaxVal, Int iExtremeType)
{}

//EXCBand
Void TDecCavlc::parseCorrBandExType( Int& iCorVal, Int& iBandNumber)
{
}
//EXCBand

Void TDecCavlc::parseROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UChar uiMode = 0;
  UInt  uiSymbol;
  xReadFlag( uiSymbol );

  uiMode = uiSymbol;

  pcCU->setROTindex( uiAbsPartIdx, uiMode );
}

Void TDecCavlc::parseAlfCtrlDepth              ( UInt& ruiAlfCtrlDepth )
{
  Int iVal = 0;
  UInt uiSymbol ;

  xReadFlag( uiSymbol );

  if( uiSymbol == 0 )
  {
    ruiAlfCtrlDepth	= iVal;
  }
  else
  {
    while ( uiSymbol == 1 )
    {
      iVal++;
      if ( iVal == ruiAlfCtrlDepth-1 ) break;
      xReadFlag( uiSymbol );
    }
    ruiAlfCtrlDepth	= iVal;
  }
}

Void TDecCavlc::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (!m_bAlfCtrl)
    return;

  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  UInt uiFlag;
  xReadFlag( uiFlag );


  if (uiDepth > m_uiMaxAlfCtrlDepth)
  {
    pcCU->setAlfCtrlFlagSubParts( uiFlag, uiAbsPartIdx, m_uiMaxAlfCtrlDepth);
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiFlag, uiAbsPartIdx, uiDepth );
  }
}

Void TDecCavlc::parseAlfFlag (UInt& ruiVal)
{
  xReadFlag(ruiVal);
}

Void TDecCavlc::parseAlfUvlc (UInt& ruiVal)
{
  xReadUvlc(ruiVal);
}

Void TDecCavlc::parseAlfSvlc (Int&  riVal)
{
  xReadSvlc(riVal);
}

Void TDecCavlc::parseULTUsedFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  printf("What????????? Impasible!!");
}
