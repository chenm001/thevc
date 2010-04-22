/* ====================================================================================================================

	The copyright in this software is being made available under the License included below.
	This software may be subject to other third party and 	contributor rights, including patent rights, and no such
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

/** \file			TComSlice.h
    \brief		slice header and SPS class (header)
*/

#ifndef __TCOMSLICE__
#define __TCOMSLICE__


#include "CommonDef.h"
#include "TComList.h"

class TComPic;
class TComGlobalMotion;

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
  UInt				m_uiMaxCUWidth;
  UInt				m_uiMaxCUHeight;
  UInt				m_uiMaxCUDepth;
  UInt        m_uiMinTrDepth;
  UInt        m_uiMaxTrDepth;

  // IP list
  Bool        m_bUseMVAC;
  Bool        m_bUseADI;
  Bool				m_bUseROT;
  Bool				m_bUseMPI;
  Bool				m_bUseAMVP;
  Bool				m_bUseIMR;
  Bool				m_bUseDIF;
  Bool				m_bUseALF;
  Bool				m_bUseDQP;
  Bool				m_bUseRNG;
  Bool				m_bUseAMP;
  Bool			  m_bUseSHV;

  Bool        m_bUseWPG;
  Bool        m_bUseWPO;
  Bool        m_bUseWPR;
  Bool        m_bUseAME;
  Bool        m_bUseIME;
  Bool        m_bUsePME;
  Bool        m_bUseLOT;
  Bool        m_bUseEXC;
  Bool        m_bUseCCP;
  Bool        m_bUseTMI;
  Bool        m_bUseLDC;
	Bool        m_bUseCIP;
	Bool        m_bUsePAD;
  Bool        m_bUseLCT;
	Bool        m_bUseQBO;
  Bool        m_bUseACS;
  Bool				m_bUseHAP;
  Bool				m_bUseHAB;
  Bool				m_bUseHME;
	Int					m_iDIFTap;
	Int					m_iDIFTapC;

	// Parameter
  AMVP_MODE		m_aeAMVPMode[MAX_CU_DEPTH];
  UInt				m_uiBitDepth;
  UInt				m_uiBitIncrement;

	// Max physical transform size
  UInt				m_uiMaxTrSize;

  Int m_iFMPAccuracy[MAX_CU_DEPTH];

