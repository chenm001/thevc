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

/** \file     TComDataCU.h
    \brief    CU data structure (header)
    \todo     not all entities are documented
*/

#ifndef _TCOMDATACU_
#define _TCOMDATACU_

#include <assert.h>

// Include files
#include "CommonDef.h"
#include "TComMotionInfo.h"
#include "TComSlice.h"
#include "TComRdCost.h"
#include "TComPattern.h"

#if HHI_IMVP || HHI_RQT
#include <algorithm>
#include <vector>
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if HHI_IMVP
class MvPredMeasure
{
public:
  TComMv getMVPred( const Int iMvCompX, const Int iMvCompY )  const
  {
    const Int iSize     = Int( m_aiYThreshold.size() )-1;
    Int       iXOrigin  = m_aiXOrigin [ iSize ];
    for(  int iLstIdx   = 0;  iLstIdx < iSize; iLstIdx++ )
    {
      if( iMvCompY <= m_aiYThreshold[ iLstIdx ] )
      {
        iXOrigin = m_aiXOrigin   [ iLstIdx ];
        break;
      }
    }
    TComMv MVPred;
    MVPred.setVer(m_iYOrigin);
    MVPred.setHor(iXOrigin);
    return MVPred;
  } 

  Int                       m_iYOrigin;
  std::vector<Int>          m_aiYThreshold;
  std::vector<Int>          m_aiXOrigin;
};

#endif

/// CU data structure class
class TComDataCU
{
private:

  // -------------------------------------------------------------------------------------------------------------------
  // class pointers
  // -------------------------------------------------------------------------------------------------------------------

  TComPic*      m_pcPic;              ///< picture class pointer
  TComSlice*    m_pcSlice;            ///< slice header pointer
  TComPattern*  m_pcPattern;          ///< neighbour access class pointer

  // -------------------------------------------------------------------------------------------------------------------
  // CU description
  // -------------------------------------------------------------------------------------------------------------------

  UInt          m_uiCUAddr;           ///< CU address in a slice
  UInt          m_uiAbsIdxInLCU;      ///< absolute address in a CU. It's Z scan order
  UInt          m_uiCUPelX;           ///< CU position in a pixel (X)
  UInt          m_uiCUPelY;           ///< CU position in a pixel (Y)
  UInt          m_uiNumPartition;     ///< total number of minimum partitions in a CU
  UChar*        m_puhWidth;           ///< array of widths
  UChar*        m_puhHeight;          ///< array of heights
  UChar*        m_puhDepth;           ///< array of depths

  // -------------------------------------------------------------------------------------------------------------------
  // CU data
  // -------------------------------------------------------------------------------------------------------------------

  PartSize*     m_pePartSize;         ///< array of partition sizes
  PredMode*     m_pePredMode;         ///< array of prediction modes
  UChar*        m_phQP;               ///< array of QP values
  UChar*        m_puhTrIdx;           ///< array of transform indices
  UChar*        m_puhCbf[3];          ///< array of coded block flags (CBF)
  TComCUMvField m_acCUMvField[2];     ///< array of motion vectors
  TCoeff*       m_pcTrCoeffY;         ///< transformed coefficient buffer (Y)
  TCoeff*       m_pcTrCoeffCb;        ///< transformed coefficient buffer (Cb)
  TCoeff*       m_pcTrCoeffCr;        ///< transformed coefficient buffer (Cr)

  // -------------------------------------------------------------------------------------------------------------------
  // neighbour access variables
  // -------------------------------------------------------------------------------------------------------------------

  TComDataCU*   m_pcCUAboveLeft;      ///< pointer of above-left CU
  TComDataCU*   m_pcCUAboveRight;     ///< pointer of above-right CU
  TComDataCU*   m_pcCUAbove;          ///< pointer of above CU
  TComDataCU*   m_pcCULeft;           ///< pointer of left CU
  TComDataCU*   m_apcCUColocated[2];  ///< pointer of temporally colocated CU's for both directions
  TComMvField   m_cMvFieldA;          ///< motion vector of position A
  TComMvField   m_cMvFieldB;          ///< motion vector of position B
  TComMvField   m_cMvFieldC;          ///< motion vector of position C
  TComMv        m_cMvPred;            ///< motion vector predictor

