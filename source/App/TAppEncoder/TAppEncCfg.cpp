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

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <cstring>
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
#if HHI_RQT
: m_bQuadtreeTUFlag( true )
, m_uiQuadtreeTULog2MaxSize( 6 )
, m_uiQuadtreeTULog2MinSize( 2 )
#endif
{
  // initialize member variables
  m_pchInputFile                  = NULL;
  m_pchBitstreamFile              = NULL;
  m_pchReconFile                  = NULL;
  m_pchdQPFile                    = NULL;
  m_iFrameRate                    = 0;
  m_iFrameSkip                    = 0;
  m_iSourceWidth                  = 0;
  m_iSourceHeight                 = 0;
  m_iFrameToBeEncoded             = 0;
  m_iIntraPeriod                  = -1;
  m_iGOPSize                      = 1;
  m_iRateGOPSize                  = m_iGOPSize;
  m_iNumOfReference               = 1;
  m_iNumOfReferenceB_L0           = 1;
  m_iNumOfReferenceB_L1           = 1;
  m_iQP                           = 30;
  m_fQP                           = m_iQP;
  m_aiTLayerQPOffset[0]           = ( MAX_QP + 1 );
  m_aiTLayerQPOffset[1]           = ( MAX_QP + 1 );
  m_aiTLayerQPOffset[2]           = ( MAX_QP + 1 );
  m_aiTLayerQPOffset[3]           = ( MAX_QP + 1 );
  m_aiPad[0]                      = 0;
  m_aiPad[1]                      = 0;
  m_aidQP                         = NULL;
  m_bHierarchicalCoding           = true;

  m_uiMinTrDepth                  = 0;
  m_uiMaxTrDepth                  = 1;
  m_iSymbolMode                   = 1;
  m_uiMCWThreshold                = 0;
  m_uiMaxPIPEDelay                = 0;
  m_uiBalancedCPUs                = 8;
  m_pchGRefMode     = NULL;
  m_bLoopFilterDisable            = false;
  m_iLoopFilterAlphaC0Offset      = 0;
  m_iLoopFilterBetaOffset         = 0;
  m_iFastSearch                   = 1;
  m_iSearchRange                  = 96;
  m_iMaxDeltaQP                   = 0;
  m_uiMaxTrSize                   = 64;
  m_iDIFTap                       = 12;
#if HHI_ALF
  m_bALFUseSeparateQT             = true;
  m_bALFFilterSymmetry            = false;
  m_iAlfMinLength                 = 3;
  m_iAlfMaxLength                 = 9;
#endif

  // initialize configuration values for each tool
  xSetCfgTool();
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

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, Char* argv[] )
{
  // '-c' option is used for specifying configuration file
  m_apcOpt->setCommandOption( 'c' );

  // register configuration strings
  m_apcOpt->setFileOption( "InputFile"                );
  m_apcOpt->setFileOption( "BitstreamFile"            );
  m_apcOpt->setFileOption( "ReconFile"                );
  m_apcOpt->setFileOption( "FrameRate"                );
  m_apcOpt->setFileOption( "FrameSkip"                );
  m_apcOpt->setFileOption( "SourceWidth"              );
  m_apcOpt->setFileOption( "SourceHeight"             );
  m_apcOpt->setFileOption( "FrameToBeEncoded"         );
  m_apcOpt->setFileOption( "IntraPeriod"              );
  m_apcOpt->setFileOption( "GOPSize"                  );
#if HHI_ALLOW_RATEGOPSIZE_IN_CFG_FILE
  m_apcOpt->setFileOption( "RateGOPSize"              );
#endif
#if HHI_ALLOW_GPB_IN_CFG_FILE
  m_apcOpt->setFileOption( "GPB"                      );
#endif
  m_apcOpt->setFileOption( "NumOfReference"           );
  m_apcOpt->setFileOption( "NumOfReferenceB_L0"       );
  m_apcOpt->setFileOption( "NumOfReferenceB_L1"       );
  m_apcOpt->setFileOption( "QP"                       );
  m_apcOpt->setFileOption( "HierarchicalCoding"       );
  m_apcOpt->setFileOption( "FastSearch"               );
  m_apcOpt->setFileOption( "SearchRange"              );
  m_apcOpt->setFileOption( "MaxDeltaQP"               );
  m_apcOpt->setFileOption( "HadamardME"               );
  m_apcOpt->setFileOption( "PFI"                      );
  m_apcOpt->setFileOption( "LowDelayCoding" );

  m_apcOpt->setFileOption( "MaxCUWidth"               );
  m_apcOpt->setFileOption( "MaxCUHeight"              );
  m_apcOpt->setFileOption( "MaxPartitionDepth"        );

#if HHI_RQT
  m_apcOpt->setFileOption( "QuadtreeTUFlag"           );
  m_apcOpt->setFileOption( "QuadtreeTULog2MaxSize"    );
  m_apcOpt->setFileOption( "QuadtreeTULog2MinSize"    );
#endif

#if HHI_INTERP_FILTER
  m_apcOpt->setFileOption( "InterpFilterType"         );
#endif

  m_apcOpt->setFileOption( "ALF"                      );

#if HHI_ALF
  m_apcOpt->setFileOption( "ALFSeparateTree"          );
  m_apcOpt->setFileOption( "ALFSymmetry"              );
  m_apcOpt->setFileOption( "ALFMinLength"             );
  m_apcOpt->setFileOption( "ALFMaxLength"             );
#endif

#if HHI_ALLOW_CIP_SWITCH
  m_apcOpt->setFileOption( "CIP"                      );
#endif
#if HHI_AIS
  m_apcOpt->setFileOption( "AIS"                      );// BB: adaptive intra smoothing
#endif
#if HHI_MRG
  m_apcOpt->setFileOption( "MRG"                      );// SOPH: merge mode
#endif
#if HHI_IMVP
  m_apcOpt->setFileOption( "IMP"                      );// SOPH: interleaved motion vector predictor
#endif
  m_apcOpt->setFileOption( "FilterLength"             );
  m_apcOpt->setFileOption( "NumOfQBits"               );
  m_apcOpt->setFileOption( "ColorNumber"              );
  m_apcOpt->setFileOption( "FilterType"               );
  m_apcOpt->setFileOption( "BitDepth"                 );
  m_apcOpt->setFileOption( "BitIncrement"             );
  m_apcOpt->setFileOption( "ROT"                      );
  m_apcOpt->setFileOption( "DeltaQpRD"                );
  m_apcOpt->setFileOption( "RDOQ"                     );
  m_apcOpt->setFileOption( "LogicalTR"                );
  m_apcOpt->setFileOption( "ExtremeCorrection"        );
  m_apcOpt->setFileOption( "GRefMode"   );
  m_apcOpt->setFileOption( "QBO"                      );
  m_apcOpt->setFileOption( "NRF"                      );
  m_apcOpt->setFileOption( "BQP"                      );
#ifdef QC_AMVRES
	m_apcOpt->setFileOption( "AMVRES"											);
#endif
#ifdef QC_SIFO_PRED
	m_apcOpt->setFileOption( "SPF"											);
#endif
  m_apcOpt->setFileOption( "DIFTap"                   );
  m_apcOpt->setFileOption( "LoopFilterDisable"        );
  m_apcOpt->setFileOption( "LoopFilterAlphaC0Offset"  );
  m_apcOpt->setFileOption( "LoopFilterBetaOffset"     );
  m_apcOpt->setFileOption( "SymbolMode"               );
  m_apcOpt->setFileOption( "MultiCodewordThreshold"   );
  m_apcOpt->setFileOption( "MaxPIPEBufferDelay"       );
  m_apcOpt->setFileOption( "FEN"                      );
  m_apcOpt->setFileOption( "BalancedCPUs"             );

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
  xSetCfgFile     ( m_apcOpt   );
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

#if HHI_RQT
  if( m_bQuadtreeTUFlag )
  {
    xConfirmPara( m_uiQuadtreeTULog2MinSize < 2,                                        "QuadtreeTULog2MinSize must be 2 or greater.");
    xConfirmPara( m_uiQuadtreeTULog2MinSize > 6,                                        "QuadtreeTULog2MinSize must be 6 or smaller.");
    xConfirmPara( m_uiQuadtreeTULog2MaxSize < 2,                                        "QuadtreeTULog2MaxSize must be 2 or greater.");
    xConfirmPara( m_uiQuadtreeTULog2MaxSize > 6,                                        "QuadtreeTULog2MaxSize must be 6 or smaller.");
    xConfirmPara( m_uiQuadtreeTULog2MaxSize < m_uiQuadtreeTULog2MinSize,                "QuadtreeTULog2MaxSize must be greater than or equal to m_uiQuadtreeTULog2MinSize.");
    xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUWidth >>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
    xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUHeight>>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
  }
#endif

  xConfirmPara( m_iSymbolMode < 0 || m_iSymbolMode > 3,                                     "SymbolMode must be equal to 0, 1, 2, or 3" );
  xConfirmPara( m_uiMaxPIPEDelay != 0 && m_uiMaxPIPEDelay < 64,                             "MaxPIPEBufferDelay must be greater than or equal to 64" );
  m_uiMaxPIPEDelay = ( m_uiMCWThreshold > 0 ? 0 : ( m_uiMaxPIPEDelay >> 6 ) << 6 );
  xConfirmPara( m_uiBalancedCPUs > 255,                                                     "BalancedCPUs must not be greater than 255" );

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

  // SBACRD is supported only for SBAC
  if ( m_iSymbolMode == 0 )
  {
    m_bUseSBACRD = false;
  }

#if !NEWVLC
  // RDOQ is supported only for SBAC
  if ( !m_bUseSBACRD )
  {
    m_bUseRDOQ = false;
  }
#endif
#ifdef QC_AMVRES
  if(m_iInterpFilterType == IPF_TEN_DIF)
    m_bUseAMVRes = false;
#endif

}

