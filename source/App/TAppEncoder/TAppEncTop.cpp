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

/** \file			TAppEncTop.cpp
    \brief		Encoder application class
*/

#include <list>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppEncTop.h"

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
  m_cTEncTop.setFrameSkip                    ( m_iFrameSkip );
  m_cTEncTop.setSourceWidth                  ( m_iSourceWidth );
  m_cTEncTop.setSourceHeight                 ( m_iSourceHeight );
  m_cTEncTop.setFrameToBeEncoded             ( m_iFrameToBeEncoded );

  m_cTEncTop.setGeneratedReferenceMode       ( m_pchGeneratedReferenceMode );

  //====== Coding Structure ========
  m_cTEncTop.setIntraPeriod                  ( m_iIntraPeriod );
  m_cTEncTop.setGOPSize                      ( m_iGOPSize );
	m_cTEncTop.setRateGOPSize                  ( m_iRateGOPSize );
  m_cTEncTop.setNumOfReference               ( m_iNumOfReference );
	m_cTEncTop.setNumOfReferenceB_L0           ( m_iNumOfReferenceB_L0 );
	m_cTEncTop.setNumOfReferenceB_L1           ( m_iNumOfReferenceB_L1 );

  m_cTEncTop.setQP                           ( m_iQP );

  m_cTEncTop.setTemporalLayerQPOffset        ( m_aiTLayerQPOffset );
  m_cTEncTop.setPad                          ( m_aiPad );

  m_cTEncTop.setMinTrDepth                   ( m_uiMinTrDepth );
  m_cTEncTop.setMaxTrDepth                   ( m_uiMaxTrDepth );

  //===== Slice ========
  m_cTEncTop.setHierarchicalCoding           ( m_bHierarchicalCoding );

  //====== Entropy Coding ========
  m_cTEncTop.setSymbolMode                   ( m_iSymbolMode );

  //====== Loop/Deblock Filter ========
  m_cTEncTop.setLoopFilterDisable            ( m_bLoopFilterDisable );
  m_cTEncTop.setLoopFilterAlphaC0Offset      ( m_iLoopFilterAlphaC0Offset );
  m_cTEncTop.setLoopFilterBetaOffset         ( m_iLoopFilterBetaOffset );

  //====== Motion search ========
  m_cTEncTop.setFastSearch                   ( m_iFastSearch  );
  m_cTEncTop.setSearchRange                  ( m_iSearchRange );
  m_cTEncTop.setMaxDeltaQP                   ( m_iMaxDeltaQP  );

  //====== IP list ========
  m_cTEncTop.setUseSBACRD										 ( m_bUseSBACRD		);
  m_cTEncTop.setUseMVAC                      ( m_bUseMVAC			);
  m_cTEncTop.setUseMVACRD                    ( m_bUseMVACRD   );
  m_cTEncTop.setDeltaQpRD										 ( m_uiDeltaQpRD  );
  m_cTEncTop.setUseADI                       ( m_bUseADI			);
	m_cTEncTop.setUseACC                       ( m_bUseACC			);
  m_cTEncTop.setUseASR                       ( m_bUseASR      );
	m_cTEncTop.setUseROT                       ( m_bUseROT			);
	m_cTEncTop.setUseMPI                       ( m_bUseMPI			);
	m_cTEncTop.setUseJMQP                      ( m_bUseJMQP			);
	m_cTEncTop.setUseJMLAMBDA                  ( m_bUseJMLAMBDA	);
	m_cTEncTop.setUseAMVP                      ( m_bUseAMVP			);
	m_cTEncTop.setUseIMR                       ( m_bUseIMR			);
	m_cTEncTop.setUseDIF                       ( m_bUseDIF			);
	m_cTEncTop.setUseHADME                     ( m_bUseHADME		);
	m_cTEncTop.setUseALF	                     ( m_bUseALF  		);
	m_cTEncTop.setUseRNG	                     ( m_bUseRNG	 		);
	m_cTEncTop.setUseGPB	                     ( m_bUseGPB	 		);
	m_cTEncTop.setUseAMP	                     ( m_bUseAMP	 		);
  m_cTEncTop.setUseSHV											 ( m_bUseSHV			);
	m_cTEncTop.setUseFAM	                     ( m_bUseFAM	 		);
	m_cTEncTop.setdQPs                         ( m_aidQP        );
  m_cTEncTop.setUseRDOQ                      ( m_bUseRDOQ     );
  m_cTEncTop.setUseLOT                       ( m_bUseLOT      );
  m_cTEncTop.setUseExC                       ( m_bUseEXC      );
  m_cTEncTop.setUseCCP                       ( m_bUseCCP      );
  m_cTEncTop.setUseTMI                       ( m_bUseTMI      );
  m_cTEncTop.setUseLDC                       ( m_bUseLDC      );
  m_cTEncTop.setUseLCT                       ( m_bUseLCT      );
	m_cTEncTop.setUseCIP                       ( m_bUseCIP      );
	m_cTEncTop.setUsePAD                       ( m_bUsePAD      );
	m_cTEncTop.setMaxTrSize										 ( m_uiMaxTrSize  );
  m_cTEncTop.setUseACS                       ( m_bUseACS      );
  m_cTEncTop.setUseQBO                       ( m_bUseQBO      );
	m_cTEncTop.setUseNRF                       ( m_bUseNRF      );
	m_cTEncTop.setUseBQP                       ( m_bUseBQP      );
  m_cTEncTop.setUseHAP                       ( m_bUseHAP      );
  m_cTEncTop.setUseHAB                       ( m_bUseHAB      );
  m_cTEncTop.setUseHME                       ( m_bUseHME      );
	m_cTEncTop.setDIFTap                       ( m_iDIFTap      );
	m_cTEncTop.setDIFTapC                      ( m_iDIFTapC     );
}

