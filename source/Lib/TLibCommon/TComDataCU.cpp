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

/** \file			TComDataCU.cpp
    \brief		CU data structure
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
  m_puiAlfCtrlFlag		 = NULL;
  m_puiTmpAlfCtrlFlag  = NULL;
  m_puhWidth           = NULL;
  m_puhHeight          = NULL;
  m_phQP               = NULL;
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

  m_apiMVPIdx[0]			 = NULL;
  m_apiMVPIdx[1]			 = NULL;
  m_apiMVPNum[0]			 = NULL;
  m_apiMVPNum[1]			 = NULL;

  m_pMPIindex					 = NULL;
  m_pROTindex					 = NULL;
  m_pCIPflag					 = NULL;
  m_pScanOrder				 = NULL;

  m_bDecSubCu = false;
  m_phHAMUsed = NULL;
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
    m_phQP               = (UChar*	  )xMalloc(UChar,		 uiNumPartition);
    m_puhDepth           = (UChar*	  )xMalloc(UChar,    uiNumPartition);
    m_puhWidth           = (UChar*	  )xMalloc(UChar,    uiNumPartition);
    m_puhHeight          = (UChar*		)xMalloc(UChar,    uiNumPartition);
    m_pePartSize         = (PartSize* )xMalloc(PartSize, uiNumPartition);
    m_pePredMode         = (PredMode* )xMalloc(PredMode, uiNumPartition);

    m_puiAlfCtrlFlag		 = (UInt*  )xMalloc(UInt,		uiNumPartition);

    m_puhLumaIntraDir    = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhChromaIntraDir  = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_pMPIindex					 = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_pROTindex					 = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_pCIPflag					 = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_apiMVPIdx[0]			 = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPIdx[1]			 = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[0]			 = (Int*   )xMalloc(Int,  uiNumPartition);
    m_apiMVPNum[1]			 = (Int*   )xMalloc(Int,  uiNumPartition);

    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pScanOrder         = (UChar* )xMalloc(UChar,  uiNumPartition);

    m_phHAMUsed        = (UChar*)xMalloc(UChar, uiNumPartition);

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

  m_apcCUColocated[0]	 = NULL;
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
		if ( m_phQP								) { xFree(m_phQP);							m_phQP							= NULL; }
		if ( m_puhDepth						) { xFree(m_puhDepth);					m_puhDepth					= NULL; }
		if ( m_puhWidth						) { xFree(m_puhWidth);					m_puhWidth					= NULL; }
		if ( m_puhHeight					) { xFree(m_puhHeight);					m_puhHeight					= NULL; }
		if ( m_pePartSize					) { xFree(m_pePartSize);				m_pePartSize				= NULL; }
		if ( m_pePredMode					) { xFree(m_pePredMode);				m_pePredMode				= NULL; }
		if ( m_puhCbf[0]					) { xFree(m_puhCbf[0]);					m_puhCbf[0]					= NULL; }
		if ( m_puhCbf[1]					) { xFree(m_puhCbf[1]);					m_puhCbf[1]					= NULL; }
		if ( m_puhCbf[2]					) { xFree(m_puhCbf[2]);					m_puhCbf[2]					= NULL; }
		if ( m_puiAlfCtrlFlag			) { xFree(m_puiAlfCtrlFlag);		m_puiAlfCtrlFlag		= NULL; }
		if ( m_puhInterDir				)	{ xFree(m_puhInterDir);				m_puhInterDir				= NULL; }
		if ( m_puhLumaIntraDir		) { xFree(m_puhLumaIntraDir);		m_puhLumaIntraDir		= NULL; }
		if ( m_puhChromaIntraDir	) { xFree(m_puhChromaIntraDir); m_puhChromaIntraDir = NULL; }
		if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);          m_puhTrIdx					= NULL; }
		if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);        m_pcTrCoeffY				= NULL; }
		if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);       m_pcTrCoeffCb				= NULL; }
		if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);       m_pcTrCoeffCr				= NULL; }
		if ( m_pScanOrder         ) { xFree( m_pScanOrder );      m_pScanOrder				= NULL; }
		if ( m_pMPIindex          ) { xFree(m_pMPIindex);					m_pMPIindex					= NULL; }
		if ( m_pROTindex          ) { xFree(m_pROTindex);					m_pROTindex					= NULL; }
		if ( m_pCIPflag           ) { xFree(m_pCIPflag);					m_pCIPflag					= NULL; }
		if ( m_apiMVPIdx[0]       ) { xFree(m_apiMVPIdx[0]);      m_apiMVPIdx[0]			= NULL; }
		if ( m_apiMVPIdx[1]       ) { xFree(m_apiMVPIdx[1]);      m_apiMVPIdx[1]			= NULL; }
		if ( m_apiMVPNum[0]       ) { xFree(m_apiMVPNum[0]);      m_apiMVPNum[0]			= NULL; }
		if ( m_apiMVPNum[1]       ) { xFree(m_apiMVPNum[1]);      m_apiMVPNum[1]			= NULL; }
		if ( m_phHAMUsed          ) { xFree(m_phHAMUsed);					m_phHAMUsed					= NULL; }

    m_acCUMvField[0].destroy();
    m_acCUMvField[1].destroy();
  }

  m_pcCUAboveLeft				= NULL;
  m_pcCUAboveRight			= NULL;
  m_pcCUAbove						= NULL;
  m_pcCULeft						= NULL;

  m_apcCUColocated[0]		= NULL;
  m_apcCUColocated[1]		= NULL;
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
		\param	pcPic			picture (TComPic) class pointer
		\param	iCUAddr		CU address
 */
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{
  m_pcPic              = pcPic;
  m_pcSlice            = pcPic->getSlice();
  m_uiCUAddr           = iCUAddr;
  m_uiCUPelX           = ( iCUAddr % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth;
  m_uiCUPelY           = ( iCUAddr / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight;
  m_uiAbsZorderIdx     = 0;

  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiNumPartition     = pcPic->getNumPartInCU();

  Int iSizeInUchar = sizeof( UChar ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt  ) * m_uiNumPartition;

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

  memset( m_puiAlfCtrlFlag,    0, iSizeInUInt  );
  memset( m_puhLumaIntraDir,   2, iSizeInUchar );
  memset( m_puhChromaIntraDir, 0, iSizeInUchar );
  memset( m_puhInterDir,       0, iSizeInUchar );
  memset( m_puhTrIdx,          0, iSizeInUchar );
  memset( m_puhCbf[0],         0, iSizeInUchar );
  memset( m_puhCbf[1],         0, iSizeInUchar );
  memset( m_puhCbf[2],         0, iSizeInUchar );
  memset( m_puhDepth,          0, iSizeInUchar );

	UChar uhWidth  = g_uiMaxCUWidth;
	UChar uhHeight = g_uiMaxCUHeight;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  memset( m_pMPIindex,         0, iSizeInUchar );
  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );
  memset( m_pScanOrder,        0, iSizeInUchar );

  memset( m_phHAMUsed,       0, iSizeInUchar );

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
  UInt uiQp = m_pcPic->getSlice()->getSliceQp();
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;

  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

  memset( m_puiAlfCtrlFlag,    0, iSizeInUInt );
  memset( m_puhLumaIntraDir,   2, iSizeInUchar );
  memset( m_puhChromaIntraDir, 0, iSizeInUchar );
  memset( m_puhInterDir,       0, iSizeInUchar );
  memset( m_puhTrIdx,          0, iSizeInUchar );
  memset( m_puhCbf[0],         0, iSizeInUchar );
  memset( m_puhCbf[1],         0, iSizeInUchar );
  memset( m_puhCbf[2],         0, iSizeInUchar );

  memset( m_pMPIindex,         0, iSizeInUchar );
  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );
  memset( m_pScanOrder,        0, iSizeInUchar );

  memset( m_phHAMUsed,       0, iSizeInUchar );

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
  m_uiAbsZorderIdx     = pcCU->getZorderIdxInCU() + uiPartOffset;
  m_uiCUPelX           = pcCU->getCUPelX() + ( pcCU->getWidth (0) >> 1 )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( pcCU->getHeight(0) >> 1 )*( uiPartUnitIdx >> 1 );

  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;

  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;

  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInUInt  = sizeof( UInt   ) * m_uiNumPartition;

  memset( m_phQP,              m_pcPic->getSlice()->getSliceQp(), iSizeInUchar );

	memset( m_puiAlfCtrlFlag,    0, iSizeInUInt );
  memset( m_puhLumaIntraDir,   2, iSizeInUchar );
  memset( m_puhChromaIntraDir, 0, iSizeInUchar );
  memset( m_puhInterDir,       0, iSizeInUchar );
  memset( m_puhTrIdx,          0, iSizeInUchar );
  memset( m_puhCbf[0],         0, iSizeInUchar );
  memset( m_puhCbf[1],         0, iSizeInUchar );
  memset( m_puhCbf[2],         0, iSizeInUchar );
  memset( m_puhDepth,          uiDepth, iSizeInUchar );

	UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
	UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );

  memset( m_pMPIindex,         0, iSizeInUchar );
  memset( m_pROTindex,         0, iSizeInUchar );
  memset( m_pCIPflag,          0, iSizeInUchar );
  memset( m_pScanOrder,        0, iSizeInUchar );

  memset( m_phHAMUsed,       0, iSizeInUchar );

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
  m_uiAbsZorderIdx     = uiAbsPartIdx;

  m_uiCUPelX           = pcCU->getCUPelX() + g_auiConvertPartIdxToPelX[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];
  m_uiCUPelY           = pcCU->getCUPelY() + g_auiConvertPartIdxToPelY[ g_auiConvertRelToAbsIdx[uiAbsPartIdx] ];

  UInt uiWidth         = g_uiMaxCUWidth  >> uiDepth;
  UInt uiHeight        = g_uiMaxCUHeight >> uiDepth;

  Int iSizeInUchar  = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInInt    = sizeof( Int    ) * m_uiNumPartition;
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;

  m_phQP=pcCU->getQP()										+ uiPart;
  m_pePartSize=pcCU->getPartitionSize()		+ uiPart;
  m_pePredMode=pcCU->getPredictionMode()	+ uiPart;

  m_puhLumaIntraDir=pcCU->getLumaIntraDir()				+ uiPart;
  m_puhChromaIntraDir= pcCU->getChromaIntraDir()	+ uiPart;
  m_puhInterDir=pcCU->getInterDir()								+ uiPart;
  m_puhTrIdx=         pcCU->getTransformIdx()			+ uiPart;

  m_puhCbf[0]= pcCU->getCbf(TEXT_LUMA)						+ uiPart;
  m_puhCbf[1]= pcCU->getCbf(TEXT_CHROMA_U)				+ uiPart;
  m_puhCbf[2]= pcCU->getCbf(TEXT_CHROMA_V)				+ uiPart;

  m_puhDepth=pcCU->getDepth()											+ uiPart;
  m_puhWidth=pcCU->getWidth()											+ uiPart;
  m_puhHeight=pcCU->getHeight()										+ uiPart;

  m_apiMVPIdx[0]=pcCU->getMVPIdx(REF_PIC_LIST_0)	+ uiPart;
  m_apiMVPIdx[1]=pcCU->getMVPIdx(REF_PIC_LIST_1)	+ uiPart;
  m_apiMVPNum[0]=pcCU->getMVPNum(REF_PIC_LIST_0)	+ uiPart;
  m_apiMVPNum[1]=pcCU->getMVPNum(REF_PIC_LIST_1)	+ uiPart;

  m_pMPIindex= pcCU->getMPIindex()								+ uiPart;
  m_pROTindex= pcCU->getROTindex()								+ uiPart;
  m_pCIPflag=  pcCU->getCIPflag()									+ uiPart;
  m_pScanOrder= pcCU->getScanOrder()							+ uiPart;

  m_phHAMUsed= pcCU->getHAMUsed() + uiPart;

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
  Int iSizeInUchar  = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInInt    = sizeof( Int    ) * m_uiNumPartition;
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;

  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsZorderIdx     = uiAbsPartIdx;

  Int iRastPartIdx		 = g_auiConvertRelToAbsIdx[uiAbsPartIdx];
  m_uiCUPelX           = pcCU->getCUPelX() + m_pcPic->getMinCUWidth ()*( iRastPartIdx % m_pcPic->getNumPartInWidth() );
  m_uiCUPelY           = pcCU->getCUPelY() + m_pcPic->getMinCUHeight()*( iRastPartIdx / m_pcPic->getNumPartInWidth() );

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();

  m_apcCUColocated[0]  = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1]  = pcCU->getCUColocated(REF_PIC_LIST_1);

	m_pePartSize				 = pcCU->getPartitionSize ()				+ uiAbsPartIdx;
  m_pePredMode				 = pcCU->getPredictionMode()				+ uiAbsPartIdx;
  m_puhInterDir				 = pcCU->getInterDir	    ()				+ uiAbsPartIdx;

  m_puhDepth					 = pcCU->getDepth ()								+ uiAbsPartIdx;
  m_puhWidth					 = pcCU->getWidth ()								+ uiAbsPartIdx;
  m_puhHeight					 = pcCU->getHeight()								+ uiAbsPartIdx;

  m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
  m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;

  m_acCUMvField[eRefPicList].setMvPtr(pcCU->getCUMvField(eRefPicList)->getMv()     + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setMvdPtr(pcCU->getCUMvField(eRefPicList)->getMvd()    + uiAbsPartIdx);
  m_acCUMvField[eRefPicList].setRefIdxPtr(pcCU->getCUMvField(eRefPicList)->getRefIdx() + uiAbsPartIdx);
}

