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

/** \file     TDecGop.cpp
    \brief    GOP decoder class
*/

extern bool g_md5_mismatch; ///< top level flag to signal when there is a decode problem

#include "TDecGop.h"
#include "TDecCAVLC.h"
#include "TDecSbac.h"
#include "TDecBinCoder.h"
#include "TDecBinCoderCABAC.h"
#include "libmd5/MD5.h"
#include "TLibCommon/SEI.h"

#include <time.h>

//! \ingroup TLibDecoder
//! \{

static void calcAndPrintMD5Status(TComPicYuv& pic, const SEImessages* seis);

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TDecGop::TDecGop()
{
  m_iGopSize = 0;
  m_dDecTime = 0;
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
                   TDecCavlc*              pcCavlcDecoder, 
                   TDecSlice*              pcSliceDecoder, 
                   TComLoopFilter*         pcLoopFilter
                   ,TComSampleAdaptiveOffset* pcSAO
                   )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcBinCABAC            = pcBinCABAC;
  m_pcCavlcDecoder        = pcCavlcDecoder;
  m_pcSliceDecoder        = pcSliceDecoder;
  m_pcLoopFilter          = pcLoopFilter;
  m_pcSAO  = pcSAO;
}


// ====================================================================================================================
// Private member functions
// ====================================================================================================================

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if G1002_RPS
Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf)
#else
#if REF_SETTING_FOR_LD
Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf, TComList<TComPic*>& rcListPic )
#else
Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf)
#endif
#endif
{
  TComSlice*  pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());

  //-- For time output for each slice
  long iBeforeTime = clock();
  
  static Bool  bFirst = true;
  static UInt  uiILSliceCount;
  static UInt* puiILSliceStartLCU;

  if (!bExecuteDeblockAndAlf)
  {
    if(bFirst)
    {
      uiILSliceCount = 0;
      puiILSliceStartLCU = new UInt[(rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU()) +1];
      bFirst = false;
    }

    UInt uiSliceStartCuAddr = pcSlice->getSliceCurStartCUAddr();
    {
      puiILSliceStartLCU[uiILSliceCount] = uiSliceStartCuAddr;
      uiILSliceCount++;
    }

      m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinCABAC );
      m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
    
    m_pcEntropyDecoder->setBitstream      (pcBitstream);
    m_pcEntropyDecoder->resetEntropy      (pcSlice);

    m_pcSliceDecoder->decompressSlice(pcBitstream, rpcPic, m_pcSbacDecoder);
    
    m_dDecTime += (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  }
  else
  {
    // deblocking filter
#if G174_DF_OFFSET
    if(pcSlice->getSPS()->getUseDF())
    {
      if(pcSlice->getInheritDblParamFromAPS())
      {
        pcSlice->setLoopFilterDisable(pcSlice->getAPS()->getLoopFilterDisable());
        if (!pcSlice->getLoopFilterDisable())
        {
          pcSlice->setLoopFilterBetaOffset(pcSlice->getAPS()->getLoopFilterBetaOffset());
          pcSlice->setLoopFilterTcOffset(pcSlice->getAPS()->getLoopFilterTcOffset());
        }
      }
    }
    m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), pcSlice->getLoopFilterBetaOffset(), pcSlice->getLoopFilterTcOffset());
#else
    m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), 0, 0);
#endif
    m_pcLoopFilter->loopFilterPic( rpcPic );

#if NONCROSS_TILE_IN_LOOP_FILTERING
    pcSlice = rpcPic->getSlice(0);
    if(pcSlice->getSPS()->getUseSAO())
    {
      puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU();
      rpcPic->createNonDBFilterInfo(puiILSliceStartLCU, uiILSliceCount);
    }
#endif

    {

      if( pcSlice->getSPS()->getUseSAO() )
      {
#if !NONCROSS_TILE_IN_LOOP_FILTERING
        m_pcSAO->setNumSlicesInPic( uiILSliceCount );
#endif
#if ALF_SAO_SLICE_FLAGS
        if(pcSlice->getSaoEnabledFlag())
#else
        if(pcSlice->getAPS()->getSaoEnabled())
#endif
        {

#if NONCROSS_TILE_IN_LOOP_FILTERING
          m_pcSAO->createPicSaoInfo(rpcPic);
#else

        if(uiILSliceCount == 1)
        {
        }
        else
        {
            m_pcSAO->setPic(rpcPic);
            puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU();
        }
#endif

        m_pcSAO->SAOProcess(rpcPic, pcSlice->getAPS()->getSaoParam());  

#if NONCROSS_TILE_IN_LOOP_FILTERING
      m_pcSAO->destroyPicSaoInfo();
#endif
      }
    }

    }

