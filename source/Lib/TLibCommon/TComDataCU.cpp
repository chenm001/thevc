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

/** \file     TComDataCU.cpp
    \brief    CU data structure
    \todo     not all entities are documented
*/

#include "TComDataCU.h"
#include "TComPic.h"

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComDataCU::TComDataCU()
{
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  m_puhDepth           = NULL;
  
  m_pePartSize         = NULL;
  m_pePredMode         = NULL;
  m_puiAlfCtrlFlag     = NULL;
  m_puiTmpAlfCtrlFlag  = NULL;
  m_puhWidth           = NULL;
  m_puhHeight          = NULL;
  m_phQP               = NULL;
  m_pbMergeFlag        = NULL;
  m_puhMergeIndex      = NULL;
 for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    m_apuhNeighbourCandIdx[ui] = NULL;
  }
  m_puhLumaIntraDir    = NULL;
  m_puhChromaIntraDir  = NULL;
  m_puhInterDir        = NULL;
  m_puhTrIdx           = NULL;
  m_puhCbf[0]          = NULL;
  m_puhCbf[1]          = NULL;
  m_puhCbf[2]          = NULL;
  m_pcTrCoeffY         = NULL;
  m_pcTrCoeffCb        = NULL;
  m_pcTrCoeffCr        = NULL;
  
#if E057_INTRA_PCM
  m_pbIPCMFlag         = NULL;
  m_pcIPCMSampleY      = NULL;
  m_pcIPCMSampleCb     = NULL;
  m_pcIPCMSampleCr     = NULL;
#endif

  m_pcPattern          = NULL;
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
  
  m_apcCUColocated[0]  = NULL;
  m_apcCUColocated[1]  = NULL;
  
  m_apiMVPIdx[0]       = NULL;
  m_apiMVPIdx[1]       = NULL;
  m_apiMVPNum[0]       = NULL;
  m_apiMVPNum[1]       = NULL;
  
  m_bDecSubCu          = false;
  m_uiSliceStartCU        = 0;
  m_uiEntropySliceStartCU = 0;
}

TComDataCU::~TComDataCU()
{
}

Void TComDataCU::create(UInt uiNumPartition, UInt uiWidth, UInt uiHeight, Bool bDecSubCu, Int unitSize)
{
  m_bDecSubCu = bDecSubCu;
  
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  m_uiNumPartition     = uiNumPartition;
  m_unitSize = unitSize;
  
  if ( !bDecSubCu )
  {
    m_phQP               = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhDepth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhWidth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhHeight          = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_pePartSize         = new Char[ uiNumPartition ];
    m_pePredMode         = new Char[ uiNumPartition ];
    
    m_puiAlfCtrlFlag     = new Bool[ uiNumPartition ];
    
    m_pbMergeFlag        = (Bool*  )xMalloc(Bool,   uiNumPartition);
    m_puhMergeIndex      = (UChar* )xMalloc(UChar,  uiNumPartition);
    for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
    {
      m_apuhNeighbourCandIdx[ ui ] = (UChar* )xMalloc(UChar, uiNumPartition);
    }
    
    m_puhLumaIntraDir    = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhChromaIntraDir  = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_apiMVPIdx[0]       = new Char[ uiNumPartition ];
    m_apiMVPIdx[1]       = new Char[ uiNumPartition ];
    m_apiMVPNum[0]       = new Char[ uiNumPartition ];
    m_apiMVPNum[1]       = new Char[ uiNumPartition ];
    
    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    
#if E057_INTRA_PCM
    m_pbIPCMFlag         = (Bool*  )xMalloc(Bool, uiNumPartition);
    m_pcIPCMSampleY      = (Pel*   )xMalloc(Pel , uiWidth*uiHeight);
    m_pcIPCMSampleCb     = (Pel*   )xMalloc(Pel , uiWidth*uiHeight/4);
    m_pcIPCMSampleCr     = (Pel*   )xMalloc(Pel , uiWidth*uiHeight/4);
#endif

    m_acCUMvField[0].create( uiNumPartition );
    m_acCUMvField[1].create( uiNumPartition );
    
  }
  else
  {
    m_acCUMvField[0].setNumPartition(uiNumPartition );
    m_acCUMvField[1].setNumPartition(uiNumPartition );
  }
  
#if FINE_GRANULARITY_SLICES
  m_uiSliceStartCU        = (UInt*  )xMalloc(UInt, uiNumPartition);
  m_uiEntropySliceStartCU = (UInt*  )xMalloc(UInt, uiNumPartition);
#endif
  // create pattern memory
  m_pcPattern            = (TComPattern*)xMalloc(TComPattern, 1);
  
  // create motion vector fields
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
  
  m_apcCUColocated[0]  = NULL;
  m_apcCUColocated[1]  = NULL;
}

Void TComDataCU::destroy()
{
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  
  if ( m_pcPattern )
  { 
    xFree(m_pcPattern);
    m_pcPattern = NULL;
  }
  
  // encoder-side buffer free
  if ( !m_bDecSubCu )
  {
    if ( m_phQP               ) { xFree(m_phQP);                m_phQP              = NULL; }
    if ( m_puhDepth           ) { xFree(m_puhDepth);            m_puhDepth          = NULL; }
    if ( m_puhWidth           ) { xFree(m_puhWidth);            m_puhWidth          = NULL; }
    if ( m_puhHeight          ) { xFree(m_puhHeight);           m_puhHeight         = NULL; }
    if ( m_pePartSize         ) { delete[] m_pePartSize;        m_pePartSize        = NULL; }
    if ( m_pePredMode         ) { delete[] m_pePredMode;        m_pePredMode        = NULL; }
    if ( m_puhCbf[0]          ) { xFree(m_puhCbf[0]);           m_puhCbf[0]         = NULL; }
    if ( m_puhCbf[1]          ) { xFree(m_puhCbf[1]);           m_puhCbf[1]         = NULL; }
    if ( m_puhCbf[2]          ) { xFree(m_puhCbf[2]);           m_puhCbf[2]         = NULL; }
    if ( m_puiAlfCtrlFlag     ) { delete[] m_puiAlfCtrlFlag;    m_puiAlfCtrlFlag    = NULL; }
    if ( m_puhInterDir        ) { xFree(m_puhInterDir);         m_puhInterDir       = NULL; }
    if ( m_pbMergeFlag        ) { xFree(m_pbMergeFlag);         m_pbMergeFlag       = NULL; }
    if ( m_puhMergeIndex      ) { xFree(m_puhMergeIndex);       m_puhMergeIndex     = NULL; }
    for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
    {
      if( m_apuhNeighbourCandIdx[ ui ] ) { xFree(m_apuhNeighbourCandIdx[ ui ]); m_apuhNeighbourCandIdx[ ui ] = NULL; }
    }
    if ( m_puhLumaIntraDir    ) { xFree(m_puhLumaIntraDir);     m_puhLumaIntraDir   = NULL; }
    if ( m_puhChromaIntraDir  ) { xFree(m_puhChromaIntraDir);   m_puhChromaIntraDir = NULL; }
    if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);            m_puhTrIdx          = NULL; }
    if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);          m_pcTrCoeffY        = NULL; }
    if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);         m_pcTrCoeffCb       = NULL; }
    if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);         m_pcTrCoeffCr       = NULL; }
#if E057_INTRA_PCM
    if ( m_pbIPCMFlag         ) { xFree(m_pbIPCMFlag   );       m_pbIPCMFlag        = NULL; }
    if ( m_pcIPCMSampleY      ) { xFree(m_pcIPCMSampleY);       m_pcIPCMSampleY     = NULL; }
    if ( m_pcIPCMSampleCb     ) { xFree(m_pcIPCMSampleCb);      m_pcIPCMSampleCb    = NULL; }
    if ( m_pcIPCMSampleCr     ) { xFree(m_pcIPCMSampleCr);      m_pcIPCMSampleCr    = NULL; }
#endif
    if ( m_apiMVPIdx[0]       ) { delete[] m_apiMVPIdx[0];      m_apiMVPIdx[0]      = NULL; }
    if ( m_apiMVPIdx[1]       ) { delete[] m_apiMVPIdx[1];      m_apiMVPIdx[1]      = NULL; }
    if ( m_apiMVPNum[0]       ) { delete[] m_apiMVPNum[0];      m_apiMVPNum[0]      = NULL; }
    if ( m_apiMVPNum[1]       ) { delete[] m_apiMVPNum[1];      m_apiMVPNum[1]      = NULL; }
    
    m_acCUMvField[0].destroy();
    m_acCUMvField[1].destroy();
    
  }
  
  m_pcCUAboveLeft       = NULL;
  m_pcCUAboveRight      = NULL;
  m_pcCUAbove           = NULL;
  m_pcCULeft            = NULL;
  
  m_apcCUColocated[0]   = NULL;
  m_apcCUColocated[1]   = NULL;

#if FINE_GRANULARITY_SLICES
  if( m_uiSliceStartCU )
  {
    xFree(m_uiSliceStartCU);
    m_uiSliceStartCU=NULL;
  }
  if(m_uiEntropySliceStartCU )
  {
    xFree(m_uiEntropySliceStartCU);
    m_uiEntropySliceStartCU=NULL;
  }
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Initialization
// --------------------------------------------------------------------------------------------------------------------

/**
 - initialize top-level CU
 - internal buffers are already created
 - set values before encoding a CU
 .
 \param  pcPic     picture (TComPic) class pointer
 \param  iCUAddr   CU address
 */
#if !FINE_GRANULARITY_SLICES
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{
  TComSlice* pcSlice   = pcPic->getSlice(pcPic->getCurrSliceIdx());
  m_pcSlice            = pcSlice;
  m_uiSliceStartCU     = pcSlice->getSliceCurStartCUAddr();
  m_uiEntropySliceStartCU  = pcSlice->getEntropySliceCurStartCUAddr();
  m_pcPic              = pcPic;
  m_uiCUAddr           = iCUAddr;
  m_uiCUPelX           = ( iCUAddr % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth;
  m_uiCUPelY           = ( iCUAddr / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight;
  m_uiAbsIdxInLCU      = 0;
  
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiNumPartition     = pcPic->getNumPartInCU();
  
  Int iSizeInUchar = sizeof( UChar ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt  ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool  ) * m_uiNumPartition;
  
  memset( m_phQP,               pcSlice->getSliceQp(), iSizeInUchar );
  memset( m_puiAlfCtrlFlag,     0, iSizeInBool  );
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memset( m_apuhNeighbourCandIdx[ ui ], 0, iSizeInUchar );
  }
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,           0, iSizeInUchar );
  
  UChar uhWidth  = g_uiMaxCUWidth;
  UChar uhHeight = g_uiMaxCUHeight;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );
#if E057_INTRA_PCM
  memset( m_pbIPCMFlag,        0, iSizeInBool );
