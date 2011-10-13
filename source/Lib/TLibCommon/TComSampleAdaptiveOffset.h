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

/** \file     TComSampleAdaptiveOffset.h
    \brief    sample adaptive offset class (header)
*/

#ifndef __TCOMSAMPLEADAPTIVEOFFSET__
#define __TCOMSAMPLEADAPTIVEOFFSET__

#include "CommonDef.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#if SAO

#define SAO_MAX_DEPTH                 4
#define SAO_BO_BITS                   5
#define LUMA_GROUP_NUM                (1<<SAO_BO_BITS)
#define MAX_NUM_SAO_CLASS             32
#define SAO_RDCO                      0
#define SAO_FGS_NIF                   SAO && FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// Sample Adaptive Offset class
class TComSampleAdaptiveOffset
{
protected:
  TComPic*          m_pcPic;

  static UInt m_uiMaxDepth;
  static const Int m_aiNumPartsInRow[5];
  static const Int m_aiNumPartsLevel[5];
  static const Int m_aiNumCulPartsLevel[5];
  static const UInt m_auiEoTable[9];
  static const UInt m_auiEoTable2D[9];
  static const UInt m_iWeightSao[MAX_NUM_SAO_TYPE];
  Int *m_iOffsetBo;
  Int m_iOffsetEo[LUMA_GROUP_NUM];

  Int  m_iPicWidth;
  Int  m_iPicHeight;
  UInt m_uiMaxSplitLevel;
  UInt m_uiMaxCUWidth;
  UInt m_uiMaxCUHeight;
  Int  m_iNumCuInWidth;
  Int  m_iNumCuInHeight;
  Int  m_iNumTotalParts;
  static Int m_iNumClass[MAX_NUM_SAO_TYPE];
  SliceType  m_eSliceType;
  Int        m_iPicNalReferenceIdc;

  UInt m_uiSaoBitIncrease;
  UInt m_uiQP;

  Pel   *m_pClipTable;
  Pel   *m_pClipTableBase;
  Pel   *m_ppLumaTableBo0;
  Pel   *m_ppLumaTableBo1;

  Int   *m_iUpBuff1;
  Int   *m_iUpBuff2;
  Int   *m_iUpBufft;
  Int   *ipSwap;
#if SAO_FGS_NIF
  Bool  m_bUseNIF;       //!< true for performing non-cross slice boundary ALF
  UInt  m_uiNumSlicesInPic;      //!< number of slices in picture
  Int   m_iSGDepth;              //!< slice granularity depth
  Bool  *m_bIsFineSliceCu;
  TComPicYuv* m_pcPicYuvMap;
#endif
#if SAO_CROSS_LCU_BOUNDARIES
  Pel* m_pTmpU1;
  Pel* m_pTmpU2;
  Pel* m_pTmpL1;
  Pel* m_pTmpL2;
  Int* m_iLcuPartIdx;
  Void initTmpSaoQuadTree(SAOQTPart *psQTPart, Int iYCbCr);
  Void disableSaoOnePart(SAOQTPart *psQTPart, UInt uiPartIdx, Int iYCbCr);
  Void xSaoQt2Lcu(SAOQTPart *psQTPart,UInt uiPartIdx);
  Void convertSaoQt2Lcu(SAOQTPart *psQTPart,UInt uiPartIdx);
  Void xSaoAllPart(SAOQTPart *psQTPart, Int iYCbCr);
#endif

public:
  TComSampleAdaptiveOffset         ();
  virtual ~TComSampleAdaptiveOffset();

  Void create( UInt uiSourceWidth, UInt uiSourceHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void destroy ();

  Int  convertLevelRowCol2Idx(int level, int row, int col);
  void convertIdx2LevelRowCol(int idx, int *level, int *row, int *col);

  Void initSAOParam   (SAOParam *pcSaoParam, Int iPartLevel, Int iPartRow, Int iPartCol, Int iParentPartIdx, Int StartCUX, Int EndCUX, Int StartCUY, Int EndCUY, Int iYCbCr);
  Void allocSaoParam  (SAOParam* pcSaoParam);
  Void resetSAOParam  (SAOParam *pcSaoParam);
  Void freeSaoParam   (SAOParam *pcSaoParam);

  Void SAOProcess(TComPic* pcPic, SAOParam* pcSaoParam);
  Void processSaoCu(Int iAddr, Int iSaoType, Int iYCbCr);
  Void processSaoOnePart(SAOQTPart *psQTPart, UInt uiPartIdx, Int iYCbCr);
  Void processSaoQuadTree(SAOQTPart *psQTPart, UInt uiPartIdx, Int iYCbCr);
  Pel* getPicYuvAddr(TComPicYuv* pcPicYuv, Int iYCbCr,Int iAddr);

#if SAO_FGS_NIF
  Void processSaoCuOrg(Int iAddr, Int iPartIdx, Int iYCbCr);  //!< LCU-basd SAO process without slice granularity 
  Void processSaoCuMap(Int iAddr, Int iPartIdx, Int iYCbCr);  //!< LCU-basd SAO process with slice granularity
  Void setNumSlicesInPic(UInt uiNum) {m_uiNumSlicesInPic = uiNum;}  //!< set num of slices in picture
  UInt getNumSlicesInPic()           {return m_uiNumSlicesInPic;}   //!< get num of slices in picture
  Void setUseNIF(Bool bVal)  {m_bUseNIF = bVal;}    //!< set use non-cross-slice-boundaries in-loop filter (NIF)
  Bool getUseNIF()           {return m_bUseNIF;}    //!< get use non-cross-slice-boundaries in-loop filter (NIF)
  Void setSliceGranularityDepth(Int iDepth) { m_iSGDepth = iDepth; }//!< set slice granularity depth
  Int  getSliceGranularityDepth()           { return m_iSGDepth;   }//!< get slice granularity depth
  Void createSliceMap(UInt iSliceIdx, UInt uiStartAddr, UInt uiEndAddr);//!< create slice map
  Void InitIsFineSliceCu(){memset(m_bIsFineSliceCu,0, sizeof(Bool)*m_iNumCuInWidth*m_iNumCuInHeight);} //!< Init is fine slice LCU
  Void setPic(TComPic* pcPic){m_pcPic = pcPic;} //!< set pic
#endif

};
#endif

//! \}

#endif
