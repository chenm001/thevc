/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, FRAUNHOFER HHI
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of FRAUNHOFER HHI
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


/** \file     TComPredFilterMOMS.cpp
    \brief    MOMS interpolation filter
*/

#include "TComPredFilterMOMS.h"

#if HHI_INTERP_FILTER

// predict luma block for a given motion vector for motion compensation
Void  TComPredFilterMOMS::predInterLumaBlkMOMS( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt )
{
  Int     iRefStride  = pcPicYuvRef->getStride();
  Int     iDstStride  = rpcYuv->getStride();

  Int     iRefOffset  = ( pcMv->getHor() >> 2 ) + ( pcMv->getVer() >> 2 ) * iRefStride;
  Pel*    piRefY      = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac      = pcMv->getHor() & 0x3;
  Int     iyFrac      = pcMv->getVer() & 0x3;

  Pel*    piDstY      = rpcYuv->getLumaAddr( uiPartAddr );

  setFiltType( ePFilt );
  m_pcInterpIf->interpolate( piDstY, iDstStride, piRefY, iRefStride, iHeight, iWidth, ixFrac<<1, iyFrac<<1, 1 );
}
#ifdef QC_AMVRES
// predict luma block for a given motion vector for motion compensation
Void  TComPredFilterMOMS::predInterLumaBlkHAM_MOMS( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt )
{
  Int     iRefStride  = pcPicYuvRef->getStride();
  Int     iDstStride  = rpcYuv->getStride();

  Int     iRefOffset  = ( pcMv->getHor() >> 3 ) + ( pcMv->getVer() >> 3 ) * iRefStride;
  Pel*    piRefY      = pcPicYuvRef->getLumaAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Int     ixFrac      = pcMv->getHor() & 0x7;
  Int     iyFrac      = pcMv->getVer() & 0x7;

  Pel*    piDstY      = rpcYuv->getLumaAddr( uiPartAddr );

  setFiltType( ePFilt );
  m_pcInterpIf->interpolate( piDstY, iDstStride, piRefY, iRefStride, iHeight, iWidth, ixFrac, iyFrac, 1 );
}
// predict luma block for a given motion vector for motion compensation
Void  TComPredFilterMOMS::predInterLumaBlkHAM_ME_MOMS(Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv, 
                                                    Int iWidth, Int iHeight, InterpFilterType ePFilt, Int dMVx, Int dMVy)
{

  Int     mv_x = pcMv->getHor()+dMVx;
  Int     mv_y = pcMv->getVer()+dMVy;
 
  Int 		iOffset = (mv_x >>3) + (mv_y >>3) * iSrcStride;
  Pel*		piRefY		 = piSrcY + iOffset; 
  Int       iRefStride = iSrcStride;

  Int 		ixFrac	= (mv_x & 0x7);
  Int 		iyFrac	= (mv_y & 0x7);
 
  setFiltType( ePFilt );
  m_pcInterpIf->interpolate( piDstY, iDstStride, piRefY, iRefStride, iHeight, iWidth, ixFrac, iyFrac, 1 );
  pcMv->set(mv_x,  mv_y  ); 
}
#endif


// predict chroma block for a given motion vector for motion compensation
Void TComPredFilterMOMS::predInterChromaBlkMOMS( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt )
{
  Int     iRefStride  = pcPicYuvRef->getCStride();
  Int     iDstStride  = rpcYuv->getCStride();

  Int     iRefOffset  = (pcMv->getHor() >> 3) + (pcMv->getVer() >> 3) * iRefStride;

  Pel*    piRefCb     = pcPicYuvRef->getCbAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;
  Pel*    piRefCr     = pcPicYuvRef->getCrAddr( pcCU->getAddr(), pcCU->getZorderIdxInCU() + uiPartAddr ) + iRefOffset;

  Pel*    piDstCb     = rpcYuv->getCbAddr( uiPartAddr );
  Pel*    piDstCr     = rpcYuv->getCrAddr( uiPartAddr );

  Int     ixFrac      = pcMv->getHor() & 0x7;
  Int     iyFrac      = pcMv->getVer() & 0x7;

  setFiltType( ePFilt );
  m_pcInterpIf->interpolate( piDstCb, iDstStride, piRefCb, iRefStride, iHeight>>1, iWidth>>1, ixFrac, iyFrac, 1 );
  m_pcInterpIf->interpolate( piDstCr, iDstStride, piRefCr, iRefStride, iHeight>>1, iWidth>>1, ixFrac, iyFrac, 1 );
}


