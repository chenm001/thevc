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

/** \file			TEncSIFO.cpp
\brief		Switched Interpolation with Offsets(SIFO) - encoder class
*/

#include "TEncTop.h"
#include "TEncSIFO.h"

#ifdef QC_SIFO

#define NORM_FACTOR                 0.25
#define COEFF_QP                        256
#define NO_SAMPLES                      {  1,  2, 16, 64} // noSamples
#define BL_SIZE                         16            // Block size
#define NO_DC_VAL                       64            // Number of DC levels

#ifdef QC_AMVRES
extern Int SIFO_Filter6 [4][7][6];
extern Int SIFO_Filter12[4][7][12];
#else
extern Int SIFO_Filter6 [4][3][6];
extern Int SIFO_Filter12[4][3][12];
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSIFO::TEncSIFO()
{
  SIFO_FILTER           = NULL;
  AccErrorP             = NULL;                    // [Filter][Sppos]
  SequenceAccErrorP     = NULL;                    // [Filter][Sppos]
  SequenceAccErrorB     = NULL;                    // [FilterF][FilterB][SpposF][SpposB]
  SIFO_FILTER           = NULL;                    // [num_SIFO][16 subpels][SQR_FILTER]
}
TEncSIFO::~TEncSIFO()
{
}
// ====================================================================================================================
// Public member functions
// ====================================================================================================================
Void TEncSIFO::init( TEncTop* pcEncTop, Int Tap)
{
  m_pcCfg             = pcEncTop;
  m_pcPredSearch			= pcEncTop->getPredSearch();
  initSIFOFilters(Tap);
}


Void TEncSIFO::initEncSIFO(TComSlice*& rpcSlice )
{
  // ------------------------------------------------------------------------------------------------------------------
  // Filter Initialization for current slice
  // ------------------------------------------------------------------------------------------------------------------
  //better to use prevFilterP & prevFilterB, instead of filterSequence
  //otherwise first B after P, will use filters of P frame.
  //and P frame after the last B, will use fitlers of B frame.

  //get the values=====
  Int prevFilter[16],predictFilterSequence[16],bestFilter;
  if(rpcSlice->getSliceType()==I_SLICE)
    return;
  else if(rpcSlice->getSliceType()==P_SLICE)
  { 
    bestFilter = m_pcPredSearch->getBestFilter_P();
    for(UInt i = 1; i < 16; ++i)
    {
      predictFilterSequence[i] = m_pcPredSearch->getPredictFilterSequenceP(i);
      prevFilter[i] = m_pcPredSearch->getPrevP_SIFOFilter(i);
    }
  }
  else
  {
    bestFilter = m_pcPredSearch->getBestFilter_B();
    for(UInt i = 1; i < 16; ++i)
    {
      predictFilterSequence[i] = m_pcPredSearch->getPredictFilterSequenceB(i);
      prevFilter[i] = m_pcPredSearch->getPrevB_SIFOFilter(i);
    }
  }
  //set the values=====
  for(UInt i = 1; i < 16; ++i)
    m_pcPredSearch->setSIFOFilter(prevFilter[i] , i); 

}

Void TEncSIFO::initSIFOFilters(Int Tap)
{
  static Int first = 1;
  UInt num_SIFO = m_pcPredSearch->getNum_SIFOFilters();

  if(first == 1)
  {  
    Int i;
    Int filterLength =  Tap;  
    Int sqrFiltLength = filterLength*filterLength;

    first = 0;
    xGet_mem3Ddouble(&SIFO_FILTER, num_SIFO, 16, sqrFiltLength);
    for (i=0; i<16; i++)
    {
      m_pcPredSearch->setSIFOFilter      (0,i);
      m_pcPredSearch->setPrevP_SIFOFilter(0,i);
    }
    initSeparableFilter(Tap);
#if USE_DIAGONAL_FILT==1
    xInitDiagonalFilter(Tap);
#endif
  }
}

Void TEncSIFO::initSeparableFilter(Int Tap)
{
  Int filterLength = Tap;

  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();

  Int oneMatrix[12];
  Double filterV[12], filterH[12], coeffQP=(Double)COEFF_QP;

  Int sub_pos, filterNo, filterNoV, filterNoH, j, k, l, filterIndV, filterIndH, counter;
  Int tmp=filterLength;
#ifdef QC_AMVRES
  UInt filterInd_HAM[3] = { 1,3,5 }; //80, Q0, 81, H, 82, Q1, 83
#endif

  for (j=0; j<filterLength; j++)
  {
    oneMatrix[j]=0;
  }
  oneMatrix[filterLength/2-1]=1;      //impulse response

  for (sub_pos=1; sub_pos<16; sub_pos++)
  {
    filterIndV=  (sub_pos)/4;// filterSel[sub_pos][0];
    filterIndH=  (sub_pos)%4;// filterSel[sub_pos][1];

    if (filterIndV==0 || filterIndH==0)
    {
      for (filterNo=0; filterNo<num_AVALABLE_FILTERS; filterNo++)
      {
        if (filterIndV==0)
        {
          for (j=0; j<filterLength; j++)    
            filterV[j]=oneMatrix[j];
        }
        else
        {
          if(Tap==6)
          {
            for (j=0; j<filterLength; j++)
#ifdef QC_AMVRES
              filterV[j]=(Double)SIFO_Filter6 [filterNo][filterInd_HAM[filterIndV-1]][j]/coeffQP;
#else
              filterV[j]=(Double)SIFO_Filter6 [filterNo][filterIndV-1][j]/coeffQP;
#endif
          }
          else
          {
            for (j=0; j<filterLength; j++)
#ifdef QC_AMVRES
              filterV[j]=(Double)SIFO_Filter12[filterNo][filterInd_HAM[filterIndV-1]][j]/coeffQP;
#else
              filterV[j]=(Double)SIFO_Filter12[filterNo][filterIndV-1][j]/coeffQP;
#endif
          }
        }

        if (filterIndH==0)
        {
          for (j=0; j<filterLength; j++)
            filterH[j]=oneMatrix[j];
        }
        else
        {
          if(Tap==6)
          {
            for (j=0; j<filterLength; j++)
#ifdef QC_AMVRES
              filterH[j]=(Double)SIFO_Filter6 [filterNo][filterInd_HAM[filterIndH-1]][j]/coeffQP;
#else
              filterH[j]=(Double)SIFO_Filter6 [filterNo][filterIndH-1][j]/coeffQP;
#endif
          }
          else
          {
            for (j=0; j<filterLength; j++)
#ifdef QC_AMVRES
              filterH[j]=(Double)SIFO_Filter12[filterNo][filterInd_HAM[filterIndH-1]][j]/coeffQP;
#else
              filterH[j]=(Double)SIFO_Filter12[filterNo][filterIndH-1][j]/coeffQP;
#endif
          }
        }

        m_pcPredSearch->setTabFilters(filterNo,sub_pos,filterNo,0);  
        m_pcPredSearch->setTabFilters(-1      ,sub_pos,filterNo,1);


        for (k=0; k<filterLength; k++)
        {
          for (l=0; l<filterLength; l++)
          {
            SIFO_FILTER[filterNo][sub_pos][k*tmp+l]=filterV[k]*filterH[l]; // modify
          }
        }
      }
    } // if (filterIndV==0 || filterIndH==0){
    else
    {
      counter=num_AVALABLE_FILTERS; // modify

      for (filterNoV=0; filterNoV<num_AVALABLE_FILTERS; filterNoV++)
      {
        for (filterNoH=0; filterNoH<num_AVALABLE_FILTERS; filterNoH++)
        {
          if(Tap==6)
          {
#ifdef QC_AMVRES
            for (j=0; j<filterLength; j++)
              filterV[j]=(Double)SIFO_Filter6 [filterNoV  ][filterInd_HAM[filterIndV-1]][j]/coeffQP;
            for (j=0; j<filterLength; j++)
              filterH[j]=(Double)SIFO_Filter6 [filterNoH  ][filterInd_HAM[filterIndH-1]][j]/coeffQP;
#else
            for (j=0; j<filterLength; j++)
              filterV[j]=(Double)SIFO_Filter6 [filterNoV  ][filterIndV-1][j]/coeffQP;
            for (j=0; j<filterLength; j++)
              filterH[j]=(Double)SIFO_Filter6 [filterNoH  ][filterIndH-1][j]/coeffQP;
#endif
          }
          else
          {
#ifdef QC_AMVRES
            for (j=0; j<filterLength; j++)
              filterV[j]=(Double)SIFO_Filter12[filterNoV+2][filterInd_HAM[filterIndV-1]][j]/coeffQP;
            for (j=0; j<filterLength; j++)
              filterH[j]=(Double)SIFO_Filter12[filterNoH+2][filterInd_HAM[filterIndH-1]][j]/coeffQP;
#else
            for (j=0; j<filterLength; j++)
              filterV[j]=(Double)SIFO_Filter12[filterNoV+2][filterIndV-1][j]/coeffQP;
            for (j=0; j<filterLength; j++)
              filterH[j]=(Double)SIFO_Filter12[filterNoH+2][filterIndH-1][j]/coeffQP;
#endif
          }

          if (filterNoV==filterNoH)
          {
            filterNo=filterNoV;
          }
          else
          {
            filterNo=counter;
            counter++;
          }

          m_pcPredSearch->setTabFilters(filterNoV + ((Tap==6)?0:2),sub_pos,filterNo,0);  
          m_pcPredSearch->setTabFilters(filterNoH + ((Tap==6)?0:2),sub_pos,filterNo,1);

          for (k=0; k<filterLength; k++)
          {
            for (l=0; l<filterLength; l++)
            {
              SIFO_FILTER[filterNo][sub_pos][k*filterLength+l]=filterV[k]*filterH[l];
            }
          }
        }
      }
    }
  } // for (sub_pos=0; sub_pos<15; sub_pos++){
}

