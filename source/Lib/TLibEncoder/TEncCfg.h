/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TEncCfg.h
    \brief    encoder configuration class (header)
*/

#ifndef __TENCCFG__
#define __TENCCFG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/CommonDef.h"
#if VPS_INTEGRATION
#include "TLibCommon/TComSlice.h"
#endif
#include <assert.h>

struct GOPEntry
{
  Int m_POC;
  Int m_QPOffset;
  Double m_QPFactor;
  Int m_temporalId;
  Bool m_refPic;
  Int m_numRefPicsActive;
  Char m_sliceType;
  Int m_numRefPics;
  Int m_referencePics[MAX_NUM_REF_PICS];
  Int m_usedByCurrPic[MAX_NUM_REF_PICS];
#if AUTO_INTER_RPS
  Int m_interRPSPrediction;
#else
  Bool m_interRPSPrediction;
#endif
  Int m_deltaRIdxMinus1;
  Int m_deltaRPS;
  Int m_numRefIdc;
  Int m_refIdc[MAX_NUM_REF_PICS+1];
  GOPEntry()
  : m_POC(-1)
  , m_QPOffset(0)
  , m_QPFactor(0)
  , m_temporalId(0)
  , m_refPic(false)
  , m_numRefPicsActive(0)
  , m_sliceType('P')
  , m_numRefPics(0)
  , m_interRPSPrediction(false)
  , m_deltaRIdxMinus1(0)
  , m_deltaRPS(0)
  , m_numRefIdc(0)
  {
    ::memset( m_referencePics, 0, sizeof(m_referencePics) );
    ::memset( m_usedByCurrPic, 0, sizeof(m_usedByCurrPic) );
    ::memset( m_refIdc,        0, sizeof(m_refIdc) );
  }
};

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry);     //input
//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TEncCfg
{
protected:
  //==== File I/O ========
  Int       m_iFrameRate;
  Int       m_FrameSkip;
  Int       m_iSourceWidth;
  Int       m_iSourceHeight;
  Int       m_croppingMode;
  Int       m_cropLeft;
  Int       m_cropRight;
  Int       m_cropTop;
  Int       m_cropBottom;
  Int       m_iFrameToBeEncoded;
  Double    m_adLambdaModifier[ MAX_TLAYER ];

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  UInt      m_uiDecodingRefreshType;            ///< the type of decoding refresh employed for the random access.
  Int       m_iGOPSize;
  GOPEntry  m_GOPList[MAX_GOP];
  Int       m_extraRPSs;
  Int       m_maxDecPicBuffering[MAX_TLAYER];
  Int       m_numReorderPics[MAX_TLAYER];
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF)
  
  Int       m_aiTLayerQPOffset[MAX_TLAYER];
  Int       m_aiPad[2];
  

  Int       m_iMaxRefPicNum;                     ///< this is used to mimic the sliding mechanism used by the decoder
                                                 // TODO: We need to have a common sliding mechanism used by both the encoder and decoder

  Int       m_maxTempLayer;                      ///< Max temporal layer
#if !REMOVE_INTER_4X4
  Bool      m_bDisInter4x4;
#endif
  Bool m_useAMP;
  //======= Transform =============
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  Bool      m_useNSQT;
  
  //====== Loop/Deblock Filter ========
  Bool      m_bLoopFilterDisable;
#if DBL_HL_SYNTAX
  Bool      m_loopFilterOffsetInPPS;
#else
  Bool      m_loopFilterOffsetInAPS;
#endif
  Int       m_loopFilterBetaOffsetDiv2;
  Int       m_loopFilterTcOffsetDiv2;
  Bool      m_DeblockingFilterControlPresent;
  Bool      m_bUseSAO;
  Int       m_maxNumOffsetsPerPic;
#if SAO_REMOVE_APS // encoder renaming
  Bool      m_saoLcuBasedOptimization;
#else
  Bool      m_saoInterleavingFlag;
#endif

  //====== Lossless ========
#if LOSSLESS_CODING
  Bool      m_useLossless;
#endif
  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_bipredSearchRange;

  //====== Quality control ========
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)
  Int       m_iMaxCuDQPDepth;                   //  Max. depth for a minimum CuDQP (0:default)

  Int       m_chromaCbQpOffset;                 //  Chroma Cb QP Offset (0:default)
  Int       m_chromaCrQpOffset;                 //  Chroma Cr Qp Offset (0:default)

