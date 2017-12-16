/*---------------------------------------------------------------------------

  file_io.c

  This file contains routines for doing direct input/output, file-related
  sorts of things.  Most of the system-specific code for unzip is contained
  here, including the non-echoing password code for decryption (bottom).

  ---------------------------------------------------------------------------*/

#include "headers.h"
#define FILE_IO_C
#include <ChUnzip.h>      

#if defined( CH_ARCH_16 )  
#include <stdlib.h>
#include <dos.h>     
#include <direct.h>
#endif
#include "MemDebug.h"



/************************************/
/*  File_IO Local Prototypes, etc.  */
/************************************/



/******************************/
/* Function open_input_file() */
/******************************/

int ChUnzip::open_input_file()    /* return non-zero if open failed */
{
    /*
     *  open the zipfile for reading and in BINARY mode to prevent cr/lf
     *  translation, which would corrupt the bitstreams
     */
	 if ( !m_pFile )
	 {
//	 	m_pFile = ::new fstream( zipfn, ios::in | ios::binary, filebuf::sh_read );
	 	m_pFile = ::new std::fstream( zipfn, std::ios::in | std::ios::binary );
	 }
	 else
	 {
//	 	m_pFile->open( zipfn, ios::in | ios::binary, filebuf::sh_read );
	 	m_pFile->open( zipfn, std::ios::in | std::ios::binary );
	 }

	 if (m_pFile->is_open() )
	 {
	 	 return 0;
	 }
	 else
	 {
	 	return 1;
	}
}





/**********************/
/* Function readbuf() */
/**********************/

int ChUnzip::readbuf( char* buf, register unsigned size)
    //char *buf;
    //register unsigned size;
{                               /* return number of bytes read into buf */
    register int count;
    int n;

    n = size;
    while (size) {
        if (incnt == 0) {
            //if ((incnt = read(zipfd, (char *)inbuf, INBUFSIZ)) <= 0)
					  incnt = m_pFile->read( (char *)inbuf, INBUFSIZ).gcount();
            if (incnt <= 0)
                return (n-size);
            /* buffer ALWAYS starts on a block boundary:  */
            cur_zipfile_bufstart += INBUFSIZ;
            inptr = inbuf;
        }
        count = MIN(size, (unsigned)incnt);
        memcpy(buf, inptr, count);
        buf += count;
        inptr += count;
        incnt -= count;
        size -= count;
    }
    return (n);
}

/*******************************/
/* Function dos_to_unix_time() */   /* only used for freshening/updating */
/*******************************/

