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

/** \file     TDecSbac.cpp
    \brief    Context-adaptive entropy decoder class
*/

#include "TDecSbac.h"

//! \ingroup TLibDecoder
//! \{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_pcTDecBinIf               ( NULL )
#if !PARAMSET_VLC_CLEANUP
, m_bAlfCtrl                  ( false )
, m_uiMaxAlfCtrlDepth         ( 0 )
#endif
, m_numContextModels          ( 0 )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            , m_contextModels + m_numContextModels, m_numContextModels )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeFlagExtSCModel    ( 1,             1,               NUM_MERGE_FLAG_EXT_CTX        , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeIdxExtSCModel     ( 1,             1,               NUM_MERGE_IDX_EXT_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAlfCtrlFlagSCModel     ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUDeltaQpSCModel         ( 1,             1,               NUM_DELTA_QP_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCURefPicSCModel          ( 1,             1,               NUM_REF_NO_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMvdSCModel             ( 1,             1,               NUM_MV_RES_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUQtCbfSCModel           ( 1,             2,               NUM_QT_CBF_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUTransSubdivFlagSCModel ( 1,             1,               NUM_TRANS_SUBDIV_FLAG_CTX     , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigCoeffGroupSCModel   ( 1,             2,               NUM_SIG_CG_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigSCModel             ( 1,             1,               NUM_SIG_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastX               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastY               ( 1,             2,               NUM_CTX_LAST_FLAG_XY          , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUOneSCModel             ( 1,             1,               NUM_ONE_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUAbsSCModel             ( 1,             1,               NUM_ABS_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cMVPIdxSCModel            ( 1,             1,               NUM_MVP_IDX_CTX               , m_contextModels + m_numContextModels, m_numContextModels)
, m_cALFFlagSCModel           ( 1,             1,               NUM_ALF_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cALFUvlcSCModel           ( 1,             1,               NUM_ALF_UVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cALFSvlcSCModel           ( 1,             1,               NUM_ALF_SVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#if AMP_CTX
, m_cCUAMPSCModel             ( 1,             1,               NUM_CU_AMP_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
#else
, m_cCUXPosiSCModel           ( 1,             1,               NUM_CU_X_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUYPosiSCModel           ( 1,             1,               NUM_CU_Y_POS_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cSaoFlagSCModel           ( 1,             1,               NUM_SAO_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoUvlcSCModel           ( 1,             1,               NUM_SAO_UVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoSvlcSCModel           ( 1,             1,               NUM_SAO_SVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#if SAO_UNIT_INTERLEAVING
, m_cSaoMergeLeftSCModel      ( 1,             1,               NUM_SAO_MERGE_LEFT_FLAG_CTX   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoMergeUpSCModel        ( 1,             1,               NUM_SAO_MERGE_UP_FLAG_CTX     , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoTypeIdxSCModel        ( 1,             1,               NUM_SAO_TYPE_IDX_CTX          , m_contextModels + m_numContextModels, m_numContextModels)
#endif
{
  assert( m_numContextModels <= MAX_NUM_CTX_MOD );
  m_iSliceGranularity = 0;
}

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecSbac::resetEntropywithQPandInitIDC (Int  iQp, Int iID)
{
  SliceType eSliceType = (SliceType)iID;

  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_ALF_CTRL_FLAG );
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PART_SIZE );
#if AMP_CTX
  m_cCUAMPSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_CU_AMP_POS );
#else
  m_cCUXPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_Y_POS );
#endif
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (UChar*)INIT_REF_PIC );
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (UChar*)INIT_DQP );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_QT_ROOT_CBF );
  m_cCUSigCoeffGroupSCModel.initBuffer   ( eSliceType, iQp, (UChar*)INIT_SIG_CG_FLAG );
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG );
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG );
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (UChar*)INIT_MVP_IDX );
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_SVLC );
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_SVLC );
#if SAO_UNIT_INTERLEAVING
  m_cSaoMergeLeftSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_SAO_MERGE_LEFT_FLAG );
  m_cSaoMergeUpSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SAO_MERGE_UP_FLAG );
  m_cSaoTypeIdxSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SAO_TYPE_IDX );
#endif

  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
  m_uiLastDQpNonZero  = 0;
  
  // new structure
  m_uiLastQp          = iQp;
  
  m_pcTDecBinIf->start();
}

/** The function does the following: Read out terminate bit. Flush CABAC. Byte-align for next tile.
 *  Intialize CABAC states. Start CABAC.
 */
Void TDecSbac::updateContextTables( SliceType eSliceType, Int iQp )
{
  UInt uiBit;
  m_pcTDecBinIf->decodeBinTrm(uiBit);
  m_pcTDecBinIf->finish();  
#if !OL_FLUSH_ALIGN
  // Account for misaligned CABAC.
  Int iCABACReadAhead = m_pcTDecBinIf->getBitsReadAhead();
  iCABACReadAhead--;
  Int iStreamBits = 8-m_pcBitstream->getNumBitsUntilByteAligned();
  if (iCABACReadAhead >= iStreamBits)
  {
    // Misaligned CABAC has read into the 1st byte of the next tile.
    // Back up a byte prior to alignment.
    m_pcBitstream->backupByte();
  }
#endif
  m_pcBitstream->readOutTrailingBits();
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
  m_cCUMergeFlagExtSCModel.initBuffer    ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT );
  m_cCUMergeIdxExtSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT );
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (UChar*)INIT_ALF_CTRL_FLAG );
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PART_SIZE );
#if AMP_CTX
  m_cCUAMPSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_CU_AMP_POS );
#else
  m_cCUXPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_X_POS );
  m_cCUYPosiSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_CU_Y_POS );
#endif
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (UChar*)INIT_REF_PIC );
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (UChar*)INIT_DQP );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (UChar*)INIT_QT_ROOT_CBF );
  m_cCUSigCoeffGroupSCModel.initBuffer   ( eSliceType, iQp, (UChar*)INIT_SIG_CG_FLAG );
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG );
  m_cCuCtxLastX.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCuCtxLastY.initBuffer               ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG );
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (UChar*)INIT_MVP_IDX );
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_ALF_SVLC );
  m_cSaoFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_FLAG );
  m_cSaoUvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_UVLC );
  m_cSaoSvlcSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_SAO_SVLC );
