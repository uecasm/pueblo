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
	the License at http://pueblo.sf,net/APL/

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

#include <iostream>
#include <fstream>

#include <ChConst.h>

#include <ChUtil.h>

#include <ChImgUtil.h>
#include <ChTxtWnd.h>
#include <ChHtmWnd.h>

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

CBrush 		ChTxtWnd::m_defBkColor;

#endif

#if (defined( CH_MSW ) && defined( CH_ARCH_16 ) ) || ( defined( CH_UNIX ) )

#define HEAP_ZERO_MEMORY 0x00000001


CH_GLOBAL_FUNC( HANDLE )
HeapCreate( int a, int b, int c );

CH_GLOBAL_FUNC( void* )
HeapAlloc( HANDLE hHeap, chuint32 flags, chuint32 luLen );

CH_GLOBAL_FUNC( int )
HeapDestroy( HANDLE hHeap );

CH_GLOBAL_FUNC( void* )
HeapReAlloc( HANDLE hHeap, chuint32 flags, void *oldbuf, chuint32 luNewSize );

CH_GLOBAL_FUNC( void )
HeapFree( HANDLE hHeap, chuint32 flags, void *buf );

CH_GLOBAL_FUNC( chuint32 )
HeapSize( HANDLE hHeap, chuint32 flags, void *buf );

#endif // defined( CH_MSW ) && defined( CH_ARCH_16 )


/*----------------------------------------------------------------------------
	ChTxtWnd class
----------------------------------------------------------------------------*/

ChTxtWnd::ChTxtWnd()
{
	m_hHeap			= 0;
	m_pDC			= 0;
	m_iExtraHeight	= 0;
	m_lTextCount	= 0;
	m_pstrText		= 0;
	m_lRunCount		= 0;
	m_pRunsTbl		= 0;
	m_lLineCount	= 0;
	m_pLinesTbl		= 0;
	m_lObjCount		= 0;
	m_pObjTbl		= 0;
	m_lStyleCount	= 0;
	m_pStyleTbl		= 0;
	m_lTextLimit	= 0;					// The default line limit
	m_lTextBlkSize	= 0;
	m_boolDeletedText = false;
	m_viewSize.cx = m_viewSize.cy = 0;
	m_boolInSelMode	= false;
	m_lSelStart = m_lSelEnd = 0;
	m_boolRedrawAll = 0;
	m_lObjTblSize   = objTblGrowSize;
	m_pstrLogBuffer = 0;
	m_lStartUpdate  = 0;
	
	m_idTimer = 0;				// Timer ID
	m_uExpostureTime = 0;		// Timer resolution

	#if defined( CH_MSW )
	m_pbkImage		= 0;
	m_pbackGround   = 0;
	#endif

	// file logging inits
	m_luLogOptions = 0;
	m_boolLog = 0;
	m_plogStream = 0;

	// Document indents
	m_viewIndents.left = m_viewIndents.top =
	m_viewIndents.right = m_viewIndents.bottom = 0;




	#if defined( CH_MSW )
	m_luSelTextColor = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
	m_brSelBackColor.CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
	#else
		m_luSelTextColor = 0;
		cerr << "color for drawing selected text" << __FILE__ << ":" << __LINE__ << endl;
	#endif

	// This variable should be set after reading the user preference
	// set it to true if the view should scroll when the bottom line is not visible and there
	// is a append
	m_luDocAttrs = 0;

#ifdef CH_UNIX
	m_boolInitialUpdate = true;
	InitView();
#endif

}

ChTxtWnd::~ChTxtWnd()
{
	::delete m_plogStream;
	delete m_pstrLogBuffer;
}


#if defined( CH_MSW )

BEGIN_MESSAGE_MAP(ChTxtWnd, ChScrollWnd)
	//{{AFX_MSG_MAP(ChTxtWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_PALETTECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
//	ON_WM_QUERYNEWPALETTE()
END_MESSAGE_MAP()

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	ChTxtWnd public methods
----------------------------------------------------------------------------*/

void ChTxtWnd::ClearPage()
{
	if ( m_idTimer )
	{
		KillTimer( m_idTimer );		 
		m_idTimer = 0;
	}

	DeleteText( 0, ChTxtWnd::GetTextCount() );

	#if defined( CH_MSW )
	{
		::SetCursor( ::LoadCursor( 0, IDC_ARROW ) );

											// Remove any old bkcolor
		if (m_bodyBkColor.GetSafeHandle())
		{
			m_bodyBkColor.DeleteObject();
		}
											// Delete the background
		delete m_pbkImage;
		m_pbkImage = 0;

		delete m_pbackGround;
		m_pbackGround = 0;
	}
	#endif	// defined( CH_MSW )

}


void ChTxtWnd::AllowScroll( bool boolHorizontal, bool boolVertical )
{
	m_luDocAttrs &= ~(docNoHorzScroll | docNoVertScroll);

	m_luDocAttrs  |= (boolHorizontal ? 0 : docNoHorzScroll );
	m_luDocAttrs  |= (boolVertical ? 0 : docNoVertScroll );

	// New scrollbar preference
	if ( GetSafeHwnd() )
	{
		ChSize size;

		GetTotalCanvasSize( size );

		if ( m_lLineCount && size.cy  > GetViewHeight() )
		{
			if ( !(m_luDocAttrs & docNoVertScroll) )
			{
				EnableScrollBar( SB_VERT, ESB_ENABLE_BOTH );
			}
			ShowScrollBar( SB_VERT, !(m_luDocAttrs & docNoVertScroll) );
		}
		else
		{
			ShowScrollBar( SB_VERT, false);
		}

		if ( size.cx > GetViewWidth() )
		{
			if ( !(m_luDocAttrs & docNoHorzScroll) )
			{
				EnableScrollBar( SB_HORZ, ESB_ENABLE_BOTH );
			}
			ShowScrollBar( SB_HORZ, !(m_luDocAttrs & docNoHorzScroll) );
		}
		else
		{
			ShowScrollBar( SB_HORZ, false );
		}
	}
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetDocumentSize()

------------------------------------------------------------------------------

Returns total height of all lines in the view.
----------------------------------------------------------------------------*/

void ChTxtWnd::GetDocumentSize( ChSize& sizeTotal )
{
	pChLine 	pLine  = GetLineTable();


	sizeTotal.cx = 0;
	sizeTotal.cy = 0;


	for ( int lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex++ )
	{
		if ( !( pLine->luLineAttr & ChTxtWnd::objAttrBreak ) )
		{
			if ( pLine->iX + pLine->iTotalWidth > sizeTotal.cx )
			{
				sizeTotal.cx = pLine->iTotalWidth;
			}
		}

		if ( (pLine->iY + pLine->iMaxHeight) > sizeTotal.cy )
		{
			sizeTotal.cy = pLine->iY + pLine->iMaxHeight;
		}
		pLine += 1;
	}

	sizeTotal.cx += ( m_viewIndents.left + m_viewIndents.right);


	if ( !GetLineCount() )
	{
		sizeTotal.cy = 0;
	}
	return;
}

/*----------------------------------------------------------------------------
												   reate
	FUNCTION	||	ChTxtWnd::SetViewIndents()

------------------------------------------------------------------------------

Sets the indents for the view

     -----------------------------
    |                             |
    |     TEXTTEXTTEXTTEXTTEXT    |
    |     TEXTTEXTTEXTTEXTTEXT    |
    |     TEXTTEXTTEXTTEXTTEXT    |
    |     TEXTTEXTTEXTTEXTTEXT    |
    |                             |
     -----------------------------

----------------------------------------------------------------------------*/

void ChTxtWnd::SetViewIndents( const ChRect& rtIndents )
{
	// Set the new indents
 	m_viewIndents = rtIndents;

	// recompute line table
	UpdateAppendText( 0, false );
}



/*----------------------------------------------------------------------------
	ChTxtWnd::GetCurrentBkBrush
------------------------------------------------------------------------------
	Returns the current brush used by the document.
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

CBrush* ChTxtWnd::GetCurrentBkBrush(  )
{
	CBrush* pBrush = 0;
	//HBRUSH  hbrBack;
	//ChWnd*		pParent = GetParent();

	// UE: Removed this code because we DON'T want the window to use the parent's
	//     idea of a good colour - it should use those set in the preferences.
	//     Especially as the foreground colour is taken from the prefs.
/*
	if ( pParent )
	{		
		hbrBack = (HBRUSH)pParent->
				SendMessage( WM_CTLCOLORDLG, 
					(WPARAM)GetContext()->GetSafeHdc(), 
					(LPARAM)m_hWnd );
	}

	if (hbrBack)
	{
		pBrush = CBrush::FromHandle( hbrBack );
	}
	else */ if ( m_bodyBkColor.GetSafeHandle() )
	{
		pBrush = &m_bodyBkColor;
	}
	else if ( m_defBkColor.GetSafeHandle() )
	{
		pBrush = &m_defBkColor;
	}
	
	return pBrush;
}

#endif

/*----------------------------------------------------------------------------
	ChTxtWnd::SetBackColor
------------------------------------------------------------------------------
	Sets the bkbrush for for the document.
----------------------------------------------------------------------------*/

void ChTxtWnd::SetBackColor( chuint32 luColor )
{
	#if defined( CH_MSW )
	if ( m_bodyBkColor.GetSafeHandle() )
	{
		m_bodyBkColor.DeleteObject();
	}
	CClientDC dc( this );

	COLORREF luNewColor =  dc.GetNearestColor(luColor);

	if ( luColor != luNewColor )
	{
	 	CreateBackground( luColor );
	}
	else
	{
		m_bodyBkColor.CreateSolidBrush( luColor );
	}

	InvalidateRect( NULL, true );

	#endif
}

/*----------------------------------------------------------------------------
	ChTxtWnd::SetBackColor
------------------------------------------------------------------------------
	Sets the bkbrush for for the document.
----------------------------------------------------------------------------*/

void ChTxtWnd::SetBackPattern( ChDib* pDib )
{
	#if defined( CH_MSW )

	m_pbkImage = pDib;

 	CreateBackground( m_pbkImage );


	InvalidateRect( NULL, true );

	#endif
}





/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::AppendText

------------------------------------------------------------------------------

	This method appends the text to the view buffer and sets the style to
	pNewStyle if specified else the style of the last run is extended.  The
	method also deletes lines at the top if the append causes the total
	lines in the view to exceed the line limit set.

----------------------------------------------------------------------------*/