#endif

  memset( m_apiMVPIdx[0], -1, sizeof(m_apiMVPIdx[0][0]) * m_uiNumPartition);
  memset( m_apiMVPIdx[1], -1, sizeof(m_apiMVPIdx[1][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[0], -1, sizeof(m_apiMVPNum[0][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[1], -1, sizeof(m_apiMVPNum[1][0]) * m_uiNumPartition);
  
  memset(m_pePartSize, SIZE_NONE, sizeof( *m_pePartSize ) * m_uiNumPartition );
  memset(m_pePredMode, MODE_NONE, sizeof( *m_pePredMode ) * m_uiNumPartition );
  
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof( TCoeff ) * uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif
  uiTmp  >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof( TCoeff ) * uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof( TCoeff ) * uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif

  // setting neighbor CU
  m_pcCULeft        = NULL;
  m_pcCUAbove       = NULL;
  m_pcCUAboveLeft   = NULL;
  m_pcCUAboveRight  = NULL;
  
  m_apcCUColocated[0] = NULL;
  m_apcCUColocated[1] = NULL;
  
  UInt uiWidthInCU = pcPic->getFrameWidthInCU();
  if ( m_uiCUAddr % uiWidthInCU )
  {
    m_pcCULeft = pcPic->getCU( m_uiCUAddr - 1 );
  }
  
  if ( m_uiCUAddr / uiWidthInCU )
  {
    m_pcCUAbove = pcPic->getCU( m_uiCUAddr - uiWidthInCU );
  }
  
  if ( m_pcCULeft && m_pcCUAbove )
  {
    m_pcCUAboveLeft = pcPic->getCU( m_uiCUAddr - uiWidthInCU - 1 );
  }
  
  if ( m_pcCUAbove && ( (m_uiCUAddr%uiWidthInCU) < (uiWidthInCU-1) )  )
  {
    m_pcCUAboveRight = pcPic->getCU( m_uiCUAddr - uiWidthInCU + 1 );
  }
  
  if ( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
  {
    m_apcCUColocated[0] = pcSlice->getRefPic( REF_PIC_LIST_0, 0)->getCU( m_uiCUAddr );
  }

  if ( pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
  {
    m_apcCUColocated[1] = pcSlice->getRefPic( REF_PIC_LIST_1, 0)->getCU( m_uiCUAddr );
  }
}
#else
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{

  m_pcPic              = pcPic;
  m_pcSlice            = pcPic->getSlice(pcPic->getCurrSliceIdx());
  m_uiCUAddr           = iCUAddr;
  m_uiCUPelX           = ( iCUAddr % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth;
  m_uiCUPelY           = ( iCUAddr / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight;
  m_uiAbsIdxInLCU      = 0;
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;
  m_uiNumPartition     = pcPic->getNumPartInCU();
  for(int i=0; i<pcPic->getNumPartInCU(); i++) 
  {
    if(iCUAddr*pcPic->getNumPartInCU()+i>=getSlice()->getSliceCurStartCUAddr())
    {
      m_uiSliceStartCU[i]=getSlice()->getSliceCurStartCUAddr();
    }
    else
    {
      m_uiSliceStartCU[i]=pcPic->getCU(getAddr())->m_uiSliceStartCU[i];
    }
  }
  for(int i=0; i<pcPic->getNumPartInCU(); i++) 
  {
    if(iCUAddr*pcPic->getNumPartInCU()+i>=getSlice()->getEntropySliceCurStartCUAddr())
    {
      m_uiEntropySliceStartCU[i]=getSlice()->getEntropySliceCurStartCUAddr();
    }
    else
    {
      m_uiEntropySliceStartCU[i]=pcPic->getCU(getAddr())->m_uiEntropySliceStartCU[i];
    }
  }

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    if(iCUAddr*pcPic->getNumPartInCU()+ui>=getSlice()->getEntropySliceCurStartCUAddr()) 
    {
      m_pePartSize[ui] = SIZE_NONE;
      m_pePredMode[ui] = MODE_NONE;
      m_puhDepth  [ui] = 0;
      m_puhTrIdx  [ui] = 0;
      m_puhWidth  [ui] = g_uiMaxCUWidth;
      m_puhHeight [ui] = g_uiMaxCUHeight;
      m_apiMVPIdx[0][ui] = -1;
      m_apiMVPIdx[1][ui] = -1;
      m_apiMVPNum[0][ui] = -1;
      m_apiMVPNum[1][ui] = -1;
      m_phQP[ui]=getSlice()->getSliceQp();
      m_puiAlfCtrlFlag[ui] = false;
      m_pbMergeFlag[ui]=0;
      m_puhMergeIndex[ui]=0;
      for( UInt ui2 = 0; ui2 < MRG_MAX_NUM_CANDS; ui2++ )
      {
        m_apuhNeighbourCandIdx[ ui2 ][ui]=0;
      }
      m_puhLumaIntraDir[ui]=2;
      m_puhChromaIntraDir[ui]=0;
      m_puhInterDir[ui]=0;
      m_puhCbf[0][ui]=0;
      m_puhCbf[1][ui]=0;
      m_puhCbf[2][ui]=0;
#if E057_INTRA_PCM
      m_pbIPCMFlag[ui] = 0;
#endif
    }
    else 
    {
      TComDataCU * pcFrom = pcPic->getCU(getAddr());
      m_pePartSize[ui] = pcFrom->getPartitionSize(ui);
      m_pePredMode[ui] = pcFrom->getPredictionMode(ui);
      m_puhDepth[ui] = pcFrom->getDepth(ui);
      m_puhWidth  [ui] = pcFrom->getWidth(ui);
      m_puhHeight [ui] = pcFrom->getHeight(ui);
      m_puhTrIdx  [ui] = pcFrom->getTransformIdx(ui);
      m_apiMVPIdx[0][ui] = pcFrom->m_apiMVPIdx[0][ui];;
      m_apiMVPIdx[1][ui] = pcFrom->m_apiMVPIdx[1][ui];
      m_apiMVPNum[0][ui] = pcFrom->m_apiMVPNum[0][ui];
      m_apiMVPNum[1][ui] = pcFrom->m_apiMVPNum[1][ui];
      m_phQP[ui]=pcFrom->m_phQP[ui];
      m_puiAlfCtrlFlag[ui]=pcFrom->m_puiAlfCtrlFlag[ui];
      m_pbMergeFlag[ui]=pcFrom->m_pbMergeFlag[ui];
      m_puhMergeIndex[ui]=pcFrom->m_puhMergeIndex[ui];
      for( UInt ui2 = 0; ui2 < MRG_MAX_NUM_CANDS; ui2++ )
      {
        m_apuhNeighbourCandIdx[ ui2 ][ui]=pcFrom->m_apuhNeighbourCandIdx[ui2][ui];
      }
      m_puhLumaIntraDir[ui]=pcFrom->m_puhLumaIntraDir[ui];
      m_puhChromaIntraDir[ui]=pcFrom->m_puhChromaIntraDir[ui];
      m_puhInterDir[ui]=pcFrom->m_puhInterDir[ui];
      m_puhCbf[0][ui]=pcFrom->m_puhCbf[0][ui];
      m_puhCbf[1][ui]=pcFrom->m_puhCbf[1][ui];
      m_puhCbf[2][ui]=pcFrom->m_puhCbf[2][ui];
#if E057_INTRA_PCM
      m_pbIPCMFlag[ui] = pcFrom->m_pbIPCMFlag[ui];
#endif
    }
  }

  UInt uiTmp = g_uiMaxCUWidth*g_uiMaxCUHeight;
  if(iCUAddr*pcPic->getNumPartInCU()>=getSlice()->getEntropySliceCurStartCUAddr()) 
  {
    m_acCUMvField[0].clearMvField();
    m_acCUMvField[1].clearMvField();
    memset( m_pcTrCoeffY , 0, sizeof( TCoeff ) * uiTmp );
#if E057_INTRA_PCM
    memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif
    uiTmp  >>= 2;
    memset( m_pcTrCoeffCb, 0, sizeof( TCoeff ) * uiTmp );
    memset( m_pcTrCoeffCr, 0, sizeof( TCoeff ) * uiTmp );
#if E057_INTRA_PCM
    memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
    memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif
  }
  else 
  {
    TComDataCU * pcFrom = pcPic->getCU(getAddr());
    m_acCUMvField[0].copyFrom(&pcFrom->m_acCUMvField[0],m_uiNumPartition,0);
    m_acCUMvField[1].copyFrom(&pcFrom->m_acCUMvField[1],m_uiNumPartition,0);
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffY[i]=pcFrom->m_pcTrCoeffY[i];
#if E057_INTRA_PCM
      m_pcIPCMSampleY[i]=pcFrom->m_pcIPCMSampleY[i];
#endif
    }
    for(int i=0; i<(uiTmp>>2); i++) 
    {
      m_pcTrCoeffCb[i]=pcFrom->m_pcTrCoeffCb[i];
      m_pcTrCoeffCr[i]=pcFrom->m_pcTrCoeffCr[i];
#if E057_INTRA_PCM
      m_pcIPCMSampleCb[i]=pcFrom->m_pcIPCMSampleCb[i];
      m_pcIPCMSampleCr[i]=pcFrom->m_pcIPCMSampleCr[i];
#endif
    }
  }

  // Setting neighbor CU
  m_pcCULeft        = NULL;
  m_pcCUAbove       = NULL;
  m_pcCUAboveLeft   = NULL;
  m_pcCUAboveRight  = NULL;

  m_apcCUColocated[0] = NULL;
  m_apcCUColocated[1] = NULL;

  UInt uiWidthInCU = pcPic->getFrameWidthInCU();
  if ( m_uiCUAddr % uiWidthInCU )
  {
    m_pcCULeft = pcPic->getCU( m_uiCUAddr - 1 );
  }

  if ( m_uiCUAddr / uiWidthInCU )
  {
    m_pcCUAbove = pcPic->getCU( m_uiCUAddr - uiWidthInCU );
  }

  if ( m_pcCULeft && m_pcCUAbove )
  {
    m_pcCUAboveLeft = pcPic->getCU( m_uiCUAddr - uiWidthInCU - 1 );
  }

  if ( m_pcCUAbove && ( (m_uiCUAddr%uiWidthInCU) < (uiWidthInCU-1) )  )
  {
    m_pcCUAboveRight = pcPic->getCU( m_uiCUAddr - uiWidthInCU + 1 );
  }

  if ( getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
  {
    m_apcCUColocated[0] = getSlice()->getRefPic( REF_PIC_LIST_0, 0)->getCU( m_uiCUAddr );
  }

  if ( getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
  {
    m_apcCUColocated[1] = getSlice()->getRefPic( REF_PIC_LIST_1, 0)->getCU( m_uiCUAddr );
  }
}
#endif

#if !SUB_LCU_DQP
// initialize prediction data
Void TComDataCU::initEstData()
{
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
#if FINE_GRANULARITY_SLICES  
  m_uiTotalBins        = 0;
#endif  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
  memset( m_phQP,              getSlice()->getSliceQp(), iSizeInUchar );
  memset( m_puiAlfCtrlFlag,     0, iSizeInBool );
#if E057_INTRA_PCM
  memset( m_pbIPCMFlag,        0, iSizeInBool );
#endif
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memset( m_apuhNeighbourCandIdx[ ui ], 0, iSizeInUchar );
  }
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  
  memset( m_apiMVPIdx[0], -1, sizeof(m_apiMVPIdx[0][0]) * m_uiNumPartition);
  memset( m_apiMVPIdx[1], -1, sizeof(m_apiMVPIdx[1][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[0], -1, sizeof(m_apiMVPNum[0][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[1], -1, sizeof(m_apiMVPNum[1][0]) * m_uiNumPartition);
  
  memset( m_pePartSize, SIZE_NONE, sizeof( *m_pePartSize) * m_uiNumPartition );
  memset( m_pePredMode, MODE_NONE, sizeof( *m_pePredMode) * m_uiNumPartition );
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif

  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif

  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
}
#else
/** initialize prediction data with enabling sub-LCU-level delta QP
*\param  uiDepth  depth of the current CU 
*\param  uiQP     QP for the current CU
*\param  uiLastQP QP for the last-coded CU
*- set CU width and CU height according to depth
*- set QP value according to input QP 
*- set last-coded QP value according to input last-coded QP 
*/
Void TComDataCU::initEstData( UInt uiDepth, UInt uiQP )
{
#if FINE_GRANULARITY_SLICES
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    if(getAddr()*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU+ui >= getSlice()->getEntropySliceCurStartCUAddr()) 
    {
      m_apiMVPIdx[0][ui] = -1;
      m_apiMVPIdx[1][ui] = -1;
      m_apiMVPNum[0][ui] = -1;
      m_apiMVPNum[1][ui] = -1;
      m_puhDepth  [ui] = uiDepth;
      m_puhWidth  [ui] = uhWidth;
      m_puhHeight [ui] = uhHeight;
      m_puhTrIdx  [ui] = 0;
      m_pePartSize[ui] = SIZE_NONE;
      m_pePredMode[ui] = MODE_NONE;
#if E057_INTRA_PCM
      m_pbIPCMFlag[ui] = 0;
#endif
      m_phQP[ui] = uiQP;
      m_puiAlfCtrlFlag[ui]= false;
      m_pbMergeFlag[ui] = 0;
      m_puhMergeIndex[ui] = 0;
      for( UInt ui2 = 0; ui2 < MRG_MAX_NUM_CANDS; ui2++ )
      {
        m_apuhNeighbourCandIdx[ ui2 ][ui] = 0;
      }
      m_puhLumaIntraDir[ui] = 2;
      m_puhChromaIntraDir[ui] = 0;
      m_puhInterDir[ui] = 0;
      m_puhCbf[0][ui] = 0;
      m_puhCbf[1][ui] = 0;
      m_puhCbf[2][ui] = 0;
    }
  }

  UInt uiTmp = uhWidth*uhHeight;

  if(getAddr()*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU >= getSlice()->getEntropySliceCurStartCUAddr()) 
  {
    m_acCUMvField[0].clearMvField();
    m_acCUMvField[1].clearMvField();
    uiTmp = uhWidth*uhHeight;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffY[i] = 0;
#if E057_INTRA_PCM
      m_pcIPCMSampleY[i] = 0;
#endif
    }
    uiTmp>>=2;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffCr[i] = 0;
      m_pcTrCoeffCb[i] = 0;
#if E057_INTRA_PCM
      m_pcIPCMSampleCb[i] = 0;
      m_pcIPCMSampleCr[i] = 0;
#endif
    }
  }
#else
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;

  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
  memset( m_phQP,              uiQP, iSizeInUchar );
  memset( m_puiAlfCtrlFlag,     0, iSizeInBool );
#if E057_INTRA_PCM
  memset( m_pbIPCMFlag,         0, iSizeInBool );
#endif
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memset( m_apuhNeighbourCandIdx[ ui ], 0, iSizeInUchar );
  }
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );

  memset( m_puhDepth,     uiDepth, iSizeInUchar );

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  memset( m_apiMVPIdx[0], -1, sizeof(m_apiMVPIdx[0][0]) * m_uiNumPartition);
  memset( m_apiMVPIdx[1], -1, sizeof(m_apiMVPIdx[1][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[0], -1, sizeof(m_apiMVPNum[0][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[1], -1, sizeof(m_apiMVPNum[1][0]) * m_uiNumPartition);

  memset( m_pePartSize, SIZE_NONE, sizeof( *m_pePartSize ) * m_uiNumPartition );
  memset( m_pePredMode, MODE_NONE, sizeof( *m_pePredMode ) * m_uiNumPartition );

  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif

  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif

  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
#endif
}
#endif


// initialize Sub partition
#if !FINE_GRANULARITY_SLICES
#if SUB_LCU_DQP
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth, UInt uiQP )
#else
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
#endif
{
  assert( uiPartUnitIdx<4 );
  
  UInt uiPartOffset = ( pcCU->getTotalNumPart()>>2 )*uiPartUnitIdx;
  
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = pcCU->getZorderIdxInCU() + uiPartOffset;
#if SUB_LCU_DQP
  m_uiCUPelX           = pcCU->getCUPelX() + ( (g_uiMaxCUWidth>>uiDepth) )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( (g_uiMaxCUWidth>>uiDepth) )*( uiPartUnitIdx >> 1 );
#else
  m_uiCUPelX           = pcCU->getCUPelX() + ( pcCU->getWidth (0) >> 1 )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( pcCU->getHeight(0) >> 1 )*( uiPartUnitIdx >> 1 );
#endif
  
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;
  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
  
#if SUB_LCU_DQP
  memset( m_phQP,              uiQP, iSizeInUchar );
#else
  memset( m_phQP,              getSlice()->getSliceQp(), iSizeInUchar );
#endif
  memset( m_puiAlfCtrlFlag,     0, iSizeInBool );
#if E057_INTRA_PCM
  memset( m_pbIPCMFlag,        0, iSizeInBool  );
#endif
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memset( m_apuhNeighbourCandIdx[ ui ], 0, iSizeInUchar );
  }
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,     uiDepth, iSizeInUchar );
  
  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );
  
  memset( m_apiMVPIdx[0], -1, sizeof(m_apiMVPIdx[0][0]) * m_uiNumPartition);
  memset( m_apiMVPIdx[1], -1, sizeof(m_apiMVPIdx[1][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[0], -1, sizeof(m_apiMVPNum[0][0]) * m_uiNumPartition);
  memset( m_apiMVPNum[1], -1, sizeof(m_apiMVPNum[1][0]) * m_uiNumPartition);
  
  memset( m_pePartSize, SIZE_NONE, sizeof( *m_pePartSize ) * m_uiNumPartition );
  memset( m_pePredMode, MODE_NONE, sizeof( *m_pePredMode ) * m_uiNumPartition );
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif

  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif

  m_pcCULeft        = pcCU->getCULeft();
  m_pcCUAbove       = pcCU->getCUAbove();
  m_pcCUAboveLeft   = pcCU->getCUAboveLeft();
  m_pcCUAboveRight  = pcCU->getCUAboveRight();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
  m_uiSliceStartCU          = pcCU->getSliceStartCU();
  m_uiEntropySliceStartCU   = pcCU->getEntropySliceStartCU();
}
#else
#if SUB_LCU_DQP
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth, UInt uiQP )
#else
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
#endif
{
  assert( uiPartUnitIdx<4 );

  UInt uiPartOffset = ( pcCU->getTotalNumPart()>>2 )*uiPartUnitIdx;

  m_pcPic              = pcCU->getPic();
  m_pcSlice            = m_pcPic->getSlice(m_pcPic->getCurrSliceIdx());
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = pcCU->getZorderIdxInCU() + uiPartOffset;

  m_uiCUPelX           = pcCU->getCUPelX() + ( g_uiMaxCUWidth>>uiDepth  )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( g_uiMaxCUHeight>>uiDepth  )*( uiPartUnitIdx >> 1 );

  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;
  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;

  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;

#if SUB_LCU_DQP
  memset( m_phQP,              uiQP, iSizeInUchar );
#else
  memset( m_phQP,              getSlice()->getSliceQp(), iSizeInUchar );
#endif

  memset( m_puiAlfCtrlFlag,     0, iSizeInBool );
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memset( m_apuhNeighbourCandIdx[ ui ], 0, iSizeInUchar );
  }
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,     uiDepth, iSizeInUchar );

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );
#if E057_INTRA_PCM
  memset( m_pbIPCMFlag,        0, iSizeInBool  );
#endif
  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
    if(getAddr()*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU+ui<getSlice()->getEntropySliceCurStartCUAddr()) 
    {
      m_apiMVPIdx[0][ui] = pcCU->m_apiMVPIdx[0][uiPartOffset+ui];
      m_apiMVPIdx[1][ui] = pcCU->m_apiMVPIdx[1][uiPartOffset+ui];;
      m_apiMVPNum[0][ui] = pcCU->m_apiMVPNum[0][uiPartOffset+ui];;
      m_apiMVPNum[1][ui] = pcCU->m_apiMVPNum[1][uiPartOffset+ui];;
      m_puhDepth  [ui] = pcCU->getDepth(uiPartOffset+ui);
      m_puhWidth  [ui] = pcCU->getWidth(uiPartOffset+ui);
      m_puhHeight  [ui] = pcCU->getHeight(uiPartOffset+ui);
      m_puhTrIdx  [ui] = pcCU->getTransformIdx(uiPartOffset+ui);
      m_pePartSize[ui] = pcCU->getPartitionSize(uiPartOffset+ui);
      m_pePredMode[ui] = pcCU->getPredictionMode(uiPartOffset+ui);
#if E057_INTRA_PCM
      m_pbIPCMFlag[ui]=pcCU->m_pbIPCMFlag[uiPartOffset+ui];
#endif
      m_phQP[ui] = pcCU->m_phQP[uiPartOffset+ui];
      m_puiAlfCtrlFlag[ui]=pcCU->m_puiAlfCtrlFlag[uiPartOffset+ui];
      m_pbMergeFlag[ui]=pcCU->m_pbMergeFlag[uiPartOffset+ui];
      m_puhMergeIndex[ui]=pcCU->m_puhMergeIndex[uiPartOffset+ui];
      for( UInt ui2 = 0; ui2 < MRG_MAX_NUM_CANDS; ui2++ )
      {
        m_apuhNeighbourCandIdx[ ui2 ][ui]=pcCU->m_apuhNeighbourCandIdx[ui2][uiPartOffset+ui];
      }
      m_puhLumaIntraDir[ui]=pcCU->m_puhLumaIntraDir[uiPartOffset+ui];
      m_puhChromaIntraDir[ui]=pcCU->m_puhChromaIntraDir[uiPartOffset+ui];
      m_puhInterDir[ui]=pcCU->m_puhInterDir[uiPartOffset+ui];
      m_puhCbf[0][ui]=pcCU->m_puhCbf[0][uiPartOffset+ui];
      m_puhCbf[1][ui]=pcCU->m_puhCbf[1][uiPartOffset+ui];
      m_puhCbf[2][ui]=pcCU->m_puhCbf[2][uiPartOffset+ui];

    }
  }
  UInt uiTmp = uhWidth*uhHeight;
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
#endif
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
#endif
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();

  if(getAddr()*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU<getSlice()->getEntropySliceCurStartCUAddr()) 
  {
    // Part of this CU contains data from an older slice. Now copy in that data.
    UInt uiMaxCuWidth=pcCU->getSlice()->getSPS()->getMaxCUWidth();
    UInt uiMaxCuHeight=pcCU->getSlice()->getSPS()->getMaxCUHeight();
    TComDataCU * bigCU = getPic()->getCU(getAddr());
    Int minui = uiPartOffset;
    minui = -minui;
    pcCU->m_acCUMvField[0].copyTo(&m_acCUMvField[0],minui,uiPartOffset,m_uiNumPartition);
    pcCU->m_acCUMvField[1].copyTo(&m_acCUMvField[1],minui,uiPartOffset,m_uiNumPartition);
    UInt uiCoffOffset = uiMaxCuWidth*uiMaxCuHeight*m_uiAbsIdxInLCU/pcCU->getPic()->getNumPartInCU();
    uiTmp = uhWidth*uhHeight;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffY[i]=bigCU->m_pcTrCoeffY[uiCoffOffset+i];
#if E057_INTRA_PCM
      m_pcIPCMSampleY[i]=bigCU->m_pcIPCMSampleY[uiCoffOffset+i];
#endif
    }
    uiTmp>>=2;
    uiCoffOffset>>=2;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffCr[i]=bigCU->m_pcTrCoeffCr[uiCoffOffset+i];
      m_pcTrCoeffCb[i]=bigCU->m_pcTrCoeffCb[uiCoffOffset+i];
#if E057_INTRA_PCM
      m_pcIPCMSampleCb[i]=bigCU->m_pcIPCMSampleCb[uiCoffOffset+i];
      m_pcIPCMSampleCr[i]=bigCU->m_pcIPCMSampleCr[uiCoffOffset+i];
#endif
    }
  }

  m_pcCULeft        = pcCU->getCULeft();
  m_pcCUAbove       = pcCU->getCUAbove();
  m_pcCUAboveLeft   = pcCU->getCUAboveLeft();
  m_pcCUAboveRight  = pcCU->getCUAboveRight();

  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiPartOffset,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiPartOffset,sizeof(UInt)*m_uiNumPartition);
}
#endif

