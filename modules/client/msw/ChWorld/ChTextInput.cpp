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

	Implementation for the ChTextInputBar class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChConst.h>
#include <ChCore.h>
#include <ChSplit.h>
#include <ChUtil.h>
#include <ChHtpCon.h>

#include "World.h"
#include "ChTextInput.h"
#include "ChTextOutput.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define INPUT_BANNER_TITLE		"_TEXT_INPUT_BANNER_"
#define MAX_MENU_STRING_LEN		30

#define TEXT_INPUT_EDIT_ID		12345


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( void )
TruncateMenuString( ChString& strText );


/*----------------------------------------------------------------------------
	ChTextInput class
----------------------------------------------------------------------------*/

ChTextInput::ChTextInput( ChWorldMainInfo* pMainInfo ) :
				m_pMainInfo( pMainInfo ),
				m_pBanner( 0 ),
				m_pEdit( 0 ),
				m_pPasswordEdit( 0 ),
				m_boolShown( false ),
				m_boolLoginRecognized( false ),
				m_sEditLines( 2 )
{
	Reset();

	UpdatePreferences();
	CreateEditField();
}


ChTextInput::~ChTextInput()
{
	if (IsShown())
	{
		Show( false );
	}

	if (m_pBanner)
	{
		ChSplitter*		pSplitter = GetMainInfo()->GetCore()->GetSplitter();

											/* Disassociate and destroy the
												edit field */
		m_pBanner->SetChild( 0 );
		if (m_pEdit)
		{
			delete m_pEdit;
			m_pEdit = 0;
		}
		if (m_pPasswordEdit)
		{
			delete m_pPasswordEdit;
			m_pPasswordEdit = 0;
		}
											// Destroy the banner
		pSplitter->DestroyBanner( m_pBanner );
		m_pBanner = 0;
	}
}


void ChTextInput::Show( bool boolShow )
{
	if (boolShow && !IsShown())
	{										// Show the banner
		if (m_pBanner)
		{
			Reset();
			m_pBanner->ShowWindow( SW_SHOW );
			SetFocus();
		}

		m_boolShown = true;
	}
	else if (!boolShow && IsShown())
	{										// Hide the banner
		if (m_pBanner)
		{
			m_pBanner->ShowWindow( SW_HIDE );
		}

		m_boolShown = false;
	}
}


void ChTextInput::SetFocus()
{
	ASSERT( 0 != GetEdit() );

	GetEdit()->PostMessage( WM_CHACO_GRABFOCUS );
}


void ChTextInput::Reset()
{
	m_boolLoginRecognized = false;

	if (m_pEdit && m_pPasswordEdit)
	{
		SetEcho( true, false );
	}
}


void ChTextInput::Clear()
{
	ASSERT( 0 != GetEdit() );

	GetEdit()->EraseText();
}


void ChTextInput::SetEcho( bool boolEcho, bool boolPreserve )
{
	if (boolEcho)
	{
		if (boolPreserve)
		{								/* Preserve what's already in the
											password edit field */
			ChString		strText;
			DWORD		dwSel;

			m_pPasswordEdit->GetWindowText( strText );
			m_pEdit->SetWindowText( strText );

			dwSel = m_pPasswordEdit->GetSel();
			m_pEdit->SetSel( dwSel );
		}
		else
		{								/* Not preserving, so empty the
											edit field */
			m_pEdit->EraseText();
		}
										// Set the new child into the banner
		m_pBanner->SetChild( m_pEdit );
		SizeEditField();

		m_pPasswordEdit->ShowWindow( SW_HIDE );
		m_pEdit->ShowWindow( SW_SHOW );
	}
	else
	{
		if (boolPreserve)
		{								/* Preserve what's already in the
											non-password edit field */
			ChString		strText;
			DWORD		dwSel;

			m_pEdit->GetWindowText( strText );
			m_pPasswordEdit->SetWindowText( strText );

			dwSel = m_pEdit->GetSel();
			m_pPasswordEdit->SetSel( dwSel );
		}
		else
		{								/* Not preserving, so empty the
											edit field */
			m_pPasswordEdit->EraseText();
		}
										// Set the new child into the banner

		m_pBanner->SetChild( m_pPasswordEdit );
		SizeEditField();

		m_pEdit->ShowWindow( SW_HIDE );
		m_pPasswordEdit->ShowWindow( SW_SHOW );
	}
}