  // -------------------------------------------------------------------------------------------------------------------
  // coding tool information
  // -------------------------------------------------------------------------------------------------------------------

#if HHI_MRG
  Bool*         m_pbMergeFlag;        ///< array of merge flags
  UChar*        m_puhMergeIndex;      ///< array of merge candidate indices
#endif
  UChar*        m_puhLumaIntraDir;    ///< array of intra directions (luma)
#if HHI_AIS
  Bool*         m_pbLumaIntraFiltFlag;///< BB: array of intra smoothing flags (luma)
#endif
  UChar*        m_puhChromaIntraDir;  ///< array of intra directions (chroma)
  UChar*        m_puhInterDir;        ///< array of inter directions
  Int*          m_apiMVPIdx[2];       ///< array of motion vector predictor candidates
  Int*          m_apiMVPNum[2];       ///< array of number of possible motion vectors predictors
  UChar*        m_pROTindex;          ///< array of ROT indices
  UChar*        m_pCIPflag;           ///< array of CIP flags
  UInt*         m_puiAlfCtrlFlag;     ///< array of ALF flags
  UInt*         m_puiTmpAlfCtrlFlag;  ///< temporal array of ALF flags
#if PLANAR_INTRA
  Int*          m_piPlanarInfo[4];
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // misc. variables
  // -------------------------------------------------------------------------------------------------------------------

  Bool          m_bDecSubCu;          ///< indicates decoder-mode
  Double        m_dTotalCost;         ///< sum of partition RD costs
  UInt          m_uiTotalDistortion;  ///< sum of partition distortion
  UInt          m_uiTotalBits;        ///< sum of partition bits

protected:

  /// add possible motion vector predictor candidates
  Bool          xAddMVPCand           ( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir );

  /// remove redundant candidates
  Void          xUniqueMVPCand        ( AMVPInfo* pInfo );
#ifdef QC_AMVRES
#if HHI_IMVP
  Void          xUniqueMVPCand_one_fourth(AMVPInfo* pInfo, Int uiPartIdx);
#else
  Void          xUniqueMVPCand_one_fourth(AMVPInfo* pInfo);
#endif
#endif
  /// compute required bits to encode MVD (used in AMVP)
  UInt          xGetMvdBits           ( TComMv cMvd );
  UInt          xGetComponentBits     ( Int iVal );

  /// compute scaling factor from POC difference
  Int           xGetDistScaleFactor   ( Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC );

  /// calculate all CBF's from coefficients
  Void          xCalcCuCbf            ( UChar* puhCbf, UInt uiTrDepth, UInt uiCbfDepth, UInt uiCuDepth );

public:
  TComDataCU();
  virtual ~TComDataCU();

  // -------------------------------------------------------------------------------------------------------------------
  // create / destroy / initialize / copy
  // -------------------------------------------------------------------------------------------------------------------

  Void          create                ( UInt uiNumPartition, UInt uiWidth, UInt uiHeight, Bool bDecSubCu );
  Void          destroy               ();