/** \todo use of global variables should be removed later
 */
Void TAppEncCfg::xSetGlobal()
{
  // set max CU width & height
  g_uiMaxCUWidth  = m_uiMaxCUWidth;
  g_uiMaxCUHeight = m_uiMaxCUHeight;

  // compute actual CU depth with respect to config depth and max transform size
  g_uiAddCUDepth  = 0;
  if( ((m_uiMaxCUWidth>>(m_uiMaxCUDepth-1)) > (m_uiMaxTrSize>>m_uiMaxTrDepth)) )
  {
    while( (m_uiMaxCUWidth>>(m_uiMaxCUDepth-1)) > (m_uiMaxTrSize<<g_uiAddCUDepth) ) g_uiAddCUDepth++;
  }
  m_uiMaxCUDepth += g_uiAddCUDepth;
  g_uiAddCUDepth++;
  g_uiMaxCUDepth = m_uiMaxCUDepth;

  // set internal bit-depth and constants
  g_uiBitDepth     = m_uiBitDepth;                      // base bit-depth
  g_uiBitIncrement = m_uiBitIncrement;                  // increments
  g_uiBASE_MAX     = ((1<<(g_uiBitDepth))-1);

#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX     = g_uiBASE_MAX << g_uiBitIncrement;
#else
  g_uiIBDI_MAX     = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);
#endif
}

