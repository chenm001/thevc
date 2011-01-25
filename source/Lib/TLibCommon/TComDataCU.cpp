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
#if HHI_MRG
  m_pbMergeFlag        = NULL;
  m_puhMergeIndex      = NULL;
#endif
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
}

TComDataCU::~TComDataCU()
{
}

Void TComDataCU::create(UInt uiNumPartition, UInt uiWidth, UInt uiHeight, Bool bDecSubCu)
{
  m_bDecSubCu = bDecSubCu;
  
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  m_uiNumPartition     = uiNumPartition;
  
  if ( !bDecSubCu )
  {
    m_phQP               = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhDepth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhWidth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhHeight          = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_pePartSize         = (PartSize* )xMalloc(PartSize, uiNumPartition);
    m_pePredMode         = (PredMode* )xMalloc(PredMode, uiNumPartition);
    
    m_puiAlfCtrlFlag     = (UInt*  )xMalloc(UInt,   uiNumPartition);
    
#if HHI_MRG
    m_pbMergeFlag        = (Bool*  )xMalloc(Bool,   uiNumPartition);
    m_puhMergeIndex      = (UChar* )xMalloc(UChar,  uiNumPartition);
#endif
    
    m_puhLumaIntraDir    = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhChromaIntraDir  = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_apiMVPIdx[0]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPIdx[1]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[0]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[1]       = (Int*   )xMalloc(Int,  uiNumPartition);
    
    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    
    m_acCUMvField[0].create( uiNumPartition );
    m_acCUMvField[1].create( uiNumPartition );
    
  }
  else
  {
    m_acCUMvField[0].setNumPartition(uiNumPartition );
    m_acCUMvField[1].setNumPartition(uiNumPartition );
  }
  
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
    if ( m_pePartSize         ) { xFree(m_pePartSize);          m_pePartSize        = NULL; }
    if ( m_pePredMode         ) { xFree(m_pePredMode);          m_pePredMode        = NULL; }
    if ( m_puhCbf[0]          ) { xFree(m_puhCbf[0]);           m_puhCbf[0]         = NULL; }
    if ( m_puhCbf[1]          ) { xFree(m_puhCbf[1]);           m_puhCbf[1]         = NULL; }
    if ( m_puhCbf[2]          ) { xFree(m_puhCbf[2]);           m_puhCbf[2]         = NULL; }
    if ( m_puiAlfCtrlFlag     ) { xFree(m_puiAlfCtrlFlag);      m_puiAlfCtrlFlag    = NULL; }
    if ( m_puhInterDir        ) { xFree(m_puhInterDir);         m_puhInterDir       = NULL; }
#if HHI_MRG
    if ( m_pbMergeFlag        ) { xFree(m_pbMergeFlag);         m_pbMergeFlag       = NULL; }
    if ( m_puhMergeIndex      ) { xFree(m_puhMergeIndex);       m_puhMergeIndex     = NULL; }
#endif
    if ( m_puhLumaIntraDir    ) { xFree(m_puhLumaIntraDir);     m_puhLumaIntraDir   = NULL; }
    if ( m_puhChromaIntraDir  ) { xFree(m_puhChromaIntraDir);   m_puhChromaIntraDir = NULL; }
    if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);            m_puhTrIdx          = NULL; }
    if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);          m_pcTrCoeffY        = NULL; }
    if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);         m_pcTrCoeffCb       = NULL; }
    if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);         m_pcTrCoeffCr       = NULL; }
    if ( m_apiMVPIdx[0]       ) { xFree(m_apiMVPIdx[0]);        m_apiMVPIdx[0]      = NULL; }
    if ( m_apiMVPIdx[1]       ) { xFree(m_apiMVPIdx[1]);        m_apiMVPIdx[1]      = NULL; }
    if ( m_apiMVPNum[0]       ) { xFree(m_apiMVPNum[0]);        m_apiMVPNum[0]      = NULL; }
    if ( m_apiMVPNum[1]       ) { xFree(m_apiMVPNum[1]);        m_apiMVPNum[1]      = NULL; }
    
    m_acCUMvField[0].destroy();
    m_acCUMvField[1].destroy();
    
  }
  
  m_pcCUAboveLeft       = NULL;
  m_pcCUAboveRight      = NULL;
  m_pcCUAbove           = NULL;
  m_pcCULeft            = NULL;
  
  m_apcCUColocated[0]   = NULL;
  m_apcCUColocated[1]   = NULL;
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
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{
  m_pcPic              = pcPic;
  m_pcSlice            = pcPic->getSlice();
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
#if HHI_MRG
  Int iSizeInBool  = sizeof( Bool  ) * m_uiNumPartition;
#endif
  
  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );
  
  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt  );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
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
  
  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
    
    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
  }
  
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof( TCoeff ) * uiTmp );
  
  uiTmp  >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof( TCoeff ) * uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof( TCoeff ) * uiTmp );
  
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
  
  if ( pcPic->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
  {
    m_apcCUColocated[0] = pcPic->getSlice()->getRefPic( REF_PIC_LIST_0, 0)->getCU( m_uiCUAddr );
  }
  
  if ( pcPic->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
  {
    m_apcCUColocated[1] = pcPic->getSlice()->getRefPic( REF_PIC_LIST_1, 0)->getCU( m_uiCUAddr );
  }
}

// initialize prediction data
Void TComDataCU::initEstData()
{
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;
#if HHI_MRG
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
#endif
  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );
  
  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  
  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
    
    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
  }
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
  
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
  
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
}

