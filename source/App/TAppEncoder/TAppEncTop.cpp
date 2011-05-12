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

/** \file     TAppEncTop.cpp
    \brief    Encoder application class
*/

#include <list>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppEncTop.h"
#include "../../Lib/TLibEncoder/AnnexBwrite.h"

using namespace std;

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncTop::TAppEncTop()
{
  m_iFrameRcvd = 0;
}

TAppEncTop::~TAppEncTop()
{
}

Void TAppEncTop::xInitLibCfg()
{
  m_cTEncTop.setFrameRate                    ( m_iFrameRate );
  m_cTEncTop.setFrameSkip                    ( m_FrameSkip );
  m_cTEncTop.setSourceWidth                  ( m_iSourceWidth );
  m_cTEncTop.setSourceHeight                 ( m_iSourceHeight );
  m_cTEncTop.setFrameToBeEncoded             ( m_iFrameToBeEncoded );
  
  //====== Coding Structure ========
  m_cTEncTop.setIntraPeriod                  ( m_iIntraPeriod );
#if DCM_DECODING_REFRESH
  m_cTEncTop.setDecodingRefreshType          ( m_iDecodingRefreshType );
#endif
  m_cTEncTop.setGOPSize                      ( m_iGOPSize );
  m_cTEncTop.setRateGOPSize                  ( m_iRateGOPSize );
  m_cTEncTop.setNumOfReference               ( m_iNumOfReference );
  m_cTEncTop.setNumOfReferenceB_L0           ( m_iNumOfReferenceB_L0 );
  m_cTEncTop.setNumOfReferenceB_L1           ( m_iNumOfReferenceB_L1 );
  
  m_cTEncTop.setQP                           ( m_iQP );
  
  m_cTEncTop.setTemporalLayerQPOffset        ( m_aiTLayerQPOffset );
  m_cTEncTop.setPad                          ( m_aiPad );
    
  m_cTEncTop.setTLayering                    ( m_bTLayering );
  m_cTEncTop.setTLayerSwitchingFlag          ( m_abTLayerSwitchingFlag );

  //===== Slice ========
  m_cTEncTop.setHierarchicalCoding           ( m_bHierarchicalCoding );
  
  //====== Entropy Coding ========
  m_cTEncTop.setSymbolMode                   ( m_iSymbolMode );
  
  //====== Loop/Deblock Filter ========
  m_cTEncTop.setLoopFilterDisable            ( m_bLoopFilterDisable       );
  m_cTEncTop.setLoopFilterAlphaC0Offset      ( m_iLoopFilterAlphaC0Offset );
  m_cTEncTop.setLoopFilterBetaOffset         ( m_iLoopFilterBetaOffset    );
  
  //====== Motion search ========
  m_cTEncTop.setFastSearch                   ( m_iFastSearch  );
  m_cTEncTop.setSearchRange                  ( m_iSearchRange );
  m_cTEncTop.setBipredSearchRange            ( m_bipredSearchRange );
  m_cTEncTop.setMaxDeltaQP                   ( m_iMaxDeltaQP  );
#if SUB_LCU_DQP
  m_cTEncTop.setMaxCuDQPDepth                ( m_iMaxCuDQPDepth  );
#endif
  
  //====== Tool list ========
  m_cTEncTop.setUseSBACRD                    ( m_bUseSBACRD   );
  m_cTEncTop.setDeltaQpRD                    ( m_uiDeltaQpRD  );
  m_cTEncTop.setUseASR                       ( m_bUseASR      );
  m_cTEncTop.setUseHADME                     ( m_bUseHADME    );
  m_cTEncTop.setUseALF                       ( m_bUseALF      );
#if MQT_ALF_NPASS
  m_cTEncTop.setALFEncodePassReduction       ( m_iALFEncodePassReduction );
#endif
  m_cTEncTop.setUseGPB                       ( m_bUseGPB      );
#if DCM_COMB_LIST
  m_cTEncTop.setUseLComb                     ( m_bUseLComb    );
  m_cTEncTop.setLCMod                        ( m_bLCMod         );
#endif
  m_cTEncTop.setdQPs                         ( m_aidQP        );
  m_cTEncTop.setUseRDOQ                      ( m_bUseRDOQ     );
  m_cTEncTop.setUseLDC                       ( m_bUseLDC      );
  m_cTEncTop.setUsePAD                       ( m_bUsePAD      );
  m_cTEncTop.setQuadtreeTULog2MaxSize        ( m_uiQuadtreeTULog2MaxSize );
  m_cTEncTop.setQuadtreeTULog2MinSize        ( m_uiQuadtreeTULog2MinSize );
  m_cTEncTop.setQuadtreeTUMaxDepthInter      ( m_uiQuadtreeTUMaxDepthInter );
  m_cTEncTop.setQuadtreeTUMaxDepthIntra      ( m_uiQuadtreeTUMaxDepthIntra );
  m_cTEncTop.setUseNRF                       ( m_bUseNRF      );
  m_cTEncTop.setUseBQP                       ( m_bUseBQP      );
  m_cTEncTop.setUseFastEnc                   ( m_bUseFastEnc  );
  m_cTEncTop.setUseMRG                       ( m_bUseMRG      ); // SOPH:

#if LM_CHROMA 
  m_cTEncTop.setUseLMChroma                  ( m_bUseLMChroma );
#endif

#if HHI_RMP_SWITCH
  m_cTEncTop.setUseRMP                     ( m_bUseRMP );
#endif
#ifdef ROUNDING_CONTROL_BIPRED
  m_cTEncTop.setUseRoundingControlBipred(m_useRoundingControlBipred);
#endif
#if CONSTRAINED_INTRA_PRED
  m_cTEncTop.setUseConstrainedIntraPred      ( m_bUseConstrainedIntraPred );
#endif
#if E057_INTRA_PCM
  m_cTEncTop.setPCMLog2MinSize          ( m_uiPCMLog2MinSize);
#endif
  //====== Slice ========
  m_cTEncTop.setSliceMode               ( m_iSliceMode                );
  m_cTEncTop.setSliceArgument           ( m_iSliceArgument            );

  //====== Entropy Slice ========
  m_cTEncTop.setEntropySliceMode        ( m_iEntropySliceMode         );
  m_cTEncTop.setEntropySliceArgument    ( m_iEntropySliceArgument     );
#if MTK_NONCROSS_INLOOP_FILTER
  if(m_iSliceMode == 0 )
  {
    m_bLFCrossSliceBoundaryFlag = true;
  }
  m_cTEncTop.setLFCrossSliceBoundaryFlag( m_bLFCrossSliceBoundaryFlag );
#endif
#if MTK_SAO
  m_cTEncTop.setUseSAO               ( m_bUseSAO         );
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_BIT_DEPTH_SYNTAX
  m_cTEncTop.setPCMInputBitDepthFlag  ( m_bPCMInputBitDepthFlag); 
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  m_cTEncTop.setPCMFilterDisableFlag  ( m_bPCMFilterDisableFlag); 
#endif

  m_cTEncTop.setPictureDigestEnabled(m_pictureDigestEnabled);
}

