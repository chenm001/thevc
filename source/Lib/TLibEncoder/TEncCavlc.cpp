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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "TEncCavlc.h"

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

#if LCEC_STAT
  m_uiBitHLS                 = 0;
  m_uiBitCIP                 = 0;
  m_uiBitindexROT            = 0;
  m_uiBitMVPId               = 0;
  m_uiBitPartSize            = 0;
  m_uiBitPredMode            = 0;
  m_uiBitMergeFlag           = 0;
  m_uiBitMergeIndex          = 0;
  m_uiBitIntraFiltFlag       = 0;
  m_uiBitAlfCtrlFlag         = 0;
  m_uiBitAlfCtrlDepth        = 0;
  m_uiBitSkipFlag            = 0;
  m_uiBitCurrSplitFlag       = 0;
  m_uiBitTransformSubdivFlag = 0;
  m_uiBitQtCbf               = 0;
  m_uiBitTransformIdx        = 0;
  m_uiBitPlanarVlc           = 0;
  m_uiBitIntraDir            = 0;
  m_uiBitIRefFrmIdx          = 0;  
  m_uiBitMVD                 = 0;
#ifdef DCM_PBIC
  m_uiBitMVDICD              = 0;
#endif
  m_uiBitDeltaQP             = 0;
  m_uiBitCbf                 = 0;
  m_uiBitAlfFlag             = 0;
  m_uiBitAlfUvlc             = 0;
  m_uiBitAlfSvlc             = 0;
  m_uiBitMVPIdx              = 0;
#ifdef DCM_PBIC
  m_uiBitICPIdx              = 0;
#endif
  m_uiBitPlanarInfo          = 0;
  m_uiBitInterDir            = 0;
#if LCEC_PHASE2
  m_uiBitMI                  = 0;
#endif
  m_uiBitSF                  = 0;
  m_uiBitCoeff               = 0;
#if LCEC_PHASE2
  m_uiBitCbp                 = 0;
#endif
#endif
}

TEncCavlc::~TEncCavlc()
{
}

#if LCEC_STAT
#define NUM_PASSES 2
Void TEncCavlc::statistics(Bool bResetFlag, UInt uiPrintVar)
{
  Int i,j;
  if (bResetFlag)
  {
    m_uiBitHLS                 = 0;
    m_uiBitCIP                 = 0;
    m_uiBitindexROT            = 0;
    m_uiBitMVPId               = 0;
    m_uiBitPartSize            = 0;
    m_uiBitPredMode            = 0;
    
    m_uiBitMergeFlag           = 0; 
    m_uiBitMergeIndex          = 0; 
    m_uiBitIntraFiltFlag       = 0;
    m_uiBitAlfCtrlFlag         = 0;
    m_uiBitAlfCtrlDepth        = 0;
    
    m_uiBitSkipFlag            = 0;
    m_uiBitCurrSplitFlag       = 0; 
    m_uiBitTransformSubdivFlag = 0;
    m_uiBitQtCbf               = 0; 
    m_uiBitTransformIdx        = 0; 
    
    m_uiBitPlanarVlc           = 0;
    m_uiBitIntraDir            = 0;
    m_uiBitIRefFrmIdx          = 0;
    m_uiBitMVD                 = 0;
#ifdef DCM_PBIC
    m_uiBitMVDICD              = 0;
#endif
    m_uiBitDeltaQP             = 0;
    
    m_uiBitCbf                 = 0; 
#if LCEC_PHASE2
    m_uiBitCbp                 = 0;
#endif
    m_uiBitAlfFlag             = 0;
    m_uiBitAlfUvlc             = 0;
    m_uiBitAlfSvlc             = 0;
    m_uiBitMVPIdx              = 0;
#ifdef DCM_PBIC
    m_uiBitICPIdx              = 0;
#endif
    
    m_uiBitPlanarInfo          = 0;
    m_uiBitInterDir            = 0;
#if LCEC_PHASE2
    m_uiBitMI                  = 0;
#endif
    m_uiBitSF                  = 0;
    m_uiBitCoeff               = 0;
  }


  if (uiPrintVar)
  {
    Int i;
    FILE *fp;
   
    UInt uiTotalBits = 0;
              
    /* Divide some of the variables by by number of passes */
    m_uiBitCIP = m_uiBitCIP/NUM_PASSES;
    m_uiBitindexROT = m_uiBitindexROT/NUM_PASSES;
    m_uiBitMVPId = m_uiBitMVPId/NUM_PASSES;
    m_uiBitPartSize = m_uiBitPartSize/NUM_PASSES;
    m_uiBitPredMode = m_uiBitPredMode/NUM_PASSES;    
    m_uiBitMergeFlag = m_uiBitMergeFlag/NUM_PASSES;
    m_uiBitMergeIndex = m_uiBitMergeIndex/NUM_PASSES;
    m_uiBitIntraFiltFlag = m_uiBitIntraFiltFlag/NUM_PASSES;    
    m_uiBitSkipFlag = m_uiBitSkipFlag/NUM_PASSES;
    m_uiBitCurrSplitFlag = m_uiBitCurrSplitFlag/NUM_PASSES;
    m_uiBitTransformSubdivFlag = m_uiBitTransformSubdivFlag/NUM_PASSES;
    m_uiBitQtCbf = m_uiBitQtCbf/NUM_PASSES;   
    m_uiBitTransformIdx = m_uiBitTransformIdx/NUM_PASSES;
    m_uiBitPlanarVlc = m_uiBitPlanarVlc/NUM_PASSES;
    m_uiBitIntraDir = m_uiBitIntraDir/NUM_PASSES;
    m_uiBitIRefFrmIdx = m_uiBitIRefFrmIdx/NUM_PASSES;
    m_uiBitMVD = m_uiBitMVD/NUM_PASSES;
#ifdef DCM_PBIC
    m_uiBitMVDICD = m_uiBitMVDICD/NUM_PASSES;
#endif
    m_uiBitDeltaQP = m_uiBitDeltaQP/NUM_PASSES;
    m_uiBitCbf = m_uiBitCbf/NUM_PASSES;
#if LCEC_PHASE2
    m_uiBitCbp = m_uiBitCbp/NUM_PASSES;
#endif
    m_uiBitMVPIdx = m_uiBitMVPIdx/NUM_PASSES;
#ifdef DCM_PBIC
    m_uiBitICPIdx = m_uiBitICPIdx/NUM_PASSES;
#endif
    m_uiBitPlanarInfo = m_uiBitPlanarInfo/NUM_PASSES;
    m_uiBitInterDir = m_uiBitInterDir/NUM_PASSES;
#if LCEC_PHASE2
    m_uiBitMI = m_uiBitMI/NUM_PASSES;
#endif
    m_uiBitCoeff = m_uiBitCoeff/NUM_PASSES;


    /* Calculate total bit usage */
    uiTotalBits += m_uiBitHLS;           
    uiTotalBits += m_uiBitCIP;
    uiTotalBits += m_uiBitindexROT;
    uiTotalBits += m_uiBitMVPId;
    uiTotalBits += m_uiBitPartSize;
    uiTotalBits += m_uiBitPredMode;
    
    uiTotalBits += m_uiBitMergeFlag;
    uiTotalBits += m_uiBitMergeIndex;
    uiTotalBits += m_uiBitIntraFiltFlag;
    uiTotalBits += m_uiBitAlfCtrlFlag;
    uiTotalBits += m_uiBitAlfCtrlDepth;
    
    uiTotalBits += m_uiBitSkipFlag;
    uiTotalBits += m_uiBitCurrSplitFlag;
    uiTotalBits += m_uiBitTransformSubdivFlag;
    uiTotalBits += m_uiBitQtCbf;   
    uiTotalBits += m_uiBitTransformIdx;
   
    uiTotalBits += m_uiBitPlanarVlc;
    uiTotalBits += m_uiBitIntraDir;
    uiTotalBits += m_uiBitIRefFrmIdx;
    uiTotalBits += m_uiBitMVD;
#ifdef DCM_PBIC
    uiTotalBits += m_uiBitMVDICD;
#endif
    uiTotalBits += m_uiBitDeltaQP;

    uiTotalBits += m_uiBitCbf;
#if LCEC_PHASE2
    uiTotalBits += m_uiBitCbp;
#endif
    uiTotalBits += m_uiBitAlfFlag;
    uiTotalBits += m_uiBitAlfUvlc;  
    uiTotalBits += m_uiBitAlfSvlc;
    uiTotalBits += m_uiBitMVPIdx;
#ifdef DCM_PBIC
    uiTotalBits += m_uiBitICPIdx;
#endif

    uiTotalBits += m_uiBitPlanarInfo;
    uiTotalBits += m_uiBitInterDir;
#if LCEC_PHASE2
    uiTotalBits += m_uiBitMI;
#endif
    uiTotalBits += m_uiBitSF;
    uiTotalBits += m_uiBitCoeff;

    /* Printout statistics */
    printf("\n");
    printf("m_uiBitHLS =                 %12d %6.1f\n",m_uiBitHLS,100.0*(float)m_uiBitHLS/(float)uiTotalBits);
    printf("m_uiBitCIP =                 %12d %6.1f\n",m_uiBitCIP,100.0*(float)m_uiBitCIP/(float)uiTotalBits);
    printf("m_uiBitindexROT =            %12d %6.1f\n",m_uiBitindexROT,100.0*(float)m_uiBitindexROT/(float)uiTotalBits);
    printf("m_uiBitMVPId =               %12d %6.1f\n",m_uiBitMVPId,100.0*(float)m_uiBitMVPId/(float)uiTotalBits);
    printf("m_uiBitPartSize =            %12d %6.1f\n",m_uiBitPartSize,100.0*(float)m_uiBitPartSize/(float)uiTotalBits);
    printf("m_uiBitPredMode =            %12d %6.1f\n",m_uiBitPredMode,100.0*(float)m_uiBitPredMode/(float)uiTotalBits);
    
    printf("m_uiBitMergeFlag =           %12d %6.1f\n",m_uiBitMergeFlag,100.0*(float)m_uiBitMergeFlag/(float)uiTotalBits);
    printf("m_uiBitMergeIndex =          %12d %6.1f\n",m_uiBitMergeIndex,100.0*(float)m_uiBitMergeIndex/(float)uiTotalBits);
    printf("m_uiBitIntraFiltFlag =       %12d %6.1f\n",m_uiBitIntraFiltFlag,100.0*(float)m_uiBitIntraFiltFlag/(float)uiTotalBits);
    printf("m_uiBitAlfCtrlFlag =         %12d %6.1f\n",m_uiBitAlfCtrlFlag,100.0*(float)m_uiBitAlfCtrlFlag/(float)uiTotalBits);
    printf("m_uiBitAlfCtrlDepth =        %12d %6.1f\n",m_uiBitAlfCtrlDepth,100.0*(float)m_uiBitAlfCtrlDepth/(float)uiTotalBits);
    
    printf("m_uiBitSkipFlag =            %12d %6.1f\n",m_uiBitSkipFlag,100.0*(float)m_uiBitSkipFlag/(float)uiTotalBits);
    printf("m_uiBitCurrSplitFlag  =      %12d %6.1f\n",m_uiBitCurrSplitFlag,100.0*(float)m_uiBitCurrSplitFlag/(float)uiTotalBits);
    printf("m_uiBitTransformSubdivFlag = %12d %6.1f\n",m_uiBitTransformSubdivFlag,100.0*(float)m_uiBitTransformSubdivFlag/(float)uiTotalBits);
    printf("m_uiBitQtCbf =               %12d %6.1f\n",m_uiBitQtCbf,100.0*(float)m_uiBitQtCbf/(float)uiTotalBits);
    printf("m_uiBitTransformIdx =        %12d %6.1f\n",m_uiBitTransformIdx,100.0*(float)m_uiBitTransformIdx/(float)uiTotalBits);
    
    printf("m_uiBitPlanarVlc =           %12d %6.1f\n",m_uiBitPlanarVlc,100.0*(float)m_uiBitPlanarVlc/(float)uiTotalBits);
    printf("m_uiBitIntraDir =            %12d %6.1f\n",m_uiBitIntraDir,100.0*(float)m_uiBitIntraDir/(float)uiTotalBits);
    printf("m_uiBitIRefFrmIdx =          %12d %6.1f\n",m_uiBitIRefFrmIdx,100.0*(float)m_uiBitIRefFrmIdx/(float)uiTotalBits);
    printf("m_uiBitMVD =                 %12d %6.1f\n",m_uiBitMVD,100.0*(float)m_uiBitMVD/(float)uiTotalBits);
#ifdef DCM_PBIC
    printf("m_uiBitMVDICD =              %12d %6.1f\n",m_uiBitMVDICD,100.0*(float)m_uiBitMVDICD/(float)uiTotalBits);
#endif
    printf("m_uiBitDeltaQP =             %12d %6.1f\n",m_uiBitDeltaQP,100.0*(float)m_uiBitDeltaQP/(float)uiTotalBits);
    
    printf("m_uiBitCbf =                 %12d %6.1f\n",m_uiBitCbf,100.0*(float)m_uiBitCbf/(float)uiTotalBits);
#if LCEC_PHASE2
    printf("m_uiBitCbp =                 %12d %6.1f\n",m_uiBitCbp,100.0*(float)m_uiBitCbp/(float)uiTotalBits);
#endif
    printf("m_uiBitAlfFlag =             %12d %6.1f\n",m_uiBitAlfFlag,100.0*(float)m_uiBitAlfFlag/(float)uiTotalBits);
    printf("m_uiBitAlfUvlc =             %12d %6.1f\n",m_uiBitAlfUvlc,100.0*(float)m_uiBitAlfUvlc/(float)uiTotalBits);
    printf("m_uiBitAlfSvlc =             %12d %6.1f\n",m_uiBitAlfSvlc,100.0*(float)m_uiBitAlfSvlc/(float)uiTotalBits);
    printf("m_uiBitMVPIdx =              %12d %6.1f\n",m_uiBitMVPIdx,100.0*(float)m_uiBitMVPIdx/(float)uiTotalBits);
#ifdef DCM_PBIC
    printf("m_uiBitICPIdx =              %12d %6.1f\n",m_uiBitICPIdx,100.0*(float)m_uiBitICPIdx/(float)uiTotalBits);
#endif
    
    printf("m_uiBitPlanarInfo =          %12d %6.1f\n",m_uiBitPlanarInfo,100.0*(float)m_uiBitPlanarInfo/(float)uiTotalBits);
    printf("m_uiBitInterDir =            %12d %6.1f\n",m_uiBitInterDir,100.0*(float)m_uiBitInterDir/(float)uiTotalBits);
#if LCEC_PHASE2
    printf("m_uiBitMI =                  %12d %6.1f\n",m_uiBitMI,100.0*(float)m_uiBitMI/(float)uiTotalBits);
#endif
    printf("m_uiBitSF =                  %12d %6.1f\n",m_uiBitSF,100.0*(float)m_uiBitSF/(float)uiTotalBits);
    printf("m_uiBitCoeff =               %12d %6.1f\n",m_uiBitCoeff,100.0*(float)m_uiBitCoeff/(float)uiTotalBits);

    printf("uiTotalBits =                %12d\n",uiTotalBits);

  } //if uiPrintFlag
}
#endif

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy()
{
  m_bRunLengthCoding = ! m_pcSlice->isIntra();
  m_uiRun = 0;
  ::memcpy(m_uiLPTableE8, g_auiLPTableE8, 10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableD8, g_auiLPTableD8, 10*128*sizeof(UInt));
  ::memcpy(m_uiLPTableE4, g_auiLPTableE4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLPTableD4, g_auiLPTableD4, 3*32*sizeof(UInt));
  ::memcpy(m_uiLastPosVlcIndex, g_auiLastPosVlcIndex, 10*sizeof(UInt));

#if LCEC_PHASE2
  ::memcpy(m_uiCBPTableE, g_auiCBPTableE, 2*8*sizeof(UInt));
  ::memcpy(m_uiCBPTableD, g_auiCBPTableD, 2*8*sizeof(UInt));
  m_uiCbpVlcIdx[0] = 0;
  m_uiCbpVlcIdx[1] = 0;
#endif

#if LCEC_PHASE2
  ::memcpy(m_uiMI1TableE, g_auiMI1TableE, 8*sizeof(UInt));
  ::memcpy(m_uiMI1TableD, g_auiMI1TableD, 8*sizeof(UInt));
  ::memcpy(m_uiMI2TableE, g_auiMI2TableE, 15*sizeof(UInt));
  ::memcpy(m_uiMI2TableD, g_auiMI2TableD, 15*sizeof(UInt));

#if MS_NO_BACK_PRED_IN_B0
  if ( m_pcSlice->getNoBackPredFlag() )
  {
    ::memcpy(m_uiMI1TableE, g_auiMI1TableENoL1, 8*sizeof(UInt));
    ::memcpy(m_uiMI1TableD, g_auiMI1TableDNoL1, 8*sizeof(UInt));
    ::memcpy(m_uiMI2TableE, g_auiMI2TableENoL1, 15*sizeof(UInt));
    ::memcpy(m_uiMI2TableD, g_auiMI2TableDNoL1, 15*sizeof(UInt));
  }
#endif

  m_uiMITableVlcIdx = 0;  

#endif


}

