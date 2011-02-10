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

/** \file     TComPrediction.cpp
    \brief    prediction class
*/

#include <memory.h>
#include "TComPrediction.h"

// ====================================================================================================================
// Constructor / destructor / initialize
// ====================================================================================================================

TComPrediction::TComPrediction()
{
  m_piYuvExt = NULL;
}

TComPrediction::~TComPrediction()
{
  m_cYuvExt.destroy();
  
  delete[] m_piYuvExt;
  
  m_acYuvPred[0].destroy();
  m_acYuvPred[1].destroy();
  
  m_cYuvPredTemp.destroy();
}

Void TComPrediction::initTempBuff()
{
  if( m_piYuvExt == NULL )
  {
    m_iYuvExtHeight  = ((g_uiMaxCUHeight + 2) << 4);
#if DCTIF_8_6_LUMA
    m_iYuvExtStride = ((g_uiMaxCUWidth  + 8) << 4);
#else
    m_iYuvExtStride = ((g_uiMaxCUWidth  + 12) << 4);
#endif    
    m_cYuvExt.create( m_iYuvExtStride, m_iYuvExtHeight );
    m_piYuvExt = new Int[ m_iYuvExtStride * m_iYuvExtHeight ];
    
    // new structure
    m_acYuvPred[0] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );
    m_acYuvPred[1] .create( g_uiMaxCUWidth, g_uiMaxCUHeight );
    
    m_cYuvPredTemp.create( g_uiMaxCUWidth, g_uiMaxCUHeight );
  }
#if !DCTIF_8_6_LUMA  
  m_iDIFHalfTap   = ( m_iDIFTap  >> 1 );
#endif
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// Function for calculating DC value of the reference samples used in Intra prediction
Pel TComPrediction::predIntraGetPredValDC( Int* pSrc, Int iSrcStride, UInt iWidth, UInt iHeight, Bool bAbove, Bool bLeft )
{
  Int iInd, iSum = 0;
  Pel pDcVal;
  
  if (bAbove)
  {
    for (iInd = 0;iInd < iWidth;iInd++)
      iSum += pSrc[iInd-iSrcStride];
  }
  if (bLeft)
  {
    for (iInd = 0;iInd < iHeight;iInd++)
      iSum += pSrc[iInd*iSrcStride-1];
  }
  
  if (bAbove && bLeft)
    pDcVal = (iSum + iWidth) / (iWidth + iHeight);
  else if (bAbove)
    pDcVal = (iSum + iWidth/2) / iWidth;
  else if (bLeft)
    pDcVal = (iSum + iHeight/2) / iHeight;
  else
    pDcVal = pSrc[-1]; // Default DC value already calculated and placed in the prediction array if no neighbors are available
  
  return pDcVal;
}

// Function for deriving the angular Intra predictions

/** Function for deriving the simplified angular intra predictions.
 * \param pSrc pointer to reconstructed sample array
 * \param srcStride the stride of the reconstructed sample array
 * \param rpDst reference to pointer for the prediction sample array
 * \param dstStride the stride of the prediction sample array
 * \param width the width of the block
 * \param height the height of the block
 * \param dirMode the intra prediction mode index
 * \param blkAboveAvailable boolean indication if the block above is available 
 * \param blkLeftAvailable boolean indication if the block to the left is available
 * \returns
 * This function derives the prediction samples for the angular mode based on the prediction direction indicated by
 * the prediction mode index. The prediction direction is given by the displacement of the bottom row of the block and 
 * the reference row above the block in the case of vertical prediction or displacement of the rightmost column 
 * of the block and reference column left from the block in the case of the horizontal prediction. The displacement 
 * is signalled at 1/32 pixel accuracy. When projection of the predicted pixel falls inbetween reference samples, 
 * the predicted value for the pixel is linearly interpolated from the reference samples. All reference samples are taken
 * from the extended main reference.
 */
