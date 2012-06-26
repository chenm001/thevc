/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TEncAdaptiveLoopFilter.cpp
 \brief    estimation part of adaptive loop filter class
 */
#include "TEncAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//! \ingroup TLibEncoder
//! \{
#if !AHG6_ALF_OPTION2
// ====================================================================================================================
// Constants
// ====================================================================================================================
#define ALF_NUM_OF_REDESIGN 1
// ====================================================================================================================
// Tables
// ====================================================================================================================
const Int TEncAdaptiveLoopFilter::m_alfNumPartsInRowTab[5] =
{
  1,      //level 0
  2,      //level 1
  4,      //level 2
  8,      //level 3
  16      //level 4
};

const Int TEncAdaptiveLoopFilter::m_alfNumPartsLevelTab[5] =
{
  1,      //level 0
  4,      //level 1
  16,     //level 2
  64,     //level 3
  256     //level 4
};

const Int TEncAdaptiveLoopFilter::m_alfNumCulPartsLevelTab[5] =
{
  1,    //level 0
  5,    //level 1
  21,   //level 2
  85,   //level 3
  341,  //level 4
};
#endif
// ====================================================================================================================
// Constructor / destructor
// ====================================================================================================================

///AlfCorrData
AlfCorrData::AlfCorrData()
{
  this->componentID = -1;
  this->ECorr  = NULL;
  this->yCorr  = NULL;
  this->pixAcc = NULL;
}

AlfCorrData::AlfCorrData(Int cIdx)
{
  const Int numCoef = ALF_MAX_NUM_COEF;
  const Int maxNumGroups = NO_VAR_BINS;

  Int numGroups = (cIdx == ALF_Y)?(maxNumGroups):(1);

  this->componentID = cIdx;

  this->ECorr = new Double**[numGroups];
  this->yCorr = new Double*[numGroups];
  this->pixAcc = new Double[numGroups];
  for(Int g= 0; g< numGroups; g++)
  {
    this->yCorr[g] = new Double[numCoef];
    for(Int j=0; j< numCoef; j++)
    {
      this->yCorr[g][j] = 0;
    }

    this->ECorr[g] = new Double*[numCoef];
    for(Int i=0; i< numCoef; i++)
    {
      this->ECorr[g][i] = new Double[numCoef];
      for(Int j=0; j< numCoef; j++)
      {
        this->ECorr[g][i][j] = 0;
      }
    }
    this->pixAcc[g] = 0;  
  }
}

AlfCorrData::~AlfCorrData()
{
  if(this->componentID >=0)
  {
    const Int numCoef = ALF_MAX_NUM_COEF;
    const Int maxNumGroups = NO_VAR_BINS;

    Int numGroups = (this->componentID == ALF_Y)?(maxNumGroups):(1);

    for(Int g= 0; g< numGroups; g++)
    {
      for(Int i=0; i< numCoef; i++)
      {
        delete[] this->ECorr[g][i];
      }
      delete[] this->ECorr[g];
      delete[] this->yCorr[g];
    }
    delete[] this->ECorr;
    delete[] this->yCorr;
    delete[] this->pixAcc;
  }

}

AlfCorrData& AlfCorrData::operator += (const AlfCorrData& src)
{
  if(this->componentID >=0)
  {
    const Int numCoef = ALF_MAX_NUM_COEF;
    const Int maxNumGroups = NO_VAR_BINS;

    Int numGroups = (this->componentID == ALF_Y)?(maxNumGroups):(1);
    for(Int g=0; g< numGroups; g++)
    {
      this->pixAcc[g] += src.pixAcc[g];

      for(Int j=0; j< numCoef; j++)
      {
        this->yCorr[g][j] += src.yCorr[g][j];
        for(Int i=0; i< numCoef; i++)
        {
          this->ECorr[g][j][i] += src.ECorr[g][j][i];
        }
      }
    }
  }

  return *this;
}


Void AlfCorrData::reset()
{
  if(this->componentID >=0)
  {
    const Int numCoef = ALF_MAX_NUM_COEF;
    const Int maxNumGroups = NO_VAR_BINS;

    Int numGroups = (this->componentID == ALF_Y)?(maxNumGroups):(1);
    for(Int g=0; g< numGroups; g++)
    {
      this->pixAcc[g] = 0;

      for(Int j=0; j< numCoef; j++)
      {
        this->yCorr[g][j] = 0;
        for(Int i=0; i< numCoef; i++)
        {
          this->ECorr[g][j][i] = 0;
        }
      }


    }
  }

}

Void AlfCorrData::mergeFrom(const AlfCorrData& src, Int* mergeTable, Bool doPixAccMerge)
{
  assert(componentID == src.componentID);

  reset();

  const Int numCoef = ALF_MAX_NUM_COEF;

  Double **srcE, **dstE;
  Double *srcy, *dsty;

  switch(componentID)
  {
  case ALF_Cb:
  case ALF_Cr:
    {
      srcE = src.ECorr  [0];
      dstE = this->ECorr[0];

      srcy  = src.yCorr[0];
      dsty  = this->yCorr[0];

      for(Int j=0; j< numCoef; j++)
      {
        for(Int i=0; i< numCoef; i++)
        {
          dstE[j][i] += srcE[j][i];
        }

        dsty[j] += srcy[j];
      }
      if(doPixAccMerge)
      {
        this->pixAcc[0] = src.pixAcc[0];
      }
    }
    break;
  case ALF_Y:
    {
      Int maxFilterSetSize = (Int)NO_VAR_BINS;
      for (Int varInd=0; varInd< maxFilterSetSize; varInd++)
      {
        Int filtIdx = (mergeTable == NULL)?(0):(mergeTable[varInd]);
        srcE = src.ECorr  [varInd];
        dstE = this->ECorr[ filtIdx ];
        srcy  = src.yCorr[varInd];
        dsty  = this->yCorr[ filtIdx ];
        for(Int j=0; j< numCoef; j++)
        {
          for(Int i=0; i< numCoef; i++)
          {
            dstE[j][i] += srcE[j][i];
          }
          dsty[j] += srcy[j];
        }
        if(doPixAccMerge)
        {
          this->pixAcc[filtIdx] += src.pixAcc[varInd];
        }
      }
    }
    break;
  default:
    {
      printf("not a legal component ID\n");
      assert(0);
      exit(-1);
    }
  }
}
#if !AHG6_ALF_OPTION2
///AlfPicQTPart
AlfPicQTPart::AlfPicQTPart()
{
  componentID = -1;
  alfUnitParam = NULL; 
  alfCorr = NULL;
}

AlfPicQTPart::~AlfPicQTPart()
{
  if(alfUnitParam != NULL)
  {
    if(alfUnitParam->alfFiltParam != NULL)
    {
      delete alfUnitParam->alfFiltParam;
      alfUnitParam->alfFiltParam = NULL;
    }
    delete alfUnitParam;
    alfUnitParam = NULL;
  }
  if(alfCorr != NULL)
  {
    delete alfCorr;
    alfCorr = NULL;
  }
}

AlfPicQTPart& AlfPicQTPart::operator= (const AlfPicQTPart& src)
{
  componentID = src.componentID;
  partCUXS    = src.partCUXS;
  partCUYS    = src.partCUYS;
  partCUXE    = src.partCUXE;
  partCUYE    = src.partCUYE;
  partIdx     = src.partIdx;
  partLevel   = src.partLevel;
  partCol     = src.partCol;
  partRow     = src.partRow;
  for(Int i=0; i<4; i++)
  {
    childPartIdx[i] = src.childPartIdx[i];
  }
  parentPartIdx = src.parentPartIdx;

  isBottomLevel = src.isBottomLevel;
  isSplit       = src.isSplit;

  isProcessed   = src.isProcessed;
  splitMinCost  = src.splitMinCost;
  splitMinDist  = src.splitMinDist;
  splitMinRate  = src.splitMinRate;
  selfMinCost   = src.selfMinCost;
  selfMinDist   = src.selfMinDist;
  selfMinRate   = src.selfMinRate;

  numFilterBudget = src.numFilterBudget;

  if(src.alfUnitParam != NULL)
  {
    if(alfUnitParam == NULL)
    {
      //create alfUnitparam
      alfUnitParam = new AlfUnitParam;
      alfUnitParam->alfFiltParam = new ALFParam(componentID);
    }
    //assign from src
    alfUnitParam->mergeType = src.alfUnitParam->mergeType;
    alfUnitParam->isEnabled = src.alfUnitParam->isEnabled;
    alfUnitParam->isNewFilt = src.alfUnitParam->isNewFilt;
    alfUnitParam->storedFiltIdx = src.alfUnitParam->storedFiltIdx;
    *(alfUnitParam->alfFiltParam) = *(src.alfUnitParam->alfFiltParam);   
  }
  else
  {
    printf("source quad-tree partition info is not complete\n");
    assert(0);
    exit(-1);
  }

  if(src.alfCorr != NULL)
  {
    if(alfCorr == NULL)
    {
      alfCorr = new AlfCorrData(componentID);
    }
    alfCorr->reset();
    (*alfCorr) += (*(src.alfCorr));
  }
  else
  {
    printf("source quad-tree partition info is not complete\n");
    assert(0);
    exit(-1);
  }
  return *this;
}
#endif

TEncAdaptiveLoopFilter::TEncAdaptiveLoopFilter()
{
#if !AHG6_ALF_OPTION2
  m_pcEntropyCoder = NULL;
  m_pcPicYuvBest = NULL;
  m_pcPicYuvTmp = NULL;

  m_iALFMaxNumberFilters = NO_FILTERS;

  m_bAlfCUCtrlEnabled = false;
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================
#if !AHG6_ALF_OPTION2
/** convert Level Row Col to Idx
 * \param   level,  row,  col
 */
Int TEncAdaptiveLoopFilter::convertLevelRowCol2Idx(Int level, Int row, Int col)
{
  Int idx;
  if (level == 0)
  {
    idx = 0;
  }
  else if (level == 1)
  {
    idx = 1 + row*2 + col;
  }
  else if (level == 2)
  {
    idx = 5 + row*4 + col;
  }
  else if (level == 3)
  {
    idx = 21 + row*8 + col;
  }
  else // (level == 4)
  {
    idx = 85 + row*16 + col;
  }
  return idx;
}

/** convert quadtree Idx to Level, Row, and Col
 * \param  idx,  *level,  *row,  *col
 */
Void TEncAdaptiveLoopFilter::convertIdx2LevelRowCol(Int idx, Int *level, Int *row, Int *col)
{
  if (idx == 0)
  {
    *level = 0;
    *row = 0;
    *col = 0;
  }
  else if (idx>=1 && idx<=4)
  {
    *level = 1;
    *row = (idx-1) / 2;
    *col = (idx-1) % 2;
  }
  else if (idx>=5 && idx<=20)
  {
    *level = 2;
    *row = (idx-5) / 4;
    *col = (idx-5) % 4;
  }
  else if (idx>=21 && idx<=84)
  {
    *level = 3;
    *row = (idx-21) / 8;
    *col = (idx-21) % 8;
  }
  else // (idx>=85 && idx<=340)
  {
    *level = 4;
    *row = (idx-85) / 16;
    *col = (idx-85) % 16;
  }
}

/** Initial picture quad-tree
 * \param [in] isPicBasedEncode picture quad-tree encoding is enabled or disabled
 */
Void TEncAdaptiveLoopFilter::initPicQuadTreePartition(Bool isPicBasedEncode)
{
  if (!isPicBasedEncode)
  {
    return;
  }
  
  Int maxDepthInWidth   = (Int)(logf((float)(m_numLCUInPicWidth     ))/logf(2.0));
  Int maxDepthInHeight  = (Int)(logf((float)(m_numLCUInPicHeight    ))/logf(2.0));
  Int maxDepthInFilters = (Int)(logf((float)(m_iALFMaxNumberFilters ))/logf(2.0));
  m_alfPQTMaxDepth = (maxDepthInWidth  > maxDepthInHeight ) ? maxDepthInHeight  : maxDepthInWidth ;
  m_alfPQTMaxDepth = (m_alfPQTMaxDepth > maxDepthInFilters) ? maxDepthInFilters : m_alfPQTMaxDepth ;

  for (Int compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    m_alfPQTPart[compIdx] = new AlfPicQTPart [ m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth] ];
    for (Int i = 0; i < m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth]; i++ )
    {
      m_alfPQTPart[compIdx][i].alfCorr = new AlfCorrData(compIdx);
      m_alfPQTPart[compIdx][i].alfUnitParam = new AlfUnitParam;
      m_alfPQTPart[compIdx][i].alfUnitParam->alfFiltParam = new ALFParam(compIdx);
    }

  }
  creatPQTPart(0, 0, 0, -1, 0, m_numLCUInPicWidth-1, 0, m_numLCUInPicHeight-1); 
}

/** Reset picture quad-tree variables
 */
Void TEncAdaptiveLoopFilter::resetPQTPart()
{
  Int compIdx, i;

  for (compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    for (i = 0; i < m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth]; i++ )
    {
      m_alfPQTPart[compIdx][i].isProcessed  = false;
      m_alfPQTPart[compIdx][i].selfMinCost  = MAX_DOUBLE;
      m_alfPQTPart[compIdx][i].splitMinCost = MAX_DOUBLE;      
      //reset correlations
      m_alfPQTPart[compIdx][i].alfCorr->reset();
      //reset ALF unit param
      m_alfPQTPart[compIdx][i].alfUnitParam->mergeType = ALF_MERGE_DISABLED;
      m_alfPQTPart[compIdx][i].alfUnitParam->isEnabled = false;
      m_alfPQTPart[compIdx][i].alfUnitParam->alfFiltParam->alf_flag = 0;
    }
  }
}

/** create picture quad-tree
 * \param [in] partLevel quad-tree level
 * \param [in] partRow row position at partLevel
 * \param [in] partCol column position at partLevel
 * \param [in] parentPartIdx parent partition index
 * \param [in] partCUXS starting LCU X position
 * \param [in] partCUXE ending LCU X position
 * \param [in] partCUYS starting LCU Y position
 * \param [in] partCUYE ending LCU Y position
 */
Void TEncAdaptiveLoopFilter::creatPQTPart(Int partLevel, Int partRow, Int partCol, Int parentPartIdx, Int partCUXS, Int partCUXE, Int partCUYS, Int partCUYE)
{
  Int partIdx = convertLevelRowCol2Idx(partLevel, partRow, partCol);

  AlfPicQTPart *alfOnePartY, *alfOnePartCb, *alfOnePartCr;

  alfOnePartY  = &(m_alfPQTPart[ALF_Y ][partIdx]);
  alfOnePartCb = &(m_alfPQTPart[ALF_Cb][partIdx]);
  alfOnePartCr = &(m_alfPQTPart[ALF_Cr][partIdx]);

  // Y, Cb, Cr
  alfOnePartY->partIdx   = alfOnePartCb->partIdx   = alfOnePartCr->partIdx   = partIdx;
  alfOnePartY->partCol   = alfOnePartCb->partCol   = alfOnePartCr->partCol   = partCol;
  alfOnePartY->partRow   = alfOnePartCb->partRow   = alfOnePartCr->partRow   = partRow;
  alfOnePartY->partLevel = alfOnePartCb->partLevel = alfOnePartCr->partLevel = partLevel;

  alfOnePartY->partCUXS  = alfOnePartCb->partCUXS  = alfOnePartCr->partCUXS  = partCUXS;  
  alfOnePartY->partCUXE  = alfOnePartCb->partCUXE  = alfOnePartCr->partCUXE  = partCUXE;
  alfOnePartY->partCUYS  = alfOnePartCb->partCUYS  = alfOnePartCr->partCUYS  = partCUYS;
  alfOnePartY->partCUYE  = alfOnePartCb->partCUYE  = alfOnePartCr->partCUYE  = partCUYE;

  alfOnePartY->parentPartIdx = alfOnePartCb->parentPartIdx = alfOnePartCr->parentPartIdx = parentPartIdx;  
  alfOnePartY->isSplit       = alfOnePartCb->isSplit       = alfOnePartCr->isSplit       = false;

#if LCUALF_FILTER_BUDGET_CONTROL_ENC
  alfOnePartY->numFilterBudget = alfOnePartCb->numFilterBudget = alfOnePartCr->numFilterBudget = m_iALFMaxNumberFilters/m_alfNumPartsLevelTab[partLevel];
#else
  alfOnePartY->numFilterBudget = alfOnePartCb->numFilterBudget = alfOnePartCr->numFilterBudget = NO_VAR_BINS;
#endif

  alfOnePartY->componentID  = ALF_Y;
  alfOnePartCb->componentID = ALF_Cb;
  alfOnePartCr->componentID = ALF_Cr;

  if (alfOnePartY->partLevel != m_alfPQTMaxDepth)
  {
    alfOnePartY->isBottomLevel = alfOnePartCb->isBottomLevel = alfOnePartCr->isBottomLevel = false;

    Int downLevel    = partLevel + 1;
    Int downRowStart = partRow << 1;
    Int downColStart = partCol << 1;

    Int downRowIdx, downColIdx;
    Int numCULeft, numCUTop;
    Int downStartCUX, downStartCUY, downEndCUX, downEndCUY;

    numCULeft = (partCUXE - partCUXS + 1) >> 1 ;
    numCUTop  = (partCUYE - partCUYS + 1) >> 1 ;

    // ChildPart00
    downStartCUX = partCUXS;
    downEndCUX   = downStartCUX + numCULeft - 1;
    downStartCUY = partCUYS;
    downEndCUY   = downStartCUY + numCUTop  - 1;
    downRowIdx   = downRowStart + 0;
    downColIdx   = downColStart + 0;

    alfOnePartY->childPartIdx[0] = alfOnePartCb->childPartIdx[0] = alfOnePartCr->childPartIdx[0] = convertLevelRowCol2Idx(downLevel, downRowIdx, downColIdx);
    creatPQTPart(downLevel, downRowIdx, downColIdx, partIdx, downStartCUX, downEndCUX, downStartCUY, downEndCUY);

    // ChildPart01
    downStartCUX = partCUXS + numCULeft;
    downEndCUX   = partCUXE;
    downStartCUY = partCUYS;
    downEndCUY   = downStartCUY + numCUTop  - 1;
    downRowIdx   = downRowStart + 0;
    downColIdx   = downColStart + 1;

    alfOnePartY->childPartIdx[1] = alfOnePartCb->childPartIdx[1] = alfOnePartCr->childPartIdx[1] = convertLevelRowCol2Idx(downLevel, downRowIdx, downColIdx);
    creatPQTPart(downLevel, downRowIdx, downColIdx, partIdx, downStartCUX, downEndCUX, downStartCUY, downEndCUY);

    // ChildPart10
    downStartCUX = partCUXS;
    downEndCUX   = downStartCUX + numCULeft - 1;
    downStartCUY = partCUYS + numCUTop;
    downEndCUY   = partCUYE;
    downRowIdx   = downRowStart + 1;
    downColIdx   = downColStart + 0;

    alfOnePartY->childPartIdx[2] = alfOnePartCb->childPartIdx[2] = alfOnePartCr->childPartIdx[2] = convertLevelRowCol2Idx(downLevel, downRowIdx, downColIdx);
    creatPQTPart(downLevel, downRowIdx, downColIdx, partIdx, downStartCUX, downEndCUX, downStartCUY, downEndCUY);

    // ChildPart11
    downStartCUX = partCUXS + numCULeft;
    downEndCUX   = partCUXE;
    downStartCUY = partCUYS + numCUTop;
    downEndCUY   = partCUYE;
    downRowIdx   = downRowStart + 1;
    downColIdx   = downColStart + 1;

    alfOnePartY->childPartIdx[3] = alfOnePartCb->childPartIdx[3] = alfOnePartCr->childPartIdx[3] = convertLevelRowCol2Idx(downLevel, downRowIdx, downColIdx);
    creatPQTPart(downLevel, downRowIdx, downColIdx, partIdx, downStartCUX, downEndCUX, downStartCUY, downEndCUY);
  }
  else
  {
    alfOnePartY->isBottomLevel = alfOnePartCb->isBottomLevel = alfOnePartCr->isBottomLevel = true;

    alfOnePartY->childPartIdx[0] = alfOnePartCb->childPartIdx[0] = alfOnePartCr->childPartIdx[0] = -1;
    alfOnePartY->childPartIdx[1] = alfOnePartCb->childPartIdx[1] = alfOnePartCr->childPartIdx[1] = -1;
    alfOnePartY->childPartIdx[2] = alfOnePartCb->childPartIdx[2] = alfOnePartCr->childPartIdx[2] = -1;
    alfOnePartY->childPartIdx[3] = alfOnePartCb->childPartIdx[3] = alfOnePartCr->childPartIdx[3] = -1;
  }
}
#endif
/** create global buffers for ALF encoding
 */
Void TEncAdaptiveLoopFilter::createAlfGlobalBuffers()
{
  for(Int compIdx =0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
#if AHG6_ALF_OPTION2
    m_alfNonSkippedCorr[compIdx] = new AlfCorrData*[m_uiNumCUsInFrame];
#else
    m_alfPicFiltUnits[compIdx] = new AlfUnitParam[m_uiNumCUsInFrame];
#endif
    m_alfCorr[compIdx] = new AlfCorrData*[m_uiNumCUsInFrame];
    for(Int n=0; n< m_uiNumCUsInFrame; n++)
    {
      m_alfCorr[compIdx][n]= new AlfCorrData(compIdx);
      m_alfCorr[compIdx][n]->reset();
#if AHG6_ALF_OPTION2
      m_alfNonSkippedCorr[compIdx][n] = new AlfCorrData(compIdx);
      m_alfNonSkippedCorr[compIdx][n]->reset();
#endif
    }

    m_alfCorrMerged[compIdx] = new AlfCorrData(compIdx);

  }

#if AHG6_ALF_OPTION2
  const Int numCoef = (Int)ALF_MAX_NUM_COEF;

  // temporary buffer for filter merge
  for(Int g=0; g< (Int)NO_VAR_BINS; g++)
  {
    m_y_merged[g] = new Double[numCoef];

    m_E_merged[g] = new Double*[numCoef];
    for(Int i=0; i< numCoef; i++)
    {
      m_E_merged[g][i]= new Double[numCoef];
    }
  }
  m_E_temp = new Double*[numCoef];
  for(Int i=0; i< numCoef; i++)
  {
    m_E_temp[i] = new Double[numCoef];
  }

  //alf params for temporal layers
  Int maxNumTemporalLayer =   (Int)(logf((float)(m_gopSize))/logf(2.0) + 1);
  m_alfPictureParam = new ALFParam**[ maxNumTemporalLayer ];
  for(Int t=0; t< maxNumTemporalLayer; t++)
  {
    m_alfPictureParam[t] = new ALFParam*[NUM_ALF_COMPONENT];
    for(Int compIdx=0; compIdx < NUM_ALF_COMPONENT; compIdx++)
    {
      m_alfPictureParam[t][compIdx] = new ALFParam(compIdx);
    }
  }

  // identity filter to estimate ALF off distortion
  for(Int i=0; i< (Int)NO_VAR_BINS; i++)
  {
    m_coeffNoFilter[i] = new Int[numCoef];
    ::memset(&(m_coeffNoFilter[i][0]), 0, sizeof(Int)*numCoef);
    m_coeffNoFilter[i][numCoef-1] = (1 << ALF_NUM_BIT_SHIFT);
  }
#else
  const Int numCoef = (Int)ALF_MAX_NUM_COEF;

  for(Int i=0; i< (Int)NO_VAR_BINS; i++)
  {
    m_coeffNoFilter[i] = new Int[numCoef];
  }
#endif
  m_numSlicesDataInOneLCU = new Int[m_uiNumCUsInFrame];

}

