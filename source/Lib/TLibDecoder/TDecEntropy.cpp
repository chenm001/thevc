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

#if HHI_ALF
Void TDecEntropy::decodeAlfParam(ALFParam* pAlfParam, TComPic* pcPic )
{
  UInt uiSymbol;
  Int iSymbol;
  Int iHorizontalNum_coeffs = 0;
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->alf_flag = uiSymbol;

  if (!pAlfParam->alf_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
    return;
  }

  Int pos;
  // filter parameters for luma
  // horizontal filter
  AlfFilter *pHorizontalFilter  = &pAlfParam->acHorizontalAlfFilter[0];
  pHorizontalFilter->bIsHorizontal = true;
  pHorizontalFilter->bIsVertical   = false;

  m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
  pHorizontalFilter->iFilterLength = ( uiSymbol <<1 ) + ALF_MIN_LENGTH ;
  m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
  pHorizontalFilter->iFilterSymmetry =  uiSymbol ;

  Int iCenterPos = 0;
  if ( pHorizontalFilter->iFilterSymmetry == 0 )
  {
    iHorizontalNum_coeffs = pHorizontalFilter->iFilterLength ;
  }
  else if (pHorizontalFilter->iFilterSymmetry == 1)
  {
    iHorizontalNum_coeffs = (pHorizontalFilter->iFilterLength + 1) >> 1 ;
  }
  iCenterPos = (pHorizontalFilter->iFilterLength + 1) >> 1 ;
  for(pos = 0; pos < iHorizontalNum_coeffs; pos++)
  {
    m_pcEntropyDecoderIf->parseAlfCoeff(iSymbol,pHorizontalFilter->iFilterLength, pos );
    pHorizontalFilter->aiQuantFilterCoeffs[pos] = iSymbol;
  }
#if ALF_DC_CONSIDERED
  m_pcEntropyDecoderIf->parseAlfDc( iSymbol );
  pHorizontalFilter->aiQuantFilterCoeffs[ iHorizontalNum_coeffs ] = iSymbol;
#endif
  // vertical filter
  AlfFilter *pVerticalFilter = &pAlfParam->acVerticalAlfFilter[0];
  pVerticalFilter->bIsVertical = true ;
  pVerticalFilter->bIsHorizontal = false ;

  m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
  pVerticalFilter->iFilterLength =(uiSymbol<<1) + ALF_MIN_LENGTH ;
  m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
  pVerticalFilter->iFilterSymmetry =  uiSymbol  ;

  Int iVerticalNum_coeffs = 0;
  if (pVerticalFilter->iFilterSymmetry == 0)
  {
    iVerticalNum_coeffs = pVerticalFilter->iFilterLength ;
  }
  else if (pVerticalFilter->iFilterSymmetry == 1)
  {
    iVerticalNum_coeffs = (pVerticalFilter->iFilterLength + 1) >> 1 ;
  }
  iCenterPos = (pVerticalFilter->iFilterLength + 1) >> 1 ;
  for(pos = 0; pos < iVerticalNum_coeffs; pos++)
  {
    m_pcEntropyDecoderIf->parseAlfCoeff(iSymbol,pVerticalFilter->iFilterLength, pos );
    pVerticalFilter->aiQuantFilterCoeffs[pos] = iSymbol;
  }
#if ALF_DC_CONSIDERED
  m_pcEntropyDecoderIf->parseAlfDc( iSymbol );
  pVerticalFilter->aiQuantFilterCoeffs[ iVerticalNum_coeffs ] = iSymbol;
#endif
  // filter parameters for chroma
    m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
    pAlfParam->chroma_idc = uiSymbol;
    for(Int iPlane = 1; iPlane <3; iPlane++)
    {
      if(pAlfParam->chroma_idc&iPlane)
      {
        m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
        pAlfParam->aiPlaneFilterMapping[iPlane] = uiSymbol ;
        if( pAlfParam->aiPlaneFilterMapping[iPlane] == iPlane )
        {
          // horizontal filter
          pHorizontalFilter  = &pAlfParam->acHorizontalAlfFilter[iPlane];
          pHorizontalFilter->bIsHorizontal = true;
          pHorizontalFilter->bIsVertical   = false;

          m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
          pHorizontalFilter->iFilterLength = ( uiSymbol <<1 ) + ALF_MIN_LENGTH ;
          m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
          pHorizontalFilter->iFilterSymmetry =  uiSymbol ;

          if ( pHorizontalFilter->iFilterSymmetry == 0 )
          {
            iHorizontalNum_coeffs = pHorizontalFilter->iFilterLength ;
          }
          else if (pHorizontalFilter->iFilterSymmetry == 1)
          {
            iHorizontalNum_coeffs = (pHorizontalFilter->iFilterLength + 1) >> 1 ;
          }
          iCenterPos = (pHorizontalFilter->iFilterLength + 1) >> 1 ;
          for(pos = 0; pos < iHorizontalNum_coeffs; pos++)
          {
            m_pcEntropyDecoderIf->parseAlfCoeff(iSymbol,pHorizontalFilter->iFilterLength, pos );
            pHorizontalFilter->aiQuantFilterCoeffs[pos] = iSymbol;
          }
#if ALF_DC_CONSIDERED
          m_pcEntropyDecoderIf->parseAlfDc(iSymbol);
          pHorizontalFilter->aiQuantFilterCoeffs[ iHorizontalNum_coeffs ] = iSymbol;
#endif
          // vertical filter
          pVerticalFilter = &pAlfParam->acVerticalAlfFilter[iPlane];
          pVerticalFilter->bIsVertical = true ;
          pVerticalFilter->bIsHorizontal = false ;

          m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
          pVerticalFilter->iFilterLength =(uiSymbol<<1) + ALF_MIN_LENGTH ;
          m_pcEntropyDecoderIf->parseAlfUvlc( uiSymbol ) ;
          pVerticalFilter->iFilterSymmetry =  uiSymbol  ;

          if (pVerticalFilter->iFilterSymmetry == 0)
          {
            iVerticalNum_coeffs = pVerticalFilter->iFilterLength ;
          }
          else if (pVerticalFilter->iFilterSymmetry == 1)
          {
            iVerticalNum_coeffs = (pVerticalFilter->iFilterLength + 1) >> 1 ;
          }
          iCenterPos = (pVerticalFilter->iFilterLength + 1) >> 1 ;
          for(pos = 0; pos < iVerticalNum_coeffs; pos++)
          {
            m_pcEntropyDecoderIf->parseAlfCoeff(iSymbol,pVerticalFilter->iFilterLength, pos );
             pVerticalFilter->aiQuantFilterCoeffs[pos] = iSymbol;
          }
#if ALF_DC_CONSIDERED
          m_pcEntropyDecoderIf->parseAlfDc(iSymbol);
          pVerticalFilter->aiQuantFilterCoeffs[ iVerticalNum_coeffs ] = iSymbol;
#endif
        }
      }
    }
  // region control parameters for luma
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->cu_control_flag = 0 != uiSymbol;
  if (pAlfParam->cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfFlag( uiSymbol );
    pAlfParam->bSeparateQt = uiSymbol ? 1 : 0;
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
    if( pAlfParam->bSeparateQt )
    {
      pAlfParam->pcQuadTree = new TComPicSym();
      pAlfParam->pcQuadTree->create( pcPic->getPicYuvRec()->getWidth(), pcPic->getPicYuvRec()->getHeight(),  g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
      for( UInt uiCUAddr = 0; uiCUAddr < pAlfParam->pcQuadTree->getNumberOfCUsInFrame(); uiCUAddr++ )
      {
        TComDataCU* pcCU = pAlfParam->pcQuadTree->getCU(uiCUAddr );
        pcCU->initCU( pcPic , uiCUAddr );
      }
      decodeAlfQuadTree( pAlfParam->pcQuadTree , uiSymbol );
    }
    else
    {
      pAlfParam->pcQuadTree = pcPic->getPicSym();
    }
  }
  else
  {
    pAlfParam->bSeparateQt = false;
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0);
  }
}

