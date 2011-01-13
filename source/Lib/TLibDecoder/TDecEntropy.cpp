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

/** \file     TDecEntropy.cpp
    \brief    entropy decoder class
*/

#include "TDecEntropy.h"

Void TDecEntropy::setEntropyDecoder         ( TDecEntropyIf* p )
{
  m_pcEntropyDecoderIf = p;
}

#include "../TLibCommon/TComAdaptiveLoopFilter.h"

Void TDecEntropy::decodeAux(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int sqrFiltLengthTab[3] = {SQR_FILT_LENGTH_9SYM, SQR_FILT_LENGTH_7SYM, SQR_FILT_LENGTH_5SYM};
  Int FiltTab[3] = {9, 7, 5};
  
  pAlfParam->filters_per_group = 0;
  
  memset (pAlfParam->filterPattern, 0 , sizeof(Int)*NO_VAR_BINS);
#if ENABLE_FORCECOEFF0
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  if (!uiSymbol) pAlfParam->filtNo = -1;
  else pAlfParam->filtNo = uiSymbol; //nonZeroCoeffs
#else
  pAlfParam->filtNo = 1; //nonZeroCoeffs
#endif
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  Int TabIdx = uiSymbol;
  pAlfParam->realfiltNo = 2-TabIdx;
  pAlfParam->tap = FiltTab[pAlfParam->realfiltNo];
  pAlfParam->num_coeff = sqrFiltLengthTab[pAlfParam->realfiltNo];
  
  if (pAlfParam->filtNo>=0)
  {
    if(pAlfParam->realfiltNo >= 0)
    {
      // filters_per_fr
      m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
      pAlfParam->noFilters = uiSymbol + 1;
      pAlfParam->filters_per_group = pAlfParam->noFilters; 

      if(pAlfParam->noFilters == 2)
      {
        m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
        pAlfParam->startSecondFilter = uiSymbol;
        pAlfParam->filterPattern [uiSymbol] = 1;
      }
      else if (pAlfParam->noFilters > 2)
      {
        pAlfParam->filters_per_group = 1;
        for (int i=1; i<NO_VAR_BINS; i++) 
        {
          m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
          pAlfParam->filterPattern[i] = uiSymbol;
          pAlfParam->filters_per_group += uiSymbol;
        }
      }
    }
  }
  else
  {
    memset (pAlfParam->filterPattern, 0, NO_VAR_BINS*sizeof(Int));
  }
  // Reconstruct varIndTab[]
  memset(pAlfParam->varIndTab, 0, NO_VAR_BINS * sizeof(int));
  if(pAlfParam->filtNo>=0)
  {
    for(Int i = 1; i < NO_VAR_BINS; ++i)
    {
      if(pAlfParam->filterPattern[i])
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1] + 1;
      else
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1];
    }
  }
}

Void TDecEntropy::readFilterCodingParams(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  int ind, scanPos;
  int golombIndexBit;
  int kMin;
  int maxScanVal;
  int *pDepthInt;
  int fl;
  
  // Determine fl
  if(pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if(pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;
  
  // Determine maxScanVal
  maxScanVal = 0;
  pDepthInt = pDepthIntTab[fl - 2];
  for(ind = 0; ind < pAlfParam->num_coeff; ind++)
    maxScanVal = max(maxScanVal, pDepthInt[ind]);
  
  // Golomb parameters
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->minKStart = 1 + uiSymbol;
  
  kMin = pAlfParam->minKStart;
  for(scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    golombIndexBit = uiSymbol;
    if(golombIndexBit)
      pAlfParam->kMinTab[scanPos] = kMin + 1;
    else
      pAlfParam->kMinTab[scanPos] = kMin;
    kMin = pAlfParam->kMinTab[scanPos];
  }
}

Int TDecEntropy::golombDecode(Int k)
{
  UInt uiSymbol;
  Int q = -1;
  Int nr = 0;
  Int m = (Int)pow(2.0, k);
  Int a;
  
  uiSymbol = 1;
  while (uiSymbol)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    q++;
  }
  for(a = 0; a < k; ++a)          // read out the sequential log2(M) bits
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    if(uiSymbol)
      nr += 1 << a;
  }
  nr += q * m;                    // add the bits and the multiple of M
  if(nr != 0)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    nr = (uiSymbol)? nr: -nr;
  }
  return nr;
}