/** destroy ALF global buffers
 * This function is used to destroy the global ALF encoder buffers
 */
Void TEncAdaptiveLoopFilter::destroyAlfGlobalBuffers()
{
  for(Int compIdx =0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
#if !AHG6_ALF_OPTION2
    delete[] m_alfPicFiltUnits[compIdx];
#endif
    for(Int n=0; n< m_uiNumCUsInFrame; n++)
    {
      delete m_alfCorr[compIdx][n];
#if AHG6_ALF_OPTION2
      delete m_alfNonSkippedCorr[compIdx][n];
#endif
    }

    delete[] m_alfCorr[compIdx];

    m_alfCorr[compIdx] = NULL;
#if AHG6_ALF_OPTION2
    delete[] m_alfNonSkippedCorr[compIdx];
    m_alfNonSkippedCorr[compIdx] = NULL;
#endif
    delete m_alfCorrMerged[compIdx];
  }
#if AHG6_ALF_OPTION2
  const Int numCoef = (Int)ALF_MAX_NUM_COEF;

  // temporary buffer for filter merge
  for(Int g=0; g< (Int)NO_VAR_BINS; g++)
  {
    delete[] m_y_merged[g]; m_y_merged[g] = NULL;
    for(Int i=0; i< numCoef; i++)
    {
      delete[] m_E_merged[g][i];
    }
    delete[] m_E_merged[g];            m_E_merged[g] = NULL;
  }
  for(Int i=0; i< numCoef; i++)
  {
    delete[] m_E_temp[i];
  }
  delete[] m_E_temp; m_E_temp = NULL;

  Int maxNumTemporalLayer =   (Int)(logf((float)(m_gopSize))/logf(2.0) + 1);
  for(Int t=0; t< maxNumTemporalLayer; t++)
  {
    for(Int compIdx=0; compIdx < NUM_ALF_COMPONENT; compIdx++)
    {
      delete m_alfPictureParam[t][compIdx];
    }
    delete[] m_alfPictureParam[t];
  }
  delete[] m_alfPictureParam; m_alfPictureParam = NULL;
#endif
  //const Int numCoef = (Int)ALF_MAX_NUM_COEF;

  for(Int i=0; i< (Int)NO_VAR_BINS; i++)
  {
    delete[] m_coeffNoFilter[i];
  }

  delete[] m_numSlicesDataInOneLCU;

}
#if !AHG6_ALF_OPTION2
/** initialize ALF encoder at picture level
 * \param [in] isAlfParamInSlice ALF parameters are coded in slice (true) or APS (false)
 * \param [in] isPicBasedEncode picture-based encoding (true) or LCU-based encoding (false)
 * \param [in] numSlices number of slices in current picture
 * \param [in, out] alfParams ALF parameter set
 * \param [in, out] alfCUCtrlParam ALF CU-on/off control parameters
 */
Void TEncAdaptiveLoopFilter::initALFEnc(Bool isAlfParamInSlice, Bool isPicBasedEncode, Int numSlices, AlfParamSet* & alfParams, std::vector<AlfCUCtrlInfo>* & alfCUCtrlParam)
{
  m_picBasedALFEncode = isPicBasedEncode;

  if(isAlfParamInSlice) 
  {
    alfParams = new AlfParamSet[m_uiNumSlicesInPic];
    Int numLCUs = m_uiNumCUsInFrame;

    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      numLCUs = (Int)(m_pcPic->getOneSliceCUDataForNDBFilter(s).size());
      alfParams[s].create(m_numLCUInPicWidth,m_numLCUInPicHeight, numLCUs );
      alfParams[s].createALFParam();
    }
    alfCUCtrlParam = NULL; 
  }
  else //ALF parameter in APS
  {
    alfParams = NULL; //ALF parameters are handled by APS
    alfCUCtrlParam = new std::vector<AlfCUCtrlInfo>;
    alfCUCtrlParam->resize(numSlices);
  }

  resetPicAlfUnit();

  if(m_picBasedALFEncode)
  {
    resetPQTPart();  
  }

  const Int numCoef = (Int)ALF_MAX_NUM_COEF;
#if LCUALF_QP_DEPENDENT_BITS
  Int numBitShift = getAlfPrecisionBit( m_alfQP ); 
#else
  Int numBitShift = (Int)ALF_NUM_BIT_SHIFT;
#endif
  for(Int i=0; i< (Int)NO_VAR_BINS; i++)
  {
    ::memset(&(m_coeffNoFilter[i][0]), 0, sizeof(Int)*numCoef);
    m_coeffNoFilter[i][numCoef-1] = (1 << numBitShift);
  }

}

/** Uninitialize ALF encoder at picture level
 * \param [in, out] alfParams ALF parameter set
 * \param [in, out] alfCUCtrlParam ALF CU-on/off control parameters
 */
Void TEncAdaptiveLoopFilter::uninitALFEnc(AlfParamSet* & alfParams, std::vector<AlfCUCtrlInfo>* & alfCUCtrlParam)
{
  if(alfParams != NULL)
  {
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      alfParams[s].releaseALFParam();
    }
    delete[] alfParams;
    alfParams = NULL;
  }

  if(alfCUCtrlParam != NULL)
  {
    delete alfCUCtrlParam;
    alfCUCtrlParam = NULL;
  }
}

/** reset ALF unit parameters in current picture
 */
Void TEncAdaptiveLoopFilter::resetPicAlfUnit()
{
  for(Int compIdx =0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    for(Int i=0; i< m_uiNumCUsInFrame; i++)
    {
      AlfUnitParam& alfUnit = m_alfPicFiltUnits[compIdx][i];
      alfUnit.mergeType = ALF_MERGE_DISABLED;
      alfUnit.isEnabled = false;
      alfUnit.isNewFilt = true;
      alfUnit.alfFiltParam = m_alfFiltInfo[compIdx][i];

      alfUnit.alfFiltParam->alf_flag = 0;
    }
  }
}

/**
 \param pcPic           picture (TComPic) pointer
 \param pcEntropyCoder  entropy coder class
 */
Void TEncAdaptiveLoopFilter::startALFEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder )
{
  m_pcEntropyCoder = pcEntropyCoder;
  xCreateTmpAlfCtrlFlags();
  
  Int iWidth = pcPic->getPicYuvOrg()->getWidth();
  Int iHeight = pcPic->getPicYuvOrg()->getHeight();
  
  m_pcPicYuvTmp = new TComPicYuv();
  m_pcPicYuvTmp->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  m_pcPicYuvBest = pcPic->getPicYuvPred();
  initMatrix_int(&m_filterCoeffSymQuant, NO_VAR_BINS, ALF_MAX_NUM_COEF); 
  initMatrix_Pel(&m_maskImg, m_img_height, m_img_width);
  initMatrix_double(&m_E_temp, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  m_y_temp = (double *) calloc(MAX_SQR_FILT_LENGTH, sizeof(double));//
  initMatrix3D_double(&m_E_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  initMatrix_double(&m_y_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); //
  m_pixAcc_merged = (double *) calloc(NO_VAR_BINS, sizeof(double));//
  m_filterCoeffQuantMod = (int *) calloc(ALF_MAX_NUM_COEF, sizeof(int));//
  m_filterCoeff = (double *) calloc(ALF_MAX_NUM_COEF, sizeof(double));//
  m_filterCoeffQuant = (int *) calloc(ALF_MAX_NUM_COEF, sizeof(int));//
  initMatrix_int(&m_diffFilterCoeffQuant, NO_VAR_BINS, ALF_MAX_NUM_COEF);//
  initMatrix_int(&m_FilterCoeffQuantTemp, NO_VAR_BINS, ALF_MAX_NUM_COEF);//

  m_tempALFp = new ALFParam(ALF_Y);
}

Void TEncAdaptiveLoopFilter::endALFEnc()
{
  xDestroyTmpAlfCtrlFlags();
  
  m_pcPicYuvTmp->destroyLuma();
  delete m_pcPicYuvTmp;
  m_pcPicYuvTmp = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  destroyMatrix_int(m_filterCoeffSymQuant);
  destroyMatrix_Pel(m_maskImg);
  destroyMatrix3D_double(m_E_merged, NO_VAR_BINS);
  destroyMatrix_double(m_y_merged);
  destroyMatrix_double(m_E_temp);
  free(m_pixAcc_merged);
  
  free(m_filterCoeffQuantMod);
  free(m_y_temp);
  
  free(m_filterCoeff);
  free(m_filterCoeffQuant);
  destroyMatrix_int(m_diffFilterCoeffQuant);
  destroyMatrix_int(m_FilterCoeffQuantTemp);
  
  delete m_tempALFp;
}

/** Assign output ALF parameters
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in, out] alfCtrlParam ALF CU-on/off control parameters
 */
Void TEncAdaptiveLoopFilter::assignALFEncoderParam(AlfParamSet* alfParamSet, std::vector<AlfCUCtrlInfo>* alfCtrlParam)
{
  //assign CU control parameters
  if(m_bAlfCUCtrlEnabled)
  {
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      (*alfCtrlParam)[s]= m_vBestAlfCUCtrlParam[s];
    }
  }

  //assign RDO results to alfParamSet
  if(m_alfCoefInSlice)
  {
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }

      if( m_bestAlfParamSet[s].isEnabled[ALF_Y] || m_bestAlfParamSet[s].isEnabled[ALF_Cb] || m_bestAlfParamSet[s].isEnabled[ALF_Cr])
      {
        m_bestAlfParamSet[s].isEnabled[ALF_Y] = true;
      }

      copyAlfParamSet(&(alfParamSet[s]), &(m_bestAlfParamSet[s]));
    }
  }
  else
  {
    if( m_bestAlfParamSet->isEnabled[ALF_Y] || m_bestAlfParamSet->isEnabled[ALF_Cb] || m_bestAlfParamSet->isEnabled[ALF_Cr])
    {
      m_bestAlfParamSet->isEnabled[ALF_Y] = true;
    }

    copyAlfParamSet(alfParamSet, m_bestAlfParamSet);
  }

  if(m_alfCoefInSlice)
  {
    delete[] m_bestAlfParamSet;
  }
  else
  {
    delete m_bestAlfParamSet;
  }
}
#endif
/** initialize ALF encoder configurations
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in, out] alfCtrlParam ALF CU-on/off control parameters
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::initALFEncoderParam()
#else
Void TEncAdaptiveLoopFilter::initALFEncoderParam(AlfParamSet* alfParamSet, std::vector<AlfCUCtrlInfo>* alfCtrlParam)
#endif
{
#if !AHG6_ALF_OPTION2
  //reset BA index map 
  memset(&m_varImg[0][0], 0, sizeof(Pel)*(m_img_height*m_img_width));
  //reset mask
  for(Int y=0; y< m_img_height; y++)
  {
    for(Int x=0; x< m_img_width; x++)
    {
      m_maskImg[y][x] = 1;
    }
  }
#endif
  //get last valid slice index
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(m_pcPic->getValidSlice(s))
    {
      m_lastSliceIdx = s;
    }
  }
#if !AHG6_ALF_OPTION2
  //reset alf CU control flags
  m_bAlfCUCtrlEnabled = (alfCtrlParam != NULL)?true:false;
  if(m_bAlfCUCtrlEnabled)
  {
    m_vBestAlfCUCtrlParam.resize(m_uiNumSlicesInPic);
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      m_vBestAlfCUCtrlParam[s].reset();
    }
  }
  else
  {
    m_vBestAlfCUCtrlParam.clear();
  }
#endif
  //get number slices in each LCU
  if(m_uiNumSlicesInPic == 1 || m_iSGDepth == 0)
  {
    for(Int n=0; n< m_uiNumCUsInFrame; n++)
    {
      m_numSlicesDataInOneLCU[n] = 1;
    }
  }
  else
  {
    Int count;
    Int prevSliceID = -1;

    for(Int n=0; n< m_uiNumCUsInFrame; n++)
    {
      std::vector<NDBFBlockInfo>& vNDBFBlock = *(m_pcPic->getCU(n)->getNDBFilterBlocks());

      count = 0;

      for(Int i=0; i< (Int)vNDBFBlock.size(); i++)
      {
        if(vNDBFBlock[i].sliceID != prevSliceID)
        {
          prevSliceID = vNDBFBlock[i].sliceID;
          count++;
        }
      }

      m_numSlicesDataInOneLCU[n] = count;
    }
  }
#if !AHG6_ALF_OPTION2
  //set redesign number
  if(m_iALFEncodePassReduction)
  {
    m_iALFNumOfRedesign = 0;
  }
  else
  {
    m_iALFNumOfRedesign = ALF_NUM_OF_REDESIGN;
  }

  //initialize m_bestAlfParamSet
  if(m_alfCoefInSlice)
  {
    m_bestAlfParamSet = new AlfParamSet[m_uiNumSlicesInPic];
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      m_bestAlfParamSet[s].create( alfParamSet[s].numLCUInWidth, alfParamSet[s].numLCUInHeight, alfParamSet[s].numLCU);
    }
  }
  else
  {
    m_bestAlfParamSet = new AlfParamSet;
    m_bestAlfParamSet->create( alfParamSet->numLCUInWidth, alfParamSet->numLCUInHeight, alfParamSet->numLCU);
  }
#endif
}

#if !AHG6_ALF_OPTION2
/** copy ALF parameter set
 * \param [out] dst destination ALF parameter set
 * \param [in] src source ALF parameter set
 */
Void TEncAdaptiveLoopFilter::copyAlfParamSet(AlfParamSet* dst, AlfParamSet* src)
{
  dst->numLCU = src->numLCU;
  dst->numLCUInWidth = src->numLCUInWidth;
  dst->numLCUInHeight = src->numLCUInHeight;

  for(Int compIdx =0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    dst->isEnabled[compIdx] = src->isEnabled[compIdx];
    dst->isUniParam[compIdx] = src->isUniParam[compIdx];

    for(Int n=0; n< src->numLCU; n++)
    {
      dst->alfUnitParam[compIdx][n].isEnabled = src->alfUnitParam[compIdx][n].isEnabled;
      dst->alfUnitParam[compIdx][n].isNewFilt = src->alfUnitParam[compIdx][n].isNewFilt;
      dst->alfUnitParam[compIdx][n].mergeType = src->alfUnitParam[compIdx][n].mergeType;
      dst->alfUnitParam[compIdx][n].storedFiltIdx = src->alfUnitParam[compIdx][n].storedFiltIdx;
      *(dst->alfUnitParam[compIdx][n].alfFiltParam) = *(src->alfUnitParam[compIdx][n].alfFiltParam);
    }
  }
}
#endif

/** ALF encoding process top function
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in, out] alfCtrlParam ALF CU-on/off control parameters
 * \param [in] dLambdaLuma lambda value for luma RDO
 * \param [in] dLambdaChroma lambda value for chroma RDO
 */
#if AHG6_ALF_OPTION2
#if ALF_CHROMA_LAMBDA
Void TEncAdaptiveLoopFilter::ALFProcess(ALFParam** alfPictureParam, Double lambdaLuma, Double lambdaChroma)
#else
Void TEncAdaptiveLoopFilter::ALFProcess(ALFParam** alfPictureParam,  Double lambda)
#endif
#else
#if ALF_CHROMA_LAMBDA
Void TEncAdaptiveLoopFilter::ALFProcess( AlfParamSet* alfParamSet, std::vector<AlfCUCtrlInfo>* alfCtrlParam, Double lambdaLuma, Double lambdaChroma)
#else
Void TEncAdaptiveLoopFilter::ALFProcess( AlfParamSet* alfParamSet, std::vector<AlfCUCtrlInfo>* alfCtrlParam, Double lambda)
#endif
#endif
{
#if ALF_CHROMA_LAMBDA
  m_dLambdaLuma   = lambdaLuma;
  m_dLambdaChroma = lambdaChroma;
#else
  m_dLambdaLuma   = lambda;
  m_dLambdaChroma = lambda;
#endif
  TComPicYuv* yuvOrg    = m_pcPic->getPicYuvOrg();
  TComPicYuv* yuvRec    = m_pcPic->getPicYuvRec();
  TComPicYuv* yuvExtRec = m_pcTempPicYuv;

  //picture boundary padding
  yuvRec->copyToPic(yuvExtRec);
  yuvExtRec->setBorderExtension( false );
  yuvExtRec->extendPicBorder   ();

  //initialize encoder parameters
#if AHG6_ALF_OPTION2
  initALFEncoderParam();
#else
  initALFEncoderParam(alfParamSet, alfCtrlParam);
#endif
  //get LCU statistics
  getStatistics(yuvOrg, yuvExtRec);

  //decide ALF parameters
#if AHG6_ALF_OPTION2
  decideParameters(alfPictureParam, yuvOrg, yuvExtRec, yuvRec);

  if(m_alfLowLatencyEncoding)
  {
    //derive time-delayed filter for next picture
    decideAlfPictureParam(m_alfPictureParam[getTemporalLayerNo(m_pcPic->getPOC(), m_gopSize)], false);
  }
#else
  decideParameters(yuvOrg, yuvExtRec, yuvRec, m_bestAlfParamSet, alfCtrlParam);

  //assign best parameters
  assignALFEncoderParam(alfParamSet, alfCtrlParam);
#endif
}

#if !AHG6_ALF_OPTION2
/** Check if the current LCU can be merged with neighboring LCU
 * \param [in] compIdx luma/chroma component index
 * \param [out] alfUnitPic ALF unit parameters for all LCUs in picture
 */
Void TEncAdaptiveLoopFilter::checkMerge(Int compIdx, AlfUnitParam* alfUnitPic)
{
  AlfUnitParam *alfUnitLeft, *alfUnitUp;

  for(Int n=0; n< m_uiNumCUsInFrame; n++)
  {
    Int lcuPosX = (Int)(n % m_numLCUInPicWidth);
    Int lcuPosY = (Int)(n / m_numLCUInPicWidth);

    AlfUnitParam& alfUnitCur = alfUnitPic[n];

    //check merge left
    if( lcuPosX != 0)
    {
      alfUnitLeft = &(alfUnitPic[n - 1]);
      if(alfUnitCur == *alfUnitLeft)
      {
        alfUnitCur.mergeType = ALF_MERGE_LEFT;
        alfUnitCur.isEnabled = alfUnitLeft->isEnabled;
        alfUnitCur.isNewFilt = alfUnitLeft->isNewFilt;
        alfUnitCur.storedFiltIdx = alfUnitLeft->storedFiltIdx;
        *(alfUnitCur.alfFiltParam) = *(alfUnitLeft->alfFiltParam);
        continue;
      }
    }

    //check merge up
    if(lcuPosY !=0 )
    {
      alfUnitUp = &(alfUnitPic[n - m_numLCUInPicWidth]);
      if(alfUnitCur == *alfUnitUp)
      {
        alfUnitCur.mergeType = ALF_MERGE_UP;
        alfUnitCur.isEnabled = alfUnitUp->isEnabled;
        alfUnitCur.isNewFilt = alfUnitUp->isNewFilt;
        alfUnitCur.storedFiltIdx = alfUnitUp->storedFiltIdx;
        *(alfUnitCur.alfFiltParam) = *(alfUnitUp->alfFiltParam);
        continue;
      }
    }
  }

}

/** Transfer ALF unit parameters for LCUs to to-be-coded ALF parameter set
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfUnitPic ALF unit parameters for all LCUs in picture
 * \param [out] alfParamSet to-be-coded ALF parameter set
 */
Void TEncAdaptiveLoopFilter::transferToAlfParamSet(Int compIdx, AlfUnitParam* alfUnitPic, AlfParamSet* & alfParamSet)
{

  Int countFiltOffLCU = 0, countNewFilts = 0;

  AlfUnitParam* alfUnitParams = alfParamSet->alfUnitParam[compIdx];
  for(Int n=0; n< m_uiNumCUsInFrame; n++)
  {
    alfUnitParams[n] = alfUnitPic[n];


    if(alfUnitParams[n].alfFiltParam->alf_flag == 0)
    {
      countFiltOffLCU++;
    }
    else
    {
      Bool isNewFiltInSlice =   (alfUnitParams[n].mergeType == ALF_MERGE_DISABLED && alfUnitParams[n].isEnabled && alfUnitParams[n].isNewFilt);
      if( isNewFiltInSlice )
      {
        countNewFilts++;
      }
    }
  }

  //slice-level parameters
  AlfUnitParam* firstAlfUnitInSlice = &(alfUnitParams[0]);
  if( countFiltOffLCU == m_uiNumCUsInFrame ) //number of filter-off LCU is equal to the number of LCUs in slice
  {
    alfParamSet->isEnabled [compIdx] = false;    
    alfParamSet->isUniParam[compIdx] = true; //uni-param, all off
    assert(firstAlfUnitInSlice->alfFiltParam->alf_flag == 0);
  }
  else
  {
    alfParamSet->isEnabled[compIdx] = true;
    if( countNewFilts == 1 && firstAlfUnitInSlice->alfFiltParam->alf_flag != 0 && countFiltOffLCU == 0)
    {
      alfParamSet->isUniParam[compIdx] = true;
    }
    else
    {
      alfParamSet->isUniParam[compIdx] = false;
    }
  }

}

/** Disable all ALF unit parameters in current component
 * \param [in] compIdx luma/chroma component index
 * \param [out] alfParamSet to-be-coded ALF parameter set
 * \param [in] alfUnitPic ALF unit parameters for all LCUs in picture
 */
Void TEncAdaptiveLoopFilter::disableComponentAlfParam(Int compIdx, AlfParamSet* alfParamSet, AlfUnitParam* alfUnitPic)
{
  alfParamSet->isEnabled [compIdx] = false;
  alfParamSet->isUniParam[compIdx] = true; //all off

  for(Int lcuPos = 0; lcuPos < m_uiNumCUsInFrame; lcuPos++)
  {
    AlfUnitParam& alfunitParam = alfUnitPic[lcuPos];

    alfunitParam.mergeType = ALF_MERGE_DISABLED; 
    alfunitParam.isEnabled = false; 
    alfunitParam.isNewFilt = false; 
    alfunitParam.storedFiltIdx = -1;
    alfunitParam.alfFiltParam->alf_flag = 0;
  }

  //check merge-up and merge-left
  checkMerge(compIdx, alfUnitPic);

  //transfer to AlfParamSet
  transferToAlfParamSet(compIdx, alfUnitPic, alfParamSet);

}

/** Picture-based encoding
 * \param [out] alfParamSet to-be-coded ALF parameter set
 * \param [in, out] alfPicQTPart picture quad-tree partition
 * \param [in] compIdx luma/chroma component index
 * \param [in] pOrg picture buffer for original picture
 * \param [in] pDec picture buffer for un-filtered picture
 * \param [out] pRest picture buffer for filtered picture
 * \param [in] stride stride size for 1-D picture memory
 * \param [in, out] alfCorrLCUs correlation values for LCUs
 */