// half-pixel upsampling for motion estimation
Void TComPredFilterMOMS::extMOMSUpSamplingH ( TComPattern* pcPattern )
{
  Int   iDx, iDy;

  Int   iWidth      = pcPattern->getROIYWidth();
  Int   iHeight     = pcPattern->getROIYHeight();

  Int   iSrcStride  = pcPattern->getPatternLStride();
  Int   iBufStride  = m_cYuvTmp.getStride();

  Pel*  piDstYPel;
  Pel*  piSrcYPel;

  piSrcYPel = pcPattern->getROIY() - iSrcStride - 1;
  piDstYPel = m_cYuvTmp.getLumaAddr();
  for ( iDy = 0; iDy <= 2; iDy += 2 )
  {
    for ( iDx = 0; iDx <= 2; iDx += 2 )
    {
      m_pcInterpIf->interpolate( piDstYPel+(iDx+iDy*iBufStride), iBufStride, piSrcYPel, iSrcStride, iHeight+1, iWidth+1, iDx<<1, iDy<<1, 4 );
    }
  }
}


// quarter-pixel upsampling for motion estimation
Void TComPredFilterMOMS::extMOMSUpSamplingQ ( TComPattern* pcPattern, TComMv cMvHalf, Pel* piDst, Int iDstStride )
{
  Int   iDx, iDy;

  Int   iWidth      = pcPattern->getROIYWidth();
  Int   iHeight     = pcPattern->getROIYHeight();

  Pel*  piSrc       = pcPattern->getROIY();
  Int   iSrcStride  = pcPattern->getPatternLStride();

  for ( iDy = -1; iDy <= 1; iDy++ )
  {
    for ( iDx = -1; iDx <= 1; iDx++ )
    {
      Int iDstDispX = ( cMvHalf.getHor() << 1 ) + iDx;
      Int iDstDispY = ( cMvHalf.getVer() << 1 ) + iDy;

      Int iSrcDispX = ( iDstDispX < 0 ? -1 : 0 );
      Int iSrcDispY = ( iDstDispY < 0 ? -1 : 0 );

      Int iPolyX    = ( iDstDispX < 0 ? 4+iDstDispX : iDstDispX );
      Int iPolyY    = ( iDstDispY < 0 ? 4+iDstDispY : iDstDispY );

      m_pcInterpIf->interpolate( piDst+(iDstDispX+iDstDispY*iDstStride), iDstStride, piSrc+(iSrcDispX+iSrcDispY*iSrcStride), iSrcStride, iHeight, iWidth, iPolyX<<1, iPolyY<<1, 4 );
    }
  }
}


// convert image samples to expansion coefficients
Void TComExpCoeff6TapMOMS::calcExpCoeffs( TComPicYuv *pcSrc, TComPicYuv *pcDst )
{
  xCalcCoeffsPlane( pcSrc->getLumaAddr(), pcDst->getLumaAddr(), pcDst->getHeight(),      pcDst->getWidth(),       pcDst->getStride() );
  xCalcCoeffsPlane( pcSrc->getCbAddr(),   pcDst->getCbAddr(),   pcDst->getHeight() >> 1, pcDst->getWidth() >> 1, pcDst->getCStride() );
  xCalcCoeffsPlane( pcSrc->getCrAddr(),   pcDst->getCrAddr(),   pcDst->getHeight() >> 1, pcDst->getWidth() >> 1, pcDst->getCStride() );
}


// convert image samples to expansion coefficients
Void TComExpCoeff6TapMOMS::xCalcCoeffsPlane( Pel* pcRefPic, Pel* pcCoeffs, Int iYMax, Int iXMax, Int iStride )
{
  xFiltIter( pcRefPic, pcCoeffs, m_i16Q15Z1, m_i16Q15Z1_c, iYMax, iXMax, iStride, m_iShift_IBDI, 6, 6, 6, 6 );
  xFiltIter( pcCoeffs, pcCoeffs, m_i16Q15Z2, m_i16Q15Z2_c, iYMax, iXMax, iStride, 0, 7, 9, 10, 13 );
}

