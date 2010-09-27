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
#ifdef QC_AMVRES
#define HAL_IDX   3
#define QU0_IDX   1
#define QU1_IDX   5
#else
#define HAL_IDX   1
#define QU0_IDX   0
#define QU1_IDX   2
#endif
// ====================================================================================================================
// Type definitions
// ====================================================================================================================

// define function pointers
typedef Int (*FpCTIFilter_VP) ( Pel* pSrc, Int* piCoeff, Int iStride );
typedef Int (*FpCTIFilter_VI) ( Int* pSrc, Int* piCoeff, Int iStride );

// filter coefficient array
#ifdef QC_AMVRES
extern Int CTI_Filter12 [5][7][12];
#else
extern Int CTI_Filter12 [5][3][12];
#endif
#ifdef QC_SIFO
#ifdef QC_AMVRES
extern Int SIFO_Filter6 [4][7][6];
extern Int SIFO_Filter12[4][7][12];
#else
extern Int SIFO_Filter6 [4][3][6];
extern Int SIFO_Filter12[4][3][12];
#endif
#endif

#if SAMSUNG_CHROMA_IF_EXT
extern Int CTI_Filter12_C [5][7][12];
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// interpolation filter class
class TComPredFilter
{
protected:
  // filter description (luma & chroma)
  Int   m_iDIFTap;

#if SAMSUNG_CHROMA_IF_EXT
  Int	m_iDIFTapC;
#endif

#ifdef QC_SIFO
  Int    m_aiFilterSequence[16], m_aiPrevPFrameFilter[16],m_aiPrevBFrameFilter[16];
  UInt   m_uiNum_AvailableFilters;
  UInt   m_uiNum_SIFOFilters;
  UInt   m_uiNUM_SIFO_TAB[16];
  Int    m_aiTabFilters[16][16][2];   //16 subpels, max there are 4 different 1D filters, combining any 2 filters out of these 4 can create 16 different 2D filters.
  Int    m_iBestFilterP,m_iBestFilterB;
  UInt   m_uiPredictFilterP;
  UInt   m_uiPredictFilterB;
  UInt   m_auiPredictFilterSequenceP[16];   //contains 0 or 1...0 = code the subpel filter, 1 = use prev frame subpel filter               
  UInt   m_auiPredictFilterSequenceB[16];   //contains 0 or 1...0 = code the subpel filter, 1 = use prev frame subpel filter               
 Int m_aiSubpelOffset[2][16]; //2 list and 16 subpels
 Int m_aiFrameOffset[2][MAX_REF_PIC_NUM];       
 Int m_aiOffset_FullpelME[2][16];// Int offsetMETab[2][16];
 Int m_aiNumOffset_FullpelME[2];
 Int m_iCurrRefFrame;
 Int m_iCurrList;
#endif

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
#if SAMSUNG_CHROMA_IF_EXT
  Void  setDIFTapC( Int i );
#endif

#if SIFO_DIF_COMPATIBILITY==1
  Int   getDIFTap() {return m_iDIFTap;}
#endif
#ifdef QC_SIFO
  Void  setSIFOFilter       (Int Val, Int i)      { m_aiFilterSequence  [i] = Val;  }
  Void  setPrevP_SIFOFilter (Int Val, Int i)      { m_aiPrevPFrameFilter[i] = Val;  }
  Void  setPrevB_SIFOFilter (Int Val, Int i)      { m_aiPrevBFrameFilter[i] = Val;  }
  Int   getSIFOFilter       (Int i)               { return m_aiFilterSequence  [i]; }
  Int   getPrevP_SIFOFilter (Int i)               { return m_aiPrevPFrameFilter[i]; }
  Int   getPrevB_SIFOFilter (Int i)               { return m_aiPrevBFrameFilter[i]; }

  Void  setNum_AvailableFilters (UInt Val)        { m_uiNum_AvailableFilters = Val;  }
  UInt  getNum_AvailableFilters ()               { return m_uiNum_AvailableFilters; }

  Void  setNum_SIFOFilters (UInt Val)        { m_uiNum_SIFOFilters = Val;  }
  UInt  getNum_SIFOFilters ()                { return m_uiNum_SIFOFilters; }

