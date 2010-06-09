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

/** \file     TComPredFilter.cpp
    \brief    interpolation filter class
*/

#include "TComPredFilter.h"

// ====================================================================================================================
// Tables
// ====================================================================================================================

// DIF filter set for half & quarter
Int CTI_Filter12 [5][3][12] =
{
  //  4-tap filter
  {
    {   -24,  224,   72,  -16,},  // Quarter0
    {   -32,  160,  160,  -32,},  // Half
    {   -16,   72,  224,  -24,},  // Quarter1
  },
  //  6-tap filter
  {
    {     8,  -32,  224,   72,  -24,    8 },  // Quarter0
    {     8,  -40,  160,  160,  -40,    8 },  // Half
    {     8,  -24,   72,  224,  -32,    8,},  // Quarter1
  },
  //  8-tap filter
  {
    {    -4,   16,  -32,  228,   68,  -28,   12,   -4 },  // Quarter0
    {    -1,    9,  -40,  160,  160,  -40,   9,    -1 },  // Half
    {    -4,   12,  -28,   68,  228,  -32,   16,   -4,},  // Quarter1
  },
  // 10-tap filter
  {
    {     4,   -8,   20,  -44,  228,   76,  -32,   16,   -8,    4 },  // Quarter0
    {     4,  -16,   28,  -48,  160,  160,  -48,   28,  -16,    4 },  // Half
    {     4,   -8,   16,  -32,   76,  228,  -44,   20,   -8,    4,},  // Quarter1
  },
  // 12-tap filter
  {
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },  // Quarter0
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },  // Half
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 },  // Quarter1
  },
};

// ====================================================================================================================
// Constructor
// ====================================================================================================================

TComPredFilter::TComPredFilter()
{
  // initial number of taps for Luma
  setDIFTap( 12 );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComPredFilter::setDIFTap( Int i )
{
  m_iDIFTap      = i;
  m_iTapIdx      = (i>>1)-2;    // 4 = 0, 6 = 1, 8 = 2, ...
  m_iLeftMargin  = (m_iDIFTap-2)>>1;
  m_iRightMargin = m_iDIFTap-m_iLeftMargin;

  // initialize function pointers
  if ( m_iDIFTap == 4 )
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP04;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS04;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI04;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS04;
    }
  }
  else if ( m_iDIFTap == 6 )
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP06;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS06;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI06;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS06;
    }
  }
  else if ( m_iDIFTap == 8 )
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP08;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS08;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI08;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS08;
    }
  }
  else if ( m_iDIFTap == 10 )
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP10;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS10;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI10;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS10;
    }
  }
  else if ( m_iDIFTap == 12 )
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
    }
  }
  else
  {
    for ( Int k=0; k<3; k++ )
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Set of DIF functions
// ------------------------------------------------------------------------------------------------

// vertical filtering (Pel)
Int TComPredFilter::xCTI_Filter_VP04( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VP06( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VP08( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VP10( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VP12( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[10]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[11];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VP14( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[10]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[11]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[12]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[13];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS04( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*3])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*2])*piCoeff[1];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS06( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*5])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*4])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*3])*piCoeff[2];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS08( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*7])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*6])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*5])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*4])*piCoeff[3];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS10( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*9])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*8])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*7])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*6])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride*5])*piCoeff[4];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS12( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*11])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*10])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride* 9])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride* 8])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride* 7])*piCoeff[4];
  iSum += (pSrc[iStride*5]+pSrc[iStride* 6])*piCoeff[5];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VPS14( Pel* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*13])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*12])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*11])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*10])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride* 9])*piCoeff[4];
  iSum += (pSrc[iStride*5]+pSrc[iStride* 8])*piCoeff[5];
  iSum += (pSrc[iStride*6]+pSrc[iStride* 7])*piCoeff[6];
  return iSum;
}

// vertical filtering (Int)
Int TComPredFilter::xCTI_Filter_VI04( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VI06( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VI08( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VI10( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VI12( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[10]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[11];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VI14( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum, iIdx = 0;
  iSum  = pSrc[0]*piCoeff[0];     iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[1];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[2];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[3];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[4];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[5];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[6];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[7];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[8];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[9];  iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[10]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[11]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[12]; iIdx+= iStride;
  iSum += pSrc[iIdx]*piCoeff[13];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS04( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*3])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*2])*piCoeff[1];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS06( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*5])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*4])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*3])*piCoeff[2];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS08( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*7])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*6])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*5])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*4])*piCoeff[3];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS10( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*9])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*8])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*7])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*6])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride*5])*piCoeff[4];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS12( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*11])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*10])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride* 9])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride* 8])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride* 7])*piCoeff[4];
  iSum += (pSrc[iStride*5]+pSrc[iStride* 6])*piCoeff[5];
  return iSum;
}
Int TComPredFilter::xCTI_Filter_VIS14( Int* pSrc, Int* piCoeff, Int iStride )
{
  Int iSum;
  iSum  = (pSrc[        0]+pSrc[iStride*13])*piCoeff[0];
  iSum += (pSrc[iStride*1]+pSrc[iStride*12])*piCoeff[1];
  iSum += (pSrc[iStride*2]+pSrc[iStride*11])*piCoeff[2];
  iSum += (pSrc[iStride*3]+pSrc[iStride*10])*piCoeff[3];
  iSum += (pSrc[iStride*4]+pSrc[iStride* 9])*piCoeff[4];
  iSum += (pSrc[iStride*5]+pSrc[iStride* 8])*piCoeff[5];
  iSum += (pSrc[iStride*6]+pSrc[iStride* 7])*piCoeff[6];
  return iSum;
}

