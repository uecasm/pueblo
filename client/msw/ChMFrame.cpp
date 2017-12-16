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

	This file consists of the ChMainFrame class implementation.  This is
	the top-level frame.

----------------------------------------------------------------------------*/

#include "headers.h"

#if defined( CH_MSW )

	#include "Pueblo.h"

#endif	// defined( CH_MSW )

#include <ChConst.h>
#include <ChRMenu.h>
#include <ChMsgTyp.h>
#include <ChReg.h>
#include <ChImgUtil.h>
#include <ChUtil.h>
#include <ChHtpCon.h>
#include <ChWebTracker.h>

#include "ChClCore.h"
#if defined( CH_UNIX )

	#include "resource.h"

	#include <ChRect.h>
	#include <time.h>

#endif	// defined( CH_UNIX )

#include "ChMFrame.h"
#include "ChPrefs.h"
#include "MemDebug.h"

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define CH_SPLITTER_COLS			2

#define PROGRESS_BAR_WIDTH			40

#define CH_STATUS_COMPRESSION_PANE	1
#define CH_STATUS_TIME_ONLINE_PANE	2
#define CH_STATUS_TIME_PANE			3
#define CH_STATUS_PROGRESS_PANE		4

#if defined( CH_MSW )

CBrush	ChMainFrame::m_brClearProgress;
CBrush	ChMainFrame::m_brShowProgress;

#endif	// defined( CH_MSW )
											/* On Unix, we want a few
												functions which are down at
												the bottom of the file */
#if defined( CH_MSW )

/*----------------------------------------------------------------------------
	ChMainFrame class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChMainFrame, ChPersistentFrame )

BEGIN_MESSAGE_MAP( ChMainFrame, ChPersistentFrame )
	//{{AFX_MSG_MAP(ChMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_VIEW_PREVIOUS, OnViewPrevious)
	ON_COMMAND(ID_VIEW_SWAP_PANES, OnViewSwapPanes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SWAP_PANES, OnUpdateViewSwapPanes)
	ON_COMMAND(ID_VIEW_TOGGLE_ORIENTATION, OnViewToggleOrientation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLE_ORIENTATION, OnUpdateViewToggleOrientation)
	ON_COMMAND(ID_HELP_PUEBLO_BUG_REPORT, OnMenuHelpPuebloBugReport)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_STOP_LOADING, OnUpdateViewStopLoading)
	ON_COMMAND(ID_VIEW_STOP_LOADING, OnViewStopLoading)
	ON_COMMAND(ID_HELP_ABOUT_PUEBLO, OnHelpAboutPueblo)
	ON_COMMAND(ID_HELP_ABOUT_CHACO, OnHelpAboutChaco)
	ON_COMMAND(ID_HELP_MORE_PUEBLO, OnHelpMorePueblo)
	//ON_COMMAND(ID_HELP_ABOUT_VRSCOUT, OnHelpAboutVrscout)
	ON_COMMAND(ID_HELP_ABOUT_UE, OnHelpAboutUE)
	ON_COMMAND(ID_EDIT_PREFERENCES, OnEditPreferences)
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_FILE_CLOSE, OnCloseWindow)
	ON_COMMAND(ID_HELP_REGISTER_PUEBLO, OnHelpRegisterPueblo)
	ON_COMMAND(ID_HELP_PUEBLO_WORLD_LIST_ENTRY, OnHelpPuebloWorldListEntry)
	ON_COMMAND(ID_VIEW_TRACE, OnViewTrace)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnPuebloMenuUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnPuebloMenuUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnPuebloMenuUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnPuebloMenuUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUS, OnPuebloMenuUpdateCommand)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PREFERENCES, OnUpdateEditPreferences)
	//}}AFX_MSG_MAP

	ON_MESSAGE( WM_CHACO_ASYNC_DISPATCH, OnAsyncDispatch )

											// Global help commands

	ON_COMMAND( ID_HELP_USING, CFrameWnd::OnHelpUsing )
	ON_COMMAND( ID_HELP, CFrameWnd::OnHelp )
	ON_COMMAND( ID_CONTEXT_HELP, CFrameWnd::OnContextHelp )
	ON_COMMAND( ID_DEFAULT_HELP, CFrameWnd::OnHelpIndex )

	ON_UPDATE_COMMAND_UI(ID_COMPRESSION_PANE, OnUpdateStatusText )
	ON_UPDATE_COMMAND_UI(ID_SESSION_PANE, OnUpdateStatusText )
	ON_UPDATE_COMMAND_UI(ID_TIME_PANE, OnUpdateStatusText )
	ON_UPDATE_COMMAND_UI(ID_PROGRESS_PANE, OnUpdateStatusText )


	#if defined( WIN32 )

	ON_COMMAND_RANGE(CH_MENU_FIRST_ID, CH_MENU_LAST_ID, OnPuebloMenuCommand)
	ON_UPDATE_COMMAND_UI_RANGE(CH_MENU_FIRST_ID, CH_MENU_LAST_ID, OnPuebloMenuUpdateCommand)

	#endif	// defined( WIN32 )

END_MESSAGE_MAP()

											/* arrays of IDs used to
												initialize control bars --
												first initialize the
												toolbar buttons (IDs are
												command buttons) */
