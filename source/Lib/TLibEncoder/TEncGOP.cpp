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

/** \file     TEncGOP.cpp
    \brief    GOP encoder class
*/

#include <list>
#include <algorithm>

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"
#include "libmd5/MD5.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/NAL.h"
#include "NALwrite.h"
#include <time.h>
#include <math.h>

using namespace std;
//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================
#if CODE_POCLSBLT_FIXEDLEN
Int getLSB(Int poc, Int maxLSB)
{
  if (poc >= 0)
  {
    return poc % maxLSB;
  }
  else
  {
    return (maxLSB - ((-poc) % maxLSB)) % maxLSB;
  }
}
#endif
TEncGOP::TEncGOP()
{
  m_iLastIDR            = 0;
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
  
  m_bRefreshPending     = 0;
  m_pocCRA            = 0;

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
  m_uiStoredStartCUAddrForEncodingSlice = new UInt [uiNumCUsInFrame*(1<<(g_uiMaxCUDepth<<1))+1];
  m_uiStoredStartCUAddrForEncodingDependentSlice = new UInt [uiNumCUsInFrame*(1<<(g_uiMaxCUDepth<<1))+1];
  m_bLongtermTestPictureHasBeenCoded = 0;
  m_bLongtermTestPictureHasBeenCoded2 = 0;
}

Void  TEncGOP::destroy()
{
  delete [] m_uiStoredStartCUAddrForEncodingSlice; m_uiStoredStartCUAddrForEncodingSlice = NULL;
  delete [] m_uiStoredStartCUAddrForEncodingDependentSlice; m_uiStoredStartCUAddrForEncodingDependentSlice = NULL;
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
  m_pcSAO                = pcTEncTop->getSAO();
  m_pcRateCtrl           = pcTEncTop->getRateCtrl();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================
Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsInGOP)
{
  TComPic*        pcPic;
  TComPicYuv*     pcPicYuvRecOut;
  TComSlice*      pcSlice;
  TComOutputBitstream  *pcBitstreamRedirect;
  pcBitstreamRedirect = new TComOutputBitstream;
  AccessUnit::iterator  itLocationToPushSliceHeaderNALU; // used to store location where NALU containing slice header is to be inserted
  UInt                  uiOneBitstreamPerSliceLength = 0;
  TEncSbac* pcSbacCoders = NULL;
  TComOutputBitstream* pcSubstreamsOut = NULL;

  xInitGOP( iPOCLast, iNumPicRcvd, rcListPic, rcListPicYuvRecOut );
  
  m_iNumPicCoded = 0;

  for ( Int iGOPid=0; iGOPid < m_iGopSize; iGOPid++ )
    {
      UInt uiColDir = 1;
      //-- For time output for each slice
      long iBeforeTime = clock();
      
      //select uiColDir
      Int iCloseLeft=1, iCloseRight=-1;
      for(Int i = 0; i<m_pcCfg->getGOPEntry(iGOPid).m_numRefPics; i++) 
      {
        Int iRef = m_pcCfg->getGOPEntry(iGOPid).m_referencePics[i];
        if(iRef>0&&(iRef<iCloseRight||iCloseRight==-1))
        {
          iCloseRight=iRef;
        }
        else if(iRef<0&&(iRef>iCloseLeft||iCloseLeft==1))
        {
          iCloseLeft=iRef;
        }
      }
      if(iCloseRight>-1)
      {
        iCloseRight=iCloseRight+m_pcCfg->getGOPEntry(iGOPid).m_POC-1;
      }
      if(iCloseLeft<1) 
      {
        iCloseLeft=iCloseLeft+m_pcCfg->getGOPEntry(iGOPid).m_POC-1;
        while(iCloseLeft<0)
        {
          iCloseLeft+=m_iGopSize;
        }
      }
      Int iLeftQP=0, iRightQP=0;
      for(Int i=0; i<m_iGopSize; i++)
      {
        if(m_pcCfg->getGOPEntry(i).m_POC==(iCloseLeft%m_iGopSize)+1)
        {
          iLeftQP= m_pcCfg->getGOPEntry(i).m_QPOffset;
        }
        if (m_pcCfg->getGOPEntry(i).m_POC==(iCloseRight%m_iGopSize)+1)
        {
          iRightQP=m_pcCfg->getGOPEntry(i).m_QPOffset;
        }
      }
      if(iCloseRight>-1&&iRightQP<iLeftQP)
      {
        uiColDir=0;
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// Initial to start encoding
      UInt uiPOCCurr = iPOCLast -iNumPicRcvd+ m_pcCfg->getGOPEntry(iGOPid).m_POC;
      Int iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_POC;
      if(iPOCLast == 0)
      {
        uiPOCCurr=0;
        iTimeOffset = 1;
      }
      if(uiPOCCurr>=m_pcCfg->getFrameToBeEncoded())
      {
        continue;
      }
        
      if(getNalUnitType(uiPOCCurr) == NAL_UNIT_CODED_SLICE_IDR)
      {
        m_iLastIDR = uiPOCCurr;
      }        
      /* start a new access unit: create an entry in the list of output
       * access units */
      accessUnitsInGOP.push_back(AccessUnit());
      AccessUnit& accessUnit = accessUnitsInGOP.back();
      xGetBuffer( rcListPic, rcListPicYuvRecOut, iNumPicRcvd, iTimeOffset, pcPic, pcPicYuvRecOut, uiPOCCurr );
      
      //  Slice data initialization
      pcPic->clearSliceBuffer();
      assert(pcPic->getNumAllocatedSlice() == 1);
      m_pcSliceEncoder->setSliceIdx(0);
      pcPic->setCurrSliceIdx(0);

      std::vector<TComAPS>& vAPS = m_pcEncTop->getAPS();
      m_pcSliceEncoder->initEncSlice ( pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iGOPid, pcSlice, m_pcEncTop->getSPS(), m_pcEncTop->getPPS() );
      pcSlice->setLastIDR(m_iLastIDR);
      pcSlice->setSliceIdx(0);
#if H0391_LF_ACROSS_SLICE_BOUNDARY_CONTROL
      //set default slice level flag to the same as SPS level flag
      pcSlice->setLFCrossSliceBoundaryFlag(  pcSlice->getSPS()->getLFCrossSliceBoundaryFlag()  );
#endif
#if !REMOVE_INTER_4X4
      m_pcEncTop->getSPS()->setDisInter4x4(m_pcEncTop->getDisInter4x4());
#endif
      pcSlice->setScalingList ( m_pcEncTop->getScalingList()  );
      if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_OFF)
      {
        m_pcEncTop->getTrQuant()->setFlatScalingList();
        m_pcEncTop->getTrQuant()->setUseScalingList(false);
#if SCALING_LIST_HL_SYNTAX
        m_pcEncTop->getSPS()->setScalingListPresentFlag(false);
        m_pcEncTop->getPPS()->setScalingListPresentFlag(false);
#endif
      }
      else if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_DEFAULT)
      {
        pcSlice->setDefaultScalingList ();
#if SCALING_LIST_HL_SYNTAX
        m_pcEncTop->getSPS()->setScalingListPresentFlag(false);
        m_pcEncTop->getPPS()->setScalingListPresentFlag(false);
#else
        pcSlice->getScalingList()->setScalingListPresentFlag(true);
#endif
        m_pcEncTop->getTrQuant()->setScalingList(pcSlice->getScalingList());
        m_pcEncTop->getTrQuant()->setUseScalingList(true);
      }
      else if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_FILE_READ)
      {
        if(pcSlice->getScalingList()->xParseScalingList(m_pcCfg->getScalingListFile()))
        {
          pcSlice->setDefaultScalingList ();
        }
        pcSlice->getScalingList()->checkDcOfMatrix();
#if SCALING_LIST_HL_SYNTAX
        m_pcEncTop->getSPS()->setScalingListPresentFlag(pcSlice->checkDefaultScalingList());
        m_pcEncTop->getPPS()->setScalingListPresentFlag(false);
#else
        pcSlice->getScalingList()->setScalingListPresentFlag(pcSlice->checkDefaultScalingList());
#endif
        m_pcEncTop->getTrQuant()->setScalingList(pcSlice->getScalingList());
        m_pcEncTop->getTrQuant()->setUseScalingList(true);
      }
      else
      {
        printf("error : ScalingList == %d no support\n",m_pcEncTop->getUseScalingListId());
        assert(0);
      }

      if(pcSlice->getSliceType()==B_SLICE&&m_pcCfg->getGOPEntry(iGOPid).m_sliceType=='P')
      {
        pcSlice->setSliceType(P_SLICE);
      }
      // Set the nal unit type
      pcSlice->setNalUnitType(getNalUnitType(uiPOCCurr));
      // Do decoding refresh marking if any 
      pcSlice->decodingRefreshMarking(m_pocCRA, m_bRefreshPending, rcListPic);
#if !SLICE_TMVP_ENABLE
      if ( !pcSlice->getPPS()->getEnableTMVPFlag() && pcPic->getTLayer() == 0 )
      {
        pcSlice->decodingMarkingForNoTMVP( rcListPic, pcSlice->getPOC() );
      }
#endif
      m_pcEncTop->selectReferencePictureSet(pcSlice, uiPOCCurr, iGOPid,rcListPic);
      pcSlice->getRPS()->setNumberOfLongtermPictures(0);

      if(pcSlice->checkThatAllRefPicsAreAvailable(rcListPic, pcSlice->getRPS(), false) != 0)
      {
         pcSlice->createExplicitReferencePictureSetFromReference(rcListPic, pcSlice->getRPS());
      }
      pcSlice->applyReferencePictureSet(rcListPic, pcSlice->getRPS());

      if(pcSlice->getTLayer() > 0)
      {
#if TEMPORALNESTINGFLAG_TLA
        if(pcSlice->isTemporalLayerSwitchingPoint(rcListPic, pcSlice->getRPS()) || pcSlice->getSPS()->getTemporalIdNestingFlag())
#else
        if(pcSlice->isTemporalLayerSwitchingPoint(rcListPic, pcSlice->getRPS()))
#endif
        {
          pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_TLA);
        }
      }
