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

/** \file     TComSlice.h
    \brief    slice header and SPS class (header)
*/

#ifndef __TCOMSLICE__
#define __TCOMSLICE__


#include "CommonDef.h"
#include "TComList.h"

class TComPic;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// SPS class
class TComSPS
{
private:
  // Structure
  UInt        m_uiWidth;
  UInt        m_uiHeight;
  Int         m_aiPad[2];
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
  Bool        m_bUseALF;
  Bool        m_bUseDQP;
  Bool        m_bUseLDC;
  Bool        m_bUsePAD;
#if !DCTIF_8_6_LUMA
  Int         m_iDIFTap;
#endif
#if HHI_MRG
  Bool        m_bUseMRG; // SOPH:
#endif

#if DOCOMO_COMB_LIST
  Bool        m_bUseLComb;
  Bool        m_bLCMod;
#endif
  
#if HHI_RMP_SWITCH
  Bool        m_bUseRMP;
#endif
  // Parameter
  AMVP_MODE   m_aeAMVPMode[MAX_CU_DEPTH];
  UInt        m_uiBitDepth;
  UInt        m_uiBitIncrement;
  
  // Max physical transform size
  UInt        m_uiMaxTrSize;
  
  Int m_iAMPAcc[MAX_CU_DEPTH];
  
public:
  TComSPS();
  virtual ~TComSPS();
  
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
  Void setPadX        ( Int  u ) { m_aiPad[0] = u; }
  Void setPadY        ( Int  u ) { m_aiPad[1] = u; }
  Int  getPad         ( Int  u ) { assert(u < 2); return m_aiPad[u];}
  Int* getPad         ( )        { return m_aiPad; }
  
  // physical transform
  Void setMaxTrSize   ( UInt u ) { m_uiMaxTrSize = u;       }
  UInt getMaxTrSize   ()         { return  m_uiMaxTrSize;   }
  
  // Tool list
  Bool getUseALF      ()         { return m_bUseALF;        }
  Bool getUseDQP      ()         { return m_bUseDQP;        }
  
  Bool getUseLDC      ()         { return m_bUseLDC;        }
  Bool getUsePAD      ()         { return m_bUsePAD;        }
#if HHI_MRG
  Bool getUseMRG      ()         { return m_bUseMRG;        } // SOPH:
#endif
#if !DCTIF_8_6_LUMA
  Int  getDIFTap      ()         { return m_iDIFTap;        }
#endif
  
  Void setUseALF      ( Bool b ) { m_bUseALF  = b;          }
  Void setUseDQP      ( Bool b ) { m_bUseDQP   = b;         }
  
  Void setUseLDC      ( Bool b ) { m_bUseLDC   = b;         }
  Void setUsePAD      ( Bool b ) { m_bUsePAD   = b;         }
#if HHI_MRG
  Void setUseMRG      ( Bool b ) { m_bUseMRG  = b;          } // SOPH:
#endif
#if !DCTIF_8_6_LUMA
  Void setDIFTap      ( Int  i ) { m_iDIFTap   = i;         }
#endif
  
#if DOCOMO_COMB_LIST
  Void setUseLComb    (Bool b)   { m_bUseLComb = b;         }
  Bool getUseLComb    ()         { return m_bUseLComb;      }
  Void setLCMod       (Bool b)   { m_bLCMod = b;     }
  Bool getLCMod       ()         { return m_bLCMod;  }
#endif

#if HHI_RMP_SWITCH
  Bool getUseRMP     ()         { return m_bUseRMP; }
  Void setUseRMP     ( Bool b ) { m_bUseRMP = b;    }
#endif
  
  // AMVP mode (for each depth)
  AMVP_MODE getAMVPMode ( UInt uiDepth ) { assert(uiDepth < g_uiMaxCUDepth);  return m_aeAMVPMode[uiDepth]; }
  Void      setAMVPMode ( UInt uiDepth, AMVP_MODE eMode) { assert(uiDepth < g_uiMaxCUDepth);  m_aeAMVPMode[uiDepth] = eMode; }
  