Void TAppEncTop::xCreateLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.open( m_pchInputFile,			false );	// read  mode
  m_cTVideoIOYuvReconFile.open( m_pchReconFile,			true  );	// write mode
  m_cTVideoIOBitsFile.openBits( m_pchBitstreamFile,	true  );	// write mode

  // Neo Decoder
  m_cTEncTop.create();
}

Void TAppEncTop::xDestroyLib()
{
  // Video I/O
  m_cTVideoIOYuvInputFile.close();
  m_cTVideoIOYuvReconFile.close();
  m_cTVideoIOBitsFile.closeBits();

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
  TComPicYuv*       pcPicYuvOrg = NULL;
  TComPicYuv*       pcPicYuvRec = NULL;
  TComBitstream*    pcBitstream = NULL;

	// initialize internal class & member variables
  xInitLibCfg();
  xCreateLib();
  xInitLib();

	// main encoder loop
  Int   iNumEncoded = 0;
  Bool  bEos = false;

	while ( !bEos )
  {
		// get buffers
    xGetBuffer( pcPicYuvOrg, pcPicYuvRec, pcBitstream );

		// read input YUV file
    m_cTVideoIOYuvInputFile.read( pcPicYuvOrg, m_aiPad );

		// increase number of received frames
    m_iFrameRcvd++;

		// check end of file
    bEos = ( m_cTVideoIOYuvInputFile.isEof() == 1 ?   true : false  );
    bEos = ( m_iFrameRcvd == m_iFrameToBeEncoded ?    true : bEos   );

		// call encoding function for one frame
    m_cTEncTop.encode( bEos, pcPicYuvOrg, m_cListPicYuvRec, m_cListBitstream, iNumEncoded );

		// write bistream to file if necessary
    if ( iNumEncoded > 0 )
		{
      xWriteOutput( iNumEncoded );
		}
  }

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
Void TAppEncTop::xGetBuffer( TComPicYuv*& rpcPicYuvOrg, TComPicYuv*& rpcPicYuvRec, TComBitstream*& rpcBitStream )
{
  assert ( m_cListPicYuvOrg.size() == m_cListPicYuvRec.size() );

  if ( m_iGOPSize == 0 )
  {
    if (m_cListPicYuvOrg.size() == 0)
    {
      rpcPicYuvOrg = new TComPicYuv;
      rpcPicYuvRec = new TComPicYuv;
      rpcBitStream = new TComBitstream;

      rpcPicYuvOrg->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
      rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
      rpcBitStream->create( (m_iSourceWidth * m_iSourceHeight * 3) >> 1 );

      m_cListPicYuvOrg.pushBack( rpcPicYuvOrg );
      m_cListPicYuvRec.pushBack( rpcPicYuvRec );
      m_cListBitstream.pushBack( rpcBitStream );
    }

    rpcPicYuvOrg = m_cListPicYuvOrg.popFront();
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
    rpcBitStream = m_cListBitstream.popFront();

    m_cListPicYuvOrg.pushBack( rpcPicYuvOrg );
    m_cListPicYuvRec.pushBack( rpcPicYuvRec );
    m_cListBitstream.pushBack( rpcBitStream );

    return;
  }

  // org. buffer
  if ( m_cListPicYuvOrg.size() == (UInt)m_iGOPSize )
  {
    rpcPicYuvOrg = m_cListPicYuvOrg.popFront();
    rpcPicYuvRec = m_cListPicYuvRec.popFront();
    rpcBitStream = m_cListBitstream.popFront();

    rpcBitStream->rewindStreamPacket();
  }
  else
  {
    rpcPicYuvOrg = new TComPicYuv;
    rpcPicYuvRec = new TComPicYuv;
    rpcBitStream = new TComBitstream;

    rpcPicYuvOrg->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
    rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxCUDepth );
    rpcBitStream->create( (m_iSourceWidth * m_iSourceHeight * 3) >> 1 );
  }
  m_cListPicYuvOrg.pushBack( rpcPicYuvOrg );
  m_cListPicYuvRec.pushBack( rpcPicYuvRec );
  m_cListBitstream.pushBack( rpcBitStream );
}

