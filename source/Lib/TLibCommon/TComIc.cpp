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

/** \file			TComIc.cpp
    \brief		Methods for TComIc class
*/

#include <assert.h>
#include <stdlib.h>

#include "CommonDef.h"
#include "TComIc.h"

#ifdef DCM_PBIC
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComIc::reset( )
{
  setIcParam( 0, 0, 0 );
  computeScaleOffset( REF_PIC_LIST_X );
}

Void TComIc::setIcParam( Int iParam0, Int iParam1, Int iParam2 )
{
  m_aiIcParam[0] = iParam0;
  m_aiIcParam[1] = iParam1;
  m_aiIcParam[2] = iParam2;
}

Void TComIc::getIcParam( Int& iParam0, Int& iParam1, Int& iParam2 )
{
  iParam0 = m_aiIcParam[0];
  iParam1 = m_aiIcParam[1];
  iParam2 = m_aiIcParam[2];
}

Void TComIc::computeScaleOffset( RefPicList eRefList )
{
  if ( eRefList == REF_PIC_LIST_X )
  {
    m_aiIcScale[0] = (1 << IC_SCALE_PREC) + m_aiIcParam[0] + (m_aiIcParam[1] << 1);
    m_aiIcScale[1] = (1 << IC_SCALE_PREC) + m_aiIcParam[0] - (m_aiIcParam[1] << 1);
    m_aiIcOffset[0] = m_aiIcOffset[1] = m_aiIcParam[2];

    if ( m_aiIcScale[0] < 0 || m_aiIcScale[1] < 0 )
    {
      m_aiIcScale [0] = m_aiIcScale [1] = (1 << IC_SCALE_PREC);
      m_aiIcOffset[0] = m_aiIcOffset[1] = 0;
    }
  }
  else
  {
    m_aiIcScale [eRefList]   = (1 << IC_SCALE_PREC) + m_aiIcParam[0];
    m_aiIcOffset[eRefList]   = m_aiIcParam[2];
    m_aiIcScale [eRefList^1] = 0;
    m_aiIcOffset[eRefList^1] = 0;

    assert( m_aiIcParam[1] == 0 );
    if ( m_aiIcScale[eRefList] < 0 )
    {
      m_aiIcScale [eRefList]   = (1 << IC_SCALE_PREC);
      m_aiIcOffset[eRefList]   = 0;
    }
  }
}

Void TComIc::addIcParamDiff( TComIc cIcd )
{
  m_aiIcParam[0] += cIcd.m_aiIcParam[0];
  m_aiIcParam[1] += cIcd.m_aiIcParam[1];
  m_aiIcParam[2] += cIcd.m_aiIcParam[2];
}

Void TComIc::addAbsIcParamDiff( TComIc cIcd )
{
  m_aiIcParam[0] += abs( cIcd.m_aiIcParam[0] );
  m_aiIcParam[1] += abs( cIcd.m_aiIcParam[1] );
  m_aiIcParam[2] += abs( cIcd.m_aiIcParam[2] );
}

Void TComIc::subIcParamPred( TComIc cIcPred )
{
  m_aiIcParam[0] -= cIcPred.m_aiIcParam[0];
  m_aiIcParam[1] -= cIcPred.m_aiIcParam[1];
  m_aiIcParam[2] -= cIcPred.m_aiIcParam[2];
}

Void TComIc::copyIcParam( TComIc cIcSrc )
{
  m_aiIcParam[0] = cIcSrc.m_aiIcParam[0];
  m_aiIcParam[1] = cIcSrc.m_aiIcParam[1];
  m_aiIcParam[2] = cIcSrc.m_aiIcParam[2];
}

Bool TComIc::isequalIcParam( TComIc cIcTmp )
{
  return ( (m_aiIcParam[0] == cIcTmp.m_aiIcParam[0]) && (m_aiIcParam[1] == cIcTmp.m_aiIcParam[1]) && (m_aiIcParam[2] == cIcTmp.m_aiIcParam[2]) );
}

#endif //#ifdef DCM_PBIC
