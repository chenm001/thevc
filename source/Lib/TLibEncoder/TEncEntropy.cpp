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

/** \file			TEncEntropy.cpp
    \brief		entropy encoder class
*/

#include "TEncEntropy.h"

Void TEncEntropy::setEntropyCoder ( TEncEntropyIf* e, TComSlice* pcSlice )
{
  m_pcEntropyCoderIf = e;
  m_pcEntropyCoderIf->setSlice ( pcSlice );
}

Void TEncEntropy::encodeSliceHeader ( TComSlice* pcSlice )
{
  m_pcEntropyCoderIf->codeSliceHeader( pcSlice );
  return;
}

Void TEncEntropy::encodeTerminatingBit      ( UInt uiIsLast )
{
  m_pcEntropyCoderIf->codeTerminatingBit( uiIsLast );

  return;
}

Void TEncEntropy::encodeSliceFinish()
{
  m_pcEntropyCoderIf->codeSliceFinish();
}

Void TEncEntropy::encodeSPS( TComSPS* pcSPS )
{
  m_pcEntropyCoderIf->codeSPS( pcSPS );
  return;
}

Void TEncEntropy::encodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  m_pcEntropyCoderIf->codeSkipFlag( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodeAlfParam(ALFParam* pAlfParam)
{
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->alf_flag);
  if (!pAlfParam->alf_flag)
    return;
  Int pos;
  // filter parameters for luma
  m_pcEntropyCoderIf->codeAlfUvlc((pAlfParam->tap-5)/2);
  for(pos=0; pos<pAlfParam->num_coeff; pos++)
  {
    m_pcEntropyCoderIf->codeAlfSvlc(pAlfParam->coeff[pos]);
  }

  // filter parameters for chroma
  m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->chroma_idc);
  if(pAlfParam->chroma_idc)
  {
    m_pcEntropyCoderIf->codeAlfUvlc((pAlfParam->tap_chroma-5)/2);

    // filter coefficients for chroma
    for(pos=0; pos<pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyCoderIf->codeAlfSvlc(pAlfParam->coeff_chroma[pos]);
    }
  }

  // region control parameters for luma
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->cu_control_flag);
  if (pAlfParam->cu_control_flag)
  {
    assert( (pAlfParam->cu_control_flag && m_pcEntropyCoderIf->getAlfCtrl()) || (!pAlfParam->cu_control_flag && !m_pcEntropyCoderIf->getAlfCtrl()));
    m_pcEntropyCoderIf->codeAlfCtrlDepth();
  }
}

Void TEncEntropy::encodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeAlfCtrlFlag( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  if (pcCU->isSkipped( uiAbsPartIdx ))
  	return;

  m_pcEntropyCoderIf->codePredMode( pcCU, uiAbsPartIdx );
}

// Split mode
Void TEncEntropy::encodeSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TEncEntropy::encodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->isSkip( uiAbsPartIdx ) )
  	return;

  m_pcEntropyCoderIf->codePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

// transform index
Void TEncEntropy::encodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeTransformIdx( pcCU, uiAbsPartIdx, uiDepth );
}

// ROT index
Void TEncEntropy::encodeROTindex  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
	if( bRD )
		uiAbsPartIdx = 0;

	if (pcCU->getPredictionMode( uiAbsPartIdx )==MODE_INTRA)
	{
		if( ( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U) + pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V) ) == 0 )
		{
			return;
		}
		m_pcEntropyCoderIf->codeROTindex( pcCU, uiAbsPartIdx, bRD );
	}
}

// CIP index
Void TEncEntropy::encodeCIPflag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  uiAbsPartIdx = 0;

	if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcEntropyCoderIf->codeCIPflag( pcCU, uiAbsPartIdx, bRD );
  }
}

// Intra direction for Luma
Void TEncEntropy::encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  m_pcEntropyCoderIf->codeIntraDirLumaAdi( pcCU, uiAbsPartIdx );
}

// Intra direction for Chroma
Void TEncEntropy::encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeIntraDirChroma( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodePredInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  if (pcCU->isSkip( uiAbsPartIdx ))
  {
    if (pcCU->getSlice()->isInterB())
    {
    	encodeInterDir(pcCU, uiAbsPartIdx, bRD);
    }
		if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 ) //if ( ref. frame list0 has at least 1 entry )
		{
			encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_0);
		}
		if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
		{
			encodeMVPIdx( pcCU, uiAbsPartIdx, REF_PIC_LIST_1);
		}
  	return;
  }

  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );

  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    if( eSize == SIZE_NxN )                                         // if it is NxN size, encode 4 intra directions.
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      // if it is NxN size, this size might be the smallest partition size.
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx                  );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset   );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*2 );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*3 );
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
    else                                                              // if it is not NxN size, encode 1 intra directions
    {
      encodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx );
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    encodeInterDir( pcCU, uiAbsPartIdx, bRD );

    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )				// if ( ref. frame list0 has at least 1 entry )
    {
			encodeRefFrmIdx ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0, bRD );
			encodeMvd       ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0, bRD );
			encodeMVPIdx    ( pcCU, uiAbsPartIdx, REF_PIC_LIST_0		  );
    }

		if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )				// if ( ref. frame list1 has at least 1 entry )
		{
			encodeRefFrmIdx ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1, bRD );
			encodeMvd       ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1, bRD );
			encodeMVPIdx    ( pcCU, uiAbsPartIdx, REF_PIC_LIST_1			);
		}
  }
}

