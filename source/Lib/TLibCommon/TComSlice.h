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

/** \file     TComSlice.h
    \brief    slice header and SPS class (header)
*/

#ifndef __TCOMSLICE__
#define __TCOMSLICE__

#include <cstring>
#include "CommonDef.h"
#include "TComRom.h"
#include "TComList.h"

//! \ingroup TLibCommon
//! \{

class TComPic;


// ====================================================================================================================
// Constants
// ====================================================================================================================

#if F747_APS
/// max number of supported APS in software
#define MAX_NUM_SUPPORTED_APS 1
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SPS class
class TComSPS
{
private:
  Int         m_SPSId;
  Int         m_ProfileIdc;
  Int         m_LevelIdc;

  UInt        m_uiMaxTLayers;           // maximum number of temporal layers

  // Structure
  UInt        m_uiWidth;
  UInt        m_uiHeight;
  Int         m_aiPad[2];
  UInt        m_uiMaxCUWidth;
  UInt        m_uiMaxCUHeight;
  UInt        m_uiMaxCUDepth;
  UInt        m_uiMinTrDepth;
  UInt        m_uiMaxTrDepth;
#if G1002_RPS
  UInt        m_uiMaxNumberOfReferencePictures;
  UInt        m_uiMaxNumberOfReorderPictures;
#endif
  
  // Tool list
  UInt        m_uiQuadtreeTULog2MaxSize;
  UInt        m_uiQuadtreeTULog2MinSize;
  UInt        m_uiQuadtreeTUMaxDepthInter;
  UInt        m_uiQuadtreeTUMaxDepthIntra;
#if DISABLE_4x4_INTER
  Bool        m_bDisInter4x4;
#endif
#if AMP
  Bool        m_useAMP;
#endif
  Bool        m_bUsePAD;
  Bool        m_bUseMRG; // SOPH:

  Bool        m_bUseLMChroma; // JL:

#if NSQT
  Bool        m_useNSQT;
#endif
  
  // Parameter
  UInt        m_uiBitDepth;
  UInt        m_uiBitIncrement;

#if G1002_RPS
  UInt        m_uiBitsForPOC;
#endif
  // Max physical transform size
  UInt        m_uiMaxTrSize;
  
  Int m_iAMPAcc[MAX_CU_DEPTH];

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  Bool        m_bUseNewRefSetting;
  UInt        m_uiMaxNumRefFrames;
#endif
#endif

public:
  TComSPS();
  virtual ~TComSPS();

  Int  getSPSId       ()         { return m_SPSId;          }
  Void setSPSId       (Int i)    { m_SPSId = i;             }
  Int  getProfileIdc  ()         { return m_ProfileIdc;     }
  Void setProfileIdc  (Int i)    { m_ProfileIdc = i;        }
  Int  getLevelIdc    ()         { return m_LevelIdc;       }
  Void setLevelIdc    (Int i)    { m_LevelIdc = i;          }
  