#if ADAPTIVE_QP_SELECTION
  Bool      m_bUseAdaptQpSelect;
#endif

  Bool      m_bUseAdaptiveQP;
  Int       m_iQPAdaptationRange;
  
  //====== Tool list ========
  Bool      m_bUseSBACRD;
  Bool      m_bUseALF;
#if AHG6_ALF_OPTION2
  Bool      m_alfLowLatencyEncoding;
#else
  Int       m_iALFEncodePassReduction;

  Int       m_iALFMaxNumberFilters;
  Bool      m_bALFParamInSlice;
  Bool      m_bALFPicBasedEncode;
#endif
  Bool      m_bUseASR;
  Bool      m_bUseHADME;
  Bool      m_bUseLComb;
#if !REMOVE_LC
  Bool      m_bLCMod;
#endif
  Bool      m_bUseRDOQ;
  Bool      m_bUseFastEnc;
  Bool      m_bUseEarlyCU;
  Bool      m_useFastDecisionForMerge;
  Bool      m_bUseCbfFastMode;
  Bool      m_useEarlySkipDetection;
  Bool      m_bUseLMChroma; 
#if INTRA_TRANSFORMSKIP
  Bool      m_useTansformSkip;
  Bool      m_useTansformSkipFast;
#endif
  Int*      m_aidQP;
  UInt      m_uiDeltaQpRD;
  
  Bool      m_bUseConstrainedIntraPred;
  Bool      m_usePCM;
  UInt      m_pcmLog2MaxSize;
  UInt      m_uiPCMLog2MinSize;
  //====== Slice ========
  Int       m_iSliceMode;
  Int       m_iSliceArgument; 
  //====== Dependent Slice ========
  Int       m_iDependentSliceMode;
  Int       m_iDependentSliceArgument;
#if DEPENDENT_SLICES
  Bool      m_bCabacIndependentFlag;
#endif
  Int       m_iSliceGranularity;
  Bool      m_bLFCrossSliceBoundaryFlag;

  Bool      m_bPCMInputBitDepthFlag;
  UInt      m_uiPCMBitDepthLuma;
  UInt      m_uiPCMBitDepthChroma;
  Bool      m_bPCMFilterDisableFlag;
#if !TILES_OR_ENTROPY_FIX
  Int       m_iTileBehaviorControlPresentFlag;
#endif
  Bool      m_bLFCrossTileBoundaryFlag;
#if !TILES_OR_ENTROPY_FIX
  Int       m_iColumnRowInfoPresent;
#endif
  Int       m_iUniformSpacingIdr;
  Int       m_iNumColumnsMinus1;
  UInt*     m_puiColumnWidth;
  Int       m_iNumRowsMinus1;
  UInt*     m_puiRowHeight;
#if !EXPLICITLY_SIGNAL_ENTRY_POINTS
  Int       m_iTileLocationInSliceHeaderFlag; //< enable(1)/disable(0) transmitssion of tile location in slice header
#endif

#if !REMOVE_TILE_MARKERS
  Int       m_iTileMarkerFlag;              //< enable(1)/disable(0) transmitssion of light weight tile marker
  Int       m_iMaxTileMarkerEntryPoints;    //< maximum number of tile markers allowed in a slice (controls degree of parallelism)
  Double    m_dMaxTileMarkerOffset;         //< Calculated offset. Light weight tile markers will be transmitted for TileIdx= Offset, 2*Offset, 3*Offset ... 
#endif

  Int       m_iWaveFrontSynchro;
  Int       m_iWaveFrontFlush;
  Int       m_iWaveFrontSubstreams;

#if HASH_TYPE
  Int m_pictureDigestEnabled;              ///< Checksum(3)/CRC(2)/MD5(1)/disable(0) acting on SEI picture_digest message
#else
  bool m_pictureDigestEnabled; ///< enable(1)/disable(0) md5 computation and SEI signalling
#endif
  //====== Weighted Prediction ========
  Bool      m_bUseWeightPred;       //< Use of Weighting Prediction (P_SLICE)
#if REMOVE_IMPLICIT_WP
  Bool      m_useWeightedBiPred;    //< Use of Bi-directional Weighting Prediction (B_SLICE)
#else
  UInt      m_uiBiPredIdc;          //< Use of Bi-Directional Weighting Prediction (B_SLICE)
#endif
  Int       m_useScalingListId;            ///< Using quantization matrix i.e. 0=off, 1=default, 2=file.
  char*     m_scalingListFile;          ///< quantization matrix file name
