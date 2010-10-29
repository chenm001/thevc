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

/** \file     TComSlice.cpp
    \brief    slice header and SPS class
*/

#include "CommonDef.h"
#include "TComSlice.h"
#include "TComPic.h"

TComSlice::TComSlice()
{
  m_iPOC                = 0;
  m_eSliceType          = I_SLICE;
  m_iSliceQp            = 0;
  m_iSymbolMode         = 1;
  m_bMultiCodeword      = false;
  m_uiMaxPIPEDelay      = 0;
  m_uiBalancedCPUs      = 8;
  m_aiNumRefIdx[0]      = 0;
  m_aiNumRefIdx[1]      = 0;
  m_bLoopFilterDisable  = false;

  m_bDRBFlag            = true;
  m_eERBIndex           = ERB_NONE;

  m_iSliceQpDelta       = 0;

  m_iDepth              = 0;

  m_pcPic               = NULL;
  m_bRefenced           = false;
#ifdef ROUNDING_CONTROL_BIPRED
  m_bRounding			= false;
#endif
  m_uiColDir = 0;

  m_aiWPmode[0]    = 0;
  m_aiWPmode[1]    = 0;
  m_iRFmode    = 0;

  m_iLumaLogWeightDenom     = 5;
  m_iChromaLogWeightDenom   = 5;
  m_iWPLumaRound            = 1 << (m_iLumaLogWeightDenom-1);
  m_iWPChromaRound          = 1 << (m_iChromaLogWeightDenom-1);

  m_auiAddRefCnt[0]  = 0;
  m_auiAddRefCnt[1]  = 0;

#ifdef EDGE_BASED_PREDICTION
    m_bEdgePredictionEnable = false;
#endif //EDGE_BASED_PREDICTION
  initEqualRef();
#ifdef DCM_PBIC
  xCreateZTrees();
#endif
#if MS_NO_BACK_PRED_IN_B0
  m_bNoBackPredFlag = false;
#endif
}

TComSlice::~TComSlice()
{
#ifdef DCM_PBIC
  xDeleteZTrees();
#endif
}

#ifdef DCM_PBIC
Void TComSlice::xCreateZTrees()
{
  Int* piPattern               = NULL;
  Int  aiPattern_MVDICDUNI[12] = {4, 1, 1, 0, 0, 1, 0, 0, 0, 1, 2, 3};
  Int  aiPattern_MVDICDBI [21] = {7, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 2, 1, 3, 4, 5, 6};
  Int  aiPattern_MVDUNI   [ 6] = {2, 1, 0, 0, 0, 1};
  Int  aiPattern_MVDBI    [12] = {4, 1, 1, 0, 0, 1, 0, 0, 0, 2, 1, 3};

  for (UInt uiZTreeIdx = 0; uiZTreeIdx < MAX_NUM_ZTREE; uiZTreeIdx++)
  {
    switch (uiZTreeIdx)
    {
    case IDX_ZTREE_MVDICDUNI:
      piPattern = aiPattern_MVDICDUNI;
      m_apcZTree[uiZTreeIdx] = new TComZeroTree(piPattern);
      break;
    case IDX_ZTREE_MVDICDBI:
      piPattern = aiPattern_MVDICDBI;
      m_apcZTree[uiZTreeIdx] = new TComZeroTree(piPattern);
      break;
    case IDX_ZTREE_MVDUNI:
      piPattern = aiPattern_MVDUNI;
      m_apcZTree[uiZTreeIdx] = new TComZeroTree(piPattern);
      break;
    case IDX_ZTREE_MVDBI:
      piPattern = aiPattern_MVDBI;
      m_apcZTree[uiZTreeIdx] = new TComZeroTree(piPattern);
      break;
    default:
      m_apcZTree[uiZTreeIdx] = NULL;
      break;
    }
  }
}

Void TComSlice::xDeleteZTrees()
{
  for (UInt uiZTreeIdx = 0; uiZTreeIdx < MAX_NUM_ZTREE; uiZTreeIdx++)
  {
    if ( m_apcZTree[uiZTreeIdx] != NULL )
      delete m_apcZTree[uiZTreeIdx];
  }
}
#endif

