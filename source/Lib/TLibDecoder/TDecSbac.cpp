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

/** \file     TDecSbac.cpp
    \brief    Context-adaptive entropy decoder class
*/

#include "TDecSbac.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_pcTDecBinIf               ( NULL )
, m_bAlfCtrl                  ( false )
, m_uiMaxAlfCtrlDepth         ( 0 )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
, m_cCUMergeFlagExtSCModel    ( 1,             1,               NUM_MERGE_FLAG_EXT_CTX        )
, m_cCUMergeIdxExtSCModel     ( 1,             1,               NUM_MERGE_IDX_EXT_CTX         )
, m_cCUAlfCtrlFlagSCModel     ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         )
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             )
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             )
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   )
#if ADD_PLANAR_MODE
, m_cPlanarFlagSCModel        ( 1,             1,               NUM_PLANARFLAG_CTX            )
#endif
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           )
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             )
, m_cCURefPicSCModel          ( 1,             1,               NUM_REF_NO_CTX                )
#if MODIFIED_MVD_CODING
, m_cCUMvdSCModel             ( 1,             1,               NUM_MV_RES_CTX                )
#else
, m_cCUMvdSCModel             ( 1,             2,               NUM_MV_RES_CTX                )
#endif
, m_cCUTransSubdivFlagSCModel ( 1,             1,               NUM_TRANS_SUBDIV_FLAG_CTX     )
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           )
, m_cCUDeltaQpSCModel         ( 1,             1,               NUM_DELTA_QP_CTX              )
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                )
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

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecSbac::resetEntropy          (TComSlice* pcSlice)
{
  Int  iQp              = pcSlice->getSliceQp();
  SliceType eSliceType  = pcSlice->getSliceType();
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_ALF_CTRL_FLAG );
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
#if MODIFIED_LAST_CODING
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST );
#else
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST_X );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (Short*)INIT_LAST_Y );
#endif
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ABS_FLAG );
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
#if MTK_SAO
  m_cAOFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_FLAG );
  m_cAOUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_UVLC );
  m_cAOSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_AO_SVLC );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
  
  m_uiLastDQpNonZero  = 0;
  
  // new structure
  m_uiLastQp          = iQp;
  
  m_pcTDecBinIf->start();
}

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{
  m_pcTDecBinIf->decodeBinTrm( ruiBit );
}


Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
  
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( ruiSymbol == 0 || uiMaxSymbol == 1 )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont && ( uiSymbol < uiMaxSymbol - 1 ) );
  
  if( uiCont && ( uiSymbol == uiMaxSymbol - 1 ) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

#if !MODIFIED_MVD_CODING
#if MVD_CTX
/** Decode a motion vector difference
 * \param riMvdComp motion vector difference
 * \param uiAbsSumL motion vector difference of left PU
 * \param uiAbsSumA motion vector difference of above PU
 * \param uiCtx index for context set based on vertical or horizontal component
 */
Void TDecSbac::xReadMvd( Int& riMvdComp, UInt uiAbsSumL, UInt uiAbsSumA, UInt uiCtx )
#else
Void TDecSbac::xReadMvd( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx )
#endif
{
  UInt uiLocalCtx = 0;

#if MVD_CTX
  uiLocalCtx += (uiAbsSumA>16) ? 1 : 0;
  uiLocalCtx += (uiAbsSumL>16) ? 1 : 0;
#else
  if( uiAbsSum >= 3 )
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 2 : 1;
  }
#endif

  riMvdComp = 0;
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );
  
  if (!uiSymbol)
  {
    return;
  }
  
  xReadExGolombMvd( uiSymbol, &m_cCUMvdSCModel.get( 0, uiCtx, 3 ), 3 );
  uiSymbol++;
  
  UInt uiSign;
  m_pcTDecBinIf->decodeBinEP( uiSign );
  
  riMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;
  
  return;
}

Void TDecSbac::xReadExGolombMvd( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiMaxBin )
{
  UInt uiSymbol;
  
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  m_pcTDecBinIf->decodeBin( uiSymbol, pcSCModel[1] );
  
  ruiSymbol = 1;
  
  if( !uiSymbol )
  {
    return;
  }
  
  pcSCModel += 2;
  UInt uiCount = 2;
  
  do
  {
    if( uiMaxBin == uiCount )
    {
      pcSCModel++;
    }
    m_pcTDecBinIf->decodeBin( uiSymbol, *pcSCModel );
    uiCount++;
  }
  while( uiSymbol && ( uiCount != 8 ) );
  
  ruiSymbol = uiCount - 1;
  
  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 3 );
    ruiSymbol += uiSymbol + 1;
  }
  
  return;
}
#endif

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  while( uiBit )
  {
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  uiCount--;
  while( uiCount-- )
  {
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount;
  }
  
  ruiSymbol = uiSymbol;
  
  return;
}

Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont );
  
  ruiSymbol = uiSymbol;
}

