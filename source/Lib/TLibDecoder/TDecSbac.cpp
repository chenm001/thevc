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

/** \file			TDecSbac.cpp
    \brief		SBAC decoder class
*/

#include "TDecSbac.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() :
  // new structure here
    m_cCUSplitFlagSCModel ( 1,             1,               3                )
  , m_cCUSkipFlagSCModel  ( 1,             1,               3                )
  , m_cCUAlfCtrlFlagSCModel ( 1,             1,               3                )
  , m_cCUPartSizeSCModel  ( 1,						 1,								7								 )
  , m_cCUXPosiSCModel     ( 1,						 1,								7								 )
  , m_cCUYPosiSCModel     ( 1,						 1,								7								 )
  , m_cCUPredModeSCModel  ( 1,             1,								2								 )
  , m_cCUIntraPredSCModel ( 1,             1,               NUM_TMI_CTX      )
  , m_cCUChromaPredSCModel( 1,             1,               4                )
  , m_cCUInterDirSCModel  ( 1,             1,               4                )
  , m_cCUMvdSCModel       ( 1,             2,               NUM_MV_RES_CTX   )
  , m_cCUMvDdSignCModel    ( 1,             2,              3                )
#if HAM_OVH_OPT && !HAM_ZEROMV_REP
  , m_cCUMvDdSCModel       ( 1,             2,              NUM_MV_RES_CTX   )
#else
  , m_cCUMvDdSCModel       ( 1,             3,              NUM_MAP_CTX+1    )
#endif
  , m_cCUULTUseSCModel     ( 1,             1,              3    )
  , m_cCURefPicSCModel    ( 1,             2,               NUM_REF_NO_CTX   )
  , m_cCUTransIdxSCModel  ( 1,             1,               NUM_TRANSFORM_SIZE_CTX )
  , m_cCUDeltaQpSCModel   ( 1,             1,               NUM_DELTA_QP_CTX )
  , m_cCUCbfSCModel       ( 1,             3,               NUM_CBF_CTX      )
  //Depth
  , m_cCUMapSCModel       ( MAX_CU_DEPTH,  2,               NUM_MAP_CTX      )
  , m_cCULastSCModel      ( MAX_CU_DEPTH,  2,               NUM_LAST_CTX     )
  , m_cCUOneSCModel       ( MAX_CU_DEPTH,  2,               NUM_ONE_CTX      )
  , m_cCUAbsSCModel       ( MAX_CU_DEPTH,  2,               NUM_ABS_CTX      )

  // ACS
  , m_cScanSCModel        ( MAX_CU_DEPTH,  2,               2                )

  , m_cCUTSigMapSCModel   ( 1,						 2,               NUM_MAP_CTX      )
  , m_cMVPIdxSCModel      ( 2, 3, 4 )

  , m_cCUMPIindexSCModel	( 1,             1,								3								 )
  , m_cCUROTindexSCModel	( 1,             1,								3								 )
  , m_cCUCIPflagCCModel		( 1,             1,								3								 )

  , m_cCUExtremValSCModel ( 1,             1,								6                )

	//EXCBand
    , m_cCUBandCorrValSCModel ( 1,             1,								6                )

	, m_cALFFlagSCModel		( 1,             1,               3                )
	, m_cALFUvlcSCModel		( 1,             1,               3                )
	, m_cALFSvlcSCModel		( 1,             1,               3                ){
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TDecSbac::~TDecSbac()
{
}

Void TDecSbac::resetEntropy          (TComSlice* pcSlice)
{
  Int  iQp              = pcSlice->getSliceQp();
  SliceType eSliceType  = pcSlice->getSliceType();

  m_cCUSplitFlagSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );

  m_cCUSkipFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUAlfCtrlFlagSCModel.initBuffer  ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUPartSizeSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_PART_SIZE );
  m_cCUXPosiSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_CU_Y_POS );
  m_cCUPredModeSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer   ( eSliceType, iQp, (Short*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_REF_PIC );

  m_cCUDeltaQpSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_DQP );
  m_cCUCbfSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_CBF );

  m_cCUMapSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_SIGMAP );
  m_cCULastSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_LAST_FLAG );
  m_cCUOneSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_TCOEFF_LEVEL );

  // ACS
  m_cScanSCModel.initBuffer			      ( eSliceType, iQp, (Short*)INIT_SIGMAP );

  m_cMVPIdxSCModel.initBuffer					(	eSliceType, iQp, (Short*)INIT_MVP_IDX );

  m_cCUMPIindexSCModel.initBuffer			( eSliceType, iQp, (Short*)INIT_MPI_IDX );
  m_cCUROTindexSCModel.initBuffer			( eSliceType, iQp, (Short*)INIT_ROT_IDX );

