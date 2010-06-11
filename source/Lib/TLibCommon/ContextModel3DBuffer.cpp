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

/** \file     ContextModel3DBuffer.cpp
    \brief    context model 3D buffer class
*/

#include "ContextModel3DBuffer.h"

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

ContextModel3DBuffer::ContextModel3DBuffer( UInt uiSizeZ, UInt uiSizeY, UInt uiSizeX ) :
  m_pcContextModel( NULL ),
  m_uiSizeX( uiSizeX ),
  m_uiSizeY( uiSizeY ),
  m_uiSizeZ( uiSizeZ )

{
  // allocate 3D buffer
  m_pcContextModel = new ContextModel[ uiSizeZ * m_uiSizeY * m_uiSizeX ];
}

ContextModel3DBuffer::~ContextModel3DBuffer()
{
  // delete 3D buffer
  delete [] m_pcContextModel;
  m_pcContextModel = NULL;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
    - initialize 3D buffer with respect to slicetype, QP and given initial probability table
    .
    \param  eSliceType      slice type
    \param  iQP             input QP value
    \param  psCtxModel      given probability table
 */
Void ContextModel3DBuffer::initBuffer( SliceType eSliceType, Int iQp, Short* psCtxModel )
{
  UInt n, z, offset = 0;

  for ( z = 0; z < m_uiSizeZ; z++ )
  {
    for ( n = 0; n < m_uiSizeY * m_uiSizeX; n++ )
    {
      m_pcContextModel[ offset + n ].init( iQp, psCtxModel + eSliceType * 2 * ( m_uiSizeZ * m_uiSizeY * m_uiSizeX ) + 2 * (n + offset) );
    }
    offset += n;
  }
  return;
}

/**
    - copy from given 3D buffer
    .
    \param  pSrc          given 3D buffer
 */
Void ContextModel3DBuffer::copyFrom( ContextModel3DBuffer* pSrc )
{
  ::memcpy( this->m_pcContextModel, pSrc->m_pcContextModel, sizeof(ContextModel) * m_uiSizeZ * m_uiSizeY * m_uiSizeX );
}

