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

/** \file     TEncSbac.cpp
    \brief    SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"
#include "SEIwrite.h"

#include <map>

#if ENC_DEC_TRACE

#define WRITE_CODE( value, length, name)    xWriteCodeTr ( value, length, name )
#define WRITE_UVLC( value,         name)    xWriteUvlcTr ( value,         name )
#define WRITE_SVLC( value,         name)    xWriteSvlcTr ( value,         name )
#define WRITE_FLAG( value,         name)    xWriteFlagTr ( value,         name )

Void  xWriteUvlcTr          ( UInt value,               const Char *pSymbolName);
Void  xWriteSvlcTr          ( Int  value,               const Char *pSymbolName);
Void  xWriteFlagTr          ( UInt value,               const Char *pSymbolName);

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


Void  TEncSbac::xWriteCodeTr (UInt value, UInt  length, const Char *pSymbolName)
{
  xWriteCode (value,length);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, value ); 
}

Void  TEncSbac::xWriteUvlcTr (UInt value, const Char *pSymbolName)
{
  xWriteUvlc (value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, value ); 
}

Void  TEncSbac::xWriteSvlcTr (Int value, const Char *pSymbolName)
{
  xWriteSvlc(value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, value ); 
}

Void  TEncSbac::xWriteFlagTr(UInt value, const Char *pSymbolName)
{
  xWriteFlag(value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, value ); 
}

#else

#define WRITE_CODE( value, length, name)     xWriteCode ( value, length )
#define WRITE_UVLC( value,         name)     xWriteUvlc ( value )
#define WRITE_SVLC( value,         name)     xWriteSvlc ( value )
#define WRITE_FLAG( value,         name)     xWriteFlag ( value )

#endif

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSbac::TEncSbac()
// new structure here
: m_pcBitIf                   ( NULL )
, m_pcSlice                   ( NULL )
, m_pcBinCabac                ( NULL )
, m_uiCoeffCost               ( 0 )
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
  
#if G633_8BIT_INIT
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
  
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT);
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT);
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
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT);
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT);
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
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
#if SAO
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_SAO_SVLC );
#endif
#endif
  // new structure
  m_uiLastQp = iQp;
  
  m_pcBinCabac->start();
  
  return;
}

void TEncSbac::codeSEI(const SEI& sei)
{
  writeSEImessage(*m_pcBitIf, sei);
}

Void TEncSbac::codeSPS( TComSPS* pcSPS )
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  WRITE_CODE( pcSPS->getProfileIdc (),     8,       "profile_idc" );
  WRITE_CODE( 0,                           8,       "reserved_zero_8bits" );
  WRITE_CODE( pcSPS->getLevelIdc (),       8,       "level_idc" );
  WRITE_UVLC( pcSPS->getSPSId (),                   "seq_parameter_set_id" );
  WRITE_CODE( pcSPS->getMaxTLayers() - 1,  3,       "max_temporal_layers_minus1" );
  WRITE_CODE( pcSPS->getWidth (),         16,       "pic_width_in_luma_samples" );
//  WRITE_UVLC( pcSPS->getWidth (),                   "pic_width_in_luma_samples" );
  WRITE_CODE( pcSPS->getHeight(),         16,       "pic_height_in_luma_samples" );
//  WRITE_UVLC( pcSPS->getHeight(),                   "pic_height_in_luma_samples" );

  WRITE_UVLC( 0,             "bit_depth_luma_minus8" );
  WRITE_UVLC( 0,             "bit_depth_chroma_minus8" );
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
  WRITE_CODE( 7, 4, "pcm_bit_depth_luma_minus1" );
  WRITE_CODE( 7, 4, "pcm_bit_depth_chroma_minus1" );
#endif
#if G1002_RPS
  WRITE_UVLC( pcSPS->getBitsForPOC()-4,                 "log2_max_pic_order_cnt_lsb_minus4" );
  WRITE_UVLC( pcSPS->getMaxNumberOfReferencePictures(), "max_num_ref_pics" ); 
  WRITE_UVLC( pcSPS->getMaxNumberOfReorderPictures(),   "max_num_reorder_pics" ); 
#endif
#if DISABLE_4x4_INTER
  WRITE_FLAG  ( (pcSPS->getDisInter4x4()) ? 1 : 0, "DisableInter4x4" );
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
  // }
  // max_num_ref_frames
  // gaps_in_frame_num_value_allowed_flag
#endif
  assert( pcSPS->getMaxCUWidth() == pcSPS->getMaxCUHeight() );
  
  UInt MinCUSize = pcSPS->getMaxCUWidth() >> ( pcSPS->getMaxCUDepth()-g_uiAddCUDepth );
  UInt log2MinCUSize = 0;
  while(MinCUSize > 1)
  {
    MinCUSize >>= 1;
    log2MinCUSize++;
  }
  
  WRITE_UVLC( log2MinCUSize - 3,                                                     "log2_min_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getMaxCUDepth()-g_uiAddCUDepth,                                 "log2_diff_max_min_coding_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MinSize() - 2,                                 "log2_min_transform_block_size_minus2" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize(), "log2_diff_max_min_transform_block_size" );
  WRITE_UVLC( 7 - 3,                                                                 "log2_min_pcm_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthInter() - 1,                               "max_transform_hierarchy_depth_inter" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthIntra() - 1,                               "max_transform_hierarchy_depth_intra" );
  WRITE_FLAG  ( (pcSPS->getUseLMChroma ()) ? 1 : 0,                                  "chroma_pred_from_luma_enabled_flag" ); 
  WRITE_FLAG( 1,                                                                     "loop_filter_across_slice_flag");
#if SAO
  WRITE_FLAG( 0,                                                                     "sample_adaptive_offset_enabled_flag");
#endif
  WRITE_FLAG( 0,                                                                     "adaptive_loop_filter_enabled_flag");
#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  WRITE_FLAG( 0,                                                                     "pcm_loop_filter_disable_flag");
#endif
  WRITE_FLAG( 0,                                                                     "cu_qp_delta_enabled_flag" );
  assert( pcSPS->getMaxTLayers() > 0 );         

  WRITE_FLAG( 0,                                                                     "temporal_id_nesting_flag" );

  // !!!KS: Syntax not in WD !!!
  
  WRITE_UVLC  ( pcSPS->getPad (0), "PadX" );
  WRITE_UVLC  ( pcSPS->getPad (1), "PadY" );

  // Tools
#if !G1002_RPS
  WRITE_FLAG  ( 1,  "Unknow");
#endif
  WRITE_FLAG  ( (pcSPS->getUseMRG ()) ? 1 : 0, "SOPH" ); // SOPH:
  
  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    WRITE_FLAG( 1, "AMVPMode");
  }

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  // these syntax elements should not be sent at SPS when the full reference frame management is supported
  WRITE_FLAG( pcSPS->getUseNewRefSetting() ? 1 : 0, "UseNewRef" );
  if ( pcSPS->getUseNewRefSetting() )
  {
    WRITE_UVLC( pcSPS->getMaxNumRefFrames(), "MaxNumRefFrames" );
  }
#endif
#endif

#if TILES
  WRITE_FLAG( 0,                                                      "uniform_spacing_idc" );
  WRITE_FLAG( 0,                                                      "tile_boundary_independence_idc" );
  WRITE_UVLC( 0,                                                      "num_tile_columns_minus1" );
  WRITE_UVLC( 0,                                                      "num_tile_rows_minus1" );
#endif
  
  // Software-only flags
#if NSQT
  WRITE_FLAG( pcSPS->getUseNSQT(), "enable_nsqt" );
#endif
#if AMP
  WRITE_FLAG( pcSPS->getUseAMP(), "enable_amp" );
#endif
}

Void TEncSbac::codePPS( TComPPS* pcPPS )
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
#if G1002_RPS
   TComRPS* pcRPSList = pcPPS->getRPSList();
#endif
  
  WRITE_UVLC( pcPPS->getPPSId(),                             "pic_parameter_set_id" );
  WRITE_UVLC( pcPPS->getSPSId(),                             "seq_parameter_set_id" );
#if G1002_RPS
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD
  TComReferencePictureSet*      pcRPS;

  WRITE_UVLC(pcRPSList->getNumberOfReferencePictureSets(), "num_short_term_ref_pic_sets" );
  for(UInt i=0; i < pcRPSList->getNumberOfReferencePictureSets(); i++)
  {
    pcRPS = pcRPSList->getReferencePictureSet(i);
    codeShortTermRefPicSet(pcPPS,pcRPS);
  }    
  WRITE_FLAG( pcPPS->getLongTermRefsPresent() ? 1 : 0,         "long_term_ref_pics_present_flag" );
#endif
  // entropy_coding_mode_flag
#if OL_USE_WPP
  // We code the entropy_coding_mode_flag, it's needed for tests.
  WRITE_FLAG( 1,                                             "entropy_coding_mode_flag" );
  {
    WRITE_UVLC( 0,                                           "entropy_coding_synchro" );
    WRITE_FLAG( 0,                                           "cabac_istate_reset" );
  }
#endif
  WRITE_UVLC( 0,                                             "num_temporal_layer_switching_point_flags" );
  //   num_ref_idx_l0_default_active_minus1
  //   num_ref_idx_l1_default_active_minus1
  //   pic_init_qp_minus26  /* relative to 26 */
  WRITE_FLAG( pcPPS->getConstrainedIntraPred() ? 1 : 0,      "constrained_intra_pred_flag" );