#if LCEC_PHASE2
UInt* TEncCavlc::GetLP8Table()
{   
  return &m_uiLPTableE8[0][0];
}

UInt* TEncCavlc::GetLP4Table()
{   
  return &m_uiLPTableE4[0][0];
}
#endif

#if LCEC_STAT
Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
  
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_PPS, 5);
  m_uiBitHLS += 8;
#endif
  return;
}



Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
  
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_SPS, 5);
  m_uiBitHLS += 8;
#endif
  // Structure
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getWidth () );
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getHeight() );

  m_uiBitHLS += xWriteUvlc  ( pcSPS->getPad (0) );
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getPad (1) );

  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxCUWidth ()   );
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxCUHeight()   );
#if HHI_RQT
  if (pcSPS->getQuadtreeTUFlag())
  {
    m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxCUDepth () - g_uiAddCUDepth );
  }
  else
#endif
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxCUDepth ()-1 ); //xWriteUvlc ( pcSPS->getMaxCUDepth ()-g_uiAddCUDepth );

  // Transform
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMinTrDepth()   );
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxTrDepth()   );

#if HHI_RQT
  xWriteFlag( pcSPS->getQuadtreeTUFlag() );
  m_uiBitHLS += 1;
  if( pcSPS->getQuadtreeTUFlag() )
  {
    m_uiBitHLS += xWriteUvlc( pcSPS->getQuadtreeTULog2MinSize() - 2 );
    if( pcSPS->getQuadtreeTULog2MinSize() < 6 )
    {
      m_uiBitHLS += xWriteUvlc( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize() );
    }
#if HHI_RQT_DEPTH
    m_uiBitHLS += xWriteUvlc  ( pcSPS->getQuadtreeTUMaxDepth () - 1 );
#endif	
  }
#endif

  // Max transform size
  m_uiBitHLS += xWriteUvlc  ( pcSPS->getMaxTrSize() == 2 ? 0 : g_aucConvertToBit[pcSPS->getMaxTrSize()]+1 );

  // Tools
  xWriteFlag  ( (pcSPS->getUseALF ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseDQP ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseWPG () || pcSPS->getUseWPO ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseQBO ()) ? 1 : 0 );
  m_uiBitHLS += 5;
#ifdef QC_AMVRES
  xWriteFlag  ( (pcSPS->getUseAMVRes ()) ? 1 : 0 );
  m_uiBitHLS += 1;
#endif
#if HHI_ALLOW_CIP_SWITCH
  xWriteFlag  ( (pcSPS->getUseCIP ()) ? 1 : 0 ); // BB:
  m_uiBitHLS += 1;
#endif
  xWriteFlag	( (pcSPS->getUseROT ()) ? 1 : 0 ); // BB:
  m_uiBitHLS += 1;
#if HHI_AIS
  xWriteFlag  ( (pcSPS->getUseAIS ()) ? 1 : 0 ); // BB:
  m_uiBitHLS += 1;
#endif
#if HHI_MRG
  xWriteFlag  ( (pcSPS->getUseMRG ()) ? 1 : 0 ); // SOPH:
  m_uiBitHLS += 1;
#endif
#if HHI_IMVP
   xWriteFlag ( (pcSPS->getUseIMP ()) ? 1 : 0 ); // SOPH:
   m_uiBitHLS += 1;
#endif
#ifdef DCM_PBIC
  xWriteFlag ( (pcSPS->getUseIC ()) ? 1 : 0 );
  m_uiBitHLS += 1;
#endif
  xWriteFlag  ( (pcSPS->getUseAMP ()) ? 1 : 0 );
  m_uiBitHLS += 1;
#if HHI_RMP_SWITCH
  xWriteFlag  ( (pcSPS->getUseRMP ()) ? 1 : 0 );
  m_uiBitHLS += 1;
#endif
  
  // write number of taps for DIF
  m_uiBitHLS += xWriteUvlc  ( (pcSPS->getDIFTap ()>>1)-2 ); // 4, 6, 8, 10, 12
#if SAMSUNG_CHROMA_IF_EXT
  m_uiBitHLS += xWriteUvlc  ( (pcSPS->getDIFTapC()>>1)-1 ); // 4, 6, 8, 10, 12
#endif

  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
    m_uiBitHLS += 1;
  }

  // Bit-depth information
  m_uiBitHLS += xWriteUvlc( pcSPS->getBitDepth() - 8 );
  m_uiBitHLS += xWriteUvlc( pcSPS->getBitIncrement() );

  m_uiBitHLS += xWriteCode( pcSPS->getBalancedCPUs(), 8);

}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
  
#if HHI_NAL_UNIT_SYNTAX
  // here someone can add an appropriated NalRefIdc type 
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_CODED_SLICE, 5);
  m_uiBitHLS += 8;
#endif
  m_uiBitHLS += xWriteCode  (pcSlice->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  m_uiBitHLS += xWriteUvlc  (pcSlice->getSliceType() );
  m_uiBitHLS += xWriteSvlc  (pcSlice->getSliceQp() );
  
  xWriteFlag  (pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 ? 1 : 0);
  m_uiBitHLS += 1;
  if( pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 )
  {
    xWriteFlag( pcSlice->getSymbolMode() > 1 ? 1 : 0 );
    m_uiBitHLS += 1;
    if( pcSlice->getSymbolMode() )
    {
      xWriteFlag( pcSlice->getMultiCodeword() ? 1 : 0 );
      m_uiBitHLS += 1;
    }
    if( pcSlice->getSymbolMode() == 2 && ! pcSlice->getMultiCodeword() )
    {
      m_uiBitHLS += xWriteUvlc( pcSlice->getMaxPIPEDelay() >> 6 );
    }
  }
  else
  {
    xWriteFlag( pcSlice->getSymbolMode() > 0 ? 1 : 0 );
    m_uiBitHLS += 1;
  }

  if (!pcSlice->isIntra())
  {
    xWriteFlag  (pcSlice->isReferenced() ? 1 : 0);
    m_uiBitHLS += 1;
#ifdef ROUNDING_CONTROL_BIPRED
	xWriteFlag  (pcSlice->isRounding() ? 1 : 0);
  m_uiBitHLS += 1;
#endif
  }

  xWriteFlag  (pcSlice->getLoopFilterDisable());
  m_uiBitHLS += 1;

  if (!pcSlice->isIntra())
  {
    m_uiBitHLS += xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_0 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_0), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
  }
  if (pcSlice->isInterB())
  {
    m_uiBitHLS += xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_1 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_1), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
  }

  xWriteFlag  (pcSlice->getDRBFlag() ? 1 : 0 );
  m_uiBitHLS += 1;
  if ( !pcSlice->getDRBFlag() )
  {
    m_uiBitHLS += xWriteCode  (pcSlice->getERBIndex(), 2);
  }

  if (!pcSlice->isIntra())   // weighted prediction information
  {
    Int  iNumPredDir = pcSlice->isInterP() ? 1 : 2;

    if (pcSlice->getSPS()->getUseWPG() || pcSlice->getSPS()->getUseWPO())
    {
      for (Int n=0; n<iNumPredDir; n++)
      {
        RefPicList eRefPicList = (RefPicList)n;

        UInt uiWpMode =  pcSlice->getWPmode(eRefPicList);
        m_uiBitHLS += xWriteCode  (uiWpMode, 1 );

        if (uiWpMode)
        {
          EFF_MODE eEffMode = (pcSlice->getSPS()->getUseWPG()? EFF_WP_SO : EFF_WP_O);
          UInt uiWeight,uiOffset;

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 0)-32);
          m_uiBitHLS += xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 0));
          m_uiBitHLS += xWriteUvlc( uiOffset );

#if GRF_WP_CHROMA
          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 1)-32);
          m_uiBitHLS += xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 1));
          m_uiBitHLS += xWriteUvlc( uiOffset );

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 2)-32);
          m_uiBitHLS += xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 2));
          m_uiBitHLS += xWriteUvlc( uiOffset );