/** Parsing of coeff_abs_level_minus3
 * \param ruiSymbol reference to coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
Void TDecSbac::xReadGoRiceExGolomb( UInt &ruiSymbol, UInt &ruiGoRiceParam )
{
  Bool bExGolomb    = false;
  UInt uiCodeWord   = 0;
  UInt uiQuotient   = 0;
  UInt uiRemainder  = 0;
  UInt uiMaxVlc     = g_auiGoRiceRange[ ruiGoRiceParam ];
  UInt uiMaxPreLen  = g_auiGoRicePrefixLen[ ruiGoRiceParam ];

  do
  {
    uiQuotient++;
    m_pcTDecBinIf->decodeBinEP( uiCodeWord );
  }
  while( uiCodeWord && uiQuotient < uiMaxPreLen );

  uiCodeWord  = 1 - uiCodeWord;
  uiQuotient -= uiCodeWord;

  for( UInt ui = 0; ui < ruiGoRiceParam; ui++ )
  {
    m_pcTDecBinIf->decodeBinEP( uiCodeWord );
    if( uiCodeWord )
    {
      uiRemainder += 1 << ui;
    }
  }

  ruiSymbol      = uiRemainder + ( uiQuotient << ruiGoRiceParam );
  bExGolomb      = ruiSymbol == ( uiMaxVlc + 1 );

  if( bExGolomb )
  {
    xReadEpExGolomb( uiCodeWord, 0 );
    ruiSymbol += uiCodeWord;
  }

  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( ruiSymbol, 15 ) ];

  return;
}


#if E057_INTRA_PCM
/** Parse I_PCM information. 
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, parse its PCM alignment bits and codes. 
 */
Void TDecSbac::parseIPCMInfo ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  m_pcTDecBinIf->decodeBinTrm(uiSymbol);

  if (uiSymbol)
  {
    Bool bIpcmFlag = true;

    m_pcTDecBinIf->decodePCMAlignBits();

    pcCU->setPartSizeSubParts  ( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts      ( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    pcCU->setIPCMFlagSubParts  ( bIpcmFlag, uiAbsPartIdx, uiDepth );

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
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
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
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
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
        UInt uiSample;
        m_pcTDecBinIf->xReadPCMCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    m_pcTDecBinIf->resetBac();
  }
}
#endif

Void TDecSbac::parseAlfCtrlDepth( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol( uiSymbol, m_cALFUvlcSCModel.get( 0 ), 1, g_uiMaxCUDepth - 1 );
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecSbac::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( !m_bAlfCtrl )
  {
    return;
  }
  
  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth( uiAbsPartIdx, m_uiMaxAlfCtrlDepth ) )
  {
    return;
  }
  
  UInt uiSymbol;
#if DNB_ALF_CTRL_FLAG
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUAlfCtrlFlagSCModel.get( 0 ) );
#else
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx ) ) );
#endif
  
  if( uiDepth > m_uiMaxAlfCtrlDepth )
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth );
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

/** parse skip flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCtxSkip = pcCU->getCtxSkipFlag( uiAbsPartIdx );
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tSkipFlag" );
  DTRACE_CABAC_T( "\tuiCtxSkip: ");
  DTRACE_CABAC_V( uiCtxSkip );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
  
  if( uiSymbol )
  {
    pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    
#if HHI_MRG_SKIP
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
    pcCU->setMergeFlagSubParts( true , uiAbsPartIdx, 0, uiDepth );
    } 
    else
#endif // HHI_MRG_SKIP
    {
      TComMv cZeroMv(0,0);
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      
      pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts  ( 0, 0, 0, uiAbsPartIdx, uiDepth );
      
      if ( pcCU->getSlice()->isInterP() )
      {
        pcCU->setInterDirSubParts( 1, uiAbsPartIdx, 0, uiDepth );
        
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
      else
      {
        pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );
        
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
    }
  }
}

/** parse merge flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPUIdx
 * \returns Void
 */
Void TDecSbac::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{
  UInt uiSymbol;
#if DNB_MERGE_FLAG
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiPUIdx, uiDepth );
#else
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
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeFlagExtSCModel.get( 0, 0, uiCtx ) );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiPUIdx, uiDepth );
#endif

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

Void TDecSbac::parseMergeIndex ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
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

  UInt uiUnaryIdx = 0;
  for( ; uiUnaryIdx < uiNumCand - 1; ++uiUnaryIdx )
  {
    UInt uiSymbol = 0;
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[uiUnaryIdx] ) );
    if( uiSymbol == 0 )
    {
      break;
    }
  }
#if !(MRG_AMVP_FIXED_IDX_F470)
  if( !bLeftInvolved )
  {
    ++uiUnaryIdx;
  }
  if( !bAboveInvolved && uiUnaryIdx >= 1 )
  {
    ++uiUnaryIdx;
  }
  if( !bCollocatedInvolved && uiUnaryIdx >= 2 )
  {
    ++uiUnaryIdx;
  }
  if( !bCornerInvolved && uiUnaryIdx >= 3 )
  {
    ++uiUnaryIdx;
  }
#endif
  ruiMergeIndex = uiUnaryIdx;

  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseMergeIndex()" )
  DTRACE_CABAC_T( "\tuiMRGIdx= " )
  DTRACE_CABAC_V( ruiMergeIndex )
  DTRACE_CABAC_T( "\tuiNumCand= " )
  DTRACE_CABAC_V( uiNumCand )
  DTRACE_CABAC_T( "\tbLeftInvolved= " )
  DTRACE_CABAC_V( bLeftInvolved )
  DTRACE_CABAC_T( "\tbAboveInvolved= " )
  DTRACE_CABAC_V( bAboveInvolved )
  DTRACE_CABAC_T( "\tbCollocatedInvolved= " )
  DTRACE_CABAC_V( bCollocatedInvolved )
  DTRACE_CABAC_T( "\tbCornerRTInvolved= " )
  DTRACE_CABAC_V( bCornerInvolved )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseMVPIdx      ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
