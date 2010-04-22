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

/** \file			TAppEncCfg.cpp
    \brief		Handle encoder configuration parameters
*/

#include "TAppEncCfg.h"

// ====================================================================================================================
// Local constants
// ====================================================================================================================

/// max value of source padding size
/** \todo replace it by command line option
 */
#define MAX_PAD_SIZE                16

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncCfg::TAppEncCfg()
{
	// initialize member variables
  m_pchInputFile                  = NULL;
  m_pchBitstreamFile              = NULL;
  m_pchReconFile                  = NULL;
  m_pchdQPFile									  = NULL;
  m_iFrameRate                    = 0;
  m_iFrameSkip                    = 0;
  m_iSourceWidth                  = 0;
  m_iSourceHeight                 = 0;
  m_iFrameToBeEncoded             = 0;
  m_iIntraPeriod                  = -1;
  m_iGOPSize                      = 1;
	m_iRateGOPSize									= m_iGOPSize;
  m_iNumOfReference               = 1;
	m_iNumOfReferenceB_L0						= 1;
	m_iNumOfReferenceB_L1						= 1;
  m_iQP                           = 30;
	m_fQP                           = m_iQP;
  m_aiTLayerQPOffset[0]						= ( MAX_QP + 1 );
  m_aiTLayerQPOffset[1]						= ( MAX_QP + 1 );
  m_aiTLayerQPOffset[2]						= ( MAX_QP + 1 );
  m_aiTLayerQPOffset[3]						= ( MAX_QP + 1 );
  m_aiPad[0]											= 0;
  m_aiPad[1]											= 0;
	m_aidQP                         = NULL;
  m_bHierarchicalCoding           = true;

  m_uiMinTrDepth                  = 0;
  m_uiMaxTrDepth                  = 1;
  m_iSymbolMode                   = 1;
	m_pchGeneratedReferenceMode     = NULL;
  m_bLoopFilterDisable            = false;
  m_iLoopFilterAlphaC0Offset      = 0;
  m_iLoopFilterBetaOffset         = 0;
  m_iFastSearch                   = 1;
  m_iSearchRange                  = 96;
  m_iMaxDeltaQP                   = 0;
	m_uiMaxTrSize										= 64;
	m_iMinCADR											= 0;
	m_iMaxCADR											= 255;
	m_iDIFTap												= 12;
	m_iDIFTapC											= 6;
  m_iProfileIdx                   = 1;

	// initialize member variables from default profile index
  xSetProfile();
}

TAppEncCfg::~TAppEncCfg()
{
	if ( m_aidQP )
	{
		delete[] m_aidQP;
	}
}

Void TAppEncCfg::create()
{
  m_apcOpt = new TAppOption();
}

