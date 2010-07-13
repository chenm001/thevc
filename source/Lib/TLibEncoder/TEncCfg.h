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

/** \file     TEncCfg.h
    \brief    encoder configuration class (header)
*/

#ifndef __TENCCFG__
#define __TENCCFG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TLibCommon/CommonDef.h"
#include <assert.h>

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TEncCfg
{
protected:
  //==== File I/O ========
  Int       m_iFrameRate;
  Int       m_iFrameSkip;
  Int       m_iSourceWidth;
  Int       m_iSourceHeight;
  Int       m_iFrameToBeEncoded;

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  Int       m_iGOPSize;
  Int       m_iRateGOPSize;
  Int       m_iNumOfReference;
  Int       m_iNumOfReferenceB_L0;
  Int       m_iNumOfReferenceB_L1;

  Int       m_iQP;                              //  if (AdaptiveQP == OFF)

  Int       m_aiTLayerQPOffset[MAX_TLAYER];
  Int       m_aiPad[2];

  //======= Transform =============
  UInt      m_uiMinTrDepth;
  UInt      m_uiMaxTrDepth;

#if HHI_RQT
  Bool      m_bQuadtreeTUFlag;
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
#endif
  //====== B Slice ========
  Bool      m_bHierarchicalCoding;              //  hierarchical-B coding

  //====== Entropy Coding ========
  Int       m_iSymbolMode;                      //  (CAVLC, CABAC, PIPE)
  UInt      m_uiMCWThreshold;                   //  threshold in bits for multi-codeword coding (CABAC & PIPE)
  UInt      m_uiMaxPIPEDelay;                   //  maximum buffer delay for single-codeword PIPE

  //====== Loop/Deblock Filter ========
  Bool      m_bLoopFilterDisable;
  Int       m_iLoopFilterAlphaC0Offset;
  Int       m_iLoopFilterBetaOffset;

#if HHI_ALF
  Bool      m_bALFSeparateQt;
  Bool      m_bALFSymmetry;
  Int       m_iALFMinLength;
  Int       m_iALFMaxLength;
#endif

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)

  //====== Generated Reference Frame Mode ========
  char*     m_pchGRefMode;


  //====== Tool list ========
  Bool      m_bUseSBACRD;
  Bool      m_bUseALF;
  Bool      m_bUseASR;
  Bool      m_bUseHADME;
  Bool      m_bUseGPB;
  Bool      m_bUseRDOQ;
  Bool      m_bUseLDC;
  Bool      m_bUsePAD;
  Bool      m_bUseQBO;
  Bool      m_bUseNRF;
  Bool      m_bUseBQP;
  Bool      m_bUseFastEnc;
#if HHI_ALLOW_CIP_SWITCH
  Bool      m_bUseCIP; // BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
  Bool      m_bUseROT; // BB:
#endif
#if HHI_AIS
  Bool      m_bUseAIS; // BB:
#endif
#if HHI_MRG
  Bool      m_bUseMRG; // SOPH:
#endif
#if HHI_IMVP
  Bool      m_bUseIMP; // SOPH:
#endif
#ifdef QC_AMVRES
  Bool      m_bUseAMVRes;
#endif
#ifdef QC_SIFO_PRED
  Bool      m_bUseSIFO_Pred;
#endif
  Int       m_iDIFTap;  // Number of interpolation filter taps

  Int*      m_aidQP;
  UInt      m_uiMaxTrSize;
  UInt      m_uiDeltaQpRD;

#if HHI_INTERP_FILTER
  Int       m_iInterpFilterType;
#endif

public:
  TEncCfg()          {}
  virtual ~TEncCfg() {}

  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }
  Void      setFrameSkip                    ( Int   i )      { m_iFrameSkip = i; }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i; }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i; }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }

  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setGOPSize                      ( Int   i )      { m_iGOPSize = i; }
  Void      setRateGOPSize                  ( Int   i )      { m_iRateGOPSize = i; }
  Void      setNumOfReference               ( Int   i )      { m_iNumOfReference = i; }
  Void      setNumOfReferenceB_L0           ( Int   i )      { m_iNumOfReferenceB_L0 = i; }
  Void      setNumOfReferenceB_L1           ( Int   i )      { m_iNumOfReferenceB_L1 = i; }

  Void      setQP                           ( Int   i )      { m_iQP = i; }

  Void      setTemporalLayerQPOffset        ( Int*  piTemporalLayerQPOffset )      { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_aiTLayerQPOffset[i] = piTemporalLayerQPOffset[i]; }
  Void      setPad                          ( Int*  iPad                   )      { for ( Int i = 0; i < 2; i++ ) m_aiPad[i] = iPad[i]; }

  //======== Transform =============
  Void      setMinTrDepth                   ( UInt  u )      { m_uiMinTrDepth = u; }
  Void      setMaxTrDepth                   ( UInt  u )      { m_uiMaxTrDepth = u; }