Void TComPrediction::xPredIntraAng( Int* pSrc, Int srcStride, Pel*& rpDst, Int dstStride, UInt width, UInt height, UInt dirMode, Bool blkAboveAvailable, Bool blkLeftAvailable )
{
  Int k,l;
  Int blkSize        = width;
  Pel* pDst          = rpDst;
  
  // Map the mode index to main prediction direction and angle
  Bool modeDC        = dirMode == 0;
  Bool modeVer       = !modeDC && (dirMode < 18);
  Bool modeHor       = !modeDC && !modeVer;
  Int intraPredAngle = modeVer ? dirMode - 9 : modeHor ? dirMode - 25 : 0;
  Int absAng         = abs(intraPredAngle);
  Int signAng        = intraPredAngle < 0 ? -1 : 1;
  
  // Set bitshifts and scale the angle parameter to block size
  Int angTable[9]    = {0,    2,    5,   9,  13,  17,  21,  26,  32};
  Int invAngTable[9] = {0, 4096, 1638, 910, 630, 482, 390, 315, 256}; // (256 * 32) / Angle
  Int invAngle       = invAngTable[absAng];
  absAng             = angTable[absAng];
  intraPredAngle     = signAng * absAng;
  
  // Do the DC prediction
  if (modeDC)
  {
    Pel dcval = predIntraGetPredValDC(pSrc, srcStride, width, height, blkAboveAvailable, blkLeftAvailable);
    
    for (k=0;k<blkSize;k++)
    {
      for (l=0;l<blkSize;l++)
      {
        pDst[k*dstStride+l] = dcval;
      }
    }
  }
  
  // Do angular predictions
  else
  {
    Pel* refMain;
    Pel* refSide;
    Pel  refAbove[2*MAX_CU_SIZE+1];
    Pel  refLeft[2*MAX_CU_SIZE+1];
    
    // Initialise the Main and Left reference array. 
    if (intraPredAngle < 0)
    {
      for (k=0;k<blkSize+1;k++)
      {
        refAbove[k+blkSize-1] = pSrc[k-srcStride-1];
      }
      for (k=0;k<blkSize+1;k++)
      {
        refLeft[k+blkSize-1] = pSrc[(k-1)*srcStride-1];
      }
      refMain = (modeVer ? refAbove : refLeft) + (blkSize-1);
      refSide = (modeVer ? refLeft : refAbove) + (blkSize-1);
      
      // Extend the Main reference to the left.
      Int invAngleSum    = 128;       // rounding for (shift by 8)
      for (k=-1; k>blkSize*intraPredAngle>>5; k--)
      {
        invAngleSum += invAngle;
        refMain[k] = refSide[invAngleSum>>8];
      }
    }
    else 
    {
      for (k=0;k<2*blkSize+1;k++)
      {
        refAbove[k] = pSrc[k-srcStride-1];
      }
      for (k=0;k<2*blkSize+1;k++)
      {
        refLeft[k] = pSrc[(k-1)*srcStride-1];
      }
      refMain = modeVer ? refAbove : refLeft;
    }
    
    if (intraPredAngle == 0)
    {
      for (k=0;k<blkSize;k++)
      {
        for (l=0;l<blkSize;l++)
        {
          pDst[k*dstStride+l] = refMain[l+1];
        }
      }
    }
    else
    {
      Int deltaPos=0;
      Int deltaInt;
      Int deltaFract;
      Int refMainIndex;
      
      for (k=0;k<blkSize;k++)
      {
        deltaPos += intraPredAngle;
        deltaInt   = deltaPos >> 5;
        deltaFract = deltaPos & (32 - 1);
        
        if (deltaFract)
        {
          // Do linear filtering
          for (l=0;l<blkSize;l++)
          {
            refMainIndex        = l+deltaInt+1;
            pDst[k*dstStride+l] = (Pel) ( ((32-deltaFract)*refMain[refMainIndex]+deltaFract*refMain[refMainIndex+1]+16) >> 5 );
          }
        }
        else
        {
          // Just copy the integer samples
          for (l=0;l<blkSize;l++)
          {
            pDst[k*dstStride+l] = refMain[l+deltaInt+1];
          }
        }
      }
    }
    
    // Flip the block if this is the horizontal mode
    if (modeHor)
    {
      Pel  tmp;
      for (k=0;k<blkSize-1;k++)
      {
        for (l=k+1;l<blkSize;l++)
        {
          tmp                 = pDst[k*dstStride+l];
          pDst[k*dstStride+l] = pDst[l*dstStride+k];
          pDst[l*dstStride+k] = tmp;
        }
      }
    }
  }
}

