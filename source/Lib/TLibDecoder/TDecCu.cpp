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

/** \file			TDecCu.cpp
    \brief		CU decoder class
*/

#include "TDecCu.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecCu::TDecCu()
{
  m_ppcYuvPred = NULL;
  m_ppcYuvResi = NULL;
  m_ppcYuvReco = NULL;

  m_ppcCU      = NULL;
}

TDecCu::~TDecCu()
{

}

Void TDecCu::init( TDecEntropy* pcEntropyDecoder, TComTrQuant* pcTrQuant, TComPrediction* pcPrediction)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcTrQuant         = pcTrQuant;
  m_pcPrediction      = pcPrediction;
}

Void TDecCu::create( UInt uiMaxDepth, UInt uiMaxWidth, UInt uiMaxHeight )
{
  m_uiMaxDepth = uiMaxDepth+1;

  m_ppcYuvPred = new TComYuv*[m_uiMaxDepth-1];
  m_ppcYuvResi = new TComYuv*[m_uiMaxDepth-1];
  m_ppcYuvReco = new TComYuv*[m_uiMaxDepth-1];
  m_ppcCU      = new TComDataCU*[m_uiMaxDepth-1];

  UInt uiNumPartitions;
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    uiNumPartitions = 1<<( ( m_uiMaxDepth - ui - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> ui;
    UInt uiHeight = uiMaxHeight >> ui;

    m_ppcYuvPred[ui] = new TComYuv;    m_ppcYuvPred[ui]->create( uiWidth, uiHeight );
    m_ppcYuvResi[ui] = new TComYuv;    m_ppcYuvResi[ui]->create( uiWidth, uiHeight );
    m_ppcYuvReco[ui] = new TComYuv;    m_ppcYuvReco[ui]->create( uiWidth, uiHeight );
    m_ppcCU     [ui] = new TComDataCU; m_ppcCU     [ui]->create( uiNumPartitions, uiWidth, uiHeight, true );
  }

  // initialize partition order.
  UInt* piTmp = &g_auiConvertRelToAbsIdx[0];
  ConvertRelToAbsIdx(m_uiMaxDepth, 1, 0, piTmp);
  ConvertAbsToRelIdx( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );

  // initialize conversion matrix from partition index to pel
  ConvertPartIdxToPel( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
}

Void TDecCu::destroy()
{
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    m_ppcYuvPred[ui]->destroy(); delete m_ppcYuvPred[ui]; m_ppcYuvPred[ui] = NULL;
    m_ppcYuvResi[ui]->destroy(); delete m_ppcYuvResi[ui]; m_ppcYuvResi[ui] = NULL;
    m_ppcYuvReco[ui]->destroy(); delete m_ppcYuvReco[ui]; m_ppcYuvReco[ui] = NULL;
    m_ppcCU     [ui]->destroy(); delete m_ppcCU     [ui]; m_ppcCU     [ui] = NULL;
  }

  delete [] m_ppcYuvPred; m_ppcYuvPred = NULL;
  delete [] m_ppcYuvResi; m_ppcYuvResi = NULL;
  delete [] m_ppcYuvReco; m_ppcYuvReco = NULL;
  delete [] m_ppcCU     ; m_ppcCU      = NULL;
}

Void TDecCu::decodeCU( TComDataCU* pcCU, UInt& ruiIsLast )
{
  TComSlice*  pcSlice = pcCU->getSlice();

  xDecodeCU( pcCU, 0, 0 );

  // dQP: only for LCU
	if ( pcCU->getSlice()->getSPS()->getUseDQP() )
	{
		if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
		{
		}
		else
		{
			m_pcEntropyDecoder->decodeQP( pcCU, 0, 0 );
		}
	}

  //--- Read terminating bit ---
  m_pcEntropyDecoder->decodeTerminatingBit( ruiIsLast );
}

Void TDecCu::xDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts      = uiCurNumParts>>2;

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
  {
	  m_pcEntropyDecoder->decodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth ) ) || bBoundary )
  {
    UInt uiIdx = uiAbsPartIdx;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xDecodeCU( pcCU, uiIdx, uiDepth+1 );

      uiIdx += uiQNumParts;
    }

    return;
  }

  m_pcEntropyDecoder->decodeAlfCtrlFlag( pcCU, uiAbsPartIdx, uiDepth );
  // decode CU mode and the partition size
  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyDecoder->decodeSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
  }

  if( pcCU->isSkip( uiAbsPartIdx ) )
  {
		if ( pcCU->getSlice()->getSPS()->getUseAMVP() )
		{
      pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);
			pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);

			pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);
			pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);

			if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
			{
				m_pcEntropyDecoder->decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_0, m_ppcCU[uiDepth]);
			}

			if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
			{
				m_pcEntropyDecoder->decodeMVPIdx( pcCU, uiAbsPartIdx, uiDepth, REF_PIC_LIST_1, m_ppcCU[uiDepth]);
			}
		}
    return;
  }

  m_pcEntropyDecoder->decodePredMode( pcCU, uiAbsPartIdx, uiDepth );
  m_pcEntropyDecoder->decodePartSize( pcCU, uiAbsPartIdx, uiDepth );


  UInt uiCurrWidth      = pcCU->getWidth ( uiAbsPartIdx );
  UInt uiCurrHeight     = pcCU->getHeight( uiAbsPartIdx );

  // prediction mode ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyDecoder->decodePredInfo( pcCU, uiAbsPartIdx, uiDepth, m_ppcCU[uiDepth]);

  // Coefficient decoding
  m_pcEntropyDecoder->decodeCoeff( pcCU, uiAbsPartIdx, uiDepth, uiCurrWidth, uiCurrHeight );

	// ROT index
	if ( pcCU->getSlice()->getSPS()->getUseROT() )
	{
	  m_pcEntropyDecoder->decodeROTIdx( pcCU, uiAbsPartIdx, uiDepth );
	}

	// CIP flag
	if ( pcCU->getSlice()->getSPS()->getUseCIP() )
	{
	  m_pcEntropyDecoder->decodeCIPflag( pcCU, uiAbsPartIdx, uiDepth );
	}
}

