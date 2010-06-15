/*! ====================================================================================================================
 * \file
    TEncBinCoderV2VwLB.cpp
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
 
#include <cmath>
#include "TEncBinCoderV2VwLB.h"
#include "TEncV2VTrees.h"

void TEncClearBuffer::init () {
    for (buffer_tail = 0; buffer_tail < StateCount; ++buffer_tail) {
        code_pos[buffer_tail] = buffer_tail;
        bit_pos[buffer_tail] = 0;
        buffer[buffer_tail].code = buffer[buffer_tail].next = 0;
    }
    for (int k = 0; k < StateCount; k++)
        symbolCount[k] = LPSCount[k] = 0;
    sourceSelectionDone = false;
}

void TEncClearBuffer::storeBit(char bit, int state) {
    assert(0 <= state && state < mergedStateCount);
    ++symbolCount[state];
    if (bit) ++LPSCount[state];

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

    if (!sourceSelectionDone) { groupStates(); sourceSelectionDone = true; }

    char bit = uiSymbol != rcSCModel.getMps();
    storeBit(bit, mergedStatesMapping[rcSCModel.getState()]);
    if( bit ) {
      rcSCModel.updateLPS();
    } else {
      rcSCModel.updateMPS();
    }
}


Void TEncClearBuffer::finish() {
    int k;
    offset = 0;

    m_pcBitIf->writeAlignOne();

    UChar buf = 0;
    for (k = 0; k < StateCount; ++k) {
        if (lastStateOfGroup[k]) buf |= 1 << (k%8);
        if (k%8 == 7 || k == StateCount - 1) {
            myPutByte(buf);
            buf = 0;
        }
    }
    assert(buf == 0);
    for (k = 0; k < TreeCount; ++k) {
        if (selectedTree[k]) buf |= 1 << (k%8);
        if (k%8 == 7 || k == TreeCount - 1) {
            myPutByte(buf);
            buf = 0;
        }
    }

    for (k = 0; k < mergedStateCount; ++k) {
        UInt len = encode_seq(k); 
        putPrefCode(len);
    }

    addLoadBalancingHeader();
    for (k = 0; k < offset; myPutByte(temp_space[k++]));

    init();
}


UInt TEncClearBuffer::putPrefCode(UInt v) {

    if (v < 128) {
        myPutByte(v << 1);
        return 1;
    }

    v -= 128;
    if (v < 16384) {
        v = (v << 2) | 1;
        myPutByte(v & 0xff);
        myPutByte(v >> 8);
        return 2;
    }

    v -= 16384;
    if (v < 2097152) {
        v = (v << 3) | 3;
        myPutByte(v & 0xff);
        v >>= 8;
        myPutByte(v & 0xff);
        myPutByte(v >> 8);
        return 3;
    }

    v -= 2097152;
    if (v >= 536870912) {
        fputs("Overflow in putPrefCode\n", stderr);
        exit(1);
    }
    v = (v << 3) | 7;
    myPutByte(v & 0xff);
    v >>= 8;
    myPutByte(v & 0xff);
    v >>= 8;
    myPutByte(v & 0xff);
    myPutByte(v >> 8);
    return 4;
}

UInt TEncClearBuffer::getPrefCost(UInt v) {
    return v < 128 ? 8 : v < 16512 ? 16 : v < 2113664 ? 24 : 32;
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

UInt TEncV2V::getBestTree(UInt totalCount, UInt LPSCount) {
    if (LPSCount >= totalCount / 2) return 0;
    if (LPSCount == 0)  return TreeCount - 1;

    double p = double(LPSCount) / totalCount;
    UInt k = 0;
    while (k + 1 < TreeCount && p < TreeProb[k+1]) ++k;
    if (k + 1 == TreeCount) return TreeCount - 1;
    double loss_k = p * log(p / TreeProb[k]) + (1-p) * log((1-p) / (1-TreeProb[k]));
    double loss_k1 = p * log(p / TreeProb[k+1]) + (1-p) * log((1-p) / (1-TreeProb[k+1]));

    return loss_k < loss_k1 ? k : k + 1;
}

double TEncV2V::getTotalCost(UInt tree, UInt totalCount, UInt LPSCount) {
    double rawCost = - (LPSCount * log(TreeProb[tree])
                          + (totalCount-LPSCount) * log(1.-TreeProb[tree])) / log(2.);
    double seqLen = (1 + TreeLoss[tree]) * rawCost + TermCost[tree] + 4;
    return seqLen + getPrefCost(int(seqLen));
}

void TEncV2V::groupStates() {

    mergedStateCount = StateCount;
    for (int k = 0; k < StateCount; k++)
        lastStateOfGroup[k] = true;
    for (int k = 0; k < 64; k++)
        mergedStatesMapping[k] = QStatesMapping[k];
    for (int k = 0; k < TreeCount; selectedTree[k++] = false);

    for (;;) {

        int bestMergeCandidate = -1;
        double bestMergeGain = -100000000.0;
        UInt k, n;

        int bestTreeThis = getBestTree(m_uiState[0], m_uipState[0][1]);
        double totalCostThis = getTotalCost(bestTreeThis, m_uiState[0], m_uipState[0][1]);

        for (k = 0; k < mergedStateCount - 1; k++) {
            int bestTreeNext = getBestTree(m_uiState[k+1], m_uipState[k+1][1]);
            int bestTreeMerged = getBestTree(m_uiState[k] + m_uiState[k+1], m_uipState[k][1] + m_uipState[k+1][1]);
            double totalCostNext = getTotalCost(bestTreeNext, m_uiState[k+1], m_uipState[k+1][1]);
            double totalCostMerged = getTotalCost(bestTreeMerged, m_uiState[k] + m_uiState[k+1], m_uipState[k][1] + m_uipState[k+1][1]);
            if (totalCostThis + totalCostNext - totalCostMerged > bestMergeGain) {
                bestMergeCandidate = k;
                bestMergeGain = totalCostThis + totalCostNext - totalCostMerged;
            }
            bestTreeThis = bestTreeNext;
            totalCostThis = totalCostNext;
        }

        if (bestMergeGain < 0) break;

        for (n = k = 0; ; k++, n++) {
            while (!lastStateOfGroup[n]) ++n;
            if (k == bestMergeCandidate) break;
        }
        lastStateOfGroup[n] = false;
        for (k = 0; k < 64; k++)
            if (mergedStatesMapping[k] > bestMergeCandidate)
                --mergedStatesMapping[k];

        m_uiState[bestMergeCandidate] += m_uiState[bestMergeCandidate + 1];
        m_uipState[bestMergeCandidate][0] += m_uipState[bestMergeCandidate + 1][0];
        m_uipState[bestMergeCandidate][1] += m_uipState[bestMergeCandidate + 1][1];
        for (k = bestMergeCandidate + 1; k < mergedStateCount - 1; k++) {
            m_uiState[k] = m_uiState[k + 1];
            m_uipState[k][0] = m_uipState[k + 1][0];
            m_uipState[k][1] = m_uipState[k + 1][1];
        }
        --mergedStateCount;
    }

    for (int k = 0; k < mergedStateCount; k++) {
        mergedTree[k] = getBestTree(m_uiState[k], m_uipState[k][1]);
        selectedTree[mergedTree[k]] = true;
    }
}


void TEncV2V::encode_bit(unsigned short state, UChar symbol) {

    vlc_pos[state] = (QEncodingTree[mergedTree[state]][vlc_pos[state]] >> 20) + !symbol;
    if ((QEncodingTree[mergedTree[state]][vlc_pos[state]] >> 20) == 0) {
        int code_length = ((QEncodingTree[mergedTree[state]][vlc_pos[state]] >> 16) & 0xf) + 1;
        codeBuffer |= (QEncodingTree[mergedTree[state]][vlc_pos[state]] & 0xffff) << bitsUsed;
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
    for (k = 0; k < StateCount; vlc_pos[k++] = 0);

    while (buffer[pos].next) {
        cw = buffer[pos].code;
        for (k = 0; k < 32; ++k, cw >>= 1)
            encode_bit(tree, cw & 1);
        pos = buffer[pos].next;
    }

    cw = buffer[pos].code;
    for (k = 0; k < bit_pos[tree]; ++k, cw >>= 1)
        encode_bit(tree, cw & 1);

    if (vlc_pos[tree] && (QEncodingTree[mergedTree[tree]][vlc_pos[tree]] >> 20)) {
        int code_length;
        do {
            vlc_pos[tree] = (QEncodingTree[mergedTree[tree]][vlc_pos[tree]] >> 20) + 1;
        } while (QEncodingTree[mergedTree[tree]][vlc_pos[tree]] >> 20);
        code_length = ((QEncodingTree[mergedTree[tree]][vlc_pos[tree]] >> 16) & 0xf) + 1;
        codeBuffer |= (QEncodingTree[mergedTree[tree]][vlc_pos[tree]] & 0xffff) << bitsUsed;
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

