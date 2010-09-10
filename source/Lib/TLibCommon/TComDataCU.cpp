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
#if HHI_AIS
  m_pbLumaIntraFiltFlag= NULL;
#endif
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

#ifdef DCM_PBIC
  m_piICPIdx           = NULL;
  m_piICPNum           = NULL;
#endif

  m_pROTindex          = NULL;
  m_pCIPflag           = NULL;

#if PLANAR_INTRA
  m_piPlanarInfo[0]    = NULL;
  m_piPlanarInfo[1]    = NULL;
  m_piPlanarInfo[2]    = NULL;
  m_piPlanarInfo[3]    = NULL;
#endif

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
#if HHI_AIS
    m_pbLumaIntraFiltFlag= (Bool*  )xMalloc(Bool,   uiNumPartition);
#endif
    m_puhChromaIntraDir  = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_pROTindex          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_pCIPflag           = (UChar* )xMalloc(UChar,  uiNumPartition);

#if PLANAR_INTRA
    m_piPlanarInfo[0]    = (Int*   )xMalloc(Int,    uiNumPartition);
    m_piPlanarInfo[1]    = (Int*   )xMalloc(Int,    uiNumPartition);
    m_piPlanarInfo[2]    = (Int*   )xMalloc(Int,    uiNumPartition);
    m_piPlanarInfo[3]    = (Int*   )xMalloc(Int,    uiNumPartition);
#endif

    m_apiMVPIdx[0]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPIdx[1]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[0]       = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[1]       = (Int*   )xMalloc(Int,  uiNumPartition);

#ifdef DCM_PBIC
    m_piICPIdx           = (Int*   )xMalloc(Int,  uiNumPartition);
    m_piICPNum           = (Int*   )xMalloc(Int,  uiNumPartition);
#endif

    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);

    m_acCUMvField[0].create( uiNumPartition );
    m_acCUMvField[1].create( uiNumPartition );

#ifdef DCM_PBIC
    m_acCUIcField.create( uiNumPartition );
#endif

  }
  else
  {
    m_acCUMvField[0].setNumPartition(uiNumPartition );
    m_acCUMvField[1].setNumPartition(uiNumPartition );

#ifdef DCM_PBIC
    m_acCUIcField.setNumPartition(uiNumPartition );
#endif

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

  if ( m_pcPattern ) { xFree(m_pcPattern); m_pcPattern = NULL; }

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
#if PLANAR_INTRA
    if ( m_piPlanarInfo[0]    ) { xFree(m_piPlanarInfo[0]);     m_piPlanarInfo[0]   = NULL; }
    if ( m_piPlanarInfo[1]    ) { xFree(m_piPlanarInfo[1]);     m_piPlanarInfo[1]   = NULL; }
    if ( m_piPlanarInfo[2]    ) { xFree(m_piPlanarInfo[2]);     m_piPlanarInfo[2]   = NULL; }
    if ( m_piPlanarInfo[3]    ) { xFree(m_piPlanarInfo[3]);     m_piPlanarInfo[3]   = NULL; }
#endif
#if HHI_MRG
    if ( m_pbMergeFlag        ) { xFree(m_pbMergeFlag);         m_pbMergeFlag       = NULL; }
    if ( m_puhMergeIndex      ) { xFree(m_puhMergeIndex);       m_puhMergeIndex     = NULL; }
#endif
    if ( m_puhLumaIntraDir    ) { xFree(m_puhLumaIntraDir);     m_puhLumaIntraDir   = NULL; }
#if HHI_AIS
    if ( m_pbLumaIntraFiltFlag) { xFree(m_pbLumaIntraFiltFlag); m_pbLumaIntraFiltFlag = NULL; }
#endif
    if ( m_puhChromaIntraDir  ) { xFree(m_puhChromaIntraDir);   m_puhChromaIntraDir = NULL; }
    if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);            m_puhTrIdx          = NULL; }
    if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);          m_pcTrCoeffY        = NULL; }
    if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);         m_pcTrCoeffCb       = NULL; }
    if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);         m_pcTrCoeffCr       = NULL; }
    if ( m_pROTindex          ) { xFree(m_pROTindex);           m_pROTindex         = NULL; }
    if ( m_pCIPflag           ) { xFree(m_pCIPflag);            m_pCIPflag          = NULL; }
    if ( m_apiMVPIdx[0]       ) { xFree(m_apiMVPIdx[0]);        m_apiMVPIdx[0]      = NULL; }
    if ( m_apiMVPIdx[1]       ) { xFree(m_apiMVPIdx[1]);        m_apiMVPIdx[1]      = NULL; }
    if ( m_apiMVPNum[0]       ) { xFree(m_apiMVPNum[0]);        m_apiMVPNum[0]      = NULL; }
    if ( m_apiMVPNum[1]       ) { xFree(m_apiMVPNum[1]);        m_apiMVPNum[1]      = NULL; }

#ifdef DCM_PBIC
    if ( m_piICPIdx           ) { xFree(m_piICPIdx);            m_piICPIdx      = NULL; }
    if ( m_piICPNum           ) { xFree(m_piICPNum);            m_piICPNum      = NULL; }
#endif

    m_acCUMvField[0].destroy();
    m_acCUMvField[1].destroy();

#ifdef DCM_PBIC
	  m_acCUIcField.destroy();
#endif

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
#if HHI_AIS || HHI_MRG
  Int iSizeInBool  = sizeof( Bool  ) * m_uiNumPartition;
#endif
#if PLANAR_INTRA
  Int iSizeInInt   = sizeof( Int   ) * m_uiNumPartition;
#endif

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt  );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
#if HHI_AIS
  memset( m_pbLumaIntraFiltFlag,1, iSizeInBool  );
#endif
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

  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );

#if PLANAR_INTRA
  memset( m_piPlanarInfo[0],   0, iSizeInInt );
  memset( m_piPlanarInfo[1],   0, iSizeInInt );
  memset( m_piPlanarInfo[2],   0, iSizeInInt );
  memset( m_piPlanarInfo[3],   0, iSizeInInt );
#endif

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;

    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
#ifdef DCM_PBIC
    m_piICPIdx[ui] = -1;
    m_piICPNum[ui] = -1;
#endif
  }

  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();

#ifdef DCM_PBIC
  m_acCUIcField.clearIcField();
#endif

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
#if HHI_AIS || HHI_MRG
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
#endif
#if PLANAR_INTRA
  Int iSizeInInt   = sizeof( Int   ) * m_uiNumPartition;
#endif

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
#if HHI_AIS
  memset( m_pbLumaIntraFiltFlag,1, iSizeInBool  );
#endif
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );

  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );

#if PLANAR_INTRA
  memset( m_piPlanarInfo[0],   0, iSizeInInt );
  memset( m_piPlanarInfo[1],   0, iSizeInInt );
  memset( m_piPlanarInfo[2],   0, iSizeInInt );
  memset( m_piPlanarInfo[3],   0, iSizeInInt );
#endif

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;

    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
#ifdef DCM_PBIC
    m_piICPIdx[ui] = -1;
    m_piICPNum[ui] = -1;
#endif
  }

  UInt uiTmp = m_puhWidth[0]*m_puhHeight[0];
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );

  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );

  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();

#ifdef DCM_PBIC
  m_acCUIcField.clearIcField();
#endif

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
#if HHI_AIS || HHI_MRG
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;
#endif
#if PLANAR_INTRA
  Int iSizeInInt   = sizeof( Int    ) * m_uiNumPartition;
#endif

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

  memset( m_puiAlfCtrlFlag,     0, iSizeInUInt );
#if HHI_MRG
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
#endif
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
#if HHI_AIS
  memset( m_pbLumaIntraFiltFlag,1, iSizeInBool  );
#endif
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

  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );

#if PLANAR_INTRA
  memset( m_piPlanarInfo[0],   0, iSizeInInt );
  memset( m_piPlanarInfo[1],   0, iSizeInInt );
  memset( m_piPlanarInfo[2],   0, iSizeInInt );
  memset( m_piPlanarInfo[3],   0, iSizeInInt );
#endif

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;

    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
#ifdef DCM_PBIC
    m_piICPIdx[ui] = -1;
    m_piICPNum[ui] = -1;
#endif

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

#ifdef DCM_PBIC
  m_acCUIcField.clearIcField();
#endif

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
#if HHI_AIS
  m_pbLumaIntraFiltFlag = pcCU->getLumaIntraFiltFlag()+ uiPart;
#endif
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

#ifdef DCM_PBIC
  m_piICPIdx = pcCU->getICPIdx() + uiPart;
  m_piICPNum = pcCU->getICPNum() + uiPart;
#endif

  m_pROTindex= pcCU->getROTindex()                + uiPart;
  m_pCIPflag=  pcCU->getCIPflag()                 + uiPart;

#if PLANAR_INTRA
  m_piPlanarInfo[0] = pcCU->getPlanarInfo(PLANAR_FLAG)    + uiPart;
  m_piPlanarInfo[1] = pcCU->getPlanarInfo(PLANAR_DELTAY)  + uiPart;
  m_piPlanarInfo[2] = pcCU->getPlanarInfo(PLANAR_DELTAU)  + uiPart;
  m_piPlanarInfo[3] = pcCU->getPlanarInfo(PLANAR_DELTAV)  + uiPart;
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

  m_pcTrCoeffY=pcCU->getCoeffY()  + uiCoffOffset;

  uiTmp >>= 2;
  uiCoffOffset >>=2;
  m_pcTrCoeffCb=pcCU->getCoeffCb() + uiCoffOffset;
  m_pcTrCoeffCr=pcCU->getCoeffCr() + uiCoffOffset;

  m_acCUMvField[0].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMv()     + uiPart);
  m_acCUMvField[0].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMvd()    + uiPart);
  m_acCUMvField[0].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx() + uiPart);
#ifdef QC_AMVRES
  m_acCUMvField[0].setMVResPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMVRes() + uiPart);
#endif
  m_acCUMvField[1].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMv()     + uiPart);
  m_acCUMvField[1].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMvd()    + uiPart);
  m_acCUMvField[1].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx() + uiPart);
#ifdef QC_AMVRES
  m_acCUMvField[1].setMVResPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMVRes() + uiPart);
#endif

#ifdef DCM_PBIC
  m_acCUIcField.setIcPtr(pcCU->getCUIcField()->getIc()     + uiPart);
  m_acCUIcField.setIcdPtr(pcCU->getCUIcField()->getIcd()    + uiPart);
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

