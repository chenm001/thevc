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

/** \file			TEncGOP.cpp
    \brief		GOP encoder class
*/

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"

#include <time.h>

//kolya
inline int mSign(int x)
 {
  // http://bits.stephan-brumme.com/sign.html
  // only for 32-bit integer with C-style shifts
  // may be unportable!
  // minusOne will be 0xFFFFFFFF (=> -1) for negative numbers, else 0x00000000
   int minusOne = x >> 31;

   // plusOne will be 0x00000001 for positive numbers, else 0x00000000
   unsigned int negateX = (unsigned int) -x;
   int plusOne = (int)(negateX >> 31);

   // at least minusOne or plusOne is zero, the other one gives our desired result
   int result = minusOne | plusOne;

   return result;
 }

TEncGOP::TEncGOP()
{
  m_iHrchDepth          = 0;
  m_iGopSize            = 0;
  m_iNumPicCoded        = 0; //Niko
  m_bFirst              = true;
  for (Int i = 0; i < 64; i++)
  {
    m_dScalingFactor[i] = 1.0;
  }

  m_pcCfg               = NULL;
  m_pcSliceEncoder      = NULL;
  m_pcListPic           = NULL;

  m_pcEntropyCoder      = NULL;
  m_pcCavlcCoder        = NULL;
  m_pcSbacCoder        = NULL;

	m_bSeqFirst						= true;

  return;
}

TEncGOP::~TEncGOP()
{
}

Void  TEncGOP::create()
{
}

Void  TEncGOP::destroy()
{
}

Void TEncGOP::init ( TEncTop* pcTEncTop )
{
  m_pcEncTop		 = pcTEncTop;
  m_pcCfg                = pcTEncTop;
  m_pcSliceEncoder       = pcTEncTop->getSliceEncoder();
  m_pcListPic            = pcTEncTop->getListPic();

  m_pcEntropyCoder       = pcTEncTop->getEntropyCoder();
  m_pcCavlcCoder         = pcTEncTop->getCavlcCoder();
  m_pcSbacCoder         = pcTEncTop->getSbacCoder();
  m_pcLoopFilter         = pcTEncTop->getLoopFilter();
  m_pcBitCounter         = pcTEncTop->getBitCounter();

  // Adaptive Loop filter
  m_pcAdaptiveLoopFilter = pcTEncTop->getAdaptiveLoopFilter();
  //--Adaptive Loop filter
}

Void TEncGOP::xInitGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut )
{
  assert( iNumPicRcvd > 0 );
  Int i;

  //  Set hierarchical B info.
  m_iGopSize    = m_pcCfg->getGOPSize();
  for( i=1 ; ; i++)
  {
    m_iHrchDepth = i;
    if((m_iGopSize >> i)==0)
    {
      break;
    }
  }

  //  Set scaling factor
  for ( i = 0; i < MAX_GOP + 1; i++ )  {    m_dScalingFactor[i] = 1.0;  }

  for ( Int iLevel = 0; iLevel < m_iHrchDepth - 1; iLevel++ )
  {
    xSetScalingFactor( iLevel, iNumPicRcvd );
  }

  //  Adjust. scaling factor for generalized B info.
  if ( m_pcCfg->getHierarchicalCoding() == false && m_iHrchDepth > 1 )
  {
    Double dScalingBase = m_dScalingFactor[ 1 << (m_iHrchDepth - 2) ];
    for ( i = 1; i < (1 << (m_iHrchDepth - 1)); i++ )
    {
      m_dScalingFactor[i] = dScalingBase;
    }
  }

  //  Exception for the first frame
  if ( iPOCLast == 0 )
  {
    m_iGopSize    = 1;
    m_iHrchDepth  = 1;
  }

  if (m_iGopSize == 0)
  {
    m_iHrchDepth = 1;
  }

  return;
}

#define SCALING_FACTOR_HACK 1

#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
#define FACTOR_53_HP  0.84779124789065851738306954082825  //sqrt(23.0/32.0)
#define FACTOR_53_LP  1.2247448713915890490986420373529   //sqrt( 3.0/ 2.0)

#define FACTOR_53_HP_BL 1
#define FACTOR_22_HP_BL 1

/*  Scaling factor for Qp control
 *  See w6716.zip, m11728.zip, JVTO045 and m11876
 */