// Copy inter prediction info to the biggest CU
Void TComDataCU::copyInterPredInfoTo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList )
{
  Int iSizeInInt    = sizeof( Int    ) * m_uiNumPartition;
  Int iSizeInTComMv = sizeof( TComMv ) * m_uiNumPartition;

  memcpy( pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx, m_apiMVPIdx[eRefPicList], iSizeInInt );
  memcpy( pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx, m_apiMVPNum[eRefPicList], iSizeInInt );
  memcpy( pcCU->getCUMvField(eRefPicList)->getMv() + uiAbsPartIdx, m_acCUMvField[eRefPicList].getMv(), iSizeInTComMv );
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

  memcpy( m_phQP       + uiOffset, pcCU->getQP(),             iSizeInUchar                        );
  memcpy( m_pePartSize + uiOffset, pcCU->getPartitionSize(),  sizeof( PartSize ) * uiNumPartition );
  memcpy( m_pePredMode + uiOffset, pcCU->getPredictionMode(), sizeof( PredMode ) * uiNumPartition );

  memcpy( m_puiAlfCtrlFlag    + uiOffset, pcCU->getAlfCtrlFlag(),    iSizeInUInt  );
  memcpy( m_puhLumaIntraDir   + uiOffset, pcCU->getLumaIntraDir(),   iSizeInUchar );
  memcpy( m_puhChromaIntraDir + uiOffset, pcCU->getChromaIntraDir(), iSizeInUchar );
  memcpy( m_puhInterDir       + uiOffset, pcCU->getInterDir(),       iSizeInUchar );
  memcpy( m_puhTrIdx          + uiOffset, pcCU->getTransformIdx(),   iSizeInUchar );

  memcpy( m_puhCbf[0] + uiOffset, pcCU->getCbf(TEXT_LUMA)    , iSizeInUchar );
  memcpy( m_puhCbf[1] + uiOffset, pcCU->getCbf(TEXT_CHROMA_U), iSizeInUchar );
  memcpy( m_puhCbf[2] + uiOffset, pcCU->getCbf(TEXT_CHROMA_V), iSizeInUchar );

  memcpy( m_puhDepth  + uiOffset, pcCU->getDepth(),  iSizeInUchar );
  memcpy( m_puhWidth  + uiOffset, pcCU->getWidth(),  iSizeInUchar );
  memcpy( m_puhHeight + uiOffset, pcCU->getHeight(), iSizeInUchar );

  memcpy( m_pMPIindex  + uiOffset, pcCU->getMPIindex(),  iSizeInUchar );
  memcpy( m_pROTindex  + uiOffset, pcCU->getROTindex(),  iSizeInUchar );
  memcpy( m_pCIPflag   + uiOffset, pcCU->getCIPflag(),   iSizeInUchar );
  memcpy( m_pScanOrder + uiOffset, pcCU->getScanOrder(), iSizeInUchar );

  memcpy( m_apiMVPIdx[0] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPIdx[1] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_1), iSizeInInt );
  memcpy( m_apiMVPNum[0] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_0), iSizeInInt );
  memcpy( m_apiMVPNum[1] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_1), iSizeInInt );

  memcpy( m_phHAMUsed + uiOffset, pcCU->getHAMUsed(), iSizeInUchar );

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
  UInt uiNumPartInWidth  = m_pcPic->getNumPartInWidth () >> uhDepth;
  UInt uiNumPartInHeight = m_pcPic->getNumPartInHeight() >> uhDepth;

  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;

  Int iSizeInUchar  = sizeof( UChar ) * m_uiNumPartition;
  Int iSizeInUInt   = sizeof( UInt  ) * m_uiNumPartition;
  Int iSizeInInt    = sizeof( Int   ) * m_uiNumPartition;

  memcpy( rpcCU->getQP() + m_uiAbsZorderIdx, m_phQP, iSizeInUchar );

  memcpy( rpcCU->getPartitionSize()  + m_uiAbsZorderIdx, m_pePartSize, sizeof( PartSize ) * m_uiNumPartition );
  memcpy( rpcCU->getPredictionMode() + m_uiAbsZorderIdx, m_pePredMode, sizeof( PredMode ) * m_uiNumPartition );

  memcpy( rpcCU->getAlfCtrlFlag()    + m_uiAbsZorderIdx, m_puiAlfCtrlFlag,    iSizeInUInt  );

  memcpy( rpcCU->getLumaIntraDir()   + m_uiAbsZorderIdx, m_puhLumaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir() + m_uiAbsZorderIdx, m_puhChromaIntraDir, iSizeInUchar );
  memcpy( rpcCU->getInterDir()       + m_uiAbsZorderIdx, m_puhInterDir,       iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()   + m_uiAbsZorderIdx, m_puhTrIdx,          iSizeInUchar );

  memcpy( rpcCU->getCbf(TEXT_LUMA)     + m_uiAbsZorderIdx, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + m_uiAbsZorderIdx, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + m_uiAbsZorderIdx, m_puhCbf[2], iSizeInUchar );

  memcpy( rpcCU->getDepth()  + m_uiAbsZorderIdx, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + m_uiAbsZorderIdx, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + m_uiAbsZorderIdx, m_puhHeight, iSizeInUchar );

  memcpy( rpcCU->getMPIindex()  + m_uiAbsZorderIdx, m_pMPIindex,  iSizeInUchar );
  memcpy( rpcCU->getROTindex()  + m_uiAbsZorderIdx, m_pROTindex,  iSizeInUchar );
  memcpy( rpcCU->getCIPflag()   + m_uiAbsZorderIdx, m_pCIPflag,   iSizeInUchar );
  memcpy( rpcCU->getScanOrder() + m_uiAbsZorderIdx, m_pScanOrder, iSizeInUchar );

  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + m_uiAbsZorderIdx, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + m_uiAbsZorderIdx, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + m_uiAbsZorderIdx, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + m_uiAbsZorderIdx, m_apiMVPNum[1], iSizeInInt );

  memcpy( rpcCU->getHAMUsed() + m_uiAbsZorderIdx, m_phHAMUsed, iSizeInUchar );

  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsZorderIdx );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsZorderIdx );

  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>(uhDepth<<1);
  UInt uiTmp2 = m_uiAbsZorderIdx*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );

  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
}

