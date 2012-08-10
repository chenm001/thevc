# Microsoft Developer Studio Project File - Name="TAppCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TAppCommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TAppCommon_vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TAppCommon_vc6.mak" CFG="TAppCommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TAppCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TAppCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "TAppCommon - Win32 Debug"

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
# ADD CPP /nologo /MTd /w /W0 /Gm /GR /GX /ZI /Od /I "..\source\Lib\\" /I "..\compat\msvc" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D max=_cpp_max /D min=_cpp_min /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TAppCommon - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /Zi /Ot /Ob2 /I "..\source\Lib\\" /I "..\compat\msvc" /D "_LIB" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D max=_cpp_max /D min=_cpp_min /GF /c
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

# Name "TAppCommon - Win32 Debug"
# Name "TAppCommon - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=..\..\source\Lib\TAppCommon\program_options_lite.cpp
DEP_CPP_PROGR=\
	"..\..\source\Lib\TAppCommon\program_options_lite.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=..\..\source\Lib\TAppCommon\program_options_lite.h
# End Source File
# End Group
# End Target
# End Project