  // structure
  Void setWidth       ( UInt u ) { m_uiWidth = u;           }
  UInt getWidth       ()         { return  m_uiWidth;       }
  Void setHeight      ( UInt u ) { m_uiHeight = u;          }
  UInt getHeight      ()         { return  m_uiHeight;      }
  Void setMaxCUWidth  ( UInt u ) { m_uiMaxCUWidth = u;      }
  UInt getMaxCUWidth  ()         { return  m_uiMaxCUWidth;  }
  Void setMaxCUHeight ( UInt u ) { m_uiMaxCUHeight = u;     }
  UInt getMaxCUHeight ()         { return  m_uiMaxCUHeight; }
  Void setMaxCUDepth  ( UInt u ) { m_uiMaxCUDepth = u;      }
  UInt getMaxCUDepth  ()         { return  m_uiMaxCUDepth;  }
#if G1002_RPS
  Void setBitsForPOC  ( UInt u ) { m_uiBitsForPOC = u;      }
  UInt getBitsForPOC  ()         { return m_uiBitsForPOC;   }
#endif
#if DISABLE_4x4_INTER
  Bool getDisInter4x4()         { return m_bDisInter4x4;        }
  Void setDisInter4x4      ( Bool b ) { m_bDisInter4x4  = b;          }
#endif
#if AMP
  Bool getUseAMP() { return m_useAMP; }
  Void setUseAMP( Bool b ) { m_useAMP = b; }
#endif
  Void setMinTrDepth  ( UInt u ) { m_uiMinTrDepth = u;      }
  UInt getMinTrDepth  ()         { return  m_uiMinTrDepth;  }
  Void setMaxTrDepth  ( UInt u ) { m_uiMaxTrDepth = u;      }
  UInt getMaxTrDepth  ()         { return  m_uiMaxTrDepth;  }
  Void setQuadtreeTULog2MaxSize( UInt u ) { m_uiQuadtreeTULog2MaxSize = u;    }
  UInt getQuadtreeTULog2MaxSize()         { return m_uiQuadtreeTULog2MaxSize; }
  Void setQuadtreeTULog2MinSize( UInt u ) { m_uiQuadtreeTULog2MinSize = u;    }
  UInt getQuadtreeTULog2MinSize()         { return m_uiQuadtreeTULog2MinSize; }
  Void setQuadtreeTUMaxDepthInter( UInt u ) { m_uiQuadtreeTUMaxDepthInter = u;    }
  Void setQuadtreeTUMaxDepthIntra( UInt u ) { m_uiQuadtreeTUMaxDepthIntra = u;    }
  UInt getQuadtreeTUMaxDepthInter()         { return m_uiQuadtreeTUMaxDepthInter; }
  UInt getQuadtreeTUMaxDepthIntra()         { return m_uiQuadtreeTUMaxDepthIntra; }
  Void setPad         (Int iPad[2]) { m_aiPad[0] = iPad[0]; m_aiPad[1] = iPad[1]; }
#if G1002_RPS
  Void setMaxNumberOfReferencePictures( UInt u ) { m_uiMaxNumberOfReferencePictures = u;    }
  UInt getMaxNumberOfReferencePictures()         { return m_uiMaxNumberOfReferencePictures; }
  Void setMaxNumberOfReorderPictures( UInt u )   { m_uiMaxNumberOfReorderPictures = u;    }
  UInt getMaxNumberOfReorderPictures()           { return m_uiMaxNumberOfReorderPictures; }
#endif
  Void setPadX        ( Int  u ) { m_aiPad[0] = u; }
  Void setPadY        ( Int  u ) { m_aiPad[1] = u; }
  Int  getPad         ( Int  u ) { assert(u < 2); return m_aiPad[u];}
  Int* getPad         ( )        { return m_aiPad; }
  
  // physical transform
  Void setMaxTrSize   ( UInt u ) { m_uiMaxTrSize = u;       }
  UInt getMaxTrSize   ()         { return  m_uiMaxTrSize;   }
  
  // Tool list
  Bool getUsePAD      ()         { return m_bUsePAD;        }
  Bool getUseMRG      ()         { return m_bUseMRG;        } // SOPH:
  
  Void setUsePAD      ( Bool b ) { m_bUsePAD   = b;         }
  Void setUseMRG      ( Bool b ) { m_bUseMRG  = b;          } // SOPH:
  
  Bool getUseLMChroma ()         { return m_bUseLMChroma;        }
  Void setUseLMChroma ( Bool b ) { m_bUseLMChroma  = b;          }

#if NSQT
  Bool getUseNSQT() { return m_useNSQT; }
  Void setUseNSQT( Bool b ) { m_useNSQT = b; }
#endif
  
#if AMP
  // AMP accuracy
  Int       getAMPAcc   ( UInt uiDepth ) { return m_iAMPAcc[uiDepth]; }
  Void      setAMPAcc   ( UInt uiDepth, Int iAccu ) { assert( uiDepth < g_uiMaxCUDepth);  m_iAMPAcc[uiDepth] = iAccu; }
#endif  

