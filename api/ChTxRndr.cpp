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

	This file consists of the implementation of the ChTxtWnd class.

	All rendering metods are in this file

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#ifdef CH_UNIX
#include <ChTypes.h>
#include <ChRect.h>
#include <ChSize.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#endif // CH_UNIX

#include <ChConst.h>
#include <ChTxtWnd.h>
#include <ChUtil.h>
#include <ChImgUtil.h>

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


inline void ChTxtWnd::DrawTextBackground( int x, int y, const char* pText, 
											int iCount, chuint32 luColor )
{
	// background color
	CBrush brBack;
	brBack.CreateSolidBrush( luColor );

	CRect rtBack( x, y, x, y );
	ChSize textSize;

	textSize = GetContext()->GetTextExtent( pText, iCount);

	rtBack.right  += textSize.cx;						 
	rtBack.bottom += textSize.cy;
	// Fill the background						 
	GetContext()->FillRect( &rtBack, &brBack );

}

inline void ChTxtWnd::DrawTextBackground( int x, int y, const char* pText, 
											int iCount, CBrush* pbrBack )
{
	CRect rtBack( x, y, x, y );
	ChSize textSize;

	textSize = GetContext()->GetTextExtent( pText, iCount);

	rtBack.right  += textSize.cx;						 
	rtBack.bottom += textSize.cy;
	// Fill the background						 
	GetContext()->FillRect( &rtBack, pbrBack );

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::DrawRange

------------------------------------------------------------------------------

	This method draws the text in the view.

----------------------------------------------------------------------------*/

bool ChTxtWnd::DrawRange()
{
	pChRun			pRun;
	pstr			pText;
	pChLine			pLine;
	pChStyleInfo	pStyleUse;
	pChStyleInfo	pCurrStyleUse;
	pChFontInfo		pFontTbl;
	int				sDrawCount;
	chint32			lStartChar, lLastChar;
	chint32			lIndex;
	int				x; 
	bool			boolNewStyle;
	COLORREF		clrOld = CH_COLOR_DEFAULT;
	ChRect			rtClipBox( 0, 0, 0, GetViewHeight() );

	if ( !GetTextCount() )
	{
		return true;
	}




	// get the area to update
	#ifdef CH_MSW
	GetContext()->GetClipBox( &rtClipBox );
	#else
	// XXX Get the clip box so we can optimize redraws
	rtClipBox.bottom = GetCanvasHeight();
	#endif	   


	ChPoint 		ptOrigin;

	// The current view position
	ptOrigin = GetDeviceScrollPosition();	 

	// set up the bottom indent
	if ( rtClipBox.bottom > ( ptOrigin.y + GetViewHeight() - m_viewIndents.bottom) )
	{
		rtClipBox.bottom = ptOrigin.y + GetViewHeight() - m_viewIndents.bottom;
	}
	// set up the top indent
	if ( rtClipBox.top < ( ptOrigin.y + m_viewIndents.top) )
	{
		rtClipBox.top = ptOrigin.y +  m_viewIndents.top;
	}



	GetContext()->SetTextAlign( TA_BASELINE );
	//GetContext()->SetTextAlign( TA_TOP );
	GetContext()->SetTextJustification(  0, 0 );
	GetContext()->SetBkMode( TRANSPARENT );
	
	pRun 		= GetRunTable();
	pText 		= GetTextBuffer();
	pLine 		= GetLineTable();
	pStyleUse 	= GetStyleTable();
	pFontTbl 	= GetFontTable();



	lStartChar 	= 0;
	lLastChar 	= GetTextCount();
	lIndex = 0;

	while ((lIndex < GetLineCount())
		&& ( ( pLine->iY + pLine->iMaxHeight ) < rtClipBox.top ))
	{
		lIndex += 1;
		pLine += 1;
	}

	// This is our new top line
	lStartChar = pLine->lStartChar;
	lIndex = GetRunIndex(  lStartChar );
	pRun += lIndex;
	pText += lStartChar;
	boolNewStyle = true;


	// normalize selection range
	chint32 lSelStart = m_lSelStart, lSelEnd = m_lSelEnd;

	if ( m_lSelStart > m_lSelEnd )
	{
		lSelStart = m_lSelEnd;
		lSelEnd   = m_lSelStart;
	}



   	// Draw all the characters till we hit the bottom of the view
	while ((lStartChar < lLastChar) && ( pLine->iY <  
							( rtClipBox.bottom + pLine->iMaxHeight ) ))
	{
		x = pLine->iX;	// setup the left margin


		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textCenter )
		{ // justify center
			x += ( (pLine->iMaxLineWidth - pLine->iTotalWidth)/2);
		}
  		else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textRight )
		{   // justify right
			//	x += ( GetViewWidth() - m_viewIndents.right - x - pLine->iTotalWidth);
			x += pLine->iMaxLineWidth - pLine->iTotalWidth;
		}


		// Draw all the charcters in this line
		while ((lStartChar < lLastChar) && (lStartChar < pLine[1].lStartChar))
		{
			if (boolNewStyle)
			{	// Set up the DC for the new style
				pCurrStyleUse = &pStyleUse[ pRun->lStyleIndex ];
				::SelectObject(  GetContext()->m_hDC, 
						pFontTbl[pCurrStyleUse->style.iFontIndex].pFont->m_hObject );
				//GetContext()->SetTextCharacterExtra(  pCurrStyleUse->style.iAddWidth );
				::SetTextColor( GetContext()->m_hDC, pCurrStyleUse->style.lColor );
				

				boolNewStyle = false;
			}
			// Number of charcters to draw
			if (pLine[1].lStartChar < pRun[1].lStartChar)
			{  // multiline run
				sDrawCount = (int)(pLine[1].lStartChar - lStartChar);
			}
			else
			{  // multiple runs in the current line
				sDrawCount = (int)(pRun[1].lStartChar - lStartChar);
			}
			// sanity check
			if (sDrawCount > (int)(lLastChar - lStartChar))
			{
				sDrawCount = (int)(lLastChar - lStartChar);
			}
			// splat it on the view
			ChSize txtOut;
			if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
			{	// Draw the line

				// width of the line from current X to the end of the line
				ChSize lineSize( GetViewWidth() - x - m_viewIndents.right, pLine->iMaxHeight );

				pStyleUse[ pRun->lStyleIndex ].style.luStyle |= textResetAnimation;

				txtOut = DrawObject(  x, pLine->iY, 
								pStyleUse[ pRun->lStyleIndex ].style, lineSize );

  				pStyleUse[ pRun->lStyleIndex ].style.luStyle &= ~textResetAnimation;

				
				sDrawCount = (int)(pRun[1].lStartChar - pRun[0].lStartChar);

			}
			else
			{	
				 
				int iCount = GetDrawCharCount( pText, sDrawCount );
				int y = pLine->iY;
				int iRunHeight = pLine->iMaxHeight;

				if (  pLine->luLineAttr & objAttrMiddle || 
							pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textVCenter )
				{
					iRunHeight = pStyleUse[ pRun->lStyleIndex ].iFontHeight + GetExtraHeight();
					if ( pLine->iMaxHeight > iRunHeight  )
					{  // center the text to the height of the control
						y = y - ((pLine->iMaxHeight >> 1) - ( iRunHeight >> 1));
					}
				}


				if ( IsSelectionEnabled() && m_lSelStart != m_lSelEnd 
								&& lStartChar <= lSelEnd  
								&& ( lStartChar + iCount ) >= lSelStart  )
				{ // draw text in selected mode

					int iPreCount = 0, iSelCount = 0, iPostCount = 0;
	  				if ( lStartChar  >= lSelStart  
								&& ( lStartChar + iCount ) < lSelEnd )
					{
						iSelCount = iCount;
					} 
					else if ( lStartChar < lSelStart )
					{
						iPreCount = (int)(lSelStart - lStartChar);

						if ( ( lStartChar + iCount - 1 ) > lSelEnd )
						{
							iSelCount = (int)(lSelEnd - lSelStart);
							iPostCount = iCount - iPreCount - iSelCount;
						}
						else
						{
							iSelCount = iCount - iPreCount;
						}
					}
					else
					{
						iSelCount = (int)(lSelEnd - lStartChar);
						iPostCount = iCount - iSelCount;
					}


					int iCurrWidth = 0;

					if ( iPreCount )
					{

						if ( !(pCurrStyleUse->style.lBackColor & CH_COLOR_DEFAULT ))
						{
							DrawTextBackground( x, 
							(y + pLine->iMaxHeight - pLine->iMaxDescent - pCurrStyleUse->iFontAscent ),
							pText, iPreCount, pCurrStyleUse->style.lBackColor );
						}

						txtOut =  GetContext()->TabbedTextOut( x, 
											y + pLine->iMaxHeight - pLine->iMaxDescent,
														pText, iPreCount,
														0, 0, 0);
						iCurrWidth = txtOut.cx;
					}

					

					if ( iSelCount )
					{


						DrawTextBackground( x + iCurrWidth, 
													(y + pLine->iMaxHeight - pLine->iMaxDescent 
													- pCurrStyleUse->iFontAscent ),
													pText + iPreCount, 
													iSelCount, &m_brSelBackColor );

						// Sel text color
						chuint32 luOldTextColor;
						luOldTextColor = GetContext()->SetTextColor( m_luSelTextColor );
					 	// draw the text
						txtOut =  GetContext()->TabbedTextOut( x + iCurrWidth, 
														y + pLine->iMaxHeight - pLine->iMaxDescent,
														pText + iPreCount, iSelCount,
														0, 0, 0);
							// restore the color 
						GetContext()->SetTextColor( luOldTextColor );
						
						iCurrWidth += txtOut.cx;
					}



					if ( iPostCount )
					{
						if ( !(pCurrStyleUse->style.lBackColor & CH_COLOR_DEFAULT ))
						{
							DrawTextBackground( x + iCurrWidth, 
										(y + pLine->iMaxHeight - pLine->iMaxDescent 
										- pCurrStyleUse->iFontAscent ), 
										pText + iPreCount + iSelCount, iPostCount, 
										pCurrStyleUse->style.lBackColor );
						}

						txtOut =  GetContext()->TabbedTextOut( x + iCurrWidth, 
														y + pLine->iMaxHeight  - pLine->iMaxDescent,
														pText + iPreCount + iSelCount, iPostCount,
														0, 0, 0);
						iCurrWidth += txtOut.cx;
					}

					// the actual width of drawn text
					txtOut.cx = iCurrWidth;



				}
				else
				{
					if ( !(pCurrStyleUse->style.lBackColor & CH_COLOR_DEFAULT ))
					{
						DrawTextBackground( x, (y + pLine->iMaxHeight - pLine->iMaxDescent - 
											pCurrStyleUse->iFontAscent ), 
											pText, iCount,pCurrStyleUse->style.lBackColor );
					}

					txtOut =  GetContext()->TabbedTextOut( x, 
													y + pLine->iMaxHeight - pLine->iMaxDescent,
													pText, iCount,
													0, 0, 0);
				}
			}

			// Set up for the next run
			x += txtOut.cx;

			pText += sDrawCount;
			lStartChar += sDrawCount;
			if (lStartChar == pRun[1].lStartChar)
			{
				pRun += 1;
				boolNewStyle = true;
			}
		}
		
		// Setup for the next line
		pLine += 1;
	}

	return (true);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::DisableHotSpots

