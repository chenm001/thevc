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

/** \file			TComGlobalMotion.h
    \brief		global motion compensation (header)
		\todo     split encoder and decoder functions (create TEncGlobalMotion class) \n
							not all entities are documented
*/

#ifndef __TCOMGLOBALMOTION__
#define __TCOMGLOBALMOTION__

#include "CommonDef.h"
#include "TComPredFilter.h"

#include <stdio.h>
#include <assert.h>

class TComPicYuv;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// global motion compensation class
class TComGlobalMotion : public TComPredFilter
{
protected:

	// ------------------------------------------------------------------------------------------------------------------
	// motion compensation functions
	// ------------------------------------------------------------------------------------------------------------------

  Void	xPredInterLumaBlk		( TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints,
															TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc,
															Int iWidth, Int iHeight, Pel* piDstY, Int iDstStride, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

  Void	xPredInterChromaBlk ( TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints,
															TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc,
															Int iWidth, Int iHeight, Pel* piDstCb, Pel* piDstCr, Int iDstStride, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

  Void	xPredInterLumaBlk   ( Int C[], Double Cd[], TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints,
															UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstY, Int iDstStride, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

  Void	xPredInterChromaBlk ( Int C[], Double Cd[], TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints,
															UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstCb, Pel* piDstCr, Int iDstStride, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

  Void	xSetWarpingParam		( Int C[], Double	Cd[], UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc, Int iHorSpatRef = 0, Int iVerSpatRef = 0);
  Int		xSynthesisLuma			( Int i,   Int j,  Int *I_,  Int *J_,  Int C[], Double Cd[], Int iHorSpatRef, Int iVerSpatRef, UInt uiNumOfSpritePoints);
  Int		xSynthesisChroma		( Int ic,  Int jc, Int *Ic_, Int *Jc_, Int C[], Double Cd[], Int iHorSpatRef, Int iVerSpatRef, UInt uiNumOfSpritePoints);

	// ------------------------------------------------------------------------------------------------------------------
	// motion estimation based on specific motion model
	// ------------------------------------------------------------------------------------------------------------------

	/// translational global motion model
  Void	xTranslationalGME	( TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

	/// isotropic global motion model
  Void	xIsotropicGME			( TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

	/// affine global motion model
  Void	xAffineGME				( TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

	/// perspective global motion model
  Void	xPerspectiveGME		( TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

	/// 3-step search
	Int		xModifiedThreeStepSearch	( Pel* pRefWork,				Pel* pCurrWork,
																		Int  iCurrWidth,			Int  iCurrHeight, Int iRefWidth, Int iRefHeight,
																		Int  iOffsetX,				Int  iOffsetY,
																		Int* piBestLocationX, Int* piBestLocationY);

	// ------------------------------------------------------------------------------------------------------------------
	// interpolation filters
	// ------------------------------------------------------------------------------------------------------------------

  Void	xThreeTapFilter( Pel *Low, Pel *Hight, Int width, Int height );

#if GMC_SIX_TAP
  Void	xFilterQuarterPelsLuma		( Pel* piSrc, Int iSrcStride, Int& P00, Int& P01, Int& P10, Int& P11, Int px, Int py );
  Void	xFilterQuarterPelsChroma	( Pel* piSrc, Int iSrcStride, Int& P00, Int& P01, Int& P10, Int& P11, Int px, Int py );
#endif

	// ------------------------------------------------------------------------------------------------------------------
	// utility functions
	// ------------------------------------------------------------------------------------------------------------------

  Int		xDeltaMP( Double *dA, Int n, Double *db, Double *dm );
  Int		xRound(Double d) { return ((Int)((d) >= 0 ? (d) + 0.5 : (d) - 0.5));}

public:
  TComGlobalMotion();
  ~TComGlobalMotion();

	// ------------------------------------------------------------------------------------------------------------------
	// trajectory
	// ------------------------------------------------------------------------------------------------------------------

  Int		makeTraj			( SpriteMotion cSpriteMotion, UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj);
  Void	makeDiffTraj	( UInt uiNumOfSpritePoints, TrajPoint *pcTraj, TrajPoint* pcDiffTraj );
  Void	addTraj				( UInt uiNumOfSpritePoints, TrajPoint* pcDiffTraj, TrajPoint* pcTraj );

	// ------------------------------------------------------------------------------------------------------------------
	// global motion estimation
	// ------------------------------------------------------------------------------------------------------------------

  Void	globalMotionEstimation	( TComPicYuv* pcRefFrame,  TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion,
																	UInt uiNumOfSpritePoints, Int iHorSpatRef = 0, Int iVerSpatRef = 0);

	// ------------------------------------------------------------------------------------------------------------------
	// global motion compensation
	// ------------------------------------------------------------------------------------------------------------------

  Void	getGlobalWarpPic				( TComPicYuv* pcPicYuvDst, TComPicYuv* pcPicYuvRef, UInt uiNumOfSpritePoints,
																	TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc,
																	Int iHorSpatRef = 0, Int iVerSpatRef = 0);
};

#endif // __TCOMGLOBALMOTION__