Void TComDataCU::copyToPic( UChar uhDepth, UInt uiPartIdx, UInt uiPartDepth )
{
  TComDataCU*& rpcCU = m_pcPic->getCU( m_uiCUAddr );
  UInt uiNumPartInWidth  = m_pcPic->getNumPartInWidth () >> (uhDepth+uiPartDepth);
  UInt uiNumPartInHeight = m_pcPic->getNumPartInHeight() >> (uhDepth+uiPartDepth);
  UInt uiQNumPart = m_uiNumPartition>>(uiPartDepth<<1);

  UInt uiPartStart = uiPartIdx*uiQNumPart;
  UInt uiPartOffset = m_uiAbsZorderIdx + uiPartStart;

  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;

  Int iSizeInUchar  = sizeof( UChar  ) * uiQNumPart;
  Int iSizeInUInt   = sizeof( UInt   ) * uiQNumPart;
  Int iSizeInInt    = sizeof( Int    ) * uiQNumPart;

  memcpy( rpcCU->getQP() + uiPartOffset, m_phQP, iSizeInUchar );

  memcpy( rpcCU->getPartitionSize()  + uiPartOffset, m_pePartSize, sizeof( PartSize ) * uiQNumPart );
  memcpy( rpcCU->getPredictionMode() + uiPartOffset, m_pePredMode, sizeof( PredMode ) * uiQNumPart );

  memcpy( rpcCU->getAlfCtrlFlag()     + uiPartOffset, m_puiAlfCtrlFlag,    iSizeInUInt  );
  memcpy( rpcCU->getLumaIntraDir()    + uiPartOffset, m_puhLumaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir()  + uiPartOffset, m_puhChromaIntraDir, iSizeInUchar );
  memcpy( rpcCU->getInterDir()        + uiPartOffset, m_puhInterDir,       iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()    + uiPartOffset, m_puhTrIdx,          iSizeInUchar );

  memcpy( rpcCU->getCbf(TEXT_LUMA)     + uiPartOffset, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + uiPartOffset, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + uiPartOffset, m_puhCbf[2], iSizeInUchar );

  memcpy( rpcCU->getDepth()  + uiPartOffset, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + uiPartOffset, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + uiPartOffset, m_puhHeight, iSizeInUchar );

  memcpy( rpcCU->getMPIindex()  + uiPartOffset, m_pMPIindex,  iSizeInUchar );
  memcpy( rpcCU->getROTindex()  + uiPartOffset, m_pROTindex,  iSizeInUchar );
  memcpy( rpcCU->getCIPflag()   + uiPartOffset, m_pCIPflag,   iSizeInUchar );
  memcpy( rpcCU->getScanOrder() + uiPartOffset, m_pScanOrder, iSizeInUchar );

  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPIdx[0], iSizeInInt );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPIdx[1], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPNum[0], iSizeInInt );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPNum[1], iSizeInInt );

  memcpy( rpcCU->getHAMUsed() + uiPartOffset, m_phHAMUsed, iSizeInUchar );

  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsZorderIdx, uiPartStart, uiQNumPart );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsZorderIdx, uiPartStart, uiQNumPart );

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

Void TComDataCU::setHAMUsedSubParts( UChar uhHAMUsed, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_phHAMUsed + uiAbsPartIdx, uhHAMUsed, sizeof(UChar)*uiCurrPartNumb );
}

Bool TComDataCU::checkHAMVal( UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiNumSCU  = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  UInt aiPartIdx[4];
  UInt uiNumPartInCUWidth  = m_pcPic->getNumPartInWidth()  >> uiDepth;
  UInt uiNumPartInCUHeight = m_pcPic->getNumPartInHeight() >> uiDepth;
  aiPartIdx[0] = uiAbsPartIdx;                                                            //left top
  aiPartIdx[1] = uiAbsPartIdx + g_auiConvertAbsToRelIdx[uiNumPartInCUWidth-1];            //right top
  aiPartIdx[2] = uiAbsPartIdx + uiNumSCU - g_auiConvertAbsToRelIdx[uiNumPartInCUWidth-1]; //left bottom
  aiPartIdx[3] = uiAbsPartIdx + uiNumSCU - 1;                                             //right bottom

  Bool bAllZero = true;
  for( UInt ui = 0 ; ui < 4 ; ui++ )
  {
    if( m_pcSlice->getNumRefIdx(REF_PIC_LIST_0) > 0 )
    {
      if( (m_acCUMvField[REF_PIC_LIST_0].getMv(aiPartIdx[ui]).getHorD() != 0) || (m_acCUMvField[REF_PIC_LIST_0].getMv(aiPartIdx[ui]).getVerD() != 0) )
      {
        bAllZero = false;
        break;
      }
    }

    if( m_pcSlice->getNumRefIdx(REF_PIC_LIST_1) > 0 )
    {
      if( (m_acCUMvField[REF_PIC_LIST_1].getMv(aiPartIdx[ui]).getHorD() != 0) || (m_acCUMvField[REF_PIC_LIST_1].getMv(aiPartIdx[ui]).getVerD() != 0) )
      {
        bAllZero = false;
        break;
      }
    }
  }
  if( bAllZero )
    memset(m_phHAMUsed + uiAbsPartIdx, 0, sizeof(UChar)*uiNumSCU);

  return bAllZero;
}

Void TComDataCU::setMPIindexSubParts( UChar MPIindex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pMPIindex + uiAbsPartIdx, MPIindex, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setCIPflagSubParts( UChar CIPflag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pCIPflag + uiAbsPartIdx, CIPflag, sizeof(UChar)*uiCurrPartNumb );
}

// ACS
Void TComDataCU::setScanOrderSubParts( UInt ucScanOrder, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pScanOrder + uiAbsPartIdx, ucScanOrder, sizeof( UChar ) * uiCurrPartNumb );
}

Void TComDataCU::setROTindexSubParts( UInt ROTindex, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset( m_pROTindex + uiAbsPartIdx, ROTindex, sizeof(UChar)*uiCurrPartNumb );
}

TComDataCU* TComDataCU::getPULeft( UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiConvertRelToAbsIdx[m_uiAbsZorderIdx];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    uiLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx - 1 ];
    if ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth )
    {
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiLPartUnitIdx -= m_uiAbsZorderIdx;
      return this;
    }
  }

  uiLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx + uiNumPartInCUWidth - 1 ];
  return m_pcCULeft;
}

TComDataCU* TComDataCU::getPUAbove( UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiConvertRelToAbsIdx[m_uiAbsZorderIdx];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiAPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx - uiNumPartInCUWidth ];
    if ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth )
    {
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiAPartUnitIdx -= m_uiAbsZorderIdx;
      return this;
    }
  }

  uiAPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAbove;
}

TComDataCU* TComDataCU::getPUAboveLeft( UInt& uiALPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdx       = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiConvertRelToAbsIdx[m_uiAbsZorderIdx];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( uiAbsPartIdx % uiNumPartInCUWidth )
  {
    if( uiAbsPartIdx / uiNumPartInCUWidth )
    {
      uiALPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx - uiNumPartInCUWidth - 1 ];
      if ( ( uiAbsPartIdx % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdx / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
      {
        return m_pcPic->getCU( getAddr() );
      }
      else
      {
        uiALPartUnitIdx -= m_uiAbsZorderIdx;
        return this;
      }
    }
    uiALPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx + getPic()->getNumPartInCU() - uiNumPartInCUWidth - 1 ];
    return m_pcCUAbove;
  }

  if( uiAbsPartIdx / uiNumPartInCUWidth )
  {
    uiALPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdx - 1 ];
    return m_pcCULeft;
  }

  uiALPartUnitIdx = g_auiConvertAbsToRelIdx[ m_pcPic->getNumPartInCU() - 1 ];
  return m_pcCUAboveLeft;
}

TComDataCU* TComDataCU::getPUAboveRight( UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxRT     = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiConvertPartIdxToPelX[uiAbsPartIdxRT] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }

  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsZorderIdx;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }

  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }

  uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}

TComDataCU* TComDataCU::getPUBelowLeft( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiConvertPartIdxToPelY[uiAbsPartIdxLB] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }

  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsZorderIdx;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }

  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}




