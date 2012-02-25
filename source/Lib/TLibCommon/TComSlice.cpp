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

/** \file     TComSlice.cpp
    \brief    slice header and SPS class
*/

#include "CommonDef.h"
#include "TComSlice.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

TComSlice::TComSlice()
: m_iPPSId                        ( -1 )
, m_iPOC                          ( 0 )
, m_iLastIDR                      ( 0 )
, m_eNalUnitType                  ( NAL_UNIT_CODED_SLICE_IDR )
, m_eSliceType                    ( I_SLICE )
, m_iSliceQp                      ( 0 )
, m_iDepth                        ( 0 )
, m_bRefenced                     ( false )
, m_pcSPS                         ( NULL )
, m_pcPPS                         ( NULL )
, m_pcPic                         ( NULL )
, m_dLambda                       ( 0.0 )
, m_uiSliceCurEndCUAddr           ( 0 )
, m_bNextSlice                    ( false )
, m_uiSliceBits                   ( 0 )
, m_bFinalized                    ( false )
#if CABAC_INIT_FLAG
, m_cabacInitFlag                 ( false )
#else
, m_cabacInitIdc                 ( -1 )
#endif
{
  m_pcRefPicList = NULL;
  m_iRefPOCList = 0;
}

TComSlice::~TComSlice()
{
}


Void TComSlice::initSlice()
{
  m_uiMaxNumMergeCand = MRG_MAX_NUM_CANDS_SIGNALED;

  m_bFinalized=false;
#if CABAC_INIT_FLAG
  m_cabacInitFlag        = false;
#endif
}

Void TComSlice::setRefPOCList       ()
{
    if ( !isIntra() )
      m_iRefPOCList = m_pcRefPicList->getPOC();
}

Void TComSlice::setRefPicList( TComPic* pcListPic[2] )
{
  if (m_eSliceType == I_SLICE)
  {
    m_pcRefPicList = NULL;

    return;
  }

  TComPic*  pcRefPic;

      pcRefPic = pcListPic[0];
      pcRefPic->getPicYuvRec()->extendPicBorder();
      // ref_pic_list_init
      m_pcRefPicList = pcRefPic;
}

/** Function for marking the reference pictures when an IDR and CDR is encountered.
 * \param uiPOCCDR POC of the CDR picture
 * \param bRefreshPending flag indicating if a deferred decoding refresh is pending
 * \param rcListPic reference to the reference picture list
 * This function marks the reference pictures as "unused for reference" in the following conditions.
 * If the nal_unit_type is IDR all pictures in the reference picture list  
 * is marked as "unused for reference" 
 * Otherwise do for the CDR case (non CDR case has no effect since both if conditions below will not be true)
 *    If the bRefreshPending flag is true (a deferred decoding refresh is pending) and the current 
 *    temporal reference is greater than the temporal reference of the latest CDR picture (uiPOCCDR), 
 *    mark all reference pictures except the latest CDR picture as "unused for reference" and set 
 *    the bRefreshPending flag to false.
 *    If the nal_unit_type is CDR, set the bRefreshPending flag to true and iPOCCDR to the temporal 
 *    reference of the current picture.
 * Note that the current picture is already placed in the reference list and its marking is not changed.
 * If the current picture has a nal_ref_idc that is not 0, it will remain marked as "used for reference".
 */
Void TComSlice::decodingRefreshMarking(TComPic* pcListPic[2])
{
  UInt uiPOCCurr = getPOC(); 

  if (getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)  // IDR
  {
    TComPic* pcPic = pcListPic[0];
    pcPic->getSlice()->setReferenced(false);
    //assert( pcPic->getPOC() != uiPOCCurr );
  }
}

int TComSlice::m_iPrevPOC = 0;

// ------------------------------------------------------------------------------------------------
// Sequence parameter set (SPS)
// ------------------------------------------------------------------------------------------------

TComSPS::TComSPS()
: m_SPSId                     (  0)
, m_ProfileIdc                (  0)
, m_LevelIdc                  (  0)
, m_chromaFormatIdc           (CHROMA_420)
// Structure
, m_picWidthInLumaSamples     (352)
, m_picHeightInLumaSamples    (288)
#if PIC_CROPPING
, m_picCroppingFlag           (false)
, m_picCropLeftOffset         (  0)
, m_picCropRightOffset        (  0)
, m_picCropTopOffset          (  0)
, m_picCropBottomOffset       (  0) 
#endif
, m_uiMaxCUWidth              ( 32)
, m_uiMaxCUHeight             ( 32)
, m_uiMaxCUDepth              (  3)
, m_uiMinTrDepth              (  0)
, m_uiMaxTrDepth              (  1)
, m_uiQuadtreeTULog2MaxSize   (  0)
, m_uiQuadtreeTULog2MinSize   (  0)
, m_uiQuadtreeTUMaxDepthInter (  0)
, m_uiQuadtreeTUMaxDepthIntra (  0)
// Tool list
, m_bDisInter4x4              (  1)
, m_bUseLMChroma              (false)
, m_uiBitsForPOC              (  8)
, m_uiMaxTrSize               ( 32)
{
  // AMVP parameter
  ::memset( m_aeAMVPMode, 0, sizeof( m_aeAMVPMode ) );
}

TComSPS::~TComSPS()
{
}

TComPPS::TComPPS()
: m_PPSId                       (0)
, m_SPSId                       (0)
, m_picInitQPMinus26            (0)
, m_pcSPS                       (NULL)
#if MULTIBITS_DATA_HIDING
, m_signHideFlag(0)
, m_signHidingThreshold(0)
#endif
#if CABAC_INIT_FLAG
, m_cabacInitPresentFlag        (false)
, m_encCABACTableIdx            (0)
#endif
{
}

TComPPS::~TComPPS()
{
}

//! \}