#if SLICE_TMVP_ENABLE
  Int       m_TMVPModeId;
#else
  Bool      m_bEnableTMVP;
#endif
  Int       m_signHideFlag;
#if !FIXED_SBH_THRESHOLD
  Int       m_signHidingThreshold;
#endif
  Bool      m_enableRateCtrl;                                ///< Flag for using rate control algorithm
  Int       m_targetBitrate;                                 ///< target bitrate
  Int       m_numLCUInUnit;                                  ///< Total number of LCUs in a frame should be divided by the NumLCUInUnit
#if CU_LEVEL_TRANSQUANT_BYPASS
  Bool      m_TransquantBypassEnableFlag;                     ///< transquant_bypass_enable_flag setting in PPS.
  Bool      m_CUTransquantBypassFlagValue;                    ///< if transquant_bypass_enable_flag, the fixed value to use for the per-CU cu_transquant_bypass_flag.
#endif
#if VPS_INTEGRATION
  TComVPS                    m_cVPS;
#endif
  
public:
  TEncCfg()
  : m_puiColumnWidth()
  , m_puiRowHeight()
  {}

  virtual ~TEncCfg()
  {
    delete[] m_puiColumnWidth;
    delete[] m_puiRowHeight;
  }
  
  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }
  Void      setFrameSkip                    ( unsigned int i ) { m_FrameSkip = i; }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i; }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i; }
  Void      setCroppingMode                 ( Int   i )      { m_croppingMode = i; }
  Void      setCropLeft                     ( Int   i )      { m_cropLeft = i; }
  Void      setCropRight                    ( Int   i )      { m_cropRight = i; }
  Void      setCropTop                      ( Int   i )      { m_cropTop = i; }
  Void      setCropBottom                   ( Int   i )      { m_cropBottom = i; }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }
  
  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setDecodingRefreshType          ( Int   i )      { m_uiDecodingRefreshType = (UInt)i; }
  Void      setGOPSize                      ( Int   i )      { m_iGOPSize = i; }
  Void      setGopList                      ( GOPEntry*  GOPList ) {  for ( Int i = 0; i < MAX_GOP; i++ ) m_GOPList[i] = GOPList[i]; }
  Void      setExtraRPSs                    ( Int   i )      { m_extraRPSs = i; }
  GOPEntry  getGOPEntry                     ( Int   i )      { return m_GOPList[i]; }
  Void      setMaxDecPicBuffering           ( UInt u, UInt tlayer ) { m_maxDecPicBuffering[tlayer] = u;    }
  Void      setNumReorderPics               ( Int  i, UInt tlayer ) { m_numReorderPics[tlayer] = i;    }
  
  Void      setQP                           ( Int   i )      { m_iQP = i; }
  
  Void      setTemporalLayerQPOffset        ( Int*  piTemporalLayerQPOffset )      { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_aiTLayerQPOffset[i] = piTemporalLayerQPOffset[i]; }
  Void      setPad                          ( Int*  iPad                   )      { for ( Int i = 0; i < 2; i++ ) m_aiPad[i] = iPad[i]; }
  
  Int       getMaxRefPicNum                 ()                              { return m_iMaxRefPicNum;           }
  Void      setMaxRefPicNum                 ( Int iMaxRefPicNum )           { m_iMaxRefPicNum = iMaxRefPicNum;  }

  Bool      getMaxTempLayer                 ()                              { return m_maxTempLayer;              } 
  Void      setMaxTempLayer                 ( Int maxTempLayer )            { m_maxTempLayer = maxTempLayer;      }
#if !REMOVE_INTER_4X4
  Bool      getDisInter4x4                  ()              { return m_bDisInter4x4;        }
  Void      setDisInter4x4                  ( Bool b )      { m_bDisInter4x4  = b;          }
#endif
  //======== Transform =============
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
  Void      setQuadtreeTUMaxDepthInter      ( UInt  u )      { m_uiQuadtreeTUMaxDepthInter = u; }
  Void      setQuadtreeTUMaxDepthIntra      ( UInt  u )      { m_uiQuadtreeTUMaxDepthIntra = u; }
  
  Void setUseNSQT( Bool b ) { m_useNSQT = b; }
  Void setUseAMP( Bool b ) { m_useAMP = b; }
  
  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable       = b; }
#if DBL_HL_SYNTAX
  Void      setLoopFilterOffsetInPPS        ( Bool  b )      { m_loopFilterOffsetInPPS      = b; }