Void TAppEncCfg::xSetCfgTool()
{
  // basic unit size
  m_uiMaxCUWidth                  = 64;
  m_uiMaxCUHeight                 = 64;
  m_uiMaxCUDepth                  = 4;

  // encoder tools
  m_bUseSBACRD                    = true;
  m_bUseHADME                     = true;
  m_bUseRDOQ                      = true;
  m_bUseNRF                       = true;
  m_bUseASR                       = false;
  m_uiDeltaQpRD                   = 0;
  m_bUseGPB                       = false;
  m_bUseLDC                       = false;
  m_bUseQBO                       = false;
  m_bUseBQP                       = false;
  m_iLDMode                       = -1;
  m_bUsePAD                       = false;
#ifdef QC_AMVRES
  m_bUseAMVRes											= true;
#endif
#ifdef QC_SIFO_PRED
  m_bUseSIFO_Pred               = true;
#endif
#if HHI_MRG
  m_bUseMRG                       = true; // SOPH: Merge Mode
#endif
#if HHI_IMVP
  m_bUseIMP                       = true; // SOPH: Interleaved Motion Vector Predictor
#endif
  m_bUseFastEnc                   = false;
  // decoder tools
  m_uiBitDepth                    = 8;
  m_uiBitIncrement                = 4;
  m_bUseALF                       = true;

#if HHI_ALF
  m_bALFUseSeparateQT             = true;
  m_bALFFilterSymmetry            = false;
  m_iAlfMinLength                 = 5;
  m_iAlfMaxLength                 = 9;
#endif

  m_iDIFTap                       = 12;
#if HHI_ALLOW_CIP_SWITCH
  m_bUseCIP                       = true;
#endif
#if HHI_ALLOW_ROT_SWITCH
  m_bUseROT                       = true;
#endif
#if HHI_AIS
  m_bUseAIS                       = true; // BB: adaptive intra smoothing
#endif
#if HHI_INTERP_FILTER
  m_iInterpFilterType             = IPF_SAMSUNG_DIF_DEFAULT;
#endif
}

