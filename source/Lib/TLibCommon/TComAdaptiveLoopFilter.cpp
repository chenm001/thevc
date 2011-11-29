/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TComAdaptiveLoopFilter.cpp
    \brief    adaptive loop filter class
*/

#include "TComAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Tables
// ====================================================================================================================

//Shape0: Star5x5
Int TComAdaptiveLoopFilter::patternShape0Sym[17] = 
{
  0,    1,    2, 
     3, 4, 5,
  6, 7, 8, 7, 6,
     5, 4, 3, 
  2,    1,    0
};

Int TComAdaptiveLoopFilter::weightsShape0Sym[10] = 
{
  2,    2,    2,    
     2, 2, 2,        
  2, 2, 1, 1
};

Int TComAdaptiveLoopFilter::patternShape0Sym_Quart[29] = 
{
  0,  0,  0,  1,  0,  2,  0,  3,  0,  0,  0,
  0,  0,  0,  0,  4,  5,  6,  0,  0,  0,  0,
  0,  0,  0,  7,  8,  9, 10
};
//Shape1: Cross11x5
Int TComAdaptiveLoopFilter::patternShape1Sym[15] = 
{
                 0,
                 1,
  2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2,                      
                 1,
                 0
};

Int TComAdaptiveLoopFilter::weightsShape1Sym[9] = 
{                      
                 2,
                 2,
  2, 2, 2, 2, 2, 1, 1
};

Int TComAdaptiveLoopFilter::patternShape1Sym_Quart[29] = 
{
  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,
  3,  4,  5,  6,  7,  8,  9
};

Int TComAdaptiveLoopFilter::m_pattern9x9Sym[39] = 
{
                   0,
               1,  2,  3,
           4,  5,  6,  7,  8,
       9, 10, 11, 12, 13, 14, 15,
      16, 17, 18, 19, 18, 17, 16,
      15, 14, 13, 12, 11, 10,  9, 
           8,  7,  6,  5,  4,
               3,  2,  1,
                   0
};
 
Int TComAdaptiveLoopFilter::m_weights9x9Sym[21] = 
{
               2,  2,  2,   
           2,  2,  2,  2,  2, 
       2,  2,  2,  2,  2,  2,  2,  
   2,  2,  2,  2,  1,  1
};

Int TComAdaptiveLoopFilter::m_pattern9x9Sym_Quart[42] = 
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  1,  2,  3,  0,  0,  0,
   0,  0,  4,  5,  6,  7,  8,  0,  0,  
   0,  9, 10, 11, 12, 13, 14, 15,  0,
  16, 17, 18, 19, 20, 21
};

Int TComAdaptiveLoopFilter::m_pattern7x7Sym[25] = 
{
                   0,
             1,  2,  3,
       4,  5,  6,  7,  8,
       9, 10, 11, 12, 11, 10, 9,
           8,  7,  6,  5,  4,
           3,  2,  1,
             0
};

Int TComAdaptiveLoopFilter::m_weights7x7Sym[14] = 
{
                  2,  
              2,  2,  2,   
        2,  2,  2,  2,  2,    
      2,  2,  2,  1,  1
};

Int TComAdaptiveLoopFilter::m_pattern7x7Sym_Quart[42] = 
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  1,  0,  0,  0,  0,
   0,  0,  0,  2,  3,  4,  0,  0,  0,
   0,  0,  5,  6,  7,  8,  9,  0,  0,  
   0, 10, 11, 12, 13, 14,  
};

Int TComAdaptiveLoopFilter::m_pattern5x5Sym[13] = 
{
                   0,
             1,  2,  3,
       4,  5,  6,  5,  4,
               3,  2,  1,
             0
};

Int TComAdaptiveLoopFilter::m_weights5x5Sym[8] = 
{
           2, 
        2, 2, 2,
     2, 2, 1, 1
};

Int TComAdaptiveLoopFilter::m_pattern5x5Sym_Quart[45] = 
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  1,  0,  0,  0,  0,
   0,  0,  0,  2,  3,  4,  0,  0,  0,
   0,  0,  5,  6,  7,  8,  0,  0,  0,  
};

Int TComAdaptiveLoopFilter::m_pattern9x9Sym_9[39] = 
{
              12, 13, 14,  
          20, 21, 22, 23, 24, 
      28, 29, 30, 31, 32, 33, 34,      
  36, 37, 38, 39, 40, 39, 38, 37, 36, 
      34, 33, 32, 31, 30, 29, 28,  
          24, 23, 22, 21, 20, 
              14, 13, 12,
};

Int TComAdaptiveLoopFilter::m_pattern9x9Sym_7[25] = 
{    
               13,   
           21, 22, 23,  
       29, 30, 31, 32, 33,       
   37, 38, 39, 40, 39, 38, 37,  
       33, 32, 31, 30, 29,   
           23, 22, 21,  
               13  
                     
};

Int TComAdaptiveLoopFilter::m_pattern9x9Sym_5[13] = 
{
          22, 
      30, 31, 32,    
  38, 39, 40, 39, 38, 
      32, 31, 30, 
          22,  
 };

// Shape0
Int TComAdaptiveLoopFilter::pattern11x5SymShape0[17] = 
{
  3,    5,    7,
    15,16,17,
  25,26,27,26,25,
    17,16,15, 
  7,    5,    3 
};
// Shape1
Int TComAdaptiveLoopFilter::pattern11x5SymShape1[15] = 
{
                  5, 
                 16, 
  22,23,24,25,26,27,26,25,24,23,22,
                 16, 
                  5  
};

Int TComAdaptiveLoopFilter::pattern11x5Sym11x5[55] =
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,
  11,12,13,14,15,16,17,18,19,20,21,
  22,23,24,25,26,27,26,25,24,23,22,
  21,20,19,18,17,16,15,14,13,12,11,
  10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

Int* TComAdaptiveLoopFilter::patternTabFiltShapes[NO_TEST_FILT] =
{
  pattern11x5SymShape0, pattern11x5SymShape1
}; 

Int* TComAdaptiveLoopFilter::patternTabShapes[NO_TEST_FILT] =
{
  patternShape0Sym, patternShape1Sym, pattern11x5Sym11x5
}; 

Int* TComAdaptiveLoopFilter::patternMapTabShapes[NO_TEST_FILT] =
{
  patternShape0Sym_Quart, patternShape1Sym_Quart
};

Int* TComAdaptiveLoopFilter::weightsTabShapes[NO_TEST_FILT] =
{
  weightsShape0Sym, weightsShape1Sym
};

Int* TComAdaptiveLoopFilter::m_patternTab_filt[NO_TEST_FILT] =
{
  m_pattern9x9Sym_9, m_pattern9x9Sym_7, m_pattern9x9Sym_5
}; 

Int* TComAdaptiveLoopFilter::m_patternTab[NO_TEST_FILT] =
{
  m_pattern9x9Sym, m_pattern7x7Sym, m_pattern5x5Sym
}; 

Int* TComAdaptiveLoopFilter::m_patternMapTab[NO_TEST_FILT] =
{
  m_pattern9x9Sym_Quart, m_pattern7x7Sym_Quart, m_pattern5x5Sym_Quart
};

Int* TComAdaptiveLoopFilter::m_weightsTab[NO_TEST_FILT] =
{
  m_weights9x9Sym, m_weights7x7Sym, m_weights5x5Sym
};

Int TComAdaptiveLoopFilter::m_flTab[NO_TEST_FILT] =
{
  9/2, 7/2, 5/2
};

Int TComAdaptiveLoopFilter::m_sqrFiltLengthTab[NO_TEST_FILT] =
{
  10, 9
};

// Shape0
Int depthIntShape0Sym[10] = 
{
  1,    3,    1,
     3, 4, 3, 
  3, 4, 5, 5                 
};
// Shape1
Int depthIntShape1Sym[9] = 
{
                 9,
                10,
  6, 7, 8, 9,10,11,11                        
};

Int depthInt9x9Sym[21] = 
{
           5, 6, 5, 
        5, 6, 7, 6, 5,
     5, 6, 7, 8, 7, 6, 5,
  5, 6, 7, 8, 9, 9 
};

Int depthInt7x7Sym[14] = 
{
           4, 
        4, 5, 4, 
     4, 5, 6, 5, 4, 
  4, 5, 6, 7, 7 
};

Int depthInt5x5Sym[8] = 
{
        3,   
     3, 4, 3,
  3, 4, 5, 5  
};

Int* pDepthIntTabShapes[NO_TEST_FILT] =
{ 
  depthIntShape0Sym, depthIntShape1Sym
};

Int* pDepthIntTab[NO_TEST_FILT] =
{
  depthInt5x5Sym, depthInt7x7Sym, depthInt9x9Sym
};