#endif
        }
      }
    }
  }

#if HHI_INTERP_FILTER
  m_uiBitHLS += xWriteUvlc  ( pcSlice->getInterpFilterType() );
#endif

#if AMVP_NEIGH_COL
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    xWriteFlag( pcSlice->getColDir() );
    m_uiBitHLS += 1;
  }
#endif
#ifdef EDGE_BASED_PREDICTION
  xWriteFlag(pcSlice->getEdgePredictionEnable());
  m_uiBitHLS += 1;
  if( pcSlice->getEdgePredictionEnable() )
    m_uiBitHLS += xWriteCode((pcSlice->getEdgeDetectionThreshold()>>8), 8);
#endif //EDGE_BASED_PREDICTION
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
  xWriteFlag( uilsLast );
  m_uiBitHLS += 1;
}

Void TEncCavlc::codeSliceFinish ()
{
  if ( m_bRunLengthCoding && m_uiRun)
  {
    m_uiBitHLS += xWriteUvlc(m_uiRun);
  }
}
#else

Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_PPS, 5);
#endif
  return;
}



Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
#if HHI_NAL_UNIT_SYNTAX
  // uiFirstByte
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_SPS, 5);
#endif
  // Structure
  xWriteUvlc  ( pcSPS->getWidth () );
  xWriteUvlc  ( pcSPS->getHeight() );

  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );

  xWriteUvlc  ( pcSPS->getMaxCUWidth ()   );
  xWriteUvlc  ( pcSPS->getMaxCUHeight()   );
#if HHI_RQT
  if( pcSPS->getQuadtreeTUFlag() )
  {
    xWriteUvlc  ( pcSPS->getMaxCUDepth() - g_uiAddCUDepth );
  }
  else
#endif
  xWriteUvlc  ( pcSPS->getMaxCUDepth ()-1 ); //xWriteUvlc ( pcSPS->getMaxCUDepth ()-g_uiAddCUDepth );
  
  // Transform
  xWriteUvlc  ( pcSPS->getMinTrDepth()   );
  xWriteUvlc  ( pcSPS->getMaxTrDepth()   );

#if HHI_RQT
  xWriteFlag( pcSPS->getQuadtreeTUFlag() );
  if( pcSPS->getQuadtreeTUFlag() )
  {
    xWriteUvlc( pcSPS->getQuadtreeTULog2MinSize() - 2 );
    if( pcSPS->getQuadtreeTULog2MinSize() < 6 )
    {
      xWriteUvlc( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize() );
    }
#if HHI_RQT_DEPTH
    xWriteUvlc  ( pcSPS->getQuadtreeTUMaxDepth () - 1 );
#endif	
  }
#endif

  // Max transform size
  xWriteUvlc  ( pcSPS->getMaxTrSize() == 2 ? 0 : g_aucConvertToBit[pcSPS->getMaxTrSize()]+1 );

  // Tools
  xWriteFlag  ( (pcSPS->getUseALF ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseDQP ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseWPG () || pcSPS->getUseWPO ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseLDC ()) ? 1 : 0 );
  xWriteFlag  ( (pcSPS->getUseQBO ()) ? 1 : 0 );
#ifdef QC_AMVRES
  xWriteFlag  ( (pcSPS->getUseAMVRes ()) ? 1 : 0 );
#endif
#if HHI_ALLOW_CIP_SWITCH
  xWriteFlag  ( (pcSPS->getUseCIP ()) ? 1 : 0 ); // BB:
#endif
	xWriteFlag	( (pcSPS->getUseROT ()) ? 1 : 0 ); // BB:
#if HHI_AIS
  xWriteFlag  ( (pcSPS->getUseAIS ()) ? 1 : 0 ); // BB:
#endif
#if HHI_MRG
  xWriteFlag  ( (pcSPS->getUseMRG ()) ? 1 : 0 ); // SOPH:
#endif
#if HHI_IMVP
   xWriteFlag ( (pcSPS->getUseIMP ()) ? 1 : 0 ); // SOPH:
#endif
#ifdef DCM_PBIC
   xWriteFlag ( (pcSPS->getUseIC  ()) ? 1 : 0 );
#endif

  xWriteFlag  ( (pcSPS->getUseAMP ()) ? 1 : 0 );
#if HHI_RMP_SWITCH
  xWriteFlag  ( (pcSPS->getUseRMP()) ? 1 : 0 );
#endif

  // write number of taps for DIF
  xWriteUvlc  ( (pcSPS->getDIFTap ()>>1)-2 ); // 4, 6, 8, 10, 12
#if SAMSUNG_CHROMA_IF_EXT
  xWriteUvlc  ( (pcSPS->getDIFTapC()>>1)-1 ); // 2, 4, 6, 8, 10, 12
#endif

  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }

  // Bit-depth information
  xWriteUvlc( pcSPS->getBitDepth() - 8 );
  xWriteUvlc( pcSPS->getBitIncrement() );

  xWriteCode( pcSPS->getBalancedCPUs(), 8);

}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
#if HHI_NAL_UNIT_SYNTAX
  // here someone can add an appropriated NalRefIdc type 
  xWriteCode( NAL_REF_IDC_PRIORITY_HIGHEST, 2);
  xWriteCode( 0, 1);
  xWriteCode( NAL_UNIT_CODED_SLICE, 5);
#endif
  xWriteCode  (pcSlice->getPOC(), 10 );   //  9 == SPS->Log2MaxFrameNum
  xWriteUvlc  (pcSlice->getSliceType() );
  xWriteSvlc  (pcSlice->getSliceQp() );
  
  xWriteFlag  (pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 ? 1 : 0);
  if( pcSlice->getSymbolMode() > 0 && pcSlice->getSymbolMode() < 3 )
  {
    xWriteFlag( pcSlice->getSymbolMode() > 1 ? 1 : 0 );
    if( pcSlice->getSymbolMode() )
    {
      xWriteFlag( pcSlice->getMultiCodeword() ? 1 : 0 );
    }
    if( pcSlice->getSymbolMode() == 2 && ! pcSlice->getMultiCodeword() )
    {
      xWriteUvlc( pcSlice->getMaxPIPEDelay() >> 6 );
    }
  }
  else
  {
    xWriteFlag( pcSlice->getSymbolMode() > 0 ? 1 : 0 );
  }

  if (!pcSlice->isIntra())
  {
    xWriteFlag  (pcSlice->isReferenced() ? 1 : 0);
#ifdef ROUNDING_CONTROL_BIPRED
	xWriteFlag  (pcSlice->isRounding() ? 1 : 0);
#endif
  }

  xWriteFlag  (pcSlice->getLoopFilterDisable());

  if (!pcSlice->isIntra())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_0 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_0), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
  }
  if (pcSlice->isInterB())
  {
    xWriteCode  ((pcSlice->getNumRefIdx( REF_PIC_LIST_1 )) -pcSlice->getAddRefCnt(REF_PIC_LIST_1), 3 );
  }
  else
  {
    pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
  }

  xWriteFlag  (pcSlice->getDRBFlag() ? 1 : 0 );
  if ( !pcSlice->getDRBFlag() )
  {
    xWriteCode  (pcSlice->getERBIndex(), 2);
  }

  if (!pcSlice->isIntra())   // weighted prediction information
  {
    Int  iNumPredDir = pcSlice->isInterP() ? 1 : 2;

    if (pcSlice->getSPS()->getUseWPG() || pcSlice->getSPS()->getUseWPO())
    {
      for (Int n=0; n<iNumPredDir; n++)
      {
        RefPicList eRefPicList = (RefPicList)n;

        UInt uiWpMode =  pcSlice->getWPmode(eRefPicList);
        xWriteCode  (uiWpMode, 1 );

        if (uiWpMode)
        {
          EFF_MODE eEffMode = (pcSlice->getSPS()->getUseWPG()? EFF_WP_SO : EFF_WP_O);
          UInt uiWeight,uiOffset;

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 0)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 0));
          xWriteUvlc( uiOffset );

#if GRF_WP_CHROMA
          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 1)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 1));
          xWriteUvlc( uiOffset );

          uiWeight = xConvertToUInt( pcSlice->getWPWeight(eRefPicList, eEffMode, 2)-32);
          xWriteUvlc( uiWeight );
          uiOffset = xConvertToUInt( pcSlice->getWPOffset(eRefPicList, eEffMode, 2));
          xWriteUvlc( uiOffset );
#endif
        }
      }
    }
  }

#if HHI_INTERP_FILTER
  xWriteUvlc  ( pcSlice->getInterpFilterType() );
#endif

#if AMVP_NEIGH_COL
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    xWriteFlag( pcSlice->getColDir() );
  }
#endif
#ifdef EDGE_BASED_PREDICTION
  xWriteFlag(pcSlice->getEdgePredictionEnable());
  if( pcSlice->getEdgePredictionEnable() )
    xWriteCode((pcSlice->getEdgeDetectionThreshold()>>8), 8);
#endif //EDGE_BASED_PREDICTION
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
#if !BUGFIX102
  xWriteFlag( uilsLast );
#endif
}

Void TEncCavlc::codeSliceFinish ()
{
  if ( m_bRunLengthCoding && m_uiRun)
  {
    xWriteUvlc(m_uiRun);
  }
}
#endif //LCEC_STAT
// CIP
Void TEncCavlc::codeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int CIPflag = pcCU->getCIPflag   ( uiAbsPartIdx );

  xWriteFlag( CIPflag );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitCIP += 1;
#endif
}

Void TEncCavlc::codeROTindex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  Int indexROT = pcCU->getROTindex( uiAbsPartIdx );
  Int dictSize = ROT_DICT;

  switch (dictSize)
  {
   case 9:
    {
      xWriteFlag( indexROT> 0 ? 0 : 1 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitindexROT += 1;
#endif
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteFlag( ( indexROT & 0x01 )        );
        xWriteFlag( ( indexROT & 0x02 )        );
        xWriteFlag( ( indexROT & 0x04 ) >> 2  );
#if LCEC_STAT
        if (m_bAdaptFlag)
          m_uiBitindexROT += 3;
#endif
      }
    }
    break;
  case 4:
    {
      xWriteFlag( ( indexROT & 0x01 )        );
      xWriteFlag( ( indexROT & 0x02 ) >> 1  );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitindexROT += 2;
#endif
    }
    break;
  case 2:
    {
      xWriteFlag( ( indexROT> 0 ? 0 : 1 ) );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitindexROT += 1;
#endif
    }
    break;
  case 5:
    {
      xWriteFlag( ( indexROT> 0 ? 0 : 1 ) );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitindexROT += 1;
#endif
      if ( indexROT > 0 )
      {
        indexROT = indexROT-1;
        xWriteFlag( ( indexROT & 0x01 )        );
        xWriteFlag( ( indexROT & 0x02 ) >> 1  );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitindexROT += 2;
#endif
      }
    }
    break;
  case 1:
    {
    }
    break;
  }
  return;
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum    = pcCU->getMVPNum(eRefList, uiAbsPartIdx);

#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitMVPIdx += xWriteUnaryMaxSymbol(iSymbol, iNum-1);
  else
#endif
  xWriteUnaryMaxSymbol(iSymbol, iNum-1);
}

#ifdef DCM_PBIC
Void TEncCavlc::codeICPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iSymbol = pcCU->getICPIdx(uiAbsPartIdx);
  Int iNum    = pcCU->getICPNum(uiAbsPartIdx);

#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitICPIdx += xWriteUnaryMaxSymbol(iSymbol, iNum-1);
  else
    xWriteUnaryMaxSymbol(iSymbol, iNum-1);
#else
  xWriteUnaryMaxSymbol(iSymbol, iNum-1);