Void TDecEntropy::decodeAlfQuadTree( TComPicSym* pcQuadTree , UInt uiMaxDepth )
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU(uiCUAddr );
    decodeAlfQuadTreeNode( pcQuadTree, pcCU, 0, 0, uiMaxDepth );
  }
}

Void TDecEntropy::decodeAlfQuadTreeNode( TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx , UInt uiDepth, UInt uiMaxDepth)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  m_pcEntropyDecoderIf->parseAlfQTSplitFlag( pcCU , uiAbsPartIdx, uiDepth, uiMaxDepth );

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ))
  {
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        decodeAlfQuadTreeNode( pcQuadTree, pcCU, uiAbsPartIdx, uiDepth+1 , uiMaxDepth );
    }
    return;
   }
   
   if(uiDepth > uiMaxDepth)
     return;

   pcCU->setDepth( uiAbsPartIdx , uiDepth );
   m_pcEntropyDecoderIf->parseAlfQTCtrlFlag( pcCU , uiAbsPartIdx, uiDepth );
}
#else
#if QC_ALF  
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
      if(pAlfParam->filterPattern[i])
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1] + 1;
      else
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1];
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
  if(nr != 0){
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
		for (int i=0; i<pAlfParam->filters_per_group; i++){
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


#endif

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
#if QC_ALF  //codeAuxInfo related
  decodeAux(pAlfParam);
  decodeFilt(pAlfParam);
#else
  // filter parameters for luma
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->tap = (uiSymbol<<1) + 5;
  pAlfParam->num_coeff = ((pAlfParam->tap*pAlfParam->tap+1)>>1) + 1;

  for(pos = 0; pos < pAlfParam->num_coeff; pos++)
  {
    m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
    pAlfParam->coeff[pos] = iSymbol;
  }
#endif
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
#endif

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
#if HHI_MRG_PU
Void TDecEntropy::decodeMergeFlag( TComDataCU* pcSubCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{ 
  TComMvField cMvFieldNeighbours[4]; // above ref_list_0, above ref_list_1, left ref_list_0, left ref_list_1
  UInt uiNeighbourInfo;
  UChar uhInterDirNeighbours[2];
#ifdef DCM_PBIC
  TComIc cIcNeighbours[2]; //above, left
  pcSubCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, cIcNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#else
  pcSubCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#endif

  if ( uiNeighbourInfo )
  {
    // at least one merge candidate exists
    m_pcEntropyDecoderIf->parseMergeFlag( pcSubCU, uiAbsPartIdx, uiDepth, uiPUIdx );
  }
  else
  {
    assert( !pcSubCU->getMergeFlag( uiAbsPartIdx ) );
  }
}

Void TDecEntropy::decodeMergeIndex( TComDataCU* pcCU, UInt uiPartIdx, UInt uiAbsPartIdx, PartSize eCUMode, UInt uiDepth )
{
  // find left and top vectors. take vectors from PUs to the left and above.
  TComMvField cMvFieldNeighbours[4]; // above ref_list_0, above ref_list_1, left ref_list_0, left ref_list_1
  UInt uiNeighbourInfo;
  UChar uhInterDirNeighbours[2];
#ifdef DCM_PBIC
  TComIc cIcNeighbours[2]; //above, left
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, cIcNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#else
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#endif

  // Merge to left or above depending on uiMergeIndex
  UInt uiMergeIndex = 0;
  if ( uiNeighbourInfo == 3 ) 
  {
    // two different motion parameter sets exist
    // parse merge index.
    m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiMergeIndex, uiAbsPartIdx, uiDepth );
  }
  else if ( uiNeighbourInfo == 2)
  {
    // Merge with Left
    uiMergeIndex = 1;
  }
  pcCU->setMergeIndexSubParts( uiMergeIndex, uiAbsPartIdx, uiPartIdx, uiDepth );
  pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiAbsPartIdx, uiPartIdx, uiDepth );

  TComMv cTmpMv( 0, 0 );
  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0  ) //if ( ref. frame list0 has at least 1 entry )
  {
    pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );

#ifdef QC_AMVRES
    if ( pcCU->getSlice()->getSPS()->getUseAMVRes())
    {
      if ( uhInterDirNeighbours[uiMergeIndex] != 2 )
      {
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField_AMVRes( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
      }
      else
      {
        pcCU->getCUMvField(REF_PIC_LIST_0)->setAllMVRes      ( false , eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
        //pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
      }
    }
    else
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
#else
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
#endif
   }

  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
  {
    pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );

#ifdef QC_AMVRES

    if ( pcCU->getSlice()->getSPS()->getUseAMVRes())
    {
      if ( uhInterDirNeighbours[uiMergeIndex] != 1 )
      {
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField_AMVRes( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
      }
      else
      {
        pcCU->getCUMvField(REF_PIC_LIST_1)->setAllMVRes      ( false , eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
        // pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
      }
    }
    else
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
#else
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
#endif
  }

#ifdef DCM_PBIC
  if (pcCU->getSlice()->getSPS()->getUseIC())
  { 
    RefPicList eRefList = (uhInterDirNeighbours[uiMergeIndex] == 3) ? REF_PIC_LIST_X : ( (uhInterDirNeighbours[uiMergeIndex] == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    cIcNeighbours[uiMergeIndex].computeScaleOffset( eRefList );
    pcCU->getCUIcField()->setAllIcField( cIcNeighbours[uiMergeIndex], eCUMode, uiAbsPartIdx, uiPartIdx, uiDepth );
  }
#endif
}
#else
Void TDecEntropy::decodeMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseMergeFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeMergeIndex ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiAbsPartIdx, uiDepth );
}
#endif
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

Void TDecEntropy::decodeROTIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (pcCU->getPredictionMode( uiAbsPartIdx )==MODE_INTRA)
  {
    if( ( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V) ) == 0 )
    {
      pcCU->setROTindexSubParts( 0, uiAbsPartIdx, uiDepth );
      return;
    }

    m_pcEntropyDecoderIf->parseROTindex( pcCU, uiAbsPartIdx, uiDepth );
    return;
  }
  pcCU->setROTindexSubParts( 0, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcEntropyDecoderIf->parseCIPflag( pcCU, uiAbsPartIdx, uiDepth );
  }
}

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
#if HHI_MRG && !HHI_MRG_PU
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
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      decodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx,                  uiDepth+1 );
      decodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset,   uiDepth+1 );
      decodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset*2, uiDepth+1 );
      decodeIntraFiltFlagLuma( pcCU, uiAbsPartIdx + uiPartOffset*3, uiDepth+1 );
      //
