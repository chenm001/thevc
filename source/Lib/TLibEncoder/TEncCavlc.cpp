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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "TEncCavlc.h"
#include "SEIwrite.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
  m_bRunLengthCoding  = false;   //  m_bRunLengthCoding  = !rcSliceHeader.isIntra();
  m_uiCoeffCost       = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
  
  m_bAdaptFlag        = true;    // adaptive VLC table
}

TEncCavlc::~TEncCavlc()
{
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy()
{
  m_bRunLengthCoding = ! m_pcSlice->isIntra();
  m_uiRun = 0;
#if !CAVLC_COEF_LRG_BLK
  ::memcpy(m_uiLPTableE8, g_auiLPTableE8, 10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableD8, g_auiLPTableD8, 10*128*sizeof(UInt));
#endif
  ::memcpy(m_uiLPTableE4, g_auiLPTableE4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLPTableD4, g_auiLPTableD4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLastPosVlcIndex, g_auiLastPosVlcIndex, 10*sizeof(UInt));
  
#if LCEC_INTRA_MODE
#if MTK_DCM_MPM
  ::memcpy(m_uiIntraModeTableD17[0], g_auiIntraModeTableD17[0], 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE17[0], g_auiIntraModeTableE17[0], 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD17[1], g_auiIntraModeTableD17[1], 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE17[1], g_auiIntraModeTableE17[1], 16*sizeof(UInt));

  ::memcpy(m_uiIntraModeTableD34[0], g_auiIntraModeTableD34[0], 33*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE34[0], g_auiIntraModeTableE34[0], 33*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD34[1], g_auiIntraModeTableD34[1], 33*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE34[1], g_auiIntraModeTableE34[1], 33*sizeof(UInt));
#else
  ::memcpy(m_uiIntraModeTableD17, g_auiIntraModeTableD17, 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE17, g_auiIntraModeTableE17, 16*sizeof(UInt));

  ::memcpy(m_uiIntraModeTableD34, g_auiIntraModeTableD34, 33*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableE34, g_auiIntraModeTableE34, 33*sizeof(UInt));
#endif
#endif
  
#if CAVLC_RQT_CBP
  ::memcpy(m_uiCBP_YUV_TableE, g_auiCBP_YUV_TableE, 4*8*sizeof(UInt));
  ::memcpy(m_uiCBP_YUV_TableD, g_auiCBP_YUV_TableD, 4*8*sizeof(UInt));
  ::memcpy(m_uiCBP_YS_TableE,  g_auiCBP_YS_TableE,  2*4*sizeof(UInt));
  ::memcpy(m_uiCBP_YS_TableD,  g_auiCBP_YS_TableD,  2*4*sizeof(UInt));
  ::memcpy(m_uiCBP_YCS_TableE, g_auiCBP_YCS_TableE, 2*8*sizeof(UInt));
  ::memcpy(m_uiCBP_YCS_TableD, g_auiCBP_YCS_TableD, 2*8*sizeof(UInt));
  ::memcpy(m_uiCBP_4Y_TableE,  g_auiCBP_4Y_TableE,  2*15*sizeof(UInt));
  ::memcpy(m_uiCBP_4Y_TableD,  g_auiCBP_4Y_TableD,  2*15*sizeof(UInt));
  m_uiCBP_4Y_VlcIdx = 0;
#else
  ::memcpy(m_uiCBPTableE, g_auiCBPTableE, 2*8*sizeof(UInt));
  ::memcpy(m_uiCBPTableD, g_auiCBPTableD, 2*8*sizeof(UInt));
  m_uiCbpVlcIdx[0] = 0;
  m_uiCbpVlcIdx[1] = 0;
  ::memcpy(m_uiBlkCBPTableE, g_auiBlkCBPTableE, 2*15*sizeof(UInt));
  ::memcpy(m_uiBlkCBPTableD, g_auiBlkCBPTableD, 2*15*sizeof(UInt));
  m_uiBlkCbpVlcIdx = 0;
#endif

#if UNIFY_INTER_TABLE
  ::memcpy(m_uiMI1TableE, g_auiComMI1TableE, 9*sizeof(UInt));
  ::memcpy(m_uiMI1TableD, g_auiComMI1TableD, 9*sizeof(UInt));
#else  
  ::memcpy(m_uiMI1TableE, g_auiMI1TableE, 8*sizeof(UInt));
  ::memcpy(m_uiMI1TableD, g_auiMI1TableD, 8*sizeof(UInt));
  ::memcpy(m_uiMI2TableE, g_auiMI2TableE, 15*sizeof(UInt));
  ::memcpy(m_uiMI2TableD, g_auiMI2TableD, 15*sizeof(UInt));
  
#if DCM_COMB_LIST
  if ( m_pcSlice->getNoBackPredFlag() || m_pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0)
#else
  if ( m_pcSlice->getNoBackPredFlag() )
#endif
  {
    ::memcpy(m_uiMI1TableE, g_auiMI1TableENoL1, 8*sizeof(UInt));
    ::memcpy(m_uiMI1TableD, g_auiMI1TableDNoL1, 8*sizeof(UInt));
    ::memcpy(m_uiMI2TableE, g_auiMI2TableENoL1, 15*sizeof(UInt));
    ::memcpy(m_uiMI2TableD, g_auiMI2TableDNoL1, 15*sizeof(UInt));
  }
#if MS_LCEC_ONE_FRAME
  if ( m_pcSlice->getNumRefIdx(REF_PIC_LIST_0) <= 1 && m_pcSlice->getNumRefIdx(REF_PIC_LIST_1) <= 1 )
  {
    if ( m_pcSlice->getNoBackPredFlag() || ( m_pcSlice->getNumRefIdx(REF_PIC_LIST_C) > 0 && m_pcSlice->getNumRefIdx(REF_PIC_LIST_C) <= 1 ) )
    {
      ::memcpy(m_uiMI1TableE, g_auiMI1TableEOnly1RefNoL1, 8*sizeof(UInt));
      ::memcpy(m_uiMI1TableD, g_auiMI1TableDOnly1RefNoL1, 8*sizeof(UInt));
    }
    else
    {
      ::memcpy(m_uiMI1TableE, g_auiMI1TableEOnly1Ref, 8*sizeof(UInt));
      ::memcpy(m_uiMI1TableD, g_auiMI1TableDOnly1Ref, 8*sizeof(UInt));
    }
  }
#endif
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
  if (m_pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0)
  {
    m_uiMI1TableE[8] = 8;
    m_uiMI1TableD[8] = 8;
  }
  else  // GPB case
  {
    m_uiMI1TableD[8] = m_uiMI1TableD[6];
    m_uiMI1TableD[6] = 8;
    
    m_uiMI1TableE[m_uiMI1TableD[8]] = 8;
    m_uiMI1TableE[m_uiMI1TableD[6]] = 6;
  }
#endif
#endif

#if QC_LCEC_INTER_MODE
  ::memcpy(m_uiSplitTableE, g_auiInterModeTableE, 4*7*sizeof(UInt));
  ::memcpy(m_uiSplitTableD, g_auiInterModeTableD, 4*7*sizeof(UInt));
#endif
  
  m_uiMITableVlcIdx = 0;  

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
  ::memset(m_ucCBP_YUV_TableCounter,   0,        4*4*sizeof(UChar));
  ::memset(m_ucCBP_4Y_TableCounter,    0,        2*2*sizeof(UChar));
  ::memset(m_ucCBP_YCS_TableCounter,   0,        2*4*sizeof(UChar));
  ::memset(m_ucCBP_YS_TableCounter,    0,        2*3*sizeof(UChar));
#else
  ::memset(m_ucCBFTableCounter,        0,        2*4*sizeof(UChar));
  ::memset(m_ucBlkCBPTableCounter,     0,        2*2*sizeof(UChar));
#endif

  ::memset(m_ucMI1TableCounter,        0,          4*sizeof(UChar));
  ::memset(m_ucSplitTableCounter,      0,        4*4*sizeof(UChar));

#if CAVLC_RQT_CBP
  m_ucCBP_YUV_TableCounterSum[0] = m_ucCBP_YUV_TableCounterSum[1] = m_ucCBP_YUV_TableCounterSum[2] = m_ucCBP_YUV_TableCounterSum[3] = 0;
  m_ucCBP_4Y_TableCounterSum[0] = m_ucCBP_4Y_TableCounterSum[1] = 0;
  m_ucCBP_YCS_TableCounterSum[0] = m_ucCBP_YCS_TableCounterSum[1] = 0;
  m_ucCBP_YS_TableCounterSum[0] = m_ucCBP_YS_TableCounterSum[1] = 0;
#else
  m_ucCBFTableCounterSum[0] = m_ucCBFTableCounterSum[1] = 0;
  m_ucBlkCBPTableCounterSum[0] = m_ucBlkCBPTableCounterSum[1] = 0;
#endif
  m_ucSplitTableCounterSum[0] = m_ucSplitTableCounterSum[1] = m_ucSplitTableCounterSum[2]= m_ucSplitTableCounterSum[3] = 0;
  m_ucMI1TableCounterSum = 0;
#endif
}

#if !CAVLC_COEF_LRG_BLK
UInt* TEncCavlc::GetLP8Table()
{   
  return &m_uiLPTableE8[0][0];
}
#endif

UInt* TEncCavlc::GetLP4Table()
{   
  return &m_uiLPTableE4[0][0];
}

#if QC_MOD_LCEC
UInt* TEncCavlc::GetLastPosVlcIndexTable()
{   
  return &m_uiLastPosVlcIndex[0];
}
#endif

/**
 * marshall the SEI message @sei.
 */
void TEncCavlc::codeSEI(const SEI& sei)
{
  codeNALUnitHeader(NAL_UNIT_SEI, NAL_REF_IDC_PRIORITY_LOWEST);
  writeSEImessage(*m_pcBitIf, sei);
}

Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
  // uiFirstByte
  codeNALUnitHeader( NAL_UNIT_PPS, NAL_REF_IDC_PRIORITY_HIGHEST );

#if CONSTRAINED_INTRA_PRED
  xWriteFlag( pcPPS->getConstrainedIntraPred() ? 1 : 0 );
#endif
  return;
}

