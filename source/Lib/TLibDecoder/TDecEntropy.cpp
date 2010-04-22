/* ====================================================================================================================

	The copyright in this software is being made available under the License included below.
	This software may be subject to other third party and 	contributor rights, including patent rights, and no such
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

/** \file			TDecEntropy.cpp
    \brief		entropy decoder class
*/

#include "TDecEntropy.h"

Void TDecEntropy::setEntropyDecoder         ( TDecEntropyIf* p )
{
  m_pcEntropyDecoderIf = p;
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
  // filter parameters for luma
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->tap = (uiSymbol<<1) + 5;
  pAlfParam->num_coeff = ((pAlfParam->tap*pAlfParam->tap+1)>>1) + 1;

  for(pos = 0; pos < pAlfParam->num_coeff; pos++)
  {
    m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
    pAlfParam->coeff[pos] = iSymbol;
  }

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
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
  }
}

Void TDecEntropy::decodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseAlfCtrlFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
}

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
//EXCBand
Void TDecEntropy:: decodeBandCorrValue( TComPic*& rpcPic)
{
	if (rpcPic->getSlice()->getSliceType()== B_SLICE  && rpcPic->getFrameHeightInCU()<=EXC_NO_EC_BSLICE) return;

	rpcPic->setMaxBandNumber(EXB_NB);

	for (Int iBandNumber = 0; iBandNumber <rpcPic->getMaxBandNumber(); iBandNumber ++)
  {
		  Int iCorVal = 0;
		  m_pcEntropyDecoderIf->parseCorrBandExType(iCorVal, iBandNumber);
		  rpcPic->setCorrBandExType(iBandNumber, iCorVal);
  }

}
//EXCBand

Void TDecEntropy:: decodeExtremeValue(TComPic*& rpcPic)
{
  Int iExtremeType;
for (iExtremeType = 0; iExtremeType <5; iExtremeType ++)
   rpcPic->setLocalExtremeValues(iExtremeType,0, 0);

	if (rpcPic->getSlice()->getSliceType()== B_SLICE && rpcPic->getFrameHeightInCU()<=EXC_NO_EC_BSLICE) return;

for (iExtremeType = EXC_CORR_LIMIT+1; iExtremeType <5; iExtremeType ++)
	{
Int iMinVal =0; Int iMaxVal =0;
	   m_pcEntropyDecoderIf->parseExtremeValue( iMinVal, iMaxVal,  iExtremeType);
	   iMinVal-=cgaiCorrDiapason[iExtremeType][0];   // -3...60
	   iMaxVal-=cgaiCorrDiapason[iExtremeType][1];  //-60...3
	   rpcPic->setLocalExtremeValues(iExtremeType,iMinVal, iMaxVal);
	}
}

Void TDecEntropy::decodeROTIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
	if (pcCU->getPredictionMode( uiAbsPartIdx )!=MODE_SKIP)
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
	if (pcCU->getPredictionMode( uiAbsPartIdx )!=MODE_SKIP)
	{
		m_pcEntropyDecoderIf->parseCIPflag( pcCU, uiAbsPartIdx, uiDepth );
	}
}

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  PartSize eMode = pcCU->getPartitionSize( uiAbsPartIdx );

  UInt indexROT = 0;

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
    decodeULTUsedFlag(pcCU, uiAbsPartIdx);
    decodeInterDir( pcCU, uiAbsPartIdx, uiDepth );

    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );
      decodeMvd       ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0 );

			if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
			{
				decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0, pcSubCU);
			}
    }

    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      decodeRefFrmIdx ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );
      decodeMvd       ( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1 );

			if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
			{
				decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1, pcSubCU);
			}
    }
  }
}

Void TDecEntropy::decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->getSPS()->getUseADI() )
    m_pcEntropyDecoderIf->parseIntraDirLumaAdi( pcCU, uiAbsPartIdx, uiDepth );
  else
    m_pcEntropyDecoderIf->parseIntraDirLuma   ( pcCU, uiAbsPartIdx, uiDepth );

	if ( pcCU->getSlice()->getSPS()->getUseMPI() )
	{
// 		UInt uiLPelX   = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
// 		UInt uiTPelY   = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
//     if (uiLPelX && uiTPelY)
      m_pcEntropyDecoderIf->parseMPIindex( pcCU, uiAbsPartIdx, uiDepth );
//     else
// 			pcCU->setMPIindexSubParts( 0, uiAbsPartIdx, uiDepth );
	}
}