// initialize Sub partition
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
{
  assert( uiPartUnitIdx<4 );
  
  UInt uiPartOffset = ( pcCU->getTotalNumPart()>>2 )*uiPartUnitIdx;
  
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = pcCU->getZorderIdxInCU() + uiPartOffset;
  m_uiCUPelX           = pcCU->getCUPelX() + ( pcCU->getWidth (0) >> 1 )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( pcCU->getHeight(0) >> 1 )*( uiPartUnitIdx >> 1 );
  
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  
  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;
  
  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;
#if HHI_MRG
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
#endif
  
  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );
  
  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
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
  
  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
    
    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
  }
  
  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
  
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
  
  m_pcCULeft        = pcCU->getCULeft();
  m_pcCUAbove       = pcCU->getCUAbove();
  m_pcCUAboveLeft   = pcCU->getCUAboveLeft();
  m_pcCUAboveRight  = pcCU->getCUAboveRight();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();
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
  m_pePartSize=pcCU->getPartitionSize()   + uiPart;
  m_pePredMode=pcCU->getPredictionMode()  + uiPart;
  
#if HHI_MRG
  m_pbMergeFlag         = pcCU->getMergeFlag()        + uiPart;
  m_puhMergeIndex       = pcCU->getMergeIndex()       + uiPart;
#endif
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
  
  m_pcTrCoeffY=pcCU->getCoeffY()  + uiCoffOffset;
  
  uiTmp >>= 2;
  uiCoffOffset >>=2;
  m_pcTrCoeffCb=pcCU->getCoeffCb() + uiCoffOffset;
  m_pcTrCoeffCr=pcCU->getCoeffCr() + uiCoffOffset;
  
  m_acCUMvField[0].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMv()     + uiPart);
  m_acCUMvField[0].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMvd()    + uiPart);
  m_acCUMvField[0].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx() + uiPart);
  m_acCUMvField[1].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMv()     + uiPart);
  m_acCUMvField[1].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMvd()    + uiPart);
  m_acCUMvField[1].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx() + uiPart);
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
  
  m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
  m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;
  
  m_acCUMvField[eRefPicList].setMvPtr(pcCU->getCUMvField(eRefPicList)->getMv()     + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setMvdPtr(pcCU->getCUMvField(eRefPicList)->getMvd()    + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setRefIdxPtr(pcCU->getCUMvField(eRefPicList)->getRefIdx() + uiAbsPartIdx);
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
  Int iSizeInUInt   = sizeof( UInt  ) * uiNumPartition;
  Int iSizeInInt    = sizeof( Int   ) * uiNumPartition;
#if HHI_MRG
  Int iSizeInBool   = sizeof( Bool  ) * uiNumPartition;
#endif
  
  memcpy( m_phQP       + uiOffset, pcCU->getQP(),             iSizeInUchar                        );
  memcpy( m_pePartSize + uiOffset, pcCU->getPartitionSize(),  sizeof( PartSize ) * uiNumPartition );
  memcpy( m_pePredMode + uiOffset, pcCU->getPredictionMode(), sizeof( PredMode ) * uiNumPartition );
  
  memcpy( m_puiAlfCtrlFlag      + uiOffset, pcCU->getAlfCtrlFlag(),       iSizeInUInt  );
#if HHI_MRG
  memcpy( m_pbMergeFlag         + uiOffset, pcCU->getMergeFlag(),         iSizeInBool  );
  memcpy( m_puhMergeIndex       + uiOffset, pcCU->getMergeIndex(),        iSizeInUchar );
#endif
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
  
  memcpy( m_apiMVPIdx[0] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPIdx[1] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_1), iSizeInInt );
  memcpy( m_apiMVPNum[0] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPNum[1] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_1), iSizeInInt );
  
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
  
  uiTmp >>= 2; uiTmp2>>= 2;
  memcpy( m_pcTrCoeffCb + uiTmp2, pcCU->getCoeffCb(), sizeof(TCoeff)*uiTmp );
  memcpy( m_pcTrCoeffCr + uiTmp2, pcCU->getCoeffCr(), sizeof(TCoeff)*uiTmp );
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
  Int iSizeInUInt   = sizeof( UInt  ) * m_uiNumPartition;
  Int iSizeInInt    = sizeof( Int   ) * m_uiNumPartition;