time_t ChUnzip::dos_to_unix_time( unsigned ddate, unsigned dtime)
//    unsigned ddate, dtime;
{
    int yr, mo, dy, hh, mm, ss;
#   define YRBASE  1970
    static short yday[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int leap;
    long m_time, days=0;
    //extern long timezone;       /* declared in <time.h> for MSC (& Borland?) */



    /* dissect date */
    yr = ((ddate >> 9) & 0x7f) + (1980 - YRBASE);
    mo = ((ddate >> 5) & 0x0f) - 1;
    dy = (ddate & 0x1f) - 1;

    /* dissect time */
    hh = (dtime >> 11) & 0x1f;
    mm = (dtime >> 5) & 0x3f;
    ss = (dtime & 0x1f) * 2;

    /* leap = # of leap years from BASE up to but not including current year */
    leap = ((yr + YRBASE - 1) / 4);   /* leap year base factor */

    /* calculate days from BASE to this year and add expired days this year */
    days = (yr * 365) + (leap - 492) + yday[mo];

    /* if year is a leap year and month is after February, add another day */
    if ((mo > 1) && ((yr+YRBASE)%4 == 0) && ((yr+YRBASE) != 2100))
        ++days;                 /* OK through 2199 */

    /* convert date & time to seconds relative to 00:00:00, 01/01/YRBASE */
    m_time = ((long)(days + dy) * 86400L) + ((long)hh * 3600) + (mm * 60) + ss;
      /* - 1;   MS-DOS times always rounded up to nearest even second */
    TRACE( "dos_to_unix_time:\n");
    TRACE1( "  m_time before timezone = %ld\n", m_time );

#if defined(CH_ARCH_32) && defined( CH_MSW )
    {
        TIME_ZONE_INFORMATION tzinfo;
        DWORD res;

        /* account for timezone differences */
        res = GetTimeZoneInformation(&tzinfo);
        if (res == TIME_ZONE_ID_STANDARD)
            m_time += 60*(tzinfo.Bias + tzinfo.StandardBias);
        else if (res == TIME_ZONE_ID_DAYLIGHT)
            m_time += 60*(tzinfo.Bias + tzinfo.DaylightBias);
        /* GRR:  are other return-values possible? */
    }
#else /* !WIN32 */
    tzset();                    /* set `timezone' variable */
    m_time += timezone;
#endif /* ?WIN32 */

    TRACE1( "  m_time after timezone =  %ld\n", m_time);

#if !defined(CH_ARCH_32) && !defined( CH_MSW )
    if (localtime((time_t *)&m_time)->tm_isdst)
        m_time -= 60L * 60L;    /* adjust for daylight savings time */
#endif /* !WIN32 */
    TRACE1( "  m_time after DST =       %ld\n", m_time);

    return m_time;

} /* end function dos_to_unix_time() */




/******************************/
/* Function check_for_newer() */   /* only used for freshening/updating */
/******************************/

int ChUnzip::check_for_newer(char * filename)   /* return 1 if existing file newer or equal; */
 //   char *filename;             /*  0 if older; -1 if doesn't exist yet */
{
    time_t existing, archive;
	struct stat	statbuf;

    if (stat(filename, &statbuf))
        return DOES_NOT_EXIST;

    /* round up existing filetime to nearest 2 seconds for comparison */
    existing = (statbuf.st_mtime & 1) ? statbuf.st_mtime+1 : statbuf.st_mtime;
    archive  = dos_to_unix_time(lrec.last_mod_file_date,
                                lrec.last_mod_file_time);

    TRACE3( "check_for_newer:  existing %ld, archive %ld, e-a %ld\n",
      existing, archive, existing-archive);

    return (existing >= archive);

} /* end function check_for_newer() */




/*********************************/
/* Function create_output_file() */
/*********************************/

int ChUnzip::open_outfile()         /* return non-0 if creat failed */
{
/*---------------------------------------------------------------------------
    Create the output file with appropriate permissions.  If we've gotten to
    this point and the file still exists, we have permission to blow it away.
  ---------------------------------------------------------------------------*/	 

  if ( !m_pOutFile )
  {
		m_pOutFile = ::new std::fstream;	
  }

  if ( m_pOutFile->is_open() )
  {
  	m_pOutFile->close();
  }

  //if (!aflag) 
  if (!(UNZIP_OPT_EBCIDIC_ASCII & m_flOptions ) ) 
  {
//  	m_pOutFile->open( filename, ios::out | ios::binary, filebuf::sh_none  );
  	m_pOutFile->open( filename, std::ios::out | std::ios::binary );
	 	// 0 denotes exclusive-shared.
  }
  else
  {
//  	m_pOutFile->open( filename, ios::out | ios::binary, filebuf::sh_none  );
  	m_pOutFile->open( filename, std::ios::out | std::ios::binary );
	 	// 0 denotes exclusive-shared.
  }

  if ( m_pOutFile->is_open() )
  {
  	return 0;
  }
  else
  {
  	return -1;
  }
}





int ChUnzip::readbyte()   /* refill inbuf and return a byte if available, else EOF */
{
    if (mem_mode || (incnt = m_pFile->read((char *)inbuf,INBUFSIZ).gcount()) <= 0)
        return EOF;
    cur_zipfile_bufstart += INBUFSIZ;   /* always starts on a block boundary */
    inptr = inbuf;

	#ifdef CRYPT
    if (pInfo->encrypted) {
        uch *p;
        int n;

        for (n = (long)incnt > csize + 1 ? (int)csize + 1 : incnt,
             p = inptr;  n--;  p++)
            zdecode(*p);
    }
	#endif /* CRYPT */

    --incnt;
    return *inptr++;

} /* end function readbyte() */


/********************/
/* Function flush() */
/********************/

int ChUnzip::flush(uch * rawbuf, ulg size, int unshrink)   /* cflag => always 0; 50 if write error */
{
    register ulg crcval = crc32val;
    register ulg n = size;
    register uch *p, *q;
    uch *transbuf;
    ulg transbufsiz;
	uch * outbuf = 0;
    
    //static int didCRlast = FALSE;


/*---------------------------------------------------------------------------
    Compute the CRC first; if testing or if disk is full, that's it.
  ---------------------------------------------------------------------------*/

    p = rawbuf;
    while (n--)
        crcval = crc_32_tab[((uch)crcval ^ (*p++)) & 0xff] ^ (crcval >> 8);
    crc32val = crcval;

    if (( m_flOptions & UNZIP_OPT_TEST ) || size == 0L)   /* testing or nothing to write:  all done */
        return 0;

	if ( m_flOptions & UNZIP_OPT_DISPLAY_FILE ) 
	{ // view window

	}

    if (disk_full)
        return 50;            /* disk already full:  ignore rest of file */

/*---------------------------------------------------------------------------
    Write the bytes rawbuf[0..size-1] to the output device, first converting
    end-of-lines and ASCII/EBCDIC as needed.  If SMALL_MEM or MED_MEM are NOT
    defined, outbuf is assumed to be at least as large as rawbuf and is not
    necessarily checked for overflow.
  ---------------------------------------------------------------------------*/

    if (!pInfo->textmode) {
        /* GRR:  note that for standard MS-DOS compilers, size argument to
         * fwrite() can never be more than 65534, so WriteError macro will
         * have to be rewritten if size can ever be that large.  For now,
         * never more than 32K.  Also note that write() returns an int, which
         * doesn't necessarily limit size to 32767 bytes if write() is used
         * on 16-bit systems but does make it more of a pain; however, because
         * at least MSC 5.1 has a lousy implementation of fwrite() (as does
         * DEC Ultrix cc), write() is used anyway.
         */
		long iPos = m_pOutFile->tellg();
		m_pOutFile->write( (char *)rawbuf, (int)size );
		long iNewPos = m_pOutFile->tellg();

		if ( ( iNewPos - iPos ) != (long)size )
		{
			disk_full = 1;
			return 50;
		}

        //if (WriteError(rawbuf, size, outfile))  /* write raw binary data */
        //    return cflag? 0 : disk_error();
    } else {
		#if 0
        if (unshrink) 
        {
            /* rawbuf = outbuf */
            transbuf = outbuf2;
            transbufsiz = TRANSBUFSIZ;
        } else 
        #endif
        {
            /* rawbuf = slide */
			outbuf  = new uch[ OUTBUFSIZ + 1];	 /* extra: ASCIIZ */

			if ( !outbuf )
			{
		    	return PK_MEM3;
			}

            transbuf = outbuf;
            transbufsiz = OUTBUFSIZ;
            TRACE1( "\ntransbufsiz = OUTBUFSIZ = %u\n", OUTBUFSIZ);
        }
        if (newfile) {
            didCRlast = FALSE;   /* no previous buffers written */
            newfile = FALSE;
        }
        p = rawbuf;
        if (*p == LF && didCRlast)
            ++p;

    /*-----------------------------------------------------------------------
        Algorithm:  CR/LF => native; lone CR => native; lone LF => native.
        This routine is only for non-raw-VMS, non-raw-VM/CMS files (i.e.,
        stream-oriented files, not record-oriented).
      -----------------------------------------------------------------------*/

        for (didCRlast = FALSE, q = transbuf;  p < rawbuf+size;  ++p) {
            if (*p == CR) {              /* lone CR or CR/LF: EOL either way */
                PutNativeEOL
                if (p == rawbuf+size-1)  /* last char in buffer */
                    didCRlast = TRUE;
                else if (p[1] == LF)     /* get rid of accompanying LF */
                    ++p;
            } else if (*p == LF)         /* lone LF */
                PutNativeEOL
            else
			#if defined( CH_UNIX )
            if (*p != CTRLZ)             /* lose all ^Z's */
			#endif
                *q++ = native(*p);

//#if (defined(SMALL_MEM) || defined(MED_MEM))
//# if (lenEOL == 1)   /* don't check unshrink:  both buffers small but equal */
            //if (!unshrink)
//# endif
                /* check for danger of buffer overflow and flush */
                if (q > transbuf+transbufsiz-lenEOL) {
                    TRACE3( "p - rawbuf = %u   q-transbuf = %u   size = %lu\n",
                      (unsigned)(p-rawbuf), (unsigned)(q-transbuf), size );

					long iPos = m_pOutFile->tellg();
					m_pOutFile->write( (char *)transbuf, (unsigned)(q-transbuf) );
					long iNewPos = m_pOutFile->tellg();

					if ( (iNewPos - iPos ) != (long)(q-transbuf)  )
					{
						disk_full = 1;
						if ( outbuf )
						{
							delete []outbuf;
						}
						return 50;
					}


                    //if (WriteError(transbuf, (unsigned)(q-transbuf), outfile))
                     //   return cflag? 0 : disk_error();
                    q = transbuf;
                    continue;
                }
//#endif /* SMALL_MEM || MED_MEM */
        }

    /*-----------------------------------------------------------------------
        Done translating:  write whatever we've got to file.
      -----------------------------------------------------------------------*/

        TRACE3( "p - rawbuf = %u   q-transbuf = %u   size = %lu\n",
          (unsigned)(p-rawbuf), (unsigned)(q-transbuf), size);
		if ( q > transbuf )
		{
			long iPos = m_pOutFile->tellg();
			m_pOutFile->write( (char *)transbuf, (unsigned)(q-transbuf) );
			long iNewPos = m_pOutFile->tellg();

			if ( ( iNewPos - iPos ) != (long)(q-transbuf)  )
			{
				disk_full = 1;
				if ( outbuf )
				{
					delete []outbuf;
				}
				return 50;
			}
		}
       	//if (q > transbuf &&
          //  WriteError(transbuf, (unsigned)(q-transbuf), outfile))
          //  return cflag? 0 : disk_error();
    }

	if ( outbuf )
	{
		delete []outbuf;
	}
    return 0;

} /* end function flush() */





/************************/
/* Function do_string() */
/************************/

int ChUnzip::do_string( unsigned int len, int option)      /* return PK-type error code */
    //unsigned int len;           /* without prototype, ush converted to this */
    //int option;
{
    long comment_bytes_left, block_length;
    int error=PK_OK;
    ush extra_len;


/*---------------------------------------------------------------------------
    This function processes arbitrary-length (well, usually) strings.  Three
    options are allowed:  SKIP, wherein the string is skipped (pretty logical,
    eh?); DISPLAY, wherein the string is printed to standard output after un-
    dergoing any necessary or unnecessary character conversions; and ZFILENAME,
    wherein the string is put into the filename[] array after undergoing ap-
    propriate conversions (including case-conversion, if that is indicated:
    see the global variable pInfo->lcflag).  The latter option should be OK,
    since filename is now dimensioned at 1025, but we check anyway.

    The string, by the way, is assumed to start at the current file-pointer
    position; its length is given by len.  So start off by checking length
    of string:  if zero, we're already done.
  ---------------------------------------------------------------------------*/

    if (!len)
        return PK_COOL;

    switch (option) {

    /*
     * First case:  print string on standard output.  First set loop vari-
     * ables, then loop through the comment in chunks of OUTBUFSIZ bytes,
     * converting formats and printing as we go.  The second half of the
     * loop conditional was added because the file might be truncated, in
     * which case comment_bytes_left will remain at some non-zero value for
     * all time.  outbuf and slide are used as scratch buffers because they
     * are available (we should be either before or in between any file pro-
     * cessing).
     */

    case DISPLAY:
	{
		uch * outbuf = 0;

        comment_bytes_left = len;
        block_length = OUTBUFSIZ;    /* for the while statement, first time */
        while (comment_bytes_left > 0 && block_length > 0) {

			outbuf  = new uch[ OUTBUFSIZ + 1];	 /* extra: ASCIIZ */

			if ( !outbuf )
			{
		    	return PK_MEM3;
			}

            register uch *p = outbuf;
            register uch *q = outbuf;
            if ((block_length = readbuf((char *)outbuf,
                   (unsigned) MIN((long)OUTBUFSIZ, comment_bytes_left))) == 0)
                return PK_EOF;
            comment_bytes_left -= block_length;

            /* this is why we allocated an extra byte for outbuf: */
            outbuf[block_length] = '\0';   /* terminate w/zero:  ASCIIZ */

            /* remove all ASCII carriage returns comment before printing
             * (since used before A_TO_N(), check for CR instead of '\r')
             */
            while (*p) {
                while (*p == CR)
                    ++p;
                *q++ = *p++;
            }
            /* could check whether (p - outbuf) == block_length here */
            *q = '\0';

            A_TO_N(outbuf);   /* translate string to native */

            /* ran out of local mem -- had to cheat */
            //WriteStringToMsgWin(outbuf, bRealTimeMsgUpdate);
			delete []outbuf;
		}
        break;
	}

    /*
     * Second case:  read string into filename[] array.  The filename should
     * never ever be longer than FILNAMSIZ-1 (1024), but for now we'll check,
     * just to be sure.
     */

    case ZFILENAME:
        extra_len = 0;
        if (len >= FILNAMSIZ) {
            //FPRINTF(stderr, LoadFarString(FilenameTooLongTrunc));
            error = PK_WARN;
            extra_len = len - FILNAMSIZ + 1;
            len = FILNAMSIZ - 1;
        }
        if (readbuf(filename, len) == 0)
            return PK_EOF;
        filename[len] = '\0';   /* terminate w/zero:  ASCIIZ */

        A_TO_N(filename);       /* translate string to native */

        if (pInfo->lcflag)      /* replace with lowercase filename */
            TOLOWER(filename, filename);

        if (pInfo->vollabel && len > 8 && filename[8] == '.') {
            char *p = filename+8;
            while (*p++)
                p[-1] = *p;  /* disk label, and 8th char is dot:  remove dot */
        }

        if (!extra_len)         /* we're done here */
            break;

        /*
         * We truncated the filename, so print what's left and then fall
         * through to the SKIP routine.
         */
        //FPRINTF(stderr, "[ %s ]\n", filename);
        len = extra_len;
        /*  FALL THROUGH...  */

    /*
     * Third case:  skip string, adjusting readbuf's internal variables
     * as necessary (and possibly skipping to and reading a new block of
     * data).
     */

    case SKIP:
        LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        break;

    /*
     * Fourth case:  assume we're at the start of an "extra field"; malloc
     * storage for it and read data into the allocated space.
     */

    case EXTRA_FIELD:
        if (extra_field != (uch *)NULL)
            free(extra_field);
        if ((extra_field = (uch *)malloc(len)) == (uch *)NULL) {
            //FPRINTF(stderr, LoadFarString(ExtraFieldTooLong), len);
            LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        } else
            if (readbuf((char *)extra_field, len) == 0)
                return PK_EOF;
        break;

    } /* end switch (option) */
    return error;

} /* end function do_string() */

/**********************/
/* Function mapattr() */
/**********************/

/* Identical to MS-DOS, OS/2 versions.                                       */
/* However, NT has a lot of extra permission stuff, so this function should  */
/*  probably be extended in the future.                                      */

int ChUnzip::mapattr()
{
    /* set archive bit (file is not backed up): */
    pInfo->file_attr = (unsigned)(crec.external_file_attributes | 32) & 0xff;
    return 0;

} /* end function mapattr() */



/************************/
/*  Function mapname()  */
/************************/

/*
 * There are presently two possibilities in OS/2:  the output filesystem is
 * FAT, or it is HPFS.  If the former, we need to map to FAT, obviously, but
 * we *also* must map to HPFS and store that version of the name in extended
 * attributes.  Either way, we need to map to HPFS, so the main mapname
 * routine does that.  In the case that the output file system is FAT, an
 * extra filename-mapping routine is called in checkdir().  While it should
 * be possible to determine the filesystem immediately upon entry to mapname(),
 * it is conceivable that the DOS APPEND utility could be added to OS/2 some-
 * day, allowing a FAT directory to be APPENDed to an HPFS drive/path.  There-
 * fore we simply check the filesystem at each path component.
 *
 * Note that when alternative IFS's become available/popular, everything will
 * become immensely more complicated.  For example, a Minix filesystem would
 * have limited filename lengths like FAT but no extended attributes in which
 * to store the longer versions of the names.  A BSD Unix filesystem would
 * support paths of length 1024 bytes or more, but it is not clear that FAT
 * EAs would allow such long .LONGNAME fields or that OS/2 would properly
 * restore such fields when moving files from FAT to the new filesystem.
 *
 * GRR:  some or all of the following chars should be checked in either
 *       mapname (HPFS) or map2fat (FAT), depending:  ,=^+'"[]<>|\t&
 */

int ChUnzip::mapname(int renamed)  /* return 0 if no error, 1 if caution (filename trunc), */
  //  int renamed;      /* 2 if warning (skip file because dir doesn't exist), */
{                     /* 3 if error (skip file), 10 if no memory (skip file), */
                      /* IZ_VOL_LABEL if can't do vol label, IZ_CREATED_DIR */
    char pathcomp[FILNAMSIZ];   /* path-component buffer */
    char *pp, *cp=NULL;         /* character pointers */
    char *lastsemi = NULL;      /* pointer to last semi-colon in pathcomp */
    int quote = FALSE;          /* flag:  next char is literal */
    int error = 0;
    register unsigned workch;   /* hold the character being tested */

    int rootlen = 0;      /* length of rootpath */
    char *rootpath = 0;       /* user's "extract-to" directory */
    char *buildpathHPFS = 0;  /* full path (so far) to extracted file, */
    char *buildpathFAT = 0;   /*  both HPFS/EA (main) and FAT versions */
    char *endHPFS = 0;        /* corresponding pointers to end of */
    char *endFAT = 0;         /*  buildpath ('\0') */

	int created_dir;         /* used by mapname(), checkdir() */
	int renamed_fullpath;    /* ditto */
	int fnlen;               /* ditto */
	unsigned nLabelDrive;    /* ditto */
	int volflag = 0;



/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    /* can create path as long as not just freshening, or if user told us */
    //create_dirs = (!fflag || renamed);
    int create_dirs = (!(m_flOptions & UNZIP_OPT_FRESHEN_ONLY ) || renamed);

    created_dir = FALSE;        /* not yet */
    renamed_fullpath = FALSE;
    fnlen = strlen(filename);

    if (renamed) {
        cp = filename - 1;      /* point to beginning of renamed name... */
        while (*++cp)
            if (*cp == '\\')    /* convert backslashes to forward */
                *cp = '/';
        cp = filename;
        /* use temporary rootpath if user gave full pathname */
        if (filename[0] == '/') {
            renamed_fullpath = TRUE;
            pathcomp[0] = '/';  /* copy the '/' and terminate */
            pathcomp[1] = '\0';
            ++cp;
        } else if (isalpha(filename[0]) && filename[1] == ':') {
            renamed_fullpath = TRUE;
            pp = pathcomp;
            *pp++ = *cp++;      /* copy the "d:" (+ '/', possibly) */
            *pp++ = *cp++;
            if (*cp == '/')
                *pp++ = *cp++;  /* otherwise add "./"? */
            *pp = '\0';
        }
    }

    /* pathcomp is ignored unless renamed_fullpath is TRUE: */
    if ((error = checkdir( pathcomp, INIT, rootlen,
				rootpath, buildpathHPFS, 
				buildpathFAT, endHPFS,
				endFAT, create_dirs,
				nLabelDrive, volflag, created_dir,
				fnlen, renamed_fullpath )) != 0)    /* initialize path buffer */
        return error;           /* ...unless no mem or vol label on hard disk */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (!renamed) {             /* cp already set if renamed */
        //if (jflag)              /* junking directories */
        if ( !(m_flOptions & UNZIP_OPT_PRESERVE_FILENAMES ))              /* junking directories */
            cp = (char *)strrchr(filename, '/');
        if (cp == NULL)             /* no '/' or not junking dirs */
            cp = filename;          /* point to internal zipfile-member pathname */
        else
            ++cp;                   /* point to start of last component of path */
    }

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        if (quote) {              /* if character quoted, */
            *pp++ = (char)workch; /*  include it literally */
            quote = FALSE;
        } else
            switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if ((error = checkdir( pathcomp, APPEND_DIR, rootlen,
										rootpath, buildpathHPFS, 
										buildpathFAT, endHPFS,
										endFAT, create_dirs,
										nLabelDrive, volflag, created_dir,
										fnlen, renamed_fullpath   ) ) > 1)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = NULL;  /* leave directory semi-colons alone */
                break;

            case ':':
                *pp++ = '_';      /* drive names not stored in zipfile, */
                break;            /*  so no colons allowed */

            case ';':             /* start of VMS version? */
                lastsemi = pp;    /* remove VMS version later... */
                *pp++ = ';';      /*  but keep semicolon for now */
                break;

            case '\026':          /* control-V quote for special chars */
                quote = TRUE;     /* set flag for next character */
                break;

            case ' ':             /* keep spaces unless specifically */
                /* NT cannot create filenames with spaces on FAT volumes */
                //if (sflag || IsVolumeOldFAT(filename))
                if ( !(m_flOptions & UNZIP_OPT_ALLOW_SPACE ) || IsVolumeOldFAT(filename) )
                    *pp++ = '_';
                else
                    *pp++ = ' ';
                break;

            default:
                /* allow European characters in filenames: */
                if (isprint(workch) || (128 <= workch && workch <= 254))
                    *pp++ = (char)workch;
            } /* end switch */

    } /* end while loop */

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended "###") */
    //if (!V_flag && lastsemi) {
    if (!(m_flOptions & UNZIP_OPT_STRIP_VMS_VER ) && lastsemi) {
        pp = lastsemi + 1;        /* semi-colon was kept:  expect #'s after */
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (filename[fnlen-1] == '/') {
        
		 checkdir( pathcomp, GETPATH, rootlen,
				rootpath, buildpathHPFS, 
				buildpathFAT, endHPFS,
				endFAT, create_dirs,
				nLabelDrive, volflag, created_dir,
				fnlen, renamed_fullpath   );

        //if (created_dir && QCOND2) {
        if (created_dir && m_flOptions & UNZIP_OPT_QUIET) {
/* GRR:  trailing '/'?  need to strip or not? */
            //FPRINTF(stdout, "   creating: %-22s\n", filename);
            /* HG: are we setting the date&time on a newly created dir?  */
            /*     Not quite sure how to do this. It does not seem to    */
            /*     be done in the MS-DOS version of mapname().           */
            return IZ_CREATED_DIR;   /* dir time already set */
        }
        return 2;   /* dir existed already; don't look for data to extract */
    }

    if (*pathcomp == '\0') {
        //FPRINTF(stderr, "mapname:  conversion of %s failed\n", filename);
        return 3;
    }

	 checkdir( pathcomp, APPEND_NAME, rootlen,
			rootpath, buildpathHPFS, 
			buildpathFAT, endHPFS,
			endFAT, create_dirs,
			nLabelDrive, volflag, created_dir,
			fnlen, renamed_fullpath   );       /* returns 1 if truncated:  care? */

	 checkdir( pathcomp, GETPATH, rootlen,
			rootpath, buildpathHPFS, 
			buildpathFAT, endHPFS,
			endFAT, create_dirs,
			nLabelDrive, volflag, created_dir,
			fnlen, renamed_fullpath   )	  ;

    TRACE2( "mapname returns with filename = [%s] (error = %d)\n\n",
      filename, error );

    if (pInfo->vollabel) {   /* set the volume label now */
        char drive[3];	   


        /* Build a drive string, e.g. "b:" */
        //drive[0] = 'a' + nLabelDrive - 1;
        drive[1] = ':';
        drive[2] = '\0';
        //if (QCOND2)
        if (m_flOptions & UNZIP_OPT_QUIET )
            //FPRINTF(stdout, "labelling %s %-22s\n", drive, filename);  
        #if defined( CH_ARCH_16 )
        #pragma message ( "SetVolumeLabel not implemented under win16" )
        #else
        if (!SetVolumeLabel(drive, filename)) {
            //FPRINTF(stderr, "mapname:  error setting volume label\n");
            return 3;
        }    
        #endif
        return 2;   /* success:  skip the "extraction" quietly */
    }

    return error;

} /* end function mapname() */

