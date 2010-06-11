/*! ====================================================================================================================
 * \file
    TEncV2V.h
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


 
#ifndef __TENCV2V__
#define __TENCV2V__

#include "TEncBinCoder.h"
#include <stdlib.h>

//using namespace std;

class TEncClearBit : public TEncBinIf {

public:
  virtual Void  init ( TComBitIf* pcTComBitIf ) { m_pcBitIf = pcTComBitIf; }
  virtual Void  uninit () {}

  virtual Void  start () {}
  virtual Void  finish () {} 

  virtual Void  copyState ( TEncBinIf* pcTEncBinIf ) { assert(0); }
  virtual Void  resetBits         () { assert(0); }
  virtual UInt  getNumWrittenBits () { assert(0); return 0; }

protected:
  virtual Void  encodeBin ( UInt uiSymbol, ContextModel& rcSCModel ) {
      m_pcBitIf->write( uiSymbol, 1 );
  }
  virtual Void  encodeBinTrm ( UInt uiBit )     {
      m_pcBitIf->write( uiBit   , 1 );
  }                                                                
  virtual Void  encodeBinEP ( UInt uiSymbol )  {
      m_pcBitIf->write( uiSymbol, 1 );
  }     

protected:
  TComBitIf*  m_pcBitIf;
};

const int TREE_NUM = 64;
//const int TREE_NUM = 28;    // Using stateMappingTable
const int BUFFER_SIZE = 5000000;
const int TEMP_SIZE = 5000000;

const int BALANCED_CPUS = 8;   // TODO: this will be user-adjustable in a later version

typedef struct {
    UInt code, next;
} parallel_buffer;

//extern UChar stateMappingTable[113];


class TEncClearBuffer : public TEncClearBit {

protected:
    parallel_buffer *buffer;
    UInt buffer_tail, code_pos[TREE_NUM], bit_pos[TREE_NUM];
    UChar *temp_space;
    UInt offset;

private:
    void init ();
    void storeBit(char bit, int state);

public:
    virtual Void encodeBin(UInt uiSymbol, ContextModel& rcSCModel);
    virtual Void encodeBinTrm ( UInt bit ) { storeBit(bit, TREE_NUM - 1); }
    virtual Void encodeBinEP ( UInt bit ) { storeBit(bit, 0);  }
    virtual Void finish();

protected:
    void myPutByte(UChar v) { m_pcBitIf->write( v, 8 ); }
    virtual void addLoadBalancingHeader() {}
    void put_pref_code(UInt v);

private:
    virtual UInt encode_seq(int tree);

public:
    TEncClearBuffer() {
        buffer = new parallel_buffer[BUFFER_SIZE];
        temp_space = new UChar[TEMP_SIZE];
        init();
    }

    virtual ~TEncClearBuffer() {
        delete [] temp_space;
        delete [] buffer;
    }
};


class TEncV2V : public TEncClearBuffer {

    UInt bitsUsed, codeBuffer;
    UInt vlc_pos[TREE_NUM]; 
    UChar *LB_temp_space;

    void encode_bit(unsigned short state, UChar symbol);
    virtual UInt encode_seq(int tree);

    virtual void addLoadBalancingHeader() {
        for (int k = 1; k < BALANCED_CPUS; ++k)
            put_pref_code(LB_temp_space[k * offset / BALANCED_CPUS]);
    }

public:
    TEncV2V() { LB_temp_space = new UChar[TEMP_SIZE]; }
    ~TEncV2V() { delete [] LB_temp_space; }
};

#endif

