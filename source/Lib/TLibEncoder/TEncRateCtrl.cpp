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

/** \file     TEncRateCtrl.cpp
    \brief    Rate control manager class
*/
#include "TEncRateCtrl.h"
#include "../TLibCommon/TComPic.h"

#include <cmath>

using namespace std;

#define ADJUSTMENT_FACTOR       0.60
#define HIGH_QSTEP_THRESHOLD    9.5238
#define HIGH_QSTEP_ALPHA        4.9371
#define HIGH_QSTEP_BETA         0.0922
#define LOW_QSTEP_ALPHA         16.7429
#define LOW_QSTEP_BETA          -1.1494

#define MAD_PRED_Y1             1.0
#define MAD_PRED_Y2             0.0

enum MAD_HISOTRY {
  MAD_PPPrevious = 0,
  MAD_PPrevious  = 1,
  MAD_Previous   = 2
};

Void    MADLinearModel::initMADLinearModel()
{
  m_bActive = false;
  m_dY1 = 1.0;
  m_dY2 = 0.0;
  m_adMAD[0] = m_adMAD[1] = m_adMAD[3] = 0.0;
}

Double  MADLinearModel::getMAD()
{
  Double dMADpred = m_dY1 * m_adMAD[MAD_Previous] + m_dY2;

  if(dMADpred < 0)
  {
    dMADpred = m_adMAD[MAD_Previous];
    m_dY1 = MAD_PRED_Y1;
    m_dY2 = MAD_PRED_Y2;
  } 
  return dMADpred;
}

Void    MADLinearModel::updateMADLiearModel()
{
  Double dNewY1         = ((m_adMAD[MAD_Previous] - m_adMAD[MAD_PPrevious]) / (m_adMAD[MAD_PPrevious] - m_adMAD[MAD_PPPrevious]));
  Double dNewY2         =  (m_adMAD[MAD_Previous] - (dNewY1*m_adMAD[MAD_PPrevious]));
  
  m_dY1 = 0.70+0.20*m_dY1+ 0.10*dNewY1;
  m_dY2 =      0.20*m_dY2+ 0.10*dNewY2;
}

Void    MADLinearModel::updateMADHistory(Double dMAD)
{
  m_adMAD[MAD_PPPrevious] = m_adMAD[MAD_PPrevious];
  m_adMAD[MAD_PPrevious ] = m_adMAD[MAD_Previous ];
  m_adMAD[MAD_Previous  ] = dMAD;
  m_bActive = (m_adMAD[MAD_Previous  ] && m_adMAD[MAD_PPrevious ] && m_adMAD[MAD_PPPrevious]);
}


Void    PixelBaseURQQuadraticModel::initPixelBaseQuadraticModel()
{
  m_dX1_HIGH = HIGH_QSTEP_ALPHA;
  m_dX2_HIGH = HIGH_QSTEP_BETA;
  m_dX1_LOW  = LOW_QSTEP_ALPHA;
  m_dX2_LOW  = LOW_QSTEP_BETA;
}

Int     PixelBaseURQQuadraticModel::getQP(Int iQP, Int iTargetBits, Int iNumberOfPixels, Double dMADpred)
{
  Double dQStep;
  Double bppPerMAD = (Double)(iTargetBits/(iNumberOfPixels*dMADpred));
  
  if(xConvertQP2QStep(iQP) >= HIGH_QSTEP_THRESHOLD)
  {
    dQStep = 1/( sqrt((bppPerMAD/m_dX1_HIGH)+((m_dX2_HIGH*m_dX2_HIGH)/(4*m_dX1_HIGH*m_dX1_HIGH*m_dX1_HIGH))) - (m_dX2_HIGH/(2*m_dX1_HIGH)));
  }
  else
  {
    dQStep = 1/( sqrt((bppPerMAD/m_dX1_LOW)+((m_dX2_LOW*m_dX2_LOW)/(4*m_dX1_LOW*m_dX1_LOW*m_dX1_LOW))) - (m_dX2_LOW/(2*m_dX1_LOW)));
  }
  
  return xConvertQStep2QP(dQStep);
}