  UInt      getMaxTLayers()                           { return m_uiMaxTLayers; }
  Void      setMaxTLayers( UInt uiMaxTLayers )        { assert( uiMaxTLayers <= MAX_TLAYER ); m_uiMaxTLayers = uiMaxTLayers; }

#if !G1002_RPS
#if REF_SETTING_FOR_LD
  Void      setUseNewRefSetting    ( Bool b ) { m_bUseNewRefSetting = b;    }
  Bool      getUseNewRefSetting    ()         { return m_bUseNewRefSetting; }
  Void      setMaxNumRefFrames     ( UInt u ) { m_uiMaxNumRefFrames = u;    }
  UInt      getMaxNumRefFrames     ()         { return m_uiMaxNumRefFrames; }
#endif
#endif
};
#if G1002_RPS
/// Reference Picture Set class
class TComReferencePictureSet
{
private:
  UInt      m_uiNumberOfPictures;
  UInt      m_uiNumberOfNegativePictures;
  UInt      m_uiNumberOfPositivePictures;
  UInt      m_uiNumberOfLongtermPictures;
  Int*      m_piDeltaPOC;
  Int*      m_piPOC;
  Bool*     m_pbUsed;
#if INTER_RPS_PREDICTION
  Bool m_bInterRPSPrediction;
  Int m_iDeltaRIdxMinus1;   
  Int m_iDeltaRPS; 
  Int m_iNumRefIdc; 
  Int* m_piRefIdc;
#endif  

public:
  TComReferencePictureSet();
  virtual ~TComReferencePictureSet();
  
#if INTER_RPS_PREDICTION
  Void  create                    (UInt uiNumberOfPictures, UInt uiNumberOfRefIdc);
#else
  Void  create                    (UInt uiNumberOfPictures);
#endif  
  Void  destroy                   ();

  Void setUsed(UInt uiBufferNum, Bool bUsed);
  Void setDeltaPOC(UInt uiBufferNum, Int iDeltaPOC);
  Void setPOC(UInt uiBufferNum, Int iDeltaPOC);
  Void setNumberOfPictures(UInt NumberOfPictures);

  UInt getUsed(UInt uiBufferNum);
  Int  getDeltaPOC(UInt uiBufferNum);
  Int  getPOC(UInt uiBufferNum);
  UInt getNumberOfPictures();

  Void setNumberOfNegativePictures(UInt Number) { m_uiNumberOfNegativePictures = Number; }
  UInt getNumberOfNegativePictures() { return m_uiNumberOfNegativePictures; }
  Void setNumberOfPositivePictures(UInt Number) { m_uiNumberOfPositivePictures = Number; }
  UInt getNumberOfPositivePictures() { return m_uiNumberOfPositivePictures; }
  Void setNumberOfLongtermPictures(UInt Number) { m_uiNumberOfLongtermPictures = Number; }
  UInt getNumberOfLongtermPictures() { return m_uiNumberOfLongtermPictures; }

#if INTER_RPS_PREDICTION
  Void setInterRPSPrediction(Bool flag) { m_bInterRPSPrediction = flag; }
  Bool getInterRPSPrediction() { return m_bInterRPSPrediction; }
  Void setDeltaRIdxMinus1(Int x) { m_iDeltaRIdxMinus1 = x; }
  Int getDeltaRIdxMinus1() { return m_iDeltaRIdxMinus1; }
  Void setDeltaRPS(Int x) { m_iDeltaRPS = x; }
  Int getDeltaRPS() { return m_iDeltaRPS; }
  Void setNumRefIdc(Int x) { m_iNumRefIdc = x; }
  Int getNumRefIdc() { return m_iNumRefIdc; }

  Void setRefIdc(UInt uiBufferNum, Int iRefIdc);
  Int  getRefIdc(UInt uiBufferNum);

  Void sortDeltaPOC();
#endif
  Void printDeltaPOC();
};