#if FINE_GRANULARITY_SLICES
  WRITE_CODE( 0, 2,               "slice_granularity");
#endif
#if !F747_APS
  WRITE_FLAG( pcPPS->getSharedPPSInfoEnabled() ? 1: 0,       "shared_pps_info_enabled_flag" );
  //   if( shared_pps_info_enabled_flag )
  //     if( adaptive_loop_filter_enabled_flag )
  //       alf_param( )
#endif

#if WEIGHT_PRED
  WRITE_FLAG( 0,  "weighted_pred_flat" );   // Use of Weighting Prediction (P_SLICE)
  WRITE_CODE( 0, 2, "weighted_bipred_idc" );  // Use of Weighting Bi-Prediction (B_SLICE)
#endif

#if TILES
  WRITE_FLAG( 0, "tile_info_present_flag" );
#endif
}

Void TEncSbac::codeSliceHeader( TComSlice* pcSlice )
{
#if ENC_DEC_TRACE  
  xTraceSliceHeader (pcSlice);
#endif
  
  Bool bEntropySlice = false;
  WRITE_FLAG( bEntropySlice ? 1 : 0, "lightweight_slice_flag" );
  
  if (!bEntropySlice)
  {
    WRITE_UVLC( pcSlice->getSliceType(),       "slice_type" );
    WRITE_UVLC( pcSlice->getPPS()->getPPSId(), "pic_parameter_set_id" );
#if G1002_RPS
    if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    {
      WRITE_UVLC( 0, "idr_pic_id" );
      WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    else
    {
      WRITE_CODE( pcSlice->getPOC()%(1<<pcSlice->getSPS()->getBitsForPOC()), pcSlice->getSPS()->getBitsForPOC(), "pic_order_cnt_lsb");
      TComReferencePictureSet* pcRPS = pcSlice->getRPS();
      if(pcSlice->getRPSidx() < 0)
      {
        WRITE_FLAG( 0, "short_term_ref_pic_set_pps_flag");
        codeShortTermRefPicSet(pcSlice->getPPS(), pcRPS);
      }
      else
      {
        WRITE_FLAG( 1, "short_term_ref_pic_set_pps_flag");
        WRITE_UVLC( pcSlice->getRPSidx(), "short_term_ref_pic_set_idx" );
      }
      if(pcSlice->getPPS()->getLongTermRefsPresent())
      {
        WRITE_UVLC( pcRPS->getNumberOfLongtermPictures(), "num_long_term_pics");
        Int maxPocLsb = 1<<pcSlice->getSPS()->getBitsForPOC();
        Int prev = 0;
        for(Int i=pcRPS->getNumberOfPictures()-1 ; i > pcRPS->getNumberOfPictures()-pcRPS->getNumberOfLongtermPictures()-1; i--)
        {
          WRITE_UVLC((maxPocLsb-pcRPS->getDeltaPOC(i)+prev-1)%maxPocLsb, "delta_poc_lsb_lt_minus1");
          prev = pcRPS->getDeltaPOC(i);
          WRITE_FLAG( pcRPS->getUsed(i), "used_by_curr_pic_lt_flag"); 
        }
      }
    }
#endif

#if !G1002_RPS
    // frame_num
    // if( IdrPicFlag )
    //   idr_pic_id
    // if( pic_order_cnt_type  = =  0 )
    //   pic_order_cnt_lsb  
    WRITE_CODE( pcSlice->getPOC(), 10, "pic_order_cnt_lsb" );   //  9 == SPS->Log2MaxFrameNum
    // if( slice_type  = =  P  | |  slice_type  = =  B ) {
    //   num_ref_idx_active_override_flag
    //   if( num_ref_idx_active_override_flag ) {
    //     num_ref_idx_l0_active_minus1
    //     if( slice_type  = =  B )
    //       num_ref_idx_l1_active_minus1
    //   }
    // }
    
#endif
    // we always set num_ref_idx_active_override_flag equal to one. this might be done in a more intelligent way 
    if (!pcSlice->isIntra())
    {
      WRITE_FLAG( 1 ,                                             "num_ref_idx_active_override_flag");
      WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) - 1, 3, "num_ref_idx_l0_active_minus1" );
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
    }
    if (pcSlice->isInterB())
    {
      WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) - 1, 3, "num_ref_idx_l1_active_minus1" );
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
    }
#if G1002_RPS
    TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
    if(!pcSlice->isIntra())
    {
      WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0() ? 1 : 0,       "ref_pic_list_modification_flag" );    
      for(Int i = 0; i < refPicListModification->getNumberOfRefPicListModificationsL0(); i++)
      {
        WRITE_UVLC( refPicListModification->getListIdcL0(i), "ref_pic_list_modification_idc");
        WRITE_UVLC( refPicListModification->getRefPicSetIdxL0(i), "ref_pic_set_idx");
      }
      if(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0())
        WRITE_UVLC( 3, "ref_pic_list_modification_idc");
    }
    if(pcSlice->isInterB())
    {    
      WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1() ? 1 : 0,       "ref_pic_list_modification_flag" );
      for(Int i = 0; i < refPicListModification->getNumberOfRefPicListModificationsL1(); i++)
      {
        WRITE_UVLC( refPicListModification->getListIdcL1(i), "ref_pic_list_modification_idc");
        WRITE_UVLC( refPicListModification->getRefPicSetIdxL1(i), "ref_pic_set_idx");
      }
      if(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1())
        WRITE_UVLC( 3, "ref_pic_list_modification_idc");
    }
#endif
  }
#if !G1002_RPS
  // ref_pic_list_modification( )
#endif
  // ref_pic_list_combination( )
  // maybe move to own function?
  if (pcSlice->isInterB())
  {
    WRITE_FLAG(0, "ref_pic_list_combination_flag" );
  }
    
  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //    slice_address
  // the slice start address seems to be aligned with the WD if FINE_GRANULARITY_SLICES is enabled

  //write slice address
  WRITE_FLAG( 1, "first_slice_in_pic_flag" );
  
  // if( !lightweight_slice_flag ) {
  if (!bEntropySlice)
  {
    //   slice_qp_delta
    WRITE_SVLC( pcSlice->getSliceQp(), "slice_qp" ); // this should be delta
  //   if( sample_adaptive_offset_enabled_flag )
  //     sao_param()
  //   if( deblocking_filter_control_present_flag ) {
  //     disable_deblocking_filter_idc
    WRITE_FLAG(pcSlice->getLoopFilterDisable(), "loop_filter_disable");  // should be an IDC

  //     if( disable_deblocking_filter_idc  !=  1 ) {
  //       slice_alpha_c0_offset_div2
  //       slice_beta_offset_div2
  //     }
  //   }
  //   if( slice_type = = B )
  //   collocated_from_l0_flag
    if ( pcSlice->getSliceType() == B_SLICE )
    {
      WRITE_FLAG( pcSlice->getColDir(), "collocated_from_l0_flag" );
    }
    //   if( adaptive_loop_filter_enabled_flag ) {
  //     if( !shared_pps_info_enabled_flag )
  //       alf_param( )
  //     alf_cu_control_param( )
  //   }
  // }
  }
  
  // !!!! sytnax elements not in the WD !!!!
  
  if (!bEntropySlice)
  {
    // ????
    xWriteFlag  (pcSlice->getDRBFlag() ? 1 : 0 );
    if ( !pcSlice->getDRBFlag() )
    {
      // looks like a long-term flag that is currently unused
      xWriteCode  (pcSlice->getERBIndex(), 2);
    }
  }
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  assert(pcSlice->getMaxNumMergeCand()<=MRG_MAX_NUM_CANDS_SIGNALED);
  assert(MRG_MAX_NUM_CANDS_SIGNALED<=MRG_MAX_NUM_CANDS);
  WRITE_UVLC(MRG_MAX_NUM_CANDS - pcSlice->getMaxNumMergeCand(), "maxNumMergeCand");
#endif
}

Void TEncSbac::codeTerminatingBit( UInt uilsLast )
{
  m_pcBinCabac->encodeBinTrm( uilsLast );
}