Void    PixelBaseURQQuadraticModel::updatePixelBasedURQQuadraticModel(Int iQP, Int iBits, Int iNumberOfPixels, Double dMAD)
{
  Double dQStep     = xConvertQP2QStep(iQP);
  Double dInvQStep = (1/dQStep);
  Double dNewBeta, dNewAlpha;
  
  if(dQStep < HIGH_QSTEP_THRESHOLD)
  {
    dNewBeta  = (((iBits/(iNumberOfPixels*dMAD))-(23.3772*dInvQStep*dInvQStep))/((1-200*dInvQStep)*dInvQStep));
    dNewAlpha = (23.3772-200*dNewBeta);
    m_dX1_HIGH = 0.70*HIGH_QSTEP_ALPHA + 0.20 * m_dX1_HIGH + 0.10 * dNewAlpha;
    m_dX2_HIGH = 0.70*HIGH_QSTEP_BETA  + 0.20 * m_dX2_HIGH + 0.10 * dNewBeta;
  }
  else
  {
    dNewBeta  = (((iBits/(iNumberOfPixels*dMAD))-(5.8091*dInvQStep*dInvQStep))/((1-9.5455*dInvQStep)*dInvQStep));
    dNewAlpha = (5.8091-9.5455*dNewBeta);
    m_dX1_LOW = 0.90*LOW_QSTEP_ALPHA + 0.09 * m_dX1_LOW + 0.01 * dNewAlpha;
    m_dX2_LOW = 0.90*LOW_QSTEP_BETA  + 0.09 * m_dX2_LOW + 0.01 * dNewBeta;
  }
}

Bool    PixelBaseURQQuadraticModel::checkUpdateAvailable(Int iReferenceQP )
{ 
  Double dCheckQStep = xConvertQP2QStep(iReferenceQP);

  if (dCheckQStep > xConvertQP2QStep(MAX_QP) 
    ||dCheckQStep < xConvertQP2QStep(MIN_QP) )
  {
    return false;
  }

  return true;
}

Double  PixelBaseURQQuadraticModel::xConvertQP2QStep( Int QP )
{
  Int i;
  Double QStep;
  static const Double QP2QSTEP[6] = { 0.625, 0.703, 0.797, 0.891, 1.000, 1.125 };

  QStep = QP2QSTEP[QP % 6];
  for( i=0; i<(QP/6); i++)
  {
    QStep *= 2;
  }

  return QStep;
}

Int     PixelBaseURQQuadraticModel::xConvertQStep2QP( Double dQStep )
{
  Int iPer = 0, iRem = 0;

  if( dQStep < xConvertQP2QStep(MIN_QP))
  {
    return MIN_QP;
  }
  else if (dQStep > xConvertQP2QStep(MAX_QP) )
  {
    return MAX_QP;
  }

  while( dQStep > xConvertQP2QStep(5) )
  {
    dQStep /= 2.0;
    iPer++;
  }

  if (dQStep <= 0.625)
  {
    iRem = 0;
  }
  else if (dQStep <= 0.703)
  {
    iRem = 1;
  }
  else if (dQStep <= 0.797)
  {
    iRem = 2;
  }
  else if (dQStep <= 0.891)
  {
    iRem = 3;
  }
  else if (dQStep <= 1.000)
  {
    iRem = 4;
  }
  else
  {
    iRem = 5;
  }
  return (iPer * 6 + iRem);
}