/// Reference Picture Set set class
class TComRPS
{
private:
  UInt      m_uiNumberOfReferencePictureSets;
  TComReferencePictureSet*      m_pReferencePictureSet;
  
public:
  TComRPS();
  virtual ~TComRPS();
  
  Void  create                    (UInt uiNumberOfEntries);
  Void  destroy                   ();


  TComReferencePictureSet* getReferencePictureSet(UInt uiReferencePictureSetNum);
  UInt getNumberOfReferencePictureSets();
  Void setNumberOfReferencePictureSets(UInt uiNumberOfReferencePictureSets);
};

/// Reference Picture Lists class
class TComRefPicListModification
{
private:
  UInt      m_bRefPicListModificationFlagL0;  
  UInt      m_bRefPicListModificationFlagL1;  
  UInt      m_uiNumberOfRefPicListModificationsL0;
  UInt      m_uiNumberOfRefPicListModificationsL1;
  UInt      m_ListIdcL0[32];
  UInt      m_RefPicSetIdxL0[32];
  UInt      m_ListIdcL1[32];
  UInt      m_RefPicSetIdxL1[32];
    
public:
  TComRefPicListModification();
  virtual ~TComRefPicListModification();
  
  Void  create                    ();
  Void  destroy                   ();

  Bool       getRefPicListModificationFlagL0() { return m_bRefPicListModificationFlagL0; }
  Void       setRefPicListModificationFlagL0(Bool flag) { m_bRefPicListModificationFlagL0 = flag; }
  Bool       getRefPicListModificationFlagL1() { return m_bRefPicListModificationFlagL1; }
  Void       setRefPicListModificationFlagL1(Bool flag) { m_bRefPicListModificationFlagL1 = flag; }
  UInt       getNumberOfRefPicListModificationsL0() { return m_uiNumberOfRefPicListModificationsL0; }
  Void       setNumberOfRefPicListModificationsL0(UInt nr) { m_uiNumberOfRefPicListModificationsL0 = nr; }
  UInt       getNumberOfRefPicListModificationsL1() { return m_uiNumberOfRefPicListModificationsL1; }
  Void       setNumberOfRefPicListModificationsL1(UInt nr) { m_uiNumberOfRefPicListModificationsL1 = nr; }
  Void       setListIdcL0(UInt idx, UInt idc) { m_ListIdcL0[idx] = idc; }
  UInt       getListIdcL0(UInt idx) { return m_ListIdcL0[idx]; }
  Void       setRefPicSetIdxL0(UInt idx, UInt refPicSetIdx) { m_RefPicSetIdxL0[idx] = refPicSetIdx; }
  UInt       getRefPicSetIdxL0(UInt idx) { return m_RefPicSetIdxL0[idx]; }
  Void       setListIdcL1(UInt idx, UInt idc) { m_ListIdcL1[idx] = idc; }
  UInt       getListIdcL1(UInt idx) { return m_ListIdcL1[idx]; }
  Void       setRefPicSetIdxL1(UInt idx, UInt refPicSetIdx) { m_RefPicSetIdxL1[idx] = refPicSetIdx; }
  UInt       getRefPicSetIdxL1(UInt idx) { return m_RefPicSetIdxL1[idx]; }
};

#endif
/// PPS class
class TComPPS
{
private:
  Int         m_PPSId;                    // pic_parameter_set_id
  Int         m_SPSId;                    // seq_parameter_set_id
  Bool        m_bConstrainedIntraPred;    // constrained_intra_pred_flag
 
  // access channel
  TComSPS*    m_pcSPS;
#if G1002_RPS
  TComRPS*    m_pcRPSList;
#endif
#if G1002_RPS
  Bool        m_bLongTermRefsPresent;
  UInt        m_uiBitsForLongTermRefs;

#endif

#if !F747_APS
  Bool        m_bSharedPPSInfoEnabled;  //!< Shared info. in PPS is enabled/disabled
#endif

public:
  TComPPS();
  virtual ~TComPPS();
  
