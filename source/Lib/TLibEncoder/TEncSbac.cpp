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

/** \file     TEncSbac.cpp
    \brief    SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"

#include <map>

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSbac::TEncSbac()
// new structure here
: m_pcBitIf                   ( NULL )
, m_pcSlice                   ( NULL )
, m_pcBinIf                   ( NULL )
, m_bAlfCtrl                  ( false )
, m_uiCoeffCost               ( 0 )
, m_uiMaxAlfCtrlDepth         ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             )
, m_cCUMergeFlagExtSCModel    ( 1,             1,               NUM_MERGE_FLAG_EXT_CTX        )
, m_cCUMergeIdxExtSCModel     ( 1,             1,               NUM_MERGE_IDX_EXT_CTX         )
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             )
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             )
, m_cCUAlfCtrlFlagSCModel     ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         )
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   )
#if ADD_PLANAR_MODE
, m_cPlanarFlagSCModel        ( 1,             1,               NUM_PLANARFLAG_CTX            )
#endif
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           )
, m_cCUDeltaQpSCModel         ( 1,             1,               NUM_DELTA_QP_CTX              )
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             )
, m_cCURefPicSCModel          ( 1,             1,               NUM_REF_NO_CTX                )
, m_cCUMvdSCModel             ( 1,             2,               NUM_MV_RES_CTX                )
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                )
, m_cCUTransSubdivFlagSCModel ( 1,             1,               NUM_TRANS_SUBDIV_FLAG_CTX     )
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           )
, m_cCUSigSCModel             ( 1,             2,               NUM_SIG_FLAG_CTX              )
, m_cCuCtxLastX               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          )
, m_cCuCtxLastY               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          )
, m_cCUOneSCModel             ( 1,             2,               NUM_ONE_FLAG_CTX              )
, m_cCUAbsSCModel             ( 1,             2,               NUM_ABS_FLAG_CTX              )
, m_cMVPIdxSCModel            ( 1,             1,               NUM_MVP_IDX_CTX               )
, m_cALFFlagSCModel           ( 1,             1,               NUM_ALF_FLAG_CTX              )
, m_cALFUvlcSCModel           ( 1,             1,               NUM_ALF_UVLC_CTX              )
, m_cALFSvlcSCModel           ( 1,             1,               NUM_ALF_SVLC_CTX              )
#if MTK_SAO
, m_cAOFlagSCModel            ( 1,             1,               NUM_AO_FLAG_CTX              )
, m_cAOUvlcSCModel            ( 1,             1,               NUM_AO_UVLC_CTX              )
, m_cAOSvlcSCModel            ( 1,             1,               NUM_AO_SVLC_CTX              )
#endif
{
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
  m_iSliceGranularity = 0;
#endif
}

TEncSbac::~TEncSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSbac::resetEntropy           ()
{
  Int  iQp              = m_pcSlice->getSliceQp();
  SliceType eSliceType  = m_pcSlice->getSliceType();
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_ALF_CTRL_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT);
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT);
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PART_SIZE );
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_INTRA_PRED_MODE );
#if ADD_PLANAR_MODE
  m_cPlanarFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PLANARFLAG );
#endif
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_REF_PIC );
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_DQP );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_QT_ROOT_CBF );
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_SIG_FLAG );
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST_X );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST_Y );
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ABS_FLAG );
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
#if MTK_SAO
  m_cAOFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_FLAG );
  m_cAOUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_UVLC );
  m_cAOSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_SVLC );
#endif
  // new structure
  m_uiLastQp = iQp;
  
  m_pcBinIf->start();
  
  return;
}

void TEncSbac::codeSEI(const SEI&)
{
  assert(0);
}

Void TEncSbac::codeSPS( TComSPS* pcSPS )
{
  assert (0);
  return;
}

Void TEncSbac::codePPS( TComPPS* pcPPS )
{
  assert (0);
  return;
}

Void TEncSbac::codeSliceHeader( TComSlice* pcSlice )
{
  assert (0);
  return;
}

Void TEncSbac::codeTerminatingBit( UInt uilsLast )
{
  m_pcBinIf->encodeBinTrm( uilsLast );
}

Void TEncSbac::codeSliceFinish()
{
  m_pcBinIf->finish();
}



Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[0] );
  
  if( 0 == uiSymbol)
  {
    return;
  }
  
  while( uiSymbol-- )
  {
    m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }
  
  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );
  
  if ( uiSymbol == 0 )
  {
    return;
  }
  
  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );
  
  while( --uiSymbol )
  {
    m_pcBinIf->encodeBin( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    m_pcBinIf->encodeBin( 0, pcSCModel[ iOffset ] );
  }
  
  return;
}

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    m_pcBinIf->encodeBinEP( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  m_pcBinIf->encodeBinEP( 0 );
  while( uiCount-- )
  {
    m_pcBinIf->encodeBinEP( (uiSymbol>>uiCount) & 1 );
  }
  
  return;
}

/** Coding of coeff_abs_level_minus3
 * \param uiSymbol value of coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
Void TEncSbac::xWriteGoRiceExGolomb( UInt uiSymbol, UInt &ruiGoRiceParam )
{
  UInt uiCount      = 0;
  UInt uiMaxVlc     = g_auiGoRiceRange[ ruiGoRiceParam ];
  Bool bExGolomb    = ( uiSymbol > uiMaxVlc );
  UInt uiCodeWord   = min<UInt>( uiSymbol, ( uiMaxVlc + 1 ) );
  UInt uiQuotient   = uiCodeWord >> ruiGoRiceParam;
  UInt uiUnaryPart  = uiQuotient;
  UInt uiRemainder  = 1;
  UInt uiMaxPreLen  = g_auiGoRicePrefixLen[ ruiGoRiceParam ];

  if( uiUnaryPart )
  {
    m_pcBinIf->encodeBinEP( 1 );
    uiCount++;

    while( --uiUnaryPart && uiCount < uiMaxPreLen )
    {
      m_pcBinIf->encodeBinEP( 1 );
      uiCount++;
    }

    if( uiCount < uiMaxPreLen )
    {
      m_pcBinIf->encodeBinEP( uiUnaryPart ? 1 : 0 );
    }
  }
  else
  {
    m_pcBinIf->encodeBinEP( 0 );
  }

  for( UInt ui = 0; ui < ruiGoRiceParam; ui++ )
  {
    m_pcBinIf->encodeBinEP( ( uiRemainder & uiCodeWord ) ? 1 : 0 );
    uiRemainder = uiRemainder << 1;
  }

  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( uiSymbol, 15 ) ];

  if( bExGolomb )
  {
    uiSymbol -= uiMaxVlc + 1;
    xWriteEpExGolomb( uiSymbol, 0 );
  }

  return;
}

// SBAC RD
Void  TEncSbac::load ( TEncSbac* pSrc)
{
  this->xCopyFrom(pSrc);
}

Void  TEncSbac::loadIntraDirModeLuma( TEncSbac* pSrc)
{
  m_pcBinIf->copyState( pSrc->m_pcBinIf );
  
  this->m_cCUIntraPredSCModel      .copyFrom( &pSrc->m_cCUIntraPredSCModel       );
#if ADD_PLANAR_MODE
  this->m_cPlanarFlagSCModel       .copyFrom( &pSrc->m_cPlanarFlagSCModel        );
#endif
}


Void  TEncSbac::store( TEncSbac* pDest)
{
  pDest->xCopyFrom( this );
}


Void TEncSbac::xCopyFrom( TEncSbac* pSrc )
{
  m_pcBinIf->copyState( pSrc->m_pcBinIf );
  
  this->m_uiCoeffCost = pSrc->m_uiCoeffCost;
  this->m_uiLastQp    = pSrc->m_uiLastQp;
  
  this->m_cCUSplitFlagSCModel      .copyFrom( &pSrc->m_cCUSplitFlagSCModel       );
  this->m_cCUSkipFlagSCModel       .copyFrom( &pSrc->m_cCUSkipFlagSCModel        );
  this->m_cCUMergeFlagExtSCModel  .copyFrom( &pSrc->m_cCUMergeFlagExtSCModel);
  this->m_cCUMergeIdxExtSCModel   .copyFrom( &pSrc->m_cCUMergeIdxExtSCModel);
  this->m_cCUPartSizeSCModel       .copyFrom( &pSrc->m_cCUPartSizeSCModel        );
  this->m_cCUPredModeSCModel       .copyFrom( &pSrc->m_cCUPredModeSCModel        );
  this->m_cCUIntraPredSCModel      .copyFrom( &pSrc->m_cCUIntraPredSCModel       );
#if ADD_PLANAR_MODE
  this->m_cPlanarFlagSCModel       .copyFrom( &pSrc->m_cPlanarFlagSCModel        );
#endif
  this->m_cCUChromaPredSCModel     .copyFrom( &pSrc->m_cCUChromaPredSCModel      );
  this->m_cCUDeltaQpSCModel        .copyFrom( &pSrc->m_cCUDeltaQpSCModel         );
  this->m_cCUInterDirSCModel       .copyFrom( &pSrc->m_cCUInterDirSCModel        );
  this->m_cCURefPicSCModel         .copyFrom( &pSrc->m_cCURefPicSCModel          );
  this->m_cCUMvdSCModel            .copyFrom( &pSrc->m_cCUMvdSCModel             );
  this->m_cCUQtCbfSCModel          .copyFrom( &pSrc->m_cCUQtCbfSCModel           );
  this->m_cCUTransSubdivFlagSCModel.copyFrom( &pSrc->m_cCUTransSubdivFlagSCModel );
  this->m_cCUQtRootCbfSCModel      .copyFrom( &pSrc->m_cCUQtRootCbfSCModel       );
  this->m_cCUSigSCModel            .copyFrom( &pSrc->m_cCUSigSCModel             );
  this->m_cCuCtxLastX              .copyFrom( &pSrc->m_cCuCtxLastX               );
  this->m_cCuCtxLastY              .copyFrom( &pSrc->m_cCuCtxLastY               );
  this->m_cCUOneSCModel            .copyFrom( &pSrc->m_cCUOneSCModel             );
  this->m_cCUAbsSCModel            .copyFrom( &pSrc->m_cCUAbsSCModel             );
  this->m_cMVPIdxSCModel           .copyFrom( &pSrc->m_cMVPIdxSCModel            );
#if MTK_SAO
  this->m_cAOFlagSCModel           .copyFrom( &pSrc->m_cAOFlagSCModel            );
  this->m_cAOUvlcSCModel           .copyFrom( &pSrc->m_cAOUvlcSCModel            );
  this->m_cAOSvlcSCModel           .copyFrom( &pSrc->m_cAOSvlcSCModel            );
#endif

}

Void TEncSbac::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
#if MRG_AMVP_FIXED_IDX_F470
  Int iNum = AMVP_MAX_NUM_CANDS;
#else
  Int iNum    = pcCU->getMVPNum(eRefList, uiAbsPartIdx);
#endif

  xWriteUnaryMaxSymbol(iSymbol, m_cMVPIdxSCModel.get(0), 1, iNum-1);
}

Void TEncSbac::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );
  
  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
    }
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinIf->encodeBin( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 3) );
      }
    }
    else
    {
#endif
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 3) );
        m_pcBinIf->encodeBin( (eSize == SIZE_2Nx2N? 0 : 1), m_cCUPartSizeSCModel.get( 0, 0, 4) );
      }
#if DISABLE_4x4_INTER
    }
#endif
    return;
  }
  
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      m_pcBinIf->encodeBin( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
    }
    return;
  }
  
  switch(eSize)
  {
    case SIZE_2Nx2N:
    {
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      break;
    }
    case SIZE_2NxN:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      
      break;
    }
    case SIZE_Nx2N:
    {
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      if (pcCU->getSlice()->isInterB())
      {
        m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
      }
    }
    else
    {
#endif
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
#if DISABLE_4x4_INTER
    }
#endif      
      break;
    }
    case SIZE_NxN:
    {
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && (pcCU->getWidth(uiAbsPartIdx)==8) && (pcCU->getHeight(uiAbsPartIdx)==8) )
    {
      assert(0);
      break;
    }
    else
    {
#endif
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
        {
          m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
          m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
        }
        
        if (pcCU->getSlice()->isInterB())
        {
          m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 3) );
        }
      }
      break;
#if DISABLE_4x4_INTER
    }
#endif
    }
    default:
    {
      assert(0);
    }
  }
}

/** code prediction mode
 * \param pcCU
 * \param uiAbsPartIdx  
 * \returns Void
 */
