/*---------------------------------------------------------------------------

  unzip.h

  This header file is used by all of the UnZip source files.  Its contents
  are divided into seven more-or-less separate sections:  predefined macros,
  OS-dependent includes, (mostly) OS-independent defines, typedefs, function 
  prototypes (or "forward declarations," in the case of non-ANSI compilers),
  macros, and global-variable declarations.

  ---------------------------------------------------------------------------*/

#ifndef __unzip_h   /* prevent multiple inclusions */
#define __unzip_h

/*****************************************/
/*  Predefined, Machine-specific Macros  */
/*****************************************/




/*---------------------------------------------------------------------------
    MS-DOS and OS/2 section:
  ---------------------------------------------------------------------------*/

#if defined(MSDOS) || defined(OS2)
#  include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  include <time.h>           /* localtime() */
#  include <fcntl.h>          /* O_BINARY for open() w/o CR/LF translation */
#  ifdef __GO32__
#    define DIR_END '/'
#  else
#    define DIR_END '\\'
#  endif
#  if (defined(M_I86CM) || defined(M_I86LM))
#    define MED_MEM
#  endif
#  if (defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__))
#    define MED_MEM
#  endif
#  ifndef __32BIT__
#    ifndef MED_MEM
#      define SMALL_MEM
#    endif
/* #    define USE_FWRITE   write() *can* write up to 65534 bytes after all */
#  endif
#  define DATE_FORMAT   dateformat()
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#endif

#ifdef OS2                    /* defined for all OS/2 compilers */
#  ifdef MSDOS
#    undef MSDOS
#  endif
#  ifdef isupper
#    undef isupper
#  endif
#  ifdef tolower
#    undef tolower
#  endif
#  define isupper(x)   IsUpperNLS((unsigned char)(x))
#  define tolower(x)   ToLowerNLS((unsigned char)(x))
#endif

#ifdef MSDOS
#  define EXE_EXTENSION ".exe"  /* OS/2 has GetLoadPath() function instead */
#endif

/*---------------------------------------------------------------------------
    NT section:
  ---------------------------------------------------------------------------*/

#ifdef WIN32  /* NT */
#  include <sys/types.h>        /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <io.h>               /* read(), open(), etc. */
#  include <time.h>
#  include <memory.h>
#  include <direct.h>           /* mkdir() */
#  include <fcntl.h>
#  if defined(FILE_IO_C)
#    include <conio.h>
#    include <sys\types.h>
//#    include <sys\utime.h>
#    include <windows.h>
#  endif
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#  define NT
#  if (defined(_MSC_VER) && !defined(MSC))
#    define MSC
#  endif
#endif


/*---------------------------------------------------------------------------
    Unix section:
  ---------------------------------------------------------------------------*/

#ifdef UNIX
#  include <sys/types.h>       /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>

#  ifndef COHERENT
#    include <fcntl.h>         /* O_BINARY for open() w/o CR/LF translation */
#  else /* COHERENT */
#    ifdef _I386
#      include <fcntl.h>       /* Coherent 4.0.x, Mark Williams C */
#    else
#      include <sys/fcntl.h>   /* Coherent 3.10, Mark Williams C */
#    endif
#    define SHORT_SYMS
#    ifndef __COHERENT__       /* Coherent 4.2 has tzset() */
#      define tzset  settz
#    endif
#  endif /* ?COHERENT */

#  ifndef NO_PARAM_H
#    ifdef NGROUPS_MAX
#      undef NGROUPS_MAX       /* SCO bug:  defined again in <sys/param.h> */
#    endif
#    ifdef BSD
#      define TEMP_BSD         /* may be defined again in <sys/param.h> */
#      undef BSD
#    endif
#    include <sys/param.h>     /* conflict with <sys/types.h>, some systems? */
#    ifdef TEMP_BSD
#      undef TEMP_BSD
#      ifndef BSD
#        define BSD
#      endif
#    endif
#  endif /* !NO_PARAM_H */

#  ifdef __osf__
#    define DIRENT
#    ifdef BSD
#      undef BSD
#    endif
#  endif /* __osf__ */

#  ifdef BSD
#    include <sys/time.h>
#    include <sys/timeb.h>
#    ifdef _AIX
#      include <time.h>
#    endif
#  else
#    include <time.h>
     struct tm *gmtime(), *localtime();