#if CODE_POCLSBLT_FIXEDLEN
      arrangeLongtermPicturesInRPS(pcSlice, rcListPic);
#endif
      TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
      refPicListModification->setRefPicListModificationFlagL0(0);
      refPicListModification->setRefPicListModificationFlagL1(0);
      pcSlice->setNumRefIdx(REF_PIC_LIST_0,min(m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive,pcSlice->getRPS()->getNumberOfPictures()));
      pcSlice->setNumRefIdx(REF_PIC_LIST_1,min(m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive,pcSlice->getRPS()->getNumberOfPictures()));

#if ADAPTIVE_QP_SELECTION
      pcSlice->setTrQuant( m_pcEncTop->getTrQuant() );
#endif      

      //  Set reference list
      pcSlice->setRefPicList ( rcListPic );
      
      //  Slice info. refinement
      if ( (pcSlice->getSliceType() == B_SLICE) && (pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0) )
      {
        pcSlice->setSliceType ( P_SLICE );
      }
      
      if (pcSlice->getSliceType() != B_SLICE || !pcSlice->getSPS()->getUseLComb())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
        pcSlice->setRefPicListCombinationFlag(false);
        pcSlice->setRefPicListModificationFlagLC(false);
      }
      else
      {
        pcSlice->setRefPicListCombinationFlag(pcSlice->getSPS()->getUseLComb());
#if !REMOVE_LC
        pcSlice->setRefPicListModificationFlagLC(pcSlice->getSPS()->getLCMod());
#endif
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, pcSlice->getNumRefIdx(REF_PIC_LIST_0));
      }
      
      if (pcSlice->getSliceType() == B_SLICE)
      {
        pcSlice->setColDir(uiColDir);
        Bool bLowDelay = true;
        Int  iCurrPOC  = pcSlice->getPOC();
        Int iRefIdx = 0;

        for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_0) && bLowDelay; iRefIdx++)
        {
          if ( pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx)->getPOC() > iCurrPOC )
          {
            bLowDelay = false;
          }
        }
        for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_1) && bLowDelay; iRefIdx++)
        {
          if ( pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx)->getPOC() > iCurrPOC )
          {
            bLowDelay = false;
          }
        }

        pcSlice->setCheckLDC(bLowDelay);  
      }
      
      uiColDir = 1-uiColDir;
      
      //-------------------------------------------------------------
      pcSlice->setRefPOCList();
      
      pcSlice->setNoBackPredFlag( false );
      if ( pcSlice->getSliceType() == B_SLICE && !pcSlice->getRefPicListCombinationFlag())
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

      if(pcSlice->getNoBackPredFlag())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
      }
      pcSlice->generateCombinedList();
      
#if SLICE_TMVP_ENABLE
      if (m_pcEncTop->getTMVPModeId() == 2)
      {
        if (iGOPid == 0) // first picture in SOP (i.e. forward B)
        {
          pcSlice->setEnableTMVPFlag(0);
        }
        else
        {
          // Note: pcSlice->getColDir() is assumed to be always 1 and getcolRefIdx() is always 0.
          pcSlice->setEnableTMVPFlag(1);
        }
        pcSlice->getSPS()->setTMVPFlagsPresent(1);
      }
      else if (m_pcEncTop->getTMVPModeId() == 1)
      {
        pcSlice->getSPS()->setTMVPFlagsPresent(1);
        pcSlice->setEnableTMVPFlag(1);
      }
      else
      {
        pcSlice->getSPS()->setTMVPFlagsPresent(0);
        pcSlice->setEnableTMVPFlag(0);
      }
#endif
      /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a slice
      //  Slice compression
      if (m_pcCfg->getUseASR())
      {
        m_pcSliceEncoder->setSearchRange(pcSlice);
      }

      Bool bGPBcheck=false;
      if ( pcSlice->getSliceType() == B_SLICE)
      {
        if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
        {
          bGPBcheck=true;
          int i;
          for ( i=0; i < pcSlice->getNumRefIdx(RefPicList( 1 ) ); i++ )
          {
            if ( pcSlice->getRefPOC(RefPicList(1), i) != pcSlice->getRefPOC(RefPicList(0), i) ) 
            {
              bGPBcheck=false;
              break;
            }
          }
        }
      }
      if(bGPBcheck)
      {
        pcSlice->setMvdL1ZeroFlag(true);
      }
      else
      {
        pcSlice->setMvdL1ZeroFlag(false);
      }
      pcPic->getSlice(pcSlice->getSliceIdx())->setMvdL1ZeroFlag(pcSlice->getMvdL1ZeroFlag());

      UInt uiNumSlices = 1;

      UInt uiInternalAddress = pcPic->getNumPartInCU()-4;
      UInt uiExternalAddress = pcPic->getPicSym()->getNumberOfCUsInFrame()-1;
      UInt uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
      UInt uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
      UInt uiWidth = pcSlice->getSPS()->getPicWidthInLumaSamples();
      UInt uiHeight = pcSlice->getSPS()->getPicHeightInLumaSamples();
      while(uiPosX>=uiWidth||uiPosY>=uiHeight) 
      {
        uiInternalAddress--;
        uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
        uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
      }
      uiInternalAddress++;
      if(uiInternalAddress==pcPic->getNumPartInCU()) 
      {
        uiInternalAddress = 0;
        uiExternalAddress++;
      }
      UInt uiRealEndAddress = uiExternalAddress*pcPic->getNumPartInCU()+uiInternalAddress;

    UInt uiCummulativeTileWidth;
    UInt uiCummulativeTileHeight;
    Int  p, j;
    UInt uiEncCUAddr;

#if !TILES_OR_ENTROPY_FIX
    if( pcSlice->getPPS()->getColumnRowInfoPresent() == 1 )    //derive the tile parameters from PPS
    {
#endif
    //set NumColumnsMinus1 and NumRowsMinus1
    pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getPPS()->getNumColumnsMinus1() );
    pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getPPS()->getNumRowsMinus1() );

    //create the TComTileArray
    pcPic->getPicSym()->xCreateTComTileArray();

    if( pcSlice->getPPS()->getUniformSpacingIdr() == 1 )
    {
      //set the width for each tile
      for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
      {
        for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1()+1; p++)
        {
          pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->
            setTileWidth( (p+1)*pcPic->getPicSym()->getFrameWidthInCU()/(pcPic->getPicSym()->getNumColumnsMinus1()+1) 
            - (p*pcPic->getPicSym()->getFrameWidthInCU())/(pcPic->getPicSym()->getNumColumnsMinus1()+1) );
        }
      }

      //set the height for each tile
      for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
      {
        for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1()+1; p++)
        {
          pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->
            setTileHeight( (p+1)*pcPic->getPicSym()->getFrameHeightInCU()/(pcPic->getPicSym()->getNumRowsMinus1()+1) 
            - (p*pcPic->getPicSym()->getFrameHeightInCU())/(pcPic->getPicSym()->getNumRowsMinus1()+1) );   
        }
      }
    }
    else
    {
      //set the width for each tile
      for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
      {
        uiCummulativeTileWidth = 0;
        for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1(); p++)
        {
          pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->setTileWidth( pcSlice->getPPS()->getColumnWidth(p) );
          uiCummulativeTileWidth += pcSlice->getPPS()->getColumnWidth(p);
        }
        pcPic->getPicSym()->getTComTile(j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
      }

      //set the height for each tile
      for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
      {
        uiCummulativeTileHeight = 0;
        for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1(); p++)
        {
          pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->setTileHeight( pcSlice->getPPS()->getRowHeight(p) );
          uiCummulativeTileHeight += pcSlice->getPPS()->getRowHeight(p);
        }
        pcPic->getPicSym()->getTComTile(p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
      }
    }
#if !TILES_OR_ENTROPY_FIX
    }
    else //derive the tile parameters from SPS
    {
      //set NumColumnsMins1 and NumRowsMinus1
      pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getSPS()->getNumColumnsMinus1() );
      pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getSPS()->getNumRowsMinus1() );

      //create the TComTileArray
      pcPic->getPicSym()->xCreateTComTileArray();

      if( pcSlice->getSPS()->getUniformSpacingIdr() == 1 )
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->
              setTileWidth( (p+1)*pcPic->getPicSym()->getFrameWidthInCU()/(pcPic->getPicSym()->getNumColumnsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameWidthInCU())/(pcPic->getPicSym()->getNumColumnsMinus1()+1) );
          }
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->
              setTileHeight( (p+1)*pcPic->getPicSym()->getFrameHeightInCU()/(pcPic->getPicSym()->getNumRowsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameHeightInCU())/(pcPic->getPicSym()->getNumRowsMinus1()+1) );   
          }
        }
      }

      else
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          uiCummulativeTileWidth = 0;
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->setTileWidth( pcSlice->getSPS()->getColumnWidth(p) );
            uiCummulativeTileWidth += pcSlice->getSPS()->getColumnWidth(p);
          }
          pcPic->getPicSym()->getTComTile(j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          uiCummulativeTileHeight = 0;
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->setTileHeight( pcSlice->getSPS()->getRowHeight(p) );
            uiCummulativeTileHeight += pcSlice->getSPS()->getRowHeight(p);
          }
          pcPic->getPicSym()->getTComTile(p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
        }
      }
    }
