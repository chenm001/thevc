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

/** \file     TEncSbac.cpp
    \brief    SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"

#if HHI_RQT
#include <map>
#endif

extern UChar  stateMappingTable[113];
extern Int entropyBits[128];

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSbac::TEncSbac()
  // new structure here
  : m_cCUSplitFlagSCModel     ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
  , m_cCUSkipFlagSCModel      ( 1,             1,               NUM_SKIP_FLAG_CTX             )
#if HHI_MRG
  , m_cCUMergeFlagSCModel     ( 1,             1,               NUM_MERGE_FLAG_CTX            )
  , m_cCUMergeIndexSCModel    ( 1,             1,               NUM_MERGE_INDEX_CTX           )
#endif
  , m_cCUAlfCtrlFlagSCModel   ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         )
  , m_cCUPartSizeSCModel      ( 1,             1,               NUM_PART_SIZE_CTX             )
  , m_cCUXPosiSCModel         ( 1,             1,               NUM_CU_X_POS_CTX              )
  , m_cCUYPosiSCModel         ( 1,             1,               NUM_CU_Y_POS_CTX              )
  , m_cCUPredModeSCModel      ( 1,             1,               NUM_PRED_MODE_CTX             )
  , m_cCUIntraPredSCModel     ( 1,             1,               NUM_ADI_CTX                   )
#if HHI_AIS
  , m_cCUIntraFiltFlagSCModel ( 1,             1,               NUM_ADI_FILT_CTX              )
#endif
  , m_cCUChromaPredSCModel    ( 1,             1,               NUM_CHROMA_PRED_CTX           )
  , m_cCUInterDirSCModel      ( 1,             1,               NUM_INTER_DIR_CTX             )
  , m_cCUMvdSCModel           ( 1,             2,               NUM_MV_RES_CTX                )
  , m_cCURefPicSCModel        ( 1,             1,               NUM_REF_NO_CTX                )
#ifdef QC_AMVRES
  , m_cCUMvResCModel       ( 1,             1,                 NUM_MV_RES_FALG_CTX )
#endif
#if HHI_RQT
  , m_cCUTransSubdivFlagSCModel( 1,          1,               NUM_TRANS_SUBDIV_FLAG_CTX )
#if HHI_RQT_ROOT
  , m_cCUQtRootCbfSCModel     ( 1,             1,               NUM_QT_ROOT_CBF_CTX   )
#endif
#endif
  , m_cCUTransIdxSCModel      ( 1,             1,               NUM_TRANS_IDX_CTX             )
  , m_cCUDeltaQpSCModel       ( 1,             1,               NUM_DELTA_QP_CTX              )
  , m_cCUCbfSCModel           ( 1,             2,               NUM_CBF_CTX                   )

#if HHI_RQT
  , m_cCUQtCbfSCModel       ( 1,             3,               NUM_QT_CBF_CTX        )
#endif

#if HHI_TRANSFORM_CODING
  , m_cCuCtxModSig            ( MAX_CU_DEPTH,  2,               NUM_SIG_FLAG_CTX              )
  , m_cCuCtxModLast           ( MAX_CU_DEPTH,  2,               NUM_LAST_FLAG_CTX             )
  , m_cCuCtxModAbsGreOne      ( 1,             2,               NUM_ABS_GREATER_ONE_CTX       )
  , m_cCuCtxModCoeffLevelM1   ( 1,             2,               NUM_COEFF_LEVEL_MINUS_ONE_CTX )
#else
  , m_cCUMapSCModel           ( MAX_CU_DEPTH,  2,               NUM_MAP_CTX                   )
  , m_cCULastSCModel          ( MAX_CU_DEPTH,  2,               NUM_LAST_CTX                  )
  , m_cCUOneSCModel           ( MAX_CU_DEPTH,  2,               NUM_ONE_CTX                   )
  , m_cCUAbsSCModel           ( MAX_CU_DEPTH,  2,               NUM_ABS_CTX                   )
#endif

  , m_cMVPIdxSCModel          ( 1,             1,               NUM_MVP_IDX_CTX               )
  , m_cCUROTindexSCModel      ( 1,             1,               NUM_ROT_IDX_CTX               )
  , m_cCUCIPflagCCModel       ( 1,             1,               NUM_CIP_FLAG_CTX              )
  , m_cALFFlagSCModel         ( 1,             1,               NUM_ALF_FLAG_CTX              )
  , m_cALFUvlcSCModel         ( 1,             1,               NUM_ALF_UVLC_CTX              )
  , m_cALFSvlcSCModel         ( 1,             1,               NUM_ALF_SVLC_CTX              )
#if HHI_ALF
  , m_cALFSplitFlagSCModel    ( 1,             1,               NUM_ALF_SPLITFLAG_CTX         )
#endif
#if PLANAR_INTRA
  , m_cPlanarIntraSCModel     ( 1,             1,               NUM_PLANAR_INTRA_CTX          )
#endif
{
  m_pcBitIf = 0;
  m_pcSlice = 0;
  m_pcBinIf = 0;

  m_uiCoeffCost = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
}

TEncSbac::~TEncSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSbac::resetEntropy           ()
{
  Int  iQp              = m_pcSlice->getSliceQp();
  SliceType eSliceType  = m_pcSlice->getSliceType();

  m_cCUSplitFlagSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );

  m_cCUSkipFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUAlfCtrlFlagSCModel.initBuffer  ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
#if HHI_MRG
  m_cCUMergeFlagSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG );
  m_cCUMergeIndexSCModel.initBuffer   ( eSliceType, iQp, (Short*)INIT_MERGE_INDEX );
#endif
  m_cCUPartSizeSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_PART_SIZE );
  m_cCUXPosiSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_CU_Y_POS );
  m_cCUPredModeSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_INTRA_PRED_MODE );
#if HHI_AIS
  m_cCUIntraFiltFlagSCModel.initBuffer( eSliceType, iQp, (Short*)INIT_INTRA_PRED_FILT );
#endif
  m_cCUChromaPredSCModel.initBuffer   ( eSliceType, iQp, (Short*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_REF_PIC );
#ifdef QC_AMVRES
  m_cCUMvResCModel.initBuffer         (eSliceType,  iQp, (Short*)INIT_MVRES_FLAG );
#endif
  m_cCUDeltaQpSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_DQP );
  m_cCUCbfSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_CBF );
#if HHI_RQT
  m_cCUQtCbfSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_QT_CBF );
#if HHI_RQT_ROOT
  m_cCUQtRootCbfSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_QT_ROOT_CBF );
#endif
#endif

#if HHI_TRANSFORM_CODING
  m_cCuCtxModSig.initBuffer           ( eSliceType, iQp, (Short*)INIT_SIG_FLAG );
  m_cCuCtxModLast.initBuffer          ( eSliceType, iQp, (Short*)INIT_LAST_FLAG );
  m_cCuCtxModAbsGreOne.initBuffer     ( eSliceType, iQp, (Short*)INIT_ABS_GREATER_ONE_FLAG );
  m_cCuCtxModCoeffLevelM1.initBuffer  ( eSliceType, iQp, (Short*)INIT_COEFF_LEVEL_MINUS_ONE_FLAG );
#else
  m_cCUMapSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_SIGMAP );
  m_cCULastSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_LAST_FLAG );
  m_cCUOneSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_TCOEFF_LEVEL );
#endif

  m_cMVPIdxSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cCUROTindexSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_ROT_IDX );
  m_cCUCIPflagCCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CIP_IDX );

#if HHI_ALF
  m_cALFSplitFlagSCModel.initBuffer   ( eSliceType, iQp, (Short*)INIT_ALF_SPLITFLAG );	
#endif
  
  m_cALFFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
#if HHI_RQT
  m_cCUTransSubdivFlagSCModel.initBuffer( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
#endif
  m_cCUTransIdxSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_TRANS_IDX );

#if PLANAR_INTRA
  m_cPlanarIntraSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_PLANAR_INTRA );
#endif

  // new structure
  m_uiLastQp          = iQp;

  m_pcBinIf->start();

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

Void TEncSbac::codePPS( TComPPS* pcPPS )
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
  m_pcBinIf->encodeBinTrm( uilsLast );
}

Void TEncSbac::codeSliceFinish()
{
  m_pcBinIf->finish();
}



Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[0] );

  if( 0 == uiSymbol)
  {
    return;
  }

  while( uiSymbol-- )
  {
    m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }

  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );

  if ( uiSymbol == 0 )
  {
    return;
  }

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    m_pcBinIf->encodeBin( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    m_pcBinIf->encodeBin( 0, pcSCModel[ iOffset ] );
  }

  return;
}

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    m_pcBinIf->encodeBinEP( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  m_pcBinIf->encodeBinEP( 0 );
  while( uiCount-- )
  {
    m_pcBinIf->encodeBinEP( (uiSymbol>>uiCount) & 1 );
  }

  return;
}

Void TEncSbac::xWriteExGolombLevel( UInt uiSymbol, ContextModel& rcSCModel  )
{
  if( uiSymbol )
  {
    m_pcBinIf->encodeBin( 1, rcSCModel );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);

    while( --uiSymbol && ++uiCount < 13 )
    {
      m_pcBinIf->encodeBin( 1, rcSCModel );
    }
    if( bNoExGo )
    {
      m_pcBinIf->encodeBin( 0, rcSCModel );
    }
    else
    {
      xWriteEpExGolomb( uiSymbol, 0 );
    }
  }
  else
  {
    m_pcBinIf->encodeBin( 0, rcSCModel );
  }

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
  m_pcBinIf->copyState( pSrc->m_pcBinIf );

  this->m_uiCoeffCost        = pSrc->m_uiCoeffCost;
  this->m_uiLastQp           = pSrc->m_uiLastQp;

  this->m_cCUSplitFlagSCModel     .copyFrom( &pSrc->m_cCUSplitFlagSCModel     );
  this->m_cCUSkipFlagSCModel      .copyFrom( &pSrc->m_cCUSkipFlagSCModel      );
#if HHI_MRG
  this->m_cCUMergeFlagSCModel     .copyFrom( &pSrc->m_cCUMergeFlagSCModel     );
  this->m_cCUMergeIndexSCModel    .copyFrom( &pSrc->m_cCUMergeIndexSCModel    );
#endif
  this->m_cCUPartSizeSCModel      .copyFrom( &pSrc->m_cCUPartSizeSCModel      );
  this->m_cCUPredModeSCModel      .copyFrom( &pSrc->m_cCUPredModeSCModel      );
  this->m_cCUIntraPredSCModel     .copyFrom( &pSrc->m_cCUIntraPredSCModel     );
#if HHI_AIS
  this->m_cCUIntraFiltFlagSCModel .copyFrom( &pSrc->m_cCUIntraFiltFlagSCModel );
#endif
  this->m_cCUChromaPredSCModel.copyFrom( &pSrc->m_cCUChromaPredSCModel  );
  this->m_cCUDeltaQpSCModel   .copyFrom( &pSrc->m_cCUDeltaQpSCModel     );
  this->m_cCUInterDirSCModel  .copyFrom( &pSrc->m_cCUInterDirSCModel    );
  this->m_cCURefPicSCModel    .copyFrom( &pSrc->m_cCURefPicSCModel      );
  this->m_cCUMvdSCModel       .copyFrom( &pSrc->m_cCUMvdSCModel         );
#ifdef QC_AMVRES
  this->m_cCUMvResCModel       .copyFrom( &pSrc->m_cCUMvResCModel         );
#endif
  this->m_cCUCbfSCModel       .copyFrom( &pSrc->m_cCUCbfSCModel         );
#if HHI_RQT
  this->m_cCUQtCbfSCModel     .copyFrom( &pSrc->m_cCUQtCbfSCModel       );
  this->m_cCUTransSubdivFlagSCModel.copyFrom( &pSrc->m_cCUTransSubdivFlagSCModel );
#if HHI_RQT_ROOT
  this->m_cCUQtRootCbfSCModel .copyFrom( &pSrc->m_cCUQtRootCbfSCModel   );
#endif
#endif
  this->m_cCUTransIdxSCModel  .copyFrom( &pSrc->m_cCUTransIdxSCModel    );

#if HHI_TRANSFORM_CODING
  this->m_cCuCtxModSig         .copyFrom( &pSrc->m_cCuCtxModSig          );
  this->m_cCuCtxModLast        .copyFrom( &pSrc->m_cCuCtxModLast         );
  this->m_cCuCtxModAbsGreOne   .copyFrom( &pSrc->m_cCuCtxModAbsGreOne    );
  this->m_cCuCtxModCoeffLevelM1.copyFrom( &pSrc->m_cCuCtxModCoeffLevelM1 );
#else
  this->m_cCUMapSCModel        .copyFrom( &pSrc->m_cCUMapSCModel         );
  this->m_cCULastSCModel       .copyFrom( &pSrc->m_cCULastSCModel        );
  this->m_cCUOneSCModel        .copyFrom( &pSrc->m_cCUOneSCModel         );
  this->m_cCUAbsSCModel        .copyFrom( &pSrc->m_cCUAbsSCModel         );
#endif

  this->m_cMVPIdxSCModel      .copyFrom( &pSrc->m_cMVPIdxSCModel        );

  this->m_cCUROTindexSCModel  .copyFrom( &pSrc->m_cCUROTindexSCModel    );
  this->m_cCUCIPflagCCModel   .copyFrom( &pSrc->m_cCUCIPflagCCModel     );
  this->m_cCUXPosiSCModel     .copyFrom( &pSrc->m_cCUXPosiSCModel       );
  this->m_cCUYPosiSCModel     .copyFrom( &pSrc->m_cCUXPosiSCModel       );

#if HHI_ALF
  this->m_cCUAlfCtrlFlagSCModel .copyFrom( &pSrc->m_cCUAlfCtrlFlagSCModel );
  this->m_cALFFlagSCModel       .copyFrom( &pSrc->m_cALFFlagSCModel       );
  this->m_cALFSplitFlagSCModel  .copyFrom( &pSrc->m_cALFSplitFlagSCModel  );
#endif    
}

// CIP
Void TEncSbac::codeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int CIPflag = pcCU->getCIPflag   ( uiAbsPartIdx );
  Int iCtx    = pcCU->getCtxCIPFlag( uiAbsPartIdx );

  m_pcBinIf->encodeBin( (CIPflag) ? 1 : 0, m_cCUCIPflagCCModel.get( 0, 0, iCtx ) );
}

Void TEncSbac::codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int indexROT = pcCU->getROTindex( uiAbsPartIdx );
  Int dictSize = ROT_DICT;

  switch (dictSize)
  {
   case 9:
    {
      m_pcBinIf->encodeBin( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        m_pcBinIf->encodeBin( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        m_pcBinIf->encodeBin( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
        m_pcBinIf->encodeBin( (indexROT & 0x04) >> 2, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
      }
    }
    break;
  case 4:
    {
      m_pcBinIf->encodeBin( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      m_pcBinIf->encodeBin( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 1 ) );
    }
    break;
  case 2:
    {
      m_pcBinIf->encodeBin( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
    }
    break;
  case 5:
    {
      m_pcBinIf->encodeBin( indexROT> 0 ? 0 : 1 , m_cCUROTindexSCModel.get( 0, 0, 0 ) );
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        m_pcBinIf->encodeBin( (indexROT & 0x01),      m_cCUROTindexSCModel.get( 0, 0, 1 ) );
        m_pcBinIf->encodeBin( (indexROT & 0x02) >> 1, m_cCUROTindexSCModel.get( 0, 0, 2 ) );
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
    m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
    m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
    m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 3) );
    m_pcBinIf->encodeBin( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 4) );
    return;
  }

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcBinIf->encodeBin( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
    return;
  }

  switch(eSize)
  {
  case SIZE_2Nx2N:
    {
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      break;
    }
  case SIZE_2NxN:
  case SIZE_2NxnU:
  case SIZE_2NxnD:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );

      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_2NxN)
        {
          m_pcBinIf->encodeBin(1, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        }
        else
        {
          m_pcBinIf->encodeBin(0, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          m_pcBinIf->encodeBin((eSize == SIZE_2NxnU? 0: 1), m_cCUYPosiSCModel.get( 0, 0, 1 ));
        }
      }
      break;
    }
  case SIZE_Nx2N:
  case SIZE_nLx2N:
  case SIZE_nRx2N:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );

      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_Nx2N)
        {
          m_pcBinIf->encodeBin(1, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        }
        else
        {
          m_pcBinIf->encodeBin(0, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          m_pcBinIf->encodeBin((eSize == SIZE_nLx2N? 0: 1), m_cCUXPosiSCModel.get( 0, 0, 1 ));
        }
      }
      break;
    }
  case SIZE_NxN:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );

      if (pcCU->getSlice()->isInterB())
      {
        m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 3) );
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

#if HHI_MRG
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    m_pcBinIf->encodeBin( iPredMode == MODE_SKIP ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
  }
#else
  m_pcBinIf->encodeBin( iPredMode == MODE_SKIP ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
#endif

  if (pcCU->getSlice()->isInterB() )
  {
    return;
  }

  if ( iPredMode != MODE_SKIP )
  {
    m_pcBinIf->encodeBin( iPredMode == MODE_INTER ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
  }
}

#if HHI_ALF
Void TEncSbac::codeAlfCoeff( Int iCoeff, Int iLength, Int iPos)
{
  if ( iCoeff == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

    // write sign
    if ( iCoeff > 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
      iCoeff = -iCoeff;
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

Void TEncSbac::codeAlfDc( Int iDc )
{
  if ( iDc == 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

      // write sign
      if ( iDc > 0 )
      {
        m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
      }
      else
      {
        m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
        iDc = -iDc;
      }

      xWriteEpExGolomb( iDc , 9 );
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
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx) ) );
}

Void TEncSbac::codeAlfQTCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0 ) );
}

Void TEncSbac::codeAlfQTSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth )
{
  //if( uiDepth >= uiMaxDepth )
  if( uiDepth >= g_uiMaxCUDepth - g_uiAddCUDepth ) // fix HS
    return;

  UInt uiSplitFlag =  ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) ? 1 : 0 ;
  //UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  m_pcBinIf->encodeBin( uiSplitFlag, m_cALFSplitFlagSCModel.get( 0, 0, 0 ) );
}
#else
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

  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx) ) );
}
#endif

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
  m_pcBinIf->encodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, pcCU->getCtxSkipFlag( uiAbsPartIdx) ) );
}

#if HHI_MRG
Void TEncSbac::codeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeFlagSCModel.get( 0, 0, pcCU->getCtxMergeFlag( uiAbsPartIdx ) ) );
}

Void TEncSbac::codeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeIndex( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeIndexSCModel.get( 0, 0, pcCU->getCtxMergeIndex( uiAbsPartIdx ) ) );
}
#endif

Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;

  UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;

  assert( uiCtx < 3 );
  m_pcBinIf->encodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiCtx ) );
#if HHI_RQT
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
#endif
  return;
}

#if HHI_RQT
Void TEncSbac::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcBinIf->encodeBin( uiSymbol, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\n" )
}
#endif

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

  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, m_cCUTransIdxSCModel.get( 0, 0, pcCU->getCtxTransIdx( uiAbsPartIdx ) ) );

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
    m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, m_cCUTransIdxSCModel.get( 0, 0, 3 ) );
    if ( uiSymbol == 0 )
    {
      return;
    }
    uiSymbol--;
  }

  return;
}

#if PLANAR_INTRA
Void TEncSbac::xPutPlanarBins( Int n, Int cn )
{
  UInt tmp  = 1<<(n-4);
  UInt code = tmp+cn%tmp;
  UInt len  = 1+(n-4)+(cn>>(n-4));
  Int  ctr;

  for( ctr = len-1; ctr >= 0; ctr-- )
    m_pcBinIf->encodeBinEP( (code & (1 << ctr)) >> ctr );
}

Void TEncSbac::xCodePlanarDelta( TComDataCU* pcCU, UInt uiAbsPartIdx , Int iDelta )
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
    xPutPlanarBins( 5, uiDeltaAbs );
  else if( uiDeltaAbs < 16 )
    xPutPlanarBins( 5, (uiDeltaAbs>>1)+2 );
  else if( uiDeltaAbs < 64)
    xPutPlanarBins( 5, (uiDeltaAbs>>2)+6 );
  else
    xPutPlanarBins( 5, (uiDeltaAbs>>3)+14 );

  if(uiDeltaAbs > 0)
    m_pcBinIf->encodeBinEP( bDeltaNegative ? 1 : 0 );

}

Void TEncSbac::codePlanarInfo( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (pcCU->isIntra( uiAbsPartIdx ))
  {
    UInt uiPlanar = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_FLAG);

    m_pcBinIf->encodeBin( uiPlanar, m_cPlanarIntraSCModel.get( 0, 0, 0 ) );

    if ( uiPlanar )
    {
      // Planar delta for Y
      xCodePlanarDelta( pcCU, uiAbsPartIdx, pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAY) );

      // Planar delta for U and V
      Int  iPlanarDeltaU = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAU);
      Int  iPlanarDeltaV = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAV);

      m_pcBinIf->encodeBin( ( iPlanarDeltaU == 0 && iPlanarDeltaV == 0 ) ? 1 : 0, m_cPlanarIntraSCModel.get( 0, 0, 1 ) );

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
Void TEncSbac::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );

  if (uiDir == iMostProbable)
    m_pcBinIf->encodeBin( 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
  else{
    m_pcBinIf->encodeBin( 0, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
    uiDir = uiDir > iMostProbable ? uiDir - 1 : uiDir;
#if UNIFIED_DIRECTIONAL_INTRA
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcBinIf->encodeBin((uiDir & 0x01), m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));
    }
    else
#endif
    if (uiDir < 31){ // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
      m_pcBinIf->encodeBin((uiDir & 0x01),      m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin((uiDir & 0x10) >> 4, m_cCUIntraPredSCModel.get(0, 0, 1));
    }
    else{
      m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
      m_pcBinIf->encodeBin((uiDir == 32) ? 1 : 0, m_cCUIntraPredSCModel.get(0, 0, 1));
    }
  }

  return;
}
#endif

Void TEncSbac::codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iIntraDirLuma = pcCU->convertIntraDirLumaAdi( pcCU, uiAbsPartIdx );
  Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);

  m_pcBinIf->encodeBin( iIntraDirLuma >= 0 ? 0 : 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );

  if (iIntraDirLuma >= 0)
  {
    m_pcBinIf->encodeBin((iIntraDirLuma & 0x01), m_cCUIntraPredSCModel.get(0, 0, 1));

    m_pcBinIf->encodeBin((iIntraDirLuma & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));

    if (g_aucIntraModeBits[iIntraIdx] >= 4)
    {
      m_pcBinIf->encodeBin((iIntraDirLuma & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));

      if (g_aucIntraModeBits[iIntraIdx] >= 5)
      {
        m_pcBinIf->encodeBin((iIntraDirLuma & 0x08) >> 3,
          m_cCUIntraPredSCModel.get(0, 0, 1));

        if (g_aucIntraModeBits[iIntraIdx] >= 6)
        {
          m_pcBinIf->encodeBin((iIntraDirLuma & 0x10) >> 4,
            m_cCUIntraPredSCModel.get(0, 0, 1));
        }
      }
    }
  }
  return;
}

#if HHI_AIS
Void TEncSbac::codeIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getLumaIntraFiltFlag( uiAbsPartIdx );
#if ANG_INTRA
  UInt uiCtx    = pcCU->angIntraEnabledPredPart( uiAbsPartIdx ) ?  pcCU->getCtxIntraFiltFlagLumaAng( uiAbsPartIdx ) : pcCU->getCtxIntraFiltFlagLuma( uiAbsPartIdx );
#else
  UInt uiCtx    = pcCU->getCtxIntraFiltFlagLuma( uiAbsPartIdx );
#endif
  m_pcBinIf->encodeBin( uiSymbol, m_cCUIntraFiltFlagSCModel.get( 0, 0, uiCtx ) );

  return;
}
#endif

Void TEncSbac::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCtx            = pcCU->getCtxIntraDirChroma( uiAbsPartIdx );
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );

  if ( 0 == uiIntraDirChroma )
  {
    m_pcBinIf->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
    xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
  }

  return;
}

Void TEncSbac::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  UInt uiCtx      = pcCU->getCtxInterDir( uiAbsPartIdx );
  uiInterDir--;
  m_pcBinIf->encodeBin( ( uiInterDir == 2 ? 1 : 0 ), m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );

  if ( uiInterDir < 2 )
  {
    m_pcBinIf->encodeBin( uiInterDir, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }

  return;
}

Void TEncSbac::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );

  UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );

  m_pcBinIf->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), m_cCURefPicSCModel.get( 0, 0, uiCtx ) );

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

#ifdef QC_AMVRES
  if(pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
	  TComMv rcMv  = pcCUMvField->getMv ( uiAbsPartIdx );
	  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
	  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
	  Int iL =   ( (pcCUMvFieldL == NULL) ? 1 : (Int)(pcCUMvFieldL->getMVRes(uiAbsPartIdxL)));
	  Int iV =   ( (pcCUMvFieldA == NULL) ? 1 : (Int)(pcCUMvFieldA->getMVRes(uiAbsPartIdxA)));

	  xWriteMvResFlag(!rcMv.isHAM(),iL+iV);
	  if (!rcMv.isHAM()) 
	  {
		  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor()>>1) ) +
					 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor()>>1) );
		  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer()>>1) ) +
					 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer()>>1) );
	      xWriteMvd( iHor/2, iHorPred, 0 );
	      xWriteMvd( iVer/2, iVerPred, 1 );
		  return;
	  }
  }
#endif
 
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
    m_pcBinIf->encodeBin( 0, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );

    UInt uiDQp = (UInt)( iDQp > 0 ? ( 2 * iDQp - 2 ) : ( -2 * iDQp - 1 ) );
    xWriteUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
  }

  return;
}

Void TEncSbac::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
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
  UInt uiCtx = pcCU->getCtxCbf( uiAbsPartIdx, eType, uiTrDepth );

  m_pcBinIf->encodeBin( uiCbf , m_cCUCbfSCModel.get( 0, eType == TEXT_LUMA ? 0 : 1, 3 - uiCtx ) );

  return;
}

#if HHI_RQT
Void TEncSbac::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf     ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1 : eType, uiCtx ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( eType )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}

UInt xCheckCoeffPlainCNoRecur( const TCoeff* pcCoef, UInt uiSize, UInt uiDepth )
{
  UInt uiNumofCoeff = 0;
  UInt ui = uiSize>>uiDepth;
  {
    UInt x, y;
    const TCoeff* pCeoff = pcCoef;
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
  return uiNumofCoeff;
}

#if HHI_RQT_ROOT
Void TEncSbac::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  UInt uiCtx = pcCU->getCtxQtRootCbf( uiAbsPartIdx );
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}
#endif
#endif

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

#if HHI_RQT
UInt xCheckCoeffPlainCNoRecur( const TCoeff* pcCoef, UInt uiSize, UInt uiDepth );
#endif

#if QC_MDDT
Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, UInt uiMode, Bool bRD )
#else
Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
#endif
{
#if HHI_RQT
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseCoeffNxN()\teType=" )
  DTRACE_CABAC_V( eTType )
  DTRACE_CABAC_T( "\twidth=" )
  DTRACE_CABAC_V( uiWidth )
  DTRACE_CABAC_T( "\theight=" )
  DTRACE_CABAC_V( uiHeight )
  DTRACE_CABAC_T( "\tdepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\tabspartidx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\ttoCU-X=" )
  DTRACE_CABAC_V( pcCU->getCUPelX() )
  DTRACE_CABAC_T( "\ttoCU-Y=" )
  DTRACE_CABAC_V( pcCU->getCUPelY() )
  DTRACE_CABAC_T( "\tCU-addr=" )
  DTRACE_CABAC_V(  pcCU->getAddr() )
  DTRACE_CABAC_T( "\tinCU-X=" )
  DTRACE_CABAC_V( g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tinCU-Y=" )
  DTRACE_CABAC_V( g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tpredmode=" )
  DTRACE_CABAC_V(  pcCU->getPredictionMode( uiAbsPartIdx ) )
  DTRACE_CABAC_T( "\n" )
#endif
#if HHI_TRANSFORM_CODING
  if( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }

  UInt uiNumSig = 0;
  UInt uiCTXIdx = xGetCTXIdxFromWidth( uiWidth );

  // compute number of significant coefficients
  UInt  uiPart = 0;
  xCheckCoeff(pcCoef, uiWidth, 0, uiNumSig, uiPart );

  if ( bRD )
  {
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }

  if ( uiNumSig == 0 )
    return;

  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );

   //----- encode significance map -----
  const UInt   uiMaxNumCoeff     = uiWidth*uiHeight;
  const UInt   uiMaxNumCoeffM1   = uiMaxNumCoeff - 1;
  const UInt   uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  UInt         uiDownLeft        = 1;
  UInt         uiNumSigTopRight  = 0;
  UInt         uiNumSigBotLeft   = 0;

#if QC_MDDT//ADAPTIVE_SCAN
  const UInt*  pucScan;
  const UInt*  pucScanX;
  const UInt*  pucScanY;
  UInt *scanStats;
  int indexROT ;
  if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && (uiWidth == 4 || uiWidth == 8 /*|| uiWidth == 16 || uiWidth == 32 || uiWidth == 64*/))//!bRD &&  eTType == TEXT_LUMA)
  {
	indexROT = pcCU->getROTindex(uiAbsPartIdx);
	//int scan_index;
#if ROT_CHECK
    if(uiWidth == 4 && indexROT == 0)
#else
    if(uiWidth == 4)
#endif
    {
       UInt uiPredMode = g_aucIntra9Mode[uiMode];
       pucScan = scanOrder4x4[uiPredMode]; pucScanX = scanOrder4x4X[uiPredMode]; pucScanY = scanOrder4x4Y[uiPredMode];

	   if(g_bUpdateStats)
       {
         scanStats = scanStats4x4[uiPredMode]; update4x4Count[uiPredMode]++;
       }
    }
#if ROT_CHECK
    else if(uiWidth == 8 && indexROT == 0)
#else
    else if(uiWidth == 8)
#endif
    {
      UInt uiPredMode = ((1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth) ?  g_aucIntra9Mode[uiMode]: g_aucAngIntra9Mode[uiMode];
      pucScan = scanOrder8x8[uiPredMode]; pucScanX = scanOrder8x8X[uiPredMode]; pucScanY = scanOrder8x8Y[uiPredMode];

	   if(g_bUpdateStats)
      {
        scanStats = scanStats8x8[uiPredMode]; update8x8Count[uiPredMode]++;
      }
    }
	/*else if(uiWidth == 16)
	{
		scan_index = LUT16x16[indexROT][uiMode];
		pucScan = scanOrder16x16[scan_index]; pucScanX = scanOrder16x16X[scan_index]; pucScanY = scanOrder16x16Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats16x16[scan_index];
		}
    }
    else if(uiWidth == 32)
    {
		scan_index = LUT32x32[indexROT][uiMode];
		pucScan = scanOrder32x32[scan_index]; pucScanX = scanOrder32x32X[scan_index]; pucScanY = scanOrder32x32Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats32x32[scan_index];
		}
    }
    else if(uiWidth == 64)
    {
		scan_index = LUT64x64[indexROT][uiMode];
		pucScan = scanOrder64x64[scan_index]; pucScanX = scanOrder64x64X[scan_index]; pucScanY = scanOrder64x64Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats64x64[scan_index];
		}
    }*/
    else
    {
      //printf("uiWidth = %d is not supported!\n", uiWidth);
      //exit(1);
    }
  }
#endif

  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeffM1; uiScanPos++ )
  {
    UInt  uiBlkPos  = g_auiSigLastScan[ uiLog2BlockSize ][ uiDownLeft ][ uiScanPos ];
#if QC_MDDT
    if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && (uiWidth == 4 || uiWidth == 8 /*|| uiWidth == 16 || uiWidth == 32 || uiWidth == 64*/))//!bRD &&  eTType == TEXT_LUMA)
    {
#if ROT_CHECK
      if(/* uiMode<=8  && */ indexROT == 0) 
        uiBlkPos = pucScan[uiScanPos];
#else
        uiBlkPos = pucScan[uiScanPos];
#endif
    }
