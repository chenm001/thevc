/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "../TLibCommon/CommonDef.h"
#include "TEncCavlc.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

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

Void  xTraceAPSHeader (TComAPS *pAPS)
{
  fprintf( g_hTrace, "=========== Adaptation Parameter Set ===========\n");
}

Void  xTraceSliceHeader (TComSlice *pSlice)
{
  fprintf( g_hTrace, "=========== Slice ===========\n");
}


Void  TEncCavlc::xWriteCodeTr (UInt value, UInt  length, const Char *pSymbolName)
{
  xWriteCode (value,length);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, value ); 
}

Void  TEncCavlc::xWriteUvlcTr (UInt value, const Char *pSymbolName)
{
  xWriteUvlc (value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, value ); 
}

Void  TEncCavlc::xWriteSvlcTr (Int value, const Char *pSymbolName)
{
  xWriteSvlc(value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, value ); 
}

Void  TEncCavlc::xWriteFlagTr(UInt value, const Char *pSymbolName)
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



// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
  m_uiCoeffCost       = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
  
  m_iSliceGranularity = 0;
}

TEncCavlc::~TEncCavlc()
{
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy()
{
}

/**
 * marshall the SEI message sei.
 */
void TEncCavlc::codeSEI(const SEI& sei)
{
  writeSEImessage(*m_pcBitIf, sei);
}

Void  TEncCavlc::codeAPSInitInfo(TComAPS* pcAPS)
{

#if ENC_DEC_TRACE  
  xTraceAPSHeader(pcAPS);
#endif
  //APS ID
  WRITE_UVLC( pcAPS->getAPSID(), "aps_id" );

  WRITE_FLAG( pcAPS->getScalingListEnabled()?1:0, "aps_scaling_list_data_present_flag");
  //DF flag
  WRITE_FLAG(pcAPS->getLoopFilterOffsetInAPS()?1:0, "aps_deblocking_filter_flag");
#if !SAO_UNIT_INTERLEAVING
  //SAO flag
  WRITE_FLAG( pcAPS->getSaoEnabled()?1:0, "aps_sample_adaptive_offset_flag"); 
#endif
#if !LCU_SYNTAX_ALF
  //ALF flag
  WRITE_FLAG( pcAPS->getAlfEnabled()?1:0, "aps_adaptive_loop_filter_flag"); 
#endif
}
#if LCU_SYNTAX_ALF
Void TEncCavlc::codeAPSAlflag(UInt uiCode)
{
  WRITE_FLAG(uiCode, "aps_adaptive_loop_filter_flag");
}
#endif

Void TEncCavlc::codeDFFlag(UInt uiCode, const Char *pSymbolName)
{
  WRITE_FLAG(uiCode, pSymbolName);
}
Void TEncCavlc::codeDFSvlc(Int iCode, const Char *pSymbolName)
{
  WRITE_SVLC(iCode, pSymbolName);
}

#if RPS_IN_SPS
Void TEncCavlc::codeShortTermRefPicSet( TComSPS* pcSPS, TComReferencePictureSet* rps )
#else
Void TEncCavlc::codeShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* rps )
#endif
{
#if PRINT_RPS_INFO
  int lastBits = getNumberOfWrittenBits();
#endif
  WRITE_FLAG( rps->getInterRPSPrediction(), "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
  if (rps->getInterRPSPrediction()) 
  {
    Int deltaRPS = rps->getDeltaRPS();
    WRITE_UVLC( rps->getDeltaRIdxMinus1(), "delta_idx_minus1" ); // delta index of the Reference Picture Set used for prediction minus 1
    WRITE_CODE( (deltaRPS >=0 ? 0: 1), 1, "delta_rps_sign" ); //delta_rps_sign
    WRITE_UVLC( abs(deltaRPS) - 1, "abs_delta_rps_minus1"); // absolute delta RPS minus 1

    for(Int j=0; j < rps->getNumRefIdc(); j++)
    {
      Int refIdc = rps->getRefIdc(j);
      WRITE_CODE( (refIdc==1? 1: 0), 1, "used_by_curr_pic_flag" ); //first bit is "1" if Idc is 1 
      if (refIdc != 1) 
      {
        WRITE_CODE( refIdc>>1, 1, "use_delta_flag" ); //second bit is "1" if Idc is 2, "0" otherwise.
      }
    }
  }
  else
  {
    WRITE_UVLC( rps->getNumberOfNegativePictures(), "num_negative_pics" );
    WRITE_UVLC( rps->getNumberOfPositivePictures(), "num_positive_pics" );
    Int prev = 0;
    for(Int j=0 ; j < rps->getNumberOfNegativePictures(); j++)
    {
      WRITE_UVLC( prev-rps->getDeltaPOC(j)-1, "delta_poc_s0_minus1" );
      prev = rps->getDeltaPOC(j);
      WRITE_FLAG( rps->getUsed(j), "used_by_curr_pic_s0_flag"); 
    }
    prev = 0;
    for(Int j=rps->getNumberOfNegativePictures(); j < rps->getNumberOfNegativePictures()+rps->getNumberOfPositivePictures(); j++)
    {
      WRITE_UVLC( rps->getDeltaPOC(j)-prev-1, "delta_poc_s1_minus1" );
      prev = rps->getDeltaPOC(j);
      WRITE_FLAG( rps->getUsed(j), "used_by_curr_pic_s1_flag" ); 
    }
  }

#if PRINT_RPS_INFO
  printf("irps=%d (%2d bits) ", rps->getInterRPSPrediction(), getNumberOfWrittenBits() - lastBits);
  rps->printDeltaPOC();
#endif
}


Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
#if !RPS_IN_SPS
  TComRPSList* rpsList = pcPPS->getRPSList();
#endif
  
  WRITE_UVLC( pcPPS->getPPSId(),                             "pic_parameter_set_id" );
  WRITE_UVLC( pcPPS->getSPSId(),                             "seq_parameter_set_id" );

#if MULTIBITS_DATA_HIDING
  WRITE_FLAG( pcPPS->getSignHideFlag(), "sign_data_hiding_flag" );
  if( pcPPS->getSignHideFlag() )
  {
    WRITE_CODE(pcPPS->getTSIG(), 4, "sign_hiding_threshold");
  }
#endif
#if CABAC_INIT_FLAG
  WRITE_FLAG( pcPPS->getCabacInitPresentFlag() ? 1 : 0,   "cabac_init_present_flag" );
#endif
#if !RPS_IN_SPS
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD
  TComReferencePictureSet*      rps;

  WRITE_UVLC(rpsList->getNumberOfReferencePictureSets(), "num_short_term_ref_pic_sets" );
  for(UInt i=0; i < rpsList->getNumberOfReferencePictureSets(); i++)
  {
    rps = rpsList->getReferencePictureSet(i);
    codeShortTermRefPicSet(pcPPS,rps);
  }
  WRITE_FLAG( pcPPS->getLongTermRefsPresent() ? 1 : 0,         "long_term_ref_pics_present_flag" );
#endif
  // entropy_coding_mode_flag
  // We code the entropy_coding_mode_flag, it's needed for tests.
  WRITE_FLAG( pcPPS->getEntropyCodingMode() ? 1 : 0,         "entropy_coding_mode_flag" );
  if (pcPPS->getEntropyCodingMode())
  {
#if !WPP_SIMPLIFICATION
    WRITE_UVLC( pcPPS->getEntropyCodingSynchro(),            "entropy_coding_synchro" );
    WRITE_FLAG( pcPPS->getCabacIstateReset() ? 1 : 0,        "cabac_istate_reset" );
#endif
#if !TILES_OR_ENTROPY_SYNC_IDC
#if !WPP_SIMPLIFICATION
    if ( pcPPS->getEntropyCodingSynchro() )
#endif
    {
      WRITE_UVLC( pcPPS->getNumSubstreams()-1,               "num_substreams_minus1" );
    }
#endif
  }
#if !H0566_TLA
  WRITE_UVLC( pcPPS->getNumTLayerSwitchingFlags(),           "num_temporal_layer_switching_point_flags" );
  for( UInt i = 0; i < pcPPS->getNumTLayerSwitchingFlags(); i++ ) 
  {
    WRITE_FLAG( pcPPS->getTLayerSwitchingFlag( i ) ? 1 : 0 , "temporal_layer_switching_point_flag" ); 
  }
#endif
  WRITE_CODE( pcPPS->getNumRefIdxL0DefaultActive()-1, 3, "num_ref_idx_l0_default_active_minus1");
  WRITE_CODE( pcPPS->getNumRefIdxL1DefaultActive()-1, 3, "num_ref_idx_l1_default_active_minus1");

  WRITE_SVLC( pcPPS->getPicInitQPMinus26(),                  "pic_init_qp_minus26");
  WRITE_FLAG( pcPPS->getConstrainedIntraPred() ? 1 : 0,      "constrained_intra_pred_flag" );
  WRITE_FLAG( pcPPS->getEnableTMVPFlag() ? 1 : 0,            "enable_temporal_mvp_flag" );
  WRITE_CODE( pcPPS->getSliceGranularity(), 2,               "slice_granularity");
  WRITE_UVLC( pcPPS->getMaxCuDQPDepth() + pcPPS->getUseDQP(),                   "max_cu_qp_delta_depth" );

  WRITE_SVLC( pcPPS->getChromaQpOffset(),                   "chroma_qp_offset"     );
  WRITE_SVLC( pcPPS->getChromaQpOffset2nd(),                "chroma_qp_offset_2nd" );

  WRITE_FLAG( pcPPS->getUseWP() ? 1 : 0,  "weighted_pred_flag" );   // Use of Weighting Prediction (P_SLICE)
  WRITE_CODE( pcPPS->getWPBiPredIdc(), 2, "weighted_bipred_idc" );  // Use of Weighting Bi-Prediction (B_SLICE)
#if H0388
  WRITE_FLAG( pcPPS->getOutputFlagPresentFlag() ? 1 : 0,  "output_flag_present_flag" );
#endif
#if TILES_OR_ENTROPY_SYNC_IDC
  if(pcPPS->getSPS()->getTilesOrEntropyCodingSyncIdc()==1)
  {
#endif
    WRITE_FLAG( pcPPS->getColumnRowInfoPresent(),           "tile_info_present_flag" );
    WRITE_FLAG( pcPPS->getTileBehaviorControlPresentFlag(),  "tile_control_present_flag");
    if( pcPPS->getColumnRowInfoPresent() == 1 )
    {
      WRITE_UVLC( pcPPS->getNumColumnsMinus1(),                                    "num_tile_columns_minus1" );
      WRITE_UVLC( pcPPS->getNumRowsMinus1(),                                       "num_tile_rows_minus1" );
      WRITE_FLAG( pcPPS->getUniformSpacingIdr(),                                   "uniform_spacing_flag" );
      if( pcPPS->getUniformSpacingIdr() == 0 )
      {
        for(UInt i=0; i<pcPPS->getNumColumnsMinus1(); i++)
        {
          WRITE_UVLC( pcPPS->getColumnWidth(i),                                    "column_width" );
        }
        for(UInt i=0; i<pcPPS->getNumRowsMinus1(); i++)
        {
          WRITE_UVLC( pcPPS->getRowHeight(i),                                      "row_height" );
        }
      }
    }

    if(pcPPS->getTileBehaviorControlPresentFlag() == 1)
    {
      Int iNumColTilesMinus1 = (pcPPS->getColumnRowInfoPresent() == 1)?(pcPPS->getNumColumnsMinus1()):(pcPPS->getSPS()->getNumColumnsMinus1());
      Int iNumRowTilesMinus1 = (pcPPS->getColumnRowInfoPresent() == 1)?(pcPPS->getNumColumnsMinus1()):(pcPPS->getSPS()->getNumRowsMinus1());

      if(iNumColTilesMinus1 !=0 || iNumRowTilesMinus1 !=0)
      {
#if !REMOVE_TILE_DEPENDENCE
        WRITE_FLAG( pcPPS->getTileBoundaryIndependenceIdr(),                         "tile_boundary_independence_flag" );
        if(pcPPS->getTileBoundaryIndependenceIdr() == 1)
        {
#endif
          WRITE_FLAG( pcPPS->getLFCrossTileBoundaryFlag()?1 : 0,            "loop_filter_across_tile_flag");
#if !REMOVE_TILE_DEPENDENCE
        }
#endif
      }
    }
#if TILES_OR_ENTROPY_SYNC_IDC
  }
  else if(pcPPS->getSPS()->getTilesOrEntropyCodingSyncIdc()==2)
  {
    WRITE_UVLC( pcPPS->getNumSubstreams()-1,               "num_substreams_minus1" );
  }
#endif

#if DBL_CONTROL
  WRITE_FLAG( pcPPS->getDeblockingFilterControlPresent()?1 : 0, "deblocking_filter_control_present_flag");
#endif
#if PARALLEL_MERGE
  WRITE_UVLC( pcPPS->getLog2ParallelMergeLevelMinus2(), "log2_parallel_merge_level_minus2");
#endif
  WRITE_FLAG( 0, "pps_extension_flag" );
}

Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  WRITE_CODE( pcSPS->getProfileIdc (),     8,       "profile_idc" );
  WRITE_CODE( 0,                           8,       "reserved_zero_8bits" );
  WRITE_CODE( pcSPS->getLevelIdc (),       8,       "level_idc" );
  WRITE_UVLC( pcSPS->getSPSId (),                   "seq_parameter_set_id" );
  WRITE_UVLC( pcSPS->getChromaFormatIdc (),         "chroma_format_idc" );
  WRITE_CODE( pcSPS->getMaxTLayers() - 1,  3,       "max_temporal_layers_minus1" );
  WRITE_UVLC( pcSPS->getPicWidthInLumaSamples (),   "pic_width_in_luma_samples" );
  WRITE_UVLC( pcSPS->getPicHeightInLumaSamples(),   "pic_height_in_luma_samples" );
