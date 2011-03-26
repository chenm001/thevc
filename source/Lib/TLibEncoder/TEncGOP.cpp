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

/** \file     TEncGOP.cpp
    \brief    GOP encoder class
*/

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"

#include <time.h>

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TEncGOP::TEncGOP()
{
  m_iHrchDepth          = 0;
  m_iGopSize            = 0;
  m_iNumPicCoded        = 0; //Niko
  m_bFirst              = true;
  
  m_pcCfg               = NULL;
  m_pcSliceEncoder      = NULL;
  m_pcListPic           = NULL;
  
  m_pcEntropyCoder      = NULL;
  m_pcCavlcCoder        = NULL;
  m_pcSbacCoder         = NULL;
  m_pcBinCABAC          = NULL;
  
  m_bSeqFirst           = true;
  
#if DCM_DECODING_REFRESH
  m_bRefreshPending     = 0;
  m_uiPOCCDR            = 0;
#endif

  return;
}

TEncGOP::~TEncGOP()
{
}

/** Create list to contain pointers to LCU start addresses of slice.
 * \param iWidth, iHeight are picture width, height. iMaxCUWidth, iMaxCUHeight are LCU width, height.
 */
Void  TEncGOP::create( Int iWidth, Int iHeight, UInt iMaxCUWidth, UInt iMaxCUHeight )
{
  UInt uiWidthInCU       = ( iWidth %iMaxCUWidth  ) ? iWidth /iMaxCUWidth  + 1 : iWidth /iMaxCUWidth;
  UInt uiHeightInCU      = ( iHeight%iMaxCUHeight ) ? iHeight/iMaxCUHeight + 1 : iHeight/iMaxCUHeight;
  UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;
  m_uiStoredStartCUAddrForEncodingSlice = new UInt [uiNumCUsInFrame+1];
  m_uiStoredStartCUAddrForEncodingEntropySlice = new UInt [uiNumCUsInFrame+1];
}

Void  TEncGOP::destroy()
{
  delete [] m_uiStoredStartCUAddrForEncodingSlice; m_uiStoredStartCUAddrForEncodingSlice = NULL;
  delete [] m_uiStoredStartCUAddrForEncodingEntropySlice; m_uiStoredStartCUAddrForEncodingEntropySlice = NULL;
}