Void TEncAdaptiveLoopFilter::executePicBasedModeDecision(AlfParamSet* alfParamSet
                                                        , AlfPicQTPart* alfPicQTPart
                                                        , Int compIdx
                                                        , Pel* pOrg, Pel* pDec, Pel* pRest, Int stride, Int formatShift
                                                        , AlfCorrData** alfCorrLCUs
                                                        )
{
  if(compIdx != ALF_Y)
  {
    if(!alfParamSet->isEnabled[ALF_Y])
    {
      disableComponentAlfParam(compIdx, alfParamSet, m_alfPicFiltUnits[compIdx]);
      return;
    }
  }

  Int picWidth = (m_img_width >> formatShift);
  Int picHeight= (m_img_height >> formatShift);

  Int64  minDist = 0;
  Int64  minRate = 0;
  Double minCost = 0;

  decideQTPartition(alfPicQTPart, alfCorrLCUs, 0, 0, minCost, minDist, minRate);

  //patch quad-tree decision to m_alfPicFiltUnits (m_alfFiltInfo[compIdx])
  patchAlfUnitParams(alfPicQTPart, 0, m_alfPicFiltUnits[compIdx]);

  //check merge-up and merge-left
  checkMerge(compIdx, m_alfPicFiltUnits[compIdx]);

  //transfer to AlfParamSet
  transferToAlfParamSet(compIdx, m_alfPicFiltUnits[compIdx], alfParamSet);

  //reconstruction
  recALF(compIdx, m_alfFiltInfo[compIdx], pDec, pRest, stride, formatShift, NULL, false);

  Double lambda = (compIdx == ALF_Y)?(m_dLambdaLuma):(m_dLambdaChroma);


  std::vector<AlfCUCtrlInfo> alfCUCtrlParamTemp(m_vBestAlfCUCtrlParam); 
  minRate = calculateAlfParamSetRateRDO(compIdx, alfParamSet, &alfCUCtrlParamTemp);
  minDist = xCalcSSD(pOrg, pRest, picWidth, picHeight, stride);
  minCost = (Double)minDist + lambda*((Double)minRate);

  //block on/off control
  if(compIdx == ALF_Y && m_bAlfCUCtrlEnabled)
  {
    decideBlockControl(pOrg, pDec, pRest, stride, alfPicQTPart, alfParamSet, minRate, minDist, minCost);  
  }

  //get filter-off distortion, rate, cost
  AlfParamSet alfParamSetOff;
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    alfCUCtrlParamTemp[s].reset();
  }
  alfParamSetOff.isEnabled[compIdx] = false;
  alfParamSetOff.isUniParam[compIdx] = true;
  Int64  offDist = xCalcSSD(pOrg, pDec, picWidth, picHeight, stride);
  Int64  offRate = calculateAlfParamSetRateRDO(compIdx, &alfParamSetOff, &alfCUCtrlParamTemp);
  Double offCost = (Double)offDist + lambda*((Double)offRate);

  if(offCost < minCost  )
  {
    //revert to filter-off results
    Pel* pelSrc = pDec;
    Pel* pelDst = pRest;
    for(Int y=0; y< picHeight; y++)
    {
      ::memcpy(pelDst, pelSrc, sizeof(Pel)*picWidth);
      pelSrc += stride;
      pelDst += stride;
    }

    alfParamSet->isEnabled[compIdx] = false;
    alfParamSet->isUniParam[compIdx] = true; //all filter-off
  }

}

/** copy picture quadtree infromation
 * \param [out] alfPicQTPartDest destination part in picture quad tree
 * \param [in ] alfPicQTPartSrc source part in picture quad tree
 */
Void TEncAdaptiveLoopFilter::copyPicQT(AlfPicQTPart* alfPicQTPartDest, AlfPicQTPart* alfPicQTPartSrc)
{
  for (Int i=0; i< m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth]; i++)
  {
    alfPicQTPartDest[i] = alfPicQTPartSrc[i];
  }
}

/** copy pixel values for one rectangular region
 * \param [out] imgDest destination part in picture quad tree
 * \param [in ] imgSrc source part in picture quad tree
 * \param [in ] stride source part in picture quad tree
 * \param [in ] yPos starting y position
 * \param [in ] height region height
 * \param [in ] xPos starting x position
 * \param [in ] width region width
 */
Void TEncAdaptiveLoopFilter::copyPixelsInOneRegion(Pel* imgDest, Pel* imgSrc, Int stride, Int yPos, Int height, Int xPos, Int width)
{
  Int offset = (yPos*stride) + xPos;
  Pel *imgDestLine = imgDest + offset;
  Pel *imgSrcLine  = imgSrc  + offset;

  for (Int j=0; j<height; j++)
  {
    ::memcpy(imgDestLine, imgSrcLine, sizeof(Pel)*width);
    imgDestLine += stride;
    imgSrcLine  += stride;
  }
}

/** Re-design ALF parameters for picture quad-tree partitions
 * \param [out] alfPicQTPart picture quad-tree partition information
 * \param [in ] partIdx partition index
 * \param [in ] partLevel partition level
 */
Void TEncAdaptiveLoopFilter::reDesignQT(AlfPicQTPart *alfPicQTPart, Int partIdx, Int partLevel)
{
  AlfPicQTPart *alfPicQTOnePart = &(alfPicQTPart[partIdx]);  
  Int nextPartLevel = partLevel + 1;

  if (!alfPicQTOnePart->isSplit)
  {
    if (alfPicQTOnePart->alfUnitParam->alfFiltParam->alf_flag)
    {
      executeModeDecisionOnePart(alfPicQTPart, m_alfCorr[ALF_Y], partIdx, partLevel) ;     
    }
  }
  else
  {
    for (Int i=0; i<4; i++)
    {
      reDesignQT(alfPicQTPart, alfPicQTOnePart->childPartIdx[i], nextPartLevel);
    }
  }  
}

/** CU-on/off control decision
 * \param [in ] imgOrg picture buffer for original picture
 * \param [in ] imgDec picture buffer for un-filtered picture
 * \param [in ] imgRest picture buffer for filtered picture
 * \param [in ] stride buffer stride size for 1-D picture memory
 * \param [in, out] alfPicQTPart picture quad-tree partition information
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in, out ] minRate minimum rate
 * \param [in, out ] minDist minimum distortion
 * \param [in, out ] minCost minimum RD cost
 */
Void TEncAdaptiveLoopFilter::decideBlockControl(Pel* imgOrg, Pel* imgDec, Pel* imgRest, Int stride, AlfPicQTPart* alfPicQTPart, AlfParamSet* & alfParamSet, Int64 &minRate, Int64 &minDist, Double &minCost)
{
  Int    rate, ctrlDepth;
  Double cost;
  UInt64 dist;
  Bool isChanged = false;
  Pel *imgYtemp = getPicBuf(m_pcPicYuvTmp, ALF_Y);
  Pel *imgYBest = getPicBuf(m_pcPicYuvBest, ALF_Y);
  std::vector<AlfCUCtrlInfo> vAlfCUCtrlParamTemp(m_vBestAlfCUCtrlParam);  

  AlfPicQTPart *alfPicQTPartNoCtrl = new AlfPicQTPart [ m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth] ];
  AlfPicQTPart *alfPicQTPartBest   = new AlfPicQTPart [ m_alfNumCulPartsLevelTab[m_alfPQTMaxDepth] ];

  // backup data of PQT without block on/off
  copyPicQT(alfPicQTPartNoCtrl, alfPicQTPart);

  for (ctrlDepth=0; ctrlDepth<4; ctrlDepth++)
  {        
    // Restore data from PQT without block on/off
    copyPixelsInOneRegion(imgYtemp, imgRest, stride, 0, m_img_height, 0, m_img_width);
    copyPicQT(alfPicQTPart, alfPicQTPartNoCtrl);

    for (Int reDesignRun=0; reDesignRun <= m_iALFNumOfRedesign; reDesignRun++)
    {
      // re-design filter
      if (reDesignRun > 0)
      {
        // re-gather statistics
        getOneCompStatistics(m_alfCorr[ALF_Y], ALF_Y, imgOrg, imgDec, stride, 0, true);

        // reDesign in each QT partition
        reDesignQT(alfPicQTPart, 0, 0);

        //patch quad-tree decision to m_alfPicFiltUnits (m_alfFiltInfo[compIdx])
        patchAlfUnitParams(alfPicQTPart, 0, m_alfPicFiltUnits[ALF_Y]);

        //reconstruction
        copyPixelsInOneRegion(imgYtemp, imgDec, stride, 0, m_img_height, 0, m_img_width);
        recALF(ALF_Y, m_alfFiltInfo[ALF_Y], imgDec, imgYtemp, stride, 0, NULL, false);
      }

      // Gest distortion and decide on/off, Pel should be changed to TComPicYUV
      setCUAlfCtrlFlags((UInt)ctrlDepth, imgOrg, imgDec, imgYtemp, stride, dist, vAlfCUCtrlParamTemp);   

      //patch quad-tree decision to m_alfPicFiltUnits (m_alfFiltInfo[compIdx])
      patchAlfUnitParams(alfPicQTPart, 0, m_alfPicFiltUnits[ALF_Y]);

      //check merge-up and merge-left
      checkMerge(ALF_Y, m_alfPicFiltUnits[ALF_Y]);

      //transfer to AlfParamSet
      transferToAlfParamSet(ALF_Y, m_alfPicFiltUnits[ALF_Y], alfParamSet);

      rate = calculateAlfParamSetRateRDO(ALF_Y, alfParamSet, &vAlfCUCtrlParamTemp);
      cost = (Double)dist + m_dLambdaLuma * ((Double)rate);

      if (cost < minCost)
      {
        isChanged     = true;
        minCost       = cost;
        minDist       = (Int64) dist;
        minRate       = rate;

        m_vBestAlfCUCtrlParam = vAlfCUCtrlParamTemp;
        copyPixelsInOneRegion(imgYBest, imgYtemp, stride, 0, m_img_height, 0, m_img_width);

        copyPicQT(alfPicQTPartBest, alfPicQTPart);
        xCopyTmpAlfCtrlFlagsFrom();
      }

    }
  }

  if (isChanged == true)
  {
    copyPicQT(alfPicQTPart, alfPicQTPartBest);
    xCopyTmpAlfCtrlFlagsTo();

    copyPixelsInOneRegion(imgRest, imgYBest, stride, 0, m_img_height, 0, m_img_width);
    xCopyDecToRestCUs(imgDec, imgRest, stride);
  }
  else
  {
    copyPicQT(alfPicQTPart, alfPicQTPartNoCtrl);
  }

  //patch quad-tree decision to m_alfPicFiltUnits (m_alfFiltInfo[compIdx])
  patchAlfUnitParams(alfPicQTPart, 0, m_alfPicFiltUnits[ALF_Y]);

  //check merge-up and merge-left
  checkMerge(ALF_Y, m_alfPicFiltUnits[ALF_Y]);

  //transfer to AlfParamSet
  transferToAlfParamSet(ALF_Y, m_alfPicFiltUnits[ALF_Y], alfParamSet);

  delete [] alfPicQTPartNoCtrl;
  alfPicQTPartNoCtrl = NULL;

  delete [] alfPicQTPartBest;
  alfPicQTPartBest = NULL;
}

/** Copy ALF unit parameters from quad-tree partition to LCUs
 * \param [in] alfPicQTPart picture quad-tree partition information
 * \param [in] partIdx partition index
 * \param [out] alfUnitPic ALF unit parameters for LCUs
 */
Void TEncAdaptiveLoopFilter::patchAlfUnitParams(AlfPicQTPart* alfPicQTPart, Int partIdx, AlfUnitParam* alfUnitPic)
{
  AlfPicQTPart* alfQTPart = &(alfPicQTPart[partIdx]);
  //Int compIdx = alfQTPart->componentID;

  if(alfQTPart->isSplit == false)
  {
    AlfUnitParam* alfpartParam = alfQTPart->alfUnitParam;

    Int lcuPos;
    for(Int lcuPosY = alfQTPart->partCUYS; lcuPosY <= alfQTPart->partCUYE; lcuPosY++)
    {
      for(Int lcuPosX = alfQTPart->partCUXS; lcuPosX <= alfQTPart->partCUXE; lcuPosX++)
      {
        lcuPos = lcuPosY*m_numLCUInPicWidth + lcuPosX;
        AlfUnitParam& alfunitParam = alfUnitPic[lcuPos];

        alfunitParam.mergeType = alfpartParam->mergeType; 
        alfunitParam.isEnabled = alfpartParam->isEnabled; 
        alfunitParam.isNewFilt = alfpartParam->isNewFilt; 
        alfunitParam.storedFiltIdx = alfpartParam->storedFiltIdx; //not used
        *(alfunitParam.alfFiltParam) = *(alfpartParam->alfFiltParam);
      }
    }
  }
  else
  {
    for(Int i=0; i< 4; i++)
    {
      patchAlfUnitParams(alfPicQTPart, alfQTPart->childPartIdx[i], alfUnitPic);      
    }
  }
}

/** Decide picture quad-tree partition
 * \param [in, out] alfPicQTPart picture quad-tree partition information
 * \param [in, out] alfPicLCUCorr correlations for LCUs
 * \param [int] partIdx partition index
 * \param [int] partLevel partition level
 * \param [in, out] cost cost for one partition
 * \param [in, out] dist distortion for one partition
 * \param [in, out] rate bitrate for one partition
 */
Void TEncAdaptiveLoopFilter::decideQTPartition(AlfPicQTPart* alfPicQTPart, AlfCorrData** alfPicLCUCorr, Int partIdx, Int partLevel, Double &cost, Int64 &dist, Int64 &rate)
{
  AlfPicQTPart* alfPicQTOnePart = &(alfPicQTPart[partIdx]);
  Int nextPartLevel = partLevel + 1;
  Int childPartIdx;
  Double splitCost = 0;
  Int64  splitRate = 0;
  Int64  splitDist = 0;  

  if (!alfPicQTOnePart->isProcessed)
  {
    executeModeDecisionOnePart(alfPicQTPart, alfPicLCUCorr, partIdx, partLevel);

    alfPicQTOnePart->isProcessed = true;
  }

  if (!alfPicQTOnePart->isBottomLevel) 
  {    
    for (Int i=0; i<4; i++)
    {      
      childPartIdx = alfPicQTOnePart->childPartIdx[i];
      decideQTPartition(alfPicQTPart, alfPicLCUCorr, childPartIdx, nextPartLevel, splitCost, splitDist, splitRate);      
    }

    alfPicQTOnePart->splitMinCost = splitCost;
    alfPicQTOnePart->splitMinDist = splitDist;
    alfPicQTOnePart->splitMinRate = splitRate;

    if (alfPicQTOnePart->splitMinCost < alfPicQTOnePart->selfMinCost)
    {
      alfPicQTOnePart->isSplit = true;
    }
    else
    { 
      alfPicQTOnePart->isSplit = false; 
    }
  }
  else
  {
    alfPicQTOnePart->isSplit = false;
    alfPicQTOnePart->splitMinCost = alfPicQTOnePart->selfMinCost;
    alfPicQTOnePart->splitMinDist = alfPicQTOnePart->selfMinDist;
    alfPicQTOnePart->splitMinRate = alfPicQTOnePart->selfMinRate;
  }

  if (alfPicQTOnePart->isSplit)
  {
    cost += alfPicQTOnePart->splitMinCost;
    rate += alfPicQTOnePart->splitMinRate;
    dist += alfPicQTOnePart->splitMinDist;
  }
  else
  {
    cost += alfPicQTOnePart->selfMinCost;
    rate += alfPicQTOnePart->selfMinRate;
    dist += alfPicQTOnePart->selfMinDist;
  }

}

/** Mode decision process for one picture quad-tree partition
 * \param [in, out] alfPicQTPart picture quad-tree partition information
 * \param [in, out] alfPicLCUCorr correlations for LCUs
 * \param [int] partIdx partition index
 * \param [int] partLevel partition level
 */
Void TEncAdaptiveLoopFilter::executeModeDecisionOnePart(AlfPicQTPart *alfPicQTPart, AlfCorrData** alfPicLCUCorr, Int partIdx, Int partLevel)
{
  AlfPicQTPart* alfQTPart = &(alfPicQTPart[partIdx]);
  Int compIdx = alfQTPart->componentID;
  Double lambda = (compIdx == ALF_Y)?(m_dLambdaLuma):(m_dLambdaChroma);

  //gather correlations
  alfQTPart->alfCorr->reset();
  for(Int lcuPosY = alfQTPart->partCUYS; lcuPosY <= alfQTPart->partCUYE; lcuPosY++)
  {
    for(Int lcuPosX = alfQTPart->partCUXS; lcuPosX <= alfQTPart->partCUXE; lcuPosX++)
    {
      *(alfQTPart->alfCorr) +=  *(alfPicLCUCorr[lcuPosY*m_numLCUInPicWidth + lcuPosX]);
    }
  }

  //test filter on
  AlfUnitParam* alfPartUnitParam = alfQTPart->alfUnitParam;
  alfPartUnitParam->mergeType = ALF_MERGE_DISABLED;
  alfPartUnitParam->isEnabled = true;
  alfPartUnitParam->isNewFilt = true;
  alfPartUnitParam->storedFiltIdx = -1;
  alfPartUnitParam->alfFiltParam->alf_flag = 1;
  deriveFilterInfo(compIdx, alfQTPart->alfCorr, alfPartUnitParam->alfFiltParam, alfQTPart->numFilterBudget);

  alfQTPart->selfMinDist = estimateFilterDistortion(compIdx, alfQTPart->alfCorr, m_filterCoeffSym, alfPartUnitParam->alfFiltParam->filters_per_group, m_varIndTab);
  alfQTPart->selfMinRate = calculateAlfUnitRateRDO(alfPartUnitParam);
  alfQTPart->selfMinCost = (Double)(alfQTPart->selfMinDist) + lambda*((Double)(alfQTPart->selfMinRate));
  
  alfQTPart->selfMinCost +=  ((lambda* 1.5)* ((Double)( (alfQTPart->partCUYE - alfQTPart->partCUYS+ 1)*(alfQTPart->partCUXE - alfQTPart->partCUXS +1) )));  //RDCO
  

  //test filter off
  AlfUnitParam alfUnitParamTemp(*(alfQTPart->alfUnitParam));
  alfUnitParamTemp.mergeType = ALF_MERGE_DISABLED;
  alfUnitParamTemp.isEnabled = false;
  Int64  dist = estimateFilterDistortion(compIdx, alfQTPart->alfCorr);
  Int64  rate = calculateAlfUnitRateRDO(&alfUnitParamTemp);
  Double cost = (Double)dist + lambda*((Double)rate);
  if(cost < alfQTPart->selfMinCost)
  {
    alfQTPart->selfMinCost = cost;
    alfQTPart->selfMinDist = dist;
    alfQTPart->selfMinRate = rate;
    *(alfQTPart->alfUnitParam) = alfUnitParamTemp;

    alfQTPart->alfUnitParam->alfFiltParam->alf_flag = 0;
  }

}
#endif

/** Derive filter coefficients
 * \param [in, out] alfPicQTPart picture quad-tree partition information
 * \param [in, out] alfPicLCUCorr correlations for LCUs
 * \param [int] partIdx partition index
 * \param [int] partLevel partition level
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::deriveFilterInfo(Int compIdx, AlfCorrData* alfCorr, ALFParam* alfFiltParam, Int maxNumFilters, Double lambda)
#else
Void TEncAdaptiveLoopFilter::deriveFilterInfo(Int compIdx, AlfCorrData* alfCorr, ALFParam* alfFiltParam, Int maxNumFilters)
#endif
{
  const Int filtNo = 0; 
  const Int numCoeff = ALF_MAX_NUM_COEF;

  switch(compIdx)
  {
  case ALF_Y:
    {
#if AHG6_ALF_OPTION2
      Int lambdaForMerge = ((Int) lambda) * (1<<(2*g_uiBitIncrement));
#else
      Int lambdaForMerge = ((Int) m_dLambdaLuma) * (1<<(2*g_uiBitIncrement));
#endif
      Int numFilters;

      ::memset(m_varIndTab, 0, sizeof(Int)*NO_VAR_BINS);
#if AHG6_ALF_OPTION2
      xfindBestFilterVarPred(alfCorr->yCorr, alfCorr->ECorr, alfCorr->pixAcc, m_filterCoeffSym, &numFilters, m_varIndTab, lambdaForMerge, maxNumFilters);
      xcodeFiltCoeff(m_filterCoeffSym,  m_varIndTab, numFilters, alfFiltParam);
#else
      xfindBestFilterVarPred(alfCorr->yCorr, alfCorr->ECorr, alfCorr->pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &numFilters, m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambdaForMerge, maxNumFilters);
      xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, numFilters, alfFiltParam);
#endif
    }
    break;
  case ALF_Cb:
  case ALF_Cr:
    {
      static Double coef[ALF_MAX_NUM_COEF];

      alfFiltParam->filters_per_group = 1;

      gnsSolveByChol(alfCorr->ECorr[0], alfCorr->yCorr[0], coef, numCoeff);
      xQuantFilterCoef(coef, m_filterCoeffSym[0], filtNo, g_uiBitDepth + g_uiBitIncrement);
      ::memcpy(alfFiltParam->coeffmulti[0], m_filterCoeffSym[0], sizeof(Int)*numCoeff);
#if AHG6_ALF_OPTION2
      predictALFCoeff(alfFiltParam->coeffmulti, numCoeff, alfFiltParam->filters_per_group);
#else
      predictALFCoeffChroma(alfFiltParam->coeffmulti[0]);
#endif
    }
    break;
  default:
    {
      printf("Not a legal component ID\n");
      assert(0);
      exit(-1);
    }
  }


}
#if !AHG6_ALF_OPTION2
/** Estimate rate-distortion cost for ALF parameter set
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfParamSet ALF parameter set
 * \param [in] alfCUCtrlParam CU-on/off control parameters
 */
Int TEncAdaptiveLoopFilter::calculateAlfParamSetRateRDO(Int compIdx, AlfParamSet* alfParamSet, std::vector<AlfCUCtrlInfo>* alfCUCtrlParam)
{
  Int rate = 0;

  m_pcEntropyCoder->resetEntropy();
  m_pcEntropyCoder->resetBits();


  m_pcEntropyCoder->encodeAlfParamSet(alfParamSet, m_numLCUInPicWidth, m_uiNumCUsInFrame, 0, true, compIdx, compIdx);

  if(m_bAlfCUCtrlEnabled)
  {
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      m_pcEntropyCoder->encodeAlfCtrlParam( (*alfCUCtrlParam)[s], m_uiNumCUsInFrame);      
    }
  }

  rate = m_pcEntropyCoder->getNumberOfWrittenBits();

  return rate;
}

/** Estimate rate-distortion cost for ALF unit parameters
 * \param [in] alfUnitParam ALF unit parameters
 * \param [in] numStoredFilters number of stored filter (set)
 */