Void  TEncRateCtrl::create(Int iIntraPeriod, Int iGOPSize, Int iFrameRate, Int iTargetKbps, Int iQP, Int iLCUNumInBasicUnit, Int iSourceWidth, Int iSourceHeight, Int iMaxCUWidth, Int iMaxCUHeight)
{
  Int iLeftInHeight, iLeftInWidth;
  Int iUnitAddr               = 0;
  m_iSourceWidthInLCU         = (iSourceWidth  / iMaxCUWidth) + (( iSourceWidth  %  iMaxCUWidth ) ? 1 : 0);
  m_iSourceHeightInLCU        = (iSourceHeight / iMaxCUHeight)+ (( iSourceHeight %  iMaxCUHeight) ? 1 : 0);  

  m_bIsLowdelay               = (iIntraPeriod == -1) ? true : false;
  m_iPrevBitrate              = iTargetKbps*1000;
  m_iCurrBitrate              = iTargetKbps*1000;
  m_iFrameRate                = iFrameRate;
  m_iRefFrameNum              = m_bIsLowdelay ? (iGOPSize) : (iGOPSize>>1);
  m_iNonRefFrameNum           = iGOPSize-m_iRefFrameNum;
  m_iGOPSize                  = iGOPSize;
  m_iNumofPixels              = ((iSourceWidth*iSourceHeight*3)>>1);
  m_iNumofGOP                 = 0;
  m_iNumofFrame               = 0;
  m_iNumofLCU                 = 0;
  m_iNumofUnit                = 0;
  m_iNumofRefFrame            = 0;
  m_iNumofNonRefFrame         = 0;
  m_iOccupancyVB              = 0;
  m_iInitialOVB               = 0;
  m_iTargetBufferLevel        = 0;
  m_iInitialTBL               = 0;
  m_iOccupancyVBInFrame       = 0;
  m_iRemainingBitsInGOP       = (m_iCurrBitrate*iGOPSize/m_iFrameRate);
  m_iRemainingBitsInFrame     = 0;
  m_iNumUnitInFrame           = m_iSourceWidthInLCU*m_iSourceHeightInLCU;
  m_cMADLinearModel.        initMADLinearModel();
  m_cPixelURQQuadraticModel.initPixelBaseQuadraticModel();

  m_dRefAvgWeighting          = 0.0;
  m_dNonRefAvgWeighting       = 0.0;
  m_dAvgbpp                   = 0.0;  
  m_bUnitLevelActive          = false;

  m_pcFrameData               = new FrameData   [iGOPSize+1];         initFrameData(iQP);
  m_pcLCUData                 = new LCUData     [m_iNumUnitInFrame];  initUnitData (iQP);

  for(Int iIdx=0; iIdx<m_iSourceHeightInLCU*iMaxCUHeight; iIdx+=iMaxCUHeight)  
  {
    iLeftInHeight = iSourceHeight - iIdx;
    iLeftInHeight = min(iLeftInHeight, iMaxCUHeight);
    for(Int jIdx=0; jIdx<m_iSourceWidthInLCU*iMaxCUWidth; jIdx+=iMaxCUWidth)
    {
      iLeftInWidth = iSourceWidth - jIdx;
      iLeftInWidth = min(iLeftInWidth, iMaxCUWidth);
      m_pcLCUData[iUnitAddr  ].m_iWidthInPixel = iLeftInWidth;
      m_pcLCUData[iUnitAddr  ].m_iHeightInPixel= iLeftInHeight;
      m_pcLCUData[iUnitAddr++].m_iPixels     = ((iLeftInHeight*iLeftInWidth*3)>>1);
    }
  }
}

Void  TEncRateCtrl::destroy()
{
  if(m_pcFrameData)
  {
    delete [] m_pcFrameData;
    m_pcFrameData = NULL;
  }
  if(m_pcLCUData)
  {
    delete [] m_pcLCUData;
    m_pcLCUData = NULL;
  }
}

Void  TEncRateCtrl::initFrameData   (Int iQP)
{
  for(Int iIdx = 0 ; iIdx <= m_iGOPSize; iIdx++)
  {
    m_pcFrameData[iIdx].m_bReferenced = false;
    m_pcFrameData[iIdx].m_dMAD        = 0.0;
    m_pcFrameData[iIdx].m_iBits       = 0;
    m_pcFrameData[iIdx].m_iQP         = iQP;
  }
}

