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
#include <assert.h>

#if G1002_RPS
struct GOPEntry
{
  Int m_iPOC;
  Int m_iQPOffset;
  Double m_iQPFactor;
  Int m_iTemporalId;
  Bool m_bRefPic;
  Int m_iRefBufSize;
  Char m_iSliceType;
  Int m_iNumRefPics;
  Int m_aiReferencePics[MAX_NUM_REF_PICS];
  Int m_aiUsedByCurrPic[MAX_NUM_REF_PICS];
#if INTER_RPS_PREDICTION
  Bool m_bInterRPSPrediction;
  Int m_iDeltaRIdxMinus1;
  Int m_iDeltaRPS;
  Int m_iNumRefIdc;
  Int m_aiRefIdc[MAX_NUM_REF_PICS+1];
#endif
  GOPEntry()
  : m_iPOC(-1)
  , m_iQPOffset()
  , m_iQPFactor()
  , m_iTemporalId()
  , m_bRefPic()
  , m_iRefBufSize()
  , m_iSliceType()
  , m_iNumRefPics()
#if INTER_RPS_PREDICTION
  , m_bInterRPSPrediction()
  , m_iDeltaRIdxMinus1()
  , m_iDeltaRPS()
  , m_iNumRefIdc()
#endif
  {
    ::memset( m_aiReferencePics, 0, sizeof(m_aiReferencePics) );
    ::memset( m_aiUsedByCurrPic, 0, sizeof(m_aiUsedByCurrPic) );
#if INTER_RPS_PREDICTION
    ::memset( m_aiRefIdc,        0, sizeof(m_aiRefIdc) );
#endif
  }
};

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry);     //input
#endif
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
  Int       m_iFrameToBeEncoded;
#if G678_LAMBDA_ADJUSTMENT  
  Double    m_adLambdaModifier[ MAX_TLAYER ];
#endif

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  UInt      m_uiDecodingRefreshType;            ///< the type of decoding refresh employed for the random access.
#if G1002_RPS
  GOPEntry  m_pcGOPList[MAX_GOP];
  Int       m_iExtraRPSs;
  UInt      m_uiMaxNumberOfReferencePictures;
  Int       m_numReorderFrames;
#else
  Int       m_iNumOfReference;
  Int       m_iNumOfReferenceB_L0;
  Int       m_iNumOfReferenceB_L1;
#endif
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF)
  
  Int       m_aiTLayerQPOffset[MAX_TLAYER];

  Int       m_iMaxRefPicNum;                     ///< this is used to mimic the sliding mechanism used by the decoder
                                                 // TODO: We need to have a common sliding mechanism used by both the encoder and decoder

  Bool      m_bTLayering;                        ///< indicates whether temporal IDs are set based on the hierarchical coding structure
  Bool      m_abTLayerSwitchingFlag[MAX_TLAYER]; ///< temporal layer switching flags corresponding to temporal layer
  Bool      m_bDisInter4x4;
#if AMP
  Bool m_useAMP;
#endif
  //======= Transform =============
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
#if NSQT
  Bool      m_useNSQT;
#endif
  
  //====== B Slice ========
#if !G1002_RPS
  Bool      m_bHierarchicalCoding;              //  hierarchical-B coding
#endif
  
  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_bipredSearchRange;

  //====== Tool list ========
  Bool      m_bUseSBACRD;

  Bool      m_bUseHADME;
#if !G1002_RPS
  Bool      m_bUseGPB;
#endif
  Bool      m_bUseLComb;
  Bool      m_bLCMod;
#if !G1002_RPS
  Bool      m_bUseLDC;
#endif
#if !G1002_RPS
  Bool      m_bUseNRF;
  Bool      m_bUseBQP;
#endif
  Bool      m_bUseFastEnc;
#if EARLY_CU_DETERMINATION
  Bool      m_bUseEarlyCU;
#endif
#if CBF_FAST_MODE
  Bool      m_bUseCbfFastMode;
#endif
  Bool      m_bUseMRG; // SOPH:

  bool m_pictureDigestEnabled; ///< enable(1)/disable(0) md5 computation and SEI signalling

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  Bool      m_bUseNewRefSetting;
#endif
#endif

