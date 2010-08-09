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

/** \file	        TComEdgeBased.cpp
    \brief		Edge based prediction class 
*/

#include "CommonDef.h"
#include "TComEdgeBased.h"
#include "TComDataCU.h"
#include "TComPic.h"

// ====================================================================================================================
// Constructor / destructor / initialize
// ====================================================================================================================

TComEdgeBased::TComEdgeBased() 
{ 
  pred_precision = 6;
}

TComEdgeBased::~TComEdgeBased() { }

Void TComEdgeBased::setEdgePredictionEnable(Bool p_edge_prediction_enable)
{
  this->edge_prediction_enable = p_edge_prediction_enable;
}

Void TComEdgeBased::setThreshold(Int p_threshold_edge_detection)
{
  this->threshold_edge_detection = p_threshold_edge_detection;
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

//-----------------------------------------------------------------------------
/*!
* \brief Right bit shift with rounding
*
* \param[in] val integer value
* \param[in] b number of shifts
* \return shifted value
*/
//-----------------------------------------------------------------------------
inline int TComEdgeBased::shift_right_round(int val, int b) 
{
  return (val + (1 << (b-1))) >> b;
}

//-----------------------------------------------------------------------------
/*!
* \brief Integer power of 2
*
* \param[in] x power
* \return 2 to the power x
*/
//-----------------------------------------------------------------------------
inline int TComEdgeBased::power2(int x) 
{
  return 1 << x;
}

//-----------------------------------------------------------------------------
/*!
* \brief  Edge detection in the neighboring blocks and prediction calculation
*
* \param[in] iHeight       Height of the partition unit
* \param[in] iWidth        Width of the partition unit
* \param[in] uiStride      Horizontal size of the coding unit
* \param[in] iPicStride    Horizontal size of the source buffer
* \param[in] Src           Source buffer
* \param[in] Dst           To store the values of the prediction
*
* \return 1 if an edge is detected, 0 otherwise
*/
//-----------------------------------------------------------------------------
Int TComEdgeBased::intrapred_luma_edge(Int iHeight, Int iWidth, Int uiStride, Int iSrcStride, Int* Src, Pel* Dst)
{
  int dir_max_x = 0, dir_max_y = 0;
  int norm_max = 0;
  int a, b;
  int edge_pred_computed;
  int a_start, a_end, b_start, b_end;
  Pel* pred = Dst;
  

  //////////////////////////////////////////////
  //Compute edge vectors and find the maximum
  //////////////////////////////////////////////
  
  if (above_flag && left_flag)
  {
    a_start = 1;
    b_start = 1;
  }
  else
  {
    b_start = (above_flag) ? 5 : 0;
    a_start = (left_flag) ? 5 : 0;
  }
  
  b_end = (above_flag) ? ((above_right_flag) ? 2*iWidth+3 : iWidth+3 ) : 0;
  a_end = (left_flag)  ? ((left_below_flag) ? 2*iHeight+3 : iHeight+3) : 0;
  
  if (above_flag && !left_flag)
  {
    a_start = 1;
    a_end = 3;
  }

  if( !above_flag && left_flag )
  {
    b_start = 1;
    b_end = 3;
  }
  
  for (a = a_start; a < a_end; a++)
  {
    int b_start_tmp = (a < 3) ? b_start : 1;
    int b_end_tmp = (a < 3) ? b_end : 3;
    
    for (b = b_start_tmp; b < b_end_tmp; b++)
    {
      // Compute gradiants using Sobel filter
      int diag1 = Clip3(0, 255, shift_right_round(Src[(a+1)*iSrcStride+(b+1)], g_uiBitIncrement)) - Clip3(0, 255, shift_right_round(Src[(a-1)*iSrcStride+(b-1)], g_uiBitIncrement));
      int diag2 = Clip3(0, 255, shift_right_round(Src[(a-1)*iSrcStride+(b+1)], g_uiBitIncrement)) - Clip3(0, 255, shift_right_round(Src[(a+1)*iSrcStride+(b-1)], g_uiBitIncrement));
      int grad_x = diag1 + diag2 + 2 * (Clip3(0, 255, shift_right_round(Src[a*iSrcStride+(b+1)], g_uiBitIncrement)) - Clip3(0, 255, shift_right_round(Src[a*iSrcStride+(b-1)], g_uiBitIncrement)));
      int grad_y = diag1 - diag2 + 2 * (Clip3(0, 255, shift_right_round(Src[(a+1)*iSrcStride+b], g_uiBitIncrement)) - Clip3(0, 255, shift_right_round(Src[(a-1)*iSrcStride+b], g_uiBitIncrement)));
      int dir_x = (-grad_y);
      int dir_y = grad_x;
      int norm = dir_x * dir_x + dir_y * dir_y;
      
      if (norm > norm_max)
      {
        if(check_direction_edge_vector(b-4, a-4, &dir_x, &dir_y, iHeight, iWidth))
        {
          dir_max_x = dir_x;
          dir_max_y = dir_y;
          norm_max = norm;
        }
      }
    }
  }
  
  //If the direction is vertical or horizontal, the mode is not used
  //because the same prediction can be obtained with VERT_PRED or HOR_PRED
  edge_pred_computed = (norm_max >= threshold_edge_detection && dir_max_x != 0 && dir_max_y != 0);

  int edge_dir_x, edge_dir_y;

  if (edge_pred_computed)
  {
    int sign = dir_max_x >> 31;
    edge_dir_x = (dir_max_x ^ sign) - sign; // take absolute value
    edge_dir_y = (dir_max_y ^ sign) - sign; // swap sign if dir_max_x was negative
  }
  else
  {
    edge_dir_x = 0;
    edge_dir_y = 0;
  }
  
  int limit_up_low = (left_flag) ? -1 : 0;
  int limit_up_high = b_end-4;
  int limit_left_low = (above_flag) ? -1 : 0;
  int limit_left_high = a_end-4;

  int max_value = 1<<(g_uiBitDepth + g_uiBitIncrement);

  /////////////////////////////////////////////////////////////////////////////
  // Calculate edge based prediction
  /////////////////////////////////////////////////////////////////////////////
  
  if (edge_pred_computed)
  {
    int k, l;
    int delta_x, delta_y[MAX_CU_SIZE];
    int weigh_left, weigh_right, weigh_up[MAX_CU_SIZE], weigh_down[MAX_CU_SIZE];
    int floor_dx, ceil_dx, floor_dy[MAX_CU_SIZE], ceil_dy[MAX_CU_SIZE];

    for(l=0; l<iHeight; l++)
    {
      for(k=0; k<iWidth; k++)
      {
        int sum_weights = 0;
        int pred_pix = 0;
        int mask = power2(pred_precision) - 1;
        
        if (above_flag && edge_dir_y != 0)
        {
          if(k == 0)  //Same values for one row
          {
            delta_x = ( (l+1) * (edge_dir_x << pred_precision) + (edge_dir_y >> 1) ) / edge_dir_y;
            weigh_left = delta_x & mask;
            weigh_right = power2(pred_precision) - weigh_left;
            floor_dx = delta_x >> pred_precision;
            ceil_dx = floor_dx + (weigh_left != 0);
          }
          
          //Check the position of the pixel
          if( (k-floor_dx <= limit_up_high) && (k-ceil_dx >= limit_up_low) )
          {
            pred_pix += shift_right_round(( weigh_left * Src[3*iSrcStride+(4+k-ceil_dx)] + weigh_right * Src[3*iSrcStride+(4+k-floor_dx)] ), pred_precision);
            sum_weights += 1;
          }
        }
        if (left_flag && edge_dir_x != 0)
        {
          if(l == 0)  //Same values for one column
          {
            delta_y[k] = ( (k+1) * (edge_dir_y << pred_precision) + (edge_dir_x >> 1) ) / edge_dir_x;
            weigh_up[k] = delta_y[k] & mask;
            weigh_down[k] = power2(pred_precision) - weigh_up[k];
            floor_dy[k] = delta_y[k] >> pred_precision;
            ceil_dy[k] = floor_dy[k] + (weigh_up[k] != 0);
          }
          
          //Check the position of the pixel
          if( (l-ceil_dy[k] >= limit_left_low) && (l-floor_dy[k] <= limit_left_high) )
          {
            pred_pix += shift_right_round(( weigh_up[k] * Src[(4+l-ceil_dy[k])*iSrcStride+3] + weigh_down[k] * Src[(4+l-floor_dy[k])*iSrcStride+3] ), pred_precision);
            sum_weights += 1;
          }
        }
        
        //Store prediction
        if (sum_weights == 2)
          pred_pix = pred_pix / 2;
        else if (sum_weights == 0)
        {
          if (k==0 && l==0 && !above_flag && left_flag)
            pred_pix = Src[4*iSrcStride+3];
          else if (k==0 && l==0 && above_flag && !left_flag)
            pred_pix = Src[3*iSrcStride+4];
          else if (k==0 && l==0 && above_flag && left_flag)
            pred_pix = Src[3*iSrcStride+3];
          else if (k==0)
            pred_pix = (pred-uiStride)[k];
          else if (l==0)
            pred_pix = pred[k-1];
          else
            pred_pix = ((pred-uiStride)[k-1] + (pred-uiStride)[k] + pred[k-1]) / 3;
        }
        
        pred[k] = (Pel)Clip3(0, max_value, pred_pix);
      }
      pred += uiStride;
    }
  }
  
  this->edge_detected = edge_pred_computed;

  return edge_pred_computed;
}

//-----------------------------------------------------------------------------
/*!
* \brief Check if the computed edge vector crosses the block
*
* \param[in] ref_x       Horizontal position of the reference
* \param[in] ref_y       Vertical position of the reference
* \param[in] dir_x       Horizontal coordinate of the vector
* \param[in] dir_y       Vertical coordinate of the vector
* \param[in] iHeight     Height of the partition unit
* \param[in] iWidth      Width of the partition unit
*/
//-----------------------------------------------------------------------------
Int TComEdgeBased::check_direction_edge_vector(const Int ref_x, const Int ref_y, Int *dir_x, Int *dir_y, Int iHeight, Int iWidth)
{
  int dist_block, first_dist_block = 0;
  int delta_x, delta_y;
  int cross_block = 0;
  int cross_block_row, cross_block_col;
  int tmp_dir_x = *dir_x;
  int tmp_dir_y = *dir_y;
  
  //Check the position of the reference pixel
  //to determine if the vector is from the blocks up or left
  //For vectors up
  if (ref_y == -3 || ref_y == -2)
  {
    if (tmp_dir_y == 0)
      return 0;
  }
  else    //For vectors left
  {
    assert(ref_x == -3 || ref_x == -2);
    if (tmp_dir_x == 0)
      return 0;
  }
    
  // Check if the direction crosses the first and last row of the block
  if(tmp_dir_y != 0)
  {
    if (tmp_dir_y < 0)
    {
      tmp_dir_y = -tmp_dir_y;
      tmp_dir_x = -tmp_dir_x;
    }
    first_dist_block = -ref_y;
    delta_x = first_dist_block * tmp_dir_x;
    cross_block_row = ( ((ref_x * tmp_dir_y + delta_x) >= 0) && ((ref_x * tmp_dir_y + delta_x) < (iWidth * tmp_dir_y)) );
    first_dist_block = -ref_y + iHeight - 1;
    delta_x = first_dist_block * tmp_dir_x;
    cross_block_row = cross_block_row || ( ((ref_x * tmp_dir_y + delta_x) >= 0) && ((ref_x * tmp_dir_y + delta_x) < (iWidth * tmp_dir_y)) );
  }
  else cross_block_row = 0;
    
  // Check if the direction crosses the first and last column of the block
  if(tmp_dir_x != 0)
  {
    if (tmp_dir_x < 0)
    {
      tmp_dir_x = -tmp_dir_x;
      tmp_dir_y = -tmp_dir_y;
    }
    first_dist_block = -ref_x;
    delta_y = first_dist_block * tmp_dir_y;
    cross_block_col = ( ((ref_y * tmp_dir_x + delta_y) >= 0) && ((ref_y * tmp_dir_x + delta_y) < (iHeight * tmp_dir_x)) );
    first_dist_block = -ref_x + iWidth - 1;
    delta_y = first_dist_block * tmp_dir_y;
    cross_block_col = cross_block_col || ( ((ref_y * tmp_dir_x + delta_y) >= 0) && ((ref_y * tmp_dir_x + delta_y) < (iHeight * tmp_dir_x)) );
  }
  else cross_block_col = 0;
  
  cross_block = cross_block_row || cross_block_col;
    
  return cross_block;
}


//-----------------------------------------------------------------------------
/*!
* \brief Store the pixels from the neighboring blocks for edge based prediction
*
* \param[in] pcCU                  Pointer ot current CU
* \param[in] uiZorderIdxInPart     Index of the prediction partition
* \param[in] uiPartDepth           Current depth of the CU
* \param[in] piEdgeBasedBuf        Pointer to the storing buffer
*/
//-----------------------------------------------------------------------------
Void TComEdgeBased::initEdgeBasedBuffer( TComDataCU* pcCU, UInt uiZorderIdxInPart, UInt uiPartDepth, Int* piEdgeBasedBuf )
{
  Pel*  piRoiOrigin;
  Pel*  piRoiTemp;
  Int*  piEdgeBasedTemp;
  UInt  uiCuWidth   = pcCU->getWidth(0) >> uiPartDepth;
  UInt  uiCuHeight  = pcCU->getHeight(0)>> uiPartDepth;
  UInt  uiWidth     = (uiCuWidth<<1) + 4;
  UInt  uiHeight    = (uiCuHeight<<1) + 4;
  Int   iPicStride  = pcCU->getPic()->getStride();
  Int   i;
  Bool  bAboveFlag      = false;
  Bool  bAboveRightFlag = false;
  Bool  bLeftFlag       = false;
  Bool  bBelowLeftFlag  = false;

  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB, uiPartDum;

  pcCU->deriveLeftRightTopIdxAdi( uiPartIdxLT, uiPartIdxRT, uiZorderIdxInPart, uiPartDepth );
  pcCU->deriveLeftBottomIdxAdi  ( uiPartIdxLB,              uiZorderIdxInPart, uiPartDepth );

  if( pcCU->getPUAbove        ( uiPartDum,             uiPartIdxLT ) ) bAboveFlag      = true;
  if( pcCU->getPUAboveRightAdi( uiPartDum, uiCuWidth,  uiPartIdxRT ) ) bAboveRightFlag = true;
  if( pcCU->getPULeft         ( uiPartDum,             uiPartIdxLT ) ) bLeftFlag       = true;
  if( pcCU->getPUBelowLeftAdi ( uiPartDum, uiCuHeight, uiPartIdxLB ) ) bBelowLeftFlag  = true;

  above_flag       = bAboveFlag;
  left_flag        = bLeftFlag;
  above_right_flag = bAboveRightFlag;
  left_below_flag  = bBelowLeftFlag;

  piRoiOrigin = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiZorderIdxInPart);

  piEdgeBasedTemp   = piEdgeBasedBuf;

  Int iDCValue = ( 1<<( g_uiBitDepth + g_uiBitIncrement - 1) );

  /*for (i=0;i<uiWidth;i++)
  {
    piEdgeBasedTemp[i]           = iDCValue;
    piEdgeBasedTemp[i+uiWidth]   = iDCValue;
    piEdgeBasedTemp[i+2*uiWidth] = iDCValue;
    piEdgeBasedTemp[i+3*uiWidth] = iDCValue;
  }
  for (i=4;i<uiHeight;i++)
  {
    piEdgeBasedTemp[i*uiWidth]   = iDCValue;
    piEdgeBasedTemp[i*uiWidth+1] = iDCValue;
    piEdgeBasedTemp[i*uiWidth+2] = iDCValue;
    piEdgeBasedTemp[i*uiWidth+3] = iDCValue;
    }*/

  piRoiTemp=piRoiOrigin;

  if (bAboveFlag)
  {

    // Samples above
    piRoiTemp = piRoiOrigin - 4 * iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piEdgeBasedTemp[4+i]           = piRoiTemp[i];
    piRoiTemp += iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piEdgeBasedTemp[4+i+uiWidth]   = piRoiTemp[i];
    piRoiTemp += iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piEdgeBasedTemp[4+i+2*uiWidth] = piRoiTemp[i];
    piRoiTemp += iPicStride;
    for (i=0;i<uiCuWidth;i++)
      piEdgeBasedTemp[4+i+3*uiWidth] = piRoiTemp[i];

    // Samples above right
    if (bAboveRightFlag){
      piRoiTemp = piRoiOrigin - 4 * iPicStride + uiCuWidth;
      for (i=0;i<uiCuWidth;i++)
        piEdgeBasedTemp[4+uiCuWidth+i]           = piRoiTemp[i];
      piRoiTemp += iPicStride;
      for (i=0;i<uiCuWidth;i++)
        piEdgeBasedTemp[4+uiCuWidth+i+uiWidth]   = piRoiTemp[i];
      piRoiTemp += iPicStride;
      for (i=0;i<uiCuWidth;i++)
        piEdgeBasedTemp[4+uiCuWidth+i+2*uiWidth] = piRoiTemp[i];
      piRoiTemp += iPicStride;
      for (i=0;i<uiCuWidth;i++)
        piEdgeBasedTemp[4+uiCuWidth+i+3*uiWidth] = piRoiTemp[i];
    }
    else {
      for (i=0;i<uiCuWidth;i++)
      {
        piEdgeBasedTemp[4+uiCuWidth+i]           = piEdgeBasedTemp[4+uiCuWidth-1];
        piEdgeBasedTemp[4+uiCuWidth+i+uiWidth]   = piEdgeBasedTemp[4+uiCuWidth-1+uiWidth];
        piEdgeBasedTemp[4+uiCuWidth+i+2*uiWidth] = piEdgeBasedTemp[4+uiCuWidth-1+2*uiWidth];
        piEdgeBasedTemp[4+uiCuWidth+i+3*uiWidth] = piEdgeBasedTemp[4+uiCuWidth-1+3*uiWidth];
      }
    }

    //Samples above left
    if (bLeftFlag){
      for (i=0; i<4; i++)
      {
        piRoiTemp = piRoiOrigin - (4-i) * iPicStride - 4;
        piEdgeBasedTemp[0+i*uiWidth] = piRoiTemp[0];
        piEdgeBasedTemp[1+i*uiWidth] = piRoiTemp[1];
        piEdgeBasedTemp[2+i*uiWidth] = piRoiTemp[2];
        piEdgeBasedTemp[3+i*uiWidth] = piRoiTemp[3];
      }
    }

  }

  if (bLeftFlag)
  {

    // Samples left
    piRoiTemp = piRoiOrigin - 4;
    for (i=0;i<uiCuHeight;i++)
    {
      piEdgeBasedTemp[(4+i)*uiWidth]   = piRoiTemp[0];
      piEdgeBasedTemp[(4+i)*uiWidth+1] = piRoiTemp[1];
      piEdgeBasedTemp[(4+i)*uiWidth+2] = piRoiTemp[2];
      piEdgeBasedTemp[(4+i)*uiWidth+3] = piRoiTemp[3];
      piRoiTemp += iPicStride;
    }

    // Samples below left
    if (bBelowLeftFlag){
      for (i=0;i<uiCuHeight;i++)
      {
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth]   = piRoiTemp[0];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+1] = piRoiTemp[1];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+2] = piRoiTemp[2];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+3] = piRoiTemp[3];
        piRoiTemp += iPicStride;
      }
    }
    else {
      for (i=0;i<uiCuHeight;i++)
      {
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth]   = piEdgeBasedTemp[(4+uiCuHeight-1)*uiWidth];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+1] = piEdgeBasedTemp[(4+uiCuHeight-1)*uiWidth+1];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+2] = piEdgeBasedTemp[(4+uiCuHeight-1)*uiWidth+2];
        piEdgeBasedTemp[(4+uiCuHeight+i)*uiWidth+3] = piEdgeBasedTemp[(4+uiCuHeight-1)*uiWidth+3];
      }
    }

  }

}