#endif
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
    else                                                                // if it is not NxN size, encode 1 intra directions
    {
      decodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx, uiDepth );
#if HHI_AIS
      //BB: intra ref. samples filtering flag
      decodeIntraFiltFlagLuma ( pcCU, uiAbsPartIdx, uiDepth );
      //
#endif
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
#if HHI_MRG_PU
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      decodePUWise( pcCU, uiAbsPartIdx, uiDepth, pcSubCU );
    }
    else
#endif
    {
      decodeInterDir( pcCU, uiAbsPartIdx, uiDepth );

#ifdef DCM_PBIC
      if (pcCU->getSlice()->getSPS()->getUseIC())
      {
        if ( ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) && ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) )
        {
          // Both ref. frame list0 & ref. frame list1 have at least 1 entry each
          decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
          decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
          decodeMvdIcd    ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_X );
          decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0, pcSubCU);
          decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1, pcSubCU);
        }
        else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        {
          // Only ref. frame list0 has at least 1 entry
          decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
          decodeMvdIcd    ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
          decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0, pcSubCU);
        }
        else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        {
          // Only ref. frame list1 has at least 1 entry
          decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
          decodeMvdIcd    ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
          decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1, pcSubCU);
        }
        if ( ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) || ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) )
        {
          decodeICPIdx  ( pcCU, uiAbsPartIdx, uiDepth, pcSubCU);
        }
      }
      else
#endif
      {
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
  }
}

#if PLANAR_INTRA
Void TDecEntropy::decodePlanarInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->isInterB() )
    return;

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcEntropyDecoderIf->parsePlanarInfo( pcCU, uiAbsPartIdx, uiDepth );
  }
}
#endif

Void TDecEntropy::decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if ANG_INTRA
  if ( pcCU->angIntraEnabledPredPart( uiAbsPartIdx ) )
    m_pcEntropyDecoderIf->parseIntraDirLumaAng( pcCU, uiAbsPartIdx, uiDepth );
  else
    m_pcEntropyDecoderIf->parseIntraDirLumaAdi( pcCU, uiAbsPartIdx, uiDepth );
#else
  m_pcEntropyDecoderIf->parseIntraDirLumaAdi( pcCU, uiAbsPartIdx, uiDepth );
