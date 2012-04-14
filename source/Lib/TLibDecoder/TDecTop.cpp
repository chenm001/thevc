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

/** \file     TDecTop.cpp
    \brief    decoder class
*/

#include "NALread.h"
#include "TDecTop.h"

//! \ingroup TLibDecoder
//! \{

TDecTop::TDecTop()
: m_SEIs(0)
{
  m_pcPic = 0;
  m_iGopSize      = 0;
  m_bGopSizeSet   = false;
  m_iMaxRefPicNum = 0;
  m_uiValidPS = 0;
#if ENC_DEC_TRACE
  g_hTrace = fopen( "TraceDec.txt", "wb" );
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif
  m_bRefreshPending = 0;
  m_pocCRA = 0;
  m_pocRandomAccess = MAX_INT;          
  m_prevPOC                = MAX_INT;
  m_bFirstSliceInPicture    = true;
  m_bFirstSliceInSequence   = true;
}

TDecTop::~TDecTop()
{
#if ENC_DEC_TRACE
  fclose( g_hTrace );
#endif
}

Void TDecTop::create()
{
  m_cGopDecoder.create();
  m_apcSlicePilot = new TComSlice;
  m_uiSliceIdx = m_uiLastSliceIdx = 0;
}

Void TDecTop::destroy()
{
  m_cGopDecoder.destroy();
  
  delete m_apcSlicePilot;
  m_apcSlicePilot = NULL;
  
  m_cSliceDecoder.destroy();
}

Void TDecTop::init()
{
  // initialize ROM
  initROM();
  m_cGopDecoder.init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cAdaptiveLoopFilter, &m_cSAO);
  m_cSliceDecoder.init( &m_cEntropyDecoder, &m_cCuDecoder );
  m_cEntropyDecoder.init(&m_cPrediction);
}

Void TDecTop::deletePicBuffer ( )
{
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    pcPic->destroy();
    
    delete pcPic;
    pcPic = NULL;
  }
  
  // destroy ALF temporary buffers
  m_cAdaptiveLoopFilter.destroy();

  m_cSAO.destroy();
  
  m_cLoopFilter.        destroy();
  
  // destroy ROM
  destroyROM();
}

Void TDecTop::xUpdateGopSize (TComSlice* pcSlice)
{
  if ( !pcSlice->isIntra() && !m_bGopSizeSet)
  {
    m_iGopSize    = pcSlice->getPOC();
    m_bGopSizeSet = true;
    
    m_cGopDecoder.setGopSize(m_iGopSize);
  }
}

Void TDecTop::xGetNewPicBuffer ( TComSlice* pcSlice, TComPic*& rpcPic )
{
  xUpdateGopSize(pcSlice);
  
#if H0567_DPB_PARAMETERS_PER_TEMPORAL_LAYER
  m_iMaxRefPicNum = pcSlice->getSPS()->getMaxDecPicBuffering(pcSlice->getTLayer())+pcSlice->getSPS()->getNumReorderPics(pcSlice->getTLayer()) + 1; // +1 to have space for the picture currently being decoded
#else
  m_iMaxRefPicNum = pcSlice->getSPS()->getMaxNumberOfReferencePictures()+pcSlice->getSPS()->getNumReorderFrames() + 1; // +1 to have space for the picture currently being decoded
#endif
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic();
    
    rpcPic->create ( pcSlice->getSPS()->getPicWidthInLumaSamples(), pcSlice->getSPS()->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, true);
    m_cListPic.pushBack( rpcPic );
    
    return;
  }
  
  Bool bBufferIsAvailable = false;
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
    rpcPic = *(iterPic++);
    if ( rpcPic->getReconMark() == false && rpcPic->getOutputMark() == false)
    {
      rpcPic->setOutputMark(false);
      bBufferIsAvailable = true;
      break;
    }

    if ( rpcPic->getSlice( 0 )->isReferenced() == false  && rpcPic->getOutputMark() == false)
    {
      rpcPic->setOutputMark(false);
      rpcPic->setReconMark( false );
      rpcPic->getPicYuvRec()->setBorderExtension( false );
      bBufferIsAvailable = true;
      break;
    }
  }
  
  if ( !bBufferIsAvailable )
  {
    //There is no room for this picture, either because of faulty encoder or dropped NAL. Extend the buffer.
    m_iMaxRefPicNum++;
    rpcPic = new TComPic();
    m_cListPic.pushBack( rpcPic );
  }
  rpcPic->destroy();
  rpcPic->create ( pcSlice->getSPS()->getPicWidthInLumaSamples(), pcSlice->getSPS()->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, true);
}

