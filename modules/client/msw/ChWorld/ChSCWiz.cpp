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

	Contains the implementation of the ChShortcutWizard class, which is
	a Wizard for creating a desktop shortcut file.

----------------------------------------------------------------------------*/

// $Header$

#if defined( CH_MSW )

#include "headers.h"
#include <sys/stat.h>
#include <ddeml.h>

#include <ChCore.h>

#include "ChSCWiz.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	ChShortcutWizard class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChShortcutWizard, ChWizard )

ChShortcutWizard::ChShortcutWizard( CWnd* pParentWnd ) :
					ChWizard( 
#if defined( CH_PUEBLO_PLUGIN )
					(chparam)AfxGetInstanceHandle(), 
#else
					(chparam)ChWorldDLL.hModule, 
#endif
						IDS_SHORTCUT_WIZARD,
								pParentWnd ),
					m_boolUseCurrWorld( false ),
					m_boolEnableCurrWorld( false )
{
	AddPages();
}


ChShortcutWizard::~ChShortcutWizard()
{
}


chint32 ChShortcutWizard::OnBack()
{
	chint32		lReturn = 0;

	if (m_boolUseCurrWorld &&
		(GetActivePageID() == IDD_SHORTCUT_WIZ_USERNAME))
	{
											// Skip the server page
		lReturn = IDD_SHORTCUT_WIZ_NAME;
	}

	return lReturn;
}


chint32 ChShortcutWizard::OnNext()
{
	chint32		lReturn = 0;

	if (m_boolUseCurrWorld &&
		(GetActivePageID() == IDD_SHORTCUT_WIZ_NAME))
	{
											// Skip the server page
		lReturn = IDD_SHORTCUT_WIZ_USERNAME;
	}

	return lReturn;
}


BEGIN_MESSAGE_MAP( ChShortcutWizard, ChWizard )
	//{{AFX_MSG_MAP(ChShortcutWizard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizard public methods
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChShortcutWizard protected methods
----------------------------------------------------------------------------*/

void ChShortcutWizard::AddPages()
{
	this->AddPage( &m_namePage );
	this->AddPage( &m_serverPage );
	this->AddPage( &m_usernamePage );
	this->AddPage( &m_optionsPage );
	this->AddPage( &m_finishPage );
}


/*----------------------------------------------------------------------------
	ChShortcutWizName class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChShortcutWizName, ChWizardPage )

ChShortcutWizName::ChShortcutWizName() :
			ChWizardPage( ChShortcutWizName::IDD ),
			m_pWiz( 0 )
{
	//{{AFX_DATA_INIT(ChShortcutWizName)
	m_strName = _T("");
	m_boolCurrWorld = FALSE;
	//}}AFX_DATA_INIT
}


ChShortcutWizName::~ChShortcutWizName()
{
}


BOOL ChShortcutWizName::OnInitPage()
{
	BOOL	boolSetFocus = ChWizardPage::OnInitPage();

	m_pWiz = (ChShortcutWizard*)GetParent();
	m_boolCurrWorld = m_pWiz->UseCurrWorld();

	m_checkCurrWorld.EnableWindow( m_pWiz->EnableUseCurrWorld() );

	m_editName.LimitText( constNameLimit );

	UpdateData( FALSE );

	return boolSetFocus;
}


BOOL ChShortcutWizName::OnNext()
{
	BOOL		boolValid = TRUE;
	ChString		strMessage;

	UpdateData();

	boolValid = !m_strName.IsEmpty();

	if (boolValid)
	{										// Update parent
		m_pWiz->SetUseCurrWorld( m_boolCurrWorld != FALSE );
	}
	else
	{
		LOADSTRING( IDS_SHORTCUT_INVALID_NAME, strMessage );
		MessageBox( strMessage, 0, MB_OK | MB_ICONEXCLAMATION );
	}

	return boolValid;
}


const ChString& ChShortcutWizName::GetFilename()
{
	GetMangledFilename( m_strFilename );

	return m_strFilename;
}


void ChShortcutWizName::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChShortcutWizName)
	DDX_Control(pDX, IDC_CHECK_CURR_WORLD, m_checkCurrWorld);
	DDX_Control(pDX, IDC_SHORTCUT_NAME, m_editName);
	DDX_Text(pDX, IDC_SHORTCUT_NAME, m_strName);
	DDX_Check(pDX, IDC_CHECK_CURR_WORLD, m_boolCurrWorld);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChShortcutWizName, ChWizardPage )
	//{{AFX_MSG_MAP(ChShortcutWizName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizName protected methods
----------------------------------------------------------------------------*/