Void TEncEntropy::encodeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );
  assert( !pcCU->isSkip( uiAbsPartIdx ) || pcCU->getSlice()->isInterB());

  if( bRD )
    uiAbsPartIdx = 0;

  if ( !pcCU->getSlice()->isInterB() )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_2NxN:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset << 1;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_Nx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      uiAbsPartIdx += uiPartOffset;
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>1) );
      break;
    }
  case SIZE_2NxnD:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset<<1) + (uiPartOffset>>1) );
      break;
    }
  case SIZE_nLx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + (uiPartOffset>>2) );
      break;
    }
  case SIZE_nRx2N:
    {
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
      m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx + uiPartOffset + (uiPartOffset>>2) );
      break;
    }
  default:
    break;
  }

  return;
}

Void TEncEntropy::encodeMVPIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      uiAbsPartIdx += uiPartOffset << 1;
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
      {
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        {
          m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);
      if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getMVPNum(eRefList, uiAbsPartIdx)> 1) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
        m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;

}

Void TEncEntropy::encodeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if( bRD )
    uiAbsPartIdx = 0;

  if ( ( pcCU->getSlice()->getNumRefIdx( eRefList ) == 1 ) || pcCU->isSkip( uiAbsPartIdx ) )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset << 1;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        {
          m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;
}

Void TEncEntropy::encodeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList, Bool bRD )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if( bRD )
    uiAbsPartIdx = 0;

  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    return;
  }

  UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;

  switch ( pcCU->getPartitionSize( uiAbsPartIdx ) )
  {
  case SIZE_2Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_2NxN:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset << 1;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_Nx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }

      uiAbsPartIdx += uiPartOffset;
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
      {
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
      }
      break;
    }

  case SIZE_NxN:
    {
      for ( Int iPartIdx = 0; iPartIdx < 4; iPartIdx++ )
      {
        if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        {
          m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
        }
        uiAbsPartIdx += uiPartOffset;
      }
      break;
    }
  case SIZE_2NxnU:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_2NxnD:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset<<1) + (uiPartOffset>>1);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nLx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  case SIZE_nRx2N:
    {
      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      uiAbsPartIdx += uiPartOffset + (uiPartOffset>>2);

      if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
        m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );

      break;
    }
  default:
    break;
  }

  return;
}

// Coded block flag
Void TEncEntropy::encodeCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}

// dQP
Void TEncEntropy::encodeQP( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
    uiAbsPartIdx = 0;

	if ( pcCU->getSlice()->getSPS()->getUseDQP() )
	{
		m_pcEntropyCoderIf->codeDeltaQP( pcCU, uiAbsPartIdx );
	}
}


// texture
Void TEncEntropy::xEncodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType, Bool bRD )
{
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
  {
    if( uiCurrTrIdx == uiTrIdx )
    {
      m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType, bRD );
    }
    else
    {
      if( uiCurrTrIdx <= uiTrIdx )
        assert(0);

      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;

      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD ); pcCoeff += uiSize; uiIdx += uiQPartNum;

      m_pcEntropyCoderIf->codeCbf( pcCU, uiIdx, eType, uiTrIdx );
      xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bRD );
    }
  }
}

Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TextType eType, Bool bRD )
{
  xEncodeCoeff( pcCU, pCoeff, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, uiTrMode, uiMaxTrMode, eType, bRD );
}

Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;

  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );

  if( pcCU->isIntra(uiAbsPartIdx) )
  {
    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );

    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );

    m_pcEntropyCoderIf->codeCbf(pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0);
    xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
  else
  {
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, 0 );
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, 0 );
    m_pcEntropyCoderIf->codeCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, 0 );

    if( pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0) || pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0) )
      encodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );

    xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
    xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
    xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
  }
}

Void TEncEntropy::encodeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType, Bool bRD )
{ // This is for Transform unit processing. This may be used at mode selection stage for Inter.
  m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiTrWidth, uiTrHeight, uiDepth, eType, bRD );
}


Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiWidth, TextType eTType)
{
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

  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA;

  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, uiCTXIdx, eTType );
}
