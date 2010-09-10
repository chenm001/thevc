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

/** \param    uiTotalDepth  total number of allowable depth
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

    m_ppcBestCU[i] = new TComDataCU; m_ppcBestCU[i]->create( uiNumPartitions, uiWidth, uiHeight, false );
    m_ppcTempCU[i] = new TComDataCU; m_ppcTempCU[i]->create( uiNumPartitions, uiWidth, uiHeight, false );

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
  m_pcBinMultiCABAC    = pcEncTop->getBinMultiCABAC();
  m_pcBinPIPE          = pcEncTop->getBinPIPE();
  m_pcBinMultiPIPE     = pcEncTop->getBinMultiPIPE();
  m_pcBinV2VwLB        = pcEncTop->getBinV2VwLB();
  m_pcBinCABAC4V2V     = pcEncTop->getBinCABAC4V2V();

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
      m_ppcBestCU[0]->getTotalCost()  = m_pcRdCost->calcRdCost( m_ppcBestCU[0]->getTotalBits(), m_ppcBestCU[0]->getTotalDistortion() );
    }
  }
}

/** \param  pcCU  pointer of CU data class
 */
Void TEncCu::encodeCU ( TComDataCU* pcCU )
{
  // encode CU data
  xEncodeCU( pcCU, 0, 0 );

  // dQP: only for LCU
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
    {
    }
    else
    {
      m_pcEntropyCoder->encodeQP( pcCU, 0 );
    }
  }

  //--- write terminating bit ---
  Bool bTerminateSlice = ( pcCU->getAddr() == pcCU->getPic()->getNumCUsInFrame()-1) ? true : false;
  m_pcEntropyCoder->encodeTerminatingBit( bTerminateSlice ? 1 : 0 );

  // Encode slice finish
  if ( bTerminateSlice )
  {
    m_pcEntropyCoder->encodeSliceFinish();
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth )
{
  TComPic* pcPic = rpcBestCU->getPic();

  // get Original YUV data from picture
  m_ppcOrigYuv[uiDepth]->copyFromPicYuv( pcPic->getPicYuvOrg(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU() );

  // variables for fast encoder decision
  TComDataCU* pcTempCU;
  Bool    bEarlySkip  = false;
  Bool    bTrySplit    = true;
  Bool    bTryAsym    = true;
  Double  fRD_Skip    = MAX_DOUBLE;

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

  if( ( uiRPelX < rpcBestCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < rpcBestCU->getSlice()->getSPS()->getHeight() ) )
  {
    // do inter modes
    if( pcPic->getSlice()->getSliceType() != I_SLICE )
    {
      // SKIP
      pcTempCU = rpcTempCU;

#if HHI_MRG
#if HHI_MRG_PU
      if( !pcPic->getSlice()->getSPS()->getUseMRG() )
#else
      if( pcPic->getSlice()->getSPS()->getUseMRG() )
      {
        xCheckRDCostMerge( rpcBestCU, rpcTempCU );            rpcTempCU->initEstData();
      }
      else
#endif
      {
        xCheckRDCostAMVPSkip ( rpcBestCU, rpcTempCU );        rpcTempCU->initEstData();
      }
#else
      xCheckRDCostAMVPSkip ( rpcBestCU, rpcTempCU );        rpcTempCU->initEstData();
#endif

#if HHI_MRG
      if( !pcPic->getSlice()->getSPS()->getUseMRG() )
      {
        // fast encoder decision for early skip
        if ( m_pcEncCfg->getUseFastEnc() )
        {
          Int iIdx = g_aucConvertToBit[ rpcBestCU->getWidth(0) ];
          if ( aiNum [ iIdx ] > 5 && fRD_Skip < EARLY_SKIP_THRES*afCost[ iIdx ]/aiNum[ iIdx ] )
          {
            bEarlySkip = true;
            bTrySplit  = false;
            bTryAsym   = false;
          }
        }
      }
#else
      // fast encoder decision for early skip
      if ( m_pcEncCfg->getUseFastEnc() )
      {
        Int iIdx = g_aucConvertToBit[ rpcBestCU->getWidth(0) ];
        if ( aiNum [ iIdx ] > 5 && fRD_Skip < EARLY_SKIP_THRES*afCost[ iIdx ]/aiNum[ iIdx ] )
        {
          bEarlySkip = true;
          bTrySplit  = false;
          bTryAsym   = false;
        }
      }
#endif

      // 2Nx2N, NxN
      if ( !bEarlySkip )
      {
#if HHI_DISABLE_INTER_NxN_SPLIT
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N );  rpcTempCU->initEstData();
        if( rpcTempCU->getWidth( 0 ) == 8 )
        {
          xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_NxN   );  rpcTempCU->initEstData();
        }
#else
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2Nx2N );  rpcTempCU->initEstData();
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_NxN   );  rpcTempCU->initEstData();
#endif
      }