static UINT BASED_CODE buttons[] =
{											/* Same order as in the bitmap
												'toolbar.bmp' */
	ID_VIEW_PREVIOUS,
		ID_SEPARATOR,
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
		ID_SEPARATOR,
	ID_VIEW_HORIZONTAL,
	ID_VIEW_SWAP_PANES,
		ID_SEPARATOR,
	ID_VIEW_STOP_LOADING,
		ID_SEPARATOR,
	ID_HELP_ABOUT_PUEBLO,
	ID_CONTEXT_HELP,
};

static UINT BASED_CODE indicators[] =
{											// Status line indicators
	ID_SEPARATOR,
	ID_COMPRESSION_PANE,			// Compression type
	ID_SESSION_PANE,						// Time online
	ID_TIME_PANE,							// Time of day
	ID_PROGRESS_PANE						// Progress bar
};



/*----------------------------------------------------------------------------
	ChMainFrame class implementation
----------------------------------------------------------------------------*/

ChString		ChMainFrame::m_strClass;

#endif // CH_MSW

#ifdef CH_UNIX
void SetTimer( chuint32 data, chuint32 interval, XtTimerCallbackProc proc )
{
	extern XtAppContext app;	// Sorry.
	XtAppAddTimeOut( app, interval, proc, (void*)data );
}
#endif

ChMainFrame::ChMainFrame() :
				m_boolInProgress( false ),
				#if defined( CH_MSW )
				m_pAppFont( 0 ),
				#endif	// defined( CH_MSW )
				m_boolInShutdown( false )
{
	m_pCore = new ChClientCore( this );
	ASSERT( GetPuebloCore() );
											/* Initialize the minute and
												second timer */

	m_secondStartTime = ChTime::GetCurrentTime();

	#if defined( CH_MSW )
	{
		LOGFONT		lf;
		HDC			hDC = ::GetDC( ::GetDesktopWindow() );
		int 		iFontSize;

		if (0 == m_strClass.GetLength())
		{
			HICON		hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

			m_strClass = AfxRegisterWndClass( 0, LoadCursor( 0, IDC_ARROW ),
												(HBRUSH)(COLOR_APPWORKSPACE + 1),		// UE
												hIcon );
		}
											// Create font for the application

		iFontSize = -1 * (::GetDeviceCaps( hDC, LOGPIXELSY ) * 9 / 72);
		::ReleaseDC( ::GetDesktopWindow(), hDC );

											// Clear the font structure
		ChMemClearStruct( &lf );

		m_pAppFont = new CFont();
		ASSERT( m_pAppFont );

		lf.lfHeight = iFontSize;
		lf.lfWeight = FW_LIGHT;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy(lf.lfFaceName, "MS Sans Serif");

		m_pAppFont->CreateFontIndirect( &lf );
	}
	// Create the brush for status messages
	if ( !m_brClearProgress.GetSafeHandle() )
	{
		int iIndex = ChUtil::GetSystemType() ==  CH_SYS_WIN95 ? COLOR_3DFACE : COLOR_BTNFACE;
		m_brClearProgress.CreateSolidBrush( GetSysColor( iIndex ) );
		m_brShowProgress.CreateSolidBrush( RGB( 0xff, 0, 0 ) );
	}

	#endif	// defined( CH_MSW )

	#if defined( CH_UNIX )
	{
		OnCreate( this );
	}
	#endif	// defined( CH_UNIX )
}


