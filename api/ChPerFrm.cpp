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

	This file consists of interfaces of the ChPersistentFrame class, which
	remembers the frame's location on the desktop.  This class is adopted
	from the book 'Inside Visual C++' by David J. Kruglinski.

----------------------------------------------------------------------------*/

#include "headers.h"

#if defined( CH_UNIX ) && defined( CH_CLIENT )
#define String XtString
#include <X11/Intrinsic.h>
#include <Xm/XmStrDefs.h>
#include <Xm/Xm.h>
#undef String
#endif

#include <ChPerFrm.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>

//#define DebugBox(msg)	::MessageBox(0, msg, "Debug", MB_OK|MB_ICONINFORMATION)
#define DebugBox(msg)

/*----------------------------------------------------------------------------
	ChPersistentFrame class
----------------------------------------------------------------------------*/


#ifdef CH_MSW

#if !defined( CH_PUEBLO_PLUGIN )		

const char		ChPersistentFrame::cRegistrySeparator = '\\';
const ChString	ChPersistentFrame::strProfileRect( "window rect" );
const ChString	ChPersistentFrame::strProfileIcon( "iconic" );
const ChString	ChPersistentFrame::strProfileMax( "maximized" );
const ChString	ChPersistentFrame::strProfileTool( "toolbar" );
const ChString	ChPersistentFrame::strProfileStatus( "status bar" );
#endif


#if !defined( CH_PUEBLO_PLUGIN )		
IMPLEMENT_DYNAMIC( ChPersistentFrame, CFrameWnd )

BEGIN_MESSAGE_MAP( ChPersistentFrame, CFrameWnd )
#else
IMPLEMENT_DYNAMIC( ChPersistentFrame, CWnd )

BEGIN_MESSAGE_MAP( ChPersistentFrame, CWnd )
#endif
    //{{AFX_MSG_MAP( ChPersistentFrame )
#if !defined( CH_PUEBLO_PLUGIN )		
    ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_SYSCOMMAND()
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif // CH_MSW

ChPersistentFrame::ChPersistentFrame() :
					#if defined( CH_MSW )
					m_pActiveWnd( 0 ),
					m_hwndFocus( 0 ),
					#endif	// defined( CH_MSW )
					boolFirstTime( true ),
					boolIconic( false )
{
	SetLabel( "" );
}


void ChPersistentFrame::SetLabel( const ChString& strLabel )
{
	m_strLabel = strLabel;

	#if defined( CH_MSW ) && !defined( CH_PUEBLO_PLUGIN )
	{
		ChString		strTempLabel = GetLabel();

		m_strRegSection = "Frames";

		if (strTempLabel.IsEmpty())
		{
			strTempLabel = "_generic_";
		}

		m_strRegSection += cRegistrySeparator + strTempLabel +
							cRegistrySeparator;
	}
	#endif	// defined( CH_MSW )
}



#if defined( CH_MSW )	&& !defined( CH_PUEBLO_PLUGIN )

/*----------------------------------------------------------------------------
	ChPersistentFrame message handlers
----------------------------------------------------------------------------*/

void ChPersistentFrame::OnDestroy()
{
	CString			text;
	CString			temp;
	CString			debugText;
	CWnd			*pBar;
	bool			boolIconic;
	bool			boolMaximized;
	WINDOWPLACEMENT	wndpl;
	bool			boolSuccess;

	wndpl.length = sizeof( WINDOWPLACEMENT );
											/* Gets current window position
												and iconized/maximized status */
	boolSuccess = (GetWindowPlacement( &wndpl ) != FALSE);

	if (wndpl.showCmd == SW_SHOWNORMAL)
	{
		boolIconic = false;
		boolMaximized = false;
	}
	else if (wndpl.showCmd == SW_SHOWMAXIMIZED)
	{
		boolIconic = false;
		boolMaximized = true;
	}
	else if (wndpl.showCmd == SW_SHOWMINIMIZED)
	{
		boolIconic = true;
		if (wndpl.flags)
		{
	    	boolMaximized = true;
		}
	  	else
	  	{
	    	boolMaximized = false;
	  	}
	}

	wsprintf( text.GetBuffer( 64 ), "%04d %04d %04d %04d",
				wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top,
				wndpl.rcNormalPosition.right, wndpl.rcNormalPosition.bottom );

	text.ReleaseBuffer();

	debugText = "Writing frame placement data...\r\n  Registry section: "
						+ m_strRegSection + "\r\n  Position: " + text;

	AfxGetApp()->WriteProfileString( GetRegSection(), strProfileRect, text );
	AfxGetApp()->WriteProfileInt( GetRegSection(), strProfileIcon,
									boolIconic );
	AfxGetApp()->WriteProfileInt( GetRegSection(), strProfileMax,
									boolMaximized );

	debugText += "\r\n  Minimised: " + CString(boolIconic ? "true" : "false");
	debugText += "\r\n  Maximised: " + CString(boolMaximized ? "true" : "false");

	if (pBar = GetDescendantWindow( AFX_IDW_TOOLBAR ))
	{
		bool visible = (pBar->GetStyle() & WS_VISIBLE) != 0L;
		AfxGetApp()->WriteProfileInt( GetRegSection(), strProfileTool,
										visible );
		debugText += "\r\n  Toolbar visible: " + CString(visible ? "true" : "false");
	}

	if (pBar = GetDescendantWindow( AFX_IDW_STATUS_BAR ))
	{
		bool visible = (pBar->GetStyle() & WS_VISIBLE) != 0L;
		AfxGetApp()->WriteProfileInt( GetRegSection(), strProfileStatus,
										visible );
		debugText += "\r\n  Statusbar visible: " + CString(visible ? "true" : "false");
	}
	
	DebugBox(debugText);
											// Call the parent class
	CFrameWnd::OnDestroy();
}