  Void          initCU                ( TComPic* pcPic, UInt uiCUAddr );
  Void          initEstData           ();
  Void          initSubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );

  Void          copySubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  Void          copyInterPredInfoFrom ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList );
  Void          copyInterPredInfoTo   ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList );
  Void          copyPartFrom          ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );

  Void          copyToPic             ( UChar uiDepth );
  Void          copyToPic             ( UChar uiDepth, UInt uiPartIdx, UInt uiPartDepth );

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU description
  // -------------------------------------------------------------------------------------------------------------------

  TComPic*      getPic                ()                        { return m_pcPic;           }
  TComSlice*    getSlice              ()                        { return m_pcSlice;         }
  UInt&         getAddr               ()                        { return m_uiCUAddr;        }
  UInt&         getZorderIdxInCU      ()                        { return m_uiAbsIdxInLCU; }
  UInt          getCUPelX             ()                        { return m_uiCUPelX;        }
  UInt          getCUPelY             ()                        { return m_uiCUPelY;        }
  TComPattern*  getPattern            ()                        { return m_pcPattern;       }

  UChar*        getDepth              ()                        { return m_puhDepth;        }
  UChar         getDepth              ( UInt uiIdx )            { return m_puhDepth[uiIdx]; }
  Void          setDepth              ( UInt uiIdx, UChar  uh ) { m_puhDepth[uiIdx] = uh;   }

  Void          setDepthSubParts      ( UInt uiDepth, UInt uiAbsPartIdx );
  Void          setIsDecoderCU        ( Bool bIsDecoder )       { m_bDecSubCu = bIsDecoder; }

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU data
  // -------------------------------------------------------------------------------------------------------------------

  PartSize*     getPartitionSize      ()                        { return m_pePartSize;        }
  PartSize      getPartitionSize      ( UInt uiIdx )            { return m_pePartSize[uiIdx]; }
  Void          setPartitionSize      ( UInt uiIdx, PartSize uh){ m_pePartSize[uiIdx] = uh;   }
  Void          setPartSizeSubParts   ( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth );

  PredMode*     getPredictionMode     ()                        { return m_pePredMode;        }
  PredMode      getPredictionMode     ( UInt uiIdx )            { return m_pePredMode[uiIdx]; }
  Void          setPredictionMode     ( UInt uiIdx, PredMode uh){ m_pePredMode[uiIdx] = uh;   }
  Void          setPredModeSubParts   ( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth );

  UChar*        getWidth              ()                        { return m_puhWidth;          }
  UChar         getWidth              ( UInt uiIdx )            { return m_puhWidth[uiIdx];   }
  Void          setWidth              ( UInt uiIdx, UChar  uh ) { m_puhWidth[uiIdx] = uh;     }

  UChar*        getHeight             ()                        { return m_puhHeight;         }
  UChar         getHeight             ( UInt uiIdx )            { return m_puhHeight[uiIdx];  }
  Void          setHeight             ( UInt uiIdx, UChar  uh ) { m_puhHeight[uiIdx] = uh;    }

  Void          setSizeSubParts       ( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth );

  UChar*        getQP                 ()                        { return m_phQP;              }
  UChar         getQP                 ( UInt uiIdx )            { return m_phQP[uiIdx];       }
  Void          setQP                 ( UInt uiIdx, UChar  uh ) { m_phQP[uiIdx] = uh;         }
  Void          setQPSubParts         ( UInt uiQP,   UInt uiAbsPartIdx, UInt uiDepth );

  UChar*        getTransformIdx       ()                        { return m_puhTrIdx;          }
  UChar         getTransformIdx       ( UInt uiIdx )            { return m_puhTrIdx[uiIdx];   }
  Void          setTransformIdx       ( UInt uiIdx, UChar  uh ) { m_puhTrIdx[uiIdx] = uh;     }
  Void          setTrIdxSubParts      ( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth );

  TComCUMvField* getCUMvField         ( RefPicList e )          { return  &m_acCUMvField[e];  }

  TCoeff*&      getCoeffY             ()                        { return m_pcTrCoeffY;        }
  TCoeff*&      getCoeffCb            ()                        { return m_pcTrCoeffCb;       }
  TCoeff*&      getCoeffCr            ()                        { return m_pcTrCoeffCr;       }

  UChar         getCbf    ( UInt uiIdx, TextType eType )                  { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx];  }
  UChar*        getCbf    ( TextType eType )                              { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]];         }
  UChar         getCbf    ( UInt uiIdx, TextType eType, UInt uiTrDepth )  { return ( ( getCbf( uiIdx, eType ) >> uiTrDepth ) & 0x1 ); }
  Void          setCbf    ( UInt uiIdx, TextType eType, UChar uh )        { m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx] = uh;    }
  Void          clearCbf  ( UInt uiIdx, TextType eType, UInt uiNumParts );

  Void          setCuCbfLuma          ( UInt uiAbsPartIdx, UInt uiLumaTrMode, UInt uiPartDepth = 0                      );
  Void          setCuCbfChroma        ( UInt uiAbsPartIdx, UInt uiChromaTrMode, UInt uiPartDepth = 0                    );
  Void          setCuCbfChromaUV      ( UInt uiAbsPartIdx, UInt uiChromaTrMode, TextType eTxt,  UInt uiPartDepth = 0    );
  Void          setCbfSubParts        ( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth          );
  Void          setCbfSubParts        ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth                    );

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for coding tool information
  // -------------------------------------------------------------------------------------------------------------------

