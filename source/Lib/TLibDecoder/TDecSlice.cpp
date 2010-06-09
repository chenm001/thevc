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

/** \file     TDecSlice.cpp
    \brief    slice decoder class
*/

#include "TDecSlice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSlice::TDecSlice()
{
  for (Int i=0; i<GRF_MAX_NUM_EFF; i++)
  for (Int j=0; j<2; j++)
  {
    m_apcVirtPic[j][i] = NULL;
  }
}

TDecSlice::~TDecSlice()
{
}

Void TDecSlice::create( TComSlice* pcSlice, Int iWidth, Int iHeight, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth )
{
  // allocate required memory for generated reference frame
  for (Int j=0; j<2; j++)
  for (Int i=0; i<pcSlice->getAddRefCnt( (RefPicList)j ) ; i++ )
  {
    if(m_apcVirtPic[j][i] == NULL)
    {
      m_apcVirtPic[j][i] = new TComPic;
      m_apcVirtPic[j][i]->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth, true );
    }
  }
}

Void TDecSlice::destroy()
{
  // free buffers for generated reference frame
  for (Int i=0; i<GRF_MAX_NUM_EFF; i++)
  for (Int j=0; j<2; j++)
  {
    if(m_apcVirtPic[j][i])
    {
      m_apcVirtPic[j][i]->destroy();
      delete m_apcVirtPic[j][i];
      m_apcVirtPic[j][i]=NULL;
    }
  }
}

Void TDecSlice::init(TDecEntropy* pcEntropyDecoder, TDecCu* pcCuDecoder)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcCuDecoder       = pcCuDecoder;
}

Void TDecSlice::decompressSlice(TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComDataCU* pcCU;
  UInt        uiIsLast = 0;

  // decoder don't need prediction & residual frame buffer
  rpcPic->setPicYuvPred( 0 );
  rpcPic->setPicYuvResi( 0 );

#if HHI_RQT
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_V( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( rpcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif
#endif
  // for all CUs
  for( Int iCUAddr = 0; !uiIsLast; iCUAddr++ )
  {
    pcCU = rpcPic->getCU( iCUAddr );
    pcCU->initCU( rpcPic, iCUAddr );

#if HHI_RQT
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
#endif
    m_pcCuDecoder->decodeCU     ( pcCU, uiIsLast );
    m_pcCuDecoder->decompressCU ( pcCU );
#if HHI_RQT
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif
#endif
  }
}

Void TDecSlice::generateRefPicNew ( TComSlice* rpcSlice )
{
  if (rpcSlice->getSliceType() == I_SLICE)
  {
    return;
  }

  if (rpcSlice->getAddRefCnt(REF_PIC_LIST_0) == 0 && rpcSlice->getAddRefCnt(REF_PIC_LIST_1) == 0) return;

  rpcSlice->setVirtRefBuffer(m_apcVirtPic);

  for (Int iDir = 0; iDir < (rpcSlice->getSliceType() == P_SLICE ? 1 :2); iDir++)
  {
    RefPicList eRefPicList = (RefPicList) iDir;
    for (Int i = 0; i < rpcSlice->getAddRefCnt(eRefPicList); i++)
    {
      EFF_MODE eEffMode = rpcSlice->getEffectMode(eRefPicList, i);
      if (eEffMode >= EFF_WP_SO && eEffMode <= EFF_WP_O)
      {
        rpcSlice->generateWPSlice(eRefPicList, eEffMode, i);
      }
    }
  }
  rpcSlice->linkVirtRefPic();
}