void ChPersistentFrame::ActivateFrame( int nCmdShow )
{
	CWnd			*pBar;
	CString			text;
	bool			boolIconic;
	bool			boolMaximized;
	bool			boolTool;
	bool			boolStatus;
	UINT			flags;
	WINDOWPLACEMENT	wndpl;
	CRect			rtFrame;
	bool			boolSuccess;

	if (boolFirstTime)
	{
		boolFirstTime = false;
		text = AfxGetApp()->GetProfileString( GetRegSection(),
												strProfileRect );
		if (!text.IsEmpty())
		{									// can't use sscanf in a DLL

			DebugBox("Restoring frame position from the registry.");
			rtFrame.left = atoi( (const char*)text );
			rtFrame.top = atoi( (const char*)text + 5 );
			rtFrame.right = atoi( (const char*)text + 10 );
			rtFrame.bottom = atoi( (const char*)text + 15 );
		}
		else
		{									// Base the rect on the screen size
			DebugBox("Unable to restore frame position from the registry.");
			rtFrame.top = 0;
			rtFrame.bottom = GetSystemMetrics( SM_CYFULLSCREEN );
			rtFrame.left = rtFrame.right = GetSystemMetrics( SM_CXFULLSCREEN );
			rtFrame.left /= 2;

			if (rtFrame.Width() < 400)
			{
				rtFrame.left = rtFrame.right - 400;
			}
		}

		boolIconic = (AfxGetApp()->GetProfileInt( GetRegSection(),
													strProfileIcon, FALSE ) != FALSE);
		boolMaximized = (AfxGetApp()->GetProfileInt( GetRegSection(),
													strProfileMax, FALSE ) != FALSE);
		if (boolIconic)
		{
			nCmdShow = SW_SHOWMINNOACTIVE;

			if (boolMaximized)
			{
				flags = WPF_RESTORETOMAXIMIZED;
			}
		}
		else
		{
			if (boolMaximized)
			{
				nCmdShow = SW_SHOWMAXIMIZED;
				flags = WPF_RESTORETOMAXIMIZED;
			}
			else
			{
				nCmdShow = SW_NORMAL;
				flags = 0;
			}
		}

		wndpl.length = sizeof( WINDOWPLACEMENT );
		wndpl.showCmd = nCmdShow;
		wndpl.flags = flags;
		wndpl.ptMinPosition = CPoint( 0, 0 );
		wndpl.ptMaxPosition = CPoint( -::GetSystemMetrics( SM_CXBORDER ),
										-::GetSystemMetrics( SM_CYBORDER ) );
		wndpl.rcNormalPosition = rtFrame;

		boolTool = (AfxGetApp()->GetProfileInt( GetRegSection(),
												strProfileTool, TRUE ) != FALSE);
		if (pBar = GetDescendantWindow( AFX_IDW_TOOLBAR ))
		{
			pBar->ShowWindow( boolTool );
		}

		boolStatus = (AfxGetApp()->GetProfileInt( GetRegSection(),
													strProfileStatus, TRUE ) != FALSE);
		if (pBar = GetDescendantWindow( AFX_IDW_STATUS_BAR ))
		{
			pBar->ShowWindow( boolStatus );
		}
											/* sets window's position and
												iconized/maximized status */
		boolSuccess = (SetWindowPlacement( &wndpl ) != FALSE);
	}

	CFrameWnd::ActivateFrame( nCmdShow );
}


void ChPersistentFrame::OnActivate( UINT nState, CWnd* pWndOther,
									BOOL boolMinimized )
{
	CWnd*	pWnd;

	CFrameWnd::OnActivate( nState, pWndOther, boolMinimized );

	switch( nState )
	{
		case WA_INACTIVE:
		{									// Save the focus window
			RememberFocus();

			if ((pWnd = GetActiveWnd()) && ::IsWindow( pWnd->m_hWnd ))
			{
				HWND	hwndOther;

				if (pWndOther)
				{
					hwndOther = pWndOther->m_hWnd;
				}
				else
				{
					hwndOther = 0;
				}

				#if defined( CH_ARCH_16 )
				{
					pWnd->SendMessage( WM_ACTIVATE, nState,
										MAKELONG( boolMinimized, hwndOther ) );
				}
				#elif defined( CH_ARCH_32 )
				{
					pWnd->SendMessage( WM_ACTIVATE,
										MAKELONG( nState, boolMinimized ),
										(LONG)hwndOther );
				}
				#else
				{
					#error "Architecture not defined!"
				}
				#endif
			}
			break;
		}

		case WA_ACTIVE:
		case WA_CLICKACTIVE:
		{
			if (!IsIconic())
			{
				RestoreFocus();
			}
			break;
		}

		default:
		{
			break;
		}
	}
}


void ChPersistentFrame::OnSysCommand( UINT nID, LONG lParam )
{
	switch( nID )
	{
		case SC_MINIMIZE:
		case SC_SCREENSAVE:
		{
			RememberFocus();
			break;
		}

		default:
		{
			break;
		}
	}

	CFrameWnd::OnSysCommand( nID, lParam );
}


#endif // CH_MSW

#ifdef CH_UNIX
void ChPersistentFrame::SetMessageText(const char* pstrText )
{
	cerr << "XXX ChPersistentFrame::SetMessageText( " << pstrText << ") " <<
				__FILE__ << ":" << __LINE__ << endl;

#ifdef CH_CLIENT
    extern Widget status_label; // XXX Ugly.
    XmString str = XmStringCreateLocalized( pstrText );

    XtVaSetValues( status_label,
		   XmNlabelString, str,
		   NULL );
    XmStringFree(str);
#endif // CH_CLIENT
}
#endif // CH_UNIX
