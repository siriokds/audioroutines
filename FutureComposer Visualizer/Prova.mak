# Microsoft Developer Studio Generated NMAKE File, Based on Prova02c.dsp
!IF "$(CFG)" == ""
CFG=Prova02c - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Prova02c - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Prova02c - Win32 Release" && "$(CFG)" !=\
 "Prova02c - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Prova02c.mak" CFG="Prova02c - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Prova02c - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Prova02c - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Prova02c - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Prova02c.exe"

!ELSE 

ALL : "$(OUTDIR)\Prova02c.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\FC16.obj"
	-@erase "$(INTDIR)\Mixer.obj"
	-@erase "$(INTDIR)\Prova02.res"
	-@erase "$(INTDIR)\Prova02c.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\WOut.obj"
	-@erase "$(OUTDIR)\Prova02c.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Prova02c.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x410 /fo"$(INTDIR)\Prova02.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Prova02c.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\Prova02c.pdb" /machine:I386 /out:"$(OUTDIR)\Prova02c.exe" 
LINK32_OBJS= \
	"$(INTDIR)\FC16.obj" \
	"$(INTDIR)\Mixer.obj" \
	"$(INTDIR)\Prova02.res" \
	"$(INTDIR)\Prova02c.obj" \
	"$(INTDIR)\WOut.obj"

"$(OUTDIR)\Prova02c.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Prova02c - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Prova02c.exe"

!ELSE 

ALL : "$(OUTDIR)\Prova02c.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\FC16.obj"
	-@erase "$(INTDIR)\Mixer.obj"
	-@erase "$(INTDIR)\Prova02.res"
	-@erase "$(INTDIR)\Prova02c.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\WOut.obj"
	-@erase "$(OUTDIR)\Prova02c.exe"
	-@erase "$(OUTDIR)\Prova02c.ilk"
	-@erase "$(OUTDIR)\Prova02c.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Prova02c.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x410 /fo"$(INTDIR)\Prova02.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Prova02c.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\Prova02c.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\Prova02c.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\FC16.obj" \
	"$(INTDIR)\Mixer.obj" \
	"$(INTDIR)\Prova02.res" \
	"$(INTDIR)\Prova02c.obj" \
	"$(INTDIR)\WOut.obj"

"$(OUTDIR)\Prova02c.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "Prova02c - Win32 Release" || "$(CFG)" ==\
 "Prova02c - Win32 Debug"
SOURCE=.\Prova02c.cpp
DEP_CPP_PROVA=\
	".\FC16.h"\
	".\types.h"\
	

"$(INTDIR)\Prova02c.obj" : $(SOURCE) $(DEP_CPP_PROVA) "$(INTDIR)"


SOURCE=.\FC16.cpp
DEP_CPP_FC16_=\
	".\FC16.h"\
	".\Mixer.h"\
	".\types.h"\
	

"$(INTDIR)\FC16.obj" : $(SOURCE) $(DEP_CPP_FC16_) "$(INTDIR)"


SOURCE=.\Mixer.cpp

!IF  "$(CFG)" == "Prova02c - Win32 Release"

DEP_CPP_MIXER=\
	".\Mixer.h"\
	".\types.h"\
	".\WOut.h"\
	

"$(INTDIR)\Mixer.obj" : $(SOURCE) $(DEP_CPP_MIXER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Prova02c - Win32 Debug"

DEP_CPP_MIXER=\
	".\Mixer.h"\
	".\types.h"\
	".\WOut.h"\
	

"$(INTDIR)\Mixer.obj" : $(SOURCE) $(DEP_CPP_MIXER) "$(INTDIR)"


!ENDIF 

SOURCE=.\WOut.cpp
DEP_CPP_WOUT_=\
	".\types.h"\
	".\WOut.h"\
	

"$(INTDIR)\WOut.obj" : $(SOURCE) $(DEP_CPP_WOUT_) "$(INTDIR)"


SOURCE=.\Prova02.rc
DEP_RSC_PROVA0=\
	"F:\fc01.fc6"\
	

"$(INTDIR)\Prova02.res" : $(SOURCE) $(DEP_RSC_PROVA0) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

