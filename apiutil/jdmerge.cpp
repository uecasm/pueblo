/*
 * jdmerge.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains code for merged upsampling/color conversion.
 *
 * This file combines functions from jdsample.c and jdcolor.c;
 * read those files first to understand what's going on.
 *
 * When the chroma components are to be upsampled by simple replication
 * (ie, box filtering), we can save some work in color conversion by
 * calculating all the output pixels corresponding to a pair of chroma
 * samples at one time.  In the conversion equations
 *	R = Y           + K1 * Cr
 *	G = Y + K2 * Cb + K3 * Cr
 *	B = Y + K4 * Cb
 * only the Y term varies among the group of pixels corresponding to a pair
 * of chroma samples, so the rest of the terms can be calculated just once.
 * At typical sampling ratios, this eliminates half or three-quarters of the
 * multiplications needed for color conversion.
 *
 * This file currently provides implementations for the following cases:
 *	YCbCr => RGB color conversion only.
 *	Sampling ratios of 2h1v or 2h2v.
 *	No scaling needed at upsample time.
 *	Corner-aligned (non-CCIR601) sampling alignment.
 * Other special cases could be added, but in most applications these are
 * the only common cases.  (For uncommon cases we fall back on the more
 * general code in jdsample.c and jdcolor.c.)
 */

// $Header$

#include "headers.h"

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#include <ChJpegDecoder.h>

#ifdef UPSAMPLE_MERGING_SUPPORTED


/* Private subobject */

class my_upsampler : public jpeg_upsampler
{
	public :
		enum tagSampleType { typeSamp1, typeSamp2 };
	public :
	   	my_upsampler()	{}
		virtual ~my_upsampler() {}

	  /* Pointer to routine to do actual upsampling/conversion of one row group */
		virtual void upsample(j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail);
		void start_pass (j_decompress_ptr cinfo);

		int m_ismpFactor;
	 private :
		void upmethod(j_decompress_ptr cinfo,
				   JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
				   JSAMPARRAY output_buf)
		{
			if ( m_ismpFactor == typeSamp2 )
			{
		    	h2v2_merged_upsample(cinfo, input_buf, in_row_group_ctr, output_buf);
			}
			else
			{
		    	h2v1_merged_upsample(cinfo, input_buf, in_row_group_ctr, output_buf);
			}
		}

		void merged_2v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail);
		void h2v2_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf);
		void h2v1_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf);
		void merged_1v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail);

		/* Private state for YCC->RGB conversion */
		int * Cr_r_tab;		/* => table for Cr to R conversion */
		int * Cb_b_tab;		/* => table for Cb to B conversion */
		INT32 * Cr_g_tab;		/* => table for Cr to G conversion */
		INT32 * Cb_g_tab;		/* => table for Cb to G conversion */

		/* For 2:1 vertical sampling, we produce two output rows at a time.
		* We need a "spare" row buffer to hold the second output row if the
		* application provides just a one-row buffer; we also use the spare
		* to discard the dummy last row if the image height is odd.
		*/
		boolean spare_full;		/* T if spare buffer is occupied */
 		JDIMENSION rows_to_go;	/* counts rows remaining in image */

	public :
		JSAMPROW spare_row;
		JDIMENSION out_row_width;	/* samples per output row */
} ;

typedef class my_upsampler * my_upsample_ptr;

#define SCALEBITS	16	/* speediest right-shift on some machines */
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))


/*
 * Initialize for an upsampling pass.
 */