#  endif

#  if defined(BSD4_4) || defined(LINUX) || (defined(SYSV) && defined(MODERN))
#    include <unistd.h>        /* this includes utime.h, at least on SGIs */
#  endif

#  if defined(BSD4_4) || defined(_POSIX_SOURCE) || defined(sgi) || defined(_AIX)
#    include <utime.h>   /* NeXT, at least, does NOT define utimbuf in here */
#  else
     struct utimbuf {
         time_t actime;        /* new access time */
         time_t modtime;       /* new modification time */
     };
#  endif /* ?(BSD4_4 || _POSIX_SOURCE || sgi || _AIX) */

#  if (defined(V7) || defined(pyr_bsd))
#    define strchr   index
#    define strrchr  rindex
#  endif
#  ifdef V7
#    define O_RDONLY 0
#    define O_WRONLY 1
#    define O_RDWR   2
#  endif

#  ifdef MINIX
#    include <stdio.h>
#  endif
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif /* UNIX */


 /* GZIP section */
#define GZ_MAGIC_1 0x1f
#define GZ_MAGIC_2 0x8b


/* gzip flag byte */

#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */



/*************/
/*  Defines  */
/*************/

#define UNZIP
#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_UNZIP_VERSION 42   /* if OS-needed-to-extract is VMS:  can do */

#if defined(MSDOS) || defined(NT) || defined(OS2)
#  define DOS_NT_OS2
#endif

#if defined(MSDOS) || defined(OS2)
#  define DOS_OS2
#endif


/* clean up with a few defaults */
#ifndef DIR_END
#  define DIR_END '/'       /* last char before program name (or filename) */
#endif
#ifndef RETURN
#  define RETURN  return    /* only used in main() */
#endif


#define DIR_BLKSIZ  64      /* number of directory entries per block
                             *  (should fit in 4096 bytes, usually) */
#ifndef WSIZE
#  define WSIZE     0x8000  /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */

#ifndef INBUFSIZ
#    define INBUFSIZ  8192  /* larger buffers for real OSes */
#endif

#define OUTBUFSIZ         INBUFSIZ

#define RAWBUFSIZ (OUTBUFSIZ>>1)
#define TRANSBUFSIZ (OUTBUFSIZ-RAWBUFSIZ)






#define FILNAMSIZ  (MAX_PATH+1)


#define ZSUFX             ".zip"
#define GZSUFX            ".gz"
#define CENTRAL_HDR_SIG   "\113\001\002"   /* the infamous "PK" signature */
#define LOCAL_HDR_SIG     "\113\003\004"   /*  bytes, sans "P" (so unzip */
#define END_CENTRAL_SIG   "\113\005\006"   /*  executable not mistaken for */
#define EXTD_LOCAL_SIG    "\113\007\010"   /*  zipfile itself) */

#define SKIP              0    /* choice of activities for do_string() */
#define DISPLAY           1
#define ZFILENAME         2
#define EXTRA_FIELD       3

#define DOES_NOT_EXIST    -1   /* return values for check_for_newer() */
#define EXISTS_AND_OLDER  0
#define EXISTS_AND_NEWER  1

#define ROOT              0    /* checkdir() extract-to path:  called once */
#define INIT              1    /* allocate buildpath:  called once per member */
#define APPEND_DIR        2    /* append a dir comp.:  many times per member */
#define APPEND_NAME       3    /* append actual filename:  once per member */
#define GETPATH           4    /* retrieve the complete path and free it */
#define END               5    /* free root path prior to exiting program */

/* version_made_by codes (central dir):  make sure these */
/*  are not defined on their respective systems!! */
#define FS_FAT_           0    /* filesystem used by MS-DOS, OS/2, NT */
#define AMIGA_            1
#define VMS_              2
#define UNIX_             3
#define VM_CMS_           4
#define ATARI_            5    /* what if it's a minix filesystem? [cjh] */
#define FS_HPFS_          6    /* filesystem used by OS/2, NT */
#define MAC_              7
#define Z_SYSTEM_         8
#define CPM_              9
#define TOPS20_           10
#define FS_NTFS_          11   /* filesystem used by Windows NT */
#define QDOS_MAYBE_       12   /* a bit premature, but somebody once started */
#define ACORN_            13   /* Archimedes Acorn RISCOS */
#define NUM_HOSTS         14   /* index of last system + 1 */