#if PIC_CROPPING
  WRITE_FLAG( pcSPS->getPicCroppingFlag(),          "pic_cropping_flag" );
  if (pcSPS->getPicCroppingFlag())
  {
    WRITE_UVLC( pcSPS->getPicCropLeftOffset(),      "pic_crop_left_offset" );
    WRITE_UVLC( pcSPS->getPicCropRightOffset(),     "pic_crop_right_offset" );
    WRITE_UVLC( pcSPS->getPicCropTopOffset(),       "pic_crop_top_offset" );
    WRITE_UVLC( pcSPS->getPicCropBottomOffset(),    "pic_crop_bottom_offset" );
  }
#endif

#if FULL_NBIT
  WRITE_UVLC( pcSPS->getBitDepth() - 8,             "bit_depth_luma_minus8" );
#else
  WRITE_UVLC( pcSPS->getBitIncrement(),             "bit_depth_luma_minus8" );
#endif
#if FULL_NBIT
  WRITE_UVLC( pcSPS->getBitDepth() - 8,             "bit_depth_chroma_minus8" );
#else
  WRITE_UVLC( pcSPS->getBitIncrement(),             "bit_depth_chroma_minus8" );
#endif

  WRITE_FLAG( pcSPS->getUsePCM() ? 1 : 0,                   "pcm_enabled_flag");

  if( pcSPS->getUsePCM() )
  {
  WRITE_CODE( pcSPS->getPCMBitDepthLuma() - 1, 4,   "pcm_bit_depth_luma_minus1" );
  WRITE_CODE( pcSPS->getPCMBitDepthChroma() - 1, 4, "pcm_bit_depth_chroma_minus1" );
  }

