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

	This file contains the interface for the PNG/MNG image object.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CHMNGIMAGE_H_ )
#define   CHMNGIMAGE_H_

#include "ChDibImage.h"

class ChMngDecoder;

class CH_EXPORT_CLASS ChMngImage : public ChDib
{
	public:
		ChMngImage();
		virtual ~ChMngImage();

		bool Load( const char* pszFileName );

		virtual bool NewImage( pChImageInfo pImage );
		virtual void NextFrame();
		virtual void Draw(CDC* pDC, int x, int y);

		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, COLORREF  clrMask );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, CBrush*  pbrMask );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, ChDib* pdibMask );

		void *GetCanvasLine(DWORD y);
		void *GetBackgroundLine(DWORD y);		// don't need this if using AlphaBlend call
		void UpdateFrame(DWORD x, DWORD y, DWORD cx, DWORD cy);
		void SetFrameDelay(DWORD msDelay);

	protected:
		typedef BOOL (WINAPI *AlphaBlendProc)( IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN BLENDFUNCTION);

		ChMngDecoder *m_decoder;
		HMODULE m_ImageLibrary;
		AlphaBlendProc m_AlphaBlend;
		bool m_ShownFirstFrame;

		void Finished();
		bool SupportsAlphaBlending();
		void ResetAnimation();
};

#endif		// CHMNGIMAGE_H_