#ifdef DCM_PBIC
  if ( eRefPicList == REF_PIC_LIST_X )
  {
    m_apiMVPIdx[REF_PIC_LIST_0] = pcCU->getMVPIdx(REF_PIC_LIST_0) + uiAbsPartIdx;
    m_apiMVPNum[REF_PIC_LIST_0] = pcCU->getMVPNum(REF_PIC_LIST_0) + uiAbsPartIdx;

    m_apiMVPIdx[REF_PIC_LIST_1] = pcCU->getMVPIdx(REF_PIC_LIST_1) + uiAbsPartIdx;
    m_apiMVPNum[REF_PIC_LIST_1] = pcCU->getMVPNum(REF_PIC_LIST_1) + uiAbsPartIdx;

    m_acCUMvField[REF_PIC_LIST_0].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMv()     + uiAbsPartIdx);
    m_acCUMvField[REF_PIC_LIST_0].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMvd()    + uiAbsPartIdx);
    m_acCUMvField[REF_PIC_LIST_0].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx() + uiAbsPartIdx);
#ifdef QC_AMVRES
    m_acCUMvField[REF_PIC_LIST_0].setMVResPtr(pcCU->getCUMvField(REF_PIC_LIST_0)->getMVRes() + uiAbsPartIdx);
#endif

    m_acCUMvField[REF_PIC_LIST_1].setMvPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMv()     + uiAbsPartIdx);
    m_acCUMvField[REF_PIC_LIST_1].setMvdPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMvd()    + uiAbsPartIdx);
    m_acCUMvField[REF_PIC_LIST_1].setRefIdxPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx() + uiAbsPartIdx);
#ifdef QC_AMVRES
    m_acCUMvField[REF_PIC_LIST_1].setMVResPtr(pcCU->getCUMvField(REF_PIC_LIST_1)->getMVRes() + uiAbsPartIdx);
#endif
  }
  else
  {
    m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
    m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;

    m_acCUMvField[eRefPicList].setMvPtr(pcCU->getCUMvField(eRefPicList)->getMv()     + uiAbsPartIdx);
    m_acCUMvField[eRefPicList].setMvdPtr(pcCU->getCUMvField(eRefPicList)->getMvd()    + uiAbsPartIdx);
    m_acCUMvField[eRefPicList].setRefIdxPtr(pcCU->getCUMvField(eRefPicList)->getRefIdx() + uiAbsPartIdx);
#ifdef QC_AMVRES
    m_acCUMvField[eRefPicList].setMVResPtr(pcCU->getCUMvField(eRefPicList)->getMVRes() + uiAbsPartIdx);
#endif
  }
#else
  m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
  m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;

  m_acCUMvField[eRefPicList].setMvPtr(pcCU->getCUMvField(eRefPicList)->getMv()     + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setMvdPtr(pcCU->getCUMvField(eRefPicList)->getMvd()    + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setRefIdxPtr(pcCU->getCUMvField(eRefPicList)->getRefIdx() + uiAbsPartIdx);
#if HHI_MRG_PU
  m_pbMergeFlag         = pcCU->getMergeFlag()        + uiAbsPartIdx;
  m_puhMergeIndex       = pcCU->getMergeIndex()       + uiAbsPartIdx;
#endif

#ifdef QC_AMVRES
  m_acCUMvField[eRefPicList].setMVResPtr(pcCU->getCUMvField(eRefPicList)->getMVRes() + uiAbsPartIdx);
#endif
#endif

#ifdef DCM_PBIC
  m_piICPIdx = pcCU->getICPIdx() + uiAbsPartIdx;
  m_piICPNum = pcCU->getICPNum() + uiAbsPartIdx;

  m_acCUIcField.setIcPtr(pcCU->getCUIcField()->getIc()     + uiAbsPartIdx);
  m_acCUIcField.setIcdPtr(pcCU->getCUIcField()->getIcd()    + uiAbsPartIdx);
#endif

}

// Copy inter prediction info to the biggest CU
Void TComDataCU::copyInterPredInfoTo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList )
{
  Int iSizeInInt    = sizeof( Int    ) * m_uiNumPartition;
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;

#ifdef DCM_PBIC
  Int iSizeInTComIc = sizeof( TComIc ) * m_uiNumPartition;
#endif

  memcpy( pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx, m_apiMVPIdx[eRefPicList], iSizeInInt );
  memcpy( pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx, m_apiMVPNum[eRefPicList], iSizeInInt );
  memcpy( pcCU->getCUMvField(eRefPicList)->getMv() + uiAbsPartIdx, m_acCUMvField[eRefPicList].getMv(), iSizeInTComMv );
#ifdef QC_AMVRES
  memcpy( pcCU->getCUMvField(eRefPicList)->getMVRes() + uiAbsPartIdx, m_acCUMvField[eRefPicList].getMVRes(), m_uiNumPartition );
#endif

#ifdef DCM_PBIC
  memcpy( pcCU->getICPIdx() + uiAbsPartIdx, m_piICPIdx, iSizeInInt );
  memcpy( pcCU->getICPNum() + uiAbsPartIdx, m_piICPNum, iSizeInInt );
  memcpy( pcCU->getCUIcField()->getIc() + uiAbsPartIdx, m_acCUIcField.getIc(), iSizeInTComIc );
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
  Int iSizeInUInt   = sizeof( UInt  ) * uiNumPartition;
  Int iSizeInInt    = sizeof( Int   ) * uiNumPartition;
#if HHI_AIS || HHI_MRG
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
#if HHI_AIS
  memcpy( m_pbLumaIntraFiltFlag + uiOffset, pcCU->getLumaIntraFiltFlag(), iSizeInBool  );
#endif
  memcpy( m_puhChromaIntraDir   + uiOffset, pcCU->getChromaIntraDir(),    iSizeInUchar );
  memcpy( m_puhInterDir         + uiOffset, pcCU->getInterDir(),          iSizeInUchar );
  memcpy( m_puhTrIdx            + uiOffset, pcCU->getTransformIdx(),      iSizeInUchar );

  memcpy( m_puhCbf[0] + uiOffset, pcCU->getCbf(TEXT_LUMA)    , iSizeInUchar );
  memcpy( m_puhCbf[1] + uiOffset, pcCU->getCbf(TEXT_CHROMA_U), iSizeInUchar );
  memcpy( m_puhCbf[2] + uiOffset, pcCU->getCbf(TEXT_CHROMA_V), iSizeInUchar );

  memcpy( m_puhDepth  + uiOffset, pcCU->getDepth(),  iSizeInUchar );
  memcpy( m_puhWidth  + uiOffset, pcCU->getWidth(),  iSizeInUchar );
  memcpy( m_puhHeight + uiOffset, pcCU->getHeight(), iSizeInUchar );

  memcpy( m_pROTindex  + uiOffset, pcCU->getROTindex(),  iSizeInUchar );
  memcpy( m_pCIPflag   + uiOffset, pcCU->getCIPflag(),   iSizeInUchar );

#if PLANAR_INTRA
  memcpy( m_piPlanarInfo[0] + uiOffset, pcCU->getPlanarInfo(PLANAR_FLAG)    , iSizeInInt );
  memcpy( m_piPlanarInfo[1] + uiOffset, pcCU->getPlanarInfo(PLANAR_DELTAY)  , iSizeInInt );
  memcpy( m_piPlanarInfo[2] + uiOffset, pcCU->getPlanarInfo(PLANAR_DELTAU)  , iSizeInInt );
  memcpy( m_piPlanarInfo[3] + uiOffset, pcCU->getPlanarInfo(PLANAR_DELTAV)  , iSizeInInt );
#endif

  memcpy( m_apiMVPIdx[0] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPIdx[1] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_1), iSizeInInt );
  memcpy( m_apiMVPNum[0] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPNum[1] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_1), iSizeInInt );

#ifdef DCM_PBIC
  memcpy( m_piICPIdx + uiOffset, pcCU->getICPIdx(), iSizeInInt );
  memcpy( m_piICPNum + uiOffset, pcCU->getICPNum(), iSizeInInt );
#endif 

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();

  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);

  m_acCUMvField[0].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_0 ), pcCU->getTotalNumPart(), uiOffset );
  m_acCUMvField[1].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_1 ), pcCU->getTotalNumPart(), uiOffset );

#ifdef DCM_PBIC
  m_acCUIcField.copyFrom( pcCU->getCUIcField(), pcCU->getTotalNumPart(), uiOffset );
#endif

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
#if HHI_AIS || HHI_MRG
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
#if HHI_AIS
  memcpy( rpcCU->getLumaIntraFiltFlag() + m_uiAbsIdxInLCU, m_pbLumaIntraFiltFlag, iSizeInBool  );
#endif
  memcpy( rpcCU->getChromaIntraDir()    + m_uiAbsIdxInLCU, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + m_uiAbsIdxInLCU, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + m_uiAbsIdxInLCU, m_puhTrIdx,            iSizeInUchar );

  memcpy( rpcCU->getCbf(TEXT_LUMA)     + m_uiAbsIdxInLCU, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + m_uiAbsIdxInLCU, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + m_uiAbsIdxInLCU, m_puhCbf[2], iSizeInUchar );

  memcpy( rpcCU->getDepth()  + m_uiAbsIdxInLCU, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + m_uiAbsIdxInLCU, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + m_uiAbsIdxInLCU, m_puhHeight, iSizeInUchar );

  memcpy( rpcCU->getROTindex()  + m_uiAbsIdxInLCU, m_pROTindex,  iSizeInUchar );
  memcpy( rpcCU->getCIPflag()   + m_uiAbsIdxInLCU, m_pCIPflag,   iSizeInUchar );

#if PLANAR_INTRA
  memcpy( rpcCU->getPlanarInfo(PLANAR_FLAG)     + m_uiAbsIdxInLCU, m_piPlanarInfo[0], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAY)   + m_uiAbsIdxInLCU, m_piPlanarInfo[1], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAU)   + m_uiAbsIdxInLCU, m_piPlanarInfo[2], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAV)   + m_uiAbsIdxInLCU, m_piPlanarInfo[3], iSizeInInt );
#endif

  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPNum[1], iSizeInInt );

