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

/** \file			TAppEncCfg.h
    \brief		Handle encoder configuration parameters (header)
*/

#ifndef __TAPPENCCFG__
#define __TAPPENCCFG__

#include "../../Lib/TLibCommon/CommonDef.h"
#include "TAppEncOption.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TAppEncCfg
{
protected:
	// storage for encoder option handling
  TAppOption*  m_apcOpt;																			///< option handling class

	// file I/O
  char*     m_pchInputFile;																		///< source file name
  char*     m_pchBitstreamFile;																///< output bitstream file
  char*     m_pchReconFile;																		///< output reconstruction file

	// source specification
  Int       m_iFrameRate;																			///< source frame-rates (Hz)
  Int       m_iFrameSkip;																			///< number of skipped frames from the beginning
  Int       m_iSourceWidth;																		///< source width in pixel
  Int       m_iSourceHeight;																	///< source height in pixel
  Int       m_iFrameToBeEncoded;															///< number of encoded frames
	Bool			m_bUsePAD;																				///< flag for using source padding
	Int       m_aiPad[2];																				///< number of padded pixels for width and height

  // coding structure
  Int       m_iIntraPeriod;																		///< period of I-slice (random access period)
  Int       m_iGOPSize;																				///< GOP size of hierarchical structure
	Int				m_iRateGOPSize;																		///< GOP size for QP variance
  Int       m_iNumOfReference;																///< total number of reference frames in P-slice
	Int				m_iNumOfReferenceB_L0;														///< total number of reference frames for reference list L0 in B-slice
	Int				m_iNumOfReferenceB_L1;														///< total number of reference frames for reference list L1 in B-slice
	Bool      m_bHierarchicalCoding;														///< flag for specifying hierarchical B structure
  Bool      m_bUseLDC;																				///< flag for using low-delay coding mode
	Int				m_iLDMode;																				///< pre-defined setting of low-delay mode, 0 = slow seq (QBO & NRF) / 1 = fast seq (BQP)
	Bool			m_bUseNRF;																				///< flag for using non-referenced frame in hierarchical structure
  Bool			m_bUseGPB;																				///< flag for using generalized P & B structure
	Bool			m_bUseQBO;																				///< flag for using quality-based reference ordering for skip mode

	// coding quality
	Double    m_fQP;																						///< QP value of key-picture (floating point)
  Int       m_iQP;																						///< QP value of key-picture (integer)
  Int       m_aiTLayerQPOffset[MAX_TLAYER];										///< QP offset corresponding to temporal layer depth
  char*     m_pchdQPFile;																			///< QP offset for each slice (initialized from external file)
	Int*			m_aidQP;																					///< array of slice QP values
  Int       m_iMaxDeltaQP;																		///< max. |delta QP|
	UInt			m_uiDeltaQpRD;																		///< dQP range for multi-pass slice QP optimization

	// coding unit (CU) definition
	UInt      m_uiMaxCUWidth;																		///< max. CU width in pixel
  UInt      m_uiMaxCUHeight;																	///< max. CU height in pixel
  UInt      m_uiMaxCUDepth;																		///< max. CU depth

  // transfom unit (TU) definition
  UInt      m_uiMinTrDepth;																		///< min. TU depth
  UInt      m_uiMaxTrDepth;																		///< max. TU depth
  UInt			m_uiMaxTrSize;																		///< max. physical transform size

	// profile definition
  Int       m_iProfileIdx;																		///< profile index for pre-defined setting, 0 = low-complexity, 1 = normal

	// coding tools (bit-depth)
	UInt      m_uiBitDepth;																			///< base bit-depth
	UInt      m_uiBitIncrement;																	///< bit-depth increment

	// coding tools (inter - motion)
	Bool      m_bUseMVAC;																				///< flag for half-pel only motion accuracy
	Bool			m_bUseAMVP;																				///< flag for using advanced motion vector prediction
	Bool			m_bUseIMR;																				///< flag for using implicit MV candidate removal
  Bool			m_bUseAMP;																				///< flag for using asymmetric motion partition
  Bool			m_bUseSHV;																				///< flag for using simultaneous H & V partition
	char*     m_pchGeneratedReferenceMode;											///< array of generated reference modes

	// coding tools (inter - interpolation filter)
	Bool			m_bUseDIF;																				///< flag for using DCT-based interpolation filter
	Int				m_iDIFTap;																				///< number of taps in DIF (luma)
	Int				m_iDIFTapC;																				///< number of taps in DIF (chroma)
  Bool      m_bUseHAP;																				///< flag for using high accuracy motion in P-slice
  Bool      m_bUseHAB;																				///< flag for using high accuracy motion in B-slice
  Bool      m_bUseHME;																				///< flag for using high accuracy motion in ME stage

	// coding tools (intra)
	Bool      m_bUseADI;																				///< flag for using arbitrary directional intra
  Bool			m_bUseTMI;																				///< flag for using template matching intra
	Bool			m_bUseCIP;																				///< flag for using combined intra prediction
	Bool			m_bUseMPI;																				///< flag for using multi-parameter intra
  Bool			m_bUseCCP;																				///< flag for using color correlation based prediction

	// coding tools (transform)
	Bool			m_bUseROT;																				///< flag for using rotational transform
  Bool      m_bUseLOT;																				///< flag for using logical transform
  Bool      m_bUseLCT;																				///< flag for using low-complexity transform

  // coding tools (loop filter)
	Bool      m_bUseALF;																				///< flag for using adaptive loop filter
  Bool      m_bLoopFilterDisable;															///< flag for using deblocking filter
  Int       m_iLoopFilterAlphaC0Offset;												///< alpha offset for deblocking filter
  Int       m_iLoopFilterBetaOffset;													///< beta offset for deblocking filter
  Bool      m_bUseEXC;																				///< flag for using extreme correction
	Bool			m_bUseCADR;																				///< flag for using context-adaptive dynamic range
	Int				m_iMinCADR;																				///< min. value for CADR
	Int				m_iMaxCADR;																				///< max. value for CADR
	Bool			m_bUseRNG;																				///< flag for using random noise generation for perceptual optimization

	// coding tools (entropy coder)
  Int       m_iSymbolMode;																		///< entropy coder mode, 0 = VLC, 1 = SBAC
  Bool			m_bUseACS;																				///< flag for using adaptive coefficient scanning

	// coding tools (encoder-only parameters)
	Bool      m_bUseMVACRD;																			///< flag for using RD optimization for MVAC
	Bool      m_bUseSBACRD;																			///< flag for using RD optimization based on SBAC
	Bool			m_bUseACC;																				///< flag for using additional coefficient clear
	Bool		  m_bUseASR;																				///< flag for using adaptive motion search range
	Bool			m_bUseJMQP;																				///< flag for using JM-style QP assignment
	Bool			m_bUseJMLAMBDA;																		///< flag for using JM-stype lambda computation
	Bool			m_bUseHADME;																			///< flag for using HAD in sub-pel ME
  Bool			m_bUseFAM;																				///< flag for using fast decision of AMP
  Bool      m_bUseRDOQ;																				///< flag for using RD optimized quantization
	Bool			m_bUseBQP;																				///< flag for using B-slice based QP assignment in low-delay hier. structure
  Int       m_iFastSearch;																		///< ME mode, 0 = full, 1 = diamond, 2 = PMVFAST
  Int       m_iSearchRange;																		///< ME search range

	// internal member functions
	Void  xSetCfgFile     ( TAppOption* pcOpt );								///< parse configuration file
  Void  xSetCfgCommand  ( int iArgc, char** pArgv );					///< parse command line options
  Void  xSetProfile     ();																		///< set configuration values according to profile index
	Void	xSetGlobal      ();																		///< set global variables
  Void  xCheckParameter ();																		///< check validity of configuration values
	Void  xPrintParameter ();																		///< print configuration values
	Void	xPrintUsage     ();																		///< print usage
  Void  xConfirmPara    ( Bool bflag, const char* message );	///< misc. function for printing configuration validity

public:
  TAppEncCfg();
  virtual ~TAppEncCfg();

public:
  Void  create    ();																					///< create option handling class
  Void  destroy   ();																					///< destroy option handling class
  Bool  parseCfg  ( Int argc, Char* argv[] );									///< parse configuration file to fill member variables

};// END CLASS DEFINITION TAppEncCfg

#endif // __TAPPENCCFG__