/*****************************/
/* Function IsVolumeOldFAT() */
/*****************************/

/*
 * Note:  8.3 limits on filenames apply only to old-style FAT filesystems.
 *        More recent versions of Windows (Windows NT 3.5 / Windows 4.0)
 *        can support long filenames (LFN) on FAT filesystems.  Check the
 *        filesystem maximum component length field to detect LFN support.
 *        [GRR:  this routine is only used to determine whether spaces in
 *        filenames are supported...]
 */

int ChUnzip::IsVolumeOldFAT(char *name)
{
 	#if defined( CH_ARCH_32 )
    char     *tmp0;
    char      rootPathName[4];
    char      tmp1[MAX_PATH], tmp2[MAX_PATH];
    unsigned long maxCompLen, fileSysFlags;
	unsigned long 	volSerNo;

    if (isalpha(name[0]) && (name[1] == ':'))
        tmp0 = name;
    else
    {
        GetFullPathName(name, MAX_PATH, tmp1, &tmp0);
        tmp0 = &tmp1[0];
    }
    strncpy(rootPathName, tmp0, 3);   /* Build the root path name, */
    rootPathName[3] = '\0';           /* e.g. "A:/"                */

    GetVolumeInformation(rootPathName, tmp1, MAX_PATH, &volSerNo,
                         &maxCompLen, &fileSysFlags, tmp2, MAX_PATH);

    /* Long Filenames (LFNs) are available if the component length is > 12 */
    
    return maxCompLen <= 12;
    #else 
    return true;
    #endif

}