Void TEncGOP::xSetScalingFactor( Int iLevel, Int iNumPicRcvd )
{
  Double  dScalingBase    = m_dScalingFactor[0];  //  m_pacControlData[0].getScalingFactor();
  Double  dScalingLowPass = 0.0;
  Int     iLowPassSize    = ( m_iGopSize >> iLevel );
  Int     iFrame;

  //--  kyohyuk.lee : mismatch compensation of encoding frame and GOP size
  if (m_pcCfg->getFrameToBeEncoded() <= m_iGopSize)
  {
    iLowPassSize = (m_pcCfg->getFrameToBeEncoded() - 1) >> iLevel;
  }
  else if (!m_bFirst && iNumPicRcvd /*m_pcCfg->getFrameToBeEncoded() % m_iGopSize*/ < m_iGopSize)
  {
    iLowPassSize = (iNumPicRcvd) >> iLevel;
  }

  //===== get low-pass scaling =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame += 2 )
  {
    Double  dScalLPCurr = 1.0;

    if( iFrame > 0 )
    {
      if( ( iFrame + 1 ) < iLowPassSize )
      {
        dScalLPCurr = ( 0.6 + 0.6 ) * ( FACTOR_53_LP - 1.0 ) / 2.0 +
                      ( 0.2 + 0.2 ) * ( FACTOR_22_LP - 1.0 ) / 2.0 + 1.0;
      }
      else
      {
        dScalLPCurr = ( 0.6 / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( 0.2       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }
    else
    {
      if( iLowPassSize )
      {
        dScalLPCurr = ( 0.6 / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( 0.2       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }

    dScalingLowPass += dScalLPCurr;
  }
  dScalingLowPass /= (Double)( 1 + ( iLowPassSize >> 1 ) );


  //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
  Double dFactor53;
  Double dFactor22;
#if SCALING_FACTOR_HACK
  // heiko.schwarz@hhi.fhg.de: This is a bad hack for ensuring that the
  // closed-loop config files work and use identical scaling factor as
  // the MCTF version. The non-update scaling factors don't work and shall
  // be completely removed in future versions.
  if ( m_pcCfg->getSourceWidth() <= 176 ) // smaller or equal to QCIF
#else
  if( false )
#endif
  {
	  dFactor53 = FACTOR_53_HP_BL;
	  dFactor22 = FACTOR_22_HP_BL;
  }
  else
  {
	  dFactor53 = FACTOR_53_HP;
	  dFactor22 = FACTOR_22_HP;
  }
  //}}Scaling factor Base Layer

  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      //===== high-pass pictures =====
      //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
      dScal *= ( 0.6       ) * ( dFactor53 - 1.0 ) +
               ( 0.2 + 0.2 ) * ( dFactor22 - 1.0 ) + 1.0;
      //}}Scaling factor Base Layer
    }
    else
    {
      //===== low-pass pictures =====
      dScal *= dScalingLowPass;
    }
    m_dScalingFactor[ iFrame << iLevel ] = dScal;
  }

}

Void TEncGOP::xGetBuffer( TComList<TComPic*>&       rcListPic,
                          TComList<TComPicYuv*>&    rcListPicYuvRecOut,
                          TComList<TComBitstream*>& rcListBitstreamOut,
                          Int                       iNumPicRcvd,
                          Int                       iTimeOffset,
                          TComPic*&                 rpcPic,
                          TComPicYuv*&              rpcPicYuvRecOut,
                          TComBitstream*&           rpcBitstreamOut,
                          UInt                      uiPOCCurr )
{
  Int i;
  //  Rec. output
  TComList<TComPicYuv*>::iterator     iterPicYuvRec = rcListPicYuvRecOut.end();
  for ( i = 0; i < iNumPicRcvd - iTimeOffset + 1; i++ )
  {
    iterPicYuvRec--;
  }

  rpcPicYuvRecOut = *(iterPicYuvRec);

  //  Bitstream output
  TComList<TComBitstream*>::iterator  iterBitstream = rcListBitstreamOut.begin();
  for ( i = 0; i < m_iNumPicCoded; i++ )
  {
    iterBitstream++;
  }
  rpcBitstreamOut = *(iterBitstream);

  //  Current pic.
  TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
  while (iterPic != rcListPic.end())
  {
    rpcPic = *(iterPic);
    if (rpcPic->getPOC() == (Int)uiPOCCurr)
    {
      break;
    }
    iterPic++;
  }

  assert (rpcPic->getPOC() == (Int)uiPOCCurr);

  return;
}

Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, TComList<TComBitstream*> rcListBitstreamOut )
{
  TComPic*        pcPic;
  TComPicYuv*     pcPicYuvRecOut;
  TComBitstream*  pcBitstreamOut;
	TComPicYuv			cPicOrg;
	TComPic*				pcOrgRefList[2][MAX_REF_PIC_NUM];

  xInitGOP( iPOCLast, iNumPicRcvd, rcListPic, rcListPicYuvRecOut );

  m_iNumPicCoded = 0;
  for ( Int iDepth = 0; iDepth < m_iHrchDepth; iDepth++ )
  {
    Int iTimeOffset = ( 1 << (m_iHrchDepth - 1 - iDepth) );
    Int iStep       = iTimeOffset << 1;

    // generalized B info.
    if ( (m_pcCfg->getHierarchicalCoding() == false) && (iDepth != 0) )
    {
      iTimeOffset   = 1;
      iStep         = 1;
    }

    UInt uiColDir = 1;

    for ( ; iTimeOffset <= iNumPicRcvd; iTimeOffset += iStep )
    {
      //-- For time output for each slice
      long iBeforeTime = clock();

      // generalized B info.
      if ( (m_pcCfg->getHierarchicalCoding() == false) && (iDepth != 0) && (iTimeOffset == m_iGopSize) && (iPOCLast != 0) )
      {
        continue;
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// Initial to start encoding
      UInt  uiPOCCurr = iPOCLast - (iNumPicRcvd - iTimeOffset);

      xGetBuffer( rcListPic, rcListPicYuvRecOut, rcListBitstreamOut, iNumPicRcvd, iTimeOffset,  pcPic, pcPicYuvRecOut, pcBitstreamOut, uiPOCCurr );

			// save original picture
			cPicOrg.create( pcPic->getPicYuvOrg()->getWidth(), pcPic->getPicYuvOrg()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
			pcPic->getPicYuvOrg()->copyToPic( &cPicOrg );

			// CADR parameter derivation
			if ( m_bSeqFirst )
			{
				if ( g_iMinCADR < 0 )
				{
					// compute CADR range
					Int iMin, iMax;
					cPicOrg.getLumaMinMax( &iMin, &iMax );

#if CADR_DERIVE_BT709
					g_iMinCADR   = Min( iMin,  16 );
					g_iMaxCADR   = Max( iMax, 235 );
#else
					g_iMinCADR   = iMin;
					g_iMaxCADR   = iMax;
#endif
					g_iRangeCADR = ( g_iMaxCADR - g_iMinCADR + 1 );

					// initialize CADR table here
					initCADRQPTable( m_pcEncTop->getSPS()->getUseLCT() );
				}
			}

			// scaling of picture
      xScalePic( pcPic );

      //  Bitstream reset
      pcBitstreamOut->resetBits();
      pcBitstreamOut->rewindStreamPacket();

      //  Slice data initialization
      Double          dScaleFactor = (iPOCLast == 0 ? m_dScalingFactor[0] : m_dScalingFactor[iTimeOffset]);
      TComSlice*      pcSlice;
      m_pcSliceEncoder->initEncSlice ( pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iTimeOffset, iDepth, dScaleFactor, pcSlice );

			//  Set SPS
			pcSlice->setSPS( m_pcEncTop->getSPS() );

      //  Set reference list
      pcSlice->setRefPicList ( rcListPic );

      //  Slice info. refinement
      if ( (pcSlice->getSliceType() == B_SLICE) && (pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0) )
      {
        pcSlice->setSliceType ( P_SLICE );
        pcSlice->setDRBFlag   ( true ); //--> srlee : generalized B info.에서 짜투리 문제
      }

			// Generalized B
			if ( m_pcCfg->getUseGPB() )
			{
				if (pcSlice->getSliceType() == P_SLICE)
				{
					pcSlice->setSliceType( B_SLICE ); // Change slice type by force

					Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
					pcSlice->setNumRefIdx( REF_PIC_LIST_1, iNumRefIdx );

					for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
					{
						pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
					}
				}
			}

			// quality-based reference reordering (QBO)
			if ( !pcSlice->isIntra() && pcSlice->getSPS()->getUseQBO() )
			{
				Int iMinIdx = 0, iMinQP, iRefIdx, iCnt;
				TComPic* pRef;

				// save original reference list & generate new reference list
				for ( Int iList = 0; iList < 2; iList++ )
				{
					iMinQP = pcSlice->getSliceQp();

					Int iNumRefIdx = pcSlice->getNumRefIdx( (RefPicList)iList );
					for ( iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++ )
					{
						pRef = pcSlice->getRefPic( (RefPicList)iList, iRefIdx );
						pcOrgRefList[ (RefPicList)iList ][ iRefIdx ] = pRef;
					}
					for ( iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++ )
					{
						pRef = pcSlice->getRefPic( (RefPicList)iList, iRefIdx );
						if ( pRef->getSlice()->getSliceQp() <= iMinQP )
						{
							iMinIdx = iRefIdx;
							break;
						}
					}

					// set highest quality reference to zero index
					pcSlice->setRefPic( pcOrgRefList[ (RefPicList)iList ][ iMinIdx ], (RefPicList)iList, 0 );

					iCnt = 1;
					for ( iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++ )
					{
						if ( iRefIdx == iMinIdx ) continue;
						pcSlice->setRefPic( pcOrgRefList[ (RefPicList)iList ][ iRefIdx ], (RefPicList)iList, iCnt++ );
					}
				}
			}

      if (pcSlice->getSliceType() == B_SLICE)
      {
        pcSlice->setColDir(uiColDir);
      }

      uiColDir = 1-uiColDir;

      if ( pcSlice->getSPS()->getUseMVAC() && pcSlice->isInterB() )
        pcSlice->setUseMVAC(true);
      else
        pcSlice->setUseMVAC(false);

      // Weighted prediction ----------------------------------------
      m_pcSliceEncoder->generateRefPicNew(pcSlice);

      //-------------------------------------------------------------
      pcSlice->setRefPOCList();

      /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a slice
      //  Slice compression
      if (m_pcCfg->getUseASR())
			{
        m_pcSliceEncoder->setSearchRange(pcSlice);
			}

      m_pcSliceEncoder->precompressSliceCU( pcPic );
      m_pcSliceEncoder->compressSliceCU   ( pcPic );

			// quality-based reference reordering (QBO)
			if ( !pcSlice->isIntra() && pcSlice->getSPS()->getUseQBO() )
			{
				// restore original reference list
				for ( Int iList = 0; iList < 2; iList++ )
				{
					Int iNumRefIdx = pcSlice->getNumRefIdx( (RefPicList)iList );
					for ( Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++ )
					{
						pcSlice->setRefPic( pcOrgRefList[ (RefPicList)iList ][ iRefIdx ], (RefPicList)iList, iRefIdx );
					}
				}
			}

      //-- Loop filter
      m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterAlphaC0Offget(), m_pcCfg->getLoopFilterBetaOffget());
      m_pcLoopFilter->loopFilterPic( pcPic );

      /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
      // Set entropy coder
      m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
      m_pcEntropyCoder->setBitstream      ( pcBitstreamOut          );

			// write SPS
			if ( m_bSeqFirst )
			{
				m_pcEntropyCoder->encodeSPS( pcSlice->getSPS() );
				m_bSeqFirst = false;
			}

			// write SliceHeader
      m_pcEntropyCoder->encodeSliceHeader ( pcSlice                 );

			// wjhan@NOTE: is it needed?
			if ( pcSlice->getSymbolMode() )
			{
				m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPic->getSlice() );
				m_pcEntropyCoder->resetEntropy    ();
				pcBitstreamOut->writeAlignOne		  ();
			}

      /////////////////////////////////////////////////////////////////////////////////////////////////// Reconstructed image output
      TComPicYuv pcPicD;
      pcPicD.create( pcPic->getPicYuvOrg()->getWidth(), pcPic->getPicYuvOrg()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );

      if(pcSlice->getSPS()->getUseExC())
      {
        Int iExtremeType = 0;
        for (iExtremeType = 0; iExtremeType <5; iExtremeType ++) pcPic->setLocalExtremeValues(iExtremeType,0,0);

        //Moved here condition check //kol
        if ( pcPic->getSlice()->getSliceType()!= B_SLICE  || pcPic->getFrameHeightInCU() > EXC_NO_EC_BSLICE )
        xExtremeCorr( pcPic,  &pcPicD  );
      }

      // Adaptive Loop filter

      if ( pcSlice->getSPS()->getUseALF() )
      {
        ALFParam cAlfParam;
        m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);

        m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcPic->getSlice() );
        m_pcEntropyCoder->resetEntropy    ();
        m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
        m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder);
        UInt uiMaxAlfCtrlDepth;

        UInt64 uiDist, uiBits;
        m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, pcPic->getSlice()->getLambda(), uiDist, uiBits, uiMaxAlfCtrlDepth );
        m_pcAdaptiveLoopFilter->endALFEnc();

        // set entropy coder for writing
        m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcPic->getSlice() );
        m_pcEntropyCoder->resetEntropy    ();
        m_pcEntropyCoder->setBitstream    ( pcBitstreamOut );
        if (cAlfParam.cu_control_flag)
        {
          m_pcEntropyCoder->setAlfCtrl(true);
          m_pcEntropyCoder->setMaxAlfCtrlDepth(uiMaxAlfCtrlDepth);
          if (pcSlice->getSymbolMode() == 0)
          {
          m_pcCavlcCoder->setAlfCtrl(true);
          m_pcCavlcCoder->setMaxAlfCtrlDepth(uiMaxAlfCtrlDepth); //D0201
          }
        }
        else
        {
          m_pcEntropyCoder->setAlfCtrl(false);
        }
        m_pcEntropyCoder->encodeAlfParam(&cAlfParam);
        m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
      }

      // EXCBand
      if(pcSlice->getSPS()->getUseExC())
      {
        //condition from within xCorrBand function moved here //kolya
        if ( !(pcPic->getSlice()->getSliceType()== B_SLICE  && pcPic->getFrameHeightInCU() <= EXC_NO_EC_BSLICE) )
          xCorrBand( pcPic, &pcPicD );
      }

      // File writing
      m_pcSliceEncoder->encodeSliceCU( pcPic, pcBitstreamOut );

      //  End of bitstream & byte align
      if( pcSlice->getSymbolMode() )
      {
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
      }

      pcBitstreamOut->flushBuffer();

			// de-scaling of picture
      xDeScalePic( pcPic, &pcPicD );

			// save original picture
			cPicOrg.copyToPic( pcPic->getPicYuvOrg() );

      //-- For time output for each slice
      Double dEncTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;

      xCalculateAddPSNR( pcPic, &pcPicD, pcBitstreamOut->getNumberOfWrittenBits(), dEncTime );

			// free original picture
			cPicOrg.destroy();

      //  Reconstruction buffer update
      pcPicD.copyToPic(pcPicYuvRecOut);

      pcPicD.destroy();

      pcPic->setReconMark   ( true );

      m_bFirst = false;
      m_iNumPicCoded++;
    }

    // generalized B info.
    if ( m_pcCfg->getHierarchicalCoding() == false && iDepth != 0 )
      break;
  }

  assert ( m_iNumPicCoded == iNumPicRcvd );
}

