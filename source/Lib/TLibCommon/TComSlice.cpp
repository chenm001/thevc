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
, m_bRefPicListModificationFlagLC ( false )
, m_bRefPicListCombinationFlag    ( false )
, m_iSliceQpDelta                 ( 0 )
, m_iDepth                        ( 0 )
, m_bRefenced                     ( false )
, m_pcSPS                         ( NULL )
, m_pcPPS                         ( NULL )
, m_pcPic                         ( NULL )
, m_uiColDir                      ( 0 )
, m_dLambda                       ( 0.0 )
, m_bNoBackPredFlag               ( false )
, m_bRefIdxCombineCoding          ( false )
, m_uiSliceCurEndCUAddr           ( 0 )
, m_bNextSlice                    ( false )
, m_uiSliceBits                   ( 0 )
, m_bFinalized                    ( false )
, m_cabacInitIdc                 ( -1 )
{
  m_aiNumRefIdx[0] = m_aiNumRefIdx[1] = m_aiNumRefIdx[2] = 0;
  
  initEqualRef();
  
  for(Int iNumCount = 0; iNumCount < MAX_NUM_REF_LC; iNumCount++)
  {
    m_iRefIdxOfLC[REF_PIC_LIST_0][iNumCount]=-1;
    m_iRefIdxOfLC[REF_PIC_LIST_1][iNumCount]=-1;
    m_eListIdFromIdxOfLC[iNumCount]=0;
    m_iRefIdxFromIdxOfLC[iNumCount]=0;
    m_iRefIdxOfL0FromRefIdxOfL1[iNumCount] = -1;
    m_iRefIdxOfL1FromRefIdxOfL0[iNumCount] = -1;
  }    
  for(Int iNumCount = 0; iNumCount < MAX_NUM_REF; iNumCount++)
  {
    m_apcRefPicList [0][iNumCount] = NULL;
    m_apcRefPicList [1][iNumCount] = NULL;
    m_aiRefPOCList  [0][iNumCount] = 0;
    m_aiRefPOCList  [1][iNumCount] = 0;
  }
  m_bCombineWithReferenceFlag = 0;
}

TComSlice::~TComSlice()
{
}


Void TComSlice::initSlice()
{
  m_aiNumRefIdx[0]      = 0;
  m_aiNumRefIdx[1]      = 0;
  
  m_uiColDir = 0;
  
  initEqualRef();
  m_bNoBackPredFlag = false;
  m_bRefIdxCombineCoding = false;
  m_bRefPicListCombinationFlag = false;
  m_bRefPicListModificationFlagLC = false;

  m_aiNumRefIdx[REF_PIC_LIST_C]      = 0;

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
      if(pcPic->getIsLongTerm())
        return pcPic;
      else
        pcStPic = pcPic;
      break;
    }

    iterPic++;
  }
  return  pcStPic;
}

Void TComSlice::setRefPOCList       ()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    for (Int iNumRefIdx = 0; iNumRefIdx < m_aiNumRefIdx[iDir]; iNumRefIdx++)
    {
      m_aiRefPOCList[iDir][iNumRefIdx] = m_apcRefPicList[iDir][iNumRefIdx]->getPOC();
    }
  }

}