Void TEncCavlc::codeNALUnitHeader( NalUnitType eNalUnitType, NalRefIdc eNalRefIdc, UInt TemporalId, Bool bOutputFlag )
{
  // uiFirstByte
  xWriteCode( 0, 1);            // forbidden_zero_flag
  xWriteCode( eNalRefIdc, 2);   // nal_ref_idc
  xWriteCode( eNalUnitType, 5); // nal_unit_type

  if ( (eNalUnitType == NAL_UNIT_CODED_SLICE) || (eNalUnitType == NAL_UNIT_CODED_SLICE_IDR) || (eNalUnitType == NAL_UNIT_CODED_SLICE_CDR) )
  {
    xWriteCode( TemporalId, 3);   // temporal_id
    xWriteFlag( bOutputFlag );    // output_flag
    xWriteCode( 1, 4);            // reseved_one_4bits
  }
}

Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
  // uiFirstByte
  codeNALUnitHeader( NAL_UNIT_SPS, NAL_REF_IDC_PRIORITY_HIGHEST );
  
  // Structure
  xWriteUvlc  ( pcSPS->getWidth () );
  xWriteUvlc  ( pcSPS->getHeight() );
  
  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );
  
  assert( pcSPS->getMaxCUWidth() == pcSPS->getMaxCUHeight() );
  xWriteUvlc  ( pcSPS->getMaxCUWidth()   );
  xWriteUvlc  ( pcSPS->getMaxCUDepth()-g_uiAddCUDepth );
  
  xWriteUvlc( pcSPS->getQuadtreeTULog2MinSize() - 2 );
  xWriteUvlc( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize() );
  xWriteUvlc( pcSPS->getQuadtreeTUMaxDepthInter() - 1 );
  xWriteUvlc( pcSPS->getQuadtreeTUMaxDepthIntra() - 1 );
  
  
  // Tools
  xWriteFlag  ( (pcSPS->getUseALF ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseDQP ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseMRG ()) ? 1 : 0 ); // SOPH:
  
#if LM_CHROMA
  xWriteFlag  ( (pcSPS->getUseLMChroma ()) ? 1 : 0 ); 
#endif

#if HHI_RMP_SWITCH
  xWriteFlag  ( (pcSPS->getUseRMP()) ? 1 : 0 );
#endif
    
  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }
  
  // Bit-depth information
#if FULL_NBIT
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
#else
#if ENABLE_IBDI
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
#endif
  xWriteUvlc( pcSPS->getBitIncrement() );
#endif

#if MTK_NONCROSS_INLOOP_FILTER
  xWriteFlag( pcSPS->getLFCrossSliceBoundaryFlag()?1 : 0);
#endif
#if MTK_SAO
  xWriteFlag( pcSPS->getUseSAO() ? 1 : 0);
#endif

}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
  // here someone can add an appropriated NalRefIdc type 
#if DCM_DECODING_REFRESH
  codeNALUnitHeader (pcSlice->getNalUnitType(), NAL_REF_IDC_PRIORITY_HIGHEST, 1, true);
#else
  codeNALUnitHeader (NAL_UNIT_CODED_SLICE, NAL_REF_IDC_PRIORITY_HIGHEST);
#endif

  Bool bEntropySlice = false;
  if (pcSlice->isNextSlice())
  {
    xWriteFlag( 0 ); // Entropy slice flag
  }
  else
  {
    bEntropySlice = true;
    xWriteFlag( 1 ); // Entropy slice flag
  }
  if (!bEntropySlice)
  {
  xWriteCode  (pcSlice->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  xWriteUvlc  (pcSlice->getSliceType() );
  xWriteSvlc  (pcSlice->getSliceQp() );
  }
  if (pcSlice->isNextSlice())
  {
    xWriteUvlc(pcSlice->getSliceCurStartCUAddr());        // start CU addr for slice
  }
  else
  {
    xWriteUvlc(pcSlice->getEntropySliceCurStartCUAddr()); // start CU addr for entropy slice
  }
  if (!bEntropySlice)
  {  
  xWriteFlag  (pcSlice->getSymbolMode() > 0 ? 1 : 0);
  
  if (!pcSlice->isIntra())
  {
    xWriteFlag  (pcSlice->isReferenced() ? 1 : 0);
#if !HIGH_ACCURACY_BI
#ifdef ROUNDING_CONTROL_BIPRED
    xWriteFlag  (pcSlice->isRounding() ? 1 : 0);
#endif
#endif
  }
  
  xWriteFlag  (pcSlice->getLoopFilterDisable());
  
  if (!pcSlice->isIntra())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_0 )), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
  }
  if (pcSlice->isInterB())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_1 )), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
  }
  
#if DCM_COMB_LIST
  if (pcSlice->isInterB())
  {
    xWriteFlag  (pcSlice->getRefPicListCombinationFlag() ? 1 : 0 );
    if(pcSlice->getRefPicListCombinationFlag())
    {
      xWriteUvlc( pcSlice->getNumRefIdx(REF_PIC_LIST_C)-1);

      xWriteFlag  (pcSlice->getRefPicListModificationFlagLC() ? 1 : 0 );
      if(pcSlice->getRefPicListModificationFlagLC())
      {
        for (UInt i=0;i<pcSlice->getNumRefIdx(REF_PIC_LIST_C);i++)
        {
          xWriteFlag( pcSlice->getListIdFromIdxOfLC(i));
          xWriteUvlc( pcSlice->getRefIdxFromIdxOfLC(i));
        }
      }
    }
  }
#endif

  xWriteFlag  (pcSlice->getDRBFlag() ? 1 : 0 );
  if ( !pcSlice->getDRBFlag() )
  {
    xWriteCode  (pcSlice->getERBIndex(), 2);
  }
  
#if AMVP_NEIGH_COL
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    xWriteFlag( pcSlice->getColDir() );
  }
#endif
  }
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
}

Void TEncCavlc::codeSliceFinish ()
{
  if ( m_bRunLengthCoding && m_uiRun)
  {
    xWriteUvlc(m_uiRun);
  }
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum    = pcCU->getMVPNum(eRefList, uiAbsPartIdx);
  
  xWriteUnaryMaxSymbol(iSymbol, iNum-1);
}
#if QC_LCEC_INTER_MODE
Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->isIntra() && pcCU->isIntra( uiAbsPartIdx ) )
  {
#if MTK_DISABLE_INTRA_NxN_SPLIT
    if( uiDepth == (g_uiMaxCUDepth - g_uiAddCUDepth))
#endif
      xWriteFlag( pcCU->getPartitionSize(uiAbsPartIdx ) == SIZE_2Nx2N? 1 : 0 );
    return;
  }


#if MTK_DISABLE_INTRA_NxN_SPLIT && HHI_DISABLE_INTER_NxN_SPLIT 
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
  {    
    if ((pcCU->getPartitionSize(uiAbsPartIdx ) == SIZE_NxN) || pcCU->isIntra( uiAbsPartIdx ))
    {
      UInt uiIntraFlag = ( pcCU->isIntra(uiAbsPartIdx));
      if (pcCU->getPartitionSize(uiAbsPartIdx ) == SIZE_2Nx2N)
      {
        xWriteFlag(1);
      }
      else
      {
        xWriteFlag(0);
#if MTK_DISABLE_INTRA_NxN_SPLIT && !HHI_DISABLE_INTER_NxN_SPLIT 
        if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#elif !MTK_DISABLE_INTRA_NxN_SPLIT && HHI_DISABLE_INTER_NxN_SPLIT 
        if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
        xWriteFlag( uiIntraFlag? 1 : 0 );
      }

      return;
    }
  }
}
#else
Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );
  
  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteFlag( 0 );
#if HHI_RMP_SWITCH
    if( pcCU->getSlice()->getSPS()->getUseRMP() )
#endif
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
    }
#if HHI_DISABLE_INTER_NxN_SPLIT
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      xWriteFlag( 0 );
    }
#else
    xWriteFlag( 0 );
#endif
#if MTK_DISABLE_INTRA_NxN_SPLIT
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
    {
      xWriteFlag( (eSize == SIZE_2Nx2N? 0 : 1) );
    }
    return;
  }
  
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
#if MTK_DISABLE_INTRA_NxN_SPLIT
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
    {
      xWriteFlag( eSize == SIZE_2Nx2N? 1 : 0 );
    }
    return;
  }
  
  switch(eSize)
  {
    case SIZE_2Nx2N:
    {
      xWriteFlag( 1 );
      break;
    }
    case SIZE_2NxN:
    {
      xWriteFlag( 0 );
      xWriteFlag( 1 );
      break;
    }
    case SIZE_Nx2N:
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
      xWriteFlag( 1 );
      break;
    }
    case SIZE_NxN:
    {
#if HHI_DISABLE_INTER_NxN_SPLIT
      if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
#endif
      {
        xWriteFlag( 0 );
#if HHI_RMP_SWITCH
        if( pcCU->getSlice()->getSPS()->getUseRMP())
#endif
        {
          xWriteFlag( 0 );
          xWriteFlag( 0 );
        }
        if (pcCU->getSlice()->isInterB())
        {
          xWriteFlag( 1 );
        }
      }
      break;
    }
    default:
    {
      assert(0);
    }
  }
}
#endif

/** code prediction mode
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncCavlc::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if QC_LCEC_INTER_MODE
  codeInterModeFlag(pcCU, uiAbsPartIdx,(UInt)pcCU->getDepth(uiAbsPartIdx),2);
  return;
#else
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );
  if ( pcCU->getSlice()->isInterB() )
  {
    return;
  }
  xWriteFlag( iPredMode == MODE_INTER ? 0 : 1 );
#endif
}

/** code merge flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncCavlc::codeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if QC_LCEC_INTER_MODE
  if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N )
     return;
#endif
  UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
}

/** code merge index
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncCavlc::codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
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
  assert( uiNumCand > 1 );
  UInt uiUnaryIdx = pcCU->getMergeIndex( uiAbsPartIdx );
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
  for( UInt ui = 0; ui < uiNumCand - 1; ++ui )
  {
    const UInt uiSymbol = ui == uiUnaryIdx ? 0 : 1;
    xWriteFlag( uiSymbol );
    if( uiSymbol == 0 )
    {
      break;
    }
  }
}

Void TEncCavlc::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{  
  if (!m_bAlfCtrl)
    return;
  
  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }
  
  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;
  
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfCtrlDepth()
{  
  if (!m_bAlfCtrl)
    return;
  
  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  
  xWriteUnaryMaxSymbol(uiDepth, g_uiMaxCUDepth-1);
}
#if QC_LCEC_INTER_MODE
Void TEncCavlc::codeInterModeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiEncMode )
{
  Bool bHasSplit = ( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )? 0 : 1;
  UInt uiSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
  UInt uiMode=0,uiControl=0;
  UInt uiTableDepth = uiDepth;
  if ( !bHasSplit )
  {
    uiTableDepth = 3;
  }
  if(!uiSplitFlag || !bHasSplit)
  {
    uiMode = 1;
    uiControl = 1;
    if (!pcCU->isSkipped(uiAbsPartIdx ))
    {
      uiControl = 2;
      uiMode = 6;
      if (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER)
      {
        if(pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N)
          uiMode=pcCU->getMergeFlag(uiAbsPartIdx) ? 2 : 3;
        else 
          uiMode=3+(UInt)pcCU->getPartitionSize(uiAbsPartIdx);
      }
    }
  }
  if (uiEncMode != uiControl )
    return;
  UInt uiEndSym = bHasSplit ? 7 : 6;
  uiDepth = uiTableDepth;
  UInt uiLength = m_uiSplitTableE[uiDepth][uiMode] + 1;
  if (uiLength == uiEndSym)
  {
    xWriteCode( 0, uiLength - 1);
  }
  else
  {
    xWriteCode( 1, uiLength );
  }
  UInt x = uiMode;
  UInt cx = m_uiSplitTableE[uiDepth][x];
  /* Adapt table */
  if ( m_bAdaptFlag)
  {
#if CAVLC_COUNTER_ADAPT
    adaptCodeword(cx, m_ucSplitTableCounter[uiDepth],  m_ucSplitTableCounterSum[uiDepth],   m_uiSplitTableD[uiDepth],  m_uiSplitTableE[uiDepth], 4);
#else
    if(cx>0)
    {
      UInt cy = Max(0,cx-1);
      UInt y = m_uiSplitTableD[uiDepth][cy];
      m_uiSplitTableD[uiDepth][cy] = x;
      m_uiSplitTableD[uiDepth][cx] = y;
      m_uiSplitTableE[uiDepth][x] = cy;
      m_uiSplitTableE[uiDepth][y] = cx; 
    }
#endif
  }
  return;
}
#endif
Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if QC_LCEC_INTER_MODE
  codeInterModeFlag(pcCU,uiAbsPartIdx,(UInt)pcCU->getDepth(uiAbsPartIdx),1);
  return;
