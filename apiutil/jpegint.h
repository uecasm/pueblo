/*
 * jpegint.h
 *
 * Copyright (C) 1991-1995, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides common declarations for the various JPEG modules.
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */

// $Header$

/* Declarations for both compression & decompression */

typedef enum {			/* Operating modes for buffer controllers */
	JBUF_PASS_THRU,		/* Plain stripwise operation */
	JBUF_CRANK_SOURCE,	/* Run source subobject, no output expected */
	/* Remaining modes require a full-image buffer to have been created */
	JBUF_SAVE_SOURCE,	/* Run source subobject only, save output */
	JBUF_CRANK_DEST,	/* Run dest subobject only, using saved data */
	JBUF_SAVE_AND_PASS	/* Run both subobjects, save output */
} J_BUF_MODE;

/* Values of global_state field */
#define CSTATE_START	100	/* after create_compress */
#define CSTATE_SCANNING	101	/* start_compress done, write_scanlines OK */
#define CSTATE_RAW_OK	102	/* start_compress done, write_raw_data OK */
#define DSTATE_START	200	/* after create_decompress */
#define DSTATE_INHEADER	201	/* read_header initialized but not done */
#define DSTATE_READY	202	/* read_header done, found image */
#define DSTATE_SCANNING	203	/* start_decompress done, read_scanlines OK */
#define DSTATE_RAW_OK	204	/* start_decompress done, read_raw_data OK */
#define DSTATE_STOPPING	205	/* done reading data, looking for EOI */



/* Declarations for decompression modules */

/* Master control module */
class  jpeg_decomp_master 
{
	public :
		jpeg_decomp_master() {}
		virtual ~jpeg_decomp_master()	{}

		virtual void prepare_for_pass(j_decompress_ptr cinfo) = 0;
		virtual void finish_pass(j_decompress_ptr cinfo) = 0;

		/* State variables made visible to other modules */
		bool is_last_pass;		/* True during last pass */
		bool eoi_processed;	/* True if EOI marker already read */
};

/* Main buffer control (downsampled-data buffer) */
class jpeg_d_main_controller 
{
	public :
		jpeg_d_main_controller() {}
		virtual ~jpeg_d_main_controller() {}

		virtual void start_pass( j_decompress_ptr cinfo, J_BUF_MODE pass_mode) = 0;
		virtual void process_data( j_decompress_ptr cinfo,
				       JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
				       JDIMENSION out_rows_avail) = 0;
		/* During input-only passes, output_buf and out_rows_avail are ignored.
		* out_row_ctr is incremented towards the limit num_chunks.
		*/
		JDIMENSION num_chunks;	/* number of chunks to be processed in pass */
};

/* Coefficient buffer control */
class jpeg_d_coef_controller 
{
	public :
		jpeg_d_coef_controller()	{}
		virtual ~jpeg_d_coef_controller()	{}	
  		virtual void start_pass(j_decompress_ptr cinfo, J_BUF_MODE pass_mode) = 0;
  		virtual bool decompress_data(j_decompress_ptr cinfo,
				     JSAMPIMAGE output_buf) = 0;
};

/* Decompression postprocessing (color quantization buffer control) */
class  jpeg_d_post_controller 
{
	public :
		jpeg_d_post_controller()	{}
		virtual ~jpeg_d_post_controller() {}

		virtual void start_pass (j_decompress_ptr cinfo, J_BUF_MODE pass_mode) = 0;
		virtual void post_process_data(j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf,
				    JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf,
				    JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail) = 0;
};

/* Marker reading & parsing */
class  jpeg_marker_reader 
{
	public :
		jpeg_marker_reader() {}
		virtual ~jpeg_marker_reader()	{}
	
		virtual void reset_marker_reader(j_decompress_ptr cinfo);
		/* Read markers until SOS or EOI.
		* Returns same codes as are defined for jpeg_read_header,
		* but HEADER_OK and HEADER_TABLES_ONLY merely indicate which marker type
		* stopped the scan --- further validation is needed to declare file OK.
		*/
		virtual int read_markers( j_decompress_ptr cinfo );
		/* Read a restart marker --- exported for use by entropy decoder only */
		virtual bool read_restart_marker (j_decompress_ptr cinfo);
	
		static bool skip_variable (j_decompress_ptr cinfo);
		static bool get_app0(j_decompress_ptr cinfo);
		static bool get_app14(j_decompress_ptr cinfo);



		//jpeg_marker_parser_method read_restart_marker;
		/* Application-overridable marker processing methods */
		jpeg_marker_parser_method process_COM;
		jpeg_marker_parser_method process_APPn[16];
		
		//jpeg_marker_parser_method process_COM;
		//jpeg_marker_parser_method process_APPn[16];

		/* State of marker reader --- nominally internal, but applications
		* supplying COM or APPn handlers might like to know the state.
		*/
		bool saw_SOI;		/* found SOI? */
		bool saw_SOF;		/* found SOF? */
		int next_restart_num;		/* next restart number expected (0-7) */
		unsigned int discarded_bytes;	/* # of bytes skipped looking for a marker */
};

/* Entropy decoding */
class jpeg_entropy_decoder 
{
	public :
		jpeg_entropy_decoder()	{}
		virtual ~jpeg_entropy_decoder() {}

		virtual void start_pass(j_decompress_ptr cinfo) = 0;
		virtual bool decode_mcu(j_decompress_ptr cinfo,
					JBLOCKROW *MCU_data) = 0;
};

/* Inverse DCT (also performs dequantization) */
CH_TYPEDEF_CALLBACK( void, inverse_DCT_method_ptr )
		(j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col );

class  jpeg_inverse_dct 
{
	public :
		jpeg_inverse_dct(){}
		virtual ~jpeg_inverse_dct()	{}