Void TEncGOP::init ( TEncTop* pcTEncTop )
{
  m_pcEncTop     = pcTEncTop;
  m_pcCfg                = pcTEncTop;
  m_pcSliceEncoder       = pcTEncTop->getSliceEncoder();
  m_pcListPic            = pcTEncTop->getListPic();
  
  m_pcEntropyCoder       = pcTEncTop->getEntropyCoder();
  m_pcCavlcCoder         = pcTEncTop->getCavlcCoder();
  m_pcSbacCoder          = pcTEncTop->getSbacCoder();
  m_pcBinCABAC           = pcTEncTop->getBinCABAC();
  m_pcLoopFilter         = pcTEncTop->getLoopFilter();
  m_pcBitCounter         = pcTEncTop->getBitCounter();
  
  // Adaptive Loop filter
  m_pcAdaptiveLoopFilter = pcTEncTop->getAdaptiveLoopFilter();
  //--Adaptive Loop filter
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, TComList<TComBitstream*> rcListBitstreamOut )
{
  TComPic*        pcPic;
  TComPicYuv*     pcPicYuvRecOut;
  TComBitstream*  pcBitstreamOut;
  TComSlice*      pcSlice;
  
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
      
      //  Bitstream reset
      pcBitstreamOut->resetBits();
      pcBitstreamOut->rewindStreamPacket();
      
      //  Slice data initialization
      pcPic->clearSliceBuffer();
      assert(pcPic->getNumAllocatedSlice() == 1);
      m_pcSliceEncoder->setSliceIdx(0);
      pcPic->setCurrSliceIdx(0);
      m_pcSliceEncoder->initEncSlice ( pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iTimeOffset, iDepth, pcSlice );
      pcSlice->setSliceIdx(0);
      
      //  Set SPS
      pcSlice->setSPS( m_pcEncTop->getSPS() );
      pcSlice->setPPS( m_pcEncTop->getPPS() );
      
#if DCM_DECODING_REFRESH
      // Set the nal unit type
      pcSlice->setNalUnitType(getNalUnitType(uiPOCCurr));
      // Do decoding refresh marking if any 
      pcSlice->decodingRefreshMarking(m_uiPOCCDR, m_bRefreshPending, rcListPic);
#endif

      //  Set reference list
      pcSlice->setRefPicList ( rcListPic );
      
      //  Slice info. refinement
      if ( (pcSlice->getSliceType() == B_SLICE) && (pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0) )
      {
        pcSlice->setSliceType ( P_SLICE );
        pcSlice->setDRBFlag   ( true );
      }
      
      // Generalized B
      if ( m_pcCfg->getUseGPB() )
      {
        if (pcSlice->getSliceType() == P_SLICE)
        {
          pcSlice->setSliceType( B_SLICE ); // Change slice type by force
          
#if DCM_COMB_LIST
          if(pcSlice->getSPS()->getUseLComb() && (m_pcCfg->getNumOfReferenceB_L1() < m_pcCfg->getNumOfReferenceB_L0()) && (pcSlice->getNumRefIdx(REF_PIC_LIST_0)>1))
          {
            pcSlice->setNumRefIdx( REF_PIC_LIST_1, m_pcCfg->getNumOfReferenceB_L1() );

            for (Int iRefIdx = 0; iRefIdx < m_pcCfg->getNumOfReferenceB_L1(); iRefIdx++)
            {
              pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
            }
          }
          else
          {
#endif
          Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
          pcSlice->setNumRefIdx( REF_PIC_LIST_1, iNumRefIdx );
          
          for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
          {
            pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
          }
#if DCM_COMB_LIST
          }
#endif
        }
      }

#if DCM_COMB_LIST
      if (pcSlice->getSliceType() != B_SLICE || !pcSlice->getSPS()->getUseLComb())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, -1);
        pcSlice->setRefPicListCombinationFlag(false);
        pcSlice->setRefPicListModificationFlagLC(false);
      }
      else
      {
        pcSlice->setRefPicListCombinationFlag(pcSlice->getSPS()->getUseLComb());
        pcSlice->setRefPicListModificationFlagLC(pcSlice->getSPS()->getLCMod());
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, pcSlice->getNumRefIdx(REF_PIC_LIST_0));
      }
#endif
      
      if (pcSlice->getSliceType() == B_SLICE)
      {
        pcSlice->setColDir(uiColDir);
      }
      
      uiColDir = 1-uiColDir;
      
      //-------------------------------------------------------------
      pcSlice->setRefPOCList();
      
      pcSlice->setNoBackPredFlag( false );
#if DCM_COMB_LIST
      if ( pcSlice->getSliceType() == B_SLICE && !pcSlice->getRefPicListCombinationFlag())
#else
      if ( pcSlice->getSliceType() == B_SLICE )
#endif
      {
        if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
        {
          pcSlice->setNoBackPredFlag( true );
          int i;
          for ( i=0; i < pcSlice->getNumRefIdx(RefPicList( 1 ) ); i++ )
          {
            if ( pcSlice->getRefPOC(RefPicList(1), i) != pcSlice->getRefPOC(RefPicList(0), i) ) 
            {
              pcSlice->setNoBackPredFlag( false );
              break;
            }
          }
        }
      }

#if DCM_COMB_LIST
      if(pcSlice->getNoBackPredFlag())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, -1);
      }
      pcSlice->generateCombinedList();
#endif
      
      /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a slice
      //  Slice compression
      if (m_pcCfg->getUseASR())
      {
        m_pcSliceEncoder->setSearchRange(pcSlice);
      }
#ifdef ROUNDING_CONTROL_BIPRED
      Bool b = true;
      if (m_pcCfg->getUseRoundingControlBipred())
      {
        if (m_pcCfg->getGOPSize()==1)
          b = ((pcSlice->getPOC()&1)==0);
        else
          b = (pcSlice->isReferenced() == 0);
      }

#if HIGH_ACCURACY_BI
      pcSlice->setRounding(false);
#else
      pcSlice->setRounding(b);
