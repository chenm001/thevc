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

/** \file			TEncSlice.cpp
    \brief		slice encoder class
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

	for (Int i=0; i<GRF_MAX_NUM_EFF; i++)
  for (Int j=0; j<2; j++)
  {
    m_apcVirtPic[j][i] = NULL;
  }

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

	// free virtual reference pictures if they were allocated
	for (Int i=0; i<GRF_MAX_NUM_EFF; i++)
	for (Int j=0; j<2; j++)
	{
		if(m_apcVirtPic[j][i])
		{
			m_apcVirtPic[j][i]->destroy();
			delete m_apcVirtPic[j][i];
			m_apcVirtPic[j][i]=NULL;
		}
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
  m_pcPredSearch			= pcEncTop->getPredSearch();

  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcCavlcCoder      = pcEncTop->getCavlcCoder();
  m_pcSbacCoder				= pcEncTop->getSbacCoder();
  m_pcTrQuant					= pcEncTop->getTrQuant();

	m_pcBitCounter      = pcEncTop->getBitCounter();
  m_pcRdCost          = pcEncTop->getRdCost();
	m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();

	// create lambda and QP arrays
  m_pdRdPicLambda			= (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_pdRdPicQp					= (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_piRdPicQp					= (Int*   )xMalloc( Int,    m_pcCfg->getDeltaQpRD() * 2 + 1 );

  // allocate additional reference frame here
  if ( m_pcCfg->getGeneratedReferenceMode() != NULL )
  {
    UInt uiNumEffFrames = 0;
    if (pcEncTop->getSPS()->getUseWPG() || pcEncTop->getSPS()->getUseWPO() || pcEncTop->getSPS()->getUseWPR()) uiNumEffFrames++;
    if (pcEncTop->getSPS()->getUseAME() || pcEncTop->getSPS()->getUseIME() || pcEncTop->getSPS()->getUsePME()) uiNumEffFrames++;

    Int iWidth		= m_apcPicYuvPred->getWidth ();
    Int iHeight		= m_apcPicYuvPred->getHeight();

    for (Int i=0; i<uiNumEffFrames; i++)
    for (Int j=0; j<2; j++)
    {
      if(m_apcVirtPic[j][i] == NULL)
      {
        m_apcVirtPic[j][i] = new TComPic;
        m_apcVirtPic[j][i]->create( iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, true );
      }
    }
  }
}

/**
   - non-referenced frame marking
	 - QP computation based on temporal structure
	 - lambda computation based on QP
	 .
	 \param	pcPic					picture class
	 \param	iPOCLast			POC of last picture
	 \param	uiPOCCurr			current POC
	 \param	iNumPicRcvd		number of received pictures
	 \param	iTimeOffset		POC offset for hierarchical structure
	 \param	iDepth				temporal layer depth
	 \param	dScaleFactor	scaling factor according to depth
	 \param rpcSlice			slice header class
 */
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, Double dScaleFactor, TComSlice*& rpcSlice )
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
    if ( m_pcCfg->getUseJMQP() )
    {
      if ( ( iPOCLast != 0 ) && ( ( uiPOCCurr % m_pcCfg->getIntraPeriod() ) != 0 ) && ( m_pcGOPEncoder->getGOPSize() != 0 ) )	// P or B-slice
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
    else // JSVM QP ???
    {
      dQP = m_pcCfg->getQP() - 6.0 * log10( dScaleFactor ) / log10( 2.0 );
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
    dQP = dOrigQP + ((iDQpIdx+1)>>1)*(iDQpIdx%2 ? -1 : 1);

    if ( m_pcCfg->getUseJMLAMBDA() )
    {
      Int    NumberBFrames = ( m_pcCfg->getRateGOPSize() - 1 );
      Int		 SHIFT_QP = 12;
      Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)NumberBFrames );
      Int		 bitdepth_luma_qp_scale = 0;
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
            Int		 iMaxDepth = 0;
            Int		 iCnt = 1;
            Int		 hierarchy_layer;

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
    }
    else
    {
      dLambda = 0.85 * pow( 2.0, Min( 52.0, dQP ) / 3.0 - 4.0 );
    }

    iQP = Max( MIN_QP, Min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

    m_pdRdPicLambda[iDQpIdx] = dLambda;
    m_pdRdPicQp    [iDQpIdx] = dQP;
    m_piRdPicQp    [iDQpIdx] = iQP;
  }

	// obtain dQP = 0 case
  dLambda = m_pdRdPicLambda[0];
  dQP			= m_pdRdPicQp		 [0];
  iQP			= m_piRdPicQp		 [0];

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
  rpcSlice->setNumRefIdx				( REF_PIC_LIST_0, eSliceType == P_SLICE ? m_pcCfg->getNumOfReference() : (eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L0()) : 0 ) );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_1, eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L1()) : 0 );

  rpcSlice->setSymbolMode       ( m_pcCfg->getSymbolMode());
  rpcSlice->setLoopFilterDisable( m_pcCfg->getLoopFilterDisable() );

  rpcSlice->setDepth            ( iDepth );

  // reference picture usage indicator for next frames
  rpcSlice->setDRBFlag          ( true );
  rpcSlice->setERBIndex         ( ERB_NONE );

  // srlee: generalized B info. (for non-reference B)
  if ( m_pcCfg->getHierarchicalCoding() == false && iDepth != 0 )
  {
    rpcSlice->setDRBFlag        ( false );
    rpcSlice->setERBIndex       ( ERB_NONE );
  }

	// set high accuracy motion (HAM) flag
  if( ( rpcSlice->getSliceType() == P_SLICE && m_pcCfg->getUseHAP() ) || ( rpcSlice->getSliceType() == B_SLICE && m_pcCfg->getUseHAB() ) )
  {
    rpcSlice->setUseHAM( true );
    rpcSlice->setUseHME( m_pcCfg->getUseHME() );
  }
  else
  {
    rpcSlice->setUseHAM( false );
    rpcSlice->setUseHME( false );
  }

  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );

	pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );

	// initialize RD variables by QP
  m_pcPredSearch->initRateDistortionModel( dQP, AMVP_MAX_TEMP_SR );
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
	 \param rpcPic		picture class
 */
