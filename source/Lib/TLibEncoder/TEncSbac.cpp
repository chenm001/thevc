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

/** \file			TEncSbac.cpp
    \brief		SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"

#define MAX_VALUE           0xdead

extern UChar stateMappingTable[113];
extern Int entropyBits[128];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TEncSbac::TEncSbac():
  // new structure here
    m_cCUSplitFlagSCModel   ( 1,             1,               NUM_SPLIT_FLAG_CTX    )
  , m_cCUSkipFlagSCModel    ( 1,             1,               NUM_SKIP_FLAG_CTX     )
  , m_cCUAlfCtrlFlagSCModel ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX )
  , m_cCUPartSizeSCModel    ( 1,						 1,								NUM_PART_SIZE_CTX     )
  , m_cCUXPosiSCModel       ( 1,						 1,								NUM_CU_X_POS_CTX      )
  , m_cCUYPosiSCModel       ( 1,						 1,								NUM_CU_Y_POS_CTX      )
  , m_cCUPredModeSCModel    ( 1,             1,								NUM_PRED_MODE_CTX     )
  , m_cCUIntraPredSCModel   ( 1,             1,               NUM_ADI_CTX           )
  , m_cCUChromaPredSCModel  ( 1,             1,               NUM_CHROMA_PRED_CTX   )
  , m_cCUInterDirSCModel    ( 1,             1,               NUM_INTER_DIR_CTX     )
  , m_cCUMvdSCModel         ( 1,             2,               NUM_MV_RES_CTX        )
  , m_cCURefPicSCModel      ( 1,             1,               NUM_REF_NO_CTX        )
  , m_cCUTransIdxSCModel    ( 1,             1,               NUM_TRANS_IDX_CTX     )
  , m_cCUDeltaQpSCModel     ( 1,             1,               NUM_DELTA_QP_CTX      )
  , m_cCUCbfSCModel         ( 1,             2,               NUM_CBF_CTX           )
                            //Depth
  , m_cCUMapSCModel         ( MAX_CU_DEPTH,  2,               NUM_MAP_CTX           )
  , m_cCULastSCModel        ( MAX_CU_DEPTH,  2,               NUM_LAST_CTX          )
  , m_cCUOneSCModel         ( MAX_CU_DEPTH,  2,               NUM_ONE_CTX           )
  , m_cCUAbsSCModel         ( MAX_CU_DEPTH,  2,               NUM_ABS_CTX           )
  , m_cMVPIdxSCModel        ( 1,						 1,								NUM_MVP_IDX_CTX       )
  , m_cCUROTindexSCModel	  ( 1,             1,								NUM_ROT_IDX_CTX       )
  , m_cCUCIPflagCCModel		  ( 1,             1,								NUM_CIP_FLAG_CTX      )
  , m_cALFFlagSCModel			  ( 1,             1,               NUM_ALF_FLAG_CTX      )
	, m_cALFUvlcSCModel			  ( 1,             1,               NUM_ALF_UVLC_CTX      )
	, m_cALFSvlcSCModel			  ( 1,             1,               NUM_ALF_SVLC_CTX      )
{
  m_uiRange = 0x10000 - 1;
  m_uiCode = 0;
  m_uiCodeBits = 11;
  m_ucPendingByte = 0;
  m_bIsPendingByte = false;
  m_uiStackedFFs = 0;
  m_uiStackedZeros = 0;

  m_uiCoeffCost = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TEncSbac::~TEncSbac()
{
}

Void TEncSbac::resetEntropy           ()
{
  Int  iQp              = m_pcSlice->getSliceQp();
  SliceType eSliceType  = m_pcSlice->getSliceType();

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
  m_cMVPIdxSCModel.initBuffer					(	eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cCUROTindexSCModel.initBuffer			( eSliceType, iQp, (Short*)INIT_ROT_IDX );
  m_cCUCIPflagCCModel.initBuffer			( eSliceType, iQp, (Short*)INIT_CIP_IDX );

	m_cALFFlagSCModel.initBuffer			  ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
	m_cALFUvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
	m_cALFSvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
  m_cCUTransIdxSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_TRANS_IDX );

  m_uiRange           = 0x10000 - 1;
  m_uiCode = 0;
  m_uiCodeBits = 11;
  m_ucPendingByte = 0;
  m_bIsPendingByte = false;
  m_uiStackedFFs = 0;
  m_uiStackedZeros = 0;

  m_uiLastDQpNonZero  = 0;

  // new structure
  m_uiLastQp          = iQp;

  return;
}

UInt TEncSbac::xGetCTXIdxFromWidth( Int iWidth )
{
	UInt uiCTXIdx;

  switch( iWidth )
  {
  case  2: uiCTXIdx = 6; break;
  case  4: uiCTXIdx = 5; break;
  case  8: uiCTXIdx = 4; break;
  case 16: uiCTXIdx = 3; break;
  case 32: uiCTXIdx = 2; break;
  case 64: uiCTXIdx = 1; break;
  default: uiCTXIdx = 0; break;
  }

	return uiCTXIdx;
}

Void TEncSbac::codeSPS( TComSPS* pcSPS )
{
  assert (0);
  return;
}

Void TEncSbac::codeSliceHeader( TComSlice* pcSlice )
{
  assert (0);
  return;
}

Void TEncSbac::codeTerminatingBit( UInt uilsLast )
{
  xWriteTerminatingBit( uilsLast );
}

Void TEncSbac::codeSliceFinish()
{
  UInt uiTemp = (m_uiCode + m_uiRange - 1) & 0xFFFF0000;

  if (uiTemp < m_uiCode) {
    uiTemp += 0x8000;
  }

  m_uiCode = uiTemp << m_uiCodeBits;
  xCarryPropagate(m_uiCode);

  m_uiCode <<= 8;
  xCarryPropagate(m_uiCode);

  xPutByte(0x00);
  while (m_uiStackedZeros > 0) {
    m_pcBitIf->write(0x00, 8);
    m_uiStackedZeros--;
  }
}

__inline Void TEncSbac::xPutByte( UChar ucByte)
{
  if (m_bIsPendingByte) {
    if (m_ucPendingByte == 0) {
      m_uiStackedZeros++;
    }
    else {
      while (m_uiStackedZeros > 0) {
        m_pcBitIf->write(0x00, 8);
        m_uiStackedZeros--;
      }
      m_pcBitIf->write(m_ucPendingByte, 8);
    }
  }
  m_ucPendingByte = ucByte;
  m_bIsPendingByte = true;
}

__inline Void TEncSbac::xCarryPropagate( UInt& ruiCode )
{
  UInt uiOutBits = ruiCode >> 19;
  ruiCode &= 0x7FFFF;

  if (uiOutBits < 0xFF) {
    while (m_uiStackedFFs !=0 ) {
      xPutByte(0xFF);
      m_uiStackedFFs--;
    }
    xPutByte(uiOutBits);
  }
  else if (uiOutBits > 0xFF) {
    m_ucPendingByte++;
    while (m_uiStackedFFs !=0 ) {
      xPutByte(0x00);
      m_uiStackedFFs--;
    }
    xPutByte(uiOutBits & 0xFF);
  }
  else {
    m_uiStackedFFs++;
  }
}

Void TEncSbac::xWriteSymbol( UInt uiSymbol, SbacContextModel& rcSCModel )
{
	UInt uiCode    = m_uiCode;
	UInt uiRange  = m_uiRange;
  UInt uiLPS;
  UChar ucNextStateLPS, ucNextStateMPS;

  uiLPS = g_auiStandardProbFsm[rcSCModel.getState()];
  ucNextStateLPS = uiLPS & 0xFF;
  uiLPS >>= 8;
  ucNextStateMPS = uiLPS & 0xFF;
  uiLPS >>= 8;

  UInt uiRangeIdx = (uiRange>>13)&3;

  uiLPS += ( (uiLPS >> (g_auiShiftParameters[uiRangeIdx][0])) + (uiLPS >> (g_auiShiftParameters[uiRangeIdx][1])) );
  uiRange -= uiLPS;

  if( uiSymbol != rcSCModel.getMps() ) {
    if ( uiRange >= uiLPS) {
      uiCode += uiRange;
      uiRange = uiLPS;
    }
    rcSCModel.setStateMps(ucNextStateLPS);
  }
  else {
    if ( uiRange >= 0x8000 ) {
      m_uiRange = uiRange;
      m_uiCode = uiCode;
      return;
    }
    if ( uiRange < uiLPS ) {
      uiCode += uiRange;
      uiRange = uiLPS;
    }
    rcSCModel.setStateMps(ucNextStateMPS);
  }
  do {
    uiRange <<= 1;
    uiCode <<= 1;
    if (--m_uiCodeBits == 0) {
      xCarryPropagate( uiCode );
      m_uiCodeBits = 8;
    }
  } while (uiRange < 0x8000);

  m_uiRange = uiRange;
  m_uiCode = uiCode;

  return;
}

Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, SbacContextModel* pcSCModel, Int iOffset )
{
  xWriteSymbol( uiSymbol ? 1 : 0, pcSCModel[0] );

  if( 0 == uiSymbol)
  {
    return;
  }

  while( uiSymbol-- )
  {
    xWriteSymbol( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }

  return;
}

Void TEncSbac::xWriteEPSymbol( UInt uiSymbol )
{
	UInt uiCode    = m_uiCode;
	UInt uiRange   = m_uiRange >> 1;

  if( uiSymbol != 0 ) {
    uiCode += uiRange;
  }

  uiRange <<= 1;
  uiCode <<= 1;
  if (--m_uiCodeBits == 0) {
    xCarryPropagate( uiCode );
    m_uiCodeBits = 8;
  }

  m_uiRange = uiRange;
  m_uiCode = uiCode;

  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, SbacContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  xWriteSymbol( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );

  if ( uiSymbol == 0 )
  {
    return;
  }

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    xWriteSymbol( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    xWriteSymbol( 0, pcSCModel[ iOffset ] );
  }

  return;
}

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xWriteEPSymbol( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteEPSymbol( 0 );
  while( uiCount-- )
  {
    xWriteEPSymbol( (uiSymbol>>uiCount) & 1 );
  }

  return;
}

Void TEncSbac::xWriteExGolombLevel( UInt uiSymbol, SbacContextModel& rcSCModel  )
{
  if( uiSymbol )
  {
    xWriteSymbol( 1, rcSCModel );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);

    while( --uiSymbol && ++uiCount < 13 )
    {
      xWriteSymbol( 1, rcSCModel );
    }
    if( bNoExGo )
    {
      xWriteSymbol( 0, rcSCModel );
    }
    else
    {
      xWriteEpExGolomb( uiSymbol, 0 );
    }
  }
  else
  {
    xWriteSymbol( 0, rcSCModel );
  }

  return;
}

Void TEncSbac::xWriteTerminatingBit( UInt uiSymbol )
{
	UInt uiCode    = m_uiCode;
	UInt uiRange   = m_uiRange - 1;

  if( uiSymbol ) {
    uiCode += uiRange;
    uiRange = 1;
  }
  else {
    if ( uiRange >= 0x8000 ) {
      m_uiRange = uiRange;
      m_uiCode = uiCode;
      return;
    }
  }
  do {
    uiRange <<= 1;
    uiCode <<= 1;
    if (--m_uiCodeBits == 0) {
      xCarryPropagate( uiCode );
      m_uiCodeBits = 8;
    }
  } while (uiRange < 0x8000);

  m_uiRange = uiRange;
  m_uiCode = uiCode;

  return;
}

// SBAC RD
Void  TEncSbac::load ( TEncSbac* pScr)
{
  this->xCopyFrom(pScr);
}

Void  TEncSbac::store( TEncSbac* pDest)
{
  pDest->xCopyFrom( this );
}

Void TEncSbac::xCopyFrom( TEncSbac* pSrc )
{
  this->m_uiRange            = pSrc->m_uiRange;
  this->m_uiCode             = pSrc->m_uiCode;
  this->m_uiCodeBits         = pSrc->m_uiCodeBits;
  this->m_ucPendingByte      = pSrc->m_ucPendingByte;
  this->m_bIsPendingByte     = pSrc->m_bIsPendingByte;
  this->m_uiStackedFFs       = pSrc->m_uiStackedFFs;
  this->m_uiStackedZeros     = pSrc->m_uiStackedZeros;

  this->m_uiCoeffCost        = pSrc->m_uiCoeffCost;
  this->m_uiLastDQpNonZero   = pSrc->m_uiLastDQpNonZero;
  this->m_uiLastQp           = pSrc->m_uiLastQp;

  this->m_cCUSplitFlagSCModel .copyFrom( &pSrc->m_cCUSplitFlagSCModel   );
  this->m_cCUSkipFlagSCModel  .copyFrom( &pSrc->m_cCUSkipFlagSCModel    );
  this->m_cCUPartSizeSCModel  .copyFrom( &pSrc->m_cCUPartSizeSCModel    );
  this->m_cCUPredModeSCModel  .copyFrom( &pSrc->m_cCUPredModeSCModel    );
  this->m_cCUIntraPredSCModel .copyFrom( &pSrc->m_cCUIntraPredSCModel   );
  this->m_cCUChromaPredSCModel.copyFrom( &pSrc->m_cCUChromaPredSCModel  );
  this->m_cCUDeltaQpSCModel   .copyFrom( &pSrc->m_cCUDeltaQpSCModel     );
  this->m_cCUInterDirSCModel  .copyFrom( &pSrc->m_cCUInterDirSCModel    );
  this->m_cCURefPicSCModel    .copyFrom( &pSrc->m_cCURefPicSCModel      );
  this->m_cCUMvdSCModel       .copyFrom( &pSrc->m_cCUMvdSCModel         );
  this->m_cCUCbfSCModel       .copyFrom( &pSrc->m_cCUCbfSCModel         );
  this->m_cCUTransIdxSCModel  .copyFrom( &pSrc->m_cCUTransIdxSCModel    );

  this->m_cCUMapSCModel       .copyFrom( &pSrc->m_cCUMapSCModel         );
  this->m_cCULastSCModel      .copyFrom( &pSrc->m_cCULastSCModel        );
  this->m_cCUOneSCModel       .copyFrom( &pSrc->m_cCUOneSCModel         );
  this->m_cCUAbsSCModel       .copyFrom( &pSrc->m_cCUAbsSCModel         );

  this->m_cMVPIdxSCModel      .copyFrom( &pSrc->m_cMVPIdxSCModel        );

  this->m_cCUROTindexSCModel	.copyFrom( &pSrc->m_cCUROTindexSCModel		);
  this->m_cCUCIPflagCCModel		.copyFrom( &pSrc->m_cCUCIPflagCCModel			);
  this->m_cCUXPosiSCModel			.copyFrom( &pSrc->m_cCUXPosiSCModel				);
  this->m_cCUYPosiSCModel			.copyFrom( &pSrc->m_cCUXPosiSCModel				);
}

// CIP
Void TEncSbac::codeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int CIPflag = pcCU->getCIPflag   ( uiAbsPartIdx );
  Int iCtx    = pcCU->getCtxCIPFlag( uiAbsPartIdx );

  xWriteSymbol ( (CIPflag) ? 1 : 0, m_cCUCIPflagCCModel.get( 0, 0, iCtx ) );
}

Void TEncSbac::codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int indexROT = pcCU->getROTindex( uiAbsPartIdx );
  Int dictSize;

  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
    dictSize = ROT_DICT;
  else
    dictSize = ROT_DICT_INTER;
  switch (dictSize)
  {
	 case 9:
    {
      xWriteSymbol( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteSymbol( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        xWriteSymbol( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
		xWriteSymbol( (indexROT & 0x04) >> 2, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
      }
    }
    break;
  case 4:
    {
      xWriteSymbol( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      xWriteSymbol( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 1 ) );
    }
    break;
  case 2:
    {
      xWriteSymbol( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
    }
    break;
  case 5:
    {
      xWriteSymbol( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteSymbol( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        xWriteSymbol( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
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

Void TEncSbac::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum    = pcCU->getMVPNum(eRefList, uiAbsPartIdx);

  xWriteUnaryMaxSymbol(iSymbol, m_cMVPIdxSCModel.get(0), 1, iNum-1);
}

Void TEncSbac::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );

  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
    xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
    xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 3) );
    xWriteSymbol( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 4) );
    return;
  }

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteSymbol( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
    return;
  }

	switch(eSize)
	{
	case SIZE_2Nx2N:
		{
			xWriteSymbol( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			break;
		}
	case SIZE_2NxN:
	case SIZE_2NxnU:
	case SIZE_2NxnD:
		{
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xWriteSymbol( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );

			if ( pcCU->getSlice()->getAMPAcc( uiDepth ) )
			{
			  if (eSize == SIZE_2NxN)
		  	{
          xWriteSymbol(1, m_cCUYPosiSCModel.get( 0, 0, 0 ));
		  	}
			  else
		  	{
          xWriteSymbol(0, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          xWriteSymbol((eSize == SIZE_2NxnU? 0: 1), m_cCUYPosiSCModel.get( 0, 0, 1 ));
		  	}
			}
			break;
		}
	case SIZE_Nx2N:
	case SIZE_nLx2N:
	case SIZE_nRx2N:
		{
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
			xWriteSymbol( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );

			if ( pcCU->getSlice()->getAMPAcc( uiDepth ) )
			{
			  if (eSize == SIZE_Nx2N)
		  	{
          xWriteSymbol(1, m_cCUXPosiSCModel.get( 0, 0, 0 ));
		  	}
			  else
		  	{
          xWriteSymbol(0, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          xWriteSymbol((eSize == SIZE_nLx2N? 0: 1), m_cCUXPosiSCModel.get( 0, 0, 1 ));
		  	}
			}
			break;
		}
	case SIZE_NxN:
		{
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
			xWriteSymbol( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );

      if (pcCU->getSlice()->isInterB())
      {
  			xWriteSymbol( 1, m_cCUPartSizeSCModel.get( 0, 0, 3) );
			}
			break;
		}
	default:
		{
			assert(0);
		}
	}
}

Void TEncSbac::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );
  UInt uiSymbol = iPredMode == 0 ? 0 : 1;
  xWriteSymbol( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );

  if (pcCU->getSlice()->isInterB() )
  {
    return;
  }

  if ( uiSymbol == 1 )
  {
    xWriteSymbol( iPredMode == 1 ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
  }
}

Void TEncSbac::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
    return;

  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;

  xWriteSymbol( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx) ) );
}

Void TEncSbac::codeAlfCtrlDepth()
{
  if (!m_bAlfCtrl)
    return;

  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  xWriteUnaryMaxSymbol(uiDepth, m_cALFUvlcSCModel.get(0), 1, g_uiMaxCUDepth-1);
}

Void TEncSbac::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  xWriteSymbol( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, pcCU->getCtxSkipFlag( uiAbsPartIdx) ) );
}

Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;

  UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;

  assert( uiCtx < 3 );
  xWriteSymbol( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiCtx ) );
  return;
}

Void TEncSbac::codeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
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

  xWriteSymbol( uiSymbol ? 1 : 0, m_cCUTransIdxSCModel.get( 0, 0, pcCU->getCtxTransIdx( uiAbsPartIdx ) ) );

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
    xWriteSymbol( uiSymbol ? 1 : 0, m_cCUTransIdxSCModel.get( 0, 0, 3 ) );
    if ( uiSymbol == 0 )
    {
      return;
    }
    uiSymbol--;
  }

  return;
}

Void TEncSbac::codeIntraDirLuma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
	Int iIntraDirLuma = pcCU->convertIntraDirLuma( uiAbsPartIdx );

	xWriteSymbol( iIntraDirLuma >= 0 ? 0 : 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );

	if ( iIntraDirLuma >= 0 )
	{
		xWriteSymbol( (iIntraDirLuma & 0x01),      m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
		xWriteSymbol( (iIntraDirLuma & 0x02) >> 1, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );

		// note: only 4 directions are allowed if PU size >= 16
		Int iPartWidth = pcCU->getWidth( uiAbsPartIdx );
		if ( pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_NxN ) iPartWidth >>= 1;
		if ( iPartWidth <= 8 )
		{
			xWriteSymbol( (iIntraDirLuma & 0x04) >> 2, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
		}
	}
	return;
}


Void TEncSbac::codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
	Int iIntraDirLuma = pcCU->convertIntraDirLumaAdi( pcCU, uiAbsPartIdx );
	Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);

	xWriteSymbol( iIntraDirLuma >= 0 ? 0 : 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );

	if (iIntraDirLuma >= 0)
	{
		xWriteSymbol((iIntraDirLuma & 0x01), m_cCUIntraPredSCModel.get(0, 0, 1));

		xWriteSymbol((iIntraDirLuma & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));

		if (g_aucIntraModeBits[iIntraIdx] >= 4)
		{
			xWriteSymbol((iIntraDirLuma & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));

			if (g_aucIntraModeBits[iIntraIdx] >= 5)
			{
				xWriteSymbol((iIntraDirLuma & 0x08) >> 3,
					m_cCUIntraPredSCModel.get(0, 0, 1));

				if (g_aucIntraModeBits[iIntraIdx] >= 6)
				{
					xWriteSymbol((iIntraDirLuma & 0x10) >> 4,
						m_cCUIntraPredSCModel.get(0, 0, 1));
				}
			}
		}
	}
	return;
}

Void TEncSbac::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
	UInt uiCtx            = pcCU->getCtxIntraDirChroma( uiAbsPartIdx );
	UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );

	if ( 0 == uiIntraDirChroma )
	{
		xWriteSymbol( 0, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
	}
	else
	{
		xWriteSymbol( 1, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
		xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
	}

	return;
}

Void TEncSbac::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  UInt uiCtx      = pcCU->getCtxInterDir( uiAbsPartIdx );
  uiInterDir--;
  xWriteSymbol( ( uiInterDir == 2 ? 1 : 0 ), m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );

  if ( uiInterDir < 2 )
  {
    xWriteSymbol( uiInterDir, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }

  return;
}

Void TEncSbac::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );

  UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );

  xWriteSymbol( ( iRefFrame == 0 ? 0 : 1 ), m_cCURefPicSCModel.get( 0, 0, uiCtx ) );

  if ( iRefFrame > 0 )
  {
		xWriteUnaryMaxSymbol( iRefFrame - 1, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
  }
  return;
}

Void TEncSbac::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
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

  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
			 ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
			 ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );

  xWriteMvd( iHor, iHorPred, 0 );
  xWriteMvd( iVer, iVerPred, 1 );

  return;
}

Void TEncSbac::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getSlice()->getSliceQp();

	if ( iDQp == 0 )
	{
		xWriteSymbol( 0, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
	}
	else
	{
		xWriteSymbol( 1, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );

    UInt uiDQp = (UInt)( iDQp > 0 ? ( 2 * iDQp - 2 ) : ( -2 * iDQp - 1 ) );
    xWriteUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
  }

  return;
}

Void TEncSbac::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf   ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCtx = pcCU->getCtxCbf( uiAbsPartIdx, eType, uiTrDepth );

  xWriteSymbol( uiCbf , m_cCUCbfSCModel.get( 0, eType == TEXT_LUMA ? 0 : 1, 3 - uiCtx ) );

  if( !uiCbf )
  {
    m_uiLastDQpNonZero = 0;
  }

  return;
}

Void TEncSbac::xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart )
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

Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
{
  UInt uiCTXIdx = xGetCTXIdxFromWidth( uiWidth );

  if( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;

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
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }

  if ( uiNumSig == 0 )
    return;

  UInt uiCodedSig = 0;
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

  UInt uiSig, uiCtx, uiLast;
  uiLast = 0;
  uiSig = piCoeff[0] ? 1 : 0;

  xWriteSymbol( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ 0 ] ) );

  if( uiSig )
  {
    uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
    xWriteSymbol( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ 0 ] ) );
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

	//----- encode significance map -----
  // DC is coded in the beginning
  for( ui = 1; ui < ( uiSize - 1 ) && !uiLast; ui++ ) // if last coeff is reached, it has to be significant
	{
		uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;

    if (uiCtxSize != uiSize)
    {
      UInt uiXX, uiYY;
      uiXX = pucScanX[ui]/(uiWidth / 8);
      uiYY = pucScanY[ui]/(uiHeight / 8);

      uiCtx = g_auiAntiScan8[uiYY*8+uiXX];
    }
    else
      uiCtx = ui * uiCtxSize / uiSize;

		xWriteSymbol( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ uiCtx ] ) );

		if( uiSig )
		{
      uiCtx = ui * uiCtxSize / uiSize;
			uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
			xWriteSymbol( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ uiCtx ] ) );

			if( uiLast )
			{
				break;
			}
		}
	}

  Int   c1 = 1;
  Int   c2 = 0;
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

      UInt uiCtx    = Min (c1, 4);
      UInt uiSymbol = uiAbs > 1 ? 1 : 0;
      xWriteSymbol( uiSymbol, m_cCUOneSCModel.get( uiCTXIdx, eTType, uiCtx ) );

      if( uiSymbol )
      {
        uiCtx  = Min (c2,4);
        uiAbs -= 2;
        c1     = 0;
        c2++;
				xWriteExGolombLevel( uiAbs, m_cCUAbsSCModel.get( uiCTXIdx, eTType, uiCtx ) );
      }
      else if( c1 )
      {
        c1++;
      }
      xWriteEPSymbol( uiSign );
    }
  }
  return;
}

Void TEncSbac::xWriteMvd( Int iMvd, UInt uiAbsSum, UInt uiCtx )
{
  UInt uiLocalCtx = 0;
  if ( uiAbsSum >= 3 )
  {
    uiLocalCtx += ( uiAbsSum > 32 ) ? 2 : 1;
  }

  UInt uiSymbol = ( 0 == iMvd ) ? 0 : 1;
  xWriteSymbol( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );
  if ( 0 == uiSymbol )
  {
    return;
  }

  UInt uiSign = 0;
  if ( 0 > iMvd )
  {
    uiSign = 1;
    iMvd   = -iMvd;
  }
  xWriteExGolombMvd( iMvd-1, &m_cCUMvdSCModel.get( 0, uiCtx, 3 ), 3 );
  xWriteEPSymbol( uiSign );

  return;
}

Void  TEncSbac::xWriteExGolombMvd( UInt uiSymbol, SbacContextModel* pcSCModel, UInt uiMaxBin )
{
  if ( ! uiSymbol )
  {
    xWriteSymbol( 0, *pcSCModel );
    return;
  }

  xWriteSymbol( 1, *pcSCModel );

  Bool bNoExGo = ( uiSymbol < 8 );
  UInt uiCount = 1;
  pcSCModel++;

  while ( --uiSymbol && ++uiCount <= 8 )
  {
    xWriteSymbol( 1, *pcSCModel );
    if ( uiCount == 2 )
    {
      pcSCModel++;
    }
    if ( uiCount == uiMaxBin )
    {
      pcSCModel++;
    }
  }

  if ( bNoExGo )
  {
    xWriteSymbol( 0, *pcSCModel );
  }
  else
  {
    xWriteEpExGolomb( uiSymbol, 3 );
  }

  return;
}

Void TEncSbac::codeAlfFlag       ( UInt uiCode )
{
	UInt uiSymbol = ( ( uiCode == 0 ) ? 0 : 1 );
	xWriteSymbol( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
}

Void TEncSbac::codeAlfUvlc       ( UInt uiCode )
{
	Int i;

	if ( uiCode == 0 )
	{
		xWriteSymbol( 0, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
	}
	else
	{
		xWriteSymbol( 1, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
		for ( i=0; i<uiCode-1; i++ )
		{
			xWriteSymbol( 1, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
		}
		xWriteSymbol( 0, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
	}
}

Void TEncSbac::codeAlfSvlc       ( Int iCode )
{
	Int i;

	if ( iCode == 0 )
	{
		xWriteSymbol( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
	}
	else
	{
		xWriteSymbol( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

		// write sign
		if ( iCode > 0 )
		{
			xWriteSymbol( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
		}
		else
		{
			xWriteSymbol( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
			iCode = -iCode;
		}

		// write magnitude
		for ( i=0; i<iCode-1; i++ )
		{
			xWriteSymbol( 1, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
		}
		xWriteSymbol( 0, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
	}
}

/*!
 ****************************************************************************
 * \brief
 *   estimate bit cost for CBP, significant map and significant coefficients
 ****************************************************************************
 */