Void TDecEntropy::readFilterCoeffs(ALFParam* pAlfParam)
{
  int ind, scanPos, i;
  int *pDepthInt;
  int fl;
  
  if(pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if(pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;
  
  pDepthInt = pDepthIntTab[fl - 2];
  
  for(ind = 0; ind < pAlfParam->filters_per_group_diff; ++ind)
  {
    for(i = 0; i < pAlfParam->num_coeff; i++)
    {
      scanPos = pDepthInt[i] - 1;
      pAlfParam->coeffmulti[ind][i] = golombDecode(pAlfParam->kMinTab[scanPos]);
    }
  }
  
}
Void TDecEntropy::decodeFilterCoeff (ALFParam* pAlfParam)
{
  readFilterCodingParams (pAlfParam);
  readFilterCoeffs (pAlfParam);
}



Void TDecEntropy::decodeFilt(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  
  if (pAlfParam->filtNo >= 0)
  {
    pAlfParam->filters_per_group_diff = pAlfParam->filters_per_group;
    if (pAlfParam->filters_per_group > 1)
    {
#if ENABLE_FORCECOEFF0
      m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
      pAlfParam->forceCoeff0 = uiSymbol;

      if (pAlfParam->forceCoeff0)
      {
        pAlfParam->filters_per_group_diff = 0;
        for (int i=0; i<pAlfParam->filters_per_group; i++)
        {
          m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
          pAlfParam->codedVarBins[i] = uiSymbol;
          pAlfParam->filters_per_group_diff += uiSymbol;
        }
      }
      else
#else
      pAlfParam->forceCoeff0 = 0;
#endif
      {
        for (int i=0; i<NO_VAR_BINS; i++)
          pAlfParam->codedVarBins[i] = 1;

      }
      m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
      pAlfParam->predMethod = uiSymbol;
    }
    else
    {
      pAlfParam->forceCoeff0 = 0;
      pAlfParam->predMethod = 0;
    }

    decodeFilterCoeff (pAlfParam);
  }
}

Void TDecEntropy::decodeAlfParam(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int iSymbol;
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->alf_flag = uiSymbol;
  
  if (!pAlfParam->alf_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
    return;
  }
  
  Int pos;
  decodeAux(pAlfParam);
  decodeFilt(pAlfParam);
  // filter parameters for chroma
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->chroma_idc = uiSymbol;
  
  if(pAlfParam->chroma_idc)
  {
    m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
    pAlfParam->tap_chroma = (uiSymbol<<1) + 5;
    pAlfParam->num_coeff_chroma = ((pAlfParam->tap_chroma*pAlfParam->tap_chroma+1)>>1) + 1;
    
    // filter coefficients for chroma
    for(pos=0; pos<pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
      pAlfParam->coeff_chroma[pos] = iSymbol;
    }
  }
  
  // region control parameters for luma
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->cu_control_flag = uiSymbol;
  if (pAlfParam->cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
#if TSB_ALF_HEADER
    pAlfParam->alf_max_depth = uiSymbol;
    decodeAlfCtrlParam(pAlfParam);
#endif
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
  }
}

#if TSB_ALF_HEADER
Void TDecEntropy::decodeAlfCtrlParam( ALFParam* pAlfParam )
{
  UInt uiSymbol;
  m_pcEntropyDecoderIf->parseAlfFlagNum( uiSymbol, pAlfParam->num_cus_in_frame, pAlfParam->alf_max_depth );
  pAlfParam->num_alf_cu_flag = uiSymbol;
  
  for(UInt i=0; i<pAlfParam->num_alf_cu_flag; i++)
  {
    m_pcEntropyDecoderIf->parseAlfCtrlFlag( pAlfParam->alf_cu_flag[i] );
  }
}
#endif

Void TDecEntropy::decodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseAlfCtrlFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
}

#if HHI_MRG
Void TDecEntropy::decodeMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseMergeFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeMergeIndex ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiAbsPartIdx, uiDepth );
}
#endif