TComDataCU* TComDataCU::getPUBelowLeftAdi(UInt& uiBLPartUnitIdx, UInt uiPuHeight,  UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxLB     = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + ((m_puhHeight[0] / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiConvertPartIdxToPelY[uiAbsPartIdxLB] + uiPuHeight ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }

  if ( uiAbsPartIdxLB / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 )
  {
    if ( uiAbsPartIdxLB % uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( ( (uiAbsPartIdxLB % uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB % uiNumPartInCUWidth) ) || ( (uiAbsPartIdxLB / uiNumPartInCUWidth) == (uiAbsZorderCUIdxLB / uiNumPartInCUWidth) ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsZorderIdx;
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
    return m_pcCULeft;
  }

  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}



TComDataCU* TComDataCU::getPUAboveRightAdi(UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx )
{
  UInt uiAbsPartIdxRT     = g_auiConvertRelToAbsIdx[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhWidth[0] / m_pcPic->getMinCUWidth()) - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiConvertPartIdxToPelX[uiAbsPartIdxRT] + uiPuWidth ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }

  if ( uiAbsPartIdxRT % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )
  {
    if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
    {
      if ( uiCurrPartUnitIdx > g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( ( uiAbsPartIdxRT % uiNumPartInCUWidth == uiAbsZorderCUIdx % uiNumPartInCUWidth ) || ( uiAbsPartIdxRT / uiNumPartInCUWidth == uiAbsZorderCUIdx / uiNumPartInCUWidth ) )
        {
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsZorderIdx;
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    return m_pcCUAbove;
  }

  if ( uiAbsPartIdxRT / uiNumPartInCUWidth )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }

  uiARPartUnitIdx = g_auiConvertAbsToRelIdx[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  return m_pcCUAboveRight;
}



PartSize TComDataCU::getMostProbablePartSize( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  PartSize    eAbove, eLeft;
  PartSize    eMostProbable;

  // Get left split flag
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  eLeft  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) < m_puhDepth[uiAbsPartIdx] ) ? SIZE_2Nx2N : ( ( pcTempCU->getDepth( uiTempPartIdx ) > m_puhDepth[uiAbsPartIdx] ) ? SIZE_NxN : pcTempCU->getPartitionSize( uiTempPartIdx ) ) ) : SIZE_NONE;

  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  Int  iMostProbable	= getMostProbableIntraDirLumaAdi( uiAbsPartIdx );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  iLeftPredMode  = pcTempCU ? pcTempCU->getPredictionMode( uiTempPartIdx ) : MODE_NONE;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;

  // TM_INTRA
  if (getSlice()->getSPS()->getUseTMI())
  {
    if (iLeftIntraDir==33)
      iLeftIntraDir =2;
  }

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;

  // TM_INTRA
  if (getSlice()->getSPS()->getUseTMI())
  {
	  if (iAboveIntraDir==33)
      iAboveIntraDir =2;
  }

  iMostProbable  = Min( iLeftIntraDir, iAboveIntraDir );

  return ( NOT_VALID == iMostProbable ) ? 2 : iMostProbable;
}

Int TComDataCU::getMostProbableIntraDirLumaAdi( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir, iMostProbable;
  Int         iIntraIdx;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;

  if (iLeftIntraDir>=0)
  {
    iIntraIdx= pcTempCU->getIntraSizeIdx(uiTempPartIdx);

  // TM_INTRA
    if (getSlice()->getSPS()->getUseTMI())
    {
      if (iLeftIntraDir == 33)
        iLeftIntraDir = 2;
      else
        iLeftIntraDir=g_aucIntraModeConv[iIntraIdx][iLeftIntraDir];
    }
    else
      iLeftIntraDir=g_aucIntraModeConv[iIntraIdx][iLeftIntraDir];

  }

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : 2 ) : NOT_VALID;
  if( iAboveIntraDir >= 0 )
  {
    iIntraIdx= pcTempCU->getIntraSizeIdx(uiTempPartIdx);
// TM_INTRA
    if (getSlice()->getSPS()->getUseTMI())
    {
      if (iAboveIntraDir == 33)
        iAboveIntraDir = 2;
      else
        iAboveIntraDir = g_aucIntraModeConv[iIntraIdx][iAboveIntraDir];
    }
    else
      iAboveIntraDir = g_aucIntraModeConv[iIntraIdx][iAboveIntraDir];
  }

  iMostProbable  = Min( iLeftIntraDir, iAboveIntraDir );

  return ( NOT_VALID == iMostProbable ) ? 2 : iMostProbable;
}

Void TComDataCU::setCuCbf( UInt uiAbsPartIdx, UInt uiLumaTrMode, UInt uiChromaTrMode, UInt uiPartDepth )
{
  UInt uiDepth = m_puhDepth[ uiAbsPartIdx ] + uiPartDepth;

  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_LUMA]]     + uiAbsPartIdx, uiLumaTrMode,   0, uiDepth );
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_U]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
  xCalcCuCbf( m_puhCbf[g_aucConvertTxtTypeToIdx[TEXT_CHROMA_V]] + uiAbsPartIdx, uiChromaTrMode, 0, uiDepth );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;

  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;

  return uiCtx;
}

UInt TComDataCU::getCtxIntraDirChroma( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getChromaIntraDir( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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

UInt TComDataCU::getCtxAlfCtrlFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;

  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->getAlfCtrlFlag( uiTempPartIdx ) : 0;

  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->getAlfCtrlFlag( uiTempPartIdx ) : 0;

  return uiCtx;
}

UInt TComDataCU::getCtxSkipFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;

  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;

  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;

  return uiCtx;
}

UInt TComDataCU::getCtxCIPFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;

  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getCIPflag( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->isIntra( uiTempPartIdx ) && pcTempCU->getCIPflag( uiTempPartIdx ) > 0 ) ? 1 : 0 ) : 0;

  return uiCtx;
}

UInt TComDataCU::getCtxPredMode( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;

  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;

  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? ( pcTempCU->getPredictionMode( uiTempPartIdx ) > 1 ? 1 : 0 ) : 0;

  return uiCtx;
}

UInt TComDataCU::getCtxInterDir( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;

  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getInterDir( uiTempPartIdx ) % 3 ) ? 0 : 1 ) : 0;

  // Get BCBP of Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  getMvField( pcTempCU, uiTempPartIdx, eRefPicList, cMvFieldTemp );
  uiCtx += cMvFieldTemp.getRefIdx() > 0 ? 1 : 0;

  // Get BCBP of Above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
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
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  if ( pcTempCU )
  {
	iTempTrMode = pcTempCU->getDepth( uiTempPartIdx ) + pcTempCU->getTransformIdx( uiTempPartIdx ) - getDepth( uiAbsPartIdx ) - uiMinTrDepth;
	uiCtx  = ( 0 <= iTempTrMode && iTempTrMode < 2 ) ? 1 : 0;    // 2 is max trmode
  }

  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsZorderIdx + uiAbsPartIdx );
  if ( pcTempCU )
  {
	iTempTrMode = pcTempCU->getDepth( uiTempPartIdx ) + pcTempCU->getTransformIdx( uiTempPartIdx ) - getDepth( uiAbsPartIdx ) - uiMinTrDepth;
	uiCtx += ( 0 <= iTempTrMode && iTempTrMode < 2 ) ? 1 : 0;    // 2 is max trmode
  }

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
  return (((m_uiAbsZorderIdx + uiAbsPartIdx)% uiPartNumb) == 0);
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
  case SIZE_SHV_LT:
    if( uiPartIdx == 0 )
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
    }
    else
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ*3);                          break;
    }
  case SIZE_SHV_RT:
    if( uiPartIdx == 0 )
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
      memset( m_puhInterDir + uiAbsPartIdx+(uiCurrPartNumQ<<1), uiDir, sizeof(UChar)*uiCurrPartNumQ<<1);     break;
    }
    else
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
    }
  case SIZE_SHV_LB:
    if( uiPartIdx == 0 )
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ<<1 );                        break;
      memset( m_puhInterDir + uiAbsPartIdx+(uiCurrPartNumQ*3), uiDir, sizeof(UChar)*uiCurrPartNumQ);         break;
    }
    else
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
    }
  case SIZE_SHV_RB:
    if( uiPartIdx == 0 )
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ*3 );                         break;
    }
    else
    {
      memset( m_puhInterDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumQ );                           break;
    }
  default:
    assert( 0 );
  }
}

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
  case SIZE_SHV_LT:
    if( uiPartIdx == 0 )
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
      }
      break;
    }
    else
    {
      pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
        pi1[i] = iMVPIdx;
        pi2[i] = iMVPIdx;
      }
      break;
    }
  case SIZE_SHV_RT:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
      Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ*3);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
        pi1[i] = iMVPIdx;
        pi2[i] = iMVPIdx;
      }
      break;
    }
    else
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
      }
      break;
    }
  case SIZE_SHV_LB:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ*3);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
        pi1[i] = iMVPIdx;
        pi2[i] = iMVPIdx;
      }
      break;
    }
    else
    {
      pi = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
      }
      break;
    }
  case SIZE_SHV_RB:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPIdx[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPIdx;
        pi1[i] = iMVPIdx;
        pi2[i] = iMVPIdx;
      }
      break;
    }
    else
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
  case SIZE_SHV_LT:
    if( uiPartIdx == 0 )
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
      break;
    }
    else
    {
      pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
        pi1[i] = iMVPNum;
        pi2[i] = iMVPNum;
      }
      break;
    }
  case SIZE_SHV_RT:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);
      Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ*3);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
        pi1[i] = iMVPNum;
        pi2[i] = iMVPNum;
      }
      break;
    }
    else
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
      break;
    }
  case SIZE_SHV_LB:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ*3);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
        pi1[i] = iMVPNum;
        pi2[i] = iMVPNum;
      }
      break;
    }
    else
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
      break;
    }
  case SIZE_SHV_RB:
    if( uiPartIdx == 0 )
    {
      pi  = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      Int* pi1 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ);
      Int* pi2 = m_apiMVPNum[eRefPicList] + uiAbsPartIdx + (uiCurrPartNumQ<<1);

      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
        pi1[i] = iMVPNum;
        pi2[i] = iMVPNum;
      }
      break;
    }
    else
    {
      pi = m_apiMVPNum[eRefPicList] + uiAbsPartIdx;
      for (i = 0; i < uiCurrPartNumQ; i++)
      {
        pi[i] = iMVPNum;
      }
      break;
    }

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
  case SIZE_2NxnU:    iNumPart = 2; break;
  case SIZE_2NxnD:    iNumPart = 2; break;
  case SIZE_nLx2N:    iNumPart = 2; break;
  case SIZE_nRx2N:    iNumPart = 2; break;
  case SIZE_SHV_LT:   iNumPart = 2; break;
  case SIZE_SHV_RT:   iNumPart = 2; break;
  case SIZE_SHV_LB:   iNumPart = 2; break;
  case SIZE_SHV_RB:   iNumPart = 2; break;

  default:            assert (0);   break;
  }

  return  iNumPart;
}

UChar TComDataCU::getNumVirtPartInter()
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

  case SIZE_SHV_LT:   iNumPart = 4; break;
  case SIZE_SHV_RT:   iNumPart = 4; break;
  case SIZE_SHV_LB:   iNumPart = 4; break;
  case SIZE_SHV_RB:   iNumPart = 4; break;

  default:            assert (0);   break;
  }

  return  iNumPart;
}


