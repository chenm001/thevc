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
#ifdef DCM_PBIC
#include "TComZeroTree.h"
#endif

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
#if HHI_RQT
  Bool        m_bQuadtreeTUFlag;
  UInt        m_uiQuadtreeTULog2MaxSize;
  UInt        m_uiQuadtreeTULog2MinSize;
#if HHI_RQT_DEPTH
  UInt        m_uiQuadtreeTUMaxDepth;
#endif
#endif
  Bool        m_bUseALF;
  Bool        m_bUseDQP;
  Bool        m_bUseWPG;
  Bool        m_bUseWPO;
  Bool        m_bUseLDC;
  Bool        m_bUsePAD;
  Bool        m_bUseQBO;
  Int         m_iDIFTap;
#if SAMSUNG_CHROMA_IF_EXT
  Int         m_iDIFTapC;
#endif

#if HHI_ALLOW_CIP_SWITCH
	Bool        m_bUseCIP; // BB:
#endif
  Bool				m_bUseROT; // BB:
#if HHI_AIS
  Bool        m_bUseAIS; // BB:
#endif
#if HHI_MRG
  Bool        m_bUseMRG; // SOPH:
#endif
#if HHI_IMVP
  Bool        m_bUseIMP; // SOPH:
#endif

#ifdef QC_AMVRES
	Bool					m_bUseAMVRes;
#endif
#ifdef QC_SIFO_PRED
  Bool          m_bUseSIFO_Pred;
#endif
#ifdef DCM_PBIC 
  Bool          m_bUseIC;
#endif

  Bool        m_bUseAMP;
#if HHI_RMP_SWITCH
  Bool        m_bUseRMP;
#endif
  // Parameter
  AMVP_MODE   m_aeAMVPMode[MAX_CU_DEPTH];
  UInt        m_uiBitDepth;
  UInt        m_uiBitIncrement;

#if HHI_ALF
  Bool        m_bALFSeparateQt;
  Bool        m_bALFSymmetry;
  Int         m_iALFMinLength;
  Int         m_iALFMaxLength;
#endif

  // Max physical transform size
  UInt        m_uiMaxTrSize;

  Int m_iAMPAcc[MAX_CU_DEPTH];

  UInt        m_uiBalancedCPUs;

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
#if HHI_RQT
  Void setQuadtreeTUFlag( Bool b )        { m_bQuadtreeTUFlag = b;            }
  Bool getQuadtreeTUFlag()                { return m_bQuadtreeTUFlag;         }
  Void setQuadtreeTULog2MaxSize( UInt u ) { m_uiQuadtreeTULog2MaxSize = u;    }
  UInt getQuadtreeTULog2MaxSize()         { return m_uiQuadtreeTULog2MaxSize; }
  Void setQuadtreeTULog2MinSize( UInt u ) { m_uiQuadtreeTULog2MinSize = u;    }
  UInt getQuadtreeTULog2MinSize()         { return m_uiQuadtreeTULog2MinSize; }
#if HHI_RQT_DEPTH
  Void setQuadtreeTUMaxDepth( UInt u ) { m_uiQuadtreeTUMaxDepth = u;    }
  UInt getQuadtreeTUMaxDepth()         { return m_uiQuadtreeTUMaxDepth; }