#if CIP_FIX
  m_cCUCIPflagCCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CIP_IDX );
#endif

  m_cCUExtremValSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_EXTREME_VALUE );

  // EXCBand
  m_cCUBandCorrValSCModel.initBuffer  ( eSliceType, iQp, (Short*)INIT_BAND_COR_VALUE );

	m_cALFFlagSCModel.initBuffer			  ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
	m_cALFUvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
	m_cALFSvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
  m_cCUTransIdxSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_TRANS_IDX );

  m_uiRange           = 0x10000 - 1;
  m_uiValue           = 0;
  m_uiWord            = 0;
  m_uiBitsLeft        = 0;

  m_uiLastDQpNonZero  = 0;

  // new structure
  m_uiLastQp          = iQp;
}

Void TDecSbac::parseMPIindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt  indexMPI = 0;
  switch (MPI_DICT)
  {
  case 4:
    {
      xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 0 ) );
      indexMPI  = uiSymbol;
      xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 1 ) );
      indexMPI |= uiSymbol << 1;
    }
    break;
  case 2:
    {
      xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 0) );
      if ( !uiSymbol ) indexMPI =1;
    }
    break;
  case 5:
    {
      xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 0) );
      if ( !uiSymbol )
      {
        xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 1 ) );
        indexMPI  = uiSymbol;
        xReadSymbol( uiSymbol, m_cCUMPIindexSCModel.get( 0, 0, 2 ) );
        indexMPI |= uiSymbol << 1;
        indexMPI=indexMPI+1;
      }
    }
    break;
  case 1:
    {
    }
    break;
  }

  pcCU->setMPIindexSubParts( (UChar)indexMPI, uiAbsPartIdx, uiDepth );
  return;
}

// CIP
Void TDecSbac::parseCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  iCtx = pcCU->getCtxCIPFlag( uiAbsPartIdx );

  xReadSymbol( uiSymbol, m_cCUCIPflagCCModel.get( 0, 0, iCtx ) );
  pcCU->setCIPflagSubParts( (UChar)uiSymbol, uiAbsPartIdx, uiDepth );
}

//EXCBand
Void TDecSbac::parseCorrBandExType( Int& iCorVal, Int& iBandNumber)
{
UInt uiSymbol;
	  xReadSymbol( uiSymbol, m_cCUBandCorrValSCModel.get( 0, 0, 0 ) );
	  if (uiSymbol==0)
	  {
		  iCorVal=0; return;
	  }
    Int Sign=1;
      xReadSymbol( uiSymbol, m_cCUBandCorrValSCModel.get( 0, 0, 1 ) );
	  if (uiSymbol==0) Sign=-1;
   uiSymbol=1;
	Int i=0;
	iCorVal=0;
	  for (i=0;i<4096; i++)
	  {
	   xReadSymbol( uiSymbol, m_cCUBandCorrValSCModel.get( 0, 0, 2 ) );
	   if (uiSymbol==0) break;
	   	   iCorVal++;
	  }
	  iCorVal=(iCorVal+1)*Sign;
}
//EXCBand