#endif
}
#endif

Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );

  if ( pcCU->getSlice()->isInterB() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteFlag( 0 );
#if HHI_RMP_SWITCH
    if( pcCU->getSlice()->getSPS()->getUseRMP() ||  pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
#endif
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
    }
#if HHI_DISABLE_INTER_NxN_SPLIT
    if( pcCU->getWidth( uiAbsPartIdx ) == 8 )
    {
      xWriteFlag( 0 );
    }
#else
    xWriteFlag( 0 );
#endif
    xWriteFlag( (eSize == SIZE_2Nx2N? 0 : 1) );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitPartSize += 5;
#endif
    return;
  }

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    xWriteFlag( eSize == SIZE_2Nx2N? 1 : 0 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitPartSize += 1;
#endif
    return;
  }

  switch(eSize)
  {
  case SIZE_2Nx2N:
    {
      xWriteFlag( 1 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitPartSize += 1;
#endif
      break;
    }
  case SIZE_2NxN:
  case SIZE_2NxnU:
  case SIZE_2NxnD:
    {
      xWriteFlag( 0 );
      xWriteFlag( 1 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitPartSize += 2;
#endif
#if HHI_RMP_SWITCH
      if (pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) && pcCU->getSlice()->getSPS()->getUseRMP() )
#else
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
#endif
      {
        if (eSize == SIZE_2NxN)
        {
          xWriteFlag( 1 );
#if LCEC_STAT
          if (m_bAdaptFlag)
          m_uiBitPartSize += 1;
#endif
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( (eSize == SIZE_2NxnU? 0: 1) );
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitPartSize += 2;
#endif
        }
      }
#if HHI_RMP_SWITCH
      else if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) && !pcCU->getSlice()->getSPS()->getUseRMP() )
      {
        xWriteFlag( (eSize == SIZE_2NxnU? 0: 1) );
      }
#endif
      break;
    }
  case SIZE_Nx2N:
  case SIZE_nLx2N:
  case SIZE_nRx2N:
    {
      xWriteFlag( 0 );
      xWriteFlag( 0 );
      xWriteFlag( 1 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitPartSize += 3;
#endif
#if HHI_RMP_SWITCH
      if (pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) && pcCU->getSlice()->getSPS()->getUseRMP() )
#else
      if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
#endif
      {
        if (eSize == SIZE_Nx2N)
        {
          xWriteFlag( 1 );
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitPartSize += 1;
#endif
        }
        else
        {
          xWriteFlag( 0 );
          xWriteFlag( (eSize == SIZE_nLx2N? 0: 1) );
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitPartSize += 2;
#endif
        }
      }
#if HHI_RMP_SWITCH
      else if ( pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) && !pcCU->getSlice()->getSPS()->getUseRMP() )
      {
        xWriteFlag( (eSize == SIZE_nLx2N? 0: 1) );
      }
#endif
      break;
    }
  case SIZE_NxN:
    {
#if HHI_DISABLE_INTER_NxN_SPLIT
      if( pcCU->getWidth( uiAbsPartIdx ) == 8 )
#endif
      {
        xWriteFlag( 0 );
#if HHI_RMP_SWITCH
        if( pcCU->getSlice()->getSPS()->getUseRMP() ||  pcCU->getSlice()->getSPS()->getAMPAcc( uiDepth ) )
#endif
        {
          xWriteFlag( 0 );
          xWriteFlag( 0 );
        }
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitPartSize += 3;
#endif
        if (pcCU->getSlice()->isInterB())
        {
          xWriteFlag( 1 );
        }
#if LCEC_STAT
        if (m_bAdaptFlag)
          m_uiBitPartSize += 1;
#endif
      }
      break;
    }
  default:
    {
      assert(0);
    }
  }
}

Void TEncCavlc::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  Int iPredMode = pcCU->getPredictionMode( uiAbsPartIdx );

#if HHI_MRG && !SAMSUNG_MRG_SKIP_DIRECT
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    xWriteFlag( iPredMode == MODE_SKIP ? 0 : 1 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitPredMode += 1;
#endif
  }
#else
  xWriteFlag( iPredMode == MODE_SKIP ? 0 : 1 );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitPredMode += 1;
#endif
#endif

  if ( pcCU->getSlice()->isInterB() )
  {
    return;
  }

  if ( iPredMode != MODE_SKIP )
  {
    xWriteFlag( iPredMode == MODE_INTER ? 0 : 1 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitPredMode += 1;
#endif
  }
}

#if HHI_MRG
Void TEncCavlc::codeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitMergeFlag += 1;
#endif
}

Void TEncCavlc::codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getMergeIndex( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitMergeIndex += 1;
#endif
}
#endif

#if HHI_ALF
Void TEncCavlc::codeAlfQTCtrlFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
      return;

    if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
    {
      return;
    }
    UInt uiSymbol = pcCU->getAlfCtrlFlag(uiAbsPartIdx);
    xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfQTSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiMaxDepth )
{
  if( uiDepth >= g_uiMaxCUDepth - g_uiAddCUDepth )
     return;

   UInt uiFlag = ( pcCU->getDepth( uiAbsPartIdx ) == uiDepth ) ? 0 : 1;
   xWriteFlag( uiFlag );
}



Void TEncCavlc::codeAlfCoeff( Int iCoeff, Int iLength, Int iPos)
{
  if ( iCoeff == 0)
    xWriteFlag( 0 );
  else
  {
      xWriteFlag( 1 );

    if ( iCoeff > 0 )
      xWriteFlag( 0 );
    else
    {
      xWriteFlag( 1 );
      iCoeff = -iCoeff ;
    }
    Int iM =4;
    if(iLength==3)
    {
      if(iPos == 0)
        iM = 4 ;
      else if(iPos == 1)
        iM = 1 ;
      else if(iPos == 2)
        iM = 2 ;
    }
    else if(iLength==5)
    {
      if(iPos == 0)
        iM = 3 ;
      else if(iPos == 1)
        iM = 5 ;
      else if(iPos == 2)
        iM = 1 ;
      else if(iPos == 3)
        iM = 3 ;
      else if(iPos == 4)
        iM = 2 ;
    }
    else if(iLength==7)
    {
      if(iPos == 0)
        iM = 3 ;
      else if(iPos == 1)
        iM = 4 ;
      else if(iPos == 2)
        iM = 5;
      else if(iPos == 3)
        iM = 1 ;
      else if(iPos == 4)
        iM = 3 ;
      else if(iPos == 5)
       iM = 3 ;
      else if(iPos == 6)
       iM = 2 ;
    }
    else if(iLength==9)
    {
      if(iPos == 0)
        iM = 2 ;
      else if(iPos == 1)
        iM = 4 ;
      else if(iPos == 2)
        iM = 4 ;
      else if(iPos == 3)
        iM = 5 ;
      else if(iPos == 4)
        iM = 1 ;
      else if(iPos == 5)
       iM = 3 ;
      else if(iPos == 6)
       iM = 3 ;
      else if(iPos == 7)
      iM = 3 ;
     else if(iPos == 8)
      iM = 2 ;
    }

    xWriteEpExGolomb( iCoeff , iM );
  }

}




Void TEncCavlc::codeAlfDc( Int iDc    )
{
  if ( iDc == 0 )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
      // write sign
    if ( iDc > 0 )
    {
      xWriteFlag( 0 ) ;
    }
    else
    {
      xWriteFlag( 1 ) ;
      iDc = -iDc;
    }
    xWriteEpExGolomb( iDc , 9 );
  }
}
#endif

#if HHI_AIS
Void TEncCavlc::codeIntraFiltFlagLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Bool bFlag = pcCU->getLumaIntraFiltFlag( uiAbsPartIdx );
  xWriteFlag( (UInt)bFlag );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitIntraFiltFlag += 1;
#endif
}
#endif

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
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfCtrlFlag += 1;
#endif
}

Void TEncCavlc::codeAlfCtrlDepth()
{  
  if (!m_bAlfCtrl)
    return;

  UInt uiDepth = m_uiMaxAlfCtrlDepth;

#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfCtrlDepth += xWriteUnaryMaxSymbol(uiDepth, g_uiMaxCUDepth-1);
  else
#endif
  xWriteUnaryMaxSymbol(uiDepth, g_uiMaxCUDepth-1);
}

Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  xWriteFlag( uiSymbol );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitSkipFlag += 1;
#endif
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    return;

  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;

  xWriteFlag( uiCurrSplitFlag );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitCurrSplitFlag += 1;
#endif
  return;
}

#if HHI_RQT
Void TEncCavlc::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  xWriteFlag( uiSymbol );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitTransformSubdivFlag += 1;
#endif
}

Void TEncCavlc::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  UInt uiCbf = pcCU->getCbf( uiAbsPartIdx, eType, uiTrDepth );
  xWriteFlag( uiCbf );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitQtCbf += 1;
#endif
}

#if HHI_RQT_ROOT
Void TEncCavlc::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  xWriteFlag( uiCbf ? 1 : 0 );
}
#endif
#endif

Void TEncCavlc::codeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiTrLevel = 0;

  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

  UInt uiMinTrDepth = pcCU->getSlice()->getSPS()->getMinTrDepth() + uiTrLevel;
  UInt uiMaxTrDepth = pcCU->getSlice()->getSPS()->getMaxTrDepth() + uiTrLevel;

  if ( uiMinTrDepth == uiMaxTrDepth )
  {
    return;
  }

  UInt uiSymbol = pcCU->getTransformIdx(uiAbsPartIdx) - uiMinTrDepth;

  xWriteFlag( uiSymbol ? 1 : 0 );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitTransformIdx += 1;
#endif

  if ( !uiSymbol )
  {
    return;
  }

  if (pcCU->getPartitionSize(uiAbsPartIdx) >= SIZE_2NxnU && pcCU->getPartitionSize(uiAbsPartIdx) <= SIZE_nRx2N && uiMinTrDepth == 0 && uiMaxTrDepth == 1 && uiSymbol)
  {
    return;
  }

  Int  iCount = 1;
  uiSymbol--;
  while( ++iCount <= (Int)( uiMaxTrDepth - uiMinTrDepth ) )
  {
    xWriteFlag( uiSymbol ? 1 : 0 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitTransformIdx += 1;
#endif
    if ( uiSymbol == 0 )
    {
      return;
    }
    uiSymbol--;
  }

  return;
}

#if PLANAR_INTRA
// Temporary VLC function
Void TEncCavlc::xPutPlanarVlc( Int n, Int cn )
{
  UInt tmp  = 1<<(n-4);
  UInt code = tmp+cn%tmp;
  UInt len  = 1+(n-4)+(cn>>(n-4));

  xWriteCode( code, len );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitPlanarVlc += len;
#endif
}

Void TEncCavlc::xCodePlanarDelta( TComDataCU* pcCU, UInt uiAbsPartIdx , Int iDelta )
{
  /* Planar quantization
  Y        qY              cW
  0-3   :  0,1,2,3         0-3
  4-15  :  4,6,8..14       4-9
  16-63 : 18,22,26..62    10-21
  64-.. : 68,76...        22-
  */
  Bool bDeltaNegative = iDelta < 0 ? true : false;
  UInt uiDeltaAbs     = abs(iDelta);

  if( uiDeltaAbs < 4 )
    xPutPlanarVlc( 5, uiDeltaAbs );
  else if( uiDeltaAbs < 16 )
    xPutPlanarVlc( 5, (uiDeltaAbs>>1)+2 );
  else if( uiDeltaAbs < 64)
    xPutPlanarVlc( 5, (uiDeltaAbs>>2)+6 );
  else
    xPutPlanarVlc( 5, (uiDeltaAbs>>3)+14 );

  if(uiDeltaAbs > 0)
    xWriteFlag( bDeltaNegative ? 1 : 0 );

#if LCEC_STAT
  if (m_bAdaptFlag)
  {
    if(uiDeltaAbs > 0)
      m_uiBitPlanarVlc += 1;
  }
#endif

}

Void TEncCavlc::codePlanarInfo( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (pcCU->isIntra( uiAbsPartIdx ))
  {
    UInt uiPlanar = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_FLAG);

    xWriteFlag( uiPlanar );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitPlanarInfo += 1;
#endif
    if ( uiPlanar )
    {
      // Planar delta for Y
      xCodePlanarDelta( pcCU, uiAbsPartIdx, pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAY) );

      // Planar delta for U and V
      Int  iPlanarDeltaU = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAU);
      Int  iPlanarDeltaV = pcCU->getPlanarInfo(uiAbsPartIdx, PLANAR_DELTAV);

      xWriteFlag( ( iPlanarDeltaU == 0 && iPlanarDeltaV == 0 ) ? 1 : 0 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitPlanarInfo += 1;
#endif
      if ( iPlanarDeltaU != 0 || iPlanarDeltaV != 0 )
      {
        xCodePlanarDelta( pcCU, uiAbsPartIdx, iPlanarDeltaU );
        xCodePlanarDelta( pcCU, uiAbsPartIdx, iPlanarDeltaV );
      }
    }
  }
}
#endif

