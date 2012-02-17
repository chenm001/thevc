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
, m_bNoBackPredFlag               ( false )
, m_bRefIdxCombineCoding          ( false )
, m_uiSliceCurEndCUAddr           ( 0 )
, m_bNextSlice                    ( false )
, m_uiSliceBits                   ( 0 )
, m_bFinalized                    ( false )
, m_cabacInitIdc                 ( -1 )
{
  m_iNumRefIdx = 0;
  
  m_pcRefPicList = NULL;
  m_iRefPOCList = 0;
}

TComSlice::~TComSlice()
{
}


Void TComSlice::initSlice()
{
  m_iNumRefIdx = 0;
  
  m_bNoBackPredFlag = false;
  m_bRefIdxCombineCoding = false;

  m_uiMaxNumMergeCand = MRG_MAX_NUM_CANDS_SIGNALED;

  m_bFinalized=false;
}

Void  TComSlice::sortPicList        (TComList<TComPic*>& rcListPic)
{
  TComPic*    pcPicExtract;
  TComPic*    pcPicInsert;
  
  TComList<TComPic*>::iterator    iterPicExtract;
  TComList<TComPic*>::iterator    iterPicExtract_1;
  TComList<TComPic*>::iterator    iterPicInsert;
  
  for (Int i = 1; i < (Int)(rcListPic.size()); i++)
  {
    iterPicExtract = rcListPic.begin();
    for (Int j = 0; j < i; j++) iterPicExtract++;
    pcPicExtract = *(iterPicExtract);
    
    iterPicInsert = rcListPic.begin();
    while (iterPicInsert != iterPicExtract)
    {
      pcPicInsert = *(iterPicInsert);
      if (pcPicInsert->getPOC() >= pcPicExtract->getPOC())
      {
        break;
      }
      
      iterPicInsert++;
    }
    
    iterPicExtract_1 = iterPicExtract;    iterPicExtract_1++;
    
    //  swap iterPicExtract and iterPicInsert, iterPicExtract = curr. / iterPicInsert = insertion position
    rcListPic.insert (iterPicInsert, iterPicExtract, iterPicExtract_1);
    rcListPic.erase  (iterPicExtract);
  }
}

TComPic* TComSlice::xGetRefPic (TComList<TComPic*>& rcListPic,
                                UInt                uiPOC)
{
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();  
  TComPic*                      pcPic = *(iterPic);
  while ( iterPic != rcListPic.end() )
  {
    if(pcPic->getPOC() == uiPOC)
      break;
    iterPic++;
    pcPic = *(iterPic);
  }
  return  pcPic;
}


TComPic* TComSlice::xGetLongTermRefPic (TComList<TComPic*>& rcListPic,
                                UInt                uiPOC)
{
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();  
  TComPic*                      pcPic = *(iterPic);
  TComPic*                      pcStPic = pcPic;
  while ( iterPic != rcListPic.end() )
  {
    pcPic = *(iterPic);
    if(pcPic && (pcPic->getPOC()%(1<<getSPS()->getBitsForPOC())) == (uiPOC%(1<<getSPS()->getBitsForPOC())))
    {
        pcStPic = pcPic;
      break;
    }

    iterPic++;
  }
  return  pcStPic;
}

Void TComSlice::setRefPOCList       ()
{
    if ( m_iNumRefIdx > 0 )
      m_iRefPOCList = m_pcRefPicList->getPOC();
}

