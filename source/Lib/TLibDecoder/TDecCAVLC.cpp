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

/** \file     TDecCAVLC.cpp
    \brief    CAVLC decoder class
*/

#include "TDecCAVLC.h"
#include "SEIread.h"

//! \ingroup TLibDecoder
//! \{

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


Void  TDecCavlc::xReadCodeTr           (UInt length, UInt& rValue, const Char *pSymbolName)
{
  xReadCode (length, rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, rValue ); 
  fflush ( g_hTrace );
}

Void  TDecCavlc::xReadUvlcTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadUvlc (rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, rValue ); 
  fflush ( g_hTrace );
}

Void  TDecCavlc::xReadSvlcTr           (Int& rValue, const Char *pSymbolName)
{
  xReadSvlc(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, rValue ); 
  fflush ( g_hTrace );
}

Void  TDecCavlc::xReadFlagTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadFlag(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, rValue ); 
  fflush ( g_hTrace );
}

#else

#define READ_CODE(length, code, name)     xReadCode ( length, code )
#define READ_UVLC(        code, name)     xReadUvlc (         code )
#define READ_SVLC(        code, name)     xReadSvlc (         code )
#define READ_FLAG(        code, name)     xReadFlag (         code )

#endif



// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCavlc::TDecCavlc()
{
}

TDecCavlc::~TDecCavlc()
{
  
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 * unmarshal a sequence of SEI messages from bitstream.
 */
void TDecCavlc::parseSEI(SEImessages& seis)
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
void TDecCavlc::parseShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* pcRPS, Int idx )
{
  UInt uiCode;
  UInt uiInterRPSPred;
  READ_FLAG(uiInterRPSPred, "inter_RPS_flag");  assert( uiInterRPSPred == 0 );
  {
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
  }
#if PRINT_RPS_INFO
  pcRPS->printDeltaPOC();
#endif
}

Void TDecCavlc::parsePPS(TComPPS* pcPPS)
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
  UInt  uiCode;

  Int   iCode;

  TComRPS* pcRPSList = pcPPS->getRPSList();
  READ_UVLC( uiCode, "pic_parameter_set_id");                      pcPPS->setPPSId (uiCode);
  READ_UVLC( uiCode, "seq_parameter_set_id");                      pcPPS->setSPSId (uiCode);
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD
  TComReferencePictureSet*      pcRPS;

  READ_UVLC( uiCode, "num_short_term_ref_pic_sets" );
  pcRPSList->create(uiCode);

  for(UInt i=0; i< pcRPSList->getNumberOfReferencePictureSets(); i++)
  {
    pcRPS = pcRPSList->getReferencePictureSet(i);
    parseShortTermRefPicSet(pcPPS,pcRPS,i);
  }
  READ_FLAG( uiCode, "long_term_ref_pics_present_flag" );          pcPPS->setLongTermRefsPresent(uiCode);
  // entropy_coding_mode_flag
  // We code the entropy_coding_mode_flag, it's needed for tests.
  READ_FLAG( uiCode, "entropy_coding_mode_flag" );                 assert( uiCode == 1 );
  {
    READ_UVLC( uiCode, "entropy_coding_synchro" );                 assert( uiCode == 0 );
    READ_FLAG( uiCode, "cabac_istate_reset" );                     assert( uiCode == 0 );
  }
  READ_UVLC( uiCode, "num_temporal_layer_switching_point_flags" ); assert( uiCode == 0 );
  
  // num_ref_idx_l0_default_active_minus1
  // num_ref_idx_l1_default_active_minus1
  READ_SVLC(iCode, "pic_init_qp_minus26" );                        pcPPS->setPicInitQPMinus26(iCode);
  READ_FLAG( uiCode, "constrained_intra_pred_flag" );              assert( uiCode == 0 );
  READ_FLAG( uiCode, "enable_temporal_mvp_flag" );                 pcPPS->setEnableTMVPFlag( uiCode ? true : false );
  READ_CODE( 2, uiCode, "slice_granularity" );                     assert( uiCode == 0 );

  // alf_param() ?

  READ_UVLC( uiCode, "max_cu_qp_delta_depth");
  assert( uiCode == 0 );

  READ_SVLC( iCode, "chroma_qp_offset");
  assert( iCode == 0 );

  READ_SVLC( iCode, "chroma_qp_offset_2nd");
  assert( iCode == 0 );

  READ_FLAG( uiCode, "weighted_pred_flag" );          // Use of Weighting Prediction (P_SLICE)
  assert( uiCode == 0 );
  READ_CODE( 2, uiCode, "weighted_bipred_idc" );      // Use of Bi-Directional Weighting Prediction (B_SLICE)
  assert( uiCode == 0 );

  READ_FLAG ( uiCode, "tile_info_present_flag" );
  assert( uiCode == 0 );
