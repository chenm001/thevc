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

//! \ingroup TLibDecoder
//! \{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_pcTDecBinIf               ( NULL )
, m_bAlfCtrl                  ( false )
, m_uiMaxAlfCtrlDepth         ( 0 )
, m_numContextModels          ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            , m_contextModels + m_numContextModels, m_numContextModels )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeFlagExtSCModel    ( 1,             1,               NUM_MERGE_FLAG_EXT_CTX        , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeIdxExtSCModel     ( 1,             1,               NUM_MERGE_IDX_EXT_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAlfCtrlFlagSCModel     ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUDeltaQpSCModel         ( 1,             1,               NUM_DELTA_QP_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
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
, m_cALFFlagSCModel           ( 1,             1,               NUM_ALF_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cALFUvlcSCModel           ( 1,             1,               NUM_ALF_UVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cALFSvlcSCModel           ( 1,             1,               NUM_ALF_SVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#if AMP
, m_cCUXPosiSCModel           ( 1,             1,               NUM_CU_X_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUYPosiSCModel           ( 1,             1,               NUM_CU_Y_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if SAO
, m_cSaoFlagSCModel           ( 1,             1,               NUM_SAO_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoUvlcSCModel           ( 1,             1,               NUM_SAO_UVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoSvlcSCModel           ( 1,             1,               NUM_SAO_SVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
{
  assert( m_numContextModels <= MAX_NUM_CTX_MOD );
#if FINE_GRANULARITY_SLICES
  m_iSliceGranularity = 0;
#endif
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
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_ALF_CTRL_FLAG );
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
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (UChar*)INIT_DQP );
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
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_SVLC );
#if SAO
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_SVLC );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
  
#else
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_ALF_CTRL_FLAG );
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
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_DQP );
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
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
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
  
  m_pcTDecBinIf->start();
}

#if TILES
/** The function does the following: Read out terminate bit. Flush CABAC. Byte-align for next tile.
 *  Intialize CABAC states. Start CABAC.
 */
Void TDecSbac::updateContextTables( SliceType eSliceType, Int iQp )
{
  UInt uiBit;
  m_pcTDecBinIf->decodeBinTrm(uiBit);
  m_pcTDecBinIf->finish();  
#if OL_USE_WPP && !OL_FLUSH_ALIGN
  // Account for misaligned CABAC.
  Int iCABACReadAhead = m_pcTDecBinIf->getBitsReadAhead();
  iCABACReadAhead--;
  Int iStreamBits = 8-m_pcBitstream->getNumBitsUntilByteAligned();
  if (iCABACReadAhead >= iStreamBits)
  {
    // Misaligned CABAC has read into the 1st byte of the next tile.
    // Back up a byte prior to alignment.
    m_pcBitstream->backupByte();
  }
#endif
  m_pcBitstream->readOutTrailingBits();
#if G633_8BIT_INIT
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_ALF_CTRL_FLAG );
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
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (UChar*)INIT_DQP );
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
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_SVLC );
#if SAO
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_SVLC );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );

#else
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_ALF_CTRL_FLAG );
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
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_DQP );
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
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
#if SAO
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_SVLC );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
#endif
  m_pcTDecBinIf->start();
}