#if HHI_MRG
  Int iSizeInBool   = sizeof( Bool  ) * m_uiNumPartition;
#endif
  
  memcpy( rpcCU->getQP() + m_uiAbsIdxInLCU, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + m_uiAbsIdxInLCU, m_pePartSize, sizeof( PartSize ) * m_uiNumPartition );
  memcpy( rpcCU->getPredictionMode() + m_uiAbsIdxInLCU, m_pePredMode, sizeof( PredMode ) * m_uiNumPartition );
  
  memcpy( rpcCU->getAlfCtrlFlag()    + m_uiAbsIdxInLCU, m_puiAlfCtrlFlag,    iSizeInUInt  );
  
#if HHI_MRG
  memcpy( rpcCU->getMergeFlag()         + m_uiAbsIdxInLCU, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + m_uiAbsIdxInLCU, m_puhMergeIndex,       iSizeInUchar );
#endif
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
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPNum[1], iSizeInInt );
  
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU );
  
  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>(uhDepth<<1);
  UInt uiTmp2 = m_uiAbsIdxInLCU*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
  
  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
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
  Int iSizeInUInt   = sizeof( UInt   ) * uiQNumPart;
  Int iSizeInInt    = sizeof( Int    ) * uiQNumPart;
#if HHI_MRG
  Int iSizeInBool   = sizeof( Bool   ) * uiQNumPart;
#endif
  
  memcpy( rpcCU->getQP() + uiPartOffset, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + uiPartOffset, m_pePartSize, sizeof( PartSize ) * uiQNumPart );
  memcpy( rpcCU->getPredictionMode() + uiPartOffset, m_pePredMode, sizeof( PredMode ) * uiQNumPart );
  
  memcpy( rpcCU->getAlfCtrlFlag()       + uiPartOffset, m_puiAlfCtrlFlag,      iSizeInUInt  );
#if HHI_MRG
  memcpy( rpcCU->getMergeFlag()         + uiPartOffset, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + uiPartOffset, m_puhMergeIndex,       iSizeInUchar );
#endif
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
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPNum[1], iSizeInInt );
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  
  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>((uhDepth+uiPartDepth)<<1);
  UInt uiTmp2 = uiPartOffset*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
  
  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
}

// --------------------------------------------------------------------------------------------------------------------
// Other public functions
// --------------------------------------------------------------------------------------------------------------------

TComDataCU* TComDataCU::getPULeft( UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    if ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth )
    {
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiLPartUnitIdx -= m_uiAbsIdxInLCU;
      return this;
    }
  }
  
  uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + uiNumPartInCUWidth - 1 ];
  return m_pcCULeft;
}

TComDataCU* TComDataCU::getPUAbove( UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth ];
    if ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth )
    {
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiAPartUnitIdx -= m_uiAbsIdxInLCU;
      return this;
    }
  }
  
  uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAbove;
}

TComDataCU* TComDataCU::getPUAboveLeft( UInt& uiALPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    if( uiAbsPartIdx / uiNumPartInCUWidth )
    {
      uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth - 1 ];
      if ( ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
      {
        return m_pcPic->getCU( getAddr() );
      }
      else
      {
        uiALPartUnitIdx -= m_uiAbsIdxInLCU;
        return this;
      }
    }
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + getPic()->getNumPartInCU() - uiNumPartInCUWidth - 1 ];
    return m_pcCUAbove;
  }
  
  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    return m_pcCULeft;
  }
  
  uiALPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - 1 ];
  return m_pcCUAboveLeft;
}

TComDataCU* TComDataCU::getPUAboveRight( UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }
  
  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}

