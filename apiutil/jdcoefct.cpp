/*
 * jdcoefct.c
 *
 * Copyright (C) 1994-1995, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the coefficient buffer controller for decompression.
 * This controller is the top level of the JPEG decompressor proper.
 * The coefficient buffer lies between entropy decoding and inverse-DCT steps.
 */

// $Header$

#include "headers.h"

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#include <ChJpegDecoder.h>


/* Private buffer controller object */

class my_coef_controller : public  jpeg_d_coef_controller
{

	public :
		my_coef_controller()		{}
		virtual ~my_coef_controller() {}

  		virtual void start_pass(j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  		virtual bool decompress_data(j_decompress_ptr cinfo,
				     JSAMPIMAGE output_buf);


	private :

		// private methods
		void start_iMCU_row(j_decompress_ptr cinfo);


		JDIMENSION iMCU_row_num;	/* iMCU row # within image */
		JDIMENSION mcu_ctr;		/* counts MCUs processed in current row */
		int MCU_vert_offset;		/* counts MCU rows within iMCU row */
		int MCU_rows_per_iMCU_row;	/* number of such rows needed */


	public :
		/* In single-pass modes without block smoothing, it's sufficient to buffer
		* just one MCU (although this may prove a bit slow in practice).
		* We allocate a workspace of MAX_BLOCKS_IN_MCU coefficient blocks,
		* and let the entropy decoder write into that workspace each time.
		* (On 80x86, the workspace is FAR even though it's not really very big;
		* this is to keep the module interfaces unchanged when a large coefficient
		* buffer is necessary.)
		* In multi-pass modes, this array points to the current MCU's blocks
		* within the virtual arrays.
		*/
		JBLOCKROW MCU_buffer[MAX_BLOCKS_IN_MCU];

		/* In multi-pass modes, we need a virtual block array for each component. */
		jvirt_barray_ptr whole_image[MAX_COMPONENTS];

};

typedef class my_coef_controller * my_coef_ptr;


void my_coef_controller::start_iMCU_row (j_decompress_ptr cinfo)
/* Reset within-iMCU-row counters for a new row */
{

  /* In an interleaved scan, an MCU row is the same as an iMCU row.
   * In a noninterleaved scan, an iMCU row has v_samp_factor MCU rows.
   * But at the bottom of the image, process only what's left.
   */
  if (cinfo->comps_in_scan > 1) 
  {
    MCU_rows_per_iMCU_row = 1;
  } 
  else 
  {
    if ( iMCU_row_num < ( cinfo->total_iMCU_rows-1))
      MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->v_samp_factor;
    else
      MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->last_row_height;
  }

  mcu_ctr = 0;
  MCU_vert_offset = 0;
}


/*
 * Initialize for a processing pass.
 */

void my_coef_controller::start_pass(j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{

  iMCU_row_num = 0;
  start_iMCU_row(cinfo);

  switch (pass_mode) 
  {
	  case JBUF_PASS_THRU:
	    if (whole_image[0] != NULL)
	      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
	    //decompress_data = decompress_data;
	    break;
	#ifdef D_MULTISCAN_FILES_SUPPORTED
	  case JBUF_SAVE_SOURCE:
	    if (whole_image[0] == NULL)
	      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
	    decompress_data = decompress_read;
	    break;
	  case JBUF_CRANK_DEST:
	    if (whole_image[0] == NULL)
	      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
	    decompress_data = decompress_output;
	    break;
	#endif
	  default:
	    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
	    break;
  }
}


/*
 * Process some data in the single-pass case.
 * Always attempts to emit one fully interleaved MCU row ("iMCU" row).
 * Returns TRUE if it completed a row, FALSE if not (suspension).
 *
 * NB: output_buf contains a plane for each component in image.
 * For single pass, this is the same as the components in the scan.
 */

bool my_coef_controller::decompress_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{

  JDIMENSION MCU_col_num;	/* index of current MCU within row */
  JDIMENSION last_MCU_col = cinfo->MCUs_per_row - 1;
  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  int blkn, ci, xindex, yindex, yoffset, useful_width;
  JSAMPARRAY output_ptr;
  JDIMENSION start_col, output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

  /* Loop to process as much as one whole iMCU row */
  for (yoffset = MCU_vert_offset; yoffset < MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = mcu_ctr; MCU_col_num <= last_MCU_col;
	 MCU_col_num++) {
      /* Try to fetch an MCU.  Entropy decoder expects buffer to be zeroed. */
      jzero_far((void FAR *) MCU_buffer[0],
		(size_t) (cinfo->blocks_in_MCU * SIZEOF(JBLOCK)));
      if (!cinfo->entropy->decode_mcu(cinfo, MCU_buffer)) {
	/* Suspension forced; update state counters and exit */
	MCU_vert_offset = yoffset;
	mcu_ctr = MCU_col_num;
	return FALSE;
      }
      /* Determine where data should go in output_buf and do the IDCT thing.
       * We skip dummy blocks at the right and bottom edges (but blkn gets
       * incremented past them!).  Note the inner loop relies on having
       * allocated the MCU_buffer[] blocks sequentially.
       */
      blkn = 0;			/* index of current DCT block within MCU */
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	/* Don't bother to IDCT an uninteresting component. */
	if (! compptr->component_needed) {
	  blkn += compptr->MCU_blocks;
	  continue;
	}
	inverse_DCT = cinfo->idct->inverse_DCT[compptr->component_index];
	useful_width = (MCU_col_num < last_MCU_col) ? compptr->MCU_width
						    : compptr->last_col_width;
	output_ptr = output_buf[ci] + yoffset * compptr->DCT_scaled_size;
	start_col = MCU_col_num * compptr->MCU_sample_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  if (iMCU_row_num < last_iMCU_row ||
	      yoffset+yindex < compptr->last_row_height) {
	    output_col = start_col;
	    for (xindex = 0; xindex < useful_width; xindex++) {
	      (*inverse_DCT) (cinfo, compptr,
			      (JCOEFPTR) MCU_buffer[blkn+xindex],
			      output_ptr, output_col);
	      output_col += compptr->DCT_scaled_size;
	    }
	  }
	  blkn += compptr->MCU_width;
	  output_ptr += compptr->DCT_scaled_size;
	}
      }
    }
    /* Completed an MCU row, but perhaps not an iMCU row */
    mcu_ctr = 0;
  }
  /* Completed the iMCU row, advance counters for next one */
  iMCU_row_num++;
  start_iMCU_row(cinfo);
  return TRUE;
}


