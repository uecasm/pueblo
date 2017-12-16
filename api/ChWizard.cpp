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

	This file consists of implementation for the ChWizardPage class, used
	to define pages for a wizard.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChWizard.h>

#include <ChCore.h>
#include <ChUtil.h>

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define WM_KICKIDLE			0x036A  // (params unused) causes idles to kick in

#define CX_BORDER			1
#define CY_BORDER			1


/*----------------------------------------------------------------------------
	Macros
----------------------------------------------------------------------------*/

#ifndef _countof
#define _countof( array )		(sizeof( array ) / sizeof( array[0] ))
#endif


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( HWND )
ChGetSafeOwner( CWnd* pParent, HWND* phTopLevel );

CH_INTERN_FUNC( void )
ChDeleteObject( HGDIOBJ* pObject );

CH_INTERN_FUNC( void )
SetCtrlFocus( HWND hWnd );

CH_INTERN_FUNC( void )
EnableDlgItem( HWND hWnd, UINT uiID, bool boolEnable, int iIDFocus = -1 );

CH_INTERN_FUNC( void )
LoadBtnText( int iID, ChString& strText );

CH_INTERN_FUNC( bool )
IsCharAfterAmpersand( LPTSTR lpsz, TCHAR chFind );


/*----------------------------------------------------------------------------
	ChWizardPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChWizardPage, CDialog )

ChWizardPage::ChWizardPage( chint32 lIDTemplate, UINT uiIDCaption ) :
				m_lTemplate( lIDTemplate )
{
	ASSERT( lIDTemplate != 0 );

	CommonConstruct( MAKEINTRESOURCE( lIDTemplate ), uiIDCaption );
}

ChWizardPage::~ChWizardPage()
{
}


chint32 ChWizardPage::GetID() const
{
	return m_lTemplate;
}


void ChWizardPage::CancelToClose()
{
	ChWizard*	pWiz = (ChWizard*)m_pParentWnd;

	ASSERT_VALID( this );

	ASSERT( pWiz != 0 );
	ASSERT( pWiz->IsKindOf( RUNTIME_CLASS( ChWizard ) ) );

	pWiz->CancelToClose();
}


BOOL ChWizardPage::OnInitPage()
{
	return true;
}


BOOL ChWizardPage::OnBack()
{
	return true;
}


BOOL ChWizardPage::OnNext()
{
	return true;
}


void ChWizardPage::OnCancel()
{
	ASSERT_VALID( this );
	Default();								/* Do not call CDialog::OnCancel
												as it will call EndDialog */
}


BOOL ChWizardPage::OnSetActive()
{
	if (0 == m_hWnd)
	{
		if (!CreatePage())
		{
			return false;
		}

		ASSERT( 0 != m_hWnd );
	}

	return true;
}


BOOL ChWizardPage::OnKillActive()
{
	ASSERT_VALID( this );
											/* Override this to perform
												validation; return false and
												this page will remain
												active... */
	if (!UpdateData( true ))
	{
		TRACE( "UpdateData failed during page deactivation\n" );
		
											/* UpdateData will set focus to
												correct item */
		return false;
	}

	return true;
}


BOOL ChWizardPage::PreTranslateMessage( MSG* pMsg )
{
	HWND		hwndFocusBefore = ::GetFocus();
	ChWizard*	pWiz = (ChWizard*)m_pParentWnd;

	ASSERT( pWiz->IsKindOf( RUNTIME_CLASS( ChWizard ) ) );

											/* Special case for VK_RETURN and
												"edit" controls with
												ES_WANTRETURN */

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		static const TCHAR	szEdit[] = _T("edit");
		TCHAR				szCompare[sizeof( szEdit ) + 1];

		::GetClassName( hwndFocusBefore, szCompare, _countof( szCompare ) );
		if ((lstrcmpi( szCompare, szEdit ) == 0) &&
			(::GetWindowLong( hwndFocusBefore, GWL_STYLE ) & ES_WANTRETURN))
		{
			::SendMessage( hwndFocusBefore, WM_CHAR, '\n', 0 );
			return true;
		}
	}
											/* Otherwise check for special
												accelerators */
	BOOL	boolResult;

	if ((pMsg->message == WM_KEYDOWN) && PreTranslateKeyDown( pMsg ))
	{
		boolResult = TRUE;
	}
	else
	{
		boolResult = pWiz->PreTranslateMessage( pMsg );
	}
											/* If focus changed, make sure
												buttons are set correctly */
	HWND	hwndFocusAfter = ::GetFocus();

	if (hwndFocusBefore != hwndFocusAfter)
	{
		pWiz->CheckDefaultButton( hwndFocusBefore, hwndFocusAfter );
	}

	return boolResult;
}


BEGIN_MESSAGE_MAP( ChWizardPage, CDialog )
	//{{AFX_MSG_MAP(ChWizardPage)
	ON_WM_CTLCOLOR()
	ON_WM_NCCREATE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWizardPage diagnostics
----------------------------------------------------------------------------*/

#if defined( CH_DEBUG )

void ChWizardPage::EndDialog( int iEndID )
{
											/* Do NOT call EndDialog for a
												page!  Coordinate with the
												parent for termination (you
												can post WM_COMMAND with IDOK
												or IDCANCEL to handle those
												cases). */
	ASSERT( false );
}

#endif	// defined( _DEBUG )


/*----------------------------------------------------------------------------
	ChWizardPage protected methods
----------------------------------------------------------------------------*/

void ChWizardPage::CommonConstruct( char* pstrTemplateName, UINT uiIDCaption )
{
	m_hbrBtnFace = 0;
	UpdateSysColors();

	#if ( _MSC_VER > 900	 )
	m_lpDialogTemplate = (const DLGTEMPLATE *)pstrTemplateName;
	#else
	m_lpDialogTemplate = (const char *)pstrTemplateName;
	#endif

	if (uiIDCaption != 0)
	{
		VERIFY( m_strCaption.LoadString( uiIDCaption ) );
	}
	else
	{
		LoadCaption();
	}
}


void ChWizardPage::UpdateSysColors()
{
	COLORREF	colorBtnFace;

	colorBtnFace = ::GetSysColor( COLOR_BTNFACE );
	m_colorBtnText = ::GetSysColor( COLOR_BTNTEXT );

	ChDeleteObject( (HGDIOBJ*)&m_hbrBtnFace );
	m_hbrBtnFace = ::CreateSolidBrush( colorBtnFace );
}


