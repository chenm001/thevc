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

// DIF filter set for Chroma
#if SAMSUNG_CHROMA_IF_EXT
Int CTI_Filter12_C [5][7][12] =
{
  //  4-tap filter
  {
    {   -14,  244,   34,   -8,},	// 1/8
    {   -24,  224,   72,  -16,},	// Quarter0
    {   -28,  192,  114,  -22,},	// 3/8
    {   -32,  160,  160,  -32,},	// Half
    {   -22,  114,  192,  -28,},	// 5/8
    {   -16,   72,  224,  -24,},	// Quarter1
    {    -8,   34,  244,  -14,}	  // 7/8

  },
    //  6-tap filter
  {
    {     5,  -22,  247,   35,  -13,    4 },	// 1/8
    {     8,  -32,  224,   72,  -24,    8 },	// Quarter0
    {    11,  -43,  196,  118,  -36,    10 },	// 3/8
    {     8,  -40,  160,  160,  -40,    8 },	// Half
    {    10,  -36,  118,  196,  -43,    11 },	// 5/8
    {     8,  -24,   72,  224,  -32,    8,},	// Quarter1
    {     4,  -13,   35,  247,  -22,    5 }	  // 7/8
  },
    //  8-tap filter
  {
    {    -3,   10,  -25,  248,   36,  -15,    7,   -2 },	// 1/8
    {    -4,   16,  -40,  228,   76,  -28,   12,   -4 },    // Quarter0
    {    -6,   21,  -48,  198,  119,  -41,   19,   -6 },	// 3/8
    {    -4,   20,  -48,  160,  160,  -48,   20,   -4 },    // Half
    {    -6,   19,  -41,  119,  198,  -48,   21,   -6 },	// 5/8
    {    -4,   12,  -28,   76,  228,  -40,   16,   -4 },    // Quarter1
    {    -2,    7,  -15,   36,  248,  -25,   10,   -3 }	  // 7/8

  },
    // 10-tap filter
  {
    {     2,   -6,   12,  -26,  248,   36,  -15,    9,   -5,    1 },	// 1/8
    {     4,   -8,   20,  -44,  228,   76,  -32,   16,   -8,    4 },	// Quarter0
    {     4,  -13,   25,  -51,  199,  120,  -43,   23,  -12,    4 },	// 3/8
    {     4,  -16,   28,  -48,  160,  160,  -48,   28,  -16,    4 },	// Half
    {     4,  -12,   23,  -43,  120,  199,  -51,   25,  -13,    4 },	// 5/8
    {     4,   -8,   16,  -32,   76,  228,  -44,   20,   -8,    4,},	// Quarter1
    {     1,   -5,    9,  -15,   36,  248,  -26,   12,   -6,    2 }	  // 7/8

  },

  {
    {    -1,    4,   -7,   13,  -27,  249,   36,  -16,    9,   -6,    3,   -1 },  // 1/8
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },	// Quarter0
    {    -3,    9,  -15,   27,  -51,  200,  119,  -44,   24,  -14,    7,   -3 },	// 3/8
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },	// Half
    {    -3,    7,  -14,   24,  -44,  119,  200,  -51,   27,  -15,    9,   -3 },	// 5/8
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 },	// Quarter1
    {    -1,    3,   -6,    9,  -16,   36,  249,  -27,   13,   -7,    4,   -1 }   // 7/8
  }
};
#endif