Void TEncSbac::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );
  if (pcCU->getSlice()->isInterB() )
  {
    return;
  }

  m_pcBinIf->encodeBin( iPredMode == MODE_INTER ? 0 : 1, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
}

Void TEncSbac::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
    return;
  
  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }
  
  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;
  
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx) ) );
}

Void TEncSbac::codeAlfCtrlDepth()
{
  if (!m_bAlfCtrl)
    return;
  
  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  xWriteUnaryMaxSymbol(uiDepth, m_cALFUvlcSCModel.get(0), 1, g_uiMaxCUDepth-1);
}

/** code skip flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  UInt uiCtxSkip = pcCU->getCtxSkipFlag( uiAbsPartIdx ) ;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tSkipFlag" );
  DTRACE_CABAC_T( "\tuiCtxSkip: ");
  DTRACE_CABAC_V( uiCtxSkip );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}

/** code merge flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCtx = 0;
#if CHANGE_MERGE_CONTEXT
  uiCtx = pcCU->getCtxMergeFlag( uiAbsPartIdx );
#else
  for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
  {
    if( pcCU->getNeighbourCandIdx( uiIter, uiAbsPartIdx ) == uiIter + 1 )
    {
      if( uiIter == 0 )
      {
        uiCtx++;
      }
      else if( uiIter == 1 )
      {
        uiCtx++;
      }
    }
  }
#endif
  UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeFlagExtSCModel.get( 0, 0, uiCtx ) );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tMergeFlag: " );
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\tAddress: " );
  DTRACE_CABAC_V( pcCU->getAddr() );
  DTRACE_CABAC_T( "\tuiAbsPartIdx: " );
  DTRACE_CABAC_V( uiAbsPartIdx );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    DTRACE_CABAC_T( "\tNumMrgCand: " );
    DTRACE_CABAC_V( ui );
    DTRACE_CABAC_T( "\t==\t" );
    DTRACE_CABAC_V( UInt( pcCU->getNeighbourCandIdx( ui, uiAbsPartIdx ) ) );
  }
  DTRACE_CABAC_T( "\n" );
}

/** code merge index
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if MRG_AMVP_FIXED_IDX_F470
  UInt uiNumCand = MRG_MAX_NUM_CANDS;
  Bool bLeftInvolved = true;
  Bool bAboveInvolved = true;
  Bool bCollocatedInvolved = true;
#else
  Bool bLeftInvolved = false;
  Bool bAboveInvolved = false;
  Bool bCollocatedInvolved = false;
  Bool bCornerInvolved = false;
  UInt uiNumCand = 0;
  for( UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; ++uiIter )
  {
    if( pcCU->getNeighbourCandIdx( uiIter, uiAbsPartIdx ) == uiIter + 1 )
    {
      uiNumCand++;
      if( uiIter == 0 )
      {
        bLeftInvolved = true;
      }
      else if( uiIter == 1 )
      {
        bAboveInvolved = true;
      }
      else if( uiIter == 2 )
      {
        bCollocatedInvolved = true;
      }
      else if( uiIter == 3 )
      {
        bCornerInvolved = true;
      }
    }
  }
#endif
  assert( uiNumCand > 1 );

  UInt auiCtx[4] = { 0, 0, 0, 3 };
  if( bLeftInvolved && bAboveInvolved )
  {
    auiCtx[0] = 0;
  }
  else if( bLeftInvolved || bAboveInvolved )
  {
    auiCtx[0] = bCollocatedInvolved ? 1 : 2;
  }
  else
  {
    auiCtx[0] = bCollocatedInvolved ? 2 : 3;
  }

  if( uiNumCand >= 3 )
  {
    if( bAboveInvolved )
  {
      auiCtx[1] = bCollocatedInvolved ? 1 : 2;
    }
    else
    {
      auiCtx[1] = bCollocatedInvolved ? 2 : 3;
    }
  }

  if( uiNumCand >= 4 )
  {
    auiCtx[2] =  bCollocatedInvolved ? 2 : 3;
  }

  UInt uiUnaryIdx = pcCU->getMergeIndex( uiAbsPartIdx );
#if !(MRG_AMVP_FIXED_IDX_F470)
  if( !bCornerInvolved && uiUnaryIdx > 3 )
  {
    --uiUnaryIdx;
  }
  if( !bCollocatedInvolved && uiUnaryIdx > 2 )
  {
    --uiUnaryIdx;
  }
  if( !bAboveInvolved && uiUnaryIdx > 1 )
  {
    --uiUnaryIdx;
  }
  if( !bLeftInvolved && uiUnaryIdx > 0 )
  {
    --uiUnaryIdx;
  }
#endif
  for( UInt ui = 0; ui < uiNumCand - 1; ++ui )
  {
    const UInt uiSymbol = ui == uiUnaryIdx ? 0 : 1;
    m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[ui] ) );
    if( uiSymbol == 0 )
    {
      break;
    }
  }
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tparseMergeIndex()" );
  DTRACE_CABAC_T( "\tuiMRGIdx= " );
  DTRACE_CABAC_V( pcCU->getMergeIndex( uiAbsPartIdx ) );
  DTRACE_CABAC_T( "\tuiNumCand= " );
  DTRACE_CABAC_V( uiNumCand );
  DTRACE_CABAC_T( "\tbLeftInvolved= " );
  DTRACE_CABAC_V( bLeftInvolved );
  DTRACE_CABAC_T( "\tbAboveInvolved= " );
  DTRACE_CABAC_V( bAboveInvolved );
  DTRACE_CABAC_T( "\tbCollocatedInvolved= " );
  DTRACE_CABAC_V( bCollocatedInvolved );
  DTRACE_CABAC_T( "\tbCornerRTInvolved= " );
  DTRACE_CABAC_V( bCornerInvolved );
  DTRACE_CABAC_T( "\n" );
}

Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;
  
  UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
  
  assert( uiCtx < 3 );
  m_pcBinIf->encodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  return;
}

Void TEncSbac::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcBinIf->encodeBin( uiSymbol, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\n" )
}
#if MTK_DCM_MPM
Void TEncSbac::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
#if ADD_PLANAR_MODE
  UInt planarFlag    = 0;
  if (uiDir == PLANAR_IDX)
  {
    uiDir = 2;
    planarFlag = 1;
  }
#endif
  
  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  

  Int uiPredIdx = -1;

  for(UInt i = 0; i < uiPredNum; i++)
  {
    if(uiDir == uiPreds[i])
    {
      uiPredIdx = i;
    }
  }
 
  if(uiPredIdx != -1)
  {
    m_pcBinIf->encodeBin( 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
    if(uiPredNum == 2)
    {
      m_pcBinIf->encodeBin( uiPredIdx, m_cCUIntraPredSCModel.get( 0, 0, 2 ) );
    }

  }
  else
  {
    m_pcBinIf->encodeBin( 0, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
  
    for(Int i = (uiPredNum - 1); i >= 0; i--)
    {
      uiDir = uiDir > uiPreds[i] ? uiDir - 1 : uiDir;
    }

    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcBinIf->encodeBin((uiDir & 0x01), m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));}
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));}
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) { m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));}
    }
    else
    {
      if (uiDir < 31){ // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
        m_pcBinIf->encodeBin((uiDir & 0x01),      m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x10) >> 4, m_cCUIntraPredSCModel.get(0, 0, 1));
      }
      else
      {
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir == 32) ? 1 : 0, m_cCUIntraPredSCModel.get(0, 0, 1));
       }
     }
   }
#if ADD_PLANAR_MODE
  uiDir = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if ( (uiDir == PLANAR_IDX) || (uiDir == 2) )
  {
    m_pcBinIf->encodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,0) );
  }
#endif
  return;
}
#else
Void TEncSbac::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );
#if ADD_PLANAR_MODE
  UInt planarFlag    = 0;
  if (uiDir == PLANAR_IDX)
  {
    uiDir = 2;
    planarFlag = 1;
  }
#endif
  
  if (uiDir == iMostProbable)
    m_pcBinIf->encodeBin( 1, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
  else
  {
    m_pcBinIf->encodeBin( 0, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
    uiDir = uiDir > iMostProbable ? uiDir - 1 : uiDir;
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcBinIf->encodeBin((uiDir & 0x01), m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));
    }
    else
    {
      if (uiDir < 31)
      { // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
        m_pcBinIf->encodeBin((uiDir & 0x01),      m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x02) >> 1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x04) >> 2, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x08) >> 3, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir & 0x10) >> 4, m_cCUIntraPredSCModel.get(0, 0, 1));
      }
      else
      {
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin(1, m_cCUIntraPredSCModel.get(0, 0, 1));
        m_pcBinIf->encodeBin((uiDir == 32) ? 1 : 0, m_cCUIntraPredSCModel.get(0, 0, 1));
      }
    }
  }
  
#if ADD_PLANAR_MODE
  uiDir = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if ( (uiDir == PLANAR_IDX) || (uiDir == 2) )
  {
    m_pcBinIf->encodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,0) );
  }
#endif
  return;
}
#endif
Void TEncSbac::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCtx            = pcCU->getCtxIntraDirChroma( uiAbsPartIdx );
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );
  
#if ADD_PLANAR_MODE
  UInt planarFlag       = 0;
  if (uiIntraDirChroma == PLANAR_IDX)
  {
    uiIntraDirChroma = 2;
    planarFlag = 1;
  }
#endif
 
  UInt uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
#if ADD_PLANAR_MODE
  if ( (uiMode == 2 ) || (uiMode == PLANAR_IDX) )
  {
    uiMode = 4;
  }
#endif

#if LM_CHROMA

  Bool bUseLMFlag = pcCU->getSlice()->getSPS()->getUseLMChroma();

  Int  iMaxMode = bUseLMFlag ? 3 : 4;

  Int  iMax = uiMode < iMaxMode ? 2 : 3; 
  
  //switch codeword
  if (uiIntraDirChroma == 4) 
  {
    uiIntraDirChroma = 0;
  } 
  else if (uiIntraDirChroma == LM_CHROMA_IDX )
  {
    uiIntraDirChroma = 1;
  }
  else
  {
    if (uiIntraDirChroma < uiMode)
      uiIntraDirChroma++;

    if (bUseLMFlag)
      uiIntraDirChroma++;

#if CHROMA_CODEWORD_SWITCH
    uiIntraDirChroma = ChromaMapping[iMax-2][uiIntraDirChroma];
#endif
  }


#else // <-- LM_CHROMA
  Int  iMax = uiMode < 4 ? 2 : 3; 
  
  //switch codeword
  if (uiIntraDirChroma == 4)
  {
    uiIntraDirChroma = 0;
  } 
  else
  {
    if (uiIntraDirChroma < uiMode)
    {
      uiIntraDirChroma++;
    }
#if CHROMA_CODEWORD_SWITCH
    uiIntraDirChroma = ChromaMapping[iMax-2][uiIntraDirChroma];
#endif
  }
#endif // <-- LM_CHROMA
  
  if ( 0 == uiIntraDirChroma )
  {
    m_pcBinIf->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, uiCtx ) );
    xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, iMax );
  }

#if ADD_PLANAR_MODE
  uiIntraDirChroma = pcCU->getChromaIntraDir( uiAbsPartIdx );
  uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
  mapPlanartoDC( uiIntraDirChroma );
  mapPlanartoDC( uiMode );
  if ( (uiIntraDirChroma == 2) && (uiMode != 2) )
  {
    m_pcBinIf->encodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,1) );
  }
#endif
  return;
}

Void TEncSbac::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  UInt uiCtx      = pcCU->getCtxInterDir( uiAbsPartIdx );
  uiInterDir--;
  m_pcBinIf->encodeBin( ( uiInterDir == 2 ? 1 : 0 ), m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );
  
  if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    assert( uiInterDir != 1 );
    return;
  }
  
  if ( uiInterDir < 2 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 0)
  {
    m_pcBinIf->encodeBin( uiInterDir, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }
  
  return;
}

Void TEncSbac::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getInterDir( uiAbsPartIdx ) != 3)
  {
    Int iRefFrame = pcCU->getSlice()->getRefIdxOfLC(eRefList, pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx ));
    UInt uiCtx;

    uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, RefPicList(pcCU->getSlice()->getListIdFromIdxOfLC(0)) );

    m_pcBinIf->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), m_cCURefPicSCModel.get( 0, 0, uiCtx ) );

    if ( iRefFrame > 0 )
    {
      xWriteUnaryMaxSymbol( iRefFrame - 1, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );
    }
  }
  else
  {
    Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
    
    UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );
    
    m_pcBinIf->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), m_cCURefPicSCModel.get( 0, 0, uiCtx ) );
    
    if ( iRefFrame > 0 )
    {
      xWriteUnaryMaxSymbol( iRefFrame - 1, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
    }
  }
  return;
}

Void TEncSbac::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
  
  UInt uiAbsPartIdxL, uiAbsPartIdxA;
#if MVD_CTX
  Int iHorPredL, iVerPredL;
  Int iHorPredA, iVerPredA;
#else
  Int iHorPred, iVerPred;
#endif

  TComDataCU* pcCUL   = pcCU->getPULeft ( uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
#if REDUCE_UPPER_MOTION_DATA
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx, true, true, true );
#else
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
#endif
  
  TComCUMvField* pcCUMvFieldL = ( pcCUL == NULL || pcCUL->isIntra( uiAbsPartIdxL ) ) ? NULL : pcCUL->getCUMvField( eRefList );
  TComCUMvField* pcCUMvFieldA = ( pcCUA == NULL || pcCUA->isIntra( uiAbsPartIdxA ) ) ? NULL : pcCUA->getCUMvField( eRefList );
  
#if MVD_CTX
  iHorPredA = ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPredA = ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );
  iHorPredL = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() );
  iVerPredL = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() );

  xWriteMvd( iHor, iHorPredL, iHorPredA, 0 );
  xWriteMvd( iVer, iVerPredL, iVerPredA, 1 );
#else  
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );

  xWriteMvd( iHor, iHorPred, 0 );
  xWriteMvd( iVer, iVerPred, 1 );
#endif

  
  return;
}

Void TEncSbac::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if SUB_LCU_DQP
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getRefQP( uiAbsPartIdx );
#else
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getSlice()->getSliceQp();
#endif
  
  if ( iDQp == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
    
    UInt uiDQp = (UInt)( iDQp > 0 ? ( 2 * iDQp - 2 ) : ( -2 * iDQp - 1 ) );
    xWriteUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
  }
  
  return;
}

Void TEncSbac::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf     ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1 : eType, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( eType )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}

#if E057_INTRA_PCM
/** Code I_PCM information. 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, code its PCM alignment bits and codes.  
 */