// scaling factor for quantization of filter coefficients (9x9)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag9x9[41] =
{
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (7x7)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag7x7[25] =
{
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (5x5)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag5x5[13] =
{
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 1
};

const Int TComAdaptiveLoopFilter::m_aiSymmetricMag9x7[32] =
{
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1
};

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

#if F747_APS
const AlfCUCtrlInfo& AlfCUCtrlInfo::operator= (const AlfCUCtrlInfo& src)
{
  this->cu_control_flag = src.cu_control_flag;
  this->alf_max_depth   = src.alf_max_depth;
  this->num_alf_cu_flag = src.num_alf_cu_flag;
  this->alf_cu_flag     = src.alf_cu_flag;
  return *this;
}
#endif

TComAdaptiveLoopFilter::TComAdaptiveLoopFilter()
{
  m_pcTempPicYuv = NULL;
  m_pSlice       = NULL;
  m_iSGDepth     = 0;
  m_piSliceSUMap = NULL;
}

Void TComAdaptiveLoopFilter:: xError(const char *text, int code)
{
  fprintf(stderr, "%s\n", text);
  exit(code);
}

Void TComAdaptiveLoopFilter:: no_mem_exit(const char *where)
{
  char errortext[200];
  sprintf(errortext, "Could not allocate memory: %s",where);
  xError (errortext, 100);
}

Void TComAdaptiveLoopFilter::initMatrix_imgpel(imgpel ***m2D, int d1, int d2)
{
  int i;
  
  if(!(*m2D = (imgpel **) calloc(d1, sizeof(imgpel *))))
    FATAL_ERROR_0("initMatrix_imgpel: memory allocation problem\n", -1);
  if(!((*m2D)[0] = (imgpel *) calloc(d1 * d2, sizeof(imgpel))))
    FATAL_ERROR_0("initMatrix_imgpel: memory allocation problem\n", -1);
  
  for(i = 1; i < d1; i++)
    (*m2D)[i] = (*m2D)[i-1] + d2;
}

Void TComAdaptiveLoopFilter::initMatrix_int(int ***m2D, int d1, int d2)
{
  int i;
  
  if(!(*m2D = (int **) calloc(d1, sizeof(int *))))
    FATAL_ERROR_0("initMatrix_int: memory allocation problem\n", -1);
  if(!((*m2D)[0] = (int *) calloc(d1 * d2, sizeof(int))))
    FATAL_ERROR_0("initMatrix_int: memory allocation problem\n", -1);
  
  for(i = 1; i < d1; i++)
    (*m2D)[i] = (*m2D)[i-1] + d2;
}

Void TComAdaptiveLoopFilter::destroyMatrix_int(int **m2D)
{
  if(m2D)
  {
    if(m2D[0])
      free(m2D[0]);
    else
      FATAL_ERROR_0("destroyMatrix_int: memory free problem\n", -1);
    free(m2D);
  } 
  else
  {
    FATAL_ERROR_0("destroyMatrix_int: memory free problem\n", -1);
  }
}

Void TComAdaptiveLoopFilter::destroyMatrix_imgpel(imgpel **m2D)
{
  if(m2D)
  {
    if(m2D[0])
      free(m2D[0]);
    else
      FATAL_ERROR_0("destroyMatrix_imgpel: memory free problem\n", -1);
    free(m2D);
  } 
  else
  {
    FATAL_ERROR_0("destroyMatrix_imgpel: memory free problem\n", -1);
  }
}

Void TComAdaptiveLoopFilter::get_mem2Dpel(imgpel ***array2D, int rows, int columns)
{
  int i;
  
  if((*array2D      = (imgpel**)calloc(rows,        sizeof(imgpel*))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");
  if(((*array2D)[0] = (imgpel* )calloc(rows*columns,sizeof(imgpel ))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");
  
  for(i=1 ; i<rows ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns  ;
}

Void TComAdaptiveLoopFilter::free_mem2Dpel(imgpel **array2D)
{
  if (array2D)
  {
    if (array2D[0])
      free (array2D[0]);
    else xError ("free_mem2Dpel: trying to free unused memory",100);
    
    free (array2D);
  }
  else
  {
    xError ("free_mem2Dpel: trying to free unused memory",100);
  }
}

Void TComAdaptiveLoopFilter::initMatrix_double(double ***m2D, int d1, int d2)
{
  int i;
  
  if(!(*m2D = (double **) calloc(d1, sizeof(double *))))
    FATAL_ERROR_0("initMatrix_double: memory allocation problem\n", -1);
  if(!((*m2D)[0] = (double *) calloc(d1 * d2, sizeof(double))))
    FATAL_ERROR_0("initMatrix_double: memory allocation problem\n", -1);
  
  for(i = 1; i < d1; i++)
    (*m2D)[i] = (*m2D)[i-1] + d2;
}

Void TComAdaptiveLoopFilter::initMatrix3D_double(double ****m3D, int d1, int d2, int d3)
{
  int  j;
  
  if(!((*m3D) = (double ***) calloc(d1, sizeof(double **))))
    FATAL_ERROR_0("initMatrix3D_double: memory allocation problem\n", -1);
  
  for(j = 0; j < d1; j++)
    initMatrix_double((*m3D) + j, d2, d3);
}


Void TComAdaptiveLoopFilter::initMatrix4D_double(double *****m4D, int d1, int d2, int d3, int d4)
{
  int  j;
  
  if(!((*m4D) = (double ****) calloc(d1, sizeof(double ***))))
    FATAL_ERROR_0("initMatrix4D_double: memory allocation problem\n", -1);
  
  for(j = 0; j < d1; j++)
    initMatrix3D_double((*m4D) + j, d2, d3, d4);
}


Void TComAdaptiveLoopFilter::destroyMatrix_double(double **m2D)
{
  if(m2D)
  {
    if(m2D[0])
      free(m2D[0]);
    else
      FATAL_ERROR_0("destroyMatrix_double: memory free problem\n", -1);
    free(m2D);
  } 
  else
  {
    FATAL_ERROR_0("destroyMatrix_double: memory free problem\n", -1);
  }
}

Void TComAdaptiveLoopFilter::destroyMatrix3D_double(double ***m3D, int d1)
{
  int i;
  
  if(m3D)
  {
    for(i = 0; i < d1; i++)
      destroyMatrix_double(m3D[i]);
    free(m3D);
  } 
  else
  {
    FATAL_ERROR_0("destroyMatrix3D_double: memory free problem\n", -1);
  }
}


Void TComAdaptiveLoopFilter::destroyMatrix4D_double(double ****m4D, int d1, int d2)
{
  int  j;
  
  if(m4D)
  {
    for(j = 0; j < d1; j++)
      destroyMatrix3D_double(m4D[j], d2);
    free(m4D);
  } 
  else
  {
    FATAL_ERROR_0("destroyMatrix4D_double: memory free problem\n", -1);
  }
}

Void TComAdaptiveLoopFilter::create( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
  if ( !m_pcTempPicYuv )
  {
    m_pcTempPicYuv = new TComPicYuv;
    m_pcTempPicYuv->create( iPicWidth, iPicHeight, uiMaxCUWidth, uiMaxCUHeight, uiMaxCUDepth );
  }
  m_img_height = iPicHeight;
  m_img_width = iPicWidth;
  initMatrix_int(&m_imgY_temp, m_img_height+2*VAR_SIZE+3, m_img_width+2*VAR_SIZE+3);
  initMatrix_int(&m_imgY_ver, m_img_height+2*VAR_SIZE+3, m_img_width+2*VAR_SIZE+3);
  initMatrix_int(&m_imgY_hor, m_img_height+2*VAR_SIZE+3, m_img_width+2*VAR_SIZE+3);
  for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
  {
    get_mem2Dpel(&(m_varImgMethods[i]), m_img_height, m_img_width);
  }

  initMatrix_int(&m_filterCoeffSym, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);
  initMatrix_int(&m_filterCoeffPrevSelected, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
  initMatrix_int(&m_filterCoeffTmp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);      
  initMatrix_int(&m_filterCoeffSymTmp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);   

  UInt uiNumLCUsInWidth   = m_img_width  / uiMaxCUWidth;
  UInt uiNumLCUsInHeight  = m_img_height / uiMaxCUHeight;

  uiNumLCUsInWidth  += ( m_img_width % uiMaxCUWidth ) ? 1 : 0;
  uiNumLCUsInHeight += ( m_img_height % uiMaxCUHeight ) ? 1 : 0;

  m_uiNumCUsInFrame = uiNumLCUsInWidth* uiNumLCUsInHeight; 

  createRegionIndexMap(m_varImgMethods[ALF_RA], m_img_width, m_img_height);
}

Void TComAdaptiveLoopFilter::destroy()
{
  if ( m_pcTempPicYuv )
  {
    m_pcTempPicYuv->destroy();
    delete m_pcTempPicYuv;
  }
  destroyMatrix_int(m_imgY_temp);

  destroyMatrix_int(m_imgY_ver);
  destroyMatrix_int(m_imgY_hor);
  for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
  {
    free_mem2Dpel(m_varImgMethods[i]);
  }
  destroyMatrix_int(m_filterCoeffSym);
  destroyMatrix_int(m_filterCoeffPrevSelected);
  destroyMatrix_int(m_filterCoeffTmp);
  destroyMatrix_int(m_filterCoeffSymTmp);
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Int TComAdaptiveLoopFilter::ALFTapHToTapV(Int tapH)
{
  return min<UInt>(tapH, 7);
}

Int TComAdaptiveLoopFilter::ALFFlHToFlV(Int flH)
{
  return min<UInt>(flH, 7/2);
}

Int TComAdaptiveLoopFilter::ALFTapHToNumCoeff(Int tapH)
{
  Int num_coeff;
  
  num_coeff = (Int)(tapH*tapH)/4 + 2;
  if (tapH == 9)
    num_coeff -= 1;
  else
    assert(tapH < 9);
  
  return num_coeff;
}

// --------------------------------------------------------------------------------------------------------------------
// allocate / free / copy functions
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::allocALFParam(ALFParam* pAlfParam)
{
  pAlfParam->alf_flag = 0;
  
  pAlfParam->coeff        = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->coeff_chroma = new Int[ALF_MAX_NUM_COEF_C];
  
  ::memset(pAlfParam->coeff,        0, sizeof(Int)*ALF_MAX_NUM_COEF   );
  ::memset(pAlfParam->coeff_chroma, 0, sizeof(Int)*ALF_MAX_NUM_COEF_C );
  pAlfParam->coeffmulti = new Int*[NO_VAR_BINS];
  for (int i=0; i<NO_VAR_BINS; i++)
  {
    pAlfParam->coeffmulti[i] = new Int[ALF_MAX_NUM_COEF];
    ::memset(pAlfParam->coeffmulti[i],        0, sizeof(Int)*ALF_MAX_NUM_COEF );
  }
#if !F747_APS
  pAlfParam->num_cus_in_frame = m_uiNumCUsInFrame;
  pAlfParam->num_alf_cu_flag  = 0;
  pAlfParam->alf_cu_flag      = new UInt[(m_uiNumCUsInFrame << ((g_uiMaxCUDepth-1)*2))];
#endif

  pAlfParam->alf_pcr_region_flag = 0;
}

Void TComAdaptiveLoopFilter::freeALFParam(ALFParam* pAlfParam)
{
  assert(pAlfParam != NULL);
  
  if (pAlfParam->coeff != NULL)
  {
    delete[] pAlfParam->coeff;
    pAlfParam->coeff = NULL;
  }
  
  if (pAlfParam->coeff_chroma != NULL)
  {
    delete[] pAlfParam->coeff_chroma;
    pAlfParam->coeff_chroma = NULL;
  }
  for (int i=0; i<NO_VAR_BINS; i++)
  {
    delete[] pAlfParam->coeffmulti[i];
    pAlfParam->coeffmulti[i] = NULL;
  }
  delete[] pAlfParam->coeffmulti;
  pAlfParam->coeffmulti = NULL;
#if !F747_APS
  if(pAlfParam->alf_cu_flag != NULL)
  {
    delete[] pAlfParam->alf_cu_flag;
    pAlfParam->alf_cu_flag = NULL;
  }
#endif
}

Void TComAdaptiveLoopFilter::copyALFParam(ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam)
{
  pDesAlfParam->alf_flag = pSrcAlfParam->alf_flag;
#if !F747_APS
  pDesAlfParam->cu_control_flag = pSrcAlfParam->cu_control_flag;
#endif
  pDesAlfParam->chroma_idc = pSrcAlfParam->chroma_idc;
  pDesAlfParam->num_coeff = pSrcAlfParam->num_coeff;
  pDesAlfParam->realfiltNo_chroma = pSrcAlfParam->realfiltNo_chroma;
  pDesAlfParam->num_coeff_chroma = pSrcAlfParam->num_coeff_chroma;

  pDesAlfParam->alf_pcr_region_flag = pSrcAlfParam->alf_pcr_region_flag;

  ::memcpy(pDesAlfParam->coeff, pSrcAlfParam->coeff, sizeof(Int)*ALF_MAX_NUM_COEF);
  ::memcpy(pDesAlfParam->coeff_chroma, pSrcAlfParam->coeff_chroma, sizeof(Int)*ALF_MAX_NUM_COEF_C);
  pDesAlfParam->realfiltNo = pSrcAlfParam->realfiltNo;
  pDesAlfParam->filtNo = pSrcAlfParam->filtNo;
  ::memcpy(pDesAlfParam->filterPattern, pSrcAlfParam->filterPattern, sizeof(Int)*NO_VAR_BINS);
  pDesAlfParam->startSecondFilter = pSrcAlfParam->startSecondFilter;
  pDesAlfParam->noFilters = pSrcAlfParam->noFilters;
  
  //Coeff send related
  pDesAlfParam->filters_per_group_diff = pSrcAlfParam->filters_per_group_diff; //this can be updated using codedVarBins
  pDesAlfParam->filters_per_group = pSrcAlfParam->filters_per_group; //this can be updated using codedVarBins
  ::memcpy(pDesAlfParam->codedVarBins, pSrcAlfParam->codedVarBins, sizeof(Int)*NO_VAR_BINS);
  pDesAlfParam->forceCoeff0 = pSrcAlfParam->forceCoeff0;
  pDesAlfParam->predMethod = pSrcAlfParam->predMethod;
  for (int i=0; i<NO_VAR_BINS; i++)
  {
    ::memcpy(pDesAlfParam->coeffmulti[i], pSrcAlfParam->coeffmulti[i], sizeof(Int)*ALF_MAX_NUM_COEF);
  }
#if !F747_APS
  pDesAlfParam->num_alf_cu_flag = pSrcAlfParam->num_alf_cu_flag;
  ::memcpy(pDesAlfParam->alf_cu_flag, pSrcAlfParam->alf_cu_flag, sizeof(UInt)*pSrcAlfParam->num_alf_cu_flag);
#endif
}

// --------------------------------------------------------------------------------------------------------------------
// prediction of filter coefficients
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::predictALFCoeff( ALFParam* pAlfParam)
{
  Int i, sum, pred, N;
  const Int* pFiltMag = weightsTabShapes[pAlfParam->realfiltNo];
  N = pAlfParam->num_coeff - 1;
  sum=0;
  for(i=0; i<N-1;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-sum;
  pAlfParam->coeff[N-1]=pred-pAlfParam->coeff[N-1];
}

Void TComAdaptiveLoopFilter::predictALFCoeffChroma( ALFParam* pAlfParam )
{
  Int i, sum, pred, N;
  const Int* pFiltMag = NULL;

  pFiltMag = weightsTabShapes[pAlfParam->realfiltNo_chroma];
  N = pAlfParam->num_coeff_chroma - 1;
  sum=0;
  for(i=0; i<N;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff_chroma[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-(sum-pAlfParam->coeff_chroma[N-1]);
  pAlfParam->coeff_chroma[N-1]=pred-pAlfParam->coeff_chroma[N-1];
}

// --------------------------------------------------------------------------------------------------------------------
// interface function for actual ALF process
// --------------------------------------------------------------------------------------------------------------------

/**
 \param [in, out] pcPic         picture (TComPic) class (input/output)
 \param [in] pcAlfParam    ALF parameter
 \param [in,out] vAlfCUCtrlParam ALF CU control parameters
 \todo   for temporal buffer, it uses residual picture buffer, which may not be safe. Make it be safer.
 */
#if F747_APS
Void TComAdaptiveLoopFilter::ALFProcess(TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam)
{
  assert(m_uiNumSlicesInPic == vAlfCUCtrlParam.size());
#else
Void TComAdaptiveLoopFilter::ALFProcess(TComPic* pcPic, ALFParam* pcAlfParam)
{
#endif
  if(!pcAlfParam->alf_flag)
  {
    return;
  }
  
  TComPicYuv* pcPicYuvRec    = pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;
  if(!m_bUseNonCrossALF)
  {
  pcPicYuvRec   ->copyToPic          ( pcPicYuvExtRec );
  pcPicYuvExtRec->setBorderExtension ( false );
  pcPicYuvExtRec->extendPicBorder    ();
  }

#if F747_APS
  if(m_uiNumSlicesInPic == 1)
  {
    AlfCUCtrlInfo* pcAlfCtrlParam = &(vAlfCUCtrlParam[0]);
    if(pcAlfCtrlParam->cu_control_flag)
    {
      UInt idx = 0;
      for(UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++)
      {
        TComDataCU *pcCU = pcPic->getCU(uiCUAddr);
        setAlfCtrlFlags(pcAlfCtrlParam, pcCU, 0, 0, idx);
      }
    }
  }
  else
  {
    transferCtrlFlagsFromAlfParam(vAlfCUCtrlParam);
  }
  xALFLuma_qc(pcPic, pcAlfParam, vAlfCUCtrlParam, pcPicYuvExtRec, pcPicYuvRec);
#else
  if(pcAlfParam->cu_control_flag)
  {
    if(m_uiNumSlicesInPic == 1)
    {
      UInt idx = 0;
      for(UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++)
      {
        TComDataCU *pcCU = pcPic->getCU(uiCUAddr);
        setAlfCtrlFlags(pcAlfParam, pcCU, 0, 0, idx);
      }
    }
    else
    {
      transferCtrlFlagsFromAlfParam(pcAlfParam);
    }
  }
  xALFLuma_qc(pcPic, pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);
#endif  
  if(pcAlfParam->chroma_idc)
  {
    predictALFCoeffChroma(pcAlfParam);
    xALFChroma( pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// ALF for luma
// --------------------------------------------------------------------------------------------------------------------
#if F747_APS
Void TComAdaptiveLoopFilter::xALFLuma_qc(TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam,TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
#else
Void TComAdaptiveLoopFilter::xALFLuma_qc(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
#endif
{
  Int    LumaStride = pcPicDec->getStride();
  imgpel* pDec = (imgpel*)pcPicDec->getLumaAddr();
  imgpel* pRest = (imgpel*)pcPicRest->getLumaAddr();
  
  //Decode and reconst filter coefficients
  DecFilter_qc(pDec,pcAlfParam,LumaStride);
  //set maskImg using cu adaptive one.

  m_uiVarGenMethod = pcAlfParam->alf_pcr_region_flag;
  m_imgY_var       = m_varImgMethods[m_uiVarGenMethod];

  memset(m_imgY_temp[0],0,sizeof(int)*(m_img_height+2*VAR_SIZE)*(m_img_width+2*VAR_SIZE));
  if(!m_bUseNonCrossALF)
  {
    calcVar(0, 0, m_imgY_var, pDec, FILTER_LENGTH/2, VAR_SIZE, m_img_height, m_img_width, LumaStride);

#if F747_APS
  Bool bCUCtrlEnabled = false;
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(m_uiNumSlicesInPic > 1)
    {
      if(!m_pSlice[s].isValidSlice()) continue;
    }
    bCUCtrlEnabled = ( vAlfCUCtrlParam[s].cu_control_flag == 1);
  }

  if(bCUCtrlEnabled)  
#else
  if(pcAlfParam->cu_control_flag)
#endif
  {
    xCUAdaptive_qc(pcPic, pcAlfParam, pRest, pDec, LumaStride);
  }  
  else
  {
    //then do whole frame filtering
    filterFrame(pRest, pDec, pcAlfParam->realfiltNo, LumaStride);
  }
  }
  else
  {
    for(UInt s=0; s< m_uiNumSlicesInPic; s++)
    {
      CAlfSlice* pSlice = &(m_pSlice[s]);

      if(!pSlice->isValidSlice()) continue;

      pSlice->copySliceLuma((Pel*)pDec, (Pel*)pRest, LumaStride);

      if(m_uiVarGenMethod != ALF_RA)
      {
        calcVarforOneSlice(pSlice, m_imgY_var, pDec, FILTER_LENGTH/2, VAR_SIZE, LumaStride);
      }
      pSlice->extendSliceBorderLuma((Pel*)pDec, LumaStride, pcAlfParam->realfiltNo);

      xFilterOneSlice(pSlice, pDec, pRest, LumaStride, pcAlfParam);

    }
  }
}


Void TComAdaptiveLoopFilter::DecFilter_qc(imgpel* imgY_rec,ALFParam* pcAlfParam, int Stride)
{
  int i;
  int numBits = NUM_BITS; 
  int **pfilterCoeffSym;
  pfilterCoeffSym= m_filterCoeffSym;
  
  if(pcAlfParam->filtNo>=0)
  {
    //// Reconstruct filter coefficients
    reconstructFilterCoeffs( pcAlfParam, pfilterCoeffSym, numBits);
  }
  else
  {
    for(i = 0; i < NO_VAR_BINS; i++)
    {
      pcAlfParam->varIndTab[i]=0;
      memset(pfilterCoeffSym[i],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
    }
  }
  getCurrentFilter(pfilterCoeffSym,pcAlfParam);
}

Void TComAdaptiveLoopFilter::getCurrentFilter(int **filterCoeffSym,ALFParam* pcAlfParam)
{ 
  int i,  k, varInd;
  int *patternMap;
  {
    for(varInd=0; varInd<NO_VAR_BINS; ++varInd)
    {
      memset(m_filterCoeffPrevSelected[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
    }
    patternMap=patternMapTabShapes[pcAlfParam->realfiltNo];
    for(varInd=0; varInd<NO_VAR_BINS; ++varInd)
    {
      k=0;
      for(i = 0; i < MAX_SQR_FILT_LENGTH; i++)
      {
        if (patternMap[i]>0)
        {
          m_filterCoeffPrevSelected[varInd][i]=filterCoeffSym[pcAlfParam->varIndTab[varInd]][k];
          k++;
        }
        else
        {
          m_filterCoeffPrevSelected[varInd][i]=0;
        }
      }
    }
  }
}

Void TComAdaptiveLoopFilter::reconstructFilterCoeffs(ALFParam* pcAlfParam,int **pfilterCoeffSym, int bit_depth)
{
  int i, src, ind;
  
  // Copy non zero filters in filterCoeffTmp
  for(ind = 0; ind < pcAlfParam->filters_per_group_diff; ++ind)
  {
    for(i = 0; i < pcAlfParam->num_coeff; i++)
      m_filterCoeffTmp[ind][i] = pcAlfParam->coeffmulti[ind][i];
  }
  // Undo prediction
  for(ind = 0; ind < pcAlfParam->filters_per_group_diff; ++ind)
  {
    if((!pcAlfParam->predMethod) || (ind == 0)) 
    {
      memcpy(m_filterCoeffSymTmp[ind],m_filterCoeffTmp[ind],sizeof(int)*pcAlfParam->num_coeff);
    }
    else
    {
      // Prediction
      for(i = 0; i < pcAlfParam->num_coeff; ++i)
        m_filterCoeffSymTmp[ind][i] = (int)(m_filterCoeffTmp[ind][i] + m_filterCoeffSymTmp[ind - 1][i]);
    }
  }
  
  // Inverse quantization
  // Add filters forced to zero
  if(pcAlfParam->forceCoeff0)
  {
    assert(pcAlfParam->filters_per_group_diff < pcAlfParam->filters_per_group);
    src = 0;
    for(ind = 0; ind < pcAlfParam->filters_per_group; ++ind)
    {
      if(pcAlfParam->codedVarBins[ind])
      {
        memcpy(pfilterCoeffSym[ind],m_filterCoeffSymTmp[src],sizeof(int)*pcAlfParam->num_coeff);
        ++src;
      }
      else
      {
        memset(pfilterCoeffSym[ind],0,sizeof(int)*pcAlfParam->num_coeff);
      }
    }
    assert(src == pcAlfParam->filters_per_group_diff);
  }
  else
  {
    assert(pcAlfParam->filters_per_group_diff == pcAlfParam->filters_per_group);
    for(ind = 0; ind < pcAlfParam->filters_per_group; ++ind)
      memcpy(pfilterCoeffSym[ind],m_filterCoeffSymTmp[ind],sizeof(int)*pcAlfParam->num_coeff);
  }
}


static imgpel Clip_post(int high, int val)
{
  return (imgpel)(((val > high)? high: val));
}
Void TComAdaptiveLoopFilter::calcVar(int ypos, int xpos, imgpel **imgY_var, imgpel *imgY_pad, int pad_size, int fl, int img_height, int img_width, int img_stride)
{

  if(m_uiVarGenMethod == ALF_RA)
  {
    return;
  }

  static Int shift_h     = (Int)(log((double)VAR_SIZE_H)/log(2.0));
  static Int shift_w     = (Int)(log((double)VAR_SIZE_W)/log(2.0));

  Int start_height = ypos;
  Int start_width  = xpos;
  Int end_height   = ypos + img_height;
  Int end_width    = xpos + img_width;
  Int i, j;
  Int fl2plusOne= (VAR_SIZE<<1)+1; //3
  Int pad_offset = pad_size-fl-1;
  Int var_max= NO_VAR_BINS-1;
  Int avg_var;
  Int vertical, horizontal;
  Int direction;
  Int step1 = NO_VAR_BINS/3 - 1;
  Int th[NO_VAR_BINS] = {0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4}; 

  for(i = 1+start_height+1; i < end_height + fl2plusOne; i=i+2) // Compute at sub-sample by 2
  {
    Int yoffset = (pad_offset+i-pad_size) * img_stride + pad_offset-pad_size;
    imgpel *p_imgY_pad = &imgY_pad[yoffset];
    imgpel *p_imgY_pad_up   = &imgY_pad[yoffset + img_stride];
    imgpel *p_imgY_pad_down = &imgY_pad[yoffset - img_stride];
    for(j = 1+start_width+1; j < end_width +fl2plusOne; j=j+2)  // Compute at sub-sample by 2
    {
      vertical = abs((p_imgY_pad[j]<<1) - p_imgY_pad_down[j] - p_imgY_pad_up[j]);
      horizontal = abs((p_imgY_pad[j]<<1) - p_imgY_pad[j+1] - p_imgY_pad[j-1]);
      m_imgY_ver[i-1][j-1] = vertical;
      m_imgY_hor[i-1][j-1] = horizontal;
    }
  }

  for(i = 1+start_height; i < end_height + 1; i=i+4)
  {
    for(j = 1+start_width; j < end_width + 1; j=j+4)  
    {
      // in a 4x4 block, only need 4 pixels as below
      m_imgY_ver[i-1][j-1] = m_imgY_ver[i][j] + m_imgY_ver[i+2][j] + m_imgY_ver[i][j+2] + m_imgY_ver[i+2][j+2];  
      m_imgY_hor[i-1][j-1] = m_imgY_hor[i][j] + m_imgY_hor[i+2][j] + m_imgY_hor[i][j+2] + m_imgY_hor[i+2][j+2];
      
      direction = 0;
      if (m_imgY_ver[i-1][j-1] > 2*m_imgY_hor[i-1][j-1]) direction = 1; //vertical
      if (m_imgY_hor[i-1][j-1] > 2*m_imgY_ver[i-1][j-1]) direction = 2; //horizontal

      m_imgY_temp [i-1][j-1] = m_imgY_ver[i-1][j-1] + m_imgY_hor[i-1][j-1]; // Compute just once for each 4x4 block
      
      avg_var = m_imgY_temp [i-1][j-1]>>2; // average for 4 pixels
      avg_var = (imgpel) Clip_post(var_max, avg_var>>(g_uiBitIncrement+1));
      avg_var = th[avg_var];
      avg_var = Clip_post(step1, (Int) avg_var ) + (step1+1)*direction;
      imgY_var[(i - 1)>>shift_h][(j - 1)>>shift_w] = avg_var;
    }
  }
}


Void TComAdaptiveLoopFilter::createRegionIndexMap(imgpel **imgYVar, Int imgWidth, Int imgHeight)
{
  int varStepSizeWidth = VAR_SIZE_W;
  int varStepSizeHeight = VAR_SIZE_H;
  int shiftHeight = (int)(log((double)varStepSizeHeight)/log(2.0));
  int shiftWidth = (int)(log((double)varStepSizeWidth)/log(2.0));

  int i, j;
  int regionTable[NO_VAR_BINS] = {0, 1, 4, 5, 15, 2, 3, 6, 14, 11, 10, 7, 13, 12,  9,  8}; 
  int xInterval;
  int yInterval;
  int yIndex;
  int yIndexOffset;
  int yStartLine;
  int yEndLine;

  xInterval = ((( (imgWidth+63)/64) + 1) / 4 * 64)>>shiftWidth;  
  yInterval = ((((imgHeight+63)/64) + 1) / 4 * 64)>>shiftHeight;

  for (yIndex = 0; yIndex < 4 ; yIndex++)
  {
    yIndexOffset = yIndex * 4;
    yStartLine = yIndex * yInterval;
    yEndLine   = (yIndex == 3) ? imgHeight>>shiftHeight : (yStartLine+yInterval);

    for(i = yStartLine; i < yEndLine ; i++)
    {
      for(j = 0; j < xInterval ; j++)
      {
        imgYVar[i][j] = regionTable[yIndexOffset+0];     
      }

      for(j = xInterval; j < xInterval*2 ; j++)
      {
        imgYVar[i][j] = regionTable[yIndexOffset+1];     
      }

      for(j = xInterval*2; j < xInterval*3 ; j++)
      {
        imgYVar[i][j] = regionTable[yIndexOffset+2];     
      }

      for(j = xInterval*3; j < imgWidth>>shiftWidth ; j++)
      {
        imgYVar[i][j] = regionTable[yIndexOffset+3];     
      }
    }
  }

}

Void TComAdaptiveLoopFilter::filterFrame(imgpel *imgYRecPost, imgpel *imgYRec, int filtNo, int stride)
{
  Int varStepSizeWidth = VAR_SIZE_W;
  Int varStepSizeHeight = VAR_SIZE_H;
  Int shiftHeight = (Int)(log((double)varStepSizeHeight)/log(2.0));

  Int i, j, pixelInt;
  imgpel *pImgYVar,*pImgYPad;
  Int maxVal=g_uiIBDI_MAX;
  imgpel *pImgYPad1,*pImgYPad2,*pImgYPad3,*pImgYPad4;
  Int lastCoef= MAX_SQR_FILT_LENGTH-1;
  Int *coef = m_filterCoeffPrevSelected[0];
  Int numBitsMinus1= NUM_BITS-1;
  Int offset = (1<<(NUM_BITS-2));

  switch(filtNo)
  {
  case 0:
    for (i = 0; i < m_img_height; i++)
    {
      pImgYVar = m_imgY_var[i>>shiftHeight];
      pImgYPad = imgYRec + i*stride;
      {
        pImgYPad1 = imgYRec + (i+1)*stride;
        pImgYPad2 = imgYRec + (i-1)*stride;
        pImgYPad3 = imgYRec + (i+2)*stride;
        pImgYPad4 = imgYRec + (i-2)*stride;
      }

      for (j = 0; j < m_img_width; j++)
      {
        if (j%varStepSizeWidth==0) 
        {
          coef = m_filterCoeffPrevSelected[*(pImgYVar++)];
        }
        
        pixelInt=coef[lastCoef];

        pixelInt += coef[3]* (pImgYPad3[j+2]+pImgYPad4[j-2]);
        pixelInt += coef[5]* (pImgYPad3[j]+pImgYPad4[j]);
        pixelInt += coef[7]* (pImgYPad3[j-2]+pImgYPad4[j+2]);

        pixelInt += coef[15]* (pImgYPad1[j+1]+pImgYPad2[j-1]);
        pixelInt += coef[16]* (pImgYPad1[j]+pImgYPad2[j]);
        pixelInt += coef[17]* (pImgYPad1[j-1]+pImgYPad2[j+1]);

        pixelInt += coef[25]* (pImgYPad[j+2]+pImgYPad[j-2]);
        pixelInt += coef[26]* (pImgYPad[j+1]+pImgYPad[j-1]);
        pixelInt += coef[27]* (pImgYPad[j]);

        pixelInt=(Int)((pixelInt+offset) >> (numBitsMinus1));
        imgYRecPost[i*stride + j]=max(0, min(pixelInt,maxVal));
      }
    } 
    break;

  case 1:
    for (i = 0; i < m_img_height; i++)
    {
      pImgYVar = m_imgY_var[i>>shiftHeight];
      pImgYPad = imgYRec + i*stride;
      {
        pImgYPad1 = imgYRec + (i+1)*stride;
        pImgYPad2 = imgYRec + (i-1)*stride;
        pImgYPad3 = imgYRec + (i+2)*stride;
        pImgYPad4 = imgYRec + (i-2)*stride;
      }

      for (j = 0; j < m_img_width; j++)
      {
        if (j%varStepSizeWidth==0) 
        {
          coef = m_filterCoeffPrevSelected[*(pImgYVar++)];
        }
        
        pixelInt=coef[lastCoef];
       
        pixelInt += coef[5]* (pImgYPad3[j]+pImgYPad4[j]);
        pixelInt += coef[16]* (pImgYPad1[j]+pImgYPad2[j]);
       
        pixelInt += coef[22]* (pImgYPad[j+5]+pImgYPad[j-5]);
        pixelInt += coef[23]* (pImgYPad[j+4]+pImgYPad[j-4]);
        pixelInt += coef[24]* (pImgYPad[j+3]+pImgYPad[j-3]);
        pixelInt += coef[25]* (pImgYPad[j+2]+pImgYPad[j-2]);
        pixelInt += coef[26]* (pImgYPad[j+1]+pImgYPad[j-1]);
        pixelInt += coef[27]* (pImgYPad[j]);

        pixelInt=(Int)((pixelInt+offset) >> (numBitsMinus1));
        imgYRecPost[i*stride + j]=max(0, min(pixelInt,maxVal));
      }
    } 
    break;
  }
}

Void TComAdaptiveLoopFilter::subfilterFrame(imgpel *imgYRecPost, imgpel *imgYRec, int filtNo, int startHeight, int endHeight, int startWidth, int endWidth, int stride)
{
  Int varStepSizeWidth = VAR_SIZE_W;
  Int varStepSizeHeight = VAR_SIZE_H;
  Int shiftHeight = (Int)(log((double)varStepSizeHeight)/log(2.0));
  Int shiftWidth = (Int)(log((double)varStepSizeWidth)/log(2.0));
  Int i, j, pixelInt;
  imgpel *pImgYVar,*pImgYPad;
  imgpel *pImgYPad1,*pImgYPad2,*pImgYPad3,*pImgYPad4;
  Int maxVal=g_uiIBDI_MAX;
  Int lastCoef= MAX_SQR_FILT_LENGTH-1;
  Int *coef = m_filterCoeffPrevSelected[0];
  Int numBitsMinus1= NUM_BITS-1;
  Int offset = (1<<(NUM_BITS-2));

  switch(filtNo)
  {
  case 0:
    for (i =  startHeight; i < endHeight; i++)
    {
      pImgYVar = m_imgY_var[i>>shiftHeight] + (startWidth>>shiftWidth);
      pImgYPad = imgYRec + i*stride;
      {
        pImgYPad1 = imgYRec + (i+1)*stride;
        pImgYPad2 = imgYRec + (i-1)*stride;
        pImgYPad3 = imgYRec + (i+2)*stride;
        pImgYPad4 = imgYRec + (i-2)*stride;
      }

      for (j = startWidth; j < endWidth; j++)
      {
        if (j%varStepSizeWidth==0) 
        {
          coef = m_filterCoeffPrevSelected[*(pImgYVar++)];
        }
        
        pixelInt=coef[lastCoef];

        pixelInt += coef[3]* (pImgYPad3[j+2]+pImgYPad4[j-2]);
        pixelInt += coef[5]* (pImgYPad3[j]+pImgYPad4[j]);
        pixelInt += coef[7]* (pImgYPad3[j-2]+pImgYPad4[j+2]);

        pixelInt += coef[15]* (pImgYPad1[j+1]+pImgYPad2[j-1]);
        pixelInt += coef[16]* (pImgYPad1[j]+pImgYPad2[j]);
        pixelInt += coef[17]* (pImgYPad1[j-1]+pImgYPad2[j+1]);

        pixelInt += coef[25]* (pImgYPad[j+2]+pImgYPad[j-2]);
        pixelInt += coef[26]* (pImgYPad[j+1]+pImgYPad[j-1]);
        pixelInt += coef[27]* (pImgYPad[j]);

        pixelInt=(Int)((pixelInt+offset) >> (numBitsMinus1));
        imgYRecPost[i*stride + j]=max(0, min(pixelInt,maxVal));
      }
    }
    break;
  case 1:
    for (i =  startHeight; i < endHeight; i++)
    {
      pImgYVar = m_imgY_var[i>>shiftHeight] + (startWidth>>shiftWidth);
      pImgYPad = imgYRec + i*stride;
      {
        pImgYPad1 = imgYRec + (i+1)*stride;
        pImgYPad2 = imgYRec + (i-1)*stride;
        pImgYPad3 = imgYRec + (i+2)*stride;
        pImgYPad4 = imgYRec + (i-2)*stride;
      }

      for (j = startWidth; j < endWidth; j++)
      {
        if (j%varStepSizeWidth==0) 
        {
          coef = m_filterCoeffPrevSelected[*(pImgYVar++)];
        }
        
        pixelInt=coef[lastCoef];

        pixelInt += coef[5]* (pImgYPad3[j]+pImgYPad4[j]);
        pixelInt += coef[16]* (pImgYPad1[j]+pImgYPad2[j]);
       
        pixelInt += coef[22]* (pImgYPad[j+5]+pImgYPad[j-5]);
        pixelInt += coef[23]* (pImgYPad[j+4]+pImgYPad[j-4]);
        pixelInt += coef[24]* (pImgYPad[j+3]+pImgYPad[j-3]);
        pixelInt += coef[25]* (pImgYPad[j+2]+pImgYPad[j-2]);
        pixelInt += coef[26]* (pImgYPad[j+1]+pImgYPad[j-1]);
        pixelInt += coef[27]* (pImgYPad[j]);

        pixelInt=(Int)((pixelInt+offset) >> (numBitsMinus1));
        imgYRecPost[i*stride + j]=max(0, min(pixelInt,maxVal));
      }
    }
    break;
  }
}


Void TComAdaptiveLoopFilter::xCUAdaptive_qc(TComPic* pcPic, ALFParam* pcAlfParam, imgpel *imgY_rec_post, imgpel *imgY_rec, Int Stride)
{
  // for every CU, call CU-adaptive ALF process
  for( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    xSubCUAdaptive_qc(pcCU, pcAlfParam, imgY_rec_post, imgY_rec, 0, 0, Stride);
  }
}

Void TComAdaptiveLoopFilter::xSubCUAdaptive_qc(TComDataCU* pcCU, ALFParam* pcAlfParam, imgpel *imgY_rec_post, imgpel *imgY_rec, UInt uiAbsPartIdx, UInt uiDepth, Int Stride)
{
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  // check picture boundary
  if ( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }
  
  // go to sub-CU?
  if ( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSubCUAdaptive_qc(pcCU, pcAlfParam, imgY_rec_post, imgY_rec, uiAbsPartIdx, uiDepth+1, Stride);
    }
    return;
  }
  
  // check maskImagedec
  if ( pcCU->getAlfCtrlFlag(uiAbsPartIdx) )
  {
    subfilterFrame(imgY_rec_post, imgY_rec, pcAlfParam->realfiltNo, uiTPelY, min(uiBPelY+1,(unsigned int)(m_img_height)), uiLPelX, min(uiRPelX+1,(unsigned int)(m_img_width)), Stride);
  }
}

// --------------------------------------------------------------------------------------------------------------------
// ALF for chroma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFChroma(ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if((pcAlfParam->chroma_idc>>1)&0x01)
  {
    if(!m_bUseNonCrossALF)
      xFrameChroma(0, 0, (m_img_height>>1), (m_img_width>>1), pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->realfiltNo_chroma, 0);
    else
    {
      Int iStride   = pcPicRest->getCStride();
      Pel* pDec     = pcPicDec->getCbAddr();
      Pel* pRest    = pcPicRest->getCbAddr();

      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        CAlfSlice* pSlice = &(m_pSlice[s]);

        if(!pSlice->isValidSlice()) continue;

        pSlice->copySliceChroma(pDec, pRest, iStride);
        pSlice->extendSliceBorderChroma(pDec, iStride, pcAlfParam->realfiltNo_chroma);
        xFrameChromaforOneSlice(pSlice, ALF_Cb, pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->realfiltNo_chroma);
      }
    }
  }
  
  if(pcAlfParam->chroma_idc&0x01)
  {
    if(!m_bUseNonCrossALF)
      xFrameChroma(0, 0, (m_img_height>>1), (m_img_width>>1), pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->realfiltNo_chroma, 1);
    else
    {
      Int iStride   = pcPicRest->getCStride();
      Pel* pDec     = pcPicDec->getCrAddr();
      Pel* pRest    = pcPicRest->getCrAddr();
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        CAlfSlice* pSlice = &(m_pSlice[s]);

        if(!pSlice->isValidSlice()) continue;

        pSlice->copySliceChroma(pDec, pRest, iStride);
        pSlice->extendSliceBorderChroma(pDec, iStride, pcAlfParam->realfiltNo_chroma);
        xFrameChromaforOneSlice(pSlice, ALF_Cr, pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->realfiltNo_chroma);
      }
    }
  }
}

/** 
 \param pcPicDec    picture before ALF
 \param pcPicRest   picture after  ALF
 \param qh          filter coefficient
 \param iTap        filter tap
 \param iColor      0 for Cb and 1 for Cr
 */
Void TComAdaptiveLoopFilter::xFrameChroma(Int ypos, Int xpos, Int iHeight, Int iWidth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor )
{
  Int x, y, value, N;
  Pel *pImgPad1,*pImgPad2,*pImgPad3,*pImgPad4;
  
  N = m_sqrFiltLengthTab[iTap] - 1;
  Pel* pDec;
  Int iDecStride = pcPicDec->getCStride();
  
  Pel* pRest;
  Int iRestStride = pcPicRest->getCStride();
  
  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;
  
  if (iColor)
  {
    pDec = pcPicDec->getCrAddr();
    pRest = pcPicRest->getCrAddr();
  }
  else
  {
    pDec = pcPicDec->getCbAddr();
    pRest = pcPicRest->getCbAddr();
  }
  if(m_bUseNonCrossALF)
  {
    pDec  += ( ypos*iDecStride  + xpos);
    pRest += ( ypos*iRestStride + xpos);
  }
  else
  {
    assert(ypos ==0 && xpos ==0);
    assert(iHeight == pcPicRest->getHeight() >> 1);
    assert(iWidth  == pcPicRest->getWidth()  >> 1);
  }

  switch(iTap)
  {
  case 0:
    for (y = 0; y < iHeight; y++)
    {
      pImgPad1 = pDec + iDecStride;
      pImgPad2 = pDec - iDecStride;
      pImgPad3 = pDec + 2*iDecStride;
      pImgPad4 = pDec - 2*iDecStride;

      for (x = 0; x < iWidth; x++)
      {
        value  = 0;
        value += qh[0] * (pImgPad3[x+2] + pImgPad4[x-2]);
        value += qh[1] * (pImgPad3[x]   + pImgPad4[x]);
        value += qh[2] * (pImgPad3[x-2] + pImgPad4[x+2]);

        value += qh[3] * (pImgPad1[x+1] + pImgPad2[x-1]);
        value += qh[4] * (pImgPad1[x]   + pImgPad2[x]);
        value += qh[5] * (pImgPad1[x-1] + pImgPad2[x+1]);

        value += qh[6] * (pDec[x+2]     + pDec[x-2]);
        value += qh[7] * (pDec[x+1]     + pDec[x-1]);
        value += qh[8] * (pDec[x]);

        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;
        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;   
      pDec += iDecStride;
    }
    break;

  case 1:
    for (y = 0; y < iHeight; y++)
    {
      pImgPad1 = pDec + iDecStride;
      pImgPad2 = pDec - iDecStride;
      pImgPad3 = pDec + 2*iDecStride;
      pImgPad4 = pDec - 2*iDecStride;

      for (x = 0; x < iWidth; x++)
      {
        value  = 0;
        value += qh[0] * (pImgPad3[x] + pImgPad4[x]);
        value += qh[1] * (pImgPad1[x] + pImgPad2[x]);

        value += qh[2] * (pDec[x+5]   + pDec[x-5]);
        value += qh[3] * (pDec[x+4]   + pDec[x-4]);
        value += qh[4] * (pDec[x+3]   + pDec[x-3]);
        value += qh[5] * (pDec[x+2]   + pDec[x-2]);
        value += qh[6] * (pDec[x+1]   + pDec[x-1]);
        value += qh[7] * (pDec[x]);

        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;
        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    break;
    default:
      assert(0);
      break;
  }
}


#if F747_APS
Void TComAdaptiveLoopFilter::setAlfCtrlFlags(AlfCUCtrlInfo* pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx)
#else
Void TComAdaptiveLoopFilter::setAlfCtrlFlags(ALFParam *pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx)
#endif
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts      = uiCurNumParts>>2;
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth ) ) || bBoundary )
  {
    UInt uiIdx = uiAbsPartIdx;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
      {
        setAlfCtrlFlags(pAlfParam, pcCU, uiIdx, uiDepth+1, idx);
      }
      uiIdx += uiQNumParts;
    }
    
    return;
  }
  
  if( uiDepth <= pAlfParam->alf_max_depth || pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, pAlfParam->alf_max_depth))
  {
    if (uiDepth > pAlfParam->alf_max_depth)
    {
      pcCU->setAlfCtrlFlagSubParts(pAlfParam->alf_cu_flag[idx], uiAbsPartIdx, pAlfParam->alf_max_depth);
    }
    else
    {
      pcCU->setAlfCtrlFlagSubParts(pAlfParam->alf_cu_flag[idx], uiAbsPartIdx, uiDepth );
    }
    idx++;
  }
}

/** create ALF slice units
 * \param pcPic picture related parameters
 */
Void TComAdaptiveLoopFilter::createSlice(TComPic* pcPic)
{
  UInt uiMaxNumSUInLCU = pcPic->getNumPartInCU();
  UInt uiNumLCUInPic   = pcPic->getNumCUsInFrame();

  m_piSliceSUMap = new Int[uiMaxNumSUInLCU * uiNumLCUInPic];
  for(UInt i=0; i< (uiMaxNumSUInLCU * uiNumLCUInPic); i++ )
  {
    m_piSliceSUMap[i] = -1;
  }

  for( UInt uiCUAddr = 0; uiCUAddr < uiNumLCUInPic ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    pcCU->setSliceSUMap(m_piSliceSUMap + (uiCUAddr* uiMaxNumSUInLCU)); 
  }

  m_pSlice = new CAlfSlice[m_uiNumSlicesInPic];
  for(Int i=0; i< m_uiNumSlicesInPic; i++)
  {
    m_pSlice[i].init(pcPic, m_iSGDepth, m_piSliceSUMap);
  }
}

/** Destroy ALF slice units
 */
Void TComAdaptiveLoopFilter::destroySlice()
{
  if(m_pSlice != NULL)
  {
    delete[] m_pSlice;
    m_pSlice = NULL;
  }

  if(m_piSliceSUMap != NULL)
  {
    delete[] m_piSliceSUMap;
    m_piSliceSUMap = NULL;
  }
}

/** Calculate ALF grouping indices for one slice
 * \param pSlice slice variables
 * \param imgY_var grouping indices buffer
 * \param imgY_pad padded picture buffer
 * \param pad_size (max. filter tap)/2
 * \param fl  VAR_SIZE
 * \param img_stride picture buffer stride
 */
Void TComAdaptiveLoopFilter::calcVarforOneSlice(CAlfSlice* pSlice, imgpel **imgY_var, imgpel *imgY_pad, Int pad_size, Int fl, Int img_stride)
{  
  Int iHeight, iWidth;
  Int ypos, xpos;

  for(Int i=0; i< pSlice->getNumLCUs(); i++)
  { 
    CAlfLCU& cAlfLCU = (*pSlice)[i];
    for(Int j=0; j< cAlfLCU.getNumSGU(); j++)
    {
      ypos    = (Int)cAlfLCU[j].posY;
      xpos    = (Int)cAlfLCU[j].posX;
      iHeight = (Int)cAlfLCU[j].height;
      iWidth  = (Int)cAlfLCU[j].width;

      if(m_bUseNonCrossALF)
      {
        imgpel* pPel = imgY_pad + (ypos * img_stride) + xpos;
        cAlfLCU.extendBorderCoreFunction((Pel*)pPel, img_stride, cAlfLCU[j].isBorderAvailable, iWidth, iHeight, fl, fl, true);
      }

      calcVar(ypos, xpos, imgY_var, imgY_pad, pad_size, fl, iHeight, iWidth, img_stride);
    }
  }
}

/** Perform ALF for one chroma slice
 * \param pSlice slice variables
 * \param ComponentID Cb or Cr
 * \param pcPicDec picture buffer before filtering
 * \param pcPicRest picture buffer after filtering
 * \param qh  filter coefficients
 * \param iTap filter tap size
 */
Void TComAdaptiveLoopFilter::xFrameChromaforOneSlice(CAlfSlice* pSlice, Int ComponentID, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap)
{
  Int iColor = (ComponentID == ALF_Cb)?(0):(1);

  Int iHeight, iWidth;
  Int ypos, xpos;

  for(Int i=0; i< pSlice->getNumLCUs(); i++)
  { 
    CAlfLCU& cAlfLCU = (*pSlice)[i];
    for(Int j=0; j< cAlfLCU.getNumSGU(); j++)
    {
      ypos    = (Int)(cAlfLCU[j].posY >> 1);
      xpos    = (Int)(cAlfLCU[j].posX >> 1);
      iHeight = (Int)(cAlfLCU[j].height >> 1);
      iWidth  = (Int)(cAlfLCU[j].width >> 1);
      xFrameChroma(ypos, xpos, iHeight, iWidth, pcPicDec, pcPicRest, qh, iTap, iColor );
    }
  }
}


/** Perform ALF for one luma slice
 * \param pSlice slice variables
 * \param pDec picture buffer before filtering
 * \param pRest picture buffer after filtering
 * \param iStride stride size of picture buffer
 * \param pcAlfParam ALF parameters
 */
Void TComAdaptiveLoopFilter::xFilterOneSlice(CAlfSlice* pSlice, imgpel* pDec, imgpel* pRest, Int iStride, ALFParam* pcAlfParam)
{
#if F747_APS
  if(pSlice->getCUCtrlEnabled())
#else
  if(pcAlfParam->cu_control_flag)
#endif
  {
    TComPic* pcPic       = pSlice->getPic();
    UInt     uiNumLCU    = pSlice->getNumLCUs();
    UInt     uiSUWidth   = pcPic->getMinCUWidth();
    UInt     uiSUHeight  = pcPic->getMinCUHeight();
    Int      iFiltNo     = pcAlfParam->realfiltNo;

    UInt idx, uiStartSU, uiEndSU, uiCurrSU, uiLCUX, uiLCUY, uiLPelX, uiTPelY;
    TComDataCU* pcCU;

    for(idx=0; idx< uiNumLCU; idx++)
    {
      CAlfLCU&    cAlfLCU    = (*pSlice)[idx];

      pcCU                   = cAlfLCU.getCU();
      uiStartSU              = cAlfLCU.getStartSU();
      uiEndSU                = cAlfLCU.getEndSU();
      uiLCUX                 = pcCU->getCUPelX();
      uiLCUY                 = pcCU->getCUPelY();

      for(uiCurrSU= uiStartSU; uiCurrSU<= uiEndSU; uiCurrSU++)
      {
        uiLPelX   = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
        uiTPelY   = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];
        if( !( uiLPelX < m_img_width )  || !( uiTPelY < m_img_height )  )
        {
          continue;
        }
        if(pcCU->getAlfCtrlFlag(uiCurrSU))
        {
          subfilterFrame(pRest, pDec, iFiltNo, uiTPelY, uiTPelY+ uiSUHeight, uiLPelX, uiLPelX+ uiSUWidth, iStride);
        }
      }
    }
  }  
  else
  {
    UInt uiNumLCU = pSlice->getNumLCUs();
    Int  iTPelY,  iLPelX;
    Int  iWidth, iHeight;

    for(UInt idx=0; idx< uiNumLCU; idx++)
    {
      CAlfLCU& cAlfLCU = (*pSlice)[idx];
      for(UInt i=0; i< cAlfLCU.getNumSGU(); i++)
      {
        iTPelY = (Int)cAlfLCU[i].posY;
        iLPelX = (Int)cAlfLCU[i].posX;
        iHeight= (Int)cAlfLCU[i].height;
        iWidth = (Int)cAlfLCU[i].width;
        subfilterFrame(pRest, pDec, pcAlfParam->realfiltNo, iTPelY, iTPelY+ iHeight, iLPelX, iLPelX+ iWidth, iStride);
      }
    }
  }
}

/** Copy ALF CU control flags from ALF parameters for slices
 * \param [in] vAlfParamSlices ALF CU control parameters
 */
#if F747_APS
Void TComAdaptiveLoopFilter::transferCtrlFlagsFromAlfParam(std::vector<AlfCUCtrlInfo>& vAlfParamSlices)
{
  assert(m_uiNumSlicesInPic == vAlfParamSlices.size());

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    AlfCUCtrlInfo& rAlfParam = vAlfParamSlices[s];

    transferCtrlFlagsFromAlfParamOneSlice(s, 
      (rAlfParam.cu_control_flag ==1)?true:false, 
      rAlfParam.alf_max_depth, 
      rAlfParam.alf_cu_flag
      );
  }
}
#else
Void TComAdaptiveLoopFilter::transferCtrlFlagsFromAlfParam(ALFParam* pcAlfParam)
{
  Bool  bCUCtrlEnabled = (pcAlfParam->cu_control_flag ==1);
  Int   iAlfDepth      = (Int)pcAlfParam->alf_max_depth;
  UInt* puiFlags       =  pcAlfParam->alf_cu_flag;

  UInt uiNumFlags = 0;
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {

    if(!m_pSlice[s].isValidSlice()) continue;

    transferCtrlFlagsFromAlfParamOneSlice(s,bCUCtrlEnabled, iAlfDepth, &(puiFlags[uiNumFlags]));
    uiNumFlags += m_pSlice[s].getNumCtrlFlags();
  }

  assert(uiNumFlags == pcAlfParam->num_alf_cu_flag);
}
#endif
/** Copy ALF CU control flags from ALF CU control parameters for one slice
 * \param [in] s slice ID 
 * \param [in] bCUCtrlEnabled true for ALF CU control enabled
 * \param [in] iAlfDepth ALF CU control depth
 * \param [in] vCtrlFlags ALF CU control flags
 */
#if F747_APS
Void TComAdaptiveLoopFilter::transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, std::vector<UInt>& vCtrlFlags)
#else
Void TComAdaptiveLoopFilter::transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, UInt* puiFlagsAlfParam)
#endif
{
  CAlfSlice& cSlice   = m_pSlice[s];
#if !F747_APS
  UInt*      puiFlags = puiFlagsAlfParam;
#endif

  cSlice.setCUCtrlEnabled(bCUCtrlEnabled);
  if(!bCUCtrlEnabled)
  {
    cSlice.setCUCtrlDepth(-1);
    return;
  }

  cSlice.setCUCtrlDepth(iAlfDepth);

  UInt uiNumCtrlFlags = 0;
  for(UInt idx=0; idx< cSlice.getNumLCUs(); idx++)
  {
    CAlfLCU& cAlfLCU = cSlice[idx];

#if F747_APS
    cAlfLCU.getCtrlFlagsFromAlfParam(iAlfDepth, &(vCtrlFlags[uiNumCtrlFlags]) );
    uiNumCtrlFlags += cAlfLCU.getNumCtrlFlags();
#else
    cAlfLCU.getCtrlFlagsFromAlfParam(iAlfDepth, puiFlags);
    UInt uiNumFlags = cAlfLCU.getNumCtrlFlags();

    puiFlags += uiNumFlags;
    uiNumCtrlFlags += uiNumFlags;
#endif

  }
  cSlice.setNumCtrlFlags(uiNumCtrlFlags);
}

//-------------- CAlfLCU -----------------//

/** Create ALF LCU unit perform slice processing
 * \param iSliceID slice ID
 * \param pcPic picture parameters
 * \param uiCUAddr LCU raster scan address
 * \param uiStartSU starting SU z-scan address of current LCU unit
 * \param uiEndSU ending SU z-scan address of current LCU unit
 * \param iSGDepth Slice granularity
 */
Void CAlfLCU::create(Int iSliceID, TComPic* pcPic, UInt uiCUAddr, UInt uiStartSU, UInt uiEndSU, Int iSGDepth)
{
  m_iSliceID       = iSliceID;
  m_pcPic          = pcPic;
  m_uiCUAddr       = uiCUAddr;
  m_pcCU           = pcPic->getCU(m_uiCUAddr);
  m_uiStartSU      = uiStartSU;

  UInt uiLCUX      = m_pcCU->getCUPelX();
  UInt uiLCUY      = m_pcCU->getCUPelY();
  UInt uiPicWidth  = m_pcCU->getSlice()->getSPS()->getWidth();
  UInt uiPicHeight = m_pcCU->getSlice()->getSPS()->getHeight();
  UInt uiMaxNumSUInLCUWidth = m_pcPic->getNumPartInWidth();
  UInt uiMAxNumSUInLCUHeight= m_pcPic->getNumPartInHeight();
  UInt uiMaxNumSUInLCU      = uiMaxNumSUInLCUWidth*uiMAxNumSUInLCUHeight;
  UInt uiMaxNumSUInSGU      = uiMaxNumSUInLCU >> (iSGDepth << 1);
  UInt uiCurrSU, uiLPelX, uiTPelY;

  //create CU ctrl flag buffer
  m_puiCUCtrlFlag = new UInt[uiMaxNumSUInLCU];
  ::memset(m_puiCUCtrlFlag, 0, sizeof(UInt)*uiMaxNumSUInLCU);
  m_iNumCUCtrlFlags = -1;

  //find number of SGU
  uiCurrSU   = m_uiStartSU;
  m_uiNumSGU = 0;
  while(uiCurrSU <= uiEndSU)
  {
    uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
    uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

    if(( uiLPelX < uiPicWidth ) && ( uiTPelY < uiPicHeight ))
    {
      m_uiNumSGU ++;
    }

    uiCurrSU += uiMaxNumSUInSGU;
  }

  m_pSGU= new AlfSGUInfo[m_uiNumSGU];       

  //initialize SGU parameters
  uiCurrSU   = m_uiStartSU;
  UInt uiSGUID = 0;
  Int* piCUSliceMap = m_pcCU->getSliceSUMap();
  while(uiCurrSU <= uiEndSU)
  {
    uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
    uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

    while(!( uiLPelX < uiPicWidth ) || !( uiTPelY < uiPicHeight ))
    {
      uiCurrSU += uiMaxNumSUInSGU;
      if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > uiEndSU)
      {
        break;
      }
      uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
      uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];
    }

    if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > uiEndSU)
    {
      break;
    }

    AlfSGUInfo& rSGU = m_pSGU[uiSGUID];
    rSGU.sliceID = m_iSliceID;
    rSGU.posY    = uiTPelY;
    rSGU.posX    = uiLPelX;
    rSGU.startSU = uiCurrSU;
    UInt uiLastValidSU  = uiCurrSU;
    UInt uiIdx, uiLPelX_su, uiTPelY_su;
    for(uiIdx = uiCurrSU; uiIdx < uiCurrSU + uiMaxNumSUInSGU; uiIdx++)
    {
      if(uiIdx > uiEndSU)
      {
        break;        
      }
      uiLPelX_su   = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY_su   = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      if( !(uiLPelX_su < uiPicWidth ) || !( uiTPelY_su < uiPicHeight ))
      {
        continue;
      }
      piCUSliceMap[uiIdx] = m_iSliceID;
      uiLastValidSU = uiIdx;
    }
    rSGU.endSU = uiLastValidSU;

    UInt rTLSU = g_auiZscanToRaster[ rSGU.startSU ];
    UInt rBRSU = g_auiZscanToRaster[ rSGU.endSU   ];
    rSGU.widthSU  = (rBRSU % uiMaxNumSUInLCUWidth) - (rTLSU % uiMaxNumSUInLCUWidth)+ 1;
    rSGU.heightSU = (UInt)(rBRSU / uiMaxNumSUInLCUWidth) - (UInt)(rTLSU / uiMaxNumSUInLCUWidth)+ 1;
    rSGU.width    = rSGU.widthSU  * m_pcPic->getMinCUWidth();
    rSGU.height   = rSGU.heightSU * m_pcPic->getMinCUHeight();

    uiCurrSU += uiMaxNumSUInSGU;
    uiSGUID ++;
  }
  assert(uiSGUID == m_uiNumSGU);

  m_uiEndSU = m_pSGU[m_uiNumSGU-1].endSU;
}


/** Destroy ALF LCU unit
 */
Void CAlfLCU::destroy()
{
  if(m_pSGU != NULL)
  {
    delete[] m_pSGU; 
    m_pSGU = NULL;
  }
  if(m_puiCUCtrlFlag != NULL)
  {
    delete[] m_puiCUCtrlFlag;
    m_puiCUCtrlFlag = NULL;
  }
}


/** Set the neighboring availabilities for one slice granularity unit
 * \param uiNumLCUInPicWidth number of LCUs in picture width
 * \param uiNumLCUInPicHeight number of LCUs in picture height
 * \param uiNumSUInLCUWidth max. number of SUs in one LCU
 * \param uiNumSUInLCUHeight max. number of SUs in one LCU
 * \param piSliceIDMap slice ID map (picture)
 */
Void CAlfLCU::setSGUBorderAvailability(UInt uiNumLCUInPicWidth, UInt uiNumLCUInPicHeight, UInt uiNumSUInLCUWidth, UInt uiNumSUInLCUHeight,Int* piSliceIDMap)
{
  UInt uiPicWidth  = m_pcCU->getSlice()->getSPS()->getWidth();
  UInt uiPicHeight = m_pcCU->getSlice()->getSPS()->getHeight();
  UInt uiNumSUInLCU = uiNumSUInLCUWidth*uiNumSUInLCUHeight;
  UInt uiLCUOffset  = m_uiCUAddr*uiNumSUInLCU;
  Int* piSliceIDMapLCU = piSliceIDMap + uiLCUOffset;

  UInt uiLPelX, uiTPelY;
  UInt uiWidth, uiHeight;
  Bool bPicRBoundary, bPicBBoundary, bPicTBoundary, bPicLBoundary;
  Bool bLCURBoundary= false, bLCUBBoundary= false, bLCUTBoundary= false, bLCULBoundary= false;

  Bool* pbAvailBorder;
  Bool* pbAvail;
  UInt rTLSU, rBRSU, uiWidthSU, uiHeightSU;
  UInt zRefSU;
  Int* piRefID;
  Int* piRefMapLCU;

  UInt rTRefSU= 0, rBRefSU= 0, rLRefSU= 0, rRRefSU= 0;
  Int* piRRefMapLCU= NULL;
  Int* piLRefMapLCU= NULL;
  Int* piTRefMapLCU= NULL;
  Int* piBRefMapLCU= NULL;

  for(Int i=0; i< m_uiNumSGU; i++)
  {
    AlfSGUInfo& rSGU = m_pSGU[i];
    uiLPelX = rSGU.posX;
    uiTPelY = rSGU.posY;
    uiWidth = rSGU.width;
    uiHeight= rSGU.height;

    rTLSU     = g_auiZscanToRaster[ rSGU.startSU ];
    rBRSU     = g_auiZscanToRaster[ rSGU.endSU   ];
    uiWidthSU = rSGU.widthSU;
    uiHeightSU= rSGU.heightSU;

    pbAvailBorder = rSGU.isBorderAvailable;

    bPicTBoundary= (uiTPelY == 0                       )?(true):(false);
    bPicLBoundary= (uiLPelX == 0                       )?(true):(false);
    bPicRBoundary= (!(uiLPelX+ uiWidth < uiPicWidth )  )?(true):(false);
    bPicBBoundary= (!(uiTPelY + uiHeight < uiPicHeight))?(true):(false);

    //       SGU_L
    pbAvail = &(pbAvailBorder[SGU_L]);
    if(bPicLBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      bLCULBoundary = (rTLSU % uiNumSUInLCUWidth == 0)?(true):(false);
      if(bLCULBoundary)
      {
        rLRefSU     = rTLSU + uiNumSUInLCUWidth -1;
        zRefSU      = g_auiRasterToZscan[rLRefSU];
        piRefMapLCU = piLRefMapLCU= (piSliceIDMapLCU - uiNumSUInLCU);
      }
      else
      {
        zRefSU   = g_auiRasterToZscan[rTLSU - 1];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_R
    pbAvail = &(pbAvailBorder[SGU_R]);
    if(bPicRBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      bLCURBoundary = ( (rTLSU+ uiWidthSU) % uiNumSUInLCUWidth == 0)?(true):(false);
      if(bLCURBoundary)
      {
        rRRefSU      = rTLSU + uiWidthSU - uiNumSUInLCUWidth;
        zRefSU       = g_auiRasterToZscan[rRRefSU];
        piRefMapLCU  = piRRefMapLCU= (piSliceIDMapLCU + uiNumSUInLCU);
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU + uiWidthSU];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_T
    pbAvail = &(pbAvailBorder[SGU_T]);
    if(bPicTBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      bLCUTBoundary = ( (UInt)(rTLSU / uiNumSUInLCUWidth)== 0)?(true):(false);
      if(bLCUTBoundary)
      {
        rTRefSU      = uiNumSUInLCU - (uiNumSUInLCUWidth - rTLSU);
        zRefSU       = g_auiRasterToZscan[rTRefSU];
        piRefMapLCU  = piTRefMapLCU= (piSliceIDMapLCU - (uiNumLCUInPicWidth*uiNumSUInLCU));
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU - uiNumSUInLCUWidth];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_B
    pbAvail = &(pbAvailBorder[SGU_B]);
    if(bPicBBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      bLCUBBoundary = ( (UInt)(rBRSU / uiNumSUInLCUWidth) == (uiNumSUInLCUHeight-1) )?(true):(false);
      if(bLCUBBoundary)
      {
        rBRefSU      = rTLSU % uiNumSUInLCUWidth;
        zRefSU       = g_auiRasterToZscan[rBRefSU];
        piRefMapLCU  = piBRefMapLCU= (piSliceIDMapLCU + (uiNumLCUInPicWidth*uiNumSUInLCU));
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU + (uiHeightSU*uiNumSUInLCUWidth)];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_TL
    pbAvail = &(pbAvailBorder[SGU_TL]);
    if(bPicTBoundary || bPicLBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      if(bLCUTBoundary && bLCULBoundary)
      {
        zRefSU       = uiNumSUInLCU -1;
        piRefMapLCU  = piSliceIDMapLCU - ( (uiNumLCUInPicWidth+1)*uiNumSUInLCU);
      }
      else if(bLCUTBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rTRefSU- 1];
        piRefMapLCU  = piTRefMapLCU;
      }
      else if(bLCULBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rLRefSU- uiNumSUInLCUWidth ];
        piRefMapLCU  = piLRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU - uiNumSUInLCUWidth -1];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_TR
    pbAvail = &(pbAvailBorder[SGU_TR]);
    if(bPicTBoundary || bPicRBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      if(bLCUTBoundary && bLCURBoundary)
      {
        zRefSU      = g_auiRasterToZscan[uiNumSUInLCU - uiNumSUInLCUWidth];
        piRefMapLCU  = piSliceIDMapLCU - ( (uiNumLCUInPicWidth-1)*uiNumSUInLCU);        
      }
      else if(bLCUTBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rTRefSU+ uiWidthSU];
        piRefMapLCU  = piTRefMapLCU;
      }
      else if(bLCURBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rRRefSU- uiNumSUInLCUWidth ];
        piRefMapLCU  = piRRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU - uiNumSUInLCUWidth +uiWidthSU];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_BL
    pbAvail = &(pbAvailBorder[SGU_BL]);
    if(bPicBBoundary || bPicLBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      if(bLCUBBoundary && bLCULBoundary)
      {
        zRefSU      = g_auiRasterToZscan[uiNumSUInLCUWidth - 1];
        piRefMapLCU  = piSliceIDMapLCU + ( (uiNumLCUInPicWidth-1)*uiNumSUInLCU);        
      }
      else if(bLCUBBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rBRefSU - 1];
        piRefMapLCU  = piBRefMapLCU;
      }
      else if(bLCULBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rLRefSU+ uiHeightSU*uiNumSUInLCUWidth ];
        piRefMapLCU  = piLRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU + uiHeightSU*uiNumSUInLCUWidth -1];
        piRefMapLCU  = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }

    //       SGU_BR
    pbAvail = &(pbAvailBorder[SGU_BR]);
    if(bPicBBoundary || bPicRBoundary)
    {
      *pbAvail = false;
    }
    else
    {
      if(bLCUBBoundary && bLCURBoundary)
      {
        zRefSU = 0;
        piRefMapLCU = piSliceIDMapLCU+ ( (uiNumLCUInPicWidth+1)*uiNumSUInLCU);
      }
      else if(bLCUBBoundary)
      {
        zRefSU      = g_auiRasterToZscan[ rBRefSU + uiWidthSU];
        piRefMapLCU = piBRefMapLCU;
      }
      else if(bLCURBoundary)
      {
        zRefSU      = g_auiRasterToZscan[ rRRefSU + (uiHeightSU*uiNumSUInLCUWidth)];
        piRefMapLCU = piRRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU      = g_auiRasterToZscan[ rTLSU + (uiHeightSU*uiNumSUInLCUWidth)+ uiWidthSU];
        piRefMapLCU = piSliceIDMapLCU;
      }
      piRefID = piRefMapLCU + zRefSU;
      *pbAvail = (*piRefID == m_iSliceID)?(true):(false);
    }
  }

}


/** Extend slice boundary border
 * \param [in,out] pPel starting pixel position in picture buffer
 * \param [in] iStride stride size of picture buffer
 * \param [in] pbAvail neighboring availabilities for current processing block
 * \param [in] uiWidth pixel width of current processing block
 * \param [in] uiHeight pixel height of current processing block
 * \param [in] uiExtSizeX extension size in horizontal direction
 * \param [in] uiExtSizeY extension size in vertical direction
 * \param [in] bPaddingForCalculatingBAIndex default value is false. "false" means function triggered by filtering process. "true" means function triggered by BA index calculation process.
 */
Void CAlfLCU::extendBorderCoreFunction(Pel* pPel, Int iStride, Bool* pbAvail, UInt uiWidth, UInt uiHeight, UInt uiExtSizeX, UInt uiExtSizeY, Bool bPaddingForCalculatingBAIndex)
{
  Pel* pPelDst;
  Pel* pPelSrc;
  Int i, j;

  for(Int pos =0; pos < NUM_SGU_BORDER; pos++)
  {
    if(pbAvail[pos])
    {
      continue;
    }
    if(bPaddingForCalculatingBAIndex)
    {
      if( !(pos == SGU_T  || pos == SGU_L || pos == SGU_TL ) )
        continue;
    }

    switch(pos)
    {
    case SGU_L:
      {
        pPelDst = pPel - uiExtSizeX;
        pPelSrc = pPel;
        for(j=0; j< uiHeight; j++)
        {
          for(i=0; i< uiExtSizeX; i++)
          {
            pPelDst[i] = *pPelSrc;
          }
          pPelDst += iStride;
          pPelSrc += iStride;
        }
      }
      break;
    case SGU_R:
      {
        pPelDst = pPel + uiWidth;
        pPelSrc = pPelDst -1;
        for(j=0; j< uiHeight; j++)
        {
          for(i=0; i< uiExtSizeX; i++)
          {
            pPelDst[i] = *pPelSrc;
          }
          pPelDst += iStride;
          pPelSrc += iStride;
        }

      }
      break;
    case SGU_T:
      {
        UInt padWidth = (pbAvail[SGU_TR])?(uiWidth- uiExtSizeX):(uiWidth);
        pPelSrc = pPel;
        pPelDst = pPel - iStride;
        for(j=0; j< uiExtSizeY; j++)
        {
          ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*padWidth);
          pPelDst -= iStride;
        }
      }
      break;
    case SGU_B:
      {
        UInt padWidth  = (pbAvail[SGU_BL])?(uiWidth- uiExtSizeX):(uiWidth);
        UInt padOffset = (pbAvail[SGU_BL])?(uiExtSizeX):(0);

        pPelDst = pPel + uiHeight*iStride + padOffset;
        pPelSrc = pPelDst - iStride;
        for(j=0; j< uiExtSizeY; j++)
        {
          ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*padWidth);
          pPelDst += iStride;
        }

      }
      break;
    case SGU_TL:
      {
        if( (!pbAvail[SGU_T]) && (!pbAvail[SGU_L]))
        {
          pPelSrc = pPel  - uiExtSizeX;
          pPelDst = pPelSrc - iStride;
          for(j=0; j< uiExtSizeY; j++)
          {
            ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiExtSizeX);
            pPelDst -= iStride;
          }         
        }
      }
      break;
    case SGU_TR:
      {
        if( (!pbAvail[SGU_T]) && (!pbAvail[SGU_R]))
        {
          pPelSrc = pPel + uiWidth;
          pPelDst = pPelSrc - iStride;
          for(j=0; j< uiExtSizeY; j++)
          {
            ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiExtSizeX);
            pPelDst -= iStride;
          }

        }

      }
      break;
    case SGU_BL:
      {
        if( (!pbAvail[SGU_B]) && (!pbAvail[SGU_L]))
        {
          pPelDst = pPel + uiHeight*iStride; pPelDst-= uiExtSizeX;
          pPelSrc = pPelDst - iStride;
          for(j=0; j< uiExtSizeY; j++)
          {
            ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiExtSizeX);
            pPelDst += iStride;
          }

        }
      }
      break;
    case SGU_BR:
      {
        if( (!pbAvail[SGU_B]) && (!pbAvail[SGU_R]))
        {
          pPelDst = pPel + uiHeight*iStride; pPelDst += uiWidth;
          pPelSrc = pPelDst - iStride;
          for(j=0; j< uiExtSizeY; j++)
          {
            ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiExtSizeX);
            pPelDst += iStride;
          }
        }
      }
      break;
    default:
      {
        printf("Not a legal neighboring availability\n");
        assert(0);
        exit(-1);
      }

    }

  }

}

/** Extend slice boundary border for one luma LCU
 * \param [in, out] pImg picture buffer
 * \param [in] iStride stride size of picture buffer
 * \param [in] filtNo filter shape
 */
Void CAlfLCU::extendLumaBorder(Pel* pImg, Int iStride, Int filtNo)
{
  UInt uiExtSizeX = (filtNo == 0)?(2):(5);
  UInt uiExtSizeY = 2;
  UInt uiWidth, uiHeight;
  UInt posX, posY;
  Pel* pPel;
  Bool* pbAvail;

  for(Int n =0; n < m_uiNumSGU; n++)
  {
    AlfSGUInfo& rSGU = m_pSGU[n];
    posX     = rSGU.posX;
    posY     = rSGU.posY;
    uiWidth  = rSGU.width;
    uiHeight = rSGU.height;
    pbAvail  = rSGU.isBorderAvailable;    
    pPel     = pImg + (posY * iStride)+ posX;    
    extendBorderCoreFunction(pPel, iStride, pbAvail, uiWidth, uiHeight, uiExtSizeX, uiExtSizeY);
  }
}


/** Extend slice boundary border for one chroma LCU
* \param [in, out] pImg picture buffer
* \param [in] iStride stride size of picture buffer
* \param [in] filtNo filter shape
 */
Void CAlfLCU::extendChromaBorder(Pel* pImg, Int iStride, UInt filtNo)
{
  UInt uiExtSizeX = (filtNo == 0)?(2):(5);
  UInt uiExtSizeY = 2;

  UInt uiWidth, uiHeight;
  UInt posX, posY;
  Pel* pPel;
  Bool* pbAvail;

  for(Int n =0; n < m_uiNumSGU; n++)
  {
    AlfSGUInfo& rSGU = m_pSGU[n];
    posX     = rSGU.posX >> 1;
    posY     = rSGU.posY >> 1;
    uiWidth  = rSGU.width >> 1;
    uiHeight = rSGU.height >> 1;
    pbAvail  = rSGU.isBorderAvailable;    
    pPel     = pImg + (posY * iStride)+ posX;    
    extendBorderCoreFunction(pPel, iStride, pbAvail, uiWidth, uiHeight, uiExtSizeX, uiExtSizeY);
  }
}


/** Copy one luma LCU
 * \param pImgDst destination picture buffer
 * \param pImgSrc souce picture buffer
 * \param iStride stride size of picture buffer
 */
Void CAlfLCU::copyLuma(Pel* pImgDst, Pel* pImgSrc, Int iStride)
{
  UInt uiWidth, uiHeight;
  UInt posX, posY;
  UInt uiOffset;
  Pel* pPelSrc;
  Pel* pPelDst;

  for(Int n =0; n < m_uiNumSGU; n++)
  {
    AlfSGUInfo& rSGU = m_pSGU[n];
    posX     = rSGU.posX;
    posY     = rSGU.posY;
    uiWidth  = rSGU.width;
    uiHeight = rSGU.height;
    uiOffset = (posY * iStride)+ posX;

    pPelDst   = pImgDst + uiOffset;    
    pPelSrc   = pImgSrc + uiOffset;    

    for(Int j=0; j< uiHeight; j++)
    {
      ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiWidth);
      pPelDst += iStride;
      pPelSrc += iStride;
    }

  }
}