#if NONCROSS_TILE_IN_LOOP_FILTERING
    if(pcSlice->getSPS()->getUseSAO())
    {
      rpcPic->destroyNonDBFilterInfo();
    }
#endif

    rpcPic->compressMotion(); 
    Char c = (pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B');
    if (!pcSlice->isReferenced()) c += 32;
    
    //-- For time output for each slice
    printf("\nPOC %4d TId: %1d ( %c-SLICE, QP%3d ) ",
          pcSlice->getPOC(),
          pcSlice->getTLayer(),
          c,
          pcSlice->getSliceQp() );

    m_dDecTime += (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
    printf ("[DT %6.3f] ", m_dDecTime );
    m_dDecTime  = 0;
    
    for (Int iRefList = 0; iRefList < 2; iRefList++)
    {
      printf ("[L%d ", iRefList);
      for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
      {
        printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
      printf ("] ");
    }
    if(pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0 && !pcSlice->getNoBackPredFlag())
    {
      printf ("[LC ");
      for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(REF_PIC_LIST_C); iRefIndex++)
      {
        printf ("%d ", pcSlice->getRefPOC((RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIndex), pcSlice->getRefIdxFromIdxOfLC(iRefIndex)));
      }
      printf ("] ");
    }

    if (m_pictureDigestEnabled)
    {
      calcAndPrintMD5Status(*rpcPic->getPicYuvRec(), rpcPic->getSEIs());
    }

#if FIXED_ROUNDING_FRAME_MEMORY
    rpcPic->getPicYuvRec()->xFixedRoundingPic();
#endif

#if G1002_RPS
    rpcPic->setOutputMark(true);
#endif
    rpcPic->setReconMark(true);

#if NO_TMVP_MARKING
    rpcPic->setUsedForTMVP( true );
#endif

#if !G1002_RPS
#if REF_SETTING_FOR_LD
      if ( rpcPic->getSlice(0)->getSPS()->getUseNewRefSetting() )
      {
        if ( rpcPic->getSlice(0)->isReferenced() )
        {
          rpcPic->getSlice(0)->decodingRefMarkingForLD( rcListPic, rpcPic->getSlice(0)->getSPS()->getMaxNumRefFrames(), rpcPic->getSlice(0)->getPOC() );
        }
      }
#endif
#endif

    uiILSliceCount = 0;
  }
}

/**
 * Calculate and print MD5 for pic, compare to picture_digest SEI if
 * present in seis.  seis may be NULL.  MD5 is printed to stdout, in
 * a manner suitable for the status line. Theformat is:
 *  [MD5:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,(yyy)]
 * Where, x..x is the md5
 *        yyy has the following meanings:
 *            OK          - calculated MD5 matches the SEI message
 *            ***ERROR*** - calculated MD5 does not match the SEI message
 *            unk         - no SEI message was available for comparison
 */
static void calcAndPrintMD5Status(TComPicYuv& pic, const SEImessages* seis)
{
  /* calculate MD5sum for entire reconstructed picture */
  unsigned char recon_digest[16];
  calcMD5(pic, recon_digest);

  /* compare digest against received version */
  const char* md5_ok = "(unk)";
  bool md5_mismatch = false;

  if (seis && seis->picture_digest)
  {
    md5_ok = "(OK)";
    for (unsigned i = 0; i < 16; i++)
    {
      if (recon_digest[i] != seis->picture_digest->digest[i])
      {
        md5_ok = "(***ERROR***)";
        md5_mismatch = true;
      }
    }
  }

  printf("[MD5:%s,%s] ", digestToString(recon_digest), md5_ok);
  if (md5_mismatch)
  {
    g_md5_mismatch = true;
    printf("[rxMD5:%s] ", digestToString(seis->picture_digest->digest));
  }
}
//! \}
