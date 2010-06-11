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

/** \file     TDecGop.cpp
    \brief    GOP decoder class
*/

#include "TDecGop.h"
#include "TDecCAVLC.h"
#include "TDecSbac.h"
#include "TDecBinCoder.h"
#include "TDecBinCoderCABAC.h"
#include "TDecBinCoderMultiCABAC.h"
#include "TDecBinCoderPIPE.h"
#include "TDecBinCoderMultiPIPE.h"
#include "TDecBinCoderV2VwLB.h"

#include <time.h>

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TDecGop::TDecGop()
{
  m_iGopSize = 0;
}

TDecGop::~TDecGop()
{

}

Void TDecGop::create()
{

}

Void TDecGop::destroy()
{

}

Void TDecGop::init( TDecEntropy*            pcEntropyDecoder, 
                    TDecSbac*               pcSbacDecoder, 
                    TDecBinCABAC*           pcBinCABAC,
                    TDecBinMultiCABAC*      pcBinMultiCABAC,
                    TDecBinPIPE*            pcBinPIPE,
                    TDecBinMultiPIPE*       pcBinMultiPIPE,
                    TDecV2V*                pcBinV2VwLB,
                    TDecCavlc*              pcCavlcDecoder, 
                    TDecSlice*              pcSliceDecoder, 
                    TComLoopFilter*         pcLoopFilter, 
                    TComAdaptiveLoopFilter* pcAdaptiveLoopFilter )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcBinCABAC            = pcBinCABAC;
  m_pcBinMultiCABAC       = pcBinMultiCABAC;
  m_pcBinPIPE             = pcBinPIPE;
  m_pcBinMultiPIPE        = pcBinMultiPIPE;
  m_pcBinV2VwLB           = pcBinV2VwLB;
  m_pcCavlcDecoder        = pcCavlcDecoder;
  m_pcSliceDecoder        = pcSliceDecoder;
  m_pcLoopFilter          = pcLoopFilter;
  m_pcAdaptiveLoopFilter  = pcAdaptiveLoopFilter;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecGop::decompressGop (Bool bEos, TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComSlice*  pcSlice = rpcPic->getSlice();

  //-- For time output for each slice
  long iBeforeTime = clock();

  UInt iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
  {
    if( iSymbolMode == 3 )
    {
      m_pcSbacDecoder->init( m_pcBinV2VwLB );
      m_pcBinV2VwLB->setBalancedCPUs( getBalancedCPUs() );
    }
    else if( iSymbolMode == 1 )
    {
      m_pcSbacDecoder->init( pcSlice->getMultiCodeword() ? (TDecBinIf*)m_pcBinMultiCABAC : (TDecBinIf*)m_pcBinCABAC );
    }
    else if( pcSlice->getMultiCodeword() )
    {
      m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinMultiPIPE );
    }
    else
    {
      m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinPIPE );
      m_pcBinPIPE->initDelay( pcSlice->getMaxPIPEDelay() );
    }
    m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
  }
  else
  {
    m_pcEntropyDecoder->setEntropyDecoder (m_pcCavlcDecoder);
  }

  m_pcEntropyDecoder->setBitstream      (pcBitstream);
  m_pcEntropyDecoder->resetEntropy      (pcSlice);

  ALFParam cAlfParam;

  if ( rpcPic->getSlice()->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
#if HHI_ALF
    m_pcEntropyDecoder->decodeAlfParam(&cAlfParam, rpcPic );
    rpcPic->getSlice()->getSPS()->setALfSeparateQt( cAlfParam.bSeparateQt );
#else
    m_pcEntropyDecoder->decodeAlfParam( &cAlfParam );
#endif
  }

  m_pcSliceDecoder->decompressSlice(pcBitstream, rpcPic);

  // deblocking filter
  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), 0, 0);
  m_pcLoopFilter->loopFilterPic( rpcPic );

  // adaptive loop filter
  if( rpcPic->getSlice()->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->ALFProcess(rpcPic, &cAlfParam);
#if HHI_ALF
    if( cAlfParam.bSeparateQt && cAlfParam.cu_control_flag )
    {
      m_pcAdaptiveLoopFilter->destroyQuadTree(&cAlfParam);
    }
#endif
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
  }

#if HHI_INTERP_FILTER
  // MOMS prefilter reconstructed pic
  if( rpcPic->getSlice()->isReferenced() && rpcPic->getSlice()->getUseMOMS() )
  {
    TComCoeffCalcMOMS cCoeffCalc;
    cCoeffCalc.calcCoeffs( rpcPic->getPicYuvRec(), rpcPic->getPicYuvRecFilt(), rpcPic->getSlice()->getInterpFilterType() );
  }
#endif

  //-- For time output for each slice
  printf("\nPOC %4d ( %c-SLICE, QP%3d ) ",
                        pcSlice->getPOC(),
                        pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B',
                        pcSlice->getSliceQp() );

  Double dDecTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  printf ("[DT %6.3f] ", dDecTime );

  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf ("[L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      UInt uiOrgNumRefIdx;
      uiOrgNumRefIdx = pcSlice->getNumRefIdx(RefPicList(iRefList))-pcSlice->getAddRefCnt(RefPicList(iRefList));
      UInt uiNewRefIdx= iRefIndex-uiOrgNumRefIdx;
      if (iRefIndex >= (int)uiOrgNumRefIdx)
      {
        if ( pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx) == EFF_WP_SO ||
             pcSlice->getEffectMode(RefPicList(iRefList), uiNewRefIdx) == EFF_WP_O )
        {
          printf ("%dw ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
        }
      }
      else {
        printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
    }
    printf ("] ");
  }

  rpcPic->setReconMark(true);
}

