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

/** \file     TEncTop.cpp
    \brief    encoder class
*/

#include "../TLibCommon/CommonDef.h"
#include "TEncTop.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncTop::TEncTop()
{
  m_iPOCLast          = -1;
  m_iNumPicRcvd       =  0;
  m_uiNumAllPicCoded  =  0;
  m_pppcRDSbacCoder   =  NULL;
  m_pppcBinCoderCABAC =  NULL;
  m_cRDGoOnSbacCoder.init( &m_cRDGoOnBinCoderCABAC );
#if HHI_RQT
#if ENC_DEC_TRACE
  g_hTrace = fopen( "TraceEnc.txt", "wb" );
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif
#endif
}

TEncTop::~TEncTop()
{
#if HHI_RQT
#if ENC_DEC_TRACE
  fclose( g_hTrace );
#endif
#endif
}

Void TEncTop::create ()
{
  // initialize global variables
  initROM();

  // create processing unit classes
  m_cGOPEncoder.        create();
  m_cSliceEncoder.      create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  m_cCuEncoder.         create( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight );
  m_cAdaptiveLoopFilter.create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
  m_cLoopFilter.        create( g_uiMaxCUDepth );
#endif

  // if SBAC-based RD optimization is used
  if( m_bUseSBACRD )
  {
    m_pppcRDSbacCoder = new TEncSbac** [g_uiMaxCUDepth+1];
    m_pppcBinCoderCABAC = new TEncBinCABAC** [g_uiMaxCUDepth+1];

    for ( Int iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
      m_pppcBinCoderCABAC[iDepth] = new TEncBinCABAC* [CI_NUM];

      for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
      {
        m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
        m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABAC;
        m_pppcRDSbacCoder   [iDepth][iCIIdx]->init( m_pppcBinCoderCABAC [iDepth][iCIIdx] );
      }
    }
  }
}

Void TEncTop::destroy ()
{
  // destroy processing unit classes
  m_cGOPEncoder.        destroy();
  m_cSliceEncoder.      destroy();
  m_cCuEncoder.         destroy();
  m_cAdaptiveLoopFilter.destroy();
#if HHI_DEBLOCKING_FILTER || TENTM_DEBLOCKING_FILTER
  m_cLoopFilter.        destroy();
#endif

  // SBAC RD
  if( m_bUseSBACRD )
  {
    Int iDepth;
    for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
      {
        delete m_pppcRDSbacCoder[iDepth][iCIIdx];
        delete m_pppcBinCoderCABAC[iDepth][iCIIdx];
      }
    }

    for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      delete [] m_pppcRDSbacCoder[iDepth];
      delete [] m_pppcBinCoderCABAC[iDepth];
    }

    delete [] m_pppcRDSbacCoder;
    delete [] m_pppcBinCoderCABAC;
  }

  // destroy ROM
  destroyROM();

  return;
}

Void TEncTop::init()
{
  // initialize SPS
  xInitSPS();

  // initialize processing unit classes
  m_cGOPEncoder.  init( this );
  m_cSliceEncoder.init( this );
  m_cCuEncoder.   init( this );

  // initialize DIF
  m_cSearch.setDIFTap ( m_cSPS.getDIFTap () );

  // initialize transform & quantization class
#if HHI_ALLOW_ROT_SWITCH
  m_cTrQuant.init( g_uiMaxCUWidth, g_uiMaxCUHeight, m_uiMaxTrSize, m_bUseROT, m_bUseRDOQ, true );
#else
  m_cTrQuant.init( g_uiMaxCUWidth, g_uiMaxCUHeight, m_uiMaxTrSize, m_bUseRDOQ, true );
#endif

  // initialize encoder search class
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, m_iFastSearch, 0, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncTop::deletePicBuffer()
{
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );

  for ( Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);

    pcPic->destroy();
    delete pcPic;
    pcPic = NULL;
  }
}

/**
   - Application has picture buffer list with size of GOP + 1
   - Picture buffer list acts like as ring buffer
   - End of the list has the latest picture
   .
   \param   bEos                true if end-of-sequence is reached
   \param   pcPicYuvOrg         original YUV picture
   \retval  rcListPicYuvRecOut  list of reconstruction YUV pictures
   \retval  rcListBitstreamOut  list of output bitstreams
   \retval  iNumEncoded         number of encoded pictures
 */