#ifdef DCM_PBIC
  memcpy( rpcCU->getICPIdx() + m_uiAbsIdxInLCU, m_piICPIdx, iSizeInInt );
  memcpy( rpcCU->getICPNum() + m_uiAbsIdxInLCU, m_piICPNum, iSizeInInt );
#endif 

  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU );

#ifdef DCM_PBIC
  m_acCUIcField.copyTo( rpcCU->getCUIcField(), m_uiAbsIdxInLCU );
#endif

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
#if HHI_AIS || HHI_MRG
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
#if HHI_AIS
  memcpy( rpcCU->getLumaIntraFiltFlag() + uiPartOffset, m_pbLumaIntraFiltFlag, iSizeInBool  );
#endif
  memcpy( rpcCU->getChromaIntraDir()    + uiPartOffset, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + uiPartOffset, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + uiPartOffset, m_puhTrIdx,            iSizeInUchar );

  memcpy( rpcCU->getCbf(TEXT_LUMA)     + uiPartOffset, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + uiPartOffset, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + uiPartOffset, m_puhCbf[2], iSizeInUchar );

  memcpy( rpcCU->getDepth()  + uiPartOffset, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + uiPartOffset, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + uiPartOffset, m_puhHeight, iSizeInUchar );

  memcpy( rpcCU->getROTindex()  + uiPartOffset, m_pROTindex,  iSizeInUchar );
  memcpy( rpcCU->getCIPflag()   + uiPartOffset, m_pCIPflag,   iSizeInUchar );

#if PLANAR_INTRA
  memcpy( rpcCU->getPlanarInfo(PLANAR_FLAG)     + uiPartOffset, m_piPlanarInfo[0], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAY)   + uiPartOffset, m_piPlanarInfo[1], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAU)   + uiPartOffset, m_piPlanarInfo[2], iSizeInInt );
  memcpy( rpcCU->getPlanarInfo(PLANAR_DELTAV)   + uiPartOffset, m_piPlanarInfo[3], iSizeInInt );
#endif

  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPNum[1], iSizeInInt );

#ifdef DCM_PBIC
  memcpy( rpcCU->getICPIdx() + uiPartOffset, m_piICPIdx, iSizeInInt );
  memcpy( rpcCU->getICPNum() + uiPartOffset, m_piICPNum, iSizeInInt );
#endif
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );

#ifdef DCM_PBIC
  m_acCUIcField.copyTo( rpcCU->getCUIcField(), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
#endif

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

Void TComDataCU::setCIPflagSubParts( UChar CIPflag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pCIPflag + uiAbsPartIdx, CIPflag, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setROTindexSubParts( UInt ROTindex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pROTindex + uiAbsPartIdx, ROTindex, sizeof(UChar)*uiCurrPartNumb );
}

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



PartSize TComDataCU::getMostProbablePartSize( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  PartSize    eAbove, eLeft;
  PartSize    eMostProbable;

  // Get left split flag
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  eLeft  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) < m_puhDepth[uiAbsPartIdx] ) ? SIZE_2Nx2N : ( ( pcTempCU->getDepth( uiTempPartIdx ) > m_puhDepth[uiAbsPartIdx] ) ? SIZE_NxN : pcTempCU->getPartitionSize( uiTempPartIdx ) ) ) : SIZE_NONE;

  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  eAbove = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) < m_puhDepth[uiAbsPartIdx] ) ? SIZE_2Nx2N : ( ( pcTempCU->getDepth( uiTempPartIdx ) > m_puhDepth[uiAbsPartIdx] ) ? SIZE_NxN : pcTempCU->getPartitionSize( uiTempPartIdx ) ) ) : SIZE_NONE;

  if( eLeft == SIZE_NONE )
  {
    if( eAbove == SIZE_NONE )
      eMostProbable = SIZE_2Nx2N;
    else
      eMostProbable = eAbove;
  }
  else
  {
    if( eAbove == SIZE_NONE )
      eMostProbable = eLeft;
    else
    {
      if( eLeft > eAbove )
        eMostProbable = eLeft;
      else
        eMostProbable = eAbove;
    }
  }

  if( m_pePredMode[uiAbsPartIdx] == MODE_INTRA )
  {
    if( (eMostProbable == SIZE_2Nx2N) || (eMostProbable == SIZE_NxN) )
    {
      return eMostProbable;
    }
    else
    {
      return SIZE_NxN;
    }
  }
  else
  {
    return eMostProbable;
  }
}

Int TComDataCU::convertIntraDirLuma( UInt uiAbsPartIdx )
{
  Int iMostProbable = getMostProbableIntraDirLuma( uiAbsPartIdx );
  Int iIntraDirLuma = getLumaIntraDir( uiAbsPartIdx );

  if ( iMostProbable == iIntraDirLuma )
  {
    return -1;
  }

  return ( iIntraDirLuma < iMostProbable ) ? iIntraDirLuma : iIntraDirLuma - 1;
}

Int TComDataCU::convertIntraDirLumaAdi(TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int  iMostProbableN;
  Int  iMostProbable  = getMostProbableIntraDirLumaAdi( uiAbsPartIdx );
  Int  iIntraDirLuma  = getLumaIntraDir( uiAbsPartIdx );
  Int  iIntraIdx      = pcCU->getIntraSizeIdx(uiAbsPartIdx);

  if( iMostProbable >= 0 ) iMostProbableN = g_aucIntraModeConvInv[iIntraIdx][iMostProbable];
  else iMostProbableN = iMostProbable;

  if( iMostProbableN >= g_aucIntraModeNum[iIntraIdx]-1 )
    iMostProbableN = 2;

  if ( iMostProbableN == iIntraDirLuma )
  {
    return -1;
  }

  return ( iIntraDirLuma < iMostProbableN ) ? iIntraDirLuma : iIntraDirLuma - 1;
}

Int TComDataCU::getMostProbablePredMode ( UInt   uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftPredMode, iAbovePredMode, iMostProbable;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iLeftPredMode  = pcTempCU ? pcTempCU->getPredictionMode( uiTempPartIdx ) : MODE_NONE;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iAbovePredMode = pcTempCU ? pcTempCU->getPredictionMode( uiTempPartIdx ) : MODE_NONE;

  iMostProbable  = Min( iLeftPredMode, iAbovePredMode );

  if (MODE_NONE == iMostProbable || (MODE_SKIP == iMostProbable && m_pcSlice->isInterP()))
  {
    return MODE_INTER;
  }

  return iMostProbable;
}

Int TComDataCU::convertPredMode         ( UInt   uiAbsPartIdx )
{
  Int  iMostProbable = getMostProbablePredMode( uiAbsPartIdx );
  Int  iPredMode     = (Int)getPredictionMode( uiAbsPartIdx );

  if ( iMostProbable == iPredMode )
  {
    return -1;
  }

  return ( iPredMode < iMostProbable ) ? iPredMode : iPredMode - 1;
}

Int TComDataCU::revertPredMode         ( UInt   uiAbsPartIdx, Int iPredMode )
{
  Int iMostProbable = getMostProbablePredMode( uiAbsPartIdx );

  if ( -1 == iPredMode )
  {
    return iMostProbable;
  }

  if (getSlice()->isInterP())
  {
    if (iMostProbable == MODE_INTER) return MODE_INTRA;
    else //if (uiMostProbable == MODE_INTRA)
      return MODE_INTER;
  }

  return ( iPredMode < iMostProbable ) ? iPredMode : iPredMode + 1;
}

UInt TComDataCU::revertIntraDirLuma( UInt uiAbsPartIdx, Int iIntraDirLuma )
{
  Int iMostProbable = getMostProbableIntraDirLuma( uiAbsPartIdx );

  if ( -1 == iIntraDirLuma )
  {
    return iMostProbable;
  }

  return ( iIntraDirLuma < iMostProbable ) ? iIntraDirLuma : iIntraDirLuma + 1;
}

UInt TComDataCU::revertIntraDirLumaAdi(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iIntraDirLuma )
{
  Int  iMostProbableN;
  Int  iMostProbable  = getMostProbableIntraDirLumaAdi( uiAbsPartIdx );
  Int  iIntraIdx      = pcCU->getIntraSizeIdx(uiAbsPartIdx);

  if (iMostProbable>=0)
    iMostProbableN = g_aucIntraModeConvInv[iIntraIdx][iMostProbable];
  else
    iMostProbableN = iMostProbable;

  if( iMostProbableN >= g_aucIntraModeNum[iIntraIdx]-1 )
    iMostProbableN = 2;

  if ( -1 == iIntraDirLuma )
  {
    return iMostProbableN;
  }

  return ( iIntraDirLuma < iMostProbableN ) ? iIntraDirLuma : iIntraDirLuma + 1;
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

#if UNIFIED_DIRECTIONAL_INTRA
  // Mode conversion process for blocks with different number of available prediction directions
  Int iIdx  = getIntraSizeIdx(uiAbsPartIdx);
  
  if ( iMostProbable >= g_aucIntraModeNumAng[iIdx] ) {
    if ( g_aucIntraModeNumAng[iIdx] == 5 )
      iMostProbable = g_aucAngModeMapping[0][g_aucAngIntraModeOrder[iMostProbable]];
    else
      iMostProbable = g_aucAngModeMapping[1][g_aucAngIntraModeOrder[iMostProbable]]; 
  } 
#endif

  return ( NOT_VALID == iMostProbable ) ? 2 : iMostProbable;
}

Int TComDataCU::getMostProbableIntraDirLumaAdi( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir, iMostProbable;
  Int         iIntraIdx;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;

  if (iLeftIntraDir>=0)
  {
    iIntraIdx= pcTempCU->getIntraSizeIdx(uiTempPartIdx);
    iLeftIntraDir=g_aucIntraModeConv[iIntraIdx][iLeftIntraDir];
  }

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
  if( iAboveIntraDir >= 0 )
  {
    iIntraIdx= pcTempCU->getIntraSizeIdx(uiTempPartIdx);
    iAboveIntraDir = g_aucIntraModeConv[iIntraIdx][iAboveIntraDir];
  }

  iMostProbable  = Min( iLeftIntraDir, iAboveIntraDir );

  return ( NOT_VALID == iMostProbable ) ? 2 : iMostProbable;
}

Void TComDataCU::setCuCbfLuma( UInt uiAbsPartIdx, UInt uiLumaTrMode, UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_LUMA]]     + uiAbsPartIdx, uiLumaTrMode,   0, uiDepth );
}