public:
  TComSPS();
  virtual ~TComSPS();

	// structure
	Void setWidth				( UInt u ) { m_uiWidth = u;						}
	UInt getWidth				()         { return  m_uiWidth;				}
	Void setHeight			( UInt u ) { m_uiHeight = u;					}
  UInt getHeight			()         { return  m_uiHeight;			}
	Void setMaxCUWidth	( UInt u ) { m_uiMaxCUWidth = u;			}
	UInt getMaxCUWidth	()				 { return  m_uiMaxCUWidth;	}
	Void setMaxCUHeight	( UInt u ) { m_uiMaxCUHeight = u;			}
	UInt getMaxCUHeight	()				 { return  m_uiMaxCUHeight; }
	Void setMaxCUDepth	( UInt u ) { m_uiMaxCUDepth = u;      }
	UInt getMaxCUDepth	()				 { return  m_uiMaxCUDepth;  }
  Void setMinTrDepth  ( UInt u ) { m_uiMinTrDepth = u;      }
  UInt getMinTrDepth  ()         { return  m_uiMinTrDepth;  }
  Void setMaxTrDepth  ( UInt u ) { m_uiMaxTrDepth = u;      }
  UInt getMaxTrDepth  ()         { return  m_uiMaxTrDepth;  }
  Void setPad         (Int iPad[2]) { m_aiPad[0] = iPad[0]; m_aiPad[1] = iPad[1]; }
  Void setPadX        ( Int  u ) { m_aiPad[0] = u; }
  Void setPadY        ( Int  u ) { m_aiPad[1] = u; }
  Int  getPad         ( Int  u ) { assert(u < 2); return m_aiPad[u];}
  Int* getPad         ( )        { return m_aiPad; }

	// physical transform
	Void setMaxTrSize		( UInt u ) { m_uiMaxTrSize = u;       }
	UInt getMaxTrSize		()				 { return  m_uiMaxTrSize;		}

  // IP List
	Bool getUseMVAC			()				 { return m_bUseMVAC;				}
  Bool getUseADI			()				 { return m_bUseADI;				}
  Bool getUseROT			()				 { return m_bUseROT;				}
  Bool getUseMPI			()				 { return m_bUseMPI;				}
  Bool getUseAMVP			()				 { return m_bUseAMVP;				}
  Bool getUseIMR			()				 { return m_bUseIMR;				}
  Bool getUseDIF			()				 { return m_bUseDIF;				}
  Bool getUseALF			()				 { return m_bUseALF;				}
  Bool getUseDQP			()				 { return m_bUseDQP;				}
  Bool getUseRNG			()				 { return m_bUseRNG;				}
  Bool getUseAMP			()				 { return m_bUseAMP;				}
  Bool getUseSHV			()				 { return m_bUseSHV;				}

  Bool getUseWPG			()				 { return m_bUseWPG;				}
  Bool getUseWPO			()				 { return m_bUseWPO;				}
  Bool getUseWPR			()				 { return m_bUseWPR;				}
  Bool getUseAME			()				 { return m_bUseAME;				}
  Bool getUseIME			()				 { return m_bUseIME;				}
  Bool getUsePME			()				 { return m_bUsePME;				}
  Bool getUseLOT      ()         { return m_bUseLOT;        }
  Bool getUseExC      ()         { return m_bUseEXC;        }
  Bool getUseCCP      ()         { return m_bUseCCP;        }
  Bool getUseTMI      ()         { return m_bUseTMI;        }
  Bool getUseLDC      ()         { return m_bUseLDC;        }
	Bool getUseCIP      ()         { return m_bUseCIP;        }
	Bool getUsePAD      ()         { return m_bUsePAD;        }
  Bool getUseLCT      ()         { return m_bUseLCT;        }
	Bool getUseQBO      ()         { return m_bUseQBO;        }

  Bool getUseACS      ()         { return m_bUseACS;        }

  Bool getUseHAP      ()         { return m_bUseHAP;        }
  Bool getUseHAB      ()         { return m_bUseHAB;        }
  Bool getUseHME      ()         { return m_bUseHME;        }

	Int	 getDIFTap			()				 { return m_iDIFTap;				}
	Int	 getDIFTapC			()				 { return m_iDIFTapC;				}

  Void setUseMVAC			( Bool b ) { m_bUseMVAC  = b;					}
  Void setUseADI			( Bool b ) { m_bUseADI	 = b;					}
  Void setUseROT			( Bool b ) { m_bUseROT	 = b;					}
  Void setUseMPI			( Bool b ) { m_bUseMPI	 = b;					}
  Void setUseAMVP			( Bool b ) { m_bUseAMVP  = b;					}
  Void setUseIMR			( Bool b ) { m_bUseIMR  = b;					}
  Void setUseDIF			( Bool b ) { m_bUseDIF  = b;					}
  Void setUseALF			( Bool b ) { m_bUseALF  = b;					}
  Void setUseDQP			( Bool b ) { m_bUseDQP   = b;					}
  Void setUseRNG			( Bool b ) { m_bUseRNG   = b;					}
  Void setUseAMP			( Bool b ) { m_bUseAMP   = b;					}
  Void setUseSHV			( Bool b ) { m_bUseSHV	 = b;					}

  Void setUseWPG			( Bool b ) { m_bUseWPG	 = b; if (m_bUseWPG && m_bUseWPO) m_bUseWPO =  false;}
  Void setUseWPO			( Bool b ) { m_bUseWPO	 = b; if (m_bUseWPG && m_bUseWPO) m_bUseWPG =  false;}
  Void setUseWPR			( Bool b ) { m_bUseWPR	 = b;					}
  Void setUseAME			( Bool b ) { m_bUseAME	 = b;					}
  Void setUseIME			( Bool b ) { m_bUseIME	 = b;					}
  Void setUsePME			( Bool b ) { m_bUsePME	 = b;					}
  Void setUseLOT      ( Bool b ) { m_bUseLOT   = b;         }
  Void setUseExC      ( Bool b ) { m_bUseEXC   = b;         }
  Void setUseCCP      ( Bool b ) { m_bUseCCP   = b;         }
  Void setUseTMI      ( Bool b ) { m_bUseTMI   = b;         }
  Void setUseLDC      ( Bool b ) { m_bUseLDC   = b;         }
	Void setUseCIP      ( Bool b ) { m_bUseCIP   = b;         }
	Void setUsePAD      ( Bool b ) { m_bUsePAD   = b;         }
  Void setUseLCT      ( Bool b ) { m_bUseLCT   = b;         }
	Void setUseQBO      ( Bool b ) { m_bUseQBO   = b;         }
  Void setUseACS      ( Bool b ) { m_bUseACS   = b;         }

  Void setUseHAP      ( Bool b ) { m_bUseHAP   = b;         }
  Void setUseHAB      ( Bool b ) { m_bUseHAB   = b;         }
  Void setUseHME      ( Bool b ) { m_bUseHME   = b;         }

	Void setDIFTap			( Int  i ) { m_iDIFTap   = i;         }
	Void setDIFTapC			( Int  i ) { m_iDIFTapC  = i;         }

	// Parameter
  AMVP_MODE getAMVPMode(UInt uiDepth) { assert(uiDepth < g_uiMaxCUDepth);	return m_aeAMVPMode[uiDepth]; }
  Void			setAMVPMode(UInt uiDepth, AMVP_MODE eMode) { assert(uiDepth < g_uiMaxCUDepth);	m_aeAMVPMode[uiDepth] = eMode; }
  Int   getFMPAccuracy (UInt uiDepth) { return m_iFMPAccuracy[uiDepth]; }
  Void  setFMPAccuracy (UInt uiDepth, Int iAccu) { assert( uiDepth < g_uiMaxCUDepth);	m_iFMPAccuracy[uiDepth] = iAccu; }
	UInt getBitDepth		()				 { return m_uiBitDepth;			}
	Void setBitDepth		( UInt u ) { m_uiBitDepth = u;        }
	UInt getBitIncrement()				 { return m_uiBitIncrement;	}
	Void setBitIncrement( UInt u ) { m_uiBitIncrement = u;    }
};// END CLASS DEFINITION TComSPS