#if SAO_UNIT_INTERLEAVING
  m_cSaoMergeLeftSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_SAO_MERGE_LEFT_FLAG );
  m_cSaoMergeUpSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SAO_MERGE_UP_FLAG );
  m_cSaoTypeIdxSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_SAO_TYPE_IDX );
#endif
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
  m_pcTDecBinIf->start();
}

Void TDecSbac::readTileMarker( UInt& uiTileIdx, UInt uiBitsUsed )
{
  UInt uiSymbol;
  uiTileIdx = 0;
  for (Int iShift=uiBitsUsed-1; iShift>=0; iShift--)
  {
    m_pcTDecBinIf->decodeBinEP ( uiSymbol );
    if (uiSymbol)
    {
      uiTileIdx |= (1<<iShift);
    }
  }
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

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  while( uiBit )
  {
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  if ( --uiCount )
  {
    UInt bins;
    m_pcTDecBinIf->decodeBinsEP( bins, uiCount );
    uiSymbol += bins;
  }
  
  ruiSymbol = uiSymbol;
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

  if ( ruiGoRiceParam > 0 )
  {
    m_pcTDecBinIf->decodeBinsEP( uiRemainder, ruiGoRiceParam );    
  }

  ruiSymbol      = uiRemainder + ( uiQuotient << ruiGoRiceParam );
  bExGolomb      = ruiSymbol == ( uiMaxVlc + 1 );

  if( bExGolomb )
  {
    xReadEpExGolomb( uiCodeWord, 0 );
    ruiSymbol += uiCodeWord;
  }

#if EIGHT_BITS_RICE_CODE
  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( ruiSymbol, 23 ) ];
#else
  ruiGoRiceParam = g_aauiGoRiceUpdate[ ruiGoRiceParam ][ min<UInt>( ruiSymbol, 15 ) ];
#endif

  return;
}


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
#if BURST_IPCM
  Int numSubseqIPCM = 0;
  Bool readPCMSampleFlag = false;

  if(pcCU->getNumSucIPCM() > 0) 
  {
    readPCMSampleFlag = true;
  }
  else
  {
    m_pcTDecBinIf->decodeBinTrm(uiSymbol);

    if (uiSymbol)
    {
      readPCMSampleFlag = true;
      m_pcTDecBinIf->decodeNumSubseqIPCM(numSubseqIPCM);
      pcCU->setNumSucIPCM(numSubseqIPCM + 1);
      m_pcTDecBinIf->decodePCMAlignBits();
    }
  }
#else
  m_pcTDecBinIf->decodeBinTrm(uiSymbol);
#endif

#if BURST_IPCM
  if (readPCMSampleFlag == true)
#else
  if (uiSymbol)
#endif
  {
    Bool bIpcmFlag = true;

#if !BURST_IPCM
    m_pcTDecBinIf->decodePCMAlignBits();
#endif

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
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();

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
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

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
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

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

#if BURST_IPCM
    pcCU->setNumSucIPCM( pcCU->getNumSucIPCM() - 1);
    if(pcCU->getNumSucIPCM() == 0)
    {
      m_pcTDecBinIf->resetBac();
    }
#else
    m_pcTDecBinIf->resetBac();
#endif
  }
}

#if !PARAMSET_VLC_CLEANUP
Void TDecSbac::parseAlfCtrlDepth( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol( uiSymbol, m_cALFUvlcSCModel.get( 0 ), 1, g_uiMaxCUDepth - 1 );
  ruiAlfCtrlDepth = uiSymbol;
}
#endif

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
    
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      pcCU->setMergeFlagSubParts( true , uiAbsPartIdx, 0, uiDepth );
    } 
    else
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
        {
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        {
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
      }
      else
      {
        pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );
        
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        {
          pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
        if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        {
          pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
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
  m_pcTDecBinIf->decodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiPUIdx, uiDepth );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tMergeFlag: " );
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\tAddress: " );
  DTRACE_CABAC_V( pcCU->getAddr() );
  DTRACE_CABAC_T( "\tuiAbsPartIdx: " );
  DTRACE_CABAC_V( uiAbsPartIdx );
  DTRACE_CABAC_T( "\n" );
}

Void TDecSbac::parseMergeIndex ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiNumCand = MRG_MAX_NUM_CANDS;
#if !MRG_IDX_CTX_RED  
  UInt auiCtx[4] = { 0, 1, 2, 3 };
#endif
  UInt uiUnaryIdx = 0;
  uiNumCand = pcCU->getSlice()->getMaxNumMergeCand();
  if ( uiNumCand > 1 )
  {
    for( ; uiUnaryIdx < uiNumCand - 1; ++uiUnaryIdx )
    {
      UInt uiSymbol = 0;
#if MRG_IDX_CTX_RED
      if ( uiUnaryIdx==0 )
      {
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, 0 ) );
      }
      else
      {
        m_pcTDecBinIf->decodeBinEP( uiSymbol );
      }
#else
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, auiCtx[uiUnaryIdx] ) );
#endif
      if( uiSymbol == 0 )
      {
        break;
      }
    }
  }
  ruiMergeIndex = uiUnaryIdx;

  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseMergeIndex()" )
  DTRACE_CABAC_T( "\tuiMRGIdx= " )
  DTRACE_CABAC_V( ruiMergeIndex )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseMVPIdx      ( Int& riMVPIdx )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, AMVP_MAX_NUM_CANDS-1);
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
    UInt uiTrLevel = 0;    
    UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
    UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
    uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
    if( eMode == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1+uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
  }
  else
  {
    UInt uiMaxNumBits = 2;
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth && !( pcCU->getSlice()->getSPS()->getDisInter4x4() && (g_uiMaxCUWidth>>uiDepth) == 8 && (g_uiMaxCUHeight>>uiDepth) == 8 ) )
    {
      uiMaxNumBits ++;
    }
    for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
      if ( uiSymbol )
      {
        break;
      }
      uiMode++;
    }
    eMode = (PartSize) uiMode;
    if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
    {
      if (eMode == SIZE_2NxN)
      {
#if AMP_CTX
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUAMPSCModel.get( 0, 0, 0 ));
#else
        m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 0 ));
