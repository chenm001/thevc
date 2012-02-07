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

/** \file     TEncSlice.cpp
    \brief    slice encoder class
*/

#include "TEncTop.h"
#include "TEncSlice.h"
#include <math.h>

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSlice::TEncSlice()
{
  m_apcPicYuvPred = NULL;
  m_apcPicYuvResi = NULL;
}

TEncSlice::~TEncSlice()
{
}

Void TEncSlice::create( Int iWidth, Int iHeight, UInt iMaxCUWidth, UInt iMaxCUHeight, UChar uhTotalDepth )
{
  // create prediction picture
  if ( m_apcPicYuvPred == NULL )
  {
    m_apcPicYuvPred  = new TComPicYuv;
    m_apcPicYuvPred->create( iWidth, iHeight, iMaxCUWidth, iMaxCUHeight, uhTotalDepth );
  }
  
  // create residual picture
  if( m_apcPicYuvResi == NULL )
  {
    m_apcPicYuvResi  = new TComPicYuv;
    m_apcPicYuvResi->create( iWidth, iHeight, iMaxCUWidth, iMaxCUHeight, uhTotalDepth );
  }
}

Void TEncSlice::destroy()
{
  // destroy prediction picture
  if ( m_apcPicYuvPred )
  {
    m_apcPicYuvPred->destroy();
    delete m_apcPicYuvPred;
    m_apcPicYuvPred  = NULL;
  }
  
  // destroy residual picture
  if ( m_apcPicYuvResi )
  {
    m_apcPicYuvResi->destroy();
    delete m_apcPicYuvResi;
    m_apcPicYuvResi  = NULL;
  }
}

Void TEncSlice::init( TEncTop* pcEncTop )
{
  m_pcCfg             = pcEncTop;
  m_pcListPic         = pcEncTop->getListPic();
  
  m_pcGOPEncoder      = pcEncTop->getGOPEncoder();
  m_pcCuEncoder       = pcEncTop->getCuEncoder();
  m_pcPredSearch      = pcEncTop->getPredSearch();
  
  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcCavlcCoder      = pcEncTop->getCavlcCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  m_pcBinCABAC        = pcEncTop->getBinCABAC();
  m_pcTrQuant         = pcEncTop->getTrQuant();
  
  m_pcBitCounter      = pcEncTop->getBitCounter();
  m_pcRdCost          = pcEncTop->getRdCost();
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();
}

/**
 - non-referenced frame marking
 - QP computation based on temporal structure
 - lambda computation based on QP
 - set temporal layer ID and the parameter sets
 .
 \param pcPic         picture class
 \param iPOCLast      POC of last picture
 \param uiPOCCurr     current POC
 \param iNumPicRcvd   number of received pictures
 \param iTimeOffset   POC offset for hierarchical structure
 \param iDepth        temporal layer depth
 \param rpcSlice      slice header class
 \param pSPS          SPS associated with the slice
 \param pPPS          PPS associated with the slice
 */