/**********************/
/* Function map2fat() */        /* Identical to OS/2 version */
/**********************/

void ChUnzip::map2fat( char * pathcomp, char **pEndFAT)
{
    char *ppc = pathcomp;       /* variable pointer to pathcomp */
    char *pEnd = *pEndFAT;      /* variable pointer to buildpathFAT */
    char *pBegin = *pEndFAT;    /* constant pointer to start of this comp. */
    char *last_dot = NULL;      /* last dot not converted to underscore */
    int dotname = FALSE;        /* flag:  path component begins with dot */
                                /*  ("." and ".." don't count) */
    register unsigned workch;   /* hold the character being tested */


    /* Only need check those characters which are legal in HPFS but not
     * in FAT:  to get here, must already have passed through mapname.
     * (GRR:  oops, small bug--if char was quoted, no longer have any
     * knowledge of that.)  Also must truncate path component to ensure
     * 8.3 compliance...
     */
    while ((workch = (uch)*ppc++) != 0) {
        switch (workch) {
            case '[':
            case ']':
                *pEnd++ = '_';      /* convert brackets to underscores */
                break;

            case '.':
                if (pEnd == *pEndFAT) {   /* nothing appended yet... */
                    if (*ppc == '\0')     /* don't bother appending a */
                        break;            /*  "./" component to the path */
                    else if (*ppc == '.' && ppc[1] == '\0') {   /* "../" */
                        *pEnd++ = '.';    /* add first dot, unchanged... */
                        ++ppc;            /* skip second dot, since it will */
                    } else {              /*  be "added" at end of if-block */
                        *pEnd++ = '_';    /* FAT doesn't allow null filename */
                        dotname = TRUE;   /*  bodies, so map .exrc -> _.exrc */
                    }                     /*  (extra '_' now, "dot" below) */
                } else if (dotname) {     /* found a second dot, but still */
                    dotname = FALSE;      /*  have extra leading underscore: */
                    *pEnd = '\0';         /*  remove it by shifting chars */
                    pEnd = *pEndFAT + 1;  /*  left one space (e.g., .p1.p2: */
                    while (pEnd[1]) {     /*  __p1 -> _p1_p2 -> _p1.p2 when */
                        *pEnd = pEnd[1];  /*  finished) [opt.:  since first */
                        ++pEnd;           /*  two chars are same, can start */
                    }                     /*  shifting at second position] */
                }
                last_dot = pEnd;    /* point at last dot so far... */
                *pEnd++ = '_';      /* convert dot to underscore for now */
                break;

            default:
                *pEnd++ = (char)workch;

        } /* end switch */
    } /* end while loop */

    *pEnd = '\0';                 /* terminate buildpathFAT */

    /* NOTE:  keep in mind that pEnd points to the end of the path
     * component, and *pEndFAT still points to the *beginning* of it...
     * Also note that the algorithm does not try to get too fancy:
     * if there are no dots already, the name either gets truncated
     * at 8 characters or the last underscore is converted to a dot
     * (only if more characters are saved that way).  In no case is
     * a dot inserted between existing characters.
     */
    if (last_dot == NULL) {       /* no dots:  check for underscores... */
        char *plu = strrchr(pBegin, '_');   /* pointer to last underscore */

        if (plu == NULL) {   /* no dots, no underscores:  truncate at 8 chars */
            *pEndFAT += 8;        /* (or could insert '.' and keep 11...?) */
            if (*pEndFAT > pEnd)
                *pEndFAT = pEnd;  /* oops...didn't have 8 chars to truncate */
            else
                **pEndFAT = '\0';
        } else if (MIN(plu - pBegin, 8) + MIN(pEnd - plu - 1, 3) > 8) {
            last_dot = plu;       /* be lazy:  drop through to next if-blk */
        } else if ((pEnd - *pEndFAT) > 8) {
            *pEndFAT += 8;        /* more fits into just basename than if */
            **pEndFAT = '\0';     /*  convert last underscore to dot */
        } else
            *pEndFAT = pEnd;      /* whole thing fits into 8 chars or less */
    }

    if (last_dot != NULL) {       /* one dot (or two, in the case of */
        *last_dot = '.';          /*  "..") is OK:  put it back in */

        if ((last_dot - pBegin) > 8) {
            char *p=last_dot, *q=pBegin+8;
            int i;

            for (i = 0;  (i < 4) && *p;  ++i)  /* too many chars in basename: */
                *q++ = *p++;                   /*  shift .ext left and trun- */
            *q = '\0';                         /*  cate/terminate it */
            *pEndFAT = q;
        } else if ((pEnd - last_dot) > 4) {    /* too many chars in extension */
            *pEndFAT = last_dot + 4;
            **pEndFAT = '\0';
        } else
            *pEndFAT = pEnd;   /* filename is fine; point at terminating zero */
    }
} /* end function map2fat() */


