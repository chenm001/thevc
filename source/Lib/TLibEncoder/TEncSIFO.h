/* ====================================================================================================================

The copyright in this software is being made available under the License included below.
This software may be subject to other third party and 	contributor rights, including patent rights, and no such
rights are granted under this license.

Copyright (c) 2010, QUALCOMM INC.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted only for
the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
promoting such standards. The following conditions are required to be met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and
the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
the following disclaimer in the documentation and/or other materials provided with the distribution.
* The name of QUALCOMM INC may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/

/** \file			TEncSIFO.h
\brief		Switched Interpolation with Offsets(SIFO) - encoder class (header)
*/

#ifndef __TENCSIFO__
#define __TENCSIFO__

// Include files
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComList.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPicYuv.h"
#include "TEncCu.h"

class TEncTop;
class TEncGOP;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// slice encoder class
class TEncSIFO
{
private:
  TEncCfg*                m_pcCfg;															///< encoder configuration class
  TEncSearch*							m_pcPredSearch;												///< encoder search class

  Double **AccErrorP;                              // [Filter][Sppos]
  Double **SequenceAccErrorP;                      // [Filter][Sppos]
  Double ****SequenceAccErrorB;                    // [FilterF][FilterB][SpposF][SpposB]
  Double ***SIFO_FILTER;                           // [num_SIFO][16 subpels][SQR_FILTER]
#if USE_DIAGONAL_FILT==1
  Double **SIFO_FILTER_DIAG;                       //           [16 subpels][SQR_FILTER] 
#endif
  Int SequenceBestCombFilterB[16];                 // [Sppos]

protected:
  Int xGet_mem2Ddouble(Double ***array2D, Int rows, Int columns);
  Int xGet_mem3Ddouble(Double ****array3D, Int frames, Int rows, Int columns);
  Int xGet_mem4Ddouble(Double *****array4D, Int idx, Int frames, Int rows, Int columns );
  Double  xComputeImgSum			( Pel* img,										Int width, Int height, Int stride		);	///< compute sum of pixel values
  Void xResetAll(TComSlice* pcSlice);
  Void xResetSequenceFilters();
  Void xAccumulateError_P(TComPic*& rpcPic);
  Void xAccumulateError_B(TComPic*& rpcPic);  
  Void xUpdateSequenceFilters_P(TComSlice* pcSlice);
  Void xUpdateSequenceFilters_P_pred(TComSlice* pcSlice);
  Void xComputeFilterCombination_B_gd(Double ****err, Int out[16]);
  Double xGet_min_d(Double ****err, Int old_d[16], Int new_d[16], Double currSAD);
  Void xUpdateSequenceFilters_B(TComSlice* pcSlice, Double ****err, Int combination[16]);
  Void xUpdateSequenceFilters_B_pred(TComSlice* pcSlice, Double ****err, Int combination[16]);
  Void xGetInterpolatedPixelArray(Int out[16], Pel* imgY, Int x, Int y, Int stride, Int img_width, Int img_height, UInt sub_pos);
  Double  xGetDCdiff(TComSlice* rpcSlice, RefPicList list, Int ref);
  Int     xGet_thDC(TComSlice* rpcSlice);
  Void    xBlockDC(TComSlice* rpcSlice, RefPicList list, Int *DCmin, Int *DCmax, Int noSamples);
  Void    xResetOffsets(TComSlice* pcSlice);
  Void    xResetFirstPassSubpelOffset(Int list);

#if USE_DIAGONAL_FILT==1
  Void xInitDiagonalFilter(Int Tap);
  Void xSingleDiagonalFilt(Int sub_pos, Int filterIndD1, Int dir);
  Void xDoubleDiagonalFilt(Int sub_pos, Int filterIndD1, Int filterIndD2);
#endif

public:
  TEncSIFO();
  virtual ~TEncSIFO();

  Void    init								( TEncTop* pcEncTop, Int Tap);
  Void    initEncSIFO				  ( TComSlice*& rpcSlice );

  Void initSIFOFilters(Int Tap);     
  Void initSeparableFilter(Int Tap);
  Void ComputeFiltersAndOffsets(TComPic*& rpcPic);
  Void    setFirstPassSubpelOffset(RefPicList iRefList, TComSlice* pcSlice);
#if PRINT_OFFSETS==1
  Void    printSIFOOffsets(TComSlice* pcSlice);
#endif
};


#endif // __TENCSIFO__