/// slice header class
class TComSlice
{

private:
  //  Bitstream writing
  Int         m_iPOC;
  SliceType   m_eSliceType;
  Int         m_iSliceQp;
  Int         m_iSymbolMode ;
  Bool        m_bLoopFilterDisable;

  Bool        m_bDRBFlag;             //  flag for future usage as reference buffer
  ERBIndex    m_eERBIndex;            //  flag for future usage as reference buffer
  Int         m_aiNumRefIdx   [2];    //  for multiple reference of current slice

  //  Data
  Int         m_iSliceQpDelta;
  TComPic*    m_apcRefPicList [2][MAX_NUM_REF+GRF_MAX_NUM_EFF];
  Int m_aiRefPOCList[2][MAX_NUM_REF+GRF_MAX_NUM_EFF];
  Int         m_iDepth;

	// Referenced slice?
  Bool        m_bRefenced;

  //  Access channel
  TComSPS*		m_pcSPS;
  TComPic*    m_pcPic;

  UInt m_uiColDir;  // direction to get colocated CUs

	Double			m_dLambda;

  Int m_iWPWeight[2][GRF_MAX_NUM_WEFF][3];
  Int m_iWPOffset[2][GRF_MAX_NUM_WEFF][3];
  Int m_aiWPmode[2]; // flag wheter wp is used
  Int m_iRFmode; // flag wheter wp refinement is used
  Int m_iLumaLogWeightDenom;
  Int m_iChromaLogWeightDenom;
  Int m_iWPLumaRound;
  Int m_iWPChromaRound;

  EFF_MODE m_aeEffectMode[2][GRF_MAX_NUM_EFF];
  UInt m_auiAddRefCnt[2];
  TComPic* m_apcVirtPic[2][GRF_MAX_NUM_EFF];
  Bool m_abEqualRef[2][MAX_NUM_REF+GRF_MAX_NUM_EFF][MAX_NUM_REF+GRF_MAX_NUM_EFF];

  UInt m_auiGMmode[2]; // Global motion model (0: 0ff, 1: isotropic, 2: affine, 3: perspective)

  UInt m_uiNumOfSpritePoints;
  UInt m_uiWarpingAccuracy;
  Int m_iHorSpatRef;
  Int m_iVerSpatRef;

  TrajPoint m_acRefPointCoord[2][4];
  TrajPoint m_acTrajPointCoord[2][4];
  TrajPoint m_acDiffTrajPointCoord[2][4];

  Bool m_bUseMVAC;
  Bool m_bUseHAM;
  Bool m_bUseHME;

public:
  TComSlice();
  virtual ~TComSlice();

  Bool getUseHAM      ()         { return m_bUseHAM;        }
  Bool getUseHME      ()         { return m_bUseHME;        }
  Void setUseHAM      ( Bool b ) { m_bUseHAM   = b;         }
  Void setUseHME      ( Bool b ) { m_bUseHME   = b;         }

  Void      initSlice       ();

	Void			setSPS					( TComSPS* pcSPS ) { m_pcSPS = pcSPS; }
	TComSPS*	getSPS					() { return m_pcSPS; }