#if HHI_RMP_SWITCH
      if( pcPic->getSlice()->getSPS()->getUseRMP() )
#endif
      { // 2NxN, Nx2N
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_Nx2N  );  rpcTempCU->initEstData();
        xCheckRDCostInter      ( rpcBestCU, rpcTempCU, SIZE_2NxN  );  rpcTempCU->initEstData();
      }

      // fast encoder decision for asymmetric motion partition: try asymmetric motion partition only when best != 2Nx2N
      if ( m_pcEncCfg->getUseFastEnc() )
      {
        // Best is skip or 2Nx2N
#if !SAMSUNG_REMOVE_AMP_FEN_PENALTY
        if ( rpcBestCU->getPartitionSize(0) == SIZE_2Nx2N ) bTryAsym = false;
#endif
      }

      // SIZE_2NxnU, SIZE_2NxnD, SIZE_nLx2N, SIZE_nRx2N
      if( bTryAsym && pcPic->getSlice()->getSPS()->getAMPAcc(uiDepth) )
      {
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU );   rpcTempCU->initEstData();
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD );   rpcTempCU->initEstData();
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N );   rpcTempCU->initEstData();
        xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N );   rpcTempCU->initEstData();
      }
    }

    // initialize special intra tools
    rpcTempCU->setROTindex( 0, 0 );
    rpcTempCU->setROTindexSubParts( 0, 0, rpcTempCU->getDepth(0) );

    rpcTempCU->setCIPflag ( 0, 0 );
    rpcTempCU->setCIPflagSubParts ( 0, 0, rpcTempCU->getDepth(0) );

#if PLANAR_INTRA
    rpcTempCU->setPlanarInfo( 0, PLANAR_FLAG, 0 );
    rpcTempCU->setPlanarInfoSubParts ( 0, 0, 0, 0, 0, rpcTempCU->getDepth(0) );
#endif

    // do normal intra modes
#if HHI_RQT_INTRA
    if ( !bEarlySkip && rpcTempCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
    {
#if 1 // speedup for inter frames
      if( pcPic->getSlice()->getSliceType() == I_SLICE || 
          rpcBestCU->getCbf( 0, TEXT_LUMA     ) != 0   ||
          rpcBestCU->getCbf( 0, TEXT_CHROMA_U ) != 0   ||
          rpcBestCU->getCbf( 0, TEXT_CHROMA_V ) != 0     ) // avoid very complex intra if it is unlikely
#endif
      {
        xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_2Nx2N ); rpcTempCU->initEstData();
        if( rpcTempCU->getWidth(0) > ( 1 << rpcTempCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() ) )
        {
          xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_NxN   ); rpcTempCU->initEstData();
        }
      }
    }
    else