Void TAppEncCfg::destroy()
{
  delete m_apcOpt;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param	argc				number of arguments
		\param	argv				array of arguments
		\retval							true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, Char* argv[] )
{
  // '-c' option is used for specifying configuration file
  m_apcOpt->setCommandOption( 'c' );

  // register configuration strings
  m_apcOpt->setFileOption( "InputFile"								);
  m_apcOpt->setFileOption( "BitstreamFile"						);
  m_apcOpt->setFileOption( "ReconFile"								);
  m_apcOpt->setFileOption( "FrameRate"								);
  m_apcOpt->setFileOption( "FrameSkip"								);
  m_apcOpt->setFileOption( "SourceWidth"							);
  m_apcOpt->setFileOption( "SourceHeight"							);
  m_apcOpt->setFileOption( "FrameToBeEncoded"					);
  m_apcOpt->setFileOption( "IntraPeriod"							);
  m_apcOpt->setFileOption( "GOPSize"									);
  m_apcOpt->setFileOption( "NumOfReference"						);
	m_apcOpt->setFileOption( "NumOfReferenceB_L0"				);
	m_apcOpt->setFileOption( "NumOfReferenceB_L1"				);
  m_apcOpt->setFileOption( "QP"												);
  m_apcOpt->setFileOption( "HierarchicalCoding"				);
  m_apcOpt->setFileOption( "FastSearch"								);
  m_apcOpt->setFileOption( "SearchRange"							);
  m_apcOpt->setFileOption( "MaxDeltaQP"								);
	m_apcOpt->setFileOption( "HadamardME"								);
  m_apcOpt->setFileOption( "PFI"											);
  m_apcOpt->setFileOption( "LowDelayCoding" );

  m_apcOpt->setFileOption( "MaxCUWidth"								);
  m_apcOpt->setFileOption( "MaxCUHeight"							);
  m_apcOpt->setFileOption( "MaxPartitionDepth"				);

  m_apcOpt->setFileOption( "ALF"											);
  m_apcOpt->setFileOption( "FilterLength"							);
  m_apcOpt->setFileOption( "NumOfQBits"								);
  m_apcOpt->setFileOption( "ColorNumber"							);
  m_apcOpt->setFileOption( "FilterType"								);
	m_apcOpt->setFileOption( "ACC"											);
  m_apcOpt->setFileOption( "BitDepth"									);
  m_apcOpt->setFileOption( "BitIncrement"							);
	m_apcOpt->setFileOption( "JMQP"											);
	m_apcOpt->setFileOption( "JMLAMBDA"									);
	m_apcOpt->setFileOption( "AMVP"											);
	m_apcOpt->setFileOption( "ROT"											);
  m_apcOpt->setFileOption( "ADI"											);
	m_apcOpt->setFileOption( "AMP"											);
	m_apcOpt->setFileOption( "FAM"											);
  m_apcOpt->setFileOption( "MVAC"											);
  m_apcOpt->setFileOption( "MVACRD"										);
  m_apcOpt->setFileOption( "DeltaQpRD"								);
	m_apcOpt->setFileOption( "MPI"											);
	m_apcOpt->setFileOption( "RNG"											);
  m_apcOpt->setFileOption( "RDOQ"											);
  m_apcOpt->setFileOption( "LogicalTR"								);
  m_apcOpt->setFileOption( "ExtremeCorrection"				);
  m_apcOpt->setFileOption( "GeneratedReferenceMode"		);
	m_apcOpt->setFileOption( "CIP"											);
	m_apcOpt->setFileOption( "CADR"											);
	m_apcOpt->setFileOption( "MinCADR"									);
	m_apcOpt->setFileOption( "MaxCADR"									);
	m_apcOpt->setFileOption( "QBO"											);
	m_apcOpt->setFileOption( "NRF"											);
	m_apcOpt->setFileOption( "BQP"											);
  m_apcOpt->setFileOption( "HAP"											);
  m_apcOpt->setFileOption( "HAB"											);
  m_apcOpt->setFileOption( "HME"											);
	m_apcOpt->setFileOption( "DIF"											);
	m_apcOpt->setFileOption( "DIFTap"										);
	m_apcOpt->setFileOption( "DIFTapC"									);
  m_apcOpt->setFileOption( "LoopFilterDisable"				);
  m_apcOpt->setFileOption( "LoopFilterAlphaC0Offset"	);
  m_apcOpt->setFileOption( "LoopFilterBetaOffset"			);
  m_apcOpt->setFileOption( "SymbolMode"								);

  // command line parsing
  m_apcOpt->processCommandArgs( argc, argv );
  if( !m_apcOpt->hasOptions() || !m_apcOpt->getValue( 'c' ) )
  {
    xPrintUsage();
    return false;
  }

  // configuration file parsing
  m_apcOpt->processFile( m_apcOpt->getValue( 'c' ) );

  // set configuration from option handling class
  xSetCfgFile			( m_apcOpt	 );
  xSetCfgCommand  ( argc, argv );

	// compute source padding size
  if ( m_bUsePAD )
  {
    if ( m_iSourceWidth%MAX_PAD_SIZE )
    {
      m_aiPad[0] = (m_iSourceWidth/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceWidth;
    }

    if ( m_iSourceHeight%MAX_PAD_SIZE )
    {
      m_aiPad[1] = (m_iSourceHeight/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceHeight;
    }
  }
  m_iSourceWidth  += m_aiPad[0];
  m_iSourceHeight += m_aiPad[1];

	// allocate slice-based dQP values
	m_aidQP = new Int[ m_iFrameToBeEncoded + m_iRateGOPSize + 1 ];
	::memset( m_aidQP, 0, sizeof(Int)*( m_iFrameToBeEncoded + m_iRateGOPSize + 1 ) );

	// handling of floating-point QP values
	// if QP is not integer, sequence is split into two sections having QP and QP+1
	m_iQP = (Int)( m_fQP );
	if ( m_iQP < m_fQP )
	{
		Int iSwitchPOC = (Int)( m_iFrameToBeEncoded - (m_fQP - m_iQP)*m_iFrameToBeEncoded + 0.5 );

		iSwitchPOC = (Int)( (Double)iSwitchPOC / m_iRateGOPSize + 0.5 )*m_iRateGOPSize;
		for ( Int i=iSwitchPOC; i<m_iFrameToBeEncoded + m_iRateGOPSize + 1; i++ )
		{
			m_aidQP[i] = 1;
		}
	}

	// reading external dQP description from file
	if ( m_pchdQPFile )
	{
		FILE* fpt=fopen( m_pchdQPFile, "r" );
		if ( fpt )
		{
			Int iValue;
			Int iPOC = 0;
			while ( iPOC < m_iFrameToBeEncoded )
			{
				if ( fscanf(fpt, "%d", &iValue ) == EOF ) break;
				m_aidQP[ iPOC ] = iValue;
				iPOC++;
			}
			fclose(fpt);
		}
	}

  // check validity of input parameters
  xCheckParameter();

	// set global varibles
	xSetGlobal();

	// print-out parameters
	xPrintParameter();

  return true;
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Void TAppEncCfg::xCheckParameter()
{
	// check range of parameters
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_iFrameSkip < 0,                                                           "Frame Skipping must be more than 0" );
  xConfirmPara( m_iFrameToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 1" );
  xConfirmPara( m_iGOPSize < 1 ,                                                            "GOP Size must be more than 1" );
  xConfirmPara( m_iGOPSize > 1 &&  m_iGOPSize % 2,                                          "GOP Size must be a multiple of 2, if GOP Size is greater than 1" );
  xConfirmPara( (m_iIntraPeriod > 0 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod == 0, "Intra period must be more than GOP size, or -1 , not 0" );
  xConfirmPara( m_iQP < 0 || m_iQP > 51,                                                    "QP exceeds supported range (0 to 51)" );
  xConfirmPara( m_iLoopFilterAlphaC0Offset < -26 || m_iLoopFilterAlphaC0Offset > 26,        "Loop Filter Alpha Offset exceeds supported range (-26 to 26)" );
  xConfirmPara( m_iLoopFilterBetaOffset < -26 || m_iLoopFilterBetaOffset > 26,              "Loop Filter Beta Offset exceeds supported range (-26 to 26)");
  xConfirmPara( m_iFastSearch < 0 || m_iFastSearch > 2,                                     "Fast Search Mode is not supported value (0:Full search  1:Diamond  2:PMVFAST)" );
  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
  xConfirmPara( m_iMaxDeltaQP > 7,                                                          "Absolute Delta QP exceeds supported range (0 to 7)" );
  xConfirmPara( m_iFrameToBeEncoded != 1 && m_iFrameToBeEncoded <= m_iGOPSize,              "Total Number of Frames to be encoded must be larger than GOP size");
  xConfirmPara( (m_uiMaxCUWidth  >> m_uiMaxCUDepth) < 4,                                    "Minimum partition width size should be larger than or equal to 4");
  xConfirmPara( (m_uiMaxCUHeight >> m_uiMaxCUDepth) < 4,                                    "Minimum partition height size should be larger than or equal to 4");
  xConfirmPara( m_uiMaxTrDepth < m_uiMinTrDepth,                                            "Max. transform depth should be equal or larger than Min. transform depth");
	xConfirmPara( m_uiMaxTrSize > 64,                                                         "Max physical transform size should be equal or smaller than 64");
  xConfirmPara( m_uiMaxTrSize < 2,                                                          "Max physical transform size should be equal or larger than 2");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)))!=0,             "Frame width should be multiple of double size of minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)))!=0,             "Frame height should be multiple of double size of minimum CU size");
	xConfirmPara( m_iDIFTap  != 4 && m_iDIFTap  != 6 && m_iDIFTap  != 8 && m_iDIFTap  != 10 && m_iDIFTap  != 12, "DIF taps 4, 6, 8, 10 and 12 are supported");
	xConfirmPara( m_iDIFTapC != 2 && m_iDIFTapC != 4 && m_iDIFTapC != 6 && m_iDIFTapC !=  8 && m_iDIFTapC != 10 && m_iDIFTapC != 12, "DIF taps 2, 4, 6, 8, 10 and 12 are supported");

	// max CU width and height should be power of 2
  UInt ui = m_uiMaxCUWidth;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Width should be 2^n");
  }
  ui = m_uiMaxCUHeight;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Height should be 2^n");
  }

	// RDOQ only supports in SBAC
  if( !m_bUseSBACRD ) m_bUseRDOQ = false;
}

/** \todo use of global variables	should be removed later
 */
