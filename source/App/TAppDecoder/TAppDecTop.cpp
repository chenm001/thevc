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

/// maximum bitstream buffer Size per 1 picture (1920*1080*1.5)
/** \todo fix this value according to suitable value
 */
#define BITS_BUF_SIZE     3110400

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
  Bool                bAlloc = false;

  // create & initialize internal classes
  xCreateDecLib();
  xInitDecLib  ();

  // main decoder loop
  Bool  bEos        = false;
  while ( !bEos )
  {
    bEos = m_cTVideoIOBitstreamFile.readBits( pcBitstream );
    if (bEos)
    {
      break;
    }

    // call actual decoding function
    m_cTDecTop.decode( bEos, pcBitstream, uiPOC, pcListPic );

    if( pcListPic )
    {
      // write reconstuction to file
      xWriteOutput( pcListPic, bAlloc );
    }
  }

  // delete temporary buffer
  if ( bAlloc )
  {
    m_cTempPicYuv.destroy();
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

  if ( m_pchReconFile )
  {
    m_cTVideoIOYuvReconFile.open( m_pchReconFile, true );         // write mode
  }

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
    \aram  bFirst    first picture?
    \todo            DYN_REF_FREE should be revised
 */
Void TAppDecTop::xWriteOutput( TComList<TComPic*>* pcListPic, Bool& rbAlloc )
{
  TComList<TComPic*>::iterator iterPic   = pcListPic->begin();

  while (iterPic != pcListPic->end())
  {
    TComPic* pcPic = *(iterPic);

    if ( pcPic->getReconMark() && pcPic->getPOC() == (m_iPOCLastDisplay + 1) )
    {
      // descaling case: IBDI
      if ( g_uiBitIncrement )
      {
        TComPicYuv* pcPicD = &m_cTempPicYuv;

        // allocate temporary buffer if first time
        if ( !rbAlloc )
        {
          m_cTempPicYuv.create( pcPic->getPicYuvRec()->getWidth (),
                                pcPic->getPicYuvRec()->getHeight(),
                                g_uiMaxCUWidth,
                                g_uiMaxCUHeight,
                                g_uiMaxCUDepth );
          rbAlloc = true;
        }

        // descaling of frame
        xDeScalePic( pcPic, pcPicD );

        // write to file
        if ( m_pchReconFile )
        {
          m_cTVideoIOYuvReconFile.write( pcPicD, pcPic->getSlice()->getSPS()->getPad() );
        }
      }
      // normal case
      else
      {
        // write to file
        if ( m_pchReconFile )
        {
          m_cTVideoIOYuvReconFile.write( pcPic->getPicYuvRec(), pcPic->getSlice()->getSPS()->getPad() );
        }
      }

      // update POC of display order
      m_iPOCLastDisplay = pcPic->getPOC();

      // erase non-referenced picture in the reference picture list after display
      if ( !pcPic->getSlice()->isReferenced() && pcPic->getReconMark() == true )
      {
#if !DYN_REF_FREE
        pcPic->setReconMark(false);

        // mark it should be extended later
        pcPic->getPicYuvRec()->setBorderExtension( false );
#if HHI_INTERP_FILTER
        pcPic->getPicYuvRecFilt()->setBorderExtension( false );
#endif

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

/** \param    pcPic   input picture to be descaled
    \retval   pcPicD  output picture which is descaled
 */
Void TAppDecTop::xDeScalePic( TComPic* pcPic, TComPicYuv* pcPicD )
{
  Pel*  pRecD   = pcPicD->getLumaAddr();
  Pel*  pRecDCb = pcPicD->getCbAddr();
  Pel*  pRecDCr = pcPicD->getCrAddr();
  Pel*  pRec    = pcPic->getPicYuvRec()->getLumaAddr();
  Pel*  pRecCb  = pcPic->getPicYuvRec()->getCbAddr();
  Pel*  pRecCr  = pcPic->getPicYuvRec()->getCrAddr();
  Int   iStride = pcPic->getStride();

  Int   iWidth  = pcPic->getPicYuvRec()->getWidth();
  Int   iHeight = pcPic->getPicYuvRec()->getHeight();
  Int   offset  = (g_uiBitIncrement>0)?(1<<(g_uiBitIncrement-1)):0;

  Int   x, y;

  // ------------------------------------------------------------------------------------------------------------------
  // Luma descaling
  // ------------------------------------------------------------------------------------------------------------------

  for( y = iHeight-1; y >= 0; y-- )
  {
    for( x = iWidth-1; x >= 0; x-- )
    {
#if IBDI_NOCLIP_RANGE
      pRecD[x] = ( pRec[x] + offset ) >> g_uiBitIncrement;
#else
      pRecD[x] = ClipMax( ( pRec[x] + offset ) >> g_uiBitIncrement );
#endif
    }
    pRecD += iStride;
    pRec  += iStride;
  }

  // ------------------------------------------------------------------------------------------------------------------
  // Chroma descaling
  // ------------------------------------------------------------------------------------------------------------------

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;

  for( y = iHeight-1; y >= 0; y-- )
  {
    for( x = iWidth-1; x >= 0; x-- )
    {
#if IBDI_NOCLIP_RANGE
      pRecDCb[x] = ( pRecCb[x] + offset) >> g_uiBitIncrement;
      pRecDCr[x] = ( pRecCr[x] + offset) >> g_uiBitIncrement;
#else
      pRecDCb[x] = ClipMax( ( pRecCb[x] + offset) >> g_uiBitIncrement );
      pRecDCr[x] = ClipMax( ( pRecCr[x] + offset) >> g_uiBitIncrement );
#endif
    }
    pRecDCb += iStride;
    pRecCb  += iStride;
    pRecDCr += iStride;
    pRecCr  += iStride;
  }
}