Void TComDataCU::getLeftTopIdxSizeVirt  ( UInt uiVirtPartIdx, UInt& ruiLTIdx,    Int&       riWidth,      Int&         riHeight )
{
  assert(uiVirtPartIdx < 4);

  switch ( m_pePartSize[0] )
  {
  case SIZE_2Nx2N: riWidth = getWidth(0);      riHeight = getHeight(0);      ruiLTIdx = 0;                                             break;
  case SIZE_2NxN:  riWidth = getWidth(0);      riHeight = getHeight(0) >> 1; ruiLTIdx = ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 1; break;
  case SIZE_Nx2N:  riWidth = getWidth(0) >> 1; riHeight = getHeight(0);      ruiLTIdx = ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 2; break;
  case SIZE_NxN:   riWidth = getWidth(0) >> 1; riHeight = getHeight(0) >> 1; ruiLTIdx = ( m_uiNumPartition >> 2 ) * uiVirtPartIdx;         break;
  case SIZE_2NxnU:
    riWidth     = getWidth(0);
    riHeight    = ( uiVirtPartIdx == 0 ) ?  getHeight(0) >> 2 : ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 );
    ruiLTIdx = ( uiVirtPartIdx == 0 ) ? 0 : m_uiNumPartition >> 3;
    break;
  case SIZE_2NxnD:
    riWidth     = getWidth(0);
    riHeight    = ( uiVirtPartIdx == 0 ) ?  ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 ) : getHeight(0) >> 2;
    ruiLTIdx = ( uiVirtPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 3);
    break;
  case SIZE_nLx2N:
    riWidth     = ( uiVirtPartIdx == 0 ) ? getWidth(0) >> 2 : ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 );
    riHeight    = getHeight(0);
    ruiLTIdx = ( uiVirtPartIdx == 0 ) ? 0 : m_uiNumPartition >> 4;
    break;
  case SIZE_nRx2N:
    riWidth     = ( uiVirtPartIdx == 0 ) ? ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 ) : getWidth(0) >> 2;
    riHeight    = getHeight(0);
    ruiLTIdx = ( uiVirtPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
    break;
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    riWidth     = ( getWidth(0) >> 1 );
    riHeight    = ( getHeight(0) >> 1 );
    ruiLTIdx = (m_uiNumPartition >> 2)*uiVirtPartIdx;
    break;
  default: assert (0); break;
  }
}


Bool TComDataCU::isNonRectPart ( UInt uiPartIdx )
{
  if ((m_pePartSize[0] == SIZE_SHV_LT && uiPartIdx == 1) || (m_pePartSize[0] >= SIZE_SHV_RT && m_pePartSize[0] <= SIZE_SHV_RB && uiPartIdx == 0))
    return true;
  else
    return false;
}

Void TComDataCU::getNonRectPartinfo ( UInt uiPartIdx, Int* piStartX, Int* piStartY, Int* piRows, Int* piCols )
{
  assert ( isNonRectPart(uiPartIdx) );

  UChar uhXpos   = m_puhWidth[0]>>1;
  UChar uhYpos   = m_puhHeight[0]>>1;
  UChar uhWidth  = m_puhWidth[0];
  UChar uhHeight = m_puhHeight[0];

  switch (m_pePartSize[0])
  {
  case SIZE_SHV_LT:
    piStartX[0] = uhXpos;
    piStartY[0] = 0;
    piRows  [0] = uhYpos;
    piCols  [0] = uhWidth - uhXpos;

    piStartX[1] = 0;
    piStartY[1] = uhYpos;
    piRows  [1] = uhHeight - uhYpos;
    piCols  [1] = uhWidth;
    break;

  case SIZE_SHV_RT:
    piStartX[0] = 0;
    piStartY[0] = 0;
    piRows  [0] = uhYpos;
    piCols  [0] = uhXpos;

    piStartX[1] = 0;
    piStartY[1] = uhYpos;
    piRows  [1] = uhHeight - uhYpos;
    piCols  [1] = uhWidth;
    break;

  case SIZE_SHV_LB:
    piStartX[0] = 0;
    piStartY[0] = 0;
    piRows  [0] = uhYpos;
    piCols  [0] = uhWidth;

    piStartX[1] = uhXpos;
    piStartY[1] = uhYpos;
    piRows  [1] = uhHeight - uhYpos;
    piCols  [1] = uhWidth - uhXpos;
    break;

  case SIZE_SHV_RB:
    piStartX[0] = 0;
    piStartY[0] = 0;
    piRows  [0] = uhYpos;
    piCols  [0] = uhWidth;

    piStartX[1] = 0;
    piStartY[1] = uhYpos;
    piRows  [1] = uhHeight - uhYpos;
    piCols  [1] = uhXpos;
    break;

  default:
    assert(0);
    break;
  }
}


UInt TComDataCU::getFirstPartIdx(UInt uiPartIdx, UInt uiCUPartAddr)
{
  assert(uiPartIdx < 4);
  assert(!(m_pePartSize[uiCUPartAddr] != SIZE_NxN && uiPartIdx > 1));
  assert(uiCUPartAddr%(m_pcPic->getNumPartInCU() >> (getDepth(uiCUPartAddr) << 1)) == 0);

  UInt uiPartAddr;
  UInt uiNumPartition = m_pcPic->getNumPartInCU() >> (getDepth(uiCUPartAddr) << 1);

  {
    switch ( m_pePartSize[uiCUPartAddr] )
    {
    case SIZE_2Nx2N:uiPartAddr = 0;                                             break;
    case SIZE_2NxN: uiPartAddr = ( uiPartIdx == 0 )? 0 : uiNumPartition >> 1; break;
    case SIZE_Nx2N: uiPartAddr = ( uiPartIdx == 0 )? 0 : uiNumPartition >> 2; break;
    case SIZE_NxN:  uiPartAddr = ( uiNumPartition >> 2 ) * uiPartIdx;         break;
    case SIZE_2NxnU:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : (uiNumPartition >> 1) + (uiNumPartition >> 3);
      break;
    case SIZE_nLx2N:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : uiNumPartition >> 4;
      break;
    case SIZE_nRx2N:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : (uiNumPartition >> 2) + (uiNumPartition >> 4);
      break;
    case SIZE_SHV_LT:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : uiNumPartition >> 2;
      break;
    case SIZE_SHV_RT:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : (uiNumPartition >> 2);
      break;
    case SIZE_SHV_LB:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : (uiNumPartition >> 1);
      break;
    case SIZE_SHV_RB:
      uiPartAddr = ( uiPartIdx == 0 ) ? 0 : (uiNumPartition >> 2) + (uiNumPartition >> 1) ;
      break;
    default: assert (0); break;
    }
  }

  return uiCUPartAddr+uiPartAddr;
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
    case SIZE_SHV_LT:
      riWidth     = ( uiPartIdx == 0 ) ? ( getWidth(0) >> 1 ) : getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ? ( getHeight(0) >> 1 ) : getHeight(0);
      ruiPartAddr = 0;
      break;
    case SIZE_SHV_RT:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) : ( getWidth(0) >> 1 );
      riHeight    = ( uiPartIdx == 0 ) ? getHeight(0) : ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2);
      break;
    case SIZE_SHV_LB:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) : ( getWidth(0) >> 1 );
      riHeight    = ( uiPartIdx == 0 ) ? getHeight(0) : ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 1);
      break;
    case SIZE_SHV_RB:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) : ( getWidth(0) >> 1 );
      riHeight    = ( uiPartIdx == 0 ) ? getHeight(0) : ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2) + (m_uiNumPartition >> 1) ;
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

Void TComDataCU::deriveLeftRightTopIdx ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsZorderIdx;
  ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ ruiPartIdxLT ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];

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
    case SIZE_SHV_LT:
      if (uiPartIdx == 0)
      {
        ruiPartIdxLT = m_uiAbsZorderIdx;
				ruiPartIdxRT -=	( m_uiNumPartition >> 2 );
      }
      break;
    case SIZE_SHV_RT:
			if (uiPartIdx == 1)
      {
        ruiPartIdxLT += ( m_uiNumPartition >> 2 ); break;
      }
      break;
    case SIZE_SHV_LB:
			if (uiPartIdx == 1)
      {
        ruiPartIdxLT += ( m_uiNumPartition >> 1 );         ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ); break;
      }
      break;
    case SIZE_SHV_RB:
			if (uiPartIdx == 1)
      {
        ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * 3;         ruiPartIdxRT +=  ( m_uiNumPartition >> 1 ); break;
      }
      break;
    default:
      assert (0);
    break;
  }

}

Void TComDataCU::deriveLeftBottomIdx( PartSize      eCUMode,   UInt  uiPartIdx,      UInt&      ruiPartIdxLB )
{
  ruiPartIdxLB      = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth()];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:	ruiPartIdxLB += m_uiNumPartition >> 1;		break;
    case SIZE_2NxN:  ruiPartIdxLB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;		break;
    case SIZE_Nx2N:  ruiPartIdxLB += ( uiPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;		break;
    case SIZE_NxN:   ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiPartIdx;		break;

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
    case SIZE_SHV_LT:
      if (uiPartIdx == 1)
      {
        ruiPartIdxLB += ( m_uiNumPartition >> 1 );
      }
      break;
    case SIZE_SHV_RT:
      if (uiPartIdx == 0)
      {
        ruiPartIdxLB += ( m_uiNumPartition >> 2 );
      }
      else //if (uiPartIdx == 1)
      {
        ruiPartIdxLB += ( m_uiNumPartition >> 1 );
      }
      break;
    case SIZE_SHV_LB:
      ruiPartIdxLB += ( m_uiNumPartition >> 1 );
      break;
    case SIZE_SHV_RB:
      if (uiPartIdx == 0)
      {
        ruiPartIdxLB += ( m_uiNumPartition >> 1 );
      }
      else //if (uiPartIdx == 1)
      {
        ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * 3;
      }
      break;
    default:
      assert (0);
      break;
  }
}