#ifdef D_MULTISCAN_FILES_SUPPORTED

/*
 * Process some data: handle an input pass for a multiple-scan file.
 * We read the equivalent of one fully interleaved MCU row ("iMCU" row)
 * per call, ie, v_samp_factor block rows for each component in the scan.
 * No data is returned; we just stash it in the virtual arrays.
 * Returns TRUE if it completed a row, FALSE if not (suspension).
 */

bool my_coef_controller::decompress_read (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{

  JDIMENSION MCU_col_num;	/* index of current MCU within row */
  int blkn, ci, xindex, yindex, yoffset;
  JDIMENSION total_width, start_col;
  JBLOCKARRAY buffer[MAX_COMPS_IN_SCAN];
  JBLOCKROW buffer_ptr;
  jpeg_component_info *compptr;

  /* Align the virtual buffers for the components used in this scan. */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, whole_image[compptr->component_index],
       iMCU_row_num * compptr->v_samp_factor, TRUE);
    /* Entropy decoder expects buffer to be zeroed. */
    total_width = (JDIMENSION) jround_up((long) compptr->width_in_blocks,
					 (long) compptr->h_samp_factor);
    for (yindex = 0; yindex < compptr->v_samp_factor; yindex++) {
      jzero_far((void FAR *) buffer[ci][yindex], 
		(size_t) (total_width * SIZEOF(JBLOCK)));
    }
  }

  /* Loop to process one whole iMCU row */
  for (yoffset = MCU_vert_offset; yoffset < MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = mcu_ctr; MCU_col_num < cinfo->MCUs_per_row;
	 MCU_col_num++) {
      /* Construct list of pointers to DCT blocks belonging to this MCU */
      blkn = 0;			/* index of current DCT block within MCU */
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	start_col = MCU_col_num * compptr->MCU_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  buffer_ptr = buffer[ci][yindex+yoffset] + start_col;
	  for (xindex = 0; xindex < compptr->MCU_width; xindex++) {
	    MCU_buffer[blkn++] = buffer_ptr++;
	  }
	}
      }
      /* Try to fetch the MCU. */
      if (! (*cinfo->entropy->decode_mcu) (cinfo, MCU_buffer)) {
	/* Suspension forced; update state counters and exit */
	MCU_vert_offset = yoffset;
	mcu_ctr = MCU_col_num;
	return FALSE;
      }
    }
    /* Completed an MCU row, but perhaps not an iMCU row */
    mcu_ctr = 0;
  }
  /* Completed the iMCU row, advance counters for next one */
  iMCU_row_num++;
  start_iMCU_row(cinfo);
  return TRUE;
}