#endif
    //intialize each tile of the current picture
    pcPic->getPicSym()->xInitTiles();

    // Allocate some coders, now we know how many tiles there are.
    Int iNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
    
    //generate the Coding Order Map and Inverse Coding Order Map
    for(p=0, uiEncCUAddr=0; p<pcPic->getPicSym()->getNumberOfCUsInFrame(); p++, uiEncCUAddr = pcPic->getPicSym()->xCalculateNxtCUAddr(uiEncCUAddr))
    {
      pcPic->getPicSym()->setCUOrderMap(p, uiEncCUAddr);
      pcPic->getPicSym()->setInverseCUOrderMap(uiEncCUAddr, p);
    }
    pcPic->getPicSym()->setCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());    
    pcPic->getPicSym()->setInverseCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());

    // Allocate some coders, now we know how many tiles there are.
    m_pcEncTop->createWPPCoders(iNumSubstreams);
    pcSbacCoders = m_pcEncTop->getSbacCoders();
    pcSubstreamsOut = new TComOutputBitstream[iNumSubstreams];

      UInt uiStartCUAddrSliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrSlice    = 0; // used to keep track of current slice's starting CU addr.
      pcSlice->setSliceCurStartCUAddr( uiStartCUAddrSlice ); // Setting "start CU addr" for current slice
      memset(m_uiStoredStartCUAddrForEncodingSlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()*pcPic->getNumPartInCU()+1));

      UInt uiStartCUAddrDependentSliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEntropyEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrDependentSlice    = 0; // used to keep track of current Dependent slice's starting CU addr.
      pcSlice->setDependentSliceCurStartCUAddr( uiStartCUAddrDependentSlice ); // Setting "start CU addr" for current Dependent slice
      
      memset(m_uiStoredStartCUAddrForEncodingDependentSlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()*pcPic->getNumPartInCU()+1));
      UInt uiNextCUAddr = 0;
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = uiNextCUAddr;
      m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx++]  = uiNextCUAddr;
#if DEPENDENT_SLICES
      pcPic->setCurrDepSliceIdx( 0 );
#endif

      while(uiNextCUAddr<uiRealEndAddress) // determine slice boundaries
      {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextDependentSlice( false );
        assert(pcPic->getNumAllocatedSlice() == uiStartCUAddrSliceIdx);
        m_pcSliceEncoder->precompressSlice( pcPic );
        m_pcSliceEncoder->compressSlice   ( pcPic );

        Bool bNoBinBitConstraintViolated = (!pcSlice->isNextSlice() && !pcSlice->isNextDependentSlice());
        if (pcSlice->isNextSlice() || (bNoBinBitConstraintViolated && m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE))
        {
#if DEPENDENT_SLICES
          pcPic->setCurrDepSliceIdx( 0 );
#endif
          uiStartCUAddrSlice                                              = pcSlice->getSliceCurEndCUAddr();
          // Reconstruction slice
          m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]  = uiStartCUAddrSlice;
          // Dependent slice
          if (uiStartCUAddrDependentSliceIdx>0 && m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx-1] != uiStartCUAddrSlice)
          {
            m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx++]  = uiStartCUAddrSlice;
          }
          
          if (uiStartCUAddrSlice < uiRealEndAddress)
          {
            pcPic->allocateNewSlice();          
            pcPic->setCurrSliceIdx                  ( uiStartCUAddrSliceIdx-1 );
            m_pcSliceEncoder->setSliceIdx           ( uiStartCUAddrSliceIdx-1 );
            pcSlice = pcPic->getSlice               ( uiStartCUAddrSliceIdx-1 );
            pcSlice->copySliceInfo                  ( pcPic->getSlice(0)      );
            pcSlice->setSliceIdx                    ( uiStartCUAddrSliceIdx-1 );
            pcSlice->setSliceCurStartCUAddr         ( uiStartCUAddrSlice      );
            pcSlice->setDependentSliceCurStartCUAddr  ( uiStartCUAddrSlice      );
            pcSlice->setSliceBits(0);
            uiNumSlices ++;
          }
        }
        else if (pcSlice->isNextDependentSlice() || (bNoBinBitConstraintViolated && m_pcCfg->getDependentSliceMode()==SHARP_FIXED_NUMBER_OF_LCU_IN_DEPENDENT_SLICE))
        {
          uiStartCUAddrDependentSlice                                                     = pcSlice->getDependentSliceCurEndCUAddr();
          m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx++]  = uiStartCUAddrDependentSlice;
          pcSlice->setDependentSliceCurStartCUAddr( uiStartCUAddrDependentSlice );
        }
        else
        {
          uiStartCUAddrSlice                                                            = pcSlice->getSliceCurEndCUAddr();
          uiStartCUAddrDependentSlice                                                     = pcSlice->getDependentSliceCurEndCUAddr();
        }        

        uiNextCUAddr = (uiStartCUAddrSlice > uiStartCUAddrDependentSlice) ? uiStartCUAddrSlice : uiStartCUAddrDependentSlice;
      }
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = pcSlice->getSliceCurEndCUAddr();
      m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx++]  = pcSlice->getSliceCurEndCUAddr();
      
      pcSlice = pcPic->getSlice(0);
      //-- Loop filter
#if !TILES_OR_ENTROPY_FIX
      Bool bLFCrossTileBoundary = (pcSlice->getPPS()->getTileBehaviorControlPresentFlag() == 1)?
                                  (pcSlice->getPPS()->getLFCrossTileBoundaryFlag()):(pcSlice->getPPS()->getSPS()->getLFCrossTileBoundaryFlag());
#else
      Bool bLFCrossTileBoundary = pcSlice->getPPS()->getLFCrossTileBoundaryFlag();
#endif
      m_pcLoopFilter->setCfg(pcSlice->getPPS()->getDeblockingFilterControlPresent(), pcSlice->getLoopFilterDisable(), pcSlice->getLoopFilterBetaOffset(), pcSlice->getLoopFilterTcOffset(), bLFCrossTileBoundary);
      m_pcLoopFilter->loopFilterPic( pcPic );

      pcSlice = pcPic->getSlice(0);
      if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
      {
        Int sliceGranularity = pcSlice->getPPS()->getSliceGranularity();
#if H0391_LF_ACROSS_SLICE_BOUNDARY_CONTROL
        std::vector<Bool> LFCrossSliceBoundaryFlag;
        for(Int s=0; s< uiNumSlices; s++)
        {
          LFCrossSliceBoundaryFlag.push_back(  ((uiNumSlices==1)?true:pcPic->getSlice(s)->getLFCrossSliceBoundaryFlag()) );
        }
        pcPic->createNonDBFilterInfo(m_uiStoredStartCUAddrForEncodingSlice, uiNumSlices, sliceGranularity, &LFCrossSliceBoundaryFlag ,pcPic->getPicSym()->getNumTiles() ,bLFCrossTileBoundary);
#else
        pcPic->createNonDBFilterInfo(m_uiStoredStartCUAddrForEncodingSlice, uiNumSlices, sliceGranularity, pcSlice->getSPS()->getLFCrossSliceBoundaryFlag(),pcPic->getPicSym()->getNumTiles() ,bLFCrossTileBoundary);
#endif
      }


      pcSlice = pcPic->getSlice(0);

      if(pcSlice->getSPS()->getUseSAO())
      {
        m_pcSAO->createPicSaoInfo(pcPic, uiNumSlices);
      }

#if !AHG6_ALF_OPTION2
      AlfParamSet* alfSliceParams = NULL;
      std::vector<AlfCUCtrlInfo>* alfCUCtrlParam = NULL;
#endif
      pcSlice = pcPic->getSlice(0);

      if(pcSlice->getSPS()->getUseALF())
      {
#if AHG6_ALF_OPTION2
        m_pcAdaptiveLoopFilter->createPicAlfInfo(pcPic, uiNumSlices);
#else
        m_pcAdaptiveLoopFilter->createPicAlfInfo(pcPic, uiNumSlices, pcSlice->getSliceQp());
        m_pcAdaptiveLoopFilter->initALFEnc(m_pcCfg->getALFParamInSlice(), m_pcCfg->getALFPicBasedEncode(), uiNumSlices, alfSliceParams, alfCUCtrlParam);
#endif
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
      // Set entropy coder
      m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );

      /* write various header sets. */
      if ( m_bSeqFirst )
      {
#if VPS_INTEGRATION
        OutputNALUnit nalu(NAL_UNIT_VPS, true);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodeVPS(m_pcEncTop->getVPS());
        writeRBSPTrailingBits(nalu.m_Bitstream);
        accessUnit.push_back(new NALUnitEBSP(nalu));
        
        nalu = NALUnit(NAL_UNIT_SPS, true);
#else
        OutputNALUnit nalu(NAL_UNIT_SPS, true);
#endif
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
#if !TILES_OR_ENTROPY_FIX
        pcSlice->getSPS()->setNumSubstreams( pcSlice->getPPS()->getNumSubstreams() );
#endif
        m_pcEntropyCoder->encodeSPS(pcSlice->getSPS());
        writeRBSPTrailingBits(nalu.m_Bitstream);
        accessUnit.push_back(new NALUnitEBSP(nalu));

        nalu = NALUnit(NAL_UNIT_PPS, true);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodePPS(pcSlice->getPPS());
        writeRBSPTrailingBits(nalu.m_Bitstream);
        accessUnit.push_back(new NALUnitEBSP(nalu));

        m_bSeqFirst = false;
      }

      /* use the main bitstream buffer for storing the marshalled picture */
      m_pcEntropyCoder->setBitstream(NULL);

      uiStartCUAddrSliceIdx = 0;
      uiStartCUAddrSlice    = 0; 

      uiStartCUAddrDependentSliceIdx = 0;
      uiStartCUAddrDependentSlice    = 0; 
      uiNextCUAddr                 = 0;
      pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);

#if DBL_HL_SYNTAX
      Int processingState = (pcSlice->getSPS()->getUseALF() || pcSlice->getSPS()->getUseSAO())?(EXECUTE_INLOOPFILTER):(ENCODE_SLICE);
#else
#if SCALING_LIST_HL_SYNTAX
      Int processingState = (pcSlice->getSPS()->getUseALF() || pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseDF())?(EXECUTE_INLOOPFILTER):(ENCODE_SLICE);
#else
      Int processingState = (pcSlice->getSPS()->getUseALF() || pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getScalingListFlag() || pcSlice->getSPS()->getUseDF())?(EXECUTE_INLOOPFILTER):(ENCODE_SLICE);