Void TEncSbac::codeSliceFinish()
{
  m_pcBinCabac->finish();
}

Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcBinCabac->encodeBin( uiSymbol ? 1 : 0, pcSCModel[0] );
  
  if( 0 == uiSymbol)
  {
    return;
  }
  
  while( uiSymbol-- )
  {
    m_pcBinCabac->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }
  
  m_pcBinCabac->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );
  
  if ( uiSymbol == 0 )
  {
    return;
  }
  
  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );
  
  while( --uiSymbol )
  {
    m_pcBinCabac->encodeBin( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    m_pcBinCabac->encodeBin( 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  UInt bins = 0;
  Int numBins = 0;
  
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    bins = 2 * bins + 1;
    numBins++;
    uiSymbol -= 1 << uiCount;
    uiCount  ++;
  }
  bins = 2 * bins + 0;
  numBins++;
  
  bins = (bins << uiCount) | uiSymbol;
  numBins += uiCount;
  
  assert( numBins <= 32 );
  m_pcBinCabac->encodeBinsEP( bins, numBins );
}

/** Coding of coeff_abs_level_minus3
 * \param uiSymbol value of coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
Void TEncSbac::xWriteGoRiceExGolomb( UInt uiSymbol, UInt &ruiGoRiceParam )
{
  UInt uiMaxVlc     = g_auiGoRiceRange[ ruiGoRiceParam ];
  Bool bExGolomb    = ( uiSymbol > uiMaxVlc );
  UInt uiCodeWord   = min<UInt>( uiSymbol, ( uiMaxVlc + 1 ) );
  UInt uiQuotient   = uiCodeWord >> ruiGoRiceParam;
  UInt uiMaxPreLen  = g_auiGoRicePrefixLen[ ruiGoRiceParam ];
  
  UInt binValues;
  Int numBins;
  
  if ( uiQuotient >= uiMaxPreLen )
  {
    numBins = uiMaxPreLen;
    binValues = ( 1 << numBins ) - 1;
  }
  else
  {
    numBins = uiQuotient + 1;
    binValues = ( 1 << numBins ) - 2;
  }
  
  m_pcBinCabac->encodeBinsEP( ( binValues << ruiGoRiceParam ) + uiCodeWord - ( uiQuotient << ruiGoRiceParam ), numBins + ruiGoRiceParam );
  
  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( uiSymbol, 15 ) ];
  
  if( bExGolomb )
  {
    uiSymbol -= uiMaxVlc + 1;
    xWriteEpExGolomb( uiSymbol, 0 );
  }
}

// SBAC RD
Void  TEncSbac::load ( TEncSbac* pSrc)
{
  this->xCopyFrom(pSrc);
}

Void  TEncSbac::loadIntraDirModeLuma( TEncSbac* pSrc)
{
  m_pcBinCabac->copyState( (TEncBinCABAC*)pSrc->m_pcBinCabac );
  
  this->m_cCUIntraPredSCModel      .copyFrom( &pSrc->m_cCUIntraPredSCModel       );
}


Void  TEncSbac::store( TEncSbac* pDest)
{
  pDest->xCopyFrom( this );
}


Void TEncSbac::xCopyFrom( TEncSbac* pSrc )
{
  m_pcBinCabac->copyState( (TEncBinCABAC*)pSrc->m_pcBinCabac );
  
  this->m_uiCoeffCost = pSrc->m_uiCoeffCost;
  this->m_uiLastQp    = pSrc->m_uiLastQp;
  
  memcpy( m_contextModels, pSrc->m_contextModels, m_numContextModels * sizeof( ContextModel ) );
}

Void TEncSbac::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum = AMVP_MAX_NUM_CANDS;

  xWriteUnaryMaxSymbol(iSymbol, m_cMVPIdxSCModel.get(0), 1, iNum-1);
}

Void TEncSbac::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );
#if !PREDTYPE_CLEANUP
  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    {
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
    }
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinCabac->encodeBin( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 3) );
      }
    }
    else
    {
#endif
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 3) );
        m_pcBinCabac->encodeBin( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 4) );
      }
#if DISABLE_4x4_INTER
    }
#endif
    return;
  }
#endif //PREDTYPE_CLEANUP
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      m_pcBinCabac->encodeBin( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
    }
    return;
  }
  
  switch(eSize)
  {
    case SIZE_2Nx2N:
    {
      m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      break;
    }
    case SIZE_2NxN:
#if AMP
    case SIZE_2NxnU:
    case SIZE_2NxnD:
#endif
    {
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );
#if AMP
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_2NxN)
        {
          m_pcBinCabac->encodeBin(1, m_cCUYPosiSCModel.get( 0, 0, 0 ));
        }
        else
        {
          m_pcBinCabac->encodeBin(0, m_cCUYPosiSCModel.get( 0, 0, 0 ));
          m_pcBinCabac->encodeBin((eSize == SIZE_2NxnU? 0: 1), m_cCUYPosiSCModel.get( 0, 0, 1 ));
        }
      }
#endif      
      break;
    }
    case SIZE_Nx2N:
#if AMP
    case SIZE_nLx2N:
    case SIZE_nRx2N:
#endif
    {
#if PREDTYPE_CLEANUP
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
#if DISABLE_4x4_INTER
      if(!pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
#else
      if( (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
#endif
      {
        m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
      }
#if AMP
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_Nx2N)
        {
          m_pcBinCabac->encodeBin(1, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        }
        else
        {
          m_pcBinCabac->encodeBin(0, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          m_pcBinCabac->encodeBin((eSize == SIZE_nLx2N? 0: 1), m_cCUXPosiSCModel.get( 0, 0, 1 ));
        }
      }
#endif
#else //PREDTYPE_CLEANUP
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      if (pcCU->getSlice()->isInterB())
      {
        m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
      }
    }
    else
    {
#endif
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
#if AMP
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
      {
        if (eSize == SIZE_Nx2N)
        {
          m_pcBinCabac->encodeBin(1, m_cCUXPosiSCModel.get( 0, 0, 0 ));
        }
        else
        {
          m_pcBinCabac->encodeBin(0, m_cCUXPosiSCModel.get( 0, 0, 0 ));
          m_pcBinCabac->encodeBin((eSize == SIZE_nLx2N? 0: 1), m_cCUXPosiSCModel.get( 0, 0, 1 ));
        }
      }
#endif      
#if DISABLE_4x4_INTER
    }
#endif      
#endif //PREDTYPE_CLEANUP
      break;
    }
    case SIZE_NxN:
    {
#if DISABLE_4x4_INTER
#if !PREDTYPE_CLEANUP
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      assert(0);
      break;
    }
    else
    {
#else //PREDTYPE_CLEANUP
    if( !pcCU->getSlice()->getSPS()->getDisInter4x4() || (pcCU->getWidth(uiAbsPartIdx)>8) || (pcCU->getHeight(uiAbsPartIdx)>8) )  
    {
#endif //PREDTYPE_CLEANUP
#endif
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
        {
          m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
          m_pcBinCabac->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
        }
#if !PREDTYPE_CLEANUP
        if (pcCU->getSlice()->isInterB())
        {
          m_pcBinCabac->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 3) );
        }
#endif
      }
      break;
#if DISABLE_4x4_INTER
    }
#endif
    }
    default:
    {
      assert(0);
    }
  }
}

/** code prediction mode
 * \param pcCU
 * \param uiAbsPartIdx  
 * \returns Void
 */
Void TEncSbac::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );
#if !PREDTYPE_CLEANUP
  if (pcCU->getSlice()->isInterB() )
  {
    return;
  }
  m_pcBinCabac->encodeBin( iPredMode == MODE_INTER ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
#else //!PREDTYPE_CLEANUP
  m_pcBinCabac->encodeBin( iPredMode == MODE_INTER ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
#endif //!PREDTYPE_CLEANUP
}

/** code skip flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  UInt uiCtxSkip = pcCU->getCtxSkipFlag( uiAbsPartIdx ) ;
  m_pcBinCabac->encodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tSkipFlag" );
  DTRACE_CABAC_T( "\tuiCtxSkip: ");
  DTRACE_CABAC_V( uiCtxSkip );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}

/** code merge flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  const UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinCabac->encodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tMergeFlag: " );
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\tAddress: " );
  DTRACE_CABAC_V( pcCU->getAddr() );
  DTRACE_CABAC_T( "\tuiAbsPartIdx: " );
  DTRACE_CABAC_V( uiAbsPartIdx );
  DTRACE_CABAC_T( "\n" );
}

/** code merge index
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiNumCand = MRG_MAX_NUM_CANDS;
  UInt auiCtx[4] = { 0, 1, 2, 3 };
  UInt uiUnaryIdx = pcCU->getMergeIndex( uiAbsPartIdx );
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  uiNumCand = pcCU->getSlice()->getMaxNumMergeCand();
  if ( uiNumCand > 1 )
  {
#endif
    for( UInt ui = 0; ui < uiNumCand - 1; ++ui )
    {
      const UInt uiSymbol = ui == uiUnaryIdx ? 0 : 1;
      m_pcBinCabac->encodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[ui] ) );
      if( uiSymbol == 0 )
      {
        break;
      }
    }
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  }
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tparseMergeIndex()" );
  DTRACE_CABAC_T( "\tuiMRGIdx= " );
  DTRACE_CABAC_V( pcCU->getMergeIndex( uiAbsPartIdx ) );
  DTRACE_CABAC_T( "\n" );
}

Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;
  
  UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
  
  assert( uiCtx < 3 );
  m_pcBinCabac->encodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  return;
}

Void TEncSbac::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcBinCabac->encodeBin( uiSymbol, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\n" )
}
Void TEncSbac::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
  
  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  

  Int uiPredIdx = -1;

  for(UInt i = 0; i < uiPredNum; i++)
  {
    if(uiDir == uiPreds[i])
    {
      uiPredIdx = i;
    }
  }
 
  if(uiPredIdx != -1)
  {
    m_pcBinCabac->encodeBin( 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
#if BYPASS_FOR_INTRA_MODE
    m_pcBinCabac->encodeBinEP( uiPredIdx );
#else
    m_pcBinCabac->encodeBin( uiPredIdx, m_cCUIntraPredSCModel.get( 0, 0, 2 ) );
#endif
  }
  else
  {
    m_pcBinCabac->encodeBin( 0, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
  
    for(Int i = (uiPredNum - 1); i >= 0; i--)
    {
      uiDir = uiDir > uiPreds[i] ? uiDir - 1 : uiDir;
    }

    if ( uiDir < 31 )
    {
#if BYPASS_FOR_INTRA_MODE
      m_pcBinCabac->encodeBinsEP( uiDir, g_aucIntraModeBitsAng[ iIntraIdx ] - 1 );
#else
      for ( Int i = 0; i < g_aucIntraModeBitsAng[ iIntraIdx ] - 1; i++ )
      {
        m_pcBinCabac->encodeBin( ( uiDir >> i ) & 1, m_cCUIntraPredSCModel.get(0, 0, 1) );            
      }
#endif
    }
    else
    {
#if BYPASS_FOR_INTRA_MODE
      m_pcBinCabac->encodeBinsEP( 31, 5 );
      m_pcBinCabac->encodeBinEP( uiDir - 31 );
#else
      for ( Int i = 0; i < 5; i++ )
      {
        m_pcBinCabac->encodeBin( 1, m_cCUIntraPredSCModel.get(0, 0, 1) );            
      }      
      m_pcBinCabac->encodeBin( uiDir - 31, m_cCUIntraPredSCModel.get(0, 0, 1) );
#endif
    }
   }
  return;
}
Void TEncSbac::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir( uiAbsPartIdx );

  if( uiIntraDirChroma == DM_CHROMA_IDX ) 
  {
    m_pcBinCabac->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
  } 
  else if( uiIntraDirChroma == LM_CHROMA_IDX )
  {
    m_pcBinCabac->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
    m_pcBinCabac->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, 1 ) );
  }
  else
  { 
    UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
    pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );

    for( Int i = 0; i < NUM_CHROMA_MODE - 2; i++ )
    {
      if( uiIntraDirChroma == uiAllowedChromaDir[i] )
      {
        uiIntraDirChroma = i;
        break;
      }
    }
    m_pcBinCabac->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );

    if (pcCU->getSlice()->getSPS()->getUseLMChroma())
    {
      m_pcBinCabac->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 1 ));
    }
    xWriteUnaryMaxSymbol( uiIntraDirChroma, m_cCUChromaPredSCModel.get( 0, 0 ) + 1, 0, 3 );
  }
  return;
}

Void TEncSbac::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  const UInt uiInterDir = pcCU->getInterDir( uiAbsPartIdx ) - 1;
  const UInt uiCtx      = pcCU->getCtxInterDir( uiAbsPartIdx );
  ContextModel *pCtx    = m_cCUInterDirSCModel.get( 0 );
  m_pcBinCabac->encodeBin( uiInterDir == 2 ? 1 : 0, *( pCtx + uiCtx ) );

  if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    assert( uiInterDir != 1 );
    return;
  }

  if ( uiInterDir < 2 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 0 )
  {
    pCtx++;
    m_pcBinCabac->encodeBin( uiInterDir, *( pCtx + 3 ) );
  }
  return;
}

Void TEncSbac::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getInterDir( uiAbsPartIdx ) != 3)
  {
    Int iRefFrame = pcCU->getSlice()->getRefIdxOfLC(eRefList, pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx ));

    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcBinCabac->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), *pCtx );

    if( iRefFrame > 0 )
    {
      xWriteUnaryMaxSymbol( iRefFrame - 1, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );
    }
  }
  else
  {
    Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcBinCabac->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), *pCtx );
    
    if( iRefFrame > 0 )
    {
      xWriteUnaryMaxSymbol( iRefFrame - 1, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
    }
  }
  return;
}

Void TEncSbac::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  const TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
  const Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  const Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
  ContextModel* pCtx = m_cCUMvdSCModel.get( 0 );

  m_pcBinCabac->encodeBin( iHor != 0 ? 1 : 0, *pCtx );
  m_pcBinCabac->encodeBin( iVer != 0 ? 1 : 0, *pCtx );

  const Bool bHorAbsGr0 = iHor != 0;
  const Bool bVerAbsGr0 = iVer != 0;
  const UInt uiHorAbs   = 0 > iHor ? -iHor : iHor;
  const UInt uiVerAbs   = 0 > iVer ? -iVer : iVer;
  pCtx++;

  if( bHorAbsGr0 )
  {
    m_pcBinCabac->encodeBin( uiHorAbs > 1 ? 1 : 0, *pCtx );
  }

  if( bVerAbsGr0 )
  {
    m_pcBinCabac->encodeBin( uiVerAbs > 1 ? 1 : 0, *pCtx );
  }

  if( bHorAbsGr0 )
  {
    if( uiHorAbs > 1 )
    {
      xWriteEpExGolomb( uiHorAbs-2, 1 );
    }

    m_pcBinCabac->encodeBinEP( 0 > iHor ? 1 : 0 );
  }

  if( bVerAbsGr0 )
  {
    if( uiVerAbs > 1 )
    {
      xWriteEpExGolomb( uiVerAbs-2, 1 );
    }

    m_pcBinCabac->encodeBinEP( 0 > iVer ? 1 : 0 );
  }
  
  return;
}

Void TEncSbac::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf     ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
#if CHROMA_CBF_CTX_REDUCTION
  m_pcBinCabac->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, eType ? TEXT_CHROMA : eType, uiCtx ) );
#else
  m_pcBinCabac->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1 : eType, uiCtx ) );
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
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

Void TEncSbac::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  UInt uiCtx = 0;
  m_pcBinCabac->encodeBin( uiCbf , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}

/** Encode (X,Y) position of the last significant coefficient
 * \param uiPosX X component of last coefficient
 * \param uiPosY Y component of last coefficient
 * \param width  Block width
 * \param height Block height
 * \param eTType plane type / luminance or chrominance
 * \param uiScanIdx scan type (zig-zag, hor, ver)
 * This method encodes the X and Y component within a block of the last significant coefficient.
 */
Void TEncSbac::codeLastSignificantXY( UInt uiPosX, UInt uiPosY, Int width, Int height, TextType eTType, UInt uiScanIdx )
{  
  // swap
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosX, uiPosY );
  }

  UInt uiCtxLast;