Void TDecSbac::parseExtremeValue( Int& iMinVal, Int& iMaxVal, Int iExtremeType)
{
	UInt uiSymbol;
	  xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 0 ) );
      iMinVal  = uiSymbol;
      xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 1 ) );
      iMinVal |= uiSymbol << 1;
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 2 ) );
      iMinVal |= uiSymbol << 2;
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 3 ) );
      iMinVal |= uiSymbol << 3;
	if (iExtremeType>2)
	{
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 4 ) );
      iMinVal |= uiSymbol << 4;
	if (iExtremeType>3)
	{
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 5 ) );
      iMinVal |= uiSymbol << 5;
	}
	}

	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 0 ) );
      iMaxVal  = uiSymbol;
      xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 1 ) );
      iMaxVal |= uiSymbol << 1;
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 2 ) );
      iMaxVal |= uiSymbol << 2;
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 3 ) );
      iMaxVal |= uiSymbol << 3;
	if (iExtremeType>2)
	{
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 4 ) );
      iMaxVal |= uiSymbol << 4;
	if (iExtremeType>3)
	{
	xReadSymbol( uiSymbol, m_cCUExtremValSCModel.get( 0, 0, 5 ) );
      iMaxVal |= uiSymbol << 5;
	}
	}
}
Void TDecSbac::parseROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt indexROT = 0;

  Int dictSize;
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
    dictSize = ROT_DICT;
  else
    dictSize = ROT_DICT_INTER;

  switch (dictSize)
  {
	case 9:
    {
      xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 0) );
      if ( !uiSymbol )
      {
        xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        indexROT  = uiSymbol;
        xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
        indexROT |= uiSymbol << 1;
		xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
        indexROT |= uiSymbol << 2;
        indexROT++;
      }
    }
    break;
  case 4:
    {
      xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      indexROT  = uiSymbol;
      xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 1 ) );
      indexROT |= uiSymbol << 1;
    }
    break;
  case 2:
    {
      xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 0) );
      if ( !uiSymbol ) indexROT =1;
    }
    break;
  case 5:
    {
      xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 0) );
      if ( !uiSymbol )
      {
        xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        indexROT  = uiSymbol;
        xReadSymbol( uiSymbol, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
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

__inline Void TDecSbac::xReadSymbol( UInt& ruiSymbol, SbacContextModel& rcSCModel )
{
	UInt uiRange = m_uiRange;
	UInt uiValue = m_uiValue;
	UInt uiLPS;
	UChar ucNextStateLPS, ucNextStateMPS;

	uiLPS = g_auiStandardProbFsm[rcSCModel.getState()];
	ucNextStateLPS = uiLPS & 0xFF;
	uiLPS >>= 8;
	ucNextStateMPS = uiLPS & 0xFF;
	uiLPS >>= 8;

	UInt uiRangeIdx = (uiRange>>13)&3;

	uiLPS += ( (uiLPS >> (g_auiShiftParameters[uiRangeIdx][0])) + (uiLPS >> (g_auiShiftParameters[uiRangeIdx][1])) );

	ruiSymbol = rcSCModel.getMps();
	uiRange -= uiLPS;

	if ( uiValue < uiRange )
	{
		if ( uiRange >= 0x8000 )
		{
			m_uiRange = uiRange;
			m_uiValue = uiValue;
			return;
		}
		else
		{
			if ( uiRange < uiLPS )
			{
				ruiSymbol = 1 - ruiSymbol;
				rcSCModel.setStateMps(ucNextStateLPS);
			}
			else
			{
				rcSCModel.setStateMps(ucNextStateMPS);
			}
		}
	}
	else
	{
		uiValue -= uiRange;
		if ( uiRange < uiLPS )
		{
			rcSCModel.setStateMps(ucNextStateMPS);
		}
		else
		{
			ruiSymbol = 1 - ruiSymbol;
			rcSCModel.setStateMps(ucNextStateLPS);
		}
		uiRange = uiLPS;
	}

	do
	{
		uiRange <<= 1;
		xReadBit(uiValue);
	}
	while ( uiRange < 0x8000 );

	m_uiRange = uiRange;
	m_uiValue = uiValue;
}

__inline Void TDecSbac::xReadBit( UInt& ruiValue )
{
  if( 0 == m_uiBitsLeft-- )
  {
    m_pcBitstream->read( 8, m_uiWord );
    m_uiBitsLeft = 7;
  }
  ruiValue += ruiValue + ((m_uiWord >> 7)&1);
  m_uiWord <<= 1;
  ruiValue &= 0xFFFF;
}

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{

  xReadTerminatingBit (ruiBit);
}

Void TDecSbac::xReadTerminatingBit( UInt& ruiSymbol )
{
  UInt uiRange = m_uiRange - 1;
  UInt uiValue = m_uiValue;

  if ( uiValue < uiRange ) {
    ruiSymbol = 0;
    if ( uiRange >= 0x8000 ) {
      m_uiRange = uiRange;
      m_uiValue = uiValue;
      return;
    }
  }
  else {
    ruiSymbol = 1;
    uiValue -= uiRange;
    uiRange = 1;
    return;
  }

  do {
    uiRange <<= 1;
    xReadBit(uiValue);
  } while ( uiRange < 0x8000 );

  m_uiRange = uiRange;
  m_uiValue = uiValue;
}

Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, SbacContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  xReadSymbol( ruiSymbol, pcSCModel[0] );

  if (ruiSymbol == 0 || uiMaxSymbol == 1)
  {
    return;
  }

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    xReadSymbol( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );

  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }

  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadMvd( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx )
{
  UInt uiLocalCtx = uiCtx;

  if( uiAbsSum >= 3)
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 3 : 2;
  }

  riMvdComp = 0;

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUMvdSCModel.get( 0, 0, uiLocalCtx ) );

  if (!uiSymbol)
  {
    return;
  }

  xReadExGolombMvd( uiSymbol, &m_cCUMvdSCModel.get( 0, 1, uiCtx ), 3 );
  uiSymbol++;

  UInt uiSign;
  xReadEpSymbol( uiSign );

  riMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;

  return;
}