Void TComDataCU::setCuCbfChromaUV( UInt uiAbsPartIdx, UInt uiChromaTrMode, TextType eTxt,  UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[eTxt]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
}

Void TComDataCU::setCuCbfChroma( UInt uiAbsPartIdx, UInt uiChromaTrMode, UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_U]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_V]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
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

#if HHI_RQT
UInt TComDataCU::getCtxQtCbf( UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
#if HHI_RQT_CHROMA_CBF_MOD
  if( getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && eType != TEXT_LUMA )
  {
    return uiTrDepth;
  }
#endif
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

#if HHI_RQT_ROOT
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
#endif
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

UInt TComDataCU::getCtxCIPFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getCIPflag( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getCIPflag( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;

  return uiCtx;
}

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

UInt TComDataCU::getCtxTransIdx( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iTempTrMode;
  UInt        uiCtx = 0;
  UInt        uiMinTrDepth = m_pcSlice->getSPS()->getMinTrDepth();

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
  iTempTrMode = pcTempCU->getDepth( uiTempPartIdx ) + pcTempCU->getTransformIdx( uiTempPartIdx ) - getDepth( uiAbsPartIdx ) - uiMinTrDepth;
  uiCtx  = ( 0 <= iTempTrMode && iTempTrMode < 2 ) ? 1 : 0;    // 2 is max trmode
  }

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  if ( pcTempCU )
  {
  iTempTrMode = pcTempCU->getDepth( uiTempPartIdx ) + pcTempCU->getTransformIdx( uiTempPartIdx ) - getDepth( uiAbsPartIdx ) - uiMinTrDepth;
  uiCtx += ( 0 <= iTempTrMode && iTempTrMode < 2 ) ? 1 : 0;    // 2 is max trmode
  }

  return uiCtx;
}



#if HHI_AIS

#if ANG_INTRA
UInt TComDataCU::getCtxIntraFiltFlagLumaAng( UInt uiAbsPartIdx )
{
  UInt uiIntraDir    = (UInt)getLumaIntraDir( uiAbsPartIdx );
  UInt uiCtx         = 0;

  if( uiIntraDir < 2 )        // vert., hor.
    uiCtx = uiIntraDir;
  else if( uiIntraDir == 2 )  // DC
    assert(0);
  else                        // angular
    uiCtx = 3;

  return uiCtx;
}
#endif

UInt TComDataCU::getCtxIntraFiltFlagLuma( UInt uiAbsPartIdx )
{
  UInt uiIntraDir    = (UInt)getLumaIntraDir( uiAbsPartIdx );
  Int  iIntraSizeIdx =       getIntraSizeIdx( uiAbsPartIdx );
  UInt uiCtx         = 0;
  // mode mapping
  uiIntraDir = g_aucIntraModeOrder[ iIntraSizeIdx ][ uiIntraDir ];
  if( uiIntraDir < 2 )        // vert., hor.
  {
    uiCtx = uiIntraDir;
  }
  else if( uiIntraDir == 2 )  // DC
  {
    assert(0);
  }              
  else if( uiIntraDir == 3 )  // planar
  {
    uiCtx = 2;
  }
  else if( uiIntraDir < 32 )  // angular
  {
    uiCtx = 3;
  }
  else                        // bi-linear
  {
    uiCtx = 4;
  }
  return uiCtx;
}
#endif

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

#if HHI_AIS
Void TComDataCU::setLumaIntraFiltFlagSubParts( Bool bFlag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pbLumaIntraFiltFlag + uiAbsPartIdx, bFlag, sizeof(Bool)*uiCurrPartNumb );
}
#endif

#if HHI_MRG_PU
Void TComDataCU::setSubPartUChar( UInt uiParameter, UChar* puhBaseLCU, UInt uiCUAddr, UInt uiCUDepth, UInt uiPUIdx )
{
  UInt uiCurrPartNumQ = (m_pcPic->getNumPartInCU() >> (uiCUDepth << 1)) >> 2;
  switch ( m_pePartSize[ uiCUAddr ] )
  {
  case SIZE_2Nx2N:
    memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*uiCurrPartNumQ << 2 );                      break;
  case SIZE_2NxN:
    memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*uiCurrPartNumQ << 1 );                      break;
  case SIZE_Nx2N:
    memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*uiCurrPartNumQ );
    memset( puhBaseLCU + uiCUAddr + ( uiCurrPartNumQ << 1 ), uiParameter, sizeof(UChar)*uiCurrPartNumQ ); break;
  case SIZE_NxN:
    memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*uiCurrPartNumQ );                           break;
  case SIZE_2NxnU:
    {
      memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      if( uiPUIdx == 0 )
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      else
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
        memset( puhBaseLCU + uiCUAddr + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) ), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      else
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>1) );
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      break;
    }
  case SIZE_nLx2N:
    {
      memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      if( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1), uiParameter, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiParameter, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      break;
    }
  default:
    assert( 0 );
  }
}

Void TComDataCU::setSubPartBool( Bool bParameter, Bool* pbBaseLCU, UInt uiCUAddr, UInt uiCUDepth, UInt uiPUIdx )
{
  UInt uiQuaterCUPartNum = (m_pcPic->getNumPartInCU() >> (uiCUDepth << 1)) >> 2;
  switch ( m_pePartSize[ uiCUAddr ] )
  {
  case SIZE_2Nx2N:
    memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*uiQuaterCUPartNum << 2 );                      break;
  case SIZE_2NxN:
    memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*uiQuaterCUPartNum << 1 );                      break;
  case SIZE_Nx2N:
    memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*uiQuaterCUPartNum );
    memset( pbBaseLCU + uiCUAddr + ( uiQuaterCUPartNum << 1 ), bParameter, sizeof(Bool)*uiQuaterCUPartNum ); break;
  case SIZE_NxN:
    memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*uiQuaterCUPartNum );                           break;
  case SIZE_2NxnU:
    {
      memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>1) );
      if( uiPUIdx == 0 )
        memset( pbBaseLCU + uiCUAddr + uiQuaterCUPartNum, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>1) );
      else
        memset( pbBaseLCU + uiCUAddr + uiQuaterCUPartNum, bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>1) + (uiQuaterCUPartNum<<1) ) );
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPUIdx == 0 )
      {
        memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>1) + (uiQuaterCUPartNum<<1) ) );
        memset( pbBaseLCU + uiCUAddr + ( uiQuaterCUPartNum + (uiQuaterCUPartNum<<1) ), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>1) );
      }
      else
      {
        memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>1) );
        memset( pbBaseLCU + uiCUAddr + uiQuaterCUPartNum, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>1) );
      }
      break;
    }
  case SIZE_nLx2N:
    {
      memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
      memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
      if( uiPUIdx == 0 )
      {
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1) + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
      }
      else
      {
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>2) + uiQuaterCUPartNum ) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1) + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>2) + uiQuaterCUPartNum ) );
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPUIdx == 0 )
      {
        memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>2) + uiQuaterCUPartNum ) );
        memset( pbBaseLCU + uiCUAddr + uiQuaterCUPartNum + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1), bParameter, sizeof(Bool)*( (uiQuaterCUPartNum>>2) + uiQuaterCUPartNum ) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1) + uiQuaterCUPartNum + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
      }
      else
      {
        memset( pbBaseLCU + uiCUAddr, bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
        memset( pbBaseLCU + uiCUAddr + (uiQuaterCUPartNum<<1) + (uiQuaterCUPartNum>>1), bParameter, sizeof(Bool)*(uiQuaterCUPartNum>>2) );
      }
      break;
    }
  default:
    assert( 0 );
  }
}
#endif

#if HHI_MRG
#if HHI_MRG_PU
Void TComDataCU::setMergeFlagSubParts ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPartBool( bMergeFlag, m_pbMergeFlag, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPartUChar( uiMergeIndex, m_puhMergeIndex, uiAbsPartIdx, uiDepth, uiPartIdx );
}

#else

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
#endif

#if ANG_INTRA
Bool TComDataCU::angIntraEnabledPredPart( UInt uiAbsPartIdx )
{
#if (ANG_INTRA == 2)
  return true;
#else
  // Return true for 8x8 blocks
  if (getIntraSizeIdx(uiAbsPartIdx) == 2 )
    return true;
  else
    return false;
#endif
}
#endif

Void TComDataCU::setChromIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_puhChromaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

#if HHI_MRG_PU

Void TComDataCU::setInterDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPartUChar( uiDir, m_puhInterDir, uiAbsPartIdx, uiDepth, uiPartIdx );
}

#else
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
  case SIZE_2NxnU:
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      if( uiPartIdx == 0 )
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      else
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ) );
        memset( m_puhInterDir + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) ), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>1) );
      }
      break;
    }
  case SIZE_nLx2N:
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      else
      {
        memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
        memset( m_puhInterDir + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1), uiDir, sizeof(UChar)*(uiCurrPartNumQ>>2) );
      }
      break;
    }
  default:
    assert( 0 );
  }
}
#endif

Void TComDataCU::setMVPIdxSubParts( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = m_pcPic->getNumPartInCU() >> (uiDepth << 1) >> 2;
  Int i;
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
  case SIZE_2NxnU:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }
      }
      else
      {
        pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iMVPIdx;
        }

        pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iMVPIdx;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iMVPIdx;
        }
        pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) );
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iMVPIdx;
        }
      }
      else
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi3 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi4 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);

        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
          pi3[i] = iMVPIdx;
          pi4[i] = iMVPIdx;
        }
      }
      else
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }

        pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }

        pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
        }
      }
      else
      {
             pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi3 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi4 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPIdx;
          pi2[i] = iMVPIdx;
          pi3[i] = iMVPIdx;
          pi4[i] = iMVPIdx;
        }
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
  Int i;
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
  case SIZE_2NxnU:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }
      }
      else
      {
        pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iMVPNum;
        }

        pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iMVPNum;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iMVPNum;
        }
        pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) );
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iMVPNum;
        }
      }
      else
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi3 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi4 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);

        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
          pi3[i] = iMVPNum;
          pi4[i] = iMVPNum;
        }
      }
      else
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }

        pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }

        pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
        }
      }
      else
      {
             pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
        Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi3 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi4 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iMVPNum;
          pi2[i] = iMVPNum;
          pi3[i] = iMVPNum;
          pi4[i] = iMVPNum;
        }
      }
      break;
    }
  default:
    assert( 0 );
  }
}

