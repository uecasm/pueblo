/*
 * jdapi.c
 *
 * Copyright (C) 1994-1995, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains application interface code for the decompression half of
 * the JPEG library.  Most of the routines intended to be called directly by
 * an application are in this file.  But also see jcomapi.c for routines
 * shared by compression and decompression.
 */

// $Header$

#include "headers.h"
//#include "resource.h"

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#include <ChImgUtil.h>
#include <ChJpegDecoder.h>
//#include <ChWnd.h>


jpeg_common_struct::~jpeg_common_struct()	
{
	if ( err )
	{
		delete err;
	}
	if ( mem )
	{
		delete mem;
	}
	if ( progress )
	{
		delete progress;
	}
}



jpeg_decompress_struct::jpeg_decompress_struct()
{
	src = 0;

	/* Basic description of image --- filled in by jpeg_read_header(). */
	/* Application may inspect these values to decide how to process image. */

	image_width = 0;	/* nominal image width (from SOF marker) */
	image_height = 0;	/* nominal image height */
	num_components = 0;		/* # of color components in JPEG image */
	jpeg_color_space = JCS_UNKNOWN; /* colorspace of JPEG image */

	/* Decompression processing parameters --- these fields must be set before
	* calling jpeg_start_decompress().  Note that jpeg_read_header() initializes
	* them to default values.
	*/

	out_color_space = JCS_UNKNOWN; /* colorspace for output */

	scale_num = scale_denom = 0; /* fraction by which to scale image */

	output_gamma = 0.0;		/* image gamma wanted in output */

	raw_data_out = 0;		/* TRUE=downsampled data wanted */

	quantize_colors = 0;	/* TRUE=colormapped output wanted */
	/* the following are ignored if not quantize_colors: */
	two_pass_quantize = 0;	/* TRUE=use two-pass color quantization */
	dither_mode = JDITHER_NONE;	/* type of color dithering to use */
	desired_number_of_colors = 0;	/* max number of colors to use */

	dct_method = JDCT_ISLOW;	/* DCT algorithm selector */
	do_fancy_upsampling = 0;	/* TRUE=apply fancy upsampling */

	/* Description of actual output image that will be returned to application.
	* These fields are computed by jpeg_start_decompress().
	* You can also use jpeg_calc_output_dimensions() to determine these values
	* in advance of calling jpeg_start_decompress().
	*/

	output_width = 0;	/* scaled image width */
	output_height = 0;	/* scaled image height */
	out_color_components = 0;	/* # of color components in out_color_space */
	output_components = 0;	/* # of color components returned */
	/* output_components is 1 (a colormap index) when quantizing colors;
	* otherwise it equals out_color_components.
	*/
	rec_outbuf_height = 0;	/* min recommended height of scanline buffer */
	/* If the buffer passed to jpeg_read_scanlines() is less than this many rows
	* high, space and time will be wasted due to unnecessary data copying.
	* Usually rec_outbuf_height will be 1 or 2, at most 4.
	*/

	/* When quantizing colors, the output colormap is described by these fields.
	* The application can supply a colormap by setting colormap non-NULL before
	* calling jpeg_start_decompress; otherwise a colormap is created during
	* jpeg_start_decompress.
	* The map has out_color_components rows and actual_number_of_colors columns.
	*/
	actual_number_of_colors = 0;	/* number of entries in use */
	colormap = 0;		/* The color map as a 2-D pixel array */

	/* State variable: index of next scaled scanline to be read from
	* jpeg_read_scanlines().  Application may use this to control its
	* processing loop, e.g., "while (output_scanline < output_height)".
	*/

	output_scanline = 0;	/* 0 .. output_height-1  */

	/* Internal JPEG parameters --- the application usually need not look at
	* these fields.
	*/

	/* Quantization and Huffman tables are carried forward across input
	* datastreams when processing abbreviated JPEG datastreams.
	*/

	ChMemClear( quant_tbl_ptrs, sizeof(quant_tbl_ptrs[NUM_QUANT_TBLS]) );
	/* ptrs to coefficient quantization tables, or NULL if not defined */

	ChMemClear( dc_huff_tbl_ptrs, sizeof( dc_huff_tbl_ptrs[NUM_HUFF_TBLS]) );
	ChMemClear( ac_huff_tbl_ptrs, sizeof(ac_huff_tbl_ptrs[NUM_HUFF_TBLS]) );
	/* ptrs to Huffman coding tables, or NULL if not defined */

	/* These parameters are never carried across datastreams, since they
	* are given in SOF/SOS markers or defined to be reset by SOI.
	*/

	data_precision = 0;		/* bits of precision in image data */

	comp_info = 0;
	/* comp_info[i] describes component that appears i'th in SOF */

	ChMemClear(  arith_dc_L, sizeof(arith_dc_L[NUM_ARITH_TBLS] )); /* L values for DC arith-coding tables */
	ChMemClear(  arith_dc_U, sizeof(arith_dc_U[NUM_ARITH_TBLS])); /* U values for DC arith-coding tables */
	ChMemClear(  arith_ac_K, sizeof(arith_ac_K[NUM_ARITH_TBLS])); /* Kx values for AC arith-coding tables */

	arith_code = 0;		/* TRUE=arithmetic coding, FALSE=Huffman */

	restart_interval = 0; /* MCUs per restart interval, or 0 for no restart */

	/* These fields record data obtained from optional markers recognized by
	* the JPEG library.
	*/
	saw_JFIF_marker = 0;	/* TRUE iff a JFIF APP0 marker was found */
	/* Data copied from JFIF marker: */
	density_unit = 0;		/* JFIF code for pixel size units */
	X_density = 0;		/* Horizontal pixel density */
	Y_density = 0;		/* Vertical pixel density */
	saw_Adobe_marker = 0;	/* TRUE iff an Adobe APP14 marker was found */
	Adobe_transform = 0;	/* Color transform code from Adobe marker */

	CCIR601_sampling = 0;	/* TRUE=first samples are cosited */

	/* Remaining fields are known throughout decompressor, but generally
	* should not be touched by a surrounding application.
	*/

	/*
	* These fields are computed during decompression startup
	*/
	max_h_samp_factor = 0;	/* largest h_samp_factor */
	max_v_samp_factor = 0;	/* largest v_samp_factor */

	min_DCT_scaled_size = 0;	/* smallest DCT_scaled_size of any component */

	total_iMCU_rows = 0;	/* # of iMCU rows to be output by coef ctlr */
	/* The coefficient controller outputs data in units of MCU rows as defined
	* for fully interleaved scans (whether the JPEG file is interleaved or not).
	* There are v_samp_factor * DCT_scaled_size sample rows of each component
	* in an "iMCU" (interleaved MCU) row.
	*/

	sample_range_limit = 0; /* table for fast range-limiting */

	/*
	* These fields are valid during any one scan.
	* They describe the components and MCUs actually appearing in the scan.
	*/
	comps_in_scan = 0;		/* # of JPEG components in this scan */
	ChMemClear( cur_comp_info, sizeof(cur_comp_info[MAX_COMPS_IN_SCAN]));
	/* *cur_comp_info[i] describes component that appears i'th in SOS */

	MCUs_per_row = 0;	/* # of MCUs across the image */
	MCU_rows_in_scan = 0;	/* # of MCU rows in the image */

	blocks_in_MCU = 0;		/* # of DCT blocks per MCU */
	ChMemClear( MCU_membership, sizeof(MCU_membership[MAX_BLOCKS_IN_MCU]) );
	/* MCU_membership[i] is index in cur_comp_info of component owning */
	/* i'th block in an MCU */

	/* This field is shared between entropy decoder and marker parser.
	* It is either zero or the code of a JPEG marker that has been
	* read from the data source, but has not yet been processed.
	*/
	unread_marker = 0;

	/*
	* Links to decompression subobjects (methods, private variables of modules)
	*/
	master = 0;
	main = 0;
	coef = 0;
	post = 0;
	marker = 0;
	entropy = 0;
	idct = 0;
	upsample = 0;
	cconvert = 0;
	cquantize = 0;
}

