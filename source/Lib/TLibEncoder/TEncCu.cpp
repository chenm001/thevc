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

/** \file     TEncCU.cpp
    \brief    CU encoder class
*/

#include <stdio.h>
#include "TEncTop.h"
#include "TEncCu.h"
#include "TEncAnalyze.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

/**
 \param    uiTotalDepth  total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 */
Void TEncCu::create(UChar uhTotalDepth, UInt uiMaxWidth, UInt uiMaxHeight)
{
  Int i;
  
  m_uhTotalDepth   = uhTotalDepth + 1;
  m_ppcBestCU      = new TComDataCU*[m_uhTotalDepth-1];
  m_ppcTempCU      = new TComDataCU*[m_uhTotalDepth-1];
  
  m_ppcPredYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcPredYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcOrigYuv     = new TComYuv*[m_uhTotalDepth-1];
  
  UInt uiNumPartitions;
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    uiNumPartitions = 1<<( ( m_uhTotalDepth - i - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> i;
    UInt uiHeight = uiMaxHeight >> i;
    
    m_ppcBestCU[i] = new TComDataCU; m_ppcBestCU[i]->create( uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );
    m_ppcTempCU[i] = new TComDataCU; m_ppcTempCU[i]->create( uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );
    
    m_ppcPredYuvBest[i] = new TComYuv; m_ppcPredYuvBest[i]->create(uiWidth, uiHeight);
    m_ppcResiYuvBest[i] = new TComYuv; m_ppcResiYuvBest[i]->create(uiWidth, uiHeight);
    m_ppcRecoYuvBest[i] = new TComYuv; m_ppcRecoYuvBest[i]->create(uiWidth, uiHeight);
    
    m_ppcPredYuvTemp[i] = new TComYuv; m_ppcPredYuvTemp[i]->create(uiWidth, uiHeight);
    m_ppcResiYuvTemp[i] = new TComYuv; m_ppcResiYuvTemp[i]->create(uiWidth, uiHeight);
    m_ppcRecoYuvTemp[i] = new TComYuv; m_ppcRecoYuvTemp[i]->create(uiWidth, uiHeight);
    
    m_ppcOrigYuv    [i] = new TComYuv; m_ppcOrigYuv    [i]->create(uiWidth, uiHeight);
  }
  
  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster( m_uhTotalDepth, 1, 0, piTmp);
  initRasterToZscan( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );
  
  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );
}

Void TEncCu::destroy()
{
  Int i;
  
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    if(m_ppcBestCU[i])
    {
      m_ppcBestCU[i]->destroy();      delete m_ppcBestCU[i];      m_ppcBestCU[i] = NULL;
    }
    if(m_ppcTempCU[i])
    {
      m_ppcTempCU[i]->destroy();      delete m_ppcTempCU[i];      m_ppcTempCU[i] = NULL;
    }
    if(m_ppcPredYuvBest[i])
    {
      m_ppcPredYuvBest[i]->destroy(); delete m_ppcPredYuvBest[i]; m_ppcPredYuvBest[i] = NULL;
    }
    if(m_ppcResiYuvBest[i])
    {
      m_ppcResiYuvBest[i]->destroy(); delete m_ppcResiYuvBest[i]; m_ppcResiYuvBest[i] = NULL;
    }
    if(m_ppcRecoYuvBest[i])
    {
      m_ppcRecoYuvBest[i]->destroy(); delete m_ppcRecoYuvBest[i]; m_ppcRecoYuvBest[i] = NULL;
    }
    if(m_ppcPredYuvTemp[i])
    {
      m_ppcPredYuvTemp[i]->destroy(); delete m_ppcPredYuvTemp[i]; m_ppcPredYuvTemp[i] = NULL;
    }
    if(m_ppcResiYuvTemp[i])
    {
      m_ppcResiYuvTemp[i]->destroy(); delete m_ppcResiYuvTemp[i]; m_ppcResiYuvTemp[i] = NULL;
    }
    if(m_ppcRecoYuvTemp[i])
    {
      m_ppcRecoYuvTemp[i]->destroy(); delete m_ppcRecoYuvTemp[i]; m_ppcRecoYuvTemp[i] = NULL;
    }
    if(m_ppcOrigYuv[i])
    {
      m_ppcOrigYuv[i]->destroy();     delete m_ppcOrigYuv[i];     m_ppcOrigYuv[i] = NULL;
    }
  }
  if(m_ppcBestCU)
  {
    delete [] m_ppcBestCU;
    m_ppcBestCU = NULL;
  }
  if(m_ppcTempCU)
  {
    delete [] m_ppcTempCU;
    m_ppcTempCU = NULL;
  }
  
  if(m_ppcPredYuvBest)
  {
    delete [] m_ppcPredYuvBest;
    m_ppcPredYuvBest = NULL;
  }
  if(m_ppcResiYuvBest)
  {
    delete [] m_ppcResiYuvBest;
    m_ppcResiYuvBest = NULL;
  }
  if(m_ppcRecoYuvBest)
  {
    delete [] m_ppcRecoYuvBest;
    m_ppcRecoYuvBest = NULL;
  }
  if(m_ppcPredYuvTemp)
  {
    delete [] m_ppcPredYuvTemp;
    m_ppcPredYuvTemp = NULL;
  }
  if(m_ppcResiYuvTemp)
  {
    delete [] m_ppcResiYuvTemp;
    m_ppcResiYuvTemp = NULL;
  }
  if(m_ppcRecoYuvTemp)
  {
    delete [] m_ppcRecoYuvTemp;
    m_ppcRecoYuvTemp = NULL;
  }
  if(m_ppcOrigYuv)
  {
    delete [] m_ppcOrigYuv;
    m_ppcOrigYuv = NULL;
  }
}

/** \param    pcEncTop      pointer of encoder class
 */