Void TEncTop::encode( bool bEos, TComPicYuv* pcPicYuvOrg, TComList<TComPicYuv*>& rcListPicYuvRecOut, TComList<TComBitstream*>& rcListBitstreamOut, Int& iNumEncoded )
{
  TComPic* pcPicCurr = NULL;

  // get original YUV
  xGetNewPicBuffer( pcPicCurr );
  pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );

  if ( m_iPOCLast != 0 && ( m_iNumPicRcvd != m_iGOPSize && m_iGOPSize ) && !bEos )
  {
    iNumEncoded = 0;
    return;
  }

  // compress GOP
  m_cGOPEncoder.compressGOP( m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, rcListBitstreamOut );

  iNumEncoded         = m_iNumPicRcvd;
  m_iNumPicRcvd       = 0;
  m_uiNumAllPicCoded += iNumEncoded;

  if (bEos)
  {
    m_cGOPEncoder.printOutSummary (m_uiNumAllPicCoded);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
   - Application has picture buffer list with size of GOP + 1
   - Picture buffer list acts like as ring buffer
   - End of the list has the latest picture
   .
   \retval rpcPic obtained picture buffer
 */
Void TEncTop::xGetNewPicBuffer ( TComPic*& rpcPic )
{
  TComSlice::sortPicList(m_cListPic);

  // bug-fix - erase frame memory (previous GOP) which is not used for reference any more
  if (m_cListPic.size() >= (UInt)(m_iGOPSize + 2 * getNumOfReference() + 1) )  // 2)   //  K. Lee bug fix - for multiple reference > 2
  {
    rpcPic = m_cListPic.popFront();

    // is it necessary without long-term reference?
    if ( rpcPic->getERBIndex() > 0 && abs(rpcPic->getPOC() - m_iPOCLast) <= 0 )
    {
      m_cListPic.pushFront(rpcPic);

      TComList<TComPic*>::iterator iterPic  = m_cListPic.begin();
      rpcPic = *(++iterPic);
      if ( abs(rpcPic->getPOC() - m_iPOCLast) <= m_iGOPSize )
      {
        rpcPic = new TComPic;
        rpcPic->create( m_iSourceWidth, m_iSourceHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
      }
      else
      {
        m_cListPic.erase( iterPic );
        TComSlice::sortPicList( m_cListPic );
      }
    }
  }
  else
  {
    rpcPic = new TComPic;
    rpcPic->create( m_iSourceWidth, m_iSourceHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  }

  m_cListPic.pushBack( rpcPic );
  rpcPic->setReconMark (false);

  m_iPOCLast++;
  m_iNumPicRcvd++;

  rpcPic->getSlice()->setPOC( m_iPOCLast );

  // mark it should be extended
  rpcPic->getPicYuvRec()->setBorderExtension(false);

#if HHI_INTERP_FILTER
  rpcPic->getPicYuvRecFilt()->setBorderExtension(false);
#endif
}

Void TEncTop::xInitSPS()
{
  m_cSPS.setWidth         ( m_iSourceWidth      );
  m_cSPS.setHeight        ( m_iSourceHeight     );
  m_cSPS.setPad           ( m_aiPad             );
  m_cSPS.setMaxCUWidth    ( g_uiMaxCUWidth      );
  m_cSPS.setMaxCUHeight   ( g_uiMaxCUHeight     );
  m_cSPS.setMaxCUDepth    ( g_uiMaxCUDepth      );
  m_cSPS.setMinTrDepth    ( m_uiMinTrDepth      );
  m_cSPS.setMaxTrDepth    ( m_uiMaxTrDepth      );

  m_cSPS.setUseALF        ( m_bUseALF           );

#if HHI_RQT
  m_cSPS.setQuadtreeTUFlag       ( m_bQuadtreeTUFlag         );
  m_cSPS.setQuadtreeTULog2MaxSize( m_uiQuadtreeTULog2MaxSize );
  m_cSPS.setQuadtreeTULog2MinSize( m_uiQuadtreeTULog2MinSize );
#endif

#if HHI_ALF
  m_cSPS.setALfSeparateQt ( m_bALFSeparateQt    );
  m_cSPS.setALFSymmetry   ( m_bALFSymmetry      );
  m_cSPS.setALFMinLength  ( m_iALFMinLength     );
  m_cSPS.setALFMaxLength  ( m_iALFMaxLength     );
#endif

  m_cSPS.setUseDQP        ( m_iMaxDeltaQP != 0  );
  m_cSPS.setUseLDC        ( m_bUseLDC           );
  m_cSPS.setUsePAD        ( m_bUsePAD           );
  m_cSPS.setUseQBO        ( m_bUseQBO           );

#if HHI_ALLOW_CIP_SWITCH
  m_cSPS.setUseCIP        ( m_bUseCIP           ); // BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
  m_cSPS.setUseROT        ( m_bUseROT           ); // BB:
#endif
#if HHI_AIS
  m_cSPS.setUseAIS        ( m_bUseAIS           ); // BB:
#endif
#if HHI_MRG
  m_cSPS.setUseMRG        ( m_bUseMRG           ); // SOPH:
#endif
#if HHI_IMVP
  m_cSPS.setUseIMP        ( m_bUseIMP           ); // SOPH:
#endif

  m_cSPS.setDIFTap        ( m_iDIFTap           );

  m_cSPS.setMaxTrSize     ( m_uiMaxTrSize       );

  Int i;
#if HHI_AMVP_OFF
  for ( i = 0; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMVPMode( i, AM_NONE );
  }
#else
  for ( i = 0; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMVPMode( i, AM_EXPL );
  }
#endif
  

  for (i = 0; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMPAcc( i, 1 );
  }

  for (i = 0; i < g_uiMaxCUDepth; i++ )
  {
    if (m_cSPS.getAMPAcc(i) > g_uiMaxCUDepth - 1 - i)
    {
      m_cSPS.setAMPAcc(i, g_uiMaxCUDepth - 1 - i);
    }
  }

  m_cSPS.setBitDepth    ( g_uiBitDepth        );
  m_cSPS.setBitIncrement( g_uiBitIncrement    );

  if (getGRefMode())
  {
    if (strchr(getGRefMode() ,'w')) m_cSPS.setUseWPG(true);
    if (strchr(getGRefMode() ,'o')) m_cSPS.setUseWPO(true);
  }
}