Void  TEncRateCtrl::initUnitData    (Int iQP)
{
  for(Int iIdx = 1 ; iIdx < m_iNumUnitInFrame; iIdx++)
  {
    m_pcLCUData[iIdx].m_iQP            = iQP;
    m_pcLCUData[iIdx].m_iBits          = 0;
    m_pcLCUData[iIdx].m_iPixels        = 0;
    m_pcLCUData[iIdx].m_iWidthInPixel  = 0;
    m_pcLCUData[iIdx].m_iHeightInPixel = 0;
    m_pcLCUData[iIdx].m_dMAD           = 0.0;
  }
}

Int  TEncRateCtrl::getFrameQP(Bool bReferenced, Int iPOC)
{
  Int iNumofReferenced = 0;
  Int iFinalQP = 0;
  FrameData* pcFrameData;

  m_iCurrFrmIdx = (iPOC%m_iGOPSize) == 0 ? m_iGOPSize : (iPOC%m_iGOPSize);
  pcFrameData = &m_pcFrameData[m_iCurrFrmIdx];
  
  
  if(m_iNumofFrame != 0)
  {
    if(bReferenced)
    {
      Double dGamma = m_bIsLowdelay ? 0.5 : 0.25;
      Double dBeta  = m_bIsLowdelay ? 0.9 : 0.6;
      Int    iNumRemainingRefFrames = m_iRefFrameNum    - m_iNumofRefFrame;
      Int    iNumRemainingNRefFrames= m_iNonRefFrameNum - m_iNumofNonRefFrame;
      
      Double dTBitsOccupancy  = (m_iCurrBitrate/(Double)m_iFrameRate) + dGamma*(m_iTargetBufferLevel-m_iOccupancyVB - (m_iInitialOVB/(Double)m_iFrameRate));
      Double dTBitsLeftBudget = ((m_dRefAvgWeighting*m_iRemainingBitsInGOP)/((m_dRefAvgWeighting*iNumRemainingRefFrames)+(m_dNonRefAvgWeighting*iNumRemainingNRefFrames)));

      m_iTargetBits = (Int)(dBeta * dTBitsLeftBudget + (1-dBeta) * dTBitsOccupancy);
  
      if(m_iTargetBits <= 0 || m_iRemainingBitsInGOP <= 0)
      {
        iFinalQP = m_pcFrameData[m_iPrevFrmIdx].m_iQP + 2;
      }
      else
      {
        Double dMADpred      = m_cMADLinearModel.getMAD();
        Int    iQPLowerBound = m_pcFrameData[m_iPrevFrmIdx].m_iQP-2;
        Int    iQPUpperBound = m_pcFrameData[m_iPrevFrmIdx].m_iQP+2;
        iFinalQP = m_cPixelURQQuadraticModel.getQP(m_pcFrameData[m_iPrevFrmIdx].m_iQP, m_iTargetBits, m_iNumofPixels, dMADpred);
        iFinalQP = max(iQPLowerBound, min(iQPUpperBound, iFinalQP));
        m_bUnitLevelActive      = true;
        m_iRemainingBitsInFrame = m_iTargetBits;
        m_dAvgbpp               = (m_iTargetBits/(Double)m_iNumofPixels);
      }

      m_iNumofRefFrame++;
    }
    else
    {
      Int iBwdQP = m_pcFrameData[m_iCurrFrmIdx-1].m_iQP;
      Int iFwdQP = m_pcFrameData[m_iCurrFrmIdx+1].m_iQP;
       
      if( (iFwdQP+iBwdQP) == m_pcFrameData[m_iCurrFrmIdx-1].m_iQP
        ||(iFwdQP+iBwdQP) == m_pcFrameData[m_iCurrFrmIdx+1].m_iQP)
      {
        iFinalQP = (iFwdQP+iBwdQP);
      }
      else if(iBwdQP != iFwdQP)
      {
        iFinalQP = ((iBwdQP+iFwdQP+2)>>1);
      }
      else
      {
        iFinalQP = iBwdQP+2;
      }
      m_iNumofNonRefFrame++;
    }
  }
  else
  {
    Int iLastQPminus2 = m_pcFrameData[0].m_iQP - 2;
    Int iLastQPplus2  = m_pcFrameData[0].m_iQP + 2;

    for(Int iIdx = 1; iIdx <= m_iGOPSize; iIdx++)
    {
      if(m_pcFrameData[iIdx].m_bReferenced)
      {
        iFinalQP += m_pcFrameData[iIdx].m_iQP;
        iNumofReferenced++;
      }
    }
    
    iFinalQP = (iNumofReferenced == 0) ? m_pcFrameData[0].m_iQP : ((iFinalQP + (1<<(iNumofReferenced>>1)))/iNumofReferenced);
    iFinalQP = max( iLastQPminus2, min( iLastQPplus2, iFinalQP));

    Double dAvgFrameBits = m_iRemainingBitsInGOP/(Double)m_iGOPSize;
    Int    iBufferLevel  = m_iOccupancyVB + m_iInitialOVB;

    if(abs(iBufferLevel) > dAvgFrameBits)
    {
      if(iBufferLevel < 0)
      {
        iFinalQP -= 2;
      }
      else
      {
        iFinalQP += 2;
      }
    }
    m_iNumofRefFrame++;
  }

  iFinalQP = max(MIN_QP, min(MAX_QP, iFinalQP));

  for(Int iLCUIdx = 0 ; iLCUIdx < m_iNumUnitInFrame; iLCUIdx++)
  {
    m_pcLCUData[iLCUIdx].m_iQP = iFinalQP;
  }

  pcFrameData->m_bReferenced = bReferenced;
  pcFrameData->m_iQP         = iFinalQP;

  return iFinalQP;
}

