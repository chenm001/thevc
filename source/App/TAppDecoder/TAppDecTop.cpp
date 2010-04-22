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
  m_apcOpt				= new TAppOption();
  m_apcBitstream	= new TComBitstream;

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

		// write reconstuction to file
		xWriteOutput( pcListPic );
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
  m_cTVideoIOBitstreamFile.openBits( m_pchBitstreamFile, false);	// read mode

	if ( m_pchReconFile )
	{
		m_cTVideoIOYuvReconFile.open( m_pchReconFile, true );					// write mode
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
    \todo            DYN_REF_FREE should be revised \n
										 no need to alloc pcPicD if no scaling is needed
 */
Void TAppDecTop::xWriteOutput( TComList<TComPic*>* pcListPic )
{
	TComList<TComPic*>::iterator iterPic   = pcListPic->begin();
	TComPicYuv* pcPicD = new TComPicYuv();

	while (iterPic != pcListPic->end())
	{
		TComPic* pcPic = *(iterPic);

		if (pcPic->getReconMark() && pcPic->getPOC() == (m_iPOCLastDisplay + 1))
		{
			// descaling case: CADR || IBDI
			if ( g_uiBitIncrement || g_bUseCADR )
			{
				// allocate destination buffer
				pcPicD->create( pcPic->getPicYuvRec()->getWidth (),
												pcPic->getPicYuvRec()->getHeight(),
												g_uiMaxCUWidth,
												g_uiMaxCUHeight,
												g_uiMaxCUDepth );

				// descaling of frame
				xDeScalePic( pcPic, pcPicD );

				// write to file
				if ( m_pchReconFile )
				{
					m_cTVideoIOYuvReconFile.write( pcPicD, pcPic->getSlice()->getSPS()->getPad() );
				}

				// destroy temporary buffer
				pcPicD->destroy();
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
#else
				pcPic->destroy();
				pcListPic->erase( iterPic );
				iterPic = pcListPic->begin(); // kenti@rewind to the beginning, non-efficient way, have to be revised!
				continue;
#endif
			}
		}

		iterPic++;
	}

	delete pcPicD;
}

/** \param		pcPic		input picture to be descaled
    \retval		pcPicD	output picture which is descaled
		\todo							check validity of clip when IBDI=0
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

	Int   x, y, itmp, itmp2;
  itmp2 = g_iMinCADR << g_uiBitIncrement;

	// ------------------------------------------------------------------------------------------------------------------
	// Luma descaling
	// ------------------------------------------------------------------------------------------------------------------

	// Case #1: both CADR & IBDI are used - clip max is used since the value lies from 0 to 2^{basebit}
	if ( g_bUseCADR && g_uiBitIncrement )
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				itmp = (CADR_INV_L( pRec[x], itmp2, g_iRangeCADR ) + offset) >> g_uiBitIncrement;
				pRecD[x] = ClipMax( itmp );
			}
			pRecD += iStride;
			pRec  += iStride;
		}
	}
	else
	// Case #2: only IBDI is used - clip max is used since the value lies from 0 to 2^{basebit}
	if ( g_uiBitIncrement )
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				itmp = ( pRec[x] + offset ) >> g_uiBitIncrement;
				pRecD[x] = ClipMax( itmp );
			}
			pRecD += iStride;
			pRec  += iStride;
		}
	}
	else
	// Case #3: only CADR is used - no clipping is needed since CADR always decreases the range
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				pRecD[x] = CADR_INV_L( pRec[x], itmp2, g_iRangeCADR );
			}
			pRecD += iStride;
			pRec  += iStride;
		}
	}

	// ------------------------------------------------------------------------------------------------------------------
	// Chroma descaling
	// ------------------------------------------------------------------------------------------------------------------

  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  itmp2 = CADR_OFFSET << g_uiBitIncrement;

	// Case #1: both CADR & IBDI are used - clip max is used since the value lies from 0 to 2^{basebit}
	if ( g_bUseCADR && g_uiBitIncrement )
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				itmp = (CADR_INV_C( pRecCb[x], itmp2, g_iRangeCADR ) + offset) >> g_uiBitIncrement;
				pRecDCb[x] = ClipMax(itmp                );

				itmp = (CADR_INV_C( pRecCr[x], itmp2, g_iRangeCADR ) + offset) >> g_uiBitIncrement;
				pRecDCr[x] = ClipMax(itmp                );
			}
			pRecDCb += iStride;
			pRecCb  += iStride;
			pRecDCr += iStride;
			pRecCr  += iStride;
		}
	}
	else
	// Case #2: only IBDI is used - clip max is used since the value lies from 0 to 2^{basebit}
	if ( g_uiBitIncrement )
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				itmp = ( pRecCb[x] + offset) >> g_uiBitIncrement;
				pRecDCb[x] = ClipMax(itmp                );

				itmp = ( pRecCr[x] + offset) >> g_uiBitIncrement;
				pRecDCr[x] = ClipMax(itmp                );
			}
			pRecDCb += iStride;
			pRecCb  += iStride;
			pRecDCr += iStride;
			pRecCr  += iStride;
		}
	}
	else
	// Case #3: only CADR is used - no clipping is needed since CADR always decreases the range
	{
		for( y = iHeight-1; y >= 0; y-- )
		{
			for( x = iWidth-1; x >= 0; x-- )
			{
				pRecDCb[x] = CADR_INV_C( pRecCb[x], itmp2, g_iRangeCADR );
				pRecDCr[x] = CADR_INV_C( pRecCr[x], itmp2, g_iRangeCADR );
			}
			pRecDCb += iStride;
			pRecCb  += iStride;
			pRecDCr += iStride;
			pRecCr  += iStride;
		}
	}
}