// convert image samples to expansion coefficients
Void TComExpCoeff6TapMOMS::xFiltIter( Pel* pcRefPic, 
                                  Pel* pcCoeffs, 
                                  const Pel i16Q15Z, 
                                  const Pel i16Q15Z_c,
                                  const Int iYMax, 
                                  const Int iXMax, 
                                  const Int iStride, 
                                  const Int iSrcShift,
                                  const Int iQRowCausal, 
                                  const Int iQRowAntiCausal, 
                                  const Int iQColCausal, 
                                  const Int iQColAntiCausal )
{
  Int iY, iX;

  Pel* pcSrc = pcRefPic;
  Pel* pcCff = pcCoeffs;

  // Q-changes
  Int iQDiffRowCausal     = iQRowCausal - 6;
  Int iQDiffRowAntiCausal = iQRowAntiCausal - iQRowCausal;

  // filter along rows
  for (iY=0; iY<iYMax; iY++)
  {
    // init causal coeff
    pcCff[0] = xInitRowCausalCoeff( pcSrc, iSrcShift, iXMax, i16Q15Z ) << iQDiffRowCausal;

    // causal filtering
    Pel cTmp = *pcCff;
    for (iX=1; iX<iXMax; iX++)
    {
      cTmp = Pel( pcSrc[iX] << (iSrcShift+iQDiffRowCausal) ) + xMult16x16( cTmp, i16Q15Z );
      pcCff[iX] = cTmp;
    }

    // init anti-causal coeff
    pcCff[iXMax-1] = xMult16x16( xMult16x16( pcCff[iXMax-2], i16Q15Z ) + pcCff[iXMax-1], i16Q15Z_c<<iQDiffRowAntiCausal );

    // anti-causal filtering
    for (iX=iXMax-2; iX>=0; iX--)
    {
      pcCff[iX] = xMult16x16( (pcCff[iX+1]>>iQDiffRowAntiCausal) - pcCff[iX], i16Q15Z<<iQDiffRowAntiCausal );
    }
    pcSrc += iStride;
    pcCff += iStride;
  }

  // Q-changes
  Int iQDiffColCausal     = iQColCausal - iQRowAntiCausal;
  Int iQDiffColAntiCausal = iQColAntiCausal - iQColCausal;

  // filter along columns
  for (iX=0; iX<iXMax; iX++)
  {
    pcCff = pcCoeffs + iX;

    // init causal coeff
    pcCff[0] = xInitColCausalCoeff( pcCff, iStride, iYMax, i16Q15Z ) << iQDiffColCausal;

    // causal filtering
    Pel cTmp = *pcCff;
    for (iY=1; iY<iYMax; iY++)
    {
      pcCff  += iStride;
      cTmp    = (*pcCff << iQDiffColCausal) + xMult16x16( cTmp, i16Q15Z );
      *pcCff  = cTmp;
    }

    // init anti-causal coeff
    cTmp   = xMult16x16( xMult16x16( *(pcCff-iStride), i16Q15Z ) + cTmp, i16Q15Z_c<<iQDiffColAntiCausal );
    *pcCff = cTmp;

    // anti-causal filtering
    for (iY=iYMax-2; iY>=0; iY--)
    {
      pcCff -= iStride;
      cTmp = xMult16x16( (cTmp>>iQDiffColAntiCausal) - *pcCff, i16Q15Z<<iQDiffColAntiCausal );
      *pcCff = cTmp;
    }
  }
}

// internal function for initialization
Pel TComExpCoeff6TapMOMS::xInitRowCausalCoeff( const Pel* pcSrc, const Int iSrcShift, const Int N, const Pel i16Q15Z )
{
  Pel i16Q15Z_n = i16Q15Z;

  // only sum up 16 values
  const Int iXMax   = (N > 16) ? 16 : N;
  Int iSum          = Int( pcSrc[0] << (iSrcShift+15) );

  for ( Int iX = 1; iX < iXMax; iX++ )
  {
    iSum      += Int( i16Q15Z_n ) * Int( pcSrc[iX] << iSrcShift );
    i16Q15Z_n  = xMult16x16    ( i16Q15Z_n, i16Q15Z );
  }

  Pel cSum = (iSum + (1<<14)) >> 15;
  return cSum;
}

