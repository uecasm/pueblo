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

	Implementation of unzip.

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"
#include <ChUnzip.h>
#include "MemDebug.h"

CH_GLOBAL_VAR ush mask_bits[17] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

const char *ChUnzip::fnames[2] = {"*", NULL};   /* default filenames vector */


ChUnzip::ChUnzip()
{
	m_pFile = m_pOutFile = 0;
}

ChUnzip::~ChUnzip()
{ 

	if ( m_pOutFile )
	{
		::delete m_pOutFile;
	}
	
	if ( m_pFile )
	{
		::delete m_pFile;
	}


}

int ChUnzip::GetZipType( const char* pstrZipFile )
{
//	 fstream *pFile = ::new fstream( pstrZipFile, ios::in | ios::binary, filebuf::sh_read );
	 std::fstream *pFile = ::new std::fstream( pstrZipFile, std::ios::in | std::ios::binary );

	 if ( pFile->is_open() )
	 {	// first check if it is GZIP
		chuint8 tmpBuf[2];
		if ( pFile->read( (char *)tmpBuf, 2 ).gcount() == 2 )
		{
		    if ( tmpBuf[0] == GZ_MAGIC_1  && tmpBuf[1] == GZ_MAGIC_2) 
		    {
				::delete pFile;
		        return typeGZIP;
		    }
			else
			{// check if it is PKZIP
			    int i, numblks;
			    chint32 tail_len;



			/*---------------------------------------------------------------------------
			    Treat case of short zipfile separately.
			  ---------------------------------------------------------------------------*/
				uch* inbuf = new uch[ INBUFSIZ ];

				int ziplen;
				int incnt;
				uch *inptr;

				#if defined( CH_MSW )
				struct _stat statbuf;

			    _stat(pstrZipFile, &statbuf); 
				#else
				struct stat statbuf;

			    stat(pstrZipFile, &statbuf); 
				#endif

		        ziplen = statbuf.st_size;

				char local_hdr_sig[5];
				char central_hdr_sig[5];
				char end_central_sig[5];

			    local_hdr_sig[0] = central_hdr_sig[0] = end_central_sig[0] = '\120';
			    local_hdr_sig[1] = central_hdr_sig[1] = end_central_sig[1] = '\0';


			    strcat(local_hdr_sig, LOCAL_HDR_SIG);
			    strcat(central_hdr_sig, CENTRAL_HDR_SIG);
			    strcat(end_central_sig, END_CENTRAL_SIG);



			    if (ziplen <= INBUFSIZ) 
			    {
			        pFile->seekg( 0L, std::ios::beg );
			        if ((incnt = (pFile->read((char *)inbuf,(unsigned int)ziplen).gcount()) ) ==
			             (int)ziplen)

			            /* 'P' must be at least 22 bytes from end of zipfile */
			            for (inptr = inbuf+(int)ziplen-22;  inptr >= inbuf;  --inptr)
			                if ((ascii_to_native(*inptr) == 'P')  &&
			                     !strncmp((char *)inptr, end_central_sig, 4)) 
			                {
			                    incnt -= inptr - inbuf;
								::delete pFile;
								delete inbuf;

			                    return typePKZIP;   /* found it! */
			                }               /* ...otherwise fall through & fail */

			/*---------------------------------------------------------------------------
			    Zipfile is longer than INBUFSIZ:  may need to loop.  Start with short
			    block at end of zipfile (if not TOO short).
			  ---------------------------------------------------------------------------*/

			    } 
			    else 
			    {
					char hold[4];
					long cur_zipfile_bufstart;

			        if ((tail_len = ziplen % INBUFSIZ) > ECREC_SIZE) 
			        {
									cur_zipfile_bufstart = pFile->seekg( ziplen-tail_len, std::ios::beg ).tellg();

			            if ((incnt = (pFile->read((char *)inbuf,(unsigned int)tail_len).gcount())) !=
			                 (int)tail_len)
			                goto fail;      /* shut up; it's expedient. */

			            /* 'P' must be at least 22 bytes from end of zipfile */
			            for (inptr = inbuf+(int)tail_len-22;  inptr >= inbuf;  --inptr)
			                if ((ascii_to_native(*inptr) == 'P')  &&
			                     !strncmp((char *)inptr, end_central_sig, 4)) 
			                {
			                    incnt -= inptr - inbuf;
								::delete pFile;
								delete inbuf;
								return typePKZIP;
			                }               /* ...otherwise search next block */
			            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
			                                                           boundary */
			        }
			        else 
			        {
			            cur_zipfile_bufstart = ziplen - tail_len;
			        }

			    /*-----------------------------------------------------------------------
			        Loop through blocks of zipfile data, starting at the end and going
			        toward the beginning.  Need only check last 65557 bytes of zipfile:
			        comment may be up to 65535 bytes long, end-of-central-directory rec-
			        ord is 18 bytes (shouldn't hardcode this number, but what the hell:
			        already did so above (22=18+4)), and sig itself is 4 bytes.
			      -----------------------------------------------------------------------*/

			        numblks = (int)
			                  ((MIN(ziplen,65557L) - tail_len + (INBUFSIZ-1)) / INBUFSIZ);
			        /*          =amount to search=   ==done==   ==rounding==    =blksiz= */

			        for (i = 1;  i <= numblks;  ++i) 
			        {
			            cur_zipfile_bufstart -= INBUFSIZ;
			            pFile->seekg(cur_zipfile_bufstart, std::ios::beg );
			            if ((incnt = (pFile->read((char *)inbuf,INBUFSIZ).gcount())) != INBUFSIZ)
			                break;          /* fall through and fail */

			            for (inptr = inbuf+INBUFSIZ-1;  inptr >= inbuf;  --inptr)
			                if ((ascii_to_native(*inptr) == 'P')  &&
			                     !strncmp((char *)inptr, end_central_sig, 4)) 
			                {
			                    incnt -= inptr - inbuf;
								::delete pFile;
								delete inbuf;
								return typePKZIP;
			                }
			            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
			                                                           boundary */
			        }

			    } /* end if (ziplen > INBUFSIZ) */

			/*---------------------------------------------------------------------------
			    Searched through whole region where signature should be without finding
			    it.  Print informational message and die a horrible death.
			  ---------------------------------------------------------------------------*/

			fail:
					delete inbuf;

			}
		}
	 }
	
 	::delete pFile;
	return typeUnknown; // not a zip file
}



