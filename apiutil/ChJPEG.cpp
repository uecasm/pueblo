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

		 Ultra Enterprises:  Gavin Lambert
		 
		      Stripped out all the jpeglib code so we could link
		      against the more recent library included with libmng.

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

#include <ChTypes.h>

#ifdef CH_UNIX
#include <ChDC.h>
#endif

#include <ChExcept.h>

#include <ChJpegDecoder.h>
#include <ChImgUtil.h>

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif							    


#if defined( CH_MSW )

	#define IOS_BINARY		std::ios::binary

#elif defined( CH_UNIX )

	#define IOS_BINARY		std::ios::bin

#endif	// defined( CH_UNIX )

METHODDEF(void) ChJPEG_error_exit(j_common_ptr cinfo)
{
	jpeg_destroy(cinfo);

	char buffer[1024];
	cinfo->err->format_message(cinfo, buffer);
  
	throw ChJPEGEx(cinfo->err->msg_code, buffer);
}
  
/////////////////////////////////////////////////////////////////////////////////

ChJPEG::ChJPEG( ChImageConsumer* pConsumer ) : ChImageDecoder( pConsumer )
{
	cinfo = ::new jpeg_decompress_struct;
}

ChJPEG::~ChJPEG() 
{
	::delete cinfo;
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
	m_pFile = NULL;

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
		TRACE2( "JPEG load failed : Cause : %d: %s\n", iCause, (LPCSTR)jpegEx.Reason );

		::delete m_pFile;
		m_pFile = NULL;
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
//	ASSERT( m_pFile );
	m_pFile = fopen( pszFileName, "rb" );

	if ( !m_pFile )
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
	  cinfo->err = jpeg_std_error( jerr );
		jerr->error_exit = ChJPEG_error_exit;
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
	  jpeg_create_decompress(cinfo);

	  /* Step 2: specify data source (eg, a file) */

	  jpeg_stdio_src(cinfo, m_pFile );

	  /* Step 3: read file parameters with jpeg_read_header() */

	  (void) jpeg_read_header(cinfo, true );
	  /* We can ignore the return value from jpeg_read_header since
	   *   (a) suspension is not possible with the stdio data source, and
	   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	   * See libjpeg.doc for more info.
	   */

	  /* Step 4: set parameters for decompression */
		SetupParameters();

	  /* Step 5: Start decompressor */

	  jpeg_start_decompress(cinfo);

	  /* We may need to do some setup of our own at this point before reading
	   * the data.  After jpeg_start_decompress() we have the correct scaled
	   * output image dimensions available, as well as the output colormap
	   * if we asked for color quantization.
	   * In this example, we need to make an output work buffer of the right size.
	   */ 
	  /* JSAMPLEs per row in output buffer */
	  row_stride = cinfo->output_width * cinfo->output_components;
	  /* Make a one-row-high sample array that will go away when done with image */
	  buffer = cinfo->mem->alloc_sarray((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, 1);

	  /* Step 6: while (scan lines remain to be read) */
	  /*           jpeg_read_scanlines(...); */

	  /* Here we use the library's state variable cinfo.output_scanline as the
	   * loop counter, so that we don't have to keep track ourselves.
	   */
	  while (cinfo->output_scanline < cinfo->output_height) {
	    (void) jpeg_read_scanlines(cinfo, buffer, 1);
	    /* Assume put_scanline_someplace wants a pointer and sample count. */
	    put_scanline_someplace( buffer[0], row_stride);
	  }

	  /* Step 7: Finish decompression */

	  (void) jpeg_finish_decompress(cinfo);
	  /* We can ignore the return value since suspension is not possible
	   * with the stdio data source.
	   */

	  /* Step 8: Release JPEG decompression object */

	  /* This is an important step since it will release a good deal of memory. */
	  jpeg_destroy_decompress(cinfo);

	  /* After finish_decompress, we can close the input file.
	   * Here we postpone it until after no more JPEG errors are possible,
	   * so as to simplify the setjmp error logic above.  (Actually, I don't
	   * think that jpeg_destroy can do an error exit, but why assume anything...)
	   */
		// close file
		//::delete m_pFile;
		fclose(m_pFile);
		m_pFile = 0;


	  /* At this point you may want to check to see whether any corrupt-data
	   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	   */

	  /* And we're done! */
	  return;
}

void ChJPEG::SetupParameters()
{
  int numColors = 1;

  if ( cinfo->jpeg_color_space != JCS_GRAYSCALE )
  {
	  if ( m_flOptions & load8Bit )
	  {
	  	numColors = 8;
	  }
	  else if ( m_flOptions & load24Bit )
	  {
	  	numColors = -1;
	  }
	  else
	  {
			numColors = ChImgUtil::MaxDeviceColors();
	  }

	  if ( numColors > 256 || numColors < 0 )
	  { // supports true color
	  	cinfo->quantize_colors = false;
	    cinfo->two_pass_quantize = true;
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
			if ( numColors > 256 || numColors < 0 )
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

  if ( cinfo->quantize_colors )
#if defined( CH_MSW )
  {
		RGBQUAD* pClrTbl;	  
	  if ( cinfo->jpeg_color_space == JCS_GRAYSCALE )
		{
		  pClrTbl = ChImgUtil::GetGrayScaleColorTable();
			cinfo->desired_number_of_colors = ChImgUtil::GetGrayScaleClrTblNumEntries();  // Number of color table entries
		} 
		else
		{
 		  pClrTbl = ChImgUtil::GetStdColorTable();
 			cinfo->desired_number_of_colors = ChImgUtil::GetStdClrTblNumEntries();  // Number of color table entries
		}

		if ( cinfo->desired_number_of_colors )
		{
			cinfo->actual_number_of_colors = cinfo->desired_number_of_colors;
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
}

bool ChJPEG::put_scanline_someplace( unsigned char* buffer, int row_stride )
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

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:18  uecasm
// Import of source tree as at version 2.53 release.
//