#endif
#endif

      static Int iCurrAPSIdx = 0;
      Int iCodedAPSIdx = 0;
      TComSlice* pcSliceForAPS = NULL;

      bool skippedSlice=false;
      while (uiNextCUAddr < uiRealEndAddress) // Iterate over all slices
      {
        switch(processingState)
        {
        case ENCODE_SLICE:
          {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextDependentSlice( false );
        if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx])
        {
          pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);
          if(uiStartCUAddrSliceIdx > 0 && pcSlice->getSliceType()!= I_SLICE)
          {
            pcSlice->checkColRefIdx(uiStartCUAddrSliceIdx, pcPic);
          }
          pcPic->setCurrSliceIdx(uiStartCUAddrSliceIdx);
          m_pcSliceEncoder->setSliceIdx(uiStartCUAddrSliceIdx);
          assert(uiStartCUAddrSliceIdx == pcSlice->getSliceIdx());
          // Reconstruction slice
          pcSlice->setSliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setSliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx+1 ] );
          // Dependent slice
          pcSlice->setDependentSliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setDependentSliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx+1 ] );

          pcSlice->setNextSlice       ( true );

          uiStartCUAddrSliceIdx++;
          uiStartCUAddrDependentSliceIdx++;
        } 
        else if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx])
        {
          // Dependent slice
          pcSlice->setDependentSliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setDependentSliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx+1 ] );

          pcSlice->setNextDependentSlice( true );

          uiStartCUAddrDependentSliceIdx++;
        }

      pcSlice->setRPS(pcPic->getSlice(0)->getRPS());
      pcSlice->setRPSidx(pcPic->getSlice(0)->getRPSidx());
        UInt uiDummyStartCUAddr;
        UInt uiDummyBoundingCUAddr;
        m_pcSliceEncoder->xDetermineStartAndBoundingCUAddr(uiDummyStartCUAddr,uiDummyBoundingCUAddr,pcPic,true);

        uiInternalAddress = pcPic->getPicSym()->getPicSCUAddr(pcSlice->getDependentSliceCurEndCUAddr()-1) % pcPic->getNumPartInCU();
        uiExternalAddress = pcPic->getPicSym()->getPicSCUAddr(pcSlice->getDependentSliceCurEndCUAddr()-1) / pcPic->getNumPartInCU();
        uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
        uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
        uiWidth = pcSlice->getSPS()->getPicWidthInLumaSamples();
        uiHeight = pcSlice->getSPS()->getPicHeightInLumaSamples();
        while(uiPosX>=uiWidth||uiPosY>=uiHeight)
        {
          uiInternalAddress--;
          uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
          uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
        }
        uiInternalAddress++;
        if(uiInternalAddress==pcPic->getNumPartInCU())
        {
          uiInternalAddress = 0;
          uiExternalAddress = pcPic->getPicSym()->getCUOrderMap(pcPic->getPicSym()->getInverseCUOrderMap(uiExternalAddress)+1);
        }
        UInt uiEndAddress = pcPic->getPicSym()->getPicSCUEncOrder(uiExternalAddress*pcPic->getNumPartInCU()+uiInternalAddress);
        if(uiEndAddress<=pcSlice->getDependentSliceCurStartCUAddr()) {
          UInt uiBoundingAddrSlice, uiBoundingAddrDependentSlice;
          uiBoundingAddrSlice        = m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx];          
          uiBoundingAddrDependentSlice = m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx];          
          uiNextCUAddr               = min(uiBoundingAddrSlice, uiBoundingAddrDependentSlice);
          if(pcSlice->isNextSlice())
          {
            skippedSlice=true;
          }
          continue;
        }
        if(skippedSlice) 
        {
          pcSlice->setNextSlice       ( true );
          pcSlice->setNextDependentSlice( false );
        }
        skippedSlice=false;
        pcSlice->allocSubstreamSizes( iNumSubstreams );
        for ( UInt ui = 0 ; ui < iNumSubstreams; ui++ )
        {
          pcSubstreamsOut[ui].clear();
        }

        m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
        m_pcEntropyCoder->resetEntropy      ();
        /* start slice NALunit */
        OutputNALUnit nalu( pcSlice->getNalUnitType(), pcSlice->isReferenced(), pcSlice->getTLayer() );
        Bool bDependentSlice = (!pcSlice->isNextSlice());
        if (!bDependentSlice)
        {
          uiOneBitstreamPerSliceLength = 0; // start of a new slice
        }
#if !REMOVE_TILE_MARKERS
        // used while writing slice header
        Int iTransmitLWHeader = (m_pcCfg->getTileMarkerFlag()==0) ? 0 : 1;
        pcSlice->setTileMarkerFlag ( iTransmitLWHeader );
#endif
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodeSliceHeader(pcSlice);
#if !AHG6_ALF_OPTION2
        if(pcSlice->isNextSlice())
        {
          if (pcSlice->getSPS()->getUseALF())
          {
            if(pcSlice->getAlfEnabledFlag())
            {

              if( pcSlice->getSPS()->getUseALFCoefInSlice())
              {
                Int iNumSUinLCU    = 1<< (g_uiMaxCUDepth << 1); 
                Int firstLCUAddr   = pcSlice->getSliceCurStartCUAddr() / iNumSUinLCU;  
                Bool isAcrossSlice = pcSlice->getSPS()->getLFCrossSliceBoundaryFlag();
                m_pcEntropyCoder->encodeAlfParam( &(alfSliceParams[pcSlice->getSliceIdx()]), false, firstLCUAddr, isAcrossSlice);
              }

              if( !pcSlice->getSPS()->getUseALFCoefInSlice())
              {
                AlfCUCtrlInfo& cAlfCUCtrlParam = (*alfCUCtrlParam)[pcSlice->getSliceIdx()];
              if(cAlfCUCtrlParam.cu_control_flag)
              {
                m_pcEntropyCoder->setAlfCtrl( true );
                m_pcEntropyCoder->setMaxAlfCtrlDepth(cAlfCUCtrlParam.alf_max_depth);
                m_pcCavlcCoder->setAlfCtrl(true);
                m_pcCavlcCoder->setMaxAlfCtrlDepth(cAlfCUCtrlParam.alf_max_depth); 
              }
              else
              {
                m_pcEntropyCoder->setAlfCtrl(false);
              }
              m_pcEntropyCoder->encodeAlfCtrlParam(cAlfCUCtrlParam, m_pcAdaptiveLoopFilter->getNumCUsInPic());
            
              }
            }
          }
        }
#endif
#if !REMOVE_TILE_MARKERS
        m_pcEntropyCoder->encodeTileMarkerFlag(pcSlice);
