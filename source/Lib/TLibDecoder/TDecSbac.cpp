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

/** \file     TDecSbac.cpp
    \brief    Context-adaptive entropy decoder class
*/

#include "TDecSbac.h"
#include "SEIread.h"

//! \ingroup TLibDecoder
//! \{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_pcTDecBinCabac            ( NULL )
, m_numContextModels          ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            , m_contextModels + m_numContextModels, m_numContextModels )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeFlagExtSCModel    ( 1,             1,               NUM_MERGE_FLAG_EXT_CTX        , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeIdxExtSCModel     ( 1,             1,               NUM_MERGE_IDX_EXT_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCURefPicSCModel          ( 1,             1,               NUM_REF_NO_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMvdSCModel             ( 1,             1,               NUM_MV_RES_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
#if CHROMA_CBF_CTX_REDUCTION
, m_cCUQtCbfSCModel           ( 1,             2,               NUM_QT_CBF_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
#else
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cCUTransSubdivFlagSCModel ( 1,             1,               NUM_TRANS_SUBDIV_FLAG_CTX     , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
#if MULTI_LEVEL_SIGNIFICANCE
, m_cCUSigCoeffGroupSCModel   ( 1,             2,               NUM_SIG_CG_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if SIGMAP_CTX_RED
, m_cCUSigSCModelLuma         ( 1,             1,               NUM_SIG_FLAG_CTX_LUMA         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigSCModelChroma       ( 1,             1,               NUM_SIG_FLAG_CTX_CHROMA       , m_contextModels + m_numContextModels, m_numContextModels)
#else
, m_cCUSigSCModel             ( 1,             2,               NUM_SIG_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cCuCtxLastX               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastY               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          , m_contextModels + m_numContextModels, m_numContextModels)
#if COEFF_CTXSET_RED
, m_cCUOneSCModelLuma         ( 1,             1,               NUM_ONE_FLAG_CTX_LUMA         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUOneSCModelChroma       ( 1,             1,               NUM_ONE_FLAG_CTX_CHROMA       , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAbsSCModelLuma         ( 1,             1,               NUM_ABS_FLAG_CTX_LUMA         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAbsSCModelChroma       ( 1,             1,               NUM_ABS_FLAG_CTX_CHROMA       , m_contextModels + m_numContextModels, m_numContextModels)
#else
, m_cCUOneSCModel             ( 1,             2,               NUM_ONE_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAbsSCModel             ( 1,             2,               NUM_ABS_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cMVPIdxSCModel            ( 1,             1,               NUM_MVP_IDX_CTX               , m_contextModels + m_numContextModels, m_numContextModels)
#if AMP
, m_cCUXPosiSCModel           ( 1,             1,               NUM_CU_X_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUYPosiSCModel           ( 1,             1,               NUM_CU_Y_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
{
  assert( m_numContextModels <= MAX_NUM_CTX_MOD );
}

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if F747_APS
Void TDecSbac::resetEntropywithQPandInitIDC (Int  iQp, Int iID)
{
  SliceType eSliceType = (SliceType)iID;

#else

Void TDecSbac::resetEntropy          (TComSlice* pcSlice)
{
  Int  iQp              = pcSlice->getSliceQp();
  SliceType eSliceType  = pcSlice->getSliceType();
#endif
#if G633_8BIT_INIT
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT );
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PART_SIZE );
#if AMP
  m_cCUXPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_Y_POS );
#endif
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (UChar*)INIT_REF_PIC );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_QT_ROOT_CBF );
#if MULTI_LEVEL_SIGNIFICANCE
  m_cCUSigCoeffGroupSCModel.initBuffer   ( eSliceType, iQp, (UChar*)INIT_SIG_CG_FLAG );
#endif
#if SIGMAP_CTX_RED
  m_cCUSigSCModelLuma.initBuffer         ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG_LUMA );
  m_cCUSigSCModelChroma.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG_CHROMA );
#else
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG );
#endif
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
#if COEFF_CTXSET_RED
  m_cCUOneSCModelLuma.initBuffer         ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG_LUMA );
  m_cCUOneSCModelChroma.initBuffer       ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG_CHROMA );
  m_cCUAbsSCModelLuma.initBuffer         ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG_LUMA );
  m_cCUAbsSCModelChroma.initBuffer       ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG_CHROMA );
#else
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG );
#endif
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (UChar*)INIT_MVP_IDX );
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
  
#else
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT );
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PART_SIZE );
#if AMP
  m_cCUXPosiSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_CU_Y_POS );
#endif
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_REF_PIC );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_QT_ROOT_CBF );
#if MULTI_LEVEL_SIGNIFICANCE
  m_cCUSigCoeffGroupSCModel.initBuffer   ( eSliceType, iQp, (Short*)INIT_SIG_CG_FLAG );
#endif
#if SIGMAP_CTX_RED
  m_cCUSigSCModelLuma.initBuffer         ( eSliceType, iQp, (Short*)INIT_SIG_FLAG_LUMA );
  m_cCUSigSCModelChroma.initBuffer       ( eSliceType, iQp, (Short*)INIT_SIG_FLAG_CHROMA );
#else
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_SIG_FLAG );
#endif
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST );
#if COEFF_CTXSET_RED
  m_cCUOneSCModelLuma.initBuffer         ( eSliceType, iQp, (Short*)INIT_ONE_FLAG_LUMA );
  m_cCUOneSCModelChroma.initBuffer       ( eSliceType, iQp, (Short*)INIT_ONE_FLAG_CHROMA );
  m_cCUAbsSCModelLuma.initBuffer         ( eSliceType, iQp, (Short*)INIT_ABS_FLAG_LUMA );
  m_cCUAbsSCModelChroma.initBuffer       ( eSliceType, iQp, (Short*)INIT_ABS_FLAG_CHROMA );
#else
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ABS_FLAG );
#endif
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (Short*)INIT_MVP_IDX );
#if SAO
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_SVLC );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
#endif
  m_uiLastDQpNonZero  = 0;
  
  // new structure
  m_uiLastQp          = iQp;
  
  m_pcTDecBinCabac->start();
}

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{
  m_pcTDecBinCabac->decodeBinTrm( ruiBit );
}


Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
  
  m_pcTDecBinCabac->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( ruiSymbol == 0 || uiMaxSymbol == 1 )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinCabac->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont && ( uiSymbol < uiMaxSymbol - 1 ) );
  
  if( uiCont && ( uiSymbol == uiMaxSymbol - 1 ) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  while( uiBit )
  {
    m_pcTDecBinCabac->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  if ( --uiCount )
  {
    UInt bins;
    m_pcTDecBinCabac->decodeBinsEP( bins, uiCount );
    uiSymbol += bins;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcTDecBinCabac->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinCabac->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont );
  
  ruiSymbol = uiSymbol;
}

/** Parsing of coeff_abs_level_minus3
 * \param ruiSymbol reference to coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
Void TDecSbac::xReadGoRiceExGolomb( UInt &ruiSymbol, UInt &ruiGoRiceParam )
{
  Bool bExGolomb    = false;
  UInt uiCodeWord   = 0;
  UInt uiQuotient   = 0;
  UInt uiRemainder  = 0;
  UInt uiMaxVlc     = g_auiGoRiceRange[ ruiGoRiceParam ];
  UInt uiMaxPreLen  = g_auiGoRicePrefixLen[ ruiGoRiceParam ];

  do
  {
    uiQuotient++;
    m_pcTDecBinCabac->decodeBinEP( uiCodeWord );
  }
  while( uiCodeWord && uiQuotient < uiMaxPreLen );

  uiCodeWord  = 1 - uiCodeWord;
  uiQuotient -= uiCodeWord;

  if ( ruiGoRiceParam > 0 )
  {
    m_pcTDecBinCabac->decodeBinsEP( uiRemainder, ruiGoRiceParam );    
  }

  ruiSymbol      = uiRemainder + ( uiQuotient << ruiGoRiceParam );
  bExGolomb      = ruiSymbol == ( uiMaxVlc + 1 );

  if( bExGolomb )
  {
    xReadEpExGolomb( uiCodeWord, 0 );
    ruiSymbol += uiCodeWord;
  }

  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( ruiSymbol, 15 ) ];

  return;
}


/** parse skip flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCtxSkip = pcCU->getCtxSkipFlag( uiAbsPartIdx );
  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tSkipFlag" );
  DTRACE_CABAC_T( "\tuiCtxSkip: ");
  DTRACE_CABAC_V( uiCtxSkip );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
  
  if( uiSymbol )
  {
    pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      pcCU->setMergeFlagSubParts( true , uiAbsPartIdx, 0, uiDepth );
    } 
    else
    {
      TComMv cZeroMv(0,0);
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      
      pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts  ( 0, 0, 0, uiAbsPartIdx, uiDepth );
      
      if ( pcCU->getSlice()->isInterP() )
      {
        pcCU->setInterDirSubParts( 1, uiAbsPartIdx, 0, uiDepth );
        
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
      else
      {
        pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );
        
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
    }
  }
}

/** parse merge flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPUIdx
 * \returns Void
 */
Void TDecSbac::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{
  UInt uiSymbol;
  m_pcTDecBinCabac->decodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiPUIdx, uiDepth );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tMergeFlag: " );
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\tAddress: " );
  DTRACE_CABAC_V( pcCU->getAddr() );
  DTRACE_CABAC_T( "\tuiAbsPartIdx: " );
  DTRACE_CABAC_V( uiAbsPartIdx );
  DTRACE_CABAC_T( "\n" );
}