Int TEncAdaptiveLoopFilter::calculateAlfUnitRateRDO(AlfUnitParam* alfUnitParam, Int numStoredFilters)
{
  Int rate = 0;

  if(alfUnitParam->mergeType != ALF_MERGE_LEFT)
  {
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();

    m_pcEntropyCoder->encodeAlfFlag( (alfUnitParam->mergeType == ALF_MERGE_UP)?1:0);

    if(alfUnitParam->mergeType != ALF_MERGE_UP)
    {
      m_pcEntropyCoder->encodeAlfFlag( (alfUnitParam->isEnabled)?1:0);

      if(alfUnitParam->isEnabled)
      {
        if(numStoredFilters > 0)
        {
          m_pcEntropyCoder->encodeAlfFlag( (alfUnitParam->isNewFilt)?1:0);
        }

        if(!(alfUnitParam->isNewFilt) && numStoredFilters > 0)
        {
          m_pcEntropyCoder->encodeAlfStoredFilterSetIdx(alfUnitParam->storedFiltIdx, numStoredFilters);
        }
        else
        {
          m_pcEntropyCoder->encodeAlfParam(alfUnitParam->alfFiltParam);
        }

      }
    }
    rate = m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  return rate;
}
#endif
/** Estimate filtering distortion
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfCorr correlations
 * \param [in] coeffSet filter coefficients
 * \param [in] filterSetSize number of filter set
 * \param [in] mergeTable merge table of filter set (only for luma BA)
 * \param [in] doPixAccMerge calculate pixel squared value (true) or not (false)
 */
Int64 TEncAdaptiveLoopFilter::estimateFilterDistortion(Int compIdx, AlfCorrData* alfCorr, Int** coeffSet, Int filterSetSize, Int* mergeTable, Bool doPixAccMerge)
{
  const Int numCoeff = (Int)ALF_MAX_NUM_COEF;
  AlfCorrData* alfMerged = m_alfCorrMerged[compIdx];

  alfMerged->mergeFrom(*alfCorr, mergeTable, doPixAccMerge);

  Int**     coeff = (coeffSet == NULL)?(m_coeffNoFilter):(coeffSet);
  Int64     iDist = 0;
  for(Int f=0; f< filterSetSize; f++)
  {
    iDist += xFastFiltDistEstimation(alfMerged->ECorr[f], alfMerged->yCorr[f], coeff[f], numCoeff);
  }
  return iDist;
}
#if !AHG6_ALF_OPTION2
/** Mode decision for ALF unit in LCU-based encoding
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfUnitPic ALF unit parmeters for LCUs in picture
 * \param [in] lcuIdx LCU index (order) in slice
 * \param [in] lcuPos LCU position in picture
 * \param [in] numLCUWidth number of width in LCU
 * \param [in, out] alfUnitParams ALF unit parameters for LCUs in slice
 * \param [in] alfCorr correlations
 * \param [in] storedFilters stored-filter buffer
 * \param [in] maxNumFilter constraint for number of filters
 * \param [in] lambda lagrangian multiplier for RDO
 * \param [in] isLeftUnitAvailable left ALF unit available (true) or not (false)
 * \param [in] isUpUnitAvailable upper ALF unit available (true) or not (false)
 */
Void TEncAdaptiveLoopFilter::decideLCUALFUnitParam(Int compIdx, AlfUnitParam* alfUnitPic, Int lcuIdx, Int lcuPos, Int numLCUWidth, AlfUnitParam* alfUnitParams, AlfCorrData* alfCorr, std::vector<ALFParam*>& storedFilters, Int maxNumFilter, Double lambda, Bool isLeftUnitAvailable, Bool isUpUnitAvailable)
{
  Int    numSliceDataInCurrLCU = m_numSlicesDataInOneLCU[lcuPos];
  Int    budgetNumFilters = (Int)(maxNumFilter/numSliceDataInCurrLCU);
  Int    numStoredFilters = (Int)storedFilters.size();
  Double cost, minCost = MAX_DOUBLE;
  Int64  dist;
  Int    rate;

  AlfUnitParam& alfUnitParamCurr = alfUnitParams[lcuIdx];

  ///--- new filter mode test ---
  AlfUnitParam alfUnitParamTemp(alfUnitParamCurr);
  alfUnitParamTemp.mergeType = ALF_MERGE_DISABLED;
  alfUnitParamTemp.isEnabled = true;
  alfUnitParamTemp.isNewFilt = true;
  alfUnitParamTemp.storedFiltIdx = -1;
  deriveFilterInfo(compIdx, alfCorr, alfUnitParamTemp.alfFiltParam, budgetNumFilters);

  dist = estimateFilterDistortion(compIdx, alfCorr, m_filterCoeffSym, alfUnitParamTemp.alfFiltParam->filters_per_group, m_varIndTab);
  rate = calculateAlfUnitRateRDO(&alfUnitParamTemp, numStoredFilters);
  cost = (Double)dist + lambda*((Double)rate);
  if(cost < minCost)
  {
    minCost = cost;
    alfUnitParamCurr = alfUnitParamTemp;

    alfUnitParamCurr.alfFiltParam->alf_flag = 1;
  }

  if(numSliceDataInCurrLCU == 1)
  {
    if(numStoredFilters > 0)
    {
      ///--- stored filter mode test ---//
      alfUnitParamTemp = alfUnitParamCurr;

      alfUnitParamTemp.mergeType = ALF_MERGE_DISABLED;
      alfUnitParamTemp.isEnabled = true;
      alfUnitParamTemp.isNewFilt = false;

      for(Int i=0; i< numStoredFilters; i++)
      {
        ALFParam* storedALFParam = storedFilters[i];

        alfUnitParamTemp.storedFiltIdx = i;
        alfUnitParamTemp.alfFiltParam  = storedALFParam;

        assert(storedALFParam->alf_flag == 1);

        reconstructCoefInfo(compIdx, storedALFParam, m_filterCoeffSym, m_varIndTab);

        dist = estimateFilterDistortion(compIdx, alfCorr, m_filterCoeffSym, alfUnitParamTemp.alfFiltParam->filters_per_group, m_varIndTab);
        rate = calculateAlfUnitRateRDO(&alfUnitParamTemp, numStoredFilters);
        cost = (Double)dist + lambda*((Double)rate);

        if(cost < minCost)
        {
          minCost = cost;
          alfUnitParamCurr = alfUnitParamTemp;
        }
      }
    }

    /// merge-up test
    if(isUpUnitAvailable)
    {
      Int addrUp = lcuPos - m_numLCUInPicWidth;
      AlfUnitParam& alfUnitParamUp = alfUnitPic[addrUp];

      if(alfUnitParamUp.alfFiltParam->alf_flag == 1)
      {
        alfUnitParamTemp = alfUnitParamUp;
        alfUnitParamTemp.mergeType    = ALF_MERGE_UP;

        reconstructCoefInfo(compIdx, alfUnitParamTemp.alfFiltParam, m_filterCoeffSym, m_varIndTab);
        dist = estimateFilterDistortion(compIdx, alfCorr, m_filterCoeffSym, alfUnitParamTemp.alfFiltParam->filters_per_group, m_varIndTab);
        rate = calculateAlfUnitRateRDO(&alfUnitParamTemp, numStoredFilters);
        cost = (Double)dist + lambda*((Double)rate);

        if(cost < minCost)
        {
          minCost = cost;

          alfUnitParamCurr = alfUnitParamTemp;
        }

      }

    } //upper unit available


    /// merge-left test
    if(isLeftUnitAvailable)
    {
      Int addrLeft = lcuPos - 1;
      AlfUnitParam& alfUnitParamLeft = alfUnitPic[addrLeft];

      if(alfUnitParamLeft.alfFiltParam->alf_flag == 1)
      {
        alfUnitParamTemp = alfUnitParamLeft;
        alfUnitParamTemp.mergeType    = ALF_MERGE_LEFT;

        reconstructCoefInfo(compIdx, alfUnitParamTemp.alfFiltParam, m_filterCoeffSym, m_varIndTab);
        dist = estimateFilterDistortion(compIdx, alfCorr, m_filterCoeffSym, alfUnitParamTemp.alfFiltParam->filters_per_group, m_varIndTab);
        rate = calculateAlfUnitRateRDO(&alfUnitParamTemp, numStoredFilters);
        cost = (Double)dist + lambda*((Double)rate);

        if(cost < minCost)
        {
          minCost = cost;

          alfUnitParamCurr = alfUnitParamTemp;
        }

      }

    } //left unit available

  }
}

/** Choose the best ALF unit parameters when filter is not enabled.
 * \param [out] alfFiltOffParam ALF unit parameters for filter-off case
 * \param [in] lcuPos LCU position in picture
 * \param [in] alfUnitPic ALF unit parmeters for LCUs in picture
 * \param [in] isLeftUnitAvailable left ALF unit available (true) or not (false)
 * \param [in] isUpUnitAvailable upper ALF unit available (true) or not (false)
 */
Void TEncAdaptiveLoopFilter::getFiltOffAlfUnitParam(AlfUnitParam* alfFiltOffParam, Int lcuPos, AlfUnitParam* alfUnitPic, Bool isLeftUnitAvailable, Bool isUpUnitAvailable)
{
  Int    numSliceDataInCurrLCU = m_numSlicesDataInOneLCU[lcuPos];

  if(numSliceDataInCurrLCU == 1)
  {
    if(isLeftUnitAvailable)
    {
      Int addrLeft = lcuPos - 1;
      AlfUnitParam& alfUnitParamLeft = alfUnitPic[addrLeft];

      if(alfUnitParamLeft.alfFiltParam->alf_flag == 0)
      {
        alfFiltOffParam->mergeType    = ALF_MERGE_LEFT;
        alfFiltOffParam->isEnabled    = false;
        alfFiltOffParam->alfFiltParam = alfUnitParamLeft.alfFiltParam;

        return;
      }
    }

    if(isUpUnitAvailable)
    {
      Int addrUp = lcuPos - m_numLCUInPicWidth;
      AlfUnitParam& alfUnitParamUp = alfUnitPic[addrUp];

      if(alfUnitParamUp.alfFiltParam->alf_flag == 0)
      {
        alfFiltOffParam->mergeType    = ALF_MERGE_UP;
        alfFiltOffParam->isEnabled    = false;
        alfFiltOffParam->alfFiltParam = alfUnitParamUp.alfFiltParam;

        return;
      }

    }
  }


  alfFiltOffParam->mergeType = ALF_MERGE_DISABLED;
  alfFiltOffParam->isEnabled = false;
  alfFiltOffParam->alfFiltParam = alfUnitPic[lcuPos].alfFiltParam;

  return;
}
#endif
/** Calculate distortion for ALF LCU
 * \param [in] skipLCUBottomLines true for considering skipping bottom LCU lines
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfLCUInfo ALF LCU information
 * \param [in] picSrc source picture buffer
 * \param [in] picCmp to-be-compared picture buffer
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 * \return the distortion
 */
Int64 TEncAdaptiveLoopFilter::calcAlfLCUDist(Bool skipLCUBottomLines, Int compIdx, AlfLCUInfo& alfLCUInfo, Pel* picSrc, Pel* picCmp, Int stride, Int formatShift)
{
  Int64 dist = 0;  
  Int  posOffset, ypos, xpos, height, width;
  Pel* pelCmp;
  Pel* pelSrc;
#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER || LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
  Int endypos;
  Bool notSkipLinesBelowVB = true;
  Int lcuAddr = alfLCUInfo.pcCU->getAddr();
  if(skipLCUBottomLines)
  {
    if(lcuAddr + m_numLCUInPicWidth < m_uiNumCUsInFrame)
    {
      notSkipLinesBelowVB = false;
    }
  }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
  Bool notSkipLinesRightVB = true;
  Int endxpos;
  if(skipLCUBottomLines)
  {
    if((lcuAddr + 1) % m_numLCUInPicWidth != 0 )
    {
      notSkipLinesRightVB = false;
    }
  }
#endif

  switch(compIdx)
  {
  case ALF_Cb:
  case ALF_Cr:
    {
      for(Int n=0; n< alfLCUInfo.numSGU; n++)
      {
        ypos    = (Int)(alfLCUInfo[n].posY   >> formatShift);
        xpos    = (Int)(alfLCUInfo[n].posX   >> formatShift);
        height  = (Int)(alfLCUInfo[n].height >> formatShift);
        width   = (Int)(alfLCUInfo[n].width  >> formatShift);

#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER
        if(!notSkipLinesBelowVB )
        {
          endypos = ypos+ height -1;
          Int iLineVBPos = m_lcuHeightChroma - 2;
          Int yEndLineInLCU = endypos % m_lcuHeightChroma;
          height = (yEndLineInLCU >= iLineVBPos) ? (height - 2) : height ; 
        }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
        if( !notSkipLinesRightVB)
        {
          endxpos = xpos+ width -1;
          Int iLineVBPos = m_lcuWidthChroma - 7;
          Int xEndLineInLCU = endxpos % m_lcuWidthChroma;
          width = (xEndLineInLCU >= iLineVBPos) ? (width - 7) : width ; 
        }
#endif
        posOffset = (ypos * stride) + xpos;
        pelCmp    = picCmp + posOffset;    
        pelSrc    = picSrc + posOffset;    


        dist  += xCalcSSD( pelSrc, pelCmp,  width, height, stride );
      }

    }
    break;
  case ALF_Y:
    {
      for(Int n=0; n< alfLCUInfo.numSGU; n++)
      {
        ypos    = (Int)(alfLCUInfo[n].posY);
        xpos    = (Int)(alfLCUInfo[n].posX);
        height  = (Int)(alfLCUInfo[n].height);
        width   = (Int)(alfLCUInfo[n].width);

#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER
        if(!notSkipLinesBelowVB)
        {
          endypos = ypos+ height -1;
          Int iLineVBPos = m_lcuHeight - 4;
          Int yEndLineInLCU = endypos % m_lcuHeight;
          height = (yEndLineInLCU >= iLineVBPos) ? (height - 4) : height ; 
        }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
        if( !notSkipLinesRightVB)
        {
          endxpos = xpos+ width -1;
          Int iLineVBPos = m_lcuWidth - 9;
          Int xEndLineInLCU = endxpos % m_lcuWidth;
          width = (xEndLineInLCU >= iLineVBPos) ? (width - 9) : width ; 
        }
#endif
        posOffset = (ypos * stride) + xpos;
        pelCmp    = picCmp + posOffset;    
        pelSrc    = picSrc + posOffset;    

        dist  += xCalcSSD( pelSrc, pelCmp,  width, height, stride );
      }

    }
    break;
  default:
    {
      printf("not a legal component ID for ALF \n");
      assert(0);
      exit(-1);
    }
  }

  return dist;
}

/** Copy one ALF LCU region
 * \param [in] alfLCUInfo ALF LCU information
 * \param [out] picDst to-be-compared picture buffer
 * \param [in] picSrc source picture buffer
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 */
Void TEncAdaptiveLoopFilter::copyOneAlfLCU(AlfLCUInfo& alfLCUInfo, Pel* picDst, Pel* picSrc, Int stride, Int formatShift)
{
  Int posOffset, ypos, xpos, height, width;
  Pel* pelDst;
  Pel* pelSrc;

  for(Int n=0; n< alfLCUInfo.numSGU; n++)
  {
    ypos    = (Int)(alfLCUInfo[n].posY   >> formatShift);
    xpos    = (Int)(alfLCUInfo[n].posX   >> formatShift);
    height  = (Int)(alfLCUInfo[n].height >> formatShift);
    width   = (Int)(alfLCUInfo[n].width  >> formatShift);

    posOffset  = ( ypos * stride)+ xpos;
    pelDst   = picDst  + posOffset;    
    pelSrc   = picSrc  + posOffset;    

    for(Int j=0; j< height; j++)
    {
      ::memcpy(pelDst, pelSrc, sizeof(Pel)*width);
      pelDst += stride;
      pelSrc += stride;
    }
  }

}

/** Reconstruct ALF LCU pixels
 * \param [in] compIdx luma/chroma component index
 * \param [in] alfLCUInfo ALF LCU information
 * \param [in] alfUnitParam ALF unit parameters
 * \param [in] picDec picture buffer for un-filtered picture 
 * \param [out] picRest picture buffer for reconstructed picture
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::reconstructOneAlfLCU(Int compIdx, AlfLCUInfo& alfLCUInfo, Bool alfEnabled, ALFParam* alfParam, Pel* picDec, Pel* picRest, Int stride, Int formatShift)
#else
Void TEncAdaptiveLoopFilter::reconstructOneAlfLCU(Int compIdx, AlfLCUInfo& alfLCUInfo, AlfUnitParam* alfUnitParam, Pel* picDec, Pel* picRest, Int stride, Int formatShift)
#endif
{
#if !AHG6_ALF_OPTION2
  ALFParam* alfParam = alfUnitParam->alfFiltParam;
#endif
  Int ypos, xpos, height, width;

#if AHG6_ALF_OPTION2
  if(alfEnabled)
#else
  if( alfUnitParam->isEnabled)
#endif
  {
    assert(alfParam->alf_flag == 1);

    //reconstruct ALF coefficients & related parameters 
    reconstructCoefInfo(compIdx, alfParam, m_filterCoeffSym, m_varIndTab);

    //filtering process
    for(Int n=0; n< alfLCUInfo.numSGU; n++)
    {
      ypos    = (Int)(alfLCUInfo[n].posY   >> formatShift);
      xpos    = (Int)(alfLCUInfo[n].posX   >> formatShift);
      height  = (Int)(alfLCUInfo[n].height >> formatShift);
      width   = (Int)(alfLCUInfo[n].width  >> formatShift);

      filterOneCompRegion(picRest, picDec, stride, (compIdx!=ALF_Y), ypos, ypos+height, xpos, xpos+width, m_filterCoeffSym, m_varIndTab, m_varImg);
    }
  }
  else
  {
    copyOneAlfLCU(alfLCUInfo, picRest, picDec, stride, formatShift);
  }
}

/** LCU-based mode decision
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in] compIdx luma/chroma component index
 * \param [in] pOrg picture buffer for original picture
 * \param [in] pDec picture buffer for un-filtered picture 
 * \param [out] pRest picture buffer for reconstructed picture
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 * \param [in] alfCorrLCUs correlations for LCUs
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::executeLCUOnOffDecision(Int compIdx, ALFParam* alfParam, Pel* pOrg, Pel* pDec, Pel* pRest, Int stride, Int formatShift,AlfCorrData** alfCorrLCUs)
#else
Void TEncAdaptiveLoopFilter::executeLCUBasedModeDecision(AlfParamSet* alfParamSet
                                                        ,Int compIdx, Pel* pOrg, Pel* pDec, Pel* pRest, Int stride, Int formatShift
                                                        ,AlfCorrData** alfCorrLCUs
                                                        )
#endif
{
  Double lambda = (compIdx == ALF_Y)?(m_dLambdaLuma):(m_dLambdaChroma);
  static Int* isProcessed = NULL;

#if !AHG6_ALF_OPTION2
  AlfUnitParam* alfUnitPic = m_alfPicFiltUnits[compIdx];
#endif

  Int64  distEnc, distOff;
  Int    rateEnc, rateOff;
  Double costEnc, costOff;
#if !AHG6_ALF_OPTION2
  Bool isLeftUnitAvailable, isUpUnitAvailable;
#endif
  isProcessed = new Int[m_uiNumCUsInFrame];
  ::memset(isProcessed, 0, sizeof(Int)*m_uiNumCUsInFrame);

#if AHG6_ALF_OPTION2
  //reset LCU enabled flags
  for(Int i=0; i< m_uiNumCUsInFrame; i++)
  {
    m_pcPic->getCU(i)->setAlfLCUEnabled(false, compIdx);
  }
#else
#if LCUALF_FILTER_BUDGET_CONTROL_ENC
  Int numProcessedLCU = 0;
  m_alfFiltBudgetPerLcu = (Double)(m_iALFMaxNumberFilters) / (Double)(m_uiNumCUsInFrame);
  m_alfUsedFilterNum = 0;
#endif
#endif
  for(Int s=0; s<= m_lastSliceIdx; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }
#if !AHG6_ALF_OPTION2
    Bool isAcrossSlice = (m_alfCoefInSlice)?(!m_isNonCrossSlice):(true);
    Int  numLCUWidth   = alfParamSet[s].numLCUInWidth;

    AlfUnitParam* alfSliceUnitParams = alfParamSet[s].alfUnitParam[compIdx];
    std::vector<ALFParam*> storedFilters;
    storedFilters.clear(); //reset stored filter buffer at the slice beginning

    Int u =0; //counter for LCU index in slice
    Int countFiltOffLCU = 0; //counter for number of LCU with filter-off mode
    Int countNewFilts = 0; //counter for number of LCU with new filter inside slice
#endif
    Int numTilesInSlice = (Int)m_pvpSliceTileAlfLCU[s].size();
    for(Int t=0; t< numTilesInSlice; t++)
    {
      std::vector<AlfLCUInfo*> & vpAlfLCU = m_pvpSliceTileAlfLCU[s][t];
      Pel* pSrc = pDec;

      if(m_bUseNonCrossALF)
      {
        pSrc = getPicBuf(m_pcSliceYuvTmp, compIdx);
        copyRegion(vpAlfLCU, pSrc, pDec, stride, formatShift);
        extendRegionBorder(vpAlfLCU, pSrc, stride, formatShift);
      }

      Int numLCUs = (Int)vpAlfLCU.size();
      for(Int n=0; n< numLCUs; n++)
      {
        AlfLCUInfo*   alfLCU       = vpAlfLCU[n];                  //ALF LCU information
        TComDataCU*   pcCU         = alfLCU->pcCU;
        Int           addr         = pcCU->getAddr();              //real LCU addr
#if !AHG6_ALF_OPTION2
        AlfUnitParam* alfUnitParam = &(alfSliceUnitParams[u]);
#endif
        if(isProcessed[addr] == 0)
        {
#if AHG6_ALF_OPTION2
          Bool lcuAlfDisabled = true;
          if(alfParam->alf_flag == 1)
          {
            //ALF on
            reconstructOneAlfLCU(compIdx, *alfLCU, true, alfParam, pSrc, pRest, stride, formatShift);
            distEnc = calcAlfLCUDist(m_alfLowLatencyEncoding, compIdx, *alfLCU, pOrg, pRest, stride, formatShift);
            rateEnc = 1;
            costEnc = (Double)distEnc + lambda*((Double)rateEnc);
            costEnc += ((lambda* 1.5)*1.0);  //RDCO

            //ALF off
            distOff = calcAlfLCUDist(m_alfLowLatencyEncoding, compIdx, *alfLCU, pOrg, pSrc, stride, formatShift);
            rateOff = 1;
            costOff = (Double)distOff + lambda*((Double)rateOff);

            lcuAlfDisabled = (costOff < costEnc);

            pcCU->setAlfLCUEnabled( (lcuAlfDisabled?0:1) , compIdx);
          }

          if(lcuAlfDisabled)
          {
            copyOneAlfLCU(*alfLCU, pRest, pSrc, stride, formatShift);
          }
#else
          Int           maxNumFilter = (Int)NO_VAR_BINS;   

#if LCUALF_FILTER_BUDGET_CONTROL_ENC
          Bool          isOutOfFilterBudget = true;
          Double        usedFiltBudget = (numProcessedLCU == 0) ? 0.0 : (Double)m_alfUsedFilterNum / (Double)(numProcessedLCU);
          if ( (m_alfFiltBudgetPerLcu >= usedFiltBudget) && (m_alfUsedFilterNum < m_iALFMaxNumberFilters) )
          {
            isOutOfFilterBudget = false;
            Int leftNumFilt = m_iALFMaxNumberFilters - m_alfUsedFilterNum;
            Int avgNumFilt  = leftNumFilt / (m_uiNumCUsInFrame - numProcessedLCU) + 1 ;
            maxNumFilter = (leftNumFilt < avgNumFilt) ? leftNumFilt : avgNumFilt ;
          }
#endif

          AlfCorrData*  alfCorr      = alfCorrLCUs[addr];            //ALF LCU correlation
          alfUnitParam->alfFiltParam = alfUnitPic[addr].alfFiltParam;

          //mode decision 
          isLeftUnitAvailable = (   (addr % m_numLCUInPicWidth != 0) && (u != 0));
          isUpUnitAvailable   = (((Int)(addr/m_numLCUInPicWidth) > 0) && ( ( (u - numLCUWidth) >= 0) || isAcrossSlice ));

          decideLCUALFUnitParam(compIdx, alfUnitPic, u, addr, numLCUWidth, alfSliceUnitParams, alfCorr, storedFilters, maxNumFilter, lambda, isLeftUnitAvailable, isUpUnitAvailable);
          reconstructOneAlfLCU(compIdx, *alfLCU, alfUnitParam, pSrc, pRest, stride, formatShift);
          distEnc = calcAlfLCUDist(!m_picBasedALFEncode, compIdx, *alfLCU, pOrg, pRest, stride, formatShift);
          rateEnc = calculateAlfUnitRateRDO(alfUnitParam, (Int)storedFilters.size());
          costEnc = (Double)distEnc + lambda*((Double)rateEnc);
          costEnc += ((lambda* 1.5)*1.0);  //RDCO

          //v.s. filter off case
          AlfUnitParam alfUnitParamOff;
          getFiltOffAlfUnitParam(&alfUnitParamOff, addr, alfUnitPic, isLeftUnitAvailable, isUpUnitAvailable);
          distOff = calcAlfLCUDist(!m_picBasedALFEncode, compIdx, *alfLCU, pOrg, pSrc, stride, formatShift);
          rateOff = calculateAlfUnitRateRDO(&alfUnitParamOff, (Int)storedFilters.size());
          costOff = (Double)distOff + lambda*((Double)rateOff);

#if LCUALF_FILTER_BUDGET_CONTROL_ENC
          if( (costOff < costEnc)  ||  isOutOfFilterBudget)
#else
          if( costOff < costEnc)
#endif
          {
            //filter off. set alf_flag = 0, copy pDest to pRest
            *alfUnitParam = alfUnitParamOff;
            alfUnitParam->alfFiltParam->alf_flag = 0;
            copyOneAlfLCU(*alfLCU, pRest, pSrc, stride, formatShift);
          }

          if(alfUnitParam->mergeType == ALF_MERGE_DISABLED)
          {
            if(alfUnitParam->isEnabled)
            {
              if(alfUnitParam->isNewFilt)
              {
                //update stored filter buffer
                storedFilters.push_back(alfUnitParam->alfFiltParam);
                assert(alfUnitParam->alfFiltParam->alf_flag == 1);
              }
            }
          }

          alfUnitPic[addr] = *alfUnitParam;

          isProcessed[addr] = 1;

#if LCUALF_FILTER_BUDGET_CONTROL_ENC
          numProcessedLCU++;
          if(alfUnitParam->mergeType == ALF_MERGE_DISABLED && alfUnitParam->isEnabled && alfUnitParam->isNewFilt)
          {
            m_alfUsedFilterNum += alfUnitParam->alfFiltParam->filters_per_group;
          }
#endif
#endif
        }
        else
        {
#if AHG6_ALF_OPTION2
          reconstructOneAlfLCU(compIdx, *alfLCU, pcCU->getAlfLCUEnabled(compIdx), alfParam, pSrc, pRest, stride, formatShift);
#else
          //keep the ALF parameters in LCU are the same
          *alfUnitParam = alfUnitPic[addr];
          reconstructOneAlfLCU(compIdx, *alfLCU, alfUnitParam, pSrc, pRest, stride, formatShift);

#if LCUALF_FILTER_BUDGET_CONTROL_ENC
          if(alfUnitParam->mergeType == ALF_MERGE_DISABLED && alfUnitParam->isEnabled && alfUnitParam->isNewFilt)
          {
            m_alfUsedFilterNum += alfUnitParam->alfFiltParam->filters_per_group;
          }
#endif
#endif
        }
#if !AHG6_ALF_OPTION2
        if(alfUnitParam->alfFiltParam->alf_flag == 0)
        {
          countFiltOffLCU++;
        }
        else
        {
          Bool isNewFiltInSlice =   (alfUnitParam->mergeType == ALF_MERGE_DISABLED && alfUnitParam->isEnabled && alfUnitParam->isNewFilt);
          Bool isMergeAcrossSlice = ( alfUnitParam->mergeType == ALF_MERGE_UP && (u-numLCUWidth < 0) );

          if( isNewFiltInSlice || isMergeAcrossSlice )
          {
            countNewFilts++;
          }
        }

        u++;      
#endif
      } //LCU
    } //tile

#if !AHG6_ALF_OPTION2
    //slice-level parameters
    AlfUnitParam* firstAlfUnitInSlice = &(alfSliceUnitParams[0]);
    if( countFiltOffLCU == u ) //number of filter-off LCU is equal to the number of LCUs in slice
    {
      alfParamSet[s].isEnabled [compIdx] = false;    
      alfParamSet[s].isUniParam[compIdx] = true; //uni-param, all off
      assert(firstAlfUnitInSlice->alfFiltParam->alf_flag == 0);
    }
    else
    {
      alfParamSet[s].isEnabled[compIdx] = true;
      if( countNewFilts == 1 && firstAlfUnitInSlice->alfFiltParam->alf_flag != 0 && countFiltOffLCU == 0 )
      {
        alfParamSet[s].isUniParam[compIdx] = true;
      }
      else
      {
        alfParamSet[s].isUniParam[compIdx] = false;
      }
    }
#endif
  } //slice


  delete[] isProcessed;
  isProcessed = NULL;
}

#if AHG6_ALF_OPTION2
Int TEncAdaptiveLoopFilter::getTemporalLayerNo(Int poc, Int picDistance)
{
  Int layer = 0;
  while(picDistance > 0)
  {
    if(poc % picDistance == 0)
    {
      break;
    }
    picDistance = (Int)(picDistance/2);
    layer++;
  }
  return layer;
}

Void TEncAdaptiveLoopFilter::setCurAlfParam(ALFParam** alfPictureParam)
{
  if(m_alfLowLatencyEncoding)
  {
    Int temporalLayer = getTemporalLayerNo(m_pcPic->getPOC(), m_gopSize);
    for(Int compIdx=0; compIdx< 3; compIdx++)
    {
      *(alfPictureParam[compIdx]) = *(m_alfPictureParam[temporalLayer][compIdx]);
    }
  }
  else
  {
    decideAlfPictureParam(alfPictureParam, true);
    estimateLcuControl(alfPictureParam);
    decideAlfPictureParam(alfPictureParam, false);    
  }
}

Void TEncAdaptiveLoopFilter::decideAlfPictureParam(ALFParam** alfPictureParam, Bool useAllLCUs)
{
  AlfCorrData*** lcuCorr = (m_alfLowLatencyEncoding?m_alfNonSkippedCorr:m_alfCorr);
  for(Int compIdx =0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    Double      lambda   = ((compIdx == ALF_Y)?m_dLambdaLuma:m_dLambdaChroma)*1.5;
    ALFParam *alfParam   = alfPictureParam[compIdx];
    AlfCorrData* picCorr = new AlfCorrData(compIdx);

    if(!useAllLCUs)
    {
      Int numStatLCU =0;
      for(Int addr=0; addr < m_uiNumCUsInFrame; addr++)
      {
        if(m_pcPic->getCU(addr)->getAlfLCUEnabled(compIdx))
        {
          numStatLCU++;
          break;
        }
      }
      if(numStatLCU ==0)
      {
        useAllLCUs = true;
      }
    }

    for(Int addr=0; addr< (Int)m_uiNumCUsInFrame; addr++)
    {
      if(useAllLCUs || (m_pcPic->getCU(addr)->getAlfLCUEnabled(compIdx)) )
      {
        *picCorr += *(lcuCorr[compIdx][addr]);
      }
    }

    Int64 offDist = estimateFilterDistortion(compIdx, picCorr);
    alfParam->alf_flag = 1;
    deriveFilterInfo(compIdx, picCorr, alfParam, NO_VAR_BINS, lambda);
    Int64 dist = estimateFilterDistortion(compIdx, picCorr, m_filterCoeffSym, alfParam->filters_per_group, m_varIndTab, false);
    UInt rate = ALFParamBitrateEstimate(alfParam);
    Double cost = dist - offDist + lambda * rate;
    if(cost >= 0)
    {
      alfParam->alf_flag = 0;
    }
    delete picCorr;
  }
}

Void TEncAdaptiveLoopFilter::estimateLcuControl(ALFParam** alfPictureParam)
{
  for(Int compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    if(alfPictureParam[compIdx]->alf_flag == 1)
    {
      reconstructCoefInfo(compIdx, alfPictureParam[compIdx], m_filterCoeffSym, m_varIndTab);
      for(Int addr = 0; addr < m_uiNumCUsInFrame; addr++)
      {
        Int64 offDist = estimateFilterDistortion(compIdx, m_alfCorr[compIdx][addr]);
        Int64 dist    = estimateFilterDistortion(compIdx, m_alfCorr[compIdx][addr], m_filterCoeffSym, alfPictureParam[compIdx]->filters_per_group, m_varIndTab, false);
        m_pcPic->getCU(addr)->setAlfLCUEnabled( ((offDist <= dist)?0:1) , compIdx);
      }
    }
  }
}

#endif


/** Decide ALF parameter set for luma/chroma components (top function) 
 * \param [in] pPicOrg picture buffer for original picture
 * \param [in] pPicDec picture buffer for un-filtered picture 
 * \param [out] pPicRest picture buffer for reconstructed picture
 * \param [in, out] alfParamSet ALF parameter set
 * \param [in, out] alfCtrlParam ALF CU-on/off control parameters
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::decideParameters(ALFParam** alfPictureParam, TComPicYuv* pPicOrg, TComPicYuv* pPicDec, TComPicYuv* pPicRest)
#else
Void TEncAdaptiveLoopFilter::decideParameters(TComPicYuv* pPicOrg, TComPicYuv* pPicDec, TComPicYuv* pPicRest
                                            , AlfParamSet* alfParamSet
                                            , std::vector<AlfCUCtrlInfo>* alfCtrlParam)
#endif
{
  static Int lumaStride        = pPicOrg->getStride();
  static Int chromaStride      = pPicOrg->getCStride();

  Pel *pOrg, *pDec, *pRest;
  Int stride, formatShift;

#if AHG6_ALF_OPTION2
  setCurAlfParam(alfPictureParam);
#endif
  for(Int compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    pOrg        = getPicBuf(pPicOrg, compIdx);
    pDec        = getPicBuf(pPicDec, compIdx);
    pRest       = getPicBuf(pPicRest, compIdx);
    stride      = (compIdx == ALF_Y)?(lumaStride):(chromaStride);
    formatShift = (compIdx == ALF_Y)?(0):(1);

    AlfCorrData** alfCorrComp     = m_alfCorr[compIdx];
#if AHG6_ALF_OPTION2
    executeLCUOnOffDecision(compIdx, alfPictureParam[compIdx], pOrg, pDec, pRest, stride, formatShift, alfCorrComp);
#else
    if(!m_picBasedALFEncode) //lcu-based optimization
    {
      executeLCUBasedModeDecision(alfParamSet, compIdx, pOrg, pDec, pRest, stride, formatShift, alfCorrComp);
    }
    else //picture-based optimization
    {
      AlfPicQTPart* alfPicQTPart = m_alfPQTPart[compIdx];
      executePicBasedModeDecision(alfParamSet, alfPicQTPart, compIdx, pOrg, pDec, pRest, stride, formatShift, alfCorrComp);
    }  
#endif
  } //component

}

/** Gather correlations for all LCUs in picture
 * \param [in] pPicOrg picture buffer for original picture
 * \param [in] pPicDec picture buffer for un-filtered picture 
 */
Void TEncAdaptiveLoopFilter::getStatistics(TComPicYuv* pPicOrg, TComPicYuv* pPicDec)
{
  Int lumaStride   = pPicOrg->getStride();
  Int chromaStride = pPicOrg->getCStride();
  const  Int chromaFormatShift = 1;
#if !AHG6_ALF_OPTION2
  //calculate BA index
  calcOneRegionVar(m_varImg, getPicBuf(pPicDec, ALF_Y), lumaStride, false, 0, m_img_height, 0, m_img_width);
#endif
  for(Int compIdx = 0; compIdx < NUM_ALF_COMPONENT; compIdx++)
  {
    AlfCorrData** alfCorrComp = m_alfCorr[compIdx];
    Int          formatShift = (compIdx == ALF_Y)?(0):(chromaFormatShift);
    Int          stride      = (compIdx == ALF_Y)?(lumaStride):(chromaStride);
#if AHG6_ALF_OPTION2
    getOneCompStatistics(alfCorrComp, compIdx, getPicBuf(pPicOrg, compIdx), getPicBuf(pPicDec, compIdx), stride, formatShift);
#else
    getOneCompStatistics(alfCorrComp, compIdx, getPicBuf(pPicOrg, compIdx), getPicBuf(pPicDec, compIdx), stride, formatShift, false);
#endif
  } 
}

/** Gather correlations for all LCUs of one luma/chroma component in picture
 * \param [out] alfCorrComp correlations for LCUs
 * \param [in] compIdx luma/chroma component index
 * \param [in] imgOrg picture buffer for original picture
 * \param [in] imgDec picture buffer for un-filtered picture 
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 * \param [in] isRedesignPhase at re-design filter stage (true) or not (false)
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::getOneCompStatistics(AlfCorrData** alfCorrComp, Int compIdx, Pel* imgOrg, Pel* imgDec, Int stride, Int formatShift)
#else
Void TEncAdaptiveLoopFilter::getOneCompStatistics(AlfCorrData** alfCorrComp, Int compIdx, Pel* imgOrg, Pel* imgDec, Int stride, Int formatShift, Bool isRedesignPhase)
#endif
{

  // initialize to zero
  for(Int n=0; n< m_uiNumCUsInFrame; n++)
  {
    alfCorrComp[n]->reset();
#if AHG6_ALF_OPTION2
    if(m_alfLowLatencyEncoding)
    {
      m_alfNonSkippedCorr[compIdx][n]->reset();
    }
#endif
  }

  for(Int s=0; s<= m_lastSliceIdx; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }
    Int numTilesInSlice = (Int)m_pvpSliceTileAlfLCU[s].size();
    for(Int t=0; t< numTilesInSlice; t++)
    {
      std::vector<AlfLCUInfo*> & vpAlfLCU = m_pvpSliceTileAlfLCU[s][t];
      Pel* pSrc = imgDec;

      if(m_bUseNonCrossALF)
      {
        pSrc = getPicBuf(m_pcSliceYuvTmp, compIdx);
        copyRegion(vpAlfLCU, pSrc, imgDec, stride, formatShift);
        extendRegionBorder(vpAlfLCU, pSrc, stride, formatShift);
      }

      Int numLCUs = (Int)vpAlfLCU.size();
      for(Int n=0; n< numLCUs; n++)
      {
        AlfLCUInfo* alfLCU = vpAlfLCU[n];
        Int addr = alfLCU->pcCU->getAddr();
#if AHG6_ALF_OPTION2
        getStatisticsOneLCU(m_alfLowLatencyEncoding, compIdx, alfLCU, alfCorrComp[addr], imgOrg, pSrc, stride, formatShift);
        if(m_alfLowLatencyEncoding)
        {
          getStatisticsOneLCU( false, compIdx, alfLCU, m_alfNonSkippedCorr[compIdx][addr], imgOrg, pSrc, stride, formatShift);
        }
#else
        getStatisticsOneLCU(!m_picBasedALFEncode, compIdx, alfLCU, alfCorrComp[addr], imgOrg, pSrc, stride, formatShift, isRedesignPhase);
#endif
      } //LCU
    } //tile
  } //slice

}

/** Gather correlations for one LCU
 * \param [out] alfCorrComp correlations for LCUs
 * \param [in] compIdx luma/chroma component index
 * \param [in] imgOrg picture buffer for original picture
 * \param [in] imgDec picture buffer for un-filtered picture 
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] formatShift 0 for luma and 1 for chroma (4:2:0)
 * \param [in] isRedesignPhase at re-design filter stage (true) or not (false)
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::getStatisticsOneLCU(Bool skipLCUBottomLines, Int compIdx, AlfLCUInfo* alfLCU, AlfCorrData* alfCorr, Pel* pPicOrg, Pel* pPicSrc, Int stride, Int formatShift)
#else
Void TEncAdaptiveLoopFilter::getStatisticsOneLCU(Bool skipLCUBottomLines, Int compIdx, AlfLCUInfo* alfLCU, AlfCorrData* alfCorr, Pel* pPicOrg, Pel* pPicSrc, Int stride, Int formatShift, Bool isRedesignPhase)
#endif
{
  Int numBlocks = alfLCU->numSGU;
#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER || LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
  Int  lcuAddr = alfLCU->pcCU->getAddr();
  Bool notSkipLinesBelowVB = true;
  Int  endypos;
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
  Bool notSkipLinesRightVB = true;
  Int endxpos;
#endif
  Bool isLastBlock;
  Int ypos, xpos, height, width;

#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER
  if(skipLCUBottomLines)
  {
    if(lcuAddr + m_numLCUInPicWidth < m_uiNumCUsInFrame)
    {
      notSkipLinesBelowVB = false;
    }
  }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
  if(skipLCUBottomLines)
  {
    if((lcuAddr + 1) % m_numLCUInPicWidth != 0 )
    {
      notSkipLinesRightVB = false;
    }
  }
#endif
  switch(compIdx)
  {
  case ALF_Cb:
  case ALF_Cr:
    {
      for(Int n=0; n< numBlocks; n++)
      {
        isLastBlock = (n== numBlocks-1);
        NDBFBlockInfo& AlfSGU = (*alfLCU)[n];

        ypos   = (Int)(AlfSGU.posY  >> formatShift);
        xpos   = (Int)(AlfSGU.posX  >> formatShift);
        height = (Int)(AlfSGU.height>> formatShift);
        width  = (Int)(AlfSGU.width >> formatShift);

#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER
        if(!notSkipLinesBelowVB )
        {
          endypos = ypos+ height -1;
          Int iLineVBPos = m_lcuHeightChroma - 2;
          Int yEndLineInLCU = endypos % m_lcuHeightChroma;
          height = (yEndLineInLCU >= iLineVBPos) ? (height - 2) : height ; 
        }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
        if( !notSkipLinesRightVB)
        {
          endxpos = xpos+ width -1;
          Int iLineVBPos = m_lcuWidthChroma - 7;
          Int xEndLineInLCU = endxpos % m_lcuWidthChroma;
          width = (xEndLineInLCU >= iLineVBPos) ? (width - 7) : width ; 
        }
#endif
        calcCorrOneCompRegionChma(pPicOrg, pPicSrc, stride, ypos, xpos, height, width, alfCorr->ECorr[0], alfCorr->yCorr[0], isLastBlock);
      }
    }
    break;
  case ALF_Y:
    {
#if !AHG6_ALF_OPTION2
      Bool forceCollection = true;

      if(isRedesignPhase)
      {
        Int numValidPels = 0;
        for(Int n=0; n< numBlocks; n++)
        {
          NDBFBlockInfo& AlfSGU = (*alfLCU)[n];

          ypos   = (Int)(AlfSGU.posY  );
          xpos   = (Int)(AlfSGU.posX  );
          height = (Int)(AlfSGU.height);
          width  = (Int)(AlfSGU.width );

          for (Int y = ypos; y < ypos+ height; y++)
          {
            for (Int x = xpos; x < xpos + width; x++)
            {
              if (m_maskImg[y][x] == 1)
              {
                numValidPels++;
              }
            }
          }
        }

        if(numValidPels > 0)
        {
          forceCollection = false;
        }
      }
#endif
      for(Int n=0; n< numBlocks; n++)
      {
        isLastBlock = (n== numBlocks-1);
        NDBFBlockInfo& AlfSGU = (*alfLCU)[n];

        ypos   = (Int)(AlfSGU.posY  );
        xpos   = (Int)(AlfSGU.posX  );
        height = (Int)(AlfSGU.height);
        width  = (Int)(AlfSGU.width );

#if LCUALF_AVOID_USING_BOTTOM_LINES_ENCODER
        endypos = ypos+ height -1;
        if(!notSkipLinesBelowVB)
        {
          Int iLineVBPos = m_lcuHeight - 4;
          Int yEndLineInLCU = endypos % m_lcuHeight;
          height = (yEndLineInLCU >= iLineVBPos) ? (height - 4) : height ; 
        }
#endif
#if LCUALF_AVOID_USING_RIGHT_LINES_ENCODER
        if( !notSkipLinesRightVB)
        {
          endxpos = xpos+ width -1;
          Int iLineVBPos = m_lcuWidth - 9;
          Int xEndLineInLCU = endxpos % m_lcuWidth;
          width = (xEndLineInLCU >= iLineVBPos) ? (width - 9) : width ; 
        }
#endif
#if AHG6_ALF_OPTION2
        calcCorrOneCompRegionLuma(pPicOrg, pPicSrc, stride, ypos, xpos, height, width, alfCorr->ECorr, alfCorr->yCorr, alfCorr->pixAcc, isLastBlock);
#else
        calcCorrOneCompRegionLuma(pPicOrg, pPicSrc, stride, ypos, xpos, height, width, alfCorr->ECorr, alfCorr->yCorr, alfCorr->pixAcc, forceCollection, isLastBlock);
#endif
      }
    }
    break;
  default:
    {
      printf("Not a legal component index for ALF\n");
      assert(0);
      exit(-1);
    }
  }
}


/** Gather correlations for one region for chroma component
 * \param [in] imgOrg picture buffer for original picture
 * \param [in] imgPad picture buffer for un-filtered picture 
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] yPos region starting y position
 * \param [in] xPos region starting x position
 * \param [in] height region height
 * \param [in] width region width
 * \param [out] eCorr auto-correlation matrix
 * \param [out] yCorr cross-correlation array
 * \param [in] isSymmCopyBlockMatrix symmetrically copy correlation values in eCorr (true) or not (false)
 */
Void TEncAdaptiveLoopFilter::calcCorrOneCompRegionChma(Pel* imgOrg, Pel* imgPad, Int stride 
                                                     , Int yPos, Int xPos, Int height, Int width
                                                     , Double **eCorr, Double *yCorr, Bool isSymmCopyBlockMatrix
                                                      )
{
  Int yPosEnd = yPos + height;
  Int xPosEnd = xPos + width;
  Int N = ALF_MAX_NUM_COEF; //m_sqrFiltLengthTab[0];

  Int imgHeightChroma = m_img_height>>1;

  Int yLineInLCU, paddingLine;
  Int ELocal[ALF_MAX_NUM_COEF];
  Pel *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;
  Int i, j, k, l, yLocal;

  imgPad += (yPos*stride);
  imgOrg += (yPos*stride);

  for (i= yPos; i< yPosEnd; i++)
  {
    yLineInLCU = i % m_lcuHeightChroma;

    if (yLineInLCU==0 && i>0)
    {
      paddingLine = yLineInLCU + 2 ;
      imgPad1 = imgPad + stride;
      imgPad2 = imgPad - stride;
      imgPad3 = imgPad + 2*stride;
      imgPad4 = imgPad - 2*stride;
      imgPad5 = (paddingLine < 3) ? imgPad : imgPad + 3*stride;
      imgPad6 = (paddingLine < 3) ? imgPad : imgPad - min(paddingLine, 3)*stride;;
    }
    else if (yLineInLCU < m_lineIdxPadBotChroma || i-yLineInLCU+m_lcuHeightChroma >= imgHeightChroma )
    {
      imgPad1 = imgPad + stride;
      imgPad2 = imgPad - stride;
      imgPad3 = imgPad + 2*stride;
      imgPad4 = imgPad - 2*stride;
      imgPad5 = imgPad + 3*stride;
      imgPad6 = imgPad - 3*stride;
    }
    else if (yLineInLCU < m_lineIdxPadTopChroma)
    {
      paddingLine = - yLineInLCU + m_lineIdxPadTopChroma - 1;
      imgPad1 = (paddingLine < 1) ? imgPad : imgPad + min(paddingLine, 1)*stride;
      imgPad2 = (paddingLine < 1) ? imgPad : imgPad - stride;
      imgPad3 = (paddingLine < 2) ? imgPad : imgPad + min(paddingLine, 2)*stride;
      imgPad4 = (paddingLine < 2) ? imgPad : imgPad - 2*stride;
      imgPad5 = (paddingLine < 3) ? imgPad : imgPad + min(paddingLine, 3)*stride;
      imgPad6 = (paddingLine < 3) ? imgPad : imgPad - 3*stride;
    }
    else
    {
      paddingLine = yLineInLCU - m_lineIdxPadTopChroma ;
      imgPad1 = (paddingLine < 1) ? imgPad : imgPad + stride;
      imgPad2 = (paddingLine < 1) ? imgPad : imgPad - min(paddingLine, 1)*stride;
      imgPad3 = (paddingLine < 2) ? imgPad : imgPad + 2*stride;
      imgPad4 = (paddingLine < 2) ? imgPad : imgPad - min(paddingLine, 2)*stride;
      imgPad5 = (paddingLine < 3) ? imgPad : imgPad + 3*stride;
      imgPad6 = (paddingLine < 3) ? imgPad : imgPad - min(paddingLine, 3)*stride;
    }

    for (j= xPos; j< xPosEnd; j++)
    {
      memset(ELocal, 0, N*sizeof(Int));

      ELocal[0] = (imgPad5[j] + imgPad6[j]);

      ELocal[1] = (imgPad3[j] + imgPad4[j]);

      ELocal[2] = (imgPad1[j+1] + imgPad2[j-1]);
      ELocal[3] = (imgPad1[j  ] + imgPad2[j  ]);
      ELocal[4] = (imgPad1[j-1] + imgPad2[j+1]);

      ELocal[5] = (imgPad[j+4] + imgPad[j-4]);
      ELocal[6] = (imgPad[j+3] + imgPad[j-3]);
      ELocal[7] = (imgPad[j+2] + imgPad[j-2]);
      ELocal[8] = (imgPad[j+1] + imgPad[j-1]);
      ELocal[9] = (imgPad[j  ]);

      yLocal= (Int)imgOrg[j];

      for(k=0; k<N; k++)
      {
        eCorr[k][k] += ELocal[k]*ELocal[k];
        for(l=k+1; l<N; l++)
        {
          eCorr[k][l] += ELocal[k]*ELocal[l];
        }

        yCorr[k] += yLocal*ELocal[k];
      }
    }

    imgPad+= stride;
    imgOrg+= stride;
  }

  if(isSymmCopyBlockMatrix)
  {
    for(j=0; j<N-1; j++)
    {
      for(i=j+1; i<N; i++)
      {
        eCorr[i][j] = eCorr[j][i];
      }
    }
  }
}

/** Gather correlations for one region for luma component
 * \param [in] imgOrg picture buffer for original picture
 * \param [in] imgPad picture buffer for un-filtered picture 
 * \param [in] stride buffer stride size for 1-D pictrue memory
 * \param [in] yPos region starting y position
 * \param [in] xPos region starting x position
 * \param [in] height region height
 * \param [in] width region width
 * \param [out] eCorr auto-correlation matrix
 * \param [out] yCorr cross-correlation array
 * \param [out] pixAcc pixel squared value
 * \param [in] isforceCollection all pixel are used for correlation calculation (true) or not (false)
 * \param [in] isSymmCopyBlockMatrix symmetrically copy correlation values in eCorr (true) or not (false)
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::calcCorrOneCompRegionLuma(Pel* imgOrg, Pel* imgPad, Int stride,Int yPos, Int xPos, Int height, Int width
                                                      ,Double ***eCorr, Double **yCorr, Double *pixAcc
                                                      ,Bool isSymmCopyBlockMatrix
                                                      )

#else
Void TEncAdaptiveLoopFilter::calcCorrOneCompRegionLuma(Pel* imgOrg, Pel* imgPad, Int stride
                                                      ,Int yPos, Int xPos, Int height, Int width
                                                      ,Double ***eCorr, Double **yCorr, Double *pixAcc
                                                      ,Bool isforceCollection, Bool isSymmCopyBlockMatrix
                                                      )
#endif
{
  Int yPosEnd = yPos + height;
  Int xPosEnd = xPos + width;
  Int yLineInLCU;
  Int paddingLine ;
  Int N = ALF_MAX_NUM_COEF; //m_sqrFiltLengthTab[0];

  Int ELocal[ALF_MAX_NUM_COEF];
  Pel *imgPad1, *imgPad2, *imgPad3, *imgPad4, *imgPad5, *imgPad6;
  Int i, j, k, l, yLocal, varInd;
  Double **E;
  Double *yy;

  imgPad += (yPos*stride);
  imgOrg += (yPos*stride);

  for (i= yPos; i< yPosEnd; i++)
  {
    yLineInLCU = i % m_lcuHeight;

    if (yLineInLCU<m_lineIdxPadBot || i-yLineInLCU+m_lcuHeight >= m_img_height)
    {
      imgPad1 = imgPad + stride;
      imgPad2 = imgPad - stride;
      imgPad3 = imgPad + 2*stride;
      imgPad4 = imgPad - 2*stride;
      imgPad5 = imgPad + 3*stride;
      imgPad6 = imgPad - 3*stride;
    }
    else if (yLineInLCU<m_lineIdxPadTop)
    {
      paddingLine = - yLineInLCU + m_lineIdxPadTop - 1;
      imgPad1 = (paddingLine < 1) ? imgPad : imgPad + min(paddingLine, 1)*stride;
      imgPad2 = (paddingLine < 1) ? imgPad : imgPad - stride;
      imgPad3 = (paddingLine < 2) ? imgPad : imgPad + min(paddingLine, 2)*stride;
      imgPad4 = (paddingLine < 2) ? imgPad : imgPad - 2*stride;
      imgPad5 = (paddingLine < 3) ? imgPad : imgPad + min(paddingLine, 3)*stride;
      imgPad6 = (paddingLine < 3) ? imgPad : imgPad - 3*stride;
    }
    else
    {
      paddingLine = yLineInLCU - m_lineIdxPadTop;
      imgPad1 = (paddingLine < 1) ? imgPad : imgPad + stride;
      imgPad2 = (paddingLine < 1) ? imgPad : imgPad - min(paddingLine, 1)*stride;
      imgPad3 = (paddingLine < 2) ? imgPad : imgPad + 2*stride;
      imgPad4 = (paddingLine < 2) ? imgPad : imgPad - min(paddingLine, 2)*stride;
      imgPad5 = (paddingLine < 3) ? imgPad : imgPad + 3*stride;
      imgPad6 = (paddingLine < 3) ? imgPad : imgPad - min(paddingLine, 3)*stride;
    }         

    for (j= xPos; j< xPosEnd; j++)
    {
#if !AHG6_ALF_OPTION2
      if ( m_maskImg[i][j] || isforceCollection )
      {
#endif
        varInd = m_varImg[i/VAR_SIZE_H][j/VAR_SIZE_W];
        memset(ELocal, 0, N*sizeof(Int));

        ELocal[0] = (imgPad5[j] + imgPad6[j]);
        ELocal[1] = (imgPad3[j] + imgPad4[j]);

        ELocal[2] = (imgPad1[j+1] + imgPad2[j-1]);
        ELocal[3] = (imgPad1[j  ] + imgPad2[j  ]);
        ELocal[4] = (imgPad1[j-1] + imgPad2[j+1]);

        ELocal[5] = (imgPad[j+4] + imgPad[j-4]);
        ELocal[6] = (imgPad[j+3] + imgPad[j-3]);
        ELocal[7] = (imgPad[j+2] + imgPad[j-2]);
        ELocal[8] = (imgPad[j+1] + imgPad[j-1]);
        ELocal[9] = (imgPad[j  ]);

        yLocal= imgOrg[j];
        pixAcc[varInd] += (yLocal*yLocal);
        E  = eCorr[varInd];
        yy = yCorr[varInd];

        for (k=0; k<N; k++)
        {
          for (l=k; l<N; l++)
          {
            E[k][l]+=(double)(ELocal[k]*ELocal[l]);
          }
          yy[k]+=(double)(ELocal[k]*yLocal);
        }
#if !AHG6_ALF_OPTION2
      }
#endif
    }
    imgPad += stride;
    imgOrg += stride;
  }

  if(isSymmCopyBlockMatrix)
  {
    for (varInd=0; varInd<NO_VAR_BINS; varInd++)
    {
      E = eCorr[varInd];
      for (k=1; k<N; k++)
      {
        for (l=0; l<k; l++)
        {
          E[k][l] = E[l][k];
        }
      }
    }
  }

}

/** PCM LF disable process.
 * \param pcPic picture (TComPic) pointer
 * \returns Void
 *
 * \note Replace filtered sample values of PCM mode blocks with the transmitted and reconstructed ones.
 */
Void TEncAdaptiveLoopFilter::PCMLFDisableProcess (TComPic* pcPic)
{
  xPCMRestoration(pcPic);
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

// ====================================================================================================================
// Private member functions
// ====================================================================================================================
#if !AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::xCreateTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->createTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xDestroyTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->destroyTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsTo()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagFromTmp();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsFrom()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagToTmp();
  }
}