#else
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
#endif
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;
#if QC_LCEC_INTER_MODE
  if (!pcCU->getSlice()->isIntra())
  {
    codeInterModeFlag(pcCU,uiAbsPartIdx,uiDepth,0);
    return;
  }
#endif
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;
  
  xWriteFlag( uiCurrSplitFlag );
  return;
}

Void TEncCavlc::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType, uiTrDepth );
  xWriteFlag( uiCbf );
}

Void TEncCavlc::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  xWriteFlag( uiCbf ? 1 : 0 );
}

#if LCEC_INTRA_MODE
#if MTK_DCM_MPM
Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
#if ADD_PLANAR_MODE
  UInt planarFlag    = 0;
  if (iDir == PLANAR_IDX)
  {
    iDir = 2;
    planarFlag = 1;
  }
#endif
 
  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  

  Int uiPredIdx = -1;

  for(UInt i = 0; i < uiPredNum; i++)
  {
    if(iDir == uiPreds[i])
    {
      uiPredIdx = i;
    }
  }

  if ( g_aucIntraModeBitsAng[iIntraIdx] < 5 )
  {
    if(uiPredIdx != -1)
    {
      xWriteFlag(1);
      if(uiPredNum == 2)
      {
        xWriteFlag((UInt)uiPredIdx);
      }
    }
    else
    {
      xWriteFlag(0);
      for(Int i = (uiPredNum - 1); i >= 0; i--)
      {
        iDir = iDir > uiPreds[i] ? iDir - 1 : iDir;
      }


      xWriteFlag( iDir & 0x01 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { xWriteFlag( iDir & 0x02 ? 1 : 0 ); }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { xWriteFlag( iDir & 0x04 ? 1 : 0 ); }
    }
    
  }
  else 
  {
     UInt uiCode, uiLength;
     Int iRankIntraMode, iRankIntraModeLarger, iDirLarger;
      
     const UInt *huff;
     const UInt *lengthHuff;
     UInt  *m_uiIntraModeTableD;
     UInt  *m_uiIntraModeTableE;

     if ( g_aucIntraModeBitsAng[iIntraIdx] == 5 )
     {
       huff = huff17_2[uiPredNum - 1];
       lengthHuff = lengthHuff17_2[uiPredNum - 1];
       m_uiIntraModeTableD = m_uiIntraModeTableD17[uiPredNum - 1];
       m_uiIntraModeTableE = m_uiIntraModeTableE17[uiPredNum - 1];
     }
     else
     {
       huff = huff34_2[uiPredNum - 1];
       lengthHuff = lengthHuff34_2[uiPredNum - 1];
       m_uiIntraModeTableD = m_uiIntraModeTableD34[uiPredNum - 1];
       m_uiIntraModeTableE = m_uiIntraModeTableE34[uiPredNum - 1];
     }

     if(uiPredIdx != -1)
     {
       uiCode=huff[0];
       uiLength=lengthHuff[0];
       xWriteCode(uiCode, uiLength);

       if(uiPredNum == 2)
       {
         xWriteFlag((UInt)uiPredIdx);
       }
     }
     else
     {
       for(Int i = (uiPredNum - 1); i >= 0; i--)
       {
         iDir = iDir > uiPreds[i] ? iDir - 1 : iDir;
       }
       iRankIntraMode=m_uiIntraModeTableE[iDir];

       uiCode=huff[iRankIntraMode+1];
       uiLength=lengthHuff[iRankIntraMode+1];

       xWriteCode(uiCode, uiLength);

       if ( m_bAdaptFlag )
       {
         iRankIntraModeLarger = Max(0,iRankIntraMode-1);
         iDirLarger = m_uiIntraModeTableD[iRankIntraModeLarger];
  
         m_uiIntraModeTableD[iRankIntraModeLarger] = iDir;
         m_uiIntraModeTableD[iRankIntraMode] = iDirLarger;
         m_uiIntraModeTableE[iDir] = iRankIntraModeLarger;
         m_uiIntraModeTableE[iDirLarger] = iRankIntraMode;
       }
     }
  }
#if ADD_PLANAR_MODE
  iDir = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if ( (iDir == PLANAR_IDX) || (iDir == 2) )
  {
    xWriteFlag( planarFlag );
  }
#endif
}
#else
Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );
  Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
  UInt uiCode, uiLength;
  Int iRankIntraMode, iRankIntraModeLarger, iDirLarger;
#if ADD_PLANAR_MODE
  UInt planarFlag    = 0;
  if (iDir == PLANAR_IDX)
  {
    iDir = 2;
    planarFlag = 1;
  }
#endif

  UInt ind=(pcCU->getLeftIntraDirLuma( uiAbsPartIdx )==pcCU->getAboveIntraDirLuma( uiAbsPartIdx ))? 0 : 1;
  
  const UInt *huff17=huff17_2[ind];
  const UInt *lengthHuff17=lengthHuff17_2[ind];
  const UInt *huff34=huff34_2[ind];
  const UInt *lengthHuff34=lengthHuff34_2[ind];

  if ( g_aucIntraModeBitsAng[iIntraIdx] < 5 )
  {
    if (iDir == iMostProbable)
      xWriteFlag( 1 );
    else{
      if (iDir>iMostProbable)
        iDir--;
      xWriteFlag( 0 );
      xWriteFlag( iDir & 0x01 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) xWriteFlag( iDir & 0x02 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) xWriteFlag( iDir & 0x04 ? 1 : 0 );
    }
  }
  else if ( g_aucIntraModeBitsAng[iIntraIdx] == 5 )
  {

    if (iDir==iMostProbable)
    {
      uiCode=huff17[0];
      uiLength=lengthHuff17[0];
    }
    else
    { 
      if (iDir>iMostProbable)
      {
        iDir--;
      }
      iRankIntraMode=m_uiIntraModeTableE17[iDir];

      uiCode=huff17[iRankIntraMode+1];
      uiLength=lengthHuff17[iRankIntraMode+1];

      if ( m_bAdaptFlag )
      {
        iRankIntraModeLarger = Max(0,iRankIntraMode-1);
        iDirLarger = m_uiIntraModeTableD17[iRankIntraModeLarger];
        
        m_uiIntraModeTableD17[iRankIntraModeLarger] = iDir;
        m_uiIntraModeTableD17[iRankIntraMode] = iDirLarger;
        m_uiIntraModeTableE17[iDir] = iRankIntraModeLarger;
        m_uiIntraModeTableE17[iDirLarger] = iRankIntraMode;
      }
    }
    xWriteCode(uiCode, uiLength);
  }
  else{
    if (iDir==iMostProbable)
    {
      uiCode=huff34[0];
      uiLength=lengthHuff34[0];
    }
    else{
      if (iDir>iMostProbable)
      {
        iDir--;
      }
      iRankIntraMode=m_uiIntraModeTableE34[iDir];

      uiCode=huff34[iRankIntraMode+1];
      uiLength=lengthHuff34[iRankIntraMode+1];

      if ( m_bAdaptFlag )
      {
        iRankIntraModeLarger = Max(0,iRankIntraMode-1);
        iDirLarger = m_uiIntraModeTableD34[iRankIntraModeLarger];

        m_uiIntraModeTableD34[iRankIntraModeLarger] = iDir;
        m_uiIntraModeTableD34[iRankIntraMode] = iDirLarger;
        m_uiIntraModeTableE34[iDir] = iRankIntraModeLarger;
        m_uiIntraModeTableE34[iDirLarger] = iRankIntraMode;
      }
    }

    xWriteCode(uiCode, uiLength);
  }

#if ADD_PLANAR_MODE
  iDir = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if ( (iDir == PLANAR_IDX) || (iDir == 2) )
  {
    xWriteFlag( planarFlag );
  }
#endif

}
#endif
#else

Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
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
  {
    xWriteFlag( 1 );
  }
  else
  {
    xWriteFlag( 0 );
    uiDir = uiDir > iMostProbable ? uiDir - 1 : uiDir;
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      xWriteFlag( uiDir & 0x01 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) xWriteFlag( uiDir & 0x02 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) xWriteFlag( uiDir & 0x04 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) xWriteFlag( uiDir & 0x08 ? 1 : 0 );
    }
    else
    {
      if (uiDir < 31)
      { // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
        xWriteFlag( uiDir & 0x01 ? 1 : 0 );
        xWriteFlag( uiDir & 0x02 ? 1 : 0 );
        xWriteFlag( uiDir & 0x04 ? 1 : 0 );
        xWriteFlag( uiDir & 0x08 ? 1 : 0 );
        xWriteFlag( uiDir & 0x10 ? 1 : 0 );
      }
      else
      {
        xWriteFlag( 1 );
        xWriteFlag( 1 );
        xWriteFlag( 1 );
        xWriteFlag( 1 );
        xWriteFlag( 1 );
        xWriteFlag( uiDir == 32 ? 1 : 0 );
      }
    }
  }

#if ADD_PLANAR_MODE
  uiDir = pcCU->getLumaIntraDir( uiAbsPartIdx );
  if ( (uiDir == PLANAR_IDX) || (uiDir == 2) )
  {
    xWriteFlag( planarFlag );
  }
#endif

}
#endif

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );
#if ADD_PLANAR_MODE
  UInt planarFlag       = 0;
  if (uiIntraDirChroma == PLANAR_IDX)
  {
    uiIntraDirChroma = 2;
    planarFlag = 1;
  }
#endif

#if CHROMA_CODEWORD
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

  Int  iMax = uiMode < iMaxMode ? 3 : 4; 
  
  //switch codeword
  if (uiIntraDirChroma == 4)
  {
    uiIntraDirChroma = 0;
  }
  else if (uiIntraDirChroma == 3 && bUseLMFlag )
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
    uiIntraDirChroma = ChromaMapping[iMax-3][uiIntraDirChroma];