void ChTextInput::SetInputLines( int iCount )
{
	ChRegistry	reg( WORLD_PREFS_GROUP );
	chint16		sOldEditLines = GetEditLines();

	if (iCount < 1)
	{
		iCount = 1;
	}
	else if (iCount > WORLD_EDIT_LINES_MAX)
	{
		iCount = WORLD_EDIT_LINES_MAX;
	}

	m_sEditLines = (chint16)iCount;

	if (sOldEditLines != GetEditLines())
	{										// Save the number of edit lines

		reg.Write( WORLD_EDIT_LINES, m_sEditLines );

											// Size the banner pane
		SizeEditField();
	}
}


void ChTextInput::UpdatePreferences()
{
	ChRegistry	reg( WORLD_PREFS_GROUP );
	chint16		sOldEditLines = GetEditLines();

											// Read in the number of edit lines

	reg.Read( WORLD_EDIT_LINES, m_sEditLines, WORLD_EDIT_LINES_DEF );

	if (GetEdit())
	{										/* Tell the edit field to update
												itself as well */
		GetEdit()->UpdatePreferences();
	}

	if (m_pBanner && (sOldEditLines != GetEditLines()))
	{
		SizeEditField();
	}
}


#if !defined( CH_PUEBLO_PLUGIN )
bool ChTextInput::CheckEditMenuItem( EditMenuItem item )
{
	bool	boolEnable;

	switch( item )
	{
		case editMenuCut:
		case editMenuCopy:
		{
			int		iStart;
			int		iEnd;

			GetEdit()->GetSel( iStart, iEnd );
			boolEnable = (iStart != iEnd);
			break;
		}

		case editMenuPaste:
		{
			if (GetEdit()->OpenClipboard())
			{
				boolEnable = (0 != ::GetClipboardData( CF_TEXT ));
				::CloseClipboard();
			}
			break;
		}

		default:
		{
			boolEnable = false;
			break;
		}
	}

	return boolEnable;
}


void ChTextInput::DoEditMenuItem( EditMenuItem item )
{
	switch( item )
	{
		case editMenuCut:
		{
			GetEdit()->Cut();
			break;
		}

		case editMenuCopy:
		{
			GetEdit()->Copy();
			break;
		}

		case editMenuPaste:
		{
			GetEdit()->Paste();
			break;
		}

		default:
		{
			break;
		}
	}
}

#endif // #if !defined( CH_PUEBLO_PLUGIN )


void ChTextInput::CheckForPasswordProtection()
{
	ChWorldInfo*	pWorldInfo = GetMainInfo()->GetWorldInfo();

	if (pWorldInfo && (unamePwLogin != pWorldInfo->GetLoginType()) &&
			!m_boolLoginRecognized)
	{
		ChTextInputEdit*	pEdit = GetEdit();

		if (pEdit)
		{
			EchoState	echo = GetMainInfo()->GetEchoState();
			ChString		strText;

			pEdit->GetWindowText( strText );

			if (echoAutoOff == echo)
			{
				if (!MatchMushLogin( strText ))
				{
					GetMainInfo()->SetEchoState( echoOn, true );
				}
			}
			else
			{
				if (echoOn == echo)
				{
					if (MatchMushLogin( strText ))
					{
						GetMainInfo()->SetEchoState( echoAutoOff, true );
					}
				}
			}
		}
	}
}


/*----------------------------------------------------------------------------
	ChTextInput protected methods
----------------------------------------------------------------------------*/

void ChTextInput::CreateEditField()
{
	ChSplitter*	pSplitter =  GetMainInfo()->GetCore()->GetSplitter();
	chint16		sClientHeight = 30;
	CSize		editSize;
											/* Get the edit size for the number
												of lines desired */

	ChTextInputEdit::GetSize( GetEditLines(), editSize );

											/* Create the banner and the edit
												fields */
	m_pEdit = new ChTextInputEdit( false );
	m_pPasswordEdit = new ChTextInputEdit( true );

	m_pBanner = pSplitter->CreateBanner( 0, INPUT_BANNER_TITLE, false, true,
											(chint16)editSize.cx,
											(chint16)editSize.cy );

	m_pEdit->Create( m_pBanner, GetMainInfo() );
	m_pPasswordEdit->Create( m_pBanner, GetMainInfo() );

											// Set the banner information
	m_pBanner->SetChild( m_pEdit );
}