#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define NUM_METHODS       9    /* index of last method + 1 */
/* don't forget to update list_files() appropriately if NUM_METHODS changes */

#define PK_OK             0    /* no error */
#define PK_COOL           0    /* no error */
#define PK_GNARLY         0    /* no error */
#define PK_WARN           1    /* warning error */
#define PK_ERR            2    /* error in zipfile */
#define PK_BADERR         3    /* severe error in zipfile */
#define PK_MEM            4    /* insufficient memory */
#define PK_MEM2           5    /* insufficient memory */
#define PK_MEM3           6    /* insufficient memory */
#define PK_MEM4           7    /* insufficient memory */
#define PK_MEM5           8    /* insufficient memory */
#define PK_NOZIP          9    /* zipfile not found */
#define PK_PARAM          10   /* bad or illegal parameters specified */
#define PK_FIND           11   /* no files found */
#define PK_DISK           50   /* disk full */
#define PK_EOF            51   /* unexpected EOF */

#define IZ_DIR            76   /* potential zipfile is a directory */
#define IZ_CREATED_DIR    77   /* directory created: set time and permissions */
#define IZ_VOL_LABEL      78   /* volume label, but can't set on hard disk */

#define DF_MDY            0    /* date format 10/26/91 (USA only) */
#define DF_DMY            1    /* date format 26/10/91 (most of the world) */
#define DF_YMD            2    /* date format 91/10/26 (a few countries) */

/*---------------------------------------------------------------------------
    True sizes of the various headers, as defined by PKWARE--so it is not
    likely that these will ever change.  But if they do, make sure both these
    defines AND the typedefs below get updated accordingly.
  ---------------------------------------------------------------------------*/
#define LREC_SIZE     26    /* lengths of local file headers, central */
#define CREC_SIZE     42    /*  directory headers, and the end-of-    */
#define ECREC_SIZE    18    /*  central-dir record, respectively      */

#define MAX_BITS      13                 /* used in old unshrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */

#define LF      10    /* '\n' on ASCII machines; must be 10 due to EBCDIC */
#define CR      13    /* '\r' on ASCII machines; must be 13 due to EBCDIC */
#define CTRLZ   26    /* DOS & OS/2 EOF marker (used in file_io.c, vms.c) */

#ifdef EBCDIC
#  define native(c)   ebcdic[(c)]
#  define NATIVE      "EBCDIC"
#endif

#ifdef MPW
#  define FFLUSH(f)   PUTC('\n',f)
#else
#  define FFLUSH      fflush
#endif

#ifdef ZMEM     /* GRR:  THIS IS AN EXPERIMENT... (seems to work) */
#  undef ZMEM
#  define memcpy(dest,src,len)   bcopy(src,dest,len)
#  define memzero                bzero
#else
#  define memzero(dest,len)      memset(dest,0,len)
#endif

#ifdef VMS
#  define ENV_UNZIP     "UNZIP_OPTS"      /* name of environment variable */
#  define ENV_ZIPINFO   "ZIPINFO_OPTS"
#else /* !VMS */
#  define ENV_UNZIP     "UNZIP"
#  define ENV_ZIPINFO   "ZIPINFO"
#endif /* ?VMS */
#define ENV_UNZIP2      "UNZIPOPT"        /* alternate name for zip compat. */
#define ENV_ZIPINFO2    "ZIPINFOOPT"

#if !defined(QQ) && !defined(NOQQ)
#  define QQ
#endif

#ifdef QQ                         /* Newtware version:  no file */
#  define QCOND     (!qflag)      /*  comments with -vq or -vqq */
#else                             /* Bill Davidsen version:  no way to */
#  define QCOND     (which_hdr)   /*  kill file comments when listing */
#endif

#ifdef OLD_QQ
#  define QCOND2    (qflag < 2)
#else
#  define QCOND2    (!qflag)
#endif

#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#endif
#ifndef FALSE
#  define FALSE     0
#endif

#ifndef SEEK_SET
#  define SEEK_SET  0
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif

#if (defined(UNIX) && defined(S_IFLNK) && !defined(MTS))
#  define SYMLINKS
#  ifndef S_ISLNK
#    define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#  endif
#endif /* UNIX && S_IFLNK && !MTS */

