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

#define ALF_MAX_NUM_COEF      10                                      //!< maximum number of filter coefficients
#define MAX_SQR_FILT_LENGTH   29                                      //!< ((max_horizontal_tap * max_vertical_tap) / 2 + 2) = ((11 * 5) / 2 + 2)

#define ALF_NUM_BIT_SHIFT     8                                       ///< bit shift parameter for quantization of ALF param.

#define VAR_SIZE_H            4
#define VAR_SIZE_W            4
#define NO_VAR_BINS           16 
#define NO_FILTERS            16
#define VAR_SIZE               1

#define MAX_SCAN_VAL          13
#define MAX_EXP_GOLOMB        16

#define imgpel  unsigned short

///
/// border direction ID of ALF processing block
///
enum SGUBorderID
{
  SGU_L = 0,
  SGU_R,
  SGU_T,
  SGU_B,
  SGU_TL,
  SGU_TR,
  SGU_BL,
  SGU_BR,
  NUM_SGU_BORDER
};

///
/// Chroma component ID
///
enum AlfChromaID
{
  ALF_Cb = 0,
  ALF_Cr = 1
};

///
/// Adaptation mode ID
///
enum ALFClassficationMethod
{
  ALF_BA =0,
  ALF_RA,
  NUM_ALF_CLASS_METHOD
};

///
/// Filter shape
///
enum ALFFilterShape
{
  ALF_STAR5x5 = 0,
  ALF_CROSS11x5,
  NUM_ALF_FILTER_SHAPE
};

extern Int depthIntShape0Sym[10];
extern Int depthIntShape1Sym[9];
extern Int *pDepthIntTabShapes[NUM_ALF_FILTER_SHAPE];

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if F747_APS
/// ALF CU control parameters
struct AlfCUCtrlInfo
{
  Int  cu_control_flag;                    //!< slice-level ALF CU control enabled/disabled flag 
  UInt num_alf_cu_flag;                    //!< number of ALF CU control flags
  UInt alf_max_depth;                      //!< ALF CU control depth
  std::vector<UInt> alf_cu_flag;           //!< ALF CU control flags (container)

  const AlfCUCtrlInfo& operator= (const AlfCUCtrlInfo& src);  //!< "=" operator
  AlfCUCtrlInfo():cu_control_flag(0), num_alf_cu_flag(0), alf_max_depth(0) {} //!< constructor

};
#endif

///
/// ALF processing block information
///
struct AlfSGUInfo
{
  Int   sliceID;  //!< slice ID
  UInt  startSU;  //!< starting SU z-scan address in LCU
  UInt  endSU;    //!< ending SU z-scan address in LCU
  UInt  widthSU;  //!< number of SUs in width
  UInt  heightSU; //!< number of SUs in height
  UInt  posX;     //!< top-left X coordinate in picture
  UInt  posY;     //!< top-left Y coordinate in picture
  UInt  width;    //!< number of pixels in width
  UInt  height;   //!< number of pixels in height
  Bool  isBorderAvailable[NUM_SGU_BORDER];  //!< the border availabilities
};

///
/// LCU-based ALF class for slice processing
///
class CAlfLCU
{
public:
  CAlfLCU(){ m_pSGU = NULL; m_puiCUCtrlFlag = NULL;}
  ~CAlfLCU(){destroy();}
public:

