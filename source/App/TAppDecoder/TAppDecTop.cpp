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

/** \file     TAppDecTop.cpp
    \brief    Decoder application class
*/

#include <list>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppDecTop.h"

// ====================================================================================================================
// Local constants
// ====================================================================================================================

/// initial bitstream buffer size
/// should be large enough for parsing SPS
/// resized as a function of picture size after parsing SPS
#define BITS_BUF_SIZE 65536

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppDecTop::TAppDecTop()
{
  ::memset (m_abDecFlag, 0, sizeof (m_abDecFlag));
  m_iPOCLastDisplay  = -1;
}

Void TAppDecTop::create()
{
  m_apcOpt        = new TAppOption();
  m_apcBitstream  = new TComBitstream;
  
  m_apcBitstream->create( BITS_BUF_SIZE );
}

Void TAppDecTop::destroy()
{
  if ( m_apcOpt )
  {
    delete m_apcOpt;
    m_apcOpt = NULL;
  }
  if ( m_apcBitstream )
  {
    m_apcBitstream->destroy();
    delete m_apcBitstream;
    m_apcBitstream = NULL;
  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal class
 - until the end of the bitstream, call decoding function in TDecTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppDecTop::decode()
{
  TComBitstream*      pcBitstream = m_apcBitstream;
  UInt                uiPOC;
  TComList<TComPic*>* pcListPic;
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
  Int                 iLastLoopLocation = 0;
#endif
  
  // create & initialize internal classes
  xCreateDecLib();
  xInitDecLib  ();
#if DCM_SKIP_DECODING_FRAMES
  m_iPOCLastDisplay += m_iSkipFrame;      // set the last displayed POC correctly for skip forward.
#endif

  // main decoder loop
  Bool  bEos        = false;
  bool recon_opened = false; // reconstruction file not yet opened. (must be performed after SPS is seen)
  Bool resizedBitstreamBuffer = false;
  
  while ( !bEos )
  {
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
    Int       iSliceCountInPicture  = 0;
    Bool      bEosDetected          = false;
    Bool      bEofDetected          = false;
    TComSPS*  pcSPS                 = m_cTDecTop.getSPS();
    Long*     alFileByteLocationSlicesInPicture;
    if (pcSPS==NULL)
    {
      // SPS not read yet so it is not known how many slices can occur within a picture
      alFileByteLocationSlicesInPicture = new Long [10]; // just has to be >= 1
    }
    else
    {
      // Determine the number of LCUs per picture and allocate memory for writing slice locations
      UInt uiWidthInCU       = ( pcSPS->getWidth()  % pcSPS->getMaxCUWidth()  ) ? pcSPS->getWidth() /pcSPS->getMaxCUWidth()  + 1 : pcSPS->getWidth() /pcSPS->getMaxCUWidth() ;
      UInt uiHeightInCU      = ( pcSPS->getHeight() % pcSPS->getMaxCUHeight() ) ? pcSPS->getHeight()/pcSPS->getMaxCUHeight() + 1 : pcSPS->getHeight()/pcSPS->getMaxCUHeight();
      UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;
      alFileByteLocationSlicesInPicture = new Long [uiNumCUsInFrame];
    }

    // Determine location of slices for a picture in the file
    do 
    {
      alFileByteLocationSlicesInPicture[ iSliceCountInPicture++ ] = iLastLoopLocation;
      Int iLocation     = m_cTVideoIOBitstreamFile.findNextStartCodeFastLookup( pcBitstream );
      iLastLoopLocation = iLocation;
      if (iLocation<0)
      {
        bEofDetected = true;
        break;
      }
    }
    while (!pcBitstream->getLastSliceEncounteredInPicture());

    if(bEosDetected)
    {
      break;
    }

    // Set file pointer and perform out-of-order slice decoding
    for (Int iSliceProcessed = 0; iSliceProcessed < iSliceCountInPicture; iSliceProcessed++)
    {
#if SHARP_ENTROPY_SLICE
      // Disable out-of-order decompression since crossing reconstruction slice boundaries causes problems
      Int iSliceIndxToDecode     = iSliceProcessed;
#else
      // Slice1 Slice0 Slice3 Slice2 Slice5 Slice4 ...
      Int iSliceIndxToDecode     = (iSliceProcessed%2==1) ? (iSliceProcessed-1) : (iSliceProcessed+1);
#endif
      iSliceIndxToDecode         = (iSliceIndxToDecode<0) ? 0 : ( (iSliceIndxToDecode > (iSliceCountInPicture-1)) ? (iSliceCountInPicture-1) : iSliceIndxToDecode );

      m_cTVideoIOBitstreamFile.setFileLocation( alFileByteLocationSlicesInPicture[ iSliceIndxToDecode ] );
      m_cTVideoIOBitstreamFile.readBits( pcBitstream );
      Long lLocalLastFileLocation = m_cTVideoIOBitstreamFile.getFileLocation();
      if (lLocalLastFileLocation < 0 )
      {
        m_cTVideoIOBitstreamFile.rewindFile();
      }

      pcBitstream->setSliceProcessed ( iSliceProcessed );
      pcBitstream->setFirstSliceEncounteredInPicture( (iSliceProcessed==0) ? true : false );
      pcBitstream->setLastSliceEncounteredInPicture( (iSliceProcessed==iSliceCountInPicture-1) ? true : false );
      // call actual decoding function
#if DCM_SKIP_DECODING_FRAMES
      m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
#else
      m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic );
#endif
    }

    delete [] alFileByteLocationSlicesInPicture;

    if (bEosDetected)
    {
      bEos = true;
      break;
    }
#else
    bEos = m_cTVideoIOBitstreamFile.readBits( pcBitstream );
    if (bEos)
    {
      break;
    }
    
    // call actual decoding function
#if DCM_SKIP_DECODING_FRAMES
    m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
#else
    m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic );
#endif
#endif
    
    if (!resizedBitstreamBuffer)
    {
      TComSPS *sps = m_cTDecTop.getSPS();
      if (sps)
      {
        pcBitstream->destroy();
        pcBitstream->create(sps->getWidth() * sps->getHeight() * 2);
        resizedBitstreamBuffer = true;
      }
    }
    
    if( pcListPic )
    {
      if ( m_pchReconFile && !recon_opened )
      {
        if ( m_outputBitDepth == 0 )
          m_outputBitDepth = g_uiBitDepth + g_uiBitIncrement;
        m_cTVideoIOYuvReconFile.open( m_pchReconFile, true, m_outputBitDepth, g_uiBitDepth + g_uiBitIncrement ); // write mode
        recon_opened = true;
      }
      // write reconstuction to file
      xWriteOutput( pcListPic );
    }
#if AD_HOC_SLICES && AD_HOC_SLICES_TEST_OUTOFORDER_DECOMPRESS
    if (bEofDetected)
    {
      break;
    }
#endif
  }
  
  // delete buffers
  m_cTDecTop.deletePicBuffer();
  
  // destroy internal classes
  xDestroyDecLib();
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TAppDecTop::xCreateDecLib()
{
  // open bitstream file
  m_cTVideoIOBitstreamFile.openBits( m_pchBitstreamFile, false);  // read mode
  
  // create decoder class
  m_cTDecTop.create();
}

Void TAppDecTop::xDestroyDecLib()
{
  // close bitstream file
  m_cTVideoIOBitstreamFile.closeBits();
  
  if ( m_pchReconFile )
  {
    m_cTVideoIOYuvReconFile. close();
  }
  
  // destroy decoder class
  m_cTDecTop.destroy();
}

Void TAppDecTop::xInitDecLib()
{
  // initialize decoder class
  m_cTDecTop.init();
}

/** \param pcListPic list of pictures to be written to file
    \param bFirst    first picture?
    \todo            DYN_REF_FREE should be revised
 */
Void TAppDecTop::xWriteOutput( TComList<TComPic*>* pcListPic )
{
  TComList<TComPic*>::iterator iterPic   = pcListPic->begin();
  
  while (iterPic != pcListPic->end())
  {
    TComPic* pcPic = *(iterPic);
    
    if ( pcPic->getReconMark() && pcPic->getPOC() == (m_iPOCLastDisplay + 1) )
    {
      // write to file
      if ( m_pchReconFile )
      {
#if AD_HOC_SLICES
        m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(), pcPic->getSlice(0)->getSPS()->getPad() );
#else
        m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(), pcPic->getSlice()->getSPS()->getPad() );
#endif
      }
      
      // update POC of display order
      m_iPOCLastDisplay = pcPic->getPOC();
      
      // erase non-referenced picture in the reference picture list after display
#if AD_HOC_SLICES
      if ( !pcPic->getSlice(0)->isReferenced() && pcPic->getReconMark() == true )
#else
      if ( !pcPic->getSlice()->isReferenced() && pcPic->getReconMark() == true )
#endif
      {
#if !DYN_REF_FREE
        pcPic->setReconMark(false);
        
        // mark it should be extended later
        pcPic->getPicYuvRec()->setBorderExtension( false );
        
#else
        pcPic->destroy();
        pcListPic->erase( iterPic );
        iterPic = pcListPic->begin(); // to the beginning, non-efficient way, have to be revised!
        continue;
#endif
      }

#if AD_HOC_SLICES && SHARP_ENTROPY_SLICE
      if(pcPic->getNumAllocatedSlice() != 1)
      {
        pcPic->clearSliceBuffer();
      }
#endif

    }
    
    iterPic++;
  }
}