#endif
}

#if HHI_AIS
Void TDecEntropy::decodeIntraFiltFlagLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  // DC (mode 2) always uses DEFAULT_IS so no parsing needed
  // (no g_aucIntraModeOrder[][] mapping needed because mode 2 always mapped to 2)
  if( (pcCU->getSlice()->getSPS()->getUseAIS()) && (pcCU->getLumaIntraDir( uiAbsPartIdx ) != 2) )
      m_pcEntropyDecoderIf->parseIntraFiltFlagLumaAdi( pcCU, uiAbsPartIdx, uiDepth );
  else
    pcCU->setLumaIntraFiltFlagSubParts( DEFAULT_IS, uiAbsPartIdx, uiDepth );
}
#endif

Void TDecEntropy::decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirChroma( pcCU, uiAbsPartIdx, uiDepth );
}

#if HHI_MRG && !HHI_MRG_PU
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
#ifdef DCM_PBIC 
  TComIc cIcNeighbours[2];
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, cIcNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#else
  pcCU->getInterMergeCandidates( uiAbsPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfo );
#endif
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
#ifdef QC_AMVRES
	if ( pcCU->getSlice()->getSPS()->getUseAMVRes())
	   pcSubCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField_AMVRes( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
	else
    pcSubCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
#else
    pcSubCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
#endif
  }

  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
  {
    pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
#ifdef QC_AMVRES
	if ( pcCU->getSlice()->getSPS()->getUseAMVRes())
		pcSubCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField_AMVRes( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
    else
    pcSubCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
#else
	pcSubCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getMv(), cMvFieldNeighbours[ 2*uiMergeIndex + 1 ].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 );
#endif
  }

#ifdef DCM_PBIC
  if (pcCU->getSlice()->getSPS()->getUseIC())
  { 
    RefPicList eRefList = (uhInterDirNeighbours[uiMergeIndex] == 3) ? REF_PIC_LIST_X : ( (uhInterDirNeighbours[uiMergeIndex] == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);;
    cIcNeighbours[uiMergeIndex].computeScaleOffset( eRefList );
    pcSubCU->getCUIcField()->setAllIcField( cIcNeighbours[uiMergeIndex], SIZE_2Nx2N, 0, 0, 0);
  }
#endif

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
  case SIZE_2NxnU:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += (uiPartOffset>>1);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  case SIZE_2NxnD:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  case SIZE_nLx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += (uiPartOffset>>2);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  case SIZE_nRx2N:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  default:
    break;
  }

  return;
}

#if HHI_MRG_PU
Void TDecEntropy::decodePUWise( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  UInt uiNumPU = ( ePartSize == SIZE_2Nx2N ? 1 : ( ePartSize == SIZE_NxN ? 4 : 2 ) );
  UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;

  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );

  for ( UInt uiPartIdx = 0, uiSubPartIdx = uiAbsPartIdx; uiPartIdx < uiNumPU; uiPartIdx++, uiSubPartIdx += uiPUOffset )
  {
    decodeMergeFlag( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
      decodeMergeIndex( pcCU, uiPartIdx, uiSubPartIdx, ePartSize, uiDepth );
    }
    else
    {
      decodeInterDirPU( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );

#ifdef DCM_PBIC
      if (pcCU->getSlice()->getSPS()->getUseIC())
      {
        if ( ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) && ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) )
        {
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, REF_PIC_LIST_0 );
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, REF_PIC_LIST_1 );
          decodeMvdIcdPU   ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx                 );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, REF_PIC_LIST_0 );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, REF_PIC_LIST_1 );
        }
        else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        {
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, REF_PIC_LIST_0 );
          decodeMvdIcdPU   ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx                 );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, REF_PIC_LIST_0 );
        }
        else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        {
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, REF_PIC_LIST_1 );
          decodeMvdIcdPU   ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx                 );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, REF_PIC_LIST_1 );
        }
        if ( ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) || ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) )
        {
          decodeICPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx                 );
        }
      }
      else
#endif
      {
        for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
        {        
          if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
          {
            decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
            decodeMvdPU      ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
            decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
          }
        }
      }
    }
  }
  
  return;
}

Void TDecEntropy::decodeInterDirPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx )
{
  UInt uiInterDir;

  if ( pcCU->getSlice()->isInterP() )
  {
    uiInterDir = 1;
  }
  else
  {
    m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
  }

  pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeRefFrmIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isSkip( uiAbsPartIdx ) );

  Int iRefFrmIdx = 0;
  Int iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );

  if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx )
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

  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );

#ifdef QC_AMVRES
  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, ePartSize, uiPartIdx );
#endif
  pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, ePartSize, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeMvdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isSkip( uiAbsPartIdx ) );

  if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
  {
    m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, uiPartIdx, uiDepth, eRefList );
  }
}

Void TDecEntropy::decodeMVPIdxPU( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  Int iMVPIdx;

  TComMv cZeroMv( 0, 0 );
  TComMv cMv     = cZeroMv;
  Int    iRefIdx = -1;

  TComCUMvField* pcSubCUMvField = pcSubCU->getCUMvField( eRefList );
  AMVPInfo* pAMVPInfo = pcSubCUMvField->getAMVPInfo();

  iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
  iMVPIdx = -1;
  cMv = cZeroMv;
  pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
  if (pcSubCUMvField->getMVRes(uiPartAddr))
    pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
  else
#endif
    pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
  if (pcSubCUMvField->getMVRes(uiPartAddr))
    pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
  else
#endif
    pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
  pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
  if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
  }
  pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
  if ( iRefIdx >= 0 )
  {
    m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
    if (pcSubCUMvField->getMVRes(uiPartAddr))
    {
#if HHI_IMVP
      if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
      {
        m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
      }
#endif
      cMv.round();
    }
    else
    {
#if HHI_IMVP
      if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
      {
        m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
      }
#endif
    }
#else
#if HHI_IMVP 
    if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
    {
      m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
    }
#endif
#endif
    cMv += pcSubCUMvField->getMvd( uiPartAddr );
  }

  PartSize ePartSize = pcSubCU->getPartitionSize( uiPartAddr );
  pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