#if LOSSLESS_CODING
  WRITE_FLAG( (pcSPS->getUseLossless ()) ? 1 : 0,                                    "qpprime_y_zero_transquant_bypass_flag" );
#endif

  WRITE_UVLC( pcSPS->getBitsForPOC()-4,                 "log2_max_pic_order_cnt_lsb_minus4" );
#if H0567_DPB_PARAMETERS_PER_TEMPORAL_LAYER
  for(UInt i=0; i <= pcSPS->getMaxTLayers()-1; i++)
  {
    WRITE_UVLC( pcSPS->getMaxDecPicBuffering(i),           "max_dec_pic_buffering[i]" );
    WRITE_UVLC( pcSPS->getNumReorderPics(i),               "num_reorder_pics[i]" );
    WRITE_UVLC( pcSPS->getMaxLatencyIncrease(i),           "max_latency_increase[i]" );
  }
#else
  WRITE_UVLC( pcSPS->getMaxNumberOfReferencePictures(), "max_num_ref_pics" ); 
  WRITE_UVLC( pcSPS->getNumReorderFrames(),             "num_reorder_frames" ); 
  WRITE_UVLC(pcSPS->getMaxDecFrameBuffering(),          "max_dec_frame_buffering" );
  WRITE_UVLC(pcSPS->getMaxLatencyIncrease(),            "max_latency_increase"    );
#endif
  assert( pcSPS->getMaxCUWidth() == pcSPS->getMaxCUHeight() );
  
  UInt MinCUSize = pcSPS->getMaxCUWidth() >> ( pcSPS->getMaxCUDepth()-g_uiAddCUDepth );
  UInt log2MinCUSize = 0;
  while(MinCUSize > 1)
  {
    MinCUSize >>= 1;
    log2MinCUSize++;
  }

#if H0412_REF_PIC_LIST_RESTRICTION
  WRITE_FLAG( pcSPS->getRestrictedRefPicListsFlag(),                                 "restricted_ref_pic_lists_flag" );
  if( pcSPS->getRestrictedRefPicListsFlag() )
  {
    WRITE_FLAG( pcSPS->getListsModificationPresentFlag(),                            "lists_modification_present_flag" );
  }
#endif
  WRITE_UVLC( log2MinCUSize - 3,                                                     "log2_min_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getMaxCUDepth()-g_uiAddCUDepth,                                 "log2_diff_max_min_coding_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MinSize() - 2,                                 "log2_min_transform_block_size_minus2" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize(), "log2_diff_max_min_transform_block_size" );

  if(log2MinCUSize == 3)
  {
    xWriteFlag  ( (pcSPS->getDisInter4x4()) ? 1 : 0 );
  }

  if( pcSPS->getUsePCM() )
  {
    WRITE_UVLC( pcSPS->getPCMLog2MinSize() - 3,                                      "log2_min_pcm_coding_block_size_minus3" );
    WRITE_UVLC( pcSPS->getPCMLog2MaxSize() - pcSPS->getPCMLog2MinSize(),             "log2_diff_max_min_pcm_coding_block_size" );
  }
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthInter() - 1,                               "max_transform_hierarchy_depth_inter" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthIntra() - 1,                               "max_transform_hierarchy_depth_intra" );
  WRITE_FLAG( pcSPS->getScalingListFlag() ? 1 : 0,                                   "scaling_list_enabled_flag" ); 
  WRITE_FLAG( pcSPS->getUseLMChroma () ? 1 : 0,                                      "chroma_pred_from_luma_enabled_flag" ); 
  WRITE_FLAG( pcSPS->getUseDF() ? 1 : 0,                                             "deblocking_filter_in_aps_enabled_flag");
  WRITE_FLAG( pcSPS->getLFCrossSliceBoundaryFlag()?1 : 0,                            "seq_loop_filter_across_slices_enabled_flag");
  WRITE_FLAG( pcSPS->getUseAMP(),                                                    "asymmetric_motion_partitions_enabled_flag" );
  WRITE_FLAG( pcSPS->getUseNSQT(),                                                   "non_square_quadtree_enabled_flag" );
  WRITE_FLAG( pcSPS->getUseSAO() ? 1 : 0,                                            "sample_adaptive_offset_enabled_flag");
  WRITE_FLAG( pcSPS->getUseALF () ? 1 : 0,                                           "adaptive_loop_filter_enabled_flag");
#if LCU_SYNTAX_ALF
  if(pcSPS->getUseALF())
  {
    WRITE_FLAG( (pcSPS->getUseALFCoefInSlice()) ? 1 : 0,                             "alf_coef_in_slice_flag");
  }
#endif

  if( pcSPS->getUsePCM() )
  {
  WRITE_FLAG( pcSPS->getPCMFilterDisableFlag()?1 : 0,                                "pcm_loop_filter_disable_flag");
  }

  assert( pcSPS->getMaxTLayers() > 0 );         

  WRITE_FLAG( pcSPS->getTemporalIdNestingFlag() ? 1 : 0,                             "temporal_id_nesting_flag" );

#if RPS_IN_SPS
  TComRPSList* rpsList = pcSPS->getRPSList();
  TComReferencePictureSet*      rps;

  WRITE_UVLC(rpsList->getNumberOfReferencePictureSets(), "num_short_term_ref_pic_sets" );
  for(Int i=0; i < rpsList->getNumberOfReferencePictureSets(); i++)
  {
    rps = rpsList->getReferencePictureSet(i);
    codeShortTermRefPicSet(pcSPS,rps);
  }    
  WRITE_FLAG( pcSPS->getLongTermRefsPresent() ? 1 : 0,         "long_term_ref_pics_present_flag" );
#endif
#if !PIC_CROPPING
  //!!!KS: Syntax not in WD !!!
  
  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );
#endif
  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }

#if TILES_WPP_ENTRY_POINT_SIGNALLING
  Int tilesOrEntropyCodingSyncIdc = 0;
  if ( pcSPS->getNumColumnsMinus1() > 0 || pcSPS->getNumRowsMinus1() > 0)
  {
    tilesOrEntropyCodingSyncIdc = 1;
  }
  else if ( pcSPS->getNumSubstreams() > 1 )
  {
    tilesOrEntropyCodingSyncIdc = 2;
  }
  pcSPS->setTilesOrEntropyCodingSyncIdc( tilesOrEntropyCodingSyncIdc );
  WRITE_CODE(tilesOrEntropyCodingSyncIdc, 2, "tiles_or_entropy_coding_sync_idc");
#endif

