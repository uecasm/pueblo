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

	Chaco GIF decoder

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"

#include <ChImgUtil.h>
#include <ChDibImage.h>


#ifdef USE_GLOBALALLOC

   #define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))   
                
   #define     GlobalUnlockPtr(lp) 	\
                GlobalUnlock(GlobalPtrHandle(lp))

   #define ALLOC(s)      (GlobalLock(GlobalAlloc((GHND), (s))))
   
   #define FREE(p)       (GlobalUnlockPtr(p), (BOOL)GlobalFree(GlobalPtrHandle(p)))

#else

    #define ALLOC(s) malloc(s)
    #define FREE(p) free(p)

#endif


// Draw the DIB to a given DC.
void ChDib::Draw(CDC* pDC, int x, int y, COLORREF clrTrans )
{

    if (clrTrans == 0xFFFFFFFF  ) 
	{ // we cache based on the last trasparent color
		ChDib::Draw( pDC, x, y );	
		return;
	}

	ChDib osb;			// buffer to do our work

	// create a DIB of the same size and NUM bits
	int iColors = ChImgUtil::MaxDeviceColors();
	osb.Create( 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth, 
					m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight,
					( iColors > 256 || iColors < 0 ) ? 24 :
					m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount );

	HDC  hDC  = pDC->GetSafeHdc();
	HDC  hDCTmp	= ::CreateCompatibleDC( hDC );

	HBITMAP hBmpTmp = ::CreateDIBitmap( hDC, 
									  &(osb.GetBitmapInfoAddress()->bmiHeader),
									  CBM_INIT,
									  osb.GetBitsAddress(),
									  osb.GetBitmapInfoAddress(),
									  DIB_RGB_COLORS );

	HBITMAP hOldBmp;

	hOldBmp = (HBITMAP)::SelectObject( hDCTmp, hBmpTmp ); 


    ::StretchBlt(  hDCTmp, 									// Copy to tmp DC
    				0,               						// Destination x
                    0,               						// Destination y
                    m_pFrameList[m_iCurrentFrame].
                    	m_pBMI->bmiHeader.biWidth,              // Destination width
                    m_pFrameList[m_iCurrentFrame].
                    	m_pBMI->bmiHeader.biHeight,             // Destination height
                    hDC,										// DC bits to copy
                    x,      	                  				// Source x
                    y,  	                      				// Source y
                    m_pFrameList[m_iCurrentFrame].
                    	m_pBMI->bmiHeader.biWidth,              // Destination width
                    m_pFrameList[m_iCurrentFrame].
                    	m_pBMI->bmiHeader.biHeight,             // Destination height
					SRCCOPY);


	::GetDIBits( hDCTmp, hBmpTmp, 0, m_pFrameList[m_iCurrentFrame].
									m_pBMI->bmiHeader.biHeight, 
						osb.GetBitsAddress(),
						osb.GetBitmapInfoAddress(),
						DIB_RGB_COLORS );

	::SelectObject( hDCTmp, hOldBmp );
	::DeleteObject( hBmpTmp );
	::DeleteDC( hDCTmp );


	// copy the non-transperent bits
	CopyBits(&osb, 0, 0, DibWidth(), DibHeight(), 0, 0, clrTrans );

	// Draw the dib	
	osb.Draw( pDC, x, y );	
  
}