Void TDecSbac::parseMergeIndex ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiNumCand = MRG_MAX_NUM_CANDS;
  UInt auiCtx[4] = { 0, 1, 2, 3 };
  UInt uiUnaryIdx = 0;
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  uiNumCand = pcCU->getSlice()->getMaxNumMergeCand();
  if ( uiNumCand > 1 )
  {
#endif
    for( ; uiUnaryIdx < uiNumCand - 1; ++uiUnaryIdx )
    {
      UInt uiSymbol = 0;
      m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[uiUnaryIdx] ) );
      if( uiSymbol == 0 )
      {
        break;
      }
    }
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  }
#endif
  ruiMergeIndex = uiUnaryIdx;

  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseMergeIndex()" )
  DTRACE_CABAC_T( "\tuiMRGIdx= " )
  DTRACE_CABAC_V( ruiMergeIndex )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseMVPIdx      ( Int& riMVPIdx )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, AMVP_MAX_NUM_CANDS-1);
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
  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth ) ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );
  
  return;
}

/** parse partition size
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol, uiMode = 0;
  PartSize eMode;
  
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    uiSymbol = 1;
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    }
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;
#if PREDTYPE_CLEANUP  
    UInt uiTrLevel = 0;    
    UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
    UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
    uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
    if( eMode == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1+uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
#endif // PREDTYPE_CLEANUP
  }
  else
  {
#if PREDTYPE_CLEANUP
      UInt uiMaxNumBits = 2;
#if DISABLE_4x4_INTER
      if(!pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
#else
      if( (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
#endif
      {
        uiMaxNumBits ++;
      }
      for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
      {
        m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
        if ( uiSymbol )
        {
          break;
        }
        uiMode++;
      }
      eMode = (PartSize) uiMode;
#if AMP
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eMode == SIZE_2NxN)
        {
          m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          if (uiSymbol == 0)
          {
            m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
            eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
          }
        }
        else if (eMode == SIZE_Nx2N)
        {
          m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          if (uiSymbol == 0)
          {
            m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
            eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
          }
        }
      }
    }
#endif
#else //PREDTYPE_CLEANUP   
    {
      UInt uiMaxNumBits = 3;
#if DISABLE_4x4_INTER
      if(pcCU->getSlice()->getSPS()->getDisInter4x4() && ( (g_uiMaxCUWidth>>uiDepth)==8) && ( (g_uiMaxCUHeight>>uiDepth)==8) )
      {
        uiMaxNumBits = 2;
      }
#endif
      for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
      {
        m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
        if ( uiSymbol )
        {
          break;
        }
        uiMode++;
      }
    }
    eMode = (PartSize) uiMode;
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && ( (g_uiMaxCUWidth>>uiDepth)==8) && ( (g_uiMaxCUHeight>>uiDepth)==8) )
    {
      if (pcCU->getSlice()->isInterB() && uiMode == 2)
      {
        uiSymbol = 0;
        m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 2) );
        if (uiSymbol == 0)
        {
          pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
          if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
          {
            m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
          }
          eMode = SIZE_NxN;
          if (uiSymbol == 0)
            eMode = SIZE_2Nx2N;
        }
      }
    }
    else
    {
#endif    
    if (pcCU->getSlice()->isInterB() && uiMode == 3)
    {
      uiSymbol = 0;
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
      }
      
      if (uiSymbol == 0)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
        {
          m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
        }
        if (uiSymbol == 0)
          eMode = SIZE_2Nx2N;
      }
    }
#if AMP
    if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
    {
      if (eMode == SIZE_2NxN)
      {
        m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
          eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
        m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          m_pcTDecBinCabac->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
          eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
        }
      }
    }
#endif
#if DISABLE_4x4_INTER
    }
#endif
  }
#endif //PREDTYPE_CLEANUP   
  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
#if !PREDTYPE_CLEANUP  
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
#endif // PREDTYPE_CLEANUP
}

/** parse prediction mode
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return;
  }
  
  UInt uiSymbol;
  Int  iPredMode = MODE_INTER;
#if !PREDTYPE_CLEANUP   
  if ( pcCU->getSlice()->isInterB() )
  {
    pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
    return;
  }
  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
#else //!PREDTYPE_CLEANUP
  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
#endif //!PREDTYPE_CLEANUP
  iPredMode += uiSymbol;
  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}
  
Void TDecSbac::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
 
  UInt uiSymbol;
  Int  intraPredMode;

  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  
 
  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
 
  if ( uiSymbol )
  {
#if BYPASS_FOR_INTRA_MODE
    m_pcTDecBinCabac->decodeBinEP( uiSymbol );
#else
    m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2 ) );
#endif
    intraPredMode = uiPreds[uiSymbol];
  }
  else
  {
    intraPredMode = 0;
    
#if BYPASS_FOR_INTRA_MODE
    m_pcTDecBinCabac->decodeBinsEP( uiSymbol, g_aucIntraModeBitsAng[iIntraIdx] - 1 );
    intraPredMode = uiSymbol;
#else
    for ( Int i = 0; i < g_aucIntraModeBitsAng[iIntraIdx] - 1; i++ )
    {
      m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
      intraPredMode |= uiSymbol << i;
    }
#endif
    
    if ( intraPredMode == 31 )
    {
#if BYPASS_FOR_INTRA_MODE
      m_pcTDecBinCabac->decodeBinEP( uiSymbol );
#else
      m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
#endif
      intraPredMode += uiSymbol;      
    }
    
    for ( Int i = 0; i < uiPredNum; i++ )
    {
      intraPredMode += ( intraPredMode >= uiPreds[i] );
    }
  }
  
  pcCU->setLumaIntraDirSubParts( (UChar)intraPredMode, uiAbsPartIdx, uiDepth );
}
Void TDecSbac::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );

  if( uiSymbol == 0 )
  {
    uiSymbol = DM_CHROMA_IDX;
  } 
  else 
  {
    if( pcCU->getSlice()->getSPS()->getUseLMChroma() )
    {
      m_pcTDecBinCabac->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      uiSymbol = 1;
    }

    if( uiSymbol == 0 )
    {
      uiSymbol = LM_CHROMA_IDX;
    } 
    else
    {
      UInt uiIPredMode;
      xReadUnaryMaxSymbol( uiIPredMode, m_cCUChromaPredSCModel.get( 0, 0 ) + 1, 0, 3 );
      UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
      pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );
      uiSymbol = uiAllowedChromaDir[ uiIPredMode ];
    }
  }
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  return;
}

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );
  ContextModel *pCtx = m_cCUInterDirSCModel.get( 0 );
  m_pcTDecBinCabac->decodeBin( uiSymbol, *( pCtx + uiCtx ) );

  if( uiSymbol )
  {
    uiSymbol = 2;
  }
  else if( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 )
  {
    uiSymbol = 0;
  }
  else if( pcCU->getSlice()->getNoBackPredFlag() )
  {
    uiSymbol = 0;
  }
  else
  {
    pCtx++;
    m_pcTDecBinCabac->decodeBin( uiSymbol, *( pCtx + 3 ) );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecSbac::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;

  if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C ) > 0 && eRefList==REF_PIC_LIST_C)
  {
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinCabac->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );
      uiSymbol++;
    }
    riRefFrmIdx = uiSymbol;
  }
  else
  {
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinCabac->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
      uiSymbol++;
    }
    riRefFrmIdx = uiSymbol;
  }

  return;
}

Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  UInt uiHorAbs;
  UInt uiVerAbs;
  UInt uiHorSign = 0;
  UInt uiVerSign = 0;
  ContextModel *pCtx = m_cCUMvdSCModel.get( 0 );

  m_pcTDecBinCabac->decodeBin( uiHorAbs, *pCtx );
  m_pcTDecBinCabac->decodeBin( uiVerAbs, *pCtx );

  const Bool bHorAbsGr0 = uiHorAbs != 0;
  const Bool bVerAbsGr0 = uiVerAbs != 0;
  pCtx++;

  if( bHorAbsGr0 )
  {
    m_pcTDecBinCabac->decodeBin( uiSymbol, *pCtx );
    uiHorAbs += uiSymbol;
  }

  if( bVerAbsGr0 )
  {
    m_pcTDecBinCabac->decodeBin( uiSymbol, *pCtx );
    uiVerAbs += uiSymbol;
  }

  if( bHorAbsGr0 )
  {
    if( 2 == uiHorAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiHorAbs += uiSymbol;
    }

    m_pcTDecBinCabac->decodeBinEP( uiHorSign );
  }

  if( bVerAbsGr0 )
  {
    if( 2 == uiVerAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiVerAbs += uiSymbol;
    }

    m_pcTDecBinCabac->decodeBinEP( uiVerSign );
  }

  const TComMv cMv( uiHorSign ? -Int( uiHorAbs ): uiHorAbs, uiVerSign ? -Int( uiVerAbs ) : uiVerAbs );
#if AMP
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth, uiPartIdx );
#else
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
#endif
  return;
}


Void TDecSbac::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  m_pcTDecBinCabac->decodeBin( ruiSubdivFlag, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiLog2TransformBlockSize ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( ruiSubdivFlag )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiLog2TransformBlockSize )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf )
{
  UInt uiSymbol;
  const UInt uiCtx = 0;
  m_pcTDecBinCabac->decodeBin( uiSymbol , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  uiQtRootCbf = uiSymbol;
}

Void TDecSbac::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
#if CHROMA_CBF_CTX_REDUCTION
  m_pcTDecBinCabac->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? TEXT_CHROMA: eType, uiCtx ) );
#else
  m_pcTDecBinCabac->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );
#endif
  
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( eType )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  pcCU->setCbfSubParts( uiSymbol << uiTrDepth, eType, uiAbsPartIdx, uiDepth );
}

/** Parse (X,Y) position of the last significant coefficient
 * \param uiPosLastX reference to X component of last coefficient
 * \param uiPosLastY reference to Y component of last coefficient
 * \param width  Block width
 * \param height Block height
 * \param eTType plane type / luminance or chrominance
 * \param uiScanIdx scan type (zig-zag, hor, ver)
 *
 * This method decodes the X and Y component within a block of the last significant coefficient.
 */