#endif
        if (uiSymbol == 0)
        {
#if AMP_CTX
          m_pcTDecBinIf->decodeBinEP(uiSymbol);
#else
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUYPosiSCModel.get( 0, 0, 1 ));
#endif
          eMode = (uiSymbol == 0? SIZE_2NxnU : SIZE_2NxnD);
        }
      }
      else if (eMode == SIZE_Nx2N)
      {
#if AMP_CTX
        m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUAMPSCModel.get( 0, 0, 0 ));
#else
        m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 0 ));
#endif
        if (uiSymbol == 0)
        {
#if AMP_CTX
          m_pcTDecBinIf->decodeBinEP(uiSymbol);
#else
          m_pcTDecBinIf->decodeBin(uiSymbol, m_cCUXPosiSCModel.get( 0, 0, 1 ));
#endif
          eMode = (uiSymbol == 0? SIZE_nLx2N : SIZE_nRx2N);
        }
      }
    }
  }
  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
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
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
  iPredMode += uiSymbol;
  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}
  
Void TDecSbac::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if !LOGI_INTRA_NAME_3MPM
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
#endif
  
  UInt uiSymbol;
  Int  intraPredMode;

#if LOGI_INTRA_NAME_3MPM
  Int uiPreds[3] = {-1, -1, -1};
#else
  Int uiPreds[2] = {-1, -1};
#endif
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  
 
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
 
  if ( uiSymbol )
  {
    m_pcTDecBinIf->decodeBinEP( uiSymbol );
#if LOGI_INTRA_NAME_3MPM
    if (uiSymbol)
    {
      m_pcTDecBinIf->decodeBinEP( uiSymbol );
      uiSymbol++;
    }
#endif
    intraPredMode = uiPreds[uiSymbol];
  }
  else
  {
    intraPredMode = 0;
    
#if LOGI_INTRA_NAME_3MPM
   
    m_pcTDecBinIf->decodeBinsEP( uiSymbol, 5 );
    intraPredMode = uiSymbol;
    
   //postponed sorting of MPMs (only in remaining branch)
    if (uiPreds[0] > uiPreds[1])
    { 
      std::swap(uiPreds[0], uiPreds[1]); 
    }
    if (uiPreds[0] > uiPreds[2])
    {
      std::swap(uiPreds[0], uiPreds[2]);
    }
    if (uiPreds[1] > uiPreds[2])
    {
      std::swap(uiPreds[1], uiPreds[2]);
    }
#else
    m_pcTDecBinIf->decodeBinsEP( uiSymbol, g_aucIntraModeBitsAng[iIntraIdx] - 1 );
    intraPredMode = uiSymbol;
    
    if ( intraPredMode == 31 )
    {
      m_pcTDecBinIf->decodeBinEP( uiSymbol );
      intraPredMode += uiSymbol;      
    }
#endif
    for ( Int i = 0; i < uiPredNum; i++ )
    {
      intraPredMode += ( intraPredMode >= uiPreds[i] );
    }
  }
  
  pcCU->setLumaIntraDirSubParts( (UChar)intraPredMode, uiAbsPartIdx, uiDepth );
}
Void TDecSbac::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
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
#if CHROMA_MODE_CODING
      m_pcTDecBinIf->decodeBinsEP( uiIPredMode, 2 );
#else
      xReadUnaryMaxSymbol( uiIPredMode, m_cCUChromaPredSCModel.get( 0, 0 ) + 1, 0, 3 );
#endif
      UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
      pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );
      uiSymbol = uiAllowedChromaDir[ uiIPredMode ];
    }
  }
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  return;
}

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );
  ContextModel *pCtx = m_cCUInterDirSCModel.get( 0 );
  m_pcTDecBinIf->decodeBin( uiSymbol, *( pCtx + uiCtx ) );

  if( uiSymbol )
  {
    uiSymbol = 2;
  }

  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecSbac::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;

  if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C ) > 0 && eRefList==REF_PIC_LIST_C)
  {
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C )-2 );
      uiSymbol++;
    }
    riRefFrmIdx = uiSymbol;
  }
  else
  {
    ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
    m_pcTDecBinIf->decodeBin( uiSymbol, *pCtx );

    if( uiSymbol )
    {
      xReadUnaryMaxSymbol( uiSymbol, pCtx + 1, 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
      uiSymbol++;
    }
    riRefFrmIdx = uiSymbol;
  }

  return;
}

Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  UInt uiHorAbs;
  UInt uiVerAbs;
  UInt uiHorSign = 0;
  UInt uiVerSign = 0;
  ContextModel *pCtx = m_cCUMvdSCModel.get( 0 );

#if H0111_MVD_L1_ZERO
  if(pcCU->getSlice()->getMvdL1ZeroFlag() && eRefList == REF_PIC_LIST_1 && pcCU->getInterDir(uiAbsPartIdx)==3)
  {
    uiHorAbs=0;
    uiVerAbs=0;
  }
  else
  {
#endif

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

#if H0111_MVD_L1_ZERO
  }