  Void create(Int iSliceID, TComPic* pcPic, UInt uiCUAddr, UInt uiStartSU, UInt uiEndSU, Int iSGDepth); /// Create ALF LCU unit perform slice processing
  Void destroy(); /// Destroy ALF LCU unit
  Void extendLumaBorder(Pel* pImg, Int iStride, Int filtNo); /// Extend slice boundary border for one luma LCU
  Void extendChromaBorder(Pel* pImg, Int iStride, UInt filtNo); /// Extend slice boundary border for one chroma LCU
  Void copyLuma(Pel* pImgDst, Pel* pImgSrc, Int iStride); /// Copy one luma LCU
  Void copyChroma(Pel* pImgDst, Pel* pImgSrc, Int iStride); /// Copy one chroma LCU
  Void setSGUBorderAvailability(UInt uiNumLCUInPicWidth, UInt uiNumLCUInPicHeight, UInt uiNumSUInLCUWidth, UInt uiNumSUInLCUHeight,Int* piSliceIDMap); /// Set the neighboring availabilities for one slice granularity unit
  Void getCtrlFlagsFromCU(Int iAlfDepth); /// Copy ALF CU control flags
  Void getCtrlFlagsFromAlfParam(Int iAlfDepth, UInt* puiFlags); /// Copy ALF CU control flags from ALF parameters
  UInt getNumSGU      ()         {return m_uiNumSGU;} /// Get number of slice granularity units
  Int  getNumCtrlFlags()         {return m_iNumCUCtrlFlags;} /// Get number of ALF CU control flags
  UInt getCUCtrlFlag  (UInt i)   {return m_puiCUCtrlFlag[i];} /// Get ALF CU control flag value
  UInt getStartSU     ()         {return m_uiStartSU;} /// get starting SU z-scan address
  UInt getEndSU       ()         {return m_uiEndSU;  } /// get ending SU z-scan address
  TComPic*    getPic  ()         {return m_pcPic;} /// get TComPic pointer
  TComDataCU* getCU   ()         {return m_pcCU;} /// get TComDataCU pointer
  Void extendBorderCoreFunction(Pel* pPel, Int iStride, Bool* pbAvail, UInt uiWidth, UInt uiHeight, UInt uiExtSizeX, UInt uiExtSizeY, Bool bPaddingForCalculatingBAIndex = false); /// Extend slice boundary border
  UInt getCUAddr() {return m_uiCUAddr;} /// get corresponding TComDataCU pointer
private:
  TComPic*    m_pcPic;           //!< TComPic pointer
  TComDataCU* m_pcCU;            //!< TComDataCU pointer
  UInt        m_uiCUAddr;        //!< LCU rater-scan address in picture
  Int         m_iSliceID;        //!< slice ID
  UInt        m_uiNumSGU;        //!< number of slice granularity blocks 
  UInt        m_uiStartSU;       //!< starting SU z-scan address in LCU
  UInt        m_uiEndSU;         //!< ending SU z-scan address in LCU
  AlfSGUInfo* m_pSGU;            //!< slice granularity unit data
  Int         m_iNumCUCtrlFlags; //!< number of ALF CU control flags
  UInt*       m_puiCUCtrlFlag;   //!< ALF CU contrl flags buffer
public:  
  /// operator for accessing slice granularity units
  AlfSGUInfo& operator[] (Int idx)
  {
    assert(idx < m_uiNumSGU);
    return m_pSGU[idx];
  }

};

///
/// Slice-based ALF class for slice processing
///
class CAlfSlice
{
public:
  CAlfSlice()
  {
    m_pcAlfLCU = NULL;
  }
  ~CAlfSlice()
  {
    destroy(); 
  }
public:
  Void init(TComPic* pcPic, Int iSGDepth, Int* piSliceSUMap); //!< Initialize one ALF slice unit
  Void create(Int iSliceID, UInt uiStartLCU, UInt uiEndLCU); //!< Create one ALF slice unit
  Void destroy(); //!< Destroy one ALF slice unit
  Void extendSliceBorderLuma(Pel* pPelSrc, Int iStride, Int filtNo); //!< Extend slice boundary for one luma slice
  Void extendSliceBorderChroma(Pel* pPelSrc, Int iStride, UInt filtNo); //!< Extend slice boundary for one chroma slice
  Void copySliceLuma(Pel* pPicDst, Pel* pPicSrc, Int iStride); //!< Copy one luma slice
  Void copySliceChroma(Pel* pPicDst, Pel* pPicSrc, Int iStride ); //!< Copy one chroma slice
  Void getCtrlFlagsForOneSlice(); //!< Copy ALF CU Control Flags for one slice
  UInt getNumLCUs      ()          {return m_uiNumLCUs;} //!< Get number of LCUs of this slice
#if F747_APS
  Bool getCUCtrlEnabled()          {return m_bCUCtrlEnabled;   } /// Get ALF CU control enabled/disable for this slice
#endif
  Void setCUCtrlEnabled(Bool b)    {m_bCUCtrlEnabled = b;   } /// Set ALF CU control enabled/disable for this slice
  void setCUCtrlDepth  (Int iDepth){m_iCUCtrlDepth = iDepth;} /// Set ALF CU control depth of this slice
  Int  getCUCtrlDepth  ()          {return m_iCUCtrlDepth;  } /// Get ALF CU control depth of this slice
  Void setNumCtrlFlags (Int iNum)  {m_iNumCUCtrlFlags = iNum;} /// Set number of ALF CU control flags of this slice
  Int  getNumCtrlFlags ()          {return m_iNumCUCtrlFlags;} /// Get number of ALF CU control flags of this slice
  Int  getSliceID      ()          {return m_iSliceID;} /// Get slice ID
  TComPic* getPic      ()          {return m_pcPic;} /// get TComPic pointer
  Bool isValidSlice()               {return m_bValidSlice;}

private: 
  Bool     m_bValidSlice;
  TComPic* m_pcPic;                  //!< pointer to TComPic
  Int*     m_piSliceSUMap;           //!< pointer to slice ID map
  CAlfLCU* m_pcAlfLCU;               //!< ALF LCU units
  Int      m_iSliceID;               //!< slice ID 
  UInt     m_uiStartLCU;             //!< starting LCU raster-scan address in picture
  UInt     m_uiEndLCU;               //!< ending LCU raster-scan address in picture
  UInt     m_uiNumLCUs;              //!< number of LCUs in this slices
  Int      m_iSGDepth;               //!< slice granularity
  UInt     m_uiFirstCUInStartLCU;    //!< first SU z-scan address of the starting LCU
  UInt     m_uiLastCUInEndLCU;       //!< last  SU z-scan address of the ending LCU
  Bool     m_bCUCtrlEnabled;         //!< ALF CU control enabled/disabled
  Int      m_iCUCtrlDepth;           //!< ALF CU control depth
  Int      m_iNumCUCtrlFlags;        //!< number of ALF CU control flags
public:  
  //operator to access CAlfLCU
  CAlfLCU& operator[] (Int idx)
  {
    assert(idx < m_uiNumLCUs);
    return m_pcAlfLCU[idx];
  }

};

