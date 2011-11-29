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

#define ALF_MAX_NUM_TAP       9                                       ///< maximum number of filter taps (9x9)
#define ALF_MIN_NUM_TAP       5                                       ///< minimum number of filter taps (5x5)
#define ALF_MAX_NUM_TAP_C     5                                       ///< number of filter taps for chroma (5x5)
#define ALF_MAX_NUM_COEF      42                                      ///< maximum number of filter coefficients
#define ALF_MIN_NUM_COEF      14                                      ///< minimum number of filter coefficients
#define ALF_MAX_NUM_COEF_C    14                                      ///< number of filter taps for chroma
#define ALF_NUM_BIT_SHIFT     8                                       ///< bit shift parameter for quantization of ALF param.
#define ALF_ROUND_OFFSET      ( 1 << ( ALF_NUM_BIT_SHIFT - 1 ) )      ///< rounding offset for ALF quantization


#define NUM_BITS               9
#define NO_TEST_FILT           3       // Filter supports (5/7/9)
#define NO_VAR_BINS           16 
#define NO_FILTERS            16

#define VAR_SIZE               1

// max tap = max_horizontal_tap = 11
#define FILTER_LENGTH         11

// ((max_horizontal_tap * max_vertical_tap) / 2 + 2) = ((11 * 5) / 2 + 2)
#define MAX_SQR_FILT_LENGTH   29

#define SQR_FILT_LENGTH_9SYM  ((9*9) / 4 + 2 - 1) 
#define SQR_FILT_LENGTH_7SYM  ((7*7) / 4 + 2) 
#define SQR_FILT_LENGTH_5SYM  ((5*5) / 4 + 2) 
// max_tap + 2 = 11 + 2 
#define MAX_SCAN_VAL    13
#define MAX_EXP_GOLOMB  16

#define imgpel  unsigned short

extern Int depthIntShape0Sym[10];
extern Int depthIntShape1Sym[9];
extern Int *pDepthIntTabShapes[NO_TEST_FILT];
extern Int depthInt9x9Sym[21];
extern Int depthInt7x7Sym[14];
extern Int depthInt5x5Sym[8];
extern Int *pDepthIntTab[NO_TEST_FILT];
void destroyMatrix_int(int **m2D);
void initMatrix_int(int ***m2D, int d1, int d2);


/// border direction ID of slice granularity unit 
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

enum AlfChromaID
{
  ALF_Cb = 0,
  ALF_Cr = 1
};

enum ALFClassficationMethod
{
  ALF_BA =0,
  ALF_RA,
  NUM_ALF_CLASS_METHOD
};
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


/// slice granularity unit information
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

/// LCU-based ALF class for slice processing
class CAlfLCU
{
public:
  CAlfLCU(){ m_pSGU = NULL; m_puiCUCtrlFlag = NULL;}
  ~CAlfLCU(){destroy();}
public:
  /// Create ALF LCU unit perform slice processing
  Void create(Int iSliceID, TComPic* pcPic, UInt uiCUAddr, UInt uiStartSU, UInt uiEndSU, Int iSGDepth);

  /// Destroy ALF LCU unit
  Void destroy();

  /// Extend slice boundary border for one luma LCU
  Void extendLumaBorder(Pel* pImg, Int iStride, Int filtNo);

  /// Extend slice boundary border for one chroma LCU
  Void extendChromaBorder(Pel* pImg, Int iStride, UInt filtNo);

  /// Copy one luma LCU
  Void copyLuma(Pel* pImgDst, Pel* pImgSrc, Int iStride);

  /// Copy one chroma LCU
  Void copyChroma(Pel* pImgDst, Pel* pImgSrc, Int iStride);

  /// Set the neighboring availabilities for one slice granularity unit
  Void setSGUBorderAvailability(UInt uiNumLCUInPicWidth, UInt uiNumLCUInPicHeight, UInt uiNumSUInLCUWidth, UInt uiNumSUInLCUHeight,Int* piSliceIDMap);

  /// Copy ALF CU control flags
  Void getCtrlFlagsFromCU(Int iAlfDepth);

  /// Copy ALF CU control flags from ALF parameters
  Void getCtrlFlagsFromAlfParam(Int iAlfDepth, UInt* puiFlags);

  /// Get number of slice granularity units
  UInt getNumSGU      ()         {return m_uiNumSGU;}