Void TEncCu::init( TEncTop* pcEncTop )
{
  m_pcEncCfg           = pcEncTop;
  m_pcPredSearch       = pcEncTop->getPredSearch();
  m_pcTrQuant          = pcEncTop->getTrQuant();
  m_pcBitCounter       = pcEncTop->getBitCounter();
  m_pcRdCost           = pcEncTop->getRdCost();
  
  m_pcEntropyCoder     = pcEncTop->getEntropyCoder();
  m_pcCavlcCoder       = pcEncTop->getCavlcCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  m_pcBinCABAC         = pcEncTop->getBinCABAC();
  
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();
  
  m_bUseSBACRD        = pcEncTop->getUseSBACRD();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  rpcCU pointer of CU data class
 */
Void TEncCu::compressCU( TComDataCU*& rpcCU )
{
#if SUB_LCU_DQP
  // initialize CU data
  m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
  m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
  m_ppcBestCU[0]->setLastCodedQP( rpcCU->getLastCodedQP() );
  m_ppcTempCU[0]->setLastCodedQP( rpcCU->getLastCodedQP() );

  // analysis of CU
  xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 );
#else
  // single-QP coding mode
  if ( rpcCU->getSlice()->getSPS()->getUseDQP() == false )
  {
    // initialize CU data
    m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
    m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );

    // analysis of CU
    xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 );
  }
  // multiple-QP coding mode
  else
  {
    Int iQP  = rpcCU->getSlice()->getSliceQp();
    Int idQP = m_pcEncCfg->getMaxDeltaQP();
    Int i;
    Int iBestQP = iQP;
    Double fBestCost = MAX_DOUBLE;

    rpcCU->getSlice()->setSliceQp( iQP );
    m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
    m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
    m_ppcBestCU[0]->setQPSubParts( iQP, 0, 0 );
    m_ppcTempCU[0]->setQPSubParts( iQP, 0, 0 );

    // first try
    xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 );

    // for non-zero residual case
    if ( !( m_ppcBestCU[0]->isSkipped( 0 ) && m_ppcBestCU[0]->getDepth( 0 ) == 0 ) )
    {
      // add dQP bits
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( m_ppcBestCU[0], 0, false );
      m_ppcBestCU[0]->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
      if(m_pcEncCfg->getUseSBACRD())
      {
        m_ppcBestCU[0]->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }
#endif
      m_ppcBestCU[0]->getTotalCost()  = m_pcRdCost->calcRdCost( m_ppcBestCU[0]->getTotalBits(), m_ppcBestCU[0]->getTotalDistortion() );

      fBestCost = m_ppcBestCU[0]->getTotalCost();

      // try every case
      for ( i=iQP-idQP; i<=iQP+idQP; i++ )
      {
        if ( i == iQP ) continue;

        rpcCU->getSlice()->setSliceQp( i );
        m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
        m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
        m_ppcBestCU[0]->setQPSubParts( i, 0, 0 );
        m_ppcTempCU[0]->setQPSubParts( i, 0, 0 );

        xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 );

        // add dQP bits
        rpcCU->getSlice()->setSliceQp( iQP );
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeQP( m_ppcBestCU[0], 0, false );
        m_ppcBestCU[0]->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
        if(m_pcEncCfg->getUseSBACRD())
        {
          m_ppcBestCU[0]->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
        }
#endif
        m_ppcBestCU[0]->getTotalCost()  = m_pcRdCost->calcRdCost( m_ppcBestCU[0]->getTotalBits(), m_ppcBestCU[0]->getTotalDistortion() );

        if ( fBestCost > m_ppcBestCU[0]->getTotalCost() )
        {
          fBestCost = m_ppcBestCU[0]->getTotalCost();
          iBestQP   = i;
        }
      }

      // perform best case
      rpcCU->getSlice()->setSliceQp( iBestQP );
      m_ppcBestCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
      m_ppcTempCU[0]->initCU( rpcCU->getPic(), rpcCU->getAddr() );
      m_ppcBestCU[0]->setQPSubParts( iBestQP, 0, 0 );
      m_ppcTempCU[0]->setQPSubParts( iBestQP, 0, 0 );

      xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 );

      // add dQP bits
      rpcCU->getSlice()->setSliceQp( iQP );
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( m_ppcBestCU[0], 0, false );
      m_ppcBestCU[0]->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
      if(m_pcEncCfg->getUseSBACRD())
      {
        m_ppcBestCU[0]->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }
#endif
      m_ppcBestCU[0]->getTotalCost()  = m_pcRdCost->calcRdCost( m_ppcBestCU[0]->getTotalBits(), m_ppcBestCU[0]->getTotalDistortion() );
    }
  }
#endif
}
/** \param  pcCU  pointer of CU data class, bForceTerminate when set to true terminates slice (default is false).
 */
Void TEncCu::encodeCU ( TComDataCU* pcCU, Bool bForceTerminate )
{
#if SNY_DQP  
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    pcCU->setdQPFlag(true); 
  }
#endif//SNY_DQP
  // Encode CU data
  xEncodeCU( pcCU, 0, 0 );
#if SNY_DQP
#if SUB_LCU_DQP
#else
  // dQP: only for LCU
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
    {
      pcCU->setQPSubParts( pcCU->getSlice()->getSliceQp(), 0, 0 ); 
    }
    else if ( pcCU->getdQPFlag())// non-skip
    {

      m_pcEntropyCoder->encodeQP( pcCU, 0 );
      pcCU->setdQPFlag(false);
    }
  }
#endif
#else
  // dQP: only for LCU
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
    {
      pcCU->setQPSubParts( pcCU->getSlice()->getSliceQp(), 0, 0 ); 
    }
    else
    {
      m_pcEntropyCoder->encodeQP( pcCU, 0 );
    }
  }
#endif//SNY_DQP
  
#if !FINE_GRANULARITY_SLICES
  //--- write terminating bit ---
  Bool bTerminateSlice = bForceTerminate;
  UInt uiCUAddr = pcCU->getAddr();

  if (uiCUAddr == (pcCU->getPic()->getNumCUsInFrame()-1) )
    bTerminateSlice = true;

  if (uiCUAddr == (pcCU->getSlice()->getSliceCurEndCUAddr()-1))
    bTerminateSlice = true;

  m_pcEntropyCoder->encodeTerminatingBit( bTerminateSlice ? 1 : 0 );
  
  // Encode slice finish
  if ( bTerminateSlice )
  {
    m_pcEntropyCoder->encodeSliceFinish();
  }
#endif
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
/** Compress a CU block recursively with enabling sub-LCU-level delta QP
 *\param   rpcBestCU
 *\param   rpcTempCU
 *\param   uiDepth
 *\returns Void
 *
 *- for loop of QP value to compress the current CU with all possible QP
*/
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth )
{
  TComPic* pcPic = rpcBestCU->getPic();

  // get Original YUV data from picture
  m_ppcOrigYuv[uiDepth]->copyFromPicYuv( pcPic->getPicYuvOrg(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU() );

  // variables for fast encoder decision
  Bool    bEarlySkip  = false;
  Bool    bTrySplit    = true;
  Double  fRD_Skip    = MAX_DOUBLE;
#if SUB_LCU_DQP
  Bool    bTrySplitDQP  = true;
#endif

  static  Double  afCost[ MAX_CU_DEPTH ];
  static  Int      aiNum [ MAX_CU_DEPTH ];

  if ( rpcBestCU->getAddr() == 0 )
  {
    ::memset( afCost, 0, sizeof( afCost ) );
    ::memset( aiNum,  0, sizeof( aiNum  ) );
  }

  Bool bBoundary = false;
  UInt uiLPelX   = rpcBestCU->getCUPelX();
  UInt uiRPelX   = uiLPelX + rpcBestCU->getWidth(0)  - 1;
  UInt uiTPelY   = rpcBestCU->getCUPelY();
  UInt uiBPelY   = uiTPelY + rpcBestCU->getHeight(0) - 1;

#if SUB_LCU_DQP
  Int iBaseQP = rpcBestCU->getSlice()->getSliceQp();
  UChar uhLastQP = rpcBestCU->getLastCodedQP();
  Int idQP;
  Int iStartQP;
  Int iQP;

  if( (g_uiMaxCUWidth>>uiDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() )
  {
    idQP = m_pcEncCfg->getMaxDeltaQP();
    iStartQP = iBaseQP;
  }
  else
  {
    idQP = 0;
    iStartQP = rpcTempCU->getQP(0);
  }
#endif

#if FINE_GRANULARITY_SLICES
  // If slice start or slice end is within this cu...
  TComSlice * pcSlice = rpcTempCU->getPic()->getSlice(rpcTempCU->getPic()->getCurrSliceIdx());
  Bool bSliceStart = pcSlice->getEntropySliceCurStartCUAddr()>rpcTempCU->getSCUAddr()&&pcSlice->getEntropySliceCurStartCUAddr()<rpcTempCU->getSCUAddr()+rpcTempCU->getTotalNumPart();
  Bool bSliceEnd = (pcSlice->getEntropySliceCurEndCUAddr()>rpcTempCU->getSCUAddr()&&pcSlice->getEntropySliceCurEndCUAddr()<rpcTempCU->getSCUAddr()+rpcTempCU->getTotalNumPart());
  Bool bInsidePicture = ( uiRPelX < rpcBestCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < rpcBestCU->getSlice()->getSPS()->getHeight() );
  // We need to split, so don't try these modes.
  if(!bSliceEnd && !bSliceStart && bInsidePicture )
#else
  if( ( uiRPelX < rpcBestCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < rpcBestCU->getSlice()->getSPS()->getHeight() ) )
#endif
  {
#if SUB_LCU_DQP
    for (iQP=iStartQP-idQP;  iQP<=iStartQP+idQP; iQP++)
    {
      // variables for fast encoder decision
      bEarlySkip  = false;
      bTrySplit    = true;
      fRD_Skip    = MAX_DOUBLE;

      rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );

      // do inter modes, SKIP and 2Nx2N
#endif
      if( rpcBestCU->getSlice()->getSliceType() != I_SLICE )
      {
        // SKIP

        if( pcPic->getSlice(0)->getSPS()->getUseMRG() )
        {
#if !HHI_MRG_SKIP
          xCheckRDCostAMVPSkip ( rpcBestCU, rpcTempCU );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
#endif
          xCheckRDCostMerge2Nx2N( rpcBestCU, rpcTempCU );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
        }
        else
        {
          xCheckRDCostAMVPSkip ( rpcBestCU, rpcTempCU );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
        }

        // fast encoder decision for early skip
        if ( m_pcEncCfg->getUseFastEnc() )
        {
          Int iIdx = g_aucConvertToBit[ rpcBestCU->getWidth(0) ];
          if ( aiNum [ iIdx ] > 5 && fRD_Skip < EARLY_SKIP_THRES*afCost[ iIdx ]/aiNum[ iIdx ] )
          {
            bEarlySkip = true;
            bTrySplit  = false;
          }
        }

#if SUB_LCU_DQP
        // 2Nx2N, NxN
        if ( !bEarlySkip )
        {
          xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N );  rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
        }
      }

      if( (g_uiMaxCUWidth>>uiDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() )
      {
        if(iQP == iBaseQP)
          bTrySplitDQP = bTrySplit;
      }
      else
      {
        bTrySplitDQP = bTrySplit;
      }
    }


    for (iQP=iStartQP-idQP;  iQP<=iStartQP+idQP; iQP++)
    {
      rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );

      // do inter modes, NxN, 2NxN, and Nx2N
      if( rpcBestCU->getSlice()->getSliceType() != I_SLICE )
      {
#endif
        // 2Nx2N, NxN
        if ( !bEarlySkip )
        {

#if !SUB_LCU_DQP
          xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N );
          rpcTempCU->initEstData();
#endif
#if HHI_DISABLE_INTER_NxN_SPLIT
          if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
          {
            xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_NxN   );
#if SUB_LCU_DQP
            rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
            rpcTempCU->initEstData();
#endif
          }
        }

#if HHI_RMP_SWITCH
        if( pcPic->getSlice(0)->getSPS()->getUseRMP() )
#endif
        { // 2NxN, Nx2N
          xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_Nx2N  );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
          xCheckRDCostInter      ( rpcBestCU, rpcTempCU, SIZE_2NxN  );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
        }

      }