/** Encode ALF CU control flags
 */
Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlags(std::vector<AlfCUCtrlInfo> &vAlfCUCtrlParam)
{
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }

    AlfCUCtrlInfo& rCUCtrlInfo = vAlfCUCtrlParam[s];
    if(rCUCtrlInfo.cu_control_flag == 1)
    {
      for(Int i=0; i< (Int)rCUCtrlInfo.alf_cu_flag.size(); i++)
      {
        m_pcEntropyCoder->encodeAlfCtrlFlag(rCUCtrlInfo.alf_cu_flag[i]);
      }
    }
  }
}
Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
#if AD_HOCS_SLICES  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
#else  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
#endif  
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
#if AD_HOCS_SLICES      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
#else
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
#endif      
        xEncodeCUAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }
  
  m_pcEntropyCoder->encodeAlfCtrlFlag(pcCU, uiAbsPartIdx);
}
#endif

#if IBDI_DISTORTION
UInt64 TEncAdaptiveLoopFilter::xCalcSSD(Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride )
{
  UInt64 uiSSD = 0;
  Int x, y;

  Int iShift = g_uiBitIncrement;
  Int iOffset = (g_uiBitIncrement>0)? (1<<(g_uiBitIncrement-1)):0;
  Int iTemp;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = ((pOrg[x]+iOffset)>>iShift) - ((pCmp[x]+iOffset)>>iShift); uiSSD += iTemp * iTemp;
    }
    pOrg += iStride;
    pCmp += iStride;
  }

  return uiSSD;;
}
#else
UInt64 TEncAdaptiveLoopFilter::xCalcSSD(Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride )
{
  UInt64 uiSSD = 0;
  Int x, y;
  
  UInt uiShift = g_uiBitIncrement<<1;
  Int iTemp;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pOrg[x] - pCmp[x]; uiSSD += ( iTemp * iTemp ) >> uiShift;
    }
    pOrg += iStride;
    pCmp += iStride;
  }
  
  return uiSSD;;
}
#endif