#endif
#endif
  Void setPad         (Int iPad[2]) { m_aiPad[0] = iPad[0]; m_aiPad[1] = iPad[1]; }
  Void setPadX        ( Int  u ) { m_aiPad[0] = u; }
  Void setPadY        ( Int  u ) { m_aiPad[1] = u; }
  Int  getPad         ( Int  u ) { assert(u < 2); return m_aiPad[u];}
  Int* getPad         ( )        { return m_aiPad; }
  Void setBalancedCPUs( UInt u ) { m_uiBalancedCPUs = u; }
  UInt getBalancedCPUs()         { return m_uiBalancedCPUs; }

  // physical transform
  Void setMaxTrSize   ( UInt u ) { m_uiMaxTrSize = u;       }
  UInt getMaxTrSize   ()         { return  m_uiMaxTrSize;   }

  // Tool list
  Bool getUseALF      ()         { return m_bUseALF;        }
  Bool getUseDQP      ()         { return m_bUseDQP;        }

  Bool getUseWPG      ()         { return m_bUseWPG;        }
  Bool getUseWPO      ()         { return m_bUseWPO;        }
  Bool getUseLDC      ()         { return m_bUseLDC;        }
  Bool getUsePAD      ()         { return m_bUsePAD;        }
  Bool getUseQBO      ()         { return m_bUseQBO;        }
#if HHI_ALLOW_CIP_SWITCH
  Bool getUseCIP      ()         { return m_bUseCIP;        }	// BB:
#endif
  Bool getUseROT			()				 { return m_bUseROT;				} // BB:
#if HHI_AIS
  Bool getUseAIS      ()         { return m_bUseAIS;        } // BB:
#endif
#if HHI_MRG
  Bool getUseMRG      ()         { return m_bUseMRG;        } // SOPH:
#endif
#if HHI_IMVP
  Bool getUseIMP      ()         { return m_bUseIMP;        } // SOPH:
#endif

  Int  getDIFTap      ()         { return m_iDIFTap;        }
#if SAMSUNG_CHROMA_IF_EXT
  Int  getDIFTapC     ()         { return m_iDIFTapC;       };
#endif

#ifdef QC_AMVRES
	Bool getUseAMVRes      ()         { return m_bUseAMVRes;        }
#endif
  Void setUseALF      ( Bool b ) { m_bUseALF  = b;          }
  Void setUseDQP      ( Bool b ) { m_bUseDQP   = b;         }

  Void setUseWPG      ( Bool b ) { m_bUseWPG   = b; if (m_bUseWPG && m_bUseWPO) m_bUseWPO =  false;}
  Void setUseWPO      ( Bool b ) { m_bUseWPO   = b; if (m_bUseWPG && m_bUseWPO) m_bUseWPG =  false;}
  Void setUseLDC      ( Bool b ) { m_bUseLDC   = b;         }
  Void setUsePAD      ( Bool b ) { m_bUsePAD   = b;         }
  Void setUseQBO      ( Bool b ) { m_bUseQBO   = b;         }
#if HHI_ALLOW_CIP_SWITCH
	Void setUseCIP      ( Bool b ) { m_bUseCIP   = b;         }	// BB:
#endif
  Void setUseROT			( Bool b ) { m_bUseROT	 = b;					} // BB:
#if HHI_AIS
  Void setUseAIS      ( Bool b ) { m_bUseAIS   = b;         } // BB:
#endif
#if HHI_MRG
  Void setUseMRG      ( Bool b ) { m_bUseMRG  = b;          } // SOPH:
#endif
#if HHI_IMVP
  Void setUseIMP      ( Bool b ) { m_bUseIMP  = b;          } // SOPH:
#endif

  Void setDIFTap      ( Int  i ) { m_iDIFTap   = i;         }
 #if SAMSUNG_CHROMA_IF_EXT
  Void setDIFTapC     ( Int  i ) { m_iDIFTapC = i;          };
#endif

#ifdef QC_AMVRES
	Void setUseAMVRes      ( Bool b ) { m_bUseAMVRes    =b;        }
#endif
#if HHI_ALF
  // ALF Parameters
  Bool getALFSeparateQt ()         { return m_bALFSeparateQt; }
  Bool getALFSymmetry   ()         { return m_bALFSymmetry;   }
  Int  getALFMinLength  ()         { return m_iALFMinLength;  }
  Int  getALFMaxLength  ()         { return m_iALFMaxLength;  }
  Void setALfSeparateQt ( Bool b ) { m_bALFSeparateQt = b;    }
  Void setALFSymmetry   ( Bool b ) { m_bALFSymmetry   = b;    }
  Void setALFMinLength  ( Int  i ) { m_iALFMinLength  = i;    }
  Void setALFMaxLength  ( Int  i ) { m_iALFMaxLength  = i;    }