  SliceType getSliceType    ()                          { return  m_eSliceType;         }
  Int       getPOC          ()                          { return  m_iPOC;						}
  Int       getSliceQp      ()                          { return  m_iSliceQp;           }
  Int       getSliceQpDelta ()                          { return  m_iSliceQpDelta;      }
  Bool      getDRBFlag          ()                      { return  m_bDRBFlag;           }
  ERBIndex  getERBIndex         ()                      { return  m_eERBIndex;          }
  Int       getSymbolMode ()														{ return  m_iSymbolMode;				}
  Bool      getLoopFilterDisable()                      { return m_bLoopFilterDisable;  }

  Int       getNumRefIdx        ( RefPicList e )                { return  m_aiNumRefIdx[e];             }
  TComPic*  getPic              ()                              { return  m_pcPic;                      }
  TComPic*  getRefPic           ( RefPicList e, Int iRefIdx)    { return  m_apcRefPicList[e][iRefIdx];  }
  Int  getRefPOC           ( RefPicList e, Int iRefIdx)    { return  m_aiRefPOCList[e][iRefIdx];  }
  Int       getDepth            ()                              { return  m_iDepth;                     }
  UInt getColDir() { return m_uiColDir; }

  UInt                getNumOfSpritePoints     () { return (m_auiGMmode[0]? m_auiGMmode[0]+1 : m_auiGMmode[1]+1); }
  UInt                getWarpingAccuracy       () { return m_uiWarpingAccuracy; }
  Int                getHorSpatRef      () { return m_iHorSpatRef; }
  Int                getVerSpatRef      () { return m_iVerSpatRef; }

  Void                setWarpingAccuracy (UInt ui)    { m_uiWarpingAccuracy = ui;   }
  Void                setHorSpatRef (Int i)    { m_iHorSpatRef = i;   }
  Void                setVerSpatRef (Int i)    { m_iVerSpatRef = i;   }

  Void      setReferenced(Bool b)                               { m_bRefenced = b; }
  Bool      isReferenced()                                      { return m_bRefenced; }

  Void      setPOC              ( Int i )                       { m_iPOC              = i;      }
  Void      setSliceType        ( SliceType e )                 { m_eSliceType        = e;      }
  Void      setSliceQp          ( Int i )                       { m_iSliceQp          = i;      }
  Void      setSliceQpDelta     ( Int i )                       { m_iSliceQpDelta     = i;      }
  Void      setDRBFlag          ( Bool b )                      { m_bDRBFlag = b;               }
  Void      setERBIndex         ( ERBIndex e )                  { m_eERBIndex = e;              }
  Void      setSymbolMode       ( Int b	 )						        	{ m_iSymbolMode				= b;      }
  Void      setLoopFilterDisable( Bool b )                      { m_bLoopFilterDisable= b;      }

  Void      setRefPic						( TComPic* p, RefPicList e, Int iRefIdx ) { m_apcRefPicList[e][iRefIdx] = p; }
  Void      setRefPOC						( Int i, RefPicList e, Int iRefIdx ) { m_aiRefPOCList[e][iRefIdx] = i; }
  Void      setNumRefIdx        ( RefPicList e, Int i )         { m_aiNumRefIdx[e]    = i;      }
  Void      setPic              ( TComPic* p )                  { m_pcPic             = p;      }
  Void      setDepth            ( Int iDepth )                  { m_iDepth            = iDepth; }

  Void      setRefPicList       ( TComList<TComPic*>& rcListPic );
  Void      setRefPOCList       ();
  Void setColDir (UInt uiDir) { m_uiColDir = uiDir; }

  Bool      isIntra         ()                          { return  m_eSliceType == I_SLICE;  }
  Bool      isInterB        ()                          { return  m_eSliceType == B_SLICE;  }
  Bool      isInterP        ()                          { return  m_eSliceType == P_SLICE;  }

	Void			setLambda( Double d ) { m_dLambda = d; }
	Double		getLambda() { return m_dLambda;				 }

  Void generateWPSlice( RefPicList e, EFF_MODE eEffMode, UInt uiInsertIdx);

  Void initWPParam( RefPicList e, EFF_MODE eEffMode, Int colorChannel);
  Void initWPAllParam( RefPicList e, EFF_MODE eEffMode);

