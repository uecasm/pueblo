/*
 * jdpostct.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the decompression postprocessing controller.
 * This controller manages the upsampling, color conversion, and color
 * quantization/reduction steps; specifically, it controls the buffering
 * between upsample/color conversion and color quantization/reduction.
 *
 * If no color quantization/reduction is required, then this module has no
 * work to do, and it just hands off to the upsample/color conversion code.
 * An integrated upsample/convert/quantize process would replace this module
 * entirely.
 */

// $Header$

#include "headers.h"

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#include <ChJpegDecoder.h>

/* Private buffer controller object */

class  my_post_controller :public jpeg_d_post_controller 
{
	public :
		enum tagPostType { type1Pass, type2Pass, typePrePass, typeUpSample };
	public :
		my_post_controller( j_decompress_ptr cinfo, bool need_full_buffer );
		virtual ~my_post_controller() {}

		virtual void start_pass (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
		virtual void post_process_data(j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf,
				    JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf,
				    JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail);
	private :
		void post_process_1pass (j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail);
		#ifdef QUANT_2PASS_SUPPORTED

		void post_process_prepass (j_decompress_ptr cinfo,
				      JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
				      JDIMENSION in_row_groups_avail,
				      JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
				      JDIMENSION out_rows_avail);
		void post_process_2pass (j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail);
		#endif
		int m_iPostProcess;
		/* Color quantization source buffer: this holds output data from
		* the upsample/color conversion step to be passed to the quantizer.
		* For two-pass color quantization, we need a full-image buffer;
		* for one-pass operation, a strip buffer is sufficient.
		*/
		jvirt_sarray_ptr whole_image;	/* virtual array, or NULL if one-pass */
		JSAMPARRAY buffer;		/* strip buffer, or current strip of virtual */
		JDIMENSION strip_height;	/* buffer size in rows */
		/* for two-pass mode only: */
		JDIMENSION starting_row;	/* row # of first row in current strip */
		JDIMENSION next_row;		/* index of next row to fill/empty in strip */
};

typedef class my_post_controller * my_post_ptr;



/*
 * Initialize for a processing pass.
 */

void my_post_controller::start_pass (j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (cinfo->quantize_colors) 
    {
      /* Single-pass processing with color quantization. */
	  	m_iPostProcess = type1Pass;
    } 
    else 
    {
      /* For single-pass processing without color quantization,
       * I have no work to do; just call the upsampler directly.
       */
 	  	m_iPostProcess = typeUpSample;
    }
    break;
#ifdef QUANT_2PASS_SUPPORTED
  case JBUF_SAVE_AND_PASS:
    /* First pass of 2-pass quantization */
    if (whole_image == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
	  m_iPostProcess = typePrePass;
    break;
  case JBUF_CRANK_DEST:
    /* Second pass of 2-pass quantization */
    if (whole_image == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
  	m_iPostProcess = type2Pass;
    break;
#endif /* QUANT_2PASS_SUPPORTED */
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
  starting_row = next_row = 0;
}

void my_post_controller::post_process_data(j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf,
				    JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf,
				    JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail)
{
	switch ( m_iPostProcess )
	{
		case type1Pass :
		{
			post_process_1pass( cinfo, input_buf,	in_row_group_ctr,
							  in_row_groups_avail, output_buf,
							  out_row_ctr, out_rows_avail);	
			break;
		}
#ifdef QUANT_2PASS_SUPPORTED
		case type2Pass :
		{
			post_process_2pass( cinfo, input_buf,	in_row_group_ctr,
							  in_row_groups_avail, output_buf,
							  out_row_ctr, out_rows_avail);	
			break;
		}
		case typePrePass :
		{
			post_process_prepass( cinfo, input_buf,	in_row_group_ctr,
							  in_row_groups_avail, output_buf,
							  out_row_ctr, out_rows_avail);	
			break;
		}
#endif
		case typeUpSample :
		{
			cinfo->upsample->upsample( cinfo, input_buf,	in_row_group_ctr,
							  in_row_groups_avail, output_buf,
							  out_row_ctr, out_rows_avail);	
			break;
		}
		default :
		{
			break;
		}
		
	}
}



/*
 * Process some data in the one-pass (strip buffer) case.
 * This is used for color precision reduction as well as one-pass quantization.
 */

void my_post_controller::post_process_1pass (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
{
  JDIMENSION num_rows, max_rows;

  /* Fill the buffer, but not more than what we can dump out in one go. */
  /* Note we rely on the upsampler to detect bottom of image. */
  max_rows = out_rows_avail - *out_row_ctr;
  if (max_rows > strip_height)
    max_rows = strip_height;
  num_rows = 0;
  cinfo->upsample->upsample(cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		buffer, &num_rows, max_rows);
  /* Quantize and emit data. */
  cinfo->cquantize->color_quantize(cinfo,
		buffer, output_buf + *out_row_ctr, (int) num_rows);
  *out_row_ctr += num_rows;
}


#ifdef QUANT_2PASS_SUPPORTED

/*
 * Process some data in the first pass of 2-pass quantization.
 */

void my_post_controller::post_process_prepass (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		      JDIMENSION in_row_groups_avail,
		      JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		      JDIMENSION out_rows_avail)
{
  JDIMENSION old_next_row, num_rows;

  /* Reposition virtual buffer if at start of strip. */
  if (next_row == 0) 
  {
    buffer = ((j_common_ptr)cinfo)->mem->access_virt_sarray
	((j_common_ptr) cinfo, whole_image, starting_row, TRUE);
  }

  /* Upsample some data (up to a strip height's worth). */
  old_next_row = next_row;
  cinfo->upsample->upsample(cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		buffer, &next_row, strip_height);

  /* Allow quantizer to scan new data.  No data is emitted, */
  /* but we advance out_row_ctr so outer loop can tell when we're done. */
  if (next_row > old_next_row) {
    num_rows = next_row - old_next_row;
    cinfo->cquantize->color_quantize(cinfo, buffer + old_next_row,
					 (JSAMPARRAY) NULL, (int) num_rows);
    *out_row_ctr += num_rows;
  }

  /* Advance if we filled the strip. */
  if (next_row >= strip_height) {
    starting_row += strip_height;
    next_row = 0;
  }
}


/*
 * Process some data in the second pass of 2-pass quantization.
 */

void my_post_controller::post_process_2pass (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
{
  JDIMENSION num_rows, max_rows;

  /* Reposition virtual buffer if at start of strip. */
  if (next_row == 0) {
    buffer = ((j_common_ptr)cinfo)->mem->access_virt_sarray
	((j_common_ptr) cinfo, whole_image, starting_row, FALSE);
  }

  /* Determine number of rows to emit. */
  num_rows = strip_height - next_row; /* available in strip */
  max_rows = out_rows_avail - *out_row_ctr; /* available in output area */
  if (num_rows > max_rows)
    num_rows = max_rows;
  /* We have to check bottom of image here, can't depend on upsampler. */
  max_rows = cinfo->output_height - starting_row;
  if (num_rows > max_rows)
    num_rows = max_rows;

  /* Quantize and emit data. */
  cinfo->cquantize->color_quantize(cinfo,
		buffer + next_row, output_buf + *out_row_ctr,
		(int) num_rows);
  *out_row_ctr += num_rows;

  /* Advance if we filled the strip. */
  next_row += num_rows;
  if (next_row >= strip_height) {
    starting_row += strip_height;
    next_row = 0;
  }
}

#endif /* QUANT_2PASS_SUPPORTED */

my_post_controller::my_post_controller( j_decompress_ptr cinfo, bool need_full_buffer )
{
  cinfo->post = this;
  
  whole_image = NULL;	/* flag for no virtual arrays */

  /* Create the quantization buffer, if needed */
  if (cinfo->quantize_colors) 
  {
    /* The buffer strip height is max_v_samp_factor, which is typically
     * an efficient number of rows for upsampling to return.
     * (In the presence of output rescaling, we might want to be smarter?)
     */
    strip_height = (JDIMENSION) cinfo->max_v_samp_factor;
    if (need_full_buffer) 
    {
      /* Two-pass color quantization: need full-image storage. */
#ifdef QUANT_2PASS_SUPPORTED
      whole_image = ((j_common_ptr)cinfo)->mem->request_virt_sarray
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 cinfo->output_width * cinfo->out_color_components,
	 cinfo->output_height, strip_height);
#else
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif /* QUANT_2PASS_SUPPORTED */
    } 
    else 
    {
      /* One-pass color quantization: just make a strip buffer. */
      buffer = ((j_common_ptr)cinfo)->mem->alloc_sarray
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 cinfo->output_width * cinfo->out_color_components,
	 strip_height);
    }
  }

}


/*
 * Initialize postprocessing controller.
 */

void ChJPEG::jinit_d_post_controller (j_decompress_ptr cinfo, bool need_full_buffer)
{
  my_post_ptr post;
		  
  post = new my_post_controller( cinfo, need_full_buffer );

  ASSERT( post );

}
