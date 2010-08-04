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

/** \file		TComEdgeBased.h
    \brief		Edge based prediction class (header)
*/

#ifndef __TCOMEDGEBASED__
#define __TCOMEDGEBASED__

#include "CommonDef.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class TComDataCU;

class TComEdgeBased
{
private:
  Bool edge_prediction_enable;                      // 1 if edge detection is enabled, 0 otherwise
  Int threshold_edge_detection;                     // Threshold for the detection of edges with the Sobel operator
  Int pred_precision;

  Bool edge_detected;                               // 1 if an edge is detected in the neighboring blocks, 0 otherwise

  // Flags to record the availability of the surrounding pixels for the current prediction unit partition
  Bool above_flag;
  Bool above_right_flag;
  Bool left_flag;
  Bool left_below_flag;

  inline int shift_right_round(int val, int b);
  inline int power2(int x);

public:
  TComEdgeBased();
  ~TComEdgeBased();

  Void setEdgePredictionEnable(Bool edge_prediction_enable);
  Void setThreshold(Int threshold_edge_detection);

  Bool get_edge_prediction_enable () { return edge_prediction_enable; }
  Bool get_edge_detected          () { return edge_detected; }

  Void initEdgeBasedBuffer(TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piEdgeBasedBuf);
  Int intrapred_luma_edge(Int iHeight, Int iWidth, Int uiStride, Int iSrcStride, Int* Src, Pel* pred);
  Int check_direction_edge_vector(const Int ref_x, const Int ref_y, Int *dir_x, Int *dir_y, Int iHeight, Int iWidth);

};


#endif //  __TCOMEDGEBASED__
