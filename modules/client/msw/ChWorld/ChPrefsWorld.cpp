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

	Implementation of the ChWorldPrefsPage class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <cderr.h>

#include "ChWorld.h"
#include "ChPrefsWorld.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define TINTIN_OPEN_DLG_FLAGS	(OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |\
									OFN_HIDEREADONLY)


/*----------------------------------------------------------------------------
	ChWorldPrefsPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChWorldPrefsPage, ChPropertyBaseClass )

ChWorldPrefsPage::ChWorldPrefsPage() :
		ChPropertyBaseClass( ChWorldPrefsPage::IDD, 0 
			#if !defined( CH_PUEBLO_PLUGIN )
				,ChWorldDLL.hModule 
			#endif
		),
		m_reg( WORLD_PREFS_GROUP )
{
	m_reg.ReadBool( WORLD_PREFS_ECHO, m_boolEcho, true );
	m_reg.ReadBool( WORLD_PREFS_ECHO_BOLD, m_boolBold, true );
	m_reg.ReadBool( WORLD_PREFS_ECHO_ITALIC, m_boolItalic, false );
	m_reg.ReadBool( WORLD_PREFS_PAUSE_DISCONNECT, m_boolPauseOnDisconnect, true );
	m_reg.ReadBool( WORLD_PREFS_PAUSE_INLINE, m_boolPauseInline, true );

	//{{AFX_DATA_INIT(ChWorldPrefsPage)
	//}}AFX_DATA_INIT
}

ChWorldPrefsPage::~ChWorldPrefsPage()
{
}

void ChWorldPrefsPage::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyBaseClass::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChWorldPrefsPage)
	DDX_Check(pDX, IDC_ECHO_BOLD, m_boolBold);
	DDX_Check(pDX, IDC_ECHO_INPUT, m_boolEcho);
	DDX_Check(pDX, IDC_ECHO_ITALIC, m_boolItalic);
	DDX_Check(pDX, IDC_PAUSE_DISCONNECT, m_boolPauseOnDisconnect);
	DDX_Check(pDX, IDC_PAUSE_INLINE, m_boolPauseInline);
	//}}AFX_DATA_MAP
}


void ChWorldPrefsPage::UpdateButtons()
{
	CButton*	pButton = (CButton*)GetDlgItem( IDC_ECHO_INPUT );

	if (0 == pButton->GetCheck())
	{
		GetDlgItem( IDC_ECHO_BOLD )->EnableWindow( false );
		GetDlgItem( IDC_ECHO_ITALIC )->EnableWindow( false );
		GetDlgItem( IDC_ECHO_STYLE_BOX )->EnableWindow( false );
	}
	else
	{
		GetDlgItem( IDC_ECHO_BOLD )->EnableWindow();
		GetDlgItem( IDC_ECHO_ITALIC )->EnableWindow();
		GetDlgItem( IDC_ECHO_STYLE_BOX )->EnableWindow();
	}
	
	pButton = (CButton*)GetDlgItem( IDC_PAUSE_DISCONNECT );
	if (0 == pButton->GetCheck()) {
		GetDlgItem( IDC_PAUSE_INLINE )->EnableWindow( false );
	} else {
		GetDlgItem( IDC_PAUSE_INLINE )->EnableWindow();
	}
}


#if defined( CH_PUEBLO_PLUGIN )
BOOL ChWorldPrefsPage::OnKillActive( )
{
	UpdateData();

	m_reg.WriteBool( WORLD_PREFS_ECHO, m_boolEcho );
	m_reg.WriteBool( WORLD_PREFS_ECHO_BOLD, m_boolBold );
	m_reg.WriteBool( WORLD_PREFS_ECHO_ITALIC, m_boolItalic );
	m_reg.WriteBool( WORLD_PREFS_PAUSE_DISCONNECT, m_boolPauseOnDisconnect );
	m_reg.WriteBool( WORLD_PREFS_PAUSE_INLINE, m_boolPauseInline );

	return true;
}
#else
void ChWorldPrefsPage::OnCommit()
{
	if (0 != m_hWnd)
	{
		UpdateData();

		m_reg.WriteBool( WORLD_PREFS_ECHO, m_boolEcho );
		m_reg.WriteBool( WORLD_PREFS_ECHO_BOLD, m_boolBold );
		m_reg.WriteBool( WORLD_PREFS_ECHO_ITALIC, m_boolItalic );
		m_reg.WriteBool( WORLD_PREFS_PAUSE_DISCONNECT, m_boolPauseOnDisconnect );
		m_reg.WriteBool( WORLD_PREFS_PAUSE_INLINE, m_boolPauseInline );
	}

	ChPropertyBaseClass::OnCommit();
}
#endif


BEGIN_MESSAGE_MAP( ChWorldPrefsPage, ChPropertyBaseClass )
	//{{AFX_MSG_MAP(ChWorldPrefsPage)
	ON_BN_CLICKED(IDC_PAUSE_DISCONNECT, OnPauseDisconnect)
	ON_BN_CLICKED(IDC_ECHO_INPUT, OnEchoInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWorldPrefsPage message handlers
----------------------------------------------------------------------------*/