#if ANG_INTRA
Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiDir         = pcCU->getLumaIntraDir( uiAbsPartIdx );
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );



  if (uiDir == iMostProbable)
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIntraDir += 1;
#endif
  }
  else{
    xWriteFlag( 0 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIntraDir += 1;
#endif
    uiDir = uiDir > iMostProbable ? uiDir - 1 : uiDir;
#if UNIFIED_DIRECTIONAL_INTRA
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      xWriteFlag( uiDir & 0x01 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) xWriteFlag( uiDir & 0x02 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) xWriteFlag( uiDir & 0x04 ? 1 : 0 );
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) xWriteFlag( uiDir & 0x08 ? 1 : 0 );

#if LCEC_STAT
      if (m_bAdaptFlag)
      {
        m_uiBitIntraDir += 1;
        if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) m_uiBitIntraDir += 1;
        if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) m_uiBitIntraDir += 1;
        if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) m_uiBitIntraDir += 1;
      }
#endif

    }
    else
#endif
    if (uiDir < 31){ // uiDir is here 0...32, 5 bits for uiDir 0...30, 31 is an escape code for coding one more bit for 31 and 32
      xWriteFlag( uiDir & 0x01 ? 1 : 0 );
      xWriteFlag( uiDir & 0x02 ? 1 : 0 );
      xWriteFlag( uiDir & 0x04 ? 1 : 0 );
      xWriteFlag( uiDir & 0x08 ? 1 : 0 );
      xWriteFlag( uiDir & 0x10 ? 1 : 0 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIntraDir += 5;
#endif
    }
    else{
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( 1 );
      xWriteFlag( uiDir == 32 ? 1 : 0 );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIntraDir += 6;
#endif
    }
  }
}
#endif

Void TEncCavlc::codeIntraDirLumaAdi( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iIntraDirLuma = pcCU->convertIntraDirLumaAdi( pcCU, uiAbsPartIdx );
  Int iIntraIdx= pcCU->getIntraSizeIdx(uiAbsPartIdx);

  xWriteFlag( iIntraDirLuma >= 0 ? 0 : 1 );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitIntraDir += 1;
#endif
  if (iIntraDirLuma >= 0)
  {
    xWriteFlag((iIntraDirLuma & 0x01));

    xWriteFlag((iIntraDirLuma & 0x02) >> 1);
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIntraDir += 2;
#endif
    if (g_aucIntraModeBits[iIntraIdx] >= 4)
    {
      xWriteFlag((iIntraDirLuma & 0x04) >> 2);
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIntraDir += 1;
#endif
      if (g_aucIntraModeBits[iIntraIdx] >= 5)
      {
        xWriteFlag((iIntraDirLuma & 0x08) >> 3);
#if LCEC_STAT
        if (m_bAdaptFlag)
          m_uiBitIntraDir += 1;
#endif
        if (g_aucIntraModeBits[iIntraIdx] >= 6)
        {
          xWriteFlag((iIntraDirLuma & 0x10) >> 4);
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitIntraDir += 1;
#endif
        }
      }
    }
  }
  return;
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiIntraDirChroma = pcCU->getChromaIntraDir   ( uiAbsPartIdx );

  if ( 0 == uiIntraDirChroma )
  {
    xWriteFlag( 0 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIntraDir += 1;
#endif
  }
  else
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    if (m_bAdaptFlag){
      m_uiBitIntraDir += 1;
      m_uiBitIntraDir += xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, 3 );
    }
    else
#endif
    xWriteUnaryMaxSymbol( uiIntraDirChroma - 1, 3 );
  }

  return;
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiInterDir = pcCU->getInterDir   ( uiAbsPartIdx );
  uiInterDir--;

#if LCEC_PHASE2
  if(pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2)
  {
    Int x,cx,y,cy;
    Int iRefFrame0,iRefFrame1;
    UInt uiIndex;

    UInt *m_uiMITableE;
    UInt *m_uiMITableD;
#ifdef QC_AMVRES
    if(pcCU->getSlice()->getSPS()->getUseAMVRes())    
    {
      Bool Mvres0,Mvres1;
      m_uiMITableE = m_uiMI2TableE;
      m_uiMITableD = m_uiMI2TableD;
      if (uiInterDir==0)
      {      
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        Mvres0 = (iRefFrame0==0) ? pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv( uiAbsPartIdx ).isHAM() : 0;
        uiIndex = (Mvres0 ? 2 : iRefFrame0);
      }
      else if (uiInterDir==1)
      {
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        Mvres1 = (iRefFrame1==0) ? pcCU->getCUMvField( REF_PIC_LIST_1 )->getMv( uiAbsPartIdx ).isHAM() : 0;
        uiIndex = 3 + (Mvres1 ? 2 : iRefFrame1);
      }
      else
      {
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        Mvres0 = (iRefFrame0==0) ? pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv( uiAbsPartIdx ).isHAM() : 0;        
        Mvres1 = (iRefFrame1==0) ? pcCU->getCUMvField( REF_PIC_LIST_1 )->getMv( uiAbsPartIdx ).isHAM() : 0;
        uiIndex = 6 + 3*(Mvres0 ? 2 : iRefFrame0) + (Mvres1 ? 2 : iRefFrame1);
      }
    }
    else
#endif
    {      
      m_uiMITableE = m_uiMI1TableE;
      m_uiMITableD = m_uiMI1TableD;
      if (uiInterDir==0)
      { 
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        uiIndex = iRefFrame0;
      }
      else if (uiInterDir==1)
      {
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        uiIndex = 2 + iRefFrame1;
      }
      else
      {
        iRefFrame0 = pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx );
        iRefFrame1 = pcCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiAbsPartIdx );
        uiIndex = 4 + 2*iRefFrame0 + iRefFrame1;
      }
    }
    
    x = uiIndex;
    
    cx = m_uiMITableE[x];
   
    /* Adapt table */
    UInt vlcn = g_auiMITableVlcNum[m_uiMITableVlcIdx];    
    if ( m_bAdaptFlag )
    {        
      cy = Max(0,cx-1);
      y = m_uiMITableD[cy];
      m_uiMITableD[cy] = x;
      m_uiMITableD[cx] = y;
      m_uiMITableE[x] = cy;
      m_uiMITableE[y] = cx;   
	    m_uiMITableVlcIdx += cx == m_uiMITableVlcIdx ? 0 : (cx < m_uiMITableVlcIdx ? -1 : 1);
    }

#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitMI += xWriteVlc( vlcn, cx );
    else
#endif
	  xWriteVlc( vlcn, cx );

    return;
  }
#endif

  xWriteFlag( ( uiInterDir == 2 ? 1 : 0 ));
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitInterDir += 1;
#endif
#if MS_NO_BACK_PRED_IN_B0
  if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    assert( uiInterDir != 1 );
    return;
  }
#endif
  if ( uiInterDir < 2 )
  {
    xWriteFlag( uiInterDir );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitInterDir += 1;
#endif
  }

  return;
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );

#if LCEC_PHASE2
    if (pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2)
    {
      return;
    }
#endif

#ifdef QC_AMVRES
  if(pcCU->getSlice()->getSPS()->getUseAMVRes())
  {
	  Bool Mvres = !pcCU->getCUMvField( eRefList )->getMv( uiAbsPartIdx ).isHAM();
	  if (iRefFrame == 0 )
	  {
		  if (Mvres)
      {
			 xWriteFlag(0);
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIRefFrmIdx += 1;
#endif
      }
		  else
		  {
			 xWriteFlag(1);
			 xWriteFlag(0);
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIRefFrmIdx += 2;
#endif
		  }
	  }
	  else
	  {
		  xWriteFlag(1);
		  xWriteFlag(1);
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIRefFrmIdx += 2;
#endif
		  if (pcCU->getSlice()->getNumRefIdx( eRefList )>2)
      {
#if LCEC_STAT
        if (m_bAdaptFlag)
          m_uiBitIRefFrmIdx += xWriteUnaryMaxSymbol( iRefFrame-1, pcCU->getSlice()->getNumRefIdx( eRefList )-2);
        else
#endif
	 		  xWriteUnaryMaxSymbol( iRefFrame-1, pcCU->getSlice()->getNumRefIdx( eRefList )-2);
      }
	  }
  }
  else
  {
	  xWriteFlag( ( iRefFrame == 0 ? 0 : 1 ) );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIRefFrmIdx += 1;
#endif
	  if ( iRefFrame > 0 )
	  {
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitIRefFrmIdx += xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
      else
#endif
		  xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
	  }
  }
#else
  xWriteFlag( ( iRefFrame == 0 ? 0 : 1 ) );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitIRefFrmIdx += 1;
#endif
  if ( iRefFrame > 0 )
  {
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitIRefFrmIdx += xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
    else
#endif
    xWriteUnaryMaxSymbol( iRefFrame - 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
  }
#endif
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
#ifdef QC_AMVRES
  if(pcCU->getSlice()->getSPS()->getUseAMVRes()&& (!pcCUMvField->getMv ( uiAbsPartIdx ).isHAM()) )
  {
	  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor()>>1) ) +
				 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor()>>1) );
	  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : (pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer()>>1) ) +
				 ( (pcCUMvFieldA == NULL) ? 0 : (pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer()>>1) );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitMVD += xWriteSvlc( iHor/2);
      else
        xWriteSvlc( iHor/2);
      if (m_bAdaptFlag)      
        m_uiBitMVD += xWriteSvlc( iVer/2);
      else
        xWriteSvlc( iVer/2);
#else
      xWriteSvlc( iHor/2);
      xWriteSvlc( iVer/2);
#endif
	  return;
  }
#endif
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
       ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
       ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );

#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitMVD += xWriteSvlc( iHor);
  else
    xWriteSvlc( iHor);
  if (m_bAdaptFlag)      
    m_uiBitMVD += xWriteSvlc( iVer);
  else
    xWriteSvlc( iVer);
#else
  xWriteSvlc( iHor );
  xWriteSvlc( iVer );
#endif

  return;
}

#ifdef DCM_PBIC
Void TEncCavlc::codeMvdIcd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iZeroPatt = 0;
  TComMv acMvd[2];
  Int iIcParam[3];
  TComZeroTree* pcZTree;

#ifdef QC_AMVRES
  // Determine and Code MV resolution flag (if necessary)
  Bool bMvResFlag[2] = {false, false};
  if ( pcCU->getSlice()->getSPS()->getUseAMVRes() )
  {
    if ( (eRefList == REF_PIC_LIST_0) || (eRefList == REF_PIC_LIST_X) )
      bMvResFlag[REF_PIC_LIST_0] = !(pcCU->getCUMvField( REF_PIC_LIST_0 )->getMv ( uiAbsPartIdx ).isHAM());
    if ( (eRefList == REF_PIC_LIST_1) || (eRefList == REF_PIC_LIST_X) )
      bMvResFlag[REF_PIC_LIST_1] = !(pcCU->getCUMvField( REF_PIC_LIST_1 )->getMv ( uiAbsPartIdx ).isHAM());
  }