/** Copy one chroma LCU
 * \param pImgDst destination picture buffer
 * \param pImgSrc souce picture buffer
 * \param iStride stride size of picture buffer
 */
Void CAlfLCU::copyChroma(Pel* pImgDst, Pel* pImgSrc, Int iStride)
{
  UInt uiWidth, uiHeight;
  UInt posX, posY;
  UInt uiOffset;
  Pel* pPelSrc;
  Pel* pPelDst;

  for(Int n =0; n < m_uiNumSGU; n++)
  {
    AlfSGUInfo& rSGU = m_pSGU[n];
    posX     = rSGU.posX >>1;
    posY     = rSGU.posY >>1;
    uiWidth  = rSGU.width >>1;
    uiHeight = rSGU.height >>1;
    uiOffset = (posY * iStride)+ posX;

    pPelDst   = pImgDst + uiOffset;    
    pPelSrc   = pImgSrc + uiOffset;    

    for(Int j=0; j< uiHeight; j++)
    {
      ::memcpy(pPelDst, pPelSrc, sizeof(Pel)*uiWidth);
      pPelDst += iStride;
      pPelSrc += iStride;
    }
  }
}


/** Copy ALF CU control flags from ALF parameters
 * \param iAlfDepth ALF CU control depth
 * \param puiFlags ALF CU control flags
 */
