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


/** \file     TComPredFilterMOMS.h
    \brief    MOMS interpolation filter (header)
*/

#ifndef TCOMPREDFILTERMOMS_H_
#define TCOMPREDFILTERMOMS_H_

#include "TComYuv.h"
#include "TComPic.h"
#include "TComMotionInfo.h"
#include "TComPattern.h"

#if HHI_INTERP_FILTER

// interface class for interpolation
class InterpolationIf
{
public:

  InterpolationIf()          {}
  virtual ~InterpolationIf() {}

  virtual Void interpolate   ( Pel* pcDst, Int iDstStride, Pel* pcSrc, Int iSrcStride, Int iDstYMax, Int iDstXMax, Int iDx, Int iDy, Int iDstStep ) = 0;
};


class TComPredFilter6TapMOMS : public InterpolationIf
{
public:

  TComPredFilter6TapMOMS()          {}
  virtual ~TComPredFilter6TapMOMS() {}

  Void interpolate            ( Pel* pcDst, Int iDstStride, Pel* pcSrc, Int iSrcStride, Int iDstYMax, Int iDstXMax, Int iDx, Int iDy, Int iDstStep );

private:
  static const Pel            sm_cFilterTable[8][6];    // table for 1/8th sample interpolation
};


class TComPredFilter4TapMOMS : public InterpolationIf
{
public:

  TComPredFilter4TapMOMS()          {}
  virtual ~TComPredFilter4TapMOMS() {}

  Void interpolate            ( Pel* pcDst, Int iDstStride, Pel* pcSrc, Int iSrcStride, Int iDstYMax, Int iDstXMax, Int iDx, Int iDy, Int iDstStep );

private:
  static const Pel            sm_cFilterTable[8][4];  // table for 1/8th sample interpolation
};


// interface class for MOMS interpolation
class TComPredFilterMOMS
{
protected:
  TComPredFilterMOMS()          { m_pcInterpIf = NULL; }
public:
  virtual ~TComPredFilterMOMS() { m_cYuvTmp.destroy(); }

  // motion compensation functions
  Void predInterLumaBlkMOMS   ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt );
  Void predInterChromaBlkMOMS ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt );
#ifdef QC_AMVRES
  Void predInterLumaBlkHAM_ME_MOMS ( Pel* piSrcY, Int iSrcStride, Pel* piDstY, Int iDstStride, TComMv* pcMv, 
                                                   Int iWidth, Int iHeight, InterpFilterType ePFilt, Int dMVx, Int dMVy);
  Void predInterLumaBlkHAM_MOMS  ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight, TComYuv*& rpcYuv, InterpFilterType ePFilt );
#endif
  // motion estimation functions
  Void extMOMSUpSamplingH     ( TComPattern* pcPattern );
  Void extMOMSUpSamplingQ     ( TComPattern* pcPattern, TComMv cMvHalf, Pel* piDst, Int iDstStride );
  Void initTempBuff()         { m_cYuvTmp.create( ((g_uiMaxCUWidth  + 1) << 2), ((g_uiMaxCUHeight + 1) << 2) ); }
  UInt getTmpStride()         { return m_cYuvTmp.getStride();   }
  Pel* getTmpLumaAddr()       { return m_cYuvTmp.getLumaAddr(); }
  
  Void setFiltType            ( InterpFilterType eFiltType ) 
  {
    switch( eFiltType )
    {
      case IPF_HHI_6TAP_MOMS:
        m_pcInterpIf = &m_cPredFilter6Tap;
        break;

      case IPF_HHI_4TAP_MOMS:
        m_pcInterpIf = &m_cPredFilter4Tap;
        break;

      default:
        break;
    }
  }

private:
  TComYuv                     m_cYuvTmp;          // temp buffer for motion estimation
  InterpolationIf*            m_pcInterpIf;       // interface pointer 
  TComPredFilter4TapMOMS      m_cPredFilter4Tap;  // 4-tap moms
  TComPredFilter6TapMOMS      m_cPredFilter6Tap;  // 6-tap moms
};

class TComExpCoeff6TapMOMS 
{
public:

  TComExpCoeff6TapMOMS()  :   m_iShift_IBDI   ( (6-g_uiBitIncrement) ), 
#if HHI_INTERP_FILTER_KERNEL_FIX
    m_i16Q15Z1      ( Pel( -0.583910163218787*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z1_c    ( Pel(  0.885989103928779*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z2      ( Pel( -0.094330857473743*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z2_c    ( Pel(  0.095177778919615*pow(2.0,15)+0.5 ) ) {}
#else
    m_i16Q15Z1      ( Pel( -0.478368402923858*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z1_c    ( Pel(  0.620320200316728*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z2      ( Pel( -0.070202438211750*pow(2.0,15)+0.5 ) ), 
    m_i16Q15Z2_c    ( Pel(  0.070550136256718*pow(2.0,15)+0.5 ) ) {}
#endif
  
  virtual ~TComExpCoeff6TapMOMS() {}

  Void calcExpCoeffs          ( TComPicYuv *pcSrc, TComPicYuv *pcDst );

private:

  Void xCalcCoeffsPlane       ( Pel* pcRefPic, Pel* pcCoeffs, Int iYMax, Int iXMax, Int iStride );

  Pel  xInitRowCausalCoeff    ( const Pel* pcSrc, const Int iSrcShift, const Int N, const Pel i16Q15Z );
  Pel  xInitColCausalCoeff    ( const Pel* pcSrc, Int iStride, Int N, const Pel i16Q15Z );
  Void xFiltIter              ( Pel* pcRefPic, Pel* pcCoeffs, const Pel i16Q15Z, const Pel i16Q15Z_c,
    const Int iYMax, const Int iXMax, const Int iStride, const Int iSrcShift,
    const Int iQRowCausal, const Int iQRowAntiCausal, 
    const Int iQColCausal, const Int iQColAntiCausal );

  __inline Pel xMult16x16     ( Pel i16A, Pel i16B )  { return Pel( ( ( Int(i16A) * Int(i16B) ) + (1<<14) ) >> 15 ); }

  const Int                   m_iShift_IBDI;
  const Pel                   m_i16Q15Z1;
  const Pel                   m_i16Q15Z1_c;
  const Pel                   m_i16Q15Z2;
  const Pel                   m_i16Q15Z2_c;
};

class TComExpCoeff4TapMOMS
{
public:

#if HHI_INTERP_FILTER_KERNEL_FIX
  TComExpCoeff4TapMOMS()  : m_iShift_IBDI( (6-g_uiBitIncrement) ), m_i16Q15Z1( -12854 ), m_i16Q15Z1_c( 15192 ) {}
#else
  TComExpCoeff4TapMOMS()  : m_iShift_IBDI( (6-g_uiBitIncrement) ), m_i16Q15Z1( -11276 ), m_i16Q15Z1_c( 12791 ) {}
#endif
  virtual ~TComExpCoeff4TapMOMS() {}

  Void calcExpCoeffs          ( TComPicYuv *pcSrc, TComPicYuv *pcDst );

private:

  Pel  xInitRowCausalCoeff    ( const Pel* pcSrc, Int N );
  Pel  xInitColCausalCoeff    ( const Pel* pcSrc, Int iStride, Int N );
  Void xCalcCoeffsPlane       ( Pel* pcRefPic, Pel* pcCoeffs, Int iYMax, Int iXMax, Int iStride );

  __inline Pel xMult16x16     ( Pel i16A, Pel i16B )  { return Pel( ( ( Int(i16A) * Int(i16B) ) + (1<<14) ) >> 15 ); }

  const Int                   m_iShift_IBDI;
  const Pel                   m_i16Q15Z1;
  const Pel                   m_i16Q15Z1_c;
};

// interface class for coefficient calculation
class TComCoeffCalcMOMS
{
public:
  TComCoeffCalcMOMS() {}
  virtual ~TComCoeffCalcMOMS() {}
  Void calcCoeffs  ( TComPicYuv *pcSrc, TComPicYuv *pcDst, Int iFiltType )
  {
    switch( iFiltType )
    {
    case IPF_HHI_4TAP_MOMS:
      m_cCoeff4Tap.calcExpCoeffs( pcSrc, pcDst );
      break;
    case IPF_HHI_6TAP_MOMS:
      m_cCoeff6Tap.calcExpCoeffs( pcSrc, pcDst );
      break;
    default:
      break;
    }
  }
private:
  TComExpCoeff4TapMOMS m_cCoeff4Tap;
  TComExpCoeff6TapMOMS m_cCoeff6Tap;
};

#endif /* HHI_INTERP_FILTER */

#endif /* TCOMPREDFILTERMOMS_H_ */
