#
# For conditions of distribution and use, see copyright notice in libmng.h
#
# makefile for libmng - THE MNG library
#
# modified for Pueblo/UE build environment by Gavin Lambert
.AutoDepend

# Define additional compilation parameters
CSM=$(INTDIR)\libmng.csm
DEF_FILE=
RES_FILE=
NCDEFS=-tWD -DZLIB_DLL;ZLIB_INTERNAL;XMD_H;USE_WINDOWS_MESSAGEBOX -w!- -E$(ASM)
NRDEFS=
NLDEFS=/Gi /b:0x40000000 /Tpd

# Include common makefile information
!include "..\Common.mak"

# Define entry-point links
all : BuildLIB

# Include common support rules
!include "$(PROJROOT)\Common2.mak"

BuildLIB : IntDir KillCSM $(LIB_MNG)

# compiler options:
#CFLAGS= -WD -O2 -Hc -w-par -k -y -v -vi -c -tWD \
#	-wuse -wucp -wstv -wstu -wsig -wpin -wnod -wnak -wdef -wcln -wbbf -wasm -wamp \
#	-wamb -Tkh30000 -ff -5 -I.;..\zlib;..\jpgsrc6b;..\lcms\include $(MNGOPT)

.path.c=.;..\zlib;..\jpgsrc6b;..\lcms\src

# object files
LIB32_OBJ1= \
	pfx_libmng_hlapi.obj \
	pfx_libmng_callback_xs.obj \
	pfx_libmng_prop_xs.obj \
	pfx_libmng_chunk_xs.obj \
	pfx_libmng_read.obj \
	pfx_libmng_write.obj \
	pfx_libmng_display.obj \
	pfx_libmng_object_prc.obj \
	pfx_libmng_chunk_prc.obj \
	pfx_libmng_chunk_io.obj \
	pfx_libmng_error.obj \
  pfx_libmng_trace.obj \
  pfx_libmng_pixels.obj \
  pfx_libmng_filter.obj \
  pfx_libmng_dither.obj \
  pfx_libmng_zlib.obj \
  pfx_libmng_jpeg.obj \
  pfx_libmng_cms.obj  \
  pfx_adler32.obj pfx_compress.obj pfx_crc32.obj pfx_deflate.obj pfx_infback.obj pfx_gzio.obj \
  pfx_inffast.obj pfx_inflate.obj pfx_inftrees.obj pfx_trees.obj pfx_uncompr.obj \
  pfx_zutil.obj pfx_jquant2.obj pfx_jcapistd.obj pfx_jccoefct.obj pfx_jccolor.obj pfx_jcdctmgr.obj \
  pfx_jchuff.obj pfx_jcinit.obj pfx_jcmainct.obj pfx_jcmarker.obj pfx_jcmaster.obj pfx_jcomapi.obj \
  pfx_jcparam.obj pfx_jcphuff.obj pfx_jcprepct.obj pfx_jcsample.obj pfx_jctrans.obj pfx_jdapistd.obj \
  pfx_jdatadst.obj pfx_jdatasrc.obj pfx_jdcoefct.obj pfx_jdcolor.obj pfx_jddctmgr.obj pfx_jdhuff.obj \
  pfx_jdinput.obj pfx_jdmainct.obj pfx_jdmarker.obj pfx_jdmaster.obj pfx_jdmerge.obj pfx_jdphuff.obj \
  pfx_jdpostct.obj pfx_jdsample.obj pfx_jdtrans.obj pfx_jerror.obj pfx_jfdctflt.obj pfx_jfdctfst.obj \
  pfx_jfdctint.obj pfx_jidctflt.obj pfx_jidctfst.obj pfx_jidctint.obj pfx_jidctred.obj \
  pfx_jmemmgr.obj pfx_jmemnobs.obj pfx_jquant1.obj pfx_jcapimin.obj pfx_jutils.obj pfx_jdapimin.obj \
  pfx_cmscam97.obj pfx_cmscnvrt.obj pfx_cmserr.obj pfx_cmsgamma.obj pfx_cmsgmt.obj pfx_cmsintrp.obj \
  pfx_cmsio1.obj pfx_cmslut.obj pfx_cmsmatsh.obj pfx_cmsmtrx.obj pfx_cmspack.obj pfx_cmspcs.obj \
  pfx_cmssamp.obj pfx_cmswtpnt.obj pfx_cmsxform.obj pfx_cmsvirt.obj pfx_cmsnamed.obj

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

$(LIB_MNG) : $(LIB32_OBJD)
  @if exist $(LIB_MNG) del $(LIB_MNG)
  $(LIB32) @&&|
  $(LIB32_FLAGS) $(LIB_MNG) $(LIB32_OBJS)
|
