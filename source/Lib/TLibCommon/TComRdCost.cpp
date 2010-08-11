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

/** \file     TComRdCost.cpp
    \brief    RD cost computation class
*/

#include <math.h>
#include <assert.h>
#include "TComRdCost.h"

TComRdCost::TComRdCost()
{
  init();
}

TComRdCost::~TComRdCost()
{
  xUninit();
}

#ifdef ROUNDING_CONTROL
  __inline Pel  xClip  (Pel x )      { return ( (x < 0) ? 0 : (x > (Pel)g_uiIBDI_MAX) ? (Pel)g_uiIBDI_MAX : x ); }
#endif

// Calculate RD functions
Double TComRdCost::calcRdCost( UInt uiBits, UInt uiDistortion, Bool bFlag, DFunc eDFunc )
{
  Double dRdCost = 0.0;
  Double dLambda = 0.0;

  switch ( eDFunc )
  {
  case DF_SSE:
    assert(0);  break;
  case DF_SAD:
    dLambda = (Double)m_uiLambdaMotionSAD;  break;
  case DF_DEFAULT:
    dLambda =         m_dLambda;            break;
  case DF_SSE_FRAME:
    dLambda =         m_dFrameLambda;       break;
  default:
    assert (0);
  }

  if (bFlag)
  {
    // Intra8x8, Intra4x4 Block only...
    dRdCost = (((Double)uiDistortion) + ((Double)uiBits * dLambda));
  }
  else
  {
    if (eDFunc == DF_SAD)
    {
      dRdCost = ((Double)uiDistortion + (Double)((Int)(uiBits * dLambda+.5)>>16));
      dRdCost = (Double)(UInt)floor(dRdCost);
    }
    else
    {
      dRdCost = ((Double)uiDistortion + (Double)((Int)(uiBits * dLambda+.5)));
      dRdCost = (Double)(UInt)floor(dRdCost);
    }
  }

  return dRdCost;
}

Double TComRdCost::calcRdCost64( UInt64 uiBits, UInt64 uiDistortion, Bool bFlag, DFunc eDFunc )
{
  Double dRdCost = 0.0;
  Double dLambda = 0.0;

  switch ( eDFunc )
  {
  case DF_SSE:
    assert(0);  break;
  case DF_SAD:
    dLambda = (Double)m_uiLambdaMotionSAD;  break;
  case DF_DEFAULT:
    dLambda =         m_dLambda;            break;
  case DF_SSE_FRAME:
    dLambda =         m_dFrameLambda;       break;
  default:
    assert (0);
  }

  if (bFlag)
  {
    // Intra8x8, Intra4x4 Block only...
    dRdCost = (((Double)(Int64)uiDistortion) + ((Double)(Int64)uiBits * dLambda));
  }
  else
  {
    if (eDFunc == DF_SAD)
    {
      dRdCost = ((Double)(Int64)uiDistortion + (Double)((Int)((Int64)uiBits * dLambda+.5)>>16));
      dRdCost = (Double)(UInt)floor(dRdCost);
    }
    else
    {
      dRdCost = ((Double)(Int64)uiDistortion + (Double)((Int)((Int64)uiBits * dLambda+.5)));
      dRdCost = (Double)(UInt)floor(dRdCost);
    }
  }

  return dRdCost;
}

Void TComRdCost::setLambda( Double dLambda )
{
  m_dLambda           = dLambda;
  m_uiLambdaMotionSAD = (UInt)floor(65536.0 * sqrt( m_dLambda ));
  m_uiLambdaMotionSSE = (UInt)floor(65536.0 *       m_dLambda  );
}


// Initalize Function Pointer by [eDFunc]
Void TComRdCost::init()
{
  m_afpDistortFunc[0]  = NULL;                  // for DF_DEFAULT

  m_afpDistortFunc[1]  = TComRdCost::xGetSSE;
  m_afpDistortFunc[2]  = TComRdCost::xGetSSE4;
  m_afpDistortFunc[3]  = TComRdCost::xGetSSE8;
  m_afpDistortFunc[4]  = TComRdCost::xGetSSE16;
  m_afpDistortFunc[5]  = TComRdCost::xGetSSE32;
  m_afpDistortFunc[6]  = TComRdCost::xGetSSE64;
  m_afpDistortFunc[7]  = TComRdCost::xGetSSE16N;

  m_afpDistortFunc[8]  = TComRdCost::xGetSAD;
  m_afpDistortFunc[9]  = TComRdCost::xGetSAD4;
  m_afpDistortFunc[10] = TComRdCost::xGetSAD8;
  m_afpDistortFunc[11] = TComRdCost::xGetSAD16;
  m_afpDistortFunc[12] = TComRdCost::xGetSAD32;
  m_afpDistortFunc[13] = TComRdCost::xGetSAD64;
  m_afpDistortFunc[14] = TComRdCost::xGetSAD16N;

  m_afpDistortFunc[15] = TComRdCost::xGetSADs;
  m_afpDistortFunc[16] = TComRdCost::xGetSADs4;
  m_afpDistortFunc[17] = TComRdCost::xGetSADs8;
  m_afpDistortFunc[18] = TComRdCost::xGetSADs16;
  m_afpDistortFunc[19] = TComRdCost::xGetSADs32;
  m_afpDistortFunc[20] = TComRdCost::xGetSADs64;
  m_afpDistortFunc[21] = TComRdCost::xGetSADs16N;

  m_afpDistortFunc[22] = TComRdCost::xGetHADs;
  m_afpDistortFunc[23] = TComRdCost::xGetHADs4;
  m_afpDistortFunc[24] = TComRdCost::xGetHADs8;
  m_afpDistortFunc[25] = TComRdCost::xGetHADs;
  m_afpDistortFunc[26] = TComRdCost::xGetHADs;
  m_afpDistortFunc[27] = TComRdCost::xGetHADs;
  m_afpDistortFunc[28] = TComRdCost::xGetHADs;

#ifdef ROUNDING_CONTROL
  m_afpDistortFuncRnd[0]  =	NULL;
  m_afpDistortFuncRnd[1]  = TComRdCost::xGetSSE;
  m_afpDistortFuncRnd[2]  = TComRdCost::xGetSSE4;
  m_afpDistortFuncRnd[3]  = TComRdCost::xGetSSE8;
  m_afpDistortFuncRnd[4]  = TComRdCost::xGetSSE16;
  m_afpDistortFuncRnd[5]  = TComRdCost::xGetSSE32;
  m_afpDistortFuncRnd[6]  = TComRdCost::xGetSSE64;
  m_afpDistortFuncRnd[7]  = TComRdCost::xGetSSE16N;

  m_afpDistortFuncRnd[8]  = TComRdCost::xGetSAD;
  m_afpDistortFuncRnd[9]  = TComRdCost::xGetSAD4;
  m_afpDistortFuncRnd[10] = TComRdCost::xGetSAD8;
  m_afpDistortFuncRnd[11] = TComRdCost::xGetSAD16;
  m_afpDistortFuncRnd[12] = TComRdCost::xGetSAD32;
  m_afpDistortFuncRnd[13] = TComRdCost::xGetSAD64;
  m_afpDistortFuncRnd[14] = TComRdCost::xGetSAD16N;

  m_afpDistortFuncRnd[15] = TComRdCost::xGetSADs;
  m_afpDistortFuncRnd[16] = TComRdCost::xGetSADs4;
  m_afpDistortFuncRnd[17] = TComRdCost::xGetSADs8;
  m_afpDistortFuncRnd[18] = TComRdCost::xGetSADs16;
  m_afpDistortFuncRnd[19] = TComRdCost::xGetSADs32;
  m_afpDistortFuncRnd[20] = TComRdCost::xGetSADs64;
  m_afpDistortFuncRnd[21] = TComRdCost::xGetSADs16N;

  m_afpDistortFuncRnd[22] = TComRdCost::xGetHADs;
  m_afpDistortFuncRnd[23] = TComRdCost::xGetHADs4;
  m_afpDistortFuncRnd[24] = TComRdCost::xGetHADs8;
  m_afpDistortFuncRnd[25] = TComRdCost::xGetHADs;
  m_afpDistortFuncRnd[26] = TComRdCost::xGetHADs;
  m_afpDistortFuncRnd[27] = TComRdCost::xGetHADs;
  m_afpDistortFuncRnd[28] = TComRdCost::xGetHADs;
#endif

  m_puiComponentCostOriginP = NULL;
  m_puiComponentCost        = NULL;
  m_puiVerCost              = NULL;
  m_puiHorCost              = NULL;
  m_uiCost                  = 0;
  m_iCostScale              = 0;
  m_iSearchLimit            = 0xdeaddead;
}

Void TComRdCost::initRateDistortionModel( Int iSubPelSearchLimit )
{
  // make it larger
  iSubPelSearchLimit += 4;
  iSubPelSearchLimit *= 8;

  if( m_iSearchLimit != iSubPelSearchLimit )
  {
    xUninit();

    m_iSearchLimit = iSubPelSearchLimit;

    m_puiComponentCostOriginP = new UInt[ 4 * iSubPelSearchLimit ];
    iSubPelSearchLimit *= 2;

    m_puiComponentCost = m_puiComponentCostOriginP + iSubPelSearchLimit;

    for( Int n = -iSubPelSearchLimit; n < iSubPelSearchLimit; n++)
    {
      m_puiComponentCost[n] = xGetComponentBits( n );
    }
  }
}

Void TComRdCost::xUninit()
{
  if( NULL != m_puiComponentCostOriginP )
  {
    delete [] m_puiComponentCostOriginP;
    m_puiComponentCostOriginP = NULL;
  }
}

UInt TComRdCost::xGetComponentBits( Int iVal )
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

