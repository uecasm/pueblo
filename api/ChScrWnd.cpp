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

	This file consists of the implementation of the ChScrollWnd view class.

----------------------------------------------------------------------------*/

#include "headers.h"
#include <afxpriv.h>
#include <limits.h>
#include <ChScrWnd.h>     

#include <ChUtil.h>


#if !defined( CH_VRML_VIEWER )
#include <ChCore.h>
#include <ChPerFrm.h>
#endif


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChScrollWnd constants
----------------------------------------------------------------------------*/

#define CH_SCROLL_WND_CLASS		"ChScrollWnd"

/*----------------------------------------------------------------------------
	ChScrollWnd class
----------------------------------------------------------------------------*/

const SIZE	ChScrollWnd::sizeDefault = { 0, 0 };
ChString		ChScrollWnd::m_strClass;


ChScrollWnd::ChScrollWnd() : m_boolInsideUpdate( false )
{
	if (0 == m_strClass.GetLength() && ChUtil::GetSystemType() != CH_SYS_WIN32S )
	{
		m_strClass = AfxRegisterWndClass( CS_DBLCLKS | CS_GLOBALCLASS,
											LoadCursor( 0, IDC_ARROW ),
											(HBRUSH)(COLOR_WINDOW + 1) );
	}
}

ChScrollWnd::~ChScrollWnd()
{
}


BOOL ChScrollWnd::Create( LPCTSTR lpszWindowName, DWORD dwStyle,
							const ChRect& rect, CWnd* pParentWnd, UINT nID )
{
	if ( ChUtil::GetSystemType() != CH_SYS_WIN32S )
	{
		return CWnd::Create( m_strClass, lpszWindowName, dwStyle, rect,
							pParentWnd, nID, 0 );
	}
	else
	{
		return CWnd::Create( NULL, lpszWindowName, dwStyle, rect,
							pParentWnd, nID, 0 );
	}
}


BOOL ChScrollWnd::CreateEx( LPCTSTR lpszWindowName, DWORD dwStyle,
							DWORD dwStyleEx, const ChRect& rect,
							CWnd* pParentWnd, UINT nID )
{
	if (ChUtil::GetSystemType() != CH_SYS_WIN32S)
	{
		HWND	hwndParent = pParentWnd ? pParentWnd->m_hWnd : 0;

		return CWnd::CreateEx( dwStyleEx, m_strClass, lpszWindowName, dwStyle,
								rect.left, rect.top, rect.Width(),
								rect.Height(), hwndParent, (HMENU)nID );
	}
	else
	{
		return CWnd::Create( NULL, lpszWindowName, dwStyle, rect,
							pParentWnd, nID, 0 );
	}
}


void ChScrollWnd::PageUp()
{
	int		y = GetScrollPos( SB_VERT );
	int		yOrig = y;

	y -= m_pageDev.cy;

	if (OnScrollBy( CSize( 0, y - yOrig ) ))
	{
		UpdateWindow();
	}
}


void ChScrollWnd::PageDown()
{
	int		y = GetScrollPos( SB_VERT );
	int		yOrig = y;

	y += m_pageDev.cy;

	if (OnScrollBy( CSize( 0, y - yOrig ) ))
	{
		UpdateWindow();
	}
}


void ChScrollWnd::Home()
{
	int		yOrig = GetScrollPos( SB_VERT );

	if (OnScrollBy( CSize( 0, -yOrig ) ))
	{
		UpdateWindow();
	}
}


void ChScrollWnd::End()
{
	chint32		lScrollMax = LONG_MAX / 2;

	if (OnScrollBy( CSize( 0, lScrollMax ) ))
	{
		UpdateWindow();
	}
}


//BEGIN_MESSAGE_MAP(ChScrollWnd, CWnd)
BEGIN_MESSAGE_MAP(ChScrollWnd, CScrollView)
	//{{AFX_MSG_MAP(ChScrollWnd)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_WM_MOUSEACTIVATE()
#if _MFC_VER < 0x0700
	ON_WM_MOUSEWHEEL()
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChScrollWnd public methods
----------------------------------------------------------------------------*/

