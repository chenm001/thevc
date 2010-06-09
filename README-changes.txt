Changes from JCTVC-A124
Contact: Woo-Jin Han, wjhan.han@samsung.com

1. Removed tools

  1.1 MVAC (motion vector accuracy control)
      - Enables to limit the motion accuracy up to 1/2 accuracy in B-slices for complexity reductio
			- Not used in CfP submission
      - Command line option in A124: MVA
  1.2 SHV (simultaneous H & V motion partition)
      - Enables to use motion partition composed of quarter and remaining regions
      - Not used in CfP submission
      - Command line option in A124: SHV
  1.3 RNG (random noise generation)
      - Insert Gaussian noise into the reconstruction file to improve perceived quality in flat areas
      - Not used in CfP submission
      - Command line option in A124: RNG
  1.4 LOT (logical transform)
      - Perform 5-3 wavelet + DCT if physical transform size is smaller than requested transform size
      - Used in CfP submission only for 128x128 block
      - Command line option in A124: LOT
  1.5 CADR (contents-adaptive dynamic range)
      - Performance source scaling within bit-depth limit
      - Used in CfP submission
      - Command line option in A124: CAD
  1.6 LCT (low-complexity transform)
      - Low-complexity version of large integer transform
      - Not used in CfP submission
      - Command line option in A124: LCT
  1.7 EXC (extreme correction) & BDC (band correction)
      - In-loop post filters based on pixel statistics
      - Used in CfP submission
      - Command line option in A124: EXC
  1.8 PTM (pattern matching intra)
      - Pixel-based pattern matching technique for intra prediction
      - Used in CfP submission
      - Command line option in A124: TMI
  1.9 MPI (multi-parametric intra)
      - Multi-parametric post-processing of intra prediction
      - Used in CfP submission
      - Command line option in A124: MPI
  1.A CCCP (color correlation based chroma prediction)
      - Chroma intra prediction based on luma reconstruction
      - Used in CfP submission
      - Command line option in A124: CCP
  1.B ACS (adaptive coefficient scanning)
      - 3 scanning patterns are used selectively (zigzag, horiz, vert)
      - Used in CfP submission
      - Command line option in A124: ACS
  1.C HAM (high accuracy motion)
      - 1/12th accuracy motion vector for luma and chroma
      - Used in CfP submission for both luma and chroma
      - Command line option in A124: HME, HAP, HAB
  1.D HME: high accuracy motion estimation
      - HAP: HAM in P-slice
      - HAB: HAM in B-slice

2. Modified tools

  2.1 DIF (DCT-based interpolation filter)
      - A124 uses 6-tap DIF for chroma
      - Replaced with AVC bi-linear (1/8th) since it uses HAM filters (1/12th), which are not included in TMuC.
  2.2 GRF (generated reference frame)
      - Weighted, weight + offset, offset, affine, isotropic and perspective are supported
      - Not used in CfP submission
      - Command line option in A124: -v <character>
      - w: weighted, o: offset, r: refinement, a: affine, i: isotropic, p: perspective
      - Affine, isotropic and perspective are removed but weighted prediction is remained due to simulate AVC WP
      - To activate: ¡®-v w¡¯ in command line (scale+offset) or ¡®-v o¡¯ for offset-only
  2.3 AMVP (advanced motion prediction)
      - A124 software supports three AMVP modes
      - AM_NONE	: use first candidate only (similar to AVC), no signaling
      - AM_EXPL	: use explicit signaling (used in CfP submission)
      - AM_IMPL	: use implicit signaling based on template matching
      - TMuC only has both explicit signaling mode and non-AMVP method
      - AM_IMPL is removed since it is not included in TMuC
      - AM_NONE is maintained since it can be a place-holder for non-AMVP method
  2.4 CIP (combined intra prediction)
      - CIPflag is now coded only for intra blocks (bug-fix)
      - A124 codes CIPflag even in inter blocks

3. Option changes

  3.1 JMQ (JM QP)
      - JMQ = 1: use JM QP assignments, JMQ = 0: use JSVM QP assignments
      - Option is removed and JMQ is always on
      - Option fix: JML (JM Lambda)
  3.2 JML = 1: use JM lambda strategy, JML = 0: use JSVM lambda strategy
      - Option is removed and JML is always on
  3.3 Option fix: ADI
      - Option is removed and ADI is always turned on
  3.4 Option fix: AMV (AMVP)
      - Option is removed and AMVP is always turned on
  3.5 Option fix: DIF
      - Option is removed and DIF is always turned on
  3.6 Option fix: AMP
      - Option is removed and AMP is always turned on
  3.7 Option fix: CIP
      - Option is removed and CIP is always turned on
  3.8 Option fix: ROT
      - Option is removed and ROT is always turned on
      - Note: set ROT_DICT = 1 to disable ROT
  3.9 Option fix: ACC
      - Try coefficient clearing in inter modes
      - Option is removed and ACC is always turned on
  3.A Profile option (-p)
      - It was used to separate A124 and A125 coding tools
      - Removed, now 

4. Misc. changes

  4.1 MAX value of IBDI is fixed (IBDI_NOCLIP_RANGE MACRO)
      - Only allows possible values of bit-depth increased signal, not all possible values of inherent high bit-depth case
  4.2 Encoder output is fixed
      - Slice size bit (32) is now added to the encoder print-out
	4.3 Source code improvements
	    - Unused functions are removed
			- Variable & function namings are changed to clarify its purpose
	
5. Known problems

  5.1 POC coding
	    - It's not implemented well. Currently, fixed 10-bit is used for POC coding.
	5.2 ROT (rotational transform)
	    - Inverse ROT code is not the best one on the aspects of the dynamic range although we already have better one.
			- It'll be replaced later with much simpler implementation.