#endif
    UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
    UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );

    //===== code significance flag =====
    UInt  uiSig    = pcCoef[ uiBlkPos ] != 0 ? 1 : 0;
    UInt  uiCtxSig = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, ( uiDownLeft > 0 ) );
    m_pcBinIf->encodeBin( uiSig, m_cCuCtxModSig.get( uiCTXIdx, eTType, uiCtxSig ) );

    if( uiSig )
    {
#if QC_MDDT//ADAPTIVE_SCAN
#if ROT_CHECK
      if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && ((uiWidth == 4 && /* uiMode<=8 && */ indexROT == 0)|| (uiWidth == 8 && /* uiMode<=8 &&*/ indexROT == 0) /*|| uiWidth == 16 || uiWidth == 32 || uiWidth == 64*/))
#else
     if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && (uiWidth == 4 || uiWidth == 8))
#endif
      {
        scanStats[uiScanPos]++;
      }
#endif
      uiNumSig--;

      if( uiPosX > uiPosY )
      {
        uiNumSigTopRight++;
      }
      else if( uiPosY > uiPosX )
      {
        uiNumSigBotLeft ++;
      }

      //===== code last flag =====
      UInt  uiLast    = ( uiNumSig == 0 ) ? 1 : 0;
      UInt  uiCtxLast = TComTrQuant::getLastCtxInc( uiPosX, uiPosY, uiLog2BlockSize );
      m_pcBinIf->encodeBin( uiLast, m_cCuCtxModLast.get( uiCTXIdx, eTType, uiCtxLast ) );

      if( uiLast )
      {
        break;
      }
    }

    //===== update scan direction =====
    if( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiHeight - 1 ) ) ||
        ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiWidth  - 1 ) )   )
    {
      uiDownLeft = ( uiNumSigTopRight >= uiNumSigBotLeft ? 1 : 0 );
    }
  }

  Int  c1, c2;
  UInt uiAbs;
  UInt uiSign;
  UInt uiPrevAbsGreOne     = 0;
  const UInt uiNumOfSets   = 4;
  const UInt uiNum4x4Blk   = max<UInt>( 1, uiMaxNumCoeff / 16 );

  if ( uiLog2BlockSize > 2 )
  {
    Bool bFirstBlock = true;

    for ( UInt uiSubBlk = 0; uiSubBlk < uiNum4x4Blk; uiSubBlk++ )
    {
      UInt uiCtxSet    = 0;
      UInt uiSubNumSig = 0;
      UInt uiSubLog2   = uiLog2BlockSize - 2;
      UInt uiSubPosX   = 0;
      UInt uiSubPosY   = 0;

      if ( uiSubLog2 > 1 )
      {
#if HHI_RQT
        uiSubPosX = g_auiFrameScanX[ uiSubLog2 - 1 ][ uiSubBlk ] * 4;
        uiSubPosY = g_auiFrameScanY[ uiSubLog2 - 1 ][ uiSubBlk ] * 4;
#else
        uiSubPosX = g_auiFrameScanX[ uiSubLog2 - 2 ][ uiSubBlk ] * 4;
        uiSubPosY = g_auiFrameScanY[ uiSubLog2 - 2 ][ uiSubBlk ] * 4;
#endif
      }
      else
      {
        uiSubPosX = ( uiSubBlk < 2      ) ? 0 : 1;
        uiSubPosY = ( uiSubBlk % 2 == 0 ) ? 0 : 1;
        uiSubPosX *= 4;
        uiSubPosY *= 4;
      }

      TCoeff* piCurr = &pcCoef[ uiSubPosX + uiSubPosY * uiWidth ];

      for ( UInt uiY = 0; uiY < 4; uiY++ )
      {
        for ( UInt uiX = 0; uiX < 4; uiX++ )
        {
          if( piCurr[ uiX ] )
          {
            uiSubNumSig++;
          }
        }
        piCurr += uiWidth;
      }

      if ( uiSubNumSig > 0 )
      {
        c1 = 1;
        c2 = 0;

        if ( bFirstBlock )
        {
          bFirstBlock = false;
          uiCtxSet = uiNumOfSets + 1;
        }
        else
        {
          uiCtxSet = uiPrevAbsGreOne / uiNumOfSets + 1;//( ( uiGreOne * uiEqRangeSize ) >> 8 ) + 1;
          uiPrevAbsGreOne = 0;
        }

        for ( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
#if HHI_RQT
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
#else
          UInt  uiBlkPos  = g_auiFrameScanXY[ 0 ][ 15 - uiScanPos ];
#endif
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = (uiSubPosY + uiPosY) * uiWidth + uiSubPosX + uiPosX;

          if( pcCoef[ uiIndex ]  )
          {
            if( pcCoef[ uiIndex ] > 0) { uiAbs = static_cast<UInt>( pcCoef[ uiIndex ]);  uiSign = 0; }
            else                       { uiAbs = static_cast<UInt>(-pcCoef[ uiIndex ]);  uiSign = 1; }

            UInt uiCtx    = min<UInt>(c1, 4);
            UInt uiSymbol = uiAbs > 1 ? 1 : 0;
            m_pcBinIf->encodeBin( uiSymbol, m_cCuCtxModAbsGreOne.get( 0, eTType, (uiCtxSet * 5) + uiCtx ) );

            if( uiSymbol )
            {
              uiCtx  = min<UInt>(c2, 4);
              uiAbs -= 2;
              c1     = 0;
              c2++;
              uiPrevAbsGreOne++;
              xWriteExGolombLevel( uiAbs, m_cCuCtxModCoeffLevelM1.get( 0, eTType, (uiCtxSet * 5) + uiCtx ) );
            }
            else if( c1 )
            {
              c1++;
            }
            m_pcBinIf->encodeBinEP( uiSign );
          }
        }
      }
    }
  }
  else
  {
    c1 = 1;
    c2 = 0;

    for ( UInt uiScanPos = 0; uiScanPos < uiWidth*uiHeight; uiScanPos++ )
    {
#if HHI_RQT
      UInt uiIndex = g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiWidth*uiHeight - uiScanPos - 1 ];
#else
      UInt uiIndex = g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] ][ uiWidth*uiHeight - uiScanPos - 1 ];