void ChScrollWnd::DoScrollWindow( int iXDist, int iYDist )
{
	if (IsWindowVisible())
	{										/* When visible, let Windows do
																the scrolling */
		ScrollWindow( iXDist, iYDist );

		return;
	}
											/* Windows does not perform any
												scrolling if the window is
												not visible.  This leaves child
												windows unscrolled.  To account
												for this oversight, the child
												windows are moved directly
												instead. */

	HWND hWndChild = ::GetWindow( m_hWnd, GW_CHILD );

	if (hWndChild != NULL)
	{
		for (; hWndChild != NULL;
				hWndChild = ::GetNextWindow( hWndChild, GW_HWNDNEXT ))
		{
			CRect	rect;

			::GetWindowRect( hWndChild, &rect );
			ScreenToClient( &rect );
			::SetWindowPos( hWndChild, 0,
							rect.left + iXDist, rect.top + iYDist, 0, 0,
							SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
		}
	}
}


ChPoint ChScrollWnd::GetDeviceScrollPosition() const
{
	ChPoint		pt( GetScrollPos( SB_HORZ ), GetScrollPos( SB_VERT ) );

	ASSERT( pt.x >= 0 && pt.y >= 0 );

	return pt;
}


void ChScrollWnd::GetScrollBarSizes( CSize& sizeSb )
{
	DWORD		dwStyle = GetStyle();

	sizeSb.cx = sizeSb.cy = 0;

	if (GetScrollBarCtrl( SB_VERT ) == 0)
	{
											/* Vert scrollbars will impact
												client area of this window */

		sizeSb.cx = GetSystemMetrics( SM_CXVSCROLL );
	}

	if (GetScrollBarCtrl( SB_HORZ ) == 0)
	{
											/* Horz scrollbars will impact
												client area of this window */

		sizeSb.cy = GetSystemMetrics( SM_CYHSCROLL );
	}
}


void ChScrollWnd::GetScrollBarState( CSize sizeClient, CSize& needSb,
										CSize& sizeRange, CPoint& ptMove,
										bool boolInsideClient )
{
											/* Get scroll bar sizes (the part
												that is in the client area) */
	CSize		sizeSb;

	GetScrollBarSizes( sizeSb );
											// Enough room to add scrollbars
	sizeRange = m_totalDev - sizeClient;
											// > 0 => need to scroll
	ptMove = GetDeviceScrollPosition();
											/* Point to move to (start at
												current scroll pos) */
	BOOL	boolNeedH = sizeRange.cx > 0;

	if (!boolNeedH)
	{
		ptMove.x = 0;                       // Jump back to origin
	}
	else if (boolInsideClient)
	{
		sizeRange.cy += sizeSb.cy;          // Need room for a scroll bar
	}

	bool	boolNeedV = sizeRange.cy > 0;

	if (!boolNeedV)
	{
		ptMove.y = 0;                       // Jump back to origin
	}
	else if (boolInsideClient)
	{
		sizeRange.cx += sizeSb.cx;          // Need room for a scroll bar
	}

	if (boolNeedV && !boolNeedH && sizeRange.cx > 0)
	{
		ASSERT( boolInsideClient );
											/* Need a horizontal scrollbar
												after all */
		boolNeedH = true;
		sizeRange.cy += sizeSb.cy;
	}
											/* If current scroll position will
												be past the limit, scroll to
												limit */

	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
	{
		ptMove.x = sizeRange.cx;
	}

	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
	{
		ptMove.y = sizeRange.cy;
	}
											/* Now update the bars as
												appropriate */
	needSb.cx = boolNeedH;
	needSb.cy = boolNeedV;
											/* needSb, sizeRange, and ptMove
												area now all updated */
}


int ChScrollWnd::GetScrollLimit( int iBar )
{
	int		iMin, iMax;

	GetScrollRange( iBar, &iMin, &iMax );

	ASSERT( iMin == 0 );

	#if defined( WIN32 )
	SCROLLINFO		info;

	info.fMask = SIF_PAGE;

	if (GetScrollInfo( iBar, &info ))
	{
		iMax -= info.nPage;
		iMax = iMax >= 0 ? iMax : 0;
	}
	#endif

	return iMax;
}


bool ChScrollWnd::GetTrueClientSize( CSize& size, CSize& sizeSb )
{
	CRect		rect;
	DWORD		dwStyle = GetStyle();

	GetClientRect( &rect );
	ASSERT( rect.top == 0 && rect.left == 0 );

	size.cx = rect.right;
	size.cy = rect.bottom;
											/* First get the size of the
												scrollbars for this window */
	GetScrollBarSizes( sizeSb );
											/* First calculate the size of a
												potential scrollbar (scroll bar
												controls do not get turned
												on/off) */
	if (sizeSb.cx != 0 && (dwStyle & WS_VSCROLL))
	{
											/* Vert scrollbars will impact
												client area of this window */

		size.cx += sizeSb.cx;				// currently on - adjust now
	}

	if (sizeSb.cy != 0 && (dwStyle & WS_HSCROLL))
	{
											/* Horz scrollbars will impact
												client area of this window */

		size.cy += sizeSb.cy;				// currently on - adjust now
	}
											// Return TRUE if enough room

	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}


void ChScrollWnd::SetScrollSizes( SIZE sizeTotal, const SIZE& sizePage,
									const SIZE& sizeLine )
{
	ASSERT( sizeTotal.cx >= 0 && sizeTotal.cy >= 0 );

	m_totalDev = sizeTotal;
	m_pageDev = sizePage;
	m_lineDev = sizeLine;

	if (m_totalDev.cy < 0)
	{
		m_totalDev.cy = -m_totalDev.cy;
	}

	if (m_pageDev.cy < 0)
	{
		m_pageDev.cy = -m_pageDev.cy;
	}

	if (m_lineDev.cy < 0)
	{
		m_lineDev.cy = -m_lineDev.cy;
	}
											// now adjust device specific sizes

	ASSERT( m_totalDev.cx >= 0 && m_totalDev.cy >= 0 );

	if (m_pageDev.cx == 0)
	{
		m_pageDev.cx = m_totalDev.cx / 10;
	}

	if (m_pageDev.cy == 0)
	{
		m_pageDev.cy = m_totalDev.cy / 10;
	}

	if (m_lineDev.cx == 0)
	{
		m_lineDev.cx = m_pageDev.cx / 10;
	}

	if (m_lineDev.cy == 0)
	{
		m_lineDev.cy = m_pageDev.cy / 10;
	}

	if (m_hWnd != NULL)
	{										/* Window has been created,
												invalidate now */
		UpdateBars();
	}
}

// Update the scroll sizes but does not update the scroll bars
void ChScrollWnd::SetScrollSizesEx( SIZE sizeTotal, const SIZE& sizePage,
									const SIZE& sizeLine )
{
	ASSERT( sizeTotal.cx >= 0 && sizeTotal.cy >= 0 );

	m_totalDev = sizeTotal;
	m_pageDev = sizePage;
	m_lineDev = sizeLine;

	if (m_totalDev.cy < 0)
	{
		m_totalDev.cy = -m_totalDev.cy;
	}

	if (m_pageDev.cy < 0)
	{
		m_pageDev.cy = -m_pageDev.cy;
	}

	if (m_lineDev.cy < 0)
	{
		m_lineDev.cy = -m_lineDev.cy;
	}
											// now adjust device specific sizes

	ASSERT( m_totalDev.cx >= 0 && m_totalDev.cy >= 0 );

	if (m_pageDev.cx == 0)
	{
		m_pageDev.cx = m_totalDev.cx / 10;
	}

	if (m_pageDev.cy == 0)
	{
		m_pageDev.cy = m_totalDev.cy / 10;
	}

	if (m_lineDev.cx == 0)
	{
		m_lineDev.cx = m_pageDev.cx / 10;
	}

	if (m_lineDev.cy == 0)
	{
		m_lineDev.cy = m_pageDev.cy / 10;
	}

}



void ChScrollWnd::ScrollToDevicePosition( POINT pt )
{
	ASSERT( pt.x >= 0 );
	ASSERT( pt.y >= 0 );

	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	int		xOrig = SetScrollPos( SB_HORZ, pt.x );
	int		yOrig = SetScrollPos( SB_VERT, pt.y );

	DoScrollWindow( xOrig - pt.x, yOrig - pt.y );
}


void ChScrollWnd::ScrollToPosition( POINT pt )
{
	int		xMax = GetScrollLimit( SB_HORZ );
	int		yMax = GetScrollLimit( SB_VERT );

	if (pt.x < 0)
	{
		pt.x = 0;
	}
	else if (pt.x > xMax)
	{
		pt.x = xMax;
	}

	if (pt.y < 0)
	{
		pt.y = 0;
	}
	else if (pt.y > yMax)
	{
		pt.y = yMax;
	}

	ScrollToDevicePosition( pt );
}

#if defined( WIN32 )
void ChScrollWnd::UpdateBars()
{
											/* UpdateBars may cause window to
												be resized - ignore those
												resizings */
	if (m_boolInsideUpdate)
	{
		return;								// Do not allow recursive calls
	}

											// Lock out recursion
	m_boolInsideUpdate = true;
											/* Update the horizontal to
												reflect reality.  NOTE: turning
												on/off the scrollbars will
												cause 'OnSize' callbacks */

	ASSERT( m_totalDev.cx >= 0 && m_totalDev.cy >= 0 );

	CRect		rectClient;
	bool		boolCalcClient = true;
	CWnd*		pParentWnd = GetParent();
	CSize		sizeClient;
	CSize		sizeSb;
											/* Allow parent to do inside-out
												layout first */
	if (pParentWnd != 0)
	{										/* If parent window responds to
												this message, use just client
												area for scroll bar calc --
												not "true" client area */
		bool	boolParentCalc;

		boolParentCalc = ((BOOL)pParentWnd->SendMessage( WM_RECALCPARENT, 0,
											(LPARAM)(LPCRECT)&rectClient ) != FALSE);

		if (boolParentCalc)
		{									/* Use rectClient instead of
												GetTrueClientSize for client
												size calculation */
			boolCalcClient = false;
		}
	}

	if (boolCalcClient)
	{										// Get client rect

		if (!GetTrueClientSize( sizeClient, sizeSb ))
		{
											/* No room for scroll bars (common
												for zero sized elements) */
			CRect		rect;

			GetClientRect( &rect );

			if (rect.right > 0 && rect.bottom > 0)
			{
											/* If entire client area is not
												invisible, assume we have
												control over our scrollbars */

				EnableScrollBarCtrl( SB_BOTH, false );
			}

			m_boolInsideUpdate = false;
			return;
		}
	}
	else
	{										/* Let parent window determine the
												"client" rect */

		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

											// Enough room to add scrollbars
	CSize		sizeRange;
	CPoint		ptMove;
	CSize		needSb;
	SCROLLINFO	info;

											/* Get the current scroll bar state
												given the true client area */

	GetScrollBarState( sizeClient, needSb, sizeRange, ptMove, boolCalcClient );
	if (needSb.cx)
	{
		sizeClient.cy -= sizeSb.cy;
	}

	if (needSb.cy)
	{
		sizeClient.cx -= sizeSb.cx;
	}
											/* First scroll the window as
												needed */

	ScrollToDevicePosition( ptMove );		/* Will set the scroll bar
												positions too */

	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;

											/* Now update the bars as
												appropriate */
	EnableScrollBarCtrl( SB_HORZ, needSb.cx );
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx;

		if (!SetScrollInfo( SB_HORZ, &info, true ))
		{
			SetScrollRange( SB_HORZ, 0, sizeRange.cx, true );
		}
	}

	EnableScrollBarCtrl( SB_VERT, needSb.cy );

	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy;

		if (!SetScrollInfo( SB_VERT, &info, true ))
		{
			SetScrollRange( SB_VERT, 0, sizeRange.cy, true );
		}
	}
											// Remove recursion lockout
	m_boolInsideUpdate = FALSE;
}
#else
void ChScrollWnd::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_boolInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_boolInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CRect rectClient;
	BOOL bCalcClient = TRUE;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if ((BOOL)pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = FALSE;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, FALSE);
			}
			m_boolInsideUpdate = FALSE;
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
		SetScrollRange(SB_HORZ, 0, sizeRange.cx, TRUE);

	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
		SetScrollRange(SB_VERT, 0, sizeRange.cy, TRUE);

	// Remove recursion lockout
	m_boolInsideUpdate = FALSE;
}

