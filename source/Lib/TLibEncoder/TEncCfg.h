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
#if PIC_CROPPING
  Int       m_croppingMode;
  Int       m_cropLeft;
  Int       m_cropRight;
  Int       m_cropTop;
  Int       m_cropBottom;
#endif
  Int       m_iFrameToBeEncoded;

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  UInt      m_uiDecodingRefreshType;            ///< the type of decoding refresh employed for the random access.
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF)
  
  Int       m_iMaxRefPicNum;                     ///< this is used to mimic the sliding mechanism used by the decoder
                                                 // TODO: We need to have a common sliding mechanism used by both the encoder and decoder

  Bool      m_bDisInter4x4;
  Bool m_useAMP;
  //======= Transform =============
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  Bool      m_useNSQT;

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame

  //====== Tool list ========
  Bool      m_bUseHADME;
  Bool      m_bUseFastEnc;
  Bool      m_bUseEarlyCU;
#if FAST_DECISION_FOR_MRG_RD_COST
  Bool      m_useFastDecisionForMerge;
#endif
  Bool      m_bUseCbfFastMode;
  Bool      m_bUseLMChroma; 

  bool m_pictureDigestEnabled; ///< enable(1)/disable(0) md5 computation and SEI signalling

  Bool      m_bEnableTMVP;
#if MULTIBITS_DATA_HIDING
  Int       m_signHideFlag;
  Int       m_signHidingThreshold;
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
#if PIC_CROPPING
  Void      setCroppingMode                 ( Int   i )      { m_croppingMode = i; }
  Void      setCropLeft                     ( Int   i )      { m_cropLeft = i; }
  Void      setCropRight                    ( Int   i )      { m_cropRight = i; }
  Void      setCropTop                      ( Int   i )      { m_cropTop = i; }
  Void      setCropBottom                   ( Int   i )      { m_cropBottom = i; }
#endif
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }
  
  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setDecodingRefreshType          ( Int   i )      { m_uiDecodingRefreshType = (UInt)i; }
  
  Void      setQP                           ( Int   i )      { m_iQP = i; }
  
  Int       getMaxRefPicNum                 ()                              { return m_iMaxRefPicNum;           }
  Void      setMaxRefPicNum                 ( Int iMaxRefPicNum )           { m_iMaxRefPicNum = iMaxRefPicNum;  }

  Bool      getDisInter4x4                  ()              { return m_bDisInter4x4;        }
  Void      setDisInter4x4                  ( Bool b )      { m_bDisInter4x4  = b;          }
  //======== Transform =============
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
  Void      setQuadtreeTUMaxDepthInter      ( UInt  u )      { m_uiQuadtreeTUMaxDepthInter = u; }
  Void      setQuadtreeTUMaxDepthIntra      ( UInt  u )      { m_uiQuadtreeTUMaxDepthIntra = u; }
  
  Void setUseNSQT( Bool b ) { m_useNSQT = b; }
  Void setUseAMP( Bool b ) { m_useAMP = b; }

  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }

  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  unsigned int getFrameSkip                 ()      { return  m_FrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
#if PIC_CROPPING
  Int       getCroppingMode                 ()      { return  m_croppingMode; }
  Int       getCropLeft                     ()      { return  m_cropLeft; }
  Int       getCropRight                    ()      { return  m_cropRight; }
  Int       getCropTop                      ()      { return  m_cropTop; }
  Int       getCropBottom                   ()      { return  m_cropBottom; }
#endif
  Int       getFrameToBeEncoded             ()      { return  m_iFrameToBeEncoded; }

  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  UInt      getDecodingRefreshType          ()      { return  m_uiDecodingRefreshType; }
  Int       getQP                           ()      { return  m_iQP; }
  
  //======== Transform =============
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
  UInt      getQuadtreeTUMaxDepthInter      ()      const { return m_uiQuadtreeTUMaxDepthInter; }
  UInt      getQuadtreeTUMaxDepthIntra      ()      const { return m_uiQuadtreeTUMaxDepthIntra; }
  

  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }
  
  //==== Tool list ========
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
  Void      setUseEarlyCU                   ( Bool  b )     { m_bUseEarlyCU = b; }
#if FAST_DECISION_FOR_MRG_RD_COST
  Void      setUseFastDecisionForMerge      ( Bool  b )     { m_useFastDecisionForMerge = b; }
#endif
  Void      setUseCbfFastMode            ( Bool  b )     { m_bUseCbfFastMode = b; }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }

  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
  Bool      getUseEarlyCU                   ()      { return m_bUseEarlyCU; }
#if FAST_DECISION_FOR_MRG_RD_COST
  Bool      getUseFastDecisionForMerge      ()      { return m_useFastDecisionForMerge; }
#endif
  Bool      getUseCbfFastMode           ()      { return m_bUseCbfFastMode; }
#if NS_HAD
  Bool      getUseNSQT                      ()      { return m_useNSQT; }
#endif

  Bool getUseLMChroma                       ()      { return m_bUseLMChroma;        }
  Void setUseLMChroma                       ( Bool b ) { m_bUseLMChroma  = b;       }

  void setPictureDigestEnabled(bool b) { m_pictureDigestEnabled = b; }
  bool getPictureDigestEnabled() { return m_pictureDigestEnabled; }

  Void      setEnableTMVP ( Bool b ) { m_bEnableTMVP = b;    }
  Bool      getEnableTMVP ()         { return m_bEnableTMVP; }
#if MULTIBITS_DATA_HIDING
  Void      setSignHideFlag( Int signHideFlag ) { m_signHideFlag = signHideFlag; }
  Void      setTSIG( Int tsig )                 { m_signHidingThreshold = tsig; }
  Int       getSignHideFlag()                    { return m_signHideFlag; }
  Int       getTSIG()                            { return m_signHidingThreshold; }
#endif
};

//! \}

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)