#if MODIFIED_LAST_XY_CODING
  ContextModel *pCtxX = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY = m_cCuCtxLastY.get( 0, eTType );
  UInt uiGroupIdxX    = g_uiGroupIdx[ uiPosX ];
  UInt uiGroupIdxY    = g_uiGroupIdx[ uiPosY ];

  // posX
  const UInt *puiCtxIdxX = g_uiLastCtx + ( g_aucConvertToBit[ width ] * ( g_aucConvertToBit[ width ] + 3 ) );
  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxX; uiCtxLast++ )
  {
    m_pcBinCabac->encodeBin( 1, *( pCtxX + puiCtxIdxX[ uiCtxLast ] ) );
  }
  if( uiGroupIdxX < g_uiGroupIdx[ width - 1 ])
  {
    m_pcBinCabac->encodeBin( 0, *( pCtxX + puiCtxIdxX[ uiCtxLast ] ) );
  }
#if !BYPASS_FOR_LAST_COEFF_MOD
  if ( uiGroupIdxX > 3 )
  {      
    UInt uiCount = ( uiGroupIdxX - 2 ) >> 1;
    uiPosX       = uiPosX - g_uiMinInGroup[ uiGroupIdxX ];
    for (Int i = uiCount - 1 ; i >= 0; i-- )
    {
      m_pcBinCabac->encodeBinEP( ( uiPosX >> i ) & 1 );
    }
  }