#endif

        // is it needed?
        {
          if (!bDependentSlice)
          {
            pcBitstreamRedirect->writeAlignOne();
          }
          else
          {
          // We've not completed our slice header info yet, do the alignment later.
          }
          m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
          m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
          m_pcEntropyCoder->resetEntropy    ();
          for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams() ; ui++ )
          {
            m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[ui], pcSlice );
            m_pcEntropyCoder->resetEntropy    ();
          }
        }

        if(pcSlice->isNextSlice())
        {
          // set entropy coder for writing
          m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
          {
            for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams() ; ui++ )
            {
              m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[ui], pcSlice );
              m_pcEntropyCoder->resetEntropy    ();
            }
            pcSbacCoders[0].load(m_pcSbacCoder);
            m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[0], pcSlice );  //ALF is written in substream #0 with CABAC coder #0 (see ALF param encoding below)
          }
          m_pcEntropyCoder->resetEntropy    ();
          // File writing
          if (!bDependentSlice)
          {
            m_pcEntropyCoder->setBitstream(pcBitstreamRedirect);
          }
          else
          {
            m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
          }
          // for now, override the TILES_DECODER setting in order to write substreams.
            m_pcEntropyCoder->setBitstream    ( &pcSubstreamsOut[0] );

        }
        pcSlice->setFinalized(true);

          m_pcSbacCoder->load( &pcSbacCoders[0] );

        pcSlice->setTileOffstForMultES( uiOneBitstreamPerSliceLength );
        if (!bDependentSlice)
        {
          pcSlice->setTileLocationCount ( 0 );
          m_pcSliceEncoder->encodeSlice(pcPic, pcBitstreamRedirect, pcSubstreamsOut); // redirect is only used for CAVLC tile position info.
        }
        else
        {
          m_pcSliceEncoder->encodeSlice(pcPic, &nalu.m_Bitstream, pcSubstreamsOut); // nalu.m_Bitstream is only used for CAVLC tile position info.
        }

        {
          // Construct the final bitstream by flushing and concatenating substreams.
          // The final bitstream is either nalu.m_Bitstream or pcBitstreamRedirect;
          UInt* puiSubstreamSizes = pcSlice->getSubstreamSizes();
          UInt uiTotalCodedSize = 0; // for padding calcs.
          UInt uiNumSubstreamsPerTile = iNumSubstreams;
          if (iNumSubstreams > 1)
          {
            uiNumSubstreamsPerTile /= pcPic->getPicSym()->getNumTiles();
          }
          for ( UInt ui = 0 ; ui < iNumSubstreams; ui++ )
          {
            // Flush all substreams -- this includes empty ones.
            // Terminating bit and flush.
            m_pcEntropyCoder->setEntropyCoder   ( &pcSbacCoders[ui], pcSlice );
            m_pcEntropyCoder->setBitstream      (  &pcSubstreamsOut[ui] );
            m_pcEntropyCoder->encodeTerminatingBit( 1 );
            m_pcEntropyCoder->encodeSliceFinish();
            
#if BYTE_ALIGNMENT  
            pcSubstreamsOut[ui].writeByteAlignment();   // Byte-alignment in slice_data() at end of sub-stream
#else
            //!KS: The following writes trailing_bits. Should use proper function call to writeRBSPTrailingBits()
            pcSubstreamsOut[ui].write( 1, 1 ); // stop bit.
            pcSubstreamsOut[ui].writeAlignZero();
#endif
            // Byte alignment is necessary between tiles when tiles are independent.
            uiTotalCodedSize += pcSubstreamsOut[ui].getNumberOfWrittenBits();

            {
              Bool bNextSubstreamInNewTile = ((ui+1) < iNumSubstreams)
                                             && ((ui+1)%uiNumSubstreamsPerTile == 0);
              if (bNextSubstreamInNewTile)
              {
                // byte align.
                while (uiTotalCodedSize&0x7)
                {
                  pcSubstreamsOut[ui].write(0, 1);
                  uiTotalCodedSize++;
                }
              }
#if !EXPLICITLY_SIGNAL_ENTRY_POINTS
              Bool bRecordOffsetNext = m_pcCfg->getTileLocationInSliceHeaderFlag()
                                            && bNextSubstreamInNewTile;
#else
              Bool bRecordOffsetNext = bNextSubstreamInNewTile;
#endif
              if (bRecordOffsetNext)
                pcSlice->setTileLocation(ui/uiNumSubstreamsPerTile, pcSlice->getTileOffstForMultES()+(uiTotalCodedSize>>3));
            }
            if (ui+1 < pcSlice->getPPS()->getNumSubstreams())
              puiSubstreamSizes[ui] = pcSubstreamsOut[ui].getNumberOfWrittenBits();
          }

          // Complete the slice header info.
          m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
          m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
#if !EXPLICITLY_SIGNAL_ENTRY_POINTS
          if (m_pcCfg->getTileLocationInSliceHeaderFlag()==0) 
          {
            pcSlice->setTileLocationCount( 0 );
          }
#endif
          m_pcEntropyCoder->encodeTilesWPPEntryPoint( pcSlice );

          // Substreams...
          TComOutputBitstream *pcOut = pcBitstreamRedirect;
          // xWriteTileLocation will perform byte-alignment...
          {
            if (bDependentSlice)
            {
              // In these cases, padding is necessary here.
              pcOut = &nalu.m_Bitstream;
              pcOut->writeAlignOne();
            }
          }
#if !REMOVE_TILE_MARKERS
          UInt uiAccumulatedLength = 0;
#endif
          for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams(); ui++ )
          {
            pcOut->addSubstream(&pcSubstreamsOut[ui]);

#if !REMOVE_TILE_MARKERS
            // Update tile marker location information
            for (Int uiMrkIdx = 0; uiMrkIdx < pcSubstreamsOut[ui].getTileMarkerLocationCount(); uiMrkIdx++)
            {
              UInt uiBottom = pcOut->getTileMarkerLocationCount();
              pcOut->setTileMarkerLocation      ( uiBottom, uiAccumulatedLength + pcSubstreamsOut[ui].getTileMarkerLocation( uiMrkIdx ) );
              pcOut->setTileMarkerLocationCount ( uiBottom + 1 );
            }
            uiAccumulatedLength = (pcOut->getNumberOfWrittenBits() >> 3);
#endif
          }
        }

        UInt uiBoundingAddrSlice, uiBoundingAddrDependentSlice;
        uiBoundingAddrSlice        = m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx];          
        uiBoundingAddrDependentSlice = m_uiStoredStartCUAddrForEncodingDependentSlice[uiStartCUAddrDependentSliceIdx];          
        uiNextCUAddr               = min(uiBoundingAddrSlice, uiBoundingAddrDependentSlice);
        // If current NALU is the first NALU of slice (containing slice header) and more NALUs exist (due to multiple dependent slices) then buffer it.
        // If current NALU is the last NALU of slice and a NALU was buffered, then (a) Write current NALU (b) Update an write buffered NALU at approproate location in NALU list.
        Bool bNALUAlignedWrittenToList    = false; // used to ensure current NALU is not written more than once to the NALU list.
        xWriteTileLocationToSliceHeader(nalu, pcBitstreamRedirect, pcSlice);
        accessUnit.push_back(new NALUnitEBSP(nalu));
        bNALUAlignedWrittenToList = true; 
        uiOneBitstreamPerSliceLength += nalu.m_Bitstream.getNumberOfWrittenBits(); // length of bitstream after byte-alignment

        if (!bNALUAlignedWrittenToList)
        {
        {
          nalu.m_Bitstream.writeAlignZero();
        }
        accessUnit.push_back(new NALUnitEBSP(nalu));
        uiOneBitstreamPerSliceLength += nalu.m_Bitstream.getNumberOfWrittenBits() + 24; // length of bitstream after byte-alignment + 3 byte startcode 0x000001
        }


        processingState = ENCODE_SLICE;
          }
          break;
        case EXECUTE_INLOOPFILTER:
          {
            TComAPS cAPS;
            allocAPS(&cAPS, pcSlice->getSPS());
#if !SAO_REMOVE_APS // APS syntax
            cAPS.setSaoInterleavingFlag(m_pcCfg->getSaoInterleavingFlag());
#endif
            // set entropy coder for RD
#if SAO_OFFSET_MAG_SIGN_SPLIT
            m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
#else
            m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
#endif
            if ( pcSlice->getSPS()->getUseSAO() )
            {
              m_pcEntropyCoder->resetEntropy();
              m_pcEntropyCoder->setBitstream( m_pcBitCounter );
#if SAO_RDO_FIX
              m_pcSAO->startSaoEnc(pcPic, m_pcEntropyCoder, m_pcEncTop->getRDSbacCoder(), m_pcEncTop->getRDGoOnSbacCoder());
#else
              m_pcSAO->startSaoEnc(pcPic, m_pcEntropyCoder, m_pcEncTop->getRDSbacCoder(), NULL);
#endif
              SAOParam& cSaoParam = *(cAPS.getSaoParam());

#if SAO_CHROMA_LAMBDA 
#if SAO_ENCODING_CHOICE
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma(), pcPic->getSlice(0)->getDepth());
#else
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma());
#endif
#else
#if ALF_CHROMA_LAMBDA
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambdaLuma());
#else
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambda());
#endif
#endif
              m_pcSAO->endSaoEnc();

              m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
            }
#if SAO_RDO
            m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
#endif
            // adaptive loop filter
            if ( pcSlice->getSPS()->getUseALF())
            {
#if AHG6_ALF_OPTION2
#if ALF_CHROMA_LAMBDA 
              m_pcAdaptiveLoopFilter->ALFProcess(cAPS.getAlfParam(), pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma() );
#else
#if SAO_CHROMA_LAMBDA
              m_pcAdaptiveLoopFilter->ALFProcess(cAPS.getAlfParam(), pcPic->getSlice(0)->getLambdaLuma());
#else
              m_pcAdaptiveLoopFilter->ALFProcess(cAPS.getAlfParam(), pcPic->getSlice(0)->getLambda());
#endif
#endif
#else
              m_pcEntropyCoder->resetEntropy    ();
              m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
              m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder );
              AlfParamSet* pAlfEncParam = (pcSlice->getSPS()->getUseALFCoefInSlice())?( alfSliceParams ):( cAPS.getAlfParam());
#if ALF_CHROMA_LAMBDA 
              m_pcAdaptiveLoopFilter->ALFProcess(pAlfEncParam, alfCUCtrlParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma() );
#else
#if SAO_CHROMA_LAMBDA
              m_pcAdaptiveLoopFilter->ALFProcess(pAlfEncParam, alfCUCtrlParam, pcPic->getSlice(0)->getLambdaLuma());
#else
              m_pcAdaptiveLoopFilter->ALFProcess(pAlfEncParam, alfCUCtrlParam, pcPic->getSlice(0)->getLambda());
#endif
#endif

              m_pcAdaptiveLoopFilter->endALFEnc();