#endif



void ChScrollWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate( nState, pWndOther, bMinimized );

	if (WA_INACTIVE != nState)
	{
		SetFocus();
	}
}

int ChScrollWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (CWnd::OnCreate( lpCreateStruct ) == -1)
	{
		return -1;
	}
											// Perform the inital update
	OnInitialUpdate();

	return 0;
}


void ChScrollWnd::OnInitialUpdate()
{
	Invalidate( true );
}


void ChScrollWnd::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	ASSERT( pScrollBar == GetScrollBarCtrl( SB_HORZ ) );

	OnScroll( MAKEWORD( nSBCode, -1 ), nPos );

	if (SB_ENDSCROLL == nSBCode)
	{
		OnMouseUp();
	}
}


void ChScrollWnd::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	ASSERT( pScrollBar == GetScrollBarCtrl( SB_VERT ) );

	OnScroll( MAKEWORD( -1, nSBCode ), nPos );

	if (SB_ENDSCROLL == nSBCode)
	{
		OnMouseUp();
	}
}

#if _MFC_VER < 0x0700
void ChScrollWnd::CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const
{
	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	bHasVertBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_VSCROLL);

	pBar = GetScrollBarCtrl(SB_HORZ);
	bHasHorzBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_HSCROLL);
}

UINT PASCAL _AfxGetMouseScrollLines();