#if G1002_RPS
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iGOPid, TComSlice*& rpcSlice, TComSPS* pSPS, TComPPS *pPPS )
#else
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, TComSlice*& rpcSlice, TComSPS* pSPS, TComPPS *pPPS )
#endif
{
  Double dQP;
  Double dLambda;
  
  rpcSlice = pcPic->getSlice(0);
  rpcSlice->setSliceBits(0);
  rpcSlice->setPic( pcPic );
  rpcSlice->initSlice();
#if G1002_RPS
  rpcSlice->setPOC( uiPOCCurr );
  
  // depth computation based on GOP size
  int iDepth;
#else
  rpcSlice->setPOC( iPOCLast - iNumPicRcvd + iTimeOffset );
  
  // depth re-computation based on rate GOP size
  if ( m_pcCfg->getGOPSize() != m_pcCfg->getRateGOPSize() )
#endif
  {
    Int i, j;
#if G1002_RPS
    Int iPOC = rpcSlice->getPOC()%m_pcCfg->getGOPSize();
#else
    Int iPOC = rpcSlice->getPOC()%m_pcCfg->getRateGOPSize();
#endif
    if ( iPOC == 0 )
    {
      iDepth = 0;
    }
    else
    {
#if G1002_RPS
      Int iStep = m_pcCfg->getGOPSize();
#else
      Int iStep = m_pcCfg->getRateGOPSize();
#endif
      iDepth    = 0;
      for( i=iStep>>1; i>=1; i>>=1 )
      {
#if G1002_RPS
        for ( j=i; j<m_pcCfg->getGOPSize(); j+=iStep )
#else
        for ( j=i; j<m_pcCfg->getRateGOPSize(); j+=iStep )
#endif
        {
          if ( j == iPOC )
          {
            i=0;
            break;
          }
        }
        iStep>>=1;
        iDepth++;
      }
    }
  }
  
  // slice type
  SliceType eSliceType;
  
#if G1002_RPS
  eSliceType=B_SLICE;
#else
#if !HB_LAMBDA_FOR_LDC
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
  else
#endif
  {
    eSliceType = ( uiPOCCurr % m_pcCfg->getGOPSize() != 0 || iDepth > 0) ? B_SLICE : P_SLICE;
  }
#endif
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType    ( eSliceType );
  
  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------
  
#if G1002_RPS
  rpcSlice->setReferenced(m_pcCfg->getGOPEntry(iGOPid).m_bRefPic);
  if(eSliceType==I_SLICE)
  {
    rpcSlice->setReferenced(true);
  }
#else
  if ( m_pcCfg->getUseNRF() )
  {
    if ( ( m_pcCfg->getRateGOPSize() != 1) && (m_pcCfg->getRateGOPSize() >> (iDepth+1)) == 0 )
    {
      rpcSlice->setReferenced(false);
    }
    else
    {
      rpcSlice->setReferenced(true);
    }
  }
  else
  {
    rpcSlice->setReferenced(true);
  }
#endif
  
  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------
  
  dQP = m_pcCfg->getQP();
#if G1002_RPS
  if(eSliceType!=I_SLICE)
  {
    dQP += m_pcCfg->getGOPEntry(iGOPid).m_iQPOffset;
  }
#else
  if ( iDepth < MAX_TLAYER && m_pcCfg->getTemporalLayerQPOffset(iDepth) != ( MAX_QP + 1 ) )
  {
    dQP += m_pcCfg->getTemporalLayerQPOffset(iDepth);
  }
  else
  {
    if ( ( iPOCLast != 0 ) && ( ( uiPOCCurr % m_pcCfg->getIntraPeriod() ) != 0 ) && ( m_pcGOPEncoder->getGOPSize() != 0 ) ) // P or B-slice
    {
      if ( m_pcCfg->getUseLDC() && !m_pcCfg->getUseBQP() )
      {
        if ( iDepth == 0 ) dQP += 1.0;
        else
        {
          dQP += iDepth+3;
        }
      }
      else
      {
        dQP += iDepth+1;
      }
    }
  }
#endif
  
  // ------------------------------------------------------------------------------------------------------------------
  // Lambda computation
  // ------------------------------------------------------------------------------------------------------------------
  
  Int iQP;

  // pre-compute lambda and QP values for all possible QP candidates
  {
    // compute lambda value
#if G1002_RPS
    Int    NumberBFrames = ( m_pcCfg->getGOPSize() - 1 );
#else
    Int    NumberBFrames = ( m_pcCfg->getRateGOPSize() - 1 );
#endif
    Int    SHIFT_QP = 12;
    Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)NumberBFrames );
    Double qp_temp = (double) dQP - SHIFT_QP;
    // Case #1: I or P-slices (key-frame)
#if G1002_RPS
    Double dQPFactor = m_pcCfg->getGOPEntry(iGOPid).m_iQPFactor;
    if ( eSliceType==I_SLICE )
    {
      dQPFactor=0.57*dLambda_scale;
    }
    dLambda = dQPFactor*pow( 2.0, qp_temp/3.0 );

    if ( iDepth>0 )
    {
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
    }
    