Bool  TEncRateCtrl::calculationUnitQP     ()
{
  if(!m_bUnitLevelActive || m_iNumofLCU == 0)
  {
    return false;
  }
  Int iUpperQPBound, iLowerQPBound, iFinalQP;
  Int    iColQP        = m_pcLCUData[m_iNumofLCU].m_iQP;
  Double dColMAD       = m_pcLCUData[m_iNumofLCU].m_dMAD;
  Double dBudgetInUnit = m_pcLCUData[m_iNumofLCU].m_iPixels*m_dAvgbpp;


  Int iTargetBitsOccupancy = (Int)(dBudgetInUnit - (m_iOccupancyVBInFrame/(m_iNumUnitInFrame-m_iNumofUnit)));
  Int iTargetBitsLeftBudget= (Int)((m_iRemainingBitsInFrame*m_pcLCUData[m_iNumofLCU].m_iPixels)/(Double)(m_iNumofPixels-m_iCodedPixels));
  Int iTargetBits = (iTargetBitsLeftBudget>>1) + (iTargetBitsOccupancy>>1);
  

  if( m_iNumofLCU >= m_iSourceWidthInLCU)
  {
    iUpperQPBound = ( (m_pcLCUData[m_iNumofLCU-1].m_iQP + m_pcLCUData[m_iNumofLCU - m_iSourceWidthInLCU].m_iQP)>>1) + MAX_DELTA_QP;
    iLowerQPBound = ( (m_pcLCUData[m_iNumofLCU-1].m_iQP + m_pcLCUData[m_iNumofLCU - m_iSourceWidthInLCU].m_iQP)>>1) - MAX_DELTA_QP;
  }
  else
  {
    iUpperQPBound = m_pcLCUData[m_iNumofLCU-1].m_iQP + MAX_DELTA_QP;
    iLowerQPBound = m_pcLCUData[m_iNumofLCU-1].m_iQP - MAX_DELTA_QP;
  }

  if(iTargetBits < 0)
  {
    iFinalQP = m_pcLCUData[m_iNumofLCU-1].m_iQP + 1;
  }
  else
  {
    iFinalQP = m_cPixelURQQuadraticModel.getQP(iColQP, iTargetBits, m_pcLCUData[m_iNumofLCU].m_iPixels, dColMAD);
  }
  
  iFinalQP = max(iLowerQPBound, min(iUpperQPBound, iFinalQP));
  m_pcLCUData[m_iNumofLCU].m_iQP = max(MIN_QP, min(MAX_QP, iFinalQP));
  
  return true;
}

