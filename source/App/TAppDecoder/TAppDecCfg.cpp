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

/** \file     TAppDecCfg.cpp
    \brief    Decoder configuration class
*/

#include "TAppDecCfg.h"

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param argc number of arguments
    \param argv array of arguments
 */
Bool TAppDecCfg::parseCfg( Int argc, Char* argv[] )
{
  // set preferences
  m_apcOpt->setVerbose();
  m_apcOpt->autoUsagePrint(true);
  
  // set usage
  m_apcOpt->addUsage( "options: (if only -b is specified, YUV writing is skipped)" );
  m_apcOpt->addUsage( "  -b  bitstream file name" );
  m_apcOpt->addUsage( "  -o  decoded YUV output file name" );
#if DCM_SKIP_DECODING_FRAMES
  m_apcOpt->addUsage( "  -s  number of frames to skip before random access" );
#endif
  
  // set command line option strings/characters
  m_apcOpt->setCommandOption( 'b' );
  m_apcOpt->setCommandOption( 'o' );
#if DCM_SKIP_DECODING_FRAMES
  m_apcOpt->setCommandOption( 's' );
#endif
  
  // command line parsing
  m_apcOpt->processCommandArgs( argc, argv );
  if( ! m_apcOpt->hasOptions() || !m_apcOpt->getValue( 'b' ) )
  {
    m_apcOpt->printUsage();
    delete m_apcOpt;
    m_apcOpt = NULL;
    return false;
  }
  
  // set configuration
  xSetCfgCommand( m_apcOpt );
  
  return true;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/** \param pcOpt option handling class
 */
Void TAppDecCfg::xSetCfgCommand   ( TAppOption* pcOpt )
{
  m_pchBitstreamFile = m_pchReconFile = NULL;
  
  if ( pcOpt->getValue( 'b' ) ) m_pchBitstreamFile = pcOpt->getValue( 'b' );
  if ( pcOpt->getValue( 'o' ) ) m_pchReconFile     = pcOpt->getValue( 'o' );
#if DCM_SKIP_DECODING_FRAMES
  m_iSkipFrame = 0;
  if ( pcOpt->getValue( 's' ) ) m_iSkipFrame       = atoi(pcOpt->getValue( 's' ));
#endif
}