jpeg_decompress_struct::~jpeg_decompress_struct()
{
	if ( src )
	{
		delete src;
	}

	if ( master )
	{
		delete master;
	}
	if ( main )
	{
		delete main;
	}
	if ( coef )
	{
		delete coef;
	}
	if ( post )
	{
		delete post;
	}
	if ( marker )
	{
		delete marker;
	}

	if ( entropy )
	{
		delete entropy;
	}

	if ( idct )
	{
		delete idct;
	}

	if ( upsample )
	{
		delete upsample;
	}
	if ( cconvert )
	{
		delete cconvert;
	}

	if ( cquantize )
	{
		delete cquantize;
	}
}

/*
 * Initialization of a JPEG decompression object.
 * The error manager must already be set up (in case memory manager fails).
 */


void ChJPEG::jpeg_create_decompress (j_decompress_ptr cinfo)
{
  int i;

  /* For debugging purposes, zero the whole master structure.
   * But error manager pointer is already there, so save and restore it.
   */

  ((j_common_ptr)cinfo)->is_decompressor = true;

  /* Initialize a memory manager instance for this object */
  jinit_memory_mgr((j_common_ptr) cinfo);

  /* Zero out pointers to permanent structures. */
  ((j_common_ptr)cinfo)->progress = NULL;
  cinfo->src = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;

  for (i = 0; i < NUM_HUFF_TBLS; i++) 
  {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  cinfo->sample_range_limit = NULL;

  /* Initialize marker processor so application can override methods
   * for COM, APPn markers before calling jpeg_read_header.
   */
  cinfo->marker = NULL;
  jinit_marker_reader(cinfo);

  /* OK, I'm ready */
  ((j_common_ptr)cinfo)->global_state = DSTATE_START;
}


/*
 * Destruction of a JPEG decompression object
 */

void ChJPEG::jpeg_destroy_decompress (j_decompress_ptr cinfo)
{
  jpeg_destroy((j_common_ptr) cinfo); /* use common routine */
}


/*
 * Install a special processing method for COM or APPn markers.
 */

void ChJPEG::jpeg_set_marker_processor (j_decompress_ptr cinfo, int marker_code,
			   jpeg_marker_parser_method routine)
{
  if (marker_code == JPEG_COM)
    cinfo->marker->process_COM = routine;
  else if (marker_code >= JPEG_APP0 && marker_code <= JPEG_APP0+15)
    cinfo->marker->process_APPn[marker_code-JPEG_APP0] = routine;
  else
    ERREXIT1(cinfo, JERR_UNKNOWN_MARKER, marker_code);
}


/*
 * Set default decompression parameters.
 */

void ChJPEG::default_decompress_parms (j_decompress_ptr cinfo)
{
  /* Guess the input colorspace, and set output colorspace accordingly. */
  /* (Wish JPEG committee had provided a real way to specify this...) */
  /* Note application may override our guesses. */
  switch (cinfo->num_components) {
  case 1:
    cinfo->jpeg_color_space = JCS_GRAYSCALE;
    cinfo->out_color_space = JCS_GRAYSCALE;
    break;
    
  case 3:
    if (cinfo->saw_JFIF_marker) {
      cinfo->jpeg_color_space = JCS_YCbCr; /* JFIF implies YCbCr */
    } else if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->jpeg_color_space = JCS_RGB;
	break;
      case 1:
	cinfo->jpeg_color_space = JCS_YCbCr;
	break;
      default:
	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->jpeg_color_space = JCS_YCbCr; /* assume it's YCbCr */
	break;
      }
    } else {
      /* Saw no special markers, try to guess from the component IDs */
      int cid0 = cinfo->comp_info[0].component_id;
      int cid1 = cinfo->comp_info[1].component_id;
      int cid2 = cinfo->comp_info[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
	cinfo->jpeg_color_space = JCS_YCbCr; /* assume JFIF w/out marker */
      else if (cid0 == 82 && cid1 == 71 && cid2 == 66)
	cinfo->jpeg_color_space = JCS_RGB; /* ASCII 'R', 'G', 'B' */
      else {
	TRACEMS3(cinfo, 1, JTRC_UNKNOWN_IDS, cid0, cid1, cid2);
	cinfo->jpeg_color_space = JCS_YCbCr; /* assume it's YCbCr */
      }
    }
    /* Always guess RGB is proper output colorspace. */
    cinfo->out_color_space = JCS_RGB;
    break;
    
  case 4:
    if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->jpeg_color_space = JCS_CMYK;
	break;
      case 2:
	cinfo->jpeg_color_space = JCS_YCCK;
	break;
      default:
	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->jpeg_color_space = JCS_YCCK; /* assume it's YCCK */
	break;
      }
    } else {
      /* No special markers, assume straight CMYK. */
      cinfo->jpeg_color_space = JCS_CMYK;
    }
    cinfo->out_color_space = JCS_CMYK;
    break;
    
  default:
    cinfo->jpeg_color_space = JCS_UNKNOWN;
    cinfo->out_color_space = JCS_UNKNOWN;
    break;
  }

  /* Set defaults for other decompression parameters. */
  cinfo->scale_num = 1;		/* 1:1 scaling */
  cinfo->scale_denom = 1;
  cinfo->output_gamma = 1.0;