#endif
    if ( !bEarlySkip )
    {
      xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_2Nx2N ); rpcTempCU->initEstData();
      xCheckRDCostIntra( rpcBestCU, rpcTempCU, SIZE_NxN   ); rpcTempCU->initEstData();
    }

    // try special intra tool only when best = intra
    if ( rpcBestCU->getPredictionMode(0) == MODE_INTRA )
    {
      PartSize eSize = rpcBestCU->getPartitionSize(0);
#if PLANAR_INTRA
      // Try planar mode
      if ( !rpcTempCU->getSlice()->isInterB() )
      {
        rpcTempCU->setPlanarInfo( 0, PLANAR_FLAG, 1 );
        rpcTempCU->setPlanarInfoSubParts ( 1, 0, 0, 0, 0, rpcTempCU->getDepth(0) );
        xCheckPlanarIntra(rpcBestCU, rpcTempCU);  rpcTempCU->initEstData();
      }
#endif
#if HHI_ALLOW_CIP_SWITCH
      if ( rpcTempCU->getSlice()->getSPS()->getUseCIP() )
#else
      if (1)
#endif
      {
        rpcTempCU->setCIPflag( 0, 1 );
        rpcTempCU->setCIPflagSubParts( 1, 0, rpcTempCU->getDepth(0) );
        xCheckRDCostIntra(rpcBestCU, rpcTempCU, eSize);  rpcTempCU->initEstData();
      }

      // try ROT
#if QC_MDDT || DISABLE_ROT_LUMA_4x4_8x8
      if ((rpcBestCU->getWidth(0) > 16) || ((rpcBestCU->getWidth(0) == 16) && (eSize == SIZE_2Nx2N)))
#endif
      {
        // try ROT without CIP
        if ( rpcTempCU->getSlice()->getSPS()->getUseROT() )
        {
          for (UChar indexROT = 1; indexROT<ROT_DICT; indexROT++)
          {
            rpcTempCU->setROTindex(0,indexROT);
            rpcTempCU->setROTindexSubParts( indexROT, 0, rpcTempCU->getDepth(0) );
            rpcTempCU->setCIPflag( 0, 0 );
            rpcTempCU->setCIPflagSubParts( 0, 0, rpcTempCU->getDepth(0) );
            xCheckRDCostIntra(rpcBestCU, rpcTempCU, eSize);  rpcTempCU->initEstData();
          }
        }
      }
    }

    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeSplitFlag( rpcBestCU, 0, uiDepth, true );
    rpcBestCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
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
  else
  {
    bBoundary = true;
  }

  // further split
  if( bTrySplit && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth )
  {
    UChar       uhNextDepth         = uiDepth+1;
    TComDataCU* pcSubBestPartCU     = m_ppcBestCU[uhNextDepth];
    TComDataCU* pcSubTempPartCU     = m_ppcTempCU[uhNextDepth];

    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      pcSubBestPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.
      pcSubTempPartCU->initSubCU( rpcBestCU, uiPartUnitIdx, uhNextDepth );           // clear sub partition datas or init.

      if( ( pcSubBestPartCU->getCUPelX() < pcSubBestPartCU->getSlice()->getSPS()->getWidth() ) && ( pcSubBestPartCU->getCUPelY() < pcSubBestPartCU->getSlice()->getSPS()->getHeight() ) )
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
    }
    rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

    if( m_bUseSBACRD )
    {
      m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
    }

    xCheckBestMode( rpcBestCU, rpcTempCU );                                          // RD compare current larger prediction
  }                                                                                  // with sub partitioned prediction.

  rpcBestCU->copyToPic(uiDepth);                                                     // Copy Best data to Picture for next partition prediction.

  if( bBoundary )
    return;

  xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getAddr(), rpcBestCU->getZorderIdxInCU(), uiDepth );   // Copy Yuv data to picture Yuv

  // Assert if Best prediction mode is NONE
  // Selected mode's RD-cost must be not MAX_DOUBLE.
  assert( rpcBestCU->getPartitionSize ( 0 ) != SIZE_NONE  );
  assert( rpcBestCU->getPredictionMode( 0 ) != MODE_NONE  );
  assert( rpcBestCU->getTotalCost     (   ) != MAX_DOUBLE );
}