#endif

      if( pcCoef[ uiIndex ]  )
      {
        if( pcCoef[ uiIndex ] > 0) { uiAbs = static_cast<UInt>( pcCoef[ uiIndex ]);  uiSign = 0; }
        else                       { uiAbs = static_cast<UInt>(-pcCoef[ uiIndex ]);  uiSign = 1; }

        UInt uiCtx    = min<UInt>(c1, 4);
        UInt uiSymbol = uiAbs > 1 ? 1 : 0;
        m_pcBinIf->encodeBin( uiSymbol, m_cCuCtxModAbsGreOne.get( 0, eTType, uiCtx ) );

        if( uiSymbol )
        {
          uiCtx  = min<UInt>(c2, 4);
          uiAbs -= 2;
          c1     = 0;
          c2++;
          xWriteExGolombLevel( uiAbs, m_cCuCtxModCoeffLevelM1.get( 0, eTType, uiCtx ) );
        }
        else if( c1 )
        {
          c1++;
        }
        m_pcBinIf->encodeBinEP( uiSign );
      }
    }
  }
  return;
#else
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
#if HHI_RQT
  assert( uiNumSig == xCheckCoeffPlainCNoRecur( piCoeff, uiWidth, 0 ) );
#endif

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
#if QC_MDDT == 0//ADAPTIVE_SCAN == 0
  uiSig = piCoeff[0] ? 1 : 0;

  m_pcBinIf->encodeBin( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ 0 ] ) );

  if( uiSig )
  {
    uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
    m_pcBinIf->encodeBin( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ 0 ] ) );
  }
