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

/** \file			TComPrediction.h
    \brief		prediction class (header)
*/

#ifndef __TCOMPREDICTION__
#define __TCOMPREDICTION__


// Include files
#include "TComPic.h"
#include "TComMotionInfo.h"
#include "TComPattern.h"
#include "TComTrQuant.h"
#include "TComPredFilter.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// prediction class
class TComPrediction : public TComPredFilter
{
protected:
  Int*			m_piYuvExt;
  Int       m_iYuvExtStride;
  Int       m_iYuvExtHeight;
  Int       m_aiBuf[16*(16+6)];
  Int 			m_iDIFHalfTap;
	Int				m_iDIFHalfTapC;

  TComYuv   m_acYuvPred[2];
  TComYuv   m_acYuvPredTemp;
  TComYuv   m_cYuvExt;
  TComYuv		m_cTemplate;
  TComYuv		m_cTemplateCand;

  Pel*			m_pLumaRecBuffer;
  Pel*			m_pHorDsLumaRecBuffer;
  Pel*			m_pDsLumaRecBuffer;
  Pel*			m_pSegBuffer;
  Int				m_iLumaRecStride;
  Int				m_iSegStride;

  UInt			m_uiLambda;
  UInt*			m_puiComponentCostMem;
  UInt*			m_puiComponentCost;
  Int				m_iMvPredVer;
  Int				m_iMvPredHor;
  UInt			m_auiMVPIdxCost[AMVP_MAX_NUM_CANDS];
  UInt			m_uiMaxTemplateSearchRange;
  Double		m_dLastQp;

	Bool			m_bUseMPI;
  Bool			m_bHAM;

  Void xGetMbAddrAndLumaIdx     ( Int& rxPxlRef, Int& ryPxlRef );
  Void xPredInterLumaPxl				( TComPicYuv* pcPicYuvRef, Int xMB, Int x, Int yMB, Int y, TComMv* pcMv, Int* piPxl );
  // Intra NxN mode with 4 directions
  Void xPredIntraDC             ( TComPattern* pcTComPattern, Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraVert           ( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraHor            ( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraPlane        ( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  //--

  Void xMPI_POST_PROCESSING( Pel* pSrc, Int iSrcStride, Pel*& tpDst, Int iDstStride, UInt iWidth, UInt iHeight, UChar indexMPI );

  // ADI
  Void xPredIntraVertAdi ( Int* pSrc, Int iSrcStride, Pel*& tpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraHorAdi  ( Int* pSrc, Int iSrcStride, Pel*& tpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraDCAdi   ( Int* pSrc, Int iSrcStride, Pel*& tpDst, Int iDstStride, UInt iWidth, UInt iHeight, Bool bAboveAvail, Bool bLeftAvail );
  Void xPredIntraPlaneAdi( Int* pSrc, Int iSrcStride, Pel*& tpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Void xPredIntraBiAdi   ( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight );
  Int  getContextPixel   ( UInt uiDirMode, UInt uiXYflag, Int iTempD, Bool& bCenter );

	// TM_INTRA
  Void xPredIntraTM   ( Pel* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, UInt iWidth, UInt iHeight	  , TComPattern* pcTComPattern);

  // Intra8x8 mode with 9 directions
  Void xFilterHorPels           ( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& pFilteredPels );
  Void xFilterVerPels           ( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& pDst );
  Void xFilterXPels             ( TComPattern* pcTComPattern, Int iBlkIdx, Pel*& rpDst );

  Void xPredIntra8Vert          ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8Hor           ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8DC            ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8DiagDownLeft  ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8DiagDownRight ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8VertRight     ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8HorDown       ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8VertLeft      ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra8HorUp         ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );

  // Intra4x4 mode with 9 directions
  Void xPredIntra4Vert          ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4Hor           ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4DC            ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4DiagDownLeft  ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4DiagDownRight ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4VertRight     ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4HorDown       ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4VertLeft      ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );
  Void xPredIntra4HorUp         ( TComPattern* pcTComPattern, Pel*& rpPred, Int iStride );

  // for inter
  Void xPredInterUni            ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx );
  Void xPredInterBi             ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight,                         TComYuv*& rpcYuvPred, Int iPartIdx );
  Void xPredInterLumaBlk        ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuv,     Int iPartIdx, Int iDstPartAddr = -1 );

	// HAM functions
  Void  xPredInterLumaBlkHMVApply ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv,
																		Int iWidth, Int iHeight, TComYuv*& rpcYuv, Int iDstPartAddr );

  Void  xPredInterLumaBlkHMVApplyME ( Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv, Int iWidth, Int iHeight, Pel* piOrg,Int iStrideOrg, Int dMVx, Int dMVy);
  Void  xPredInterLumaBlkHMVEstimateApplyME ( Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv,
														  Int iWidth, Int iHeight, Pel* piOrg,Int iStrideOrg, Int dMVx, Int dMVy);
  Void  xPredInterLumaBlkHMVEstimateApply ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv,
	  Int iWidth, Int iHeight, TComYuv*& rpcYuv, Int iDstPartAddr );

