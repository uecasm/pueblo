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

	This file contains the implementation for the PNG/MNG image decoder.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChTypes.h>

#ifdef CH_UNIX
#include <ChDC.h>
#endif
#include "ChMngDecoder.h"
#include <ChMngImage.h>

#include <fstream>
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif			

static mng_ptr MNG_DECL mymng_alloc(mng_size_t iSize) {
  return calloc(1, iSize);
}

static void MNG_DECL mymng_free(mng_ptr pPtr, mng_size_t iSize) {
  free(pPtr);
}

ChMngDecoder::ChMngDecoder( ChImageConsumer* pConsumer, bool supportsTransparency ) : ChImageDecoder( pConsumer )
{
	m_stream = NULL;
	m_TransparencySupport = supportsTransparency;
	m_handle = mng_initialize(this, mymng_alloc, mymng_free, MNG_NULL);
	
	if (m_handle != NULL)
	{
		mng_setcb_openstream(m_handle, openstream);
		mng_setcb_readdata(m_handle, readdata);
		mng_setcb_closestream(m_handle, closestream);
		mng_setcb_processheader(m_handle, processheader);
		mng_setcb_gettickcount(m_handle, gettickcount);
		mng_setcb_getcanvasline(m_handle, getcanvasline);
		mng_setcb_refresh(m_handle, refresh);
		mng_setcb_settimer(m_handle, settimer);

		if(m_TransparencySupport == false)
		{
			mng_setcb_getbkgdline(m_handle, getbkgdline);
		}
	}
}

ChMngDecoder::~ChMngDecoder()
{
	if (m_handle != NULL)
	{
		mng_cleanup(&m_handle);
	}
	delete m_stream;
}

bool ChMngDecoder::Load(WORD wResid, HINSTANCE hInst)
{
	ASSERT( false );
	return false;
}

bool ChMngDecoder::Load(LZHANDLE lzHdl)
{
	ASSERT( false );
	return false;
}

bool ChMngDecoder::Load( const char *pszFileName, chuint32 flOptions /* = ChDIB::loadAuto */ )
{
	if (!m_handle) { return false; }

	// for paranoia's sake, we'll clean up beforehand
	mng_retcode result = mng_reset(m_handle);
	if (result != MNG_NOERROR)
	{
		ASSERT( false );
		return false;
	}

	delete m_stream;	// just in case!
	m_stream = new std::ifstream(pszFileName, std::ios::in | std::ios::binary);
	ASSERT(m_stream);
	if (!m_stream->good()) { return false; }

	result = mng_read(m_handle);
	if (result == MNG_NOERROR) { return true; }

	delete m_stream;
	m_stream = NULL;

	ASSERT( false );
	return false;
}

bool ChMngDecoder::FirstFrame()
{
	mng_retcode result = mng_display(m_handle);
	return (result == MNG_NEEDTIMERWAIT);
}

bool ChMngDecoder::NextFrame()
{
	mng_retcode result = mng_display_resume(m_handle);
	return (result == MNG_NEEDTIMERWAIT);
}

void ChMngDecoder::Reset()
{
	// necessary for example when the background changes; will reset the animation
	// back to the first frame and force it to requery the background canvas.
	mng_retcode result = mng_display_freeze(m_handle);
	result = mng_display_reset(m_handle);
}

/*static*/ mng_bool ChMngDecoder::openstream(mng_handle hHandle)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);

	return ((decoder->m_stream == NULL) ? MNG_FALSE : MNG_TRUE);
}

/*static*/ mng_bool ChMngDecoder::closestream(mng_handle hHandle)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);

	delete decoder->m_stream;
	decoder->m_stream = NULL;

	return MNG_TRUE;
}

/*static*/ mng_bool ChMngDecoder::readdata(mng_handle hHandle, mng_ptr pBuf, mng_uint32 iBuflen, mng_uint32p pRead)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);

	if(decoder->m_stream == NULL) { return MNG_FALSE; }

	decoder->m_stream->read((char *)pBuf, iBuflen);
	*pRead = decoder->m_stream->gcount();

	return MNG_TRUE;
}

/*static*/ mng_bool ChMngDecoder::processheader(mng_handle hHandle, mng_uint32 iWidth, mng_uint32 iHeight)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);

	ChImageInfo imageInfo;
	ChMemClearStruct( &imageInfo );
	imageInfo.iWidth = iWidth;
	imageInfo.iHeight = iHeight;
	imageInfo.boolMultiframe = true;	// all MNGs are multiframe until proven otherwise

	decoder->GetConsumer()->NewImage(&imageInfo);

	if(decoder->m_TransparencySupport)
	{
		// The Windows alpha bitmap format is BBGGRRAA, with premultiplied alpha.
		mng_set_canvasstyle(hHandle, MNG_CANVAS_BGRA8PM);
	}
	else
	{
		// When we're getting libmng to composite it itself, we're still creating
		// a primary 32-bit image; but there's a spare byte now.  We also need to
		// specify the background format; typically this will be 24-bit BBGGRR;
		// there's a chance it might go 8-bit on 256-colour systems, but I really
		// hope not.  We'll see.
		mng_set_canvasstyle(hHandle, MNG_CANVAS_BGRX8);
		mng_set_bkgdstyle(hHandle, MNG_CANVAS_BGR8);
	}
	
	return MNG_TRUE;
}

/*static*/ mng_uint32 ChMngDecoder::gettickcount(mng_handle hHandle)
{
	return ::GetTickCount();
}

/*static*/ mng_ptr ChMngDecoder::getcanvasline(mng_handle hHandle, mng_uint32 iLinenr)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);
	ChMngImage *image = (ChMngImage *)decoder->GetConsumer();

	return image->GetCanvasLine(iLinenr);
}

/*static*/ mng_ptr ChMngDecoder::getbkgdline(mng_handle hHandle, mng_uint32 iLinenr)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);
	ChMngImage *image = (ChMngImage *)decoder->GetConsumer();

	return image->GetBackgroundLine(iLinenr);
}

/*static*/ mng_bool ChMngDecoder::refresh(mng_handle hHandle, mng_uint32 iX, mng_uint32 iY, mng_uint32 iWidth, mng_uint32 iHeight)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);
	ChMngImage *image = (ChMngImage *)decoder->GetConsumer();

	image->UpdateFrame(iX, iY, iWidth, iHeight);
	return MNG_TRUE;
}

/*static*/ mng_bool ChMngDecoder::settimer(mng_handle hHandle, mng_uint32 iMsecs)
{
	ChMngDecoder *decoder = (ChMngDecoder *)mng_get_userdata(hHandle);
	ChMngImage *image = (ChMngImage *)decoder->GetConsumer();

	image->SetFrameDelay(iMsecs);
	return MNG_TRUE;
}

// $Log$