BOOL ChWizardPage::PreTranslateKeyDown( MSG* pMsg )
{
	ChWizard*	pWiz = (ChWizard*)m_pParentWnd;

	ASSERT( pWiz->IsKindOf( RUNTIME_CLASS( ChWizard ) ) );
	ASSERT( pMsg->message == WM_KEYDOWN );

	DWORD		dwDlgCode = ::SendMessage( ::GetFocus(), WM_GETDLGCODE, 0, 0 );

	if (pMsg->wParam == VK_TAB)
	{
		if (dwDlgCode & DLGC_WANTTAB)
		{
			return false;
		}
											// Handle tab key
		if (ProcessTab( pMsg ))
		{
			return true;
		}
	}
	else if ((pMsg->wParam == VK_RETURN) && (0 == pWiz->m_hwndDefault))
	{
		if (dwDlgCode & DLGC_WANTALLKEYS)
		{
			return false;
		}
											// Handle return key

		m_pParentWnd->PostMessage( WM_KEYDOWN, VK_RETURN, pMsg->lParam );
		return true;
	}
	else if (pMsg->wParam == VK_ESCAPE)
	{
		if (dwDlgCode & DLGC_WANTALLKEYS)
		{
			return false;
		}
											// Escape key handled

		m_pParentWnd->PostMessage( WM_KEYDOWN, VK_ESCAPE, pMsg->lParam );
		return true;
	}

	return false;
}


BOOL ChWizardPage::ProcessTab( MSG* pMsg )
{
											/* Handle tabbing back into the
												property sheet when tabbing
												away from either end of the
												dialog's tab order */
	if (GetKeyState( VK_CONTROL ) < 0)
	{
		return false;
	}

	bool	boolShift = GetKeyState( VK_SHIFT ) < 0;
	DWORD	dwDlgCode;

	dwDlgCode = ::SendMessage( ::GetFocus(), WM_GETDLGCODE, 0, 0 );
	if ((dwDlgCode & (DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_WANTTAB)) == 0)
	{
		HWND	hwndFocus = ::GetFocus();
		HWND	hwndCtl = hwndFocus;

		if (::IsChild( m_hWnd, hwndCtl ))
		{
			do
			{
				static const TCHAR	szComboBox[] = _T( "combobox" );

				HWND				hwndParent = ::GetParent( hwndCtl );
				TCHAR				szCompare[_countof( szComboBox ) + 1];
				int					iCmd = boolShift ? GW_HWNDPREV :
														GW_HWNDNEXT;

				ASSERT( hwndParent != 0 );
				::GetClassName( hwndParent, szCompare, _countof( szCompare ) );

				if (lstrcmpi( szCompare, szComboBox ) == 0)
				{
					hwndCtl = ::GetWindow( hwndParent, iCmd );
				}
				else
				{
					hwndCtl = ::GetWindow( hwndCtl, iCmd );
				}

				if (0 == hwndCtl)
				{
					SetCtrlFocus( ::GetNextDlgTabItem( m_pParentWnd->m_hWnd,
														m_hWnd, boolShift ) );

					return true;			// Handled one way or the other
				}
			}
			while ((::GetWindowLong( hwndCtl, GWL_STYLE) &
						(WS_DISABLED | WS_TABSTOP | WS_VISIBLE)) !=
							(WS_TABSTOP | WS_VISIBLE));
		}
	}

	return false;
}


bool ChWizardPage::CreatePage()
{
	if (!Create( (const char*)m_lpDialogTemplate, m_pParentWnd ))
	{
		return false;						// Create() failed...
	}
											/* Must be a child for obvious
												reasons, and must be disabled
												to prevent it from taking the
												focus away from the tab area
												during initialization... */

	ASSERT( (GetStyle() & (WS_DISABLED | WS_CHILD)) ==
				(WS_DISABLED | WS_CHILD) );

	return true;							// Success
}


void ChWizardPage::LoadCaption()
{
	HINSTANCE		hInst;
	HRSRC			hResource;
	HGLOBAL			hTemplate;
	DLGTEMPLATE*	pDlgTemplate;
	LPCWSTR			pDialogData;			/* Use a LPWSTR because all
												resources are UNICODE */

	hInst = AfxFindResourceHandle( (const char *)m_lpDialogTemplate,
									RT_DIALOG );
	ASSERT( hInst != 0 );

	hResource = ::FindResource( hInst, (const char *)m_lpDialogTemplate,
								RT_DIALOG );
	ASSERT( hResource != 0 );

	hTemplate = ::LoadResource( hInst, hResource );
	ASSERT( hTemplate != 0 );
											/* Resources don't have to be
												freed or unlocked in Win32 */

	pDlgTemplate = (DLGTEMPLATE*)::LockResource( hTemplate );
	ASSERT( pDlgTemplate != 0 );

	pDialogData = (LPCWSTR)((BYTE*)pDlgTemplate + sizeof( DLGTEMPLATE ));
	
											// Skip menu stuff

	pDialogData += (*pDialogData == 0xffff) ? 2 : wcslen( pDialogData ) + 1;
	
											// Skip window class stuff

	pDialogData += (*pDialogData == 0xffff) ? 2 : wcslen( pDialogData ) + 1;
	
											// We're now at the caption
	m_strCaption = pDialogData;
}	


void ChWizardPage::UpdateNameValue( ChString& strBuffer, 
						const ChString& strName, const ChString& strValue )
{
	// Update the name value pair in HTML form style
	if ( strValue.IsEmpty() )
	{
		return;
	}

	ChString strAttrName( strName );

	strAttrName.TrimLeft();
	strAttrName.TrimRight();

	// check name
	int iIndex;
	while ( (iIndex = strAttrName.FindOneOf( TEXT( " :" ) ) ) != - 1 )
	{
		if ( strAttrName[iIndex] == TEXT( ' ' ) )
		{ 	// replace space with _
			LPSTR pstrValue = strAttrName.GetBuffer( strAttrName.GetLength() + 1 );
			pstrValue[iIndex] = TEXT( '_' );
			strAttrName.ReleaseBuffer();
				
		}
		else
		{
			if ( iIndex == ( strAttrName.GetLength() - 1) )
			{ // remove if it is at the end
				strAttrName = strAttrName.Left( iIndex );
			}
			else
			{	// replace space with _
				LPSTR pstrValue = strAttrName.GetBuffer( strAttrName.GetLength() + 1 );
				pstrValue[iIndex] = TEXT( '_' );
				strAttrName.ReleaseBuffer();
			}
		}	
	}

	// Format the value
	ChUtil::HtmlAddNameValuePair( strBuffer, strAttrName, strValue );
}



/*----------------------------------------------------------------------------
	ChWizardPage message handlers
----------------------------------------------------------------------------*/

BOOL ChWizardPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	return OnInitPage();
}