Void CAlfLCU::getCtrlFlagsFromAlfParam(Int iAlfDepth, UInt* puiFlags)
{
  UInt  uiMaxNumSUInLCU     = m_pcPic->getNumPartInCU();
  Bool  bAllSUsInSameSlice  = (m_uiStartSU == 0)&&( m_uiEndSU == uiMaxNumSUInLCU -1);

  UInt  uiCurrSU, iCUDepth, iSetDepth, uiCtrlNumSU;
  UInt  uiAlfFlag;

  uiCurrSU = m_uiStartSU;
  m_iNumCUCtrlFlags = 0;

  if(bAllSUsInSameSlice) 
  {
    while(uiCurrSU < uiMaxNumSUInLCU)
    {
      //depth of this CU
      iCUDepth = m_pcCU->getDepth(uiCurrSU);

      //choose the min. depth for ALF
      iSetDepth   = (iAlfDepth < iCUDepth)?(iAlfDepth):(iCUDepth);
      uiCtrlNumSU = uiMaxNumSUInLCU >> (iSetDepth << 1);

      uiAlfFlag= puiFlags[m_iNumCUCtrlFlags];

      m_pcCU->setAlfCtrlFlagSubParts(uiAlfFlag, uiCurrSU, (UInt)iSetDepth);

      m_iNumCUCtrlFlags++;
      uiCurrSU += uiCtrlNumSU;
    }
    ::memcpy(m_puiCUCtrlFlag, puiFlags, sizeof(UInt)*m_iNumCUCtrlFlags);
    return;
  }


  UInt  uiLCUX              = m_pcCU->getCUPelX();
  UInt  uiLCUY              = m_pcCU->getCUPelY();
  UInt  uiPicWidth          = m_pcCU->getSlice()->getSPS()->getWidth();
  UInt  uiPicHeight         = m_pcCU->getSlice()->getSPS()->getHeight();

  Bool  bFirst, bValidCU;
  UInt uiIdx, uiLPelX_su, uiTPelY_su;

  bFirst= true;
  while(uiCurrSU <= m_uiEndSU)
  {
    //check picture boundary
    while(!( uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ] < uiPicWidth  ) || 
          !( uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ] < uiPicHeight )
         )
    {
      uiCurrSU++;
      if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > m_uiEndSU)
      {
        break;
      }
    }

    if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > m_uiEndSU)
    {
      break;
    }

    //depth of this CU
    iCUDepth = m_pcCU->getDepth(uiCurrSU);

    //choose the min. depth for ALF
    iSetDepth   = (iAlfDepth < iCUDepth)?(iAlfDepth):(iCUDepth);
    uiCtrlNumSU = uiMaxNumSUInLCU >> (iSetDepth << 1);

    if(bFirst)
    {
      if(uiCurrSU !=0 )
      {
        uiCurrSU = ((UInt)(uiCurrSU/uiCtrlNumSU))* uiCtrlNumSU;
      }
      bFirst = false;
    }

    //alf flag for this CU
    uiAlfFlag= puiFlags[m_iNumCUCtrlFlags];

    bValidCU = false;
    for(uiIdx = uiCurrSU; uiIdx < uiCurrSU + uiCtrlNumSU; uiIdx++)
    {
      if(uiIdx < m_uiStartSU || uiIdx > m_uiEndSU)
      {
        continue;
      }

      uiLPelX_su   = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY_su   = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

      if( !( uiLPelX_su < uiPicWidth )  || !( uiTPelY_su < uiPicHeight )  )
      {
        continue;
      }

      bValidCU = true;
      m_pcCU->setAlfCtrlFlag(uiIdx, uiAlfFlag);
    }

    if(bValidCU)
    {
      m_puiCUCtrlFlag[m_iNumCUCtrlFlags] = uiAlfFlag;
      m_iNumCUCtrlFlags++;
    }

    uiCurrSU += uiCtrlNumSU;
  }
}