  /// Get number of ALF CU control flags
  Int  getNumCtrlFlags()         {return m_iNumCUCtrlFlags;}

  /// Get ALF CU control flag value
  UInt getCUCtrlFlag  (UInt i)   {return m_puiCUCtrlFlag[i];}

  /// get starting SU z-scan address
  UInt getStartSU     ()         {return m_uiStartSU;}

  /// get ending SU z-scan address
  UInt getEndSU       ()         {return m_uiEndSU;  }

  /// get TComPic pointer
  TComPic*    getPic  ()         {return m_pcPic;}

  /// get TComDataCU pointer
  TComDataCU* getCU   ()         {return m_pcCU;}

  /// Extend slice boundary border
  Void extendBorderCoreFunction(Pel* pPel, Int iStride, Bool* pbAvail, UInt uiWidth, UInt uiHeight, UInt uiExtSizeX, UInt uiExtSizeY, Bool bPaddingForCalculatingBAIndex = false);

  /// get corresponding TComDataCU pointer
  UInt getCUAddr() {return m_uiCUAddr;}
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

/// Slice-based ALF class for slice processing
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
  /// Initialize one ALF slice unit
  Void init(TComPic* pcPic, Int iSGDepth, Int* piSliceSUMap);

  /// Create one ALF slice unit
  Void create(Int iSliceID, UInt uiStartLCU, UInt uiEndLCU);

  /// Destroy one ALF slice unit
  Void destroy();

  /// Extend slice boundary for one luma slice
  Void extendSliceBorderLuma(Pel* pPelSrc, Int iStride, Int filtNo);

  /// Extend slice boundary for one chroma slice
  Void extendSliceBorderChroma(Pel* pPelSrc, Int iStride, UInt filtNo);

  /// Copy one luma slice
  Void copySliceLuma(Pel* pPicDst, Pel* pPicSrc, Int iStride);

  /// Copy one chroma slice
  Void copySliceChroma(Pel* pPicDst, Pel* pPicSrc, Int iStride );

  /// Copy ALF CU Control Flags for one slice
  Void getCtrlFlagsForOneSlice();

  /// Get number of LCUs of this slice
  UInt getNumLCUs      ()          {return m_uiNumLCUs;}

#if F747_APS
  /// Get ALF CU control enabled/disable for this slice
  Bool getCUCtrlEnabled()          {return m_bCUCtrlEnabled;   }
#endif

  /// Set ALF CU control enabled/disable for this slice
  Void setCUCtrlEnabled(Bool b)    {m_bCUCtrlEnabled = b;   }

  /// Set ALF CU control depth of this slice
  void setCUCtrlDepth  (Int iDepth){m_iCUCtrlDepth = iDepth;}

  /// Get ALF CU control depth of this slice
  Int  getCUCtrlDepth  ()          {return m_iCUCtrlDepth;  }

  /// Set number of ALF CU control flags of this slice
  Void setNumCtrlFlags (Int iNum)  {m_iNumCUCtrlFlags = iNum;}

  /// Get number of ALF CU control flags of this slice
  Int  getNumCtrlFlags ()          {return m_iNumCUCtrlFlags;}

  /// Get slice ID
  Int  getSliceID      ()          {return m_iSliceID;}

  /// get TComPic pointer
  TComPic* getPic      ()          {return m_pcPic;}

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


/// adaptive loop filter class
class TComAdaptiveLoopFilter
{
protected:
  // quantized filter coefficients
  static const  Int m_aiSymmetricMag9x9[41];             ///< quantization scaling factor for 9x9 filter
  static const  Int m_aiSymmetricMag7x7[25];             ///< quantization scaling factor for 7x7 filter
  static const  Int m_aiSymmetricMag5x5[13];             ///< quantization scaling factor for 5x5 filter
  static const  Int m_aiSymmetricMag9x7[32];             ///< quantization scaling factor for 9x7 filter
  
  // temporary picture buffer
  TComPicYuv*   m_pcTempPicYuv;                          ///< temporary picture buffer for ALF processing
  