// DIF filter set for half & quarter
#ifdef QC_AMVRES
Int CTI_Filter12 [5][7][12] =
{
  //  4-tap filter
  {
    {   -14,  244,   34,   -8,},	// 1/8
    {   -24,  224,   72,  -16,},	// Quarter0
    {   -28,  192,  114,  -22,},	// 3/8
    {   -32,  160,  160,  -32,},	// Half
    {   -22,  114,  192,  -28,},	// 5/8
    {   -16,   72,  224,  -24,},	// Quarter1
    {    -8,   34,  244,  -14,}	  // 7/8

  },
    //  6-tap filter
  {
    {     5,  -22,  247,   35,  -13,    4 },	// 1/8
    {     8,  -32,  224,   72,  -24,    8 },	// Quarter0
    {    11,  -43,  196,  118,  -36,    10 },	// 3/8
    {     8,  -40,  160,  160,  -40,    8 },	// Half
    {    10,  -36,  118,  196,  -43,    11 },	// 5/8
    {     8,  -24,   72,  224,  -32,    8,},	// Quarter1
    {     4,  -13,   35,  247,  -22,    5 }	  // 7/8
  },
    //  8-tap filter
  {
    {    -3,   10,  -25,  248,   36,  -15,    7,   -2 },	// 1/8
    {    -4,   16,  -40,  228,   76,  -28,   12,   -4 },    // Quarter0
    {    -6,   21,  -48,  198,  119,  -41,   19,   -6 },	// 3/8
    {    -4,   20,  -48,  160,  160,  -48,   20,   -4 },    // Half
    {    -6,   19,  -41,  119,  198,  -48,   21,   -6 },	// 5/8
    {    -4,   12,  -28,   76,  228,  -40,   16,   -4 },    // Quarter1
    {    -2,    7,  -15,   36,  248,  -25,   10,   -3 }	  // 7/8

  },
    // 10-tap filter
  {
    {     2,   -6,   12,  -26,  248,   36,  -15,    9,   -5,    1 },	// 1/8
    {     4,   -8,   20,  -44,  228,   76,  -32,   16,   -8,    4 },	// Quarter0
    {     4,  -13,   25,  -51,  199,  120,  -43,   23,  -12,    4 },	// 3/8
    {     4,  -16,   28,  -48,  160,  160,  -48,   28,  -16,    4 },	// Half
    {     4,  -12,   23,  -43,  120,  199,  -51,   25,  -13,    4 },	// 5/8
    {     4,   -8,   16,  -32,   76,  228,  -44,   20,   -8,    4,},	// Quarter1
    {     1,   -5,    9,  -15,   36,  248,  -26,   12,   -6,    2 }	  // 7/8

  },
    // 12-tap filter
#ifdef QC_AMVRES_LOW_COMPLEXTY
  {
    {     0,    0,   -3,   10,  -25,  248,   36,  -15,    7,   -2,    0,    0 },  // 1/8
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },	// Quarter0
    {     0,    0,   -6,   21,  -48,  198,  119,  -41,   19,   -6,    0,    0 },	// 3/8
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },	// Half
    {     0,    0,   -6,   19,  -41,  119,  198,  -48,   21,   -6,    0,    0 },	// 5/8
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 },	// Quarter1
    {     0,    0,   -2,    7,  -15,   36,  248,  -25,   10,   -3,    0,    0 }   // 7/8
  }
#else
  {
    {    -1,    4,   -7,   13,  -27,  249,   36,  -16,    9,   -6,    3,   -1 },  // 1/8
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },	// Quarter0
    {    -3,    9,  -15,   27,  -51,  200,  119,  -44,   24,  -14,    7,   -3 },	// 3/8
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },	// Half
    {    -3,    7,  -14,   24,  -44,  119,  200,  -51,   27,  -15,    9,   -3 },	// 5/8
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 },	// Quarter1
    {    -1,    3,   -6,    9,  -16,   36,  249,  -27,   13,   -7,    4,   -1 }   // 7/8
  }
#endif
};
#else
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
    {    -4,   16,  -40,  228,   76,  -28,   12,   -4 },  // Quarter0
    {    -4,   20,  -48,  160,  160,  -48,   20,   -4 },  // Half
    {    -4,   12,  -28,   76,  228,  -40,   16,   -4 },  // Quarter1
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
#endif