Void TComPrediction::predIntraLumaAng(TComPattern* pcTComPattern, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft )
{
  Pel *pDst = piPred;
  Int *ptrSrc;
  
  // only assign variable in debug mode
#ifndef NDEBUG
  // get intra direction
  Int iIntraSizeIdx = g_aucConvertToBit[ iWidth ] + 1;
  
  assert( iIntraSizeIdx >= 1 ); //   4x  4
  assert( iIntraSizeIdx <= 6 ); // 128x128
  assert( iWidth == iHeight  );
#endif //NDEBUG
  
#if QC_MDIS
  ptrSrc = pcTComPattern->getPredictorPtr( uiDirMode, g_aucConvertToBit[ iWidth ] + 1, iWidth, iHeight, m_piYuvExt );
#else
  ptrSrc = pcTComPattern->getAdiOrgBuf( iWidth, iHeight, m_piYuvExt );
#endif //QC_MDIS
  
  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;
  
  // get converted direction
  uiDirMode = g_aucAngIntraModeOrder[ uiDirMode ];
  
  // Create the prediction
  xPredIntraAng( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode, bAbove,  bLeft );
}

// Angular chroma
Void TComPrediction::predIntraChromaAng( TComPattern* pcTComPattern, Int* piSrc, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAbove, Bool bLeft )
{
  Pel *pDst = piPred;
  Int *ptrSrc = piSrc;
  
  // get starting pixel in block
  Int sw = ( iWidth<<1 ) + 1;
  
  // get converted direction
  uiDirMode = g_aucAngIntraModeOrder[ uiDirMode ];
  
  // Create the prediction
  xPredIntraAng( ptrSrc+sw+1, sw, pDst, uiStride, iWidth, iHeight, uiDirMode, bAbove,  bLeft );
}

Void TComPrediction::motionCompensation ( TComDataCU* pcCU, TComYuv* pcYuvPred, RefPicList eRefPicList, Int iPartIdx )
{
  Int         iWidth;
  Int         iHeight;
  UInt        uiPartAddr;
  
  if ( iPartIdx >= 0 )
  {
    pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
    if ( eRefPicList != REF_PIC_LIST_X )
    {
      xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
    }
    else
    {
      xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
    }
#if HIGH_ACCURACY_BI
    pcYuvPred->shiftBack( uiPartAddr, iWidth, iHeight);
#endif
    return;
  }
  
  for ( iPartIdx = 0; iPartIdx < pcCU->getNumPartInter(); iPartIdx++ )
  {
    pcCU->getPartIndexAndSize( iPartIdx, uiPartAddr, iWidth, iHeight );
    
    if ( eRefPicList != REF_PIC_LIST_X )
    {
      xPredInterUni (pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcYuvPred, iPartIdx );
    }
    else
    {
      xPredInterBi  (pcCU, uiPartAddr, iWidth, iHeight, pcYuvPred, iPartIdx );
    }
#if HIGH_ACCURACY_BI
    pcYuvPred->shiftBack( uiPartAddr, iWidth, iHeight);
#endif
  }
  return;
}

Void TComPrediction::xPredInterUni ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  Int         iRefIdx     = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );           assert (iRefIdx >= 0);
  TComMv      cMv         = pcCU->getCUMvField( eRefPicList )->getMv( uiPartAddr );
#if DCTIF_8_6_LUMA && DCTIF_4_6_CHROMA
  pcCU->clipMv(cMv);

#if HIGH_ACCURACY_BI
  xPredInterLumaBlk_ha  ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  xPredInterChromaBlk_ha ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  //xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
#else
  xPredInterLumaBlk       ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
#endif

  
#else
  InterpFilterType ePFilt = (InterpFilterType)pcCU->getSlice()->getInterpFilterType();
  
  pcCU->clipMv(cMv);

  switch ( ePFilt )
  {
#if TEN_DIRECTIONAL_INTERP
    case IPF_TEN_DIF:
      xPredInterLumaBlk_TEN   ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
      xPredInterChromaBlk ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec()    , uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
      break;
#endif
    default:
      xPredInterLumaBlk       ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
      xPredInterChromaBlk     ( pcCU, pcCU->getSlice()->getRefPic( eRefPicList, iRefIdx )->getPicYuvRec(), uiPartAddr, &cMv, iWidth, iHeight, rpcYuvPred );
  }
#endif
}