#if TILES_DECODER
Void TDecSbac::readTileMarker( UInt& uiTileIdx, UInt uiBitsUsed )
{
  UInt uiSymbol;
  uiTileIdx = 0;
  for (Int iShift=uiBitsUsed-1; iShift>=0; iShift--)
  {
    m_pcTDecBinIf->decodeBinEP ( uiSymbol );
    if (uiSymbol)
    {
      uiTileIdx |= (1<<iShift);
    }
  }
}
#endif
#endif

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{
  m_pcTDecBinIf->decodeBinTrm( ruiBit );
}


Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
  
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( ruiSymbol == 0 || uiMaxSymbol == 1 )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
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
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  if ( --uiCount )
  {
    UInt bins;
    m_pcTDecBinIf->decodeBinsEP( bins, uiCount );
    uiSymbol += bins;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
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
    m_pcTDecBinIf->decodeBinEP( uiCodeWord );
  }
  while( uiCodeWord && uiQuotient < uiMaxPreLen );

  uiCodeWord  = 1 - uiCodeWord;
  uiQuotient -= uiCodeWord;

  if ( ruiGoRiceParam > 0 )
  {
    m_pcTDecBinIf->decodeBinsEP( uiRemainder, ruiGoRiceParam );    
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


/** Parse I_PCM information. 
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, parse its PCM alignment bits and codes. 
 */
Void TDecSbac::parseIPCMInfo ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  m_pcTDecBinIf->decodeBinTrm(uiSymbol);

  if (uiSymbol)
  {
    Bool bIpcmFlag = true;

    m_pcTDecBinIf->decodePCMAlignBits();

    pcCU->setPartSizeSubParts  ( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts      ( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    pcCU->setIPCMFlagSubParts  ( bIpcmFlag, uiAbsPartIdx, uiDepth );

    UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
    UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
    UInt uiChromaOffset = uiLumaOffset>>2;

    Pel* piPCMSample;
    UInt uiWidth;
    UInt uiHeight;
    UInt uiSampleBits;
    UInt uiX, uiY;

    piPCMSample = pcCU->getPCMSampleY() + uiLumaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx);
    uiHeight = pcCU->getHeight(uiAbsPartIdx);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCb() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCr() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    m_pcTDecBinIf->resetBac();
  }
}

Void TDecSbac::parseAlfCtrlDepth( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol( uiSymbol, m_cALFUvlcSCModel.get( 0 ), 1, g_uiMaxCUDepth - 1 );
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecSbac::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( !m_bAlfCtrl )
  {
    return;
  }
  
  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth( uiAbsPartIdx, m_uiMaxAlfCtrlDepth ) )
  {
    return;
  }
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUAlfCtrlFlagSCModel.get( 0 ) );
  
  if( uiDepth > m_uiMaxAlfCtrlDepth )
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth );
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
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
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
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
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );
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
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[uiUnaryIdx] ) );
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
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth ) ) );
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
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
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
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
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
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          if (uiSymbol == 0)
          {
            m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
            eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
          }
        }
        else if (eMode == SIZE_Nx2N)
        {
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          if (uiSymbol == 0)
          {
            m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
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
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
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
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 2) );
        if (uiSymbol == 0)
        {
          pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
          if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
          {
            m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
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
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
      }
      
      if (uiSymbol == 0)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
        {
          m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
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
        m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
          eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
        m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        if (uiSymbol == 0)
        {
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
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
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
#else //!PREDTYPE_CLEANUP
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
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
 
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
 
  if ( uiSymbol )
  {
#if BYPASS_FOR_INTRA_MODE
    m_pcTDecBinIf->decodeBinEP( uiSymbol );
#else
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2 ) );
#endif
    intraPredMode = uiPreds[uiSymbol];
  }
  else
  {
    intraPredMode = 0;
    
#if BYPASS_FOR_INTRA_MODE
    m_pcTDecBinIf->decodeBinsEP( uiSymbol, g_aucIntraModeBitsAng[iIntraIdx] - 1 );
    intraPredMode = uiSymbol;
#else
    for ( Int i = 0; i < g_aucIntraModeBitsAng[iIntraIdx] - 1; i++ )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
      intraPredMode |= uiSymbol << i;
    }
#endif
    
    if ( intraPredMode == 31 )
    {
#if BYPASS_FOR_INTRA_MODE
      m_pcTDecBinIf->decodeBinEP( uiSymbol );
#else
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
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

  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );

  if( uiSymbol == 0 )
  {
    uiSymbol = DM_CHROMA_IDX;
  } 
  else 
  {
    if( pcCU->getSlice()->getSPS()->getUseLMChroma() )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 1 ) );
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
  m_pcTDecBinIf->decodeBin( uiSymbol, *( pCtx + uiCtx ) );

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
    m_pcTDecBinIf->decodeBin( uiSymbol, *( pCtx + 3 ) );
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
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

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
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

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

  m_pcTDecBinIf->decodeBin( uiHorAbs, *pCtx );
  m_pcTDecBinIf->decodeBin( uiVerAbs, *pCtx );

  const Bool bHorAbsGr0 = uiHorAbs != 0;
  const Bool bVerAbsGr0 = uiVerAbs != 0;
  pCtx++;

  if( bHorAbsGr0 )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );
    uiHorAbs += uiSymbol;
  }

  if( bVerAbsGr0 )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );
    uiVerAbs += uiSymbol;
  }

  if( bHorAbsGr0 )
  {
    if( 2 == uiHorAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiHorAbs += uiSymbol;
    }

    m_pcTDecBinIf->decodeBinEP( uiHorSign );
  }

  if( bVerAbsGr0 )
  {
    if( 2 == uiVerAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiVerAbs += uiSymbol;
    }

    m_pcTDecBinIf->decodeBinEP( uiVerSign );
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
  m_pcTDecBinIf->decodeBin( ruiSubdivFlag, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiLog2TransformBlockSize ) );
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
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
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