// internal function for initialization
Pel TComExpCoeff6TapMOMS::xInitColCausalCoeff( const Pel* pcSrc, Int iStride, Int N, const Pel i16Q15Z )
{
  Pel i16Q15Z_n = i16Q15Z;

  // only sum up 16 values
  const Int iYMax   = (N > 16) ? 16 : N;
  Int iSum          = Int( pcSrc[0] << 15 );

  for ( Int iY = 1; iY < iYMax; iY++ )
  {
    pcSrc     += iStride;
    iSum      += Int( i16Q15Z_n ) * Int ( *pcSrc );
    i16Q15Z_n  = xMult16x16    ( i16Q15Z_n, i16Q15Z );
  }

  Pel cSum = (iSum + (1<<14)) >> 15;
  return cSum;
}


// table for 1/8th sample interpolation
const Pel TComPredFilter6TapMOMS::sm_cFilterTable[8][6]=
{
  { 440,      7422,    17044,     7422,       440,         0, },
  { 253,      5904,    16832,     9052,       726,         1, },
  { 137,      4551,    16207,    10731,      1138,         4, },
  {  69,      3397,    15208,    12378,      1704,        12, },
  {  31,      2451,    13902,    13902,      2451,        31, },
  {  12,      1704,    12378,    15208,      3397,        69, },
  {   4,      1138,    10731,    16207,      4551,       137, },
  {   1,       726,     9052,    16832,      5904,       253, },
};

// main interpolation function
Void TComPredFilter6TapMOMS::interpolate( Pel* pcDst, Int iDstStride, Pel* pcSrc, Int iSrcStride, Int iDstYMax, Int iDstXMax, Int iDx, Int iDy, Int iDstStep )
{
  // temp buffer
  Pel* pcTmpBufOrg         = new Pel[iDstXMax*(iDstYMax+5)];
  const Int iQ6Gain        = Int( pow((63360.0/850.0),2) * pow(2.0,6) + 0.5 );

  // interpolation in horizontal direction
  {
    Pel* piTmp             = pcTmpBufOrg;
    const Int iTmpStride   = iDstXMax;
    const Pel* piSrcLn     = pcSrc - 2*iSrcStride - 2;

    // 6-tap values
    const Int f0           = sm_cFilterTable[iDx][0];
    const Int f1           = sm_cFilterTable[iDx][1];
    const Int f2           = sm_cFilterTable[iDx][2];
    const Int f3           = sm_cFilterTable[iDx][3];
    const Int f4           = sm_cFilterTable[iDx][4];
    const Int f5           = sm_cFilterTable[iDx][5];

    for ( Int iY = 0; iY < iDstYMax + 5; iY++ )
    {
      for ( Int iX = 0; iX < iDstXMax; iX++ )
      {
        Int iSum;
        const Pel* piSrc = piSrcLn + iX;

        // 6-tap filter
        iSum  = f0 * piSrc[0];
        iSum += f1 * piSrc[1];
        iSum += f2 * piSrc[2];
        iSum += f3 * piSrc[3];
        iSum += f4 * piSrc[4];
        iSum += f5 * piSrc[5];

        // round & store result
        piTmp[iX] = Pel( ( iSum + (1<<14) ) >> 15 );
      }
      piTmp += iTmpStride;
      piSrcLn += iSrcStride;
    }
  }

  // interpolation in vertical direction
  {
    Pel* piDst             = pcDst;
    const Int iTmpStride   = iDstXMax;
    const Pel* piTmpLn     = pcTmpBufOrg;

    // 6-tap values
    const Int f0           = sm_cFilterTable[iDy][0];
    const Int f1           = sm_cFilterTable[iDy][1];
    const Int f2           = sm_cFilterTable[iDy][2];
    const Int f3           = sm_cFilterTable[iDy][3];
    const Int f4           = sm_cFilterTable[iDy][4];
    const Int f5           = sm_cFilterTable[iDy][5];
    const Int iF           = (9+13-g_uiBitIncrement);

    for ( Int iY = 0; iY < iDstYMax; iY++ )
    {
      for ( Int iX = 0; iX < iDstXMax; iX++ )
      {
        Int iSum;
        const Pel* piTmp   = piTmpLn + iX;

        // 6-tap filter
        iSum  = f0 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f1 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f2 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f3 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f4 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f5 * ( *piTmp );

        iSum  = ( iSum + (1 << 11) ) >> 12;
        iSum  = iQ6Gain * iSum;

        // round & store result
        piDst[iDstStep*iX] = Clip( Pel( ( iSum + (1<<(iF-1)) ) >> iF ) );
      }
      piDst   += iDstStep*iDstStride;
      piTmpLn += iTmpStride;
    }
  }

  delete[] pcTmpBufOrg;
}