/** Copy ALF CU control flags
 * \param iAlfDepth ALF CU control depth
 */
Void CAlfLCU::getCtrlFlagsFromCU(Int iAlfDepth)
{
  UInt  uiMaxNumSUInLCU     = m_pcPic->getNumPartInCU();
  Bool  bAllSUsInSameSlice  = (m_uiStartSU == 0)&&( m_uiEndSU == uiMaxNumSUInLCU -1);

  UInt  uiCurrSU, iCUDepth, iSetDepth, uiCtrlNumSU;
  UInt* puiFlag;

  uiCurrSU = m_uiStartSU;
  m_iNumCUCtrlFlags = 0;
  puiFlag = m_puiCUCtrlFlag;

  if(bAllSUsInSameSlice)
  {
    while(uiCurrSU < uiMaxNumSUInLCU)
    {
      //depth of this CU
      iCUDepth = m_pcCU->getDepth(uiCurrSU);

      //choose the min. depth for ALF
      iSetDepth   = (iAlfDepth < iCUDepth)?(iAlfDepth):(iCUDepth);
      uiCtrlNumSU = uiMaxNumSUInLCU >> (iSetDepth << 1);

      *puiFlag = m_pcCU->getAlfCtrlFlag(uiCurrSU); 

      puiFlag++;
      m_iNumCUCtrlFlags++;

      uiCurrSU += uiCtrlNumSU;
    }

    return;
  }


  UInt  uiLCUX              = m_pcCU->getCUPelX();
  UInt  uiLCUY              = m_pcCU->getCUPelY();
  UInt  uiPicWidth          = m_pcCU->getSlice()->getSPS()->getWidth();
  UInt  uiPicHeight         = m_pcCU->getSlice()->getSPS()->getHeight();

  Bool  bFirst, bValidCU;
  UInt uiIdx, uiLPelX_su, uiTPelY_su;

  bFirst= true;
  while(uiCurrSU <= m_uiEndSU)
  {
    //check picture boundary
    while(!( uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ] < uiPicWidth  ) || 
          !( uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ] < uiPicHeight )
         )
    {
      uiCurrSU++;

      if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > m_uiEndSU)
      {
        break;
      }
    }

    if(uiCurrSU >= uiMaxNumSUInLCU || uiCurrSU > m_uiEndSU)
    {
      break;
    }

    //depth of this CU
    iCUDepth = m_pcCU->getDepth(uiCurrSU);

    //choose the min. depth for ALF
    iSetDepth   = (iAlfDepth < iCUDepth)?(iAlfDepth):(iCUDepth);
    uiCtrlNumSU = uiMaxNumSUInLCU >> (iSetDepth << 1);

    if(bFirst)
    {
      if(uiCurrSU !=0 )
      {
        uiCurrSU = ((UInt)(uiCurrSU/uiCtrlNumSU))* uiCtrlNumSU;
      }
      bFirst = false;
    }

    bValidCU = false;
    for(uiIdx = uiCurrSU; uiIdx < uiCurrSU + uiCtrlNumSU; uiIdx++)
    {
      if(uiIdx < m_uiStartSU || uiIdx > m_uiEndSU)
      {
        continue;
      }

      uiLPelX_su   = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY_su   = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

      if( !( uiLPelX_su < uiPicWidth )  || !( uiTPelY_su < uiPicHeight )  )
      {
        continue;
      }

      bValidCU = true;
    }

    if(bValidCU)
    {
      *puiFlag = m_pcCU->getAlfCtrlFlag(uiCurrSU); 

      puiFlag++;
      m_iNumCUCtrlFlags++;
    }

    uiCurrSU += uiCtrlNumSU;
  }
}