#if HAM_ZEROMV_REP
Void TDecSbac::xReadMvDN( Int& riHorD, Int& riVerD )
{
  UInt uiSymbol = 0;
  xReadSymbol( uiSymbol, m_cCUMvDdSignCModel.get( 0, 0, 0 ) );

  if( uiSymbol == 0 )
  {
    riHorD = 0;
    riVerD = 0;
  }
  else
  {
    UInt ui = 0, uiB = 0;

    while( 1 )
    {
      xReadSymbol( uiSymbol, m_cCUMvDdSCModel.get(0, 0, ui) );

      if( uiSymbol )
        break;

      uiB |= 1<<ui;

      ui++;

      if( ui == HAM_RANGE )
        break;
    }

    riHorD = ui;

    ui = 0;
    UInt uiAdd = (riHorD == 0) ? 1 : 0;
    while( 1 )
    {
      if( ui == (HAM_RANGE-uiAdd) )
        break;

      xReadSymbol( uiSymbol, m_cCUMvDdSCModel.get(0, ((uiB>>ui)&1)+1, ui) );

      if( uiSymbol )
        break;

      ui++;
    }

    riVerD = ui+uiAdd;

    if( riHorD != 0 )
    {
      xReadSymbol( uiSymbol, m_cCUMvDdSignCModel.get(0, 0, 1) );
      if( uiSymbol )
        riHorD = -riHorD;
    }

    if( riVerD != 0 )
    {
      xReadSymbol( uiSymbol, m_cCUMvDdSignCModel.get(0, 1, (riHorD == 0) ? 0 : ( (riHorD > 0) ? 1 : 2 ) ) );
      if( uiSymbol )
        riVerD = -riVerD;
    }
  }
}
#endif
Void TDecSbac::xReadMvDd( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx )
{
#if HAM_OVH_OPT
  UInt uiLocalCtx = uiCtx;

  if( uiAbsSum >= 3)
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 3 : 2;
  }

  riMvdComp = 0;

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUMvDdSCModel.get( 0, 0, uiLocalCtx ) );

  if (!uiSymbol)
  {
    return;
  }

   xReadSymbol( uiSymbol, m_cCUMvDdSCModel.get( 0, 0, 3 ) );
   if (uiSymbol == 0) riMvdComp = -1;
   else riMvdComp = 1;

#else
  UInt ui = 0;
  UInt uiSymbol, uiSign;
  UInt uiMV = 0;
  while( true )
  {
    xReadSymbol( uiSymbol, m_cCUMvDdSCModel.get( 0, uiCtx, ui ) );
    uiMV += uiSymbol;
    if( uiSymbol )
      break;

    ui++;
  }

  uiMV = ui;

  if( uiMV )
  {
    xReadEpSymbol( uiSign );

    if( uiSign )
    {
      riMvdComp = -uiMV;
    }
    else
    {
      riMvdComp = uiMV;
    }
  }
  else
  {
    riMvdComp = 0;
  }
#endif
  return;
}

Void TDecSbac::xReadExGolombMvd( UInt& ruiSymbol, SbacContextModel* pcSCModel, UInt uiMaxBin )
{
  UInt uiSymbol;

  xReadSymbol( ruiSymbol, pcSCModel[0] );

  if (!ruiSymbol) { return; }

  xReadSymbol( uiSymbol, pcSCModel[1] );

  ruiSymbol = 1;

  if (!uiSymbol)  { return; }

  pcSCModel += 2;
  UInt uiCount = 2;

  do
  {
    if( uiMaxBin == uiCount )
    {
      pcSCModel++;
    }
    xReadSymbol( uiSymbol, *pcSCModel );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 8));

  ruiSymbol = uiCount-1;

	if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 3 );
    ruiSymbol += uiSymbol+1;
  }

  return;
}

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;


  while( uiBit )
  {
    xReadEpSymbol( uiBit );
    uiSymbol += uiBit << uiCount++;
  }

  uiCount--;
	while( uiCount-- )
  {
    xReadEpSymbol( uiBit );
  	uiSymbol += uiBit << uiCount;
  }

  ruiSymbol = uiSymbol;

  return;
}

