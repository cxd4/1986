# Microsoft Developer Studio Project File - Name="1964" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=1964 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "1964.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "1964.mak" CFG="1964 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "1964 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "1964 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "1964 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "1964___Win32_Debug"
# PROP BASE Intermediate_Dir "1964___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../make/obj/dbg_1964"
# PROP Intermediate_Dir "../make/obj/dbg_1964"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /GX /ZI /I ".." /D "WIN32" /D "WINDEBUG_1964" /D "_WINDOWS" /D "_MBCS" /D "DEBUG_COMMON" /D "_DEBUG" /D "VIDEO" /Fr /YX /FD /c
# ADD CPP /nologo /Gr /MTd /ZI /I ".." /D "DYNAREC" /D "DYN_DEBUG" /D "DEBUG_COMMON" /D "WIN32" /D "WINDEBUG_1964" /D "_WINDOWS" /D "_MBCS" /D "_DEBUG" /D "VIDEO" /D "ZIP_SUPPORT" /D "HLE" /D "GRAPHICS_TRACER" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WINDEBUG_1964"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WINDEBUG_1964"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /out:"../Debug/dbg_1964.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dxguid.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib opengl32.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /out:"../make/Debug/1964dbug.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "1964___Win32_Release"
# PROP BASE Intermediate_Dir "1964___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../make/obj/obj_1964"
# PROP Intermediate_Dir "../make/obj/obj_1964"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Gr /W3 /Zi /O2 /Ob2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAs /Fr /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /Gr /MT /W3 /Zi /O1 /Ob0 /I ".." /D "DYNAREC" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "NOSOUND" /FAs /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib ole32.lib uuid.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"../Release/1964.exe"
# SUBTRACT BASE LINK32 /profile /debug /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../make/Release/1964.exe"
# SUBTRACT LINK32 /profile /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "1964 - Win32 Debug"
# Name "1964 - Win32 Release"
# Begin Group "1964"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Interpreter"

# PROP Default_Filter ""
# Begin Group "Header Files Interpreter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\r4300i.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\FPU.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\i_chips.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\r4300i.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\tlb.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# End Group
# Begin Group "Dynarec"

# PROP Default_Filter ""
# Begin Group "Header Files Dynarec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\dynarec\dynarec.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\dynarec\dynarec.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\dynarec\dynarec_chips.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# ADD CPP /D "SAFE_DYNAREC"
# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Group "Header Files win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\win32\Dll_Audio.h
# End Source File
# Begin Source File

SOURCE=..\win32\Dll_Input.h
# End Source File
# Begin Source File

SOURCE=..\win32\Dll_Video.h
# End Source File
# Begin Source File

SOURCE=..\plugins.h
# End Source File
# Begin Source File

SOURCE=..\win32\registry.h
# End Source File
# Begin Source File

SOURCE=..\win32\windebug.h
# End Source File
# Begin Source File

SOURCE=..\win32\wingui.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\win32\Dll_Audio.c
# End Source File
# Begin Source File

SOURCE=..\win32\Dll_Input.c
# End Source File
# Begin Source File

SOURCE=..\win32\Dll_Video.c
# End Source File
# Begin Source File

SOURCE=..\win32\registry.c
# End Source File
# Begin Source File

SOURCE=..\win32\windebug.c
# End Source File
# Begin Source File

SOURCE=..\win32\wingui.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\controller.h
# End Source File
# Begin Source File

SOURCE=..\DbgPrint.h
# End Source File
# Begin Source File

SOURCE=..\globals.h
# End Source File
# Begin Source File

SOURCE=..\interrupt.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\controller.c
# End Source File
# Begin Source File

SOURCE=..\DebugR4300i.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\dma.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\emulators.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fileio.c
# End Source File
# Begin Source File

SOURCE=..\globals.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\interrupt.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\memory.c

!IF  "$(CFG)" == "1964 - Win32 Debug"

!ELSEIF  "$(CFG)" == "1964 - Win32 Release"

# SUBTRACT CPP /D "DYNAREC"

!ENDIF 

# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=..\zlib\infblock.c
# End Source File
# Begin Source File

SOURCE=..\zlib\infblock.h
# End Source File
# Begin Source File

SOURCE=..\zlib\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\zlib\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\zlib\infutil.c
# End Source File
# Begin Source File

SOURCE=..\zlib\infutil.h
# End Source File
# Begin Source File

SOURCE=..\zlib\maketree.c
# End Source File
# Begin Source File

SOURCE=..\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\zlib\unzip.c
# End Source File
# Begin Source File

SOURCE=..\zlib\unzip.h
# End Source File
# Begin Source File

SOURCE=..\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\zlib\zutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\win32\icon2.ico
# End Source File
# Begin Source File

SOURCE=..\win32\logo.bmp
# End Source File
# Begin Source File

SOURCE=..\win32\wingui.rc
# End Source File
# End Group
# Begin Source File

SOURCE=..\COPYRIGHT.txt
# End Source File
# End Target
# End Project