BOOL ChWizardPage::OnNcCreate( LPCREATESTRUCT lpcs )
{
	ModifyStyle( WS_CAPTION | WS_BORDER, WS_GROUP | WS_TABSTOP );
	ModifyStyleEx( WS_EX_WINDOWEDGE, 0, SWP_DRAWFRAME );

	return CDialog::OnNcCreate( lpcs );
}


int ChWizardPage::OnCreate( LPCREATESTRUCT lpcs )
{
	if (CDialog::OnCreate( lpcs ) == -1)
	{
		return -1;
	}

	CRect		rect;

	GetWindowRect( &rect );
	rect.bottom -= GetSystemMetrics( SM_CYCAPTION ) - CY_BORDER;
	SetWindowPos( NULL, 0, 0, rect.Width(), rect.Height(),
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE );
	return 0;
}


HBRUSH ChWizardPage::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT uiCtlColor )
{
	LRESULT		lResult;

	if (pWnd->SendChildNotifyLastMsg( &lResult ))
	{
		return (HBRUSH)lResult;
	}

	if (!GrayCtlColor( pDC->m_hDC, pWnd->GetSafeHwnd(), uiCtlColor,
						m_hbrBtnFace, m_colorBtnText ))
	{
		return (HBRUSH)Default();
	}
	
	return m_hbrBtnFace;
}


void ChWizardPage::OnClose()
{
	GetParent()->PostMessage( WM_CLOSE );
}


/*----------------------------------------------------------------------------
	ChWizard class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChWizard, CWnd )

ChWizard::ChWizard( chparam resources, int iCaptionID, CWnd* pParent ) :
			m_hModule( (HINSTANCE)resources ),
			m_pParentWnd( pParent ),
			m_iCurPage( 0 ),
			m_hwndFocus( 0 ),
			m_boolParentDisabled( false ),
			m_hwndDefault( 0 ),
			m_hwndLastFocus( 0 ),
			m_hFont( 0 )
{
	const ChClientInfo*	pClientInfo = ChCore::GetClientInfo();
	OSType				osType = pClientInfo->GetPlatform();
	ChVersion			osVersion = pClientInfo->GetPlatformVersion();
	ChVersion			ntDrawEdgeVer( 3, 51 );

											/* Figure out if we can call the
												'DrawEdge' function */

	m_boolDrawEdgeAvailable = ((osWin95 == osType) ||
								(osWin98 == osType) || 
								(osWinXP == osType) ||
								((osWinNT == osType) &&
									(osVersion >= ntDrawEdgeVer)));

											// Load the caption string

	ChUtil::Load( resources, iCaptionID, m_strCaption );
}


ChWizard::~ChWizard()
{
}


void ChWizard::AddPage( ChWizardPage* pPage )
{
	ASSERT( pPage != NULL );

	m_pages.Add( pPage );

	ASSERT( pPage->m_pParentWnd == 0 );
	pPage->m_pParentWnd = this;
}


int ChWizard::DoModal()
{
	int			iResult = IDABORT;
	HWND		hwndTopLevel;
	CWnd*		pParentWnd;
	HINSTANCE	hInstOld = AfxGetResourceHandle();

	AfxSetResourceHandle( m_hModule );
											/* This window should not have been
												created yet! */
	ASSERT( 0 == m_hWnd );
											/* Allow OLE servers to disable
												themselves */
	CWinApp* pApp = AfxGetApp();
	pApp->EnableModeless( false );
											// Find parent HWND

	pParentWnd = CWnd::FromHandle( ChGetSafeOwner( m_pParentWnd,
													&hwndTopLevel ) );
	if (hwndTopLevel != 0)
	{
											// Disable the top-level parent
		::EnableWindow( hwndTopLevel, false );
	}
											/* Create the dialog, then enter
												modal loop */

	if (Create( pParentWnd,
				WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME ))
	{
		bool	boolShown;
		MSG		msg;
											/* Disable parent (should not
												disable this window) */
		m_boolParentDisabled = false;
		if ((pParentWnd != 0) && (pParentWnd->IsWindowEnabled()))
		{
			pParentWnd->EnableWindow( false );
			m_boolParentDisabled = true;
		}

		ASSERT( IsWindowEnabled() );		// Should not be disabled to start!
		SetActiveWindow();
											// For tracking the idle time state

		boolShown = (GetStyle() & WS_VISIBLE) != 0;
		m_iID = -1;
											/* Acquire and dispatch messages
												until a WM_QUIT message is
												received */
		while ((m_iID == -1) && (m_hWnd != 0))
		{
											/* Phase 1: Check to see if we can
														do idle work */

			if (!::PeekMessage( &msg, 0, 0, 0, PM_NOREMOVE ))
			{
											/* Send WM_ENTERIDLE since queue
												is empty */

				if ((pParentWnd != 0) &&
					!(pParentWnd->GetStyle() & DS_NOIDLEMSG))
				{
					pParentWnd->SendMessage( WM_ENTERIDLE, MSGF_DIALOGBOX,
												(LPARAM)m_hWnd );
				}

				if (!boolShown)
				{							// Show and activate the window
					boolShown = true;
					ShowWindow( SW_SHOWNORMAL );
				}
			}
											/* Phase 2: Pump messages while
														available */
			do
			{
											/* Pump message -- if WM_QUIT
												assume cancel and repost */
				if (!PumpMessage())
				{
					AfxPostQuitMessage( (int)msg.wParam );
					m_iID = IDCANCEL;
					break;
				}

			} while ((m_iID == -1) && (m_hWnd != 0) &&
						::PeekMessage( &msg, 0, 0, 0, PM_NOREMOVE ));
		}

		if (m_iID != -1)
		{
			iResult = m_iID;
		}

		if (m_hWnd != 0)
		{
			EndDialog( iResult );
		}
	}
											/* Allow OLE servers to enable
												themselves */
	pApp->EnableModeless( true );
											/* Enable top level parent window
												again */
	if (hwndTopLevel != 0)
	{
		::EnableWindow( hwndTopLevel, true );
	}

	AfxSetResourceHandle( hInstOld );

	return iResult;
}


void ChWizard::EndDialog( int iEndID )
{
	m_iID = iEndID;

	DestroyWindow();
}