void ChShortcutWizName::GetMangledFilename( ChString& strFilename )
{
	ChString			strExtension;
	struct _stat	statInfo;
	ChString			strFilePath;
	int				iTry = 1;
	ChString			strTestName;

	LOADSTRING( IDS_SHORTCUT_EXTENSION, strExtension );

	strFilename = GetName();
	strFilename = strFilename.Left( 30 );
											/* Strip out characters in the name
												that we don't want to allow.
												This includes illegal filename
												characters. */
	StripChars( strFilename, "<>:\"/\\|." );

	switch( ChUtil::GetSystemType() )
	{
		case CH_SYS_WIN32S:
		case CH_SYS_WIN3X:
		{									/* On older versions of Windows, we
												also need to strip out more
												characters and truncate to 8
												chars long */
			StripChars( strFilename, " ?*+&" );
			strFilename = strFilename.Left( 8 );
			break;
		}

		default:
		{									// Don't need to do anything
			break;
		}
	}

	if (strFilename.IsEmpty())
	{
		strFilename = "ShtCut";
	}
	else if (isdigit( strFilename[0] ))
	{
		strFilename = "S" + strFilename;

		switch( ChUtil::GetSystemType() )
		{
			case CH_SYS_WIN32S:
			case CH_SYS_WIN3X:
			{									/* On older versions of Windows, we
													also need to strip out more
													characters and truncate to 8
													chars long */
				strFilename = strFilename.Left( 8 );
				break;
			}

			default:
			{									// Don't need to do anything
				break;
			}
		}
	}


	do
	{
		strTestName = strFilename;

		if (1 == iTry)
		{
			strTestName += "." + strExtension;
		}
		else
		{
			char	buffer[10];

			sprintf( buffer, "%d", iTry );

			switch( ChUtil::GetSystemType() )
			{
				case CH_SYS_WIN32S:
				case CH_SYS_WIN3X:
				{
					int		iLen;

					iLen = strTestName.GetLength() + strlen( buffer );
					if (iLen > 8)
					{
						int		iNewLen = 8 - strlen( buffer );

						strTestName = strTestName.Left( iNewLen );
					}
					strTestName += buffer;
					break;
				}

				default:
				{
					strTestName += " ";
					strTestName += buffer;
					break;
				}
			}

			strTestName += "." + strExtension;
		}

		strFilePath = GetPath() + "\\" + strTestName;

		iTry++;

	} while (_stat( strFilePath, &statInfo ) == 0);

	strFilename = strTestName;
}


void ChShortcutWizName::StripChars( ChString& strData, const ChString& strChars )
{
	int		iIndex;

	while (-1 != (iIndex = strData.FindOneOf( strChars )))
	{
		strData = strData.Left( iIndex ) + strData.Mid( iIndex + 1 );
	}
}