bool ChUnzip::UnzipFile( const char* pstrFile, chuint32 flOptions /* = UNZIP_DEF_OPTION */,
						 const char **ppstrFile/* = 0 */, const char* pstrDstDir )
{
	int iRet = -1;

	if ( InitUnzip( flOptions, pstrFile, ppstrFile, pstrDstDir, 0 ) )
	{
		if ( m_flOptions & UNZIP_OPT_GZIP_FILE )
		{
			iRet = UncompressGZFile();	
		}	
		else
		{
			iRet = process_zipfile(); 
		}

		CleanupUnzip();
	}

	return ( iRet == 0 );
}


bool ChUnzip::UnzipFileTo( const char* pstrFile, chuint32 flOptions /* = UNZIP_DEF_OPTION */,
						  const char* pstrDstDir, const char* pstrNewName )
{
	int iRet = -1;

	if ( InitUnzip( flOptions, pstrFile, 0, pstrDstDir, pstrNewName ) )
	{
		if ( m_flOptions & UNZIP_OPT_GZIP_FILE )
		{  // do gzip

			iRet = UncompressGZFile( );
		}
		else
		{		
			iRet = process_zipfile(); 
		}
		CleanupUnzip();
	}

	return ( iRet == 0 );
}


bool ChUnzip::InitUnzip( chuint32 flOptions, const char * pstrZipFile, 
				const char* *ppstrFiles, const char* pstrDstDir, const char * pstrNewName )
	
	//int ncflag, int ntflag, int nvflag, int nUflag, 
    //   int nzflag, int ndflag, int noflag, int naflag, int argc,
   //    LPSTR lpszZipFN, PSTR *FNV)
{
    /* clear all global flags -- need to or not. */
	m_flOptions = flOptions;

	if ( ppstrFiles )
	{
        fnv = ppstrFiles;
		m_flOptions &= ~UNZIP_OPT_PROCESS_ALL_FILES;
	}
	else
	{
    	fnv = &fnames[0];       /* assign default file name vector */
		m_flOptions |= UNZIP_OPT_PROCESS_ALL_FILES;
	}

	#if 0
	jflag = !ndflag; /* WizUnZip perspective is "recreate directory structure" */
    cflag = ncflag ; overwrite_all = noflag;
    tflag = ntflag ; vflag = nvflag; zflag = nzflag; U_flag = nUflag;
    aflag = naflag;
    sflag = 1;
	#endif

	m_flOptions |= UNZIP_OPT_ALLOW_SPACE;	// allow space in file names

    local_hdr_sig[0] = central_hdr_sig[0] = end_central_sig[0] = '\120';
    local_hdr_sig[1] = central_hdr_sig[1] = end_central_sig[1] = '\0';

	cur_zipfile_bufstart = 0L; 

    zipfn = new char [MAX_PATH];
	ASSERT( zipfn );

    lstrcpy(zipfn, pstrZipFile );

	#if defined( CH_MSW )
	struct _stat statbuf;
	#else
	struct stat statbuf;
	#endif

	#if defined( CH_MSW )
    if (_stat(zipfn, &statbuf) || (statbuf.st_mode & S_IFMT) == S_IFDIR)
	#else
    if (stat(zipfn, &statbuf) || (statbuf.st_mode & S_IFMT) == S_IFDIR)
	#endif
	{
		if ( flOptions & UNZIP_OPT_GZIP_FILE )
		{
        	strcat(zipfn, GZSUFX);
		}
		else
		{
        	strcat(zipfn, ZSUFX);
		}
	}
	

	#if defined( CH_MSW )
    if (_stat(zipfn, &statbuf)) 
	#else
    if (stat(zipfn, &statbuf)) 
	#endif
    {  /* try again */
        return FALSE;              /* 9:  file not found */
    } 
    else
	{
        ziplen = statbuf.st_size;
	}

	if ( pstrDstDir )
	{
		m_flOptions |= UNZIP_OPT_DST_DIR;	// we have a dst dir
	}

	m_pstrDstPath = pstrDstDir;

	if ( pstrNewName )
	{
		m_flOptions |= UNZIP_OPT_DST_RENAME;	// we have a dst dir
	}

	m_pstrNewName = pstrNewName;


	pInfo=info;

	mem_mode = 0;

	extra_bytes = 0;
	extra_field = 0;
	fixed_tl = 0;
	didCRlast = FALSE;
	disk_full  = 0;


/*---------------------------------------------------------------------------
    Okey dokey, we have everything we need to get started.  Let's roll.
  ---------------------------------------------------------------------------*/

	slide = new uch[WSIZE];
	ASSERT( slide );


	inbuf	= new uch[ INBUFSIZ+4 ];   /* 4 extra: hold[] */
	ASSERT( inbuf );

	filename = new char[MAX_PATH];
	ASSERT( filename );

    if ((inbuf == NULL) || (slide == NULL ) ||
    		 (zipfn == NULL) || (filename == NULL))
        return false;

    hold = &inbuf[INBUFSIZ];   /* to check for boundary-spanning signatures */

    return true;    /* set up was OK */
}