Void TEncSlice::precompressSliceCU( TComPic*& rpcPic )
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
  Int		 SHIFT_QP = 12;

	// set frame lambda
  if (m_pcCfg->getGOPSize() > 1)
	{
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0]  - SHIFT_QP) / 3.0) * (pcSlice->isInterB()? 2 : 1);
	}
  else
	{
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0] - SHIFT_QP) / 3.0);
	}
  m_pcRdCost			->setFrameLambda(dFrameLambda);

	// for each QP candidate
  for ( UInt uiQpIdx = 0; uiQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; uiQpIdx++ )
  {
    pcSlice       ->setSliceQp             ( m_piRdPicQp    [uiQpIdx] );
    m_pcRdCost    ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    m_pcTrQuant   ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
    m_pcPredSearch->initRateDistortionModel( m_pdRdPicQp    [uiQpIdx], AMVP_MAX_TEMP_SR);

		// try compress
    compressSliceCU   ( rpcPic );

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
  m_pcPredSearch->initRateDistortionModel( m_pdRdPicQp    [uiQpIdxBest], AMVP_MAX_TEMP_SR);
}

/** \param rpcPic		picture class
 */
Void TEncSlice::compressSliceCU( TComPic*& rpcPic )
{
  UInt  uiCUAddr;

	// initialize cost values
	m_uiPicTotalBits	= 0;
  m_dPicRdCost			= 0;
  m_uiPicDist				= 0;

  // set entropy coder
  if( m_pcCfg->getUseSBACRD() )
  {
    m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, rpcPic->getSlice() );
    m_pcEntropyCoder->resetEntropy      ();
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->load(m_pcSbacCoder);
  }
  else
  {
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, rpcPic->getSlice() );
    m_pcEntropyCoder->resetEntropy      ();
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  }

	// initialize ALF parameters
  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0); //unnecessary

  m_pcTrQuant->precalculateUnaryExpGolombLevel();

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
    }

    m_uiPicTotalBits += pcCU->getTotalBits();
    m_dPicRdCost     += pcCU->getTotalCost();
    m_uiPicDist      += pcCU->getTotalDistortion();
  }
}

/** \param	rpcPic				picture class
		\retval	rpcBitstream	bitstream class
 */
