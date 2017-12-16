/*----------------------------------------------------------------------------
                        _                              _ _       
        /\             | |                            | (_)      
       /  \   _ __   __| |_ __ ___  _ __ ___   ___  __| |_  __ _ 
      / /\ \ | '_ \ / _` | '__/ _ \| '_ ` _ \ / _ \/ _` | |/ _` |
     / ____ \| | | | (_| | | | (_) | | | | | |  __/ (_| | | (_| |
    /_/    \_\_| |_|\__,_|_|  \___/|_| |_| |_|\___|\__,_|_|\__,_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo client code, released November 4, 1998.

    The Initial Developer of the Original Code is Andromedia Incorporated.
	Portions created by Andromedia are Copyright (C) 1998 Andromedia
	Incorporated.  All Rights Reserved.

	Andromedia Incorporated                         415.365.6700
	818 Mission Street - 2nd Floor                  415.365.6701 fax
	San Francisco, CA 94103

    Contributor(s):
	--------------------------------------------------------------------------
	   Chaco team:  Dan Greening, Glenn Crocker, Jim Doubek,
	                Coyote Lussier, Pritham Shetty.

					Wrote and designed original codebase.

------------------------------------------------------------------------------

	Interface for  ChUnzip class.

----------------------------------------------------------------------------*/

#include <unzip.h>
#include <iostream>
#include <fstream>
// it would be nice to use enum for this but on 16 bit 
// enum is a int which is 16 bits only
#define UNZIP_OPT_EBCIDIC_ASCII			0x00000001L
#define UNZIP_OPT_EXTRACT_FILE			0x00000002L
#define UNZIP_OPT_FRESHEN_ONLY			0x00000004L
#define UNZIP_OPT_RESTORE_PATHNAME		0x00000008L
#define UNZIP_OPT_OVERWRITE_NONE		0x00000010L
#define UNZIP_OPT_OVERWRITE_ALL			0x00000020L
#define UNZIP_OPT_FORCE					0x00000040L
#define UNZIP_OPT_TEST					0x00000080L
#define UNZIP_OPT_UPDATE				0x00000100L
#define UNZIP_OPT_PRESERVE_FILENAMES	0x00000200L
#define UNZIP_OPT_VIEW_DIRECTORY		0x00000400L
#define UNZIP_OPT_STRIP_VMS_VER			0x00000800L
#define UNZIP_OPT_DISPLAY_COMMENT		0x00001000L
#define UNZIP_OPT_PROCESS_ALL_FILES		0x00002000L	
#define UNZIP_OPT_ALLOW_SPACE			0x00004000L	
#define UNZIP_OPT_QUIET					0x00008000L	
#define UNZIP_OPT_DISPLAY_FILE			0x00010000L
#define UNZIP_OPT_DST_DIR				0x00020000L
#define UNZIP_OPT_DST_RENAME			0x00040000L

#define UNZIP_OPT_GZIP_FILE				0x00080000L  // input file is in gzip format

#define UNZIP_DEF_OPTION 	( UNZIP_OPT_PROCESS_ALL_FILES	|	 \
							  UNZIP_OPT_QUIET |					 \
							  UNZIP_OPT_PRESERVE_FILENAMES | 	 \
							  UNZIP_OPT_ALLOW_SPACE		|		 \
							  UNZIP_OPT_FORCE			|		\
							  UNZIP_OPT_OVERWRITE_ALL )

#if !defined (PATH_SEPARATOR )
#if defined ( CH_MSW )
#define  PATH_SEPARATOR		TEXT( '\\' )
#elif defined( CH_UNIX )
#define  PATH_SEPARATOR		TEXT( '/' )
#else
#error "Unknown platform"
#endif
#endif //   PATH_SEPARATOR		)



								


typedef uch f_array[64];        /* for followers[256][64] */

struct huft;

typedef struct leaf {
    struct leaf *parent;
    struct leaf *next_sibling;
    struct leaf *first_child;
    uch value;
} NODE;

CH_EXTERN_VAR ush  mask_bits[17];


class CH_EXPORT_CLASS ChUnzip
{
	public :

		enum tagZipType { typeUnknown = 0, typeGZIP, typePKZIP };

		ChUnzip();
		~ChUnzip();