#if NONCROSS_TILE_IN_LOOP_FILTERING
  READ_FLAG ( uiCode, "tile_control_present_flag" );
  assert( uiCode == 0 );
#endif

  return;
}

Void TDecCavlc::parseSPS(TComSPS* pcSPS)
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  
  UInt  uiCode;

  READ_CODE( 8,  uiCode, "profile_idc" );                        pcSPS->setProfileIdc( uiCode );
  READ_CODE( 8,  uiCode, "reserved_zero_8bits" );
  READ_CODE( 8,  uiCode, "level_idc" );                          pcSPS->setLevelIdc( uiCode );
  READ_UVLC(     uiCode, "seq_parameter_set_id" );               pcSPS->setSPSId( uiCode );
  READ_UVLC(     uiCode, "chroma_format_idc" );                  pcSPS->setChromaFormatIdc( uiCode );
  READ_CODE( 3,  uiCode, "max_temporal_layers_minus1" );         assert( uiCode == 0 );
  READ_UVLC (    uiCode, "pic_width_in_luma_samples" );          pcSPS->setWidth       ( uiCode    );
  READ_UVLC (    uiCode, "pic_height_in_luma_samples" );         pcSPS->setHeight      ( uiCode    );

  READ_UVLC(     uiCode, "bit_depth_luma_minus8" );
  assert( uiCode == 0 );
  
  READ_UVLC( uiCode,    "bit_depth_chroma_minus8" );

  READ_FLAG( uiCode, "pcm_enabled_flag" ); assert( uiCode == 0 );

  READ_UVLC( uiCode,    "log2_max_pic_order_cnt_lsb_minus4" );   pcSPS->setBitsForPOC( 4 + uiCode );
  READ_UVLC( uiCode,    "max_num_ref_pics" );                    assert( uiCode == 1 );
  READ_UVLC( uiCode,    "num_reorder_frames" );                  assert( uiCode == 0 );
  READ_UVLC ( uiCode, "max_dec_frame_buffering");
  pcSPS->setMaxDecFrameBuffering( uiCode );
  READ_UVLC ( uiCode, "max_latency_increase");
  pcSPS->setMaxLatencyIncrease( uiCode );
  READ_UVLC( uiCode, "log2_min_coding_block_size_minus3" );
  UInt log2MinCUSize = uiCode + 3;
  READ_UVLC( uiCode, "log2_diff_max_min_coding_block_size" );
  UInt uiMaxCUDepthCorrect = uiCode;
  pcSPS->setMaxCUWidth  ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUWidth  = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  pcSPS->setMaxCUHeight ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUHeight = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  READ_UVLC( uiCode, "log2_min_transform_block_size_minus2" );   pcSPS->setQuadtreeTULog2MinSize( uiCode + 2 );

  READ_UVLC( uiCode, "log2_diff_max_min_transform_block_size" ); pcSPS->setQuadtreeTULog2MaxSize( uiCode + pcSPS->getQuadtreeTULog2MinSize() );
  pcSPS->setMaxTrSize( 1<<(uiCode + pcSPS->getQuadtreeTULog2MinSize()) );

  if(log2MinCUSize == 3)
  {
    xReadFlag( uiCode ); pcSPS->setDisInter4x4( uiCode ? true : false );
  }

  READ_UVLC( uiCode, "max_transform_hierarchy_depth_inter" );    pcSPS->setQuadtreeTUMaxDepthInter( uiCode+1 );
  READ_UVLC( uiCode, "max_transform_hierarchy_depth_intra" );    pcSPS->setQuadtreeTUMaxDepthIntra( uiCode+1 );
  g_uiAddCUDepth = 0;
  while( ( pcSPS->getMaxCUWidth() >> uiMaxCUDepthCorrect ) > ( 1 << ( pcSPS->getQuadtreeTULog2MinSize() + g_uiAddCUDepth )  ) ) g_uiAddCUDepth++;    
  pcSPS->setMaxCUDepth( uiMaxCUDepthCorrect+g_uiAddCUDepth  ); g_uiMaxCUDepth  = uiMaxCUDepthCorrect+g_uiAddCUDepth;
  // BB: these parameters may be removed completly and replaced by the fixed values
  pcSPS->setMinTrDepth( 0 );
  pcSPS->setMaxTrDepth( 1 );
  READ_FLAG( uiCode, "scaling_list_enable_flag" );               assert( uiCode == 0 );
  READ_FLAG( uiCode, "chroma_pred_from_luma_enabled_flag" );     assert( uiCode == 0 );
  READ_FLAG( uiCode, "deblocking_filter_In_APS_enabled_flag" );  assert( uiCode == 0 );
  READ_FLAG( uiCode, "loop_filter_across_slice_flag" );          assert( uiCode == 1 );
  READ_FLAG( uiCode, "sample_adaptive_offset_enabled_flag" );    assert( uiCode == 0 );
  READ_FLAG( uiCode, "adaptive_loop_filter_enabled_flag" );      assert( uiCode == 0 );

  READ_FLAG( uiCode, "temporal_id_nesting_flag" );               assert( uiCode == 0 );

  // !!!KS: Syntax not in WD !!!

  xReadUvlc ( uiCode ); assert( uiCode == 0 );
  xReadUvlc ( uiCode ); assert( uiCode == 0 );

  xReadFlag( uiCode ); pcSPS->setUseMRG ( uiCode ? true : false );
  
  // AMVP mode for each depth (AM_NONE or AM_EXPL)
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xReadFlag( uiCode );
    pcSPS->setAMVPMode( i, (AMVP_MODE)uiCode );
  }

  READ_FLAG ( uiCode, "uniform_spacing_flag" ); 
  assert( uiCode == 0 );
  READ_UVLC ( uiCode, "num_tile_columns_minus1" );
  assert( uiCode == 0 );
  READ_UVLC ( uiCode, "num_tile_rows_minus1" ); 
  assert( uiCode == 0 );

  // Software-only flags
  READ_FLAG( uiCode, "enable_nsqt" );
  pcSPS->setUseNSQT( uiCode );
  READ_FLAG( uiCode, "enable_amp" );
  pcSPS->setUseAMP( uiCode );
  return;
}

