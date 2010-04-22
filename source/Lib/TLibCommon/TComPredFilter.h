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

/** \file			TComPredFilter.h
    \brief		interpolation filter class (header)
*/

#ifndef __TCOMPREDFILTER__
#define __TCOMPREDFILTER__

// Include files
#include "TComPic.h"
#include "TComMotionInfo.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

// Local type definitions
#define HAL_IDX   ((HAM_ACC>>1) + (HAM_ACC>>2) - 1)
#define QU0_IDX   ((HAM_ACC>>1) - 1                 )
#define QU1_IDX   ((HAM_ACC   ) - 1                 )

// ====================================================================================================================
// Type definitions
// ====================================================================================================================

// define function pointers
typedef Int (*FpCTIFilter_VP) ( Pel* pSrc, Int* piCoeff, Int iStride );
typedef Int (*FpCTIFilter_VI) ( Int* pSrc, Int* piCoeff, Int iStride );

// filter coefficient array
extern Int CTI_Filter12 [6][14][14];

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// interpolation filter class
class TComPredFilter
{
protected:
	// filter description (luma & chroma)
	Int		m_iDIFTap;
	Int		m_iDIFTapC;
private:

	// filter description (luma)
	Int	  m_iTapIdx;
	Int   m_iLeftMargin;
	Int		m_iRightMargin;

	// filter functions
	FpCTIFilter_VP xCTI_Filter_VP  [14];	// Pel-type
	FpCTIFilter_VP xCTI_Filter_VPS [14];	// Pel-type, symmetric
	FpCTIFilter_VI xCTI_Filter_VI  [14];	// Int-type
	FpCTIFilter_VI xCTI_Filter_VIS [14];	// Int-type, symmetric

	// filter description (chroma)
	Int	  m_iTapIdxC;
	Int   m_iLeftMarginC;
	Int		m_iRightMarginC;

	// filter functions
	FpCTIFilter_VP xCTI_Filter_VPC  [14];	// Pel-type
	FpCTIFilter_VP xCTI_Filter_VPSC [14];	// Pel-type, symmetric
	FpCTIFilter_VI xCTI_Filter_VIC  [14];	// Int-type
	FpCTIFilter_VI xCTI_Filter_VISC [14];	// Int-type, symmetric

public:
  TComPredFilter();

	Void	setDIFTap ( Int i );
	Void	setDIFTapC( Int i );

