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

/** \file			TDecSlice.cpp
    \brief		slice decoder class
*/

#include "TDecSlice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSlice::TDecSlice()
{
  m_apcPicYuvPred = NULL;
  m_apcPicYuvResi = NULL;

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
  //-- create once...
  if ( m_apcPicYuvPred == NULL)
  {
    m_apcPicYuvPred  = new TComPicYuv;
    m_apcPicYuvPred->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  }
  if ( m_apcPicYuvResi == NULL)
  {
    m_apcPicYuvResi  = new TComPicYuv;
    m_apcPicYuvResi->create( iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  }

	// allocate only required memory
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
  if(m_apcPicYuvPred)
  {
    m_apcPicYuvPred->destroy();
    delete m_apcPicYuvPred;
    m_apcPicYuvPred  = NULL;
  }
  if(m_apcPicYuvResi)
  {
    m_apcPicYuvResi->destroy();
    delete m_apcPicYuvResi;
    m_apcPicYuvResi  = NULL;
  }

	// free non-NULL buffer
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

Void TDecSlice::decompressSliceCU(TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  UInt    uiIsLast = 0;
  TComDataCU* pcCU;

  UInt uiNumPartsInWidth = rpcPic->getNumPartInWidth();
  UInt uiNumParts        = rpcPic->getNumPartInCU();

  assert(m_apcPicYuvPred);
  assert(m_apcPicYuvResi);

	rpcPic->setPicYuvPred(m_apcPicYuvPred);
  rpcPic->setPicYuvResi(m_apcPicYuvResi);

  if(rpcPic->getSlice()->getSPS()->getUseExC())
  {
    m_pcEntropyDecoder->decodeExtremeValue(rpcPic);

    //EXCBand
	  m_pcEntropyDecoder->decodeBandCorrValue( rpcPic);
  }

	for( Int iCUAddr = 0; !uiIsLast; iCUAddr++ )
  {
    pcCU = rpcPic->getCU( iCUAddr );
    pcCU->initCU( rpcPic, iCUAddr );

    m_pcCuDecoder->decodeCU    ( pcCU, uiIsLast );
    m_pcCuDecoder->decompressCU( pcCU );
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
      if (eEffMode >= EFF_WP_SO && eEffMode <= EFF_WP_M1)
        rpcSlice->generateWPSlice(eRefPicList, eEffMode, i);
      if (eEffMode >= EFF_IM && eEffMode <= EFF_PM)
      {
        xSetGMParam(rpcSlice, eRefPicList);
        rpcSlice->generateGMSlice(eRefPicList, i, &m_cGlobalMotion);
      }
    }
  }
  rpcSlice->linkVirtRefPic();
}

Void TDecSlice::xSetGMParam(TComSlice* rpcSlice, RefPicList e)
{
  UInt ui;
  UInt uiNumOfSpritePoints = rpcSlice->getNumOfSpritePoints();
  Int iHorSpatRef = rpcSlice->getHorSpatRef();
  Int iVerSpatRef = rpcSlice->getVerSpatRef();

  TrajPoint* pcRefPointCoord = new TrajPoint[uiNumOfSpritePoints];
  for (ui = 0; ui < uiNumOfSpritePoints; ui++)
  {
    pcRefPointCoord[ui].x = iHorSpatRef + (ui%2)*rpcSlice->getSPS()->getWidth();
    pcRefPointCoord[ui].y = iVerSpatRef + (ui>>1)*rpcSlice->getSPS()->getHeight();
    rpcSlice->setRefPoint(e, ui, pcRefPointCoord[ui]);
  }

  m_cGlobalMotion.addTraj(uiNumOfSpritePoints, rpcSlice->getDiffTrajPoints(e), rpcSlice->getTrajPoints(e));
  delete[] pcRefPointCoord;
}