		virtual void start_input_pass( j_decompress_ptr cinfo) = 0;
		virtual void start_output_pass (j_decompress_ptr cinfo) = 0;
		/* It is useful to allow each component to have a separate IDCT method. */
		inverse_DCT_method_ptr inverse_DCT[MAX_COMPONENTS];
};

/* Upsampling (note that upsampler must also call color converter) */
class  jpeg_upsampler 
{
	public :
		jpeg_upsampler()	{}
		virtual ~jpeg_upsampler() {}

		virtual void start_pass(j_decompress_ptr cinfo) = 0;
		virtual void upsample(j_decompress_ptr cinfo,
					   JSAMPIMAGE input_buf,
					   JDIMENSION *in_row_group_ctr,
					   JDIMENSION in_row_groups_avail,
					   JSAMPARRAY output_buf,
					   JDIMENSION *out_row_ctr,
					   JDIMENSION out_rows_avail) = 0;

  		bool need_context_rows;	/* TRUE if need rows above & below */
};

/* Colorspace conversion */
class jpeg_color_deconverter 
{
	public :
		jpeg_color_deconverter() {}
		virtual ~jpeg_color_deconverter(){}

		virtual void start_pass( j_decompress_ptr cinfo) = 0;
		virtual void color_convert(j_decompress_ptr cinfo,
					JSAMPIMAGE input_buf, JDIMENSION input_row,
					JSAMPARRAY output_buf, int num_rows) = 0;
};

CH_TYPEDEF_FUNC( void, pprocColorQuantize )
				( j_decompress_ptr cinfo,
					 JSAMPARRAY input_buf, JSAMPARRAY output_buf,
					 int num_rows );
CH_TYPEDEF_FUNC( void, pprocFinishQuantize )
				( j_decompress_ptr cinfo );



/* Color quantization or color precision reduction */
class jpeg_color_quantizer 
{
	public :
		jpeg_color_quantizer() {}
		virtual ~jpeg_color_quantizer(){}
		virtual void start_pass(j_decompress_ptr cinfo, bool is_pre_scan) = 0;
		pprocColorQuantize  color_quantize;
		pprocFinishQuantize finish_pass;
};


/* Miscellaneous useful macros */

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))


/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an INT32 quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS	INT32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif


CH_EXTERN_FUNC( long )
jdiv_round_up(long a, long b);
CH_EXTERN_FUNC( long )
jround_up(long a, long b);

CH_EXTERN_FUNC( void )
jcopy_sample_rows( JSAMPARRAY input_array, int source_row,
				   JSAMPARRAY output_array, int dest_row,
				   int num_rows, JDIMENSION num_cols);
CH_EXTERN_FUNC( void )
jcopy_block_row(JBLOCKROW input_row, JBLOCKROW output_row,
				 JDIMENSION num_blocks);
CH_EXTERN_FUNC( void )
jzero_far(void FAR * target, size_t bytestozero);



/* Short forms of external names for systems with brain-damaged linkers. */


#if 0
/* Compression module initialization routines */
EXTERN void jinit_master_compress JPP((j_compress_ptr cinfo));
EXTERN void jinit_c_main_controller JPP((j_compress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_c_prep_controller JPP((j_compress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_c_coef_controller JPP((j_compress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_color_converter JPP((j_compress_ptr cinfo));
EXTERN void jinit_downsampler JPP((j_compress_ptr cinfo));
EXTERN void jinit_forward_dct JPP((j_compress_ptr cinfo));
EXTERN void jinit_huff_encoder JPP((j_compress_ptr cinfo));
EXTERN void jinit_marker_writer JPP((j_compress_ptr cinfo));
/* Decompression module initialization routines */
EXTERN void jinit_master_decompress JPP((j_decompress_ptr cinfo));
EXTERN void jinit_d_main_controller JPP((j_decompress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_d_coef_controller JPP((j_decompress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_d_post_controller JPP((j_decompress_ptr cinfo,
					 bool need_full_buffer));
EXTERN void jinit_marker_reader JPP((j_decompress_ptr cinfo));
EXTERN void jinit_huff_decoder JPP((j_decompress_ptr cinfo));
EXTERN void jinit_inverse_dct JPP((j_decompress_ptr cinfo));
EXTERN void jinit_upsampler JPP((j_decompress_ptr cinfo));
EXTERN void jinit_color_deconverter JPP((j_decompress_ptr cinfo));
EXTERN void jinit_1pass_quantizer JPP((j_decompress_ptr cinfo));
EXTERN void jinit_2pass_quantizer JPP((j_decompress_ptr cinfo));
EXTERN void jinit_merged_upsampler JPP((j_decompress_ptr cinfo));
/* Memory manager initialization */
EXTERN void jinit_memory_mgr JPP((j_common_ptr cinfo));

/* Utility routines in jutils.c */
EXTERN long jdiv_round_up JPP((long a, long b));
EXTERN long jround_up JPP((long a, long b));
EXTERN void jcopy_sample_rows JPP((JSAMPARRAY input_array, int source_row,
				   JSAMPARRAY output_array, int dest_row,
				   int num_rows, JDIMENSION num_cols));
EXTERN void jcopy_block_row JPP((JBLOCKROW input_row, JBLOCKROW output_row,
				 JDIMENSION num_blocks));
EXTERN void jzero_far JPP((void FAR * target, size_t bytestozero));
#endif


/* Suppress undefined-structure complaints if necessary. */

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef AM_MEMORY_MANAGER	/* only jmemmgr.c defines these */
struct jvirt_sarray_control { long dummy; };
struct jvirt_barray_control { long dummy; };
#endif
#endif /* INCOMPLETE_TYPES_BROKEN */