  Int       getPPSId ()      { return m_PPSId; }
  Void      setPPSId (Int i) { m_PPSId = i; }
  Int       getSPSId ()      { return m_SPSId; }
  Void      setSPSId (Int i) { m_SPSId = i; }
  
  Bool      getConstrainedIntraPred ()         { return  m_bConstrainedIntraPred; }
  Void      setConstrainedIntraPred ( Bool b ) { m_bConstrainedIntraPred = b;     }

#if G1002_RPS
  Bool      getLongTermRefsPresent()         { return m_bLongTermRefsPresent; }
  Void      setLongTermRefsPresent(Bool b)   { m_bLongTermRefsPresent=b;      }
  UInt      getBitsForLongTermRefs()         { return m_uiBitsForLongTermRefs;}
  Void      setBitsForLongTermRefs(UInt ui)  { m_uiBitsForLongTermRefs=ui;    }
#endif
  Void      setSPS              ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS              ()         { return m_pcSPS;          }
#if G1002_RPS
  Void      setRPSList              ( TComRPS* pcRPSList ) { m_pcRPSList = pcRPSList; }
  TComRPS*  getRPSList              ()         { return m_pcRPSList;          }
#endif

#if !F747_APS
  ///  set shared PPS info enabled/disabled
  Void      setSharedPPSInfoEnabled(Bool b) {m_bSharedPPSInfoEnabled = b;   }
  /// get shared PPS info enabled/disabled flag
  Bool      getSharedPPSInfoEnabled()       {return m_bSharedPPSInfoEnabled;}
#endif
};

#if F747_APS
/// APS class
class TComAPS
{
public:
  TComAPS();
  virtual ~TComAPS();

  Void      setAPSID      (Int iID)   {m_apsID = iID;            }  //!< set APS ID 
  Int       getAPSID      ()          {return m_apsID;           }  //!< get APS ID

  Void      setCABACForAPS(Bool bVal) {m_bCABACForAPS = bVal;    }  //!< set CABAC enabled/disabled in APS
  Bool      getCABACForAPS()          {return m_bCABACForAPS;    }  //!< get CABAC enabled/disabled in APS
  Void      setCABACinitIDC(Int iVal) {m_CABACinitIDC = iVal;    }  //!< set CABAC initial IDC number for APS coding
  Int       getCABACinitIDC()         {return m_CABACinitIDC;    }  //!< get CABAC initial IDC number for APS coding
  Void      setCABACinitQP(Int iVal)  {m_CABACinitQP = iVal;     }  //!< set CABAC initial QP value for APS coding
  Int       getCABACinitQP()          {return m_CABACinitQP;     }  //!< get CABAC initial QP value for APS coding

private:
  Int         m_apsID;        //!< APS ID
  Bool        m_bCABACForAPS; //!< CABAC coding enabled/disabled for APS (true for enabling CABAC)
  Int         m_CABACinitIDC; //!< CABAC initial IDC number for APS coding
  Int         m_CABACinitQP;  //!< CABAC initial QP value for APS coding

public:
  TComAPS& operator= (const TComAPS& src);  //!< "=" operator for APS object
};
#endif


/// slice header class
class TComSlice
{
  
private:
  //  Bitstream writing
#if F747_APS
  Int         m_iAPSId; //!< APS ID in slice header
#endif
  Int         m_iPPSId;               ///< picture parameter set ID
  Int         m_iPOC;
#if G1002_RPS
  static Int  m_iPrevPOC;
  TComReferencePictureSet *m_pcRPS;
  TComReferencePictureSet m_LocalRPS;
  Int         m_iBDidx; 
  Int         m_iCombinationBDidx;
  Bool        m_bCombineWithReferenceFlag;
  TComRefPicListModification m_RefPicListModification;
#endif
  NalUnitType m_eNalUnitType;         ///< Nal unit type for the slice
  SliceType   m_eSliceType;
  Int         m_iSliceQp;
  Bool        m_bLoopFilterDisable;
  