Void TComSlice::generateCombinedList()
{
  if(m_aiNumRefIdx[REF_PIC_LIST_C] > 0)
  {
    m_aiNumRefIdx[REF_PIC_LIST_C]=0;
    for(Int iNumCount = 0; iNumCount < MAX_NUM_REF_LC; iNumCount++)
    {
      m_iRefIdxOfLC[REF_PIC_LIST_0][iNumCount]=-1;
      m_iRefIdxOfLC[REF_PIC_LIST_1][iNumCount]=-1;
      m_eListIdFromIdxOfLC[iNumCount]=0;
      m_iRefIdxFromIdxOfLC[iNumCount]=0;
      m_iRefIdxOfL0FromRefIdxOfL1[iNumCount] = -1;
      m_iRefIdxOfL1FromRefIdxOfL0[iNumCount] = -1;
    }

    for (Int iNumRefIdx = 0; iNumRefIdx < MAX_NUM_REF; iNumRefIdx++)
    {
      if(iNumRefIdx < m_aiNumRefIdx[REF_PIC_LIST_0])
      {
        Bool bTempRefIdxInL2 = true;
        for ( Int iRefIdxLC = 0; iRefIdxLC < m_aiNumRefIdx[REF_PIC_LIST_C]; iRefIdxLC++ )
        {
          if ( m_apcRefPicList[REF_PIC_LIST_0][iNumRefIdx]->getPOC() == m_apcRefPicList[m_eListIdFromIdxOfLC[iRefIdxLC]][m_iRefIdxFromIdxOfLC[iRefIdxLC]]->getPOC() )
          {
            m_iRefIdxOfL1FromRefIdxOfL0[iNumRefIdx] = m_iRefIdxFromIdxOfLC[iRefIdxLC];
            m_iRefIdxOfL0FromRefIdxOfL1[m_iRefIdxFromIdxOfLC[iRefIdxLC]] = iNumRefIdx;
            bTempRefIdxInL2 = false;
            break;
          }
        }

        if(bTempRefIdxInL2 == true)
        { 
          m_eListIdFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = REF_PIC_LIST_0;
          m_iRefIdxFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = iNumRefIdx;
          m_iRefIdxOfLC[REF_PIC_LIST_0][iNumRefIdx] = m_aiNumRefIdx[REF_PIC_LIST_C]++;
        }
      }

      if(iNumRefIdx < m_aiNumRefIdx[REF_PIC_LIST_1])
      {
        Bool bTempRefIdxInL2 = true;
        for ( Int iRefIdxLC = 0; iRefIdxLC < m_aiNumRefIdx[REF_PIC_LIST_C]; iRefIdxLC++ )
        {
          if ( m_apcRefPicList[REF_PIC_LIST_1][iNumRefIdx]->getPOC() == m_apcRefPicList[m_eListIdFromIdxOfLC[iRefIdxLC]][m_iRefIdxFromIdxOfLC[iRefIdxLC]]->getPOC() )
          {
            m_iRefIdxOfL0FromRefIdxOfL1[iNumRefIdx] = m_iRefIdxFromIdxOfLC[iRefIdxLC];
            m_iRefIdxOfL1FromRefIdxOfL0[m_iRefIdxFromIdxOfLC[iRefIdxLC]] = iNumRefIdx;
            bTempRefIdxInL2 = false;
            break;
          }
        }
        if(bTempRefIdxInL2 == true)
        {
          m_eListIdFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = REF_PIC_LIST_1;
          m_iRefIdxFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = iNumRefIdx;
          m_iRefIdxOfLC[REF_PIC_LIST_1][iNumRefIdx] = m_aiNumRefIdx[REF_PIC_LIST_C]++;
        }
      }
    }
  }
}