#ifdef QC_SIFO
#ifdef QC_AMVRES
Int SIFO_Filter6[4][7][6]=
{
  //6-tap filters
  {
    {},
    {     8,  -32,  224,   72,  -24,    8 },	  // Quarter0
    {},
    {     8,  -40,  160,  160,  -40,    8 },	  // Half
    {},
    {     8,  -24,   72,  224,  -32,    8 }, 	  // Quarter1
    {}
  },
  {
    {},
    {     6,  -30,  222,   74,  -20,    4 },		// Quarter0
    {},
    {     6,  -34,  156,  156,  -34,    6 },		// Half
    {},
    {     4,  -20,   74,  222,  -30,    6 },		  // Quarter1
    {}
  },
  {
    {},
    {    -6,    8,  182,   86,  -12,   -2 },		// Quarter0
    {},
    {    -8,   -8,  144,  144,   -8,   -8 },		// Half
    {},
    {    -2,  -12,   86,  182,    8,   -6 },		  // Quarter1
    {}
  },
  {
    {},
    {    12,  -36,  226,   70,  -24,    8 },		// Quarter0
    {},
    {    14,  -46,  160,  160,  -46,   14 },	  // Half
    {},
    {     8,  -24,   70,  226,  -36,   12 },		  // Quarter1
    {}
  }
};

Int SIFO_Filter12[4][7][12]=
{
  //12-tap filters
  {
    {},
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },	// Quarter0
    {},
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },	// Half
    {},
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 },	  // Quarter1
    {}
  },
  {
    {},
    {     0,    0,   -3,   12,  -37,  229,   71,  -21,    6,   -1,    0,    0 },	// Quarter0
    {},
    {     0,    0,   -3,   12,  -39,  158,  158,  -39,   12,   -3,    0,    0 },	// Half
    {},
    {     0,    0,   -1,    6,  -21,   71,  229,  -37,   12,   -3,    0,    0 },	  // Quarter1
    {}
  },
  {
    {},
    {     0,    0,   -4,   16,  -40,  228,   76,  -28,   12,   -4,    0,    0},   // Quarter0
    {},
    {     0,    0,   -4,   20,  -48,  160,  160,  -48,   20,   -4,    0,    0},   // Half
    {},
    {     0,    0,   -4,   12,  -28,   76,  228,  -40,   16,   -4,    0,    0},   // Quarter1
    {}
  },
  {
    {},
    {     0,    0,   -3,   12,  -37,  229,   71,  -21,    6,   -1,    0,    0 },	// Quarter0
    {},
    {     0,    0,   -3,   12,  -39,  158,  158,  -39,   12,   -3,    0,    0 },	// Half
    {},
    {     0,    0,   -1,    6,  -21,   71,  229,  -37,   12,   -3,    0,    0 },	  // Quarter1
    {}
  }
};
#else
Int SIFO_Filter6[4][3][6]=
{
  //6-tap filters
  {
    {     8,  -32,  224,   72,  -24,    8 },	  // Quarter0
    {     8,  -40,  160,  160,  -40,    8 },	  // Half
    {     8,  -24,   72,  224,  -32,    8 } 	  // Quarter1
  },
  {
    {     6,  -30,  222,   74,  -20,    4 },		// Quarter0
    {     6,  -34,  156,  156,  -34,    6 },		// Half
    {     4,  -20,   74,  222,  -30,    6 }		  // Quarter1
  },
  {
    {    -6,    8,  182,   86,  -12,   -2 },		// Quarter0
    {    -8,   -8,  144,  144,   -8,   -8 },		// Half
    {    -2,  -12,   86,  182,    8,   -6 }		  // Quarter1
  },
  {
    {    12,  -36,  226,   70,  -24,    8 },		// Quarter0
    {    14,  -46,  160,  160,  -46,   14 },	  // Half
    {     8,  -24,   70,  226,  -36,   12 }		  // Quarter1
  }
};