  // Bit-depth
  UInt      getBitDepth     ()         { return m_uiBitDepth;     }
  Void      setBitDepth     ( UInt u ) { m_uiBitDepth = u;        }
  UInt      getBitIncrement ()         { return m_uiBitIncrement; }
  Void      setBitIncrement ( UInt u ) { m_uiBitIncrement = u;    }
};

/// PPS class
class TComPPS
{
private:
  
public:
  TComPPS();
  virtual ~TComPPS();
  
};

/// slice header class
class TComSlice
{
  
private:
  //  Bitstream writing
  Int         m_iPOC;
#if DCM_DECODING_REFRESH
  NalUnitType m_eNalUnitType;         ///< Nal unit type for the slice
#endif
  SliceType   m_eSliceType;
  Int         m_iSliceQp;
  Int         m_iSymbolMode;
  Bool        m_bLoopFilterDisable;
  
  Bool        m_bDRBFlag;             //  flag for future usage as reference buffer
  ERBIndex    m_eERBIndex;            //  flag for future usage as reference buffer
#if DOCOMO_COMB_LIST
  Int         m_aiNumRefIdx   [3];    //  for multiple reference of current slice

  Int         m_iRefIdxOfLC[2][MAX_NUM_REF_LC];
  Int         m_eListIdFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxFromIdxOfLC[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL1FromRefIdxOfL0[MAX_NUM_REF_LC];
  Int         m_iRefIdxOfL0FromRefIdxOfL1[MAX_NUM_REF_LC];
  Bool        m_bRefPicListCombinationFlag;
#else
  Int         m_aiNumRefIdx   [2];    //  for multiple reference of current slice
#endif  

  //  Data
  Int         m_iSliceQpDelta;
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF];
  Int         m_aiRefPOCList  [2][MAX_NUM_REF];
  Int         m_iDepth;
  
  // referenced slice?
  Bool        m_bRefenced;
#ifdef ROUNDING_CONTROL_BIPRED
  Bool        m_bRounding;
#endif
  
  // access channel
  TComSPS*    m_pcSPS;
  TComPPS*    m_pcPPS;
  TComPic*    m_pcPic;
  
  UInt        m_uiColDir;  // direction to get colocated CUs
  
  Double      m_dLambda;
  
  Bool        m_abEqualRef  [2][MAX_NUM_REF][MAX_NUM_REF];
  
  Int         m_iInterpFilterType;
#if MS_NO_BACK_PRED_IN_B0
  Bool        m_bNoBackPredFlag;
#endif
  
public:
  TComSlice();
  virtual ~TComSlice();
  
  Void      initSlice       ();
  
  Void      setSPS          ( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
  TComSPS*  getSPS          () { return m_pcSPS; }
  
  Void      setPPS          ( TComPPS* pcPPS ) { m_pcPPS = pcPPS; }
  TComPPS*  getPPS          () { return m_pcPPS; }
  
  SliceType getSliceType    ()                          { return  m_eSliceType;         }
  Int       getPOC          ()                          { return  m_iPOC;           }
  Int       getSliceQp      ()                          { return  m_iSliceQp;           }
  Int       getSliceQpDelta ()                          { return  m_iSliceQpDelta;      }
  Bool      getDRBFlag      ()                          { return  m_bDRBFlag;           }
  ERBIndex  getERBIndex     ()                          { return  m_eERBIndex;          }
  Int       getSymbolMode   ()                          { return  m_iSymbolMode;        }
  Bool      getLoopFilterDisable()                      { return  m_bLoopFilterDisable; }
  Int       getNumRefIdx        ( RefPicList e )                { return  m_aiNumRefIdx[e];             }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
  TComPic*  getRefPic           ( RefPicList e, Int iRefIdx)    { return  m_apcRefPicList[e][iRefIdx];  }
  Int       getRefPOC           ( RefPicList e, Int iRefIdx)    { return  m_aiRefPOCList[e][iRefIdx];   }
  Int       getDepth            ()                              { return  m_iDepth;                     }
  UInt      getColDir           ()                              { return  m_uiColDir;                   }
  
#if DOCOMO_COMB_LIST 
  Int       getRefIdxOfLC       (RefPicList e, Int iRefIdx)     { return m_iRefIdxOfLC[e][iRefIdx];           }
  Int       getListIdFromIdxOfLC(Int iRefIdx)                   { return m_eListIdFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxFromIdxOfLC(Int iRefIdx)                   { return m_iRefIdxFromIdxOfLC[iRefIdx];       }
  Int       getRefIdxOfL0FromRefIdxOfL1(Int iRefIdx)            { return m_iRefIdxOfL0FromRefIdxOfL1[iRefIdx];}
  Int       getRefIdxOfL1FromRefIdxOfL0(Int iRefIdx)            { return m_iRefIdxOfL1FromRefIdxOfL0[iRefIdx];}
  Bool      getRefPicListCombinationFlag()                      {return m_bRefPicListCombinationFlag;}
  Void      setRefPicListCombinationFlag(Bool bflag)            {m_bRefPicListCombinationFlag=bflag;}     
  Void      setListIdFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_eListIdFromIdxOfLC[iRefIdx]=uiVal; }
  Void      setRefIdxFromIdxOfLC(Int  iRefIdx, UInt uiVal)      { m_iRefIdxFromIdxOfLC[iRefIdx]=uiVal; }
#endif

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }
#ifdef ROUNDING_CONTROL_BIPRED
  Void      setRounding(Bool bRound)                            { m_bRounding = bRound; }
  Bool      isRounding()                                        { return m_bRounding; }
#endif
  
