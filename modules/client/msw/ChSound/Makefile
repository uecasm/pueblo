# UE Makefile for Pueblo Sound Module DLL
#
# Define additional compilation parameters
CSM=$(INTDIR)\ChSound.csm
DEF_FILE=ChSound.def
RES_FILE=$(INTDIR)\ChSound.res
NCDEFS=-tWD
NRDEFS=
NLDEFS=/b:0x24000000 /Tpd

# Include common makefile information
!include "..\..\..\..\Common.mak"

# Define entry-point links
all : BuildDLL

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildDLL : IntDir KillCSM $(OUTDIR)\modules\ChSound.dll

LINK32_OBJS= \
	ChMPlay.obj \
	ChNotify.obj \
	ChSOpen.obj \
	ChSound.obj \
	ChSoundStream.obj \
	ChSoundUtils.obj \
	ChSpeechPrefs.obj \
	ChSPrefs.obj \
	ChTNT.obj \
	Dll.obj \
	headers.obj

LIBS=$(LIB_API) $(LIB_APIUTIL)

$(OUTDIR)\modules\ChSound.dll : $(DEF_FILE) $(RES_FILE) $(LIBS) $(LINK32_OBJS)
    $(LINK32) @&&|
  $(LINK32_FLAGS) c0d32.obj $(LINK32_OBJS)
  $@
  $*.map
  cw32mti import32 $(BFC) $(LIBS)
  $(DEF_FILE)
  $(RES_FILE)
|

DEP_RSC=\
	
	
$(RES_FILE) : ChSound.rc resource.h $(DEP_RSC)
   $(RSC) $(RSC_PROJ) ChSound.rc
