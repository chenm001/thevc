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

/** \file     TEncSlice.cpp
    \brief    slice encoder class
*/

#include "TEncTop.h"
#include "TEncSlice.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSlice::TEncSlice()
{
  m_apcPicYuvPred = NULL;
  m_apcPicYuvResi = NULL;
  
  m_pdRdPicLambda = NULL;
  m_pdRdPicQp     = NULL;
  m_piRdPicQp     = NULL;
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
  
  // free lambda and QP arrays
  if ( m_pdRdPicLambda ) { xFree( m_pdRdPicLambda ); m_pdRdPicLambda = NULL; }
  if ( m_pdRdPicQp     ) { xFree( m_pdRdPicQp     ); m_pdRdPicQp     = NULL; }
  if ( m_piRdPicQp     ) { xFree( m_piRdPicQp     ); m_piRdPicQp     = NULL; }
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
  
  // create lambda and QP arrays
  m_pdRdPicLambda     = (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_pdRdPicQp         = (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_piRdPicQp         = (Int*   )xMalloc( Int,    m_pcCfg->getDeltaQpRD() * 2 + 1 );
}

/**
 - non-referenced frame marking
 - QP computation based on temporal structure
 - lambda computation based on QP
 .
 \param pcPic         picture class
 \param iPOCLast      POC of last picture
 \param uiPOCCurr     current POC
 \param iNumPicRcvd   number of received pictures
 \param iTimeOffset   POC offset for hierarchical structure
 \param iDepth        temporal layer depth
 \param rpcSlice      slice header class
 */
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, TComSlice*& rpcSlice )
{
  Double dQP;
  Double dLambda;
  
  rpcSlice = pcPic->getSlice();
  rpcSlice->setPic( pcPic );
  rpcSlice->initSlice();
  
  // depth re-computation based on rate GOP size
  if ( m_pcCfg->getGOPSize() != m_pcCfg->getRateGOPSize() )
  {
    Int i, j;
    Int iPOC = rpcSlice->getPOC()%m_pcCfg->getRateGOPSize();
    if ( iPOC == 0 ) iDepth = 0;
    else
    {
      Int iStep = m_pcCfg->getRateGOPSize();
      iDepth    = 0;
      for( i=iStep>>1; i>=1; i>>=1 )
      {
        for ( j=i; j<m_pcCfg->getRateGOPSize(); j+=iStep )
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
  
#if !HB_LAMBDA_FOR_LDC
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
  else
#endif
  {
    eSliceType = iDepth > 0 ? B_SLICE : P_SLICE;
  }
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType    ( eSliceType );
  rpcSlice->setPOC          ( iPOCLast - iNumPicRcvd + iTimeOffset );
  
  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------
  
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
  
  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------
  
  dQP = m_pcCfg->getQP();
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
  
  // modify QP
  Int* pdQPs = m_pcCfg->getdQPs();
  if ( pdQPs )
  {
    dQP += pdQPs[ pcPic->getSlice()->getPOC() ];
  }
  
  // ------------------------------------------------------------------------------------------------------------------
  // Lambda computation
  // ------------------------------------------------------------------------------------------------------------------
  
  Int iQP;
  Double dOrigQP = dQP;
  
  // pre-compute lambda and QP values for all possible QP candidates
  for ( Int iDQpIdx = 0; iDQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; iDQpIdx++ )
  {
    // compute QP value
    dQP = dOrigQP + ((iDQpIdx+1)>>1)*(iDQpIdx%2 ? -1 : 1);
    
    // compute lambda value
    Int    NumberBFrames = ( m_pcCfg->getRateGOPSize() - 1 );
    Int    SHIFT_QP = 12;
    Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)NumberBFrames );
    Int    bitdepth_luma_qp_scale = 0;
    Double qp_temp = (double) dQP + bitdepth_luma_qp_scale - SHIFT_QP;
    
    // Case #1: I or P-slices (key-frame)
    if ( iDepth == 0 )
    {
      if ( m_pcCfg->getUseRDOQ() && pcPic->getSlice()->isIntra() && dQP == dOrigQP )
      {
        dLambda = 0.57 * pow( 2.0, qp_temp/3.0 );
      }
      else
      {
        if ( NumberBFrames > 0 ) // HB structure or HP structure
        {
          dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
        }
        else                     // IPP structure
        {
          dLambda = 0.85 * pow( 2.0, qp_temp/3.0 );
        }
      }
      dLambda *= dLambda_scale;
    }
    else // P or B slices for HB or HP structure
    {
      dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
      if ( pcPic->getSlice()->isInterB () )
      {
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
        if ( rpcSlice->isReferenced() ) // HB structure and referenced
        {
          Int    iMaxDepth = 0;
          Int    iCnt = 1;
          Int    hierarchy_layer;
          
          while ( iCnt < m_pcCfg->getRateGOPSize() ) { iCnt <<= 1; iMaxDepth++; }
          hierarchy_layer = iMaxDepth - iDepth;
          
          dLambda *= 0.80;
          dLambda *= dLambda_scale;
        }
      }
      else
      {
        dLambda *= dLambda_scale;
      }
    }
    // if hadamard is used in ME process
    if ( !m_pcCfg->getUseHADME() ) dLambda *= 0.95;
    
    iQP = Max( MIN_QP, Min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
    
    m_pdRdPicLambda[iDQpIdx] = dLambda;
    m_pdRdPicQp    [iDQpIdx] = dQP;
    m_piRdPicQp    [iDQpIdx] = iQP;
  }
  
  // obtain dQP = 0 case
  dLambda = m_pdRdPicLambda[0];
  dQP     = m_pdRdPicQp    [0];
  iQP     = m_piRdPicQp    [0];
  
  // store lambda
  m_pcRdCost ->setLambda( dLambda );
  m_pcTrQuant->setLambda( dLambda );
  rpcSlice   ->setLambda( dLambda );
  
#if HB_LAMBDA_FOR_LDC
  // restore original slice type
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType        ( eSliceType );
#endif
  
  rpcSlice->setSliceQp          ( iQP );
  rpcSlice->setSliceQpDelta     ( 0 );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_0, eSliceType == P_SLICE ? m_pcCfg->getNumOfReference() : (eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L0()) : 0 ) );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_1, eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L1()) : 0 );
  
  rpcSlice->setSymbolMode       ( m_pcCfg->getSymbolMode());
  rpcSlice->setLoopFilterDisable( m_pcCfg->getLoopFilterDisable() );
  
  rpcSlice->setDepth            ( iDepth );
  
  // reference picture usage indicator for next frames
  rpcSlice->setDRBFlag          ( true );
  rpcSlice->setERBIndex         ( ERB_NONE );
  
  // generalized B info. (for non-reference B)
  if ( m_pcCfg->getHierarchicalCoding() == false && iDepth != 0 )
  {
    rpcSlice->setDRBFlag        ( false );
    rpcSlice->setERBIndex       ( ERB_NONE );
  }
  
  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );
  
  pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );
  rpcSlice->setInterpFilterType ( m_pcCfg->getInterpFilterType() );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSlice::setSearchRange( TComSlice* pcSlice )
{
  Int iCurrPOC = pcSlice->getPOC();
  Int iRefPOC;
  Int iRateGOPSize = m_pcCfg->getRateGOPSize();
  Int iOffset = (iRateGOPSize >> 1);
  Int iMaxSR = m_pcCfg->getSearchRange();
  Int iNumPredDir = pcSlice->isInterP() ? 1 : 2;
  
  for (Int iDir = 0; iDir <= iNumPredDir; iDir++)
  {
    RefPicList e = (RefPicList)iDir;
    for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(e); iRefIdx++)
    {
      iRefPOC = pcSlice->getRefPic(e, iRefIdx)->getPOC();
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iRateGOPSize);
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iRefIdx, iNewSR);
    }
  }
}