  UInt  getNum_SIFOTab (Int i)            { return m_uiNUM_SIFO_TAB[i]; }
  
  Void  setTabFilters  (Int Val, Int subpel, Int filter, Int i)   { m_aiTabFilters[subpel][filter][i] = Val ; }
  Int   getTabFilters  (Int subpel, Int filter, Int i)            { return m_aiTabFilters[subpel][filter][i]; }

  Void  setBestFilter_P  (Int Val)      { m_iBestFilterP = Val; }
  Void  setBestFilter_B  (Int Val)      { m_iBestFilterB = Val; }
  Int   getBestFilter_P  ()             { return m_iBestFilterP; }
  Int   getBestFilter_B  ()             { return m_iBestFilterB; }


  Void  setPredictFilterP (UInt Val)        { m_uiPredictFilterP = Val;  }
  Void  setPredictFilterB (UInt Val)        { m_uiPredictFilterB = Val;  }
  UInt  getPredictFilterP ()                { return m_uiPredictFilterP; }
  UInt  getPredictFilterB ()                { return m_uiPredictFilterB; }

  Void  setPredictFilterSequenceP (Int Val, Int i)      { m_auiPredictFilterSequenceP[i] = Val;  }
  Int   getPredictFilterSequenceP (Int i)               { return m_auiPredictFilterSequenceP[i]; }
  Void  setPredictFilterSequenceB (Int Val, Int i)      { m_auiPredictFilterSequenceB[i] = Val;  }
  Int   getPredictFilterSequenceB (Int i)               { return m_auiPredictFilterSequenceB[i]; }

  Void  setSubpelOffset       (Int Val, Int list, Int subpel)      { m_aiSubpelOffset[list][subpel] = Val;  }
  Int   getSubpelOffset       (Int list, Int subpel)               { return m_aiSubpelOffset[list][subpel]; }
  Void  setFrameOffset       (Int Val, Int list, Int frame)        { m_aiFrameOffset[list][frame] = Val;  }
  Int   getFrameOffset       (Int list, Int frame)                 { return m_aiFrameOffset[list][frame]; }
  Int   getSIFOOffset        (Int list, Int subpel, Int frame)     { return (frame ? m_aiFrameOffset[list][frame] : m_aiSubpelOffset[list][subpel]); }
  Void  setOffsets_toZero    ()
  { 
      memset(m_aiSubpelOffset[0]  , 0, 16 * sizeof(Int));
      memset(m_aiSubpelOffset[1]  , 0, 16 * sizeof(Int));
      memset(m_aiFrameOffset[0]   , 0, MAX_REF_PIC_NUM * sizeof(Int));
      memset(m_aiFrameOffset[1]   , 0, MAX_REF_PIC_NUM * sizeof(Int));
  }
  Int isOffsetZero          (TComSlice* pcSlice, Int list)
  {
    for(UInt frame = 0; frame < pcSlice->getNumRefIdx(RefPicList(list)); ++frame)
    {
      if(frame == 0)
      {     
        for(UInt sub_pos = 0; sub_pos < 16; ++sub_pos)   
          if(m_aiSubpelOffset[list][sub_pos] != 0)
            return 1;
      }
      else
      {
        if(m_aiFrameOffset[list][frame] != 0)
          return 1;
      }
    }
    return 0;
  }
  Void  setOffset_FullpelME       (Int Val, Int list, Int Tab)      { m_aiOffset_FullpelME[list][Tab] = Val;  }
  Int   getOffset_FullpelME       (Int list, Int Tab)               { return m_aiOffset_FullpelME[list][Tab]; }
  Void  setNum_Offset_FullpelME    (Int Val, Int list)      { m_aiNumOffset_FullpelME[list] = Val;  }
  Int   getNum_Offset_FullpelME    (Int list)               { return m_aiNumOffset_FullpelME[list]; }

  Void  setCurrRefFrame            (Int Val)    {  m_iCurrRefFrame = Val;}
  Int   getCurrRefFrame            ()           { return m_iCurrRefFrame;}
  Void  setCurrList                (Int Val)    {  m_iCurrList = Val;}
  Int   getCurrList                ()           { return m_iCurrList;}

#endif