Void TEncSbac::codeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx)
{
  UInt uiIPCM = (pcCU->getIPCMFlag(uiAbsPartIdx) == true)? 1 : 0;

  m_pcBinIf->encodeBinTrm (uiIPCM);

  if (uiIPCM)
  {
    m_pcBinIf->encodePCMAlignBits();

    UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
    UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
    UInt uiChromaOffset = uiLumaOffset>>2;
    Pel* piPCMSample;
    UInt uiWidth;
    UInt uiHeight;
    UInt uiSampleBits;
    UInt uiX, uiY;

    piPCMSample = pcCU->getPCMSampleY() + uiLumaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx);
    uiHeight = pcCU->getHeight(uiAbsPartIdx);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        m_pcBinIf->xWritePCMCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCb() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        m_pcBinIf->xWritePCMCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCr() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        m_pcBinIf->xWritePCMCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }
    m_pcBinIf->resetBac();
  }
}
#endif

Void TEncSbac::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  UInt uiCtx = pcCU->getCtxQtRootCbf( uiAbsPartIdx );
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}

/** Encode (X,Y) position of the last significant coefficient
 * \param uiPosX X component of last coefficient
 * \param uiPosY Y component of last coefficient
 * \param uiWidth block width
 * \param eTType plane type / luminance or chrominance
 * \param uiCTXIdx block size context
 * \param uiScanIdx scan type (zig-zag, hor, ver)
 * \returns Void
 * This method encodes the X and Y component within a block of the last significant coefficient.
 */