#endif
  }

  xWriteUnaryMaxSymbol( uiIntraDirChroma, iMax);

#else //<--LM_CHROMA
  Int  iMax = uiMode < 4 ? 3 : 4; 
  
  //switch codeword
  if (uiIntraDirChroma == 4)
  {
    uiIntraDirChroma = 0;
  }
#if CHROMA_CODEWORD_SWITCH 
  else
  {
    if (uiIntraDirChroma < uiMode)
    {
      uiIntraDirChroma++;
    }
    uiIntraDirChroma = ChromaMapping[iMax-3][uiIntraDirChroma];
  }
#else
  else if (uiIntraDirChroma < uiMode)
  {
    uiIntraDirChroma++;
  }
#endif
  xWriteUnaryMaxSymbol( uiIntraDirChroma, iMax);
#endif //<-- LM_CHROMA

#else // CHROMA_CODEWORD
  if ( 0 == uiIntraDirChroma )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
    xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, 3 );
  }
#endif

#if ADD_PLANAR_MODE
  uiIntraDirChroma = pcCU->getChromaIntraDir( uiAbsPartIdx );
#if CHROMA_CODEWORD
  uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
  mapPlanartoDC( uiIntraDirChroma );
  mapPlanartoDC( uiMode );
  if ( (uiIntraDirChroma == 2) && (uiMode != 2) )
#else
  if ( (uiIntraDirChroma == PLANAR_IDX) || (uiIntraDirChroma == 2) )
#endif
  {
    xWriteFlag( planarFlag );
  }
#endif
  return;
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  uiInterDir--;

#if UNIFY_INTER_TABLE
#if DCM_COMB_LIST
  UInt uiNumRefIdxOfLC = pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C);
#endif
  #define min(a, b) (((a) < (b)) ? (a) : (b))
#if DCM_COMB_LIST
  UInt uiValNumRefIdxOfLC = min(4,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C));
#endif
  UInt uiValNumRefIdxOfL0 = min(2,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_0));
  UInt uiValNumRefIdxOfL1 = min(2,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_1));

  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
  {
#if CAVLC_COUNTER_ADAPT
    Int x,cx;
#else
    Int x,cx,y,cy;
#endif
    Int iRefFrame0,iRefFrame1;
    UInt uiIndex = 0;
    
    UInt *m_uiMITableE;
    UInt *m_uiMITableD;
       
    m_uiMITableE = m_uiMI1TableE;
    m_uiMITableD = m_uiMI1TableD;

    UInt uiMaxVal;
#if DCM_COMB_LIST
    if (uiNumRefIdxOfLC > 0)
    {
      uiMaxVal = uiValNumRefIdxOfLC + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }
    else
#endif
    if (m_pcSlice->getNoBackPredFlag())
    {
      uiMaxVal = uiValNumRefIdxOfL0 + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }
    else
    {
      uiMaxVal = uiValNumRefIdxOfL0 + uiValNumRefIdxOfL1 + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }

    if (uiInterDir==0)
    { 
#if DCM_COMB_LIST
      if(uiNumRefIdxOfLC > 0)
      {
        iRefFrame0 = pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ));
      }
      else
#endif
      {
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
      }
      uiIndex = iRefFrame0;
#if DCM_COMB_LIST      
      if(uiNumRefIdxOfLC > 0)
      {
        if ( iRefFrame0 >= 4 )
        {
          uiIndex = uiMaxVal;
        }
      }
      else
#endif
      {
        if ( iRefFrame0 > 1 )
        {
          uiIndex = uiMaxVal;
        }
      }        
    }
    else if (uiInterDir==1)
    {
#if DCM_COMB_LIST
      if(uiNumRefIdxOfLC > 0)
      {
        iRefFrame1 = pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx ));
        uiIndex = iRefFrame1;
      }
      else
#endif
      {
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        uiIndex = uiValNumRefIdxOfL0 + iRefFrame1;
      }
#if DCM_COMB_LIST
      if(uiNumRefIdxOfLC > 0)
      {
        if ( iRefFrame1 >= 4 )
        {
          uiIndex = uiMaxVal;
        }
      }
      else
#endif
      {
        if ( iRefFrame1 > 1 )
        {
          uiIndex = uiMaxVal;
        }
      } 
    }
    else
    {
      iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
      iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
      if ( iRefFrame0 >= 2 || iRefFrame1 >= 2 )
      {
        uiIndex = uiMaxVal;
      }
      else
      {
  #if DCM_COMB_LIST
        if(uiNumRefIdxOfLC > 0)
        {
          uiIndex = uiValNumRefIdxOfLC + iRefFrame0*uiValNumRefIdxOfL1 + iRefFrame1;
        } 
        else
  #endif
        if (m_pcSlice->getNoBackPredFlag())
        {
          uiMaxVal = uiValNumRefIdxOfL0 + iRefFrame0*uiValNumRefIdxOfL1 + iRefFrame1;
        }
        else
        {
          uiIndex = uiValNumRefIdxOfL0 + uiValNumRefIdxOfL1 + iRefFrame0*uiValNumRefIdxOfL1 + iRefFrame1;
        }
      }
    }
      
    x = uiIndex;
    
    cx = m_uiMITableE[x];
    
    /* Adapt table */
    if ( m_bAdaptFlag )
    {        
#if CAVLC_COUNTER_ADAPT
      adaptCodeword(cx, m_ucMI1TableCounter,  m_ucMI1TableCounterSum,   m_uiMITableD,  m_uiMITableE, 4);
#else
      cy = Max(0,cx-1);
      y = m_uiMITableD[cy];
      m_uiMITableD[cy] = x;
      m_uiMITableD[cx] = y;
      m_uiMITableE[x] = cy;
      m_uiMITableE[y] = cx;   
      m_uiMITableVlcIdx += cx == m_uiMITableVlcIdx ? 0 : (cx < m_uiMITableVlcIdx ? -1 : 1);
#endif
    }
    
    xWriteUnaryMaxSymbol( cx, uiMaxVal );
    
    if ( x<uiMaxVal ) 
    {
      return;
    }
  }

#else //UNIFY_INTER_TABLE  
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
#else
  if(pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2)
#endif
  {
#if CAVLC_COUNTER_ADAPT
    Int x,cx;
#else
    Int x,cx,y,cy;
#endif
    Int iRefFrame0,iRefFrame1;
    UInt uiIndex;
    
    UInt *m_uiMITableE;
    UInt *m_uiMITableD;
    {      
      m_uiMITableE = m_uiMI1TableE;
      m_uiMITableD = m_uiMI1TableD;
      if (uiInterDir==0)
      { 
#if DCM_COMB_LIST
        if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
        {
          iRefFrame0 = pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ));
        }
        else
        {
          iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        }
#else
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
#endif
        uiIndex = iRefFrame0;
        
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
        if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
        {
          if ( iRefFrame0 >= 4 )
          {
            uiIndex = 8;
          }
        }
        else
        {
          if ( iRefFrame0 > MS_LCEC_UNI_EXCEPTION_THRES )
          {
            uiIndex = 8;
          }
        }        
#endif        
      }
      else if (uiInterDir==1)
      {
#if DCM_COMB_LIST
        if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
        {
          iRefFrame1 = pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx ));
          uiIndex = iRefFrame1;
        }
        else
        {
          iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
          uiIndex = 2 + iRefFrame1;
        }
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
        if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
        {
          if ( iRefFrame1 >= 4 )
          {
            uiIndex = 8;
          }
        }
        else
        {
          if ( iRefFrame1 > MS_LCEC_UNI_EXCEPTION_THRES )
          {
            uiIndex = 8;
          }
        } 
#endif
#else
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        uiIndex = 2 + iRefFrame1;
#endif
      }
      else
      {
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        uiIndex = 4 + 2*iRefFrame0 + iRefFrame1;
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
        if ( iRefFrame0 >= 2 || iRefFrame1 >= 2 )
        {
          uiIndex = 8;
        }
#endif
      }
    }
    
    x = uiIndex;
    
    cx = m_uiMITableE[x];
    
    /* Adapt table */
#if !MS_LCEC_LOOKUP_TABLE_MAX_VALUE
    UInt vlcn = g_auiMITableVlcNum[m_uiMITableVlcIdx];    
#endif
    if ( m_bAdaptFlag )
    {
#if CAVLC_COUNTER_ADAPT
      adaptCodeword(cx, m_ucMI1TableCounter,  m_ucMI1TableCounterSum,   m_uiMITableD,  m_uiMITableE, 4);
#else
      cy = Max(0,cx-1);
      y = m_uiMITableD[cy];
      m_uiMITableD[cy] = x;
      m_uiMITableD[cx] = y;
      m_uiMITableE[x] = cy;
      m_uiMITableE[y] = cx;   
      m_uiMITableVlcIdx += cx == m_uiMITableVlcIdx ? 0 : (cx < m_uiMITableVlcIdx ? -1 : 1);
#endif
    }
    
    {
#if MS_LCEC_LOOKUP_TABLE_MAX_VALUE
      UInt uiMaxVal = 7;
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
      uiMaxVal = 8;
#endif
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 1 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 1 )
      {
        if ( pcCU->getSlice()->getNoBackPredFlag() || ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 1 ) )
        {
          uiMaxVal = 1;
        }
        else
        {
          uiMaxVal = 2;
        }
      }
      else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2 )
      {
        if ( pcCU->getSlice()->getNoBackPredFlag() || ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 2 ) )
        {
          uiMaxVal = 5;
        }
        else
        {
          uiMaxVal = 7;
        }
      }
      else if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 0 ) // GPB case
      {
        uiMaxVal = 4+1+MS_LCEC_UNI_EXCEPTION_THRES;
      }
      
      xWriteUnaryMaxSymbol( cx, uiMaxVal );
#else
      xWriteVlc( vlcn, cx );
#endif
    }
    
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
    if ( x<8 ) 
#endif   
    {
      return;
    }
  }
#endif //UNIFY_INTER_TABLE
  
  xWriteFlag( ( uiInterDir == 2 ? 1 : 0 ));
  if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    assert( uiInterDir != 1 );
    return;
  }
#if DCM_COMB_LIST
  if ( uiInterDir < 2 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 0)
#else
  if ( uiInterDir < 2 )
#endif
  {
    xWriteFlag( uiInterDir );
  }
  
  return;
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
#if DCM_COMB_LIST
  Int iRefFrame;
  RefPicList eRefListTemp;

  if( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C)>0)
  {
    if ( pcCU->getInterDir( uiAbsPartIdx ) != 3)
    {
      eRefListTemp = REF_PIC_LIST_C;
      iRefFrame = pcCU->getSlice()->getRefIdxOfLC(eRefList, pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx ));
    }
    else
    {
      eRefListTemp = eRefList;
      iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
    }
  }
  else
  {
    eRefListTemp = eRefList;
    iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
  }