Void TComPrediction::xPredInterBi ( TComDataCU* pcCU, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvPred, Int iPartIdx )
{
  TComYuv* pcMbYuv;
  Int      iRefIdx[2] = {-1, -1};
  
  for ( Int iRefList = 0; iRefList < 2; iRefList++ )
  {
    RefPicList eRefPicList = (iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0);
    iRefIdx[iRefList] = pcCU->getCUMvField( eRefPicList )->getRefIdx( uiPartAddr );
    
    if ( iRefIdx[iRefList] < 0 )  
    { 
      continue; 
    }
    
    assert( iRefIdx[iRefList] < pcCU->getSlice()->getNumRefIdx(eRefPicList) );
    
    pcMbYuv = &m_acYuvPred[iRefList];
    xPredInterUni ( pcCU, uiPartAddr, iWidth, iHeight, eRefPicList, pcMbYuv, iPartIdx );
  }
  
  xWeightedAverage( pcCU, &m_acYuvPred[0], &m_acYuvPred[1], iRefIdx[0], iRefIdx[1], uiPartAddr, iWidth, iHeight, rpcYuvPred );
}

#if HIGH_ACCURACY_BI0
Void  TComPrediction::xPredInterLumaBlk_ha( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();
  
  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;
  
  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  UInt shiftNum = 14-8-g_uiBitIncrement;
  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piRefY[x]<<shiftNum;   
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }
#if DCTIF_8_6_LUMA
    //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor_ha ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );

  }
  
  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
    return;
  }
  
  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();
 
  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {

	xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );	

	return;
  }
  
  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );	  
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }

	  return;
	}
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY -3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );	
	  	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
	  return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
	   
      return;
    }
  }
  
  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
  
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );	
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
	  return;
    }
  }
  
  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );	  	  
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY -3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	
      return;
    }
  }
  
  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
  
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
	 
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
 
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piDstY[x]<<shiftNum;   
      piDstY += iDstStride;      
    }
	  	  
      return;
    }
  }
#else
  //This branch is not supported

  //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();
  
  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer (piRefY - 6,  iRefStride, 1, iWidth + 12, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor (piExtY + 6,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
#endif
}
#endif

#if HIGH_ACCURACY_BI

Void  TComPrediction::xPredInterLumaBlk_ha( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();
  
  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;
  
  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
    UInt shiftNum = 14-8-g_uiBitIncrement;
  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
		  piDstY[x] = piRefY[x]<<shiftNum;  
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }

    //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor_ha ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer_ha ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();
 
  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {

	xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

    return;
  }
  
  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );	
      return;
    }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY -3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );	  	
      return;
    }
  }
  
  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver_ha( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY -3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
	  	
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor_ha (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );

      return;
    }
  }
  }

#endif

Void  TComPrediction::xPredInterLumaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();
  
  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;
  
  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  
  //  Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstY, piRefY, sizeof(Pel)*iWidth);
      piDstY += iDstStride;
      piRefY += iRefStride;
    }
    return;
  }
