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

// define function pointers
typedef Int (*FpCTIFilter_VP) ( Pel* pSrc, Int* piCoeff, Int iStride );
typedef Int (*FpCTIFilter_VI) ( Int* pSrc, Int* piCoeff, Int iStride );

// filter coefficient array
extern Int CTI_Filter12 [5][3][12];

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// interpolation filter class
class TComPredFilter
{
protected:
  // filter description (luma & chroma)
  Int   m_iDIFTap;
  
private:
  
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
  
public:
  TComPredFilter();
  
  Void  setDIFTap ( Int i );
  
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
  
private:
  
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
}

// ------------------------------------------------------------------------------------------------
// Optimized DIF filters
// ------------------------------------------------------------------------------------------------

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