#if E057_INTRA_PCM
      // initialize PCM flag
      rpcTempCU->setIPCMFlag( 0, false);
      rpcTempCU->setIPCMFlagSubParts ( false, 0, uiDepth); //SUB_LCU_DQP
#endif


      // do normal intra modes
      if ( !bEarlySkip )
      {
        // speedup for inter frames
        if( rpcBestCU->getSlice()->getSliceType() == I_SLICE || 
          rpcBestCU->getCbf( 0, TEXT_LUMA     ) != 0   ||
          rpcBestCU->getCbf( 0, TEXT_CHROMA_U ) != 0   ||
          rpcBestCU->getCbf( 0, TEXT_CHROMA_V ) != 0     ) // avoid very complex intra if it is unlikely
        {
          xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_2Nx2N );
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
#if MTK_DISABLE_INTRA_NxN_SPLIT
          if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
          {
            if( rpcTempCU->getWidth(0) > ( 1 << rpcTempCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() ) )
            {
              xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_NxN   );
#if SUB_LCU_DQP
              rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
              rpcTempCU->initEstData();
#endif
            }
          }
        }
      }

#if E057_INTRA_PCM
      // test PCM
      if(rpcTempCU->getWidth(0) >= (1<<pcPic->getSlice(0)->getSPS()->getPCMLog2MinSize()))
      {
        UInt uiRawBits = (g_uiBitDepth * rpcBestCU->getWidth(0) * rpcBestCU->getHeight(0) * 3 / 2);
        UInt uiBestBits = rpcBestCU->getTotalBits();
        if((uiBestBits > uiRawBits) || (rpcBestCU->getTotalCost() > m_pcRdCost->calcRdCost(uiRawBits, 0)))
        {
          xCheckIntraPCM (rpcBestCU, rpcTempCU);
#if SUB_LCU_DQP
          rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );
#else
          rpcTempCU->initEstData();
#endif
        }
      }
#endif
#if SUB_LCU_DQP
    }
#endif

    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSplitFlag( rpcBestCU, 0, uiDepth, true );
    rpcBestCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
#if FINE_GRANULARITY_SLICES
    if(m_pcEncCfg->getUseSBACRD())
    {
      rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
    }
#endif
    rpcBestCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcBestCU->getTotalBits(), rpcBestCU->getTotalDistortion() );

    // accumulate statistics for early skip
    if ( m_pcEncCfg->getUseFastEnc() )
    {
      if ( rpcBestCU->isSkipped(0) )
      {
        Int iIdx = g_aucConvertToBit[ rpcBestCU->getWidth(0) ];
        afCost[ iIdx ] += rpcBestCU->getTotalCost();
        aiNum [ iIdx ] ++;
      }
    }
  }
#if FINE_GRANULARITY_SLICES
  else if(!(bSliceEnd && bInsidePicture))
#else
  else
#endif
  {
    bBoundary = true;
  }