Void TEncCu::xEncodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
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
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xEncodeCU( pcCU, uiAbsPartIdx, uiDepth+1 );
    }
    return;
  }

#if HHI_ALF
  if( !pcCU->getSlice()->getSPS()->getALFSeparateQt() )
  {
    m_pcEntropyCoder->encodeAlfCtrlFlag( pcCU, uiAbsPartIdx, false , false );
  }
#else
  m_pcEntropyCoder->encodeAlfCtrlFlag( pcCU, uiAbsPartIdx );
#endif

  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyCoder->encodeSkipFlag( pcCU, uiAbsPartIdx );
  }

  if( pcCU->isSkipped( uiAbsPartIdx ) )
  {
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_0);
    }
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
    {
      m_pcEntropyCoder->encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_1);
    }
#ifdef DCM_PBIC
    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      m_pcEntropyCoder->encodeICPIdx( pcCU, uiAbsPartIdx );
    }
#endif
    return;
  }

#if HHI_MRG && !HHI_MRG_PU
  m_pcEntropyCoder->encodeMergeInfo( pcCU, uiAbsPartIdx );
#endif
  m_pcEntropyCoder->encodePredMode( pcCU, uiAbsPartIdx );

#if PLANAR_INTRA
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcEntropyCoder->encodePlanarInfo( pcCU, uiAbsPartIdx );
    if ( pcCU->getPlanarInfo   ( uiAbsPartIdx, PLANAR_FLAG ) )
      return;
  }
#endif

  m_pcEntropyCoder->encodePartSize( pcCU, uiAbsPartIdx, uiDepth );

  // prediction Info ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyCoder->encodePredInfo( pcCU, uiAbsPartIdx );

  // Encode Coefficients
#if QC_MDDT//ADAPTIVE_SCAN
  g_bUpdateStats = true;
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, pcCU->getWidth (uiAbsPartIdx), pcCU->getHeight(uiAbsPartIdx) );
#else
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, pcCU->getWidth (uiAbsPartIdx), pcCU->getHeight(uiAbsPartIdx) );
  // ROT index
  if ( pcCU->getSlice()->getSPS()->getUseROT() )
  {
#if DISABLE_ROT_LUMA_4x4_8x8
    if (pcCU->getWidth (uiAbsPartIdx) > 8)
#endif
    m_pcEntropyCoder->encodeROTindex( pcCU, uiAbsPartIdx, uiDepth );
  }
#endif


  // CIP index
#if HHI_ALLOW_CIP_SWITCH
  if ( pcCU->isIntra( uiAbsPartIdx ) && pcCU->getSlice()->getSPS()->getUseCIP() )
#else
  if ( pcCU->isIntra( uiAbsPartIdx ) )
#endif
  {
    m_pcEntropyCoder->encodeCIPflag( pcCU, uiAbsPartIdx, uiDepth );
  }
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
#if HHI_RQT
                                              m_ppcResiYuvBest[uhDepth],
#endif
                                              m_ppcRecoYuvTemp[uhDepth],
                                              bSkipRes );

  xCheckBestMode(rpcBestCU, rpcTempCU);
}