#endif

  const TComMv cMv( uiHorSign ? -Int( uiHorAbs ): uiHorAbs, uiVerSign ? -Int( uiVerAbs ) : uiVerAbs );
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth, uiPartIdx );
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
  const UInt uiCtx = 0;
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
#if H0736_AVC_STYLE_QP_RANGE
  Int qp;
#endif
  UInt uiDQp;
  Int  iDQp;
  
  m_pcTDecBinIf->decodeBin( uiDQp, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  
  if ( uiDQp == 0 )
  {
#if H0736_AVC_STYLE_QP_RANGE
    qp = pcCU->getRefQP(uiAbsPartIdx);
#else
    uiDQp = pcCU->getRefQP(uiAbsPartIdx);
#endif
  }
  else
  {
    UInt uiSign;
#if H0736_AVC_STYLE_QP_RANGE
    Int qpBdOffsetY = pcCU->getSlice()->getSPS()->getQpBDOffsetY();
#else
    UInt uiQpBdOffsetY = 6*(g_uiBitIncrement + g_uiBitDepth - 8);
#endif
    m_pcTDecBinIf->decodeBinEP(uiSign);

#if H0736_AVC_STYLE_QP_RANGE
    UInt uiMaxAbsDQpMinus1 = 24 + (qpBdOffsetY/2) + (uiSign);
#else
    UInt uiMaxAbsDQpMinus1 = 24 + (uiQpBdOffsetY/2) + (uiSign);
#endif
    UInt uiAbsDQpMinus1;
    xReadUnaryMaxSymbol (uiAbsDQpMinus1,  &m_cCUDeltaQpSCModel.get( 0, 0, 1 ), 1, uiMaxAbsDQpMinus1);

    iDQp = uiAbsDQpMinus1 + 1;

    if(uiSign)
    {
      iDQp = -iDQp;
    }

#if H0736_AVC_STYLE_QP_RANGE
    qp = (((Int) pcCU->getRefQP( uiAbsPartIdx ) + iDQp + 52 + 2*qpBdOffsetY )%(52+qpBdOffsetY)) - qpBdOffsetY;
#else
    uiDQp = pcCU->getRefQP(uiAbsPartIdx) + iDQp;
#endif
  }
  
  UInt uiAbsQpCUPartIdx = (uiAbsPartIdx>>(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)) ;
  UInt uiQpCUDepth =   min(uiDepth,pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()) ;
#if H0736_AVC_STYLE_QP_RANGE
  pcCU->setQPSubParts( qp, uiAbsQpCUPartIdx, uiQpCUDepth );
#else
  pcCU->setQPSubParts( uiDQp, uiAbsQpCUPartIdx, uiQpCUDepth );
#endif
}

Void TDecSbac::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? TEXT_CHROMA: eType, uiCtx ) );
  
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
 * \param width  Block width
 * \param height Block height
 * \param eTType plane type / luminance or chrominance
 * \param uiScanIdx scan type (zig-zag, hor, ver)
 *
 * This method decodes the X and Y component within a block of the last significant coefficient.
 */