#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) == rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() )
  {
    idQP = m_pcEncCfg->getMaxDeltaQP();
    iStartQP = iBaseQP;
  }
  else if( (g_uiMaxCUWidth>>uiDepth) > rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() )
  {
    idQP = 0;
    iStartQP = iBaseQP;
  }
  else
  {
    idQP = 0;
#if FINE_GRANULARITY_SLICES
    if( pcPic->getCU( rpcTempCU->getAddr() )->getEntropySliceStartCU(rpcTempCU->getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr())
    {
      iStartQP = rpcTempCU->getQP(0);
    }
    else
    {
      UInt uiCurSliceStartPartIdx = pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU() - rpcTempCU->getZorderIdxInCU();
      iStartQP = rpcTempCU->getQP(uiCurSliceStartPartIdx);
    }
#else
    iStartQP = rpcTempCU->getQP(0);
#endif
  }

  for (iQP=iStartQP-idQP;  iQP<=iStartQP+idQP; iQP++)
  {
    rpcTempCU->initEstData( uiDepth, iQP, uhLastQP );

    // further split
    if( bTrySplitDQP && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth )
#else
    // further split
    if( bTrySplit && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
    {
      UChar       uhNextDepth         = uiDepth+1;
      TComDataCU* pcSubBestPartCU     = m_ppcBestCU[uhNextDepth];
      TComDataCU* pcSubTempPartCU     = m_ppcTempCU[uhNextDepth];

      for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
      {
#if SUB_LCU_DQP
        pcSubBestPartCU->initSubCU( rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP );           // clear sub partition datas or init.
        pcSubTempPartCU->initSubCU( rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP );           // clear sub partition datas or init.
        pcSubBestPartCU->setLastCodedQP( rpcTempCU->getLastCodedQP() );
        pcSubTempPartCU->setLastCodedQP( rpcTempCU->getLastCodedQP() );
#else
        pcSubBestPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.
        pcSubTempPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.
#endif

#if FINE_GRANULARITY_SLICES
        Bool bInSlice = pcSubBestPartCU->getSCUAddr()+pcSubBestPartCU->getTotalNumPart()>pcSlice->getEntropySliceCurStartCUAddr()&&pcSubBestPartCU->getSCUAddr()<pcSlice->getEntropySliceCurEndCUAddr();
        if(bInSlice && ( pcSubBestPartCU->getCUPelX() < pcSlice->getSPS()->getWidth() ) && ( pcSubBestPartCU->getCUPelY() < pcSlice->getSPS()->getHeight() ) )
#else
        if( ( pcSubBestPartCU->getCUPelX() < pcSubBestPartCU->getSlice()->getSPS()->getWidth() ) && ( pcSubBestPartCU->getCUPelY() < pcSubBestPartCU->getSlice()->getSPS()->getHeight() ) )
#endif
        {
          if( m_bUseSBACRD )
          {
            if ( 0 == uiPartUnitIdx) //initialize RD with previous depth buffer
            {
              m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
            }
            else
            {
              m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
            }
          }

          xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth );

          rpcTempCU->copyPartFrom( pcSubBestPartCU, uiPartUnitIdx, uhNextDepth );         // Keep best part data to current temporary data.
          xCopyYuv2Tmp( pcSubBestPartCU->getTotalNumPart()*uiPartUnitIdx, uhNextDepth );
        }
      }

      if( !bBoundary )
      {
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeSplitFlag( rpcTempCU, 0, uiDepth, true );

        rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
#if FINE_GRANULARITY_SLICES
        if(m_pcEncCfg->getUseSBACRD())
        {
          rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
        }
#endif
      }
      rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

#if SUB_LCU_DQP
      if( (g_uiMaxCUWidth>>uiDepth) == rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() && rpcTempCU->getSlice()->getSPS()->getUseDQP())
      {
        Bool bHasRedisual = false;
        for( UInt uiBlkIdx = 0; uiBlkIdx < rpcTempCU->getTotalNumPart(); uiBlkIdx ++)
        {
#if FINE_GRANULARITY_SLICES
          if( ( pcPic->getCU( rpcTempCU->getAddr() )->getEntropySliceStartCU(uiBlkIdx+rpcTempCU->getZorderIdxInCU()) == rpcTempCU->getSlice()->getEntropySliceCurStartCUAddr() ) && 
              ( rpcTempCU->getCbf( uiBlkIdx, TEXT_LUMA ) || rpcTempCU->getCbf( uiBlkIdx, TEXT_CHROMA_U ) || rpcTempCU->getCbf( uiBlkIdx, TEXT_CHROMA_V ) ) )
#else
          if ( rpcTempCU->getCbf( uiBlkIdx, TEXT_LUMA ) || rpcTempCU->getCbf( uiBlkIdx, TEXT_CHROMA_U ) || rpcTempCU->getCbf( uiBlkIdx, TEXT_CHROMA_V ) )
#endif
          {
            bHasRedisual = true;
            break;
          }
        }

        if ( bHasRedisual )
        {
          m_pcEntropyCoder->resetBits();
#if FINE_GRANULARITY_SLICES
          if( pcPic->getCU( rpcTempCU->getAddr() )->getEntropySliceStartCU(rpcTempCU->getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr())
          {
            m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
          }
          else
          {
            UInt uiCurSliceStartPartIdx = pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU() - rpcTempCU->getZorderIdxInCU();
            m_pcEntropyCoder->encodeQP( rpcTempCU, uiCurSliceStartPartIdx, false );
          }
#else
          m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
#endif
          rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
          if(m_pcEncCfg->getUseSBACRD())
          {
            rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
          }
#endif
          rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
        }
        else
        {
#if FINE_GRANULARITY_SLICES
          if( pcPic->getCU( rpcTempCU->getAddr() )->getEntropySliceStartCU(rpcTempCU->getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr())
          {
            rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
          }
          else
          {
            UInt uiCurSliceStartPartIdx = pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU() - rpcTempCU->getZorderIdxInCU();
            rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( uiCurSliceStartPartIdx ), 0, uiDepth ); // set QP to default QP
          }
#else
          rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
#endif
        }

#if FINE_GRANULARITY_SLICES
        if( pcPic->getCU( rpcTempCU->getAddr() )->getEntropySliceStartCU(rpcTempCU->getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr())
        {
          rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
        }
        else
        {
          UInt uiCurSliceStartPartIdx = (pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU()) - rpcTempCU->getZorderIdxInCU();
          rpcTempCU->setLastCodedQP( rpcTempCU->getQP( uiCurSliceStartPartIdx ) );
        }
#else
        rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
#endif
      }
#endif

      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
      }
#if FINE_GRANULARITY_SLICES
      Bool bEntropyLimit=false;
      Bool bSliceLimit=false;
      bSliceLimit=rpcBestCU->getSlice()->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE&&(rpcBestCU->getTotalBits()>rpcBestCU->getSlice()->getSliceArgument()<<3);
      if(rpcBestCU->getSlice()->getSliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE&&m_pcEncCfg->getUseSBACRD())
      {
        if(rpcBestCU->getTotalBins()>rpcBestCU->getSlice()->getEntropySliceArgument())
        {
          bEntropyLimit=true;
        }
      }
      else if(rpcBestCU->getSlice()->getSliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE)
      {
        if(rpcBestCU->getTotalBits()>rpcBestCU->getSlice()->getEntropySliceArgument())
        {
          bEntropyLimit=true;
        }
      }
      if(rpcBestCU->getDepth(0)>=rpcBestCU->getSlice()->getPPS()->getSliceGranularity())
      {
        bSliceLimit=false;
        bEntropyLimit=false;
      }
      if(bSliceLimit||bEntropyLimit)
      {
        rpcBestCU->getTotalCost()=rpcTempCU->getTotalCost()+1;
      }
#endif
#if SUB_LCU_DQP
      xCheckBestMode( rpcBestCU, rpcTempCU, uiDepth);                                  // RD compare current larger prediction
#else
      xCheckBestMode( rpcBestCU, rpcTempCU );                                          // RD compare current larger prediction
#endif
    }                                                                                  // with sub partitioned prediction.

#if SUB_LCU_DQP
  }
#endif

  rpcBestCU->copyToPic(uiDepth);                                                     // Copy Best data to Picture for next partition prediction.

#if SUB_LCU_DQP
  xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth, uiDepth, rpcBestCU, uiLPelX, uiTPelY );   // Copy Yuv data to picture Yuv
#endif
#if FINE_GRANULARITY_SLICES
  if( bBoundary ||(bSliceEnd && bInsidePicture))
#else
  if( bBoundary )
#endif
    return;

#if !SUB_LCU_DQP
  xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth );   // Copy Yuv data to picture Yuv
#endif



  // Assert if Best prediction mode is NONE
  // Selected mode's RD-cost must be not MAX_DOUBLE.
  assert( rpcBestCU->getPartitionSize ( 0 ) != SIZE_NONE  );
  assert( rpcBestCU->getPredictionMode( 0 ) != MODE_NONE  );
  assert( rpcBestCU->getTotalCost     (   ) != MAX_DOUBLE );
}

#if FINE_GRANULARITY_SLICES
/** finish encoding a cu and handle end-of-slice conditions
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth 
 * \returns Void
 */