#endif

#ifdef QC_SIFO_PRED
	Void setUseSIFO_Pred      ( Bool b ) { m_bUseSIFO_Pred    =b;        }
	Bool getUseSIFO_Pred      ()         { return m_bUseSIFO_Pred;        }
#endif

#ifdef DCM_PBIC 
	Void setUseIC       ( Bool b ) { m_bUseIC   = b;          }
  Bool getUseIC       ()         { return m_bUseIC;         }
#endif

  Bool getUseAMP      ()         { return m_bUseAMP; }
  Void setUseAMP      ( Bool b ) { m_bUseAMP  = b; }

#if HHI_RMP_SWITCH
  Bool getUseRMP     ()         { return m_bUseRMP; }
  Void setUseRMP     ( Bool b ) { m_bUseRMP = b;    }
#endif

  // AMVP mode (for each depth)
  AMVP_MODE getAMVPMode ( UInt uiDepth ) { assert(uiDepth < g_uiMaxCUDepth);  return m_aeAMVPMode[uiDepth]; }
  Void      setAMVPMode ( UInt uiDepth, AMVP_MODE eMode) { assert(uiDepth < g_uiMaxCUDepth);  m_aeAMVPMode[uiDepth] = eMode; }

  // AMP accuracy
  Int       getAMPAcc   ( UInt uiDepth ) { return m_iAMPAcc[uiDepth]; }
  Void      setAMPAcc   ( UInt uiDepth, Int iAccu ) { assert( uiDepth < g_uiMaxCUDepth);  m_iAMPAcc[uiDepth] = iAccu; }

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
  SliceType   m_eSliceType;
  Int         m_iSliceQp;
  Int         m_iSymbolMode ;
  Bool        m_bMultiCodeword;
  UInt        m_uiMaxPIPEDelay;
  Bool        m_bLoopFilterDisable;

  UInt        m_uiBalancedCPUs;

  Bool        m_bDRBFlag;             //  flag for future usage as reference buffer
  ERBIndex    m_eERBIndex;            //  flag for future usage as reference buffer
  Int         m_aiNumRefIdx   [2];    //  for multiple reference of current slice

  //  Data
  Int         m_iSliceQpDelta;
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF+GRF_MAX_NUM_EFF];
  Int m_aiRefPOCList[2][MAX_NUM_REF+GRF_MAX_NUM_EFF];
  Int         m_iDepth;

  // referenced slice?
  Bool        m_bRefenced;
#ifdef ROUNDING_CONTROL_BIPRED
  Bool		  m_bRounding;
#endif

  // access channel
  TComSPS*    m_pcSPS;
  TComPPS*    m_pcPPS;
  TComPic*    m_pcPic;

  UInt        m_uiColDir;  // direction to get colocated CUs

  Double      m_dLambda;

  // generated reference frame for weighted prediction
  EFF_MODE    m_aeEffectMode[2][GRF_MAX_NUM_EFF];
  UInt        m_auiAddRefCnt[2];
  TComPic*    m_apcVirtPic  [2][GRF_MAX_NUM_EFF];
  Bool        m_abEqualRef  [2][MAX_NUM_REF+GRF_MAX_NUM_EFF][MAX_NUM_REF+GRF_MAX_NUM_EFF];

  Int         m_iWPWeight   [2][GRF_MAX_NUM_WEFF][3];
  Int         m_iWPOffset   [2][GRF_MAX_NUM_WEFF][3];
  Int         m_aiWPmode[2];
  Int         m_iRFmode;
  Int         m_iLumaLogWeightDenom;
  Int         m_iChromaLogWeightDenom;
  Int         m_iWPLumaRound;
  Int         m_iWPChromaRound;