#else
  Void      setLoopFilterOffsetInAPS        ( Bool  b )      { m_loopFilterOffsetInAPS      = b; }
#endif
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_loopFilterBetaOffsetDiv2  = i; }
  Void      setLoopFilterTcOffset           ( Int   i )      { m_loopFilterTcOffsetDiv2    = i; }
  Void      setDeblockingFilterControlPresent ( Bool b ) { m_DeblockingFilterControlPresent = b; }

  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setBipredSearchRange            ( Int   i )      { m_bipredSearchRange = i; }

  //====== Quality control ========
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i; }
  Void      setMaxCuDQPDepth                ( Int   i )      { m_iMaxCuDQPDepth = i; }

  Void      setChromaCbQpOffset             ( Int   i )      { m_chromaCbQpOffset = i; }
  Void      setChromaCrQpOffset             ( Int   i )      { m_chromaCrQpOffset = i; }

#if ADAPTIVE_QP_SELECTION
  Void      setUseAdaptQpSelect             ( Bool   i ) { m_bUseAdaptQpSelect    = i; }
  Bool      getUseAdaptQpSelect             ()           { return   m_bUseAdaptQpSelect; }
#endif

  Void      setUseAdaptiveQP                ( Bool  b )      { m_bUseAdaptiveQP = b; }
  Void      setQPAdaptationRange            ( Int   i )      { m_iQPAdaptationRange = i; }
  
  //====== Lossless ========
#if LOSSLESS_CODING
  Void      setUseLossless                  (Bool    b  )        { m_useLossless = b;  }
#endif
  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  unsigned int getFrameSkip                 ()      { return  m_FrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
  Int       getCroppingMode                 ()      { return  m_croppingMode; }
  Int       getCropLeft                     ()      { return  m_cropLeft; }
  Int       getCropRight                    ()      { return  m_cropRight; }
  Int       getCropTop                      ()      { return  m_cropTop; }
  Int       getCropBottom                   ()      { return  m_cropBottom; }
  Int       getFrameToBeEncoded             ()      { return  m_iFrameToBeEncoded; }
  void setLambdaModifier                    ( UInt uiIndex, Double dValue ) { m_adLambdaModifier[ uiIndex ] = dValue; }
  Double getLambdaModifier                  ( UInt uiIndex ) const { return m_adLambdaModifier[ uiIndex ]; }

  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  UInt      getDecodingRefreshType          ()      { return  m_uiDecodingRefreshType; }
  Int       getGOPSize                      ()      { return  m_iGOPSize; }
  Int       getMaxDecPicBuffering           (UInt tlayer) { return m_maxDecPicBuffering[tlayer]; }
  Int       getNumReorderPics               (UInt tlayer) { return m_numReorderPics[tlayer]; }
  Int       getQP                           ()      { return  m_iQP; }
  
  Int       getTemporalLayerQPOffset        ( Int i )      { assert (i < MAX_TLAYER ); return  m_aiTLayerQPOffset[i]; }
  Int       getPad                          ( Int i )      { assert (i < 2 );                      return  m_aiPad[i]; }
  
  //======== Transform =============
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
  UInt      getQuadtreeTUMaxDepthInter      ()      const { return m_uiQuadtreeTUMaxDepthInter; }
  UInt      getQuadtreeTUMaxDepthIntra      ()      const { return m_uiQuadtreeTUMaxDepthIntra; }
  
  //==== Loop/Deblock Filter ========
  Bool      getLoopFilterDisable            ()      { return  m_bLoopFilterDisable;       }
#if DBL_HL_SYNTAX
  Bool      getLoopFilterOffsetInPPS        ()      { return m_loopFilterOffsetInPPS; }
#else
  Bool      getLoopFilterOffsetInAPS        ()      { return m_loopFilterOffsetInAPS; }
#endif
  Int       getLoopFilterBetaOffset         ()      { return m_loopFilterBetaOffsetDiv2; }
  Int       getLoopFilterTcOffset           ()      { return m_loopFilterTcOffsetDiv2; }
  Bool      getDeblockingFilterControlPresent()  { return  m_DeblockingFilterControlPresent; }

  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }

  //==== Quality control ========
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP; }
  Int       getMaxCuDQPDepth                ()      { return  m_iMaxCuDQPDepth; }
  Bool      getUseAdaptiveQP                ()      { return  m_bUseAdaptiveQP; }
  Int       getQPAdaptationRange            ()      { return  m_iQPAdaptationRange; }
  //====== Lossless ========