#if TILES_OR_ENTROPY_SYNC_IDC
  if(tilesOrEntropyCodingSyncIdc == 1)
  {
#endif
    WRITE_UVLC( pcSPS->getNumColumnsMinus1(),                           "num_tile_columns_minus1" );
    WRITE_UVLC( pcSPS->getNumRowsMinus1(),                              "num_tile_rows_minus1" );
    WRITE_FLAG( pcSPS->getUniformSpacingIdr(),                          "uniform_spacing_flag" );

    if( pcSPS->getUniformSpacingIdr()==0 )
    {
      for(UInt i=0; i<pcSPS->getNumColumnsMinus1(); i++)
      {
        WRITE_UVLC( pcSPS->getColumnWidth(i),                           "column_width" );
      }
      for(UInt i=0; i<pcSPS->getNumRowsMinus1(); i++)
      {
        WRITE_UVLC( pcSPS->getRowHeight(i),                             "row_height" );
      }
    }

    if( pcSPS->getNumColumnsMinus1() !=0 || pcSPS->getNumRowsMinus1() != 0)
    {
#if !REMOVE_TILE_DEPENDENCE
      WRITE_FLAG( pcSPS->getTileBoundaryIndependenceIdr(),                "tile_boundary_independence_flag" );
      if(pcSPS->getTileBoundaryIndependenceIdr() == 1)
      {
#endif
        WRITE_FLAG( pcSPS->getLFCrossTileBoundaryFlag()?1 : 0,            "loop_filter_across_tile_flag");
#if !REMOVE_TILE_DEPENDENCE
      }
#endif
    }
#if TILES_OR_ENTROPY_SYNC_IDC
  }
#endif
  WRITE_FLAG( 0, "sps_extension_flag" );
}

Void TEncCavlc::writeTileMarker( UInt uiTileIdx, UInt uiBitsUsed )
{
  xWriteCode( uiTileIdx, uiBitsUsed );
}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
#if ENC_DEC_TRACE  
  xTraceSliceHeader (pcSlice);
#endif

  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //    slice_address
  //calculate number of bits required for slice address
  Int maxAddrOuter = pcSlice->getPic()->getNumCUsInFrame();
  Int reqBitsOuter = 0;
  while(maxAddrOuter>(1<<reqBitsOuter)) 
  {
    reqBitsOuter++;
  }
  Int maxAddrInner = pcSlice->getPic()->getNumPartInCU()>>(2);
  maxAddrInner = (1<<(pcSlice->getPPS()->getSliceGranularity()<<1));
  Int reqBitsInner = 0;
  
  while(maxAddrInner>(1<<reqBitsInner))
  {
    reqBitsInner++;
  }
  Int lCUAddress;
  Int innerAddress;
  if (pcSlice->isNextSlice())
  {
    // Calculate slice address
    lCUAddress = (pcSlice->getSliceCurStartCUAddr()/pcSlice->getPic()->getNumPartInCU());
    innerAddress = (pcSlice->getSliceCurStartCUAddr()%(pcSlice->getPic()->getNumPartInCU()))>>((pcSlice->getSPS()->getMaxCUDepth()-pcSlice->getPPS()->getSliceGranularity())<<1);
  }
  else
  {
    // Calculate slice address
    lCUAddress = (pcSlice->getEntropySliceCurStartCUAddr()/pcSlice->getPic()->getNumPartInCU());
    innerAddress = (pcSlice->getEntropySliceCurStartCUAddr()%(pcSlice->getPic()->getNumPartInCU()))>>((pcSlice->getSPS()->getMaxCUDepth()-pcSlice->getPPS()->getSliceGranularity())<<1);
    
  }
  //write slice address
  Int address = (pcSlice->getPic()->getPicSym()->getCUOrderMap(lCUAddress) << reqBitsInner) + innerAddress;
  WRITE_FLAG( address==0, "first_slice_in_pic_flag" );
  if(address>0) 
  {
    WRITE_CODE( address, reqBitsOuter+reqBitsInner, "slice_address" );
  }

  WRITE_UVLC( pcSlice->getSliceType(),       "slice_type" );
  Bool bEntropySlice = (!pcSlice->isNextSlice());
  WRITE_FLAG( bEntropySlice ? 1 : 0, "lightweight_slice_flag" );
  
  if (!bEntropySlice)
  {
    WRITE_UVLC( pcSlice->getPPS()->getPPSId(), "pic_parameter_set_id" );
#if H0388
    if( pcSlice->getPPS()->getOutputFlagPresentFlag() )
    {
      WRITE_FLAG( pcSlice->getPicOutputFlag() ? 1 : 0, "pic_output_flag" );
    }
#endif
    if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    {
      WRITE_UVLC( 0, "idr_pic_id" );
      WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    else
    {
      WRITE_CODE( (pcSlice->getPOC()-pcSlice->getLastIDR()+(1<<pcSlice->getSPS()->getBitsForPOC()))%(1<<pcSlice->getSPS()->getBitsForPOC()), pcSlice->getSPS()->getBitsForPOC(), "pic_order_cnt_lsb");
      TComReferencePictureSet* rps = pcSlice->getRPS();
      if(pcSlice->getRPSidx() < 0)
      {
        WRITE_FLAG( 0, "short_term_ref_pic_set_sps_flag");
#if RPS_IN_SPS
        codeShortTermRefPicSet(pcSlice->getSPS(), rps);
#else
        codeShortTermRefPicSet(pcSlice->getPPS(), rps);
#endif
      }
      else
      {
        WRITE_FLAG( 1, "short_term_ref_pic_set_sps_flag");
        WRITE_UVLC( pcSlice->getRPSidx(), "short_term_ref_pic_set_idx" );
      }
#if RPS_IN_SPS
      if(pcSlice->getSPS()->getLongTermRefsPresent())
#else
      if(pcSlice->getPPS()->getLongTermRefsPresent())
#endif
      {
        WRITE_UVLC( rps->getNumberOfLongtermPictures(), "num_long_term_pics");
        Int maxPocLsb = 1<<pcSlice->getSPS()->getBitsForPOC();
        Int prev = 0;
#if LTRP_MULT
        Int prevDeltaPocLt=0;
        Int currDeltaPocLt=0;
#endif
        for(Int i=rps->getNumberOfPictures()-1 ; i > rps->getNumberOfPictures()-rps->getNumberOfLongtermPictures()-1; i--)
        {
          WRITE_UVLC((maxPocLsb-rps->getDeltaPOC(i)+prev)%maxPocLsb, "delta_poc_lsb_lt");
          
#if LTRP_MULT
          currDeltaPocLt=((maxPocLsb-rps->getDeltaPOC(i)+prev)%maxPocLsb)+prevDeltaPocLt;

          Int deltaMsbCycle=0;
          if( (i==(rps->getNumberOfPictures()-1)) )
          {
            deltaMsbCycle=((-rps->getDeltaPOC(i))/maxPocLsb)-1;
          }
          else if( prevDeltaPocLt!=currDeltaPocLt )
          {
            deltaMsbCycle=((-rps->getDeltaPOC(i))/maxPocLsb)-1;
            if( ((prevDeltaPocLt==maxPocLsb-1) && (currDeltaPocLt==maxPocLsb+1)) ||  ((prevDeltaPocLt==maxPocLsb-2) && (currDeltaPocLt==maxPocLsb)))
            {
              deltaMsbCycle=deltaMsbCycle-1;
            }
          }
          else
          {
            deltaMsbCycle=((rps->getDeltaPOC(i+1)-rps->getDeltaPOC(i))/maxPocLsb)-1;
          }

          if(deltaMsbCycle>=0)
          {
            WRITE_FLAG( 1, "delta_poc_msb_present_flag");
            WRITE_UVLC(deltaMsbCycle, "delta_poc_msb_cycle_lt_minus1");
          }
          else
          {
            WRITE_FLAG( 0, "delta_poc_msb_present_flag");
          }
          prevDeltaPocLt=currDeltaPocLt;
#endif
          prev = rps->getDeltaPOC(i);
          WRITE_FLAG( rps->getUsed(i), "used_by_curr_pic_lt_flag"); 
        }
      }
    }
    if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF()|| pcSlice->getSPS()->getScalingListFlag() || pcSlice->getSPS()->getUseDF())
    {
      if (pcSlice->getSPS()->getUseALF())
      {
#if !LCU_SYNTAX_ALF
         if (pcSlice->getAlfEnabledFlag())
         {
           assert (pcSlice->getAPS()->getAlfEnabled());
         }
#endif
         WRITE_FLAG( pcSlice->getAlfEnabledFlag(), "ALF on/off flag in slice header" );
      }
      if (pcSlice->getSPS()->getUseSAO())
      {
#if SAO_UNIT_INTERLEAVING
        WRITE_FLAG( pcSlice->getSaoInterleavingFlag(), "SAO interleaving flag" );
#endif
         assert (pcSlice->getSaoEnabledFlag() == pcSlice->getAPS()->getSaoEnabled());
         WRITE_FLAG( pcSlice->getSaoEnabledFlag(), "SAO on/off flag in slice header" );
#if SAO_UNIT_INTERLEAVING
         if (pcSlice->getSaoInterleavingFlag()&&pcSlice->getSaoEnabledFlag() )
         {
           WRITE_FLAG( pcSlice->getAPS()->getSaoParam()->bSaoFlag[1], "SAO on/off flag for Cb in slice header" );
           WRITE_FLAG( pcSlice->getAPS()->getSaoParam()->bSaoFlag[2], "SAO on/off flag for Cr in slice header" );
         }
#endif
      }
      WRITE_UVLC( pcSlice->getAPS()->getAPSID(), "aps_id");
    }

    //check if numrefidxes match the defaults. If not, override
    if (!pcSlice->isIntra())
    {
      Bool overrideFlag = (pcSlice->getNumRefIdx( REF_PIC_LIST_0 )!=pcSlice->getPPS()->getNumRefIdxL0DefaultActive()||(pcSlice->isInterB()&&pcSlice->getNumRefIdx( REF_PIC_LIST_1 )!=pcSlice->getPPS()->getNumRefIdxL1DefaultActive()));
      WRITE_FLAG( overrideFlag ? 1 : 0,                               "num_ref_idx_active_override_flag");
      if (overrideFlag) 
      {
        WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) - 1, 3,   "num_ref_idx_l0_active_minus1" );
        if (pcSlice->isInterB())
        {
          WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) - 1, 3, "num_ref_idx_l1_active_minus1" );
        }
        else
        {
          pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
        }
      }
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
      pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
    }