#endif

  // posY
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ height ] * ( g_aucConvertToBit[ height ] + 3 ) );
  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxY; uiCtxLast++ )
  {
    m_pcBinCabac->encodeBin( 1, *( pCtxY + puiCtxIdxY[ uiCtxLast ] ) );
  }
  if( uiGroupIdxY < g_uiGroupIdx[ height - 1 ])
  {
    m_pcBinCabac->encodeBin( 0, *( pCtxY + puiCtxIdxY[ uiCtxLast ] ) );
  }
#if BYPASS_FOR_LAST_COEFF_MOD
  if ( uiGroupIdxX > 3 )
  {      
    UInt uiCount = ( uiGroupIdxX - 2 ) >> 1;
    uiPosX       = uiPosX - g_uiMinInGroup[ uiGroupIdxX ];
    for (Int i = uiCount - 1 ; i >= 0; i-- )
    {
      m_pcBinCabac->encodeBinEP( ( uiPosX >> i ) & 1 );
    }
  }
#endif
  if ( uiGroupIdxY > 3 )
  {      
    UInt uiCount = ( uiGroupIdxY - 2 ) >> 1;
    uiPosY       = uiPosY - g_uiMinInGroup[ uiGroupIdxY ];
    for ( Int i = uiCount - 1 ; i >= 0; i-- )
    {
      m_pcBinCabac->encodeBinEP( ( uiPosY >> i ) & 1 );
    }
  }
#else
  const UInt *puiCtxIdx;
  UInt uiPosX0 = uiPosX;
  UInt uiPosY0 = uiPosY;
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

  if ( halfWidth >= minWidth )
  {
    uiPosX = min<UInt>( uiPosX, halfWidth );
  }
  if ( halfHeight >= minHeight )
  {
    uiPosY = min<UInt>( uiPosY, halfHeight );
  }

  // posX
  puiCtxIdx    = g_uiLastCtx + ( halfWidth >= minWidth ? halfWidth : 0 );

  for( uiCtxLast = 0; uiCtxLast < uiPosX; uiCtxLast++ )
  {
    m_pcBinCabac->encodeBin( 0, *( pCtxX + puiCtxIdx[ uiCtxLast ] ) );
  }
  if( uiPosX < maxWidth - 1 )
  {
    m_pcBinCabac->encodeBin( 1, *( pCtxX + puiCtxIdx[ uiCtxLast ] ) );
  }

#if !BYPASS_FOR_LAST_COEFF_MOD
  if( uiPosX0 >= halfWidth && halfWidth >= minWidth )
  {
    uiPosX0     -= halfWidth;
    UInt uiCount = 0;
    while( uiCount < log2BlkWidth )
    {
      m_pcBinCabac->encodeBinEP( ( uiPosX0 >> uiCount ) & 1 );
      uiCount++;
    }
  }
#endif
  
  // posY
  puiCtxIdx    = g_uiLastCtx + ( halfHeight >= minHeight ? halfHeight : 0 );
  
  for( uiCtxLast = 0; uiCtxLast < uiPosY; uiCtxLast++ )
  {
    m_pcBinCabac->encodeBin( 0, *( pCtxY + puiCtxIdx[ uiCtxLast ] ) );
  }
  if( uiPosY < maxHeight - 1 )
  {
    m_pcBinCabac->encodeBin( 1, *( pCtxY + puiCtxIdx[ uiCtxLast ] ) );
  }

#if !BYPASS_FOR_LAST_COEFF_MOD
  if( uiPosY0 >= halfHeight && halfHeight >= minHeight )
  {
    uiPosY0     -= halfHeight;
    UInt uiCount = 0;
    while( uiCount < log2BlkHeight )
    {
      m_pcBinCabac->encodeBinEP( ( uiPosY0 >> uiCount ) & 1 );
      uiCount++;
    }
  }
#else  
  if ( uiPosX0 >= halfWidth && halfWidth >= minWidth )
  {
    m_pcBinCabac->encodeBinsEP( uiPosX0 - halfWidth, log2BlkWidth );
  }
  if ( uiPosY0 >= halfHeight && halfHeight >= minHeight )
  {
    m_pcBinCabac->encodeBinsEP( uiPosY0 - halfHeight, log2BlkHeight );
  }
#endif
#endif

}

Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
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

#if NSQT && !NSQT_DIAG_SCAN
  Bool bNonSqureFlag = ( uiWidth != uiHeight );
  UInt uiNonSqureScanTableIdx = 0;
  if( bNonSqureFlag )
  {
    UInt uiWidthBit  =  g_aucConvertToBit[ uiWidth ] + 2;
    UInt uiHeightBit =  g_aucConvertToBit[ uiHeight ] + 2;
#if NSQT_TX_ORDER
    uiNonSqureScanTableIdx = ( uiWidth * uiHeight ) == 64 ? 2 * ( uiHeight > uiWidth ) : 2 * ( uiHeight > uiWidth ) + 1;
#else
    uiNonSqureScanTableIdx = ( uiWidth * uiHeight ) == 64 ? 0 : 1;
#endif
    uiWidth  = 1 << ( ( uiWidthBit + uiHeightBit ) >> 1 );
    uiHeight = uiWidth;
  }    
#endif

  if( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  
  UInt uiNumSig = 0;
  
  // compute number of significant coefficients
#if NSQT_DIAG_SCAN
  uiNumSig = TEncEntropy::countNonZeroCoeffs(pcCoef, uiWidth * uiHeight);
#else
  uiNumSig = TEncEntropy::countNonZeroCoeffs(pcCoef, uiWidth);
#endif
  
  if ( uiNumSig == 0 )
    return;
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- encode significance map -----
  const UInt   uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
#if NSQT && !NSQT_DIAG_SCAN
  const UInt   uiMaxNumCoeff     = uiWidth * uiHeight;
#endif
#if DIAG_SCAN
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
  uiScanIdx = ( uiScanIdx == SCAN_ZIGZAG ) ? SCAN_DIAG : uiScanIdx; // Map zigzag to diagonal scan
#else
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif
#if NSQT_DIAG_SCAN
  Int blockType = uiLog2BlockSize;
  if (uiWidth != uiHeight)
  {
    uiScanIdx = SCAN_DIAG;
    blockType = 4;
  }
#endif
  
#if NSQT && !NSQT_DIAG_SCAN
  static TCoeff orgCoeff[ 256 ];
  if( bNonSqureFlag )
  {
#if !NSQT_MOD
    memcpy( &orgCoeff[ 0 ], pcCoef, uiMaxNumCoeff * sizeof( TCoeff ) );
#endif
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
      UInt uiBlkPos = g_auiNonSquareSigLastScan[ uiNonSqureScanTableIdx ][ uiScanPos ];
#if NSQT_MOD
      orgCoeff[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiScanPos ] ] = pcCoef[ uiBlkPos ];
#else
      pcCoef[ g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiScanPos ] ] = orgCoeff[ uiBlkPos ];
#endif
    }
#if NSQT_MOD
    pcCoef = orgCoeff;
#endif
  }
#endif

#if NSQT_DIAG_SCAN
  const UInt * scan;
  if (uiWidth == uiHeight)
  {
    scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize - 1 ];
  }
  else
  {
    scan = g_sigScanNSQT[ uiLog2BlockSize - 2 ];
  }
#else
  const UInt * const scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize - 1 ];
#endif
  
  // Find position of last coefficient
  Int scanPosLast = -1;
  Int posLast;

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  const UInt * scanCG;
  if (uiWidth == uiHeight)
  {
    scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0 ];
  }
  else
  {
    scanCG = g_sigCGScanNSQT[ uiLog2BlockSize - 2 ];
  }
#else
  const UInt * const scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0 ];
#endif
  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
  static const UInt uiShift = MLS_CG_SIZE >> 1;
  const UInt uiNumBlkSide = uiWidth >> uiShift;

#if NSQT_DIAG_SCAN
  if( blockType > 3 )
#else
  if( uiLog2BlockSize > 3 )