  Bool        m_bDRBFlag;             //  flag for future usage as reference buffer
  ERBIndex    m_eERBIndex;            //  flag for future usage as reference buffer
  Int         m_aiNumRefIdx   [3];    //  for multiple reference of current slice

  Int         m_iRefIdxOfLC[2][MAX_NUM_REF_LC];
  Int         m_eListIdFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL1FromRefIdxOfL0[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL0FromRefIdxOfL1[MAX_NUM_REF_LC];

  Bool        m_bCheckLDC;

  //  Data
  Int         m_iSliceQpDelta;
#if G1002_RPS
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF+1];
  Int         m_aiRefPOCList  [2][MAX_NUM_REF+1];
#else
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF];
  Int         m_aiRefPOCList  [2][MAX_NUM_REF];
#endif
  Int         m_iDepth;
  
  // referenced slice?
  Bool        m_bRefenced;
  
  // access channel
  TComSPS*    m_pcSPS;
  TComPPS*    m_pcPPS;
  TComPic*    m_pcPic;
#if F747_APS
  TComAPS*    m_pcAPS;  //!< pointer to APS parameter object
#endif

  UInt        m_uiColDir;  // direction to get colocated CUs
  
#if SAO_CHROMA_LAMBDA
  Double      m_dLambdaLuma;
  Double      m_dLambdaChroma;
#else
  Double      m_dLambda;
#endif

  Bool        m_abEqualRef  [2][MAX_NUM_REF][MAX_NUM_REF];
  
  Bool        m_bNoBackPredFlag;
  Bool        m_bRefIdxCombineCoding;

  UInt        m_uiSliceCurEndCUAddr;
  UInt        m_uiSliceIdx;
  UInt        m_uiSliceBits;

public:
  TComSlice();
  
  Void      initSlice       ();
  
  Void      setSPS          ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS          () { return m_pcSPS; }
  
  Void      setPPS          ( TComPPS* pcPPS )         { assert(pcPPS!=NULL); m_pcPPS = pcPPS; m_iPPSId = pcPPS->getPPSId(); }
  TComPPS*  getPPS          () { return m_pcPPS; }

  Void      setPPSId        ( Int PPSId )         { m_iPPSId = PPSId; }
  Int       getPPSId        () { return m_iPPSId; }
#if F747_APS
  Void      setAPS          ( TComAPS* pcAPS ) { m_pcAPS = pcAPS; } //!< set APS pointer
  TComAPS*  getAPS          ()                 { return m_pcAPS;  } //!< get APS pointer
  Void      setAPSId        ( Int Id)          { m_iAPSId =Id;    } //!< set APS ID
  Int       getAPSId        ()                 { return m_iAPSId; } //!< get APS ID
#endif
  
#if G1002_RPS
  Void      setRPS          ( TComReferencePictureSet *pcRPS ) { m_pcRPS = pcRPS; }
  TComReferencePictureSet*  getRPS          () { return m_pcRPS; }
  TComReferencePictureSet*  getLocalRPS     () { return &m_LocalRPS; }

  Void      setRPSidx          ( Int iBDidx ) { m_iBDidx = iBDidx; }
  Int       getRPSidx          () { return m_iBDidx; }
  Void      setCombinationBDidx          ( Int iCombinationBDidx ) { m_iCombinationBDidx = iCombinationBDidx; }
  Int       getCombinationBDidx          () { return m_iCombinationBDidx; }
  Void      setCombineWithReferenceFlag          ( Bool bCombineWithReferenceFlag ) { m_bCombineWithReferenceFlag = bCombineWithReferenceFlag; }
  Bool      getCombineWithReferenceFlag          () { return m_bCombineWithReferenceFlag; }
  Int       getPrevPOC      ()                          { return  m_iPrevPOC;       }

  TComRefPicListModification* getRefPicListModification() { return &m_RefPicListModification; }