#if MRG_AMVP_FIXED_IDX_F470
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, AMVP_MAX_NUM_CANDS-1);
#else
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, iMVPNum-1);
#endif
  riMVPIdx = uiSymbol;
}

Void TDecSbac::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return;
  }
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth ) ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );
  
  return;
}

/** parse partition size
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol, uiMode = 0;
  PartSize eMode;
  
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    uiSymbol = 1;
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    }
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;
  }
  else
  {
    {
      UInt uiMaxNumBits = 3;
#if DISABLE_4x4_INTER
      if(pcCU->getSlice()->getSPS()->getDisInter4x4() && ( (g_uiMaxCUWidth>>uiDepth)==8) && ( (g_uiMaxCUHeight>>uiDepth)==8) )
      {
        uiMaxNumBits = 2;
      }
#endif
      for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
      {
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
        if ( uiSymbol )
        {
          break;
        }
        uiMode++;
      }
    }
    eMode = (PartSize) uiMode;
#if DISABLE_4x4_INTER
    if(pcCU->getSlice()->getSPS()->getDisInter4x4() && ( (g_uiMaxCUWidth>>uiDepth)==8) && ( (g_uiMaxCUHeight>>uiDepth)==8) )
    {
      if (pcCU->getSlice()->isInterB() && uiMode == 2)
      {
        uiSymbol = 0;
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 2) );
        if (uiSymbol == 0)
        {
          pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
          if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
          {
            m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
          }
          eMode = SIZE_NxN;
          if (uiSymbol == 0)
            eMode = SIZE_2Nx2N;
        }
      }
    }
    else
    {
#endif    
    if (pcCU->getSlice()->isInterB() && uiMode == 3)
    {
      uiSymbol = 0;
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
      {
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
      }
      
      if (uiSymbol == 0)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
        {
          m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
        }
        if (uiSymbol == 0)
          eMode = SIZE_2Nx2N;
      }
    }
#if DISABLE_4x4_INTER
    }
#endif
  }
  
  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
  
  UInt uiTrLevel = 0;
  
  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
  
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
  {
    if( pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1+uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
  }
}

/** parse prediction mode
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecSbac::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return;
  }
  
  UInt uiSymbol;
  Int  iPredMode = MODE_INTER;
  if ( pcCU->getSlice()->isInterB() )
  {
    pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
    return;
  }
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
  iPredMode += uiSymbol;
  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}
#if MTK_DCM_MPM
Void TDecSbac::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
 
  UInt uiSymbol;
  Int  uiIPredMode;

  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  
 
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
 
  if ( uiSymbol )
  {
#if FIXED_MPM
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2 ) );
    uiIPredMode = uiPreds[uiSymbol];
#else
    if(uiPredNum == 1)   
    {
      uiIPredMode = uiPreds[0];
    }
    else 
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 2) );
      uiIPredMode = uiPreds[uiSymbol];
    }
#endif
  }
  else
  {
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3; }
    }
    else
    {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1;
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2;
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3;
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 4;

    if (uiIPredMode == 31){ // Escape coding for the last two modes
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
      uiIPredMode = uiSymbol ? 32 : 31;
    }

  }

#if FIXED_MPM
    // planar mode is coded with shortest codeword (0)
    if( uiIPredMode == 0 && uiPreds[uiPredNum - 1] != PLANAR_IDX )
    {
      uiIPredMode = PLANAR_IDX;
    }
    else
    {
      if( uiPreds[uiPredNum - 1] != PLANAR_IDX )
      {
        uiIPredMode--;
      }

      for( UInt i = 0; i < uiPredNum; i++ )
      {
        if( uiIPredMode >= uiPreds[i] )
        { 
          uiIPredMode ++; 
        }
      }
    }
#else
    for(UInt i = 0; i < uiPredNum; i++)
    {
      if(uiIPredMode >= uiPreds[i]) {  uiIPredMode ++; }
    }
#endif
  }
#if ADD_PLANAR_MODE && !FIXED_MPM
  if (uiIPredMode == 2)
  {
    UInt planarFlag;
    m_pcTDecBinIf->decodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,0) );
    if ( planarFlag )
    {
      uiIPredMode = PLANAR_IDX;
    }
  }
#endif

  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );
}
#else
Void TDecSbac::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
  
  if ( uiSymbol )
    uiIPredMode = iMostProbable;
  else
  {
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3; }
    }
    else
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 4;
      
      if (uiIPredMode == 31)
      { // Escape coding for the last two modes
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
        uiIPredMode = uiSymbol ? 32 : 31;
      }
    }
    
    if (uiIPredMode >= iMostProbable)
      uiIPredMode++;
  }
  
#if ADD_PLANAR_MODE
  if (uiIPredMode == 2)
  {
    UInt planarFlag;
    m_pcTDecBinIf->decodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,0) );
    if ( planarFlag )
    {
      uiIPredMode = PLANAR_IDX;
    }
  }
#endif
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );
}
#endif
Void TDecSbac::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if FIXED_MPM
  UInt uiSymbol;

  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );

  if( uiSymbol == 0 )
  {
    uiSymbol = DM_CHROMA_IDX;
  } 
  else 
  {
    if( pcCU->getSlice()->getSPS()->getUseLMChroma() )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      uiSymbol = 1;
    }

    if( uiSymbol == 0 )
    {
      uiSymbol = LM_CHROMA_IDX;
    } 
    else
    {
      UInt uiIPredMode;
      xReadUnaryMaxSymbol( uiIPredMode, m_cCUChromaPredSCModel.get( 0, 0 ) + 1, 0, 3 );
#if CHROMA_CODEWORD_SWITCH 
      uiIPredMode = ChromaMapping[uiIPredMode];
#endif
      UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
      pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );
      uiSymbol = uiAllowedChromaDir[ uiIPredMode ];
    }
  }
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  return;
#else
  UInt uiSymbol;
  
  UInt uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
#if ADD_PLANAR_MODE
  if ( (uiMode == 2 ) || (uiMode == PLANAR_IDX) )
  {
    uiMode = 4;
  }
#endif

#if LM_CHROMA
  Int  iMaxMode = pcCU->getSlice()->getSPS()->getUseLMChroma() ? 3 : 4;
  Int  iMax = uiMode < iMaxMode ? 2 : 3; 
  
#if DNB_INTRA_CHR_PRED_MODE
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUChromaPredSCModel.get( 0 ) );
#else
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );
#endif
  
  if ( uiSymbol )
  {
#if DNB_INTRA_CHR_PRED_MODE
    xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0 ) + 1, 0, iMax );
#else
    xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, iMax );
#endif
    uiSymbol++;
  }
  
  //switch codeword
  if (uiSymbol == 0)
  {
    uiSymbol = 4;
  } 
  else if (uiSymbol == 1 && pcCU->getSlice()->getSPS()->getUseLMChroma())
  {
    uiSymbol = LM_CHROMA_IDX;
  }
  else 
  {
#if CHROMA_CODEWORD_SWITCH
    uiSymbol = ChromaMapping[iMax-2][uiSymbol];
#endif
    
    if (pcCU->getSlice()->getSPS()->getUseLMChroma())
       uiSymbol --;

    if (uiSymbol <= uiMode)
       uiSymbol --;
  }
#else // <-- LM_CHROMA
  Int  iMax = uiMode < 4 ? 2 : 3;
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );
  
  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, iMax );
    uiSymbol++;
  }
  
  //switch codeword
  if (uiSymbol == 0)
  {
    uiSymbol = 4;
  } 
  else
  {
#if CHROMA_CODEWORD_SWITCH
    uiSymbol = ChromaMapping[iMax-2][uiSymbol];
#endif
    if (uiSymbol <= uiMode)
    {
      uiSymbol --;
    }
  }
#endif // <-- LM_CHROMA

#if ADD_PLANAR_MODE
  if (uiSymbol == 2)
  {
    uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
    if (uiMode == 2)
    {
      uiSymbol = PLANAR_IDX;
    }
    else if (uiMode != PLANAR_IDX)
    {
      UInt planarFlag;
      m_pcTDecBinIf->decodeBin( planarFlag, m_cPlanarFlagSCModel.get(0,0,1) );
      if ( planarFlag )
      {
        uiSymbol = PLANAR_IDX;
      }
    }
  }
#endif
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  
  return;
#endif
}

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
#if DNB_INTER_PRED_MODE
  const UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );
  ContextModel *pCtx = m_cCUInterDirSCModel.get( 0 );
  m_pcTDecBinIf->decodeBin( uiSymbol, *( pCtx + uiCtx ) );

  if( uiSymbol )
  {
    uiSymbol = 2;
  }
  else if( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 )
  {
    uiSymbol = 0;
  }
  else if( pcCU->getSlice()->getNoBackPredFlag() )
  {
    uiSymbol = 0;
  }
  else
  {
    pCtx++;
    m_pcTDecBinIf->decodeBin( uiSymbol, *( pCtx + 3 ) );
  }
#else
  UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );
  
  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
  else if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
  {
    uiSymbol = 0;
  }
  else if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    uiSymbol = 0;
  }
  else
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }
#endif
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecSbac::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;

  if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C ) > 0 && eRefList==REF_PIC_LIST_C)
  {
#if DNB_REF_FRAME_IDX
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );
      uiSymbol++;
    }
#else
    UInt uiCtx;

    uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, RefPicList(pcCU->getSlice()->getListIdFromIdxOfLC(0)) );

    m_pcTDecBinIf->decodeBin ( uiSymbol, m_cCURefPicSCModel.get( 0, 0, uiCtx ) );

    if ( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );

      uiSymbol++;
    }
#endif
    riRefFrmIdx = uiSymbol;
  }
  else
  {
#if DNB_REF_FRAME_IDX
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
      uiSymbol++;
    }
#else
    UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );
    
    m_pcTDecBinIf->decodeBin ( uiSymbol, m_cCURefPicSCModel.get( 0, 0, uiCtx ) );
    if ( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
      
      uiSymbol++;
    }
#endif
    riRefFrmIdx = uiSymbol;
  }

  return;
}

Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
#if MODIFIED_MVD_CODING
  UInt uiSymbol;
  UInt uiHorAbs;
  UInt uiVerAbs;
  UInt uiHorSign;
  UInt uiVerSign;
  ContextModel *pCtx = m_cCUMvdSCModel.get( 0 );

  m_pcTDecBinIf->decodeBin( uiHorAbs, *pCtx );
  m_pcTDecBinIf->decodeBin( uiVerAbs, *pCtx );

  const Bool bHorAbsGr0 = uiHorAbs != 0;
  const Bool bVerAbsGr0 = uiVerAbs != 0;
  pCtx++;

  if( bHorAbsGr0 )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );
    uiHorAbs += uiSymbol;
  }

  if( bVerAbsGr0 )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );
    uiVerAbs += uiSymbol;
  }

  if( bHorAbsGr0 )
  {
    if( 2 == uiHorAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiHorAbs += uiSymbol;
    }

    m_pcTDecBinIf->decodeBinEP( uiHorSign );
  }

  if( bVerAbsGr0 )
  {
    if( 2 == uiVerAbs )
    {
      xReadEpExGolomb( uiSymbol, 1 );
      uiVerAbs += uiSymbol;
    }

    m_pcTDecBinIf->decodeBinEP( uiVerSign );
  }

  const TComMv cMv( uiHorSign ? -Int( uiHorAbs ): uiHorAbs, uiVerSign ? -Int( uiVerAbs ) : uiVerAbs );
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
#else
  Int iHor, iVer;
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
  iHorPredL = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() );
  iVerPredL = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() );
  iHorPredA = ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPredA = ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );
#else
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );
#endif

  TComMv cTmpMv( 0, 0 );
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
  
#if MVD_CTX
  xReadMvd( iHor, iHorPredL, iHorPredA, 0 );
  xReadMvd( iVer, iVerPredL, iVerPredA, 1 );
#else
  xReadMvd( iHor, iHorPred, 0 );
  xReadMvd( iVer, iVerPred, 1 );
#endif  

  // set mvd
  TComMv cMv( iHor, iVer );
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
#endif
  return;
}


Void TDecSbac::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  m_pcTDecBinIf->decodeBin( ruiSubdivFlag, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiLog2TransformBlockSize ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( ruiSubdivFlag )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiLog2TransformBlockSize )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtRootCbf( uiAbsPartIdx );
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  uiQtRootCbf = uiSymbol;
}

Void TDecSbac::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiDQp;
  Int  iDQp;
  
  m_pcTDecBinIf->decodeBin( uiDQp, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  
  if ( uiDQp == 0 )
  {
#if SUB_LCU_DQP
    uiDQp = pcCU->getRefQP(uiAbsPartIdx);
#else
    uiDQp = pcCU->getSlice()->getSliceQp();
#endif
  }
  else
  {
    xReadUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
    iDQp = ( uiDQp + 2 ) / 2;
    
    if ( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
#if SUB_LCU_DQP
    uiDQp = pcCU->getRefQP(uiAbsPartIdx) + iDQp;
#else
    uiDQp = pcCU->getSlice()->getSliceQp() + iDQp;
#endif
  }
  
#if SUB_LCU_DQP
  UInt uiAbsQpCUPartIdx = (uiAbsPartIdx>>(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)) ;
  UInt uiQpCUDepth =   min(uiDepth,pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()) ;
  pcCU->setQPSubParts( uiDQp, uiAbsQpCUPartIdx, uiQpCUDepth );
#else
  pcCU->setQPSubParts( uiDQp, uiAbsPartIdx, uiDepth );
#endif
}

Void TDecSbac::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );
  
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( eType )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  pcCU->setCbfSubParts( uiSymbol << uiTrDepth, eType, uiAbsPartIdx, uiDepth );
}

/** Parse (X,Y) position of the last significant coefficient
 * \param uiPosLastX reference to X component of last coefficient
 * \param uiPosLastY reference to Y component of last coefficient
 * \param uiWidth block width
 * \param eTType plane type / luminance or chrominance
 * \param uiCTXIdx block size context
 * \param uiScanIdx scan type (zig-zag, hor, ver)
 * \returns Void
 * This method decodes the X and Y component within a block of the last significant coefficient.
 */