Void TDecSbac::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiDQp;
  Int  iDQp;
  
  m_pcTDecBinIf->decodeBin( uiDQp, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  
  if ( uiDQp == 0 )
  {
    uiDQp = pcCU->getRefQP(uiAbsPartIdx);
  }
  else
  {
#if F745_DQP_BINARIZATION
    UInt uiSign;
    UInt uiQpBdOffsetY = 6*(g_uiBitIncrement + g_uiBitDepth - 8);
    m_pcTDecBinIf->decodeBinEP(uiSign);

    UInt uiMaxAbsDQpMinus1 = 24 + (uiQpBdOffsetY/2) + (uiSign);
    UInt uiAbsDQpMinus1;
    xReadUnaryMaxSymbol (uiAbsDQpMinus1,  &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1, uiMaxAbsDQpMinus1);

    iDQp = uiAbsDQpMinus1 + 1;

    if(uiSign)
    {
      iDQp = -iDQp;
    }
#else
    xReadUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
    iDQp = ( uiDQp + 2 ) / 2;
    
    if ( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
#endif
    uiDQp = pcCU->getRefQP(uiAbsPartIdx) + iDQp;

  }
  
  UInt uiAbsQpCUPartIdx = (uiAbsPartIdx>>(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)) ;
  UInt uiQpCUDepth =   min(uiDepth,pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()) ;
  pcCU->setQPSubParts( uiDQp, uiAbsQpCUPartIdx, uiQpCUDepth );
}

Void TDecSbac::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
#if CHROMA_CBF_CTX_REDUCTION
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? TEXT_CHROMA: eType, uiCtx ) );
#else
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );
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
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxX + puiCtxIdxX[ uiPosLastX ] ) );
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
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastX = g_uiMinInGroup[ uiPosLastX ] + uiTemp;
  }
#endif

  // posY
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ height ] * ( g_aucConvertToBit[ height ] + 3 ) );
  for( uiPosLastY = 0; uiPosLastY < g_uiGroupIdx[ height - 1 ]; uiPosLastY++ )
  {
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxY + puiCtxIdxY[ uiPosLastY ] ) );
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
      m_pcTDecBinIf->decodeBinEP( uiLast );
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
      m_pcTDecBinIf->decodeBinEP( uiLast );
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
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxX + puiCtxIdx[ uiPosLastX ] ) );

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
      m_pcTDecBinIf->decodeBinEP( uiLast );
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
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxY + puiCtxIdx[ uiPosLastY ] ) );

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
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << uiCount;
      uiCount++;
    }
    uiPosLastY += uiTemp;
  }
