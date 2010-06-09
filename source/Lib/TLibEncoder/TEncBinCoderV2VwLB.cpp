/*! ====================================================================================================================
 * \file
    TEncV2V.cpp
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
 
#include "TEncBinCoderV2VwLB.h"

void TEncClearBuffer::init () {
    for (buffer_tail = 0; buffer_tail < TREE_NUM; ++buffer_tail) {
        code_pos[buffer_tail] = buffer_tail;
        bit_pos[buffer_tail] = 0;
        buffer[buffer_tail].code = buffer[buffer_tail].next = 0;
    }
}

void TEncClearBuffer::storeBit(char bit, int state) {

    if (bit_pos[state] == 32) {
        if (buffer_tail >= BUFFER_SIZE) {
            fputs("Output buffer overflow\n", stderr);
            exit(1);
        }
        buffer[buffer_tail].code = buffer[buffer_tail].next = 0;
        buffer[code_pos[state]].next = buffer_tail;
        code_pos[state] = buffer_tail++;
        bit_pos[state] = 0;
    }
    if (bit) buffer[code_pos[state]].code |= 1 << bit_pos[state];
    ++bit_pos[state];
}


Void TEncClearBuffer::encodeBin(UInt uiSymbol, ContextModel& rcSCModel) {

    char bit = uiSymbol != rcSCModel.getMps();

    storeBit(bit, rcSCModel.getState());

    if( bit ) {
      rcSCModel.updateLPS();
    } else {
      rcSCModel.updateMPS();
    }
        /*
        if (cts_index < TEMP_SIZE2) {
            cabac_tmp_spc[cts_index++] = (bit << 7) | state;
            if (cabac_cost < input->minimum_encoded_frame_size << 7 && cabac_cost < 0xff000000)
                cabac_cost += entropyBits[bit ? 64 + state : 63 - state] >> 8;
        }
        */
}


Void TEncClearBuffer::finish() {
    int k;
    offset = 0;
    m_pcBitIf->writeAlignOne();

    for (k = 0; k < TREE_NUM; ++k) {
        UInt len = encode_seq(k); 
        put_pref_code(len);
    }

    addLoadBalancingHeader();

    for (k = 0; k < offset; myPutByte(temp_space[k++]));

    init();
}


void TEncClearBuffer::put_pref_code(UInt v) {

    if (v < 128) {
        myPutByte(v << 1);
        return;
    }

    v -= 128;
    if (v < 16384) {
        v = (v << 2) | 1;
        myPutByte(v & 0xff);
        myPutByte(v >> 8);
        return;
    }

    v -= 16384;
    if (v < 2097152) {
        v = (v << 3) | 3;
        myPutByte(v & 0xff);
        v >>= 8;
        myPutByte(v & 0xff);
        myPutByte(v >> 8);
        return;
    }

    v -= 2097152;
    if (v >= 536870912) {
        fputs("Overflow in put_pref_code\n", stderr);
        exit(1);
    }
    v = (v << 3) | 7;
    myPutByte(v & 0xff);
    v >>= 8;
    myPutByte(v & 0xff);
    v >>= 8;
    myPutByte(v & 0xff);
    myPutByte(v >> 8);
}

UInt TEncClearBuffer::encode_seq(int tree) {
    UInt pos = tree, len = 0, cw;

    while (buffer[pos].next) {
        cw = buffer[pos].code;
        temp_space[offset++] = cw & 0xff; cw >>= 8;
        temp_space[offset++] = cw & 0xff; cw >>= 8;
        temp_space[offset++] = cw & 0xff; cw >>= 8;
        temp_space[offset++] = cw & 0xff;
        pos = buffer[pos].next;
        len += 4;
    }

    cw = buffer[pos].code;
    for (int k = bit_pos[tree]; k > 0; k -= 8, cw >>= 8) {
        temp_space[offset++] = cw & 0xff;
        ++len;
    }

    return len;
}


#include "TEncV2VTrees67k.h"

void TEncV2V::encode_bit(unsigned short state, UChar symbol) {

    vlc_pos[state] = (enc_tree[state][vlc_pos[state]] >> 20) + !symbol;
    if ((enc_tree[state][vlc_pos[state]] >> 20) == 0) {
        int code_length = ((enc_tree[state][vlc_pos[state]] >> 16) & 0xf) + 1;
        //code_offset = code_length + bitsUsed;
        codeBuffer |= (enc_tree[state][vlc_pos[state]] & 0xffff) << bitsUsed;
        bitsUsed += code_length;
        while (bitsUsed >= 8) {
            LB_temp_space[offset] = bitsUsed;
            temp_space[offset++] = codeBuffer & 0xff;
            codeBuffer >>= 8;
            bitsUsed -= 8;
        }
        vlc_pos[state] = 0;
    }
}

UInt TEncV2V::encode_seq(int tree) {
    UInt pos = tree, old_offset = offset, cw, k;

    bitsUsed = codeBuffer = 0;
    for (k = 0; k < TREE_NUM; vlc_pos[k++] = 0);

    while (buffer[pos].next) {
        cw = buffer[pos].code;
        for (k = 0; k < 32; ++k, cw >>= 1)
            encode_bit(tree, cw & 1);
        pos = buffer[pos].next;
    }

    cw = buffer[pos].code;
    for (k = 0; k < bit_pos[tree]; ++k, cw >>= 1)
        encode_bit(tree, cw & 1);

    if (vlc_pos[tree] && (enc_tree[tree][vlc_pos[tree]] >> 20)) {
        int code_length;
        do {
            vlc_pos[tree] = (enc_tree[tree][vlc_pos[tree]] >> 20) + 1;
        } while (enc_tree[tree][vlc_pos[tree]] >> 20);
        code_length = ((enc_tree[tree][vlc_pos[tree]] >> 16) & 0xf) + 1;
        codeBuffer |= (enc_tree[tree][vlc_pos[tree]] & 0xffff) << bitsUsed;
        bitsUsed += code_length;
        while (bitsUsed >= 8) {
            LB_temp_space[offset] = bitsUsed;
            temp_space[offset++] = codeBuffer & 0xff;
            codeBuffer >>= 8;
            bitsUsed -= 8;
        }
        vlc_pos[tree] = 0;
    }
    if (bitsUsed) {
        LB_temp_space[offset] = 8;   // Byte-aligned segments
        //LB_temp_space[offset] = bitsUsed;   // Segments not aligned
        temp_space[offset++] = codeBuffer & 0xff;
        codeBuffer = 0;
        bitsUsed = 0;
    }

    return offset - old_offset;
}