Void TComSlice::setRefPicList( TComList<TComPic*>& rcListPic )
{
  if (m_eSliceType == I_SLICE)
  {
    ::memset( m_apcRefPicList, 0, sizeof (m_apcRefPicList));
    ::memset( m_aiNumRefIdx,   0, sizeof ( m_aiNumRefIdx ));
    
    return;
  }
  
  m_aiNumRefIdx[0] = getNumRefIdx(REF_PIC_LIST_0);
  m_aiNumRefIdx[1] = getNumRefIdx(REF_PIC_LIST_1);

  TComPic*  pcRefPic;
  TComPic*  RefPicSetStCurr0[16];
  TComPic*  RefPicSetStCurr1[16];
  TComPic*  RefPicSetLtCurr[16];
  UInt NumPocStCurr0 = 0;
  UInt NumPocStCurr1 = 0;
  UInt NumPocLtCurr = 0;
  Int i;

  for(i=0; i < m_pcRPS->getNumberOfNegativePictures(); i++)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetRefPic(rcListPic, getPOC()+m_pcRPS->getDeltaPOC(i));
      pcRefPic->setIsLongTerm(0);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetStCurr0[NumPocStCurr0] = pcRefPic;
      NumPocStCurr0++;
    }
  }
  for(; i < m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures(); i++)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetRefPic(rcListPic, getPOC()+m_pcRPS->getDeltaPOC(i));
      pcRefPic->setIsLongTerm(0);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetStCurr1[NumPocStCurr1] = pcRefPic;
      NumPocStCurr1++;
    }
  }
  for(i = m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures()+m_pcRPS->getNumberOfLongtermPictures()-1; i > m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures()-1 ; i--)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetLongTermRefPic(rcListPic, m_pcRPS->getPOC(i));
      pcRefPic->setIsLongTerm(1);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetLtCurr[NumPocLtCurr] = pcRefPic;
      NumPocLtCurr++;
    }
  }

  // ref_pic_list_init
  UInt cIdx = 0;
  UInt num_ref_idx_l0_active_minus1 = m_aiNumRefIdx[0] - 1;
  UInt num_ref_idx_l1_active_minus1 = m_aiNumRefIdx[1] - 1;
  while( cIdx <= num_ref_idx_l0_active_minus1 )
  {
    for( i=0; i < NumPocStCurr0 && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetStCurr0[ i ];
    for( i=0;  i < NumPocStCurr1 && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetStCurr1[ i ]; 
    for( i=0; i < NumPocLtCurr && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetLtCurr[ i ];
  }

  if ( m_eSliceType == P_SLICE )
    {
      m_aiNumRefIdx[1] = 0;
      ::memset( m_apcRefPicList[1], 0, sizeof(m_apcRefPicList[1]));
    }
  else
  {
    // ref_pic_list_init
    cIdx = 0;
    while( cIdx <= num_ref_idx_l1_active_minus1 )
    {
      for( i=0; i < NumPocStCurr1 && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetStCurr1[ i ];
      for( i=0;  i < NumPocStCurr0 && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetStCurr0[ i ];
      for( i=0; i < NumPocLtCurr && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetLtCurr[ i ];
    }
  }

  //ref_pic_list_modification_l0
  if(m_RefPicListModification.getRefPicListModificationFlagL0())
  {
    for( i = 0; i < m_RefPicListModification.getNumberOfRefPicListModificationsL0(); i++)
    {
      for( cIdx = num_ref_idx_l0_active_minus1 + 1; cIdx > i; cIdx-- )
        m_apcRefPicList[0][ cIdx ] = m_apcRefPicList[0][ cIdx - 1];
      if(m_RefPicListModification.getListIdcL0(i) == 0)
        m_apcRefPicList[0][ i ] =  RefPicSetStCurr0[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      else if(m_RefPicListModification.getListIdcL0(i) == 1)
        m_apcRefPicList[0][ i ] =  RefPicSetStCurr1[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      else if(m_RefPicListModification.getListIdcL0(i) == 2)
        m_apcRefPicList[0][ i ] =  RefPicSetLtCurr[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      UInt nIdx = i+1;
      for( cIdx = i+1; cIdx <= num_ref_idx_l0_active_minus1 + 1; cIdx++ )
      {
        if( m_apcRefPicList[0][ cIdx ] != m_apcRefPicList[0][ i ] )
          m_apcRefPicList[0][ nIdx++ ] = m_apcRefPicList[0][ cIdx ];
      }
    }
  }
  
  //ref_pic_list_modification_l1
  if(m_eSliceType == B_SLICE && m_RefPicListModification.getRefPicListModificationFlagL1())
  {
    for( i = 0; i < m_RefPicListModification.getNumberOfRefPicListModificationsL1(); i++)
    {
      for( cIdx = num_ref_idx_l1_active_minus1 + 1; cIdx > i; cIdx-- )
        m_apcRefPicList[1][ cIdx ] = m_apcRefPicList[1][ cIdx - 1];
      if(m_RefPicListModification.getListIdcL1(i) == 0)
        m_apcRefPicList[1][ i ] =  RefPicSetStCurr1[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      else if(m_RefPicListModification.getListIdcL1(i) == 1)
        m_apcRefPicList[1][ i ] =  RefPicSetStCurr0[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      else if(m_RefPicListModification.getListIdcL1(i) == 2)
        m_apcRefPicList[1][ i ] =  RefPicSetLtCurr[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      UInt nIdx = i+1;
      for( cIdx = i+1; cIdx <= num_ref_idx_l1_active_minus1 + 1; cIdx++ )
      {
        if( m_apcRefPicList[1][ cIdx ] != m_apcRefPicList[1][ i ] )
          m_apcRefPicList[1][ nIdx++ ] = m_apcRefPicList[1][ cIdx ];
      }
    }
  }
}

Void TComSlice::initEqualRef()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    for (Int iRefIdx1 = 0; iRefIdx1 < MAX_NUM_REF; iRefIdx1++)
    {
      for (Int iRefIdx2 = iRefIdx1; iRefIdx2 < MAX_NUM_REF; iRefIdx2++)
      {
        m_abEqualRef[iDir][iRefIdx1][iRefIdx2] = m_abEqualRef[iDir][iRefIdx2][iRefIdx1] = (iRefIdx1 == iRefIdx2? true : false);
      }
    }
  }
}

Void TComSlice::checkCRA(TComReferencePictureSet *pReferencePictureSet, UInt& pocCRA, TComList<TComPic*>& rcListPic)
{
  for(Int i = 0; i < pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures(); i++)
  {
    if(pocCRA < MAX_UINT && getPOC() > pocCRA)
    {
      assert(getPOC()+pReferencePictureSet->getDeltaPOC(i) >= pocCRA);
    }
  }
  for(Int i = pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures(); i < pReferencePictureSet->getNumberOfPictures(); i++)
  {
    if(pocCRA < MAX_UINT && getPOC() > pocCRA)
    {
      assert(pReferencePictureSet->getPOC(i) >= pocCRA);
    }
  }
  if (getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR) // CDR picture found
  {
    pocCRA = getPOC();
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

Void TComSlice::copySliceInfo(TComSlice *pSrc)
{
  assert( pSrc != NULL );

  Int i, j, k;

  m_iPOC                 = pSrc->m_iPOC;
  m_eNalUnitType         = pSrc->m_eNalUnitType;
  m_eSliceType           = pSrc->m_eSliceType;
  m_iSliceQp             = pSrc->m_iSliceQp;
  
  for (i = 0; i < 3; i++)
  {
    m_aiNumRefIdx[i]     = pSrc->m_aiNumRefIdx[i];
  }

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF_LC; j++)
    {
       m_iRefIdxOfLC[i][j]  = pSrc->m_iRefIdxOfLC[i][j];
    }
  }
  for (i = 0; i < MAX_NUM_REF_LC; i++)
  {
    m_eListIdFromIdxOfLC[i] = pSrc->m_eListIdFromIdxOfLC[i];
    m_iRefIdxFromIdxOfLC[i] = pSrc->m_iRefIdxFromIdxOfLC[i];
    m_iRefIdxOfL1FromRefIdxOfL0[i] = pSrc->m_iRefIdxOfL1FromRefIdxOfL0[i];
    m_iRefIdxOfL0FromRefIdxOfL1[i] = pSrc->m_iRefIdxOfL0FromRefIdxOfL1[i];
  }
  m_bRefPicListModificationFlagLC = pSrc->m_bRefPicListModificationFlagLC;
  m_bRefPicListCombinationFlag    = pSrc->m_bRefPicListCombinationFlag;
  m_iSliceQpDelta        = pSrc->m_iSliceQpDelta;
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF; j++)
    {
      m_apcRefPicList[i][j]  = pSrc->m_apcRefPicList[i][j];
      m_aiRefPOCList[i][j]   = pSrc->m_aiRefPOCList[i][j];
    }
  }  
  m_iDepth               = pSrc->m_iDepth;

  // referenced slice
  m_bRefenced            = pSrc->m_bRefenced;

  // access channel
  m_pcSPS                = pSrc->m_pcSPS;
  m_pcPPS                = pSrc->m_pcPPS;
  m_pcRPS                = pSrc->m_pcRPS;
  m_iLastIDR             = pSrc->m_iLastIDR;

  m_pcPic                = pSrc->m_pcPic;
  m_iAPSId               = pSrc->m_iAPSId;

  m_uiColDir             = pSrc->m_uiColDir;
  m_dLambda              = pSrc->m_dLambda;
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF; j++)
    {
      for (k =0; k < MAX_NUM_REF; k++)
      {
        m_abEqualRef[i][j][k] = pSrc->m_abEqualRef[i][j][k];
      }
    }
  }

  m_bNoBackPredFlag      = pSrc->m_bNoBackPredFlag;
  m_bRefIdxCombineCoding = pSrc->m_bRefIdxCombineCoding;

  m_uiSliceCurEndCUAddr           = pSrc->m_uiSliceCurEndCUAddr;
  m_bNextSlice                    = pSrc->m_bNextSlice;
}

int TComSlice::m_iPrevPOC = 0;

/** Function for applying picture marking based on the Reference Picture Set in pReferencePictureSet.
*/
Void TComSlice::applyReferencePictureSet( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet)
{
  TComPic* rpcPic;
  Int i, isReference;

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
    for(i=0;i<pReferencePictureSet->getNumberOfPositivePictures()+pReferencePictureSet->getNumberOfNegativePictures();i++)
    {
      if(!rpcPic->getIsLongTerm() && rpcPic->getPicSym()->getSlice()->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i))
      {
        isReference = 1;
        rpcPic->setUsedByCurr(pReferencePictureSet->getUsed(i));
        rpcPic->setIsLongTerm(0);
      }
    }
    for(;i<pReferencePictureSet->getNumberOfPictures();i++)
    {
      if(rpcPic->getIsLongTerm() && (rpcPic->getPicSym()->getSlice()->getPOC()%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC())) == pReferencePictureSet->getPOC(i)%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC()))
      {
        isReference = 1;
        rpcPic->setUsedByCurr(pReferencePictureSet->getUsed(i));
      }
    }
    // mark the picture as "unused for reference" if it is not in
    // the Reference Picture Set
    if(rpcPic->getPicSym()->getSlice()->getPOC() != this->getPOC() && isReference == 0)    
    {            
      rpcPic->getSlice()->setReferenced( false );   
      rpcPic->setIsLongTerm(0);
    }
  }  
}

/** Function for applying picture marking based on the Reference Picture Set in pReferencePictureSet.
*/
Int TComSlice::checkThatAllRefPicsAreAvailable( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet, Bool outputFlag)
{
  TComPic* rpcPic;
  Int i, isAvailable, j;
  Int atLeastOneLost = 0;
  Int atLeastOneRemoved = 0;
  Int iPocLost = 0;

  // loop through all long-term pictures in the Reference Picture Set
  // to see if the picture should be kept as reference picture
  for(i=pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures();i<pReferencePictureSet->getNumberOfPictures();i++)
  {
    j = 0;
    isAvailable = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);
      if(rpcPic->getIsLongTerm() && (rpcPic->getPicSym()->getSlice()->getPOC()%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC())) == pReferencePictureSet->getPOC(i)%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC()) && rpcPic->getSlice()->isReferenced())
      {
        isAvailable = 1;
      }
    }
    // if there was no such long-term check the short terms
    if(!isAvailable)
    {
      iterPic = rcListPic.begin();
      while ( iterPic != rcListPic.end())
      {
        j++;
        rpcPic = *(iterPic++);

        if((rpcPic->getPicSym()->getSlice()->getPOC()%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC())) == (this->getPOC() + pReferencePictureSet->getDeltaPOC(i))%(1<<rpcPic->getPicSym()->getSlice()->getSPS()->getBitsForPOC()) && rpcPic->getSlice()->isReferenced())
        {
          isAvailable = 1;
          rpcPic->setIsLongTerm(1);
          break;
        }
      }
    }
    // report that a picture is lost if it is in the Reference Picture Set
    // but not available as reference picture
    if(isAvailable == 0)    
    {            
      if(!pReferencePictureSet->getUsed(i) )
      {
        if(outputFlag)
          printf("\nLong-term reference picture with POC = %3d seems to have been removed or not correctly decoded.", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneRemoved = 1;
      }
      else
      {
        if(outputFlag)
          printf("\nLong-term reference picture with POC = %3d is lost or not correctly decoded!", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneLost = 1;
        iPocLost=this->getPOC() + pReferencePictureSet->getDeltaPOC(i);
      }
    }
  }  
  // loop through all short-term pictures in the Reference Picture Set
  // to see if the picture should be kept as reference picture
  for(i=0;i<pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures();i++)
  {
    j = 0;
    isAvailable = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);

      if(!rpcPic->getIsLongTerm() && rpcPic->getPicSym()->getSlice()->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i) && rpcPic->getSlice()->isReferenced())
      {
        isAvailable = 1;
      }
    }
    // report that a picture is lost if it is in the Reference Picture Set
    // but not available as reference picture
    if(isAvailable == 0)    
    {            
      if(!pReferencePictureSet->getUsed(i) )
      {
        if(outputFlag)
          printf("\nShort-term reference picture with POC = %3d seems to have been removed or not correctly decoded.", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneRemoved = 1;
      }
      else
      {
        if(outputFlag)
          printf("\nShort-term reference picture with POC = %3d is lost or not correctly decoded!", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneLost = 1;
        iPocLost=this->getPOC() + pReferencePictureSet->getDeltaPOC(i);
      }
    }
  }    
  if(atLeastOneLost)
  {
    return iPocLost+1;
  }
  if(atLeastOneRemoved)
  {
    return -2;
  }
  else
    return 0;
}