#if H0412_REF_PIC_LIST_RESTRICTION
    if( pcSlice->getSPS()->getListsModificationPresentFlag() )
    {
#endif
      TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
#if H0137_0138_LIST_MODIFICATION
      if(!pcSlice->isIntra())
      {
        WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0() ? 1 : 0,       "ref_pic_list_modification_flag_l0" );
        if (pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0())
        {
          Int numRpsCurrTempList0 = pcSlice->getNumRpsCurrTempList();
          if (numRpsCurrTempList0 > 1)
          {
            Int length = 1;
            numRpsCurrTempList0 --;
            while ( numRpsCurrTempList0 >>= 1) 
            {
              length ++;
            }
            for(Int i = 0; i < pcSlice->getNumRefIdx( REF_PIC_LIST_0 ); i++)
            {
              WRITE_CODE( refPicListModification->getRefPicSetIdxL0(i), length, "list_entry_l0");
            }
          }
        }
      }
      if(pcSlice->isInterB())
      {    
        WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1() ? 1 : 0,       "ref_pic_list_modification_flag_l1" );
        if (pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1())
        {
          Int numRpsCurrTempList1 = pcSlice->getNumRpsCurrTempList();
          if ( numRpsCurrTempList1 > 1 )
          {
            Int length = 1;
            numRpsCurrTempList1 --;
            while ( numRpsCurrTempList1 >>= 1)
            {
              length ++;
            }
            for(Int i = 0; i < pcSlice->getNumRefIdx( REF_PIC_LIST_1 ); i++)
            {
              WRITE_CODE( refPicListModification->getRefPicSetIdxL1(i), length, "list_entry_l1");
            }
          }
        }
      }
#else
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
#if H0412_REF_PIC_LIST_RESTRICTION
  }
#endif
  // ref_pic_list_combination( )
  // maybe move to own function?
  if (pcSlice->isInterB())
  {
    WRITE_FLAG(pcSlice->getRefPicListCombinationFlag() ? 1 : 0,       "ref_pic_list_combination_flag" );
    if(pcSlice->getRefPicListCombinationFlag())
    {
      WRITE_UVLC( pcSlice->getNumRefIdx(REF_PIC_LIST_C) - 1,          "num_ref_idx lc_active_minus1");
      
#if H0412_REF_PIC_LIST_RESTRICTION
      if( pcSlice->getSPS()->getListsModificationPresentFlag() )
      {
#endif
        WRITE_FLAG( pcSlice->getRefPicListModificationFlagLC() ? 1 : 0, "ref_pic_list_modification_flag_lc" );
        if(pcSlice->getRefPicListModificationFlagLC())
        {
          for (UInt i=0;i<pcSlice->getNumRefIdx(REF_PIC_LIST_C);i++)
          {
            WRITE_FLAG( pcSlice->getListIdFromIdxOfLC(i),               "pic_from_list_0_flag" );
#if H0137_0138_LIST_MODIFICATION
          if (((pcSlice->getListIdFromIdxOfLC(i)==REF_PIC_LIST_0) && pcSlice->getNumRefIdx( REF_PIC_LIST_0 )>1 ) || ((pcSlice->getListIdFromIdxOfLC(i)==REF_PIC_LIST_1) && pcSlice->getNumRefIdx( REF_PIC_LIST_1 )>1 ) )
          {
            WRITE_UVLC( pcSlice->getRefIdxFromIdxOfLC(i),               "ref_idx_list_curr" );
          }
#else
            WRITE_UVLC( pcSlice->getRefIdxFromIdxOfLC(i),               "ref_idx_list_curr" );
#endif
          }
        }
#if H0412_REF_PIC_LIST_RESTRICTION
      }
#endif
    }
  }
    
#if H0111_MVD_L1_ZERO
  if (pcSlice->isInterB())
  {
    WRITE_FLAG( pcSlice->getMvdL1ZeroFlag() ? 1 : 0,   "mvd_l1_zero_flag");
  }