__inline Void TDecSbac::parseLastSignificantXY( UInt& uiPosLastX, UInt& uiPosLastY, const UInt uiWidth, const TextType eTType, const UInt uiCTXIdx, const UInt uiScanIdx )
{
#if MODIFIED_LAST_CODING
  UInt uiLast;
  const UInt uiMinWidth    = min<UInt>( 4, uiWidth );
  const UInt uiHalfWidth   = uiWidth >> 1;
  const UInt uiLog2BlkSize = g_aucConvertToBit[ uiHalfWidth ] + 2;
  const UInt uiMaxWidth    = max<UInt>( uiMinWidth, uiHalfWidth + 1 );
  const UInt *puiCtxIdx    = g_uiLastCtx + ( uiHalfWidth >= uiMinWidth ? uiHalfWidth : 0 );
  ContextModel *pCtxX      = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY      = m_cCuCtxLastY.get( 0, eTType );

  // posX
  for( uiPosLastX = 0; uiPosLastX < uiMaxWidth-1; uiPosLastX++ )
  {
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxX + puiCtxIdx[ uiPosLastX ] ) );

    if( uiLast )
    {
      break;
    }
  }

  if( !uiLast && uiHalfWidth >= uiMinWidth )
  {
    UInt uiCount = 0, uiTemp = 0;

    while( uiCount < uiLog2BlkSize )
    {
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << uiCount;
      uiCount++;
    }
    uiPosLastX += uiTemp;
  }

  // posY
  for( uiPosLastY = 0; uiPosLastY < uiMaxWidth-1; uiPosLastY++ )
  {
    m_pcTDecBinIf->decodeBin( uiLast, *( pCtxY + puiCtxIdx[ uiPosLastY ] ) );

    if( uiLast )
    {
      break;
    }
  }

  if( !uiLast && uiHalfWidth >= uiMinWidth )
  {
    UInt uiCount = 0, uiTemp = 0;

    while( uiCount < uiLog2BlkSize )
    {
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << uiCount;
      uiCount++;
    }
    uiPosLastY += uiTemp;
  }

  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosLastX, uiPosLastY );
  }