#else
  if ( !lastX && halfWidth >= minWidth )
  {
    UInt temp;
    m_pcTDecBinIf->decodeBinsEP( temp, log2BlkWidth );
    uiPosLastX += temp;
  }
  if ( !uiLast && halfHeight >= minHeight )
  {
    UInt temp;
    m_pcTDecBinIf->decodeBinsEP( temp, log2BlkHeight );
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
    uiScanIdx = DIAG_SCAN;
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
          m_pcTDecBinIf->decodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
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
            m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
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
      m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
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
          m_pcTDecBinIf->decodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
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
            m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
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
        m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
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
        m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c1] );
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
            m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c2] );
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
      m_pcTDecBinIf->decodeBinsEP( coeffSigns, numNonZero );
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

Void TDecSbac::parseAlfFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
  
  ruiVal = uiSymbol;
}

Void TDecSbac::parseAlfFlagNum( UInt& ruiVal, UInt minValue, UInt depth )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (depth*2));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  ruiVal = 0;
  UInt uiBit;
  if(uiLength)
  {
    while( uiLength-- )
    {
      m_pcTDecBinIf->decodeBinEP( uiBit );
      ruiVal += uiBit << uiLength;
    }
  }
  else
  {
    ruiVal = 0;
  }
  ruiVal += minValue;
}

Void TDecSbac::parseAlfCtrlFlag( UInt &ruiAlfCtrlFlag )
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0 ) );
  ruiAlfCtrlFlag = uiSymbol;
}

Void TDecSbac::parseAlfUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }
  
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
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
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  
  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }
  
  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
  
  if ( uiCode == 0 ) iSign =  1;
  else               iSign = -1;
  
  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  
  riVal = i*iSign;
}

#if SAO
Void TDecSbac::parseSaoFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cSaoFlagSCModel.get( 0, 0, 0 ) );

  ruiVal = uiSymbol;
}

Void TDecSbac::parseSaoUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }

  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  ruiVal = i;
}

Void TDecSbac::parseSaoSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 0 ) );

  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }

  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 1 ) );

  if ( uiCode == 0 ) iSign =  1;
  else               iSign = -1;

  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  riVal = i*iSign;
}
#endif

#if OL_USE_WPP
/**
 - Initialize our contexts from the nominated source.
 .
 \param pSrc Contexts to be copied.
 */
