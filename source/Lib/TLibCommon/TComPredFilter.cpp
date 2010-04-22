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

/** \file			TComPredFilter.cpp
    \brief		interpolation filter class
*/

#include "TComPredFilter.h"

// ====================================================================================================================
// Tables
// ====================================================================================================================

// DIF filter set for 1/12
Int CTI_Filter12 [6][14][14] =
{
	//  4-tap filter
	{
		{    29,  252,  -34,    9,},
		{    13,  257,  -19,    5,},
		{     0,  256,    0,    0,},
		{   -11,  250,   22,   -5,},
		{   -18,  238,   46,  -10,},
		{   -24,  224,   72,  -16,},//==CTI_Filter_Quater0
		{   -27,  203,  100,  -20,},
		{   -28,  180,  128,  -24,},
		{   -32,  160,  160,  -32,},//==CTI_Filter_Half
		{   -24,  128,  180,  -28,},
		{   -20,  100,  203,  -27,},
		{   -16,   72,  224,  -24,},//==CTI_Filter_Quater1
		{   -10,   46,  238,  -18,},
		{    -5,   22,  250,  -11,},
	},
	//  6-tap filter
	{
		{    -9,   41,  248,  -34,   14,   -4,},
		{    -4,   19,  254,  -19,    8,   -2,},
		{     0,    0,  256,    0,    0,    0,},
		{     4,  -16,  252,   22,   -8,    2,},
		{     6,  -28,  242,   48,  -17,    5,},
		{     8,  -32,  224,   72,  -24,    8 },//==CTI_Filter_Quater0
		{    11,  -42,  208,  103,  -33,    9,},
		{    12,  -44,  184,  132,  -39,   11,},
		{     8,  -40,  160,  160,  -40,    8 },//==CTI_Filter_Half
		{    11,  -39,  132,  184,  -44,   12,},
		{     9,  -33,  103,  208,  -42,   11,},
		{     8,  -24,   72,  224,  -32,    8,},//==CTI_Filter_Quater1
		{     5,  -17,   48,  242,  -28,    6,},
		{     2,   -8,   22,  252,  -16,    4,},
	},
	//  8-tap filter
	{
		{     4,  -16,   46,  246,  -35,   16,   -8,    3,},
		{     2,   -8,   21,  254,  -19,    9,   -4,    1,},
		{     0,    0,    0,  256,    0,    0,    0,    0,},
		{    -2,    7,  -18,  252,   23,  -10,    5,   -1,},
		{    -4,   13,  -31,  243,   48,  -19,    9,   -3,},
		{    -4,   16,  -32,  228,   68,  -28,   12,   -4 },//==CTI_Filter_Quater0
		{    -6,   20,  -47,  210,  104,  -37,   17,   -5,},
		{    -6,   21,  -49,  187,  133,  -44,   20,   -6,},
		{    -1,    9,  -40,  160,  160,  -40,   9,    -1 },//==CTI_Filter_Half
		{    -6,   20,  -44,  133,  187,  -49,   21,   -6,},
		{    -5,   17,  -37,  104,  210,  -47,   20,   -6,},
		{    -4,   12,  -28,   68,  228,  -32,   16,   -4,},//==CTI_Filter_Quater1
		{    -3,    9,  -19,   48,  243,  -31,   13,   -4,},
		{    -1,    5,  -10,   23,  252,  -18,    7,   -2,},
	},
	// 10-tap filter
	{
		{    -3,    9,  -18,   46,  246,  -34,   17,  -10,    5,   -2,},
		{    -1,    4,   -9,   22,  253,  -20,    9,   -5,    3,   -1,},
		{     0,    0,    0,    0,  256,    0,    0,    0,    0,    0,},
		{     1,   -4,    8,  -18,  252,   23,  -10,    6,   -3,    1,},
		{     2,   -7,   15,  -33,  244,   48,  -20,   11,   -6,    2,},
		{     4,   -8,   20,  -44,  228,   76,  -32,   16,   -8,    4 },//==CTI_Filter_Quater0
		{     4,  -12,   24,  -49,  210,  105,  -39,   21,  -11,    3,},
		{     4,  -13,   26,  -52,  187,  134,  -46,   24,  -12,    4,},
		{     4,  -16,   28,  -48,  160,  160,  -48,   28,  -16,    4 },//==CTI_Filter_Half
		{     4,  -12,   24,  -46,  134,  187,  -52,   26,  -13,    4,},
		{     3,  -11,   21,  -39,  105,  210,  -49,   24,  -12,    4,},
		{     4,   -8,   16,  -32,   76,  228,  -44,   20,   -8,    4,},//==CTI_Filter_Quater1
		{     2,   -6,   11,  -20,   48,  244,  -33,   15,   -7,    2,},
		{     1,   -3,    6,  -10,   23,  254,  -18,    8,   -4,    1,},
	},
	// 12-tap filter
	{
	  {     2,   -5,   10,  -19,   47,  245,  -35,   18,  -11,    7,   -4,    1 },
	  {     1,   -3,    5,  -10,   22,  253,  -19,   10,   -6,    4,   -2,    1 },
	  {     0,    0,    0,    0,    0,  256,    0,    0,    0,    0,    0,    0 },
	  {    -1,    3,   -5,    9,  -19,  253,   23,  -10,    6,   -4,    2,   -1 },
	  {    -2,    5,   -9,   16,  -34,  244,   49,  -21,   12,   -7,    4,   -1 },
	  {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },//==CTI_Filter_Quater0
	  {    -3,    8,  -15,   26,  -50,  211,  105,  -40,   22,  -13,    7,   -2 },
	  {    -3,    9,  -16,   28,  -53,  188,  134,  -47,   26,  -15,    8,   -3 },
	  {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },//==CTI_Filter_Half
	  {    -3,    8,  -15,   26,  -47,  134,  188,  -53,   28,  -16,    9,   -3 },
	  {    -2,    7,  -13,   22,  -40,  105,  211,  -50,   26,  -15,    8,   -3 },
	  {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,   -12,    5,   -1 },//==CTI_Filter_Quater1
	  {    -1,    4,   -7,   12,  -21,   49,  244,  -34,   16,   -9,    5,   -2 },
	  {    -1,    2,   -4,    6,  -10,   23,  253,  -19,    9,   -5,    3,   -1 }
	},
	// 14-tap filter
	{
		{    -1,    4,   -7,   12,  -20,   47,  244,  -36,   18,  -11,    8,   -5,    3,   -1,},
		{    -1,    2,   -4,    6,  -10,   22,  254,  -19,   10,   -6,    4,   -3,    1,    0,},
		{     0,    0,    0,    0,    0,    0,  256,    0,    0,    0,    0,    0,    0,    0,},
		{     1,   -2,    3,   -5,    9,  -19,  254,   23,  -11,    6,   -4,    3,   -2,    0,},
		{     1,   -3,    6,  -10,   17,  -34,  243,   49,  -21,   13,   -8,    5,   -3,    1,},
		{	    1,   -4,    8,  -16,   24,  -48,  232,   76,  -31,   21,  -12,    8,   -4,    1 },//==CTI_Filter_Quater0
		{     2,   -6,   10,  -17,   27,  -51,  211,  105,  -41,   24,  -15,   10,   -5,    2,},
		{     2,   -6,   11,  -18,   29,  -54,  189,  134,  -48,   27,  -17,   11,   -6,    2,},
		{     1,   -4,    8,  -16,   28,  -48,  159,  159,  -48,   28,  -16,   8,    -4,    1 },//==CTI_Filter_Half
		{     2,   -6,   11,  -17,   27,  -48,  134,  188,  -54,   29,  -18,   11,   -6,    2,},
		{     2,   -5,   10,  -15,   24,  -41,  105,  211,  -51,   27,  -17,   10,   -6,    2,},
		{     1,   -4,    8,  -12,   21,  -31,   76,  232,  -48,   24,  -16,    8,   -4,    1,},//==CTI_Filter_Quater1
		{     1,   -3,    5,   -8,   13,  -21,   49,  243,  -34,   17,  -10,    6,   -3,    1,},
		{     0,   -2,    3,   -4,    6,  -11,   23,  253,  -19,    9,   -5,    3,   -2,    1,},
	}
};