Void TEncSlice::encodeSliceCU   ( TComPic*& rpcPic, TComBitstream*& rpcBitstream )
{
  UInt			 uiCUAddr;
  TComSlice* pcSlice = rpcPic->getSlice();

	// choose entropy coder
  Int iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
	{
    m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
	}
  else
	{
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
	}

	// set bitstream
  m_pcEntropyCoder->setBitstream( rpcBitstream );

	// write EXC parameters before CU data
  if ( pcSlice->getSPS()->getUseExC() )
  {
    m_pcEntropyCoder->encodeExtremeValue	( rpcPic );
		m_pcEntropyCoder->encodeBandCorrValue	( rpcPic );
  }

	// for every CU
  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    m_pcCuEncoder->setQpLast( rpcPic->getSlice()->getSliceQp() );

		TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
		m_pcCuEncoder->encodeCU( pcCU );
  }
}

Void TEncSlice::generateRefPicNew ( TComSlice* rpcSlice )
{
  if (rpcSlice->getSliceType() == I_SLICE)
	{
    return;
  }

	// initialize number of additional reference count
  rpcSlice->clearAddRefCnt(REF_PIC_LIST_0);
  rpcSlice->clearAddRefCnt(REF_PIC_LIST_1);

	// check generation modes
  if ( rpcSlice->getSPS()->getUseWPG() )
  {
    rpcSlice->setWPmode(REF_PIC_LIST_0, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_WP_SO);		// scale + offset

    rpcSlice->setWPmode(REF_PIC_LIST_1, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_WP_SO);		// scale + offset
  }

  if ( rpcSlice->getSPS()->getUseWPO() )
  {
    rpcSlice->setWPmode(REF_PIC_LIST_0, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_WP_O);		// scale

    rpcSlice->setWPmode(REF_PIC_LIST_1, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_WP_O);		// scale
  }

  if ( rpcSlice->getSPS()->getUseWPR() )
  {
    rpcSlice->setRFmode(1);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_WP_P1);		// +1
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_WP_M1);		// -1

    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_WP_P1);		// +1
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_WP_M1);		// -1
  }

	// isotropic
  if ( rpcSlice->getSPS()->getUseIME() )
  {
    rpcSlice->setGMmode(REF_PIC_LIST_0, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_IM);

    rpcSlice->setGMmode(REF_PIC_LIST_1, 1);
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_IM);
  }
	// affine
  else if ( rpcSlice->getSPS()->getUseAME() )
  {
    rpcSlice->setGMmode(REF_PIC_LIST_0, 2);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_AM);

    rpcSlice->setGMmode(REF_PIC_LIST_1, 2);
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_AM);
  }
	// perspective
  else if ( rpcSlice->getSPS()->getUsePME() )
  {
    rpcSlice->setGMmode(REF_PIC_LIST_0, 3);
    rpcSlice->addEffectMode(REF_PIC_LIST_0, EFF_PM);

    rpcSlice->setGMmode(REF_PIC_LIST_1, 3);
    rpcSlice->addEffectMode(REF_PIC_LIST_1, EFF_PM);
  }

	// if no virtual reference is needed, return
  if (rpcSlice->getAddRefCnt(REF_PIC_LIST_0) == 0 && rpcSlice->getAddRefCnt(REF_PIC_LIST_1) == 0) return;

	// set virtual reference buffer
  rpcSlice->setVirtRefBuffer( m_apcVirtPic );

	// for each direction
  for ( Int iDir = 0; iDir < (rpcSlice->getSliceType() == P_SLICE ? 1 :2); iDir++ )
  {
    RefPicList eRefPicList = (RefPicList) iDir;

		// for each added effect
    for ( Int i = 0; i < rpcSlice->getAddRefCnt(eRefPicList); i++ )
    {
      EFF_MODE eEffMode = rpcSlice->getEffectMode(eRefPicList, i);
      if (eEffMode >= EFF_WP_SO && eEffMode <= EFF_WP_M1)
      {
        Bool bGen = xEstimateWPSlice(rpcSlice, eRefPicList, eEffMode);
        if (bGen) rpcSlice->generateWPSlice(eRefPicList, eEffMode, i);
      }
      if (eEffMode >= EFF_IM && eEffMode <= EFF_PM)
      {
        Bool bGen = xEstimateGMSlice(rpcSlice, eRefPicList);
        if (bGen) rpcSlice->generateGMSlice(eRefPicList, i, &m_cGlobalMotion);
      }
    }

    // remove unnecessary virtual references here if net effect is same
    if ( rpcSlice->getWPmode(eRefPicList) )
    {
      EFF_MODE eEffMode = (rpcSlice->getSPS()->getUseWPG()? EFF_WP_SO : EFF_WP_O);
      if (rpcSlice->isEqualWPAllParam(eRefPicList, eEffMode, EFF_NONE))
      {
        rpcSlice->removeEffectMode(eRefPicList, eEffMode);
        rpcSlice->setWPmode(eRefPicList, 0);
      }
    }
    if ( rpcSlice->getWPmode(eRefPicList) && rpcSlice->getRFmode() )
    {
      EFF_MODE eEffMode = (rpcSlice->getSPS()->getUseWPG()? EFF_WP_SO : EFF_WP_O);
      if (rpcSlice->isEqualWPAllParam(eRefPicList, eEffMode, EFF_WP_P1))
      {
        rpcSlice->removeEffectMode(eRefPicList, eEffMode);
        rpcSlice->setWPmode(eRefPicList, 0);
      }
      else if (rpcSlice->isEqualWPAllParam(eRefPicList, eEffMode, EFF_WP_M1))
      {
        rpcSlice->removeEffectMode(eRefPicList, eEffMode);
        rpcSlice->setWPmode(eRefPicList, 0);
      }
    }
    if (rpcSlice->getGMmode(eRefPicList))
    {
      if (rpcSlice->isTrajAllZero(eRefPicList))
      {
        //FIXME: initialization or some other action is necessary here
        //coz assert hushes in release build //kolya
        EFF_MODE eEffMode;
        if (rpcSlice->getGMmode(eRefPicList) == 1)
        {
          eEffMode = EFF_IM;
        }
        else if (rpcSlice->getGMmode(eRefPicList) == 2)
        {
          eEffMode = EFF_AM;
        }
        else if (rpcSlice->getGMmode(eRefPicList) == 3)
        {
          eEffMode = EFF_PM;
        }
        else
        {
          assert(0);
        }
        rpcSlice->removeEffectMode(eRefPicList, eEffMode);
        rpcSlice->setGMmode(eRefPicList, 0);
      }
    }
  }

	// add virtual reference to reference list
  rpcSlice->linkVirtRefPic();
}