#define  TRY_QUANT 1

  #if TRY_QUANT
  int numColors = 1;

  if ( cinfo->jpeg_color_space != JCS_GRAYSCALE )
  {

	  if ( m_flOptions & load8Bit )
	  {
	  	numColors = 8;
	  }
	  else  if ( m_flOptions & load24Bit )
	  {
	  	numColors = -1;
	  }
	  else
	  {
		numColors = ChImgUtil::MaxDeviceColors();
	  }

	  if (  numColors > 256 ||  numColors < 0 )
	  { // supports true color
	  	cinfo->quantize_colors = FALSE;
	    cinfo->two_pass_quantize = TRUE;
		m_flOptions	= load24Bit;
	  }
	  else
	  {
	  	cinfo->quantize_colors = true;
		m_flOptions	= load8Bit;
	  }
  }
  else
  {

		if ( m_flOptions & loadAuto )
		{
			numColors = ChImgUtil::MaxDeviceColors();	 
			if (  numColors > 256 ||  numColors < 0 )
			{
			    cinfo->two_pass_quantize = true;
				m_flOptions	= load24Bit;
			}
			else
			{
				m_flOptions	= load8Bit;
			}
		}
		else
		{
			if ( m_flOptions & load24Bit )
			{
			    cinfo->two_pass_quantize = true;
			}
		}
	  	cinfo->quantize_colors = true;

		//GetConsumer()->SetGrayscale( true );
  }
  #else
  if ( m_flOptions & ChDib::loadAuto )
  {
#ifdef CH_MSW
	  int numColors;
	  HWND hWnd = ::GetDesktopWindow();
	  HDC hDC   = ::GetDC( hWnd );
	  numColors = ::GetDeviceCaps( hDC, NUMCOLORS);
	  ::ReleaseDC( hWnd, hDC );
#else
	cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ << endl;
#endif

	  if (  numColors > 256 ||  numColors < 0 )
	  { // supports true color
		m_flOptions	= ChDib::load24Bit;
	  }
	  else
	  {
		m_flOptions	= ChDib::load8Bit;
	  }

  }
  cinfo->quantize_colors = FALSE;
  cinfo->two_pass_quantize = TRUE;
  #endif

  cinfo->raw_data_out = FALSE;
  /* We set these in case application only sets quantize_colors. */
  cinfo->dither_mode = JDITHER_FS;
 // cinfo->dither_mode = JDITHER_ORDERED;
  cinfo->desired_number_of_colors = 256;
  cinfo->colormap = NULL;


  if ( cinfo->quantize_colors )
  #if defined( CH_MSW )
  {
		RGBQUAD* pClrTbl;	  
	  	if ( cinfo->jpeg_color_space == JCS_GRAYSCALE )
		{
		    pClrTbl = ChImgUtil::GetGrayScaleColorTable();
			cinfo->desired_number_of_colors =  ChImgUtil::GetGrayScaleClrTblNumEntries();  // Number of color table entries
		} 
		else
		{
 		    pClrTbl = ChImgUtil::GetStdColorTable();
 			cinfo->desired_number_of_colors =  ChImgUtil::GetStdClrTblNumEntries();  // Number of color table entries

		}

		if ( cinfo->desired_number_of_colors )
		{
			cinfo->actual_number_of_colors  =  cinfo->desired_number_of_colors ;
			// allocate memory for the color table
		  	cinfo->colormap = ((j_common_ptr)cinfo)->mem->alloc_sarray
		    ((j_common_ptr) cinfo, JPOOL_IMAGE,
		     (JDIMENSION) cinfo->desired_number_of_colors, (JDIMENSION) 3 );
		
			  for ( int i = 0; i < cinfo->desired_number_of_colors; i++) 
			  {
					cinfo->colormap[0][i] = pClrTbl[i].rgbRed;
					cinfo->colormap[1][i] = pClrTbl[i].rgbGreen;
					cinfo->colormap[2][i] = pClrTbl[i].rgbBlue;
			  }
		}

 }
 #else
 {
		cerr << "Using default color table " << __FILE__ << ":" << __LINE__ << endl;
 }
 #endif




  /* DCT algorithm preference */
  cinfo->dct_method = JDCT_DEFAULT;
  cinfo->dct_method = JDCT_FLOAT;
  cinfo->do_fancy_upsampling = TRUE;
}