#if LOSSLESS_CODING
  Bool      getUseLossless                  ()      { return  m_useLossless;  }
#endif
  
  //==== Tool list ========
  Void      setUseSBACRD                    ( Bool  b )     { m_bUseSBACRD  = b; }
  Void      setUseASR                       ( Bool  b )     { m_bUseASR     = b; }
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseALF                       ( Bool  b )     { m_bUseALF   = b; }
  Void      setUseLComb                     ( Bool  b )     { m_bUseLComb   = b; }
#if !REMOVE_LC
  Void      setLCMod                        ( Bool  b )     { m_bLCMod   = b;    }
#endif
  Void      setUseRDOQ                      ( Bool  b )     { m_bUseRDOQ    = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
  Void      setUseEarlyCU                   ( Bool  b )     { m_bUseEarlyCU = b; }
  Void      setUseFastDecisionForMerge      ( Bool  b )     { m_useFastDecisionForMerge = b; }
  Void      setUseCbfFastMode            ( Bool  b )     { m_bUseCbfFastMode = b; }
  Void      setUseEarlySkipDetection        ( Bool  b )     { m_useEarlySkipDetection = b; }
  Void      setUseConstrainedIntraPred      ( Bool  b )     { m_bUseConstrainedIntraPred = b; }
  Void      setPCMInputBitDepthFlag         ( Bool  b )     { m_bPCMInputBitDepthFlag = b; }
  Void      setPCMFilterDisableFlag         ( Bool  b )     {  m_bPCMFilterDisableFlag = b; }
  Void      setUsePCM                       ( Bool  b )     {  m_usePCM = b;               }
  Void      setPCMLog2MaxSize               ( UInt u )      { m_pcmLog2MaxSize = u;      }
  Void      setPCMLog2MinSize               ( UInt u )     { m_uiPCMLog2MinSize = u;      }
  Void      setdQPs                         ( Int*  p )     { m_aidQP       = p; }
  Void      setDeltaQpRD                    ( UInt  u )     {m_uiDeltaQpRD  = u; }
  Bool      getUseSBACRD                    ()      { return m_bUseSBACRD;  }
  Bool      getUseASR                       ()      { return m_bUseASR;     }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }
  Bool      getUseALF                       ()      { return m_bUseALF;     }
#if AHG6_ALF_OPTION2
  Void      setALFLowLatencyEncoding        (Bool b) {m_alfLowLatencyEncoding = b;    }
  Bool      getALFLowLatencyEncoding        ()       { return m_alfLowLatencyEncoding;}
#else
  Void      setALFEncodePassReduction       (Int i)  { m_iALFEncodePassReduction = i; }
  Int       getALFEncodePassReduction       ()       { return m_iALFEncodePassReduction; }

  Void      setALFMaxNumberFilters          (Int i)  { m_iALFMaxNumberFilters = i; } 
  Int       getALFMaxNumberFilters          ()       { return m_iALFMaxNumberFilters; } 
  Void      setALFParamInSlice              (Bool b) {m_bALFParamInSlice = b;}
  Bool      getALFParamInSlice              ()       {return m_bALFParamInSlice;}
  Void      setALFPicBasedEncode            (Bool b) {m_bALFPicBasedEncode = b;}
  Bool      getALFPicBasedEncode            ()       {return m_bALFPicBasedEncode;}
#endif
  Bool      getUseLComb                     ()      { return m_bUseLComb;   }
#if !REMOVE_LC
  Bool      getLCMod                        ()      { return m_bLCMod; }
#endif
  Bool      getUseRDOQ                      ()      { return m_bUseRDOQ;    }
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
  Bool      getUseEarlyCU                   ()      { return m_bUseEarlyCU; }
  Bool      getUseFastDecisionForMerge      ()      { return m_useFastDecisionForMerge; }
  Bool      getUseCbfFastMode           ()      { return m_bUseCbfFastMode; }
  Bool      getUseEarlySkipDetection        ()      { return m_useEarlySkipDetection; }
  Bool      getUseConstrainedIntraPred      ()      { return m_bUseConstrainedIntraPred; }
#if NS_HAD
  Bool      getUseNSQT                      ()      { return m_useNSQT; }
