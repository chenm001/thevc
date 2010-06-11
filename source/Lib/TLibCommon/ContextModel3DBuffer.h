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

/** \file     ContextModel3DBuffer.h
    \brief    context model 3D buffer class (header)
*/

#ifndef __CONTEXT_MODEL_3DBUFFER__
#define __CONTEXT_MODEL_3DBUFFER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <assert.h>
#include <memory.h>

#include "CommonDef.h"
#include "ContextModel.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// context model 3D buffer class
class ContextModel3DBuffer
{
protected:
  ContextModel* m_pcContextModel;                                         ///< array of context models
  const UInt    m_uiSizeX;                                                ///< X size of 3D buffer
  const UInt    m_uiSizeY;                                                ///< Y size of 3D buffer
  const UInt    m_uiSizeZ;                                                ///< Z size of 3D buffer

public:
  ContextModel3DBuffer  ( UInt uiSizeZ, UInt uiSizeY, UInt uiSizeX );
  ~ContextModel3DBuffer ();

  // access functions
  ContextModel& get( UInt uiZ, UInt uiY, UInt uiX )
  {
    return  m_pcContextModel[ ( uiZ * m_uiSizeY + uiY ) * m_uiSizeX + uiX ];
  }
  ContextModel* get( UInt uiZ, UInt uiY )
  {
    return &m_pcContextModel[ ( uiZ * m_uiSizeY + uiY ) * m_uiSizeX       ];
  }
  ContextModel* get( UInt uiZ )
  {
    return &m_pcContextModel[ ( uiZ * m_uiSizeY       ) * m_uiSizeX       ];
  }

  // initialization & copy functions
  Void initBuffer( SliceType eSliceType, Int iQp, Short* psCtxModel );          ///< initialize 3D buffer by slice type & QP
  Void copyFrom  ( ContextModel3DBuffer* pSrc                       );          ///< copy from given 3D buffer
};

#endif