Void TDecSbac::parseLastSignificantXY( UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, TextType eTType, UInt uiScanIdx )
{
  UInt uiLast;
  ContextModel *pCtxX = m_cCuCtxLastX.get( 0, eTType );
  ContextModel *pCtxY = m_cCuCtxLastY.get( 0, eTType );

  // posX
#if LAST_CTX_REDUCTION
  Int widthCtx = eTType ? 4 : width;
  const UInt *puiCtxIdxX = g_uiLastCtx + ( g_aucConvertToBit[ widthCtx ] * ( g_aucConvertToBit[ widthCtx ] + 3 ) );
#else
  const UInt *puiCtxIdxX = g_uiLastCtx + ( g_aucConvertToBit[ width ] * ( g_aucConvertToBit[ width ] + 3 ) );
#endif
  for( uiPosLastX = 0; uiPosLastX < g_uiGroupIdx[ width - 1 ]; uiPosLastX++ )
  {
#if LAST_CTX_REDUCTION
    if ( eTType  )
    {
      m_pcTDecBinIf->decodeBin( uiLast, *( pCtxX + (uiPosLastX>>g_aucConvertToBit[ width ])  ) );
    }
    else
    {
#endif
      m_pcTDecBinIf->decodeBin( uiLast, *( pCtxX + puiCtxIdxX[ uiPosLastX ] ) );
#if LAST_CTX_REDUCTION
    }
#endif
    if( !uiLast )
    {
      break;
    }
  }

  // posY
#if LAST_CTX_REDUCTION
  Int heightCtx = eTType? 4 : height;
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ heightCtx ] * ( g_aucConvertToBit[ heightCtx ] + 3 ) );
#else
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ height ] * ( g_aucConvertToBit[ height ] + 3 ) );
#endif
  for( uiPosLastY = 0; uiPosLastY < g_uiGroupIdx[ height - 1 ]; uiPosLastY++ )
  {
#if LAST_CTX_REDUCTION
    if (eTType)
    {
      m_pcTDecBinIf->decodeBin( uiLast, *( pCtxY + (uiPosLastY>>g_aucConvertToBit[ height ]) ) );
    }
    else
    {
#endif
      m_pcTDecBinIf->decodeBin( uiLast, *( pCtxY + puiCtxIdxY[ uiPosLastY ] ) );
#if LAST_CTX_REDUCTION
    }
#endif
    if( !uiLast )
    {
      break;
    }
  }
  if ( uiPosLastX > 3 )
  {
    UInt uiTemp  = 0;
    UInt uiCount = ( uiPosLastX - 2 ) >> 1;
    for ( Int i = uiCount - 1; i >= 0; i-- )
    {
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastX = g_uiMinInGroup[ uiPosLastX ] + uiTemp;
  }
  if ( uiPosLastY > 3 )
  {
    UInt uiTemp  = 0;
    UInt uiCount = ( uiPosLastY - 2 ) >> 1;
    for ( Int i = uiCount - 1; i >= 0; i-- )
    {
      m_pcTDecBinIf->decodeBinEP( uiLast );
      uiTemp += uiLast << i;
    }
    uiPosLastY = g_uiMinInGroup[ uiPosLastY ] + uiTemp;
  }
  
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosLastX, uiPosLastY );
  }
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
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- parse significance map -----
  const UInt  uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt  uiMaxNumCoeff     = uiWidth * uiHeight;
  const UInt  uiMaxNumCoeffM1   = uiMaxNumCoeff - 1;
  UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
  int blockType = uiLog2BlockSize;
  if (uiWidth != uiHeight)
  {
    uiScanIdx = SCAN_DIAG;
    blockType = 4;
  }
  
  //===== decode last significant =====
  UInt uiPosLastX, uiPosLastY;
  parseLastSignificantXY( uiPosLastX, uiPosLastY, uiWidth, uiHeight, eTType, uiScanIdx );
  UInt uiBlkPosLast      = uiPosLastX + (uiPosLastY<<uiLog2BlockSize);
  pcCoef[ uiBlkPosLast ] = 1;

  //===== decode significance flags =====
  UInt uiScanPosLast   = uiBlkPosLast;
  if (uiScanIdx == SCAN_ZIGZAG)
  {
    // Map zigzag to diagonal scan
    uiScanIdx = SCAN_DIAG;
  }
  const UInt * scan;
  if (uiWidth == uiHeight)
  {
    scan = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize-1 ];
  }
  else
  {
    scan = g_sigScanNSQT[ uiLog2BlockSize - 2 ];
  }
  for( uiScanPosLast = 0; uiScanPosLast < uiMaxNumCoeffM1; uiScanPosLast++ )
  {
    UInt uiBlkPos = scan[ uiScanPosLast ];
    if( uiBlkPosLast == uiBlkPos )
    {
      break;
    }
  }

  ContextModel * const baseCoeffGroupCtx = m_cCUSigCoeffGroupSCModel.get( 0, eTType );
  ContextModel * const baseCtx = (eTType==TEXT_LUMA) ? m_cCUSigSCModel.get( 0, 0 ) : m_cCUSigSCModel.get( 0, 0 ) + NUM_SIG_FLAG_CTX_LUMA;

  const Int  iLastScanSet      = uiScanPosLast >> LOG2_SCAN_SET_SIZE;
  UInt uiNumOne                = 0;
  UInt uiGoRiceParam           = 0;

#if MULTIBITS_DATA_HIDING
  UInt const tsig = pcCU->getSlice()->getPPS()->getTSIG();
  Bool beValid = pcCU->getSlice()->getPPS()->getSignHideFlag() > 0;
  UInt absSum = 0;
#endif  // MULTIBITS_DATA_HIDING

  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];
  ::memset( uiSigCoeffGroupFlag, 0, sizeof(UInt) * MLS_GRP_NUM );
  const UInt uiNumBlkSide = uiWidth >> (MLS_CG_SIZE >> 1);
  const UInt * scanCG;
  if (uiWidth == uiHeight)
  {
    scanCG = g_auiSigLastScan[ uiScanIdx ][ uiLog2BlockSize > 3 ? uiLog2BlockSize-2-1 : 0  ];    
#if MULTILEVEL_SIGMAP_EXT
    if( uiLog2BlockSize == 3 )
    {
      scanCG = g_sigLastScan8x8[ uiScanIdx ];
    }
    else if( uiLog2BlockSize == 5 )
    {
      scanCG = g_sigLastScanCG32x32;
    }
#endif
  }
  else
  {
    scanCG = g_sigCGScanNSQT[ uiLog2BlockSize - 2 ];
  }
  Int  iScanPosSig             = (Int) uiScanPosLast;
  for( Int iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- )
  {
    Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
    uiGoRiceParam    = 0;
    Int numNonZero = 0;
    
#if MULTIBITS_DATA_HIDING
    Int lastNZPosInCG = -1, firstNZPosInCG = SCAN_SET_SIZE;
#endif

    Int pos[SCAN_SET_SIZE];
    if( iScanPosSig == (Int) uiScanPosLast )
    {
#if MULTIBITS_DATA_HIDING
      lastNZPosInCG  = iScanPosSig;
      firstNZPosInCG = iScanPosSig;
#endif
      iScanPosSig--;
      pos[ numNonZero ] = uiBlkPosLast;
      numNonZero = 1;
    }

#if !MULTILEVEL_SIGMAP_EXT
    if( blockType > 3 )
    {
#endif
      // decode significant_coeffgroup_flag
      Int iCGBlkPos = scanCG[ iSubSet ];
      Int iCGPosY   = iCGBlkPos / uiNumBlkSide;
      Int iCGPosX   = iCGBlkPos - (iCGPosY * uiNumBlkSide);
#if MULTILEVEL_SIGMAP_EXT
      if( uiWidth == 8 && uiHeight == 8 && (uiScanIdx == SCAN_HOR || uiScanIdx == SCAN_VER) )
      {
        iCGPosY = (uiScanIdx == SCAN_HOR ? iCGBlkPos : 0);
        iCGPosX = (uiScanIdx == SCAN_VER ? iCGBlkPos : 0);
      }
#endif
#if !REMOVE_INFER_SIGGRP
      Bool bInferredCGFlag = false;
#endif
#if REMOVE_INFER_SIGGRP
      if( iSubSet == iLastScanSet || iSubSet == 0)
#else
      if( iSubSet == iLastScanSet ) 
#endif
      {
        uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
      }
      else
      {
#if !REMOVE_INFER_SIGGRP
#if MULTILEVEL_SIGMAP_EXT
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiScanIdx, uiWidth, uiHeight) && ( iSubSet ) )
#else
        if( !TComTrQuant::bothCGNeighboursOne( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight) && ( iSubSet ) )
#endif
        {
#endif
          UInt uiSigCoeffGroup;
#if MULTILEVEL_SIGMAP_EXT
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiScanIdx, uiWidth, uiHeight );
#else
          UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, uiWidth, uiHeight );