/*
 * Process some data: output from the virtual arrays after reading is done.
 * Always emits one fully interleaved MCU row ("iMCU" row).
 * Always returns TRUE --- suspension is not possible.
 *
 * NB: output_buf contains a plane for each component in image.
 */

bool my_coef_controller::decompress_output (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{

  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  JDIMENSION block_num;
  int ci, block_row, block_rows;
  JBLOCKARRAY buffer;
  JBLOCKROW buffer_ptr;
  JSAMPARRAY output_ptr;
  JDIMENSION output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    /* Don't bother to IDCT an uninteresting component. */
    if (! compptr->component_needed)
      continue;
    /* Align the virtual buffer for this component. */
    buffer = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, whole_image[ci],
       iMCU_row_num * compptr->v_samp_factor, FALSE);
    /* Count non-dummy DCT block rows in this iMCU row. */
    if (iMCU_row_num < last_iMCU_row)
      block_rows = compptr->v_samp_factor;
    else {
      /* NB: can't use last_row_height here, since may not be set! */
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
    }
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
    /* Loop over all DCT blocks to be processed. */
    for (block_row = 0; block_row < block_rows; block_row++) {
      buffer_ptr = buffer[block_row];
      output_col = 0;
      for (block_num = 0; block_num < compptr->width_in_blocks; block_num++) {
	(*inverse_DCT) (cinfo, compptr, (JCOEFPTR) buffer_ptr,
			output_ptr, output_col);
	buffer_ptr++;
	output_col += compptr->DCT_scaled_size;
      }
      output_ptr += compptr->DCT_scaled_size;
    }
  }

  iMCU_row_num++;
  return TRUE;
}

#endif /* D_MULTISCAN_FILES_SUPPORTED */


/*
 * Initialize coefficient buffer controller.
 */

void ChJPEG::jinit_d_coef_controller (j_decompress_ptr cinfo, bool need_full_buffer)
{
  my_coef_ptr coef;
  int  i;
  JBLOCKROW buffer;

  coef = new my_coef_controller;
  ASSERT( coef );

  cinfo->coef =  coef;

  /* Create the coefficient buffer. */
  if (need_full_buffer) 
  {
#ifdef D_MULTISCAN_FILES_SUPPORTED
    /* Allocate a full-image virtual array for each component, */
    /* padded to a multiple of samp_factor DCT blocks in each direction. */
    /* Note memmgr implicitly pads the vertical direction. */
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      coef->whole_image[ci] = cinfo->mem->request_virt_barray
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 compptr->height_in_blocks,
	 (JDIMENSION) compptr->v_samp_factor);
    }
#else
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
  } 
  else 
  {
    /* We only need a single-MCU buffer. */
    buffer = (JBLOCKROW)
      ((j_common_ptr)cinfo)->mem->alloc_large((j_common_ptr) cinfo, JPOOL_IMAGE,
				  MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));

    for (i = 0; i < MAX_BLOCKS_IN_MCU; i++) {
      coef->MCU_buffer[i] = buffer + i;
    }
    coef->whole_image[0] = NULL; /* flag for no virtual arrays */
  }
}