#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif


#ifndef IS_VOLID
#  define IS_VOLID(m)  ((m) & 0x08)
#endif





/**************/
/*  Typedefs  */
/**************/

typedef unsigned char     uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short    ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long     ulg;  /*  predefined on some systems) & match zip  */	
typedef long              LONGINT;
typedef unsigned int 	  extent;


typedef struct min_info {
    long offset;
    ulg compr_size;          /* compressed size (needed if extended header) */
    ulg crc;                 /* crc (needed if extended header) */
    int hostnum;
    unsigned file_attr;      /* local flavor, as used by creat(), chmod()... */
    unsigned encrypted : 1;  /* file encrypted: decrypt before uncompressing */
    unsigned ExtLocHdr : 1;  /* use time instead of CRC for decrypt check */
    unsigned textfile : 1;   /* file is text (according to zip) */
    unsigned textmode : 1;   /* file is to be extracted as text */
    unsigned lcflag : 1;     /* convert filename to lowercase */
    unsigned vollabel : 1;   /* "file" is an MS-DOS volume (disk) label */
} min_info;

typedef struct VMStimbuf {
    char *revdate;           /* (both correspond to Unix modtime/st_mtime) */
    char *credate;
} VMStimbuf;

/*---------------------------------------------------------------------------
    Zipfile work area declarations.
  ---------------------------------------------------------------------------*/
#if 0

#define  MALLOC_WORK

#ifdef MALLOC_WORK

   union work {
     struct {
       short *Prefix_of;            /* (8193 * sizeof(short)) */
       uch *Suffix_of;
       uch *Stack;
     } shrink;                      /* unshrink() */
     uch *Slide;                    /* explode(), inflate(), unreduce() */
   };
#  define prefix_of  area.shrink.Prefix_of
#  define suffix_of  area.shrink.Suffix_of
#  define stack      area.shrink.Stack

#else /* !MALLOC_WORK */

#  ifdef NEW_UNSHRINK   /* weird segmentation violations if union NODE array */
     union work {
       uch Stack[8192];             /* unshrink() */
       uch Slide[WSIZE];            /* explode(), inflate(), unreduce() */
     };
#    define stack  area.Stack
#  else
     union work {
       struct {
         short Prefix_of[HSIZE];    /* (8192 * sizeof(short)) */
         uch Suffix_of[HSIZE];
         uch Stack[HSIZE];
       } shrink;
       uch Slide[WSIZE];            /* explode(), inflate(), unreduce() */
     };
#    define prefix_of  area.shrink.Prefix_of
#    define suffix_of  area.shrink.Suffix_of
#    define stack      area.shrink.Stack
#  endif /* ?NEW_UNSHRINK */

#endif /* ?MALLOC_WORK */

#define slide  area.Slide
#endif  // 0

/*---------------------------------------------------------------------------
    Zipfile layout declarations.  If these headers ever change, make sure the
    xxREC_SIZE defines (above) change with them!
  ---------------------------------------------------------------------------*/

   typedef uch   local_byte_hdr[ LREC_SIZE ];
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

   typedef uch   cdir_byte_hdr[ CREC_SIZE ];
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

   typedef uch   ec_byte_rec[ ECREC_SIZE+4 ];
/*     define SIGNATURE                         0   space-holder only */
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10
#      define SIZE_CENTRAL_DIRECTORY            12
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20


   typedef struct local_file_header {                 /* LOCAL */
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
   } local_file_hdr;

   typedef struct central_directory_file_header {     /* CENTRAL */
       uch version_made_by[2];
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
       ush file_comment_length;
       ush disk_number_start;
       ush internal_file_attributes;
       ulg external_file_attributes;
       ulg relative_offset_local_header;
   } cdir_file_hdr;

   typedef struct end_central_dir_record {            /* END CENTRAL */
       ush number_this_disk;
       ush num_disk_with_start_central_dir;
       ush num_entries_centrl_dir_ths_disk;
       ush total_entries_central_dir;
       ulg size_central_directory;
       ulg offset_start_central_directory;
       ush zipfile_comment_length;
   } ecdir_rec;





#if 0

/*************************/
/*  Function Prototypes  */
/*************************/

#ifndef __
#  define __   OF
#endif

/*---------------------------------------------------------------------------
    Functions in unzip.c (main initialization/driver routines):
  ---------------------------------------------------------------------------*/

