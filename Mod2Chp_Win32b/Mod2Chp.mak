# Microsoft Developer Studio Generated NMAKE File, Based on Mod2Chp.dsp
!IF "$(CFG)" == ""
CFG=Mod2Chp - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Mod2Chp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Mod2Chp - Win32 Release" && "$(CFG)" !=\
 "Mod2Chp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mod2Chp.mak" CFG="Mod2Chp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mod2Chp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Mod2Chp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Mod2Chp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Mod2Chp.exe"

!ELSE 

ALL : "$(OUTDIR)\Mod2Chp.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Mod2Chp.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\Mod2Chp.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\Mod2Chp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c\
 
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Mod2Chp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\Mod2Chp.pdb" /machine:I386 /out:"$(OUTDIR)\Mod2Chp.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Mod2Chp.obj"

"$(OUTDIR)\Mod2Chp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Mod2Chp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Mod2Chp.exe"

!ELSE 

ALL : "$(OUTDIR)\Mod2Chp.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Mod2Chp.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\Mod2Chp.exe"
	-@erase "$(OUTDIR)\Mod2Chp.ilk"
	-@erase "$(OUTDIR)\Mod2Chp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_MBCS" /Fp"$(INTDIR)\Mod2Chp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Mod2Chp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\Mod2Chp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Mod2Chp.exe"\
 /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Mod2Chp.obj"

"$(OUTDIR)\Mod2Chp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "Mod2Chp - Win32 Release" || "$(CFG)" ==\
 "Mod2Chp - Win32 Debug"
SOURCE=.\Mod2Chp.cpp
DEP_CPP_MOD2C=\
	".\Mod2Chp.h"\
	

"$(INTDIR)\Mod2Chp.obj" : $(SOURCE) $(DEP_CPP_MOD2C) "$(INTDIR)"



!ENDIF 