#ifdef QC_AMVRES
  pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
}

#ifdef DCM_PBIC
Void TDecEntropy::decodeMvdIcdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx )
{
  assert( !pcCU->isSkip( uiAbsPartIdx ) );

  UChar uhInterDir = pcCU->getInterDir( uiAbsPartIdx );
  assert( (uhInterDir == 1) || (uhInterDir == 2) || (uhInterDir == 3) );
  RefPicList eRefList = (uhInterDir == 3) ? REF_PIC_LIST_X : ( (uhInterDir == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);

  m_pcEntropyDecoderIf->parseMvdIcd( pcCU, uiAbsPartIdx, uiPartIdx, uiDepth, eRefList );
}

Void TDecEntropy::decodeICPIdxPU( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx )
{
  Int iICPIdx = -1;
  TComIc cIc;

  UChar uhInterDir = pcSubCU->getInterDir(uiPartAddr);
  assert ( (uhInterDir == 1) || (uhInterDir == 2) || (uhInterDir == 3) );
  RefPicList eRefList = (uhInterDir == 3) ? REF_PIC_LIST_X : ( (uhInterDir == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);

  PartSize ePartSize = pcSubCU->getPartitionSize( uiPartAddr );
  TComCUIcField* pcSubCUIcField = pcSubCU->getCUIcField();
  AICPInfo* pAICPInfo = pcSubCUIcField->getAICPInfo();

  pcSubCU->fillICPCand(uiPartIdx, uiPartAddr, pAICPInfo);
  pcSubCU->setICPNumSubParts(pAICPInfo->iN, uiPartAddr, uiPartIdx, uiDepth);
  if (pAICPInfo->iN > 1)
  {
    m_pcEntropyDecoderIf->parseICPIdx( pcSubCU, iICPIdx, pAICPInfo->iN, uiPartAddr, uiDepth );
  }
  pcSubCU->setICPIdxSubParts( iICPIdx, uiPartAddr, uiPartIdx, uiDepth );

  m_pcPrediction->getIcPredAICP( pcSubCU, uiPartIdx, uiPartAddr, cIc);

  cIc.addIcParamDiff( pcSubCUIcField->getIcd(uiPartAddr) );
  cIc.computeScaleOffset( eRefList );

  pcSubCU->getCUIcField()->setAllIc(cIc, ePartSize, uiPartAddr, uiPartIdx, 0);
}
#endif

#endif

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
#if HHI_IMVP 
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(0))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(0), pAMVPInfo, eRefList, 0, iRefIdx);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(0), pAMVPInfo, eRefList, 0, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(0))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(0), pAMVPInfo);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(0), pAMVPInfo);
#endif
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, 0, 0, uiDepth);
      if ( (pcSubCU->getInterDir(0) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(0) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, 0, 0, uiDepth );

      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredAMVP( pcSubCU, 0, 0, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(0))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, 0, 0, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, 0, 0, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP 
        if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
        {
          m_pcPrediction->getMvPredIMVP( pcSubCU, 0, 0, eRefList, iRefIdx, pcSubCUMvField, cMv );
        }
#endif
#endif
        cMv += pcSubCUMvField->getMvd( 0 );
      }

      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, 0, 0, 0);
#ifdef QC_AMVRES
	  pcSubCU->getCUMvField( eRefList )->setAllMVRes((!cMv.isHAM())&&(iRefIdx!=-1), ePartSize, 0, 0, 0);
#endif
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
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);

#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);

#ifdef QC_AMVRES
  	   pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
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
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );

        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);

#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
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
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
		}
#else
#if HHI_IMVP 
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
      }
      break;
    }
  case SIZE_2NxnU:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 2; uiPartIdx++, uiPartAddr+=(uiPartOffset>>1))
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx = -1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv );
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);

#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
      }
      break;
    }
  case SIZE_2NxnD:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 2; uiPartIdx++, uiPartAddr+=(uiPartOffset<<1) + (uiPartOffset>>1))
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx = -1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
		}
#else
#if HHI_IMVP 
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);

#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
      }
      break;
    }
  case SIZE_nLx2N:
    {
      for ( UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < 2; uiPartIdx++, uiPartAddr+=(uiPartOffset>>2))
      {
        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx = -1;
        cMv = cZeroMv;
        pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
		}
#else
#if HHI_IMVP 
          if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
          {
            m_pcPrediction->getMvPredIMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
          }
#endif
#endif
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }

        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, uiPartIdx, 0);
#endif
      }
      break;
    }
  case SIZE_nRx2N:
    {
      UInt uiPartAddr = 0;
      iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
      iMVPIdx =-1;
      cMv = cZeroMv;
      pcSubCU->fillMvpCand(0, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, 0, iRefIdx);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, 0, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, 0, uiDepth);
      if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, 0, uiDepth );

      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredAMVP( pcSubCU, 0, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, 0, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, 0, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP
        if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(0) )
        {
          m_pcPrediction->getMvPredIMVP( pcSubCU, 0, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
        }
#endif
#endif
        cMv += pcSubCUMvField->getMvd( uiPartAddr );
      }

      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0, 0);