int    uz_opts                   __((int *pargc, char ***pargv));
int    usage                     __((int error));
int    process_zipfiles          __((void));
int    do_seekable               __((int lastchance));
int    uz_end_central            __((void));
int    process_cdir_file_hdr     __((void));
int    process_local_file_hdr    __((void));

/*---------------------------------------------------------------------------
    Functions in zipinfo.c (zipfile-listing routines):
  ---------------------------------------------------------------------------*/

int    zi_opts                   __((int *pargc, char ***pargv));
int    zi_end_central            __((void));
int    zipinfo                   __((void));
/* static int    zi_long         __((void)); */
/* static int    zi_short        __((void)); */
/* static char  *zi_time         __((ush *datez, ush *timez)); */
ulg    SizeOfEAs                 __((void *extra_field));  /* also in os2.c? */
int    list_files                __((void));
/* static int    ratio           __((ulg uc, ulg c)); */

/*---------------------------------------------------------------------------
    Functions in file_io.c:
  ---------------------------------------------------------------------------*/

int      open_input_file    __((void));
int      open_outfile       __((void));                        /* also vms.c */
unsigned readbuf            __((char *buf, register unsigned len));
int      FillBitBuffer      __((void));
int      readbyte           __((void));
#ifdef FUNZIP
   int   flush              __((ulg size));
#else
   int   flush              __((uch *buf, ulg size, int unshrink));
#endif
void     handler            __((int signal));
time_t   dos_to_unix_time   __((unsigned ddate, unsigned dtime));
int      check_for_newer    __((char *filename));       /* also os2.c, vms.c */
int      find_ecrec         __((long searchlen));
int      get_cdir_ent       __((void));
int      do_string          __((unsigned int len, int option));
ush      makeword           __((uch *b));
ulg      makelong           __((uch *sig));
int      zstrnicmp __((register char *s1, register char *s2, register int n));

#ifdef ZMEM   /* MUST be ifdef'd because of conflicts with the standard def. */
   char *memset __((register char *, register char, register unsigned int));
   char *memcpy __((register char *, register char *, register unsigned int));
#endif

#ifdef SMALL_MEM
   char *LoadFarString         __((char Far *sz));
   char *LoadFarStringSmall    __((char Far *sz));
   char *LoadFarStringSmall2   __((char Far *sz));
   char Far * Far zfstrcpy     __((char Far *s1, const char Far *s2));
#endif


/*---------------------------------------------------------------------------
    Functions in extract.c:
  ---------------------------------------------------------------------------*/

int    extract_or_test_files     __((void));
/* static int   store_info               __((void)); */
/* static int   extract_or_test_member   __((void)); */
int    memextract                __((uch *, ulg, uch *, ulg));
int    FlushMemory               __((void));
int    ReadMemoryByte            __((ush *x));

/*---------------------------------------------------------------------------
    Decompression functions:
  ---------------------------------------------------------------------------*/

int    explode                   __((void));                    /* explode.c */
int    inflate                   __((void));                    /* inflate.c */
int    inflate_free              __((void));                    /* inflate.c */
void   unreduce                  __((void));                   /* unreduce.c */
/* static void  LoadFollowers    __((void));                    * unreduce.c */
int    unshrink                  __((void));                   /* unshrink.c */
/* static void  partial_clear    __((void));                    * unshrink.c */

/*---------------------------------------------------------------------------
    Human68K-only functions:
  ---------------------------------------------------------------------------*/

#ifdef __human68k__
   void     InitTwentyOne        __((void));
#endif

/*---------------------------------------------------------------------------
    Macintosh-only functions:
  ---------------------------------------------------------------------------*/

#ifdef MACOS
   int      macmkdir             __((char *, short, long));
   short    macopen              __((char *, short, short, long));
   FILE    *macfopen             __((char *, char *, short, long));
   short    maccreat             __((char *, short, long, OSType, OSType));
   short    macread              __((short, char *, unsigned));
   long     macwrite             __((short, char *, unsigned));
   short    macclose             __((short));
   long     maclseek             __((short, long, short));
   char    *macgetenv            __((char *));
   char    *wfgets               __((char *, int, FILE *));
   void     wfprintf             __((FILE *, char *, ...));
   void     wprintf              __((char *, ...));