  Bool isEqualWPParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2, Int colorChannel);
  Bool isEqualWPAllParam( RefPicList e, EFF_MODE eEffMode1, EFF_MODE eEffMode2);

  Void setWPWeight ( RefPicList e, EFF_MODE eEffMode, Int colorChannel, Int weight)    { m_iWPWeight[e][eEffMode-EFF_WP_SO][colorChannel]=weight; }
  Void setWPOffset ( RefPicList e, EFF_MODE eEffMode, Int colorChannel, Int offset)    { m_iWPOffset[e][eEffMode-EFF_WP_SO][colorChannel]=offset; }
  Int getWPWeight ( RefPicList e, EFF_MODE eEffMode, Int colorChannel ) { return m_iWPWeight[e][eEffMode-EFF_WP_SO][colorChannel]; }
  Int getWPOffset ( RefPicList e, EFF_MODE eEffMode, Int colorChannel ) { return m_iWPOffset[e][eEffMode-EFF_WP_SO][colorChannel]; }

  Void setWPmode(RefPicList e, Int iWpMode) { m_aiWPmode[e]=iWpMode; }
  Int  getWPmode(RefPicList e) { return m_aiWPmode[e]; }
  Void setRFmode(Int iRfMode) { m_iRFmode=iRfMode; }
  Int  getRFmode() { return m_iRFmode; }
  Bool isTrajAllZero(RefPicList e);
  Bool isTrajZero(RefPicList e, UInt uiPoint);
  Void setGMmode(RefPicList e, UInt uiGmMode) { m_auiGMmode[e] = uiGmMode; }
  UInt  getGMmode(RefPicList e) { return m_auiGMmode[e]; }

  Void setDiffTrajPoint(RefPicList e, UInt uiPoint, TrajPoint cTrajPoint) { m_acDiffTrajPointCoord[e][uiPoint] = cTrajPoint; }
  TrajPoint  getDiffTrajPoint(RefPicList e, UInt uiPoint) { return m_acDiffTrajPointCoord[e][uiPoint]; }
  TrajPoint*  getDiffTrajPoints(RefPicList e) { return m_acDiffTrajPointCoord[e]; }

  Void setTrajPoint(RefPicList e, UInt uiPoint, TrajPoint cTrajPoint) { m_acTrajPointCoord[e][uiPoint] = cTrajPoint; }
  TrajPoint*  getTrajPoints(RefPicList e) { return m_acTrajPointCoord[e]; }

  Void setRefPoint(RefPicList e, UInt uiPoint, TrajPoint cTrajPoint) { m_acRefPointCoord[e][uiPoint] = cTrajPoint; }
  TrajPoint*  getRefPoints(RefPicList e) { return m_acRefPointCoord[e]; }

  Void generateGMSlice( RefPicList e, UInt uiInsertIdx, TComGlobalMotion* pcGlobalMotion);

  EFF_MODE getEffectMode(RefPicList e, Int i) {return m_aeEffectMode[e][i]; }
  Void addEffectMode(RefPicList e, EFF_MODE eEffMode) {m_aeEffectMode[e][m_auiAddRefCnt[e]]=eEffMode; m_auiAddRefCnt[e] ++;}
  Void removeEffectMode(RefPicList e, EFF_MODE eEffMode);

  UInt getAddRefCnt(RefPicList e) { return m_auiAddRefCnt[e];  }
	Void clearAddRefCnt(RefPicList e) { m_auiAddRefCnt[e] = 0;     }

  Void setVirtRefBuffer(TComPic* pSrc[2][GRF_MAX_NUM_EFF]);
  Void linkVirtRefPic();

  Void initEqualRef();
  Bool isEqualRef(RefPicList e, Int iRefIdx1, Int iRefIdx2) { if (iRefIdx1 < 0 || iRefIdx2 < 0) return false;	return m_abEqualRef[e][iRefIdx1][iRefIdx2]; }
  Void setEqualRef(RefPicList e, Int iRefIdx1, Int iRefIdx2, Bool b) { m_abEqualRef[e][iRefIdx1][iRefIdx2] = m_abEqualRef[e][iRefIdx2][iRefIdx1] = b; }

  static Void      sortPicList         (TComList<TComPic*>& rcListPic);

  Bool getUseMVAC()         { return m_bUseMVAC; }
  Void setUseMVAC( Bool b ) { m_bUseMVAC = b;    }

  Int   getFMPAccuracy (UInt uiDepth) { return m_pcSPS->getFMPAccuracy(uiDepth); }

protected:
  TComPic*  xGetRefPic  (TComList<TComPic*>& rcListPic,
                         Bool                bDRBFlag,
                         ERBIndex            eERBIndex,
                         UInt                uiPOCCurr,
                         RefPicList          eRefPicList,
                         UInt                uiNthRefPic );

};// END CLASS DEFINITION TComSlice


#endif // __TCOMSLICE__