#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, 0, 0);
#endif

      uiPartAddr = uiPartOffset + (uiPartOffset>>2);
      iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
      iMVPIdx =-1;
      cMv = cZeroMv;
      pcSubCU->fillMvpCand(1, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
#if HHI_IMVP
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo, eRefList, uiPartAddr, iRefIdx);
#else
#ifdef QC_AMVRES
	  if (pcSubCUMvField->getMVRes(uiPartAddr))
		  pcSubCU->clearMVPCand_one_fourth(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
	  else
#endif
      pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
#endif
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, 1, uiDepth);
      if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, 1, uiDepth );

      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredAMVP( pcSubCU, 1, uiPartAddr, eRefList, iRefIdx, cMv);
#ifdef QC_AMVRES
		if (pcSubCUMvField->getMVRes(uiPartAddr))
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP_onefourth( pcSubCU, 1, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		   cMv.round();
		}
		else
		{
#if HHI_IMVP
			if (  pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
			{
			  m_pcPrediction->getMvPredIMVP( pcSubCU, 1, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
			}
#endif
		}
#else
#if HHI_IMVP
        if ( pcSubCU->getSlice()->getSPS()->getUseIMP() && !pcSubCU->isSkip(uiPartAddr) )
        {
          m_pcPrediction->getMvPredIMVP( pcSubCU, 1, uiPartAddr, eRefList, iRefIdx, pcSubCUMvField, cMv );
        }
#endif
#endif
        cMv += pcSubCUMvField->getMvd( uiPartAddr );
      }

      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 1, 0);

#ifdef QC_AMVRES
		pcSubCU->getCUMvField( eRefList )->setAllMVRes(!cMv.isHAM()&&(iRefIdx!=-1), ePartSize, uiPartAddr, 1, 0);
#endif
      break;
    }
  default:
    break;
  }

  return;
}

#ifdef DCM_PBIC
Void TDecEntropy::decodeICPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  Int iICPIdx;
  TComIc cIc;
  UInt uiPartIdxTotal;
  UInt uiPartAddrInc;
  UChar uhInterDir;
  RefPicList eRefList;

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );

  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_X );

  TComCUIcField* pcSubCUIcField = pcSubCU->getCUIcField();
  AICPInfo* pAICPInfo = pcSubCUIcField->getAICPInfo();

  switch ( ePartSize )
  {
  case SIZE_2Nx2N:
    {
      uiPartIdxTotal = 1;
      uiPartAddrInc  = 0;
      break;
    }
  case SIZE_2NxN:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset << 1);
      break;
    }
  case SIZE_Nx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = uiPartOffset;
      break;
    }
  case SIZE_NxN:
    {
      uiPartIdxTotal = 4;
      uiPartAddrInc  = uiPartOffset;
      break;
    }
  case SIZE_2NxnU:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset>>1);
      break;
    }
  case SIZE_2NxnD:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset<<1) + (uiPartOffset>>1);
      break;
    }
  case SIZE_nLx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset>>2);
      break;
    }
  case SIZE_nRx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = uiPartOffset + (uiPartOffset>>2);
      break;
    }
  default:
    {
      uiPartIdxTotal = 0;
      uiPartAddrInc  = 0;
      break;
    }
  }
  for (UInt uiPartIdx = 0, uiPartAddr = 0; uiPartIdx < uiPartIdxTotal; uiPartIdx++, uiPartAddr += uiPartAddrInc)
  {
    cIc.reset();
    iICPIdx = -1;
    uhInterDir = pcSubCU->getInterDir(uiPartAddr);
    assert ( (uhInterDir == 1) || (uhInterDir == 2) || (uhInterDir == 3) );
    eRefList = (uhInterDir == 3) ? REF_PIC_LIST_X : ( (uhInterDir == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);

    pcSubCU->fillICPCand(uiPartIdx, uiPartAddr, pAICPInfo);
    pcSubCU->setICPNumSubParts(pAICPInfo->iN, uiPartAddr, uiPartIdx, uiDepth);
    if (pAICPInfo->iN > 1)
    {
      m_pcEntropyDecoderIf->parseICPIdx( pcSubCU, iICPIdx, pAICPInfo->iN, uiPartAddr, uiDepth );
    }
    pcSubCU->setICPIdxSubParts( iICPIdx, uiPartAddr, uiPartIdx, uiDepth );

    m_pcPrediction->getIcPredAICP( pcSubCU, uiPartIdx, uiPartAddr, cIc);

    cIc.addIcParamDiff( pcSubCUIcField->getIcd(uiPartAddr) );
    cIc.computeScaleOffset( eRefList );

    pcSubCU->getCUIcField()->setAllIc(cIc, ePartSize, uiPartAddr, uiPartIdx, 0);
  }
}
#endif

#ifdef QC_AMVRES
Void TDecEntropy::xDecodeMvRes(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList, Int &iRefFrmIdx, Int iParseRefFrmIdx, PartSize ePartSize,UInt PartIdx)
{
	  if (pcCU->getSlice()->getSymbolMode()==0 && pcCU->getSlice()->getSPS()->getUseAMVRes() && iRefFrmIdx >= 0)
	  {
		  Bool bMVres=true;
		  if (pcCU->getSlice()->getNumRefIdx( eRefList ) == 1&& iParseRefFrmIdx)
		  {
			   m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
		  }
		  if (iRefFrmIdx == pcCU->getSlice()->getNumRefIdx( eRefList ))
		  {
			  bMVres = 0;
			  iRefFrmIdx = 0;
		  }	
		  pcCU->getCUMvField( eRefList )->setAllMVRes (bMVres, ePartSize, uiAbsPartIdx, PartIdx, uiDepth );
	  }
	  else
	  {
		  pcCU->getCUMvField( eRefList )->setAllMVRes (false, ePartSize, uiAbsPartIdx, PartIdx, uiDepth );
	  }
}
#endif

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

