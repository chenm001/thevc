/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.   
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
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

/** \file     TDecCu.cpp
    \brief    CU decoder class
*/

#include "TDecCu.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCu::TDecCu()
{
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

/**
 \param    uiMaxDepth    total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 */
Void TDecCu::create( UInt uiMaxDepth, UInt uiMaxWidth, UInt uiMaxHeight )
{
  m_uiMaxDepth = uiMaxDepth+1;
  
  m_ppcYuvResi = new TComYuv*[m_uiMaxDepth-1];
  m_ppcYuvReco = new TComYuv*[m_uiMaxDepth-1];
  m_ppcCU      = new TComDataCU*[m_uiMaxDepth-1];
  
  UInt uiNumPartitions;
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    uiNumPartitions = 1<<( ( m_uiMaxDepth - ui - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> ui;
    UInt uiHeight = uiMaxHeight >> ui;
    
    m_ppcYuvResi[ui] = new TComYuv;    m_ppcYuvResi[ui]->create( uiWidth, uiHeight );
    m_ppcYuvReco[ui] = new TComYuv;    m_ppcYuvReco[ui]->create( uiWidth, uiHeight );
    m_ppcCU     [ui] = new TComDataCU; m_ppcCU     [ui]->create( uiNumPartitions, uiWidth, uiHeight, true );
  }
  
  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster(m_uiMaxDepth, 1, 0, piTmp);
  initRasterToZscan( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
  
  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
}

Void TDecCu::destroy()
{
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    m_ppcYuvResi[ui]->destroy(); delete m_ppcYuvResi[ui]; m_ppcYuvResi[ui] = NULL;
    m_ppcYuvReco[ui]->destroy(); delete m_ppcYuvReco[ui]; m_ppcYuvReco[ui] = NULL;
    m_ppcCU     [ui]->destroy(); delete m_ppcCU     [ui]; m_ppcCU     [ui] = NULL;
  }
  
  delete [] m_ppcYuvResi; m_ppcYuvResi = NULL;
  delete [] m_ppcYuvReco; m_ppcYuvReco = NULL;
  delete [] m_ppcCU     ; m_ppcCU      = NULL;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param    pcCU        pointer of CU data
 \param    ruiIsLast   last data?
 */
Void TDecCu::decodeCU( TComDataCU* pcCU, UInt& ruiIsLast )
{
#if SNY_DQP   
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    pcCU->setdQPFlag(true); 
  }
#endif//SNY_DQP
  // start from the top level CU
  xDecodeCU( pcCU, 0, 0 );
  
#if SNY_DQP 
  // dQP: only for LCU
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
    {
    }
    else if ( pcCU->getdQPFlag())// non-skip
    {
      m_pcEntropyDecoder->decodeQP( pcCU, 0, 0 );
      pcCU->setdQPFlag(false);
    }
  }
#else
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
#endif//SNY_DQP
  
  //--- Read terminating bit ---
  m_pcEntropyDecoder->decodeTerminatingBit( ruiIsLast );
}

/** \param    pcCU        pointer of CU data
 */
Void TDecCu::decompressCU( TComDataCU* pcCU )
{
  xDecompressCU( pcCU, pcCU, 0,  0 );
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/** decode CU block recursively
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth 
 * \returns Void
 */
Void TDecCu::xDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts      = uiCurNumParts>>2;
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
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
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xDecodeCU( pcCU, uiIdx, uiDepth+1 );
      
      uiIdx += uiQNumParts;
    }
    
    return;
  }
  
#if TSB_ALF_HEADER
#else
  m_pcEntropyDecoder->decodeAlfCtrlFlag( pcCU, uiAbsPartIdx, uiDepth );