Void TDecCu::xDecompressCU( TComDataCU* pcCU, TComDataCU* pcCUCur, UInt uiAbsPartIdx,  UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth ) ) || bBoundary )
  {
    UInt uiNextDepth = uiDepth + 1;
    UInt uiQNumParts = pcCU->getTotalNumPart() >> (uiNextDepth<<1);
    UInt uiIdx = uiAbsPartIdx;
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++ )
    {
      uiLPelX = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiIdx] ];
      uiTPelY = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xDecompressCU(pcCU, m_ppcCU[uiNextDepth], uiIdx, uiNextDepth );

      uiIdx += uiQNumParts;
    }
    return;
  }

  // Residual reconstruction
  m_ppcYuvResi[uiDepth]->clear();

  m_ppcCU[uiDepth]->copySubCU( pcCU, uiAbsPartIdx, uiDepth );

  switch( m_ppcCU[uiDepth]->getPredictionMode(0) )
  {
  case MODE_SKIP:
  case MODE_INTER:
    xReconInter( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
    break;
  case MODE_INTRA:
    xReconIntra(m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth);
    break;
  default:
    assert(0);
    break;
  }

  if ( !pcCU->getSlice()->getSPS()->getUseAMVP() )
  {
    pcCU->getCUMvField(REF_PIC_LIST_0)->copyFrom(m_ppcCU[uiDepth]->getCUMvField(REF_PIC_LIST_0), m_ppcCU[uiDepth]->getTotalNumPart(), uiAbsPartIdx);
    pcCU->getCUMvField(REF_PIC_LIST_1)->copyFrom(m_ppcCU[uiDepth]->getCUMvField(REF_PIC_LIST_1), m_ppcCU[uiDepth]->getTotalNumPart(), uiAbsPartIdx);
  }

  xCopyToPic( m_ppcCU[uiDepth], pcPic, uiAbsPartIdx, uiDepth );
}

Void TDecCu::decompressCU( TComDataCU* pcCU )
{
  xDecompressCU( pcCU, pcCU, 0,  0 );
}