#endif
  // initialize scan
  const UInt*  pucScan;
  const UInt*  pucScanX;
  const UInt*  pucScanY;

  UInt uiConvBit = g_aucConvertToBit[ uiWidth ];
#if HHI_RQT
  pucScan  = g_auiFrameScanXY[ uiConvBit + 1 ]; 
  pucScanX = g_auiFrameScanX [ uiConvBit + 1 ];
  pucScanY = g_auiFrameScanY [ uiConvBit + 1 ];
#else
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
#endif
#if QC_MDDT//ADAPTIVE_SCAN
  UInt *scanStats;
  int indexROT ;
  if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA)//!bRD &&  eTType == TEXT_LUMA)
  {
    UInt uiPredMode = g_aucIntra9Mode[uiMode];
	indexROT = pcCU->getROTindex(uiAbsPartIdx);
	int scan_index;
#if ROT_CHECK
    if(uiWidth == 4 && indexROT == 0)
#else
    if(uiWidth == 4)
#endif
    {
       UInt uiPredMode = g_aucIntra9Mode[uiMode];

       pucScan = scanOrder4x4[uiPredMode]; pucScanX = scanOrder4x4X[uiPredMode]; pucScanY = scanOrder4x4Y[uiPredMode];


	   if(g_bUpdateStats)
       {
         scanStats = scanStats4x4[uiPredMode]; update4x4Count[uiPredMode]++;
       }
    }
#if ROT_CHECK
    else if(uiWidth == 8 && indexROT == 0)
#else
    else if(uiWidth == 8)
#endif
    {
      UInt uiPredMode = ((1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth) ? g_aucIntra9Mode[uiMode] : g_aucAngIntra9Mode[uiMode];
      pucScan = scanOrder8x8[uiPredMode]; pucScanX = scanOrder8x8X[uiPredMode]; pucScanY = scanOrder8x8Y[uiPredMode];

	   if(g_bUpdateStats)
      {
        scanStats = scanStats8x8[uiPredMode]; update8x8Count[uiPredMode]++;
      }
    }
	else if(uiWidth == 16)
	{
		scan_index = LUT16x16[indexROT][uiMode];
		pucScan = scanOrder16x16[scan_index]; pucScanX = scanOrder16x16X[scan_index]; pucScanY = scanOrder16x16Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats16x16[scan_index];
		}
    }
    else if(uiWidth == 32)
    {
		scan_index = LUT32x32[indexROT][uiMode];
		pucScan = scanOrder32x32[scan_index]; pucScanX = scanOrder32x32X[scan_index]; pucScanY = scanOrder32x32Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats32x32[scan_index];
		}
    }
    else if(uiWidth == 64)
    {
		scan_index = LUT64x64[indexROT][uiMode];
		pucScan = scanOrder64x64[scan_index]; pucScanX = scanOrder64x64X[scan_index]; pucScanY = scanOrder64x64Y[scan_index];

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
	//----- encode significance map -----
  // DC is coded in the beginning
#if QC_MDDT//ADAPTIVE_SCAN
  for( ui = 0; ui < ( uiSize - 1 ) && !uiLast; ui++ ) // if last coeff is reached, it has to be significant
#else
  for( ui = 1; ui < ( uiSize - 1 ) && !uiLast; ui++ ) // if last coeff is reached, it has to be significant
#endif
	{
		uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;

#if HHI_RQT
    if (uiCtxSize < uiSize)
#else
    if (uiCtxSize != uiSize)
#endif
    {
      UInt uiXX, uiYY;
      uiXX = pucScanX[ui]/(uiWidth / 8);
      uiYY = pucScanY[ui]/(uiHeight / 8);

      uiCtx = g_auiAntiScan8[uiYY*8+uiXX];
    }
    else
      uiCtx = ui * uiCtxSize / uiSize;

#if QC_MDDT//ADAPTIVE_SCAN
#if ROT_CHECK
     if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && uiWidth == 8 /* && uiMode<=8*/ && indexROT == 0)//!bRD &&  eTType == TEXT_LUMA)
#else
     if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && uiWidth == 8)