Void TComDataCU::setOutsideCUPart( UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiNumPartition = m_uiNumPartition >> (uiDepth << 1);
  UInt uiSizeInUchar = sizeof( UChar  ) * uiNumPartition;

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhDepth    + uiAbsPartIdx,     uiDepth,  uiSizeInUchar );
  memset( m_puhWidth    + uiAbsPartIdx,     uhWidth,  uiSizeInUchar );
  memset( m_puhHeight   + uiAbsPartIdx,     uhHeight, uiSizeInUchar );
}

// --------------------------------------------------------------------------------------------------------------------
// Copy
// --------------------------------------------------------------------------------------------------------------------

Void TComDataCU::copySubCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiPart = uiAbsPartIdx;
  
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  m_uiCUPelX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  m_uiCUPelY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  
  UInt uiWidth         = g_uiMaxCUWidth  >> uiDepth;
  UInt uiHeight        = g_uiMaxCUHeight >> uiDepth;
  
  m_phQP=pcCU->getQP()                    + uiPart;
  m_pePartSize = pcCU->getPartitionSize() + uiPart;
  m_pePredMode=pcCU->getPredictionMode()  + uiPart;
  
  m_pbMergeFlag         = pcCU->getMergeFlag()        + uiPart;
  m_puhMergeIndex       = pcCU->getMergeIndex()       + uiPart;
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    m_apuhNeighbourCandIdx[ ui ] = pcCU->getNeighbourCandIdx( ui ) + uiPart;
  }

  m_puhLumaIntraDir     = pcCU->getLumaIntraDir()     + uiPart;
  m_puhChromaIntraDir   = pcCU->getChromaIntraDir()   + uiPart;
  m_puhInterDir         = pcCU->getInterDir()         + uiPart;
  m_puhTrIdx            = pcCU->getTransformIdx()     + uiPart;
  
  m_puhCbf[0]= pcCU->getCbf(TEXT_LUMA)            + uiPart;
  m_puhCbf[1]= pcCU->getCbf(TEXT_CHROMA_U)        + uiPart;
  m_puhCbf[2]= pcCU->getCbf(TEXT_CHROMA_V)        + uiPart;
  
  m_puhDepth=pcCU->getDepth()                     + uiPart;
  m_puhWidth=pcCU->getWidth()                     + uiPart;
  m_puhHeight=pcCU->getHeight()                   + uiPart;
  
  m_apiMVPIdx[0]=pcCU->getMVPIdx(REF_PIC_LIST_0)  + uiPart;
  m_apiMVPIdx[1]=pcCU->getMVPIdx(REF_PIC_LIST_1)  + uiPart;
  m_apiMVPNum[0]=pcCU->getMVPNum(REF_PIC_LIST_0)  + uiPart;
  m_apiMVPNum[1]=pcCU->getMVPNum(REF_PIC_LIST_1)  + uiPart;
  
#if E057_INTRA_PCM
  m_pbIPCMFlag         = pcCU->getIPCMFlag()        + uiPart;
#endif

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  UInt uiTmp = uiWidth*uiHeight;
  UInt uiMaxCuWidth=pcCU->getSlice()->getSPS()->getMaxCUWidth();
  UInt uiMaxCuHeight=pcCU->getSlice()->getSPS()->getMaxCUHeight();
  
  UInt uiCoffOffset = uiMaxCuWidth*uiMaxCuHeight*uiAbsPartIdx/pcCU->getPic()->getNumPartInCU();
  
  m_pcTrCoeffY = pcCU->getCoeffY() + uiCoffOffset;
#if E057_INTRA_PCM
  m_pcIPCMSampleY = pcCU->getPCMSampleY() + uiCoffOffset;
#endif

  uiTmp >>= 2;
  uiCoffOffset >>=2;
  m_pcTrCoeffCb=pcCU->getCoeffCb() + uiCoffOffset;
  m_pcTrCoeffCr=pcCU->getCoeffCr() + uiCoffOffset;
#if E057_INTRA_PCM
  m_pcIPCMSampleCb = pcCU->getPCMSampleCb() + uiCoffOffset;
  m_pcIPCMSampleCr = pcCU->getPCMSampleCr() + uiCoffOffset;
#endif

  m_acCUMvField[0].linkToWithOffset( pcCU->getCUMvField(REF_PIC_LIST_0), uiPart );
  m_acCUMvField[1].linkToWithOffset( pcCU->getCUMvField(REF_PIC_LIST_1), uiPart );
#if FINE_GRANULARITY_SLICES
  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiPart,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiPart,sizeof(UInt)*m_uiNumPartition);
#else
  m_uiSliceStartCU        = pcCU->getSliceStartCU();
  m_uiEntropySliceStartCU = pcCU->getEntropySliceStartCU();
#endif
}

// Copy inter prediction info from the biggest CU
Void TComDataCU::copyInterPredInfoFrom    ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList )
{
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  Int iRastPartIdx     = g_auiZscanToRaster[uiAbsPartIdx];
  m_uiCUPelX           = pcCU->getCUPelX() + m_pcPic->getMinCUWidth ()*( iRastPartIdx % m_pcPic->getNumPartInWidth() );
  m_uiCUPelY           = pcCU->getCUPelY() + m_pcPic->getMinCUHeight()*( iRastPartIdx / m_pcPic->getNumPartInWidth() );
  
  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0]  = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1]  = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_pePartSize         = pcCU->getPartitionSize ()        + uiAbsPartIdx;
  m_pePredMode         = pcCU->getPredictionMode()        + uiAbsPartIdx;
  m_puhInterDir        = pcCU->getInterDir      ()        + uiAbsPartIdx;
  
  m_puhDepth           = pcCU->getDepth ()                + uiAbsPartIdx;
  m_puhWidth           = pcCU->getWidth ()                + uiAbsPartIdx;
  m_puhHeight          = pcCU->getHeight()                + uiAbsPartIdx;
  
  m_pbMergeFlag        = pcCU->getMergeFlag()             + uiAbsPartIdx;
  m_puhMergeIndex      = pcCU->getMergeIndex()            + uiAbsPartIdx;
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui ++ )
  {
    m_apuhNeighbourCandIdx[ui] = pcCU->getNeighbourCandIdx( ui ) + uiAbsPartIdx;
  }

  m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
  m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;
  
  m_acCUMvField[ eRefPicList ].linkToWithOffset( pcCU->getCUMvField(eRefPicList), uiAbsPartIdx );

#if FINE_GRANULARITY_SLICES
  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiAbsPartIdx,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiAbsPartIdx,sizeof(UInt)*m_uiNumPartition);
#else
  m_uiSliceStartCU        = pcCU->getSliceStartCU();
  m_uiEntropySliceStartCU = pcCU->getEntropySliceStartCU();
#endif
}

// Copy small CU to bigger CU.
// One of quarter parts overwritten by predicted sub part.
Void TComDataCU::copyPartFrom( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
{
  assert( uiPartUnitIdx<4 );
  
  m_dTotalCost         += pcCU->getTotalCost();
  m_uiTotalDistortion  += pcCU->getTotalDistortion();
  m_uiTotalBits        += pcCU->getTotalBits();
  
  UInt uiOffset         = pcCU->getTotalNumPart()*uiPartUnitIdx;
  
  UInt uiNumPartition = pcCU->getTotalNumPart();
  Int iSizeInUchar  = sizeof( UChar ) * uiNumPartition;
  Int iSizeInBool   = sizeof( Bool  ) * uiNumPartition;
  
  memcpy( m_phQP       + uiOffset, pcCU->getQP(),             iSizeInUchar                        );
  memcpy( m_pePartSize + uiOffset, pcCU->getPartitionSize(),  sizeof( *m_pePartSize ) * uiNumPartition );
  memcpy( m_pePredMode + uiOffset, pcCU->getPredictionMode(), sizeof( *m_pePredMode ) * uiNumPartition );
  
  memcpy( m_puiAlfCtrlFlag      + uiOffset, pcCU->getAlfCtrlFlag(),       iSizeInBool  );
  memcpy( m_pbMergeFlag         + uiOffset, pcCU->getMergeFlag(),         iSizeInBool  );
  memcpy( m_puhMergeIndex       + uiOffset, pcCU->getMergeIndex(),        iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memcpy( m_apuhNeighbourCandIdx[ ui ] + uiOffset, pcCU->getNeighbourCandIdx( ui ), iSizeInUchar );
  }
  memcpy( m_puhLumaIntraDir     + uiOffset, pcCU->getLumaIntraDir(),      iSizeInUchar );
  memcpy( m_puhChromaIntraDir   + uiOffset, pcCU->getChromaIntraDir(),    iSizeInUchar );
  memcpy( m_puhInterDir         + uiOffset, pcCU->getInterDir(),          iSizeInUchar );
  memcpy( m_puhTrIdx            + uiOffset, pcCU->getTransformIdx(),      iSizeInUchar );
  
  memcpy( m_puhCbf[0] + uiOffset, pcCU->getCbf(TEXT_LUMA)    , iSizeInUchar );
  memcpy( m_puhCbf[1] + uiOffset, pcCU->getCbf(TEXT_CHROMA_U), iSizeInUchar );
  memcpy( m_puhCbf[2] + uiOffset, pcCU->getCbf(TEXT_CHROMA_V), iSizeInUchar );
  
  memcpy( m_puhDepth  + uiOffset, pcCU->getDepth(),  iSizeInUchar );
  memcpy( m_puhWidth  + uiOffset, pcCU->getWidth(),  iSizeInUchar );
  memcpy( m_puhHeight + uiOffset, pcCU->getHeight(), iSizeInUchar );
  
  memcpy( m_apiMVPIdx[0] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_0), iSizeInUchar );
  memcpy( m_apiMVPIdx[1] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_1), iSizeInUchar );
  memcpy( m_apiMVPNum[0] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_0), iSizeInUchar );
  memcpy( m_apiMVPNum[1] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_1), iSizeInUchar );
  
#if E057_INTRA_PCM
  memcpy( m_pbIPCMFlag + uiOffset, pcCU->getIPCMFlag(), iSizeInBool );
#endif

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_acCUMvField[0].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_0 ), pcCU->getTotalNumPart(), uiOffset );
  m_acCUMvField[1].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_1 ), pcCU->getTotalNumPart(), uiOffset );
  
  UInt uiTmp  = g_uiMaxCUWidth*g_uiMaxCUHeight >> (uiDepth<<1);
  UInt uiTmp2 = uiPartUnitIdx*uiTmp;
  memcpy( m_pcTrCoeffY  + uiTmp2, pcCU->getCoeffY(),  sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memcpy( m_pcIPCMSampleY + uiTmp2 , pcCU->getPCMSampleY(), sizeof(Pel) * uiTmp );
#endif

  uiTmp >>= 2; uiTmp2>>= 2;
  memcpy( m_pcTrCoeffCb + uiTmp2, pcCU->getCoeffCb(), sizeof(TCoeff)*uiTmp );
  memcpy( m_pcTrCoeffCr + uiTmp2, pcCU->getCoeffCr(), sizeof(TCoeff)*uiTmp );
#if E057_INTRA_PCM
  memcpy( m_pcIPCMSampleCb + uiTmp2 , pcCU->getPCMSampleCb(), sizeof(Pel) * uiTmp );
  memcpy( m_pcIPCMSampleCr + uiTmp2 , pcCU->getPCMSampleCr(), sizeof(Pel) * uiTmp );
#endif
#if FINE_GRANULARITY_SLICES
  m_uiTotalBins += pcCU->getTotalBins();
  memcpy( m_uiSliceStartCU        + uiOffset, pcCU->m_uiSliceStartCU,        sizeof( UInt ) * uiNumPartition  );
  memcpy( m_uiEntropySliceStartCU + uiOffset, pcCU->m_uiEntropySliceStartCU, sizeof( UInt ) * uiNumPartition  );
#else
  m_uiSliceStartCU        = pcCU->getSliceStartCU();
  m_uiEntropySliceStartCU = pcCU->getEntropySliceStartCU();
#endif
}

// Copy current predicted part to a CU in picture.
// It is used to predict for next part
Void TComDataCU::copyToPic( UChar uhDepth )
{
  TComDataCU*& rpcCU = m_pcPic->getCU( m_uiCUAddr );
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;
  
  Int iSizeInUchar  = sizeof( UChar ) * m_uiNumPartition;
  Int iSizeInBool   = sizeof( Bool  ) * m_uiNumPartition;
  
  memcpy( rpcCU->getQP() + m_uiAbsIdxInLCU, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + m_uiAbsIdxInLCU, m_pePartSize, sizeof( *m_pePartSize ) * m_uiNumPartition );
  memcpy( rpcCU->getPredictionMode() + m_uiAbsIdxInLCU, m_pePredMode, sizeof( *m_pePredMode ) * m_uiNumPartition );
  
  memcpy( rpcCU->getAlfCtrlFlag()    + m_uiAbsIdxInLCU, m_puiAlfCtrlFlag,    iSizeInBool  );
  
  memcpy( rpcCU->getMergeFlag()         + m_uiAbsIdxInLCU, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + m_uiAbsIdxInLCU, m_puhMergeIndex,       iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memcpy( rpcCU->getNeighbourCandIdx( ui ) + m_uiAbsIdxInLCU, m_apuhNeighbourCandIdx[ui], iSizeInUchar );
  }
  memcpy( rpcCU->getLumaIntraDir()      + m_uiAbsIdxInLCU, m_puhLumaIntraDir,     iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir()    + m_uiAbsIdxInLCU, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + m_uiAbsIdxInLCU, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + m_uiAbsIdxInLCU, m_puhTrIdx,            iSizeInUchar );
  
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + m_uiAbsIdxInLCU, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + m_uiAbsIdxInLCU, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + m_uiAbsIdxInLCU, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()  + m_uiAbsIdxInLCU, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + m_uiAbsIdxInLCU, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + m_uiAbsIdxInLCU, m_puhHeight, iSizeInUchar );
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPIdx[0], iSizeInUchar );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPIdx[1], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPNum[0], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPNum[1], iSizeInUchar );
  
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU );
  
#if E057_INTRA_PCM
  memcpy( rpcCU->getIPCMFlag() + m_uiAbsIdxInLCU, m_pbIPCMFlag,         iSizeInBool  );
#endif

  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>(uhDepth<<1);
  UInt uiTmp2 = m_uiAbsIdxInLCU*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
#if E057_INTRA_PCM
  memcpy( rpcCU->getPCMSampleY() + uiTmp2 , m_pcIPCMSampleY, sizeof(Pel)*uiTmp );
#endif

  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
#if E057_INTRA_PCM
  memcpy( rpcCU->getPCMSampleCb() + uiTmp2 , m_pcIPCMSampleCb, sizeof( Pel ) * uiTmp );
  memcpy( rpcCU->getPCMSampleCr() + uiTmp2 , m_pcIPCMSampleCr, sizeof( Pel ) * uiTmp );
#endif
#if FINE_GRANULARITY_SLICES
  rpcCU->getTotalBins() = m_uiTotalBins;
  memcpy( rpcCU->m_uiSliceStartCU        + m_uiAbsIdxInLCU, m_uiSliceStartCU,        sizeof( UInt ) * m_uiNumPartition  );
  memcpy( rpcCU->m_uiEntropySliceStartCU + m_uiAbsIdxInLCU, m_uiEntropySliceStartCU, sizeof( UInt ) * m_uiNumPartition  );
#else
  rpcCU->setSliceStartCU( m_uiSliceStartCU );
  rpcCU->setEntropySliceStartCU( m_uiEntropySliceStartCU );
#endif
}

Void TComDataCU::copyToPic( UChar uhDepth, UInt uiPartIdx, UInt uiPartDepth )
{
  TComDataCU*&  rpcCU       = m_pcPic->getCU( m_uiCUAddr );
  UInt          uiQNumPart  = m_uiNumPartition>>(uiPartDepth<<1);
  
  UInt uiPartStart          = uiPartIdx*uiQNumPart;
  UInt uiPartOffset         = m_uiAbsIdxInLCU + uiPartStart;
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;
  
  Int iSizeInUchar  = sizeof( UChar  ) * uiQNumPart;
  Int iSizeInBool   = sizeof( Bool   ) * uiQNumPart;
  
  memcpy( rpcCU->getQP() + uiPartOffset, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + uiPartOffset, m_pePartSize, sizeof( *m_pePartSize ) * uiQNumPart );
  memcpy( rpcCU->getPredictionMode() + uiPartOffset, m_pePredMode, sizeof( *m_pePredMode ) * uiQNumPart );
  
  memcpy( rpcCU->getAlfCtrlFlag()       + uiPartOffset, m_puiAlfCtrlFlag,      iSizeInBool  );
  memcpy( rpcCU->getMergeFlag()         + uiPartOffset, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + uiPartOffset, m_puhMergeIndex,       iSizeInUchar );
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ui++ )
  {
    memcpy( rpcCU->getNeighbourCandIdx( ui ) + uiPartOffset, m_apuhNeighbourCandIdx[ui], iSizeInUchar );
  }
  memcpy( rpcCU->getLumaIntraDir()      + uiPartOffset, m_puhLumaIntraDir,     iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir()    + uiPartOffset, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + uiPartOffset, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + uiPartOffset, m_puhTrIdx,            iSizeInUchar );
  
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + uiPartOffset, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + uiPartOffset, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + uiPartOffset, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()  + uiPartOffset, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + uiPartOffset, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + uiPartOffset, m_puhHeight, iSizeInUchar );
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPIdx[0], iSizeInUchar );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPIdx[1], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPNum[0], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPNum[1], iSizeInUchar );
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  
#if E057_INTRA_PCM
  memcpy( rpcCU->getIPCMFlag() + uiPartOffset, m_pbIPCMFlag,         iSizeInBool  );