#endif
          m_pcTDecBinIf->decodeBin( uiSigCoeffGroup, baseCoeffGroupCtx[ uiCtxSig ] );
          uiSigCoeffGroupFlag[ iCGBlkPos ] = uiSigCoeffGroup;
#if !REMOVE_INFER_SIGGRP
        }
        else
        {
          uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
          bInferredCGFlag = true;
        }
#endif
      }

      // decode significant_coeff_flag
      UInt uiBlkPos, uiPosY, uiPosX, uiSig, uiCtxSig;
      for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
      {
        uiBlkPos  = scan[ iScanPosSig ];
        uiPosY    = uiBlkPos >> uiLog2BlockSize;
        uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
        uiSig     = 0;
        
        if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
        {
#if REMOVE_INFER_SIGGRP
          if( iScanPosSig > iSubPos || iSubSet == 0  || numNonZero )
#else
          if( iScanPosSig > iSubPos || bInferredCGFlag || numNonZero )
#endif
          {
            uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
            m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
          }
          else
          {
            uiSig = 1;
          }
        }
        pcCoef[ uiBlkPos ] = uiSig;
        if( uiSig )
        {
          pos[ numNonZero ] = uiBlkPos;
          numNonZero ++;
#if MULTIBITS_DATA_HIDING
          if( lastNZPosInCG == -1 )
          {
            lastNZPosInCG = iScanPosSig;
          }
          firstNZPosInCG = iScanPosSig;
#endif
        }
      }
#if !MULTILEVEL_SIGMAP_EXT
    }
    else
    {
      for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
      {
        UInt uiBlkPos   = scan[ iScanPosSig ];
        UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
        UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
        UInt  uiSig     = 0;
        UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, blockType, uiWidth, uiHeight, eTType );
        m_pcTDecBinIf->decodeBin( uiSig, baseCtx[ uiCtxSig ] );
        pcCoef[ uiBlkPos ] = uiSig;
        if( uiSig )
        {
          pos[ numNonZero ] = uiBlkPos;
          numNonZero ++;
#if MULTIBITS_DATA_HIDING
          if( lastNZPosInCG == -1 )
          {
            lastNZPosInCG = iScanPosSig;
          }
          firstNZPosInCG = iScanPosSig;
#endif
        }
      }
    }
#endif

    
    if( numNonZero )
    {
#if MULTIBITS_DATA_HIDING
      Bool signHidden = ( lastNZPosInCG - firstNZPosInCG >= (Int)tsig );
      absSum = 0;
#endif  // MULTIBITS_DATA_HIDING

      UInt c1 = 1;
#if !RESTRICT_GR1GR2FLAG_NUMBER
      UInt c2 = 0;
#endif
#if LEVEL_CTX_LUMA_RED
      UInt uiCtxSet    = (iSubSet > 0 && eTType==TEXT_LUMA) ? 2 : 0;
#else
      UInt uiCtxSet    = (iSubSet > 0 && eTType==TEXT_LUMA) ? 3 : 0;
#endif
      UInt uiBin;
      
      if( uiNumOne > 0 )
      {
        uiCtxSet++;
#if !LEVEL_CTX_LUMA_RED
        if(eTType==TEXT_LUMA && uiNumOne > 3)
        {
          uiCtxSet++;
        }
#endif
      }
      
      uiNumOne       >>= 1;
      ContextModel *baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUOneSCModel.get( 0, 0 ) + 4 * uiCtxSet : m_cCUOneSCModel.get( 0, 0 ) + NUM_ONE_FLAG_CTX_LUMA + 4 * uiCtxSet;
      Int absCoeff[SCAN_SET_SIZE];

#if RESTRICT_GR1GR2FLAG_NUMBER
      for ( Int i = 0; i < numNonZero; i++) absCoeff[i] = 1;   
      Int numC1Flag = min(numNonZero, C1FLAG_NUMBER);
      Int firstC2FlagIdx = -1;

      for( Int idx = 0; idx < numC1Flag; idx++ )
#else
      for( Int idx = 0; idx < numNonZero; idx++ )
#endif
      {
        m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c1] );
        if( uiBin == 1 )
        {
          c1 = 0;
#if RESTRICT_GR1GR2FLAG_NUMBER
          if (firstC2FlagIdx == -1)
          {
            firstC2FlagIdx = idx;
          }
#endif
        }
        else if( (c1 < 3) && (c1 > 0) )
        {
          c1++;
        }
        absCoeff[ idx ] = uiBin + 1;
      }
      
      if (c1 == 0)
      {
#if RESTRICT_GR1GR2FLAG_NUMBER
        baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUAbsSCModel.get( 0, 0 ) + uiCtxSet : m_cCUAbsSCModel.get( 0, 0 ) + NUM_ABS_FLAG_CTX_LUMA + uiCtxSet;
        if ( firstC2FlagIdx != -1)
        {
          m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[0] ); 
          absCoeff[ firstC2FlagIdx ] = uiBin + 2;
        }