#endif
  
  // decode CU mode and the partition size
  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyDecoder->decodeSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  

  if( pcCU->isSkipped(uiAbsPartIdx) )
  {
#if HHI_MRG_SKIP
    m_ppcCU[uiDepth]->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
    m_ppcCU[uiDepth]->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
    TComMvField cMvFieldNeighbours[MRG_MAX_NUM_CANDS << 1]; // double length for mv of both lists
    UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
    UInt uiNeighbourCandIdx[MRG_MAX_NUM_CANDS]; //MVs with same idx => same cand
    for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
    {
      uhInterDirNeighbours[ui] = 0;
      uiNeighbourCandIdx[ui] = 0;
    }
    m_ppcCU[uiDepth]->getInterMergeCandidates( 0, 0, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourCandIdx );
    for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
  {
      pcCU->setNeighbourCandIdxSubParts( uiIter, uiNeighbourCandIdx[uiIter], uiAbsPartIdx, 0, uiDepth );
    }
    m_pcEntropyDecoder->decodeMergeIndex( pcCU, 0, uiAbsPartIdx, SIZE_2Nx2N, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
#else
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
#endif
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
  
}

Void TDecCu::xDecompressCU( TComDataCU* pcCU, TComDataCU* pcCUCur, UInt uiAbsPartIdx,  UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
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
      uiLPelX = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
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
      xReconIntraQT( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
      break;
    default:
      assert(0);
      break;
  }
  
  xCopyToPic( m_ppcCU[uiDepth], pcPic, uiAbsPartIdx, uiDepth );
}

Void TDecCu::xReconInter( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  
  // inter prediction
  m_pcPrediction->motionCompensation( pcCU, m_ppcYuvReco[uiDepth] );
  
  // inter recon
  xDecodeInterTexture( pcCU, 0, uiDepth );
  
  // clip for only non-zero cbp case
  if  ( ( pcCU->getCbf( 0, TEXT_LUMA ) ) || ( pcCU->getCbf( 0, TEXT_CHROMA_U ) ) || ( pcCU->getCbf(0, TEXT_CHROMA_V ) ) )
  {
    m_ppcYuvReco[uiDepth]->addClip( m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth], 0, pcCU->getWidth( 0 ) );
  }
  else
  {
    m_ppcYuvReco[uiDepth]->copyPartToPartYuv( m_ppcYuvReco[uiDepth],0, pcCU->getWidth( 0 ),pcCU->getHeight( 0 ));
  }
}

Void TDecCu::xDecodeIntraTexture( TComDataCU* pcCU, UInt uiPartIdx, Pel* piReco, Pel* piPred, Pel* piResi, UInt uiStride, TCoeff* pCoeff, UInt uiWidth, UInt uiHeight, UInt uiCurrDepth )
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
    
    pcPattern->initAdiPattern(pcCU, uiPartIdx, uiCurrDepth, m_pcPrediction->getPredicBuf(), m_pcPrediction->getPredicBufWidth(), m_pcPrediction->getPredicBufHeight(), bAboveAvail, bLeftAvail);
    
    m_pcPrediction->predIntraLumaAng( pcPattern, pcCU->getLumaIntraDir(uiPartIdx), pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    
    m_pcTrQuant->setQPforQuant( pcCU->getQP(uiPartIdx), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
    m_pcTrQuant->invtransformNxN( pResi, uiStride, pCoeff, uiWidth, uiHeight );
    // Reconstruction
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
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiWidth;
    pReco      = piReco + uiWidth;
    pPred      = piPred + uiWidth;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiPelOffset;
    pReco      = piReco + uiPelOffset;
    pPred      = piPred + uiPelOffset;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth );
    uiPartIdx += uiPartOffset;
    pCoeff    += uiCoeffOffset;
    pResi      = piResi + uiPelOffset + uiWidth;
    pReco      = piReco + uiPelOffset + uiWidth;
    pPred      = piPred + uiPelOffset + uiWidth;
    xDecodeIntraTexture( pcCU, uiPartIdx, pReco, pPred, pResi, uiStride, pCoeff, uiWidth, uiHeight, uiCurrDepth );
  }
}