#if HHI_MRG && !HHI_MRG_PU
Void TEncCu::xCheckRDCostMerge( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  // test if Top and Left exist
  // test if top and left are inter
  // get the MVs from Top and left neighbours
  // test if MVs from Top and Left are the same or not
  // if they are the same, only test one
  // if they are different, test both
  UInt        uiNeighbourInfos = 0;
  TComMvField  cMFieldNeighbours[4]; // 0: above ref_list 0, above ref list 1, left ref list 0, left ref list 1
  UChar uhInterDirNeighbours[2];

#ifdef DCM_PBIC
  TComIc cIcNeighbours[2]; // 0: above 1: left
  rpcTempCU->getInterMergeCandidates( 0, cMFieldNeighbours, cIcNeighbours,uhInterDirNeighbours, uiNeighbourInfos );
#else
  rpcTempCU->getInterMergeCandidates( 0, cMFieldNeighbours, uhInterDirNeighbours, uiNeighbourInfos );
#endif

  // uiNeighbourInfos (binary):
  // 000: no merge candidate
  // 001: only above is merge candidate
  // 010: only left is merge candidate
  // 011: above and left are different candidates
  // 100: above and left have the same motion parameters. only one merge candidate exists.

  UInt uiNeighbourIdx = 0; // 0: top, 1: left
  if ( uiNeighbourInfos == 2 )
  {
    // test left parameters first
    uiNeighbourIdx = 1;
  }
 
  UChar uhDepth = rpcTempCU->getDepth( 0 );
    
  while ( uiNeighbourInfos > 0 && uiNeighbourIdx < 2 )
  {
    rpcTempCU->setPredModeSubParts( MODE_INTER, 0, uhDepth );
    rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,  0, uhDepth );
    rpcTempCU->setMergeFlagSubParts( true, 0, uhDepth );
    rpcTempCU->setMergeIndexSubParts( uiNeighbourIdx, 0, uhDepth );

    TComMvField cMvFieldNeighbourToTest[2];
    UChar uhInterDirNeighbourToTest;
    cMvFieldNeighbourToTest[0] = cMFieldNeighbours[0 + 2*uiNeighbourIdx];
    cMvFieldNeighbourToTest[1] = cMFieldNeighbours[1 + 2*uiNeighbourIdx];
    uhInterDirNeighbourToTest = uhInterDirNeighbours[uiNeighbourIdx];

    m_pcPredSearch->predInterMergeSearch( rpcTempCU,
                                          m_ppcOrigYuv    [uhDepth],
                                          m_ppcPredYuvTemp[uhDepth],
                                          m_ppcResiYuvTemp[uhDepth],
                                          m_ppcRecoYuvTemp[uhDepth],
                                          cMvFieldNeighbourToTest,
#ifdef DCM_PBIC
                                          cIcNeighbours[uiNeighbourIdx],
#endif
                                          uhInterDirNeighbourToTest);

    m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU,
                                               m_ppcOrigYuv    [uhDepth],
                                               m_ppcPredYuvTemp[uhDepth],
                                               m_ppcResiYuvTemp[uhDepth],
#if HHI_RQT
                                               m_ppcResiYuvBest[uhDepth],
#endif
                                               m_ppcRecoYuvTemp[uhDepth],
                                               false );
    xCheckBestMode(rpcBestCU, rpcTempCU);

    if ( uiNeighbourInfos != 3 )
    {
      // there is only one merge candidate
      // only one candidate has to be tested.
      break;
    }
    else
    {
      uiNeighbourIdx++;
      rpcTempCU->initEstData();
    }
  }

}
#endif

Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize )
{
  UChar uhDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setDepthSubParts( uhDepth, 0 );

  rpcTempCU->setPartSizeSubParts  ( ePartSize,  0, uhDepth );
  rpcTempCU->setPredModeSubParts  ( MODE_INTER, 0, uhDepth );

  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] );
#if HHI_RQT
  m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcResiYuvBest[uhDepth], m_ppcRecoYuvTemp[uhDepth], false );
#else
  m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth], false );
#endif

  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckBestMode( rpcBestCU, rpcTempCU );
}

#if PLANAR_INTRA
Void TEncCu::xCheckPlanarIntra( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  rpcTempCU->setPlanarInfoSubParts ( 1, 0, 0, 0, 0, uiDepth );

  m_pcPredSearch->predIntraPlanarSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth] );

  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
