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

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include "TAppEncCfg.h"
#include "../../App/TAppCommon/program_options_lite.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

/* configuration helper funcs */
void doOldStyleCmdlineLDM(po::Options& opts, const std::string& arg);
void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg);
void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg);

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
  m_aidQP = NULL;
  m_pchGRefMode = NULL;
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
}

Void TAppEncCfg::destroy()
{
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
  bool do_help = false;

  string cfg_InputFile;
  string cfg_BitstreamFile;
  string cfg_ReconFile;
  string cfg_dQPFile;
  string cfg_GRefMode;
  po::Options opts;
  opts.addOptions()
    ("help", do_help, false, "this help text")
    ("c", po::parseConfigFile, "configuration file name")

    /* File, I/O and source parameters */
    ("InputFile,i",     cfg_InputFile,     string(""), "original YUV input file name")
    ("BitstreamFile,b", cfg_BitstreamFile, string(""), "bitstream output file name")
    ("ReconFile,o",     cfg_ReconFile,     string(""), "reconstructed YUV output file name")

    ("SourceWidth,wdt",       m_iSourceWidth,  0, "Source picture width")
    ("SourceHeight,hgt",      m_iSourceHeight, 0, "Source picture height")
    ("BitDepth",              m_uiBitDepth,    8u)
    ("BitIncrement",          m_uiBitIncrement,4u, "bit-depth increasement")
    ("HorizontalPadding,pdx", m_aiPad[0],      0, "horizontal source padding size")
    ("VerticalPadding,pdy",   m_aiPad[1],      0, "vertical source padding size")
    ("PAD",                   m_bUsePAD,   false, "automatic source padding of multiple of 16" )
    ("FrameRate,fr",          m_iFrameRate,        0, "Frame rate")
    ("FrameSkip,fs",          m_iFrameSkip,        0, "Number of frames to skip at start of input YUV")
    ("FramesToBeEncoded,f",   m_iFrameToBeEncoded, 0, "number of frames to be encoded (default=all)")
    ("FrameToBeEncoded",      m_iFrameToBeEncoded, 0, "depricated alias of FramesToBeEncoded")

    /* Unit definition parameters */
    ("MaxCUWidth",          m_uiMaxCUWidth,  64u)
    ("MaxCUHeight",         m_uiMaxCUHeight, 64u)
    /* todo: remove defaults from MaxCUSize */
    ("MaxCUSize,s",         m_uiMaxCUWidth,  64u, "max CU size")
    ("MaxCUSize,s",         m_uiMaxCUHeight, 64u, "max CU size")
    ("MaxPartitionDepth,h", m_uiMaxCUDepth,   4u, "CU depth")

    ("MaxTrSize,t",    m_uiMaxTrSize, 64u, "max transform size")
    ("MaxTrDepth,utd", m_uiMaxTrDepth, 1u, "max transform depth")
    ("MinTrDepth,ltd", m_uiMinTrDepth, 0u, "min transform depth")

#if HHI_RQT
    ("QuadtreeTUFlag", m_bQuadtreeTUFlag, true)
    ("QuadtreeTULog2MaxSize", m_uiQuadtreeTULog2MaxSize, 6u)
    ("QuadtreeTULog2MinSize", m_uiQuadtreeTULog2MinSize, 2u)
#endif

    /* Coding structure paramters */
    ("IntraPeriod,ip", m_iIntraPeriod, -1, "intra period in frames, (-1: only first frame)")
    ("GOPSize,g",      m_iGOPSize,      1, "GOP size of temporal structure")
    ("RateGOPSize,rg", m_iRateGOPSize, -1, "GOP size of hierarchical QP assignment")
    ("NumOfReference,r",       m_iNumOfReference,     1, "Number of reference (P)")
    ("NumOfReferenceB_L0,rb0", m_iNumOfReferenceB_L0, 1, "Number of reference (B_L0)")
    ("NumOfReferenceB_L1,rb1", m_iNumOfReferenceB_L1, 1, "Number of reference (B_L1)")
    ("HierarchicalCoding",     m_bHierarchicalCoding, true)
    ("LowDelayCoding",         m_bUseLDC,             false, "low-delay mode")
    ("GPB", m_bUseGPB, false, "generalized B instead of P in low-delay mode")
    ("QBO", m_bUseQBO, false, "skip refers highest quality picture")
    ("NRF", m_bUseNRF,  true, "non-reference frame marking in last layer")
    ("BQP", m_bUseBQP, false, "hier-P style QP assignment in low-delay mode")
    ("-ldm", doOldStyleCmdlineLDM, "recommended low-delay setting (with LDC), (0=slow sequence, 1=fast sequence)")

    /* Interpolation filter options */
#if HHI_INTERP_FILTER
    ("InterpFilterType,-int", m_iInterpFilterType, (Int)IPF_SAMSUNG_DIF_DEFAULT)
#endif
    ("DIFTap,tap", m_iDIFTap, 12, "number of interpolation filter taps (luma)")
#ifdef QC_SIFO_PRED
    ("SPF",m_bUseSIFO_Pred, true)
#endif

    /* motion options */
    ("FastSearch", m_iFastSearch, 1, "0:Full search  1:Diamond  2:PMVFAST")
    ("SearchRange,sr", m_iSearchRange, 96, "motion search range")
    ("HadamardME", m_bUseHADME, true, "hadamard ME for fractional-pel")
    ("ASR", m_bUseASR, false, "adaptive motion search range")
#ifdef QC_AMVRES
    ("AMVRES", m_bUseAMVRes, true, "Adaptive motion resolution")
#endif
    ("GRefMode,v", cfg_GRefMode, string(""), "additional reference for weighted prediction (w: scale+offset, o: offset)")

    /* Quantization parameters */
    ("QP,q",          m_fQP,             30.0, "Qp value, if value is float, QP is switched once during encoding")
    ("DeltaQpRD,dqr", m_uiDeltaQpRD,       0u, "max dQp offset for slice")
    ("MaxDeltaQP,d",  m_iMaxDeltaQP,        0, "max dQp offset for block")
    ("dQPFile,m",     cfg_dQPFile, string(""), "dQP file name")
    ("RDOQ",          m_bUseRDOQ, true)
    ("TemporalLayerQPOffset_L0,tq0", m_aiTLayerQPOffset[0], MAX_QP + 1, "QP offset of temporal layer 0")
    ("TemporalLayerQPOffset_L1,tq1", m_aiTLayerQPOffset[1], MAX_QP + 1, "QP offset of temporal layer 1")
    ("TemporalLayerQPOffset_L2,tq2", m_aiTLayerQPOffset[2], MAX_QP + 1, "QP offset of temporal layer 2")
    ("TemporalLayerQPOffset_L3,tq3", m_aiTLayerQPOffset[3], MAX_QP + 1, "QP offset of temporal layer 3")

    /* Entropy coding parameters */
    ("SymbolMode,sym", m_iSymbolMode, 1, "symbol mode (0=VLC, 1=SBAC)")
    ("SBACRD", m_bUseSBACRD, true, "SBAC based RD estimation")
    ("MultiCodewordThreshold", m_uiMCWThreshold, 0u)
    ("MaxPIPEBufferDelay", m_uiMaxPIPEDelay, 0u)
    ("BalancedCPUs", m_uiBalancedCPUs, 8u)

    /* Deblocking filter parameters */
    ("LoopFilterDisable", m_bLoopFilterDisable, false)
    ("LoopFilterAlphaC0Offset", m_iLoopFilterAlphaC0Offset, 0)
    ("LoopFilterBetaOffset", m_iLoopFilterBetaOffset, 0 )

    /* Coding tools */
#if HHI_ALLOW_CIP_SWITCH
    ("CIP", m_bUseCIP, true, "combined intra prediction")
#endif
#if HHI_AIS
    ("AIS", m_bUseAIS, true, "adaptive intra smoothing")
#endif
#if HHI_MRG
    ("MRG", m_bUseMRG, true, "merging of motion partitions")
#endif
#if HHI_IMVP
    ("IMP", m_bUseIMP, true, "interleaved motion vector predictor")
#endif
#if HHI_ALLOW_ROT_SWITCH
    ("ROT", m_bUseROT, true)
#endif
    ("ALF", m_bUseALF, true, "Adaptive Loop Filter")
#if HHI_ALF
    ("ALFSeparateTree", m_bALFUseSeparateQT, true)
    ("ALFSymmetry", m_bALFFilterSymmetry, false)
    ("ALFMinLength", m_iAlfMinLength, 5)
    ("ALFMaxLength", m_iAlfMaxLength, 9)
#endif

    /* Misc. */
    ("FEN", m_bUseFastEnc, false, "fast encoder setting")

    /* Compatability with old style -1 FOO or -0 FOO options. */
    ("1", doOldStyleCmdlineOn, "turn option <name> on")
    ("0", doOldStyleCmdlineOff, "turn option <name> off")
    ;

  po::setDefaults(opts);
  po::scanArgv(opts, argc, (const char**) argv);

  if (argc == 1 || do_help) {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    xPrintUsage();
    return false;
  }

  /*
   * Set any derived parameters
   */
  /* convert std::string to c string for compatability */
  m_pchInputFile = cfg_InputFile.empty() ? NULL : strdup(cfg_InputFile.c_str());
  m_pchBitstreamFile = cfg_BitstreamFile.empty() ? NULL : strdup(cfg_BitstreamFile.c_str());
  m_pchReconFile = cfg_ReconFile.empty() ? NULL : strdup(cfg_ReconFile.c_str());
  m_pchdQPFile = cfg_dQPFile.empty() ? NULL : strdup(cfg_dQPFile.c_str());
  m_pchGRefMode = cfg_GRefMode.empty() ? NULL : strdup(cfg_GRefMode.c_str());

  if (m_iRateGOPSize == -1) {
    /* if rateGOPSize has not been specified, the default value is GOPSize */
    m_iRateGOPSize = m_iGOPSize;
  }

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

/* helper for -ldm */
void doOldStyleCmdlineLDM(po::Options& opts, const std::string& arg)
{
    /* xxx: warn this option is depricated */
    if (arg == "1") {
        po::storePair(opts, "QBO", "1");
        po::storePair(opts, "BQP", "0");
        po::storePair(opts, "NRF", "1");
        return;
    }
    if (arg == "0") {
        po::storePair(opts, "QBO", "0");
        po::storePair(opts, "BQP", "1");
        po::storePair(opts, "NRF", "0");
        return;
    }
    /* invalid parse */
    cerr << "Invalid argument to -ldm: `" << arg << "'" << endl;
}

/* helper function */
/* for handling "-1/-0 FOO" */
void translateOldStyleCmdline(const char* value, po::Options& opts, const std::string& arg)
{
	const char* argv[] = {arg.c_str(), value};
	/* replace some short names with their long name varients */
	if (arg == "LDC") {
		argv[0] = "LowDelayCoding";
	}
	else if (arg == "RDQ") {
		argv[0] = "RDOQ";
	}
	else if (arg == "HAD") {
		argv[0] = "HadamardME";
	}
	else if (arg == "SRD") {
		argv[0] = "SBACRD";
	}
	else if (arg == "IBD") {
		argv[0] = "BitIncrement";
	}
	/* issue a warning for change in FEN behaviour */
	if (arg == "FEN") {
		/* xxx todo */
	}
	po::storePair(opts, argv[0], argv[1]);
}

void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg)
{
	if (arg == "IBD") {
		translateOldStyleCmdline("4", opts, arg);
		return;
	}
	translateOldStyleCmdline("1", opts, arg);
}
void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg)
{
	translateOldStyleCmdline("0", opts, arg);
}