#endif
              m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
            }
            iCodedAPSIdx = iCurrAPSIdx;  
            pcSliceForAPS = pcSlice;

            assignNewAPS(cAPS, iCodedAPSIdx, vAPS, pcSliceForAPS);
            iCurrAPSIdx = (iCurrAPSIdx +1)%MAX_NUM_SUPPORTED_APS;
            processingState = ENCODE_APS;

            //set APS link to the slices
            for(Int s=0; s< uiNumSlices; s++)
            {
              if (pcSlice->getSPS()->getUseALF())
              {
#if AHG6_ALF_OPTION2
                for(Int compIdx =0; compIdx< 3; compIdx++)
                {
                  pcPic->getSlice(s)->setAlfEnabledFlag( cAPS.getAlfEnabled(compIdx), compIdx);
                }
#else
                pcPic->getSlice(s)->setAlfEnabledFlag(  (pcSlice->getSPS()->getUseALFCoefInSlice())?(alfSliceParams[s].isEnabled[ALF_Y]):(cAPS.getAlfEnabled())   );
#endif
              }
              if (pcSlice->getSPS()->getUseSAO())
              {
                pcPic->getSlice(s)->setSaoEnabledFlag((cAPS.getSaoParam()->bSaoFlag[0]==1)?true:false);
              }
              pcPic->getSlice(s)->setAPS(&(vAPS[iCodedAPSIdx]));
              pcPic->getSlice(s)->setAPSId(iCodedAPSIdx);
            }

            /* The destructor of cAPS that is about to be called will free
             * the resource held by cAPS, which will cause problems since it
             * has been aliased elsewhere.
             *   Hint: never ever write an assignment operator that copies
             *         pointers without the use of smart pointers.
             * The following will clear the saved state before the destructor.
             */
            cAPS = TComAPS();
          }
          break;
        case ENCODE_APS:
          {
            OutputNALUnit nalu(NAL_UNIT_APS, true);
            encodeAPS(&(vAPS[iCodedAPSIdx]), nalu.m_Bitstream, pcSliceForAPS);
            accessUnit.push_back(new NALUnitEBSP(nalu));

            processingState = ENCODE_SLICE;
          }
          break;
        default:
          {
            printf("Not a supported encoding state\n");
            assert(0);
            exit(-1);
          }
        }
      } // end iteration over slices


      if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
      {
        if(pcSlice->getSPS()->getUseSAO())
        {
          m_pcSAO->destroyPicSaoInfo();
        }

        if(pcSlice->getSPS()->getUseALF())
        {
#if !AHG6_ALF_OPTION2
          m_pcAdaptiveLoopFilter->uninitALFEnc(alfSliceParams, alfCUCtrlParam);
#endif
          m_pcAdaptiveLoopFilter->destroyPicAlfInfo();
        }

        pcPic->destroyNonDBFilterInfo();
      }

      pcPic->compressMotion(); 
      
      //-- For time output for each slice
      Double dEncTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;

      const char* digestStr = NULL;
      if (m_pcCfg->getPictureDigestEnabled())
      {
        /* calculate MD5sum for entire reconstructed picture */
        SEIpictureDigest sei_recon_picture_digest;
#if HASH_TYPE
        if(m_pcCfg->getPictureDigestEnabled() == 1)
        {
          sei_recon_picture_digest.method = SEIpictureDigest::MD5;
          calcMD5(*pcPic->getPicYuvRec(), sei_recon_picture_digest.digest);
          digestStr = digestToString(sei_recon_picture_digest.digest, 16);
        }
        else if(m_pcCfg->getPictureDigestEnabled() == 2)
        {
          sei_recon_picture_digest.method = SEIpictureDigest::CRC;
          calcCRC(*pcPic->getPicYuvRec(), sei_recon_picture_digest.digest);
          digestStr = digestToString(sei_recon_picture_digest.digest, 2);
        }
        else if(m_pcCfg->getPictureDigestEnabled() == 3)
        {
          sei_recon_picture_digest.method = SEIpictureDigest::CHECKSUM;
          calcChecksum(*pcPic->getPicYuvRec(), sei_recon_picture_digest.digest);
          digestStr = digestToString(sei_recon_picture_digest.digest, 4);
        }
#else
        sei_recon_picture_digest.method = SEIpictureDigest::MD5;
        calcMD5(*pcPic->getPicYuvRec(), sei_recon_picture_digest.digest);
        digestStr = digestToString(sei_recon_picture_digest.digest);
#endif

#if TEMPORAL_ID_RESTRICTION
        OutputNALUnit nalu(NAL_UNIT_SEI, false, pcSlice->getTLayer());
#else
        OutputNALUnit nalu(NAL_UNIT_SEI, false);
#endif

        /* write the SEI messages */
        m_pcEntropyCoder->setEntropyCoder(m_pcCavlcCoder, pcSlice);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodeSEI(sei_recon_picture_digest);
        writeRBSPTrailingBits(nalu.m_Bitstream);

        /* insert the SEI message NALUnit before any Slice NALUnits */
        AccessUnit::iterator it = find_if(accessUnit.begin(), accessUnit.end(), mem_fun(&NALUnit::isSlice));
        accessUnit.insert(it, new NALUnitEBSP(nalu));
      }

      xCalculateAddPSNR( pcPic, pcPic->getPicYuvRec(), accessUnit, dEncTime );

#if HASH_TYPE
      if (digestStr)
      {
        if(m_pcCfg->getPictureDigestEnabled() == 1)
        {
          printf(" [MD5:%s]", digestStr);
        }
        else if(m_pcCfg->getPictureDigestEnabled() == 2)
        {
          printf(" [CRC:%s]", digestStr);
        }
        else if(m_pcCfg->getPictureDigestEnabled() == 3)
        {
          printf(" [Checksum:%s]", digestStr);
        }
      }
#else
      if (digestStr)
        printf(" [MD5:%s]", digestStr);
#endif
      if(m_pcCfg->getUseRateCtrl())
      {
        unsigned  frameBits = m_vRVM_RP[m_vRVM_RP.size()-1];
        m_pcRateCtrl->updataRCFrameStatus((Int)frameBits, pcSlice->getSliceType());
      }

#if FIXED_ROUNDING_FRAME_MEMORY
      /* TODO: this should happen after copyToPic(pcPicYuvRecOut) */
      pcPic->getPicYuvRec()->xFixedRoundingPic();
#endif
      pcPic->getPicYuvRec()->copyToPic(pcPicYuvRecOut);
      
      pcPic->setReconMark   ( true );
#if !SLICE_TMVP_ENABLE
      pcPic->setUsedForTMVP ( true );
#endif
      m_bFirst = false;
      m_iNumPicCoded++;

      /* logging: insert a newline at end of picture period */
      printf("\n");
      fflush(stdout);

      delete[] pcSubstreamsOut;
  }
  if(m_pcCfg->getUseRateCtrl())
  {
    m_pcRateCtrl->updateRCGOPStatus();
  }
  delete pcBitstreamRedirect;

  assert ( m_iNumPicCoded == iNumPicRcvd );
}

/** Memory allocation for APS
  * \param [out] pAPS APS pointer
  * \param [in] pSPS SPS pointer
  */
Void TEncGOP::allocAPS (TComAPS* pAPS, TComSPS* pSPS)
{
  if(pSPS->getUseSAO())
  {
    pAPS->createSaoParam();
    m_pcSAO->allocSaoParam(pAPS->getSaoParam());
  }
#if AHG6_ALF_OPTION2
  pAPS->createAlfParam();
#else
  if(pSPS->getUseALF())
  {
    pAPS->createAlfParam();
    //alf Enabled flag in APS is false after pAPS->createAlfParam();
    if(!pSPS->getUseALFCoefInSlice())
    {
      pAPS->getAlfParam()->create(m_pcAdaptiveLoopFilter->getNumLCUInPicWidth(), m_pcAdaptiveLoopFilter->getNumLCUInPicHeight(), m_pcAdaptiveLoopFilter->getNumCUsInPic());
      pAPS->getAlfParam()->createALFParam();
    }
  }
#endif
}

/** Memory deallocation for APS
  * \param [out] pAPS APS pointer
  * \param [in] pSPS SPS pointer
  */
Void TEncGOP::freeAPS (TComAPS* pAPS, TComSPS* pSPS)
{
  if(pSPS->getUseSAO())
  {
    if(pAPS->getSaoParam() != NULL)
    {
      m_pcSAO->freeSaoParam(pAPS->getSaoParam());
      pAPS->destroySaoParam();

    }
  }
#if AHG6_ALF_OPTION2
  pAPS->destroyAlfParam();
#else
  if(pSPS->getUseALF())
  {
    if(pAPS->getAlfParam() != NULL)
    {
      if(!pSPS->getUseALFCoefInSlice())
      {
        pAPS->getAlfParam()->releaseALFParam();
      }
      pAPS->destroyAlfParam();
    }
  }
#endif
}

/** Assign APS object into APS container according to APS ID
  * \param [in] cAPS APS object
  * \param [in] apsID APS ID
  * \param [in,out] vAPS APS container
  * \param [in] pcSlice pointer to slice
  */
Void TEncGOP::assignNewAPS(TComAPS& cAPS, Int apsID, std::vector<TComAPS>& vAPS, TComSlice* pcSlice)
{

  cAPS.setAPSID(apsID);
#if !SCALING_LIST_HL_SYNTAX
  if(pcSlice->getPOC() == 0)
  {
    cAPS.setScalingListEnabled(pcSlice->getSPS()->getScalingListFlag());
  }
  else
  {
    cAPS.setScalingListEnabled(false);
  }
#endif
#if !SAO_REMOVE_APS // APS syntax
  cAPS.setSaoEnabled(pcSlice->getSPS()->getUseSAO() ? (cAPS.getSaoParam()->bSaoFlag[0] ):(false));
#endif
#if !AHG6_ALF_OPTION2
  cAPS.setAlfEnabled(pcSlice->getSPS()->getUseALF() ? (cAPS.getAlfParam()->isEnabled[0]):(false));
#endif
#if !DBL_HL_SYNTAX
  cAPS.setLoopFilterOffsetInAPS(m_pcCfg->getLoopFilterOffsetInAPS());
  cAPS.setLoopFilterDisable(m_pcCfg->getLoopFilterDisable());
  cAPS.setLoopFilterBetaOffset(m_pcCfg->getLoopFilterBetaOffset());
  cAPS.setLoopFilterTcOffset(m_pcCfg->getLoopFilterTcOffset());
#endif

  //assign new APS into APS container
  Int apsBufSize= (Int)vAPS.size();

  if(apsID >= apsBufSize)
  {
    vAPS.resize(apsID +1);
  }

  freeAPS(&(vAPS[apsID]), pcSlice->getSPS());
  vAPS[apsID] = cAPS;
}


/** encode APS syntax elements
  * \param [in] pcAPS APS pointer
  * \param [in, out] APSbs bitstream
  * \param [in] pointer to slice (just used for entropy coder initialization)
  */
Void TEncGOP::encodeAPS(TComAPS* pcAPS, TComOutputBitstream& APSbs, TComSlice* pcSlice)
{
  m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice);
  m_pcEntropyCoder->resetEntropy      ();
  m_pcEntropyCoder->setBitstream(&APSbs);

  m_pcEntropyCoder->encodeAPSInitInfo(pcAPS);
#if !SCALING_LIST_HL_SYNTAX
  if(pcAPS->getScalingListEnabled())
  {
    m_pcEntropyCoder->encodeScalingList( pcSlice->getScalingList() );
  }
#endif
#if !DBL_HL_SYNTAX
  if(pcAPS->getLoopFilterOffsetInAPS())
  {
    m_pcEntropyCoder->encodeDFParams(pcAPS);
  }
#endif
#if !SAO_REMOVE_APS
  m_pcEntropyCoder->encodeSaoParam(pcAPS);
#endif
#if AHG6_ALF_OPTION2
  for(Int compIdx=0; compIdx < 3; compIdx++)
  {
    m_pcEntropyCoder->encodeAlfParam( (pcAPS->getAlfParam())[compIdx]);
  }
#else
  m_pcEntropyCoder->encodeAPSAlfFlag( pcAPS->getAlfEnabled()?1:0);
  if(pcAPS->getAlfEnabled())
  {
    m_pcEntropyCoder->encodeAlfParam(pcAPS->getAlfParam());
  }