__inline Void TEncSbac::codeLastSignificantXY( UInt uiPosX, UInt uiPosY, const UInt uiWidth, const TextType eTType, const UInt uiCTXIdx, const UInt uiScanIdx )
{  
  UInt  uiCtxLast;
  const UInt uiCtxOffset = g_uiCtxXYOffset[uiCTXIdx];

#if QC_MDCS
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosX, uiPosY );
  }
#endif
  
  for(uiCtxLast=0; uiCtxLast<uiPosX; uiCtxLast++)
  {
    m_pcBinIf->encodeBin( 0, m_cCuCtxLastX.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtxLast] ) );
  }
  if(uiPosX < uiWidth - 1)
  {
    m_pcBinIf->encodeBin( 1, m_cCuCtxLastX.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtxLast] ) );
  }

  for(uiCtxLast=0; uiCtxLast<uiPosY; uiCtxLast++)
  {
    m_pcBinIf->encodeBin( 0, m_cCuCtxLastY.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtxLast] ) );
  }
  if(uiPosY < uiWidth - 1)
  {
    m_pcBinIf->encodeBin( 1, m_cCuCtxLastY.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtxLast] ) );
  }
}

Void TEncSbac::codeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseCoeffNxN()\teType=" )
  DTRACE_CABAC_V( eTType )
  DTRACE_CABAC_T( "\twidth=" )
  DTRACE_CABAC_V( uiWidth )
  DTRACE_CABAC_T( "\theight=" )
  DTRACE_CABAC_V( uiHeight )
  DTRACE_CABAC_T( "\tdepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\tabspartidx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\ttoCU-X=" )
  DTRACE_CABAC_V( pcCU->getCUPelX() )
  DTRACE_CABAC_T( "\ttoCU-Y=" )
  DTRACE_CABAC_V( pcCU->getCUPelY() )
  DTRACE_CABAC_T( "\tCU-addr=" )
  DTRACE_CABAC_V(  pcCU->getAddr() )
  DTRACE_CABAC_T( "\tinCU-X=" )
  DTRACE_CABAC_V( g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tinCU-Y=" )
  DTRACE_CABAC_V( g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tpredmode=" )
  DTRACE_CABAC_V(  pcCU->getPredictionMode( uiAbsPartIdx ) )
  DTRACE_CABAC_T( "\n" )
  if( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  
  UInt uiNumSig = 0;
  UInt uiCTXIdx = 0;
  
  switch( uiWidth )
  {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
  }
  
  // compute number of significant coefficients
  uiNumSig = TEncEntropy::countNonZeroCoeffs(pcCoef, uiWidth);
  
  if ( uiNumSig == 0 )
    return;
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- encode significance map -----
  const UInt   uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt   uiMaxNumCoeff     = 1 << ( uiLog2BlockSize << 1 );
  const UInt   uiNum4x4Blk       = max<UInt>( 1, uiMaxNumCoeff >> 4 );
#if QC_MDCS
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif //QC_MDCS
  
    //===== code last coeff =====
    UInt uiScanPosLast = 0, uiPosLastX, uiPosLastY;
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
#if QC_MDCS
      UInt uiBlkPos    = g_auiSigLastScan[uiScanIdx][uiLog2BlockSize-1][uiScanPos]; 
#else
      UInt  uiBlkPos   = g_auiFrameScanXY[ uiLog2BlockSize-1 ][ uiScanPos ];
#endif //QC_MDCS
      uiPosLastY = uiBlkPos >> uiLog2BlockSize;
      uiPosLastX = uiBlkPos - ( uiPosLastY << uiLog2BlockSize );

      if( pcCoef[ uiBlkPos ] != 0 )
      {
        uiNumSig--;
        if( uiNumSig == 0 )
        {
#if QC_MDCS
          codeLastSignificantXY(uiPosLastX, uiPosLastY, uiWidth, eTType, uiCTXIdx, uiScanIdx);
#else
          codeLastSignificantXY(uiPosLastX, uiPosLastY, uiWidth, eTType, uiCTXIdx, 0);
#endif
          uiScanPosLast = uiScanPos;
          break;
        }
      }
    }

    //===== code significance flag =====
    {
      for( UInt uiScanPos = 0; uiScanPos < uiScanPosLast; uiScanPos++ )
      {
#if QC_MDCS
        UInt uiBlkPos   = g_auiSigLastScan[uiScanIdx][uiLog2BlockSize-1][uiScanPos]; 
#else
        UInt  uiBlkPos  = g_auiFrameScanXY[ uiLog2BlockSize-1 ][ uiScanPos ];
#endif //QC_MDCS
        UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
        UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
        UInt  uiSig     = pcCoef[ uiBlkPos ] != 0 ? 1 : 0;
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
        m_pcBinIf->encodeBin( uiSig, m_cCUSigSCModel.get( 0, eTType, uiCtxSig ) );
      }
    }

  /*
   * Sign and bin0 PCP (Section 3.2 and 3.3 of JCTVC-B088)
   */
  Int  c1, c2;
  UInt uiGoRiceParam = 0;

  if( uiNum4x4Blk > 1 )
  {
    Bool b1stBlk  = true;
    UInt uiNumOne = 0;
    
    for( UInt uiSubBlk = 0; uiSubBlk < uiNum4x4Blk; uiSubBlk++ )
    {
      UInt uiCtxSet    = 0;
      UInt uiSubNumSig = 0;
      UInt uiSubPosX   = 0;
      UInt uiSubPosY   = 0;
      uiGoRiceParam    = 0;

      uiSubPosX = g_auiFrameScanX[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
      uiSubPosY = g_auiFrameScanY[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
      
      TCoeff* piCurr = &pcCoef[ uiSubPosX + uiSubPosY * uiWidth ];
      
      for( UInt uiY = 0; uiY < 4; uiY++ )
      {
        for( UInt uiX = 0; uiX < 4; uiX++ )
        {
          if( piCurr[ uiX ] )
          {
            uiSubNumSig++;
          }
        }
        piCurr += uiWidth;
      }
      
      if( uiSubNumSig > 0 )
      {
        c1 = 1;
        c2 = 0;
        
        if( b1stBlk )
        {
          b1stBlk  = false;
          uiCtxSet = 5;
        }
        else
        {
          uiCtxSet = ( uiNumOne >> 2 ) + 1;
          uiNumOne = 0;
        }
        
        TCoeff sigCoeff[16];
        Int sigCoeffCount = 0;
        ContextModel *baseCtxMod;
        
        baseCtxMod = m_cCUOneSCModel.get( 0, eTType ) + 5 * uiCtxSet;
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos & 3;
          UInt  uiIndex   = ( ( uiSubPosY + uiPosY ) << uiLog2BlockSize ) + uiSubPosX + uiPosX;
          TCoeff val = pcCoef[uiIndex];
          
          if( val )
          {
            sigCoeff[sigCoeffCount++] = val;
            UInt symbol = abs(val) > 1;
            m_pcBinIf->encodeBin( symbol, baseCtxMod[c1] );
            
            if( symbol )
            {
              c1 = 0;
            }
            else if( c1 & 3 ) // Increment if c1 is 1, 2 or 3 (but not 0 or 4)
            {
              c1++;
            }
          }
        }
        
        if (c1 == 0)
        {
          baseCtxMod = m_cCUAbsSCModel.get( 0, eTType ) + 5 * uiCtxSet;
          for (Int idx = 0; idx < sigCoeffCount; idx++)
          {
            UInt absVal = abs(sigCoeff[idx]);
            if (absVal > 1)
            {
              UInt symbol = absVal > 2;
              
              m_pcBinIf->encodeBin( symbol, baseCtxMod[c2] );
              
              if( symbol )
              {
                xWriteGoRiceExGolomb( absVal - 3, uiGoRiceParam );
              }
              c2 += (c2 < 4); // Increment c2 up to a maximum value of 4
              uiNumOne++;
            }
          }
        }
        
        for (Int idx = 0; idx < sigCoeffCount; idx++)
        {
          m_pcBinIf->encodeBinEP( sigCoeff[idx] < 0 );
        }
      }
    }
  }
  else
  {
    c1 = 1;
    c2 = 0;
    TCoeff sigCoeff[16];
    Int sigCoeffCount = 0;
    ContextModel *baseCtxMod;
    
    baseCtxMod = m_cCUOneSCModel.get( 0, eTType );    
    for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
      TCoeff val = pcCoef[uiIndex];
      
      if( val )
      {
        sigCoeff[sigCoeffCount++] = val;
        UInt symbol = abs(val) > 1;
        m_pcBinIf->encodeBin( symbol, baseCtxMod[c1] );
        
        if( symbol )
        {
          c1 = 0;
        }
        else if( c1 & 3 ) // Increment if c1 is 1, 2 or 3 (but not 0 or 4)
        {
          c1++;
        }
      }
    }
    
    if (c1 == 0)
    {
      baseCtxMod = m_cCUAbsSCModel.get( 0, eTType );    
      for (Int idx = 0; idx < sigCoeffCount; idx++)
      {
        UInt absVal = abs(sigCoeff[idx]);
        if (absVal > 1)
        {
          UInt symbol = absVal > 2;
          
          m_pcBinIf->encodeBin( symbol, baseCtxMod[c2] );
          
          if( symbol )
          {
            xWriteGoRiceExGolomb( absVal - 3, uiGoRiceParam );
          }
          c2 += (c2 < 4); // Increment c2 up to a maximum value of 4
        }
      }
    }
    
    for (Int idx = 0; idx < sigCoeffCount; idx++)
    {
      m_pcBinIf->encodeBinEP( sigCoeff[idx] < 0 );
    }
  }
  return;
}

#if MVD_CTX
/** Encode a motion vector difference
 * \param iMvd motion vector difference
 * \param uiAbsSumL motion vector difference of left PU
 * \param uiAbsSumA motion vector difference of above PU
 * \param uiCtx index for context set based on vertical or horizontal component
 */
Void TEncSbac::xWriteMvd( Int iMvd, UInt uiAbsSumL, UInt uiAbsSumA, UInt uiCtx )
#else
Void TEncSbac::xWriteMvd( Int iMvd, UInt uiAbsSum, UInt uiCtx )
#endif
{
  UInt uiLocalCtx = 0;
#if MVD_CTX
  uiLocalCtx += (uiAbsSumA>16) ? 1 : 0;
  uiLocalCtx += (uiAbsSumL>16) ? 1 : 0;
#else
  if ( uiAbsSum >= 3 )
  {
    uiLocalCtx += ( uiAbsSum > 32 ) ? 2 : 1;
  }
#endif

  UInt uiSymbol = ( 0 == iMvd ) ? 0 : 1;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );
  if ( 0 == uiSymbol )
  {
    return;
  }
  
  UInt uiSign = 0;
  if ( 0 > iMvd )
  {
    uiSign = 1;
    iMvd   = -iMvd;
  }
  xWriteExGolombMvd( iMvd-1, &m_cCUMvdSCModel.get( 0, uiCtx, 3 ), 3 );
  m_pcBinIf->encodeBinEP( uiSign );
  
  return;
}

