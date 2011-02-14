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

/** \file     TComAdaptiveLoopFilter.cpp
    \brief    adaptive loop filter class
*/

#include "TComAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ====================================================================================================================
// Tables
// ====================================================================================================================

#if TI_ALF_MAX_VSIZE_7
Int TComAdaptiveLoopFilter::m_pattern9x9Sym[39] = 
#else
Int TComAdaptiveLoopFilter::m_pattern9x9Sym[41] = 
#endif
{
                   0,
               1,  2,  3,
           4,  5,  6,  7,  8,
       9, 10, 11, 12, 13, 14, 15,
#if TI_ALF_MAX_VSIZE_7
      16, 17, 18, 19, 18, 17, 16,
#else
  16, 17, 18, 19, 20, 19, 18, 17, 16,
#endif
      15, 14, 13, 12, 11, 10,  9, 
           8,  7,  6,  5,  4,
               3,  2,  1,
                   0
};
 
#if TI_ALF_MAX_VSIZE_7
Int TComAdaptiveLoopFilter::m_weights9x9Sym[21] = 
#else
Int TComAdaptiveLoopFilter::m_weights9x9Sym[22] = 
#endif
{
#if !TI_ALF_MAX_VSIZE_7
                   2,
#endif
               2,  2,  2,   
           2,  2,  2,  2,  2, 
       2,  2,  2,  2,  2,  2,  2,  
   2,  2,  2,  2,  1,  1
};

#if TI_ALF_MAX_VSIZE_7
Int TComAdaptiveLoopFilter::m_pattern9x9Sym_Quart[42] = 
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  1,  2,  3,  0,  0,  0,
   0,  0,  4,  5,  6,  7,  8,  0,  0,  
   0,  9, 10, 11, 12, 13, 14, 15,  0,
  16, 17, 18, 19, 20, 21
};
#else
Int TComAdaptiveLoopFilter::m_pattern9x9Sym_Quart[42] = 
{
   0,  0,  0,  0,  1,  0,  0,  0,  0,
   0,  0,  0,  2,  3,  4,  0,  0,  0,
   0,  0,  5,  6,  7,  8,  9,  0,  0,  
   0, 10, 11, 12, 13, 14, 15, 16,  0,
  17, 18, 19, 20, 21, 22
};
#endif

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

#if TI_ALF_MAX_VSIZE_7
Int TComAdaptiveLoopFilter::m_pattern9x9Sym_9[39] = 
#else
Int TComAdaptiveLoopFilter::m_pattern9x9Sym_9[41] = 
#endif
{
#if !TI_ALF_MAX_VSIZE_7
                   4,
#endif
              12, 13, 14,  
          20, 21, 22, 23, 24, 
      28, 29, 30, 31, 32, 33, 34,      
  36, 37, 38, 39, 40, 39, 38, 37, 36, 
      34, 33, 32, 31, 30, 29, 28,  
          24, 23, 22, 21, 20, 
              14, 13, 12,
#if !TI_ALF_MAX_VSIZE_7
                   4,  
#endif
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
  SQR_FILT_LENGTH_9SYM, SQR_FILT_LENGTH_7SYM, SQR_FILT_LENGTH_5SYM
};