#endif

  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>((uhDepth+uiPartDepth)<<1);
  UInt uiTmp2 = uiPartOffset*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
 
#if E057_INTRA_PCM
  memcpy( rpcCU->getPCMSampleY() + uiTmp2 , m_pcIPCMSampleY, sizeof( Pel ) * uiTmp );
#endif

  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );

#if E057_INTRA_PCM
  memcpy( rpcCU->getPCMSampleCb() + uiTmp2 , m_pcIPCMSampleCb, sizeof( Pel ) * uiTmp );
  memcpy( rpcCU->getPCMSampleCr() + uiTmp2 , m_pcIPCMSampleCr, sizeof( Pel ) * uiTmp );
#endif
#if FINE_GRANULARITY_SLICES
  rpcCU->getTotalBins() = m_uiTotalBins;
  memcpy( rpcCU->m_uiSliceStartCU        + uiPartOffset, m_uiSliceStartCU,        sizeof( UInt ) * uiQNumPart  );
  memcpy( rpcCU->m_uiEntropySliceStartCU + uiPartOffset, m_uiEntropySliceStartCU, sizeof( UInt ) * uiQNumPart  );
#else
  rpcCU->setSliceStartCU( m_uiSliceStartCU );
  rpcCU->setEntropySliceStartCU( m_uiEntropySliceStartCU );
#endif
}

// --------------------------------------------------------------------------------------------------------------------
// Other public functions
// --------------------------------------------------------------------------------------------------------------------

TComDataCU* TComDataCU::getPULeft( UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroCol( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    if ( RasterAddress::isEqualCol( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
    {
#if FINE_GRANULARITY_SLICES 
      TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
      if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
        ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
      {
        return NULL;
      }
#endif
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiLPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES 
      if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
        ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
      {
        return NULL;
      }
#endif
      return this;
    }
  }
  
  uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + uiNumPartInCUWidth - 1 ];

#if FINE_GRANULARITY_SLICES
  if ((bEnforceSliceRestriction        && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
    ||(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }
  return m_pcCULeft;
}

TComDataCU* TComDataCU::getPUAbove( UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth ];
    if ( RasterAddress::isEqualRow( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
    {
#if FINE_GRANULARITY_SLICES 
      TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
      if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
        ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
      {
        return NULL;
      }
#endif
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiAPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES 
      if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
        ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
      {
        return NULL;
      }
#endif
      return this;
    }
  }
  
  uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];

#if FINE_GRANULARITY_SLICES
  if ((bEnforceSliceRestriction        && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
    ||(bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
  if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
       (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }
  return m_pcCUAbove;
}

TComDataCU* TComDataCU::getPUAboveLeft( UInt& uiALPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroCol( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
    {
      uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth - 1 ];
      if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
      {
#if FINE_GRANULARITY_SLICES 
        TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
      if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
        ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
        {
          return NULL;
        }
#endif
        return m_pcPic->getCU( getAddr() );
      }
      else
      {
        uiALPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES 
        if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
          ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
        {
          return NULL;
        }
#endif
        return this;
      }
    }
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + getPic()->getNumPartInCU() - uiNumPartInCUWidth - 1 ];
#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction        && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction        && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiALPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - 1 ];
#if FINE_GRANULARITY_SLICES
  if ((bEnforceSliceRestriction        && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL || m_pcCUAboveLeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
    ||(bEnforceEntropySliceRestriction && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL || m_pcCUAboveLeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
  if ( (bEnforceSliceRestriction && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL || m_pcCUAboveLeft->getAddr() < m_uiSliceStartCU)) ||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL|| m_pcCUAboveLeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }
  return m_pcCUAboveLeft;
}

TComDataCU* TComDataCU::getPUAboveRight( UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanCol( uiAbsPartIdxRT, uiNumPartInCUWidth - 1, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxRT, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
        {
#if FINE_GRANULARITY_SLICES 
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
#endif
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
#endif
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];

#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction        && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
#if FINE_GRANULARITY_SLICES
  if ((bEnforceSliceRestriction        && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
    ||(bEnforceEntropySliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
  if ( (bEnforceSliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getAddr() < m_uiSliceStartCU)) ||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }
  return m_pcCUAboveRight;
}

TComDataCU* TComDataCU::getPUBelowLeft( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanRow( uiAbsPartIdxLB, m_pcPic->getNumPartInHeight() - 1, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroCol( uiAbsPartIdxLB, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxLB, uiAbsZorderCUIdxLB, uiNumPartInCUWidth ) )
        {
#if FINE_GRANULARITY_SLICES
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
#endif
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
#endif
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction        && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

TComDataCU* TComDataCU::getPUBelowLeftAdi(UInt& uiBLPartUnitIdx, UInt uiPuHeight,  UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ((m_puhHeight[0] / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + uiPuHeight ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanRow( uiAbsPartIdxLB, m_pcPic->getNumPartInHeight() - uiPartUnitOffset, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroCol( uiAbsPartIdxLB, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiPartUnitOffset * uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiPartUnitOffset * uiNumPartInCUWidth - 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxLB, uiAbsZorderCUIdxLB, uiNumPartInCUWidth ) )
        {
#if FINE_GRANULARITY_SLICES
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
#endif
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
#endif
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + (1+uiPartUnitOffset) * uiNumPartInCUWidth - 1 ];
#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction        && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

TComDataCU* TComDataCU::getPUAboveRightAdi(UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhWidth[0] / m_pcPic->getMinCUWidth()) - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + uiPuWidth ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanCol( uiAbsPartIdxRT, uiNumPartInCUWidth - uiPartUnitOffset, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + uiPartUnitOffset ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + uiPartUnitOffset ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxRT, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
        {
#if FINE_GRANULARITY_SLICES
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
           ||( bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))

          {
            return NULL;
          }
#endif
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
#if FINE_GRANULARITY_SLICES
          if ((bEnforceSliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
#endif
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + uiPartUnitOffset ];
#if FINE_GRANULARITY_SLICES
    if ((bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
      ||(bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
#else
    if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiSliceStartCU)) ||
         (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getAddr() < m_uiEntropySliceStartCU)) )
#endif
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + uiPartUnitOffset-1 ];
#if FINE_GRANULARITY_SLICES
  if (bEnforceSliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
#else
  if ( (bEnforceSliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getAddr() < m_uiSliceStartCU)) ||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || m_pcCUAboveRight->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }
  return m_pcCUAboveRight;
}

#if SUB_LCU_DQP
/** Get left QpMinCu
*\param   uiLPartUnitIdx
*\param   uiCurrAbsIdxInLCU
*\param   bEnforceSliceRestriction
*\param   bEnforceEntropySliceRestriction
*\returns TComDataCU*   point of TComDataCU of left QpMinCu
*/
TComDataCU* TComDataCU::getQpMinCuLeft( UInt& uiLPartUnitIdx, UInt uiCurrAbsIdxInLCU, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction)
{
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  UInt uiAbsRorderQpMinCUIdx   = g_auiZscanToRaster[(uiCurrAbsIdxInLCU>>(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1))];
  UInt uiNumPartInQpMinCUWidth = getSlice()->getPPS()->getMinCuDQPSize()>>2;

#if FINE_GRANULARITY_SLICES 
  
  UInt uiAbsZorderQpMinCUIdx   = (uiCurrAbsIdxInLCU>>(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1));
  if( (uiCurrAbsIdxInLCU != uiAbsZorderQpMinCUIdx) && 
    ((bEnforceSliceRestriction && (m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrAbsIdxInLCU) != m_pcPic->getCU( getAddr() )->getSliceStartCU (uiAbsZorderQpMinCUIdx))) ||
    (bEnforceEntropySliceRestriction &&(m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrAbsIdxInLCU) != m_pcPic->getCU( getAddr() )->getEntropySliceStartCU (uiAbsZorderQpMinCUIdx)) )) )
  {
    return NULL;
  }
#endif

  if ( !RasterAddress::isZeroCol( uiAbsRorderQpMinCUIdx, uiNumPartInCUWidth ) )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsRorderQpMinCUIdx - uiNumPartInQpMinCUWidth ];
#if FINE_GRANULARITY_SLICES 
    TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
    if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiAbsZorderQpMinCUIdx)))
      ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiAbsZorderQpMinCUIdx))))
    {
      return NULL;
    }
#endif
    return m_pcPic->getCU( getAddr() );
  }

  uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsRorderQpMinCUIdx + uiNumPartInCUWidth - uiNumPartInQpMinCUWidth ];

#if FINE_GRANULARITY_SLICES
  if ((bEnforceSliceRestriction        && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiAbsZorderQpMinCUIdx)))
    ||(bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiAbsZorderQpMinCUIdx))))
#else
  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiSliceStartCU)) ||
    (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getAddr() < m_uiEntropySliceStartCU)) )
#endif
  {
    return NULL;
  }

  return m_pcCULeft;
}

/** Get reference QP from left QpMinCu or latest coded QP
*\param   uiCurrAbsIdxInLCU
*\returns UChar   reference QP value
*/
UChar TComDataCU::getRefQP( UInt uiCurrAbsIdxInLCU )
{
  // Left CU
  TComDataCU* pcCULeft;
  UInt        uiLPartIdx;
  pcCULeft = getQpMinCuLeft( uiLPartIdx, m_uiAbsIdxInLCU + uiCurrAbsIdxInLCU );
  if ( pcCULeft )
  {
    return pcCULeft->getQP(uiLPartIdx);
  }
  // Last QP
  return getLastCodedQP( uiCurrAbsIdxInLCU );
}

Int TComDataCU::getLastValidPartIdx( Int iAbsPartIdx )
{
  Int iLastValidPartIdx = iAbsPartIdx-1;
  while ( iLastValidPartIdx >= 0
       && getPredictionMode( iLastValidPartIdx ) == MODE_NONE )
  {
    UInt uiDepth = getDepth( iLastValidPartIdx );
    iLastValidPartIdx -= m_uiNumPartition>>(uiDepth<<1);
  }
  return iLastValidPartIdx;
}

UChar TComDataCU::getLastCodedQP( UInt uiAbsPartIdx )
{
  UInt uiQUPartIdxMask = ~((1<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))-1);
  Int iLastValidPartIdx = getLastValidPartIdx( uiAbsPartIdx&uiQUPartIdxMask );
#if FINE_GRANULARITY_SLICES
  if ( uiAbsPartIdx < m_uiNumPartition
    && (getSCUAddr()+iLastValidPartIdx < getSliceStartCU(m_uiAbsIdxInLCU+uiAbsPartIdx) || getSCUAddr()+iLastValidPartIdx < getEntropySliceStartCU(m_uiAbsIdxInLCU+uiAbsPartIdx) ))
  {
    return getSlice()->getSliceQp();
  }
  else
#endif
  if ( iLastValidPartIdx >= 0 )
  {
    return getQP( iLastValidPartIdx );
  }
  else
  {
    if ( getZorderIdxInCU() > 0 )
    {
      return getPic()->getCU( getAddr() )->getLastCodedQP( getZorderIdxInCU() );
    }
    else if ( getAddr() > 0 )
    {
      return getPic()->getCU( getAddr()-1 )->getLastCodedQP( getPic()->getNumPartInCU() );
    }
    else
    {
      return getSlice()->getSliceQp();
    }
  }
}
#endif

Int TComDataCU::getMostProbableIntraDirLuma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir, iMostProbable;
  
  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, true, false );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iLeftIntraDir );
#endif
  
  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, true, false );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iAboveIntraDir );
#endif
  
  iMostProbable  = min( iLeftIntraDir, iAboveIntraDir );
  
  // Mode conversion process for blocks with different number of available prediction directions
  Int iIdx  = getIntraSizeIdx(uiAbsPartIdx);
  
  if ( iMostProbable >= g_aucIntraModeNumAng[iIdx] )
  {
    if ( g_aucIntraModeNumAng[iIdx] == 5 )
      iMostProbable = g_aucAngModeMapping[0][g_aucAngIntraModeOrder[iMostProbable]];
    if ( g_aucIntraModeNumAng[iIdx] == 3 )
      iMostProbable = g_aucAngModeMapping[3][g_aucAngIntraModeOrder[iMostProbable]];
    else
      iMostProbable = g_aucAngModeMapping[1][g_aucAngIntraModeOrder[iMostProbable]]; 
  } 
  
  return ( NOT_VALID == iMostProbable ) ? 2 : iMostProbable;
}
#if MTK_DCM_MPM
Int TComDataCU::getIntraDirLumaPredictor( UInt uiAbsPartIdx, Int uiIntraDirPred[] )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir;
  Int         uiPredNum = 0;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : 2;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iLeftIntraDir );
#endif

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : 2;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iAboveIntraDir );
#endif

  Int iIdx  = getIntraSizeIdx(uiAbsPartIdx);

   
  if ( iLeftIntraDir >= g_aucIntraModeNumAng[iIdx] ) 
  {
   if ( g_aucIntraModeNumAng[iIdx] == 5 )
   {
      iLeftIntraDir = g_aucAngModeMapping[0][g_aucAngIntraModeOrder[iLeftIntraDir]];
   }
   if ( g_aucIntraModeNumAng[iIdx] == 3 )
   {
      iLeftIntraDir = g_aucAngModeMapping[3][g_aucAngIntraModeOrder[iLeftIntraDir]];
   }
   else
   {
      iLeftIntraDir = g_aucAngModeMapping[1][g_aucAngIntraModeOrder[iLeftIntraDir]]; 
   }
  }
   
   
 if ( iAboveIntraDir >= g_aucIntraModeNumAng[iIdx] ) 
 {
   if ( g_aucIntraModeNumAng[iIdx] == 5 )
   {
      iAboveIntraDir = g_aucAngModeMapping[0][g_aucAngIntraModeOrder[iAboveIntraDir]];
   }
   if ( g_aucIntraModeNumAng[iIdx] == 3 )
   {
      iAboveIntraDir = g_aucAngModeMapping[3][g_aucAngIntraModeOrder[iAboveIntraDir]];
   }
   else
   {
      iAboveIntraDir = g_aucAngModeMapping[1][g_aucAngIntraModeOrder[iAboveIntraDir]]; 
   }
 }
   
 if(iLeftIntraDir == iAboveIntraDir)
 {
   uiPredNum = 1;
   uiIntraDirPred[0] = iLeftIntraDir ;
 }
 else
 {
   uiPredNum = 2;
   uiIntraDirPred[0] = min(iLeftIntraDir, iAboveIntraDir);
   uiIntraDirPred[1] = max(iLeftIntraDir, iAboveIntraDir);
 }


  return uiPredNum;
}
#endif

Int TComDataCU::getLeftIntraDirLuma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iLeftIntraDir );
#endif

  return iLeftIntraDir;
}

Int TComDataCU::getAboveIntraDirLuma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iAboveIntraDir;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
#if ADD_PLANAR_MODE
  mapPlanartoDC( iAboveIntraDir );
#endif

  return iAboveIntraDir;
}

UInt TComDataCU::getCtxSplitFlag( UInt uiAbsPartIdx, UInt uiDepth )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;
  // Get left split flag
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxIntraDirChroma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;
  
  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getChromaIntraDir( uiTempPartIdx ) == 4 ) ? 1 : 0 ) : 0;
  
  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getChromaIntraDir( uiTempPartIdx ) == 4 ) ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxQtCbf( UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  if( getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && eType != TEXT_LUMA )
  {
    return uiTrDepth;
  }
  UInt uiCtx = 0;
  const UInt uiDepth = getDepth( uiAbsPartIdx );
  const UInt uiLog2TrafoSize = g_aucConvertToBit[getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth - uiTrDepth;
  
  if( uiTrDepth == 0 || uiLog2TrafoSize == getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    TComDataCU* pcTempCU;
    UInt        uiTempPartIdx, uiTempTrDepth;
    
    // Get Cbf of left PU
    pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      uiTempTrDepth = pcTempCU->getTransformIdx( uiTempPartIdx );
#if E057_INTRA_PCM
      if(pcTempCU->getIPCMFlag(uiTempPartIdx) == true)
        uiCtx = 1;
      else
#endif
      uiCtx = pcTempCU->getCbf( uiTempPartIdx, eType, uiTempTrDepth );
    }
    
    // Get Cbf of above PU
    pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      uiTempTrDepth = pcTempCU->getTransformIdx( uiTempPartIdx );
#if E057_INTRA_PCM
      if(pcTempCU->getIPCMFlag(uiTempPartIdx) == true)
        uiCtx += 2;
      else
#endif
      uiCtx += pcTempCU->getCbf( uiTempPartIdx, eType, uiTempTrDepth ) << 1;
    }
    uiCtx++;
  }
  return uiCtx;
}

UInt TComDataCU::getCtxQtRootCbf( UInt uiAbsPartIdx )
{
  UInt uiCtx = 0;
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  
  // Get RootCbf of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
#if E057_INTRA_PCM
    if(pcTempCU->getIPCMFlag(uiTempPartIdx) == true)
      uiCtx = 1;
    else
#endif
    uiCtx = pcTempCU->getQtRootCbf( uiTempPartIdx );
  }
  
  // Get RootCbf of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
#if E057_INTRA_PCM
    if(pcTempCU->getIPCMFlag(uiTempPartIdx) == true)
      uiCtx += 2;
    else
#endif
    uiCtx += pcTempCU->getQtRootCbf( uiTempPartIdx ) << 1;
  }
  
  return uiCtx;
}