ChMainFrame::~ChMainFrame()
{
	#if defined( CH_MSW )
	{
		if ( m_pAppFont )
		{
			delete m_pAppFont;
		}
	}
	#endif	// defined( CH_MSW )

	delete m_pCore;
	m_pCore = 0;
}


#if defined( CH_MSW )


/*----------------------------------------------------------------------------

	FUNCTION	||	ChMainFrame::PreCreateWindow

	cs			||	Creation structure.  Fields in this structure may be
					changed to modify how the frame will be created.

------------------------------------------------------------------------------

	This function is called before the frame window is created.

----------------------------------------------------------------------------*/

BOOL ChMainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
	cs.style = WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU |
				WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;
	cs.lpszClass = m_strClass;

	return CFrameWnd::PreCreateWindow( cs );
}


/*----------------------------------------------------------------------------
	ChMainFrame diagnostics
----------------------------------------------------------------------------*/

#if (defined( _DEBUG ))

void ChMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}


void ChMainFrame::Dump( CDumpContext& dc ) const
{
	CFrameWnd::Dump( dc );
}

#endif	// defined( _DEBUG )

#endif // CH_MSW


#if defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChMainFrame message handlers
----------------------------------------------------------------------------*/

BOOL ChMainFrame::OnCreateClient( LPCREATESTRUCT pCreate,
									CCreateContext* pContext )
{
											/* Create a splitter with 1 row
												and 2 columns */
	if (!m_splitter.Create( this ))
	{
		TRACE0( "Failed to create the splitter\n" );
		return false;
	}
											// Add this frame to our list

	((ChApp*)AfxGetApp())->AddToFrameList( this );

											/* Changed this to call core to
												initialize menu - jwd */
	m_bAutoMenuEnable = false;
											// End menu change - jwd  
	return true;
}


void ChMainFrame::ActivateFrame( int nCmdShow )
{
	ChPersistentFrame::ActivateFrame( nCmdShow );
}



void ChMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{

	if ( !m_boolInShutdown )
	{
		ChPersistentFrame::OnPaletteChanged( pFocusWnd );

												/* This will notify all text wnds
													and graphic panes if any */
		SendMessageToDescendants( WM_PALETTECHANGED,
									(WPARAM)(pFocusWnd->GetSafeHwnd() ) );
	}

}