Void TDecCu::xReconInter( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

  // inter prediction
	if ( !pcCU->getSlice()->getSPS()->getUseAMVP() )
	{
		m_pcPrediction->calcMotionVector  ( pcCU );
	}

  m_pcPrediction->motionCompensation( pcCU, m_ppcYuvPred[uiDepth] );

  // inter recon
  xDecodeInterTexture( pcCU, 0, uiDepth );

	// clip for only non-zero cbp case
	if  ((pcCU->getCbf(0, TEXT_LUMA))||(pcCU->getCbf(0, TEXT_CHROMA_U))||(pcCU->getCbf(0, TEXT_CHROMA_V)))
	{
		m_ppcYuvReco[uiDepth]->addClip( m_ppcYuvPred[uiDepth], m_ppcYuvResi[uiDepth], 0, pcCU->getWidth( 0 ) );
	}
	else
	{
		m_ppcYuvPred[uiDepth]->copyPartToPartYuv( m_ppcYuvReco[uiDepth],0, pcCU->getWidth( 0 ),pcCU->getHeight( 0 ));
	}
}

Void TDecCu::xDecodeIntraTexture( TComDataCU* pcCU, UInt uiPartIdx, Pel* piReco, Pel* piPred, Pel* piResi, UInt uiStride, TCoeff* pCoeff, UInt uiWidth, UInt uiHeight, UInt uiCurrDepth, UInt indexROT, UInt indexMPI )
{
  if( pcCU->getTransformIdx(0) == uiCurrDepth )
  {
    UInt uiX, uiY;
    TComPattern* pcPattern = pcCU->getPattern();
    UInt uiZorder          = pcCU->getZorderIdxInCU()+uiPartIdx;
    Pel* pPred             = piPred;
    Pel* pResi             = piResi;
    Pel* piPicReco         = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZorder);
    UInt uiPicStride       = pcCU->getPic()->getPicYuvRec()->getStride();

    pcPattern->initPattern( pcCU, uiCurrDepth, uiPartIdx );

    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;

    if( pcCU->getSlice()->getSPS()->getUseADI() )
    {
      pcPattern->initAdiPattern(pcCU, uiPartIdx, uiCurrDepth, m_pcPrediction->getPredicBuf(), m_pcPrediction->getPredicBufWidth(), m_pcPrediction->getPredicBufHeight(), bAboveAvail, bLeftAvail);
      m_pcPrediction->predIntraLumaNxNAdi( pcPattern, pcCU->getLumaIntraDir(uiPartIdx), pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail, indexMPI );
    }
    else
      m_pcPrediction->predIntraLumaNxN   ( pcPattern, pcCU->getLumaIntraDir(uiPartIdx), pPred, uiStride, uiWidth, uiHeight, indexMPI);

    m_pcTrQuant->setQPforQuant( pcCU->getQP(uiPartIdx), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
    m_pcTrQuant->invtransformNxN( pResi, uiStride, pCoeff, uiWidth, uiHeight, indexROT );

    // Reconstruction
    if ( pcCU->getSlice()->getSPS()->getUseCIP() && pcCU->getCIPflag( uiPartIdx ) )
    {
      m_pcPrediction->recIntraLumaNxNCIP( pcCU->getPattern(), piPred, piResi, piReco, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );

      // update to picture
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          piPicReco[uiX] = piReco[uiX];
        }
        piReco += uiStride;
        piPicReco += uiPicStride;
      }
    }
    else
    {
      pResi = piResi;
      pPred = piPred;
      for( uiY = 0; uiY < uiHeight; uiY++ )
      {
        for( uiX = 0; uiX < uiWidth; uiX++ )
        {
          piReco   [uiX] = Clip(pPred[uiX] + pResi[uiX]);
          piPicReco[uiX] = piReco[uiX];
        }
        piReco    += uiStride;
        pPred     += uiStride;
        pResi     += uiStride;
        piPicReco += uiPicStride;
      }
    }
  }
  else
  {
    uiCurrDepth++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiPartOffset  = pcCU->getTotalNumPart()>>(uiCurrDepth<<1);
    UInt uiCoeffOffset = uiWidth  * uiHeight;
    UInt uiPelOffset   = uiHeight * uiStride;
    Pel* pResi = piResi;
    Pel* pReco = piReco;
    Pel* pPred = piPred;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth, indexROT, indexMPI );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiWidth;
    pReco      = piReco + uiWidth;
    pPred      = piPred + uiWidth;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth, indexROT, indexMPI );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiPelOffset;
    pReco      = piReco + uiPelOffset;
    pPred      = piPred + uiPelOffset;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth, indexROT, indexMPI );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiPelOffset + uiWidth;
    pReco      = piReco + uiPelOffset + uiWidth;
    pPred      = piPred + uiPelOffset + uiWidth;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth, indexROT, indexMPI );
  }
}