Void TAppEncCfg::xSetCfgFile( TAppOption* pcOpt )
{
  if ( pcOpt->getValue( "InputFile" ) )                     m_pchInputFile                  = pcOpt->getValue( "InputFile" );
  if ( pcOpt->getValue( "BitstreamFile" ) )                 m_pchBitstreamFile              = pcOpt->getValue( "BitstreamFile" );
  if ( pcOpt->getValue( "ReconFile" ) )                     m_pchReconFile                  = pcOpt->getValue( "ReconFile" );

  if ( pcOpt->getValue( "dQPFile" ) )                       m_pchdQPFile                    = pcOpt->getValue( "dQPFile" );
  if ( pcOpt->getValue( "FrameRate" ) )                     m_iFrameRate                    = atoi( pcOpt->getValue( "FrameRate" ) );
  if ( pcOpt->getValue( "FrameSkip" ) )                     m_iFrameSkip                    = atoi( pcOpt->getValue( "FrameSkip" ) );
  if ( pcOpt->getValue( "SourceWidth" ) )                   m_iSourceWidth                  = atoi( pcOpt->getValue( "SourceWidth" ) );
  if ( pcOpt->getValue( "SourceHeight" ) )                  m_iSourceHeight                 = atoi( pcOpt->getValue( "SourceHeight" ) );
  if ( pcOpt->getValue( "FrameToBeEncoded" ) )              m_iFrameToBeEncoded             = atoi( pcOpt->getValue( "FrameToBeEncoded" ) );

  if ( pcOpt->getValue( "IntraPeriod" ) )                   m_iIntraPeriod                  = atoi( pcOpt->getValue( "IntraPeriod" ) );
  if ( pcOpt->getValue( "GOPSize" ) )                       { m_iGOPSize                      = atoi( pcOpt->getValue( "GOPSize" ) ); m_iRateGOPSize = m_iGOPSize; }
  if ( pcOpt->getValue( "RateGOPSize" ) )                   m_iRateGOPSize                  = atoi( pcOpt->getValue( "RateGOPSize" ) );
  if ( pcOpt->getValue( "NumOfReference" ) )                m_iNumOfReference               = atoi( pcOpt->getValue( "NumOfReference" ) );
  if ( pcOpt->getValue( "NumOfReferenceB_L0" ) )            m_iNumOfReferenceB_L0           = atoi( pcOpt->getValue( "NumOfReferenceB_L0" ) );
  if ( pcOpt->getValue( "NumOfReferenceB_L1" ) )            m_iNumOfReferenceB_L1           = atoi( pcOpt->getValue( "NumOfReferenceB_L1" ) );

  if ( pcOpt->getValue( "QP" ) )                            m_fQP                           = atof( pcOpt->getValue( "QP" ) );

  if ( pcOpt->getValue( "TemporalLayerQPOffset_L0" ) )      m_aiTLayerQPOffset[0]     = atoi( pcOpt->getValue( "TemporalLayerQPOffset_L0" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L1" ) )      m_aiTLayerQPOffset[1]     = atoi( pcOpt->getValue( "TemporalLayerQPOffset_L1" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L2" ) )      m_aiTLayerQPOffset[2]     = atoi( pcOpt->getValue( "TemporalLayerQPOffset_L2" ) );
  if ( pcOpt->getValue( "TemporalLayerQPOffset_L3" ) )      m_aiTLayerQPOffset[3]     = atoi( pcOpt->getValue( "TemporalLayerQPOffset_L3" ) );

  if ( pcOpt->getValue( "HorizontalPadding" ) )             m_aiPad[0]      = atoi( pcOpt->getValue( "HorizontalPadding" ) );
  if ( pcOpt->getValue( "VerticalPadding" ) )               m_aiPad[1]      = atoi( pcOpt->getValue( "VerticalPadding" ) );

  if ( pcOpt->getValue( "HierarchicalCoding" ) )            m_bHierarchicalCoding           = atoi( pcOpt->getValue( "HierarchicalCoding" ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "SymbolMode" ) )                    m_iSymbolMode                   = atoi( pcOpt->getValue( "SymbolMode" ) );
  if ( pcOpt->getValue( "MultiCodewordThreshold" ) )        m_uiMCWThreshold                = atoi( pcOpt->getValue( "MultiCodewordThreshold" ) );
  if ( pcOpt->getValue( "MaxPIPEBufferDelay" ) )            m_uiMaxPIPEDelay                = atoi( pcOpt->getValue( "MaxPIPEBufferDelay" ) );
  if ( pcOpt->getValue( "BalancedCPUs" ) )                  m_uiBalancedCPUs                = atoi( pcOpt->getValue( "BalancedCPUs" ) );

  if ( pcOpt->getValue( "LoopFilterDisable" ) )             m_bLoopFilterDisable            = atoi( pcOpt->getValue( "LoopFilterDisable" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "LoopFilterAlphaC0Offset" ) )       m_iLoopFilterAlphaC0Offset      = atoi( pcOpt->getValue( "LoopFilterAlphaC0Offset" ) );
  if ( pcOpt->getValue( "LoopFilterBetaOffset" ) )          m_iLoopFilterBetaOffset         = atoi( pcOpt->getValue( "LoopFilterBetaOffset" ) );

  if ( pcOpt->getValue( "FastSearch" ) )                    m_iFastSearch                   = atoi( pcOpt->getValue( "FastSearch" ) );
  if ( pcOpt->getValue( "SearchRange" ) )                   m_iSearchRange                  = atoi( pcOpt->getValue( "SearchRange" ) );
  if ( pcOpt->getValue( "MaxDeltaQP" ) )                    m_iMaxDeltaQP                   = atoi( pcOpt->getValue( "MaxDeltaQP" ) );
  if ( pcOpt->getValue( "HadamardME" ) )                    m_bUseHADME                     = atoi( pcOpt->getValue( "HadamardME" ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "MaxCUWidth" ) )                    m_uiMaxCUWidth                  = atoi( pcOpt->getValue( "MaxCUWidth" ) );
  if ( pcOpt->getValue( "MaxCUHeight" ) )                   m_uiMaxCUHeight                 = atoi( pcOpt->getValue( "MaxCUHeight" ) );
  if ( pcOpt->getValue( "MaxPartitionDepth" ) )             m_uiMaxCUDepth                  = atoi( pcOpt->getValue( "MaxPartitionDepth" ) );

#if HHI_RQT
  if ( pcOpt->getValue( "QuadtreeTUFlag"        ) )         m_bQuadtreeTUFlag               = atoi( pcOpt->getValue( "QuadtreeTUFlag"        ) ) != 0;
  if ( pcOpt->getValue( "QuadtreeTULog2MaxSize" ) )         m_uiQuadtreeTULog2MaxSize       = atoi( pcOpt->getValue( "QuadtreeTULog2MaxSize" ) );
  if ( pcOpt->getValue( "QuadtreeTULog2MinSize" ) )         m_uiQuadtreeTULog2MinSize       = atoi( pcOpt->getValue( "QuadtreeTULog2MinSize" ) );
#endif

  if ( pcOpt->getValue( "MinTrDepth" ) )                    m_uiMinTrDepth                  = atoi( pcOpt->getValue( "MinTrDepth" ) );
  if ( pcOpt->getValue( "MaxTrDepth" ) )                    m_uiMaxTrDepth                  = atoi( pcOpt->getValue( "MaxTrDepth" ) );

#if HHI_ALLOW_CIP_SWITCH
	if ( pcOpt->getValue( "CIP" ) )														m_bUseCIP												= atoi( pcOpt->getValue( "CIP" ) ) == 0 ? false : true;	
#endif
#if HHI_ALLOW_ROT_SWITCH
 	if ( pcOpt->getValue( "ROT" ) )														m_bUseROT												= atoi( pcOpt->getValue( "ROT" ) ) == 0 ? false : true;	 	
#endif 	
#if HHI_AIS
  if ( pcOpt->getValue( "AIS" ) )                           m_bUseAIS                       = atoi( pcOpt->getValue( "AIS" ) ) == 0 ? false : true; // BB: adaptive intra smoothing
#endif
#if HHI_MRG
  if ( pcOpt->getValue( "MRG" ) )                           m_bUseMRG                       = atoi( pcOpt->getValue( "MRG" ) ) == 0 ? false : true;  // SOPH:  Merge Mode
#endif
#if HHI_IMVP
  if ( pcOpt->getValue( "IMP" ) )                           m_bUseIMP                       = atoi( pcOpt->getValue( "IMP" ) ) == 0 ? false : true;  // SOPH:  Interleaved MV Predictor
#endif
  if ( pcOpt->getValue( "ALF" ) )                           m_bUseALF                       = atoi( pcOpt->getValue( "ALF"             ) ) == 0 ? false : true;

#if HHI_ALF
  if ( pcOpt->getValue( "ALFSeparateTree" ) )               m_bALFUseSeparateQT             = atoi( pcOpt->getValue( "ALFSeparateTree" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "ALFSymmetry" ) )                   m_bALFFilterSymmetry            = atoi( pcOpt->getValue( "ALFSymmetry"     ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "ALFMinLength" ) )                  m_iAlfMinLength                 = atoi( pcOpt->getValue( "ALFMinLength"    ) );
  if ( pcOpt->getValue( "ALFMaxLength" ) )                  m_iAlfMaxLength                 = atoi( pcOpt->getValue( "ALFMaxLength"    ) );
#endif

  if ( pcOpt->getValue( "BitDepth" ) )                      m_uiBitDepth                    = atoi( pcOpt->getValue( "BitDepth" ) );
  if ( pcOpt->getValue( "BitIncrement" ) )                  m_uiBitIncrement                = atoi( pcOpt->getValue( "BitIncrement" ) );

  if ( pcOpt->getValue( "DeltaQpRD" ) )                     m_uiDeltaQpRD                   = atoi( pcOpt->getValue( "DeltaQpRD"  ) );
  if ( pcOpt->getValue( "SBACRD" ) )                        m_bUseSBACRD                    = atoi( pcOpt->getValue( "SBACRD" ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "DIFTap" ) )                        m_iDIFTap                       = atoi( pcOpt->getValue( "DIFTap"  ) );
  if ( pcOpt->getValue( "ASR" ) )                           m_bUseASR                       = atoi( pcOpt->getValue( "ASR"     ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "GPB" ) )                           m_bUseGPB                       = atoi( pcOpt->getValue( "GPB"     ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "MaxTrSize" ) )                     m_uiMaxTrSize                   = atoi( pcOpt->getValue( "MaxTrSize" ) );

  if ( pcOpt->getValue( "RDOQ" ) )                          m_bUseRDOQ                      = atoi( pcOpt->getValue( "RDOQ"      ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "GRefMode" ) )        m_pchGRefMode     = pcOpt->getValue( "GRefMode" );
  if ( pcOpt->getValue( "LowDelayCoding" ) )                m_bUseLDC                       = atoi( pcOpt->getValue( "LowDelayCoding"    ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "PAD" ) )                           m_bUsePAD                       = atoi( pcOpt->getValue( "PAD"       ) ) == 0 ? false : true;

  if ( pcOpt->getValue( "QBO" ) )                           m_bUseQBO                       = atoi( pcOpt->getValue( "QBO"       ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "NRF" ) )                           m_bUseNRF                       = atoi( pcOpt->getValue( "NRF"       ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "BQP" ) )                           m_bUseBQP                       = atoi( pcOpt->getValue( "BQP"       ) ) == 0 ? false : true;
  if ( pcOpt->getValue( "FEN" ) )                           m_bUseFastEnc                   = atoi( pcOpt->getValue( "FEN"       ) ) == 0 ? false : true;

#if HHI_INTERP_FILTER
  if ( pcOpt->getValue( "InterpFilterType" ) )              m_iInterpFilterType             = atoi( pcOpt->getValue( "InterpFilterType" ) );
#endif
#ifdef QC_AMVRES
	if ( pcOpt->getValue( "AMVRES" ) )														m_bUseAMVRes												= atoi( pcOpt->getValue( "AMVRES"       ) ) == 0 ? false : true;
#endif
#ifdef QC_SIFO_PRED
	if ( pcOpt->getValue( "SPF" ) )														m_bUseSIFO_Pred									= atoi( pcOpt->getValue( "SPF"       ) ) == 0 ? false : true;
#endif
  return;
}