#ifdef ROUNDING_CONTROL
// Setting the Distortion Parameter for Inter (ME)
Void TComRdCost::setDistParam_Bi( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, DistParam& rcDistParam )
{
  // set Original & Curr Pointer / Stride
  rcDistParam.pOrg = pcPatternKey->getROIY();
  rcDistParam.pCur = piRefY;

  rcDistParam.iStrideOrg = pcPatternKey->getPatternLStride();
  rcDistParam.iStrideCur = iRefStride;

  // set Block Width / Height
  rcDistParam.iCols    = pcPatternKey->getROIYWidth();
  rcDistParam.iRows    = pcPatternKey->getROIYHeight();
  rcDistParam.DistFuncRnd = m_afpDistortFuncRnd[DF_SAD + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];

  // initialize
  rcDistParam.iSubShift  = 0;
}

// Setting the Distortion Parameter for Inter (subpel ME with step)
Void TComRdCost::setDistParam_Bi( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iStep, DistParam& rcDistParam, Bool bHADME )
{
  // set Original & Curr Pointer / Stride
  rcDistParam.pOrg = pcPatternKey->getROIY();
  rcDistParam.pCur = piRefY;

  rcDistParam.iStrideOrg = pcPatternKey->getPatternLStride();
  rcDistParam.iStrideCur = iRefStride * iStep;

  // set Step for interpolated buffer
  rcDistParam.iStep = iStep;

  // set Block Width / Height
  rcDistParam.iCols    = pcPatternKey->getROIYWidth();
  rcDistParam.iRows    = pcPatternKey->getROIYHeight();

  // set distortion function
  if ( !bHADME )
  {
    rcDistParam.DistFuncRnd = m_afpDistortFuncRnd[DF_SADS + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];
  }
  else
  {
    rcDistParam.DistFuncRnd = m_afpDistortFuncRnd[DF_HADS + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];
  }

  // initialize
  rcDistParam.iSubShift  = 0;
}
#endif
// Setting the Distortion Parameter for getDistLumBlk / getDistCbBlk / getDistCrBlk
Void TComRdCost::setDistParam( UInt uiBlkWidth, UInt uiBlkHeight, DFunc eDFunc, DistParam& rcDistParam )
{
  // set Block Width / Height
  rcDistParam.iCols    = uiBlkWidth;
  rcDistParam.iRows    = uiBlkHeight;
  rcDistParam.DistFunc = m_afpDistortFunc[eDFunc + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];

  // initialize
  rcDistParam.iSubShift  = 0;
}

// Setting the Distortion Parameter for Inter (ME)
Void TComRdCost::setDistParam( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, DistParam& rcDistParam )
{
  // set Original & Curr Pointer / Stride
  rcDistParam.pOrg = pcPatternKey->getROIY();
  rcDistParam.pCur = piRefY;

  rcDistParam.iStrideOrg = pcPatternKey->getPatternLStride();
  rcDistParam.iStrideCur = iRefStride;

  // set Block Width / Height
  rcDistParam.iCols    = pcPatternKey->getROIYWidth();
  rcDistParam.iRows    = pcPatternKey->getROIYHeight();
  rcDistParam.DistFunc = m_afpDistortFunc[DF_SAD + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];

  // initialize
  rcDistParam.iSubShift  = 0;
}

// Setting the Distortion Parameter for Inter (subpel ME with step)
Void TComRdCost::setDistParam( TComPattern* pcPatternKey, Pel* piRefY, Int iRefStride, Int iStep, DistParam& rcDistParam, Bool bHADME )
{
  // set Original & Curr Pointer / Stride
  rcDistParam.pOrg = pcPatternKey->getROIY();
  rcDistParam.pCur = piRefY;

  rcDistParam.iStrideOrg = pcPatternKey->getPatternLStride();
  rcDistParam.iStrideCur = iRefStride * iStep;

  // set Step for interpolated buffer
  rcDistParam.iStep = iStep;

  // set Block Width / Height
  rcDistParam.iCols    = pcPatternKey->getROIYWidth();
  rcDistParam.iRows    = pcPatternKey->getROIYHeight();

  // set distortion function
  if ( !bHADME )
  {
    rcDistParam.DistFunc = m_afpDistortFunc[DF_SADS + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];
  }
  else
  {
    rcDistParam.DistFunc = m_afpDistortFunc[DF_HADS + g_aucConvertToBit[ rcDistParam.iCols ] + 1 ];
  }

  // initialize
  rcDistParam.iSubShift  = 0;
}