#endif

  // Identify the non-zero components
  if (eRefList == REF_PIC_LIST_X)
  {
    acMvd[ REF_PIC_LIST_0 ] = pcCU->getCUMvField( REF_PIC_LIST_0 )->getMvd( uiAbsPartIdx );
#ifdef QC_AMVRES
    if (bMvResFlag[REF_PIC_LIST_0] == true)
      acMvd[REF_PIC_LIST_0].scale_down();
#endif
    acMvd[ REF_PIC_LIST_1 ] = pcCU->getCUMvField( REF_PIC_LIST_1 )->getMvd( uiAbsPartIdx );
#ifdef QC_AMVRES
    if (bMvResFlag[REF_PIC_LIST_1] == true)
      acMvd[REF_PIC_LIST_1].scale_down();
#endif
    iZeroPatt |= ( acMvd[REF_PIC_LIST_0].getHor() == 0 ) ? 0 : 1;
    iZeroPatt |= ( acMvd[REF_PIC_LIST_0].getVer() == 0 ) ? 0 : 2;
    iZeroPatt |= ( acMvd[REF_PIC_LIST_1].getHor() == 0 ) ? 0 : 4;
    iZeroPatt |= ( acMvd[REF_PIC_LIST_1].getVer() == 0 ) ? 0 : 8;

    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      pcCU->getCUIcField()->getIcd( uiAbsPartIdx ).getIcParam( iIcParam[0], iIcParam[1], iIcParam[2] );
      iZeroPatt |= ( iIcParam[0] == 0 ) ? 0 : 16;
      iZeroPatt |= ( iIcParam[1] == 0 ) ? 0 : 32;
      iZeroPatt |= ( iIcParam[2] == 0 ) ? 0 : 64;

      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDICDBI);
    }
    else
    {
      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDBI);
    }
  }
  else
  {
    acMvd[ eRefList ] = pcCU->getCUMvField( eRefList )->getMvd( uiAbsPartIdx );
#ifdef QC_AMVRES
    if (bMvResFlag[eRefList] == true)
      acMvd[eRefList].scale_down();
#endif
    iZeroPatt |= ( acMvd[eRefList].getHor() == 0 ) ? 0 : 1;
    iZeroPatt |= ( acMvd[eRefList].getVer() == 0 ) ? 0 : 2;

    if (pcCU->getSlice()->getSPS()->getUseIC())
    {
      pcCU->getCUIcField()->getIcd( uiAbsPartIdx ).getIcParam( iIcParam[0], iIcParam[1], iIcParam[2] );
      iZeroPatt |= ( iIcParam[0] == 0 ) ? 0 : 4;
      assert ( iIcParam[1] == 0 );
      iZeroPatt |= ( iIcParam[2] == 0 ) ? 0 : 8;

      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDICDUNI);
    }
    else
    {
      pcZTree    = pcCU->getSlice()->getZTree(IDX_ZTREE_MVDUNI);
    }
  }

  // Encode zeroflag and zerotree (if necessary)
  if (iZeroPatt == 0)
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitMVDICD += 1;
#endif
  }
  else
  {
    xWriteFlag( 0 );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitMVDICD += 1;
#endif
    pcZTree->updateVal(iZeroPatt);
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitMVDICD += codeZTree( pcZTree, pcZTree->m_pcRoot );
    else
      codeZTree( pcZTree, pcZTree->m_pcRoot );
#else
    codeZTree( pcZTree, pcZTree->m_pcRoot );
#endif
  }

  //Encode the non-zero components
  if ( (eRefList == REF_PIC_LIST_X) || (eRefList == REF_PIC_LIST_0) )
  {
#if LCEC_STAT
    if (m_bAdaptFlag)
    {
      m_uiBitMVDICD += xWriteSvlcNZ( acMvd[REF_PIC_LIST_0].getHor() );
      m_uiBitMVDICD += xWriteSvlcNZ( acMvd[REF_PIC_LIST_0].getVer() );
    }
    else
#endif
    {
      xWriteSvlcNZ( acMvd[REF_PIC_LIST_0].getHor() );
      xWriteSvlcNZ( acMvd[REF_PIC_LIST_0].getVer() );
    }
  }
  if ( (eRefList == REF_PIC_LIST_X) || (eRefList == REF_PIC_LIST_1) )
  {
#if LCEC_STAT
    if (m_bAdaptFlag)
    {
      m_uiBitMVDICD += xWriteSvlcNZ( acMvd[REF_PIC_LIST_1].getHor() );
      m_uiBitMVDICD += xWriteSvlcNZ( acMvd[REF_PIC_LIST_1].getVer() );
    }
    else
#endif
    {
      xWriteSvlcNZ( acMvd[REF_PIC_LIST_1].getHor() );
      xWriteSvlcNZ( acMvd[REF_PIC_LIST_1].getVer() );
    }
  }
  if (pcCU->getSlice()->getSPS()->getUseIC())
  {
#if LCEC_STAT
    if (m_bAdaptFlag)
    {
      m_uiBitMVDICD += xWriteSvlcNZ( iIcParam[0] );
      m_uiBitMVDICD += xWriteSvlcNZ( iIcParam[1] );
      m_uiBitMVDICD += xWriteSvlcNZ( iIcParam[2] );
    }
    else
#endif
    {
      xWriteSvlcNZ( iIcParam[0] );
      xWriteSvlcNZ( iIcParam[1] );
      xWriteSvlcNZ( iIcParam[2] );
    }
  }
}

#if LCEC_STAT
UInt TEncCavlc::codeZTree( TComZeroTree* pcZTree, TComZTNode* pcZTNode )
{
  Int iVal, iLval, iRval;
  UInt uiNumBits = 0;

  if (pcZTNode->IsLeaf() == false)
  {
    iLval = pcZTNode->m_pcLeft->m_iVal;
    iRval = pcZTNode->m_pcRight->m_iVal;

    iVal = iLval & iRval;
    xWriteFlag( iVal );
    uiNumBits++;

    if (iVal == 0)
    {
      xWriteFlag( iLval );
      uiNumBits++;
    }

    if (iLval != 0)
      uiNumBits += codeZTree( pcZTree,  pcZTNode->m_pcLeft );
    if (iRval != 0)
      uiNumBits += codeZTree( pcZTree, pcZTNode->m_pcRight );
  }
  return uiNumBits;
}
#else
Void TEncCavlc::codeZTree( TComZeroTree* pcZTree, TComZTNode* pcZTNode )
{
  Int iVal, iLval, iRval;

  if (pcZTNode->IsLeaf() == false)
  {
    iLval = pcZTNode->m_pcLeft->m_iVal;
    iRval = pcZTNode->m_pcRight->m_iVal;

    iVal = iLval & iRval;
    xWriteFlag( iVal );

    if (iVal == 0)
      xWriteFlag( iLval );

    if (iLval != 0)
      codeZTree( pcZTree,  pcZTNode->m_pcLeft );
    if (iRval != 0)
      codeZTree( pcZTree, pcZTNode->m_pcRight );
  }
}
#endif

ContextModel* TEncCavlc::getZTreeCtx ( Int iIdx )
{
  return NULL;
}
#endif

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getSlice()->getSliceQp();

#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitDeltaQP += 1;
#endif

  if ( iDQp == 0 )
  {
    xWriteFlag( 0 );
  }
  else
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    if (m_bAdaptFlag)      
      m_uiBitDeltaQP += xWriteSvlc( iDQp );
    else
#endif
    xWriteSvlc( iDQp );
  }

  return;
}

Void TEncCavlc::codeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
#if HHI_RQT
  if( pcCU->getSlice()->getSPS()->getQuadtreeTUFlag() )
  {
#if HHI_RQT_INTRA
    return;
#else
    if( !pcCU->isIntra( uiAbsPartIdx ) )
    {
      return;
    }
#endif
  }
#endif

#if LCEC_PHASE2
  if (eType == TEXT_ALL){
    Int n,x,cx,y,cy;
    UInt uiCBFY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0);
    UInt uiCBFU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0);
    UInt uiCBFV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0);
    UInt uiCBP = (uiCBFV<<2) + (uiCBFU<<1) + (uiCBFY<<0);

    /* Start adaptation */
    n = pcCU->isIntra( uiAbsPartIdx ) ? 0 : 1;
    x = uiCBP;
    cx = m_uiCBPTableE[n][x];
    
    UInt vlcn = g_auiCbpVlcNum[n][m_uiCbpVlcIdx[n]];

    if ( m_bAdaptFlag )
    {                
        
        cy = Max(0,cx-1);
        y = m_uiCBPTableD[n][cy];
        m_uiCBPTableD[n][cy] = x;
        m_uiCBPTableD[n][cx] = y;
        m_uiCBPTableE[n][x] = cy;
        m_uiCBPTableE[n][y] = cx;
        m_uiCbpVlcIdx[n] += cx == m_uiCbpVlcIdx[n] ? 0 : (cx < m_uiCbpVlcIdx[n] ? -1 : 1);
    }
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCbp += xWriteVlc( vlcn, cx );
    else
#endif
    xWriteVlc( vlcn, cx );

    return;
  }
#endif

  UInt uiCbf = pcCU->getCbf   ( uiAbsPartIdx, eType, uiTrDepth );

  xWriteFlag( uiCbf );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitCbf += 1;
#endif

  return;
}

#if QC_MDDT
Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, UInt uiMode, Bool bRD )
#else
Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType, Bool bRD )
#endif
{
  if ( uiWidth > m_pcSlice->getSPS()->getMaxTrSize() ) {
    uiWidth  = m_pcSlice->getSPS()->getMaxTrSize();
    uiHeight = m_pcSlice->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;

  // point to coefficient
  TCoeff* piCoeff = pcCoef;
  UInt uiNumSig = 0;
  UInt uiScanning;
#if !QC_MDDT && !LCEC_PHASE1_ADAPT_ENABLE
  UInt uiInterleaving;
#endif

  // compute number of significant coefficients
  UInt  uiPart = 0;
  xCheckCoeff(piCoeff, uiWidth, 0, uiNumSig, uiPart );

  if ( bRD ) {
    UInt uiTempDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
    pcCU->setCbfSubParts( ( uiNumSig ? 1 : 0 ) << uiTempDepth, eTType, uiAbsPartIdx, uiDepth );
    codeCbf( pcCU, uiAbsPartIdx, eTType, uiTempDepth );
  }

  if ( uiNumSig == 0 ) {
    return;
  }

  // initialize scan
  const UInt*  pucScan;
#if LCEC_PHASE1
  //UInt uiConvBit = g_aucConvertToBit[ Min(8,uiWidth)    ];
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : Min(8,uiWidth)    ];
#else
  UInt uiConvBit = g_aucConvertToBit[ uiWidth    ];
#endif
#if HHI_RQT
  pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];
#else
  pucScan         = g_auiFrameScanXY  [ uiConvBit ];
#endif

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
  UInt *scanStats;
  int indexROT ;
  if(pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA && (uiWidth == 4 || uiWidth == 8 || uiWidth == 16 || uiWidth == 32 || uiWidth == 64))//!bRD &&  eTType == TEXT_LUMA)
  {
	indexROT = pcCU->getROTindex(uiAbsPartIdx);
	int scan_index;
    if(uiWidth == 4)// && uiMode<=8&&indexROT == 0)
    {
      UInt uiPredMode = g_aucIntra9Mode[uiMode];
       pucScan = scanOrder4x4[uiPredMode]; //pucScanX = scanOrder4x4X[ipredmode]; pucScanY = scanOrder4x4Y[ipredmode];

	   if(g_bUpdateStats)
       {
         scanStats = scanStats4x4[uiPredMode]; update4x4Count[uiPredMode]++;
       }
    }
    else if(uiWidth == 8)// && uiMode<=8 && indexROT == 0)
    {
      UInt uiPredMode = ((1 << (pcCU->getIntraSizeIdx( uiAbsPartIdx ) + 1)) != uiWidth) ? g_aucIntra9Mode[uiMode] : g_aucAngIntra9Mode[uiMode];
      pucScan = scanOrder8x8[uiPredMode]; //pucScanX = scanOrder8x8X[ipredmode]; pucScanY = scanOrder8x8Y[ipredmode];

	   if(g_bUpdateStats)
      {
        scanStats = scanStats8x8[uiPredMode]; update8x8Count[uiPredMode]++;
      }
    }
	else if(uiWidth == 16)
	{
		scan_index = LUT16x16[indexROT][uiMode];
		pucScan = scanOrder16x16[scan_index]; //pucScanX = scanOrder16x16X[scan_index]; pucScanY = scanOrder16x16Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats16x16[scan_index];
		}
    }
    else if(uiWidth == 32)
    {
		scan_index = LUT32x32[indexROT][uiMode];
		pucScan = scanOrder32x32[scan_index]; //pucScanX = scanOrder32x32X[scan_index]; pucScanY = scanOrder32x32Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats32x32[scan_index];
		}
    }
    else if(uiWidth == 64)
    {
		scan_index = LUT64x64[indexROT][uiMode];
		pucScan = scanOrder64x64[scan_index]; //pucScanX = scanOrder64x64X[scan_index]; pucScanY = scanOrder64x64Y[scan_index];

	   if(g_bUpdateStats)
		{
			scanStats = scanStats64x64[scan_index];
		}
    }
    else
    {
      //printf("uiWidth = %d is not supported!\n", uiWidth);
      //exit(1);
    }
  }
#endif

  TCoeff scoeff[64];
  Int iBlockType;
#if !QC_MDDT && !LCEC_PHASE1_ADAPT_ENABLE
  UInt uiNumSigInterleaved;
