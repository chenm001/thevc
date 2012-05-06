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

/** \file     TComAdaptiveLoopFilter.h
    \brief    adaptive loop filter class (header)
*/

#ifndef __TCOMADAPTIVELOOPFILTER__
#define __TCOMADAPTIVELOOPFILTER__

#include "CommonDef.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

  #define LCUALF_QP_DEPENDENT_BITS    1  

#define ALF_FILTER_LEN       10
#define ALF_MAX_NUM_COEF     ALF_FILTER_LEN    //!< maximum number of filter coefficients
#define MAX_SQR_FILT_LENGTH   41                                      //!< ((max_horizontal_tap * max_vertical_tap) / 2 + 1) = ((11 * 5) / 2 + 1)

#if LCUALF_QP_DEPENDENT_BITS
#define ALF_QP1               28 
#define ALF_QP2               34 
#define ALF_QP3               39 
#else
#define ALF_NUM_BIT_SHIFT     8                                       ///< bit shift parameter for quantization of ALF param.
#endif

#define VAR_SIZE_H            4
#define VAR_SIZE_W            4
#define NO_VAR_BINS           16 
#define NO_FILTERS            16
#define MAX_SCAN_VAL          13
#define MAX_EXP_GOLOMB        16



/// Luma/Chroma component ID
enum ALFComponentID
{
  ALF_Y = 0,
  ALF_Cb,
  ALF_Cr,
  NUM_ALF_COMPONENT
};
/// ALF LCU merge type
enum ALFLCUMergeType
{
  ALF_MERGE_DISABLED = 0,
  ALF_MERGE_UP,
  ALF_MERGE_LEFT,
  ALF_MERGE_FIRST,
  NUM_ALF_MERGE_TYPE
};
///
/// Filter shape
///
enum ALFFilterShape
{
  ALF_CROSS9x7_SQUARE3x3 = 0,
  NUM_ALF_FILTER_SHAPE
};

extern Int* kTableTabShapes[NUM_ALF_FILTER_SHAPE];
extern Int depthIntShape1Sym[ALF_MAX_NUM_COEF+1];
extern Int *pDepthIntTabShapes[NUM_ALF_FILTER_SHAPE];

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// ALF CU control parameters
struct AlfCUCtrlInfo
{
  Int  cu_control_flag;                    //!< slice-level ALF CU control enabled/disabled flag 
  UInt num_alf_cu_flag;                    //!< number of ALF CU control flags
  UInt alf_max_depth;                      //!< ALF CU control depth
  std::vector<UInt> alf_cu_flag;           //!< ALF CU control flags (container)

  const AlfCUCtrlInfo& operator= (const AlfCUCtrlInfo& src);  //!< "=" operator
  AlfCUCtrlInfo():cu_control_flag(0), num_alf_cu_flag(0), alf_max_depth(0) {} //!< constructor
  Void reset();
};


///
/// LCU-based ALF processing info
///
struct AlfLCUInfo
{
  TComDataCU* pcCU;            //!< TComDataCU pointer
  Int         sliceID;        //!< slice ID
  Int         tileID;         //!< tile ID
  UInt        numSGU;        //!< number of slice granularity blocks 
  UInt        startSU;       //!< starting SU z-scan address in LCU
  UInt        endSU;         //!< ending SU z-scan address in LCU
  Bool        bAllSUsInLCUInSameSlice; //!< true: all SUs in this LCU belong to same slice
  std::vector<NDBFBlockInfo*> vpAlfBlock; //!< container for filter block pointers

  NDBFBlockInfo& operator[] (Int idx) { return *( vpAlfBlock[idx]); } //!< [] operator
  AlfLCUInfo():pcCU(NULL), sliceID(0), tileID(0), numSGU(0), startSU(0), endSU(0), bAllSUsInLCUInSameSlice(false) {} //!< constructor
};


///
/// adaptive loop filter class
///
class TComAdaptiveLoopFilter
{

protected: //protected member variables

  // filter shape information
  static Int weightsShape1Sym[ALF_MAX_NUM_COEF+1];
  static Int *weightsTabShapes[NUM_ALF_FILTER_SHAPE];
  static Int m_sqrFiltLengthTab[NUM_ALF_FILTER_SHAPE];

  // temporary buffer
  TComPicYuv*   m_pcTempPicYuv;                          ///< temporary picture buffer for ALF processing
  TComPicYuv* m_pcSliceYuvTmp;    //!< temporary picture buffer pointer when non-across slice boundary ALF is enabled


  //filter coefficients buffer
  Int **m_filterCoeffSym;

  //classification
  Int      m_varIndTab[NO_VAR_BINS];
  Pel** m_varImg;

  //parameters
  Int   m_img_height;
  Int   m_img_width;
  Bool  m_bUseNonCrossALF;       //!< true for performing non-cross slice boundary ALF
  UInt  m_uiNumSlicesInPic;      //!< number of slices in picture
  Int   m_iSGDepth;              //!< slice granularity depth
  UInt  m_uiNumCUsInFrame;

  Int m_lcuHeight;
  Int m_lineIdxPadBot;
  Int m_lineIdxPadTop;

  Int m_lcuHeightChroma;
  Int m_lineIdxPadBotChroma;
  Int m_lineIdxPadTopChroma;

  //slice
  TComPic* m_pcPic;
  AlfLCUInfo** m_ppSliceAlfLCUs;
  std::vector< AlfLCUInfo* > *m_pvpAlfLCU;
  std::vector< std::vector< AlfLCUInfo* > > *m_pvpSliceTileAlfLCU;