Void TEncCu::finishCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());

  //Calculate end address
  UInt uiCUAddr = pcCU->getSCUAddr()+uiAbsPartIdx;

  UInt uiInternalAddress = (pcSlice->getEntropySliceCurEndCUAddr()-1) % pcCU->getPic()->getNumPartInCU();
  UInt uiExternalAddress = (pcSlice->getEntropySliceCurEndCUAddr()-1) / pcCU->getPic()->getNumPartInCU();
  UInt uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiWidth = pcSlice->getSPS()->getWidth();
  UInt uiHeight = pcSlice->getSPS()->getHeight();
  while(uiPosX>=uiWidth||uiPosY>=uiHeight)
  {
    uiInternalAddress--;
    uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  uiInternalAddress++;
  if(uiInternalAddress==pcCU->getPic()->getNumPartInCU())
  {
    uiInternalAddress = 0;
    uiExternalAddress++;
  }
  UInt uiRealEndAddress = uiExternalAddress*pcCU->getPic()->getNumPartInCU()+uiInternalAddress;

  // Encode slice finish
  Bool bTerminateSlice = false;
  if (uiCUAddr+(pcCU->getPic()->getNumPartInCU()>>(uiDepth<<1)) == uiRealEndAddress)
  {
    bTerminateSlice = true;
  }
  m_pcEntropyCoder->encodeTerminatingBit( bTerminateSlice ? 1 : 0 );
  if ( bTerminateSlice )
  {
    m_pcEntropyCoder->encodeSliceFinish();
  }
  // Calculate slice end IF this CU puts us over slice bit size.
  unsigned iGranularitySize = pcCU->getPic()->getNumPartInCU()>>(pcSlice->getPPS()->getSliceGranularity()<<1);
  int iGranularityEnd = ((pcCU->getSCUAddr()+uiAbsPartIdx)/iGranularitySize)*iGranularitySize;
  if(iGranularityEnd<=pcSlice->getEntropySliceCurStartCUAddr()) 
  {
    iGranularityEnd+=max(iGranularitySize,(pcCU->getPic()->getNumPartInCU()>>(uiDepth<<1)));
  }
  // Set slice end parameter
  if(pcSlice->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE&&!pcSlice->getFinalized()&&pcSlice->getSliceBits()+m_pcBitCounter->getNumberOfWrittenBits()>pcSlice->getSliceArgument()<<3) 
  {
    pcSlice->setEntropySliceCurEndCUAddr(iGranularityEnd);
    pcSlice->setSliceCurEndCUAddr(iGranularityEnd);
    return;
  }
  // Set entropy slice end parameter
  if(m_pcEncCfg->getUseSBACRD()) 
  {
    TEncBinCABAC *pppcRDSbacCoder = (TEncBinCABAC *) m_pppcRDSbacCoder[0][CI_CURR_BEST]->getEncBinIf();
    UInt uiBinsCoded = pppcRDSbacCoder->getBinsCoded();
    if(pcSlice->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE&&!pcSlice->getFinalized()&&pcSlice->getEntropySliceCounter()+uiBinsCoded>pcSlice->getEntropySliceArgument())
    {
      pcSlice->setEntropySliceCurEndCUAddr(iGranularityEnd);
    }
  }
  else
  {
    if(pcSlice->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE&&!pcSlice->getFinalized()&&pcSlice->getEntropySliceCounter()+m_pcBitCounter->getNumberOfWrittenBits()>pcSlice->getEntropySliceArgument()) 
    {
      pcSlice->setEntropySliceCurEndCUAddr(iGranularityEnd);
    }
  }
}
#endif
/** encode a CU block recursively
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth 
 * \returns Void
 */
Void TEncCu::xEncodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
#if FINE_GRANULARITY_SLICES
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  // If slice start is within this cu...
  Bool bSliceStart = pcSlice->getEntropySliceCurStartCUAddr()>pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx&&pcSlice->getEntropySliceCurStartCUAddr()<pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx+( pcPic->getNumPartInCU() >> (uiDepth<<1) );
  // We need to split, so don't try these modes.
  if(!bSliceStart&&( uiRPelX < pcSlice->getSPS()->getWidth() ) && ( uiBPelY < pcSlice->getSPS()->getHeight() ) )
