/*
 * jerror.c
 *
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains simple error-reporting and trace-message routines.
 * These are suitable for Unix-like systems and others where writing to
 * stderr is the right thing to do.  Many applications will want to replace
 * some or all of these routines.
 *
 * These routines are used by both the compression and decompression code.
 */

// $Header$

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "headers.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "jversion.h"
#include "jerror.h"

#include <ChJpegDecoder.h>
#include <ChExcept.h>

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif


/*
 * Create the message string table.
 * We do this from the master message list in jerror.h by re-reading
 * jerror.h with a suitable definition for macro JMESSAGE.
 * The message table is made an external symbol just in case any applications
 * want to refer to it directly.
 */

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_std_message_table	jMsgTable
#endif

#define JMESSAGE(code,str)	str ,

const char * const jpeg_std_message_table[] = {
//#include "jerror.h"
  NULL
};


/*
 * Error exit handler: must not return to caller.
 *
 * Applications may override this if they want to get control back after
 * an error.  Typically one would longjmp somewhere instead of exiting.
 * The setjmp buffer can be made a private field within an expanded error
 * handler object.  Note that the info needed to generate an error message
 * is stored in the error object, so you can generate the message now or
 * later, at your convenience.
 * You should make sure that the JPEG object is cleaned up (with jpeg_abort
 * or jpeg_destroy) at some point.
 */

void jpeg_error_mgr::error_exit (j_common_ptr cinfo)
{
  /* Always display the message */
  output_message(cinfo);

  /* Let the memory manager delete any temp files before we die */
  ChJPEG::jpeg_destroy(cinfo);
  // throw exception
	#if defined( CH_EXCEPTIONS )
	{
		#if defined( CH_MSW) && defined( CH_ARCH_16 )  
		{  
					
			THROW( new ChJPEGEx( msg_code ));
		}
		#else
		throw ChJPEGEx( msg_code );    
		#endif
	}
	#else
	{
		//herror( "sockinetaddr::sockinetaddr -- Bad host name" );   
		ASSERT( false );
	}
	#endif

}


/*
 * Actual output of an error or trace message.
 * Applications may override this method to send JPEG messages somewhere
 * other than stderr.
 */

void jpeg_error_mgr::output_message(j_common_ptr cinfo)
{
#if 0
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  format_message(cinfo, buffer);

  TRACE( buffer );
  #endif
}


/*
 * Decide whether to emit a trace or warning message.
 * msg_level is one of:
 *   -1: recoverable corrupt-data warning, may want to abort.
 *    0: important advisory messages (always display to user).
 *    1: first level of tracing detail.
 *    2,3,...: successively more detailed tracing messages.
 * An application might override this method if it wanted to abort on warnings
 * or change the policy about which messages to display.
 */

void jpeg_error_mgr::emit_message (j_common_ptr cinfo, int msg_level)
{

  if (msg_level < 0) 
  {
    /* It's a warning message.  Since corrupt files may generate many warnings,
     * the policy implemented here is to show only the first warning,
     * unless trace_level >= 3.
     */
    if (num_warnings == 0 || trace_level >= 3)
      	output_message(cinfo);
    /* Always count warnings in num_warnings. */
    num_warnings++;
  } 
  else 
  {
    /* It's a trace message.  Show it if trace_level >= msg_level. */
    if (trace_level >= msg_level)
      output_message(cinfo);
  }
}


/*
 * Format a message string for the most recent JPEG error or message.
 * The message is stored into buffer, which should be at least JMSG_LENGTH_MAX
 * characters.  Note that no '\n' character is added to the string.
 * Few applications should need to override this method.
 */

void jpeg_error_mgr::format_message (j_common_ptr cinfo, char * buffer)
{
#if 0	
  int msg_code = msg_code;
  const char * msgtext = NULL;
  const char * msgptr;
  char ch;
  boolean isstring;

  /* Look up message string in proper table */
  if (msg_code > 0 && msg_code <= last_jpeg_message) 
  {
    msgtext = jpeg_message_table[msg_code];
  } 
  else if (addon_message_table != NULL &&
	     msg_code >= first_addon_message &&
	     msg_code <= last_addon_message) {
    msgtext = addon_message_table[msg_code - first_addon_message];
  }

  /* Defend against bogus message number */
  if (msgtext == NULL) {
    msg_parm.i[0] = msg_code;
    msgtext = jpeg_message_table[0];
  }

  /* Check for string parameter, as indicated by %s in the message text */
  isstring = FALSE;
  msgptr = msgtext;
  while ((ch = *msgptr++) != '\0') {
    if (ch == '%') {
      if (*msgptr == 's') isstring = TRUE;
      break;
    }
  }

  /* Format the message into the passed buffer */
  if (isstring)
    wsprintf(buffer, msgtext, msg_parm.s);
  else
    wsprintf(buffer, msgtext,
	    msg_parm.i[0], msg_parm.i[1],
	    msg_parm.i[2], msg_parm.i[3],
	    msg_parm.i[4], msg_parm.i[5],
	    msg_parm.i[6], msg_parm.i[7]);
	#endif
}


/*
 * Reset error state variables at start of a new image.
 * This is called during compression startup to reset trace/error
 * processing to default state, without losing any application-specific
 * method pointers.  An application might possibly want to override
 * this method if it has additional error processing state.
 */

void jpeg_error_mgr::reset_error_mgr (j_common_ptr cinfo)
{
  num_warnings = 0;
  /* trace_level is not reset since it is an application-supplied parameter */
  msg_code = 0;	/* may be useful as a flag for "no error" */
}


/*
 * Fill in the standard error-handling methods in a jpeg_error_mgr object.
 * Typical call is:
 *	struct jpeg_compress_struct cinfo;
 *	struct jpeg_error_mgr err;
 *
 *	cinfo.err = jpeg_std_error(&err);
 * after which the application may override some of the methods.
 */

void jpeg_error_mgr::init_error_mgr()
{
  trace_level = 0;		/* default = no tracing */
  num_warnings = 0;	/* no warnings emitted yet */
  msg_code = 0;		/* may be useful as a flag for "no error" */

  /* Initialize message table pointers */
  jpeg_message_table = jpeg_std_message_table;
  last_jpeg_message = (int) JMSG_LASTMSGCODE - 1;

  addon_message_table = NULL;
  first_addon_message = 0;	/* for safety */
  last_addon_message = 0;
}

jpeg_error_mgr *ChJPEG::jpeg_std_error ( jpeg_error_mgr * err)
{
	err->init_error_mgr();

  	return err;
}