Void TEncSbac::estBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType)
{
  estCBFBit (pcEstBitsSbac, 0, eTType);

  // encode significance map
  estSignificantMapBit (pcEstBitsSbac, uiCTXIdx, eTType);

  // encode significant coefficients
  estSignificantCoefficientsBit (pcEstBitsSbac, uiCTXIdx, eTType);
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost for each CBP bit
 ****************************************************************************
 */
Void TEncSbac::estCBFBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType)
{
  Int ctx;
  Short cbp_bit;

  for ( ctx = 0; ctx <= 3; ctx++ )
  {
    cbp_bit = 0;
    pcEstBitsSbac->blockCbpBits[ctx][cbp_bit] = biari_no_bits (cbp_bit, m_cCUCbfSCModel.get( uiCTXIdx, eTType, ctx ));

    cbp_bit = 1;
    pcEstBitsSbac->blockCbpBits[ctx][cbp_bit] = biari_no_bits (cbp_bit, m_cCUCbfSCModel.get( uiCTXIdx, eTType, ctx ));
  }
}

/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient map
 ****************************************************************************
 */
Void TEncSbac::estSignificantMapBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType)
{
  Int    k;
  UShort sig, last;
  Int    k1 = 15;

  const Int* pos2ctx_map  = pos2ctx_nomap;
  const Int* pos2ctx_last = pos2ctx_nomap;


  for ( k = 0; k < k1; k++ ) // if last coeff is reached, it has to be significant
  {
    sig = 0;
    pcEstBitsSbac->significantBits[pos2ctx_map[k]][sig] = biari_no_bits (sig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ k ] ));

    sig = 1;
    pcEstBitsSbac->significantBits[pos2ctx_map[k]][sig] = biari_no_bits (sig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ k ] ));

    last = 0;
    pcEstBitsSbac->lastBits[pos2ctx_last[k]][last] = biari_no_bits (last, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ k ] ));

    last = 1;
    pcEstBitsSbac->lastBits[pos2ctx_last[k]][last] = biari_no_bits (last, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ k ] ));
  }

  // if last coeff is reached, it has to be significant
  pcEstBitsSbac->significantBits[pos2ctx_map[k1]][0] = 0;
  pcEstBitsSbac->significantBits[pos2ctx_map[k1]][1] = 0;
  pcEstBitsSbac->lastBits[pos2ctx_last[k1]][0] = 0;
  pcEstBitsSbac->lastBits[pos2ctx_last[k1]][1] = 0;
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoefficientsBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType)
{
  Int   ctx;
  Short greater_one;

  for ( ctx = 0; ctx <= 4; ctx++ )
  {
    greater_one = 0;
    pcEstBitsSbac->greaterOneBits[0][ctx][greater_one] = biari_no_bits (greater_one, m_cCUOneSCModel.get( uiCTXIdx, eTType, ctx ));

    greater_one = 1;
    pcEstBitsSbac->greaterOneBits[0][ctx][greater_one] = biari_no_bits (greater_one, m_cCUOneSCModel.get( uiCTXIdx, eTType, ctx ));
  }

  for ( ctx = 0; ctx <= 4; ctx++ )
  {
    pcEstBitsSbac->greaterOneBits[1][ctx][0] = biari_no_bits(0, m_cCUAbsSCModel.get( uiCTXIdx, eTType, ctx ));

    pcEstBitsSbac->greaterOneState[ctx] = biari_state(0, m_cCUAbsSCModel.get( uiCTXIdx, eTType, ctx ));

    pcEstBitsSbac->greaterOneBits[1][ctx][1] = biari_no_bits(1, m_cCUAbsSCModel.get( uiCTXIdx, eTType, ctx ));
  }
}

Int TEncSbac::biari_no_bits (Short symbol, SbacContextModel& rcSCModel)
{
  Int ctx_state, estBits;

  symbol = (Short) (symbol != 0);

  ctx_state = (symbol == rcSCModel.getMps() ) ? 64 + stateMappingTable[rcSCModel.getState()] : 63 - stateMappingTable[rcSCModel.getState()];

  estBits = entropyBits[127-ctx_state];

  return estBits;
}

Int TEncSbac::biari_state (Short symbol, SbacContextModel& rcSCModel)
{
  Int ctx_state;

  symbol = (Short) (symbol != 0);

  ctx_state = ( symbol == rcSCModel.getMps() ) ? 64 + stateMappingTable[rcSCModel.getState()] : 63 - stateMappingTable[rcSCModel.getState()];

  return ctx_state;
}