TComDataCU* TComDataCU::getPUBelowLeft( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

TComDataCU* TComDataCU::getPUBelowLeftAdi(UInt& uiBLPartUnitIdx, UInt uiPuHeight,  UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ((m_puhHeight[0] / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + uiPuHeight ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}



TComDataCU* TComDataCU::getPUAboveRightAdi(UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhWidth[0] / m_pcPic->getMinCUWidth()) - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + uiPuWidth ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }
  
  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}

Int TComDataCU::getMostProbableIntraDirLuma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir, iMostProbable;
  
  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
  
  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
  
  iMostProbable  = Min( iLeftIntraDir, iAboveIntraDir );
  
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
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getChromaIntraDir( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;
  
  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getChromaIntraDir( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxCbf( UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx, uiTempTrDepth;
  UInt        uiErrRet = !isIntra(uiAbsPartIdx) ? 0 : 1;
  UInt        uiCtx = 0;
  
  // Get Cbf of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    uiTempTrDepth = pcTempCU->getTransformIdx( uiTempPartIdx );
    uiCtx = pcTempCU->getCbf( uiTempPartIdx, eType, uiTempTrDepth < uiTrDepth ? uiTempTrDepth : uiTrDepth );
  }
  else
  {
    uiCtx = uiErrRet;
  }
  
  // Get Cbf of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    uiTempTrDepth = pcTempCU->getTransformIdx( uiTempPartIdx );
    uiCtx += pcTempCU->getCbf( uiTempPartIdx, eType, uiTempTrDepth < uiTrDepth ? uiTempTrDepth : uiTrDepth ) << 1;
  }
  else
  {
    uiCtx += uiErrRet << 1;
  }
  
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
      uiCtx = pcTempCU->getCbf( uiTempPartIdx, eType, uiTempTrDepth );
    }
    
    // Get Cbf of above PU
    pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
    if ( pcTempCU )
    {
      uiTempTrDepth = pcTempCU->getTransformIdx( uiTempPartIdx );
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
    uiCtx = pcTempCU->getQtRootCbf( uiTempPartIdx );
  }
  
  // Get RootCbf of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
    uiCtx += pcTempCU->getQtRootCbf( uiTempPartIdx ) << 1;
  }
  
  return uiCtx;
}

UInt TComDataCU::getQuadtreeTULog2MinSizeInCU( UInt uiIdx )
{
#if HHI_RQT_DISABLE_SUB
  return getQuadtreeTULog2RootSizeInCU(uiIdx);
#else
#if HHI_RQT_FORCE_SPLIT_ACC2_PU
  UInt uiLog2MinTUSizeInCU = getQuadtreeTULog2RootSizeInCU(uiIdx);
#else
  UInt uiLog2MinTUSizeInCU = g_aucConvertToBit[getWidth( uiIdx )] + 2;
  
  if ( getPredictionMode( uiIdx ) == MODE_INTRA && getPartitionSize( uiIdx ) == SIZE_NxN )
  {
    uiLog2MinTUSizeInCU--;
  }
#endif  
  
  UInt uiQuadtreeTUMaxDepth = getPredictionMode( uiIdx ) == MODE_INTRA ? m_pcSlice->getSPS()->getQuadtreeTUMaxDepthIntra() : m_pcSlice->getSPS()->getQuadtreeTUMaxDepthInter();
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
  return uiLog2MinTUSizeInCU;
#endif
}

#if HHI_RQT_FORCE_SPLIT_ACC2_PU || HHI_RQT_DISABLE_SUB
UInt TComDataCU::getQuadtreeTULog2RootSizeInCU( UInt uiIdx )
{
  UInt uiLog2RootTUSizeInCU = g_aucConvertToBit[getWidth( uiIdx )] +2;
  
#if !HHI_RQT_FORCE_SPLIT_NxN && !HHI_RQT_DISABLE_SUB
  if ( getPredictionMode( uiIdx ) == MODE_INTRA && getPartitionSize( uiIdx ) == SIZE_NxN )
  {
    uiLog2RootTUSizeInCU--;
    return (uiLog2RootTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() ? m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() : uiLog2RootTUSizeInCU);
  }
#endif
  
  if (m_pePartSize[ uiIdx ] == SIZE_2Nx2N)
  {
    return (uiLog2RootTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() ? m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() : uiLog2RootTUSizeInCU);
  }
  else if (m_pePartSize[ uiIdx ] >= SIZE_2NxN && m_pePartSize[ uiIdx ] <= SIZE_Nx2N)  
  {
#if HHI_RQT_FORCE_SPLIT_RECT || HHI_RQT_DISABLE_SUB
    uiLog2RootTUSizeInCU--;
#endif
    return (uiLog2RootTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() ? m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() : uiLog2RootTUSizeInCU);
  }
  else if (m_pePartSize[ uiIdx ] == SIZE_NxN)  
  {
#if HHI_RQT_FORCE_SPLIT_NxN || HHI_RQT_DISABLE_SUB
    uiLog2RootTUSizeInCU--;
#endif
    return (uiLog2RootTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() ? m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() : uiLog2RootTUSizeInCU);
  }
  
  return (uiLog2RootTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() ? m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() : uiLog2RootTUSizeInCU);
}
#endif

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