Void TDecSbac::xReadEpSymbol( UInt& ruiSymbol )
{
  UInt uiRange = m_uiRange >> 1;
  UInt uiValue = m_uiValue;

  if ( uiValue < uiRange ) {
    ruiSymbol = 0;
  }
  else {
    uiValue -= uiRange;
    ruiSymbol = 1;
  }

  uiRange <<= 1;
  xReadBit(uiValue);

  m_uiRange = uiRange;
  m_uiValue = uiValue;
}

Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, SbacContextModel* pcSCModel, Int iOffset )
{
  xReadSymbol( ruiSymbol, pcSCModel[0] );

  if (!ruiSymbol) { return; }

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    xReadSymbol( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont );

  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadExGolombLevel( UInt& ruiSymbol, SbacContextModel& rcSCModel  )
{
  UInt uiSymbol;
  UInt uiCount = 0;
  do
  {
    xReadSymbol( uiSymbol, rcSCModel );
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

Void TDecSbac::preloadSbac              ()
{
  m_pcBitstream->readAlignOne();
  m_pcBitstream->setModeSbac();

  while( !m_pcBitstream->isWordAligned() && ( 8 > m_uiBitsLeft) )
  {
    UInt uiByte;
    m_pcBitstream->read( 8, uiByte );
    m_uiWord <<= 8;
    m_uiWord += uiByte;
    m_uiBitsLeft += 8;
  }

  m_uiWord <<= 8-m_uiBitsLeft;

  for( UInt n = 0; n < 16; n++ )
  {
    xReadBit( m_uiValue );
  }

  return;
}

Void TDecSbac::parseAlfCtrlDepth              ( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, m_cALFUvlcSCModel.get(0), 1, g_uiMaxCUDepth-1);
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecSbac::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (!m_bAlfCtrl)
    return;

  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx ) ) );

  if (uiDepth > m_uiMaxAlfCtrlDepth)
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth);
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

Void TDecSbac::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, pcCU->getCtxSkipFlag( uiAbsPartIdx ) ) );

  if( uiSymbol )
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

Void TDecSbac::parseMVPIdx      ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, iMVPNum-1);
  riMVPIdx = uiSymbol;
 }

Void TDecSbac::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return;
  }

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth ) ) );
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );

  return;
}

Void TDecSbac::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    return;
  }

  UInt uiSymbol, uiMode = 0;
  PartSize eMode;

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;
  }
  else
  {
    if (!pcCU->getSlice()->getSPS()->getUseSHV())
    {
			UInt uiMaxNumBits = 3;
			for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
			{
				xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
				if ( uiSymbol )
				{
					break;
				}
				uiMode++;
			}

			eMode = (PartSize) uiMode;

      if (pcCU->getSlice()->isInterB() && uiMode == 3)
      {
    		xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
    		if (uiSymbol == 0)
    		{
    		  pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
      		xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
      		if (uiSymbol == 0)
      		  eMode = SIZE_2Nx2N;
    		}
      }
    }
    else
    {
			UInt uiMaxNumBits = 3;
			for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
			{
				xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
				if ( uiSymbol )
				{
					break;
				}
				uiMode++;
			}
			if (uiMode >=2)
			{
				uiMode++;
			}

			// for 2NxN or Nx2N mode
			if (uiMode == 1)
			{
				xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
				uiMode += uiSymbol;
			}

			eMode = (PartSize) uiMode;

			// for SHV mode
			if(uiMode == 4)
			{
				xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
				uiMode += uiSymbol;
				xReadSymbol( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 5) );
				uiMode += uiSymbol*2;
        eMode = (PartSize) (uiMode+4);
			}
    }

    if (pcCU->getSlice()->getSPS()->getUseAMP() && pcCU->getSlice()->getFMPAccuracy( uiDepth ))
    {
      if (eMode == SIZE_2NxN)
      {
        xReadSymbol(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          xReadSymbol(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
					eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
        xReadSymbol(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          xReadSymbol(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
					eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
        }
      }
    }
  }

  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );

  UInt uiTrLevel = 0;
  if( !pcCU->getSlice()->getSPS()->getUseLOT() && (pcCU->getWidth(uiAbsPartIdx) > pcCU->getSlice()->getSPS()->getMaxTrSize()) )
  {
    while( pcCU->getWidth(uiAbsPartIdx) > (pcCU->getSlice()->getSPS()->getMaxTrSize()<<uiTrLevel) ) uiTrLevel++;
  }

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

Void TDecSbac::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return;
  }
  UInt uiSymbol;
  Int  iPredMode = 0;

  xReadSymbol( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
  iPredMode = uiSymbol;

  if (pcCU->getSlice()->isInterB() && !pcCU->getSlice()->getSPS()->getUseSHV())
  {
    pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
    return;
  }

  if ( uiSymbol == 1 )
  {
    xReadSymbol( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1) );
    iPredMode += uiSymbol;
  }

  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseIntraDirLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;

  // TM_INTRA
  if (pcCU->getSlice()->getSPS()->getUseTMI())
  {
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2) );
    if (uiSymbol == 1)
    {pcCU->setLumaIntraDirSubParts( 33, uiAbsPartIdx, uiDepth ); return;}
  }

  xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );

  if ( !uiSymbol )
  {
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
    uiIPredMode  = uiSymbol;
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
    uiIPredMode |= uiSymbol << 1;

		// note: only 4 directions are allowed if PU size >= 16
		Int iPartWidth = pcCU->getWidth( uiAbsPartIdx );
		if ( pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_NxN ) iPartWidth >>= 1;
		if ( iPartWidth <= 8 )
		{
			xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
	    uiIPredMode |= uiSymbol << 2;
		}

    uiIPredMode  = pcCU->revertIntraDirLuma( uiAbsPartIdx, uiIPredMode );
  }
  else
  {
    uiIPredMode  = pcCU->revertIntraDirLuma( uiAbsPartIdx, -1 );
  }
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );

  return;
}