Void TDecEntropy::decodeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePredMode( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif
  
  PartSize eMode = pcCU->getPartitionSize( uiAbsPartIdx );
  
  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    if( eMode == SIZE_NxN )                                         // if it is NxN size, encode 4 intra directions.
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      // if it is NxN size, this size might be the smallest partition size.                                                         // if it is NxN size, this size might be the smallest partition size.
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx,                  uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset,   uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*2, uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*3, uiDepth+1 );
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
    else                                                                // if it is not NxN size, encode 1 intra directions
    {
      decodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx, uiDepth );
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    decodeInterDir( pcCU, uiAbsPartIdx, uiDepth );
    
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
      decodeMvd       ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
      decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0, pcSubCU);
    }
    
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
      decodeMvd       ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
      decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1, pcSubCU);
    }
  }
}

Void TDecEntropy::decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirLumaAng( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirChroma( pcCU, uiAbsPartIdx, uiDepth );
}

#if HHI_MRG
Void TDecEntropy::decodeMergeInfo ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    return;
  }
  
  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  
#if SAMSUNG_MRG_SKIP_DIRECT
  if ( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_SKIP )
  {
    return;
  }
#endif
  
  // find left and top vectors. take vectors from PUs to the left and above.
  TComMvField cMvFieldNeighbours[4]; // above ref_list_0, above ref_list_1, left ref_list_0, left ref_list_1
  UInt uiNeighbourInfo;
  UChar uhInterDirNeighbours[2];
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
  if ( uiNeighbourInfo )
  {
    // at least one merge candidate exists
    decodeMergeFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    assert( !pcCU->getMergeFlag( uiAbsPartIdx ) );
  }
  
  if ( !pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    // CU is not merged
    return;
  }
  
  UInt uiMergeIndex = 0;
  if ( uiNeighbourInfo == 3 )
  {
    // two different motion parameter sets exist
    // parse merge index.
    decodeMergeIndex( pcCU, uiAbsPartIdx, uiDepth );
    uiMergeIndex = pcCU->getMergeIndex( uiAbsPartIdx );
  }
  else if ( uiNeighbourInfo == 2)
  {
    // Merge with Left
    uiMergeIndex = 1;
  }
  else
  {
    // Either two identical parameter sets exist or only the above parameter set exists.
    // Merge with above.
    // uiMergeIndex = 0;
  }
  
  pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
  
  // Merge to left or above depending on uiMergeIndex
  // things to copy: spatial vector, reference index, inter directions
  pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiAbsPartIdx, 0, uiDepth );
  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
  {
    pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
    pcSubCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
  }
  
  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
  {
    pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
    pcSubCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
  }
  
}
#endif

Void TDecEntropy::decodeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->isInterP() )
  {
    memset( pcCU->getInterDir() + uiAbsPartIdx, 1, sizeof(UChar)*( pcCU->getTotalNumPart() >> (uiDepth << 1) ) );
    return;
  }
  
  UInt uiInterDir;
  
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;
  
  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
      
    case SIZE_2Nx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
      break;
    }
      
    case SIZE_2NxN:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
      
      uiAbsPartIdx += uiPartOffset << 1;
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
      break;
    }
    case SIZE_Nx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
      
      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
      break;
    }
    case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
        pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
    default:
      break;
  }
  
  return;
}


