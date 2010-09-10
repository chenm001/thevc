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

/** \file			TComIc.h
    \brief		IC parameter class (header)
*/

#ifndef __TCOMIC__
#define __TCOMIC__

#include <math.h>
#include "CommonDef.h"

#ifdef DCM_PBIC

#ifndef WIN32    // for gcc compiler
  #include <cstdlib>
  using namespace std;
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// basic IC parameter class
class TComIc
{
private:
  Int   m_aiIcScale[2];	 	///< scale component of IC parameter for each list (6-bit fixed point representation, a value of 64 implies a scale factor of 1)
  Int   m_aiIcOffset[2];	///< offset component of IC parameter for each list (if pixel value representation is 8-bit, an offset of 1 will increase the pixel value by 1)
  Int   m_aiIcParam[3];   ///< IC parameters to be coded (scale_sum, scale_diff, offset in that order)
public:

// ------------------------------------------------------------------------------------------------------------------
// constructors
// ------------------------------------------------------------------------------------------------------------------

  TComIc()
  {
    reset();
  }

// ------------------------------------------------------------------------------------------------------------------
// set
// ------------------------------------------------------------------------------------------------------------------

  Void  reset();
  Void  setIcParam( Int iParam0, Int iParam1, Int iParam2 );

// ------------------------------------------------------------------------------------------------------------------
// get
// ------------------------------------------------------------------------------------------------------------------

  Int   getIcScale	(RefPicList eRefList)    { return m_aiIcScale[eRefList];		       	}
  Int   getIcOffset	(RefPicList eRefList)    { return m_aiIcOffset[eRefList];				}
  Int   getAbsIcScale (RefPicList eRefList)  { return abs( m_aiIcScale[eRefList] );		}
  Int   getAbsIcOffset(RefPicList eRefList)  { return abs( m_aiIcOffset[eRefList] );		}
  Void  getIcParam( Int& iParam0, Int& iParam1, Int& iParam2 );

// ------------------------------------------------------------------------------------------------------------------
// operations
// ------------------------------------------------------------------------------------------------------------------
  Void  computeScaleOffset( RefPicList eRefList );
  Void  addIcParamDiff( TComIc cIcd );
  Void  addAbsIcParamDiff( TComIc cIcd );
  Void  subIcParamPred( TComIc cIcPred );
  Void  copyIcParam   ( TComIc cIcSrc );
  Bool  isequalIcParam( TComIc cIcTmp );

  const TComIc& operator = (const TComIc& rcIc)
  {
    m_aiIcScale[0]  = rcIc.m_aiIcScale[0];
    m_aiIcScale[1]  = rcIc.m_aiIcScale[1];
    m_aiIcOffset[0] = rcIc.m_aiIcOffset[0];
    m_aiIcOffset[1] = rcIc.m_aiIcOffset[1];
    m_aiIcParam[0]  = rcIc.m_aiIcParam[0];
    m_aiIcParam[1]  = rcIc.m_aiIcParam[1];
    m_aiIcParam[2]  = rcIc.m_aiIcParam[2];
    return  *this;
  }

};// END CLASS DEFINITION TComIc

#endif //#ifdef DCM_PBIC

#endif // __TCOMIC__