#else
    if ( iDepth == 0 )
    {
        if ( NumberBFrames > 0 ) // HB structure or HP structure
        {
          dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
        }
        else                     // IPP structure
        {
          dLambda = 0.85 * pow( 2.0, qp_temp/3.0 );
        }
      dLambda *= dLambda_scale;
    }
    else // P or B slices for HB or HP structure
    {
      dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
      if ( pcPic->getSlice(0)->isInterB () )
      {
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
        if ( rpcSlice->isReferenced() ) // HB structure and referenced
        {
          dLambda *= 0.80;
          dLambda *= dLambda_scale;
        }
      }
      else
      {
        dLambda *= dLambda_scale;
      }
    }
#endif
    // if hadamard is used in ME process
    if ( !m_pcCfg->getUseHADME() )
    {
      dLambda *= 0.95;
    }
    
    iQP = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
  }
  
#if G678_LAMBDA_ADJUSTMENT
  if( rpcSlice->getSliceType( ) != I_SLICE )
  {
    dLambda *= m_pcCfg->getLambdaModifier( iDepth );
  }
#endif

  // store lambda
  m_pcRdCost ->setLambda( dLambda );
#if WEIGHTED_CHROMA_DISTORTION
// for RDO
  // in RdCost there is only one lambda because the luma and chroma bits are not separated, instead we weight the distortion of chroma.
  double weight = pow( 2.0, (iQP-g_aucChromaScale[iQP])/3.0 );  // takes into account of the chroma qp mapping without chroma qp Offset
  m_pcRdCost ->setChromaDistortionWeight( weight );     
#endif

#if RDOQ_CHROMA_LAMBDA 
// for RDOQ
  m_pcTrQuant->setLambda( dLambda, dLambda / weight );    
#else
  m_pcTrQuant->setLambda( dLambda );
#endif

  rpcSlice   ->setLambda( dLambda );
  
#if HB_LAMBDA_FOR_LDC
  // restore original slice type
#if !G1002_RPS
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
#endif
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType        ( eSliceType );
#endif
  
  rpcSlice->setSliceQp          ( iQP );
  rpcSlice->setSliceQpDelta     ( 0 );
#if G1002_RPS
  rpcSlice->setNumRefIdx(REF_PIC_LIST_0,m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize);
  rpcSlice->setNumRefIdx(REF_PIC_LIST_1,m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize);
#else
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_0, eSliceType == P_SLICE ? m_pcCfg->getNumOfReference() : (eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L0()) : 0 ) );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_1, eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L1()) : 0 );
#endif
  
  rpcSlice->setDepth            ( iDepth );
  
#if G1002_RPS
  pcPic->setTLayer( m_pcCfg->getGOPEntry(iGOPid).m_iTemporalId );
  if(eSliceType==I_SLICE)
  {
    pcPic->setTLayer(0);
  }
#else
  if ( pSPS->getMaxTLayers() > 1 )
  {
    assert( iDepth < pSPS->getMaxTLayers() );
    pcPic->setTLayer( iDepth );
  }
  else 
  {
    pcPic->setTLayer( 0 );
  }
#endif
  rpcSlice->setTLayer( pcPic->getTLayer() );
  rpcSlice->setTLayerSwitchingFlag( pPPS->getTLayerSwitchingFlag( pcPic->getTLayer() ) );

  rpcSlice->setSPS( pSPS );
  rpcSlice->setPPS( pPPS );

#if !G1002_RPS
  // reference picture usage indicator for next frames
  rpcSlice->setDRBFlag          ( true );
  rpcSlice->setERBIndex         ( ERB_NONE );