Void TDecTop::executeDeblockAndAlf(UInt& ruiPOC, TComList<TComPic*>*& rpcListPic, Int& iSkipFrame, Int& iPOCLastDisplay)
{
  if (!m_pcPic)
  {
    /* nothing to deblock */
    return;
  }
  
  TComPic*&   pcPic         = m_pcPic;

  // Execute Deblock and ALF only + Cleanup

  m_cGopDecoder.decompressGop(NULL, pcPic, true);

  TComSlice::sortPicList( m_cListPic ); // sorting for application output
  ruiPOC              = pcPic->getSlice(m_uiSliceIdx-1)->getPOC();
  rpcListPic          = &m_cListPic;  
  m_cCuDecoder.destroy();        
  m_bFirstSliceInPicture  = true;

  return;
}

Void TDecTop::xCreateLostPicture(Int iLostPoc) 
{
  printf("\ninserting lost poc : %d\n",iLostPoc);
  TComSlice cFillSlice;
  cFillSlice.setSPS( m_parameterSetManagerDecoder.getFirstSPS() );
  cFillSlice.setPPS( m_parameterSetManagerDecoder.getFirstPPS() );
  cFillSlice.initSlice();
  cFillSlice.initTiles();
  TComPic *cFillPic;
  xGetNewPicBuffer(&cFillSlice,cFillPic);
  cFillPic->getSlice(0)->setSPS( m_parameterSetManagerDecoder.getFirstSPS() );
  cFillPic->getSlice(0)->setPPS( m_parameterSetManagerDecoder.getFirstPPS() );
  cFillPic->getSlice(0)->initSlice();
  cFillPic->getSlice(0)->initTiles();

  
  
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int closestPoc = 1000000;
  while ( iterPic != m_cListPic.end())
  {
    TComPic * rpcPic = *(iterPic++);
    if(abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)<closestPoc&&abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)!=0&&rpcPic->getPicSym()->getSlice(0)->getPOC()!=m_apcSlicePilot->getPOC())
    {
      closestPoc=abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc);
    }
  }
  iterPic = m_cListPic.begin();
  while ( iterPic != m_cListPic.end())
  {
    TComPic *rpcPic = *(iterPic++);
    if(abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)==closestPoc&&rpcPic->getPicSym()->getSlice(0)->getPOC()!=m_apcSlicePilot->getPOC())
    {
      printf("copying picture %d to %d (%d)\n",rpcPic->getPicSym()->getSlice(0)->getPOC() ,iLostPoc,m_apcSlicePilot->getPOC());
      rpcPic->getPicYuvRec()->copyToPic(cFillPic->getPicYuvRec());
      break;
    }
  }
  cFillPic->setCurrSliceIdx(0);
  for(Int i=0; i<cFillPic->getNumCUsInFrame(); i++) 
  {
    cFillPic->getCU(i)->initCU(cFillPic,i);
  }
  cFillPic->getSlice(0)->setReferenced(true);
  cFillPic->getSlice(0)->setPOC(iLostPoc);
  cFillPic->setReconMark(true);
  cFillPic->setOutputMark(true);
  if(m_pocRandomAccess == MAX_INT)
  {
    m_pocRandomAccess = iLostPoc;
  }
}