Void TDecEntropy::decodeMVPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList, TComDataCU* pcSubCU )
{
  Int iMVPIdx;
  
  TComMv cZeroMv( 0, 0 );
  TComMv cMv     = cZeroMv;
  Int    iRefIdx = -1;
  
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  
  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, eRefList );
  
  TComCUMvField* pcSubCUMvField = pcSubCU->getCUMvField( eRefList );
  AMVPInfo* pAMVPInfo = pcSubCUMvField->getAMVPInfo();
  
  switch ( ePartSize )
  {
    case SIZE_2Nx2N:
    {
      iRefIdx = pcSubCUMvField->getRefIdx(0);
      iMVPIdx =-1;
      cMv = cZeroMv;
      pcSubCU->fillMvpCand(0, 0, eRefList, iRefIdx, pAMVPInfo);
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(0), pAMVPInfo);
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, 0, 0, uiDepth);
      if ( (pcSubCU->getInterDir(0) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(0) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, 0, 0, uiDepth );
      
      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredAMVP( pcSubCU, 0, 0, eRefList, iRefIdx, cMv);
        cMv += pcSubCUMvField->getMvd( 0 );
      }
      
      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, 0, 0, 0);
      break;
    }
    case SIZE_2NxN:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 2; uiPartIdx++, uiPartAddr+=(uiPartOffset << 1))
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx =-1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
        
      }
      break;
    }
    case SIZE_Nx2N:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 2; uiPartIdx++, uiPartAddr+=uiPartOffset)
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx =-1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
        
      }
      break;
    }
    case SIZE_NxN:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 4; uiPartIdx++, uiPartAddr+=uiPartOffset)
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx =-1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
      }
      break;
    }
    default:
      break;
  }
  
  return;
}


Void TDecEntropy::decodeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  Int iRefFrmIdx = 0;
  Int iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );
  
  if ( pcCU->isSkip( uiAbsPartIdx ) ) // direct
  {
    if (pcCU->getSlice()->isInterP() && eRefList == REF_PIC_LIST_1)
    {
      iRefFrmIdx = -1;
    }
    
    if (pcCU->getSlice()->isInterB() && !iParseRefFrmIdx)
    {
      iRefFrmIdx = NOT_VALID;
    }
    
    pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    return;
  }
  
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
  
  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
    case SIZE_2Nx2N:
    {
      if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
      {
        m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
      }
      else if ( !iParseRefFrmIdx )
      {
        iRefFrmIdx = NOT_VALID;
      }
      else
      {
        iRefFrmIdx = 0;
      }
      
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      break;
    }
    case SIZE_2NxN:
    {
      if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
      {
        m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
      }
      else if ( !iParseRefFrmIdx )
      {
        iRefFrmIdx = NOT_VALID;
      }
      else
      {
        iRefFrmIdx = 0;
      }
      
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxN, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += (uiPartOffset << 1);
      
      iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );
      
      if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
      {
        m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
      }
      else if ( !iParseRefFrmIdx )
      {
        iRefFrmIdx = NOT_VALID;
      }
      else
      {
        iRefFrmIdx = 0;
      }
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxN, uiAbsPartIdx, 1, uiDepth );
      break;
    }
    case SIZE_Nx2N:
    {
      if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
      {
        m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
      }
      else if ( !iParseRefFrmIdx )
      {
        iRefFrmIdx = NOT_VALID;
      }
      else
      {
        iRefFrmIdx = 0;
      }
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_Nx2N, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += uiPartOffset;
      
      iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );
      
      if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
      {
        m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
      }
      else if ( !iParseRefFrmIdx )
      {
        iRefFrmIdx = NOT_VALID;
      }
      else
      {
        iRefFrmIdx = 0;
      }
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_Nx2N, uiAbsPartIdx, 1, uiDepth );
      break;
    }
    case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );
        
        if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
        {
          m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
        }
        else if ( !iParseRefFrmIdx )
        {
          iRefFrmIdx = NOT_VALID;
        }
        else
        {
          iRefFrmIdx = 0;
        }
        pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_NxN, uiAbsPartIdx, iPartIdx, uiDepth );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
    default:
      break;
  }
  return;
}

Void TDecEntropy::decodeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  if ( pcCU->isSkip( uiAbsPartIdx ) ) // direct
  {
    TComMv cZeroMv;
    pcCU->getCUMvField( eRefList )->setAllMvd( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    return;
  }
  
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;
  
  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
    case SIZE_2Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      break;
    }
    case SIZE_2NxN:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      
      uiAbsPartIdx += (uiPartOffset << 1);
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      break;
    }
    case SIZE_Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      
      uiAbsPartIdx += uiPartOffset;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      break;
    }
    case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        {
          m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
    default:
      break;
  }
  return;
}