// UE: unfortunately it's necessary to replace this routine with an almost
//     identical copy of it under MFC 6 and earlier; without a cast to int
//     it appears that it misinterprets a negative as a large positive,
//     thereby preventing using the mouse wheel to scroll upwards.
BOOL ChScrollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// we don't handle anything but scrolling
	if (nFlags & (MK_SHIFT | MK_CONTROL))
		return FALSE;

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasVertBar && !bHasHorzBar)
		return FALSE;

	BOOL bResult = FALSE;
	UINT uWheelScrollLines = _AfxGetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	if (bHasVertBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cy;
			nDisplacement = min(nDisplacement, (int)m_pageDev.cy);
		}
		bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cx;
			nDisplacement = min(nDisplacement, (int)m_pageDev.cx);
		}
		bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}
#endif
//BOOL ChScrollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
//{
//	int lines;
//	if(!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0))
//	{
//		lines = 3;
//	}
//	for(int line = 0; line < lines; ++line)
//	{
//		if (zDelta < 0)
//		{
//			OnScroll( MAKEWORD( -1, SB_LINEDOWN ), 0 );
//		}
//		else
//		{
//			OnScroll( MAKEWORD( -1, SB_LINEUP ), 0 );
//		}
//	}
//	return TRUE;
//}

BOOL ChScrollWnd::OnScroll( UINT nScrollCode, UINT nPos, BOOL boolDoScroll )
{
											// Calc new x position
	int		x = GetScrollPos( SB_HORZ );
	int		xOrig = x;

	switch (LOBYTE( nScrollCode ))
	{
		case SB_TOP:
		{
			x = 0;
			break;
		}

		case SB_BOTTOM:
		{
			x = LONG_MAX / 2;
			break;
		}

		case SB_LINEUP:
		{
			x -= m_lineDev.cx;
			break;
		}

		case SB_LINEDOWN:
		{
			x += m_lineDev.cx;
			break;
		}

		case SB_PAGEUP:
		{
			x -= m_pageDev.cx;
			break;
		}

		case SB_PAGEDOWN:
		{
			x += m_pageDev.cx;
			break;
		}

		case SB_THUMBTRACK:
		{
			x = nPos;
			break;
		}
	}
											// Calc new y position
	int		y = GetScrollPos( SB_VERT );
	int		yOrig = y;

	switch (HIBYTE( nScrollCode ))
	{
		case SB_TOP:
		{
			y = 0;
			break;
		}

		case SB_BOTTOM:
		{
			y = LONG_MAX / 2;
			break;
		}

		case SB_LINEUP:
		{
			y -= m_lineDev.cy;
			break;
		}

		case SB_LINEDOWN:
		{
			y += m_lineDev.cy;
			break;
		}

		case SB_PAGEUP:
		{
			y -= m_pageDev.cy;
			break;
		}

		case SB_PAGEDOWN:
		{
			y += m_pageDev.cy;
			break;
		}

		case SB_THUMBTRACK:
		{
			y = nPos;
			break;
		}
	}

	BOOL	boolResult = OnScrollBy( CSize( x - xOrig, y - yOrig ),
										boolDoScroll );
	if (boolResult && boolDoScroll)
	{
		UpdateWindow();
	}

	return boolResult;
}