// ADI chroma
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
    
    pcCU->getPattern()->initPattern( pcCU, uiCurrTrMode, uiAbsPartIdx );
    
    Bool bAboveAvail = false;
    Bool bLeftAvail  = false;
    
    pcCU->getPattern()->initAdiPatternChroma(pcCU,uiAbsPartIdx, uiCurrTrMode, m_pcPrediction->getPredicBuf(),m_pcPrediction->getPredicBufWidth(),m_pcPrediction->getPredicBufHeight(),bAboveAvail,bLeftAvail);
    
    UInt uiModeL        = pcCU->getLumaIntraDir(0);
    UInt uiMode         = pcCU->getChromaIntraDir(0);
    
    if (uiMode==4) uiMode = uiModeL;
    
    Int*   pPatChr;
    
    if (eText==TEXT_CHROMA_U)
    {
      pPatChr=  pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() );
    }
    else // (eText==TEXT_CHROMA_V)
    {
      pPatChr=  pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() );
    }
    
    m_pcPrediction-> predIntraChromaAng( pcCU->getPattern(), pPatChr, uiMode, pPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
    
    // Inverse Transform
    if( pcCU->getCbf(0, eText, uiCurrTrMode) )
    {
      m_pcTrQuant->invtransformNxN( pResi, uiStride, piCoeff, uiWidth, uiHeight );
    }
    
    pResi = piResi;
    
    for( uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( uiX = 0; uiX < uiWidth; uiX++ )
      {
        piReco   [uiX] = Clip( pPred[uiX] + pResi[uiX] );
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

Void
TDecCu::xIntraRecLumaBlk( TComDataCU* pcCU,
                         UInt        uiTrDepth,
                         UInt        uiAbsPartIdx,
                         TComYuv*    pcRecoYuv,
                         TComYuv*    pcPredYuv, 
                         TComYuv*    pcResiYuv )
{
  UInt    uiWidth           = pcCU     ->getWidth   ( 0 ) >> uiTrDepth;
  UInt    uiHeight          = pcCU     ->getHeight  ( 0 ) >> uiTrDepth;
  UInt    uiStride          = pcRecoYuv->getStride  ();
  Pel*    piReco            = pcRecoYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piPred            = pcPredYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piResi            = pcResiYuv->getLumaAddr( uiAbsPartIdx );
  
  UInt    uiNumCoeffInc     = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
  TCoeff* pcCoeff           = pcCU->getCoeffY() + ( uiNumCoeffInc * uiAbsPartIdx );
  
  UInt    uiLumaPredMode    = pcCU->getLumaIntraDir     ( uiAbsPartIdx );
  
  UInt    uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*    piRecIPred        = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiZOrder );
  UInt    uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getStride  ();
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern   ( pcCU, uiTrDepth, uiAbsPartIdx );
  pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiTrDepth, 
                                     m_pcPrediction->getPredicBuf       (),
                                     m_pcPrediction->getPredicBufWidth  (),
                                     m_pcPrediction->getPredicBufHeight (),
                                     bAboveAvail, bLeftAvail );
  
  //===== get prediction signal =====
  m_pcPrediction->predIntraLumaAng( pcCU->getPattern(), uiLumaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
  
  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant  ( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
  m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight );
  
  //===== reconstruction =====
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecIPred += uiRecIPredStride;
    }
  }
}


Void
TDecCu::xIntraRecChromaBlk( TComDataCU* pcCU,
                           UInt        uiTrDepth,
                           UInt        uiAbsPartIdx,
                           TComYuv*    pcRecoYuv,
                           TComYuv*    pcPredYuv, 
                           TComYuv*    pcResiYuv,
                           UInt        uiChromaId )
{
  UInt uiFullDepth  = pcCU->getDepth( 0 ) + uiTrDepth;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( uiTrDepth > 0 );
    uiTrDepth--;
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth ) << 1 );
    Bool bFirstQ = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    if( !bFirstQ )
    {
      return;
    }
  }
  
  TextType  eText             = ( uiChromaId > 0 ? TEXT_CHROMA_V : TEXT_CHROMA_U );
  UInt      uiWidth           = pcCU     ->getWidth   ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiHeight          = pcCU     ->getHeight  ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiStride          = pcRecoYuv->getCStride ();
  Pel*      piReco            = ( uiChromaId > 0 ? pcRecoYuv->getCrAddr( uiAbsPartIdx ) : pcRecoYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piPred            = ( uiChromaId > 0 ? pcPredYuv->getCrAddr( uiAbsPartIdx ) : pcPredYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piResi            = ( uiChromaId > 0 ? pcResiYuv->getCrAddr( uiAbsPartIdx ) : pcResiYuv->getCbAddr( uiAbsPartIdx ) );
  
  UInt      uiNumCoeffInc     = ( ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 ) ) >> 2;
  TCoeff*   pcCoeff           = ( uiChromaId > 0 ? pcCU->getCoeffCr() : pcCU->getCoeffCb() ) + ( uiNumCoeffInc * uiAbsPartIdx );
  
  UInt      uiChromaPredMode  = pcCU->getChromaIntraDir( 0 );
  
#if !LM_CHROMA
  if( uiChromaPredMode == 4 )
  {
    uiChromaPredMode          = pcCU->getLumaIntraDir( 0 );
  }
#endif
  
  UInt      uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*      piRecIPred        = ( uiChromaId > 0 ? pcCU->getPic()->getPicYuvRec()->getCrAddr( pcCU->getAddr(), uiZOrder ) : pcCU->getPic()->getPicYuvRec()->getCbAddr( pcCU->getAddr(), uiZOrder ) );
  UInt      uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getCStride();
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern         ( pcCU, uiTrDepth, uiAbsPartIdx );
  pcCU->getPattern()->initAdiPatternChroma( pcCU, uiAbsPartIdx, uiTrDepth, 
                                           m_pcPrediction->getPredicBuf       (),
                                           m_pcPrediction->getPredicBufWidth  (),
                                           m_pcPrediction->getPredicBufHeight (),
                                           bAboveAvail, bLeftAvail );
  Int* pPatChroma   = ( uiChromaId > 0 ? pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() ) : pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() ) );
  
  //===== get prediction signal =====