Void TAppEncCfg::xSetCfgCommand( int iArgc, char** pArgv )
{
  Int    i;
  char*  pStr;
  char*  pToolName;

  i=1;
  while ( i < iArgc )
  {
    pStr = pArgv[i];
    if      ( !strcmp( pStr, "-c" ) )   { i++; i++; }
    else if ( !strcmp( pStr, "-i" ) )   { i++; m_pchInputFile         = pArgv[i];         i++; }
    else if ( !strcmp( pStr, "-b" ) )   { i++; m_pchBitstreamFile     = pArgv[i];         i++; }
    else if ( !strcmp( pStr, "-o" ) )   { i++; m_pchReconFile         = pArgv[i];         i++; }
#ifdef QC_CONFIG
		else if ( !strcmp( pStr, "-wdt" ) )	{	i++; m_iSourceWidth					= atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-hgt" ) )	{	i++; m_iSourceHeight				= atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-fr" ) )	{	i++; m_iFrameRate					  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-fs" ) )	{	i++; m_iFrameSkip					  = atoi( pArgv[i] ); i++; }
#endif
    else if ( !strcmp( pStr, "-m" ) )   { i++; m_pchdQPFile           = pArgv[i];         i++; }
    else if ( !strcmp( pStr, "-v" ) )   { i++; m_pchGRefMode        = pArgv[i];         i++; }
    else if ( !strcmp( pStr, "-f" ) )   { i++; m_iFrameToBeEncoded    = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-q" ) )   { i++; m_fQP                  = atof( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-g" ) )   { i++; m_iGOPSize             = atoi( pArgv[i] ); i++; m_iRateGOPSize = m_iGOPSize; }
    else if ( !strcmp( pStr, "-rg" ) )  { i++; m_iRateGOPSize         = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-r" ) )   { i++; m_iNumOfReference      = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-rb0" ) ) { i++; m_iNumOfReferenceB_L0  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-rb1" ) ) { i++; m_iNumOfReferenceB_L1  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-ip" ) )  { i++; m_iIntraPeriod         = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-d" ) )   { i++; m_iMaxDeltaQP          = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-dqr" ) ) { i++; m_uiDeltaQpRD          = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-s" ) )   { i++; m_uiMaxCUWidth         = m_uiMaxCUHeight  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-h" ) )   { i++; m_uiMaxCUDepth         = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-t" ) )   { i++; m_uiMaxTrSize          = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-ltd" ) ) { i++; m_uiMinTrDepth         = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-utd" ) ) { i++; m_uiMaxTrDepth         = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tq0" ) ) { i++; m_aiTLayerQPOffset[0]  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tq1" ) ) { i++; m_aiTLayerQPOffset[1]  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tq2" ) ) { i++; m_aiTLayerQPOffset[2]  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tq3" ) ) { i++; m_aiTLayerQPOffset[3]  = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-pdx" ) ) { i++; m_aiPad[0]             = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-pdy" ) ) { i++; m_aiPad[1]             = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-sym" ) )  { i++; m_iSymbolMode          = atoi( pArgv[i] );  i++; }
    else if ( !strcmp( pStr, "-ldm" ) ) { i++; m_iLDMode              = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-tap" ) ) { i++; m_iDIFTap              = atoi( pArgv[i] ); i++; }
    else if ( !strcmp( pStr, "-sr"  ) )  { i++; m_iSearchRange         = atoi( pArgv[i] ); i++; }
#if TEN_DIRECTIONAL_INTERP
    else if ( !strcmp( pStr, "-int"  ) )  { i++; m_iInterpFilterType  = atoi( pArgv[i] ); i++; }
#endif
    else
    if ( !strcmp( pStr, "-1" ) )
    {
      i++;
      pToolName = pArgv[i];
      if ( !strcmp( pToolName, "ASR" ) )  m_bUseASR         = true;
      else
#if HHI_ALLOW_CIP_SWITCH
      if ( !strcmp( pToolName, "CIP" ) )	m_bUseCIP         = true;
      else
#endif

#if HHI_ALLOW_ROT_SWITCH
      if ( !strcmp( pToolName, "ROT" ) )	m_bUseROT         = true;
      else
#endif
#if HHI_AIS
      if ( !strcmp( pToolName, "AIS" ) )  m_bUseAIS         = true; // BB: adaptive intra smoothing
      else
#endif      
#if HHI_MRG
      if ( !strcmp( pToolName, "MRG" ) )  m_bUseMRG         = true; // SOPH: Merge Mode
      else
#endif
#if HHI_IMVP
      if ( !strcmp( pToolName, "IMP" ) )  m_bUseIMP         = true; // SOPH: Interleaved MV Predictor
      else
#endif
      if ( !strcmp( pToolName, "ALF" ) )  m_bUseALF         = true;
      else
      if ( !strcmp( pToolName, "IBD" ) )  m_uiBitIncrement  = 4;
      else
      if ( !strcmp( pToolName, "HAD" ) )  m_bUseHADME       = true;
      else
      if ( !strcmp( pToolName, "SRD" ) )  m_bUseSBACRD      = true;
      else
      if ( !strcmp( pToolName, "GPB" ) )  m_bUseGPB         = true;
      else
      if ( !strcmp( pToolName, "RDQ" ) )  m_bUseRDOQ        = true;
      else
      if ( !strcmp( pToolName, "LDC" ) )  m_bUseLDC         = true;
      else
      if ( !strcmp( pToolName, "PAD" ) )  m_bUsePAD         = true;
      else
      if ( !strcmp( pToolName, "QBO" ) )  m_bUseQBO         = true;
      else
      if ( !strcmp( pToolName, "NRF" ) )  m_bUseNRF         = true;
      else
      if ( !strcmp( pToolName, "BQP" ) )  m_bUseBQP         = true;
      else
      if ( !strcmp( pToolName, "FEN" ) )
      {
  m_bUseFastEnc  = true;
  m_bUseASR       = true;
      }
#ifdef QC_AMVRES
      else
      if ( !strcmp( pToolName, "AMVRES" ) )	m_bUseAMVRes         = true;
#endif
#ifdef QC_SIFO_PRED
      else
      if ( !strcmp( pToolName, "SPF" ) )	m_bUseSIFO_Pred   = true;
#endif
      i++;
    }
    else
    if ( !strcmp( pStr, "-0" ) )
    {
      i++;
      pToolName = pArgv[i];
      if ( !strcmp( pToolName, "ASR" ) )  m_bUseASR         = false;
      else
#if HHI_ALLOW_CIP_SWITCH
      if ( !strcmp( pToolName, "CIP" ) )	m_bUseCIP         = false;
      else
#endif

#if HHI_ALLOW_ROT_SWITCH
      if ( !strcmp( pToolName, "ROT" ) )	m_bUseROT         = false;
      else
#endif
#if HHI_AIS
      if ( !strcmp( pToolName, "AIS" ) )  m_bUseAIS         = false; // BB: adaptive intra smoothing
      else
#endif     
#if HHI_MRG
      if ( !strcmp( pToolName, "MRG" ) )  m_bUseMRG         = false; // SOPH:  Merge Mode
      else
#endif
#if HHI_IMVP
      if ( !strcmp( pToolName, "IMP" ) )  m_bUseIMP         = false; // SOPH: Interleaved MV Predictor
      else
#endif
      if ( !strcmp( pToolName, "ALF" ) )  m_bUseALF         = false;
      else
      if ( !strcmp( pToolName, "IBD" ) )  m_uiBitIncrement  = 0;
      else
      if ( !strcmp( pToolName, "HAD" ) )  m_bUseHADME       = false;
      else
      if ( !strcmp( pToolName, "SRD" ) )  m_bUseSBACRD      = false;
      else
      if ( !strcmp( pToolName, "GPB" ) )  m_bUseGPB         = false;
      else
      if ( !strcmp( pToolName, "RDQ" ) )  m_bUseRDOQ        = false;
      else
      if ( !strcmp( pToolName, "LDC" ) )  m_bUseLDC         = false;
      else
      if ( !strcmp( pToolName, "PAD" ) )  m_bUsePAD         = false;
      else
      if ( !strcmp( pToolName, "QBO" ) )  m_bUseQBO         = false;
      else
      if ( !strcmp( pToolName, "NRF" ) )  m_bUseNRF         = false;
      else
      if ( !strcmp( pToolName, "BQP" ) )  m_bUseBQP         = false;
      else
      if ( !strcmp( pToolName, "FEN" ) )  m_bUseFastEnc     = false;
#ifdef QC_AMVRES
      else
      if ( !strcmp( pToolName, "AMVRES" ) )	m_bUseAMVRes         = false;
#endif
#ifdef QC_SIFO_PRED
      else
      if ( !strcmp( pToolName, "SPF" ) )	m_bUseSIFO_Pred   = false;
#endif
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
      m_bUseQBO = true;       // use quality-based reference reordering
      m_bUseBQP = false;      // use hier-P QP
      m_bUseNRF = true;       // use non-reference marking
    }
    else
    if ( m_iLDMode == 1 )
    {
      m_bUseQBO = false;      // don't use quality-based reference reordering
      m_bUseBQP = true;       // user hier-P QP
      m_bUseNRF = false;      // don't use non-reference marking
    }
  }
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File          : %s\n", m_pchInputFile          );
  printf("Bitstream      File          : %s\n", m_pchBitstreamFile      );
  printf("Reconstruction File          : %s\n", m_pchReconFile          );
  printf("Real     Format              : %dx%d %dHz\n", m_iSourceWidth - m_aiPad[0], m_iSourceHeight-m_aiPad[1], m_iFrameRate );
  printf("Internal Format              : %dx%d %dHz\n", m_iSourceWidth, m_iSourceHeight, m_iFrameRate );
  printf("Frame index                  : %d - %d (%d frames)\n", m_iFrameSkip, m_iFrameSkip+m_iFrameToBeEncoded-1, m_iFrameToBeEncoded );
  printf("Number of Ref. frames (P)    : %d\n", m_iNumOfReference);
  printf("Number of Ref. frames (B_L0) : %d\n", m_iNumOfReferenceB_L0);
  printf("Number of Ref. frames (B_L1) : %d\n", m_iNumOfReferenceB_L1);
  printf("Number of Reference frames   : %d\n", m_iNumOfReference);
  printf("CU size / depth              : %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth );
  printf("Transform depth (min / max)  : %d / %d\n", m_uiMinTrDepth, m_uiMaxTrDepth );
  printf("Motion search range          : %d\n", m_iSearchRange );
  printf("Intra period                 : %d\n", m_iIntraPeriod );
  printf("QP                           : %5.2f\n", m_fQP );
  printf("GOP size                     : %d\n", m_iGOPSize );
  printf("Rate GOP size                : %d\n", m_iRateGOPSize );
  printf("Max physical trans. size     : %d\n", m_uiMaxTrSize );
  printf("Bit increment                : %d\n", m_uiBitIncrement );

#if HHI_INTERP_FILTER
  switch ( m_iInterpFilterType )
  {
#if TEN_DIRECTIONAL_INTERP
    case IPF_TEN_DIF:
      printf("Luma interpolation           : %s\n", "TEN directional interpolation filter"  );
      printf("Chroma interpolation         : %s\n", "TEN two-stage bi-linear filter"  );
      break;
#endif
    case IPF_HHI_4TAP_MOMS:
      printf("Luma interpolation           : %s\n", "HHI 4-tap MOMS filter"  );
      printf("Chroma interpolation         : %s\n", "HHI 4-tap MOMS filter"  );
      break;
    case IPF_HHI_6TAP_MOMS:
      printf("Luma interpolation           : %s\n", "HHI 6-tap MOMS filter"  );
      printf("Chroma interpolation         : %s\n", "HHI 6-tap MOMS filter"  );
      break;
#ifdef QC_SIFO   
    case IPF_QC_SIFO:
      printf("Luma   interpolation         : Qualcomm %d-tap SIFO\n", m_iDIFTap  );
      printf("Chroma interpolation         : %s\n", "Bi-linear filter"       );
      break;
#endif
    default:
#ifdef QC_CONFIG
      printf("Luma   interpolation         : Samsung %d-tap filter\n", m_iDIFTap  );
      printf("Chroma interpolation         : %s\n", "Bi-linear filter"       );
#else
      printf("Luma interpolation           : %s\n", "Samsung 12-tap filter"  );
      printf("Chroma interpolation         : %s\n", "Bi-linear filter"       );
#endif
  }
#else
  printf("Number of int. taps (luma)   : %d\n", m_iDIFTap  );
  printf("Number of int. taps (chroma) : %d\n", 2          );
#endif

  if ( m_pchGRefMode != NULL )
  {
    printf("Additional reference frame   : weighted prediction\n");
  }

  if ( m_iSymbolMode == 0 )
  {
    printf("Entropy coder                : VLC\n");
  }
  else if( m_iSymbolMode == 1 )
  {
    printf("Entropy coder                : CABAC\n");
  }
  else if( m_iSymbolMode == 2 )
  {
    printf("Entropy coder                : PIPE\n");
  }
  else
  {
    printf("Entropy coder                : V2V with load balancing on %d bin decoders\n", m_uiBalancedCPUs);
  }

  printf("\n");

  printf("TOOL CFG: ");
  printf("ALF:%d ", m_bUseALF             );
  printf("IBD:%d ", m_uiBitIncrement!=0   );
  printf("HAD:%d ", m_bUseHADME           );
  printf("SRD:%d ", m_bUseSBACRD          );
  printf("RDQ:%d ", m_bUseRDOQ            );
  printf("SQP:%d ", m_uiDeltaQpRD         );
  printf("ASR:%d ", m_bUseASR             );
  printf("PAD:%d ", m_bUsePAD             );
  printf("LDC:%d ", m_bUseLDC             );
  printf("NRF:%d ", m_bUseNRF             );
  printf("BQP:%d ", m_bUseBQP             );
  printf("QBO:%d ", m_bUseQBO             );
  printf("GPB:%d ", m_bUseGPB             );
  printf("FEN:%d ", m_bUseFastEnc         );
#if HHI_RQT
  printf("RQT:%d ", m_bQuadtreeTUFlag     );
#endif
#if HHI_ALLOW_CIP_SWITCH
  printf("CIP:%d ", m_bUseCIP             );
#endif
#if HHI_ALLOW_ROT_SWITCH
  printf("ROT:%d ", m_bUseROT             );
#endif
#if HHI_AIS
  printf("AIS:%d ", m_bUseAIS             ); // BB: adaptive intra smoothing
#endif
#if HHI_MRG
  printf("MRG:%d ", m_bUseMRG             ); // SOPH: Merge Mode
#endif
#if HHI_IMVP
  printf("IMP:%d ", m_bUseIMP             ); // SOPH: Interleaved MV Predictor
#endif
#ifdef QC_AMVRES
	printf("AMVRES:%d ", m_bUseAMVRes							);
#endif
#ifdef QC_SIFO_PRED
	printf("SPF:%d ", m_bUseSIFO_Pred			);
#endif
  printf("\n");

  fflush(stdout);
}

