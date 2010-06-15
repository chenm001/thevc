/*! ====================================================================================================================
 * \file
    TEncBinCoderCABAC4V2V.h
 *  \brief
    Copyright information.
 *  \par Copyright statements
    HEVC (JCTVC cfp)

    This software, including its corresponding source code, object code and executable, may only be used for
    (1) research purposes or (2) for evaluation for standardisation purposes within the joint collaborative team on
    video coding for HEVC , provided that this copyright notice and this corresponding notice appear in all copies,
    and that the name of Research in Motion Limited not be used in advertising or publicity without specific, written
    prior permission.  This software, as defined above, is provided as a proof-of-concept and for demonstration
    purposes only; there is no representation about the suitability of this software, as defined above, for any purpose.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    TRADEMARKS, Product and company names mentioned herein may be the trademarks of their respective owners.
    Any rights not expressly granted herein are reserved.

    Copyright (C) 2010 by Research in Motion Limited, Canada
    All rights reserved.

 *  \par Full Contact Information
    Standards & Licensing Department      (standards-ipr@rim.com)
    Research in Motion
    122 West John Carpenter Parkway
    Irving, TX 75039, USA

 * ====================================================================================================================
*/



#ifndef __TENC_CABAC_4V2V__
#define __TENC_CABAC_4V2V__

#include "TEncBinCoderCABAC.h"
#include "TEncV2VTrees.h"

class TEncBinCABAC4V2V : public TEncBinCABAC
{
public:
  TEncBinCABAC4V2V () { m_uiCntFlag = 0; }
  ~TEncBinCABAC4V2V() {}

  Void encodeBin(UInt uiBin, ContextModel& rcCtxModel)
  {
    UInt  ctxState = rcCtxModel.getState();
    UInt  sym;

    if(m_uiCntFlag)
    {
      sym = rcCtxModel.getMps();
      m_uiState[QStatesMapping[ctxState]]++;
      m_uipState[QStatesMapping[ctxState]][rcCtxModel.getMps()!=uiBin]++;
    }

    TEncBinCABAC::encodeBin( uiBin, rcCtxModel );
  }

  Void encodeBinEP(UInt uiBin)
  {
    if(m_uiCntFlag)
    {
      m_uiState[QStatesMapping[0]]++;
      m_uipState[QStatesMapping[0]][1==uiBin]++;
    }

    TEncBinCABAC::encodeBinEP( uiBin );
  }

  Void encodeBinTrm(UInt uiBin)
  {
    if(m_uiCntFlag)
    {
      m_uiState[QStatesMapping[63]]++;
      m_uipState[QStatesMapping[63]][1==uiBin]++;
    }

    TEncBinCABAC::encodeBinTrm( uiBin );
  }

  TEncBinCABAC4V2V* getTEncBinCABAC4V2V()  { return this; }

  Void clearStats()
  {
    ::memset( m_uiState, 0, sizeof( UInt ) * StateCount );
    ::memset( &m_uipState[0][0], 0, sizeof( UInt ) * StateCount * 2 );
  }

  Void  processStats() {}
  Void  setCntFlag(UInt ui) { m_uiCntFlag = ui; }
  UInt* getState()  { return m_uiState; }
  UInt* getpState() { return &m_uipState[0][0]; }

private:
  UInt    m_uiState[StateCount];
  UInt    m_uipState[StateCount][2];
  UInt    m_uiCntFlag;
};

#endif