UInt TComDataCU::getQuadtreeTULog2MinSizeInCU( UInt uiIdx )
{
  UInt uiLog2MinTUSizeInCU = g_aucConvertToBit[getWidth( uiIdx )] + 2;
  
  if ( getPredictionMode( uiIdx ) == MODE_INTRA && getPartitionSize( uiIdx ) == SIZE_NxN )
  {
    uiLog2MinTUSizeInCU--;
  }
 
  UInt uiQuadtreeTUMaxDepth = getPredictionMode( uiIdx ) == MODE_INTRA ? m_pcSlice->getSPS()->getQuadtreeTUMaxDepthIntra() : m_pcSlice->getSPS()->getQuadtreeTUMaxDepthInter();

#ifdef MOT_TUPU_MAXDEPTH1
  PartSize  partSize  = getPartitionSize( uiIdx );
  Int   SplitFlag = ((uiQuadtreeTUMaxDepth == 1) && (getPredictionMode( uiIdx ) == MODE_INTER) && (partSize == SIZE_Nx2N || partSize == SIZE_2NxN) );

  if (uiLog2MinTUSizeInCU < m_pcSlice->getSPS()->getQuadtreeTULog2MinSize() + uiQuadtreeTUMaxDepth - 1 + SplitFlag)
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MinSize();  
  }
  else
  {
    uiLog2MinTUSizeInCU -= uiQuadtreeTUMaxDepth - 1 + SplitFlag;  
  }
  
  if ( uiLog2MinTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize())
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() - SplitFlag;
  }  
#else
  if (uiLog2MinTUSizeInCU < m_pcSlice->getSPS()->getQuadtreeTULog2MinSize() + uiQuadtreeTUMaxDepth - 1)
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MinSize();  
  }
  else
  {
    uiLog2MinTUSizeInCU -= uiQuadtreeTUMaxDepth - 1;  
  }
  
  if ( uiLog2MinTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize())
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize();
  }  
#endif

  return uiLog2MinTUSizeInCU;
}

UInt TComDataCU::getCtxAlfCtrlFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->getAlfCtrlFlag( uiTempPartIdx ) : 0;
  
  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->getAlfCtrlFlag( uiTempPartIdx ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxSkipFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;
  
  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;
  
  return uiCtx;
}

/** CABAC context derivation for merge flag
 * \param uiAbsPartIdx
 * \returns context offset
 */
UInt TComDataCU::getCtxMergeFlag( UInt uiAbsPartIdx )
{
  UInt uiCtx = 0;
#if CHANGE_MERGE_CONTEXT
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;

  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->getMergeFlag( uiTempPartIdx ) : 0;

  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->getMergeFlag( uiTempPartIdx ) : 0;
#endif
  return uiCtx;
}

UInt TComDataCU::getCtxInterDir( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getInterDir( uiTempPartIdx ) % 3 ) ? 0 : 1 ) : 0;
  
  // Get BCBP of Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getInterDir( uiTempPartIdx ) % 3 ) ? 0 : 1 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxRefIdx( UInt uiAbsPartIdx, RefPicList eRefPicList )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  TComMvField cMvFieldTemp;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  getMvField( pcTempCU, uiTempPartIdx, eRefPicList, cMvFieldTemp );
  uiCtx += cMvFieldTemp.getRefIdx() > 0 ? 1 : 0;
  
  // Get BCBP of Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  getMvField( pcTempCU, uiTempPartIdx, eRefPicList, cMvFieldTemp );
  uiCtx += cMvFieldTemp.getRefIdx() > 0 ? 2 : 0;
  
  return uiCtx;
}

Void TComDataCU::setCbfSubParts( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[0] + uiAbsPartIdx, uiCbfY, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[1] + uiAbsPartIdx, uiCbfU, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[2] + uiAbsPartIdx, uiCbfV, sizeof( UChar ) * uiCurrPartNumb );
}

Void TComDataCU::setCbfSubParts( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[g_aucConvertTxtTypeToIdx[eTType]] + uiAbsPartIdx, uiCbf, sizeof( UChar ) * uiCurrPartNumb );
}

#if HHI_MRG_SKIP
/** Sets a coded block flag for all sub-partitions of a partition
 * \param uiCbf The value of the coded block flag to be set
 * \param eTType
 * \param uiAbsPartIdx
 * \param uiPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TComDataCU::setCbfSubParts ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiCbf, m_puhCbf[g_aucConvertTxtTypeToIdx[eTType]], uiAbsPartIdx, uiDepth, uiPartIdx );
}
#endif

Void TComDataCU::setDepthSubParts( UInt uiDepth, UInt uiAbsPartIdx )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhDepth + uiAbsPartIdx, uiDepth, sizeof(UChar)*uiCurrPartNumb );
}

Bool TComDataCU::isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  return (((m_uiAbsIdxInLCU + uiAbsPartIdx)% uiPartNumb) == 0);
}

Void TComDataCU::setAlfCtrlFlagSubParts( Bool uiFlag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> ( 2 * uiDepth );
  memset( m_puiAlfCtrlFlag + uiAbsPartIdx, uiFlag, sizeof( *m_puiAlfCtrlFlag ) * uiCurrPartNumb );
}

Void TComDataCU::createTmpAlfCtrlFlag()
{
  m_puiTmpAlfCtrlFlag = new Bool[ m_uiNumPartition ];
}

Void TComDataCU::destroyTmpAlfCtrlFlag()
{
  if(m_puiTmpAlfCtrlFlag)
  {
    delete[] m_puiTmpAlfCtrlFlag;
    m_puiTmpAlfCtrlFlag = NULL;
  }
}

Void TComDataCU::copyAlfCtrlFlagToTmp()
{
  memcpy( m_puiTmpAlfCtrlFlag, m_puiAlfCtrlFlag, sizeof( *m_puiAlfCtrlFlag )*m_uiNumPartition );
}

Void TComDataCU::copyAlfCtrlFlagFromTmp()
{
  memcpy( m_puiAlfCtrlFlag, m_puiTmpAlfCtrlFlag, sizeof( *m_puiAlfCtrlFlag )*m_uiNumPartition );
}

Void TComDataCU::setPartSizeSubParts( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert( sizeof( *m_pePartSize) == 1 );
  memset( m_pePartSize + uiAbsPartIdx, eMode, m_pcPic->getNumPartInCU() >> ( 2 * uiDepth ) );
}

Void TComDataCU::setPredModeSubParts( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert( sizeof( *m_pePartSize) == 1 );
  memset( m_pePredMode + uiAbsPartIdx, eMode, m_pcPic->getNumPartInCU() >> ( 2 * uiDepth ) );
}

Void TComDataCU::setQPSubParts( UInt uiQP, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
#if FINE_GRANULARITY_SLICES
  TComSlice * pcSlice = getPic()->getSlice(getPic()->getCurrSliceIdx());

  for(UInt uiSCUIdx = uiAbsPartIdx; uiSCUIdx < uiAbsPartIdx+uiCurrPartNumb; uiSCUIdx++)
  {
    if( m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiSCUIdx+getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr() )
    {
      m_phQP[uiSCUIdx] = uiQP;
    }
  }
#else
  memset( m_phQP + uiAbsPartIdx, uiQP, sizeof(Char)*uiCurrPartNumb );
#endif
}

Void TComDataCU::setLumaIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhLumaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

template<typename T>
Void TComDataCU::setSubPart( T uiParameter, T* puhBaseLCU, UInt uiCUAddr, UInt uiCUDepth, UInt uiPUIdx )
{
  assert( sizeof(T) == 1 ); // Using memset() works only for types of size 1
  
  UInt uiCurrPartNumQ = (m_pcPic->getNumPartInCU() >> (2 * uiCUDepth)) >> 2;
  switch ( m_pePartSize[ uiCUAddr ] )
  {
    case SIZE_2Nx2N:
      memset( puhBaseLCU + uiCUAddr, uiParameter, 4 * uiCurrPartNumQ );
      break;
    case SIZE_2NxN:
      memset( puhBaseLCU + uiCUAddr, uiParameter, 2 * uiCurrPartNumQ );
      break;
    case SIZE_Nx2N:
      memset( puhBaseLCU + uiCUAddr, uiParameter, uiCurrPartNumQ );
      memset( puhBaseLCU + uiCUAddr + 2 * uiCurrPartNumQ, uiParameter, uiCurrPartNumQ );
      break;
    case SIZE_NxN:
      memset( puhBaseLCU + uiCUAddr, uiParameter, uiCurrPartNumQ ); 
      break;
    default:
      assert( 0 );
  }
}

Void TComDataCU::setMergeFlagSubParts ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart( bMergeFlag, m_pbMergeFlag, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiMergeIndex, m_puhMergeIndex, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setChromIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhChromaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setInterDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiDir, m_puhInterDir, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMVPIdxSubParts( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<Char>( iMVPIdx, m_apiMVPIdx[eRefPicList], uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMVPNumSubParts( Int iMVPNum, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<Char>( iMVPNum, m_apiMVPNum[eRefPicList], uiAbsPartIdx, uiDepth, uiPartIdx );
}


Void TComDataCU::setTrIdxSubParts( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhTrIdx + uiAbsPartIdx, uiTrIdx, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setSizeSubParts( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhWidth  + uiAbsPartIdx, uiWidth,  sizeof(UChar)*uiCurrPartNumb );
  memset( m_puhHeight + uiAbsPartIdx, uiHeight, sizeof(UChar)*uiCurrPartNumb );
}

UChar TComDataCU::getNumPartInter()
{
  UChar iNumPart = 0;
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:    iNumPart = 1; break;
    case SIZE_2NxN:     iNumPart = 2; break;
    case SIZE_Nx2N:     iNumPart = 2; break;
    case SIZE_NxN:      iNumPart = 4; break;
    default:            assert (0);   break;
  }
  
  return  iNumPart;
}

Void TComDataCU::getPartIndexAndSize( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight )
{
  switch ( m_pePartSize[0] )
  {
    case SIZE_2NxN:
      riWidth = getWidth(0);      riHeight = getHeight(0) >> 1; ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0);      ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0) >> 1; ruiPartAddr = ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    default:
      assert ( m_pePartSize[0] == SIZE_2Nx2N );
      riWidth = getWidth(0);      riHeight = getHeight(0);      ruiPartAddr = 0;
      break;
  }
}


Void TComDataCU::getMvField ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList, TComMvField& rcMvField )
{
  if ( pcCU == NULL )  // OUT OF BOUNDARY
  {
    TComMv  cZeroMv;
    rcMvField.setMvField( cZeroMv, NOT_VALID );
    return;
  }
  
  TComCUMvField*  pcCUMvField = pcCU->getCUMvField( eRefPicList );
  rcMvField.setMvField( pcCUMvField->getMv( uiAbsPartIdx ), pcCUMvField->getRefIdx( uiAbsPartIdx ) );
}

Void TComDataCU::deriveLeftRightTopIdxGeneral ( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU + uiAbsPartIdx;
  UInt uiPUWidth = 0;
  
  switch ( m_pePartSize[uiAbsPartIdx] )
  {
    case SIZE_2Nx2N: uiPUWidth = m_puhWidth[uiAbsPartIdx];  break;
    case SIZE_2NxN:  uiPUWidth = m_puhWidth[uiAbsPartIdx];   break;
    case SIZE_Nx2N:  uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 1;  break;
    case SIZE_NxN:   uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 1; break;
    default:
      assert (0);
      break;
  }
  
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + uiPUWidth / m_pcPic->getMinCUWidth() - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxGeneral( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLB )
{
  UInt uiPUHeight = 0;
  switch ( m_pePartSize[uiAbsPartIdx] )
  {
    case SIZE_2Nx2N: uiPUHeight = m_puhHeight[uiAbsPartIdx];    break;
    case SIZE_2NxN:  uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 1;    break;
    case SIZE_Nx2N:  uiPUHeight = m_puhHeight[uiAbsPartIdx];  break;
    case SIZE_NxN:   uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 1;    break;
    default:
      assert (0);
      break;
  }
  
  ruiPartIdxLB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU + uiAbsPartIdx ] + ((uiPUHeight / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth()];
}

Void TComDataCU::deriveLeftRightTopIdx ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU;
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:                                                                                                                                break;
    case SIZE_2NxN:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1; ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2; ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * uiPartIdx;         ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 );
      break;
    default:
      assert (0);
      break;
  }
  
}

Void TComDataCU::deriveLeftBottomIdx( PartSize      eCUMode,   UInt  uiPartIdx,      UInt&      ruiPartIdxLB )
{
  ruiPartIdxLB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth()];
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:
      ruiPartIdxLB += m_uiNumPartition >> 1;
      break;
    case SIZE_2NxN:
      ruiPartIdxLB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;
      break;
    case SIZE_NxN:
      ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    default:
      assert (0);
      break;
  }
}

#if MTK_TMVP_H_MRG || MTK_TMVP_H_AMVP
/** Derives the partition index of neighbouring bottom right block
 * \param in eCUMode
 * \param in uiPartIdx 
 * \param out ruiPartIdx 
 * \returns Void
 */
Void TComDataCU::deriveRightBottomIdx( PartSize      eCUMode,   UInt  uiPartIdx,      UInt&      ruiPartIdxRB )
{
  ruiPartIdxRB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth() +  m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:  
      ruiPartIdxRB += m_uiNumPartition >> 1;    
      break;
    case SIZE_2NxN:  
      ruiPartIdxRB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;   
      break;
    case SIZE_Nx2N:  
      ruiPartIdxRB += ( uiPartIdx == 0 )? m_uiNumPartition >> 2 : (m_uiNumPartition >> 1);   
      break;
    case SIZE_NxN:   
      ruiPartIdxRB += ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 );   
      break;
    default:
      assert (0);
      break;
  }
}
#endif

Void TComDataCU::deriveLeftRightTopIdxAdi ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiNumPartInWidth = (m_puhWidth[0]/m_pcPic->getMinCUWidth())>>uiPartDepth;
  ruiPartIdxLT = m_uiAbsIdxInLCU + uiPartOffset;
  ruiPartIdxRT = g_auiRasterToZscan[ g_auiZscanToRaster[ ruiPartIdxLT ] + uiNumPartInWidth - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxAdi( UInt& ruiPartIdxLB, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiAbsIdx;
  UInt uiMinCuWidth, uiWidthInMinCus;
  
  uiMinCuWidth    = getPic()->getMinCUWidth();
  uiWidthInMinCus = (getWidth(0)/uiMinCuWidth)>>uiPartDepth;
  uiAbsIdx        = getZorderIdxInCU()+uiPartOffset+(m_uiNumPartition>>(uiPartDepth<<1))-1;
  uiAbsIdx        = g_auiZscanToRaster[uiAbsIdx]-(uiWidthInMinCus-1);
  ruiPartIdxLB    = g_auiRasterToZscan[uiAbsIdx];
}

Bool TComDataCU::hasEqualMotion( UInt uiAbsPartIdx, TComDataCU* pcCandCU, UInt uiCandAbsPartIdx )
{
  assert( getInterDir( uiAbsPartIdx ) != 0 );

  if ( getInterDir( uiAbsPartIdx ) != pcCandCU->getInterDir( uiCandAbsPartIdx ) )
  {
    return false;
  }

  for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
  {
    if ( getInterDir( uiAbsPartIdx ) & ( 1 << uiRefListIdx ) )
    {
      if ( getCUMvField( RefPicList( uiRefListIdx ) )->getMv( uiAbsPartIdx )     != pcCandCU->getCUMvField( RefPicList( uiRefListIdx ) )->getMv( uiCandAbsPartIdx ) || 
        getCUMvField( RefPicList( uiRefListIdx ) )->getRefIdx( uiAbsPartIdx ) != pcCandCU->getCUMvField( RefPicList( uiRefListIdx ) )->getRefIdx( uiCandAbsPartIdx ) )
      {
        return false;
      }
    }
  }

  return true;
}

Bool TComDataCU::avoidMergeCandidate( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComDataCU* pcCandCU, UInt uiCandAbsPartIdx )
{
  // uiAbsPartIdx has to be the first index of the current PU

  if ( !pcCandCU )
  {
    return true;
  }

  PartSize ePartSize = getPartitionSize( uiAbsPartIdx );

  if ( ePartSize != SIZE_NxN && ePartSize != SIZE_2Nx2N )
  {
    // we have exactly 2 PUs. Avoid imitating 2Nx2N.
    if ( uiPUIdx == 1 )
    {
      // we would merge the second PU
      UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;
      UInt uiAbsPartIdxOfFirstPU = uiAbsPartIdx - uiPUOffset;
      if ( hasEqualMotion( uiAbsPartIdxOfFirstPU, pcCandCU, uiCandAbsPartIdx ) )
      {
        // don't merge
        return true;
      }
    }
  }
  if ( ePartSize == SIZE_NxN )
  {
    if ( uiPUIdx == 3 )
    {
      UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;
      UInt uiPUOffsetIdx0 = uiAbsPartIdx - 3*uiPUOffset;
      UInt uiPUOffsetIdx1 = uiAbsPartIdx - 2*uiPUOffset;
      UInt uiPUOffsetIdx2 = uiAbsPartIdx -   uiPUOffset;

      // avoid imitating 2Nx2N and Nx2N partitioning.
      if ( hasEqualMotion( uiPUOffsetIdx0, this, uiPUOffsetIdx2 ) &&
           hasEqualMotion( uiPUOffsetIdx1, pcCandCU, uiCandAbsPartIdx ) )
      {
        // don't merge
        return true;
      }

      // avoid imitating 2NxN partitioning.
      if ( hasEqualMotion( uiPUOffsetIdx0, this, uiPUOffsetIdx1 ) &&
           hasEqualMotion( uiPUOffsetIdx2, pcCandCU, uiCandAbsPartIdx ) )
      {
        // don't merge
        return true;
      }
    }
  }
  return false;
}

/** Constructs a list of merging candidates
 * \param uiAbsPartIdx
 * \param uiPUIdx 
 * \param uiDepth
 * \param pcMvFieldNeighbours
 * \param puhInterDirNeighbours
 * \param puiNeighbourCandIdx
 * \returns Void
 */
Void TComDataCU::getInterMergeCandidates( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComMvField* pcMvFieldNeighbours, UChar* puhInterDirNeighbours, UInt* puiNeighbourCandIdx )
{
  UInt uiAbsPartAddr = m_uiAbsIdxInLCU + uiAbsPartIdx;
  UInt uiIdx = 1;
  bool abCandIsInter[ MRG_MAX_NUM_CANDS ];
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
  {
    abCandIsInter[ui] = false;
  }
  //left [0]
  UInt uiLeftPartIdx = 0;
  TComDataCU* pcCULeft = 0;
  pcCULeft  = getPULeft( uiLeftPartIdx, uiAbsPartAddr );
  if( avoidMergeCandidate( uiAbsPartIdx, uiPUIdx, uiDepth, pcCULeft, uiLeftPartIdx ) )
  {
    pcCULeft = NULL;
  }
  if( pcCULeft && !pcCULeft->isIntra( uiLeftPartIdx ) )
  {
    abCandIsInter[0] = true;
    puiNeighbourCandIdx[0] = uiIdx;
  }
  if ( abCandIsInter[0] )
  {
    // get Inter Dir
    puhInterDirNeighbours[0] = pcCULeft->getInterDir( uiLeftPartIdx );
    // get Mv from Left
    pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[0] );
    if ( getSlice()->isInterB() )
    {
      pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[1] );
    }
  }
  uiIdx++;


  // top [1]
  UInt uiAbovePartIdx = 0;
  TComDataCU* pcCUAbove = 0;
  pcCUAbove = getPUAbove( uiAbovePartIdx, uiAbsPartAddr );
  if( avoidMergeCandidate( uiAbsPartIdx, uiPUIdx, uiDepth, pcCUAbove, uiAbovePartIdx ) )
  {
    pcCUAbove = NULL;
  }
  if ( pcCUAbove && !pcCUAbove->isIntra( uiAbovePartIdx ) )
  {
    abCandIsInter[1] = true;
    puiNeighbourCandIdx[1] = uiIdx;
  }
  if ( abCandIsInter[1] )
  {
    // get Inter Dir
    puhInterDirNeighbours[1] = pcCUAbove->getInterDir( uiAbovePartIdx );
    // get Mv from Above
    pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[2] );
    if ( getSlice()->isInterB() )
    {
      pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[3] );
    }
  }
  uiIdx++;

  // col [2]
