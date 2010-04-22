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

/** \file			TComGlobalMotion.cpp
    \brief		global motion compensation
		\todo			not all entities are documented
*/

#include "TComGlobalMotion.h"
#include "TComPicYuv.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComGlobalMotion::TComGlobalMotion()
{
}

TComGlobalMotion::~TComGlobalMotion()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Int TComGlobalMotion::makeTraj(SpriteMotion cSpriteMotion, UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj)
{
  Double	denom, nom_x, nom_y;
  Int 	  spriteorvop_point_coord_x,spriteorvop_point_coord_y;
  UInt     ui;

  UInt uiAbsSum = 0;

  for (ui=0; ui<uiNumOfSpritePoints; ui++)
  {
    denom = cSpriteMotion.a7 * pcRefPointCoord[ui].x +
		    cSpriteMotion.a8 * pcRefPointCoord[ui].y
        + 1;
    nom_x = cSpriteMotion.a1 * pcRefPointCoord[ui].x +
		    cSpriteMotion.a2 * pcRefPointCoord[ui].y +
        cSpriteMotion.a3;
    nom_y = cSpriteMotion.a4 * pcRefPointCoord[ui].x +
		    cSpriteMotion.a5 * pcRefPointCoord[ui].y +
        cSpriteMotion.a6;

    /* coordinates in the sprite or vop corresponding to the reference points (quantized to half-pel) */
    spriteorvop_point_coord_x = xRound(GMC_AFF_ACC*nom_x/denom);
    spriteorvop_point_coord_y = xRound(GMC_AFF_ACC*nom_y/denom);

    /* motion vectors according to Sec. 3.9.5.1.4 */
    pcTraj[ui].x =  spriteorvop_point_coord_x - GMC_AFF_ACC*pcRefPointCoord[ui].x;
    pcTraj[ui].y =  spriteorvop_point_coord_y - GMC_AFF_ACC*pcRefPointCoord[ui].y;

    uiAbsSum += abs(pcTraj[ui].x);
    uiAbsSum += abs(pcTraj[ui].y);
  }

  return uiAbsSum;
}

Void TComGlobalMotion::makeDiffTraj (UInt uiNumOfSpritePoints, TrajPoint* pcTraj, TrajPoint* pcDiffTraj)
{
  UInt ui;

  pcDiffTraj[0].x = pcTraj[0].x;
  pcDiffTraj[0].y = pcTraj[0].y;

  for (ui=1; ui<Min(uiNumOfSpritePoints,3); ui++)
  {
    pcDiffTraj[ui].x = pcTraj[ui].x - pcTraj[0].x;
    pcDiffTraj[ui].y = pcTraj[ui].y - pcTraj[0].y;
  }
  if (uiNumOfSpritePoints==4)
  {
    pcDiffTraj[3].x = pcTraj[3].x - pcTraj[2].x - pcTraj[1].x + pcTraj[0].x;
    pcDiffTraj[3].y = pcTraj[3].y - pcTraj[2].y - pcTraj[1].y + pcTraj[0].y;
  }

  return;
}

Void TComGlobalMotion::addTraj (UInt uiNumOfSpritePoints, TrajPoint* pcDiffTraj, TrajPoint* pcTraj)
{
  UInt ui;

  pcTraj[0].x = pcDiffTraj[0].x;
  pcTraj[0].y = pcDiffTraj[0].y;

  for (ui=1; ui<Min(uiNumOfSpritePoints,3); ui++)
  {
    pcTraj[ui].x = pcDiffTraj[ui].x + pcTraj[0].x;
    pcTraj[ui].y = pcDiffTraj[ui].y + pcTraj[0].y;
  }
  if (uiNumOfSpritePoints==4)
  {
    pcTraj[3].x = pcDiffTraj[3].x + pcTraj[2].x + pcTraj[1].x - pcTraj[0].x;
    pcTraj[3].y = pcDiffTraj[3].y + pcTraj[2].y + pcTraj[1].y - pcTraj[0].y;
  }

  return;
}

/* specification of parameters
    a1 x + a2 y + a3    a4 x + a5 y + a6
    ----------------    ----------------
    a7 x + a8 y + 1     a7 x + a8 y + 1

   d=2  X'i = X + M[0], Y'i = Y + M[1]
   d=4  X'i = M[0]X + M[1]Y + M[2], Y'i = -M[1]X + M[0]Y + M[3]
   d=6  X'i = M[0]X + M[1]Y + M[2], Y'i = M[3]X + M[4]Y + M[5]
   d=8  X'i = M[0]X + M[1]Y + M[2], Y'i = M[3]X + M[4]Y + M[5]
              --------------------        --------------------
              M[6]X + M[7]Y +  1          M[6]X + M[7]Y +  1
*/
Void TComGlobalMotion::globalMotionEstimation(TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, UInt uiNumOfSpritePoints, Int iHorSpatRef , Int iVerSpatRef)
{
  switch(uiNumOfSpritePoints) {
    case 0:
      rcSpriteMotion.a1 = 1.0;
      rcSpriteMotion.a2 = 0.0;
      rcSpriteMotion.a3 = 0.0;
      rcSpriteMotion.a4 = 0.0;
      rcSpriteMotion.a5 = 1.0;
      rcSpriteMotion.a6 = 0.0;
      rcSpriteMotion.a7 = 0.0;
      rcSpriteMotion.a8 = 0.0;
      break;
    case 1:
      xTranslationalGME(pcRefFrame, pcCurrFrame, rcSpriteMotion, iHorSpatRef, iVerSpatRef);
      break;
    case 2:
      xIsotropicGME(pcRefFrame, pcCurrFrame, rcSpriteMotion, iHorSpatRef, iVerSpatRef);
      break;
    case 3:
      xAffineGME(pcRefFrame, pcCurrFrame, rcSpriteMotion, iHorSpatRef, iVerSpatRef);
      break;
    case 4:
      xPerspectiveGME(pcRefFrame, pcCurrFrame, rcSpriteMotion, iHorSpatRef, iVerSpatRef);
      break;
    default:
      printf("no_of_sprite_warping_points > 4\n");
      assert(0);
      break;
  }
}