#if HHI_MRG
  Bool*         getMergeFlag          ()                        { return m_pbMergeFlag;               }
  Bool          getMergeFlag          ( UInt uiIdx )            { return m_pbMergeFlag[uiIdx];        }
  Void          setMergeFlag          ( UInt uiIdx, Bool b )    { m_pbMergeFlag[uiIdx] = b;           }
  Void          setMergeFlagSubParts  ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiDepth );
  UChar*        getMergeIndex         ()                        { return m_puhMergeIndex;                         }
  UChar         getMergeIndex         ( UInt uiIdx )            { return m_puhMergeIndex[uiIdx];                  }
  Void          setMergeIndex         ( UInt uiIdx, UInt uiMergeIndex ) { m_puhMergeIndex[uiIdx] = uiMergeIndex;  }
  Void          setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  UChar*        getLumaIntraDir       ()                        { return m_puhLumaIntraDir;           }
  UChar         getLumaIntraDir       ( UInt uiIdx )            { return m_puhLumaIntraDir[uiIdx];    }
  Void          setLumaIntraDir       ( UInt uiIdx, UChar  uh ) { m_puhLumaIntraDir[uiIdx] = uh;      }
  Void          setLumaIntraDirSubParts( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth );

#if HHI_AIS
  Bool*         getLumaIntraFiltFlag  ()                        { return m_pbLumaIntraFiltFlag;       }
  Bool          getLumaIntraFiltFlag  ( UInt uiIdx )            { return m_pbLumaIntraFiltFlag[uiIdx];}
  Void          setLumaIntraFiltFlag  ( UInt uiIdx, Bool b )    { m_pbLumaIntraFiltFlag[uiIdx] = b;   }
  Void          setLumaIntraFiltFlagSubParts( Bool bFlag, UInt uiAbsPartIdx, UInt uiDepth );