/*----------------------------------------------------------------------------
	ChShortcutWizName message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChShortcutWizServer class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChShortcutWizServer, ChWizardPage )

ChShortcutWizServer::ChShortcutWizServer() :
			ChWizardPage( ChShortcutWizServer::IDD ),
			m_type( undefinedType )
{
	//{{AFX_DATA_INIT(ChShortcutWizServer)
	m_strType = _T("");
	m_strHost = _T("");
	m_iPort = 0;
	//}}AFX_DATA_INIT
}


ChShortcutWizServer::~ChShortcutWizServer()
{
}


BOOL ChShortcutWizServer::OnInitPage()
{
	BOOL	boolSetFocus = ChWizardPage::OnInitPage();

	ChWorldType::FillTypeList( &m_comboTypes );

	m_editHost.LimitText( constHostLimit );
	m_editPort.LimitText( constPortLimit );
											// Make sure the data is up-to-date
	UpdateData( FALSE );

	return boolSetFocus;
}


BOOL ChShortcutWizServer::OnNext()
{
	bool		boolValid = true;
	ChString		strMessage;
	ChString		strCaption;

	UpdateData();

	boolValid = !m_strHost.IsEmpty() &&
				(0 != m_iPort) &&
				(m_type != undefinedType);

	if (!boolValid)
	{
		LOADSTRING( IDS_SHORTCUT_INVALID_SERVER_CAPTION, strCaption );
	}

	if (m_strHost.IsEmpty())
	{
		LOADSTRING( IDS_SHORTCUT_INVALID_SERVER_HOST, strMessage );
		MessageBox( strMessage, strCaption, MB_OK | MB_ICONEXCLAMATION );
	}
	else if (0 == m_iPort)
	{
		LOADSTRING( IDS_SHORTCUT_INVALID_SERVER_PORT, strMessage );
		MessageBox( strMessage, strCaption, MB_OK | MB_ICONEXCLAMATION );
	}
	else if (m_type == undefinedType)
	{
		LOADSTRING( IDS_SHORTCUT_INVALID_SERVER_TYPE, strMessage );
		MessageBox( strMessage, strCaption, MB_OK | MB_ICONEXCLAMATION );
	}

	return boolValid;
}


void ChShortcutWizServer::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChShortcutWizServer)
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_HOST, m_editHost);
	DDX_Control(pDX, IDC_COMBO_TYPES, m_comboTypes);
	DDX_CBString(pDX, IDC_COMBO_TYPES, m_strType);
	DDX_Text(pDX, IDC_EDIT_HOST, m_strHost);
	DDX_Text(pDX, IDC_EDIT_PORT, m_iPort);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{

		if ( 0 == m_strType.GetLength() 
						&& m_comboTypes.GetCurSel() != CB_ERR )
		{  					// GetWindowText does not work under Win32s
							// On Win32s try getting data based on the selection
			m_comboTypes.GetLBText( m_comboTypes.GetCurSel(), m_strType );	
		}

		if (m_strType.GetLength())
		{
			m_type.Set( m_strType );
		}
		else
		{
			m_type = undefinedType;
		}
	}
	else
	{
		if (m_type == undefinedType)
		{
			m_comboTypes.SetCurSel( -1 );
		}
		else
		{
			ChString		strType = m_type.GetName();
			int			iResult;

			iResult = m_comboTypes.SelectString( -1, strType );
		}
	}
}


BEGIN_MESSAGE_MAP( ChShortcutWizServer, ChWizardPage )
	//{{AFX_MSG_MAP(ChShortcutWizServer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizServer protected methods
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChShortcutWizServer message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChShortcutWizUsername class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChShortcutWizUsername, ChWizardPage )

ChShortcutWizUsername::ChShortcutWizUsername() :
			ChWizardPage( ChShortcutWizUsername::IDD ),
			m_type( undefinedType )
{
	//{{AFX_DATA_INIT(ChShortcutWizUsername)
	m_strPassword = _T("");
	m_strUsername = _T("");
	m_iLoginStyle = -1;
	//}}AFX_DATA_INIT

	m_loginType = variableLogin;
}


ChShortcutWizUsername::~ChShortcutWizUsername()
{
}


BOOL ChShortcutWizUsername::OnInitPage()
{
	BOOL		boolSetFocus = ChWizardPage::OnInitPage();

	m_editUsername.LimitText( constUsernameLimit );
	m_editPassword.LimitText( constPasswordLimit );

	return boolSetFocus;
}


void ChShortcutWizUsername::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChShortcutWizUsername)
	DDX_Control(pDX, IDC_STATIC_LOGIN_STYLE, m_staticLoginStyle);
	DDX_Control(pDX, IDC_RADIO_LOGIN_MUD, m_radioMudLogin);
	DDX_Control(pDX, IDC_RADIO_LOGIN_CONNECT, m_radioConnectLogin);
	DDX_Control(pDX, IDC_SHORTCUT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_SHORTCUT_USERNAME, m_editUsername);
	DDX_Text(pDX, IDC_SHORTCUT_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_SHORTCUT_USERNAME, m_strUsername);
	DDX_Radio(pDX, IDC_RADIO_LOGIN_MUD, m_iLoginStyle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChShortcutWizUsername, ChWizardPage )
	//{{AFX_MSG_MAP(ChShortcutWizUsername)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizUsername message handlers
----------------------------------------------------------------------------*/