//



//-------------- CAlfSlice -----------------//

/** Initialize one ALF slice unit
 * \param pcPic picture parameters
 * \param iSGDepth slice granularity
 * \param piSliceSUMap slice ID map
 */
Void CAlfSlice::init(TComPic* pcPic, Int iSGDepth, Int* piSliceSUMap)
{
  m_pcPic          = pcPic;
  m_iSGDepth       = iSGDepth;
  m_piSliceSUMap   = piSliceSUMap;
  m_bCUCtrlEnabled = false;
  m_iCUCtrlDepth   = -1;
  m_bValidSlice    = true;
}

/** Create one ALF slice unit
 * \param iSliceID slice ID
 * \param uiStartAddr starting address of the current processing slice
 * \param uiEndAddr ending address of the current processing slice
 */
Void CAlfSlice::create(Int iSliceID, UInt uiStartAddr, UInt uiEndAddr)
{
  m_iSliceID    = iSliceID;
  UInt uiNumSUInLCUHeight = m_pcPic->getNumPartInHeight();
  UInt uiNumSUInLCUWidth  = m_pcPic->getNumPartInWidth();
  UInt uiNumSUInLCU = uiNumSUInLCUHeight * uiNumSUInLCUWidth;

  //start LCU and SU address
#if TILES
  m_uiStartLCU             = m_pcPic->getPicSym()->getPicSCUAddr(uiStartAddr) / uiNumSUInLCU;
#else
  m_uiStartLCU             = uiStartAddr / uiNumSUInLCU;
#endif
  m_uiFirstCUInStartLCU    = uiStartAddr % uiNumSUInLCU;

  //check if the star SU is out of picture boundary
  UInt uiPicWidth  = m_pcPic->getCU(m_uiStartLCU)->getSlice()->getSPS()->getWidth();
  UInt uiPicHeight = m_pcPic->getCU(m_uiStartLCU)->getSlice()->getSPS()->getHeight();
  UInt uiLCUX      = m_pcPic->getCU(m_uiStartLCU)->getCUPelX();
  UInt uiLCUY      = m_pcPic->getCU(m_uiStartLCU)->getCUPelY();
  UInt uiLPelX     = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[m_uiFirstCUInStartLCU] ];
  UInt uiTPelY     = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[m_uiFirstCUInStartLCU] ];
  UInt uiCurrSU    = m_uiFirstCUInStartLCU;
  Bool bMoveToNextLCU = false;

