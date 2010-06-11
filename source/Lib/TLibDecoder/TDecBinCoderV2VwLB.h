/*! ====================================================================================================================
 * \file
    TDecBinCoderV2VwLB.h
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
 
#ifndef __TDECV2V__
#define __TDECV2V__

#include <cstdlib>
#include "TDecBinCoder.h"
#include "TDecV2VTrees.h"

class TDecClearBit : public TDecBinIf {

public:
  virtual Void  init( TComBitstream* pcTComBitstream ) { m_pcBitstream = pcTComBitstream; }
  virtual Void  uninit() {}

  virtual Void  start() {
      m_pcBitstream->readAlignOne();
  }
  virtual Void  finish() {}

  virtual Void  decodeBin( UInt& ruiSymbol, ContextModel& rcSCModel ) {
      m_pcBitstream->read( 1, ruiSymbol );
  }
  virtual Void  decodeBinEP( UInt& ruiSymbol ) {
      m_pcBitstream->read( 1, ruiSymbol );
  }                                            
  virtual Void  decodeBinTrm ( UInt& ruiBit ) {
      m_pcBitstream->read( 1, ruiBit    );
  }                   

  Void  setBalancedCPUs( UInt ui ) { m_uiBalancedCPUs = ui; }
  UInt  getBalancedCPUs() { return m_uiBalancedCPUs; }

protected:
  TComBitstream*  m_pcBitstream;

private:
	UInt m_uiBalancedCPUs;
};


const int TEMP_SIZE = 5000000;


class TDecClearBuffer : public TDecClearBit {

    UInt seq_coded_len[StateCount];
    UInt offset[StateCount];
    UInt term_offset[StateCount];
    UInt bit_pos[StateCount];

protected:
    UChar *temp_space;
    UInt index;

public:
  virtual Void  start() {
      m_pcBitstream->readAlignOne();
      init();
  }

protected:
    UChar myReadByte() {
        UInt c = 0;
        m_pcBitstream->read( 8, c );
        return c;
    }

    UInt mergedStateCount;
    bool lastStateOfGroup[StateCount];
    bool selectedTree[TreeCount];
    UInt mergedStatesMapping[64];
    UInt mergedTree[StateCount];

private:
    UInt get_pref_code() {

        UInt n;

        n = myReadByte();
        if (!(n & 1)) return n >> 1;
        n >>= 1;

        n |= myReadByte() << 7;
        if (!(n & 1)) return 128 + (n >> 1);
        n >>= 1;

        n |= myReadByte() << 14;
        if (!(n & 1)) return 16512 + (n >> 1);
        n >>= 1;

        n |= myReadByte() << 21;
        return 2113664 + n;
    }

    virtual void decode_seq(int tree, int len) {
        if (len == 0) return;
        while (len--)
            temp_space[index++] = myReadByte();
    }

    void init();

    char retrieveBit(int state) {

        char bit = 0;

        if (temp_space[offset[state]] & (1 << bit_pos[state]))
            bit = 1;

        if (++bit_pos[state] == 8) {
            bit_pos[state] = 0;
            if (++offset[state] == term_offset[state]) {
                offset[state] = 0;
                term_offset[state] = 1;
            }
        }

        return bit;
    }

    virtual Void decodeBin(UInt& ruiSymbol, ContextModel& rcSCModel) {

        ruiSymbol = rcSCModel.getMps();
        if (retrieveBit(mergedStatesMapping[rcSCModel.getState()])) {
            ruiSymbol = !ruiSymbol;
            rcSCModel.updateLPS();
        } else {
            rcSCModel.updateMPS();
        }
    }

    virtual Void decodeBinEP( UInt& bit ) { bit = retrieveBit(mergedStatesMapping[0]); }
    virtual Void decodeBinTrm ( UInt& bit ) { bit = retrieveBit(mergedStatesMapping[62]); }

public:
    TDecClearBuffer() { temp_space = new UChar[TEMP_SIZE]; }
    ~TDecClearBuffer() { delete [] temp_space; }

};

class TDecV2V: public TDecClearBuffer {

    UInt bitsUsed, codeBuffer;

    virtual void decode_seq(int tree, int len);

};

#endif