Void TComDataCU::deriveLeftTopIdxVirt( PartSize eCUMode, UInt uiVirtPartIdx, UInt& ruiPartIdxLT )
{
	ruiPartIdxLT = m_uiAbsZorderIdx;

  switch ( m_pePartSize[0] )
  {
  case SIZE_2Nx2N:    break;
  case SIZE_2NxN:  ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 1; break;
  case SIZE_Nx2N:  ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 2; break;
  case SIZE_NxN:
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
		ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * uiVirtPartIdx;         break;
  case SIZE_2NxnU:
    ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
    break;
  case SIZE_2NxnD:
    ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
    break;
  case SIZE_nLx2N:
    ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 4;
    break;
  case SIZE_nRx2N:
    ruiPartIdxLT += ( uiVirtPartIdx == 0 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
    break;
  default:
    assert (0);
    break;
  }
}

Void TComDataCU::deriveRightTopIdxVirt( PartSize eCUMode, UInt uiVirtPartIdx, UInt& ruiPartIdxRT )
{
	ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];

  switch ( m_pePartSize[0] )
  {
  case SIZE_2Nx2N:    break;
  case SIZE_2NxN:  ruiPartIdxRT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 1;  break;
  case SIZE_Nx2N:  ruiPartIdxRT -= ( uiVirtPartIdx == 1 )? 0 : m_uiNumPartition >> 2;  break;
  case SIZE_NxN:
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
		ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ) * ( uiVirtPartIdx - 1 ); break;
  case SIZE_2NxnU:
    ruiPartIdxRT += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
    break;
  case SIZE_2NxnD:
    ruiPartIdxRT += ( uiVirtPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
    break;
  case SIZE_nLx2N:
    ruiPartIdxRT -= ( uiVirtPartIdx == 1 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
    break;
  case SIZE_nRx2N:
    ruiPartIdxRT -= ( uiVirtPartIdx == 1 )? 0 : m_uiNumPartition >> 4;
    break;
/*
  case SIZE_2NxN:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];
    }
    else //if (uiVirtPartIdx == 1)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight())*m_pcPic->getNumPartInWidth()];
    }
    break;
  case SIZE_Nx2N:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhXPartPos[0] / m_pcPic->getMinCUWidth() -1];
    }
    else //if (uiVirtPartIdx == 1)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];
    }
    break;
  case SIZE_NxN:
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhXPartPos[0] / m_pcPic->getMinCUWidth() -1];
    }
    else if (uiVirtPartIdx == 1)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];
    }
    else if (uiVirtPartIdx == 2)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhXPartPos[0] / m_pcPic->getMinCUWidth() -1 + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight())*m_pcPic->getNumPartInWidth()];
    }
    else // if (uiVirtPartIdx == 3)
    {
      ruiPartIdxRT = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight())*m_pcPic->getNumPartInWidth()];
    }
    break;
    */
  default:
    assert (0);
    break;
  }
}

Void TComDataCU::deriveLeftBottomIdxVirt  ( PartSize eCUMode, UInt uiVirtPartIdx, UInt& ruiPartIdxLB )
{
  ruiPartIdxLB      = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth()];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:	ruiPartIdxLB += m_uiNumPartition >> 1;		break;
    case SIZE_2NxN:  ruiPartIdxLB += ( uiVirtPartIdx == 0 )? 0 : m_uiNumPartition >> 1;		break;
    case SIZE_Nx2N:  ruiPartIdxLB += ( uiVirtPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;		break;
    case SIZE_NxN:
    case SIZE_SHV_LT:
    case SIZE_SHV_RT:
    case SIZE_SHV_LB:
    case SIZE_SHV_RB:
			ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiVirtPartIdx;		break;
    case SIZE_2NxnU:
      ruiPartIdxLB += ( uiVirtPartIdx == 0 ) ? -((Int)m_uiNumPartition >> 3) : m_uiNumPartition >> 1;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLB += ( uiVirtPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3): m_uiNumPartition >> 1;
      break;
    case SIZE_nLx2N:
      ruiPartIdxLB += ( uiVirtPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 4);
      break;
    case SIZE_nRx2N:
      ruiPartIdxLB += ( uiVirtPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default:
      assert (0);
      break;
  }
/*
  switch ( m_pePartSize[0] )
  {
  case SIZE_2Nx2N:
    ruiPartIdxLB      = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    break;
  case SIZE_2NxN:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    }
    else //if (uiVirtPartIdx == 1)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    }
    break;
  case SIZE_Nx2N:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxLB      = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    }
    else //if (uiVirtPartIdx == 1)
    {
      ruiPartIdxLB      = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth() + m_puhXPartPos[0] / m_pcPic->getMinCUWidth()];
    }
    break;
  case SIZE_NxN:
  case SIZE_SHV_LT:
  case SIZE_SHV_RT:
  case SIZE_SHV_LB:
  case SIZE_SHV_RB:
    if (uiVirtPartIdx == 0)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    }
    else if (uiVirtPartIdx == 1)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhYPartPos[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth() + m_puhXPartPos[0] / m_pcPic->getMinCUWidth()];
    }
    else if (uiVirtPartIdx == 2)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth()];
    }
    else // if (uiVirtPartIdx == 3)
    {
      ruiPartIdxLB = g_auiConvertAbsToRelIdx [g_auiConvertRelToAbsIdx[ m_uiAbsZorderIdx ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth() + m_puhXPartPos[0] / m_pcPic->getMinCUWidth()];
    }
    break;
  default:
    assert (0);
    break;
  }
  */
}
//--

Void TComDataCU::deriveLeftRightTopIdxAdi ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiNumPartInWidth = (m_puhWidth[0]/m_pcPic->getMinCUWidth())>>uiPartDepth;
  ruiPartIdxLT = m_uiAbsZorderIdx + uiPartOffset;
  ruiPartIdxRT = g_auiConvertAbsToRelIdx[ g_auiConvertRelToAbsIdx[ ruiPartIdxLT ] + uiNumPartInWidth - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxAdi( UInt& ruiPartIdxLB, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiAbsIdx;
  UInt uiMinCuWidth, uiWidthInMinCus;

  uiMinCuWidth    = getPic()->getMinCUWidth();
  uiWidthInMinCus = (getWidth(0)/uiMinCuWidth)>>uiPartDepth;
  uiAbsIdx        = getZorderIdxInCU()+uiPartOffset+(m_uiNumPartition>>(uiPartDepth<<1))-1;
  uiAbsIdx        = g_auiConvertRelToAbsIdx[uiAbsIdx]-(uiWidthInMinCus-1);
  ruiPartIdxLB    = g_auiConvertAbsToRelIdx[uiAbsIdx];
}

Void TComDataCU::getMvPred( PartSize eCUMode, UInt uiPartIdx, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred )
{
  UInt uiPartIdxLT, uiPartIdxRT;
  UInt uiAbsPartIdxL, uiAbsPartIdxA, uiAbsPartIdxAR;

  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );

  TComDataCU* pcCUL   = getPULeft       ( uiAbsPartIdxL,  uiPartIdxLT );
  TComDataCU* pcCUA   = getPUAbove      ( uiAbsPartIdxA,  uiPartIdxLT );
  TComDataCU* pcCUAR  = getPUAboveRight ( uiAbsPartIdxAR, uiPartIdxRT );

  getMvField( pcCUL,  uiAbsPartIdxL,  eRefPicList, m_cMvFieldA );
  getMvField( pcCUA,  uiAbsPartIdxA,  eRefPicList, m_cMvFieldB );
  getMvField( pcCUAR, uiAbsPartIdxAR, eRefPicList, m_cMvFieldC );

  if ( pcCUAR == NULL /*m_cMvFieldC.getRefIdx() < 0*/ )  // Block Not Available
  {
    UInt uiAbsPartIdxAL;
    TComDataCU* pcCUAL = getPUAboveLeft( uiAbsPartIdxAL, uiPartIdxLT );
    getMvField( pcCUAL, uiAbsPartIdxAL, eRefPicList, m_cMvFieldC );
  }

  if ( ( ((eCUMode == SIZE_2NxN) || (eCUMode == SIZE_2NxnU) || (eCUMode == SIZE_2NxnD)) && uiPartIdx == 1 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldA.getRefIdx(), iRefIdx) ) ||
       ( ((eCUMode == SIZE_Nx2N) || (eCUMode == SIZE_nLx2N) || (eCUMode == SIZE_nRx2N)) && uiPartIdx == 0 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldA.getRefIdx(), iRefIdx) ) ||
       ( m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldA.getRefIdx(), iRefIdx) && !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldB.getRefIdx(), iRefIdx) && !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldC.getRefIdx(), iRefIdx) ) ||
       ( pcCUA == NULL && pcCUAR == NULL) )
  {
    rcMvPred.set (m_cMvFieldA.getHor(), m_cMvFieldA.getVer());
    rcMvPred.setD (m_cMvFieldA.getHorD(), m_cMvFieldA.getVerD());
  }
  else if ( ( ((eCUMode == SIZE_2NxN) || (eCUMode == SIZE_2NxnU) || (eCUMode == SIZE_2NxnD)) && uiPartIdx == 0 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldB.getRefIdx(), iRefIdx) ) ||
       ( !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldA.getRefIdx(), iRefIdx) && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldB.getRefIdx(), iRefIdx) && !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldC.getRefIdx(), iRefIdx) ))
  {
    rcMvPred.set (m_cMvFieldB.getHor(), m_cMvFieldB.getVer());
	    rcMvPred.setD (m_cMvFieldB.getHorD(), m_cMvFieldB.getVerD());
  }
  else if ( ( ((eCUMode == SIZE_Nx2N) || (eCUMode == SIZE_nLx2N) || (eCUMode == SIZE_nRx2N)) && uiPartIdx == 1 && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldC.getRefIdx(), iRefIdx)) ||
       ( !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldA.getRefIdx(), iRefIdx) && !m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldB.getRefIdx(), iRefIdx) && m_pcSlice->isEqualRef(eRefPicList, m_cMvFieldC.getRefIdx(), iRefIdx) ))
  {
    rcMvPred.set (m_cMvFieldC.getHor(), m_cMvFieldC.getVer());
    rcMvPred.setD (m_cMvFieldC.getHorD(), m_cMvFieldC.getVerD());
  }
  else
  {
    rcMvPred.setHor ( Median (m_cMvFieldA.getHor(), m_cMvFieldB.getHor(), m_cMvFieldC.getHor()) );
    rcMvPred.setVer ( Median (m_cMvFieldA.getVer(), m_cMvFieldB.getVer(), m_cMvFieldC.getVer()) );
    rcMvPred.setHorD ( Median (m_cMvFieldA.getHorD(), m_cMvFieldB.getHorD(), m_cMvFieldC.getHorD()) );
    rcMvPred.setVerD ( Median (m_cMvFieldA.getVerD(), m_cMvFieldB.getVerD(), m_cMvFieldC.getVerD()) );
  }

  if ( getSlice()->getUseMVAC() )
    restrictMvpAccuracy(rcMvPred);

  m_cMvPred = rcMvPred;
}