int ChUnzip::isfloppy(int nDrive)   /* 1 == A:, 2 == B:, etc. */
{  
	#if defined( CH_ARCH_16 )
     return (GetDriveType(nDrive - 1) == DRIVE_REMOVABLE);
  	#else
    char rootPathName[4];

    rootPathName[0] = 'A' + nDrive - 1;   /* Build the root path name, */
    rootPathName[1] = ':';                /* e.g. "A:/"                */
    rootPathName[2] = '/';
    rootPathName[3] = '\0';

    return (GetDriveType(rootPathName) == DRIVE_REMOVABLE);
    #endif
   

} /* end function isfloppy() */

/******************************/
/* Function IsFileNameValid() */
/******************************/

int ChUnzip::IsFileNameValid(char *name)
{
    HFILE    hf;
    OFSTRUCT of;

    hf = OpenFile(name, &of, OF_READ | OF_SHARE_DENY_NONE);
    if (hf == HFILE_ERROR) 
    	#if defined( CH_ARCH_16 ) 
    	#define ERROR_INVALID_NAME			0x0002
    	#define ERROR_FILENAME_EXCED_RANGE	0x0003
        switch ( of.nErrCode )
    	#else
        switch (GetLastError())
        #endif
        {
            case ERROR_INVALID_NAME:
            case ERROR_FILENAME_EXCED_RANGE:
                return FALSE;
            default:
                return TRUE;
        }
    else
        _lclose(hf);
    return TRUE;     
    
#if defined( CH_ARCH_16 ) 
#undef ERROR_INVALID_NAME			
#undef ERROR_FILENAME_EXCED_RANGE	
#endif    
}



#define MKDIR(path,mode)   mkdir(path)


/***********************/       /* Borrowed from os2.c for UnZip 5.1.        */
/* Function checkdir() */       /* Difference: no EA stuff                   */
/***********************/       /*             HPFS stuff works on NTFS too  */

int ChUnzip::checkdir( char * pathcomp, int flag, int& rootlen,
				char*& rootpath, char*&  buildpathHPFS, 
				char*&  buildpathFAT, char*&  endHPFS,
				char*&  endFAT, int& create_dirs,
				unsigned& nLabelDrive, int& volflag, int& created_dir,
				int &fnlen, int& renamed_fullpath )
    //char *pathcomp;
    //int flag;