Void TDecEntropy::decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirChroma( pcCU, uiAbsPartIdx, uiDepth );
}

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
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  case SIZE_SHV_LB:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += uiPartOffset<<1;

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

      break;
    }
  case SIZE_SHV_RB:
    {
      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 0, uiDepth );

      uiAbsPartIdx += uiPartOffset*3;

      m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
      pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, 1, uiDepth );

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

  TComMv cZeroMv(0,0,0,0);

  TComMv cMv = cZeroMv;
  Int iRefIdx = -1;

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
        m_pcPrediction->getMvPredTemplateMatch( pcSubCU, 0, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
        cMv += pcSubCUMvField->getMvd( 0 );
      }
      cMv.setD(pcSubCUMvField->getMv( 0 ).getHorD(), pcSubCUMvField->getMv( 0 ).getVerD());
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
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
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
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
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
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
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
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
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
        pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
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
				pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
        pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
        if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
        {
          m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
        }
        pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
        pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, uiPartIdx, 0);
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
			pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, 0, uiDepth);
      if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, 0, uiDepth );

      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredTemplateMatch( pcSubCU, 0, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
        cMv += pcSubCUMvField->getMvd( uiPartAddr );
      }
      cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0, 0);

      uiPartAddr = uiPartOffset + (uiPartOffset>>2);
      iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
      iMVPIdx =-1;
      cMv = cZeroMv;
      pcSubCU->fillMvpCand(1, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
			pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
      pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, 1, uiDepth);
      if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
      {
        m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiAbsPartIdx, uiDepth, eRefList );
      }
      pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, 1, uiDepth );

      if ( iRefIdx >= 0 )
      {
        m_pcPrediction->getMvPredTemplateMatch( pcSubCU, 1, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
        cMv += pcSubCUMvField->getMvd( uiPartAddr );
      }
      cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
      pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 1, 0);
      break;
    }
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    {
      for (Int uiPartIdx = 0; uiPartIdx < 2; uiPartIdx++ )
      {
        UInt uiPartAddr = pcSubCU->getFirstPartIdx( uiPartIdx, 0);

        iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
        iMVPIdx =-1;
        cMv = cZeroMv;

        if (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList ))
        {
          pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
					pcSubCU->clearMVPCand(pcSubCUMvField->getMvd(uiPartAddr), pAMVPInfo);
          pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);

          if ( (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
          {
            m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
          }
          pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
        }

        if ( iRefIdx >= 0 )
        {
          m_pcPrediction->getMvPredTemplateMatch( pcSubCU, uiPartIdx, eRefList, iRefIdx, cMv, AMVP_TEMP_SIZE, AMVP_TEMP_SR, true);
          cMv += pcSubCUMvField->getMvd( uiPartAddr );
        }
        cMv.setD(pcSubCUMvField->getMv( uiPartAddr ).getHorD(), pcSubCUMvField->getMv( uiPartAddr ).getVerD());
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
      pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, SIZE_nRx2N, uiAbsPartIdx, 1, uiDepth );
      break;
    }
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    {
      for (Int iPartIdx = 0; iPartIdx < 2; iPartIdx++ )
      {
        UInt uiPartAddr = pcCU->getFirstPartIdx( iPartIdx, uiAbsPartIdx);

        iParseRefFrmIdx = pcCU->getInterDir( uiPartAddr ) & ( 1 << eRefList );

        if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx)
        {
          m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiPartAddr, uiDepth, eRefList );
        }
        else if ( !iParseRefFrmIdx )
        {
          iRefFrmIdx = NOT_VALID;
        }
        else
        {
          iRefFrmIdx = 0;
        }
        pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, pcCU->getPartitionSize( uiAbsPartIdx ), uiPartAddr, iPartIdx, uiDepth );

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
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    {
      for (Int iPartIdx = 0; iPartIdx < 2; iPartIdx++ )
      {
        UInt uiPartAddr = pcCU->getFirstPartIdx( iPartIdx, uiAbsPartIdx);

        if ( pcCU->getInterDir( uiPartAddr ) & ( 1 << eRefList ) )
        {
          m_pcEntropyDecoderIf->parseMvd( pcCU, uiPartAddr, iPartIdx, uiDepth, eRefList );
        }
      }
      break;
    }
  default:
    break;
  }
  return;
}

Void TDecEntropy::decodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseTransformIdx( pcCU, uiAbsPartIdx, uiDepth );
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
    if( uiCurrTrIdx == uiTrIdx )
    {
      m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType );
    }
    else
    {
      if( uiCurrTrIdx <= uiTrIdx )
        assert(0);

      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;

      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyDecoderIf->parseCbf( pcCU, uiIdx, eType, uiTrIdx, uiDepth );
      xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
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
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );

    m_pcEntropyDecoderIf->parseCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth);
    xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
  else
  {
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0, uiDepth );
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0, uiDepth );
    m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0, uiDepth );

    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
      decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );

    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

    xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
    xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
    xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
}

Void TDecEntropy::decodeULTUsedFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if( ( pcCU->isIntra(uiAbsPartIdx) || pcCU->isSkip(uiAbsPartIdx) ) || !pcCU->getSlice()->getUseHAM() )
  {
    pcCU->setHAMUsedSubParts(0, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx));
    return;
  }

  m_pcEntropyDecoderIf->parseULTUsedFlag(pcCU, uiAbsPartIdx);
}