Bool TEncSlice::xEstimateGMSlice( TComSlice* rpcSlice, RefPicList eRefPicList )
{
  UInt ui;
  UInt uiNumOfSpritePoints	= rpcSlice->getNumOfSpritePoints();
  Int iHorSpatRef						= rpcSlice->getHorSpatRef();
  Int iVerSpatRef						= rpcSlice->getVerSpatRef();

  SpriteMotion cAffineMotion = {1, 0, 0, 0, 1, 0, 0, 0};
  m_cGlobalMotion.globalMotionEstimation( rpcSlice->getRefPic(eRefPicList, 0)->getPicYuvRec(),
																					rpcSlice->getPic()->getPicYuvOrg(),
																					cAffineMotion, uiNumOfSpritePoints, iHorSpatRef, iVerSpatRef);

	TrajPoint* pcRefPointCoord  = new TrajPoint[uiNumOfSpritePoints];
  TrajPoint* pcTrajPointCoord = new TrajPoint[uiNumOfSpritePoints];

  for (ui = 0; ui < uiNumOfSpritePoints; ui++)
  {
    pcRefPointCoord[ui].x = iHorSpatRef + (ui%2)*rpcSlice->getSPS()->getWidth();
    pcRefPointCoord[ui].y = iVerSpatRef + (ui>>1)*rpcSlice->getSPS()->getHeight();
    rpcSlice->setRefPoint(eRefPicList, ui, pcRefPointCoord[ui]);
  }

  // produce trajectories and quant the motion param
  // compute trajectories from global motion
  UInt uiAbsSum = m_cGlobalMotion.makeTraj(cAffineMotion, uiNumOfSpritePoints, pcRefPointCoord, pcTrajPointCoord);

  for (ui = 0; ui < uiNumOfSpritePoints; ui++)
  {
    rpcSlice->setTrajPoint(eRefPicList, ui, pcTrajPointCoord[ui]);
  }

  if (uiAbsSum == 0)
  {
    delete[] pcRefPointCoord;
    delete[] pcTrajPointCoord;
    return false;
  }

  // predict the motion param
  TrajPoint* pcDiffTrajPointCoord = new TrajPoint[uiNumOfSpritePoints];
  m_cGlobalMotion.makeDiffTraj(uiNumOfSpritePoints, pcTrajPointCoord, pcDiffTrajPointCoord);

  for (ui = 0; ui < uiNumOfSpritePoints; ui++)
  {
    rpcSlice->setDiffTrajPoint(eRefPicList, ui, pcDiffTrajPointCoord[ui]);
  }

  delete[] pcRefPointCoord;
  delete[] pcTrajPointCoord;
  delete[] pcDiffTrajPointCoord;

  return true;
}