Void TDecSbac::parseLastSignificantXY( UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, TextType eTType, UInt uiScanIdx )
{
  UInt uiLast;
#if MODIFIED_LAST_XY_CODING
  ContextModel *pCtxX = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY = m_cCuCtxLastY.get( 0, eTType );

  // posX
  const UInt *puiCtxIdxX = g_uiLastCtx + ( g_aucConvertToBit[ width ] * ( g_aucConvertToBit[ width ] + 3 ) );
  for( uiPosLastX = 0; uiPosLastX < g_uiGroupIdx[ width - 1 ]; uiPosLastX++ )
  {
    m_pcTDecBinCabac->decodeBin( uiLast, *( pCtxX + puiCtxIdxX[ uiPosLastX ] ) );
    if( !uiLast )
    {
      break;
    }
  }
#if !BYPASS_FOR_LAST_COEFF_MOD
  if ( uiPosLastX > 3 )
  {
    UInt uiTemp  = 0;
    UInt uiCount = ( uiPosLastX - 2 ) >> 1;
    for ( Int i = uiCount - 1; i >= 0; i-- )
    {
      m_pcTDecBinCabac->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastX = g_uiMinInGroup[ uiPosLastX ] + uiTemp;
  }
#endif

  // posY
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ height ] * ( g_aucConvertToBit[ height ] + 3 ) );
  for( uiPosLastY = 0; uiPosLastY < g_uiGroupIdx[ height - 1 ]; uiPosLastY++ )
  {
    m_pcTDecBinCabac->decodeBin( uiLast, *( pCtxY + puiCtxIdxY[ uiPosLastY ] ) );
    if( !uiLast )
    {
      break;
    }
  }
#if BYPASS_FOR_LAST_COEFF_MOD
  if ( uiPosLastX > 3 )
  {
    UInt uiTemp  = 0;
    UInt uiCount = ( uiPosLastX - 2 ) >> 1;
    for ( Int i = uiCount - 1; i >= 0; i-- )
    {
      m_pcTDecBinCabac->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastX = g_uiMinInGroup[ uiPosLastX ] + uiTemp;
  }
#endif
  if ( uiPosLastY > 3 )
  {
    UInt uiTemp  = 0;
    UInt uiCount = ( uiPosLastY - 2 ) >> 1;
    for ( Int i = uiCount - 1; i >= 0; i-- )
    {
      m_pcTDecBinCabac->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastY = g_uiMinInGroup[ uiPosLastY ] + uiTemp;
  }
#else
  const UInt *puiCtxIdx;
  
  Int minWidth    = min<Int>( 4, width );
  Int minHeight   = min<Int>( 4, height );
  Int halfWidth   = width >> 1;
  Int halfHeight  = height >> 1;
  Int log2BlkWidth  = g_aucConvertToBit[ halfWidth  ] + 2;
  Int log2BlkHeight = g_aucConvertToBit[ halfHeight ] + 2;
  Int maxWidth    = max<Int>( minWidth,  halfWidth  + 1 );
  Int maxHeight   = max<Int>( minHeight, halfHeight + 1 );
  ContextModel *pCtxX      = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY      = m_cCuCtxLastY.get( 0, eTType );

  // posX
  puiCtxIdx    = g_uiLastCtx + ( halfWidth >= minWidth ? halfWidth : 0 );
  
  for( uiPosLastX = 0; uiPosLastX < maxWidth-1; uiPosLastX++ )
  {
    m_pcTDecBinCabac->decodeBin( uiLast, *( pCtxX + puiCtxIdx[ uiPosLastX ] ) );

    if( uiLast )
    {
      break;
    }
  }

#if BYPASS_FOR_LAST_COEFF_MOD
  Int lastX = uiLast;
#else
  if( !uiLast && halfWidth >= minWidth )
  {
    UInt uiCount = 0, uiTemp = 0;

    while( uiCount < log2BlkWidth )
    {
      m_pcTDecBinCabac->decodeBinEP( uiLast );
      uiTemp += uiLast << uiCount;
      uiCount++;
    }
    uiPosLastX += uiTemp;
  }
#endif
  
  // posY
  puiCtxIdx    = g_uiLastCtx + ( halfHeight >= minHeight ? halfHeight : 0 );
  
  for( uiPosLastY = 0; uiPosLastY < maxHeight-1; uiPosLastY++ )
  {
    m_pcTDecBinCabac->decodeBin( uiLast, *( pCtxY + puiCtxIdx[ uiPosLastY ] ) );

    if( uiLast )
    {
      break;
    }
  }

#if !BYPASS_FOR_LAST_COEFF_MOD
  if( !uiLast && halfHeight >= minHeight )
  {
    UInt uiCount = 0, uiTemp = 0;

    while( uiCount < log2BlkHeight )
    {
      m_pcTDecBinCabac->decodeBinEP( uiLast );
      uiTemp += uiLast << uiCount;
      uiCount++;
    }
    uiPosLastY += uiTemp;
  }
#else
  if ( !lastX && halfWidth >= minWidth )
  {
    UInt temp;
    m_pcTDecBinCabac->decodeBinsEP( temp, log2BlkWidth );
    uiPosLastX += temp;
  }
  if ( !uiLast && halfHeight >= minHeight )
  {
    UInt temp;
    m_pcTDecBinCabac->decodeBinsEP( temp, log2BlkHeight );
    uiPosLastY += temp;
  }
#endif
#endif
  
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosLastX, uiPosLastY );
  }
}

Void TDecSbac::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
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
  
#if NSQT_MOD && !NSQT_DIAG_SCAN
  Int orgWidth = uiWidth;
  Int orgHeight = uiHeight;
  if (orgWidth != orgHeight)
  {
    int log2BlkSize = 2 + ( ( g_aucConvertToBit[ orgWidth ] + g_aucConvertToBit [ orgHeight ] ) >> 1 );
    uiWidth = 1 << log2BlkSize;
    uiHeight = 1 << log2BlkSize;
  }
#endif
  
  if( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- parse significance map -----
  const UInt  uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt  uiMaxNumCoeff     = uiWidth * uiHeight;
  const UInt  uiMaxNumCoeffM1   = uiMaxNumCoeff - 1;
#if DIAG_SCAN
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#else
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif
#if NSQT_DIAG_SCAN
  int blockType = uiLog2BlockSize;
  if (uiWidth != uiHeight)
  {
    uiScanIdx = SCAN_DIAG;
    blockType = 4;
  }
#endif
#if !UNIFIED_SCAN_PASSES
  Int sigCoeffCount = 0;
#endif
  
  //===== decode last significant =====
  UInt uiPosLastX, uiPosLastY;
#if NSQT_DIAG_SCAN
  parseLastSignificantXY( uiPosLastX, uiPosLastY, uiWidth, uiHeight, eTType, uiScanIdx );
#else
  parseLastSignificantXY( uiPosLastX, uiPosLastY, uiWidth, uiWidth, eTType, uiScanIdx );
#endif
  UInt uiBlkPosLast      = uiPosLastX + (uiPosLastY<<uiLog2BlockSize);
  pcCoef[ uiBlkPosLast ] = 1;
#if !UNIFIED_SCAN_PASSES
  sigCoeffCount++;
#endif

  //===== decode significance flags =====
  UInt uiScanPosLast   = uiBlkPosLast;
#if SUBBLOCK_SCAN
  uiScanIdx = ( uiScanIdx == SCAN_ZIGZAG ) ? SCAN_DIAG : uiScanIdx; // Map zigzag to diagonal scan
#if NSQT_DIAG_SCAN
  const UInt * scan;
  if (uiWidth == uiHeight)
  {
    scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize-1 ];
  }
  else
  {
    scan = g_sigScanNSQT[ uiLog2BlockSize - 2 ];
  }
#else
  const UInt * const scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize-1 ];