/*
 * returns:  1 - (on APPEND_NAME) truncated filename
 *           2 - path doesn't exist, not allowed to create
 *           3 - path doesn't exist, tried to create and failed; or
 *               path exists and is not a directory, but is supposed to be
 *           4 - path is too long
 *          10 - can't allocate memory for filename buffers
 */
{
    //static int rootlen = 0;      /* length of rootpath */
    //static char *rootpath;       /* user's "extract-to" directory */
    //static char *buildpathHPFS;  /* full path (so far) to extracted file, */
    //static char *buildpathFAT;   /*  both HPFS/EA (main) and FAT versions */
    //static char *endHPFS;        /* corresponding pointers to end of */
    //static char *endFAT;         /*  buildpath ('\0') */

#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)

struct stat statbuf;



/*---------------------------------------------------------------------------
    APPEND_DIR:  append the path component to the path being built and check
    for its existence.  If doesn't exist and we are creating directories, do
    so for this one; else signal success or error as appropriate.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_DIR) {
        char *p = pathcomp;
        int too_long=FALSE;

        TRACE1(  "appending dir segment [%s]\n", pathcomp );

        while ((*endHPFS = *p++) != '\0')     /* copy to HPFS filename */
            ++endHPFS;

        if (IsFileNameValid(buildpathHPFS)) {
            p = pathcomp;
            while ((*endFAT = *p++) != '\0')  /* copy to FAT filename, too */
                ++endFAT;
        } else {
			/* GRR:  check error return? */
            map2fat(pathcomp, &endFAT);  /* map, put in FAT fn, update endFAT */
        }

        /* GRR:  could do better check, see if overrunning buffer as we go:
         * check endHPFS-buildpathHPFS after each append, set warning variable
         * if within 20 of FILNAMSIZ; then if var set, do careful check when
         * appending.  Clear variable when begin new path. */

        /* next check:  need to append '/', at least one-char name, '\0' */
        if ((endHPFS-buildpathHPFS) > FILNAMSIZ-3)
            too_long = TRUE;                 /* check if extracting dir? */
        if (stat(buildpathFAT, &statbuf))    /* path doesn't exist */
        {
            if (!create_dirs) {   /* told not to create (freshening) */
                delete []buildpathHPFS;
                delete []buildpathFAT;
                //free(buildpathHPFS);
                //free(buildpathFAT);
                return 2;         /* path doesn't exist:  nothing to do */
            }
            if (too_long) {   /* GRR:  should allow FAT extraction w/o EAs */
               // FPRINTF(stderr, "checkdir error:  path too long: %s\n",
               //   buildpathHPFS);
                //fflush(stderr);
                delete []buildpathHPFS;
                delete []buildpathFAT;
                //free(buildpathHPFS);
                //free(buildpathFAT);
                return 4;         /* no room for filenames:  fatal */
            }
            if (MKDIR(buildpathFAT, 0777) == -1) {   /* create the directory */
                //fflush(stderr);
                delete []buildpathHPFS;
                delete []buildpathFAT;
                //free(buildpathHPFS);
                //free(buildpathFAT);
                return 3;      /* path didn't exist, tried to create, failed */
            }
            created_dir = TRUE;
        } else if (!S_ISDIR(statbuf.st_mode)) {
            //fflush(stderr);
            delete []buildpathHPFS;
            delete []buildpathFAT;
            //free(buildpathHPFS);
            //free(buildpathFAT);
            return 3;          /* path existed but wasn't dir */
        }
        if (too_long) {
            //FPRINTF(stderr, "checkdir error:  path too long: %s\n",
              //buildpathHPFS);
            //fflush(stderr);
            delete []buildpathHPFS;
            delete []buildpathFAT;
            //free(buildpathHPFS);
            //free(buildpathFAT);
            return 4;         /* no room for filenames:  fatal */
        }
        *endHPFS++ = '/';
        *endFAT++ = '/';
        *endHPFS = *endFAT = '\0';
        TRACE1( "buildpathHPFS now = [%s]\n", buildpathHPFS );
        TRACE1( "buildpathFAT now =  [%s]\n", buildpathFAT );
        return 0;

    } /* end if (FUNCTION == APPEND_DIR) */

/*---------------------------------------------------------------------------
    GETPATH:  copy full FAT path to the string pointed at by pathcomp (want
    filename to reflect name used on disk, not EAs; if full path is HPFS,
    buildpathFAT and buildpathHPFS will be identical).  Also free both paths.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == GETPATH) {
        TRACE1("getting and freeing FAT path [%s]\n", buildpathFAT);
        TRACE1("freeing HPFS path [%s]\n", buildpathHPFS);
        strcpy(pathcomp, buildpathFAT);
        delete []buildpathHPFS;
        delete []buildpathFAT;
        //free(buildpathHPFS);
        //free(buildpathFAT);
        buildpathHPFS = buildpathFAT = endHPFS = endFAT = NULL;
        return 0;
    }

/*---------------------------------------------------------------------------
    APPEND_NAME:  assume the path component is the filename; append it and
    return without checking for existence.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_NAME) {
        char *p = pathcomp;
        int error = 0;

        TRACE1( "appending filename [%s]\n", pathcomp );
        while ((*endHPFS = *p++) != '\0') {    /* copy to HPFS filename */
            ++endHPFS;
            if ((endHPFS-buildpathHPFS) >= FILNAMSIZ) {
                //fflush(stderr);
                error = 1;   /* filename truncated */
            }
        }

        if ( pInfo->vollabel || IsFileNameValid(buildpathHPFS)) {
            p = pathcomp;
            while ((*endFAT = *p++) != '\0')   /* copy to FAT filename, too */
                ++endFAT;
        } else {
            map2fat(pathcomp, &endFAT);  /* map, put in FAT fn, update endFAT */
        }
        //Trace((stderr, "buildpathHPFS: %s\nbuildpathFAT:  %s\n",
         // buildpathHPFS, buildpathFAT));

        return error;  /* could check for existence, prompt for new name... */

    } /* end if (FUNCTION == APPEND_NAME) */

/*---------------------------------------------------------------------------
    INIT:  allocate and initialize buffer space for the file currently being
    extracted.  If file was renamed with an absolute path, don't prepend the
    extract-to path.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == INIT) {

/* HG: variable not used here */
/*      char *p;              */

        //Trace((stderr, "initializing buildpathHPFS and buildpathFAT to "));
		buildpathHPFS = new char[fnlen+rootlen+1];
		if ( !buildpathHPFS )
        //if ((buildpathHPFS = (char *)malloc(fnlen+rootlen+1)) == NULL)
            return 10;

		buildpathFAT = new char[fnlen+rootlen+1];
		if ( !buildpathFAT ) {
        //if ((buildpathFAT = (char *)malloc(fnlen+rootlen+1)) == NULL) {
            //free(buildpathHPFS);
            delete []buildpathHPFS;
            return 10;
        }

        if (pInfo->vollabel) {  /* use root or renamed path, but don't store */
/* GRR:  for network drives, do strchr() and return IZ_VOL_LABEL if not [1] */
            if (renamed_fullpath && pathcomp[1] == ':')
                *buildpathHPFS = ToLower(*pathcomp);
            else if (!renamed_fullpath && rootpath && rootpath[1] == ':')
                *buildpathHPFS = ToLower(*rootpath);
            else { 
            	#if defined( CH_ARCH_16 )       
              	char tmpN[MAX_PATH];
              	if ( !_fullpath( tmpN, ".", MAX_PATH ) )
              	{
              		return 1;		
              	}
            	#else
                char tmpN[MAX_PATH], *tmpP;
                if (GetFullPathName(".", MAX_PATH, tmpN, &tmpP) > MAX_PATH)
                { /* by definition of MAX_PATH we should never get here */
                    //FPRINTF(stderr,
                    //      "checkdir warning: current dir path too long\n");
                    return 1;   /* can't get drive letter */
                }  
                #endif
                nLabelDrive = ToLower( *tmpN ) - 'a' + 1;
                *buildpathHPFS = (char)(nLabelDrive - 1 + 'a');
            }
            nLabelDrive = *buildpathHPFS - 'a' + 1;     /* save for mapname() */
            if (volflag == 0 || *buildpathHPFS < 'a' ||   /* no labels/bogus? */
                (volflag == 1 && !isfloppy(nLabelDrive))) {  /* -$:  no fixed */
		        delete []buildpathHPFS;
		        delete []buildpathFAT;
		        //free(buildpathHPFS);
		        //free(buildpathFAT);
                return IZ_VOL_LABEL;   /* skipping with message */
            }
            *buildpathHPFS = '\0';
        } else if (renamed_fullpath)   /* pathcomp = valid data */
            strcpy(buildpathHPFS, pathcomp);
        else if (rootlen > 0)
            strcpy(buildpathHPFS, rootpath);
        else
            *buildpathHPFS = '\0';
        endHPFS = buildpathHPFS;
        endFAT = buildpathFAT;
        while ((*endFAT = *endHPFS) != '\0') {
            ++endFAT;
            ++endHPFS;
        }
        //Trace((stderr, "[%s]\n", buildpathHPFS));
        return 0;
    }