#else
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
#endif  

  if (pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2 && pcCU->getSlice()->isInterB())
  {
    return;
  }
  
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
  if ( pcCU->getSlice()->getRefIdxCombineCoding() && pcCU->getInterDir(uiAbsPartIdx)==3 &&
      pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ) < 2 &&
      pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx ) < 2 )
  {
    return;
  }
  else if ( pcCU->getSlice()->getRefIdxCombineCoding() && pcCU->getInterDir(uiAbsPartIdx)==1 && 
           ( ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C)>0  && pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx )) < 4 ) || 
            ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C)<=0 && pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ) <= MS_LCEC_UNI_EXCEPTION_THRES ) ) )
  {
    return;
  }
  else if ( pcCU->getSlice()->getRefIdxCombineCoding() && pcCU->getInterDir(uiAbsPartIdx)==2 && pcCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx )) < 4 )
  {
    return;
  }
  
  UInt uiRefFrmIdxMinus = 0;
  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
  {
    if ( pcCU->getInterDir( uiAbsPartIdx ) != 3 )
    {
      if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 )
      {
        uiRefFrmIdxMinus = 4;
        assert( iRefFrame >=4 );
      }
      else
      {
        uiRefFrmIdxMinus = MS_LCEC_UNI_EXCEPTION_THRES+1;
        assert( iRefFrame > MS_LCEC_UNI_EXCEPTION_THRES );
      }
      
    }
    else if ( eRefList == REF_PIC_LIST_1 && pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ) < 2 )
    {
      uiRefFrmIdxMinus = 2;
      assert( iRefFrame >= 2 );
    }
  }
  
  if ( pcCU->getSlice()->getNumRefIdx( eRefListTemp ) - uiRefFrmIdxMinus <= 1 )
  {
    return;
  }
  xWriteFlag( ( iRefFrame - uiRefFrmIdxMinus == 0 ? 0 : 1 ) );
#else
  xWriteFlag( ( iRefFrame == 0 ? 0 : 1 ) );
#endif
  
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
  if ( iRefFrame - uiRefFrmIdxMinus > 0 )
#else    
  if ( iRefFrame > 0 )
#endif
  {
    {
#if DCM_COMB_LIST
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
      xWriteUnaryMaxSymbol( iRefFrame - 1 - uiRefFrmIdxMinus, pcCU->getSlice()->getNumRefIdx( eRefListTemp )-2 - uiRefFrmIdxMinus );
#else
      xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefListTemp )-2 );
#endif
#else
#if MS_LCEC_LOOKUP_TABLE_EXCEPTION
      xWriteUnaryMaxSymbol( iRefFrame - 1 - uiRefFrmIdxMinus, pcCU->getSlice()->getNumRefIdx( eRefList )-2 - uiRefFrmIdxMinus );      
#else
      xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
#endif
#endif
    }
  }
  return;
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
  
  UInt uiAbsPartIdxL, uiAbsPartIdxA;
  Int iHorPred, iVerPred;
  
  TComDataCU* pcCUL   = pcCU->getPULeft ( uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  
  TComCUMvField* pcCUMvFieldL = ( pcCUL == NULL || pcCUL->isIntra( uiAbsPartIdxL ) ) ? NULL : pcCUL->getCUMvField( eRefList );
  TComCUMvField* pcCUMvFieldA = ( pcCUA == NULL || pcCUA->isIntra( uiAbsPartIdxA ) ) ? NULL : pcCUA->getCUMvField( eRefList );
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );
  
  xWriteSvlc( iHor );
  xWriteSvlc( iVer );
  
  return;
}

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getSlice()->getSliceQp();
  
  if ( iDQp == 0 )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
    xWriteSvlc( iDQp );
  }
  
  return;
}

#if CAVLC_RQT_CBP
/** Function for coding cbf and split flag
* \param pcCU pointer to CU
* \param uiAbsPartIdx CU index
* \param uiDepth CU Depth
* \returns 
* This function performs coding of cbf and split flag
*/
Void TEncCavlc::codeCbfTrdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt n,cx;
  UInt uiTrDepth = uiDepth - pcCU->getDepth(uiAbsPartIdx);
  UInt uiCBFY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth);
  UInt uiCBFU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth);
  UInt uiCBFV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth);
  UInt uiQPartNumParent  = pcCU->getPic()->getNumPartInCU() >> ((uiDepth-1) << 1);
  UInt uiQPartNumCurr    = pcCU->getPic()->getNumPartInCU() >> ((uiDepth) << 1);

  const UInt uiSubdiv = pcCU->getTransformIdx( uiAbsPartIdx ) + pcCU->getDepth( uiAbsPartIdx ) > uiDepth;
  UInt uiFlagPattern = xGetFlagPattern( pcCU, uiAbsPartIdx, uiDepth );
  n = pcCU->isIntra( uiAbsPartIdx ) ? 0 : 1;

  if(uiFlagPattern < 8)
  {
    UInt uiFullDepth = pcCU->getDepth(uiAbsPartIdx) + uiTrDepth;
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
    if( uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      UInt uiCBFU_Parent = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth-1);
      UInt uiCBFV_Parent = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth-1);
      if(uiCBFU_Parent)
      {
        xWriteFlag(uiCBFU);
      }
      if(uiCBFV_Parent)
      {
        xWriteFlag(uiCBFV);
      }
    }

    if(uiFlagPattern & 0x01)
    {
      codeTransformSubdivFlag( uiSubdiv, 0);
    }
  }
  else
  {
    if(uiFlagPattern == 8)
    {
      if (uiAbsPartIdx % uiQPartNumParent ==0)
      {
        codeBlockCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, uiTrDepth, uiQPartNumCurr);
      }
    } 
    else if(uiFlagPattern == 9)
    {
      bool bNeedToCode = true;
      if ( n==1 && (uiAbsPartIdx%uiQPartNumParent) / uiQPartNumCurr == 3 )  
      {
        UInt uiTempAbsPartIdx = uiAbsPartIdx/uiQPartNumParent*uiQPartNumParent;
        if ( pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*0, TEXT_LUMA, uiTrDepth ) ||
          pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*1, TEXT_LUMA, uiTrDepth ) ||
          pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*2, TEXT_LUMA, uiTrDepth ) )
        {
          bNeedToCode = true;
        }
        else
        {
          bNeedToCode = false;
          xWriteFlag( uiSubdiv );
        }
      }
      if ( bNeedToCode )
      {
        cx = m_uiCBP_YS_TableE[n][(uiCBFY<<1)+uiSubdiv];
        xWriteUnaryMaxSymbol(cx,n?2:3);//intra 3; inter 2

        if ( m_bAdaptFlag )
        {
          adaptCodeword(cx, m_ucCBP_YS_TableCounter[n],  m_ucCBP_YS_TableCounterSum[n],  m_uiCBP_YS_TableD[n],  m_uiCBP_YS_TableE[n], 3);
        }
      }
    }
    else if( uiFlagPattern == 14)
    {
      UInt  uiIdx = uiTrDepth? (2 + n) : n;
      cx = m_uiCBP_YUV_TableE[uiIdx][(uiCBFV<<0) + (uiCBFU<<1) + (uiCBFY<<2)];
      xWriteUnaryMaxSymbol(cx,7);

      if ( m_bAdaptFlag )
      {
        adaptCodeword(cx,  m_ucCBP_YUV_TableCounter[uiIdx],  m_ucCBP_YUV_TableCounterSum[uiIdx],  m_uiCBP_YUV_TableD[uiIdx],  m_uiCBP_YUV_TableE[uiIdx], 4);
      }
    }
    else if ( uiFlagPattern == 11 || uiFlagPattern == 13 || uiFlagPattern == 15 )
    {
      cx = m_uiCBP_YCS_TableE[n][(uiCBFY<<2)+((uiCBFU||uiCBFV?1:0)<<1)+uiSubdiv];
      xWriteCode(g_auiCBP_YCS_Table[n][cx], g_auiCBP_YCS_TableLen[n][cx]);

      if ( m_bAdaptFlag )
      {
        adaptCodeword(cx, m_ucCBP_YCS_TableCounter[n],  m_ucCBP_YCS_TableCounterSum[n],  m_uiCBP_YCS_TableD[n],  m_uiCBP_YCS_TableE[n], 4);
      }

      //U and V          
      if ( uiFlagPattern == 15)
      {
        UInt uiCBFUV = (uiCBFU<<1) + uiCBFV;
        if(uiCBFUV > 0)
        {
          xWriteUnaryMaxSymbol(n? (uiCBFUV - 1) : (3-uiCBFUV), 2); 
        }
      }
    }
    else if (uiFlagPattern == 10 || uiFlagPattern == 12)
    {
      xWriteUnaryMaxSymbol(g_auiCBP_YC_TableE[n][(uiCBFY<<1)+(uiCBFU||uiCBFV?1:0)],3);//intra 3; inter 2
    }
  }
  return;
}


/** Function for parsing cbf and split 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU Depth
 * \returns flag pattern
 * This function gets flagpattern for cbf and split flag
 */
UInt TEncCavlc::xGetFlagPattern( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  UInt uiTrDepth =  uiDepth - pcCU->getDepth( uiAbsPartIdx );
  UInt patternYUV, patternDiv;
  UInt bY, bU, bV;

  UInt uiFullDepth = uiDepth;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  if(uiTrDepth == 0)
  {
    patternYUV = 7;
  }
  else if( uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    bY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth - 1)?1:0;
    bU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1)?1:0;
    bV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1)?1:0;
    patternYUV = (bY<<2) + (bU<<1) + bV;
  }
  else
  {
    bY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth - 1)?1:0;
    patternYUV = bY<<2;
  }

  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    patternDiv = 0;
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    patternDiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    patternDiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    patternDiv = 0;
  }
  else
  {
    patternDiv = 1;
  }
  return ((patternYUV<<1)+patternDiv);
}
#endif

Void TEncCavlc::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  if (eType == TEXT_ALL)
  {
#if CAVLC_COUNTER_ADAPT
    Int n,x,cx;
#else
    Int n,x,cx,y,cy;
#endif

    UInt uiCBFY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0);
    UInt uiCBFU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0);
    UInt uiCBFV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0);
    UInt uiCBP = (uiCBFV<<2) + (uiCBFU<<1) + (uiCBFY<<0);
    
    /* Start adaptation */
    n = pcCU->isIntra( uiAbsPartIdx ) ? 0 : 1;
    x = uiCBP;
#if CAVLC_RQT_CBP
    cx = m_uiCBP_YUV_TableE[n][x];
    UInt vlcn = 0;
#else
    cx = m_uiCBPTableE[n][x];
    UInt vlcn = g_auiCbpVlcNum[n][m_uiCbpVlcIdx[n]];