#endif
  for( uiScanPosLast = 0; uiScanPosLast < uiMaxNumCoeffM1; uiScanPosLast++ )
  {
    UInt uiBlkPos = scan[ uiScanPosLast ];
    if( uiBlkPosLast == uiBlkPos )
    {
      break;
    }
  }
#else
  if( uiScanIdx == SCAN_ZIGZAG )
  {
    UInt uiD         = uiPosLastY + uiPosLastX;
    if( uiD < uiWidth )
    {
#if DIAG_SCAN
      uiScanPosLast  = uiPosLastX + (( uiD * ( uiD + 1 ) ) >> 1);
#else
      uiScanPosLast  = ( uiD * ( uiD + 1 ) ) >> 1;
      uiScanPosLast += uiD % 2 ? uiPosLastY : uiPosLastX;
#endif
    } 
    else
    {
      UInt uiDI      = ( (uiWidth-1) << 1 ) - uiD;
#if DIAG_SCAN
      uiScanPosLast  = uiMaxNumCoeffM1 - ( uiDI * ( uiDI + 1 ) >> 1 ) - uiWidth + 1 + uiPosLastX;
#else
      uiScanPosLast  = uiMaxNumCoeffM1 - ( uiDI * ( uiDI + 1 ) >> 1 );
      uiScanPosLast -= uiDI % 2 ? uiWidth - 1 - uiPosLastY : uiWidth - 1 - uiPosLastX;
#endif
    }
  }
  else if( uiScanIdx == SCAN_VER )
  {
    uiScanPosLast = uiPosLastY + (uiPosLastX<<uiLog2BlockSize);
  }
#endif

#if !SUBBLOCK_SCAN
#if DIAG_SCAN
  uiScanIdx = ( uiScanIdx == SCAN_ZIGZAG ) ? SCAN_DIAG : uiScanIdx; // Map zigzag to diagonal scan
#endif
  
  const UInt * const scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize-1 ];
#endif

#if MULTI_LEVEL_SIGNIFICANCE
  ContextModel * const baseCoeffGroupCtx = m_cCUSigCoeffGroupSCModel.get( 0, eTType );
#endif
#if SIGMAP_CTX_RED
  ContextModel * const baseCtx = (eTType==TEXT_LUMA) ? m_cCUSigSCModelLuma.get( 0, 0 ) : m_cCUSigSCModelChroma.get( 0, 0 );
#else
  ContextModel * const baseCtx = m_cCUSigSCModel.get( 0, eTType );
#endif
#if !UNIFIED_SCAN_PASSES
#if MULTI_LEVEL_SIGNIFICANCE
  const UInt * const scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0  ];
  if( uiLog2BlockSize > 3 )
  {
    UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
    ::memset( uiSigCoeffGroupFlag, 0, sizeof(UInt) * MLS_GRP_NUM );
    
    static const UInt uiShift  = MLS_CG_SIZE >> 1;    // 2
    static const UInt uiCGSize = (1 << MLS_CG_SIZE);  // 16
    const UInt uiNumBlkSide = uiWidth >> uiShift;     // uiWidth / 4
    
    // decode significant coefficient group flag
    Int iCGLastScanPos = uiScanPosLast >> MLS_CG_SIZE;
    for( Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos -- )
    {
      Int iCGBlkPos = scanCG[ iCGScanPos ];
      Int iCGPosY   = iCGBlkPos / uiNumBlkSide;
      Int iCGPosX   = iCGBlkPos - (iCGPosY * uiNumBlkSide);
      Bool bInferredCGFlag = false;

      if( iCGScanPos == iCGLastScanPos ) 
      {
        uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
      }
      else
      {
#if NSQT_DIAG_SCAN
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight ) && ( iCGScanPos ) )
#else
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize ) && ( iCGScanPos ) )
#endif
        {
          UInt uiSigCoeffGroup;
#if NSQT_DIAG_SCAN
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight );
#else
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize );
#endif
          m_pcTDecBinCabac->decodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
          uiSigCoeffGroupFlag[ iCGBlkPos ] = uiSigCoeffGroup;
        }
        else
        {
          uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
          bInferredCGFlag = true;
        }
      }
    
      // decode significant coefficient flag       
      if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
      {
        Int  iScanPos;
        UInt uiBlkPos, uiPosY, uiPosX, uiSig, uiCtxSig;
        UInt uiNumNonZeroesInCG = (iCGScanPos == iCGLastScanPos)? 1 : 0;

        for( Int iScanPosOffset = (iCGScanPos == iCGLastScanPos)? uiScanPosLast - iCGScanPos*uiCGSize - 1 : uiCGSize - 1; 
          iScanPosOffset >= 0; iScanPosOffset-- )
        {
          iScanPos  = iCGScanPos*uiCGSize + iScanPosOffset;
          uiBlkPos  = scan[ iScanPos ];
          uiPosY    = uiBlkPos >> uiLog2BlockSize;
          uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
       
          if (iScanPosOffset > 0 || bInferredCGFlag || uiNumNonZeroesInCG )
          {
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
            uiCtxSig = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
            uiCtxSig = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
            uiCtxSig = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, eTType );
#else
            uiCtxSig = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
#endif
#endif
            m_pcTDecBinCabac->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
            uiNumNonZeroesInCG += uiSig;                  
            pcCoef[ uiBlkPos ] = uiSig;
            sigCoeffCount += uiSig;
          }
          else
          {
            uiSig = 1;
            pcCoef[ uiBlkPos ] = uiSig;
            sigCoeffCount += uiSig;
          }
        }
      } 
      else
      {
        for( Int iScanPosOffset = uiCGSize - 1; iScanPosOffset >= 0; iScanPosOffset-- )
        {
          Int  iScanPos = iCGScanPos*uiCGSize + iScanPosOffset;
          UInt uiBlkPos = scan[ iScanPos ];
          pcCoef[ uiBlkPos ] = 0;
        }
      } // end if ( uiSigCoeffGroupFlag[ iCGBlkPos ] )
    } // end for( Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos -- )
  }
  else
#endif
  {
    for( UInt uiScanPos = uiScanPosLast-1; uiScanPos != -1; uiScanPos-- )
    {
      UInt uiBlkPos = scan[ uiScanPos ];
      UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
      UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
      UInt  uiSig     = 0;
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
      UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
      UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
      UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, eTType );
#else
      UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
#endif
#endif
      m_pcTDecBinCabac->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
      pcCoef[ uiBlkPos ] = uiSig;
      sigCoeffCount += uiSig;
    }
  }
#endif

  const Int  iLastScanSet      = uiScanPosLast >> LOG2_SCAN_SET_SIZE;
  UInt uiNumOne                = 0;
  UInt uiGoRiceParam           = 0;

#if UNIFIED_SCAN_PASSES
#if MULTI_LEVEL_SIGNIFICANCE
  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
  ::memset( uiSigCoeffGroupFlag, 0, sizeof(UInt) * MLS_GRP_NUM );
  const UInt uiNumBlkSide = uiWidth >> (MLS_CG_SIZE >> 1);
#if NSQT_DIAG_SCAN
  const UInt * scanCG;
  if (uiWidth == uiHeight)
  {
    scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0  ];    
  }
  else
  {
    scanCG = g_sigCGScanNSQT[ uiLog2BlockSize - 2 ];
  }
#else
  const UInt * const scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0  ];
#endif
#endif
  Int  iScanPosSig             = (Int) uiScanPosLast;
  for( Int iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- )
#else
  for( Int iSubSet = iLastScanSet; iSubSet >= 0 && sigCoeffCount > 0; iSubSet-- )
#endif
  {
    Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
    uiGoRiceParam    = 0;
    Int numNonZero = 0;
    
#if UNIFIED_SCAN_PASSES
    Int pos[SCAN_SET_SIZE];
    if( iScanPosSig == (Int) uiScanPosLast )
    {
      iScanPosSig--;
      pos[ numNonZero ] = uiBlkPosLast;
      numNonZero = 1;
    }

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
    if( blockType > 3 )
#else
    if( uiLog2BlockSize > 3 )
#endif
    {
      // decode significant_coeffgroup_flag
      Int iCGBlkPos = scanCG[ iSubSet ];
      Int iCGPosY   = iCGBlkPos / uiNumBlkSide;
      Int iCGPosX   = iCGBlkPos - (iCGPosY * uiNumBlkSide);
      Bool bInferredCGFlag = false;

      if( iSubSet == iLastScanSet ) 
      {
        uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
      }
      else
      {
#if NSQT_DIAG_SCAN
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight) && ( iSubSet ) )
#else
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize) && ( iSubSet ) )
#endif
        {
          UInt uiSigCoeffGroup;
#if NSQT_DIAG_SCAN
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight );
#else
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize );
#endif
          m_pcTDecBinCabac->decodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
          uiSigCoeffGroupFlag[ iCGBlkPos ] = uiSigCoeffGroup;
        }
        else
        {
          uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
          bInferredCGFlag = true;
        }
      }

      // decode significant_coeff_flag
      UInt uiBlkPos, uiPosY, uiPosX, uiSig, uiCtxSig;
      for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
      {
        uiBlkPos  = scan[ iScanPosSig ];
        uiPosY    = uiBlkPos >> uiLog2BlockSize;
        uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
        uiSig     = 0;
        
        if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
        {
          if( iScanPosSig > iSubPos || bInferredCGFlag || numNonZero )
          {
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
            uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
            uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
            uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, eTType );
#else
            uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
#endif
#endif
            m_pcTDecBinCabac->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
          }
          else
          {
            uiSig = 1;
          }
        }
        pcCoef[ uiBlkPos ] = uiSig;
        if( uiSig )
        {
          pos[ numNonZero ] = uiBlkPos;
          numNonZero ++;
        }
      }
    }
    else
