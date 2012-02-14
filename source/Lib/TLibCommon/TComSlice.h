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

/** \file     TComSlice.h
    \brief    slice header and SPS class (header)
*/

#ifndef __TCOMSLICE__
#define __TCOMSLICE__

#include <cstring>
#include <map>
#include "CommonDef.h"
#include "TComRom.h"
#include "TComList.h"

//! \ingroup TLibCommon
//! \{

class TComPic;
class TComTrQuant;

// ====================================================================================================================
// Constants
// ====================================================================================================================

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
  Int         m_chromaFormatIdc;

  // Structure
  UInt        m_uiWidth;
  UInt        m_uiHeight;
  UInt        m_uiMaxCUWidth;
  UInt        m_uiMaxCUHeight;
  UInt        m_uiMaxCUDepth;
  UInt        m_uiMinTrDepth;
  UInt        m_uiMaxTrDepth;
  
  // Tool list
  UInt        m_uiQuadtreeTULog2MaxSize;
  UInt        m_uiQuadtreeTULog2MinSize;
  UInt        m_uiQuadtreeTUMaxDepthInter;
  UInt        m_uiQuadtreeTUMaxDepthIntra;
  Bool        m_bDisInter4x4;
  Bool        m_useAMP;
  Bool        m_bUseMRG; // SOPH:

  Bool        m_bUseLComb;
  Bool        m_bLCMod;
  Bool        m_useNSQT;
  
  // Parameter
  AMVP_MODE   m_aeAMVPMode[MAX_CU_DEPTH];

  UInt        m_uiBitsForPOC;
  // Max physical transform size
  UInt        m_uiMaxTrSize;
  
  Int m_iAMPAcc[MAX_CU_DEPTH];

  UInt        m_uiMaxDecFrameBuffering; 
  UInt        m_uiMaxLatencyIncrease;

public:
  TComSPS();
  virtual ~TComSPS();

  Int  getSPSId       ()         { return m_SPSId;          }
  Void setSPSId       (Int i)    { m_SPSId = i;             }
  Int  getProfileIdc  ()         { return m_ProfileIdc;     }
  Void setProfileIdc  (Int i)    { m_ProfileIdc = i;        }
  Int  getLevelIdc    ()         { return m_LevelIdc;       }
  Void setLevelIdc    (Int i)    { m_LevelIdc = i;          }

  Int  getChromaFormatIdc ()         { return m_chromaFormatIdc;       }
  Void setChromaFormatIdc (Int i)    { m_chromaFormatIdc = i;          }
  
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
  Void setBitsForPOC  ( UInt u ) { m_uiBitsForPOC = u;      }
  UInt getBitsForPOC  ()         { return m_uiBitsForPOC;   }
  Bool getDisInter4x4()         { return m_bDisInter4x4;        }
  Void setDisInter4x4      ( Bool b ) { m_bDisInter4x4  = b;          }
  Bool getUseAMP() { return m_useAMP; }
  Void setUseAMP( Bool b ) { m_useAMP = b; }
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
  
  // physical transform
  Void setMaxTrSize   ( UInt u ) { m_uiMaxTrSize = u;       }
  UInt getMaxTrSize   ()         { return  m_uiMaxTrSize;   }
  
  // Tool list
  Bool getUseMRG      ()         { return m_bUseMRG;        } // SOPH:
  
  Void setUseMRG      ( Bool b ) { m_bUseMRG  = b;          } // SOPH:
  
  Void setUseLComb    (Bool b)   { m_bUseLComb = b;         }
  Bool getUseLComb    ()         { return m_bUseLComb;      }
  Void setLCMod       (Bool b)   { m_bLCMod = b;     }
  Bool getLCMod       ()         { return m_bLCMod;  }

  Bool getUseNSQT() { return m_useNSQT; }
  Void setUseNSQT( Bool b ) { m_useNSQT = b; }
  
  // AMVP mode (for each depth)
  AMVP_MODE getAMVPMode ( UInt uiDepth ) { assert(uiDepth < g_uiMaxCUDepth);  return m_aeAMVPMode[uiDepth]; }
  Void      setAMVPMode ( UInt uiDepth, AMVP_MODE eMode) { assert(uiDepth < g_uiMaxCUDepth);  m_aeAMVPMode[uiDepth] = eMode; }
  
  // AMP accuracy
  Int       getAMPAcc   ( UInt uiDepth ) { return m_iAMPAcc[uiDepth]; }
  Void      setAMPAcc   ( UInt uiDepth, Int iAccu ) { assert( uiDepth < g_uiMaxCUDepth);  m_iAMPAcc[uiDepth] = iAccu; }

  UInt getMaxDecFrameBuffering  ()            { return m_uiMaxDecFrameBuffering; }
  Void setMaxDecFrameBuffering  ( UInt ui )   { m_uiMaxDecFrameBuffering = ui;   }
  UInt getMaxLatencyIncrease    ()            { return m_uiMaxLatencyIncrease;   }
  Void setMaxLatencyIncrease    ( UInt ui )   { m_uiMaxLatencyIncrease= ui;      }
};

