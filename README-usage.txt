Software usage example
Contact: Woo-Jin Han, wjhan.han@samsung.com

1. Installation and compilation
   1.1 Source tree
       - Root
         |--- bin
         |--- build
              |--- linux
         |--- cfg
              |--- cfp
         |--- doc
         |--- source
              |--- App
                   |--- TAppDecoder
                   |--- TAppEncoder
              |--- Lib
                   |--- TLibCommon
                   |--- TLibDecoder
                   |--- TLibEncoder
                   |--- TLibVideoIO
                   
   1.2 Windows using MS Visual Studio
       - Workspaces of VC6 and VC2008 are included in Root/build directory
   1.3 Linux
       - Makefile is included in Root/build/linux directory

2. Encoder option
   2.1 Parameters
        - TAppEncoder.exe -c config.cfg [options]
        - Options
            -c      configuration file name
            -i      original YUV input file name
            -o      decoded YUV output file name
            -b      bitstream file name
            -m      dQP file name
            -f      number of frames to be encoded (default EOS)
            -q      Qp value, if value is float, QP is switched once during encoding
            -g      GOP size of temporal structure
            -rg     GOP size of hierarchical QP assignment
            -s      max CU size
            -h      CU depth
            -r      Number of reference (P)
            -rb0    Number of reference (B_L0)
            -rb1    Number of reference (B_L1)
            -ip     intra period in frames, (-1: only first frame)
            -ldm    recommended low-delay setting (with LDC), (0=slow sequence, 1=fast sequence)
            -d      max dQp offset for block
            -dqr    max dQp offset for slice
            -t      max transform size
            -ltd    min transform depth
            -utd    max transform depth
            -v      additional reference for weighted prediction (w: scale+offset, o: offset)
            -tap    number of interpolation filter taps (luma)
            -tq0    QP offset of temporal layer 0
            -tq1    QP offset of temporal layer 1
            -tq2    QP offset of temporal layer 2
            -tq3    QP offset of temporal layer 3
            -pdx    horizontal source padding size
            -pdy    vertical source padding size
            -1/0    <name>: turn on/off <name>
                    <name> = ALF - adaptive loop filter
                             IBD - bit-depth increasement
                             GPB - generalized B instead of P in low-delay mode
                             HAD - hadamard ME for fractional-pel
                             SRD - SBAC based RD estimation
                             RDQ - RDOQ
                             LDC - low-delay mode
                             NRF - non-reference frame marking in last layer
                             BQP - hier-P style QP assignment in low-delay mode
                             PAD - automatic source padding of multiple of 16
                             QBO - skip refers highest quality picture
                             ASR - adaptive motion search range
   2.2 Config file (example)
   
      #======== File I/O ===============
      InputFile                     : d:\test\origcfp\RaceHorses_416x240_30.yuv
      BitstreamFile                 : RaceHorses.bin
      ReconFile                     : RaceHorses_enc.yuv
      FrameRate                     : 30          # Frame Rate per second
      FrameSkip                     : 0           # Number of frames to be skipped in input
      SourceWidth                   : 416         # Input  frame width
      SourceHeight                  : 240         # Input  frame height
      FrameToBeEncoded              : 9           # Number of frames to be coded

      #======== Coding Structure =======
      IntraPeriod                   : 32          # Period of I-Frame ( -1 = only first)
      GOPSize                       : 8           # GOP Size (number of B slice = GOPSize-1)
      NumOfReference                : 1           # Number of reference frames
      NumOfReferenceB_L0            : 1           # Number of reference frames for L0 for B-slices
      NumOfReferenceB_L1            : 1           # Number of reference frames for L1 for B-slices
      QP                            : 32          # Quantization parameter(0-51)

      #======== New Structure ===========
      MaxCUWidth                    : 128         # Maximum Coding Unit size in width
      MaxCUHeight                   : 128         # Maximum Coding Unit size in Height
      MaxPartitionDepth             : 5           # Maximum partition depth. ( minimum width = MaxWidth >> (MaxPartitionDepth-1) )

      #=========== B Slice ===================
      HierarchicalCoding            : 1           # B hierarchical coding ON/OFF (if OFF, no reference B is used)

      #=========== Entropy Coding ============
      SymbolMode                    : 1           # CAVLC: 0, SBAC: 1, only 1 supported, CAVLC implementation is not completed

      #=========== Loop/Deblock Filter =======
      LoopFilterDisable             : 0           # Disable loop filter in slice header (0=Filter, 1=No Filter)
      LoopFilterAlphaC0Offset       : 0           # Range: -26 ~ 26
      LoopFilterBetaOffset          : 0           # Range: -26 ~ 26

      #=========== Motion search =============
      FastSearch                    : 1           # 0:Full search  1:Diamond  2:PMVFAST(not supported) 
      SearchRange                   : 128         # (0: Search range is a Full frame)
      MaxDeltaQP                    : 0           # Absolute delta QP (1:default)
      
   2.3 Typical example
  
      Example 1) TAppEncoder.exe -c test.cfg -q 32 -g 8 -f 9 -s 64 -h 4
      (Hier-B)    -> QP 32, hierarchical-B GOP 8, 9 frames, 64x64-8x8 CU (~4x4 PU)

      Example 2) TAppEncoder.exe -c test.cfg -q 32 -g 4 -f 9 -s 64 -h 4 -1 LDC
      (Hier-P)    -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-8x8 CU (~4x4 PU)
			
      Example 3) TAppEncoder.exe -c test.cfg -q 32 -g 1 -f 9 -s 64 -h 4 -1 LDC
      (IPPP)      -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-8x8 CU (~4x4 PU)
      
3. Decoder option
   3.1 Parameters
        - TAppDecoder.exe -b test.bin -o test.yuv
          . Decode test.bin and make test.yuv as the reconstructed YUV
        - TAppDecoder.exe -b test.bin
          . Decode test.bin but YUV writing is skipped

4. Contact point

    =========================================================== 
    Woo-Jin Han, Principal Engineer
    (wjhan.han@samsung.com)

    M/M Platform Lab 

    Digital Media & Communications R&D Center 
    Digital Media & Communications Business
    Samsung Electronics Co. Ltd.

    Korea phone) 
    +82-31-279-8831 (office), +82-10-3329-6393 (cellular)
    ===========================================================