#ifdef QC_AMVRES
	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2Nx2N,0);
#endif
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
#ifdef QC_AMVRES
	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxN,0);
#endif

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
#ifdef QC_AMVRES
	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxN,1);
#endif
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
#ifdef QC_AMVRES
	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_Nx2N,0);
#endif
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
#ifdef QC_AMVRES
	  	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_Nx2N,0);
#endif
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
#ifdef QC_AMVRES
      xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_NxN,iPartIdx);
#endif
        pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_NxN, uiAbsPartIdx, iPartIdx, uiDepth );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
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
#ifdef QC_AMVRES
	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxnU,0);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxnU, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += (uiPartOffset >> 1);

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
#ifdef QC_AMVRES
 	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxnU,1);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxnU, uiAbsPartIdx, 1, uiDepth );
      break;
    }
  case SIZE_2NxnD:
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
#ifdef QC_AMVRES
 	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxnD,0);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxnD, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset >> 1);

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
#ifdef QC_AMVRES
   	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_2NxnD,1);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_2NxnD, uiAbsPartIdx, 1, uiDepth );
      break;
    }
  case SIZE_nLx2N:
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
#ifdef QC_AMVRES
   	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_nLx2N,0);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_nLx2N, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += (uiPartOffset >> 2);

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
#ifdef QC_AMVRES
 	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_nLx2N,1);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_nLx2N, uiAbsPartIdx, 1, uiDepth );
      break;
    }
  case SIZE_nRx2N:
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
#ifdef QC_AMVRES
   	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_nRx2N,0);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_nRx2N, uiAbsPartIdx, 0, uiDepth );
      uiAbsPartIdx += uiPartOffset + (uiPartOffset >> 2);

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
#ifdef QC_AMVRES
   	  xDecodeMvRes(pcCU, uiAbsPartIdx, uiDepth, eRefList, iRefFrmIdx, iParseRefFrmIdx, SIZE_nRx2N,1);
#endif
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_nRx2N, uiAbsPartIdx, 1, uiDepth );
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
  case SIZE_2NxnU:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      uiAbsPartIdx += (uiPartOffset>>1);
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 1, uiDepth, eRefList );
      }
      break;
    }
  case SIZE_2NxnD:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 1, uiDepth, eRefList );
      }
      break;
    }
  case SIZE_nLx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      uiAbsPartIdx += (uiPartOffset>>2);
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 1, uiDepth, eRefList );
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 0, uiDepth, eRefList );
      }
      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, 1, uiDepth, eRefList );
      }
      break;
    }
  default:
    break;
  }
  return;
}

#ifdef DCM_PBIC
Void TDecEntropy::decodeMvdIcd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  if ( pcCU->isSkip( uiAbsPartIdx ) ) // direct
  {
    TComMv cZeroMv;
    if (eRefList == REF_PIC_LIST_X)
    {
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
    else
      pcCU->getCUMvField(       eRefList )->setAllMvd( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );

    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      TComIc cDefaultIc;
      pcCU->getCUIcField()->setAllIcd( cDefaultIc, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
    return;
  }

  UInt uiPartIdxTotal, uiPartAddrInc;
  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( uiDepth << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      uiPartIdxTotal = 1;
      uiPartAddrInc  = 0;
      break;
    }
  case SIZE_2NxN:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset << 1);
      break;
    }
  case SIZE_Nx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = uiPartOffset;
      break;
    }
  case SIZE_NxN:
    {
      uiPartIdxTotal = 4;
      uiPartAddrInc  = uiPartOffset;
      break;
    }
  case SIZE_2NxnU:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset>>1);
      break;
    }
  case SIZE_2NxnD:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset<<1) + (uiPartOffset>>1);
      break;
    }
  case SIZE_nLx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = (uiPartOffset>>2);
      break;
    }
  case SIZE_nRx2N:
    {
      uiPartIdxTotal = 2;
      uiPartAddrInc  = uiPartOffset + (uiPartOffset>>2);
      break;
    }
  default:
    {
      uiPartIdxTotal = 0;
      uiPartAddrInc  = 0;
      break;
    }
  }

  UChar uhInterDir;
  for (UInt uiPartIdx = 0, uiPartAddr = uiAbsPartIdx; uiPartIdx < uiPartIdxTotal; uiPartIdx++, uiPartAddr += uiPartAddrInc)
  {
    uhInterDir = pcCU->getInterDir( uiPartAddr );
    assert ( (uhInterDir == 1) || (uhInterDir == 2) || (uhInterDir == 3) );
    eRefList = (uhInterDir == 3) ? REF_PIC_LIST_X : ( (uhInterDir == 2) ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    m_pcEntropyDecoderIf->parseMvdIcd( pcCU, uiPartAddr, uiPartIdx, uiDepth, eRefList );
  }
}
#endif

#if HHI_RQT

#if MS_LAST_CBF
Void TDecEntropy::xDecodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 )
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;

#if HHI_RQT_INTRA
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#else
  if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#endif
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    uiSubdiv = 0;
  }