Void TEncSIFO::ComputeFiltersAndOffsets( TComPic*& rpcPic)
{
  TComSlice* pcSlice = rpcPic->getSlice();

  if (pcSlice->getSliceType() == I_SLICE)
  {
    xResetSequenceFilters();
  }

  xResetAll(pcSlice);
  xResetOffsets(pcSlice);

  if(pcSlice->getSliceType() == P_SLICE)
  {
    xAccumulateError_P(rpcPic);       // Accumulate squared error for filter decision
#ifdef QC_SIFO_PRED
    if (m_pcPredSearch->getPredictFilterP() > 0 && pcSlice->getSPS()->getUseSIFO_Pred()) 
#else
    if (m_pcPredSearch->getPredictFilterP() > 0)
#endif
    {
      xUpdateSequenceFilters_P_pred(pcSlice); //compute sequence filter for P based on first pass
      m_pcPredSearch->setPredictFilterP(2);
    }
    else
    {
      xUpdateSequenceFilters_P(pcSlice);
      m_pcPredSearch->setPredictFilterP(1);
    }
  }
  else if(pcSlice->getSliceType() == B_SLICE)
  {
    xAccumulateError_B(rpcPic);       // Accumulate squared error for frame and sequence
    xComputeFilterCombination_B_gd(SequenceAccErrorB, SequenceBestCombFilterB);
    // Compute best single sequence filter
#ifdef QC_SIFO_PRED
    if (m_pcPredSearch->getPredictFilterB() > 0 && pcSlice->getSPS()->getUseSIFO_Pred())
#else
    if (m_pcPredSearch->getPredictFilterB() > 0)
#endif
    {
      xUpdateSequenceFilters_B_pred(pcSlice, SequenceAccErrorB, SequenceBestCombFilterB);
      m_pcPredSearch->setPredictFilterB(2);
    }
    else
    {
      xUpdateSequenceFilters_B     (pcSlice, SequenceAccErrorB, SequenceBestCombFilterB);
      m_pcPredSearch->setPredictFilterB(1);
    }
  }
}

Void TEncSIFO::setFirstPassSubpelOffset(RefPicList iRefList, TComSlice* pcSlice)
{
  Int subpelOffset[16];
  Int imgOffset[MAX_REF_PIC_NUM]; 
  Int offsetMETab[16];
  Int NumMEOffsets;
  Double DCdiff[MAX_REF_PIC_NUM];
  Int sign, i;
  Int list = (iRefList == REF_PIC_LIST_0)? 0:1;

  Int subPelPosOrder[15] = {5, 15, 13, 7, 9, 6, 11, 14, 1, 4, 3, 12, 10, 2, 8};
  Int DCmin, DCmax, thDC;
  Int noOffsets, noOffsetsSecond, DCint, firstVal, secondVal, addOffset;
  Int roundFact;
  Int DCminF;
  Int DCmaxF;
  Int NumOffsets;


  memset(DCdiff, 0, MAX_REF_PIC_NUM * sizeof(Double));
  memset(offsetMETab, 0, 16 * sizeof(Int));

  for (Int ref_frame = 0; ref_frame < pcSlice->getNumRefIdx(RefPicList(iRefList)); ref_frame++)
  {    
    DCdiff[ref_frame] = xGetDCdiff(pcSlice, iRefList, ref_frame);

    if(ref_frame==0)
    {

      sign = (DCdiff[ref_frame] >= 0)? 1: -1;
      noOffsets = (Int)(fabs(DCdiff[ref_frame]) + 0.5);
      DCint = noOffsets * sign;

      if(noOffsets < 2)
      {
        DCmax = noOffsets;
        DCmin = -noOffsets;
      }
      else
      {
        thDC = xGet_thDC(pcSlice);//determine the threshold of the number of samples  for each offset value
        xBlockDC(pcSlice, iRefList, &DCmin, &DCmax, thDC); 
        DCmax = max(DCint, DCmax);
        DCmin = min(DCint, DCmin);
      }

      roundFact = (Int)ceil((Double)(DCmax - DCmin) / 15.0);
      roundFact = max(roundFact, 1);      
      DCminF = (Int)(abs(DCmin) / roundFact + 0.5);
      DCmaxF = (Int)(abs(DCmax) / roundFact + 0.5);
      DCmin = -roundFact * DCminF;
      DCmax = roundFact * DCmaxF;

      noOffsetsSecond = 0;
      if(noOffsets < 2)
        noOffsetsSecond = min((Int)(fabs(DCdiff[ref_frame]) / 0.1 + 0.5), 15);

      if(noOffsetsSecond > 0)//number of subple using the second offset value
      {
        firstVal = 0;
        secondVal = sign;
        if(fabs(DCdiff[ref_frame]) > 0.5)
        {
          firstVal = sign; 
          secondVal = 0;
          noOffsetsSecond = 16 - noOffsetsSecond;
        }
      }
      else
      {
        firstVal = 0; 
        secondVal = 0;
      }
      addOffset = -firstVal;

      //firstVal and secondVal are based on the whole frame DCdiff, DCmin and DCmax is based on DCdiff and local DC diff
      if(((firstVal == DCmin) && (secondVal == DCmax)) || ((firstVal == DCmax) && (secondVal == DCmin)))
      {
        DCmin = 0; 
        DCmax = 0;
        DCminF = 0; 
        DCmaxF = 0;
      }

      for(i = 0; i < 16; i++)
        subpelOffset[i] = firstVal;

      NumOffsets = 0;
      for(i = DCmin; i <= DCmax; i += roundFact)
      {
        if((i != firstVal) && (i != secondVal))
        {
          subpelOffset[subPelPosOrder[NumOffsets]] = i;
          offsetMETab[NumOffsets] = i + addOffset;
          NumOffsets++;
          if(NumOffsets == 15)
            break;
        }
      }

      NumMEOffsets = NumOffsets;
      if((NumOffsets < 15) && (noOffsetsSecond > 0))
      {
        offsetMETab[NumMEOffsets] = secondVal + addOffset;
        (NumMEOffsets)++;
      }

      for(i = 0; i < noOffsetsSecond; i++)
      {
        if(NumOffsets == 15)
          break;

        subpelOffset[subPelPosOrder[NumOffsets]] = secondVal;
        (NumOffsets)++;
      }
    }
    else
    {
      sign = (DCdiff[ref_frame] >= 0)? 1: -1;
#define TFO 2.0
      imgOffset[ref_frame] = (fabs(DCdiff[ref_frame]) < TFO)? 0: 1;  
      if(sign < 0)
        imgOffset[ref_frame] = -imgOffset[ref_frame];
    }
  }

  if(g_uiBitIncrement)
  {
    for (Int ref_frame = 0; ref_frame < pcSlice->getNumRefIdx(RefPicList(iRefList)); ref_frame++)
    { 
      if(ref_frame==0)
      {
        for(i = 0; i < 16; i++)
        {
          subpelOffset[i] *= (1<<g_uiBitIncrement);
          offsetMETab[i] *= (1<<g_uiBitIncrement);
        }

      }
      else
      {
        imgOffset[ref_frame] *= (1<<g_uiBitIncrement);
      }
    }
  }


  //set the values..........
  m_pcPredSearch->setNum_Offset_FullpelME(NumMEOffsets,list);
  for (Int ref_frame = 0; ref_frame < pcSlice->getNumRefIdx(RefPicList(iRefList)); ref_frame++)
  { 
    if(ref_frame==0)
    {
      for(i = 0; i < 16; i++)
      {
        m_pcPredSearch->setSubpelOffset(subpelOffset[i],list,i);
        m_pcPredSearch->setOffset_FullpelME(offsetMETab[i],list,i);
      }
    }
    else
    {
      m_pcPredSearch->setFrameOffset(imgOffset[ref_frame],list,ref_frame);
    }
  }
  //........................

}






// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Double TEncSIFO::xComputeImgSum( Pel* img, Int width, Int height, Int stride )
{
  Int x, y;
  Double sum=0.;

  for (y=0; y<height; y++)
  {
    for (x=0; x<width; x++)
    {
      sum += (Double)(img[x]);
    }
    img += stride;
  }
  return sum;
}

Int TEncSIFO::xGet_mem2Ddouble(Double ***array2D, Int rows, Int columns)
{
  Int i,j;

  if((*array2D      = (Double**)calloc(rows, sizeof(Double*))) == NULL)
  {
    printf("xGet_mem2Ddouble: array2D");
    exit(100);
  }

  if(((*array2D)[0] = (Double* )calloc(columns*rows,sizeof(Double ))) == NULL)
  {
    printf("xGet_mem3Ddouble: array2D");
    exit(100);
  }

  for(i=1;i<rows;i++)
    (*array2D)[i] = (*array2D)[i-1] + columns ;

  for(i = 0; i < rows; i++)
    for(j = 0; j < columns; j++)
      (*array2D)[i][j] = 0.0;
  return sizeof(Double)*rows*columns;
}

Int TEncSIFO::xGet_mem3Ddouble(Double ****array3D, Int frames, Int rows, Int columns)
{
  Int  j;

  if(((*array3D) = (Double***)calloc(frames,sizeof(Double**))) == NULL)
  {
    printf("xGet_mem3Ddouble: array3D");
    exit(100);
  }

  for(j=0;j<frames;j++)
    xGet_mem2Ddouble( (*array3D)+j, rows, columns ) ;

  return frames*rows*columns*sizeof(Double);
}

Int TEncSIFO::xGet_mem4Ddouble(Double *****array4D, Int idx, Int frames, Int rows, Int columns )
{
  Int  j;

  if(((*array4D) = (Double****)calloc(idx,sizeof(Double***))) == NULL)
  {
    printf("xGet_mem4Ddouble: array4D");
    exit(100);
  }

  for(j=0;j<idx;j++)
    xGet_mem3Ddouble( (*array4D)+j, frames, rows, columns ) ;

  return idx*frames*rows*columns*sizeof(Double);
}

Void TEncSIFO::xResetSequenceFilters()
{
  Int i; 
  for (i=0; i<16; i++)
  {
    m_pcPredSearch->setSIFOFilter      (0,i);
    m_pcPredSearch->setPrevP_SIFOFilter(0,i);
    m_pcPredSearch->setPrevB_SIFOFilter(0,i);
  }
  m_pcPredSearch->setBestFilter_B(0+1);
  m_pcPredSearch->setBestFilter_P(0+2);
  m_pcPredSearch->setPredictFilterP(0);
  m_pcPredSearch->setPredictFilterB(0);
}

Void TEncSIFO::xResetAll(TComSlice* pcSlice)
{
  Int a, b, c, d;
  static Int firstP = 1;
  static Int firstB = 1;
  UInt num_SIFO = m_pcPredSearch->getNum_SIFOFilters();

  if(pcSlice->getSliceType() == P_SLICE)
  {
    if(firstP == 1)
    {
      xGet_mem2Ddouble(&AccErrorP, num_SIFO, 16);
      xGet_mem2Ddouble(&SequenceAccErrorP, 16, num_SIFO);
    }

    for(a = 0; a < num_SIFO; ++a)
      memset(AccErrorP[a], 0, 16 * sizeof(Double));

    if(firstP == 1)
    {
      firstP = 0;

      for(a = 0; a < 16; ++a)
        for(b = 0; b < num_SIFO; ++b)
          SequenceAccErrorP[a][b] = 0;
    }
    else
    {
      for(a = 0; a < 16; ++a)
        for(b = 0; b < num_SIFO; ++b)
          SequenceAccErrorP[a][b] *= NORM_FACTOR;
    }
  }
  else if(pcSlice->getSliceType() == B_SLICE)
  {
    if(firstB == 1)
    {  
      xGet_mem4Ddouble(&SequenceAccErrorB, num_SIFO, num_SIFO, 16, 16);
    }

    if(firstB == 1)
    {    
      firstB = 0;
      memset(SequenceBestCombFilterB, 0, 16 * sizeof(Int));

      for(a = 0; a < num_SIFO; ++a)
        for(b = 0; b < num_SIFO; ++b)
          for(c = 0; c < 16; ++c)
            for(d = 0; d < 16; ++d)
              SequenceAccErrorB[a][b][c][d] = 0;
    }
    else
    {
      for(a = 0; a < num_SIFO; ++a)
        for(b = 0; b < num_SIFO; ++b)
          for(c = 0; c < 16; ++c)
            for(d = 0; d < 16; ++d)
              SequenceAccErrorB[a][b][c][d] *= NORM_FACTOR;
    }
  }  
}