#ifdef DCM_PBIC
Void TComDataCU::setICPIdxSubParts( Int iICPIdx, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = m_pcPic->getNumPartInCU() >> (uiDepth << 1) >> 2;
  Int i;
  Int* pi;
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
  case SIZE_2Nx2N:
    {
      pi = m_piICPIdx + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 2); i++)
      {
        pi[i] = iICPIdx;
      }
   }
  break;
  case SIZE_2NxN:
    {
      pi = m_piICPIdx + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 1); i++)
      {
        pi[i] = iICPIdx;
      }
   }
  break;
  case SIZE_Nx2N:
    {
      Int* pi2 = m_piICPIdx + uiAbsPartIdx + ( uiCurrPartNumQ << 1 );
      pi = m_piICPIdx + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi [i] = iICPIdx;
        pi2[i] = iICPIdx;
      }
      break;
    }
  case SIZE_NxN:
    {
      pi = m_piICPIdx + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iICPIdx;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }
      }
      else
      {
        pi = m_piICPIdx + uiAbsPartIdx;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iICPIdx;
        }

        pi = m_piICPIdx + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iICPIdx;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        pi = m_piICPIdx + uiAbsPartIdx;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iICPIdx;
        }
        pi = m_piICPIdx + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) );
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iICPIdx;
        }
      }
      else
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi3 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi4 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);

        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
          pi3[i] = iICPIdx;
          pi4[i] = iICPIdx;
        }
      }
      else
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }

        pi  = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }

        pi  = m_piICPIdx + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
        }
      }
      else
      {
             pi  = m_piICPIdx + uiAbsPartIdx;
        Int* pi2 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi3 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi4 = m_piICPIdx + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPIdx;
          pi2[i] = iICPIdx;
          pi3[i] = iICPIdx;
          pi4[i] = iICPIdx;
        }
      }
      break;
    }
  default:
    assert( 0 );
  }
}

Void TComDataCU::setICPNumSubParts( Int iICPNum, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumQ = m_pcPic->getNumPartInCU() >> (uiDepth << 1) >> 2;
  Int i;
  Int* pi;
  switch ( m_pePartSize[ uiAbsPartIdx ] )
  {
  case SIZE_2Nx2N:
    {
      pi = m_piICPNum + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 2); i++)
      {
        pi[i] = iICPNum;
      }
   }
  break;
  case SIZE_2NxN:
    {
      pi = m_piICPNum + uiAbsPartIdx;
      for (i = 0; i < (uiCurrPartNumQ << 1); i++)
      {
        pi[i] = iICPNum;
      }
   }
  break;
  case SIZE_Nx2N:
    {
      Int* pi2 = m_piICPNum + uiAbsPartIdx + ( uiCurrPartNumQ << 1 );
      pi = m_piICPNum + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi [i] = iICPNum;
        pi2[i] = iICPNum;
      }
      break;
    }
  case SIZE_NxN:
    {
      pi = m_piICPNum + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iICPNum;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }
      }
      else
      {
        pi = m_piICPNum + uiAbsPartIdx;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iICPNum;
        }

        pi = m_piICPNum + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iICPNum;
        }
      }
      break;
    }
  case SIZE_2NxnD:
    {
      if( uiPartIdx == 0 )
      {
        pi = m_piICPNum + uiAbsPartIdx;
        for (i = 0; i < ( (uiCurrPartNumQ>>1) + (uiCurrPartNumQ<<1) ); i++)
        {
          pi[i] = iICPNum;
        }
        pi = m_piICPNum + uiAbsPartIdx + ( uiCurrPartNumQ + (uiCurrPartNumQ<<1) );
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi[i] = iICPNum;
        }
      }
      else
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + uiCurrPartNumQ;
        for (i = 0; i < (uiCurrPartNumQ>>1); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }
      }
      break;
    }
  case SIZE_nLx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi3 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi4 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);

        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
          pi3[i] = iICPNum;
          pi4[i] = iICPNum;
        }
      }
      else
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }

        pi  = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }
      }
      break;
    }
  case SIZE_nRx2N:
    {
      if( uiPartIdx == 0 )
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        for (i = 0; i < ( (uiCurrPartNumQ>>2) + uiCurrPartNumQ ); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }

        pi  = m_piICPNum + uiAbsPartIdx + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1) + uiCurrPartNumQ + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
        }
      }
      else
      {
             pi  = m_piICPNum + uiAbsPartIdx;
        Int* pi2 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ>>1);
        Int* pi3 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1);
        Int* pi4 = m_piICPNum + uiAbsPartIdx + (uiCurrPartNumQ<<1) + (uiCurrPartNumQ>>1);
        for (i = 0; i < (uiCurrPartNumQ>>2); i++)
        {
          pi [i] = iICPNum;
          pi2[i] = iICPNum;
          pi3[i] = iICPNum;
          pi4[i] = iICPNum;
        }
      }
      break;
    }
  default:
    assert( 0 );
  }
}
#endif //DCM_PBIC

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
  case SIZE_2NxnU:    iNumPart = 2; break;
  case SIZE_2NxnD:    iNumPart = 2; break;
  case SIZE_nLx2N:    iNumPart = 2; break;
  case SIZE_nRx2N:    iNumPart = 2; break;
  default:            assert (0);   break;
  }

  return  iNumPart;
}

Void TComDataCU::getPartIndexAndSize( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight )
{
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N: riWidth = getWidth(0);      riHeight = getHeight(0);      ruiPartAddr = 0;                                             break;
    case SIZE_2NxN:  riWidth = getWidth(0);      riHeight = getHeight(0) >> 1; ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1; break;
    case SIZE_Nx2N:  riWidth = getWidth(0) >> 1; riHeight = getHeight(0);      ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2; break;
    case SIZE_NxN:   riWidth = getWidth(0) >> 1; riHeight = getHeight(0) >> 1; ruiPartAddr = ( m_uiNumPartition >> 2 ) * uiPartIdx;         break;
    case SIZE_2NxnU:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  getHeight(0) >> 2 : ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 ) : getHeight(0) >> 2;
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 3);
      break;
    case SIZE_nLx2N:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) >> 2 : ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 );
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 4;
      break;
    case SIZE_nRx2N:
      riWidth     = ( uiPartIdx == 0 ) ? ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 ) : getWidth(0) >> 2;
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default: assert (0); break;
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

#ifdef DCM_PBIC
Void TComDataCU::getIc ( TComDataCU* pcCU, UInt uiAbsPartIdx, TComIc& rcIc )
{
  if ( pcCU == NULL )  // OUT OF BOUNDARY
  {
    rcIc.reset( );
    return;
  }

  rcIc = pcCU->getCUIcField()->getIc(uiAbsPartIdx);
}
#endif

Void TComDataCU::deriveLeftRightTopIdx ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU;
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:                                                                                                                                break;
    case SIZE_2NxN:  ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1; ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;  break;
    case SIZE_Nx2N:  ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2; ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 2;  break;
    case SIZE_NxN:   ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * uiPartIdx;         ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 ); break;

    case SIZE_2NxnU:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      break;
    case SIZE_nLx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 4;
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      break;
    case SIZE_nRx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 4;
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
    case SIZE_2Nx2N:  ruiPartIdxLB += m_uiNumPartition >> 1;    break;
    case SIZE_2NxN:  ruiPartIdxLB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;   break;
    case SIZE_Nx2N:  ruiPartIdxLB += ( uiPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;   break;
    case SIZE_NxN:   ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiPartIdx;   break;

    case SIZE_2NxnU:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? -((Int)m_uiNumPartition >> 3) : m_uiNumPartition >> 1;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3): m_uiNumPartition >> 1;
      break;
    case SIZE_nLx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 4);
      break;
    case SIZE_nRx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
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
#ifdef DCM_PBIC
Void TComDataCU::getInterMergeCandidates( UInt uiAbsPartIdx, TComMvField cMFieldNeighbours[4], TComIc cIcNeighbours[2],UChar uhInterDirNeighbours[2], UInt& uiNeighbourInfos )
#else
Void TComDataCU::getInterMergeCandidates( UInt uiAbsPartIdx, TComMvField cMFieldNeighbours[4], UChar uhInterDirNeighbours[2], UInt& uiNeighbourInfos )
#endif
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

#ifdef DCM_PBIC
	  // get Ic from Above
		pcCUAbove->getIc ( pcCUAbove, uiAbovePartIdx, cIcNeighbours[0] );
#endif

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

#ifdef DCM_PBIC
	  // get Ic from Left
	  pcCULeft->getIc ( pcCULeft, uiLeftPartIdx, cIcNeighbours[1] );
#endif 

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
         uhInterDirNeighbours[0] ==  uhInterDirNeighbours[1]
#ifdef DCM_PBIC 
         && cIcNeighbours[0].isequalIcParam(cIcNeighbours[1])
