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

	Chaco JPEG decoder

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"

#ifdef CH_UNIX
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define _STREAM_COMPAT 1
#endif

#include <string.h>
#include <iostream>
#include <fstream>

#include <ChTypes.h>

#ifdef CH_UNIX
#include <ChDC.h>
#endif

#include <ChExcept.h>

#include <ChJpegDecoder.h>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif							    


#if defined( CH_MSW )

	#define IOS_BINARY		std::ios::binary

#elif defined( CH_UNIX )

	#define IOS_BINARY		std::ios::bin

#endif	// defined( CH_UNIX )


  
/////////////////////////////////////////////////////////////////////////////////

ChJPEG::ChJPEG( ChImageConsumer* pConsumer ) : ChImageDecoder( pConsumer )
{
}

ChJPEG::~ChJPEG() 
{
	
}
bool ChJPEG::Load(WORD wResid, HINSTANCE hInst)
{
	ASSERT( false );
	return false;
}
bool ChJPEG::Load(LZHANDLE lzHdl)
{
	ASSERT( false );
	return false;
}

bool ChJPEG::Load( const char* pszFileName, chuint32 flOptions /* = loadAuto */ )// Load JPEG from disk file
{
	m_pFile = 0;

	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	TRY
	#else
	try
	#endif
	{
		// load the jpeg file
		LoadJPEG ( pszFileName, flOptions ); 
		return true;
	}
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	CATCH( ChJPEGEx, jpegEx )
	#else
	catch( ChJPEGEx jpegEx )
	#endif
	{
		int iCause;
		#if defined( CH_EXCEPTIONS )
			#if defined( CH_MSW ) && defined( CH_ARCH_16 )
			iCause = jpegEx->GetCause();
			#else
			iCause = jpegEx.GetCause();
			#endif
		#endif
		TRACE1( "JPEG load failed : Cause : %d\n", iCause );  

		::delete m_pFile;
		m_pFile = 0;


	}
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	END_CATCH
	#endif 

	return false;

}

void  ChJPEG::LoadJPEG( const char* pszFileName, chuint32 flOptions )
{

	m_boolFirst = true;
	m_iCurrScanLine = 0;
	m_flOptions = flOptions;

//	m_pFile = ::new fstream( pszFileName, ios::in | IOS_BINARY | ios::nocreate, filebuf::sh_read   );
	m_pFile = ::new std::fstream( pszFileName, std::ios::in | std::ios::binary );
	ASSERT( m_pFile );

	if ( !m_pFile->is_open() )
	{
		#if defined( CH_EXCEPTIONS )
		{
			#if defined( CH_MSW) && defined( CH_ARCH_16 )  
			{  
							
				THROW( new ChJPEGEx( ChJPEGEx::inputFileOpenError ));
			}
			#else
			throw ChJPEGEx( ChJPEGEx::inputFileOpenError );    
			#endif
		}
		#else
		{
			ASSERT( false );
			return;
		}
		#endif
	}

	  jpeg_error_mgr* jerr = new jpeg_error_mgr;
	  /* More stuff */
	  JSAMPARRAY buffer;		/* Output row buffer */
	  int row_stride;		/* physical row width in output buffer */


	  /* Step 1: allocate and initialize JPEG decompression object */

	  /* We set up the normal JPEG error routines, then override error_exit. */
	  cinfo.m_common.err = jpeg_std_error( jerr );
	  //jerr.pub.error_exit = my_error_exit;
	  /* Establish the setjmp return context for my_error_exit to use. */
	  //if (setjmp(jerr.setjmp_buffer)) {
	    /* If we get here, the JPEG code has signaled an error.
	     * We need to clean up the JPEG object, close the input file, and return.
	     */
	    //jpeg_destroy_decompress(&cinfo);
	    //fclose(infile);
	    //return 0;
	  //}

	  /* Now we can initialize the JPEG decompression object. */
	  jpeg_create_decompress(&cinfo);

	  /* Step 2: specify data source (eg, a file) */

	  jpeg_stdio_src(&cinfo, m_pFile );

	  /* Step 3: read file parameters with jpeg_read_header() */

	  (void) jpeg_read_header(&cinfo, true );
	  /* We can ignore the return value from jpeg_read_header since
	   *   (a) suspension is not possible with the stdio data source, and
	   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	   * See libjpeg.doc for more info.
	   */

	  /* Step 4: set parameters for decompression */

	  /* In this example, we don't need to change any of the defaults set by
	   * jpeg_read_header(), so we do nothing here.
	   */

	  /* Step 5: Start decompressor */

	  jpeg_start_decompress(&cinfo);

	  /* We may need to do some setup of our own at this point before reading
	   * the data.  After jpeg_start_decompress() we have the correct scaled
	   * output image dimensions available, as well as the output colormap
	   * if we asked for color quantization.
	   * In this example, we need to make an output work buffer of the right size.
	   */ 
	  /* JSAMPLEs per row in output buffer */
	  row_stride = cinfo.output_width * cinfo.output_components;
	  /* Make a one-row-high sample array that will go away when done with image */
	  buffer = cinfo.m_common.mem->alloc_sarray
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	  /* Step 6: while (scan lines remain to be read) */
	  /*           jpeg_read_scanlines(...); */

	  /* Here we use the library's state variable cinfo.output_scanline as the
	   * loop counter, so that we don't have to keep track ourselves.
	   */
	  while (cinfo.output_scanline < cinfo.output_height) {
	    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	    /* Assume put_scanline_someplace wants a pointer and sample count. */
	    put_scanline_someplace( &cinfo, buffer[0], row_stride);
	  }

	  /* Step 7: Finish decompression */

	  (void) jpeg_finish_decompress(&cinfo);
	  /* We can ignore the return value since suspension is not possible
	   * with the stdio data source.
	   */

	  /* Step 8: Release JPEG decompression object */

	  /* This is an important step since it will release a good deal of memory. */
	  jpeg_destroy_decompress(&cinfo);

	  /* After finish_decompress, we can close the input file.
	   * Here we postpone it until after no more JPEG errors are possible,
	   * so as to simplify the setjmp error logic above.  (Actually, I don't
	   * think that jpeg_destroy can do an error exit, but why assume anything...)
	   */
		// close file
		::delete m_pFile;
		m_pFile = 0;


	  /* At this point you may want to check to see whether any corrupt-data
	   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	   */

	  /* And we're done! */
	  return;
}

// $Log$