  // DIF filter interface (for half & quarter)
#if TEN_DIRECTIONAL_INTERP
  __inline Void xCTI_FilterDIF_TEN(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int yFrac, Int xFrac);
#endif
#if SIFO_DIF_COMPATIBILITY==1
  __inline Void xCTI_FilterDIF_TEN(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int yFrac, Int xFrac,Int filterNo);
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

#ifdef QC_SIFO //with offset parameter
  __inline Void xCTI_FilterHalfHor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);
  __inline Void xCTI_FilterHalfHor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);

  __inline Void xCTI_FilterQuarter0Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);
  __inline Void xCTI_FilterQuarter0Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);

  __inline Void xCTI_FilterQuarter1Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);
  __inline Void xCTI_FilterQuarter1Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offset);

  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel , Int filter, Int Offset);
  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst , Int filter, Int Offset);
  __inline Void xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst , Int filter, Int Offset);
  
  __inline Void xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst , Int filter, Int Offset);
  __inline Void xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst , Int filter, Int Offset);

  __inline Void xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst , Int filter, Int Offset);
  __inline Void xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst , Int filter, Int Offset);
#endif
#ifdef QC_AMVRES
  __inline Void xCTI_Filter2DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMv);
  __inline Void xCTI_Filter2DHor (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DHor (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
#endif

#if SAMSUNG_CHROMA_IF_EXT
  __inline Void xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMv);
  __inline Void xCTI_Filter2DHorC (Int* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DHorC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
  __inline Void xCTI_Filter1DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Pel*& rpiDst, Int iMV);
#endif

private:

  // set of DIF filters
  static Int xCTI_Filter_VP04     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP06     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP08     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP10     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP12     ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VP14     ( Pel* pSrc, Int* piCoeff, Int iStride );

  static Int xCTI_Filter_VPS04    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS06    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS08    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS10    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS12    ( Pel* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VPS14    ( Pel* pSrc, Int* piCoeff, Int iStride );

  static Int xCTI_Filter_VI04     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI06     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI08     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI10     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI12     ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VI14     ( Int* pSrc, Int* piCoeff, Int iStride );

  static Int xCTI_Filter_VIS04    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS06    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS08    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS10    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS12    ( Int* pSrc, Int* piCoeff, Int iStride );
  static Int xCTI_Filter_VIS14    ( Int* pSrc, Int* piCoeff, Int iStride );

  // multiplication-free implementation
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
          piDst[x*iDstStep] = Clip( (2*piSrcTmp[0*iSrcStride+5] + (-10)*piSrcTmp[1*iSrcStride+4] + 111*piSrcTmp[2*iSrcStride+3] + 37*piSrcTmp[3*iSrcStride+2] + (-10)*piSrcTmp[4*iSrcStride+1] + 2*piSrcTmp[5*iSrcStride+0] + 64)>>7);
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

#if SIFO_DIF_COMPATIBILITY==1
__inline Void TComPredFilter::xCTI_FilterDIF_TEN(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int yFrac, Int xFrac, Int filterNo)
{
  Pel*  piDst    = rpiDst;
  Pel*  piSrcTmp;
  Int pos = 4*yFrac+xFrac;
  Int *piFilter, *piFilter1, *piFilter2 ;

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
	  piFilter = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;		
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0] + piFilter[1]*piSrcTmp[1] + piFilter[2]*piSrcTmp[2] + piFilter[3]*piSrcTmp[3] + piFilter[4]*piSrcTmp[4] + piFilter[5]*piSrcTmp[5] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 2:
	  piFilter = SIFO_Filter6[filterNo][HAL_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0] + piFilter[1]*piSrcTmp[1] + piFilter[2]*piSrcTmp[2] + piFilter[3]*piSrcTmp[3] + piFilter[4]*piSrcTmp[4] + piFilter[5]*piSrcTmp[5] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 3:
	  piFilter = SIFO_Filter6[filterNo][QU1_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0] + piFilter[1]*piSrcTmp[1] + piFilter[2]*piSrcTmp[2] + piFilter[3]*piSrcTmp[3] + piFilter[4]*piSrcTmp[4] + piFilter[5]*piSrcTmp[5] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 4:
	  piFilter = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride] + piFilter[1]*piSrcTmp[1*iSrcStride] + piFilter[2]*piSrcTmp[2*iSrcStride] + piFilter[3]*piSrcTmp[3*iSrcStride] + piFilter[4]*piSrcTmp[4*iSrcStride] + piFilter[5]*piSrcTmp[5*iSrcStride] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 5:
	  piFilter = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride+0] + piFilter[1]*piSrcTmp[1*iSrcStride+1] + piFilter[2]*piSrcTmp[2*iSrcStride+2] + piFilter[3]*piSrcTmp[3*iSrcStride+3] + piFilter[4]*piSrcTmp[4*iSrcStride+4] + piFilter[5]*piSrcTmp[5*iSrcStride+5] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 6:
	  piFilter1 = SIFO_Filter6[filterNo][QU0_IDX];
	  piFilter2 = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter1[0]*piSrcTmp[0*iSrcStride+0] + piFilter1[1]*piSrcTmp[1*iSrcStride+1] + piFilter1[2]*piSrcTmp[2*iSrcStride+2] + piFilter1[3]*piSrcTmp[3*iSrcStride+3] + piFilter1[4]*piSrcTmp[4*iSrcStride+4] + piFilter1[5]*piSrcTmp[5*iSrcStride+5] +
                                     piFilter2[0]*piSrcTmp[0*iSrcStride+5] + piFilter2[1]*piSrcTmp[1*iSrcStride+4] + piFilter2[2]*piSrcTmp[2*iSrcStride+3] + piFilter2[3]*piSrcTmp[3*iSrcStride+2] + piFilter2[4]*piSrcTmp[4*iSrcStride+1] + piFilter2[5]*piSrcTmp[5*iSrcStride+0] + 256)>>9);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 7:
      piFilter = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride+5] + piFilter[1]*piSrcTmp[1*iSrcStride+4] + piFilter[2]*piSrcTmp[2*iSrcStride+3] + piFilter[3]*piSrcTmp[3*iSrcStride+2] + piFilter[4]*piSrcTmp[4*iSrcStride+1] + piFilter[5]*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 8:
      piFilter = SIFO_Filter6[filterNo][HAL_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride] + piFilter[1]*piSrcTmp[1*iSrcStride] + piFilter[2]*piSrcTmp[2*iSrcStride] + piFilter[3]*piSrcTmp[3*iSrcStride] + piFilter[4]*piSrcTmp[4*iSrcStride] + piFilter[5]*piSrcTmp[5*iSrcStride] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
   case 9:
	  piFilter1 = SIFO_Filter6[filterNo][QU0_IDX];
	  piFilter2 = SIFO_Filter6[filterNo][QU1_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter1[0]*piSrcTmp[0*iSrcStride+0] + piFilter1[1]*piSrcTmp[1*iSrcStride+1] + piFilter1[2]*piSrcTmp[2*iSrcStride+2] + piFilter1[3]*piSrcTmp[3*iSrcStride+3] + piFilter1[4]*piSrcTmp[4*iSrcStride+4] + piFilter1[5]*piSrcTmp[5*iSrcStride+5] +
                                     piFilter2[0]*piSrcTmp[0*iSrcStride+5] + piFilter2[1]*piSrcTmp[1*iSrcStride+4] + piFilter2[2]*piSrcTmp[2*iSrcStride+3] + piFilter2[3]*piSrcTmp[3*iSrcStride+2] + piFilter2[4]*piSrcTmp[4*iSrcStride+1] + piFilter2[5]*piSrcTmp[5*iSrcStride+0] + 256)>>9);
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
	  piFilter1 = SIFO_Filter6[filterNo][QU1_IDX];
	  piFilter2 = SIFO_Filter6[filterNo][QU0_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter1[0]*piSrcTmp[0*iSrcStride+0] + piFilter1[1]*piSrcTmp[1*iSrcStride+1] + piFilter1[2]*piSrcTmp[2*iSrcStride+2] + piFilter1[3]*piSrcTmp[3*iSrcStride+3] + piFilter1[4]*piSrcTmp[4*iSrcStride+4] + piFilter1[5]*piSrcTmp[5*iSrcStride+5] +
                                     piFilter2[0]*piSrcTmp[0*iSrcStride+5] + piFilter2[1]*piSrcTmp[1*iSrcStride+4] + piFilter2[2]*piSrcTmp[2*iSrcStride+3] + piFilter2[3]*piSrcTmp[3*iSrcStride+2] + piFilter2[4]*piSrcTmp[4*iSrcStride+1] + piFilter2[5]*piSrcTmp[5*iSrcStride+0] + 256)>>9);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 12:
	  piFilter = SIFO_Filter6[filterNo][QU1_IDX];	  
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride] + piFilter[1]*piSrcTmp[1*iSrcStride] + piFilter[2]*piSrcTmp[2*iSrcStride] + piFilter[3]*piSrcTmp[3*iSrcStride] + piFilter[4]*piSrcTmp[4*iSrcStride] + piFilter[5]*piSrcTmp[5*iSrcStride] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
   case 13:
	  piFilter = SIFO_Filter6[filterNo][QU1_IDX];	  
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride+5] + piFilter[1]*piSrcTmp[1*iSrcStride+4] + piFilter[2]*piSrcTmp[2*iSrcStride+3] + piFilter[3]*piSrcTmp[3*iSrcStride+2] + piFilter[4]*piSrcTmp[4*iSrcStride+1] + piFilter[5]*piSrcTmp[5*iSrcStride+0] + 128)>>8);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
   case 14:
      piFilter1 = SIFO_Filter6[filterNo][QU1_IDX];
	  piFilter2 = SIFO_Filter6[filterNo][QU1_IDX];
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = piDst[x*iDstStep] = Clip( (piFilter1[0]*piSrcTmp[0*iSrcStride+0] + piFilter1[1]*piSrcTmp[1*iSrcStride+1] + piFilter1[2]*piSrcTmp[2*iSrcStride+2] + piFilter1[3]*piSrcTmp[3*iSrcStride+3] + piFilter1[4]*piSrcTmp[4*iSrcStride+4] + piFilter1[5]*piSrcTmp[5*iSrcStride+5] +
                                     piFilter2[0]*piSrcTmp[0*iSrcStride+5] + piFilter2[1]*piSrcTmp[1*iSrcStride+4] + piFilter2[2]*piSrcTmp[2*iSrcStride+3] + piFilter2[3]*piSrcTmp[3*iSrcStride+2] + piFilter2[4]*piSrcTmp[4*iSrcStride+1] + piFilter2[5]*piSrcTmp[5*iSrcStride+0] + 256)>>9);
        piSrc += iSrcStride;
        piDst += iDstStride;
      }
      break;
    case 15:
	  piFilter = SIFO_Filter6[filterNo][QU1_IDX];	  
      for (Int y=0; y<iHeight; y++)
      {
        piSrcTmp = piSrc-2*iSrcStride-2;
        for ( Int x = 0; x < iWidth; x++, piSrcTmp++)
          piDst[x*iDstStep] = Clip( (piFilter[0]*piSrcTmp[0*iSrcStride+0] + piFilter[1]*piSrcTmp[1*iSrcStride+1] + piFilter[2]*piSrcTmp[2*iSrcStride+2] + piFilter[3]*piSrcTmp[3*iSrcStride+3] + piFilter[4]*piSrcTmp[4*iSrcStride+4] + piFilter[5]*piSrcTmp[5*iSrcStride+5] + 128)>>8);
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
        iSum                = xCTI_Filter_VP06_QU1( piSrcTmp, piFilter, iSrcStride );
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
// DIF filters (for AMVRES)
// ------------------------------------------------------------------------------------------------
#ifdef QC_AMVRES
__inline Void TComPredFilter::xCTI_Filter2DVer (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMV)
{
  Int*  piDst = rpiDst;
	Int   iSum;
	Pel*  piSrcTmp;
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+AMVRES_ACC_IDX_OFFSET];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == HAL_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU0_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU1_IDX )
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
			iSum      = xCTI_Filter_VP[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, iSrcStride );
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
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+AMVRES_ACC_IDX_OFFSET];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == HAL_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU0_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU1_IDX )
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
			iSum         = xCTI_Filter_VI[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, 1 );
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
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+AMVRES_ACC_IDX_OFFSET];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == HAL_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU0_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU1_IDX )
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
			iSum      = xCTI_Filter_VP[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, iSrcStride );
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
	Int*  piFilter = CTI_Filter12[m_iTapIdx][iMV+AMVRES_ACC_IDX_OFFSET];

	if ( m_iDIFTap == 12 )
	{
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == HAL_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU0_IDX )
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
		if ( ( iMV+AMVRES_ACC_IDX_OFFSET ) == QU1_IDX )
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
			iSum         = xCTI_Filter_VP[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, 1 );
			piDst   [x ] = Clip( (iSum +  128) >>  8 );
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}
#endif
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
  Int iSum;
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
  Int iSum;
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
  Int iSum;
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
  Int iSum;
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


