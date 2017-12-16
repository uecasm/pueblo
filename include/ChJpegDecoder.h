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

#if !defined( CHJPEGDECODER_H_ )
#define   CHJPEGDECODER_H_

#include <ChImgConsumer.h>
#include <ChImageDecoder.h>

typedef struct jpeg_decompress_struct * j_decompress_ptr;

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

	 	bool put_scanline_someplace( unsigned char* buffer, int row_stride);
		void SetupParameters();

		j_decompress_ptr cinfo;
		FILE * 	m_pFile;
		bool	 	m_boolFirst;
		int 	 	m_iCurrScanLine;

};

#endif // CHJPEGDECODER_H_
