# Microsoft Developer Studio Project File - Name="TLibCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TLibCommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TLibCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TLibCommon.mak" CFG="TLibCommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TLibCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TLibCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TLibCommon - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/lib/Release"
# PROP Intermediate_Dir "../../../build/Lib/TLibCommon/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../TLibEncoder" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TLibCommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/lib/Debug"
# PROP Intermediate_Dir "../../../build/Lib/TLibCommon/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../TLibEncoder" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TLibCommon - Win32 Release"
# Name "TLibCommon - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SbacContextModel.cpp
# End Source File
# Begin Source File

SOURCE=.\SbacContextModel3DBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\TComAdaptiveLoopFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\TComBitStream.cpp
# End Source File
# Begin Source File

SOURCE=.\TComDataCU.cpp
# End Source File
# Begin Source File

SOURCE=.\TComLoopFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComMotionInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPattern.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPicSym.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPicYuv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPredFilter.cpp

!IF  "$(CFG)" == "TLibCommon - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "TLibCommon - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPrediction.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComRdCost.cpp

!IF  "$(CFG)" == "TLibCommon - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "TLibCommon - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TComRom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComSlice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComTrQuant.cpp
# End Source File
# Begin Source File

SOURCE=.\TComYuv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CommonDef.h
# End Source File
# Begin Source File

SOURCE=.\ContextTables.h
# End Source File
# Begin Source File

SOURCE=.\SbacContextModel.h
# End Source File
# Begin Source File

SOURCE=.\SbacContextModel3DBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SbacTables.h
# End Source File
# Begin Source File

SOURCE=.\TComAdaptiveLoopFilter.h
# End Source File
# Begin Source File

SOURCE=.\TComBitcounter.h
# End Source File
# Begin Source File

SOURCE=.\TComBitStream.h
# End Source File
# Begin Source File

SOURCE=.\TComDataCU.h
# End Source File
# Begin Source File

SOURCE=.\TComGlobalMotion.h
# End Source File
# Begin Source File

SOURCE=.\TComList.h
# End Source File
# Begin Source File

SOURCE=.\TComLoopFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComMotionInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComMV.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPattern.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPicSym.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPicYuv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPredFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComPrediction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComRdCost.h
# End Source File
# Begin Source File

SOURCE=.\TComRom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibCommon\TComTrQuant.h
# End Source File
# Begin Source File

SOURCE=.\TComYuv.h
# End Source File
# Begin Source File

SOURCE=.\TypeDef.h
# End Source File
# End Group
# End Target
# End Project