#else
  if( ( uiRPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
  {
    m_pcEntropyCoder->encodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
#if SUB_LCU_DQP
    if( (g_uiMaxCUWidth>>uiDepth) == pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
    {
      pcCU->setdQPFlag(true); 
    }
#endif
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
#if FINE_GRANULARITY_SLICES
      Bool bInSlice = pcCU->getSCUAddr()+uiAbsPartIdx+uiQNumParts>pcSlice->getEntropySliceCurStartCUAddr()&&pcCU->getSCUAddr()+uiAbsPartIdx<pcSlice->getEntropySliceCurEndCUAddr();
      if(bInSlice&&( uiLPelX < pcSlice->getSPS()->getWidth() ) && ( uiTPelY < pcSlice->getSPS()->getHeight() ) )
#else
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
      {
        xEncodeCU( pcCU, uiAbsPartIdx, uiDepth+1 );
      }
    }
#if SUB_LCU_DQP
    if( (g_uiMaxCUWidth>>uiDepth) == pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
    {
      uiAbsPartIdx -= uiQNumParts * 4;
#if FINE_GRANULARITY_SLICES
      if( pcPic->getCU( pcCU->getAddr() )->getEntropySliceStartCU(uiAbsPartIdx) == pcSlice->getEntropySliceCurStartCUAddr())
      {
        if( pcCU->getdQPFlag())
        {
          pcCU->setQPSubParts( pcCU->getRefQP( uiAbsPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
        }
        pcCU->setLastCodedQP( pcCU->getQP( uiAbsPartIdx ));
      }
      else
      {
        UInt uiCurSliceStartAbsPartIdx = pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU();
        if( pcCU->getdQPFlag())
        {
          pcCU->setQPSubParts( pcCU->getRefQP( uiCurSliceStartAbsPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
        }
        pcCU->setLastCodedQP( pcCU->getQP( uiCurSliceStartAbsPartIdx ));
      }
#else
        if( pcCU->getdQPFlag())
        {
          pcCU->setQPSubParts( pcCU->getRefQP( uiAbsPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
        }
        pcCU->setLastCodedQP( pcCU->getQP( uiAbsPartIdx ));
#endif
    }
#endif
    return;
  }
  
#if TSB_ALF_HEADER
#else
  m_pcEntropyCoder->encodeAlfCtrlFlag( pcCU, uiAbsPartIdx );
#endif
  
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
  {
    pcCU->setdQPFlag(true); 
  }
#endif
  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyCoder->encodeSkipFlag( pcCU, uiAbsPartIdx );
  }
  
  if( pcCU->isSkipped( uiAbsPartIdx ) )
  {
#if HHI_MRG_SKIP
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      m_pcEntropyCoder->encodeMergeIndex( pcCU, uiAbsPartIdx, 0 );
    } 
    else
#endif
    {
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {        
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          m_pcEntropyCoder->encodeMVPIdxPU( pcCU, uiAbsPartIdx, RefPicList( uiRefListIdx ));
        }
      }
    }
#if SUB_LCU_DQP
    if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
    {
      pcCU->setQPSubParts( pcCU->getRefQP( uiAbsPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
      pcCU->setLastCodedQP( pcCU->getQP( uiAbsPartIdx ));
    }
#endif
#if FINE_GRANULARITY_SLICES
    finishCU(pcCU,uiAbsPartIdx,uiDepth);
#endif
    return;
  }
  m_pcEntropyCoder->encodePredMode( pcCU, uiAbsPartIdx );
  
  m_pcEntropyCoder->encodePartSize( pcCU, uiAbsPartIdx, uiDepth );
  
#if E057_INTRA_PCM
  if (pcCU->isIntra( uiAbsPartIdx ) && pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N )
  {
    m_pcEntropyCoder->encodeIPCMInfo( pcCU, uiAbsPartIdx );

    if(pcCU->getIPCMFlag(uiAbsPartIdx))
    {
#if SUB_LCU_DQP
      if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
      {
        pcCU->setQPSubParts( pcCU->getRefQP(uiAbsPartIdx), uiAbsPartIdx, uiDepth ); // set QP to default QP
        pcCU->setLastCodedQP( pcCU->getQP( uiAbsPartIdx ));
      }
#endif
#if FINE_GRANULARITY_SLICES
      // Encode slice finish
      finishCU(pcCU,uiAbsPartIdx,uiDepth);
#endif
      return;
    }
  }
#endif

  // prediction Info ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyCoder->encodePredInfo( pcCU, uiAbsPartIdx );
  
  // Encode Coefficients
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, pcCU->getWidth (uiAbsPartIdx), pcCU->getHeight(uiAbsPartIdx) );

#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
  {
    if( pcCU->getdQPFlag())
    {
      pcCU->setQPSubParts( pcCU->getRefQP( uiAbsPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
    }
    pcCU->setLastCodedQP( pcCU->getQP( uiAbsPartIdx ));
  }
#endif
  // --- write terminating bit ---
#if FINE_GRANULARITY_SLICES
  finishCU(pcCU,uiAbsPartIdx,uiDepth);
#endif
}

Void TEncCu::xCheckRDCostSkip( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, Bool bSkipRes )
{
  UChar uhDepth = rpcTempCU->getDepth( 0 );
  
  rpcTempCU->setPredModeSubParts( MODE_SKIP,   0, uhDepth );
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,  0, uhDepth );
  
  m_pcPredSearch->predInterSkipSearch       ( rpcTempCU,
                                             m_ppcOrigYuv    [uhDepth],
                                             m_ppcPredYuvTemp[uhDepth],
                                             m_ppcResiYuvTemp[uhDepth],
                                             m_ppcRecoYuvTemp[uhDepth] );
  
  m_pcPredSearch->encodeResAndCalcRdInterCU ( rpcTempCU,
                                             m_ppcOrigYuv    [uhDepth],
                                             m_ppcPredYuvTemp[uhDepth],
                                             m_ppcResiYuvTemp[uhDepth],
                                             m_ppcResiYuvBest[uhDepth],
                                             m_ppcRecoYuvTemp[uhDepth],
                                             bSkipRes );
  
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uhDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() && rpcTempCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( rpcTempCU->getCbf( 0, TEXT_LUMA, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_U, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_V, 0 ) )
    {
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
      rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
      if(m_pcEncCfg->getUseSBACRD())
      {
        rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }
#endif
      rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
    }
    else
    {
      rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uhDepth ); // set QP to default QP
    }
    rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
  }

  UInt uiDepth = uhDepth;
  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth);
#else
  xCheckBestMode(rpcBestCU, rpcTempCU);
#endif
}

/** check RD costs for a CU block encoded with merge
 * \param rpcBestCU
 * \param rpcTempCU
 * \returns Void
 */
Void TEncCu::xCheckRDCostMerge2Nx2N( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  assert( rpcTempCU->getSlice()->getSliceType() != I_SLICE );
  TComMvField  cMvFieldNeighbours[MRG_MAX_NUM_CANDS << 1]; // double length for mv of both lists
  UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
  UInt uiNeighbourCandIdx[MRG_MAX_NUM_CANDS]; //MVs with same idx => same cand

  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
  {
    uhInterDirNeighbours[ui] = 0;
    uiNeighbourCandIdx[ui] = 0;
  }
  UChar uhDepth = rpcTempCU->getDepth( 0 );
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to LCU level
  rpcTempCU->getInterMergeCandidates( 0, 0, uhDepth, cMvFieldNeighbours,uhInterDirNeighbours, uiNeighbourCandIdx );
  
  Bool bValidCands = false;
  for( UInt uiMergeCand = 0; uiMergeCand < MRG_MAX_NUM_CANDS; ++uiMergeCand )
  {
    if( uiNeighbourCandIdx[uiMergeCand] == ( uiMergeCand + 1 ) )
    {
#if HHI_MRG_SKIP
      TComYuv* pcPredYuvTemp = NULL;
      for( UInt uiNoResidual = 0; uiNoResidual < 2; ++uiNoResidual )
      {
#endif
      bValidCands = true;
      // set MC parameters
#if HHI_MRG_SKIP
      rpcTempCU->setPredModeSubParts( MODE_SKIP, 0, uhDepth ); // interprets depth relative to LCU level
#else
      rpcTempCU->setPredModeSubParts( MODE_INTER, 0, uhDepth ); // interprets depth relative to LCU level
#endif
      rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to LCU level
      rpcTempCU->setMergeFlagSubParts( true, 0, 0, uhDepth ); // interprets depth relative to LCU level
      rpcTempCU->setMergeIndexSubParts( uiMergeCand, 0, 0, uhDepth ); // interprets depth relative to LCU level
      rpcTempCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeCand], 0, 0, uhDepth ); // interprets depth relative to LCU level
      for( UInt uiInner = 0; uiInner < MRG_MAX_NUM_CANDS; uiInner++ )
      {
        rpcTempCU->setNeighbourCandIdxSubParts( uiInner, uiNeighbourCandIdx[uiInner], 0, 0,uhDepth );
      }
      rpcTempCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[0 + 2*uiMergeCand].getMv(), cMvFieldNeighbours[0 + 2*uiMergeCand].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 ); // interprets depth relative to rpcTempCU level
      rpcTempCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[1 + 2*uiMergeCand].getMv(), cMvFieldNeighbours[1 + 2*uiMergeCand].getRefIdx(), SIZE_2Nx2N, 0, 0, 0 ); // interprets depth relative to rpcTempCU level

#if HHI_MRG_SKIP
      // do MC
      if ( uiNoResidual == 0 )
      {
        m_pcPredSearch->motionCompensation ( rpcTempCU, m_ppcPredYuvTemp[uhDepth] );
        // save pred adress
        pcPredYuvTemp = m_ppcPredYuvTemp[uhDepth];
        
      }
      else
      {
        if ( pcPredYuvTemp != m_ppcPredYuvTemp[uhDepth])
        {
          //adress changes take best (old temp)
          pcPredYuvTemp = m_ppcPredYuvBest[uhDepth];
        }
      }
      // estimate residual and encode everything
      m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU,
                                                m_ppcOrigYuv    [uhDepth],
                                                pcPredYuvTemp,
                                                m_ppcResiYuvTemp[uhDepth],
                                                m_ppcResiYuvBest[uhDepth],
                                                m_ppcRecoYuvTemp[uhDepth],
                                                (uiNoResidual? true:false) );     
      Bool bQtRootCbf = rpcTempCU->getQtRootCbf(0) == 1;
#else
      // do MC
      m_pcPredSearch->motionCompensation ( rpcTempCU, m_ppcPredYuvTemp[uhDepth] );

      // estimate residual and encode everything
      m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU,
                                                 m_ppcOrigYuv    [uhDepth],
                                                 m_ppcPredYuvTemp[uhDepth],
                                                 m_ppcResiYuvTemp[uhDepth],
                                                 m_ppcResiYuvBest[uhDepth],
                                                 m_ppcRecoYuvTemp[uhDepth],
                                                 false );
#endif

#if SUB_LCU_DQP
      UInt uiOrgQP = rpcTempCU->getQP( 0 );
      UInt uiOrgLastQP = rpcTempCU->getLastCodedQP();
      if( (g_uiMaxCUWidth>>uhDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() && rpcTempCU->getSlice()->getSPS()->getUseDQP() )
      {
        if ( rpcTempCU->getCbf( 0, TEXT_LUMA, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_U, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_V, 0 ) )
        {
          m_pcEntropyCoder->resetBits();
          m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
          rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
          if(m_pcEncCfg->getUseSBACRD())
          {
            rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
          }
#endif
          rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
        }
        else
        {
          rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uhDepth ); // set QP to default QP
        }
        rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
      }

      UInt uiDepth = uhDepth;
      xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth);
      rpcTempCU->initEstData( uhDepth, uiOrgQP, uiOrgLastQP );
#else
      xCheckBestMode(rpcBestCU, rpcTempCU);
      rpcTempCU->initEstData();
#endif

#if HHI_MRG_SKIP
      if (!bQtRootCbf)
        break;
      }