// ====================================================================================================================
// Constructor
// ====================================================================================================================

TComPredFilter::TComPredFilter()
{
	// initial number of taps for Luma
	setDIFTap( 12 );

	// initial number of taps for Chroma
	setDIFTapC( 6 );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComPredFilter::setDIFTap( Int i )
{
	m_iDIFTap			 = i;
	m_iTapIdx			 = (i>>1)-2;		// 4 = 0, 6 = 1, 8 = 2, ...
	m_iLeftMargin  = (m_iDIFTap-2)>>1;
	m_iRightMargin = m_iDIFTap-m_iLeftMargin;

	// initialize function pointers
	if ( m_iDIFTap == 4 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP04;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS04;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI04;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS04;
		}
	}
	else if ( m_iDIFTap == 6 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP06;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS06;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI06;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS06;
		}
	}
	else if ( m_iDIFTap == 8 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP08;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS08;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI08;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS08;
		}
	}
	else if ( m_iDIFTap == 10 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP10;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS10;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI10;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS10;
		}
	}
	else if ( m_iDIFTap == 12 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
		}
	}
	else if ( m_iDIFTap == 14 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP14;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS14;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI14;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS14;
		}
	}
	else
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
			xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
			xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
			xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
		}
	}
}

Void TComPredFilter::setDIFTapC( Int i )
{
	m_iDIFTapC			 = i;
	m_iTapIdxC			 = (i>>1)-2;		// 4 = 0, 6 = 1, 8 = 2, ...
	m_iLeftMarginC  = (m_iDIFTapC-2)>>1;
	m_iRightMarginC = m_iDIFTapC-m_iLeftMarginC;

	// initialize function pointers
	if ( m_iDIFTapC == 4 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP04;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS04;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI04;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS04;
		}
	}
	else if ( m_iDIFTapC == 6 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP06;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS06;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI06;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS06;
		}
	}
	else if ( m_iDIFTapC == 8 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP08;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS08;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI08;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS08;
		}
	}
	else if ( m_iDIFTapC == 10 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP10;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS10;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI10;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS10;
		}
	}
	else if ( m_iDIFTapC == 12 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP12;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS12;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI12;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS12;
		}
	}
	else if ( m_iDIFTapC == 14 )
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP14;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS14;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI14;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS14;
		}
	}
	else
	{
		for ( Int k=0; k<14; k++ )
		{
			xCTI_Filter_VPC [k] = TComPredFilter::xCTI_Filter_VP12;
			xCTI_Filter_VPSC[k] = TComPredFilter::xCTI_Filter_VPS12;
			xCTI_Filter_VIC [k] = TComPredFilter::xCTI_Filter_VI12;
			xCTI_Filter_VISC[k] = TComPredFilter::xCTI_Filter_VIS12;
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
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VP06( Pel* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VP08( Pel* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VP10( Pel* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VP12( Pel* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[10];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[11];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VP14( Pel* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[10];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[11];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[12];	iIdx+= iStride;
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
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VI06( Int* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VI08( Int* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VI10( Int* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VI12( Int* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[10];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[11];
	return iSum;
}
Int TComPredFilter::xCTI_Filter_VI14( Int* pSrc, Int* piCoeff, Int iStride )
{
	Int iSum, iIdx = 0;
	iSum  = pSrc[0]*piCoeff[0];			iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[1];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[2];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[3];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[4];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[5];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[6];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[7];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[8];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[9];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[10];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[11];	iIdx+= iStride;
	iSum += pSrc[iIdx]*piCoeff[12];	iIdx+= iStride;
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

// ------------------------------------------------------------------------------------------------
// Bi-linear functions
// ------------------------------------------------------------------------------------------------

void TComPredFilter::xFilterQuarterHor      (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst[x * iDstStep] = (piSrc[x*iSrcStep] + piSrc[x*iSrcStep + iSrcStep] + 1) >> 1;

    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

void TComPredFilter::xFilterQuarterVer      (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst[x * iDstStep] = (piSrc[x*iSrcStep] + piSrc[x*iSrcStep + iSrcStride] + 1) >> 1;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

void TComPredFilter::xFilterQuarterDiagUp   (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst[x * iDstStep] = (piSrc[x*iSrcStep] + piSrc[x*iSrcStep + iSrcStep - iSrcStride] + 1) >> 1;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}

void TComPredFilter::xFilterQuarterDiagDown (Pel* piSrc, Int iSrcStride, Int iSrcStep, Int iWidth, Int iHeight, Int iDstStride, Int iDstStep, Pel*& rpiDst)
{
  Pel*  piDst = rpiDst;
  for ( Int y = 0; y < iHeight; y++ )
  {
    for ( Int x = 0; x < iWidth; x++ )
    {
      piDst[x * iDstStep] = (piSrc[x*iSrcStep] + piSrc[x*iSrcStep + iSrcStep + iSrcStride] + 1) >> 1;
    }
    piSrc += iSrcStride;
    piDst += iDstStride;
  }
  return;
}