#endif
  Bool      getPCMInputBitDepthFlag         ()      { return m_bPCMInputBitDepthFlag;   } 
  Bool      getPCMFilterDisableFlag         ()      { return m_bPCMFilterDisableFlag;   } 
  Bool      getUsePCM                       ()      { return m_usePCM;                 }
  UInt      getPCMLog2MaxSize               ()      { return m_pcmLog2MaxSize;  }
  UInt      getPCMLog2MinSize               ()      { return  m_uiPCMLog2MinSize;  }

  Bool getUseLMChroma                       ()      { return m_bUseLMChroma;        }
  Void setUseLMChroma                       ( Bool b ) { m_bUseLMChroma  = b;       }
#if INTRA_TRANSFORMSKIP
  Bool getUseTransformSkip                             ()      { return m_useTansformSkip;        }
  Void setUseTransformSkip                             ( Bool b ) { m_useTansformSkip  = b;       }
  Bool getUseTransformSkipFast                         ()      { return m_useTansformSkipFast;    }
  Void setUseTransformSkipFast                         ( Bool b ) { m_useTansformSkipFast  = b;   }
#endif
  Int*      getdQPs                         ()      { return m_aidQP;       }
  UInt      getDeltaQpRD                    ()      { return m_uiDeltaQpRD; }

  //====== Slice ========
  Void  setSliceMode                   ( Int  i )       { m_iSliceMode = i;              }
  Void  setSliceArgument               ( Int  i )       { m_iSliceArgument = i;          }
  Int   getSliceMode                   ()              { return m_iSliceMode;           }
  Int   getSliceArgument               ()              { return m_iSliceArgument;       }
  //====== Dependent Slice ========
  Void  setDependentSliceMode            ( Int  i )      { m_iDependentSliceMode = i;       }
  Void  setDependentSliceArgument        ( Int  i )      { m_iDependentSliceArgument = i;   }
  Int   getDependentSliceMode            ()              { return m_iDependentSliceMode;    }
  Int   getDependentSliceArgument        ()              { return m_iDependentSliceArgument;}
#if DEPENDENT_SLICES
  Void  setCabacIndependentFlag            ( Bool  i )      { m_bCabacIndependentFlag = i;       }
  Bool  getCabacIndependentFlag     ()                    { return m_bCabacIndependentFlag;   }
#endif
  Void  setSliceGranularity            ( Int  i )      { m_iSliceGranularity = i;       }
  Int   getSliceGranularity            ()              { return m_iSliceGranularity;    }
  Void      setLFCrossSliceBoundaryFlag     ( Bool   bValue  )    { m_bLFCrossSliceBoundaryFlag = bValue; }
  Bool      getLFCrossSliceBoundaryFlag     ()                    { return m_bLFCrossSliceBoundaryFlag;   }

  Void      setUseSAO                  (Bool bVal)     {m_bUseSAO = bVal;}
  Bool      getUseSAO                  ()              {return m_bUseSAO;}
  Void  setMaxNumOffsetsPerPic                   (Int iVal)            { m_maxNumOffsetsPerPic = iVal; }
  Int   getMaxNumOffsetsPerPic                   ()                    { return m_maxNumOffsetsPerPic; }
#if SAO_REMOVE_APS // encoder renaming
  Void  setSaoLcuBasedOptimization               (bool bVal)           { m_saoLcuBasedOptimization = bVal; }
  Bool  getSaoLcuBasedOptimization               ()                    { return m_saoLcuBasedOptimization; }
#else
  Void  setSaoInterleavingFlag                   (bool bVal)           { m_saoInterleavingFlag = bVal; }
  Bool  getSaoInterleavingFlag                   ()                    { return m_saoInterleavingFlag; }
#endif
#if !TILES_OR_ENTROPY_FIX
  Void  setTileBehaviorControlPresentFlag        ( Int i )             { m_iTileBehaviorControlPresentFlag = i;    }
  Int   getTileBehaviorControlPresentFlag        ()                    { return m_iTileBehaviorControlPresentFlag; }
#endif
  Void  setLFCrossTileBoundaryFlag               ( Bool   bValue  )    { m_bLFCrossTileBoundaryFlag = bValue; }
  Bool  getLFCrossTileBoundaryFlag               ()                    { return m_bLFCrossTileBoundaryFlag;   }
#if !TILES_OR_ENTROPY_FIX
  Void  setColumnRowInfoPresent        ( Int i )           { m_iColumnRowInfoPresent = i; }
  Int   getColumnRowInfoPresent        ()                  { return m_iColumnRowInfoPresent; }