#endif
    }
  }
  if( bValidCands )
  {
    for( UInt uiMergeCand = 0; uiMergeCand < MRG_MAX_NUM_CANDS; uiMergeCand++ )
    {
      rpcBestCU->setNeighbourCandIdxSubParts( uiMergeCand, uiNeighbourCandIdx[uiMergeCand], 0, 0,uhDepth );
    }
  }
}

Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize )
{
  UChar uhDepth = rpcTempCU->getDepth( 0 );
  
  rpcTempCU->setDepthSubParts( uhDepth, 0 );
  
  rpcTempCU->setPartSizeSubParts  ( ePartSize,  0, uhDepth );
  rpcTempCU->setPredModeSubParts  ( MODE_INTER, 0, uhDepth );
  
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] );
#if PART_MRG
  if (rpcTempCU->getSlice()->getSPS()->getUseMRG() && rpcTempCU->getWidth(0) > 8 && !rpcTempCU->getMergeFlag(0) && (ePartSize != SIZE_2Nx2N && ePartSize != SIZE_NxN))
  {
    return;
  }
#endif
  m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcResiYuvBest[uhDepth], m_ppcRecoYuvTemp[uhDepth], false );
  
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uhDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() && rpcTempCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( rpcTempCU->getCbf( 0, TEXT_LUMA, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_U, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_V, 0 ) )
    {
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
      rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
      if(m_pcEncCfg->getUseSBACRD())
      {
        rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }
#endif
    }
    else
    {
      rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uhDepth ); // set QP to default QP
    }
    rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
  }

  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
  UInt uiDepth = uhDepth;
  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth);
#else
  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
  xCheckBestMode(rpcBestCU, rpcTempCU);
#endif
}

Void TEncCu::xCheckRDCostIntra( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize eSize )
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );
  
  rpcTempCU->setPartSizeSubParts( eSize, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  
  Bool bSeparateLumaChroma = true; // choose estimation mode
  UInt uiPreCalcDistC      = 0;
  if( !bSeparateLumaChroma )
  {
    m_pcPredSearch->preestChromaPredMode( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth] );
  }
  m_pcPredSearch  ->estIntraPredQT      ( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], uiPreCalcDistC, bSeparateLumaChroma );

#if LM_CHROMA
  m_ppcRecoYuvTemp[uiDepth]->copyToPicLuma(rpcTempCU->getPic()->getPicYuvRec(), rpcTempCU->getAddr(), rpcTempCU->getZorderIdxInCU() );
#endif 
  
  m_pcPredSearch  ->estIntraPredChromaQT( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], uiPreCalcDistC );
  
  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePredMode( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePartSize( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodePredInfo( rpcTempCU, 0,          true );
#if E057_INTRA_PCM
  m_pcEntropyCoder->encodeIPCMInfo(rpcTempCU, 0, true );
#endif

  // Encode Coefficients
  m_pcEntropyCoder->encodeCoeff( rpcTempCU, 0, uiDepth, rpcTempCU->getWidth (0), rpcTempCU->getHeight(0) );
  
  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
  
  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
#if FINE_GRANULARITY_SLICES
  if(m_pcEncCfg->getUseSBACRD())
  {
    rpcBestCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
  }
#endif
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
  
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) >= rpcTempCU->getSlice()->getPPS()->getMinCuDQPSize() && rpcTempCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( rpcTempCU->getCbf( 0, TEXT_LUMA, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_U, 0 ) || rpcTempCU->getCbf( 0, TEXT_CHROMA_V, 0 ) )
    {
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( rpcTempCU, 0, false );
      rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
#if FINE_GRANULARITY_SLICES
      if(m_pcEncCfg->getUseSBACRD())
      {
        rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      }
#endif
      rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );
    }
    else
    {
      rpcTempCU->setQPSubParts( rpcTempCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
    }
    rpcTempCU->setLastCodedQP( rpcTempCU->getQP( 0 ) );
  }

  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth);
#else
  xCheckBestMode( rpcBestCU, rpcTempCU );
#endif
}

#if E057_INTRA_PCM
/** Check R-D costs for a CU with PCM mode. 
 * \param rpcBestCU pointer to best mode CU data structure
 * \param rpcTempCU pointer to testing mode CU data structure
 * \returns Void
 * 
 * \note Current PCM implementation encodes sample values in a lossless way. The distortion of PCM mode CUs are zero. PCM mode is selected if the best mode yields bits greater than that of PCM mode.
 */
Void TEncCu::xCheckIntraPCM( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setIPCMFlag(0, true);
  rpcTempCU->setIPCMFlagSubParts (true, 0, rpcTempCU->getDepth(0));
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );

  m_pcPredSearch->IPCMSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth]);

  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePredMode ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePartSize ( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodeIPCMInfo ( rpcTempCU, 0, true );

  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
#if FINE_GRANULARITY_SLICES
  if(m_pcEncCfg->getUseSBACRD())
  {
    rpcBestCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
  }
#endif
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckBestMode( rpcBestCU, rpcTempCU );
}
#endif

// check whether current try is the best
Void TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  if( rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost() )
  {
    TComYuv* pcYuv;
    UChar uhDepth = rpcBestCU->getDepth(0);
    if( rpcTempCU->getSlice()->getSPS()->getUseMRG() && uhDepth == rpcTempCU->getDepth( 0 ) && rpcTempCU->getSlice()->getSliceType() != I_SLICE && rpcTempCU->getPartitionSize( 0 ) == SIZE_2Nx2N && !rpcTempCU->getMergeFlag( 0 ) )
    {
      for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
      {
        rpcTempCU->setNeighbourCandIdxSubParts( ui, rpcBestCU->getNeighbourCandIdx( ui, 0 ), 0, 0, uhDepth );
      }          
    }      
    
    // Change Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;
    
    // Change Prediction data
    pcYuv = m_ppcPredYuvBest[uhDepth];
    m_ppcPredYuvBest[uhDepth] = m_ppcPredYuvTemp[uhDepth];
    m_ppcPredYuvTemp[uhDepth] = pcYuv;
    
    // Change Reconstruction data
    pcYuv = m_ppcRecoYuvBest[uhDepth];
    m_ppcRecoYuvBest[uhDepth] = m_ppcRecoYuvTemp[uhDepth];
    m_ppcRecoYuvTemp[uhDepth] = pcYuv;
    
    pcYuv = NULL;
    pcCU  = NULL;
    
    if( m_bUseSBACRD )  // store temp best CI for next CU coding
      m_pppcRDSbacCoder[uhDepth][CI_TEMP_BEST]->store(m_pppcRDSbacCoder[uhDepth][CI_NEXT_BEST]);
  }
}

#if SUB_LCU_DQP
/** check whether current try is the best with identifying the depth of current try
 * \param rpcBestCU
 * \param rpcTempCU
 * \returns Void
 */
Void TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth )
{
  if( rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost() )
  {
    TComYuv* pcYuv;
    if( rpcTempCU->getSlice()->getSPS()->getUseMRG() && uiDepth == rpcTempCU->getDepth( 0 ) && rpcTempCU->getSlice()->getSliceType() != I_SLICE && rpcTempCU->getPartitionSize( 0 ) == SIZE_2Nx2N && !rpcTempCU->getMergeFlag( 0 ) )
    {
      for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
      {
        rpcTempCU->setNeighbourCandIdxSubParts( ui, rpcBestCU->getNeighbourCandIdx( ui, 0 ), 0, 0, uiDepth );
      }          
    }      

    // Change Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;

    // Change Prediction data
    pcYuv = m_ppcPredYuvBest[uiDepth];
    m_ppcPredYuvBest[uiDepth] = m_ppcPredYuvTemp[uiDepth];
    m_ppcPredYuvTemp[uiDepth] = pcYuv;

    // Change Reconstruction data
    pcYuv = m_ppcRecoYuvBest[uiDepth];
    m_ppcRecoYuvBest[uiDepth] = m_ppcRecoYuvTemp[uiDepth];
    m_ppcRecoYuvTemp[uiDepth] = pcYuv;

    pcYuv = NULL;
    pcCU  = NULL;

    if( m_bUseSBACRD )  // store temp best CI for next CU coding
      m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);
  }
}
#endif