#if HHI_MRG && !HHI_MRG_PU
  m_pcEntropyCoder->encodeMergeInfo( rpcTempCU, 0,          true );
#endif
  m_pcEntropyCoder->encodePredMode( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePlanarInfo( rpcTempCU, 0,        true );

  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckBestMode( rpcBestCU, rpcTempCU );
}
#endif

Void TEncCu::xCheckRDCostIntra( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize eSize )
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setPartSizeSubParts( eSize, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );

#if HHI_RQT_INTRA
  if( rpcTempCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
  {
    Bool bSeparateLumaChroma = true; // choose estimation mode
    UInt uiPreCalcDistC      = 0;
    if( !bSeparateLumaChroma )
    {
      m_pcPredSearch->preestChromaPredMode( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth] );
    }
    m_pcPredSearch  ->estIntraPredQT      ( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], uiPreCalcDistC, bSeparateLumaChroma );
    m_pcPredSearch  ->estIntraPredChromaQT( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], uiPreCalcDistC );
  }
  else
  {
#endif

    m_pcPredSearch->predIntraLumaAdiSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth] );

    UInt uiChromaTrMode = 0;

    UInt uiWidthInBit  = g_aucConvertToBit[rpcTempCU->getWidth(0)>>1]+2;
    UInt uiTrSizeInBit = g_aucConvertToBit[rpcTempCU->getSlice()->getSPS()->getMaxTrSize()]+2;
    uiChromaTrMode     = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

    rpcTempCU->setCuCbfLuma( 0, rpcTempCU->getTransformIdx(0) );

    if( m_bUseSBACRD ) m_pppcRDSbacCoder[uiDepth][CI_CHROMA_INTRA]->load( m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST] );

    m_pcPredSearch->predIntraChromaAdiSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], uiChromaTrMode );
    rpcTempCU->setCuCbfChroma( 0, uiChromaTrMode );

    if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

#if HHI_RQT_INTRA
  }
#endif

  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
#if HHI_MRG && !HHI_MRG_PU
  m_pcEntropyCoder->encodeMergeInfo( rpcTempCU, 0,          true );
#endif
  m_pcEntropyCoder->encodePredMode( rpcTempCU, 0,          true );
#if PLANAR_INTRA
  m_pcEntropyCoder->encodePlanarInfo( rpcTempCU, 0,        true );
#endif
  m_pcEntropyCoder->encodePartSize( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodePredInfo( rpcTempCU, 0,          true );

  // Encode Coefficients
#if QC_MDDT//ADAPTIVE_SCAN
  g_bUpdateStats = false;
  m_pcEntropyCoder->encodeCoeff( rpcTempCU, 0, uiDepth, rpcTempCU->getWidth (0), rpcTempCU->getHeight(0) );
#else
  m_pcEntropyCoder->encodeCoeff( rpcTempCU, 0, uiDepth, rpcTempCU->getWidth (0), rpcTempCU->getHeight(0) );

  // ROT index
  if ( rpcTempCU->getSlice()->getSPS()->getUseROT() )
  {
#if DISABLE_ROT_LUMA_4x4_8x8
    if (rpcTempCU->getWidth (0) > 8)
#endif
    m_pcEntropyCoder->encodeROTindex( rpcTempCU, 0, uiDepth );
  }
#endif

  // CIP index
#if HHI_ALLOW_CIP_SWITCH
  if ( rpcTempCU->isIntra( 0 ) && rpcTempCU->getSlice()->getSPS()->getUseCIP() )
#else
  if ( rpcTempCU->isIntra( 0 ) )
#endif
  {
    m_pcEntropyCoder->encodeCIPflag( rpcTempCU, 0, uiDepth );
  }

  if( m_bUseSBACRD ) m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckBestMode( rpcBestCU, rpcTempCU );
}