#if HHI_MRG
UInt TComDataCU::getCtxMergeFlag( UInt uiAbsPartIdx )
{
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get Merge Flag of left PU
  TComDataCU* pcTopCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  Bool bTopIsInter = ( pcTopCU && !pcTopCU->isIntra( uiTempPartIdx ) );
  Bool bTopMergeFlag = ( pcTopCU && pcTopCU->getMergeFlag( uiTempPartIdx ) );
  TComDataCU* pcLeftCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  Bool bLeftIsInter = ( pcLeftCU && !pcLeftCU->isIntra( uiTempPartIdx ) );
  Bool bLeftMergeFlag = ( pcLeftCU && pcLeftCU->getMergeFlag( uiTempPartIdx ) );
  
  if ( !bTopIsInter || !bLeftIsInter )
  {
    uiCtx = 0;
  }
  else
  {
    uiCtx = ( ( bTopMergeFlag && bLeftMergeFlag ) ? 2 : 1 );
  }
  
  return uiCtx;
}

UInt TComDataCU::getCtxMergeIndex( UInt uiAbsPartIdx )
{
  UInt uiCtx = 0;
  return uiCtx;
}
#endif

UInt TComDataCU::getCtxPredMode( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;
  
  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;
  
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

Void TComDataCU::setAlfCtrlFlagSubParts         ( UInt uiFlag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  for (UInt ui = 0; ui < uiCurrPartNumb; ui++ )
  {
    m_puiAlfCtrlFlag[uiAbsPartIdx + ui] = uiFlag;
  }
}

Void TComDataCU::createTmpAlfCtrlFlag()
{
  m_puiTmpAlfCtrlFlag = (UInt* )xMalloc(UInt, m_uiNumPartition);
}

Void TComDataCU::destroyTmpAlfCtrlFlag()
{
  if(m_puiTmpAlfCtrlFlag)
  {
    xFree(m_puiTmpAlfCtrlFlag);        m_puiTmpAlfCtrlFlag = NULL;
  }
}

Void TComDataCU::copyAlfCtrlFlagToTmp()
{
  memcpy( m_puiTmpAlfCtrlFlag, m_puiAlfCtrlFlag, sizeof(UInt)*m_uiNumPartition );
}

Void TComDataCU::copyAlfCtrlFlagFromTmp()
{
  memcpy( m_puiAlfCtrlFlag, m_puiTmpAlfCtrlFlag, sizeof(UInt)*m_uiNumPartition );
}

Void TComDataCU::setPartSizeSubParts( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  for (UInt ui = 0; ui < uiCurrPartNumb; ui++ )
  {
    m_pePartSize[uiAbsPartIdx + ui] = eMode;
  }
}

Void TComDataCU::setPredModeSubParts( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  for (UInt ui = 0; ui < uiCurrPartNumb; ui++ )
  {
    m_pePredMode[uiAbsPartIdx + ui] = eMode;
  }
}

Void TComDataCU::setQPSubParts( UInt uiQP, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_phQP + uiAbsPartIdx, uiQP, sizeof(Char)*uiCurrPartNumb );
}

Void TComDataCU::setLumaIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhLumaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}


#if HHI_MRG
Void TComDataCU::setMergeFlagSubParts ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_pbMergeFlag + uiAbsPartIdx, bMergeFlag, sizeof(Bool)*uiCurrPartNumb );
}

Void TComDataCU::setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhMergeIndex + uiAbsPartIdx, uiMergeIndex, sizeof(UChar)*uiCurrPartNumb );
}
#endif

Void TComDataCU::setChromIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhChromaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setInterDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = (m_pcPic->getNumPartInCU() >> (uiDepth << 1)) >> 2;
  
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
    case SIZE_2Nx2N:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ << 2 );                      break;
    case SIZE_2NxN:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ << 1 );                      break;
    case SIZE_Nx2N:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );
      memset( m_puhInterDir + uiAbsPartIdx + ( uiCurrPartNumQ << 1 ), uiDir, sizeof(UChar)*uiCurrPartNumQ ); break;
    case SIZE_NxN:
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
    default:
      assert( 0 );
  }
}

Void TComDataCU::setMVPIdxSubParts( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = m_pcPic->getNumPartInCU() >> (uiDepth << 1) >> 2;
  UInt i;
  Int* pi;
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
    case SIZE_2Nx2N:
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 2); i++)
      {
        pi[i] = iMVPIdx;
      }
    }
      break;
    case SIZE_2NxN:
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 1); i++)
      {
        pi[i] = iMVPIdx;
      }
    }
      break;
    case SIZE_Nx2N:
    {
      Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + ( uiCurrPartNumQ << 1 );
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi [i] = iMVPIdx;
        pi2[i] = iMVPIdx;
      }
      break;
    }
    case SIZE_NxN:
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
      }
      break;
    }
    default:
      assert( 0 );
  }
}