#endif
     {
       assert(uiCtxSize == uiSize);
#ifdef QC_CTX
	   m_pcBinIf->encodeBin( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ uiCtx ] ) );
#else
       m_pcBinIf->encodeBin( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ g_auiAntiScan8[pucScan[ui]] ] ) );
#endif
     }
     else
#endif
		m_pcBinIf->encodeBin( uiSig, m_cCUMapSCModel.get( uiCTXIdx, eTType, pos2ctx_map[ uiCtx ] ) );

		if( uiSig )
		{
#if QC_MDDT//ADAPTIVE_SCAN
#if ROT_CHECK
      if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && ((uiWidth == 4 && /*uiMode<=8 &&*/ indexROT == 0)|| (uiWidth == 8 &&/*uiMode<=8 &&*/ indexROT == 0) || uiWidth == 16 || uiWidth == 32 || uiWidth == 64)) 
#else
      if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA )
#endif
      {
			  scanStats[ui]++;
      }
#endif
      uiCtx = ui * uiCtxSize / uiSize;
			uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
			m_pcBinIf->encodeBin( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, pos2ctx_last[ uiCtx ] ) );

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

      uiCtx    = Min (c1, 4);
      UInt uiSymbol = uiAbs > 1 ? 1 : 0;
      m_pcBinIf->encodeBin( uiSymbol, m_cCUOneSCModel.get( uiCTXIdx, eTType, uiCtx ) );

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
      m_pcBinIf->encodeBinEP( uiSign );
    }
  }
  return;