void ChUnzip::CleanupUnzip(void)
{
    if (inbuf) 
    {
		delete []inbuf;
        inbuf = NULL;
    }


    if (zipfn) 
    {
		delete []zipfn;
        zipfn = NULL;
    }

    if (filename) 
    {
		delete [] filename;
        filename = NULL;
    }
  
    if (slide) 
    {
		delete [] slide;
        slide = NULL;
    }

	m_flOptions = 0;

	if ( m_pOutFile )
	{
		::delete m_pOutFile;
		m_pOutFile = 0;
	}
	
	if ( m_pFile )
	{
		::delete m_pFile;
		m_pFile = 0;
	}

}




int  ChUnzip::UncompressGZFile( )
{

	incnt = 0;		// number of bytes read so far

    if (open_input_file( ))      /* this should never happen, given the */
        return 9;               /*   stat() test in main(), but... */





    chuint8 method = 0;
    chuint8 flags = 0;
    chuint8 xflags = 0;
    chuint8 time[4];
    chuint8 osCode;
	char tmpBuf[10];


	if ( readbuf( tmpBuf, 2) <=0 )
		return 51;

    if ( (chuint8)tmpBuf[0] != GZ_MAGIC_1  || (chuint8)tmpBuf[1] != GZ_MAGIC_2) 
    {
        return 51;
    }

 	if ( readbuf( tmpBuf, 8) <=0 )
		return 51;

	method = tmpBuf[0];
	flags  = tmpBuf[1];
	ChMemCopy( time, &tmpBuf[2], 4 );
	xflags = tmpBuf[6];
	osCode = tmpBuf[7];


    if (method != DEFLATED || (flags & RESERVED) != 0) 
    {
        return 51;
    }

	if ( flags & ASCII_FLAG )
	{
     	pInfo->textmode = TRUE;   /* bit field */
	}
	else
	{
     	pInfo->textmode = FALSE;   /* bit field */
	}

    if ((flags & GZ_EXTRA_FIELD) != 0) 
    { /* skip the extra field */
        long len;
	 	if ( readbuf(tmpBuf, 2) <=0 )
			return 51;

        len = tmpBuf[0] + ((long)tmpBuf[1]<<8);

        int error = do_string( len, SKIP);
        if (error) 
        {
            if (error > 1)  /* fatal */
                return error;
        }
    }

	ChString strNewName;
    if ((flags & ORIG_NAME) != 0) 
    { /* skip the original file name */
		while( readbuf(tmpBuf, 1 ) == 1 && tmpBuf[0] != 0 )
		{
			if ( !(m_flOptions & UNZIP_OPT_DST_RENAME) )	// we have a dst dir
			{
				strNewName += tmpBuf[0];
			}
		}
		if ( !(m_flOptions & UNZIP_OPT_DST_RENAME) )	// we have a dst dir
		{
			m_pstrNewName = strNewName;
		}

    }

    if ((flags & COMMENT) != 0) 
    {   /* skip the .gz file comment */
		while( readbuf( tmpBuf, 1 ) == 1 && tmpBuf[0] != 0 );
    }
    if ((flags & HEAD_CRC) != 0) 
    {  /* skip the header crc */
	 	if ( readbuf( tmpBuf, 2) <=0 )
			return 51;
    }	


	ChString strDst;

	if ( m_pstrDstPath )
	{
		strDst = m_pstrDstPath;
	}

	if ( strDst.GetLength() && strDst[strDst.GetLength() -1 ] != PATH_SEPARATOR )
	{
		strDst +=  PATH_SEPARATOR;
	}

	if ( m_pstrNewName  )
	{
		if ( *m_pstrNewName == PATH_SEPARATOR )
		{
			m_pstrNewName++;	
		}
		strDst = m_pstrNewName;
		
	}
	lstrcpy( filename, strDst );

	if ( open_outfile() )        /* return non-0 if creat failed */
	{ // cleanup and return;
		return -1;	
	}

    crc_32_tab = new ulg[256];
	if ( !crc_32_tab )	 
        return PK_MEM2;

    makecrc( crc_32_tab );


    bits_left = 0;     /* unreduce and unshrink only */
    bitbuf = 0L;       /* unreduce and unshrink only */
    zipeof = 0;        /* unreduce and unshrink only */
    newfile = true;		    
    crc32val = 0xFFFFFFFFL;

	csize = ziplen -  ( inptr - inbuf);


	// uncompress the block
	inflate();	

    delete []crc_32_tab;

	return 0;
}												