BOOL ChMainFrame::OnQueryNewPalette()
{
	BOOL boolChanged = false;
	HWND hWnd = GetSafeHwnd();
	// walk through HWNDs to avoid creating temporary CWnd objects
	// unless we need to call this function recursively
	for (HWND hWndChild = ::GetTopWindow(hWnd);
		!boolChanged && hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		boolChanged = ((BOOL)::SendMessage(hWndChild, WM_QUERYNEWPALETTE,
											0, 0 ) != FALSE);

		if ( !boolChanged && ::GetTopWindow(hWndChild) != NULL)
		{
			HWND hWnd = hWndChild;
			for (HWND hWndChild = ::GetTopWindow(hWnd);
				!boolChanged && hWndChild != NULL;
				hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
			{
											// Send to child windows after parent
				boolChanged =
					((BOOL)::SendMessage( hWndChild, WM_QUERYNEWPALETTE, 0, 0 ) != FALSE);
			}
		}
	}

	if (!boolChanged)
	{										/* 3DR did not set the palette so
												realize our APP palette */
		CPalette * pPal = ChImgUtil::GetStdPalette();

		if ( pPal )
		{

	        CDC*	pDC = GetDC();
			CPalette * pOldPal = pDC->SelectPalette( pPal, false );

	        boolChanged	 = (BOOL)pDC->RealizePalette();

			pDC->SelectPalette( pOldPal, true );

	        pDC->RealizePalette();

	        ReleaseDC( pDC );
		}
	}
	return boolChanged;
}


#endif	// defined( CH_MSW )


#if 0
/*----------------------------------------------------------------------------

	FUNCTION	||	FormatTimeSpan

------------------------------------------------------------------------------

	This function is called to format a time span into a string.

----------------------------------------------------------------------------*/

static void FormatTimeSpan( const ChTimeSpan& timeSpan, ChString& strTimeSpan )
{
	chint32	iTime;
	ChString	strTime;

	if (iTime = timeSpan.GetDays())
	{
		strTime.Format( " %dd %dh ", iTime, timeSpan.GetHours() );
	}
	else if (iTime = timeSpan.GetHours())
	{
		strTime.Format( " %d:%02dh ", iTime, timeSpan.GetMinutes() );
	}
	else
	{
		strTime.Format( " %d min ", timeSpan.GetMinutes() );
	}

	LOADSTRING( IDS_ONLINE, strTimeSpan );
	strTimeSpan = " " + strTimeSpan + strTime;
}
#endif // 0

#ifdef CH_MSW


/*----------------------------------------------------------------------------

	FUNCTION	||	ChMainFrame::OnPuebloMenuCommand

------------------------------------------------------------------------------

	This function is called when an item added by a module is selected.

----------------------------------------------------------------------------*/

void ChMainFrame::OnPuebloMenuCommand( UINT uId )
{
	GetPuebloCore()->GetMenuMgr()->Notify( uId );
}


void ChMainFrame::OnEditCut()
{
	GetPuebloCore()->GetMenuMgr()->Notify( ID_EDIT_CUT );
}

void ChMainFrame::OnEditCopy()
{
	GetPuebloCore()->GetMenuMgr()->Notify( ID_EDIT_COPY );
}

void ChMainFrame::OnEditPaste()
{
	GetPuebloCore()->GetMenuMgr()->Notify( ID_EDIT_PASTE );
}

void ChMainFrame::OnEditClear()
{
	GetPuebloCore()->GetMenuMgr()->Notify( ID_EDIT_CLEAR );
}

void ChMainFrame::OnViewPrevious()
{
	GetPuebloCore()->GetMenuMgr()->Notify( ID_VIEW_PREVIOUS );
}


void ChMainFrame::OnViewSwapPanes()
{
	m_splitter.SwapPanes();
}


void ChMainFrame::OnUpdateViewSwapPanes( CCmdUI* pCmdUI )
{
	bool	boolEnable;

	boolEnable = m_splitter.GetPaneCount() > 1;
	pCmdUI->Enable( boolEnable );
}


void ChMainFrame::OnViewToggleOrientation()
{
	m_splitter.TogglePaneOrientation();
	UpdateOrientationUI();
}

void ChMainFrame::OnUpdateViewToggleOrientation(CCmdUI* pCmdUI)
{
	bool	boolEnable;

	boolEnable = m_splitter.GetPaneCount() > 1;
	pCmdUI->Enable( boolEnable );
}


void ChMainFrame::OnUpdateViewStopLoading(CCmdUI* pCmdUI)
{
	if (GetPuebloCore()->GetClientMode() == ChClientCore::modeDisabled)
	{
		pCmdUI->Enable( false );
	}
	else
	{
		if (m_boolInProgress)
		{
			pCmdUI->Enable( true );
		}
		else
		{									/* Check if we are trying to
												connect to a world */
			bool	boolEnabled = false;

			GetPuebloCore()->GetMenuMgr()->Notify( pCmdUI->m_nID,
													CH_MSG_MENU_SHOW,
													&boolEnabled );
			pCmdUI->Enable( boolEnabled );
		}
	}
}


void ChMainFrame::OnViewStopLoading()
{
	if (GetPuebloCore()->GetHTTPConn()->IsActive())
	{
											// cleanup only current connections

		ChHTTPSocketConn::CloseAllConnections();
	}
	else
	{
		GetPuebloCore()->GetMenuMgr()->Notify( ID_VIEW_STOP_LOADING );
	}
}


void ChMainFrame::OnViewTrace() 
{
	GetPuebloCore()->OpenTraceWindow();
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChMainFrame::OnPuebloMenuUpdateCommand

------------------------------------------------------------------------------

	This function is called when an item should be updated.

----------------------------------------------------------------------------*/

void ChMainFrame::OnPuebloMenuUpdateCommand( CCmdUI* pCmdUI )
{
	if (!pCmdUI->m_pSubMenu && 
			GetPuebloCore()->GetClientMode() != ChClientCore::modeDisabled)
	{
											// We don't handle submenu items...
		bool	boolEnabled;
											/* Note that there may be problems
												in the menu manager code if
												there are toolbar items which
												don't occur in the menu */

		GetPuebloCore()->GetMenuMgr()->Notify( pCmdUI->m_nID, CH_MSG_MENU_SHOW,
												&boolEnabled );
		pCmdUI->Enable( boolEnabled );
	}
}


#endif // CH_MSW

/*----------------------------------------------------------------------------
	Replace the handling of menu strings with our own menu mgr
----------------------------------------------------------------------------*/

void ChMainFrame::GetMessageString( UINT nID, ChString& rMessage ) const
{
	if (!GetPuebloCore()->GetMenuMgr()->GetMessageString( nID, rMessage ))
	{
		#if defined( WIN32 )
		{
			CFrameWnd::GetMessageString( nID, rMessage );
		}
		#else
		{
			TRACE0( "CFrameWnd::GetMessageString not available under Win16\n" );
			rMessage = TEXT("" );
		}
		#endif
	}
}

void ChMainFrame::OnUpdateStatusText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
}



/*----------------------------------------------------------------------------

	FUNCTION		||	ChMainFrame::InitStatusBar

------------------------------------------------------------------------------

	This function will perform initialization on the status bar.

----------------------------------------------------------------------------*/

void ChMainFrame::InitStatusBar()
{
#ifdef CH_MSW
	CDC		*pDC;
	CFont	*pOldFont;

	// The status bar font is too big, replace the font
	m_wndStatusBar.SetFont( GetAppFont() );
	pDC = m_wndStatusBar.GetDC();
	pOldFont = pDC->SelectObject(  GetAppFont() );
	pDC->SelectObject( pOldFont );

	m_wndStatusBar.SetPaneText( CH_STATUS_COMPRESSION_PANE, "" );
	m_wndStatusBar.SetPaneText( CH_STATUS_TIME_ONLINE_PANE, "" );
	m_wndStatusBar.SetPaneText( CH_STATUS_TIME_PANE, "" );
	m_wndStatusBar.SetPaneText( CH_STATUS_PROGRESS_PANE, "" );

#else
	cerr << "XXX ChMainFrame::InitStatusBar " << __FILE__ << ":" << __LINE__ << endl;
#endif
}

#if defined( CH_UNIX )
void TenthSecondTimerProc( XtPointer nIDEvent, XtIntervalId* id )
{
	ChMainFrame*	pFrame = (ChMainFrame*)ChCore::GetCore()->GetFrameWnd();

	pFrame->TenthSecondTick( (UINT)nIDEvent );
}
#endif


/*----------------------------------------------------------------------------

	FUNCTION		||	ChMainFrame::DrawProgressBar

------------------------------------------------------------------------------

	This function is called to update the progress bar.  It should be called
	continuously.

----------------------------------------------------------------------------*/

void ChMainFrame::DrawProgressBar()
{
	#ifdef CH_MSW
		ChRect		rtProgress;
		chint16		sProgressPixels;
		CDC*		pBarDC;
	#endif


	if (GetPuebloCore()->GetHTTPConn()->IsActive())
	{
		int			iPercent;
		ChString 		strMessage;

		GetPuebloCore()->GetHTTPConn()->GetProgressMsg( strMessage, iPercent );

		m_boolInProgress = true;

		SetMessageText( strMessage );


		#if defined( CH_MSW )
		m_wndStatusBar.GetItemRect( CH_STATUS_PROGRESS_PANE, &rtProgress );


 		rtProgress.InflateRect( -1, -1 );		// Leave room for the 3d borders
 
 		sProgressPixels = (chint16)((rtProgress.Width() * iPercent) / 100);

		pBarDC = m_wndStatusBar.GetDC();

		if (sProgressPixels < (rtProgress.right - rtProgress.left))
		{
			CRect	rtEmpty;

			rtEmpty = rtProgress;
			rtEmpty.left = rtProgress.left + sProgressPixels;

			pBarDC->FillRect( &rtEmpty, &m_brClearProgress );
		}

		if (sProgressPixels)
		{
			rtProgress.right = rtProgress.left + sProgressPixels;

			pBarDC->FillRect( &rtProgress, &m_brShowProgress );
		}

		m_wndStatusBar.ReleaseDC( pBarDC );
		#endif


	} 
	else
	{
		if ( m_boolInProgress )
		{
			ClearStatus();
		}
	}
}

void ChMainFrame::ClearStatus( )
{
	if ( m_boolInProgress )
	{
		//Update once just to get rid of any old progress state
		m_boolInProgress = false;
	}
	SetMessageText( TEXT( "" ) );

	CRect		rtProgress;
	m_wndStatusBar.GetItemRect( CH_STATUS_PROGRESS_PANE, &rtProgress );

	rtProgress.InflateRect( -1, -1 );		// Leave room for the 3d borders


	#ifdef CH_MSW
	CDC*		pBarDC;
	pBarDC = m_wndStatusBar.GetDC();
	pBarDC->FillRect( &rtProgress, &m_brClearProgress );
	m_wndStatusBar.ReleaseDC( pBarDC );
	#endif

}



/*----------------------------------------------------------------------------

	FUNCTION		||	ChMainFrame::SetOrientationButton

------------------------------------------------------------------------------

	This function will set the proper bitmap and text for the horizontal/
	vertical buttons in the toolbar and the menu.

----------------------------------------------------------------------------*/

void ChMainFrame::UpdateOrientationUI()
{
	#if defined( CH_MSW )
	{										// Adjust the toolbar
		if (m_splitter.IsVertical())
		{
			m_wndToolBar.SetButtonInfo( CH_TB_IDX_ORIENTATION,
										ID_VIEW_TOGGLE_ORIENTATION,
										TBBS_BUTTON, CH_TB_IMAGE_HORIZONTAL );
		}
		else
		{
			m_wndToolBar.SetButtonInfo( CH_TB_IDX_ORIENTATION,
										ID_VIEW_TOGGLE_ORIENTATION,
										TBBS_BUTTON, CH_TB_IMAGE_VERTICAL );
		}
	}
	#endif	// defined( CH_MSW )
}


/*----------------------------------------------------------------------------

	FUNCTION		||	ChMainFrame::OnCreate

	lpCreateStruct	||	Pointer to the structure containing information
						about the object being created.

------------------------------------------------------------------------------

	This function is called during processing of the WM_CREATE message,
	after the window is created but before it is shown.

----------------------------------------------------------------------------*/

int ChMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	#if defined( CH_MSW )
	{
		if (CFrameWnd::OnCreate( lpCreateStruct ) == -1)
		{
			return -1;
		}

		if (!m_wndToolBar.Create( this ) ||
			!m_wndToolBar.LoadBitmap( IDR_MAINFRAME ) ||
			!m_wndToolBar.SetButtons( buttons, sizeof( buttons )/sizeof( UINT ) ))
		{
			TRACE0( "Failed to create toolbar\n" );

			return( -1 );					// Failed to create the toolbar
		}

		UpdateOrientationUI();
	}
	#endif	// defined( CH_MSW )

	#if defined( CH_MSW )
	{
		if (!m_wndStatusBar.Create( this ) ||
			!m_wndStatusBar.SetIndicators( indicators,
											sizeof( indicators ) /
												sizeof( UINT ) ))
		{
			TRACE0( "Failed to create status bar\n" );

			return( -1 );					// Failed to create the status
		}
		else
		{
			InitStatusBar();
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "XXX Not creating status bar: " << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )

	//#if defined( WIN32 )
	//{
	//	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
	//	EnableDocking( CBRS_ALIGN_ANY );
	//	DockControlBar( &m_wndToolBar );
	//}
	//#endif	// defined( WIN32 )

	#ifdef CH_MSW
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() |
								CBRS_TOOLTIPS | CBRS_FLYBY );
	#endif

	#ifndef CH_MSW
	SetTimer( ID_TENTH_SECOND_TIMER, TENTH_SECOND_TIMER_DURATION,
				TenthSecondTimerProc );
	#endif

	return( 0 );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChMainFrame::OnSize

	nType		||	Specifies the type of resizing requested:

						SIZE_MAXIMIZED  The window has been zoomed.
						SIZE_MINIMIZED  The window has been iconized.
						SIZE_RESTORED   The window has been resized, but
										neither SIZE_MINIMIZED nor
										SIZE_MAXIMIZED applies.

	cx			||	Specifies the new width of the client area.

	cy			||	Specifies the new height of the client area.

------------------------------------------------------------------------------

	This function is called during processing of the WM_SIZE message.

----------------------------------------------------------------------------*/

#if defined( CH_MSW )

void ChMainFrame::OnSize( UINT nType, int cx, int cy )
{
	ChPersistentFrame::OnSize( nType, cx, cy );
}

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------

	FUNCTION	||	ChMainFrame::OnClose

------------------------------------------------------------------------------

	This function will prompt to discover whether or not it is safe to
	terminate the application.

	When the Pueblo client is logged in, it terminates by sending a request
	to the server.  The server then controls the termination sequence.

	If the Pueblo client is offline, termination proceeds normally.

----------------------------------------------------------------------------*/

void ChMainFrame::OnClose()
{
	m_boolInShutdown = true;
											// Remove this from our list
	ChApp*	pApp = (ChApp*)AfxGetApp();
	pApp->RemoveFromFrameList( this );

	#if defined( CH_MSW )
	{
    	ShowWindow( SW_HIDE );
	}
	#endif	// defined( CH_MSW )

	GetPuebloCore()->StartShutdown();
											/* Replace the current frame with
												the next frame in our list */
	if (pApp->m_pMainWnd == this)
	{
		const ChMainFrame*	pFrame = pApp->GetNextFrame( this );

		if (pFrame)
		{									/* Note that we're converting
												here from a const to a
												non-const 'CWnd*' */
			pApp->m_pMainWnd = (CWnd*)pFrame;
		}
		else
		{  	// We are shutting down
#ifndef CH_NO_WEBTRACKER
			// Close all WebTracker panes if any
			ChWebTracker::CloseAllFrames( );
#endif
			// This will shutdown all threads
			ChHTTPSocketConn::CloseAllConnections( false );
			
			((ChApp *)AfxGetApp())->HandleOutdatedClient();
		}
	}

	#if defined( CH_MSW )
	{
    	ChPersistentFrame::OnClose();
	}
	#endif	// defined( CH_MSW )
}


void ChMainFrame::OnSecondTick( time_t timeCurr )
{
	ChTime		timeNow = timeCurr;
	ChString		strTime;

	DrawProgressBar();
											/* Timer called approximately
												once a second */

											// Format the time-of-day

	#if defined( CH_MSW )
	{										// Use locale time format
		char	cBuffer[128];

		if (0 < ::GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, 0, 0,
									cBuffer, sizeof( cBuffer ) ))
		{
			strTime = " ";
			strTime += cBuffer;
		}
	}
	#else	// defined( CH_MSW )
	{
		strTime = timeNow.Format( " %#I:%M %p" );
	}
	#endif	// defined( CH_MSW )

	#if defined( CH_MSW )
	{
		m_wndStatusBar.SetPaneText( CH_STATUS_TIME_PANE, strTime );
	}
	#endif	// defined( CH_MSW )

	if (GetPuebloCore())
	{
		/* Flash the icon if some module
				has requested to do so*/
		#if defined( CH_MSW )
		if (GetPuebloCore()->IsFlashWindow())
		{
											// This only flashes the icon once:
			FlashWindow( TRUE );
		}
		else if (GetPuebloCore()->WasFlashWindow())
		{
			FlashWindow( FALSE );
		}
		#endif	// defined( CH_MSW )

		/* Notify the client core of a
			 second's passing */
		GetPuebloCore()->OnSecondTick( timeCurr );
	}
}