///
/// adaptive loop filter class
///
class TComAdaptiveLoopFilter
{

protected: //protected member variables

  // filter shape information
  static Int weightsShape0Sym[10];
  static Int weightsShape1Sym[9];
  static Int *weightsTabShapes[NUM_ALF_FILTER_SHAPE];
  static Int m_sqrFiltLengthTab[NUM_ALF_FILTER_SHAPE];

  // temporary buffer
  TComPicYuv*   m_pcTempPicYuv;                          ///< temporary picture buffer for ALF processing


  //filter coefficients buffer
  Int **m_filterCoeffSym;

  //classification
  Int      m_varIndTab[NO_VAR_BINS];
  UInt     m_uiVarGenMethod;
  imgpel** m_varImgMethods[NUM_ALF_CLASS_METHOD];
  imgpel** m_varImg;

  //parameters
  Int   m_img_height;
  Int   m_img_width;
  Bool  m_bUseNonCrossALF;       //!< true for performing non-cross slice boundary ALF
  UInt  m_uiNumSlicesInPic;      //!< number of slices in picture
  Int   m_iSGDepth;              //!< slice granularity depth
  UInt  m_uiNumCUsInFrame;

  //slice
  CAlfSlice*  m_pSlice;                //!< ALF slice units
  Int*        m_piSliceSUMap;          //!< slice ID map

private: //private member variables


protected: //protected methods


  Void calcVarforOneSlice         (CAlfSlice* pSlice, imgpel **imgY_var, imgpel *imgY_pad, Int fl, Int img_stride); //! Calculate ALF grouping indices for one slice
  Void createRegionIndexMap(imgpel **imgY_var, Int img_width, Int img_height); //!< create RA index for regions
  Void calcVar(int ypos, int xpos, imgpel **imgY_var, imgpel *imgY_pad, int fl, int img_height, int img_width, int img_stride);