Void  TEncRateCtrl::updateRCGOPStatus()
{
  m_iRemainingBitsInGOP = ((m_iCurrBitrate/m_iFrameRate)*m_iGOPSize) - m_iOccupancyVB;
  m_iNumofGOP++;

  FrameData cFrameData= m_pcFrameData[m_iGOPSize];
  initFrameData();
  m_pcFrameData[0]    = cFrameData;
  m_iNumofFrame       = 0;
  m_iNumofRefFrame    = 0;
  m_iNumofNonRefFrame = 0;
}

Void  TEncRateCtrl::updataRCFrameStatus(Int iFrameBits, SliceType eSliceType)
{
  FrameData* pcFrameData = &m_pcFrameData[m_iCurrFrmIdx];
  Int iOccupancy;
  Double dAdjustmentBits;

  m_iRemainingBitsInGOP = m_iRemainingBitsInGOP + ( ((m_iCurrBitrate-m_iPrevBitrate)/m_iFrameRate)*(m_iGOPSize-m_iNumofFrame) ) - iFrameBits;
  
  iOccupancy = (Int)((Double)iFrameBits - (m_iCurrBitrate/(Double)m_iFrameRate));
  
  if( (iOccupancy < 0) && (m_iInitialOVB > 0) )
  {
    dAdjustmentBits = xAdjustmentBits(iOccupancy, m_iInitialOVB );

    if(m_iInitialOVB < 0)
    {
      dAdjustmentBits = m_iInitialOVB;
      iOccupancy     += (Int)dAdjustmentBits;
      m_iInitialOVB   =  0;
    }
  }
  else if( (iOccupancy > 0) && (m_iInitialOVB < 0) )
  {
    dAdjustmentBits = xAdjustmentBits(m_iInitialOVB, iOccupancy );
    
    if(iOccupancy < 0)
    {
      dAdjustmentBits = iOccupancy;
      m_iInitialOVB  += (Int)dAdjustmentBits;
      iOccupancy      =  0;
    }
  }

  if(m_iNumofGOP == 0)
  {
    m_iInitialOVB = iOccupancy;
  }
  else
  {
    m_iOccupancyVB= m_iOccupancyVB + iOccupancy;
  }

  if(pcFrameData->m_bReferenced)
  {
    m_dRefAvgWeighting  = ((pcFrameData->m_iBits*pcFrameData->m_iQP)/8.0) + (7.0*(m_dRefAvgWeighting)/8.0);

    if(m_iNumofFrame == 0)
    {
      m_iInitialTBL = m_iTargetBufferLevel  = (iFrameBits - (m_iCurrBitrate/m_iFrameRate));
    }
    else
    {
      Int iDistance = (m_dNonRefAvgWeighting == 0) ? 0 : 1;
      m_iTargetBufferLevel =  m_iTargetBufferLevel 
                            - (m_iInitialTBL/(m_iRefFrameNum-1)) 
                            + (Int)((m_dRefAvgWeighting*(iDistance+1)*m_iCurrBitrate)/(m_iFrameRate*(m_dRefAvgWeighting+(m_dNonRefAvgWeighting*iDistance)))) 
                            - (m_iCurrBitrate/m_iFrameRate);
    }

    if(m_cMADLinearModel.IsUpdateAvailable())
    {
      m_cMADLinearModel.updateMADLiearModel();
    }

    if(eSliceType != I_SLICE &&
       m_cPixelURQQuadraticModel.checkUpdateAvailable(pcFrameData->m_iQP))
    {
      m_cPixelURQQuadraticModel.updatePixelBasedURQQuadraticModel(pcFrameData->m_iQP, pcFrameData->m_iBits, m_iNumofPixels, pcFrameData->m_dMAD);
    }
  }
  else
  {
    m_dNonRefAvgWeighting = ((pcFrameData->m_iBits*pcFrameData->m_iQP)/8.0) + (7.0*(m_dNonRefAvgWeighting)/8.0);
  }

  m_iNumofFrame++;
  m_iNumofLCU             = 0;
  m_iNumofUnit            = 0;
  m_iOccupancyVBInFrame   = 0;
  m_iRemainingBitsInFrame = 0;
  m_iCodedPixels          = 0;
  m_bUnitLevelActive      = false;
  m_dAvgbpp               = 0.0;
}
Void  TEncRateCtrl::updataRCUnitStatus ()
{
  if(!m_bUnitLevelActive || m_iNumofLCU == 0)
  {
    return;
  }

  m_iCodedPixels  += m_pcLCUData[m_iNumofLCU-1].m_iPixels;
  m_iRemainingBitsInFrame = m_iRemainingBitsInFrame - m_pcLCUData[m_iNumofLCU-1].m_iBits;
  m_iOccupancyVBInFrame   = (Int)(m_iOccupancyVBInFrame + m_pcLCUData[m_iNumofLCU-1].m_iBits - m_pcLCUData[m_iNumofLCU-1].m_iPixels*m_dAvgbpp);

  if( m_cPixelURQQuadraticModel.checkUpdateAvailable(m_pcLCUData[m_iNumofLCU-1].m_iQP) )
  {
    m_cPixelURQQuadraticModel.updatePixelBasedURQQuadraticModel(m_pcLCUData[m_iNumofLCU-1].m_iQP, m_pcLCUData[m_iNumofLCU-1].m_iBits, m_pcLCUData[m_iNumofLCU-1].m_iPixels, m_pcLCUData[m_iNumofLCU-1].m_dMAD);
  }

  m_iNumofUnit++;
}

