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

/** \file			TDecGop.cpp
    \brief		GOP decoder class
*/

#include "TDecGop.h"
#include "TDecCAVLC.h"
#include "TDecSbac.h"

#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

inline int mSign(int x)
 {
  // http://bits.stephan-brumme.com/sign.html
  // only for 32-bit integer with C-style shifts
  // may be unportable!
  // minusOne will be 0xFFFFFFFF (=> -1) for negative numbers, else 0x00000000
   int minusOne = x >> 31;

   // plusOne will be 0x00000001 for positive numbers, else 0x00000000
   unsigned int negateX = (unsigned int) -x;
   int plusOne = (int)(negateX >> 31);

   // at least minusOne or plusOne is zero, the other one gives our desired result
   int result = minusOne | plusOne;
   return result;
 }

TDecGop::TDecGop()
{
  m_iGopSize = 0;
}

TDecGop::~TDecGop()
{

}

Void TDecGop::create()
{

}

Void TDecGop::destroy()
{

}

Void TDecGop::init (TDecEntropy* pcEntropyDecoder, TDecSbac* pcSbacDecoder, TDecCavlc* pcCavlcDecoder, TDecSlice* pcSliceDecoder, TComLoopFilter* pcLoopFilter, TComAdaptiveLoopFilter* pcAdaptiveLoopFilter )
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcSbacDecoder    = pcSbacDecoder;
  m_pcCavlcDecoder    = pcCavlcDecoder;
  m_pcSliceDecoder    = pcSliceDecoder;
  m_pcLoopFilter      = pcLoopFilter;
  // Adaptive Loop filter
  m_pcAdaptiveLoopFilter      = pcAdaptiveLoopFilter;
  //--Adaptive Loop filter
}