Void TEncSIFO::xAccumulateError_P(TComPic*& rpcPic)
{
  UInt uiCUAddr;
  TComPicYuv* pcPicOrg = rpcPic->getPicYuvOrg();
  TComPicYuv* pcPicRef;
  UInt64 uiSSD = 0;
  Int x, y,f;
  UInt uiShift = g_uiBitIncrement<<1;	
  Int iTemp;
  Int	iOrgStride = pcPicOrg->getStride();
  Int intPixelTab[16];
  UInt NUM_SIFO_TAB[16];

  for(UInt i=0;i<16;i++)
    NUM_SIFO_TAB[i] = m_pcPredSearch->getNum_SIFOTab(i);

  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {	
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );	

    UInt number_of_BasicUnit_perCU = rpcPic->getNumPartInCU();
    UInt BasicUnit_Width  = g_uiMaxCUWidth  / rpcPic->getNumPartInWidth();
    UInt BasicUnit_Height = g_uiMaxCUHeight / rpcPic->getNumPartInHeight();

    for(UInt uiIdx=0; uiIdx<number_of_BasicUnit_perCU; uiIdx++)
    {
      Int iRefIdx = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx(uiIdx);
      if(iRefIdx!=-1)
      {
        pcPicRef = pcCU->getSlice()->getRefPic( REF_PIC_LIST_0, iRefIdx )->getPicYuvRec();
        //TComMv mv = pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv(uiIdx);

        //for(UInt H=0;H<4;H++)
        //{
        //	for(UInt V=0;V<4;V++)
        //	{
        //		m_pcPredSearch->LumaPrediction ( pcCU, pcPicRef, uiIdx, &mv, BasicUnit_Width, BasicUnit_Height, pcYuv_filter, H, V);
        //	}
        //}
        //  m_pcCuEncoder->PredInterLumaBlk ( pcCU, pcPicRef, uiIdx, &mv, BasicUnit_Width, BasicUnit_Height, pcPicYuvFilt);

        uiSSD = 0;
        TComMv mv = pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv(uiIdx);
        Int mv_x;
        Int mv_y;

        pcCU->clipMv(mv);

#ifdef QC_AMVRES
        if (pcCU->getSlice()->getSPS()->getUseAMVRes())
        {
          if (mv.isHAM())
            continue;
          else
            mv.scale_down();
        }
#endif
        mv_x = mv.getHor();
        mv_y = mv.getVer();
        UInt     uixFrac  = mv_x & 0x3;
        UInt     uiyFrac  = mv_y & 0x3;
        UInt     uiSubPos = uixFrac+ 4*uiyFrac;

        Int     iRefStride = pcPicRef->getStride();

        Int     iRefOffset = ( mv_x >> 2 ) + ( mv_y >> 2 ) * iRefStride;

        UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ]; 
        UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
        //if (uiLPelX >= pcPicOrg->getWidth() || uiTPelY >= pcPicOrg->getHeight())
        //{
        //	continue;
        //}

        Pel* pOrgY = pcPicOrg->getLumaAddr(uiCUAddr, uiIdx);
        Pel* pRefY = pcPicRef->getLumaAddr(uiCUAddr, uiIdx) + iRefOffset;    

        for( y = 0; y < BasicUnit_Height; y++ )
        {
          for( x = 0; x < BasicUnit_Width; x++ )
          {	
#if BUGFIX50TMP
            xGetInterpolatedPixelArray(intPixelTab, (pRefY+x), uiLPelX, uiTPelY, iRefStride, pcPicRef->getWidth(), pcPicRef->getMaxAddr(), uiSubPos);
#else
            xGetInterpolatedPixelArray(intPixelTab, (pRefY+x), uiLPelX, uiTPelY, iRefStride, pcPicRef->getWidth(), pcPicRef->getHeight(), uiSubPos);
#endif
            for(f = 0; f < NUM_SIFO_TAB[uiSubPos]; ++f)
            {
              iTemp = *(pOrgY+x) - intPixelTab[f];
              uiSSD = ( iTemp * iTemp ) >> uiShift;
              AccErrorP[f][uiSubPos] += uiSSD;
            }
          }
          pOrgY += iOrgStride;
          pRefY += iRefStride;			
        }  	
      }
    }
  }
}

#if BUGFIX50TMP
Void TEncSIFO:: xGetInterpolatedPixelArray(int out[16], Pel *imgY, int x, int y, int Stride, int img_width, Pel *maxAddr, UInt sub_pos)
#else
Void TEncSIFO:: xGetInterpolatedPixelArray(int out[16], Pel *imgY, int x, int y, int Stride, int img_width, int img_height, UInt sub_pos)
#endif
{
  Double ipVal[16];
  Int ii, jj;
  Int filterNo;
  Pel *imgY_tmp, *imgY_tmp1;
  Int filter_length = m_pcCfg->getDIFTap();
  Int filter_offset = (filter_length/2 - 1);
  UInt num_SIFO = m_pcPredSearch->getNum_SIFOFilters();
  UInt NUM_SIFO_TAB[16];

  for(UInt i=0;i<16;i++)
    NUM_SIFO_TAB[i] = m_pcPredSearch->getNum_SIFOTab(i);

  for (filterNo=0; filterNo < num_SIFO; filterNo++)
    ipVal[filterNo] = 0.0;

  if(sub_pos)
  {
    imgY_tmp = imgY - filter_offset - (filter_offset*Stride);


    for(ii = 0; ii < filter_length; ++ii)
    {	  
      imgY_tmp1 = imgY_tmp;
      for(jj = 0; jj < filter_length; ++jj)
      {
#if BUGFIX50TMP
        if (imgY_tmp1 >= maxAddr)
          break;
#endif
        for (filterNo=0; filterNo < NUM_SIFO_TAB[sub_pos]; filterNo++)
        {
          ipVal[filterNo] += (SIFO_FILTER[filterNo][sub_pos][filter_length * ii + jj] * (*imgY_tmp1));
        }
        imgY_tmp1++;
      }
      imgY_tmp += Stride;
    }
    for (filterNo=0; filterNo < NUM_SIFO_TAB[sub_pos]; filterNo++)
    {
      //out[filterNo] = Clip3(0, (1<<(g_uiBitDepth + g_uiBitIncrement))-1, (int)(ipVal[filterNo] + 0.5));
      out[filterNo] = Clip((Int)(ipVal[filterNo] + 0.5));
    }
  }
}

Void TEncSIFO::xUpdateSequenceFilters_P(TComSlice* pcSlice)
{
  Int i,bestFilter/*,bestFilterP*/;
  Int filterSequence[16];
  Int    f, filterCombination[16], best, bitsPerFilter1, bitsPerFilter2, bits1, bits2;
  Double minVal, errTmp[4], lagr0, lagr1, lagr2, errorBest;

  Double lambda_md = pcSlice->getLambda();
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();
  UInt num_SIFO = m_pcPredSearch->getNum_SIFOFilters();
  UInt NUM_SIFO_TAB[16];

  for(UInt i=0;i<16;i++)
    NUM_SIFO_TAB[i] = m_pcPredSearch->getNum_SIFOTab(i);

  // Compute bit cost of sending the filters
  bitsPerFilter1=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); // modify
  bitsPerFilter2=(Int)ceil(log10((Double)num_SIFO)/log10((Double)2));

  bits2=0;
  for(i = 1; i < 16; ++i)
  {
    if (i==1 || i==2 || i==3 || i==4 || i==8 || i==12)
    {
      bits2+=bitsPerFilter1;
    }
    else
    {
      bits2+=bitsPerFilter2;
    }
  }
  bits1=15*bitsPerFilter1;

  // Filter selection based on the sequence - up to 2 filters per location
  for(i = 1; i < 16; ++i)
  {
    for(f = 0; f < NUM_SIFO_TAB[i]; ++f)
      SequenceAccErrorP[i][f] += AccErrorP[f][i];

    filterSequence[i] = 0;
    minVal = SequenceAccErrorP[i][0];
    for(f = 1; f < NUM_SIFO_TAB[i]; ++f)
    {
      if(SequenceAccErrorP[i][f] < minVal)
      {
        filterSequence[i] = f;
        minVal = SequenceAccErrorP[i][f];
      }
    }
  }

  lagr2=lambda_md * (double)bits2;
  for(i = 1; i < 16; ++i){
    lagr2+=AccErrorP[filterSequence[i]][i];
  }

  // Filter selection based on the sequence - 1 filter per location
  for(i = 1; i < 16; ++i)
  {
    filterCombination[i] = 0;
    minVal = SequenceAccErrorP[i][0];
    for(f = 1; f < num_AVALABLE_FILTERS; ++f) // modify
    {
      if(SequenceAccErrorP[i][f] < minVal)
      {
        filterCombination[i]=f;
        minVal = SequenceAccErrorP[i][f];
      }
    }
  }

  lagr1=lambda_md * (double)bits1;
  for(i = 1; i < 16; ++i){
    lagr1+=AccErrorP[filterCombination[i]][i];
  }

  // Single filter for all locations
  for (f=0; f < num_AVALABLE_FILTERS; f++){ // modify
    errTmp[f]=0.0;
    for(i = 1; i < 16; i++){
      errTmp[f] += SequenceAccErrorP[i][f];
    }
  }

  errorBest=errTmp[0]; best=0;
  for (f=1; f < num_AVALABLE_FILTERS; f++){ // modify
    if (errorBest>errTmp[f]){
      errorBest=errTmp[f];
      best=f;
    }
  }

  lagr0=0;
  for(i = 1; i < 16; ++i){
    lagr0+=AccErrorP[best][i];
  }

  bestFilter = 0;
  if(lagr0 < lagr1 && lagr0 < lagr2)
  {
    bestFilter = best+2;
    for(i = 1; i < 16; ++i)
      filterSequence[i] = best;
  }
  else if (lagr1 < lagr2){
    bestFilter = 1;
    for(i = 1; i < 16; ++i)
      filterSequence[i] = filterCombination[i];
  }

  for(i = 1; i < 16; ++i)
  {
    m_pcPredSearch->setSIFOFilter      (filterSequence[i],i);
    m_pcPredSearch->setPrevP_SIFOFilter(filterSequence[i],i);
  }
  m_pcPredSearch->setBestFilter_P(bestFilter);
}