BOOL ChScrollWnd::OnScrollBy( CSize sizeScroll, BOOL boolDoScroll )
{
	chint32		lOrigX;
	chint32		lMaxX;
	chint32		lX;
	chint32		lOrigY;
	chint32		lMaxY;
	chint32		lY;
											/* Don't scroll if there is no
												valid scroll range (ie. no
												scroll bar) */
	CScrollBar*		pBar;
	DWORD			dwStyle = GetStyle();

	pBar = GetScrollBarCtrl( SB_VERT );
	if ((pBar != 0 && !pBar->IsWindowEnabled()) ||
		(pBar == 0 && !(dwStyle & WS_VSCROLL)))
	{
											// Vert. scroll bar not enabled
		sizeScroll.cy = 0;
	}

	pBar = GetScrollBarCtrl( SB_HORZ );
	if ((pBar != 0 && !pBar->IsWindowEnabled()) ||
		(pBar == 0 && !(dwStyle & WS_HSCROLL)))
	{
											// Horiz. scroll bar not enabled
		sizeScroll.cx = 0;
	}
											// Adjust current x position
	lOrigX = lX = GetScrollPos( SB_HORZ );
	lMaxX = GetScrollLimit( SB_HORZ );

	lX += sizeScroll.cx;
	if (lX < 0)
	{
		lX = 0;
	}
	else if (lX > lMaxX)
	{
		lX = lMaxX;
	}
											// Adjust current y position
	lOrigY = lY = GetScrollPos( SB_VERT );
	lMaxY = GetScrollLimit( SB_VERT );

	lY += sizeScroll.cy;
	if (lY < 0)
	{
		lY = 0;
	}
	else if (lY > lMaxY)
	{
		lY = lMaxY;
	}
											// Did anything change?
	if (lX == lOrigX && lY == lOrigY)
	{
		return false;
	}

	if (boolDoScroll)
	{										/* Do scroll and update scroll
												positions */

		DoScrollWindow( -(lX - lOrigX), -(lY - lOrigY) );
		if (lX != lOrigX)
		{
			SetScrollPos( SB_HORZ, lX );
		}

		if (lY != lOrigY)
		{
			SetScrollPos( SB_VERT, lY );
		}
	}

	return true;
}


void ChScrollWnd::OnSize( UINT nType, int cx, int cy )
{
	CWnd::OnSize( nType, cx, cy );	  

											/* UpdateBars() handles locking out
												recursion */
	UpdateBars();
}


int ChScrollWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	SetFocus();

	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


void ChScrollWnd::OnPaint() 
{
	CPaintDC dc( this );					// Device context for painting
	
	OnPrepareDC( &dc );
	OnDraw( &dc );
}


void ChScrollWnd::OnPrepareDC( CDC* pDC, CPrintInfo* pInfo )
{
	CPoint		ptVpOrg( 0, 0 );			// Assume no shift for printing

	ASSERT_VALID( pDC );
	ASSERT( m_totalDev.cx >= 0 && m_totalDev.cy >= 0 );

	if (!pDC->IsPrinting())
	{
		ASSERT( pDC->GetWindowOrg() == CPoint( 0, 0 ) );

											/* By default shift viewport origin
												in negative direction of
												scroll */
		ptVpOrg = -GetDeviceScrollPosition();
	}

	pDC->SetViewportOrg( ptVpOrg );
}


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA    
#endif

// Local Variables: ***
// tab-width:4 ***
// End: ***