  Void filterLuma(imgpel *pImgYRes, imgpel *pImgYPad, Int stride, Int ypos, Int yposEnd, Int xpos, Int xposEnd, Int filtNo, Int** filterSet, Int* mergeTable, imgpel** ppVarImg); //!< filtering operation for luma region
  Void filterChroma(imgpel *pImgRes, imgpel *pImgPad, Int stride, Int ypos, Int yposEnd, Int xpos, Int xposEnd, Int filtNo, Int* coef);
  Void xFilterOneSlice             (CAlfSlice* pSlice, imgpel* pDec, imgpel* pRest, Int iStride, ALFParam* pcAlfParam); //!< Perform ALF for one luma slice
  Void xFilterOneChromaSlice(CAlfSlice* pSlice, imgpel* pDec, imgpel* pRest, Int iStride, Int *coeff, Int filtNo, Int iChromaFormatShift); //!< ALF for chroma component
  Void xCUAdaptive   (TComPic* pcPic, Int filtNo, imgpel *imgYFilt, imgpel *imgYRec, Int Stride);
  Void xSubCUAdaptive(TComDataCU* pcCU, Int filtNo, imgpel *imgYFilt, imgpel *imgYRec, UInt uiAbsPartIdx, UInt uiDepth, Int Stride);
  Void reconstructFilterCoeffs(ALFParam* pcAlfParam,int **pfilterCoeffSym);
  Void decodeFilterSet(ALFParam* pcAlfParam, Int* varIndTab, Int** filterCoeff);
  Void xALFChroma   ( ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xFilterChromaOneCmp(imgpel *pDec, imgpel *pRest, Int iStride, Int iShape, Int *pCoeff);
#if F747_APS
  Void xALFLuma( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void setAlfCtrlFlags(AlfCUCtrlInfo* pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
  Void transferCtrlFlagsFromAlfParam(std::vector<AlfCUCtrlInfo>& vAlfParamSlices); //!< Copy ALF CU control flags from ALF parameters for slices  
  Void transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, std::vector<UInt>& vCtrlFlags); //!< Copy ALF CU control flags from ALF parameter for one slice
#else
  Void  setAlfCtrlFlags (ALFParam *pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
  Void transferCtrlFlagsFromAlfParam(ALFParam* pcAlfParam); //!< Copy ALF CU control flags from ALF parameters for slices  
  Void transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, UInt* puiFlags); //!< Copy ALF CU control flags from ALF parameter for one slice
  Void xALFLuma( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
#endif


  Void xPCMRestoration        (TComPic* pcPic);
  Void xPCMCURestoration      (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth);
  Void xPCMSampleRestoration  (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, TextType ttText);

public: //public methods, interface functions

  TComAdaptiveLoopFilter();
  virtual ~TComAdaptiveLoopFilter() {}

  Void create  ( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void destroy ();
  Void predictALFCoeffChroma  ( ALFParam* pAlfParam );                  //!< prediction of chroma ALF coefficients

#if F747_APS
  Void ALFProcess             ( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam ); ///< interface function for ALF process
#else
  Void ALFProcess             ( TComPic* pcPic, ALFParam* pcAlfParam ); ///< interface function for ALF process
#endif

  Void allocALFParam  ( ALFParam* pAlfParam ); //!< allocate ALF parameters
  Void freeALFParam   ( ALFParam* pAlfParam ); //!< free ALF parameters
  Void copyALFParam   ( ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam ); //!< copy ALF parameters

#if F747_APS
  Int  getNumCUsInPic()  {return m_uiNumCUsInFrame;} //!< get number of LCU in picture for ALF process
#endif

#if FINE_GRANULARITY_SLICES
  Void setSliceGranularityDepth(Int iDepth) { m_iSGDepth = iDepth;} //!< Set slice granularity
  Int  getSliceGranularityDepth()           { return m_iSGDepth;  } //!< get slice granularity
#endif
  Void setNumSlicesInPic(UInt uiNum) {m_uiNumSlicesInPic = uiNum;} //!< Set number of slices in picture
  UInt getNumSlicesInPic()           {return m_uiNumSlicesInPic;} //!< Get number of slices in picture
  Void setUseNonCrossAlf(Bool bVal)  {m_bUseNonCrossALF = bVal;} //!< Set across/non-across slice boundary ALF
  Bool getUseNonCrossAlf()           {return m_bUseNonCrossALF;} //!< Get across/non-across slice boundary ALF
  Void createSlice (TComPic* pcPic); //!< Create ALF slice units
  Void destroySlice     (); //!< Destroy ALF slice units

#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  Void PCMLFDisableProcess    ( TComPic* pcPic);                        ///< interface function for ALF process 
#endif

protected: //memory allocation
  Void destroyMatrix_imgpel(imgpel **m2D);
  Void destroyMatrix_int(int **m2D);
  Void initMatrix_int(int ***m2D, int d1, int d2);
  Void initMatrix_imgpel(imgpel ***m2D, int d1, int d2);
  Void destroyMatrix4D_double(double ****m4D, int d1, int d2);
  Void destroyMatrix3D_double(double ***m3D, int d1);
  Void destroyMatrix_double(double **m2D);
  Void initMatrix4D_double(double *****m4D, int d1, int d2, int d3, int d4);
  Void initMatrix3D_double(double ****m3D, int d1, int d2, int d3);
  Void initMatrix_double(double ***m2D, int d1, int d2);
  Void no_mem_exit(const char *where);
  Void xError(const char *text, int code);

public: 
  //operator to access Alf slice units
  CAlfSlice& operator[] (UInt i)
  {
    assert(i < m_uiNumSlicesInPic);
    return m_pSlice[i];
  }

};

//! \}

#endif