/*
 * Decompression startup: read start of JPEG datastream to see what's there.
 * Need only initialize JPEG object and supply a data source before calling.
 *
 * This routine will read as far as the first SOS marker (ie, actual start of
 * compressed data), and will save all tables and parameters in the JPEG
 * object.  It will also initialize the decompression parameters to default
 * values, and finally return JPEG_HEADER_OK.  On return, the application may
 * adjust the decompression parameters and then call jpeg_start_decompress.
 * (Or, if the application only wanted to determine the image parameters,
 * the data need not be decompressed.  In that case, call jpeg_abort or
 * jpeg_destroy to release any temporary space.)
 * If an abbreviated (tables only) datastream is presented, the routine will
 * return JPEG_HEADER_TABLES_ONLY upon reaching EOI.  The application may then
 * re-use the JPEG object to read the abbreviated image datastream(s).
 * It is unnecessary (but OK) to call jpeg_abort in this case.
 * The JPEG_SUSPENDED return code only occurs if the data source module
 * requests suspension of the decompressor.  In this case the application
 * should load more source data and then re-call jpeg_read_header to resume
 * processing.
 * If a non-suspending data source is used and require_image is TRUE, then the
 * return code need not be inspected since only JPEG_HEADER_OK is possible.
 */

int ChJPEG::jpeg_read_header (j_decompress_ptr cinfo, bool require_image)
{
  int retcode;

  if (((j_common_ptr)cinfo)->global_state == DSTATE_START) 
  {
    /* First-time actions: reset appropriate modules */
    ((j_common_ptr)cinfo)->err->reset_error_mgr((j_common_ptr) cinfo);
    cinfo->marker->reset_marker_reader(cinfo);
    cinfo->src->init_source(cinfo);
    ((j_common_ptr)cinfo)->global_state = DSTATE_INHEADER;
  } 
  else if (((j_common_ptr)cinfo)->global_state != DSTATE_INHEADER) 
  {
    ERREXIT1(cinfo, JERR_BAD_STATE, ((j_common_ptr)cinfo)->global_state);
  }

  retcode = cinfo->marker->read_markers(cinfo);

  switch (retcode) {
  case JPEG_HEADER_OK:		/* Found SOS, prepare to decompress */
    /* Set up default parameters based on header data */
    default_decompress_parms(cinfo);
    /* Set global state: ready for start_decompress */
    ((j_common_ptr)cinfo)->global_state = DSTATE_READY;
    break;

  case JPEG_HEADER_TABLES_ONLY:	/* Found EOI before any SOS */
    if (cinfo->marker->saw_SOF)
      ERREXIT(cinfo, JERR_SOF_NO_SOS);
    if (require_image)		/* Complain if application wants an image */
      ERREXIT(cinfo, JERR_NO_IMAGE);
    /* We need not do any cleanup since only permanent storage (for DQT, DHT)
     * has been allocated.
     */
    /* Set global state: ready for a new datastream */
    ((j_common_ptr)cinfo)->global_state = DSTATE_START;
    break;

  case JPEG_SUSPENDED:		/* Had to suspend before end of headers */
    /* no work */
    break;
  }

  return retcode;
}