#ifdef EDGE_BASED_PREDICTION
  Bool        m_bEdgePredictionEnable;
  Int         m_iEdgeDetectionThreshold;
#endif //EDGE_BASED_PREDICTION
#if HHI_INTERP_FILTER
  Int         m_iInterpFilterType;
#endif
#ifdef QC_SIFO_PRED
	Bool        m_bUseSIFO_Pred;
#endif
#ifdef DCM_PBIC
  TComZeroTree* m_apcZTree  [MAX_NUM_ZTREE];
  Void        xCreateZTrees();
  Void        xDeleteZTrees();
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
  Bool      getDRBFlag          ()                      { return  m_bDRBFlag;           }
  ERBIndex  getERBIndex         ()                      { return  m_eERBIndex;          }
  Int       getSymbolMode ()                            { return  m_iSymbolMode;        }
  Bool      getMultiCodeword    ()                      { return  m_bMultiCodeword;     }
  UInt      getMaxPIPEDelay     ()                      { return  m_uiMaxPIPEDelay;     }
  Bool      getLoopFilterDisable()                      { return  m_bLoopFilterDisable; }
  UInt      getBalancedCPUs     ()                      { return  m_uiBalancedCPUs;     }
#ifdef QC_SIFO_PRED
	Void setUseSIFO_Pred      ( Bool b ) { m_bUseSIFO_Pred    =b;        }
	Bool getUseSIFO_Pred      ()         { return m_bUseSIFO_Pred;        }
#endif
  Int       getNumRefIdx        ( RefPicList e )                { return  m_aiNumRefIdx[e];             }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
  TComPic*  getRefPic           ( RefPicList e, Int iRefIdx)    { return  m_apcRefPicList[e][iRefIdx];  }
  Int       getRefPOC           ( RefPicList e, Int iRefIdx)    { return  m_aiRefPOCList[e][iRefIdx];   }
  Int       getDepth            ()                              { return  m_iDepth;                     }
  UInt      getColDir           ()                              { return  m_uiColDir;                   }
#ifdef DCM_PBIC
  TComZeroTree* getZTree        ( UInt uiZTreeIdx )             { return  m_apcZTree[uiZTreeIdx];       }
#endif

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }
#ifdef ROUNDING_CONTROL_BIPRED
  Void      setRounding(Bool bRound)                            { m_bRounding = bRound; }
  Bool      isRounding()                                        { return m_bRounding; }