Void  TEncSbac::xWriteExGolombMvd( UInt uiSymbol, ContextModel* pcSCModel, UInt uiMaxBin )
{
  if ( ! uiSymbol )
  {
    m_pcBinIf->encodeBin( 0, *pcSCModel );
    return;
  }
  
  m_pcBinIf->encodeBin( 1, *pcSCModel );
  
  Bool bNoExGo = ( uiSymbol < 8 );
  UInt uiCount = 1;
  pcSCModel++;
  
  while ( --uiSymbol && ++uiCount <= 8 )
  {
    m_pcBinIf->encodeBin( 1, *pcSCModel );
    if ( uiCount == 2 )
    {
      pcSCModel++;
    }
    if ( uiCount == uiMaxBin )
    {
      pcSCModel++;
    }
  }
  
  if ( bNoExGo )
  {
    m_pcBinIf->encodeBin( 0, *pcSCModel );
  }
  else
  {
    xWriteEpExGolomb( uiSymbol, 3 );
  }
  
  return;
}

Void TEncSbac::codeAlfFlag       ( UInt uiCode )
{
  UInt uiSymbol = ( ( uiCode == 0 ) ? 0 : 1 );
  m_pcBinIf->encodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
}

#if MTK_NONCROSS_INLOOP_FILTER
/** Code number of ALF CU control flags
 * \param uiCode number of ALF CU control flags
 * \param minValue predictor of number of ALF CU control flags
 * \param iDepth the possible max. processing CU depth
 */