BOOL ChWizard::OnPageChanging()
{											/* Returns true if it's okay to
												switch to a different page */
	bool			boolOkayToSwitch = true;
	ChWizardPage*	pPage;

	ASSERT_VALID( this );

	ASSERT( m_iCurPage < GetPageCount() );
	ASSERT( m_iCurPage >= 0 );

	pPage = GetPage( m_iCurPage );
	if (pPage->m_hWnd != 0)
	{
		if (!pPage->OnKillActive())
		{
			boolOkayToSwitch = false;
		}
		else
		{
			ChWizardPage*	pPage = GetPage( m_iCurPage );

			if (pPage->m_hWnd != 0)
			{
				pPage->ShowWindow( SW_HIDE );
			}
		}
	}

	return boolOkayToSwitch;
}


BOOL ChWizard::PreTranslateMessage( MSG* pMsg )
{
	BOOL	boolResult = FALSE;
											/* Post message to check for change
												in focus later */
	if (pMsg->message != WM_KICKIDLE)
	{
		MSG		msg;

		PeekMessage( &msg, NULL, WM_KICKIDLE, WM_KICKIDLE, PM_REMOVE );
		PostMessage( WM_KICKIDLE );
	}
											// Process special case keystrokes
	if (ProcessChars( pMsg ))
	{
		boolResult = TRUE;
	}
	else if (ProcessTab( pMsg ))
	{
		boolResult = TRUE;
	}
	else
	{										/* Handle normal accelerator
												keystrokes */
		ChWizardPage*	pPage = GetActivePage();

		if ((::IsChild( pPage->m_hWnd, pMsg->hwnd ) &&
				::IsDialogMessage( pPage->m_hWnd, pMsg )) ||
				::IsDialogMessage( m_hWnd, pMsg ))
		{
			boolResult = TRUE;
		}
	}

	if (!boolResult)
	{
		boolResult = CWnd::PreTranslateMessage( pMsg );
	}
											/* Handle WM_KICKIDLE message to
												check for focus changes */
	if (pMsg->message == WM_KICKIDLE)
	{
		CheckFocusChange();
	}

	return boolResult;
}


BOOL ChWizard::DestroyWindow()
{
	BOOL	boolResult;
											/* Re-enable parent if it was
												disabled */

	CWnd*	pParentWnd = (m_pParentWnd != 0) ? m_pParentWnd : GetParent();

	if (m_boolParentDisabled && (pParentWnd != 0))
	{
		pParentWnd->EnableWindow();
	}
											/* Transfer the focus to ourselves
												to give the active control a
												chance at WM_KILLFOCUS */

	if ((::GetActiveWindow() == m_hWnd) && (::IsChild( m_hWnd, ::GetFocus() )))
	{
		m_hwndFocus = 0;
		SetFocus();
	}
											/* Hide this window and move
												activation to the parent */

	SetWindowPos( 0, 0, 0, 0, 0,
					SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE |
						SWP_NOZORDER );

	pParentWnd = GetParent();
	if (pParentWnd != 0)
	{
		pParentWnd->SetActiveWindow();
	}
											// Finally, destroy this window
	boolResult = CWnd::DestroyWindow();
											/* Delete the font (will be created
												next time DoModal/Create is
												called) */
	ChDeleteObject( (HGDIOBJ*)&m_hFont );

	return boolResult;
}


chint32 ChWizard::OnBack()
{
	return 0;
}


chint32 ChWizard::OnNext()
{
	return 0;
}


void ChWizard::OnFinish()
{
	ASSERT_VALID( this );

	EndDialog( IDFINISH );
}


BEGIN_MESSAGE_MAP( ChWizard, CWnd )
	//{{AFX_MSG_MAP(ChWizard)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_COMMAND(IDBACK, OnBackClick)
	ON_COMMAND(IDNEXT, OnNextClick)
	ON_COMMAND(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWizard protected methods
----------------------------------------------------------------------------*/

UINT	ChWizard::stdButtons[3] = { IDBACK, IDNEXT, IDCANCEL };

BOOL ChWizard::Create( CWnd* pParent, DWORD dwStyle, DWORD dwExStyle )
{
	LPCSTR		pstrClassName;

	pstrClassName = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS,
											LoadCursor( 0, IDC_ARROW ),
											(HBRUSH)(COLOR_BTNFACE + 1) );

	return CreateEx( dwExStyle, pstrClassName, "Wizard", dwStyle,
						CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
						pParent->GetSafeHwnd(), 0 );
}


BOOL ChWizard::PumpMessage()
{
	MSG		msg;

	if (!::GetMessage( &msg, 0, 0, 0 ))
	{
		return false;
	}
											/* Let's see if the message
												should be handled at all */
	if (CallMsgFilter( &msg, MSGF_DIALOGBOX ))
	{
		return true;
	}
											// Process this message
	if (!WalkPreTranslateTree( m_hWnd, &msg ))
	{
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}

	return true;
}


BOOL ChWizard::ProcessChars( MSG* pMsg )
{
	ChWizardPage*	pPage = GetActivePage();
	HWND			hWnd = pMsg->hwnd;
	UINT			message = pMsg->message;

	if (0 == pPage)
	{
		return false;
	}

	if (0 == hWnd)
	{
		return false;
	}

	switch (message)
	{
		case WM_SYSCHAR:
		{									/* If no control has focus, and Alt
												not down, then ignore */

			if ((::GetFocus() == 0) && (GetKeyState( VK_MENU ) >= 0))
			{
				return false;
			}
			// Fall through
		}

		case WM_CHAR:
		{									/* Ignore chars sent to the dialog
												box (rather than the control) */
			WORD		code;
			HWND		hwndNext;

			if ((hWnd == m_hWnd) || (hWnd == pPage->m_hWnd))
			{
				return false;
			}

			code = (WORD)(DWORD)::SendMessage( hWnd, WM_GETDLGCODE,
												pMsg->wParam,
												(LPARAM)(LPMSG)pMsg );

											/* If the control wants to process
												the message, then don't check
												for possible mnemonic key */

											/* Check if control wants to handle
												this message itself */
			if (code & DLGC_WANTMESSAGE)
			{
				return false;
			}

			if ((WM_CHAR == message) && (code & DLGC_WANTCHARS))
			{
				return false;
			}

			hwndNext = FindNextControl( hWnd, (TCHAR)pMsg->wParam );
			if (0 == hwndNext)
			{								// Nothing found
				return false;
			}
											/* Once we know we are going to
												handle it, call the filter */

			if (CallMsgFilter( pMsg, MSGF_DIALOGBOX ))
			{
				return true;
			}

			GotoControl( hwndNext, (TCHAR)pMsg->wParam );
			return true;
		}
	}

	return false;
}