Void TAppEncCfg::xPrintUsage()
{
  printf("\n");

  printf( "  -c      configuration file name\n" );
  printf( "  -i      original YUV input file name\n" );
  printf( "  -o      decoded YUV output file name\n" );
  printf( "  -b      bitstream file name\n" );
#ifdef QC_CONFIG
  printf( "  -wdt    Source Width\n" );
  printf( "  -hgt    Source Height\n" );
  printf( "  -fr     Frame Rate\n" );
  printf( "  -fs     Frame Skip\n" );
#endif
  printf( "  -m      dQP file name\n" );
  printf( "  -f      number of frames to be encoded (default EOS)\n" );
  printf( "  -q      Qp value, if value is float, QP is switched once during encoding\n" );
  printf( "  -g      GOP size of temporal structure\n" );
  printf( "  -rg     GOP size of hierarchical QP assignment\n" );
  printf( "  -s      max CU size\n" );
  printf( "  -h      CU depth\n" );
  printf( "  -r      Number of reference (P)\n" );
  printf( "  -rb0    Number of reference (B_L0)\n" );
  printf( "  -rb1    Number of reference (B_L1)\n" );
  printf( "  -ip     intra period in frames, (-1: only first frame)\n" );
  printf( "  -ldm    recommended low-delay setting (with LDC), (0=slow sequence, 1=fast sequence)\n" );
  printf( "  -d      max dQp offset for block\n");
  printf( "  -dqr    max dQp offset for slice\n");
  printf( "  -t      max transform size\n" );
  printf( "  -ltd    min transform depth\n" );
  printf( "  -utd    max transform depth\n" );
  printf( "  -v      additional reference for weighted prediction (w: scale+offset, o: offset)\n" );
  printf( "  -tap    number of interpolation filter taps (luma)\n");
  printf( "  -tq0    QP offset of temporal layer 0\n" );
  printf( "  -tq1    QP offset of temporal layer 1\n" );
  printf( "  -tq2    QP offset of temporal layer 2\n" );
  printf( "  -tq3    QP offset of temporal layer 3\n" );
  printf( "  -pdx    horizontal source padding size\n");
  printf( "  -pdy    vertical source padding size\n");
  printf( "  -sym    symbol mode (0=VLC, 1=SBAC)\n");
  printf( "  -sr     motion search range\n");
  printf( "  -1/0    <name>: turn on/off <name>\n");
  printf( "          <name> = ALF - adaptive loop filter\n");
  printf( "                   IBD - bit-depth increasement\n");
  printf( "                   GPB - generalized B instead of P in low-delay mode\n");
  printf( "                   HAD - hadamard ME for fractional-pel\n");
  printf( "                   SRD - SBAC based RD estimation\n");
  printf( "                   RDQ - RDOQ\n");
  printf( "                   LDC - low-delay mode\n");
  printf( "                   NRF - non-reference frame marking in last layer\n");
  printf( "                   BQP - hier-P style QP assignment in low-delay mode\n");
  printf( "                   PAD - automatic source padding of multiple of 16\n");
  printf( "                   QBO - skip refers highest quality picture\n");
  printf( "                   ASR - adaptive motion search range\n");
  printf( "                   FEN - fast encoder setting\n");  
#if HHI_AIS
  printf( "                   AIS - adaptive intra smoothing\n"); // BB: adaptive intra smoothing
#endif
#if HHI_MRG
  printf( "                   MRG - merging of motion partitions\n"); // SOPH: Merge Mode
#endif
#if HHI_IMVP
  printf( "                   IMP - interleaved motion vector predictor\n"); /// SOPH: Interleaved MV Predictor
#endif
#ifdef QC_AMVRES
	printf( "                   AMVRES - Adaptive motion resolution\n");
#endif
  printf( "\n" );
  printf( "  Example 1) TAppEncoder.exe -c test.cfg -q 32 -g 8 -f 9 -s 64 -h 4\n");
  printf("              -> QP 32, hierarchical-B GOP 8, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
  printf( "  Example 2) TAppEncoder.exe -c test.cfg -q 32 -g 4 -f 9 -s 64 -h 4 -1 LDC\n");
  printf("              -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
  printf( "  Example 3) TAppEncoder.exe -c test.cfg -q 32 -g 1 -f 9 -s 64 -h 4 -1 LDC -ldm 0 -rg 4\n");
  printf("              -> QP 32, IPPP with hierarchical-P of GOP 4 style QP, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
  printf( "  Example 4) TAppEncoder.exe -c test.cfg -q 32 -g 1 -f 9 -s 64 -h 4 -1 LDC -ldm 1 -rg 4\n");
  printf("              -> QP 32, IPPP with hierarchical-B of GOP 4 style QP, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
}

Void TAppEncCfg::xConfirmPara(Bool bflag, const char* message)
{
  if( bflag )
  {
    printf("\n%s\n",message);
    exit(0);
  }
}