Void TDecGop::decompressGop (Bool bEos, TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComSlice*  pcSlice = rpcPic->getSlice();

  //-- For time output for each slice
  long iBeforeTime = clock();

  UInt iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
  {
    m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
  }
  else
  {
    m_pcEntropyDecoder->setEntropyDecoder (m_pcCavlcDecoder);
  }

  m_pcEntropyDecoder->setBitstream      (pcBitstream);
  m_pcEntropyDecoder->resetEntropy      (pcSlice);

  //  Byte align & start SBAC
  if (iSymbolMode)
  {
    m_pcEntropyDecoder->preloadSbac();
  }

  ALFParam cAlfParam;

	if ( rpcPic->getSlice()->getSPS()->getUseALF() )
	{
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
    m_pcEntropyDecoder->decodeAlfParam(&cAlfParam);
	}

  m_pcSliceDecoder->decompressSliceCU(pcBitstream, rpcPic);

  //-- Loop filter
  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), 0, 0);
  m_pcLoopFilter->loopFilterPic( rpcPic );

	if( pcSlice->getSPS()->getUseExC() )
	{
		// Nothing to do if B-slice or small picture
		// Moved here from xExtremePass body
		//kolya
		if (rpcPic->getSlice()->getSliceType() != B_SLICE
			|| rpcPic->getFrameHeightInCU() > EXC_NO_EC_BSLICE)
			xExtremePass( rpcPic );
	}

  // Adaptive Loop filter
  if( rpcPic->getSlice()->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->ALFProcess(rpcPic, &cAlfParam);
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
	}
  //--Adaptive Loop filter

	// EXCBand
	// condition check moved here //kolya
	if( pcSlice->getSPS()->getUseExC() )
	{
		if (!(rpcPic->getSlice()->getSliceType() == B_SLICE
			&& rpcPic->getFrameHeightInCU() <= EXC_NO_EC_BSLICE))
			xCorrBand( rpcPic );
	}

	//-- For time output for each slice
  printf("\nPOC %4d ( %c-SLICE, QP%3d ) ",
                        pcSlice->getPOC(),
                        pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B',
                        pcSlice->getSliceQp() );

  Double dDecTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  printf ("[DT %6.3f] ", dDecTime );

  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf ("[L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      UInt uiOrgNumRefIdx;
      uiOrgNumRefIdx = pcSlice->getNumRefIdx(RefPicList(iRefList))-pcSlice->getAddRefCnt(RefPicList(iRefList));
      UInt uiNewRefIdx= iRefIndex-uiOrgNumRefIdx;
      if (iRefIndex >= (int)uiOrgNumRefIdx) {
        if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_P1)
          printf ("%dr+ ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_M1)
          printf ("%dr- ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_WP_SO)
          printf ("%dw ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_AM)
          printf ("%da ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_IM)
          printf ("%di ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        else if (pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx)==EFF_PM)
          printf ("%dp ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
      else {
        printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
    }
    printf ("] ");
  }

  rpcPic->setReconMark(true);
}

//EXCBand
Void
TDecGop::xCorrBand(TComPic*& rpcPic)
{
	Int x, y;
	Pel* pRec = rpcPic->getPicYuvRec()->getLumaAddr();

	Int iStride = rpcPic->getStride();
	Int iWidth = rpcPic->getPicYuvRec()->getWidth();
	Int iHeight = rpcPic->getPicYuvRec()->getHeight();

	// Moved here for getMaxBandNumber not to be called from within loop
	//kolya
	Int iMaxBandNumber =  rpcPic->getMaxBandNumber();
	Int iExtractingMask = (EXB_NB - 1) << ( 12 - EXB_BITS );

	Int aiCor[64];

	////for overhead writing
	for (int i = 0; i < iMaxBandNumber; i++)
	{
		aiCor[i] = rpcPic->getCorrBandExType(i);
#if EXB_CUT
		aiCor[i] = aiCor[i] * 2;
#endif
	}

	pRec = rpcPic->getPicYuvRec()->getLumaAddr();

	Pel   iMax = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);

	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			pRec[x] = Clip3(0,iMax,pRec[x] + aiCor[ (pRec[x] & iExtractingMask) >> (12 - EXB_BITS)  ]);
		}
		pRec += iStride;
	}
}
//EXCBand

Void
TDecGop::xExtremePass(TComPic*& rpcPic)
{
	Int x = 0;
	Int y = 0;

	Pel* pRec = rpcPic->getPicYuvRec()->getLumaAddr();
	Int iStride = rpcPic->getStride();
  Int iWidth = rpcPic->getPicYuvRec()->getWidth();
	Int iHeight = rpcPic->getPicYuvRec()->getHeight();

	Int iExtremeType;

	// NOTE: all "new"s here may be substitude by constant, e.g. for IPPP case
	// where "new" will be called frequently
	// but still I think this won't bother anyone //kolya

	// Array of upper signs
	//kolya
	Int* iUpperSignBits = new Int[iWidth];

	// Array of corrections //kolya (optimized for EXC_CORR_LIMIT 1)
	Int iExtremeCorrArray[9];
	iExtremeCorrArray[4] = 0;
	iExtremeCorrArray[3] = 0;
	iExtremeCorrArray[5] = 0;

	for (x = 6; x < 9; x++)
	{
		iExtremeCorrArray[x] = rpcPic->getMinValCorr(x - 4);
		iExtremeCorrArray[8 - x] = rpcPic->getMaxValCorr(x - 4);
	}

	Pel iMax = ((1 << (g_uiBitDepth + g_uiBitIncrement)) - 1);

	// Temporary storage
	Pel* pRecD = new Pel[iWidth - 2];
	Pel* pRecD2 = new Pel[iWidth - 2];
	Pel* pSwap;
	memcpy( pRecD2, pRec + 1, sizeof(Pel)*(iWidth - 2) );

	Int iLeft = 0;
	Int iRight = 0;
	Int iSignL = 0;
	Int iSignR = 0;
	Int iSignD = 0;

	pRec += iStride;

	// augmented for not to do arithmetic inside loop //kolya
	pRecD--;
	pRecD2--;

	// Upper signs array explicit initialization
	for (x = 1; x < iWidth - 1; x++)
	{
		iUpperSignBits[x] = mSign(pRec[x - iStride] - pRec[x]);
	}

	for (y = 1; y < iHeight - 1; y++)
	{
		iLeft = pRec[0] - pRec[1];
		iSignL = mSign(iLeft);

		for (x = 1; x < iWidth - 1; x++)
		{
			iExtremeType = 4;

			// left-right pair
			iRight = pRec[x + 1] - pRec[x];
			iSignR = mSign( iRight );

			iExtremeType = iExtremeType + iSignR + iSignL;

			// save sign for next step
			iSignL = -iSignR;

			// upper-lower pair
			iSignD = mSign(pRec[x + iStride] - pRec[x]);

			iExtremeType = iExtremeType + iSignD + iUpperSignBits[x];
			iUpperSignBits[x] = -iSignD;

			pRecD[x] = Clip3(0,iMax,(Int)pRec[x]+iExtremeCorrArray[iExtremeType]);
		}
		// copy previous line
		memcpy( pRec - iStride + 1, pRecD2 + 1, sizeof(Pel)*(iWidth-2) );
		// swap saving lines
		pSwap = pRecD2;
		pRecD2 = pRecD;
		pRecD = pSwap;

		pRec += iStride;
	}
	memcpy( pRec - iStride + 1, pRecD2 + 1, sizeof(Pel)*(iWidth-2) );

	pRecD2++;
	pRecD++;

	delete [] iUpperSignBits;
	delete [] pRecD;
	delete [] pRecD2;
}