#if MS_LAST_CBF
Void TDecEntropy::xDecodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 )
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    uiSubdiv = 0;
  }
  else
  {
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#if HHI_RQT_FORCE_SPLIT_ACC2_PU
    const UInt uiTrMode = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    UInt uiCtx = uiDepth;
#if HHI_RQT_FORCE_SPLIT_NxN
    const Bool bNxNOK = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN && uiTrMode > 0;
#else
    const Bool bNxNOK = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN;
#endif
#if HHI_RQT_FORCE_SPLIT_RECT
    const Bool bSymmetricOK  = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxN  && pcCU->getPartitionSize( uiAbsPartIdx ) < SIZE_NxN   && uiTrMode > 0;
#else
    const Bool bSymmetricOK  = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxN  && pcCU->getPartitionSize( uiAbsPartIdx ) < SIZE_NxN;
#endif
    const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
    bNxNOK || bSymmetricOK;
    
    if( ! bNeedSubdivFlag )
    {
      uiSubdiv = 1;
    }
    else
      m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, uiCtx );
#else
    m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, uiDepth );
#endif
  }
  
  const UInt uiTrDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  
#if LCEC_CBP_YUV_ROOT
  if(pcCU->getSlice()->getSymbolMode()==0)
  {
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      
      for( Int i = 0; i < 4; i++ )
      {
        UInt uiDummyCbfY = 0;
        UInt uiDummyCbfU = 0;
        UInt uiDummyCbfV = 0;
        xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i, uiDummyCbfY, uiDummyCbfU, uiDummyCbfV );
        uiAbsPartIdx += uiQPartNum;
      }
    }
    else
    {
      assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
      pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
    }
  }
  else
  {
#endif
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
    {
      const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepth == 0;
      if( bFirstCbfOfCU )
      {
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
      }
      if( bFirstCbfOfCU || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) )
        {
          if ( uiInnerQuadIdx == 3 && uiUCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
          {
            uiUCbfFront3++;
            pcCU->setCbfSubParts( 1 << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
            //printf( " \nsave bits, U Cbf");
          }
          else
          {
            m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth, uiDepth );
            uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
          }
        }
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) )
        {
          if ( uiInnerQuadIdx == 3 && uiVCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
          {
            uiVCbfFront3++;
            pcCU->setCbfSubParts( 1 << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
            //printf( " \nsave bits, V Cbf");
          }
          else
          {
            m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth, uiDepth );
            uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
          }
        }
      }
      else
      {
        pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
        pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
        
        if ( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
        {
          uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
          uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
        }
      }
    }
    
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      const UInt uiStartAbsPartIdx = uiAbsPartIdx;
      UInt uiLumaTrMode, uiChromaTrMode;
      pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth+1, uiLumaTrMode, uiChromaTrMode );
      UInt uiYCbf = 0;
      UInt uiUCbf = 0;
      UInt uiVCbf = 0;
      
      UInt uiCurrentCbfY = 0;
      UInt uiCurrentCbfU = 0;
      UInt uiCurrentCbfV = 0;
      
      for( Int i = 0; i < 4; i++ )
      {
        xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
        uiYCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
        uiUCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode );
        uiVCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode );
        uiAbsPartIdx += uiQPartNum;
      }
      
      uiYCbfFront3 += uiCurrentCbfY;
      uiUCbfFront3 += uiCurrentCbfU;
      uiVCbfFront3 += uiCurrentCbfV;
      
      pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
      for( UInt ui = 0; ui < 4 * uiQPartNum; ++ui )
      {
        pcCU->getCbf( TEXT_LUMA     )[uiStartAbsPartIdx + ui] |= uiYCbf << uiLumaTrMode;
        pcCU->getCbf( TEXT_CHROMA_U )[uiStartAbsPartIdx + ui] |= uiUCbf << uiChromaTrMode;
        pcCU->getCbf( TEXT_CHROMA_V )[uiStartAbsPartIdx + ui] |= uiVCbf << uiChromaTrMode;
      }
    }
    else
    {
      assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
      pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
      
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tTrIdx: abspart=" );
        DTRACE_CABAC_V( uiAbsPartIdx );
        DTRACE_CABAC_T( "\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttrdepth=" );
        DTRACE_CABAC_V( uiTrDepth );
        DTRACE_CABAC_T( "\n" );
      }
      
      UInt uiLumaTrMode, uiChromaTrMode;
      pcCU->convertTransIdx( uiAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
      pcCU->setCbfSubParts ( 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
      if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
      {
        pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
      }
      else
      {
        if ( pcCU->getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && uiInnerQuadIdx == 3 && uiYCbfFront3 == 0 && uiUCbfFront3 == 0 && uiVCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
        {
          pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
          //printf( " \nsave bits, Y Cbf");
          uiYCbfFront3++;    
        }
        else
        {
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode, uiDepth );
          uiYCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
        }
      }
      
      if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
      {
        Bool bCodeChroma   = true;
        UInt uiDepthChroma = uiDepth;
        if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
        {
          UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
          bCodeChroma  = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
          uiDepthChroma--;
        }
        if( bCodeChroma )
        {
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepthChroma );
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode, uiDepthChroma );
        }
      }
    }