// table for 1/8th sample interpolation
const Pel TComPredFilter4TapMOMS::sm_cFilterTable[8][4]=
{
  { 8192,   26624,    8192,      0, },
  { 5698,   26378,   10790,    142, },
  { 3792,   25040,   13808,    368, },
  { 2390,   22862,   16994,    762, },
  { 1408,   20096,   20096,   1408, },
  {  762,   16994,   22862,   2390, },
  {  368,   13808,   25040,   3792, },
  {  142,   10790,   26378,   5698, },
};


// main interpolation function
Void TComPredFilter4TapMOMS::interpolate( Pel* pcDst, Int iDstStride, Pel* pcSrc, Int iSrcStride, Int iDstYMax, Int iDstXMax, Int iDx, Int iDy, Int iDstStep )
{
  // temp buffer
  Pel* pcTmpBufOrg         = new Pel[iDstXMax*(iDstYMax+3)];

  // interpolation in horizontal direction
  {
    Pel* piTmp             = pcTmpBufOrg;
    const Int iTmpStride   = iDstXMax;
    const Pel* piSrcLn     = pcSrc - iSrcStride - 1;

    // 4-tap values
    const Int f0           = sm_cFilterTable[iDx][0];
    const Int f1           = sm_cFilterTable[iDx][1];
    const Int f2           = sm_cFilterTable[iDx][2];
    const Int f3           = sm_cFilterTable[iDx][3];

    for ( Int iY = 0; iY < iDstYMax + 3; iY++ )
    {
      for ( Int iX = 0; iX < iDstXMax; iX++ )
      {
        Int iSum;
        const Pel* piSrc = piSrcLn + iX;

        // 4-tap filter
        iSum  = f0 * piSrc[0];
        iSum += f1 * piSrc[1];
        iSum += f2 * piSrc[2];
        iSum += f3 * piSrc[3];

        // round & store result
        piTmp[iX] = Pel( ( iSum + (1<<12) ) >> 13 );
      }
      piTmp += iTmpStride;
      piSrcLn += iSrcStride;
    }
  }

  // interpolation in vertical direction
  {
    Pel* piDst             = pcDst;
    const Int iTmpStride   = iDstXMax;
    const Pel* piTmpLn     = pcTmpBufOrg;

    // 4-tap values
    const Int f0           = sm_cFilterTable[iDy][0];
    const Int f1           = sm_cFilterTable[iDy][1];
    const Int f2           = sm_cFilterTable[iDy][2];
    const Int f3           = sm_cFilterTable[iDy][3];

    for ( Int iY = 0; iY < iDstYMax; iY++ )
    {
      for ( Int iX = 0; iX < iDstXMax; iX++ )
      {
        Int iSum;
        const Pel* piTmp = piTmpLn + iX;

        // 4-tap filter
        iSum  = f0 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f1 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f2 * ( *piTmp ); piTmp += iTmpStride;
        iSum += f3 * ( *piTmp );

        // round & store result
        piDst[iDstStep*iX] = Clip( Pel( ( iSum + (1<<((13+6-g_uiBitIncrement)-1)) ) >> ((13+6-g_uiBitIncrement)) ) );
      }
      piDst   += iDstStep*iDstStride;
      piTmpLn += iTmpStride;
    }
  }

  delete[] pcTmpBufOrg;
}


// convert image samples to expansion coefficients
Void TComExpCoeff4TapMOMS::calcExpCoeffs( TComPicYuv *pcSrc, TComPicYuv *pcDst )
{
  xCalcCoeffsPlane( pcSrc->getLumaAddr(), pcDst->getLumaAddr(), pcDst->getHeight(),      pcDst->getWidth(),       pcDst->getStride() );
  xCalcCoeffsPlane( pcSrc->getCbAddr(),   pcDst->getCbAddr(),   pcDst->getHeight() >> 1, pcDst->getWidth() >> 1, pcDst->getCStride() );
  xCalcCoeffsPlane( pcSrc->getCrAddr(),   pcDst->getCrAddr(),   pcDst->getHeight() >> 1, pcDst->getWidth() >> 1, pcDst->getCStride() );
}