/*
 * Decompression initialization.
 * jpeg_read_header must be completed before calling this.
 *
 * If a multipass operating mode was selected, this will do all but the
 * last pass, and thus may take a great deal of time.
 */

void ChJPEG::jpeg_start_decompress (j_decompress_ptr cinfo)
{
  JDIMENSION chunk_ctr, last_chunk_ctr;

  if (((j_common_ptr)cinfo)->global_state != DSTATE_READY)
    ERREXIT1(cinfo, JERR_BAD_STATE, ((j_common_ptr)cinfo)->global_state);
  /* Perform master selection of active modules */
  jinit_master_decompress(cinfo);
  /* Do all but the final (output) pass, and set up for that one. */
  for (;;) 
  {
    cinfo->master->prepare_for_pass(cinfo);
    if (cinfo->master->is_last_pass)
      break;
    chunk_ctr = 0;
    while (chunk_ctr < cinfo->main->num_chunks) 
    {
      /* Call progress monitor hook if present */
      if (((j_common_ptr)cinfo)->progress != NULL) 
      {
		((j_common_ptr)cinfo)->progress->pass_counter = (long) chunk_ctr;
		((j_common_ptr)cinfo)->progress->pass_limit = (long) cinfo->main->num_chunks;
		((j_common_ptr)cinfo)->progress->progress_monitor((j_common_ptr) cinfo);
      }
      /* Process some data */
      last_chunk_ctr = chunk_ctr;
      cinfo->main->process_data( cinfo, (JSAMPARRAY) NULL,
				    &chunk_ctr, (JDIMENSION) 0);
      if (chunk_ctr == last_chunk_ctr) /* check for failure to make progress */
		ERREXIT(cinfo, JERR_CANT_SUSPEND);
    }
    cinfo->master->finish_pass(cinfo);
  }
  /* Ready for application to drive last pass through jpeg_read_scanlines
   * or jpeg_read_raw_data.
   */
  cinfo->output_scanline = 0;
  ((j_common_ptr)cinfo)->global_state = (cinfo->raw_data_out ? DSTATE_RAW_OK : DSTATE_SCANNING);
}