/*----------------------------------------------------------------------------
	Menu handlers
----------------------------------------------------------------------------*/

void ChMainFrame::OnUpdateEditPreferences(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetPuebloCore()->GetClientMode() 
			!= ChClientCore::modeDisabled );
}

void ChMainFrame::OnEditPreferences()
{
	ChPrefs		prefsDlg( GetPuebloCore(), IDS_TITLE_PREFS_DLG );

											/* Add the property pages for this
												property sheet */
	prefsDlg.AddModulePages();
											/* Execute the property sheet
												modally */
	prefsDlg.DoModal();
}


void ChMainFrame::OnHelpIndex()
{
	int		iSysType = ChUtil::GetSystemType();

	if (iSysType == CH_SYS_WIN32S)
	{
		AfxGetApp()->WinHelp( 0L, HELP_INDEX );
	}
	else
	{
		AfxGetApp()->WinHelp( 0L, HELP_FINDER );
	}
}


void ChMainFrame::OnHelpAboutPueblo()
{
	ChAbout		aboutDlg( GetPuebloCore(), IDS_ABOUT_TITLE );

											/* Add the property pages for this
												property sheet */
	aboutDlg.AddModulePages();

	aboutDlg.DoModal();
}

void ChMainFrame::OnMenuHelpPuebloBugReport()
{
	ChString strFeedback;
	LOADSTRING( IDS_FEEDBACK_URL, strFeedback );

	GetPuebloCore()->DisplayWebPage( strFeedback, ChCore::browserUserPref );
}