#endif
#endif

      UInt uiStartCUAddrSliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrSlice    = 0; // used to keep track of current slice's starting CU addr.
      pcSlice->setSliceCurStartCUAddr( uiStartCUAddrSlice ); // Setting "start CU addr" for current slice
      memset(m_uiStoredStartCUAddrForEncodingSlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()+1));

      UInt uiStartCUAddrEntropySliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEntropyEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrEntropySlice    = 0; // used to keep track of current Entropy slice's starting CU addr.
      pcSlice->setEntropySliceCurStartCUAddr( uiStartCUAddrEntropySlice ); // Setting "start CU addr" for current Entropy slice
      memset(m_uiStoredStartCUAddrForEncodingEntropySlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()+1));

      UInt uiNextCUAddr = 0;
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = uiNextCUAddr;
      m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiNextCUAddr;

      while(uiNextCUAddr<pcPic->getPicSym()->getNumberOfCUsInFrame()) // determine slice boundaries
      {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextEntropySlice( false );
        assert(pcPic->getNumAllocatedSlice() == uiStartCUAddrSliceIdx);
        m_pcSliceEncoder->precompressSlice( pcPic );
        m_pcSliceEncoder->compressSlice   ( pcPic );

        Bool bNoBinBitConstraintViolated = (!pcSlice->isNextSlice() && !pcSlice->isNextEntropySlice());
        if (pcSlice->isNextSlice() || (bNoBinBitConstraintViolated && m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE))
        {
          uiStartCUAddrSlice                                              = pcSlice->getSliceCurEndCUAddr();
          // Reconstruction slice
          m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]  = uiStartCUAddrSlice;
          // Entropy slice
          if (uiStartCUAddrEntropySliceIdx>0 && m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx-1] != uiStartCUAddrSlice)
          {
            m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiStartCUAddrSlice;
          }
          
          if (uiStartCUAddrSlice < pcPic->getPicSym()->getNumberOfCUsInFrame())
          {
            pcPic->allocateNewSlice();          
            pcPic->setCurrSliceIdx                  ( uiStartCUAddrSliceIdx-1 );
            m_pcSliceEncoder->setSliceIdx           ( uiStartCUAddrSliceIdx-1 );
            pcSlice = pcPic->getSlice               ( uiStartCUAddrSliceIdx-1 );
            pcSlice->copySliceInfo                  ( pcPic->getSlice(0)      );
            pcSlice->setSliceIdx                    ( uiStartCUAddrSliceIdx-1 );
            pcSlice->setSliceCurStartCUAddr         ( uiStartCUAddrSlice      );
            pcSlice->setEntropySliceCurStartCUAddr  ( uiStartCUAddrSlice      );
            pcSlice->setSliceBits(0);
          }
        }
        else if (pcSlice->isNextEntropySlice() || (bNoBinBitConstraintViolated && m_pcCfg->getEntropySliceMode()==SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE))
        {
          uiStartCUAddrEntropySlice                                                     = pcSlice->getEntropySliceCurEndCUAddr();
          m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiStartCUAddrEntropySlice;
          pcSlice->setEntropySliceCurStartCUAddr( uiStartCUAddrEntropySlice );
        }
        else
        {
          uiStartCUAddrSlice                                                            = pcSlice->getSliceCurEndCUAddr();
          uiStartCUAddrEntropySlice                                                     = pcSlice->getEntropySliceCurEndCUAddr();
        }        

        uiNextCUAddr = (uiStartCUAddrSlice > uiStartCUAddrEntropySlice) ? uiStartCUAddrSlice : uiStartCUAddrEntropySlice;
      }
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = pcSlice->getSliceCurEndCUAddr();
      m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = pcSlice->getSliceCurEndCUAddr();
      
      pcSlice = pcPic->getSlice(0);

      //-- Loop filter
      m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterAlphaC0Offget(), m_pcCfg->getLoopFilterBetaOffget());
      m_pcLoopFilter->loopFilterPic( pcPic );