Void TComGlobalMotion::getGlobalWarpPic( TComPicYuv* pcPicYuvDst, TComPicYuv* pcPicYuvRef, UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc, Int iHorSpatRef, Int iVerSpatRef)
{
  Int		C[10];		/* for warping caluculation */
  Double	Cd[10];		/* for warping caluculation */

  /* setting warping Consts */
  xSetWarpingParam(C, Cd, uiNumOfSpritePoints, pcRefPointCoord, pcTraj, uiWarpAcc, iHorSpatRef, iVerSpatRef );
  Pel* piDstY = pcPicYuvDst->getLumaAddr();
  Int iDstStride = pcPicYuvDst->getStride();
  Pel* piDstCb = pcPicYuvDst->getCbAddr();
  Pel* piDstCr = pcPicYuvDst->getCrAddr();
  Int iDstCStride = pcPicYuvDst->getCStride();

  Int iWidth = pcPicYuvDst->getWidth();
  Int iHeight = pcPicYuvDst->getHeight();

  xPredInterLumaBlk( C, Cd, pcPicYuvRef, 0, 0, uiNumOfSpritePoints, uiWarpAcc, iWidth, iHeight, piDstY, iDstStride, iHorSpatRef, iVerSpatRef);
  xPredInterChromaBlk( C, Cd, pcPicYuvRef, 0, 0, uiNumOfSpritePoints, uiWarpAcc, iWidth, iHeight, piDstCb, piDstCr, iDstCStride, iHorSpatRef, iVerSpatRef);
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#define		round_toward_positive(x)	((Int) (((x) >= 0) ? (x) + 0.5 : floor((x) + 0.5)))

Int TComGlobalMotion::xSynthesisLuma(Int		i,
		    Int		j,
		    Int		*I_,
		    Int		*J_,
		    Int		C[],
		    Double 	Cd[],
           Int iHorSpatRef,
           Int iVerSpatRef,
           UInt uiNumOfSpritePoints)
{
    Int		I = i - iHorSpatRef;
    Int		J = j - iVerSpatRef;
    Double	temp, denom;

    switch (uiNumOfSpritePoints) {
      case 0:	/* Stationary transform */
	/* I' = si,	J' = sj
		C[2] = s
	*/
	*I_ = C[2] * i;
	*J_ = C[2] * j;
	break;
      case 1:	/* Translational transform */
	/* I' = i0' + sI,  J' = j0' + sJ
		C[0] = i0'
		C[1] = j0'
		C[2] = s
	*/
	*I_ = C[0] + C[2] * I;
	*J_ = C[1] + C[2] * J;
	break;
      case 2:	/* Isotropic transform */
	/* I' = (i0' + ((-r i0' + i1")I + ( r j0' - j1")J) // (W' r)
	   J' = (j0' + ((-r j0' + j1")I + (-r i0' - i1")J) // (W' r)
		C[0] = i0'
		C[1] = -r i0' + i1"
		C[2] =  r j0' - j1"
		C[3] = j0'
		C[4] = -r j0' + j1"
		C[5] = -r i0' - i1"
		C[6] = W' r
	*/
      case 3:	/* Affine transform */
	/* I' = (i0' + ((-r i0' + i1")H' I + (-r i0' + i2")W' J) // (W' H' r)
	   J' = (j0' + ((-r j0' + j1")H' I + (-r j0' + j2")W' J) // (W' H' r)
		C[0] = i0'
		C[1] = (-r i0' + i1")H'
		C[2] = (-r i0' + i2")W'
		C[3] = j0'
		C[4] = (-r j0' + j1")H'
		C[5] = (-r j0' + j2")W'
		C[6] = W' H' r
	*/
	temp = (Double) C[0] + ((Double) (C[1] * I + C[2] * J) / (Double) C[6]);
	*I_ = round_toward_positive(temp);
	temp = (Double) C[3] + ((Double) (C[4] * I + C[5] * J) / (Double) C[6]);
	*J_ = round_toward_positive(temp);
	break;
      case 4:	/* Perspective transform */
	/* I' = (a i + b j + c) // (g i + h j + D W H)
	   J' = (d i + e j + f) // (g i + h j + D W H)
		Cd[0] = a,	Cd[1] = b,	Cd[2] = c
		Cd[3] = d,	Cd[4] = e,	Cd[5] = f
		Cd[6] = g,	Cd[7] = h,	Cd[8] = D W H
	*/
	denom = Cd[6] * I + Cd[7] * J + Cd[8];
	/* Error check */
	if (denom == 0.0) {
     assert(0);
	}

	temp = (Cd[0] * I + Cd[1] * J + Cd[2]) / denom;
	*I_ = round_toward_positive(temp);
	temp = (Cd[3] * I + Cd[4] * J + Cd[5]) / denom;
	*J_ = round_toward_positive(temp);
	break;
      default:
     assert(0);
  }

    return 1;
}

Int TComGlobalMotion::xSynthesisChroma(Int		ic,
		    Int		jc,
		    Int		*Ic_,
		    Int		*Jc_,
		    Int		C[],
		    Double	Cd[],
           Int iHorSpatRef,
           Int iVerSpatRef,
           UInt uiNumOfSpritePoints)
{
    Int		Ic = 4 * ic - 2 * iHorSpatRef + 1;
    Int		Jc = 4 * jc - 2 * iVerSpatRef + 1;
    Double	temp, denom;

    switch (uiNumOfSpritePoints) {
      case 0:	/* Stationary transform */
	/* Ic' = s ic,	J' = s jc
		C[2] = s
		C[3] = C[4] = 0
	*/
	*Ic_ = C[3] + C[2] * ic;
	*Jc_ = C[4] + C[2] * jc;
	break;
      case 1:	/* Translational transform */
	/* Ic' = (((i0' >> 1) | (i0' & 1)) + s (ic - i0 / 2),
	   Jc' = (((j0' >> 1) | (j0' & 1)) + s (jc - j0 / 2),
		C[2] = s
		C[3] = ((i0' >> 1) | (i0' & 1))
		C[4] = ((j0' >> 1) | (j0' & 1))
	*/
	*Ic_ = C[3] + C[2] * (ic - iHorSpatRef / 2);
	*Jc_ = C[4] + C[2] * (jc - iVerSpatRef   / 2);
	break;
      case 2:	/* Isotropic transform */
	/* Ic' = (((-r i0' + i1")Ic + ( r j0' - j1")Jc + 2 W' r i0' - 16 W') // (4 W' r)
	   Jc' = (((-r j0' + j1")Ic + (-r i0' - i1")Jc + 2 W' r j0' - 16 W') // (4 W' r)
		C[1] = -r i0' + i1"
		C[2] =  r j0' - j1"
		C[4] = -r j0' + j1"
		C[5] = -r i0' - i1"
		C[7] = 4 W' r
		C[8] = 2 W' r i0' - 16 W'
		C[9] = 2 W' r j0' - 16 W'
	*/
      case 3:	/* Affine transform */
	/* I' = (((-r i0' + i1")H' Ic + (-r i0' + i2")W' Jc + 2 W' H' r i0' - 16 W' H') // (4 W' H' r)
	   J' = (((-r j0' + j1")H' Ic + (-r j0' + j2")W' Jc + 2 W' H' r j0' - 16 W' H') // (4 W' H' r)
		C[1] = (-r i0' + i1")H'
		C[2] = (-r i0' + i2")W'
		C[4] = (-r j0' + j1")H'
		C[5] = (-r j0' + j2")W'
		C[7] = 4 W' H' r
		C[8] = 2 W' H' r i0' - 16 W' H'
		C[9] = 2 W' H' r j0' - 16 W' H'
	*/
	temp = ((Double) C[1]*Ic + (Double) C[2]*Jc + (Double) C[8]) / (Double) C[7];
	*Ic_ = round_toward_positive(temp);
	temp = ((Double) C[4]*Ic + (Double) C[5]*Jc + (Double) C[9]) / (Double) C[7];
	*Jc_ = round_toward_positive(temp);
	break;
      case 4:	/* Perspective transform */
	/* I' = (2 a Ic + b Jc + 4 c - (g Ic + h Jc + 2 D W H)s) // (4 g Ic + 4 h Jc + 8 D W H)
	   J' = (2 d Ic + e Jc + 4 f - (g Ic + h Jc + 2 D W H)s) // (4 g Ic + 4 h Jc + 8 D W H)
		Cd[0] = a,	Cd[1] = b,	Cd[2] = c
		Cd[3] = d,	Cd[4] = e,	Cd[5] = f
		Cd[6] = g,	Cd[7] = h,	Cd[8] = D W H,	Cd[9] = s
	*/
	denom = 4*Cd[6]*Ic + 4*Cd[7]*Jc + 8*Cd[8];
	/* Error check */
	if (denom == 0.0) {
        assert(0);
	}

	temp = (2*Cd[0]*Ic + 2*Cd[1]*Jc + 4*Cd[2] - (Cd[6]*Ic + Cd[7]*Jc + 2*Cd[8])*Cd[9]) / denom;
	*Ic_ = round_toward_positive(temp);
	temp = (2*Cd[3]*Ic + 2*Cd[4]*Jc + 4*Cd[5] - (Cd[6]*Ic + Cd[7]*Jc + 2*Cd[8])*Cd[9]) / denom;
	*Jc_ = round_toward_positive(temp);
	break;
      default:
     assert(0);
  }

    return 1;
}

#if GMC_SIX_TAP
Void TComGlobalMotion::xFilterQuarterPelsLuma (Pel* piSrc, Int iSrcStride, Int& P00, Int& P01, Int& P10, Int& P11, Int px, Int py)
{
  Pel asTemp[6*6];
  Pel* psTemp = asTemp;
  Int iTempX, iTempY, iTemp[6];
  Int iTempStride = 6;
  Int idx;
  Int x, y;
  Int Pb, Ph, Pm, Ps, Pj;


  for (y = -2; y < 4; y++)
  {
    for (x = -2; x < 4; x++)
    {
      psTemp[(y+2)*6 + (x+2)] = piSrc[x+y*iSrcStride];
    }
  }

  iTempStride = 6;

  Pel* piTmpSrc = psTemp + 2 * iTempStride + 2;
  iTempX  = piTmpSrc[  0 ];
  iTempX += piTmpSrc[  1 ];
  iTempX  = iTempX << 2;
  iTempX -= piTmpSrc[ -1 ];
  iTempX -= piTmpSrc[  2 ];
  iTempX += iTempX << 2;
  iTempX += piTmpSrc[ -2 ];
  iTempX += piTmpSrc[  3 ];
  Pb = Clip( (iTempX + 16) / 32 );

  iTempY  = piTmpSrc[ 0*iTempStride];
  iTempY += piTmpSrc[ 1*iTempStride];
  iTempY  = iTempY << 2;
  iTempY -= piTmpSrc[-1*iTempStride];
  iTempY -= piTmpSrc[ 2*iTempStride];
  iTempY += iTempY << 2;
  iTempY += piTmpSrc[-2*iTempStride];
  iTempY += piTmpSrc[ 3*iTempStride];
  Ph = Clip( (iTempY + 16) / 32 );

  piTmpSrc = psTemp + 3 * iTempStride + 2;
  iTempX  = piTmpSrc[  0 ];
  iTempX += piTmpSrc[  1 ];
  iTempX  = iTempX << 2;
  iTempX -= piTmpSrc[ -1 ];
  iTempX -= piTmpSrc[  2 ];
  iTempX += iTempX << 2;
  iTempX += piTmpSrc[ -2 ];
  iTempX += piTmpSrc[  3 ];
  Ps = Clip( (iTempX + 16) / 32 );

  piTmpSrc = psTemp + 2 * iTempStride + 3;
  iTempY  = piTmpSrc[ 0*iTempStride];
  iTempY += piTmpSrc[ 1*iTempStride];
  iTempY  = iTempY << 2;
  iTempY -= piTmpSrc[-1*iTempStride];
  iTempY -= piTmpSrc[ 2*iTempStride];
  iTempY += iTempY << 2;
  iTempY += piTmpSrc[-2*iTempStride];
  iTempY += piTmpSrc[ 3*iTempStride];
  Pm = Clip( (iTempY + 16) / 32 );

  for (idx = 0; idx < 6; idx++)
  {
    piTmpSrc = psTemp + idx * iTempStride + 2;
    iTemp[idx]  = piTmpSrc[  0 ];
    iTemp[idx] += piTmpSrc[  1 ];
    iTemp[idx]  = iTemp[idx] << 2;
    iTemp[idx] -= piTmpSrc[ -1 ];
    iTemp[idx] -= piTmpSrc[  2 ];
    iTemp[idx] += iTemp[idx] << 2;
    iTemp[idx] += piTmpSrc[ -2 ];
    iTemp[idx] += piTmpSrc[  3 ];
  }
  iTempY  = iTemp[ 2 ];
  iTempY += iTemp[ 3 ];
  iTempY  = iTempY << 2;
  iTempY -= iTemp[ 1 ];
  iTempY -= iTemp[ 4 ];
  iTempY += iTempY << 2;
  iTempY += iTemp[ 0 ];
  iTempY += iTemp[ 5 ];
  Pj = Clip( (iTempY + 512) / 1024 );

  piTmpSrc = psTemp + 2 * iTempStride + 2;

  if ( py == 0 )
  {
    if ( px == 0 )
    {
      P00 = piTmpSrc[ 0 ];
      P01 = (Pb + piTmpSrc[ 0 ] + 1) / 2;
      P10 = (Ph + piTmpSrc[ 0 ] + 1) / 2;
      P11 = (Pb + Ph + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (Pb + piTmpSrc[ 0 ] + 1) / 2;
      P01 = Pb;
      P10 = (Pb + Ph + 1) / 2;
      P11 = (Pb + Pj + 1) / 2;
    }
    else if ( px == 2 )
    {
      P00 = Pb;
      P01 = (Pb + piTmpSrc[ 1 ] + 1) / 2;
      P10 = (Pb + Pj + 1) / 2;
      P11 = (Pb + Pm + 1) / 2;
    }
    else // px == 3
    {
      P00 = (Pb + piTmpSrc[ 1 ] + 1) / 2;
      P01 = piTmpSrc[ 1 ];
      P10 = (Pb + Pm + 1) / 2;
      P11 = (Pm + piTmpSrc[ 1 ] + 1) / 2;
    }
  }
  else if ( py == 1 )
  {
    if ( px == 0 )
    {
      P00 = (Ph + piTmpSrc[ 0 ] + 1) / 2;
      P01 = (Pb + Ph + 1) / 2;
      P10 = Ph;
      P11 = (Ph + Pj + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (Pb + Ph + 1) / 2;
      P01 = (Pb + Pj + 1) / 2;
      P10 = (Ph + Pj + 1) / 2;
      P11 = Pj;
    }
    else if ( px == 2 )
    {
      P00 = (Pb + Pj + 1) / 2;
      P01 = (Pb + Pm + 1) / 2;
      P10 = Pj;
      P11 = (Pj + Pm + 1) / 2;
    }
    else // px == 3
    {
      P00 = (Pb + Pm + 1) / 2;
      P01 = (Pm + piTmpSrc[ 1 ] + 1) / 2;
      P10 = (Pj + Pm + 1) / 2;
      P11 = Pm;
    }
  }
  else if ( py == 2 )
  {
    if ( px == 0 )
    {
      P00 = Ph;
      P01 = (Ph + Pj + 1) / 2;
      P10 = (Ph + piTmpSrc[ iTempStride ] + 1) / 2;
      P11 = (Ph + Ps + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (Ph + Pj + 1) / 2;
      P01 = Pj;
      P10 = (Ph + Ps + 1) / 2;
      P11 = (Pj + Ps + 1) / 2;
    }
    else if ( px == 2 )
    {
      P00 = Pj;
      P01 = (Pj + Pm + 1) / 2;
      P10 = (Pj + Ps + 1) / 2;
      P11 = (Pm + Ps + 1) / 2;
    }
    else // px == 3
    {
      P00 = (Pj + Pm + 1) / 2;
      P01 = Pm;
      P10 = (Ps + Pm + 1) / 2;
      P11 = (Pm + piTmpSrc[iTempStride + 1] + 1) / 2;
    }
  }
  else // py == 3
  {
    if ( px == 0 )
    {
      P00 = (Ph + piTmpSrc[ iTempStride ] + 1) / 2;
      P01 = (Ph + Ps + 1) / 2;
      P10 = piTmpSrc[ iTempStride ];
      P11 = (Ps + piTmpSrc[ iTempStride ] + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (Ph + Ps + 1) / 2;
      P01 = (Pj + Ps + 1) / 2;
      P10 = (Ps + piTmpSrc[ iTempStride ] + 1) / 2;
      P11 = Ps;
    }
    else if ( px == 2 )
    {
      P00 = (Pj + Ps + 1) / 2;
      P01 = (Pm + Ps + 1) / 2;
      P10 = Ps;
      P11 = (Ps + piTmpSrc[ iTempStride + 1 ] + 1) / 2;
    }
    else // px == 3
    {
      P00 = (Ps + Pm + 1) / 2;
      P01 = (Pm + piTmpSrc[ iTempStride + 1 ] + 1) / 2;
      P10 = (Ps + piTmpSrc[ iTempStride + 1 ] + 1) / 2;
      P11 = piTmpSrc[iTempStride + 1];
    }
  }
}

Void TComGlobalMotion::xFilterQuarterPelsChroma (Pel* piSrc, Int iSrcStride, Int& P00, Int& P01, Int& P10, Int& P11, Int px, Int py)
{
  if ( py == 0 )
  {
    if ( px == 0 )
    {
      P00 = piSrc[ 0 ];
      P01 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P01 = piSrc[ 1 ];
      P10 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else if ( px == 2)
    {
      P00 = piSrc[ 0 ];
      P01 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else // px == 3
    {
      P00 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P01 = piSrc[ 1 ];
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
  }
  else if ( py == 1 )
  {
    if ( px == 0 )
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P10 = piSrc[ iSrcStride ];
      P11 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = piSrc[ iSrcStride+1 ];
    }
    else if ( px == 2)
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = piSrc[ iSrcStride ];
      P11 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else // px == 3
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = piSrc[ iSrcStride+1 ];
    }
  }
  else if ( py == 2 )
  {
    if ( px == 0 )
    {
      P00 = piSrc[ 0 ];
      P01 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P01 = piSrc[ 1 ];
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else if ( px == 2)
    {
      P00 = piSrc[ 0 ];
      P01 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P10 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
    }
    else // px == 3
    {
      P00 = (piSrc[ 0 ] + piSrc[ 1 ] + 1) / 2;
      P01 = piSrc[ 1 ];
      P10 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P11 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
  }
  else // py == 3
  {
    if ( px == 0 )
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = piSrc[ iSrcStride ];
      P11 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else if ( px == 1 )
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = piSrc[ iSrcStride+1 ];
    }
    else if ( px == 2)
    {
      P00 = (piSrc[ 0 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P10 = piSrc[ iSrcStride ];
      P11 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
    }
    else // px == 3
    {
      P00 = (piSrc[ 1 ] + piSrc[ iSrcStride ] + 1) / 2;
      P01 = (piSrc[ 1 ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P10 = (piSrc[ iSrcStride ] + piSrc[ iSrcStride+1 ] + 1) / 2;
      P11 = piSrc[ iSrcStride+1 ];
    }
  }
}
#endif

Void TComGlobalMotion::xPredInterLumaBlk( TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstY, Int iDstStride, Int iHorSpatRef, Int iVerSpatRef)
{
  Int		C[10];		/* for warping caluculation */
  Double	Cd[10];		/* for warping caluculation */

  /* setting warping Consts */
  xSetWarpingParam(C, Cd, uiNumOfSpritePoints, pcRefPointCoord, pcTraj, uiWarpAcc, iHorSpatRef, iVerSpatRef );
  xPredInterLumaBlk(C, Cd, pcPicYuvRef, uiPelX, uiPelY, uiNumOfSpritePoints, uiWarpAcc, iWidth, iHeight, piDstY, iDstStride, iHorSpatRef, iVerSpatRef);
}

Void TComGlobalMotion::xPredInterChromaBlk( TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstCb, Pel* piDstCr, Int iDstStride, Int iHorSpatRef, Int iVerSpatRef)
{
  Int		C[10];		/* for warping caluculation */
  Double	Cd[10];		/* for warping caluculation */

  /* setting warping Consts */
  xSetWarpingParam(C, Cd, uiNumOfSpritePoints, pcRefPointCoord, pcTraj, uiWarpAcc, iHorSpatRef, iVerSpatRef );
  xPredInterChromaBlk(C, Cd, pcPicYuvRef, uiPelX, uiPelY, uiNumOfSpritePoints, uiWarpAcc, iWidth, iHeight, piDstCb, piDstCr, iDstStride, iHorSpatRef, iVerSpatRef);
}

Void TComGlobalMotion::xPredInterLumaBlk( Int C[], Double Cd[], TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints, UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstY, Int iDstStride, Int iHorSpatRef, Int iVerSpatRef )
{
  Int		i, j;
  Int		col, row;
  Int		I_, J_;
  Int		px, py;
  Int		x0, y0;
  Int		s			= 2 << uiWarpAcc;
  Int		br_x			= iHorSpatRef;
  Int		br_y			= iVerSpatRef;
  Pel* piSrc	;
  Int iSrcStride = pcPicYuvRef->getStride();
  Int		sizex			= pcPicYuvRef->getWidth();
  Int		sizey			= pcPicYuvRef->getHeight();
  Int		margin			= pcPicYuvRef->getLumaMargin();
  Int		RoundType		= 0; //GetVopRoundingType(comp);

  for (j = 0, row = uiPelY; j < iHeight; j++, row++)
	{
    for (i = 0, col = uiPelX; i < iWidth; i++, col++)
		{
      if (! xSynthesisLuma(col+br_x, row+br_y, &I_, &J_, C, Cd,
    		      br_x, br_y, uiNumOfSpritePoints)) {
        /* warping Error check */
        assert(0);
      }

      x0 = xRound(floor((Double) I_ / s));
      y0 = xRound(floor((Double) J_ / s));

      x0 -= iHorSpatRef;
      y0 -= iVerSpatRef;

      #if GMC_SIX_TAP
      x0 = Clip3(-margin+3, sizex-1+margin-3-1, x0);
      y0 = Clip3(-margin+3, sizey-1+margin-3-1, y0);
      #else
      x0 = Clip3(-margin, sizex-1+margin-1, x0);
      y0 = Clip3(-margin, sizey-1+margin-1, y0);
      #endif

      piSrc = pcPicYuvRef->getLumaAddr()+x0+y0*iSrcStride;

      #if GMC_SIX_TAP
      Int		P00, P01, P10, P11, P;

      px = (uiWarpAcc < 1? (I_ << 1) & 3 : (I_ >> (uiWarpAcc-1)) & 3);
      py = (uiWarpAcc < 1? (J_ << 1) & 3 : (J_ >> (uiWarpAcc-1)) & 3);
      xFilterQuarterPelsLuma(piSrc, iSrcStride, P00, P01, P10, P11, px, py);

      if (uiWarpAcc > 1)
      {
        Int pxx, pyy;
        Int ss;

        // bilinear interpolation between POO, P01, P10, P11
        pxx = I_ & 3;
        pyy = J_ & 3;

        ss = s >> 2;

        P = ((ss - pyy) * ((ss - pxx) * P00 + pxx * P01) + pyy * ((ss - pxx) * P10 + pxx * P11) + (ss * ss / 2) - RoundType) / (ss * ss);
      }
      else
      {
        P = P00;
      }

      piDstY[col] = Clip(P);
			#else
      px = I_ & (s - 1);
      py = J_ & (s - 1);

      xFilterBilinearRect(piSrc, iSrcStride, 1, 1, 1, piDstY + col, iDstStride, 1, px, py, s);
      piDstY[col] = Clip(piDstY[col]);
      #endif
    }
    piDstY += iDstStride;
  }
}

Void TComGlobalMotion::xPredInterChromaBlk      ( Int C[], Double Cd[], TComPicYuv* pcPicYuvRef, UInt uiPelX, UInt uiPelY, UInt uiNumOfSpritePoints, UInt uiWarpAcc, Int iWidth, Int iHeight, Pel* piDstCb, Pel* piDstCr, Int iDstStride, Int iHorSpatRef, Int iVerSpatRef)
{
  Int		i, j;
  Int		col, row;
  Int		Ic_, Jc_;
  Int		px, py;
  Int		x0, y0;
  Int		s			= 2 << uiWarpAcc;
  Pel* piSrcCb;
  Pel* piSrcCr;
  Int iSrcStride = pcPicYuvRef->getCStride();
  Int		sizex			= pcPicYuvRef->getWidth() >> 1;
  Int		sizey			= pcPicYuvRef->getHeight() >> 1;
  Int		margin			= pcPicYuvRef->getChromaMargin();
  Int		RoundType		= 0; //GetVopRoundingType(comp);

  uiPelX >>= 1;
  uiPelY >>= 1;
  iHeight >>= 1;
  iWidth >>= 1;
  iHorSpatRef >>= 1;
  iVerSpatRef >>= 1;

  Int		br_x			= iHorSpatRef;
  Int		br_y			= iVerSpatRef;

  for (j = 0, row = uiPelY; j < iHeight; j++, row++)
	{
    for (i = 0, col = uiPelX; i < iWidth; i++, col++)
		{
      if (! xSynthesisChroma(col+br_x, row+br_y, &Ic_, &Jc_, C, Cd,
    		      br_x*2, br_y*2, uiNumOfSpritePoints)) {
        /* warping Error check */
        assert(0);
      }

      x0 = xRound(floor((Double) Ic_ / s));
      y0 = xRound(floor((Double) Jc_ / s));

      x0 -= iHorSpatRef;
      y0 -= iVerSpatRef;

      #if GMC_SIX_TAP
      x0 = Clip3(-margin+3, sizex-1+margin-3-1, x0);
      y0 = Clip3(-margin+3, sizey-1+margin-3-1, y0);
      #else
      x0 = Clip3(-margin, sizex-1+margin-1, x0);
      y0 = Clip3(-margin, sizey-1+margin-1, y0);
      #endif

      piSrcCb = pcPicYuvRef->getCbAddr()+x0+y0*iSrcStride;
      piSrcCr = pcPicYuvRef->getCrAddr()+x0+y0*iSrcStride;

      #if GMC_SIX_TAP
      Int		P00, P01, P10, P11, P;

      px = (uiWarpAcc < 1? (Ic_ << 1) & 3 : (Ic_ >> (uiWarpAcc-1)) & 3);
      py = (uiWarpAcc < 1? (Jc_ << 1) & 3 : (Jc_ >> (uiWarpAcc-1)) & 3);

      Int pxx, pyy;
      Int ss;

      if (uiWarpAcc > 1)
      {
        pxx = Ic_ & 3;
        pyy = Jc_ & 3;

        ss = s >> 2;
      }

      xFilterQuarterPelsChroma(piSrcCb, iSrcStride, P00, P01, P10, P11, px, py);

      if (uiWarpAcc > 1)
      {
        // bilinear interpolation between POO, P01, P10, P11
        P = ((ss - pyy) * ((ss - pxx) * P00 + pxx * P01) + pyy * ((ss - pxx) * P10 + pxx * P11) + (ss * ss / 2) - RoundType) / (ss * ss);
      }
      else
      {
        P = P00;
      }

      piDstCb[col] = Clip(P);

      xFilterQuarterPelsChroma(piSrcCr, iSrcStride, P00, P01, P10, P11, px, py);

      if (uiWarpAcc > 1)
      {
        // bilinear interpolation between POO, P01, P10, P11
        P = ((ss - pyy) * ((ss - pxx) * P00 + pxx * P01) + pyy * ((ss - pxx) * P10 + pxx * P11) + (ss * ss / 2) - RoundType) / (ss * ss);
      }
      else
      {
        P = P00;
      }

      piDstCr[col] = Clip(P);
	  #else
      px = Ic_ - (xRound(floor((Double) Ic_ / s)) * s);
      py = Jc_ - (xRound(floor((Double) Jc_ / s)) * s);

      xFilterBilinearRect(piSrcCb, iSrcStride, 1, 1, 1, piDstCb + col, iDstStride, 1, px, py, s);
      piDstCb[col] = Clip(piDstCb[col]);

      xFilterBilinearRect(piSrcCr, iSrcStride, 1, 1, 1, piDstCr + col, iDstStride, 1, px, py, s);
      piDstCr[col] = Clip(piDstCr[col]);
      #endif
    }
    piDstCb += iDstStride;
    piDstCr += iDstStride;
  }
}

Void TComGlobalMotion::xSetWarpingParam(Int C[], Double	Cd[], UInt uiNumOfSpritePoints, TrajPoint* pcRefPointCoord, TrajPoint* pcTraj, UInt uiWarpAcc, Int iHorSpatRef, Int iVerSpatRef )
{
  Int		s	= 2 << uiWarpAcc;
  Int		r	= 16 / s;
  Int		W, H;
  Int		W_, H_;
  Int		i_[4], j_[4];
  Int		i__[4], j__[4];
  Int		x[4], y[4];
  Int		Idx;

  if (uiNumOfSpritePoints == 0)
  {
    C[0] = C[1] = 0;
    C[2] = s;
    C[3] = C[4] = 0;

    return;
  }

  W = pcRefPointCoord[1].x - pcRefPointCoord[0].x;
  H = pcRefPointCoord[2].y - pcRefPointCoord[0].y;

  /* W' */
  for (W_ = 1; W_ < W; ) {
  W_ <<= 1;
  }

  /* H' */
  for (H_ = 1; H_ < H; ) {
  H_ <<= 1;
  }

  /* x, y */
  x[0] = iHorSpatRef;	y[0] = iVerSpatRef;
  x[1] = x[0] + W;			y[1] = y[0];
  x[2] = x[0];			y[2] = y[0] + H;
  x[3] = x[1];			y[3] = y[2];
  /* ir'(n), jr'(n) */
  for (Idx = 0; Idx < uiNumOfSpritePoints; Idx++)
  {
    i_[Idx] = (s/GMC_AFF_ACC) * (GMC_AFF_ACC * x[Idx] + pcTraj[Idx].x);
    j_[Idx] = (s/GMC_AFF_ACC) * (GMC_AFF_ACC * y[Idx] + pcTraj[Idx].y);
  }

  /* ir'', jr'' */
  if (uiNumOfSpritePoints >= 2)
  {
    i__[1] = 16 * (x[0] + W_) +
     xRound((Double) ((W - W_) * (r * i_[0] - 16 * x[0]) +
    	    W_      * (r * i_[1] - 16 * x[1])) / (Double) W);
    j__[1] = 16 * y[0] +
     xRound((Double) ((W - W_) * (r * j_[0] - 16 * y[0]) +
    	    W_      * (r * j_[1] - 16 * y[1])) / (Double) W);

    if (uiNumOfSpritePoints >= 3)
    {
       i__[2] = 16 * x[0] +
      xRound((Double) ((H - H_) * (r * i_[0] - 16 * x[0]) +
      		H_      * (r * i_[2] - 16 * x[2])) / (Double) H);
       j__[2] = 16 * (y[0] + H_) +
      xRound((Double) ((H - H_) * (r * j_[0] - 16 * y[0]) +
      		H_      * (r * j_[2] - 16 * y[2])) / (Double) H);
    }
  }

  switch (uiNumOfSpritePoints) {
   case 0:	/* Stationary transform */
  C[0] = C[1] = 0;
  C[2] = s;
  C[3] = C[4] = 0;
   case 1:	/* Translational transform */
  C[0] = i_[0];
  C[1] = j_[0];
  C[2] = s;
  C[3] = ((i_[0] >> 1) | (i_[0] & 1));
  C[4] = ((j_[0] >> 1) | (j_[0] & 1));
  break;
   case 2:	/* Isotropic transform */
  C[0] = i_[0];
  C[1] = (-r * i_[0] + i__[1]);
  C[2] = ( r * j_[0] - j__[1]);
  C[3] = j_[0];
  C[4] = (-r * j_[0] + j__[1]);
  C[5] = (-r * i_[0] + i__[1]);
  C[6] = W_ * r;
  C[7] = 4 * W_ * r;
  C[8] = (2 * W_ * r * i_[0]) - (16 * W_);
  C[9] = (2 * W_ * r * j_[0]) - (16 * W_);
  break;
   case 3:	/* Affine transform */
  /*
  C[0] = i_[0];
  C[1] = (-r * i_[0] + i__[1]) * H_;
  C[2] = (-r * i_[0] + i__[2]) * W_;
  C[3] = j_[0];
  C[4] = (-r * j_[0] + j__[1]) * H_;
  C[5] = (-r * j_[0] + j__[2]) * W_;
  C[6] = W_ * H_ * r;
  C[7] = 4 * W_ * H_ * r;
  C[8] = (2 * W_ * H_ * r * i_[0]) - (16 * W_ * H_);
  C[9] = (2 * W_ * H_ * r * j_[0]) - (16 * W_ * H_);
  */

  C[0] = i_[0];
  C[3] = j_[0];
  if (W_ > H_) {
   C[1] = (-r * i_[0] + i__[1]);
   C[2] = (-r * i_[0] + i__[2]) * (W_ / H_);
   C[4] = (-r * j_[0] + j__[1]);
   C[5] = (-r * j_[0] + j__[2]) * (W_ / H_);
   C[6] = W_ * r;
   C[7] = 4 * W_ * r;
   C[8] = (2 * W_ * r * i_[0]) - (16 * W_ );
   C[9] = (2 * W_ * r * j_[0]) - (16 * W_ );
  } else {
   C[1] = (-r * i_[0] + i__[1]) * (H_ / W_);
   C[2] = (-r * i_[0] + i__[2]);
   C[4] = (-r * j_[0] + j__[1]) * (H_ / W_);
   C[5] = (-r * j_[0] + j__[2]);
   C[6] = H_ * r;
   C[7] = 4 * H_ * r;
   C[8] = (2 * H_ * r * i_[0]) - (16 * H_);
   C[9] = (2 * H_ * r * j_[0]) - (16 * H_);
  }

  break;
   case 4:	/* Perspective transform */
  Cd[6] = ((Double) (i_[0] - i_[1] - i_[2] + i_[3]) * (j_[2] - j_[3]) -
   (Double) (i_[2] - i_[3]) * (j_[0] - j_[1] - j_[2] + j_[3])) * H;
  Cd[7] = ((Double) (i_[1] - i_[3]) * (j_[0] - j_[1] - j_[2] + j_[3]) -
   (Double) (i_[0] - i_[1] - i_[2] + i_[3]) * (j_[1] - j_[3])) * W;
  Cd[8] = ((Double) (i_[1] - i_[3]) * (j_[2] - j_[3]) -
   (Double) (i_[2] - i_[3]) * (j_[1] - j_[3]));
  Cd[0] = Cd[8] * (i_[1] - i_[0]) * H + Cd[6] * i_[1];
  Cd[1] = Cd[8] * (i_[2] - i_[0]) * W + Cd[7] * i_[2];
  Cd[2] = Cd[8] * i_[0] * W * H;
  Cd[3] = Cd[8] * (j_[1] - j_[0]) * H + Cd[6] * j_[1];
  Cd[4] = Cd[8] * (j_[2] - j_[0]) * W + Cd[7] * j_[2];
  Cd[5] = Cd[8] * j_[0] * W * H;
  Cd[8] *= (W * H);
  Cd[9] = s;
  break;
   default:
   assert(0);
  }
}

// 3-tap filter with coefficients [1/4 1/2 1/4]
Void TComGlobalMotion::xThreeTapFilter(Pel *Low, Pel *Hight, Int width, Int height)
{
  Int i, j, hwidth = width/2, wwidth = width*2, width2 = width+2;
  Pel *lt, *ct, *rt,
    *lc, *cc, *rc,
    *lb, *cb, *rb, *p;

  p=Low; lt=Hight; ct=Hight; rt=Hight+1;
  lc=Hight; cc=Hight; rc=Hight+1;
  lb=lc+width; cb=cc+width; rb=rc+width;
  *p = (*lt + (*ct<<1) + *rt +
        (*lc<<1) + (*cc<<2) + (*rc<<1) +
        *lb + (*cb<<1) + *rb + 8) >> 4;

  p=Low+1; lt=Hight+1; ct=Hight+2; rt=Hight+3;
  lc=Hight+1; cc=Hight+2; rc=Hight+3;
  lb=lc+width; cb=cc+width; rb=rc+width;
  for(i=1; i<width/2; i++, p++, lt+=2, ct+=2, rt+=2,
        lc+=2, cc+=2, rc+=2,
        lb+=2, cb+=2, rb+=2)
    *p = (*lt + (*ct<<1) + *rt +
          (*lc<<1) + (*cc<<2) + (*rc<<1) +
          *lb + (*cb<<1) + *rb + 8) >> 4;

  p=Low+hwidth; lt=Hight+width; ct=Hight+width; rt=Hight+width+1;
  lc=lt+width; cc=ct+width; rc=rt+width;
  lb=lc+width; cb=cc+width; rb=rc+width;
  for(i=1; i<height/2; i++, p+=hwidth, lt+=wwidth, ct+=wwidth, rt+=wwidth,
        lc+=wwidth, cc+=wwidth, rc+=wwidth,
        lb+=wwidth, cb+=wwidth, rb+=wwidth)
    *p = (*lt + (*ct<<1) + *rt +
          (*lc<<1) + (*cc<<2) + (*rc<<1) +
          *lb + (*cb<<1) + *rb + 8) >> 4;

  p=Low+hwidth+1; lt=Hight+1; ct=Hight+2; rt=Hight+3;
  lc=lt+width; cc=ct+width; rc=rt+width;
  lb=lc+width; cb=cc+width; rb=rc+width;
  for(j=1; j<height/2; j++, p++, lt+=width2, ct+=width2, rt+=width2,
        lc+=width2, cc+=width2, rc+=width2,
        lb+=width2, cb+=width2, rb+=width2)
    for(i=1; i<hwidth; i++, p++, lt+=2, ct+=2, rt+=2,
          lc+=2, cc+=2, rc+=2,
          lb+=2, cb+=2, rb+=2)
      *p = (*lt + (*ct<<1) + *rt +
            (*lc<<1) + (*cc<<2) + (*rc<<1) +
            *lb + (*cb<<1) + *rb + 8) >> 4;

}

Void TComGlobalMotion::xAffineGME(TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef, Int iVerSpatRef)
{
  Pel* ref, *curr;
  Pel *ref_P[3], *curr_P[3];
  Pel *ref_work, *curr_work;
  Int curr_width, curr_height, curr_left, curr_top;
  Int ref_width, ref_height, ref_left, ref_top;

  Int i, j, best_locationx, best_locationy;
  Int curr_pel, offset_x, offset_y, ref_pel;
  Int ref_pel1, ref_pel2, ref_pel3;
  Double dm[6], db[6], dA[36], M[6];
  Double dE2 = 0;
  Int stop = 0, level = 2, ite = 0;
  Int x, y, x1, y1;
  Double dx1, dy1, dx, dy;
  Double dt, du, dk, d1mk, dl, d1ml,
    dI1, de, dI1dx, dI1dy;
  Double ddedm[6];
  Double I1x1y1[4];
  Int* error_histgram = new Int[g_uiIBDI_MAX+1];
  Int threshold_T=0, check = 1;

//  pcRefFrame->extendPicBorder();

  ref = pcRefFrame->getLumaAddr();
  curr = pcCurrFrame->getLumaAddr();

  ref_left = iHorSpatRef;
  ref_top = iVerSpatRef;
  ref_width = pcRefFrame->getWidth() >> 2;
  ref_height = pcRefFrame->getHeight() >> 2;

  curr_left = iHorSpatRef;
  curr_top = iVerSpatRef;
  curr_width = pcCurrFrame->getWidth() >> 2;
  curr_height = pcCurrFrame->getHeight() >> 2;

  ref_P[0] = (Pel *)malloc(ref_width*ref_height*16*sizeof(Pel));
  curr_P[0] = (Pel *)malloc(curr_width*curr_width*16*sizeof(Pel));
  ref_P[1] = (Pel *)malloc(ref_width*ref_height*4*sizeof(Pel));
  curr_P[1] = (Pel *)malloc(curr_width*curr_height*4*sizeof(Pel));
  ref_P[2] = (Pel *)malloc(ref_width*ref_height*sizeof(Pel));
  curr_P[2] = (Pel *)malloc(curr_width*curr_height*sizeof(Pel));

  Int iStride = pcRefFrame->getStride();
  Pel* piTmp = ref_P[0];

  for (j = 0; j < pcRefFrame->getHeight(); j++)
  for (i = 0; i < pcRefFrame->getWidth(); i++)
  {
    piTmp[i+j*ref_width*4] = ref[i+j*iStride];
  }

  iStride = pcCurrFrame->getStride();
  piTmp = curr_P[0];

  for (j = 0; j < pcCurrFrame->getHeight(); j++)
  for (i = 0; i < pcCurrFrame->getWidth(); i++)
  {
    piTmp[i+j*curr_width*4] = curr[i+j*iStride];
  }

  xThreeTapFilter(ref_P[1], ref_P[0], ref_width*4, ref_height*4);
  xThreeTapFilter(curr_P[1], curr_P[0], curr_width*4, curr_height*4);
  xThreeTapFilter(ref_P[2], ref_P[1], ref_width*2, ref_height*2);
  xThreeTapFilter(curr_P[2], curr_P[1], curr_width*2, curr_height*2);

  for(i = 0; i < 6; i++)
    dm[i] = 0;
  for(i = 0; i < 6; i++)
    M[i] = 0;
  M[0]=1.0;
  M[4]=1.0;

  ref_work = ref_P[2];
  curr_work = curr_P[2];

  best_locationx = 0;
  best_locationy = 0;
/* for the absolute coordinate system */
  offset_x = curr_left - ref_left;
  offset_y = curr_top - ref_top;

  threshold_T=xModifiedThreeStepSearch(ref_work, curr_work,
                                      curr_width, curr_height, ref_width, ref_height,
                                      offset_x, offset_y,
                                      &best_locationx, &best_locationy);

  M[2] = (Double)best_locationx;
  M[5] = (Double)best_locationy;

  for(level = 2; level >= 0; level--)
  {
    ref_work = ref_P[level];
    curr_work = curr_P[level];

    for(i=0;i<g_uiIBDI_MAX+1;i++)
      error_histgram[i]=0;

    for(ite = 0; ite < 32; ite++)
    {
      dE2 = 0.;
      stop = 0;
      curr_pel = 0;

      for(j = 0; j < 36; j++)
        dA[j] = 0;
      for(i = 0; i < 6; i++)
        db[i] = 0;

      for(y=0; y<curr_height; y++)
      {
        dy = (Double)(y + curr_top);
        for(x=0; x<curr_width; x++, curr_pel++)
        {
          dx = (Double)(x + curr_left);

          dt = M[0] * dx + M[1] * dy + M[2];
          du = M[3] * dx + M[4] * dy + M[5];
          dx1 = dt-ref_left;
          dy1 = du-ref_top;
          x1 = (Int) dx1;
          y1 = (Int) dy1;
          if(x1>=0 && (x1+1)<ref_width && y1>=0 && (y1+1)<ref_height)
          {
            ref_pel = x1 + ref_width * y1;
            ref_pel1 = x1+1 + ref_width * y1;
            ref_pel2 = x1 + ref_width * (y1+1);
            ref_pel3 = x1+1 + ref_width * (y1+1);
            stop++;
            dk = dx1 - x1;
            d1mk = 1. - dk;
            dl = dy1 - y1;
            d1ml = 1. -dl;
            I1x1y1[0] = ref_work[ref_pel];
            I1x1y1[1] = ref_work[ref_pel1];
            I1x1y1[2] = ref_work[ref_pel2];
            I1x1y1[3] = ref_work[ref_pel3];
            dI1 = d1mk*d1ml*I1x1y1[0] + dk*d1ml*I1x1y1[1] + d1mk*dl*I1x1y1[2] + dk*dl*I1x1y1[3];
            de = dI1 - curr_work[curr_pel];
            if(ite==0)
              error_histgram[(Int)(fabs(de))]++;
            if(fabs(de) <= threshold_T)
            {
              dI1dx = (I1x1y1[1]-I1x1y1[0])*d1ml + (I1x1y1[3]-I1x1y1[2])*dl;
              dI1dy = (I1x1y1[2]-I1x1y1[0])*d1mk + (I1x1y1[3]-I1x1y1[1])*dk;
              ddedm[0] = dx * dI1dx;
              ddedm[1] = dy * dI1dx;
              ddedm[2] = dI1dx;
              ddedm[3] = dx * dI1dy;
              ddedm[4] = dy * dI1dy;
              ddedm[5] = dI1dy;
              db[0] += -de*ddedm[0];
              db[1] += -de*ddedm[1];
              db[2] += -de*ddedm[2];
              db[3] += -de*ddedm[3];
              db[4] += -de*ddedm[4];
              db[5] += -de*ddedm[5];
              for(j=0; j<6; j++)
              {
                for(i=0; i<=j; i++)
                {
                  dA[j*6+i] += ddedm[j] * ddedm[i];
                }
              }
              dE2 += de*de;
            } /* threshold */
          } /* limit of curr_luma area */
        } /* x */
      } /* y */

      if(ite==0)
      {
        stop = stop*80/100;
        j=0;
        for(i=0;i<g_uiIBDI_MAX+1;i++)
        {
          j+=error_histgram[i];
          if(j > stop)
          {
            threshold_T = i;
            break;
          }
        }
      }

      for(j=0; j<6; j++)
      {
        for(i=j+1; i<6; i++)
        {
          dA[j*6+i] = dA[i*6+j];
        }
      }

      check = xDeltaMP(dA, 6, db, dm);

      if(check)
      {
        for(i=0; i<6; i++)
          M[i] += dm[i];

        if(fabs(dm[2]) < 0.001 && fabs(dm[5]) < 0.001 && fabs(dm[0]) < 0.00001 &&
          fabs(dm[1]) < 0.00001 && fabs(dm[3]) < 0.00001 &&
          fabs(dm[4]) < 0.00001 ) break;
      }
      else
      {
        break;
      }
    } /* iteration */

    curr_width *= 2;
    curr_height *= 2;
    curr_left *= 2;
    curr_top *= 2;
    ref_width *= 2;
    ref_height *= 2;
    ref_left *= 2;
    ref_top *= 2;
    offset_x *= 2;
    offset_y *= 2;

    if(level){
      M[2] *= 2;
      M[5] *= 2;
    }
  } /* level */

  rcSpriteMotion.a1 = M[0];
  rcSpriteMotion.a2 = M[1];
  rcSpriteMotion.a3 = M[2];
  rcSpriteMotion.a4 = M[3];
  rcSpriteMotion.a5 = M[4];
  rcSpriteMotion.a6 = M[5];
  rcSpriteMotion.a7 = 0.0;
  rcSpriteMotion.a8 = 0.0;


  for(i=0;i<3;i++) {
    free(ref_P[i]);
    free(curr_P[i]);
  }

  delete[] error_histgram;
}

Void TComGlobalMotion::xTranslationalGME(TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef, Int iVerSpatRef)
{
  Pel* ref, *curr;
  Pel *ref_P[3], *curr_P[3];
  Pel *ref_work, *curr_work;
  Int curr_width, curr_height, curr_left, curr_top;
  Int ref_width, ref_height, ref_left, ref_top;

  Int i, j, best_locationx, best_locationy;
  Int curr_pel, offset_x, offset_y, ref_pel;
  Int ref_pel1, ref_pel2, ref_pel3;
  Double dm[2], db[2], dA[4], M[2];
  Double dE2 = 0;
  Int stop = 0, level = 2, ite = 0;
  Int x, y, x1, y1;
  Double dx1, dy1, dx, dy;
  Double dt, du, dk, d1mk, dl, d1ml,
    dI1, de, dI1dx, dI1dy;
  Double ddedm[2];
  Double I1x1y1[4];
  Int* error_histgram = new Int[g_uiIBDI_MAX+1];
  Int threshold_T=0, check = 1;

//  pcRefFrame->extendPicBorder();

  ref = pcRefFrame->getLumaAddr();
  curr = pcCurrFrame->getLumaAddr();

  ref_left = iHorSpatRef;
  ref_top = iVerSpatRef;
  ref_width = pcRefFrame->getWidth() >> 2;
  ref_height = pcRefFrame->getHeight() >> 2;

  curr_left = iHorSpatRef;
  curr_top = iVerSpatRef;
  curr_width = pcCurrFrame->getWidth() >> 2;
  curr_height = pcCurrFrame->getHeight() >> 2;

  ref_P[0] = (Pel *)malloc(ref_width*ref_height*16*sizeof(Pel));
  curr_P[0] = (Pel *)malloc(curr_width*curr_width*16*sizeof(Pel));
  ref_P[1] = (Pel *)malloc(ref_width*ref_height*4*sizeof(Pel));
  curr_P[1] = (Pel *)malloc(curr_width*curr_height*4*sizeof(Pel));
  ref_P[2] = (Pel *)malloc(ref_width*ref_height*sizeof(Pel));
  curr_P[2] = (Pel *)malloc(curr_width*curr_height*sizeof(Pel));

  Int iStride = pcRefFrame->getStride();
  Pel* piTmp = ref_P[0];
  for (j = 0; j < pcRefFrame->getHeight(); j++)
  for (i = 0; i < pcRefFrame->getWidth(); i++)
  {
    piTmp[i+j*ref_width*4] = ref[i+j*iStride];
  }

  iStride = pcCurrFrame->getStride();
  piTmp = curr_P[0];
  for (j = 0; j < pcCurrFrame->getHeight(); j++)
  for (i = 0; i < pcCurrFrame->getWidth(); i++)
  {
    piTmp[i+j*curr_width*4] = curr[i+j*iStride];
  }

  xThreeTapFilter(ref_P[1], ref_P[0], ref_width*4, ref_height*4);
  xThreeTapFilter(curr_P[1], curr_P[0], curr_width*4, curr_height*4);
  xThreeTapFilter(ref_P[2], ref_P[1], ref_width*2, ref_height*2);
  xThreeTapFilter(curr_P[2], curr_P[1], curr_width*2, curr_height*2);

  for(i = 0; i < 2; i++)
    dm[i] = 0;
  for(i = 0; i < 2; i++)
    M[i] = 0;

  ref_work = ref_P[2];
  curr_work = curr_P[2];

  best_locationx = 0;
  best_locationy = 0;
/* for the absolute coordinate system */
  offset_x = curr_left - ref_left;
  offset_y = curr_top - ref_top;

  threshold_T=xModifiedThreeStepSearch(ref_work, curr_work,
                                      curr_width, curr_height, ref_width, ref_height,
                                      offset_x, offset_y,
                                      &best_locationx, &best_locationy);

  M[0] = (Double)best_locationx;
  M[1] = (Double)best_locationy;

  for(level = 2; level >= 0; level--){

    ref_work = ref_P[level];
    curr_work = curr_P[level];

    for(i=0;i<g_uiIBDI_MAX+1;i++)
      error_histgram[i]=0;

    for(ite = 0; ite < 32; ite++){

      dE2 = 0.;
      stop = 0;
      curr_pel = 0;

      for(j = 0; j < 4; j++)
	dA[j] = 0;
      for(i = 0; i < 2; i++)
	db[i] = 0;

      for(y=0; y<curr_height; y++) {
        dy = (Double)(y + curr_top);
        for(x=0; x<curr_width; x++, curr_pel++) {
          dx = (Double)(x + curr_left);

          dt = dx + M[0] ;
          du = dy + M[1] ;
          dx1 = dt-ref_left;
          dy1 = du-ref_top;
          x1 = (Int) dx1;
          y1 = (Int) dy1;
          if(x1>=0 && (x1+1)<ref_width && y1>=0 && (y1+1)<ref_height) {
            ref_pel = x1 + ref_width * y1;
            ref_pel1 = x1+1 + ref_width * y1;
            ref_pel2 = x1 + ref_width * (y1+1);
            ref_pel3 = x1+1 + ref_width * (y1+1);
            stop++;
            dk = dx1 - x1;
            d1mk = 1. - dk;
            dl = dy1 - y1;
            d1ml = 1. -dl;
            I1x1y1[0] = ref_work[ref_pel];
            I1x1y1[1] = ref_work[ref_pel1];
            I1x1y1[2] = ref_work[ref_pel2];
            I1x1y1[3] = ref_work[ref_pel3];
            dI1 = d1mk*d1ml*I1x1y1[0] + dk*d1ml*I1x1y1[1] +
              d1mk*dl*I1x1y1[2] + dk*dl*I1x1y1[3];
            de = dI1 - curr_work[curr_pel];
            if(ite==0)
              error_histgram[(Int)(fabs(de))]++;
            if(fabs(de) <= threshold_T) {
              dI1dx = (I1x1y1[1]-I1x1y1[0])*d1ml + (I1x1y1[3]-I1x1y1[2])*dl;
              dI1dy = (I1x1y1[2]-I1x1y1[0])*d1mk + (I1x1y1[3]-I1x1y1[1])*dk;
              ddedm[0] = dI1dx;
              ddedm[1] = dI1dy;
              db[0] += -de*ddedm[0];
              db[1] += -de*ddedm[1];
              for(j=0; j<2; j++)
                for(i=0; i<=j; i++)
                  dA[j*2+i] += ddedm[j] * ddedm[i];
              dE2 += de*de;
            } /* threshold */
          } /* limit of curr_luma area */
        } /* x */
      } /* y */

      if(ite==0){
        stop = stop*80/100;
        j=0;
        for(i=0;i<g_uiIBDI_MAX+1;i++){
          j+=error_histgram[i];
          if(j > stop){
            threshold_T = i;
            break;
          }
        }
      }

      for(j=0; j<2; j++)
	for(i=j+1; i<2; i++)
          dA[j*2+i] = dA[i*2+j];

      check = xDeltaMP(dA, 2, db, dm);

      if(check){
        for(i=0; i<2; i++)
          M[i] += dm[i];
	if(fabs(dm[0]) < 0.001 && fabs(dm[1]) < 0.001 ) break;
      }else{
        break;
      }

    } /* iteration */

    curr_width *= 2;
    curr_height *= 2;
    curr_left *= 2;
    curr_top *= 2;
    ref_width *= 2;
    ref_height *= 2;
    ref_left *= 2;
    ref_top *= 2;
    offset_x *= 2;
    offset_y *= 2;

    if(level){
      M[0] *= 2;
      M[1] *= 2;
    }
  } /* level */

  rcSpriteMotion.a1 = 1.0;
  rcSpriteMotion.a2 = 0.0;
  rcSpriteMotion.a3 = M[0];
  rcSpriteMotion.a4 = 0.0;
  rcSpriteMotion.a5 = 1.0;
  rcSpriteMotion.a6 = M[1];
  rcSpriteMotion.a7 = 0.0;
  rcSpriteMotion.a8 = 0.0;


  for(i=0;i<3;i++) {
    free(ref_P[i]);
    free(curr_P[i]);
  }

  delete[] error_histgram;
}

Void TComGlobalMotion::xIsotropicGME(TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef, Int iVerSpatRef)
{
  Pel* ref, *curr;
  Pel *ref_P[3], *curr_P[3];
  Pel *ref_work, *curr_work;
  Int curr_width, curr_height, curr_left, curr_top;
  Int ref_width, ref_height, ref_left, ref_top;

  Int i, j, best_locationx, best_locationy;
  Int curr_pel, offset_x, offset_y, ref_pel;
  Int ref_pel1, ref_pel2, ref_pel3;
  Double dm[4], db[4], dA[16], M[4];
  Double dE2 = 0;
  Int stop = 0, level = 2, ite = 0;
  Int x, y, x1, y1;
  Double dx1, dy1, dx, dy;
  Double dt, du, dk, d1mk, dl, d1ml,
    dI1, de, dI1dx, dI1dy;
  Double ddedm[4];
  Double I1x1y1[4];
  Int* error_histgram = new Int[g_uiIBDI_MAX+1];
  Int threshold_T=0, check = 1;

//  pcRefFrame->extendPicBorder();

  ref = pcRefFrame->getLumaAddr();
  curr = pcCurrFrame->getLumaAddr();

  ref_left = iHorSpatRef;
  ref_top = iVerSpatRef;
  ref_width = pcRefFrame->getWidth() >> 2;
  ref_height = pcRefFrame->getHeight() >> 2;

  curr_left = iHorSpatRef;
  curr_top = iVerSpatRef;
  curr_width = pcCurrFrame->getWidth() >> 2;
  curr_height = pcCurrFrame->getHeight() >> 2;

  ref_P[0] = (Pel *)malloc(ref_width*ref_height*16*sizeof(Pel));
  curr_P[0] = (Pel *)malloc(curr_width*curr_width*16*sizeof(Pel));
  ref_P[1] = (Pel *)malloc(ref_width*ref_height*4*sizeof(Pel));
  curr_P[1] = (Pel *)malloc(curr_width*curr_height*4*sizeof(Pel));
  ref_P[2] = (Pel *)malloc(ref_width*ref_height*sizeof(Pel));
  curr_P[2] = (Pel *)malloc(curr_width*curr_height*sizeof(Pel));

  Int iStride = pcRefFrame->getStride();
  Pel* piTmp = ref_P[0];
  for (j = 0; j < pcRefFrame->getHeight(); j++)
  for (i = 0; i < pcRefFrame->getWidth(); i++)
  {
    piTmp[i+j*ref_width*4] = ref[i+j*iStride];
  }

  iStride = pcCurrFrame->getStride();
  piTmp = curr_P[0];
  for (j = 0; j < pcCurrFrame->getHeight(); j++)
  for (i = 0; i < pcCurrFrame->getWidth(); i++)
  {
    piTmp[i+j*curr_width*4] = curr[i+j*iStride];
  }

  xThreeTapFilter(ref_P[1], ref_P[0], ref_width*4, ref_height*4);
  xThreeTapFilter(curr_P[1], curr_P[0], curr_width*4, curr_height*4);
  xThreeTapFilter(ref_P[2], ref_P[1], ref_width*2, ref_height*2);
  xThreeTapFilter(curr_P[2], curr_P[1], curr_width*2, curr_height*2);

  for(i = 0; i < 4; i++)
    dm[i] = 0;
  for(i = 0; i < 4; i++)
    M[i] = 0;
  M[0]=1.0;

  ref_work = ref_P[2];
  curr_work = curr_P[2];

  best_locationx = 0;
  best_locationy = 0;
/* for the absolute coordinate system */
  offset_x = curr_left - ref_left;
  offset_y = curr_top - ref_top;

  threshold_T=xModifiedThreeStepSearch(ref_work, curr_work,
                                      curr_width, curr_height, ref_width, ref_height,
                                      offset_x, offset_y,
                                      &best_locationx, &best_locationy);

  M[2] = (Double)best_locationx;
  M[3] = (Double)best_locationy;

  for(level = 2; level >= 0; level--){

    ref_work = ref_P[level];
    curr_work = curr_P[level];

    for(i=0;i<g_uiIBDI_MAX+1;i++)
      error_histgram[i]=0;

    for(ite = 0; ite < 32; ite++){

      dE2 = 0.;
      stop = 0;
      curr_pel = 0;

      for(j = 0; j < 16; j++)
	dA[j] = 0;
      for(i = 0; i < 4; i++)
	db[i] = 0;

      for(y=0; y<curr_height; y++) {
        dy = (Double)(y + curr_top);
        for(x=0; x<curr_width; x++, curr_pel++) {
          dx = (Double)(x + curr_left);

          dt = M[0] * dx + M[1] * dy + M[2];
          du = -M[1] * dx + M[0] * dy + M[3];
          dx1 = dt-ref_left;
          dy1 = du-ref_top;
          x1 = (Int) dx1;
          y1 = (Int) dy1;
          if(x1>=0 && (x1+1)<ref_width && y1>=0 && (y1+1)<ref_height) {
            ref_pel = x1 + ref_width * y1;
            ref_pel1 = x1+1 + ref_width * y1;
            ref_pel2 = x1 + ref_width * (y1+1);
            ref_pel3 = x1+1 + ref_width * (y1+1);
            stop++;
            dk = dx1 - x1;
            d1mk = 1. - dk;
            dl = dy1 - y1;
            d1ml = 1. -dl;
            I1x1y1[0] = ref_work[ref_pel];
            I1x1y1[1] = ref_work[ref_pel1];
            I1x1y1[2] = ref_work[ref_pel2];
            I1x1y1[3] = ref_work[ref_pel3];
            dI1 = d1mk*d1ml*I1x1y1[0] + dk*d1ml*I1x1y1[1] +
              d1mk*dl*I1x1y1[2] + dk*dl*I1x1y1[3];
            de = dI1 - curr_work[curr_pel];
            if(ite==0)
              error_histgram[(Int)(fabs(de))]++;
            if(fabs(de) <= threshold_T) {
              dI1dx = (I1x1y1[1]-I1x1y1[0])*d1ml + (I1x1y1[3]-I1x1y1[2])*dl;
              dI1dy = (I1x1y1[2]-I1x1y1[0])*d1mk + (I1x1y1[3]-I1x1y1[1])*dk;
              ddedm[0] = dx * dI1dx + dy * dI1dy;
              ddedm[1] = dy * dI1dx - dx * dI1dy;
              ddedm[2] = dI1dx;
              ddedm[3] = dI1dy;
              db[0] += -de*ddedm[0];
              db[1] += -de*ddedm[1];
              db[2] += -de*ddedm[2];
              db[3] += -de*ddedm[3];
              for(j=0; j<4; j++)
                for(i=0; i<=j; i++)
                  dA[j*4+i] += ddedm[j] * ddedm[i];
              dE2 += de*de;
            } /* threshold */
          } /* limit of curr_luma area */
        } /* x */
      } /* y */

      if(ite==0){
        stop = stop*80/100;
        j=0;
        for(i=0;i<g_uiIBDI_MAX+1;i++){
          j+=error_histgram[i];
          if(j > stop){
            threshold_T = i;
            break;
          }
        }
      }

      for(j=0; j<4; j++)
	for(i=j+1; i<4; i++)
          dA[j*4+i] = dA[i*4+j];

      check = xDeltaMP(dA, 4, db, dm);

      if(check){
        for(i=0; i<4; i++)
          M[i] += dm[i];

        if(fabs(dm[2]) < 0.001 && fabs(dm[3]) < 0.001 && fabs(dm[0]) < 0.00001 &&
           fabs(dm[1]) < 0.00001 ) break;
      }else {
        break;
      }

    } /* iteration */

    curr_width *= 2;
    curr_height *= 2;
    curr_left *= 2;
    curr_top *= 2;
    ref_width *= 2;
    ref_height *= 2;
    ref_left *= 2;
    ref_top *= 2;
    offset_x *= 2;
    offset_y *= 2;

    if(level){
      M[2] *= 2;
      M[3] *= 2;
    }
  } /* level */

  rcSpriteMotion.a1 = M[0];
  rcSpriteMotion.a2 = M[1];
  rcSpriteMotion.a3 = M[2];
  rcSpriteMotion.a4 = -M[1];
  rcSpriteMotion.a5 = M[0];
  rcSpriteMotion.a6 = M[3];
  rcSpriteMotion.a7 = 0.0;
  rcSpriteMotion.a8 = 0.0;

  for(i=0;i<3;i++) {
    free(ref_P[i]);
    free(curr_P[i]);
  }

  delete[] error_histgram;
}

Void TComGlobalMotion::xPerspectiveGME(TComPicYuv* pcRefFrame, TComPicYuv* pcCurrFrame, SpriteMotion& rcSpriteMotion, Int iHorSpatRef, Int iVerSpatRef)
{
  Pel* ref, *curr;
  Pel *ref_P[3], *curr_P[3];
  Pel *ref_work, *curr_work;
  Int curr_width, curr_height, curr_left, curr_top;
  Int ref_width, ref_height, ref_left, ref_top;

  Int i, j, best_locationx, best_locationy;
  Int curr_pel, offset_x, offset_y, ref_pel;
  Int ref_pel1, ref_pel2, ref_pel3;
  Double dm[8], db[8], dA[64], M[8];
  Double dE2 = 0;
  Int stop = 0, level = 2, ite = 0;
  Int x, y, x1, y1;
  Double dx1, dy1, dx, dy;
  Double dt, du, dv, dtOv, duOv, dlOv, dxOv, dyOv, dk, d1mk, dl, d1ml, dI1, de, dI1dx, dI1dy;
  Double ddedm[8];
  Double I1x1y1[4];
  Int* error_histgram = new Int[g_uiIBDI_MAX+1];
  Int threshold_T=0, check = 1;

//  pcRefFrame->extendPicBorder();

  ref = pcRefFrame->getLumaAddr();
  curr = pcCurrFrame->getLumaAddr();

  ref_left = iHorSpatRef;
  ref_top = iVerSpatRef;
  ref_width = pcRefFrame->getWidth() >> 2;
  ref_height = pcRefFrame->getHeight() >> 2;

  curr_left = iHorSpatRef;
  curr_top = iVerSpatRef;
  curr_width = pcCurrFrame->getWidth() >> 2;
  curr_height = pcCurrFrame->getHeight() >> 2;

  ref_P[0] = (Pel *)malloc(ref_width*ref_height*16*sizeof(Pel));
  curr_P[0] = (Pel *)malloc(curr_width*curr_width*16*sizeof(Pel));
  ref_P[1] = (Pel *)malloc(ref_width*ref_height*4*sizeof(Pel));
  curr_P[1] = (Pel *)malloc(curr_width*curr_height*4*sizeof(Pel));
  ref_P[2] = (Pel *)malloc(ref_width*ref_height*sizeof(Pel));
  curr_P[2] = (Pel *)malloc(curr_width*curr_height*sizeof(Pel));

  Int iStride = pcRefFrame->getStride();
  Pel* piTmp = ref_P[0];
  for (j = 0; j < pcRefFrame->getHeight(); j++)
  for (i = 0; i < pcRefFrame->getWidth(); i++)
  {
    piTmp[i+j*ref_width*4] = ref[i+j*iStride];
  }

  iStride = pcCurrFrame->getStride();
  piTmp = curr_P[0];
  for (j = 0; j < pcCurrFrame->getHeight(); j++)
  for (i = 0; i < pcCurrFrame->getWidth(); i++)
  {
    piTmp[i+j*curr_width*4] = curr[i+j*iStride];
  }

  xThreeTapFilter(ref_P[1], ref_P[0], ref_width*4, ref_height*4);
  xThreeTapFilter(curr_P[1], curr_P[0], curr_width*4, curr_height*4);
  xThreeTapFilter(ref_P[2], ref_P[1], ref_width*2, ref_height*2);
  xThreeTapFilter(curr_P[2], curr_P[1], curr_width*2, curr_height*2);

  for(i = 0; i < 8; i++)
    dm[i] = 0;
  for(i = 0; i < 8; i++)
    M[i] = 0;
  M[0]=1.0;
  M[4]=1.0;

  ref_work = ref_P[2];
  curr_work = curr_P[2];

  best_locationx = 0;
  best_locationy = 0;
/* for the absolute coordinate system */
  offset_x = curr_left - ref_left;
  offset_y = curr_top - ref_top;

  threshold_T=xModifiedThreeStepSearch(ref_work, curr_work,
                                      curr_width, curr_height, ref_width, ref_height,
                                      offset_x, offset_y,
                                      &best_locationx, &best_locationy);

  M[2] = (Double)best_locationx;
  M[5] = (Double)best_locationy;

  for(level = 2; level >= 0; level--){

    ref_work = ref_P[level];
    curr_work = curr_P[level];

    for(i=0;i<g_uiIBDI_MAX+1;i++)
      error_histgram[i]=0;

    for(ite = 0; ite < 32; ite++){

      dE2 = 0.;
      stop = 0;
      curr_pel = 0;

      for(j = 0; j < 64; j++)
	dA[j] = 0;
      for(i = 0; i < 8; i++)
	db[i] = 0;

      for(y=0; y<curr_height; y++) {
        dy = (Double)(y + curr_top);
        for(x=0; x<curr_width; x++, curr_pel++) {
          dx = (Double)(x + curr_left);

          dt = M[0] * dx + M[1] * dy + M[2];
          du = M[3] * dx + M[4] * dy + M[5];
          dv = M[6] * dx + M[7] * dy + 1;
          dtOv = dt / dv;
          duOv = du / dv;
          dx1 = dtOv-ref_left;
          dy1 = duOv-ref_top;
          x1 = (Int) dx1;
          y1 = (Int) dy1;
          if(x1>=0 && (x1+1)<ref_width && y1>=0 && (y1+1)<ref_height) {
            ref_pel = x1 + ref_width * y1;
            ref_pel1 = x1+1 + ref_width * y1;
            ref_pel2 = x1 + ref_width * (y1+1);
            ref_pel3 = x1+1 + ref_width * (y1+1);
            stop++;
            dlOv = 1 / dv;
            dxOv = dx / dv;
            dyOv = dy / dv;
            dk = dx1 - x1;
            d1mk = 1. - dk;
            dl = dy1 - y1;
            d1ml = 1. -dl;
            I1x1y1[0] = ref_work[ref_pel];
            I1x1y1[1] = ref_work[ref_pel1];
            I1x1y1[2] = ref_work[ref_pel2];
            I1x1y1[3] = ref_work[ref_pel3];
            dI1 = d1mk*d1ml*I1x1y1[0] + dk*d1ml*I1x1y1[1] +
              d1mk*dl*I1x1y1[2] + dk*dl*I1x1y1[3];
            de = dI1 - curr_work[curr_pel];
            if(ite==0)
              error_histgram[(Int)(fabs(de))]++;
            if(fabs(de) <= threshold_T) {
              dI1dx = (I1x1y1[1]-I1x1y1[0])*d1ml + (I1x1y1[3]-I1x1y1[2])*dl;
              dI1dy = (I1x1y1[2]-I1x1y1[0])*d1mk + (I1x1y1[3]-I1x1y1[1])*dk;
              ddedm[0] = dxOv * dI1dx;
              ddedm[1] = dyOv * dI1dx;
              ddedm[2] = dlOv * dI1dx;
              ddedm[3] = dxOv * dI1dy;
              ddedm[4] = dyOv * dI1dy;
              ddedm[5] = dlOv * dI1dy;
              ddedm[6] = -dtOv*dxOv*dI1dx-duOv*dxOv*dI1dy;
              ddedm[7] = -dtOv*dyOv*dI1dx-duOv*dyOv*dI1dy;
              db[0] += -de*ddedm[0];
              db[1] += -de*ddedm[1];
              db[2] += -de*ddedm[2];
              db[3] += -de*ddedm[3];
              db[4] += -de*ddedm[4];
              db[5] += -de*ddedm[5];
              db[6] += -de*ddedm[6];
              db[7] += -de*ddedm[7];
              for(j=0; j<8; j++)
                for(i=0; i<=j; i++)
                  dA[j*8+i] += ddedm[j] * ddedm[i];
              dE2 += de*de;
            } /* threshold */
          } /* limit of curr_luma area */
        } /* x */
      } /* y */

      if(ite==0){
        stop = stop*80/100;
        j=0;
        for(i=0;i<g_uiIBDI_MAX+1;i++){
          j+=error_histgram[i];
          if(j > stop){
            threshold_T = i;
            break;
          }
        }
      }

      for(j=0; j<8; j++)
	for(i=j+1; i<8; i++)
          dA[j*8+i] = dA[i*8+j];

      check = xDeltaMP(dA, 8, db, dm);

      if(check){
        for(i=0; i<8; i++)
          M[i] += dm[i];

        if(fabs(dm[2]) < 0.001 && fabs(dm[5]) < 0.001 && fabs(dm[0]) < 0.00001 &&
           fabs(dm[1]) < 0.00001 && fabs(dm[3]) < 0.00001 && fabs(dm[4]) < 0.00001 &&
           fabs(dm[6]) < 0.00001 && fabs(dm[7]) < 0.00001) break;
      }else {
        break;
      }

    } /* iteration */

    curr_width *= 2;
    curr_height *= 2;
    curr_left *= 2;
    curr_top *= 2;
    ref_width *= 2;
    ref_height *= 2;
    ref_left *= 2;
    ref_top *= 2;
    offset_x *= 2;
    offset_y *= 2;

    if(level){
      M[2] *= 2;
      M[5] *= 2;
      M[6] /= 2;
      M[7] /= 2;
    }
  } /* level */

  rcSpriteMotion.a1 = M[0];
  rcSpriteMotion.a2 = M[1];
  rcSpriteMotion.a3 = M[2];
  rcSpriteMotion.a4 = M[3];
  rcSpriteMotion.a5 = M[4];
  rcSpriteMotion.a6 = M[5];
  rcSpriteMotion.a7 = M[6];
  rcSpriteMotion.a8 = M[7];


  for(i=0;i<3;i++) {
    free(ref_P[i]);
    free(curr_P[i]);
  }

  delete[] error_histgram;
}

Int TComGlobalMotion::xModifiedThreeStepSearch( Pel*   pRefWork,
                                              Pel*   pCurrWork,
                                              Int     iCurrWidth,
                                              Int     iCurrHeight,
                                              Int     iRefWidth,
                                              Int     iRefHeight,
                                              Int     iOffsetX,
                                              Int     iOffsetY,
                                              Int*    piBestLocationX,
                                              Int*    piBestLocationY)
{
  Int i,j, locationx, locationy, no_of_pel=0;
  Int curr_pel, ref_pel, ref_x, ref_y;
  Int range_locationx=0, range_locationy=0;
  Int thrs=g_uiIBDI_MAX, total_no;
  Double dE1 = 0, min_error=300;
  Int* hist = new Int [g_uiIBDI_MAX+1];
  Int round=0, last_thrs;
  Int error;

  do
  {
    last_thrs=thrs;

    range_locationx=*piBestLocationX;
    range_locationy=*piBestLocationY;
    for (locationy = range_locationy-8; locationy <= range_locationy+8; locationy ++)
    {
      for (locationx = range_locationx-8; locationx <= range_locationx+8; locationx ++)
      {
        dE1 = 0.0;
        no_of_pel = 0;
        total_no = 0;
        curr_pel = 0;
        for (j = 0; j< iCurrHeight ; j++)
        {
          for (i = 0; i< iCurrWidth ; i++, curr_pel++)
          {

            ref_x = locationx + i + iOffsetX;
            ref_y = locationy + j + iOffsetY;
            ref_pel = ref_y * iRefWidth + ref_x;

            if ((ref_x >=  0) && (ref_x < iRefWidth) &&
              (ref_y >=  0) && (ref_y < iRefHeight))
            {

              if((error=abs(pCurrWork[curr_pel] - pRefWork[ref_pel]))<=last_thrs)
              {
                dE1 += error;
                no_of_pel ++;
              }
              total_no++;

            } // limit of ref area
          } // i
        } // j

        if (no_of_pel > 0)
          dE1 = dE1 / no_of_pel;
        else
          dE1 = min_error+1;
        if (((dE1 < min_error) && (no_of_pel*2>total_no)) ||
          ((dE1 == min_error) && (no_of_pel*2>total_no) && (abs(*piBestLocationX) +
          abs(*piBestLocationY)) > (abs(locationx) + abs(locationy))))
        {
          min_error = dE1;
          *piBestLocationX = locationx;
          *piBestLocationY = locationy;
        }

      } // locationx
    } // locationy

    for (j = 0; j< g_uiIBDI_MAX+1 ; j++)
      hist[j]=0;

    no_of_pel=0;
    curr_pel = 0;
    for (j = 0; j< iCurrHeight ; j++)
    {
      for (i = 0; i< iCurrWidth ; i++, curr_pel++)
      {

        ref_x = *piBestLocationX + i + iOffsetX;
        ref_y = *piBestLocationY + j + iOffsetY;
        ref_pel = ref_y * iRefWidth + ref_x;

        if ((ref_x >=  0) && (ref_x < iRefWidth) &&
          (ref_y >=  0) && (ref_y < iRefHeight))
        {
          hist[abs(pCurrWork[curr_pel] - pRefWork[ref_pel])]++;
          no_of_pel ++;

        } // limit of ref area
      } // i
    } // j

    no_of_pel=(Int)((Double)no_of_pel*0.8);

    j=0;
    for (i=0;i<g_uiIBDI_MAX+1;i++)
    {
      j+=hist[i];
      if(j>no_of_pel){thrs=i;break;}
    }

    round++;

  } while(thrs!=last_thrs && round<5);


  delete[] hist;
  return(thrs);
}

Int TComGlobalMotion::xDeltaMP(Double *dA, Int n, Double *db, Double *dm)
{
  Int i, j, i2, n2=n*2, k, i_pivot;
  Double *dAi = new Double[n * n * 2];
  Double pivot, tmp;

  pivot = *dA;
  for(j = 0; j < n; j++)
  {
    for(i = 0; i < n; i++)
    {
      if(fabs(*(dA + j * n + i)) > fabs(pivot))
        pivot = *(dA + j * n + i);
    }
  }

  if(fabs(pivot) < 0.000000001)
  {
    delete dAi;
    return(0);
  }
  pivot = 1.0 / pivot;

  for(j = 0; j < n; j++)
  {
    for(i = 0, i2 = n; i < n; i++, i2++)
    {
      *(dAi + j * n2 + i) = (*(dA + j * n + i))*pivot;
      if(i == j)
        *(dAi + j * n2 + i2) = pivot;
      else
        *(dAi + j * n2 + i2) = 0.0;
    }
  }

  for(i = 0; i < n; i++)
  {
    i_pivot = i;
    pivot = *(dAi + i * n2 + i);
    for(j = i; j < n; j++)
    {
      if(fabs(*(dAi + j * n2 + i)) > fabs(pivot))
      {
        i_pivot = j;
        pivot = *(dAi + j * n2 + i);
      }
    }
    if(fabs(pivot) < 0.000000001)
    {
      free(dAi);
      return(0);
    }
    if(i_pivot!=i)
    {
      for(k = 0; k < n2; k++)
      {
        tmp = *(dAi + i * n2 + k);
        *(dAi + i * n2 + k) = *(dAi + i_pivot * n2 + k);
        *(dAi + i_pivot * n2 + k) = tmp;
      }
    }

    for(j = 0; j < n; j++)
    {
      if(j != i)
      {
        pivot = *(dAi + j * n2 + i) / *(dAi + i * n2 + i);
        for(k = i+1; k < n2; k++)
        {
          *(dAi + j * n2 + k) -= pivot*(*(dAi + i * n2 + k));
        }
      }
    }

  }

  for(j = 0; j < n; j++)
  {
    for(i = 0, i2 = n; i < n; i++, i2++)
    {
      *(dAi + j * n2 + i2) /= *(dAi + j * n2 + j);
    }
  }

  for(i = 0; i < n; i++)
  {
    *(dm + i) = 0.0;
  }

  for(j = 0; j < n; j++)
  {
    for(i = 0, i2 = n; i < n; i++, i2++)
    {
      *(dm + j) += (*(dAi + j * n2 +i2))*(*(db + i));
    }
  }

  delete dAi;

  return(1);
}