// Draw the DIB to a given DC.
void ChDib::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, COLORREF  clrMask )
{

	if (  clrMask == 0xFFFFFFFF || clrTrans == 0xFFFFFFFF  || !IsTransparent() )
	{
		ChDib::Draw( pDC, x, y );	
		return;
	}
    else if ( clrMask == (COLORREF)m_pFrameList[m_iCurrentFrame].m_pMask 
    								&& clrTrans == GetTransparentColor() ) 
	{ // we cache based on the last trasparent color
		if ( m_pFrameList[m_iCurrentFrame].m_pTransDib )
		{
			m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
		}
		else
		{
			ChDib::Draw( pDC, x, y );	
		}
		return;
	}
		
	
	SetTransparentColor( clrTrans );
	m_pFrameList[m_iCurrentFrame].m_pMask 		 = (void*)clrMask;

	if ( 0 == m_pFrameList[m_iCurrentFrame].m_pTransDib  )
	{
		m_pFrameList[m_iCurrentFrame].m_pTransDib = new ChDib;		
	}
	 
	// create a DIB of the same size and NUM bits
	int iColors = ChImgUtil::MaxDeviceColors();

	m_pFrameList[m_iCurrentFrame].m_pTransDib->
					Create( 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth, 
							m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight,
						( iColors > 256 || iColors < 0 ) ? 24 :
							m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount );

	if ( m_pFrameList[m_iCurrentFrame].
				m_pTransDib->GetBitmapInfoAddress()->bmiHeader.biBitCount <= 8 )
	{	// set the color for the DIB
		RGBQUAD * pClrTbl = m_pFrameList[m_iCurrentFrame].m_pTransDib->GetClrTabAddress( 0 );

		pClrTbl[0].rgbRed = GetRValue( clrMask );
		pClrTbl[0].rgbGreen = GetGValue( clrMask );
		pClrTbl[0].rgbBlue = GetBValue( clrMask );

	}
	else
	{
		HDC  hDC  = pDC->GetSafeHdc();
		HDC  hDCTmp	= ::CreateCompatibleDC( hDC );


		HBITMAP hBmpTmp = ::CreateDIBitmap( hDC, 
										  &(m_pFrameList[m_iCurrentFrame].
										  m_pTransDib->GetBitmapInfoAddress()->bmiHeader),
										  CBM_INIT,
										  m_pFrameList[m_iCurrentFrame].
										  	m_pTransDib->GetBitsAddress(),
										  m_pFrameList[m_iCurrentFrame].
										  	m_pTransDib->GetBitmapInfoAddress(),
										  DIB_RGB_COLORS );

		HBITMAP hOldBmp;

		hOldBmp = (HBITMAP)::SelectObject( hDCTmp, hBmpTmp );
	 

		// Fill the Memdc with the mask color

		HBRUSH hBrMask = ::CreateSolidBrush( clrMask );

		CRect rtFill( 0, 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth,
							m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight );


		::FillRect( hDCTmp, rtFill, hBrMask );
	
		::DeleteObject( hBrMask );  

		::GetDIBits( hDCTmp, hBmpTmp, 0, 
							m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight, 
							m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitsAddress(),
							m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitmapInfoAddress(),
							DIB_RGB_COLORS );

		::SelectObject( hDCTmp, hOldBmp );
		::DeleteObject( hBmpTmp );
		::DeleteDC( hDCTmp );
	}


	// copy the non-transperent bits
	CopyBits( m_pFrameList[m_iCurrentFrame].m_pTransDib, 
				0, 0, DibWidth(), DibHeight(), 0, 0, 
				GetTransparentColor() );

	// Draw the dib	
	m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
  
}