#else    
        baseCtxMod = ( eTType==TEXT_LUMA ) ? m_cCUAbsSCModel.get( 0, 0 ) + 3 * uiCtxSet : m_cCUAbsSCModel.get( 0, 0 ) + NUM_ABS_FLAG_CTX_LUMA + 3 * uiCtxSet;
        for( Int idx = 0; idx < numNonZero; idx++ )
        {
          if( absCoeff[ idx ] == 2 ) 
          {
            m_pcTDecBinIf->decodeBin( uiBin, baseCtxMod[c2] );
            absCoeff[ idx ] = uiBin + 2;
            c2 += (c2 < 2);
            uiNumOne++;
          }
        }
#endif
      }

#if MULTIBITS_DATA_HIDING
      UInt coeffSigns;
      if ( signHidden && beValid )
      {
        m_pcTDecBinIf->decodeBinsEP( coeffSigns, numNonZero-1 );
        coeffSigns <<= 32 - (numNonZero-1);
      }
      else
      {
        m_pcTDecBinIf->decodeBinsEP( coeffSigns, numNonZero );
        coeffSigns <<= 32 - numNonZero;
      }
#else
      UInt coeffSigns;
      m_pcTDecBinIf->decodeBinsEP( coeffSigns, numNonZero );
      coeffSigns <<= 32 - numNonZero;
#endif
      
#if RESTRICT_GR1GR2FLAG_NUMBER
      Int iFirstCoeff2 = 1;    
      if (c1 == 0 || numNonZero > C1FLAG_NUMBER)
#else
      if (c1 == 0)
#endif
      {
        for( Int idx = 0; idx < numNonZero; idx++ )
        {
#if RESTRICT_GR1GR2FLAG_NUMBER   
          UInt baseLevel  = (idx < C1FLAG_NUMBER)? (2 + iFirstCoeff2) : 1;

          if( absCoeff[ idx ] == baseLevel)
          {
            UInt uiLevel;
            xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
            absCoeff[ idx ] = uiLevel + baseLevel;
          }

          if(absCoeff[ idx ] >= 2)  
          {
            iFirstCoeff2 = 0;
            uiNumOne++;
          }
#else
          if( absCoeff[ idx ] == 3 )
          {
            UInt uiLevel;
            xReadGoRiceExGolomb( uiLevel, uiGoRiceParam );
            absCoeff[ idx ] = uiLevel + 3;
          }
#endif
        }
      }

      for( Int idx = 0; idx < numNonZero; idx++ )
      {
        Int blkPos = pos[ idx ];
#if MULTIBITS_DATA_HIDING
        // Signs applied later.
        pcCoef[ blkPos ] = absCoeff[ idx ];
        absSum += absCoeff[ idx ];

        if ( idx == numNonZero-1 && signHidden && beValid )
        {
          // Infer sign of 1st element.
          if (absSum&0x1)
            pcCoef[ blkPos ] = -pcCoef[ blkPos ];
        }
        else
        {
          Int sign = static_cast<Int>( coeffSigns ) >> 31;
          pcCoef[ blkPos ] = ( pcCoef[ blkPos ] ^ sign ) - sign;
          coeffSigns <<= 1;
        }
#else
        Int sign = static_cast<Int>( coeffSigns ) >> 31;
        pcCoef[ blkPos ] = ( absCoeff[ idx ] ^ sign ) - sign;
        coeffSigns <<= 1;
#endif
      }
    }
    else
    {
      uiNumOne >>= 1;
    }
  }
  
  return;
}

#if !PARAMSET_VLC_CLEANUP
Void TDecSbac::parseAlfFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
  
  ruiVal = uiSymbol;
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
    if ( uiCode == 0 )
    {
      break;
    }
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
  
  if ( uiCode == 0 )
  {
    iSign =  1;
  }
  else
  {
    iSign = -1; 
  }
  
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

Void TDecSbac::parseSaoFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cSaoFlagSCModel.get( 0, 0, 0 ) );

  ruiVal = uiSymbol;
}

Void TDecSbac::parseSaoUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }

  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  ruiVal = i;
}

Void TDecSbac::parseSaoSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 0 ) );

  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }

  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 1 ) );

  if ( uiCode == 0 )
  {
    iSign =  1;
  }
  else
  {
    iSign = -1;
  }
  
  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  riVal = i*iSign;
}
#endif


#if SAO_UNIT_INTERLEAVING
Void TDecSbac::parseSaoUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }

  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  ruiVal = i;
}

Void TDecSbac::parseSaoSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;

  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 0 ) );

  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }

  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 1 ) );

  if ( uiCode == 0 )
  {
    iSign =  1;
  }
  else
  {
    iSign = -1;
  }

  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }

  riVal = i*iSign;
}

Void TDecSbac::parseSaoUflc (UInt&  riVal)
{
  UInt uiSymbol;
  riVal = 0;
  for (Int i=0;i<5;i++)
  {
    m_pcTDecBinIf->decodeBinEP ( uiSymbol );
    if (uiSymbol)
    {
      riVal |= (1<<i);
    }
  }
}
Void TDecSbac::parseSaoMergeLeft (UInt&  ruiVal, UInt uiCompIdx)
{
  UInt uiCode;
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoMergeLeftSCModel.get( 0, 0, uiCompIdx ) );
  ruiVal = (Int)uiCode;
}