#endif
  
  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );
  
  pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  rpcSlice->setMaxNumMergeCand      (MRG_MAX_NUM_CANDS_SIGNALED);
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSlice::setSearchRange( TComSlice* pcSlice )
{
  Int iCurrPOC = pcSlice->getPOC();
  Int iRefPOC;
#if G1002_RPS
  Int iGOPSize = m_pcCfg->getGOPSize();
  Int iOffset = (iGOPSize >> 1);
#else
  Int iRateGOPSize = m_pcCfg->getRateGOPSize();
  Int iOffset = (iRateGOPSize >> 1);
#endif
  Int iMaxSR = m_pcCfg->getSearchRange();
  Int iNumPredDir = pcSlice->isInterP() ? 1 : 2;
  
  for (Int iDir = 0; iDir <= iNumPredDir; iDir++)
  {
    RefPicList e = (RefPicList)iDir;
    for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(e); iRefIdx++)
    {
      iRefPOC = pcSlice->getRefPic(e, iRefIdx)->getPOC();
#if G1002_RPS
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iGOPSize);
#else
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iRateGOPSize);
#endif
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iRefIdx, iNewSR);
    }
  }
}

/** \param rpcPic   picture class
 */
Void TEncSlice::compressSlice( TComPic*& rpcPic )
{
  UInt  uiCUAddr;
  UInt   uiStartCUAddr;
  UInt   uiBoundingCUAddr;
  TEncBinCABAC* pppcRDSbacCoder = NULL;
  TComSlice* pcSlice            = rpcPic->getSlice(getSliceIdx());
  xDetermineStartAndBoundingCUAddr ( uiStartCUAddr, uiBoundingCUAddr, rpcPic, false );
  
  // initialize cost values
  m_uiPicTotalBits  = 0;
  m_dPicRdCost      = 0;
  m_uiPicDist       = 0;
  
  // set entropy coder
  if( m_pcCfg->getUseSBACRD() )
  {
    m_pcSbacCoder->init( m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy      ();
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->load(m_pcSbacCoder);
    pppcRDSbacCoder = (TEncBinCABAC *) m_pppcRDSbacCoder[0][CI_CURR_BEST]->getEncBinIf();
    pppcRDSbacCoder->setBinCountingEnableFlag( false );
  }
  else
  {
    m_pcCavlcCoder  ->setAdaptFlag    ( false );
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy      ();
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  }
  
  // chen
  TEncTop* pcEncTop = (TEncTop*) m_pcCfg;
  m_pcBitCounter = pcEncTop->getBitCounter();
  
  // for every CU in slice
  for(  uiCUAddr = uiStartCUAddr/rpcPic->getNumPartInCU(); uiCUAddr < (uiBoundingCUAddr+(rpcPic->getNumPartInCU()-1))/rpcPic->getNumPartInCU(); uiCUAddr++  )
  {
    // initialize CU encoder
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
    pcCU->initCU( rpcPic, uiCUAddr );

    // if RD based on SBAC is used
    if( m_pcCfg->getUseSBACRD() )
    {
      // set go-on entropy coder
      m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, pcSlice );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      
      ((TEncBinCABAC*)m_pcRDGoOnSbacCoder->getEncBinIf())->setBinCountingEnableFlag(true);
      // run CU encoder
      m_pcCuEncoder->compressCU( pcCU );
      
      // restore entropy coder to an initial stage
      m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], pcSlice );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      m_pcCuEncoder->setBitCounter      ( m_pcBitCounter );
      pppcRDSbacCoder->setBinCountingEnableFlag( true );
      m_pcBitCounter->resetBits();
      m_pcCuEncoder->encodeCU( pcCU );

      pppcRDSbacCoder->setBinCountingEnableFlag( false );
    }
    // other case: encodeCU is not called
    else
    {
      m_pcCuEncoder->compressCU( pcCU );
      m_pcCavlcCoder ->setAdaptFlag(true);
      m_pcCuEncoder->encodeCU( pcCU );
      m_pcCavlcCoder ->setAdaptFlag(false);
    }
    
    m_uiPicTotalBits += pcCU->getTotalBits();
    m_dPicRdCost     += pcCU->getTotalCost();
    m_uiPicDist      += pcCU->getTotalDistortion();
  }
}