#else
  UInt uiLast;
  const UInt uiCtxOffset = g_uiCtxXYOffset[uiCTXIdx];

  for(uiPosLastX=0; uiPosLastX<uiWidth-1; uiPosLastX++)
  {
    m_pcTDecBinIf->decodeBin( uiLast, m_cCuCtxLastX.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiPosLastX] ) );
    if(uiLast)
    {
      break;
    }
  }

  for(uiPosLastY=0; uiPosLastY<uiWidth-1; uiPosLastY++)
  {
    m_pcTDecBinIf->decodeBin( uiLast, m_cCuCtxLastY.get( 0, eTType, uiCtxOffset + g_uiCtxXY[uiPosLastY] ) );
    if(uiLast)
    {
      break;
    }
  }

#if QC_MDCS
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosLastX, uiPosLastY );
  }
#endif
#endif
}

Void TDecSbac::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
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
  
  if( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  
  UInt uiCTXIdx;
  
  switch(uiWidth)
  {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
  }
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- parse significance map -----
  const UInt  uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt  uiMaxNumCoeff     = 1 << ( uiLog2BlockSize << 1 );
  const UInt  uiMaxNumCoeffM1   = uiMaxNumCoeff - 1;
#if !UNIFIED_SCAN
  const UInt  uiNum4x4Blk       = max<UInt>( 1, uiMaxNumCoeff >> 4 );
#endif
#if QC_MDCS
#if DIAG_SCAN
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#else
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif
#endif //QC_MDCS
  Int sigCoeffCount = 0;
  
  //===== decode last significant =====
  UInt uiPosLastX, uiPosLastY;
#if QC_MDCS
  parseLastSignificantXY( uiPosLastX, uiPosLastY, uiWidth, eTType, uiCTXIdx, uiScanIdx );
#else
  parseLastSignificantXY( uiPosLastX, uiPosLastY, uiWidth, eTType, uiCTXIdx, 0 );
#endif
  UInt uiBlkPosLast      = uiPosLastX + (uiPosLastY<<uiLog2BlockSize);
  pcCoef[ uiBlkPosLast ] = 1;
  sigCoeffCount++;

  //===== decode significance flags =====
#if UNIFIED_SCAN
  UInt uiScanPosLast   = uiBlkPosLast;
  if( uiScanIdx == SCAN_ZIGZAG )
  {
    UInt uiD         = uiPosLastY + uiPosLastX;
    if( uiD < uiWidth )
    {
#if DIAG_SCAN
      uiScanPosLast  = uiPosLastX + (( uiD * ( uiD + 1 ) ) >> 1);
#else
      uiScanPosLast  = ( uiD * ( uiD + 1 ) ) >> 1;
      uiScanPosLast += uiD % 2 ? uiPosLastY : uiPosLastX;
#endif
    } 
    else
    {
      UInt uiDI      = ( (uiWidth-1) << 1 ) - uiD;
#if DIAG_SCAN
      uiScanPosLast  = uiMaxNumCoeffM1 - ( uiDI * ( uiDI + 1 ) >> 1 ) - uiWidth + 1 + uiPosLastX;
#else
      uiScanPosLast  = uiMaxNumCoeffM1 - ( uiDI * ( uiDI + 1 ) >> 1 );
      uiScanPosLast -= uiDI % 2 ? uiWidth - 1 - uiPosLastY : uiWidth - 1 - uiPosLastX;
#endif
    }
  }
  else if( uiScanIdx == SCAN_VER )
  {
    uiScanPosLast = uiPosLastY + (uiPosLastX<<uiLog2BlockSize);
  }

#if DIAG_SCAN
  uiScanIdx = ( uiScanIdx ) ? uiScanIdx : 3; // Map 0 to diagonal scan
#endif
  
  for( UInt uiScanPos = uiScanPosLast-1; uiScanPos != -1; uiScanPos-- )
#else
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeffM1; uiScanPos++ )
#endif
  {
#if QC_MDCS
    UInt uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlockSize-1][uiScanPos]; 