#endif
    {
      for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
      {
        UInt uiBlkPos   = scan[ iScanPosSig ];
        UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
        UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
        UInt  uiSig     = 0;
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
#else
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight );
#endif
#else
#if SIGMAP_CTX_RED
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, eTType );
#else
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
#endif
#endif
        m_pcTDecBinCabac->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
        pcCoef[ uiBlkPos ] = uiSig;
        if( uiSig )
        {
          pos[ numNonZero ] = uiBlkPos;
          numNonZero ++;
        }
      }
    }
#else
    const UInt *puiSetScan  = scan + iSubPos;
    Int pos[SCAN_SET_SIZE];
    
    for( Int iScanPos = SCAN_SET_SIZE-1; iScanPos >= 0; iScanPos-- )
    {
      UInt uiBlkPos = puiSetScan[ iScanPos ];
      pos[ numNonZero ] = uiBlkPos;
      numNonZero += pcCoef[ uiBlkPos ] != 0;
    }
#endif
    
    if( numNonZero )
    {
      UInt c1 = 1;
      UInt c2 = 0;
#if COEFF_CTXSET_RED
      UInt uiCtxSet    = (iSubSet > 0 && eTType==TEXT_LUMA) ? 3 : 0;
#else
      UInt uiCtxSet = iSubSet > 0 ? 3 : 0;
#endif
      UInt uiBin;
      
      if( uiNumOne > 0 )
      {
        uiCtxSet++;
#if COEFF_CTXSET_RED
        if(eTType==TEXT_LUMA && uiNumOne > 3)
#else
        if( uiNumOne > 3 )
#endif
        {
          uiCtxSet++;
        }
      }
      
#if !UNIFIED_SCAN_PASSES
      sigCoeffCount -= numNonZero;
#endif
      uiNumOne       >>= 1;
#if COEFF_CTXSET_RED
#if COEFF_CTX_RED
      ContextModel *baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUOneSCModelLuma.get( 0, 0 ) + 4 * uiCtxSet : m_cCUOneSCModelChroma.get( 0, 0 ) + 4 * uiCtxSet;
#else
      ContextModel *baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUOneSCModelLuma.get( 0, 0 ) + 5 * uiCtxSet : m_cCUOneSCModelChroma.get( 0, 0 ) + 5 * uiCtxSet;
#endif 
#else
#if COEFF_CTX_RED
      ContextModel *baseCtxMod = m_cCUOneSCModel.get( 0, eTType ) + 4 * uiCtxSet;
#else
      ContextModel *baseCtxMod = m_cCUOneSCModel.get( 0, eTType ) + 5 * uiCtxSet;
#endif      
#endif      
      Int absCoeff[SCAN_SET_SIZE];
      for( Int idx = 0; idx < numNonZero; idx++ )
      {
        m_pcTDecBinCabac->decodeBin( uiBin, baseCtxMod[c1] );
        if( uiBin == 1 )
        {
          c1 = 0;
        }
#if COEFF_CTX_RED
        else if( (c1 < 3) && (c1 > 0) )
#else
        else if( c1 & 3 )
#endif
        {
          c1++;
        }
        absCoeff[ idx ] = uiBin + 1;
      }
      
      if (c1 == 0)
      {
#if COEFF_CTXSET_RED
#if COEFF_CTX_RED
        baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUAbsSCModelLuma.get( 0, 0 ) + 3 * uiCtxSet : m_cCUAbsSCModelChroma.get( 0, 0 ) + 3 * uiCtxSet;
#else
        baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUAbsSCModelLuma.get( 0, 0 ) + 5 * uiCtxSet : m_cCUAbsSCModelChroma.get( 0, 0 ) + 5 * uiCtxSet;
#endif 
#else
#if COEFF_CTX_RED
        baseCtxMod = m_cCUAbsSCModel.get( 0, eTType ) + 3 * uiCtxSet;
#else
        baseCtxMod = m_cCUAbsSCModel.get( 0, eTType ) + 5 * uiCtxSet;
#endif
#endif
        for( Int idx = 0; idx < numNonZero; idx++ )
        {
          if( absCoeff[ idx ] == 2 ) 
          {
            m_pcTDecBinCabac->decodeBin( uiBin, baseCtxMod[c2] );
            absCoeff[ idx ] = uiBin + 2;
#if COEFF_CTX_RED
            c2 += (c2 < 2);
#else
            c2 += (c2 < 4);
#endif
            uiNumOne++;
          }
        }
      }
      
      UInt coeffSigns;
      m_pcTDecBinCabac->decodeBinsEP( coeffSigns, numNonZero );
      coeffSigns <<= 32 - numNonZero;
      
      if (c1 == 0)
      {
        for( Int idx = 0; idx < numNonZero; idx++ )
        {
          if( absCoeff[ idx ] == 3 )
          {
            UInt uiLevel;
            xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
            absCoeff[ idx ] = uiLevel + 3;
          }
        }
      }
      
      for( Int idx = 0; idx < numNonZero; idx++ )
      {
        Int blkPos = pos[ idx ];
        Int sign = static_cast<Int>( coeffSigns ) >> 31;
        pcCoef[ blkPos ] = ( absCoeff[ idx ] ^ sign ) - sign;
        coeffSigns <<= 1;
      }
    }
    else
    {
      uiNumOne >>= 1;
    }
  }
  
#if NSQT_MOD && !NSQT_DIAG_SCAN
  if (orgHeight != orgWidth)
  {
    TCoeff  orgCoeff[ 256 ];
#if NSQT_TX_ORDER
    Int tableIdx = ( orgWidth * orgHeight ) == 64 ? 2 * ( orgHeight > orgWidth ) : 2 * ( orgHeight > orgWidth ) + 1;
#else
    Int tableIdx = ( orgWidth * orgHeight ) == 64 ? 0 : 1;
#endif
    memcpy( &orgCoeff[0], pcCoef, orgWidth * orgHeight * sizeof( TCoeff ) ); 
    for( Int scanPos = 0; scanPos < orgWidth * orgHeight; scanPos++ )
    {
      Int blkPos = g_auiNonSquareSigLastScan[ tableIdx ][ scanPos ];
      pcCoef[ blkPos ] = orgCoeff[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ scanPos ] ];
    }
  }
#endif
  return;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if ENC_DEC_TRACE

#define READ_CODE(length, code, name)     xReadCodeTr ( length, code, name )
#define READ_UVLC(        code, name)     xReadUvlcTr (         code, name )
#define READ_SVLC(        code, name)     xReadSvlcTr (         code, name )
#define READ_FLAG(        code, name)     xReadFlagTr (         code, name )

Void  xTraceSPSHeader (TComSPS *pSPS)
{
  fprintf( g_hTrace, "=========== Sequence Parameter Set ID: %d ===========\n", pSPS->getSPSId() );
}

Void  xTracePPSHeader (TComPPS *pPPS)
{
  fprintf( g_hTrace, "=========== Picture Parameter Set ID: %d ===========\n", pPPS->getPPSId() );
}

Void  xTraceSliceHeader (TComSlice *pSlice)
{
  fprintf( g_hTrace, "=========== Slice ===========\n");
}


Void  TDecSbac::xReadCodeTr           (UInt length, UInt& rValue, const Char *pSymbolName)
{
  xReadCode (length, rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, rValue ); 
}

Void  TDecSbac::xReadUvlcTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadUvlc (rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, rValue ); 
}

Void  TDecSbac::xReadSvlcTr           (Int& rValue, const Char *pSymbolName)
{
  xReadSvlc(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, rValue ); 
}

Void  TDecSbac::xReadFlagTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadFlag(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, rValue ); 
}

#else

#define READ_CODE(length, code, name)     xReadCode ( length, code )
#define READ_UVLC(        code, name)     xReadUvlc (         code )
#define READ_SVLC(        code, name)     xReadSvlc (         code )
#define READ_FLAG(        code, name)     xReadFlag (         code )

#endif
/**
 * unmarshal a sequence of SEI messages from bitstream.
 */
void TDecSbac::parseSEI(SEImessages& seis)
{
  assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  do
  {
    parseSEImessage(*m_pcBitstream, seis);
    /* SEI messages are an integer number of bytes, something has failed
     * in the parsing if bitstream not byte-aligned */
    assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  } while (0x80 != m_pcBitstream->peekBits(8));
  assert(m_pcBitstream->getNumBitsLeft() == 8); /* rsbp_trailing_bits */
}