/*---------------------------------------------------------------------------
    ROOT:  if appropriate, store the path in rootpath and create it if neces-
    sary; else assume it's a zipfile member and return.  This path segment
    gets used in extracting all members from every zipfile specified on the
    command line.  Note that under OS/2 and MS-DOS, if a candidate extract-to
    directory specification includes a drive letter (leading "x:"), it is
    treated just as if it had a trailing '/'--that is, one directory level
    will be created if the path doesn't exist, unless this is otherwise pro-
    hibited (e.g., freshening).
  ---------------------------------------------------------------------------*/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (FUNCTION == ROOT) {
        //Trace((stderr, "initializing root path to [%s]\n", pathcomp));
        if (pathcomp == NULL) {
            rootlen = 0;
            return 0;
        }
        if ((rootlen = strlen(pathcomp)) > 0) {
            int had_trailing_pathsep=FALSE, has_drive=FALSE, xtra=2;

            if (isalpha(pathcomp[0]) && pathcomp[1] == ':')
                has_drive = TRUE;   /* drive designator */
            if (pathcomp[rootlen-1] == '/') {
                pathcomp[--rootlen] = '\0';
                had_trailing_pathsep = TRUE;
            }
            if (has_drive && (rootlen == 2)) {
                if (!had_trailing_pathsep)   /* i.e., original wasn't "x:/" */
                    xtra = 3;      /* room for '.' + '/' + 0 at end of "x:" */
            } else if (rootlen > 0) {     /* need not check "x:." and "x:/" */
                if (stat(pathcomp, &statbuf) || !S_ISDIR(statbuf.st_mode)) {
                    /* path does not exist */
                    if (!create_dirs                 /* || iswild(pathcomp) */
#ifdef OLD_EXDIR
                                     || (!has_drive && !had_trailing_pathsep)
#endif
                                                                             ) {
                        rootlen = 0;
                        return 2;   /* treat as stored file */
                    }
                    /* create directory (could add loop here to scan pathcomp
                     * and create more than one level, but really necessary?) */
                    if (MKDIR(pathcomp, 0777) == -1) {
                        //FPRINTF(stderr,
                          //"checkdir:  can't create extraction directory: %s\n",
                          //pathcomp);
                        //fflush(stderr);
                        rootlen = 0;   /* path didn't exist, tried to create, */
                        return 3;  /* failed:  file exists, or need 2+ levels */
                    }
                }
            }
			rootpath = new char[rootlen+xtra];
			if ( !rootpath ) {
            //if ((rootpath = (char *)malloc(rootlen+xtra)) == NULL) {
                rootlen = 0;
                return 10;
            }
            strcpy(rootpath, pathcomp);
            if (xtra == 3)                  /* had just "x:", make "x:." */
                rootpath[rootlen++] = '.';
            rootpath[rootlen++] = '/';
            rootpath[rootlen] = '\0';
        }
        //Trace((stderr, "rootpath now = [%s]\n", rootpath));
        return 0;
    }
#endif /* !SFX || SFX_EXDIR */

/*---------------------------------------------------------------------------
    END:  free rootpath, immediately prior to program exit.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == END) {
        //Trace((stderr, "freeing rootpath\n"));
        if (rootlen > 0)
            //free(rootpath);
            delete []rootpath;
        return 0;
    }

    return 99;  /* should never reach */

} /* end function checkdir() */















#if defined( CH_UNIX )

/***********************/
/* Function dos2unix() */
/***********************/

static int ChUnzip::dos2unix( unsigned char * buf, int len)   /* GRR:  rewrite for generic text conversions */
{
    int new_len;
    int i;
    unsigned char *walker;

    new_len = len;
    walker = outout;
#ifdef MACOS
    /*
     * Mac wants to strip LFs instead CRs from CRLF pairs
     */
    if (CR_flag && *buf == LF) {
        buf++;
        new_len--;
        len--;
        CR_flag = buf[len] == CR;
    }
    else
        CR_flag = buf[len - 1] == CR;
    for (i = 0; i < len; i += 1) {
        *walker++ = ascii_to_native(*buf);
        if (*buf == LF) walker[-1] = CR;
        if (*buf++ == CR && *buf == LF) {
            new_len--;
            buf++;
            i++;
        }
    }
#else /* !MACOS */
    if (CR_flag && *buf != LF)
        *walker++ = ascii_to_native(CR);
    CR_flag = buf[len - 1] == CR;
    for (i = 0; i < len; i += 1) {
        *walker++ = ascii_to_native(*buf);
        if (*buf++ == CR && *buf == LF) {
            new_len--;
            walker[-1] = ascii_to_native(*buf++);
            i++;
        }
    }
    /*
     * If the last character is a CR, then "ignore it" for now...
     */
    if (walker[-1] == ascii_to_native(CR))
        new_len--;
#endif /* ?MACOS */
    return new_len;
}

#endif /* CH_UNIX */





#ifdef CH_MSW

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void ChUnzip::set_file_time_and_close()
 /*
  * MS-DOS AND OS/2 VERSION (Mac, Unix/VMS versions are below)
  *
  * Set the output file date/time stamp according to information from the
  * zipfile directory record for this member, then close the file and set
  * its permissions (archive, hidden, read-only, system).  Aside from closing
  * the file, this routine is optional (but most compilers support it).
  */
{

/*---------------------------------------------------------------------------
     Do not attempt to set the time stamp on standard output.
  ---------------------------------------------------------------------------*/

    //if (cflag) {
    if (m_flOptions & UNZIP_OPT_DISPLAY_FILE ) 
    {
        //close(outfd);
        return;
    }

/*---------------------------------------------------------------------------
    Copy and/or convert time and date variables, if necessary; then set the
    file time/date.
  ---------------------------------------------------------------------------*/

#if	defined( CH_MSW ) && defined( CH_ARCH_32 )
    {
        FILETIME ft;     /* 64-bit value made up of two 32 bit [low & high] */
        WORD wDOSDate;   /* for vconvertin from DOS date to Windows NT */
        WORD wDOSTime;
        HANDLE hFile;    /* file handle (defined in Windows NT) */

        wDOSTime = (WORD) lrec.last_mod_file_time;
        wDOSDate = (WORD) lrec.last_mod_file_date;

        /* The DosDateTimeToFileTime() function converts a DOS date/time
         * into a 64 bit Windows NT file time */
        DosDateTimeToFileTime(wDOSDate, wDOSTime, &ft);

        /* Close the file and then re-open it using the Win32
         * CreateFile call, so that the file can be created
         * with GENERIC_WRITE access, otherwise the SetFileTime
         * call will fail. */
        m_pOutFile->close();

        hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
             FILE_ATTRIBUTE_NORMAL, NULL);

        if (!SetFileTime(hFile, NULL, NULL, &ft))
            printf("\nSetFileTime failed: %d\n", GetLastError());
        CloseHandle(hFile);
        return;
    }
#else /* !WIN32 */
    _dos_setftime( m_pOutFile->fd(), lrec.last_mod_file_date, lrec.last_mod_file_time);

/*---------------------------------------------------------------------------
    And finally we can close the file...at least everybody agrees on how to
    do *this*.  I think...  Oh yeah, also change the mode according to the
    stored file attributes, since we didn't do that when we opened the dude.
  ---------------------------------------------------------------------------*/

    m_pOutFile->close();
#endif /* ?WIN32 */
} /* end function set_file_time_and_close() (DOS, OS/2) */