#if MRG_NEIGH_COL
#if PANASONIC_MRG_TMVP_REFIDX
  Int iRefIdxSkip[2] = {-1, -1};
  TComDataCU* pcTmpCU = NULL;
  UInt uiIdxblk;
  Int iRefIdxLeft[2] = {-1, -1};
  Int iRefIdxAbove[2] = {-1, -1};
  Int iRefIdxCor[2] = {-1, -1};

  UInt uiPUIdxLT = 0;
  UInt uiPUIdxRT  = 0;
  UInt uiPUIdxLB = 0;
  PartSize cCurPS = getPartitionSize( uiAbsPartIdx );
  deriveLeftRightTopIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPUIdxLT, uiPUIdxRT );
  deriveLeftBottomIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPUIdxLB );

  for (int i=0; i<2; i++) 
  {
    RefPicList  eRefPicList = ( i==1 ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
  
    pcTmpCU = getPULeft(uiIdxblk, uiPUIdxLT);
    iRefIdxLeft[i] = (pcTmpCU != NULL) ? pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdxblk) : -1;
    pcTmpCU = getPUAbove(uiIdxblk, uiPUIdxLT);
    iRefIdxAbove[i] = (pcTmpCU != NULL) ? pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdxblk) : -1;
    pcTmpCU = getPUAboveRight( uiIdxblk, uiPUIdxRT );
    if (pcTmpCU == NULL) 
    {
      pcTmpCU = getPUBelowLeft( uiIdxblk, uiPUIdxLB );
    }
    if (pcTmpCU == NULL) 
    {
      pcTmpCU = getPUAboveLeft( uiIdxblk, uiPUIdxLT );
    }
    iRefIdxCor[i] = (pcTmpCU != NULL) ? pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdxblk) : -1;

    if (iRefIdxLeft[i] == iRefIdxAbove[i] && iRefIdxAbove[i] == iRefIdxCor[i])
    {
      iRefIdxSkip[i] = (iRefIdxLeft[i] == -1) ? 0 : iRefIdxLeft[i];
    }
    else if (iRefIdxLeft[i] == iRefIdxAbove[i])
    {
      iRefIdxSkip[i] = (iRefIdxLeft[i] == -1) ? iRefIdxCor[i] : iRefIdxLeft[i];
    }
    else if (iRefIdxAbove[i] == iRefIdxCor[i])
    {
      iRefIdxSkip[i] = (iRefIdxAbove[i] == -1) ? iRefIdxLeft[i] : iRefIdxAbove[i];
    }
    else if (iRefIdxLeft[i] == iRefIdxCor[i])
    {
      iRefIdxSkip[i] = (iRefIdxLeft[i] == -1) ? iRefIdxAbove[i] : iRefIdxLeft[i];
    }
    else if (iRefIdxLeft[i] == -1)
    {
      iRefIdxSkip[i] = min(iRefIdxAbove[i], iRefIdxCor[i]);
    }
    else if (iRefIdxAbove[i] == -1)
    {
      iRefIdxSkip[i] = min(iRefIdxLeft[i], iRefIdxCor[i]);
    }
    else if (iRefIdxCor[i] == -1)
    {
      iRefIdxSkip[i] = min(iRefIdxLeft[i], iRefIdxAbove[i]);
    }
    else
    {
      iRefIdxSkip[i] = min( min(iRefIdxLeft[i], iRefIdxAbove[i]), iRefIdxCor[i]);
    }
  }
#endif
#if MTK_TMVP_H_MRG
  //>> MTK colocated-RightBottom
  UInt uiPartIdxRB;
  Int uiLCUIdx = getAddr();
  PartSize eCUMode = getPartitionSize( 0 );

  deriveRightBottomIdx( eCUMode, uiPUIdx, uiPartIdxRB );  

  UInt uiAbsPartIdxTmp = g_auiZscanToRaster[uiPartIdxRB];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  TComMv cColMv;
  Int iRefIdx;

  if      ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxTmp] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )  // image boundary check
  {
    uiLCUIdx = -1;
  }
  else if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxTmp] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiLCUIdx = -1;
  }
  else
  {
    if ( ( uiAbsPartIdxTmp % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 ) &&           // is not at the last column of LCU 
         ( uiAbsPartIdxTmp / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) ) // is not at the last row    of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdxTmp + uiNumPartInCUWidth + 1 ];
      uiLCUIdx = getAddr();
    }
    else if ( uiAbsPartIdxTmp % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )           // is not at the last column of LCU But is last row of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ (uiAbsPartIdxTmp + uiNumPartInCUWidth + 1) % m_pcPic->getNumPartInCU() ];
      uiLCUIdx = getAddr() + m_pcPic->getFrameWidthInCU();
    }
    else if ( uiAbsPartIdxTmp / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) // is not at the last row of LCU But is last column of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdxTmp + 1 ];
      uiLCUIdx = getAddr() + 1;
    }
    else //is the right bottom corner of LCU                       
    {
      uiAbsPartAddr = 0;
      uiLCUIdx = getAddr() + m_pcPic->getFrameWidthInCU() + 1;
    }
  }
#if PANASONIC_MRG_TMVP_REFIDX
  iRefIdx = iRefIdxSkip[0];
#else
  iRefIdx = 0; // scaled to 1st ref pic for List0/List1
#endif
  
#if FT_TCTR_MRG
  Bool bExistMV = false;
  UInt uiPartIdxCenter;
  UInt uiCurLCUIdx = getAddr();
  xDeriveCenterIdx( eCUMode, uiPUIdx, uiPartIdxCenter );
  bExistMV = uiLCUIdx >= 0 && xGetColMVP( REF_PIC_LIST_0, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx );
  if( bExistMV == false )
  {
    bExistMV = xGetColMVP( REF_PIC_LIST_0, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx );
  }
  if( bExistMV )
#else
  if (uiLCUIdx >= 0 && xGetColMVP( REF_PIC_LIST_0, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx ) )
#endif
  {
    UInt uiArrayAddr = uiIdx - 1;
    abCandIsInter[uiArrayAddr] = true;
#if PANASONIC_MRG_TMVP_REFIDX
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cColMv, iRefIdx );
#else
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cColMv, 0 );
#endif
    puiNeighbourCandIdx[uiArrayAddr] = uiIdx;

    if ( getSlice()->isInterB() )
    {       
#if PANASONIC_MRG_TMVP_REFIDX
      iRefIdx = iRefIdxSkip[1];
#endif
#if FT_TCTR_MRG
      bExistMV = uiLCUIdx >= 0 && xGetColMVP( REF_PIC_LIST_1, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx);
      if( bExistMV == false )
      {
        bExistMV = xGetColMVP( REF_PIC_LIST_1, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx );
      }
      if( bExistMV )
#else
      if (xGetColMVP( REF_PIC_LIST_1, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx) )
#endif
  {
#if PANASONIC_MRG_TMVP_REFIDX
        pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cColMv, iRefIdx );
#else
        pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cColMv, 0 );
#endif
        puhInterDirNeighbours[uiArrayAddr] = 3;
      }
      else
      {
        puhInterDirNeighbours[uiArrayAddr] = 1;
      }
    }
    else
    {
      puhInterDirNeighbours[uiArrayAddr] = 1;
    }
  }
  uiIdx++;
#else //MTK_TMVP_H_MRG
#if FT_TCTR_MRG
    TComMv cMvTCenter[2];
#if PANASONIC_MRG_TMVP_REFIDX
  Int iRefIdx = iRefIdxSkip[0];
  if (xGetCenterCol( uiPUIdx, REF_PIC_LIST_0, iRefIdx, &cMvTCenter[0] ))
#else
    if (xGetCenterCol( uiPUIdx, REF_PIC_LIST_0, 0, &cMvTCenter[0] ))
#endif
    {
      UInt uiArrayAddr = uiIdx - 1;
      abCandIsInter[uiArrayAddr] = true;
#if PANASONIC_MRG_TMVP_REFIDX
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cMvTCenter[0], iRefIdx );
#else
      pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cMvTCenter[0], 0 );
#endif
      puiNeighbourCandIdx[uiArrayAddr] = uiIdx;

      if ( getSlice()->isInterB() )
      {       
#if PANASONIC_MRG_TMVP_REFIDX
      iRefIdx = iRefIdxSkip[1];
      if ( xGetCenterCol( uiPUIdx, REF_PIC_LIST_1, iRefIdx, &cMvTCenter[1] ) )
#else
        if ( xGetCenterCol( uiPUIdx, REF_PIC_LIST_1, 0, &cMvTCenter[1] ) )
#endif
        {
#if PANASONIC_MRG_TMVP_REFIDX
        pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cMvTCenter[1], iRefIdx );
#else
          pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cMvTCenter[1], 0 );
#endif
          puhInterDirNeighbours[uiArrayAddr] = 3;
        }
        else
        {
          puhInterDirNeighbours[uiArrayAddr] = 1;
        }
      }
      else
      {
        puhInterDirNeighbours[uiArrayAddr] = 1;
      }
    }
  uiIdx++;
#else //FT_TCTR_MRG
  UInt uiColDir = ( m_pcSlice->isInterB()? m_pcSlice->getColDir() : 0 );
  TComDataCU* pcCUColocated = getCUColocated( RefPicList( uiColDir ) );
  RefPicList eColRefPicList = ( m_pcSlice->isInterB()? RefPicList( 1-uiColDir ) : REF_PIC_LIST_0 );
#if PANASONIC_MERGETEMPORALEXT
  RefPicList eColRefPicList2 = (m_pcSlice->isInterB()? RefPicList(uiColDir) : REF_PIC_LIST_0);
#endif
  if( pcCUColocated && !pcCUColocated->isIntra( uiAbsPartAddr ) &&
    pcCUColocated->getCUMvField( eColRefPicList )->getRefIdx( uiAbsPartAddr ) >= 0 )
  {
    Int iColPOC = pcCUColocated->getSlice()->getPOC();
    Int iColRefPOC = pcCUColocated->getSlice()->getRefPOC( eColRefPicList, pcCUColocated->getCUMvField( eColRefPicList )->getRefIdx( uiAbsPartAddr ) );
    TComMv cColMv = pcCUColocated->getCUMvField( eColRefPicList )->getMv( uiAbsPartAddr );

    Int iCurrPOC = m_pcSlice->getPOC();
    Int iCurrRefPOC = m_pcSlice->getRefPic( REF_PIC_LIST_0, 0 )->getPOC();

    TComMv cMv;
    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC );

    if( iScale == 1024 )
    {
      cMv = cColMv;
    }
    else
    {
      cMv = cColMv.scaleMv( iScale );
    }
    clipMv( cMv );
    UInt uiArrayAddr = uiIdx - 1;
    abCandIsInter[uiArrayAddr] = true;
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cMv, 0 );
    puiNeighbourCandIdx[uiArrayAddr] = uiIdx;
    if ( getSlice()->isInterB() )
    {
      iCurrRefPOC = m_pcSlice->getRefPic( REF_PIC_LIST_1, 0 )->getPOC();
      TComMv cMvB;
      iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC );
      if( iScale == 1024 )
      {
        cMvB = cColMv;
      }
      else
      {
        cMvB = cColMv.scaleMv( iScale );
      }
      clipMv( cMvB );
      pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cMvB, 0 );
      puhInterDirNeighbours[uiArrayAddr] = 3;
    }
    else
    {
      puhInterDirNeighbours[uiArrayAddr] = 1;
    }
  }
#if PANASONIC_MERGETEMPORALEXT
  else if( pcCUColocated && !pcCUColocated->isIntra( uiAbsPartAddr ) &&
          pcCUColocated->getCUMvField( eColRefPicList2 )->getRefIdx( uiAbsPartAddr ) >= 0 )
  {
    Int iColPOC = pcCUColocated->getSlice()->getPOC();
    Int iColRefPOC = pcCUColocated->getSlice()->getRefPOC( eColRefPicList2, pcCUColocated->getCUMvField( eColRefPicList2 )->getRefIdx( uiAbsPartAddr ) );
    TComMv cColMv = pcCUColocated->getCUMvField( eColRefPicList2 )->getMv( uiAbsPartAddr );
    
    Int iCurrPOC = m_pcSlice->getPOC();
    Int iCurrRefPOC = m_pcSlice->getRefPic( REF_PIC_LIST_0, 0 )->getPOC();
    
    TComMv cMv;
    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC );
    
    if( iScale == 1024 )
    {
      cMv = cColMv;
    }
    else
    {
      cMv = cColMv.scaleMv( iScale );
    }
    clipMv( cMv );
    UInt uiArrayAddr = uiIdx - 1;
    abCandIsInter[uiArrayAddr] = true;
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cMv, 0 );
    puiNeighbourCandIdx[uiArrayAddr] = uiIdx;
    if ( getSlice()->isInterB() )
    {
      iCurrRefPOC = m_pcSlice->getRefPic( REF_PIC_LIST_1, 0 )->getPOC();
      TComMv cMvB;
      iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC );
      if( iScale == 1024 )
      {
        cMvB = cColMv;
      }
      else
      {
        cMvB = cColMv.scaleMv( iScale );
      }
      clipMv( cMvB );
      pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cMvB, 0 );
      puhInterDirNeighbours[uiArrayAddr] = 3;
    }
    else
    {
      puhInterDirNeighbours[uiArrayAddr] = 1;
    }
  }
#endif // PANASONIC_MERGETEMPORALEXT
  uiIdx++;
#endif // FT_TCTR_MRG
#endif // MTK_TMVP_H_MRG 
#endif // MRG_NEIGH_COL

  // cor [3]
#if MRG_NEIGH_COL && PANASONIC_MRG_TMVP_REFIDX
  uiPUIdxLT = 0;
  uiPUIdxRT  = 0;
  uiPUIdxLB = 0;
  cCurPS = getPartitionSize( uiAbsPartIdx );
#else
  UInt uiPUIdxLT = 0;
  UInt uiPUIdxRT  = 0;
  UInt uiPUIdxLB = 0;
  PartSize cCurPS = getPartitionSize( uiAbsPartIdx );