#if G1002_RPS
#if INTER_RPS_PREDICTION
void TDecSbac::parseShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* pcRPS, Int idx )
#else
void TDecSbac::parseShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* pcRPS )
#endif
{
  UInt uiCode;
#if INTER_RPS_PREDICTION
  pcRPS->create(pcPPS->getSPS()->getMaxNumberOfReferencePictures(), pcPPS->getSPS()->getMaxNumberOfReferencePictures()+1);
  UInt uiInterRPSPred;
  READ_FLAG(uiInterRPSPred, "inter_RPS_flag");  pcRPS->setInterRPSPrediction(uiInterRPSPred);
  if (uiInterRPSPred) 
  {
    UInt uiBit;
    READ_UVLC(uiCode, "delta_idx_minus1" ); // delta index of the Reference Picture Set used for prediction minus 1
    Int rIdx =  idx - 1 - uiCode;
    assert (rIdx <= idx && rIdx >= 0);
    TComReferencePictureSet*   pcRPSRef = pcPPS->getRPSList()->getReferencePictureSet(rIdx);
    Int k = 0, k0 = 0, k1 = 0;
    READ_CODE(1, uiBit, "delta_rps_sign"); // delta_RPS_sign
    READ_UVLC(uiCode, "abs_delta_rps_minus1");  // absolute delta RPS minus 1
    Int deltaRPS = (1 - (uiBit<<1)) * (uiCode + 1); // delta_RPS
    for(Int j=0 ; j <= pcRPSRef->getNumberOfPictures(); j++)
    {
      READ_CODE(1, uiBit, "ref_idc0" ); //first bit is "1" if Idc is 1 
      Int refIdc = uiBit;
      if (refIdc == 0) 
      {
        READ_CODE(1, uiBit, "ref_idc1" ); //second bit is "1" if Idc is 2, "0" otherwise.
        refIdc = uiBit<<1; //second bit is "1" if refIdc is 2, "0" if refIdc = 0.
      }
      if (refIdc == 1 || refIdc == 2)
      {
        Int deltaPOC = deltaRPS + ((j < pcRPSRef->getNumberOfPictures())? pcRPSRef->getDeltaPOC(j) : 0);
        pcRPS->setDeltaPOC(k, deltaPOC);
        pcRPS->setUsed(k, (refIdc == 1));

        if (deltaPOC < 0) {
          k0++;
        }
        else 
        {
          k1++;
        }
        k++;
      }  
      pcRPS->setRefIdc(j,refIdc);  
    }
    pcRPS->setNumRefIdc(pcRPSRef->getNumberOfPictures()+1);  
    pcRPS->setNumberOfPictures(k);
    pcRPS->setNumberOfNegativePictures(k0);
    pcRPS->setNumberOfPositivePictures(k1);
    pcRPS->sortDeltaPOC();
  }
  else
  {
#else
    pcRPS->create(pcPPS->getSPS()->getMaxNumberOfReferencePictures());
#endif //INTER_RPS_PREDICTION
    READ_UVLC(uiCode, "num_negative_pics");           pcRPS->setNumberOfNegativePictures(uiCode);
    READ_UVLC(uiCode, "num_positive_pics");           pcRPS->setNumberOfPositivePictures(uiCode);
    Int prev = 0;
    Int poc;
    for(Int j=0 ; j < pcRPS->getNumberOfNegativePictures(); j++)
    {
      READ_UVLC(uiCode, "delta_poc_s0_minus1");
      poc = prev-uiCode-1;
      prev = poc;
      pcRPS->setDeltaPOC(j,poc);
      READ_FLAG(uiCode, "used_by_curr_pic_s0_flag");  pcRPS->setUsed(j,uiCode);
    }
    prev = 0;
    for(Int j=pcRPS->getNumberOfNegativePictures(); j < pcRPS->getNumberOfNegativePictures()+pcRPS->getNumberOfPositivePictures(); j++)
    {
      READ_UVLC(uiCode, "delta_poc_s1_minus1");
      poc = prev+uiCode+1;
      prev = poc;
      pcRPS->setDeltaPOC(j,poc);
      READ_FLAG(uiCode, "used_by_curr_pic_s1_flag");  pcRPS->setUsed(j,uiCode);
    }
    pcRPS->setNumberOfPictures(pcRPS->getNumberOfNegativePictures()+pcRPS->getNumberOfPositivePictures());
#if INTER_RPS_PREDICTION
  }
#endif // INTER_RPS_PREDICTION   
  pcRPS->printDeltaPOC();
}
#endif

#if F747_APS
Void TDecSbac::parseAPSInitInfo(TComAPS& cAPS)
{
  UInt uiCode;
  //aps ID
  xReadUvlc(uiCode);      cAPS.setAPSID(uiCode);
  //SAO flag
  xReadFlag(uiCode);      assert( uiCode == 0 );
  //ALF flag
  xReadFlag(uiCode);      assert( uiCode==0 );
}
#endif



Void TDecSbac::parsePPS(TComPPS* pcPPS)
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
  UInt  uiCode;
#if G1002_RPS
  TComRPS* pcRPSList = pcPPS->getRPSList();
#endif
  READ_UVLC( uiCode, "pic_parameter_set_id");                      pcPPS->setPPSId (uiCode);
  READ_UVLC( uiCode, "seq_parameter_set_id");                      pcPPS->setSPSId (uiCode);
#if G1002_RPS
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD
  TComReferencePictureSet*      pcRPS;

  READ_UVLC( uiCode, "num_short_term_ref_pic_sets" );
  pcRPSList->create(uiCode);

  for(UInt i=0; i< pcRPSList->getNumberOfReferencePictureSets(); i++)
  {
    pcRPS = pcRPSList->getReferencePictureSet(i);
#if INTER_RPS_PREDICTION
    parseShortTermRefPicSet(pcPPS,pcRPS,i);
#else
    parseShortTermRefPicSet(pcPPS,pcRPS);
#endif
  }
  READ_FLAG( uiCode, "long_term_ref_pics_present_flag" );          pcPPS->setLongTermRefsPresent(uiCode);
#endif
  // entropy_coding_mode_flag
#if OL_USE_WPP
  // We code the entropy_coding_mode_flag, it's needed for tests.
  READ_FLAG( uiCode, "entropy_coding_mode_flag" );                 assert( uiCode == 1 );
    READ_UVLC( uiCode, "entropy_coding_synchro" );                 assert( uiCode == 0 );
    READ_FLAG( uiCode, "cabac_istate_reset" );                     assert( uiCode == 0 );
#endif
  READ_UVLC( uiCode, "num_temporal_layer_switching_point_flags" ); assert(uiCode == 0);
  
  // num_ref_idx_l0_default_active_minus1
  // num_ref_idx_l1_default_active_minus1
  // pic_init_qp_minus26  /* relative to 26 */
  READ_FLAG( uiCode, "constrained_intra_pred_flag" );              pcPPS->setConstrainedIntraPred( uiCode ? true : false );
#if FINE_GRANULARITY_SLICES
  READ_CODE( 2, uiCode, "slice_granularity" );                     assert(uiCode == 0);
#endif

#if !F747_APS
  READ_FLAG( uiCode, "shared_pps_info_enabled_flag" );             pcPPS->setSharedPPSInfoEnabled( uiCode ? true : false);
#endif
  // alf_param() ?

#if WEIGHT_PRED
  READ_FLAG( uiCode, "weighted_pred_flag" );          // Use of Weighting Prediction (P_SLICE)
  assert( uiCode==0 );
  READ_CODE( 2, uiCode, "weighted_bipred_idc" );      // Use of Bi-Directional Weighting Prediction (B_SLICE)
  assert( uiCode==0 );
  printf("TDecSbac::parsePPS():\tm_bUseWeightPred=%d\tm_uiBiPredIdc=%d\n", 0, 0);
#endif

#if TILES
  READ_FLAG ( uiCode, "tile_info_present_flag" );
  assert(uiCode == 0);
#endif
  return;
}

Void TDecSbac::parseSPS(TComSPS* pcSPS)
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  
  UInt  uiCode;

  READ_CODE( 8,  uiCode, "profile_idc" );                        pcSPS->setProfileIdc( uiCode );
  READ_CODE( 8,  uiCode, "reserved_zero_8bits" );
  READ_CODE( 8,  uiCode, "level_idc" );                          pcSPS->setLevelIdc( uiCode );
  READ_UVLC(     uiCode, "seq_parameter_set_id" );               pcSPS->setSPSId( uiCode );
  READ_CODE( 3,  uiCode, "max_temporal_layers_minus1" );         pcSPS->setMaxTLayers( uiCode+1 );
  READ_CODE( 16, uiCode, "pic_width_in_luma_samples" );          pcSPS->setWidth       ( uiCode    );
  READ_CODE( 16, uiCode, "pic_height_in_luma_samples" );         pcSPS->setHeight      ( uiCode    );
  //READ_UVLC ( uiCode, "pic_width_in_luma_samples" ); pcSPS->setWidth       ( uiCode    );
  //READ_UVLC ( uiCode, "pic_height_in_luma_samples" ); pcSPS->setHeight      ( uiCode    );
  READ_UVLC(     uiCode, "bit_depth_luma_minus8" );
  assert(uiCode == 0);
  
  g_uiBASE_MAX  = ((1<<8)-1);
  