Void TEncSIFO::xUpdateSequenceFilters_P_pred(TComSlice* pcSlice)
{
  Int i,bestFilter/*,bestFilterP*/;
  Int filterSequence[16],prevFilterP[16],predictFilterSequence[16];
  Int    f, best, bitsPerFilter1, bitsPerFilter2, bits2;
  Double minVal, errTmp[4], lagr0, lagr2, errCurrFilt, errPrevFilt, errorBest;
  Double lambda_md = pcSlice->getLambda();
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();
  UInt num_SIFO = m_pcPredSearch->getNum_SIFOFilters();
  UInt NUM_SIFO_TAB[16];

  for(UInt i=0;i<16;i++)
  {
    NUM_SIFO_TAB[i] = m_pcPredSearch->getNum_SIFOTab(i);
    prevFilterP[i]  = m_pcPredSearch->getPrevP_SIFOFilter(i);
  }
  // Compute bit cost of sending the filters
  bitsPerFilter1=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); // modify
  bitsPerFilter2=(Int)ceil(log10((Double)num_SIFO)/log10((Double)2));

  // Filter selection based on the sequence - up to 2 filters per location
  for(i = 1; i < 16; ++i)
  {
    for(f = 0; f < NUM_SIFO_TAB[i]; ++f)
      SequenceAccErrorP[i][f] += AccErrorP[f][i];

    filterSequence[i] = 0;
    minVal = SequenceAccErrorP[i][0];
    for(f = 1; f < NUM_SIFO_TAB[i]; ++f)
    {
      if(SequenceAccErrorP[i][f] < minVal)
      {
        filterSequence[i] = f;
        minVal = SequenceAccErrorP[i][f];
      }
    }
  }

  bits2=0;
  for(i = 1; i < 16; ++i){
    bits2++;
    errCurrFilt=AccErrorP[filterSequence[i]][i];
    errPrevFilt=AccErrorP[prevFilterP[i]][i];
    if (i<=4 || i==8 || i==12){
      if (errPrevFilt<(errCurrFilt+lambda_md * (Double)bitsPerFilter1))
      {
        filterSequence[i]=prevFilterP[i];
        predictFilterSequence[i]=1;
      }
      else{
        bits2+=bitsPerFilter1;
        predictFilterSequence[i]=0;
      }
    }
    else{
      if (errPrevFilt<(errCurrFilt+lambda_md * (Double)bitsPerFilter2)){
        filterSequence[i]=prevFilterP[i];
        predictFilterSequence[i]=1;
      }
      else{
        bits2+=bitsPerFilter2;
        predictFilterSequence[i]=0;
      }
    }
  }

  lagr2=lambda_md * (Double)bits2;
  for(i = 1; i < 16; ++i){
    lagr2+=AccErrorP[filterSequence[i]][i];
  }

  // Single filter for all locations
  for (f=0; f < num_AVALABLE_FILTERS; f++){ // modify
    errTmp[f]=0.0;
    for(i = 1; i < 16; i++){
      errTmp[f] += SequenceAccErrorP[i][f];
    }
  }

  errorBest=errTmp[0]; best=0;
  for (f=1; f < num_AVALABLE_FILTERS; f++){ // modify
    if (errorBest>errTmp[f]){
      errorBest=errTmp[f];
      best=f;
    }
  }

  lagr0=0;
  for(i = 1; i < 16; ++i)
  {
    lagr0+=AccErrorP[best][i];
  }

  bestFilter = 0;
  if(lagr0 < lagr2)
  {
    bestFilter = best+1;
    for(i = 1; i < 16; ++i)
      filterSequence[i] = best;
  }


  for(i = 1; i < 16; ++i)
  {
    m_pcPredSearch->setSIFOFilter      (filterSequence[i],i);
    m_pcPredSearch->setPrevP_SIFOFilter(filterSequence[i],i);
    m_pcPredSearch->setPredictFilterSequenceP(predictFilterSequence[i],i);
  }
  m_pcPredSearch->setBestFilter_P(bestFilter);
}


Double TEncSIFO::xGet_min_d(Double ****err, Int old_d[16], Int new_d[16], Double currSAD)
{
  Int spp, flt, sppF, sppB;
  Int tmp_d[16];
  Double minSAD = currSAD;
  Double tmpSAD;
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();

  for(spp = 1; spp < 16; ++spp)
  {
    // Copy a fresh old_d into tmp_d        
    memcpy(tmp_d, old_d, sizeof(Int) * 16);

    for(flt = 0; flt < num_AVALABLE_FILTERS - 1; ++flt)
    {
      tmp_d[spp] = (tmp_d[spp] + 1) % num_AVALABLE_FILTERS;
      tmpSAD = 0;
      for(sppF = 1; sppF < 16; ++sppF)
        for(sppB = 1; sppB < 16; ++sppB)
          tmpSAD += err[tmp_d[sppF]][tmp_d[sppB]][sppF][sppB];

      if(tmpSAD < minSAD)
      {
        minSAD = tmpSAD;
        memcpy(new_d, tmp_d, sizeof(Int) * 16);
      }
    }
  }
  return minSAD;
}

Void TEncSIFO::xComputeFilterCombination_B_gd(Double ****err, Int out[16])
{
  Int min_d[16] = {0};
  Int sppF, sppB;
  Int tmp_d[16] = {0};
  Double oldSAD = 0.0;
  Double minSAD;

  // Compute error for min_d = 0^n
  for(sppF = 1; sppF < 16; ++sppF)
    for(sppB = 1; sppB < 16; ++sppB)
      oldSAD += err[0][0][sppF][sppB];

  // While SAD can be improved
  while((minSAD = xGet_min_d(err, min_d, tmp_d, oldSAD)) < oldSAD)
  {
    oldSAD = minSAD;        
    memcpy(min_d, tmp_d, sizeof(Int) * 16);
  }

  // Copy result
  memcpy(out, min_d, sizeof(Int) * 16);
}

Void TEncSIFO::xUpdateSequenceFilters_B(TComSlice* pcSlice, Double ****err, Int combination[16])
{
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();

  Int bitsPerFilter1=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); // modify

  Int bits = 0;
  Int i,bestFilter/*,bestFilterB*/;
  Int filterSequence[16];
  Double errComb = 0.0;
  Double errBest = 0.0;
  Int sppF, sppB;
  Int best = -1;
  Double lambda_md = pcSlice->getLambda();

  // Lowest error with a single filter
  for(i = 0; i < num_AVALABLE_FILTERS; ++i)
  {
    double errTmp = 0.0;
    for(sppF = 1; sppF < 16; ++sppF)
      for(sppB = 1; sppB < 16; ++sppB)
        errTmp += err[i][i][sppF][sppB];
    if((best == -1) || (errTmp < errBest))
    {
      best = i;
      errBest = errTmp;
    }
  }
  // Compute bit cost of sending the filters
  bits = bitsPerFilter1*15;

  // Filter selection based on the sequence - 1 filter per location
  // Error with filter combination
  for(sppF = 1; sppF < 16; ++sppF)
    for(sppB = 1; sppB < 16; ++sppB)
      errComb += err[combination[sppF]][combination[sppB]][sppF][sppB];


  if(errBest < (errComb + lambda_md*(Double)bits))
  {
    for(i = 1; i < 16; ++i)
    {
      combination[i] = best;
    }
    bestFilter = best + 1;
  }
  else
  {
    bestFilter = 0;
  }

  for(i = 1; i < 16; ++i)
  {
    filterSequence[i]=combination[i];
  }


  for(i = 1; i < 16; ++i)
  {
    m_pcPredSearch->setSIFOFilter      (filterSequence[i], i);
    m_pcPredSearch->setPrevB_SIFOFilter(filterSequence[i], i);
  }
  m_pcPredSearch->setBestFilter_B(bestFilter);
}