#endif
  m_pcEntropyCoder->encodeApsExtensionFlag();
  //neither SAO and ALF is enabled
  writeRBSPTrailingBits(APSbs);
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

  printf("\nRVM: %.3lf\n" , xCalculateRVM());
}

Void TEncGOP::preLoopFilterPicAll( TComPic* pcPic, UInt64& ruiDist, UInt64& ruiBits )
{
  TComSlice* pcSlice = pcPic->getSlice(pcPic->getCurrSliceIdx());
  Bool bCalcDist = false;
  m_pcLoopFilter->setCfg(pcSlice->getPPS()->getDeblockingFilterControlPresent(), pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterBetaOffset(), m_pcCfg->getLoopFilterTcOffset(), m_pcCfg->getLFCrossTileBoundaryFlag());
  m_pcLoopFilter->loopFilterPic( pcPic );
  
  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcSlice );
  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  pcSlice = pcPic->getSlice(0);
  if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
  {
#if H0391_LF_ACROSS_SLICE_BOUNDARY_CONTROL
    std::vector<Bool> LFCrossSliceBoundaryFlag(1, true);
    UInt sliceStartAddress[2];
    sliceStartAddress[0] = 0;
    sliceStartAddress[1] = pcPic->getNumCUsInFrame()* pcPic->getNumPartInCU();
    pcPic->createNonDBFilterInfo(sliceStartAddress, 1, 0, &LFCrossSliceBoundaryFlag);
#else
    pcPic->createNonDBFilterInfo();
#endif
  }
  
  // Adaptive Loop filter
  if( pcSlice->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->createPicAlfInfo(pcPic);

#if AHG6_ALF_OPTION2
    ALFParam* alfPicParam[3];
    for(Int compIdx=0; compIdx < 3; compIdx++)
    {
      alfPicParam[compIdx] = new ALFParam(compIdx);
    }

#if ALF_CHROMA_LAMBDA 
    m_pcAdaptiveLoopFilter->ALFProcess(alfPicParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma() );
#else
#if SAO_CHROMA_LAMBDA
    m_pcAdaptiveLoopFilter->ALFProcess(alfPicParam, pcPic->getSlice(0)->getLambdaLuma());
#else
    m_pcAdaptiveLoopFilter->ALFProcess(alfPicParam, pcPic->getSlice(0)->getLambda());
#endif
#endif
    for(Int compIdx=0; compIdx < 3; compIdx++)
    {
      delete alfPicParam[compIdx]; alfPicParam[compIdx] = NULL;
    }
#else
    AlfParamSet* alfParamSet;
    std::vector<AlfCUCtrlInfo>* alfCUCtrlParam = NULL;
    alfParamSet= new AlfParamSet;
    alfParamSet->create( m_pcAdaptiveLoopFilter->getNumLCUInPicWidth(), m_pcAdaptiveLoopFilter->getNumLCUInPicHeight(), m_pcAdaptiveLoopFilter->getNumCUsInPic());
    alfParamSet->createALFParam();
    m_pcAdaptiveLoopFilter->initALFEnc(false, true, 1, alfParamSet, alfCUCtrlParam);
    m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder);
    

#if ALF_CHROMA_LAMBDA 
    m_pcAdaptiveLoopFilter->ALFProcess(alfParamSet, NULL, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma() );
#else
#if SAO_CHROMA_LAMBDA
    m_pcAdaptiveLoopFilter->ALFProcess(alfParamSet, NULL, pcPic->getSlice(0)->getLambdaLuma());
#else
    m_pcAdaptiveLoopFilter->ALFProcess(alfParamSet, NULL, pcPic->getSlice(0)->getLambda());
#endif
#endif

    m_pcAdaptiveLoopFilter->endALFEnc();

    alfParamSet->releaseALFParam();
    delete alfParamSet;
    delete alfCUCtrlParam;
#endif
    m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
    m_pcAdaptiveLoopFilter->destroyPicAlfInfo();
  }
  if( pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
  {
    pcPic->destroyNonDBFilterInfo();
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
  //  Exception for the first frame
  if ( iPOCLast == 0 )
  {
    m_iGopSize    = 1;
  }
  else
    m_iGopSize    = m_pcCfg->getGOPSize();
  
  assert (m_iGopSize > 0); 

  return;
}

Void TEncGOP::xGetBuffer( TComList<TComPic*>&       rcListPic,
                         TComList<TComPicYuv*>&    rcListPicYuvRecOut,
                         Int                       iNumPicRcvd,
                         Int                       iTimeOffset,
                         TComPic*&                 rpcPic,
                         TComPicYuv*&              rpcPicYuvRecOut,
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

#if VERBOSE_RATE
static const char* nalUnitTypeToString(NalUnitType type)
{
  switch (type)
  {
#if NEW_NAL_UNIT_TYPES
    case NAL_UNIT_CODED_SLICE: return "SLICE";
    case NAL_UNIT_CODED_SLICE_TFD: return "TFD";
    case NAL_UNIT_CODED_SLICE_TLA: return "TLA";
    case NAL_UNIT_CODED_SLICE_CRA: return "CRA";
    case NAL_UNIT_CODED_SLICE_CRANT: return "CRANT";
    case NAL_UNIT_CODED_SLICE_BLA: return "BLA";
    case NAL_UNIT_CODED_SLICE_BLANT: return "BLANT";
    case NAL_UNIT_CODED_SLICE_IDR: return "IDR";
    case NAL_UNIT_VPS: return "VPS";
    case NAL_UNIT_SPS: return "SPS";
    case NAL_UNIT_PPS: return "PPS";
    case NAL_UNIT_APS: return "APS";
    case NAL_UNIT_ACCESS_UNIT_DELIMITER: return "AUD";
    case NAL_UNIT_FILLER_DATA: return "FILLER";
    case NAL_UNIT_SEI: return "SEI";
    default: return "UNK";
#else
  case NAL_UNIT_CODED_SLICE: return "SLICE";
  case NAL_UNIT_CODED_SLICE_CRA: return "CRA";
  case NAL_UNIT_CODED_SLICE_TLA: return "TLA";
  case NAL_UNIT_CODED_SLICE_IDR: return "IDR";
  case NAL_UNIT_SEI: return "SEI";
  case NAL_UNIT_SPS: return "SPS";
  case NAL_UNIT_PPS: return "PPS";
  case NAL_UNIT_FILLER_DATA: return "FILLER";
  default: return "UNK";
#endif
  }
}
#endif

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, const AccessUnit& accessUnit, Double dEncTime )
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

  /* calculate the size of the access unit, excluding:
   *  - any AnnexB contributions (start_code_prefix, zero_byte, etc.,)
   *  - SEI NAL units
   */
  unsigned numRBSPBytes = 0;
  for (AccessUnit::const_iterator it = accessUnit.begin(); it != accessUnit.end(); it++)
  {
    unsigned numRBSPBytes_nal = unsigned((*it)->m_nalUnitData.str().size());
#if VERBOSE_RATE
    printf("*** %6s numBytesInNALunit: %u\n", nalUnitTypeToString((*it)->m_nalUnitType), numRBSPBytes_nal);
#endif
    if ((*it)->m_nalUnitType != NAL_UNIT_SEI)
      numRBSPBytes += numRBSPBytes_nal;
  }

  unsigned uibits = numRBSPBytes * 8;
  m_vRVM_RP.push_back( uibits );

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

  Char c = (pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B');
  if (!pcSlice->isReferenced()) c += 32;

#if ADAPTIVE_QP_SELECTION
  printf("POC %4d TId: %1d ( %c-SLICE, nQP %d QP %d ) %10d bits",
         pcSlice->getPOC(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQpBase(),
         pcSlice->getSliceQp(),
         uibits );
#else
  printf("POC %4d TId: %1d ( %c-SLICE, QP %d ) %10d bits",
         pcSlice->getPOC()-pcSlice->getLastIDR(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQp(),
         uibits );
#endif

  printf(" [Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]", dYPSNR, dUPSNR, dVPSNR );
  printf(" [ET %5.0f ]", dEncTime );
  
  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf(" [L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex)-pcSlice->getLastIDR());
    }
    printf("]");
  }
#if !REMOVE_LC
  if(pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0 && !pcSlice->getNoBackPredFlag())
  {
    printf(" [LC ");
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(REF_PIC_LIST_C); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC((RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIndex), pcSlice->getRefIdxFromIdxOfLC(iRefIndex))-pcSlice->getLastIDR());
    }
    printf("]");
  }
#endif
}

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
      return NAL_UNIT_CODED_SLICE_CRA;
    }
    else if (m_pcCfg->getDecodingRefreshType() == 2)
    {
      return NAL_UNIT_CODED_SLICE_IDR;
    }
  }
#if CRA_BLA_TFD_MODIFICATIONS
  if(m_pocCRA>0)
  {
    if(uiPOCCurr<m_pocCRA)
    {
      // All leading pictures are being marked as TFD pictures here since current encoder uses all 
      // reference pictures while encoding leading pictures. An encoder can ensure that a leading 
      // picture can be still decodable when random accessing to a CRA/CRANT/BLA/BLANT picture by 
      // controlling the reference pictures used for encoding that leading picture. Such a leading 
      // picture need not be marked as a TFD picture.
      return NAL_UNIT_CODED_SLICE_TFD;
    }
  }
#endif
  return NAL_UNIT_CODED_SLICE;
}

