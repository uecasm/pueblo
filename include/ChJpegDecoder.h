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

	Chaco JPEG decoder interface

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CHJPEGDECODER_H_ )
#define   CHJPEGDECODER_H_

#include <ChImgConsumer.h>
#include <ChImageDecoder.h>

#include <fstream>
#include <jpeglib.h>

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


//class fstream;

class CH_EXPORT_CLASS ChJPEG : public ChImageDecoder
{
	public:
	    ChJPEG( ChImageConsumer* pConsumer );
	    virtual ~ChJPEG();
	    virtual bool Load(const char* pszFileName = NULL, 
	    				chuint32 flOption = ChImageDecoder::loadAuto );			// Load JPEG from disk file
	    virtual bool Load(WORD wResid, HINSTANCE hInst = 0); // Load JPEG from resource
	    virtual bool Load(LZHANDLE lzHdl);             // Load JPEG from LZ File	

	private :
		void  LoadJPEG( const char* pszFileName, chuint32 flOptions );
		// methods from jdapi.cpp
		void jpeg_create_decompress (j_decompress_ptr cinfo);
		void jpeg_destroy_decompress (j_decompress_ptr cinfo);
		void jpeg_set_marker_processor (j_decompress_ptr cinfo, int marker_code,
					   jpeg_marker_parser_method routine);
		void default_decompress_parms (j_decompress_ptr cinfo);
		int  jpeg_read_header (j_decompress_ptr cinfo, bool require_image);
		void jpeg_start_decompress (j_decompress_ptr cinfo);
		JDIMENSION jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines,
				     JDIMENSION max_lines);
		JDIMENSION jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data,
				    JDIMENSION max_lines);
		bool jpeg_finish_decompress (j_decompress_ptr cinfo);
		void jpeg_abort_decompress (j_decompress_ptr cinfo);	  

		// method in jdatasrc.cpp
		void jpeg_stdio_src (j_decompress_ptr cinfo, std::fstream * infile);

		public :
		// method in jdcoefct.cpp
		static void jinit_d_coef_controller (j_decompress_ptr cinfo, bool need_full_buffer);

		// method in jdcolor.cpp
		static void jinit_color_deconverter (j_decompress_ptr cinfo);
		// method in jddctmgr.cpp
		static void jinit_inverse_dct (j_decompress_ptr cinfo);
		// method in jdhuff.cpp
		static void jinit_huff_decoder (j_decompress_ptr cinfo);
		// method in jdmainct.cpp
		static void jinit_d_main_controller (j_decompress_ptr cinfo, bool need_full_buffer);
		// method in jdmarker.cpp
		static bool jpeg_resync_to_restart (j_decompress_ptr cinfo);
		static void jinit_marker_reader (j_decompress_ptr cinfo);
		// method in jdmaster.cpp
		static void jpeg_calc_output_dimensions (j_decompress_ptr cinfo);
		// method in jdmerge.cpp
		static void jinit_merged_upsampler (j_decompress_ptr cinfo);
		// method in jdpostct.cpp
		static void jinit_d_post_controller (j_decompress_ptr cinfo, bool need_full_buffer);
		// method in jdsample.cpp
		static void jinit_upsampler (j_decompress_ptr cinfo);
		// method in jderror.cpp
		jpeg_error_mgr *jpeg_std_error (jpeg_error_mgr * err);
		// method in jfdctint.cpp
		//void jpeg_fdct_islow (DCTELEM * data);
		// method in jfdmaster.cpp
		static void jinit_master_decompress (j_decompress_ptr cinfo);

		//
		static void jinit_1pass_quantizer (j_decompress_ptr cinfo);
		static void jinit_2pass_quantizer (j_decompress_ptr cinfo);


		static void jinit_memory_mgr( j_common_ptr cinfo);
		static long jpeg_mem_init( j_common_ptr cinfo); 
		static void   jpeg_mem_term(j_common_ptr cinfo);		/* system-dependent cleanup */


		static void jpeg_destroy( j_common_ptr cinfo); /* use common routine */
		static void jpeg_abort( j_common_ptr cinfo); /* use common routine */

		private :




	 	bool put_scanline_someplace( j_decompress_ptr cinfo, unsigned char* buffer, int row_stride);


		jpeg_decompress_struct cinfo;
		std::fstream * 	m_pFile;
		bool	 	m_boolFirst;
		int 	 	m_iCurrScanLine;

};

// $Log$

#endif // CHJPEGDECODER_H_