#endif

  if(pcSlice->getPPS()->getEntropyCodingMode() && !pcSlice->isIntra())
  {
#if CABAC_INIT_FLAG
    if (!pcSlice->isIntra() && pcSlice->getPPS()->getCabacInitPresentFlag())
    {
      SliceType sliceType   = pcSlice->getSliceType();
      Int  encCABACTableIdx = pcSlice->getPPS()->getEncCABACTableIdx();
      Bool encCabacInitFlag = (sliceType!=encCABACTableIdx && encCABACTableIdx!=0) ? true : false;
      pcSlice->setCabacInitFlag( encCabacInitFlag );
      WRITE_FLAG( encCabacInitFlag?1:0, "cabac_init_flag" );
    }
#else
    WRITE_UVLC(pcSlice->getCABACinitIDC(),  "cabac_init_idc");
#endif
  }

  // if( !lightweight_slice_flag ) {
  if (!bEntropySlice)
  {
    Int iCode = pcSlice->getSliceQp() - ( pcSlice->getPPS()->getPicInitQPMinus26() + 26 );
    WRITE_SVLC( iCode, "slice_qp_delta" ); 
#if DBL_CONTROL
    if (pcSlice->getPPS()->getDeblockingFilterControlPresent())
    {
      if ( pcSlice->getSPS()->getUseDF() )
      {
        WRITE_FLAG(pcSlice->getInheritDblParamFromAPS(), "inherit_dbl_param_from_APS_flag");
      }
#else
    WRITE_FLAG(pcSlice->getInheritDblParamFromAPS(), "inherit_dbl_param_from_APS_flag");
#endif
      if (!pcSlice->getInheritDblParamFromAPS())
      {
        WRITE_FLAG(pcSlice->getLoopFilterDisable(), "loop_filter_disable");  // should be an IDC
        if(!pcSlice->getLoopFilterDisable())
        {
          WRITE_SVLC (pcSlice->getLoopFilterBetaOffset(), "beta_offset_div2");
          WRITE_SVLC (pcSlice->getLoopFilterTcOffset(), "tc_offset_div2");
        }
      }
#if DBL_CONTROL
    }
#endif
    if ( pcSlice->getSliceType() == B_SLICE )
    {
      WRITE_FLAG( pcSlice->getColDir(), "collocated_from_l0_flag" );
    }

#if COLLOCATED_REF_IDX
    if ( pcSlice->getSliceType() != I_SLICE &&
      ((pcSlice->getColDir()==0 && pcSlice->getNumRefIdx(REF_PIC_LIST_0)>1)||
      (pcSlice->getColDir()==1  && pcSlice->getNumRefIdx(REF_PIC_LIST_1)>1)))
    {
      WRITE_UVLC( pcSlice->getColRefIdx(), "collocated_ref_idx" );
    }
#endif
  
    if ( (pcSlice->getPPS()->getUseWP() && pcSlice->getSliceType()==P_SLICE) || (pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getSliceType()==B_SLICE) )
    {
      xCodePredWeightTable( pcSlice );
    }
  }

  // !!!! sytnax elements not in the WD !!!!
  
  assert(pcSlice->getMaxNumMergeCand()<=MRG_MAX_NUM_CANDS_SIGNALED);
  assert(MRG_MAX_NUM_CANDS_SIGNALED<=MRG_MAX_NUM_CANDS);
  WRITE_UVLC(MRG_MAX_NUM_CANDS - pcSlice->getMaxNumMergeCand(), "maxNumMergeCand");
}


Void TEncCavlc::codeTileMarkerFlag(TComSlice* pcSlice) 
{
  Bool bEntropySlice = (!pcSlice->isNextSlice());
  if (!bEntropySlice)
  {
    xWriteFlag  (pcSlice->getTileMarkerFlag() ? 1 : 0 );
  }
}

/**
 - write wavefront substreams sizes for the slice header.
 .
 \param pcSlice Where we find the substream size information.
 */
#if TILES_WPP_ENTRY_POINT_SIGNALLING
Void  TEncCavlc::codeTilesWPPEntryPoint( TComSlice* pSlice )
{
  Int tilesOrEntropyCodingSyncIdc = pSlice->getSPS()->getTilesOrEntropyCodingSyncIdc();

  if ( tilesOrEntropyCodingSyncIdc == 0 )
  {
    return;
  }

  UInt numEntryPointOffsets = 0, offsetLenMinus1 = 0, maxOffset = 0;
  UInt *entryPointOffset = NULL;
  if (tilesOrEntropyCodingSyncIdc == 1) // tiles
  {
    numEntryPointOffsets = pSlice->getTileLocationCount();
    entryPointOffset     = new UInt[numEntryPointOffsets];
    for (Int idx=0; idx<pSlice->getTileLocationCount(); idx++)
    {
      if ( idx == 0 )
      {
        entryPointOffset [ idx ] = pSlice->getTileLocation( 0 );
      }
      else
      {
        entryPointOffset [ idx ] = pSlice->getTileLocation( idx ) - pSlice->getTileLocation( idx-1 );
      }

      if ( entryPointOffset[ idx ] > maxOffset )
      {
        maxOffset = entryPointOffset[ idx ];
      }
    }
  }
  else if (tilesOrEntropyCodingSyncIdc == 2) // wavefront
  {
    Int  numZeroSubstreamsAtEndOfSlice  = 0;
    UInt* pSubstreamSizes               = pSlice->getSubstreamSizes();
    // Find number of zero substreams at the end of slice
    for (Int idx=pSlice->getPPS()->getNumSubstreams()-2; idx>=0; idx--)
    {
      if ( pSubstreamSizes[ idx ] ==  0 )
      {
        numZeroSubstreamsAtEndOfSlice++; 
      }
      else
      {
        break;
      }
    }
    numEntryPointOffsets       = pSlice->getPPS()->getNumSubstreams() - 1 - numZeroSubstreamsAtEndOfSlice;
    entryPointOffset           = new UInt[numEntryPointOffsets];
    for (Int idx=0; idx<numEntryPointOffsets; idx++)
    {
      entryPointOffset[ idx ] = ( pSubstreamSizes[ idx ] >> 3 ) ;
      if ( entryPointOffset[ idx ] > maxOffset )
      {
        maxOffset = entryPointOffset[ idx ];
      }
    }
  }

  maxOffset += ((m_pcBitIf->getNumberOfWrittenBits() + 16) >> 3) + 8 + 2; // allowing for NALU header, slice header, bytes added for "offset_len_minus1" and "num_entry_point_offsets"

  // Determine number of bits "offsetLenMinus1+1" required for entry point information
  offsetLenMinus1 = 0;
  while (1)
  {
    if (maxOffset >= (1 << offsetLenMinus1) )
    {
      offsetLenMinus1++;
      if ( offsetLenMinus1 > 32 )
      {
        FATAL_ERROR_0("exceeded 32-bits", -1);
      }
    }
    else
    {
      break;
    }
  }

  WRITE_UVLC(numEntryPointOffsets, "num_entry_point_offsets");
  if (numEntryPointOffsets>0)
  {
    WRITE_UVLC(offsetLenMinus1, "offset_len_minus1");
  }

  for (UInt idx=0; idx<numEntryPointOffsets; idx++)
  {
    if ( idx == 0 )
    {
      // Adding sizes of NALU header and slice header information to entryPointOffset[ 0 ]
      Int bitDistFromNALUHdrStart    = m_pcBitIf->getNumberOfWrittenBits() + 16;
      entryPointOffset[ idx ] += ( bitDistFromNALUHdrStart + numEntryPointOffsets*(offsetLenMinus1+1) ) >> 3;
    }
    WRITE_CODE(entryPointOffset[ idx ], offsetLenMinus1+1, "entry_point_offset");
  }

  delete [] entryPointOffset;
}
#else
Void TEncCavlc::codeSliceHeaderSubstreamTable( TComSlice* pcSlice )
{
  UInt uiNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
  UInt*puiSubstreamSizes = pcSlice->getSubstreamSizes();

  // Write header information for all substreams except the last.
  for (UInt ui = 0; ui+1 < uiNumSubstreams; ui++)
  {
    UInt uiNumbits = puiSubstreamSizes[ui];

    //the 2 first bits are used to give the size of the header
    if ( uiNumbits < (1<<8) )
    {
      xWriteCode(0,         2  );
      xWriteCode(uiNumbits, 8  );
    }
    else if ( uiNumbits < (1<<16) )
    {
      xWriteCode(1,         2  );
      xWriteCode(uiNumbits, 16 );
    }
    else if ( uiNumbits < (1<<24) )
    {
      xWriteCode(2,         2  );
      xWriteCode(uiNumbits, 24 );
    }
    else if ( uiNumbits < (1<<31) )
    {
      xWriteCode(3,         2  );
      xWriteCode(uiNumbits, 32 );
    }
    else
    {
      printf("Error in codeSliceHeaderTable\n");
      exit(-1);
    }
  }
}
#endif

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
}