/*
 * Read some scanlines of data from the JPEG decompressor.
 *
 * The return value will be the number of lines actually read.
 * This may be less than the number requested in several cases,
 * including bottom of image, data source suspension, and operating
 * modes that emit multiple scanlines at a time.
 *
 * Note: we warn about excess calls to jpeg_read_scanlines() since
 * this likely signals an application programmer error.  However,
 * an oversize buffer (max_lines > scanlines remaining) is not an error.
 */

JDIMENSION ChJPEG::jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines,
		     JDIMENSION max_lines)
{
  JDIMENSION row_ctr;

  if (((j_common_ptr)cinfo)->global_state != DSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, ((j_common_ptr)cinfo)->global_state);
  if (cinfo->output_scanline >= cinfo->output_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

  /* Call progress monitor hook if present */
  if (((j_common_ptr)cinfo)->progress != NULL) 
  {
    ((j_common_ptr)cinfo)->progress->pass_counter = (long) cinfo->output_scanline;
    ((j_common_ptr)cinfo)->progress->pass_limit = (long) cinfo->output_height;
    ((j_common_ptr)cinfo)->progress->progress_monitor((j_common_ptr) cinfo);
  }

  /* Process some data */
  row_ctr = 0;
  cinfo->main->process_data(cinfo, scanlines, &row_ctr, max_lines);
  cinfo->output_scanline += row_ctr;
  return row_ctr;
}


/*
 * Alternate entry point to read raw data.
 * Processes exactly one iMCU row per call, unless suspended.
 */

JDIMENSION ChJPEG::jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data,
		    JDIMENSION max_lines)
{
  JDIMENSION lines_per_iMCU_row;

  if (((j_common_ptr)cinfo)->global_state != DSTATE_RAW_OK)
    ERREXIT1(cinfo, JERR_BAD_STATE, ((j_common_ptr)cinfo)->global_state);
  
  if (cinfo->output_scanline >= cinfo->output_height) 
  {
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);
    return 0;
  }

  /* Call progress monitor hook if present */
  if (((j_common_ptr)cinfo)->progress != NULL) 
  {
    ((j_common_ptr)cinfo)->progress->pass_counter = (long) cinfo->output_scanline;
    ((j_common_ptr)cinfo)->progress->pass_limit = (long) cinfo->output_height;
    ((j_common_ptr)cinfo)->progress->progress_monitor((j_common_ptr) cinfo);
  }

  /* Verify that at least one iMCU row can be returned. */
  lines_per_iMCU_row = cinfo->max_v_samp_factor * cinfo->min_DCT_scaled_size;
  if (max_lines < lines_per_iMCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

  /* Decompress directly into user's buffer. */
  if (! cinfo->coef->decompress_data(cinfo, data))
    return 0;			/* suspension forced, can do nothing more */

  /* OK, we processed one iMCU row. */
  cinfo->output_scanline += lines_per_iMCU_row;
  return lines_per_iMCU_row;
}


