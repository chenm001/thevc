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

/** \file			TEncCfg.h
    \brief		encoder configuration class (header)
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
	Int				m_iRateGOPSize;
  Int       m_iNumOfReference;
	Int       m_iNumOfReferenceB_L0;
	Int       m_iNumOfReferenceB_L1;

  Int       m_iQP;                              //  if (AdaptiveQP == OFF)

  Int       m_aiTLayerQPOffset[MAX_TLAYER];
  Int       m_aiPad[2];

  //======= Transform =============
  UInt      m_uiMinTrDepth;
  UInt      m_uiMaxTrDepth;

  //====== B Slice ========
  Bool      m_bHierarchicalCoding;              //  ON/OFF  (OFF에서는 reference B를 사용 안 함)

  //====== Entropy Coding ========
  Int       m_iSymbolMode;                      //  (CAVLC, SBAC)

  //====== Loop/Deblock Filter ========
  Bool      m_bLoopFilterDisable;
  Int       m_iLoopFilterAlphaC0Offset;
  Int       m_iLoopFilterBetaOffset;

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)

  //====== Generated Reference Frame Mode ========
  char*     m_pchGeneratedReferenceMode;


  //====== IP list ========
  Bool      m_bUseSBACRD;
  Bool      m_bUseMVAC;
  Bool      m_bUseMVACRD;
  Bool      m_bUseALF;
  Bool      m_bUseADI;
  Bool      m_bUseACC;
  Bool	  	m_bUseASR;
  Bool      m_bUseROT;
  Bool      m_bUseMPI;
  Bool      m_bUseJMQP;
  Bool      m_bUseJMLAMBDA;
  Bool      m_bUseAMVP;
  Bool      m_bUseIMR;
  Bool      m_bUseDIF;
  Bool      m_bUseHADME;
  Bool      m_bUseRNG;
  Bool      m_bUseGPB;
  Bool      m_bUseAMP;
  Bool      m_bUseSHV;
  Bool      m_bUseFAM;
  Bool      m_bUseRDOQ;
  Bool      m_bUseLOT;
  Bool      m_bUseEXC;
  Bool      m_bUseCCP;
  Bool      m_bUseTMI;
  Bool      m_bUseLDC;
  Bool      m_bUseCIP;
	Bool      m_bUsePAD;
  Bool      m_bUseLCT;
  Bool      m_bUseQBO;
  Bool      m_bUseNRF;
  Bool      m_bUseBQP;
  Bool      m_bUseACS;

  Bool      m_bUseHAP;	// Height Accuracy Motion for P
  Bool      m_bUseHAB;	// Height Accuracy Motion for B
  Bool      m_bUseHME;	// Height Accuracy Motion for ME

	Int				m_iDIFTap;	// Number of interpolation filter taps
	Int				m_iDIFTapC;	// Number of interpolation filter taps

  Int*      m_aidQP;
  UInt      m_uiMaxTrSize;
  UInt      m_uiDeltaQpRD;
public:
	TEncCfg()					 {}
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

  //====== b; Slice ========
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }

  //====== Entropy Coding ========
  Void      setSymbolMode                   ( Int   i )      { m_iSymbolMode = i; }

  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable = b; }
  Void      setLoopFilterAlphaC0Offset      ( Int   i )      { m_iLoopFilterAlphaC0Offset = i; }
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_iLoopFilterBetaOffset = i; }

  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i; }

   //====== Generated Reference Frame Mode ========
  Void      setGeneratedReferenceMode       (char*  c)       {m_pchGeneratedReferenceMode=c; }

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

  //==== b; Slice ========
  Bool      getHierarchicalCoding           ()      { return  m_bHierarchicalCoding; }

  //==== Entropy Coding ========
  Int       getSymbolMode                   ()      { return  m_iSymbolMode; }

  //==== Loop/Deblock Filter ========
  Bool      getLoopFilterDisable            ()      { return  m_bLoopFilterDisable; }
  Int       getLoopFilterAlphaC0Offget      ()      { return  m_iLoopFilterAlphaC0Offset; }
  Int       getLoopFilterBetaOffget         ()      { return  m_iLoopFilterBetaOffset; }

  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP; }

   //====== Generated Reference Frame Mode ========
  char*      getGeneratedReferenceMode       ()       { return m_pchGeneratedReferenceMode; }

  //==== IP list ========
  Void      setUseSBACRD                   	( Bool  b )     { m_bUseSBACRD	= b; }
  Void      setUseMVAC                      ( Bool  b )     { m_bUseMVAC		= b; }
  Void      setUseMVACRD                    ( Bool  b )     { m_bUseMVACRD  = b; }
  Void      setUseADI                       ( Bool  b )     { m_bUseADI			= b; }
  Void      setUseACC                       ( Bool  b )     { m_bUseACC			= b; }
  Void	  	setUseASR 					  					( Bool  b )	  	{ m_bUseASR		  = b; }
  Void      setUseROT                       ( Bool  b )     { m_bUseROT			= b; }
  Void      setUseMPI												( Bool  b )     { m_bUseMPI			= b; }
  Void      setUseJMQP											( Bool  b )     { m_bUseJMQP		= b; }
  Void      setUseJMLAMBDA									( Bool  b )     { m_bUseJMLAMBDA= b; }
  Void      setUseAMVP											( Bool  b )     { m_bUseAMVP		= b; }
  Void      setUseIMR												( Bool  b )     { m_bUseIMR		= b; }
  Void      setUseDIF												( Bool  b )     { m_bUseDIF		= b; }
  Void      setUseHADME											( Bool  b )     { m_bUseHADME		= b; }
  Void      setUseALF												( Bool  b )     { m_bUseALF		= b; }
  Void      setUseRNG												( Bool  b )     { m_bUseRNG			= b; }
  Void      setUseGPB												( Bool  b )     { m_bUseGPB			= b; }
  Void      setUseAMP												( Bool  b )     { m_bUseAMP			= b; }
  Void      setUseSHV 											( Bool	b ) 		{ m_bUseSHV 		= b; }
  Void      setUseFAM												( Bool  b )     { m_bUseFAM			= b; }
  Void      setUseRDOQ											( Bool  b )     { m_bUseRDOQ  	= b; }
  Void      setUseLOT 											( Bool  b )     { m_bUseLOT     = b; }
  Void      setUseExC 											( Bool  b )     { m_bUseEXC     = b; }
  Void      setUseCCP 											( Bool  b )     { m_bUseCCP     = b; }
  Void      setUseTMI 											( Bool  b )     { m_bUseTMI     = b; }
  Void      setUseLDC                       ( Bool  b )     { m_bUseLDC     = b; }
	Void      setUseCIP                       ( Bool  b )     { m_bUseCIP     = b; }
	Void      setUsePAD                       ( Bool  b )     { m_bUsePAD     = b; }
  Void      setUseLCT                       ( Bool  b )     { m_bUseLCT     = b; }
  Void      setUseQBO                       ( Bool  b )     { m_bUseQBO     = b; }
  Void      setUseNRF                       ( Bool  b )     { m_bUseNRF     = b; }
  Void      setUseBQP                       ( Bool  b )     { m_bUseBQP     = b; }
  Void      setUseACS 											( Bool  b )     { m_bUseACS     = b; }

  Void      setUseHAP 											( Bool  b )     { m_bUseHAP     = b; }
  Void      setUseHAB 											( Bool  b )     { m_bUseHAB     = b; }
  Void      setUseHME 											( Bool  b )     { m_bUseHME     = b; }

	Void      setDIFTap 											( Int   i )     { m_iDIFTap     = i; }
	Void      setDIFTapC 											( Int   i )     { m_iDIFTapC    = i; }

	Void			setdQPs                         ( Int*  p )     { m_aidQP       = p; }
	Void			setMaxTrSize                    ( UInt  u )			{ m_uiMaxTrSize = u; }
  Void			setDeltaQpRD										( UInt  u )			{m_uiDeltaQpRD  = u; }

  Bool      getUseSBACRD										()			{ return m_bUseSBACRD; 	}
  Bool      getUseMVAC                      ()      { return m_bUseMVAC;    }
  Bool      getUseMVACRD                    ()      { return m_bUseMVACRD;  }
  Bool      getUseADI                       ()      { return m_bUseADI;     }
	Bool      getUseACC                       ()      { return m_bUseACC;     }
  Bool	    getUseASR 					  					()	  	{ return m_bUseASR;	  	}
	Bool      getUseROT                       ()      { return m_bUseROT;     }
	Bool      getUseMPI		                    ()      { return m_bUseMPI;	    }
	Bool      getUseJMQP	                    ()      { return m_bUseJMQP;		}
	Bool      getUseJMLAMBDA                  ()      { return m_bUseJMLAMBDA;}
	Bool      getUseAMVP	                    ()      { return m_bUseAMVP;    }
	Bool      getUseIMR	                    	()      { return m_bUseIMR;    	}
	Bool      getUseDIF	                    	()      { return m_bUseDIF;    	}
	Bool      getUseHADME	                    ()      { return m_bUseHADME;   }
	Bool      getUseALF												()      { return m_bUseALF;			}
	Bool      getUseRNG												()      { return m_bUseRNG;			}
	Bool      getUseGPB												()      { return m_bUseGPB;			}
	Bool      getUseAMP												()      { return m_bUseAMP;			}
  Bool			getUseSHV 											()			{ return m_bUseSHV; 		}
	Bool      getUseFAM												()      { return m_bUseFAM;			}
  Bool      getUseRDOQ											()			{ return m_bUseRDOQ;		}
  Bool      getUseLOT                       ()      { return m_bUseLOT;     }
  Bool      getUseExC                       ()      { return m_bUseEXC;     }
  Bool      getUseCCP                       ()      { return m_bUseCCP;     }
  Bool      getUseTMI                       ()      { return m_bUseTMI;     }
  Bool      getUseLDC                       ()      { return m_bUseLDC;     }
	Bool      getUseCIP                       ()      { return m_bUseCIP;     }
	Bool      getUsePAD                       ()      { return m_bUsePAD;     }
  Bool      getUseLCT                       ()      { return m_bUseLCT;     }
	Bool      getUseQBO                       ()      { return m_bUseQBO;     }
	Bool      getUseNRF                       ()      { return m_bUseNRF;     }
	Bool      getUseBQP                       ()      { return m_bUseBQP;     }

  Bool      getUseACS                       ()      { return m_bUseACS;     }

  Bool      getUseHAP 											()      { return m_bUseHAP; }
  Bool      getUseHAB 											()      { return m_bUseHAB; }
  Bool      getUseHME 											()      { return m_bUseHME; }

	Int       getDIFTap 											()      { return m_iDIFTap;  }
	Int       getDIFTapC 											()      { return m_iDIFTapC; }

	Int*			getdQPs                         ()      { return m_aidQP;       }
	UInt			getMaxTrSize                    ()      { return m_uiMaxTrSize; }
  UInt			getDeltaQpRD                    ()      { return m_uiDeltaQpRD; }
};

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)