Int TEncAdaptiveLoopFilter::xGauss(Double **a, Int N)
{
  Int i, j, k;
  Double t;
  
  for(k=0; k<N; k++)
  {
    if (a[k][k] <0.000001)
    {
      return 1;
    }
  }
  
  for(k=0; k<N-1; k++)
  {
    for(i=k+1;i<N; i++)
    {
      t=a[i][k]/a[k][k];
      for(j=k+1; j<=N; j++)
      {
        a[i][j] -= t * a[k][j];
        if(i==j && fabs(a[i][j])<0.000001) return 1;
      }
    }
  }
  for(i=N-1; i>=0; i--)
  {
    t = a[i][N];
    for(j=i+1; j<N; j++)
    {
      t -= a[i][j] * a[j][N];
    }
    a[i][N] = t / a[i][i];
  }
  return 0;
}

Void TEncAdaptiveLoopFilter::xFilterCoefQuickSort( Double *coef_data, Int *coef_num, Int upper, Int lower )
{
  Double mid, tmp_data;
  Int i, j, tmp_num;
  
  i = upper;
  j = lower;
  mid = coef_data[(lower+upper)>>1];
  do
  {
    while( coef_data[i] < mid ) i++;
    while( mid < coef_data[j] ) j--;
    if( i <= j )
    {
      tmp_data = coef_data[i];
      tmp_num  = coef_num[i];
      coef_data[i] = coef_data[j];
      coef_num[i]  = coef_num[j];
      coef_data[j] = tmp_data;
      coef_num[j]  = tmp_num;
      i++;
      j--;
    }
  } while( i <= j );
  if ( upper < j ) 
  {
    xFilterCoefQuickSort(coef_data, coef_num, upper, j);
  }
  if ( i < lower ) 
  {
    xFilterCoefQuickSort(coef_data, coef_num, i, lower);
  }
}

Void TEncAdaptiveLoopFilter::xQuantFilterCoef(Double* h, Int* qh, Int tap, int bit_depth)
{
  Int i, N;
  Int max_value, min_value;
  Double dbl_total_gain;
  Int total_gain, q_total_gain;
  Int upper, lower;
  Double *dh;
  Int    *nc;
  const Int    *pFiltMag;
#if AHG6_ALF_OPTION2
  N = (Int)ALF_MAX_NUM_COEF;
#else
#if LCUALF_QP_DEPENDENT_BITS
  Int alfPrecisionBit = getAlfPrecisionBit( m_alfQP );
#endif

  N = m_sqrFiltLengthTab[tap];
#endif
  pFiltMag = weightsShape1Sym;

  dh = new Double[N];
  nc = new Int[N];
#if AHG6_ALF_OPTION2
  max_value =   (1<<(1+ALF_NUM_BIT_SHIFT))-1;
  min_value = 0-(1<<(1+ALF_NUM_BIT_SHIFT));
#else  
#if LCUALF_QP_DEPENDENT_BITS  
  max_value =   (1<<(1+alfPrecisionBit))-1;
  min_value = 0-(1<<(1+alfPrecisionBit));
#else
  max_value =   (1<<(1+ALF_NUM_BIT_SHIFT))-1;
  min_value = 0-(1<<(1+ALF_NUM_BIT_SHIFT));
#endif
#endif
  dbl_total_gain=0.0;
  q_total_gain=0;
  for(i=0; i<N; i++)
  {
    if(h[i]>=0.0)
    {
#if AHG6_ALF_OPTION2
      qh[i] =  (Int)( h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#else
#if LCUALF_QP_DEPENDENT_BITS
      qh[i] =  (Int)( h[i]*(1<<alfPrecisionBit)+0.5);
#else
      qh[i] =  (Int)( h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#endif
#endif
    }
    else
    {
#if AHG6_ALF_OPTION2
      qh[i] = -(Int)(-h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#else
#if LCUALF_QP_DEPENDENT_BITS
      qh[i] = -(Int)(-h[i]*(1<<alfPrecisionBit)+0.5);
#else
      qh[i] = -(Int)(-h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#endif
#endif
    }
#if AHG6_ALF_OPTION2
    dh[i] = (Double)qh[i]/(Double)(1<<ALF_NUM_BIT_SHIFT) - h[i];
#else
#if LCUALF_QP_DEPENDENT_BITS
    dh[i] = (Double)qh[i]/(Double)(1<<alfPrecisionBit) - h[i];
#else
    dh[i] = (Double)qh[i]/(Double)(1<<ALF_NUM_BIT_SHIFT) - h[i];
#endif
#endif
    dh[i]*=pFiltMag[i];
    dbl_total_gain += h[i]*pFiltMag[i];
    q_total_gain   += qh[i]*pFiltMag[i];
    nc[i] = i;
  }
  
  // modification of quantized filter coefficients
#if AHG6_ALF_OPTION2
  total_gain = (Int)(dbl_total_gain*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#else
#if LCUALF_QP_DEPENDENT_BITS
  total_gain = (Int)(dbl_total_gain*(1<<alfPrecisionBit)+0.5);
#else
  total_gain = (Int)(dbl_total_gain*(1<<ALF_NUM_BIT_SHIFT)+0.5);
#endif  
#endif
  if( q_total_gain != total_gain )
  {
    xFilterCoefQuickSort(dh, nc, 0, N-1);
    if( q_total_gain > total_gain )
    {
      upper = N-1;
      while( q_total_gain > total_gain+1 )
      {
        i = nc[upper%N];
        qh[i]--;
        q_total_gain -= pFiltMag[i];
        upper--;
      }
      if( q_total_gain == total_gain+1 )
      {
        if(dh[N-1]>0)
        {
          qh[N-1]--;
        }
        else
        {
          i=nc[upper%N];
          qh[i]--;
          qh[N-1]++;
        }
      }
    }
    else if( q_total_gain < total_gain )
    {
      lower = 0;
      while( q_total_gain < total_gain-1 )
      {
        i=nc[lower%N];
        qh[i]++;
        q_total_gain += pFiltMag[i];
        lower++;
      }
      if( q_total_gain == total_gain-1 )
      {
        if(dh[N-1]<0)
        {
          qh[N-1]++;
        }
        else
        {
          i=nc[lower%N];
          qh[i]++;
          qh[N-1]--;
        }
      }
    }
  }
  
  // set of filter coefficients
  for(i=0; i<N; i++)
  {
    qh[i] = max(min_value,min(max_value, qh[i]));
  }

  checkFilterCoeffValue(qh, N, true);

  delete[] dh;
  dh = NULL;
  
  delete[] nc;
  nc = NULL;
}

#if !AHG6_ALF_OPTION2
/** Restore the not-filtered pixels
 * \param [in] imgDec picture buffer before filtering
 * \param [out] imgRest picture buffer after filtering
 * \param [in] stride stride size for 1-D picture memory
 */
Void TEncAdaptiveLoopFilter::xCopyDecToRestCUs(Pel* imgDec, Pel* imgRest, Int stride)
{

  if(m_uiNumSlicesInPic > 1)
  {
    Pel* pPicDecLuma  = imgDec;
    Pel* pPicRestLuma = imgRest;
    UInt SUWidth      = m_pcPic->getMinCUWidth();
    UInt SUHeight     = m_pcPic->getMinCUHeight();

    UInt startSU, endSU, LCUX, LCUY, currSU, LPelX, TPelY;
    UInt posOffset;
    Pel *pDec, *pRest;

    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }
      std::vector< AlfLCUInfo* >&  vpSliceAlfLCU = m_pvpAlfLCU[s]; 
      for(Int i=0; i< vpSliceAlfLCU.size(); i++)
      {
        AlfLCUInfo& rAlfLCU    = *(vpSliceAlfLCU[i]);
        TComDataCU* pcCU       = rAlfLCU.pcCU;
        startSU                = rAlfLCU.startSU;
        endSU                  = rAlfLCU.endSU;
        LCUX                 = pcCU->getCUPelX();
        LCUY                 = pcCU->getCUPelY();

        for(currSU= startSU; currSU<= endSU; currSU++)
        {
          LPelX   = LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[currSU] ];
          TPelY   = LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[currSU] ];
          if( !( LPelX < m_img_width )  || !( TPelY < m_img_height )  )
          {
            continue;
          }
          if(!pcCU->getAlfCtrlFlag(currSU))
          {
            posOffset = TPelY*stride + LPelX;
            pDec = pPicDecLuma + posOffset;
            pRest= pPicRestLuma+ posOffset;
            for(Int y=0; y< SUHeight; y++)
            {
              ::memcpy(pRest, pDec, sizeof(Pel)*SUWidth);
              pDec += stride;
              pRest+= stride;
            }
          }
        }
      }
    }
    return;
  }

  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xCopyDecToRestCU(pcCU, 0, 0, imgDec, imgRest, stride);
  }
}

Void TEncAdaptiveLoopFilter::xCopyDecToRestCU(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Pel* imgDec, Pel* imgRest, Int stride)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )      
        xCopyDecToRestCU(pcCU, uiAbsPartIdx, uiDepth+1, imgDec, imgRest, stride);
    }
    return;
  }
  
  if (!pcCU->getAlfCtrlFlag(uiAbsPartIdx))
  {
    Int iWidth = pcCU->getWidth(uiAbsPartIdx);
    Int iHeight = pcCU->getHeight(uiAbsPartIdx);
    copyPixelsInOneRegion(imgRest, imgDec, stride, (Int)uiTPelY, iHeight, (Int)uiLPelX, iWidth);
  }
}
#endif

double TEncAdaptiveLoopFilter::xfindBestCoeffCodMethod(int **filterCoeffSymQuant, int filter_shape, int sqrFiltLength, int filters_per_fr, double errorForce0CoeffTab[NO_VAR_BINS][2], 
  double lambda)
{
  Int coeffBits, i;
  Double error=0, lagrangian;
#if AHG6_ALF_OPTION2
  static Bool  isFirst = true;
  static Int** coeffmulti = NULL;
  if(isFirst)
  {
    coeffmulti = new Int*[NO_VAR_BINS];
    for(Int g=0; g< NO_VAR_BINS; g++)
    {
      coeffmulti[g] = new Int[ALF_MAX_NUM_COEF];
    }
    isFirst = false;
  }

  for(Int g=0; g< filters_per_fr; g++)
  {
    for(i=0; i< sqrFiltLength; i++)
    {
      coeffmulti[g][i] = filterCoeffSymQuant[g][i];
    }
  }
  predictALFCoeff(coeffmulti, sqrFiltLength, filters_per_fr);
  //golomb encode bitrate estimation
  coeffBits = 0;
  for(Int g=0; g< filters_per_fr; g++)
  {
    coeffBits += filterCoeffBitrateEstimate(ALF_Y, coeffmulti[g]);
  }
#else
  coeffBits = xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, filters_per_fr, 
    0, m_tempALFp);
#endif
  for(i=0;i<filters_per_fr;i++)
  {
    error += errorForce0CoeffTab[i][1];
  }
  lagrangian = error + lambda * coeffBits;
  return (lagrangian);
}
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::predictALFCoeff(Int** coeff, Int numCoef, Int numFilters)
{
  for(Int g=0; g< numFilters; g++ )
  {
    Int sum=0;
    for(Int i=0; i< numCoef-1;i++)
    {
      sum += (2* coeff[g][i]);
    }

    Int pred = (1<<ALF_NUM_BIT_SHIFT) - (sum);
    coeff[g][numCoef-1] = coeff[g][numCoef-1] - pred;
  }
}
#else
/** Predict ALF luma filter coefficients. Centre coefficient is always predicted. Determines if left neighbour should be predicted.
 */