#if LCEC_CBP_YUV_ROOT
  }
#endif
}
#else
Void TDecEntropy::xDecodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx )
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    uiSubdiv = 0;
  }
  else
  {
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#if HHI_RQT_FORCE_SPLIT_ACC2_PU
    const UInt uiTrMode = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    UInt uiCtx = uiDepth;
#if HHI_RQT_FORCE_SPLIT_NxN
    const Bool bNxNOK = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN && uiTrMode > 0;
#else
    const Bool bNxNOK = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN;
#endif
#if HHI_RQT_FORCE_SPLIT_RECT
    const Bool bSymmetricOK  = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxN  && pcCU->getPartitionSize( uiAbsPartIdx ) < SIZE_NxN   && uiTrMode > 0;
#else
    const Bool bSymmetricOK  = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxN  && pcCU->getPartitionSize( uiAbsPartIdx ) < SIZE_NxN;
#endif
    const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
    bNxNOK || bSymmetricOK;
    
    if( ! bNeedSubdivFlag )
    {
      uiSubdiv = 1;
    }
    else
      m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, uiCtx );
#else
    m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, uiDepth );
#endif
  }
  
  const UInt uiTrDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  
#if LCEC_CBP_YUV_ROOT
  if(pcCU->getSlice()->getSymbolMode()==0)
  {
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      const UInt uiStartAbsPartIdx = uiAbsPartIdx;
      
      for( Int i = 0; i < 4; i++ )
      {
        xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i );
        uiAbsPartIdx += uiQPartNum;
      }
    }
    else
    {
      assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
      pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
    }
  }
  else
  {
#endif
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
    {
      const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepth == 0;
      if( bFirstCbfOfCU )
      {
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
      }
      if( bFirstCbfOfCU || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) )
        {
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth, uiDepth );
        }
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) )
        {
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth, uiDepth );
        }
      }
      else
      {
        pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
        pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
      }
    }
    
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      const UInt uiStartAbsPartIdx = uiAbsPartIdx;
      UInt uiLumaTrMode, uiChromaTrMode;
      pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth+1, uiLumaTrMode, uiChromaTrMode );
      UInt uiYCbf = 0;
      UInt uiUCbf = 0;
      UInt uiVCbf = 0;
      
      for( Int i = 0; i < 4; i++ )
      {
        xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i );
        uiYCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
        uiUCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode );
        uiVCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode );
        uiAbsPartIdx += uiQPartNum;
      }
      
      pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
      for( UInt ui = 0; ui < 4 * uiQPartNum; ++ui )
      {
        pcCU->getCbf( TEXT_LUMA     )[uiStartAbsPartIdx + ui] |= uiYCbf << uiLumaTrMode;
        pcCU->getCbf( TEXT_CHROMA_U )[uiStartAbsPartIdx + ui] |= uiUCbf << uiChromaTrMode;
        pcCU->getCbf( TEXT_CHROMA_V )[uiStartAbsPartIdx + ui] |= uiVCbf << uiChromaTrMode;
      }
    }
    else
    {
      assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
      pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
      
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tTrIdx: abspart=" );
        DTRACE_CABAC_V( uiAbsPartIdx );
        DTRACE_CABAC_T( "\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttrdepth=" );
        DTRACE_CABAC_V( uiTrDepth );
        DTRACE_CABAC_T( "\n" );
      }
      
      UInt uiLumaTrMode, uiChromaTrMode;
      pcCU->convertTransIdx( uiAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
      pcCU->setCbfSubParts ( 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
      if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
      {
        pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
      }
      else
        m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode, uiDepth );
      if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
      {
        Bool bCodeChroma   = true;
        UInt uiDepthChroma = uiDepth;
        if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
        {
          UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
          bCodeChroma  = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
          uiDepthChroma--;
        }
        if( bCodeChroma )
        {
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepthChroma );
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode, uiDepthChroma );
        }
      }
    }
