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

/** \file			TComZeroTree.cpp
    \brief		Methods for TComZeroTree class
*/

#include "CommonDef.h"
#include "TComZeroTree.h"

#ifdef DCM_PBIC
// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================
//-----------------------------------------------------------------------------
/*!
* \brief Constructor
*
* Constructs a zerotree based on Pattern
*
* \param[in] piPattern
*/
//-----------------------------------------------------------------------------
TComZeroTree::TComZeroTree(Int* piPattern)
{
  Int iIdx;

  m_iNleaves    = piPattern[0];
  m_iSize       = (m_iNleaves << 1) - 1;

  m_pcNodes     = new TComZTNode[m_iSize];
  m_piLeafIdx   = new Int[m_iNleaves];
  m_piStructure = new Int[m_iSize];

  for (iIdx = 0; iIdx < m_iSize; iIdx++) 
  {
    m_pcNodes[iIdx].m_id = iIdx - m_iNleaves;
    m_piStructure[iIdx]  = piPattern[iIdx + 1];
  }

  for (iIdx = 0; iIdx < m_iNleaves; iIdx++)
    m_piLeafIdx[iIdx]    = piPattern[iIdx + m_iSize + 1];

  m_iTreeIdx  = 1;           // tree indices start at position 1 of piPattern
  m_iCoeffIdx = m_iSize + 1; // coeff indices start at position m_iSize+1 of piPattern
  m_iNodeIdx  = m_iNleaves;  // int nodes come after lead nodes

  m_pcRoot = xGrowChildren(piPattern);
}

//-----------------------------------------------------------------------------
/*!
* \brief Destructor
*/
//-----------------------------------------------------------------------------
TComZeroTree::~TComZeroTree()
{
  delete[] m_pcNodes;
  delete[] m_piLeafIdx;
  delete[] m_piStructure;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================
//-----------------------------------------------------------------------------
/*!
* \brief Update values of zerotree leaves
*
* Initially called with the root node as argument. Recursively updates values
* of leaf nodes.
* 
* \param[in] node Current node
* \param[in] val  Values of a sub-tree
*/
//-----------------------------------------------------------------------------
Void TComZeroTree::updateVal(Int iZeroPatt)
{
  Int iIdx;
  for (iIdx = 0; iIdx < m_iNleaves; iIdx++)
  {
    m_pcNodes[iIdx].m_iVal = ( (iZeroPatt & (1<<iIdx)) != 0);
  }

  for (iIdx = 2*m_iNleaves - 2; iIdx >= m_iNleaves; iIdx--)
  {
    m_pcNodes[iIdx].m_iVal = m_pcNodes[iIdx].m_pcLeft->m_iVal | m_pcNodes[iIdx].m_pcRight->m_iVal;
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
//-----------------------------------------------------------------------------
/*!
* \brief Add a node to zerotree and further grow tree if needed
*
* Is called during the zerotree construction phase. Adds a node and further
* grows children nodes if not a leaf node.
* 
* \param[in] piPattern
* \return Root node
*/
//-----------------------------------------------------------------------------
TComZTNode *TComZeroTree::xGrowChildren(Int* piPattern)
{
  TComZTNode* pcZTNode;

  if (piPattern[m_iTreeIdx++] == 0)
    pcZTNode = &m_pcNodes[piPattern[m_iCoeffIdx++]];
  else 
  {
    pcZTNode = &m_pcNodes[m_iNodeIdx++];
    pcZTNode->m_pcLeft  = xGrowChildren(piPattern);
    pcZTNode->m_pcRight = xGrowChildren(piPattern);
  }

  return pcZTNode;
}

#endif // #ifdef DCM_PBIC