Void TComSlice::initSlice()
{
  m_aiNumRefIdx[0]      = 0;
  m_aiNumRefIdx[1]      = 0;

  m_bDRBFlag            = true;
  m_eERBIndex           = ERB_NONE;

#if HHI_INTERP_FILTER
  m_iInterpFilterType   = IPF_SAMSUNG_DIF_DEFAULT;
#endif

  m_uiColDir = 0;

  m_aiWPmode[0]    = 0;
  m_aiWPmode[1]    = 0;
  m_iRFmode    = 0;

  m_iLumaLogWeightDenom     = 5;
  m_iChromaLogWeightDenom   = 5;
  m_iWPLumaRound            = 1 << (m_iLumaLogWeightDenom-1);
  m_iWPChromaRound          = 1 << (m_iChromaLogWeightDenom-1);

  m_auiAddRefCnt[0]  = 0;
  m_auiAddRefCnt[1]  = 0;
#ifdef QC_SIFO_PRED
	m_bUseSIFO_Pred    = true;
#endif
  initEqualRef();
#if MS_NO_BACK_PRED_IN_B0
  m_bNoBackPredFlag = false;
#endif
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
                                Bool                bDRBFlag,
                                ERBIndex            eERBIndex,
                                UInt                uiPOCCurr,
                                RefPicList          eRefPicList,
                                UInt                uiNthRefPic)
{
  //  find current position
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();
  TComPic*                      pcRefPic   = NULL;

  TComPic*                      pcPic = *(iterPic);
  while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
  {
    iterPic++;
    pcPic = *(iterPic);
  }
  assert (pcPic->getPOC() == (Int)uiPOCCurr);

  //  find n-th reference picture with bDRBFlag and eERBIndex
  UInt  uiCount = 0;

  if( eRefPicList == REF_PIC_LIST_0 )
  {
    while(1)
    {
      if (iterPic == rcListPic.begin())
        break;

      iterPic--;
      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
          ( bDRBFlag  != pcPic->getSlice()->getDRBFlag()  ) ||
          ( eERBIndex != pcPic->getSlice()->getERBIndex() ) )
        continue;

      if( !pcPic->getSlice()->isReferenced() )
        continue;

      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }

    if ( !m_pcSPS->getUseLDC() )
    {

    iterPic = rcListPic.begin();
    pcPic = *(iterPic);
    while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
    {
      iterPic++;
      pcPic = *(iterPic);
    }
    assert (pcPic->getPOC() == (Int)uiPOCCurr);

    while(1)
    {
      iterPic++;
      if (iterPic == rcListPic.end())
        break;

      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
          ( bDRBFlag  != pcPic->getSlice()->getDRBFlag()  ) ||
          ( eERBIndex != pcPic->getSlice()->getERBIndex() ) )
        continue;

      if( !pcPic->getSlice()->isReferenced() )
        continue;

      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }
  }
  }
  else
  {
    while(1)
    {
      iterPic++;
      if (iterPic == rcListPic.end())
        break;

      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
          ( bDRBFlag  != pcPic->getSlice()->getDRBFlag()  ) ||
          ( eERBIndex != pcPic->getSlice()->getERBIndex() ) )
        continue;

      if( !pcPic->getSlice()->isReferenced() )
        continue;

      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }

    iterPic = rcListPic.begin();
    pcPic = *(iterPic);
    while ( (pcPic->getPOC() != (Int)uiPOCCurr) && (iterPic != rcListPic.end()) )
    {
      iterPic++;
      pcPic = *(iterPic);
    }
    assert (pcPic->getPOC() == (Int)uiPOCCurr);

    while(1)
    {
      if (iterPic == rcListPic.begin())
        break;

      iterPic--;
      pcPic = *(iterPic);
      if( ( !pcPic->getReconMark()                        ) ||
          ( bDRBFlag  != pcPic->getSlice()->getDRBFlag()  ) ||
          ( eERBIndex != pcPic->getSlice()->getERBIndex() ) )
        continue;

      if( !pcPic->getSlice()->isReferenced() )
        continue;

      uiCount++;
      if (uiCount == uiNthRefPic)
      {
        pcRefPic = pcPic;
        return  pcRefPic;
      }
    }
  }

  return  pcRefPic;
}

