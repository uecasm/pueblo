# UE Makefile for Pueblo World Module DLL
#
# Define additional compilation parameters
CSM=$(INTDIR)\ChWorld.csm
DEF_FILE=ChWorld.def
RES_FILE=$(INTDIR)\ChWorld.res
NCDEFS=-tWD -DZLIB_DLL
NRDEFS=
NLDEFS=/b:0x20000000 /Tpd

# Include common makefile information
!include "..\..\..\..\Common.mak"

# Define entry-point links
all : BuildDLL

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildDLL : IntDir KillCSM $(OUTDIR)\modules\ChWorld.dll
    
LINK32_OBJS= \
	ChAbout.obj \
	ChConnectDlg.obj \
	ChHist.obj \
	ChKeyMapType.obj \
	ChMCCP.obj \
	ChPrefsNotify.obj \
	ChPrefsWorld.obj \
	ChQuickConnect.obj \
	ChSaveAs.obj \
	ChSCWiz.obj \
	ChTextInput.obj \
	ChTextOutput.obj \
	ChWConn.obj \
	ChWInfo.obj \
	ChWList.obj \
	ChWListD.obj \
	ChWorld.obj \
	ChWorldCmdLine.obj \
	ChWorldStream.obj \
	Dll.obj

LIBS=$(LIB_API) $(LIB_APIUTIL) $(INTDIR)\TinTin.lib $(LIB_MNG)

$(OUTDIR)\modules\ChWorld.dll : $(DEF_FILE) $(RES_FILE) $(LIBS) $(LINK32_OBJS)
    $(LINK32) @&&|
  $(LINK32_FLAGS) c0d32.obj $(LINK32_OBJS)
  $@
  $*.map
  cw32mti import32 $(BFC) $(LIBS)
  $(DEF_FILE)
  $(RES_FILE)
|

LIB32_OBJ1= \
	pfx_TinTinAction.obj \
	pfx_TinTinAlias.obj \
	pfx_TinTinFiles.obj \
	pfx_TinTinIf.obj \
	pfx_TinTinInfo.obj \
	pfx_TinTinList.obj \
	pfx_TinTinMain.obj \
	pfx_TinTinMisc.obj \
	pfx_TinTinParse.obj \
	pfx_TinTinPath.obj \
	pfx_TinTinSession.obj \
	pfx_TinTinSettings.obj \
	pfx_TinTinSubs.obj \
	pfx_TinTinTicks.obj \
	pfx_TinTinUtils.obj \
	pfx_TinTinVars.obj

#HACK: for some unknown reason $(INTDIR) isn't expanding in the _OBJS
#      substitution, so I've put the value in directly.
LIB32_OBJD=$(LIB32_OBJ1:pfx_=)
!IF "$(DEBUG)" == "0"
LIB32_OBJS=$(LIB32_OBJ1:pfx_=&^
+ release^\)
!ELSE
LIB32_OBJS=$(LIB32_OBJ1:pfx_=&^
+ debug^\)
!ENDIF

$(INTDIR)\TinTin.lib : $(LIB32_OBJD)
  @if exist $(INTDIR)\TinTin.lib del $(INTDIR)\TinTin.lib
  $(LIB32) @&&|
  $(LIB32_FLAGS) $(INTDIR)\TinTin.lib $(LIB32_OBJS)
|
	if exist $(CSM) del $(CSM)

DEP_RSC=\
	res\chaco.ico \
	res\planet.ico \
	res\wheel.ico
	
$(RES_FILE) : ChWorld.rc resource.h $(DEP_RSC)
   $(RSC) $(RSC_PROJ) ChWorld.rc