UInt TComRdCost::calcHAD( Pel* pi0, Int iStride0, Pel* pi1, Int iStride1, Int iWidth, Int iHeight )
{
  UInt uiSum = 0;
  Int x, y;

  if ( ( (iWidth % 8) == 0 ) && ( (iHeight % 8) == 0 ) )
  {
    for ( y=0; y<iHeight; y+= 8 )
    {
      for ( x=0; x<iWidth; x+= 8 )
      {
        uiSum += xCalcHADs8x8( &pi0[x], &pi1[x], iStride0, iStride1, 1 );
      }
      pi0 += iStride0*8;
      pi1 += iStride1*8;
    }
  }
  else if ( ( (iWidth % 4) == 0 ) && ( (iHeight % 4) == 0 ) )
  {
    for ( y=0; y<iHeight; y+= 4 )
    {
      for ( x=0; x<iWidth; x+= 4 )
      {
        uiSum += xCalcHADs4x4( &pi0[x], &pi1[x], iStride0, iStride1, 1 );
      }
      pi0 += iStride0*4;
      pi1 += iStride1*4;
    }
  }
  else
  {
    for ( y=0; y<iHeight; y+= 2 )
    {
      for ( x=0; x<iWidth; x+= 2 )
      {
        uiSum += xCalcHADs8x8( &pi0[x], &pi1[x], iStride0, iStride1, 1 );
      }
      pi0 += iStride0*2;
      pi1 += iStride1*2;
    }
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::getDistPart( Pel* piCur, Int iCurStride,  Pel* piOrg, Int iOrgStride, UInt uiBlkWidth, UInt uiBlkHeight, DFunc eDFunc )
{
  DistParam cDtParam;
  setDistParam( uiBlkWidth, uiBlkHeight, eDFunc, cDtParam );
  cDtParam.pOrg       = piOrg;
  cDtParam.pCur       = piCur;
  cDtParam.iStrideOrg = iOrgStride;
  cDtParam.iStrideCur = iCurStride;
  return cDtParam.DistFunc( &cDtParam );
}

// ====================================================================================================================
// Distortion functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// SAD
// --------------------------------------------------------------------------------------------------------------------

#ifdef ROUNDING_CONTROL

UInt TComRdCost::xGetSAD( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Pel  pred;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
	  pred = xClip( (piCur[n] + piRef[n] + bRound) >> 1 );
	  uiSum += abs( piOrg[n] - pred );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD4( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
	  for(i = 0; i < 4; i++)
	  {
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD8( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg      = pcDtParam->pOrg;
  Pel* piCur      = pcDtParam->pCur;
  Pel* piRef	  = pRefY;
  Int  iRows      = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
	  for( i = 0; i < 8; i++)
	  {
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
	  piCur += iStrideCur;
	  piRef += pcDtParam->iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD16( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
	  for( i = 0; i < 16; i++)
	  {
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD16N( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {
      for(i = 0; i < 16; i++)
	  {
		  pred = xClip( (piCur[n+ i] + piRef[n+ i] + bRound) >> 1 );
		  uiSum += abs( piOrg[n+ i] - pred );
	  }
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD32( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
	  for(i = 0; i < 32; i++)
	  {
		  pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		  uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD64( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
	  for(i = 0; i < 64; i++)
	  {
		  pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		  uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols*iSubStep;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

#endif

UInt TComRdCost::xGetSAD( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
      uiSum += abs( piOrg[n] - piCur[n] );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD4( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    uiSum += abs( piOrg[0] - piCur[0] );
    uiSum += abs( piOrg[1] - piCur[1] );
    uiSum += abs( piOrg[2] - piCur[2] );
    uiSum += abs( piOrg[3] - piCur[3] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD8( DistParam* pcDtParam )
{
  Pel* piOrg      = pcDtParam->pOrg;
  Pel* piCur      = pcDtParam->pCur;
  Int  iRows      = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    uiSum += abs( piOrg[0] - piCur[0] );
    uiSum += abs( piOrg[1] - piCur[1] );
    uiSum += abs( piOrg[2] - piCur[2] );
    uiSum += abs( piOrg[3] - piCur[3] );
    uiSum += abs( piOrg[4] - piCur[4] );
    uiSum += abs( piOrg[5] - piCur[5] );
    uiSum += abs( piOrg[6] - piCur[6] );
    uiSum += abs( piOrg[7] - piCur[7] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD16( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    uiSum += abs( piOrg[0] - piCur[0] );
    uiSum += abs( piOrg[1] - piCur[1] );
    uiSum += abs( piOrg[2] - piCur[2] );
    uiSum += abs( piOrg[3] - piCur[3] );
    uiSum += abs( piOrg[4] - piCur[4] );
    uiSum += abs( piOrg[5] - piCur[5] );
    uiSum += abs( piOrg[6] - piCur[6] );
    uiSum += abs( piOrg[7] - piCur[7] );
    uiSum += abs( piOrg[8] - piCur[8] );
    uiSum += abs( piOrg[9] - piCur[9] );
    uiSum += abs( piOrg[10] - piCur[10] );
    uiSum += abs( piOrg[11] - piCur[11] );
    uiSum += abs( piOrg[12] - piCur[12] );
    uiSum += abs( piOrg[13] - piCur[13] );
    uiSum += abs( piOrg[14] - piCur[14] );
    uiSum += abs( piOrg[15] - piCur[15] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD16N( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {
      uiSum += abs( piOrg[n+ 0] - piCur[n+ 0] );
      uiSum += abs( piOrg[n+ 1] - piCur[n+ 1] );
      uiSum += abs( piOrg[n+ 2] - piCur[n+ 2] );
      uiSum += abs( piOrg[n+ 3] - piCur[n+ 3] );
      uiSum += abs( piOrg[n+ 4] - piCur[n+ 4] );
      uiSum += abs( piOrg[n+ 5] - piCur[n+ 5] );
      uiSum += abs( piOrg[n+ 6] - piCur[n+ 6] );
      uiSum += abs( piOrg[n+ 7] - piCur[n+ 7] );
      uiSum += abs( piOrg[n+ 8] - piCur[n+ 8] );
      uiSum += abs( piOrg[n+ 9] - piCur[n+ 9] );
      uiSum += abs( piOrg[n+10] - piCur[n+10] );
      uiSum += abs( piOrg[n+11] - piCur[n+11] );
      uiSum += abs( piOrg[n+12] - piCur[n+12] );
      uiSum += abs( piOrg[n+13] - piCur[n+13] );
      uiSum += abs( piOrg[n+14] - piCur[n+14] );
      uiSum += abs( piOrg[n+15] - piCur[n+15] );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD32( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    uiSum += abs( piOrg[0] - piCur[0] );
    uiSum += abs( piOrg[1] - piCur[1] );
    uiSum += abs( piOrg[2] - piCur[2] );
    uiSum += abs( piOrg[3] - piCur[3] );
    uiSum += abs( piOrg[4] - piCur[4] );
    uiSum += abs( piOrg[5] - piCur[5] );
    uiSum += abs( piOrg[6] - piCur[6] );
    uiSum += abs( piOrg[7] - piCur[7] );
    uiSum += abs( piOrg[8] - piCur[8] );
    uiSum += abs( piOrg[9] - piCur[9] );
    uiSum += abs( piOrg[10] - piCur[10] );
    uiSum += abs( piOrg[11] - piCur[11] );
    uiSum += abs( piOrg[12] - piCur[12] );
    uiSum += abs( piOrg[13] - piCur[13] );
    uiSum += abs( piOrg[14] - piCur[14] );
    uiSum += abs( piOrg[15] - piCur[15] );
    uiSum += abs( piOrg[16] - piCur[16] );
    uiSum += abs( piOrg[17] - piCur[17] );
    uiSum += abs( piOrg[18] - piCur[18] );
    uiSum += abs( piOrg[19] - piCur[19] );
    uiSum += abs( piOrg[20] - piCur[20] );
    uiSum += abs( piOrg[21] - piCur[21] );
    uiSum += abs( piOrg[22] - piCur[22] );
    uiSum += abs( piOrg[23] - piCur[23] );
    uiSum += abs( piOrg[24] - piCur[24] );
    uiSum += abs( piOrg[25] - piCur[25] );
    uiSum += abs( piOrg[26] - piCur[26] );
    uiSum += abs( piOrg[27] - piCur[27] );
    uiSum += abs( piOrg[28] - piCur[28] );
    uiSum += abs( piOrg[29] - piCur[29] );
    uiSum += abs( piOrg[30] - piCur[30] );
    uiSum += abs( piOrg[31] - piCur[31] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSAD64( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iSubShift  = pcDtParam->iSubShift;
  Int  iSubStep   = ( 1 << iSubShift );
  Int  iStrideCur = pcDtParam->iStrideCur*iSubStep;
  Int  iStrideOrg = pcDtParam->iStrideOrg*iSubStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-=iSubStep )
  {
    uiSum += abs( piOrg[0] - piCur[0] );
    uiSum += abs( piOrg[1] - piCur[1] );
    uiSum += abs( piOrg[2] - piCur[2] );
    uiSum += abs( piOrg[3] - piCur[3] );
    uiSum += abs( piOrg[4] - piCur[4] );
    uiSum += abs( piOrg[5] - piCur[5] );
    uiSum += abs( piOrg[6] - piCur[6] );
    uiSum += abs( piOrg[7] - piCur[7] );
    uiSum += abs( piOrg[8] - piCur[8] );
    uiSum += abs( piOrg[9] - piCur[9] );
    uiSum += abs( piOrg[10] - piCur[10] );
    uiSum += abs( piOrg[11] - piCur[11] );
    uiSum += abs( piOrg[12] - piCur[12] );
    uiSum += abs( piOrg[13] - piCur[13] );
    uiSum += abs( piOrg[14] - piCur[14] );
    uiSum += abs( piOrg[15] - piCur[15] );
    uiSum += abs( piOrg[16] - piCur[16] );
    uiSum += abs( piOrg[17] - piCur[17] );
    uiSum += abs( piOrg[18] - piCur[18] );
    uiSum += abs( piOrg[19] - piCur[19] );
    uiSum += abs( piOrg[20] - piCur[20] );
    uiSum += abs( piOrg[21] - piCur[21] );
    uiSum += abs( piOrg[22] - piCur[22] );
    uiSum += abs( piOrg[23] - piCur[23] );
    uiSum += abs( piOrg[24] - piCur[24] );
    uiSum += abs( piOrg[25] - piCur[25] );
    uiSum += abs( piOrg[26] - piCur[26] );
    uiSum += abs( piOrg[27] - piCur[27] );
    uiSum += abs( piOrg[28] - piCur[28] );
    uiSum += abs( piOrg[29] - piCur[29] );
    uiSum += abs( piOrg[30] - piCur[30] );
    uiSum += abs( piOrg[31] - piCur[31] );
    uiSum += abs( piOrg[32] - piCur[32] );
    uiSum += abs( piOrg[33] - piCur[33] );
    uiSum += abs( piOrg[34] - piCur[34] );
    uiSum += abs( piOrg[35] - piCur[35] );
    uiSum += abs( piOrg[36] - piCur[36] );
    uiSum += abs( piOrg[37] - piCur[37] );
    uiSum += abs( piOrg[38] - piCur[38] );
    uiSum += abs( piOrg[39] - piCur[39] );
    uiSum += abs( piOrg[40] - piCur[40] );
    uiSum += abs( piOrg[41] - piCur[41] );
    uiSum += abs( piOrg[42] - piCur[42] );
    uiSum += abs( piOrg[43] - piCur[43] );
    uiSum += abs( piOrg[44] - piCur[44] );
    uiSum += abs( piOrg[45] - piCur[45] );
    uiSum += abs( piOrg[46] - piCur[46] );
    uiSum += abs( piOrg[47] - piCur[47] );
    uiSum += abs( piOrg[48] - piCur[48] );
    uiSum += abs( piOrg[49] - piCur[49] );
    uiSum += abs( piOrg[50] - piCur[50] );
    uiSum += abs( piOrg[51] - piCur[51] );
    uiSum += abs( piOrg[52] - piCur[52] );
    uiSum += abs( piOrg[53] - piCur[53] );
    uiSum += abs( piOrg[54] - piCur[54] );
    uiSum += abs( piOrg[55] - piCur[55] );
    uiSum += abs( piOrg[56] - piCur[56] );
    uiSum += abs( piOrg[57] - piCur[57] );
    uiSum += abs( piOrg[58] - piCur[58] );
    uiSum += abs( piOrg[59] - piCur[59] );
    uiSum += abs( piOrg[60] - piCur[60] );
    uiSum += abs( piOrg[61] - piCur[61] );
    uiSum += abs( piOrg[62] - piCur[62] );
    uiSum += abs( piOrg[63] - piCur[63] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  uiSum <<= iSubShift;
  return ( uiSum >> g_uiBitIncrement );
}

// --------------------------------------------------------------------------------------------------------------------
// SAD with step (used in fractional search)
// --------------------------------------------------------------------------------------------------------------------

#ifdef ROUNDING_CONTROL

UInt TComRdCost::xGetSADs( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
	  pred = xClip( (piCur[n*iStep] + piRef[n] + bRound) >> 1 );
      uiSum += abs( piOrg[n] - pred );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs4( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for(i = 0; i < 4; i++)
	{
		pred = xClip( (piCur[i*iStep] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	}
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs8( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 8; i++)
	  {
		  pred = xClip( (piCur[i*iStep] + piRef[i] + bRound) >> 1 );
		  uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs16( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep   = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 16; i++)
	  {
		pred = xClip( (piCur[i*iStep] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
	  piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs16N( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {
		for(i = 0; i < 16; i++)
		{
			pred = xClip( (piCur[iStep*(n +i)] + piRef[n + i] + bRound) >> 1 );
			uiSum += abs( piOrg[n +i] -  pred );
		}
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs32( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 32; i++)
	  {
		pred = xClip( (piCur[iStep*i] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs64( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 64; i++)
	  {
		pred = xClip( (piCur[iStep*i] + piRef[i] + bRound) >> 1 );
		uiSum += abs( piOrg[i] - pred );
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum >> g_uiBitIncrement );
}

#endif

UInt TComRdCost::xGetSADs( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
      uiSum += abs( piOrg[n] - piCur[n*iStep] );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs4( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  iStep2 = iStep<<1;
  Int  iStep3 = iStep2 + iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    uiSum += abs( piOrg[0] - piCur[     0] );
    uiSum += abs( piOrg[1] - piCur[iStep ] );
    uiSum += abs( piOrg[2] - piCur[iStep2] );
    uiSum += abs( piOrg[3] - piCur[iStep3] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs8( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  iStep2 = iStep<<1;
  Int  iStep3 = iStep2 + iStep;
  Int  iStep4 = iStep3 + iStep;
  Int  iStep5 = iStep4 + iStep;
  Int  iStep6 = iStep5 + iStep;
  Int  iStep7 = iStep6 + iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    uiSum += abs( piOrg[0] - piCur[     0] );
    uiSum += abs( piOrg[1] - piCur[iStep ] );
    uiSum += abs( piOrg[2] - piCur[iStep2] );
    uiSum += abs( piOrg[3] - piCur[iStep3] );
    uiSum += abs( piOrg[4] - piCur[iStep4] );
    uiSum += abs( piOrg[5] - piCur[iStep5] );
    uiSum += abs( piOrg[6] - piCur[iStep6] );
    uiSum += abs( piOrg[7] - piCur[iStep7] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs16( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep   = pcDtParam->iStep;
  Int  iStep2  = iStep<<1;
  Int  iStep3  = iStep2  + iStep;
  Int  iStep4  = iStep3  + iStep;
  Int  iStep5  = iStep4  + iStep;
  Int  iStep6  = iStep5  + iStep;
  Int  iStep7  = iStep6  + iStep;
  Int  iStep8  = iStep7  + iStep;
  Int  iStep9  = iStep8  + iStep;
  Int  iStep10 = iStep9  + iStep;
  Int  iStep11 = iStep10 + iStep;
  Int  iStep12 = iStep11 + iStep;
  Int  iStep13 = iStep12 + iStep;
  Int  iStep14 = iStep13 + iStep;
  Int  iStep15 = iStep14 + iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    uiSum += abs( piOrg[ 0] - piCur[      0] );
    uiSum += abs( piOrg[ 1] - piCur[iStep  ] );
    uiSum += abs( piOrg[ 2] - piCur[iStep2 ] );
    uiSum += abs( piOrg[ 3] - piCur[iStep3 ] );
    uiSum += abs( piOrg[ 4] - piCur[iStep4 ] );
    uiSum += abs( piOrg[ 5] - piCur[iStep5 ] );
    uiSum += abs( piOrg[ 6] - piCur[iStep6 ] );
    uiSum += abs( piOrg[ 7] - piCur[iStep7 ] );
    uiSum += abs( piOrg[ 8] - piCur[iStep8 ] );
    uiSum += abs( piOrg[ 9] - piCur[iStep9 ] );
    uiSum += abs( piOrg[10] - piCur[iStep10] );
    uiSum += abs( piOrg[11] - piCur[iStep11] );
    uiSum += abs( piOrg[12] - piCur[iStep12] );
    uiSum += abs( piOrg[13] - piCur[iStep13] );
    uiSum += abs( piOrg[14] - piCur[iStep14] );
    uiSum += abs( piOrg[15] - piCur[iStep15] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs16N( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {
      uiSum += abs( piOrg[n +0] - piCur[iStep*(n +0)] );
      uiSum += abs( piOrg[n +1] - piCur[iStep*(n +1)] );
      uiSum += abs( piOrg[n +2] - piCur[iStep*(n +2)] );
      uiSum += abs( piOrg[n +3] - piCur[iStep*(n +3)] );
      uiSum += abs( piOrg[n +4] - piCur[iStep*(n +4)] );
      uiSum += abs( piOrg[n +5] - piCur[iStep*(n +5)] );
      uiSum += abs( piOrg[n +6] - piCur[iStep*(n +6)] );
      uiSum += abs( piOrg[n +7] - piCur[iStep*(n +7)] );
      uiSum += abs( piOrg[n +8] - piCur[iStep*(n +8)] );
      uiSum += abs( piOrg[n +9] - piCur[iStep*(n +9)] );
      uiSum += abs( piOrg[n+10] - piCur[iStep*(n+10)] );
      uiSum += abs( piOrg[n+11] - piCur[iStep*(n+11)] );
      uiSum += abs( piOrg[n+12] - piCur[iStep*(n+12)] );
      uiSum += abs( piOrg[n+13] - piCur[iStep*(n+13)] );
      uiSum += abs( piOrg[n+14] - piCur[iStep*(n+14)] );
      uiSum += abs( piOrg[n+15] - piCur[iStep*(n+15)] );
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs32( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  iStep2  = iStep<<1;
  Int  iStep3  = iStep2  + iStep;
  Int  iStep4  = iStep3  + iStep;
  Int  iStep5  = iStep4  + iStep;
  Int  iStep6  = iStep5  + iStep;
  Int  iStep7  = iStep6  + iStep;
  Int  iStep8  = iStep7  + iStep;
  Int  iStep9  = iStep8  + iStep;
  Int  iStep10 = iStep9  + iStep;
  Int  iStep11 = iStep10 + iStep;
  Int  iStep12 = iStep11 + iStep;
  Int  iStep13 = iStep12 + iStep;
  Int  iStep14 = iStep13 + iStep;
  Int  iStep15 = iStep14 + iStep;
  Int  iStep16 = iStep15 + iStep;
  Int  iStep17 = iStep16 + iStep;
  Int  iStep18 = iStep17 + iStep;
  Int  iStep19 = iStep18 + iStep;
  Int  iStep20 = iStep19 + iStep;
  Int  iStep21 = iStep20 + iStep;
  Int  iStep22 = iStep21 + iStep;
  Int  iStep23 = iStep22 + iStep;
  Int  iStep24 = iStep23 + iStep;
  Int  iStep25 = iStep24 + iStep;
  Int  iStep26 = iStep25 + iStep;
  Int  iStep27 = iStep26 + iStep;
  Int  iStep28 = iStep27 + iStep;
  Int  iStep29 = iStep28 + iStep;
  Int  iStep30 = iStep29 + iStep;
  Int  iStep31 = iStep30 + iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    uiSum += abs( piOrg[ 0] - piCur[      0] );
    uiSum += abs( piOrg[ 1] - piCur[iStep  ] );
    uiSum += abs( piOrg[ 2] - piCur[iStep2 ] );
    uiSum += abs( piOrg[ 3] - piCur[iStep3 ] );
    uiSum += abs( piOrg[ 4] - piCur[iStep4 ] );
    uiSum += abs( piOrg[ 5] - piCur[iStep5 ] );
    uiSum += abs( piOrg[ 6] - piCur[iStep6 ] );
    uiSum += abs( piOrg[ 7] - piCur[iStep7 ] );
    uiSum += abs( piOrg[ 8] - piCur[iStep8 ] );
    uiSum += abs( piOrg[ 9] - piCur[iStep9 ] );
    uiSum += abs( piOrg[10] - piCur[iStep10] );
    uiSum += abs( piOrg[11] - piCur[iStep11] );
    uiSum += abs( piOrg[12] - piCur[iStep12] );
    uiSum += abs( piOrg[13] - piCur[iStep13] );
    uiSum += abs( piOrg[14] - piCur[iStep14] );
    uiSum += abs( piOrg[15] - piCur[iStep15] );
    uiSum += abs( piOrg[16] - piCur[iStep16] );
    uiSum += abs( piOrg[17] - piCur[iStep17] );
    uiSum += abs( piOrg[18] - piCur[iStep18] );
    uiSum += abs( piOrg[19] - piCur[iStep19] );
    uiSum += abs( piOrg[20] - piCur[iStep20] );
    uiSum += abs( piOrg[21] - piCur[iStep21] );
    uiSum += abs( piOrg[22] - piCur[iStep22] );
    uiSum += abs( piOrg[23] - piCur[iStep23] );
    uiSum += abs( piOrg[24] - piCur[iStep24] );
    uiSum += abs( piOrg[25] - piCur[iStep25] );
    uiSum += abs( piOrg[26] - piCur[iStep26] );
    uiSum += abs( piOrg[27] - piCur[iStep27] );
    uiSum += abs( piOrg[28] - piCur[iStep28] );
    uiSum += abs( piOrg[29] - piCur[iStep29] );
    uiSum += abs( piOrg[30] - piCur[iStep30] );
    uiSum += abs( piOrg[31] - piCur[iStep31] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetSADs64( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  iStep2  = iStep<<1;
  Int  iStep3  = iStep2  + iStep;
  Int  iStep4  = iStep3  + iStep;
  Int  iStep5  = iStep4  + iStep;
  Int  iStep6  = iStep5  + iStep;
  Int  iStep7  = iStep6  + iStep;
  Int  iStep8  = iStep7  + iStep;
  Int  iStep9  = iStep8  + iStep;
  Int  iStep10 = iStep9  + iStep;
  Int  iStep11 = iStep10 + iStep;
  Int  iStep12 = iStep11 + iStep;
  Int  iStep13 = iStep12 + iStep;
  Int  iStep14 = iStep13 + iStep;
  Int  iStep15 = iStep14 + iStep;
  Int  iStep16 = iStep15 + iStep;
  Int  iStep17 = iStep16 + iStep;
  Int  iStep18 = iStep17 + iStep;
  Int  iStep19 = iStep18 + iStep;
  Int  iStep20 = iStep19 + iStep;
  Int  iStep21 = iStep20 + iStep;
  Int  iStep22 = iStep21 + iStep;
  Int  iStep23 = iStep22 + iStep;
  Int  iStep24 = iStep23 + iStep;
  Int  iStep25 = iStep24 + iStep;
  Int  iStep26 = iStep25 + iStep;
  Int  iStep27 = iStep26 + iStep;
  Int  iStep28 = iStep27 + iStep;
  Int  iStep29 = iStep28 + iStep;
  Int  iStep30 = iStep29 + iStep;
  Int  iStep31 = iStep30 + iStep;
  Int  iStep32 = iStep31 + iStep;
  Int  iStep33 = iStep32 + iStep;
  Int  iStep34 = iStep33 + iStep;
  Int  iStep35 = iStep34 + iStep;
  Int  iStep36 = iStep35 + iStep;
  Int  iStep37 = iStep36 + iStep;
  Int  iStep38 = iStep37 + iStep;
  Int  iStep39 = iStep38 + iStep;
  Int  iStep40 = iStep39 + iStep;
  Int  iStep41 = iStep40 + iStep;
  Int  iStep42 = iStep41 + iStep;
  Int  iStep43 = iStep42 + iStep;
  Int  iStep44 = iStep43 + iStep;
  Int  iStep45 = iStep44 + iStep;
  Int  iStep46 = iStep45 + iStep;
  Int  iStep47 = iStep46 + iStep;
  Int  iStep48 = iStep47 + iStep;
  Int  iStep49 = iStep48 + iStep;
  Int  iStep50 = iStep49 + iStep;
  Int  iStep51 = iStep50 + iStep;
  Int  iStep52 = iStep51 + iStep;
  Int  iStep53 = iStep52 + iStep;
  Int  iStep54 = iStep53 + iStep;
  Int  iStep55 = iStep54 + iStep;
  Int  iStep56 = iStep55 + iStep;
  Int  iStep57 = iStep56 + iStep;
  Int  iStep58 = iStep57 + iStep;
  Int  iStep59 = iStep58 + iStep;
  Int  iStep60 = iStep59 + iStep;
  Int  iStep61 = iStep60 + iStep;
  Int  iStep62 = iStep61 + iStep;
  Int  iStep63 = iStep62 + iStep;

  UInt uiSum = 0;

  for( ; iRows != 0; iRows-- )
  {
    uiSum += abs( piOrg[ 0] - piCur[      0] );
    uiSum += abs( piOrg[ 1] - piCur[iStep  ] );
    uiSum += abs( piOrg[ 2] - piCur[iStep2 ] );
    uiSum += abs( piOrg[ 3] - piCur[iStep3 ] );
    uiSum += abs( piOrg[ 4] - piCur[iStep4 ] );
    uiSum += abs( piOrg[ 5] - piCur[iStep5 ] );
    uiSum += abs( piOrg[ 6] - piCur[iStep6 ] );
    uiSum += abs( piOrg[ 7] - piCur[iStep7 ] );
    uiSum += abs( piOrg[ 8] - piCur[iStep8 ] );
    uiSum += abs( piOrg[ 9] - piCur[iStep9 ] );
    uiSum += abs( piOrg[10] - piCur[iStep10] );
    uiSum += abs( piOrg[11] - piCur[iStep11] );
    uiSum += abs( piOrg[12] - piCur[iStep12] );
    uiSum += abs( piOrg[13] - piCur[iStep13] );
    uiSum += abs( piOrg[14] - piCur[iStep14] );
    uiSum += abs( piOrg[15] - piCur[iStep15] );
    uiSum += abs( piOrg[16] - piCur[iStep16] );
    uiSum += abs( piOrg[17] - piCur[iStep17] );
    uiSum += abs( piOrg[18] - piCur[iStep18] );
    uiSum += abs( piOrg[19] - piCur[iStep19] );
    uiSum += abs( piOrg[20] - piCur[iStep20] );
    uiSum += abs( piOrg[21] - piCur[iStep21] );
    uiSum += abs( piOrg[22] - piCur[iStep22] );
    uiSum += abs( piOrg[23] - piCur[iStep23] );
    uiSum += abs( piOrg[24] - piCur[iStep24] );
    uiSum += abs( piOrg[25] - piCur[iStep25] );
    uiSum += abs( piOrg[26] - piCur[iStep26] );
    uiSum += abs( piOrg[27] - piCur[iStep27] );
    uiSum += abs( piOrg[28] - piCur[iStep28] );
    uiSum += abs( piOrg[29] - piCur[iStep29] );
    uiSum += abs( piOrg[30] - piCur[iStep30] );
    uiSum += abs( piOrg[31] - piCur[iStep31] );
    uiSum += abs( piOrg[32] - piCur[iStep32] );
    uiSum += abs( piOrg[33] - piCur[iStep33] );
    uiSum += abs( piOrg[34] - piCur[iStep34] );
    uiSum += abs( piOrg[35] - piCur[iStep35] );
    uiSum += abs( piOrg[36] - piCur[iStep36] );
    uiSum += abs( piOrg[37] - piCur[iStep37] );
    uiSum += abs( piOrg[38] - piCur[iStep38] );
    uiSum += abs( piOrg[39] - piCur[iStep39] );
    uiSum += abs( piOrg[40] - piCur[iStep40] );
    uiSum += abs( piOrg[41] - piCur[iStep41] );
    uiSum += abs( piOrg[42] - piCur[iStep42] );
    uiSum += abs( piOrg[43] - piCur[iStep43] );
    uiSum += abs( piOrg[44] - piCur[iStep44] );
    uiSum += abs( piOrg[45] - piCur[iStep45] );
    uiSum += abs( piOrg[46] - piCur[iStep46] );
    uiSum += abs( piOrg[47] - piCur[iStep47] );
    uiSum += abs( piOrg[48] - piCur[iStep48] );
    uiSum += abs( piOrg[49] - piCur[iStep49] );
    uiSum += abs( piOrg[50] - piCur[iStep50] );
    uiSum += abs( piOrg[51] - piCur[iStep51] );
    uiSum += abs( piOrg[52] - piCur[iStep52] );
    uiSum += abs( piOrg[53] - piCur[iStep53] );
    uiSum += abs( piOrg[54] - piCur[iStep54] );
    uiSum += abs( piOrg[55] - piCur[iStep55] );
    uiSum += abs( piOrg[56] - piCur[iStep56] );
    uiSum += abs( piOrg[57] - piCur[iStep57] );
    uiSum += abs( piOrg[58] - piCur[iStep58] );
    uiSum += abs( piOrg[59] - piCur[iStep59] );
    uiSum += abs( piOrg[60] - piCur[iStep60] );
    uiSum += abs( piOrg[61] - piCur[iStep61] );
    uiSum += abs( piOrg[62] - piCur[iStep62] );
    uiSum += abs( piOrg[63] - piCur[iStep63] );

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

// --------------------------------------------------------------------------------------------------------------------
// SSE
// --------------------------------------------------------------------------------------------------------------------

#ifdef ROUNDING_CONTROL

UInt TComRdCost::xGetSSE( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int iTemp;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
	  pred = xClip( (piCur[n] + piRef[n] + bRound) >> 1 );
      iTemp = piOrg[n] - pred;
      uiSum += ( iTemp * iTemp ) >> uiShift;
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE4( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 4; i++)
	  {
		  pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1 );
		  iTemp = piOrg[i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
	  }    
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE8( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
	  for(i = 0; i < 8; i++)
	  {
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1);
		iTemp = piOrg[i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
	  }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
	  piRef += pcDtParam->iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE16( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
	for(i = 0; i < 16; i++)
	{
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1);
		iTemp = piOrg[i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
	}
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += pcDtParam->iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE16N( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {
		for(i = 0; i < 16; i++)
		{
			pred = xClip( (piCur[n+ i] + piRef[n+ i] + bRound) >> 1);
			iTemp = piOrg[n+ i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
		}
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE32( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
	for(i = 0; i < 32; i++)
	{
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1);
		iTemp = piOrg[i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
	}
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += pcDtParam->iCols;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE64( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Pel  pred;
  Int  i;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
	for(i = 0; i < 32; i++)
	{
		pred = xClip( (piCur[i] + piRef[i] + bRound) >> 1);
		iTemp = piOrg[i] - pred; uiSum += ( iTemp * iTemp ) >> uiShift;
	}
    piOrg += iStrideOrg;
    piCur += iStrideCur;
	piRef += pcDtParam->iCols;
  }

  return ( uiSum );
}

#endif

UInt TComRdCost::xGetSSE( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int iTemp;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n++ )
    {
      iTemp = piOrg[n  ] - piCur[n  ];
      uiSum += ( iTemp * iTemp ) >> uiShift;
    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE4( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {

    iTemp = piOrg[0] - piCur[0]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[1] - piCur[1]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[2] - piCur[2]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[3] - piCur[3]; uiSum += ( iTemp * iTemp ) >> uiShift;

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE8( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
    iTemp = piOrg[0] - piCur[0]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[1] - piCur[1]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[2] - piCur[2]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[3] - piCur[3]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[4] - piCur[4]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[5] - piCur[5]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[6] - piCur[6]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[7] - piCur[7]; uiSum += ( iTemp * iTemp ) >> uiShift;

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE16( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;

  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {

    iTemp = piOrg[ 0] - piCur[ 0]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 1] - piCur[ 1]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 2] - piCur[ 2]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 3] - piCur[ 3]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 4] - piCur[ 4]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 5] - piCur[ 5]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 6] - piCur[ 6]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 7] - piCur[ 7]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 8] - piCur[ 8]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 9] - piCur[ 9]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[10] - piCur[10]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[11] - piCur[11]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[12] - piCur[12]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[13] - piCur[13]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[14] - piCur[14]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[15] - piCur[15]; uiSum += ( iTemp * iTemp ) >> uiShift;

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE16N( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
    for (Int n = 0; n < iCols; n+=16 )
    {

      iTemp = piOrg[n+ 0] - piCur[n+ 0]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 1] - piCur[n+ 1]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 2] - piCur[n+ 2]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 3] - piCur[n+ 3]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 4] - piCur[n+ 4]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 5] - piCur[n+ 5]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 6] - piCur[n+ 6]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 7] - piCur[n+ 7]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 8] - piCur[n+ 8]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+ 9] - piCur[n+ 9]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+10] - piCur[n+10]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+11] - piCur[n+11]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+12] - piCur[n+12]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+13] - piCur[n+13]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+14] - piCur[n+14]; uiSum += ( iTemp * iTemp ) >> uiShift;
      iTemp = piOrg[n+15] - piCur[n+15]; uiSum += ( iTemp * iTemp ) >> uiShift;

    }
    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE32( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {

    iTemp = piOrg[ 0] - piCur[ 0]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 1] - piCur[ 1]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 2] - piCur[ 2]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 3] - piCur[ 3]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 4] - piCur[ 4]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 5] - piCur[ 5]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 6] - piCur[ 6]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 7] - piCur[ 7]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 8] - piCur[ 8]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 9] - piCur[ 9]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[10] - piCur[10]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[11] - piCur[11]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[12] - piCur[12]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[13] - piCur[13]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[14] - piCur[14]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[15] - piCur[15]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[16] - piCur[16]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[17] - piCur[17]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[18] - piCur[18]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[19] - piCur[19]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[20] - piCur[20]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[21] - piCur[21]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[22] - piCur[22]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[23] - piCur[23]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[24] - piCur[24]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[25] - piCur[25]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[26] - piCur[26]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[27] - piCur[27]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[28] - piCur[28]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[29] - piCur[29]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[30] - piCur[30]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[31] - piCur[31]; uiSum += ( iTemp * iTemp ) >> uiShift;

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

UInt TComRdCost::xGetSSE64( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStrideCur = pcDtParam->iStrideCur;

  UInt uiSum = 0;
  UInt uiShift = g_uiBitIncrement<<1;
  Int  iTemp;

  for( ; iRows != 0; iRows-- )
  {
    iTemp = piOrg[ 0] - piCur[ 0]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 1] - piCur[ 1]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 2] - piCur[ 2]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 3] - piCur[ 3]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 4] - piCur[ 4]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 5] - piCur[ 5]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 6] - piCur[ 6]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 7] - piCur[ 7]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 8] - piCur[ 8]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[ 9] - piCur[ 9]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[10] - piCur[10]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[11] - piCur[11]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[12] - piCur[12]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[13] - piCur[13]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[14] - piCur[14]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[15] - piCur[15]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[16] - piCur[16]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[17] - piCur[17]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[18] - piCur[18]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[19] - piCur[19]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[20] - piCur[20]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[21] - piCur[21]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[22] - piCur[22]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[23] - piCur[23]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[24] - piCur[24]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[25] - piCur[25]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[26] - piCur[26]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[27] - piCur[27]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[28] - piCur[28]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[29] - piCur[29]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[30] - piCur[30]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[31] - piCur[31]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[32] - piCur[32]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[33] - piCur[33]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[34] - piCur[34]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[35] - piCur[35]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[36] - piCur[36]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[37] - piCur[37]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[38] - piCur[38]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[39] - piCur[39]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[40] - piCur[40]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[41] - piCur[41]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[42] - piCur[42]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[43] - piCur[43]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[44] - piCur[44]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[45] - piCur[45]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[46] - piCur[46]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[47] - piCur[47]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[48] - piCur[48]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[49] - piCur[49]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[50] - piCur[50]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[51] - piCur[51]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[52] - piCur[52]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[53] - piCur[53]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[54] - piCur[54]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[55] - piCur[55]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[56] - piCur[56]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[57] - piCur[57]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[58] - piCur[58]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[59] - piCur[59]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[60] - piCur[60]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[61] - piCur[61]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[62] - piCur[62]; uiSum += ( iTemp * iTemp ) >> uiShift;
    iTemp = piOrg[63] - piCur[63]; uiSum += ( iTemp * iTemp ) >> uiShift;

    piOrg += iStrideOrg;
    piCur += iStrideCur;
  }

  return ( uiSum );
}

// --------------------------------------------------------------------------------------------------------------------
// HADAMARD with step (used in fractional search)
// --------------------------------------------------------------------------------------------------------------------

#ifdef ROUNDING_CONTROL

UInt TComRdCost::xCalcHADs2x2( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep, Pel* pRefY, Int refYStride, Bool bRound )
{
  Int satd = 0, diff[4], m[4];
  Pel pred;

  pred = xClip( (piCur[0] + pRefY[0] + bRound) >> 1);
  diff[0] = (piOrg[0             ] - pred) << 1;
  pred = xClip( (piCur[iStep] + pRefY[1] + bRound) >> 1);
  diff[1] = (piOrg[1             ] - pred) << 1;
  pred = xClip( (piCur[iStrideCur] + pRefY[refYStride] + bRound) >> 1);
  diff[2] = (piOrg[iStrideOrg    ] - pred) << 1;
  pred = xClip( (piCur[iStep + iStrideCur] + pRefY[refYStride + 1] + bRound) >> 1);
  diff[3] = (piOrg[iStrideOrg + 1] - pred) << 1;

  m[0] = diff[0] + diff[2];
  m[1] = diff[1] + diff[3];
  m[2] = diff[0] - diff[2];
  m[3] = diff[1] - diff[3];

  satd += abs(m[0] + m[1]);
  satd += abs(m[0] - m[1]);
  satd += abs(m[2] + m[3]);
  satd += abs(m[2] - m[3]);

  return satd;
}

UInt TComRdCost::xCalcHADs4x4( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep, Pel* pRefY, Int refYStride, Bool bRound )
{
  Int k, satd = 0, diff[16], m[16], d[16];
  Pel pred;
  Pel* piRef = pRefY;

  for( k = 0; k < 16; k+=4 )
  {
    pred = xClip( (piCur[0*iStep] + piRef[0] + bRound) >> 1);
    diff[k+0] = (piOrg[0] - pred) << 1;
	pred = xClip( (piCur[1*iStep] + piRef[1] + bRound) >> 1);
    diff[k+1] = (piOrg[1] - pred) << 1;
	pred = xClip( (piCur[2*iStep] + piRef[2] + bRound) >> 1);
    diff[k+2] = (piOrg[2] - pred) << 1;
	pred = xClip( (piCur[3*iStep] + piRef[3] + bRound) >> 1);
    diff[k+3] = (piOrg[3] - pred) << 1;

    piCur += iStrideCur;
    piOrg += iStrideOrg;
	piRef += refYStride;
  }

  /*===== hadamard transform =====*/
  m[ 0] = diff[ 0] + diff[12];
  m[ 1] = diff[ 1] + diff[13];
  m[ 2] = diff[ 2] + diff[14];
  m[ 3] = diff[ 3] + diff[15];
  m[ 4] = diff[ 4] + diff[ 8];
  m[ 5] = diff[ 5] + diff[ 9];
  m[ 6] = diff[ 6] + diff[10];
  m[ 7] = diff[ 7] + diff[11];
  m[ 8] = diff[ 4] - diff[ 8];
  m[ 9] = diff[ 5] - diff[ 9];
  m[10] = diff[ 6] - diff[10];
  m[11] = diff[ 7] - diff[11];
  m[12] = diff[ 0] - diff[12];
  m[13] = diff[ 1] - diff[13];
  m[14] = diff[ 2] - diff[14];
  m[15] = diff[ 3] - diff[15];

  d[ 0] = m[ 0] + m[ 4];
  d[ 1] = m[ 1] + m[ 5];
  d[ 2] = m[ 2] + m[ 6];
  d[ 3] = m[ 3] + m[ 7];
  d[ 4] = m[ 8] + m[12];
  d[ 5] = m[ 9] + m[13];
  d[ 6] = m[10] + m[14];
  d[ 7] = m[11] + m[15];
  d[ 8] = m[ 0] - m[ 4];
  d[ 9] = m[ 1] - m[ 5];
  d[10] = m[ 2] - m[ 6];
  d[11] = m[ 3] - m[ 7];
  d[12] = m[12] - m[ 8];
  d[13] = m[13] - m[ 9];
  d[14] = m[14] - m[10];
  d[15] = m[15] - m[11];

  m[ 0] = d[ 0] + d[ 3];
  m[ 1] = d[ 1] + d[ 2];
  m[ 2] = d[ 1] - d[ 2];
  m[ 3] = d[ 0] - d[ 3];
  m[ 4] = d[ 4] + d[ 7];
  m[ 5] = d[ 5] + d[ 6];
  m[ 6] = d[ 5] - d[ 6];
  m[ 7] = d[ 4] - d[ 7];
  m[ 8] = d[ 8] + d[11];
  m[ 9] = d[ 9] + d[10];
  m[10] = d[ 9] - d[10];
  m[11] = d[ 8] - d[11];
  m[12] = d[12] + d[15];
  m[13] = d[13] + d[14];
  m[14] = d[13] - d[14];
  m[15] = d[12] - d[15];

  d[ 0] = m[ 0] + m[ 1];
  d[ 1] = m[ 0] - m[ 1];
  d[ 2] = m[ 2] + m[ 3];
  d[ 3] = m[ 3] - m[ 2];
  d[ 4] = m[ 4] + m[ 5];
  d[ 5] = m[ 4] - m[ 5];
  d[ 6] = m[ 6] + m[ 7];
  d[ 7] = m[ 7] - m[ 6];
  d[ 8] = m[ 8] + m[ 9];
  d[ 9] = m[ 8] - m[ 9];
  d[10] = m[10] + m[11];
  d[11] = m[11] - m[10];
  d[12] = m[12] + m[13];
  d[13] = m[12] - m[13];
  d[14] = m[14] + m[15];
  d[15] = m[15] - m[14];

  for (k=0; k<16; ++k)
  {
    satd += abs(d[k]);
  }
  satd = ((satd+1)>>1);

  return satd;
}

UInt TComRdCost::xCalcHADs8x8( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep, Pel* pRefY, Int refYStride, Bool bRound )
{
  Int k, i, j, jj, sad=0;
  Int diff[64], m1[8][8], m2[8][8], m3[8][8];
  Pel pred;
  Pel* piRef = pRefY;

  for( k = 0; k < 64; k+=8 )
  {
	  for(i = 0; i < 8; i++)
	  {
		pred = xClip( (piCur[i*iStep] + piRef[i] + bRound) >> 1 );
		diff[k+i] = (piOrg[i] - pred) << 1;
	  }
      piCur += iStrideCur;
      piOrg += iStrideOrg;
	  piRef += refYStride;
  }

  //horizontal
  for (j=0; j < 8; j++)
  {
    jj = j << 3;
    m2[j][0] = diff[jj  ] + diff[jj+4];
    m2[j][1] = diff[jj+1] + diff[jj+5];
    m2[j][2] = diff[jj+2] + diff[jj+6];
    m2[j][3] = diff[jj+3] + diff[jj+7];
    m2[j][4] = diff[jj  ] - diff[jj+4];
    m2[j][5] = diff[jj+1] - diff[jj+5];
    m2[j][6] = diff[jj+2] - diff[jj+6];
    m2[j][7] = diff[jj+3] - diff[jj+7];

    m1[j][0] = m2[j][0] + m2[j][2];
    m1[j][1] = m2[j][1] + m2[j][3];
    m1[j][2] = m2[j][0] - m2[j][2];
    m1[j][3] = m2[j][1] - m2[j][3];
    m1[j][4] = m2[j][4] + m2[j][6];
    m1[j][5] = m2[j][5] + m2[j][7];
    m1[j][6] = m2[j][4] - m2[j][6];
    m1[j][7] = m2[j][5] - m2[j][7];

    m2[j][0] = m1[j][0] + m1[j][1];
    m2[j][1] = m1[j][0] - m1[j][1];
    m2[j][2] = m1[j][2] + m1[j][3];
    m2[j][3] = m1[j][2] - m1[j][3];
    m2[j][4] = m1[j][4] + m1[j][5];
    m2[j][5] = m1[j][4] - m1[j][5];
    m2[j][6] = m1[j][6] + m1[j][7];
    m2[j][7] = m1[j][6] - m1[j][7];
  }

  //vertical
  for (i=0; i < 8; i++)
  {
    m3[0][i] = m2[0][i] + m2[4][i];
    m3[1][i] = m2[1][i] + m2[5][i];
    m3[2][i] = m2[2][i] + m2[6][i];
    m3[3][i] = m2[3][i] + m2[7][i];
    m3[4][i] = m2[0][i] - m2[4][i];
    m3[5][i] = m2[1][i] - m2[5][i];
    m3[6][i] = m2[2][i] - m2[6][i];
    m3[7][i] = m2[3][i] - m2[7][i];

    m1[0][i] = m3[0][i] + m3[2][i];
    m1[1][i] = m3[1][i] + m3[3][i];
    m1[2][i] = m3[0][i] - m3[2][i];
    m1[3][i] = m3[1][i] - m3[3][i];
    m1[4][i] = m3[4][i] + m3[6][i];
    m1[5][i] = m3[5][i] + m3[7][i];
    m1[6][i] = m3[4][i] - m3[6][i];
    m1[7][i] = m3[5][i] - m3[7][i];

    m2[0][i] = m1[0][i] + m1[1][i];
    m2[1][i] = m1[0][i] - m1[1][i];
    m2[2][i] = m1[2][i] + m1[3][i];
    m2[3][i] = m1[2][i] - m1[3][i];
    m2[4][i] = m1[4][i] + m1[5][i];
    m2[5][i] = m1[4][i] - m1[5][i];
    m2[6][i] = m1[6][i] + m1[7][i];
    m2[7][i] = m1[6][i] - m1[7][i];
  }
  for (j=0; j < 8; j++)
    for (i=0; i < 8; i++)
      sad += (abs(m2[j][i]));

  sad=((sad+2)>>2);

  return sad;
}

UInt TComRdCost::xGetHADs4( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  y;
  Int  iOffsetOrg = iStrideOrg<<2;
  Int  iOffsetCur = iStrideCur<<2;

  UInt uiSum = 0;

  for ( y=0; y<iRows; y+= 4 )
  {
    uiSum += xCalcHADs4x4( piOrg, piCur, iStrideOrg, iStrideCur, iStep, piRef, pcDtParam->iCols, bRound );
    piOrg += iOffsetOrg;
    piCur += iOffsetCur;
	piRef += (pcDtParam->iCols << 2);
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetHADs8( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  y;

  UInt uiSum = 0;

  if ( iRows == 4 ) // 8x4 case
  {
    uiSum += xCalcHADs4x4( piOrg+0, piCur        , iStrideOrg, iStrideCur, iStep, piRef, pcDtParam->iCols, bRound );
    uiSum += xCalcHADs4x4( piOrg+4, piCur+4*iStep, iStrideOrg, iStrideCur, iStep, piRef+4, pcDtParam->iCols, bRound );
  }
  else
  {
    Int  iOffsetOrg = iStrideOrg<<3;
    Int  iOffsetCur = iStrideCur<<3;
    for ( y=0; y<iRows; y+= 8 )
    {
      uiSum += xCalcHADs8x8( piOrg, piCur, iStrideOrg, iStrideCur, iStep, piRef, pcDtParam->iCols, bRound );
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
	  piRef += (pcDtParam->iCols << 3);
    }
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetHADs( DistParam* pcDtParam, Pel* pRefY, Bool bRound )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Pel* piRef   = pRefY;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;

  Int  x, y;

  UInt uiSum = 0;

  if( ( iRows % 8 == 0) && (iCols % 8 == 0) )
  {
    Int  iOffsetOrg = iStrideOrg<<3;
    Int  iOffsetCur = iStrideCur<<3;
    for ( y=0; y<iRows; y+= 8 )
    {
      for ( x=0; x<iCols; x+= 8 )  // do HAD over 8xiCols pixels
      {
        uiSum += xCalcHADs8x8( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep, &piRef[x], iCols, bRound );
      }
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
	  piRef += (iCols << 3);
    }
  }
  else if( ( iRows % 4 == 0) && (iCols % 4 == 0) )
  {
    Int  iOffsetOrg = iStrideOrg<<2;
    Int  iOffsetCur = iStrideCur<<2;

    for ( y=0; y<iRows; y+= 4 )
    {
      for ( x=0; x<iCols; x+= 4 ) // do HAD over 4xiCols pixels 
      {
        uiSum += xCalcHADs4x4( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep, &piRef[x], iCols, bRound );
      }
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
	  piRef += (iCols << 2);
    }
  }
  else
  {
    for ( y=0; y<iRows; y+=2 )
    {
      for ( x=0; x<iCols; x+=2 )// do HAD over 2xiCols pixels 
      {
        uiSum += xCalcHADs2x2( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep, &piRef[x], iCols, bRound );
      }
      piOrg += iStrideOrg;  // should this be (iStrideOrg << 1) ? 
      piCur += iStrideCur;
	  piRef += iCols;
    }
  }

  return ( uiSum >> g_uiBitIncrement );
}

#endif

UInt TComRdCost::xCalcHADs2x2( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep )
{
  Int satd = 0, diff[4], m[4];
  diff[0] = piOrg[0             ] - piCur[0*iStep];
  diff[1] = piOrg[1             ] - piCur[1*iStep];
  diff[2] = piOrg[iStrideOrg    ] - piCur[0*iStep + iStrideCur];
  diff[3] = piOrg[iStrideOrg + 1] - piCur[1*iStep + iStrideCur];

  m[0] = diff[0] + diff[2];
  m[1] = diff[1] + diff[3];
  m[2] = diff[0] - diff[2];
  m[3] = diff[1] - diff[3];

  satd += abs(m[0] + m[1]);
  satd += abs(m[0] - m[1]);
  satd += abs(m[2] + m[3]);
  satd += abs(m[2] - m[3]);

  return satd;
}

UInt TComRdCost::xCalcHADs4x4( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep )
{
  Int k, satd = 0, diff[16], m[16], d[16];

  for( k = 0; k < 16; k+=4 )
  {
    diff[k+0] = piOrg[0] - piCur[0*iStep];
    diff[k+1] = piOrg[1] - piCur[1*iStep];
    diff[k+2] = piOrg[2] - piCur[2*iStep];
    diff[k+3] = piOrg[3] - piCur[3*iStep];

    piCur += iStrideCur;
    piOrg += iStrideOrg;
  }

  /*===== hadamard transform =====*/
  m[ 0] = diff[ 0] + diff[12];
  m[ 1] = diff[ 1] + diff[13];
  m[ 2] = diff[ 2] + diff[14];
  m[ 3] = diff[ 3] + diff[15];
  m[ 4] = diff[ 4] + diff[ 8];
  m[ 5] = diff[ 5] + diff[ 9];
  m[ 6] = diff[ 6] + diff[10];
  m[ 7] = diff[ 7] + diff[11];
  m[ 8] = diff[ 4] - diff[ 8];
  m[ 9] = diff[ 5] - diff[ 9];
  m[10] = diff[ 6] - diff[10];
  m[11] = diff[ 7] - diff[11];
  m[12] = diff[ 0] - diff[12];
  m[13] = diff[ 1] - diff[13];
  m[14] = diff[ 2] - diff[14];
  m[15] = diff[ 3] - diff[15];

  d[ 0] = m[ 0] + m[ 4];
  d[ 1] = m[ 1] + m[ 5];
  d[ 2] = m[ 2] + m[ 6];
  d[ 3] = m[ 3] + m[ 7];
  d[ 4] = m[ 8] + m[12];
  d[ 5] = m[ 9] + m[13];
  d[ 6] = m[10] + m[14];
  d[ 7] = m[11] + m[15];
  d[ 8] = m[ 0] - m[ 4];
  d[ 9] = m[ 1] - m[ 5];
  d[10] = m[ 2] - m[ 6];
  d[11] = m[ 3] - m[ 7];
  d[12] = m[12] - m[ 8];
  d[13] = m[13] - m[ 9];
  d[14] = m[14] - m[10];
  d[15] = m[15] - m[11];

  m[ 0] = d[ 0] + d[ 3];
  m[ 1] = d[ 1] + d[ 2];
  m[ 2] = d[ 1] - d[ 2];
  m[ 3] = d[ 0] - d[ 3];
  m[ 4] = d[ 4] + d[ 7];
  m[ 5] = d[ 5] + d[ 6];
  m[ 6] = d[ 5] - d[ 6];
  m[ 7] = d[ 4] - d[ 7];
  m[ 8] = d[ 8] + d[11];
  m[ 9] = d[ 9] + d[10];
  m[10] = d[ 9] - d[10];
  m[11] = d[ 8] - d[11];
  m[12] = d[12] + d[15];
  m[13] = d[13] + d[14];
  m[14] = d[13] - d[14];
  m[15] = d[12] - d[15];

  d[ 0] = m[ 0] + m[ 1];
  d[ 1] = m[ 0] - m[ 1];
  d[ 2] = m[ 2] + m[ 3];
  d[ 3] = m[ 3] - m[ 2];
  d[ 4] = m[ 4] + m[ 5];
  d[ 5] = m[ 4] - m[ 5];
  d[ 6] = m[ 6] + m[ 7];
  d[ 7] = m[ 7] - m[ 6];
  d[ 8] = m[ 8] + m[ 9];
  d[ 9] = m[ 8] - m[ 9];
  d[10] = m[10] + m[11];
  d[11] = m[11] - m[10];
  d[12] = m[12] + m[13];
  d[13] = m[12] - m[13];
  d[14] = m[14] + m[15];
  d[15] = m[15] - m[14];

  for (k=0; k<16; ++k)
  {
    satd += abs(d[k]);
  }
  satd = ((satd+1)>>1);

  return satd;
}

UInt TComRdCost::xCalcHADs8x8( Pel *piOrg, Pel *piCur, Int iStrideOrg, Int iStrideCur, Int iStep )
{
  Int k, i, j, jj, sad=0;
  Int diff[64], m1[8][8], m2[8][8], m3[8][8];
  Int iStep2 = iStep<<1;
  Int iStep3 = iStep2 + iStep;
  Int iStep4 = iStep3 + iStep;
  Int iStep5 = iStep4 + iStep;
  Int iStep6 = iStep5 + iStep;
  Int iStep7 = iStep6 + iStep;

  for( k = 0; k < 64; k+=8 )
  {
    diff[k+0] = piOrg[0] - piCur[     0];
    diff[k+1] = piOrg[1] - piCur[iStep ];
    diff[k+2] = piOrg[2] - piCur[iStep2];
    diff[k+3] = piOrg[3] - piCur[iStep3];
    diff[k+4] = piOrg[4] - piCur[iStep4];
    diff[k+5] = piOrg[5] - piCur[iStep5];
    diff[k+6] = piOrg[6] - piCur[iStep6];
    diff[k+7] = piOrg[7] - piCur[iStep7];

    piCur += iStrideCur;
    piOrg += iStrideOrg;
  }

  //horizontal
  for (j=0; j < 8; j++)
  {
    jj = j << 3;
    m2[j][0] = diff[jj  ] + diff[jj+4];
    m2[j][1] = diff[jj+1] + diff[jj+5];
    m2[j][2] = diff[jj+2] + diff[jj+6];
    m2[j][3] = diff[jj+3] + diff[jj+7];
    m2[j][4] = diff[jj  ] - diff[jj+4];
    m2[j][5] = diff[jj+1] - diff[jj+5];
    m2[j][6] = diff[jj+2] - diff[jj+6];
    m2[j][7] = diff[jj+3] - diff[jj+7];

    m1[j][0] = m2[j][0] + m2[j][2];
    m1[j][1] = m2[j][1] + m2[j][3];
    m1[j][2] = m2[j][0] - m2[j][2];
    m1[j][3] = m2[j][1] - m2[j][3];
    m1[j][4] = m2[j][4] + m2[j][6];
    m1[j][5] = m2[j][5] + m2[j][7];
    m1[j][6] = m2[j][4] - m2[j][6];
    m1[j][7] = m2[j][5] - m2[j][7];

    m2[j][0] = m1[j][0] + m1[j][1];
    m2[j][1] = m1[j][0] - m1[j][1];
    m2[j][2] = m1[j][2] + m1[j][3];
    m2[j][3] = m1[j][2] - m1[j][3];
    m2[j][4] = m1[j][4] + m1[j][5];
    m2[j][5] = m1[j][4] - m1[j][5];
    m2[j][6] = m1[j][6] + m1[j][7];
    m2[j][7] = m1[j][6] - m1[j][7];
  }

  //vertical
  for (i=0; i < 8; i++)
  {
    m3[0][i] = m2[0][i] + m2[4][i];
    m3[1][i] = m2[1][i] + m2[5][i];
    m3[2][i] = m2[2][i] + m2[6][i];
    m3[3][i] = m2[3][i] + m2[7][i];
    m3[4][i] = m2[0][i] - m2[4][i];
    m3[5][i] = m2[1][i] - m2[5][i];
    m3[6][i] = m2[2][i] - m2[6][i];
    m3[7][i] = m2[3][i] - m2[7][i];

    m1[0][i] = m3[0][i] + m3[2][i];
    m1[1][i] = m3[1][i] + m3[3][i];
    m1[2][i] = m3[0][i] - m3[2][i];
    m1[3][i] = m3[1][i] - m3[3][i];
    m1[4][i] = m3[4][i] + m3[6][i];
    m1[5][i] = m3[5][i] + m3[7][i];
    m1[6][i] = m3[4][i] - m3[6][i];
    m1[7][i] = m3[5][i] - m3[7][i];

    m2[0][i] = m1[0][i] + m1[1][i];
    m2[1][i] = m1[0][i] - m1[1][i];
    m2[2][i] = m1[2][i] + m1[3][i];
    m2[3][i] = m1[2][i] - m1[3][i];
    m2[4][i] = m1[4][i] + m1[5][i];
    m2[5][i] = m1[4][i] - m1[5][i];
    m2[6][i] = m1[6][i] + m1[7][i];
    m2[7][i] = m1[6][i] - m1[7][i];
  }
  for (j=0; j < 8; j++)
    for (i=0; i < 8; i++)
      sad += (abs(m2[j][i]));

  sad=((sad+2)>>2);

  return sad;
}

UInt TComRdCost::xGetHADs4( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  y;
  Int  iOffsetOrg = iStrideOrg<<2;
  Int  iOffsetCur = iStrideCur<<2;

  UInt uiSum = 0;

  for ( y=0; y<iRows; y+= 4 )
  {
    uiSum += xCalcHADs4x4( piOrg, piCur, iStrideOrg, iStrideCur, iStep );
    piOrg += iOffsetOrg;
    piCur += iOffsetCur;
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetHADs8( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;
  Int  y;

  UInt uiSum = 0;

  if ( iRows == 4 )
  {
    uiSum += xCalcHADs4x4( piOrg+0, piCur        , iStrideOrg, iStrideCur, iStep );
    uiSum += xCalcHADs4x4( piOrg+4, piCur+4*iStep, iStrideOrg, iStrideCur, iStep );
  }
  else
  {
    Int  iOffsetOrg = iStrideOrg<<3;
    Int  iOffsetCur = iStrideCur<<3;
    for ( y=0; y<iRows; y+= 8 )
    {
      uiSum += xCalcHADs8x8( piOrg, piCur, iStrideOrg, iStrideCur, iStep );
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
    }
  }

  return ( uiSum >> g_uiBitIncrement );
}

UInt TComRdCost::xGetHADs( DistParam* pcDtParam )
{
  Pel* piOrg   = pcDtParam->pOrg;
  Pel* piCur   = pcDtParam->pCur;
  Int  iRows   = pcDtParam->iRows;
  Int  iCols   = pcDtParam->iCols;
  Int  iStrideCur = pcDtParam->iStrideCur;
  Int  iStrideOrg = pcDtParam->iStrideOrg;
  Int  iStep  = pcDtParam->iStep;

  Int  x, y;

  UInt uiSum = 0;

  if( ( iRows % 8 == 0) && (iCols % 8 == 0) )
  {
    Int  iOffsetOrg = iStrideOrg<<3;
    Int  iOffsetCur = iStrideCur<<3;
    for ( y=0; y<iRows; y+= 8 )
    {
      for ( x=0; x<iCols; x+= 8 )
      {
        uiSum += xCalcHADs8x8( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep );
      }
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
    }
  }
  else if( ( iRows % 4 == 0) && (iCols % 4 == 0) )
  {
    Int  iOffsetOrg = iStrideOrg<<2;
    Int  iOffsetCur = iStrideCur<<2;

    for ( y=0; y<iRows; y+= 4 )
    {
      for ( x=0; x<iCols; x+= 4 )
      {
        uiSum += xCalcHADs4x4( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep );
      }
      piOrg += iOffsetOrg;
      piCur += iOffsetCur;
    }
  }
  else
  {
    for ( y=0; y<iRows; y+=2 )
    {
      for ( x=0; x<iCols; x+=2 )
      {
        uiSum += xCalcHADs2x2( &piOrg[x], &piCur[x*iStep], iStrideOrg, iStrideCur, iStep );
      }
      piOrg += iStrideOrg;
      piCur += iStrideCur;
    }
  }

  return ( uiSum >> g_uiBitIncrement );
}

