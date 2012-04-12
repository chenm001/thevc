/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TEncRateCtrl.h
    \brief    Rate control manager class
*/

#ifndef _HM_TENCRATECTRL_H_
#define _HM_TENCRATECTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComDataCU.h"

#include <vector>
#include <algorithm>

using namespace std;

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================
#define MAX_DELTA_QP    2
#define MAX_CUDQP_DEPTH 0 

typedef struct FrameData
{
  Bool       m_bReferenced;
  Int        m_iQP;
  Int        m_iBits;
  Double     m_dMAD;
}FrameData;

typedef struct LCUData
{
  Int     m_iQP;            ///<  coded QP
  Int     m_iBits;          ///<  actually generated bits
  Int     m_iPixels;        ///<  number of pixels for a unit
  Int     m_iWidthInPixel;  ///<  number of pixels for width
  Int     m_iHeightInPixel; ///<  number of pixels for height
  Double  m_dMAD;           ///<  texture complexity for a unit
}LCUData;

class MADLinearModel
{
private:
  Bool   m_bActive;
  Double m_dY1;
  Double m_dY2;
  Double m_adMAD[3];

public:
  MADLinearModel ()   {};
  ~MADLinearModel()   {};
  
  Void    initMADLinearModel      ();
  Double  getMAD                  ();
  Void    updateMADLiearModel     ();
  Void    updateMADHistory        (Double dMAD);
  Bool    IsUpdateAvailable       ()              { return m_bActive; }
};

class PixelBaseURQQuadraticModel
{
private:
  Double m_dX1_HIGH;
  Double m_dX2_HIGH;
  Double m_dX1_LOW;
  Double m_dX2_LOW;
public:
  PixelBaseURQQuadraticModel () {};
  ~PixelBaseURQQuadraticModel() {};

  Void    initPixelBaseQuadraticModel       ();
  Int     getQP                             (Int iQP, Int iTargetBits, Int iNumberOfPixels, Double dMADpred);
  Void    updatePixelBasedURQQuadraticModel (Int iQP, Int iBits, Int iNumberOfPixels, Double dMAD);
  Bool    checkUpdateAvailable              (Int iReferenceQP );
  Double  xConvertQP2QStep                  (Int QP );
  Int     xConvertQStep2QP                  (Double QStep );
};

class TEncRateCtrl
{
private:
  Bool            m_bIsLowdelay;
  Int             m_iPrevBitrate;
  Int             m_iCurrBitrate;
  Int             m_iFrameRate;
  Int             m_iRefFrameNum;
  Int             m_iNonRefFrameNum;
  Int             m_iNumofPixels;
  Int             m_iSourceWidthInLCU;
  Int             m_iSourceHeightInLCU;      
  Int             m_iGOPSize;
  Int             m_iNumofGOP;
  Int             m_iNumofFrame;
  Int             m_iNumofLCU;
  Int             m_iNumofUnit;
  Int             m_iNumofRefFrame;
  Int             m_iNumofNonRefFrame;
  Int             m_iCurrFrmIdx;
  Int             m_iPrevFrmIdx;
  Int             m_iOccupancyVB;
  Int             m_iInitialOVB;
  Int             m_iTargetBufferLevel;
  Int             m_iInitialTBL;
  Int             m_iRemainingBitsInGOP;
  Int             m_iRemainingBitsInFrame;
  Int             m_iOccupancyVBInFrame;
  Int             m_iTargetBits;
  Int             m_iNumUnitInFrame;
  Int             m_iCodedPixels;
  Bool            m_bUnitLevelActive;
  Double          m_dNonRefAvgWeighting;
  Double          m_dRefAvgWeighting;
  Double          m_dAvgbpp;         
  
  FrameData*      m_pcFrameData;
  LCUData*        m_pcLCUData;

  MADLinearModel              m_cMADLinearModel;
  PixelBaseURQQuadraticModel  m_cPixelURQQuadraticModel;
  
public:
  TEncRateCtrl         () {};
  virtual ~TEncRateCtrl() {};

  Void          create                (Int iIntraPeriod, Int iGOPSize, Int iFrameRate, Int iTargetKbps, Int iQP, Int iLCUNumInBasicUnit, Int iSourceWidth, Int iSourceHeight, Int iMaxCUWidth, Int iMaxCUHeight);
  Void          destroy               ();

  Void          initFrameData         (Int iQP = 0);
  Void          initUnitData          (Int iQP = 0);
  Int           getFrameQP            (Bool bReferenced, Int iPOC);
  Bool          calculateUnitQP       ();
  Int           getUnitQP             ()                                          { return m_pcLCUData[m_iNumofLCU].m_iQP;  }
  Void          updateRCGOPStatus     ();
  Void          updataRCFrameStatus   (Int iFrameBits, SliceType eSliceType);
  Void          updataRCUnitStatus    ();
  Void          updateLCUData         (TComDataCU* pcCU, UInt64 uiBits, Int iQP);
  Void          updateFrameData       (UInt64 uiBits);
  Double        xAdjustmentBits       (Int& iReductionFact, Int& iCompensationFact);
  Int           getGOPId              ()                                          { return m_iNumofFrame; }
};
#endif