#endif

  deriveLeftRightTopIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPUIdxLT, uiPUIdxRT );
  deriveLeftBottomIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPUIdxLB );
  
  TComDataCU* pcCorner = 0;
  UInt uiCornerIdx = 0;
  UInt uiArrayAddr = uiIdx - 1;

  Bool bRTCornerCand = false;
  Bool bLBCornerCand = false;
  for( UInt uiIter = 0; uiIter < 2; uiIter++ )
  {
    if( !bRTCornerCand )
    {
      pcCorner = getPUAboveRight( uiCornerIdx, uiPUIdxRT );
      xCheckCornerCand( pcCorner, uiCornerIdx, uiIter, bRTCornerCand );
      if( avoidMergeCandidate( uiAbsPartIdx, uiPUIdx, uiDepth, pcCorner, uiCornerIdx ) )
      {
        bRTCornerCand = false;
      }
      if( bRTCornerCand )
      {

        uiArrayAddr = 3;
        abCandIsInter[uiArrayAddr] = true;
        puiNeighbourCandIdx[uiArrayAddr] = uiArrayAddr+1;
        // get Inter Dir
        puhInterDirNeighbours[uiArrayAddr] = pcCorner->getInterDir( uiCornerIdx );
        // get Mv from Left
        pcCorner->getMvField( pcCorner, uiCornerIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[uiArrayAddr << 1] );
        if ( getSlice()->isInterB() )
        {
          pcCorner->getMvField( pcCorner, uiCornerIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[( uiArrayAddr << 1 ) + 1] );
        }
      }
    }
    if( !bLBCornerCand )
    {
      pcCorner = getPUBelowLeft( uiCornerIdx, uiPUIdxLB );
      xCheckCornerCand( pcCorner, uiCornerIdx, uiIter, bLBCornerCand );
      if( avoidMergeCandidate( uiAbsPartIdx, uiPUIdx, uiDepth, pcCorner, uiCornerIdx ) )
      {
        bLBCornerCand = false;
      }
      if( bLBCornerCand )
      {
        uiArrayAddr = 4;
        abCandIsInter[uiArrayAddr] = true;
        puiNeighbourCandIdx[uiArrayAddr] = uiArrayAddr+1;
        // get Inter Dir
        puhInterDirNeighbours[uiArrayAddr] = pcCorner->getInterDir( uiCornerIdx );
        // get Mv from Left
        pcCorner->getMvField( pcCorner, uiCornerIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[uiArrayAddr << 1] );
        if ( getSlice()->isInterB() )
        {
          pcCorner->getMvField( pcCorner, uiCornerIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[( uiArrayAddr << 1 ) + 1] );
        }
      }
    }
  }

  for( UInt uiOuter = 0; uiOuter < MRG_MAX_NUM_CANDS; uiOuter++ )
  {
    if( abCandIsInter[ uiOuter ] && ( puiNeighbourCandIdx[uiOuter] == uiOuter + 1 ) )
    {
      for( UInt uiIter = uiOuter + 1; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
      {
        if( abCandIsInter[ uiIter ] )
        {
          UInt uiMvFieldNeighIdxCurr = uiOuter << 1;
          UInt uiMvFieldNeighIdxComp = uiIter << 1;
          if( getSlice()->isInterB() )
          {
            if( puhInterDirNeighbours[ uiOuter ] == 3 )
            {
              if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getRefIdx() &&
                pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() && pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getMv() &&
                puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
              {
                puiNeighbourCandIdx[ uiIter ] = puiNeighbourCandIdx[ uiOuter ];
                abCandIsInter[ uiIter ] = false;
              }
            }
            else if( puhInterDirNeighbours[ uiOuter ] == 2 )
            {
              if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getRefIdx() &&
                pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getMv() &&
                puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
              {
                puiNeighbourCandIdx[ uiIter ] = puiNeighbourCandIdx[ uiOuter ];
                abCandIsInter[ uiIter ] = false;
              }
            }
            else
            {
              if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() &&
                pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() &&
                puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
              {
                puiNeighbourCandIdx[ uiIter ] = puiNeighbourCandIdx[ uiOuter ];
                abCandIsInter[ uiIter ] = false;
              }
            }
          }
          else
          {
            if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && 
                pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() )
            {
              assert( puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] );
              puiNeighbourCandIdx[ uiIter ] = puiNeighbourCandIdx[ uiOuter ];
              abCandIsInter[ uiIter ] = false;
            }
          }
        }
      }
    }
  }
#if AVOID_ZERO_MERGE_CANDIDATE
  // if all merge candidate
  int i;
  for ( i=0; i<MRG_MAX_NUM_CANDS; i++ )
  {
    if ( abCandIsInter[i] )
    {
      break;
    }
  }
  if ( i==MRG_MAX_NUM_CANDS ) // no merge candidate
  {
    abCandIsInter[0] = true;
    puiNeighbourCandIdx[0] = 1;
    puhInterDirNeighbours[0] = 1;
    pcMvFieldNeighbours[0].setMvField( TComMv(0, 0), 0 );
    if ( getSlice()->isInterB() )
    {
      puhInterDirNeighbours[0] = 3;
      pcMvFieldNeighbours[1].setMvField( TComMv(0, 0), 0 );
    }
  }
#endif
}

Void TComDataCU::xCheckCornerCand( TComDataCU* pcCorner, UInt uiCornerPUIdx, UInt uiIter, Bool& rbValidCand )
{
  if( uiIter == 0 )
  {
    if( pcCorner && !pcCorner->isIntra( uiCornerPUIdx ) )
    {
      rbValidCand = true;
      if( getSlice()->isInterB() )
      {
        if ( pcCorner->getInterDir( uiCornerPUIdx ) == 1 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
        else if ( pcCorner->getInterDir( uiCornerPUIdx ) == 2 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
        else
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 || pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
      }
      else if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 )
      {
        rbValidCand = false;
      }
    }
  }
  else
  {
    if( pcCorner && !pcCorner->isIntra( uiCornerPUIdx ) )
    {
      rbValidCand = true;
      if( getSlice()->isInterB() )
      {
        if ( pcCorner->getInterDir( uiCornerPUIdx ) == 1 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
        else if ( pcCorner->getInterDir( uiCornerPUIdx ) == 2 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
        else
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 || pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
      }
      else if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 )
      {
        rbValidCand = false;
      }
    }
  }
}

Void TComDataCU::setNeighbourCandIdxSubParts( UInt uiCandIdx, UChar uhNumCands, UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth )
{
  setSubPart<UChar>( uhNumCands, m_apuhNeighbourCandIdx[uiCandIdx], uiAbsPartIdx, uiDepth, uiPUIdx );
}

AMVP_MODE TComDataCU::getAMVPMode(UInt uiIdx)
{
  return m_pcSlice->getSPS()->getAMVPMode(m_puhDepth[uiIdx]);
}

/** Constructs a list of candidates for AMVP
 * \param uiPartIdx
 * \param uiPartAddr 
 * \param eRefPicList
 * \param iRefIdx
 * \param pInfo
 * \param puiNeighbourCandIdx
 * \returns Void
 */
Void TComDataCU::fillMvpCand ( UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, AMVPInfo* pInfo )
{
  PartSize eCUMode = getPartitionSize( 0 );
  
  TComMv cMvPred;
  
  pInfo->iN = 0;
  
  if (iRefIdx < 0)
  {
    return;
  }
  
  //-- Get Spatial MV
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
#if (!TI_AMVP_SMVP_SIMPLIFIED) || MTK_TMVP_H_AMVP
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
#endif
  Bool bAdded = false;
  
  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );
  
  // Left predictor search
  bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
#if TI_AMVP_SMVP_SIMPLIFIED
  if (!bAdded) 
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_LEFT );
  }
#else
  for ( Int idx = (Int)g_auiZscanToRaster[uiPartIdxLB]; !bAdded && idx >= (Int)g_auiZscanToRaster[uiPartIdxLT]; idx-= uiNumPartInCUWidth )
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[idx], MD_LEFT );
  }
#endif
  
#if MTK_AMVP_SMVP_DERIVATION
  if(!bAdded)
  {
    bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
#if TI_AMVP_SMVP_SIMPLIFIED
    if (!bAdded) 
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_LEFT );
    }
#else
    for ( Int idx = (Int)g_auiZscanToRaster[uiPartIdxLB]; !bAdded && idx >= (Int)g_auiZscanToRaster[uiPartIdxLT]; idx-= uiNumPartInCUWidth )
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[idx], MD_LEFT );
    }
#endif
  }
#endif
  // Above predictor search
  bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
  if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
  {
    pInfo->iN--; //remove duplicate entries
    bAdded = false;
  }
#if TI_AMVP_SMVP_SIMPLIFIED
  if (!bAdded) 
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE);
  }
  if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
  {
    pInfo->iN--; //remove duplicate entries
    bAdded = false;
  }
#else  
  for ( Int idx = (Int)g_auiZscanToRaster[uiPartIdxRT]; !bAdded && idx >= (Int)g_auiZscanToRaster[uiPartIdxLT]; idx-- )
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[idx], MD_ABOVE);
    if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
    {
      pInfo->iN--; //remove duplicate entries
      bAdded = false;
    }
  }
#endif
  if(!bAdded)
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
    if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
    {
      pInfo->iN--; //remove duplicate entries
      bAdded = false;
    }
  }
#if MTK_AMVP_SMVP_DERIVATION
  if(!bAdded)
  {
    bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
    if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
    {
      pInfo->iN--; //remove duplicate entries
      bAdded = false;
    }
#if TI_AMVP_SMVP_SIMPLIFIED
    if (!bAdded) 
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE);
    }
    if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
    {
      pInfo->iN--; //remove duplicate entries
      bAdded = false;
    }
#else
    for ( Int idx = (Int)g_auiZscanToRaster[uiPartIdxRT]; !bAdded && idx >= (Int)g_auiZscanToRaster[uiPartIdxLT]; idx-- )
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[idx], MD_ABOVE);
      if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
      {
        pInfo->iN--; //remove duplicate entries
        bAdded = false;
      }
    }
#endif
    if(!bAdded)
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
      if (bAdded && pInfo->iN==2 && pInfo->m_acMvCand[0] == pInfo->m_acMvCand[1])
      {
        pInfo->iN--; //remove duplicate entries
      }
    }
  }
#endif
  
  if (getAMVPMode(uiPartAddr) == AM_NONE)  //Should be optimized later for special cases
  {
    assert(pInfo->iN > 0);
    pInfo->iN = 1;
    return;
  }
  
  // Get Temporal Motion Predictor
#if AMVP_NEIGH_COL
#if MTK_TMVP_H_AMVP
  int iRefIdx_Col = iRefIdx;
  TComMv cColMv;
  UInt uiPartIdxRB;
  UInt uiAbsPartIdx;  
  UInt uiAbsPartAddr;
  int uiLCUIdx = getAddr();

  deriveRightBottomIdx( eCUMode, uiPartIdx, uiPartIdxRB );
  uiAbsPartAddr = m_uiAbsIdxInLCU + uiPartAddr;

  //----  co-located RightBottom Temporal Predictor (H) ---//
  uiAbsPartIdx = g_auiZscanToRaster[uiPartIdxRB];
  if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdx] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )  // image boundary check
  {
    uiLCUIdx = -1;
  }
  else if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdx] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiLCUIdx = -1;
  }
  else
  {
    if ( ( uiAbsPartIdx % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 ) &&           // is not at the last column of LCU 
      ( uiAbsPartIdx / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) ) // is not at the last row    of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdx + uiNumPartInCUWidth + 1 ];
      uiLCUIdx = getAddr();
    }
    else if ( uiAbsPartIdx % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )           // is not at the last column of LCU But is last row of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ (uiAbsPartIdx + uiNumPartInCUWidth + 1) % m_pcPic->getNumPartInCU() ];
      uiLCUIdx = getAddr() + m_pcPic->getFrameWidthInCU();
    }
    else if ( uiAbsPartIdx / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) // is not at the last row of LCU But is last column of LCU
    {
      uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdx + 1 ];
      uiLCUIdx = getAddr() + 1;
    }
    else //is the right bottom corner of LCU                       
    {
      uiAbsPartAddr = 0;
      uiLCUIdx = getAddr() + m_pcPic->getFrameWidthInCU() + 1;
    }
  }
  if ( uiLCUIdx >= 0 && xGetColMVP( eRefPicList, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx_Col ) )
  {
    pInfo->m_acMvCand[pInfo->iN++] = cColMv;
  }
#if FT_TCTR_AMVP
  else 
  {
    UInt uiPartIdxCenter;
    UInt uiCurLCUIdx = getAddr();
    xDeriveCenterIdx( eCUMode, uiPartIdx, uiPartIdxCenter );
    if (xGetColMVP( eRefPicList, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx_Col ))
  {
      pInfo->m_acMvCand[pInfo->iN++] = cColMv;
    }
  }
#endif
  //----  co-located RightBottom Temporal Predictor  ---//

#else //MTK_TMVP_H_AMVP
#if FT_TCTR_AMVP
    TComMv cMvTCenter;
    if (xGetCenterCol( uiPartIdx, eRefPicList, iRefIdx, &cMvTCenter ))
    {
      for (int i = pInfo->iN-1; i >= 0; i--)
      {
        pInfo->m_acMvCand[i+1] = pInfo->m_acMvCand[i];
      }
      pInfo->m_acMvCand[0] = cMvTCenter;
      pInfo->iN++;
    }
#else //FT_TCTR_AMVP
  UInt uiAbsPartAddr = m_uiAbsIdxInLCU + uiPartAddr;
  
  UInt uiColDir = (m_pcSlice->isInterB()? m_pcSlice->getColDir() : 0);
  
  TComDataCU* pcCUColocated = getCUColocated(RefPicList(uiColDir));
  
  RefPicList eColRefPicList = (m_pcSlice->isInterB()? RefPicList(1-uiColDir) : REF_PIC_LIST_0);
#if PANASONIC_AMVPTEMPORALEXT
  RefPicList eColRefPicList2 = (m_pcSlice->isInterB()? RefPicList(uiColDir) : REF_PIC_LIST_0);
#endif
  
  if ( pcCUColocated && !pcCUColocated->isIntra(uiAbsPartAddr) &&
      pcCUColocated->getCUMvField(eColRefPicList)->getRefIdx(uiAbsPartAddr) >= 0 )
  {
    Int iColPOC = pcCUColocated->getSlice()->getPOC();
    Int iColRefPOC = pcCUColocated->getSlice()->getRefPOC(eColRefPicList, pcCUColocated->getCUMvField(eColRefPicList)->getRefIdx(uiAbsPartAddr));
    TComMv cColMv = pcCUColocated->getCUMvField(eColRefPicList)->getMv(uiAbsPartAddr);
    
    Int iCurrPOC = m_pcSlice->getPOC();
    Int iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, iRefIdx)->getPOC();
    
    TComMv cMv;
    Int iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
    
    if (iScale == 1024)
    {
      cMv = cColMv;
    }
    else
    {
      cMv = cColMv.scaleMv( iScale );
    }
    
    clipMv(cMv);
    
    pInfo->m_acMvCand[pInfo->iN++] = cMv ;
  }
#if PANASONIC_AMVPTEMPORALEXT
  else if ( pcCUColocated && !pcCUColocated->isIntra(uiAbsPartAddr) &&
           pcCUColocated->getCUMvField(eColRefPicList2)->getRefIdx(uiAbsPartAddr) >= 0 )
  {
    Int iColPOC = pcCUColocated->getSlice()->getPOC();
    Int iColRefPOC = pcCUColocated->getSlice()->getRefPOC(eColRefPicList2, pcCUColocated->getCUMvField(eColRefPicList2)->getRefIdx(uiAbsPartAddr));
    TComMv cColMv = pcCUColocated->getCUMvField(eColRefPicList2)->getMv(uiAbsPartAddr);
    
    Int iCurrPOC = m_pcSlice->getPOC();
    Int iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, iRefIdx)->getPOC();
    
    TComMv cMv;
    Int iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
    
    if (iScale == 1024)
    {
      cMv = cColMv;
    }
    else
    {
      cMv = cColMv.scaleMv( iScale );
    }
    
    clipMv(cMv);
    
    pInfo->m_acMvCand[pInfo->iN++] = cMv ;
  }
#endif // PANASONIC_AMVPTEMPORALEXT
#endif // FT_TCTR_AMVP
#endif // MTK_TMVP_H_AMVP
#endif // AMVP_NEIGH_COL
  // Check No MV Candidate
  xUniqueMVPCand( pInfo );
  return ;
}

Int TComDataCU::searchMVPIdx(TComMv cMv, AMVPInfo* pInfo)
{
  for ( Int i=0; i<pInfo->iN; i++ )
  {
    if (cMv == pInfo->m_acMvCand[i])
      return i;
  }
  
  assert(0);
  return -1;
}