void my_upsampler::start_pass (j_decompress_ptr cinfo)
{
  INT32 i, x;
  SHIFT_TEMPS

  /* Mark the spare buffer empty */
  spare_full = FALSE;
  /* Initialize total-height counter for detecting bottom of image */
  rows_to_go = cinfo->output_height;

  /* Initialize the YCC=>RGB conversion tables.
   * This is taken directly from jdcolor.c; see that file for more info.
   */
  Cr_r_tab = (int *)
    ((j_common_ptr)cinfo)->mem->alloc_small((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  Cb_b_tab = (int *)
    ((j_common_ptr)cinfo)->mem->alloc_small((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(int));
  Cr_g_tab = (INT32 *)
    ((j_common_ptr)cinfo)->mem->alloc_small((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));
  Cb_g_tab = (INT32 *)
    ((j_common_ptr)cinfo)->mem->alloc_small((j_common_ptr) cinfo, JPOOL_IMAGE,
				(MAXJSAMPLE+1) * SIZEOF(INT32));

  for (i = 0, x = -CENTERJSAMPLE; i <= MAXJSAMPLE; i++, x++) {
    /* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
    /* Cr=>R value is nearest int to 1.40200 * x */
    Cr_r_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
    /* Cb=>B value is nearest int to 1.77200 * x */
    Cb_b_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
    /* Cr=>G value is scaled-up -0.71414 * x */
    Cr_g_tab[i] = (- FIX(0.71414)) * x;
    /* Cb=>G value is scaled-up -0.34414 * x */
    /* We also add in ONE_HALF so that need not do it in inner loop */
    Cb_g_tab[i] = (- FIX(0.34414)) * x + ONE_HALF;
  }
}


/*
 * Control routine to do upsampling (and color conversion).
 *
 * The control routine just handles the row buffering considerations.
 */

void my_upsampler::merged_2v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
/* 2:1 vertical sampling case: may need a spare row. */
{
  JSAMPROW work_ptrs[2];
  JDIMENSION num_rows;		/* number of rows returned to caller */

  if (spare_full) {
    /* If we have a spare row saved from a previous cycle, just return it. */
    jcopy_sample_rows(& spare_row, 0, output_buf + *out_row_ctr, 0,
		      1, out_row_width);
    num_rows = 1;
    spare_full = FALSE;
  } else {
    /* Figure number of rows to return to caller. */
    num_rows = 2;
    /* Not more than the distance to the end of the image. */
    if (num_rows > rows_to_go)
      num_rows = rows_to_go;
    /* And not more than what the client can accept: */
    out_rows_avail -= *out_row_ctr;
    if (num_rows > out_rows_avail)
      num_rows = out_rows_avail;
    /* Create output pointer array for upsampler. */
    work_ptrs[0] = output_buf[*out_row_ctr];
    if (num_rows > 1) {
      work_ptrs[1] = output_buf[*out_row_ctr + 1];
    } else {
      work_ptrs[1] = spare_row;
      spare_full = TRUE;
    }
    /* Now do the upsampling. */
   	upmethod(cinfo, input_buf, *in_row_group_ctr, work_ptrs);
  }

  /* Adjust counts */
  *out_row_ctr += num_rows;
  rows_to_go -= num_rows;
  /* When the buffer is emptied, declare this input row group consumed */
  if (! spare_full)
    (*in_row_group_ctr)++;
}


void my_upsampler::merged_1v_upsample (j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
/* 1:1 vertical sampling case: much easier, never need a spare row. */
{

  /* Just do the upsampling. */
  upmethod(cinfo, input_buf, *in_row_group_ctr,
			 output_buf + *out_row_ctr);
  /* Adjust counts */
  (*out_row_ctr)++;
  (*in_row_group_ctr)++;
}


/*
 * These are the routines invoked by the control routines to do
 * the actual upsampling/conversion.  One row group is processed per call.
 *
 * Note: since we may be writing directly into application-supplied buffers,
 * we have to be honest about the output width; we can't assume the buffer
 * has been rounded up to an even width.
 */


/*
 * Upsample and color convert for the case of 2:1 horizontal and 1:1 vertical.
 */

void my_upsampler::h2v1_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf)
{
  register int y, cred, cgreen, cblue;
  int cb, cr;
  register JSAMPROW outptr;
  JSAMPROW inptr0, inptr1, inptr2;
  JDIMENSION col;
  /* copy these pointers into registers if possible */
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  int * Crrtab = Cr_r_tab;
  int * Cbbtab = Cb_b_tab;
  INT32 * Crgtab = Cr_g_tab;
  INT32 * Cbgtab = Cb_g_tab;
  SHIFT_TEMPS

  inptr0 = input_buf[0][in_row_group_ctr];
  inptr1 = input_buf[1][in_row_group_ctr];
  inptr2 = input_buf[2][in_row_group_ctr];
  outptr = output_buf[0];
  /* Loop for each pair of output pixels */
  for (col = cinfo->output_width >> 1; col > 0; col--) {
    /* Do the chroma part of the calculation */
    cb = GETJSAMPLE(*inptr1++);
    cr = GETJSAMPLE(*inptr2++);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    /* Fetch 2 Y values and emit 2 pixels */
    y  = GETJSAMPLE(*inptr0++);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
    outptr += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr0++);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
    outptr += RGB_PIXELSIZE;
  }
  /* If image width is odd, do the last output column separately */
  if (cinfo->output_width & 1) {
    cb = GETJSAMPLE(*inptr1);
    cr = GETJSAMPLE(*inptr2);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    y  = GETJSAMPLE(*inptr0);
    outptr[RGB_RED] =   range_limit[y + cred];
    outptr[RGB_GREEN] = range_limit[y + cgreen];
    outptr[RGB_BLUE] =  range_limit[y + cblue];
  }
}


/*
 * Upsample and color convert for the case of 2:1 horizontal and 2:1 vertical.
 */

void my_upsampler::h2v2_merged_upsample (j_decompress_ptr cinfo,
		      JSAMPIMAGE input_buf, JDIMENSION in_row_group_ctr,
		      JSAMPARRAY output_buf)
{
  register int y, cred, cgreen, cblue;
  int cb, cr;
  register JSAMPROW outptr0, outptr1;
  JSAMPROW inptr00, inptr01, inptr1, inptr2;
  JDIMENSION col;
  /* copy these pointers into registers if possible */
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  int * Crrtab = Cr_r_tab;
  int * Cbbtab = Cb_b_tab;
  INT32 * Crgtab = Cr_g_tab;
  INT32 * Cbgtab = Cb_g_tab;
  SHIFT_TEMPS

  inptr00 = input_buf[0][in_row_group_ctr*2];
  inptr01 = input_buf[0][in_row_group_ctr*2 + 1];
  inptr1 = input_buf[1][in_row_group_ctr];
  inptr2 = input_buf[2][in_row_group_ctr];
  outptr0 = output_buf[0];
  outptr1 = output_buf[1];
  /* Loop for each group of output pixels */
  for (col = cinfo->output_width >> 1; col > 0; col--) {
    /* Do the chroma part of the calculation */
    cb = GETJSAMPLE(*inptr1++);
    cr = GETJSAMPLE(*inptr2++);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    /* Fetch 4 Y values and emit 4 pixels */
    y  = GETJSAMPLE(*inptr00++);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    outptr0 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr00++);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    outptr0 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr01++);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
    outptr1 += RGB_PIXELSIZE;
    y  = GETJSAMPLE(*inptr01++);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
    outptr1 += RGB_PIXELSIZE;
  }
  /* If image width is odd, do the last output column separately */
  if (cinfo->output_width & 1) {
    cb = GETJSAMPLE(*inptr1);
    cr = GETJSAMPLE(*inptr2);
    cred = Crrtab[cr];
    cgreen = (int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS);
    cblue = Cbbtab[cb];
    y  = GETJSAMPLE(*inptr00);
    outptr0[RGB_RED] =   range_limit[y + cred];
    outptr0[RGB_GREEN] = range_limit[y + cgreen];
    outptr0[RGB_BLUE] =  range_limit[y + cblue];
    y  = GETJSAMPLE(*inptr01);
    outptr1[RGB_RED] =   range_limit[y + cred];
    outptr1[RGB_GREEN] = range_limit[y + cgreen];
    outptr1[RGB_BLUE] =  range_limit[y + cblue];
  }
}