Void TDecCavlc::parseSliceHeader (TComSlice*& rpcSlice)
{
  UInt  uiCode;
  Int   iCode;
  
#if ENC_DEC_TRACE
  xTraceSliceHeader(rpcSlice);
#endif
  
  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //   slice_address
  Int numCUs = ((rpcSlice->getSPS()->getWidth()+rpcSlice->getSPS()->getMaxCUWidth()-1)/rpcSlice->getSPS()->getMaxCUWidth())*((rpcSlice->getSPS()->getHeight()+rpcSlice->getSPS()->getMaxCUHeight()-1)/rpcSlice->getSPS()->getMaxCUHeight());
  Int maxParts = (1<<(rpcSlice->getSPS()->getMaxCUDepth()<<1));
  Int reqBitsOuter = 0;
  while(numCUs>(1<<reqBitsOuter))
  {
    reqBitsOuter++;
  }
  READ_FLAG( uiCode, "first_slice_in_pic_flag" );
  assert( uiCode == 1 );

  //   slice_type
  READ_UVLC (    uiCode, "slice_type" );            rpcSlice->setSliceType((SliceType)uiCode);
  // lightweight_slice_flag
  READ_FLAG( uiCode, "lightweight_slice_flag" );
  assert( uiCode == 0 );

  {
    rpcSlice->setNextSlice        ( true  );
    
    rpcSlice->setSliceCurEndCUAddr(numCUs*maxParts);
  }
  
  // if( !lightweight_slice_flag ) {
  {
    READ_UVLC (    uiCode, "pic_parameter_set_id" );  rpcSlice->setPPSId(uiCode);
    if(rpcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    { 
      READ_UVLC( uiCode, "idr_pic_id" );  //ignored
      READ_FLAG( uiCode, "no_output_of_prior_pics_flag" );  //ignored
      rpcSlice->setPOC(0);
      TComReferencePictureSet* pcRPS = rpcSlice->getLocalRPS();
      pcRPS->setNumberOfNegativePictures(0);
      pcRPS->setNumberOfPositivePictures(0);
      pcRPS->setNumberOfLongtermPictures(0);
      pcRPS->setNumberOfPictures(0);
      rpcSlice->setRPS(pcRPS);
    }
    else
    {
      READ_CODE(rpcSlice->getSPS()->getBitsForPOC(), uiCode, "pic_order_cnt_lsb");  
      Int iPOClsb = uiCode;
      Int iPrevPOC = rpcSlice->getPrevPOC();
      Int iMaxPOClsb = 1<<rpcSlice->getSPS()->getBitsForPOC();
      Int iPrevPOClsb = iPrevPOC%iMaxPOClsb;
      Int iPrevPOCmsb = iPrevPOC-iPrevPOClsb;
      Int iPOCmsb;
      if( ( iPOClsb  <  iPrevPOClsb ) && ( ( iPrevPOClsb - iPOClsb )  >=  ( iMaxPOClsb / 2 ) ) )
      {
        iPOCmsb = iPrevPOCmsb + iMaxPOClsb;
      }
      else if( (iPOClsb  >  iPrevPOClsb )  && ( (iPOClsb - iPrevPOClsb )  >  ( iMaxPOClsb / 2 ) ) ) 
      {
        iPOCmsb = iPrevPOCmsb - iMaxPOClsb;
      }
      else
      {
        iPOCmsb = iPrevPOCmsb;
      }
      rpcSlice->setPOC              (iPOCmsb+iPOClsb);

      TComReferencePictureSet* pcRPS;
      READ_FLAG( uiCode, "short_term_ref_pic_set_pps_flag" );
      if(uiCode == 0) // use short-term reference picture set explicitly signalled in slice header
      {
        pcRPS = rpcSlice->getLocalRPS();
        parseShortTermRefPicSet(rpcSlice->getPPS(),pcRPS, rpcSlice->getPPS()->getRPSList()->getNumberOfReferencePictureSets());
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
      {
        refPicListModification->setNumberOfRefPicListModificationsL0(0); 
      }
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
      {
        refPicListModification->setNumberOfRefPicListModificationsL1(0);
      }
    }  
    else
    {
      refPicListModification->setRefPicListModificationFlagL1(0);
      refPicListModification->setNumberOfRefPicListModificationsL1(0);
    }
  }
  // ref_pic_list_combination( )
  if (rpcSlice->isInterB())
  {
    READ_FLAG( uiCode, "ref_pic_list_combination_flag" );       rpcSlice->setRefPicListCombinationFlag( uiCode ? 1 : 0 );
    if(uiCode)
    {
      READ_UVLC( uiCode, "num_ref_idx_lc_active_minus1" );      rpcSlice->setNumRefIdx( REF_PIC_LIST_C, uiCode + 1 );
      
      READ_FLAG( uiCode, "ref_pic_list_modification_flag_lc" ); rpcSlice->setRefPicListModificationFlagLC( uiCode ? 1 : 0 );
      if(uiCode)
      {
        for (UInt i=0;i<rpcSlice->getNumRefIdx(REF_PIC_LIST_C);i++)
        {
          READ_FLAG( uiCode, "pic_from_list_0_flag" );
          rpcSlice->setListIdFromIdxOfLC(i, uiCode);
          READ_UVLC( uiCode, "ref_idx_list_curr" );
          rpcSlice->setRefIdxFromIdxOfLC(i, uiCode);
          rpcSlice->setRefIdxOfLC((RefPicList)rpcSlice->getListIdFromIdxOfLC(i), rpcSlice->getRefIdxFromIdxOfLC(i), i);
        }
      }
    }
    else
    {
      rpcSlice->setRefPicListModificationFlagLC(false);
      rpcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
    }
  }
  else
  {
    rpcSlice->setRefPicListCombinationFlag(false);      
  }
  
  if(!rpcSlice->isIntra())
  {
    READ_UVLC(uiCode, "cabac_init_idc");
    rpcSlice->setCABACinitIDC(uiCode);
  }
  else if (rpcSlice->isIntra())
  {
    rpcSlice->setCABACinitIDC(0);
  }

  {
    READ_SVLC( iCode, "slice_qp_delta" ); 
    rpcSlice->setSliceQp (26 + rpcSlice->getPPS()->getPicInitQPMinus26() + iCode);
    //   if( sample_adaptive_offset_enabled_flag )
    //     sao_param()
    //   if( deblocking_filter_control_present_flag ) {
    //     disable_deblocking_filter_idc
    // this should be an idc
    READ_FLAG ( uiCode, "inherit_dbl_param_from_APS_flag" ); assert( uiCode == 0 );
    READ_FLAG ( uiCode, "loop_filter_disable" );  assert( uiCode == 1 );
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
    
    //   if( adaptive_loop_filter_enabled_flag ) {
    //     if( !shared_pps_info_enabled_flag )
    //       alf_param( )
    //     alf_cu_control_param( )
    //   }
    // }
  }

  // !!!! Syntax elements not in the WD  !!!!!
  
  READ_UVLC( uiCode, "MaxNumMergeCand");
  rpcSlice->setMaxNumMergeCand(MRG_MAX_NUM_CANDS - uiCode);
  assert(rpcSlice->getMaxNumMergeCand()==MRG_MAX_NUM_CANDS_SIGNALED);
}

Void TDecCavlc::parseWPPTileInfoToSliceHeader(TComSlice*& rpcSlice)
{
  UInt uiCode;

      xReadCode(1, uiCode); // read flag indicating if tile markers transmitted
      assert( uiCode == 0 );

    // Reading location information
    {   
      xReadCode(1, uiCode); // read flag indicating if location information signaled in slice header
      assert( uiCode == 0 );

      // read out trailing bits
      m_pcBitstream->readOutTrailingBits();
    }
}


Void TDecCavlc::resetEntropy          (TComSlice* pcSlice)
{
  m_bRunLengthCoding = ! pcSlice->isIntra();
  m_uiRun = 0;
  
  ::memset(m_ucCBP_YUV_TableCounter, 0, 4*4*sizeof(UChar));
  ::memset(m_ucCBP_4Y_TableCounter,  0, 2*2*sizeof(UChar));
  ::memset(m_ucCBP_YCS_TableCounter, 0, 2*4*sizeof(UChar));
  ::memset(m_ucCBP_YS_TableCounter,  0, 2*3*sizeof(UChar));

  ::memset(m_ucMI1TableCounter,        0,          4*sizeof(UChar));
  ::memset(m_ucSplitTableCounter,      0,        4*4*sizeof(UChar));

  m_ucCBP_YUV_TableCounterSum[0] = m_ucCBP_YUV_TableCounterSum[1] = m_ucCBP_YUV_TableCounterSum[2] = m_ucCBP_YUV_TableCounterSum[3] = 0;
  m_ucCBP_4Y_TableCounterSum[0] = m_ucCBP_4Y_TableCounterSum[1] = 0;
  m_ucCBP_YCS_TableCounterSum[0] = m_ucCBP_YCS_TableCounterSum[1] = 0;
  m_ucCBP_YS_TableCounterSum[0] = m_ucCBP_YS_TableCounterSum[1] = 0;

  m_ucSplitTableCounterSum[0] = m_ucSplitTableCounterSum[1] = m_ucSplitTableCounterSum[2]= m_ucSplitTableCounterSum[3] = 0;
  m_ucMI1TableCounterSum = 0;
}

Void TDecCavlc::parseTerminatingBit( UInt& ruiBit )
{
  ruiBit = false;
  Int iBitsLeft = m_pcBitstream->getNumBitsLeft();
  if(iBitsLeft <= 8)
  {
    UInt uiPeekValue = m_pcBitstream->peekBits(iBitsLeft);
    if (uiPeekValue == (1<<(iBitsLeft-1)))
    {
      ruiBit = true;
    }
  }
}

Void TDecCavlc::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseMVPIdx( Int& riMVPIdx )
{
  assert(0);
}

Void TDecCavlc::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{ 
  assert(0);
}

Void TDecCavlc::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  assert(0);
}

Void TDecCavlc::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  assert(0);
}