// EXCBand
Void
TEncGOP::xCorrBand(TComPic* pcPic, TComPicYuv* pcPicD)
{
	//TODO: fix array indexing for it not to be extra big (64 -> 16 I mean etc.) //kolya

	pcPic->setMaxBandNumber(EXB_NB);

	//kolya
	Int iMaxBandNumber = EXB_NB;
	Int iExtractingMask = (EXB_NB - 1) << (12 - EXB_BITS); // to get 4 eldest bits
	// 12 (tending to infinity) here is from BitDepthIncrease

	Int x, y;
	Pel* pRec = pcPic->getPicYuvRec()->getLumaAddr();
	Pel* pOrg = pcPic ->getPicYuvOrg()->getLumaAddr();
	Int iStride = pcPic->getStride();
	Int iWidth = pcPic->getPicYuvOrg()->getWidth();
	Int iHeight = pcPic->getPicYuvOrg()->getHeight();

	//kolya //TODO: change 64 to smth more intuitively clear
	Int iCounter[64];
	Double dSum[64];
	Int i;
	::memset(iCounter, 0, sizeof(int) * 64);
	::memset(dSum, 0, sizeof(double) * 64 );


	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			dSum[ (pRec[x] & iExtractingMask) >> (12 - EXB_BITS) ] += (pOrg[x] - pRec[x]);
			iCounter[ (pRec[x] & iExtractingMask) >> (12 - EXB_BITS) ] += 1;
		}
		pRec += iStride;
		pOrg += iStride;
	}

	Int iCor[64];

	for ( i = 0; i < iMaxBandNumber; i++)
	{
		double a = 0.0;

		if (iCounter[i] != 0)
			a = dSum[i] / iCounter[i];
		else
			iCor[i] = 0;
#if !EXB_CUT
		if (a >= 0.0)
			iCor[i] = (int) (a + 0.5);
		else
			iCor[i] = (int) (a - 0.5);
#else
		if(a >= 0.0)
			iCor[i]=(int)(a/2.0+0.5);
		else
			iCor[i]=(int)(a/2.0-0.5);
#endif

	}


	////for overhead writing

	for ( i = 0; i < iMaxBandNumber; i++)
	{
		pcPic->setCorrBandExType(i, iCor[i]);
	}
	///////
	pRec = pcPic->getPicYuvRec()->getLumaAddr();

	Pel   iMax = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);

	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