// internal function for initialization
Pel TComExpCoeff4TapMOMS::xInitRowCausalCoeff( const Pel* pcSrc,  Int N )
{
  Pel i16Q15Z_n = m_i16Q15Z1;

  // only sum up 16 values
  const Int iXMax   = (N > 16) ? 16 : N;
  Int iSum          = Int( pcSrc[0] << (m_iShift_IBDI+15) );

  for ( Int iX = 1; iX < iXMax; iX++ )
  {
    iSum      += Int( i16Q15Z_n ) * Int( pcSrc[iX] << m_iShift_IBDI );
    i16Q15Z_n  = xMult16x16    ( i16Q15Z_n, m_i16Q15Z1 );
  }

  Pel cSum = (iSum + (1<<14)) >> 15;
  return cSum;
}


// internal function for initialization
Pel TComExpCoeff4TapMOMS::xInitColCausalCoeff( const Pel* pcSrc, Int iStride, Int N )
{
  Pel i16Q15Z_n = m_i16Q15Z1;

  // only sum up 16 values
  const Int iYMax   = (N > 16) ? 16 : N;
  Int iSum          = Int( pcSrc[0] << 15 );

  for ( Int iY = 1; iY < iYMax; iY++ )
  {
    pcSrc     += iStride;
    iSum      += Int( i16Q15Z_n ) * Int ( *pcSrc );
    i16Q15Z_n  = xMult16x16    ( i16Q15Z_n, m_i16Q15Z1 );
  }

  Pel cSum = (iSum + (1<<14)) >> 15;
  return cSum;
}


// convert image samples to expansion coefficients
Void TComExpCoeff4TapMOMS::xCalcCoeffsPlane( Pel* pcRefPic, Pel* pcCoeffs, Int iYMax, Int iXMax, Int iStride )
{
  Int iY, iX;

  Pel* pcSrc = pcRefPic;
  Pel* pcCff = pcCoeffs;

  // filter along rows
  for (iY=0; iY<iYMax; iY++)
  {
    // init causal coeff
    pcCff[0] = xInitRowCausalCoeff( pcSrc, iXMax );

    // causal filtering
    Pel cTmp = *pcCff;
    for (iX=1; iX<iXMax; iX++)
    {
      cTmp = Pel( pcSrc[iX] << m_iShift_IBDI ) + xMult16x16( cTmp, m_i16Q15Z1 );
      pcCff[iX] = cTmp;
    }

    // init anti-causal coeff
    pcCff[iXMax-1] = xMult16x16( xMult16x16( pcCff[iXMax-2], m_i16Q15Z1 ) + pcCff[iXMax-1], m_i16Q15Z1_c );

    // anti-causal filtering
    for (iX=iXMax-2; iX>=0; iX--)
    {
      pcCff[iX] = xMult16x16( pcCff[iX+1] - pcCff[iX], m_i16Q15Z1 );
    }
    pcSrc += iStride;
    pcCff += iStride;
  }

  // filter along columns
  for (iX=0; iX<iXMax; iX++)
  {
    pcCff = pcCoeffs + iX;

    // init causal coeff
    pcCff[0] = xInitColCausalCoeff( pcCff, iStride, iYMax );

    // causal filtering
    Pel cTmp = *pcCff;
    for (iY=1; iY<iYMax; iY++)
    {
      pcCff  += iStride;
      cTmp    = *pcCff + xMult16x16( cTmp, m_i16Q15Z1 );
      *pcCff  = cTmp;
    }

    // init anti-causal coeff
    cTmp   = xMult16x16( xMult16x16( *(pcCff-iStride), m_i16Q15Z1 ) + cTmp, m_i16Q15Z1_c );
    *pcCff = cTmp;

    // anti-causal filtering
    for (iY=iYMax-2; iY>=0; iY--)
    {
      pcCff -= iStride;
      cTmp = xMult16x16( cTmp - *pcCff, m_i16Q15Z1 );
      *pcCff = cTmp;
    }
  }
}

#endif /* HHI_INTERP_FILTER */