/** Function for constructing an explicit Reference Picture Set out of the available pictures in a referenced Reference Picture Set
*/
Void TComSlice::createExplicitReferencePictureSetFromReference( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet)
{
  TComPic* rpcPic;
  Int i, j;
  Int k = 0;
  Int nrOfNegativePictures = 0;
  Int nrOfPositivePictures = 0;
  TComReferencePictureSet* pcRPS = this->getLocalRPS();

  // loop through all pictures in the Reference Picture Set
  for(i=0;i<pReferencePictureSet->getNumberOfPictures();i++)
  {
    j = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);

      if(rpcPic->getPicSym()->getSlice()->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i) && rpcPic->getSlice()->isReferenced())
      {
        // This picture exists as a reference picture
        // and should be added to the explicit Reference Picture Set
        pcRPS->setDeltaPOC(k, pReferencePictureSet->getDeltaPOC(i));
        pcRPS->setUsed(k, pReferencePictureSet->getUsed(i));
        if(pcRPS->getDeltaPOC(k) < 0)
          nrOfNegativePictures++;
        else
          nrOfPositivePictures++;
        k++;
      }
    }
  }
  pcRPS->setNumberOfNegativePictures(nrOfNegativePictures);
  pcRPS->setNumberOfPositivePictures(nrOfPositivePictures);
  pcRPS->setNumberOfPictures(nrOfNegativePictures+nrOfPositivePictures);

  this->setRPS(pcRPS);
  this->setRPSidx(-1);
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
, m_bLongTermRefsPresent        (false)
, m_uiBitsForLongTermRefs       (0)
{
}