// Draw the DIB to a given DC.
void ChDib::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, CBrush*  pbrMask )
{
	if (  pbrMask == 0 || clrTrans == 0xFFFFFFFF  || !IsTransparent() )
	{
		ChDib::Draw( pDC, x, y );	
		return;
	}
    else if ( pbrMask->GetSafeHandle() == m_pFrameList[m_iCurrentFrame].m_pMask 
    								&& clrTrans == GetTransparentColor()  ) 
	{ // we cache based on the last trasparent color
		if ( m_pFrameList[m_iCurrentFrame].m_pTransDib )
		{
			m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
		}
		else
		{
			ChDib::Draw( pDC, x, y );	
		}
		return;
	}
		
	
	SetTransparentColor( clrTrans );
	m_pFrameList[m_iCurrentFrame].m_pMask 		 = pbrMask->GetSafeHandle();

	if ( 0 == m_pFrameList[m_iCurrentFrame].m_pTransDib  )
	{
		m_pFrameList[m_iCurrentFrame].m_pTransDib = new ChDib;		
	}
	 
	// create a DIB of the same size and NUM bits
	int iColors = ChImgUtil::MaxDeviceColors();
	m_pFrameList[m_iCurrentFrame].m_pTransDib->
				Create( 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth, 
						 m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight,
						( iColors > 256 || iColors < 0 ) ? 24 :
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount );


	HDC  hDC  = pDC->GetSafeHdc();
	HDC  hDCTmp	= ::CreateCompatibleDC( hDC );


	HBITMAP hBmpTmp = ::CreateDIBitmap( hDC, 
									  &(m_pFrameList[m_iCurrentFrame].
									  	m_pTransDib->GetBitmapInfoAddress()->bmiHeader),
									  CBM_INIT,
									  m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitsAddress(),
									  m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitmapInfoAddress(),
									  DIB_RGB_COLORS );

	HBITMAP hOldBmp;

	hOldBmp = (HBITMAP)::SelectObject( hDCTmp, hBmpTmp ); 

	// Fill the Memdc with the mask color

	CRect rtFill( 0, 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth,
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight );

	::FillRect( hDCTmp, rtFill, (HBRUSH)pbrMask->GetSafeHandle() );
	

	::GetDIBits( hDCTmp, hBmpTmp, 0, 
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight, 
						m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitsAddress(),
						m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitmapInfoAddress(),
						DIB_RGB_COLORS );

	::SelectObject( hDCTmp, hOldBmp );
	::DeleteObject( hBmpTmp );
	::DeleteDC( hDCTmp );


	// copy the non-transperent bits
	CopyBits( m_pFrameList[m_iCurrentFrame].m_pTransDib, 
					0, 0, DibWidth(), DibHeight(), 0, 0, GetTransparentColor() );

	// Draw the dib	
	m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
  
}
// Draw the DIB to a given DC.
void ChDib::Draw(CDC* pDC, int x, int y, COLORREF clrTrans, ChDib* pdibMask )
{
	if (  pdibMask == 0 || clrTrans == 0xFFFFFFFF  || !IsTransparent() )
	{
		ChDib::Draw( pDC, x, y );	
		return;
	}
    else if ( pdibMask == m_pFrameList[m_iCurrentFrame].m_pMask 
    								&& clrTrans == GetTransparentColor() ) 
	{ // we cache based on the last trasparent color
		if ( m_pFrameList[m_iCurrentFrame].m_pTransDib )
		{
			m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
		}
		else
		{
			ChDib::Draw( pDC, x, y );	
		}
		return;
	}
		
	
	SetTransparentColor( clrTrans );
	m_pFrameList[m_iCurrentFrame].m_pMask = pdibMask;

	if ( 0 == m_pFrameList[m_iCurrentFrame].m_pTransDib  )
	{
		m_pFrameList[m_iCurrentFrame].m_pTransDib = new ChDib;		
	}
	 
	// create a DIB of the same size and NUM bits
	int iColors = ChImgUtil::MaxDeviceColors();
	m_pFrameList[m_iCurrentFrame].
			m_pTransDib->Create( 0, m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth, 
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight,
						( iColors > 256 || iColors < 0 ) ? 24 :
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount );


	HDC  hDC  = pDC->GetSafeHdc();
	HDC  hDCTmp	= ::CreateCompatibleDC( hDC );


	HBITMAP hBmpTmp = ::CreateDIBitmap( hDC, 
									  &(m_pFrameList[m_iCurrentFrame].
									  	m_pTransDib->GetBitmapInfoAddress()->bmiHeader),
									  CBM_INIT,
									  m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitsAddress(),
									  m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitmapInfoAddress(),
									  DIB_RGB_COLORS );

	HBITMAP hOldBmp;

	hOldBmp = (HBITMAP)::SelectObject( hDCTmp, hBmpTmp ); 

	int xSrc = 0, 
	ySrc = 0, 
	dstWidth = pdibMask->GetWidth(), 
	dstHeight = pdibMask->GetHeight();

	if ( x  < dstWidth && ( x + m_pFrameList[m_iCurrentFrame].
								m_pTransDib->GetWidth() ) <= dstWidth )
	{
		xSrc = x;
		dstWidth = m_pFrameList[m_iCurrentFrame].m_pTransDib->GetWidth(); 
	}
	else if ( dstWidth > m_pFrameList[m_iCurrentFrame].m_pTransDib->GetWidth()  )
	{
		dstWidth = m_pFrameList[m_iCurrentFrame].m_pTransDib->GetWidth();
	}

	if ( y  < dstHeight && ( y + m_pFrameList[m_iCurrentFrame].
						m_pTransDib->GetHeight() ) <= dstHeight )
	{
		ySrc = y;
		dstHeight = m_pFrameList[m_iCurrentFrame].m_pTransDib->GetHeight(); 
	}
	else if ( dstHeight > m_pFrameList[m_iCurrentFrame].m_pTransDib->GetHeight()  )
	{
		dstHeight = m_pFrameList[m_iCurrentFrame].m_pTransDib->GetHeight();
	}

	HPALETTE hPalette = 0;

	if ( pdibMask->GetDIBPalette() )
	{
    	CPalette *pPal = pdibMask->GetDIBPalette();
		hPalette = (HPALETTE)pPal->GetSafeHandle();
	}

	if ( hPalette )
	{

		hPalette = ::SelectPalette( hDCTmp,  hPalette, true );
		::RealizePalette( hDCTmp );
	}

    ::StretchDIBits( hDCTmp, 								// Copy to tmp DC
    				0,               						// Destination x
                    0,               						// Destination y
                    m_pFrameList[m_iCurrentFrame].
                    	m_pTransDib->GetBitmapInfoAddress()->bmiHeader.biWidth,              // Destination width
                    m_pFrameList[m_iCurrentFrame].
                    	m_pTransDib->GetBitmapInfoAddress()->bmiHeader.biHeight,             // Destination height
                    xSrc,                        				// Source x
                    ySrc,                        				// Source y
                    dstWidth,              				// Destination width
                    dstWidth,             				// Destination height
					pdibMask->GetBitsAddress(),
					pdibMask->GetBitmapInfoAddress(),
					pdibMask->GetBitmapInfoAddress()->bmiHeader.biBitCount <= 8 ?
										DIB_RGB_COLORS	 : 0,
					SRCCOPY);



	::GetDIBits( hDCTmp, hBmpTmp, 0, 
						m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight, 
						m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitsAddress(),
						m_pFrameList[m_iCurrentFrame].m_pTransDib->GetBitmapInfoAddress(),
						DIB_RGB_COLORS );

	if ( hPalette )
	{
		::SelectObject( hDCTmp, hPalette );
	}



	::SelectObject( hDCTmp, hOldBmp );
	::DeleteObject( hBmpTmp );
	::DeleteDC( hDCTmp );


	// copy the non-transperent bits
	CopyBits( m_pFrameList[m_iCurrentFrame].m_pTransDib, 0, 0, DibWidth(), DibHeight(), 0, 0, GetTransparentColor() );

	// Draw the dib	
	m_pFrameList[m_iCurrentFrame].m_pTransDib->ChDib::Draw( pDC, x, y );	
}

// $Log$
