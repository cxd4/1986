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
!MESSAGE "1964 - Win32 Release Opcode Debugger" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/1964", RDAAAAAA"
# PROP Scc_LocalPath "."
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
# PROP Output_Dir "obj/Debug"
# PROP Intermediate_Dir "obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /GX /ZI /I ".." /D "WIN32" /D "WINDEBUG_1964" /D "_WINDOWS" /D "_MBCS" /D "DEBUG_COMMON" /D "_DEBUG" /D "VIDEO" /Fr /YX /FD /c
# ADD CPP /nologo /Gr /Zp16 /MTd /W3 /Gm /Gi /ZI /I ".." /D "DYNAREC" /D "DYN_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "VIDEO" /D "ZIP_SUPPORT" /D "HLE" /D "GRAPHICS_TRACER" /D "ZLIB_DLL" /D "ENABLE_64BIT_FPU" /D "DEBUG_COMMON" /D "WINDEBUG_1964" /D "_DEBUG" /FAcs /Fr /YX /FD /c
# SUBTRACT CPP /WX
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
# ADD LINK32 dxguid.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib opengl32.lib zlib/zlib.lib comctl32.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /out:"../Release/1964dbug.exe"
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
# PROP Output_Dir "obj/Release"
# PROP Intermediate_Dir "obj/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Gr /W3 /Zi /O2 /Ob2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAs /Fr /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /Gr /MT /W3 /Gi /O1 /Op /I ".." /D "DYNAREC" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "ZLIB_DLL" /D "WIN32_LEAN_AND_MEAN" /D "ENABLE_64BIT_FPU" /FAcs /FR /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib zlib\zlib.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../Release/1964.exe"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "1964 - Win32 Release Opcode Debugger"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "1964___Win32_Release_Opcode_Debugger"
# PROP BASE Intermediate_Dir "1964___Win32_Release_Opcode_Debugger"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/Release_Opcode_Debugger"
# PROP Intermediate_Dir "obj/Release_Opcode_Debugger"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Gr /MT /W3 /Gi /O1 /Op /I ".." /D "DYNAREC" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "ZLIB_DLL" /D "WIN32_LEAN_AND_MEAN" /D "ENABLE_64BIT_FPU" /FAcs /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /Gr /MT /W3 /Gi /O1 /Op /I ".." /D "ENABLE_OPCODE_DEBUGGER" /D "DYNAREC" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "ZLIB_DLL" /D "WIN32_LEAN_AND_MEAN" /D "ENABLE_64BIT_FPU" /FAcs /FR /FD /c
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
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib zlib\zlib.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../Release/1964.exe"
# SUBTRACT BASE LINK32 /profile /pdb:none /incremental:yes /debug /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib zlib\zlib.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"../Release/1964-opcodedebugger.exe"
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /map /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "1964 - Win32 Debug"
# Name "1964 - Win32 Release"
# Name "1964 - Win32 Release Opcode Debugger"
# Begin Group "1964"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "r4300i"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DbgPrint.h
# End Source File
# Begin Source File

SOURCE=.\DebugR4300i.c
# End Source File
# Begin Source File

SOURCE=.\dma.c
# End Source File
# Begin Source File

SOURCE=.\dma.h
# End Source File
# Begin Source File

SOURCE=.\FPU.c
# End Source File
# Begin Source File

SOURCE=.\i_chips.c
# End Source File
# Begin Source File

SOURCE=.\interrupt.c
# End Source File
# Begin Source File

SOURCE=.\interrupt.h
# End Source File
# Begin Source File

SOURCE=.\R4300i.c
# End Source File
# Begin Source File

SOURCE=.\r4300i.h
# End Source File
# Begin Source File

SOURCE=.\Tlb.c
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cheatcode.c
# End Source File
# Begin Source File

SOURCE=.\cheatcode.h
# End Source File
# Begin Source File

SOURCE=.\win32\registry.c
# End Source File
# Begin Source File

SOURCE=.\win32\registry.h
# End Source File
# Begin Source File