void ChTextInput::SizeEditField()
{
	ChSize	editSize;
											// Size the banner pane

	ChTextInputEdit::GetSize( GetEditLines(), editSize );
	m_pBanner->SetChildSize( (chint16)editSize.cx, (chint16)editSize.cy );
}


bool ChTextInput::MatchMushLogin( const ChString& strText )
{
	const char*		pstrText = strText;
	bool			boolMatch = false;
											/* This function will return
												true if the specified text
												matches the form:
													'c[o|r]* * ?' */
	if (*pstrText == 'c' || *pstrText == 'C')
	{										// Found 'c'
		pstrText++;

		if ((*pstrText == 'o') || (*pstrText == 'O'))
		{
											// Found 'co'
			pstrText++;

			boolMatch = true;
		}
		else if ((*pstrText == 'r') || (*pstrText == 'R'))
		{
											// Found 'cr'
			pstrText++;

			boolMatch = true;
		}

		if (boolMatch)
		{
			boolMatch = false;

			while (*pstrText && !isspace( *pstrText ))
			{
											/* Eat everything after 'c?'
												up to a space */
				pstrText++;
			}

			if (isspace( *pstrText ))
			{								// Found 'c* '
				while (isspace( *pstrText ))
				{
					pstrText++;
				}

				while (*pstrText && !isspace( *pstrText ))
				{
					pstrText++;
				}

				if (*pstrText)
				{							// Found 'c* *'

					while (isspace( *pstrText ))
					{
						pstrText++;
					}

					if (*pstrText)
					{						// Found 'c* * ?'
						boolMatch = true;
					}
				}
			}
		}
	}

	return boolMatch;
}


/*----------------------------------------------------------------------------
	ChTextInputEdit constants
----------------------------------------------------------------------------*/

#define NUM_EDIT_CHARS_PER_LINE		80

#define INPUT_EDIT_STYLE			(WS_CHILD | WS_BORDER | WS_VISIBLE | \
										ES_AUTOVSCROLL | ES_MULTILINE)

#define INPUT_EDIT_PW_STYLE			(WS_CHILD | WS_BORDER | ES_AUTOVSCROLL | \
										ES_PASSWORD)


/*----------------------------------------------------------------------------
	ChTextInputEdit class
----------------------------------------------------------------------------*/

ChTextInputEdit::ChTextInputEdit( bool boolPassword ) :
					m_boolPassword( boolPassword ),
					m_h3dLib( 0 ),
					m_pprocSubclassCtl3d( 0 ),
					m_boolBrowsingHistory( false ),
					m_tabCompletionMode( tabModeReset ),
					m_posTabCompletion( 0 )
{
	const ChClientInfo*	pClientInfo = ChCore::GetClientInfo();
	OSType				osType = pClientInfo->GetPlatform();

	m_boolWindows95 = ((osWin95 == osType) || (osWinXP == osType));

	if (!m_boolWindows95 && (m_h3dLib = LoadLibrary( "Ctl3d32.dll" )))
	{
		m_pprocSubclassCtl3d =
			(SubclassCtl3dProc)GetProcAddress( m_h3dLib, "Ctl3dSubclassCtl" );
	}
											// Init registry values
	UpdatePreferences();
}


ChTextInputEdit::~ChTextInputEdit()
{
	if (m_h3dLib)
	{
		FreeLibrary( m_h3dLib );
		m_h3dLib = 0;
		m_pprocSubclassCtl3d = 0;
	}
}


BOOL ChTextInputEdit::Create( ChWnd* pParent, ChWorldMainInfo* pMainInfo )
{
	CRect		rtEdit( 0, 0, 100, 100 );
	chflag32	flStyle;
											// Cache the pMainInfo
	m_pMainInfo = pMainInfo;
											// Create the window

	flStyle = IsPassword() ? INPUT_EDIT_PW_STYLE : INPUT_EDIT_STYLE;

	return CEdit::CreateEx( WS_EX_CLIENTEDGE, "edit", "_Text_In_Module_Edit_",
							flStyle, rtEdit.left, rtEdit.top,
							rtEdit.Width(), rtEdit.Height(),
							pParent->GetSafeHwnd(),
							(HMENU)TEXT_INPUT_EDIT_ID );
}