Void TEncSIFO::xUpdateSequenceFilters_B_pred(TComSlice* pcSlice, Double ****err, Int combination[16])
{
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();

  Int bitsPerFilter1=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); // modify

  Int bits = 0;
  Int i,bestFilter/*,bestFilterB*/;
  Int filterSequence[16],prevFilterB[16],predictFilterSequence[16];
  Double errComb = 0.0;
  Double errBest = 0.0;
  Int sppF, sppB;
  Int best = -1;
  Double lambda_md = pcSlice->getLambda();

  for(i = 1; i < 16; ++i)
  {
    prevFilterB[i] = m_pcPredSearch->getPrevB_SIFOFilter(i);
  }

  // Lowest error with a single filter
  for(i = 0; i < num_AVALABLE_FILTERS; ++i)
  {
    double errTmp = 0.0;
    for(sppF = 1; sppF < 16; ++sppF)
      for(sppB = 1; sppB < 16; ++sppB)
        errTmp += err[i][i][sppF][sppB];
    if((best == -1) || (errTmp < errBest))
    {
      best = i;
      errBest = errTmp;
    }
  }
  // Compute bit cost of sending the filters
  bits = 0;

  // Filter selection based on the sequence - 1 filter per location
  // Error with filter combination

  for(sppF = 1; sppF < 16; ++sppF)
  {
    Double errFilt_Pred = 0.0;
    Double errFilt_Comb = 0.0;

    for(sppB = 1; sppB < 16; ++sppB)
    {
      errFilt_Pred += err[prevFilterB [sppF]][combination[sppB]][sppF][sppB];
      errFilt_Comb += err[combination[sppF]][combination[sppB]][sppF][sppB];
      if (sppB == sppF)
      {
        for(i = 1; i < 16; ++i)
        {
          errFilt_Pred += err[combination[i]][prevFilterB [sppB]][i][sppB];
          errFilt_Comb += err[combination[i]][combination[sppB]][i][sppB];
        }
        errFilt_Pred -= err[prevFilterB [sppF]][prevFilterB [sppB]][sppF][sppB];
        errFilt_Comb -= err[combination[sppF]][combination[sppB]][sppF][sppB];
      }
    }
    if (errFilt_Pred<(errFilt_Comb+lambda_md*(Double)bitsPerFilter1))
    {
      bits ++;
      combination[sppF]=prevFilterB[sppF];
      predictFilterSequence[sppF]=1;
    }
    else
    {
      bits+=bitsPerFilter1+1;
      predictFilterSequence[sppF]=0;
    }
  }
  for(sppF = 1; sppF < 16; ++sppF)
    for(sppB = 1; sppB < 16; ++sppB)
      errComb += err[combination[sppF]][combination[sppB]][sppF][sppB];


  if(errBest < (errComb + lambda_md*(Double)bits))
  {
    for(i = 1; i < 16; ++i)
    {
      combination[i] = best;
    }
    bestFilter = best + 1;
  }
  else
  {
    bestFilter = 0;
  }

  for(i = 1; i < 16; ++i)
    filterSequence[i]=combination[i];

  for(i = 1; i < 16; ++i)
  {
    m_pcPredSearch->setSIFOFilter      (filterSequence[i], i);
    m_pcPredSearch->setPrevB_SIFOFilter(filterSequence[i], i);
    m_pcPredSearch->setPredictFilterSequenceB(predictFilterSequence[i],i);
  }
  m_pcPredSearch->setBestFilter_B(bestFilter);
}

Void TEncSIFO::xAccumulateError_B(TComPic*& rpcPic)
{
  UInt uiCUAddr;
  TComPicYuv* pcPicOrg = rpcPic->getPicYuvOrg();
  TComPicYuv* pcPicRef;
  TComPicYuv* pcPicRefBi[2];
  UInt64 uiSSD = 0;
  Int x, y,f,f0,f1;
  UInt uiShift = g_uiBitIncrement<<1;	
  Int iTemp;
  Int	iOrgStride = pcPicOrg->getStride();
  Int intPixelTab[16],intPixelTabF[16],intPixelTabB[16];
  Int iaRefIdx[2];
  UInt num_AVALABLE_FILTERS = m_pcPredSearch->getNum_AvailableFilters();

  for( uiCUAddr = 0; uiCUAddr < rpcPic->getPicSym()->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {	
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );	

    UInt number_of_BasicUnit_perCU = rpcPic->getNumPartInCU();
    UInt BasicUnit_Width  = g_uiMaxCUWidth  / rpcPic->getNumPartInWidth();
    UInt BasicUnit_Height = g_uiMaxCUHeight / rpcPic->getNumPartInHeight();

    for(UInt uiIdx=0; uiIdx<number_of_BasicUnit_perCU; uiIdx++)
    {
      iaRefIdx[0] = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx(uiIdx);
      iaRefIdx[1] = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx(uiIdx);

      if(iaRefIdx[0] == -1 && iaRefIdx[1] == -1)
        continue;

      if(iaRefIdx[0] != -1 && iaRefIdx[1] != -1)
      {
        uiSSD = 0;

        pcPicRefBi[0] = pcCU->getSlice()->getRefPic(REF_PIC_LIST_0, iaRefIdx[0] )->getPicYuvRec();
        pcPicRefBi[1] = pcCU->getSlice()->getRefPic(REF_PIC_LIST_1, iaRefIdx[1] )->getPicYuvRec();
        TComMv mv0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv(uiIdx);
        TComMv mv1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getMv(uiIdx);

        Int mv_x0,mv_x1;
        Int mv_y0,mv_y1;
        pcCU->clipMv(mv0);                          pcCU->clipMv(mv1);

#ifdef QC_AMVRES
        if (pcCU->getSlice()->getSPS()->getUseAMVRes())
        {
          if (mv0.isHAM()||mv1.isHAM())
            continue;
          else
          {
            mv0.scale_down();
            mv1.scale_down();
          }
        }
#endif
        mv_x0 = mv0.getHor();                       mv_x1 = mv1.getHor();
        mv_y0 = mv0.getVer();                       mv_y1 = mv1.getVer();
        UInt     uixFrac0  = mv_x0 & 0x3;           UInt     uixFrac1  = mv_x1 & 0x3;
        UInt     uiyFrac0  = mv_y0 & 0x3;           UInt     uiyFrac1  = mv_y1 & 0x3;
        UInt     uiSubPos0 = uixFrac0+ 4*uiyFrac0;  UInt     uiSubPos1 = uixFrac1+ 4*uiyFrac1;

        Int     iRefStride0 = pcPicRefBi[0]->getStride();
        Int     iRefOffset0 = ( mv_x0 >> 2 ) + ( mv_y0 >> 2 ) * iRefStride0;
        Int     iRefStride1 = pcPicRefBi[1]->getStride();
        Int     iRefOffset1 = ( mv_x1 >> 2 ) + ( mv_y1 >> 2 ) * iRefStride1;

        UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ]; 
        UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

        Pel* pOrgY = pcPicOrg->getLumaAddr(uiCUAddr, uiIdx);
        Pel* pRefY0 = pcPicRefBi[0]->getLumaAddr(uiCUAddr, uiIdx) + iRefOffset0;    
        Pel* pRefY1 = pcPicRefBi[1]->getLumaAddr(uiCUAddr, uiIdx) + iRefOffset1;    

        for( y = 0; y < BasicUnit_Height; y++ )
        {
          for( x = 0; x < BasicUnit_Width; x++ )
          {	
#if BUGFIX50TMP
            xGetInterpolatedPixelArray(intPixelTabF, (pRefY0+x), uiLPelX, uiTPelY, iRefStride0, pcPicRefBi[0]->getWidth(), pcPicRefBi[0]->getMaxAddr(), uiSubPos0);
            xGetInterpolatedPixelArray(intPixelTabB, (pRefY1+x), uiLPelX, uiTPelY, iRefStride1, pcPicRefBi[1]->getWidth(), pcPicRefBi[1]->getMaxAddr(), uiSubPos1);
#else
            xGetInterpolatedPixelArray(intPixelTabF, (pRefY0+x), uiLPelX, uiTPelY, iRefStride0, pcPicRefBi[0]->getWidth(), pcPicRefBi[0]->getHeight(), uiSubPos0);
            xGetInterpolatedPixelArray(intPixelTabB, (pRefY1+x), uiLPelX, uiTPelY, iRefStride1, pcPicRefBi[1]->getWidth(), pcPicRefBi[1]->getHeight(), uiSubPos1);
#endif
            for(f0 = 0; f0 < num_AVALABLE_FILTERS; ++f0)
            {
              for(f1 = 0; f1 < num_AVALABLE_FILTERS; ++f1)
              {						
                iTemp = *(pOrgY+x) - ((intPixelTabF[f0]+intPixelTabB[f1]+1)>>1);
                uiSSD = ( iTemp * iTemp ) >> uiShift;
                SequenceAccErrorB[f0][f1][uiSubPos0][uiSubPos1] += uiSSD;
              }
            }
#if !BUGFIX50
            pOrgY  += iOrgStride;
            pRefY0 += iRefStride0;			
            pRefY1 += iRefStride1;
#endif
          }  	
#if BUGFIX50
          pOrgY  += iOrgStride;
          pRefY0 += iRefStride0;			
          pRefY1 += iRefStride1;
#endif
        }
      }
      else
      {
        Int iRefIdx = (iaRefIdx[0]!= -1)? iaRefIdx[0] : iaRefIdx[1];
        RefPicList list = (iaRefIdx[0]!= -1)? REF_PIC_LIST_0 : REF_PIC_LIST_1;

        pcPicRef = pcCU->getSlice()->getRefPic( list, iRefIdx )->getPicYuvRec();

        uiSSD = 0;
        TComMv mv = pcCU->getCUMvField( list )->getMv(uiIdx);
        pcCU->clipMv(mv);

        Int mv_x ;
        Int mv_y ;
#ifdef QC_AMVRES
        if (pcCU->getSlice()->getSPS()->getUseAMVRes())
        {
          if (mv.isHAM())
            continue;
          else
            mv.scale_down();
        }
#endif

        mv_x = mv.getHor();
        mv_y = mv.getVer();
        UInt     uixFrac  = mv_x & 0x3;
        UInt     uiyFrac  = mv_y & 0x3;
        UInt     uiSubPos = uixFrac+ 4*uiyFrac;

        Int     iRefStride = pcPicRef->getStride();
        Int     iRefOffset = ( mv_x >> 2 ) + ( mv_y >> 2 ) * iRefStride;

        UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ]; 
        UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];

        Pel* pOrgY = pcPicOrg->getLumaAddr(uiCUAddr, uiIdx);
        Pel* pRefY = pcPicRef->getLumaAddr(uiCUAddr, uiIdx) + iRefOffset;    

        for( y = 0; y < BasicUnit_Height; y++ )
        {
          for( x = 0; x < BasicUnit_Width; x++ )
          {	
            // Unidirectional error is accumulated along the diagonal
#if BUGFIX50TMP
            xGetInterpolatedPixelArray(intPixelTab, (pRefY+x), uiLPelX, uiTPelY, iRefStride, pcPicRef->getWidth(), pcPicRef->getMaxAddr(), uiSubPos);
#else
            xGetInterpolatedPixelArray(intPixelTab, (pRefY+x), uiLPelX, uiTPelY, iRefStride, pcPicRef->getWidth(), pcPicRef->getHeight(), uiSubPos);
#endif
            for(f = 0; f < num_AVALABLE_FILTERS; ++f)
            {
              iTemp = *(pOrgY+x) - intPixelTab[f];
              uiSSD = ( iTemp * iTemp ) >> uiShift;
              //AccErrorP[f][uiSubPos] += uiSSD;
              SequenceAccErrorB[f][f][uiSubPos][uiSubPos] += uiSSD;
            }
          }
          pOrgY += iOrgStride;
          pRefY += iRefStride;			
        }  	
      }
    }
  }
}