BOOL ChWorldPrefsPage::OnInitDialog() 
{
	ChPropertyBaseClass ::OnInitDialog();

	CenterWindow();
	UpdateButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ChWorldPrefsPage::OnEchoInput() 
{
	UpdateButtons();
}


void ChWorldPrefsPage::OnPauseDisconnect() 
{
	UpdateButtons();
}


/*----------------------------------------------------------------------------
	ChTextInputPrefsPage class
----------------------------------------------------------------------------*/
	
IMPLEMENT_DYNCREATE( ChTextInputPrefsPage, ChPropertyBaseClass )

ChTextInputPrefsPage::ChTextInputPrefsPage() :
			ChPropertyBaseClass( ChTextInputPrefsPage::IDD, 0
			#if !defined( CH_PUEBLO_PLUGIN )
				, ChWorldDLL.hModule 
			#endif
			 ),
			m_reg( WORLD_PREFS_GROUP ),
			m_boolDirty( false ),
			m_boolInitialized( false ),
			m_sEditLines( 0 )
{
	//{{AFX_DATA_INIT(ChTextInputPrefsPage)
	m_iKeyMap = -1;
	m_boolClear = FALSE;
	m_strLineCount = _T("");
	//}}AFX_DATA_INIT
}

ChTextInputPrefsPage::~ChTextInputPrefsPage()
{
}


BOOL ChTextInputPrefsPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyBaseClass::OnSetActive();

	if (!m_boolInitialized)
	{
		ChString		strKeyMap;

		m_reg.Read( WORLD_PREFS_KEYMAP, strKeyMap,
					WORLD_PREFS_KEYMAP_DEF );
		m_keyMap.Set( strKeyMap );

		m_reg.Read( WORLD_EDIT_LINES, m_sEditLines,
					WORLD_EDIT_LINES_DEF );
		m_reg.ReadBool( WORLD_PREFS_CLEAR, m_boolClear,
						WORLD_PREFS_CLEAR_DEF );

		m_reg.Read( WORLD_TINTIN_FILE, m_strTinTinFile,
					WORLD_TINTIN_FILE_DEF );
		DisplayTinTinName( m_strTinTinFile );
											// Initialize the dialog data
		UpdateData( FALSE );
											// Limit the edit field to 2 digits
		m_editLineCount.LimitText( 2 );
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


BOOL ChTextInputPrefsPage::OnKillActive()
{
	BOOL	boolSuccess;

	if (boolSuccess = ChPropertyBaseClass::OnKillActive())
	{
		if ((GetEditLines() <= 0) || (GetEditLines() > WORLD_EDIT_LINES_MAX))
		{
			ChString		strFormat;
			ChString		strOut;

			LOADSTRING( IDS_LINES_OUT_OF_RANGE, strFormat );
			strOut.Format( strFormat, (int)WORLD_EDIT_LINES_MAX );
			MessageBox( strOut );
			m_editLineCount.SetFocus();

			boolSuccess = FALSE;
		}
	}

	if ( boolSuccess )
	{
		UpdateData();

		m_reg.Write( WORLD_PREFS_KEYMAP, m_keyMap.GetName() );
		m_reg.Write( WORLD_EDIT_LINES, m_sEditLines );
		m_reg.WriteBool( WORLD_PREFS_CLEAR, m_boolClear );
		m_reg.Write( WORLD_TINTIN_FILE, m_strTinTinFile );
	}

	return boolSuccess;
}


#if !defined( CH_PUEBLO_PLUGIN )

void ChTextInputPrefsPage::OnCommit()
{
	if (m_boolInitialized)
	{										// Write data to registry

		m_reg.Write( WORLD_PREFS_KEYMAP, m_keyMap.GetName() );
		m_reg.Write( WORLD_EDIT_LINES, m_sEditLines );
		m_reg.WriteBool( WORLD_PREFS_CLEAR, m_boolClear );
		m_reg.Write( WORLD_TINTIN_FILE, m_strTinTinFile );
	}     
}
#endif


void ChTextInputPrefsPage::SetEditLines( chint16 sLines )
{
	m_sEditLines = sLines;

	if (GetEditLines() <= 0)
	{
		m_sEditLines = 1;
	}
	else if (GetEditLines() > WORLD_EDIT_LINES_MAX)
	{
		m_sEditLines = WORLD_EDIT_LINES_MAX;
	}
}


void ChTextInputPrefsPage::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyBaseClass::DoDataExchange( pDX );

	if (!pDX->m_bSaveAndValidate)
	{
		m_strLineCount.Format( "%hd", m_sEditLines );

		switch( m_keyMap )
		{
			case keymapEmacs:
			{
				m_iKeyMap = 1;
				break;
			}

			default:
			{
				m_iKeyMap = 0;
				break;
			}
		}
	}

	//{{AFX_DATA_MAP(ChTextInputPrefsPage)
	DDX_Control(pDX, IDC_EDIT_LINE_COUNT, m_editLineCount);
	DDX_Radio(pDX, IDC_RADIO_WINDOWS, m_iKeyMap);
	DDX_Check(pDX, IDC_CHECK_CLEAR, m_boolClear);
	DDX_Text(pDX, IDC_EDIT_LINE_COUNT, m_strLineCount);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_sEditLines = atoi( m_strLineCount );

		switch( m_iKeyMap )
		{
			case 1:
			{
				m_keyMap = keymapEmacs;
				break;
			}

			default:
			{
				m_keyMap = keymapWindows;
				break;
			}
		}
	}
}