Void TDecCavlc::parseCbfTrdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiTrDepth, UInt uiDepth, UInt& uiSubdiv )
{
  assert(0);
}

UInt TDecCavlc::xGetFlagPattern( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiSubdiv )
{
  assert(0);
  return 0;
}

Void TDecCavlc::parseCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseBlockCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth, UInt uiQPartNum )
{
  assert(0);
}

Void TDecCavlc::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  assert(0);
}

Void TDecCavlc::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  assert(0);
}

Void TDecCavlc::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  assert(0);
}

Void TDecCavlc::parseQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf )
{
  assert(0);
}

Void TDecCavlc::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{
  assert(0);
}

Void TDecCavlc::parseMergeIndex ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

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

Void TDecCavlc::xReadUnaryMaxSymbol( UInt& ruiSymbol, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
  
  xReadFlag( ruiSymbol );
  
  if (ruiSymbol == 0 || uiMaxSymbol == 1)
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    xReadFlag( uiCont );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );
  
  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecCavlc::xReadExGolombLevel( UInt& ruiSymbol )
{
  UInt uiSymbol ;
  UInt uiCount = 0;
  do
  {
    xReadFlag( uiSymbol );
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

Void TDecCavlc::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  
  while( uiBit )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  uiCount--;
  while( uiCount-- )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount;
  }
  
  ruiSymbol = uiSymbol;
  
  return;
}

UInt TDecCavlc::xGetBit()
{
  UInt ruiCode;
  m_pcBitstream->read( 1, ruiCode );
  return ruiCode;
}


//! \}