Void TAppEncTop::xCreateLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.open( m_pchInputFile,     false, m_uiInputBitDepth, m_uiInternalBitDepth );  // read  mode
  m_cTVideoIOYuvInputFile.skipFrames(m_FrameSkip, m_iSourceWidth, m_iSourceHeight);

  m_cTVideoIOYuvReconFile.open( m_pchReconFile,     true, m_uiOutputBitDepth, m_uiInternalBitDepth);  // write mode
  
  // Neo Decoder
  m_cTEncTop.create();
}

Void TAppEncTop::xDestroyLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.close();
  m_cTVideoIOYuvReconFile.close();
  
  // Neo Decoder
  m_cTEncTop.destroy();
}

Void TAppEncTop::xInitLib()
{
  m_cTEncTop.init();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal variable
 - until the end of input YUV file, call encoding function in TEncTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppEncTop::encode()
{
  fstream bitstreamFile(m_pchBitstreamFile, fstream::binary | fstream::out);
  if (!bitstreamFile)
  {
    fprintf(stderr, "\nfailed to open bitstream file `%s' for writing\n", m_pchBitstreamFile);
    exit(EXIT_FAILURE);
  }

  TComPicYuv*       pcPicYuvOrg = new TComPicYuv;
  TComPicYuv*       pcPicYuvRec = NULL;
  
  // initialize internal class & member variables
  xInitLibCfg();
  xCreateLib();
  xInitLib();
  
  // main encoder loop
  Int   iNumEncoded = 0;
  Bool  bEos = false;
  
  list<AccessUnit> outputAccessUnits; ///< list of access units to write out.  is populated by the encoding process

  // allocate original YUV buffer
  pcPicYuvOrg->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
  
  while ( !bEos )
  {
    // get buffers
    xGetBuffer(pcPicYuvRec);
    
    // read input YUV file
    m_cTVideoIOYuvInputFile.read( pcPicYuvOrg, m_aiPad );
    
    // increase number of received frames
    m_iFrameRcvd++;
    
    // check end of file
    bEos = ( m_cTVideoIOYuvInputFile.isEof() == 1 ?   true : false  );
    bEos = ( m_iFrameRcvd == m_iFrameToBeEncoded ?    true : bEos   );
    
    // call encoding function for one frame
    m_cTEncTop.encode( bEos, pcPicYuvOrg, m_cListPicYuvRec, outputAccessUnits, iNumEncoded );
    
    // write bistream to file if necessary
    if ( iNumEncoded > 0 )
    {
      xWriteOutput(bitstreamFile, iNumEncoded, outputAccessUnits);
      outputAccessUnits.clear();
    }
  }
  // delete original YUV buffer
  pcPicYuvOrg->destroy();
  delete pcPicYuvOrg;
  pcPicYuvOrg = NULL;
  
  // delete used buffers in encoder class
  m_cTEncTop.deletePicBuffer();
  
  // delete buffers & classes
  xDeleteBuffer();
  xDestroyLib();
  
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - application has picture buffer list with size of GOP
 - picture buffer list acts as ring buffer
 - end of the list has the latest picture
 .
 */
Void TAppEncTop::xGetBuffer( TComPicYuv*& rpcPicYuvRec)
{
  if ( m_iGOPSize == 0 )
  {
    if (m_cListPicYuvRec.size() == 0)
    {
      rpcPicYuvRec = new TComPicYuv;
      rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
      m_cListPicYuvRec.pushBack( rpcPicYuvRec );
    }
    
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
    
    m_cListPicYuvRec.pushBack( rpcPicYuvRec );
    
    return;
  }
  
  // org. buffer
  if ( m_cListPicYuvRec.size() == (UInt)m_iGOPSize )
  {
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
  }
  else
  {
    rpcPicYuvRec = new TComPicYuv;
    
    rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
  }
  m_cListPicYuvRec.pushBack( rpcPicYuvRec );
}

Void TAppEncTop::xDeleteBuffer( )
{
  TComList<TComPicYuv*>::iterator iterPicYuvRec  = m_cListPicYuvRec.begin();
  
  Int iSize = Int( m_cListPicYuvRec.size() );
  
  for ( Int i = 0; i < iSize; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    pcPicYuvRec->destroy();
    delete pcPicYuvRec; pcPicYuvRec = NULL;
  }
  
}

/** \param iNumEncoded  number of encoded frames
 */
Void TAppEncTop::xWriteOutput(ostream& bitstreamFile, Int iNumEncoded, const list<AccessUnit>& accessUnits)
{
  Int i;
  
  TComList<TComPicYuv*>::iterator iterPicYuvRec = m_cListPicYuvRec.end();
  list<AccessUnit>::const_iterator iterBitstream = accessUnits.begin();
  
  for ( i = 0; i < iNumEncoded; i++ )
  {
    --iterPicYuvRec;
  }
  
  for ( i = 0; i < iNumEncoded; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    m_cTVideoIOYuvReconFile.write( pcPicYuvRec, m_aiPad );

    const AccessUnit& au = *(iterBitstream++);
    writeAnnexB(bitstreamFile, au);
  }
}

