# UE Makefile for Pueblo Client application
#
# Define additional compilation parameters
CSM=$(INTDIR)\Pueblo.csm
DEF_FILE=
RES_FILE=$(INTDIR)\Pueblo.res
NCDEFS=-DCH_NO_WEBTRACKER
NLDEFS=/Tpe
NRDEFS=$(NCDEFS:-D=/d)

# Include common makefile information
!include "..\..\Common.mak"

# Define entry-point links
all : BuildEXE

run : all
	@start $(OUTDIR)\Pueblo.exe

debug : all
	@start td32 -sd$(PROJROOT)\modules\client\msw\chworld;$(PROJROOT)\client\msw;$(PROJROOT)\api;$(PROJROOT)\apiutil -t$(OUTDIR) $(OUTDIR)\Pueblo.exe

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildEXE : IntDir KillCSM $(OUTDIR)\Pueblo.exe

LINK32_OBJS= \
	ChAbout.obj			\
	ChAccDlg.obj		\
	ChBaseVw.obj		\
	ChClCore.obj		\
	ChCoreHandlers.obj	\
	ChCoreStream.obj	\
	ChLicDlg.obj		\
	ChMFrame.obj		\
	ChNList.obj			\
	ChOODDlg.obj		\
	ChPagMgr.obj		\
	ChPbModuleMgr.obj	\
	ChPrApps.obj		\
	ChPrDbg.obj			\
	ChPrefs.obj			\
	ChPrefsProxy.obj	\
	ChPrefsTrace.obj	\
	ChPrFont.obj		\
	ChPrNet.obj			\
	ChPuebloFrameMgr.obj	\
	ChRunTimeInfo.obj	\
	headers.obj			\
	Pueblo.obj

LIBS=$(LIB_APIUTIL) $(LIB_API)

$(OUTDIR)\Pueblo.exe : $(DEF_FILE) $(RES_FILE) $(LINK32_OBJS) $(LIBS)
    $(LINK32) @&&|
  $(LINK32_FLAGS) c0w32.obj $(LINK32_OBJS)
  $@
  $*.map
  cw32mti import32 $(BFC) $(LIBS)
  $(DEF_FILE)
  $(RES_FILE)
|

DEP_RSC=\
	res\idr_main.ico \
	res\idr_shor.ico \
	res\toolbar.bmp \
	res\idr_disc.htm \
	res\chaco.dib \
	res\uelogo.dib
	
$(RES_FILE) : Pueblo.rc resource.h $(DEP_RSC)
   $(RSC) $(RSC_PROJ) Pueblo.rc