void ChWizard::GotoControl( HWND hWnd, TCHAR ch )
{
	HWND	hwndFirst;

	for (hwndFirst = 0; hwndFirst != hWnd; hWnd = FindNextControl( hWnd, ch ))
	{
		WORD		code;

		if (0 == hwndFirst)
		{
			hwndFirst = hWnd;
		}

		code = (WORD)(DWORD)::SendMessage( hWnd, WM_GETDLGCODE, 0, 0L );

											/* If a non-disabled static item,
												then jump ahead to nearest
												tabstop */

		if ((code & DLGC_STATIC) && ::IsWindowEnabled( hWnd ))
		{
			ChWizardPage*	pPage = GetActivePage();

			if (::IsChild( pPage->m_hWnd, hWnd ))
			{
				hWnd = ::GetNextDlgTabItem( pPage->m_hWnd, hWnd, false );
			}
			else
			{
				hWnd = ::GetNextDlgTabItem( m_hWnd, hWnd, false );
			}

			code = (WORD)(DWORD)::SendMessage( hWnd, WM_GETDLGCODE, 0, 0L );
		}

		if (::IsWindowEnabled( hWnd ))
		{									// Is it a Pushbutton?
			if (!(code & DLGC_BUTTON))
			{
				SetCtrlFocus( hWnd );
			}
			else
			{								/* Yes, click it, but don't give
												it the focus */

				if ((code & DLGC_DEFPUSHBUTTON) ||
					(code & DLGC_UNDEFPUSHBUTTON))
				{
					chint16		sControlID;
											// Flash the button

					::SendMessage( hWnd, BM_SETSTATE, true, 0L );
					::Sleep( 100 );			// Delay
					::SendMessage( hWnd, BM_SETSTATE, false, 0L );

											// Send the WM_COMMAND message

					sControlID = (chint16)::GetDlgCtrlID( hWnd );
					::SendMessage( ::GetParent( hWnd ), WM_COMMAND,
									MAKEWPARAM( sControlID, (UINT)BN_CLICKED ),
									(LPARAM)hWnd );
				}
				else
				{
					::SetFocus( hWnd );
											/* Send click message if button has
												a UNIQUE mnemonic */

					if (FindNextControl( hWnd, ch ) == hWnd)
					{
						::SendMessage( hWnd, WM_LBUTTONDOWN, 0, 0L );
						::SendMessage( hWnd, WM_LBUTTONUP, 0, 0L );
					}
				}
			}
			return;
		}
	}
}


BOOL ChWizard::ProcessTab( MSG* pMsg )
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB))
	{
		HWND	hWnd = ::GetFocus();
		WORD	code;

		code = (WORD)(DWORD)::SendMessage( hWnd, WM_GETDLGCODE, 0, 0 );

		if ((code & (DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_WANTTAB)) == 0)
		{
			bool	boolShift = (GetKeyState( VK_SHIFT ) < 0);

			if (boolShift &&
					!::IsChild( GetActivePage()->m_hWnd, pMsg->hwnd ) &&
					(::GetNextDlgTabItem( m_hWnd, pMsg->hwnd, true ) ==
						GetActivePage()->m_hWnd))
			{
											/* Shift-tabbing from the sheet
												into the page */

				HWND	hwndPage = GetActivePage()->m_hWnd;

											// Get the first control

				HWND	hwndCtrl = ::GetWindow( hwndPage, GW_CHILD );

											/* Get previous tab item (i.e.
												last tab item in page) */

				hwndCtrl = ::GetNextDlgTabItem( hwndPage, hwndCtrl, true );
				SetCtrlFocus( hwndCtrl );

				return true;
			}
		}
	}

	return false;
}


void ChWizard::CheckDefaultButton( HWND hwndFocusBefore, HWND hwndFocusAfter )
{
	ASSERT( hwndFocusBefore != hwndFocusAfter );

											// Determine old default button
	HWND	hwndOldDefault = 0;
	DWORD	dwOldDefault = 0;

	if (::IsChild( m_hWnd, hwndFocusBefore ))
	{
		hwndOldDefault = hwndFocusBefore;

		if (hwndFocusBefore != 0)
		{
			dwOldDefault = (DWORD)::SendMessage( hwndFocusBefore,
													WM_GETDLGCODE, 0, 0 );
		}

		if (!(dwOldDefault & (DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON)))
		{
			hwndOldDefault = ::GetDlgItem( m_hWnd, IDOK );
			dwOldDefault = (DWORD)::SendMessage( hwndOldDefault, WM_GETDLGCODE,
													0, 0 );
		}
	}
											// Determine new default button
	HWND	hwndDefault = 0;
	DWORD	dwDefault = 0;

	if (::IsChild( m_hWnd, hwndFocusAfter ))
	{
		hwndDefault = hwndFocusAfter;

		if (hwndFocusAfter != 0)
		{
			dwDefault = (DWORD)::SendMessage( hwndFocusAfter, WM_GETDLGCODE,
												0, 0 );
		}

		if (!(dwDefault & (DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON)))
		{
			hwndDefault = ::GetDlgItem( m_hWnd, IDOK );
			dwDefault = (DWORD)::SendMessage( hwndDefault, WM_GETDLGCODE,
												0, 0 );
		}
	}
											// Set new styles

	if ((hwndOldDefault != hwndDefault) && (dwOldDefault & DLGC_DEFPUSHBUTTON))
	{
		::SendMessage( hwndOldDefault, BM_SETSTYLE, BS_PUSHBUTTON, TRUE );
	}

	if (dwDefault & DLGC_UNDEFPUSHBUTTON)
	{
		::SendMessage( hwndDefault, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE );
	}
											/* Remember special case default
												button */

	hwndDefault = (hwndDefault == hwndFocusAfter) ? hwndFocusAfter : 0;
}


void ChWizard::CheckFocusChange()
{
	HWND	hwndFocus = ::GetFocus();

	if (hwndFocus != m_hwndLastFocus)
	{
		CheckDefaultButton( m_hwndLastFocus, hwndFocus );
		m_hwndLastFocus = hwndFocus;
	}
}


bool ChWizard::PageNext()
{
	bool	boolSuccess;

	if (m_iCurPage + 1 < GetPageCount())
	{
		boolSuccess = SetActivePage( m_iCurPage + 1 );
	}
	else
	{
		boolSuccess = false;
	}

	return boolSuccess;
}


bool ChWizard::PageBack()
{
	bool	boolSuccess;

	if (m_iCurPage > 0)
	{
		boolSuccess = SetActivePage( m_iCurPage - 1 );
	}
	else
	{
		boolSuccess = false;
	}

	return boolSuccess;
}


bool ChWizard::GotoPage( chint32 lPageID )
{
	bool	boolSuccess;
	int		iIndex;

	iIndex = FindPage( lPageID );
	if (iIndex >= 0)
	{
		boolSuccess = SetActivePage( iIndex );
	}
	else
	{
		boolSuccess = false;
	}

	return boolSuccess;
}