#endif

/*---------------------------------------------------------------------------
    MSDOS-only functions:
  ---------------------------------------------------------------------------*/

#if (defined(__GO32__) || (defined(MSDOS) && defined(__EMX__)))
   unsigned _dos_getcountryinfo(void *);                          /* msdos.c */
   void _dos_setftime(int, unsigned short, unsigned short);       /* msdos.c */
   void _dos_setfileattr(char *, int);                            /* msdos.c */
   unsigned _dos_creat(char *, unsigned, int *);                  /* msdos.c */
   void _dos_getdrive(unsigned *);                                /* msdos.c */
   unsigned _dos_close(int);                                      /* msdos.c */
#endif

/*---------------------------------------------------------------------------
    OS/2-only functions:
  ---------------------------------------------------------------------------*/

#ifdef OS2  /* GetFileTime conflicts with something in NT header files */
   int   GetCountryInfo   __((void));                               /* os2.c */
   long  GetFileTime      __((char *name));                         /* os2.c */
   void  SetPathInfo      __((char *path, ush moddate, ush modtime, int flags));
   int   IsEA             __((void *extra_field));                  /* os2.c */
   void  SetEAs           __((char *path, void *eablock));          /* os2.c */
   ulg   SizeOfEAs        __((void *extra_field));                  /* os2.c */
/* static int   IsFileNameValid __((char *name));                      os2.c */
/* static void  map2fat         __((char *pathcomp, char **pEndFAT));  os2.c */
/* static int   SetLongNameEA   __((char *name, char *longname));      os2.c */
/* static void  InitNLS         __((void));                            os2.c */
   int   IsUpperNLS       __((int nChr));                           /* os2.c */
   int   ToLowerNLS       __((int nChr));                           /* os2.c */
   void  DebugMalloc      __((void));                               /* os2.c */
#endif

/*---------------------------------------------------------------------------
    TOPS20-only functions:
  ---------------------------------------------------------------------------*/

#ifdef TOPS20
   int upper               __((char *s));                        /* tops20.c */
   int enquote             __((char *s));                        /* tops20.c */
   int dequote             __((char *s));                        /* tops20.c */
   int fnlegal             __(());  /* error if prototyped(?) */ /* tops20.c */
#endif

/*---------------------------------------------------------------------------
    VMS-only functions:
  ---------------------------------------------------------------------------*/

#ifdef VMS
   int    check_format        __((void));                           /* vms.c */
   int    find_vms_attrs      __((void));                           /* vms.c */
   int    CloseOutputFile     __((void));                           /* vms.c */
/* static uch *extract_block  __((struct extra_block *, int *, uch *, int)); */
/* static int  _flush_blocks  __((int final_flag));                  * vms.c */
/* static int  _flush_records __((int final_flag));                  * vms.c */
/* static int  WriteBuffer    __((unsigned char *buf, int len));     * vms.c */
/* static int  WriteRecord    __((unsigned char *rec, int len));     * vms.c */
/* static void message        __((int string, char *status));        * vms.c */
   void   return_VMS          __((int zip_error));                  /* vms.c */
#ifdef VMSCLI
   ulg    vms_unzip_cmdline   __((int *, char ***));            /* cmdline.c */
#endif
#endif

/*---------------------------------------------------------------------------
    Miscellaneous/shared functions:
  ---------------------------------------------------------------------------*/

int      match           __((char *s, char *p, int ic));          /* match.c */
int      iswild          __((char *p));                           /* match.c */

void     envargs         __((int *, char ***, char *, char *)); /* envargs.c */
void     mksargs         __((int *, char ***));                 /* envargs.c */

int      dateformat      __((void));
void     version         __((void));                                /* local */
int      mapattr         __((void));                                /* local */
int      mapname         __((int renamed));                         /* local */
int      checkdir        __((char *pathcomp, int flag));            /* local */
char    *do_wild         __((char *wildzipfn));                     /* local */
char    *GetLoadPath     __((void));                                /* local */
#ifndef MTS /* macro in MTS */
   void  close_outfile   __((void));                                /* local */
#endif

#endif // 0





/************/
/*  Macros  */
/************/

#ifndef MAX
#  define MAX(a,b)   ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif

#if 0
#ifdef DEBUG
#  define Trace(x)   FPRINTF x
#else
#  define Trace(x)
#endif
#endif // 0