#endif
        )
      {
        uiNeighbourInfos =4;
      }
    }
    else
    {
      if( cMFieldNeighbours[0].getRefIdx() ==  cMFieldNeighbours[2].getRefIdx() && cMFieldNeighbours[0].getMv() == cMFieldNeighbours[2].getMv() && 
        uhInterDirNeighbours[0] ==  uhInterDirNeighbours[1]
#ifdef DCM_PBIC 
        && cIcNeighbours[0].isequalIcParam(cIcNeighbours[1])
#endif
        )
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

#if HHI_IMVP
Void TComDataCU::getYThresXPredLists( UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, Int& riYPred, std::vector<Int>& racYThresList, std::vector<Int>& racXPredList )
{
  racYThresList.clear();
  racXPredList .clear();
  std::vector <TComMv> acMvPredCandYList;
  std::vector <TComMv> acMvPredCandXList;
  getMvPredCandLstStd( eRefPicList, uiPartIdx, iRefIdx, acMvPredCandYList );
  getMvPredCandLstExt( eRefPicList, uiPartIdx, iRefIdx, acMvPredCandXList );
  setYThresXPredLists(  acMvPredCandXList, racYThresList, racXPredList );
  riYPred = getMvPredYCompInd  ( acMvPredCandYList );
}

bool TComDataCU::insertNeighbourMvs( RefPicList eRefPicList, Int iRefIdx, TComDataCU* pcCUNeighbour, UInt uiIdxNeighbour, std::vector <TComMv>& racNeighbourMvs, Bool  bAvoidMultipleInsertion )
{
  if ( pcCUNeighbour!= NULL && !pcCUNeighbour->isIntra( uiIdxNeighbour ) &&  m_pcSlice->isEqualRef( eRefPicList, pcCUNeighbour->getCUMvField(eRefPicList)->getRefIdx(uiIdxNeighbour), iRefIdx ) )
  {
     TComMv cMv = pcCUNeighbour->getCUMvField(eRefPicList)->getMv(uiIdxNeighbour);
     clipMv(cMv);
     bool bInsert = true;
     // avoid multiple insertion
     if( bAvoidMultipleInsertion )
     {
       for( UInt uiIdx = 0; uiIdx < UInt( racNeighbourMvs.size() ); uiIdx++ )
       {
         if( racNeighbourMvs[ uiIdx ] == cMv )
         {
           bInsert = false;
           break;
         }
       }
     }
     if( bInsert )
     {
       racNeighbourMvs.push_back( cMv );
     }
     return true; 
  }
  return false;
}

Void TComDataCU::getMvPredCandLstStd( RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, std::vector <TComMv>& rcMvPredCandLst )
{
  PartSize eCUMode = m_pePartSize[0];
  rcMvPredCandLst.clear();
  UInt uiNeighbourIdx = 0;
  TComDataCU* pcCUNeighbour = 0;
 
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;

  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );
 
  // left
  pcCUNeighbour =       getPULeft( uiNeighbourIdx, uiPartIdxLT );
  insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, false );

  // above
  pcCUNeighbour =       getPUAbove( uiNeighbourIdx, uiPartIdxLT );
  insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, false );

  // above right
  pcCUNeighbour =       getPUAboveRight( uiNeighbourIdx, uiPartIdxRT );
  if ( !insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, false ) )
  {
    // above left
    pcCUNeighbour =       getPUAboveLeft( uiNeighbourIdx, uiPartIdxLT );
    insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, false );
  }
  //----- check for empty list -----
  if( rcMvPredCandLst.empty() )
  {
    rcMvPredCandLst.push_back( TComMv( 0, 0 ) );
  }
}

Void TComDataCU::insertCollocated( RefPicList eRefPicList, Int uiPartIdx, Int iRefIdx, std::vector <TComMv>& racNeighbourMvs)
{
  Int iRoiWidth = 0; 
  Int iRoiHeight = 0;
  UInt uiPartAddr = 0;
  getPartIndexAndSize( uiPartIdx, uiPartAddr, iRoiWidth, iRoiHeight );
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

    for( UInt uiIdx = 0; uiIdx < UInt( racNeighbourMvs.size() ); uiIdx++ )
    {
      if( racNeighbourMvs[ uiIdx ] == cMv )
      {
        return;
      }
    }
    racNeighbourMvs.push_back( cMv );
  }
}

Void TComDataCU::getMvPredCandLstExt( RefPicList eRefPicList,  UInt uiPartIdx,  Int iRefIdx, std::vector <TComMv>& rcMvPredCandLst )
{
  rcMvPredCandLst.clear();
  UInt uiNeighbourIdx;
  TComDataCU* pcCUNeighbour = 0;

  PartSize eCUMode = m_pePartSize[0];


  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
 
  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );
  
  //Left 
  for ( UInt uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxLB]; uiIdx+= uiNumPartInCUWidth )
  {
    pcCUNeighbour =       getPULeft( uiNeighbourIdx,g_auiRasterToZscan[uiIdx] );
    insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, true );
  }
  // Top Left
  pcCUNeighbour =       getPUAboveLeft( uiNeighbourIdx,uiPartIdxLT );
  insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, true );
  // Top 
  for ( UInt uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxRT]; uiIdx++ )
  {
    pcCUNeighbour =       getPUAbove( uiNeighbourIdx,g_auiRasterToZscan[uiIdx] );
    insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, true );
  }
  // Top Right
  pcCUNeighbour =       getPUAboveRight( uiNeighbourIdx,uiPartIdxRT );
  insertNeighbourMvs( eRefPicList, iRefIdx, pcCUNeighbour, uiNeighbourIdx, rcMvPredCandLst, true );

#if 1
  // Collocated
  insertCollocated( eRefPicList, uiPartIdx, iRefIdx, rcMvPredCandLst);
#endif

  //----- check for empty list -----
  if( rcMvPredCandLst.empty() )
  {
    rcMvPredCandLst.push_back( TComMv( 0, 0 ) );
  }
}

Void TComDataCU::setYThresXPredLists(  std::vector <TComMv>& rcMvPredCandLst, std::vector<Int>& rcYThresLst, std::vector<Int>& rcXPredLst )
{
  //===== get sorted list of y-components (without multiple occurrences) =====
  std::vector<Int>  aiSortedYValues;
  for( UInt uiCandIdx   = 0; uiCandIdx < UInt( rcMvPredCandLst.size() ); uiCandIdx++ )
  {
    const Int iYComp  = rcMvPredCandLst[ uiCandIdx ].getVer();
    bool      bInsert = true;
    for( UInt uiIdx   = 0; uiIdx < UInt( aiSortedYValues.size() ); uiIdx++ )
    {
      if( aiSortedYValues[ uiIdx ] == iYComp )
      {
        bInsert = false;
        break;
      }
    }
    if( bInsert )
    {
      aiSortedYValues.push_back( iYComp );
    }
  }
  sort( aiSortedYValues.begin(), aiSortedYValues.end() );

  //===== set x-prediction for sorted values =====
  for( UInt uiIdx = 0; uiIdx < UInt( aiSortedYValues.size() ); uiIdx++ )
  {
    std::vector<int>  aiX;
    const int         iYComp  = aiSortedYValues[ uiIdx ];
    //--- create list of x-components for given y-component ---
    for( UInt uiCandIdx = 0; uiCandIdx < UInt( rcMvPredCandLst.size() ); uiCandIdx++ )
    {
      TComMv& rcCandMv = rcMvPredCandLst[ uiCandIdx ];
      if( iYComp == rcCandMv.getVer() )
      {
        aiX.push_back( rcCandMv.getHor() );
      }
    }
    //--- derive predictor for x ---
    int iMvPredX = 0;
    if ( aiX.size() == 1 )
    {
      iMvPredX  = aiX[0];
    }
    else if( aiX.size() == 2 )
    {
      iMvPredX  = ( aiX[0] + aiX[1] + 1 ) >> 1;
    }
    else
    {
      sort( aiX.begin(), aiX.end() );
      UInt uiId = UInt( aiX.size() ) >> 1;
      iMvPredX  = aiX[ uiId ];
    }
    //--- insert x-component predictor ---
    rcXPredLst.push_back( iMvPredX );
  }
  //===== determine decisions thresholds for y-component ----
  UInt uiSizeMin1 = UInt( aiSortedYValues.size() ) - 1;
  for( UInt uiIdx = 0; uiIdx < uiSizeMin1; uiIdx++ )
  {
    int iYThreshold = ( aiSortedYValues[ uiIdx ] + aiSortedYValues[ uiIdx + 1 ] ) >> 1;
    rcYThresLst.push_back( iYThreshold );
  }
  rcYThresLst.push_back( MAX_INT );
}


Int TComDataCU::getMvPredYCompInd( std::vector <TComMv>& rcMvPredCandLst )
{

  if(  rcMvPredCandLst.size() == 1 )
  {
    return rcMvPredCandLst[0].getVer();
  }
  if ( rcMvPredCandLst.size() == 2 )
  {
    return ( rcMvPredCandLst[0].getVer() + rcMvPredCandLst[1].getVer() + 1 ) >> 1;
  }
  const UInt uiNumEntries = UInt( rcMvPredCandLst.size() );
  std::vector<int>          aiY ( rcMvPredCandLst.size() );
  for( UInt ui = 0; ui < uiNumEntries; ui++ )
  {
    aiY[ui] = rcMvPredCandLst[ui].getVer();
  }
  sort ( aiY.begin(), aiY.end() );
  return aiY[ uiNumEntries >> 1 ];
}


Void TComDataCU::getMvPredYInd(  RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, Int& riMvPredY )
{
  std::vector<TComMv> acMvPredCandList;
  getMvPredCandLstStd( eRefPicList, uiPartIdx, iRefIdx, acMvPredCandList );
  riMvPredY = getMvPredYCompInd( acMvPredCandList );
}

Void TComDataCU::getMvPredXDep( RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx, const Int iMvY, Int& riMvPredX )
{
  std::vector<TComMv> acMvPredCandList;
  std::vector<Int>          acYThresList;
  std::vector<Int>          acXPredList;
  getMvPredCandLstExt( eRefPicList, uiPartIdx, iRefIdx, acMvPredCandList );
  setYThresXPredLists(  acMvPredCandList, acYThresList, acXPredList );
  riMvPredX = getMvPredXCompDep( acYThresList, acXPredList, iMvY );
}