#endif
}

Void TEncSbac::xWriteMvd( Int iMvd, UInt uiAbsSum, UInt uiCtx )
{
  UInt uiLocalCtx = 0;
  if ( uiAbsSum >= 3 )
  {
    uiLocalCtx += ( uiAbsSum > 32 ) ? 2 : 1;
  }

  UInt uiSymbol = ( 0 == iMvd ) ? 0 : 1;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );
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
  m_pcBinIf->encodeBinEP( uiSign );

  return;
}

Void  TEncSbac::xWriteExGolombMvd( UInt uiSymbol, ContextModel* pcSCModel, UInt uiMaxBin )
{
  if ( ! uiSymbol )
  {
    m_pcBinIf->encodeBin( 0, *pcSCModel );
    return;
  }

  m_pcBinIf->encodeBin( 1, *pcSCModel );

  Bool bNoExGo = ( uiSymbol < 8 );
  UInt uiCount = 1;
  pcSCModel++;

  while ( --uiSymbol && ++uiCount <= 8 )
  {
    m_pcBinIf->encodeBin( 1, *pcSCModel );
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
    m_pcBinIf->encodeBin( 0, *pcSCModel );
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
  m_pcBinIf->encodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
}

Void TEncSbac::codeAlfUvlc       ( UInt uiCode )
{
  Int i;

  if ( uiCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
    for ( i=0; i<uiCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
  }
}

Void TEncSbac::codeAlfSvlc       ( Int iCode )
{
  Int i;

  if ( iCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

    // write sign
    if ( iCode > 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
      iCode = -iCode;
    }

    // write magnitude
    for ( i=0; i<iCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
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
#if HHI_TRANSFORM_CODING
  Int    k;
  UShort sig, last;
  Int    k1 = 16;

  for ( k = 0; k < k1; k++ )
  {
    sig = 0;
    pcEstBitsSbac->significantBits[k][sig] = biari_no_bits (sig, m_cCuCtxModSig.get( uiCTXIdx, eTType, k ));

    sig = 1;
    pcEstBitsSbac->significantBits[k][sig] = biari_no_bits (sig, m_cCuCtxModSig.get( uiCTXIdx, eTType, k ));

    last = 0;
    pcEstBitsSbac->lastBits[k][last] = biari_no_bits (last, m_cCuCtxModLast.get( uiCTXIdx, eTType, k ));

    last = 1;
    pcEstBitsSbac->lastBits[k][last] = biari_no_bits (last, m_cCuCtxModLast.get( uiCTXIdx, eTType, k ));
  }
#else
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
#endif
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoefficientsBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType)
{
#if HHI_TRANSFORM_CODING
  Int   ctx;
  uiCTXIdx = 0;

  for ( UInt ui = 0; ui < 6; ui++ )
  {
    for ( ctx = 0; ctx <= 4; ctx++ )
    {
      pcEstBitsSbac->greaterOneBits[ui][0][ctx][0] = biari_no_bits( 0, m_cCuCtxModAbsGreOne.get( uiCTXIdx, eTType, (ui * 5) + ctx ) );
      pcEstBitsSbac->greaterOneBits[ui][0][ctx][1] = biari_no_bits( 1, m_cCuCtxModAbsGreOne.get( uiCTXIdx, eTType, (ui * 5) + ctx ) );
    }

    for ( ctx = 0; ctx <= 4; ctx++ )
    {
      pcEstBitsSbac->greaterOneBits[ui][1][ctx][0] = biari_no_bits( 0, m_cCuCtxModCoeffLevelM1.get( uiCTXIdx, eTType, (ui * 5) + ctx ) );
      pcEstBitsSbac->greaterOneBits[ui][1][ctx][1] = biari_no_bits( 1, m_cCuCtxModCoeffLevelM1.get( uiCTXIdx, eTType, (ui * 5) + ctx ) );
    }
  }
#else
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
#endif
}

Int TEncSbac::biari_no_bits (Short symbol, ContextModel& rcSCModel)
{
  Int ctx_state, estBits;

  symbol = (Short) (symbol != 0);

  ctx_state = (symbol == rcSCModel.getMps() ) ? 64 + rcSCModel.getState() : 63 - rcSCModel.getState();

  estBits = entropyBits[127-ctx_state];

  return estBits;
}

Int TEncSbac::biari_state (Short symbol, ContextModel& rcSCModel)
{
  Int ctx_state;

  symbol = (Short) (symbol != 0);

  ctx_state = ( symbol == rcSCModel.getMps() ) ? 64 + rcSCModel.getState() : 63 - rcSCModel.getState();

  return ctx_state;
}

#ifdef QC_AMVRES
Void TEncSbac::xWriteMvResFlag( Int iVal,Int Ctx_idx)
{
  // send flag
  m_pcBinIf->encodeBin( iVal, m_cCUMvResCModel.get( 0, 0, Ctx_idx ) );
}
#endif


#ifdef QC_SIFO
Void TEncSbac::encodeSwitched_Filters(TComSlice* pcSlice,TComPrediction *m_cPrediction)
{
	assert(0);
  return;
}
#endif