Void  TEncRateCtrl::updateFrameData(UInt64 uiBits)
{
  Double dMAD = 0.0;
  
  for(Int i = 0; i < m_iNumUnitInFrame; i++)
  {
    dMAD    += m_pcLCUData[i].m_dMAD;
  }
  
  m_pcFrameData[m_iCurrFrmIdx].m_dMAD   = (dMAD/(Double)m_iNumUnitInFrame);
  m_pcFrameData[m_iCurrFrmIdx].m_iBits  = (Int)uiBits;
  
  if(m_pcFrameData[m_iCurrFrmIdx].m_bReferenced)
  {
    m_iPrevFrmIdx = m_iCurrFrmIdx;
    m_cMADLinearModel.updateMADHistory(m_pcFrameData[m_iCurrFrmIdx].m_dMAD);
  }
}

Void  TEncRateCtrl::updateLCUData(TComDataCU* pcCU, UInt64 uiBits, Int iQP)
{
  Int     x, y;
  double dMAD = 0.0;

  Pel*  pOrg    = pcCU->getPic()->getPicYuvOrg()->getLumaAddr(pcCU->getAddr(), 0);
  Pel*  pRec    = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), 0);
  Int   iStride = pcCU->getPic()->getStride();

  Int   iWidth  = m_pcLCUData[m_iNumofLCU].m_iWidthInPixel;
  Int   iHeight = m_pcLCUData[m_iNumofLCU].m_iHeightInPixel;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      dMAD += abs( pOrg[x] - pRec[x] );
    }
    pOrg += iStride;
    pRec += iStride;
  }
  m_pcLCUData[m_iNumofLCU  ].m_iQP   = iQP;
  m_pcLCUData[m_iNumofLCU  ].m_dMAD  = (dMAD /(Double)(iWidth*iHeight));
  m_pcLCUData[m_iNumofLCU++].m_iBits = (Int)uiBits;
}

Double TEncRateCtrl::xAdjustmentBits( Int& iReductionFact, Int& iCompensationFact)
{
  Double dAdjustment   = ADJUSTMENT_FACTOR*iReductionFact;
  iReductionFact      -= (Int)dAdjustment;
  iCompensationFact   += (Int)dAdjustment;

  return dAdjustment;
}