// check whether current try is the best
Void TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  if( rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost() )
  {
    TComYuv* pcYuv;
    UChar uhDepth = rpcBestCU->getDepth(0);

    // Change Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;

    // Change Prediction data
    pcYuv = m_ppcPredYuvBest[uhDepth];
    m_ppcPredYuvBest[uhDepth] = m_ppcPredYuvTemp[uhDepth];
    m_ppcPredYuvTemp[uhDepth] = pcYuv;

#if HHI_RQT
#else
    // Change Residual data
    pcYuv = m_ppcResiYuvBest[uhDepth];
    m_ppcResiYuvBest[uhDepth] = m_ppcResiYuvTemp[uhDepth];
    m_ppcResiYuvTemp[uhDepth] = pcYuv;
#endif

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

  for (iMVP0 = (cAMVPInfo0.iN > 0? 0:-1); iMVP0 < cAMVPInfo0.iN; iMVP0++)
  for (iMVP1 = (cAMVPInfo1.iN > 0? 0:-1); iMVP1 < cAMVPInfo1.iN; iMVP1++)
  for (Int iSkipRes = 0; iSkipRes <= 1; iSkipRes++)
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
    xCheckRDCostSkip ( rpcBestCU, rpcTempCU, (iSkipRes? true:false) );      rpcTempCU->initEstData();
  }

  if (rpcTempCU->getSlice()->isInterB())
  {
    // List0
    for (iMVP0 = (cAMVPInfo0.iN > 0? 0:-1); iMVP0 < cAMVPInfo0.iN; iMVP0++)
    {
      rpcTempCU->setPredModeSubParts( MODE_SKIP,    0,  uhDepth );
      rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,   0,  uhDepth );
      rpcTempCU->setInterDirSubParts( 1, 0, 0,          uhDepth );
      rpcTempCU->setMVPIdxSubParts  ( iMVP0,          REF_PIC_LIST_0, 0, 0, uhDepth );
      rpcTempCU->setMVPNumSubParts  ( cAMVPInfo0.iN,  REF_PIC_LIST_0, 0, 0, uhDepth );

      // copy AMVP information to CU
      xCopyAMVPInfo(&cAMVPInfo0, rpcTempCU->getCUMvField(REF_PIC_LIST_0)->getAMVPInfo());

      // compute RD cost
      xCheckRDCostSkip ( rpcBestCU, rpcTempCU, false ); rpcTempCU->initEstData();
    }
    // List1
    for (iMVP1 = (cAMVPInfo1.iN > 0? 0:-1); iMVP1 < cAMVPInfo1.iN; iMVP1++)
    {
      rpcTempCU->setPredModeSubParts( MODE_SKIP,    0,  uhDepth );
      rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N,   0,  uhDepth );
      rpcTempCU->setInterDirSubParts( 2, 0, 0,          uhDepth );
      rpcTempCU->setMVPIdxSubParts  ( iMVP1,          REF_PIC_LIST_1, 0, 0, uhDepth );
      rpcTempCU->setMVPNumSubParts  ( cAMVPInfo1.iN,  REF_PIC_LIST_1, 0, 0, uhDepth );

      // copy AMVP information to CU
      xCopyAMVPInfo(&cAMVPInfo1, rpcTempCU->getCUMvField(REF_PIC_LIST_1)->getAMVPInfo());

      // compute RD cost
      xCheckRDCostSkip ( rpcBestCU, rpcTempCU, false ); rpcTempCU->initEstData();
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

Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsZorderIdx, UInt uiDepth)
{
  m_ppcRecoYuvBest[uiDepth]->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsZorderIdx );
}

Void TEncCu::xCopyYuv2Tmp( UInt uiPartUnitIdx, UInt uiNextDepth )
{
  UInt uiCurrDepth = uiNextDepth - 1;
  m_ppcRecoYuvBest[uiNextDepth]->copyToPartYuv( m_ppcRecoYuvTemp[uiCurrDepth], uiPartUnitIdx );
}

