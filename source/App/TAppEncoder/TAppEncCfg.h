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

/** \file     TAppEncCfg.h
    \brief    Handle encoder configuration parameters (header)
*/

#ifndef __TAPPENCCFG__
#define __TAPPENCCFG__

#include "../../Lib/TLibCommon/CommonDef.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TAppEncCfg
{
protected:
  // file I/O
  char*     m_pchInputFile;                                   ///< source file name
  char*     m_pchBitstreamFile;                               ///< output bitstream file
  char*     m_pchReconFile;                                   ///< output reconstruction file

  // source specification
  Int       m_iFrameRate;                                     ///< source frame-rates (Hz)
  Int       m_iFrameSkip;                                     ///< number of skipped frames from the beginning
  Int       m_iSourceWidth;                                   ///< source width in pixel
  Int       m_iSourceHeight;                                  ///< source height in pixel
  Int       m_iFrameToBeEncoded;                              ///< number of encoded frames
  Bool      m_bUsePAD;                                        ///< flag for using source padding
  Int       m_aiPad[2];                                       ///< number of padded pixels for width and height

  // coding structure
  Int       m_iIntraPeriod;                                   ///< period of I-slice (random access period)
  Int       m_iGOPSize;                                       ///< GOP size of hierarchical structure
  Int       m_iRateGOPSize;                                   ///< GOP size for QP variance
  Int       m_iNumOfReference;                                ///< total number of reference frames in P-slice
  Int       m_iNumOfReferenceB_L0;                            ///< total number of reference frames for reference list L0 in B-slice
  Int       m_iNumOfReferenceB_L1;                            ///< total number of reference frames for reference list L1 in B-slice
  Bool      m_bHierarchicalCoding;                            ///< flag for specifying hierarchical B structure
  Bool      m_bUseLDC;                                        ///< flag for using low-delay coding mode
  Bool      m_bUseNRF;                                        ///< flag for using non-referenced frame in hierarchical structure
  Bool      m_bUseGPB;                                        ///< flag for using generalized P & B structure
  Bool      m_bUseQBO;                                        ///< flag for using quality-based reference ordering for skip mode
#ifdef QC_AMVRES
	Bool      m_bUseAMVRes;
#endif
#ifdef QC_SIFO_PRED
  Bool      m_bUseSIFO_Pred;
#endif
  // coding quality
  Double    m_fQP;                                            ///< QP value of key-picture (floating point)
  Int       m_iQP;                                            ///< QP value of key-picture (integer)
  Int       m_aiTLayerQPOffset[MAX_TLAYER];                   ///< QP offset corresponding to temporal layer depth
  char*     m_pchdQPFile;                                     ///< QP offset for each slice (initialized from external file)
  Int*      m_aidQP;                                          ///< array of slice QP values
  Int       m_iMaxDeltaQP;                                    ///< max. |delta QP|
  UInt      m_uiDeltaQpRD;                                    ///< dQP range for multi-pass slice QP optimization

  // coding unit (CU) definition
  UInt      m_uiMaxCUWidth;                                   ///< max. CU width in pixel
  UInt      m_uiMaxCUHeight;                                  ///< max. CU height in pixel
  UInt      m_uiMaxCUDepth;                                   ///< max. CU depth

  // transfom unit (TU) definition
#if HHI_RQT
  Bool      m_bQuadtreeTUFlag;
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
#endif
  UInt      m_uiMinTrDepth;                                   ///< min. TU depth
  UInt      m_uiMaxTrDepth;                                   ///< max. TU depth
  UInt      m_uiMaxTrSize;                                    ///< max. physical transform size

  // coding tools (bit-depth)
  UInt      m_uiBitDepth;                                     ///< base bit-depth
  UInt      m_uiBitIncrement;                                 ///< bit-depth increment

  // coding tools (inter - motion)
  char*     m_pchGRefMode;                                    ///< array of generated reference modes

  // coding tools (inter - interpolation filter)
  Int       m_iDIFTap;                                        ///< number of taps in DIF (luma)

  // coding tools (loop filter)
  Bool      m_bUseALF;                                        ///< flag for using adaptive loop filter

#if HHI_ALF
  Bool      m_bALFUseSeparateQT;                              ///< flag for using a separate quad tree
  Bool      m_bALFFilterSymmetry;                             ///< flag for using symmetric filter
  Int       m_iAlfMinLength;                                  ///< minimum filter length to test
  Int       m_iAlfMaxLength;                                  ///< maximum filter length to test
#endif

  Bool      m_bLoopFilterDisable;                             ///< flag for using deblocking filter
  Int       m_iLoopFilterAlphaC0Offset;                       ///< alpha offset for deblocking filter
  Int       m_iLoopFilterBetaOffset;                          ///< beta offset for deblocking filter

  // coding tools (entropy coder)
  Int       m_iSymbolMode;                                    ///< entropy coder mode, 0 = VLC, 1 = CABAC, 2 = PIPE, 3 = V2V with load balancing
  UInt      m_uiMCWThreshold;                                 ///< threshold in bits for multi-codeword coding (CABAC & PIPE)
  UInt      m_uiMaxPIPEDelay;                                 ///< maximum buffer delay for single-codeword PIPE
  UInt      m_uiBalancedCPUs;                                 ///< number of CPUs for load balancing: 0 or 1 - ignored

	// coding tools (intra)
#if HHI_ALLOW_CIP_SWITCH
	Bool			m_bUseCIP;																				///< flag for using combined intra prediction
#endif  
#if HHI_AIS
  Bool      m_bUseAIS;                                        ///< BB: flag for using adaptive intra smoothing
#endif  
	// coding tools (transform)
	Bool			m_bUseROT;																				///< flag for using rotational transform

#if HHI_MRG
  // coding tools (inter - merge motion partitions)
  Bool      m_bUseMRG;                                        ///< SOPH: flag for using motion partition Merge Mode
#endif

#if HHI_IMVP
  Bool      m_bUseIMP;                                        // SOPH : coding tools (interleaved MV Predictor)
#endif

#ifdef DCM_PBIC 
  Bool      m_bUseIC;                                         // Partition-based IC
#endif

  Bool      m_bUseAMP;                                        ///< flag for using asymmetric partition
#if HHI_RMP_SWITCH
  Bool      m_bUseRMP;
#endif

  // coding tools (encoder-only parameters)
  Bool      m_bUseSBACRD;                                     ///< flag for using RD optimization based on SBAC
  Bool      m_bUseASR;                                        ///< flag for using adaptive motion search range
  Bool      m_bUseHADME;                                      ///< flag for using HAD in sub-pel ME
  Bool      m_bUseRDOQ;                                       ///< flag for using RD optimized quantization
  Bool      m_bUseBQP;                                        ///< flag for using B-slice based QP assignment in low-delay hier. structure
  Int       m_iFastSearch;                                    ///< ME mode, 0 = full, 1 = diamond, 2 = PMVFAST
  Int       m_iSearchRange;                                   ///< ME search range
  Bool      m_bUseFastEnc;                                    ///< flag for using fast encoder setting

#ifdef EDGE_BASED_PREDICTION
  // coding tool: edge based prediction
  Bool      m_bEdgePredictionEnable;
  Int       m_iEdgeDetectionThreshold;
#endif //EDGE_BASED_PREDICTION
#if HHI_INTERP_FILTER
  // coding tool (interpolation filter)
  Int       m_iInterpFilterType;                              ///< interpolation filter type
#endif

  // internal member functions
  Void  xSetGlobal      ();                                   ///< set global variables
  Void  xCheckParameter ();                                   ///< check validity of configuration values
  Void  xPrintParameter ();                                   ///< print configuration values
  Void  xPrintUsage     ();                                   ///< print usage

public:
  TAppEncCfg();
  virtual ~TAppEncCfg();

public:
  Void  create    ();                                         ///< create option handling class
  Void  destroy   ();                                         ///< destroy option handling class
  Bool  parseCfg  ( Int argc, Char* argv[] );                 ///< parse configuration file to fill member variables

};// END CLASS DEFINITION TAppEncCfg

#endif // __TAPPENCCFG__