Void TEncCu::xCheckRDCostAMVPSkip           ( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  UChar uhDepth = rpcTempCU->getDepth(0);
  
  AMVPInfo cAMVPInfo0;
  cAMVPInfo0.iN = 0;
  
  AMVPInfo cAMVPInfo1;
  cAMVPInfo1.iN = 0;
  
  if (rpcTempCU->getAMVPMode(0) == AM_EXPL)
  {
    rpcTempCU->setPredModeSubParts( MODE_SKIP, 0, uhDepth );
    rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,  0, uhDepth );
    
    if ( rpcTempCU->getSlice()->isInterP() && rpcTempCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
    {
      rpcTempCU->fillMvpCand(0, 0, REF_PIC_LIST_0, 0, &cAMVPInfo0);
    }
    else if ( rpcTempCU->getSlice()->isInterB() &&
             rpcTempCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 &&
             rpcTempCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0  )
    {
      rpcTempCU->fillMvpCand(0, 0, REF_PIC_LIST_0, 0, &cAMVPInfo0);
      rpcTempCU->fillMvpCand(0, 0, REF_PIC_LIST_1, 0, &cAMVPInfo1);
    }
    else
    {
      assert( 0 );
    }
  }
  
  Int iMVP0, iMVP1;
  
#if SUB_LCU_DQP
  UInt uiOrgQP = rpcTempCU->getQP( 0 );
  UInt uiOrgLastQP = rpcTempCU->getLastCodedQP();
#endif

  for (iMVP0 = (cAMVPInfo0.iN > 0? 0:-1); iMVP0 < cAMVPInfo0.iN; iMVP0++)
  {
    for (iMVP1 = (cAMVPInfo1.iN > 0? 0:-1); iMVP1 < cAMVPInfo1.iN; iMVP1++)
    {
      rpcTempCU->setPredModeSubParts( MODE_SKIP, 0, uhDepth );
      rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,  0, uhDepth );

      if (rpcTempCU->getSlice()->isInterB())
        rpcTempCU->setInterDirSubParts( 3, 0, 0, uhDepth );

      rpcTempCU->setMVPIdxSubParts( iMVP0, REF_PIC_LIST_0, 0, 0, uhDepth );
      rpcTempCU->setMVPIdxSubParts( iMVP1, REF_PIC_LIST_1, 0, 0, uhDepth );

      rpcTempCU->setMVPNumSubParts( cAMVPInfo0.iN, REF_PIC_LIST_0, 0, 0, uhDepth );
      rpcTempCU->setMVPNumSubParts( cAMVPInfo1.iN, REF_PIC_LIST_1, 0, 0, uhDepth );

      xCopyAMVPInfo(&cAMVPInfo0, rpcTempCU->getCUMvField(REF_PIC_LIST_0)->getAMVPInfo());
      xCopyAMVPInfo(&cAMVPInfo1, rpcTempCU->getCUMvField(REF_PIC_LIST_1)->getAMVPInfo());
      xCheckRDCostSkip ( rpcBestCU, rpcTempCU, true );      
#if SUB_LCU_DQP
      rpcTempCU->initEstData( uhDepth, uiOrgQP, uiOrgLastQP );
#else
      rpcTempCU->initEstData();
#endif
    }
  }
}

Void TEncCu::xCopyAMVPInfo (AMVPInfo* pSrc, AMVPInfo* pDst)
{
  pDst->iN = pSrc->iN;
  for (Int i = 0; i < pSrc->iN; i++)
  {
    pDst->m_acMvCand[i] = pSrc->m_acMvCand[i];
  }
}
#if SUB_LCU_DQP
Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsPartIdx, UInt uiDepth, UInt uiSrcDepth, TComDataCU* pcCU, UInt uiLPelX, UInt uiTPelY )
#else
Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth)
#endif
{
#if SUB_LCU_DQP
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
#if FINE_GRANULARITY_SLICES
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  Bool bSliceStart = pcSlice->getEntropySliceCurStartCUAddr()>pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx&&pcSlice->getEntropySliceCurStartCUAddr()<pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx+( pcCU->getPic()->getNumPartInCU() >> (uiDepth<<1) );
  Bool bSliceEnd   = pcSlice->getEntropySliceCurEndCUAddr()>pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx&&pcSlice->getEntropySliceCurEndCUAddr()<pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx+( pcCU->getPic()->getNumPartInCU() >> (uiDepth<<1) );
  if(!bSliceEnd && !bSliceStart && ( uiRPelX < pcSlice->getSPS()->getWidth() ) && ( uiBPelY < pcSlice->getSPS()->getHeight() ) )
#else
  if( ( uiRPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
  {
    UInt uiAbsPartIdxInRaster = g_auiZscanToRaster[uiAbsPartIdx];
    UInt uiSrcBlkWidth = rpcPic->getNumPartInWidth() >> (uiSrcDepth);
    UInt uiBlkWidth    = rpcPic->getNumPartInWidth() >> (uiDepth);
    UInt uiPartIdxX = ( ( uiAbsPartIdxInRaster % rpcPic->getNumPartInWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
    UInt uiPartIdxY = ( ( uiAbsPartIdxInRaster / rpcPic->getNumPartInWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
    UInt uiPartIdx = uiPartIdxY * ( uiSrcBlkWidth / uiBlkWidth ) + uiPartIdxX;
    m_ppcRecoYuvBest[uiSrcDepth]->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsPartIdx, uiDepth - uiSrcDepth, uiPartIdx);
  }
  else
  {
    UInt uiQNumParts = ( pcCU->getPic()->getNumPartInCU() >> (uiDepth<<1) )>>2;

    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      UInt uiSubCULPelX   = uiLPelX + ( g_uiMaxCUWidth >>(uiDepth+1) )*( uiPartUnitIdx &  1 );
      UInt uiSubCUTPelY   = uiTPelY + ( g_uiMaxCUHeight>>(uiDepth+1) )*( uiPartUnitIdx >> 1 );

#if FINE_GRANULARITY_SLICES
      Bool bInSlice = pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx+uiQNumParts>pcSlice->getEntropySliceCurStartCUAddr()&&pcCU->getAddr()*pcCU->getPic()->getNumPartInCU()+uiAbsPartIdx<pcSlice->getEntropySliceCurEndCUAddr();
      if(bInSlice&&( uiSubCULPelX < pcSlice->getSPS()->getWidth() ) && ( uiSubCUTPelY < pcSlice->getSPS()->getHeight() ) )
#else
      if( ( uiSubCULPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiSubCUTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
      {
        xCopyYuv2Pic( rpcPic, uiCUAddr, uiAbsPartIdx, uiDepth+1, uiSrcDepth, pcCU, uiSubCULPelX, uiSubCUTPelY );   // Copy Yuv data to picture Yuv
      }
    }
  }
#else
  m_ppcRecoYuvBest[uiDepth]->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsZorderIdx );
#endif
}

Void TEncCu::xCopyYuv2Tmp( UInt uiPartUnitIdx, UInt uiNextDepth )
{
  UInt uiCurrDepth = uiNextDepth - 1;
  m_ppcRecoYuvBest[uiNextDepth]->copyToPartYuv( m_ppcRecoYuvTemp[uiCurrDepth], uiPartUnitIdx );
}