#if DCTIF_8_6_LUMA
    //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();
 
  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {

	xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY -3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY -3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - 3,  iRefStride, 1, iWidth +7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - 3,  iRefStride, 1, iWidth + 7, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + 3,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
#else
  //  Half-pel horizontal
  if ( ixFrac == 2 && iyFrac == 0 )
  {
    xCTI_FilterHalfHor ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Half-pel vertical
  if ( ixFrac == 0 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  Int   iExtStride = m_iYuvExtStride;//m_cYuvExt.getStride();
  Int*  piExtY     = m_piYuvExt;//m_cYuvExt.getLumaAddr();
  
  //  Half-pel center
  if ( ixFrac == 2 && iyFrac == 2 )
  {
    xCTI_FilterHalfVer (piRefY - 6,  iRefStride, 1, iWidth + 12, iHeight, iExtStride, 1, piExtY );
    xCTI_FilterHalfHor (piExtY + 6,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
    return;
  }
  
  //  Quater-pel horizontal
  if ( iyFrac == 0)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Hor( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 2 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterHalfVer (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  //  Quater-pel vertical
  if( ixFrac == 0 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  if( ixFrac == 2 )
  {
    if( iyFrac == 1 )
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if( iyFrac == 3 )
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterHalfHor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  
  /// Quarter-pel center
  if ( iyFrac == 1)
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter0Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
  if ( iyFrac == 3 )
  {
    if ( ixFrac == 1)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter0Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
    if ( ixFrac == 3)
    {
      xCTI_FilterQuarter1Ver (piRefY - m_iDIFHalfTap + 1,  iRefStride, 1, iWidth + m_iDIFTap - 1, iHeight, iExtStride, 1, piExtY );
      xCTI_FilterQuarter1Hor (piExtY + m_iDIFHalfTap - 1,  iExtStride, 1, iWidth    , iHeight, iDstStride, 1, piDstY );
      return;
    }
  }
#endif
}

#if HIGH_ACCURACY_BI
Void TComPrediction::xPredInterChromaBlk_ha( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride  = pcPicYuvRef->getCStride();
  Int     iDstStride  = rpcYuv->getCStride();
  
  Int     iRefOffset  = (pcMv->getHor() >> 3) + (pcMv->getVer() >> 3) * iRefStride;
  
  Pel*    piRefCb     = pcPicYuvRef->getCbAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  Pel*    piRefCr     = pcPicYuvRef->getCrAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Pel* piDstCb = rpcYuv->getCbAddr( uiPartAddr );
  Pel* piDstCr = rpcYuv->getCrAddr( uiPartAddr );
  
  Int     ixFrac  = pcMv->getHor() & 0x7;
  Int     iyFrac  = pcMv->getVer() & 0x7;
#if !DCTIF_4_6_CHROMA
  Int     x, y;
#endif
  UInt    uiCWidth  = iWidth  >> 1;
  UInt    uiCHeight = iHeight >> 1;
  
#if DCTIF_4_6_CHROMA
  xDCTIF_FilterC_ha(piRefCb, iRefStride,piDstCb,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  xDCTIF_FilterC_ha(piRefCr, iRefStride,piDstCr,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
  return;
#else
  //This branch is not supported
  // Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( y = 0; y < uiCHeight; y++ )
    {
      ::memcpy(piDstCb, piRefCb, sizeof(Pel)*uiCWidth);
      ::memcpy(piDstCr, piRefCr, sizeof(Pel)*uiCWidth);
      piDstCb += iDstStride;
      piDstCr += iDstStride;
      piRefCb += iRefStride;
      piRefCr += iRefStride;
    }
    return;
  }
  
  // Horizontal point
  if ( iyFrac == 0 )
  {
    Pel* piRefCbP1;
    Pel* piRefCrP1;
    
    if ( ixFrac == 4 )
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piRefCbP1= piRefCb + 1;
        piRefCrP1= piRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + 1) >> 1;
          piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + 1) >> 1;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    else
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piRefCbP1= piRefCb + 1;
        piRefCrP1= piRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = ( ( piRefCb[x] << 3 ) + ixFrac * ( piRefCbP1[x] - piRefCb[x] ) + 4 ) >> 3;
          piDstCr[x] = ( ( piRefCr[x] << 3 ) + ixFrac * ( piRefCrP1[x] - piRefCr[x] ) + 4 ) >> 3;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    return;
  }
  
  // Vertical point
  if ( ixFrac == 0 )
  {
    Pel* piNextRefCb;
    Pel* piNextRefCr;
    
    if (iyFrac == 4)
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piNextRefCb[x] + 1) >> 1;
          piDstCr[x] = (piRefCr[x] + piNextRefCr[x] + 1) >> 1;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    else
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = ( ( piRefCb[x] << 3 ) + iyFrac * ( piNextRefCb[x] - piRefCb[x] ) + 4) >> 3;
          piDstCr[x] = ( ( piRefCr[x] << 3 ) + iyFrac * ( piNextRefCr[x] - piRefCr[x] ) + 4) >> 3;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    return;
  }
  
  // Center point
  {
    Pel* piNextRefCb;
    Pel* piNextRefCr;
    
    Pel* piRefCbP1;
    Pel* piNextRefCbP1;
    Pel* piRefCrP1;
    Pel* piNextRefCrP1;
    
    if (ixFrac == 4 && iyFrac == 4)
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        
        piRefCbP1= piRefCb + 1;
        piNextRefCbP1 = piNextRefCb + 1;
        piRefCrP1= piRefCr + 1;
        piNextRefCrP1 = piNextRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + piNextRefCb[x] + piNextRefCbP1[x] + 2) >> 2;
          piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + piNextRefCr[x] + piNextRefCrP1[x] + 2) >> 2;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
      return;
    }
    
    Int aCb, bCb, cCb;
    Int aCr, bCr, cCr;
    
    for ( y = 0; y < uiCHeight; y++ )
    {
      piNextRefCb = piRefCb + iRefStride;
      piNextRefCr = piRefCr + iRefStride;
      
      aCb = ( piRefCb[0] << 3 ) + iyFrac * ( piNextRefCb[0] - piRefCb[0] ); aCb *= ( 8 - ixFrac );
      aCr = ( piRefCr[0] << 3 ) + iyFrac * ( piNextRefCr[0] - piRefCr[0] ); aCr *= ( 8 - ixFrac );
      
      piRefCbP1     = piRefCb     + 1;
      piNextRefCbP1 = piNextRefCb + 1;
      piRefCrP1     = piRefCr     + 1;
      piNextRefCrP1 = piNextRefCr + 1;
      
      for ( x = 0; x < uiCWidth; x++ )
      {
        bCb = ( piRefCbP1[x] << 3 ) + iyFrac * ( piNextRefCbP1[x] - piRefCbP1[x] );
        bCr = ( piRefCrP1[x] << 3 ) + iyFrac * ( piNextRefCrP1[x] - piRefCrP1[x] );
        
        cCb = ixFrac     * bCb;
        cCr = ixFrac     * bCr;
        
        piDstCb[x] = ( aCb + cCb + 32 ) >> 6;
        piDstCr[x] = ( aCr + cCr + 32 ) >> 6;
        
        aCb = (bCb<<3) - cCb;
        aCr = (bCr<<3) - cCr;
      }
      piDstCb += iDstStride;
      piDstCr += iDstStride;
      piRefCb += iRefStride;
      piRefCr += iRefStride;
    }
  }
#endif
}
#endif

//--
Void TComPrediction::xPredInterChromaBlk( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride  = pcPicYuvRef->getCStride();
  Int     iDstStride  = rpcYuv->getCStride();
  
  Int     iRefOffset  = (pcMv->getHor() >> 3) + (pcMv->getVer() >> 3) * iRefStride;
  
  Pel*    piRefCb     = pcPicYuvRef->getCbAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  Pel*    piRefCr     = pcPicYuvRef->getCrAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Pel* piDstCb = rpcYuv->getCbAddr( uiPartAddr );
  Pel* piDstCr = rpcYuv->getCrAddr( uiPartAddr );
  
  Int     ixFrac  = pcMv->getHor() & 0x7;
  Int     iyFrac  = pcMv->getVer() & 0x7;
#if !DCTIF_4_6_CHROMA
  Int     x, y;
#endif
  UInt    uiCWidth  = iWidth  >> 1;
  UInt    uiCHeight = iHeight >> 1;
  
#if DCTIF_4_6_CHROMA
  xDCTIF_FilterC(piRefCb, iRefStride,piDstCb,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
	xDCTIF_FilterC(piRefCr, iRefStride,piDstCr,iDstStride,uiCWidth,uiCHeight, iyFrac, ixFrac);
	return;
#else
  // Integer point
  if ( ixFrac == 0 && iyFrac == 0 )
  {
    for ( y = 0; y < uiCHeight; y++ )
    {
      ::memcpy(piDstCb, piRefCb, sizeof(Pel)*uiCWidth);
      ::memcpy(piDstCr, piRefCr, sizeof(Pel)*uiCWidth);
      piDstCb += iDstStride;
      piDstCr += iDstStride;
      piRefCb += iRefStride;
      piRefCr += iRefStride;
    }
    return;
  }
  
  // Horizontal point
  if ( iyFrac == 0 )
  {
    Pel* piRefCbP1;
    Pel* piRefCrP1;
    
    if ( ixFrac == 4 )
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piRefCbP1= piRefCb + 1;
        piRefCrP1= piRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + 1) >> 1;
          piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + 1) >> 1;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    else
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piRefCbP1= piRefCb + 1;
        piRefCrP1= piRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = ( ( piRefCb[x] << 3 ) + ixFrac * ( piRefCbP1[x] - piRefCb[x] ) + 4 ) >> 3;
          piDstCr[x] = ( ( piRefCr[x] << 3 ) + ixFrac * ( piRefCrP1[x] - piRefCr[x] ) + 4 ) >> 3;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    return;
  }
  
  // Vertical point
  if ( ixFrac == 0 )
  {
    Pel* piNextRefCb;
    Pel* piNextRefCr;
    
    if (iyFrac == 4)
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piNextRefCb[x] + 1) >> 1;
          piDstCr[x] = (piRefCr[x] + piNextRefCr[x] + 1) >> 1;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    else
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = ( ( piRefCb[x] << 3 ) + iyFrac * ( piNextRefCb[x] - piRefCb[x] ) + 4) >> 3;
          piDstCr[x] = ( ( piRefCr[x] << 3 ) + iyFrac * ( piNextRefCr[x] - piRefCr[x] ) + 4) >> 3;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
    }
    return;
  }
  
  // Center point
  {
    Pel* piNextRefCb;
    Pel* piNextRefCr;
    
    Pel* piRefCbP1;
    Pel* piNextRefCbP1;
    Pel* piRefCrP1;
    Pel* piNextRefCrP1;
    
    if (ixFrac == 4 && iyFrac == 4)
    {
      for ( y = 0; y < uiCHeight; y++ )
      {
        piNextRefCb = piRefCb + iRefStride;
        piNextRefCr = piRefCr + iRefStride;
        
        piRefCbP1= piRefCb + 1;
        piNextRefCbP1 = piNextRefCb + 1;
        piRefCrP1= piRefCr + 1;
        piNextRefCrP1 = piNextRefCr + 1;
        
        for ( x = 0; x < uiCWidth; x++ )
        {
          piDstCb[x] = (piRefCb[x] + piRefCbP1[x] + piNextRefCb[x] + piNextRefCbP1[x] + 2) >> 2;
          piDstCr[x] = (piRefCr[x] + piRefCrP1[x] + piNextRefCr[x] + piNextRefCrP1[x] + 2) >> 2;
        }
        piDstCb += iDstStride;
        piDstCr += iDstStride;
        piRefCb += iRefStride;
        piRefCr += iRefStride;
      }
      return;
    }
    
    Int aCb, bCb, cCb;
    Int aCr, bCr, cCr;
    
    for ( y = 0; y < uiCHeight; y++ )
    {
      piNextRefCb = piRefCb + iRefStride;
      piNextRefCr = piRefCr + iRefStride;
      
      aCb = ( piRefCb[0] << 3 ) + iyFrac * ( piNextRefCb[0] - piRefCb[0] ); aCb *= ( 8 - ixFrac );
      aCr = ( piRefCr[0] << 3 ) + iyFrac * ( piNextRefCr[0] - piRefCr[0] ); aCr *= ( 8 - ixFrac );
      
      piRefCbP1     = piRefCb     + 1;
      piNextRefCbP1 = piNextRefCb + 1;
      piRefCrP1     = piRefCr     + 1;
      piNextRefCrP1 = piNextRefCr + 1;
      
      for ( x = 0; x < uiCWidth; x++ )
      {
        bCb = ( piRefCbP1[x] << 3 ) + iyFrac * ( piNextRefCbP1[x] - piRefCbP1[x] );
        bCr = ( piRefCrP1[x] << 3 ) + iyFrac * ( piNextRefCrP1[x] - piRefCrP1[x] );
        
        cCb = ixFrac     * bCb;
        cCr = ixFrac     * bCr;
        
        piDstCb[x] = ( aCb + cCb + 32 ) >> 6;
        piDstCr[x] = ( aCr + cCr + 32 ) >> 6;
        
        aCb = (bCb<<3) - cCb;
        aCr = (bCr<<3) - cCr;
      }
      piDstCb += iDstStride;
      piDstCr += iDstStride;
      piRefCb += iRefStride;
      piRefCr += iRefStride;
    }
  }