BEGIN_MESSAGE_MAP( ChTextInputPrefsPage, ChPropertyBaseClass )
	//{{AFX_MSG_MAP(ChTextInputPrefsPage)
	ON_EN_UPDATE(IDC_EDIT_LINE_COUNT, OnUpdateEditLineCount)
	ON_BN_CLICKED(IDC_TINTIN_FILE, OnTintinFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTextInputPrefsPage protected methods
----------------------------------------------------------------------------*/

void ChTextInputPrefsPage::DisplayTinTinName( const ChString& strName )
{
	CStatic*	pStatic = (CStatic*)GetDlgItem( IDC_STATIC_TINTIN_FILE );

	if (strName.GetLength())
	{
		ChString		strBrowserName( strName );
		int			iIndex = strBrowserName.ReverseFind( '\\' );

		if (-1 != iIndex)
		{
			strBrowserName = strBrowserName.Mid( iIndex + 1 );
		}

		pStatic->SetWindowText( strBrowserName );
	}
	else
	{	
		ChString		strTag;

		LOADSTRING( IDS_TINTIN_NO_FILE, strTag );
		pStatic->SetWindowText( strTag );
	}
}


/*----------------------------------------------------------------------------
	ChTextInputPrefsPage message handlers
----------------------------------------------------------------------------*/

void ChTextInputPrefsPage::OnUpdateEditLineCount() 
{
	ChString		strNewText;
	const char*	pstrOld;

	m_editLineCount.GetWindowText( m_strLineCount );
	pstrOld = m_strLineCount;

	while (*pstrOld)
	{
		if (isdigit( *pstrOld ))
		{
			strNewText += *pstrOld;
		}

		pstrOld++;
	}

	if (strNewText != m_strLineCount)
	{
		int		iStart;
		int		iEnd;

		m_editLineCount.GetSel( iStart, iEnd );

		m_editLineCount.SetWindowText( strNewText );

											/* Subtract one since we're
												removing a character from
												the edit field */
		if (iStart > 0)	iStart--;
		if (iEnd > 0)	iEnd--;

		m_editLineCount.SetSel( iStart, iEnd );
		MessageBeep( MB_OK );
	}
}


void ChTextInputPrefsPage::OnTintinFile() 
{
	ChString		strFilter;
	ChString		strTitle;
	int			iResult;

	LOADSTRING( IDS_OPEN_TINTIN_FILTER, strFilter );
	LOADSTRING( IDS_OPEN_TINTIN_TITLE, strTitle );

	ChTinTinSelectFileDlg	browseDlg( strTitle, m_strTinTinFile, strFilter,
											AfxGetMainWnd() );

	iResult = browseDlg.DoModal();

	// Read the registry again
	m_reg.Read( WORLD_TINTIN_FILE, m_strTinTinFile, WORLD_TINTIN_FILE_DEF );
	DisplayTinTinName( m_strTinTinFile );
}


/*----------------------------------------------------------------------------
	ChTinTinSelectFileDlg class
----------------------------------------------------------------------------*/

ChTinTinSelectFileDlg::ChTinTinSelectFileDlg( const ChString& strTitle,
														const ChString& strBrowser,
														const ChString& strFilter,
														CWnd* pParent ) :
		ChFileDialog( true, "txt", 0, TINTIN_OPEN_DLG_FLAGS, strFilter ),
		m_reg( WORLD_PREFS_GROUP )
{
	ChString		strBrowserName( strBrowser );
	int			iIndex = strBrowser.ReverseFind( '\\' );

	if (-1 != iIndex)
	{
		m_strInitialDir = strBrowser.Left( iIndex + 1 );
		strBrowserName = strBrowserName.Mid( iIndex + 1 );
	}
											/* Setup initial file name
												(Copied into buffer in
													CFileDialog) */
	lstrcpyn( m_szFileName, strBrowserName,
				(sizeof( m_szFileName ) / sizeof( m_szFileName[0] )) );

	m_ofn.lpstrTitle = strTitle;
	m_ofn.lpstrInitialDir = m_strInitialDir;

	SetTemplate(
#if defined( CH_PUEBLO_PLUGIN )
			0,		// defaults to AfxGetInstanceHandle()
#else
			ChWorldDLL.hModule,
#endif
			IDD_SELECT_TINTIN, IDD_SELECT_TINTIN_95);

	//{{AFX_DATA_INIT(ChTinTinSelectFileDlg)
	//}}AFX_DATA_INIT
}


int ChTinTinSelectFileDlg::DoModal() 
{
	int		iResult;

	if (IDOK == (iResult = ChFileDialog::DoModal()))
	{
		m_reg.Write( WORLD_TINTIN_FILE, GetPathName() );
	}
	else if (IDC_NO_TINTIN_FILE == iResult)
	{
		m_reg.Write( WORLD_TINTIN_FILE, "" );
	}

	return iResult;
}


void ChTinTinSelectFileDlg::DoDataExchange( CDataExchange* pDX )
{
	ChFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChTinTinSelectFileDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChTinTinSelectFileDlg, ChFileDialog )
	//{{AFX_MSG_MAP(ChTinTinSelectFileDlg)
    ON_BN_CLICKED(IDC_NO_TINTIN_FILE, OnNoTinTinFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTinTinSelectFileDlg message handlers
----------------------------------------------------------------------------*/

void ChTinTinSelectFileDlg::OnNoTinTinFile()
{
  EndDialog(IDC_NO_TINTIN_FILE);
}

// $Log$