#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX  = g_uiBASE_MAX;
#else
  g_uiIBDI_MAX  = ((1<<8)-1);
#endif
  READ_UVLC( uiCode,    "bit_depth_chroma_minus8" );
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
  READ_CODE( 4, uiCode, "pcm_bit_depth_luma_minus1" );           assert(uiCode == 7);
  READ_CODE( 4, uiCode, "pcm_bit_depth_chroma_minus1" );         assert(uiCode == 7);
#endif
#if G1002_RPS
  READ_UVLC( uiCode,    "log2_max_pic_order_cnt_lsb_minus4" );   pcSPS->setBitsForPOC( 4 + uiCode );
  READ_UVLC( uiCode,    "max_num_ref_pics" );                    pcSPS->setMaxNumberOfReferencePictures(uiCode);
  READ_UVLC( uiCode,    "max_num_reorder_pics" );                pcSPS->setMaxNumberOfReorderPictures(uiCode);
#endif
#if DISABLE_4x4_INTER
  xReadFlag( uiCode ); pcSPS->setDisInter4x4( uiCode ? true : false );
#endif
#if !G1002_RPS
  // log2_max_frame_num_minus4
  // pic_order_cnt_type
  // if( pic_order_cnt_type  = =  0 )
  //   log2_max_pic_order_cnt_lsb_minus4
  // else if( pic_order_cnt_type  = =  1 ) {
  //   delta_pic_order_always_zero_flag
  //   offset_for_non_ref_pic
  //   num_ref_frames_in_pic_order_cnt_cycle
  //   for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
  //     offset_for_ref_frame[ i ]
  //  }
  // max_num_ref_frames
  // gaps_in_frame_num_value_allowed_flag
#endif
  READ_UVLC( uiCode, "log2_min_coding_block_size_minus3" );
  UInt log2MinCUSize = uiCode + 3;
  READ_UVLC( uiCode, "log2_diff_max_min_coding_block_size" );
  UInt uiMaxCUDepthCorrect = uiCode;
  pcSPS->setMaxCUWidth  ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUWidth  = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  pcSPS->setMaxCUHeight ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUHeight = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  READ_UVLC( uiCode, "log2_min_transform_block_size_minus2" );   pcSPS->setQuadtreeTULog2MinSize( uiCode + 2 );
  READ_UVLC( uiCode, "log2_diff_max_min_transform_block_size" ); pcSPS->setQuadtreeTULog2MaxSize( uiCode + pcSPS->getQuadtreeTULog2MinSize() );
  pcSPS->setMaxTrSize( 1<<(uiCode + pcSPS->getQuadtreeTULog2MinSize()) );
  READ_UVLC( uiCode, "log2_min_pcm_coding_block_size_minus3" );  assert(uiCode+3 == 7); 
  READ_UVLC( uiCode, "max_transform_hierarchy_depth_inter" );    pcSPS->setQuadtreeTUMaxDepthInter( uiCode+1 );
  READ_UVLC( uiCode, "max_transform_hierarchy_depth_intra" );    pcSPS->setQuadtreeTUMaxDepthIntra( uiCode+1 );
  g_uiAddCUDepth = 0;
  while( ( pcSPS->getMaxCUWidth() >> uiMaxCUDepthCorrect ) > ( 1 << ( pcSPS->getQuadtreeTULog2MinSize() + g_uiAddCUDepth )  ) ) g_uiAddCUDepth++;    
  pcSPS->setMaxCUDepth( uiMaxCUDepthCorrect+g_uiAddCUDepth  ); g_uiMaxCUDepth  = uiMaxCUDepthCorrect+g_uiAddCUDepth;
  // BB: these parameters may be removed completly and replaced by the fixed values
  pcSPS->setMinTrDepth( 0 );
  pcSPS->setMaxTrDepth( 1 );
  READ_FLAG( uiCode, "chroma_pred_from_luma_enabled_flag" );     pcSPS->setUseLMChroma ( uiCode ? true : false ); 
  READ_FLAG( uiCode, "loop_filter_across_slice_flag" );          assert(uiCode == 1);
#if SAO
  READ_FLAG( uiCode, "sample_adaptive_offset_enabled_flag" );    assert( uiCode == 0 );
#endif
  READ_FLAG( uiCode, "adaptive_loop_filter_enabled_flag" );      assert( uiCode == 0 );
#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  READ_FLAG( uiCode, "pcm_loop_filter_disable_flag" );           assert(uiCode == 0);
#endif
  READ_FLAG( uiCode, "cu_qp_delta_enabled_flag" );               assert( uiCode == 0 );
  READ_FLAG( uiCode, "temporal_id_nesting_flag" );               assert( uiCode == 0 );

  // !!!KS: Syntax not in WD !!!

  xReadUvlc ( uiCode ); pcSPS->setPadX        ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadY        ( uiCode    );

#if !G1002_RPS
  xReadFlag( uiCode ); assert(uiCode == 1);
#endif
  xReadFlag( uiCode ); pcSPS->setUseMRG ( uiCode ? true : false );
  
  // AMVP mode for each depth (AM_NONE or AM_EXPL)
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xReadFlag( uiCode );
    pcSPS->setAMVPMode( i, (AMVP_MODE)uiCode );
  }

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  // these syntax elements should not be sent at SPS when the full reference frame management is supported
  xReadFlag( uiCode ); pcSPS->setUseNewRefSetting( uiCode>0 ? true : false );
  if ( pcSPS->getUseNewRefSetting() )
  {
    xReadUvlc( uiCode );
    pcSPS->setMaxNumRefFrames( uiCode );
  }
#endif
#endif

#if TILES
  READ_FLAG ( uiCode, "uniform_spacing_idc" ); 
  assert(uiCode == 0);
  READ_FLAG ( uiCode, "tile_boundary_independence_idc" );  
  assert(uiCode == 0);
 
  READ_UVLC ( uiCode, "num_tile_columns_minus1" );
  assert(uiCode == 0);
  READ_UVLC ( uiCode, "num_tile_rows_minus1" ); 
  assert(uiCode == 0);
#endif

  // Software-only flags
#if NSQT
  READ_FLAG( uiCode, "enable_nsqt" );
  pcSPS->setUseNSQT( uiCode );
#endif
#if AMP
  READ_FLAG( uiCode, "enable_amp" );
  pcSPS->setUseAMP( uiCode );
#endif
  return;
}

