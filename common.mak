# UE Common Makefile settings for Pueblo/UE
#
.AutoDepend

# Specify debugging mode.  This can be one of the following:
#   0:  Release mode.       No internal debugging, linked to release libraries.
#   1:  Half debug mode.    Source files compiled with debugging info (but without
#                           internal debugging) and linked to the release libs.
#   2:  Full debug mode.    Both compiled and linked in debug mode.
# If the mode is 0, files will be created in the RELEASE directories - anything
# else will be created in the DEBUG directories.  If you change this mode, it
# would probably be worthwhile to execute a MAKE -B to recompile everything and
# ensure that old files left lying around don't confuse things.
DEBUG=0

# Define standard paths.
PROJROOT=D:\Pueblo
CCBIN=$(MAKEDIR)

CCROOT=$(CCBIN)\..
MFCINCLUDE=$(CCROOT)\Include\MFC
MFCLIB=
MNGINCLUDE=$(PROJROOT)\libmng;$(PROJROOT)\zlib;$(PROJROOT)\jpgsrc6b;$(PROJROOT)\lcms\include

!IF $(DEBUG) == 0
OUTDIR=$(PROJROOT)\release
INTDIR=release
!ELSE
OUTDIR=$(PROJROOT)\debug
INTDIR=debug
!ENDIF
.path.i=$(INTDIR)
.path.obj=$(INTDIR)
.path.res=$(INTDIR)
INCLUDE=$(PROJROOT)\Include;$(MNGINCLUDE);$(MFCINCLUDE);$(CCROOT)\Include
LIB=$(MFCLIB);$(CCROOT)\Lib

# Compilation tools
CPP=$(CCBIN)\bcc32.exe
CPPPP=$(CCBIN)\cpp32.exe
ASM=$(CCBIN)\tasm32.exe
LIB32=$(CCBIN)\tlib.exe
RSC=$(CCBIN)\brcc32.exe
LINK32=$(CCBIN)\ilink32.exe

# Common defines
CH_DEFS=CH_MSW;CH_CLIENT;CH_EXCEPTIONS
AFX_DEFS=_AFXDLL;_AFX_NOFORCE_LIBS;_AFX_PORTABLE;_CRT_PORTABLE;_AFX_NO_DEBUG_CRT
DLL_DEFS=_RTLDLL;_MBCS
VER_DEFS=_MSC_VER=1100;_X86_;WIN32;_WINDOWS;WINVER=0x0400
CDEFS=$(VER_DEFS);$(DLL_DEFS);$(AFX_DEFS);$(CH_DEFS)
RDEFS=-d$(CDEFS:;= /d)

# Utility parameters
CPP_NWARN=*hid *pia *aus *par *inl *pch
CPP_WARN=$(CPP_NWARN:*=-w-)
CPP_COMMON=-5 -a8 -b -D$(CDEFS) -I$(INCLUDE) -H=$(CSM) -Hh=headers.h -Hc \
  -tW -tWM -VF -VM -K -g0 -X- -w! $(CPP_WARN) $(NCDEFS)
!IF $(DEBUG)==0
# CPP_PROJ=$(CPP_COMMON) /O2 /Oi -v- /DNDEBUG
CPP_PROJ=$(CPP_COMMON) -v -O2 -Oi -OS -r /DNDEBUG
!ELIF $(DEBUG)==1
CPP_PROJ=$(CPP_COMMON) /Od -r- -y -v /DNDEBUG
!ELSE
CPP_PROJ=$(CPP_COMMON) /Od -r- -k -y -v /D_DEBUG
!ENDIF

RSC_COMMON=-l0x409 -fo$(RES_FILE) -x -i$(INCLUDE) $(RDEFS) $(NRDEFS)
!IF $(DEBUG)==2
RSC_PROJ=$(RSC_COMMON) /d_DEBUG
!ELSE
RSC_PROJ=$(RSC_COMMON) /dNDEBUG
!ENDIF

LINK32_COMMON=/Gn /s /aa /I$(INTDIR) /j$(INTDIR) -L$(LIB) $(NLDEFS)
!IF $(DEBUG)==0
LINK32_FLAGS=$(LINK32_COMMON) /v-
!ELSE
LINK32_FLAGS=$(LINK32_COMMON) /v
!ENDIF

#!IF $(DEBUG)==0
#LIB32_FLAGS=/C
#!ELSE
# The extra directory space is required because the debugging OBJs are larger...
LIB32_FLAGS=/C /P128
#!ENDIF

# Implicit rules
.c.obj:
   $(CPP) $(CPP_PROJ) -n$(@D) -c {$< }

.cpp.obj:
   $(CPP) $(CPP_PROJ) -n$(@D) -c {$< }

.cpp.i:
   $(CPPPP) -D$(CDEFS);_MT -I$(INCLUDE) -Sd -n$(@D) $<

# Libraries
!IF $(DEBUG)==2
BFC=bfcs42d bfc42d
!ELSE
BFC=bfcs42 bfc42
!ENDIF

LIB_API=$(OUTDIR)\Pueblo32.lib
LIB_APIUTIL=$(OUTDIR)\PbUtil32.lib
LIB_MNG=$(OUTDIR)\libmng.lib