  Void  xHAM_Filter (Pel*  piRefY, Int iRefStride,Pel*  piDstY,Int iDstStride, Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac);
  Void  xHAM_FilterC(Pel*  piRefY, Int iRefStride,Pel*  piDstY,Int iDstStride, Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac);

  Void  xQuantRemainder(double alfa1, double alfa2, Int& dMVx, Int& dMVy);

  Void xPredInterLumaBlk_NIF		  ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, Int iDstPartAddr = -1 );

  Void xPredInterChromaBlk      ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv );

  Void xGetNeighTemplate( TComDataCU* pcCU, UInt uiPartAddr, TComYuv* pcTemplate, Int iTemplSize, Int iSizeX, Int iSizeY );
  UInt xCalcDistNeighTemplate( TComYuv* pcTemplate, TComYuv* pcTemplateCand, Int iTemplSize, Int iSizeX, Int iSizeY, UInt uiThresDist );
  Void xNeighTemplateMatchingProcess( TComDataCU*     pcCU, UInt uiPartAddr,
                                              TComYuv* pcTemplate,
                                              TComYuv* pcTemplateCand,
                                              TComMv      cMvCand,
                                              TComMv&      rcMatchedMv,
                                              RefPicList  eRefPicList,
                                              Int         iRefIdx,
                                              Int iTemplSize, Int iSizeX, Int iSizeY,
                                              Int         iSearchRange,
                                              UInt&     uiDist,
                                              UInt     uiThresDist );
  Void xGetCurrTemplate( TComDataCU* pcCU, UInt uiPartAddr, TComYuv* pcTemplate, Int iSizeX, Int iSizeY );

  UInt xCalcDistCurrTemplate( TComYuv* pcTemplate, TComYuv* pcTemplateCand, UInt uiPartAddr, Int iSizeX, Int iSizeY, UInt uiThresDist );
  Void xCurrTemplateMatchingProcess( TComDataCU*     pcCU,
                                        UInt uiPartIdx,
                                              TComYuv* pcTemplate,
                                              TComYuv* pcTemplateCand,
                                              TComMv      cMvCand,
                                              Int iMVPIdx, //--T0430
                                              TComMv&      rcMatchedMv,
                                              RefPicList  eRefPicList,
                                              Int         iRefIdx,
                                              Int iSizeX, Int iSizeY,
                                              Int         iSearchRange,
                                              UInt&     uiDist,
                                              UInt     uiThresDist );

  UInt xGetMvBits( Int x, Int y, UInt uiShift = 0)           { return m_puiComponentCost[(x-m_iMvPredHor) >> uiShift] + m_puiComponentCost[(y-m_iMvPredVer) >> uiShift]; }  //{ return m_puiHorCost[x] + m_puiVerCost[y]; }
  UInt xGetMVPIdxBits( Int iMVPIdx )           { return m_auiMVPIdxCost[iMVPIdx]; }
  Void xSetPredictor( TComMv cMv )
  {
    m_iMvPredHor = cMv.getHor();
    m_iMvPredVer = cMv.getVer();
  }
  Void xUninitRateDistortionModel();
  UInt xGetComponentBits( Int iVal );
  UInt xGetSymbolBits( Int iVal );
  UInt xCalcRdCost( UInt uiBits, UInt uiDistortion );

  Void xWeightedAverage( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, Int iRefIdx0, Int iRefIdx1, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst );

  // CCCP
  Void xPelsSet             ( Pel* pcDst, Pel cValue, UInt uiNum );
  Void xGetRecPixels        ( TComPattern* pcPattern, Pel* pRecSrc, Int iRecSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt );
  Void xSegmentation        ( Pel* pSrc, Int iSrcStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt );
  Void xSegmentation        ( Pel* pSrc, Int iStride, UInt uiWidth0, UInt uiHeight0, UInt uiExt, Int iPartNum );
  Void xGetThresholds       ( Pel* pSrc, Int iStride, Int* piThreshold, Int iMinPel, Int iMaxPel, Int iShift, Int iPartNum, UInt uiWidth, UInt uiHeight );
  Void xGetSegmentPrediction( TComPattern* pcPattern, Pel* pSrc0, Int iSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth, UInt uiHeight, UInt uiExt0, Int iPartNum );
  Void xDownsampling        ( UInt uiWidth, UInt uiHeight, UInt uiExt );