Void TComSlice::setRefPOCList       ()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  for (Int iNumRefIdx = 0; iNumRefIdx < m_aiNumRefIdx[iDir]; iNumRefIdx++)
  {
    m_aiRefPOCList[iDir][iNumRefIdx] = m_apcRefPicList[iDir][iNumRefIdx]->getPOC();
  }
}

Void TComSlice::setRefPicList       ( TComList<TComPic*>& rcListPic )
{
  if (m_eSliceType == I_SLICE)
  {
    ::memset( m_apcRefPicList, 0, sizeof (m_apcRefPicList));
    ::memset( m_aiNumRefIdx,   0, sizeof ( m_aiNumRefIdx ));

    return;
  }

  m_aiNumRefIdx[0] = Min ( m_aiNumRefIdx[0], (Int)(rcListPic.size())-1 );
  m_aiNumRefIdx[1] = Min ( m_aiNumRefIdx[1], (Int)(rcListPic.size())-1 );

  sortPicList(rcListPic);

  TComPic*  pcRefPic;
  for (Int n = 0; n < 2; n++)
  {
    RefPicList  eRefPicList = (RefPicList)n;

    UInt  uiOrderDRB  = 1;
    UInt  uiOrderERB  = 1;
    Int   iRefIdx     = 0;
    UInt  uiActualListSize = 0;

    if ( m_eSliceType == P_SLICE && eRefPicList == REF_PIC_LIST_1 )
    {
      m_aiNumRefIdx[eRefPicList] = 0;
      ::memset( m_apcRefPicList[eRefPicList], 0, sizeof(m_apcRefPicList[eRefPicList]));
      break;
    }

    //  First DRB
    pcRefPic = xGetRefPic(rcListPic, true, ERB_NONE, m_iPOC, eRefPicList, uiOrderDRB);
    if (pcRefPic != NULL)
    {
      m_apcRefPicList[eRefPicList][iRefIdx] = pcRefPic;

#if HHI_INTERP_FILTER
      pcRefPic->getPicYuvRec()    ->extendPicBorder( getInterpFilterType() );
      pcRefPic->getPicYuvRecFilt()->extendPicBorder( getInterpFilterType() );
#else
      pcRefPic->getPicYuvRec()->extendPicBorder();
#endif

      iRefIdx++;
      uiOrderDRB++;
      uiActualListSize++;
    }

    if ( (Int)uiActualListSize >= m_aiNumRefIdx[eRefPicList] )
    {
      m_aiNumRefIdx[eRefPicList] = uiActualListSize;
      continue;
    }

    // Long term reference
    // Should be enabled to support long term refernce
    //*
    //  First ERB
    pcRefPic = xGetRefPic(rcListPic, false, ERB_LTR, m_iPOC, eRefPicList, uiOrderERB);
    if (pcRefPic != NULL)
    {
      Bool  bChangeDrbErb = false;
      if      (iRefIdx > 0 && eRefPicList == REF_PIC_LIST_0 && pcRefPic->getPOC() > m_apcRefPicList[eRefPicList][iRefIdx-1]->getPOC())
      {
        bChangeDrbErb = true;
      }
      else if (iRefIdx > 0 && eRefPicList == REF_PIC_LIST_1 && pcRefPic->getPOC() < m_apcRefPicList[eRefPicList][iRefIdx-1]->getPOC())
      {
        bChangeDrbErb = true;
      }

      if ( bChangeDrbErb)
      {
        m_apcRefPicList[eRefPicList][iRefIdx]   = m_apcRefPicList[eRefPicList][iRefIdx-1];
        m_apcRefPicList[eRefPicList][iRefIdx-1] = pcRefPic;
      }
      else
      {
        m_apcRefPicList[eRefPicList][iRefIdx] = pcRefPic;
      }

#if HHI_INTERP_FILTER
      pcRefPic->getPicYuvRec()    ->extendPicBorder( getInterpFilterType() );
      pcRefPic->getPicYuvRecFilt()->extendPicBorder( getInterpFilterType() );
#else
      pcRefPic->getPicYuvRec()->extendPicBorder();
#endif

      iRefIdx++;
      uiOrderERB++;
      uiActualListSize++;
    }
    //*/

    // Short term reference
    //  Residue DRB
    UInt  uiBreakCount = 17 - iRefIdx;
    while (1)
    {
      uiBreakCount--;
      if ( (Int)uiActualListSize >= m_aiNumRefIdx[eRefPicList] || uiBreakCount == 0 )
      {
        break;
      }

      pcRefPic = xGetRefPic(rcListPic, true, ERB_NONE, m_iPOC, eRefPicList, uiOrderDRB);
      if (pcRefPic != NULL)
      {
        uiOrderDRB++;

        // Not same level reference use
        /*
        Bool b = false;
        Int iRefPoc = pcRefPic->getPOC();
        Int iCnt = 1;
        while(1)
        {
          if( iRefPoc & iCnt )
          {
            b = true;
            break;
          }
          else if( m_iPOC & iCnt )
          {
            break;
          }
          iCnt<<=1;
        }
        if( b )
        {
          continue;
        }
        //*/

        // Key frames always referenced




        // If to support different reference indexing algorithm for multiple reference indexing in short term reference,
        // this code should be fixed.
        /*
        //if ( m_eSliceType == B_SLICE && (pcRefPic->getPOC() - m_iPOC)%2 == 0)
        TComPic*  pc2RefPic = m_apcRefPicList[eRefPicList][0]->getSlice()->getRefPic(eRefPicList, 0);
        if ( pc2RefPic != pcRefPic )
        {
          continue;
        }
        //*/
      }

      if (pcRefPic != NULL)
      {
        m_apcRefPicList[eRefPicList][iRefIdx] = pcRefPic;

#if HHI_INTERP_FILTER
        pcRefPic->getPicYuvRec()    ->extendPicBorder( getInterpFilterType() );
        pcRefPic->getPicYuvRecFilt()->extendPicBorder( getInterpFilterType() );
#else
        pcRefPic->getPicYuvRec()->extendPicBorder();
#endif
        iRefIdx++;
        uiActualListSize++;
      }
    }

    m_aiNumRefIdx[eRefPicList] = uiActualListSize;
  }
}

