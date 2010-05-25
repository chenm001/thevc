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

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecSbac::TDecSbac() :
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
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

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
  m_cMVPIdxSCModel.initBuffer					(	eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cCUROTindexSCModel.initBuffer			( eSliceType, iQp, (Short*)INIT_ROT_IDX );
  m_cCUCIPflagCCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CIP_IDX );

	m_cALFFlagSCModel.initBuffer			  ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
	m_cALFUvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
	m_cALFSvlcSCModel.initBuffer				( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
  m_cCUTransIdxSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_TRANS_IDX );

  m_uiRange           = 0x10000 - 1;
  m_uiValue           = 0;
  m_uiWord            = 0;
  m_uiBitsLeft        = 0;

  // new structure
  m_uiLastQp          = iQp;
}

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{
  xReadTerminatingBit(ruiBit);
}

Void TDecSbac::parseCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  iCtx = pcCU->getCtxCIPFlag( uiAbsPartIdx );

  xReadSymbol( uiSymbol, m_cCUCIPflagCCModel.get( 0, 0, iCtx ) );
  pcCU->setCIPflagSubParts( (UChar)uiSymbol, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt indexROT = 0;
  Int	 dictSize	= ROT_DICT;

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

    if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
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

  if (pcCU->getSlice()->isInterB() )
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

  xReadSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );

  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
    uiSymbol++;
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

  TComMv cTmpMv( 0, 0 );
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  xReadMvd( iHor, iHorPred, 0 );
  xReadMvd( iVer, iVerPred, 1 );

	// set mvd
  TComMv cMv( iHor, iVer );
	pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );

  return;
}

Void TDecSbac::parseTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
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

  UInt uiSig, uiLast, uiCtx, uiCtxOffst;
  uiLast = 0;

  xReadSymbol( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ 0 ] ) );

  piCoeff[0] = uiSig;

  if( uiSig )
    xReadSymbol( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ 0 ] ) );

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
  case 64: pucScan = g_auiFrameScanXY[ uiConvBit ]; pucScanX = g_auiFrameScanX[uiConvBit]; pucScanY = g_auiFrameScanY[uiConvBit]; break;
  default: pucScan = g_auiFrameScanXY[ uiConvBit ]; pucScanX = g_auiFrameScanX[uiConvBit]; pucScanY = g_auiFrameScanY[uiConvBit]; break;
  }

	//----- parse significance map -----
  // DC is decoded in the beginning
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

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

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
  UInt uiLocalCtx = 0;

  if( uiAbsSum >= 3)
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 2 : 1;
  }

  riMvdComp = 0;

  UInt uiSymbol;
  xReadSymbol( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );

  if (!uiSymbol)
  {
    return;
  }

  xReadExGolombMvd( uiSymbol, &m_cCUMvdSCModel.get( 0, uiCtx, 3 ), 3 );
  uiSymbol++;

  UInt uiSign;
  xReadEpSymbol( uiSign );

  riMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;

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