/// Reference Picture Set class
class TComReferencePictureSet
{
private:
  UInt      m_uiNumberOfPictures;
  UInt      m_uiNumberOfNegativePictures;
  UInt      m_uiNumberOfPositivePictures;
  UInt      m_uiNumberOfLongtermPictures;
  Int       m_piDeltaPOC[MAX_NUM_REF_PICS];
  Int       m_piPOC[MAX_NUM_REF_PICS];
  Bool      m_pbUsed[MAX_NUM_REF_PICS];

public:
  TComReferencePictureSet();
  virtual ~TComReferencePictureSet();

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

/// PPS class
class TComPPS
{
private:
  Int         m_PPSId;                    // pic_parameter_set_id
  Int         m_SPSId;                    // seq_parameter_set_id
  Int         m_picInitQPMinus26;
 
  // access channel
  TComSPS*    m_pcSPS;
  TComRPS*    m_pcRPSList;

  Bool        m_bLongTermRefsPresent;
  UInt        m_uiBitsForLongTermRefs;

  Bool     m_enableTMVPFlag;

public:
  TComPPS();
  virtual ~TComPPS();
  
  Int       getPPSId ()      { return m_PPSId; }
  Void      setPPSId (Int i) { m_PPSId = i; }
  Int       getSPSId ()      { return m_SPSId; }
  Void      setSPSId (Int i) { m_SPSId = i; }
  
  Int       getPicInitQPMinus26 ()         { return  m_picInitQPMinus26; }
  Void      setPicInitQPMinus26 ( Int i )  { m_picInitQPMinus26 = i;     }

  Bool      getLongTermRefsPresent()         { return m_bLongTermRefsPresent; }
  Void      setLongTermRefsPresent(Bool b)   { m_bLongTermRefsPresent=b;      }
  UInt      getBitsForLongTermRefs()         { return m_uiBitsForLongTermRefs;}
  Void      setBitsForLongTermRefs(UInt ui)  { m_uiBitsForLongTermRefs=ui;    }
  Void      setSPS              ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS              ()         { return m_pcSPS;          }
  Void      setRPSList              ( TComRPS* pcRPSList ) { m_pcRPSList = pcRPSList; }
  TComRPS*  getRPSList              ()         { return m_pcRPSList;          }

  Void     setEnableTMVPFlag( Bool b )  { m_enableTMVPFlag = b;    }
  Bool     getEnableTMVPFlag()          { return m_enableTMVPFlag; }
};

/// slice header class
class TComSlice
{
  
private:
  //  Bitstream writing
  Int         m_iAPSId; //!< APS ID in slice header
  Int         m_iPPSId;               ///< picture parameter set ID
  Int         m_iPOC;
  Int         m_iLastIDR;
  static Int  m_iPrevPOC;
  TComReferencePictureSet *m_pcRPS;
  TComReferencePictureSet m_LocalRPS;
  Int         m_iBDidx; 
  Int         m_iCombinationBDidx;
  Bool        m_bCombineWithReferenceFlag;
  TComRefPicListModification m_RefPicListModification;
  NalUnitType m_eNalUnitType;         ///< Nal unit type for the slice
  SliceType   m_eSliceType;
  Int         m_iSliceQp;
  
  Int         m_aiNumRefIdx   [3];    //  for multiple reference of current slice

  Int         m_iRefIdxOfLC[2][MAX_NUM_REF_LC];
  Int         m_eListIdFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL1FromRefIdxOfL0[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL0FromRefIdxOfL1[MAX_NUM_REF_LC];
  Bool        m_bRefPicListModificationFlagLC;
  Bool        m_bRefPicListCombinationFlag;

  //  Data
  Int         m_iSliceQpDelta;
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF+1];
  Int         m_aiRefPOCList  [2][MAX_NUM_REF+1];
  Int         m_iDepth;
  
  // referenced slice?
  Bool        m_bRefenced;
  
  // access channel
  TComSPS*    m_pcSPS;
  TComPPS*    m_pcPPS;
  TComPic*    m_pcPic;
  UInt        m_uiColDir;  // direction to get colocated CUs
  
  Double      m_dLambda;

  Bool        m_abEqualRef  [2][MAX_NUM_REF][MAX_NUM_REF];
  
  Bool        m_bNoBackPredFlag;
  Bool        m_bRefIdxCombineCoding;

