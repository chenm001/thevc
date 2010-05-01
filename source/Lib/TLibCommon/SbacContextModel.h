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


/** \file			SbacContextModel.h
    \brief		SBAC context model class (header)
*/

#ifndef __SBACCONTEXTMODEL__
#define __SBACCONTEXTMODEL__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDef.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SBAC context model class
class SbacContextModel
{
private:
  UChar m_ucState;																																				///< internal state variable

public:
	SbacContextModel()										{ m_ucState = 0;						 }

  const UChar getState()								{ return (m_ucState & 0x7F); }										///< get current state
  const UChar getMps()									{ return (m_ucState >>   7); }										///< get curret MPS

	Void initEqualProbability()           { m_ucState = 0;             }										///< set probability to 0.5
	Void toggleMps()											{ m_ucState ^= 0x80;         }										///< toggle MPS
  Void setState( UChar ucState )				{ m_ucState = (ucState + (getMps() << 7));		 }	///< set state with given value
  Void setStateMps( UChar ucStateMps )	{ m_ucState = (m_ucState & 0x80) ^ ucStateMps; }	///< set MPS with given value

	Void init( Int iQp, Short asCtxInit[] );																								///< initialize state with initial prob.
};

#endif