SOURCE=.\romlist.c
# End Source File
# Begin Source File

SOURCE=.\romlist.h
# End Source File
# Begin Source File

SOURCE=.\win32\windebug.c
# End Source File
# Begin Source File

SOURCE=.\win32\windebug.h
# End Source File
# Begin Source File

SOURCE=.\win32\wingui.c
# End Source File
# Begin Source File

SOURCE=.\win32\wingui.h
# End Source File
# Begin Source File

SOURCE=.\win32\wingui2.c
# End Source File
# End Group
# Begin Group "N64 Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\1964ini.c
# End Source File
# Begin Source File

SOURCE=.\1964ini.h
# End Source File
# Begin Source File

SOURCE=.\compiler.c
# End Source File
# Begin Source File

SOURCE=.\compiler.h
# End Source File
# Begin Source File

SOURCE=.\debug_option.c
# End Source File
# Begin Source File

SOURCE=.\debug_option.h
# End Source File
# Begin Source File

SOURCE=.\emulator.c
# End Source File
# Begin Source File

SOURCE=.\emulator.h
# End Source File
# Begin Source File

SOURCE=.\fileio.c
# End Source File
# Begin Source File

SOURCE=.\fileio.h
# End Source File
# Begin Source File

SOURCE=.\gamesave.c
# End Source File
# Begin Source File

SOURCE=.\gamesave.h
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\Globals.h
# End Source File
# Begin Source File

SOURCE=.\hardware.h
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\memory2.c
# End Source File
# Begin Source File

SOURCE=.\n64rcp.c
# End Source File
# Begin Source File

SOURCE=.\n64rcp.h
# End Source File
# Begin Source File

SOURCE=.\netplay.c
# End Source File
# Begin Source File

SOURCE=.\netplay.h
# End Source File
# Begin Source File

SOURCE=.\profiler.c
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# End Group
# Begin Group "Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32\Dll_Audio.c
# End Source File
# Begin Source File

SOURCE=.\win32\Dll_Audio.h
# End Source File
# Begin Source File

SOURCE=.\win32\Dll_Input.c
# End Source File
# Begin Source File

SOURCE=.\win32\Dll_Input.h
# End Source File
# Begin Source File

SOURCE=.\win32\DLL_Rsp.c
# End Source File
# Begin Source File

SOURCE=.\win32\DLL_Rsp.h
# End Source File
# Begin Source File

SOURCE=.\win32\Dll_Video.c
# End Source File
# Begin Source File

SOURCE=.\win32\Dll_Video.h
# End Source File
# Begin Source File

SOURCE=.\plugins.h
# End Source File
# End Group
# Begin Group "Controller"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iPIF.c
# End Source File
# Begin Source File

SOURCE=.\iPIF.h
# End Source File
# Begin Source File

SOURCE=.\Kaillera\Kaillera.c
# End Source File
# Begin Source File

SOURCE=.\Kaillera\Kaillera.h
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\unzip.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.h
# End Source File
# End Group
# Begin Group "Dynarec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dynaRec\Analyze.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaBranch.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaCOP1.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaCOP1.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaCPU.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaCPU.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaCPU_defines.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaHelper.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaLog.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaLog.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\dynaRec.h
# End Source File
# Begin Source File

SOURCE=.\hle.c
# End Source File
# Begin Source File

SOURCE=.\hle.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\OpcodeDebugger.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\opcodeDebugger.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\regcache.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\regcache.h
# End Source File
# Begin Source File

SOURCE=.\dynaRec\x86.c
# End Source File
# Begin Source File

SOURCE=.\dynaRec\x86.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\win32\19644.ico
# End Source File
# Begin Source File

SOURCE=.\win32\19646.ico
# End Source File
# Begin Source File

SOURCE=.\win32\audio.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\win32\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\win32\input.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\win32\rom_open.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\video.bmp
# End Source File
# Begin Source File

SOURCE=.\win32\wingui.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\COPYRIGHT.txt
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.lib
# End Source File
# End Target
# End Project