Void TDecSbac::parseIntraDirLumaAdi  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);
// TM_INTRA
  if (pcCU->getSlice()->getSPS()->getUseTMI())
  {
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2) );
  	if (uiSymbol == 1)
    {
      pcCU->setLumaIntraDirSubParts( 33, uiAbsPartIdx, uiDepth );
      return;
    }
  }

	xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );

  if ( !uiSymbol )
  {
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
    uiIPredMode  = uiSymbol;
    xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
    uiIPredMode |= uiSymbol << 1;
    if (g_aucIntraModeBits[iIntraIdx]>=4)
    {
      xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
      uiIPredMode |= uiSymbol << 2;
      if (g_aucIntraModeBits[iIntraIdx]>=5)
      {
        xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
        uiIPredMode |= uiSymbol << 3;
        if (g_aucIntraModeBits[iIntraIdx]>=6)
        {
           xReadSymbol( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
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


Void TDecSbac::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  if ( pcCU->getSlice()->getSPS()->getUseCCP() )
  {
    UInt uiCCCPIdx;

    if (pcCU->getSlice()->getSPS()->getUseADI())
    {
      uiCCCPIdx = CCCP_ADI_MODE;
    }
    else
    {
      uiCCCPIdx = CCCP_MODE;
    }

    xReadSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );

    if ( uiSymbol ==0 )
    {
      pcCU->setChromIntraDirSubParts( uiCCCPIdx, uiAbsPartIdx, uiDepth );
      return;
    }
    else
    {
      if( pcCU->getSlice()->getSPS()->getUseADI() )
      {
        xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
      }
      else
      {
        xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 2 );
      }

      if (uiSymbol>=uiCCCPIdx)
        uiSymbol++;
    }
  }
  else
  {
    xReadSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );

    if ( uiSymbol )
    {
      if( pcCU->getSlice()->getSPS()->getUseADI() )
      {
        xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
      }
      else
      {
        xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 2 );
      }
      uiSymbol++;
    }
  }

  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );

  return;
}

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );

  xReadSymbol( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );

  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
  else
  {
    xReadSymbol( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecSbac::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );

  xReadSymbol ( uiSymbol, m_cCURefPicSCModel.get( 0, 0, uiCtx ) );
  if ( uiSymbol )
  {
		xReadUnaryMaxSymbol( uiSymbol, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );

    uiSymbol++;
  }
  riRefFrmIdx = uiSymbol;
  return;
}

Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
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
  Int iHorD = 0, iVerD = 0;
  if( pcCU->getSlice()->getUseHAM() )
  {
    if( pcCU->getHAMUsed(uiAbsPartIdx) )
    {
#if HAM_ZEROMV_REP
      xReadMvDN( iHorD, iVerD );
#else
      xReadMvDd( iHorD, 0, 0 );
      xReadMvDd( iVerD, 0, 1 );
#endif
    }
  }

  TComMv cTmpMv(0,0,iHorD, iVerD);
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  // MVAC
  if ( pcCU->getSlice()->getUseMVAC() )
  {
	UInt	uiHorPred;
	UInt	uiVerPred;

	Int	iSignHorPred;
	Int iSignVerPred;

	uiHorPred	= (UInt)abs(iHorPred);
	uiVerPred	=	(UInt)abs(iVerPred);

	iSignHorPred = iHorPred >= 0 ? 1 : -1;
	iSignVerPred = iVerPred >= 0 ? 1 : -1;

	uiHorPred >>=	1; // half-pel
	uiVerPred >>=	1; // half-pel

	iHorPred = uiHorPred * iSignHorPred;
	iVerPred = uiVerPred * iSignVerPred;
  }
  //-- MVAC

  xReadMvd( iHor, iHorPred, 0 );
  xReadMvd( iVer, iVerPred, 5 );

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
  //-- MVAC

  TComMv cMv( iHor, iVer );

	pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  return;
}