bool ChWizard::SetActivePage( int iPage )
{
	if (OnPageChanging())
	{
		ChWizardPage*	pPage;
		CRect			rect;

		rect.SetRectEmpty();
											/* Get rectangle from previous
												page if it exists */
		if (m_iCurPage >= 0)
		{
			pPage = GetPage( m_iCurPage );
			if (pPage->m_hWnd != 0)
			{
				pPage->GetWindowRect( &rect );
			}
			ScreenToClient( &rect );
		}
											// Activate next page
		if (iPage >= 0)
		{
			pPage = GetPage( iPage );
			ASSERT( pPage->m_pParentWnd == this );

			if (!pPage->OnSetActive())
			{
				return false;
			}
		}

		m_iCurPage = iPage;
											// Layout next page
		if (m_iCurPage >= 0)
		{
			ChString		strPageCaption;
			ChString		strWizCaption( m_strCaption );

			if (!rect.IsRectEmpty())
			{
				pPage->SetWindowPos( 0, rect.left, rect.top,
										rect.Width(), rect.Height(),
										SWP_NOACTIVATE | SWP_NOZORDER );
			}

			pPage->ShowWindow( SW_SHOW );
			pPage->EnableWindow();
			pPage->SetFocus();
											/* Set the Wizard window title
												for this page */

			pPage->GetWindowText( strPageCaption );
			if (!strWizCaption.IsEmpty() && !strPageCaption.IsEmpty())
			{
				strWizCaption += " - ";
			}
			strWizCaption += strPageCaption;
			SetWindowText( strWizCaption );

			UpdateButtons();
		}
	}

	return true;
}


int ChWizard::FindPage( chint32 lPageID )
{
	int		iIndex = -1;
	int		iLoop;

	for (iLoop = 0; (iLoop < GetPageCount()) && (-1 == iIndex); iLoop++)
	{
		if (GetPage( iLoop )->GetID() == lPageID)
		{
			iIndex = iLoop;
		}
	}

	return iIndex;
}


