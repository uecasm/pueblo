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

	This file consists of the interface for the ChScrollWnd view class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSCRWND_H )
#define _CHSCRWND_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#if 0
#if defined( CH_MSW ) && defined( CH_ARCH_16 )

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008

typedef struct tagSCROLLINFO
{
	UINT    cbSize;
	UINT    fMask;
	int     nMin;
	int     nMax;
	int     nPage;
	int     nPos;
} SCROLLINFO;
typedef SCROLLINFO* LPSCROLLINFO;
typedef const SCROLLINFO* LPCSCROLLINFO;
#endif

#endif


/*----------------------------------------------------------------------------
	ChScrollWnd class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChScrollWnd : public ChScrollView //CWnd
{
	public:
		static const SIZE	sizeDefault;

	public:
		ChScrollWnd();
		virtual ~ChScrollWnd();

		virtual BOOL Create( LPCTSTR lpszWindowName, DWORD dwStyle,
								const ChRect& rect, CWnd* pParentWnd,
								UINT nID = 0 );
		virtual BOOL CreateEx( LPCTSTR lpszWindowName, DWORD dwStyle,
								DWORD dwStyleEx, const ChRect& rect,
								CWnd* pParentWnd, UINT nID = 0 );

											/* Method called when mouse is
												released, either in the client
												or non-client area of this
												window */
		virtual void OnMouseUp() {}

		virtual void PageUp();
		virtual void PageDown();
		virtual void Home();
		virtual void End();

	protected:
		virtual void OnDraw( CDC* pDC ) = 0;

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChScrollWnd)
		//}}AFX_VIRTUAL

	protected:
		void DoScrollWindow( int iXDist, int iYDist );
		CPoint GetDeviceScrollPosition() const;
		void GetScrollBarSizes( CSize& sizeSb );
		void GetScrollBarState( CSize sizeClient, CSize& needSb,
								CSize& sizeRange, CPoint& ptMove,
								bool boolInsideClient );
		int GetScrollLimit( int iBar );
		bool GetTrueClientSize( CSize& size, CSize& sizeSb );

		void SetScrollSizes( SIZE sizeTotal,
								const SIZE& sizePage = sizeDefault,
								const SIZE& sizeLine = sizeDefault );
		void SetScrollSizesEx( SIZE sizeTotal,
								const SIZE& sizePage = sizeDefault,
								const SIZE& sizeLine = sizeDefault );

		void ScrollToDevicePosition( POINT pt );
		void ScrollToPosition( POINT pt );
		void ChScrollWnd::UpdateBars();

	protected:
											// Message handlers
		virtual void OnInitialUpdate();

											// Generated message map functions
	protected:
		//{{AFX_MSG(ChScrollWnd)
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnPaint();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
#if _MFC_VER < 0x0700
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
#endif
		//}}AFX_MSG

		BOOL OnScroll( UINT nScrollCode, UINT nPos, BOOL boolDoScroll = true );
		BOOL OnScrollBy( CSize sizeScroll, BOOL boolDoScroll = true );
		void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = 0 );

#if _MFC_VER < 0x0700
		void CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const;
#endif


	protected:
		static ChString	m_strClass;
		CSize			m_totalDev;			// Total size
		//CSize			m_pageDev;			// Per page scroll size
		//CSize			m_lineDev;			// Per line scroll size

		bool			m_boolInsideUpdate;

	DECLARE_MESSAGE_MAP()
};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHSCRWND_H )


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:52  uecasm
// Import of source tree as at version 2.53 release.
//