void ChShortcutWizUsername::OnShowWindow( BOOL boolShow, UINT nStatus )
{
	ChWizardPage::OnShowWindow( boolShow, nStatus );

	if (boolShow)
	{
		ChShortcutWizard*	pWiz = (ChShortcutWizard*)GetParent();
		ChString				strHost;
		chint16				sPort;
		ChWorldType			type;

		ASSERT( pWiz );
		pWiz->GetServer( strHost, sPort, type );

		if (type != m_type)
		{									// Type has changed
			m_type = type;
	
			if (variableLogin == m_loginType)
			{
				m_loginType = m_type.GetLoginType();
			}

			if (connectLogin == m_loginType)
			{
				m_iLoginStyle = 1;
			}
			else
			{
				m_iLoginStyle = 0;
			}
											/* If the server type allows
												variable login, then allow
												them to change it */

			if (variableLogin == type.GetLoginType())
			{
				m_staticLoginStyle.ShowWindow( SW_SHOW );
				m_radioMudLogin.ShowWindow( SW_SHOW );
				m_radioConnectLogin.ShowWindow( SW_SHOW );
			}
			else
			{
				m_staticLoginStyle.ShowWindow( SW_HIDE );
				m_radioMudLogin.ShowWindow( SW_HIDE );
				m_radioConnectLogin.ShowWindow( SW_HIDE );
			}

			UpdateData( false );
		}
	}
}


/*----------------------------------------------------------------------------
	ChShortcutWizOptions class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChShortcutWizOptions, ChWizardPage )

ChShortcutWizOptions::ChShortcutWizOptions() :
			ChWizardPage( ChShortcutWizOptions::IDD )
{
	//{{AFX_DATA_INIT(ChShortcutWizOptions)
	m_strGroup = _T("");
	//}}AFX_DATA_INIT
}


ChShortcutWizOptions::~ChShortcutWizOptions()
{
}


BOOL ChShortcutWizOptions::OnInitPage()
{
	BOOL	boolSetFocus = ChWizardPage::OnInitPage();

	if (ChUtil::GetSystemType() == CH_SYS_WIN95)
	{
		SetupForProgman();
	}
	else
	{
		SetupForProgman();
	}

	return boolSetFocus;
}


void ChShortcutWizOptions::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChShortcutWizOptions)
	DDX_Control(pDX, IDC_SHORTCUT_FOLDER, m_comboShortcutFolder);
	DDX_Control(pDX, IDC_SHORTCUT_MSG, m_staticShortcutMsg);
	DDX_CBString(pDX, IDC_SHORTCUT_FOLDER, m_strGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChShortcutWizOptions, ChWizardPage )
	//{{AFX_MSG_MAP(ChShortcutWizOptions)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizOptions protected methods
----------------------------------------------------------------------------*/