#endif

    if ( m_bAdaptFlag )
    {                
#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
      adaptCodeword(cx, m_ucCBP_YUV_TableCounter[n], m_ucCBP_YUV_TableCounterSum[n], m_uiCBP_YUV_TableD[n], m_uiCBP_YUV_TableE[n], 4);
#else
      adaptCodeword(cx, m_ucCBFTableCounter[n],  m_ucCBFTableCounterSum[n],  m_uiCBPTableD[n],  m_uiCBPTableE[n], 4);
#endif
#else
      cy = Max(0,cx-1);
      y = m_uiCBPTableD[n][cy];
      m_uiCBPTableD[n][cy] = x;
      m_uiCBPTableD[n][cx] = y;
      m_uiCBPTableE[n][x] = cy;
      m_uiCBPTableE[n][y] = cx;
      m_uiCbpVlcIdx[n] += cx == m_uiCbpVlcIdx[n] ? 0 : (cx < m_uiCbpVlcIdx[n] ? -1 : 1);
#endif
    }
    xWriteVlc( vlcn, cx );
  }
}

Void TEncCavlc::codeBlockCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiQPartNum, Bool bRD )
{
  UInt uiCbf0 = pcCU->getCbf   ( uiAbsPartIdx, eType, uiTrDepth );
  UInt uiCbf1 = pcCU->getCbf   ( uiAbsPartIdx + uiQPartNum, eType, uiTrDepth );
  UInt uiCbf2 = pcCU->getCbf   ( uiAbsPartIdx + uiQPartNum*2, eType, uiTrDepth );
  UInt uiCbf3 = pcCU->getCbf   ( uiAbsPartIdx + uiQPartNum*3, eType, uiTrDepth );
  UInt uiCbf = (uiCbf0<<3) | (uiCbf1<<2) | (uiCbf2<<1) | uiCbf3;
  
  assert(uiTrDepth > 0);
  
  if(bRD && uiCbf==0)
  {
    xWriteCode(0, 4); 
    return;
  }
  
  assert(uiCbf > 0);
  
  uiCbf --;

#if CAVLC_COUNTER_ADAPT
  Int x,cx;
#else
  Int x,cx,y,cy;
#endif

  UInt n = (pcCU->isIntra(uiAbsPartIdx) && eType == TEXT_LUMA)? 0:1;
  x = uiCbf;

#if CAVLC_RQT_CBP
  cx = m_uiCBP_4Y_TableE[n][uiCbf];
  UInt vlcn = (n==0)?g_auiCBP_4Y_VlcNum[m_uiCBP_4Y_VlcIdx]:11;
#else
  cx = m_uiBlkCBPTableE[n][uiCbf];
  UInt vlcn = (n==0)?g_auiBlkCbpVlcNum[m_uiBlkCbpVlcIdx]:11;
#endif


  if ( m_bAdaptFlag )
  {                

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
    adaptCodeword(cx, m_ucCBP_4Y_TableCounter[n],  m_ucCBP_4Y_TableCounterSum[n],  m_uiCBP_4Y_TableD[n],  m_uiCBP_4Y_TableE[n], 2);
#else
    adaptCodeword(cx, m_ucBlkCBPTableCounter[n],  m_ucBlkCBPTableCounterSum[n],  m_uiBlkCBPTableD[n],  m_uiBlkCBPTableE[n], 2);
#endif
#else
    cy = Max(0,cx-1);
    y = m_uiBlkCBPTableD[n][cy];
    m_uiBlkCBPTableD[n][cy] = x;
    m_uiBlkCBPTableD[n][cx] = y;
    m_uiBlkCBPTableE[n][x] = cy;
    m_uiBlkCBPTableE[n][y] = cx;
#endif

#if CAVLC_RQT_CBP
    if(n==0)
      m_uiCBP_4Y_VlcIdx += cx == m_uiCBP_4Y_VlcIdx ? 0 : (cx < m_uiCBP_4Y_VlcIdx ? -1 : 1);
#else
    if(n==0)
      m_uiBlkCbpVlcIdx += cx == m_uiBlkCbpVlcIdx ? 0 : (cx < m_uiBlkCbpVlcIdx ? -1 : 1);
#endif
  }
  
  xWriteVlc( vlcn, cx );
  return;
}

Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
{
  if ( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() )
  {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;
  
  // point to coefficient
  TCoeff* piCoeff = pcCoef;
  UInt uiNumSig = 0;
  UInt uiScanning;
  
  // compute number of significant coefficients
  UInt  uiPart = 0;
  xCheckCoeff(piCoeff, uiWidth, 0, uiNumSig, uiPart );
  
  if ( bRD )
  {
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }
  
  if ( uiNumSig == 0 )
  {
    return;
  }
  
  // initialize scan
  const UInt*  pucScan;

#if CAVLC_COEF_LRG_BLK
  UInt maxBlSize = (eTType==TEXT_LUMA)?32:8;
  UInt uiBlSize = Min(maxBlSize,uiWidth);
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth :uiBlSize];
  UInt uiNoCoeff = uiBlSize*uiBlSize;
#else
  //UInt uiConvBit = g_aucConvertToBit[ Min(8,uiWidth)    ];
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : Min(8,uiWidth)    ];
#endif
  pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];
  
#if QC_MDCS
  UInt uiBlkPos;
#if CAVLC_COEF_LRG_BLK
  UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : uiBlSize] + 2;
#else
  UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : Min(8,uiWidth)    ] + 2;
#endif
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif //QC_MDCS
  
#if CAVLC_COEF_LRG_BLK
  static TCoeff scoeff[1024];
#else
  TCoeff scoeff[64];
#endif
  Int iBlockType;
  UInt uiCodeDCCoef = 0;
  TCoeff dcCoeff = 0;
  if (pcCU->isIntra(uiAbsPartIdx))
  {
    UInt uiAbsPartIdxL, uiAbsPartIdxA;
    TComDataCU* pcCUL   = pcCU->getPULeft (uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    TComDataCU* pcCUA   = pcCU->getPUAbove(uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    if (pcCUL == NULL && pcCUA == NULL)
    {
      uiCodeDCCoef = 1;
      xWriteVlc((eTType == TEXT_LUMA ? 3 : 1) , abs(piCoeff[0]));
      if (piCoeff[0] != 0)
      {
        UInt sign = (piCoeff[0] < 0) ? 1 : 0;
        xWriteFlag(sign);
      }
      dcCoeff = piCoeff[0];
      piCoeff[0] = 1;
    }
  }
  
  if( uiSize == 2*2 )
  {
    // hack: re-use 4x4 coding
    ::memset( scoeff, 0, 16*sizeof(TCoeff) );
    for (uiScanning=0; uiScanning<4; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      scoeff[15-uiScanning] = piCoeff[ uiBlkPos ];
#else
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
#endif //QC_MDCS
    }
#if QC_MOD_LCEC
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#else
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();
#endif
    
#if CAVLC_COEF_LRG_BLK
    xCodeCoeff( scoeff, iBlockType, 4 );
#else
    xCodeCoeff4x4( scoeff, iBlockType );
#endif
  }
  else if ( uiSize == 4*4 )
  {
    for (uiScanning=0; uiScanning<16; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      scoeff[15-uiScanning] = piCoeff[ uiBlkPos ];
#else
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
#endif //QC_MDCS
    }
#if QC_MOD_LCEC
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#else
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();
#endif
    
#if CAVLC_COEF_LRG_BLK
    xCodeCoeff( scoeff, iBlockType, 4 );
#else
    xCodeCoeff4x4( scoeff, iBlockType );
#endif
  }
  else if ( uiSize == 8*8 )
  {
    for (uiScanning=0; uiScanning<64; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      scoeff[63-uiScanning] = piCoeff[ uiBlkPos ];
#else
      scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
#endif //QC_MDCS
    }
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
    
#if CAVLC_COEF_LRG_BLK
    xCodeCoeff( scoeff, iBlockType, 8 );
#else
    xCodeCoeff8x8( scoeff, iBlockType );
#endif
  }
  else
  {
    if(!pcCU->isIntra( uiAbsPartIdx ))
    {
#if CAVLC_COEF_LRG_BLK
      UInt uiBlSizeInBit = g_aucConvertToBit[uiBlSize] + 2;
      UInt uiWidthInBit = g_aucConvertToBit[uiWidth] + 2;
      for (uiScanning=0; uiScanning<uiNoCoeff; uiScanning++)
      {
#if QC_MDCS
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        uiBlkPos = ((uiBlkPos>>uiBlSizeInBit) <<uiWidthInBit) + (uiBlkPos&(uiBlSize-1));
        scoeff[uiNoCoeff-uiScanning-1] = piCoeff[ uiBlkPos ];
#else
        scoeff[uiNoCoeff-uiScanning-1] = piCoeff[((pucScan[uiScanning]>>uiBlSizeInBit)<<uiWidthInBit) + (pucScan[uiScanning]&(uiBlSize-1))];      
#endif
      }
#else
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      uiBlkPos = (uiBlkPos/8) * uiWidth + uiBlkPos%8;
      scoeff[63-uiScanning] = piCoeff[ uiBlkPos ];
#else
        scoeff[63-uiScanning] = piCoeff[(pucScan[uiScanning]/8)*uiWidth + (pucScan[uiScanning]%8)];      
#endif //QC_MDCS
      }
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
      return;
#endif
    }    
    
    if(pcCU->isIntra( uiAbsPartIdx ))
    {
#if CAVLC_COEF_LRG_BLK
      for (uiScanning=0; uiScanning<uiNoCoeff; uiScanning++)
      {
#if QC_MDCS
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        scoeff[uiNoCoeff-uiScanning-1] = piCoeff[ uiBlkPos ];
#else
        scoeff[uiNoCoeff-uiScanning-1] = piCoeff[ pucScan[ uiScanning ] ];
#endif
      }
#else
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      scoeff[63-uiScanning] = piCoeff[ uiBlkPos ];
#else
        scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
#endif //QC_MDCS
      }
      
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
#endif
    }

#if CAVLC_COEF_LRG_BLK 
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
    {
      iBlockType = eTType-2;
    }
    else
    {
      iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
    }
    xCodeCoeff( scoeff, iBlockType, uiBlSize);
#endif

    //#endif
  }
  
  if (uiCodeDCCoef == 1)
  {
    piCoeff[0] = dcCoeff;
  }
}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{
  
  xWriteFlag( uiCode );
}

#if TSB_ALF_HEADER
Void TEncCavlc::codeAlfFlagNum( UInt uiCode, UInt minValue )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (this->getMaxAlfCtrlDepth()*2));
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
  if(uiLength)
  {
    xWriteCode( uiCode - minValue, uiLength );
  }
}

Void TEncCavlc::codeAlfCtrlFlag( UInt uiSymbol )
{
  xWriteFlag( uiSymbol );
}
#endif