Void TEncSbac::codeAlfFlagNum( UInt uiCode, UInt minValue, Int iDepth)
#else
Void TEncSbac::codeAlfFlagNum( UInt uiCode, UInt minValue )
#endif
{
  UInt uiLength = 0;
#if MTK_NONCROSS_INLOOP_FILTER
  UInt maxValue = (minValue << (iDepth*2));
#else
  UInt maxValue = (minValue << (this->getMaxAlfCtrlDepth()*2));
#endif
  assert((uiCode>=minValue)&&(uiCode<=maxValue));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  UInt uiSymbol = uiCode - minValue;
  if(uiLength)
  {
    while( uiLength-- )
    {
      m_pcBinIf->encodeBinEP( (uiSymbol>>uiLength) & 0x1 );
    }
  }
}

Void TEncSbac::codeAlfCtrlFlag( UInt uiSymbol )
{
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0) );
}

Void TEncSbac::codeAlfUvlc       ( UInt uiCode )
{
  Int i;
  
  if ( uiCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
    for ( i=0; i<uiCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
  }
}

Void TEncSbac::codeAlfSvlc       ( Int iCode )
{
  Int i;
  
  if ( iCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
    
    // write sign
    if ( iCode > 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
      iCode = -iCode;
    }
    
    // write magnitude
    for ( i=0; i<iCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
  }
}

#if MTK_SAO
Void TEncSbac::codeAoFlag       ( UInt uiCode )
{
  UInt uiSymbol = ( ( uiCode == 0 ) ? 0 : 1 );
  m_pcBinIf->encodeBin( uiSymbol, m_cAOFlagSCModel.get( 0, 0, 0 ) );
}
Void TEncSbac::codeAoUvlc       ( UInt uiCode )
{
  Int i;

  if ( uiCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cAOUvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cAOUvlcSCModel.get( 0, 0, 0 ) );
    for ( i=0; i<uiCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cAOUvlcSCModel.get( 0, 0, 1 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cAOUvlcSCModel.get( 0, 0, 1 ) );
  }
}
Void TEncSbac::codeAoSvlc       ( Int iCode )
{
  Int i;

  if ( iCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cAOSvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cAOSvlcSCModel.get( 0, 0, 0 ) );

    // write sign
    if ( iCode > 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cAOSvlcSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cAOSvlcSCModel.get( 0, 0, 1 ) );
      iCode = -iCode;
    }

    // write magnitude
    for ( i=0; i<iCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cAOSvlcSCModel.get( 0, 0, 2 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cAOSvlcSCModel.get( 0, 0, 2 ) );
  }
}
#endif

/*!
 ****************************************************************************
 * \brief
 *   estimate bit cost for CBP, significant map and significant coefficients
 ****************************************************************************
 */
Void TEncSbac::estBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  estCBFBit( pcEstBitsSbac, 0, eTType );
  
  // encode significance map
  estSignificantMapBit( pcEstBitsSbac, uiCTXIdx, eTType );
  
  // encode significant coefficients
  estSignificantCoefficientsBit( pcEstBitsSbac, uiCTXIdx, eTType );
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost for each CBP bit
 ****************************************************************************
 */
Void TEncSbac::estCBFBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  ContextModel *pCtx = m_cCUQtCbfSCModel.get( 0 );

  for( UInt uiCtxInc = 0; uiCtxInc < 3*NUM_QT_CBF_CTX; uiCtxInc++ )
  {
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 0 ] = biari_no_bits( 0, pCtx[ uiCtxInc ] );
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 1 ] = biari_no_bits( 1, pCtx[ uiCtxInc ] );
  }

  for( UInt uiCtxInc = 0; uiCtxInc < 4; uiCtxInc++ )
  {
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 0 ] = biari_no_bits( 0, m_cCUQtRootCbfSCModel.get( 0, 0, uiCtxInc ) );
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 1 ] = biari_no_bits( 1, m_cCUQtRootCbfSCModel.get( 0, 0, uiCtxInc ) );
  }
}