  // ------------------------------------------------------------------------------------------------------------------
  // For luma component
  // ------------------------------------------------------------------------------------------------------------------
  static Int patternShape0Sym[17];
  static Int weightsShape0Sym[10];
  static Int patternShape0Sym_Quart[29];
  static Int patternShape1Sym[15];
  static Int weightsShape1Sym[9];
  static Int patternShape1Sym_Quart[29];
  static Int *patternTabFiltShapes[NO_TEST_FILT];
  static Int *patternTabShapes[NO_TEST_FILT]; 
  static Int *patternMapTabShapes[NO_TEST_FILT];
  static Int *weightsTabShapes[NO_TEST_FILT];
  static Int m_pattern9x9Sym[39];
  static Int m_weights9x9Sym[21];
  static Int m_pattern9x9Sym_Quart[42];
  static Int m_pattern7x7Sym[25];
  static Int m_weights7x7Sym[14];
  static Int m_pattern7x7Sym_Quart[42];
  static Int m_pattern5x5Sym[13];
  static Int m_weights5x5Sym[8];
  static Int m_pattern5x5Sym_Quart[45];
  static Int pattern11x5SymShape0[17];
  static Int pattern11x5SymShape1[15];
  static Int pattern11x5Sym11x5[55];
  static Int m_pattern9x9Sym_9[39];
  static Int m_pattern9x9Sym_7[25];
  static Int m_pattern9x9Sym_5[13];
  
  static Int *m_patternTab_filt[NO_TEST_FILT];
  static Int m_flTab[NO_TEST_FILT];
  static Int *m_patternTab[NO_TEST_FILT]; 
  static Int *m_patternMapTab[NO_TEST_FILT];
  static Int *m_weightsTab[NO_TEST_FILT];
  static Int m_sqrFiltLengthTab[NO_TEST_FILT];
  
  Int m_img_height,m_img_width;
  
  imgpel **m_imgY_pad;
  imgpel **m_imgY_var;
  Int    **m_imgY_temp;
  
  Int**    m_imgY_ver;
  Int**    m_imgY_hor;
  UInt     m_uiVarGenMethod;
  imgpel** m_varImgMethods[NUM_ALF_CLASS_METHOD];

  Int **m_filterCoeffSym;
  Int **m_filterCoeffPrevSelected;
  Int **m_filterCoeffTmp;
  Int **m_filterCoeffSymTmp;
  

  Bool        m_bUseNonCrossALF;       //!< true for performing non-cross slice boundary ALF

  UInt        m_uiNumSlicesInPic;      //!< number of slices in picture
  CAlfSlice*  m_pSlice;                //!< ALF slice units
  Bool        m_bIsFirstDecodedSlice;  //!< true for the first decoding slice
  Int         m_iSGDepth;              //!< slice granularity depth
  Int*        m_piSliceSUMap;          //!< slice ID map

  //// Perform ALF for one luma slice
  Void xFilterOneSlice            (CAlfSlice* pSlice, imgpel* pDec, imgpel* pRest, Int iStride, ALFParam* pcAlfParam);

  /// Calculate ALF grouping indices for one slice
  Void calcVarforOneSlice         (CAlfSlice* pSlice, imgpel **imgY_var, imgpel *imgY_pad, Int pad_size, Int fl, Int img_stride);

  /// Perform ALF for one chroma slice
  Void xFrameChromaforOneSlice    (CAlfSlice* pSlice, Int ComponentID, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap);

  Void createRegionIndexMap(imgpel **imgY_var, Int img_width, Int img_height);