Void TDecCu::xRecurIntraInvTrans(TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piResi, Pel* piPred, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiWidth, UInt uiHeight, UInt uiCurrTrMode, UInt indexROT )
{
  if( pcCU->getTransformIdx(0) == uiCurrTrMode )
  {
    UInt uiX, uiY;
    UInt uiZorder    = pcCU->getZorderIdxInCU()+uiAbsPartIdx;
    Pel* pResi       = piResi;
    Pel* pPred       = piPred;
    Pel* piPicReco   = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), uiZorder);
    UInt uiPicStride = pcCU->getPic()->getPicYuvRec()->getStride();

    m_pcTrQuant->setQPforQuant( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
    m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );

    pResi = piResi;
    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        piReco   [uiX] = Clip(pPred[uiX]+pResi[uiX]);
        piPicReco[uiX] = piReco[uiX];
      }

      pPred     += uiStride;
      pResi     += uiStride;
      piReco    += uiStride;
      piPicReco += uiPicStride;
    }
  }
  else
  {
    uiCurrTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiCoeffOffset = uiWidth*uiHeight;
    UInt uiPelOffset   = uiHeight*uiStride;
    UInt uiPartOffst   = pcCU->getTotalNumPart()>>(uiCurrTrMode<<1);
    Pel* pResi = piResi;
    Pel* pPred = piPred;
    Pel* pReco = piReco;
    xRecurIntraInvTrans( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiCurrTrMode, indexROT );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiWidth; pPred = piPred + uiWidth; pReco = piReco + uiWidth;
    xRecurIntraInvTrans( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiCurrTrMode, indexROT );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset; pPred = piPred + uiPelOffset; pReco = piReco + uiPelOffset;
    xRecurIntraInvTrans( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiCurrTrMode, indexROT );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset + uiWidth; pPred = piPred + uiPelOffset + uiWidth; pReco = piReco + uiPelOffset + uiWidth;
    xRecurIntraInvTrans( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiCurrTrMode, indexROT );
  }
}



