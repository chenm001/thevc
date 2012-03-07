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

Void TEncCavlc::codeShortTermRefPicSet()
{
#if PRINT_RPS_INFO
  int lastBits = getNumberOfWrittenBits();
#endif
  WRITE_FLAG( 0, "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
    WRITE_UVLC( 1, "num_negative_pics" );
    WRITE_UVLC( 0, "num_positive_pics" );
      WRITE_UVLC( 0, "delta_poc_s0_minus1" );
  WRITE_FLAG( 1, "used_by_curr_pic_s0_flag"); 

#if PRINT_RPS_INFO
  printf("irps=%d (%2d bits) ", 0, getNumberOfWrittenBits() - lastBits);
#endif
}


Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
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

  WRITE_UVLC(1, "num_short_term_ref_pic_sets" );
    codeShortTermRefPicSet();
  WRITE_FLAG( 0,                                             "long_term_ref_pics_present_flag" );
#endif
  // entropy_coding_mode_flag
  // We code the entropy_coding_mode_flag, it's needed for tests.
  WRITE_FLAG( 1,                                             "entropy_coding_mode_flag" );
  {
#if !WPP_SIMPLIFICATION
    WRITE_UVLC( 0,                                           "entropy_coding_synchro" );
    WRITE_FLAG( 0,                                           "cabac_istate_reset" );
#endif
  }
#if !H0566_TLA
  WRITE_UVLC( 0,                                             "num_temporal_layer_switching_point_flags" );
#endif
  //   num_ref_idx_l0_default_active_minus1
  //   num_ref_idx_l1_default_active_minus1
  WRITE_SVLC( pcPPS->getPicInitQPMinus26(),                  "pic_init_qp_minus26");
  WRITE_FLAG( 0,                                             "constrained_intra_pred_flag" );
  WRITE_FLAG( pcPPS->getEnableTMVPFlag() ? 1 : 0,            "enable_temporal_mvp_flag" );
  WRITE_CODE( 0, 2,                                          "slice_granularity");
  WRITE_UVLC( 0,                                             "max_cu_qp_delta_depth" );

  WRITE_SVLC( 0,                                             "chroma_qp_offset"     );
  WRITE_SVLC( 0,                                             "chroma_qp_offset_2nd" );

  WRITE_FLAG( 0,    "weighted_pred_flag" );   // Use of Weighting Prediction (P_SLICE)
  WRITE_CODE( 0, 2, "weighted_bipred_idc" );  // Use of Weighting Bi-Prediction (B_SLICE)

  WRITE_FLAG( 0,    "output_flag_present_flag" );

#if DBL_CONTROL
  WRITE_FLAG( 1,    "deblocking_filter_control_present_flag");
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
  WRITE_CODE( 0,  3,                                "max_temporal_layers_minus1" );
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

  WRITE_UVLC( 0,                                    "bit_depth_luma_minus8" );
  WRITE_UVLC( 0,                                    "bit_depth_chroma_minus8" );

  WRITE_FLAG( 0,                                    "pcm_enabled_flag");

#if LOSSLESS_CODING
  WRITE_FLAG( 0,                                    "qpprime_y_zero_transquant_bypass_flag" );
#endif

  WRITE_UVLC( pcSPS->getBitsForPOC()-4,             "log2_max_pic_order_cnt_lsb_minus4" );
#if H0567_DPB_PARAMETERS_PER_TEMPORAL_LAYER
  WRITE_UVLC( 1,                                    "max_dec_pic_buffering[i]" );
  WRITE_UVLC( 0,                                    "num_reorder_pics[i]" );
  WRITE_UVLC( 0,                                    "max_latency_increase[i]" );
#else
  WRITE_UVLC( 1,                                    "max_num_ref_pics" ); 
  WRITE_UVLC( 0,                                    "num_reorder_frames" ); 
  WRITE_UVLC( 1,                                    "max_dec_frame_buffering" );
  WRITE_UVLC( 0,                                    "max_latency_increase"    );
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
  WRITE_FLAG( 1,                                                                     "restricted_ref_pic_lists_flag" );
  WRITE_FLAG( 0,                                                                     "lists_modification_present_flag" );
#endif
  WRITE_UVLC( log2MinCUSize - 3,                                                     "log2_min_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getMaxCUDepth()-g_uiAddCUDepth,                                 "log2_diff_max_min_coding_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MinSize() - 2,                                 "log2_min_transform_block_size_minus2" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize(), "log2_diff_max_min_transform_block_size" );

  if(log2MinCUSize == 3)
  {
    xWriteFlag  ( (pcSPS->getDisInter4x4()) ? 1 : 0 );
  }

  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthInter() - 1,                               "max_transform_hierarchy_depth_inter" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthIntra() - 1,                               "max_transform_hierarchy_depth_intra" );
  WRITE_FLAG( 0,                                                                     "scaling_list_enabled_flag" ); 
  WRITE_FLAG( pcSPS->getUseLMChroma () ? 1 : 0,                                      "chroma_pred_from_luma_enabled_flag" ); 
  WRITE_FLAG( 0,                                                                     "deblocking_filter_in_aps_enabled_flag");
  WRITE_FLAG( 1,                                                                     "seq_loop_filter_across_slices_enabled_flag");
  WRITE_FLAG( pcSPS->getUseAMP(),                                                    "asymmetric_motion_partitions_enabled_flag" );
  WRITE_FLAG( pcSPS->getUseNSQT(),                                                   "non_square_quadtree_enabled_flag" );
  WRITE_FLAG( 0,                                                                     "sample_adaptive_offset_enabled_flag");
  WRITE_FLAG( 0,                                                                     "adaptive_loop_filter_enabled_flag");
  WRITE_FLAG( 0,                                                                     "temporal_id_nesting_flag" );