Void TComDataCU::restrictMvpAccuracy( TComMv& rcMvPred )
{
  Int	iMvPredHor	=	rcMvPred.getHor();
  Int	iMvPredVer	=	rcMvPred.getVer();

  UInt uiPredHor = (UInt)abs(iMvPredHor);
  UInt uiPredVer = (UInt)abs(iMvPredVer);

  Int iSignHor = iMvPredHor >=0 ? 1 : -1;
  Int iSignVer = iMvPredVer >=0 ? 1 : -1;

  Int iShiftDigit	=	1;

  uiPredHor >>= iShiftDigit;  uiPredHor <<= iShiftDigit;
  uiPredVer >>= iShiftDigit;  uiPredVer <<= iShiftDigit;

  iMvPredHor = uiPredHor * iSignHor;
  iMvPredVer = uiPredVer * iSignVer;

  rcMvPred.setHor(iMvPredHor);
  rcMvPred.setVer(iMvPredVer);
}

AMVP_MODE TComDataCU::getAMVPMode(UInt uiIdx)
{
  if (m_pcSlice->getSPS()->getAMVPMode(m_puhDepth[uiIdx]) == AM_IMPL && (m_puhDepth[uiIdx] != 0 || m_pePartSize[uiIdx] != SIZE_2Nx2N))  //T0428
  {
    return AM_EXPL;
  }

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

  pInfo->m_acMvCand[pInfo->iN++] = cMvPred; 	//dummy mv

	//-- Get Spatial MV
	UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
	UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
	Bool bAdded = false;
	Int iLeftMvIdx = -1;
	Int iAboveMvIdx = -1;
	Int iCornerMvIdx = -1;

	// Sickle shaped partition in SHV modes
	if ( isNonRectPart( uiPartIdx) )
	{
		Int iPartIdxLStart0, iPartIdxLStart1;
		Int iPartIdxLEnd0, iPartIdxLEnd1;

		Int iPartIdxAStart0, iPartIdxAStart1;
		Int iPartIdxAEnd0, iPartIdxAEnd1;

		UInt uiPartIdxAR;
		UInt uiPartIdxBL;
		UInt uiPartIdxAL;

		UInt uiPartIdxLTVirt0, uiPartIdxLTVirt1, uiPartIdxLTVirt2, uiPartIdxLTVirt3;
		UInt uiPartIdxRTVirt0, uiPartIdxRTVirt1, uiPartIdxRTVirt2, uiPartIdxRTVirt3;
		UInt uiPartIdxLBVirt0, uiPartIdxLBVirt1, uiPartIdxLBVirt2, uiPartIdxLBVirt3;

		if (eCUMode == SIZE_SHV_LT)
		{
			deriveLeftTopIdxVirt ( eCUMode, 1, uiPartIdxLTVirt1 );
			deriveLeftTopIdxVirt ( eCUMode, 2, uiPartIdxLTVirt2 );

			deriveRightTopIdxVirt ( eCUMode, 1, uiPartIdxRTVirt1 );
			deriveRightTopIdxVirt ( eCUMode, 2, uiPartIdxRTVirt2 );

			deriveLeftBottomIdxVirt( eCUMode, 1, uiPartIdxLBVirt1 );
			deriveLeftBottomIdxVirt( eCUMode, 2, uiPartIdxLBVirt2 );

			iPartIdxLStart0 = uiPartIdxLTVirt1;
			iPartIdxLEnd0 = uiPartIdxLBVirt1;
			iPartIdxLStart1 = uiPartIdxLTVirt2;
			iPartIdxLEnd1 = uiPartIdxLBVirt2;

			iPartIdxAStart0 = uiPartIdxLTVirt2;
			iPartIdxAEnd0 = uiPartIdxRTVirt2;
			iPartIdxAStart1 = uiPartIdxLTVirt1;
			iPartIdxAEnd1 = uiPartIdxRTVirt1;

			uiPartIdxAR = uiPartIdxRTVirt1;
			uiPartIdxBL = uiPartIdxLBVirt2;
			uiPartIdxAL = uiPartIdxLTVirt1;
		}
		else if (eCUMode == SIZE_SHV_RT)
		{
			deriveLeftTopIdxVirt ( eCUMode, 0, uiPartIdxLTVirt0 );
			deriveLeftTopIdxVirt ( eCUMode, 2, uiPartIdxLTVirt2 );
			deriveLeftTopIdxVirt ( eCUMode, 3, uiPartIdxLTVirt3 );

			deriveRightTopIdxVirt ( eCUMode, 0, uiPartIdxRTVirt0 );
			deriveRightTopIdxVirt ( eCUMode, 3, uiPartIdxRTVirt3 );

			deriveLeftBottomIdxVirt( eCUMode, 0, uiPartIdxLBVirt0 );
			deriveLeftBottomIdxVirt( eCUMode, 2, uiPartIdxLBVirt2 );

			iPartIdxLStart0 = uiPartIdxLTVirt0;
			iPartIdxLEnd0 = uiPartIdxLBVirt0;
			iPartIdxLStart1 = uiPartIdxLTVirt2;
			iPartIdxLEnd1 = uiPartIdxLBVirt2;

			iPartIdxAStart0 = uiPartIdxLTVirt0;
			iPartIdxAEnd0 = uiPartIdxRTVirt0;
			iPartIdxAStart1 = -1;
			iPartIdxAEnd1 = -1;

			uiPartIdxAR = uiPartIdxRTVirt0;
			uiPartIdxBL = uiPartIdxLBVirt2;
			uiPartIdxAL = uiPartIdxLTVirt0;
		}
		else if (eCUMode == SIZE_SHV_LB)
		{
			deriveLeftTopIdxVirt ( eCUMode, 0, uiPartIdxLTVirt0 );
			deriveLeftTopIdxVirt ( eCUMode, 1, uiPartIdxLTVirt1 );
			deriveLeftTopIdxVirt ( eCUMode, 3, uiPartIdxLTVirt3 );

			deriveRightTopIdxVirt ( eCUMode, 0, uiPartIdxRTVirt0 );
			deriveRightTopIdxVirt ( eCUMode, 1, uiPartIdxRTVirt1 );

			deriveLeftBottomIdxVirt( eCUMode, 0, uiPartIdxLBVirt0 );
			deriveLeftBottomIdxVirt( eCUMode, 3, uiPartIdxLBVirt3 );

			iPartIdxLStart0 = uiPartIdxLTVirt0;
			iPartIdxLEnd0 = uiPartIdxLBVirt0;
			iPartIdxLStart1 = -1;
			iPartIdxLEnd1 = -1;

			iPartIdxAStart0 = uiPartIdxLTVirt0;
			iPartIdxAEnd0 = uiPartIdxRTVirt0;
			iPartIdxAStart1 = uiPartIdxLTVirt1;
			iPartIdxAEnd1 = uiPartIdxRTVirt1;

			uiPartIdxAR = uiPartIdxRTVirt1;
			uiPartIdxBL = uiPartIdxLBVirt0;
			uiPartIdxAL = uiPartIdxLTVirt0;
		}
		else //if (eCUMode == SIZE_SHV_RB)
		{
			deriveLeftTopIdxVirt ( eCUMode, 0, uiPartIdxLTVirt0 );
			deriveLeftTopIdxVirt ( eCUMode, 1, uiPartIdxLTVirt1 );
			deriveLeftTopIdxVirt ( eCUMode, 2, uiPartIdxLTVirt2 );

			deriveRightTopIdxVirt ( eCUMode, 0, uiPartIdxRTVirt0 );
			deriveRightTopIdxVirt ( eCUMode, 1, uiPartIdxRTVirt1 );

			deriveLeftBottomIdxVirt( eCUMode, 0, uiPartIdxLBVirt0 );
			deriveLeftBottomIdxVirt( eCUMode, 2, uiPartIdxLBVirt2 );

			iPartIdxLStart0 = uiPartIdxLTVirt0;
			iPartIdxLEnd0 = uiPartIdxLBVirt0;
			iPartIdxLStart1 = uiPartIdxLTVirt2;
			iPartIdxLEnd1 = uiPartIdxLBVirt2;

			iPartIdxAStart0 = uiPartIdxLTVirt0;
			iPartIdxAEnd0 = uiPartIdxRTVirt0;
			iPartIdxAStart1 = uiPartIdxLTVirt1;
			iPartIdxAEnd1 = uiPartIdxRTVirt1;

			uiPartIdxAR = uiPartIdxRTVirt1;
			uiPartIdxBL = uiPartIdxLBVirt2;
			uiPartIdxAL = uiPartIdxLTVirt0;
		}

		//Left
		for ( uiIdx = g_auiConvertRelToAbsIdx[iPartIdxLStart0]; uiIdx <= g_auiConvertRelToAbsIdx[iPartIdxLEnd0]; uiIdx+= uiNumPartInCUWidth )
		{
			bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_LEFT );
			if (bAdded && iLeftMvIdx < 0)
			{
				iLeftMvIdx = pInfo->iN-1;
			}
    #if !AMVP_NEIGH_ALL
			if (bAdded) break;
		#else
			if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
    #endif
		}
  #if !AMVP_NEIGH_ALL
		if (!bAdded && iPartIdxLStart1>=0)
  #endif
		{
			for ( uiIdx = g_auiConvertRelToAbsIdx[iPartIdxLStart1]; uiIdx <= g_auiConvertRelToAbsIdx[iPartIdxLEnd1]; uiIdx+= uiNumPartInCUWidth )
			{
				bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_LEFT );
				if (bAdded && iLeftMvIdx < 0)
				{
					iLeftMvIdx = pInfo->iN-1;
				}
      #if !AMVP_NEIGH_ALL
				if (bAdded) break;
      #else
				if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
      #endif
			}
		}

		bAdded = false;
		//Above
		for ( uiIdx = g_auiConvertRelToAbsIdx[iPartIdxAStart0]; uiIdx <= g_auiConvertRelToAbsIdx[iPartIdxAEnd0]; uiIdx++ )
		{
			bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_ABOVE);
			if (bAdded && iAboveMvIdx < 0)
			{
				iAboveMvIdx = pInfo->iN-1;
			}
    #if !AMVP_NEIGH_ALL
			if (bAdded) break;
    #else
			if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
    #endif
		}
  #if !AMVP_NEIGH_ALL
		if (!bAdded&&iPartIdxAStart1>=0)
  #endif
		{
			for ( uiIdx = g_auiConvertRelToAbsIdx[iPartIdxAStart1]; uiIdx <= g_auiConvertRelToAbsIdx[iPartIdxAEnd1]; uiIdx++ )
			{
				bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_ABOVE);
				if (bAdded && iAboveMvIdx < 0)
				{
					iAboveMvIdx = pInfo->iN-1;
				}
      #if !AMVP_NEIGH_ALL
				if (bAdded) break;
      #else
				if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
      #endif
			}
		}

		bAdded = false;
		//Above Right
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxAR, MD_ABOVE_RIGHT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}

		//Below Left
  #if !AMVP_NEIGH_ALL
		if (!bAdded)
  #else
		if (getAMVPMode(uiPartAddr) != AM_NONE || !bAdded)
  #endif
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxBL, MD_BELOW_LEFT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}

		//Above Left
  #if !AMVP_NEIGH_ALL
		if (!bAdded)
  #else
    if (getAMVPMode(uiPartAddr) != AM_NONE || !bAdded)
  #endif
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxAL, MD_ABOVE_LEFT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}

  #if !AMVP_NEIGH_ALL
		if (!bAdded && eCUMode == SIZE_SHV_LT)
  #else
		if (eCUMode == SIZE_SHV_LT && (getAMVPMode(uiPartAddr) != AM_NONE || !bAdded))
  #endif
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLTVirt2, MD_ABOVE_LEFT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}
	}
	else
	{
		deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
		deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );

		//Left
		for ( uiIdx = g_auiConvertRelToAbsIdx[uiPartIdxLT]; uiIdx <= g_auiConvertRelToAbsIdx[uiPartIdxLB]; uiIdx+= uiNumPartInCUWidth )
		{
			bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_LEFT );
			if (bAdded && iLeftMvIdx < 0)
			{
				iLeftMvIdx = pInfo->iN-1;
			}
    #if !AMVP_NEIGH_ALL
			if (bAdded) break;
    #else
			if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
    #endif
		}

		bAdded = false;
		//Above
		for ( uiIdx = g_auiConvertRelToAbsIdx[uiPartIdxLT]; uiIdx <= g_auiConvertRelToAbsIdx[uiPartIdxRT]; uiIdx++ )
		{
			bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, g_auiConvertAbsToRelIdx[uiIdx], MD_ABOVE);
			if (bAdded && iAboveMvIdx < 0)
			{
				iAboveMvIdx = pInfo->iN-1;
			}
    #if !AMVP_NEIGH_ALL
			if (bAdded) break;
    #else
			if (getAMVPMode(uiPartAddr) == AM_NONE && bAdded) break;
    #endif
		}

		bAdded = false;
		//Above Right
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}

		//Below Left
  #if !AMVP_NEIGH_ALL
		if (!bAdded)
  #else
		if (getAMVPMode(uiPartAddr) != AM_NONE || !bAdded)
  #endif
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}

		//Above Left
  #if !AMVP_NEIGH_ALL
		if (!bAdded)
  #else
		if (getAMVPMode(uiPartAddr) != AM_NONE || !bAdded)
  #endif
		bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
		if (bAdded && iCornerMvIdx < 0)
		{
			iCornerMvIdx = pInfo->iN-1;
		}
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

	if ( m_pcSlice->getUseMVAC() )
		restrictMvpAccuracy(pInfo->m_acMvCand[0]);

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
	UInt uiAbsPartAddr = m_uiAbsZorderIdx + uiPartAddr;

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

		if ( m_pcSlice->getUseMVAC() )
		{
			restrictMvpAccuracy(cMv);
		}

		pInfo->m_acMvCand[pInfo->iN++] = cMv ;
	}