void ChMainFrame::OnHelpPuebloWorldListEntry() 
{
	ChString		strFormURL;

	LOADSTRING( IDS_URL_WORLD_LIST_ENTRY, strFormURL );

	GetPuebloCore()->DisplayWebPage( strFormURL, ChCore::browserUserPref );
}


void ChMainFrame::OnHelpAboutChaco()
{
	ChString strAboutChaco;
	LOADSTRING( IDS_ABOUT_CHACO_URL, strAboutChaco );

	GetPuebloCore()->DisplayWebPage( strAboutChaco, ChCore::browserUserPref );
}


void ChMainFrame::OnHelpMorePueblo()
{
	ChString strAboutPueblo;
	LOADSTRING( IDS_ABOUT_PUEBLO_URL, strAboutPueblo );

	GetPuebloCore()->DisplayWebPage( strAboutPueblo, ChCore::browserUserPref );
}


/*
void ChMainFrame::OnHelpAboutVrscout()
{
	ChString strAboutVRScoutPage;
	LOADSTRING( IDS_ABOUT_VRSCOUT_URL, strAboutVRScoutPage );

	GetPuebloCore()->DisplayWebPage( strAboutVRScoutPage, ChCore::browserUserPref );
}
*/


void ChMainFrame::OnHelpAboutUE()
{
	ChString strAboutUEPage;
	LOADSTRING( IDS_ABOUT_UE_URL, strAboutUEPage );

	GetPuebloCore()->DisplayWebPage( strAboutUEPage, ChCore::browserUserPref );
}