Double TEncSIFO::xGetDCdiff(TComSlice* rpcSlice, RefPicList list, Int ref)
{
  Double dDCOrg		= 0.;
  Double dDCRef		= 0.;

  TComPicYuv* pcPicYuvOrg = rpcSlice->getPic()->getPicYuvOrg();
  TComPicYuv* pcPicYuvRef = rpcSlice->getRefPic(list, ref)->getPicYuvRec();

  Int iWidth  = pcPicYuvOrg->getWidth();
  Int iHeight = pcPicYuvOrg->getHeight();

  dDCOrg = xComputeImgSum(pcPicYuvOrg->getLumaAddr(), iWidth, iHeight, pcPicYuvOrg->getStride());
  dDCRef = xComputeImgSum(pcPicYuvRef->getLumaAddr(), iWidth, iHeight, pcPicYuvRef->getStride());

  if(g_uiBitIncrement)
  {
    dDCOrg /= (1<<g_uiBitIncrement);
    dDCRef /= (1<<g_uiBitIncrement);
  }
  return ((dDCOrg - dDCRef) / (Double)(iWidth * iHeight));
}

Int TEncSIFO::xGet_thDC(TComSlice* rpcSlice)
{
  TComPicYuv* pcPicYuvOrg = rpcSlice->getPic()->getPicYuvOrg();
  Int ioWidth  = pcPicYuvOrg->getWidth();
  Int ioHeight = pcPicYuvOrg->getHeight();

  Int noMB = (ioWidth*ioHeight)/(16*16);
  Int thDC[4] = NO_SAMPLES;
  Int mb[4]   = {   99,  396, 3600, 8100};
  Int i;
  Int last = 1;

  for(i = 0; i < 4; ++i)
    if(noMB <= mb[i])
      return thDC[i];
    else
      last = thDC[i];
  return last; 
}

Void TEncSIFO::xBlockDC(TComSlice* rpcSlice, RefPicList list, Int *DCmin, Int *DCmax, Int noSamples)
{
  Int i, j, ii, jj;
  Int ref_frame, noPixels;
  Double DCOrg = 0.0;
  Double DCRef = 0.0;
  Double DCdiff, temp0, temp1, err0, err1;
  Int DCInt;
  Int DCinterval[2][NO_DC_VAL] = {{0}};

  memset(DCinterval, 0, 2 * NO_DC_VAL * sizeof(Int));
  ref_frame = 0;

  TComPicYuv* pcPicYuvOrg = rpcSlice->getPic()->getPicYuvOrg();
  TComPicYuv* pcPicYuvRef = rpcSlice->getRefPic(list, 0)->getPicYuvRec();

  Int img_width  = pcPicYuvOrg->getWidth();
  Int img_height = pcPicYuvOrg->getHeight();
  Int ioStride = pcPicYuvOrg->getStride();
  Int irStride = pcPicYuvRef->getStride();

  Pel *imgY_org = pcPicYuvOrg->getLumaAddr();
  Pel *imgY_ref = pcPicYuvRef->getLumaAddr();
  Pel *imgY_org1 = imgY_org;
  Pel *imgY_ref1 = imgY_ref;


  for(i = 0; i < img_height; i+=BL_SIZE)
  {
    for(j = 0; j < img_width; j+=BL_SIZE)
    {
      DCOrg = 0.0;
      DCRef = 0.0;
      noPixels = 0;

      imgY_org = imgY_org1 + j + (i*ioStride);
      imgY_ref = imgY_ref1 + j + (i*irStride);
      for(ii = 0; ii < BL_SIZE; ii++)
      {
        for(jj = 0; jj < BL_SIZE; jj++)
        {
          DCOrg += imgY_org[jj];
          DCRef += imgY_ref[jj];
          noPixels++;
        }
        imgY_org += ioStride;
        imgY_ref += irStride;
      }

      DCdiff = (DCOrg - DCRef) / (Double)noPixels;
      DCOrg = DCOrg / (Double)noPixels;
      DCRef = DCRef / (Double)noPixels;


      imgY_org = imgY_org1 + j + (i*ioStride);
      imgY_ref = imgY_ref1 + j + (i*irStride);
      err0 = 0.0;      err1 = 0.0;
      for(ii = 0; ii < BL_SIZE; ii++)
      {
        for(jj = 0; jj < BL_SIZE; jj++)
        {
          temp0 =  imgY_org[jj]  -  imgY_ref[jj];
          temp1 = (imgY_org[jj] - DCOrg) - (imgY_ref[jj] - DCRef);
          err0 += temp0 * temp0;
          err1 += temp1 * temp1;
        }
        imgY_org += ioStride;
        imgY_ref += irStride;
      }

      if(g_uiBitIncrement)
        DCdiff /= (1<<g_uiBitIncrement);

      DCInt = min((Int)(fabs(DCdiff) + 0.5), NO_DC_VAL - 1);

      if(err1 < (err0 - err1))
      {    
        if(DCdiff < 0)
        {
          DCinterval[0][DCInt]++;
          DCInt = -DCInt;// ??? redundant 
        }
        else
        {
          DCinterval[1][DCInt]++;
        }
      }


    } //width
  } //height

  (*DCmin) = 0;
  (*DCmax) = 0;

  {
    int flag;
    for(i=0, flag=0; i<NO_DC_VAL; i++)
    {
      if(DCinterval[1][i]>=noSamples)
      {
        (*DCmax)=(*DCmax)+1;
        flag = 1;
      }
      else if(flag)
      {
        i = NO_DC_VAL;
      }
    }
    for(i=0, flag=0; i<NO_DC_VAL; i++)
    {
      if(DCinterval[0][i]>=noSamples)
      {
        (*DCmin)=(*DCmin)+1;
        flag = 1;
      }
      else if(flag)
      {
        i = NO_DC_VAL;
      }
    }
  }

  (*DCmin) = -(*DCmin);
}