void ChTextInputEdit::GetSize( int iLines, CSize& size )
{
	CWindowDC	dc( CWnd::GetDesktopWindow() );
	TEXTMETRIC	tm;

	dc.GetTextMetrics( &tm );

	size.cy = (chint16)(tm.tmHeight + tm.tmExternalLeading) * iLines;
	size.cx = (chint16)(tm.tmAveCharWidth * NUM_EDIT_CHARS_PER_LINE);

	size.cx += (3 * GetSystemMetrics( SM_CXFRAME ));
	size.cy += (3 * GetSystemMetrics( SM_CYFRAME ));

	#if (INPUT_EDIT_STYLE & WS_VSCROLL)
		size.cx += GetSystemMetrics( SM_CXVSCROLL );
	#endif

	#if (INPUT_EDIT_STYLE & WS_HSCROLL)
		size.cy += GetSystemMetrics( SM_CYHSCROLL );
	#endif
}


BOOL ChTextInputEdit::OnChildNotify( UINT message, WPARAM wParam,
										LPARAM lParam, LRESULT* pResult )
{
	if ((WM_COMMAND == message) && (EN_UPDATE == HIWORD( wParam )))
	{
		ChTextInput*	pTextInput = GetMainInfo()->GetTextInput();

		if (pTextInput)
		{
			pTextInput->CheckForPasswordProtection();
		}
	}

	return CEdit::OnChildNotify( message, wParam, lParam, pResult );
}


void ChTextInputEdit::EraseText()
{
	SetWindowText( "" );
}


void ChTextInputEdit::UpdatePreferences()
{
	ChRegistry	reg( WORLD_PREFS_GROUP );
	ChString		strKeyMap;

	reg.Read( WORLD_PREFS_KEYMAP, strKeyMap, WORLD_PREFS_KEYMAP_DEF );
	m_keyMapType.Set( strKeyMap );
	m_keyMapType.CreateMap( m_keyMap );

	reg.ReadBool( WORLD_PREFS_CLEAR, m_boolClearOnSend,
					WORLD_PREFS_CLEAR_DEF );
}


BEGIN_MESSAGE_MAP( ChTextInputEdit, CEdit )
	//{{AFX_MSG_MAP(ChTextInputEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
#if defined( CH_PUEBLO_PLUGIN )
	ON_WM_KEYUP()
#endif
	ON_MESSAGE( WM_CHACO_GRABFOCUS, OnGrabFocus )
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTextInputEdit protected methods
----------------------------------------------------------------------------*/

void ChTextInputEdit::OnSendKey()
{
	ChString				strText;
	ChWorldMainInfo*	pMainInfo = GetMainInfo();

	GetWindowText( strText );
	pMainInfo->GetTinTin()->ParseInput( strText );

											/* Scroll to the end of the
												output buffer */

	GetMainInfo()->GetTextOutput()->GetOutputWnd()->End();

	if (pMainInfo->GetEchoState() == echoAutoOff)
	{
		pMainInfo->GetTextInput()->SetLoginRecognized();
	}
											/* Don't add blank lines or passwords
												to the history */

	if (!IsPassword() && (strText.GetLength() > 0))
	{
		m_history.Add( strText );			// Add to the history
	}

	if (m_boolClearOnSend || IsPassword())
	{
		EraseText();
	}
	else
	{
		SetSel( 0, -1 );					// Select all text
	}
											/* We're no longer browsing the
												history, if we were before... */
	SetBrowsingHistory( false );

	if (pMainInfo->GetEchoState() == echoAutoOff)
	{
		pMainInfo->SetEchoState( echoOn );
	}
}


void ChTextInputEdit::MoveInHistory( bool boolUp )
{
	ChString		strLine;

	if (boolUp)
	{
		if (!IsBrowsingHistory())
		{									/* When we start browsing the
												history, we save the current
												text and use it as the 'end'
												text for browsing */
			GetWindowText( m_strEndText );
			SetBrowsingHistory( true );
		}

		if (m_history.GetPrevious( strLine ))
		{
			bool	boolNewText;
			ChString	strCurrent;

			GetWindowText( strCurrent );
											/* We haven't modified since
												we sent the text -- skip the
												last stored line */
			if (strLine == strCurrent)
			{
				boolNewText = m_history.GetPrevious( strLine );
			}
			else
			{
				boolNewText = true;
			}

			if (boolNewText)
			{
				SetHistoryText( strLine );
			}
		}
	}
	else
	{
		if (m_history.GetNext( strLine ))
		{
			SetHistoryText( strLine );
		}
		else
		{									/* We're at the end of the history
												again... */
			SetHistoryText( m_strEndText );
			SetBrowsingHistory( false );
		}
	}
}