#else
    UInt  uiBlkPos  = g_auiFrameScanXY[ uiLog2BlockSize-1 ][ uiScanPos ];
#endif //QC_MDCS
#if !UNIFIED_SCAN
    if( uiBlkPosLast == uiBlkPos )
    {
      break;
    }
#endif
    UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
    UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
    UInt  uiSig     = 0;
    UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth );
    m_pcTDecBinIf->decodeBin( uiSig, m_cCUSigSCModel.get( 0, eTType, uiCtxSig ) );
    pcCoef[ uiBlkPos ] = uiSig;
    sigCoeffCount += uiSig;
  }

#if UNIFIED_SCAN
  const UInt uiLog2BlockSizeM1 = uiLog2BlockSize - 1;
  const Int  iLastScanSet      = uiScanPosLast >> LOG2_SCAN_SET_SIZE;
  UInt uiNumOne                = 0;
  UInt uiGoRiceParam           = 0;

  for( Int iSubSet = iLastScanSet; iSubSet >= 0 && sigCoeffCount > 0; iSubSet-- )
  {
    Bool bSigSubSet  = false;
    Int  iLastPos    = 0;
    Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
    uiGoRiceParam    = 0;

    const UInt *puiSetScan  = g_auiSigLastScan[uiScanIdx][uiLog2BlockSizeM1] + iSubPos;
    for( Int iScanPos = SCAN_SET_SIZE-1; iScanPos >= 0; iScanPos-- )
    {
      UInt uiBlkPos = puiSetScan[ iScanPos ];
      if( pcCoef[ uiBlkPos ] )
      {
        bSigSubSet  = true;
        iLastPos    = iScanPos;
        break;
      }
    }

    if( bSigSubSet )
    {
      UInt c1 = 1;
      UInt c2 = 0;
      UInt uiCtxSet = iSubSet > 0 ? 3 : 0;
      UInt uiBin;

      if( uiNumOne > 0 )
      {
        uiCtxSet++;
        if( uiNumOne > 3 )
        {
          uiCtxSet++;
        }
      }

      uiNumOne       >>= 1;
      ContextModel *baseCtxMod = m_cCUOneSCModel.get( 0, eTType ) + 5 * uiCtxSet;

      for( Int iScanPos = iLastPos; iScanPos >= 0; iScanPos-- )
      {
        UInt uiBlkPos = puiSetScan[ iScanPos ]; 
        if( pcCoef[ uiBlkPos ] )
        {
          m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c1] );
          if( uiBin == 1 )
          {
            c1 = 0;
          }
          else if( c1 & 3 )
          {
            c1++;
          }
          pcCoef[ uiBlkPos ] = uiBin + 1;
        }
      }

      if (c1 == 0)
      {
        baseCtxMod = m_cCUAbsSCModel.get( 0, eTType ) + 5 * uiCtxSet;
        for( Int iScanPos = iLastPos; iScanPos >= 0; iScanPos-- )
        {
          UInt uiBlkPos = puiSetScan[ iScanPos ]; 
          if( pcCoef[ uiBlkPos ] == 2 ) 
          {
            m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c2] );
#if CABAC_COEFF_DATA_REORDER
            pcCoef[ uiBlkPos ] = uiBin + 2;
#else
            if( uiBin )
            {
              UInt uiLevel;
              xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
              pcCoef[ uiBlkPos ] = uiLevel + 3;
            }
#endif
            c2 += (c2 < 4);
            uiNumOne++;
          }
        }
      }