  /// ALF for luma component
#if F747_APS
  Void xALFLuma_qc( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
#else
  Void xALFLuma_qc( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
#endif

  Void reconstructFilterCoeffs(ALFParam* pcAlfParam,int **pfilterCoeffSym, int bit_depth);
  Void getCurrentFilter(int **filterCoeffSym,ALFParam* pcAlfParam);
  // memory allocation
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
  Void free_mem2Dpel(imgpel **array2D);
  Void get_mem2Dpel(imgpel ***array2D, int rows, int columns);
  Void no_mem_exit(const char *where);
  Void xError(const char *text, int code);
  Void calcVar(int ypos, int xpos, imgpel **imgY_var, imgpel *imgY_pad, int pad_size, int fl, int img_height, int img_width, int img_stride);
  Void DecFilter_qc(imgpel* imgY_rec,ALFParam* pcAlfParam, int Stride);
  Void xSubCUAdaptive_qc(TComDataCU* pcCU, ALFParam* pcAlfParam, imgpel *imgY_rec_post, imgpel *imgY_rec, UInt uiAbsPartIdx, UInt uiDepth, Int Stride);
  Void xCUAdaptive_qc(TComPic* pcPic, ALFParam* pcAlfParam, imgpel *imgY_rec_post, imgpel *imgY_rec, Int Stride);
  Void subfilterFrame(imgpel *imgY_rec_post, imgpel *imgY_rec, int filtNo, int start_height, int end_height, int start_width, int end_width, int Stride);
  Void filterFrame(imgpel *imgY_rec_post, imgpel *imgY_rec, int filtNo, int Stride);
  UInt  m_uiNumCUsInFrame;
#if F747_APS
  Void  setAlfCtrlFlags(AlfCUCtrlInfo* pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
#else
  Void  setAlfCtrlFlags (ALFParam *pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
#endif

#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  Void xPCMRestoration        (TComPic* pcPic);
  Void xPCMCURestoration      (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth);
  Void xPCMSampleRestoration  (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, TextType ttText);
#endif

  // ------------------------------------------------------------------------------------------------------------------
  // For chroma component
  // ------------------------------------------------------------------------------------------------------------------
  
  /// ALF for chroma component
  Void xALFChroma   ( ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  
  /// sub function: non-adaptive ALF process for chroma
  Void xFrameChroma ( Int ypos, Int xpos, Int iHeight, Int iWidth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor );

public:
  TComAdaptiveLoopFilter();
  virtual ~TComAdaptiveLoopFilter() {}
  
  // initialize & destory temporary buffer
  Void create  ( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void destroy ();
  
  // alloc & free & set functions
  Void allocALFParam  ( ALFParam* pAlfParam );
  Void freeALFParam   ( ALFParam* pAlfParam );
  Void copyALFParam   ( ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam );
  
  // predict filter coefficients
  Void predictALFCoeff        ( ALFParam* pAlfParam );                  ///< prediction of luma ALF coefficients
  Void predictALFCoeffChroma  ( ALFParam* pAlfParam );                  ///< prediction of chroma ALF coefficients
  
  // interface function
#if F747_APS
  Void ALFProcess             ( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam ); ///< interface function for ALF process
#else
  Void ALFProcess             ( TComPic* pcPic, ALFParam* pcAlfParam ); ///< interface function for ALF process
#endif

#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  Void PCMLFDisableProcess    ( TComPic* pcPic);                        ///< interface function for ALF process 
#endif

  static Int ALFTapHToTapV(Int tapH);
  static Int ALFTapHToNumCoeff(Int tapH);
  static Int ALFFlHToFlV(Int flH);

#if F747_APS
public:
  /// get number of LCU in picture for ALF process
  Int  getNumCUsInPic()  {return m_uiNumCUsInFrame;}
#endif

public:

#if F747_APS
  /// Copy ALF CU control flags from ALF parameters for slices
  Void transferCtrlFlagsFromAlfParam(std::vector<AlfCUCtrlInfo>& vAlfParamSlices);
  /// Copy ALF CU control flags from ALF parameter for one slice
  Void transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, std::vector<UInt>& vCtrlFlags);
#else
  /// Copy ALF CU control flags from ALF parameters for slices
  Void transferCtrlFlagsFromAlfParam(ALFParam* pcAlfParam);
  
  /// Copy ALF CU control flags from ALF parameter for one slice
  Void transferCtrlFlagsFromAlfParamOneSlice(UInt s, Bool bCUCtrlEnabled, Int iAlfDepth, UInt* puiFlags);
#endif

#if FINE_GRANULARITY_SLICES
  /// Set slice granularity
  Void setSliceGranularityDepth(Int iDepth) { m_iSGDepth = iDepth;}

  /// get slice granularity
  Int  getSliceGranularityDepth()           { return m_iSGDepth;  }
#endif
  /// Set number of slices in picture
  Void setNumSlicesInPic(UInt uiNum) {m_uiNumSlicesInPic = uiNum;}

  /// Get number of slices in picture
  UInt getNumSlicesInPic()           {return m_uiNumSlicesInPic;}

  /// Set across/non-across slice boundary ALF
  Void setUseNonCrossAlf(Bool bVal)  {m_bUseNonCrossALF = bVal;}

  /// Get across/non-across slice boundary ALF
  Bool getUseNonCrossAlf()           {return m_bUseNonCrossALF;}

  /// Create ALF slice units
  Void createSlice (TComPic* pcPic);

  /// Destroy ALF slice units
  Void destroySlice     ();

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