bool ChTxtWnd::AppendText( const char* pstrNewText, chint32 lNewCount,
								ChTextStyle *pNewStyle )
{
	if (lNewCount == 0)
	{
		return ( false );
	}

	chint32		lStartRun;
	chint32		lStartChar;
	bool		boolOK = false;
 	bool		boolDisplayAppend = false;
  	ChPoint 	ptPos;
	ChRect		rtUpdateRect( 0, 0,
				GetViewWidth(), GetViewHeight() );

	if (lNewCount == -1)
	{
		lNewCount = lstrlen( pstrNewText );
	}

	ptPos = GetDeviceScrollPosition();

	if (AlwaysDisplayAppend())
	{

		// If we are currently at the bottom of the view or
		// if the view has no scroll bars we will display what we append.

		if ( ((ptPos.y + GetViewHeight()) ==  GetCanvasHeight() ) // Are we at the bottom of the view
					|| (GetCanvasHeight() <= GetViewHeight() ))   // Do we have scrollbars
		{
			boolDisplayAppend = true;
		}
	}


	lStartChar = AppendTextBuffer( pstrNewText, GetTextCount(), lNewCount );
	{
		ChClientDC	dc( this );

		m_pDC = &dc;
 		// AppendTextBuffer enforces buffer limit which can change the counts,
		// update starts
		lStartRun  = GetRunCount() - 1;

		if ( !pNewStyle  )
		{
			pChRun 	 	 pRun 		= GetRunTable();
			pChStyleInfo pStyleUse 	= GetStyleTable();
			// if the last style is an object and we have no style
			// then add new text with default style
			if ( lStartRun > -1 &&
				!( pStyleUse[ pRun[lStartRun].lStyleIndex ].style.luStyle
											& ChTxtWnd::textObject ) )
			{  	// We are extending the current run.
				boolOK = true;
			}
			else if ( ChangeStyle( GetDefaultStyle(), lStartRun, lStartChar,
									(GetTextCount() - lStartChar) ))
			{  // first run and no style specified use default
				boolOK = true;
			}
		}
		else
		{
			ChStyle style;
			// Get the style information
			style.iFontIndex = GetFont( pNewStyle->GetFont() );

			#if defined( CH_MSW )
			{
				chuint32 luColor = pNewStyle->GetTextColor();
				if ( !(luColor & CH_COLOR_DEFAULT) )
				{
					style.lColor = luColor;
				}
				else
				{
					style.lColor = luColor;
				}
				luColor = pNewStyle->GetBackColor();
				if ( !(luColor & CH_COLOR_DEFAULT) )
				{
					style.lBackColor = luColor;
				}
				else
				{
					style.lBackColor = luColor;
				}
			}
			#else
			style.lColor = pNewStyle->GetTextColor();
			style.lBackColor = pNewStyle->GetBackColor();
			#endif
			style.luStyle   = pNewStyle->GetStyle();
			style.iLeftIndent  = pNewStyle->GetLeftIndent( );
			style.userData    = pNewStyle->GetUserData();
			style.iObjectIndex = -1;

			if (ChangeStyle( &style, lStartRun, lStartChar, (GetTextCount() - lStartChar) ))
			{
				boolOK = true;
			}
		}

		if (boolOK)
		{
			boolOK = UpdateLineTable( lStartChar,
						(GetTextCount() - lStartChar), vwInsert );
		}
	}

	// get the location of the first character appended
	chint32 lUpdateHeight;
	GetLineIndex( lStartChar, lUpdateHeight );

	if ( boolOK && (boolDisplayAppend || AlwaysDisplayAppend() ) )
	{
		// check if the last line appended is visible, if not scroll the view to update
		if ( m_sizeTotal.cy > (ptPos.y +  GetViewHeight()) )
		{ // The last line is not visible, scroll enough to make it visible

			ptPos.y = m_sizeTotal.cy;
			ScrollToPosition( ptPos );  // scroll to view
			// updated x,y position
			rtUpdateRect.top =  (int)(GetViewHeight() -
											(m_sizeTotal.cy - lUpdateHeight));
			rtUpdateRect.bottom = GetViewHeight();
		}
		else
		{	// compute the rect to update
			rtUpdateRect.top = (int)lUpdateHeight > ptPos.y
											? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
		}
	}
	else
	{
		rtUpdateRect.top = (int)lUpdateHeight > ptPos.y
										? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
	}
	// draw the new append if it is visible
	if ( rtUpdateRect.top < rtUpdateRect.bottom )
	{
		InvalidateRect( &rtUpdateRect, false );
	}
	return (boolOK);
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::AppendTextRun

------------------------------------------------------------------------------

	This method appends the text to the view buffer and sets the style to
	pNewStyle if specified else the style of the last run is extended. This method
	does not update the line table, call UpdateLineTable to commit the changes.

----------------------------------------------------------------------------*/

bool ChTxtWnd::AppendTextRun( const char* pstrNewText, chint32 lNewCount,
								ChTextStyle *pNewStyle /* = NULL */ )
{
	if (lNewCount == 0)
	{
		return ( false );
	}

	chint32		lStartRun;
	chint32		lStartChar;
	bool		boolOK = false;

	if (lNewCount == -1)
	{
		lNewCount = lstrlen( pstrNewText );
	}


	lStartChar = AppendTextBuffer( pstrNewText,  GetTextCount(), lNewCount );
	{
		ChClientDC	dc( this );

		m_pDC = &dc;
		// AppendTextBuffer enforces buffer limit which can change the counts,
		// update starts
		lStartRun  = GetRunCount() - 1;



		if ( !pNewStyle  )
		{
			pChRun 	      pRun 			= GetRunTable();
			pChStyleInfo  pStyleUse 	= GetStyleTable();

			// if the last style is an object and we have no style
			// then add new text with default style
			if ( lStartRun > -1 &&
				!( pStyleUse[ pRun[lStartRun].lStyleIndex ].style.luStyle
											& ChTxtWnd::textObject ) )
			{  	// We are extending the current run.
				boolOK = true;
			}
			else if ( ChangeStyle( GetDefaultStyle(), lStartRun, lStartChar,
									GetTextCount() - lStartChar ))
			{  // first run and no style specified use default
				boolOK = true;
			}
		}
		else
		{
			ChStyle style;
			// Get the style information
			style.iFontIndex = GetFont( pNewStyle->GetFont() );

			#if defined( CH_MSW )
			{
				chuint32 luColor = pNewStyle->GetTextColor();
				if ( !(luColor & CH_COLOR_DEFAULT) )
				{
					style.lColor = luColor;
				}
				else
				{
					style.lColor = luColor;
				}
				luColor = pNewStyle->GetBackColor();
				if ( !(luColor & CH_COLOR_DEFAULT) )
				{
					style.lBackColor = luColor;
				}
				else
				{
					style.lBackColor = luColor;
				}
			}
			#else
			style.lColor = pNewStyle->GetTextColor();
			style.lBackColor = pNewStyle->GetBackColor();
			#endif
			style.luStyle   = pNewStyle->GetStyle();
			style.iLeftIndent  = pNewStyle->GetLeftIndent( );
			style.userData    = pNewStyle->GetUserData();
			style.iObjectIndex = -1;

			if (ChangeStyle( &style, lStartRun, lStartChar,
									(GetTextCount() - lStartChar) ))
			{
				boolOK = true;
			}
		}

	}

	return (boolOK);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::UpdateAppendText

------------------------------------------------------------------------------

	This method recomputes the line table and renders the changes.

----------------------------------------------------------------------------*/

bool ChTxtWnd::UpdateAppendText( chint32 lStartChar, bool boolDisplayAppend )
{

	ChRect		rtUpdateRect( 0, 0,
				GetViewWidth(), GetViewHeight() );;
	bool		boolOK = false;

	if ( m_boolDeletedText )
	{	// if we deleted text than lets recompute the table
		lStartChar = 0;
		m_boolDeletedText = false;
	}

	if ( lStartChar >= GetTextCount() )
	{
		return true;
	}

	{
		ChClientDC	dc( this );

		m_pDC = &dc;

		boolOK = UpdateLineTable( lStartChar, GetTextCount() - lStartChar, vwInsert );
	}

	// get the location of the first character appended
	chint32 lUpdateHeight;
	GetLineIndex( lStartChar, lUpdateHeight );

	if ( boolOK && (boolDisplayAppend || AlwaysDisplayAppend() ) )
	{
		ChPoint 		ptPos;

		ptPos  = GetDeviceScrollPosition();
		// check if the last line appended is visible, if not scroll the view to update
		if ( m_sizeTotal.cy > (ptPos.y +  GetViewHeight()) )
		{ // The last line is not visible, scroll enough to make it visible

			// scroll only if the previous last line was visble
			if ( lUpdateHeight <= ( ptPos.y + GetViewHeight() ))
			{

				ptPos.y = m_sizeTotal.cy;
				ScrollToPosition( ptPos );  // scroll to view
				// updated x,y position

				rtUpdateRect.top =  (int)(GetViewHeight() -
										(m_sizeTotal.cy - lUpdateHeight));

				if ( rtUpdateRect.top < 0 )
				{
					rtUpdateRect.top = 0;
				}
			
				rtUpdateRect.bottom = GetViewHeight();
			}
			else
			{
				rtUpdateRect.top = GetViewHeight();
				rtUpdateRect.bottom = GetViewHeight();
			}
		}
		else
		{	// compute the rect to update
			ptPos = GetDeviceScrollPosition();
			rtUpdateRect.top = (int)lUpdateHeight > ptPos.y
											? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
		}
	}
	else
	{
		ChPoint ptPos = GetDeviceScrollPosition();
		rtUpdateRect.top = (int)lUpdateHeight > ptPos.y
										? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
	}
	// draw the neww append if it is visible
	if ( rtUpdateRect.top < rtUpdateRect.bottom )
	{
		InvalidateRect( &rtUpdateRect, false );
		UpdateWindow();	// immediate feedback
	}

	m_lStartUpdate = GetTextCount();
	return boolOK;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::SetStyle

------------------------------------------------------------------------------

	This method changes the style for a specified range of characters..

----------------------------------------------------------------------------*/


bool ChTxtWnd::SetStyle(  chint32 lStartChar, chint32 lCharCount,
										ChTextStyle *pNewStyle )
{
	chint32		lStartRun;
	bool		boolOK;

	if ((lStartChar < 0) || (lCharCount <= 0))
	{
		return (false);
	}
	if (lStartChar + lCharCount > GetTextCount())
	{
		return (false);
	}

	boolOK = false;

	lStartRun = GetRunIndex( lStartChar );
	ChStyle style;
	// Get the style information
	style.iFontIndex = GetFont( pNewStyle->GetFont() );

	#if defined( CH_MSW )
	{
		chuint32 luColor = pNewStyle->GetTextColor();
		if ( !(luColor & CH_COLOR_DEFAULT) )
		{
			style.lColor = luColor;
		}
		else
		{
			style.lColor = luColor;
		}
		luColor = pNewStyle->GetBackColor();
		if ( !(luColor & CH_COLOR_DEFAULT) )
		{
			style.lBackColor = luColor;
		}
		else
		{
			style.lBackColor = luColor;
		}
	}
	#else
	style.lColor = pNewStyle->GetTextColor();
	style.lBackColor = pNewStyle->GetBackColor();
	#endif
	style.iObjectIndex = -1;

	if (ChangeStyle( &style, lStartRun, lStartChar, lCharCount ))
	{
		if (UpdateLineTable( lStartChar, lCharCount, vwUpdate ))
		{
			boolOK = true;
			InvalidateRect( 0, true );

		}
	}
	return (boolOK);
}


/*----------------------------------------------------------------------------
	ChTxtWnd protected methods
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::InitView

------------------------------------------------------------------------------

	Initialize all the tables for text view, this method is called by the
	OnCreate member function.

----------------------------------------------------------------------------*/

void ChTxtWnd::InitView()
{
	m_iExtraHeight = 0;

 	// Create a heap which has no size limit
	m_hHeap = ::HeapCreate( 0, 1024 * 10, 0 );
	ASSERT( GetHeap() );

	m_lTextCount = 0;
	m_lTextBlkSize = bufferGrowSize;

	// Allocate all the tables
	m_pstrText = (pstr)HeapAlloc( GetHeap(), HEAP_ZERO_MEMORY, GetTextBufferSize() );
	ASSERT( GetTextBuffer() );

	m_lRunCount = 0;
	m_pRunsTbl = (pChRun)HeapAlloc(GetHeap(), HEAP_ZERO_MEMORY,
						(chint32)(2 * sizeof( ChRun )));
	ASSERT( GetRunTable() );

	m_lLineCount = 0;
	m_pLinesTbl = (pChLine)HeapAlloc( GetHeap(), HEAP_ZERO_MEMORY,
							(chint32)(2 * sizeof( ChLine )));
	ASSERT( GetLineTable() );

	m_lObjCount = 0;
	m_lObjTblSize   = objTblGrowSize;
	m_pObjTbl = (ChTextObject**)HeapAlloc( GetHeap(), HEAP_ZERO_MEMORY,
							(chint32)(m_lObjTblSize * sizeof( ChTextObject * )));
	ASSERT( GetObjectTable() );


	m_lStyleCount = 1;
	m_pStyleTbl = (pChStyleInfo)HeapAlloc(GetHeap(), HEAP_ZERO_MEMORY,
										(chint32)sizeof( ChStyleInfo ));
	ASSERT( GetStyleTable() );



	// Create the default font
	ChMemClearStruct( GetDefaultStyle() );

	// Default font is at index 0;
	m_defStyle.iFontIndex = 0;


	m_lFontCount = fontTblGrowSize;
	m_pFontTbl = (pChFontInfo)HeapAlloc(GetHeap(), HEAP_ZERO_MEMORY,
										(chint32)sizeof( ChFontInfo ) * m_lFontCount );
	ASSERT( GetFontTable() );

	m_pFontTbl[0].iUseCount = 1;
	m_pFontTbl[0].pFont = (ChFont *)new ChFont;

#ifdef CH_MSW
	::GetObject( ::GetStockObject( SYSTEM_FONT ), sizeof( LOGFONT ), &m_pFontTbl[0].fontInfo );
	m_defStyle.lColor = COLOR_DEF_TEXT;	//::GetSysColor( COLOR_WINDOWTEXT );
#else
	extern Widget formWidget;

	if (formWidget)
		m_pFontTbl[0].fontInfo = ChFontCacheLoadQueryFont( XtDisplay( formWidget ), "-*-times-medium-r-*-*-*-140-*-*-*-*-iso8859-*" );
	m_defStyle.lColor = CH_COLOR_DEFAULT;
#endif
	m_pFontTbl[0].pFont->CreateFontIndirect( &m_pFontTbl[0].fontInfo );
	m_defStyle.lBackColor = CH_COLOR_DEFAULT;
	m_defStyle.luStyle	  = ChTxtWnd::textLeft;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::TermView

------------------------------------------------------------------------------

	This method does all the cleanup, this is called by the OnDestroy member
	function.

----------------------------------------------------------------------------*/

void ChTxtWnd::TermView()
{
	if ( m_idTimer )
	{
		KillTimer( m_idTimer );		 
		m_idTimer = 0;
	}

	ClearAll();		// Cleanup all tables, and delete all resources allocated

	delete 	GetFontTable()[0].pFont;

	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	::HeapFree( GetHeap(), 0, GetTextBuffer() );
	::HeapFree( GetHeap(), 0, GetLineTable() );
	::HeapFree( GetHeap(), 0, GetRunTable() );
	::HeapFree( GetHeap(), 0, GetObjectTable() );
	::HeapFree( GetHeap(), 0, GetStyleTable() );
	::HeapFree( GetHeap(), 0, GetFontTable() );
	#endif


	::HeapDestroy( GetHeap() );
}




/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::DeleteText()

------------------------------------------------------------------------------

	Deletes a range of characters from the text buffer and updates all the
	tables.

----------------------------------------------------------------------------*/

bool ChTxtWnd::DeleteText( chint32 lStartChar, chint32 lCharCount )
{
	chint32		lStartRun;
	chint32		lNewStyleRun;
	ChStyle		myStyle;
	bool		boolOK;

	if ((lStartChar < 0) || (lCharCount <= 0))
	{
		return (false);
	}
	if (lStartChar + lCharCount > GetTextCount())
	{
		return (false);
	}

	boolOK = false;

	if ((lStartChar == 0) && (lCharCount >= GetTextCount()))
	{	// Buffer is empty, cleanup
		ClearAll();
		return ( true );
	}

	lStartRun = GetRunIndex( lStartChar );

	lNewStyleRun = GetRunIndex( (lStartChar + lCharCount < GetTextCount())
		? (lStartChar + lCharCount) : ( lStartChar - 1 ) );

	myStyle = GetStyleTable()[ GetRunTable()[ lNewStyleRun ].lStyleIndex ].style;

	if (ChangeStyle( &myStyle, lStartRun, lStartChar, lCharCount ))
	{
		lStartRun = GetRunIndex( lStartChar );

		if (RemoveText( lStartChar, lCharCount ))
		{
			UpdateRunStarts(  lStartRun, -lCharCount );

			if (UpdateLineTable(  lStartChar, lCharCount, vwDelete ))
			{
				boolOK = true;
				m_lStartUpdate = GetTextCount();
			}
		}
	}

	if ( m_lSelStart != m_lSelEnd )
	{// if there is selection then check the ranges after delete

		if ( m_lSelStart > m_lSelEnd )
		{ //normalize
			chint32 lTemp = m_lSelEnd;
			m_lSelEnd = m_lSelStart;
			m_lSelStart = lTemp;
		}
		// update the ranges
		if ( m_lSelStart > GetTextCount() )
		{	// remove selection
			m_lSelStart = m_lSelEnd = 0;
		}
		else if ( m_lSelEnd > GetTextCount() )
		{  // update the sel end
			m_lSelEnd = GetTextCount();
		}
	}

	return (boolOK);
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::InsertText()

------------------------------------------------------------------------------

	This method inserts or appends the view text buffer with the speciifed
	number of characters.

----------------------------------------------------------------------------*/

bool ChTxtWnd::InsertText( const char* pstrNewText, chint32 lStartChar,
							chint32 lNewCount )
{

	if ( lNewCount <= 0 )
	{
		return false;
	}

	if (GetTextBufferSize() <=
			(GetTextCount() + lNewCount + (int)sizeof( char )))
	{
		while (GetTextBufferSize() <=
				(GetTextCount() + lNewCount + (int)sizeof( char )))
		{
											/* Grow the buffer in
												'bufferGrowSize' blocks */
			m_lTextBlkSize += bufferGrowSize;
		}

		m_pstrText = (pstr)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY,
										GetTextBuffer(),
										GetTextBufferSize() );
		if (!GetTextBuffer())
		{
			return false;
		}
	}
	// make room for the new text
	ChMemMove( GetTextBuffer() + lStartChar + lNewCount,
				GetTextBuffer() + lStartChar,
				(chuint16)(GetTextCount() - lStartChar) );

	int iNewLines = 0;
											/* Process for new-line -
												carriage return */
	if (lNewCount > 1 )
	{
		chint32		lIndex = 0;

		while (lIndex < lNewCount)
		{
			if ( lIndex && (pstrNewText[lIndex - 1] == TEXT( '\r' )) &&
				(pstrNewText[lIndex] == TEXT( '\n' )))
			{
				lIndex++;					// Skip the newline char
				iNewLines++;
			}
			else
			{
			 	m_pstrText[lStartChar++] = pstrNewText[lIndex++];
			}
		}
	}
	else
	{
	 	m_pstrText[lStartChar++] = pstrNewText[0] ==
	 						TEXT( '\n' ) ? TEXT( '\r' ) : pstrNewText[0];
	}

	if ( iNewLines )
	{ // we had some \r\n 's

		ChMemMove( GetTextBuffer() + lStartChar,
				GetTextBuffer() +  iNewLines,
				(GetTextCount() + lNewCount - lStartChar) );
		lNewCount -= iNewLines;

	}

	GetTextBuffer()[ GetTextCount() + lNewCount ] = 0;

	m_lTextCount += lNewCount;

	// Update the line and run table
	chint32 lStartRun = GetRunIndex( lStartChar );
	UpdateRunStarts(  lStartRun, lNewCount );

	// Update line starts
	chint32 lAboveHt;
	chint32 lStartLine = GetLineIndex( lStartChar, lAboveHt );
	UpdateLineStarts(  lStartLine, lNewCount );


	// do we have a text limit

	if ( GetBufferLimit() &&  GetTextCount()  > GetBufferLimit() )
	{  // this will exceed the limit
		// when we delete the text we delete 30% more than the limit so that
		// appends after this will have some room to add without recomputing all
		// the tables.
		chint32 lDelCount = GetTextCount()  - GetBufferLimit() +
									( (30 * GetBufferLimit() )/100 );
		if ( lDelCount >= GetTextCount() )
		{
			lDelCount = GetTextCount();
		}

		{
			ChClientDC	dc( this );

			m_pDC = &dc;
			DeleteText( 0, lDelCount );
		}

		// update the wnd
		// In case of delete we shall repaint the entire window.
		InvalidateRect( NULL, true );
	}

	return true;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::AppendTextBuffer()

------------------------------------------------------------------------------

	This method inserts or appends the view text buffer with the speciifed
	number of characters. Returns the start of insertion.

----------------------------------------------------------------------------*/
chint32 ChTxtWnd::AppendTextBuffer( const char* pstrNewText, chint32 lStartChar,
									chint32 lNewCount )
{

	if ( lNewCount <= 0 )
	{
		return false;
	}

	// On Win95 and Win3.1 the max canvas size is 32768 we will limit the size to
	// 30,000.

	chint32 lDelCount = 0;
	if ( ChUtil::GetSystemType() != CH_SYS_WINNT && GetCanvasHeight() > 30000 )
	{ // Truncate the canvas to 20000
		chint32		lLineIndex;
		pChLine 	pLine  = GetLineTable();

		for (lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex += 1)
		{
			if ( GetCanvasHeight() - pLine->iY < 20000 )
			{
				lDelCount = pLine->lStartChar;
				break;	
			}
			pLine += 1;
		}


	}
	// do we have a text limit

	if ( GetBufferLimit() &&  GetTextCount() + lNewCount > GetBufferLimit() )
	{  // this will exceed the limit
		// when we delete the text we delete 30% more than the limit so that
		// appends after this will have some room to add without recomputing all
		// the tables.
		lDelCount = GetTextCount() + lNewCount - GetBufferLimit() +
									( (40 * GetBufferLimit() )/100 );
		if ( lDelCount >= GetTextCount() )
		{
			pstrNewText += ( lDelCount - GetTextCount() );
			lNewCount -= ( lDelCount - GetTextCount() );
			lDelCount = GetTextCount();
		}
		else
		{	// Truncate to the begining of a new line of optimal performance
			chint32		lLineIndex;
			pChLine 	pLine  = GetLineTable();

			for (lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex += 1)
			{
				if ( pLine->lStartChar >  lDelCount )
				{
					lDelCount = pLine[1].lStartChar;
					break;	
				}
				pLine += 1;
			}

		}
	}

	if ( lDelCount > 0 )
	{
		{
			ChClientDC	dc( this );

			m_pDC = &dc;
			DeleteText( 0, lDelCount );
		}

		// update the wnd
		// In case of delete we shall repaint the entire window.
		InvalidateRect( NULL, true );

		// Start char has changed
		lStartChar =  GetTextCount();
		m_boolDeletedText = true;
	}

	if (GetTextBufferSize() <=
			(GetTextCount() + lNewCount + (int)sizeof( char )))
	{
		while (GetTextBufferSize() <=
				(GetTextCount() + lNewCount + (int)sizeof( char )))
		{
											/* Grow the buffer in
												'bufferGrowSize' blocks */
			m_lTextBlkSize += bufferGrowSize;
		}

		m_pstrText = (pstr)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY,
										GetTextBuffer(),
										GetTextBufferSize() );
		if (!GetTextBuffer())
		{
			return false;
		}
	}


											/* Process for new-line -
												carriage return */
	if (lNewCount > 1 )
	{
		chint32		lIndex = 0;

		while (lIndex < lNewCount)
		{

			if ( lIndex && (pstrNewText[lIndex - 1] == TEXT( '\r' )) &&
				(pstrNewText[lIndex] == TEXT( '\n' )))
			{
				lIndex++;					// Skip the newline char
			}
			else
			{
			 	m_pstrText[m_lTextCount++] = pstrNewText[lIndex++];
			}

			if ( m_lTextCount > 1 && m_pstrText[m_lTextCount - 1] == TEXT( '\r' )
							&& m_pstrText[m_lTextCount - 2]  == TEXT( '\t' ) )
			{
			 	m_pstrText[m_lTextCount - 2] = TEXT( ' ' );
			}

		}
	}
	else
	{
	 	m_pstrText[m_lTextCount++] = pstrNewText[0] == TEXT( '\n' ) ? TEXT( '\r' ) : pstrNewText[0];
	}
 	GetTextBuffer()[ GetTextCount() ] = 0;

	// Update the line and run table
	chint32 lStartRun = GetRunIndex( lStartChar );
	UpdateRunStarts(  lStartRun, GetTextCount() - lStartChar );

	// Update line starts
	chint32 lAboveHt;
	chint32 lStartLine = GetLineIndex( lStartChar, lAboveHt );
	UpdateLineStarts(  lStartLine, GetTextCount() - lStartChar );

	if ( LogPlainText() )
	{
		LogToFile( pstrNewText, lNewCount );
	}

	return lStartChar;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::RemoveText()

------------------------------------------------------------------------------

	This method deletes text from view buffer.

----------------------------------------------------------------------------*/

bool ChTxtWnd::RemoveText( chint32 lStartChar, chint32 lCharCount )
{


	ChMemCopy( GetTextBuffer() + lStartChar, GetTextBuffer() + lStartChar + lCharCount,
				(chuint16)(GetTextCount() - (lStartChar + lCharCount)));
	GetTextBuffer()[ GetTextCount() - lCharCount ] = 0;

	m_lTextCount -= lCharCount;

	#if 0
	chint32	lNewSize;
	lNewSize = GetTextCount() + bufferGrowSize + sizeof( char );

	/* Do we need to resize ? */
	if((int)((lNewSize/bufferGrowSize)) < (int)(GetTextBufferSize()/bufferGrowSize))
	{


		while ((int)((lNewSize/bufferGrowSize)) < (int)(GetTextBufferSize()/bufferGrowSize))
		{
			m_lTextBlkSize -= bufferGrowSize;
		}
		m_pstrText = (pstr)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY,
								GetTextBuffer(), GetTextBufferSize());
	}
	#endif

	return (true);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::LogToFile()

------------------------------------------------------------------------------

	This method deletes text from view buffer.

----------------------------------------------------------------------------*/

bool ChTxtWnd::LogToFile( const char* pstrBuffer, chint32 lCount )
{
 	int 	iIndex = 0;

	while ( iIndex < lCount )
	{
		if ( pstrBuffer[iIndex++] < TEXT( ' ' ) )
		{
			if (  TEXT( '\b' ) == pstrBuffer[iIndex - 1] &&
						!(writeHTML & m_luLogOptions) )
			{
				while ( iIndex < lCount && pstrBuffer[iIndex++] != TEXT( '\b' ) )
				{
				}
				// replace objects with new lines in text mode
				if ( m_iLogIndex < logBufferSize - 1 )
				{
					m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\r' );
					m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\n' );
				}
				else
				{

					long iPos = GetLogFile()->tellp();

					if ( iPos == WriteLogFile( m_iLogIndex, true ) )
					{	// unable to write to log file
						CloseFile();
						return false;
					}
					m_iLogIndex = 0;
				}
			}
			else if ( TEXT( '\r' ) == pstrBuffer[iIndex - 1] 
						||  TEXT( '\n' ) == pstrBuffer[iIndex - 1] )
			{
				if ( m_iLogIndex < logBufferSize - 1 )
				{
					m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\r' );
					m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\n' );
				}
				else
				{

					long iPos = GetLogFile()->tellp();

					if ( iPos == WriteLogFile( m_iLogIndex, 
										!(writeHTML & m_luLogOptions) ) )
					{	// unable to write to log file
						CloseFile();
						return false;
					}
					m_iLogIndex = 0;

					if ( writeHTML & m_luLogOptions )
					{
						m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\r' );
						m_pstrLogBuffer[m_iLogIndex++] = TEXT( '\n' );
					}
				}
				
			}
		}
		else
		{
			if ( m_iLogIndex < logBufferSize )
			{
				m_pstrLogBuffer[m_iLogIndex++] = pstrBuffer[iIndex - 1];
			}		
			else
			{ // check if there is a new line in the buffer	 

				long iPos = GetLogFile()->tellp();
				if ( writeHTML & m_luLogOptions )
				{
					if ( iPos == WriteLogFile( m_iLogIndex ) )
					{	// unable to write to log file
						CloseFile();
						return false;
					}
					m_iLogIndex = 0;
				}
				else
				{

					int iLast = m_iLogIndex - 1;

					while( iLast && m_pstrLogBuffer[iLast] !=  TEXT( '\n' ) )
					{
						iLast--;
					}


					if ( iLast )
					{
						iLast++;
						if ( iPos == WriteLogFile( iLast ) )
						{	// unable to write to log file
							CloseFile();
							return false;
						}
						if ( (m_iLogIndex - iLast) > 0 )
						{
							ChMemCopy( m_pstrLogBuffer, &m_pstrLogBuffer[ iLast ], m_iLogIndex - iLast );
							m_iLogIndex = m_iLogIndex - iLast;
						}
						else
						{
							m_iLogIndex = 0;
						}
					}
					else
					{	// no new line break the char at the first white space
						iLast = m_iLogIndex - 1;
						while( iLast && m_pstrLogBuffer[iLast] != TEXT( ' ' ) )
						{
							iLast--;
						}
						if ( iLast )
						{
							iLast++;
							if ( iPos == WriteLogFile( iLast, true ) )
							{	// unable to write to log file
								CloseFile();
								return false;
							}

							if ( (m_iLogIndex - iLast) > 0 )
							{
								ChMemCopy( m_pstrLogBuffer, &m_pstrLogBuffer[ iLast ], m_iLogIndex - iLast );
								m_iLogIndex = m_iLogIndex - iLast;
							}
							else
							{
								m_iLogIndex = 0;
							}
						}
						else
						{	// no space write the complete line
							if ( iPos == WriteLogFile( m_iLogIndex, true ) )
							{	// unable to write to log file
								CloseFile();
								return false;
							}
							m_iLogIndex = 0;
						}
					}
				}
				// copy the char
				m_pstrLogBuffer[m_iLogIndex++] = pstrBuffer[iIndex - 1];
			}
		}
	}

	return true;
}

chint32	 ChTxtWnd::WriteLogFile( chint32 lCount, bool boolAddNewline /*= false */)
{
	chint32 lPos = m_plogStream->write( m_pstrLogBuffer, lCount ).tellp();

	if ( boolAddNewline )
	{
		lPos = m_plogStream->write( TEXT( "\r\n" ), 2 ).tellp();
	}

	return lPos;
}


/*----------------------------------------------------------------------------
	This method will turn on logging to a file.

	WriteFile will continuously write the window text to a file.  The
	following options may be specified:
		writeAll	--	In addition to new text, all existing text
						in the buffer is written to the file.
		writeHTML	--	Output is written with HTML tags included.

	(If an file error occurs, the file will be closed.  This handles
	 disk-full errors nicely.)
----------------------------------------------------------------------------*/

bool ChTxtWnd::WriteFile( const char* pstrFilePath, chflag32 flOptions )
{

	ASSERT( 0 != pstrFilePath );
											/* First, if an existing file is
												open, close the file */	   
	int iMode = std::ios::out | std::ios::binary;
	// UE: added ios::binary because the logfile is always sent the full \r\n
	//     sequence anyway; under text mode translation this becomes \r\r\n, which
	//     is annoying, to say the least.
	// TODO: strip the extra \r's if a UNIX implementation ever happens...

	if ( flOptions & writeNew )
	{
		iMode |= std::ios::trunc;
	}
	else
	{
		iMode |= std::ios::ate;
	}

	if ( 0 == m_pstrLogBuffer )
	{
		m_pstrLogBuffer = new char[logBufferSize];
		ASSERT( m_pstrLogBuffer );
	}
	m_iLogIndex = 0;

	TRACE2("ChTxtWnd::WriteFile(%s, %d)\n", pstrFilePath, flOptions);

//	m_plogStream = ::new fstream( pstrFilePath, iMode, filebuf::sh_read );
	m_plogStream = ::new std::fstream( pstrFilePath, iMode );
	ASSERT( m_plogStream );

	if ( m_plogStream->is_open() )
	{
		TRACE0("Logging active.\n");
		m_boolLog = true;
		m_luLogOptions = flOptions;
											/* Now, open the new file and
												save to it */
		if ( m_luLogOptions & writeAll )
		{
			if ( m_luLogOptions & writeHTML )
			{ // add the <pre> tag
				LogToFile( "<pre>", 5 );
			}
		
			// log all text in current buffer to file
			if ( m_boolLog &&  LogToFile( GetTextBuffer(), GetTextCount() ) )
			{
				// terminate the <pre> tag
				if ( m_luLogOptions & writeHTML  )
				{
					LogToFile( "</pre>", 6 );
				}
			}
		}
	}
	else
	{  	// unable to open logfile.
		TRACE0("Logging failed.\n");
		CloseFile();
#ifdef CH_MSW
		ChString msg("Unable to open logfile \"");
		msg += pstrFilePath;
		msg += "\".\nPerhaps it is read-only?";
		::MessageBox(0, msg, "Not Logging", MB_OK | MB_ICONSTOP);
#endif
	}

	return m_boolLog;
}


/*----------------------------------------------------------------------------
	This method will turn off logging to a file, and close the file.
----------------------------------------------------------------------------*/

bool ChTxtWnd::CloseFile()
{

	if ( m_iLogIndex && m_plogStream )
	{
		m_plogStream->write( m_pstrLogBuffer, m_iLogIndex );
		m_plogStream->write( TEXT( "\r\n" ), 2 );

	}

	if ( m_plogStream )
	{
		::delete m_plogStream;
	}

	delete m_pstrLogBuffer;

	m_plogStream 	= 0;
	m_boolLog    	= 0;
	m_luLogOptions 	= 0;
	m_iLogIndex     = 0;
	m_pstrLogBuffer = 0;

	return true;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetRunIndex()

------------------------------------------------------------------------------

Returns -1 if no runs.  Returns the last run if the start char is past the
end of the last run.
----------------------------------------------------------------------------*/

int ChTxtWnd::GetRunIndex( chint32 lStartChar )
{
	chint32		lRunIndex;
	pChRun 		pRun = GetRunTable();

	for (lRunIndex = 0; lRunIndex < GetRunCount(); lRunIndex += 1)
	{
		if (lStartChar < pRun[1].lStartChar)
		{
			return ((int)lRunIndex);
		}
		pRun += 1;
	}
	return ((int)GetRunCount() - 1);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetRunIndex()

------------------------------------------------------------------------------

Returns -1 if no lines.  Returns the last line if the start char is past the
end of the last line.

The total height of all lines preceding the line containing start char is
returned.
----------------------------------------------------------------------------*/
int ChTxtWnd::GetLineIndex( chint32 lStartChar, chint32& lTotalHeight )
{
	chint32		lLineIndex;
	pChLine 	pLine  = GetLineTable();

	lTotalHeight = 0;

	for (lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex += 1)
	{
		lTotalHeight = pLine->iY;
		if (lStartChar < pLine[1].lStartChar)
		{
			return ((int)lLineIndex);
		}
		pLine += 1;
	}
	return ((int)GetLineCount() - 1);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetTotalCanvasSize()

------------------------------------------------------------------------------

Returns total height of all lines in the view.
----------------------------------------------------------------------------*/

void ChTxtWnd::GetTotalCanvasSize( ChSize& sizeTotal )
{
	pChLine 	pLine  = GetLineTable();


	sizeTotal.cx = GetViewWidth();
	sizeTotal.cy = 0;


	for ( int lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex++ )
	{
		if ( (pLine->iX + pLine->iTotalWidth) > sizeTotal.cx )
		{
			sizeTotal.cx = pLine->iX + pLine->iTotalWidth;
		}
		if ( (pLine->iY + pLine->iMaxHeight) > sizeTotal.cy )
		{
			sizeTotal.cy = pLine->iY + pLine->iMaxHeight;
		}
		pLine += 1;
	}

	if ( !GetLineCount() )
	{
		sizeTotal.cy = GetViewHeight();
	}
	else
	{
	 	sizeTotal.cy += m_viewIndents.bottom;
	}
	return;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::IsHotSpot()

------------------------------------------------------------------------------
	This method determines if the current x,y location is on the hotspot, if true
	then it returns true and also fills in the user data for the hotspot

	Note : x,y is in window client co-ordinate

----------------------------------------------------------------------------*/

int ChTxtWnd::PointOn( int iHitX, int iHitY, chparam& userData, ChPoint * pptRel /* = 0 */ )
{



	if (0 == GetTextCount())
	{
		return locUnknown;
	}

	// if hotspot is a ISMAP then this will be positive values
	if ( pptRel )
	{
		pptRel->x = -1;
		pptRel->y = -1;
	}

	// check if it is within our view area
	if ( iHitX < m_viewIndents.left || iHitX > ( GetViewWidth() - m_viewIndents.right) ||
		 iHitY < m_viewIndents.top  || iHitY > ( GetViewHeight() - m_viewIndents.bottom) )
	{
		return locUnknown;
	}

	pChLine			pLine;
	chint32			lStartChar,
					lIndex = 0;


	pLine 		= GetLineTable();

	ChPoint 		ptOrigin;

	// The current view position
	ptOrigin = GetDeviceScrollPosition();

	// x,y in view co-ordinates
	ptOrigin.x += iHitX;
	ptOrigin.y += iHitY;

	// skip lines till we hit the line with current y
	while ((lIndex < GetLineCount())
			&& ( pLine->iY <=  ptOrigin.y ) )
	{
	 	if ( (pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )
		{
			break;
		}
		lIndex += 1;
		pLine += 1;
	}

	if ( lIndex >= GetLineCount() )

	{ // sanity check
		return locUnknown;
	}

	// we have the line where x, y is located

	pChRun			pRun;
 	pChStyleInfo	pStyleUse;
 	pChFontInfo		pFontTbl;

	pStyleUse = GetStyleTable();
	pFontTbl  = GetFontTable();


	bool  boolHotSpot = false;

	pChLine			pLineOrig = pLine;


	while ( !boolHotSpot &&
					pLine < ( GetLineTable() + GetLineCount()) &&
					pLine->iY <=  ptOrigin.y  )
	{
		lStartChar = pLine->lStartChar;
		lIndex = GetRunIndex(  lStartChar );
	 	pRun   = GetRunTable();
		pRun   += lIndex;

		// check if we have any runs with hotspots on this line
		while (  pLine < ( GetLineTable() + GetLineCount()) &&
				((pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )
						&& lStartChar < pLine[1].lStartChar )
		{
			if ( pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textHotSpot )
			{
				boolHotSpot = true;
				break;
			}
			++pRun;	  	// next run
			lStartChar = pRun->lStartChar;
		}
		pLine++;
	}


	//if ( !boolHotSpot )
	{
	//	return false;
	}

	// This line has a hot spot are we really on the hot spot ?
	pLine = pLineOrig;

	int x;


	ChClientDC	dc( this );

	m_pDC = &dc;

	while (  pLine < ( GetLineTable() + GetLineCount()) &&
							pLine->iY <  ptOrigin.y )
	{

		lStartChar = pLine->lStartChar;
		lIndex = GetRunIndex(  lStartChar );
		pRun   = GetRunTable();
		pRun   += lIndex;

		x = pLine->iX;	// setup the left margin


//		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textIndentLeft )
//		{  // indent to the left
//			x += pStyleUse[ pRun->lStyleIndex ].style.iLeftIndent;
//		}

		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textCenter )
		{ // justify center
			//x += (( (GetViewWidth() - m_viewIndents.right - x) - pLine->iTotalWidth)/2);
			x += ( (pLine->iMaxLineWidth - pLine->iTotalWidth)/2);
		}
		else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textRight )
		{   // justify right
			//x += ( GetViewWidth() - m_viewIndents.right - x - pLine->iTotalWidth);
			x += pLine->iMaxLineWidth - pLine->iTotalWidth;
		}


		pstr	  pText;
		bool	  boolNewStyle = true;
		pText 	  = GetTextBuffer();
		pText     += lStartChar;

		while (lStartChar < pLine[1].lStartChar)
		{
			int sDrawCount;

			if (boolNewStyle)
			{	// Set up the DC for the new style
				::SelectObject(  GetContext()->m_hDC,
								pFontTbl[pStyleUse[ pRun->lStyleIndex ].
							style.iFontIndex].pFont->m_hObject );

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
			if (sDrawCount > (int)(pLine[1].lStartChar - lStartChar))
			{
				sDrawCount = (int)(pLine[1].lStartChar - lStartChar);
			}

			ChSize txtOut;
			if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
			{

				chuint32 luAttr;
				GetObjectSizeAndAttr( pStyleUse[ pRun->lStyleIndex ].style, txtOut, luAttr );


				sDrawCount = (int)(pRun[1].lStartChar - pRun[0].lStartChar);

			}
			else
			{
//				#if defined( CH_MSW )
//				txtOut =  GetContext()->GetTabbedTextExtent( pText, sDrawCount, 0, 0 );
//				#else
				txtOut =  GetContext()->GetTextExtent( pText, sDrawCount );
//				#endif
			}
			// Set up for the next run
			if (  pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textHotSpot &&
						ptOrigin.x > x &&	ptOrigin.x <= x + txtOut.cx &&
						pLine->iY <  ptOrigin.y &&
						( pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )
			{

				if ( pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textObject )
				{
					ChPoint ptTopLeft( x, pLine->iY );
					if ( PointInObject( pStyleUse[pRun->lStyleIndex].style.iObjectIndex,
										ptTopLeft, ptOrigin, pptRel ) )
					{
						if ( pptRel && !(pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textISMAP) )
						{  // not a ISMAP
							pptRel->x = -1;
							pptRel->y = -1;
						}
					}
					else
					{
						return locObject;
					}
				}
				else
				{
					if ( pLine->luLineAttr & objAttrMiddle || 
								pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textVCenter )
					{
						int iRunHeight = pStyleUse[ pRun->lStyleIndex ].iFontHeight + GetExtraHeight();
						if ( pLine->iMaxHeight > iRunHeight  )
						{  // center the text to the height of the control
							int y = pLine->iY + ((pLine->iMaxHeight >> 1) - ( iRunHeight >> 1));	

							if ( !(y <  ptOrigin.y && ( y + pLine->iMaxHeight) >= ptOrigin.y) )
							{
								return locText;
							}

						}
					}
				}
				userData = pStyleUse[ pRun->lStyleIndex ].style.userData;
				return locHotspot;
			}
			else if (  !(pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textHotSpot) &&
						ptOrigin.x > x &&	ptOrigin.x <= x + txtOut.cx &&
						pLine->iY <  ptOrigin.y &&
						( pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )
			{
				if ( pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textObject )
				{
					return locObject;
				}
				else
				{
					return locText;
				}

			}
			// go to the next run
			x += txtOut.cx;

			if ( x > ptOrigin.x && pLine[1].iY > ( ptOrigin.y + pLine->iMaxHeight ))
			{
				return locUnknown;
			}

			pText += sDrawCount;
			lStartChar += sDrawCount;
			if (lStartChar == pRun[1].lStartChar)
			{
				pRun += 1;
				boolNewStyle = true;
			}
		}
		pLine++;
	}

	return (locUnknown);
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetLocToIndex()

------------------------------------------------------------------------------
	This method gets the index of the character in the text buffer given the
	x, y location in client-co-ordinate. Returns -1 if there is no character
	at the current x,y location.

----------------------------------------------------------------------------*/

chint32 ChTxtWnd::GetLocToIndex( int x, int y )
{

	if (0 == GetTextCount())
	{
		return 0;
	}

	pChLine			pLine;
	chint32			lStartChar,
					lIndex = 0;


	pLine 		= GetLineTable();

	ChPoint 		ptOrigin;

	// The current view position
	ptOrigin = GetDeviceScrollPosition();

	// x,y in view co-ordinates
	ptOrigin.x += x;
	ptOrigin.y += y;

	// skip lines till we hit the line with current y
	while ((lIndex < GetLineCount())
			&& ( pLine->iY <=  ptOrigin.y ) )
	{
	 	if ( (pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )
		{
			if ( pLine->luLineAttr & objAttrFloat  )
			{
				if ( x >= pLine->iX	 && x <= (pLine->iX + pLine->iTotalWidth) )
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		lIndex += 1;
		pLine += 1;
	}

	if ( lIndex < GetLineCount() &&
				!(( pLine->iY <=  ptOrigin.y ) &&
			 	( (pLine->iY + pLine->iMaxHeight) >= ptOrigin.y )))

	{  // not on text
		return (pLine->lStartChar - 1) >= 0 ? pLine->lStartChar - 1 : 0;
	}


	if ( lIndex >= 	GetLineCount() )
	{
		pLine -= 1;
	}


	// we have the line where x, y is located

	pChRun			pRun;
 	pChStyleInfo	pStyleUse;
 	pChFontInfo		pFontTbl;

	pStyleUse 	= GetStyleTable();
	pFontTbl 	= GetFontTable();
 	pRun 		= GetRunTable();

	// get the run offset
	lStartChar = pLine->lStartChar;
	lIndex = GetRunIndex(  lStartChar );
	pRun += lIndex;



	if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textIndentLeft
			&& ptOrigin.x < pStyleUse[ pRun->lStyleIndex ].style.iLeftIndent )
	{  // indent to the left

		return pLine->lStartChar;
	}



	x = pLine->iX;	// setup the left margin

//	if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textIndentLeft )
//	{  // indent to the left
//		x += pStyleUse[ pRun->lStyleIndex ].style.iLeftIndent;
//	}

	if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textCenter )
	{ // justify center
		//x += (( (GetViewWidth() - m_viewIndents.right - x) - pLine->iTotalWidth)/2);
		x += ( (pLine->iMaxLineWidth - pLine->iTotalWidth)/2);
	}
	else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textRight )
	{   // justify right
		//x += ( GetViewWidth() - m_viewIndents.right - x - pLine->iTotalWidth);
		x += pLine->iMaxLineWidth - pLine->iTotalWidth;
	}

	if ( ptOrigin.x < x )
	{  // not on the text
		return pLine->lStartChar;
	}

	if ( ptOrigin.x > pLine->iTotalWidth + x )
	{  // not on any text
		return pLine[1].lStartChar;
	}


	pstr			pText;
	pText 	  = GetTextBuffer();
	pText     += lStartChar;


	ChClientDC	dc( this );

	m_pDC = &dc;


	int iWidth = x;
	int iCount = 0;

	while (	iWidth <  ptOrigin.x )
	{
		int 	iRunChars;
		ChSize  txtOut;


		::SelectObject(  GetContext()->m_hDC, pFontTbl[pStyleUse[ pRun->lStyleIndex ].
									style.iFontIndex].pFont->m_hObject );


		txtOut.cx = iRunChars = 0;

		iRunChars = (int)(pLine[1].lStartChar < pRun[1].lStartChar  ? pLine[1].lStartChar - lStartChar
														  : pRun[1].lStartChar - lStartChar);

		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{

			chuint32 luAttr;
			GetObjectSizeAndAttr( pStyleUse[ pRun->lStyleIndex ].style, txtOut, luAttr );
		}
		else
		{
//			#if defined( CH_MSW )
//			txtOut =  GetContext()->GetTabbedTextExtent( pText, iRunChars, 0, 0 );
//			#else
			txtOut =  GetContext()->GetTextExtent( pText, iRunChars );
//			#endif
		}

		if ( (txtOut.cx + iWidth)  <= ptOrigin.x )
		{
			iCount += iRunChars;
		}
		else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{
			return  pLine->lStartChar + iCount + iRunChars;
		}
		else
		{
			chint32 lLow = 1, lHigh = iRunChars;
			chint32 lMid;

			do
			{

				lMid = ( lLow + lHigh ) >> 1;

//				#if defined( CH_MSW )
//				txtOut =  GetContext()->GetTabbedTextExtent( pText, (int)lMid, 0, 0 );
//				#else
				txtOut =  GetContext()->GetTextExtent( pText, (int)lMid );
//				#endif

				if ( (txtOut.cx + iWidth)  < ptOrigin.x )
				{
				 	lLow = lMid + 1;
				}
				else if ( (txtOut.cx + iWidth)  > ptOrigin.x )
				{
					lHigh = lMid -1;
				}
				else
				{
					break;
				}
			}
			while( lLow < lHigh );

			if ( (txtOut.cx + iWidth)  > ptOrigin.x )
			{

				int iLastWidth = txtOut.cx;
				while( (lMid > 0 ) && (txtOut.cx + iWidth)  > ptOrigin.x )
				{
					lMid--;
					iLastWidth = txtOut.cx;
//					txtOut =  GetContext()->GetTabbedTextExtent( pText, (int)lMid, 0, 0 );
					txtOut =  GetContext()->GetTextExtent( pText, (int)lMid );
				}

				if ( (iWidth + (( iLastWidth - txtOut.cx ) >> 1 ) + txtOut.cx )	<= ptOrigin.x )
				{
					lMid++;
				}

				iCount += lMid;
			}
			else if ( (txtOut.cx + iWidth)  < ptOrigin.x )
			{
				int iLastWidth = txtOut.cx;
				while( (lMid < iRunChars ) && (txtOut.cx + iWidth)  < ptOrigin.x )
				{
					iLastWidth = txtOut.cx;
					lMid++;
//					txtOut =  GetContext()->GetTabbedTextExtent( pText, (int)lMid, 0, 0 );
					txtOut =  GetContext()->GetTextExtent( pText, (int)lMid );
				}

				if ( (iWidth + ((  txtOut.cx - iLastWidth ) >> 1 ) + txtOut.cx ) <= ptOrigin.x )
				{
					lMid++;
				}

				iCount += lMid;

 				//iCount += (int)max( lMid, lHigh );
			}
			else
			{
				iCount += lMid;
			}

			return  pLine->lStartChar + iCount;
		}

		iWidth += txtOut.cx;

		pText += iRunChars;
		pRun += 1;
		lStartChar = pRun->lStartChar;

		if (lStartChar >= pLine[1].lStartChar)
			break;
	}
	return ( pLine->lStartChar + iCount );

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetIndexToLoc()

------------------------------------------------------------------------------
	This method gets the index of the character in the text buffer given the
	x, y location in client-co-ordinate. Returns -1 if there is no character
	at the current x,y location.

----------------------------------------------------------------------------*/

void ChTxtWnd::GetIndexToLoc( chint32 lCharIndex, int& x, int& y, int& iLineMaxHt )
{

	if (0 == GetTextCount())
	{
		x = y = 0;
		return;
	}

	if ( lCharIndex > GetTextCount() )
	{
		lCharIndex = GetTextCount();
	}

	pChLine			pLine;
	chint32			lStartChar,
					lAboveHeight = 0,
					lIndex = 0;


	pLine 		= GetLineTable();

	lIndex = GetLineIndex( lCharIndex, lAboveHeight );
	pLine  += lIndex;

	iLineMaxHt = pLine->iMaxHeight;

	y = (int)pLine->iY;

	pChRun			pRun;
 	pChStyleInfo	pStyleUse;
 	pChFontInfo		pFontTbl;

	pStyleUse 	= GetStyleTable();
	pFontTbl 	= GetFontTable();

	lStartChar 	= pLine->lStartChar;
	lIndex 		= GetRunIndex(  lStartChar );
 	pRun 		= GetRunTable();
	pRun 		+= lIndex;

	#if 0
											/* Skip all the object at the
												begining of the line */
	while( lIndex < GetRunCount() &&
			pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
	{
		++pRun;
		++lIndex;
		if ( lStartChar < pRun->lStartChar )
		{
			lStartChar = pRun->lStartChar;
		}
	}
	#endif

	x = pLine->iX;							// setup the left margin

//	if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textIndentLeft )
//	{  // indent to the left
//		x += pStyleUse[ pRun->lStyleIndex ].style.iLeftIndent;
//	}

	if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textCenter )
	{ // justify center
		//x += (( (GetViewWidth() - m_viewIndents.right - x) - pLine->iTotalWidth)/2);
		x += ( (pLine->iMaxLineWidth - pLine->iTotalWidth)/2);
	}
	else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textRight )
	{   // justify right
		//x += ( GetViewWidth() - m_viewIndents.right - x - pLine->iTotalWidth);
		x += pLine->iMaxLineWidth - pLine->iTotalWidth;
	}

	pstr			pText;
	pText 		= GetTextBuffer();
	pText     	+= lStartChar;


	ChClientDC	dc( this );

	m_pDC = &dc;


	int iWidth = x;

	while (	lStartChar <  lCharIndex )
	{
		int 	iRunChars;
		ChSize  txtOut;


		::SelectObject(  GetContext()->m_hDC, pFontTbl[pStyleUse[ pRun->lStyleIndex ].
							style.iFontIndex].pFont->m_hObject );


		txtOut.cx = iRunChars = 0;

		iRunChars =(int)( lCharIndex < pRun[1].lStartChar  ? lCharIndex - lStartChar
														  : pRun[1].lStartChar - lStartChar);

		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{
			chuint32 luAttr;
			GetObjectSizeAndAttr( pStyleUse[ pRun->lStyleIndex ].style, txtOut, luAttr );
		}
		else
		{
//			#if defined( CH_MSW )
//			txtOut =  GetContext()->GetTabbedTextExtent( pText, iRunChars, 0, 0 );
//			#else
			txtOut =  GetContext()->GetTextExtent( pText, iRunChars );
//			#endif
		}


		iWidth += txtOut.cx;

		pText += iRunChars;
		pRun += 1;

		lStartChar = pRun->lStartChar;

		if (lStartChar >= pLine[1].lStartChar)
			break;
	}

	x = iWidth;

	return;

}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetSel( )

------------------------------------------------------------------------------
	Returns the current selection
----------------------------------------------------------------------------*/
void ChTxtWnd::GetSel( chint32& lStart, chint32& lEnd )
{
	if ( m_lSelStart > m_lSelEnd )
	{
		lStart = m_lSelEnd;
		lEnd   = m_lSelStart;
	}
	else
	{
		lStart = m_lSelStart;
		lEnd   = m_lSelEnd;
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::SetSel( )

------------------------------------------------------------------------------
	Select new text
----------------------------------------------------------------------------*/
void ChTxtWnd::SetSel( chint32 lStart, chint32 lEnd )
{
	if ( IsSelectionEnabled() )
	{
		if ( m_lSelStart != m_lSelEnd )
		{ // we have selection, remove the selection
			ChRect rtBound;
			GetBoundingRect( m_lSelStart, m_lSelEnd, rtBound );
			// Erase the rect if we are currently visible
			if ( !rtBound.IsRectEmpty() )
			{
				InvalidateRect( &rtBound );
			}
		}

		m_lSelStart = lStart;
		m_lSelEnd 	= lEnd;

		if ( m_lSelStart == - 1)
		{	// remove all selection
			m_lSelStart = m_lSelEnd = 0;
		}

		if ( m_lSelEnd == -1 )
		{   // select m_lSelStart to end of text
			m_lSelEnd = GetTextCount();
		}

		if ( m_lSelStart != m_lSelEnd )
		{ // we have  a new selection
			ChRect rtBound;
			GetBoundingRect( m_lSelStart, m_lSelEnd, rtBound );
			// Erase the rect if we are currently visible
			if ( !rtBound.IsRectEmpty() )
			{
				InvalidateRect( &rtBound );
				UpdateWindow();
			}
		}
	}
}




/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetBoundingRect( )

------------------------------------------------------------------------------
	This method gets the bounding rectangle of the text starting at lStartIndex
	to lEndIndex.

----------------------------------------------------------------------------*/

void ChTxtWnd::GetBoundingRect( chint32 lStartIndex, chint32 lEndIndex, ChRect& rtBound )
{

	ChRect rtText( 0, 0, 0, 0 );

	// make sure the range is specified right
	if ( lStartIndex > lEndIndex )
	{
		chint32 lTemp = lEndIndex;
		lEndIndex = lStartIndex;
		lStartIndex = lTemp;
	}

	// check the range bounds
	if (0 == GetTextCount() || lStartIndex > GetTextCount() )
	{
		rtBound = rtText;
		return;
	}

	if ( lEndIndex > GetTextCount() )
	{
		lEndIndex = GetTextCount();
	}

	chint32			lAboveHeight, lIndex;

	GetLineIndex( lStartIndex, lAboveHeight );

	rtText.top = (int) lAboveHeight;

	lIndex = GetLineIndex( lEndIndex, lAboveHeight );

	rtText.bottom = (int) lAboveHeight + GetLineTable()[lIndex].iMaxHeight;


	// convert the rect to client co-ordinates
	ChPoint 		ptOrigin;

	// The current view position
	ptOrigin = GetDeviceScrollPosition();

	rtText.top -= ptOrigin.y;
	rtText.bottom -= ptOrigin.y;
	rtText.right = GetViewWidth();

	rtBound = rtText;

	return;
}


#if defined( CH_MSW )

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetBoundingRgn( )

------------------------------------------------------------------------------
	This method gets the bounding rectangle of the text starting at lStartIndex
	to lEndIndex.

----------------------------------------------------------------------------*/

void ChTxtWnd::GetBoundingRgn( chint32 lStartIndex, chint32 lEndIndex, CRgn& rtBound )
{

	ChRect rtText( 0, 0, 0, 0 );

	// make sure the range is specified right
	if ( lStartIndex > lEndIndex )
	{
		chint32 lTemp = lEndIndex;
		lEndIndex = lStartIndex;
		lStartIndex = lTemp;
	}


	// check the range bounds
	if (0 == GetTextCount() || lStartIndex > GetTextCount() )
	{
		rtBound.CreateRectRgnIndirect( &rtText );
		return;
	}

	if ( lEndIndex > GetTextCount() )
	{
		lEndIndex = GetTextCount();
	}


	// The current view position
	ChPoint ptOrigin;
	ptOrigin = GetDeviceScrollPosition();

	int x, y, iMaxHt;

	GetIndexToLoc( lStartIndex,  x,  y,  iMaxHt );

	y -= ptOrigin.y;

	rtText.left = x;
	rtText.top = y;
	rtText.bottom = rtText.top + iMaxHt;
	rtText.right = GetViewWidth() - m_viewIndents.right;

	GetIndexToLoc( lEndIndex,  x,  y,  iMaxHt );

	y -= ptOrigin.y;

	if ( rtText.top == y )
	{
		rtText.right = x;
		rtBound.CreateRectRgnIndirect( &rtText );
		return;
	}
	else
	{
		rtBound.CreateRectRgnIndirect( &rtText );

		CRgn rgnTop;

		if ( rtText.bottom == y )
		{

			rtText.left = 0;
			rtText.top = y;
			rtText.bottom = rtText.top + iMaxHt;
			rtText.right = x;
			rgnTop.CreateRectRgnIndirect( &rtText );
		}
		else
		{
			CRgn rgnTmp;

			rgnTmp.CreateRectRgn( 0, rtText.bottom,
					GetViewWidth() - m_viewIndents.right, y );
			rtText.left = 0;
			rtText.top =  y;
			rtText.bottom = rtText.top + iMaxHt;
			rtText.right = x;
			rgnTop.CreateRectRgnIndirect( &rtText );

			rgnTop.CombineRgn( &rgnTop, &rgnTmp,  RGN_OR );

		}
		rtBound.CombineRgn( &rtBound, &rgnTop, RGN_OR );
		return;
	}
}


void ChTxtWnd::CopyToClipboard()
{
	if (m_lSelStart != m_lSelEnd)
	{
		if (m_lSelStart > m_lSelEnd)
		{
			chint32		lTemp = m_lSelEnd;

			m_lSelEnd = m_lSelStart;
			m_lSelStart = lTemp;
		}
	}
	else
	{
		return;								// No selection
	}
											/* Open the clipboard, and
												empty it */
    if (!OpenClipboard())
	{										// In use !!!
       	return;
	}

    EmptyClipboard();
											// Allocate memory to copy the text
	chint32			lExtra = 100;
	chint32			lSize = (m_lSelEnd - m_lSelStart) + lExtra;
	HGLOBAL			hMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, lSize );

	ASSERT( hMem );

	char*			pstrClipData = (char*)GlobalLock( hMem );
	pChRun			pRun;
 	pChStyleInfo	pStyleUse = GetStyleTable();
	chint32			lStartChar = m_lSelStart;
	chint32			lIndex = GetRunIndex( lStartChar );

 	pRun = GetRunTable();
	pRun += lIndex;

	if (pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textObject)
	{
											/* We copy only text, all other
												items like lines graphics are
												ignored */
	  	lStartChar = pRun[1].lStartChar;
		pRun++;
	}

   	chint32			lCurrIndex = 0;
											// Copy the data
	while (lStartChar < m_lSelEnd)
	{
		if (pStyleUse[pRun->lStyleIndex].style.luStyle & ChTxtWnd::textObject)
		{
											/* We copy only text, all other
												items like lines graphics are
												ignored */

			lStartChar += pRun[1].lStartChar - pRun->lStartChar;

			if ((lCurrIndex + 1) >= lSize)
			{
				lSize += lExtra;
				GlobalUnlock( hMem );
				hMem = GlobalReAlloc( hMem, lSize,
										GMEM_DDESHARE | GMEM_MOVEABLE );
				pstrClipData = (char*)GlobalLock( hMem );
			}
											// Convert objects to new lines

			pstrClipData[lCurrIndex++] = TEXT( '\r' );
			pstrClipData[lCurrIndex++] = TEXT( '\n' );
		}
		else
		{
			chint32		lCount;
			char*		pstrBuffer = GetTextBuffer();

			if (pRun[1].lStartChar < m_lSelEnd)
			{
				lCount = pRun[1].lStartChar - lStartChar;
			}
			else
			{
				lCount = m_lSelEnd - lStartChar;
			}

			for (int i = 0; i < lCount; i++)
			{
				if ((lCurrIndex + 1) >= lSize)
				{
					lSize += lExtra;
					GlobalUnlock( hMem );
					hMem = GlobalReAlloc( hMem, lSize, GMEM_DDESHARE | GMEM_MOVEABLE );
					pstrClipData = (char*)GlobalLock( hMem );
				}
				pstrClipData[lCurrIndex++] = pstrBuffer[lStartChar++];

				if (pstrBuffer[lStartChar - 1] == TEXT( '\r' ))
				{
					pstrClipData[lCurrIndex++] = TEXT( '\n' );
				}
			}
		}

		pRun++;
	}

	pstrClipData[lCurrIndex] = 0;
	GlobalUnlock( hMem );
	hMem = GlobalReAlloc( hMem, lCurrIndex + 1, GMEM_DDESHARE | GMEM_MOVEABLE );

											/* Place the handle on the
												clipboard */
    SetClipboardData( CF_TEXT, hMem );
											// Close the clipboard
    CloseClipboard();
}

#endif	// defined( CH_MSW )


void ChTxtWnd::OnSize( UINT nType, int cx, int cy )
{
	#if defined( CH_MSW )
	{
	//	ChScrollWnd::OnSize( nType, cx, cy );
	}
	#endif	// defined( CH_MSW )

	if (cx > 0 && cy > 0)
	{
		if (GetTextCount())
		{									// New view size
			bool	boolWidthChanging;
			int		iNewSize;

			if (cx < minWidth)
			{
				iNewSize = minWidth;
			}
			else
			{
				iNewSize = cx; // - GetScrollBarWidth();
			}
			boolWidthChanging = iNewSize != GetViewWidth();

			m_viewSize.cx =	iNewSize;
			m_viewSize.cy = cy;

			m_sizeTotal.cx = (m_sizeTotal.cx < GetViewWidth()) ?
									GetViewWidth() : m_sizeTotal.cx;

			bool		boolChanged = false;

			if (boolWidthChanging)
			{
				ChClientDC	dc( this );

				m_pDC = &dc;
				UpdateLineTable( 0, GetTextCount(), vwUpdate, &boolChanged );

			}
			else
			{
				SetViewSize();

			}

			ChPoint  ptPos;
			ptPos = GetDeviceScrollPosition();

			if (AlwaysDisplayAppend() && m_sizeTotal.cy > cy )
			{

				if ( ptPos.y  != m_sizeTotal.cy )
				{
					ptPos.y = m_sizeTotal.cy;
					ScrollToPosition( ptPos );  // scroll to view	  
					boolChanged = true;
				}

			}

			if ( boolWidthChanging || boolChanged)
			{							// Redraw the window
				ChRect rtUpdateRect; // = GetUpdateRect();

				rtUpdateRect.left =  0;
				rtUpdateRect.top  =  0;
				rtUpdateRect.right =  GetViewWidth();
				rtUpdateRect.bottom =  GetViewHeight();

				InvalidateRect( rtUpdateRect );
			}
		}
		else 
		{
			m_viewSize.cx = m_sizeTotal.cx = cx < minWidth ? minWidth : cx;
			m_viewSize.cy = m_sizeTotal.cy = cy;
			CSize pageSize( m_viewSize );
			pageSize.cx = 0;
			SetScrollSizesEx( m_sizeTotal, pageSize );
		}

	}
}

void ChTxtWnd::OnInitialUpdate()
{
#ifdef CH_MSW
	ChScrollWnd::OnInitialUpdate();
#endif

	// TODO: calculate the total size of this view
	{
		ChRect rtClient;

		GetClientRect( &rtClient );
		// New view size
		m_viewSize.cx =  rtClient.Width() < minWidth ? minWidth : rtClient.Width(); 
		m_viewSize.cy =  rtClient.Height();
	}

	m_sizeTotal.cx = m_viewSize.cx ;
	m_sizeTotal.cy = m_viewSize.cy ;
	SetScrollSizesEx( m_sizeTotal );
}


#if defined( CH_MSW )

/*----------------------------------------------------------------------------
	ChTxtWnd drawing
----------------------------------------------------------------------------*/

void ChTxtWnd::OnDraw(CDC* pDC)
{
	m_pDC = pDC;
	DrawRange( );
}


/*----------------------------------------------------------------------------
	ChTxtWnd message handlers
----------------------------------------------------------------------------*/

BOOL ChTxtWnd::PreCreateWindow( CREATESTRUCT& cs )
{
	HCURSOR		hCursor;

	if (!ChScrollWnd::PreCreateWindow( cs ))
	{
		return false;
	}

	hCursor = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	return true;
}


int ChTxtWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (ChScrollWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	InitView();
	return 0;
}

void ChTxtWnd::OnDestroy()
{
	ChScrollWnd::OnDestroy();

	TermView();
}



void ChTxtWnd::OnPaletteChanged(CWnd* pFocusWnd) 
{
	// TODO: Add your message handler code here
	if ( pFocusWnd != this )
	{
	   	CPalette * pPal = ChImgUtil::GetStdPalette();
    	
		pPal->UnrealizeObject();

    	CDC*	pDC = GetDC();

        pDC->RealizePalette();
        InvalidateRect( 0, false );		// Repaint the lot
		ReleaseDC( pDC );
	}
	
}

#if 0
BOOL ChTxtWnd::OnQueryNewPalette() 
{
	// TODO: Add your message handler code here and/or call default
	CPalette * pPal = ChDib::GetStdPalette();

	if ( pPal )
	{

        CDC*	pDC = GetDC();
		CPalette * pOldPal = pDC->SelectPalette( pPal, false );

        UINT	u = pDC->RealizePalette();


		if ( u )
		{  	// colors changed
        	InvalidateRect( 0, true );		// Repaint the lot
		}
		
		pDC->SelectPalette( pOldPal, true );

        pDC->RealizePalette();

        ReleaseDC( pDC );
	}


	return TRUE;
}
#endif





BOOL ChTxtWnd::OnEraseBkgnd( CDC* pDC )
{

	bool		boolErasing = false;

	if ( m_pbackGround )
	{
	 	boolErasing = true;
		DrawBackground( pDC );
	}
	else
	{
		HDC			hDC = pDC->GetSafeHdc();
		//HBRUSH		hbrFill = 0;
		CBrush*		pBrush;
		//ChWnd*		pParent = GetParent();

		// UE: Eliminated this code - we don't want the parent's idea of the
		//     colours, we want the explicit colours.
	/*
		if ( pParent )
		{		
			hbrFill = (HBRUSH)pParent->
					SendMessage( WM_CTLCOLORDLG, (WPARAM)hDC, (LPARAM)m_hWnd );
		}

		if (hbrFill)
		{
			pBrush = CBrush::FromHandle( hbrFill );
			boolErasing = true;
		}
		else*/ if (m_bodyBkColor.GetSafeHandle())
		{
			pBrush = &m_bodyBkColor;
			boolErasing = true;
		}
		else if (m_defBkColor.GetSafeHandle())
		{
			pBrush = &m_defBkColor;
			boolErasing = true;
		}

		if (boolErasing)
		{
			ChRect	rtClip;

			pDC->GetClipBox( rtClip );
			pBrush->UnrealizeObject();
			pDC->FillRect( rtClip, pBrush );
		}
	}

	return boolErasing;
}


HBRUSH ChTxtWnd::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH   hbrFill;

	if (CTLCOLOR_BTN == nCtlColor)
	{
		hbrFill = (HBRUSH)GetParent()->
						SendMessage( WM_CTLCOLORDLG, (WPARAM)pDC->GetSafeHdc(),
										(LPARAM)m_hWnd );
		if (0 == hbrFill)
		{
			if (m_bodyBkColor.GetSafeHandle())
			{
				hbrFill = (HBRUSH)m_bodyBkColor.GetSafeHandle();
			}
			else if (m_defBkColor.GetSafeHandle())
			{
				hbrFill = (HBRUSH)m_defBkColor.GetSafeHandle();
			}
			else
			{
				hbrFill = ChScrollWnd::OnCtlColor( pDC, pWnd, nCtlColor );
			}
		}
	}
	else
	{
		hbrFill = ChScrollWnd::OnCtlColor( pDC, pWnd, nCtlColor );
	}

	return hbrFill;
}


void ChTxtWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	switch (nChar)
	{
		case 'C':
		{
			if (GetKeyState( VK_CONTROL ) & 0x8000)
			{
											// Ctrl + C
				CopyToClipboard();
											// Send to parent

				GetParent()->SendMessage( WM_KEYDOWN, nChar,
											MAKELONG( (short)nRepCnt,
														(short)nFlags ) );
			}
			break;
		}

		case VK_UP:
		{
			SendMessage( WM_VSCROLL, SB_LINEUP,
							(LPARAM)GetScrollBarCtrl( SB_VERT ) );
			break;
		}

		case VK_DOWN:
		{
			SendMessage( WM_VSCROLL, SB_LINEDOWN,
							(LPARAM)GetScrollBarCtrl( SB_VERT ) );
			break;
		}

		case VK_PRIOR:
		{
			SendMessage( WM_VSCROLL, SB_PAGEUP,
							(LPARAM)GetScrollBarCtrl( SB_VERT ) );
			break;
		}

		case VK_NEXT:
		{
			SendMessage( WM_VSCROLL, SB_PAGEDOWN,
							(LPARAM)GetScrollBarCtrl( SB_VERT ) );
			break;
		}

		case VK_RIGHT:
		{
			SendMessage( WM_HSCROLL, SB_LINERIGHT,
							(LPARAM)GetScrollBarCtrl( SB_HORZ ) );
			break;
		}

		case VK_LEFT:
		{
			SendMessage( WM_HSCROLL, SB_LINELEFT,
							(LPARAM)GetScrollBarCtrl( SB_HORZ ) );
			break;
		}
	}

	ChScrollWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}


void ChTxtWnd::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	#if 0
	{
		if ((nChar == TEXT( 'c' )) || (nChar == TEXT( 'C' )) &&
			(GetKeyState( VK_CONTROL ) & 0x8000))
		{
			CopyToClipboard();
		}
	}
	#endif	// 0
}


void ChTxtWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	if (IsSelectionEnabled())
	{										// Capture the mouse
		SetCapture();

		if (m_lSelStart != m_lSelEnd)
		{									/* We have selection, remove the
												selection and start a new
												selection */
			ChRect		rtBound;

			GetBoundingRect( m_lSelStart, m_lSelEnd, rtBound );
			
											/* Erase the rect if we are
												currently visible */
			if (!rtBound.IsRectEmpty())
			{
				m_lSelStart = m_lSelEnd = 0;
				InvalidateRect( &rtBound );
				UpdateWindow();
			}
		}

		m_boolInSelMode = true;
		m_lSelStart = m_lSelEnd = GetLocToIndex( point.x, point.y );
	}

	ChScrollWnd::OnLButtonDown( nFlags, point );
}


void ChTxtWnd::OnMouseMove( UINT nFlags, CPoint point )
{
	if ((nFlags & MK_LBUTTON) && IsSelectionEnabled())
	{
		chint32		lSelEnd = GetLocToIndex( point.x, point.y );
		CRgn		rgnBound;

		if (m_boolInSelMode && (lSelEnd != m_lSelEnd))
		{
											// The new selection rect

			GetBoundingRgn( lSelEnd, m_lSelEnd, rgnBound );
			m_lSelEnd = lSelEnd;
											/* Erase the rect if we are
												currently visible */
			//if ( !rtBound.IsRectEmpty() )
			{
				InvalidateRgn( &rgnBound );
				UpdateWindow();
			}
		}
	}

	ChScrollWnd::OnMouseMove( nFlags, point );
}


void ChTxtWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	if (IsSelectionEnabled())
	{
		chint32		lSelEnd = GetLocToIndex( point.x, point.y );
		CRgn		rgnBound;

		if (m_boolInSelMode)
		{
			m_boolInSelMode = false;		// out of selection mode
			if (lSelEnd != m_lSelEnd)
			{
				GetBoundingRgn( lSelEnd, m_lSelEnd, rgnBound );
				m_lSelEnd = lSelEnd;
											/* Erase the rect if we are
												currently visible */
				//if ( !rtBound.IsRectEmpty() )
				{
					InvalidateRgn( &rgnBound );
					UpdateWindow();
				}
			}
		}
		else
		{
			m_lSelStart = m_lSelEnd = 0;
		}

		ReleaseCapture();
	}

	ChScrollWnd::OnLButtonUp( nFlags, point );

											/* Let our parent know the mouse
												just came up */
	OnMouseUp();
}


void ChTxtWnd::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	if ( m_idTimer == (int)nIDEvent )
	{ 
		UpdateAnimation(); 
	}
	else
	{
		ChScrollWnd::OnTimer(nIDEvent);
	}
}


#else	// defined( CH_MSW )


void ChTxtWnd::OnDraw( Widget widget, XtPointer client_data, XtPointer call_data )
{
	ChClientDC	dc( this );
	m_pDC = &dc;

	ChScrollView::OnDraw( widget, client_data, call_data );

	if (m_boolInitialUpdate) {
		OnInitialUpdate();
		m_boolInitialUpdate = false;
	}

	DrawRange( );
}


void ChTxtWnd::OnInput( Widget widget, XtPointer client_data, XtPointer call_data )
{
	//	cerr << "XXX ChTxtWnd::OnInput" << endl;
	ChClientDC	dc( this );
	m_pDC = &dc;

	ChScrollView::OnInput( widget, client_data, call_data );
}


void ChTxtWnd::OnLButtonUp( chuint32 nFlags, ChPoint& point )
{
	ChClientDC	dc( this );
	m_pDC = &dc;
}



#endif	// defined( CH_MSW )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