#endif
  Void  setUniformSpacingIdr           ( Int i )           { m_iUniformSpacingIdr = i; }
  Int   getUniformSpacingIdr           ()                  { return m_iUniformSpacingIdr; }
  Void  setNumColumnsMinus1            ( Int i )           { m_iNumColumnsMinus1 = i; }
  Int   getNumColumnsMinus1            ()                  { return m_iNumColumnsMinus1; }
  Void  setColumnWidth ( char* str )
  {
    char *columnWidth;
    int  i=0;
    Int  m_iWidthInCU = ( m_iSourceWidth%g_uiMaxCUWidth ) ? m_iSourceWidth/g_uiMaxCUWidth + 1 : m_iSourceWidth/g_uiMaxCUWidth;

    if( m_iUniformSpacingIdr == 0 && m_iNumColumnsMinus1 > 0 )
    {
      m_puiColumnWidth = new UInt[m_iNumColumnsMinus1];

      columnWidth = strtok(str, " ,-");
      while(columnWidth!=NULL)
      {
        if( i>=m_iNumColumnsMinus1 )
        {
          printf( "The number of columns whose width are defined is larger than the allowed number of columns.\n" );
          exit( EXIT_FAILURE );
        }
        *( m_puiColumnWidth + i ) = atoi( columnWidth );
        printf("col: m_iWidthInCU= %4d i=%4d width= %4d\n",m_iWidthInCU,i,m_puiColumnWidth[i]); //AFU
        columnWidth = strtok(NULL, " ,-");
        i++;
      }
      if( i<m_iNumColumnsMinus1 )
      {
        printf( "The width of some columns is not defined.\n" );
        exit( EXIT_FAILURE );
      }
    }
  }
  UInt  getColumnWidth                 ( UInt columnidx )  { return *( m_puiColumnWidth + columnidx ); }
  Void  setNumRowsMinus1               ( Int i )           { m_iNumRowsMinus1 = i; }
  Int   getNumRowsMinus1               ()                  { return m_iNumRowsMinus1; }
  Void  setRowHeight (char* str)
  {
    char *rowHeight;
    int  i=0;
    Int  m_iHeightInCU = ( m_iSourceHeight%g_uiMaxCUHeight ) ? m_iSourceHeight/g_uiMaxCUHeight + 1 : m_iSourceHeight/g_uiMaxCUHeight;

    if( m_iUniformSpacingIdr == 0 && m_iNumRowsMinus1 > 0 )
    {
      m_puiRowHeight = new UInt[m_iNumRowsMinus1];

      rowHeight = strtok(str, " ,-");
      while(rowHeight!=NULL)
      {
        if( i>=m_iNumRowsMinus1 )
        {
          printf( "The number of rows whose height are defined is larger than the allowed number of rows.\n" );
          exit( EXIT_FAILURE );
        }
        *( m_puiRowHeight + i ) = atoi( rowHeight );
        printf("row: m_iHeightInCU=%4d i=%4d height=%4d\n",m_iHeightInCU,i,m_puiRowHeight[i]); //AFU
        rowHeight = strtok(NULL, " ,-");
        i++;
      }
      if( i<m_iNumRowsMinus1 )
      {
        printf( "The height of some rows is not defined.\n" );
        exit( EXIT_FAILURE );
     }
    }
  }
  UInt  getRowHeight                   ( UInt rowIdx )     { return *( m_puiRowHeight + rowIdx ); }
  Void  xCheckGSParameters();
#if !EXPLICITLY_SIGNAL_ENTRY_POINTS
  Int  getTileLocationInSliceHeaderFlag ()                 { return m_iTileLocationInSliceHeaderFlag; }
  Void setTileLocationInSliceHeaderFlag ( Int iFlag )      { m_iTileLocationInSliceHeaderFlag = iFlag;}
#endif
#if !REMOVE_TILE_MARKERS
  Int  getTileMarkerFlag              ()                 { return m_iTileMarkerFlag;              }
  Void setTileMarkerFlag              ( Int iFlag )      { m_iTileMarkerFlag = iFlag;             }
  Int  getMaxTileMarkerEntryPoints    ()                 { return m_iMaxTileMarkerEntryPoints;    }
  Void setMaxTileMarkerEntryPoints    ( Int iCount )     { m_iMaxTileMarkerEntryPoints = iCount;  }
  Double getMaxTileMarkerOffset       ()                 { return m_dMaxTileMarkerOffset;         }
  Void setMaxTileMarkerOffset         ( Double dCount )  { m_dMaxTileMarkerOffset = dCount;       }