#if NO_TMVP_MARKING
  Bool      m_bEnableTMVP;
#endif

public:
  TEncCfg()          {}
  virtual ~TEncCfg()
  {
  }
  
  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }
  Void      setFrameSkip                    ( unsigned int i ) { m_FrameSkip = i; }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i; }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i; }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }
  
  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setDecodingRefreshType          ( Int   i )      { m_uiDecodingRefreshType = (UInt)i; }
#if G1002_RPS
  Void      setGopList                      ( GOPEntry*  piGOPList )      {  for ( Int i = 0; i < MAX_GOP; i++ ) m_pcGOPList[i] = piGOPList[i]; }
  Void      setExtraRPSs                    ( Int   i )      { m_iExtraRPSs = i; }
  GOPEntry  getGOPEntry                     ( Int   i )      { return m_pcGOPList[i]; }
  Void      setMaxNumberOfReferencePictures ( UInt u )       { m_uiMaxNumberOfReferencePictures = u;    }
  Void      setNumReorderFrames             ( Int  i )       { m_numReorderFrames = i;    }
#else
  Void      setNumOfReference               ( Int   i )      { m_iNumOfReference = i; }
  Void      setNumOfReferenceB_L0           ( Int   i )      { m_iNumOfReferenceB_L0 = i; }
  Void      setNumOfReferenceB_L1           ( Int   i )      { m_iNumOfReferenceB_L1 = i; }
#endif
  
  Void      setQP                           ( Int   i )      { m_iQP = i; }
  
  Void      setTemporalLayerQPOffset        ( Int*  piTemporalLayerQPOffset )      { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_aiTLayerQPOffset[i] = piTemporalLayerQPOffset[i]; }
  
  Int       getMaxRefPicNum                 ()                              { return m_iMaxRefPicNum;           }
  Void      setMaxRefPicNum                 ( Int iMaxRefPicNum )           { m_iMaxRefPicNum = iMaxRefPicNum;  }

  Bool      getTLayering                    ()                              { return m_bTLayering;              } 
  Void      setTLayering                    ( Bool bTLayering )             { m_bTLayering = bTLayering;        }
  Bool      getTLayerSwitchingFlag          ( UInt uiTLayer )               { assert (uiTLayer < MAX_TLAYER ); return  m_abTLayerSwitchingFlag[uiTLayer];                   }
  Void      setTLayerSwitchingFlag          ( Bool* pbTLayerSwitchingFlag ) { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_abTLayerSwitchingFlag[i] = pbTLayerSwitchingFlag[i]; }

  Bool      getDisInter4x4                  ()              { return m_bDisInter4x4;        }
  Void      setDisInter4x4                  ( Bool b )      { m_bDisInter4x4  = b;          }
  //======== Transform =============
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
  Void      setQuadtreeTUMaxDepthInter      ( UInt  u )      { m_uiQuadtreeTUMaxDepthInter = u; }
  Void      setQuadtreeTUMaxDepthIntra      ( UInt  u )      { m_uiQuadtreeTUMaxDepthIntra = u; }
  
#if NSQT
  Void setUseNSQT( Bool b ) { m_useNSQT = b; }
#endif
#if AMP
  Void setUseAMP( Bool b ) { m_useAMP = b; }
#endif
  
  //====== b; Slice ========