#if TILES
  m_uiStartLCU             = uiStartAddr / uiNumSUInLCU;
#endif

  m_uiEndLCU               = uiEndAddr   / uiNumSUInLCU;
  m_uiLastCUInEndLCU       = uiEndAddr   % uiNumSUInLCU;   
  m_bValidSlice            = true;

  Bool bSliceInOneLCU      = (m_uiStartLCU == m_uiEndLCU);

  while(!( uiLPelX < uiPicWidth ) || !( uiTPelY < uiPicHeight ))
  {
    uiCurrSU ++;

    if(bSliceInOneLCU)
    {
      if(uiCurrSU > m_uiLastCUInEndLCU)
      {
        m_bValidSlice = false;
        break;
      }
    }

    if(uiCurrSU >= uiNumSUInLCU )
    {
      bMoveToNextLCU = true;
      break;
    }

    uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
    uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

  }

  if(!m_bValidSlice)
  {
    return;
  }

  if(uiCurrSU != m_uiFirstCUInStartLCU)
  {
    if(!bMoveToNextLCU)
    {
      m_uiFirstCUInStartLCU = uiCurrSU;
    }
    else
    {
      m_uiStartLCU++;
      m_uiFirstCUInStartLCU = 0;
      assert(m_uiStartLCU < m_pcPic->getNumCUsInFrame());
    }

    assert(m_uiStartLCU*uiNumSUInLCU + m_uiFirstCUInStartLCU < uiEndAddr);

  }


  m_uiNumLCUs              = m_uiEndLCU - m_uiStartLCU +1;

  m_pcAlfLCU = new CAlfLCU[m_uiNumLCUs];
  for(UInt uiAddr= m_uiStartLCU; uiAddr <=  m_uiEndLCU; uiAddr++)
  {
    UInt uiStartSU = (uiAddr == m_uiStartLCU)?(m_uiFirstCUInStartLCU):(0);
    UInt uiEndSU   = (uiAddr == m_uiEndLCU  )?(m_uiLastCUInEndLCU   ):(uiNumSUInLCU -1);

#if TILES
    m_pcAlfLCU[uiAddr - m_uiStartLCU].create(m_iSliceID, m_pcPic, m_pcPic->getPicSym()->getCUOrderMap(uiAddr), uiStartSU, uiEndSU, m_iSGDepth);
#else
    m_pcAlfLCU[uiAddr - m_uiStartLCU].create(m_iSliceID, m_pcPic, uiAddr, uiStartSU, uiEndSU, m_iSGDepth);
#endif
  }


  UInt uiNumLCUInPicWidth = m_pcPic->getFrameWidthInCU();
  UInt uiNumLCUInPicHeight= m_pcPic->getFrameHeightInCU();
  for(UInt i= 0; i <  m_uiNumLCUs; i++)
  {
    m_pcAlfLCU[i].setSGUBorderAvailability(uiNumLCUInPicWidth, uiNumLCUInPicHeight,uiNumSUInLCUWidth, uiNumSUInLCUHeight, m_piSliceSUMap);
  }
}