#endif
  SliceType getSliceType    ()                          { return  m_eSliceType;         }
  Int       getPOC          ()                          { return  m_iPOC;           }
  Int       getSliceQp      ()                          { return  m_iSliceQp;           }
  Int       getSliceQpDelta ()                          { return  m_iSliceQpDelta;      }
  Bool      getDRBFlag      ()                          { return  m_bDRBFlag;           }
  ERBIndex  getERBIndex     ()                          { return  m_eERBIndex;          }
  Bool      getLoopFilterDisable()                      { return  m_bLoopFilterDisable; }
  Int       getNumRefIdx        ( RefPicList e )                { return  m_aiNumRefIdx[e];             }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
  TComPic*  getRefPic           ( RefPicList e, Int iRefIdx)    { return  m_apcRefPicList[e][iRefIdx];  }
  Int       getRefPOC           ( RefPicList e, Int iRefIdx)    { return  m_aiRefPOCList[e][iRefIdx];   }
  Int       getDepth            ()                              { return  m_iDepth;                     }
  UInt      getColDir           ()                              { return  m_uiColDir;                   }
  Bool      getCheckLDC     ()                                  { return m_bCheckLDC; }

  Int       getRefIdxOfLC       (RefPicList e, Int iRefIdx)     { return m_iRefIdxOfLC[e][iRefIdx];           }
  Int       getListIdFromIdxOfLC(Int iRefIdx)                   { return m_eListIdFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxFromIdxOfLC(Int iRefIdx)                   { return m_iRefIdxFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxOfL0FromRefIdxOfL1(Int iRefIdx)            { return m_iRefIdxOfL0FromRefIdxOfL1[iRefIdx];}
  Int       getRefIdxOfL1FromRefIdxOfL0(Int iRefIdx)            { return m_iRefIdxOfL1FromRefIdxOfL0[iRefIdx];}
  Void      setListIdFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_eListIdFromIdxOfLC[iRefIdx]=uiVal; }
  Void      setRefIdxFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_iRefIdxFromIdxOfLC[iRefIdx]=uiVal; }
  Void      setRefIdxOfLC       (RefPicList e, Int iRefIdx, Int RefIdxLC)     { m_iRefIdxOfLC[e][iRefIdx]=RefIdxLC;}

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }
  
#if G1002_RPS
  Void      setPOC              ( Int i )                       { m_iPOC              = i; m_iPrevPOC=i; }
#else
  Void      setPOC              ( Int i )                       { m_iPOC              = i;      }
