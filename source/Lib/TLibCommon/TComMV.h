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

/** \file			TComMv.h
    \brief		motion vector class (header)
*/

#ifndef __TCOMMV__
#define __TCOMMV__

#include <math.h>
#include "CommonDef.h"

#ifndef WIN32    // for gcc compiler
  #include <cstdlib>
  using namespace std;
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// basic motion vector class
class TComMv
{
private:
  Int   m_iHor;			///< horizontal component of motion vector
  Int   m_iVer;			///< vertical component of motion vector
  Int   m_iHorD;		///< horizontal component of motion vector difference
  Int   m_iVerD;		///< vertical component of motion vector difference

public:

	// ------------------------------------------------------------------------------------------------------------------
	// constructors
	// ------------------------------------------------------------------------------------------------------------------

  TComMv() :
    m_iHor(0),
    m_iVer(0)
  {
    m_iHorD = 0;
    m_iVerD = 0;
  }

  TComMv( Int iHor, Int iVer, Int iHorD = 0, Int iVerD = 0 ) :
    m_iHor(iHor),
    m_iVer(iVer)
  {
    m_iHorD = iHorD;
    m_iVerD = iVerD;
  }

	// ------------------------------------------------------------------------------------------------------------------
	// set
	// ------------------------------------------------------------------------------------------------------------------

  Void  set				( Int iHor, Int iVer)			{ m_iHor = iHor;  m_iVer = iVer;						}
  Void  setHor		( Int i )									{ m_iHor = i;																}
  Void  setVer		( Int i )									{ m_iVer = i;																}
  Void  setD			( Int iHorD, Int iVerD)		{ m_iHorD = iHorD;  m_iVerD = iVerD;				}
  Void  setHorD		( Int i )									{ m_iHorD = i;															}
  Void  setVerD		( Int i )									{ m_iVerD = i;															}
  Void  setZero		()                        { m_iHor = m_iVer = m_iHorD = m_iVerD = 0;  }
  Void  setZeroD	()                        { m_iHorD = m_iVerD = 0;										}

	// ------------------------------------------------------------------------------------------------------------------
	// get
	// ------------------------------------------------------------------------------------------------------------------

  Int   getHor		()  { return m_iHor;					}
  Int   getVer		()  { return m_iVer;					}
  Int   getAbsHor	()  { return abs( m_iHor );		}
  Int   getAbsVer	()  { return abs( m_iVer );		}
  Int   getHorD		()  { return m_iHorD;					}
  Int   getVerD		()  { return m_iVerD;					}
  Int   getAbsHorD()  { return abs( m_iHorD );	}
  Int   getAbsVerD()  { return abs( m_iVerD );	}

	// ------------------------------------------------------------------------------------------------------------------
	// operations
	// ------------------------------------------------------------------------------------------------------------------

  const TComMv& operator = (const TComMv& rcMv)
  {
    m_iHor = rcMv.m_iHor;
    m_iVer = rcMv.m_iVer;
    m_iHorD = rcMv.m_iHorD;
    m_iVerD = rcMv.m_iVerD;
    return  *this;
  }

  const TComMv& operator += (const TComMv& rcMv)
  {
    m_iHor += rcMv.m_iHor;
    m_iVer += rcMv.m_iVer;
    return  *this;
  }

  const TComMv& operator-= (const TComMv& rcMv)
  {
    m_iHor -= rcMv.m_iHor;
    m_iVer -= rcMv.m_iVer;
    return  *this;
  }

  const TComMv& operator>>= (const Int i)
  {
    m_iHor >>= i;
    m_iVer >>= i;
    return  *this;
  }

  const TComMv& operator<<= (const Int i)
  {
    m_iHor <<= i;
    m_iVer <<= i;
    return  *this;
  }

  const TComMv operator - ( const TComMv& rcMv ) const
  {
    return TComMv( m_iHor - rcMv.m_iHor, m_iVer - rcMv.m_iVer );
  }

  const TComMv operator + ( const TComMv& rcMv )
  {
    return TComMv( m_iHor + rcMv.m_iHor, m_iVer + rcMv.m_iVer );
  }

  Bool operator== ( const TComMv& rcMv )
  {
    return (m_iHor==rcMv.m_iHor && m_iVer==rcMv.m_iVer);
  }

  Bool operator!= ( const TComMv& rcMv )
  {
    return (m_iHor!=rcMv.m_iHor || m_iVer!=rcMv.m_iVer);
  }

  const TComMv scaleMv( Int iScale )
  {
    return TComMv( (iScale * getHor() + 128) >> 8, (iScale * getVer() + 128) >> 8);
  }
};// END CLASS DEFINITION TComMV


#endif // __TCOMMV__
