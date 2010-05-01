# Microsoft Developer Studio Project File - Name="TLibEncoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TLibEncoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TLibEncoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TLibEncoder.mak" CFG="TLibEncoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TLibEncoder - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TLibEncoder - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TLibEncoder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/lib/Release"
# PROP Intermediate_Dir "../../../build/Lib/TLibEncoder/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../TLibCommon" /I "../TLibVideoIO" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TLibEncoder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/lib/Debug"
# PROP Intermediate_Dir "../../../build/Lib/TLibEncoder/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../TLibCommon" /I "../TLibVideoIO" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "TLibEncoder - Win32 Release"
# Name "TLibEncoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\TEncAdaptiveLoopFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\TEncAnalyze.cpp
# End Source File
# Begin Source File

SOURCE=.\TEncCavlc.cpp
# End Source File
# Begin Source File

SOURCE=.\TEncCu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncEntropy.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncGOP.cpp
# End Source File
# Begin Source File

SOURCE=.\TEncSbac.cpp
# End Source File
# Begin Source File

SOURCE=.\TEncSearch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncSlice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncTop.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\TEncAnalyze.h
# End Source File
# Begin Source File

SOURCE=.\TEncCavlc.h
# End Source File
# Begin Source File

SOURCE=.\TEncCfg.h
# End Source File
# Begin Source File

SOURCE=.\TEncCu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncEntropy.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncGOP.h
# End Source File
# Begin Source File

SOURCE=.\TEncSbac.h
# End Source File
# Begin Source File

SOURCE=.\TEncSearch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncSlice.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Lib\TLibEncoder\TEncTop.h
# End Source File
# End Group
# End Target
# End Project
