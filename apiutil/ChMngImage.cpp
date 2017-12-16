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

	This file contains the implementation for the PNG/MNG image object.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChTypes.h>

#ifdef CH_UNIX
#include <ChDC.h>
#endif
#include <ChMngImage.h>
#include "ChMngDecoder.h"

#include <fstream>
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif			

ChMngImage::ChMngImage() : ChDib()
{
	m_AlphaBlend = NULL;
	m_ImageLibrary = LoadLibrary("Msimg32.dll");
	if(m_ImageLibrary != NULL)
	{
		m_AlphaBlend = (AlphaBlendProc)GetProcAddress(m_ImageLibrary, "AlphaBlend");
	}

	m_decoder = new ChMngDecoder(this, SupportsAlphaBlending());
}

ChMngImage::~ChMngImage()
{
	Finished();

	if(m_ImageLibrary != NULL)
	{
		FreeLibrary(m_ImageLibrary);
		m_ImageLibrary = NULL;
	}
}

void ChMngImage::Finished()
{
	m_imgInfo.boolMultiframe = false;
	delete m_decoder;
	m_decoder = NULL;
	//TRACE0("MngImage: Finished\n");
}

bool ChMngImage::SupportsAlphaBlending()
{
	return (m_AlphaBlend != NULL);
}

void ChMngImage::ResetAnimation()
{
	if(m_decoder != NULL)
	{
		m_decoder->Reset();
		m_ShownFirstFrame = false;
	}
}

bool ChMngImage::Load( const char* pszFileName )
{
	if (m_decoder == NULL) { return false; }
	if (!m_decoder->Load(pszFileName)) { return false; }

	//TRACE0("MngImage: Loaded\n");
	m_ShownFirstFrame = false;
	return true;
}

/*virtual*/ bool ChMngImage::NewImage( pChImageInfo pImage )
{
	if (ChDib::NewImage(pImage) == false) { return false; }

	// allocate primary drawing canvas
	bool result = Create(0, pImage->iWidth, pImage->iHeight, 32);

	if(result && (SupportsAlphaBlending() == false))
	{
		// mark as transparent so that we can create background data
		m_pFrameList[0].m_frameInfo.luAttrs |= ChImageConsumer::imgTransparent;
		// create an initial background image, with a default colour.  It'll look
		// quite terrible if it is ever drawn, but we can hope it won't be.
		//CClientDC dcDesktop(CWnd::GetDesktopWindow());
		CreateBackgroundImage(NULL, 0, 0, RGB(255, 0, 255));
	}
	return result;
}

/*virtual*/ void ChMngImage::NextFrame()
{
	// The MNG image doesn't have multiple frames in the same manner as GIF;
	// in order to advance to the next frame we have to ask the decoder to
	// update the existing canvas for us.
	bool moreFrames = false;
	if(m_ShownFirstFrame)
	{
		moreFrames = m_decoder->NextFrame();
	}
	else
	{
		moreFrames = m_decoder->FirstFrame();
		m_ShownFirstFrame = true;
	}

	if(moreFrames == false)
	{
		// Couldn't advance, either because of an error or because we're out of frames.
		Finished();
	}
	//TRACE0("MngImage: frame complete\n");
}

/*virtual*/ void ChMngImage::Draw(CDC* pDC, int x, int y)
{
	CDC dcBitmap;
	dcBitmap.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bitmap.CreateBitmap(GetWidth(), GetHeight(), 1, 32, GetBitsAddress());
	BITMAP info;
	bitmap.GetBitmap(&info);

	CBitmap *oldBitmap = dcBitmap.SelectObject(&bitmap);

	if(SupportsAlphaBlending())
	{
		BLENDFUNCTION blendFn;
		blendFn.AlphaFormat = AC_SRC_ALPHA;
		blendFn.BlendFlags = 0;
		blendFn.BlendOp = AC_SRC_OVER;
		blendFn.SourceConstantAlpha = 255;

		m_AlphaBlend(pDC->GetSafeHdc(), x, y, m_imgInfo.iWidth, m_imgInfo.iHeight,
					dcBitmap.GetSafeHdc(), 0, 0, m_imgInfo.iWidth, m_imgInfo.iHeight, blendFn);
	}
	else
	{
		// hopefully libmng has composited the image for us, so all we need to do is
		// draw it without any transparency.
		pDC->BitBlt(x, y, m_imgInfo.iWidth, m_imgInfo.iHeight, &dcBitmap, 0, 0, SRCCOPY);
	}

	dcBitmap.SelectObject(oldBitmap);
	bitmap.DeleteObject();
	dcBitmap.DeleteDC();
}

/*virtual*/ void ChMngImage::Draw(CDC* pDC, int x, int y, COLORREF clrTrans )
{
	Draw(pDC, x, y);
}

/*virtual*/ void ChMngImage::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, COLORREF  clrMask )
{
	if ((m_pFrameList[0].m_pTransDib == NULL) ||
			(clrMask != (COLORREF)m_pFrameList[0].m_pMask))
	{
		CreateBackgroundImage(pDC, x, y, clrMask);
		ResetAnimation();
	}

	Draw(pDC, x, y);
}

/*virtual*/ void ChMngImage::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, CBrush*  pbrMask )
{
	if ((m_pFrameList[0].m_pTransDib == NULL) ||
			(pbrMask->GetSafeHandle() != m_pFrameList[0].m_pMask))
	{
		CreateBackgroundImage(pDC, x, y, pbrMask);
		ResetAnimation();
	}

	Draw(pDC, x, y);
}

/*virtual*/ void ChMngImage::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, ChDib* pdibMask )
{
	if ((m_pFrameList[0].m_pTransDib == NULL) ||
			(pdibMask != m_pFrameList[0].m_pMask))
	{
		CreateBackgroundImage(pDC, x, y, pdibMask);
		ResetAnimation();
	}

	Draw(pDC, x, y);
}

void *ChMngImage::GetCanvasLine(DWORD y)
{
	return (m_pFrameList[0].m_pBits + (y * StorageWidth(0)));
}

void *ChMngImage::GetBackgroundLine(DWORD y)
{
	ChDib *pTransDib = m_pFrameList[0].m_pTransDib;
	if (pTransDib != NULL)
	{
		LPBYTE pScanLine = ((LPBYTE)pTransDib->GetBitsAddress() + (y * pTransDib->StorageWidth(0)));
		return pScanLine;
	}
	return NULL;
}

void ChMngImage::UpdateFrame(DWORD x, DWORD y, DWORD cx, DWORD cy)
{
	// if we're not using a separate backbuffer there's not much that this needs to do
}

void ChMngImage::SetFrameDelay(DWORD msDelay)
{
	m_pFrameList[0].m_frameInfo.iExposture = msDelay;
	//TRACE1("SetFrameDelay: %d\n", msDelay);
}