#endif
  {
    ::memset( uiSigCoeffGroupFlag, 0, sizeof(UInt) * MLS_GRP_NUM );

    do
    {
      posLast = scan[ ++scanPosLast ];

      // get L1 sig map
      UInt uiPosY    = posLast >> uiLog2BlockSize;
      UInt uiPosX    = posLast - ( uiPosY << uiLog2BlockSize );
      UInt uiBlkIdx  = uiNumBlkSide * (uiPosY >> uiShift) + (uiPosX >> uiShift);
      if( pcCoef[ posLast ] )
      {
        uiSigCoeffGroupFlag[ uiBlkIdx ] = 1;
      }

      uiNumSig -= ( pcCoef[ posLast ] != 0 );
    }
    while ( uiNumSig > 0 );
  }
  else
  {
#endif
  
  do
  {
    posLast = scan[ ++scanPosLast ];
    uiNumSig -= ( pcCoef[ posLast ] != 0 );
  }
  while ( uiNumSig > 0 );

#if MULTI_LEVEL_SIGNIFICANCE
  }
#endif

  // Code position of last coefficient
  Int posLastY = posLast >> uiLog2BlockSize;
  Int posLastX = posLast - ( posLastY << uiLog2BlockSize );
#if NSQT_DIAG_SCAN
  codeLastSignificantXY(posLastX, posLastY, uiWidth, uiHeight, eTType, uiScanIdx);
#else
  codeLastSignificantXY(posLastX, posLastY, uiWidth, uiWidth, eTType, uiScanIdx);
#endif
  
  //===== code significance flag =====
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
#if NSQT_DIAG_SCAN
  if( blockType > 3 )
#else
  if( uiLog2BlockSize > 3 )
#endif
  {
    // encode significant coefficient group flag
    Int iCGLastScanPos = scanPosLast >> MLS_CG_SIZE;
    static const UInt uiCGSize = 1 << MLS_CG_SIZE;

    for( Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos-- )
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
#if NSQT_DIAG_SCAN
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight );
#else
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize );
#endif
          UInt uiSig     = (uiSigCoeffGroupFlag[ iCGBlkPos ] != 0);
          m_pcBinCabac->encodeBin( uiSig, baseCoeffGroupCtx[ uiCtxSig ] );
        }
        else
        {
          uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
          bInferredCGFlag = true;
        }
      }
      
      // encode significant coefficient flag      
      if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
      {
        Int  iScanPos;
        UInt uiBlkPos, uiPosY, uiPosX, uiSig, uiCtxSig;
        UInt uiNumNonZeroesInCG = (iCGScanPos == iCGLastScanPos)? 1 : 0;

        for (Int iScanPosOffset = (iCGScanPos == iCGLastScanPos)? scanPosLast - iCGScanPos*uiCGSize - 1 : uiCGSize - 1; 
           iScanPosOffset >= 0; iScanPosOffset--)
        {
          iScanPos  = iCGScanPos*uiCGSize + iScanPosOffset;
          uiBlkPos  = scan[ iScanPos ];
          uiPosY    = uiBlkPos >> uiLog2BlockSize;
          uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
      
          if (iScanPosOffset > 0 || bInferredCGFlag || uiNumNonZeroesInCG )
          {
            uiSig     = (pcCoef[ uiBlkPos ] != 0);
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
            m_pcBinCabac->encodeBin( uiSig, baseCtx[ uiCtxSig ] );
            
            uiNumNonZeroesInCG += uiSig;
          }
        }
      } // end if ( uiSigCoeffGroupFlag[ iCGBlkPos ] )
    } // end for( Int iCGScanPos = iCGLastScanPos; iCGScanPos >= 0; iCGScanPos-- )
  }
  else
  {
#endif

  for( UInt uiScanPos = scanPosLast-1; uiScanPos != -1; uiScanPos-- )
  {
    UInt  uiBlkPos  = scan[ uiScanPos ]; 
    UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
    UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
    UInt  uiSig     = pcCoef[ uiBlkPos ] != 0 ? 1 : 0;
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
    m_pcBinCabac->encodeBin( uiSig, baseCtx[ uiCtxSig ] );
  }

#if MULTI_LEVEL_SIGNIFICANCE
  }
#endif
#endif

  const Int  iLastScanSet      = scanPosLast >> LOG2_SCAN_SET_SIZE;
  UInt uiNumOne                = 0;
  UInt uiGoRiceParam           = 0;
#if UNIFIED_SCAN_PASSES
  Int  iScanPosSig             = scanPosLast;
#endif

  for( Int iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- )
  {
    Int numNonZero = 0;
    Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
    uiGoRiceParam    = 0;
    Int absCoeff[16];
    UInt coeffSigns = 0;
    
#if UNIFIED_SCAN_PASSES
    if( iScanPosSig == scanPosLast )
    {
      absCoeff[ 0 ] = abs( pcCoef[ posLast ] );
      coeffSigns    = ( pcCoef[ posLast ] < 0 );
      numNonZero    = 1;
      iScanPosSig--;
    }

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
    if( blockType > 3 )
#else
    if( uiLog2BlockSize > 3 )
#endif
    {
      // encode significant_coeffgroup_flag
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
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight ) && ( iSubSet ) )
#else
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize ) && ( iSubSet ) )
#endif
        {
          UInt uiSigCoeffGroup   = (uiSigCoeffGroupFlag[ iCGBlkPos ] != 0);
#if NSQT_DIAG_SCAN
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight );
#else
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiLog2BlockSize );
#endif
          m_pcBinCabac->encodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
        }
        else
        {
          uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
          bInferredCGFlag = true;
        }
      }
      
      // encode significant_coeff_flag
      if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
      {
        UInt uiBlkPos, uiPosY, uiPosX, uiSig, uiCtxSig;
        for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
        {
          uiBlkPos  = scan[ iScanPosSig ]; 
          uiPosY    = uiBlkPos >> uiLog2BlockSize;
          uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
          uiSig     = (pcCoef[ uiBlkPos ] != 0);

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
            m_pcBinCabac->encodeBin( uiSig, baseCtx[ uiCtxSig ] );
          }
          if( uiSig )
          {
            absCoeff[ numNonZero ] = abs( pcCoef[ uiBlkPos ] );
            coeffSigns = 2 * coeffSigns + ( pcCoef[ uiBlkPos ] < 0 );
            numNonZero++;
          }
        }
      }
      else
      {
        iScanPosSig = iSubPos - 1;
      }
    } 
    else
    {
#endif

    for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
    {
      UInt  uiBlkPos  = scan[ iScanPosSig ]; 
      UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
      UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
      UInt  uiSig     = 0; 
      if( pcCoef[ uiBlkPos ] != 0 )
      {
        uiSig = 1;
        absCoeff[ numNonZero ] = abs( pcCoef[ uiBlkPos ] );
        coeffSigns = 2 * coeffSigns + ( pcCoef[ uiBlkPos ] < 0 );
        numNonZero++;
      }      
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
      m_pcBinCabac->encodeBin( uiSig, baseCtx[ uiCtxSig ] );
    }

#if MULTI_LEVEL_SIGNIFICANCE
    }
#endif
#else
    const UInt * const puiSetScan  = scan + iSubPos;
    for( Int iScanPos = SCAN_SET_SIZE-1; iScanPos >= 0; iScanPos-- )
    {
      UInt uiBlkPos = puiSetScan[ iScanPos ];
      if( pcCoef[ uiBlkPos ] )
      {
        absCoeff[ numNonZero ] = abs( pcCoef[ uiBlkPos ] );
        coeffSigns = 2 * coeffSigns + ( pcCoef[ uiBlkPos ] < 0 );
        numNonZero++;
      }
    }
#endif
    
    if( numNonZero > 0 )
    {
      UInt c1 = 1;
      UInt c2 = 0;
#if COEFF_CTXSET_RED
      UInt uiCtxSet = (iSubSet > 0 && eTType==TEXT_LUMA) ? 3 : 0;
#else
      UInt uiCtxSet = iSubSet > 0 ? 3 : 0;
#endif
      
      if( uiNumOne > 0 )
      {
        uiCtxSet++;
#if COEFF_CTXSET_RED
        if(eTType==TEXT_LUMA)
        {
#endif
        if( uiNumOne > 3 )
        {
          uiCtxSet++;
        }
#if COEFF_CTXSET_RED
        }
#endif
      }
      
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
      
      for ( Int idx = 0; idx < numNonZero; idx++ )
      {
        UInt uiSymbol = absCoeff[ idx ] > 1;
        m_pcBinCabac->encodeBin( uiSymbol, baseCtxMod[c1] );
        if( uiSymbol )
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
        for ( Int idx = 0; idx < numNonZero; idx++ )
        {
          if( absCoeff[ idx ] > 1 )
          {
            UInt symbol = absCoeff[ idx ] > 2;
            m_pcBinCabac->encodeBin( symbol, baseCtxMod[c2] );
#if COEFF_CTX_RED
            c2 += (c2 < 2);
#else
            c2 += (c2 < 4); // Increment c2 up to a maximum value of 4
#endif
            uiNumOne++;
          }
        }
      }
      
      m_pcBinCabac->encodeBinsEP( coeffSigns, numNonZero );
      
      if (c1 == 0)
      {
        for ( Int idx = 0; idx < numNonZero; idx++ )
        {
          if ( absCoeff[ idx ] > 2 )
          {
            xWriteGoRiceExGolomb( absCoeff[ idx ]  - 3, uiGoRiceParam );            
          }
        }        
      }
    }
    else
    {
      uiNumOne >>= 1;
    }
  }
  return;
}