#if defined(UNIX) || defined(T20_VMS)   /* generally old systems */
#  define ToLower(x)   ((char)(isupper((int)x)? tolower((int)x) : x))
#else
#  define ToLower      tolower          /* assumed "smart"; used in match() */
#endif

#if 0
#define LSEEK(abs_offset) {LONGINT request=(abs_offset)+extra_bytes,\
   inbuf_offset=request%INBUFSIZ, bufstart=request-inbuf_offset;\
   if(request<0) { /*FPRINTF(stderr, LoadFarStringSmall(SeekMsg), LoadFarString(ReportMsg)); */ return(3);}\
   else if(bufstart!=cur_zipfile_bufstart)\
   {cur_zipfile_bufstart=lseek(zipfd,(LONGINT)bufstart,SEEK_SET);\
   if((incnt=read(zipfd,(char *)inbuf,INBUFSIZ))<=0) return(51);\
   inptr=inbuf+(int)inbuf_offset; incnt-=(int)inbuf_offset;} else\
   {incnt+=(inptr-inbuf)-(int)inbuf_offset; inptr=inbuf+(int)inbuf_offset;}}
#else
#define LSEEK(abs_offset) {LONGINT request=(abs_offset)+extra_bytes,\
   inbuf_offset=request%INBUFSIZ, bufstart=request-inbuf_offset;\
   if(request<0) { /*FPRINTF(stderr, LoadFarStringSmall(SeekMsg), LoadFarString(ReportMsg)); */ return(3);}\
   else if(bufstart!=cur_zipfile_bufstart)\
   {cur_zipfile_bufstart=(m_pFile->seekg((LONGINT)bufstart,std::ios::beg).tellg());\
   if((incnt= m_pFile->read((char *)inbuf,INBUFSIZ).gcount())<=0) return(51);\
   inptr=inbuf+(int)inbuf_offset; incnt-=(int)inbuf_offset;} else\
   {incnt+=(inptr-inbuf)-(int)inbuf_offset; inptr=inbuf+(int)inbuf_offset;}}
#endif

/*
 *  Seek to the block boundary of the block which includes abs_offset,
 *  then read block into input buffer and set pointers appropriately.
 *  If block is already in the buffer, just set the pointers.  This macro
 *  is used by process_end_central_dir (unzip.c) and do_string (file_io.c).
 *  A slightly modified version is embedded within extract_or_test_files
 *  (unzip.c).  ReadByte and readbuf (file_io.c) are compatible.
 *
 *  macro LSEEK(abs_offset)
 *      ulg abs_offset;
 *  {
 *      LONGINT   request = abs_offset + extra_bytes;
 *      LONGINT   inbuf_offset = request % INBUFSIZ;
 *      LONGINT   bufstart = request - inbuf_offset;
 *
 *      if (request < 0) {
 *          FPRINTF(stderr, LoadFarStringSmall(SeekMsg),
 *            LoadFarString(ReportMsg));
 *          return(3);             /-* 3:  severe error in zipfile *-/
 *      } else if (bufstart != cur_zipfile_bufstart) {
 *          cur_zipfile_bufstart = lseek(zipfd, (LONGINT)bufstart, SEEK_SET);
 *          if ((incnt = read(zipfd,inbuf,INBUFSIZ)) <= 0)
 *              return(51);        /-* 51:  unexpected EOF *-/
 *          inptr = inbuf + (int)inbuf_offset;
 *          incnt -= (int)inbuf_offset;
 *      } else {
 *          incnt += (inptr-inbuf) - (int)inbuf_offset;
 *          inptr = inbuf + (int)inbuf_offset;
 *      }
 *  }
 *
 */


#define SKIP_(length) if(length&&((error=do_string(length,SKIP))!=0))\
  {error_in_archive=error; if(error>1) return error;}

/*
 *  Skip a variable-length field, and report any errors.  Used in zipinfo.c
 *  and unzip.c in several functions.
 *
 *  macro SKIP_(length)
 *      ush length;
 *  {
 *      if (length && ((error = do_string(length, SKIP)) != 0)) {
 *          error_in_archive = error;   /-* might be warning *-/
 *          if (error > 1)              /-* fatal *-/
 *              return (error);
 *      }
 *  }
 *
 */