void ChWizard::RecalcLayout()
{
	CRect	rectPage;
	int		iWidth;
	int		iHeight;
											/* Determine size of the active
												page (active page determines
												initial size) */
	GetActivePage()->GetWindowRect( rectPage );

	iWidth = 2 * m_sizeTabMargin.cx + rectPage.Width() + 3;

											/* Determine total size of the
												buttons */
	int		cxButtons[_countof( stdButtons )];
	int		cxButtonTotal = 0;
	int		cxButtonGap = m_cxButtonGap;
	int		iLoop;

	for (iLoop = 0; iLoop < _countof( stdButtons ); iLoop++)
	{
		ChString		strTemp;
		int			iIndex;

		cxButtons[iLoop] = m_sizeButton.cx;

											/* Load the button caption
												information (may contain button
												size info) */

		#if defined( CH_PUEBLO_PLUGIN )
		ChUtil::Load( (chparam)AfxGetInstanceHandle(), IDS_WIZARD_BTN_BACK + iLoop,
						strTemp );
		#else
		ChUtil::Load( (chparam)PuebloDLL.hModule, IDS_WIZARD_BTN_BACK + iLoop,
						strTemp );
		#endif

		iIndex = strTemp.Find( '\n' );
											/* Format is Apply\n50
												(ie. text\nCX) */
		if (-1 != iIndex)
		{									/* Convert CX fields from text
												dialog units to binary pixels */
			CRect		rect( 0, 0, 0, 0 );

			strTemp = strTemp.Mid( iIndex + 1 );

			rect.right = atoi( strTemp );
			GetActivePage()->MapDialogRect( &rect );
			cxButtons[iLoop] = rect.Width();
		}

		HWND		hWnd = ::GetDlgItem( m_hWnd, stdButtons[iLoop] );

		if ((hWnd != 0) && (GetWindowLong( hWnd, GWL_STYLE ) & WS_VISIBLE))
		{
			cxButtonTotal += cxButtons[iLoop];
		}
	}

	// Margin <Back Next> buttonGap Cancel margin
	// Margin is same as tab margin
	// Button sizes are totaled in cxButtonTotal + cxButtonGap

	iWidth = max( iWidth,
					(2 * m_sizeTabMargin.cx) + cxButtonTotal + cxButtonGap );

	iHeight = (2 * m_sizeTabMargin.cy) + rectPage.Height() + 4 +
					m_sizeTabMargin.cy + m_sizeButton.cy;

	CRect	rectSheet( 0, 0, iWidth, iHeight );
	CRect	rectClient = rectSheet;

	::AdjustWindowRectEx( rectSheet, GetStyle(), false, GetExStyle() );

	SetWindowPos( 0, 0, 0, rectSheet.Width(), rectSheet.Height(),
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	CenterWindow();

	GetActivePage()->SetWindowPos( 0, m_sizeTabMargin.cx + 1,
									m_sizeTabMargin.cy,
									iWidth - m_sizeTabMargin.cx * 2 - 3,
									rectPage.Height(),
									SWP_NOACTIVATE | SWP_NOZORDER );

	int		x = iWidth - m_sizeTabMargin.cx - cxButtonTotal - cxButtonGap;
	int		y = (iHeight - m_sizeTabMargin.cy) - m_sizeButton.cy;

	for (iLoop = 0; iLoop < _countof( stdButtons ); iLoop++)
	{
		HWND	hWnd = ::GetDlgItem( m_hWnd, stdButtons[iLoop] );

		if ((hWnd != 0) && (GetWindowLong( hWnd, GWL_STYLE ) & WS_VISIBLE))
		{
			::MoveWindow( hWnd, x, y, cxButtons[iLoop], m_sizeButton.cy,
							true );
			x += cxButtons[iLoop];

			if (IDNEXT == stdButtons[iLoop])
			{
				x += m_cxButtonGap;
			}
		}
	}

	UpdateButtons();
}


bool ChWizard::CreateStandardButtons()
{
	int		iLoop;

	for (iLoop = 0; iLoop < _countof( stdButtons ); iLoop++)
	{
											/* Load the caption (remove any
												width information) */
		ChString		strCaption;
		HWND		hWnd;

		LoadBtnText( IDS_WIZARD_BTN_BACK + iLoop, strCaption );

											// Create the control

		hWnd = ::CreateWindow( _T("button"), strCaption,
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP |
									BS_PUSHBUTTON,
								0, 0, 0, 0, m_hWnd,
								(HMENU)stdButtons[iLoop],
								AfxGetInstanceHandle(), 0 );
		if (0 == hWnd)
		{
			TRACE0( "Warning: failed to create standard Wizard buttons\n" );
			return false;
		}
											// Set the font
		if (m_hFont != 0)
		{
			::SendMessage( hWnd, WM_SETFONT, (WPARAM)m_hFont, 0 );
		}
	}
											// Special case enable/disable

	::EnableDlgItem( m_hWnd, ID_HELP, IsHelpEnabled() );

	return true;
}


void ChWizard::UpdateButtons()
{
	if (0 == m_iCurPage)
	{										/* Disable the 'back' button
												since we're at the first page */
		EnableDlgItem( m_hWnd, IDBACK, false );
	}
	else
	{
		ChString	strCaption;

		EnableDlgItem( m_hWnd, IDBACK, true );

		if (m_iCurPage + 1 == GetPageCount())
		{									/* Change 'Next' button to
												'Finish' */

			LoadBtnText( IDS_WIZARD_BTN_FINISH, strCaption );
			::SetDlgItemText( m_hWnd, IDNEXT, strCaption );
		}
		else
		{									/* Change 'Next' button to
												'Next' */

			LoadBtnText( IDS_WIZARD_BTN_NEXT, strCaption );
			::SetDlgItemText( m_hWnd, IDNEXT, strCaption );
		}
	}
}


void ChWizard::CancelToClose()
{
	ChString	strCaption;

	#if defined( CH_PUEBLO_PLUGIN )
	ChUtil::Load( (chparam)AfxGetInstanceHandle(), IDS_WIZARD_BTN_CLOSE,
					strCaption );
	#else
	ChUtil::Load( (chparam)PuebloDLL.hModule, IDS_WIZARD_BTN_CLOSE,
					strCaption );
	#endif

	::SetDlgItemText( m_hWnd, IDCANCEL, strCaption );
}


HWND ChWizard::FindNextControl( HWND hWnd, TCHAR ch )
{
	ASSERT( m_hWnd != NULL );

	TCHAR	szText[256];
	HWND	hWndStart;
	HWND	hWndFirst;
	DWORD	dwDlgCode;

	// Check if we are in a group box so we can find local mnemonics.
	hWndStart = GetFirstLevelChild(hWnd);
	hWndFirst = ::GetNextDlgGroupItem(m_hWnd, hWndStart, FALSE);
	hWndFirst = ::GetNextDlgGroupItem(m_hWnd, hWndFirst, TRUE);
	while ((hWndStart = ::GetNextDlgGroupItem(m_hWnd, hWndStart, FALSE)) != NULL)
	{
		if (hWndStart == hWnd || hWndStart == hWndFirst)
			break;

		// Only check for matching mnemonic if control doesn't want characters
		// and control isn't a static control with SS_NOPREFIX
		dwDlgCode = (DWORD) ::SendMessage(hWndStart, WM_GETDLGCODE, 0, 0L);
		if (!(dwDlgCode & DLGC_WANTCHARS) && (!(dwDlgCode & DLGC_STATIC) ||
			!(::GetWindowLong(hWndStart,GWL_STYLE)&& SS_NOPREFIX)))
		{
			::GetWindowText(hWndStart, szText, _countof(szText));
			if (IsCharAfterAmpersand(szText, ch))
				return hWndStart;
		}
	}

	hWnd = hWndStart = GetFirstLevelChild(hWnd);
	for (;;)
	{
		hWnd = ::GetWindow(hWnd,GW_HWNDNEXT);
		if (hWnd == NULL)
			hWnd = ::GetWindow(m_hWnd, GW_CHILD);

		// Only check for matching mnemonic if control doesn't want characters
		// and control isn't a static control with SS_NOPREFIX
		dwDlgCode = (DWORD) ::SendMessage(hWnd, WM_GETDLGCODE, 0, 0L);
		if (!(dwDlgCode & DLGC_WANTCHARS) && (!(dwDlgCode & DLGC_STATIC) ||
			!(::GetWindowLong(hWnd,GWL_STYLE) & SS_NOPREFIX)))
		{
			::GetWindowText(hWnd, szText, _countof(szText));
			if (IsCharAfterAmpersand(szText, ch))
				break;
		}

		if (hWnd == hWndStart)
			return NULL;
	}

	return hWnd;
}


HWND ChWizard::GetFirstLevelChild( HWND hWndLevel )
{
	if ((hWndLevel == m_hWnd) ||
		!(::GetWindowLong(hWndLevel,GWL_STYLE) & WS_CHILD))
	{
		return NULL;
	}

	HWND hWnd = hWndLevel;
	do
	{
		if (hWndLevel == m_hWnd)
			break;
		if (!(::GetWindowLong(hWndLevel,GWL_STYLE) & WS_CHILD))
			break;
		hWnd = hWndLevel;
	} while ((hWndLevel = ::GetParent(hWndLevel)) != NULL);

	return hWnd;
}


/*----------------------------------------------------------------------------
	ChWizard message handlers
----------------------------------------------------------------------------*/

int ChWizard::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (CWnd::OnCreate( lpCreateStruct ) == -1)
	{
		return -1;
	}
											/* Fix-up the system menu so this
												looks like a dialog box */
	CMenu*	pSysMenu = GetSystemMenu( false );

	ASSERT( pSysMenu != 0 );

	int		iLoop;
	int		iCount = pSysMenu->GetMenuItemCount();

	for (iLoop = 0; iLoop < iCount; iLoop++)
	{
		UINT	uiID = pSysMenu->GetMenuItemID( iLoop );

		if ((uiID != SC_MOVE) && (uiID != SC_CLOSE))
		{
			pSysMenu->DeleteMenu( iLoop, MF_BYPOSITION );
			iLoop--;
			iCount--;
		}
	}
											// Set active page and active tab
	SetActivePage( m_iCurPage );
											// Initialize font used for buttons
	ChWizardPage*	pPage = GetActivePage();
	HFONT			hFont;
	CRect			rect( 0, 0, 0, 0 );

	ASSERT( 0 == m_hFont );
	ASSERT_VALID( pPage );

	hFont = (HFONT)pPage->SendMessage( WM_GETFONT );
	if (hFont != 0)
	{
		LOGFONT		logFont;
		VERIFY(::GetObject( hFont, sizeof( LOGFONT ), &logFont ) );

		m_hFont = CreateFontIndirect( &logFont );
	}
											/* Calculate button sizes and
												separator */

	rect.right = 50;						// normal size buttons
	rect.bottom = 14;
	rect.left = 4;							// button gap is 4 dialog units

	pPage->MapDialogRect( rect );
	m_sizeButton.cx = rect.right;
	m_sizeButton.cy = rect.bottom;
	m_cxButtonGap = rect.left;
											// Calculate tab margin area

	rect.bottom = rect.right = 4;			/* Standard dialog margin is 6
												dialog units */
	pPage->MapDialogRect( rect );
	m_sizeTabMargin.cx = rect.right;
	m_sizeTabMargin.cy = rect.bottom;
											// Create standard buttons
	if (!CreateStandardButtons())
	{
		return -1;
	}

	RecalcLayout();

	pPage = GetPage( m_iCurPage );
	if (pPage != 0)
	{
		pPage->SetFocus();
	}

	return 0;   // success
}