Void TComDataCU::setMVPNumSubParts( Int iMVPNum, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = m_pcPic->getNumPartInCU() >> (uiDepth << 1) >> 2;
  UInt i;
  Int* pi;
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
    case SIZE_2Nx2N:
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 2); i++)
      {
        pi[i] = iMVPNum;
      }
    }
      break;
    case SIZE_2NxN:
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 1); i++)
      {
        pi[i] = iMVPNum;
      }
    }
      break;
    case SIZE_Nx2N:
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + ( uiCurrPartNumQ << 1 );
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
    }
      break;
    case SIZE_NxN:
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
    }
      break;
    default:
      assert( 0 );
  }
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
    case SIZE_2Nx2N:
      riWidth = getWidth(0);      riHeight = getHeight(0);      ruiPartAddr = 0;
      break;
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
      assert (0);
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

#if HHI_MRG
Void TComDataCU::getInterMergeCandidates( UInt uiAbsPartIdx, TComMvField cMFieldNeighbours[4], UChar uhInterDirNeighbours[2], UInt& uiNeighbourInfos )
{
  // uiNeighbourInfos (binary):
  // 000: no merge candidate
  // 001: only above is merge candidate
  // 010: only left is merge candidate
  // 011: above and left are different candidates
  // 100: above and left have the same motion parameters. only one merge candidate exists.
  
  // find left and top vectors. take vectors from PUs to the left and top.
  uiNeighbourInfos = 0;
  bool bIsCUAboveInter = false;
  bool bIsCULeftInter = false;
  UInt uiAbovePartIdx;
  UInt uiLeftPartIdx;
  
  // Get left and top vectors
  TComDataCU* pcCUAbove = 0;
  TComDataCU* pcCULeft = 0;
  pcCUAbove = getPUAbove( uiAbovePartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  pcCULeft  = getPULeft( uiLeftPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  
  if ( pcCUAbove && !pcCUAbove->isIntra( uiAbovePartIdx ) )
  {
    bIsCUAboveInter = true;
    uiNeighbourInfos += 1;
  }
  
  if ( pcCULeft && !pcCULeft->isIntra( uiLeftPartIdx ) )
  {
    bIsCULeftInter = true;
    uiNeighbourInfos += 2;
  }
  
  if ( bIsCUAboveInter )
  {
    // get Inter Dir
    uhInterDirNeighbours[0] = pcCUAbove->getInterDir(uiAbovePartIdx);
    // get Mv from Above   
    pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_0, cMFieldNeighbours[0] );
    
    if ( getSlice()->isInterB() )
    {
      pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_1, cMFieldNeighbours[1] );
      
    }
  }
  if ( bIsCULeftInter )
  {
    // get Inter Dir
    uhInterDirNeighbours[1] = pcCULeft->getInterDir(uiLeftPartIdx);
    // get Mv from Left
    pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_0, cMFieldNeighbours[2] );
    
    if ( getSlice()->isInterB() )
    {
      pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_1, cMFieldNeighbours[3] );
    }
  }
  
  // compare left and top vectors
  if ( bIsCUAboveInter && bIsCULeftInter )
  {
    if ( getSlice()->isInterB() )
    {
      if( cMFieldNeighbours[0].getRefIdx() ==  cMFieldNeighbours[2].getRefIdx() && cMFieldNeighbours[0].getMv() == cMFieldNeighbours[2].getMv() &&
         cMFieldNeighbours[1].getRefIdx() ==  cMFieldNeighbours[3].getRefIdx() && cMFieldNeighbours[1].getMv() == cMFieldNeighbours[3].getMv() && 
         uhInterDirNeighbours[0] ==  uhInterDirNeighbours[1])
      {
        uiNeighbourInfos =4;
      }
    }
    else
    {
      if( cMFieldNeighbours[0].getRefIdx() ==  cMFieldNeighbours[2].getRefIdx() && cMFieldNeighbours[0].getMv() == cMFieldNeighbours[2].getMv() && 
         uhInterDirNeighbours[0] ==  uhInterDirNeighbours[1])
      {
        uiNeighbourInfos =4;
      }
    }
  }
}
#endif

AMVP_MODE TComDataCU::getAMVPMode(UInt uiIdx)
{
  return m_pcSlice->getSPS()->getAMVPMode(m_puhDepth[uiIdx]);
}