#ifdef QC_SIFO// with offset parameter
__inline Void TComPredFilter::xCTI_FilterHalfHor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][HAL_IDX] : SIFO_Filter12[filter][HAL_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterHalfHor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][HAL_IDX] : SIFO_Filter12[filter][HAL_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VIS[HAL_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU0_IDX] : SIFO_Filter12[filter][QU0_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU0_IDX] : SIFO_Filter12[filter][QU0_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VI[QU0_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU1_IDX] : SIFO_Filter12[filter][QU1_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  128) >>  8 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Hor(Int* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst    = rpiDst;
  Int   iSum;
  Int*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU1_IDX] : SIFO_Filter12[filter][QU1_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ (0-m_iLeftMargin)*iSrcStep ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                   = xCTI_Filter_VI[QU1_IDX]( piSrcTmp, piFilter, iSrcStep );
      piDst   [x * iDstStep] = Clip( (iSum +  (1<<15)) >>  16 );
      piDst   [x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int iDstStridePel, Pel*& rpiDstPel , Int filter, Int Offsets)
{
  Int*  piDst = rpiDst;
  Pel*  piDstPel = rpiDstPel;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][HAL_IDX] : SIFO_Filter12[filter][HAL_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum									 = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep]		 = iSum; 
      piDstPel[x * iDstStep] = Clip( (iSum +  128) >>  8 );
      piDst   [x * iDstStep] =       piDst      [x * iDstStep] + Offsets;
      piDstPel[x * iDstStep] = Clip( piDstPel   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
    piDstPel += iDstStridePel;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int filter, Int Offsets)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][HAL_IDX] : SIFO_Filter12[filter][HAL_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = iSum; 
      piDst[x * iDstStep] =       piDst      [x * iDstStep] + Offsets;
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterHalfVer (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][HAL_IDX] : SIFO_Filter12[filter][HAL_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                = xCTI_Filter_VPS[HAL_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 );
      piDst[x * iDstStep] = Clip( piDst  [x * iDstStep] + Offsets);

      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int filter, Int Offsets)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU0_IDX] : SIFO_Filter12[filter][QU0_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = iSum; 
      piDst[x * iDstStep] =       piDst      [x * iDstStep] + Offsets;
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter0Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU0_IDX] : SIFO_Filter12[filter][QU0_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                = xCTI_Filter_VP[QU0_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 ); 
      piDst[x * iDstStep] = Clip( piDst   [x * iDstStep] + Offsets);
      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Int*& rpiDst, Int filter, Int Offsets)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU1_IDX] : SIFO_Filter12[filter][QU1_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum								= xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = iSum; 
      piDst[x * iDstStep] =       piDst      [x * iDstStep] + Offsets;

      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

__inline Void TComPredFilter::xCTI_FilterQuarter1Ver (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst, Int filter, Int Offsets)
{
  Pel*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Int*  piFilter = (m_iTapIdx==1)? SIFO_Filter6[filter][QU1_IDX] : SIFO_Filter12[filter][QU1_IDX];

  for ( Int y = iHeight; y != 0; y-- )
  {
    piSrcTmp = &piSrc[ -m_iLeftMargin*iSrcStride ];
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum                = xCTI_Filter_VP[QU1_IDX]( piSrcTmp, piFilter, iSrcStride );
      piDst[x * iDstStep] = Clip( (iSum +  128) >>  8 ); 
      piDst[x * iDstStep] = Clip( piDst[x * iDstStep] + Offsets);

      piSrcTmp += iSrcStep;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

#endif

// ------------------------------------------------------------------------------------------------
// DCTIF filters for Chroma
// ------------------------------------------------------------------------------------------------
#if SAMSUNG_CHROMA_IF_EXT
__inline Void TComPredFilter::xCTI_Filter2DVerC (Pel* piSrc, Int iSrcStride,  Int iWidth, Int iHeight, Int iDstStride,  Int*& rpiDst, Int iMV)
{
  Int*  piDst = rpiDst;
  Int   iSum;
  Pel*  piSrcTmp;
  Pel*  piSrcTmp2;

  Int*  piFilter = CTI_Filter12_C[m_iTapIdxC][iMV+AMVRES_ACC_IDX_OFFSET];

  for ( Int y = iHeight; y != 0; y-- )
  {
	piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
	piSrcTmp2 = piSrc;
    for ( Int x = 0; x < iWidth; x++ )
    {
      iSum      = xCTI_Filter_VPC[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, iSrcStride );

	  if(piSrcTmp2[0] <= piSrcTmp2[iSrcStride])
	  {
		  iSum = Clip3(piSrcTmp2[0]<<8, piSrcTmp2[iSrcStride]<<8, iSum);
	  }
	  else
	  {
		  iSum = Clip3(piSrcTmp2[iSrcStride]<<8, piSrcTmp2[0]<<8, iSum);
	  }

	  piDst[x ] = iSum;
	  piSrcTmp++;
	  piSrcTmp2++;
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
	Int*  piFilter = CTI_Filter12_C[m_iTapIdxC][iMV+AMVRES_ACC_IDX_OFFSET];
	
  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VIC[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, 1 );

      if(piSrc[x] <= piSrc[x+1])
	  {
		  iSum = Clip3(piSrc[x]<<8, piSrc[x+1]<<8, iSum);
	  }
	  else
	  {
		  iSum = Clip3(piSrc[x+1]<<8, piSrc[x]<<8, iSum);
	  }

	      piDst   [x ] = (iSum +  32768) >>  16 ;
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
	Pel*  piSrcTmp2;
	Int*  piFilter = CTI_Filter12_C[m_iTapIdxC][iMV+AMVRES_ACC_IDX_OFFSET];
	
  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC*iSrcStride ];
		piSrcTmp2 = piSrc;
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum      = xCTI_Filter_VPC[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, iSrcStride );
	  
      if(piSrcTmp2[0] <= piSrcTmp2[iSrcStride])
	  {
		  iSum = Clip3(piSrcTmp2[0]<<8, piSrcTmp2[iSrcStride]<<8, iSum);
	  }
	  else
	  {
		  iSum = Clip3(piSrcTmp2[iSrcStride]<<8, piSrcTmp2[0]<<8, iSum);
	  }

	  piDst[x ] = (iSum +  128) >>  8 ;
      piSrcTmp++;
	  piSrcTmp2++;
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
	Int*  piFilter = CTI_Filter12_C[m_iTapIdxC][iMV+AMVRES_ACC_IDX_OFFSET];

  for ( Int y = iHeight; y != 0; y-- )
  {
		piSrcTmp = &piSrc[ 0-m_iLeftMarginC ];
    for ( Int x = 0; x < iWidth; x++ )
    {
			iSum         = xCTI_Filter_VPC[iMV+AMVRES_ACC_IDX_OFFSET]( piSrcTmp, piFilter, 1 );

      if(piSrc[x] <= piSrc[x+1])
	  {
		  iSum = Clip3(piSrc[x]<<8, piSrc[x+1]<<8, iSum);
	  }
	  else
	  {
		  iSum = Clip3(piSrc[x+1]<<8, piSrc[x]<<8, iSum);
	  }

	  piDst[x ] = (iSum +  128) >>  8 ;
			piSrcTmp++;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}
#endif

#endif // __TCOMPREDFILTER__