Void TAppEncTop::xDeleteBuffer( )
{
  TComList<TComPicYuv*>::iterator iterPicYuvOrg  = m_cListPicYuvOrg.begin();
  TComList<TComPicYuv*>::iterator iterPicYuvRec  = m_cListPicYuvRec.begin();
  TComList<TComBitstream*>::iterator iterBitstream = m_cListBitstream.begin();

  Int iSize = m_cListPicYuvOrg.size();

  for ( Int i = 0; i < iSize; i++ )
  {
    TComPicYuv*  pcPicYuvOrg  = *(iterPicYuvOrg++);
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    TComBitstream* pcBitstream = *(iterBitstream++);

    pcPicYuvOrg->destroy();
    pcPicYuvRec->destroy();
    pcBitstream->destroy();

    delete pcPicYuvOrg; pcPicYuvOrg = NULL;
    delete pcPicYuvRec; pcPicYuvRec = NULL;
    delete pcBitstream; pcBitstream = NULL;
  }

}

/** \param iNumEncoded	number of encoded frames
 */
Void TAppEncTop::xWriteOutput( Int iNumEncoded )
{
  Int i;

  TComList<TComPicYuv*>::iterator iterPicYuvRec = m_cListPicYuvRec.end();
  TComList<TComBitstream*>::iterator iterBitstream = m_cListBitstream.begin();

  for ( i = 0; i < iNumEncoded; i++ )
  {
    --iterPicYuvRec;
  }

  for ( i = 0; i < iNumEncoded; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    TComBitstream* pcBitstream = *(iterBitstream++);

    m_cTVideoIOYuvReconFile.write( pcPicYuvRec, m_aiPad );
    m_cTVideoIOBitsFile.writeBits( pcBitstream );
  }
}