#if HHI_RQT
  Void      setQuadtreeTUFlag               ( Bool  b )      { m_bQuadtreeTUFlag = b; }
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
#endif
  //====== b; Slice ========
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }

  //====== Entropy Coding ========
  Void      setSymbolMode                   ( Int   i )      { m_iSymbolMode = i; }
  Void      setMCWThreshold                 ( UInt ui )      { m_uiMCWThreshold = ui; }
  Void      setMaxPIPEDelay                 ( UInt ui )      { m_uiMaxPIPEDelay = ui; }
  UInt      m_uiBalancedCPUs; //  number of CPUs for load balancing: 0 or 1 - disabled

  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable       = b; }
  Void      setLoopFilterAlphaC0Offset      ( Int   i )      { m_iLoopFilterAlphaC0Offset = i; }
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_iLoopFilterBetaOffset    = i; }

#if HHI_ALF
  Void      setALFSeparateQt                ( Bool  b )      { m_bALFSeparateQt           = b; }  //MS:
  Void      setALFSymmetry                  ( Bool  b )      { m_bALFSymmetry             = b; }  //MS:
  Void      setALFMinLength                 ( Int   i )      { m_iALFMinLength            = i; }  //MS:
  Void      setALFMaxLength                 ( Int   i )      { m_iALFMaxLength            = i; }  //MS:
#endif

  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i; }

   //====== Generated Reference Frame Mode ========
  Void      setGRefMode       (char*  c)       {m_pchGRefMode=c; }

  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  Int       getFrameSkip                    ()      { return  m_iFrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
  Int       getFrameToBeEncoded             ()      { return  m_iFrameToBeEncoded; }

  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  Int       getGOPSize                      ()      { return  m_iGOPSize; }
  Int       getRateGOPSize                  ()      { return  m_iRateGOPSize; }
  Int       getNumOfReference               ()      { return  m_iNumOfReference; }
  Int       getNumOfReferenceB_L0           ()      { return  m_iNumOfReferenceB_L0; }
  Int       getNumOfReferenceB_L1           ()      { return  m_iNumOfReferenceB_L1; }

  Int       getQP                           ()      { return  m_iQP; }

  Int       getTemporalLayerQPOffset        ( Int i )      { assert (i < MAX_TLAYER ); return  m_aiTLayerQPOffset[i]; }
  Int       getPad                          ( Int i )      { assert (i < 2 );                      return  m_aiPad[i]; }

  //======== Transform =============
  UInt      getMinTrDepth                   ()      { return  m_uiMinTrDepth; }
  UInt      getMaxTrDepth                   ()      { return  m_uiMaxTrDepth; }

#if HHI_RQT
  Bool      getQuadtreeTUFlag               ()      const { return m_bQuadtreeTUFlag; }
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
#endif
  //==== b; Slice ========
  Bool      getHierarchicalCoding           ()      { return  m_bHierarchicalCoding; }

  //==== Entropy Coding ========
  Int       getSymbolMode                   ()      { return  m_iSymbolMode; }
  UInt      getMCWThreshold                 ()      { return  m_uiMCWThreshold; }
  UInt      getMaxPIPEDelay                 ()      { return  m_uiMaxPIPEDelay; }
  Void      setBalancedCPUs                 ( UInt ui )  { m_uiBalancedCPUs = ui; }
  UInt      getBalancedCPUs                 ()           { return m_uiBalancedCPUs; }

  //==== Loop/Deblock Filter ========
  Bool      getLoopFilterDisable            ()      { return  m_bLoopFilterDisable;       }
  Int       getLoopFilterAlphaC0Offget      ()      { return  m_iLoopFilterAlphaC0Offset; }
  Int       getLoopFilterBetaOffget         ()      { return  m_iLoopFilterBetaOffset;    }

#if HHI_ALF
  Bool      getALFSeparateQt                ()      { return  m_bALFSeparateQt;           } //MS:
  Bool      getALFSymmetry                  ()      { return  m_bALFSymmetry;             } //MS:
  Int       getALFMinLength                 ()      { return  m_iALFMinLength;            } //MS:
  Int       getALFMaxLength                 ()      { return  m_iALFMaxLength;            } //MS:
#endif

  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP; }

   //====== Generated Reference Frame Mode ========
  char*      getGRefMode       ()       { return m_pchGRefMode; }

  //==== Tool list ========
  Void      setUseSBACRD                    ( Bool  b )     { m_bUseSBACRD  = b; }
  Void      setUseASR                       ( Bool  b )     { m_bUseASR     = b; }
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseALF                       ( Bool  b )     { m_bUseALF   = b; }
  Void      setUseGPB                       ( Bool  b )     { m_bUseGPB     = b; }
  Void      setUseRDOQ                      ( Bool  b )     { m_bUseRDOQ    = b; }
  Void      setUseLDC                       ( Bool  b )     { m_bUseLDC     = b; }
  Void      setUsePAD                       ( Bool  b )     { m_bUsePAD     = b; }
  Void      setUseQBO                       ( Bool  b )     { m_bUseQBO     = b; }
  Void      setUseNRF                       ( Bool  b )     { m_bUseNRF     = b; }
  Void      setUseBQP                       ( Bool  b )     { m_bUseBQP     = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
#if HHI_ALLOW_CIP_SWITCH
  Void      setUseCIP                       ( Bool  b )     { m_bUseCIP     = b; } // BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
  Void      setUseROT                       ( Bool  b )     { m_bUseROT	    = b; } // BB:
#endif
#if HHI_AIS
  Void      setUseAIS                       ( Bool  b )     { m_bUseAIS     = b; } // BB:
#endif
#if HHI_MRG
  Void      setUseMRG                       ( Bool  b )     { m_bUseMRG     = b; } // SOPH:
#endif
#if HHI_IMVP
  Void      setUseIMP                       ( Bool  b )     { m_bUseIMP     = b; } // SOPH:
#endif
#ifdef QC_AMVRES
 Void      setUseAMVRes                       ( Bool  b )     { m_bUseAMVRes     = b; }
#endif
#ifdef QC_SIFO_PRED
 Void      setUseSIFO_Pred                       ( Bool  b )     { m_bUseSIFO_Pred     = b; }
 Bool      getUseSIFO_Pred                     ()     {return  m_bUseSIFO_Pred; }
#endif

  Void      setDIFTap                       ( Int   i )     { m_iDIFTap     = i; }

  Void      setdQPs                         ( Int*  p )     { m_aidQP       = p; }
  Void      setMaxTrSize                    ( UInt  u )     { m_uiMaxTrSize = u; }
  Void      setDeltaQpRD                    ( UInt  u )     {m_uiDeltaQpRD  = u; }

  Bool      getUseSBACRD                    ()      { return m_bUseSBACRD;  }
  Bool      getUseASR                       ()      { return m_bUseASR;     }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }
  Bool      getUseALF                       ()      { return m_bUseALF;     }
  Bool      getUseGPB                       ()      { return m_bUseGPB;     }
  Bool      getUseRDOQ                      ()      { return m_bUseRDOQ;    }
  Bool      getUseLDC                       ()      { return m_bUseLDC;     }
  Bool      getUsePAD                       ()      { return m_bUsePAD;     }
  Bool      getUseQBO                       ()      { return m_bUseQBO;     }
  Bool      getUseNRF                       ()      { return m_bUseNRF;     }
  Bool      getUseBQP                       ()      { return m_bUseBQP;     }
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
#if HHI_ALLOW_CIP_SWITCH
	Bool      getUseCIP                       ()      { return m_bUseCIP;     }	// BB:
#endif
#if HHI_ALLOW_ROT_SWITCH
	Bool      getUseROT                       ()      { return m_bUseROT;     } // BB:
#endif
#if HHI_AIS
  Bool      getUseAIS                       ()      { return m_bUseAIS;     } // BB:
#endif
#if HHI_MRG
  Bool      getUseMRG                       ()      { return m_bUseMRG;     } // SOPH:
#endif
#if HHI_IMVP
  Bool      getUseIMP                       ()      { return m_bUseIMP;     } // SOPH:
#endif
#ifdef QC_AMVRES
 Bool      getUseAMVRes                     ()     {return  m_bUseAMVRes; }
#endif
  Int       getDIFTap                       ()      { return m_iDIFTap;  }

  Int*      getdQPs                         ()      { return m_aidQP;       }
  UInt      getMaxTrSize                    ()      { return m_uiMaxTrSize; }
  UInt      getDeltaQpRD                    ()      { return m_uiDeltaQpRD; }

#if HHI_INTERP_FILTER
  Void      setInterpFilterType             ( Int   i )     { m_iInterpFilterType = i;    }
  Int       getInterpFilterType             ()              { return m_iInterpFilterType; }
#endif
};

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)