/*!
 ****************************************************************************
 * \brief
 *   estimate bit cost for CBP, significant map and significant coefficients
 ****************************************************************************
 */
#if NSQT_DIAG_SCAN
Void TEncSbac::estBit( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType )
#else
Void TEncSbac::estBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
#endif
{
  estCBFBit( pcEstBitsSbac, 0, eTType );

#if MULTI_LEVEL_SIGNIFICANCE
#if NSQT_DIAG_SCAN
  estSignificantCoeffGroupMapBit( pcEstBitsSbac, 0, eTType );
#else
  estSignificantCoeffGroupMapBit( pcEstBitsSbac, uiCTXIdx, eTType );
#endif
#endif
  
  // encode significance map
#if NSQT_DIAG_SCAN
  estSignificantMapBit( pcEstBitsSbac, width, height, eTType );
#else
  estSignificantMapBit( pcEstBitsSbac, uiCTXIdx, eTType );
#endif
  
  // encode significant coefficients
#if NSQT_DIAG_SCAN
  estSignificantCoefficientsBit( pcEstBitsSbac, 0, eTType );
#else
  estSignificantCoefficientsBit( pcEstBitsSbac, uiCTXIdx, eTType );
#endif
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost for each CBP bit
 ****************************************************************************
 */
Void TEncSbac::estCBFBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  ContextModel *pCtx = m_cCUQtCbfSCModel.get( 0 );

  for( UInt uiCtxInc = 0; uiCtxInc < 3*NUM_QT_CBF_CTX; uiCtxInc++ )
  {
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 0 ] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 1 ] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }

  pCtx = m_cCUQtRootCbfSCModel.get( 0 );
  
  for( UInt uiCtxInc = 0; uiCtxInc < 4; uiCtxInc++ )
  {
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 0 ] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 1 ] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}


#if MULTI_LEVEL_SIGNIFICANCE
/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient group map
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoeffGroupMapBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  Int firstCtx = 0, numCtx = NUM_SIG_CG_FLAG_CTX;

  for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
  {
    for( UInt uiBin = 0; uiBin < 2; uiBin++ )
    {
      pcEstBitsSbac->significantCoeffGroupBits[ ctxIdx ][ uiBin ] = m_cCUSigCoeffGroupSCModel.get(  0, eTType, ctxIdx ).getEntropyBits( uiBin );
    }
  }
}
#endif


/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient map
 ****************************************************************************
 */
#if NSQT_DIAG_SCAN
Void TEncSbac::estSignificantMapBit( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType )
#else
Void TEncSbac::estSignificantMapBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
#endif
{
#if NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
  Int firstCtx = 0, numCtx = (eTType == TEXT_LUMA) ? 9 : 6;
  if (std::max(width, height) >= 16)
  {
    firstCtx = (eTType == TEXT_LUMA) ? 20 : 17;
    numCtx = (eTType == TEXT_LUMA) ? 7 : 4;    
  }
  else if (width == 8)
  {
    firstCtx = (eTType == TEXT_LUMA) ? 9 : 6;
    numCtx = 11;
  }
#else
  Int firstCtx = 0, numCtx = 15;
  if (std::max(width, height) >= 16)
  {
    firstCtx = 31;
    numCtx = 13;    
  }
  else if (width == 8)
  {
    firstCtx = 15;
    numCtx = 16;
  }
#endif
#else // NSQT_DIAG_SCAN
#if SIGMAP_CTX_RED
  Int firstCtx, numCtx = (eTType == TEXT_LUMA) ? 9 : 6;
#else
  Int firstCtx, numCtx = 15;
#endif
  switch (uiCTXIdx)
  {
    case 2: // 32x32
    case 3: // 16x16
#if SIGMAP_CTX_RED
    if (eTType == TEXT_LUMA)
    {
      firstCtx = 20;
      numCtx = 7;
    }
    else
    {
      firstCtx = 17;
      numCtx = 4;
    }
#else
      firstCtx = 31;
      numCtx = 13;
#endif
      break;
    case 4: // 8x8
#if SIGMAP_CTX_RED
    firstCtx = (eTType == TEXT_LUMA) ? 9 : 6;
    numCtx = 11;
#else
      firstCtx = 15;
      numCtx = 16;
#endif
      break;
    default: // 4x4 (case 5)
      firstCtx = 0;
      break;
  }
#endif
  
#if SIGMAP_CTX_RED
  if (eTType == TEXT_LUMA )
  {
    for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
    {
      for( UInt uiBin = 0; uiBin < 2; uiBin++ )
      {
         pcEstBitsSbac->significantBits[ ctxIdx ][ uiBin ] = m_cCUSigSCModelLuma.get(  0, 0, ctxIdx ).getEntropyBits( uiBin );
      }
    }
  }
  else
  {
    for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
    {
      for( UInt uiBin = 0; uiBin < 2; uiBin++ )
      {
        pcEstBitsSbac->significantBits[ ctxIdx ][ uiBin ] = m_cCUSigSCModelChroma.get(  0, 0, ctxIdx ).getEntropyBits( uiBin );
      }
    }
  }
#else
  for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
  {
    for( UInt uiBin = 0; uiBin < 2; uiBin++ )
    {
      pcEstBitsSbac->significantBits[ ctxIdx ][ uiBin ] = m_cCUSigSCModel.get(  0, eTType, ctxIdx ).getEntropyBits( uiBin );
    }
  }
#endif
  Int iBitsX = 0, iBitsY = 0;
#if NSQT_DIAG_SCAN
  const UInt *puiCtxIdx;
  Int ctx;
  
#if MODIFIED_LAST_XY_CODING
  puiCtxIdx = g_uiLastCtx + (g_aucConvertToBit[ width ]*(g_aucConvertToBit[ width ]+3));
#else
  Int minWidth = std::min(4, width);
  Int halfWidth = width >> 1;
  puiCtxIdx = g_uiLastCtx + ( halfWidth >= minWidth ? halfWidth : 0 );
  Int widthMinus1 = std::max( minWidth, halfWidth + 1 ) - 1;
#endif
  ContextModel *pCtxX      = m_cCuCtxLastX.get( 0, eTType );
#if MODIFIED_LAST_XY_CODING
  for (ctx = 0; ctx < g_uiGroupIdx[ width - 1 ]; ctx++)
#else
  for (ctx = 0; ctx < widthMinus1; ctx++)
#endif
  {
    Int ctxOffset = puiCtxIdx[ ctx ];
#if MODIFIED_LAST_XY_CODING
    pcEstBitsSbac->lastXBits[ ctx ] = iBitsX + pCtxX[ ctxOffset ].getEntropyBits( 0 );
    iBitsX += pCtxX[ ctxOffset ].getEntropyBits( 1 );
#else
    pcEstBitsSbac->lastXBits[ ctx ] = iBitsX + pCtxX[ ctxOffset ].getEntropyBits( 1 );
    iBitsX += pCtxX[ ctxOffset ].getEntropyBits( 0 );
#endif
  }
  pcEstBitsSbac->lastXBits[ctx] = iBitsX;

#if MODIFIED_LAST_XY_CODING
  puiCtxIdx = g_uiLastCtx + (g_aucConvertToBit[ height ]*(g_aucConvertToBit[ height ]+3));
#else
  Int minHeight = std::min(4, height);
  Int halfHeight = height >> 1;
  puiCtxIdx = g_uiLastCtx + ( halfHeight >= minHeight ? halfHeight : 0 );
  Int heightMinus1 = std::max( minHeight, halfHeight + 1 ) - 1;
#endif
  ContextModel *pCtxY      = m_cCuCtxLastY.get( 0, eTType );
#if MODIFIED_LAST_XY_CODING
  for (ctx = 0; ctx < g_uiGroupIdx[ height - 1 ]; ctx++)
#else
  for (ctx = 0; ctx < heightMinus1; ctx++)
#endif
  {
    Int ctxOffset = puiCtxIdx[ ctx ];
#if MODIFIED_LAST_XY_CODING
    pcEstBitsSbac->lastYBits[ ctx ] = iBitsY + pCtxY[ ctxOffset ].getEntropyBits( 0 );
    iBitsY += pCtxY[ ctxOffset ].getEntropyBits( 1 );
#else
    pcEstBitsSbac->lastYBits[ ctx ] = iBitsY + pCtxY[ ctxOffset ].getEntropyBits( 1 );
    iBitsY += pCtxY[ ctxOffset ].getEntropyBits( 0 );
#endif
  }
  pcEstBitsSbac->lastYBits[ctx] = iBitsY;
#else // NSQT_DIAG_SCAN
  const UInt uiWidth       = ( 1 << ( 7 - uiCTXIdx ) );
#if MODIFIED_LAST_XY_CODING
  const UInt *puiCtxIdx    = g_uiLastCtx + (g_aucConvertToBit[ uiWidth ]*(g_aucConvertToBit[ uiWidth ]+3));
  const UInt uiWidthM1     = g_uiGroupIdx[uiWidth-1];
#else
  const UInt uiMinWidth    = min<UInt>( 4, uiWidth );
  const UInt uiHalfWidth   = uiWidth >> 1;
  const UInt *puiCtxIdx    = g_uiLastCtx + ( uiHalfWidth >= uiMinWidth ? uiHalfWidth : 0 );
  const UInt uiWidthM1     = max<UInt>( uiMinWidth, uiHalfWidth + 1 ) - 1;
#endif
  ContextModel *pCtxX      = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY      = m_cCuCtxLastY.get( 0, eTType );
  for ( UInt uiCtx = 0; uiCtx < uiWidthM1; uiCtx++ )
  {
    Int ctxOffset = puiCtxIdx[ uiCtx ];
#if MODIFIED_LAST_XY_CODING
    pcEstBitsSbac->lastXBits[ uiCtx ] = iBitsX + pCtxX[ ctxOffset ].getEntropyBits( 0 );
    pcEstBitsSbac->lastYBits[ uiCtx ] = iBitsY + pCtxY[ ctxOffset ].getEntropyBits( 0 );
    iBitsX += pCtxX[ ctxOffset ].getEntropyBits( 1 );
    iBitsY += pCtxY[ ctxOffset ].getEntropyBits( 1 );
#else
    pcEstBitsSbac->lastXBits[ uiCtx ] = iBitsX + pCtxX[ ctxOffset ].getEntropyBits( 1 );
    pcEstBitsSbac->lastYBits[ uiCtx ] = iBitsY + pCtxY[ ctxOffset ].getEntropyBits( 1 );
    iBitsX += pCtxX[ ctxOffset ].getEntropyBits( 0 );
    iBitsY += pCtxY[ ctxOffset ].getEntropyBits( 0 );
#endif
  }
  pcEstBitsSbac->lastXBits[uiWidthM1] = iBitsX;
  pcEstBitsSbac->lastYBits[uiWidthM1] = iBitsY;
#endif // NSQT_DIAG_SCAN
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoefficientsBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
#if COEFF_CTXSET_RED
  if (eTType==TEXT_LUMA)
  {
    ContextModel *ctxOne = m_cCUOneSCModelLuma.get(0, 0);
    ContextModel *ctxAbs = m_cCUAbsSCModelLuma.get(0, 0);

    for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX_LUMA; ctxIdx++)
    {
      pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 0 ] = ctxOne[ ctxIdx ].getEntropyBits( 0 );
      pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 1 ] = ctxOne[ ctxIdx ].getEntropyBits( 1 );    
    }

    for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX_LUMA; ctxIdx++)
    {
      pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 0 ] = ctxAbs[ ctxIdx ].getEntropyBits( 0 );
      pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 1 ] = ctxAbs[ ctxIdx ].getEntropyBits( 1 );    
    }
  }
  else
  {
    ContextModel *ctxOne = m_cCUOneSCModelChroma.get(0, 0);
    ContextModel *ctxAbs = m_cCUAbsSCModelChroma.get(0, 0);

    for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX_CHROMA; ctxIdx++)
    {
      pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 0 ] = ctxOne[ ctxIdx ].getEntropyBits( 0 );
      pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 1 ] = ctxOne[ ctxIdx ].getEntropyBits( 1 );    
    }

    for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX_CHROMA; ctxIdx++)
    {
      pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 0 ] = ctxAbs[ ctxIdx ].getEntropyBits( 0 );
      pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 1 ] = ctxAbs[ ctxIdx ].getEntropyBits( 1 );    
    }
  }