Void TComSlice::initEqualRef()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  for (Int iRefIdx1 = 0; iRefIdx1 < MAX_NUM_REF+GRF_MAX_NUM_EFF; iRefIdx1++)
  for (Int iRefIdx2 = iRefIdx1; iRefIdx2 < MAX_NUM_REF+GRF_MAX_NUM_EFF; iRefIdx2++)
  {
    m_abEqualRef[iDir][iRefIdx1][iRefIdx2] = m_abEqualRef[iDir][iRefIdx2][iRefIdx1] = (iRefIdx1 == iRefIdx2? true : false);
  }
}

Void TComSlice::setVirtRefBuffer(TComPic* pSrc[2][GRF_MAX_NUM_EFF])
{
  for (Int i=0; i<GRF_MAX_NUM_EFF; i++)
  for (Int j=0; j<2; j++)
  {
    m_apcVirtPic[j][i] = pSrc[j][i];
  }
}

Void TComSlice::linkVirtRefPic()
{
  Int numRefIdxN,numRefIdx, n, i;
  Int numDir = ((getSliceType()) == P_SLICE) ? 1 : 2;
  RefPicList eRefPicList;

  for ( n=0; n<numDir; n++ )
  {
    eRefPicList = (RefPicList)n;
    numRefIdxN  = getAddRefCnt(eRefPicList);
    numRefIdx   = getNumRefIdx(eRefPicList);

    for ( i=0; i<numRefIdxN; i++ )
    {
      setRefPic(m_apcVirtPic[n][i], eRefPicList, numRefIdx+i);

#if HHI_INTERP_FILTER
      m_apcVirtPic[n][i]->getPicYuvRec()    ->extendPicBorder( getInterpFilterType() );
      m_apcVirtPic[n][i]->getPicYuvRecFilt()->extendPicBorder( getInterpFilterType() );
#else
      m_apcVirtPic[n][i]->getPicYuvRec()->extendPicBorder();
#endif

      setEqualRef(eRefPicList, 0, numRefIdx+i, true);
    }
    setNumRefIdx(eRefPicList, numRefIdx+numRefIdxN);
  }
}