#if CABAC_COEFF_DATA_REORDER
      UInt coeffSign[SCAN_SET_SIZE];
      for( Int iScanPos = iLastPos; iScanPos >= 0; iScanPos-- )
      {
        UInt uiBlkPos = puiSetScan[ iScanPos ]; 
        if( pcCoef[ uiBlkPos ] )
        {
          m_pcTDecBinIf->decodeBinEP( coeffSign[iScanPos] );          
        }
      }
      
      if (c1 == 0)
      {
        for( Int iScanPos = iLastPos; iScanPos >= 0; iScanPos-- )
        {
          UInt uiBlkPos = puiSetScan[ iScanPos ]; 
          if( pcCoef[ uiBlkPos ] == 3 )
          {
            UInt uiLevel;
            xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
            pcCoef[ uiBlkPos ] = uiLevel + 3;            
          }
        }
      }
#endif
      for( Int iScanPos = iLastPos; iScanPos >= 0; iScanPos-- )
      {
        UInt uiBlkPos = puiSetScan[ iScanPos ]; 
        if( pcCoef[ uiBlkPos ] )
        {
#if CABAC_COEFF_DATA_REORDER
          uiBin = coeffSign[ iScanPos ];
#else
          m_pcTDecBinIf->decodeBinEP( uiBin );
#endif
          pcCoef[ uiBlkPos ] = ( uiBin ? -pcCoef[ uiBlkPos ] : pcCoef[ uiBlkPos ] );
        }
      }
    }
    else
    {
      uiNumOne >>= 1;
    }
  }