#endif
  UChar*        getChromaIntraDir     ()                        { return m_puhChromaIntraDir;         }
  UChar         getChromaIntraDir     ( UInt uiIdx )            { return m_puhChromaIntraDir[uiIdx];  }
  Void          setChromaIntraDir     ( UInt uiIdx, UChar  uh ) { m_puhChromaIntraDir[uiIdx] = uh;    }
  Void          setChromIntraDirSubParts( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth );

  UChar*        getInterDir           ()                        { return m_puhInterDir;               }
  UChar         getInterDir           ( UInt uiIdx )            { return m_puhInterDir[uiIdx];        }
  Void          setInterDir           ( UInt uiIdx, UChar  uh ) { m_puhInterDir[uiIdx] = uh;          }
  Void          setInterDirSubParts   ( UInt uiDir,  UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );

  UChar*        getROTindex           ()                        { return m_pROTindex;                 }
  UChar         getROTindex           ( UInt uiIdx )            { return m_pROTindex[uiIdx];          }
  Void          setROTindex           ( UInt uiIdx, UChar  uh ) { m_pROTindex[uiIdx] = uh;            }
  Void          setROTindexSubParts   ( UInt ROTindex, UInt uiAbsPartIdx, UInt uiDepth );

  UInt*         getAlfCtrlFlag        ()                        { return m_puiAlfCtrlFlag;            }
  UInt          getAlfCtrlFlag        ( UInt uiIdx )            { return m_puiAlfCtrlFlag[uiIdx];     }
  Void          setAlfCtrlFlag        ( UInt uiIdx, UInt uiFlag){ m_puiAlfCtrlFlag[uiIdx] = uiFlag;   }
  Void          setAlfCtrlFlagSubParts( UInt uiFlag, UInt uiAbsPartIdx, UInt uiDepth );

  Void          createTmpAlfCtrlFlag  ();
  Void          destroyTmpAlfCtrlFlag ();
  Void          copyAlfCtrlFlagToTmp  ();
  Void          copyAlfCtrlFlagFromTmp();

  UChar*        getCIPflag            ()                        { return m_pCIPflag;                  }
  UChar         getCIPflag            ( UInt uiIdx )            { return m_pCIPflag[uiIdx];           }
  Void          setCIPflag            ( UInt uiIdx, UChar  uh ) { m_pCIPflag[uiIdx] = uh;             }
  Void          setCIPflagSubParts    ( UChar CIPflag, UInt uiAbsPartIdx, UInt uiDepth );

#if ANG_INTRA
  Bool          angIntraEnabledPredPart( UInt uiAbsPartIdx );
#endif

#if PLANAR_INTRA
  Int           getPlanarInfo   ( UInt uiIdx, PlanarType pType )            { return m_piPlanarInfo[pType][uiIdx];  }
  Int*          getPlanarInfo   ( PlanarType pType )                        { return m_piPlanarInfo[pType];         }
  Void          setPlanarInfo   ( UInt uiIdx, PlanarType pType, Int i )     { m_piPlanarInfo[pType][uiIdx] = i;     }
  Void          setPlanarInfoSubParts( Int iPlanarFlag, Int iPlanarDeltaY, Int iPlanarDeltaU, Int iPlanarDeltaV, UInt uiAbsPartIdx, UInt uiDepth );
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for accessing partition information
  // -------------------------------------------------------------------------------------------------------------------

  Void          getPartIndexAndSize   ( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight );
  UChar         getNumPartInter       ();
  Bool          isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth);

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for motion vector
  // -------------------------------------------------------------------------------------------------------------------

  Void          getMvField            ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList, TComMvField& rcMvField );
  
#if HHI_IMVP
  Void          getYThresXPredLists   ( UInt uiPartIdx, RefPicList  eRefPicList, Int iRefIdx, Int& riYPred, std::vector<Int>& racYThresList, std::vector<Int>& racXPredList );
  Void          getMvPredCandLstStd   ( RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, std::vector<TComMv>& rcMvPredCandLst );
  Void          getMvPredCandLstExt   ( RefPicList eRefPicList,  UInt uiPartIdx, Int iRefIdx, std::vector<TComMv>& rcMvPredCandLst );
  Void          setYThresXPredLists   ( std::vector<TComMv>& rcMvPredCandLst, std::vector<Int>& rcYThresList, std::vector<Int>& rcXPredList );
  Int           getMvPredYCompInd     ( std::vector<TComMv>& rcMvPredCandLst );
  Bool          insertNeighbourMvs    ( RefPicList eRefPicList, Int iRefIdx, TComDataCU* pcCUNeighbour, UInt uiIdxNeighbour, std::vector <TComMv>& racNeighbourMvs, Bool bAvoidMultipleInsertion );
  Void          getMvPredYInd         (  RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, Int& riMvPredY );
  Void          getMvPredXDep         ( RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, const Int iMvY, Int& riMvPredX );
  Int           getMvPredXCompDep     ( const std::vector<Int>& rcYThresLst, const std::vector<Int>& rcXPredLst, const Int iYComponent );
  Void          insertCollocated      ( RefPicList eRefPicList, Int uiPartIdx, Int iRefIdx, std::vector <TComMv>& racNeighbourMvs);