#endif
  Void  setWaveFrontSynchro(Int iWaveFrontSynchro)       { m_iWaveFrontSynchro = iWaveFrontSynchro; }
  Int   getWaveFrontsynchro()                            { return m_iWaveFrontSynchro; }
  Void  setWaveFrontFlush(Int iWaveFrontFlush)           { m_iWaveFrontFlush = iWaveFrontFlush; }
  Int   getWaveFrontFlush()                              { return m_iWaveFrontFlush; }
  Void  setWaveFrontSubstreams(Int iWaveFrontSubstreams) { m_iWaveFrontSubstreams = iWaveFrontSubstreams; }
  Int   getWaveFrontSubstreams()                         { return m_iWaveFrontSubstreams; }
#if HASH_TYPE
  void setPictureDigestEnabled(Int b) { m_pictureDigestEnabled = b; }
  Int getPictureDigestEnabled() { return m_pictureDigestEnabled; }
#else
  void setPictureDigestEnabled(bool b) { m_pictureDigestEnabled = b; }
  bool getPictureDigestEnabled() { return m_pictureDigestEnabled; }
#endif

  Void      setUseWP               ( Bool  b )   { m_bUseWeightPred    = b;    }
#if REMOVE_IMPLICIT_WP
  Void      setWPBiPred            ( Bool b )    { m_useWeightedBiPred = b;    }
#else
  Void      setWPBiPredIdc         ( UInt u )    { m_uiBiPredIdc       = u;    }
#endif
  Bool      getUseWP               ()            { return m_bUseWeightPred;    }
#if REMOVE_IMPLICIT_WP
  Bool      getWPBiPred            ()            { return m_useWeightedBiPred; }
#else
  UInt      getWPBiPredIdc         ()            { return m_uiBiPredIdc;       }
#endif
  Void      setUseScalingListId    ( Int  u )    { m_useScalingListId       = u;   }
  Int       getUseScalingListId    ()            { return m_useScalingListId;      }
  Void      setScalingListFile     ( char*  pch ){ m_scalingListFile     = pch; }
  char*     getScalingListFile     ()            { return m_scalingListFile;    }
#if SLICE_TMVP_ENABLE
  Void      setTMVPModeId ( Int  u ) { m_TMVPModeId = u;    }
  Int       getTMVPModeId ()         { return m_TMVPModeId; }
#else
  Void      setEnableTMVP ( Bool b ) { m_bEnableTMVP = b;    }
  Bool      getEnableTMVP ()         { return m_bEnableTMVP; }
#endif
  Void      setSignHideFlag( Int signHideFlag ) { m_signHideFlag = signHideFlag; }
#if !FIXED_SBH_THRESHOLD
  Void      setTSIG( Int tsig )                 { m_signHidingThreshold = tsig; }
#endif
  Int       getSignHideFlag()                    { return m_signHideFlag; }
#if !FIXED_SBH_THRESHOLD
  Int       getTSIG()                            { return m_signHidingThreshold; }
#endif
  Bool      getUseRateCtrl    ()                { return m_enableRateCtrl;    }
  Void      setUseRateCtrl    (Bool flag)       { m_enableRateCtrl = flag;    }
  Int       getTargetBitrate  ()                { return m_targetBitrate;     }
  Void      setTargetBitrate  (Int target)      { m_targetBitrate  = target;  }
  Int       getNumLCUInUnit   ()                { return m_numLCUInUnit;      }
  Void      setNumLCUInUnit   (Int numLCUs)     { m_numLCUInUnit   = numLCUs; }
#if CU_LEVEL_TRANSQUANT_BYPASS
  Bool      getTransquantBypassEnableFlag()           { return m_TransquantBypassEnableFlag; }
  Void      setTransquantBypassEnableFlag(Bool flag)  { m_TransquantBypassEnableFlag = flag; }
  Bool      getCUTransquantBypassFlagValue()          { return m_CUTransquantBypassFlagValue; }
  Void      setCUTransquantBypassFlagValue(Bool flag) { m_CUTransquantBypassFlagValue = flag; }
#endif
#if VPS_INTEGRATION
  Void setVPS(TComVPS *p) { m_cVPS = *p; }
  TComVPS *getVPS() { return &m_cVPS; }
#endif
};

//! \}

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)
