# UE Makefile for Pueblo API DLL
#
# Define additional compilation parameters
CSM = $(INTDIR)\Pueblo32.csm
DEF_FILE=Pueblo32.def
RES_FILE=$(INTDIR)\Pueblo32.res
NCDEFS=-WD -D_AFXEXT
# Warning PCH occurs in <afxtempl.h> only in DEBUG mode.
NRDEFS=/d_AFXEXT
NLDEFS=/Gi /b:0x10000000 /Tpd

# Include common makefile information
!include "..\Common.mak"

# Define library filename
DLL_API=$(LIB_API:.lib=.dll)

# Define entry-point links
all : BuildDLL

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildDLL : OutDir IntDir KillCSM $(DLL_API)

LINK32_OBJS= \
	ChAcct.obj \
	ChArch.obj \
	ChArgList.obj \
	ChBodyElement.obj \
	ChBodyTags.obj \
	ChClInfo.obj \
	ChConn.obj \
	ChCore.obj \
	ChCreateTag.obj \
	ChData.obj \
	ChDb.obj \
	ChDDE.obj \
	ChDDECli.obj \
	ChDDESrv.obj \
	ChDefnTag.obj \
	ChDispat.obj \
	ChDlg.obj \
	ChExcept.obj \
	ChFileDlg.obj \
	ChFontElements.obj \
	ChFormTags.obj \
	ChHeadElement.obj \
	ChHook.obj \
	ChHtmFrm.obj \
	ChHtmlArgs.obj \
	ChHtmlFrameMgr.obj \
	ChHtmlPane.obj \
	ChHtmlParser.obj \
	ChHtmlSettings.obj \
	ChHTMLStream.obj \
	ChHtmlTag.obj \
	ChHtmlView.obj \
	ChHtmMsg.obj \
	ChHtmSym.obj \
	ChHtmWnd.obj \
	ChHTNtfy.obj \
	ChHTPriv.obj \
	ChHTTP.obj \
	ChHttpCookie.obj \
	ChHttpStream.obj \
	ChHttpThreadMgr.obj \
	ChHtUtil.obj \
	ChInlineTag.obj \
	ChKeyMap.obj \
	ChList.obj \
	ChMData.obj \
	ChMenu.obj \
	ChModule.obj \
	ChMsg.obj \
	ChMsgTyp.obj \
	ChNdbm.obj \
	ChPage.obj \
	ChPane.obj \
	ChPaneTag.obj \
	ChPblDoc.obj \
	ChPerFrm.obj \
	ChPlainTag.obj \
	ChPlgInInst.obj \
	ChPlgInMgr.obj \
	ChPlugInAPI.obj \
	ChProp.obj \
	ChPuebloScript.obj \
	ChReg.obj \
	ChRMenu.obj \
	ChScrWnd.obj \
	ChSock.obj \
	ChSocksProxy.obj \
	ChSplay.obj \
	ChSplit.obj \
	ChStrmbl.obj \
	ChThread.obj \
	ChTime.obj \
	ChTxRndr.obj \
	ChTxtAnimation.obj \
	ChTxtEng.obj \
	ChTxtObj.obj \
	ChTxtWnd.obj \
	ChUrlMap.obj \
	ChUtil.obj \
	ChVers.obj \
	ChWizard.obj \
	ChWType.obj \
	headers.obj \
	Pueblo.obj

LIBS=$(LIB_APIUTIL)

$(DLL_API) : $(DEF_FILE) $(RES_FILE) $(LINK32_OBJS) $(LIBS)
    $(LINK32) @&&|
  $(LINK32_FLAGS) c0d32.obj $(LINK32_OBJS)
  $@
  $*.map
  cw32mti import32 $(BFC) $(LIBS)
  $(DEF_FILE)
  $(RES_FILE)
|

DEP_RSC=\
	res\book.ico			\
	res\book_log.ico	\
	res\colsplit.cur	\
	res\hotspot.cur		\
	res\map.ico				\
	res\rowsplit.cur	\
	res\scroll.ico
	
$(RES_FILE) : pueblo.rc resource.h $(DEP_RSC)
   $(RSC) $(RSC_PROJ) pueblo.rc

release\ChSplay.asm : ChSplay.cpp
  $(CPP) $(CPP_COMMON) -S -v -O2 -Oi -OS -r -nRelease /DNDEBUG ChSplay.cpp