#endif

  AMVP_MODE     getAMVPMode           ( UInt uiIdx );
  Void          fillMvpCand           ( UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, AMVPInfo* pInfo );
#ifdef QC_AMVRES
#if HHI_IMVP
  Bool          clearMVPCand_one_fourth        ( TComMv cMvd, AMVPInfo* pInfo, RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx );
#else
  Bool          clearMVPCand_one_fourth        ( TComMv cMvd, AMVPInfo* pInfo);
#endif
  Int           searchMVPIdx_one_fourth(TComMv cMv, AMVPInfo* pInfo);
#endif
#if HHI_IMVP
  Bool          clearMVPCand          ( TComMv cMvd, AMVPInfo* pInfo, RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx );
#else
  Bool          clearMVPCand          ( TComMv cMvd, AMVPInfo* pInfo );
#endif
  Int           searchMVPIdx          ( TComMv cMv,  AMVPInfo* pInfo );

  Void          setMVPIdx             ( RefPicList eRefPicList, UInt uiIdx, Int iMVPIdx)  { m_apiMVPIdx[eRefPicList][uiIdx] = iMVPIdx;  }
  Int           getMVPIdx             ( RefPicList eRefPicList, UInt uiIdx)               { return m_apiMVPIdx[eRefPicList][uiIdx];     }
  Int*          getMVPIdx             ( RefPicList eRefPicList )                          { return m_apiMVPIdx[eRefPicList];            }

  Void          setMVPNum             ( RefPicList eRefPicList, UInt uiIdx, Int iMVPNum ) { m_apiMVPNum[eRefPicList][uiIdx] = iMVPNum;  }
  Int           getMVPNum             ( RefPicList eRefPicList, UInt uiIdx )              { return m_apiMVPNum[eRefPicList][uiIdx];     }
  Int*          getMVPNum             ( RefPicList eRefPicList )                          { return m_apiMVPNum[eRefPicList];            }

  Void          setMVPIdxSubParts     ( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
  Void          setMVPNumSubParts     ( Int iMVPNum, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );

  Void          clipMv                ( TComMv&     rcMv     );
  Void          getMvPredLeft         ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldA.getMv(); }
  Void          getMvPredAbove        ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldB.getMv(); }
  Void          getMvPredAboveRight   ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldC.getMv(); }

  // -------------------------------------------------------------------------------------------------------------------
  // utility functions for neighbouring information
  // -------------------------------------------------------------------------------------------------------------------

  TComDataCU*   getCULeft                   () { return m_pcCULeft;       }
  TComDataCU*   getCUAbove                  () { return m_pcCUAbove;      }
  TComDataCU*   getCUAboveLeft              () { return m_pcCUAboveLeft;  }
  TComDataCU*   getCUAboveRight             () { return m_pcCUAboveRight; }
  TComDataCU*   getCUColocated              ( RefPicList eRefPicList ) { return m_apcCUColocated[eRefPicList]; }

  TComDataCU*   getPULeft                   ( UInt&  uiLPartUnitIdx , UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAbove                  ( UInt&  uiAPartUnitIdx , UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAboveLeft              ( UInt&  uiALPartUnitIdx, UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAboveRight             ( UInt&  uiARPartUnitIdx, UInt uiCurrPartUnitIdx );

  TComDataCU*   getPUBelowLeft              ( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUAboveRightAdi          ( UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx );
  TComDataCU*   getPUBelowLeftAdi           ( UInt& uiBLPartUnitIdx, UInt uiPuHeight, UInt uiCurrPartUnitIdx );

  Void          deriveLeftRightTopIdx       ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT );
  Void          deriveLeftBottomIdx         ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLB );

  Void          deriveLeftRightTopIdxAdi    ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth );
  Void          deriveLeftBottomIdxAdi      ( UInt& ruiPartIdxLB, UInt  uiPartOffset, UInt uiPartDepth );

#if HHI_MRG
  Void          getInterMergeCandidates     ( UInt uiAbsPartIdx, TComMvField cMFieldNeighbours[4], UChar uhInterDirNeighbours[2], UInt& uiNeighbourInfos );
#endif


  // -------------------------------------------------------------------------------------------------------------------
  // member functions for modes
  // -------------------------------------------------------------------------------------------------------------------

  Bool          isIntra   ( UInt uiPartIdx )  { return m_pePredMode[ uiPartIdx ] == MODE_INTRA; }
  Bool          isSkip    ( UInt uiPartIdx )  { return m_pePredMode[ uiPartIdx ] == MODE_SKIP;  } ///< SKIP+DIRECT
  Bool          isSkipped ( UInt uiPartIdx );                                                     ///< SKIP (no residual)

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for symbol prediction (most probable / mode conversion)
  // -------------------------------------------------------------------------------------------------------------------

  Int           getMostProbableIntraDirLuma     ( UInt uiAbsPartIdx                                       );
  Int           convertIntraDirLuma             ( UInt uiAbsPartIdx                                       );
  UInt          revertIntraDirLuma              ( UInt uiAbsPartIdx, Int  iIntraDirLuma                   );

  Int           getMostProbableIntraDirLumaAdi  ( UInt uiAbsPartIdx                                       );
  Int           convertIntraDirLumaAdi          ( TComDataCU* pcCU, UInt uiAbsPartIdx                     );
  UInt          revertIntraDirLumaAdi           ( TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirLuma  );

  PartSize      getMostProbablePartSize         ( UInt uiAbsPartIdx                                       );

  Int           getMostProbablePredMode         ( UInt uiAbsPartIdx                                       );
  Int           convertPredMode                 ( UInt uiAbsPartIdx                                       );
  Int           revertPredMode                  ( UInt uiAbsPartIdx, Int   iPredMode                      );

  UInt          getIntraSizeIdx                 ( UInt uiAbsPartIdx                                       );
  Void          convertTransIdx                 ( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode );

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for SBAC context
  // -------------------------------------------------------------------------------------------------------------------

  UInt          getCtxSplitFlag                 ( UInt   uiAbsPartIdx, UInt uiDepth                   );
  UInt          getCtxCbf                       ( UInt   uiAbsPartIdx, TextType eType, UInt uiTrDepth );
#if HHI_RQT
  UInt          getCtxQtCbf                     ( UInt   uiAbsPartIdx, TextType eType, UInt uiTrDepth );
#endif
  UInt          getCtxRefIdx                    ( UInt   uiAbsPartIdx, RefPicList eRefPicList         );
  UInt          getCtxSkipFlag                  ( UInt   uiAbsPartIdx                                 );
#if HHI_MRG
  UInt          getCtxMergeFlag                 ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxMergeIndex                ( UInt   uiAbsPartIdx                                 );
#endif
  UInt          getCtxAlfCtrlFlag               ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxPredMode                  ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxInterDir                  ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxTransIdx                  ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxIntraDirChroma            ( UInt   uiAbsPartIdx                                 );
  UInt          getCtxCIPFlag                   ( UInt   uiAbsPartIdx                                 );
#if HHI_AIS
#if ANG_INTRA
  UInt          getCtxIntraFiltFlagLumaAng      ( UInt   uiAbsPartIdx                                 );
#endif
  UInt          getCtxIntraFiltFlagLuma         ( UInt   uiAbsPartIdx                                 );
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // member functions for RD cost storage
  // -------------------------------------------------------------------------------------------------------------------

  Double&       getTotalCost()                  { return m_dTotalCost;        }
  UInt&         getTotalDistortion()            { return m_uiTotalDistortion; }
  UInt&         getTotalBits()                  { return m_uiTotalBits;       }
  UInt&         getTotalNumPart()               { return m_uiNumPartition;    }
};

#endif

