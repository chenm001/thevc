# Microsoft Developer Studio Project File - Name="TLibEncoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TLibEncoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TLibEncoder_vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TLibEncoder_vc6.mak" CFG="TLibEncoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TLibEncoder - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TLibEncoder - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "TLibEncoder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\source\Lib\\" /I "..\..\compat\msvc" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /GR /GX /ZI /Od /I "..\..\source\Lib\\" /I "..\..\compat\msvc" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D max=_cpp_max /D min=_cpp_min /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TLibEncoder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Ot /Ob2 /I "..\..\source\Lib\\" /I "..\..\compat\msvc" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /GF /c
# ADD CPP /nologo /MT /w /W0 /GR /GX /Zi /Ot /Ob2 /I "..\..\source\Lib\\" /I "..\..\compat\msvc" /D "_LIB" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D max=_cpp_max /D min=_cpp_min /GF /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TLibEncoder - Win32 Debug"
# Name "TLibEncoder - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\NALwrite.cpp
DEP_CPP_NALWR=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\NAL.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\NALwrite.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\SEIwrite.cpp
DEP_CPP_SEIWR=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\SEI.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\SEIwrite.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.cpp
DEP_CPP_TENCA=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncAnalyze.cpp
DEP_CPP_TENCAN=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.cpp
DEP_CPP_TENCB=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.cpp
DEP_CPP_TENCBI=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncCavlc.cpp
DEP_CPP_TENCC=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\SEIwrite.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncCu.cpp
DEP_CPP_TENCCU=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\AccessUnit.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncCu.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncGOP.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	"..\..\source\Lib\TLibEncoder\TEncSlice.h"\
	"..\..\source\Lib\TLibEncoder\TEncTop.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	"..\..\source\Lib\TLibVideoIO\TVideoIOYuv.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncEntropy.cpp
DEP_CPP_TENCE=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncGOP.cpp
DEP_CPP_TENCG=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\libmd5\libmd5.h"\
	"..\..\source\Lib\libmd5\MD5.h"\
	"..\..\source\Lib\TLibCommon\AccessUnit.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\NAL.h"\
	"..\..\source\Lib\TLibCommon\SEI.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\NALwrite.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncCu.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncGOP.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	"..\..\source\Lib\TLibEncoder\TEncSlice.h"\
	"..\..\source\Lib\TLibEncoder\TEncTop.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	"..\..\source\Lib\TLibVideoIO\TVideoIOYuv.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncPic.cpp
DEP_CPP_TENCP=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncPreanalyzer.cpp
DEP_CPP_TENCPR=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncRateCtrl.cpp
DEP_CPP_TENCR=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.cpp
DEP_CPP_TENCS=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSbac.cpp
DEP_CPP_TENCSB=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\AccessUnit.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncCu.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncGOP.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	"..\..\source\Lib\TLibEncoder\TEncSlice.h"\
	"..\..\source\Lib\TLibEncoder\TEncTop.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	"..\..\source\Lib\TLibVideoIO\TVideoIOYuv.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSearch.cpp
DEP_CPP_TENCSE=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSlice.cpp
DEP_CPP_TENCSL=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\AccessUnit.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncCu.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncGOP.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	"..\..\source\Lib\TLibEncoder\TEncSlice.h"\
	"..\..\source\Lib\TLibEncoder\TEncTop.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	"..\..\source\Lib\TLibVideoIO\TVideoIOYuv.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncTop.cpp
DEP_CPP_TENCT=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\AccessUnit.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitCounter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibEncoder\TEncAnalyze.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncCfg.h"\
	"..\..\source\Lib\TLibEncoder\TEncCu.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncGOP.h"\
	"..\..\source\Lib\TLibEncoder\TEncPic.h"\
	"..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h"\
	"..\..\source\Lib\TLibEncoder\TEncRateCtrl.h"\
	"..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncSearch.h"\
	"..\..\source\Lib\TLibEncoder\TEncSlice.h"\
	"..\..\source\Lib\TLibEncoder\TEncTop.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	"..\..\source\Lib\TLibVideoIO\TVideoIOYuv.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\WeightPredAnalysis.cpp
DEP_CPP_WEIGH=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComPic.h"\
	"..\..\source\Lib\TLibCommon\TComPicSym.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	"..\..\source\Lib\TLibEncoder\TEncCavlc.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\AnnexBwrite.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\NALwrite.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\SEIwrite.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncAdaptiveLoopFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncAnalyze.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncBinCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncCavlc.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncCfg.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncCu.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncEntropy.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncGOP.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncPic.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncPreanalyzer.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncRateCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSampleAdaptiveOffset.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSbac.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSearch.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\TEncTop.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibEncoder\WeightPredAnalysis.h
# End Source File
# End Group
# End Target
# End Project