/**
 - multi-loop slice encoding for different slice QP
 .
 \param rpcPic    picture class
 */
Void TEncSlice::precompressSlice( TComPic*& rpcPic )
{
  // if deltaQP RD is not used, simply return
  if ( m_pcCfg->getDeltaQpRD() == 0 ) return;
  
  TComSlice* pcSlice        = rpcPic->getSlice();
  Double     dPicRdCostBest = MAX_DOUBLE;
  Double dSumCURdCostBest;
  UInt64     uiPicDistBest;
  UInt64     uiPicBitsBest;
  UInt       uiQpIdxBest = 0;
  
  Double dFrameLambda;
  Int    SHIFT_QP = 12;
  
  // set frame lambda
  if (m_pcCfg->getGOPSize() > 1)
  {
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0]  - SHIFT_QP) / 3.0) * (pcSlice->isInterB()? 2 : 1);
  }
  else
  {
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0] - SHIFT_QP) / 3.0);
  }
  m_pcRdCost      ->setFrameLambda(dFrameLambda);
  
  // for each QP candidate
  for ( UInt uiQpIdx = 0; uiQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; uiQpIdx++ )
  {
    pcSlice       ->setSliceQp             ( m_piRdPicQp    [uiQpIdx] );
    m_pcRdCost    ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    m_pcTrQuant   ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    
    // try compress
    compressSlice   ( rpcPic );
    
    Double dPicRdCost;
    UInt64 uiPicDist        = m_uiPicDist;
    UInt64 uiALFBits        = 0;
    
    m_pcGOPEncoder->preLoopFilterPicAll( rpcPic, uiPicDist, uiALFBits );
    
    // compute RD cost and choose the best
    dPicRdCost = m_pcRdCost->calcRdCost64( m_uiPicTotalBits + uiALFBits, uiPicDist, true, DF_SSE_FRAME);
    
    if ( dPicRdCost < dPicRdCostBest )
    {
      uiQpIdxBest    = uiQpIdx;
      dPicRdCostBest = dPicRdCost;
      dSumCURdCostBest = m_dPicRdCost;
      
      uiPicBitsBest = m_uiPicTotalBits + uiALFBits;
      uiPicDistBest = uiPicDist;
    }
  }
  
  // set best values
  pcSlice       ->setSliceQp             ( m_piRdPicQp    [uiQpIdxBest] );
  m_pcRdCost    ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
  m_pcTrQuant   ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
  pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
}