Void TComSlice::removeEffectMode(RefPicList e, EFF_MODE eEffMode)
{
  Int iPosIdx = 0, iTmpIdx;

  for (iTmpIdx = 0; iTmpIdx < m_auiAddRefCnt[e]; iTmpIdx++)
  {
    if(m_aeEffectMode[e][iTmpIdx] == eEffMode)
    {
      iPosIdx = iTmpIdx;
      break;
    }
  }

  if (iTmpIdx >= m_auiAddRefCnt[e]) return;

  m_auiAddRefCnt[e]--;

  for (iTmpIdx = iPosIdx; iTmpIdx < m_auiAddRefCnt[e]; iTmpIdx++)
  {
    m_aeEffectMode[e][iTmpIdx] = m_aeEffectMode[e][iTmpIdx+1];
    m_apcVirtPic[e][iTmpIdx] = m_apcVirtPic[e][iTmpIdx+1];
  }
}

Void TComSlice::initWPParam( RefPicList eRefPicList, EFF_MODE eEffMode, Int colorChannel)
{
  Int iDefaultWeight;

  if (colorChannel == 0)  // y
  {
    iDefaultWeight = 1 << m_iLumaLogWeightDenom;
  }
  else if (colorChannel == 1) // u
  {
    iDefaultWeight = 1 << m_iChromaLogWeightDenom;
  }
  else  // v
  {
    iDefaultWeight = 1 << m_iChromaLogWeightDenom;
  }

  Int iDefaultOffset = 0;

  setWPWeight(eRefPicList, eEffMode, colorChannel, iDefaultWeight);
  setWPOffset(eRefPicList, eEffMode, colorChannel, iDefaultOffset);
}

Void TComSlice::initWPAllParam( RefPicList eRefPicList, EFF_MODE eEffMode)
{
  for (Int colorChannel = 0; colorChannel < 3; colorChannel++)
  {
    initWPParam(eRefPicList, eEffMode, colorChannel);
  }
}

Bool TComSlice::isEqualWPParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2, Int colorChannel)
{
  if (eEffMode1 == eEffMode2) return true;

  Int iWeight1, iOffset1, iWeight2, iOffset2;

  if (colorChannel == 0)  // y
  {
    iWeight1 = iWeight2 = 1 << m_iLumaLogWeightDenom;
  }
  else if (colorChannel == 1) // u
  {
    iWeight1 = iWeight2 = 1 << m_iChromaLogWeightDenom;
  }
  else  // v
  {
    iWeight1 = iWeight2 = 1 << m_iChromaLogWeightDenom;
  }
  iOffset1 = iOffset2 = 0;

  if (eEffMode1 != EFF_NONE)
  {
    iWeight1 = getWPWeight(e, eEffMode1, colorChannel);
    iOffset1 = getWPOffset(e, eEffMode1, colorChannel);
  }

  if (eEffMode2 != EFF_NONE)
  {
    iWeight2 = getWPWeight(e, eEffMode2, colorChannel);
    iOffset2 = getWPOffset(e, eEffMode2, colorChannel);
  }

  return ( (iWeight1 == iWeight2) && (iOffset1 == iOffset2) );
}

Bool TComSlice::isEqualWPAllParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2)
{
  if (eEffMode1 == eEffMode2) return true;

  for (Int colorChannel = 0; colorChannel < 3; colorChannel++)
  {
    if (!isEqualWPParam(e, eEffMode1, eEffMode2, colorChannel)) return false;
  }

  return true;
}