/**
 \param  rpcPic        picture class
 \retval rpcBitstream  bitstream class
 */
Void TEncSlice::encodeSlice   ( TComPic*& rpcPic, TComOutputBitstream* pcBitstream )
{
  UInt       uiCUAddr;
  UInt       uiStartCUAddr;
  UInt       uiBoundingCUAddr;
  TComSlice* pcSlice = rpcPic->getSlice(getSliceIdx());

  uiStartCUAddr=pcSlice->getSliceCurStartCUAddr();
  uiBoundingCUAddr=pcSlice->getSliceCurEndCUAddr();
  // choose entropy coder
    m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
  
  m_pcCuEncoder->setBitCounter( NULL );
  m_pcBitCounter = NULL;

  // set bitstream
  m_pcEntropyCoder->setBitstream( pcBitstream );
  // for every CU
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( rpcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif

  for(  uiCUAddr = uiStartCUAddr/rpcPic->getNumPartInCU(); uiCUAddr<(uiBoundingCUAddr+rpcPic->getNumPartInCU()-1)/rpcPic->getNumPartInCU(); uiCUAddr++  )
  {
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );    
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
      m_pcCuEncoder->encodeCU( pcCU );
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif    
  }

}

/** Determines the starting and bounding LCU address of current slice / entropy slice
 * \param bEncodeSlice Identifies if the calling function is compressSlice() [false] or encodeSlice() [true]
 * \returns Updates uiStartCUAddr, uiBoundingCUAddr with appropriate LCU address
 */
Void TEncSlice::xDetermineStartAndBoundingCUAddr  ( UInt& uiStartCUAddr, UInt& uiBoundingCUAddr, TComPic*& rpcPic, Bool bEncodeSlice )
{
  TComSlice* pcSlice = rpcPic->getSlice(getSliceIdx());
  UInt uiStartCUAddrSlice, uiBoundingCUAddrSlice;
  uiStartCUAddrSlice        = pcSlice->getSliceCurStartCUAddr();
  UInt uiNumberOfCUsInFrame = rpcPic->getNumCUsInFrame();
  uiBoundingCUAddrSlice     = uiNumberOfCUsInFrame;
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
    pcSlice->setSliceCurEndCUAddr( uiBoundingCUAddrSlice );

  //calculate real slice start address
  UInt uiInternalAddress = (pcSlice->getSliceCurStartCUAddr()) % rpcPic->getNumPartInCU();
  UInt uiExternalAddress = (pcSlice->getSliceCurStartCUAddr()) / rpcPic->getNumPartInCU();
  UInt uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiWidth = pcSlice->getSPS()->getWidth();
  UInt uiHeight = pcSlice->getSPS()->getHeight();
  while((uiPosX>=uiWidth||uiPosY>=uiHeight)&&!(uiPosX>=uiWidth&&uiPosY>=uiHeight))
  {
    uiInternalAddress++;
    if(uiInternalAddress>=rpcPic->getNumPartInCU())
    {
      uiInternalAddress=0;
      uiExternalAddress++;
    }
    uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  UInt uiRealStartAddress = uiExternalAddress*rpcPic->getNumPartInCU()+uiInternalAddress;
  
  pcSlice->setSliceCurStartCUAddr(uiRealStartAddress);
  uiStartCUAddrSlice=uiRealStartAddress;
  
  // Make a joint decision based on reconstruction and entropy slice bounds
  uiStartCUAddr    = uiStartCUAddrSlice;
  uiBoundingCUAddr = uiBoundingCUAddrSlice;


  if (!bEncodeSlice)
  {
    // For fixed number of LCU within an entropy and reconstruction slice we already know whether we will encounter end of entropy and/or reconstruction slice
    // first. Set the flags accordingly.
      pcSlice->setNextSlice       ( false );
  }
}
//! \}