Void TComDataCU::fillMvpCand ( UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, AMVPInfo* pInfo )
{
  PartSize eCUMode = m_pePartSize[0];
  
  TComMv cMvPred;
  
  pInfo->iN = 0;
  UInt uiIdx;
  
  if (iRefIdx < 0)
    return;
  
  pInfo->m_acMvCand[pInfo->iN++] = cMvPred;   //dummy mv
  
  //-- Get Spatial MV
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  Bool bAdded = false;
  Int iLeftMvIdx = -1;
  Int iAboveMvIdx = -1;
  Int iCornerMvIdx = -1;
  
  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );
  
  //Left
  for ( uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxLB]; uiIdx+= uiNumPartInCUWidth )
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[uiIdx], MD_LEFT );
    if (bAdded && iLeftMvIdx < 0)
    {
      iLeftMvIdx = pInfo->iN-1;
    }
    if (bAdded) break;
  }
  
  bAdded = false;
  //Above
  for ( uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxRT]; uiIdx++ )
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiRasterToZscan[uiIdx], MD_ABOVE);
    if (bAdded && iAboveMvIdx < 0)
    {
      iAboveMvIdx = pInfo->iN-1;
    }
    if (bAdded) break;
  }
  
  bAdded = false;
  //Above Right
  bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
  if (bAdded && iCornerMvIdx < 0)
  {
    iCornerMvIdx = pInfo->iN-1;
  }
  
  //Below Left
  if (!bAdded)
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
  }
  if (bAdded && iCornerMvIdx < 0)
  {
    iCornerMvIdx = pInfo->iN-1;
  }
  
  //Above Left
  if (!bAdded)
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
  }
  
  if (bAdded && iCornerMvIdx < 0)
  {
    iCornerMvIdx = pInfo->iN-1;
  }
  
  assert(iLeftMvIdx!=0 && iAboveMvIdx!=0 && iCornerMvIdx!=0);
  
  if (iLeftMvIdx < 0 && iAboveMvIdx < 0 && iCornerMvIdx < 0)
  {
    //done --> already zero Mv
  }
  else if ( (iLeftMvIdx > 0 && iAboveMvIdx > 0 && iCornerMvIdx > 0) || iLeftMvIdx*iAboveMvIdx*iCornerMvIdx < 0)
  {
    TComMv cLeftMv, cAboveMv, cCornerMv;
    
    if (iLeftMvIdx > 0)
      cLeftMv = pInfo->m_acMvCand[iLeftMvIdx];
    
    if (iAboveMvIdx > 0)
      cAboveMv = pInfo->m_acMvCand[iAboveMvIdx];
    
    if (iCornerMvIdx > 0)
      cCornerMv = pInfo->m_acMvCand[iCornerMvIdx];
    
    pInfo->m_acMvCand[0].setHor ( Median (cLeftMv.getHor(), cAboveMv.getHor(), cCornerMv.getHor()) );
    pInfo->m_acMvCand[0].setVer ( Median (cLeftMv.getVer(), cAboveMv.getVer(), cCornerMv.getVer()) );
  }
  else //only one is available among three candidates
  {
    if (iLeftMvIdx > 0)
    {
      pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iLeftMvIdx];
    }
    else if (iAboveMvIdx > 0)
    {
      pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iAboveMvIdx];
    }
    else if (iCornerMvIdx > 0)
    {
      pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iCornerMvIdx];
    }
    else
    {
      assert(0);
    }
  }
  
  clipMv(pInfo->m_acMvCand[0]);
  
  TComMv cTempMv;
  if ( ( ( ((eCUMode == SIZE_2NxN)) && uiPartIdx == 1 ) ||
        ( ((eCUMode == SIZE_Nx2N)) && uiPartIdx == 0 ) )
      && iLeftMvIdx > 0 )
  {
    cTempMv = pInfo->m_acMvCand[0];
    pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iLeftMvIdx];
    pInfo->m_acMvCand[iLeftMvIdx] = cTempMv;
  }
  
  if (getAMVPMode(uiPartAddr) == AM_NONE)  //Should be optimized later for special cases
  {
    assert(pInfo->iN > 0);
    pInfo->iN = 1;
    return;
  }
  
  // Get Temporal Motion Predictor
#if AMVP_NEIGH_COL
  UInt uiAbsPartAddr = m_uiAbsIdxInLCU + uiPartAddr;
  
  UInt uiColDir = (m_pcSlice->isInterB()? m_pcSlice->getColDir() : 0);
  
  TComDataCU* pcCUColocated = getCUColocated(RefPicList(uiColDir));
  
  RefPicList eColRefPicList = (m_pcSlice->isInterB()? RefPicList(1-uiColDir) : REF_PIC_LIST_0);
  
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
#endif
  // Check No MV Candidate
  xUniqueMVPCand( pInfo );
  return ;
}