#if RPS_IN_SPS
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD

  WRITE_UVLC(1, "num_short_term_ref_pic_sets" );
    codeShortTermRefPicSet();
  WRITE_FLAG( 0,                                             "long_term_ref_pics_present_flag" );
#endif
#if !PIC_CROPPING
  //!!!KS: Syntax not in WD !!!
  
  xWriteUvlc  ( 0 );
  xWriteUvlc  ( 0 );
#endif
  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }

#if TILES_WPP_ENTRY_POINT_SIGNALLING
  WRITE_CODE( 0, 2,                                                   "tiles_or_entropy_coding_sync_idc" );
#endif

  WRITE_FLAG( 0, "sps_extension_flag" );
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
  
  //write slice address
  WRITE_FLAG( 1, "first_slice_in_pic_flag" );

  WRITE_UVLC( pcSlice->getSliceType(),       "slice_type" );
  WRITE_FLAG( 0, "lightweight_slice_flag" );
  
  {
    WRITE_UVLC( pcSlice->getPPS()->getPPSId(), "pic_parameter_set_id" );
    if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    {
      WRITE_UVLC( 0, "idr_pic_id" );
      WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    else
    {
      WRITE_CODE( (pcSlice->getPOC()-pcSlice->getLastIDR()+(1<<pcSlice->getSPS()->getBitsForPOC()))%(1<<pcSlice->getSPS()->getBitsForPOC()), pcSlice->getSPS()->getBitsForPOC(), "pic_order_cnt_lsb");
        WRITE_FLAG( 1, "short_term_ref_pic_set_pps_flag");
        WRITE_UVLC( 0, "short_term_ref_pic_set_idx" );
    }

    // we always set num_ref_idx_active_override_flag equal to one. this might be done in a more intelligent way 
    if (!pcSlice->isIntra())
    {
      WRITE_FLAG( 1 ,                                             "num_ref_idx_active_override_flag");
      WRITE_CODE( 0 , 3,                                          "num_ref_idx_l0_active_minus1" );
    }
  }
  // ref_pic_list_combination( )
  // maybe move to own function?
    
  if(!pcSlice->isIntra())
  {
#if CABAC_INIT_FLAG
    if (!pcSlice->isIntra() && pcSlice->getPPS()->getCabacInitPresentFlag())
    {
      SliceType sliceType   = pcSlice->getSliceType();
      Int  encCABACTableIdx = pcSlice->getPPS()->getEncCABACTableIdx();
      Bool encCabacInitFlag = (sliceType!=encCABACTableIdx && encCABACTableIdx!=0) ? 1 : 0;
      pcSlice->setCabacInitFlag( encCabacInitFlag );
      WRITE_FLAG( encCabacInitFlag?1:0, "cabac_init_flag" );
    }
#else
    WRITE_UVLC(pcSlice->getCABACinitIDC(),  "cabac_init_idc");
#endif
  }

  // if( !lightweight_slice_flag ) {
  {
    Int iCode = pcSlice->getSliceQp() - ( pcSlice->getPPS()->getPicInitQPMinus26() + 26 );
    WRITE_SVLC( iCode, "slice_qp_delta" ); 
  //   if( sample_adaptive_offset_enabled_flag )
  //     sao_param()
  //   if( deblocking_filter_control_present_flag ) {
  //     disable_deblocking_filter_idc
    WRITE_FLAG(1, "loop_filter_disable");  // should be an IDC
  //     if( disable_deblocking_filter_idc  !=  1 ) {
  //       slice_alpha_c0_offset_div2
  //       slice_beta_offset_div2
  //     }
  //   }
  //   if( slice_type = = B )
  //   collocated_from_l0_flag
    //   if( adaptive_loop_filter_enabled_flag ) {
  //     if( !shared_pps_info_enabled_flag )
  //       alf_param( )
  //     alf_cu_control_param( )
  //   }
  // }
  }

  // !!!! sytnax elements not in the WD !!!!
  
  assert(pcSlice->getMaxNumMergeCand()<=MRG_MAX_NUM_CANDS_SIGNALED);
  assert(MRG_MAX_NUM_CANDS_SIGNALED<=MRG_MAX_NUM_CANDS);
  WRITE_UVLC(MRG_MAX_NUM_CANDS - pcSlice->getMaxNumMergeCand(), "maxNumMergeCand");

}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
}

Void TEncCavlc::codeSliceFinish ()
{
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx )
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

Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  assert(0);
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

//! \}