void ChTextInputEdit::SetBrowsingHistory( bool boolBrowsing )
{
	if (m_boolBrowsingHistory != boolBrowsing)
	{
		m_boolBrowsingHistory = boolBrowsing;

		if (!m_boolBrowsingHistory)
		{									// Reset the history to the end
			m_history.Reset();
		}
	}
}


void ChTextInputEdit::DoTabCompletion()
{
	ChString		strText;

	if (tabModeReset == m_tabCompletionMode)
	{										// Reset the expansion string
		GetWindowText( m_strTabCompletion );
	}

	if ((tabModeReset == m_tabCompletionMode) ||
		(tabModeStart == m_tabCompletionMode))
	{										// Start from the beginning
		m_posTabCompletion = 0;
		m_tabCompletionMode = tabModeHistory;
	}

	strText = m_strTabCompletion;

	switch( m_tabCompletionMode )
	{
		case tabModeHistory:
		{
			if (m_history.GetExpansion( strText, m_posTabCompletion ))
			{
				int		iPos = m_strTabCompletion.GetLength();

				SetWindowText( strText );
				SetSel( iPos, iPos );

				if (0 == m_posTabCompletion)
				{							// We're at the end of the list

					m_tabCompletionMode = tabModeEnd;
				}
			}
			else
			{								// We've hit the end of the list
				MessageBeep( MB_ICONASTERISK );
				m_tabCompletionMode = tabModeStart;
			}
			break;
		}

		case tabModeEnd:
		{
			MessageBeep( MB_ICONASTERISK );
			m_tabCompletionMode = tabModeStart;
			break;
		}

		case tabModeReset:
		default:
		{
			break;
		}
	}
}


void ChTextInputEdit::DoRightButtonMenu( CPoint ptMouse )
{
	CMenu	popupMenu;

	popupMenu.CreatePopupMenu();
	ConstructRightButtonMenu( popupMenu );

	popupMenu.TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x,
								ptMouse.y, this, 0 );
}


void ChTextInputEdit::ConstructRightButtonMenu( CMenu& menu )
{
	int		iHistoryCount = m_history.GetCount();
	ChString	strText;
											// Add history items
	if (iHistoryCount > 0)
	{
		int		iItem = 0;
		int		iItemsInMenu;

		iItemsInMenu = min( maxMenuHistory, iHistoryCount );

		for (iItem = iHistoryCount - iItemsInMenu; iItem < iHistoryCount; iItem++)
		{
			ChString		strMenu;

			strMenu = m_history.GetString( iItem );
			TruncateMenuString( strMenu );
			menu.AppendMenu( MF_STRING, popupMenuHistoryBase + iItem,
								strMenu );
		}
	}
}


int ChTextInputEdit::GetEndOfLineIndex( int iLine ) const
{
	int		iChar;

	if (iLine == GetLineCount() - 1)
	{
		iChar = GetWindowTextLength();
	}
	else
	{
		iChar = LineIndex( iLine + 1 ) - 1;
	}

	return iChar;
}


void ChTextInputEdit::SendKeyDown( UINT uiKey, LPARAM lParam,
									bool boolStripCtrl )
{
											/* This function will send a key to
												the edit control, first
												stripping off the control key
												if it is pressed */
	static BYTE	bKeyStateArray[256];

	bool		boolControlWasPressed;
	BYTE		bControlBack;

	if (boolStripCtrl)
	{
		GetKeyboardState( bKeyStateArray );

		bControlBack = bKeyStateArray[VK_CONTROL];

		if (boolControlWasPressed = !!(bControlBack & 0x80))
		{
											// Turn off the control key

			bKeyStateArray[VK_CONTROL] &= ~0x80;
			SetKeyboardState( bKeyStateArray );
		}
	}
											// Send the key
	SendMessage( WM_KEYDOWN, uiKey, lParam );

	if (boolStripCtrl && boolControlWasPressed)
	{										// Restore the control key
		
		bKeyStateArray[VK_CONTROL] = bControlBack;
		SetKeyboardState( bKeyStateArray );
	}
}