Void TDecCu::xRecurIntraInvTransChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piResi, Pel* piPred, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiWidth, UInt uiHeight, UInt uiTrMode, UInt uiCurrTrMode, TextType eText )
{
  if( uiTrMode == uiCurrTrMode )
  {
    UInt uiX, uiY;
    UInt uiZorder    = pcCU->getZorderIdxInCU()+uiAbsPartIdx;
    Pel* pResi       = piResi;
    Pel* pPred       = piPred;
    Pel* piPicReco;
    if( eText == TEXT_CHROMA_U )
      piPicReco= pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), uiZorder);
    else
      piPicReco= pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), uiZorder);

    UInt uiPicStride = pcCU->getPic()->getPicYuvRec()->getCStride();

    UChar indexROT = pcCU->getROTindex(0);
    pcCU->getPattern()->initPattern( pcCU, uiCurrTrMode, uiAbsPartIdx );

    // CCCP
    if ( pcCU->getSlice()->getSPS()->getUseCCP() && pcCU->getChromaIntraDir(0) == CCCP_MODE)
      m_pcPrediction->predCCCPIntraChroma( pcCU->getPattern(), piPicReco, uiPicStride, pPred, uiStride, uiWidth, uiHeight, CCCP_BOUNDARY_EXT >> 1 );
    else
      m_pcPrediction->predIntraChromaNxN( pcCU->getPattern(), piPicReco, uiPicStride, pcCU->getChromaIntraDir(0), pPred, uiStride, uiWidth, uiHeight );

    // Inverse Transform
    if( pcCU->getCbf(0, eText, uiCurrTrMode) )
    {
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
    }

    pResi = piResi;

    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        piReco   [uiX] = Clip(pPred[uiX]+pResi[uiX]);
        piPicReco[uiX] = piReco[uiX];
      }

      pPred     += uiStride;
      pResi     += uiStride;
      piReco    += uiStride;
      piPicReco += uiPicStride;
    }
  }
  else
  {
    uiCurrTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiCoeffOffset = uiWidth*uiHeight;
    UInt uiPelOffset   = uiHeight*uiStride;
    UInt uiPartOffst   = pcCU->getTotalNumPart()>>(uiCurrTrMode<<1);
    Pel* pResi = piResi;
    Pel* pPred = piPred;
    Pel* pReco = piReco;
    xRecurIntraInvTransChroma( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiWidth; pPred = piPred + uiWidth; pReco = piReco + uiWidth;
    xRecurIntraInvTransChroma( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset; pPred = piPred + uiPelOffset; pReco = piReco + uiPelOffset;
    xRecurIntraInvTransChroma( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset + uiWidth; pPred = piPred + uiPelOffset + uiWidth; pReco = piReco + uiPelOffset + uiWidth;
    xRecurIntraInvTransChroma( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
  }
}

// ADI_CHROMA --
Void TDecCu::xRecurIntraInvTransChromaADI(TComDataCU* pcCU, UInt uiAbsPartIdx, Pel* piResi, Pel* piPred, Pel* piReco, UInt uiStride, TCoeff* piCoeff, UInt uiWidth, UInt uiHeight, UInt uiTrMode, UInt uiCurrTrMode, TextType eText )
{
  if( uiTrMode == uiCurrTrMode )
  {
    UInt uiX, uiY;
    UInt uiZorder    = pcCU->getZorderIdxInCU()+uiAbsPartIdx;
    Pel* pResi       = piResi;
    Pel* pPred       = piPred;
    Pel* piPicReco;
    if( eText == TEXT_CHROMA_U )
      piPicReco= pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), uiZorder);
    else
      piPicReco= pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), uiZorder);

    UInt uiPicStride = pcCU->getPic()->getPicYuvRec()->getCStride();

    UChar indexROT = pcCU->getROTindex(0);

    pcCU->getPattern()->initPattern( pcCU, uiCurrTrMode, uiAbsPartIdx );

    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;

    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, m_pcPrediction->getPredicBuf(),m_pcPrediction->getPredicBufWidth(),m_pcPrediction->getPredicBufHeight(),bAboveAvail,bLeftAvail);

    Int  iIntraIdx      = pcCU->getIntraSizeIdx(0);
    UInt uiModeL        = g_aucIntraModeOrder[iIntraIdx][pcCU->getLumaIntraDir(0)];

