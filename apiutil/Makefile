# UE Makefile for Pueblo API Utilities DLL
#
# Define additional compilation parameters
CSM=$(INTDIR)\PbUtil32.csm
DEF_FILE=PbUtil32.def
RES_FILE=$(INTDIR)\PbUtil32.res
NCDEFS=-WD -D_AFXEXT;XMD_H;ZLIB_DLL;ZLIB_INTERNAL 
NRDEFS=/d_AFXEXT
NLDEFS=/Gi /b:0x18000000 /Tpd

# Include common makefile information
!include "..\Common.mak"

# Define library filename
DLL_APIUTIL=$(LIB_APIUTIL:.lib=.dll)

# Define entry-point links
all : BuildDLL

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildDLL : OutDir IntDir KillCSM $(DLL_APIUTIL)

LINK32_OBJS= \
	ChDibBmp.obj		\
	ChDibDecoder.obj	\
	ChDibImage.obj	\
	ChDibPal.obj		\
	ChDibTransparent.obj	\
	ChDibUtl.obj		\
	ChGIF.obj				\
	ChJPEG.obj			\
	ChMngDecoder.obj \
	ChMngImage.obj	\
	ChSFImage.obj		\
	ChUnzip.obj			\
	explode.obj			\
	extract.obj			\
	file_io.obj			\
	headers.obj			\
	inflate.obj			\
	match.obj				\
	PbUtil32.obj		\
	unreduce.obj		\
	unshrink.obj

LIBS=$(LIB_MNG)

$(DLL_APIUTIL) : $(DEF_FILE) $(RES_FILE) $(LINK32_OBJS) $(LIBS)
    $(LINK32) @&&|
  $(LINK32_FLAGS) c0d32.obj $(LINK32_OBJS)
  $@
  $*.map
  cw32mti import32 $(BFC) $(LIBS)
  $(DEF_FILE)
  $(RES_FILE)
|

DEP_RSC=\
  res\gray.pal
	
$(RES_FILE) : PbUtil32.rc resource.h $(DEP_RSC)
   $(RSC) $(RSC_PROJ) PbUtil32.rc