Void TDecSbac::xCopyContextsFrom( TDecSbac* pSrc )
{
  m_cCUSplitFlagSCModel       .copyFrom( &pSrc->m_cCUSplitFlagSCModel       );
  m_cCUSkipFlagSCModel        .copyFrom( &pSrc->m_cCUSkipFlagSCModel        );
  m_cCUMergeFlagExtSCModel    .copyFrom( &pSrc->m_cCUMergeFlagExtSCModel    );
  m_cCUMergeIdxExtSCModel     .copyFrom( &pSrc->m_cCUMergeIdxExtSCModel     );
  m_cCUAlfCtrlFlagSCModel     .copyFrom( &pSrc->m_cCUAlfCtrlFlagSCModel     );
  m_cCUPartSizeSCModel        .copyFrom( &pSrc->m_cCUPartSizeSCModel        );
#if AMP
  m_cCUXPosiSCModel           .copyFrom( &pSrc->m_cCUXPosiSCModel           );
  m_cCUYPosiSCModel           .copyFrom( &pSrc->m_cCUYPosiSCModel           );
#endif
  m_cCUPredModeSCModel        .copyFrom( &pSrc->m_cCUPredModeSCModel        );
  m_cCUIntraPredSCModel       .copyFrom( &pSrc->m_cCUIntraPredSCModel       );
  m_cCUChromaPredSCModel      .copyFrom( &pSrc->m_cCUChromaPredSCModel      );
  m_cCUInterDirSCModel        .copyFrom( &pSrc->m_cCUInterDirSCModel        );
  m_cCUMvdSCModel             .copyFrom( &pSrc->m_cCUMvdSCModel             );
  m_cCURefPicSCModel          .copyFrom( &pSrc->m_cCURefPicSCModel          );
  m_cCUDeltaQpSCModel         .copyFrom( &pSrc->m_cCUDeltaQpSCModel         );
  m_cCUQtCbfSCModel           .copyFrom( &pSrc->m_cCUQtCbfSCModel           );
  m_cCUQtRootCbfSCModel       .copyFrom( &pSrc->m_cCUQtRootCbfSCModel       );
#if MULTI_LEVEL_SIGNIFICANCE
  m_cCUSigCoeffGroupSCModel   .copyFrom( &pSrc->m_cCUSigCoeffGroupSCModel   );
#endif
#if SIGMAP_CTX_RED
  m_cCUSigSCModelLuma         .copyFrom( &pSrc->m_cCUSigSCModelLuma         );
  m_cCUSigSCModelChroma       .copyFrom( &pSrc->m_cCUSigSCModelChroma       );
#else
  m_cCUSigSCModel             .copyFrom( &pSrc->m_cCUSigSCModel             );
#endif
  m_cCuCtxLastX               .copyFrom( &pSrc->m_cCuCtxLastX               );
  m_cCuCtxLastY               .copyFrom( &pSrc->m_cCuCtxLastY               );
#if COEFF_CTXSET_RED
  m_cCUOneSCModelLuma         .copyFrom( &pSrc->m_cCUOneSCModelLuma         );
  m_cCUOneSCModelChroma       .copyFrom( &pSrc->m_cCUOneSCModelChroma       );
  m_cCUAbsSCModelLuma         .copyFrom( &pSrc->m_cCUAbsSCModelLuma         );
  m_cCUAbsSCModelChroma       .copyFrom( &pSrc->m_cCUAbsSCModelChroma       );
#else
  m_cCUOneSCModel             .copyFrom( &pSrc->m_cCUOneSCModel             );
  m_cCUAbsSCModel             .copyFrom( &pSrc->m_cCUAbsSCModel             );
#endif
  m_cMVPIdxSCModel            .copyFrom( &pSrc->m_cMVPIdxSCModel            );
  m_cALFFlagSCModel           .copyFrom( &pSrc->m_cALFFlagSCModel           );
  m_cALFUvlcSCModel           .copyFrom( &pSrc->m_cALFUvlcSCModel           );
  m_cALFSvlcSCModel           .copyFrom( &pSrc->m_cALFSvlcSCModel           );
#if SAO
  m_cSaoFlagSCModel            .copyFrom( &pSrc->m_cSaoFlagSCModel            );
  m_cSaoUvlcSCModel            .copyFrom( &pSrc->m_cSaoUvlcSCModel            );
  m_cSaoSvlcSCModel            .copyFrom( &pSrc->m_cSaoSvlcSCModel            );
#endif
  m_cCUTransSubdivFlagSCModel .copyFrom( &pSrc->m_cCUTransSubdivFlagSCModel );
}

Void TDecSbac::xCopyFrom( TDecSbac* pSrc )
{
  m_pcTDecBinIf->copyState( pSrc->m_pcTDecBinIf );

  m_uiLastQp           = pSrc->m_uiLastQp;
  xCopyContextsFrom( pSrc );

}

Void TDecSbac::load ( TDecSbac* pScr )
{
  xCopyFrom(pScr);
}

Void TDecSbac::loadContexts ( TDecSbac* pScr )
{
  xCopyContextsFrom(pScr);
}

#endif
#if OL_FLUSH
Void TDecSbac::decodeFlush ( )
{
  UInt uiBit;
  m_pcTDecBinIf->decodeBinTrm(uiBit);
  m_pcTDecBinIf->flush();

}
#endif
//! \}