Void TComSlice::generateWPSlice( RefPicList e, EFF_MODE eEffMode, UInt uiInsertIdx)
{
  assert( eEffMode >= EFF_WP_SO && eEffMode <= EFF_WP_O );

  Int x, y, iWidth, iHeight, iStride, iWeight, iOffset;
  TComPicYuv* pcPicYuvRef;
  Pel *pDst, *pSrc;

  pcPicYuvRef = getRefPic(e, 0)->getPicYuvRec();

  m_apcVirtPic[e][uiInsertIdx]->getSlice()->setPOC( getRefPic(e, 0)->getPOC() );

  // Luma
  iWidth  = pcPicYuvRef->getWidth();
  iHeight = pcPicYuvRef->getHeight();
  iStride = pcPicYuvRef->getStride();
  pSrc    = pcPicYuvRef->getLumaAddr();
  pDst    = m_apcVirtPic[e][uiInsertIdx]->getPicYuvRec()->getLumaAddr();

  iWeight = getWPWeight(e, eEffMode, 0);
  iOffset = getWPOffset(e, eEffMode, 0);


  for (y=0; y<iHeight; y++) {
    for (x=0; x<iWidth; x++) {
      pDst[x] = Clip( ((iWeight*pSrc[x] + m_iWPLumaRound) >> m_iLumaLogWeightDenom) + iOffset );
    }
    pDst += iStride;
    pSrc += iStride;
  }

  // Chroma
  iWidth  >>= 1;
  iHeight >>= 1;
  iStride >>= 1;

  iWeight = getWPWeight(e, eEffMode, 1);
  iOffset = getWPOffset(e, eEffMode, 1);

  pSrc    = pcPicYuvRef->getCbAddr();
  pDst    = m_apcVirtPic[e][uiInsertIdx]->getPicYuvRec()->getCbAddr();

  for (y=0; y<iHeight; y++) {
    for (x=0; x<iWidth; x++) {
      pDst[x] = Clip( ((iWeight*pSrc[x] + m_iWPChromaRound) >> m_iChromaLogWeightDenom) + iOffset );

    }
    pDst += iStride;
    pSrc += iStride;
  }

  iWeight = getWPWeight(e, eEffMode, 2);
  iOffset = getWPOffset(e, eEffMode, 2);

  pSrc    = pcPicYuvRef->getCrAddr();
  pDst    = m_apcVirtPic[e][uiInsertIdx]->getPicYuvRec()->getCrAddr();

  for (y=0; y<iHeight; y++) {
    for (x=0; x<iWidth; x++) {
      pDst[x] = Clip( ((iWeight*pSrc[x] + m_iWPChromaRound) >> m_iChromaLogWeightDenom) + iOffset );
    }
    pDst += iStride;
    pSrc += iStride;
  }
}

// ------------------------------------------------------------------------------------------------
// Sequence parameter set (SPS)
// ------------------------------------------------------------------------------------------------

TComSPS::TComSPS()
{
  // Structure
  m_uiWidth       = 352;
  m_uiHeight      = 288;
  m_uiMaxCUWidth  = 32;
  m_uiMaxCUHeight = 32;
  m_uiMaxCUDepth  = 3;
  m_uiMinTrDepth  = 0;
  m_uiMaxTrDepth  = 1;
  m_uiMaxTrSize   = 32;

  // Tool list
  m_bUseALF       = false;
  m_bUseDQP       = false;

  m_bUseWPG      = false;
  m_bUseWPO      = false;

  m_bUseROT				= false; // BB:
#if HHI_AIS
  m_bUseAIS      = false; // BB:
#endif
#if HHI_MRG
  m_bUseMRG      = false; // SOPH:
#endif
#if HHI_IMVP
  m_bUseIMP      = false; // SOPH:
#endif

#if HHI_ALF
  m_bALFSeparateQt = true; //MS
  m_bALFSymmetry   = false;//MS
  m_iALFMinLength  = 3;    //MS
  m_iALFMaxLength  = 9;    //MS
#endif

#ifdef DCM_PBIC
  m_bUseIC       = false;
#endif

  // AMVP parameter
  ::memset( m_aeAMVPMode, 0, sizeof( m_aeAMVPMode ) );
}

TComSPS::~TComSPS()
{
}

TComPPS::TComPPS()
{
}

TComPPS::~TComPPS()
{
}