#if LM_CHROMA
  if(pcCU->getSlice()->getSPS()->getUseLMChroma() && uiChromaPredMode == 3)
  {
      m_pcPrediction->predLMIntraChroma( pcCU->getPattern(), pPatChroma, piPred, uiStride, uiWidth, uiHeight, uiChromaId );
  }
  else
  {
    if( uiChromaPredMode == 4 )
    {
    uiChromaPredMode          = pcCU->getLumaIntraDir( 0 );
    }
  m_pcPrediction->predIntraChromaAng( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );  
  }
#else // LM_CHROMA
  m_pcPrediction->predIntraChromaAng( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif

  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant  ( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), eText );
  m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight );
  //===== reconstruction =====
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecIPred += uiRecIPredStride;
    }
  }
}

Void
TDecCu::xIntraRecQT( TComDataCU* pcCU,
                    UInt        uiTrDepth,
                    UInt        uiAbsPartIdx,
                    TComYuv*    pcRecoYuv,
                    TComYuv*    pcPredYuv, 
                    TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecLumaBlk  ( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 0 );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 1 );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}

Void
TDecCu::xReconIntraQT( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt  uiInitTrDepth = ( pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1 );
  UInt  uiNumPart     = pcCU->getNumPartInter();
  UInt  uiNumQParts   = pcCU->getTotalNumPart() >> 2;
  
#if LM_CHROMA
  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraLumaRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }  

  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraChromaRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }
#else

  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }
#endif

}

#if LM_CHROMA

/** Funtion for deriving recontructed PU/CU Luma sample with QTree structure
 * \param pcCU pointer of current CU
 * \param uiTrDepth current tranform split depth
 * \param uiAbsPartIdx  part index
 * \param pcRecoYuv pointer to reconstructed sample arrays
 * \param pcPredYuv pointer to prediction sample arrays
 * \param pcResiYuv pointer to residue sample arrays
 * 
 \ This function dervies recontructed PU/CU Luma sample with recursive QTree structure
 */
Void
TDecCu::xIntraLumaRecQT( TComDataCU* pcCU,
                     UInt        uiTrDepth,
                     UInt        uiAbsPartIdx,
                     TComYuv*    pcRecoYuv,
                     TComYuv*    pcPredYuv, 
                     TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecLumaBlk  ( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraLumaRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}

/** Funtion for deriving recontructed PU/CU chroma samples with QTree structure
 * \param pcCU pointer of current CU
 * \param uiTrDepth current tranform split depth
 * \param uiAbsPartIdx  part index
 * \param pcRecoYuv pointer to reconstructed sample arrays
 * \param pcPredYuv pointer to prediction sample arrays
 * \param pcResiYuv pointer to residue sample arrays
 * 
 \ This function dervies recontructed PU/CU chroma samples with QTree recursive structure
 */
Void
TDecCu::xIntraChromaRecQT( TComDataCU* pcCU,
                     UInt        uiTrDepth,
                     UInt        uiAbsPartIdx,
                     TComYuv*    pcRecoYuv,
                     TComYuv*    pcPredYuv, 
                     TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 0 );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 1 );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraChromaRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}
#endif

Void TDecCu::xCopyToPic( TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth )
{
  UInt uiCUAddr = pcCU->getAddr();
  
  m_ppcYuvReco[uiDepth]->copyToPicYuv  ( pcPic->getPicYuvRec (), uiCUAddr, uiZorderIdx );
  
  return;
}

Void TDecCu::xDecodeInterTexture ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt    uiWidth    = pcCU->getWidth ( uiAbsPartIdx );
  UInt    uiHeight   = pcCU->getHeight( uiAbsPartIdx );
  TCoeff* piCoeff;
  
  Pel*    pResi;
  UInt    uiLumaTrMode, uiChromaTrMode;
  
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
  
  // Y
  piCoeff = pcCU->getCoeffY();
  pResi = m_ppcYuvResi[uiDepth]->getLumaAddr();
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_LUMA, pResi, 0, m_ppcYuvResi[uiDepth]->getStride(), uiWidth, uiHeight, uiLumaTrMode, 0, piCoeff );
  
  // Cb and Cr
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
  
  uiWidth  >>= 1;
  uiHeight >>= 1;
  piCoeff = pcCU->getCoeffCb(); pResi = m_ppcYuvResi[uiDepth]->getCbAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_U, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff );
  piCoeff = pcCU->getCoeffCr(); pResi = m_ppcYuvResi[uiDepth]->getCrAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_V, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff );
}

