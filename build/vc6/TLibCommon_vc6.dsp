# Microsoft Developer Studio Project File - Name="TLibCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TLibCommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TLibCommon_vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TLibCommon_vc6.mak" CFG="TLibCommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TLibCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TLibCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "TLibCommon - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\source\Lib\\" /I "..\compat\msvc" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /GR /GX /ZI /Od /I "..\..\source\Lib\\" /I "..\..\compat\msvc" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D max=_cpp_max /D min=_cpp_min /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TLibCommon - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Ot /Ob2 /I "..\source\Lib\\" /I "..\compat\msvc" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /GF /c
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

# Name "TLibCommon - Win32 Debug"
# Name "TLibCommon - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\ContextModel.cpp
DEP_CPP_CONTE=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\ContextModel3DBuffer.cpp
DEP_CPP_CONTEX=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextModel.h"\
	"..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\libmd5\libmd5.c
DEP_CPP_LIBMD=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\libmd5\libmd5.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.cpp
DEP_CPP_TCOMA=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
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
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComBitStream.cpp
DEP_CPP_TCOMB=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComCABACTables.cpp
DEP_CPP_TCOMC=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComCABACTables.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComDataCU.cpp
DEP_CPP_TCOMD=\
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
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComInterpolationFilter.cpp
DEP_CPP_TCOMI=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComLoopFilter.cpp
DEP_CPP_TCOML=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComLoopFilter.h"\
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
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComMotionInfo.cpp
DEP_CPP_TCOMM=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComMotionInfo.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPattern.cpp
DEP_CPP_TCOMP=\
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
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPic.cpp
DEP_CPP_TCOMPI=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\SEI.h"\
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
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPicSym.cpp
DEP_CPP_TCOMPIC=\
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
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPicYuv.cpp
DEP_CPP_TCOMPICY=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPicYuvMD5.cpp
DEP_CPP_TCOMPICYU=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\libmd5\libmd5.h"\
	"..\..\source\Lib\libmd5\MD5.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPrediction.cpp
DEP_CPP_TCOMPR=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
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
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRdCost.cpp
DEP_CPP_TCOMR=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.cpp
DEP_CPP_TCOMRD=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComList.h"\
	"..\..\source\Lib\TLibCommon\TComMv.h"\
	"..\..\source\Lib\TLibCommon\TComPattern.h"\
	"..\..\source\Lib\TLibCommon\TComRdCost.h"\
	"..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRom.cpp
DEP_CPP_TCOMRO=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.cpp
DEP_CPP_TCOMS=\
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
	"..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h"\
	"..\..\source\Lib\TLibCommon\TComSlice.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComSlice.cpp
DEP_CPP_TCOMSL=\
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
	"..\..\source\Lib\TLibDecoder\TDecBinCoder.h"\
	"..\..\source\Lib\TLibDecoder\TDecEntropy.h"\
	"..\..\source\Lib\TLibDecoder\TDecSbac.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoder.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABAC.h"\
	"..\..\source\Lib\TLibEncoder\TEncBinCoderCABACCounter.h"\
	"..\..\source\Lib\TLibEncoder\TEncEntropy.h"\
	"..\..\source\Lib\TLibEncoder\TEncSbac.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComTrQuant.cpp
DEP_CPP_TCOMT=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
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
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComWeightPrediction.cpp
DEP_CPP_TCOMW=\
	"..\..\compat\msvc\stdint.h"\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\ContextTables.h"\
	"..\..\source\Lib\TLibCommon\TComBitStream.h"\
	"..\..\source\Lib\TLibCommon\TComDataCU.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
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
	"..\..\source\Lib\TLibCommon\TComTrQuant.h"\
	"..\..\source\Lib\TLibCommon\TComWeightPrediction.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComYuv.cpp
DEP_CPP_TCOMY=\
	"..\..\source\Lib\TLibCommon\CommonDef.h"\
	"..\..\source\Lib\TLibCommon\TComInterpolationFilter.h"\
	"..\..\source\Lib\TLibCommon\TComPicYuv.h"\
	"..\..\source\Lib\TLibCommon\TComRom.h"\
	"..\..\source\Lib\TLibCommon\TComYuv.h"\
	"..\..\source\Lib\TLibCommon\TypeDef.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\AccessUnit.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\CommonDef.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\ContextModel.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\ContextModel3DBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\ContextTables.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\libmd5\libmd5.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\libmd5\MD5.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\NAL.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\SEI.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComAdaptiveLoopFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComBitCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComBitStream.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComCABACTables.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComDataCU.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComInterpolationFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComList.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComLoopFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComMotionInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComMv.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPattern.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPic.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPicSym.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPicYuv.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComPrediction.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRdCost.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRdCostWeightPrediction.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComRom.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComSampleAdaptiveOffset.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComTrQuant.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComWeightPrediction.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TComYuv.h
# End Source File
# Begin Source File

SOURCE=..\..\source\Lib\TLibCommon\TypeDef.h
# End Source File
# End Group
# End Target
# End Project