  UInt        m_uiSliceCurEndCUAddr;
  Bool        m_bNextSlice;
  UInt        m_uiSliceBits;
  Bool        m_bFinalized;

  Int         m_cabacInitIdc; 

public:
  TComSlice();
  virtual ~TComSlice();
  
  Void      initSlice       ();
  
  Void      setSPS          ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS          () { return m_pcSPS; }
  
  Void      setPPS          ( TComPPS* pcPPS )         { assert(pcPPS!=NULL); m_pcPPS = pcPPS; m_iPPSId = pcPPS->getPPSId(); }
  TComPPS*  getPPS          () { return m_pcPPS; }

  Void      setPPSId        ( Int PPSId )         { m_iPPSId = PPSId; }
  Int       getPPSId        () { return m_iPPSId; }
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
  Void      setLastIDR(Int iIDRPOC)                       { m_iLastIDR = iIDRPOC; }
  Int       getLastIDR()                                  { return m_iLastIDR; }
  SliceType getSliceType    ()                          { return  m_eSliceType;         }
  Int       getPOC          ()                          { return  m_iPOC;           }
  Int       getSliceQp      ()                          { return  m_iSliceQp;           }
  Int       getSliceQpDelta ()                          { return  m_iSliceQpDelta;      }

  Int       getNumRefIdx        ( RefPicList e )                { return  m_aiNumRefIdx[e];             }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
  TComPic*  getRefPic           ( RefPicList e, Int iRefIdx)    { return  m_apcRefPicList[e][iRefIdx];  }
  Int       getRefPOC           ( RefPicList e, Int iRefIdx)    { return  m_aiRefPOCList[e][iRefIdx];   }
  Int       getDepth            ()                              { return  m_iDepth;                     }
  UInt      getColDir           ()                              { return  m_uiColDir;                   }

  Int       getRefIdxOfLC       (RefPicList e, Int iRefIdx)     { return m_iRefIdxOfLC[e][iRefIdx];           }
  Int       getListIdFromIdxOfLC(Int iRefIdx)                   { return m_eListIdFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxFromIdxOfLC(Int iRefIdx)                   { return m_iRefIdxFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxOfL0FromRefIdxOfL1(Int iRefIdx)            { return m_iRefIdxOfL0FromRefIdxOfL1[iRefIdx];}
  Int       getRefIdxOfL1FromRefIdxOfL0(Int iRefIdx)            { return m_iRefIdxOfL1FromRefIdxOfL0[iRefIdx];}
  Bool      getRefPicListModificationFlagLC()                   {return m_bRefPicListModificationFlagLC;}
  Void      setRefPicListModificationFlagLC(Bool bflag)         {m_bRefPicListModificationFlagLC=bflag;}     
  Bool      getRefPicListCombinationFlag()                      {return m_bRefPicListCombinationFlag;}
  Void      setRefPicListCombinationFlag(Bool bflag)            {m_bRefPicListCombinationFlag=bflag;}     
  Void      setListIdFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_eListIdFromIdxOfLC[iRefIdx]=uiVal; }
  Void      setRefIdxFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_iRefIdxFromIdxOfLC[iRefIdx]=uiVal; }
  Void      setRefIdxOfLC       (RefPicList e, Int iRefIdx, Int RefIdxLC)     { m_iRefIdxOfLC[e][iRefIdx]=RefIdxLC;}

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }
  
  Void      setPOC              ( Int i )                       { m_iPOC              = i; m_iPrevPOC=i; }
  Void      setNalUnitType      ( NalUnitType e )               { m_eNalUnitType      = e;      }
  NalUnitType getNalUnitType    ()                              { return m_eNalUnitType;        }
  Void      decodingRefreshMarking(UInt& uiPOCCDR, Bool& bRefreshPending, TComList<TComPic*>& rcListPic);
  Void      setSliceType        ( SliceType e )                 { m_eSliceType        = e;      }
  Void      setSliceQp          ( Int i )                       { m_iSliceQp          = i;      }
  Void      setSliceQpDelta     ( Int i )                       { m_iSliceQpDelta     = i;      }
  