public:
  TComPrediction();
  virtual ~TComPrediction();

	Void	init( Bool bUseMPI ) { m_bUseMPI = bUseMPI; }
  Void	initTempBuff();
  Void   setHAMUsed( Bool b ) { m_bHAM = b; }

  // CCCP
  Void predCCCPIntraChroma( TComPattern* pcPattern, Pel* piSrc, UInt uiSrcStride, Pel* pPred, UInt uiPredStride, UInt uiCWidth, UInt uiCHeight, UInt uiCExt );

	// intra
  Void predIntraLumaNxN         ( TComPattern* pcTComPattern, UInt iPredMode, Pel*& rpcYuvPred, UInt uiStride, UInt iWidth, UInt iHeight, UChar  indexMPI );
  Bool checkIntraLumaAvailable  ( TComPattern* pcTComPattern, UInt iPredMode, UInt iWidth, UInt iHeight );
  Bool checkIntraLumaAvailableADI( TComDataCU*  pcCU,          UInt uiDirMode, UInt uiWidth, Bool bAbove, Bool bLeft );
  Void predIntraChromaNxN       ( TComPattern* pcTComPattern, Pel* piSrc, UInt uiSrcStride, UInt iPredMode, Pel* piPred, Int  iDstStride, UInt iWidth, UInt iHeight );
  Bool CheckIntraChromaNxN      ( TComPattern* pcTComPattern, UInt iPredMode );
  Void calcMotionVector         ( TComDataCU*  pcCU);
  Void motionCompensation       ( TComDataCU*  pcCU, TComYuv* pcYuvPred, RefPicList eRefPicList = REF_PIC_LIST_X, Int iPartIdx = -1 );

  Void getMvPredTemplateMatch( TComDataCU* pcCU, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred, Int iTemplSize = AMVP_TEMP_SIZE, Int iSearchRange = AMVP_TEMP_SR, Bool bFilled = false );
  Void initRateDistortionModel( Double dQp, UInt uiSearchLimit = AMVP_MAX_TEMP_SR ); //--T0430

  // ADI
  Void predIntraLumaNxNAdi(TComPattern* pcTComPattern, UInt uiDirMode, Pel*& rpPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft, UChar indexMPI );
  Void predIntraChromaAdi(TComPattern* pcTComPattern,Int* piSrc, UInt uiDirMode, Pel* piPred, Int  iPredStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft, Pel* piSrcOld, UInt uiSrcStrideOld );

	// CIP
	Void recIntraLumaNxNCIP		( TComPattern* pcTComPattern, Pel* pPred, Pel* pResi, Pel* pReco, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAboveAvail, Bool bLeftAvail );

  Int* getPredicBuf()       { return m_piYuvExt;     }
  Int  getPredicBufWidth()  {return m_iYuvExtStride; }
  Int  getPredicBufHeight() {return m_iYuvExtHeight; }

};// END CLASS DEFINITION TComPrediction


#endif // __TCOMPREDICTION__