#endif
#if LCEC_PHASE1
  UInt uiCodeDCCoef = 0;
  TCoeff dcCoeff = 0;
  if (pcCU->isIntra(uiAbsPartIdx))
  {
    UInt uiAbsPartIdxL, uiAbsPartIdxA;
    TComDataCU* pcCUL   = pcCU->getPULeft (uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    TComDataCU* pcCUA   = pcCU->getPUAbove(uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    if (pcCUL == NULL && pcCUA == NULL)
    {
#if 1
      uiCodeDCCoef = 1;
      xWriteVlc((eTType == TEXT_LUMA ? 3 : 1) , abs(piCoeff[0]));
      if (piCoeff[0] != 0)
      {
        UInt sign = (piCoeff[0] < 0) ? 1 : 0;
        xWriteFlag(sign);
      }
      dcCoeff = piCoeff[0];
      piCoeff[0] = 1;
#else
        xWriteFlag(1);
#endif
    }
  }
#endif

#if HHI_RQT
  if( uiSize == 2*2 )
  {
    // hack: re-use 4x4 coding
    ::memset( scoeff, 0, 16*sizeof(TCoeff) );
    for (uiScanning=0; uiScanning<4; uiScanning++)
    {
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
    }
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();

    xCodeCoeff4x4( scoeff, iBlockType );
  }
  else if ( uiSize == 4*4 )
#else
  if ( uiSize == 4*4 )
#endif
  {
    for (uiScanning=0; uiScanning<16; uiScanning++)
    {
      scoeff[15-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
      if(scoeff[15-uiScanning])
      {
        if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA)// && (uiWidth == 4 && uiMode<=8&&indexROT == 0))
        {
          scanStats[uiScanning]++;
        }
      }
#endif
    }
    iBlockType = pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType();

    xCodeCoeff4x4( scoeff, iBlockType );
  }
  else if ( uiSize == 8*8 )
  {
    for (uiScanning=0; uiScanning<64; uiScanning++)
    {
      scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];

#if QC_MDDT// VLC_MDDT ADAPTIVE_SCAN
      if(scoeff[63-uiScanning])
      {
        if(g_bUpdateStats && pcCU->isIntra( uiAbsPartIdx ) && eTType == TEXT_LUMA)// && (uiWidth == 8 && uiMode<=8 && indexROT == 0))
        {
          scanStats[uiScanning]++;
        }
      }
#endif
    }
#if LCEC_PHASE1
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
#endif
    iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );

    xCodeCoeff8x8( scoeff, iBlockType );
  }
  else
  {
#if LCEC_PHASE1
    if(!pcCU->isIntra( uiAbsPartIdx ))
    {
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        scoeff[63-uiScanning] = piCoeff[(pucScan[uiScanning]/8)*uiWidth + (pucScan[uiScanning]%8)];      
	    }
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
      return;
    }    
#endif


#if QC_MDDT
    if(pcCU->isIntra( uiAbsPartIdx ))
    {
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        if(scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ])
        {
          if(g_bUpdateStats && eTType == TEXT_LUMA )
            scanStats[ uiScanning ]++;
        }
      }

      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
    }
#else
#if LCEC_PHASE1_ADAPT_ENABLE
    if(pcCU->isIntra( uiAbsPartIdx ))
    {
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        scoeff[63-uiScanning] = piCoeff[ pucScan[ uiScanning ] ];
      }

      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
      xCodeCoeff8x8( scoeff, iBlockType );
    }
#else
    for (uiInterleaving=0; uiInterleaving<uiSize/64; uiInterleaving++)
    {
      uiNumSigInterleaved = 0;
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
        scoeff[63-uiScanning] = piCoeff[ pucScan[ (uiSize/64) * uiScanning + uiInterleaving ] ];

        if ( scoeff[63-uiScanning] )
        {
          uiNumSigInterleaved++;
        }
      }
      if ( uiNumSigInterleaved )
      {
        xWriteFlag( 1 );
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
        xCodeCoeff8x8( scoeff, iBlockType );
      }
      else
      {
        xWriteFlag( 0 );
      }
    }
#endif
#endif // QC_MDDT
//#endif
  }

#if LCEC_PHASE1
  if (uiCodeDCCoef == 1)
  {
    piCoeff[0] = dcCoeff;
  }
#endif

}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{
  
  xWriteFlag( uiCode );
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfFlag += 1;
#endif
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
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfUvlc += xWriteUvlc( uiCode );
  else
#endif
  xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
#if LCEC_STAT
  if (m_bAdaptFlag)
    m_uiBitAlfSvlc += xWriteSvlc( iCode );
  else
#endif
  xWriteSvlc( iCode );
}

Void TEncCavlc::estBit( estBitsSbacStruct* pcEstBitsCabac, UInt uiCTXIdx, TextType eTType )
{
#if !LCEC_PHASE1
  assert(0);
#endif
  // printf("error : no VLC mode support in this version\n");
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#if LCEC_STAT
UInt TEncCavlc::xWriteCode     ( UInt uiCode, UInt uiLength )
#else
Void TEncCavlc::xWriteCode     ( UInt uiCode, UInt uiLength )
#endif
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
#if LCEC_STAT
  return uiLength;
#endif
}

#if LCEC_STAT
UInt TEncCavlc::xWriteUvlc     ( UInt uiCode )
#else
Void TEncCavlc::xWriteUvlc     ( UInt uiCode )
#endif
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
#if LCEC_STAT
  return uiLength;
#endif
}

#if LCEC_STAT
UInt TEncCavlc::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;
  UInt uiNumBits;

  uiCode = xConvertToUInt( iCode );
  uiNumBits = xWriteUvlc( uiCode );
  return uiNumBits;
}
#else
Void TEncCavlc::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;

  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
}
#endif
#ifdef DCM_PBIC
#if LCEC_STAT
UInt TEncCavlc::xWriteSvlcNZ   ( Int iCode )
{
  UInt uiNumBits;
  
  if (iCode == 0)
    return 0;
  
  UInt uiSign = 0;
  if (iCode < 0)
  {
    uiSign = 1;
    iCode = -iCode;
  }
  
  uiNumBits = xWriteUvlc( iCode-1 );
  xWriteFlag( uiSign );
  return uiNumBits+1;
}
#else // LCEC_STAT
Void TEncCavlc::xWriteSvlcNZ   ( Int iCode )
{
  if (iCode == 0)
    return;

  UInt uiSign = 0;
  if (iCode < 0)
  {
    uiSign = 1;
    iCode = -iCode;
  }

  xWriteUvlc( iCode-1 );
  xWriteFlag( uiSign );
}
#endif // LCEC_STAT
#endif // DCM_PBIC

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

#if LCEC_STAT
UInt TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
#else
Void TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
#endif
{
#if LCEC_STAT
  UInt uiNumBits = 0;
#endif
  xWriteFlag( uiSymbol ? 1 : 0 );
#if LCEC_STAT
  uiNumBits +=1;
#endif
  if ( uiSymbol == 0 )
  {
#if LCEC_STAT
    return uiNumBits;
#else
    return;
#endif
  }

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    uiNumBits +=1;
#endif
  }
  if( bCodeLast )
  {
    xWriteFlag( 0 );
#if LCEC_STAT
    uiNumBits +=1;
#endif
  }
#if LCEC_STAT
  return uiNumBits;
#else
  return;
#endif
}

#if LCEC_STAT
UInt TEncCavlc::xWriteExGolombLevel( UInt uiSymbol )
{
  UInt uiNumBits = 0;
#else
Void TEncCavlc::xWriteExGolombLevel( UInt uiSymbol )
{
#endif
  if( uiSymbol )
  {
    xWriteFlag( 1 );
#if LCEC_STAT
    uiNumBits += 1;
#endif
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);

    while( --uiSymbol && ++uiCount < 13 )
    {
      xWriteFlag( 1 );
#if LCEC_STAT
      uiNumBits += 1;
#endif
    }
    if( bNoExGo )
    {
      xWriteFlag( 0 );
#if LCEC_STAT
      uiNumBits += 1;
#endif
    }
    else
    {
#if LCEC_STAT     
      uiNumBits += xWriteEpExGolomb( uiSymbol, 0 );
#else
      xWriteEpExGolomb( uiSymbol, 0 );
#endif
    }
  }
  else
  {
    xWriteFlag( 0 );
#if LCEC_STAT
    uiNumBits += 1;
#endif
  }

#if LCEC_STAT
  return uiNumBits;
#else
  return;
#endif
}

#if LCEC_STAT
UInt TEncCavlc::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  UInt uiNumBits = 0;
#else
Void TEncCavlc::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
#endif
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xWriteFlag( 1 );
#if LCEC_STAT    
    uiNumBits += 1;
#endif
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteFlag( 0 );
#if LCEC_STAT    
  uiNumBits += 1;
#endif
  while( uiCount-- )
  {
    xWriteFlag( (uiSymbol>>uiCount) & 1 );
#if LCEC_STAT    
    uiNumBits += 1;
#endif
  }
#if LCEC_STAT
  return uiNumBits;
#else
  return;
#endif
}

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

