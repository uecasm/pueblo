/*
 * jdatasrc.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

// $Header$

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "headers.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include <fstream>

#include <ChJpegDecoder.h>

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */

/* Expanded data source object for stdio input */

class my_source_mgr	: public jpeg_source_mgr
{
	public :
		my_source_mgr( j_common_ptr cinfo )			 
				{
					buffer = (JOCTET *)
					  		cinfo->mem->alloc_small( cinfo, JPOOL_PERMANENT,
								  		INPUT_BUF_SIZE * SIZEOF(JOCTET));  
					ASSERT( buffer );
				
				}
		virtual ~my_source_mgr() {}

		virtual void init_source( j_decompress_ptr cinfo );
		virtual bool fill_input_buffer( j_decompress_ptr cinfo );
		virtual void skip_input_data( j_decompress_ptr cinfo, long num_bytes );
		virtual bool resync_to_restart( j_decompress_ptr cinfo );
		virtual void term_source( j_decompress_ptr cinfo );
		void SetFile( std::fstream * pFile )	{ m_pInfile = pFile; }

	private :
		std::fstream*		m_pInfile;			// GIF file
		JOCTET * buffer;		/* start of buffer */
	  	bool start_of_file;	/* have we gotten any data yet? */
};

typedef class my_source_mgr * my_src_ptr;



/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
void my_source_mgr::init_source (j_decompress_ptr cinfo)
{
  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

bool my_source_mgr::fill_input_buffer(j_decompress_ptr cinfo)
{
  //my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = JFREAD(m_pInfile, buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) 
  {
    if (start_of_file)	/* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    buffer[0] = (JOCTET) 0xFF;
    buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  SetNextInputByte( buffer );
  SetBytesInBuffer( nbytes );
  start_of_file = false;

  return true;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

void my_source_mgr::skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  //my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) 
  {
    while (num_bytes > (long) GetBytesInBuffer() ) 
    {
      num_bytes -= (long) GetBytesInBuffer();
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    SetNextInputByte( GetNextInputByte() + (size_t) num_bytes );
    SetBytesInBuffer( GetBytesInBuffer() - (size_t) num_bytes );
  }
}

bool my_source_mgr::resync_to_restart(j_decompress_ptr cinfo)
{
	return ChJPEG::jpeg_resync_to_restart( cinfo );
}



/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

void my_source_mgr::term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

void ChJPEG::jpeg_stdio_src (j_decompress_ptr cinfo, std::fstream * infile)
{
  	my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) 
  {	/* first time for this JPEG object? */
  		cinfo->src = new my_source_mgr( (j_common_ptr)cinfo );
		ASSERT( cinfo->src );

		src = (my_src_ptr)cinfo->src;
  }

  src =  (my_src_ptr)cinfo->src;

  src->SetFile( infile );
  src->SetBytesInBuffer( 0 ); /* forces fill_input_buffer on first read */
  src->SetNextInputByte( NULL ); /* until buffer loaded */
}