		static int GetZipType( const char* pstrZipFile );

		bool UnzipFile( const char* pstrZipFile, chuint32 flOptions = UNZIP_DEF_OPTION,
						 const char **ppstrFile = 0, const char* pstrDstDir = 0 );
		bool UnzipFileTo( const char* pstrZipFile, chuint32 flOptions = UNZIP_DEF_OPTION,
						const char* pstrDstDir = 0, const char* pstrNewFileName = 0 );

	private :
		bool InitUnzip( chuint32 flOptions, const char * pstrZipFile, 
							const char* *ppstrFiles, const char* pstrDstDir = 0,
							const char* pstrNewName = 0  );
		void CleanupUnzip(void);
		int  process_zipfile();    /* return PK-type error code */
		int  find_end_central_dir();   /* return 0 if found, 1 otherwise */
		int  process_end_central_dir();    /* return PK-type error code */
		int  process_cdir_file_hdr();    /* return PK-type error code */
		int  process_local_file_hdr();    /* return PK-type error code */

		int  UncompressGZFile();		


	private :
		chuint32	m_flOptions;
		#if 0
		bool	aflag;		// EBCIDIC - ASCII
		bool	cflag;		// extarct file to view window
		bool	fflag;		// freshen
		bool	jflag;		// junk path names
		bool	overwrite_none;	
		bool	overwrite_all;
		bool	force_flag;	
		bool	tflag;		// test flag
		bool	uflag;		// update flag
		bool	U_flag;		// leave filenames in upper or mixed case 
		bool	vflag;		// view directory (only used in unzip.c) 
		bool	V_flag;		// don't strip VMS version numbers
		bool	zflag;		// display only the archive comment
		bool	process_all_files;
		#endif

		chint32	 ziplen;		// length of zip file
		chint32  csize;        /* used by list_files(), ReadByte(): must be signed */
		chint32 ucsize;       /* used by list_files(), unReduce(), explode() */

		const char 	**fnv;
		char 	sig[5];
		//char 	answerbuf[10];

		min_info info[DIR_BLKSIZ]; 
		min_info *pInfo;
		ulg crc32val;

		//union work area;              /* see unzip.h for the definition of work */
		uch	*slide;


		uch *inbuf, *inptr;     /* input buffer (any size is legal) and pointer */
		int incnt;

		ulg 	bitbuf;
		int 	bits_left;
		bool 	zipeof;

		std::fstream *m_pFile, *m_pOutFile;
		char *zipfn;
		int  newfile;

		char local_hdr_sig[5];    /* remaining signature bytes come later   */
		char central_hdr_sig[5];  /*  (must initialize at runtime so unzip  */
		char end_central_sig[5];  /*  executable won't look like a zipfile) */
		static const char *fnames[2];   /* default filenames vector */

		cdir_file_hdr crec;      /* used in unzip.c, extract.c, misc.c */
		local_file_hdr lrec;     /* used in unzip.c, extract.c */
		ecdir_rec ecrec;         /* used in unzip.c, extract.c */

		chint32 extra_bytes;        	/* used in unzip.c, misc.c */
		chint32 cur_zipfile_bufstart;   /* extract_or_test_files, readbuf, ReadByte */


		//uch *outbuf;                   /* buffer for rle look-back */
		//uch *outptr;
		//uch *outout;                	/* scratch pad for ASCII-native trans */
	   	char *filename;
		const char *m_pstrDstPath; 			// dst dir , new file name
		const char* m_pstrNewName;
		uch  *hold;

		uch *extra_field;  /* used by VMS, Mac and OS/2 versions */
		chint32 outpos;                 /* absolute position in outfile */
		int outcnt;                     /* current position in outbuf */
		//int outfd;
		int mem_mode;
		int disk_full;
	    int didCRlast;


		// Inflate.cpp

		unsigned wp;            		/* current position in slide */
		ulg 	 bb;                    /* bit buffer */
		unsigned bk;                    /* bits in bit buffer */
		unsigned hufts;         		/* track memory usage */

		int huft_build(unsigned *b, unsigned n, unsigned s, ush *d, ush *e,
		                   struct huft **t, int *m);
		int huft_free(struct huft *);

		int inflate_codes(struct huft *, struct huft *, int, int);