#else                           /* !DOS_OS2 */
#ifdef MACOS                    /* Mac */

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void set_file_time_and_close()
 /*
  * MAC VERSION
  */
{
    long m_time;
    DateTimeRec dtr;
    ParamBlockRec pbr;
    HParamBlockRec hpbr;
    OSErr err;

    if (outfd != 1) {
        close(outfd);

        /*
         * Macintosh bases all file modification times on the number of seconds
         * elapsed since Jan 1, 1904, 00:00:00.  Therefore, to maintain
         * compatibility with MS-DOS archives, which date from Jan 1, 1980,
         * with NO relation to GMT, the following conversions must be made:
         *      the Year (yr) must be incremented by 1980;
         *      and converted to seconds using the Mac routine Date2Secs(),
         *      almost similar in complexity to the Unix version :-)
         *                                     J. Lee
         */

        dtr.year = (((lrec.last_mod_file_date >> 9) & 0x7f) + 1980);
        dtr.month = ((lrec.last_mod_file_date >> 5) & 0x0f);
        dtr.day = (lrec.last_mod_file_date & 0x1f);

        dtr.hour = ((lrec.last_mod_file_time >> 11) & 0x1f);
        dtr.minute = ((lrec.last_mod_file_time >> 5) & 0x3f);
        dtr.second = ((lrec.last_mod_file_time & 0x1f) * 2);

        Date2Secs(&dtr, (unsigned long *)&m_time);
        CtoPstr(filename);
        if (hfsflag) {
            hpbr.fileParam.ioNamePtr = (StringPtr)filename;
            hpbr.fileParam.ioVRefNum = gnVRefNum;
            hpbr.fileParam.ioDirID = glDirID;
            hpbr.fileParam.ioFDirIndex = 0;
            err = PBHGetFInfo(&hpbr, 0L);
            hpbr.fileParam.ioFlMdDat = m_time;
            if ( !fMacZipped )
                hpbr.fileParam.ioFlCrDat = m_time;
            hpbr.fileParam.ioDirID = glDirID;
            if (err == noErr)
                err = PBHSetFInfo(&hpbr, 0L);
            if (err != noErr)
                printf("error:  can't set the time for %s\n", filename);
        } else {
            pbr.fileParam.ioNamePtr = (StringPtr)filename;
            pbr.fileParam.ioVRefNum = pbr.fileParam.ioFVersNum =
              pbr.fileParam.ioFDirIndex = 0;
            err = PBGetFInfo(&pbr, 0L);
            pbr.fileParam.ioFlMdDat = pbr.fileParam.ioFlCrDat = m_time;
            if (err == noErr)
                err = PBSetFInfo(&pbr, 0L);
            if (err != noErr)
                printf("error:  can't set the time for %s\n", filename);
        }

        /* set read-only perms if needed */
        if ((err == noErr) && !(pInfo->unix_attr & S_IWRITE)) {
            if (hfsflag) {
                hpbr.fileParam.ioNamePtr = (StringPtr)filename;
                hpbr.fileParam.ioVRefNum = gnVRefNum;
                hpbr.fileParam.ioDirID = glDirID;
                err = PBHSetFLock(&hpbr, 0);
            } else
                err = SetFLock((ConstStr255Param)filename, 0);
        }
        PtoCstr(filename);
    }
}





#else /* !MACOS... */
#if (!defined(MTS) && !defined(VMS))   /* && !MTS (can't do) && !VMS: only one
                                  * left is UNIX (for VMS use code in vms.c) */

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void ChUnzip::set_file_time_and_close()
 /*
  * UNIX VERSION (MS-DOS & OS/2, Mac versions are above)
  */
{
    static short yday[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    long m_time;
    int yr, mo, dy, hh, mm, ss, leap, days=0;
    struct utimbuf {
        time_t actime;          /* new access time */
        time_t modtime;         /* new modification time */
    } tp;
#ifdef AMIGA
#   define YRBASE  1978         /* in AmigaDos, counting begins 01-Jan-1978 */
    struct DateStamp myadate;
/*  extern char *_TZ;   no longer used? */
#else /* !AMIGA */
#   define YRBASE  1970
#ifdef BSD
#ifndef __386BSD__
    static struct timeb tbp;
#endif /* !__386BSD__ */
#else /* !BSD */
    extern long timezone;
#endif /* ?BSD */
#endif /* ?AMIGA */


    /*
     * Close the file *before* setting its time under Unix and AmigaDos.
     */
#ifdef AMIGA
    if (cflag)                  /* can't set time on stdout */
        return;
    close(outfd);
#else /* !AMIGA */
    close(outfd);
    if (cflag)                  /* can't set time on stdout */
        return;
#endif /* ?AMIGA */

    /*
     * These date conversions look a little weird, so I'll explain.
     * UNIX bases all file modification times on the number of seconds
     * elapsed since Jan 1, 1970, 00:00:00 GMT.  Therefore, to maintain
     * compatibility with MS-DOS archives, which date from Jan 1, 1980,
     * with NO relation to GMT, the following conversions must be made:
     *      the Year (yr) must be incremented by 10;
     *      the Date (dy) must be decremented by 1;
     *      and the whole mess must be adjusted by TWO factors:
     *          relationship to GMT (ie.,Pacific Time adds 8 hrs.),
     *          and whether or not it is Daylight Savings Time.
     * Also, the usual conversions must take place to account for leap years,
     * etc.
     *                                     C. Seaman
     */

    /* dissect date */
    yr = ((lrec.last_mod_file_date >> 9) & 0x7f) + (1980 - YRBASE);
    mo = ((lrec.last_mod_file_date >> 5) & 0x0f) - 1;
    dy = (lrec.last_mod_file_date & 0x1f) - 1;

    /* dissect time */
    hh = (lrec.last_mod_file_time >> 11) & 0x1f;
    mm = (lrec.last_mod_file_time >> 5) & 0x3f;
    ss = (lrec.last_mod_file_time & 0x1f) * 2;

    /* leap = # of leap years from BASE up to but not including current year */
    leap = ((yr + YRBASE - 1) / 4);   /* leap year base factor */

    /* How many days from BASE to this year? (& add expired days this year) */
    days = (yr * 365) + (leap - 492) + yday[mo];

    /* if year is a leap year and month is after February, add another day */
    if ((mo > 1) && ((yr+YRBASE)%4 == 0) && ((yr+YRBASE) != 2100))
        ++days;                 /* OK through 2199 */

#ifdef AMIGA
/*  _TZ = getenv("TZ"); does Amiga not have TZ and tzset() after all? */
    myadate.ds_Days   =   days+dy-2;   /* off by one? */
    myadate.ds_Minute =   hh*60+mm;
    myadate.ds_Tick   =   ss*TICKS_PER_SECOND;

    if (!(SetFileDate(filename, &myadate)))
        fprintf(stderr, "error:  can't set the time for %s\n", filename);

#else /* !AMIGA */
    /* convert date & time to seconds relative to 00:00:00, 01/01/YRBASE */
    m_time = ((days + dy) * 86400) + (hh * 3600) + (mm * 60) + ss;

#ifdef BSD
#ifndef __386BSD__
    ftime(&tbp);
    m_time += tbp.timezone * 60L;
#endif
/* #elif WIN32
 * don't do anything right now (esp. since "elif" is not legal for old cc's */
#else /* !BSD */
    tzset();                    /* set `timezone' */
    m_time += timezone;         /* account for timezone differences */
#endif /* ?BSD */

#ifdef __386BSD__
    m_time += localtime(&m_time)->tm_gmtoff;
#else
    if (localtime(&m_time)->tm_isdst)
        m_time -= 60L * 60L;    /* adjust for daylight savings time */
#endif

    tp.actime = m_time;         /* set access time */
    tp.modtime = m_time;        /* set modification time */

    /* set the time stamp on the file */
    if (utime(filename, &tp))
        fprintf(stderr, "error:  can't set the time for %s\n", filename);
#endif /* ?AMIGA */
}

#endif /* !MTS && !VMS */
#endif /* ?MACOS */
#endif /* ?DOS_OS2 */