#endif

  Void      setPOC              ( Int i )                       { m_iPOC              = i;      }
  Void      setSliceType        ( SliceType e )                 { m_eSliceType        = e;      }
  Void      setSliceQp          ( Int i )                       { m_iSliceQp          = i;      }
  Void      setSliceQpDelta     ( Int i )                       { m_iSliceQpDelta     = i;      }
  Void      setDRBFlag          ( Bool b )                      { m_bDRBFlag = b;               }
  Void      setERBIndex         ( ERBIndex e )                  { m_eERBIndex = e;              }
  Void      setSymbolMode       ( Int b  )                      { m_iSymbolMode       = b;      }
  Void      setMultiCodeword    ( Bool b )                      { m_bMultiCodeword    = b;      }
  Void      setMaxPIPEDelay     ( UInt ui )                     { m_uiMaxPIPEDelay    = ui;     }
  Void      setLoopFilterDisable( Bool b )                      { m_bLoopFilterDisable= b;      }
  Void      setBalancedCPUs     ( UInt ui )                     { m_uiBalancedCPUs    = ui;     }

  Void      setRefPic           ( TComPic* p, RefPicList e, Int iRefIdx ) { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC           ( Int i, RefPicList e, Int iRefIdx ) { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setNumRefIdx        ( RefPicList e, Int i )         { m_aiNumRefIdx[e]    = i;      }
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }

  Void      setRefPicList       ( TComList<TComPic*>& rcListPic );
  Void      setRefPOCList       ();
  Void setColDir (UInt uiDir) { m_uiColDir = uiDir; }

  Bool      isIntra         ()                          { return  m_eSliceType == I_SLICE;  }
  Bool      isInterB        ()                          { return  m_eSliceType == B_SLICE;  }
  Bool      isInterP        ()                          { return  m_eSliceType == P_SLICE;  }

  Void      setLambda( Double d ) { m_dLambda = d; }
  Double    getLambda() { return m_dLambda;        }
#ifdef EDGE_BASED_PREDICTION
  Void      setEdgePredictionEnable (Bool b)  { m_bEdgePredictionEnable = b; } 
  Bool      getEdgePredictionEnable ()        { return m_bEdgePredictionEnable; }
  Void      setEdgeDetectionThreshold (Int i) { m_iEdgeDetectionThreshold = i; }
  Int       getEdgeDetectionThreshold ()      { return m_iEdgeDetectionThreshold; }
#endif //EDGE_BASED_PREDICTION
  Void generateWPSlice( RefPicList e, EFF_MODE eEffMode, UInt uiInsertIdx);

  Void initWPParam( RefPicList e, EFF_MODE eEffMode, Int colorChannel);
  Void initWPAllParam( RefPicList e, EFF_MODE eEffMode);

  Bool isEqualWPParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2, Int colorChannel);
  Bool isEqualWPAllParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2);

  Void  setWPWeight ( RefPicList e, EFF_MODE eEffMode, Int colorChannel, Int weight)    { m_iWPWeight[e][eEffMode-EFF_WP_SO][colorChannel]=weight; }
  Void  setWPOffset ( RefPicList e, EFF_MODE eEffMode, Int colorChannel, Int offset)    { m_iWPOffset[e][eEffMode-EFF_WP_SO][colorChannel]=offset; }
  Int   getWPWeight ( RefPicList e, EFF_MODE eEffMode, Int colorChannel ) { return m_iWPWeight[e][eEffMode-EFF_WP_SO][colorChannel]; }
  Int   getWPOffset ( RefPicList e, EFF_MODE eEffMode, Int colorChannel ) { return m_iWPOffset[e][eEffMode-EFF_WP_SO][colorChannel]; }

  Void  setWPmode(RefPicList e, Int iWpMode) { m_aiWPmode[e]=iWpMode; }
  Int   getWPmode(RefPicList e) { return m_aiWPmode[e]; }
  Void  setRFmode(Int iRfMode) { m_iRFmode=iRfMode; }
  Int   getRFmode() { return m_iRFmode; }

  EFF_MODE getEffectMode(RefPicList e, Int i) {return m_aeEffectMode[e][i]; }
  Void addEffectMode(RefPicList e, EFF_MODE eEffMode) {m_aeEffectMode[e][m_auiAddRefCnt[e]]=eEffMode; m_auiAddRefCnt[e] ++;}
  Void removeEffectMode(RefPicList e, EFF_MODE eEffMode);

  UInt getAddRefCnt(RefPicList e) { return m_auiAddRefCnt[e];  }
  Void clearAddRefCnt(RefPicList e) { m_auiAddRefCnt[e] = 0;     }

  Void setVirtRefBuffer(TComPic* pSrc[2][GRF_MAX_NUM_EFF]);
  Void linkVirtRefPic();

  Void initEqualRef();
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

#if HHI_INTERP_FILTER
  Int  getInterpFilterType     ()         { return m_iInterpFilterType;       }
  Void setInterpFilterType     ( Int  i ) { m_iInterpFilterType  = i;         }
  Bool getUseMOMS              ()         { return ( ( m_iInterpFilterType == IPF_HHI_4TAP_MOMS || m_iInterpFilterType == IPF_HHI_6TAP_MOMS ) ); }
#endif

#ifdef QC_SIFO
  Bool getUseSIFO              ()         { return ( m_iInterpFilterType == IPF_QC_SIFO); }
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