#endif  
  else
  {
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#else
    assert( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
#endif
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
#if HHI_RQT_FORCE_SPLIT_ASYM
  const Bool bAsymmetricOK   = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsPartIdx ) <= SIZE_nRx2N && uiTrMode > 1;
  const Bool b2NxnUOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2NxnU && uiInnerQuadIdx > 1      && uiTrMode == 1;
  const Bool b2NxnDOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2NxnD && uiInnerQuadIdx < 2      && uiTrMode == 1;
  const Bool bnLx2NOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_nLx2N &&  ( uiInnerQuadIdx & 1 ) && uiTrMode == 1;
  const Bool bnRx2NOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_nRx2N && !( uiInnerQuadIdx & 1 ) && uiTrMode == 1;
  const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
                               bNxNOK || bSymmetricOK || bAsymmetricOK || b2NxnUOK || b2NxnDOK || bnLx2NOK || bnRx2NOK;
#else
  const Bool bAsymmetricOK   = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsPartIdx ) <= SIZE_nRx2N;
  const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
                               bNxNOK || bSymmetricOK || bAsymmetricOK;
#endif

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
#if HHI_RQT_CHROMA_CBF_MOD
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
#endif

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
#if HHI_RQT_ROOT && HHI_RQT_CHROMA_CBF_MOD
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
    {
      pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    }
    else
#endif
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
#if HHI_RQT_CHROMA_CBF_MOD
    if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
#endif
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

#if HHI_RQT_INTRA
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#else
  if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#endif
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    uiSubdiv = 0;
  }
#endif  
  else
  {
#if HHI_RQT_DEPTH || HHI_RQT_DISABLE_SUB
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
#else
    assert( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() );
#endif
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
#if HHI_RQT_FORCE_SPLIT_ASYM
  const Bool bAsymmetricOK   = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsPartIdx ) <= SIZE_nRx2N && uiTrMode > 1;
  const Bool b2NxnUOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2NxnU && uiInnerQuadIdx > 1      && uiTrMode == 1;
  const Bool b2NxnDOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2NxnD && uiInnerQuadIdx < 2      && uiTrMode == 1;
  const Bool bnLx2NOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_nLx2N &&  ( uiInnerQuadIdx & 1 ) && uiTrMode == 1;
  const Bool bnRx2NOK        = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_nRx2N && !( uiInnerQuadIdx & 1 ) && uiTrMode == 1;
  const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
                               bNxNOK || bSymmetricOK || bAsymmetricOK || b2NxnUOK || b2NxnDOK || bnLx2NOK || bnRx2NOK;
#else
  const Bool bAsymmetricOK   = pcCU->getPartitionSize( uiAbsPartIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsPartIdx ) <= SIZE_nRx2N;
  const Bool bNeedSubdivFlag = pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N || pcCU->getPredictionMode( uiAbsPartIdx ) == MODE_INTRA ||
                               bNxNOK || bSymmetricOK || bAsymmetricOK;
#endif

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
#if HHI_RQT_CHROMA_CBF_MOD
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
#endif

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
#if HHI_RQT_ROOT && HHI_RQT_CHROMA_CBF_MOD
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
    {
      pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    }
    else
#endif
    m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode, uiDepth );
#if HHI_RQT_CHROMA_CBF_MOD
    if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
#endif
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
#endif

Void TDecEntropy::decodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_RQT
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
#else
  m_pcEntropyDecoderIf->parseTransformIdx( pcCU, uiAbsPartIdx, uiDepth );
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
#if HHI_RQT
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    const UInt uiStopTrMode = eType == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;

    if( uiTrIdx == uiStopTrMode )
#else
    if( uiCurrTrIdx == uiTrIdx )
#endif
    {
#if HHI_RQT
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
#endif
      m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType );
    }
    else
    {
#if HHI_RQT
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing down\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttridx=" );
        DTRACE_CABAC_V( uiTrIdx );
        DTRACE_CABAC_T( "\n" );
      }
#endif
      if( uiCurrTrIdx <= uiTrIdx )
#if HHI_RQT
        assert(1);
#else
        assert(0);
#endif
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
#if HHI_RQT
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
#else
        m_pcEntropyDecoderIf->parseBlockCbf(pcCU, uiIdx, eType, uiTrIdx, uiDepth, uiQPartNum);
#endif
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
      }
      else
      {
#endif
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
#if LCEC_CBP_YUV_ROOT
      }
#endif
#if HHI_RQT
      {
        DTRACE_CABAC_V( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing up\n" );
      }
#endif
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
#if HHI_RQT_INTRA
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
#endif

    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

#if QC_MDDT

#if LCEC_PHASE2
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode())
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );
    }
#else
    if (pcCU->getSlice()->getSymbolMode()==0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
    }
    else
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );
    }
#endif
#else
	m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth);
	m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth);
	m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth);
#endif

#if !QC_MDDT_ROT_UNIFIED
    if (pcCU->getSlice()->getSPS()->getUseROT() && uiWidth >= 16)
#endif
  		decodeROTIdx( pcCU, uiAbsPartIdx, uiDepth );
	xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
	xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
	xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
	
#else

#if LCEC_PHASE2
#if LCEC_CBP_YUV_ROOT
    if (pcCU->getSlice()->getSymbolMode() == 0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
         return;
    }
#else
    if (pcCU->getSlice()->getSymbolMode()==0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
    }
#endif
    else
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );
    }

    xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
    xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
    xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
#else

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
#endif
#endif
  }
  else
  {
#if HHI_RQT_ROOT
#if LCEC_CBP_YUV_ROOT
    if( pcCU->getSlice()->getSymbolMode())
#else
    if( 1 )
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
#endif

#if LCEC_PHASE2
    if (pcCU->getSlice()->getSymbolMode()==0)
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
#if LCEC_CBP_YUV_ROOT
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
         return;
#endif
    }
    else
    {
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );
    }
#else
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );
#endif

#if HHI_RQT
#else
    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
#endif
      decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );

    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

    xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
    xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
    xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
}