  Void      setRefPic           ( TComPic* p, RefPicList e, Int iRefIdx ) { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC           ( Int i, RefPicList e, Int iRefIdx ) { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setNumRefIdx        ( RefPicList e, Int i )         { m_aiNumRefIdx[e]    = i;      }
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }
  
  Void      setRefPicList       ( TComList<TComPic*>& rcListPic );
  Void      setRefPOCList       ();
  Void      setColDir           ( UInt uiDir ) { m_uiColDir = uiDir; }
  
  Bool      isIntra         ()                          { return  m_eSliceType == I_SLICE;  }
  Bool      isInterP        ()                          { return  m_eSliceType == P_SLICE;  }
  
  Void      setLambda( Double d ) { m_dLambda = d; }
  Double    getLambda() { return m_dLambda;        }
  
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
  Void      applyReferencePictureSet( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pcRPSList);
  Int       checkThatAllRefPicsAreAvailable( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet, Bool outputFlag);
  Void      createExplicitReferencePictureSetFromReference( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet);

  Void decodingMarkingForNoTMVP( TComList<TComPic*>& rcListPic, Int currentPOC );

  UInt m_uiMaxNumMergeCand;
  Void setMaxNumMergeCand               (UInt maxNumMergeCand ) { m_uiMaxNumMergeCand = maxNumMergeCand;  }
  UInt getMaxNumMergeCand               ()                  {return m_uiMaxNumMergeCand;                  }

  Void setSliceCurEndCUAddr             ( UInt uiAddr )     { m_uiSliceCurEndCUAddr = uiAddr;             }
  UInt getSliceCurEndCUAddr             ()                  { return m_uiSliceCurEndCUAddr;               }
  Void setNextSlice                     ( Bool b )          { m_bNextSlice = b;                           }
  Bool isNextSlice                      ()                  { return m_bNextSlice;                        }
  Void setSliceBits                     ( UInt uiVal )      { m_uiSliceBits = uiVal;                      }
  UInt getSliceBits                     ()                  { return m_uiSliceBits;                       }  
  Void setFinalized                     ( Bool uiVal )      { m_bFinalized = uiVal;                       }
  Bool getFinalized                     ()                  { return m_bFinalized;                        }
  Void      setCABACinitIDC(Int iVal) {m_cabacInitIdc = iVal;    }  //!< set CABAC initial IDC number 
  Int       getCABACinitIDC()         {return m_cabacInitIdc;    }  //!< get CABAC initial IDC number 
protected:
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         UInt                uiPOC);
  TComPic*  xGetLongTermRefPic  (TComList<TComPic*>& rcListPic,
                         UInt                uiPOC);
};// END CLASS DEFINITION TComSlice


#if PARAMSET_VLC_CLEANUP

template <class T> class ParameterSetMap
{
public:
  ParameterSetMap(Int maxId)
  :m_maxId (maxId)
  {}

  ~ParameterSetMap()
  {
    for (typename std::map<Int,T *>::iterator i = m_paramsetMap.begin(); i!= m_paramsetMap.end(); i++)
    {
      delete (*i).second;
    }
  }

  Void storePS(Int psId, T *ps)
  {
    assert ( psId < m_maxId );
    if ( m_paramsetMap.find(psId) != m_paramsetMap.end() )
    {
      delete m_paramsetMap[psId];
    }
    m_paramsetMap[psId] = ps; 
  }

  Void mergePSList(ParameterSetMap<T> &rPsList)
  {
    for (typename std::map<Int,T *>::iterator i = rPsList.m_paramsetMap.begin(); i!= rPsList.m_paramsetMap.end(); i++)
    {
      storePS(i->first, i->second);
    }
    rPsList.m_paramsetMap.clear();
  }


  T* getPS(Int psId)
  {
    return ( m_paramsetMap.find(psId) == m_paramsetMap.end() ) ? NULL : m_paramsetMap[psId];
  }

  T* getFirstPS()
  {
    return (m_paramsetMap.begin() == m_paramsetMap.end() ) ? NULL : m_paramsetMap.begin()->second;
  }

private:
  std::map<Int,T *> m_paramsetMap;
  Int               m_maxId;
};

class ParameterSetManager
{
public:
  ParameterSetManager();
  virtual ~ParameterSetManager();

  //! store sequence parameter set and take ownership of it 
  Void storeSPS(TComSPS *sps) { m_spsMap.storePS( sps->getSPSId(), sps); };
  //! get pointer to existing sequence parameter set  
  TComSPS* getSPS(Int spsId)  { return m_spsMap.getPS(spsId); };
  TComSPS* getFirstSPS()      { return m_spsMap.getFirstPS(); };

  //! store picture parameter set and take ownership of it 
  Void storePPS(TComPPS *pps) { m_ppsMap.storePS( pps->getPPSId(), pps); };
  //! get pointer to existing picture parameter set  
  TComPPS* getPPS(Int ppsId)  { return m_ppsMap.getPS(ppsId); };
  TComPPS* getFirstPPS()      { return m_ppsMap.getFirstPS(); };

protected:
  ParameterSetMap<TComSPS> m_spsMap; 
  ParameterSetMap<TComPPS> m_ppsMap; 
};

#endif

//! \}

#endif // __TCOMSLICE__
