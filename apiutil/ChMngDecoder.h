/*----------------------------------------------------------------------------
  _   _ _  _                 ____        _                     _
 | | | | || |               |  __|      | |                   (_)
 | | | | || |_ _ __  __ _   | |_  _ __ _| |_ ___ _ ___ __ _ __ _ ___  ___ ___
 | | | | |_  _| '__|/ _` |  |  _|| '_ \_   _| _ \ '__|'_ \ '__| | __|/ _ \ __|
 | '-' | || | | |  | (_| |  | |__| | | || ||  __/ | | |_) ||  | |__ |  __/__ |
  \___/|_||_| |_|   \__,_|  |____|_| |_||_| \___|_| | .__/_|  |_|___|\___|___|
                                                    | |     
                                                    |_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo/UE client code, first released April 1, 2002.

    The Initial Developer of the Original Code is Ultra Enterprises.

    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Created this class.

------------------------------------------------------------------------------

	This file contains the interface for the PNG/MNG image decoder.

----------------------------------------------------------------------------*/

#if !defined( CHMNGDECODER_H_ )
#define   CHMNGDECODER_H_

#include <iostream>

//#define MNG_USE_DLL
//#define MNG_SKIP_LCMS		// it's all internal stuff anyway, we don't need it out here
#undef HUGE
#include <libmng.h>

#include <ChImgConsumer.h>
#include "ChImageDecoder.h"

typedef struct mng_data_struct *mng_handle;     // generic LIBMNG handle

class ChMngDecoder : public ChImageDecoder
{
	public:
	  ChMngDecoder( ChImageConsumer* pConsumer, bool supportsTransparency );
	  virtual ~ChMngDecoder();
	  virtual bool Load( const char* pszFileName = NULL, 
	    				chuint32 flOption = loadAuto );		// Load MNG from disk file

		// the following two methods are not implemented and will always fail:
	  virtual bool Load(WORD wResid, HINSTANCE hInst = 0); // Load MNG from resource
	  virtual bool Load(LZHANDLE lzHdl);             // Load MNG from LZ File	

		bool FirstFrame();
		bool NextFrame();
		void Reset();

	private:
		mng_handle m_handle;
		std::istream *m_stream;
		bool m_TransparencySupport;

		static mng_bool MNG_DECL openstream(mng_handle hHandle);
		static mng_bool MNG_DECL readdata(mng_handle hHandle, mng_ptr pBuf, mng_uint32 iBuflen, mng_uint32p pRead);
		static mng_bool MNG_DECL closestream(mng_handle hHandle);
		static mng_bool MNG_DECL processheader(mng_handle hHandle, mng_uint32 iWidth, mng_uint32 iHeight);
		static mng_uint32 MNG_DECL gettickcount(mng_handle hHandle);
		static mng_ptr MNG_DECL getcanvasline(mng_handle hHandle, mng_uint32 iLinenr);
		static mng_ptr MNG_DECL getbkgdline(mng_handle hHandle, mng_uint32 iLinenr);
		static mng_bool MNG_DECL refresh(mng_handle hHandle, mng_uint32 iX, mng_uint32 iY, mng_uint32 iWidth, mng_uint32 iHeight);
		static mng_bool MNG_DECL settimer(mng_handle hHandle, mng_uint32 iMsecs);
};

#endif // defined(CHMNGDECODER_H_
