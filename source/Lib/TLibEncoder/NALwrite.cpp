/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
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

#include <vector>
#include <algorithm>
#include <ostream>

#include "../TLibCommon/NAL.h"
#include "../TLibCommon/TComBitStream.h"
#include "NALwrite.h"

using namespace std;


/**
 * write @nalu@ to bytestream @out@, performing RBSP anti startcode
 * emulation as required.  @nalu@.m_RBSPayload must be byte aligned.
 */
void write(ostream& out, const NALUnit& nalu)
{
  TComOutputBitstream bsNALUHeader;

  bsNALUHeader.write(0,1); // forbidden_zero_flag
  bsNALUHeader.write(nalu.m_RefIDC, 2); // nal_ref_idc
  bsNALUHeader.write(nalu.m_UnitType, 5); // nal_unit_type

  switch (nalu.m_UnitType) {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_CODED_SLICE_CDR:
    bsNALUHeader.write(nalu.m_TemporalID, 3); // temporal_id
    bsNALUHeader.write(nalu.m_OutputFlag, 1); // output_flag
    bsNALUHeader.write(1, 4); // reserved_one_4bits
    break;
  default: break;
  }

  out.write(bsNALUHeader.getByteStream(), bsNALUHeader.getByteStreamLength());

  /* write out rsbp_byte's, inserting any required
   * emulation_prevention_three_byte's */
  const vector<uint8_t>& rbsp = *nalu.m_RBSPayload;

  for (vector<uint8_t>::const_iterator it = rbsp.begin(); it != rbsp.end();)
  {
    /* 1) find the next emulated start_code_prefix
     * 2a) if not found, write all remaining bytes out, stop.
     * 2b) otherwise, write all non-emulated bytes out
     * 3) insert emulation_prevention_three_byte
     */
    static const char start_code_prefix[] = {0,0,1};
    vector<uint8_t>::const_iterator found = search(it, rbsp.end(), start_code_prefix, start_code_prefix+3);
    unsigned num_nonemulated_bytes = found - it;
    out.write((char*)&*it, num_nonemulated_bytes);
    if (found != rbsp.end())
    {
      static const char emulation_prevention_prefix[] = {0,0,3};
      out.write(emulation_prevention_prefix, 3);
      num_nonemulated_bytes += 2;
    }
    it += num_nonemulated_bytes;
  };
}

/**
 * Write rbsp_trailing_bits to @bs@ causing it to become byte-aligned
 */
void writeRBSPTrailingBits(TComOutputBitstream& bs)
{
  bs.write( 1, 1 );
  bs.writeAlignZero();
}