Void TEncCavlc::codeSliceFinish ()
{
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TEncCavlc::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{  
  if (!m_bAlfCtrl)
  {
    return;
  }
  
  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }
  
  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;
  
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeApsExtensionFlag ()
{
  WRITE_FLAG(0, "aps_extension_flag");
}

Void TEncCavlc::codeAlfCtrlDepth()
{  
  if (!m_bAlfCtrl)
  {
    return;
  }
  
  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  
  xWriteUvlc(uiDepth);
}

Void TEncCavlc::codeInterModeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiEncMode )
{
  assert(0);
}

Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TEncCavlc::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  assert(0);
}

Void TEncCavlc::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  assert(0);
}

Void TEncCavlc::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

#if BURST_IPCM
/** Code I_PCM information. 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \param numIPCM the number of succesive IPCM blocks with the same size 
 * \param firstIPCMFlag 
 * \returns Void
 */
Void TEncCavlc::codeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Int numIPCM, Bool firstIPCMFlag)
{
  assert(0);
}
#else
/** Code I_PCM information. 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, code its PCM alignment bits and codes.  
 */
Void TEncCavlc::codeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx)
{
  UInt uiIPCM = (pcCU->getIPCMFlag(uiAbsPartIdx) == true)? 1 : 0;

  xWriteFlag(uiIPCM);

  if (uiIPCM)
  {
    xWritePCMAlignZero();

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
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCb() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCr() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }
  }
}
#endif

Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getRefQP( uiAbsPartIdx );

#if H0736_AVC_STYLE_QP_RANGE
  Int qpBdOffsetY =  pcCU->getSlice()->getSPS()->getQpBDOffsetY();
  iDQp = (iDQp + 78 + qpBdOffsetY + (qpBdOffsetY/2)) % (52 + qpBdOffsetY) - 26 - (qpBdOffsetY/2);
#endif

  xWriteSvlc( iDQp );
  
  return;
}

Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  assert(0);
}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{  
  xWriteFlag( uiCode );
}

Void TEncCavlc::codeAlfCtrlFlag( UInt uiSymbol )
{
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfUvlc( UInt uiCode )
{
  xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
  xWriteSvlc( iCode );
}
#if LCU_SYNTAX_ALF
/** Code the fixed length code (smaller than one max value) in OSALF
 * \param idx:  coded value 
 * \param maxValue: max value
 */
Void TEncCavlc::codeAlfFixedLengthIdx( UInt idx, UInt maxValue)
{
  UInt length = 0;
  assert(idx<=maxValue);

  UInt temp = maxValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      length = i+1;
    }
    temp = (temp >> 1);
  }

  if(length)
  {
    xWriteCode( idx, length );
  }
}
#endif

Void TEncCavlc::codeSaoFlag( UInt uiCode )
{
  xWriteFlag( uiCode );
}

Void TEncCavlc::codeSaoUvlc( UInt uiCode )
{
    xWriteUvlc( uiCode );
}

Void TEncCavlc::codeSaoSvlc( Int iCode )
{
    xWriteSvlc( iCode );
}
#if SAO_UNIT_INTERLEAVING
/** Code SAO run. 
 * \param uiCode
 * \param maxValue
 */
Void TEncCavlc::codeSaoRun( UInt uiCode, UInt maxValue)
{
  UInt uiLength = 0;
  if (!maxValue)
  {
    return;
  }
  assert(uiCode<=maxValue);              

  for(UInt i=0; i<32; i++)                                     
  {                                                            
    if(maxValue&0x1)                                               
    {                                                          
      uiLength = i+1;                                          
    }                                                          
    maxValue = (maxValue >> 1);                                        
  }
  WRITE_CODE( uiCode, uiLength, "sao_run_diff");
}
#endif

Void TEncCavlc::estBit( estBitsSbacStruct* pcEstBitsCabac, Int width, Int height, TextType eTType )
{
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
  
  //m_pcBitIf->write( uiCode, uiLength );
  // Take care of cases where uiLength > 32
  m_pcBitIf->write( 0, uiLength >> 1);
  m_pcBitIf->write( uiCode, (uiLength+1) >> 1);
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

/** Write PCM alignment bits. 
 * \returns Void
 */
Void  TEncCavlc::xWritePCMAlignZero    ()
{
  m_pcBitIf->writeAlignZero();
}

Void TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }
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

/** code explicit wp tables
 * \param TComSlice* pcSlice
 * \returns Void
 */
Void TEncCavlc::xCodePredWeightTable( TComSlice* pcSlice )
{
  wpScalingParam  *wp;
  Bool            bChroma     = true; // color always present in HEVC ?
  Int             iNbRef       = (pcSlice->getSliceType() == B_SLICE ) ? (2) : (1);
  Bool            bDenomCoded  = false;

  UInt            uiMode = 0;
  if ( (pcSlice->getSliceType()==P_SLICE && pcSlice->getPPS()->getUseWP()) || (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getRefPicListCombinationFlag()==0 ) )
    uiMode = 1; // explicit
  else if ( pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==2 )
    uiMode = 2; // implicit (does not use this mode in this syntax)
  if (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getRefPicListCombinationFlag())
    uiMode = 3; // combined explicit
  if(uiMode == 1)
  {
    for ( Int iNumRef=0 ; iNumRef<iNbRef ; iNumRef++ ) 
    {
      RefPicList  eRefPicList = ( iNumRef ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
      for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(eRefPicList) ; iRefIdx++ ) 
      {
        pcSlice->getWpScaling(eRefPicList, iRefIdx, wp);
        if ( !bDenomCoded ) 
        {
          Int iDeltaDenom;
          WRITE_UVLC( wp[0].uiLog2WeightDenom, "luma_log2_weight_denom" );     // ue(v): luma_log2_weight_denom

          if( bChroma )
          {
            iDeltaDenom = (wp[1].uiLog2WeightDenom - wp[0].uiLog2WeightDenom);
            WRITE_SVLC( iDeltaDenom, "delta_chroma_log2_weight_denom" );       // se(v): delta_chroma_log2_weight_denom
          }
          bDenomCoded = true;
        }

        WRITE_FLAG( wp[0].bPresentFlag, "luma_weight_lX_flag" );               // u(1): luma_weight_lX_flag

        if ( wp[0].bPresentFlag ) 
        {
          Int iDeltaWeight = (wp[0].iWeight - (1<<wp[0].uiLog2WeightDenom));
          WRITE_SVLC( iDeltaWeight, "delta_luma_weight_lX" );                  // se(v): delta_luma_weight_lX
          WRITE_SVLC( wp[0].iOffset, "luma_offset_lX" );                       // se(v): luma_offset_lX
        }

        if ( bChroma ) 
        {
          WRITE_FLAG( wp[1].bPresentFlag, "chroma_weight_lX_flag" );           // u(1): chroma_weight_lX_flag

          if ( wp[1].bPresentFlag )
          {
            for ( Int j=1 ; j<3 ; j++ ) 
            {
              Int iDeltaWeight = (wp[j].iWeight - (1<<wp[1].uiLog2WeightDenom));
              WRITE_SVLC( iDeltaWeight, "delta_chroma_weight_lX" );            // se(v): delta_chroma_weight_lX

              Int iDeltaChroma = (wp[j].iOffset + ( ( (g_uiIBDI_MAX>>1)*wp[j].iWeight)>>(wp[j].uiLog2WeightDenom) ) - (g_uiIBDI_MAX>>1));
              WRITE_SVLC( iDeltaChroma, "delta_chroma_offset_lX" );            // se(v): delta_chroma_offset_lX
            }
          }
        }
      }
    }
  }
  else if (uiMode == 3)
  {
    for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(REF_PIC_LIST_C) ; iRefIdx++ ) 
    {
      RefPicList  eRefPicList = (RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIdx);
      Int iCombRefIdx = pcSlice->getRefIdxFromIdxOfLC(iRefIdx);

      pcSlice->getWpScaling(eRefPicList, iCombRefIdx, wp);
      if ( !bDenomCoded ) 
      {
        Int iDeltaDenom;
        WRITE_UVLC( wp[0].uiLog2WeightDenom, "luma_log2_weight_denom" );       // ue(v): luma_log2_weight_denom

        if( bChroma )
        {
          iDeltaDenom = (wp[1].uiLog2WeightDenom - wp[0].uiLog2WeightDenom);
          WRITE_SVLC( iDeltaDenom, "delta_chroma_log2_weight_denom" );         // se(v): delta_chroma_log2_weight_denom
        }
        bDenomCoded = true;
      }

      WRITE_FLAG( wp[0].bPresentFlag, "luma_weight_lc_flag" );                 // u(1): luma_weight_lc_flag

      if ( wp[0].bPresentFlag ) 
      {
        Int iDeltaWeight = (wp[0].iWeight - (1<<wp[0].uiLog2WeightDenom));
        WRITE_SVLC( iDeltaWeight, "delta_luma_weight_lc" );                    // se(v): delta_luma_weight_lc
        WRITE_SVLC( wp[0].iOffset, "luma_offset_lc" );                         // se(v): luma_offset_lc
      }
      if ( bChroma ) 
      {
        WRITE_FLAG( wp[1].bPresentFlag, "chroma_weight_lc_flag" );             // u(1): luma_weight_lc_flag

        if ( wp[1].bPresentFlag )
        {
          for ( Int j=1 ; j<3 ; j++ ) 
          {
            Int iDeltaWeight = (wp[j].iWeight - (1<<wp[1].uiLog2WeightDenom));
            WRITE_SVLC( iDeltaWeight, "delta_chroma_weight_lc" );              // se(v): delta_chroma_weight_lc

            Int iDeltaChroma = (wp[j].iOffset + ( ( (g_uiIBDI_MAX>>1)*wp[j].iWeight)>>(wp[j].uiLog2WeightDenom) ) - (g_uiIBDI_MAX>>1));
            WRITE_SVLC( iDeltaChroma, "delta_chroma_offset_lc" );              // se(v): delta_chroma_offset_lc
          }
        }
      }
    }
  }
}