#if LCEC_STAT
UInt TEncCavlc::xWriteVlc(UInt uiTableNumber, UInt uiCodeNumber)
{
  UInt uiNumBits = 0;
#else
Void TEncCavlc::xWriteVlc(UInt uiTableNumber, UInt uiCodeNumber)
{
#endif
  assert( uiTableNumber<=10 );

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
  xWriteCode(uiCode, uiLength);
#if LCEC_STAT
  return uiLength;
#endif
}

Void TEncCavlc::xCodeCoeff4x4(TCoeff* scoeff, Int n )
{
  Int i;
  UInt cn;
  Int level,vlc,sign,done,last_pos,start;
  Int run_done,maxrun,run,lev;
  Int tmprun, vlc_adaptive=0;
  static const int atable[5] = {4,6,14,28,0xfffffff};
  Int tmp;

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

  {
    int x,y,cx,cy,vlcNum;
    int vlcTable[3] = {2,2,2};

    x = 16*lev + last_pos;
    
    cx = m_uiLPTableE4[n][x];
    vlcNum = vlcTable[n];

#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( vlcNum, cx );
    else
#endif
    xWriteVlc( vlcNum, cx );

    if ( m_bAdaptFlag )
    {
     
      cy = Max( 0, cx-1 );
      y = m_uiLPTableD4[n][cy];
      m_uiLPTableD4[n][cy] = x;
      m_uiLPTableD4[n][cx] = y;
      m_uiLPTableE4[n][x] = cy;
      m_uiLPTableE4[n][y] = cx;
    }
  }

  sign = (scoeff[i] < 0) ? 1 : 0;
  if (level > 1)
  {
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( 0, 2*(level-2)+sign );
    else
#endif
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += 1;
#endif
  }
  i++;

  if (i < 16)
  {
    /* Go into run mode */
    run_done = 0;
    while (!run_done)
    {
      maxrun = 15-i;
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
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitCoeff += xWriteVlc( vlc, cn );
          else
#endif
          xWriteVlc( vlc, cn );

          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += xWriteVlc( 0, 2*(level-2)+sign );
            else
#endif
            xWriteVlc( 0, 2*(level-2)+sign );
            run_done = 1;
          }
          else
          {
            xWriteFlag( sign );
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += 1;
#endif
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
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += xWriteVlc( vlc, cn );
            else
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
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( vlc_adaptive, tmp );
    else
#endif
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitCoeff += 1;
#endif
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
  int tmprun,vlc_adaptive=0;
  static const int atable[5] = {4,6,14,28,0xfffffff};
  int tmp;

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
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( vlcNum, cx );
    else
#endif
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
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( 0, 2*(level-2)+sign );
    else
#endif
    xWriteVlc( 0, 2*(level-2)+sign );
  }
  else
  {
    xWriteFlag( sign );
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += 1;
#endif
  }
  i++;

  if (i < 64)
  {
    /* Go into run mode */
    run_done = 0;
    while ( !run_done )
    {
      maxrun = 63-i;
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
          if (maxrun > 27)
          {
            cn = g_auiLumaRun8x8[28][lev][run];
          }
          else
          {
            cn = g_auiLumaRun8x8[maxrun][lev][run];
          }
#if LCEC_STAT
          if (m_bAdaptFlag)
            m_uiBitCoeff += xWriteVlc( vlc, cn );
          else
#endif
          xWriteVlc( vlc, cn );

          sign = (scoeff[i] < 0) ? 1 : 0;
          if (level > 1)
          {
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += xWriteVlc( 0, 2*(level-2)+sign );
            else
#endif
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
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += 1;
#endif
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
            if (maxrun > 27)
            {
              cn = g_auiLumaRun8x8[28][0][run];
            }
            else
            {
              cn = g_auiLumaRun8x8[maxrun][0][run];
            }
#if LCEC_STAT
            if (m_bAdaptFlag)
              m_uiBitCoeff += xWriteVlc( vlc, cn );
            else
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
#if LCEC_STAT
    if (m_bAdaptFlag)
      m_uiBitCoeff += xWriteVlc( vlc_adaptive, tmp );
    else
#endif
    xWriteVlc( vlc_adaptive, tmp );
    if (scoeff[i])
    {
      sign = (scoeff[i] < 0) ? 1 : 0;
      xWriteFlag( sign );
#if LCEC_STAT
      if (m_bAdaptFlag)
        m_uiBitCoeff += 1;
#endif
    }
    if (tmp>atable[vlc_adaptive])
    {
      vlc_adaptive++;
    }
  }

  return;
}

#ifdef QC_SIFO
#if SIFO_DIF_COMPATIBILITY==1
Void TEncCavlc::encodeSwitched_Filters(TComSlice* pcSlice,TComPrediction *m_cPrediction)
{
  if(pcSlice->getSliceType() != I_SLICE)
  {	
    UInt num_AVALABLE_FILTERS = m_cPrediction->getNum_AvailableFilters();
    UInt num_SIFO = m_cPrediction->getNum_SIFOFilters();

#if SIFO_DIF_COMPATIBILITY==1    //16,17,18,19 ----> 4,5,6,7
    UInt SIFOFilter[16];
    for(UInt sub_pos = 1; sub_pos < 16; ++sub_pos)
      SIFOFilter[sub_pos] = m_cPrediction->getSIFOFilter(sub_pos);

    if(pcSlice->getSPS()->getDIFTap()==6 && pcSlice->getSliceType()==B_SLICE)
    {
      num_AVALABLE_FILTERS <<= 1;
      UInt DIF_filter_position = num_SIFO - m_cPrediction->getNum_AvailableFilters();
      for(UInt sub_pos = 1; sub_pos < 16; ++sub_pos)
      {
        if(SIFOFilter[sub_pos] >= DIF_filter_position)
          SIFOFilter[sub_pos] -= (num_SIFO-num_AVALABLE_FILTERS);
      }
    }
#endif

    Int bitsPerFilter=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); 
    Int bitsPer2Filters=(Int)ceil(log10((Double)num_SIFO)/log10((Double)2)); 
    Int sub_pos;
    //Int bestFilter = m_cPrediction->getBestFilter();

#ifdef QC_SIFO_PRED
    Int predict_filter_flag = pcSlice->getSPS()->getUseSIFO_Pred()? 1 : 0;
    Int predFiltP = m_cPrediction->getPredictFilterP();
    Int predFiltB = m_cPrediction->getPredictFilterB();

    if (predict_filter_flag && pcSlice->getSliceType()==P_SLICE && predFiltP<2)
      predict_filter_flag = 0;
    if (predict_filter_flag && pcSlice->getSliceType()==B_SLICE && predFiltB<2)
      predict_filter_flag = 0;

    xWriteFlag ( predict_filter_flag );
#if LCEC_STAT
    m_uiBitSF += 1;
#endif
#endif

    if (pcSlice->getSliceType()==P_SLICE)
    {
      Int bestFilter = m_cPrediction->getBestFilter_P();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      Int predictFilterP = m_cPrediction->getPredictFilterP();
      if (predictFilterP < 2)
#endif
      {
#if LCEC_STAT
        m_uiBitSF +=   
#endif
          xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
#if LCEC_STAT
          m_uiBitSF += 
#endif
            xWriteCode(bestFilter==1,1);
          if (bestFilter>1)
#if LCEC_STAT
            m_uiBitSF += 
#endif
            xWriteCode(bestFilter-2,bitsPerFilter);
        }

        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
#if LCEC_STAT
              m_uiBitSF +=       
#endif
              xWriteCode(SIFOFilter[sub_pos],bitsPerFilter);
            else
#if LCEC_STAT
              m_uiBitSF +=       
#endif
              xWriteCode(SIFOFilter[sub_pos],bitsPer2Filters);
          }
        }

        if (bestFilter == 1)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
#if LCEC_STAT
            m_uiBitSF +=       
#endif
              xWriteCode(SIFOFilter[sub_pos],bitsPerFilter);
          }
        }
      }
      else
      {				// Predictive coding
#if LCEC_STAT
        m_uiBitSF +=       
#endif
          xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
#if LCEC_STAT
          m_uiBitSF +=       
#endif
            xWriteCode(bestFilter-1,bitsPerFilter);
        }
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceP(sub_pos);
#if LCEC_STAT
            m_uiBitSF +=       
#endif
              xWriteCode(predict,1);
            if (predict==0)
            {
              if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
#if LCEC_STAT
                m_uiBitSF +=       
#endif
                xWriteCode(SIFOFilter[sub_pos],bitsPerFilter);
              else
#if LCEC_STAT
                m_uiBitSF +=       
#endif
                xWriteCode(SIFOFilter[sub_pos],bitsPer2Filters);
            }
          }
        }
      }
    }
    else  //B slice
    {
      Int bestFilter = m_cPrediction->getBestFilter_B();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      Int predictFilterB = m_cPrediction->getPredictFilterB();
      if (predictFilterB < 2)
#endif
      {
#if LCEC_STAT
        m_uiBitSF +=   
#endif
          xWriteCode(bestFilter==0,1);
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
#if LCEC_STAT
            m_uiBitSF +=   
#endif
              xWriteCode(SIFOFilter[sub_pos],bitsPerFilter);
          }
        }
        else
        {
#if LCEC_STAT
          m_uiBitSF +=   
#endif
            xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
      else
      {   // Predictive coding
#if LCEC_STAT
        m_uiBitSF +=   
#endif
          xWriteCode(bestFilter==0,1);
        if (bestFilter==0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceB(sub_pos);
#if LCEC_STAT
            m_uiBitSF +=   
#endif
              xWriteCode(predict,1);
            if (predict!=1)
#if LCEC_STAT
              m_uiBitSF +=   
#endif
              xWriteCode(SIFOFilter[sub_pos],bitsPerFilter);
          }
        }
        else
        {
#if LCEC_STAT
          m_uiBitSF +=   
#endif
            xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
    }
  }

  //----encode Offsets
  if(pcSlice->getSliceType() != I_SLICE)
  {
    Int offset;
    Int listNo = (pcSlice->getSliceType() == B_SLICE)? 2: 1;
    for(Int list = 0; list < listNo; ++list) 
    {
      UInt nonzero = 0;
      nonzero = m_cPrediction->isOffsetZero(pcSlice, list);
#if LCEC_STAT
      m_uiBitSF +=   
#endif
        xWriteCode(nonzero,1);
      if(nonzero)
      {
        for(UInt frame = 0; frame < pcSlice->getNumRefIdx(RefPicList(list)); ++frame)
        {
          if(frame == 0)     
          {    
            for(UInt sub_pos = 0; sub_pos < 16; ++sub_pos)   
            {
              offset = m_cPrediction->getSubpelOffset(list,sub_pos);
              offset /= (1<<g_uiBitIncrement);
#if LCEC_STAT
              m_uiBitSF +=   
#endif
                xWriteSvlc(offset);
            }         
          }
          else              
          {
            offset = m_cPrediction->getFrameOffset(list,frame);
            offset /= (1<<g_uiBitIncrement);
#if LCEC_STAT
            m_uiBitSF +=   
#endif
              xWriteSvlc(offset);
          }
        }
      }
    }
  }

}

#else
Void TEncCavlc::encodeSwitched_Filters(TComSlice* pcSlice,TComPrediction *m_cPrediction)
{
  if(pcSlice->getSliceType() != I_SLICE)
  {	
    UInt num_AVALABLE_FILTERS = m_cPrediction->getNum_AvailableFilters();
    UInt num_SIFO = m_cPrediction->getNum_SIFOFilters();

    Int bitsPerFilter=(Int)ceil(log10((Double)num_AVALABLE_FILTERS)/log10((Double)2)); 
    Int bitsPer2Filters=(Int)ceil(log10((Double)num_SIFO)/log10((Double)2)); 
    Int sub_pos;
    //Int bestFilter = m_cPrediction->getBestFilter();

#ifdef QC_SIFO_PRED
    Int predict_filter_flag = pcSlice->getSPS()->getUseSIFO_Pred()? 1 : 0;
    Int predFiltP = m_cPrediction->getPredictFilterP();
    Int predFiltB = m_cPrediction->getPredictFilterB();

    if (predict_filter_flag && pcSlice->getSliceType()==P_SLICE && predFiltP<2)
      predict_filter_flag = 0;
    if (predict_filter_flag && pcSlice->getSliceType()==B_SLICE && predFiltB<2)
      predict_filter_flag = 0;

      xWriteFlag ( predict_filter_flag );
#if LCEC_STAT
      m_uiBitSF += 1;
#endif
#endif

    if (pcSlice->getSliceType()==P_SLICE)
    {
      Int bestFilter = m_cPrediction->getBestFilter_P();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      Int predictFilterP = m_cPrediction->getPredictFilterP();
      if (predictFilterP < 2)
#endif
      {
#if LCEC_STAT
        m_uiBitSF += xWriteCode(bestFilter==0,1);    
#endif
        xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
#if LCEC_STAT
          m_uiBitSF += xWriteCode(bestFilter==1,1);         
#endif
          xWriteCode(bestFilter==1,1);
          if (bestFilter>1)
#if LCEC_STAT
            m_uiBitSF += xWriteCode(bestFilter-2,bitsPerFilter);           
#endif
            xWriteCode(bestFilter-2,bitsPerFilter);
        }

        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
#if LCEC_STAT
              m_uiBitSF +=       
#endif
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
            else
#if LCEC_STAT
              m_uiBitSF += 
#endif
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPer2Filters);
          }
        }

        if (bestFilter == 1)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
#if LCEC_STAT
            m_uiBitSF +=     
#endif
            xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
      }
      else
      {				// Predictive coding
#if LCEC_STAT
        m_uiBitSF += 
#endif
        xWriteCode(bestFilter==0,1);
        if (bestFilter>0)
        {
#if LCEC_STAT
          m_uiBitSF +=   
#endif
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceP(sub_pos);
#if LCEC_STAT
            m_uiBitSF +=     
#endif
            xWriteCode(predict,1);
            if (predict==0)
            {
              if (sub_pos<=4 || sub_pos==8 || sub_pos==12)
#if LCEC_STAT
                m_uiBitSF +=           
#endif
                xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
              else
#if LCEC_STAT
                m_uiBitSF +=           
#endif
                xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPer2Filters);
            }
          }
        }
      }
    }
    else  //B slice
    {      
      Int bestFilter = m_cPrediction->getBestFilter_B();
#ifdef QC_SIFO_PRED
      if (!predict_filter_flag)
#else
      Int predictFilterB = m_cPrediction->getPredictFilterB();
      if (predictFilterB < 2)
#endif
      {
#if LCEC_STAT
        m_uiBitSF +=   
#endif
        xWriteCode(bestFilter==0,1);
        if (bestFilter == 0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {

#if LCEC_STAT
            m_uiBitSF +=   
#endif
            xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
        else
        {
#if LCEC_STAT
          m_uiBitSF +=      
#endif
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
      else
      {
#if LCEC_STAT
        m_uiBitSF += 
#endif
        xWriteCode(bestFilter==0,1);
        if (bestFilter==0)
        {
          for(sub_pos = 1; sub_pos < 16; ++sub_pos)
          {
            UInt predict = m_cPrediction->getPredictFilterSequenceB(sub_pos);
#if LCEC_STAT
            m_uiBitSF +=    
#endif
            xWriteCode(predict,1);
            if (predict!=1)
#if LCEC_STAT
              m_uiBitSF +=     
#endif
              xWriteCode(m_cPrediction->getSIFOFilter(sub_pos),bitsPerFilter);
          }
        }
        else
        {
#if LCEC_STAT
          m_uiBitSF += 
#endif
          xWriteCode(bestFilter-1,bitsPerFilter);
        }
      }
    }
  }

//----encode Offsets
  if(pcSlice->getSliceType() != I_SLICE)
  {
    Int offset;
    Int listNo = (pcSlice->getSliceType() == B_SLICE)? 2: 1;
    for(Int list = 0; list < listNo; ++list) 
    {
      UInt nonzero = 0;
      nonzero = m_cPrediction->isOffsetZero(pcSlice, list);

#if LCEC_STAT
      m_uiBitSF += 
#endif
      xWriteCode(nonzero,1);
      if(nonzero)
      {
        for(UInt frame = 0; frame < pcSlice->getNumRefIdx(RefPicList(list)); ++frame)
        {
          if(frame == 0)     
          {    
            for(UInt sub_pos = 0; sub_pos < 16; ++sub_pos)   
            {
              offset = m_cPrediction->getSubpelOffset(list,sub_pos);
              offset /= (1<<g_uiBitIncrement);
#if LCEC_STAT
              m_uiBitSF +=    
#endif
              xWriteSvlc(offset);
            }         
          }
          else              
          {
            offset = m_cPrediction->getFrameOffset(list,frame);
            offset /= (1<<g_uiBitIncrement);
#if LCEC_STAT
            m_uiBitSF +=  
#endif
            xWriteSvlc(offset);
          }
        }
      }
    }
  }

}
#endif  //SIFO_DIF_COMPATIBILITY
#endif  //QC_SIFO