  Void      setPOC              ( Int i )                       { m_iPOC              = i;      }
#if DCM_DECODING_REFRESH
  Void      setNalUnitType      ( NalUnitType e )               { m_eNalUnitType      = e;      }
  NalUnitType getNalUnitType    ()                              { return m_eNalUnitType;        }
  Void      decodingRefreshMarking(UInt& uiPOCCDR, Bool& bRefreshPending, TComList<TComPic*>& rcListPic);
#endif
  Void      setSliceType        ( SliceType e )                 { m_eSliceType        = e;      }
  Void      setSliceQp          ( Int i )                       { m_iSliceQp          = i;      }
  Void      setSliceQpDelta     ( Int i )                       { m_iSliceQpDelta     = i;      }
  Void      setDRBFlag          ( Bool b )                      { m_bDRBFlag = b;               }
  Void      setERBIndex         ( ERBIndex e )                  { m_eERBIndex = e;              }
  Void      setSymbolMode       ( Int b  )                      { m_iSymbolMode       = b;      }
  Void      setLoopFilterDisable( Bool b )                      { m_bLoopFilterDisable= b;      }
  
  Void      setRefPic           ( TComPic* p, RefPicList e, Int iRefIdx ) { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC           ( Int i, RefPicList e, Int iRefIdx ) { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setNumRefIdx        ( RefPicList e, Int i )         { m_aiNumRefIdx[e]    = i;      }
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }
  
  Void      setRefPicList       ( TComList<TComPic*>& rcListPic );
  Void      setRefPOCList       ();
  Void      setColDir           ( UInt uiDir ) { m_uiColDir = uiDir; }
  
  Bool      isIntra         ()                          { return  m_eSliceType == I_SLICE;  }
  Bool      isInterB        ()                          { return  m_eSliceType == B_SLICE;  }
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
  
  Int  getInterpFilterType     ()         { return m_iInterpFilterType;       }
  Void setInterpFilterType     ( Int  i ) { m_iInterpFilterType  = i;         }
  
#if MS_NO_BACK_PRED_IN_B0
  Bool getNoBackPredFlag() { return m_bNoBackPredFlag; }
  Void setNoBackPredFlag( Bool b ) { m_bNoBackPredFlag = b; }
#endif
#if DOCOMO_COMB_LIST
  Void      generateCombinedList       ();
#endif
  
protected:
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         Bool                bDRBFlag,
                         ERBIndex            eERBIndex,
                         UInt                uiPOCCurr,
                         RefPicList          eRefPicList,
                         UInt                uiNthRefPic );
  
};// END CLASS DEFINITION TComSlice


#endif // __TCOMSLICE__