/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient map
 ****************************************************************************
 */
Void TEncSbac::estSignificantMapBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  Int firstCtx, numCtx = 15;
  switch (uiCTXIdx)
  {
    case 2: // 32x32
      firstCtx = 35;
      break;
    case 3: // 16x16
      firstCtx = 31;
      break;
    case 4: // 8x8
      firstCtx = 15;
      numCtx = 16;
      break;
    default: // 4x4 (case 5)
      firstCtx = 0;
      break;
  }
  for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
  {
    for( UInt uiBin = 0; uiBin < 2; uiBin++ )
    {
      pcEstBitsSbac->significantBits[ ctxIdx ][ uiBin ] = biari_no_bits ( uiBin, m_cCUSigSCModel.get(  0, eTType, ctxIdx ) );
    }
  }

  Int iBitsX = 0, iBitsY = 0;
  const UInt uiCtxOffset = g_uiCtxXYOffset[uiCTXIdx];
  const UInt uiWidthM1   = (1 << (7-uiCTXIdx)) - 1;
  for ( UInt uiCtx = 0; uiCtx < uiWidthM1; uiCtx++ )
  {
    pcEstBitsSbac->lastXBits[uiCtx] = iBitsX + biari_no_bits (1, m_cCuCtxLastX.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtx] ));
    pcEstBitsSbac->lastYBits[uiCtx] = iBitsY + biari_no_bits (1, m_cCuCtxLastY.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtx] ));
    iBitsX += biari_no_bits (0, m_cCuCtxLastX.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtx] ));
    iBitsY += biari_no_bits (0, m_cCuCtxLastY.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiCtx] ));
  }
  pcEstBitsSbac->lastXBits[uiWidthM1] = iBitsX;
  pcEstBitsSbac->lastYBits[uiWidthM1] = iBitsY;
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoefficientsBit( estBitsSbacStruct* pcEstBitsSbac, UInt uiCTXIdx, TextType eTType )
{
  ContextModel *ctxOne = m_cCUOneSCModel.get(0, eTType);
  ContextModel *ctxAbs = m_cCUAbsSCModel.get(0, eTType);
  
  for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX; ctxIdx++)
  {
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 0 ] = biari_no_bits( 0, ctxOne[ ctxIdx ] );
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 1 ] = biari_no_bits( 1, ctxOne[ ctxIdx ] );    
  }
  for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX; ctxIdx++)
  {
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 0 ] = biari_no_bits( 0, ctxAbs[ ctxIdx ] );
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 1 ] = biari_no_bits( 1, ctxAbs[ ctxIdx ] );    
  }
}

Int TEncSbac::biari_no_bits( Short symbol, ContextModel& rcSCModel )
{
  return rcSCModel.getEntropyBits(symbol);
}