#if MTK_NONCROSS_INLOOP_FILTER
      pcSlice = pcPic->getSlice(0);

      if(pcSlice->getSPS()->getUseALF())
      {
        if(pcSlice->getSPS()->getLFCrossSliceBoundaryFlag())
        {
          m_pcAdaptiveLoopFilter->setUseNonCrossAlf(false);
        }
        else
        {
          UInt uiNumSlices = uiStartCUAddrSliceIdx-1;
          m_pcAdaptiveLoopFilter->setUseNonCrossAlf( (uiNumSlices > 1)  );
          if(m_pcAdaptiveLoopFilter->getUseNonCrossAlf())
          {
            m_pcAdaptiveLoopFilter->setNumSlicesInPic( uiNumSlices );
            m_pcAdaptiveLoopFilter->createSlice();

            //set the startLCU and endLCU addr. to ALF slices
            for(UInt i=0; i< uiNumSlices ; i++)
            {
              (*m_pcAdaptiveLoopFilter)[i].create(pcPic, i, 
                                                  m_uiStoredStartCUAddrForEncodingSlice[i], 
                                                  m_uiStoredStartCUAddrForEncodingSlice[i+1]-1
                                                  );

            }
          }
        }
      }
#endif
      /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
      // Set entropy coder
      m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
      m_pcEntropyCoder->setBitstream      ( pcBitstreamOut          );
      
      // write SPS
      if ( m_bSeqFirst )
      {
        m_pcEntropyCoder->encodeSPS( pcSlice->getSPS() );
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
        // generate start code
        pcBitstreamOut->write( 1, 32);
        
        m_pcEntropyCoder->encodePPS( pcSlice->getPPS() );
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
        // generate start code
        pcBitstreamOut->write( 1, 32);
        m_bSeqFirst = false;
      }
      
      UInt uiPosBefore      = pcBitstreamOut->getNumberOfWrittenBits()>>3;
      uiStartCUAddrSliceIdx = 0;
      uiStartCUAddrSlice    = 0; 
      pcBitstreamOut->allocateMemoryForSliceLocations( pcPic->getPicSym()->getNumberOfCUsInFrame() ); // Assuming number of slices <= number of LCU. Needs to be changed for sub-LCU slice coding.
      pcBitstreamOut->setSliceCount( 0 );                                      // intialize number of slices to zero, used while converting RBSP to NALU

      uiStartCUAddrEntropySliceIdx = 0;
      uiStartCUAddrEntropySlice    = 0; 
      uiNextCUAddr                 = 0;
      pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);
      while (uiNextCUAddr < pcPic->getPicSym()->getNumberOfCUsInFrame()) // Iterate over all slices
      {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextEntropySlice( false );
        if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx])
        {
          pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);
          pcPic->setCurrSliceIdx(uiStartCUAddrSliceIdx);
          m_pcSliceEncoder->setSliceIdx(uiStartCUAddrSliceIdx);
          assert(uiStartCUAddrSliceIdx == pcSlice->getSliceIdx());
          // Reconstruction slice
          pcSlice->setSliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setSliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx+1 ] );
          // Entropy slice
          pcSlice->setEntropySliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setEntropySliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx+1 ] );

          pcSlice->setNextSlice       ( true );

          uiStartCUAddrSliceIdx++;
          uiStartCUAddrEntropySliceIdx++;
        } 
        else if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx])
        {
          // Entropy slice
          pcSlice->setEntropySliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setEntropySliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx+1 ] );

          pcSlice->setNextEntropySlice( true );

          uiStartCUAddrEntropySliceIdx++;
        }

        // Get ready for writing slice header (other than the first one in the picture)
        if (uiNextCUAddr!=0)
        {
          m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
          m_pcEntropyCoder->setBitstream      ( pcBitstreamOut          );
          m_pcEntropyCoder->resetEntropy      ();
        }

      // write SliceHeader
      m_pcEntropyCoder->encodeSliceHeader ( pcSlice                 );
      
      // is it needed?
      if ( pcSlice->getSymbolMode() )
      {
        m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
        m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
        m_pcEntropyCoder->resetEntropy    ();
      }
      
        if (uiNextCUAddr==0)  // Compute ALF params and write only for first slice header
        {
          // adaptive loop filter
          if ( pcSlice->getSPS()->getUseALF() )
          {
            ALFParam cAlfParam;
#if TSB_ALF_HEADER
            m_pcAdaptiveLoopFilter->setNumCUsInFrame(pcPic);
#endif
            m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
            
            // set entropy coder for RD
            if ( pcSlice->getSymbolMode() )
            {
              m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcSlice );
            }
            else
            {
              m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
            }
            m_pcEntropyCoder->resetEntropy    ();
            m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
            
            m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder );
            
            UInt uiMaxAlfCtrlDepth;
            
            UInt64 uiDist, uiBits;
            m_pcAdaptiveLoopFilter->ALFProcess( &cAlfParam, pcSlice->getLambda(), uiDist, uiBits, uiMaxAlfCtrlDepth );
            m_pcAdaptiveLoopFilter->endALFEnc();
            
            // set entropy coder for writing
            m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
            
            if ( pcSlice->getSymbolMode() )
            {
              m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
            }
            else
            {
              m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
            }
            m_pcEntropyCoder->resetEntropy    ();
            m_pcEntropyCoder->setBitstream    ( pcBitstreamOut );
            if (cAlfParam.cu_control_flag)
            {
              m_pcEntropyCoder->setAlfCtrl( true );
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
            
#if TSB_ALF_HEADER
            if(cAlfParam.cu_control_flag)
            {
              m_pcEntropyCoder->encodeAlfCtrlParam(&cAlfParam);
            }
#endif
            
            m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
          }
        }
        
        // File writing
        m_pcSliceEncoder->encodeSlice( pcPic, pcBitstreamOut );
        
        //  End of bitstream & byte align
        pcBitstreamOut->write( 1, 1 );
        pcBitstreamOut->writeAlignZero();
        
        UInt uiBoundingAddrSlice, uiBoundingAddrEntropySlice;
        uiBoundingAddrSlice        = m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx];          
        uiBoundingAddrEntropySlice = m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx];          
        uiNextCUAddr               = min(uiBoundingAddrSlice, uiBoundingAddrEntropySlice);
        if (uiNextCUAddr < pcPic->getPicSym()->getNumberOfCUsInFrame())   // if more slices to be encoded insert start code
        {
          UInt uiSliceCount = pcBitstreamOut->getSliceCount();
          pcBitstreamOut->setSliceByteLocation( uiSliceCount, (pcBitstreamOut->getNumberOfWrittenBits()>>3) );
          pcBitstreamOut->setSliceCount( uiSliceCount+1 );
          pcBitstreamOut->write( 1, 32);
        }
      } // end iteration over slices
      
      