		int inflate_stored(void);

		int inflate_fixed(void);

		int inflate_dynamic(void);

		int inflate_block(int *);

		int inflate(void);

		int inflate_free(void);


		struct huft *fixed_tl;
		struct huft *fixed_td;
		int fixed_bl, fixed_bd;
		// unzip.c
		long used_csize;

		// extract.c
		//int newfile;      /* used also in file_io.c (flush()) */
		ulg *crc_32_tab;  /* used also in file_io.c and crypt.c (full version) */

		// unreduce.cpp
		f_array *followers;
		uch Slen[256];
		int factor;
        void unreduce();
		void LoadFollowers();





		/* routines in explode.c  */
		int get_tree(unsigned * l, unsigned n);

		int explode_lit8(struct huft * tb, struct huft * tl, struct huft * td,
		                     int bb, int bl , int bd );

		int explode_lit4(struct huft * tl, struct huft * td, struct huft *,
		                     int, int, int);

		int explode_nolit8(struct huft * tl, struct huft * td, int bl, int bd);

		int explode_nolit4(struct huft * tl, struct huft * td, int bl, int bd);

		int explode(void);

		// extract.c
		static void makecrc( ulg* crc_32_tab );
		int store_info(void);
		int extract_or_test_member(void);
		int extract_or_test_files();    /* return PK-type error code */
		int memextract(uch *tgt, ulg tgtsize, uch *src, ulg srcsize);   /* extract compressed extra */

		/* match.cpp  */
		int matche(register const char *p, register const char *t);
		int matche_after_star (register const char *p, register const char *t);
		int match(const char * string, const char *pattern);

		// file_io.cpp
		int open_input_file();    /* return non-zero if open failed */
		int readbuf( char* buf, register unsigned size);
		int open_outfile();         /* return non-0 if creat failed */
		int readbyte();   /* refill inbuf and return a byte if available, else EOF */
		int flush(uch * rawbuf, ulg size, int unshrink);   /* cflag => always 0; 50 if write error */
		//int FlushOutput();
		int dos2unix( unsigned char * buf, int len);  /* GRR:  rewrite for generic text conversions */
		int do_string(unsigned int len, int option);      /* return PK-type error code */
		int check_for_newer( char *filename);   /* return 1 if existing file newer or equal; */
 		static time_t dos_to_unix_time( unsigned ddate, unsigned dtime);
		void set_file_time_and_close();
		//ulg UpdateCRC(uch *s, ulg n);
		void close_outfile()
		{
			m_pOutFile->close();			
		}
		static int IsVolumeOldFAT(char *name);



		inline ush makeword(uch *b)
		{
		    /*
		     * Convert Intel style 'short' integer to non-Intel non-16-bit
		     * host format.  This routine also takes care of byte-ordering.
		     */
		    return (ush)((b[1] << 8) | b[0]);
		}

		inline ulg makelong(uch *sig)
		{
		    /*
		     * Convert intel style 'long' variable to non-Intel non-16-bit
		     * host format.  This routine also takes care of byte-ordering.
		     */
		    return (((ulg)sig[3]) << 24)
		        + (((ulg)sig[2]) << 16)
		        + (((ulg)sig[1]) << 8)
		        + ((ulg)sig[0]);
		}

		// unshrink.c
		int unshrink();   /* return PK-type error code */
		void partial_clear( NODE *cursib);   /* like, totally recursive, eh? */


		#if !defined( CH_ARCH_16 )
		NODE *node, *bogusnode, *lastfreenode;
		#else
		NODE _huge *node, _huge *bogusnode, _huge *lastfreenode;
		#endif



		// os-dependent methods
		int mapname( int renamed);
		int mapattr();
		static void map2fat( char * pathcomp, char **pEndFAT);
		static int isfloppy(int nDrive);   /* 1 == A:, 2 == B:, etc. */
		static int IsFileNameValid(char *name);

		int checkdir( char * pathcomp, int flag, int& rootlen,
				char*& rootpath, char*&  buildpathHPFS, 
				char*&  buildpathFAT, char*&  endHPFS,
				char*&  endFAT, int& create_dirs,
				unsigned& nLabelDrive, int& volflag, int& created_dir,
				int& fnlen, int& renamed_fullpath   );


 };