Void TEncAdaptiveLoopFilter::predictALFCoeffLumaEnc(ALFParam* pcAlfParam, Int **pfilterCoeffSym, Int filter_shape)
{
#if LCUALF_QP_DEPENDENT_BITS
  Int alfPrecisionBit = getAlfPrecisionBit( m_alfQP );
#endif
  Int sum, coeffPred, ind;
  const Int* pFiltMag = NULL;
  pFiltMag = weightsTabShapes[filter_shape];
  for(ind = 0; ind < pcAlfParam->filters_per_group; ++ind)
  {
    sum = 0;
    for(Int i = 0; i < pcAlfParam->num_coeff-2; i++)
    {
      sum +=  pFiltMag[i]*pfilterCoeffSym[ind][i];
    }

    if((pcAlfParam->predMethod==0)|(ind==0))
    {
#if LCUALF_QP_DEPENDENT_BITS
      coeffPred = ((1<<alfPrecisionBit)-sum) >> 2;
#else
      coeffPred = ((1<<ALF_NUM_BIT_SHIFT)-sum) >> 2;
#endif
    }
    else
    {
      coeffPred = (0-sum) >> 2;
    }
    if(abs(pfilterCoeffSym[ind][pcAlfParam->num_coeff-2]-coeffPred) < abs(pfilterCoeffSym[ind][pcAlfParam->num_coeff-2]))
    {
      pcAlfParam->nbSPred[ind] = 0; 
    }
    else
    {
      pcAlfParam->nbSPred[ind] = 1; 
      coeffPred = 0;
    }
    sum += pFiltMag[pcAlfParam->num_coeff-2]*pfilterCoeffSym[ind][pcAlfParam->num_coeff-2];
    pfilterCoeffSym[ind][pcAlfParam->num_coeff-2] -= coeffPred; 
    if((pcAlfParam->predMethod==0)|(ind==0))
    {
#if LCUALF_QP_DEPENDENT_BITS
      coeffPred = (1<<alfPrecisionBit)-sum;
#else
      coeffPred = (1<<ALF_NUM_BIT_SHIFT)-sum;
#endif
    }
    else
    {
      coeffPred = -sum;
    }
    pfilterCoeffSym[ind][pcAlfParam->num_coeff-1] -= coeffPred;
  }
}

Int TEncAdaptiveLoopFilter::xsendAllFiltersPPPred(int **FilterCoeffQuant, int fl, int sqrFiltLength, 
                                                  int filters_per_group, int createBistream, ALFParam* ALFp)
{
  int ind, bit_ct = 0, bit_ct0 = 0, i;
  int predMethod = 0;
  int force0 = 0;
  Int64 Newbit_ct;
  
  for(ind = 0; ind < filters_per_group; ind++)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      m_FilterCoeffQuantTemp[ind][i]=FilterCoeffQuant[ind][i];
    }
  }
  ALFp->filters_per_group = filters_per_group;
  ALFp->predMethod = 0;
  ALFp->num_coeff = sqrFiltLength;
  predictALFCoeffLumaEnc(ALFp, m_FilterCoeffQuantTemp, fl);
  Int nbFlagIntra[16];
  for(ind = 0; ind < filters_per_group; ind++)
  {
    nbFlagIntra[ind] = ALFp->nbSPred[ind];
  }
  bit_ct0 = xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group, 0);
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    if(ind == 0)
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = FilterCoeffQuant[ind][i];
    }
    else
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = FilterCoeffQuant[ind][i] - FilterCoeffQuant[ind-1][i];
    }
  }
  ALFp->predMethod = 1;
  predictALFCoeffLumaEnc(ALFp, m_diffFilterCoeffQuant, fl);
  
  if(xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, 0) >= bit_ct0)
  {
    predMethod = 0;  
    if(filters_per_group > 1)
    {
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    }
    bit_ct += xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  else
  {
    predMethod = 1;
    if(filters_per_group > 1)
    {
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    }
    bit_ct += xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  ALFp->filters_per_group = filters_per_group;
  ALFp->predMethod = predMethod;
  ALFp->num_coeff = sqrFiltLength;
  ALFp->filter_shape = fl;
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      if (predMethod) ALFp->coeffmulti[ind][i] = m_diffFilterCoeffQuant[ind][i];
      else 
      {
        ALFp->coeffmulti[ind][i] = m_FilterCoeffQuantTemp[ind][i];
      }
    }
    if(predMethod==0)
    {
      ALFp->nbSPred[ind] = nbFlagIntra[ind];
    }
  }
  m_pcEntropyCoder->codeFiltCountBit(ALFp, &Newbit_ct);
  
  //  return(bit_ct);
  return ((Int)Newbit_ct);
}

Int TEncAdaptiveLoopFilter::xcodeAuxInfo(int filters_per_fr, int varIndTab[NO_VAR_BINS], int filter_shape, ALFParam* ALFp)
{
  int i, filterPattern[NO_VAR_BINS], startSecondFilter=0, bitCt=0;
  Int64 NewbitCt;

  //send realfiltNo (tap related)
  ALFp->filter_shape = filter_shape;

  // decide startSecondFilter and filterPattern
  memset(filterPattern, 0, NO_VAR_BINS * sizeof(int)); 
  if(filters_per_fr > 1)
  {
    for(i = 1; i < NO_VAR_BINS; ++i)
    {
      if(varIndTab[i] != varIndTab[i-1])
      {
        filterPattern[i] = 1;
        startSecondFilter = i;
      }
    }
  }
  memcpy (ALFp->filterPattern, filterPattern, NO_VAR_BINS * sizeof(int));
  ALFp->startSecondFilter = startSecondFilter;

  assert(filters_per_fr>0);
  m_pcEntropyCoder->codeAuxCountBit(ALFp, &NewbitCt);

  bitCt = (int) NewbitCt;
  return(bitCt);
}

Int   TEncAdaptiveLoopFilter::xcodeFilterCoeff(int **pDiffQFilterCoeffIntPP, int fl, int sqrFiltLength, 
                                               int filters_per_group, int createBitstream)
{
  int i, k, kMin, kStart, minBits, ind, scanPos, maxScanVal, coeffVal, len = 0,
    *pDepthInt=NULL, kMinTab[MAX_SCAN_VAL], bitsCoeffScan[MAX_SCAN_VAL][MAX_EXP_GOLOMB],
  minKStart, minBitsKStart, bitsKStart;
  Int minScanVal = MIN_SCAN_POS_CROSS;
  pDepthInt = pDepthIntTabShapes[fl];
  
  maxScanVal = 0;
  for(i = 0; i < sqrFiltLength; i++)
  {
    maxScanVal = max(maxScanVal, pDepthInt[i]);
  }
  
  // vlc for all
  memset(bitsCoeffScan, 0, MAX_SCAN_VAL * MAX_EXP_GOLOMB * sizeof(int));
  for(ind=0; ind<filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {     
      scanPos=pDepthInt[i]-1;
      coeffVal=abs(pDiffQFilterCoeffIntPP[ind][i]);
      for (k=1; k<15; k++)
      {
        bitsCoeffScan[scanPos][k]+=lengthGolomb(coeffVal, k);
      }
    }
  }
  
  minBitsKStart = 0;
  minKStart = -1;
  for(k = 1; k < 8; k++)
  { 
    bitsKStart = 0; 
    kStart = k;
    for(scanPos = minScanVal; scanPos < maxScanVal; scanPos++)
    {
      kMin = kStart; 
      minBits = bitsCoeffScan[scanPos][kMin];
      
      if(bitsCoeffScan[scanPos][kStart+1] < minBits)
      {
        kMin = kStart + 1; 
        minBits = bitsCoeffScan[scanPos][kMin];
      }
      kStart = kMin;
      bitsKStart += minBits;
    }
    if((bitsKStart < minBitsKStart) || (k == 1))
    {
      minBitsKStart = bitsKStart;
      minKStart = k;
    }
  }
  
  kStart = minKStart; 
  for(scanPos = minScanVal; scanPos < maxScanVal; scanPos++)
  {
    kMin = kStart; 
    minBits = bitsCoeffScan[scanPos][kMin];
    
    if(bitsCoeffScan[scanPos][kStart+1] < minBits)
    {
      kMin = kStart + 1; 
      minBits = bitsCoeffScan[scanPos][kMin];
    }
    
    kMinTab[scanPos] = kMin;
    kStart = kMin;
  }
  
  // Coding parameters
  //  len += lengthFilterCodingParams(minKStart, maxScanVal, kMinTab, createBitstream);
  if (filters_per_group == 1)
  {
    len += lengthFilterCoeffs(sqrFiltLength, filters_per_group, pDepthInt, pDiffQFilterCoeffIntPP, 
      kTableTabShapes[ALF_CROSS9x7_SQUARE3x3], createBitstream);
  }
  else
  {
  len += (3 + maxScanVal);
  
  // Filter coefficients
  len += lengthFilterCoeffs(sqrFiltLength, filters_per_group, pDepthInt, pDiffQFilterCoeffIntPP, 
                            kMinTab, createBitstream);
  }

  return len;
}

Int TEncAdaptiveLoopFilter::lengthGolomb(int coeffVal, int k)
{
  int m = 2 << (k - 1);
  int q = coeffVal / m;
  if(coeffVal != 0)
  {
    return(q + 2 + k);
  }
  else
  {
    return(q + 1 + k);
  }
}

Int TEncAdaptiveLoopFilter::lengthPredFlags(int force0, int predMethod, int codedVarBins[NO_VAR_BINS], 
                                            int filters_per_group, int createBitstream)
{
  int bit_cnt = 0;
  
  if(force0)
  {
    bit_cnt = 2 + filters_per_group;
  }
  else
  {
    bit_cnt = 2;
  }
  return bit_cnt;
  
}
//important
Int TEncAdaptiveLoopFilter::lengthFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[], 
                                               int **FilterCoeff, int kMinTab[], int createBitstream)
{
  int ind, scanPos, i;
  int bit_cnt = 0;
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      scanPos = pDepthInt[i] - 1;
      Int k = (filters_per_group == 1) ? kMinTab[i] : kMinTab[scanPos];
      bit_cnt += lengthGolomb(abs(FilterCoeff[ind][i]), k);
    }
  }
  return bit_cnt;
}

#endif

#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::xfindBestFilterVarPred(double **ySym, double ***ESym, double *pixAcc, Int **filterCoeffSym, Int *filters_per_fr_best, Int varIndTab[], double lambda_val, Int numMaxFilters)
#else
Void TEncAdaptiveLoopFilter::xfindBestFilterVarPred(double **ySym, double ***ESym, double *pixAcc, Int **filterCoeffSym, Int **filterCoeffSymQuant, Int filter_shape, Int *filters_per_fr_best, Int varIndTab[], Pel **imgY_rec, Pel **varImg, Pel **maskImg, Pel **imgY_pad, double lambda_val, Int numMaxFilters)
#endif
{
#if AHG6_ALF_OPTION2
  static Bool isFirst = true;
  static Int* filterCoeffSymQuant[NO_VAR_BINS];
  if(isFirst)
  {
    for(Int g=0; g< NO_VAR_BINS; g++)
    {
      filterCoeffSymQuant[g] = new Int[ALF_MAX_NUM_COEF];
    }
    isFirst = false;
  }
  Int filter_shape = 0;
#endif
  Int filters_per_fr, firstFilt, interval[NO_VAR_BINS][2], intervalBest[NO_VAR_BINS][2];
  int i;
  double  lagrangian, lagrangianMin;
  int sqrFiltLength;
  int *weights;
#if !AHG6_ALF_OPTION2
  Int coeffBits;
#endif
  double errorForce0CoeffTab[NO_VAR_BINS][2];
  
#if AHG6_ALF_OPTION2
  sqrFiltLength= (Int)ALF_MAX_NUM_COEF;
  weights = weightsShape1Sym;
#else
  sqrFiltLength= m_sqrFiltLengthTab[filter_shape] ;
  weights = weightsTabShapes[filter_shape];
#endif
  // zero all variables 
  memset(varIndTab,0,sizeof(int)*NO_VAR_BINS);

  for(i = 0; i < NO_VAR_BINS; i++)
  {
    memset(filterCoeffSym[i],0,sizeof(int)*ALF_MAX_NUM_COEF);
    memset(filterCoeffSymQuant[i],0,sizeof(int)*ALF_MAX_NUM_COEF);
  }

  firstFilt=1;  lagrangianMin=0;
  filters_per_fr=NO_FILTERS;

  while(filters_per_fr>=1)
  {
    mergeFiltersGreedy(ySym, ESym, pixAcc, interval, sqrFiltLength, filters_per_fr);
    findFilterCoeff(ESym, ySym, pixAcc, filterCoeffSym, filterCoeffSymQuant, interval,
      varIndTab, sqrFiltLength, filters_per_fr, weights, errorForce0CoeffTab);

    lagrangian=xfindBestCoeffCodMethod(filterCoeffSymQuant, filter_shape, sqrFiltLength, filters_per_fr, errorForce0CoeffTab, lambda_val);
    if (lagrangian<lagrangianMin || firstFilt==1 || filters_per_fr == numMaxFilters)
    {
      firstFilt=0;
      lagrangianMin=lagrangian;

      (*filters_per_fr_best)=filters_per_fr;
      memcpy(intervalBest, interval, NO_VAR_BINS*2*sizeof(int));
    }
    filters_per_fr--;
  }
  findFilterCoeff(ESym, ySym, pixAcc, filterCoeffSym, filterCoeffSymQuant, intervalBest,
    varIndTab, sqrFiltLength, (*filters_per_fr_best), weights, errorForce0CoeffTab);

#if !AHG6_ALF_OPTION2
  xfindBestCoeffCodMethod(filterCoeffSymQuant, filter_shape, sqrFiltLength, (*filters_per_fr_best), errorForce0CoeffTab, lambda_val);
  coeffBits = xcodeAuxInfo((*filters_per_fr_best), varIndTab, filter_shape, m_tempALFp);
  coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, (*filters_per_fr_best), 0, m_tempALFp);
#endif
  if( *filters_per_fr_best == 1)
  {
    ::memset(varIndTab, 0, sizeof(Int)*NO_VAR_BINS);
  }
}

/** code filter coefficients
 * \param filterCoeffSymQuant filter coefficients buffer
 * \param filtNo filter No.
 * \param varIndTab[] merge index information
 * \param filters_per_fr_best the number of filters used in this picture
 * \param frNo 
 * \param ALFp ALF parameters
 * \returns bitrate
 */
#if AHG6_ALF_OPTION2
Void TEncAdaptiveLoopFilter::xcodeFiltCoeff(Int **filterCoeff, Int* varIndTab, Int numFilters, ALFParam* alfParam)
{
  Int filterPattern[NO_VAR_BINS], startSecondFilter=0;
  ::memset(filterPattern, 0, NO_VAR_BINS * sizeof(Int)); 

  alfParam->filter_shape=0;
  alfParam->num_coeff = (Int)ALF_MAX_NUM_COEF;
  alfParam->filters_per_group = numFilters;

  //merge table assignment
  if(alfParam->filters_per_group > 1)
  {
    for(Int i = 1; i < NO_VAR_BINS; ++i)
    {
      if(varIndTab[i] != varIndTab[i-1])
      {
        filterPattern[i] = 1;
        startSecondFilter = i;
      }
    }
  }
  ::memcpy (alfParam->filterPattern, filterPattern, NO_VAR_BINS * sizeof(Int));
  alfParam->startSecondFilter = startSecondFilter;

  //coefficient prediction
  for(Int g=0; g< alfParam->filters_per_group; g++)
  {
    for(Int i=0; i< alfParam->num_coeff; i++)
    {
      alfParam->coeffmulti[g][i] = filterCoeff[g][i];
    }
  }
  predictALFCoeff(alfParam->coeffmulti, alfParam->num_coeff, alfParam->filters_per_group);
}
#else

UInt TEncAdaptiveLoopFilter::xcodeFiltCoeff(Int **filterCoeffSymQuant, Int filter_shape, Int varIndTab[], Int filters_per_fr_best, ALFParam* ALFp)
{
  Int coeffBits;   
  Int sqrFiltLength = m_sqrFiltLengthTab[filter_shape] ; 

  ALFp->filters_per_group = filters_per_fr_best;

  coeffBits = xcodeAuxInfo(filters_per_fr_best, varIndTab, filter_shape, ALFp);


  ALFp->predMethod = 0;
  ALFp->num_coeff = sqrFiltLength;
  ALFp->filter_shape=filter_shape;

  if (filters_per_fr_best <= 1)
  {
    ALFp->predMethod = 0;
  }

  coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, 
    filters_per_fr_best, 1, ALFp);

  return (UInt)coeffBits;
}

Void TEncAdaptiveLoopFilter::getCtrlFlagsFromCU(AlfLCUInfo* pcAlfLCU, std::vector<UInt> *pvFlags, Int alfDepth, UInt maxNumSUInLCU)
{
  const UInt startSU               = pcAlfLCU->startSU;
  const UInt endSU                 = pcAlfLCU->endSU;
  const Bool bAllSUsInLCUInSameSlice = pcAlfLCU->bAllSUsInLCUInSameSlice;

  TComDataCU* pcCU = pcAlfLCU->pcCU;
  UInt  currSU, CUDepth, setDepth, ctrlNumSU;

  currSU = startSU;

  if(bAllSUsInLCUInSameSlice)
  {
    while(currSU < maxNumSUInLCU)
    {
      //depth of this CU
      CUDepth = pcCU->getDepth(currSU);

      //choose the min. depth for ALF
      setDepth   = (alfDepth < CUDepth)?(alfDepth):(CUDepth);
      ctrlNumSU = maxNumSUInLCU >> (setDepth << 1);

      pvFlags->push_back(pcCU->getAlfCtrlFlag(currSU));
      currSU += ctrlNumSU;
    }

    return;
  }


  const UInt  LCUX = pcCU->getCUPelX();
  const UInt  LCUY = pcCU->getCUPelY();

  Bool  bFirst, bValidCU;
  UInt  idx, LPelXSU, TPelYSU;

  bFirst= true;
  while(currSU <= endSU)
  {
    //check picture boundary
    while(!( LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[currSU] ] < m_img_width  ) || 
          !( LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[currSU] ] < m_img_height )
      )
    {
      currSU++;

      if(currSU >= maxNumSUInLCU || currSU > endSU)
      {
        break;
      }
    }

    if(currSU >= maxNumSUInLCU || currSU > endSU)
    {
      break;
    }

    //depth of this CU
    CUDepth = pcCU->getDepth(currSU);

    //choose the min. depth for ALF
    setDepth   = (alfDepth < CUDepth)?(alfDepth):(CUDepth);
    ctrlNumSU = maxNumSUInLCU >> (setDepth << 1);

    if(bFirst)
    {
      if(currSU !=0 )
      {
        currSU = ((UInt)(currSU/ctrlNumSU))* ctrlNumSU;
      }
      bFirst = false;
    }

    bValidCU = false;
    for(idx = currSU; idx < currSU + ctrlNumSU; idx++)
    {
      if(idx < startSU || idx > endSU)
      {
        continue;
      }

      LPelXSU   = LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[idx] ];
      TPelYSU   = LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[idx] ];

      if( !( LPelXSU < m_img_width )  || !( TPelYSU < m_img_height )  )
      {
        continue;
      }

      bValidCU = true;
    }

    if(bValidCU)
    {
      pvFlags->push_back(pcCU->getAlfCtrlFlag(currSU));
    }

    currSU += ctrlNumSU;
  }
}


/** set ALF CU control flags
 * \param [in] uiAlfCtrlDepth ALF CU control depth
 * \param [in] pcPicOrg picture of original signal
 * \param [in] pcPicDec picture before filtering
 * \param [in] pcPicRest picture after filtering
 * \param [out] ruiDist distortion after CU control
 * \param [in,out]vAlfCUCtrlParam ALF CU control parameters 
 */
Void TEncAdaptiveLoopFilter::setCUAlfCtrlFlags(UInt uiAlfCtrlDepth, Pel* imgOrg, Pel* imgDec, Pel* imgRest, Int stride, UInt64& ruiDist, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam)
{
  ruiDist = 0;
  std::vector<UInt> uiFlags;

  //initial
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    vAlfCUCtrlParam[s].cu_control_flag = 1;
    vAlfCUCtrlParam[s].alf_max_depth   = uiAlfCtrlDepth;

    vAlfCUCtrlParam[s].alf_cu_flag.reserve(m_uiNumCUsInFrame << ((g_uiMaxCUDepth-1)*2));
    vAlfCUCtrlParam[s].alf_cu_flag.resize(0);
  }

  //LCU-based on/off control
  for( UInt CUAddr = 0; CUAddr < m_pcPic->getNumCUsInFrame() ; CUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( CUAddr );
    setCUAlfCtrlFlag(pcCU, 0, 0, uiAlfCtrlDepth, imgOrg, imgDec, imgRest, stride, ruiDist, vAlfCUCtrlParam[0].alf_cu_flag);
  }
  vAlfCUCtrlParam[0].num_alf_cu_flag = (UInt)(vAlfCUCtrlParam[0].alf_cu_flag.size());


  if(m_uiNumSlicesInPic > 1)
  {
    //reset the first slice on/off flags
    vAlfCUCtrlParam[0].alf_cu_flag.resize(0);

    //distribute on/off flags to slices
    std::vector<UInt> vCtrlFlags;
    vCtrlFlags.reserve(1 << ((g_uiMaxCUDepth-1)*2));

    for(Int s=0; s < m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }
      std::vector< AlfLCUInfo* >& vpAlfLCU = m_pvpAlfLCU[s];
      for(Int i=0; i< vpAlfLCU.size(); i++)
      {
        //get on/off flags for one LCU
        vCtrlFlags.resize(0);
        getCtrlFlagsFromCU(vpAlfLCU[i], &vCtrlFlags, (Int)uiAlfCtrlDepth, m_pcPic->getNumPartInCU());

        for(Int k=0; k< vCtrlFlags.size(); k++)
        {
          vAlfCUCtrlParam[s].alf_cu_flag.push_back( vCtrlFlags[k]);
        }
      } //i (LCU)
      vAlfCUCtrlParam[s].num_alf_cu_flag = (UInt)(vAlfCUCtrlParam[s].alf_cu_flag.size());
    } //s (Slice)
  }
}

Void TEncAdaptiveLoopFilter::setCUAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, Pel* imgOrg, Pel* imgDec, Pel* imgRest, Int stride, UInt64& ruiDist, std::vector<UInt>& vCUCtrlFlag)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() ) )
        setCUAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth+1, uiAlfCtrlDepth, imgOrg, imgDec, imgRest, stride, ruiDist, vCUCtrlFlag);
    }
    return;
  }
  
  if( uiDepth > uiAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    return;
  }
  UInt64 uiRecSSD = 0;
  UInt64 uiFiltSSD = 0;
  
  Int iWidth;
  Int iHeight;
  UInt uiSetDepth;
  
  if (uiDepth > uiAlfCtrlDepth && pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    iWidth = g_uiMaxCUWidth >> uiAlfCtrlDepth;
    iHeight = g_uiMaxCUHeight >> uiAlfCtrlDepth;
    
    uiRPelX   = uiLPelX + iWidth  - 1;
    uiBPelY   = uiTPelY + iHeight - 1;

    if( uiRPelX >= pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() )
    {
      iWidth = pcCU->getSlice()->getSPS()->getPicWidthInLumaSamples() - uiLPelX;
    }
    
    if( uiBPelY >= pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() )
    {
      iHeight = pcCU->getSlice()->getSPS()->getPicHeightInLumaSamples() - uiTPelY;
    }
    
    uiSetDepth = uiAlfCtrlDepth;
  }
  else
  {
    iWidth = pcCU->getWidth(uiAbsPartIdx);
    iHeight = pcCU->getHeight(uiAbsPartIdx);
    uiSetDepth = uiDepth;
  }
  
  Int  offset = uiTPelY*stride + uiLPelX;
  Pel* pOrg  = imgOrg  + offset;
  Pel* pRec  = imgDec  + offset;
  Pel* pFilt = imgRest + offset;

  uiRecSSD  += xCalcSSD( pOrg, pRec,  iWidth, iHeight, stride );
  uiFiltSSD += xCalcSSD( pOrg, pFilt, iWidth, iHeight, stride );
  if (uiFiltSSD < uiRecSSD)
  {
    ruiDist += uiFiltSSD;
    pcCU->setAlfCtrlFlagSubParts(1, uiAbsPartIdx, uiSetDepth);
    vCUCtrlFlag.push_back(1);

    for (int i=uiTPelY ;i<=min(uiBPelY,(unsigned int)(m_img_height-1))  ;i++)
    {
      for (int j=uiLPelX ;j<=min(uiRPelX,(unsigned int)(m_img_width-1)) ;j++)
      { 
        m_maskImg[i][j]=1;
      }
    }
  }
  else
  {
    ruiDist += uiRecSSD;
    pcCU->setAlfCtrlFlagSubParts(0, uiAbsPartIdx, uiSetDepth);
    vCUCtrlFlag.push_back(0);
    for (int i=uiTPelY ;i<=min(uiBPelY,(unsigned int)(m_img_height-1))  ;i++)
    {
      for (int j=uiLPelX ;j<=min(uiRPelX,(unsigned int)(m_img_width-1)) ;j++)
      { 
        m_maskImg[i][j]=0;
      }
    }
  }
}
#endif