Void TAppEncCfg::xSetGlobal()
{
	// set max CU width & height
  g_uiMaxCUWidth  = m_uiMaxCUWidth;
  g_uiMaxCUHeight = m_uiMaxCUHeight;

	// compute actual CU depth with respect to config depth and max transform size
  g_uiAddCUDepth  = 0;
  if( !m_bUseLOT && ((m_uiMaxCUWidth>>(m_uiMaxCUDepth-1)) > (m_uiMaxTrSize>>m_uiMaxTrDepth)) )
  {
    while( (m_uiMaxCUWidth>>(m_uiMaxCUDepth-1)) > (m_uiMaxTrSize<<g_uiAddCUDepth) ) g_uiAddCUDepth++;
  }
  m_uiMaxCUDepth += g_uiAddCUDepth;
  g_uiAddCUDepth++;
  g_uiMaxCUDepth = m_uiMaxCUDepth;

	// set internal bit-depth and constants
  g_uiBitDepth     = m_uiBitDepth;       // base bit-depth
  g_uiBitIncrement = m_uiBitIncrement;   // increments
  g_uiIBDI_MAX     = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);
  g_uiBASE_MAX		 = ((1<<(g_uiBitDepth))-1);

	// set CADR constants
	g_bUseCADR       = m_bUseCADR;
	if ( g_bUseCADR )
	{
		g_iMinCADR       = m_iMinCADR;
		g_iMaxCADR       = m_iMaxCADR;
		g_iRangeCADR		 = ( m_iMaxCADR - m_iMinCADR + 1 );
	}
}

Void TAppEncCfg::xSetProfile()
{
  switch(m_iProfileIdx)
  {

	// --------------------------------------------------------------------------------------------------------------
	// Profile 0 - low complexity profile
	// --------------------------------------------------------------------------------------------------------------

  case 0:
    {
			// basic unit size
      m_uiMaxCUWidth                  = 64;
      m_uiMaxCUHeight                 = 64;
      m_uiMaxCUDepth                  = 4;

			// encoder tools (used)
      m_bUseACC												= true;
      m_bUseSBACRD                    = true;
      m_bUseJMQP											= true;
      m_bUseJMLAMBDA									= true;
      m_bUseHADME											= true;
      m_bUseRDOQ                      = true;
			m_bUseNRF												= true;
      m_bUseASR                       = false;

			// encoder tools (not-used)
      m_bUseMVACRD                    = false;
      m_uiDeltaQpRD                   = 0;
			m_bUseGPB												= false;
			m_bUseFAM												= false;
      m_bUseLDC                       = false;
			m_bUseQBO												= false;
			m_bUseBQP												= false;
			m_iLDMode												= -1;
      m_bUsePAD                       = false;

			// decoder tools (used)
      m_bUseADI                       = true;
      m_bUseDIF												= true;
			m_iDIFTap												= 6;
			m_iDIFTapC                      = 2;
      m_bUseACS                       = true;
      m_bUseAMVP											= true;
      m_bUseIMR												= true;
      m_uiBitDepth                    = 8;
      m_uiBitIncrement                = 4;
			m_bUseCIP                       = true;

			// decoder tools (not-used)
      m_bUseEXC                       = false;
      m_bUseCCP                       = false;
			m_bUseCADR                      = false;
      m_bUseALF                       = false;
      m_bUseLOT                       = false;
      m_bUseMVAC                      = false;
      m_bUseROT												= false;
      m_bUseMPI												= false;
			m_bUseRNG												= false;
			m_bUseAMP												= false;
			m_bUseSHV												= false;
      m_bUseTMI                       = false;
      m_bUseLCT                       = false;
      m_bUseHAP												= false;
      m_bUseHAB												= false;
      m_bUseHME												= false;
    }
    break;

	// --------------------------------------------------------------------------------------------------------------
	// Profile 1 - normal complexity profile
	// --------------------------------------------------------------------------------------------------------------

  case 1:
    {
			// basic unit size
      m_uiMaxCUWidth                  = 128;
      m_uiMaxCUHeight                 = 128;
      m_uiMaxCUDepth                  = 5;

			// encoder tools (used)
      m_bUseACC												= true;
      m_bUseSBACRD                    = true;
      m_bUseJMQP											= true;
      m_bUseJMLAMBDA									= true;
      m_bUseHADME											= true;
      m_bUseRDOQ                      = true;
			m_bUseNRF												= true;
      m_bUseACS                       = true;
      m_bUseASR                       = false;

			// encoder tools (not used)
      m_bUseMVACRD                    = false;
      m_uiDeltaQpRD                    = 0;
			m_bUseGPB												= false;
			m_bUseFAM												= false;
      m_bUseLDC                       = false;
			m_bUseQBO												= false;
			m_bUseBQP												= false;
			m_iLDMode												= -1;
      m_bUsePAD                       = false;

			// decoder tools (used)
      m_uiBitDepth                    = 8;
      m_uiBitIncrement                = 4;
      m_bUseADI                       = true;
      m_bUseEXC                       = true;
      m_bUseCCP                       = true;
			m_bUseCADR                      = true;
      m_bUseALF                       = true;
      m_bUseAMVP											= true;
      m_bUseIMR												= true;
      m_bUseDIF												= true;
      m_bUseLOT                       = true;
      m_bUseROT												= true;
      m_bUseMPI												= true;
			m_bUseAMP												= true;
      m_bUseTMI                       = true;
			m_iDIFTap												= 12;
			m_iDIFTapC											= 6;

			// decoder tools (not used)
      m_bUseMVAC                      = false;
			m_bUseRNG												= false;
			m_bUseSHV												= false;
      m_bUseLCT                       = false;
			m_bUseCIP                       = false;
      m_bUseHAP												= true;
      m_bUseHAB												= true;
      m_bUseHME												= true;
    }
    break;

	// --------------------------------------------------------------------------------------------------------------
	// Profile 2 - experimental profile for testing
	// --------------------------------------------------------------------------------------------------------------

  case 2:
    {
			// basic unit size
			m_uiMaxCUWidth                  = 128;
      m_uiMaxCUHeight                 = 128;
      m_uiMaxCUDepth                  = 5;

			// encoder tools (used)
      m_bUseACC												= true;
      m_bUseSBACRD                    = true;
      m_bUseJMQP											= true;
      m_bUseJMLAMBDA									= true;
      m_bUseHADME											= true;
      m_bUseRDOQ                      = true;
			m_bUseNRF												= true;
      m_bUseACS                       = true;
      m_bUseASR                       = false;

			// encoder tools (not used)
      m_bUseMVACRD                    = false;
      m_uiDeltaQpRD                   = 0;
			m_bUseGPB												= false;
			m_bUseFAM												= false;
      m_bUseLDC                       = false;
			m_bUseQBO												= false;
			m_bUseBQP												= false;
			m_iLDMode												= -1;
      m_bUsePAD                       = false;

			// decoder tools (used)
      m_uiBitDepth                    = 8;
      m_uiBitIncrement                = 4;
      m_bUseADI                       = true;
      m_bUseEXC                       = true;
      m_bUseCCP                       = true;
			m_bUseCADR                      = true;
      m_bUseALF                       = true;
      m_bUseAMVP											= true;
      m_bUseIMR												= true;
      m_bUseDIF												= true;
      m_bUseLOT                       = true;
      m_bUseROT												= true;
      m_bUseMPI												= true;
			m_bUseAMP												= true;
      m_bUseTMI                       = true;
			m_bUseSHV												= true;
			m_iDIFTap												= 12;
			m_iDIFTapC											= 6;

			// decoder tools (not used)
      m_bUseMVAC                      = false;
			m_bUseRNG												= false;
      m_bUseLCT                       = false;
			m_bUseCIP                       = false;
      m_bUseHAP												= true;
      m_bUseHAB												= true;
      m_bUseHME												= true;
    }
    break;
  default:
    {
      xPrintUsage();
      exit(0);
    }
    break;
  }
}