/** Destroy one ALF slice unit
 */
Void CAlfSlice::destroy()
{

  if(m_pcAlfLCU != NULL)
  {
    delete[] m_pcAlfLCU;
    m_pcAlfLCU = NULL;
  }

}

/** Extend slice boundary for one luma slice
 * \param [in,out] pPelSrc picture buffer
 * \param [in] iStride stride size of picture buffer
 * \param [in] filtNo filter shape
 */
Void CAlfSlice::extendSliceBorderLuma(Pel* pPelSrc, Int iStride, Int filtNo)
{
  for(UInt idx = 0; idx < m_uiNumLCUs; idx++)
  {
    m_pcAlfLCU[idx].extendLumaBorder(pPelSrc, iStride, filtNo);
  }
}

/** Extend slice boundary for one chroma slice
* \param [in,out] pPelSrc picture buffer
* \param [in] iStride stride size of picture buffer
* \param [in] filtNo filter shape
 */
Void CAlfSlice::extendSliceBorderChroma(Pel* pPelSrc, Int iStride, UInt filtNo)
{
  for(UInt idx = 0; idx < m_uiNumLCUs; idx++)
  {
    m_pcAlfLCU[idx].extendChromaBorder(pPelSrc, iStride, filtNo);
  }

}

/** Copy one luma slice
 * \param pPicDst destination picture buffer
 * \param pPicSrc source picture buffer
 * \param iStride stride size of picture buffer
 */
Void CAlfSlice::copySliceLuma(Pel* pPicDst, Pel* pPicSrc, Int iStride )
{
  for(UInt idx = 0; idx < m_uiNumLCUs; idx++)
  {
    m_pcAlfLCU[idx].copyLuma(pPicDst, pPicSrc, iStride);
  }
}

/** Copy one chroma slice
 * \param pPicDst destination picture buffer
 * \param pPicSrc source picture buffer
 * \param iStride stride size of picture buffer
 */
Void CAlfSlice::copySliceChroma(Pel* pPicDst, Pel* pPicSrc, Int iStride )
{

  for(UInt idx = 0; idx < m_uiNumLCUs; idx++)
  {
    m_pcAlfLCU[idx].copyChroma(pPicDst, pPicSrc, iStride);
  }

}

/** Copy ALF CU Control Flags for one slice
 */
Void CAlfSlice::getCtrlFlagsForOneSlice()
{
  assert(m_bCUCtrlEnabled);

  m_iNumCUCtrlFlags = 0;
  for(UInt idx =0; idx < m_uiNumLCUs; idx++)
  {
    m_pcAlfLCU[idx].getCtrlFlagsFromCU(m_iCUCtrlDepth);
    m_iNumCUCtrlFlags += m_pcAlfLCU[idx].getNumCtrlFlags();
  }
}


#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
/** PCM LF disable process. 
 * \param pcPic picture (TComPic) pointer
 * \returns Void
 *
 * \note Replace filtered sample values of PCM mode blocks with the transmitted and reconstructed ones.
 */
Void TComAdaptiveLoopFilter::PCMLFDisableProcess (TComPic* pcPic)
{
  xPCMRestoration(pcPic);
}

/** Picture-level PCM restoration. 
 * \param pcPic picture (TComPic) pointer
 * \returns Void
 */
Void TComAdaptiveLoopFilter::xPCMRestoration(TComPic* pcPic)
{
  Bool  bPCMFilter = (pcPic->getSlice(0)->getSPS()->getPCMFilterDisableFlag() && ((1<<pcPic->getSlice(0)->getSPS()->getPCMLog2MinSize()) <= g_uiMaxCUWidth))? true : false;

  if(bPCMFilter)
  {
    for( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame() ; uiCUAddr++ )
    {
      TComDataCU* pcCU = pcPic->getCU(uiCUAddr);

      xPCMCURestoration(pcCU, 0, 0); 
    } 
  }
}

/** PCM CU restoration. 
 * \param pcCU pointer to current CU
 * \param uiAbsPartIdx part index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TComAdaptiveLoopFilter::xPCMCURestoration ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
{
  TComPic* pcPic     = pcCU->getPic();
  UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts   = uiCurNumParts>>2;

  // go to sub-CU
  if( pcCU->getDepth(uiAbsZorderIdx) > uiDepth )
  {
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xPCMCURestoration( pcCU, uiAbsZorderIdx, uiDepth+1 );
    }
    return;
  }

  // restore PCM samples
  if (pcCU->getIPCMFlag(uiAbsZorderIdx))
  {
    xPCMSampleRestoration (pcCU, uiAbsZorderIdx, uiDepth, TEXT_LUMA    );
    xPCMSampleRestoration (pcCU, uiAbsZorderIdx, uiDepth, TEXT_CHROMA_U);
    xPCMSampleRestoration (pcCU, uiAbsZorderIdx, uiDepth, TEXT_CHROMA_V);
  }
}

/** PCM sample restoration. 
 * \param pcCU pointer to current CU
 * \param uiAbsPartIdx part index
 * \param uiDepth CU depth
 * \param ttText texture component type
 * \returns Void
 */
Void TComAdaptiveLoopFilter::xPCMSampleRestoration (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, TextType ttText)
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc;
  Pel* piPcm;
  UInt uiStride;
  UInt uiWidth;
  UInt uiHeight;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
  UInt uiPcmLeftShiftBit; 
#endif
  UInt uiX, uiY;
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsZorderIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;

  if( ttText == TEXT_LUMA )
  {
    piSrc = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx);
    piPcm = pcCU->getPCMSampleY() + uiLumaOffset;
    uiStride  = pcPicYuvRec->getStride();
    uiWidth  = (g_uiMaxCUWidth >> uiDepth);
    uiHeight = (g_uiMaxCUHeight >> uiDepth);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiPcmLeftShiftBit = g_uiBitDepth + g_uiBitIncrement - pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();
#endif
  }
  else
  {
    if( ttText == TEXT_CHROMA_U )
    {
      piSrc = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
      piPcm = pcCU->getPCMSampleCb() + uiChromaOffset;
    }
    else
    {
      piSrc = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
      piPcm = pcCU->getPCMSampleCr() + uiChromaOffset;
    }

    uiStride = pcPicYuvRec->getCStride();
    uiWidth  = ((g_uiMaxCUWidth >> uiDepth)/2);
    uiHeight = ((g_uiMaxCUWidth >> uiDepth)/2);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiPcmLeftShiftBit = g_uiBitDepth + g_uiBitIncrement - pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#endif
  }

  for( uiY = 0; uiY < uiHeight; uiY++ )
  {
    for( uiX = 0; uiX < uiWidth; uiX++ )
    {
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
      piSrc[uiX] = (piPcm[uiX] << uiPcmLeftShiftBit);
#else
      if(g_uiBitIncrement > 0)
      {
        piSrc[uiX] = (piPcm[uiX] << g_uiBitIncrement);
      }
      else
      {
        piSrc[uiX] = piPcm[uiX];
      }
#endif
    }
    piPcm += uiWidth;
    piSrc += uiStride;
  }
}
#endif
//! \}
