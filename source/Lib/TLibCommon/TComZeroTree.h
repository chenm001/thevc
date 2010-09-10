/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, NTT DOCOMO, INC. and DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of NTT DOCOMO, INC. nor the name of DOCOMO COMMUNICATIONS LABORATORIES USA, INC.
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

/** \file			TComZeroTree.h
    \brief		TComZeroTree & TComZTNode classes (header)
*/

#ifndef __TCOMZEROTREE__
#define __TCOMZEROTREE__

#include "CommonDef.h"

#ifdef DCM_PBIC 
// ====================================================================================================================
// Class definition
// ====================================================================================================================
//-----------------------------------------------------------------------------
/*!
* \brief Zero tree node class
*
*/
//-----------------------------------------------------------------------------
class TComZTNode
{
public:
  TComZTNode*   m_pcLeft;      //!< Pointer to left child
  TComZTNode*   m_pcRight;     //!< Pointer to right child
  Int           m_id;          //!< Node ID. \note Leaf nodes have negative ID values.
  Int           m_iVal;        //!< Node value

  TComZTNode()
  {
    m_pcLeft  = NULL;
    m_pcRight = NULL;
    m_id      = MAX_INT;
    m_iVal    = 0;
  }

  Bool IsLeaf() { return (m_id < 0); } // determines whether node is a leaf
};

// ====================================================================================================================
// Class definition
// ====================================================================================================================
//-----------------------------------------------------------------------------
/*!
* \brief Zero tree class
*
*/
//-----------------------------------------------------------------------------
class TComZeroTree
{
protected:
  Int           m_iTreeIdx;    //!< Index into tree structure of pattern
  Int           m_iCoeffIdx;   //!< Index into coeff structure of pattern
  Int           m_iNodeIdx;    //!< Node counter
  TComZTNode*   m_pcNodes;     //!< Array of nodes in tree

  TComZTNode* xGrowChildren(Int* piPattern);

public:
  TComZTNode*   m_pcRoot;      //!< Pointer to root node
  Int           m_iNleaves;    //!< Number of leaf nodes in tree
  Int           m_iSize;       //!< Number of nodes in tree
  Int*          m_piStructure; //!< Bit pattern of tree structure
  Int*          m_piLeafIdx;   //!< Mapping of leaf indices to array of data represented by tree

  TComZeroTree(Int *pattern);
  ~TComZeroTree();

  Void updateVal(Int iZeroPatt);
};

#endif // #ifdef DCM_PBIC

#endif // #ifndef __TCOMZEROTREE__
