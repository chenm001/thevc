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
#if DCM_DECODING_REFRESH
  Int       m_iDecodingRefreshType;                           ///< random access type
#endif
  Int       m_iGOPSize;                                       ///< GOP size of hierarchical structure
  Int       m_iRateGOPSize;                                   ///< GOP size for QP variance
  Int       m_iNumOfReference;                                ///< total number of reference frames in P-slice
  Int       m_iNumOfReferenceB_L0;                            ///< total number of reference frames for reference list L0 in B-slice
  Int       m_iNumOfReferenceB_L1;                            ///< total number of reference frames for reference list L1 in B-slice
  Bool      m_bHierarchicalCoding;                            ///< flag for specifying hierarchical B structure
  Bool      m_bUseLDC;                                        ///< flag for using low-delay coding mode
  Bool      m_bUseNRF;                                        ///< flag for using non-referenced frame in hierarchical structure
  Bool      m_bUseGPB;                                        ///< flag for using generalized P & B structure
#if DCM_COMB_LIST
  Bool      m_bUseLComb;                                      ///< flag for using combined reference list for uni-prediction in B-slices (JCTVC-D421)
  Bool      m_bLCMod;                                         ///< flag for specifying whether the combined reference list for uni-prediction in B-slices is uploaded explicitly
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
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  // coding tools (bit-depth)
  UInt      m_uiInputBitDepth;                                ///< bit-depth of input file
  UInt      m_uiOutputBitDepth;                               ///< bit-depth of output file
  UInt      m_uiBitIncrement;                                 ///< bit-depth increment
  UInt      m_uiInternalBitDepth;                             ///< Internal bit-depth (BitDepth+BitIncrement)
  
  // coding tools (inter - interpolation filter)
#if !DCTIF_8_6_LUMA
  Int       m_iDIFTap;                                        ///< number of taps in DIF (luma)
#endif
  
  // coding tools (loop filter)
  Bool      m_bUseALF;                                        ///< flag for using adaptive loop filter
  
  Bool      m_bLoopFilterDisable;                             ///< flag for using deblocking filter
  Int       m_iLoopFilterAlphaC0Offset;                       ///< alpha offset for deblocking filter
  Int       m_iLoopFilterBetaOffset;                          ///< beta offset for deblocking filter
  
  // coding tools (entropy coder)
  Int       m_iSymbolMode;                                    ///< entropy coder mode, 0 = VLC, 1 = CABAC
  
#if HHI_MRG
  // coding tools (inter - merge motion partitions)
  Bool      m_bUseMRG;                                        ///< SOPH: flag for using motion partition Merge Mode
#endif
  
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
  Int       m_bipredSearchRange;                              ///< ME search range for bipred refinement
  Bool      m_bUseFastEnc;                                    ///< flag for using fast encoder setting
  
#if !DCTIF_8_6_LUMA
  // coding tool (interpolation filter)
  Int       m_iInterpFilterType;                              ///< interpolation filter type
#endif
  
#ifdef ROUNDING_CONTROL_BIPRED
  Bool m_useRoundingControlBipred;
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