Int SIFO_Filter12[4][3][12]=
{
  //12-tap filters
  {
    {    -1,    5,  -12,   20,  -40,  229,   76,  -32,   16,   -8,    4,   -1 },	// Quarter0
    {    -1,    8,  -16,   24,  -48,  161,  161,  -48,   24,  -16,    8,   -1 },	// Half
    {    -1,    4,   -8,   16,  -32,   76,  229,  -40,   20,  -12,    5,   -1 }	  // Quarter1
  },
  {
    {     0,    0,   -3,   12,  -37,  229,   71,  -21,    6,   -1,    0,    0 },	// Quarter0
    {     0,    0,   -3,   12,  -39,  158,  158,  -39,   12,   -3,    0,    0 },	// Half
    {     0,    0,   -1,    6,  -21,   71,  229,  -37,   12,   -3,    0,    0 }	  // Quarter1
  },
  {
    {     0,    0,   -4,   16,  -40,  228,   76,  -28,   12,   -4,    0,    0 },  // Quarter0
    {     0,    0,   -4,   20,  -48,  160,  160,  -48,   20,   -4,    0,    0 },  // Half
    {     0,    0,   -4,   12,  -28,   76,  228,  -40,   16,   -4,    0,    0 }   // Quarter1
  },
  {
    {     0,    0,   -3,   12,  -37,  229,   71,  -21,    6,   -1,    0,    0 },	// Quarter0
    {     0,    0,   -3,   12,  -39,  158,  158,  -39,   12,   -3,    0,    0 },	// Half
    {     0,    0,   -1,    6,  -21,   71,  229,  -37,   12,   -3,    0,    0 }	  // Quarter1
  }
};
#endif  //QC_AMVRES
#endif  //QC_SIFO
// ====================================================================================================================
// Constructor
// ====================================================================================================================

TComPredFilter::TComPredFilter()
{
  // initial number of taps for Luma
  setDIFTap( 12 );

#if SAMSUNG_CHROMA_IF_EXT
  setDIFTapC( 6 );
#endif
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
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP04;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS04;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI04;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS04;
    }
  }
  else if ( m_iDIFTap == 6 )
  {
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP06;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS06;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI06;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS06;
    }
  }
  else if ( m_iDIFTap == 8 )
  {
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP08;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS08;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI08;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS08;
    }
  }
  else if ( m_iDIFTap == 10 )
  {
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP10;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS10;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI10;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS10;
    }
  }
  else if ( m_iDIFTap == 12 )
  {
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
    }
  }
  else
  {
#ifdef QC_AMVRES
		for ( Int k=0; k<7; k++ )
#else
    for ( Int k=0; k<3; k++ )
#endif
    {
      xCTI_Filter_VP [k] = TComPredFilter::xCTI_Filter_VP12;
      xCTI_Filter_VPS[k] = TComPredFilter::xCTI_Filter_VPS12;
      xCTI_Filter_VI [k] = TComPredFilter::xCTI_Filter_VI12;
      xCTI_Filter_VIS[k] = TComPredFilter::xCTI_Filter_VIS12;
    }
  }

#ifdef QC_SIFO
  m_uiNum_AvailableFilters = (m_iDIFTap == 6) ? 4 : 2;
  m_uiNum_SIFOFilters = m_uiNum_AvailableFilters*m_uiNum_AvailableFilters;

#if SIFO_DIF_COMPATIBILITY==1
  if(m_iDIFTap == 6)
    m_uiNum_SIFOFilters += m_uiNum_AvailableFilters; //Directional is separate
#endif

  for(i = 0; i < 16; ++i)
  {
    if (i<=4 || i==8 || i==12)
    {
      m_uiNUM_SIFO_TAB[i]=m_uiNum_AvailableFilters; 
    }
    else
    {
      m_uiNUM_SIFO_TAB[i]=m_uiNum_SIFOFilters;
    }
  }
#endif
}

#if SAMSUNG_CHROMA_IF_EXT
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
#endif
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