#endif
}
#if DCTIF_4_6_CHROMA
Void  TComPrediction::xDCTIF_FilterC ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,
                                       Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  // Integer point
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for ( Int y = 0; y < iHeight; y++ )
    {
      ::memcpy(piDstC, piRefC, sizeof(Pel)*iWidth);
      piDstC += iDstStride;
      piRefC += iRefStride;
    }
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHorC (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVxFrac );
    return;
  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVerC (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVyFrac );
    return;
}

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtC     = m_piYuvExt;

  xCTI_Filter2DVerC (piRefC - 1,  iRefStride,  iWidth + 3, iHeight, iExtStride,  piExtC, iMVyFrac );
  xCTI_Filter2DHorC (piExtC + 1,  iExtStride,  iWidth             , iHeight, iDstStride,  piDstC, iMVxFrac );
}

#if HIGH_ACCURACY_BI

Void  TComPrediction::xDCTIF_FilterC_ha ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,
                                       Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac)
{
  UInt    shiftNumOrg = 6 - g_uiBitIncrement;
  // Integer point
  if ( iMVxFrac == 0 && iMVyFrac == 0 )
  {
    for (Int y = 0; y < iHeight; y++ )
    {
       for(Int x=0; x<iWidth; x++)
	   {
		   piDstC[x] = (piRefC[x]<<shiftNumOrg); 
	   }
	   piDstC += iDstStride;
	   piRefC += iRefStride;
	}
    return;
  }

  if ( iMVyFrac == 0 )
  {
    xCTI_Filter1DHorC_ha (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVxFrac );
	return;

  }

  if ( iMVxFrac == 0 )
  {
    xCTI_Filter1DVerC_ha (piRefC, iRefStride,  iWidth, iHeight, iDstStride,  piDstC, iMVyFrac );
    return;
  }

  Int   iExtStride = m_iYuvExtStride;
  Int*  piExtC     = m_piYuvExt;

  xCTI_Filter2DVerC (piRefC - 1,  iRefStride,  iWidth + 3, iHeight, iExtStride,  piExtC, iMVyFrac );
  xCTI_Filter2DHorC_ha (piExtC + 1,  iExtStride,  iWidth , iHeight, iDstStride,  piDstC, iMVxFrac );
  return;
  
}