// TM_INTRA
    if (pcCU->getSlice()->getSPS()->getUseTMI())
      if (pcCU->getLumaIntraDir(0)==33) uiModeL=33;

    UInt uiMode     =     pcCU->getChromaIntraDir(0);
    if (uiMode==4)
       uiMode = uiModeL;

    Int*   pPatChr;

    if (eText==TEXT_CHROMA_U)
      pPatChr=  pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() );
    else // (eText==TEXT_CHROMA_V)
      pPatChr=  pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() );

    if ( pcCU->getSlice()->getSPS()->getUseCCP() && uiMode == CCCP_ADI_MODE )
    {
      m_pcPrediction-> predCCCPIntraChroma( pcCU->getPattern(), piPicReco, uiPicStride, pPred, uiStride, uiWidth, uiHeight, CCCP_BOUNDARY_EXT >> 1 );
    }
    else
    {
       m_pcPrediction-> predIntraChromaAdi( pcCU->getPattern(),pPatChr,uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail, piPicReco, uiPicStride);
    }

    // Inverse Transform
    if( pcCU->getCbf(0, eText, uiCurrTrMode) )
    {
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight, indexROT );
    }

    pResi = piResi;

    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        piReco   [uiX] = Clip(pPred[uiX]+pResi[uiX]);
        piPicReco[uiX] = piReco[uiX];
      }

      pPred     += uiStride;
      pResi     += uiStride;
      piReco    += uiStride;
      piPicReco += uiPicStride;
    }
  }
  else
  {
    uiCurrTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiCoeffOffset = uiWidth*uiHeight;
    UInt uiPelOffset   = uiHeight*uiStride;
    UInt uiPartOffst   = pcCU->getTotalNumPart()>>(uiCurrTrMode<<1);
    Pel* pResi = piResi;
    Pel* pPred = piPred;
    Pel* pReco = piReco;
    xRecurIntraInvTransChromaADI( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiWidth; pPred = piPred + uiWidth; pReco = piReco + uiWidth;
    xRecurIntraInvTransChromaADI( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset; pPred = piPred + uiPelOffset; pReco = piReco + uiPelOffset;
    xRecurIntraInvTransChromaADI( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
    uiAbsPartIdx += uiPartOffst;
    piCoeff      += uiCoeffOffset;
    pResi         = piResi + uiPelOffset + uiWidth; pPred = piPred + uiPelOffset + uiWidth; pReco = piReco + uiPelOffset + uiWidth;
    xRecurIntraInvTransChromaADI( pcCU, uiAbsPartIdx, pResi, pPred, pReco, uiStride, piCoeff, uiWidth, uiHeight, uiTrMode, uiCurrTrMode, eText );
  }
}
// -- ADI_CHROMA

Void TDecCu::xReconIntra( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiPU;
  UInt uiPartDepth = pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1;
  UInt uiNumPart   = pcCU->getNumPartInter();

  UInt uiPartIdx;
  UInt uiQNumParts = pcCU->getTotalNumPart() >> 2;
  UInt uiCoeffOffset;

  TComPattern* pcPattern  = pcCU->getPattern();
  Pel* pY;
  UInt uiWidth  = pcCU->getWidth (0) >> uiPartDepth;
  UInt uiHeight = pcCU->getHeight(0) >> uiPartDepth;

  UInt uiPartOffset = uiWidth*uiHeight;

  TCoeff* pCoeff;
  Pel* piResi;
  Pel* piPred;
  Pel* piReco;
  UInt uiStride = m_ppcYuvResi[uiDepth]->getStride();

  uiPartIdx     = 0;
  uiCoeffOffset = 0;

  UInt uiWidthBit = pcCU->getIntraSizeIdx(0);

  // Luma
  for( uiPU = 0 ; uiPU < uiNumPart; uiPU++ )
  {
    pCoeff    = pcCU->getCoeffY() + uiCoeffOffset;
    pY        = m_ppcYuvResi[uiDepth]->getLumaAddr(uiPU, uiWidth);
    piPred    = m_ppcYuvPred[uiDepth]->getLumaAddr(uiPU, uiWidth);
    piReco    = m_ppcYuvReco[uiDepth]->getLumaAddr(uiPU, uiWidth);
    piResi    = pY;

    UInt indexROT = pcCU->getROTindex(0) ;
    UInt indexMPI = Optimal_MPI_index[uiWidthBit][g_aucIntraModeConv[uiWidthBit][pcCU->getLumaIntraDir(uiPartIdx)]][pcCU->getMPIindex(uiPartIdx)];

    xDecodeIntraTexture( pcCU, uiPartIdx, piReco, piPred, piResi, uiStride, pCoeff, uiWidth, uiHeight, uiPartDepth, indexROT, indexMPI );

    uiPartIdx     += uiQNumParts;
    uiCoeffOffset += uiPartOffset;
  } // PU loop

  // Cb and Cr
  Pel* pResiCb = m_ppcYuvResi[uiDepth]->getCbAddr();
  Pel* pResiCr = m_ppcYuvResi[uiDepth]->getCrAddr();
  Pel* pPredCb = m_ppcYuvPred[uiDepth]->getCbAddr();
  Pel* pPredCr = m_ppcYuvPred[uiDepth]->getCrAddr();
  Pel* pRecoCb = m_ppcYuvReco[uiDepth]->getCbAddr();
  Pel* pRecoCr = m_ppcYuvReco[uiDepth]->getCrAddr();
  UInt uiCStride = m_ppcYuvReco[uiDepth]->getCStride();
  TCoeff* pCoefCb = pcCU->getCoeffCb();
  TCoeff* pCoefCr = pcCU->getCoeffCr();
  UInt uiCWidth  = pcCU->getWidth (0)>>1;
  UInt uiCHeight = pcCU->getHeight(0)>>1;

  UInt uiChromaTrMode = 0;
  if( !pcCU->getSlice()->getSPS()->getUseLOT() && (uiCWidth > pcCU->getSlice()->getSPS()->getMaxTrSize()) )
  {
    while( uiCWidth > (pcCU->getSlice()->getSPS()->getMaxTrSize()<<uiChromaTrMode) ) uiChromaTrMode++;
  }

  m_pcTrQuant->setQPforQuant( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_CHROMA );

  if( pcCU->getSlice()->getSPS()->getUseADI() )
  {
    xRecurIntraInvTransChromaADI( pcCU, 0, pResiCb, pPredCb, pRecoCb, uiCStride, pCoefCb, uiCWidth, uiCHeight, uiChromaTrMode, 0, TEXT_CHROMA_U );
    xRecurIntraInvTransChromaADI( pcCU, 0, pResiCr, pPredCr, pRecoCr, uiCStride, pCoefCr, uiCWidth, uiCHeight, uiChromaTrMode, 0, TEXT_CHROMA_V );
  }
  else
  {
    xRecurIntraInvTransChroma( pcCU, 0, pResiCb, pPredCb, pRecoCb, uiCStride, pCoefCb, uiCWidth, uiCHeight, uiChromaTrMode, 0, TEXT_CHROMA_U );
    xRecurIntraInvTransChroma( pcCU, 0, pResiCr, pPredCr, pRecoCr, uiCStride, pCoefCr, uiCWidth, uiCHeight, uiChromaTrMode, 0, TEXT_CHROMA_V );
  }
}

Void TDecCu::xCopyToPic( TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth )
{
  UInt uiCUAddr = pcCU->getAddr();

  m_ppcYuvReco[uiDepth]->copyToPicYuv  ( pcPic->getPicYuvRec (), uiCUAddr, uiZorderIdx );

	if ( !pcCU->getSlice()->getSPS()->getUseAMVP() )
	{
		if ( !pcCU->isIntra( 0 ) )
		{
			TComDataCU* pcPicCU = pcCU->getPic()->getCU( uiCUAddr );
			pcCU->getCUMvField( REF_PIC_LIST_0 )->copyMvTo( pcPicCU->getCUMvField( REF_PIC_LIST_0 ), uiZorderIdx );
			pcCU->getCUMvField( REF_PIC_LIST_1 )->copyMvTo( pcPicCU->getCUMvField( REF_PIC_LIST_1 ), uiZorderIdx );
		}
	}

  return;
}

Void TDecCu::xCopyToPicLuma( TComPic* pcPic, UInt uiCUAddr, UInt uiZorderIdx, UInt uiDepth )
{
  m_ppcYuvReco[uiDepth]->copyToPicLuma( pcPic->getPicYuvRec (), uiCUAddr, uiZorderIdx );
}

Void TDecCu::xCopyToPicChroma( TComPic* pcPic, UInt uiCUAddr, UInt uiZorderIdx, UInt uiDepth )
{
  m_ppcYuvReco[uiDepth]->copyToPicChroma( pcPic->getPicYuvRec (), uiCUAddr, uiZorderIdx );
}

Void TDecCu::xDecodeInterTexture ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt    uiWidth    = pcCU->getWidth ( uiAbsPartIdx );
  UInt    uiHeight   = pcCU->getHeight( uiAbsPartIdx );
  UInt    uiCWidth   = uiWidth >>1;
  UInt    uiCHeight  = uiHeight>>1;
  TCoeff* piCoeff;

  Pel*    pResi;
  UInt    uiLumaTrMode, uiChromaTrMode;

  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );

	UChar indexROT = pcCU->getROTindex(0);

  // Y
  piCoeff = pcCU->getCoeffY();
  pResi = m_ppcYuvResi[uiDepth]->getLumaAddr();
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_LUMA, pResi, 0, m_ppcYuvResi[uiDepth]->getStride(), uiWidth, uiHeight, uiLumaTrMode, 0, piCoeff, indexROT );

  // Cb and Cr
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_CHROMA );

  uiWidth  >>= 1;
  uiHeight >>= 1;
  piCoeff = pcCU->getCoeffCb(); pResi = m_ppcYuvResi[uiDepth]->getCbAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_U, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff, indexROT );
  piCoeff = pcCU->getCoeffCr(); pResi = m_ppcYuvResi[uiDepth]->getCrAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_V, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff, indexROT );
}