#endif
  Void      setNalUnitType      ( NalUnitType e )               { m_eNalUnitType      = e;      }
  NalUnitType getNalUnitType    ()                              { return m_eNalUnitType;        }
  Void      decodingRefreshMarking(UInt& uiPOCCDR, Bool& bRefreshPending, TComList<TComPic*>& rcListPic);
  Void      setSliceType        ( SliceType e )                 { m_eSliceType        = e;      }
  Void      setSliceQp          ( Int i )                       { m_iSliceQp          = i;      }
  Void      setSliceQpDelta     ( Int i )                       { m_iSliceQpDelta     = i;      }
  Void      setDRBFlag          ( Bool b )                      { m_bDRBFlag = b;               }
  Void      setERBIndex         ( ERBIndex e )                  { m_eERBIndex = e;              }
  Void      setLoopFilterDisable( Bool b )                      { m_bLoopFilterDisable= b;      }
  
  Void      setRefPic           ( TComPic* p, RefPicList e, Int iRefIdx ) { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC           ( Int i, RefPicList e, Int iRefIdx ) { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setNumRefIdx        ( RefPicList e, Int i )         { m_aiNumRefIdx[e]    = i;      }
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }
  
  Void      setRefPicList       ( TComList<TComPic*>& rcListPic );
  Void      setRefPOCList       ();
  Void      setColDir           ( UInt uiDir ) { m_uiColDir = uiDir; }
  Void      setCheckLDC         ( Bool b )                      { m_bCheckLDC = b; }
  
  Bool      isIntra         ()                          { return  m_eSliceType == I_SLICE;  }
  Bool      isInterB        ()                          { return  m_eSliceType == B_SLICE;  }
  Bool      isInterP        ()                          { return  m_eSliceType == P_SLICE;  }
  
#if SAO_CHROMA_LAMBDA  
  Void      setLambda( Double d, Double e ) { m_dLambdaLuma = d; m_dLambdaChroma = e;}
  Double    getLambdaLuma() { return m_dLambdaLuma;        }
  Double    getLambdaChroma() { return m_dLambdaChroma;        }
#else
  Void      setLambda( Double d ) { m_dLambda = d; }
  Double    getLambda() { return m_dLambda;        }
#endif
  
  Void      initEqualRef();
  Bool      isEqualRef  ( RefPicList e, Int iRefIdx1, Int iRefIdx2 )
  {
    if (iRefIdx1 < 0 || iRefIdx2 < 0) return false;
    return m_abEqualRef[e][iRefIdx1][iRefIdx2];
  }
  
  Void setEqualRef( RefPicList e, Int iRefIdx1, Int iRefIdx2, Bool b)
  {
    m_abEqualRef[e][iRefIdx1][iRefIdx2] = m_abEqualRef[e][iRefIdx2][iRefIdx1] = b;
  }
  
  static Void      sortPicList         ( TComList<TComPic*>& rcListPic );
  
  Bool getNoBackPredFlag() { return m_bNoBackPredFlag; }
  Void setNoBackPredFlag( Bool b ) { m_bNoBackPredFlag = b; }
  Bool getRefIdxCombineCoding() { return m_bRefIdxCombineCoding; }
  Void setRefIdxCombineCoding( Bool b ) { m_bRefIdxCombineCoding = b; }
  Void generateCombinedList       ();

  Void decodingMarking( TComList<TComPic*>& rcListPic, Int& iMaxRefPicNum ); 
#if G1002_RPS
  Void      applyReferencePictureSet( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pcRPSList);
  Int       checkThatAllRefPicsAreAvailable( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet, Bool outputFlag);
  Void      createExplicitReferencePictureSetFromReference( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet);
#else

#if REF_SETTING_FOR_LD
  Int getActualRefNumber( TComList<TComPic*>& rcListPic );
  Void decodingRefMarkingForLD( TComList<TComPic*>& rcListPic, Int iMaxNumRefFrames, Int iCurrentPOC );
#endif
#endif
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
  UInt m_uiMaxNumMergeCand;
  Void setMaxNumMergeCand               (UInt maxNumMergeCand ) { m_uiMaxNumMergeCand = maxNumMergeCand;  }
  UInt getMaxNumMergeCand               ()                  {return m_uiMaxNumMergeCand;                  }
#endif

  Void setSliceCurEndCUAddr             ( UInt uiAddr )     { m_uiSliceCurEndCUAddr = uiAddr;             }
  UInt getSliceCurEndCUAddr             ()                  { return m_uiSliceCurEndCUAddr;               }
  Void setSliceIdx                      ( UInt i)           { m_uiSliceIdx = i;                           }
  UInt getSliceIdx                      ()                  { return  m_uiSliceIdx;                       }
  Void copySliceInfo                    (TComSlice *pcSliceSrc);
  Void setSliceBits                     ( UInt uiVal )      { m_uiSliceBits = uiVal;                      }
  UInt getSliceBits                     ()                  { return m_uiSliceBits;                       }  
protected:
#if G1002_RPS
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         UInt                uiPOC);
  TComPic*  xGetLongTermRefPic  (TComList<TComPic*>& rcListPic,
                         UInt                uiPOC);
#else
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         Bool                bDRBFlag,
                         ERBIndex            eERBIndex,
                         UInt                uiPOCCurr,
                         RefPicList          eRefPicList,
                         UInt                uiNthRefPic );
#endif
};// END CLASS DEFINITION TComSlice

//! \}

#endif // __TCOMSLICE__