Void TDecSbac::parseSliceHeader (TComSlice*& rpcSlice)
{
  UInt  uiCode;
  Int   iCode;
  
#if ENC_DEC_TRACE
  xTraceSliceHeader(rpcSlice);
#endif
  
  // lightweight_slice_flag
  READ_FLAG( uiCode, "lightweight_slice_flag" );
  assert( uiCode == 0 );

  
  // if( !lightweight_slice_flag ) {
  {
    //   slice_type
    READ_UVLC (    uiCode, "slice_type" );            rpcSlice->setSliceType((SliceType)uiCode);
    READ_UVLC (    uiCode, "pic_parameter_set_id" );  rpcSlice->setPPSId(uiCode);
#if G1002_RPS
    if(rpcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    { 
      READ_UVLC( uiCode, "idr_pic_id" );  //ignored
      READ_FLAG( uiCode, "no_output_of_prior_pics_flag" );  //ignored
      rpcSlice->setPOC(0);
      TComReferencePictureSet* pcRPS = rpcSlice->getLocalRPS();
#if INTER_RPS_PREDICTION
      pcRPS->create(rpcSlice->getPPS()->getSPS()->getMaxNumberOfReferencePictures(), rpcSlice->getPPS()->getSPS()->getMaxNumberOfReferencePictures()+1);
#else
      pcRPS->create(rpcSlice->getPPS()->getSPS()->getMaxNumberOfReferencePictures());
#endif
      pcRPS->setNumberOfNegativePictures(0);
      pcRPS->setNumberOfPositivePictures(0);
      pcRPS->setNumberOfLongtermPictures(0);
      pcRPS->setNumberOfPictures(0);
      rpcSlice->setRPS(pcRPS);
    }
    else {
      READ_CODE(rpcSlice->getSPS()->getBitsForPOC(), uiCode, "pic_order_cnt_lsb");  
      Int iPOClsb = uiCode;
      Int iPrevPOC = rpcSlice->getPrevPOC();
      Int iMaxPOClsb = 1<<rpcSlice->getSPS()->getBitsForPOC();
      Int iPrevPOClsb = iPrevPOC%iMaxPOClsb;
      Int iPrevPOCmsb = iPrevPOC-iPrevPOClsb;
      Int iPOCmsb;
      if( ( iPOClsb  <  iPrevPOClsb ) && ( ( iPrevPOClsb - iPOClsb )  >=  ( iMaxPOClsb / 2 ) ) )
        iPOCmsb = iPrevPOCmsb + iMaxPOClsb;
      else if( (iPOClsb  >  iPrevPOClsb )  && ( (iPOClsb - iPrevPOClsb )  >  ( iMaxPOClsb / 2 ) ) )  
        iPOCmsb = iPrevPOCmsb - iMaxPOClsb;
      else
        iPOCmsb = iPrevPOCmsb;
      rpcSlice->setPOC              (iPOCmsb+iPOClsb);

      TComReferencePictureSet* pcRPS;
      READ_FLAG( uiCode, "short_term_ref_pic_set_pps_flag" );
      if(uiCode == 0) // use short-term reference picture set explicitly signalled in slice header
      {
        pcRPS = rpcSlice->getLocalRPS();
#if INTER_RPS_PREDICTION
        parseShortTermRefPicSet(rpcSlice->getPPS(),pcRPS, rpcSlice->getPPS()->getRPSList()->getNumberOfReferencePictureSets());
#else
        parseShortTermRefPicSet(rpcSlice->getPPS(),pcRPS);
#endif        
        rpcSlice->setRPS(pcRPS);
      }
      else // use reference to short-term reference picture set in PPS
      {
        READ_UVLC( uiCode, "short_term_ref_pic_set_idx"); rpcSlice->setRPS(rpcSlice->getPPS()->getRPSList()->getReferencePictureSet(uiCode));
        pcRPS = rpcSlice->getRPS();
      }
      if(rpcSlice->getPPS()->getLongTermRefsPresent())
      {
        Int offset = pcRPS->getNumberOfNegativePictures()+pcRPS->getNumberOfPositivePictures();
        READ_UVLC( uiCode, "num_long_term_pics");             pcRPS->setNumberOfLongtermPictures(uiCode);
        Int prev = 0;
        for(Int j=pcRPS->getNumberOfLongtermPictures()+offset-1 ; j > offset-1; j--)
        {
          READ_UVLC(uiCode,"delta_poc_lsb_lt_minus1"); 
          prev += 1+uiCode;
          pcRPS->setPOC(j,rpcSlice->getPOC()-prev);          
          pcRPS->setDeltaPOC(j,-(Int)prev);
          READ_FLAG( uiCode, "used_by_curr_pic_lt_flag");     pcRPS->setUsed(j,uiCode);
        }
        offset += pcRPS->getNumberOfLongtermPictures();
        pcRPS->setNumberOfPictures(offset);        
      }  
    }
#endif
#if !G1002_RPS
    //   frame_num
    //   if( IdrPicFlag )
    //     idr_pic_id
    //   if( pic_order_cnt_type  = =  0 )
    //     pic_order_cnt_lsb  
    READ_CODE (10, uiCode, "pic_order_cnt_lsb" );     rpcSlice->setPOC(uiCode);             // 9 == SPS->Log2MaxFrameNum()
    //   if( slice_type  = =  P  | |  slice_type  = =  B ) {
    //     num_ref_idx_active_override_flag
    //   if( num_ref_idx_active_override_flag ) {
    //     num_ref_idx_l0_active_minus1
    //     if( slice_type  = =  B )
    //       num_ref_idx_l1_active_minus1
    //   }
#endif
    if (!rpcSlice->isIntra())
    {
      READ_FLAG( uiCode, "num_ref_idx_active_override_flag");
      if (uiCode)
      {
        READ_CODE (3, uiCode, "num_ref_idx_l0_active_minus1" );  rpcSlice->setNumRefIdx( REF_PIC_LIST_0, uiCode + 1 );
        if (rpcSlice->isInterB())
        {
          READ_CODE (3, uiCode, "num_ref_idx_l1_active_minus1" );  rpcSlice->setNumRefIdx( REF_PIC_LIST_1, uiCode + 1 );
        }
        else
        {
          rpcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
        }
      }
      else
      {
        rpcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
        rpcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
      }
    }
    // }
#if G1002_RPS
    TComRefPicListModification* refPicListModification = rpcSlice->getRefPicListModification();
    if(!rpcSlice->isIntra())
    {
      READ_FLAG( uiCode, "ref_pic_list_modification_flag_l0" ); refPicListModification->setRefPicListModificationFlagL0( uiCode ? 1 : 0 );
      
      if(refPicListModification->getRefPicListModificationFlagL0())
      {
        uiCode = 0;
        Int i = 0;
        Int list_modification_idc = 0;
        while(list_modification_idc != 3)  
        {
          READ_UVLC( uiCode, "list_modification_idc" ); refPicListModification->setListIdcL0(i, uiCode );
          list_modification_idc = uiCode;
          if(uiCode != 3)
          {
            READ_UVLC( uiCode, "ref_pic_set_idx" ); refPicListModification->setRefPicSetIdxL0(i, uiCode );
          }
          i++;
        }
        refPicListModification->setNumberOfRefPicListModificationsL0(i-1);
      }
      else
        refPicListModification->setNumberOfRefPicListModificationsL0(0); 
    }
    else
    {
      refPicListModification->setRefPicListModificationFlagL0(0);
      refPicListModification->setNumberOfRefPicListModificationsL0(0);
    }
    if(rpcSlice->isInterB())
    {
      READ_FLAG( uiCode, "ref_pic_list_modification_flag_l1" ); refPicListModification->setRefPicListModificationFlagL1( uiCode ? 1 : 0 );
      if(refPicListModification->getRefPicListModificationFlagL1())
      {
        uiCode = 0;
        Int i = 0;
        Int list_modification_idc = 0;
        while(list_modification_idc != 3)  
        {
          READ_UVLC( uiCode, "list_modification_idc" ); refPicListModification->setListIdcL1(i, uiCode );
          list_modification_idc = uiCode;
          if(uiCode != 3)
          {
            READ_UVLC( uiCode, "ref_pic_set_idx" ); refPicListModification->setRefPicSetIdxL1(i, uiCode );
          }
          i++;
        }
        refPicListModification->setNumberOfRefPicListModificationsL1(i-1);
      }
      else
        refPicListModification->setNumberOfRefPicListModificationsL1(0);
    }  
    else
    {
      refPicListModification->setRefPicListModificationFlagL1(0);
      refPicListModification->setNumberOfRefPicListModificationsL1(0);
    }
#endif
  }
#if !G1002_RPS
  // ref_pic_list_modification( )
#endif
  // ref_pic_list_combination( )
  if (rpcSlice->isInterB())
  {
    READ_FLAG( uiCode, "ref_pic_list_combination_flag" );       assert(uiCode == 0);
      rpcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
  }
  
  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //   slice_address
#if FINE_GRANULARITY_SLICES
  int iNumCUs = ((rpcSlice->getSPS()->getWidth()+rpcSlice->getSPS()->getMaxCUWidth()-1)/rpcSlice->getSPS()->getMaxCUWidth())*((rpcSlice->getSPS()->getHeight()+rpcSlice->getSPS()->getMaxCUHeight()-1)/rpcSlice->getSPS()->getMaxCUHeight());
  int iMaxParts = (1<<(rpcSlice->getSPS()->getMaxCUDepth()<<1));
  READ_FLAG( uiCode, "first_slice_in_pic_flag" );
  assert( uiCode == 1 );
#endif
  {
    rpcSlice->setSliceCurEndCUAddr(iNumCUs*iMaxParts);
    // if( !lightweight_slice_flag ) {
    //   slice_qp_delta
    // should be delta
    READ_SVLC( iCode, "slice_qp" );  rpcSlice->setSliceQp          (iCode);
   
    //   if( sample_adaptive_offset_enabled_flag )
    //     sao_param()
    //   if( deblocking_filter_control_present_flag ) {
    //     disable_deblocking_filter_idc
    // this should be an idc
    READ_FLAG ( uiCode, "loop_filter_disable" );  rpcSlice->setLoopFilterDisable(uiCode ? 1 : 0);
    //     if( disable_deblocking_filter_idc  !=  1 ) {
    //       slice_alpha_c0_offset_div2
    //       slice_beta_offset_div2
    //     }
    //   }
    //   if( slice_type = = B )
    //     collocated_from_l0_flag
    if ( rpcSlice->getSliceType() == B_SLICE )
    {
      READ_FLAG( uiCode, "collocated_from_l0_flag" );
      rpcSlice->setColDir(uiCode);
    }
  }

  // !!!! Syntax elements not in the WD  !!!!!
  
  {
    xReadFlag (uiCode);   rpcSlice->setDRBFlag          (uiCode ? 1 : 0);
    if ( !rpcSlice->getDRBFlag() )
    {
      xReadCode(2, uiCode); rpcSlice->setERBIndex( (ERBIndex)uiCode );    assert (uiCode == ERB_NONE || uiCode == ERB_LTR);
    }      
  }
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  READ_UVLC( uiCode, "MaxNumMergeCand");
  rpcSlice->setMaxNumMergeCand(MRG_MAX_NUM_CANDS - uiCode);
  assert(rpcSlice->getMaxNumMergeCand()==MRG_MAX_NUM_CANDS_SIGNALED);
#endif

  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TDecSbac::xReadCode (UInt uiLength, UInt& ruiCode)
{
  assert ( uiLength > 0 );
  m_pcBitstream->read (uiLength, ruiCode);
}

Void TDecSbac::xReadUvlc( UInt& ruiVal)
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

Void TDecSbac::xReadSvlc( Int& riVal)
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

Void TDecSbac::xReadFlag (UInt& ruiCode)
{
  m_pcBitstream->read( 1, ruiCode );
}


//! \}
