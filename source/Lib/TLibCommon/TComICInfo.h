/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, NTT DOCOMO, INC. and DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of NTT DOCOMO, INC. nor the name of DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
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

/** \file			TComICInfo.h
    \brief		Ic parameter information handling classes (header)
*/

#ifndef __TCOMICINFO__
#define __TCOMICINFO__

#include <memory.h>
#include "CommonDef.h"
#include "TComIc.h"

#ifdef DCM_PBIC

// ====================================================================================================================
// Type definition
// ====================================================================================================================

/// parameters for AICP
typedef struct _AICPInfo
{
  TComIc m_acIcCand[ AICP_MAX_NUM_CANDS ];	///< array of IC paramter predictor candidates
  Int    iN;								///< number of IC parameter predictor candidates
} AICPInfo;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// class for IC parameter in one CU
class TComCUIcField
{
private:
  TComIc*   m_pcIc;
  TComIc*   m_pcIcd;

  UInt      m_uiNumPartition;
  AICPInfo  m_cAICPInfo;

public:
  TComCUIcField()
  {
    m_pcIc     = NULL;
    m_pcIcd    = NULL;
  
  }
  ~TComCUIcField()
  {
    m_pcIc     = NULL;
    m_pcIcd    = NULL;

  }

	// ------------------------------------------------------------------------------------------------------------------
	// create / destroy
	// ------------------------------------------------------------------------------------------------------------------

  Void    create        ( UInt uiNumPartition );
  Void    destroy       ();

	// ------------------------------------------------------------------------------------------------------------------
	// clear / copy
	// ------------------------------------------------------------------------------------------------------------------

  Void    clearIc       ( Int iPartAddr, UInt uiDepth );
  Void    clearIcd      ( Int iPartAddr, UInt uiDepth );
  Void    clearIcField  ();

  Void    copyFrom		( TComCUIcField* pcCUIcFieldSrc, Int iNumPartSrc, Int iPartAddrDst );
  Void    copyTo		( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst );
  Void    copyTo		( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst, UInt uiOffset, UInt uiNumPart );
  Void    copyIcTo		( TComCUIcField* pcCUIcFieldDst, Int iPartAddrDst );

	// ------------------------------------------------------------------------------------------------------------------
	// get
	// ------------------------------------------------------------------------------------------------------------------

  TComIc& getIc		( Int iIdx )               { return  m_pcIc    [iIdx]; }
  TComIc* getIc		()						   { return  m_pcIc;           }
  TComIc& getIcd	( Int iIdx )               { return  m_pcIcd   [iIdx]; }
  TComIc* getIcd	()                         { return  m_pcIcd;          }

  AICPInfo* getAICPInfo	() { return &m_cAICPInfo; }

	// ------------------------------------------------------------------------------------------------------------------
	// set
	// ------------------------------------------------------------------------------------------------------------------

  Void    setIc			( TComIc  cIc,     Int iIdx ) { m_pcIc    [iIdx] = cIc;     }
  Void    setIcd		( TComIc  cIcd,    Int iIdx ) { m_pcIcd   [iIdx] = cIcd;    }
 
  Void    setIcPtr		( TComIc*  cIcPtr	) { m_pcIc   = cIcPtr;					}
  Void    setIcdPtr		( TComIc*  cIcdPtr  ) { m_pcIcd  = cIcdPtr;					}
 
  Void    setNumPartition	( Int iNumPart) { m_uiNumPartition=iNumPart;	}

  Void    setAllIc		( TComIc& rcIc,    PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );
  Void    setAllIcd		( TComIc& rcIcd,   PartSize eCUMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );
 
  Void    setAllIcField	( TComIc& rcIc, PartSize eMbMode, Int iPartAddr, Int iPartIdx, UInt uiDepth );
};

#endif //#ifdef DCM_PBIC

#endif // __TCOMMOTIONINFO__