#if !G1002_RPS
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }
#endif
  
  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setBipredSearchRange            ( Int   i )      { m_bipredSearchRange = i; }

  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  unsigned int getFrameSkip                 ()      { return  m_FrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
  Int       getFrameToBeEncoded             ()      { return  m_iFrameToBeEncoded; }
#if G678_LAMBDA_ADJUSTMENT  
  void setLambdaModifier                    ( UInt uiIndex, Double dValue ) { m_adLambdaModifier[ uiIndex ] = dValue; }
  Double getLambdaModifier                  ( UInt uiIndex ) const { return m_adLambdaModifier[ uiIndex ]; }
#endif

  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  UInt      getDecodingRefreshType          ()      { return  m_uiDecodingRefreshType; }
#if !G1002_RPS
  Int       getNumOfReference               ()      { return  m_iNumOfReference; }
  Int       getNumOfReferenceB_L0           ()      { return  m_iNumOfReferenceB_L0; }
  Int       getNumOfReferenceB_L1           ()      { return  m_iNumOfReferenceB_L1; }
  
#else
  UInt      getMaxNumberOfReferencePictures ()      { return m_uiMaxNumberOfReferencePictures; }
  Int       geNumReorderFrames              ()      { return m_numReorderFrames; }
#endif
  Int       getQP                           ()      { return  m_iQP; }
  
  Int       getTemporalLayerQPOffset        ( Int i )      { assert (i < MAX_TLAYER ); return  m_aiTLayerQPOffset[i]; }
  
  //======== Transform =============
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
  UInt      getQuadtreeTUMaxDepthInter      ()      const { return m_uiQuadtreeTUMaxDepthInter; }
  UInt      getQuadtreeTUMaxDepthIntra      ()      const { return m_uiQuadtreeTUMaxDepthIntra; }
  
  //==== b; Slice ========
#if !G1002_RPS
  Bool      getHierarchicalCoding           ()      { return  m_bHierarchicalCoding; }
#endif
  
  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }
  
  //==== Tool list ========
  Void      setUseSBACRD                    ( Bool  b )     { m_bUseSBACRD  = b; }
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
#if !G1002_RPS
  Void      setUseGPB                       ( Bool  b )     { m_bUseGPB     = b; }
#endif
  Void      setUseLComb                     ( Bool  b )     { m_bUseLComb   = b; }
  Void      setLCMod                        ( Bool  b )     { m_bLCMod   = b;    }
#if !G1002_RPS
  Void      setUseLDC                       ( Bool  b )     { m_bUseLDC     = b; }
#endif
#if !G1002_RPS
  Void      setUseNRF                       ( Bool  b )     { m_bUseNRF     = b; }
  Void      setUseBQP                       ( Bool  b )     { m_bUseBQP     = b; }
#endif
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
#if EARLY_CU_DETERMINATION
  Void      setUseEarlyCU                   ( Bool  b )     { m_bUseEarlyCU = b; }
#endif
#if CBF_FAST_MODE
  Void      setUseCbfFastMode            ( Bool  b )     { m_bUseCbfFastMode = b; }
#endif
  Void      setUseMRG                       ( Bool  b )     { m_bUseMRG     = b; } // SOPH:
  Bool      getUseSBACRD                    ()      { return m_bUseSBACRD;  }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }

#if !G1002_RPS
  Bool      getUseGPB                       ()      { return m_bUseGPB;     }
#endif
  Bool      getUseLComb                     ()      { return m_bUseLComb;   }
  Bool      getLCMod                        ()      { return m_bLCMod; }
#if !G1002_RPS
  Bool      getUseLDC                       ()      { return m_bUseLDC;     }
#endif
#if !G1002_RPS
  Bool      getUseNRF                       ()      { return m_bUseNRF;     }
  Bool      getUseBQP                       ()      { return m_bUseBQP;     }
#endif
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
#if EARLY_CU_DETERMINATION
  Bool      getUseEarlyCU                   ()      { return m_bUseEarlyCU; }
#endif
#if CBF_FAST_MODE
  Bool      getUseCbfFastMode           ()      { return m_bUseCbfFastMode; }
#endif
  Bool      getUseMRG                       ()      { return m_bUseMRG;     } // SOPH:
#if NS_HAD
  Bool      getUseNSQT                      ()      { return m_useNSQT; }
#endif

  void setPictureDigestEnabled(bool b) { m_pictureDigestEnabled = b; }
  bool getPictureDigestEnabled() { return m_pictureDigestEnabled; }

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  Void      setUseNewRefSetting    ( Bool b ) { m_bUseNewRefSetting = b;    }
  Bool      getUseNewRefSetting    ()         { return m_bUseNewRefSetting; }
#endif
#endif

#if NO_TMVP_MARKING
  Void      setEnableTMVP ( Bool b ) { m_bEnableTMVP = b;    }
  Bool      getEnableTMVP ()         { return m_bEnableTMVP; }
#endif
};

//! \}

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)