#ifdef FUNZIP
#  define FLUSH    flush
#  define NEXTBYTE getc(in)   /* redefined in crypt.h if full version */
#else
#  define FLUSH(w) if (mem_mode) outcnt=(w); else flush(slide,(ulg)w,0)

#  define NEXTBYTE \
     (csize-- <= 0L ? EOF : (--incnt >= 0 ? (int)(*inptr++) : readbyte()))
#endif


#define READBITS(nbits,zdest) {if(nbits>bits_left) {int temp; zipeof=1;\
  while (bits_left<=8*(sizeof(bitbuf)-1) && (temp=NEXTBYTE)!=EOF) {\
  bitbuf|=(ulg)temp<<bits_left; bits_left+=8; zipeof=0;}}\
  zdest=(int)((ush)bitbuf&mask_bits[nbits]);bitbuf>>=nbits;bits_left-=nbits;}

/*
 * macro READBITS(nbits,zdest)    * only used by unreduce and unshrink *
 *  {
 *      if (nbits > bits_left) {  * fill bitbuf, which is 8*sizeof(ulg) bits *
 *          int temp;
 *
 *          zipeof = 1;
 *          while (bits_left <= 8*(sizeof(bitbuf)-1) &&
 *                 (temp = NEXTBYTE) != EOF) {
 *              bitbuf |= (ulg)temp << bits_left;
 *              bits_left += 8;
 *              zipeof = 0;
 *          }
 *      }
 *      zdest = (int)((ush)bitbuf & mask_bits[nbits]);
 *      bitbuf >>= nbits;
 *      bits_left -= nbits;
 *  }
 *
 */


/* GRR:  should change name to STRLOWER and use StringLower if possible */

/*
 *  Copy the zero-terminated string in str1 into str2, converting any
 *  uppercase letters to lowercase as we go.  str2 gets zero-terminated
 *  as well, of course.  str1 and str2 may be the same character array.
 */
#ifdef __human68k__
#  define TOLOWER(str1, str2) \
   { \
       char *p=(str1), *q=(str2); \
       uch c; \
       while ((c = *p++) != '\0') { \
           if (iskanji(c)) { \
               if (*p == '\0') \
                   break; \
               *q++ = c; \
               *q++ = *p++; \
           } else \
               *q++ = isupper(c) ? tolower(c) : c; \
       } \
       *q = '\0'; \
   }
#else
#  define TOLOWER(str1, str2) \
   { \
       char  *p, *q; \
       p = (str1) - 1; \
       q = (str2); \
       while (*++p) \
           *q++ = (char)(isupper((int)(*p))? tolower((int)(*p)) : *p); \
       *q = '\0'; \
   }
#endif
/*
 *  NOTES:  This macro makes no assumptions about the characteristics of
 *    the tolower() function or macro (beyond its existence), nor does it
 *    make assumptions about the structure of the character set (i.e., it
 *    should work on EBCDIC machines, too).  The fact that either or both
 *    of isupper() and tolower() may be macros has been taken into account;
 *    watch out for "side effects" (in the C sense) when modifying this
 *    macro.
 */

#if 0

#ifndef native
#  define native(c)   (c)
#  define A_TO_N(str1)
#else
#  ifndef NATIVE
#    define NATIVE     "native chars"
#  endif
#  define A_TO_N(str1) {register unsigned char *p;\
     for (p=str1; *p; p++) *p=native(*p);}
#endif

#endif

#  define native(c)   (c)
#  define ascii_to_native(c)   (c)
#  define A_TO_N(str1)


/*
 *  Translate the zero-terminated string in str1 from ASCII to the native
 *  character set. The translation is performed in-place and uses the
 *  "native" macro to translate each character.
 *
 *  macro A_TO_N( str1 )
 *  {
 *      register unsigned char *p;
 *
 *      for (p = str1;  *p;  ++p)
 *          *p = native(*p);
 *  }
 *
 *  NOTE:  Using the "native" macro means that is it the only part of unzip
 *    which knows which translation table (if any) is actually in use to
 *    produce the native character set.  This makes adding new character set
 *    translation tables easy, insofar as all that is needed is an appropriate
 *    "native" macro definition and the translation table itself.  Currently,
 *    the only non-ASCII native character set implemented is EBCDIC, but this
 *    may not always be so.
 */



#endif /* !__unzip_h */