void my_upsampler::upsample(j_decompress_ptr cinfo,
		    JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr,
		    JDIMENSION in_row_groups_avail,
		    JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
		    JDIMENSION out_rows_avail)
{
	if ( m_ismpFactor == typeSamp2 )
	{
    	merged_2v_upsample(cinfo, input_buf, in_row_group_ctr,
		    			in_row_groups_avail, output_buf, out_row_ctr,
		    			out_rows_avail);
	}
	else
	{
    	merged_1v_upsample( cinfo, input_buf, in_row_group_ctr,
		    			in_row_groups_avail, output_buf, out_row_ctr,
		    			out_rows_avail);
	}
}



/*
 * Module initialization routine for merged upsampling/color conversion.
 *
 * NB: this is called under the conditions determined by use_merged_upsample()
 * in jdmaster.c.  That routine MUST correspond to the actual capabilities
 * of this module; no safety checks are made here.
 */

void ChJPEG::jinit_merged_upsampler (j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample;

  upsample = new my_upsampler;
  ASSERT( upsample );

  cinfo->upsample = upsample;
  upsample->need_context_rows = FALSE;

  upsample->out_row_width = cinfo->output_width * cinfo->out_color_components;

  if (cinfo->max_v_samp_factor == 2) 
  {
  	upsample->m_ismpFactor = my_upsampler::typeSamp2;
    /* Allocate a spare row buffer */
    upsample->spare_row = (JSAMPROW)
      ((j_common_ptr) cinfo)->mem->alloc_large((j_common_ptr) cinfo, JPOOL_IMAGE,
		(size_t) (upsample->out_row_width * SIZEOF(JSAMPLE)));
  } 
  else 
  {
  	upsample->m_ismpFactor = my_upsampler::typeSamp1;
    /* No spare row needed */
    upsample->spare_row = NULL;
  }
}

#endif /* UPSAMPLE_MERGING_SUPPORTED */