Void TComDataCU::clipMv    (TComMv&  rcMv)
{
  Int  iMvShift = 2;
  Int iHorMax = (m_pcSlice->getSPS()->getWidth() - m_uiCUPelX - 1 )<<iMvShift;
  Int iHorMin = (      -(Int)g_uiMaxCUWidth - (Int)m_uiCUPelX + 1 )<<iMvShift;
  
  Int iVerMax = (m_pcSlice->getSPS()->getHeight() - m_uiCUPelY - 1 )<<iMvShift;
  Int iVerMin = (      -(Int)g_uiMaxCUHeight - (Int)m_uiCUPelY + 1 )<<iMvShift;
  
  rcMv.setHor( min (iHorMax, max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( min (iVerMax, max (iVerMin, rcMv.getVer())) );
}


Void TComDataCU::convertTransIdx( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode )
{
  ruiLumaTrMode   = uiTrIdx;
  ruiChromaTrMode = uiTrIdx;
  return;
}

UInt TComDataCU::getIntraSizeIdx(UInt uiAbsPartIdx)
{
  UInt uiShift = ( (m_puhTrIdx[uiAbsPartIdx]==0) && (m_pePartSize[uiAbsPartIdx]==SIZE_NxN) ) ? m_puhTrIdx[uiAbsPartIdx]+1 : m_puhTrIdx[uiAbsPartIdx];
  uiShift = ( m_pePartSize[uiAbsPartIdx]==SIZE_NxN ? 1 : 0 );
  
  UChar uiWidth = m_puhWidth[uiAbsPartIdx]>>uiShift;
  UInt  uiCnt = 0;
  while( uiWidth )
  {
    uiCnt++;
    uiWidth>>=1;
  }
  uiCnt-=2;
  return uiCnt > 6 ? 6 : uiCnt;
}

Void TComDataCU::clearCbf( UInt uiIdx, TextType eType, UInt uiNumParts )
{
  ::memset( &m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx], 0, sizeof(UChar)*uiNumParts);
}

#if E057_INTRA_PCM
/** Set a I_PCM flag for all sub-partitions of a partition.
 * \param bIpcmFlag I_PCM flag
 * \param uiAbsPartIdx patition index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TComDataCU::setIPCMFlagSubParts  (Bool bIpcmFlag, UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset(m_pbIPCMFlag + uiAbsPartIdx, bIpcmFlag, sizeof(Bool)*uiCurrPartNumb );
}
#endif

/** Test whether the current block is skipped
 * \param uiPartIdx Block index
 * \returns Flag indicating whether the block is skipped
 */
Bool TComDataCU::isSkipped( UInt uiPartIdx )
{
  if ( m_pcSlice->isIntra () )
  {
    return false;
  }
#if HHI_MRG_SKIP
  if ( getSlice()->getSPS()->getUseMRG() )
  {
  return ( m_pePredMode[ uiPartIdx ] == MODE_SKIP && getMergeFlag( uiPartIdx ) && !getQtRootCbf( uiPartIdx ) );
  }
  else
#endif
  {
    if ( m_pcSlice->isInterP() )
    {
      return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) );
    }
    else //if ( m_pcSlice->isInterB()  )
    {
      return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) && (m_puhInterDir[uiPartIdx] == 3) );
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Bool TComDataCU::xAddMVPCand( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir )
{
  TComDataCU* pcTmpCU = NULL;
  UInt uiIdx;
  switch( eDir )
  {
    case MD_LEFT:
    {
      pcTmpCU = getPULeft(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_ABOVE:
    {
      pcTmpCU = getPUAbove(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_ABOVE_RIGHT:
    {
      pcTmpCU = getPUAboveRight(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_BELOW_LEFT:
    {
      pcTmpCU = getPUBelowLeft(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_ABOVE_LEFT:
    {
      pcTmpCU = getPUAboveLeft(uiIdx, uiPartUnitIdx);
      break;
    }
    default:
    {
      break;
    }
  }
 
  if ( pcTmpCU != NULL && m_pcSlice->isEqualRef(eRefPicList, pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx), iRefIdx) )
  {
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList)->getMv(uiIdx);
    clipMv(cMvPred);
    
    pInfo->m_acMvCand[ pInfo->iN++] = cMvPred;
    return true;
  }
  return false;
}

/**
 * Reduce list of motion vector predictors to a set of unique predictors
 * \param pInfo list of motion vector predictors
 */
Void TComDataCU::xUniqueMVPCand(AMVPInfo* pInfo)
{
  Int n = 1;
  if ( pInfo->iN == 0 )
  {
    // Add a zero candidate is none is available
    pInfo->m_acMvCand[ 0 ].setZero();
  }
  else
  {
    for (Int i = 1; i < pInfo->iN; i++)
    {
      Int j;
      for (j = n - 1; j >= 0; j--)
      {
        if ( pInfo->m_acMvCand[ i ] == pInfo->m_acMvCand[ j ] )
          break;
      }
      if ( j < 0 )
      {
        pInfo->m_acMvCand[ n++ ] = pInfo->m_acMvCand[ i ];
      }
    }
  }
  pInfo->iN = n;
}

#if MTK_AMVP_SMVP_DERIVATION
/** 
 * \param pInfo
 * \param eRefPicList 
 * \param iRefIdx
 * \param uiPartUnitIdx
 * \param eDir
 * \returns Bool
 */
Bool TComDataCU::xAddMVPCandOrder( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir )
{
  TComDataCU* pcTmpCU = NULL;
  UInt uiIdx;
  switch( eDir )
  {
  case MD_LEFT:
    {
      pcTmpCU = getPULeft(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_ABOVE:
    {
      pcTmpCU = getPUAbove(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_ABOVE_RIGHT:
    {
      pcTmpCU = getPUAboveRight(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_BELOW_LEFT:
    {
      pcTmpCU = getPUBelowLeft(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_ABOVE_LEFT:
    {
      pcTmpCU = getPUAboveLeft(uiIdx, uiPartUnitIdx);
      break;
    }
  default:
    {
      break;
    }
  }

  if ( pcTmpCU == NULL ) 
    return false;
    
  RefPicList eRefPicList2nd = REF_PIC_LIST_0;
  if(       eRefPicList == REF_PIC_LIST_0 )
    eRefPicList2nd = REF_PIC_LIST_1;
  else if ( eRefPicList == REF_PIC_LIST_1)
    eRefPicList2nd = REF_PIC_LIST_0;


  Int iCurrPOC = m_pcSlice->getPOC();
  Int iCurrRefPOC = m_pcSlice->getRefPic( eRefPicList, iRefIdx)->getPOC();
  Int iNeibPOC = iCurrPOC;
  Int iNeibRefPOC;


  if( pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) >= 0 )
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList2nd, pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) );
    if( iNeibRefPOC == iCurrRefPOC ) // Same Reference Frame But Diff List//
    {
      TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList2nd)->getMv(uiIdx);

      clipMv(cMvPred);
      pInfo->m_acMvCand[ pInfo->iN++] = cMvPred;
      return true;
    }
  }
  //---------------  V1 (END) ------------------//
  if( pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx) >= 0)
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList, pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx) );
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList)->getMv(uiIdx);
    TComMv rcMv;

    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iNeibPOC, iNeibRefPOC );
    if ( iScale == 1024 )
    {
      rcMv = cMvPred;
    }
    else
    {
      rcMv = cMvPred.scaleMv( iScale );
    }

    clipMv(rcMv);

    pInfo->m_acMvCand[ pInfo->iN++] = rcMv;
    return true;
  }
  //---------------------- V2(END) --------------------//
  if( pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) >= 0)
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList2nd, pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) );
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList2nd)->getMv(uiIdx);
    TComMv rcMv;

    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iNeibPOC, iNeibRefPOC );
    if ( iScale == 1024 )
    {
      rcMv = cMvPred;
    }
    else
    {
      rcMv = cMvPred.scaleMv( iScale );
    }

    clipMv(rcMv);

    pInfo->m_acMvCand[ pInfo->iN++] = rcMv;
    return true;
  }
  //---------------------- V3(END) --------------------//
  return false;
}
#endif

/** 
 * \param eRefPicList
 * \param uiCUAddr 
 * \param uiPartUnitIdx
 * \param riRefIdx
 * \returns Bool
 */
#if MTK_TMVP_H_MRG || MTK_TMVP_H_AMVP
Bool TComDataCU::xGetColMVP( RefPicList eRefPicList, Int uiCUAddr, Int uiPartUnitIdx, TComMv& rcMv, Int& riRefIdx )
{
  UInt uiAbsPartAddr = uiPartUnitIdx;

  RefPicList  eColRefPicList;
  Int iColPOC, iColRefPOC, iCurrPOC, iCurrRefPOC, iScale;
  TComMv cColMv;

  iCurrPOC = m_pcSlice->getPOC();    
  iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, riRefIdx)->getPOC();

  // use coldir.
  TComPic *pColPic = getSlice()->getRefPic( RefPicList(getSlice()->isInterB() ? getSlice()->getColDir() : 0), 0);
  TComDataCU *pColCU = pColPic->getCU( uiCUAddr );
  iColPOC = pColCU->getSlice()->getPOC();  

  if (pColCU->isIntra(uiAbsPartAddr))
  {
    return false;
  }
  // Prefer a vector crossing us.  Prefer shortest.
  eColRefPicList = REF_PIC_LIST_0;
  bool bFirstCrosses = false;
  Int  iFirstColDist = -1;

  for (Int l = 0; l < 2; l++)
  {
    bool bSaveIt = false;
    int iColRefIdx = pColCU->getCUMvField(RefPicList(l))->getRefIdx(uiAbsPartAddr);
    if (iColRefIdx < 0)
    {
      continue;
    }
    iColRefPOC = pColCU->getSlice()->getRefPOC(RefPicList(l), iColRefIdx);
    int iColDist = abs(iColRefPOC - iColPOC);
    bool bCrosses = iColPOC < iCurrPOC ? iColRefPOC > iCurrPOC : iColRefPOC < iCurrPOC;
    if (iFirstColDist < 0)
    {
      bSaveIt = true;
    }
    else if (bCrosses && !bFirstCrosses)
    {
      bSaveIt = true;
    }
#if PANASONIC_AMVPTEMPORALMOD
    else if (bCrosses == bFirstCrosses && l == eRefPicList)
#else
    else if (bCrosses == bFirstCrosses && iColDist < iFirstColDist)
#endif
    {
      bSaveIt = true;
    }
    if (bSaveIt)
    {
      bFirstCrosses = bCrosses;
      iFirstColDist = iColDist;
      eColRefPicList = RefPicList(l);
    }
  }
  // Scale the vector.
  iColRefPOC = pColCU->getSlice()->getRefPOC(eColRefPicList, pColCU->getCUMvField(eColRefPicList)->getRefIdx(uiAbsPartAddr));
  cColMv = pColCU->getCUMvField(eColRefPicList)->getMv(uiAbsPartAddr);

  iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, riRefIdx)->getPOC();
  iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
  if (iScale == 1024)
  {
    rcMv = cColMv;
  }
  else
  {
    rcMv = cColMv.scaleMv( iScale );
  }
  clipMv(rcMv);
  return true;
}
#endif

UInt TComDataCU::xGetMvdBits(TComMv cMvd)
{
  return ( xGetComponentBits(cMvd.getHor()) + xGetComponentBits(cMvd.getVer()) );
}

UInt TComDataCU::xGetComponentBits(Int iVal)
{
  UInt uiLength = 1;
  UInt uiTemp   = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);
  
  assert ( uiTemp );
  
  while ( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  return uiLength;
}


Int TComDataCU::xGetDistScaleFactor(Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC)
{
  Int iDiffPocD = iColPOC - iColRefPOC;
  Int iDiffPocB = iCurrPOC - iCurrRefPOC;
  
  if( iDiffPocD == iDiffPocB )
  {
    return 1024;
  }
  else
  {
    Int iTDB      = Clip3( -128, 127, iDiffPocB );
    Int iTDD      = Clip3( -128, 127, iDiffPocD );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
    Int iScale    = Clip3( -1024, 1023, (iTDB * iX + 32) >> 6 );
    return iScale;
  }
}

#if FT_TCTR_AMVP || FT_TCTR_MRG
/** 
 * \param eCUMode
 * \param uiPartIdx 
 * \param ruiPartIdxCenter
 * \returns Void
 */
Void TComDataCU::xDeriveCenterIdx( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxCenter )
{
  UInt uiPartAddr;
  Int  iPartWidth;
  Int  iPartHeight;
  getPartIndexAndSize( uiPartIdx, uiPartAddr, iPartWidth, iPartHeight);
  
  ruiPartIdxCenter = m_uiAbsIdxInLCU+uiPartAddr; // partition origin.
  ruiPartIdxCenter = g_auiRasterToZscan[ g_auiZscanToRaster[ ruiPartIdxCenter ]
                                        + ( iPartHeight/m_pcPic->getMinCUHeight() -1 )/2*m_pcPic->getNumPartInWidth()
                                        + ( iPartWidth/m_pcPic->getMinCUWidth()  -1 )/2];
}

/** 
 * \param uiPartIdx
 * \param eRefPicList 
 * \param iRefIdx
 * \param pcMv
 * \returns Bool
 */
Bool TComDataCU::xGetCenterCol( UInt uiPartIdx, RefPicList eRefPicList, int iRefIdx, TComMv *pcMv )
{
  PartSize eCUMode = getPartitionSize( 0 );
  
  Int iCurrPOC = m_pcSlice->getPOC();
  
  // use coldir.
  TComPic *pColPic = getSlice()->getRefPic( RefPicList(getSlice()->isInterB() ? getSlice()->getColDir() : 0), 0);
  TComDataCU *pColCU = pColPic->getCU( m_uiCUAddr );
  
  Int iColPOC = pColCU->getSlice()->getPOC();
  UInt uiPartIdxCenter;
  xDeriveCenterIdx( eCUMode, uiPartIdx, uiPartIdxCenter );
  
  if (pColCU->isIntra(uiPartIdxCenter))
    return false;
  
  // Prefer a vector crossing us.  Prefer shortest.
  RefPicList eColRefPicList = REF_PIC_LIST_0;
  bool bFirstCrosses = false;
  Int  iFirstColDist = -1;
  for (Int l = 0; l < 2; l++)
  {
    bool bSaveIt = false;
    int iColRefIdx = pColCU->getCUMvField(RefPicList(l))->getRefIdx(uiPartIdxCenter);
    if (iColRefIdx < 0)
      continue;
    int iColRefPOC = pColCU->getSlice()->getRefPOC(RefPicList(l), iColRefIdx);
    int iColDist = abs(iColRefPOC - iColPOC);
    bool bCrosses = iColPOC < iCurrPOC ? iColRefPOC > iCurrPOC : iColRefPOC < iCurrPOC;
    if (iFirstColDist < 0)
      bSaveIt = true;
    else if (bCrosses && !bFirstCrosses)
      bSaveIt = true;
#if PANASONIC_AMVPTEMPORALMOD
    else if (bCrosses == bFirstCrosses && l == eRefPicList)
#else
    else if (bCrosses == bFirstCrosses && iColDist < iFirstColDist)
#endif
      bSaveIt = true;
    
    if (bSaveIt)
    {
      bFirstCrosses = bCrosses;
      iFirstColDist = iColDist;
      eColRefPicList = RefPicList(l);
    }
  }
  
  // Scale the vector.
  Int iColRefPOC = pColCU->getSlice()->getRefPOC(eColRefPicList, pColCU->getCUMvField(eColRefPicList)->getRefIdx(uiPartIdxCenter));
  TComMv cColMv = pColCU->getCUMvField(eColRefPicList)->getMv(uiPartIdxCenter);
  
  Int iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, iRefIdx)->getPOC();
  Int iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
  if (iScale == 1024)
  {
    pcMv[0] = cColMv;
  }
  else
  {
    pcMv[0] = cColMv.scaleMv( iScale );
  }
  clipMv(pcMv[0]);
  
  return true;
}
#endif

#if AMVP_BUFFERCOMPRESS
Void TComDataCU::compressMV()
{
  Int scaleFactor = 4 * AMVP_DECIMATION_FACTOR / m_unitSize;
  if (scaleFactor > 0)
  {
    m_acCUMvField[0].compress(m_pePredMode, scaleFactor);
    m_acCUMvField[1].compress(m_pePredMode, scaleFactor);    
  }
}
#endif 

#if QC_MDCS
UInt TComDataCU::getCoefScanIdx(UInt uiAbsPartIdx, UInt uiWidth, Bool bIsLuma, Bool bIsIntra)
{
#if ADD_PLANAR_MODE || LM_CHROMA
  static const UChar aucIntraDirToScanIdx[MAX_CU_DEPTH][NUM_INTRA_MODE] =
#else
  static const UChar aucIntraDirToScanIdx[MAX_CU_DEPTH][34] =
#endif
  {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {1, 2, 0, 0, 1, 1, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {1, 2, 0, 0, 1, 1, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if ADD_PLANAR_MODE || LM_CHROMA
      0, 0
#endif
    },
  };
  
  UInt uiCTXIdx;
  UInt uiScanIdx;
  UInt uiDirMode;

  if ( !bIsIntra ) 
  {
    uiScanIdx = SCAN_ZIGZAG;
    return uiScanIdx;
  }

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

  if ( bIsLuma )
  {
    uiDirMode = getLumaIntraDir(uiAbsPartIdx);
    uiScanIdx = aucIntraDirToScanIdx[uiCTXIdx][uiDirMode];
  }
  else
  {
    uiDirMode = getChromaIntraDir(uiAbsPartIdx);
    if (uiDirMode == 4)
    {
      uiDirMode = getLumaIntraDir(uiAbsPartIdx);
    }
    uiScanIdx = aucIntraDirToScanIdx[max<Int>(uiCTXIdx-1,0)][uiDirMode];
  }

  return uiScanIdx;
}
#endif //QC_MDCS

Bool TComDataCU::isSuroundingRefIdxException     ( UInt   uiAbsPartIdx )
{
  if ( !getSlice()->isInterB() )
    return true;
  
  if ( getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2)
  {
    return false;
  }
  
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  TComMvField cMvFieldTemp;
  
  if ( getSlice()->getNumRefIdx ( REF_PIC_LIST_C ) > 0 )  // combined list case
  {
    // Left PU
    pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
      {
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
        
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
      }
    }
    
    // Above PU
    pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
      {
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
        
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
      }
    }
    
    // Above left PU
    pcTempCU = getPUAboveLeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_0, pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
      {
        if ( pcTempCU->getSlice()->getRefIdxOfLC(REF_PIC_LIST_1, pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx )) > 3 )
          return true;
      }
      else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
      {
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
        
        if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
          return true;
      }
    }
    
    return false;
  }
  
  // not combined list case
  // Left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
      
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
    }
  }
  
  // Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
      
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
    }
  }
  
  // Above left PU
  pcTempCU = getPUAboveLeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    if (pcTempCU->getInterDir(uiTempPartIdx) == 1 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if (pcTempCU->getInterDir(uiTempPartIdx) == 2 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > MS_LCEC_UNI_EXCEPTION_THRES )
        return true;
    }
    else if ( pcTempCU->getInterDir(uiTempPartIdx) == 3 )
    {
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
      
      if ( pcTempCU->getCUMvField( REF_PIC_LIST_1 )->getRefIdx( uiTempPartIdx ) > 1 )
        return true;
    }
  }
  
  return false;
}
