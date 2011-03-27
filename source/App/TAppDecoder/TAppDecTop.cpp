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
  m_apcBitstream  = new TComBitstream;
  
  m_apcBitstream->create( BITS_BUF_SIZE );
}

Void TAppDecTop::destroy()
{
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
  Bool bFirstSliceDecoded = true;

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
    Long lLocation          = m_cTVideoIOBitstreamFile.getFileLocation();
    bEos                    = m_cTVideoIOBitstreamFile.readBits( pcBitstream );
    if (bEos)
    {
      if (!bFirstSliceDecoded) m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
      m_cTDecTop.executeDeblockAndAlf( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
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
      break;
    }
    
    // call actual decoding function
#if DCM_SKIP_DECODING_FRAMES
    Bool bNewPicture     = m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
    bFirstSliceDecoded   = true;
    if (bNewPicture)
    {
      m_cTDecTop.executeDeblockAndAlf( bEos, pcBitstream, uiPOC, pcListPic, m_iSkipFrame, m_iPOCLastDisplay);
      if (!m_cTVideoIOBitstreamFile.good()) m_cTVideoIOBitstreamFile.clear();
      m_cTVideoIOBitstreamFile.setFileLocation( lLocation );
      bFirstSliceDecoded = false;
    }
#else
    m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic );
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
        m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(), pcPic->getSlice(0)->getSPS()->getPad() );
      }
      
      // update POC of display order
      m_iPOCLastDisplay = pcPic->getPOC();
      
      // erase non-referenced picture in the reference picture list after display
      if ( !pcPic->getSlice(0)->isReferenced() && pcPic->getReconMark() == true )
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
    }
    
    iterPic++;
  }
}