Double TEncGOP::xCalculateRVM()
{
  Double dRVM = 0;
  
  if( m_pcCfg->getGOPSize() == 1 && m_pcCfg->getIntraPeriod() != 1 && m_pcCfg->getFrameToBeEncoded() > RVM_VCEGAM10_M * 2 )
  {
    // calculate RVM only for lowdelay configurations
    std::vector<Double> vRL , vB;
    size_t N = m_vRVM_RP.size();
    vRL.resize( N );
    vB.resize( N );
    
    Int i;
    Double dRavg = 0 , dBavg = 0;
    vB[RVM_VCEGAM10_M] = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      vRL[i] = 0;
      for( Int j = i - RVM_VCEGAM10_M ; j <= i + RVM_VCEGAM10_M - 1 ; j++ )
        vRL[i] += m_vRVM_RP[j];
      vRL[i] /= ( 2 * RVM_VCEGAM10_M );
      vB[i] = vB[i-1] + m_vRVM_RP[i] - vRL[i];
      dRavg += m_vRVM_RP[i];
      dBavg += vB[i];
    }
    
    dRavg /= ( N - 2 * RVM_VCEGAM10_M );
    dBavg /= ( N - 2 * RVM_VCEGAM10_M );
    
    double dSigamB = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      Double tmp = vB[i] - dBavg;
      dSigamB += tmp * tmp;
    }
    dSigamB = sqrt( dSigamB / ( N - 2 * RVM_VCEGAM10_M ) );
    
    double f = sqrt( 12.0 * ( RVM_VCEGAM10_M - 1 ) / ( RVM_VCEGAM10_M + 1 ) );
    
    dRVM = dSigamB / dRavg * f;
  }
  
  return( dRVM );
}

/** Determine the difference between consecutive tile sizes (in bytes) and writes it to  bistream rNalu [slice header]
 * \param rpcBitstreamRedirect contains the bitstream to be concatenated to rNalu. rpcBitstreamRedirect contains slice payload. rpcSlice contains tile location information.
 * \returns Updates rNalu to contain concatenated bitstream. rpcBitstreamRedirect is cleared at the end of this function call.
 */
Void TEncGOP::xWriteTileLocationToSliceHeader (OutputNALUnit& rNalu, TComOutputBitstream*& rpcBitstreamRedirect, TComSlice*& rpcSlice)
{
  // Byte-align
#if BYTE_ALIGNMENT
  rNalu.m_Bitstream.writeByteAlignment();   // Slice header byte-alignment
#else
  rNalu.m_Bitstream.writeAlignOne();
#endif

#if !REMOVE_TILE_MARKERS
  // Update tile marker locations
  TComOutputBitstream *pcOut = &rNalu.m_Bitstream;
  UInt uiAccumulatedLength   = pcOut->getNumberOfWrittenBits() >> 3;
  for (Int uiMrkIdx = 0; uiMrkIdx < rpcBitstreamRedirect->getTileMarkerLocationCount(); uiMrkIdx++)
  {
    UInt uiBottom = pcOut->getTileMarkerLocationCount();
    pcOut->setTileMarkerLocation      ( uiBottom, uiAccumulatedLength + rpcBitstreamRedirect->getTileMarkerLocation( uiMrkIdx ) );
    pcOut->setTileMarkerLocationCount ( uiBottom + 1 );
  }
#endif

  // Perform bitstream concatenation
  if (rpcBitstreamRedirect->getNumberOfWrittenBits() > 0)
  {
    UInt uiBitCount  = rpcBitstreamRedirect->getNumberOfWrittenBits();
    if (rpcBitstreamRedirect->getByteStreamLength()>0)
    {
      UChar *pucStart  =  reinterpret_cast<UChar*>(rpcBitstreamRedirect->getByteStream());
      UInt uiWriteByteCount = 0;
      while (uiWriteByteCount < (uiBitCount >> 3) )
      {
        UInt uiBits = (*pucStart);
        rNalu.m_Bitstream.write(uiBits, 8);
        pucStart++;
        uiWriteByteCount++;
      }
    }
    UInt uiBitsHeld = (uiBitCount & 0x07);
    for (UInt uiIdx=0; uiIdx < uiBitsHeld; uiIdx++)
    {
      rNalu.m_Bitstream.write((rpcBitstreamRedirect->getHeldBits() & (1 << (7-uiIdx))) >> (7-uiIdx), 1);
    }          
  }

  m_pcEntropyCoder->setBitstream(&rNalu.m_Bitstream);

  delete rpcBitstreamRedirect;
  rpcBitstreamRedirect = new TComOutputBitstream;
}
#if CODE_POCLSBLT_FIXEDLEN
// Function will arrange the long-term pictures in the decreasing order of poc_lsb_lt, 
// and among the pictures with the same lsb, it arranges them in increasing delta_poc_msb_cycle_lt value
Void TEncGOP::arrangeLongtermPicturesInRPS(TComSlice *pcSlice, TComList<TComPic*>& rcListPic)
{
  TComReferencePictureSet *rps = pcSlice->getRPS();
  if(!rps->getNumberOfLongtermPictures())
  {
    return;
  }

  // Arrange long-term reference pictures in the correct order of LSB and MSB,
  // and assign values for pocLSBLT and MSB present flag
  Int longtermPicsPoc[MAX_NUM_REF_PICS], longtermPicsLSB[MAX_NUM_REF_PICS], indices[MAX_NUM_REF_PICS];
  Bool mSBPresentFlag[MAX_NUM_REF_PICS];
  ::memset(longtermPicsPoc, 0, sizeof(longtermPicsPoc));    // Store POC values of LTRP
  ::memset(longtermPicsLSB, 0, sizeof(longtermPicsLSB));    // Store POC LSB values of LTRP
  ::memset(indices        , 0, sizeof(indices));            // Indices to aid in tracking sorted LTRPs
  ::memset(mSBPresentFlag , 0, sizeof(mSBPresentFlag));     // Indicate if MSB needs to be present

  // Get the long-term reference pictures 
  Int offset = rps->getNumberOfNegativePictures() + rps->getNumberOfPositivePictures();
  Int i, ctr = 0;
  Int maxPicOrderCntLSB = 1 << pcSlice->getSPS()->getBitsForPOC();
  for(i = rps->getNumberOfPictures() - 1; i >= offset; i--, ctr++)
  {
    longtermPicsPoc[ctr] = rps->getPOC(i);                                  // LTRP POC
    longtermPicsLSB[ctr] = getLSB(longtermPicsPoc[ctr], maxPicOrderCntLSB); // LTRP POC LSB
    indices[ctr]      = i; 
  }
  Int numLongPics = rps->getNumberOfLongtermPictures();
  assert(ctr == numLongPics);

  // Arrange LTR pictures in decreasing order of LSB
  for(i = 0; i < numLongPics; i++)
  {
    for(Int j = 0; j < numLongPics - 1; j++)
    {
      if(longtermPicsLSB[j] < longtermPicsLSB[j+1])
      {
        std::swap(longtermPicsPoc[j], longtermPicsPoc[j+1]);
        std::swap(longtermPicsLSB[j], longtermPicsLSB[j+1]);
        std::swap(indices[j]        , indices[j+1]        );
      }
    }
  }
  // Now for those pictures that have the same LSB, arrange them 
  // in increasing MSB cycle, or equivalently decreasing MSB
  for(i = 0; i < numLongPics;)    // i incremented using j
  {
    Int j = i + 1;
    Int pocLSB = longtermPicsLSB[i];
    for(; j < numLongPics; j++)
    {
      if(pocLSB != longtermPicsLSB[j])
      {
        break;
      }
    }
    // Last index upto which lsb equals pocLSB is j - 1 
    // Now sort based on the MSB values
    Int sta, end;
    for(sta = i; sta < j; sta++)
    {
      for(end = i; end < j - 1; end++)
      {
      // longtermPicsMSB = longtermPicsPoc - longtermPicsLSB
        if(longtermPicsPoc[end] - longtermPicsLSB[end] < longtermPicsPoc[end+1] - longtermPicsLSB[end+1])
        {
          std::swap(longtermPicsPoc[end], longtermPicsPoc[end+1]);
          std::swap(longtermPicsLSB[end], longtermPicsLSB[end+1]);
          std::swap(indices[end]        , indices[end+1]        );
        }
      }
    }
    i = j;
  }
  for(i = 0; i < numLongPics; i++)
  {
    // Check if MSB present flag should be enabled.
    // Check if the buffer contains any pictures that have the same LSB.
    TComList<TComPic*>::iterator  iterPic = rcListPic.begin();  
    TComPic*                      pcPic;
    while ( iterPic != rcListPic.end() )
    {
      pcPic = *iterPic;
      if( (getLSB(pcPic->getPOC(), maxPicOrderCntLSB) == longtermPicsLSB[i])   &&     // Same LSB
                                      (pcPic->getSlice(0)->getNalRefFlag())     &&    // Reference picture
                                        (pcPic->getPOC() != longtermPicsPoc[i])    )  // Not the LTRP itself
      {
        mSBPresentFlag[i] = true;
        break;
      }
      iterPic++;      
    }
  }

  // tempArray for usedByCurr flag
  Bool tempArray[MAX_NUM_REF_PICS]; ::memset(tempArray, 0, sizeof(tempArray));
  for(i = 0; i < numLongPics; i++)
  {
    tempArray[i] = rps->getUsed(indices[i]);
  }
  // Now write the final values;
  ctr = 0;
  Int currMSB = 0, currLSB = 0;
  // currPicPoc = currMSB + currLSB
  currLSB = getLSB(pcSlice->getPOC(), maxPicOrderCntLSB);  
  currMSB = pcSlice->getPOC() - currLSB;

  for(i = rps->getNumberOfPictures() - 1; i >= offset; i--, ctr++)
  {
    rps->setPOC                   (i, longtermPicsPoc[ctr]);
    rps->setDeltaPOC              (i, - pcSlice->getPOC() + longtermPicsPoc[ctr]);
    rps->setUsed                  (i, tempArray[ctr]);
    rps->setPocLSBLT              (i, longtermPicsLSB[ctr]);
    rps->setDeltaPocMSBCycleLT    (i, (currMSB - (longtermPicsPoc[ctr] - longtermPicsLSB[ctr])) / maxPicOrderCntLSB);
    rps->setDeltaPocMSBPresentFlag(i, mSBPresentFlag[ctr]);     

    assert(rps->getDeltaPocMSBCycleLT(i) >= 0);   // Non-negative value
  }
}
#endif
//! \}