void ChWizard::OnBackClick()
{
	ASSERT_VALID( this );

	if (GetActivePage()->OnBack())
	{
		chint32		lResult = OnBack();

		if (0 == lResult)
		{
			PageBack();
		}
		else if (-1 != lResult)
		{
			GotoPage( lResult );
		}
	}
}


void ChWizard::OnNextClick()
{
	ASSERT_VALID( this );

	if (GetActivePage()->OnNext())
	{
		if (m_iCurPage + 1 == GetPageCount())
		{
			OnPageChanging();				// Does the KillActive notification
			OnFinish();
		}
		else
		{
			chint32		lResult = OnNext();

			if (0 == lResult)
			{
				PageNext();
			}
			else if (-1 != lResult)
			{
				GotoPage( lResult );
			}
		}
	}
}


void ChWizard::OnCancel()
{
	ASSERT_VALID( this );

	GetActivePage()->OnCancel();

	EndDialog( IDCANCEL );
}


void ChWizard::OnPaint()
{
	if (-1 == m_iCurPage)
	{
		return;
	}

	ASSERT( m_pages.GetSize() > 0 );

	ChWizardPage*	pPage = GetPage( m_iCurPage );
	CRect			rect;

	pPage->GetWindowRect( &rect );
	ScreenToClient( &rect );

	rect.top = rect.bottom + 1;
	rect.bottom += CY_BORDER * 2;

	CPaintDC		dc( this );

	if (m_boolDrawEdgeAvailable)
	{
		HDC		hDC = dc.GetSafeHdc();

		DrawEdge( hDC, &rect, EDGE_ETCHED, BF_TOP );
	}
	else
	{										/* Hacked edge for versions that
												don't have DrawEdge... Draw a
												black and a white line */

		HPEN	hpenBlack = (HPEN)GetStockObject( BLACK_PEN );
		HPEN	hpenWhite = (HPEN)GetStockObject( WHITE_PEN );
		CPen*	pTempPen = CPen::FromHandle( hpenBlack );
		CPen*	pOldPen = dc.SelectObject( pTempPen );

		dc.MoveTo( rect.left, rect.top );
		dc.LineTo( rect.left, rect.top );

		pTempPen = CPen::FromHandle( hpenWhite );
		dc.SelectObject( pTempPen );

		dc.MoveTo( rect.left, rect.top + 1 );
		dc.LineTo( rect.left, rect.top + 1 );

		if (pOldPen)
		{
			dc.SelectObject( pOldPen );
		}
	}
}


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( HWND )
ChGetSafeOwner( CWnd* pParent, HWND* phTopLevel )
{
											/* Get parent window for modal
												dialogs and message boxes */
	CWnd*	pWnd = pParent;
											/* Attempt to find window to
												start with */
	if (pWnd->GetSafeHwnd() == 0)
	{
		pWnd = AfxGetMainWnd();
	}
											/* Get top-level parent from
												pParent */

	CWnd*	pTopLevel = pWnd->GetTopLevelParent();
	HWND	hTopLevel = pTopLevel->GetSafeHwnd();

											/* Don't return disabled windows
												as top-level parent */

	if ((hTopLevel != 0) && !::IsWindowEnabled( hTopLevel ))
	{
		hTopLevel = 0;
	}
											/* Remember top level parent, if
												necessary */
	if (phTopLevel != 0)
	{
		*phTopLevel = hTopLevel;
	}
											/* Return last active popup on the
												top level window, except when
												pParent was provided */
	if ((pParent != 0) || (pTopLevel == 0))
	{
		return pParent->GetSafeHwnd();
	}
	else
	{
		return ::GetLastActivePopup( pTopLevel->GetSafeHwnd() );
	}
}


CH_INTERN_FUNC( void )
ChDeleteObject( HGDIOBJ* pObject )
{
	ASSERT( pObject != 0 );

	if (*pObject != 0)
	{
		DeleteObject( *pObject );
		*pObject = 0;
	}
}


CH_INTERN_FUNC( void )
SetCtrlFocus( HWND hWnd )
{
	if (::SendMessage( hWnd, WM_GETDLGCODE, 0, 0L ) & DLGC_HASSETSEL)
	{
		::SendMessage( hWnd, EM_SETSEL, 0, -1 );
	}

	::SetFocus( hWnd );
}


CH_INTERN_FUNC( void )
EnableDlgItem( HWND hWnd, UINT uiID, bool boolEnable, int iIDFocus )
{
	HWND	hwndItem = ::GetDlgItem( hWnd, uiID );

	if ((::GetFocus() == hwndItem) && (iIDFocus != -1))
	{
		HWND	hwndTemp = ::GetDlgItem( hWnd, iIDFocus );

		if (hwndTemp != 0)
		{
			::SetFocus( hwndTemp );
		}
	}

	if (hwndItem != 0)
	{
		::EnableWindow( hwndItem, boolEnable );
	}
}


CH_INTERN_FUNC( void )
LoadBtnText( int iID, ChString& strText )
{
	int		iIndex;

	#if defined( CH_PUEBLO_PLUGIN )
	ChUtil::Load( (chparam)AfxGetInstanceHandle(), iID, strText );
	#else
	ChUtil::Load( (chparam)PuebloDLL.hModule, iID, strText );
	#endif

	iIndex = strText.Find( '\n' );
	if (-1 != iIndex)
	{
		strText = strText.Left( iIndex );
	}
}


CH_INTERN_FUNC( bool )
IsCharAfterAmpersand( LPTSTR lpsz, TCHAR chFind )
{
	ASSERT( AfxIsValidString( lpsz ) );

	CharLowerBuff( &chFind, 1 );

	while (*lpsz != '\0')
	{
		if (*lpsz == '&')
		{
			++lpsz; // Note: '&' is not lead-byte
			if (*lpsz != '&')
			{
				TCHAR	ch = *lpsz;

				CharLowerBuff( &ch, 1 );

				return ch == chFind;
			}
		}

		lpsz = _tcsinc( lpsz );
	}

	return false;
}