/** code quantization matrix
 *  \param scalingList quantization matrix information
 */
Void TEncCavlc::codeScalingList( TComScalingList* scalingList )
{
  UInt listId,sizeId;
  Bool scalingListPredModeFlag;

#if SCALING_LIST_OUTPUT_RESULT
  Int startBit;
  Int startTotalBit;
  startBit = m_pcBitIf->getNumberOfWrittenBits();
  startTotalBit = m_pcBitIf->getNumberOfWrittenBits();
#endif

  WRITE_FLAG( scalingList->getScalingListPresentFlag (), "scaling_list_present_flag" );

  if(scalingList->getScalingListPresentFlag () == false)
  {
#if SCALING_LIST_OUTPUT_RESULT
    printf("Header Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startBit);
#endif
    //for each size
    for(sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
    {
      for(listId = 0; listId < g_scalingListNum[sizeId]; listId++)
      {
#if SCALING_LIST_OUTPUT_RESULT
        startBit = m_pcBitIf->getNumberOfWrittenBits();
#endif
        scalingListPredModeFlag = scalingList->checkPredMode( sizeId, listId );
        WRITE_FLAG( scalingListPredModeFlag, "scaling_list_pred_mode_flag" );
        if(!scalingListPredModeFlag)// Copy Mode
        {
          WRITE_UVLC( (Int)listId - (Int)scalingList->getRefMatrixId (sizeId,listId) - 1, "scaling_list_pred_matrix_id_delta");
        }
        else// DPCM Mode
        {
          xCodeScalingList(scalingList, sizeId, listId);
        }
#if SCALING_LIST_OUTPUT_RESULT
        printf("Matrix [%d][%d] Bit %d\n",sizeId,listId,m_pcBitIf->getNumberOfWrittenBits() - startBit);
#endif
      }
    }
  }
#if SCALING_LIST_OUTPUT_RESULT
  else
  {
    printf("Header Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startTotalBit);
  }
  printf("Total Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startTotalBit);
#endif
  return;
}
/** code DPCM
 * \param scalingList quantization matrix information
 * \param sizeIdc size index
 * \param listIdc list index
 */
Void TEncCavlc::xCodeScalingList(TComScalingList* scalingList, UInt sizeId, UInt listId)
{
#if SCALING_LIST
  Int coefNum = min(MAX_MATRIX_COEF_NUM,(Int)g_scalingListSize[sizeId]);
  UInt* scan    = g_auiFrameScanXY [ (sizeId == 0)? 1 : 2];
#else
  Int coefNum = (Int)g_scalingListSize[sizeId];
  UInt* scan    = g_auiFrameScanXY [ sizeId + 1];
#endif
  Int nextCoef = SCALING_LIST_START_VALUE;
  Int data;
  Int *src = scalingList->getScalingListAddress(sizeId, listId);
#if SCALING_LIST
  if(sizeId > SCALING_LIST_8x8 && scalingList->getUseDefaultScalingMatrixFlag(sizeId,listId))
  {
    WRITE_SVLC( -8, "scaling_list_dc_coef_minus8");
  }
  else if(sizeId < SCALING_LIST_16x16 && scalingList->getUseDefaultScalingMatrixFlag(sizeId,listId))
  {
    WRITE_SVLC( -8, "scaling_list_delta_coef");
  }
  else
  {
    if( sizeId > SCALING_LIST_8x8 )
    {
      WRITE_SVLC( scalingList->getScalingListDC(sizeId,listId) - 8, "scaling_list_dc_coef_minus8");
    }
    for(Int i=0;i<coefNum;i++)
    {
      data = src[scan[i]] - nextCoef;
      nextCoef = src[scan[i]];
      if(data > 127)
      {
        data = data - 256;
      }
      if(data < -128)
      {
        data = data + 256;
      }

      WRITE_SVLC( data,  "scaling_list_delta_coef");
    }
  }
#else
  for(Int i=0;i<coefNum;i++)
  {
    data = src[scan[i]] - nextCoef;
    nextCoef = src[scan[i]];
    if(data > 127)
    {
      data = data - 256;
    }
    if(data < -128)
    {
      data = data + 256;
    }

    WRITE_SVLC( data,  "delta_coef");
  }
#endif
}
Bool TComScalingList::checkPredMode(UInt sizeId, UInt listId)
{
  for(Int predListIdx = (Int)listId -1 ; predListIdx >= 0; predListIdx--)
  {
#if SCALING_LIST
    if( !memcmp(getScalingListAddress(sizeId,listId),getScalingListAddress(sizeId, predListIdx),sizeof(Int)*min(MAX_MATRIX_COEF_NUM,(Int)g_scalingListSize[sizeId])) // check value of matrix
     && ((sizeId < SCALING_LIST_16x16) || (getScalingListDC(sizeId,listId) == getScalingListDC(sizeId,predListIdx)))) // check DC value
#else
    if( !memcmp(getScalingListAddress(sizeId,listId),getScalingListAddress(sizeId, predListIdx),sizeof(Int)*(Int)g_scalingListSize[sizeId])) // check value of matrix
#endif
    {
      setRefMatrixId(sizeId, listId, predListIdx);
      return false;
    }
  }
  return true;
}
//! \}
