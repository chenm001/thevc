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

/** \file     TComPredFilter.h
    \brief    interpolation filter class (header)
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
#define HAL_IDX   1
#define QU0_IDX   0
#define QU1_IDX   2
// ====================================================================================================================
// Type definitions
// ====================================================================================================================

#if !DCTIF_8_6_LUMA
// define function pointers
typedef Int (*FpCTIFilter_VP) ( Pel* pSrc, Int* piCoeff, Int iStride );
typedef Int (*FpCTIFilter_VI) ( Int* pSrc, Int* piCoeff, Int iStride );

// filter coefficient array
extern Int CTI_Filter12 [5][3][12];
#endif
// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// interpolation filter class
class TComPredFilter
{
protected:
  // filter description (luma & chroma)
#if !DCTIF_8_6_LUMA
	Int   m_iDIFTap;
#endif
  
private:
  
 #if !DCTIF_8_6_LUMA
	// filter description (luma)
  Int   m_iTapIdx;
  Int   m_iLeftMargin;
  Int   m_iRightMargin;
  
  // filter functions
  FpCTIFilter_VP xCTI_Filter_VP  [14];  // Pel-type
  FpCTIFilter_VP xCTI_Filter_VPS [14];  // Pel-type, symmetric
  FpCTIFilter_VI xCTI_Filter_VI  [14];  // Int-type
  FpCTIFilter_VI xCTI_Filter_VIS [14];  // Int-type, symmetric
  
  // filter description (chroma)
  Int   m_iTapIdxC;
  Int   m_iLeftMarginC;
  Int   m_iRightMarginC;
  
  // filter functions
  FpCTIFilter_VP xCTI_Filter_VPC  [14]; // Pel-type
  FpCTIFilter_VP xCTI_Filter_VPSC [14]; // Pel-type, symmetric
  FpCTIFilter_VI xCTI_Filter_VIC  [14]; // Int-type
  FpCTIFilter_VI xCTI_Filter_VISC [14]; // Int-type, symmetric
#endif

public:
  TComPredFilter();
  
#if !DCTIF_8_6_LUMA
  Void  setDIFTap ( Int i );
#endif
  
  // DIF filter interface (for half & quarter)
#if TEN_DIRECTIONAL_INTERP
  __inline Void xCTI_FilterDIF_TEN(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int yFrac, Int xFrac);
#endif
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
  
#if DCTIF_4_6_CHROMA
  __inline Void xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMv);
  __inline Void xCTI_Filter2DHorC (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DHorC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);

   __inline Int xCTI_Filter_VPS04_C_HAL( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VIS04_C_HAL( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_OCT0( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_OCT0( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_QUA0( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_QUA0( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_OCT1( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_OCT1( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_OCT2( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_OCT2( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_QUA1( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_QUA1( Int* pSrc, Int iStride );
   __inline Int xCTI_Filter_VP04_C_OCT3( Pel* pSrc, Int iStride );
   __inline Int xCTI_Filter_VI04_C_OCT3( Int* pSrc, Int iStride );

#endif

#if HIGH_ACCURACY_BI
  __inline Void xCTI_FilterHalfHor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst); //
  __inline Void xCTI_FilterHalfHor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst);//
  
    
  __inline Void xCTI_FilterQuarter0Hor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst); //
  __inline Void xCTI_FilterQuarter0Hor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst); //
  
  __inline Void xCTI_FilterQuarter1Hor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst); //
  __inline Void xCTI_FilterQuarter1Hor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst); //
  
  __inline Void xCTI_FilterHalfVer_ha (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst ); //
  
  __inline Void xCTI_FilterQuarter0Ver_ha (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst );
  __inline Void xCTI_FilterQuarter1Ver_ha (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst );
  
  __inline Void xCTI_Filter1DHorC_ha (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVerC_ha (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter2DHorC_ha (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);

#endif
  
private:
#if !DCTIF_8_6_LUMA  
  // set of DIF filters
  static Int xCTI_Filter_VP04     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP06     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP08     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP10     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP12     ( Pel* pSrc, Int* piCoeff, Int iStride );
  
  static Int xCTI_Filter_VPS04    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS06    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS08    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS10    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS12    ( Pel* pSrc, Int* piCoeff, Int iStride );
  
  static Int xCTI_Filter_VI04     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI06     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI08     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI10     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI12     ( Int* pSrc, Int* piCoeff, Int iStride );
  
  static Int xCTI_Filter_VIS04    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS06    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS08    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS10    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS12    ( Int* pSrc, Int* piCoeff, Int iStride );
  
  // multiplication-free implementation
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
#endif
  
};

#if TEN_DIRECTIONAL_INTERP
__inline Void TComPredFilter::xCTI_FilterDIF_TEN(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int yFrac, Int xFrac)
{
  Pel*  piDst    = rpiDst;
  Pel*  piSrcTmp;
  Int pos = 4*yFrac+xFrac;
  
  switch ( pos )
  {
    case 0:
      for ( Int y=0; y<iHeight; y++ )
      {
        ::memcpy(piDst, piSrc, sizeof(Pel)*iWidth);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 1:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0] + (-15)*piSrcTmp[1] + 111*piSrcTmp[2] + 37*piSrcTmp[3] + (-10)*piSrcTmp[4] + 2*piSrcTmp[5] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 2:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0] + (-17)*piSrcTmp[1] + 78*piSrcTmp[2] + 78*piSrcTmp[3] + (-17)*piSrcTmp[4] + 3*piSrcTmp[5] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 3:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0] + (-10)*piSrcTmp[1] + 37*piSrcTmp[2] + 111*piSrcTmp[3] + (-15)*piSrcTmp[4] + 3*piSrcTmp[5] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 4:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride] + (-15)*piSrcTmp[1*iSrcStride] + 111*piSrcTmp[2*iSrcStride] + 37*piSrcTmp[3*iSrcStride] + (-10)*piSrcTmp[4*iSrcStride] + 2*piSrcTmp[5*iSrcStride] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 5:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride+0] + (-15)*piSrcTmp[1*iSrcStride+1] + 111*piSrcTmp[2*iSrcStride+2] + 37*piSrcTmp[3*iSrcStride+3] + (-10)*piSrcTmp[4*iSrcStride+4] + 2*piSrcTmp[5*iSrcStride+5] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 6:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride+0] + (-15)*piSrcTmp[1*iSrcStride+1] + 111*piSrcTmp[2*iSrcStride+2] + 37*piSrcTmp[3*iSrcStride+3] + (-10)*piSrcTmp[4*iSrcStride+4] + 2*piSrcTmp[5*iSrcStride+5] +
                                     3*piSrcTmp[0*iSrcStride+5] + (-15)*piSrcTmp[1*iSrcStride+4] + 111*piSrcTmp[2*iSrcStride+3] + 37*piSrcTmp[3*iSrcStride+2] + (-10)*piSrcTmp[4*iSrcStride+1] + 2*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 7:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride+5] + (-15)*piSrcTmp[1*iSrcStride+4] + 111*piSrcTmp[2*iSrcStride+3] + 37*piSrcTmp[3*iSrcStride+2] + (-10)*piSrcTmp[4*iSrcStride+1] + 2*piSrcTmp[5*iSrcStride+0] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 8:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride] + (-17)*piSrcTmp[1*iSrcStride] + 78*piSrcTmp[2*iSrcStride] + 78*piSrcTmp[3*iSrcStride] + (-17)*piSrcTmp[4*iSrcStride] + 3*piSrcTmp[5*iSrcStride] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 9:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (3*piSrcTmp[0*iSrcStride+0] + (-15)*piSrcTmp[1*iSrcStride+1] + 111*piSrcTmp[2*iSrcStride+2] +  37*piSrcTmp[3*iSrcStride+3] + (-10)*piSrcTmp[4*iSrcStride+4] + 2*piSrcTmp[5*iSrcStride+5] +
                                     2*piSrcTmp[0*iSrcStride+5] + (-10)*piSrcTmp[1*iSrcStride+4] +  37*piSrcTmp[2*iSrcStride+3] + 111*piSrcTmp[3*iSrcStride+2] + (-15)*piSrcTmp[4*iSrcStride+1] + 3*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 10:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-1*iSrcStride-1;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (0*piSrcTmp[0*iSrcStride+0] +  5*piSrcTmp[0*iSrcStride+1] +  5*piSrcTmp[0*iSrcStride+2] + 0*piSrcTmp[0*iSrcStride+3] +
                                     5*piSrcTmp[1*iSrcStride+0] + 22*piSrcTmp[1*iSrcStride+1] + 22*piSrcTmp[1*iSrcStride+2] + 5*piSrcTmp[1*iSrcStride+3] +
                                     5*piSrcTmp[2*iSrcStride+0] + 22*piSrcTmp[2*iSrcStride+1] + 22*piSrcTmp[2*iSrcStride+2] + 5*piSrcTmp[2*iSrcStride+3] +
                                     0*piSrcTmp[3*iSrcStride+0] +  5*piSrcTmp[3*iSrcStride+1] +  5*piSrcTmp[3*iSrcStride+2] + 0*piSrcTmp[3*iSrcStride+3] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 11:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride+0] + (-10)*piSrcTmp[1*iSrcStride+1] +  37*piSrcTmp[2*iSrcStride+2] + 111*piSrcTmp[3*iSrcStride+3] + (-15)*piSrcTmp[4*iSrcStride+4] + 3*piSrcTmp[5*iSrcStride+5] +
                                     3*piSrcTmp[0*iSrcStride+5] + (-15)*piSrcTmp[1*iSrcStride+4] + 111*piSrcTmp[2*iSrcStride+3] + 37*piSrcTmp[3*iSrcStride+2] + (-10)*piSrcTmp[4*iSrcStride+1] + 2*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 12:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride] + (-10)*piSrcTmp[1*iSrcStride] + 37*piSrcTmp[2*iSrcStride] + 111*piSrcTmp[3*iSrcStride] + (-15)*piSrcTmp[4*iSrcStride] + 3*piSrcTmp[5*iSrcStride] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 13:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride+5] + (-10)*piSrcTmp[1*iSrcStride+4] + 37*piSrcTmp[2*iSrcStride+3] + 111*piSrcTmp[3*iSrcStride+2] + (-15)*piSrcTmp[4*iSrcStride+1] + 3*piSrcTmp[5*iSrcStride+0] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 14:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride+0] + (-10)*piSrcTmp[1*iSrcStride+1] +  37*piSrcTmp[2*iSrcStride+2] + 111*piSrcTmp[3*iSrcStride+3] + (-15)*piSrcTmp[4*iSrcStride+4] + 3*piSrcTmp[5*iSrcStride+5] +
                                     2*piSrcTmp[0*iSrcStride+5] + (-10)*piSrcTmp[1*iSrcStride+4] +  37*piSrcTmp[2*iSrcStride+3] + 111*piSrcTmp[3*iSrcStride+2] + (-15)*piSrcTmp[4*iSrcStride+1] + 3*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 15:
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride+0] + (-10)*piSrcTmp[1*iSrcStride+1] + 37*piSrcTmp[2*iSrcStride+2] + 111*piSrcTmp[3*iSrcStride+3] + (-15)*piSrcTmp[4*iSrcStride+4] + 3*piSrcTmp[5*iSrcStride+5] + 64)>>7);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    default:
      assert (0);
      break;
  }
  return;
}
#endif

#if HIGH_ACCURACY_BI
__inline Void TComPredFilter::xCTI_FilterHalfHor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[        0]+piSrcTmp[iSrcStep7];
      iTmp1 = piSrcTmp[iSrcStep]+piSrcTmp[iSrcStep6];
      iTmp2 = piSrcTmp[iSrcStep2]+piSrcTmp[iSrcStep5];
      iTmp3 = piSrcTmp[iSrcStep3]+piSrcTmp[iSrcStep4];

      iTmpA = (iTmp3 << 2) - iTmp2;

      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;

      piDst   [x * iDstStep] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}

__inline Void TComPredFilter::xCTI_FilterHalfHor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
 
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  Int shiftNum = 6 + g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[        0]+piSrcTmp[iSrcStep7];
      iTmp1 = piSrcTmp[iSrcStep ]+piSrcTmp[iSrcStep6];
      iTmp2 = piSrcTmp[iSrcStep2]+piSrcTmp[iSrcStep5];
      iTmp3 = piSrcTmp[iSrcStep3]+piSrcTmp[iSrcStep4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;
      
      piDst   [x * iDstStep] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}


__inline Void TComPredFilter::xCTI_FilterQuarter0Hor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int  iTmp1, iTmp2;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep3] + piSrcTmp[iSrcStep5];
      iTmp2 = piSrcTmp[iSrcStep6] + piSrcTmp[iSrcStep4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
             - ( ( piSrcTmp[iSrcStep2] - iTmp2 ) << 1 )
             + (  piSrcTmp[iSrcStep]             << 2 )
             - ( ( piSrcTmp[iSrcStep2] + iTmp1 ) << 3 )
             + (   piSrcTmp[iSrcStep4]           << 4 )
			 + ( piSrcTmp[iSrcStep3]             << 6);
      
      piDst   [x * iDstStep] = Clip3(0,16383, (iSum + shiftOffset) >> shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;


}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
  
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;
  Int shiftNum = 6 + g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
    
      iTmp1 = piSrcTmp[iSrcStep3] + piSrcTmp[iSrcStep5];
      iTmp2 = piSrcTmp[iSrcStep6] + piSrcTmp[iSrcStep4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
      	- ( ( piSrcTmp[iSrcStep2] - iTmp2 ) << 1 )
      	+ (  piSrcTmp[iSrcStep]             << 2 )
      	- ( ( piSrcTmp[iSrcStep2] + iTmp1 ) << 3 )
      	+ (   piSrcTmp[iSrcStep4]           << 4 )
      	+ (   piSrcTmp[iSrcStep3]           << 6 );
    
      piDst   [x * iDstStep] = Clip3(0, 16383, (iSum +  shiftOffset) >>  shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}


__inline Void TComPredFilter::xCTI_FilterQuarter1Hor_ha(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;

  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep4] + piSrcTmp[iSrcStep2];
      iTmp2 = piSrcTmp[iSrcStep ] + piSrcTmp[iSrcStep3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
            - ( ( piSrcTmp[iSrcStep5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStep6]           << 2 )
            - ( ( piSrcTmp[iSrcStep5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStep3]           << 4 )
			+ (   piSrcTmp[iSrcStep4]           << 6 );
			;
      
      piDst   [x * iDstStep] = Clip3(0,16383, (iSum + shiftOffset) >> shiftNum);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;


}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor_ha(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;

  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;
  Int shiftNum = 6+g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep4] + piSrcTmp[iSrcStep2];
      iTmp2 = piSrcTmp[iSrcStep ] + piSrcTmp[iSrcStep3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
            - ( ( piSrcTmp[iSrcStep5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStep6]           << 2 )
            - ( ( piSrcTmp[iSrcStep5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStep3]           << 4 )
            + (   piSrcTmp[iSrcStep4]           << 6 );
      
      piDst   [x * iDstStep] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}



__inline Void TComPredFilter::xCTI_FilterQuarter0Ver_ha (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStride3] + piSrcTmp[iSrcStride5];
      iTmp2 = piSrcTmp[iSrcStride6] + piSrcTmp[iSrcStride4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride2] - iTmp2 ) << 1 )
            + (  piSrcTmp[iSrcStride]             << 2 )
            - ( ( piSrcTmp[iSrcStride2] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride4]           << 4 )
			+ (   piSrcTmp[iSrcStride3]           << 6);
      
      piDst[x * iDstStep] = Clip3(0,16383, (iSum + shiftOffset)>>shiftNum);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}


__inline Void TComPredFilter::xCTI_FilterQuarter1Ver_ha (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      /// {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      iTmp1 = piSrcTmp[iSrcStride4] + piSrcTmp[iSrcStride2];
      iTmp2 = piSrcTmp[iSrcStride ] + piSrcTmp[iSrcStride3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStride6]           << 2 )
            - ( ( piSrcTmp[iSrcStride5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride3]           << 4 )
			+ (   piSrcTmp[iSrcStride4]           << 6 );
            
      piDst[x * iDstStep] = Clip3(0,16383, (iSum + shiftOffset)>>shiftNum);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}

__inline Void TComPredFilter::xCTI_FilterHalfVer_ha	 (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;
  Int shiftNum = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
  Int  iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[          0]+piSrcTmp[iSrcStride7];
      iTmp1 = piSrcTmp[iSrcStride ]+piSrcTmp[iSrcStride6];
      iTmp2 = piSrcTmp[iSrcStride2]+piSrcTmp[iSrcStride5];
      iTmp3 = piSrcTmp[iSrcStride3]+piSrcTmp[iSrcStride4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;        
      
      piDst[x * iDstStep] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;

}


#endif

// ------------------------------------------------------------------------------------------------
// DCTIF filters
// ------------------------------------------------------------------------------------------------

__inline Void TComPredFilter::xCTI_FilterHalfHor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[        0]+piSrcTmp[iSrcStep7];
      iTmp1 = piSrcTmp[iSrcStep]+piSrcTmp[iSrcStep6];
      iTmp2 = piSrcTmp[iSrcStep2]+piSrcTmp[iSrcStep5];
      iTmp3 = piSrcTmp[iSrcStep3]+piSrcTmp[iSrcStep4];

      iTmpA = (iTmp3 << 2) - iTmp2;

      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;

      piDst   [x * iDstStep] = Clip( (iSum +  32) >>  6 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else
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
#endif
}

__inline Void TComPredFilter::xCTI_FilterHalfHor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
#if DCTIF_8_6_LUMA  
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[        0]+piSrcTmp[iSrcStep7];
      iTmp1 = piSrcTmp[iSrcStep ]+piSrcTmp[iSrcStep6];
      iTmp2 = piSrcTmp[iSrcStep2]+piSrcTmp[iSrcStep5];
      iTmp3 = piSrcTmp[iSrcStep3]+piSrcTmp[iSrcStep4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;
      
      piDst   [x * iDstStep] = Clip( (iSum +  2048) >>  12 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else 
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
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA 
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep3] + piSrcTmp[iSrcStep5];
      iTmp2 = piSrcTmp[iSrcStep6] + piSrcTmp[iSrcStep4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
             - ( ( piSrcTmp[iSrcStep2] - iTmp2 ) << 1 )
             + (  piSrcTmp[iSrcStep]             << 2 )
             - ( ( piSrcTmp[iSrcStep2] + iTmp1 ) << 3 )
             + (   piSrcTmp[iSrcStep4]           << 4 );
      
      piDst   [x * iDstStep] = Clip(( (iSum +  32) >>  6 )+ piSrcTmp[iSrcStep3]);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else
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
#endif

}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
#if DCTIF_8_6_LUMA  
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
    
      iTmp1 = piSrcTmp[iSrcStep3] + piSrcTmp[iSrcStep5];
      iTmp2 = piSrcTmp[iSrcStep6] + piSrcTmp[iSrcStep4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
      	- ( ( piSrcTmp[iSrcStep2] - iTmp2 ) << 1 )
      	+ (  piSrcTmp[iSrcStep]             << 2 )
      	- ( ( piSrcTmp[iSrcStep2] + iTmp1 ) << 3 )
      	+ (   piSrcTmp[iSrcStep4]           << 4 )
      	+ (   piSrcTmp[iSrcStep3]           << 6 );
    
      piDst   [x * iDstStep] = Clip( (iSum +  2048) >>  12 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep4] + piSrcTmp[iSrcStep2];
      iTmp2 = piSrcTmp[iSrcStep ] + piSrcTmp[iSrcStep3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
            - ( ( piSrcTmp[iSrcStep5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStep6]           << 2 )
            - ( ( piSrcTmp[iSrcStep5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStep3]           << 4 );
      
      piDst   [x * iDstStep] = Clip( ((iSum +  32) >>  6) + piSrcTmp[iSrcStep4] );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else
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
  {
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
  }
  return;
#endif

}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
#if DCTIF_8_6_LUMA  
  Int iSrcStep2 = iSrcStep*2;
  Int iSrcStep3 = iSrcStep*3;
  Int iSrcStep4 = iSrcStep*4;
  Int iSrcStep5 = iSrcStep*5;
  Int iSrcStep6 = iSrcStep*6;
  Int iSrcStep7 = iSrcStep*7;

  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStep4] + piSrcTmp[iSrcStep2];
      iTmp2 = piSrcTmp[iSrcStep ] + piSrcTmp[iSrcStep3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStep7]
            - ( ( piSrcTmp[iSrcStep5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStep6]           << 2 )
            - ( ( piSrcTmp[iSrcStep5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStep3]           << 4 )
            + (   piSrcTmp[iSrcStep4]           << 6 );
      
      piDst   [x * iDstStep] = Clip( (iSum +  2048) >>  12 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else  
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel )
{
  Int*  piDst = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[          0]+piSrcTmp[iSrcStride7];
      iTmp1 = piSrcTmp[iSrcStride ]+piSrcTmp[iSrcStride6];
      iTmp2 = piSrcTmp[iSrcStride2]+piSrcTmp[iSrcStride5];
      iTmp3 = piSrcTmp[iSrcStride3]+piSrcTmp[iSrcStride4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;
      
      piDst[x * iDstStep]    = iSum;
      piDstPel[x * iDstStep] = Clip( (iSum +  32) >>  6 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
    piDstPel += iDstStridePel;
  }
 return;
#else  
  Int*  piFilter = CTI_Filter12[m_iTapIdx][HAL_IDX];
  
  if ( m_iDIFTap == 6 )
  {
    for ( Int y = iHeight; y != 0; y-- )
    {
      piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
      for ( Int x = 0; x < iWidth; x++ )
      {
        iSum                   = xCTI_Filter_VPS06_HAL( piSrcTmp, piFilter, iSrcStride );
        piDst[x * iDstStep]    = iSum;
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
    if ( m_iDIFTap == 12 )
    {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum                   = xCTI_Filter_VPS12_HAL( piSrcTmp, piFilter, iSrcStride );
          piDst[x * iDstStep]    = iSum;
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
          iSum                   = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
          piDst[x * iDstStep]    = iSum;
          piDstPel[x * iDstStep] = Clip( (iSum +  128) >>  8 );
          piSrcTmp += iSrcStep;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
        piDstPel += iDstStridePel;
      }
    }
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[          0]+piSrcTmp[iSrcStride7];
      iTmp1 = piSrcTmp[iSrcStride ]+piSrcTmp[iSrcStride6];
      iTmp2 = piSrcTmp[iSrcStride2]+piSrcTmp[iSrcStride5];
      iTmp3 = piSrcTmp[iSrcStride3]+piSrcTmp[iSrcStride4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;        
      
      piDst[x * iDstStep] = iSum;
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else   
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp0, iTmp1, iTmp2, iTmp3, iTmpA;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // { -1,4,-11,40,40,-11,4,-1   } 
      iTmp0 = piSrcTmp[          0]+piSrcTmp[iSrcStride7];
      iTmp1 = piSrcTmp[iSrcStride ]+piSrcTmp[iSrcStride6];
      iTmp2 = piSrcTmp[iSrcStride2]+piSrcTmp[iSrcStride5];
      iTmp3 = piSrcTmp[iSrcStride3]+piSrcTmp[iSrcStride4];
      
      iTmpA = (iTmp3 << 2) - iTmp2;
      
      iSum  = (   iTmp1          << 2 )
            + (   iTmpA          << 3 )
            + (   iTmpA          << 1 )
            -    iTmp0 -  iTmp2;        
      
      piDst[x * iDstStep] = Clip( (iSum +  32) >>  6 );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else 
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStride3] + piSrcTmp[iSrcStride5];
      iTmp2 = piSrcTmp[iSrcStride6] + piSrcTmp[iSrcStride4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride2] - iTmp2 ) << 1 )
            + (  piSrcTmp[iSrcStride]             << 2 )
            - ( ( piSrcTmp[iSrcStride2] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride4]           << 4 )
            + (   piSrcTmp[iSrcStride3]           << 6 );
      
      piDst[x * iDstStep] = iSum;
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else   
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp1, iTmp2;
  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      // {-1,   4,  -10,  57,   19,  -7,   3,   -1  },
      
      iTmp1 = piSrcTmp[iSrcStride3] + piSrcTmp[iSrcStride5];
      iTmp2 = piSrcTmp[iSrcStride6] + piSrcTmp[iSrcStride4];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride2] - iTmp2 ) << 1 )
            + (  piSrcTmp[iSrcStride]             << 2 )
            - ( ( piSrcTmp[iSrcStride2] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride4]           << 4 );
      
      piDst[x * iDstStep] = Clip( ((iSum +  32) >>  6) + piSrcTmp[iSrcStride3] );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else 
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
  {
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
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      /// {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      iTmp1 = piSrcTmp[iSrcStride4] + piSrcTmp[iSrcStride2];
      iTmp2 = piSrcTmp[iSrcStride ] + piSrcTmp[iSrcStride3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStride6]           << 2 )
            - ( ( piSrcTmp[iSrcStride5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride3]           << 4 )
            + (   piSrcTmp[iSrcStride4]           << 6 );
            
      piDst[x * iDstStep] = iSum;
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else   
  Int*  piFilter = CTI_Filter12[m_iTapIdx][QU1_IDX];
  
  if ( m_iDIFTap == 6 )
  {
    for ( Int y = iHeight; y != 0; y-- )
    {
      piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
      for ( Int x = 0; x < iWidth; x++ )
      {
        iSum                = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
        piDst[x * iDstStep] = iSum;
        piSrcTmp += iSrcStep;
      }
      piSrc += iSrcStride;
      piDst += iDstStride;
    }
  }
  else
  {
    if ( m_iDIFTap == 12 )
    {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum                = xCTI_Filter_VP12_QU1( piSrcTmp, piFilter, iSrcStride );
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
          iSum                = xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStride );
          piDst[x * iDstStep] = iSum;
          piSrcTmp += iSrcStep;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
    }
  }
  return;
#endif
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
#if DCTIF_8_6_LUMA
  Int iSrcStride2 = iSrcStride*2;
  Int iSrcStride3 = iSrcStride*3;
  Int iSrcStride4 = iSrcStride*4;
  Int iSrcStride5 = iSrcStride*5;
  Int iSrcStride6 = iSrcStride*6;
  Int iSrcStride7 = iSrcStride*7;

  Int  iTmp1, iTmp2;

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -3*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      /// {-1,   3,  -7,  19,   57,  -10,   4,   -1  },
      iTmp1 = piSrcTmp[iSrcStride4] + piSrcTmp[iSrcStride2];
      iTmp2 = piSrcTmp[iSrcStride ] + piSrcTmp[iSrcStride3];
      
      iSum  =  iTmp1 + iTmp2 - piSrcTmp[0] - piSrcTmp[iSrcStride7]
            - ( ( piSrcTmp[iSrcStride5] - iTmp2 ) << 1 )
            + (   piSrcTmp[iSrcStride6]           << 2 )
            - ( ( piSrcTmp[iSrcStride5] + iTmp1 ) << 3 )
            + (   piSrcTmp[iSrcStride3]           << 4 );
            
      piDst[x * iDstStep] = Clip( ((iSum +  32) >>  6) +  piSrcTmp[iSrcStride4] );
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
#else  
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
  {
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
  }
  return;
#endif
}




// ------------------------------------------------------------------------------------------------
// Optimized DCTIF filters
// ------------------------------------------------------------------------------------------------
#if !DCTIF_8_6_LUMA
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

#endif
// ------------------------------------------------------------------------------------------------
// DCTIF filters for Chroma
// ------------------------------------------------------------------------------------------------
#if DCTIF_4_6_CHROMA
__inline Void TComPredFilter::xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMV)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT0( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA0( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:  
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA1( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:  
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT1( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT2( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT3( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4: 
  {

      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VPS04_C_HAL( piSrcTmp, iSrcStride );
          piDst[x ] = iSum;
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter2DHorC(Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT0( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_QUA0( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_QUA1( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT1( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT2( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT3( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
			iSum      = xCTI_Filter_VIS04_C_HAL( piSrcTmp, 1 );
          piDst   [x ] = Clip ((iSum +  2048) >>  12 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
  }

  return;
}

__inline Void TComPredFilter::xCTI_Filter1DVerC (Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;	

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT0( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA0( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA1( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT1( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT2( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT3( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[-iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VPS04_C_HAL( piSrcTmp, iSrcStride );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
  }
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DHorC(Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride, Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT0( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_QUA0( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_QUA1( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT1( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT2( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT3( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
			iSum         = xCTI_Filter_VPS04_C_HAL( piSrcTmp,  1 );
          piDst[x ] = Clip ((iSum +  32) >>  6 );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
 }
  return;
}

#if HIGH_ACCURACY_BI
__inline Void TComPredFilter::xCTI_Filter2DHorC_ha(Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
  Int shiftNum  = 6 + g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT0( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_QUA0( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_QUA1( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT1( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT2( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VI04_C_OCT3( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
			iSum      = xCTI_Filter_VIS04_C_HAL( piSrcTmp, 1 );
          piDst   [x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
  }

  return;
}

__inline Void TComPredFilter::xCTI_Filter1DVerC_ha (Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;	
  Int shiftNum  = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;
    
  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT0( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA0( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA1( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT1( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT2( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT3( piSrcTmp,  iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[-iSrcStride ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VPS04_C_HAL( piSrcTmp, iSrcStride );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );
  }    
  return;
}

__inline Void TComPredFilter::xCTI_Filter1DHorC_ha(Pel* piSrc, Int iSrcStride, Int iWidth, Int iHeight, Int iDstStride, Pel*& rpiDst, Int iMV)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int shiftNum  = g_uiBitIncrement + g_uiBitDepth - 8;
  Int shiftOffset = (shiftNum > 0) ? ( 1 << (shiftNum - 1)) : 0 ;

  switch (iMV)
  {
  case 1:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT0( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 2:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_QUA0( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 6:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_QUA1( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 3:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum      = xCTI_Filter_VP04_C_OCT1( piSrcTmp,  1 );
          piDst[x ] = Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 5:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT2( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 7:
  {  
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
          iSum         = xCTI_Filter_VP04_C_OCT3( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  case 4:
  {
      for ( Int y = iHeight; y != 0; y-- )
      {
        piSrcTmp = &piSrc[ -1 ];
        for ( Int x = 0; x < iWidth; x++ )
        {
			iSum    = xCTI_Filter_VPS04_C_HAL( piSrcTmp,  1 );
          piDst[x ] =  Clip3(0,16383, (iSum +  shiftOffset) >>  shiftNum );
          piSrcTmp++;
        }
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
  }
  break;
  default:
    assert( 0 );

 }
  return;
}
#endif


__inline Int TComPredFilter::xCTI_Filter_VP04_C_OCT0( Pel* pSrc,  Int iStride )
{// {  -3,  60,   8,   -1,} // 1/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p1<<6) -((p1+p0)<<2) +p0 +(p2<<3) -p3;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_OCT0( Int* pSrc, Int iStride )
{ // {  -3,  60,   8,   -1,} //1/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p1<<6) -((p1+p0)<<2) +p0 +(p2<<3) -p3;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_C_QUA0( Pel* pSrc,  Int iStride )
{// {  -4,  54,  16,   -2,} // 1/4
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p1 << 6) + (p2 << 4) - (p1 << 3) - ( p0 << 2) - ((p1 + p3) << 1);

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_QUA0( Int* pSrc, Int iStride )
{ // {  -4,  54,  16,   -2,} //1/4
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p1 << 6) + (p2 << 4) - (p1 << 3) - ( p0 << 2) - ((p1 + p3) << 1);

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_C_QUA1( Pel* pSrc,  Int iStride )
{// {  -2,  16,  54,   -4,}// 3/4
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p2 << 6) + (p1 << 4) - (p2 << 3) - ( p3 << 2) - ((p2 + p0) << 1);

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_QUA1( Int* pSrc, Int iStride )
{// {  -2,  16,  54,   -4,}// 3/4
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p2 << 6) + (p1 << 4) - (p2 << 3) - ( p3 << 2) - ((p2 + p0) << 1);

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_C_OCT1( Pel* pSrc,  Int iStride )
{// {  -5,  46,  27,   -4,} // 3/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  Int t = p0 + p2;
  iSum = ((p1 + p2) << 5) + (p1 << 4) - ( (t + p3) << 2) - ( p1 << 1) - t;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_OCT1( Int* pSrc, Int iStride )
{ // {  -5,  46,  27,   -4,} //3/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  Int t = p0 + p2;
  iSum = ((p1 + p2) << 5) + (p1 << 4) - ( (t + p3) << 2) - ( p1 << 1) - t;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VPS04_C_HAL( Pel* pSrc, Int iStride )
{
	// {  -4,  36,  36,   -4,}, // 1/2
  Int iSum;
  Int iTemp0 = pSrc[iStride*1]+pSrc[iStride*2];
  Int iTemp1 = pSrc[        0]+pSrc[iStride*3];

  iSum  = ((iTemp0<<3) + iTemp0 -iTemp1)<<2;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VIS04_C_HAL( Int* pSrc, Int iStride )
{
	// {  -4,  36,  36,   -4,}, //1/2
  Int iSum;
  Int iTemp0 = pSrc[iStride*1]+pSrc[iStride*2];
  Int iTemp1 = pSrc[        0]+pSrc[iStride*3];

  iSum  = ((iTemp0<<3) + iTemp0 -iTemp1)<<2;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_C_OCT2( Pel* pSrc,  Int iStride )
{// {  -4,  27,  46,   -5,}, // 5/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  Int t = p1 + p3;
  iSum = ((p1 + p2) << 5) + (p2 << 4) - ( (t + p0) << 2) - ( p2 << 1) - t;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_OCT2( Int* pSrc, Int iStride )
{ // {  -4,  27,  46,   -5,}, // 5/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  Int t = p1 + p3;
  iSum = ((p1 + p2) << 5) + (p2 << 4) - ( (t + p0) << 2) - ( p2 << 1) - t;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VP04_C_OCT3( Pel* pSrc,  Int iStride )
{// {  -1,   8,  60,   -3,} // 7/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p2<<6) -((p2+p3)<<2) +p3 +(p1<<3) -p0;

  return iSum;
}
__inline Int TComPredFilter::xCTI_Filter_VI04_C_OCT3( Int* pSrc, Int iStride )
{ // {  -1,   8,  60,   -3,} // 7/8
  Int iSum, iIdx = 0;

  Int p0 = pSrc[0];     iIdx+= iStride;
  Int p1 = pSrc[iIdx];  iIdx+= iStride;
  Int p2 = pSrc[iIdx];  iIdx+= iStride;
  Int p3 = pSrc[iIdx];  
  iSum = (p2<<6) -((p2+p3)<<2) +p3 +(p1<<3) -p0;

  return iSum;
}



#endif

#endif // __TCOMPREDFILTER__