bool ChTextInputEdit::ProcessKey( UINT& uiChar, LPARAM lParam )
{
	bool		boolContinue = false;
	bool		boolInTabCompletion = false;
	ChPosition	pos;
	chflag32	flMods = 0;
	bool		boolStripControl;

	if (GetKeyState( VK_CONTROL ) & 0x8000)
	{
		boolStripControl = true;
		flMods |= ACTION_MOD_CONTROL;
	}
	else
	{
		boolStripControl = false;
	}

	pos = m_keyMap.FindItem( uiChar, flMods );
	if (pos)
	{
		ChKeyMapItem*	pItem = m_keyMap.GetItem( pos );

		ASSERT( pItem );

		switch( (KeyAction)pItem->GetUserData() )
		{
			case actSend:
			{
				OnSendKey();
				break;
			}

			case actTranspose:
			{
				int		iStart;
				int		iEnd;
				bool	boolLegal = false;

				GetSel( iStart, iEnd );

				if (iStart == iEnd)
				{
					int		iLen = GetWindowTextLength();

					if ((iEnd > 0) && (iEnd < iLen))
					{
						ChString		strText;
						ChString		strNewText;

						GetWindowText( strText );
						strNewText = strText.Left( iEnd - 1 );
						strNewText += strText[iEnd];
						strNewText += strText[iEnd - 1];
						strNewText += strText.Mid( iEnd + 1 );

						SetWindowText( strNewText );
						SetSel( iEnd, iEnd );

						boolLegal = true;
					}
				}

				if (!boolLegal)
				{							/* Transpose doesn't work here...
												beep */
					MessageBeep( MB_OK );
				}
				break;
			}

			case actHistoryPrev:
			{
				MoveInHistory( true );
				break;
			}

			case actHistoryNext:
			{
				MoveInHistory( false );
				break;
			}

			case actCursorLeft:
			{
				SendKeyDown( VK_LEFT, lParam, boolStripControl );
				break;
			}

			case actCursorRight:
			{
				SendKeyDown( VK_RIGHT, lParam, boolStripControl );
				break;
			}

			case actCursorHome:
			{
				SetSel( 0, 0 );
				break;
			}

			case actCursorEnd:
			{
				int		iChar = GetWindowTextLength();

				SetSel( iChar, iChar );
				break;
			}

			case actCursorStartLine:
			{
				SendKeyDown( VK_HOME, lParam, boolStripControl );
				break;
			}

			case actCursorEndLine:
			{
				SendKeyDown( VK_END, lParam, boolStripControl );
				break;
			}

			case actCursorUp:
			{
				if (0 == LineFromChar())
				{
					MoveInHistory( true );
				}
				else
				{
					SendKeyDown( VK_UP, lParam, boolStripControl );
				}
				break;
			}

			case actCursorDown:
			{
				int		iLastLineIndex = GetLineCount() - 1;

				if (iLastLineIndex == LineFromChar())
				{
					MoveInHistory( false );
				}
				else
				{
					SendKeyDown( VK_DOWN, lParam, boolStripControl );
				}
				break;
			}

			case actTabCompletion:
			{
				DoTabCompletion();
				boolInTabCompletion = true;
				break;
			}

			case actDeleteText:
			{
				EraseText();
				break;
			}

			case actDeleteToEndOfBuffer:
			{
				int		iStart;
				int		iEnd;

				GetSel( iStart, iEnd );

				if (iStart == iEnd)
				{							// No selection

					SetSel( iStart, GetWindowTextLength() );
				}
											// Clear the selection
				Clear();
				break;
			}

			case actDeleteNextChar:
			{
				int		iStart;
				int		iEnd;
				int		iLen = GetWindowTextLength();
				bool	boolLegal = true;

				GetSel( iStart, iEnd );

				if (iStart == iEnd)
				{							// No selection
					if (iEnd < iLen)
					{
						iEnd++;
						SetSel( iStart, iEnd );
					}
					else
					{
						MessageBeep( MB_OK );
						boolLegal = false;
					}
				}

				if (boolLegal)
				{							// Clear the selection
					Clear();
				}
				break;
			}

			case actLogPageUp:
			{
				GetMainInfo()->GetTextOutput()->GetOutputWnd()->PageUp();
				break;
			}

			case actLogPageDown:
			{
				GetMainInfo()->GetTextOutput()->GetOutputWnd()->PageDown();
				break;
			}

			case actLogHome:
			{
				GetMainInfo()->GetTextOutput()->GetOutputWnd()->Home();
				break;
			}

			case actLogEnd:
			{
				GetMainInfo()->GetTextOutput()->GetOutputWnd()->End();
				break;
			}

			default:
			{
				boolContinue = true;
				break;
			}
		}
	}
	else
	{
		boolContinue = true;
	}

	if (!boolInTabCompletion)
	{
		m_tabCompletionMode = tabModeReset;
	}

	return boolContinue;
}