------------------------------------------------------------------------------

	This method disbale all hotspot and updates the color of the hotspot text
	to reflect the change.

----------------------------------------------------------------------------*/

void ChTxtWnd::DisableHotSpots( chuint32 luColor )
{
	// find the first run using this object
	pChStyleInfo pStyleUse 	= GetStyleTable();
 	pChRun 		 pRun 		= GetRunTable();
	bool		boolUpdated = false;
	ChRect		rtUpdateRect( 0, 0, 
				GetViewWidth(), GetViewHeight() );;

	// do we have any runs with hot spot in this line
	chint32 lIndex = 0;

	// skip all the object at the begining of the line
	while( lIndex < GetRunCount() ) 
	{
		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textHotSpot )
		{
			pStyleUse[ pRun->lStyleIndex ].style.luStyle &= ~ChTxtWnd::textHotSpot;

			pStyleUse[ pRun->lStyleIndex ].style.lColor = luColor;

			if ( !boolUpdated )
			{
				boolUpdated = true;

				ChPoint ptPos = GetDeviceScrollPosition();
			
				chint32 lUpdateHeight;
				GetLineIndex( pRun->lStartChar, lUpdateHeight );
  				rtUpdateRect.top = (int)lUpdateHeight > ptPos.y 
										? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
			}

		}
		++pRun;
		++lIndex;
	}
	// if there was a hotspot currently visible then update
	// the view to reflect the new state
	if ( boolUpdated && rtUpdateRect.top < rtUpdateRect.bottom  )
	{
		InvalidateRect( rtUpdateRect, false );
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::SetViewSize()

------------------------------------------------------------------------------

Set the new canvas size

----------------------------------------------------------------------------*/

bool ChTxtWnd::SetViewSize()
{ 
	ChSize size;
	GetTotalCanvasSize( size );

	m_sizeTotal = size;

	if ( m_luDocAttrs & docNoVertScroll )
	{
		size.cy = GetViewHeight();
	}
	if ( m_luDocAttrs & docNoHorzScroll )
	{
		size.cx = GetViewWidth();
	}
	
	ChSize sizePage, sizeLine;

	#if defined( CH_MSW )
	sizePage = ChScrollWnd::sizeDefault;
	sizePage.cy = m_viewSize.cy;
	sizeLine = ChScrollWnd::sizeDefault;
	if ( m_lLineCount )
	{ 
		sizeLine.cy = (int)(size.cy/ (int)m_lLineCount); // avg line height
	}
	#else
	sizePage = ChScrollView::sizeDefault;
	sizeLine = ChScrollView::sizeDefault;
	if ( m_lLineCount )
	{ 
		sizeLine.cy = size.cy/ m_lLineCount; // avg line height
	}
	#endif



	CSize 		sizeClient;
	CSize 		sizeSb;
	DWORD		dwStyle = CWnd::GetStyle();

	// get client rect
	GetTrueClientSize(sizeClient, sizeSb);

	if ( size.cx > sizeClient.cx  && !(m_luDocAttrs & docNoHorzScroll) )
	{ 	// add the left indent width for space on the right
		size.cx += m_viewIndents.left; 	
	 	if ( size.cy > sizeClient.cy  && !(m_luDocAttrs & docNoVertScroll) )
		{ 	// add the left indent width for space below
			size.cy += ((m_viewIndents.left >> 1 ) * 3); 
		}

		m_sizeTotal = size;
	}
	else if ( size.cy > sizeClient.cy  && !(m_luDocAttrs & docNoVertScroll) )
	{ 	// add the left indent width for space below
		size.cy += (m_viewIndents.left >> 1); 
		m_sizeTotal = size;
	}


	SetScrollSizesEx( size, sizePage, sizeLine );		// New canvas size

	CSize		sizeRange;
	sizeRange = size - sizeClient;


	SCROLLINFO	info;
	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;

	if ( size.cx > sizeClient.cx && (dwStyle & WS_HSCROLL) )
	{ // set the new pos for the scroll bars

		info.nPage = sizeClient.cx;
		info.nMax = size.cx;

		if (!SetScrollInfo( SB_HORZ, &info, true ))
		{
			SetScrollRange( SB_HORZ, 0, sizeRange.cx, true );
		}

	}
	else if ( size.cx > sizeClient.cx && !(dwStyle & WS_HSCROLL) )
	{ // we need scroll bar, adding scroll bar will change the client width
	  // which will cause this function to be called again, lets post a message 
	  // to set the scroll bars
		EnableScrollBarCtrl( SB_HORZ, true );

		info.nPage = m_viewSize.cx;
		info.nMax = m_sizeTotal.cx;

		if (!SetScrollInfo( SB_HORZ, &info, true ))
		{
			sizeRange = m_sizeTotal - m_viewSize;
			SetScrollRange( SB_HORZ, 0, sizeRange.cx, true );
		}
	}
	else if ( size.cx <= sizeClient.cx && (dwStyle & WS_HSCROLL))
	{ // we need scroll bar, adding scroll bar will change the client width
	  // which will cause this function to be called again, lets post a message 
	  // to set the scroll bars
		EnableScrollBarCtrl( SB_HORZ, false );
	}


	if ( size.cy > sizeClient.cy && (dwStyle & WS_VSCROLL) )
	{ // set the new pos for the scroll bars
		info.nPage = sizeClient.cy;
		info.nMax = size.cy;

		if (!SetScrollInfo( SB_VERT, &info, true ))
		{
			SetScrollRange( SB_VERT, 0, sizeRange.cy, true );
		}
	}
	else if ( size.cy > sizeClient.cy && !(dwStyle & WS_VSCROLL) )
	{ // we need scroll bar, adding scroll bar will change the client width
	  // which will cause this function to be called again, lets post a message 
	  // to set the scroll bars

		EnableScrollBarCtrl( SB_VERT, true );

		info.nPage = m_viewSize.cy;
		info.nMax = m_sizeTotal.cy;

		if (!SetScrollInfo( SB_VERT, &info, true ))
		{
			sizeRange = m_sizeTotal - m_viewSize;
			SetScrollRange( SB_VERT, 0, sizeRange.cy, true );
		}
	}
	else if ( size.cy <= sizeClient.cy && (dwStyle & WS_VSCROLL) )
	{ // we need scroll bar, adding scroll bar will change the client width
	  // which will cause this function to be called again, lets post a message 
	  // to set the scroll bars

		EnableScrollBarCtrl( SB_VERT, false );
	}

	return true;

}

void ChTxtWnd::CreateBackground( ChDib* pDib )
{ 
	delete m_pbackGround;

	CPalette* pStdPal = ChImgUtil::GetStdPalette();

	if ( pDib->GetBitmapInfoAddress()->bmiHeader.biBitCount == 8 
			&& pStdPal )
	{
		pDib->MapColorsToPalette( pStdPal );	
	}
	else
	{
		m_pbackGround = 0;
		return;	
	}


	m_pbackGround = new ChDib;
	ASSERT( m_pbackGround );	   

	ChImageInfo imgInfo;
	ChMemClearStruct( &imgInfo );
	imgInfo.iWidth = GetViewWidth();
	imgInfo.iHeight = GetViewHeight();

	m_pbackGround->NewImage( &imgInfo );

	ChImageFrameInfo frameInfo;

	ChMemClearStruct( &frameInfo );
	frameInfo.iFrame = 0;
	frameInfo.iWidth = GetViewWidth();
	frameInfo.iHeight = GetViewHeight();

	m_pbackGround->Create( &frameInfo, 8 );	

	CDC* pDC = GetDC();

	CDC 		memDC;

	if ( !memDC.CreateCompatibleDC( pDC ) )
	{
		return;
	}


	HBITMAP hBmpTmp = ::CreateDIBitmap( pDC->GetSafeHdc(), 
									  &(m_pbackGround->GetBitmapInfoAddress()->bmiHeader),
									  CBM_INIT,
									  m_pbackGround->GetBitsAddress(),
									  m_pbackGround->GetBitmapInfoAddress(),
									  DIB_RGB_COLORS );

	ReleaseDC( pDC ); 

	HBITMAP hOldBmp;

	hOldBmp = (HBITMAP)::SelectObject(  memDC.GetSafeHdc(), hBmpTmp ); 


	CPalette *pOldPal = 0;
	
	if ( pStdPal )
	{
		pOldPal = memDC.SelectPalette( pStdPal, TRUE );
		memDC.RealizePalette();
	} 

	int iDibWidth = pDib->GetWidth();
	int iDibHeight = pDib->GetHeight();
	int x = 0, y = 0; 

	for ( y = 0;  y < m_viewSize.cy; y += iDibHeight )
	{
		int iBltHt = (y + iDibHeight) < m_viewSize.cy ? iDibHeight : m_viewSize.cy - y;
		for ( x = 0; x < GetViewWidth(); x += iDibWidth )
		{
			int iBltWidth = (x + iDibWidth) < m_viewSize.cx ? iDibWidth : m_viewSize.cx - x;
		    ::StretchDIBits( memDC.GetSafeHdc(),
		                    x,                        // Destination x
		                    y,                        // Destination y
		                    iBltWidth,          // Destination width
		                    iBltHt,         // Destination height
		                    0,                        // Source x
		                    0,                        // Source y
		                    iBltWidth,          // Source width
		                    iBltHt,         // Source height
		                    pDib->GetBitsAddress(),         // Pointer to bits
		                    pDib->GetBitmapInfoAddress(),   // BITMAPINFO
		                    DIB_RGB_COLORS,           // Options
		                    SRCCOPY);                 // Raster operation code (ROP)
		
		}

	}

	::GetDIBits(  memDC.GetSafeHdc(), hBmpTmp, 0, GetViewHeight(), 
						m_pbackGround->GetBitsAddress(),
						m_pbackGround->GetBitmapInfoAddress(),
						DIB_RGB_COLORS );
	if ( pOldPal )
	{
		memDC.SelectPalette( pOldPal, FALSE );
	}

}

void ChTxtWnd::CreateBackground( COLORREF clrBack )
{ 
	delete m_pbackGround;

	m_pbackGround = new ChDib;
	ASSERT( m_pbackGround );

	ChImageInfo imgInfo;
	ChMemClearStruct( &imgInfo );
	imgInfo.iWidth = GetViewWidth();
	imgInfo.iHeight = GetViewHeight();

	m_pbackGround->NewImage( &imgInfo );

	ChImageFrameInfo frameInfo;

	ChMemClearStruct( &frameInfo );
	frameInfo.iFrame = 0;
	frameInfo.iWidth = GetViewWidth();
	frameInfo.iHeight = GetViewHeight();

	m_pbackGround->Create( &frameInfo, 8 );	


	RGBQUAD * pClrTbl = m_pbackGround->GetClrTabAddress( 0 );

	pClrTbl[0].rgbRed = GetRValue( clrBack );
	pClrTbl[0].rgbGreen = GetGValue( clrBack );
	pClrTbl[0].rgbBlue = GetBValue( clrBack );


}

void ChTxtWnd::DrawBackground( CDC* pDC )
{


	if ( m_pbackGround->GetWidth() < GetViewWidth() 
					||  m_pbackGround->GetHeight() < GetViewHeight() )
	{  	
		if ( m_pbkImage )
		{
			CreateBackground( m_pbkImage  );	
		}
		else
		{
			m_pbackGround->SetSize( GetViewWidth(), GetViewHeight() );	
		}

	}

	CRect	rtClip;
	pDC->GetClipBox( &rtClip );	

	ChPoint 		ptPos;

	ptPos  = GetDeviceScrollPosition();



	CPalette *pOldPal = 0;
	
	CPalette* pStdPal = ChImgUtil::GetStdPalette();
	if ( pStdPal )
	{
		pOldPal = pDC->SelectPalette( pStdPal, TRUE );
		pDC->RealizePalette();
	} 

	bool boolWinNT = (ChUtil::GetSystemType() == CH_SYS_WINNT );

	ptPos.y += rtClip.top;

	int iTopX = rtClip.top;
	
	while(  iTopX <  rtClip.bottom  )
	{
	  	int iTop, iBottom;


		if ( ptPos.y < GetViewHeight() )
		{
			iTop = ptPos.y;
		}
		else
		{
			iTop =  ptPos.y % GetViewHeight();
		}

		iBottom = iTop + (rtClip.bottom - iTopX);

		if ( iBottom > GetViewHeight() )
		{
			iBottom = GetViewHeight();
		}


		if ( boolWinNT )
		{
			// Make it top down bmp temporarily
			m_pbackGround->GetBitmapInfoAddress()->bmiHeader.biHeight *= -1;

		    ::StretchDIBits(pDC->GetSafeHdc(),
		                    0,  			// Destination x
		                    iTopX,      	// Destination y
		                    GetViewWidth(), // Destination width
		                    iBottom - iTop, // Destination height
		                    0,  			// Source x
		                    iTop,    		// Source y
		                    GetViewWidth(), // Source width
		                    iBottom - iTop, // Source height
		                    m_pbackGround->GetBitsAddress(),        // Pointer to bits
		                    m_pbackGround->GetBitmapInfoAddress(), // BITMAPINFO
		                    DIB_RGB_COLORS,         // Options
		                    SRCCOPY);                // Raster operation code (ROP)
				// restore it to bottomup bmp 
				m_pbackGround->GetBitmapInfoAddress()->bmiHeader.biHeight *= -1;
		}
		else
		{
		    int iNum = ::SetDIBitsToDevice(pDC->GetSafeHdc(),
			                    0,  			// Destination x
			                    iTopX, 			// Destination y
			                    GetViewWidth(), // Destination width
			                    iBottom - iTop, // Destination height
			                    0,  			// Source x
			                    iBottom,		// Source y
			                    iBottom, 		// First scan line
			                    iBottom - iTop, // num scan lines
			                    m_pbackGround->GetPixelAddress( 0, iBottom - 1),  // Pointer to bits
			                    m_pbackGround->GetBitmapInfoAddress(), // BITMAPINFO
			                    DIB_RGB_COLORS );         // Options 
		}


		iTopX += iBottom - iTop;
		ptPos.y += iBottom - iTop;
	} 
	   

	if ( pOldPal )
	{
		pDC->SelectPalette( pOldPal, FALSE );
	}

}

// $Log$