  __inline Void  xFilterHalfHorRf0  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel);
  __inline Void  xFilterHalfHorRf1  (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel);
  __inline Void  xFilterHalfHorRf2  (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel);
  __inline Void  xFilterHalfVerRf0  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst);
  __inline Void  xFilterHalfVerRf1  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst);
  __inline Void  xFilterHalfVerRf2  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst);
  __inline Void  xRoundOffHalfVerRf1(Int iWidth,	Int iHeight,	 Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel );

  //for MC
  __inline Void  xFilterHalfHorRf1  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void  xFilterHalfHorRf1  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst);
  __inline Void  xFilterHalfHorRf2  (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst);
  __inline Void  xFilterHalfHorRf2  (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void  xFilterHalfVerRf1  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void  xFilterBilinear    (Pel* piSrc0, Int iSrcStride0, Int iSrcStep0, Int* piSrc1, Int iSrcStride1, Int iSrcStep1,  Int iWidth,   Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void  xFilterBilinear    (Int* piSrc0, Int iSrcStride0, Int iSrcStep0, Int* piSrc1, Int iSrcStride1, Int iSrcStep1,  Int iWidth,   Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void  xRoundOffHalfVerRf1(Int iWidth,  Int iHeight,	  Int iDstStride, Int iDstStep, Int*& rpiDst);
  //--for MC

  __inline Void  xFilterBilinearRect    (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Pel* piDst, Int iDstStride, Int iDstStep, Int px, Int py, Int s);

  Void  xFilterQuarterHor     (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  Void  xFilterQuarterVer     (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  Void  xFilterQuarterDiagUp  (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  Void  xFilterQuarterDiagDown(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);

	//--DIF
  __inline Void xCTI_FilterHalfHor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void xCTI_FilterHalfHor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);

  __inline Void xCTI_FilterQuarter0Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void xCTI_FilterQuarter0Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);

  __inline Void xCTI_FilterQuarter1Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);
  __inline Void xCTI_FilterQuarter1Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);

  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel );
  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst );
  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst );

  __inline Void xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst );
  __inline Void xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst );

  __inline Void xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst );
  __inline Void xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst );
	//--

	//-- HAM
  __inline Void xCTI_Filter2DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMv);
  __inline Void xCTI_Filter2DHor (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DHor (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);

	__inline Void xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMv);
  __inline Void xCTI_Filter2DHorC (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DHorC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
	//--

private:
	static Int xCTI_Filter_VP04			( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VP06			( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VP08			( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VP10			( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VP12			( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VP14			( Pel* pSrc, Int* piCoeff, Int iStride );

	static Int xCTI_Filter_VPS04		( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VPS06		( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VPS08		( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VPS10		( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VPS12		( Pel* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VPS14		( Pel* pSrc, Int* piCoeff, Int iStride );

	static Int xCTI_Filter_VI04			( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VI06			( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VI08			( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VI10			( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VI12			( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VI14			( Int* pSrc, Int* piCoeff, Int iStride );

	static Int xCTI_Filter_VIS04		( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VIS06		( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VIS08		( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VIS10		( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VIS12		( Int* pSrc, Int* piCoeff, Int iStride );
	static Int xCTI_Filter_VIS14		( Int* pSrc, Int* piCoeff, Int iStride );

	__inline Int xCTI_Filter_VPS04_HAL( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VIS04_HAL( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP04_QU0 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI04_QU0 ( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP04_QU1 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI04_QU1 ( Int* pSrc, Int* piCoeff, Int iStride );

	__inline Int xCTI_Filter_VPS06_HAL( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VIS06_HAL( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP06_QU0 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI06_QU0 ( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP06_QU1 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI06_QU1 ( Int* pSrc, Int* piCoeff, Int iStride );

	__inline Int xCTI_Filter_VPS12_HAL( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VIS12_HAL( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP12_QU0 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI12_QU0 ( Int* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VP12_QU1 ( Pel* pSrc, Int* piCoeff, Int iStride );
	__inline Int xCTI_Filter_VI12_QU1 ( Int* pSrc, Int* piCoeff, Int iStride );

};// END CLASS DEFINITION TComPredFilter

// ------------------------------------------------------------------------------------------------
// AVC filters
// ------------------------------------------------------------------------------------------------

__inline Void TComPredFilter::xFilterHalfHorRf0 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel)
{
  Int*  piDst    = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = iSum;
      piDstPel[x * iDstStep] = iSum;
    }
    piSrc    += iSrcStride;
    piDst    += iDstStride;
    piDstPel += iDstStridePel;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfHorRf1 (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel )
{
  Int*  piDst    = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum +  16) >>  5 );
      piDstPel[x * iDstStep] = Clip( (iSum +  16) >>  5 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
    piDstPel += iDstStridePel;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfHorRf2 (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel)
{
  Int*  piDst    = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int   iSum;
  for( Int y = iHeight; y != 0; y-- )
  {
    for( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum + 512) >> 10 );
      piDstPel[x * iDstStep] = Clip( (iSum + 512) >> 10 );
    }
    piSrc    += iSrcStride;
    piDst    += iDstStride;
    piDstPel += iDstStridePel;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfHorRf2 (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum + 512) >> 10 );
    }
    piSrc    += iSrcStride;
    piDst    += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfVerRf0 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst    = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[x*iSrcStep - 0*iSrcStride] + piSrc[x*iSrcStep + 1*iSrcStride]) << 2;
      iSum -= (piSrc[x*iSrcStep - 1*iSrcStride] + piSrc[x*iSrcStep + 2*iSrcStride]);
      iSum += iSum << 2;
      iSum += (piSrc[x*iSrcStep - 2*iSrcStride] + piSrc[x*iSrcStep + 3*iSrcStride]);

      piDst   [x * iDstStep] = iSum;
    }
    piSrc    += iSrcStride;
    piDst    += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfVerRf1 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[x*iSrcStep - 0*iSrcStride] + piSrc[x*iSrcStep + 1*iSrcStride]) << 2;
      iSum -= (piSrc[x*iSrcStep - 1*iSrcStride] + piSrc[x*iSrcStep + 2*iSrcStride]);
      iSum += iSum << 2;
      iSum += (piSrc[x*iSrcStep - 2*iSrcStride] + piSrc[x*iSrcStep + 3*iSrcStride]);

      piDst[x * iDstStep] = Clip( (iSum +  16) >>  5 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfVerRf2 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[x*iSrcStep - 0*iSrcStride] + piSrc[x*iSrcStep + 1*iSrcStride]) << 2;
      iSum -= (piSrc[x*iSrcStep - 1*iSrcStride] + piSrc[x*iSrcStep + 2*iSrcStride]);
      iSum += iSum << 2;
      iSum += (piSrc[x*iSrcStep - 2*iSrcStride] + piSrc[x*iSrcStep + 3*iSrcStride]);

      piDst[x * iDstStep] = Clip( (iSum + 512) >> 10 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}


__inline Void TComPredFilter::xRoundOffHalfVerRf1 (Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel)
{
  Int*  piDst = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int tmp;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      tmp = Clip( (piDst[x * iDstStep] +  16) >>  5 );
      piDst   [x * iDstStep] = tmp;
      piDstPel[x * iDstStep] = tmp;
    }
    piDst    += iDstStride;
    piDstPel += iDstStridePel;
  }
  return;
}

//for MC
__inline Void TComPredFilter::xFilterHalfHorRf1 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum +  16) >>  5 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfHorRf2 (Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum + 512) >> 10 );
    }
    piSrc    += iSrcStride;
    piDst    += iDstStride;
  }
  return;
}
__inline Void TComPredFilter::xRoundOffHalfVerRf1 (Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst   [x * iDstStep] = Clip( (piDst[x * iDstStep] +  16) >>  5 );
    }
    piDst    += iDstStride;
  }
  return;
}
__inline Void TComPredFilter::xFilterHalfVerRf1 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[x*iSrcStep - 0*iSrcStride] + piSrc[x*iSrcStep + 1*iSrcStride]) << 2;
      iSum -= (piSrc[x*iSrcStep - 1*iSrcStride] + piSrc[x*iSrcStep + 2*iSrcStride]);
      iSum += iSum << 2;
      iSum += (piSrc[x*iSrcStep - 2*iSrcStride] + piSrc[x*iSrcStep + 3*iSrcStride]);

      piDst[x * iDstStep] = Clip( (iSum +  16) >>  5 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xFilterHalfHorRf1 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum  = (piSrc[(x - 0)*iSrcStep] + piSrc[(x + 1)*iSrcStep]) << 2;
      iSum -= (piSrc[(x - 1)*iSrcStep] + piSrc[(x + 2)*iSrcStep]);
      iSum += iSum << 2;
      iSum += (piSrc[(x - 2)*iSrcStep] + piSrc[(x + 3)*iSrcStep]);

      piDst   [x * iDstStep] = Clip( (iSum +  16) >>  5 );
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}
__inline Void TComPredFilter::xFilterBilinear(Int* piSrc0, Int iSrcStride0, Int iSrcStep0, Int* piSrc1, Int iSrcStride1, Int iSrcStep1, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst   [x * iDstStep] = (piSrc0[x*iSrcStep0] + piSrc1[x*iSrcStep1] + 1) >> 1;
    }
    piSrc0 += iSrcStride0;
    piSrc1 += iSrcStride1;
    piDst  += iDstStride;
  }
  return;
}
__inline Void TComPredFilter::xFilterBilinear(Pel* piSrc0, Int iSrcStride0, Int iSrcStep0, Int* piSrc1, Int iSrcStride1, Int iSrcStep1, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int tmp;
  for ( Int y = iHeight; y != 0; y-- )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      tmp = (piSrc0[x*iSrcStep0] + piSrc1[x*iSrcStep1] + 1) >> 1;
      piDst   [x * iDstStep] = tmp;
    }
    piSrc0 += iSrcStride0;
    piSrc1 += iSrcStride1;
    piDst  += iDstStride;
  }
  return;
}

 __inline Void  TComPredFilter::xFilterBilinearRect    (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Pel* piDst, Int iDstStride, Int iDstStep, Int px, Int py, Int s)
{
  Int		P00, P01, P10, P11, P;

  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      P00 = piSrc[x*iSrcStep + y*iSrcStride];
      P01 = piSrc[(x+1)*iSrcStep + y*iSrcStride];
      P10 = piSrc[x*iSrcStep + (y+1)*iSrcStride];
      P11 = piSrc[(x+1)*iSrcStep + (y+1)*iSrcStride];

      P = ((s - py) * ((s - px) * P00 + px * P01) +
       py       * ((s - px) * P10 + px * P11) + (s * s / 2)) / (s * s);

      piDst[x * iDstStep+y*iDstStride] = P;
    }
  }
  return;
}

// ------------------------------------------------------------------------------------------------
// DIF filters
// ------------------------------------------------------------------------------------------------

__inline Void TComPredFilter::xCTI_FilterHalfHor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterHalfHor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Int*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VIS06_HAL( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VIS12_HAL( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VIS[HAL_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU0_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Int*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU0_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI06_QU0( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI12_QU0( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI[QU0_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU1_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst    = rpiDst;
	Int   iSum;
	Int*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU1_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI06_QU1( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI12_QU1( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                   = xCTI_Filter_VI[QU1_IDX]( piSrcTmp, piFilter, iSrcStep );
				piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel )
{
	Int*  piDst = rpiDst;
	Pel*  piDstPel = rpiDstPel;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum									 = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep]		 = iSum;
				piDstPel[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
			piDstPel += iDstStridePel;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum									 = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep]		 = iSum;
				piDstPel[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
			piDstPel += iDstStridePel;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum									 = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep]		 = iSum;
				piDstPel[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
			piDstPel += iDstStridePel;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
	Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
	Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU0_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU0_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
	Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU1_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum								= xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum								= xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum								= xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = iSum;
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
	Pel*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][QU1_IDX];

	if ( m_iDIFTap == 6 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	if ( m_iDIFTap == 12 )
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	else
	{
		for ( Int y = iHeight; y != 0; y-- )
		{
			piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
			for ( Int x = 0; x < iWidth; x++ )
			{
				iSum                = xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStride );
				piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
				piSrcTmp += iSrcStep;
			}
			piSrc += iSrcStride;
			piDst += iDstStride;
		}
	}
	return;
}

// ------------------------------------------------------------------------------------------------
// DIF filters (for HAM)
// ------------------------------------------------------------------------------------------------

__inline Void TComPredFilter::xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMV)
{
  Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdxC][iMV+2];

	if ( m_iDIFTapC == 6 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum      = xCTI_Filter_VPC[iMV+2]( piSrcTmp, piFilter, iSrcStride );
			piDst[x ] = iSum;
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter2DHorC(Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
	Int*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdxC][iMV+2];

	if ( m_iDIFTapC == 6 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VIS06_HAL( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VI06_QU0( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VI06_QU1( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VIC[iMV+2]( piSrcTmp, piFilter, 1 );
      piDst   [x ] = Clip( (iSum +  32768) >>  16 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DVerC (Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdxC][iMV+2];

	if ( m_iDIFTapC == 6 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum      = xCTI_Filter_VPC[iMV+2]( piSrcTmp, piFilter, iSrcStride );
      piDst[x ] = Clip( (iSum +  128) >>  8 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DHorC(Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride, Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdxC][iMV+2];

	if ( m_iDIFTapC == 6 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VP06_QU0( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VPC[iMV+2]( piSrcTmp, piFilter, 1 );
      piDst   [x ] = Clip( (iSum +  128) >>  8 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter2DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMV)
{
  Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+2];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = iSum;
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum      = xCTI_Filter_VP[iMV+2]( piSrcTmp, piFilter, iSrcStride );
			piDst[x ] = iSum;
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter2DHor(Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
	Int*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+2];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VIS12_HAL( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VI12_QU0( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VI12_QU1( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  32768) >>  16 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VI[iMV+2]( piSrcTmp, piFilter, 1 );
      piDst   [x ] = Clip( (iSum +  32768) >>  16 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DVer (Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+2];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum      = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStride );
					piDst[x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum      = xCTI_Filter_VP[iMV+2]( piSrcTmp, piFilter, iSrcStride );
      piDst[x ] = Clip( (iSum +  128) >>  8 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DHor(Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride, Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+2];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+2 ) == HAL_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU0_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VP12_QU0( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
		else
		if ( ( iMV+2 ) == QU1_IDX )
		{
			for ( Int y = iHeight; y != 0; y-- )
			{
				piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
				for ( Int x = 0; x < iWidth; x++ )
				{
					iSum         = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, 1 );
					piDst   [x ] = Clip( (iSum +  128) >>  8 );
					piSrcTmp++;
				}
				piSrc += iSrcStride;
				piDst += iDstStride;
			}
			return;
		}
	}

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMargin ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VP[iMV+2]( piSrcTmp, piFilter, 1 );
      piDst   [x ] = Clip( (iSum +  128) >>  8 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

// ------------------------------------------------------------------------------------------------
// Optimized DIF filters
// ------------------------------------------------------------------------------------------------

// 4-tap
__inline Int TComPredFilter::xCTI_Filter_VPS04_HAL( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {   -32,  160,  160,  -32,}
	Int iSum, iTemp;
	iTemp = pSrc[iStride*1]+pSrc[iStride*2];
	iSum  = (iTemp-pSrc[0]-pSrc[iStride*3]) << 5;
	iSum += (iTemp) << 7;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VIS04_HAL( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {   -32,  160,  160,  -32,}
	Int iSum, iTemp;
	iTemp = pSrc[iStride*1]+pSrc[iStride*2];
	iSum  = (iTemp-pSrc[0]-pSrc[iStride*3]) << 5;
	iSum += (iTemp) << 7;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_QU0( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {   -24,  224,   72,  -16,},
	Int iSum, iIdx = 0;
	iSum  = (-pSrc[0] + pSrc[2*iStride]) << 3;
	iSum += (-pSrc[0] - pSrc[3*iStride]) << 4;
	iSum -= pSrc[iStride] << 5;
	iSum += pSrc[2*iStride] << 6;
	iSum += pSrc[iStride] << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_QU0( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {   -24,  224,   72,  -16,},
	Int iSum, iIdx = 0;
	iSum  = (-pSrc[0] + pSrc[2*iStride]) << 3;
	iSum += (-pSrc[0] - pSrc[3*iStride]) << 4;
	iSum -= pSrc[iStride] << 5;
	iSum += pSrc[2*iStride] << 6;
	iSum += pSrc[iStride] << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_QU1( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {   -16,   72,  224,  -24,},
	Int iSum, iIdx = 0;
	iSum  = (-pSrc[3*iStride] + pSrc[iStride]) << 3;
	iSum += (-pSrc[3*iStride] - pSrc[0]) << 4;
	iSum -= pSrc[2*iStride] << 5;
	iSum += pSrc[iStride] << 6;
	iSum += pSrc[2*iStride] << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_QU1( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {   -16,   72,  224,  -24,},
	Int iSum, iIdx = 0;
	iSum  = (-pSrc[3*iStride] + pSrc[iStride]) << 3;
	iSum += (-pSrc[3*iStride] - pSrc[0]) << 4;
	iSum -= pSrc[2*iStride] << 5;
	iSum += pSrc[iStride] << 6;
	iSum += pSrc[2*iStride] << 8;
	return iSum;
}

// 6-tap
__inline Int TComPredFilter::xCTI_Filter_VPS06_HAL( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -40,  160,  160,  -40,    8 }
	Int iSum, a0, a1, a2;
	a0 = pSrc[        0]+pSrc[iStride*5];
	a1 = pSrc[iStride*1]+pSrc[iStride*4];
	a2 = pSrc[iStride*2]+pSrc[iStride*3];

	iSum  = (a2 << 2) - a1;
	iSum += (iSum << 2);
	iSum += a0;
	iSum = (iSum << 3);
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VIS06_HAL( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -40,  160,  160,  -40,    8 }
	Int iSum, a0, a1, a2;
	a0 = pSrc[        0]+pSrc[iStride*5];
	a1 = pSrc[iStride*1]+pSrc[iStride*4];
	a2 = pSrc[iStride*2]+pSrc[iStride*3];

	iSum  = (a2 << 2) - a1;
	iSum += (iSum << 2);
	iSum += a0;
	iSum = (iSum << 3);
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP06_QU0( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -32,  224,   72,  -24,    8 },
	Int iSum;
	iSum  = (pSrc[0] + pSrc[3*iStride] + pSrc[4*iStride] + pSrc[5*iStride]) << 3;
	iSum -= (pSrc[iStride] + pSrc[2*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[3*iStride]) << 6;
	iSum += (pSrc[2*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI06_QU0( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -32,  224,   72,  -24,    8 },
	Int iSum;
	iSum  = (pSrc[0] + pSrc[3*iStride] + pSrc[4*iStride] + pSrc[5*iStride]) << 3;
	iSum -= (pSrc[iStride] + pSrc[2*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[3*iStride]) << 6;
	iSum += (pSrc[2*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP06_QU1( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -24,   72,  224,  -32,    8,},
	Int iSum;
	iSum  = (pSrc[0] + pSrc[2*iStride] + pSrc[iStride] + pSrc[5*iStride]) << 3;
	iSum -= (pSrc[iStride] + pSrc[3*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[2*iStride]) << 6;
	iSum += (pSrc[3*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI06_QU1( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {     8,  -24,   72,  224,  -32,    8,},
	Int iSum;
	iSum  = (pSrc[0] + pSrc[2*iStride] + pSrc[iStride] + pSrc[5*iStride]) << 3;
	iSum -= (pSrc[iStride] + pSrc[3*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[2*iStride]) << 6;
	iSum += (pSrc[3*iStride]) << 8;
	return iSum;
}

// 12-tap
__inline Int TComPredFilter::xCTI_Filter_VPS12_HAL( Pel* pSrc, Int* piCoeff, Int iStride )
{
	// {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 }
	Int iSum, a0, a1, a2, a3, a4, a5;
	a0 = pSrc[        0]+pSrc[iStride*11];
	a1 = pSrc[iStride*1]+pSrc[iStride*10];
	a2 = pSrc[iStride*2]+pSrc[iStride* 9];
	a3 = pSrc[iStride*3]+pSrc[iStride* 8];
	a4 = pSrc[iStride*4]+pSrc[iStride* 7];
	a5 = pSrc[iStride*5]+pSrc[iStride* 6];

	iSum  = (-a0 + a5);
	iSum += (a1 + a3) << 3;
	iSum += (-a2 + a3 - a4) << 4;
	iSum += (-a4 + a5) << 5;
	iSum += a5 << 7;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VIS12_HAL( Int* pSrc, Int* piCoeff, Int iStride )
{
	// {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 }
	Int iSum, a0, a1, a2, a3, a4, a5;
	a0 = pSrc[        0]+pSrc[iStride*11];
	a1 = pSrc[iStride*1]+pSrc[iStride*10];
	a2 = pSrc[iStride*2]+pSrc[iStride* 9];
	a3 = pSrc[iStride*3]+pSrc[iStride* 8];
	a4 = pSrc[iStride*4]+pSrc[iStride* 7];
	a5 = pSrc[iStride*5]+pSrc[iStride* 6];

	iSum  = (-a0 + a5);
	iSum += (a1 + a3) << 3;
	iSum += (-a2 + a3 - a4) << 4;
	iSum += (-a4 + a5) << 5;
	iSum += a5 << 7;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP12_QU0( Pel* pSrc, Int* piCoeff, Int iStride )
{

	//v.1.1
	// { -1, 5, -12, 20, -40, 229, 76, -32, 16, -8, 4, -1 }

	Int iSum, iTemp1, iTemp2;
	iTemp1 = pSrc[iStride] + pSrc[5*iStride];
	iTemp2 = -pSrc[2*iStride] + pSrc[6*iStride];
	iSum  = (-pSrc[0] + iTemp1 - pSrc[11*iStride]);
	iSum += (iTemp1 + iTemp2 + pSrc[3*iStride] + pSrc[10*iStride]) << 2;
	iSum += (iTemp2 - pSrc[4*iStride] - pSrc[9*iStride]) << 3;
	iSum += (pSrc[3*iStride] + pSrc[8*iStride]) << 4;
	iSum -= (pSrc[4*iStride] + pSrc[5*iStride] + pSrc[7*iStride]) << 5;
	iSum += (pSrc[6*iStride]) << 6;
	iSum += (pSrc[5*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI12_QU0( Int* pSrc, Int* piCoeff, Int iStride )
{

	//v.1.1
	// { -1, 5, -12, 20, -40, 229, 76, -32, 16, -8, 4, -1 }

	Int iSum, iTemp1, iTemp2;
	iTemp1 = pSrc[iStride] + pSrc[5*iStride];
	iTemp2 = -pSrc[2*iStride] + pSrc[6*iStride];
	iSum  = (-pSrc[0] + iTemp1 - pSrc[11*iStride]);
	iSum += (iTemp1 + iTemp2 + pSrc[3*iStride] + pSrc[10*iStride]) << 2;
	iSum += (iTemp2 - pSrc[4*iStride] - pSrc[9*iStride]) << 3;
	iSum += (pSrc[3*iStride] + pSrc[8*iStride]) << 4;
	iSum -= (pSrc[4*iStride] + pSrc[5*iStride] + pSrc[7*iStride]) << 5;
	iSum += (pSrc[6*iStride]) << 6;
	iSum += (pSrc[5*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP12_QU1( Pel* pSrc, Int* piCoeff, Int iStride )
{

	//v.1.1
	// {-1,  4,  -8,  16, -32,  76, 229, -40, 20, -12,  5, -1 },

	Int iSum, iTemp1, iTemp2;
	iTemp1 = pSrc[10*iStride] + pSrc[6*iStride];
	iTemp2 = -pSrc[9*iStride] + pSrc[5*iStride];
	iSum  = (-pSrc[11*iStride] + iTemp1 - pSrc[0]);
	iSum += (iTemp1 + iTemp2 + pSrc[8*iStride] + pSrc[iStride]) << 2;
	iSum += (iTemp2 - pSrc[7*iStride] - pSrc[2*iStride]) << 3;
	iSum += (pSrc[8*iStride] + pSrc[3*iStride]) << 4;
	iSum -= (pSrc[7*iStride] + pSrc[6*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[5*iStride]) << 6;
	iSum += (pSrc[6*iStride]) << 8;
	return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI12_QU1( Int* pSrc, Int* piCoeff, Int iStride )
{

	//v.1.1
	// {-1,  4,  -8,  16, -32,  76, 229, -40, 20, -12,  5, -1 },

	Int iSum, iTemp1, iTemp2;
	iTemp1 = pSrc[10*iStride] + pSrc[6*iStride];
	iTemp2 = -pSrc[9*iStride] + pSrc[5*iStride];
	iSum  = (-pSrc[11*iStride] + iTemp1 - pSrc[0]);
	iSum += (iTemp1 + iTemp2 + pSrc[8*iStride] + pSrc[iStride]) << 2;
	iSum += (iTemp2 - pSrc[7*iStride] - pSrc[2*iStride]) << 3;
	iSum += (pSrc[8*iStride] + pSrc[3*iStride]) << 4;
	iSum -= (pSrc[7*iStride] + pSrc[6*iStride] + pSrc[4*iStride]) << 5;
	iSum += (pSrc[5*iStride]) << 6;
	iSum += (pSrc[6*iStride]) << 8;
	return iSum;
}

#endif // __TCOMPREDFILTER__