Bool TEncSlice::xEstimateWPSlice( TComSlice* rpcSlice, RefPicList eRefPicList, EFF_MODE eEffMode )
{
  Int iDefaultWeight[3];

  Int numDir = ((rpcSlice->getSliceType()) == P_SLICE) ? 1 : 2;
  Int iWeight[3];
  Int iOffset[3] = {0, 0, 0};

  Double dDCOrg = 0.;
  Double dDCRef = 0.;
  Double dNormOrg =0.;
  Double dNormRef =0.;
  Double dNumer=0.;
  Double dDenom =0.;

  TComPicYuv* pcPicYuvOrg = rpcSlice->getPic()->getPicYuvOrg();
  TComPicYuv* pcPicYuvRef;

  Int iWidth = pcPicYuvOrg->getWidth();
  Int iHeight = pcPicYuvOrg->getHeight();
  Int iStride = pcPicYuvOrg->getStride();

  rpcSlice->initWPAllParam(eRefPicList, eEffMode);

  iDefaultWeight[0] = rpcSlice->getWPWeight(eRefPicList, eEffMode, 0);
  iDefaultWeight[1] = rpcSlice->getWPWeight(eRefPicList, eEffMode, 1);
  iDefaultWeight[2] = rpcSlice->getWPWeight(eRefPicList, eEffMode, 2);

  if ( eEffMode==EFF_WP_P1 || eEffMode==EFF_WP_M1 ) return true;

  dDCOrg	 = xComputeImgSum(pcPicYuvOrg->getLumaAddr(), iWidth, iHeight, iStride);
  dNormOrg = dDCOrg / (Double)(iWidth * iHeight);
  dNumer	 = xComputeNormMean(pcPicYuvOrg->getLumaAddr(), dNormOrg, iWidth, iHeight, iStride);

  Int iShift = g_uiBitDepth+g_uiBitIncrement-8;

  // Luma
  pcPicYuvRef = rpcSlice->getRefPic(eRefPicList, 0)->getPicYuvRec();
  iWidth			= pcPicYuvRef->getWidth	();
  iHeight			= pcPicYuvRef->getHeight();
  iStride			= pcPicYuvRef->getStride();

  dDCRef		= xComputeImgSum(pcPicYuvRef->getLumaAddr(), iWidth, iHeight, iStride);
  dNormRef	= dDCRef / (Double)(iWidth*iHeight);
  dDenom		= xComputeNormMean(pcPicYuvRef->getLumaAddr(), dNormRef, iWidth, iHeight, iStride);

	// scale + offset mode
  if (eEffMode==EFF_WP_SO)
	{
    if (dDenom != 0.)
		{
      iWeight[0] = (Int) ((Double)(iDefaultWeight[0]) * dNumer / dDenom + 0.5);
    }
    else
		{
      iWeight[0] = iDefaultWeight[0];
    }
    iWeight[0] = Clip3(-128, 127, iWeight[0]);

    iOffset[0] = (Int) (dNormOrg - ((Double) iWeight[0] * dNormRef / (Double) iDefaultWeight[0]) + 0.5);

    if (iShift)
    {
      iOffset[0] = (iOffset[0]+(1<<(iShift-1)))>>iShift;
      iOffset[0] = Clip3( -128, 127, iOffset[0]);
      iOffset[0] = iOffset[0]<<iShift;
    }
    else
    {
      iOffset[0] = Clip3( -128, 127, iOffset[0]);
    }
  }
	// offset mode
  else if (eEffMode==EFF_WP_O)
	{
    iOffset[0] = (Int) ((dDCOrg - dDCRef)/(iWidth*iHeight)+0.5);

    if (iShift)
    {
      iOffset[0] = (iOffset[0]+(1<<(iShift-1)))>>iShift;
      iOffset[0] = Clip3( -128, 127, iOffset[0]);
      iOffset[0] = iOffset[0]<<iShift;
    }
    else
    {
      iOffset[0] = Clip3( -128, 127, iOffset[0]);
    }
    iWeight[0] = iDefaultWeight[0];
  }

  rpcSlice->setWPWeight(eRefPicList, eEffMode, 0, iWeight[0]);
  rpcSlice->setWPOffset(eRefPicList, eEffMode, 0, iOffset[0]);

#if GRF_WP_CHROMA
  iWidth	= iWidth>>1;
  iHeight = iHeight>>1;
  iStride = pcPicYuvOrg->getCStride();

  Double dDCOrgU = xComputeImgSum(pcPicYuvOrg->getCbAddr(), iWidth, iHeight, iStride);
  Double dDCOrgV = xComputeImgSum(pcPicYuvOrg->getCrAddr(), iWidth, iHeight, iStride);
  Double dDCRefU = xComputeImgSum(pcPicYuvRef->getCbAddr(), iWidth, iHeight, iStride);
  Double dDCRefV = xComputeImgSum(pcPicYuvRef->getCrAddr(), iWidth, iHeight, iStride);

	// scale + offset mode
  if ( eEffMode==EFF_WP_SO )
	{
    if (dDCRefU != 0.)
		{
      iWeight[1] = (Int) (iDefaultWeight[1] * dDCOrgU / dDCRefU + 0.5);
    }
    else
		{
      iWeight[1] = iDefaultWeight[1];
    }
    iWeight[1] = Clip3(-128, 127, iWeight[1]);

    if (dDCRefV != 0.)
		{
      iWeight[2] = (Int) (iDefaultWeight[2] * dDCOrgV / dDCRefV + 0.5);
    }
    else
		{
      iWeight[2] = iDefaultWeight[2];
    }
    iWeight[2] = Clip3(-128, 127, iWeight[2]);
  }
	// offset mode
  else if (eEffMode==EFF_WP_O)
	{
    iOffset[1] = (Int) ((dDCOrgU - dDCRefU)/(iWidth*iHeight)+0.5);

		if (iShift)
    {
      iOffset[1] = (iOffset[1]+(1<<(iShift-1)))>>iShift;
      iOffset[1] = Clip3( -128, 127, iOffset[1]);
      iOffset[1] = iOffset[1]<<iShift;
    }
    else
    {
      iOffset[1] = Clip3( -128, 127, iOffset[1]);
    }
    iWeight[1] = iDefaultWeight[1];

    iOffset[2] = (Int) ((dDCOrgV - dDCRefV)/(iWidth*iHeight)+0.5);
    if (iShift)
    {
      iOffset[2] = (iOffset[2]+(1<<(iShift-1)))>>iShift;
      iOffset[2] = Clip3( -128, 127, iOffset[2]);
      iOffset[2] = iOffset[2]<<iShift;
    }
    else
    {
      iOffset[2] = Clip3( -128, 127, iOffset[2]);
    }
    iWeight[2] = iDefaultWeight[2];
  }

  rpcSlice->setWPWeight(eRefPicList, eEffMode, 1, iWeight[1]);
  rpcSlice->setWPOffset(eRefPicList, eEffMode, 1, iOffset[1]);
  rpcSlice->setWPWeight(eRefPicList, eEffMode, 2, iWeight[2]);
  rpcSlice->setWPOffset(eRefPicList, eEffMode, 2, iOffset[2]);
#endif

#if GRF_WP_CHROMA
  if (iWeight[0] == iDefaultWeight[0] && iOffset[0] == 0 &&
	    iWeight[1] == iDefaultWeight[1] && iOffset[1] == 0 &&
	    iWeight[2] == iDefaultWeight[2] && iOffset[2] == 0 )	 //-> same to original reference
#else
  if (iWeight[0] == iDefaultWeight[0] && iOffset[0] == 0)		 //-> same to original reference
#endif
  {
    return false;
  }

  return true;
}

Double TEncSlice::xComputeImgSum( Pel* img, Int width, Int height, Int stride )
{
  Int x, y;
  Double sum=0.;

  for (y=0; y<height; y++)
	{
    for (x=0; x<width; x++)
		{
      sum += (Double)(img[x]);
    }
    img += stride;
  }
  return sum;
}

Double TEncSlice::xComputeNormMean( Pel* img, Double meanValue, Int width, Int height, Int stride )
{
  Int x, y;
  Double sum=0.;

  for (y=0; y<height; y++)
	{
    for (x=0; x<width; x++)
		{
      sum += fabs((Double)(img[x]) - meanValue);
    }
    img += stride;
  }
  return sum;
}