#if EXB_CUT
			pRec[x] = Clip3(0,iMax,pRec[x] + 2*iCor[ (pRec[x] & iExtractingMask) >> (12 - EXB_BITS) ]);
#else
			pRec[x] = Clip3(0,iMax,pRec[x] + iCor[ (pRec[x] & iExtractingMask) >> (12 - EXB_BITS) ];
#endif
		}
		pRec += iStride;
	}

}
// EXCBand


Void
TEncGOP::xExtremeCorr( TComPic* pcPic, TComPicYuv* pcPicD  )
{
  Int     x, y;
  Pel*  pRec    = pcPic->getPicYuvRec()->getLumaAddr();
  Pel*  pRecD   = pcPicD->getLumaAddr();
  Pel*  pOrg    = pcPic ->getPicYuvOrg()->getLumaAddr();
  Int   iStride = pcPic->getStride();
  Int   iWidth  = pcPic->getPicYuvOrg()->getWidth();
  Int   iHeight = pcPic->getPicYuvOrg()->getHeight();

  Pel   iMax = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);

  Int iMinNumber[5];
  Int iMaxNumber[5];
  Int iMinValue[5];
  Int iMaxValue[5];

  ::memset(iMinNumber,  0, sizeof(Int)*5);
  ::memset(iMaxNumber,  0, sizeof(Int)*5);
  ::memset(iMinValue,  0, sizeof(Int)*5);
  ::memset(iMaxValue,  0, sizeof(Int)*5);

  Int iExtremeCorrLimit = EXC_CORR_LIMIT;

  Int iExtremeCorr = 0;
  Int iExtremeType = 0;

  Int iLeft = 0;
  Int iRight = 0;
  Int iSignL = 0;
  Int iSignR = 0;

  pRec += iStride;
  pOrg += iStride;

    for (y = 1; y < iHeight - 1; y++)
        {
          iLeft = pRec[0] - pRec[1];
          iSignL = mSign(iLeft);

          for (x = 1; x < iWidth - 1; x++)
            {
              iExtremeType = 0;
              iExtremeCorr = 0;

              // left-right pair
              iRight = pRec[x + 1] - pRec[x];
              iSignR = mSign(iRight);

              iExtremeType = iExtremeType + iSignR + iSignL;

              // save sign for next step
              iSignL = -iSignR;

              // upper-lower pair
              // TODO: may be optimized thru additional array of upper signs
              //kolya
              iExtremeType += mSign(pRec[x + iStride] - pRec[x]);
              iExtremeType += mSign(pRec[x - iStride] - pRec[x]);

              if (iExtremeType > iExtremeCorrLimit)
                {
                  iMinNumber[iExtremeType]++;
                  iMinValue[iExtremeType] += (pOrg[x] - pRec[x]);
                }

              else if (iExtremeType < -iExtremeCorrLimit)
                {
                  iMaxNumber[-iExtremeType]++;
                  iMaxValue[-iExtremeType] += (pOrg[x] - pRec[x]);
                }
            }
          pRec += iStride;
          pOrg += iStride;
        }

    for (iExtremeType = iExtremeCorrLimit + 1; iExtremeType < 5; iExtremeType++)
    {

      if (iMinNumber[iExtremeType])
        if (iMinValue[iExtremeType] > 0)
          iMinValue[iExtremeType] = (iMinValue[iExtremeType]
              + iMinNumber[iExtremeType] / 2) / iMinNumber[iExtremeType];
        else
          iMinValue[iExtremeType] = (iMinValue[iExtremeType]
              - iMinNumber[iExtremeType] / 2) / iMinNumber[iExtremeType];

      if (iMaxNumber[iExtremeType])
        if (iMaxValue[iExtremeType] > 0)
          iMaxValue[iExtremeType] = (iMaxValue[iExtremeType]
              + iMaxNumber[iExtremeType] / 2) / iMaxNumber[iExtremeType];
        else
          iMaxValue[iExtremeType] = (iMaxValue[iExtremeType]
              - iMaxNumber[iExtremeType] / 2) / iMaxNumber[iExtremeType];
      ///
      iMinValue[iExtremeType]
          = Max( Min(iMinValue[iExtremeType], cgaiCorrDiapason[iExtremeType][1]), -cgaiCorrDiapason[iExtremeType][0]); //  -3...60
      iMaxValue[iExtremeType]
          = Max(Min(iMaxValue[iExtremeType],cgaiCorrDiapason[iExtremeType][0]), -cgaiCorrDiapason[iExtremeType][1]); // -60...3
      ///
      pcPic->setLocalExtremeValues(iExtremeType, iMinValue[iExtremeType],
          iMaxValue[iExtremeType]);
    }

  pRecD   = pcPicD->getLumaAddr();
  pRec    = pcPic->getPicYuvRec()->getLumaAddr();

  pRecD += iStride;
  pRec += iStride;

  // As decoder does
  iExtremeCorrLimit = EXC_CORR_LIMIT;
  iExtremeCorr = 0;
  iExtremeType = 0;

  iLeft = 0;
  iRight = 0;
  iSignL = 0;
  iSignR = 0;

  for (y = 1; y < iHeight - 1; y++)
    {
      iLeft = pRec[0] - pRec[1];
      iSignL = mSign(iLeft);

      for (x = 1; x < iWidth - 1; x++)
        {
          iExtremeCorr = 0;
          iExtremeType = 0;

          // left-right pair
          iRight = pRec[x + 1] - pRec[x];
          iSignR = mSign(iRight);

          iExtremeType = iExtremeType + iSignR + iSignL;

          // save sign for next step
          iSignL = -iSignR;

          // upper-lower pair
          // TODO: may be optimized thru additional array of upper signs
          //kolya
          iExtremeType += mSign(pRec[x + iStride] - pRec[x]);
          iExtremeType += mSign(pRec[x - iStride] - pRec[x]);

          if (iExtremeType > iExtremeCorrLimit)
            iExtremeCorr = pcPic->getMinValCorr(iExtremeType);
          else if (iExtremeType < -iExtremeCorrLimit)
            iExtremeCorr = pcPic->getMaxValCorr(-iExtremeType);

          //pRecD[x] =pRec[x]+iExtremeCorr;
          pRecD[x] = Clip3(0,iMax,(Int) pRec[x]+iExtremeCorr);
        }
      pRecD += iStride;
      pRec += iStride;
    }

  //Restore addresses

  pRecD   = pcPicD->getLumaAddr();
  pRec    = pcPic->getPicYuvRec()->getLumaAddr();

  pRecD += iStride;
  pRec += iStride;

  pRecD += 1;
  pRec += 1;

  for( y = 1; y < iHeight - 1; y++ )
  {
    ::memcpy(pRec, pRecD,sizeof(Pel)*(iWidth - 2));
    pRecD += iStride;
    pRec += iStride;
  }
}