#if LCEC_CBP_YUV_ROOT
  }
#endif
}
#endif

Void TDecEntropy::decodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\n" )
#if MS_LAST_CBF
  UInt temp = 0;
  UInt temp1 = 0;
  UInt temp2 = 0;
  xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0, temp, temp1, temp2 );
#else
  xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0 );
#endif
}

Void TDecEntropy::decodeQP          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    m_pcEntropyDecoderIf->parseDeltaQP( pcCU, uiAbsPartIdx, uiDepth );
  }
}


Void TDecEntropy::xDecodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType )
{
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
  {
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    const UInt uiStopTrMode = eType == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
    
    if( uiTrIdx == uiStopTrMode )
    {
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
      if( eType != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
        {
          return;
        }
        uiWidth  <<= 1;
        uiHeight <<= 1;
      }
      m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType );
    }
    else
    {
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing down\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttridx=" );
        DTRACE_CABAC_V( uiTrIdx );
        DTRACE_CABAC_T( "\n" );
      }
      if( uiCurrTrIdx <= uiTrIdx )
        assert(1);
      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;
      
      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;
      
#if LCEC_CBP_YUV_ROOT
      if(pcCU->getSlice()->getSymbolMode() == 0)
      {
        if (1)
        {
          UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
          if( eType == TEXT_LUMA || uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
            m_pcEntropyDecoderIf->parseBlockCbf(pcCU, uiIdx, eType, uiTrIdx, uiDepth, uiQPartNum);
          else
          {
            UInt uiCbf = pcCU->getCbf( uiIdx, eType );
            pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx, uiDepth ); 
            uiCbf = pcCU->getCbf( uiIdx + uiQPartNum, eType );
            pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + uiQPartNum, uiDepth ); 
            uiCbf = pcCU->getCbf( uiIdx + 2*uiQPartNum, eType );
            pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + 2*uiQPartNum, uiDepth ); 
            uiCbf = pcCU->getCbf( uiIdx + 3*uiQPartNum, eType );
            pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + 3*uiQPartNum, uiDepth ); 
          }
        }
        else
          m_pcEntropyDecoderIf->parseBlockCbf(pcCU, uiIdx, eType, uiTrIdx, uiDepth, uiQPartNum);
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
      }
      else
      {
#endif
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
#if LCEC_CBP_YUV_ROOT
      }
#endif
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing up\n" );
      }
    }
  }
}

Void TDecEntropy::decodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
  UInt uiLumaTrMode, uiChromaTrMode;
  
  if( pcCU->isIntra(uiAbsPartIdx) )
  {
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
    
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode() == 0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
        return;
    }
#endif
  }
  else
  {
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode()==0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
        return;
    }
    else
#endif
    {
      UInt uiQtRootCbf;
      m_pcEntropyDecoderIf->parseQtRootCbf( pcCU, uiAbsPartIdx, uiDepth, uiQtRootCbf );
      if ( !uiQtRootCbf )
      {
        pcCU->setCbfSubParts( 0, 0, 0, uiAbsPartIdx, uiDepth );
        pcCU->setTrIdxSubParts( 0 , uiAbsPartIdx, uiDepth );
        return;
      }
    }
    
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
  }
  
  xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
  xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
  xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
}