Void TEncCavlc::codeAlfUvlc( UInt uiCode )
{
  xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
  xWriteSvlc( iCode );
}
#if MTK_SAO
Void TEncCavlc::codeAoFlag( UInt uiCode )
{

  xWriteFlag( uiCode );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfFlag += 1;
#endif
}

Void TEncCavlc::codeAoUvlc( UInt uiCode )
{
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfUvlc += xWriteUvlc( uiCode );
  else
#endif
    xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAoSvlc( Int iCode )
{
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfSvlc += xWriteSvlc( iCode );
  else
#endif
    xWriteSvlc( iCode );
}

#endif

Void TEncCavlc::estBit( estBitsSbacStruct* pcEstBitsCabac, UInt uiCTXIdx, TextType eTType )
{
  // printf("error : no VLC mode support in this version\n");
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncCavlc::xWriteCode     ( UInt uiCode, UInt uiLength )
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncCavlc::xWriteUvlc     ( UInt uiCode )
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;
  
  assert ( uiTemp );
  
  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncCavlc::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;
  
  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
}

Void TEncCavlc::xWriteFlag( UInt uiCode )
{
  m_pcBitIf->write( uiCode, 1 );
}

Void TEncCavlc::xCheckCoeff( TCoeff* pcCoef, UInt uiSize, UInt uiDepth, UInt& uiNumofCoeff, UInt& uiPart )
{
  UInt ui = uiSize>>uiDepth;
  if( uiPart == 0 )
  {
    if( ui <= 4 )
    {
      UInt x, y;
      TCoeff* pCeoff = pcCoef;
      for( y=0 ; y<ui ; y++ )
      {
        for( x=0 ; x<ui ; x++ )
        {
          if( pCeoff[x] != 0 )
          {
            uiNumofCoeff++;
          }
        }
        pCeoff += uiSize;
      }
    }
    else
    {
      xCheckCoeff( pcCoef,                            uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //1st Part
      xCheckCoeff( pcCoef             + (ui>>1),      uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //2nd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize,           uiSize, uiDepth+1, uiNumofCoeff, uiPart ); uiPart++; //3rd Part
      xCheckCoeff( pcCoef + (ui>>1)*uiSize + (ui>>1), uiSize, uiDepth+1, uiNumofCoeff, uiPart );           //4th Part
    }
  }
  else
  {
    UInt x, y;
    TCoeff* pCeoff = pcCoef;
    for( y=0 ; y<ui ; y++ )
    {
      for( x=0 ; x<ui ; x++ )
      {
        if( pCeoff[x] != 0 )
        {
          uiNumofCoeff++;
        }
      }
      pCeoff += uiSize;
    }
  }
}

Void TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }
  xWriteFlag( uiSymbol ? 1 : 0 );
  if ( uiSymbol == 0 )
  {
    return;
  }
  
  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );
  
  while( --uiSymbol )
  {
    xWriteFlag( 1 );
  }
  if( bCodeLast )
  {
    xWriteFlag( 0 );
  }
  return;
}

Void TEncCavlc::xWriteExGolombLevel( UInt uiSymbol )
{
  if( uiSymbol )
  {
    xWriteFlag( 1 );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);
    
    while( --uiSymbol && ++uiCount < 13 )
    {
      xWriteFlag( 1 );
    }
    if( bNoExGo )
    {
      xWriteFlag( 0 );
    }
    else
    {
      xWriteEpExGolomb( uiSymbol, 0 );
    }
  }
  else
  {
    xWriteFlag( 0 );
  }
  return;
}

Void TEncCavlc::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xWriteFlag( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteFlag( 0 );
  while( uiCount-- )
  {
    xWriteFlag( (uiSymbol>>uiCount) & 1 );
  }
  return;
}

#if !QC_MOD_LCEC_RDOQ
UInt TEncCavlc::xLeadingZeros(UInt uiCode)
{
  UInt uiCount = 0;
  Int iDone = 0;
  
  if (uiCode)
  {
    while (!iDone)
    {
      uiCode >>= 1;
      if (!uiCode) iDone = 1;
      else uiCount++;
    }
  }
  return uiCount;
}
#endif

Void TEncCavlc::xWriteVlc(UInt uiTableNumber, UInt uiCodeNumber)
{
#if CAVLC_COEF_LRG_BLK
  assert( uiTableNumber<=13 );
#else
  assert( uiTableNumber<=11 );
#endif
  
  UInt uiTemp;
  UInt uiLength = 0;
  UInt uiCode = 0;
  
  if ( uiTableNumber < 5 )
  {
    if ((Int)uiCodeNumber < (6 * (1 << uiTableNumber)))
    {
      uiTemp = 1<<uiTableNumber;
      uiCode = uiTemp+uiCodeNumber%uiTemp;
      uiLength = 1+uiTableNumber+(uiCodeNumber>>uiTableNumber);
    }
    else
    {
      uiCode = uiCodeNumber - (6 * (1 << uiTableNumber)) + (1 << uiTableNumber);
      uiLength = (6-uiTableNumber)+1+2*xLeadingZeros(uiCode);
    }
  }
  else if (uiTableNumber < 8)
  {
    uiTemp = 1<<(uiTableNumber-4);
    uiCode = uiTemp+uiCodeNumber%uiTemp;
    uiLength = 1+(uiTableNumber-4)+(uiCodeNumber>>(uiTableNumber-4));
  }
  else if (uiTableNumber == 8)
  {
    assert( uiCodeNumber<=2 );
    if (uiCodeNumber == 0)
    {
      uiCode = 1;
      uiLength = 1;
    }
    else if (uiCodeNumber == 1)
    {
      uiCode = 1;
      uiLength = 2;
    }
    else if (uiCodeNumber == 2)
    {
      uiCode = 0;
      uiLength = 2;
    }
  }
  else if (uiTableNumber == 9)
  {
    if (uiCodeNumber == 0)
    {
      uiCode = 4;
      uiLength = 3;
    }
    else if (uiCodeNumber == 1)
    {
      uiCode = 10;
      uiLength = 4;
    }
    else if (uiCodeNumber == 2)
    {
      uiCode = 11;
      uiLength = 4;
    }
    else if (uiCodeNumber < 11)
    {
      uiCode = uiCodeNumber+21;
      uiLength = 5;
    }
    else
    {
      uiTemp = 1<<4;
      uiCode = uiTemp+(uiCodeNumber+5)%uiTemp;
      uiLength = 5+((uiCodeNumber+5)>>4);
    }
  }
  else if (uiTableNumber == 10)
  {
    uiCode = uiCodeNumber+1;
    uiLength = 1+2*xLeadingZeros(uiCode);
  }
  else if (uiTableNumber == 11)
  {
    if (uiCodeNumber == 0)
    {
      uiCode = 0;
      uiLength = 3;
    }
    else
    {
      uiCode = uiCodeNumber + 1;
      uiLength = 4;
    }
  }
#if CAVLC_COEF_LRG_BLK
  else if (uiTableNumber == 12)
  {
    uiCode = 64+(uiCodeNumber&0x3f);
    uiLength = 7+(uiCodeNumber>>6);
    if (uiLength>32)
    {
      xWriteCode(0, uiLength-32);
      uiLength  = 32;
    }
  }
  else if (uiTableNumber == 13)
  {
    uiTemp = 1<<4;
    uiCode = uiTemp+(uiCodeNumber&0x0f);
    uiLength = 5+(uiCodeNumber>>4);
  }
#endif

  xWriteCode(uiCode, uiLength);
}

#if CAVLC_COEF_LRG_BLK
/** Function for encoding a block of transform coeffcients in CAVLC.
 * \param scoeff pointer to transform coefficient buffer
 * \param n block type information, e.g. luma, chroma, intra, inter, etc. 
 * \param blSize block size
 * \returns 
 * This function performs encoding for a block of transform coefficient in CAVLC. 
 */
Void TEncCavlc::xCodeCoeff( TCoeff* scoeff, Int n, Int blSize)
{
  static const int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
  int i, noCoeff = blSize*blSize;
  unsigned int cn;
  int level,vlc,sign,done,last_pos,start;
  int run_done,maxrun,run,lev;
  int tmprun,vlc_adaptive=0;
  static const int atable[5] = {4,6,14,28,0xfffffff};
  int sum_big_coef = 0;
  Int tr1;

  /* Do the last coefficient first */
  i = 0;
  done = 0;
  while (!done && i < noCoeff)
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i++;
    }
  }
  if (i == noCoeff)
  {
    return;
  }

  last_pos = noCoeff-i-1;
  level = abs(scoeff[i]);
  lev = (level == 1) ? 0 : 1;

  if(blSize >= 8)
  {
    cn = xLastLevelInd(lev, last_pos, blSize);
    // ADAPT_VLC_NUM
    vlc = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
    xWriteVlc( vlc, cn );

    if ( m_bAdaptFlag ){
      // ADAPT_VLC_NUM
      cn = (blSize==8)? cn:(cn>>2);
      m_uiLastPosVlcIndex[n] += cn == m_uiLastPosVlcIndex[n] ? 0 : (cn < m_uiLastPosVlcIndex[n] ? -1 : 1);
    }
  }
  else
  {
    int x,y,cx,cy;
    int nTab = max(0,n-2);
    
    x = (lev<<4) + last_pos;
    cx = m_uiLPTableE4[nTab][x];
    xWriteVlc( 2, cx );
    
    if ( m_bAdaptFlag )
    {
      cy = Max( 0, cx-1 );
      y = m_uiLPTableD4[nTab][cy];
      m_uiLPTableD4[nTab][cy] = x;
      m_uiLPTableD4[nTab][cx] = y;
      m_uiLPTableE4[nTab][x] = cy;
      m_uiLPTableE4[nTab][y] = cx;
    }
  }

  sign = (scoeff[i++] < 0) ? 1 : 0;
  if (level > 1)
  {
    xWriteVlc( 0, ((level-2)<<1)+sign );
    tr1=0;
  }
  else
  {
    xWriteFlag( sign );
    tr1=1;
  }

  if (i < noCoeff)
  {
    /* Go into run mode */
    run_done = 0;
    const UInt *vlcTable = (n==2||n==5)? ((blSize<=8)? g_auiVlcTable8x8Intra:g_auiVlcTable16x16Intra):
      ((blSize<=8)? g_auiVlcTable8x8Inter:g_auiVlcTable16x16Inter);
    const UInt **pLumaRunTr1 = (blSize==4)? g_pLumaRunTr14x4:g_pLumaRunTr18x8;
    while ( !run_done )
    {
      maxrun = noCoeff-i-1;
      tmprun = Min(maxrun, 28);

      vlc = vlcTable[tmprun];
      run = 0;
      done = 0;
      while (!done)
      {
        if (!scoeff[i])
        {
          run++;
        }
        else
        {
          level = abs(scoeff[i]);
          lev = (level == 1) ? 0 : 1;

          if(n == 2 || n == 5)
          {
            cn = xRunLevelInd(lev, run, maxrun, pLumaRunTr1[tr1][tmprun]);
          }
          else
          {
            cn = xRunLevelIndInter(lev, run, maxrun);
          }

          xWriteVlc( vlc, cn );

          if (tr1==0 || level>=2)
          {
            tr1=0;
          }
          else if (tr1 < MAX_TR1)
          {
            tr1++;
          }

          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
            xWriteVlc( 0, ((level-2)<<1)+sign );

            sum_big_coef += level;
            if (blSize == 4 || i > switch_thr[n] || sum_big_coef > 2)
            {
              run_done = 1;
            }
          }
          else
          {
            xWriteFlag( sign );
          }
          run = 0;
          done = 1;
        }
        if (i == (noCoeff-1))
        {
          done = 1;
          run_done = 1;
          if (run)
          {
            if(n == 2 || n == 5)
            {
              cn = xRunLevelInd(0, run, maxrun, pLumaRunTr1[tr1][tmprun]);
            }
            else
            {
              cn = xRunLevelIndInter(0, run, maxrun);
            }

            xWriteVlc( vlc, cn );
          }
        }
        i++;
      }
    }
  }

  /* Code the rest in level mode */
  start = i;
  for ( i=start; i<noCoeff; i++ )
  {
    int tmp = abs(scoeff[i]);

    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      xWriteFlag( (scoeff[i] < 0) ? 1 : 0 );
      if (tmp > atable[vlc_adaptive])
      {
        vlc_adaptive++;
      }
    }
  }

  return;
}