Bool TComDataCU::clearMVPCand( TComMv cMvd, AMVPInfo* pInfo )
{
  // only works for multiple candidates
  if (pInfo->iN <= 1)
  {
    return false;
  }
  
  // only works for non-zero mvd case
  if (cMvd.getHor() == 0 && cMvd.getVer() == 0)
  {
    return false;
  }
  
  TComMv  acMv[ AMVP_MAX_NUM_CANDS ];
  Int aiValid [ AMVP_MAX_NUM_CANDS ];
  
  Int iNTmp, i, j;
  
  for ( i=0; i<pInfo->iN; i++ )
  {
    aiValid[i] = 1;
  }
  
  for ( i=0; i<pInfo->iN; i++ )
  {
    TComMv cMvCand = pInfo->m_acMvCand[i] + cMvd;
    
    UInt uiBestBits = xGetMvdBits(cMvd);
    for ( j=0; j<pInfo->iN; j++ )
    {
      if (aiValid[j] && i!=j && xGetMvdBits(cMvCand-pInfo->m_acMvCand[j]) < uiBestBits)
      {
        aiValid[i] = 0;
      }
    }
  }
  
  iNTmp = 0;
  for ( i=0; i<pInfo->iN; i++ )
  {
    if (aiValid[i])
      iNTmp++;
  }
  
  if (iNTmp == pInfo->iN)
  {
    return false;
  }
  
  assert(iNTmp > 0);
  
  iNTmp = 0;
  for ( i=0; i<pInfo->iN; i++ )
  {
    if (aiValid[i])
    {
      acMv[iNTmp++] = pInfo->m_acMvCand[i];
    }
  }
  
  for ( i=0; i<iNTmp; i++ ) pInfo->m_acMvCand[i] = acMv[i];
  pInfo->iN = iNTmp;
  
  return true;
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
  
  rcMv.setHor( Min (iHorMax, Max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( Min (iVerMax, Max (iVerMin, rcMv.getVer())) );
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

Bool TComDataCU::isSkipped( UInt uiPartIdx )
{
  if ( m_pcSlice->isIntra () )
  {
    return false;
  }
  if ( m_pcSlice->isInterP() )
  {
    return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) );
  }
  else //if ( m_pcSlice->isInterB()  )
  {
    return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) && (m_puhInterDir[uiPartIdx] == 3) );
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

Void TComDataCU::xUniqueMVPCand(AMVPInfo* pInfo)
{
  if ( pInfo->iN == 0 )
  {
    pInfo->m_acMvCand[ pInfo->iN++ ].setZero();
    return;
  }
  
  TComMv  acMv[ AMVP_MAX_NUM_CANDS ];
  Int iNTmp, i, j;
  
  // make it be unique
  iNTmp = 0;
  acMv[ iNTmp++ ] = pInfo->m_acMvCand[0];
  for ( i=1; i<pInfo->iN; i++ )
  {
    // BugFix for 1603
    for ( j=iNTmp - 1; j>=0; j-- )
    {
      if ( pInfo->m_acMvCand[i] == acMv[j] ) break;
    }
    if ( j<0 )
    {
      acMv[ iNTmp++ ] = pInfo->m_acMvCand[i];
    }
  }
  for ( i=0; i<iNTmp; i++ ) pInfo->m_acMvCand[i] = acMv[i];
  pInfo->iN = iNTmp;
  
  return ;
}

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

Void TComDataCU::xCalcCuCbf( UChar* puhCbf, UInt uiTrDepth, UInt uiCbfDepth, UInt uiCuDepth )
{
  if ( uiTrDepth == 0 )
    return;
  
  UInt ui, uiNumSig = 0;
  
  UInt uiNumPart  = m_pcPic->getNumPartInCU() >> ( uiCuDepth << 1 );
  UInt uiQNumPart = uiNumPart >> 2;
  
  UInt uiCbfDepth1 = uiCbfDepth + 1;
  if( uiNumPart == 1 )
  {
    if ( ( puhCbf[0] >> uiCbfDepth1 ) & 0x1 )
    {
      uiNumSig = 1;
    }
    puhCbf[0] |= uiNumSig << uiCbfDepth;
    
    return;
  }
  assert( uiQNumPart );
  
  if ( uiCbfDepth < ( uiTrDepth - 1 ) )
  {
    UChar* puhNextCbf = puhCbf;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 ); puhNextCbf += uiQNumPart;
    xCalcCuCbf( puhNextCbf, uiTrDepth, uiCbfDepth1, uiCuDepth+1 );
  }
  
  for ( ui = 0; ui < uiNumPart; ui += uiQNumPart )
  {
    if ( ( puhCbf[ui] >> uiCbfDepth1 ) & 0x1 )
    {
      uiNumSig = 1;
      break;
    }
  }
  
  uiNumSig <<= uiCbfDepth;
  for ( ui = 0; ui < uiNumPart; ui++ )
  {
    puhCbf[ui] |= uiNumSig;
  }
}