TComPPS::~TComPPS()
{
}

TComReferencePictureSet::TComReferencePictureSet()
: m_uiNumberOfPictures (0)
, m_uiNumberOfNegativePictures (0)
, m_uiNumberOfPositivePictures (0)
, m_uiNumberOfLongtermPictures (0)
{
  ::memset( m_piDeltaPOC, 0, sizeof(m_piDeltaPOC) );
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

Void TComReferencePictureSet::setDeltaPOC(UInt uiBufferNum, Int iDeltaPOC)
{
   m_piDeltaPOC[uiBufferNum] = iDeltaPOC;
}

Void TComReferencePictureSet::setNumberOfPictures(UInt NumberOfPictures)
{
   m_uiNumberOfPictures = NumberOfPictures;
}

UInt TComReferencePictureSet::getUsed(UInt uiBufferNum)
{
   return (UInt)m_pbUsed[uiBufferNum];
}

Int TComReferencePictureSet::getDeltaPOC(UInt uiBufferNum)
{
   return m_piDeltaPOC[uiBufferNum];
}

UInt TComReferencePictureSet::getNumberOfPictures()
{
   return m_uiNumberOfPictures;
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
  for(Int j=0; j < getNumberOfPictures(); j++)
  {
    printf("%d%s ", getDeltaPOC(j), (getUsed(j)==1)?"*":"");
  } 
  printf("}\n");
}

TComRPS::TComRPS()
{
}

TComRPS::~TComRPS()
{
}

Void TComRPS::create( UInt uiNumberOfReferencePictureSets)
{
  m_uiNumberOfReferencePictureSets = uiNumberOfReferencePictureSets;
  m_pReferencePictureSet = new TComReferencePictureSet[uiNumberOfReferencePictureSets];
}

Void TComRPS::destroy()
{
  delete [] m_pReferencePictureSet;     
  m_uiNumberOfReferencePictureSets = 0;
  m_pReferencePictureSet = NULL;
}



TComReferencePictureSet* TComRPS::getReferencePictureSet(UInt uiReferencePictureSetNum)
{
   return &m_pReferencePictureSet[uiReferencePictureSetNum];
}

UInt TComRPS::getNumberOfReferencePictureSets()
{
   return m_uiNumberOfReferencePictureSets;
}

Void TComRPS::setNumberOfReferencePictureSets(UInt uiNumberOfReferencePictureSets)
{
   m_uiNumberOfReferencePictureSets = uiNumberOfReferencePictureSets;
}

TComRefPicListModification::TComRefPicListModification()
: m_bRefPicListModificationFlagL0 (false)
, m_bRefPicListModificationFlagL1 (false)
, m_uiNumberOfRefPicListModificationsL0 (0)
, m_uiNumberOfRefPicListModificationsL1 (0)
{
  ::memset( m_ListIdcL0, 0, sizeof(m_ListIdcL0) );
  ::memset( m_RefPicSetIdxL0, 0, sizeof(m_RefPicSetIdxL0) );
  ::memset( m_ListIdcL1, 0, sizeof(m_ListIdcL1) );
  ::memset( m_RefPicSetIdxL1, 0, sizeof(m_RefPicSetIdxL1) );
}

TComRefPicListModification::~TComRefPicListModification()
{
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