Void TDecSbac::parseTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiTrLevel = 0;
  if( !pcCU->getSlice()->getSPS()->getUseLOT() && (pcCU->getWidth(uiAbsPartIdx) > pcCU->getSlice()->getSPS()->getMaxTrSize()) )
  {
    while( pcCU->getWidth(uiAbsPartIdx) > (pcCU->getSlice()->getSPS()->getMaxTrSize()<<uiTrLevel) ) uiTrLevel++;
  }

  UInt uiMinTrDepth = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel;
  UInt uiMaxTrDepth = pcCU->getSlice()->getSPS()->getMaxTrDepth() + uiTrLevel;

  if ( uiMinTrDepth == uiMaxTrDepth )
  {
    pcCU->setTrIdxSubParts( uiMinTrDepth, uiAbsPartIdx, uiDepth );
    return;
  }

  UInt uiTrIdx;
  xReadSymbol( uiTrIdx, m_cCUTransIdxSCModel.get( 0, 0, pcCU->getCtxTransIdx( uiAbsPartIdx ) ) );

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
    xReadSymbol( uiSymbol, m_cCUTransIdxSCModel.get( 0, 0, 3 ) );
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

  return;
}

Void TDecSbac::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	UInt uiDQp;
	Int  iDQp;

	xReadSymbol( uiDQp, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );

	if ( uiDQp == 0 )
	{
		uiDQp = pcCU->getSlice()->getSliceQp();
	}
	else
	{
    xReadUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
    iDQp = ( uiDQp + 2 ) / 2;

    if ( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
		uiDQp = pcCU->getSlice()->getSliceQp() + iDQp;
	}

	pcCU->setQPSubParts( uiDQp, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  UInt uiCtx = pcCU->getCtxCbf( uiAbsPartIdx, eType, uiTrDepth );

  xReadSymbol( uiSymbol , m_cCUCbfSCModel.get( 0, eType == TEXT_LUMA ? 0 : 1, 3 - uiCtx ) );
  pcCU->setCbfSubParts( uiCbf | ( uiSymbol << uiTrDepth ), eType, uiAbsPartIdx, uiDepth );

  if( !uiSymbol )
  {
    m_uiLastDQpNonZero = 0;
  }

  return;
}

Void TDecSbac::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{

  UInt uiCTXIdx;

  switch(uiWidth)
  {
  case  2: uiCTXIdx = 6; break;
  case  4: uiCTXIdx = 5; break;
  case  8: uiCTXIdx = 4; break;
  case 16: uiCTXIdx = 3; break;
  case 32: uiCTXIdx = 2; break;
  case 64: uiCTXIdx = 1; break;
  default: uiCTXIdx = 0; break;
  }

  if( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;

	// pre-compute shift from size
	UInt uiShift  = ( g_aucConvertToBit[ uiWidth ] + 2 ) + ( g_aucConvertToBit[ uiHeight ] + 2 );

  // point to coefficient
  TCoeff* piCoeff = pcCoef;
  UInt ui;
  UInt uiCtxSize  = 64;
	const Int* pos2ctx_map  = pos2ctx_map8x8;
  const Int* pos2ctx_last = pos2ctx_last8x8;

  if ( uiWidth < 8 )
  {
    pos2ctx_map  = pos2ctx_nomap;
    pos2ctx_last = pos2ctx_nomap;
    uiCtxSize    = 16;
  }

  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );

  // ACS
  Bool bApplyScan = eTType == TEXT_LUMA;
  UInt uiSig, uiLast, uiCtx, uiCtxOffst;
  UInt uiScanIdx = SCAN_ZIGZAG;
  uiLast = 0;

  xReadSymbol( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ 0 ] ) );

  piCoeff[0] = uiSig;

  if( uiSig )
    xReadSymbol( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ 0 ] ) );

  if( pcCU->getSlice()->getSPS()->getUseACS() && bApplyScan && !uiLast )
  {
    xReadSymbol( uiScanIdx, m_cScanSCModel.get( uiCTXIdx, eTType, 0 ) );
    if( uiScanIdx > 0 )
    {
      UInt uiSymbol;
      xReadSymbol( uiSymbol, m_cScanSCModel.get( uiCTXIdx, eTType, 1 ) );
      uiScanIdx += uiSymbol;
    }
  }
  //--

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
  case  8: pucScan = g_auiFrameScanXY[ uiScanIdx ][ uiConvBit ]; pucScanX = g_auiFrameScanX[ uiScanIdx ][uiConvBit]; pucScanY = g_auiFrameScanY[ uiScanIdx ][uiConvBit]; break;
  case 16:
  case 32:
  case 64: pucScan = g_auiFrameScanXY[ uiScanIdx ][ uiConvBit ]; pucScanX = g_auiFrameScanX[ uiScanIdx ][uiConvBit]; pucScanY = g_auiFrameScanY[ uiScanIdx ][uiConvBit]; break;
  default: pucScan = g_auiFrameScanXY[ uiScanIdx ][ uiConvBit ]; pucScanX = g_auiFrameScanX[ uiScanIdx ][uiConvBit]; pucScanY = g_auiFrameScanY[ uiScanIdx ][uiConvBit]; break;
  }

	//----- parse significance map -----
  // ACS: DC is decoded in the beginning (Vadim)
	Int		uiScanXShift = g_aucConvertToBit[ uiWidth  >> 3 ] + 2;
	Int		uiScanYShift = g_aucConvertToBit[ uiHeight >> 3 ] + 2;
  UInt	uiXX, uiYY;

	SbacContextModel* pMapCCModel  = m_cCUMapSCModel.get ( uiCTXIdx, eTType );
	SbacContextModel* pLastCCModel = m_cCULastSCModel.get( uiCTXIdx, eTType );

	for ( ui=1, uiCtxOffst=uiCtxSize ; ui<(uiSize-1) && !uiLast ; ui++, uiCtxOffst+=uiCtxSize ) // if last coeff is reached, it has to be significant
	{
    if (uiCtxSize != uiSize)
    {
      uiXX  = pucScanX[ui] >> uiScanXShift;
      uiYY  = pucScanY[ui] >> uiScanYShift;
      uiCtx = g_auiAntiScan8[ ( uiYY << 3 )+uiXX];
    }
    else
		{
      uiCtx = uiCtxOffst >> uiShift;
		}

    // SBAC_SEP
		xReadSymbol( uiSig, pMapCCModel[ pos2ctx_map[ uiCtx ] ] );

		piCoeff[ pucScan[ui] ] = uiSig;

		if( uiSig )
		{
      uiCtx       = uiCtxOffst >> uiShift;

      // SBAC_SEP
			xReadSymbol( uiLast, pLastCCModel[ pos2ctx_last[ uiCtx ] ] );
			if( uiLast )
			{
				break;
			}
		}
	}

  //--- last coefficient must be significant if no last symbol was received ---
  if ( ui == uiSize - 1 )
  {
    piCoeff[ pucScan[ui] ] = 1;
  }

	SbacContextModel* pOnesCCModel = m_cCUOneSCModel.get( uiCTXIdx, eTType );
	SbacContextModel* pAbsCCModel  = m_cCUAbsSCModel.get( uiCTXIdx, eTType );

  Int   c1 = 1;
  Int   c2 = 0;

  ui++;
  while( (ui--) != 0 )
  {
    Int   iIndex  = pucScan[ui];
    UInt  uiCoeff = piCoeff[ iIndex ];

    if( uiCoeff )
    {
      UInt uiCtx = Min (c1,4);

      // SBAC_SEP
      xReadSymbol( uiCoeff, pOnesCCModel[ uiCtx ] );
      if( 1 == uiCoeff )
      {
        uiCtx = Min (c2,4);
        // SBAC_SEP
				xReadExGolombLevel( uiCoeff, pAbsCCModel[ uiCtx ] );
        uiCoeff += 2;

        c1=0;
        c2++;
      }
      else if (c1)
      {
        uiCoeff++;
        c1++;
      }
      else
      {
        uiCoeff++;
      }

      UInt uiSign;
      xReadEpSymbol( uiSign );
      piCoeff[iIndex] = ( uiSign ? -(Int)uiCoeff : (Int)uiCoeff );
    }
  }
  return;
}