#endif
	// Check No MV Candidate
	xUniqueMVPCand( pInfo );
	return ;
}


Bool TComDataCU::clearMVPCand(TComMv cMvd, AMVPInfo* pInfo)
{
	if (!m_pcSlice->getSPS()->getUseAMVP() || !m_pcSlice->getSPS()->getUseIMR())
  {
    return false;
  }

  if (pInfo->iN <= 1)
  {
    return false;
  }

  if (cMvd.getHor() == 0 && cMvd.getVer() == 0)
  {
    return false;
  }

	TComMv	acMv[ AMVP_MAX_NUM_CANDS ];
	Int aiValid[ AMVP_MAX_NUM_CANDS ];

	Int	iNTmp, i, j;

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
  Int iHorMax = (m_pcSlice->getSPS()->getWidth() - m_uiCUPelX - 1 )<<2;
  Int iHorMin = (      -(Int)g_uiMaxCUWidth - (Int)m_uiCUPelX + 1 )<<2;

  Int iVerMax = (m_pcSlice->getSPS()->getHeight() - m_uiCUPelY - 1 )<<2;
  Int iVerMin = (      -(Int)g_uiMaxCUHeight - (Int)m_uiCUPelY + 1 )<<2;

  rcMv.setHor( Min (iHorMax, Max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( Min (iVerMax, Max (iVerMin, rcMv.getVer())) );
}

Void TComDataCU::getMvPredSkip( Bool bBi )
{
  Int     iRefIdxL0 = 0;
  Int     iRefIdxL1 = NOT_VALID;
  TComMv  cZeroMv, cMvPred;

  getMvPred( SIZE_2Nx2N, 0, REF_PIC_LIST_0, 0, cMvPred );

  if ( ( m_pcCULeft == NULL || m_pcCUAbove == NULL )  ||
       ( m_cMvFieldA.getHor() == 0 && m_cMvFieldA.getVer() == 0 && m_cMvFieldA.getRefIdx() == 0 ) ||
       ( m_cMvFieldB.getHor() == 0 && m_cMvFieldB.getVer() == 0 && m_cMvFieldB.getRefIdx() == 0 ) )
  {
    cMvPred.setZero();
  }

  getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvPred, iRefIdxL0, SIZE_2Nx2N, 0, 0, 0 );
  getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv,            SIZE_2Nx2N, 0, 0, 0 );

  if ( bBi )
  {
    iRefIdxL1 = 0;
    getMvPred( SIZE_2Nx2N, 0, REF_PIC_LIST_1, 0, cMvPred );

    if ( ( m_pcCULeft == NULL || m_pcCUAbove == NULL )  ||
      ( m_cMvFieldA.getHor() == 0 && m_cMvFieldA.getVer() == 0 && m_cMvFieldA.getRefIdx() == 0 ) ||
      ( m_cMvFieldB.getHor() == 0 && m_cMvFieldB.getVer() == 0 && m_cMvFieldB.getRefIdx() == 0 ) )
    {
      cMvPred.setZero();
    }
    getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvPred, iRefIdxL1, SIZE_2Nx2N, 0, 0, 0 );
    getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv,            SIZE_2Nx2N, 0, 0, 0 );
  }
  else
  {
    getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cZeroMv, iRefIdxL1, SIZE_2Nx2N, 0, 0, 0 );
    getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv,            SIZE_2Nx2N, 0, 0, 0 );
  }
}

Void TComDataCU::convertTransIdx( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode )
{
  if( getPredictionMode( uiAbsPartIdx ) == MODE_INTRA )
  {
    ruiLumaTrMode   = uiTrIdx;
    ruiChromaTrMode = 0;

    if( !getSlice()->getSPS()->getUseLOT() && ((m_puhWidth[uiAbsPartIdx]>>1) > getSlice()->getSPS()->getMaxTrSize()) )
    {
      while( (m_puhWidth[uiAbsPartIdx]>>1) > (getSlice()->getSPS()->getMaxTrSize()<<ruiChromaTrMode) ) ruiChromaTrMode++;
    }
    return;
  }

  UInt uiSizeBit        = g_aucConvertToBit[ Min( m_puhWidth [ uiAbsPartIdx ], m_puhHeight[ uiAbsPartIdx ] ) ] + 2;
  UInt uiMinCUSizeBit   = g_aucConvertToBit[ Min( m_pcPic->getMinCUWidth(),    m_pcPic->getMinCUHeight()   ) ] + 2;
  UInt uiLowerBnd       = uiMinCUSizeBit;//Max( uiMinCUSizeBit, 2 );

  ruiLumaTrMode   = uiTrIdx;
  ruiChromaTrMode = uiTrIdx;

  if ( (Int)uiSizeBit - (Int)uiTrIdx <= (Int)uiLowerBnd  )
  {
    ruiLumaTrMode   = uiSizeBit - uiLowerBnd;
    ruiChromaTrMode = ruiLumaTrMode;
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
	return true;
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

    if ( m_pcSlice->getUseMVAC() )
      restrictMvpAccuracy(cMvPred);

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

	TComMv	acMv[ AMVP_MAX_NUM_CANDS ];
	Int	iNTmp, i, j;

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
	UInt uiTemp 	= ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);

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
		Int iTDB			= Clip3( -128, 127, iDiffPocB );
		Int iTDD			= Clip3( -128, 127, iDiffPocD );
		Int iX				= (0x4000 + abs(iTDD/2)) / iTDD;
		Int iScale		= Clip3( -1024, 1023, (iTDB * iX + 32) >> 6 );
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