/*
 * Finish JPEG decompression.
 *
 * This will normally just verify the file trailer and release temp storage.
 *
 * Returns FALSE if suspended.  The return value need be inspected only if
 * a suspending data source is used.
 */

bool ChJPEG::jpeg_finish_decompress (j_decompress_ptr cinfo)
{
  if (((j_common_ptr)cinfo)->global_state == DSTATE_SCANNING ||
      ((j_common_ptr)cinfo)->global_state == DSTATE_RAW_OK) {
    /* Terminate final pass */
    if (cinfo->output_scanline < cinfo->output_height)
      ERREXIT(cinfo, JERR_TOO_LITTLE_DATA);
    cinfo->master->finish_pass(cinfo);
    ((j_common_ptr)cinfo)->global_state = DSTATE_STOPPING;
  } 
  else if (((j_common_ptr)cinfo)->global_state != DSTATE_STOPPING) 
  {
    /* Repeat call after a suspension? */
    ERREXIT1(cinfo, JERR_BAD_STATE, ((j_common_ptr)cinfo)->global_state);
  }
  /* Check for EOI in source file, unless master control already read it */
  if (! cinfo->master->eoi_processed) {
    switch (cinfo->marker->read_markers(cinfo)) 
    {
	    case JPEG_HEADER_OK:	/* Found SOS!? */
	      ERREXIT(cinfo, JERR_EOI_EXPECTED);
	      break;
	    case JPEG_HEADER_TABLES_ONLY: /* Found EOI, A-OK */
	      break;
	    case JPEG_SUSPENDED:	/* Suspend, come back later */
	      return FALSE;
    }
  }
  /* Do final cleanup */
  cinfo->src->term_source(cinfo);
  /* We can use jpeg_abort to release memory and reset global_state */
  jpeg_abort((j_common_ptr) cinfo);
  return TRUE;
}


/*
 * Abort processing of a JPEG decompression operation,
 * but don't destroy the object itself.
 */

void ChJPEG::jpeg_abort_decompress (j_decompress_ptr cinfo)
{
  jpeg_abort((j_common_ptr) cinfo); /* use common routine */
}