#else
  ContextModel *ctxOne = m_cCUOneSCModel.get(0, eTType);
  ContextModel *ctxAbs = m_cCUAbsSCModel.get(0, eTType);
  
  for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX; ctxIdx++)
  {
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 0 ] = ctxOne[ ctxIdx ].getEntropyBits( 0 );
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 1 ] = ctxOne[ ctxIdx ].getEntropyBits( 1 );    
  }
  
  for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX; ctxIdx++)
  {
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 0 ] = ctxAbs[ ctxIdx ].getEntropyBits( 0 );
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 1 ] = ctxAbs[ ctxIdx ].getEntropyBits( 1 );    
  }
#endif
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncSbac::xWriteCode     ( UInt uiCode, UInt uiLength )
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncSbac::xWriteUvlc     ( UInt uiCode )
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;
  
  assert ( uiTemp );
  
  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  //m_pcBitIf->write( uiCode, uiLength );
  // Take care of cases where uiLength > 32
  m_pcBitIf->write( 0, uiLength >> 1);
  m_pcBitIf->write( uiCode, (uiLength+1) >> 1);
}

Void TEncSbac::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;
  
  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
}

Void TEncSbac::xWriteFlag( UInt uiCode )
{
  m_pcBitIf->write( uiCode, 1 );
}

#if G1002_RPS
Void TEncSbac::codeShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* pcRPS )
{
#if PRINT_RPS_BITS_WRITTEN
  int lastBits = getNumberOfWrittenBits();
#endif
#if INTER_RPS_PREDICTION
  WRITE_FLAG( pcRPS->getInterRPSPrediction(), "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
  if (pcRPS->getInterRPSPrediction()) 
  {
    //Int rIdx = i - pcRPS->getDeltaRIdxMinus1() - 1;
    Int deltaRPS = pcRPS->getDeltaRPS();
    //assert (rIdx <= i);
    WRITE_UVLC( pcRPS->getDeltaRIdxMinus1(), "delta_idx_minus1" ); // delta index of the Reference Picture Set used for prediction minus 1
    WRITE_CODE( (deltaRPS >=0 ? 0: 1), 1, "delta_rps_sign" ); //delta_rps_sign
    WRITE_UVLC( abs(deltaRPS) - 1, "abs_delta_rps_minus1"); // absolute delta RPS minus 1

    for(Int j=0; j < pcRPS->getNumRefIdc(); j++)
    {
      Int refIdc = pcRPS->getRefIdc(j);
      WRITE_CODE( (refIdc==1? 1: 0), 1, "ref_idc0" ); //first bit is "1" if Idc is 1 
      if (refIdc != 1) 
      {
        WRITE_CODE( refIdc>>1, 1, "ref_idc1" ); //second bit is "1" if Idc is 2, "0" otherwise.
      }
    }
  }
  else
  {
#endif //INTER_RPS_PREDICTION
    WRITE_UVLC( pcRPS->getNumberOfNegativePictures(), "num_negative_pics" );
    WRITE_UVLC( pcRPS->getNumberOfPositivePictures(), "num_positive_pics" );
    Int prev = 0;
    for(Int j=0 ; j < pcRPS->getNumberOfNegativePictures(); j++)
    {
      WRITE_UVLC( prev-pcRPS->getDeltaPOC(j)-1, "delta_poc_s0_minus1" );
      prev = pcRPS->getDeltaPOC(j);
      WRITE_FLAG( pcRPS->getUsed(j), "used_by_curr_pic_s0_flag"); 
    }
    prev = 0;
    for(Int j=pcRPS->getNumberOfNegativePictures(); j < pcRPS->getNumberOfNegativePictures()+pcRPS->getNumberOfPositivePictures(); j++)
    {
      WRITE_UVLC( pcRPS->getDeltaPOC(j)-prev-1, "delta_poc_s1_minus1" );
      prev = pcRPS->getDeltaPOC(j);
      WRITE_FLAG( pcRPS->getUsed(j), "used_by_curr_pic_s1_flag" ); 
    }
#if INTER_RPS_PREDICTION
  }
#endif // INTER_RPS_PREDICTION

#if PRINT_RPS_BITS_WRITTEN
#if INTER_RPS_PREDICTION  
  printf("irps=%d (%2d bits) ", pcRPS->getInterRPSPrediction(), getNumberOfWrittenBits() - lastBits);
#else
  printf("(%2d bits) ", getNumberOfWrittenBits() - lastBits);
#endif
  pcRPS->printDeltaPOC();
#endif
}
#endif

//! \}