Void TDecSbac::parseSaoMergeUp (UInt&  ruiVal)
{
  UInt uiCode;
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoMergeUpSCModel.get( 0, 0, 0 ) );
  ruiVal = (Int)uiCode;
}
Void TDecSbac::parseSaoTypeIdx (UInt&  ruiVal)
{
  UInt uiCode;
  Int  i;
  m_pcTDecBinIf->decodeBin( uiCode, m_cSaoTypeIdxSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cSaoTypeIdxSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  ruiVal = i;
}

inline Void copySaoOneLcuParam(SaoLcuParam* psDst,  SaoLcuParam* psSrc)
{
  Int i;
  psDst->partIdx = psSrc->partIdx;
  psDst->typeIdx    = psSrc->typeIdx;
  if (psDst->typeIdx != -1)
  {
    if (psDst->typeIdx == SAO_BO)
    {
      psDst->bandPosition = psSrc->bandPosition ;
    }
    else
    {
      psDst->bandPosition = 0;
    }
    psDst->length  = psSrc->length;
    for (i=0;i<psDst->length;i++)
    {
      psDst->offset[i] = psSrc->offset[i];
    }
  }
  else
  {
    psDst->length  = 0;
    for (i=0;i<SAO_BO_LEN;i++)
    {
      psDst->offset[i] = 0;
    }
  }
}
Void TDecSbac::parseSaoOffset(SaoLcuParam* psSaoLcuParam)
{
  UInt uiSymbol;
  Int iSymbol;
  static Int iTypeLength[MAX_NUM_SAO_TYPE] = {
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_BO_LEN
  }; 

  parseSaoTypeIdx(uiSymbol);
  psSaoLcuParam->typeIdx = (Int)uiSymbol - 1;
  if (uiSymbol)
  {
    psSaoLcuParam->length = iTypeLength[psSaoLcuParam->typeIdx];
    if( psSaoLcuParam->typeIdx == SAO_BO )
    {
      // Parse Left Band Index
      parseSaoUflc( uiSymbol );
      psSaoLcuParam->bandPosition = uiSymbol;
      for(Int i=0; i< psSaoLcuParam->length; i++)
      {
        parseSaoSvlc(iSymbol);
        psSaoLcuParam->offset[i] = iSymbol;
      }   
    }
    else if( psSaoLcuParam->typeIdx < 4 )
    {
      parseSaoUvlc(uiSymbol); psSaoLcuParam->offset[0] = uiSymbol;
      parseSaoUvlc(uiSymbol); psSaoLcuParam->offset[1] = uiSymbol;
      parseSaoUvlc(uiSymbol); psSaoLcuParam->offset[2] = -(Int)uiSymbol;
      parseSaoUvlc(uiSymbol); psSaoLcuParam->offset[3] = -(Int)uiSymbol;
    }
  }
  else
  {
    psSaoLcuParam->length = 0;
  }
}

Void TDecSbac::parseSaoOneLcuInterleaving(Int rx, Int ry, SAOParam* pSaoParam, TComDataCU* pcCU, Int iCUAddrInSlice, Int iCUAddrUpInSlice, Bool bLFCrossSliceBoundaryFlag)
{
  Int iAddr = pcCU->getAddr();
  UInt uiSymbol;
  for (Int iCompIdx=0; iCompIdx<3; iCompIdx++)
  {
    pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeUpFlag    = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeLeftFlag  = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].bandPosition   = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].typeIdx        = -1;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].offset[0]     = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].offset[1]     = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].offset[2]     = 0;
    pSaoParam->saoLcuParam[iCompIdx][iAddr].offset[3]     = 0;

    if (pSaoParam->bSaoFlag[iCompIdx])
    {
      if (rx>0 && iCUAddrInSlice!=0)
      {
        parseSaoMergeLeft(uiSymbol,iCompIdx); pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeLeftFlag = (Int)uiSymbol;
      }
      else
      {
        pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeLeftFlag = 0;
      }

      if (pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeLeftFlag==0)
      {
        if ((ry > 0) && (iCUAddrUpInSlice>0||bLFCrossSliceBoundaryFlag))
        {
          parseSaoMergeUp(uiSymbol);  pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeUpFlag = uiSymbol;
        }
        else
        {
          pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeUpFlag = 0;
        }
        if (!pSaoParam->saoLcuParam[iCompIdx][iAddr].mergeUpFlag)
        {
          parseSaoOffset(&(pSaoParam->saoLcuParam[iCompIdx][iAddr]));
        }
        else
        {
          copySaoOneLcuParam(&pSaoParam->saoLcuParam[iCompIdx][iAddr], &pSaoParam->saoLcuParam[iCompIdx][iAddr-pSaoParam->numCuInWidth]);
        }
      }
      else
      {
        copySaoOneLcuParam(&pSaoParam->saoLcuParam[iCompIdx][iAddr],  &pSaoParam->saoLcuParam[iCompIdx][iAddr-1]);
      }
    }
    else
    {
      pSaoParam->saoLcuParam[iCompIdx][iAddr].typeIdx = -1;
      pSaoParam->saoLcuParam[iCompIdx][iAddr].bandPosition = 0;
    }
  }
}
#endif

/**
 - Initialize our contexts from the nominated source.
 .
 \param pSrc Contexts to be copied.
 */
Void TDecSbac::xCopyContextsFrom( TDecSbac* pSrc )
{
  memcpy(m_contextModels, pSrc->m_contextModels, m_numContextModels*sizeof(m_contextModels[0]));
}

Void TDecSbac::xCopyFrom( TDecSbac* pSrc )
{
  m_pcTDecBinIf->copyState( pSrc->m_pcTDecBinIf );

  m_uiLastQp           = pSrc->m_uiLastQp;
  xCopyContextsFrom( pSrc );

}

Void TDecSbac::load ( TDecSbac* pScr )
{
  xCopyFrom(pScr);
}

Void TDecSbac::loadContexts ( TDecSbac* pScr )
{
  xCopyContextsFrom(pScr);
}

#if OL_FLUSH
Void TDecSbac::decodeFlush ( )
{
  UInt uiBit;
  m_pcTDecBinIf->decodeBinTrm(uiBit);
  m_pcTDecBinIf->flush();

}
#endif
//! \}