Void TComSlice::setRefPicList( TComList<TComPic*>& rcListPic )
{
  if (m_eSliceType == I_SLICE)
  {
    m_pcRefPicList = NULL;
    m_iNumRefIdx = 0;

    return;
  }

  TComPic*  pcRefPic;

    if(m_pcRPS->getUsed(0))
    {
      pcRefPic = xGetRefPic(rcListPic, getPOC()-1);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      // ref_pic_list_init
      m_pcRefPicList = pcRefPic;
    }
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
Void TComSlice::decodingRefreshMarking(UInt& uiPOCCDR, Bool& bRefreshPending, TComList<TComPic*>& rcListPic)
{
  TComPic*                 rpcPic;
  UInt uiPOCCurr = getPOC(); 

  if (getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)  // IDR
  {
    // mark all pictures as not used for reference
    TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
    while (iterPic != rcListPic.end())
    {
      rpcPic = *(iterPic);
      if (rpcPic->getPOC() != uiPOCCurr) rpcPic->getSlice()->setReferenced(false);
      iterPic++;
    }
  }
  else // CDR or No DR
  {
    if (bRefreshPending==true && uiPOCCurr > uiPOCCDR) // CDR reference marking pending 
    {
      TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
      while (iterPic != rcListPic.end())
      {
        rpcPic = *(iterPic);
        if (rpcPic->getPOC() != uiPOCCurr && rpcPic->getPOC() != uiPOCCDR) rpcPic->getSlice()->setReferenced(false);
        iterPic++;
      }
      bRefreshPending = false; 
    }
    if (getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR) // CDR picture found
    {
      bRefreshPending = true; 
      uiPOCCDR = uiPOCCurr;
    }
  }
}

int TComSlice::m_iPrevPOC = 0;

/** Function for applying picture marking based on the Reference Picture Set in pReferencePictureSet.
*/
Void TComSlice::applyReferencePictureSet( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet)
{
  TComPic* rpcPic;
  Int isReference;

  Int j = 0;
  // loop through all pictures in the reference picture buffer
  TComList<TComPic*>::iterator iterPic = rcListPic.begin();
  while ( iterPic != rcListPic.end())
  {
    j++;
    rpcPic = *(iterPic++);

    isReference = 0;
    // loop through all pictures in the Reference Picture Set
    // to see if the picture should be kept as reference picture
      if(rpcPic->getPicSym()->getSlice()->getPOC() == this->getPOC() - 1)
      {
        isReference = 1;
        rpcPic->setUsedByCurr(pReferencePictureSet->getUsed(0));
      }
    // mark the picture as "unused for reference" if it is not in
    // the Reference Picture Set
    if(rpcPic->getPicSym()->getSlice()->getPOC() != this->getPOC() && isReference == 0)    
    {            
      rpcPic->getSlice()->setReferenced( false );   
    }
  }  
}

Void TComSlice::decodingMarkingForNoTMVP( TComList<TComPic*>& rcListPic, Int currentPOC )
{
  TComList<TComPic*>::iterator it;
  for ( it = rcListPic.begin(); it != rcListPic.end(); it++ )
  {
    if ( (*it)->getSlice()->getPOC() != currentPOC )
    {
      (*it)->setUsedForTMVP( false );
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Sequence parameter set (SPS)
// ------------------------------------------------------------------------------------------------

TComSPS::TComSPS()
: m_SPSId                     (  0)
, m_ProfileIdc                (  0)
, m_LevelIdc                  (  0)
, m_chromaFormatIdc           (CHROMA_420)
// Structure
, m_uiWidth                   (352)
, m_uiHeight                  (288)
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
, m_bUseMRG                   (false)
, m_bUseLComb                 (false)
, m_bLCMod                    (false)
, m_uiBitsForPOC              (  8)
, m_uiMaxTrSize               ( 32)
, m_uiMaxDecFrameBuffering    (  0)
, m_uiMaxLatencyIncrease      (  0)
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
, m_uiBitsForLongTermRefs       (0)
{
}

TComPPS::~TComPPS()
{
}

TComReferencePictureSet::TComReferencePictureSet()
{
  ::memset( m_piPOC, 0, sizeof(m_piPOC) );
  ::memset( m_pbUsed, 0, sizeof(m_pbUsed) );
}

TComReferencePictureSet::~TComReferencePictureSet()
{
}

Void TComReferencePictureSet::setUsed(UInt uiBufferNum, Bool bUsed)
{
   m_pbUsed[uiBufferNum] = bUsed;
}

UInt TComReferencePictureSet::getUsed(UInt uiBufferNum)
{
   return (UInt)m_pbUsed[uiBufferNum];
}

Int TComReferencePictureSet::getPOC(UInt uiBufferNum)
{
   return m_piPOC[uiBufferNum];
}
Void TComReferencePictureSet::setPOC(UInt uiBufferNum, Int iPOC)
{
   m_piPOC[uiBufferNum] = iPOC;
}

/** Prints the deltaPOC and RefIdc (if available) values in the RPS.
 *  A "*" is added to the deltaPOC value if it is Used bu current.
 * \returns Void
 */
Void TComReferencePictureSet::printDeltaPOC()
{
  printf("DeltaPOC = { ");
    printf("%d%s ", -1, (getUsed(0)==1)?"*":"");
  printf("}\n");
}

#if PARAMSET_VLC_CLEANUP

ParameterSetManager::ParameterSetManager()
: m_spsMap(MAX_NUM_SPS)
, m_ppsMap(MAX_NUM_PPS)
{
}


ParameterSetManager::~ParameterSetManager()
{
}

#endif

//! \}