/*----------------------------------------------------------------------------
	ChTextInputEdit message handlers
----------------------------------------------------------------------------*/

#if !defined( CH_PUEBLO_PLUGIN )
BOOL ChTextInputEdit::PreTranslateMessage( MSG* pMsg )
{
	BOOL boolCancel;

	if (WM_KEYDOWN == pMsg->message)
	{
		if (ProcessKey( pMsg->wParam, pMsg->lParam ))
		{
			boolCancel = CEdit::PreTranslateMessage( pMsg );
		}
		else
		{
			boolCancel = TRUE;
		}
	}
	else if (WM_KEYUP == pMsg->message)
	{
		if (VK_RETURN == pMsg->wParam)
		{
			boolCancel = TRUE;
		}
	}
	else
	{
		boolCancel = CEdit::PreTranslateMessage( pMsg );
	}

	return boolCancel;
}
#endif


int ChTextInputEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	if (m_pprocSubclassCtl3d)
	{
		m_pprocSubclassCtl3d( GetSafeHwnd() );
	}

	return 0;
}


BOOL ChTextInputEdit::OnCommand( WPARAM wParam, LPARAM lParam )
{
	bool	boolProcessed = true;
	ChString	strText;

	if ((wParam >= popupMenuHistoryBase) &&
				(wParam <= popupMenuHistoryBase + maxHistory))
	{
		int		iIndex = wParam - popupMenuHistoryBase;

		strText = m_history.GetString( iIndex );
	}
	else
	{
		boolProcessed = false;
	}

	if (boolProcessed && !strText.IsEmpty())
	{
		SetWindowText( strText );
		OnSendKey();
	}

	return boolProcessed;
}


void ChTextInputEdit::OnKillFocus( CWnd* pNewWnd )
{
	CEdit::OnKillFocus( pNewWnd );

	GetMainInfo()->SetFocusTarget( focusTextInput, false );
}


void ChTextInputEdit::OnSetFocus( CWnd* pOldWnd )
{
	CEdit::OnSetFocus( pOldWnd );

	GetMainInfo()->SetFocusTarget( focusTextInput, true );
}


void ChTextInputEdit::OnRButtonDown( UINT nFlags, CPoint ptMouse )
{
	ClientToScreen( &ptMouse );
	DoRightButtonMenu( ptMouse );
}


void ChTextInputEdit::OnShowWindow( BOOL boolShow, UINT nStatus )
{
	CEdit::OnShowWindow( boolShow, nStatus );

	if (boolShow)
	{
		PostMessage( WM_CHACO_GRABFOCUS );
	}
}


LONG ChTextInputEdit::OnGrabFocus( UINT wParam, LONG lParam )
{
	SetFocus();
	return 0;
}


#if defined( CH_PUEBLO_PLUGIN )

LRESULT ChTextInputEdit::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{

	if (WM_KEYDOWN == message )
	{
		if (!ProcessKey( wParam, lParam ))
		{
			MSG msg;
			PeekMessage( &msg, GetSafeHwnd(), WM_CHAR, WM_CHAR, PM_REMOVE );
			return 0;
		}
	}
	else if (WM_KEYUP == message )
	{
		if (VK_RETURN == wParam)
		{
			return 0;
		}
	}

	return CEdit::WindowProc(  message,  wParam,  lParam );

}
  

void ChTextInputEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	if ( nChar != VK_RETURN )
	{
		if ( GetKeyState( VK_CONTROL ) & 0x8000  )
		{
			if ( nChar == TEXT( 'C' ) )
			{
				Copy();
			}
			else if ( nChar == TEXT( 'V' ) )
			{
				Paste();
			}
			else if ( nChar == TEXT( 'X' ) )
			{
				Cut();
			}
			else if ( nChar == TEXT( 'Z' ) || nChar == TEXT( 'A' ))
			{
				Undo();
			}

		}
		CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}
#endif

/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( void )
TruncateMenuString( ChString& strText )
{
	if (strText.GetLength() > MAX_MENU_STRING_LEN)
	{
									/* Truncate the string to make a
										happy menu, and add ellipses */

		strText = strText.Left( MAX_MENU_STRING_LEN );
		strText += "...";
	}
}

// $Log$