Void TAppEncCfg::xSetCfgFile( TAppOption* pcOpt )
{
  if ( pcOpt->getValue( "InputFile" ) )                     m_pchInputFile                  = pcOpt->getValue( "InputFile" );
  if ( pcOpt->getValue( "BitstreamFile" ) )                 m_pchBitstreamFile              = pcOpt->getValue( "BitstreamFile" );
  if ( pcOpt->getValue( "ReconFile" ) )                     m_pchReconFile                  = pcOpt->getValue( "ReconFile" );

  if ( pcOpt->getValue( "dQPFile" ) )                       m_pchdQPFile										= pcOpt->getValue( "dQPFile" );
  if ( pcOpt->getValue( "FrameRate" ) )                     m_iFrameRate                    = atoi( pcOpt->getValue( "FrameRate" ) );
  if ( pcOpt->getValue( "FrameSkip" ) )                     m_iFrameSkip                    = atoi( pcOpt->getValue( "FrameSkip" ) );
  if ( pcOpt->getValue( "SourceWidth" ) )                   m_iSourceWidth                  = atoi( pcOpt->getValue( "SourceWidth" ) );
  if ( pcOpt->getValue( "SourceHeight" ) )                  m_iSourceHeight                 = atoi( pcOpt->getValue( "SourceHeight" ) );
  if ( pcOpt->getValue( "FrameToBeEncoded" ) )              m_iFrameToBeEncoded             = atoi( pcOpt->getValue( "FrameToBeEncoded" ) );

  if ( pcOpt->getValue( "IntraPeriod" ) )                   m_iIntraPeriod                  = atoi( pcOpt->getValue( "IntraPeriod" ) );
	if ( pcOpt->getValue( "GOPSize" ) )                       { m_iGOPSize                      = atoi( pcOpt->getValue( "GOPSize" ) ); m_iRateGOPSize = m_iGOPSize; }
	if ( pcOpt->getValue( "RateGOPSize" ) )                   m_iRateGOPSize                  = atoi( pcOpt->getValue( "RateGOPSize" ) );
  if ( pcOpt->getValue( "NumOfReference" ) )                m_iNumOfReference               = atoi( pcOpt->getValue( "NumOfReference" ) );
	if ( pcOpt->getValue( "NumOfReferenceB_L0" ) )            m_iNumOfReferenceB_L0						= atoi( pcOpt->getValue( "NumOfReferenceB_L0" ) );
	if ( pcOpt->getValue( "NumOfReferenceB_L1" ) )            m_iNumOfReferenceB_L1						= atoi( pcOpt->getValue( "NumOfReferenceB_L1" ) );

  if ( pcOpt->getValue( "QP" ) )                            m_fQP                           = atof( pcOpt->getValue( "QP" ) );

  if ( pcOpt->getValue( "TemporalLayerQPOffset_L0" ) )      m_aiTLayerQPOffset[0]			= atoi( pcOpt->getValue( "TemporalLayerQPOffset_L0" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L1" ) )      m_aiTLayerQPOffset[1]			= atoi( pcOpt->getValue( "TemporalLayerQPOffset_L1" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L2" ) )      m_aiTLayerQPOffset[2]			= atoi( pcOpt->getValue( "TemporalLayerQPOffset_L2" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L3" ) )      m_aiTLayerQPOffset[3]			= atoi( pcOpt->getValue( "TemporalLayerQPOffset_L3" ) );

  if ( pcOpt->getValue( "HorizontalPadding" ) )							m_aiPad[0]			= atoi( pcOpt->getValue( "HorizontalPadding" ) );
  if ( pcOpt->getValue( "VerticalPadding" ) )								m_aiPad[1]			= atoi( pcOpt->getValue( "VerticalPadding" ) );

  if ( pcOpt->getValue( "HierarchicalCoding" ) )            m_bHierarchicalCoding           = atoi( pcOpt->getValue( "HierarchicalCoding" ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "SymbolMode" ) )                    m_iSymbolMode                   = atoi( pcOpt->getValue( "SymbolMode" ) );

  if ( pcOpt->getValue( "LoopFilterDisable" ) )             m_bLoopFilterDisable            = atoi( pcOpt->getValue( "LoopFilterDisable" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "LoopFilterAlphaC0Offset" ) )       m_iLoopFilterAlphaC0Offset      = atoi( pcOpt->getValue( "LoopFilterAlphaC0Offset" ) );
  if ( pcOpt->getValue( "LoopFilterBetaOffset" ) )          m_iLoopFilterBetaOffset         = atoi( pcOpt->getValue( "LoopFilterBetaOffset" ) );

  if ( pcOpt->getValue( "FastSearch" ) )                    m_iFastSearch                   = atoi( pcOpt->getValue( "FastSearch" ) );
  if ( pcOpt->getValue( "SearchRange" ) )                   m_iSearchRange                  = atoi( pcOpt->getValue( "SearchRange" ) );
  if ( pcOpt->getValue( "MaxDeltaQP" ) )                    m_iMaxDeltaQP                   = atoi( pcOpt->getValue( "MaxDeltaQP" ) );
	if ( pcOpt->getValue( "HadamardME" ) )                    m_bUseHADME											= atoi( pcOpt->getValue( "HadamardME" ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "MaxCUWidth" ) )                    m_uiMaxCUWidth                  = atoi( pcOpt->getValue( "MaxCUWidth" ) );
  if ( pcOpt->getValue( "MaxCUHeight" ) )                   m_uiMaxCUHeight                 = atoi( pcOpt->getValue( "MaxCUHeight" ) );
  if ( pcOpt->getValue( "MaxPartitionDepth" ) )             m_uiMaxCUDepth                  = atoi( pcOpt->getValue( "MaxPartitionDepth" ) );

  if ( pcOpt->getValue( "MinTrDepth" ) )										m_uiMinTrDepth  								= atoi( pcOpt->getValue( "MinTrDepth" ) );
  if ( pcOpt->getValue( "MaxTrDepth" ) )										m_uiMaxTrDepth									= atoi( pcOpt->getValue( "MaxTrDepth" ) );

  if ( pcOpt->getValue( "ALF" ) )                           m_bUseALF                       = atoi( pcOpt->getValue( "ALF" ) ) == 0  ? false : true;
  if ( pcOpt->getValue( "BitDepth" ) )                      m_uiBitDepth                    = atoi( pcOpt->getValue( "BitDepth" ) );
  if ( pcOpt->getValue( "BitIncrement" ) )                  m_uiBitIncrement                = atoi( pcOpt->getValue( "BitIncrement" ) );

  if ( pcOpt->getValue( "MVAC" ) )                          m_bUseMVAC                      = atoi( pcOpt->getValue( "MVAC"    ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "MVACRD" ) )                        m_bUseMVACRD                    = atoi( pcOpt->getValue( "MVACRD"  ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "DeltaQpRD" ) )                     m_uiDeltaQpRD                   = atoi( pcOpt->getValue( "DeltaQpRD"  ) );
  if ( pcOpt->getValue( "ADI" ) )                           m_bUseADI                       = atoi( pcOpt->getValue( "ADI"     ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "SBACRD" ) )												m_bUseSBACRD                    = atoi( pcOpt->getValue( "SBACRD" ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "AMVP" ) )													m_bUseAMVP										  = atoi( pcOpt->getValue( "AMVP"    ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "IMR" ) )													  m_bUseIMR 										  = atoi( pcOpt->getValue( "IMR"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "DIF" ) )													  m_bUseDIF 										  = atoi( pcOpt->getValue( "DIF"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "DIFTap" ) )											  m_iDIFTap 										  = atoi( pcOpt->getValue( "DIFTap"  ) );
	if ( pcOpt->getValue( "DIFTapC" ) )											  m_iDIFTapC 										  = atoi( pcOpt->getValue( "DIFTapC"  ) );
	if ( pcOpt->getValue( "ACC" ) )														m_bUseACC												= atoi( pcOpt->getValue( "ACC"     ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "ASR" ) )                           m_bUseASR                       = atoi( pcOpt->getValue( "ASR"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "JMQP" ) )													m_bUseJMQP                      = atoi( pcOpt->getValue( "JMQP"    ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "JMLAMBDA" ) )											m_bUseJMLAMBDA                  = atoi( pcOpt->getValue( "JMLAMBDA") ) == 0 ? false : true;
	if ( pcOpt->getValue( "ROT" ) )														m_bUseROT												= atoi( pcOpt->getValue( "ROT"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "MPI" ) )														m_bUseMPI												= atoi( pcOpt->getValue( "MPI"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "RNG" ) )														m_bUseRNG												= atoi( pcOpt->getValue( "RNG"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "GPB" ) )														m_bUseGPB												= atoi( pcOpt->getValue( "GPB"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "AMP" ) )														m_bUseAMP												= atoi( pcOpt->getValue( "AMP"     ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "SHV" ) )														m_bUseSHV												= atoi( pcOpt->getValue( "SHV"     ) ) == 0 ? false : true;

	if ( pcOpt->getValue( "FAM" ) )														m_bUseFAM												= atoi( pcOpt->getValue( "FAM"     ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "PFI" ) )                           m_iProfileIdx                   = atoi( pcOpt->getValue( "PFI"     ) );

	if ( pcOpt->getValue( "MaxTrSize" ) )											m_uiMaxTrSize										= atoi( pcOpt->getValue( "MaxTrSize" ) );

  if ( pcOpt->getValue( "RDOQ" ) )													m_bUseRDOQ											= atoi( pcOpt->getValue( "RDOQ"      ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "LogicalTR" ) )											m_bUseLOT                       = atoi( pcOpt->getValue( "LogicalTR" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "ExtremeCorrection" ) )							m_bUseEXC                       = atoi( pcOpt->getValue( "ExtremeCorrection" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "CCP" ) )						                m_bUseCCP                       = atoi( pcOpt->getValue( "CCP" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "TMIntra" ) )						          	m_bUseTMI                       = atoi( pcOpt->getValue( "TMIntra" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "GeneratedReferenceMode" ) )				m_pchGeneratedReferenceMode     = pcOpt->getValue( "GeneratedReferenceMode" );
  if ( pcOpt->getValue( "LowDelayCoding" ) )								m_bUseLDC                       = atoi( pcOpt->getValue( "LowDelayCoding"    ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "LowComplexityTransform" ) )				m_bUseLCT                       = atoi( pcOpt->getValue( "LowComplexityTransform"    ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "CIP" ) )														m_bUseCIP												= atoi( pcOpt->getValue( "CIP"       ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "PAD" ) )														m_bUsePAD												= atoi( pcOpt->getValue( "PAD"       ) ) == 0 ? false : true;

	if ( pcOpt->getValue( "CADR" ) )													m_bUseCADR											= atoi( pcOpt->getValue( "CADR"      ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "MinCADR" ) )												m_iMinCADR											= atoi( pcOpt->getValue( "MinCADR"   ) );
	if ( pcOpt->getValue( "MaxCADR" ) )												m_iMaxCADR											= atoi( pcOpt->getValue( "MaxCADR"   ) );

	if ( pcOpt->getValue( "QBO" ) )														m_bUseQBO												= atoi( pcOpt->getValue( "QBO"       ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "NRF" ) )														m_bUseNRF												= atoi( pcOpt->getValue( "NRF"       ) ) == 0 ? false : true;
	if ( pcOpt->getValue( "BQP" ) )														m_bUseBQP												= atoi( pcOpt->getValue( "BQP"       ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "ACS" ) )						                m_bUseACS                       = atoi( pcOpt->getValue( "ACS" ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "HAP" ) )						                m_bUseHAP                       = atoi( pcOpt->getValue( "HAP" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "HAB" ) )						                m_bUseHAB                       = atoi( pcOpt->getValue( "HAB" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "HME" ) )						                m_bUseHME                       = atoi( pcOpt->getValue( "HME" ) ) == 0 ? false : true;

  return;
}

Void TAppEncCfg::xSetCfgCommand( int iArgc, char** pArgv )
{
	Int		 i;
	char*	 pStr;
	char*  pToolName;

	i=1;
	while ( i < iArgc )
	{
		pStr = pArgv[i];
		if			( !strcmp( pStr, "-c" ) )		{ i++; i++; }
		else if ( !strcmp( pStr, "-i" ) )		{ i++; m_pchInputFile					= pArgv[i];					i++; }
		else if ( !strcmp( pStr, "-b" ) )		{	i++; m_pchBitstreamFile			= pArgv[i];					i++; }
		else if ( !strcmp( pStr, "-o" ) )		{	i++; m_pchReconFile					= pArgv[i];					i++; }
		else if ( !strcmp( pStr, "-m" ) )		{	i++; m_pchdQPFile						= pArgv[i];					i++; }
		else if ( !strcmp( pStr, "-v" ) )		{	i++; m_pchGeneratedReferenceMode	= pArgv[i];				i++; }
    else if ( !strcmp( pStr, "-f" ) )		{	i++; m_iFrameToBeEncoded		= atoi( pArgv[i] ); i++; }
		else if ( !strcmp( pStr, "-q" ) )		{	i++; m_fQP									= atof( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-g" ) )		{	i++; m_iGOPSize							= atoi( pArgv[i] );	i++; m_iRateGOPSize = m_iGOPSize; }
		else if ( !strcmp( pStr, "-rg" ) )	{	i++; m_iRateGOPSize					= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-r" ) )		{	i++; m_iNumOfReference			= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-rb0" ) )	{	i++; m_iNumOfReferenceB_L0	= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-rb1" ) )	{	i++; m_iNumOfReferenceB_L1	= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-ip" ) )	{	i++; m_iIntraPeriod					= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-d" ) )		{	i++; m_iMaxDeltaQP					= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-dqr" ) )	{	i++; m_uiDeltaQpRD					= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-s" ) )		{	i++; m_uiMaxCUWidth					= m_uiMaxCUHeight  = atoi( pArgv[i] ); i++; }
		else if ( !strcmp( pStr, "-h" ) )		{	i++; m_uiMaxCUDepth					= atoi( pArgv[i] );	i++; }
    else if ( !strcmp( pStr, "-p" ) )		{	i++; m_iProfileIdx					= atoi( pArgv[i] ); i++; xSetProfile(); }
		else if ( !strcmp( pStr, "-t" ) )		{	i++; m_uiMaxTrSize					= atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-ltd" ) )	{	i++; m_uiMinTrDepth					= atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-utd" ) )	{	i++; m_uiMaxTrDepth					= atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tq0" ) )	{	i++; m_aiTLayerQPOffset[0]	= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-tq1" ) )	{	i++; m_aiTLayerQPOffset[1]	= atoi( pArgv[i] );	i++; }
    else if ( !strcmp( pStr, "-tq2" ) )	{	i++; m_aiTLayerQPOffset[2]	= atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-tq3" ) )	{	i++; m_aiTLayerQPOffset[3]	= atoi( pArgv[i] );	i++; }
    else if ( !strcmp( pStr, "-pdx" ) )	{	i++; m_aiPad[0]	            = atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-pdy" ) )	{	i++; m_aiPad[1]	            = atoi( pArgv[i] );	i++; }
		else if ( !strcmp( pStr, "-ldm" ) )	{	i++; m_iLDMode              = atoi( pArgv[i] ); i++; }
		else if ( !strcmp( pStr, "-tap" ) )	{	i++; m_iDIFTap              = atoi( pArgv[i] ); i++; }
		else if ( !strcmp( pStr, "-taq" ) )	{	i++; m_iDIFTapC							= atoi( pArgv[i] ); i++; }
		else
		if ( !strcmp( pStr, "-1" ) )
		{
			i++;
			pToolName = pArgv[i];
			if ( !strcmp( pToolName, "ACC" ) )	m_bUseACC					= true;
			else
			if ( !strcmp( pToolName, "ADI" ) )	m_bUseADI					= true;
      else
      if ( !strcmp( pToolName, "ASR" ) )m_bUseASR						= true;
			else
			if ( !strcmp( pToolName, "MVA" ) )	m_bUseMVAC				= true;
			else
			if ( !strcmp( pToolName, "MVR" ) ){ m_bUseMVACRD      = true; m_bUseMVAC = true; }
			else
			if ( !strcmp( pToolName, "ALF" ) )	m_bUseALF					= true;
			else
			if ( !strcmp( pToolName, "IBD" ) )	m_uiBitIncrement	= 4;
			else
			if ( !strcmp( pToolName, "ROT" ) )	m_bUseROT					= true;
			else
			if ( !strcmp( pToolName, "MPI" ) )	m_bUseMPI					= true;
			else
			if ( !strcmp( pToolName, "JMQ" ) )	m_bUseJMQP				= true;
			else
			if ( !strcmp( pToolName, "JML" ) )	m_bUseJMLAMBDA		= true;
			else
			if ( !strcmp( pToolName, "AMV" ) )	m_bUseAMVP				= true;
			else
			if ( !strcmp( pToolName, "IMR" ) )	m_bUseIMR 				= true;
			else
			if ( !strcmp( pToolName, "DIF" ) )	m_bUseDIF 				= true;
			else
			if ( !strcmp( pToolName, "HAD" ) )	m_bUseHADME				= true;
			else
			if ( !strcmp( pToolName, "RNG" ) )	m_bUseRNG					= true;
      else
			if ( !strcmp( pToolName, "SRD" ) )	m_bUseSBACRD			= true;
      else
			if ( !strcmp( pToolName, "GPB" ) )	m_bUseGPB					= true;
      else
			if ( !strcmp( pToolName, "AMP" ) )	m_bUseAMP					= true;
      else
			if ( !strcmp( pToolName, "SHV" ) )	m_bUseSHV					= true;
      else
			if ( !strcmp( pToolName, "FAM" ) )	m_bUseFAM					= true;
      else
      if ( !strcmp( pToolName, "RDO" ) )	m_bUseRDOQ				= true;
      else
      if ( !strcmp( pToolName, "LOT" ) )	m_bUseLOT         = true;
      else
      if ( !strcmp( pToolName, "EXC" ) )	m_bUseEXC         = true;
      else
      if ( !strcmp( pToolName, "CCP" ) )	m_bUseCCP         = true;
      else
      if ( !strcmp( pToolName, "TMI" ) )	m_bUseTMI         = true;
      else
      if ( !strcmp( pToolName, "LDC" ) )	m_bUseLDC         = true;
      else
      if ( !strcmp( pToolName, "LCT" ) )	m_bUseLCT         = true;
      else
      if ( !strcmp( pToolName, "CIP" ) )	m_bUseCIP         = true;
      else
      if ( !strcmp( pToolName, "PAD" ) )	m_bUsePAD         = true;
      else
      if ( !strcmp( pToolName, "CAD" ) )	m_bUseCADR        = true;
      else
      if ( !strcmp( pToolName, "QBO" ) )	m_bUseQBO         = true;
      else
      if ( !strcmp( pToolName, "NRF" ) )	m_bUseNRF         = true;
      else
      if ( !strcmp( pToolName, "BQP" ) )	m_bUseBQP         = true;
      else
      if ( !strcmp( pToolName, "ACS" ) )	m_bUseACS         = true;
      else
      if ( !strcmp( pToolName, "HAP" ) )	m_bUseHAP         = true;
      else
      if ( !strcmp( pToolName, "HAB" ) )	m_bUseHAB         = true;
      else
      if ( !strcmp( pToolName, "HME" ) )	m_bUseHME         = true;
			i++;
		}
		else
		if ( !strcmp( pStr, "-0" ) )
		{
			i++;
			pToolName = pArgv[i];
			if ( !strcmp( pToolName, "ACC" ) )	m_bUseACC					= false;
			else
			if ( !strcmp( pToolName, "ADI" ) )	m_bUseADI					= false;
      else
      if ( !strcmp( pToolName, "ASR" ) )m_bUseASR						= false;
			else
			if ( !strcmp( pToolName, "MVA" ) ){	m_bUseMVAC				= false; m_bUseMVACRD = false; }
			else
			if ( !strcmp( pToolName, "MVR" ) )	m_bUseMVACRD			= false;
			else
			if ( !strcmp( pToolName, "ALF" ) )	m_bUseALF					= false;
			else
			if ( !strcmp( pToolName, "IBD" ) )	m_uiBitIncrement	= 0;
			else
			if ( !strcmp( pToolName, "ROT" ) )	m_bUseROT					= false;
			else
			if ( !strcmp( pToolName, "MPI" ) )	m_bUseMPI					= false;
			else
			if ( !strcmp( pToolName, "JMQ" ) )	m_bUseJMQP				= false;
			else
			if ( !strcmp( pToolName, "JML" ) )	m_bUseJMLAMBDA		= false;
			else
			if ( !strcmp( pToolName, "AMV" ) )	m_bUseAMVP				= false;
			else
			if ( !strcmp( pToolName, "IMR" ) )	m_bUseIMR				= false;
			else
			if ( !strcmp( pToolName, "DIF" ) )	m_bUseDIF				= false;
			else
			if ( !strcmp( pToolName, "HAD" ) )	m_bUseHADME				= false;
			else
			if ( !strcmp( pToolName, "RNG" ) )	m_bUseRNG					= false;
      else
			if ( !strcmp( pToolName, "SRD" ) )	m_bUseSBACRD			= false;
      else
			if ( !strcmp( pToolName, "GPB" ) )	m_bUseGPB					= false;
      else
			if ( !strcmp( pToolName, "AMP" ) )	m_bUseAMP					= false;
      else
			if ( !strcmp( pToolName, "SHV" ) )	m_bUseSHV					= false;
      else
			if ( !strcmp( pToolName, "FAM" ) )	m_bUseFAM					= false;
      else
      if ( !strcmp( pToolName, "RDO" ) )	m_bUseRDOQ				= false;
      else
      if ( !strcmp( pToolName, "LOT" ) )	m_bUseLOT         = false;
      else
      if ( !strcmp( pToolName, "EXC" ) )	m_bUseEXC         = false;
      else
      if ( !strcmp( pToolName, "CCP" ) )	m_bUseCCP         = false;
      else
      if ( !strcmp( pToolName, "TMI" ) )	m_bUseTMI         = false;
      else
      if ( !strcmp( pToolName, "LDC" ) )	m_bUseLDC				  = false;
      else
      if ( !strcmp( pToolName, "LCT" ) )	m_bUseLCT				  = false;
      else
			if ( !strcmp( pToolName, "CIP" ) )	m_bUseCIP				  = false;
      else
			if ( !strcmp( pToolName, "PAD" ) )	m_bUsePAD				  = false;
      else
      if ( !strcmp( pToolName, "CAD" ) )	m_bUseCADR        = false;
      else
      if ( !strcmp( pToolName, "QBO" ) )	m_bUseQBO         = false;
      else
      if ( !strcmp( pToolName, "NRF" ) )	m_bUseNRF         = false;
      else
      if ( !strcmp( pToolName, "BQP" ) )	m_bUseBQP         = false;
      else
      if ( !strcmp( pToolName, "ACS" ) )	m_bUseACS         = false;
      else
      if ( !strcmp( pToolName, "HAP" ) )	m_bUseHAP         = false;
      else
      if ( !strcmp( pToolName, "HAB" ) )	m_bUseHAB         = false;
      else
      if ( !strcmp( pToolName, "HME" ) )	m_bUseHME         = false;
			i++;
		}
		else
		{
			i++;
		}
	}

	// for convenience
	if ( m_iLDMode != -1 )
	{
		if ( m_iLDMode == 0 )
		{
			m_bUseQBO = true;				// use quality-based reference reordering
			m_bUseBQP = false;			// use hier-P QP
			m_bUseNRF = true;				// use non-reference marking
		}
		else
		if ( m_iLDMode == 1 )
		{
			m_bUseQBO = false;			// don't use quality-based reference reordering
			m_bUseBQP = true;				// user hier-P QP
			m_bUseNRF = false;			// don't use non-reference marking
		}
	}
}

Void TAppEncCfg::xPrintParameter()
{
	printf("\n");
  printf("Input          File          : %s\n", m_pchInputFile					);
  printf("Bitstream      File          : %s\n", m_pchBitstreamFile			);
  printf("Reconstruction File          : %s\n", m_pchReconFile					);
  printf("Real     Format              : %dx%d %dHz\n", m_iSourceWidth - m_aiPad[0], m_iSourceHeight-m_aiPad[1], m_iFrameRate );
  printf("Internal Format              : %dx%d %dHz\n", m_iSourceWidth, m_iSourceHeight, m_iFrameRate );
  printf("Frame index                  : %d - %d (%d frames)\n", m_iFrameSkip, m_iFrameSkip+m_iFrameToBeEncoded-1, m_iFrameToBeEncoded );
  printf("Number of Ref. frames (P)    : %d\n", m_iNumOfReference);
  printf("Number of Ref. frames (B_L0) : %d\n", m_iNumOfReferenceB_L0);
  printf("Number of Ref. frames (B_L1) : %d\n", m_iNumOfReferenceB_L1);
  printf("Number of Reference frames   : %d\n", m_iNumOfReference);
	printf("Intra period                 : %d\n", m_iIntraPeriod);
  printf("CU size / depth              : %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth );
  printf("Transform depth (min / max)  : %d / %d\n", m_uiMinTrDepth, m_uiMaxTrDepth );
  printf("QP                           : %5.2f\n", m_fQP );
  printf("GOP size                     : %d\n", m_iGOPSize );
	printf("Rate GOP size                : %d\n", m_iRateGOPSize );
	printf("Max physical trans. size     : %d\n", m_uiMaxTrSize );
	printf("Bit increment                : %d\n", m_uiBitIncrement );

	if ( m_bUseDIF )
	{
		printf("Number of int. taps (luma)   : %d\n", m_iDIFTap  );
		printf("Number of int. taps (chroma) : %d\n", m_iDIFTapC );
	}
	else
	{
		printf("Number of int. taps (luma)   : %d\n", 6					 );
		printf("Number of int. taps (chroma) : %d\n", 2					 );
	}

	if ( m_iSymbolMode == 0 )
	{
		printf("Entropy coder                : VLC\n");
	}
	else
	{
		printf("Entropy coder                : SBAC\n");
	}

	if ( g_bUseCADR )
	{
		if ( m_iMinCADR >= 0 )
		{
			printf("CADR range                   : %d - %d\n", m_iMinCADR, m_iMaxCADR);
		}
		else
		{
			printf("CADR range                   : derived\n");
		}
	}

  printf("\n");

	printf("ENC TOOL: ");
	printf("ACC:%d ",	m_bUseACC						);
  printf("ASR:%d ", m_bUseASR           );
	printf("JMQ:%d ",	m_bUseJMQP					);
	printf("JML:%d ",	m_bUseJMLAMBDA			);
	printf("HAD:%d ",	m_bUseHADME					);
  printf("SRD:%d ",	m_bUseSBACRD 				);
	printf("FAM:%d ",	m_bUseFAM		 				);
	printf("SQP:%d ",	m_uiDeltaQpRD			  );
	printf("MVR:%d ",	m_bUseMVACRD  			);
  printf("RDO:%d ", m_bUseRDOQ          );
  printf("LDC:%d ", m_bUseLDC           );
	printf("NRF:%d ", m_bUseNRF           );
	printf("BQP:%d ", m_bUseBQP           );
  printf("PAD:%d ", m_bUsePAD           );
	printf("HME:%d ", m_bUseHME           );
	printf("\n");
	printf("DEC TOOL: ");
	printf("ALF:%d ",	m_bUseALF					);
	printf("AMV:%d ",	m_bUseAMVP					);
	printf("IMR:%d ",	m_bUseIMR 					);
	printf("DIF:%d ",	m_bUseDIF 					);
	printf("MVA:%d ",	m_bUseMVAC					);
	printf("ADI:%d ",	m_bUseADI						);
	printf("ROT:%d ",	m_bUseROT						);
	printf("MPI:%d ",	m_bUseMPI						);
	printf("RNG:%d ",	m_bUseRNG						);
	printf("GPB:%d ",	m_bUseGPB						);
	printf("AMP:%d ",	m_bUseAMP						);
  printf("SHV:%d ", m_bUseSHV 					);
  printf("LOT:%d ",	m_bUseLOT						);
  printf("EXC:%d ",	m_bUseEXC						);
  printf("GRF:%d ",	m_pchGeneratedReferenceMode != NULL );

	printf("\n");
	printf("          ");

  printf("CCP:%d ",	m_bUseCCP						);
  printf("TMI:%d ",	m_bUseTMI						);
  printf("QBO:%d ", m_bUseQBO           );
  printf("LCT:%d ", m_bUseLCT           );
  printf("ACS:%d ",	m_bUseACS						);
  printf("CIP:%d ", m_bUseCIP           );
  printf("HAP:%d ", m_bUseHAP           );
  printf("HAB:%d ", m_bUseHAB           );
  printf("\n");

  fflush(stdout);
}

Void TAppEncCfg::xPrintUsage()
{
	printf("\n");

  printf( "  -c    config. file name\n" );
  printf( "  -i    original YUV input file name\n" );
  printf( "  -o    decoded YUV output file name\n" );
  printf( "  -b    bitstream file name\n" );
  printf( "  -m    dQP file name\n" );
  printf( "  -f    number of frames to be encoded (default EOS)\n" );
  printf( "  -q    Qp value\n" );
  printf( "  -g    GOP size\n" );
  printf( "  -r    Number of reference (P)\n" );
  printf( "  -rb0  Number of reference (B_L0)\n" );
	printf( "  -rb1  Number of reference (B_L1)\n" );
  printf( "  -tq0  QP offset of temporal layer 0\n" );
	printf( "  -tq1  QP offset of temporal layer 1\n" );
  printf( "  -tq2  QP offset of temporal layer 2\n" );
	printf( "  -tq3  QP offset of temporal layer 3\n" );
  printf( "  -ip   intra period\n" );
  printf( "  -d    max dQp offset\n");
	printf( "  -s    max CU size\n" );
	printf( "  -h    CU depth\n" );
  printf( "  -t    Max. transform size\n" );
  printf( "  -ltd  Min. transform depth\n" );
  printf( "  -utd  Max. transform depth\n" );
  printf( "  -p    Profiling (0==all tools off, 1==default tools on 2==all tools on)\n" );
  printf( "  -v    Types of artificial references (w:weighted pred, o:offset pred, r:refinement pred, a: affine, i: isotropic, p: perspective)\n" );
  printf( "  -dqr  max dQp offset for Rd picture selection\n");
	printf( "  -tap  Number of interpolation filter taps (luma)\n");
	printf( "  -taq  Number of interpolation filter taps (chroma)\n");
	printf( "  -1    tool: on  [ACC, JMQ, JML, HAD, RNG, SRD, FAM, RDO, MVR, LDC, NRF, BQP, IMR, DIF, PAD]\n" );
	printf( "  -1    tool: on  [ALF, IBD, AMV, MVA, ADI, ROT, MPI, AMP, LOT, EXC, CCP, TMI, QBO, ACS, CIP, HAP, HAB, HME]\n" );
	printf( "  -0    tool: off [ACC, JMQ, JML, HAD, RNG, SRD, FAM, RDO, MVR, LDC, NRF, BQP, IMR, DIF, PAD]\n" );
	printf( "  -0    tool: off [ALF, IBD, AMV, MVA, ADI, ROT, MPI, AMP, LOT, EXC, CCP, TMI, QBO, ACS, CIP, HAP, HAB, HME]\n" );
}

Void TAppEncCfg::xConfirmPara(Bool bflag, const char* message)
{
  if( bflag )
  {
    printf("\n%s\n",message);
    exit(0);
  }
}