Void TDecTop::xActivateParameterSets()
{
  m_parameterSetManagerDecoder.applyPrefetchedPS();

  TComPPS *pps = m_parameterSetManagerDecoder.getPPS(m_apcSlicePilot->getPPSId());
  assert (pps != 0);

  TComSPS *sps = m_parameterSetManagerDecoder.getSPS(pps->getSPSId());
  assert (sps != 0);

  m_apcSlicePilot->setPPS(pps);
  m_apcSlicePilot->setSPS(sps);
  pps->setSPS(sps);

  if(sps->getUseSAO() || sps->getUseALF()|| sps->getScalingListFlag() || sps->getUseDF())
  {
    m_apcSlicePilot->setAPS( m_parameterSetManagerDecoder.getAPS(m_apcSlicePilot->getAPSId())  );
  }
  pps->setMinCuDQPSize( sps->getMaxCUWidth() >> ( pps->getMaxCuDQPDepth()) );

  for (Int i = 0; i < sps->getMaxCUDepth() - 1; i++)
  {
    sps->setAMPAcc( i, sps->getUseAMP() );
  }

  for (Int i = sps->getMaxCUDepth() - 1; i < sps->getMaxCUDepth(); i++)
  {
    sps->setAMPAcc( i, 0 );
  }

#if !LCU_SYNTAX_ALF
  // create ALF temporary buffer
  m_cAdaptiveLoopFilter.create( sps->getPicWidthInLumaSamples(), sps->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#endif
  m_cSAO.create( sps->getPicWidthInLumaSamples(), sps->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  m_cLoopFilter.        create( g_uiMaxCUDepth );
}

#if SKIPFRAME_BUGFIX
Bool TDecTop::xDecodeSlice(InputNALUnit &nalu, Int &iSkipFrame, Int iPOCLastDisplay )
#else
Bool TDecTop::xDecodeSlice(InputNALUnit &nalu, Int iSkipFrame, Int iPOCLastDisplay )
#endif
{
  TComPic*&   pcPic         = m_pcPic;
  m_apcSlicePilot->initSlice();

  //!!!KS: DIRTY HACK
  m_apcSlicePilot->setPPSId(0);
  m_apcSlicePilot->setPPS(m_parameterSetManagerDecoder.getPrefetchedPPS(0));
  m_apcSlicePilot->setSPS(m_parameterSetManagerDecoder.getPrefetchedSPS(0));
  m_apcSlicePilot->initTiles();

  if (m_bFirstSliceInPicture)
  {
    m_uiSliceIdx     = 0;
    m_uiLastSliceIdx = 0;
  }
  m_apcSlicePilot->setSliceIdx(m_uiSliceIdx);
  if (!m_bFirstSliceInPicture)
  {
    m_apcSlicePilot->copySliceInfo( pcPic->getPicSym()->getSlice(m_uiSliceIdx-1) );
  }

  m_apcSlicePilot->setNalUnitType(nalu.m_nalUnitType);
#if NAL_REF_FLAG
  m_apcSlicePilot->setReferenced(nalu.m_nalRefFlag);
#else
  m_apcSlicePilot->setReferenced(nalu.m_nalRefIDC != NAL_REF_IDC_PRIORITY_LOWEST);
#endif
  m_apcSlicePilot->setTLayerInfo(nalu.m_temporalId);

  // ALF CU parameters should be part of the slice header -> needs to be fixed 
#if LCU_SYNTAX_ALF
  m_cEntropyDecoder.decodeSliceHeader (m_apcSlicePilot, &m_parameterSetManagerDecoder, m_cGopDecoder.getAlfCuCtrlParam(), m_cGopDecoder.getAlfParamSet());
#else
  m_cEntropyDecoder.decodeSliceHeader (m_apcSlicePilot, &m_parameterSetManagerDecoder, m_cGopDecoder.getAlfCuCtrlParam() );
#endif
  // byte align
  {
    Int numBitsForByteAlignment = nalu.m_Bitstream->getNumBitsUntilByteAligned();
    if ( numBitsForByteAlignment > 0 )
    {
      UInt bitsForByteAlignment;
      nalu.m_Bitstream->read( numBitsForByteAlignment, bitsForByteAlignment );
      assert( bitsForByteAlignment == ( ( 1 << numBitsForByteAlignment ) - 1 ) );
    }
  }

  // exit when a new picture is found
  if (m_apcSlicePilot->isNextSlice() && m_apcSlicePilot->getPOC()!=m_prevPOC && !m_bFirstSliceInSequence)
  {
#if START_DECODING_AT_CRA
    if (m_prevPOC >= m_pocRandomAccess)
    {
      m_prevPOC = m_apcSlicePilot->getPOC();
      return true;
    }
    m_prevPOC = m_apcSlicePilot->getPOC();
#else
    m_prevPOC = m_apcSlicePilot->getPOC();
    return true;
#endif
  }
  // actual decoding starts here
  xActivateParameterSets();
  m_apcSlicePilot->initTiles();

  if (m_apcSlicePilot->isNextSlice()) 
  {
    m_prevPOC = m_apcSlicePilot->getPOC();
  }
  m_bFirstSliceInSequence = false;
  if (m_apcSlicePilot->isNextSlice())
  {
    // Skip pictures due to random access
    if (isRandomAccessSkipPicture(iSkipFrame, iPOCLastDisplay))
    {
      return false;
    }
  }
  //detect lost reference picture and insert copy of earlier frame.
#if START_DECODING_AT_CRA
  while(m_apcSlicePilot->checkThatAllRefPicsAreAvailable(m_cListPic, m_apcSlicePilot->getRPS(), true, m_pocRandomAccess) > 0)
#else
  while(m_apcSlicePilot->checkThatAllRefPicsAreAvailable(m_cListPic, m_apcSlicePilot->getRPS(), true) > 0)
#endif
  {
    xCreateLostPicture(m_apcSlicePilot->checkThatAllRefPicsAreAvailable(m_cListPic, m_apcSlicePilot->getRPS(), false)-1);
  }
  if (m_bFirstSliceInPicture)
  {
    // Buffer initialize for prediction.
    m_cPrediction.initTempBuff();
    m_apcSlicePilot->applyReferencePictureSet(m_cListPic, m_apcSlicePilot->getRPS());
    //  Get a new picture buffer
    xGetNewPicBuffer (m_apcSlicePilot, pcPic);

    /* transfer any SEI messages that have been received to the picture */
    pcPic->setSEIs(m_SEIs);
    m_SEIs = NULL;

    // Recursive structure
    m_cCuDecoder.create ( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight );
    m_cCuDecoder.init   ( &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction );
    m_cTrQuant.init     ( g_uiMaxCUWidth, g_uiMaxCUHeight, m_apcSlicePilot->getSPS()->getMaxTrSize());

    m_cSliceDecoder.create( m_apcSlicePilot, m_apcSlicePilot->getSPS()->getPicWidthInLumaSamples(), m_apcSlicePilot->getSPS()->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  }

  //  Set picture slice pointer
  TComSlice*  pcSlice = m_apcSlicePilot;
  Bool bNextSlice     = pcSlice->isNextSlice();

  UInt uiCummulativeTileWidth;
  UInt uiCummulativeTileHeight;
  UInt i, j, p;

#if !REMOVE_TILE_DEPENDENCE
  //set the TileBoundaryIndependenceIdr
  if(pcSlice->getPPS()->getTileBehaviorControlPresentFlag() == 1)
  {
    pcPic->getPicSym()->setTileBoundaryIndependenceIdr( pcSlice->getPPS()->getTileBoundaryIndependenceIdr() );
  }
  else
  {
    pcPic->getPicSym()->setTileBoundaryIndependenceIdr( pcSlice->getPPS()->getSPS()->getTileBoundaryIndependenceIdr() );
  }
#endif

  if( pcSlice->getPPS()->getColumnRowInfoPresent() == 1 )
  {
    //set NumColumnsMins1 and NumRowsMinus1
    pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getPPS()->getNumColumnsMinus1() );
    pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getPPS()->getNumRowsMinus1() );

    //create the TComTileArray
    pcPic->getPicSym()->xCreateTComTileArray();

    if( pcSlice->getPPS()->getUniformSpacingIdr() == 1)
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
      for(j=0; j < pcSlice->getPPS()->getNumRowsMinus1()+1; j++)
      {
        uiCummulativeTileWidth = 0;
        for(i=0; i < pcSlice->getPPS()->getNumColumnsMinus1(); i++)
        {
          pcPic->getPicSym()->getTComTile(j * (pcSlice->getPPS()->getNumColumnsMinus1()+1) + i)->setTileWidth( pcSlice->getPPS()->getColumnWidth(i) );
          uiCummulativeTileWidth += pcSlice->getPPS()->getColumnWidth(i);
        }
        pcPic->getPicSym()->getTComTile(j * (pcSlice->getPPS()->getNumColumnsMinus1()+1) + i)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
      }

      //set the height for each tile
      for(j=0; j < pcSlice->getPPS()->getNumColumnsMinus1()+1; j++)
      {
        uiCummulativeTileHeight = 0;
        for(i=0; i < pcSlice->getPPS()->getNumRowsMinus1(); i++)
        { 
          pcPic->getPicSym()->getTComTile(i * (pcSlice->getPPS()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcSlice->getPPS()->getRowHeight(i) );
          uiCummulativeTileHeight += pcSlice->getPPS()->getRowHeight(i);
        }
        pcPic->getPicSym()->getTComTile(i * (pcSlice->getPPS()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
      }
    }
  }
  else
  {
    //set NumColumnsMins1 and NumRowsMinus1
    pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getSPS()->getNumColumnsMinus1() );
    pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getSPS()->getNumRowsMinus1() );

    //create the TComTileArray
    pcPic->getPicSym()->xCreateTComTileArray();

    //automatically set the column and row boundary if UniformSpacingIdr = 1
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
      for(j=0; j < pcSlice->getSPS()->getNumRowsMinus1()+1; j++)
      {
        uiCummulativeTileWidth = 0;
        for(i=0; i < pcSlice->getSPS()->getNumColumnsMinus1(); i++)
        {
          pcPic->getPicSym()->getTComTile(j * (pcSlice->getSPS()->getNumColumnsMinus1()+1) + i)->setTileWidth( pcSlice->getSPS()->getColumnWidth(i) );
          uiCummulativeTileWidth += pcSlice->getSPS()->getColumnWidth(i);
        }
        pcPic->getPicSym()->getTComTile(j * (pcSlice->getSPS()->getNumColumnsMinus1()+1) + i)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
      }

      //set the height for each tile
      for(j=0; j < pcSlice->getSPS()->getNumColumnsMinus1()+1; j++)
      {
        uiCummulativeTileHeight = 0;
        for(i=0; i < pcSlice->getSPS()->getNumRowsMinus1(); i++)
        { 
          pcPic->getPicSym()->getTComTile(i * (pcSlice->getSPS()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcSlice->getSPS()->getRowHeight(i) );
          uiCummulativeTileHeight += pcSlice->getSPS()->getRowHeight(i);
        }
        pcPic->getPicSym()->getTComTile(i * (pcSlice->getSPS()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
      }
    }
  }

  pcPic->getPicSym()->xInitTiles();

  //generate the Coding Order Map and Inverse Coding Order Map
  UInt uiEncCUAddr;
  for(i=0, uiEncCUAddr=0; i<pcPic->getPicSym()->getNumberOfCUsInFrame(); i++, uiEncCUAddr = pcPic->getPicSym()->xCalculateNxtCUAddr(uiEncCUAddr))
  {
    pcPic->getPicSym()->setCUOrderMap(i, uiEncCUAddr);
    pcPic->getPicSym()->setInverseCUOrderMap(uiEncCUAddr, i);
  }
  pcPic->getPicSym()->setCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());
  pcPic->getPicSym()->setInverseCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());

  //convert the start and end CU addresses of the slice and entropy slice into encoding order
  pcSlice->setEntropySliceCurStartCUAddr( pcPic->getPicSym()->getPicSCUEncOrder(pcSlice->getEntropySliceCurStartCUAddr()) );
  pcSlice->setEntropySliceCurEndCUAddr( pcPic->getPicSym()->getPicSCUEncOrder(pcSlice->getEntropySliceCurEndCUAddr()) );
  if(pcSlice->isNextSlice())
  {
    pcSlice->setSliceCurStartCUAddr(pcPic->getPicSym()->getPicSCUEncOrder(pcSlice->getSliceCurStartCUAddr()));
    pcSlice->setSliceCurEndCUAddr(pcPic->getPicSym()->getPicSCUEncOrder(pcSlice->getSliceCurEndCUAddr()));
  }

  if (m_bFirstSliceInPicture) 
  {
    if(pcPic->getNumAllocatedSlice() != 1)
    {
      pcPic->clearSliceBuffer();
    }
  }
  else
  {
    pcPic->allocateNewSlice();
  }
  assert(pcPic->getNumAllocatedSlice() == (m_uiSliceIdx + 1));
  m_apcSlicePilot = pcPic->getPicSym()->getSlice(m_uiSliceIdx); 
  pcPic->getPicSym()->setSlice(pcSlice, m_uiSliceIdx);

  pcPic->setTLayer(nalu.m_temporalId);

  if (bNextSlice)
  {
    pcSlice->checkCRA(pcSlice->getRPS(), m_pocCRA, m_cListPic); 

    if ( !pcSlice->getPPS()->getEnableTMVPFlag() && pcPic->getTLayer() == 0 )
    {
      pcSlice->decodingMarkingForNoTMVP( m_cListPic, pcSlice->getPOC() );
    }

    // Set reference list
    pcSlice->setRefPicList( m_cListPic );

    // For generalized B
    // note: maybe not existed case (always L0 is copied to L1 if L1 is empty)
    if (pcSlice->isInterB() && pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0)
    {
      Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
      pcSlice->setNumRefIdx        ( REF_PIC_LIST_1, iNumRefIdx );

      for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
      {
        pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
      }
    }
    if (pcSlice->isInterB())
    {
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

    //---------------
    pcSlice->setRefPOCList();

    if(!pcSlice->getRefPicListModificationFlagLC())
    {
      pcSlice->generateCombinedList();
    }

    if( pcSlice->getRefPicListCombinationFlag() && pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getSliceType()==B_SLICE )
    {
      pcSlice->setWpParamforLC();
    }
    pcSlice->setNoBackPredFlag( false );
    if ( pcSlice->getSliceType() == B_SLICE && !pcSlice->getRefPicListCombinationFlag())
    {
      if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
      {
        pcSlice->setNoBackPredFlag( true );
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
  }

  pcPic->setCurrSliceIdx(m_uiSliceIdx);
  if(pcSlice->getSPS()->getScalingListFlag())
  {
    if(pcSlice->getAPS()->getScalingListEnabled())
    {
      pcSlice->setScalingList ( pcSlice->getAPS()->getScalingList()  );
      if(pcSlice->getScalingList()->getScalingListPresentFlag())
      {
        pcSlice->setDefaultScalingList();
      }
      m_cTrQuant.setScalingListDec(pcSlice->getScalingList());
    }
    m_cTrQuant.setUseScalingList(true);
  }
  else
  {
    m_cTrQuant.setFlatScalingList();
    m_cTrQuant.setUseScalingList(false);
  }

  //  Decode a picture
  m_cGopDecoder.decompressGop(nalu.m_Bitstream, pcPic, false);

  m_bFirstSliceInPicture = false;
  m_uiSliceIdx++;

  return false;
}


Void TDecTop::xDecodeSPS()
{
  TComSPS* sps = new TComSPS();
#if RPS_IN_SPS
  TComRPSList* rps = new TComRPSList();
  sps->setRPSList(rps);
#endif
  m_cEntropyDecoder.decodeSPS( sps );
  m_parameterSetManagerDecoder.storePrefetchedSPS(sps);
#if LCU_SYNTAX_ALF
  m_cAdaptiveLoopFilter.create( sps->getPicWidthInLumaSamples(), sps->getPicHeightInLumaSamples(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#endif
}

Void TDecTop::xDecodePPS()
{
#if !RPS_IN_SPS
  TComRPSList* rps = new TComRPSList();
#endif
  TComPPS* pps = new TComPPS();
#if !RPS_IN_SPS
  pps->setRPSList(rps);
#endif
#if TILES_OR_ENTROPY_SYNC_IDC
  m_cEntropyDecoder.decodePPS( pps, &m_parameterSetManagerDecoder );
#else
  m_cEntropyDecoder.decodePPS( pps );
#endif
  m_parameterSetManagerDecoder.storePrefetchedPPS( pps );

  //!!!KS: Activate parameter sets for parsing APS (unless dependency is resolved)
  m_apcSlicePilot->setPPSId(pps->getPPSId());
  xActivateParameterSets();
  m_apcSlicePilot->initTiles();
}

Void TDecTop::xDecodeAPS()
{
  TComAPS  *aps = new TComAPS();
  allocAPS (aps);
  decodeAPS(aps);
  m_parameterSetManagerDecoder.storePrefetchedAPS(aps);
}

Void TDecTop::xDecodeSEI()
{
  m_SEIs = new SEImessages;
  m_cEntropyDecoder.decodeSEI(*m_SEIs);
}

Bool TDecTop::decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay)
{
  // Initialize entropy decoder
  m_cEntropyDecoder.setEntropyDecoder (&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream      (nalu.m_Bitstream);

  switch (nalu.m_nalUnitType)
  {
    case NAL_UNIT_SPS:
      xDecodeSPS();
      return false;

    case NAL_UNIT_PPS:
      xDecodePPS();
      return false;
    case NAL_UNIT_APS:
      xDecodeAPS();
      return false;

    case NAL_UNIT_SEI:
      xDecodeSEI();
      return false;

    case NAL_UNIT_CODED_SLICE:
    case NAL_UNIT_CODED_SLICE_IDR:
#if H0566_TLA
    case NAL_UNIT_CODED_SLICE_CRA:
    case NAL_UNIT_CODED_SLICE_TLA:
#else
    case NAL_UNIT_CODED_SLICE_CDR:
#endif
      return xDecodeSlice(nalu, iSkipFrame, iPOCLastDisplay);
      break;
    default:
      assert (1);
  }

  return false;
}

/** Function for checking if picture should be skipped because of random access
 * \param iSkipFrame skip frame counter
 * \param iPOCLastDisplay POC of last picture displayed
 * \returns true if the picture shold be skipped in the random access.
 * This function checks the skipping of pictures in the case of -s option random access.
 * All pictures prior to the random access point indicated by the counter iSkipFrame are skipped.
 * It also checks the type of Nal unit type at the random access point.
 * If the random access point is CRA, pictures with POC equal to or greater than the CRA POC are decoded.
 * If the random access point is IDR all pictures after the random access point are decoded.
 * If the random access point is not IDR or CRA, a warning is issues, and decoding of pictures with POC 
 * equal to or greater than the random access point POC is attempted. For non IDR/CRA random 
 * access point there is no guarantee that the decoder will not crash.
 */
Bool TDecTop::isRandomAccessSkipPicture(Int& iSkipFrame,  Int& iPOCLastDisplay)
{
  if (iSkipFrame) 
  {
    iSkipFrame--;   // decrement the counter
    return true;
  }
  else if (m_pocRandomAccess == MAX_INT) // start of random access point, m_pocRandomAccess has not been set yet.
  {
#if H0566_TLA
    if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA)
#else
    if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR)
#endif
    {
      m_pocRandomAccess = m_apcSlicePilot->getPOC(); // set the POC random access since we need to skip the reordered pictures in CRA.
    }
    else if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)
    {
      m_pocRandomAccess = 0; // no need to skip the reordered pictures in IDR, they are decodable.
    }
    else 
    {
#if START_DECODING_AT_CRA
      static bool warningMessage = false;
      if(!warningMessage)
      {
        printf("\nWarning: this is not a valid random access point and the data is discarded until the first CRA picture");
        warningMessage = true;
      }
      return true;
#else
      printf("\nUnsafe random access point. Decoder may crash.");
      m_pocRandomAccess = 0;
#endif
    }
  }
  else if (m_apcSlicePilot->getPOC() < m_pocRandomAccess)  // skip the reordered pictures if necessary
  {
    iPOCLastDisplay++;
    return true;
  }
  // if we reach here, then the picture is not skipped.
  return false; 
}

Void TDecTop::allocAPS (TComAPS* pAPS)
{
  // we don't know the SPS before it has been activated. These fields could exist
  // depending on the corresponding flags in the APS, but SAO/ALF allocation functions will
  // have to be moved for that
  pAPS->createScalingList();
  pAPS->createSaoParam();
  m_cSAO.allocSaoParam(pAPS->getSaoParam());
  pAPS->createAlfParam();
#if !LCU_SYNTAX_ALF
  m_cAdaptiveLoopFilter.allocALFParam(pAPS->getAlfParam());
#endif
}

//! \}