Void TEncSIFO::xResetOffsets(TComSlice* pcSlice)
{
  if(pcSlice->getSliceType() == P_SLICE)
  {
    xResetFirstPassSubpelOffset(0);
  }
  else if(pcSlice->getSliceType() == B_SLICE)
  {
    xResetFirstPassSubpelOffset(0);
    xResetFirstPassSubpelOffset(1);
  }
}

Void TEncSIFO::xResetFirstPassSubpelOffset(Int list)
{
  m_pcPredSearch->setNum_Offset_FullpelME(0,list);
  for(UInt i = 0; i < 16; ++i) 
    m_pcPredSearch->setOffset_FullpelME(0,list,i);
}


#if USE_DIAGONAL_FILT==1

#define DIAGONAL_FILETR_NO               1


Void TEncSIFO::xInitDiagonalFilter(Int Tap)
{
  Int sub_pos, filterIndD1, filterIndD2;
  Int filterLength, sqrFiltLength, j;

  if(Tap!=6)
    return;

  filterLength = 6;
  sqrFiltLength=filterLength*filterLength;

  xGet_mem2Ddouble(&SIFO_FILTER_DIAG, 16, sqrFiltLength);
  for (sub_pos=1; sub_pos<16; sub_pos++)
  {
    if (sub_pos==5)
    {
      filterIndD1=QU0_IDX;
      xSingleDiagonalFilt(sub_pos, filterIndD1, 0);
    }
    if (sub_pos==7)
    {
      filterIndD1=QU0_IDX;
      xSingleDiagonalFilt(sub_pos, filterIndD1, 1);
    }
    if (sub_pos==15)
    {
      filterIndD1=QU1_IDX;
      xSingleDiagonalFilt(sub_pos, filterIndD1, 0);
    }
    if (sub_pos==13)
    {
      filterIndD1=QU1_IDX;
      xSingleDiagonalFilt(sub_pos, filterIndD1, 1);
    }

    if (sub_pos==6)
    {
      filterIndD1=QU0_IDX; filterIndD2=QU0_IDX;
      xDoubleDiagonalFilt(sub_pos, filterIndD1, filterIndD2);
    }
    if (sub_pos==14)
    {
      filterIndD1=QU1_IDX; filterIndD2=QU1_IDX;
      xDoubleDiagonalFilt(sub_pos, filterIndD1, filterIndD2);
    }
    if (sub_pos==9)
    {
      filterIndD1=QU0_IDX; filterIndD2=QU1_IDX;
      xDoubleDiagonalFilt(sub_pos, filterIndD1, filterIndD2);
    }
    if (sub_pos==11)
    {
      filterIndD1=QU1_IDX; filterIndD2=QU0_IDX;
      xDoubleDiagonalFilt(sub_pos, filterIndD1, filterIndD2);
    }
    if (sub_pos==10)
    {
      filterIndD1=HAL_IDX; filterIndD2=HAL_IDX;
      xDoubleDiagonalFilt(sub_pos, filterIndD1, filterIndD2);
    }
  } // for (sub_pos=0; sub_pos<15; sub_pos++)


  Int counter=0, filterNo, filterDiag[9]={8, 5, 8, 10, 5, 10, 8, 5, 8};
  for (sub_pos=1; sub_pos<16; sub_pos++)
  {
    if (sub_pos>4 && sub_pos!=8 && sub_pos!=12)
    {
      filterNo=filterDiag[counter];
      for(j = 0; j < sqrFiltLength; ++j)
      {
        SIFO_FILTER[filterNo][sub_pos][j] = SIFO_FILTER_DIAG[sub_pos][j];
      }
      counter++;
    }
  }

static Double STRONG_FILT[4][4]=
{
  {  0.0       ,  5.0/128.0   ,   5.0/128.0   ,    0.0      },
  {5.0/128.0   , 22.0/128.0   ,  22.0/128.0  ,  5.0/128.0  }, 
  {5.0/128.0   , 22.0/128.0   ,  22.0/128.0  ,  5.0/128.0  }, 
  {  0.0       ,  5.0/128.0   ,   5.0/128.0   ,    0.0      }       
};

    for(j = 0; j < sqrFiltLength; ++j)
    {
        SIFO_FILTER[5][10][j] = 0.0;      
        SIFO_FILTER[5][10][j] = 0.0;   
    }
    UInt start_strong_filt=(filterLength-4)/2;
    UInt i;
    //generating 6x6 filter by padding 0's surrounding 4x4 filter.....
    for(i = start_strong_filt; i < start_strong_filt+4; ++i)
    {
      for(j = start_strong_filt; j < start_strong_filt+4; ++j)
      {
          SIFO_FILTER[5][10][i*filterLength+j] = STRONG_FILT[i-start_strong_filt][j-start_strong_filt];   
      }
    }


}

Void TEncSIFO::xSingleDiagonalFilt(Int sub_pos, Int filterIndD1, Int dir)
{
  Double filterD1[6], coeffQP=256;
  Int filterLength = 6;
  Int filterNo=DIAGONAL_FILETR_NO, j, k, l;

  for (j=0; j<filterLength; j++)
    filterD1[j]=(Double)SIFO_Filter6[filterNo][filterIndD1][j]/coeffQP;

  for (k=0; k<filterLength; k++){
    for (l=0; l<filterLength; l++){
      if (dir==0)
      {
        if (k==l)
        {
          SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=filterD1[k];
        }
        else
        {
          SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=0;
        }
      }
      else
      {
        if ((k+l)==(filterLength-1))
        {
          SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=filterD1[k];
        }
        else
        {
          SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=0;
        }
      }
    }
  }
}


Void TEncSIFO::xDoubleDiagonalFilt(Int sub_pos, Int filterIndD1, Int filterIndD2)
{
  Double filterD1[6], filterD2[6], coeffQP=256;
  Int filterLength = 6;
  Int filterNo=DIAGONAL_FILETR_NO, j, k, l;

  for (j=0; j<filterLength; j++)
    filterD1[j]=(Double)SIFO_Filter6[filterNo][filterIndD1][j]/coeffQP;
  for (j=0; j<filterLength; j++)
    filterD2[j]=(Double)SIFO_Filter6[filterNo][filterIndD2][j]/coeffQP;

  for (k=0; k<filterLength; k++){
    for (l=0; l<filterLength; l++){
      if (k==l){
        SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=0.5*filterD1[k];
      }
      else if ((k+l)==(filterLength-1)){
        SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=0.5*filterD2[k];
      }
      else{
        SIFO_FILTER_DIAG[sub_pos][k*filterLength+l]=0;
      }
    }
  }

}
#endif
#endif