#else
Void TEncCavlc::xCodeCoeff4x4(TCoeff* scoeff, Int n )
{
  Int i;
  UInt cn;
  Int level,vlc,sign,done,last_pos,start;
  Int run_done,maxrun,run,lev;
#if QC_MOD_LCEC
  Int vlc_adaptive=0;
#else
  Int tmprun, vlc_adaptive=0;
#endif
  static const int atable[5] = {4,6,14,28,0xfffffff};
  Int tmp;
#if QC_MOD_LCEC
  Int nTab = max(0,n-2);
  Int tr1;
#endif
  
  /* Do the last coefficient first */
  i = 0;
  done = 0;
  
  while (!done && i < 16)
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i++;
    }
  }
  if (i == 16)
  {
    return;
  }
  
  last_pos = 15-i;
  level = abs(scoeff[i]);
  lev = (level == 1) ? 0 : 1;
  
#if QC_MOD_LCEC
  if (level>1)
  {
    tr1=0;
  }
  else
  {
    tr1=1;
  }
#endif

  {
    int x,y,cx,cy,vlcNum;
    int vlcTable[3] = {2,2,2};
    
    x = 16*lev + last_pos;
    
#if QC_MOD_LCEC
    cx = m_uiLPTableE4[nTab][x];
    vlcNum = vlcTable[nTab];
#else
    cx = m_uiLPTableE4[n][x];
    vlcNum = vlcTable[n];
#endif
    
    xWriteVlc( vlcNum, cx );
    
    if ( m_bAdaptFlag )
    {
      cy = Max( 0, cx-1 );
#if QC_MOD_LCEC
      y = m_uiLPTableD4[nTab][cy];
      m_uiLPTableD4[nTab][cy] = x;
      m_uiLPTableD4[nTab][cx] = y;
      m_uiLPTableE4[nTab][x] = cy;
      m_uiLPTableE4[nTab][y] = cx;
#else
      y = m_uiLPTableD4[n][cy];
      m_uiLPTableD4[n][cy] = x;
      m_uiLPTableD4[n][cx] = y;
      m_uiLPTableE4[n][x] = cy;
      m_uiLPTableE4[n][y] = cx;
#endif
    }
  }
  
  sign = (scoeff[i] < 0) ? 1 : 0;
  if (level > 1)
  {
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
  }
  i++;
  
  if (i < 16)
  {
    /* Go into run mode */
    run_done = 0;
    while (!run_done)
    {
      maxrun = 15-i;
#if QC_MOD_LCEC
      if ( n == 2 )
        vlc = g_auiVlcTable8x8Intra[maxrun];
      else
        vlc = g_auiVlcTable8x8Inter[maxrun];
#else
      tmprun = maxrun;
      if (maxrun > 27)
      {
        vlc = 3;
        tmprun = 28;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }
#endif
      
      run = 0;
      done = 0;
      while (!done)
      {
        if (!scoeff[i])
        {
          run++;
        }
        else
        {
          level = abs(scoeff[i]);
          lev = (level == 1) ? 0 : 1;
#if QC_MOD_LCEC
          if ( n == 2 ){
            cn = xRunLevelInd(lev, run, maxrun, g_auiLumaRunTr14x4[tr1][maxrun]);
          }
          else{
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(lev, run, maxrun);
#else
            cn = g_auiLumaRun8x8[maxrun][lev][run];
#endif
          }
#else
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
#endif
            xWriteVlc( vlc, cn );
          
#if QC_MOD_LCEC
          if (tr1>0 && tr1 < MAX_TR1)
          {
            tr1++;
          }
#endif
          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
            xWriteVlc( 0, 2*(level-2)+sign );
            run_done = 1;
          }
          else
          {
            xWriteFlag( sign );
          }
          
          run = 0;
          done = 1;
        }
        if (i == 15)
        {
          done = 1;
          run_done = 1;
          if (run)
          {
#if QC_MOD_LCEC
            if (n==2){
              cn=xRunLevelInd(0, run, maxrun, g_auiLumaRunTr14x4[tr1][maxrun]);
            }
            else{
#if RUNLEVEL_TABLE_CUT
              cn = xRunLevelIndInter(0, run, maxrun);
#else
              cn = g_auiLumaRun8x8[maxrun][0][run];
#endif
            }
#else
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
#endif
            xWriteVlc( vlc, cn );
          }
        }
        i++;
      }
    }
  }
  
  /* Code the rest in level mode */
  start = i;
  for ( i=start; i<16; i++ )
  {
    tmp = abs(scoeff[i]);
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
    }
    if ( tmp > atable[vlc_adaptive] )
    {
      vlc_adaptive++;
    }
  }
  return;
}

Void TEncCavlc::xCodeCoeff8x8( TCoeff* scoeff, Int n )
{
  int i;
  unsigned int cn;
  int level,vlc,sign,done,last_pos,start;
  int run_done,maxrun,run,lev;
#if QC_MOD_LCEC
  int vlc_adaptive=0;
#else
  int tmprun,vlc_adaptive=0;
#endif
  static const int atable[5] = {4,6,14,28,0xfffffff};
  int tmp;
#if QC_MOD_LCEC
  Int tr1;
#endif
  
  static const int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
  int sum_big_coef = 0;
  
  
  /* Do the last coefficient first */
  i = 0;
  done = 0;
  while (!done && i < 64)
  {
    if (scoeff[i])
    {
      done = 1;
    }
    else
    {
      i++;
    }
  }
  if (i == 64)
  {
    return;
  }
  
  last_pos = 63-i;
  level = abs(scoeff[i]);
  lev = (level == 1) ? 0 : 1;
  
  {
    int x,y,cx,cy,vlcNum;
    x = 64*lev + last_pos;
    
    cx = m_uiLPTableE8[n][x];
    // ADAPT_VLC_NUM
    vlcNum = g_auiLastPosVlcNum[n][Min(16,m_uiLastPosVlcIndex[n])];
    xWriteVlc( vlcNum, cx );
    
    if ( m_bAdaptFlag )
    {
      // ADAPT_VLC_NUM
      m_uiLastPosVlcIndex[n] += cx == m_uiLastPosVlcIndex[n] ? 0 : (cx < m_uiLastPosVlcIndex[n] ? -1 : 1);
      cy = Max(0,cx-1);
      y = m_uiLPTableD8[n][cy];
      m_uiLPTableD8[n][cy] = x;
      m_uiLPTableD8[n][cx] = y;
      m_uiLPTableE8[n][x] = cy;
      m_uiLPTableE8[n][y] = cx;
    }
  }
  
  sign = (scoeff[i] < 0) ? 1 : 0;
  if (level > 1)
  {
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
  }
  i++;
#if QC_MOD_LCEC
  if (level>1){
    tr1=0;
  }
  else
  {
    tr1=1;
  }
#endif
  
  if (i < 64)
  {
    /* Go into run mode */
    run_done = 0;
    while ( !run_done )
    {
      maxrun = 63-i;
#if QC_MOD_LCEC
      if(n == 2 || n == 5)
        vlc = g_auiVlcTable8x8Intra[Min(maxrun,28)];
      else
        vlc = g_auiVlcTable8x8Inter[Min(maxrun,28)];
#else
      tmprun = maxrun;
      if (maxrun > 27)
      {
        vlc = 3;
        tmprun = 28;
      }
      else
      {
        vlc = g_auiVlcTable8x8[maxrun];
      }
#endif   
      
      run = 0;
      done = 0;
      while (!done)
      {
        if (!scoeff[i])
        {
          run++;
        }
        else
        {
          level = abs(scoeff[i]);
          lev = (level == 1) ? 0 : 1;
#if QC_MOD_LCEC
          if(n == 2 || n == 5)
            cn = xRunLevelInd(lev, run, maxrun, g_auiLumaRunTr18x8[tr1][min(maxrun,28)]);
          else
#if RUNLEVEL_TABLE_CUT
            cn = xRunLevelIndInter(lev, run, maxrun);
#else
            cn = g_auiLumaRun8x8[min(maxrun,28)][lev][run];
#endif
#else
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
#endif
          xWriteVlc( vlc, cn );
          
#if QC_MOD_LCEC
          if (tr1==0 || level >=2)
          {
            tr1=0;
          }
          else if (tr1 < MAX_TR1)
          {
            tr1++;
          }
#endif
          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
            xWriteVlc( 0, 2*(level-2)+sign );
            
            sum_big_coef += level;
            if (i > switch_thr[n] || sum_big_coef > 2)
            {
              run_done = 1;
            }
          }
          else
          {
            xWriteFlag( sign );
          }
          run = 0;
          done = 1;
        }
        if (i == 63)
        {
          done = 1;
          run_done = 1;
          if (run)
          {
#if QC_MOD_LCEC
            if(n == 2 || n == 5)
              cn=xRunLevelInd(0, run, maxrun, g_auiLumaRunTr18x8[tr1][min(maxrun,28)]);
            else
#if RUNLEVEL_TABLE_CUT
              cn = xRunLevelIndInter(0, run, maxrun);
#else
              cn = g_auiLumaRun8x8[min(maxrun,28)][0][run];
#endif
#else
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
#endif
            xWriteVlc( vlc, cn );
          }
        }
        i++;
      }
    }
  }
  
  /* Code the rest in level mode */
  start = i;
  for ( i=start; i<64; i++ )
  {
    tmp = abs(scoeff[i]);
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
    }
    if (tmp>atable[vlc_adaptive])
    {
      vlc_adaptive++;
    }
  }
  
  return;
}
#endif