void ChMainFrame::OnHelpRegisterPueblo()
{
	if (GetPuebloCore()->DoRegistration() &&
			(GetPuebloCore()->GetClientMode() == ChClientCore::modeDisabled))
	{
		GetPuebloCore()->StartPueblo( ChString( "" ), ChClientCore::doLoginNotify );
	}
}

// Added to send activate to VRML window so it can get processed by RL
// Pritham: rip this out when we go to a plugin

void ChMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	// Check if I am the main wnd on activate, if not set myself 
	// as active wnd

	if ( nState != WA_INACTIVE )
	{
		ChApp* pApp = (ChApp*)AfxGetApp();
		if ( pApp->m_pMainWnd != this )
		{
			pApp->m_pMainWnd = this;
		}

		// Turn off flashing
		FlashWindow( false );
		GetPuebloCore()->EnableFlashWindow( false );
	}	

	ChPersistentFrame::OnActivate( nState, pWndOther, bMinimized );
	// This message is processed only by the maze wnd.
	SendMessageToDescendants( WM_VRML_ACTIVATE,
								MAKEWORD( nState, bMinimized),
								LPARAM( pWndOther->GetSafeHwnd() ) );
}

LONG ChMainFrame::OnAsyncDispatch( UINT wParam, LONG lParam )
{
	ChMsg* pMsg = (ChMsg*)lParam;

	GetPuebloCore()->DispatchMsg( (ChModuleID)wParam, *pMsg );

	delete pMsg;
	
	return 0;
}


void ChMainFrame::OnCloseWindow() 
{
	OnClose();
}

// Local Variables: ***
// tab-width:4 ***
// End: ***