Int TComDataCU::getMvPredXCompDep( const std::vector<Int>& rcYThresLst, const std::vector<Int>& rcXPredLst, const Int iYComponent )
{
  const UInt uiSM1 = UInt( rcYThresLst.size() ) - 1;
  for(  UInt uiIdx = 0; uiIdx < uiSM1; uiIdx++ )
  {
    if( iYComponent <= rcYThresLst[ uiIdx ] )
    {
      return rcXPredLst[ uiIdx ];
    }
  }
  return rcXPredLst[ uiSM1 ];
}
#endif

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
  if ( ( ( ((eCUMode == SIZE_2NxN) || (eCUMode == SIZE_2NxnU) || (eCUMode == SIZE_2NxnD)) && uiPartIdx == 1 ) ||
    ( ((eCUMode == SIZE_Nx2N) || (eCUMode == SIZE_nLx2N) || (eCUMode == SIZE_nRx2N)) && uiPartIdx == 0 ) )
    && iLeftMvIdx > 0 )
  {
    cTempMv = pInfo->m_acMvCand[0];
    pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iLeftMvIdx];
    pInfo->m_acMvCand[iLeftMvIdx] = cTempMv;
  }
  else if ( ( ((eCUMode == SIZE_2NxN) || (eCUMode == SIZE_2NxnU) || (eCUMode == SIZE_2NxnD)) && uiPartIdx == 0 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldB.getRefIdx(), iRefIdx) )
    && iAboveMvIdx > 0 )
  {
    cTempMv = pInfo->m_acMvCand[0];
    pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iAboveMvIdx];
    pInfo->m_acMvCand[iAboveMvIdx] = cTempMv;
  }
  else if ( ( ((eCUMode == SIZE_Nx2N) || (eCUMode == SIZE_nLx2N) || (eCUMode == SIZE_nRx2N)) && uiPartIdx == 1 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldC.getRefIdx(), iRefIdx))
    && iCornerMvIdx > 0 )
  {
    cTempMv = pInfo->m_acMvCand[0];
    pInfo->m_acMvCand[0] = pInfo->m_acMvCand[iCornerMvIdx];
    pInfo->m_acMvCand[iCornerMvIdx] = cTempMv;
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

#ifdef QC_AMVRES
#if HHI_IMVP
Void TComDataCU::xUniqueMVPCand_one_fourth(AMVPInfo* pInfo, Int uiPartIdx)
#else
Void TComDataCU::xUniqueMVPCand_one_fourth(AMVPInfo* pInfo)
#endif
{
    if ( pInfo->iN == 0 )
	{
		pInfo->m_acMvCand[ pInfo->iN++ ].setZero();
		return;
	}

	TComMv	acMv[ AMVP_MAX_NUM_CANDS ];
	TComMv	acMv_round[ AMVP_MAX_NUM_CANDS ];
	Int	iNTmp, i, j;

	// make it be unique
	iNTmp = 0;
	acMv      [iNTmp] = pInfo->m_acMvCand[0];
    acMv_round[iNTmp] = pInfo->m_acMvCand[0];
	acMv_round[iNTmp ++].scale_down();

	for ( i=1; i<pInfo->iN; i++ )
	{
    // BugFix for 1603
		for ( j=iNTmp - 1; j>=0; j-- )
		{
			TComMv predMV = pInfo->m_acMvCand[i];
			predMV.scale_down();
#if HHI_IMVP
			if ( getSlice()->getSPS()->getUseIMP() && !isSkip(uiPartIdx) )
			{
				if ( predMV.getVer() == acMv_round[j].getVer() ) break;
			}
	        else
			{
				if ( predMV == acMv_round[j] ) break;
	        }
#else
			if ( predMV == acMv_round[j] ) break;
#endif
		}
		if ( j<0 )
		{
			acMv[ iNTmp] = pInfo->m_acMvCand[i];
			acMv_round[ iNTmp] = pInfo->m_acMvCand[i];
			acMv_round[iNTmp ++].scale_down();
		}
	}
	for ( i=0; i<iNTmp; i++ ) 
		pInfo->m_acMvCand[i] = acMv[i];
	pInfo->iN = iNTmp;

  return ;
}
#if HHI_IMVP
Bool TComDataCU::clearMVPCand_one_fourth( TComMv cMvd, AMVPInfo* pInfo, RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx )
#else
Bool TComDataCU::clearMVPCand_one_fourth( TComMv cMvd, AMVPInfo* pInfo)
#endif 
{
  UInt uiPredMV_num = pInfo->iN;
#if HHI_IMVP
  xUniqueMVPCand_one_fourth(pInfo,uiPartIdx);
#else
  xUniqueMVPCand_one_fourth(pInfo);
#endif

	// only works for multiple candidates
  if (pInfo->iN <= 1)
  {
    return (uiPredMV_num != pInfo->iN);
  }

	// only works for non-zero mvd case
  if (cMvd.getHor() == 0 && cMvd.getVer() == 0)
  {
    return (uiPredMV_num != pInfo->iN);
  }

	TComMv	acMv[ AMVP_MAX_NUM_CANDS ];
	Int aiValid [ AMVP_MAX_NUM_CANDS ];

	Int	iNTmp, i, j;

  for ( i=0; i<pInfo->iN; i++ )
  {
    aiValid[i] = 1;
  }
	for ( i=0; i<pInfo->iN; i++ )
	{
		TComMv cMvPred_round,cMvd_round;
		TComMv cMvCand;
		cMvPred_round=pInfo->m_acMvCand[i];
		cMvd_round=cMvd; 
		cMvPred_round.round();
		cMvd_round.round();
		// recreate the MV 
		cMvCand = cMvPred_round + cMvd_round;

      cMvPred_round.scale_down();
      cMvd_round.scale_down();
      cMvCand = cMvPred_round + cMvd_round;
      
#if HHI_IMVP
      UInt uiBestYBits = xGetComponentBits(cMvd_round.getVer());
#endif
      UInt uiBestBits = xGetMvdBits(cMvd_round);
      for ( j=0; j<pInfo->iN; j++ )
      {
        TComMv cMvPred_round_j;
        
        cMvPred_round_j=pInfo->m_acMvCand[j];
        cMvPred_round_j.scale_down();
#if HHI_IMVP
        if ( getSlice()->getSPS()->getUseIMP() )
        {
          if (aiValid[j] && i!=j && xGetComponentBits(cMvCand.getVer()-cMvPred_round_j.getVer()) < uiBestYBits)
          {
            aiValid[i] = 0;
          }
        }
        else
#endif
        {
          if (aiValid[j] && i!=j && xGetMvdBits(cMvCand-cMvPred_round_j) < uiBestBits)
          {
            aiValid[i] = 0;
          }
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
		return (uiPredMV_num != pInfo->iN);
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

	for ( i=0; i<iNTmp; i++ ) 
		pInfo->m_acMvCand[i] = acMv[i];
	pInfo->iN = iNTmp;

	return true;
}

Int TComDataCU::searchMVPIdx_one_fourth(TComMv cMv, AMVPInfo* pInfo)
{
#if HHI_IMVP
  if ( getSlice()->getSPS()->getUseIMP() )
  {
    for ( Int i=0; i<pInfo->iN; i++ )
    {
		TComMv cCurMv=cMv;
		TComMv cCurPredMv=pInfo->m_acMvCand[i];
		cCurMv.scale_down();
		cCurPredMv.scale_down();
        if (cCurMv.getVer() == cCurPredMv.getVer() )
            return i;
    }
  }
  else
#endif
  {
	for ( Int i=0; i<pInfo->iN; i++ )
	{
		TComMv cCurMv=cMv;
		TComMv cCurPredMv=pInfo->m_acMvCand[i];
		cCurMv.scale_down();
		cCurPredMv.scale_down();
		if (cCurMv == cCurPredMv)
			return i;
    }
  }
	assert(0);
	return -1;
}

#endif

#if HHI_IMVP
Bool TComDataCU::clearMVPCand( TComMv cMvd, AMVPInfo* pInfo, RefPicList eRefPicList, UInt uiPartIdx, Int iRefIdx )
#else
Bool TComDataCU::clearMVPCand( TComMv cMvd, AMVPInfo* pInfo )
#endif
{
  // only works for multiple candidates
  if (pInfo->iN <= 1)
  {
    return false;
  }

#if HHI_IMVP
  Int iMvCandY[ AMVP_MAX_NUM_CANDS ];
  Int iNOri = pInfo->iN;
  if ( getSlice()->getSPS()->getUseIMP() && !isSkip(uiPartIdx) )
  {
    Int iNTmp, i, j;
    // make it be unique
    iNTmp = 0;
    iMvCandY[ iNTmp++ ] = pInfo->m_acMvCand[0].getVer();
    for ( i=1; i<pInfo->iN; i++ )
    {
      for ( j=iNTmp - 1; j>=0; j-- )
      {
        if ( pInfo->m_acMvCand[i].getVer() == iMvCandY[j] ) break;
      }
      if ( j<0 )
      {
        iMvCandY[ iNTmp++ ] = pInfo->m_acMvCand[i].getVer();
      }
    }
    for ( i=0; i<iNTmp; i++ ) pInfo->m_acMvCand[i].setVer(iMvCandY[i]) ;
    pInfo->iN = iNTmp;

    if (pInfo->iN <= 1)
    {
      return true;
    }
  }  
#endif
  
  // only works for non-zero mvd case
  if (cMvd.getHor() == 0 && cMvd.getVer() == 0)
  {
#if HHI_IMVP
    if ( getSlice()->getSPS()->getUseIMP() )
    {
      return (iNOri != pInfo->iN);
    }
#endif
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
#if HHI_IMVP
    TComMv cMvCand;
    UInt uiBestYBits = 0;
    if ( getSlice()->getSPS()->getUseIMP() )
    {
      // recreate the MV 
      // MVy from the AMVP y component
      cMvCand.setVer(  pInfo->m_acMvCand[i].getVer() + cMvd.getVer() );
      uiBestYBits = xGetComponentBits(cMvd.getVer());
    }
    else
    {
      cMvCand = pInfo->m_acMvCand[i] + cMvd;
    }
#else
    TComMv cMvCand = pInfo->m_acMvCand[i] + cMvd;
#endif

    UInt uiBestBits = xGetMvdBits(cMvd);
    for ( j=0; j<pInfo->iN; j++ )
    {
#if HHI_IMVP
      if ( getSlice()->getSPS()->getUseIMP() )
      {
        int iYPred = pInfo->m_acMvCand[j].getVer();
        if (aiValid[j] && i!=j && xGetComponentBits(cMvCand.getVer()-iYPred) < uiBestYBits)
        {
          aiValid[i] = 0;
        }
      }
      else
      {
        if (aiValid[j] && i!=j && xGetMvdBits(cMvCand-pInfo->m_acMvCand[j]) < uiBestBits)
        {
          aiValid[i] = 0;
        }
      }
#else
     if (aiValid[j] && i!=j && xGetMvdBits(cMvCand-pInfo->m_acMvCand[j]) < uiBestBits)
     {
       aiValid[i] = 0;
     }
#endif
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
#if HHI_IMVP
    if ( getSlice()->getSPS()->getUseIMP() )
    {
      return (iNOri != pInfo->iN);
    }
#endif
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
#if HHI_IMVP
  if ( getSlice()->getSPS()->getUseIMP() )
  {
    for ( Int i=0; i<pInfo->iN; i++ )
    {
      if (cMv.getVer() == pInfo->m_acMvCand[i].getVer() )
        return i;
    }
  }
  else
  {
    for ( Int i=0; i<pInfo->iN; i++ )
    {
      if (cMv == pInfo->m_acMvCand[i])
        return i;
    }
  }
#else
  for ( Int i=0; i<pInfo->iN; i++ )
  {
    if (cMv == pInfo->m_acMvCand[i])
      return i;
  }
#endif
  
  assert(0);
  return -1;
}

Void TComDataCU::clipMv    (TComMv&  rcMv)
{
#ifdef QC_AMVRES
  Int  iMvShift = (m_pcSlice->getSPS()->getUseAMVRes())?3:2;
#else
  Int  iMvShift = 2;
#endif
  Int iHorMax = (m_pcSlice->getSPS()->getWidth() - m_uiCUPelX - 1 )<<iMvShift;
  Int iHorMin = (      -(Int)g_uiMaxCUWidth - (Int)m_uiCUPelX + 1 )<<iMvShift;

  Int iVerMax = (m_pcSlice->getSPS()->getHeight() - m_uiCUPelY - 1 )<<iMvShift;
  Int iVerMin = (      -(Int)g_uiMaxCUHeight - (Int)m_uiCUPelY + 1 )<<iMvShift;

  rcMv.setHor( Min (iHorMax, Max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( Min (iVerMax, Max (iVerMin, rcMv.getVer())) );
}

#ifdef DCM_PBIC
Void TComDataCU::fillICPCand( UInt uiPartIdx, UInt uiPartAddr, AICPInfo* pInfo )
{
  Bool bAdded;
  UInt uiIdx;
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  Int aiRefIdx[2];
  Int iLeftIcIdx = -1;
  Int iAboveIcIdx = -1;
  Int iCornerIcIdx = -1;
  UChar uhInterDir = getInterDir(uiPartAddr);
  PartSize eCUMode = m_pePartSize[0];
  TComIc cIcDefault;

  if (uhInterDir & 1)
    aiRefIdx[REF_PIC_LIST_0] = getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartAddr);
  else
    aiRefIdx[REF_PIC_LIST_0] = NOT_VALID;

  if (uhInterDir & 2)
    aiRefIdx[REF_PIC_LIST_1] = getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartAddr);
  else
    aiRefIdx[REF_PIC_LIST_1] = NOT_VALID;

  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );

  pInfo->iN = 0;

  //Add Default Predictor
  pInfo->m_acIcCand[pInfo->iN++] = cIcDefault;

  bAdded = false;
  //Left
  for ( uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxLB]; uiIdx+= uiNumPartInCUWidth )
  {
    bAdded = xAddICPCand( pInfo, uhInterDir, aiRefIdx, g_auiRasterToZscan[uiIdx], MD_LEFT );
    if (bAdded && iLeftIcIdx < 0)
    {
      iLeftIcIdx = pInfo->iN-1;
    }
    if (bAdded) break;
  }

  bAdded = false;
  //Above
  for ( uiIdx = g_auiZscanToRaster[uiPartIdxLT]; uiIdx <= g_auiZscanToRaster[uiPartIdxRT]; uiIdx++ )
  {
    bAdded = xAddICPCand( pInfo, uhInterDir, aiRefIdx, g_auiRasterToZscan[uiIdx], MD_ABOVE);
    if (bAdded && iAboveIcIdx < 0)
    {
      iAboveIcIdx = pInfo->iN-1;
    }
    if (bAdded) break;
  }

  bAdded = false;
  //Above Right
  bAdded = xAddICPCand( pInfo, uhInterDir, aiRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
  if (bAdded && iCornerIcIdx < 0)
  {
    iCornerIcIdx = pInfo->iN-1;
  }
  //Below Left
  if (!bAdded)
  {
    bAdded = xAddICPCand( pInfo, uhInterDir, aiRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
  }
  if (bAdded && iCornerIcIdx < 0)
  {
    iCornerIcIdx = pInfo->iN-1;
  }
  //Above Left
  if (!bAdded)
  {
    bAdded = xAddICPCand( pInfo, uhInterDir, aiRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
  }
  if (bAdded && iCornerIcIdx < 0)
  {
    iCornerIcIdx = pInfo->iN-1;
  }

  if ( iLeftIcIdx >= 0 && iAboveIcIdx >= 0 &&
       ( ( ((eCUMode == SIZE_2NxN) || (eCUMode == SIZE_2NxnU) || (eCUMode == SIZE_2NxnD)) && uiPartIdx == 0 ) ||
         ( ((eCUMode == SIZE_Nx2N) || (eCUMode == SIZE_nLx2N) || (eCUMode == SIZE_nRx2N)) && uiPartIdx == 1 ) ))
  {
    //Swap Left and Above Predictors
    TComIc cIcTemp;
    cIcTemp                        = pInfo->m_acIcCand[iLeftIcIdx];
    pInfo->m_acIcCand[iLeftIcIdx]  = pInfo->m_acIcCand[iAboveIcIdx];
    pInfo->m_acIcCand[iAboveIcIdx] = cIcTemp;
  }

  //Remove Duplicates
  xUniqueICPCand( pInfo );

}

Int TComDataCU::searchICPIdx(TComIc cIc, AICPInfo* pInfo)
{
  for ( Int i=0; i<pInfo->iN; i++ )
  {
    if ( cIc.isequalIcParam( pInfo->m_acIcCand[i] ) )
      return i;
  }
  
  assert(0);
  return -1;
}
#endif

Void TComDataCU::convertTransIdx( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode )
{
#if HHI_RQT_INTRA
  if( getPredictionMode( uiAbsPartIdx ) == MODE_INTRA && !getSlice()->getSPS()->getQuadtreeTUFlag() )
#else
  if( getPredictionMode( uiAbsPartIdx ) == MODE_INTRA )
#endif
  {
    ruiLumaTrMode      = uiTrIdx;

    UInt uiWidthInBit  = g_aucConvertToBit[m_puhWidth[uiAbsPartIdx]>>1]+2;
    UInt uiTrSizeInBit = g_aucConvertToBit[getSlice()->getSPS()->getMaxTrSize()]+2;
    ruiChromaTrMode    = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;

    return;
  }

  UInt uiSizeBit        = g_aucConvertToBit[ Min( m_puhWidth [ uiAbsPartIdx ], m_puhHeight[ uiAbsPartIdx ] ) ] + 2;
  UInt uiMinCUSizeBit   = g_aucConvertToBit[ Min( m_pcPic->getMinCUWidth(),    m_pcPic->getMinCUHeight()   ) ] + 2;
  UInt uiLowerBnd       = uiMinCUSizeBit;//Max( uiMinCUSizeBit, 2 );

  ruiLumaTrMode   = uiTrIdx;
  ruiChromaTrMode = uiTrIdx;

#if HHI_RQT
  if( !m_pcPic->getSlice()->getSPS()->getQuadtreeTUFlag() )
#endif
  if ( (Int)uiSizeBit - (Int)uiTrIdx <= (Int)uiLowerBnd  )
  {
    ruiLumaTrMode   = uiSizeBit - uiLowerBnd;
    ruiChromaTrMode = ruiLumaTrMode;
#if HHI_RQT
    assert( uiLowerBnd == uiMinCUSizeBit); // the if statement below can be removed
#endif
    if ( uiLowerBnd == uiMinCUSizeBit)//2 )
    {
      ruiChromaTrMode--;
    }
  }

  return;
}

UInt TComDataCU::getIntraSizeIdx(UInt uiAbsPartIdx)
{
  UInt uiShift = ( (m_puhTrIdx[uiAbsPartIdx]==0) && (m_pePartSize[uiAbsPartIdx]==SIZE_NxN) ) ? m_puhTrIdx[uiAbsPartIdx]+1 : m_puhTrIdx[uiAbsPartIdx];
#if HHI_RQT_INTRA
  if( this->getSlice()->getSPS()->getQuadtreeTUFlag() )
  {
    uiShift = ( m_pePartSize[uiAbsPartIdx]==SIZE_NxN ? 1 : 0 );
  }
#endif

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

#if PLANAR_INTRA
Void TComDataCU::setPlanarInfoSubParts( Int iPlanarFlag, Int iPlanarDeltaY, Int iPlanarDeltaU, Int iPlanarDeltaV, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  UInt uiCounter;

  for (uiCounter = 0;uiCounter < uiCurrPartNumb;uiCounter++)
  {
    m_piPlanarInfo[0][uiAbsPartIdx+uiCounter] = iPlanarFlag;
    m_piPlanarInfo[1][uiAbsPartIdx+uiCounter] = iPlanarDeltaY;
    m_piPlanarInfo[2][uiAbsPartIdx+uiCounter] = iPlanarDeltaU;
    m_piPlanarInfo[3][uiAbsPartIdx+uiCounter] = iPlanarDeltaV;
  }
}
#endif

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

#ifdef DCM_PBIC
Bool TComDataCU::xAddICPCand( AICPInfo* pInfo, UChar uhInterDir, Int* piRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir )
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

  if ( pcTmpCU != NULL )
  {
    if ( uhInterDir == pcTmpCU->getInterDir(uiIdx))
    {
      if      ( (uhInterDir & 1) && !(m_pcSlice->isEqualRef(REF_PIC_LIST_0, pcTmpCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiIdx), piRefIdx[REF_PIC_LIST_0])) )
        pcTmpCU = NULL;
      else if ( (uhInterDir & 2) && !(m_pcSlice->isEqualRef(REF_PIC_LIST_1, pcTmpCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiIdx), piRefIdx[REF_PIC_LIST_1])) )
        pcTmpCU = NULL;
    }
    else
      pcTmpCU = NULL;
  }

  if ( pcTmpCU != NULL )
  {
    TComIc cIcPred;
    cIcPred.copyIcParam( pcTmpCU->getCUIcField()->getIc(uiIdx) );
    pInfo->m_acIcCand[pInfo->iN++] = cIcPred;
    return true;
  }
  return false;
}

Void TComDataCU::xUniqueICPCand(AICPInfo* pInfo)
{
  Int iNTmp, i, j;

  iNTmp = 1;
  for ( i=1; i<pInfo->iN; i++ )
  {
    for ( j=iNTmp-1; j>=0; j-- )
    {
      if ( pInfo->m_acIcCand[i].isequalIcParam( pInfo->m_acIcCand[j] ) ) break;
    }
    if ( j<0 )
    {
      pInfo->m_acIcCand[ iNTmp++ ] = pInfo->m_acIcCand[i];
    }
  }
  pInfo->iN = iNTmp;
}

UInt TComDataCU::xGetIcdBits(TComIc cIcd)
{
  Int aiParam[3];
  cIcd.getIcParam( aiParam[0], aiParam[1], aiParam[2] );
  return ( xGetComponentBits(aiParam[0]) + xGetComponentBits(aiParam[1]) + xGetComponentBits(aiParam[2]) );
}
#endif

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