  Int m_suWidth;
  Int m_suHeight;
  Int m_numLCUInPicWidth;
  Int m_numLCUInPicHeight;
  ALFParam** m_alfFiltInfo[NUM_ALF_COMPONENT];
  Bool m_isNonCrossSlice;
  Int m_alfQP;

private: //private member variables


protected: //protected methods

  Void createLCUAlfInfo();
  Void destroyLCUAlfInfo();
  Pel* getPicBuf(TComPicYuv* pPicYuv, Int compIdx);
  Void predictALFCoeffChroma(Int* coeff, Int numCoef= ALF_MAX_NUM_COEF);
  Void assignAlfOnOffControlFlags(TComPic* pcPic, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam);
  Void recALF(Int compIdx, ALFParam** alfLCUParams, Pel* pDec, Pel* pRest, Int stride, Int formatShift, std::vector<AlfCUCtrlInfo>* alfCUCtrlParam, Bool caculateBAIdx);
  Void reconstructCoefInfo(Int compIdx, ALFParam* alfLCUParam, Int** filterCoeff, Int* varIndTab= NULL);
  Void reconstructLumaCoefficients(ALFParam* alfLCUParam, Int** filterCoeff);
  Void reconstructChromaCoefficients(ALFParam* alfLCUParam, Int** filterCoeff);
  Void filterRegion(Int compIdx, ALFParam** alfLCUParams, std::vector<AlfLCUInfo*>& regionLCUInfo, Pel* pDec, Pel* pRest, Int stride, Int formatShift, Bool caculateBAIdx);
  Void filterRegionCUControl(ALFParam** alfLCUParams, std::vector<AlfLCUInfo*>& regionLCUInfo, Pel* pDec, Pel* pRest, Int stride, Bool caculateBAIdx);
  Bool isEnabledComponent(ALFParam** alfLCUParam);
  Int  getAlfPrecisionBit(Int qp);
  Void filterOneCompRegion(Pel *imgRes, Pel *imgPad, Int stride, Bool isChroma, Int yPos, Int yPosEnd, Int xPos, Int xPosEnd, Int** filterSet, Int* mergeTable, Pel** varImg);  
  Void calcOneRegionVar(Pel **imgYvar, Pel *imgYpad, Int stride, Bool isOnlyOneGroup, Int yPos, Int yPosEnd, Int xPos, Int xPosEnd);


  Void InitAlfLCUInfo(AlfLCUInfo& rAlfLCU, Int sliceID, Int tileID, TComDataCU* pcCU, UInt maxNumSUInLCU);
  Void checkFilterCoeffValue( Int *filter, Int filterLength, Bool isChroma );
  Void setAlfCtrlFlags(AlfCUCtrlInfo* pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
  Void transferCtrlFlagsFromAlfParam(std::vector<AlfCUCtrlInfo>& vAlfParamSlices); //!< Copy ALF CU control flags from ALF parameters for slices  
  Void transferCtrlFlagsFromAlfParamOneSlice(std::vector<AlfLCUInfo*> &vpAlfLCU, Bool bCUCtrlEnabled, Int iAlfDepth, std::vector<UInt>& vCtrlFlags); //!< Copy ALF CU control flags from ALF parameter for one slice
  Void extendBorderCoreFunction(Pel* pPel, Int stride, Bool* pbAvail, UInt width, UInt height, UInt extSize); //!< Extend slice boundary border  
  Void copyRegion(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* pPicDst, Pel* pPicSrc, Int stride, Int formatShift = 0);
  Void extendRegionBorder(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* pPelSrc, Int stride, Int formatShift = 0);
  Int  getCtrlFlagsFromAlfParam(AlfLCUInfo* pcAlfLCU, Int iAlfDepth, UInt* puiFlags);

  Void xPCMRestoration        (TComPic* pcPic);
  Void xPCMCURestoration      (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth);
  Void xPCMSampleRestoration  (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, TextType ttText);

public: //public methods, interface functions

  TComAdaptiveLoopFilter();
  virtual ~TComAdaptiveLoopFilter() {}

  Void create  ( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void destroy ();

  Void ALFProcess          (TComPic* pcPic, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam, Bool isAlfCoefInSlice);
  Void resetLCUAlfInfo     ();
  Int  getNumLCUInPicWidth ()  {return m_numLCUInPicWidth;}
  Int  getNumLCUInPicHeight() {return m_numLCUInPicHeight;}

  ALFParam*** getAlfLCUParam() {return m_alfFiltInfo;}
  Int  getNumCUsInPic()  {return m_uiNumCUsInFrame;} //!< get number of LCU in picture for ALF process
  Void createPicAlfInfo (TComPic* pcPic, Int uiNumSlicesInPic = 1, Int alfQP = 26);
  Void destroyPicAlfInfo();

  Void PCMLFDisableProcess    ( TComPic* pcPic);                        ///< interface function for ALF process 

protected: //memory allocation
  Void destroyMatrix_Pel(Pel **m2D);
  Void destroyMatrix_int(int **m2D);
  Void initMatrix_int(int ***m2D, int d1, int d2);
  Void initMatrix_Pel(Pel ***m2D, int d1, int d2);
  Void destroyMatrix4D_double(double ****m4D, int d1, int d2);
  Void destroyMatrix3D_double(double ***m3D, int d1);
  Void destroyMatrix_double(double **m2D);
  Void initMatrix4D_double(double *****m4D, int d1, int d2, int d3, int d4);
  Void initMatrix3D_double(double ****m3D, int d1, int d2, int d3);
  Void initMatrix_double(double ***m2D, int d1, int d2);
  Void no_mem_exit(const char *where);
  Void xError(const char *text, int code);
};

//! \}

#endif