void ChShortcutWizOptions::SetupForProgman()
{
	DWORD		idDDEInst = ChCore::GetDDEInstance();
	HCONV		hDDEConversation;
	HSZ			hszService;
	HSZ			hszTopic;
	HSZ			hszRequest;
	HDDEDATA	hProgData;
	char*		pstrData;
											/* Get a list of program manager
												groups (via DDE) and add them
												to the combo box */

	hszService = DdeCreateStringHandle( idDDEInst, PROGMAN_SERVICE,
										CP_WINANSI );
	hszTopic = DdeCreateStringHandle( idDDEInst, PROGMAN_TOPIC,
										CP_WINANSI );

											// Initiate a conversation

	hDDEConversation = DdeConnect( idDDEInst, hszService, hszTopic, 0 );

											// Send the request to the server

	hszRequest = DdeCreateStringHandle( idDDEInst, "GROUPS", CP_WINANSI );
	hProgData = DdeClientTransaction( 0, 0, hDDEConversation, hszRequest,
										CF_TEXT, XTYP_REQUEST, 5000, 0 );

											// Lock the returned data

	pstrData =  (char*)DdeAccessData( hProgData, 0 );

											/* Now parse the returned data,
												adding each item to the list */
	ChString		strData( pstrData );
	ChString		strGroup;
	int			iIndex;

	while (!strData.IsEmpty())
	{
		iIndex = strData.Find( '\r' );
		if (iIndex == -1)
		{
			strGroup = strData;
			strData = "";
		}
		else
		{
			int		iTrim = 0;

			strGroup = strData.Left( iIndex );
			strData = strData.Mid( iIndex + 1 );

			while ((iTrim < strData.GetLength()) && isspace( strData[iTrim] ))
			{
				iTrim++;
			}

			strData = strData.Mid( iTrim );
		}
											// Add the item

		m_comboShortcutFolder.AddString( strGroup );
	}
											// Unlock the data
	DdeUnaccessData( hProgData );
											// Clean up from DDE
	DdeFreeStringHandle( idDDEInst, hszService );
	DdeFreeStringHandle( idDDEInst, hszTopic );
	DdeFreeStringHandle( idDDEInst, hszRequest );

											// Now select an appropriate choice
	ChString		strAppName;

	LOADSTRING( IDS_APP_NAME, strAppName );
	iIndex = m_comboShortcutFolder.FindString( -1, strAppName );
	if (CB_ERR == iIndex)
	{										/* The group beginning with the
												app name wasn't found, so add
												an option for the user */
		ChString		strGroup;

		LOADSTRING( IDS_GROUP_NAME, strGroup );
		iIndex = m_comboShortcutFolder.AddString( strGroup );
	}

	if (CB_ERR != iIndex)
	{
		m_comboShortcutFolder.SetCurSel( iIndex );
	}
	else
	{
		m_comboShortcutFolder.SetCurSel( -1 );
	}
}


/*----------------------------------------------------------------------------
	ChShortcutWizOptions message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChShortcutWizFinish class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChShortcutWizFinish, ChWizardPage )

ChShortcutWizFinish::ChShortcutWizFinish() :
			ChWizardPage( ChShortcutWizFinish::IDD )
{
	//{{AFX_DATA_INIT(ChShortcutWizFinish)
	//}}AFX_DATA_INIT
}


ChShortcutWizFinish::~ChShortcutWizFinish()
{
}


BOOL ChShortcutWizFinish::OnInitPage()
{
	BOOL				boolSetFocus = ChWizardPage::OnInitPage();
	ChShortcutWizard*	pWizard = (ChShortcutWizard*)GetParent();

	if (ChUtil::GetSystemType() == CH_SYS_WIN95)
	{
		ChString		strPath;
		ChString		strFilePath;
		ChString		strGroupName;
		ChString		strName;
		ChString		strFormat;
		ChString		strData;

		pWizard->GetData( strGroupName, strName );

		LOADSTRING( IDS_SHORTCUT_FINISH_WIN95, strFormat );
		strData.Format( strFormat, (const char*)strName,
									(const char*)strGroupName,
									(const char*)strGroupName );

		m_staticFinishSysMsg.SetWindowText( strData );
	}
	else
	{
		ChString		strPath;
		ChString		strFilePath;
		ChString		strGroupName;
		ChString		strName;
		ChString		strFormat;
		ChString		strData;

		pWizard->GetData( strGroupName, strName );

		LOADSTRING( IDS_SHORTCUT_FINISH_PROGMAN, strFormat );
		strData.Format( strFormat, (const char*)strName,
									(const char*)strGroupName );

		m_staticFinishSysMsg.SetWindowText( strData );
	}

	return boolSetFocus;
}


void ChShortcutWizFinish::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChShortcutWizFinish)
	DDX_Control(pDX, IDC_SHORTCUT_MSG_SYSTEM, m_staticFinishSysMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChShortcutWizFinish, ChWizardPage )
	//{{AFX_MSG_MAP(ChShortcutWizFinish)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChShortcutWizFinish message handlers
----------------------------------------------------------------------------*/


#endif	// defined( CH_MSW )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:12  uecasm
// Import of source tree as at version 2.53 release.
//