Void TDecSbac::parseAlfFlag (UInt& ruiVal)
{
	UInt uiSymbol;
	xReadSymbol( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );

	ruiVal = uiSymbol;
}

Void TDecSbac::parseAlfUvlc (UInt& ruiVal)
{
	UInt uiCode;
	Int  i;

	xReadSymbol( uiCode, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
	if ( uiCode == 0 )
	{
		ruiVal = 0;
		return;
	}

	i=1;
	while (1)
	{
		xReadSymbol( uiCode, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
		if ( uiCode == 0 ) break;
		i++;
	}

	ruiVal = i;
}

Void TDecSbac::parseAlfSvlc (Int&  riVal)
{
	UInt uiCode;
	Int  iSign;
	Int  i;

	xReadSymbol( uiCode, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

	if ( uiCode == 0 )
	{
		riVal = 0;
		return;
	}

	// read sign
	xReadSymbol( uiCode, m_cALFSvlcSCModel.get( 0, 0, 1 ) );

	if ( uiCode == 0 ) iSign =  1;
	else               iSign = -1;

	// read magnitude
	i=1;
	while (1)
	{
		xReadSymbol( uiCode, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
		if ( uiCode == 0 ) break;
		i++;
	}

	riVal = i*iSign;
}

Void TDecSbac::parseULTUsedFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol;
  xReadSymbol(uiSymbol, m_cCUULTUseSCModel.get(0,0,0));

  pcCU->setHAMUsedSubParts(uiSymbol, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
}