#if MTK_NONCROSS_INLOOP_FILTER
      if(pcSlice->getSPS()->getUseALF())
      {
        if(m_pcAdaptiveLoopFilter->getUseNonCrossAlf())
          m_pcAdaptiveLoopFilter->destroySlice();
      }
#endif 
      
      
      pcBitstreamOut->flushBuffer();
      pcBitstreamOut->convertRBSPToPayload( uiPosBefore );
      
#if AMVP_BUFFERCOMPRESS
      pcPic->compressMotion(); 
#endif 
      pcBitstreamOut->freeMemoryAllocatedForSliceLocations();
      
      //-- For time output for each slice
      Double dEncTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
      
      xCalculateAddPSNR( pcPic, pcPic->getPicYuvRec(), pcBitstreamOut->getNumberOfWrittenBits(), dEncTime );

#if FIXED_ROUNDING_FRAME_MEMORY
      pcPic->getPicYuvRec()->xFixedRoundingPic();
#endif
      
      pcPic->getPicYuvRec()->copyToPic(pcPicYuvRecOut);
      
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
  TComSlice* pcSlice = pcPic->getSlice(pcPic->getCurrSliceIdx());
  Bool bCalcDist = false;
  
  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterAlphaC0Offget(), m_pcCfg->getLoopFilterBetaOffget());
  m_pcLoopFilter->loopFilterPic( pcPic );
  
  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcSlice );
  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  
  // Adaptive Loop filter
  if( pcSlice->getSPS()->getUseALF() )
  {
    ALFParam cAlfParam;
#if TSB_ALF_HEADER
    m_pcAdaptiveLoopFilter->setNumCUsInFrame(pcPic);
#endif
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
    
    m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder);
    
    UInt uiMaxAlfCtrlDepth;
    m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, pcSlice->getLambda(), ruiDist, ruiBits, uiMaxAlfCtrlDepth );
    m_pcAdaptiveLoopFilter->endALFEnc();
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
  }
  
  m_pcEntropyCoder->resetEntropy    ();
  ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  
  if (!bCalcDist)
    ruiDist = xFindDistortionFrame(pcPic->getPicYuvOrg(), pcPic->getPicYuvRec());
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

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
  