#endif

#endif

#if TEN_DIRECTIONAL_INTERP
Void  TComPrediction::xPredInterLumaBlk_TEN( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv )
{
  Int     iRefStride = pcPicYuvRef->getStride();
  Int     iDstStride = rpcYuv->getStride();
  
  Int     iRefOffset = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY     = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  
  Int     ixFrac  = pcMv->getHor() & 0x3;
  Int     iyFrac  = pcMv->getVer() & 0x3;
  
  Pel* piDstY = rpcYuv->getLumaAddr( uiPartAddr );
  
  xCTI_FilterDIF_TEN ( piRefY, iRefStride, 1, iWidth, iHeight, iDstStride, 1, piDstY, iyFrac, ixFrac);
  return;
}

#endif

Void TComPrediction::xWeightedAverage( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, Int iRefIdx0, Int iRefIdx1, UInt uiPartIdx, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst )
{
  if( iRefIdx0 >= 0 && iRefIdx1 >= 0 )
  {
#ifdef ROUNDING_CONTROL_BIPRED
    rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight, pcCU->getSlice()->isRounding());
#else
    rpcYuvDst->addAvg( pcYuvSrc0, pcYuvSrc1, uiPartIdx, iWidth, iHeight );
#endif
  }
  else if ( iRefIdx0 >= 0 && iRefIdx1 <  0 )
  {
    pcYuvSrc0->copyPartToPartYuv( rpcYuvDst, uiPartIdx, iWidth, iHeight );
  }
  else if ( iRefIdx0 <  0 && iRefIdx1 >= 0 )
  {
    pcYuvSrc1->copyPartToPartYuv( rpcYuvDst, uiPartIdx, iWidth, iHeight );
  }
  else
  {
    assert (0);
  }
}

// AMVP
Void TComPrediction::getMvPredAMVP( TComDataCU* pcCU, UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred )
{
  AMVPInfo* pcAMVPInfo = pcCU->getCUMvField(eRefPicList)->getAMVPInfo();
  
  if( pcCU->getAMVPMode(uiPartAddr) == AM_NONE || (pcAMVPInfo->iN <= 1 && pcCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    rcMvPred = pcAMVPInfo->m_acMvCand[0];
    
    pcCU->setMVPIdxSubParts( 0, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    pcCU->setMVPNumSubParts( pcAMVPInfo->iN, eRefPicList, uiPartAddr, uiPartIdx, pcCU->getDepth(uiPartAddr));
    return;
  }
  
  assert(pcCU->getMVPIdx(eRefPicList,uiPartAddr) >= 0);
  rcMvPred = pcAMVPInfo->m_acMvCand[pcCU->getMVPIdx(eRefPicList,uiPartAddr)];
  return;
}