bool ChJPEG::put_scanline_someplace( j_decompress_ptr cinfo, unsigned char* buffer, 
														int row_stride)
{

	if ( m_boolFirst )
	{
		m_boolFirst = false;

		ChImageInfo  imageInfo;
		ChMemClearStruct( &imageInfo );
		imageInfo.iWidth = cinfo->output_width; 
		imageInfo.iHeight = cinfo->output_height;
		GetConsumer()->NewImage( &imageInfo );

		// create a dib which matches our JPEG size
		ChImageFrameInfo frameInfo;
		ChMemClearStruct( &frameInfo );	
		frameInfo.iFrame = 0;
		frameInfo.iWidth = cinfo->output_width;
		frameInfo.iHeight = cinfo->output_height;

		if ( cinfo->jpeg_color_space == JCS_GRAYSCALE )
		{
			frameInfo.luAttrs  |= ChImageConsumer::grayScale;
		}

		GetConsumer()->Create( &frameInfo, m_flOptions & load8Bit
														? 8 : 24 );
		// copy the color map			

		JSAMPARRAY colormap = cinfo->colormap;
		int num_colors = cinfo->actual_number_of_colors <= 256 
								? cinfo->actual_number_of_colors  : 256;


		if ( num_colors && colormap )
		{

   			RGBQUAD* pDibClrTbl = new RGBQUAD[num_colors];
			ASSERT( pDibClrTbl );
			if (cinfo->out_color_components == 3) 
			{
			  /* Normal case with RGB colormap */
			  for ( int i = 0; i < num_colors; i++) 
			  {
					pDibClrTbl[i].rgbRed = GETJSAMPLE(colormap[0][i] );
					pDibClrTbl[i].rgbGreen = GETJSAMPLE(colormap[1][i] );
					pDibClrTbl[i].rgbBlue = GETJSAMPLE(colormap[2][i] );
			  }
			} 
			else 
			{
			  /* Grayscale colormap (only happens with grayscale quantization) */
			  for ( int i = 0; i < num_colors; i++) 
			  {
					pDibClrTbl[i].rgbRed = GETJSAMPLE(colormap[0][i] );
					pDibClrTbl[i].rgbGreen = GETJSAMPLE(colormap[0][i] );
					pDibClrTbl[i].rgbBlue = GETJSAMPLE(colormap[0][i] );
			  }
			}

			GetConsumer()->SetColorTable( 0, pDibClrTbl, num_colors );
			delete [] pDibClrTbl;
		}
	}

	if ( m_iCurrScanLine >= (int)cinfo->output_height )
	{
    	ERREXIT(cinfo, JERR_IMAGE_TOO_BIG);
	}

	#if 0
	// copy the bits to our buffer
	unsigned char * pBits = ( unsigned char * )ChDib::GetBitsAddress();
	if ( m_flOptions & ChDib::load8Bit )
	{
	    pBits += ((cinfo->output_width + 3) & ~3) * ( ChDib::GetHeight() - m_iCurrScanLine - 1);
	}
	else
	{
	    pBits += (((cinfo->output_width * 3 ) + 3) & ~3) 
	    			* ( cinfo->output_height - m_iCurrScanLine - 1);
	}
	#endif

	if (cinfo->out_color_components == 3) 
	{

		if ( row_stride == (int)cinfo->output_width )
		{
			#if 0
			for (int col = cinfo->output_width; col > 0; col--) 
			{
				*pBits++ = *buffer++;
			}
			#endif
	
			GetConsumer()->SetScanLine( 0, m_iCurrScanLine,
									buffer, cinfo->output_width, 
									ChImageConsumer::format8Bit );

		}
		else
		{ 
			#if 0
			for (int col = cinfo->output_width; col > 0; col--) 
			{
					*pBits++ = buffer[2];	
					*pBits++ = buffer[1];
					*pBits++ = buffer[0];
					buffer += 3;
			}
			#endif

			GetConsumer()->SetScanLine( 0, m_iCurrScanLine,
									buffer, cinfo->output_width, 
									ChImageConsumer::format24BGR );

		}
	}
	else
	{
		if ( m_flOptions & load24Bit )
		{
			LPBYTE pData = new BYTE[(((cinfo->output_width * 3 ) + 3) & ~3) ];
			ASSERT( pData );  

			LPBYTE pBits = pData;
			JSAMPARRAY colormap = cinfo->colormap;
			for (int col = cinfo->output_width; col > 0; col--) 
			{
		
				*pBits++ = GETJSAMPLE(colormap[0][*buffer] );	
				*pBits++ = GETJSAMPLE(colormap[0][*buffer] );	
				*pBits++ = GETJSAMPLE(colormap[0][*buffer] );	

				buffer++;
			}
			GetConsumer()->SetScanLine( 0, m_iCurrScanLine,
									pData, cinfo->output_width, 
									ChImageConsumer::format24RGB );
		}
		else
		{
			#if 0
			for (int col = cinfo->output_width; col > 0; col--) 
			{
				*pBits++ = *buffer++;	
			}
			#endif

			GetConsumer()->SetScanLine( 0, m_iCurrScanLine,
									buffer, cinfo->output_width, 
									ChImageConsumer::format8Bit );
		}


	}
	m_iCurrScanLine++;
	
	return true;
}