#define ROUND(a)  (((a) < 0)? (int)((a) - 0.5) : (int)((a) + 0.5))
#define REG              0.0001
#define REG_SQR          0.0000001

//Find filter coeff related
Int TEncAdaptiveLoopFilter::gnsCholeskyDec(Double **inpMatr, Double outMatr[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Int noEq)
{ 
  Int i, j, k;     /* Looping Variables */
  Double scale;       /* scaling factor for each row */
  Double invDiag[ALF_MAX_NUM_COEF];  /* Vector of the inverse of diagonal entries of outMatr */
  
  //  Cholesky decomposition starts
  
  for(i = 0; i < noEq; i++)
  {
    for(j = i; j < noEq; j++)
    {
      /* Compute the scaling factor */
      scale = inpMatr[i][j];
      if ( i > 0) 
      {
        for( k = i - 1 ; k >= 0 ; k--)
        {
          scale -= outMatr[k][j] * outMatr[k][i];
        }
      }
      /* Compute i'th row of outMatr */
      if(i == j)
      {
        if(scale <= REG_SQR ) // if(scale <= 0 )  /* If inpMatr is singular */
        {
          return 0;
        }
        else
        {
           /* Normal operation */
           invDiag[i] =  1.0 / (outMatr[i][i] = sqrt(scale));
        }
      }
      else
      {
        outMatr[i][j] = scale * invDiag[i]; /* Upper triangular part          */
        outMatr[j][i] = 0.0;              /* Lower triangular part set to 0 */
      }                    
    }
  }
  return 1; /* Signal that Cholesky factorization is successfully performed */
}


Void TEncAdaptiveLoopFilter::gnsTransposeBacksubstitution(Double U[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Double rhs[], Double x[], Int order)
{
  Int i,j;              /* Looping variables */
  Double sum;              /* Holds backsubstitution from already handled rows */
  
  /* Backsubstitution starts */
  x[0] = rhs[0] / U[0][0];               /* First row of U'                   */
  for (i = 1; i < order; i++)
  {         /* For the rows 1..order-1           */
    
    for (j = 0, sum = 0.0; j < i; j++) /* Backsubst already solved unknowns */
    {
      sum += x[j] * U[j][i];
    }
    x[i] = (rhs[i] - sum) / U[i][i];       /* i'th component of solution vect.  */
  }
}
#if AHG6_ALF_OPTION2
Void  TEncAdaptiveLoopFilter::gnsBacksubstitution(Double R[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Double z[ALF_MAX_NUM_COEF], Int R_size, Double A[ALF_MAX_NUM_COEF])
#else
Void  TEncAdaptiveLoopFilter::gnsBacksubstitution(Double R[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Double z[ALF_MAX_NUM_COEF], Int R_size, Double A[MAX_SQR_FILT_LENGTH])
#endif
{
  Int i, j;
  Double sum;
  
  R_size--;
  
  A[R_size] = z[R_size] / R[R_size][R_size];
  
  for (i = R_size-1; i >= 0; i--)
  {
    for (j = i + 1, sum = 0.0; j <= R_size; j++)
    {
      sum += R[i][j] * A[j];
    }
    
    A[i] = (z[i] - sum) / R[i][i];
  }
}


Int TEncAdaptiveLoopFilter::gnsSolveByChol(Double **LHS, Double *rhs, Double *x, Int noEq)
{
  assert(noEq > 0);

  Double aux[ALF_MAX_NUM_COEF];     /* Auxiliary vector */
  Double U[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF];    /* Upper triangular Cholesky factor of LHS */
  Int  i, singular;          /* Looping variable */
  
  /* The equation to be solved is LHSx = rhs */
  
  /* Compute upper triangular U such that U'*U = LHS */
  if(gnsCholeskyDec(LHS, U, noEq)) /* If Cholesky decomposition has been successful */
  {
    singular = 1;
    /* Now, the equation is  U'*U*x = rhs, where U is upper triangular
     * Solve U'*aux = rhs for aux
     */
    gnsTransposeBacksubstitution(U, rhs, aux, noEq);         
    
    /* The equation is now U*x = aux, solve it for x (new motion coefficients) */
    gnsBacksubstitution(U, aux, noEq, x);   
    
  }
  else /* LHS was singular */ 
  {
    singular = 0;
    
    /* Regularize LHS */
    for(i=0; i < noEq; i++)
    {
      LHS[i][i] += REG;
    }
    /* Compute upper triangular U such that U'*U = regularized LHS */
    singular = gnsCholeskyDec(LHS, U, noEq);
    if ( singular == 1 )
    {
      /* Solve  U'*aux = rhs for aux */  
      gnsTransposeBacksubstitution(U, rhs, aux, noEq);   
      
      /* Solve U*x = aux for x */
      gnsBacksubstitution(U, aux, noEq, x);      
    }
    else
    {
      x[0] = 1.0;
      for (i = 1; i < noEq; i++ )
      {
        x[i] = 0.0;
      }
    }
  }  
  return singular;
}

Void TEncAdaptiveLoopFilter::add_A(Double **Amerged, Double ***A, Int start, Int stop, Int size)
{ 
  Int i, j, ind;          /* Looping variable */
  
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      Amerged[i][j] = 0;
      for (ind = start; ind <= stop; ind++)
      {
        Amerged[i][j] += A[ind][i][j];
      }
    }
  }
}

Void TEncAdaptiveLoopFilter::add_b(Double *bmerged, Double **b, Int start, Int stop, Int size)
{ 
  Int i, ind;          /* Looping variable */
  
  for (i = 0; i < size; i++)
  {
    bmerged[i] = 0;
    for (ind = start; ind <= stop; ind++)
    {
      bmerged[i] += b[ind][i];
    }
  }
}

Double TEncAdaptiveLoopFilter::calculateErrorCoeffProvided(Double **A, Double *b, Double *c, Int size)
{
  Int i, j;
  Double error, sum = 0;
  
  error = 0;
  for (i = 0; i < size; i++)   //diagonal
  {
    sum = 0;
    for (j = i + 1; j < size; j++)
    {
      sum += (A[j][i] + A[i][j]) * c[j];
    }
    error += (A[i][i] * c[i] + sum - 2 * b[i]) * c[i];
  }
  
  return error;
}

Double TEncAdaptiveLoopFilter::calculateErrorAbs(Double **A, Double *b, Double y, Int size)
{
  Int i;
  Double error, sum;
  Double c[ALF_MAX_NUM_COEF];
  
  gnsSolveByChol(A, b, c, size);
  
  sum = 0;
  for (i = 0; i < size; i++)
  {
    sum += c[i] * b[i];
  }
  error = y - sum;
  
  return error;
}

Double TEncAdaptiveLoopFilter::mergeFiltersGreedy(Double **yGlobalSeq, Double ***EGlobalSeq, Double *pixAccGlobalSeq, Int intervalBest[NO_VAR_BINS][2], Int sqrFiltLength, Int noIntervals)
{
  Int first, ind, ind1, ind2, i, j, bestToMerge ;
  Double error, error1, error2, errorMin;
  static Double pixAcc_temp, error_tab[NO_VAR_BINS],error_comb_tab[NO_VAR_BINS];
  static Int indexList[NO_VAR_BINS], available[NO_VAR_BINS], noRemaining;
  if (noIntervals == NO_FILTERS)
  {
    noRemaining = NO_VAR_BINS;
    for (ind=0; ind<NO_VAR_BINS; ind++)
    {
      indexList[ind] = ind; 
      available[ind] = 1;
      m_pixAcc_merged[ind] = pixAccGlobalSeq[ind];
      memcpy(m_y_merged[ind], yGlobalSeq[ind], sizeof(Double)*sqrFiltLength);
      for (i=0; i < sqrFiltLength; i++)
      {
        memcpy(m_E_merged[ind][i], EGlobalSeq[ind][i], sizeof(Double)*sqrFiltLength);
      }
    }
  }
  // Try merging different matrices
  if (noIntervals == NO_FILTERS)
  {
    for (ind = 0; ind < NO_VAR_BINS; ind++)
    {
      error_tab[ind] = calculateErrorAbs(m_E_merged[ind], m_y_merged[ind], m_pixAcc_merged[ind], sqrFiltLength);
    }
    for (ind = 0; ind < NO_VAR_BINS - 1; ind++)
    {
      ind1 = indexList[ind];
      ind2 = indexList[ind+1];
      
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i = 0; i < sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j = 0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
  }
  while (noRemaining > noIntervals)
  {
    errorMin = 0; 
    first = 1;
    bestToMerge = 0;
    for (ind = 0; ind < noRemaining - 1; ind++)
    {
      error = error_comb_tab[indexList[ind]];
      if ((error < errorMin || first == 1))
      {
        errorMin = error;
        bestToMerge = ind;
        first = 0;
      }
    }
    ind1 = indexList[bestToMerge];
    ind2 = indexList[bestToMerge+1];
    m_pixAcc_merged[ind1] += m_pixAcc_merged[ind2];
    for (i = 0; i < sqrFiltLength; i++)
    {
      m_y_merged[ind1][i] += m_y_merged[ind2][i];
      for (j = 0; j < sqrFiltLength; j++)
      {
        m_E_merged[ind1][i][j] += m_E_merged[ind2][i][j];
      }
    }
    available[ind2] = 0;
    
    //update error tables
    error_tab[ind1] = error_comb_tab[ind1] + error_tab[ind1] + error_tab[ind2];
    if (indexList[bestToMerge] > 0)
    {
      ind1 = indexList[bestToMerge-1];
      ind2 = indexList[bestToMerge];
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i = 0; i < sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j = 0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
    if (indexList[bestToMerge+1] < NO_VAR_BINS - 1)
    {
      ind1 = indexList[bestToMerge];
      ind2 = indexList[bestToMerge+2];
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i=0; i<sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j=0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
    
    ind=0;
    for (i = 0; i < NO_VAR_BINS; i++)
    {
      if (available[i] == 1)
      {
        indexList[ind] = i;
        ind++;
      }
    }
    noRemaining--;
  }
  
  errorMin = 0;
  for (ind = 0; ind < noIntervals; ind++)
  {
    errorMin += error_tab[indexList[ind]];
  }
  
  for (ind = 0; ind < noIntervals - 1; ind++)
  {
    intervalBest[ind][0] = indexList[ind]; 
    intervalBest[ind][1] = indexList[ind+1] - 1;
  }
  
  intervalBest[noIntervals-1][0] = indexList[noIntervals-1]; 
  intervalBest[noIntervals-1][1] = NO_VAR_BINS-1;
  
  return(errorMin);
}

Void TEncAdaptiveLoopFilter::roundFiltCoeff(Int *FilterCoeffQuan, Double *FilterCoeff, Int sqrFiltLength, Int factor)
{
  Int i;
  Double diff; 
  Int diffInt, sign; 
  
  for(i = 0; i < sqrFiltLength; i++)
  {
    sign = (FilterCoeff[i] > 0)? 1 : -1; 
    diff = FilterCoeff[i] * sign; 
    diffInt = (Int)(diff * (Double)factor + 0.5); 
    FilterCoeffQuan[i] = diffInt * sign;
  }
}

Double TEncAdaptiveLoopFilter::QuantizeIntegerFilterPP(Double *filterCoeff, Int *filterCoeffQuant, Double **E, Double *y, Int sqrFiltLength, Int *weights)
{
  double error;
#if AHG6_ALF_OPTION2
  static Bool isFirst = true;
  static Int* filterCoeffQuantMod= NULL;
  if(isFirst)
  {
    filterCoeffQuantMod = new Int[ALF_MAX_NUM_COEF];
    isFirst = false;
  }
  Int factor = (1<<  ((Int)ALF_NUM_BIT_SHIFT)  );
#else
#if LCUALF_QP_DEPENDENT_BITS
  Int factor = (1<<(getAlfPrecisionBit(m_alfQP)));
#else
  Int factor = (1<<  ((Int)ALF_NUM_BIT_SHIFT)  );
#endif
#endif
  Int i;
  int quantCoeffSum, minInd, targetCoeffSumInt, k, diff;
  double targetCoeffSum, errMin;
  
  gnsSolveByChol(E, y, filterCoeff, sqrFiltLength);
  targetCoeffSum=0;
  for (i=0; i<sqrFiltLength; i++)
  {
    targetCoeffSum+=(weights[i]*filterCoeff[i]*factor);
  }
  targetCoeffSumInt=ROUND(targetCoeffSum);
  roundFiltCoeff(filterCoeffQuant, filterCoeff, sqrFiltLength, factor);
  quantCoeffSum=0;
  for (i=0; i<sqrFiltLength; i++)
  {
    quantCoeffSum+=weights[i]*filterCoeffQuant[i];
  }
  
  int count=0;
  while(quantCoeffSum!=targetCoeffSumInt && count < 10)
  {
    if (quantCoeffSum>targetCoeffSumInt)
    {
      diff=quantCoeffSum-targetCoeffSumInt;
      errMin=0; minInd=-1;
      for (k=0; k<sqrFiltLength; k++)
      {
        if (weights[k]<=diff)
        {
#if AHG6_ALF_OPTION2
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          filterCoeffQuantMod[k]--;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)filterCoeffQuantMod[i]/(double)factor;
          }
#else
          for (i=0; i<sqrFiltLength; i++)
          {
            m_filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          m_filterCoeffQuantMod[k]--;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)m_filterCoeffQuantMod[i]/(double)factor;
          }
#endif
          error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
          if (error<errMin || minInd==-1)
          {
            errMin=error;
            minInd=k;
          }
        } // if (weights(k)<=diff)
      } // for (k=0; k<sqrFiltLength; k++)
      filterCoeffQuant[minInd]--;
    }
    else
    {
      diff=targetCoeffSumInt-quantCoeffSum;
      errMin=0; minInd=-1;
      for (k=0; k<sqrFiltLength; k++)
      {
        if (weights[k]<=diff)
        {
#if AHG6_ALF_OPTION2
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          filterCoeffQuantMod[k]++;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)filterCoeffQuantMod[i]/(double)factor;
          }
#else
          for (i=0; i<sqrFiltLength; i++)
          {
            m_filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          m_filterCoeffQuantMod[k]++;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)m_filterCoeffQuantMod[i]/(double)factor;
          }
#endif
          error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
          if (error<errMin || minInd==-1)
          {
            errMin=error;
            minInd=k;
          }
        } // if (weights(k)<=diff)
      } // for (k=0; k<sqrFiltLength; k++)
      filterCoeffQuant[minInd]++;
    }
    
    quantCoeffSum=0;
    for (i=0; i<sqrFiltLength; i++)
    {
      quantCoeffSum+=weights[i]*filterCoeffQuant[i];
    }
  }
  if( count == 10 )
  {
    for (i=0; i<sqrFiltLength; i++)
    {
      filterCoeffQuant[i] = 0;
    }
  }
  
  checkFilterCoeffValue(filterCoeffQuant, sqrFiltLength, false);

  for (i=0; i<sqrFiltLength; i++)
  {
    filterCoeff[i]=(double)filterCoeffQuant[i]/(double)factor;
  }
  
  error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
  return(error);
}
Double TEncAdaptiveLoopFilter::findFilterCoeff(double ***EGlobalSeq, double **yGlobalSeq, double *pixAccGlobalSeq, int **filterCoeffSeq, int **filterCoeffQuantSeq, int intervalBest[NO_VAR_BINS][2], int varIndTab[NO_VAR_BINS], int sqrFiltLength, int filters_per_fr, int *weights, double errorTabForce0Coeff[NO_VAR_BINS][2])
{
  static double pixAcc_temp;
#if AHG6_ALF_OPTION2
  static Bool isFirst = true;
  static Int* filterCoeffQuant = NULL;
  static Double* filterCoeff = NULL;
  if(isFirst)
  {
    filterCoeffQuant = new Int[ALF_MAX_NUM_COEF];
    filterCoeff = new Double[ALF_MAX_NUM_COEF];
    isFirst = false;
  }
#endif
  double error;
  int k, filtNo;
  
  error = 0;
  for(filtNo = 0; filtNo < filters_per_fr; filtNo++)
  {
    add_A(m_E_temp, EGlobalSeq, intervalBest[filtNo][0], intervalBest[filtNo][1], sqrFiltLength);
    add_b(m_y_temp, yGlobalSeq, intervalBest[filtNo][0], intervalBest[filtNo][1], sqrFiltLength);
    
    pixAcc_temp = 0;    
    for(k = intervalBest[filtNo][0]; k <= intervalBest[filtNo][1]; k++)
      pixAcc_temp += pixAccGlobalSeq[k];
    
    // Find coeffcients
#if AHG6_ALF_OPTION2
    errorTabForce0Coeff[filtNo][1] = pixAcc_temp + QuantizeIntegerFilterPP(filterCoeff, filterCoeffQuant, m_E_temp, m_y_temp, sqrFiltLength, weights);
#else
    errorTabForce0Coeff[filtNo][1] = pixAcc_temp + QuantizeIntegerFilterPP(m_filterCoeff, m_filterCoeffQuant, m_E_temp, m_y_temp, sqrFiltLength, weights);
#endif
    errorTabForce0Coeff[filtNo][0] = pixAcc_temp;
    error += errorTabForce0Coeff[filtNo][1];
    
    for(k = 0; k < sqrFiltLength; k++)
    {
#if AHG6_ALF_OPTION2
      filterCoeffSeq[filtNo][k] = filterCoeffQuant[k];
      filterCoeffQuantSeq[filtNo][k] = filterCoeffQuant[k];
#else
      filterCoeffSeq[filtNo][k] = m_filterCoeffQuant[k];
      filterCoeffQuantSeq[filtNo][k] = m_filterCoeffQuant[k];
#endif
    }
  }
  
  for(filtNo = 0; filtNo < filters_per_fr; filtNo++)
  {
    for(k = intervalBest[filtNo][0]; k <= intervalBest[filtNo][1]; k++)
      varIndTab[k] = filtNo;
  }
  
  return(error);
}


/** Estimate filtering distortion by correlation values and filter coefficients
 * \param ppdE auto-correlation matrix
 * \param pdy cross-correlation array
 * \param piCoeff  filter coefficients
 * \param iFiltLength numbr of filter taps
 * \returns estimated distortion
 */
Int64 TEncAdaptiveLoopFilter::xFastFiltDistEstimation(Double** ppdE, Double* pdy, Int* piCoeff, Int iFiltLength)
{
  //static memory
  Double pdcoeff[ALF_MAX_NUM_COEF];
  //variable
  Int    i,j;
  Int64  iDist;
  Double dDist, dsum;
#if !AHG6_ALF_OPTION2
#if LCUALF_QP_DEPENDENT_BITS
  Int alfPrecisionBit = getAlfPrecisionBit( m_alfQP );
#endif
#endif

  for(i=0; i< iFiltLength; i++)
  {
#if AHG6_ALF_OPTION2
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< ((Int)ALF_NUM_BIT_SHIFT) );
#else
#if LCUALF_QP_DEPENDENT_BITS
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<<alfPrecisionBit);
#else
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< ((Int)ALF_NUM_BIT_SHIFT) );
#endif
#endif
  }

  dDist =0;
  for(i=0; i< iFiltLength; i++)
  {
    dsum= ((Double)ppdE[i][i]) * pdcoeff[i];
    for(j=i+1; j< iFiltLength; j++)
    {
      dsum += (Double)(2*ppdE[i][j])* pdcoeff[j];
    }

    dDist += ((dsum - 2.0 * pdy[i])* pdcoeff[i] );
  }


  UInt uiShift = g_uiBitIncrement<<1;
  if(dDist < 0)
  {
    iDist = -(((Int64)(-dDist + 0.5)) >> uiShift);
  }
  else //dDist >=0
  {
    iDist= ((Int64)(dDist+0.5)) >> uiShift;
  }

  return iDist;

}

#if AHG6_ALF_OPTION2
UInt TEncAdaptiveLoopFilter::uvlcBitrateEstimate(Int val)
{
  val++;
  assert ( val );
  UInt length = 1;
  while( 1 != val )
  {
    val >>= 1;
    length += 2;
  }
  return ((length >> 1) + ((length+1) >> 1));
}

#if ALF_COEFF_EXP_GOLOMB_K
UInt TEncAdaptiveLoopFilter::golombBitrateEstimate(Int coeff, Int k)
{
  UInt symbol = (UInt)abs(coeff);
  UInt bitcnt = 0;

  while( symbol >= (UInt)(1<<k) )
  {
    bitcnt++;
    symbol -= (1<<k);
    k  ++;
  }
  bitcnt++;
  while( k-- )
  {
    bitcnt++;
  }
  if(coeff != 0)
  {
    bitcnt++;
  }
  return bitcnt;
}
#endif

UInt TEncAdaptiveLoopFilter::filterCoeffBitrateEstimate(Int compIdx, Int* coeff)
{
  UInt bitrate =0;
  for(Int i=0; i< (Int)ALF_MAX_NUM_COEF; i++)
  {
    bitrate += (compIdx == ALF_Y)?(golombBitrateEstimate(coeff[i], kTableTabShapes[ALF_CROSS9x7_SQUARE3x3][i])):(svlcBitrateEsitmate(coeff[i]));
  }
  return bitrate;
}

UInt TEncAdaptiveLoopFilter::ALFParamBitrateEstimate(ALFParam* alfParam)
{
  UInt bitrate = 1; //alf enabled flag
  if(alfParam->alf_flag == 1)
  {
    if(alfParam->componentID == ALF_Y)
    {
      Int noFilters = min(alfParam->filters_per_group-1, 2);
      bitrate += uvlcBitrateEstimate(noFilters);
      if(noFilters == 1)
      {
        bitrate += uvlcBitrateEstimate(alfParam->startSecondFilter);
      }
      else if (noFilters == 2)
      {
        bitrate += ((Int)NO_VAR_BINS -1);
      }
    }
    for(Int g=0; g< alfParam->filters_per_group; g++)
    {
      bitrate += filterCoeffBitrateEstimate(alfParam->componentID, alfParam->coeffmulti[g]);
    }
  }
  return bitrate;
}

#endif
//! \}