Void TEncGOP::xScalePic( TComPic* pcPic )
{
  Int     x, y;

  //===== calculate PSNR =====
  Pel*  pRec    = pcPic->getPicYuvOrg()->getLumaAddr();
  Int   iStride = pcPic->getStride();
  Int   iWidth  = pcPic->getPicYuvOrg()->getWidth();
  Int   iHeight = pcPic->getPicYuvOrg()->getHeight();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      pRec[x] = Clip(CADR_FWD_L( pRec[x]<<g_uiBitIncrement, g_iMinCADR<<g_uiBitIncrement, g_iRangeCADR ));
    }
    pRec += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pRec  = pcPic->getPicYuvOrg()->getCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      pRec[x] = CADR_FWD_C( pRec[x]<<g_uiBitIncrement, CADR_OFFSET<<g_uiBitIncrement, g_iRangeCADR );
    }
    pRec += iStride;
  }

  pRec  = pcPic->getPicYuvOrg()->getCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      pRec[x] = CADR_FWD_C( pRec[x]<<g_uiBitIncrement, CADR_OFFSET<<g_uiBitIncrement, g_iRangeCADR );
    }
    pRec += iStride;
  }
}

Void TEncGOP::xDeScalePic( TComPic* pcPic, TComPicYuv* pcPicD )
{
  Int     x, y;
  Int     offset = (g_uiBitIncrement>0)?(1<<(g_uiBitIncrement-1)):0;

  //===== calculate PSNR =====
  Pel*  pRecD   = pcPicD->getLumaAddr();
  Pel*  pRec    = pcPic->getPicYuvRec()->getLumaAddr();
  Int   iStride = pcPic->getStride();
  Int   iWidth  = pcPic->getPicYuvOrg()->getWidth();
  Int   iHeight = pcPic->getPicYuvOrg()->getHeight();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
			pRecD[x] = Clip3(0, g_uiBASE_MAX, ( CADR_INV_L( pRec[x], g_iMinCADR<<g_uiBitIncrement, g_iRangeCADR ) + offset ) >> g_uiBitIncrement );
    }
    pRecD += iStride;
    pRec += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pRecD = pcPicD->getCbAddr();
  pRec  = pcPic->getPicYuvRec()->getCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      pRecD[x] = Clip3(0, g_uiBASE_MAX, ( CADR_INV_C( pRec[x], CADR_OFFSET<<g_uiBitIncrement, g_iRangeCADR ) + offset ) >> g_uiBitIncrement );
    }
    pRecD += iStride;
    pRec += iStride;
  }

  pRecD = pcPicD->getCrAddr();
  pRec  = pcPic->getPicYuvRec()->getCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      pRecD[x] = Clip3(0, g_uiBASE_MAX, ( CADR_INV_C( pRec[x], CADR_OFFSET<<g_uiBitIncrement, g_iRangeCADR ) + offset ) >> g_uiBitIncrement );
    }
    pRecD += iStride;
    pRec += iStride;
  }
}