#if MQT_ALF_NPASS
  m_pcAdaptiveLoopFilter->setGOPSize( m_iGopSize );
#endif

  return;
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
    rpcPic->setCurrSliceIdx(0);
    if (rpcPic->getPOC() == (Int)uiPOCCurr)
    {
      break;
    }
    iterPic++;
  }
  
  assert (rpcPic->getPOC() == (Int)uiPOCCurr);
  
  return;
}

UInt64 TEncGOP::xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1)
{
  Int     x, y;
  Pel*  pSrc0   = pcPic0 ->getLumaAddr();
  Pel*  pSrc1   = pcPic1 ->getLumaAddr();
#if IBDI_DISTORTION
  Int  iShift = g_uiBitIncrement;
  Int  iOffset = 1<<(g_uiBitIncrement-1);
#else
  UInt  uiShift = g_uiBitIncrement<<1;
#endif
  Int   iTemp;
  
  Int   iStride = pcPic0->getStride();
  Int   iWidth  = pcPic0->getWidth();
  Int   iHeight = pcPic0->getHeight();
  
  UInt64  uiTotalDiff = 0;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
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
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
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
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  return uiTotalDiff;
}

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, UInt uibits, Double dEncTime )
{
  Int     x, y;
  UInt64 uiSSDY  = 0;
  UInt64 uiSSDU  = 0;
  UInt64 uiSSDV  = 0;
  
  Double  dYPSNR  = 0.0;
  Double  dUPSNR  = 0.0;
  Double  dVPSNR  = 0.0;
  
  //===== calculate PSNR =====
  Pel*  pOrg    = pcPic ->getPicYuvOrg()->getLumaAddr();
  Pel*  pRec    = pcPicD->getLumaAddr();
  Int   iStride = pcPicD->getStride();
  
  Int   iWidth;
  Int   iHeight;
  
  iWidth  = pcPicD->getWidth () - m_pcEncTop->getPad(0);
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
  
  unsigned int maxval = 255 * (1<<(g_uiBitDepth + g_uiBitIncrement -8));
  Double fRefValueY = (double) maxval * maxval * iSize;
  Double fRefValueC = fRefValueY / 4.0;
  dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
  dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
  dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  
  // fix: total bits should consider slice size bits (32bit)
  uibits += 32;
  
  //===== add PSNR =====
  m_gcAnalyzeAll.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  TComSlice*  pcSlice = pcPic->getSlice(0);
  if (pcSlice->isIntra())
  {
    m_gcAnalyzeI.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcSlice->isInterP())
  {
    m_gcAnalyzeP.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcSlice->isInterB())
  {
    m_gcAnalyzeB.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
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
      printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
    }
    printf ("] ");
  }
#if DCM_COMB_LIST
  if(pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0 && !pcSlice->getNoBackPredFlag())
  {
    printf ("[LC ");
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(REF_PIC_LIST_C); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC((RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIndex), pcSlice->getRefIdxFromIdxOfLC(iRefIndex)));
    }
    printf ("] ");
  }
#endif

  fflush(stdout);
}

#if DCM_DECODING_REFRESH
/** Function for deciding the nal_unit_type.
 * \param uiPOCCurr POC of the current picture
 * \returns the nal_unit type of the picture
 * This function checks the configuration and returns the appropriate nal_unit_type for the picture.
 */
NalUnitType TEncGOP::getNalUnitType(UInt uiPOCCurr)
{
  if (uiPOCCurr == 0)
  {
    return NAL_UNIT_CODED_SLICE_IDR;
  }
  if (uiPOCCurr % m_pcCfg->getIntraPeriod() == 0)
  {
    if (m_pcCfg->getDecodingRefreshType() == 1)
    {
      return NAL_UNIT_CODED_SLICE_CDR;
    }
    else if (m_pcCfg->getDecodingRefreshType() == 2)
    {
      return NAL_UNIT_CODED_SLICE_IDR;
    }
  }
  return NAL_UNIT_CODED_SLICE;
}
#endif