/** \param rpcPic   picture class
 */
Void TEncSlice::compressSlice( TComPic*& rpcPic )
{
  UInt  uiCUAddr;
  
  // initialize cost values
  m_uiPicTotalBits  = 0;
  m_dPicRdCost      = 0;
  m_uiPicDist       = 0;
  
  // set entropy coder
  if( m_pcCfg->getUseSBACRD() )
  {
    m_pcSbacCoder->init( m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, rpcPic->getSlice() );
    m_pcEntropyCoder->resetEntropy      ();
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->load(m_pcSbacCoder);
  }
  else
  {
    m_pcCavlcCoder  ->setAdaptFlag    ( false );
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, rpcPic->getSlice() );
    m_pcEntropyCoder->resetEntropy      ();
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  }
  
  // initialize ALF parameters
  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0); //unnecessary
  
  // for every CU
  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    // set QP
    m_pcCuEncoder->setQpLast( rpcPic->getSlice()->getSliceQp() );
    
    // initialize CU encoder
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
    pcCU->initCU( rpcPic, uiCUAddr );
    
    // if RD based on SBAC is used
    if( m_pcCfg->getUseSBACRD() )
    {
      // set go-on entropy coder
      m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, rpcPic->getSlice() );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      
      // run CU encoder
      m_pcCuEncoder->compressCU( pcCU );
      
      // restore entropy coder to an initial stage
      m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], rpcPic->getSlice() );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      
      m_pcCuEncoder->encodeCU( pcCU );
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
Void TEncSlice::encodeSlice   ( TComPic*& rpcPic, TComBitstream*& rpcBitstream )
{
  UInt       uiCUAddr;
  TComSlice* pcSlice = rpcPic->getSlice();
  
  // choose entropy coder
  Int iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
  {
    m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
  }
  else
  {
    m_pcCavlcCoder  ->setAdaptFlag( true );
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy();
  }
  
  // set bitstream
  m_pcEntropyCoder->setBitstream( rpcBitstream );
  // for every CU
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_V( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( rpcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif
  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    m_pcCuEncoder->setQpLast( rpcPic->getSlice()->getSliceQp() );
    
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