UInt64 TEncGOP::xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1)
{
  Int     x, y;
  Pel*  pSrc0   = pcPic0 ->getLumaAddr();
  Pel*  pSrc1   = pcPic1 ->getLumaAddr();
  UInt  uiShift = g_uiBitIncrement<<1;
  Int   iTemp;

  Int   iStride = pcPic0->getStride();
  Int   iWidth  = pcPic0->getWidth();
  Int   iHeight = pcPic0->getHeight();

  UInt64  uiTotalDiff = 0;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;

  pSrc0  = pcPic0->getCbAddr();
  pSrc1  = pcPic1->getCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }

  pSrc0  = pcPic0->getCrAddr();
  pSrc1  = pcPic1->getCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }

  return uiTotalDiff;
}

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, UInt uibits, Double dEncTime )
{
  Int     x, y;
  UInt    uiSSDY  = 0;
  UInt    uiSSDU  = 0;
  UInt    uiSSDV  = 0;

  Double  dYPSNR  = 0.0;
  Double  dUPSNR  = 0.0;
  Double  dVPSNR  = 0.0;

  Int     offset = (g_uiBitIncrement>0)?(1<<(g_uiBitIncrement-1)):0;

  //===== calculate PSNR =====
  Pel*  pOrg    = pcPic ->getPicYuvOrg()->getLumaAddr();
  Pel*  pRec    = pcPicD->getLumaAddr();
  Int   iStride = pcPicD->getStride();

  Int   iWidth;
  Int   iHeight;

  iWidth  = pcPicD->getWidth() - m_pcEncTop->getPad(0);
  iHeight = pcPicD->getHeight() - m_pcEncTop->getPad(1);

  Int   iSize   = iWidth*iHeight;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDY   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pOrg  = pcPic ->getPicYuvOrg()->getCbAddr();
  pRec  = pcPicD->getCbAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDU   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }

  pOrg  = pcPic ->getPicYuvOrg()->getCrAddr();
  pRec  = pcPicD->getCrAddr();

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDV   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }

  Double fRefValueY = 255.0 * 255.0 * (Double)iSize;
  Double fRefValueC = fRefValueY / 4.0;
  dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
  dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
  dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );

  //===== add PSNR =====
  m_gcAnalyzeAll.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  if (pcPic->getSlice()->isIntra())
  {
    m_gcAnalyzeI.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcPic->getSlice()->isInterP())
  {
    m_gcAnalyzeP.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcPic->getSlice()->isInterB())
  {
    m_gcAnalyzeB.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }

  //===== output =====
  TComSlice*  pcSlice = pcPic->getSlice();
  printf("\nPOC %4d ( %c-SLICE, QP %d ) %10d bits ",
                                                              pcSlice->getPOC(),
                                                              pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B',
                                                              pcSlice->getSliceQp(),
                                                              uibits );
  printf( "[Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]  ", dYPSNR, dUPSNR, dVPSNR );
  printf ("[ET %5.0f ] ", dEncTime );

  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf ("[L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      UInt uiOrgNumRefIdx;
      uiOrgNumRefIdx = pcSlice->getNumRefIdx(RefPicList(iRefList))-pcSlice->getAddRefCnt(RefPicList(iRefList));
      UInt uiNewRefIdx= iRefIndex-uiOrgNumRefIdx;

      if (iRefIndex >= uiOrgNumRefIdx)
	  {
        if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_P1)
        {
          printf ("%dr+ ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_M1)
        {
          printf ("%dr- ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_SO || pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_O)
        {
          printf ("%dw ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_AM)
        {
          printf ("%da ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_IM)
        {
          printf ("%di ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_PM)
        {
          printf ("%dp ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
      }
      else
      {
        printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
    }
    printf ("] ");
  }

	fflush(stdout);
}

Void TEncGOP::printOutSummary(UInt uiNumAllPicCoded)
{
  assert (uiNumAllPicCoded == m_gcAnalyzeAll.getNumPic());

  //--CFG_KDY
  m_gcAnalyzeAll.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeI.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeP.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeB.setFrmRate( m_pcCfg->getFrameRate() );

  //-- all
  printf( "\n\nSUMMARY --------------------------------------------------------\n" );
  m_gcAnalyzeAll.printOut('a');

  printf( "\n\nI Slices--------------------------------------------------------\n" );
  m_gcAnalyzeI.printOut('i');

  printf( "\n\nP Slices--------------------------------------------------------\n" );
  m_gcAnalyzeP.printOut('p');

  printf( "\n\nB Slices--------------------------------------------------------\n" );
  m_gcAnalyzeB.printOut('b');


#if _SUMMARY_OUT_
  m_gcAnalyzeAll.printSummaryOut();
#endif
#if _SUMMARY_PIC_
  m_gcAnalyzeI.printSummary('I');
  m_gcAnalyzeP.printSummary('P');
  m_gcAnalyzeB.printSummary('B');
#endif
}

Void TEncGOP::preLoopFilterPicAll( TComPic* pcPic, UInt64& ruiDist, UInt64& ruiBits )
{
  TComSlice* pcSlice = pcPic->getSlice();
  Bool bCalcDist = false;

  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterAlphaC0Offget(), m_pcCfg->getLoopFilterBetaOffget());
  m_pcLoopFilter->loopFilterPic( pcPic );

  TComPicYuv cTmpPic;
  cTmpPic.create( pcPic->getPicYuvOrg()->getWidth(), pcPic->getPicYuvOrg()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  Int iExtremeType = 0;
  for (iExtremeType = 0; iExtremeType <5; iExtremeType ++)
    pcPic->setLocalExtremeValues(iExtremeType,0,0);

  // Moved here condition check to avoid extra function call //kolya
  if ( pcPic->getSlice()->getSliceType()!= B_SLICE  || pcPic->getFrameHeightInCU()>EXC_NO_EC_BSLICE )
  xExtremeCorr( pcPic,  &cTmpPic  );

  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcSlice );
  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );

  // Adaptive Loop filter
  if( pcSlice->getSPS()->getUseALF() )
  {
    ALFParam cAlfParam;
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
    m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder);
    UInt uiMaxAlfCtrlDepth;
    m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, pcPic->getSlice()->getLambda(), ruiDist, ruiBits, uiMaxAlfCtrlDepth );
    m_pcAdaptiveLoopFilter->endALFEnc();
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
  }

  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->encodeExtremeValue(pcPic);
  ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();

  cTmpPic.destroy();

  if (!bCalcDist)
  ruiDist = xFindDistortionFrame(pcPic->getPicYuvOrg(), pcPic->getPicYuvRec());

}


