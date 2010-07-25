/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, FRAUNHOFER HHI
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of FRAUNHOFER HHI
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

/** \file     ContextModel.cpp
    \brief    context model class
*/

#include "ContextModel.h"

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
    - initialize context model with respect to QP and initial probability
    .
    \param  iQp         input QP value
    \param  asCtxInit   initial probability table
 */
Void 
ContextModel::init( Int iQp, Short asCtxInit[] )
{
  Int iInitState  = ( ( asCtxInit[ 0 ] * iQp ) >> 4 ) + asCtxInit[ 1 ];
  iInitState      = Min( Max( 1, iInitState ), 126 );
  if( iInitState >= 64 )
  {
    m_ucState     = Min( 62, iInitState - 64 );
    m_ucState    += m_ucState + 1;
  }
  else
  {
    m_ucState     = Min( 62, 63 - iInitState );
    m_ucState    += m_ucState;
  }
}


const UChar 
ContextModel::m_aucNextStateMPS[ 64 ] =
{
  1, 2, 3, 4, 5, 6, 7, 8,   
  9, 10,11,12,13,14,15,16,  
  17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,  
  33,34,35,36,37,38,39,40,  
  41,42,43,44,45,46,47,48,
  49,50,51,52,53,54,55,56,  
  57,58,59,60,61,62,62,63
};

const UChar 
ContextModel::m_aucNextStateLPS[ 64 ] =
{
  0, 0, 1, 2, 2, 4, 4, 5,   
  6, 7, 8, 9, 9, 11,11,12,  
  13,13,15,15,16,16,18,18,
  19,19,21,21,22,22,23,24,  
  24,25,26,26,27,27,28,29,  
  29,30,30,30,31,32,32,33,
  33,33,34,34,35,35,35,36,  
  36,36,37,37,37,38,38,63
};