int ChUnzip::process_zipfile()    /* return PK-type error code */
{
    int 	error=0, error_in_archive;
    chint32 real_ecrec_offset, expect_ecrec_offset;


/*---------------------------------------------------------------------------
    Open the zipfile for reading and in BINARY mode to prevent CR/LF trans-
    lation, which would corrupt the bitstreams.
  ---------------------------------------------------------------------------*/

    if (open_input_file())      /* this should never happen, given the */
        return 9;               /*   stat() test in main(), but... */

/*---------------------------------------------------------------------------
    Reconstruct the various PK signature strings, and find and process the
    end-of-central-directory header.
  ---------------------------------------------------------------------------*/

    strcat(local_hdr_sig, LOCAL_HDR_SIG);
    strcat(central_hdr_sig, CENTRAL_HDR_SIG);
    strcat(end_central_sig, END_CENTRAL_SIG);
/*  strcat(extd_local_sig, EXTD_LOCAL_SIG);  */

    if (find_end_central_dir()) 
    {   /* not found; nothing to do */
        ::delete m_pFile;
		m_pFile = 0;
        //close(zipfd);
        return 2;                   /* 2:  error in zipfile */
    }

    real_ecrec_offset = cur_zipfile_bufstart+(inptr-inbuf);

    if ((error_in_archive = process_end_central_dir()) > 1) 
    {
        ::delete m_pFile;
		m_pFile = 0;
        //close(zipfd);
        return error_in_archive;
    }

    if (m_flOptions & UNZIP_OPT_DISPLAY_COMMENT ) 
    {
        ::delete m_pFile;
        //close(zipfd);
        return 0;
    }

/*---------------------------------------------------------------------------
    Test the end-of-central-directory info for incompatibilities and incon-
    sistencies.
  ---------------------------------------------------------------------------*/

#ifndef PAKFIX
    if (ecrec.number_this_disk == 0) 
    {
#else /* PAKFIX */
    error = ((ecrec.number_this_disk == 1) &&
             (ecrec.num_disk_with_start_central_dir == 1));
    if ((ecrec.number_this_disk == 0) || error) 
    {
        if (error) 
        {
			/*
		            fprintf(stderr,
		     "\n     Warning:  zipfile claims to be disk 2 of a two-part archive;\n\
		     attempting to process anyway.  If no further errors occur, this\n\
		     archive was probably created by PAK v2.51 or earlier.  This bug\n\
		     was reported to NoGate in March 1991 and was supposed to have been\n\
		     fixed by mid-1991; as of mid-1992 it still hadn't been.\n\n");
			*/
            error_in_archive = 1;  /* 1:  warning */
        }
#endif /* ?PAKFIX */
        expect_ecrec_offset = ecrec.offset_start_central_directory +
                              ecrec.size_central_directory;
        if ((extra_bytes = real_ecrec_offset - expect_ecrec_offset) < 0) 
        {	 
			/*
            fprintf(stderr, "\nerror:  missing %ld bytes in zipfile (\
			attempting to process anyway)\n\n", -extra_bytes);
			*/
            error_in_archive = 2;       /* 2:  (weak) error in zipfile */
        } 
        else if (extra_bytes > 0) 
        {
            if ((ecrec.offset_start_central_directory == 0) &&
                (ecrec.size_central_directory != 0))   /* zip 1.5 -go bug */
            {
				/*
                fprintf(stderr, "\nerror:  NULL central directory offset (\
				attempting to process anyway)\n\n");
				*/
                ecrec.offset_start_central_directory = extra_bytes;
                extra_bytes = 0;
                error_in_archive = 2;   /* 2:  (weak) error in zipfile */
            } 
            else 
            {
				/*
                fprintf(stderr, "\nwarning:  extra %ld bytes at beginning or\
 				within zipfile\n          (attempting to process anyway)\n\n", extra_bytes);
 				*/
                error_in_archive = 1;   /* 1:  warning error */
            }
        }

    /*-----------------------------------------------------------------------
        Check for empty zipfile and exit now if so.
      -----------------------------------------------------------------------*/

        if (expect_ecrec_offset == 0L  &&  ecrec.size_central_directory == 0) 
        {
            //fprintf(stderr, "warning:  zipfile is empty\n");
	        ::delete m_pFile;
	        //close(zipfd);
            return (error_in_archive > 1)? error_in_archive : 1;
        }

    /*-----------------------------------------------------------------------
        Compensate for missing or extra bytes, and seek to where the start
        of central directory should be.  If header not found, uncompensate
        and try again (necessary for at least some Atari archives created
        with STZIP, as well as archives created by J.H. Holm's ZIPSPLIT).
      -----------------------------------------------------------------------*/

        LSEEK( ecrec.offset_start_central_directory )
        if ((readbuf(sig, 4) <= 0) || strncmp(sig, central_hdr_sig, 4)) 
        {
            chint32 tmp = extra_bytes;

            extra_bytes = 0;
            LSEEK( ecrec.offset_start_central_directory )
            if ((readbuf(sig, 4) <= 0) || strncmp(sig, central_hdr_sig, 4)) 
            {
				/*
                fprintf(stderr,
            	"error:  start of central directory not found; zipfile corrupt.\n");
                fprintf(stderr, ReportMsg);
				*/
		        ::delete m_pFile;
		        //close(zipfd);
                return 3;           /* 3:  severe error in zipfile */
            }
			/*
            fprintf(stderr, "error:  reported length of central directory is \
			%d bytes too\n        long (Atari STZIP zipfile?  J.H. Holm ZIPSPLIT zipfile?)\
			.\n        Compensating...\n\n", -tmp);
			*/
            error_in_archive = 2;   /* 2:  (weak) error in zipfile */
        }

    /*-----------------------------------------------------------------------
        Seek to the start of the central directory one last time, since we
        have just read the first entry's signature bytes; then list, extract
        or test member files as instructed, and close the zipfile.
      -----------------------------------------------------------------------*/

        LSEEK( ecrec.offset_start_central_directory )
        if ( m_flOptions & UNZIP_OPT_VIEW_DIRECTORY )
		{
			#if 0
            error = list_files();               /* LIST 'EM */
			#endif
		}
        else
		{
            error = extract_or_test_files();    /* EXTRACT OR TEST 'EM */
		}
        if (error > error_in_archive)   /* don't overwrite stronger error */
		{
            error_in_archive = error;   /*  with (for example) a warning */
		}
    } 
    else 
    {
		/*
        fprintf(stderr, "\nerror:  zipfile is part of multi-disk archive \
		(sorry, not supported).\n");
		*/
    /*  fprintf(stderr, "Please report to zip-bugs@cs.ucla.edu\n");  */
        error_in_archive = 11;  /* 11:  no files found */
    }

    ::delete m_pFile;
	m_pFile = 0;
    //close(zipfd);
    return error_in_archive;

} /* end function process_zipfile() */





/************************************/
/*  Function find_end_central_dir() */
/************************************/

int ChUnzip::find_end_central_dir()   /* return 0 if found, 1 otherwise */
{
    int i, numblks;
    chint32 tail_len;



/*---------------------------------------------------------------------------
    Treat case of short zipfile separately.
  ---------------------------------------------------------------------------*/

    if (ziplen <= INBUFSIZ) 
    {
        m_pFile->seekg( 0L, std::ios::beg );
        if ((incnt = (m_pFile->read((char *)inbuf,(unsigned int)ziplen).gcount()) ) ==
             (int)ziplen)

            /* 'P' must be at least 22 bytes from end of zipfile */
            for (inptr = inbuf+(int)ziplen-22;  inptr >= inbuf;  --inptr)
                if ((ascii_to_native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) 
                {
                    incnt -= inptr - inbuf;
                    return 0;   /* found it! */
                }               /* ...otherwise fall through & fail */

/*---------------------------------------------------------------------------
    Zipfile is longer than INBUFSIZ:  may need to loop.  Start with short
    block at end of zipfile (if not TOO short).
  ---------------------------------------------------------------------------*/

    } 
    else 
    {
        if ((tail_len = ziplen % INBUFSIZ) > ECREC_SIZE) 
        {
            cur_zipfile_bufstart = m_pFile->seekg( ziplen-tail_len, std::ios::beg ).tellg();

            if ((incnt = (m_pFile->read((char *)inbuf,(unsigned int)tail_len).gcount())) !=
                 (int)tail_len)
                goto fail;      /* shut up; it's expedient. */

            /* 'P' must be at least 22 bytes from end of zipfile */
            for (inptr = inbuf+(int)tail_len-22;  inptr >= inbuf;  --inptr)
                if ((ascii_to_native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) 
                {
                    incnt -= inptr - inbuf;
                    return 0;   /* found it */
                }               /* ...otherwise search next block */
            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
                                                           boundary */
        }
        else 
        {
            cur_zipfile_bufstart = ziplen - tail_len;
        }

    /*-----------------------------------------------------------------------
        Loop through blocks of zipfile data, starting at the end and going
        toward the beginning.  Need only check last 65557 bytes of zipfile:
        comment may be up to 65535 bytes long, end-of-central-directory rec-
        ord is 18 bytes (shouldn't hardcode this number, but what the hell:
        already did so above (22=18+4)), and sig itself is 4 bytes.
      -----------------------------------------------------------------------*/

        numblks = (int)
                  ((MIN(ziplen,65557L) - tail_len + (INBUFSIZ-1)) / INBUFSIZ);
        /*          =amount to search=   ==done==   ==rounding==    =blksiz= */

        for (i = 1;  i <= numblks;  ++i) 
        {
            cur_zipfile_bufstart -= INBUFSIZ;
            m_pFile->seekg(cur_zipfile_bufstart, std::ios::beg );
            if ((incnt = (m_pFile->read((char *)inbuf,INBUFSIZ).gcount())) != INBUFSIZ)
                break;          /* fall through and fail */

            for (inptr = inbuf+INBUFSIZ-1;  inptr >= inbuf;  --inptr)
                if ((ascii_to_native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) 
                {
                    incnt -= inptr - inbuf;
                    return 0;   /* found it */
                }
            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
                                                           boundary */
        }

    } /* end if (ziplen > INBUFSIZ) */

/*---------------------------------------------------------------------------
    Searched through whole region where signature should be without finding
    it.  Print informational message and die a horrible death.
  ---------------------------------------------------------------------------*/

fail:

	 /*
    fprintf(stderr, "\nFile:  %s\n\n\
     End-of-central-directory signature not found.  Either this file is not\n\
     a zipfile, or it constitutes one disk of a multi-part archive.  In the\n\
     latter case the central directory and zipfile comment will be found on\n\
     the last disk(s) of this archive.\n", zipfn);
	*/

    return 1;

} /* end function find_end_central_dir() */





/***************************************/
/*  Function process_end_central_dir() */
/***************************************/

int ChUnzip::process_end_central_dir()    /* return PK-type error code */
{
    ec_byte_rec byterec;
    int error=0;


/*---------------------------------------------------------------------------
    Read the end-of-central-directory record and do any necessary machine-
    type conversions (byte ordering, structure padding compensation) by
    reading data into character array, then copying to struct.
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, ECREC_SIZE+4) <= 0)
        return 51;

    ecrec.number_this_disk =
        makeword(&byterec[NUMBER_THIS_DISK]);
    ecrec.num_disk_with_start_central_dir =
        makeword(&byterec[NUM_DISK_WITH_START_CENTRAL_DIR]);
    ecrec.num_entries_centrl_dir_ths_disk =
        makeword(&byterec[NUM_ENTRIES_CENTRL_DIR_THS_DISK]);
    ecrec.total_entries_central_dir =
        makeword(&byterec[TOTAL_ENTRIES_CENTRAL_DIR]);
    ecrec.size_central_directory =
        makelong(&byterec[SIZE_CENTRAL_DIRECTORY]);
    ecrec.offset_start_central_directory =
        makelong(&byterec[OFFSET_START_CENTRAL_DIRECTORY]);
    ecrec.zipfile_comment_length =
        makeword(&byterec[ZIPFILE_COMMENT_LENGTH]);

/*---------------------------------------------------------------------------
    Get the zipfile comment, if any, and print it out.  (Comment may be up
    to 64KB long.  May the fleas of a thousand camels infest the armpits of
    anyone who actually takes advantage of this fact.)  Then position the
    file pointer to the beginning of the central directory and fill buffer.
  ---------------------------------------------------------------------------*/

    LONGINT cchComment = ecrec.zipfile_comment_length; /* save for comment button */
    if (ecrec.zipfile_comment_length && m_flOptions & UNZIP_OPT_DISPLAY_COMMENT ) 
    {
        if (do_string(ecrec.zipfile_comment_length,DISPLAY)) 
        {
            //fprintf(stderr, "\ncaution:  zipfile comment truncated\n");
            error = 1;          /* 1:  warning error */
        }
    }

    return error;

} /* end function process_end_central_dir() */




#if 0
/* also referenced in UpdateListBox() in updatelb.c (Windows version) */
char *Headers[][2] = {
    {" Length    Date    Time    Name",
     " ------    ----    ----    ----"},
    {" Length  Method   Size  Ratio   Date    Time   CRC-32     Name",
     " ------  ------   ----  -----   ----    ----   ------     ----"}
};

/*************************/
/* Function list_files() */
/*************************/

int ChUnzip::list_files()    /* return PK-type error code */
{
    char **fnamev;
    int do_this_file=FALSE, ratio, error, error_in_archive=0;
    int which_hdr=(vflag>1), date_format;
    UWORD j, yr, mo, dy, hh, mm, members=0;
    ULONG tot_csize=0L, tot_ucsize=0L;
#ifdef OS2
    ULONG tot_easize=0L, tot_eafiles=0L, ea_size;
#endif
#ifdef MSWIN
    PSTR psLBEntry;  /* list box entry */
#endif
    min_info info;
    static char *method[NUM_METHODS+1] =
        {"Stored", "Shrunk", "Reduce1", "Reduce2", "Reduce3", "Reduce4",
         "Implode", "Token", "Deflate", unkn};



/*---------------------------------------------------------------------------
    Unlike extract_or_test_files(), this routine confines itself to the cen-
    tral directory.  Thus its structure is somewhat simpler, since we can do
    just a single loop through the entire directory, listing files as we go.

    So to start off, print the heading line and then begin main loop through
    the central directory.  The results will look vaguely like the following:

  Length  Method   Size  Ratio   Date    Time   CRC-32     Name ("^" ==> case
  ------  ------   ----  -----   ----    ----   ------     ----   conversion)
   44004  Implode  13041  71%  11-02-89  19:34  8b4207f7   Makefile.UNIX
    3438  Shrunk    2209  36%  09-15-90  14:07  a2394fd8  ^dos-file.ext
  ---------------------------------------------------------------------------*/

    pInfo = &info;
    date_format = dateformat();


    for (j = 0; j < ecrec.total_entries_central_dir; ++j) 
    {

        if (readbuf(sig, 4) <= 0)
            return 51;          /* 51:  unexpected EOF */
        if (strncmp(sig, central_hdr_sig, 4)) 
        {  /* just to make sure */
            //fprintf(stderr, CentSigMsg, j);  /* sig not found */
            //fprintf(stderr, ReportMsg);   /* check binary transfers */
            return 3;           /* 3:  error in zipfile */
        }
        if ((error = process_cdir_file_hdr()) != 0)  /* (sets pInfo->lcflag) */
            return error;       /* only 51 (EOF) defined */

        /*
         * We could DISPLAY the filename instead of storing (and possibly trun-
         * cating, in the case of a very long name) and printing it, but that
         * has the disadvantage of not allowing case conversion--and it's nice
         * to be able to see in the listing precisely how you have to type each
         * filename in order for unzip to consider it a match.  Speaking of
         * which, if member names were specified on the command line, check in
         * with match() to see if the current file is one of them, and make a
         * note of it if it is.
         */

        if ((error = do_string(crec.filename_length, ZFILENAME)) != 0) 
        {
            error_in_archive = error;  /*             ^--(uses pInfo->lcflag) */
            if (error > 1)      /* fatal:  can't continue */
                return error;
        }
        if (extra_field != (byte *)NULL)
            delete extra_field;

        extra_field = (byte *)NULL;
        if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0) 
        {
            error_in_archive = error;  
            if (error > 1)      /* fatal:  can't continue */
                return error;
        }
        if (!process_all_files) 
        {   /* check if specified on command line */
            do_this_file = FALSE;
            fnamev = fnv;       /* don't destroy permanent filename ptr */
            for (--fnamev; *++fnamev;)
                if (match(filename, *fnamev)) 
                {
                    do_this_file = TRUE;
                    break;      /* found match, so stop looping */
                }
        }
        /*
         * If current file was specified on command line, or if no names were
         * specified, do the listing for this file.  Otherwise, get rid of the
         * file comment and go back for the next file.
         */

        if (process_all_files || do_this_file) 
        {

            yr = (((crec.last_mod_file_date >> 9) & 0x7f) + 80) % (unsigned)100;
            mo = (crec.last_mod_file_date >> 5) & 0x0f;
            dy = crec.last_mod_file_date & 0x1f;

            /* twist date so it displays according to national convention */
            switch (date_format) 
            {
                case DF_YMD:
                    hh = mo; mo = yr; yr = dy; dy = hh; break;
                case DF_DMY:
                    hh = mo; mo = dy; dy = hh;
            }
            hh = (crec.last_mod_file_time >> 11) & 0x1f;
            mm = (crec.last_mod_file_time >> 5) & 0x3f;

            csize = (LONGINT) crec.csize;
            ucsize = (LONGINT) crec.ucsize;
            if (crec.general_purpose_bit_flag & 1)
                csize -= 12;    /* if encrypted, don't count encrypt hdr */

            ratio = (ucsize == 0) ? 0 :   /* .zip can have 0-length members */
                ((ucsize > 2000000L) ?    /* risk signed overflow if mult. */
                (int) ((ucsize-csize) / (ucsize/1000L)) + 5 :   /* big */
                (int) ((1000L*(ucsize-csize)) / ucsize) + 5);   /* small */


#if 0
#ifdef MSWIN
		{
		DWORD dwAddStringOutcome = 0L;	/* outcome from adding string: index or error code */
		static char __based(__segname("STRINGS_TEXT")) szNoLBSpace[] =
            "This archive contains too many files for WizUnZip to list them all!";
		static char __based(__segname("STRINGS_TEXT")) szLBError[] =
            "A Windows error occurred while filling the listbox!";


#ifdef NEED_EARLY_REDRAW
            /* turn on listbox redrawing just before adding last line */
            if (j == (ecrec.total_entries_central_dir-1))
                (void)SendMessage(hWndList, WM_SETREDRAW, TRUE, 0L);
#endif /* NEED_EARLY_REDRAW */
            psLBEntry =
              (PSTR)LocalAlloc(LMEM_FIXED, FILNAMSIZ+LONG_FORM_FNAME_INX);
            switch (which_hdr) {
                case 0:   /* short form */
                    OemToAnsi(filename, filename);  /* translate to ANSI */
                    wsprintf(psLBEntry, "%7ld  %02u-%02u-%02u  %02u:%02u  %c%s",
                      (long)ucsize, mo, dy, yr, hh, mm, (pInfo->lcflag?'^':' '),
                      (LPSTR)filename);
                    dwAddStringOutcome = SendMessage(hWndList, LB_ADDSTRING, 0,
                      (LONG)(LPSTR)psLBEntry);
                    break;
                case 1:   /* verbose */
                    OemToAnsi(filename, filename);  /* translate to ANSI */
                    wsprintf(psLBEntry, 
                 "%7ld  %-7s%7ld %3d%%  %02u-%02u-%02u  %02u:%02u  %08lx  %c%s",
                      (long)ucsize, (LPSTR)method[methnum], (long)csize,
                      ratio/10, mo, dy, yr, hh, mm, (unsigned long)crec.crc32,
                      (pInfo->lcflag?'^':' '), (LPSTR)filename);
                    dwAddStringOutcome = SendMessage(hWndList, LB_ADDSTRING, 0,
                      (LONG)(LPSTR)psLBEntry);
            }
            LocalFree((HANDLE)psLBEntry);
			if (dwAddStringOutcome == LB_ERR)
			{
				MessageBox(hWndMain, szLBError, NULL, MB_OK | MB_ICONEXCLAMATION);
				return 4; /* probably memory problem. Ignored in WizUnZip  */
			}
			else if (dwAddStringOutcome == LB_ERRSPACE)
			{
				MessageBox(hWndMain, szNoLBSpace, NULL, MB_OK | MB_ICONEXCLAMATION);
				return 4; /* Hit WIN 64K limit. Ignored in WizUnZip version */
			}
		}
#endif /* ?MSWIN */

#endif

            error = do_string(crec.file_comment_length, (QCOND? DISPLAY:SKIP));
            if (error) 
            {
                error_in_archive = error;  /* might be just warning */
                if (error > 1)  /* fatal */
                    return error;
            }
            tot_ucsize += (ULONG) ucsize;
            tot_csize += (ULONG) csize;
            ++members;
#ifdef OS2
            if ((ea_size = SizeOfEAs(extra_field)) != 0) 
            {
                tot_easize += ea_size;
                tot_eafiles++;
            }
#endif
        } 
        else 
        {        /* not listing this file */
            SKIP_(crec.file_comment_length)
        }
    }                   /* end for-loop (j: files in central directory) */

/*---------------------------------------------------------------------------
    Print footer line and totals (compressed size, uncompressed size, number
    of members in zipfile).
  ---------------------------------------------------------------------------*/

    ratio = (tot_ucsize == 0) ? 
        0 : ((tot_ucsize > 4000000L) ?   /* risk unsigned overflow if mult. */
        (int) ((tot_ucsize - tot_csize) / (tot_ucsize/1000L)) + 5 :
        (int) ((tot_ucsize - tot_csize) * 1000L / tot_ucsize) + 5);

    if ( !(m_flOption & UNZIP_OPT_QUIET ))
    {
	#if 0
#ifdef MSWIN
        /* Display just the totals since the dashed lines get displayed
         * in UpdateListBox(). Get just enough space to display total.
         */
        switch (which_hdr) {
            case 0:         /* short */
                wsprintf(lpumb->szTotalsLine, "%7lu                    %-7u", 
                  (ULONG)tot_ucsize, members);
                break;
            case 1:         /* verbose */
                wsprintf(lpumb->szTotalsLine, 
                  "%7lu         %7lu %3d%%                              %-7u", 
                  (ULONG)tot_ucsize, (ULONG)tot_csize, ratio / 10, members);
                break;
        }
#endif /* ?MSWIN */
#endif

#ifdef OS2
        if (tot_eafiles && tot_easize)
            printf("\n%ld file%s %ld bytes of EA's attached.\n", tot_eafiles, 
              tot_eafiles == 1 ? " has" : "s have a total of", tot_easize);
#endif
    }
/*---------------------------------------------------------------------------
    Double check that we're back at the end-of-central-directory record.
  ---------------------------------------------------------------------------*/

    readbuf(sig, 4);
    if (strncmp(sig, end_central_sig, 4)) 
    {     /* just to make sure again */
        //fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
/*      fprintf(stderr, ReportMsg);   */
        error_in_archive = 1;        /* 1:  warning error */
    }
    return error_in_archive;

} /* end function list_files() */

#endif // 0





/**************************************/
/*  Function process_cdir_file_hdr()  */
/**************************************/

int ChUnzip::process_cdir_file_hdr()    /* return PK-type error code */
{
    cdir_byte_hdr byterec;


/*---------------------------------------------------------------------------
    Read the next central directory entry and do any necessary machine-type
    conversions (byte ordering, structure padding compensation--do so by
    copying the data from the array into which it was read (byterec) to the
    usable struct (crec)).
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, CREC_SIZE) <= 0)
        return 51;   /* 51:  unexpected EOF */

    crec.version_made_by[0] = byterec[C_VERSION_MADE_BY_0];
    crec.version_made_by[1] = byterec[C_VERSION_MADE_BY_1];
    crec.version_needed_to_extract[0] = byterec[C_VERSION_NEEDED_TO_EXTRACT_0];
    crec.version_needed_to_extract[1] = byterec[C_VERSION_NEEDED_TO_EXTRACT_1];

    crec.general_purpose_bit_flag =
        makeword(&byterec[C_GENERAL_PURPOSE_BIT_FLAG]);
    crec.compression_method =
        makeword(&byterec[C_COMPRESSION_METHOD]);
    crec.last_mod_file_time =
        makeword(&byterec[C_LAST_MOD_FILE_TIME]);
    crec.last_mod_file_date =
        makeword(&byterec[C_LAST_MOD_FILE_DATE]);
    crec.crc32 =
        makelong(&byterec[C_CRC32]);
    crec.csize =
        makelong(&byterec[C_COMPRESSED_SIZE]);
    crec.ucsize =
        makelong(&byterec[C_UNCOMPRESSED_SIZE]);
    crec.filename_length =
        makeword(&byterec[C_FILENAME_LENGTH]);
    crec.extra_field_length =
        makeword(&byterec[C_EXTRA_FIELD_LENGTH]);
    crec.file_comment_length =
        makeword(&byterec[C_FILE_COMMENT_LENGTH]);
    crec.disk_number_start =
        makeword(&byterec[C_DISK_NUMBER_START]);
    crec.internal_file_attributes =
        makeword(&byterec[C_INTERNAL_FILE_ATTRIBUTES]);
    crec.external_file_attributes =
        makelong(&byterec[C_EXTERNAL_FILE_ATTRIBUTES]);  /* LONG, not word! */
    crec.relative_offset_local_header =
        makelong(&byterec[C_RELATIVE_OFFSET_LOCAL_HEADER]);

    pInfo->hostnum = MIN(crec.version_made_by[1], NUM_HOSTS);
/*  extnum = MIN(crec.version_needed_to_extract[1], NUM_HOSTS); */
    LONGINT methnum = MIN(crec.compression_method, NUM_METHODS);
    
    //if (methnum == NUM_METHODS)
    //    wsprintf(unkn, "Unk:%03d", crec.compression_method);

/*---------------------------------------------------------------------------
    Set flag for lowercase conversion of filename, depending on which OS the
    file is coming from.  This section could be ifdef'd if some people have
    come to love DOS uppercase filenames under Unix...but really, guys, get
    a life. :)  NOTE THAT ALL SYSTEM NAMES NOW HAVE TRAILING UNDERSCORES!!!
    This is to prevent interference with compiler command-line defines such
    as -DUNIX, for example, which are then used in "#ifdef UNIX" constructs.
  ---------------------------------------------------------------------------*/

    pInfo->lcflag = 0;
    //if (!U_flag)   /* as long as user hasn't specified case-preservation */
	if ( m_flOptions & UNZIP_OPT_PRESERVE_FILENAMES )
        switch (pInfo->hostnum) {
         //   case DOS_OS2_FAT_:
        /*  case VMS_:              VMS Zip converts filenames to lowercase */
            case VM_CMS_:           /* all caps? */
            case CPM_:              /* like DOS, right? */
                pInfo->lcflag = 1;  /* convert filename to lowercase */
                break;

            default:                /* AMIGA_, UNIX_, (ATARI_), OS2_HPFS_, */
                break;              /*   MAC_, (Z_SYSTEM_):  no conversion */
        }

    /* do Amigas (AMIGA_) also have volume labels? */
    if (IS_VOLID(crec.external_file_attributes) &&
        (pInfo->hostnum == FS_FAT_ || pInfo->hostnum == FS_HPFS_ ||
         pInfo->hostnum == FS_NTFS_ || pInfo->hostnum == ATARI_))
    {
        pInfo->vollabel = TRUE;
        pInfo->lcflag = 0;        /* preserve case of volume labels */
    } else
        pInfo->vollabel = FALSE;


    return 0;

} /* end function process_cdir_file_hdr() */





/***************************************/
/*  Function process_local_file_hdr()  */
/***************************************/

int ChUnzip::process_local_file_hdr()    /* return PK-type error code */
{
    local_byte_hdr byterec;


/*---------------------------------------------------------------------------
    Read the next local file header and do any necessary machine-type con-
    versions (byte ordering, structure padding compensation--do so by copy-
    ing the data from the array into which it was read (byterec) to the
    usable struct (lrec)).
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, LREC_SIZE) <= 0)
        return 51;   /* 51:  unexpected EOF */

    lrec.version_needed_to_extract[0] = byterec[L_VERSION_NEEDED_TO_EXTRACT_0];
    lrec.version_needed_to_extract[1] = byterec[L_VERSION_NEEDED_TO_EXTRACT_1];

    lrec.general_purpose_bit_flag = makeword(&byterec[L_GENERAL_PURPOSE_BIT_FLAG]);
    lrec.compression_method = makeword(&byterec[L_COMPRESSION_METHOD]);
    lrec.last_mod_file_time = makeword(&byterec[L_LAST_MOD_FILE_TIME]);
    lrec.last_mod_file_date = makeword(&byterec[L_LAST_MOD_FILE_DATE]);
    lrec.crc32 = makelong(&byterec[L_CRC32]);
    lrec.csize = makelong(&byterec[L_COMPRESSED_SIZE]);
    lrec.ucsize = makelong(&byterec[L_UNCOMPRESSED_SIZE]);
    lrec.filename_length = makeword(&byterec[L_FILENAME_LENGTH]);
    lrec.extra_field_length = makeword(&byterec[L_EXTRA_FIELD_LENGTH]);

    csize = (LONGINT) lrec.csize;
    ucsize = (LONGINT) lrec.ucsize;

    if ((lrec.general_purpose_bit_flag & 8) != 0) 
    {
        /* Can't trust local header, use central directory: */
      lrec.crc32 = pInfo->crc;
        lrec.csize = pInfo->compr_size;
        csize = (LONGINT) lrec.csize;
    }

    return 0;                 /* 0:  no error */

} /* end function process_local_file_hdr() */

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:21  uecasm
// Import of source tree as at version 2.53 release.
//