#if TI_ALF_MAX_VSIZE_7
Int depthInt9x9Sym[21] = 
#else
Int depthInt9x9Sym[22] = 
#endif
{
#if !TI_ALF_MAX_VSIZE_7
              5, 
#endif
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

#if TI_ALF_MAX_VSIZE_7
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag9x7[32] =
{
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1
};
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComAdaptiveLoopFilter::TComAdaptiveLoopFilter()
{
  m_pcTempPicYuv = NULL;
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
  initMatrix_imgpel(&m_imgY_var, m_img_height, m_img_width); 
  initMatrix_int(&m_imgY_temp, m_img_height+2*VAR_SIZE+3, m_img_width+2*VAR_SIZE+3);
  
  initMatrix_int(&m_filterCoeffSym, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);
  initMatrix_int(&m_filterCoeffPrevSelected, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
  initMatrix_int(&m_filterCoeffTmp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);      
  initMatrix_int(&m_filterCoeffSymTmp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);   
}

Void TComAdaptiveLoopFilter::destroy()
{
  if ( m_pcTempPicYuv )
  {
    m_pcTempPicYuv->destroy();
    delete m_pcTempPicYuv;
  }
  destroyMatrix_imgpel(m_imgY_var); 
  
  destroyMatrix_int(m_filterCoeffSym);
  destroyMatrix_int(m_filterCoeffPrevSelected);
  destroyMatrix_int(m_filterCoeffTmp);
  destroyMatrix_int(m_filterCoeffSymTmp);
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if TI_ALF_MAX_VSIZE_7
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
#endif

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
#if TSB_ALF_HEADER
  pAlfParam->num_cus_in_frame = m_uiNumCUsInFrame;
  pAlfParam->num_alf_cu_flag  = 0;
  pAlfParam->alf_cu_flag      = new UInt[(m_uiNumCUsInFrame << ((g_uiMaxCUDepth-1)*2))];
#endif
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
#if TSB_ALF_HEADER
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
  pDesAlfParam->cu_control_flag = pSrcAlfParam->cu_control_flag;
  pDesAlfParam->chroma_idc = pSrcAlfParam->chroma_idc;
  pDesAlfParam->tap = pSrcAlfParam->tap;
#if TI_ALF_MAX_VSIZE_7
  pDesAlfParam->tapV = pSrcAlfParam->tapV;
#endif
  pDesAlfParam->num_coeff = pSrcAlfParam->num_coeff;
  pDesAlfParam->tap_chroma = pSrcAlfParam->tap_chroma;
  pDesAlfParam->num_coeff_chroma = pSrcAlfParam->num_coeff_chroma;
  
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
  
#if TSB_ALF_HEADER
  pDesAlfParam->num_alf_cu_flag = pSrcAlfParam->num_alf_cu_flag;
  ::memcpy(pDesAlfParam->alf_cu_flag, pSrcAlfParam->alf_cu_flag, sizeof(UInt)*pSrcAlfParam->num_alf_cu_flag);
#endif
}

// --------------------------------------------------------------------------------------------------------------------
// prediction of filter coefficients
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::predictALFCoeff( ALFParam* pAlfParam)
{
  Int i, sum, pred, tap, N;
  const Int* pFiltMag = NULL;
  
  tap = pAlfParam->tap;
#if TI_ALF_MAX_VSIZE_7
  Int tapV = pAlfParam->tapV;
#endif
  
  switch(tap)
  {
    case 5:
      pFiltMag = m_aiSymmetricMag5x5;
      break;
    case 7:
      pFiltMag = m_aiSymmetricMag7x7;
      break;
    case 9:
#if TI_ALF_MAX_VSIZE_7
      pFiltMag = m_aiSymmetricMag9x7;
#else
      pFiltMag = m_aiSymmetricMag9x9;
#endif
      break;
    default:
      assert(0);
      break;
  }
#if TI_ALF_MAX_VSIZE_7
  N = (tap * tapV + 1) >> 1;
#else
  N = (tap * tap + 1) >> 1;
#endif
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
  Int i, sum, pred, tap, N;
  const Int* pFiltMag = NULL;
  
  tap = pAlfParam->tap_chroma;
  switch(tap)
  {
    case 5:
      pFiltMag = m_aiSymmetricMag5x5;
      break;
    case 7:
      pFiltMag = m_aiSymmetricMag7x7;
      break;
    case 9:
      pFiltMag = m_aiSymmetricMag9x9;
      break;
    default:
      assert(0);
      break;
  }
  N = (tap*tap+1)>>1;
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
 \param pcPic         picture (TComPic) class (input/output)
 \param pcAlfParam    ALF parameter
 \todo   for temporal buffer, it uses residual picture buffer, which may not be safe. Make it be safer.
 */
Void TComAdaptiveLoopFilter::ALFProcess(TComPic* pcPic, ALFParam* pcAlfParam)
{
  if(!pcAlfParam->alf_flag)
  {
    return;
  }
  
  TComPicYuv* pcPicYuvRec    = pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;
  
  pcPicYuvRec   ->copyToPic          ( pcPicYuvExtRec );
  pcPicYuvExtRec->setBorderExtension ( false );
  pcPicYuvExtRec->extendPicBorder    ();
  
#if TSB_ALF_HEADER
  if(pcAlfParam->cu_control_flag)
  {
    UInt idx = 0;
    for(UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++)
    {
      TComDataCU *pcCU = pcPic->getCU(uiCUAddr);
      setAlfCtrlFlags(pcAlfParam, pcCU, 0, 0, idx);
    }
  }
#endif
  xALFLuma_qc(pcPic, pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);
  
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
Void TComAdaptiveLoopFilter::xALFLuma_qc(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Int    LumaStride = pcPicDec->getStride();
  imgpel* pDec = (imgpel*)pcPicDec->getLumaAddr();
  imgpel* pRest = (imgpel*)pcPicRest->getLumaAddr();
  
  //Decode and reconst filter coefficients
  DecFilter_qc(pDec,pcAlfParam,LumaStride);
  //set maskImg using cu adaptive one.
  if(pcAlfParam->cu_control_flag)
  {
    xCUAdaptive_qc(pcPic, pcAlfParam, pRest, pDec, LumaStride);
  }  
  else
  {
    //then do whole frame filtering
    filterFrame(pRest, pDec, pcAlfParam->realfiltNo, LumaStride);
  }
}


Void TComAdaptiveLoopFilter::DecFilter_qc(imgpel* imgY_rec,ALFParam* pcAlfParam, int Stride)
{
  int i;
  int numBits = NUM_BITS; 
  int fl=FILTER_LENGTH/2;
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
  
  memset(m_imgY_temp[0],0,sizeof(int)*(m_img_height+2*VAR_SIZE)*(m_img_width+2*VAR_SIZE));
  
  calcVar(m_imgY_var, imgY_rec, fl, VAR_SIZE, m_img_height, m_img_width, Stride);
}

Void TComAdaptiveLoopFilter::getCurrentFilter(int **filterCoeffSym,ALFParam* pcAlfParam)
{ 
  int i,  k, varInd;
  int *patternMap;
  int *patternMapTab[3]={m_pattern9x9Sym_Quart, m_pattern7x7Sym_Quart, m_pattern5x5Sym_Quart};
  {
    for(varInd=0; varInd<NO_VAR_BINS; ++varInd)
    {
      memset(m_filterCoeffPrevSelected[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
    }
    patternMap=patternMapTab[pcAlfParam->realfiltNo];
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

Void TComAdaptiveLoopFilter::calcVar(imgpel **imgY_var, imgpel *imgY_pad, int pad_size, int fl, int img_height, int img_width, int img_stride)
{
  int i, j, ii, jj;
  int sum;
  int *p_imgY_temp;
#if FULL_NBIT
  int shift= (11+ g_uiBitIncrement + g_uiBitDepth - 8);
#else
  int shift= (11+ g_uiBitIncrement);
#endif
  int fl2plusOne= (VAR_SIZE<<1)+1;
  int pad_offset = pad_size-fl-1;
  int var_max= NO_VAR_BINS-1;
  int mult_fact_int_tab[4]= {1,114,41,21};
  int mult_fact_int = mult_fact_int_tab[VAR_SIZE];
  
  if (VAR_SIZE ==0)
  {
    imgpel *p_imgY_var;
#if FULL_NBIT
    shift = g_uiBitIncrement + g_uiBitDepth - 8;
#else
    shift = g_uiBitIncrement;
#endif
    //current
    for(i = 1; i < img_height + fl2plusOne; i++)
    {
      imgpel *p_imgY_pad = &imgY_pad[(pad_offset+i-pad_size) * img_stride + pad_offset-pad_size];
      imgpel *p_imgY_pad_up   = &imgY_pad[(pad_offset+i+1-pad_size) * img_stride + pad_offset-pad_size];
      imgpel *p_imgY_pad_down = &imgY_pad[(pad_offset+i-1-pad_size) * img_stride + pad_offset-pad_size];
      p_imgY_temp = (int*)&m_imgY_temp[i-1][0];
      p_imgY_var  = &imgY_var [i-1][0];
      for(j = 1; j < img_width +fl2plusOne; j++)
      {
        *(p_imgY_temp) = abs((p_imgY_pad[j]<<1) - p_imgY_pad[j+1] - p_imgY_pad[j-1])+
        abs((p_imgY_pad[j]<<1) - p_imgY_pad_down[j] - p_imgY_pad_up[j]);
        *(p_imgY_var++) =(imgpel) Clip_post(var_max, (int) ((*(p_imgY_temp++))>>shift));
      }
    }
    return;
  }
  
  //current
  for(i = 1; i < img_height + fl2plusOne; i++)
  {
    imgpel *p_imgY_pad = &imgY_pad[(pad_offset+i-pad_size) * img_stride + pad_offset-pad_size];
    imgpel *p_imgY_pad_up   = &imgY_pad[(pad_offset+i+1-pad_size) * img_stride + pad_offset-pad_size];
    imgpel *p_imgY_pad_down = &imgY_pad[(pad_offset+i-1-pad_size) * img_stride + pad_offset-pad_size];
    p_imgY_temp = (int*)&m_imgY_temp[i-1][0];
    for(j = 1; j < img_width +fl2plusOne; j++)
    {
      *(p_imgY_temp++) = abs((p_imgY_pad[j]<<1) - p_imgY_pad[j+1] - p_imgY_pad[j-1])+
        abs((p_imgY_pad[j]<<1) - p_imgY_pad_down[j] - p_imgY_pad_up[j]);
    }
  }
  {
    //int temp;
    int sum_0=0;
    int y=img_height+((VAR_SIZE+1)<<1);

    int *p_imgY_temp_sum;
    i = fl;
    j = fl;

    memset(m_imgY_temp[y],0,sizeof(int)*(img_width+((VAR_SIZE+1)<<1)));
    //--------------------------------------------------------------------------------------------

    for(ii = i - fl; ii <= i + fl; ii++)
    {
      p_imgY_temp= (int*)&m_imgY_temp[ii][j - fl];
      p_imgY_temp_sum=(int*)&m_imgY_temp[y][j - fl];
      for(jj = j - fl; jj <= j + fl; jj++)
      {
        *(p_imgY_temp_sum++) += *(p_imgY_temp++);
      }
    }
    p_imgY_temp_sum=(int*)&m_imgY_temp[y][j - fl];
    for(jj = j - fl; jj <= j + fl; jj++)
      sum_0+=*(p_imgY_temp_sum++);
    imgY_var[i - fl][j - fl] = (imgpel) Clip_post(var_max, (int) ((sum_0 * mult_fact_int)>>shift));
    //--------------------------------------------------------------------------------------------
    sum = sum_0;
    for(j = fl+1; j < img_width + fl; ++j)
    {
      int k=j+fl;
      for(ii = i - fl; ii <= i + fl; ii++)
        m_imgY_temp[y][k] += (m_imgY_temp[ii][k]);

      sum += (m_imgY_temp[y][k]-m_imgY_temp[y][j - fl-1]);
      imgY_var[i - fl][j - fl] = (imgpel) Clip_post(var_max, (int) ((sum * mult_fact_int)>>shift));

    }
    //--------------------------------------------------------------------------------------------

    for(i = fl+1; i < img_height + fl; ++i)
    {
      imgpel  *pimgY_var= &imgY_var[i-fl][0];
      int *p_imgY_temp1;
      int *p_imgY_temp2;
      sum = sum_0;
      j= fl;
      p_imgY_temp1= (int*)&m_imgY_temp[i+fl  ][j - fl];
      p_imgY_temp2= (int*)&m_imgY_temp[i-fl-1][j - fl];
      p_imgY_temp_sum=(int*)&m_imgY_temp[y][j - fl];
      for(jj = j - fl; jj <= j + fl; jj++)
      {
        int diff = *(p_imgY_temp1++)-*(p_imgY_temp2++);
        *(p_imgY_temp_sum++) += diff;
        sum += diff;
      }
      sum_0=sum;

      *(pimgY_var++) = (imgpel) Clip_post(var_max, (int) ((sum * mult_fact_int)>>shift));
      //--------------------------------------------------------------------------------------------
      p_imgY_temp_sum=(int*)m_imgY_temp[y];
      for(j = fl+1; j < img_width + fl; ++j)
      {
        int k = j+fl;
        p_imgY_temp_sum[k] += (m_imgY_temp[i + fl][k]-m_imgY_temp[i-fl-1][k]);
        sum += (p_imgY_temp_sum[k]-p_imgY_temp_sum[j-fl-1]);
        *(pimgY_var++) = (imgpel) Clip_post(var_max, (int) ((sum * mult_fact_int)>>shift));
      }
    }
  }
}

Void TComAdaptiveLoopFilter::filterFrame(imgpel *imgY_rec_post, imgpel *imgY_rec, int filtNo, int Stride)
{
  int i, j, ii, jj, pixelInt,m=0;
  imgpel *p_imgY_var,*p_imgY_pad;
  int max_val=g_uiIBDI_MAX;
  int fl=FILTER_LENGTH/2;
  int *pattern=m_pattern9x9Sym;
  int fl_temp;
  int last_coef= MAX_SQR_FILT_LENGTH-1;
  imgpel *im1,*im2;
  int *coef;
  int num_bits_minus_1= NUM_BITS-1;
  int offset = (1<<(NUM_BITS-2));
  int *pattern_fix=m_patternTab_filt[filtNo];
  fl_temp=m_flTab[filtNo];
#if TI_ALF_MAX_VSIZE_7
  Int fl_tempV = ALFFlHToFlV(fl_temp);
#endif
  
  // Filter
  for (i = fl; i < m_img_height+fl; i++)
  {
    p_imgY_var = m_imgY_var[i-fl];
    p_imgY_pad = imgY_rec + (i-fl)*Stride;
    for (j = fl; j < m_img_width+fl; j++)
    {
      coef = m_filterCoeffPrevSelected[*(p_imgY_var++)];
      pixelInt=coef[last_coef];
      pattern=pattern_fix;

#if TI_ALF_MAX_VSIZE_7
      for (ii = -fl_tempV, m = fl_temp - fl_tempV; ii < 0; ii++, m++)
#else
      for (ii = -fl_temp, m = 0; ii < 0; ii++, m++)
#endif
      {
        im1= &(imgY_rec[(i-fl+ii)*Stride + j-fl-m]);
        im2= &(imgY_rec[(i-fl-ii)*Stride + j-fl+m]);
        for (jj=-m; jj<=m; jj++)
          pixelInt+=((*(im1++)+ *(im2--))*coef[*(pattern++)]);
      }

      im1= &(p_imgY_pad[(j-fl)-fl_temp]);
      im2= &(p_imgY_pad[(j-fl)+fl_temp]);
      for (jj=0; jj<fl_temp; jj++)
        pixelInt+=((*(im1++)+ *(im2--))*coef[*(pattern++)]);

      pixelInt+=(p_imgY_pad[j-fl]*coef[*(pattern++)]);
      pixelInt=(int)((pixelInt+offset) >> (num_bits_minus_1));
      imgY_rec_post[(i-fl)*Stride + j-fl]=max(0, min(pixelInt,max_val));
    }
  }
}

Void TComAdaptiveLoopFilter::subfilterFrame(imgpel *imgY_rec_post, imgpel *imgY_rec, int filtNo, int start_height, int end_height, int start_width, int end_width, int Stride)
{
  int i, j, ii, jj, pixelInt,m=0;
  imgpel *p_imgY_var,*p_imgY_pad;
  int max_val=g_uiIBDI_MAX;
  int fl=FILTER_LENGTH/2;
  int *pattern=m_pattern9x9Sym;
  int fl_temp;
  int last_coef= MAX_SQR_FILT_LENGTH-1;
  imgpel *im1,*im2;
  int *coef;
  int num_bits_minus_1= NUM_BITS-1;
  int offset = (1<<(NUM_BITS-2));
  int *pattern_fix=m_patternTab_filt[filtNo];
  fl_temp=m_flTab[filtNo];
#if TI_ALF_MAX_VSIZE_7
  Int fl_tempV = ALFFlHToFlV(fl_temp);
#endif
  
  // Filter
  for (i = fl + start_height; i < end_height+fl; i++)
  {
    p_imgY_var = m_imgY_var[i-fl] + start_width;
    p_imgY_pad = imgY_rec + (i-fl)*Stride;
    for (j = fl + start_width; j < end_width+fl; j++)
    {
      coef = m_filterCoeffPrevSelected[*(p_imgY_var++)];
      pixelInt=coef[last_coef];
      pattern=pattern_fix;

#if TI_ALF_MAX_VSIZE_7
      for (ii = -fl_tempV, m = fl_temp - fl_tempV; ii < 0; ii++, m++)
#else
      for (ii = -fl_temp, m = 0; ii < 0; ii++, m++)
#endif
      {
        im1= &(imgY_rec[(i-fl+ii)*Stride + j-fl-m]);
        im2= &(imgY_rec[(i-fl-ii)*Stride + j-fl+m]);
        for (jj=-m; jj<=m; jj++)
          pixelInt+=((*(im1++)+ *(im2--))*coef[*(pattern++)]);
      }

      im1= &(p_imgY_pad[(j-fl)-fl_temp]);
      im2= &(p_imgY_pad[(j-fl)+fl_temp]);
      for (jj=0; jj<fl_temp; jj++)
        pixelInt+=((*(im1++)+ *(im2--))*coef[*(pattern++)]);

      pixelInt+=(p_imgY_pad[j-fl]*coef[*(pattern++)]);
      pixelInt=(int)((pixelInt+offset) >> (num_bits_minus_1));
      imgY_rec_post[(i-fl)*Stride + j-fl]=max(0, min(pixelInt,max_val));
    }
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
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 0);
  }
  
  if(pcAlfParam->chroma_idc&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 1);
  }
}

/** 
 \param pcPicDec    picture before ALF
 \param pcPicRest   picture after  ALF
 \param qh          filter coefficient
 \param iTap        filter tap
 \param iColor      0 for Cb and 1 for Cr
 */
Void TComAdaptiveLoopFilter::xFrameChroma( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor )
{
  Int i, x, y, value, N, offset;
  //  Pel PixSum[ALF_MAX_NUM_COEF_C];// th
  Pel PixSum[ALF_MAX_NUM_COEF]; 
  
  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;
  
  Int iHeight = pcPicRest->getHeight() >> 1;
  Int iWidth = pcPicRest->getWidth() >> 1;
  
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
  
  Pel* pTmpDec1, *pTmpDec2;
  Pel* pTmpPixSum;
  
  switch(iTap)
  {
    case 5:
    {
      Int iJump = iDecStride - 4;
      pDec -= iDecStride*2;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-2;
          pTmpDec2 = pTmpDec1+4+(4*iDecStride);
          pTmpPixSum = PixSum;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);
          
          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;
          
          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
      break;
    case 7:
    {
      Int iJump = iDecStride - 6;
      pDec -= iDecStride*3;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-3;
          pTmpDec2 = pTmpDec1+6+(6*iDecStride);
          pTmpPixSum = PixSum;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);
          
          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;
          
          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
      break;
    case 9:
    {
      Int iJump = iDecStride - 8;
      pDec -= iDecStride*4;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-4;
          pTmpDec2 = pTmpDec1+8+(8*iDecStride);
          pTmpPixSum = PixSum;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;
          
          
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum =(*pTmpDec1);
          
          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;
          
          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
      break;
    default:
      assert(0);
      break;
  }
}

#if TSB_ALF_HEADER
Void TComAdaptiveLoopFilter::setNumCUsInFrame(TComPic *pcPic)
{
  m_uiNumCUsInFrame = pcPic->getNumCUsInFrame();
}

Void TComAdaptiveLoopFilter::setAlfCtrlFlags(ALFParam *pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx)
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
#endif
