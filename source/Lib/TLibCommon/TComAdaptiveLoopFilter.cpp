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

/** \file     TComAdaptiveLoopFilter.cpp
    \brief    adaptive loop filter class
*/

#include "TComAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#if HHI_ALF
// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComAdaptiveLoopFilter::TComAdaptiveLoopFilter()
{
  m_pcTempPicYuv  = NULL;
}

Void TComAdaptiveLoopFilter::create( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
  if ( !m_pcTempPicYuv )
  {
    m_pcTempPicYuv = new TComPicYuv;
    m_pcTempPicYuv->create( iPicWidth, iPicHeight, uiMaxCUWidth, uiMaxCUHeight, uiMaxCUDepth );
  }
}

Void TComAdaptiveLoopFilter::destroy()
{
  if ( m_pcTempPicYuv )
  {
    m_pcTempPicYuv->destroy();
    delete m_pcTempPicYuv;
  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// allocate / free / copy functions
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::allocALFParam(ALFParam* pAlfParam)
{
  pAlfParam->alf_flag = false;
  pAlfParam->pcQuadTree = NULL;
  pAlfParam->bSeparateQt = false;

  pAlfParam->acHorizontalAlfFilter   = new AlfFilter[ ALF_FILT_FOR_CHROMA +1 ];
  pAlfParam->acVerticalAlfFilter     = new AlfFilter[ ALF_FILT_FOR_CHROMA +1 ];

  ::memset(pAlfParam->acHorizontalAlfFilter,  0, sizeof(AlfFilter)*( ALF_FILT_FOR_CHROMA +1 ) );
  ::memset(pAlfParam->acVerticalAlfFilter  ,  0, sizeof(AlfFilter)*( ALF_FILT_FOR_CHROMA +1 ) );


  pAlfParam->acHorizontalAlfFilter[ 0 ].bIsHorizontal         = true;
  pAlfParam->acHorizontalAlfFilter[ 0 ].bIsVertical           = false;
  pAlfParam->acHorizontalAlfFilter[ 0 ].aiQuantFilterCoeffs   = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->acHorizontalAlfFilter[ 0 ].aiTapCoeffMapping     = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->acHorizontalAlfFilter[ 0 ].aiCoeffWeights        = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->acVerticalAlfFilter  [ 0 ].bIsHorizontal         = false;
  pAlfParam->acVerticalAlfFilter  [ 0 ].bIsVertical           = true;
  pAlfParam->acVerticalAlfFilter  [ 0 ].aiQuantFilterCoeffs   = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->acVerticalAlfFilter  [ 0 ].aiTapCoeffMapping     = new Int[ALF_MAX_NUM_COEF];
  pAlfParam->acVerticalAlfFilter  [ 0 ].aiCoeffWeights        = new Int[ALF_MAX_NUM_COEF];
  ::memset( pAlfParam->acHorizontalAlfFilter[ 0 ].aiQuantFilterCoeffs , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  ::memset( pAlfParam->acHorizontalAlfFilter[ 0 ].aiTapCoeffMapping   , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  ::memset( pAlfParam->acHorizontalAlfFilter[ 0 ].aiCoeffWeights      , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  ::memset( pAlfParam->acVerticalAlfFilter  [ 0 ].aiQuantFilterCoeffs , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  ::memset( pAlfParam->acVerticalAlfFilter  [ 0 ].aiTapCoeffMapping   , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  ::memset( pAlfParam->acVerticalAlfFilter  [ 0 ].aiCoeffWeights      , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );

  for( UInt uiIndx=1; uiIndx <= ALF_FILT_FOR_CHROMA ; uiIndx++ )
  {
    pAlfParam->acHorizontalAlfFilter[ uiIndx ].bIsHorizontal         = true;
    pAlfParam->acHorizontalAlfFilter[ uiIndx ].bIsVertical           = false;
    pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs   = new Int[ALF_MAX_NUM_COEF];
    pAlfParam->acVerticalAlfFilter  [ uiIndx ].bIsHorizontal         = false;
    pAlfParam->acVerticalAlfFilter  [ uiIndx ].bIsVertical           = true;
    pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiQuantFilterCoeffs   = new Int[ALF_MAX_NUM_COEF];
    pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiTapCoeffMapping     = new Int[ALF_MAX_NUM_COEF];
    pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiCoeffWeights        = new Int[ALF_MAX_NUM_COEF];
    pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiTapCoeffMapping     = new Int[ALF_MAX_NUM_COEF];
    pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiCoeffWeights        = new Int[ALF_MAX_NUM_COEF];
    ::memset( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
    ::memset( pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiQuantFilterCoeffs , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
    ::memset( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiTapCoeffMapping   , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
    ::memset( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiCoeffWeights      , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
    ::memset( pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiTapCoeffMapping   , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
    ::memset( pAlfParam->acVerticalAlfFilter  [ uiIndx ].aiCoeffWeights      , 0 , sizeof(Int) * ALF_MAX_NUM_COEF );
  }

}


Void TComAdaptiveLoopFilter::freeALFParam(ALFParam* pAlfParam)
{
  assert(pAlfParam != NULL);

  if( pAlfParam->acHorizontalAlfFilter != NULL )
  {
    for( UInt uiIndx=0; uiIndx <=  ALF_FILT_FOR_CHROMA  ; uiIndx++ )
    {
      if( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs != NULL )
      {
        delete[] pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs;
        pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs = NULL;
      }
      if( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiTapCoeffMapping != NULL )
      {
        delete[] pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiTapCoeffMapping;
        pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiTapCoeffMapping = NULL;
      }
      if( pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiCoeffWeights != NULL )
      {
        delete[] pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiCoeffWeights;
        pAlfParam->acHorizontalAlfFilter[ uiIndx ].aiCoeffWeights = NULL;
      }
    }
    delete[] pAlfParam->acHorizontalAlfFilter;
    pAlfParam->acHorizontalAlfFilter = NULL;
  }


  if( pAlfParam->acVerticalAlfFilter != NULL )
    {
      for( UInt uiIndx=0; uiIndx <=  ALF_FILT_FOR_CHROMA  ; uiIndx++ )
      {
        if( pAlfParam->acVerticalAlfFilter[ uiIndx ].aiQuantFilterCoeffs != NULL )
        {
          delete[] pAlfParam->acVerticalAlfFilter[ uiIndx ].aiQuantFilterCoeffs;
          pAlfParam->acVerticalAlfFilter[ uiIndx ].aiQuantFilterCoeffs = NULL;
        }
        if( pAlfParam->acVerticalAlfFilter[ uiIndx ].aiTapCoeffMapping != NULL )
        {
          delete[] pAlfParam->acVerticalAlfFilter[ uiIndx ].aiTapCoeffMapping;
          pAlfParam->acVerticalAlfFilter[ uiIndx ].aiTapCoeffMapping = NULL;
        }
        if( pAlfParam->acVerticalAlfFilter[ uiIndx ].aiCoeffWeights != NULL )
        {
          delete[] pAlfParam->acVerticalAlfFilter[ uiIndx ].aiCoeffWeights;
          pAlfParam->acVerticalAlfFilter[ uiIndx ].aiCoeffWeights = NULL;
        }
      }
      delete[] pAlfParam->acVerticalAlfFilter;
      pAlfParam->acVerticalAlfFilter = NULL;
    }

  pAlfParam->pcQuadTree = NULL;

}

Void TComAdaptiveLoopFilter::destroyQuadTree(ALFParam* pcAlfParam)
{
  pcAlfParam->pcQuadTree->destroy();
  delete pcAlfParam->pcQuadTree;
  pcAlfParam->pcQuadTree = NULL;
}

Void TComAdaptiveLoopFilter::copyALFParam(ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam)
{
  pDesAlfParam->alf_flag        = pSrcAlfParam->alf_flag;
  pDesAlfParam->cu_control_flag = pSrcAlfParam->cu_control_flag;
  pDesAlfParam->pcQuadTree      = pSrcAlfParam->pcQuadTree;
  pDesAlfParam->bSeparateQt     = pSrcAlfParam->bSeparateQt;
  pDesAlfParam->chroma_idc      = pSrcAlfParam->chroma_idc;

  if( pSrcAlfParam->acVerticalAlfFilter != NULL )
  {
    for( UInt uiIndx=0; uiIndx <=  ALF_FILT_FOR_CHROMA  ; uiIndx++ )
    {
      xFillAlfFilterInitParam( pDesAlfParam->acVerticalAlfFilter[ uiIndx ] , pSrcAlfParam->acVerticalAlfFilter[ uiIndx ].iFilterLength, pSrcAlfParam->acVerticalAlfFilter[ uiIndx ].iFilterSymmetry );
      ::memcpy( pDesAlfParam->acVerticalAlfFilter[ uiIndx ].aiQuantFilterCoeffs, pSrcAlfParam->acVerticalAlfFilter[ uiIndx ].aiQuantFilterCoeffs, sizeof(Int)*ALF_MAX_NUM_COEF );
      pDesAlfParam->acVerticalAlfFilter[ uiIndx ].bIsValid =  pSrcAlfParam->acVerticalAlfFilter[ uiIndx ].bIsValid;
    }
  }


  if( pSrcAlfParam->acHorizontalAlfFilter != NULL )
  {
    for( UInt uiIndx=0; uiIndx <=  ALF_FILT_FOR_CHROMA  ; uiIndx++ )
    {
      xFillAlfFilterInitParam( pDesAlfParam->acHorizontalAlfFilter[ uiIndx ] , pSrcAlfParam->acHorizontalAlfFilter[ uiIndx ].iFilterLength, pSrcAlfParam->acHorizontalAlfFilter[ uiIndx ].iFilterSymmetry );
      ::memcpy( pDesAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs, pSrcAlfParam->acHorizontalAlfFilter[ uiIndx ].aiQuantFilterCoeffs, sizeof(Int)*ALF_MAX_NUM_COEF );
      pDesAlfParam->acHorizontalAlfFilter[ uiIndx ].bIsValid =  pSrcAlfParam->acHorizontalAlfFilter[ uiIndx ].bIsValid;
    }
  }

  for(Int i = 0; i<3; i++)
  {
    pDesAlfParam->aiPlaneFilterMapping[i] = pSrcAlfParam->aiPlaneFilterMapping[i] ;
  }
}

Void TComAdaptiveLoopFilter::copyALFFilter(AlfFilter& rDesAlfFilter, AlfFilter& rSrcAlfFilter)
{
  xFillAlfFilterInitParam( rDesAlfFilter , rSrcAlfFilter.iFilterLength, rSrcAlfFilter.iFilterSymmetry );
  ::memcpy( rDesAlfFilter.aiQuantFilterCoeffs, rSrcAlfFilter.aiQuantFilterCoeffs, sizeof(Int)*ALF_MAX_NUM_COEF );
  rDesAlfFilter.bIsValid =  rSrcAlfFilter.bIsValid;
}

// --------------------------------------------------------------------------------------------------------------------
// prediction of filter coefficients
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xPredictALFCoeff( ALFParam* pAlfParam)
{
  Int iCenterPos = 0;
  Int iLength    = 0;
  for(Int i=0; i<ALF_FILT_FOR_CHROMA+1; i++)
  {
    if ( ( pAlfParam->chroma_idc & i) || ( i == 0 ) )
    {
      //horizontal
      iLength = pAlfParam->acHorizontalAlfFilter[i].iFilterLength;
      iCenterPos = iLength >> 1;
    if(pAlfParam->acHorizontalAlfFilter[i].iFilterSymmetry == 0)
      {
        for(Int j=0 ; j < iCenterPos; j++ )
                pAlfParam->acHorizontalAlfFilter[i].aiQuantFilterCoeffs[ iLength-j-1 ] += pAlfParam->acHorizontalAlfFilter[i].aiQuantFilterCoeffs[j];
      }

      Int iSum = 0 ;
      for (Int j = 0; j< pAlfParam->acHorizontalAlfFilter[i].iFilterLength; j++)
      {
        if(j!=iCenterPos)
          iSum += pAlfParam->acHorizontalAlfFilter[i].aiQuantFilterCoeffs[ pAlfParam->acHorizontalAlfFilter[i].aiTapCoeffMapping[j] ] ;
      }

      pAlfParam->acHorizontalAlfFilter[i].aiQuantFilterCoeffs[iCenterPos]+= ((1<<ALF_NUM_BIT_SHIFT) - iSum) ;

      ///
      iLength = pAlfParam->acVerticalAlfFilter[i].iFilterLength;
      iCenterPos = iLength >> 1;
    if(pAlfParam->acVerticalAlfFilter[i].iFilterSymmetry == 0)
      {
        for(Int j=0 ; j < iCenterPos; j++ )
          pAlfParam->acVerticalAlfFilter[i].aiQuantFilterCoeffs[ iLength-j-1 ] += pAlfParam->acVerticalAlfFilter[i].aiQuantFilterCoeffs[j];
      }
      iSum = 0 ;
      for (Int j = 0; j< pAlfParam->acVerticalAlfFilter[i].iFilterLength; j++)
      {
        if(j!=iCenterPos)
          iSum += pAlfParam->acVerticalAlfFilter[i].aiQuantFilterCoeffs[ pAlfParam->acVerticalAlfFilter[i].aiTapCoeffMapping[j] ] ;
      }
      pAlfParam->acVerticalAlfFilter[i].aiQuantFilterCoeffs[iCenterPos]+= ((1<<ALF_NUM_BIT_SHIFT) - iSum) ;
    }
  }
}


// --------------------------------------------------------------------------------------------------------------------
// interface function for actual ALF process
// --------------------------------------------------------------------------------------------------------------------

/**
    \param  pcPic         picture (TComPic) class (input/output)
    \param  pcAlfParam    ALF parameter
 */
Void TComAdaptiveLoopFilter::ALFProcess(  TComPic* pcPic , ALFParam* pcAlfParam )
{
  if(!pcAlfParam->alf_flag)
  {
    return;
  }
  
  xFillAlfFilterInitParam(pcAlfParam->acHorizontalAlfFilter[0], pcAlfParam->acHorizontalAlfFilter[0].iFilterLength, pcAlfParam->acHorizontalAlfFilter[0].iFilterSymmetry) ;
  xFillAlfFilterInitParam(pcAlfParam->acVerticalAlfFilter[0], pcAlfParam->acVerticalAlfFilter[0].iFilterLength, pcAlfParam->acVerticalAlfFilter[0].iFilterSymmetry) ;

  for (Int iPlane = 1; iPlane < ALF_FILT_FOR_CHROMA + 1; iPlane++)
  {
    if ( pcAlfParam->chroma_idc & iPlane)
    {
      xFillAlfFilterInitParam( pcAlfParam->acHorizontalAlfFilter[iPlane], pcAlfParam->acHorizontalAlfFilter[iPlane].iFilterLength, pcAlfParam->acHorizontalAlfFilter[iPlane].iFilterSymmetry ) ;
      xFillAlfFilterInitParam( pcAlfParam->acVerticalAlfFilter  [iPlane], pcAlfParam->acVerticalAlfFilter  [iPlane].iFilterLength, pcAlfParam->acVerticalAlfFilter  [iPlane].iFilterSymmetry ) ;
      if( pcAlfParam->aiPlaneFilterMapping[iPlane] != iPlane)
      {
        copyALFFilter(pcAlfParam->acHorizontalAlfFilter[iPlane],pcAlfParam->acHorizontalAlfFilter[pcAlfParam->aiPlaneFilterMapping[iPlane]]);
        copyALFFilter(pcAlfParam->acVerticalAlfFilter[iPlane],pcAlfParam->acVerticalAlfFilter[pcAlfParam->aiPlaneFilterMapping[iPlane]]);
      }
    }
  }
  xPredictALFCoeff(pcAlfParam);

  TComPicYuv* pcPicYuvRec    = pcPic->getPicYuvRec();
  xALFFilter( pcPic , pcAlfParam, pcPicYuvRec, 0 );

  for (Int iPlane = 1; iPlane < ALF_FILT_FOR_CHROMA + 1 ; iPlane++)
  {
    if ( pcAlfParam->chroma_idc & iPlane )
    {
      xALFFilter( pcPic , pcAlfParam, pcPicYuvRec, iPlane );
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================


Void TComAdaptiveLoopFilter::xFillAlfFilterInitParam( AlfFilter& rcFilter, Int iLength, Int iSymmetry )
{
  rcFilter.iFilterLength   = iLength;
  rcFilter.iFilterSymmetry = iSymmetry;
  rcFilter.iNumOfCoeffs    = iSymmetry ? ( iLength >> 1 ) + 1 : iLength;
  rcFilter.iOverlap        = iLength >> 1;
  if( iSymmetry )
  {
    for( Int i = 0; i < rcFilter.iNumOfCoeffs; i++ )
    {
      rcFilter.aiTapCoeffMapping[ i ]               = i;
      rcFilter.aiTapCoeffMapping[ iLength - i  -1 ] = i;
      rcFilter.aiCoeffWeights   [ i ]               = 2;
    }
    rcFilter.aiCoeffWeights[ rcFilter.iNumOfCoeffs - 1 ] = 1;
  }
  else
  {
    for( Int i = 0; i < iLength ; i++ )
    {
      rcFilter.aiTapCoeffMapping[i] = i;
      rcFilter.aiCoeffWeights   [i] = 1;
    }
  }
  if( rcFilter.bIsHorizontal )
  {
    rcFilter.iHorizontalOverlap = rcFilter.iOverlap;
    rcFilter.iVerticalOverlap   = 0;
  }
  else if( rcFilter.bIsVertical )
  {
    rcFilter.iHorizontalOverlap = 0;
    rcFilter.iVerticalOverlap   = rcFilter.iOverlap;
  }

  rcFilter.bIsValid = false;
}



// --------------------------------------------------------------------------------------------------------------------
// ALF for luma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFFilter( TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicRest, Int iPlane)
{
  if(pcAlfParam->cu_control_flag && iPlane ==0 )
  {
    // block-adaptive ALF process
    TComPicYuv* pcTempPicYuv2 = new TComPicYuv;
    pcTempPicYuv2->create( pcPicRest->getWidth(), pcPicRest->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
    pcPicRest->copyToPicLuma( pcTempPicYuv2 );
    xApplyFrame      ( pcPicRest, m_pcTempPicYuv , pcAlfParam->acVerticalAlfFilter  [0] );
    xApplyFrame      ( m_pcTempPicYuv , pcPicRest, pcAlfParam->acHorizontalAlfFilter[0] );
    //xCUAdaptive      ( m_pcAlfQuadTree, pcAlfParam->acHorizontalAlfFilter[0] , m_pcTempPicYuv, pcPicRest );
    xCopyDecToRestCUs( pcAlfParam->pcQuadTree ,  pcTempPicYuv2 , pcPicRest );
    pcTempPicYuv2->destroy();
    delete pcTempPicYuv2;
  }
  else
  {
    // non-adaptive ALF process
    xApplyFrame( pcPicRest, m_pcTempPicYuv , pcAlfParam->acVerticalAlfFilter  [iPlane], iPlane );
    xApplyFrame( m_pcTempPicYuv, pcPicRest , pcAlfParam->acHorizontalAlfFilter[iPlane], iPlane );
  }
}

Void TComAdaptiveLoopFilter::xCUAdaptive(TComPicSym* pcQuadTree, AlfFilter& rcFilter , TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  // for every CU, call CU-adaptive ALF process
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    xSubCUAdaptive(pcQuadTree, pcCU, rcFilter, pcPicDec, pcPicRest, 0, 0);
  }
}

/**
    - For every sub-CU's, it is called recursively
    .
    \param  pcCU          CU data structure
    \param  pcAlfParam    ALF parameter
    \param  pcPicDec      picture before ALF
    \retval pcPicRest     picture after  ALF
    \param  uiAbsPartIdx  current sub-CU position
    \param  uiDepth       current sub-CU depth
 */
Void TComAdaptiveLoopFilter::xSubCUAdaptive(TComPicSym* pcQuadTree, TComDataCU* pcCU, AlfFilter& rcFilter, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt uiAbsPartIdx, UInt uiDepth)
{
  Bool bBoundary = false;
  Int iLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  Int iRPelX   = iLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  Int iTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  Int iBPelY   = iTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  // check picture boundary
  if ( ( iRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( iBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  // go to sub-CU?
  if ( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      iLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      iTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( iLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( iTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSubCUAdaptive(pcQuadTree, pcCU, rcFilter, pcPicDec, pcPicRest, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }

  // check CU-based flag
  if ( pcCU->getAlfCtrlFlag(uiAbsPartIdx) )
  {
    const Int iOffsetX = iLPelX - Max( 0 , iLPelX - rcFilter.iHorizontalOverlap );
    const Int iOffsetY = iTPelY - Max( 0 , iTPelY - rcFilter.iVerticalOverlap   );
    const Int iMaxX    = pcCU->getSlice()->getSPS()->getWidth()  - iLPelX + iOffsetX;
    const Int iMaxY    = pcCU->getSlice()->getSPS()->getHeight() - iTPelY + iOffsetY;

    Pel* pDec = pcPicDec->getLumaAddr( pcCU->getAddr(), uiAbsPartIdx );
    pDec -= iOffsetX + pcPicDec->getStride() * iOffsetY;


    xApplyFilter( pDec,
                  pcPicRest->getLumaAddr(pcCU->getAddr(), uiAbsPartIdx),
                  pcCU->getWidth(uiAbsPartIdx),
                  pcCU->getHeight(uiAbsPartIdx),
                  pcPicDec->getStride(),
                  pcPicRest->getStride(),
                  rcFilter,
                  iOffsetX,
                  iOffsetY,
                  iMaxX,
                  iMaxY
                  );

  }
}



Void TComAdaptiveLoopFilter::xApplyFilter  ( Pel* pDec, Pel* pRest, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride, AlfFilter& rcFilter )
{
  xApplyFilter(pDec, pRest, iWidth, iHeight, iDecStride, iRestStride, rcFilter, 0, 0, iWidth, iHeight );
}


Void TComAdaptiveLoopFilter::xApplyFilter  ( Pel* pDec, Pel* pRest, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride, AlfFilter& rcFilter,Int iOffsetX, Int iOffsetY, Int iMaxX, Int iMaxY )
{
  Int iAccu = 0;

  const Int iFilterLength  = rcFilter.iFilterLength;
  const Int iFilterOverlap = iFilterLength >> 1;
//  const Int iBitShift      = g_uiBitDepth + g_uiBitIncrement - 8;
  const Int iBitShift      = ALF_NUM_BIT_SHIFT ;
  Int iDcOffset      = 0;

  const Pel *pcSourceData = pDec;
        Pel *pcDestData   = pRest;

  const Int iSourceStride      = iDecStride ;
  const Int iDestStride        = iRestStride ;

  const Int iAdd = 1 << (iBitShift - 1 );

  const Int iSizeX    = iWidth;
  const Int iSizeY    = iHeight;

  Int piCoeffs[ALF_MAX_NUM_TAP];
  if( rcFilter.iFilterSymmetry == 0 )
  {
#if ALF_DC_CONSIDERED
    iDcOffset = rcFilter.aiQuantFilterCoeffs[ rcFilter.iNumOfCoeffs ];
#else
    iDcOffset = 0;
#endif
    for(Int i = 0; i < iFilterLength; i++)
    {
      piCoeffs[i] = rcFilter.aiQuantFilterCoeffs[i];
    }
  }
  else
  {
#if ALF_DC_CONSIDERED
    iDcOffset = rcFilter.aiQuantFilterCoeffs[ rcFilter.iNumOfCoeffs ];
#else
    iDcOffset = 0;
#endif
    for(Int i = 0; i <= iFilterOverlap; i++)
    {
      piCoeffs[i] = rcFilter.aiQuantFilterCoeffs[i];
      piCoeffs[ iFilterLength -1 -i ] = rcFilter.aiQuantFilterCoeffs[i];
    }

  }



  if (rcFilter.bIsHorizontal )
  {
  Int iStartX = Max( iOffsetX , iFilterOverlap       );
  Int iStopX  = Min( iMaxX - iFilterOverlap , iSizeX + iOffsetX);

  for (Int y = 0; y < iSizeY; y++)
  {
   for (Int x = iOffsetX ; x < iFilterOverlap ; x++)  // x - uiFilterOverlap + i < 0
   {
     iAccu = 0 ;
     for (Int i = 0; i < iFilterOverlap-x; i++)
     {
       iAccu += pcSourceData[ iSourceStride * y  ] * piCoeffs[i];
     }
     for (Int i = iFilterOverlap-x; i < iFilterLength; i++)
     {
       iAccu += pcSourceData[ x - iFilterOverlap + i  + iSourceStride * y  ] * piCoeffs[i];
     }
     pcDestData[x - iOffsetX + iDestStride * y ] =  (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift)  ;
   }

   for (Int x = iStartX ; x < iStopX ; x++)  // 0 < x - uiFilterOverlap + i < uiSizeX
   {
     iAccu = 0 ;
     for (Int i = 0; i < iFilterLength; i++)
     {
       iAccu += pcSourceData[ int(x - iFilterOverlap + i ) + iSourceStride * y  ] * piCoeffs[i];
     }
     pcDestData[x -iOffsetX + iDestStride * y ] =  (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift) ;
   }

   for (Int x = iStopX ; x < iSizeX + iOffsetX ; x++)   // uiSizeX < x - uiFilterOverlap + i
   {
     iAccu = 0 ;
     for (Int i = 0; i < iSizeX + iOffsetX + iFilterOverlap - x; i++)
     {
       iAccu += pcSourceData[ x - iFilterOverlap + i + iSourceStride * y  ] * piCoeffs[i];
     }
     for (Int i  = iSizeX + iOffsetX + iFilterOverlap-x; i < iFilterLength; i++)
     {
       iAccu += pcSourceData[ iSizeX + iOffsetX - 1 + iSourceStride * y  ] * piCoeffs[i];
     }
     pcDestData[x -iOffsetX + iDestStride * y ] =  (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift) ;
   }
  }

  }
  else if( rcFilter.bIsVertical )
  {


  Int iStartY = Max( iOffsetY , iFilterOverlap       );
  Int iStopY  = Min( iMaxY - iFilterOverlap , iSizeY + iOffsetY);

  for (Int y = iOffsetY; y < iFilterOverlap ; y++)
  {
   for (Int x = 0; x < iSizeX; x++)
   {
     iAccu = 0 ;
     for (Int i = 0; i < iFilterOverlap-y; i++)
     {
         iAccu += pcSourceData[ x ] * piCoeffs[i];
     }

     for (Int i = iFilterOverlap-y; i < iFilterLength; i++)
     {
         iAccu += pcSourceData[  x + iSourceStride * (y - iFilterOverlap + i ) ] * piCoeffs[i];
     }
     pcDestData[x + iDestStride * ( y - iOffsetY ) ] =  (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift) ;
   }
  }
  for (Int y = iStartY ; y < iStopY ; y++)
  {
   for (Int x = 0; x < iSizeX; x++)
   {
     iAccu = 0 ;
     for (Int i = 0; i < iFilterLength; i++)
     {
         iAccu += pcSourceData[  x + iSourceStride *  int(y - iFilterOverlap + i ) ] * piCoeffs[i];
     }
     pcDestData[x + iDestStride * ( y - iOffsetY ) ] = (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift) ;
   }
  }
  for (Int y = iStopY  ; y < iSizeY + iOffsetY ; y++)
  {
   for (Int x = 0; x < iSizeX; x++)
   {
     iAccu = 0 ;
     for (Int i = 0; i < iSizeY+ iOffsetY + iFilterOverlap-y; i++)
     {
         iAccu += pcSourceData[  x + iSourceStride * (y - iFilterOverlap + i ) ] * piCoeffs[i];
     }

     for (Int i = iSizeY+ iOffsetY + iFilterOverlap-y; i < iFilterLength; i++)
     {
         iAccu += pcSourceData[  x + iSourceStride * ( iHeight + iOffsetY - 1 ) ] * piCoeffs[i];
     }
     pcDestData[x + iDestStride * ( y - iOffsetY ) ] =  (Pel)Clip((iAccu  + iDcOffset + iAdd)>>iBitShift) ;
   }
  }
  }

}

Void TComAdaptiveLoopFilter::xApplyFrame(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, AlfFilter& rcFilter, Int iPlane )
{
  Int iHeight = 0;
  Int iWidth  = 0;

  Pel* pDec = NULL;
  Int iDecStride = 0;

  Pel* pRest = NULL;
  Int iRestStride = 0;

  if (iPlane == 0)
  {
    iHeight     = pcPicRest->getHeight();
    iWidth      = pcPicRest->getWidth();

    pDec        = pcPicDec->getLumaAddr();
    iDecStride  = pcPicDec->getStride();

    pRest       = pcPicRest->getLumaAddr();
    iRestStride = pcPicRest->getStride();
  }
  else if (iPlane ==1)
  {
    iHeight     = pcPicRest->getHeight() >> 1;
    iWidth      = pcPicRest->getWidth()  >> 1;

    pDec        = pcPicDec->getCbAddr();
    iDecStride  = pcPicDec->getCStride();

    pRest       = pcPicRest->getCbAddr();
    iRestStride = pcPicRest->getCStride();
  }
  else if (iPlane ==2)
  {
    iHeight     = pcPicRest->getHeight() >> 1;
    iWidth      = pcPicRest->getWidth()  >> 1;

    pDec        = pcPicDec->getCrAddr();
    iDecStride  = pcPicDec->getCStride();

    pRest       = pcPicRest->getCrAddr();
    iRestStride = pcPicRest->getCStride();
  }
  xApplyFilter( pDec, pRest, iWidth, iHeight, iDecStride, iRestStride, rcFilter );

}


Void TComAdaptiveLoopFilter::xApplyFrame(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, AlfFilter& rcFilter )
{

  Int iHeight = pcPicRest->getHeight();
  Int iWidth = pcPicRest->getWidth();

  Pel* pDec = pcPicDec->getLumaAddr();
  Int iDecStride = pcPicDec->getStride();

  Pel* pRest = pcPicRest->getLumaAddr();
  Int iRestStride = pcPicRest->getStride();

  xApplyFilter( pDec, pRest, iWidth, iHeight, iDecStride, iRestStride, rcFilter );




}




Void TComAdaptiveLoopFilter::xCopyDecToRestCUs(TComPicSym* pcQuadTree, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  for( UInt uiCUAddr = 0; uiCUAddr < pcQuadTree->getNumberOfCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcQuadTree->getCU( uiCUAddr );
    xCopyDecToRestCU( pcQuadTree, pcCU, 0, 0, pcPicDec, pcPicRest);
  }
}

Void TComAdaptiveLoopFilter::xCopyDecToRestCU(TComPicSym* pcQuadTree, TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcQuadTree->getNumPartition() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xCopyDecToRestCU(pcQuadTree, pcCU, uiAbsPartIdx, uiDepth+1, pcPicDec, pcPicRest);
    }
    return;
  }

  if (!pcCU->getAlfCtrlFlag(uiAbsPartIdx))
  {
    UInt uiCUAddr = pcCU->getAddr();

    Int iWidth  = g_uiMaxCUWidth  >> uiDepth;
    Int iHeight = g_uiMaxCUHeight >> uiDepth;

    Pel* pRec = pcPicDec->getLumaAddr(uiCUAddr, uiAbsPartIdx);
    Pel* pFilt = pcPicRest->getLumaAddr(uiCUAddr, uiAbsPartIdx);

    Int iRecStride = pcPicDec->getStride();
    Int iFiltStride = pcPicRest->getStride();

    for (Int y = 0; y < iHeight; y++)
    {
      for (Int x = 0; x < iWidth; x++)
      {
        pFilt[x] = pRec[x];
      }
      pRec += iRecStride;
      pFilt += iFiltStride;
    }
  }
}


// --------------------------------------------------------------------------------------------------------------------
// ALF for chroma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFChroma(ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if((pcAlfParam->chroma_idc>>1)&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam, 0);
  }

  if(pcAlfParam->chroma_idc&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam , 1);
  }
}

/** \param  pcPicDec      picture before ALF
    \param  pcPicRest     picture after  ALF
    \param  pcAlfParam    AlfParameter
    \param  iColor        0 for Cb and 1 for Cr
 */
Void TComAdaptiveLoopFilter::xFrameChroma( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, ALFParam* pcAlfParam , Int iColor )
{
  Int iHeight = pcPicRest->getHeight() >> 1;
  Int iWidth = pcPicRest->getWidth() >> 1;

  Pel* pDec;
  Int iDecStride = pcPicDec->getCStride();

  Pel* pRest;
  Int iRestStride = pcPicRest->getCStride();

  AlfFilter* pcAlfFilterHorizontal;
  AlfFilter* pcAlfFilterVertical;

  if (iColor)
  {
    pDec  = pcPicDec->getCrAddr();
    pRest = pcPicRest->getCrAddr();
    pcAlfFilterHorizontal = &pcAlfParam->acHorizontalAlfFilter[ 1 ];
    pcAlfFilterVertical   = &pcAlfParam->acVerticalAlfFilter[ 1 ];
  }
  else
  {
    pDec  = pcPicDec->getCbAddr();
    pRest = pcPicRest->getCbAddr();
    pcAlfFilterHorizontal = &pcAlfParam->acHorizontalAlfFilter[ ALF_FILT_FOR_CHROMA ];
    pcAlfFilterVertical   = &pcAlfParam->acVerticalAlfFilter  [ ALF_FILT_FOR_CHROMA ];
  }

  xApplyFilter( pDec, pRest, iWidth, iHeight, iDecStride, iRestStride, *pcAlfFilterVertical   );
  xApplyFilter( pDec, pRest, iWidth, iHeight, iDecStride, iRestStride, *pcAlfFilterHorizontal );
}
#else
// ====================================================================================================================
// Tables
// ====================================================================================================================

// scaling factor for quantization of filter coefficients (9x9)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag9x9[41] =
{
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (7x7)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag7x7[25] =
{
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 1
};

// scaling factor for quantization of filter coefficients (5x5)
const Int TComAdaptiveLoopFilter::m_aiSymmetricMag5x5[13] =
{
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 1
};

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComAdaptiveLoopFilter::TComAdaptiveLoopFilter()
{
	m_pcTempPicYuv = NULL;
}

Void TComAdaptiveLoopFilter::create( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth )
{
	if ( !m_pcTempPicYuv )
	{
		m_pcTempPicYuv = new TComPicYuv;
		m_pcTempPicYuv->create( iPicWidth, iPicHeight, uiMaxCUWidth, uiMaxCUHeight, uiMaxCUDepth );
	}
}

Void TComAdaptiveLoopFilter::destroy()
{
	if ( m_pcTempPicYuv )
	{
		m_pcTempPicYuv->destroy();
		delete m_pcTempPicYuv;
	}
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// allocate / free / copy functions
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::allocALFParam(ALFParam* pAlfParam)
{
  pAlfParam->alf_flag = 0;

  pAlfParam->coeff				= new Int[ALF_MAX_NUM_COEF];
  pAlfParam->coeff_chroma = new Int[ALF_MAX_NUM_COEF_C];

  ::memset(pAlfParam->coeff,				0, sizeof(Int)*ALF_MAX_NUM_COEF		);
  ::memset(pAlfParam->coeff_chroma, 0, sizeof(Int)*ALF_MAX_NUM_COEF_C	);
}

Void TComAdaptiveLoopFilter::freeALFParam(ALFParam* pAlfParam)
{
  assert(pAlfParam != NULL);

  if (pAlfParam->coeff != NULL)
  {
    delete[] pAlfParam->coeff;
    pAlfParam->coeff = NULL;
  }

  if (pAlfParam->coeff_chroma != NULL)
  {
    delete[] pAlfParam->coeff_chroma;
    pAlfParam->coeff_chroma = NULL;
  }
}

Void TComAdaptiveLoopFilter::copyALFParam(ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam)
{
  pDesAlfParam->alf_flag = pSrcAlfParam->alf_flag;
  pDesAlfParam->cu_control_flag = pSrcAlfParam->cu_control_flag;
  pDesAlfParam->chroma_idc = pSrcAlfParam->chroma_idc;
  pDesAlfParam->tap = pSrcAlfParam->tap;
  pDesAlfParam->num_coeff = pSrcAlfParam->num_coeff;
  pDesAlfParam->tap_chroma = pSrcAlfParam->tap_chroma;
  pDesAlfParam->num_coeff_chroma = pSrcAlfParam->num_coeff_chroma;

  ::memcpy(pDesAlfParam->coeff, pSrcAlfParam->coeff, sizeof(Int)*ALF_MAX_NUM_COEF);
  ::memcpy(pDesAlfParam->coeff_chroma, pSrcAlfParam->coeff_chroma, sizeof(Int)*ALF_MAX_NUM_COEF_C);
}

// --------------------------------------------------------------------------------------------------------------------
// prediction of filter coefficients
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::predictALFCoeff( ALFParam* pAlfParam)
{
  Int i, sum, pred, tap, N;
  const Int* pFiltMag = NULL;

  tap = pAlfParam->tap;

  switch(tap)
  {
  case 5:
    pFiltMag = m_aiSymmetricMag5x5;
    break;
  case 7:
    pFiltMag = m_aiSymmetricMag7x7;
    break;
  case 9:
    pFiltMag = m_aiSymmetricMag9x9;
    break;
  default:
    assert(0);
    break;
  }
  N = (tap*tap+1)>>1;
  sum=0;
  for(i=0; i<N-1;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-sum;
  pAlfParam->coeff[N-1]=pred-pAlfParam->coeff[N-1];
}

Void TComAdaptiveLoopFilter::predictALFCoeffChroma( ALFParam* pAlfParam )
{
  Int i, sum, pred, tap, N;
  const Int* pFiltMag = NULL;

  tap = pAlfParam->tap_chroma;
  switch(tap)
  {
  case 5:
    pFiltMag = m_aiSymmetricMag5x5;
    break;
  case 7:
    pFiltMag = m_aiSymmetricMag7x7;
    break;
  case 9:
    pFiltMag = m_aiSymmetricMag9x9;
    break;
  default:
    assert(0);
    break;
  }
  N = (tap*tap+1)>>1;
  sum=0;
  for(i=0; i<N;i++)
  {
    sum+=pFiltMag[i]*pAlfParam->coeff_chroma[i];
  }
  pred=(1<<ALF_NUM_BIT_SHIFT)-(sum-pAlfParam->coeff_chroma[N-1]);
  pAlfParam->coeff_chroma[N-1]=pred-pAlfParam->coeff_chroma[N-1];
}

// --------------------------------------------------------------------------------------------------------------------
// interface function for actual ALF process
// --------------------------------------------------------------------------------------------------------------------

/**
		\param	pcPic					picture (TComPic) class (input/output)
		\param	pcAlfParam		ALF parameter
		\todo   for temporal buffer, it uses residual picture buffer, which may not be safe. Make it be safer.
 */
Void TComAdaptiveLoopFilter::ALFProcess(TComPic* pcPic, ALFParam* pcAlfParam)
{
  if(!pcAlfParam->alf_flag)
  {
    return;
  }

  TComPicYuv* pcPicYuvRec    = pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;

  pcPicYuvRec		->copyToPic						( pcPicYuvExtRec );
	pcPicYuvExtRec->setBorderExtension	( false );
  pcPicYuvExtRec->extendPicBorder			();

	predictALFCoeff(pcAlfParam);
  xALFLuma(pcPic, pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);

  if(pcAlfParam->chroma_idc)
  {
		predictALFCoeffChroma(pcAlfParam);
		xALFChroma( pcAlfParam, pcPicYuvExtRec, pcPicYuvRec);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// ALF for luma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFLuma(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if(pcAlfParam->cu_control_flag)
  {
		// block-adaptive ALF process
    xCUAdaptive(pcPic, pcAlfParam, pcPicDec, pcPicRest);
  }
  else
  {
		// non-adaptive ALF process
    xFrame(pcPicDec, pcPicRest, pcAlfParam->coeff, pcAlfParam->tap);
  }
}

Void TComAdaptiveLoopFilter::xCUAdaptive(TComPic* pcPic, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
	// for every CU, call CU-adaptive ALF process
  for( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    xSubCUAdaptive(pcCU, pcAlfParam, pcPicDec, pcPicRest, 0, 0);
  }
}

/**
		- For every sub-CU's, it is called recursively
		.
		\param	pcCU					CU data structure
		\param	pcAlfParam		ALF parameter
		\param  pcPicDec			picture before ALF
		\retval	pcPicRest			picture after  ALF
		\param  uiAbsPartIdx	current sub-CU position
		\param	uiDepth				current sub-CU depth
 */
Void TComAdaptiveLoopFilter::xSubCUAdaptive(TComDataCU* pcCU, ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt uiAbsPartIdx, UInt uiDepth)
{
  TComPic* pcPic = pcCU->getPic();

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

	// check picture boundary
  if ( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }

	// go to sub-CU?
  if ( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSubCUAdaptive(pcCU, pcAlfParam, pcPicDec, pcPicRest, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }

	// check CU-based flag
  if ( pcCU->getAlfCtrlFlag(uiAbsPartIdx) )
  {
    xSubCUFilter( pcPicDec->getLumaAddr(pcCU->getAddr(), uiAbsPartIdx), pcPicRest->getLumaAddr(pcCU->getAddr(), uiAbsPartIdx),
                       pcAlfParam->coeff, pcAlfParam->tap,
                       pcCU->getWidth(uiAbsPartIdx), pcCU->getHeight(uiAbsPartIdx),
                       pcPicDec->getStride(), pcPicRest->getStride() );
  }
}

/** \todo		filtering operation is not optimized as frame-based functions
 */
Void TComAdaptiveLoopFilter::xSubCUFilter( Pel* pDec, Pel* pRest, Int *qh, Int iTap, Int iWidth, Int iHeight, Int iDecStride, Int iRestStride )
{
  Int i, x, y, value, N, offset;
  Pel PixSum[ALF_MAX_NUM_COEF];

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Pel* pTmpDec;

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  switch(iTap)
  {
  case 5:
    {
    pDec -= iDecStride*2;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 2;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[5] = pTmpDec[0];
        PixSum[6] = pTmpDec[1];
        PixSum[7] = pTmpDec[2];
        PixSum[8] = pTmpDec[3];
        PixSum[9] = pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[10] = pTmpDec[0];
        PixSum[11] = pTmpDec[1];
        PixSum[12] = pTmpDec[2];
        PixSum[11]+= pTmpDec[3];
        PixSum[10]+= pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[9]+= pTmpDec[0];
        PixSum[8]+= pTmpDec[1];
        PixSum[7]+= pTmpDec[2];
        PixSum[6]+= pTmpDec[3];
        PixSum[5]+= pTmpDec[4];

        pTmpDec += iDecStride;
        PixSum[4]+= pTmpDec[0];
        PixSum[3]+= pTmpDec[1];
        PixSum[2]+= pTmpDec[2];
        PixSum[1]+= pTmpDec[3];
        PixSum[0]+= pTmpDec[4];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = ( value + ALF_ROUND_OFFSET ) >> ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  case 7:
    {
    pDec -= iDecStride*3;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 3;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];
        PixSum[5] = pTmpDec[5];
        PixSum[6] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[7] = pTmpDec[0];
        PixSum[8] = pTmpDec[1];
        PixSum[9] = pTmpDec[2];
        PixSum[10] = pTmpDec[3];
        PixSum[11] = pTmpDec[4];
        PixSum[12] = pTmpDec[5];
        PixSum[13] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[14] = pTmpDec[0];
        PixSum[15] = pTmpDec[1];
        PixSum[16] = pTmpDec[2];
        PixSum[17] = pTmpDec[3];
        PixSum[18] = pTmpDec[4];
        PixSum[19] = pTmpDec[5];
        PixSum[20] = pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[21] = pTmpDec[0];
        PixSum[22] = pTmpDec[1];
        PixSum[23] = pTmpDec[2];
        PixSum[24] = pTmpDec[3];
        PixSum[23]+= pTmpDec[4];
        PixSum[22]+= pTmpDec[5];
        PixSum[21]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[20]+= pTmpDec[0];
        PixSum[19]+= pTmpDec[1];
        PixSum[18]+= pTmpDec[2];
        PixSum[17]+= pTmpDec[3];
        PixSum[16]+= pTmpDec[4];
        PixSum[15]+= pTmpDec[5];
        PixSum[14]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[13]+= pTmpDec[0];
        PixSum[12]+= pTmpDec[1];
        PixSum[11]+= pTmpDec[2];
        PixSum[10]+= pTmpDec[3];
        PixSum[9]+= pTmpDec[4];
        PixSum[8]+= pTmpDec[5];
        PixSum[7]+= pTmpDec[6];

        pTmpDec += iDecStride;
        PixSum[6]+= pTmpDec[0];
        PixSum[5]+= pTmpDec[1];
        PixSum[4]+= pTmpDec[2];
        PixSum[3]+= pTmpDec[3];
        PixSum[2]+= pTmpDec[4];
        PixSum[1]+= pTmpDec[5];
        PixSum[0]+= pTmpDec[6];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  case 9:
    {
    pDec -= iDecStride*4;
    for (y = 0; y < iHeight; y++)
    {
      for (x = 0; x < iWidth; x++)
      {
        pTmpDec = pDec + x - 4;
        PixSum[0] = pTmpDec[0];
        PixSum[1] = pTmpDec[1];
        PixSum[2] = pTmpDec[2];
        PixSum[3] = pTmpDec[3];
        PixSum[4] = pTmpDec[4];
        PixSum[5] = pTmpDec[5];
        PixSum[6] = pTmpDec[6];
        PixSum[7] = pTmpDec[7];
        PixSum[8] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[9] = pTmpDec[0];
        PixSum[10] = pTmpDec[1];
        PixSum[11] = pTmpDec[2];
        PixSum[12] = pTmpDec[3];
        PixSum[13] = pTmpDec[4];
        PixSum[14] = pTmpDec[5];
        PixSum[15] = pTmpDec[6];
        PixSum[16] = pTmpDec[7];
        PixSum[17] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[18] = pTmpDec[0];
        PixSum[19] = pTmpDec[1];
        PixSum[20] = pTmpDec[2];
        PixSum[21] = pTmpDec[3];
        PixSum[22] = pTmpDec[4];
        PixSum[23] = pTmpDec[5];
        PixSum[24] = pTmpDec[6];
        PixSum[25] = pTmpDec[7];
        PixSum[26] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[27] = pTmpDec[0];
        PixSum[28] = pTmpDec[1];
        PixSum[29] = pTmpDec[2];
        PixSum[30] = pTmpDec[3];
        PixSum[31] = pTmpDec[4];
        PixSum[32] = pTmpDec[5];
        PixSum[33] = pTmpDec[6];
        PixSum[34] = pTmpDec[7];
        PixSum[35] = pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[36] = pTmpDec[0];
        PixSum[37] = pTmpDec[1];
        PixSum[38] = pTmpDec[2];
        PixSum[39] = pTmpDec[3];
        PixSum[40] = pTmpDec[4];
        PixSum[39]+= pTmpDec[5];
        PixSum[38]+= pTmpDec[6];
        PixSum[37]+= pTmpDec[7];
        PixSum[36]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[35]+= pTmpDec[0];
        PixSum[34]+= pTmpDec[1];
        PixSum[33]+= pTmpDec[2];
        PixSum[32]+= pTmpDec[3];
        PixSum[31]+= pTmpDec[4];
        PixSum[30]+= pTmpDec[5];
        PixSum[29]+= pTmpDec[6];
        PixSum[28]+= pTmpDec[7];
        PixSum[27]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[26]+= pTmpDec[0];
        PixSum[25]+= pTmpDec[1];
        PixSum[24]+= pTmpDec[2];
        PixSum[23]+= pTmpDec[3];
        PixSum[22]+= pTmpDec[4];
        PixSum[21]+= pTmpDec[5];
        PixSum[20]+= pTmpDec[6];
        PixSum[19]+= pTmpDec[7];
        PixSum[18]+= pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[17]+= pTmpDec[0];
        PixSum[16]+= pTmpDec[1];
        PixSum[15]+= pTmpDec[2];
        PixSum[14]+= pTmpDec[3];
        PixSum[13]+= pTmpDec[4];
        PixSum[12]+= pTmpDec[5];
        PixSum[11]+= pTmpDec[6];
        PixSum[10]+= pTmpDec[7];
        PixSum[9]+=  pTmpDec[8];

        pTmpDec += iDecStride;
        PixSum[8]+= pTmpDec[0];
        PixSum[7]+= pTmpDec[1];
        PixSum[6]+= pTmpDec[2];
        PixSum[5]+= pTmpDec[3];
        PixSum[4]+= pTmpDec[4];
        PixSum[3]+= pTmpDec[5];
        PixSum[2]+= pTmpDec[6];
        PixSum[1]+= pTmpDec[7];
        PixSum[0]+= pTmpDec[8];

        value = 0;
        for(i=0; i<N; i++)
        {
          value += qh[i]*PixSum[i];
        }
        // DC offset
        value += qh[N] << iShift;
        value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

        pRest[x] = (Pel) Clip(value);
      }
      pRest += iRestStride;
      pDec += iDecStride;
    }
    }
    break;
  default:
    assert(0);
    break;
  }
}

Void TComAdaptiveLoopFilter::xFrame(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap)
{
  Int i, x, y, value, N, offset;
  Pel PixSum[ALF_MAX_NUM_COEF];

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Int iHeight = pcPicRest->getHeight();
  Int iWidth = pcPicRest->getWidth();

  Pel* pDec = pcPicDec->getLumaAddr();
  Int iDecStride = pcPicDec->getStride();

  Pel* pRest = pcPicRest->getLumaAddr();
  Int iRestStride = pcPicRest->getStride();

  Pel* pTmpDec1, *pTmpDec2;
  Pel* pTmpPixSum;

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  switch(iTap)
  {
  case 5:
    {
      Int iJump = iDecStride - 4;
      pDec -= iDecStride*2;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-2;
          pTmpDec2 = pTmpDec1+4+(4*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;


          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<13; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[13] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 7:
    {
      Int iJump = iDecStride - 6;
      pDec -= iDecStride*3;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-3;
          pTmpDec2 = pTmpDec1+6+(6*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<25; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[25] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 9:
    {
      Int iJump = iDecStride - 8;
      pDec -= iDecStride*4;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {

          pTmpDec1 = pDec+x-4;
          pTmpDec2 = pTmpDec1+8+(8*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<41; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[41] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  default:
    assert(0);
    break;
  }
}

// --------------------------------------------------------------------------------------------------------------------
// ALF for chroma
// --------------------------------------------------------------------------------------------------------------------

Void TComAdaptiveLoopFilter::xALFChroma(ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  if((pcAlfParam->chroma_idc>>1)&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 0);
  }

  if(pcAlfParam->chroma_idc&0x01)
  {
    xFrameChroma(pcPicDec, pcPicRest, pcAlfParam->coeff_chroma, pcAlfParam->tap_chroma, 1);
  }
}

/** \param	pcPicDec			picture before ALF
		\param	pcPicRest			picture after  ALF
		\param	qh						filter coefficient
		\param	iTap					filter tap
		\param	iColor				0 for Cb and 1 for Cr
 */
Void TComAdaptiveLoopFilter::xFrameChroma( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Int *qh, Int iTap, Int iColor )
{
  Int i, x, y, value, N, offset;
//  Pel PixSum[ALF_MAX_NUM_COEF_C];// th
  Pel PixSum[ALF_MAX_NUM_COEF]; 

  N      = (iTap*iTap+1)>>1;
  offset = iTap>>1;

  Int iHeight = pcPicRest->getHeight() >> 1;
  Int iWidth = pcPicRest->getWidth() >> 1;

  Pel* pDec;
  Int iDecStride = pcPicDec->getCStride();

  Pel* pRest;
  Int iRestStride = pcPicRest->getCStride();

  Int iShift = g_uiBitDepth + g_uiBitIncrement - 8;

  if (iColor)
  {
    pDec = pcPicDec->getCrAddr();
    pRest = pcPicRest->getCrAddr();
  }
  else
  {
    pDec = pcPicDec->getCbAddr();
    pRest = pcPicRest->getCbAddr();
  }

  Pel* pTmpDec1, *pTmpDec2;
  Pel* pTmpPixSum;

  switch(iTap)
  {
  case 5:
    {
      Int iJump = iDecStride - 4;
      pDec -= iDecStride*2;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-2;
          pTmpDec2 = pTmpDec1+4+(4*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 7:
    {
      Int iJump = iDecStride - 6;
      pDec -= iDecStride*3;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-3;
          pTmpDec2 = pTmpDec1+6+(6*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum = (*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  case 9:
    {
      Int iJump = iDecStride - 8;
      pDec -= iDecStride*4;
      for (y = 0; y < iHeight; y++)
      {
        for (x = 0; x < iWidth; x++)
        {
          pTmpDec1 = pDec+x-4;
          pTmpDec2 = pTmpDec1+8+(8*iDecStride);
          pTmpPixSum = PixSum;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;

          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1 += iJump; pTmpDec2 -= iJump;


          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++; pTmpDec2--;
          *pTmpPixSum = ((*pTmpDec1) + (*pTmpDec2));
          pTmpPixSum++; pTmpDec1++;
          *pTmpPixSum =(*pTmpDec1);

          value = 0;
          for(i=0; i<N; i++)
          {
            value += qh[i]*PixSum[i];
          }
          // DC offset
          value += qh[N] << iShift;
          value = (value + ALF_ROUND_OFFSET)>>ALF_NUM_BIT_SHIFT;

          pRest[x] = (Pel) Clip(value);
        }
        pRest += iRestStride;
        pDec += iDecStride;
      }
    }
    break;
  default:
    assert(0);
    break;
  }
}
#endif