#else
  /*
   * Sign and bin0 PCP (Section 3.2 and 3.3 of JCTVC-B088)
   */
  Int  c1, c2;
  UInt uiSign;
  UInt uiLevel;
  UInt uiGoRiceParam = 0;

  if( uiNum4x4Blk > 1 )
  {
    Bool b1stBlk  = true;
    UInt uiNumOne = 0;
    
    for( UInt uiSubBlk = 0; uiSubBlk < uiNum4x4Blk && sigCoeffCount > 0; uiSubBlk++ )
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
      sigCoeffCount -= uiSubNumSig;
      
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
        ContextModel *baseCtxMod;

        baseCtxMod = m_cCUOneSCModel.get( 0, eTType ) + 5 * uiCtxSet;
        for (Int idx = 0; idx < uiSubNumSig; idx++)
        {
          m_pcTDecBinIf->decodeBin( uiLevel, baseCtxMod[c1] );
          if( uiLevel == 1 )
          {
            c1 = 0;
          }
          else if( c1 & 3)
          {
            c1++;
          }
          sigCoeff[ idx ] = uiLevel + 1;      
        }
        
        if (c1 == 0)
        {
          baseCtxMod = m_cCUAbsSCModel.get( 0, eTType ) + 5 * uiCtxSet;
          for (Int idx = 0; idx < uiSubNumSig; idx++)
          {
            if( sigCoeff[idx] == 2 )
            {
              m_pcTDecBinIf->decodeBin( uiLevel, baseCtxMod[c2] );
              
#if CABAC_COEFF_DATA_REORDER
              sigCoeff[idx] = uiLevel + 2;
#else
              if( uiLevel )
              {
                xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
                sigCoeff[idx] = uiLevel + 3;
              }
#endif
              c2 += (c2 < 4);
              
              uiNumOne++;
            }
          }
        }
        
#if CABAC_COEFF_DATA_REORDER
        UInt coeffSign[16];
        
        for (Int idx = 0; idx < uiSubNumSig; idx++)
        {
          m_pcTDecBinIf->decodeBinEP( coeffSign[idx] );
        }
        
        if (c1 == 0)
        {
          for (Int idx = 0; idx < uiSubNumSig; idx++)
          {
            if( sigCoeff[idx] == 3 )
            {
              xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
              sigCoeff[idx] = uiLevel + 3;
            }
          }
        }
#endif
        Int idx = 0;
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos & 3;
          UInt  uiIndex   = ((uiSubPosY + uiPosY) << uiLog2BlockSize) + uiSubPosX + uiPosX;
          
          uiLevel = pcCoef[ uiIndex ];
          
          if( uiLevel )
          {
#if CABAC_COEFF_DATA_REORDER
            uiSign = coeffSign[idx];
#else
            m_pcTDecBinIf->decodeBinEP( uiSign );
#endif
            TCoeff val = sigCoeff[idx++];
            pcCoef[ uiIndex ] = ( uiSign ? -(Int)val : (Int)val );
          }
        }
      }
    }
  }
  else
  {
    c1 = 1;
    c2 = 0;
    UInt uiSubNumSig = sigCoeffCount;
    TCoeff sigCoeff[16];
    ContextModel *baseCtxMod;
    
    baseCtxMod = m_cCUOneSCModel.get( 0, eTType );    
    for (Int idx = 0; idx < uiSubNumSig; idx++)
    {
      m_pcTDecBinIf->decodeBin( uiLevel, baseCtxMod[c1] );
      if( uiLevel == 1 )
      {
        c1 = 0;
      }
      else if( c1 & 3)
      {
        c1++;
      }
      sigCoeff[ idx ] = uiLevel + 1;      
    }
    
    if (c1 == 0)
    {
      baseCtxMod = m_cCUAbsSCModel.get( 0, eTType );    
      for (Int idx = 0; idx < uiSubNumSig; idx++)
      {
        if( sigCoeff[idx] == 2 )
        {
          m_pcTDecBinIf->decodeBin( uiLevel, baseCtxMod[c2] );
          
#if CABAC_COEFF_DATA_REORDER
          sigCoeff[idx] = uiLevel + 2;
#else
          if( uiLevel )
          {
            xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
            sigCoeff[idx] = uiLevel + 3;
          }
#endif
          c2 += (c2 < 4);
        }
      }
    }
    
#if CABAC_COEFF_DATA_REORDER
    UInt coeffSign[16];
    
    for (Int idx = 0; idx < uiSubNumSig; idx++)
    {
      m_pcTDecBinIf->decodeBinEP( coeffSign[idx] );
    }
    
    if (c1 == 0)
    {
      for (Int idx = 0; idx < uiSubNumSig; idx++)
      {
        if( sigCoeff[idx] == 3 )
        {
          xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
          sigCoeff[idx] = uiLevel + 3;
        }
      }
    }
#endif
    Int idx = 0;
    for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
      uiLevel = pcCoef[ uiIndex ];
      
      if( uiLevel )
      {
#if CABAC_COEFF_DATA_REORDER
        uiSign = coeffSign[idx];
#else
        m_pcTDecBinIf->decodeBinEP( uiSign );
#endif
        TCoeff val = sigCoeff[idx++];
        pcCoef[ uiIndex ] = ( uiSign ? -(Int)val : (Int)val );
      }
    }
  }
#endif
  
  return;
}

Void TDecSbac::parseAlfFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
  
  ruiVal = uiSymbol;
}

Void TDecSbac::parseAlfFlagNum( UInt& ruiVal, UInt minValue, UInt depth )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (depth*2));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  ruiVal = 0;
  UInt uiBit;
  if(uiLength)
  {
    while( uiLength-- )
    {
      m_pcTDecBinIf->decodeBinEP( uiBit );
      ruiVal += uiBit << uiLength;
    }
  }
  else
  {
    ruiVal = 0;
  }
  ruiVal += minValue;
}

Void TDecSbac::parseAlfCtrlFlag( UInt &ruiAlfCtrlFlag )
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0 ) );
  ruiAlfCtrlFlag = uiSymbol;
}

Void TDecSbac::parseAlfUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }
  
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  
  ruiVal = i;
}

Void TDecSbac::parseAlfSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  
  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }
  
  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
  
  if ( uiCode == 0 ) iSign =  1;
  else               iSign = -1;
  
  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  
  riVal = i*iSign;
}

#if MTK_SAO
Void TDecSbac::parseAoFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cAOFlagSCModel.get( 0, 0, 0 ) );

  ruiVal = uiSymbol;
}

Void TDecSbac::parseAoUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cAOUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }

  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cAOUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  ruiVal = i;
}

Void TDecSbac::parseAoSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cAOSvlcSCModel.get( 0, 0, 0 ) );

  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }

  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cAOSvlcSCModel.get( 0, 0, 1 ) );

  if ( uiCode == 0 ) iSign =  1;
  else               iSign = -1;

  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cAOSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  riVal = i*iSign;
}
#endif
