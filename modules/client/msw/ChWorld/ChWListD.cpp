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

	Implementation for the ChWorldListDlg class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChCore.h>
#include <ChUtil.h>

#include "ChWListD.h"


#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChWorldListDlg class
----------------------------------------------------------------------------*/

ChWorldListDlg::ChWorldListDlg( ChCore* pCore, bool boolConnected, CWnd* pParent ) :
				ChDialog( 
#if defined( CH_PUEBLO_PLUGIN )
					(chparam)AfxGetInstanceHandle(), 
#else
					(chparam)ChWorldDLL.hModule, 
#endif
					ChWorldListDlg::IDD,
						pParent ),
				m_pCore( pCore ),
				m_boolConnected( boolConnected )
{
	//{{AFX_DATA_INIT(ChWorldListDlg)
	//}}AFX_DATA_INIT
}


void ChWorldListDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChWorldListDlg)
	DDX_Control(pDX, IDC_LIST_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_LIST_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_LIST_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_LIST_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_WORLD_LIST, m_listWorlds);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChWorldListDlg, CDialog )
	//{{AFX_MSG_MAP( ChWorldListDlg )
	ON_BN_CLICKED(IDC_LIST_ADD, OnListAdd)
	ON_LBN_SELCHANGE(IDC_WORLD_LIST, OnSelchangeWorldList)
	ON_BN_CLICKED(IDC_LIST_DELETE, OnListDelete)
	ON_BN_CLICKED(IDC_LIST_CONNECT, OnListConnect)
	ON_BN_CLICKED(IDC_LIST_EDIT, OnListEdit)
	ON_BN_CLICKED(IDC_LIST_CREATE_SHORTCUT, OnListCreateShortcut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWorldListDlg protected methods
----------------------------------------------------------------------------*/
 
void ChWorldListDlg::Add( bool boolChange, const ChString& strName,
							const ChString& strDesc, const ChString& strHost,
							chint16 sPort, const ChWorldType& type,
							ChLoginType login, const ChString& strUsername,
							const ChString& strPassword,
							const ChString& strHomePage )
{
	if (!boolChange)
	{										// Add name to list box
		int		iIndex;

		iIndex = AddNameToList( strName, strUsername );

		if (LB_ERR != iIndex)
		{
			m_listWorlds.SetCurSel( iIndex );
			UpdateButtons();
		}
	}
											/* Add or change the item in the
												data structure */

	m_worldList.Add( strName, strDesc, strHost, sPort, type, login,
						strUsername, strPassword, strHomePage );
}


void ChWorldListDlg::UpdateButtons()
{
	bool		boolEnable;

	if (m_listWorlds.GetCurSel() != LB_ERR)
	{
		boolEnable = true;
	}
	else
	{
		boolEnable = false;
	}

	m_btnDelete.EnableWindow( boolEnable );
	m_btnEdit.EnableWindow( boolEnable );
											/* Only enable the connect button
												if we're not already
												connected */

	m_btnConnect.EnableWindow( boolEnable && !m_boolConnected );
}


void ChWorldListDlg::InstallWorldList()
{
	ChPosition	pos;

	pos = m_worldList.GetHead();

	while( pos )
	{
		ChWorldInfo*	pInfo;

		pInfo = m_worldList.GetData( pos );
		AddNameToList( pInfo->GetName(), pInfo->GetUsername() );
		m_worldList.GetNext( pos );
	}

	if (m_listWorlds.GetCount() > 0)
	{
		m_listWorlds.SetCurSel( 0 );
		OnSelchangeWorldList();
	}

	UpdateButtons();
}


void ChWorldListDlg::CreateShortcut()
{
	ChWorldInfo		info( GetName(), GetDesc(), GetHost(), GetPort(),
							GetType(), GetLoginType(), GetUsername(),
							GetPassword(), GetHomePage() );

	info.CreateShortcut( m_pCore );
}


int ChWorldListDlg::AddNameToList( const ChString& strName,
									const ChString& strUsername )
{
	ChString		strEntry( strName );

	if (!strUsername.IsEmpty())
	{
		strEntry += " ";
		strEntry += WORLD_NAME_SEPARATOR;
		strEntry += " " + strUsername;
	}

	return m_listWorlds.AddString( strEntry );
}


void ChWorldListDlg::ExtractName( const ChString& strListEntry, ChString& strName,
									ChString& strUsername )
{
	int		iLoc;

	if (-1 != (iLoc = strListEntry.Find( WORLD_NAME_SEPARATOR )))
	{
		strName = strListEntry.Left( iLoc - 1 );
		strUsername = strListEntry.Mid( iLoc + 2 );
	}
	else
	{
		strName = strListEntry;
		strUsername = "";
	}
}


/*----------------------------------------------------------------------------
	SyncDataFields

	This method will take the data for the currently selected item in
	the list and store this data in the dialog's data fields.
----------------------------------------------------------------------------*/

bool ChWorldListDlg::SyncDataFields()
{
	ChWorldInfo*	pInfo = GetCurrInfo();
	bool			boolSuccess;

	if (0 != pInfo)
	{
		m_strName = pInfo->GetName();
		m_strDesc = pInfo->GetDesc();
		m_strHost = pInfo->GetHost();
		m_strAddr = pInfo->GetAddr();
		m_sPort = pInfo->GetPort();
		m_type = pInfo->GetType();
		m_loginType = pInfo->GetLoginType();
		m_strUsername = pInfo->GetUsername();
		m_strPassword = pInfo->GetPassword();
		m_strHomePage = pInfo->GetHomePage();

		boolSuccess = true;
	}
	else
	{
		boolSuccess = false;
	}

	return boolSuccess;
}


ChWorldInfo* ChWorldListDlg::GetCurrInfo()
{
	ChWorldInfo	*pInfo;
	int			iSel;

	if (LB_ERR == (iSel = m_listWorlds.GetCurSel()))
	{
		pInfo = 0;
	}
	else
	{
		ChString		strItem;
		ChString		strName;
		ChString		strUsername;

		m_listWorlds.GetText( iSel, strItem );
		ExtractName( strItem, strName, strUsername );

		pInfo = m_worldList.FindName( strName, strUsername );
	}

	return pInfo;
}


void ChWorldListDlg::Delete( int iIndex )
{											/* Deletes an indexed item from
												the list */
	ChString		strEntry;
	ChString		strName;
	ChString		strUsername;
	int			iCount;

	m_listWorlds.GetText( iIndex, strEntry );
	ExtractName( strEntry, strName, strUsername );
	m_worldList.Remove( strName, strUsername );
	m_listWorlds.DeleteString( iIndex );

	iCount = m_listWorlds.GetCount();
	if (iCount > 0)
	{
		if (iIndex >= iCount)
		{
			iIndex = iCount - 1;
		}

		m_listWorlds.SetCurSel( iIndex );
	}
}


/*----------------------------------------------------------------------------
	ChWorldListDlg message handlers
----------------------------------------------------------------------------*/

BOOL ChWorldListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

	InstallWorldList();
	UpdateButtons();

	if (m_listWorlds.GetCount() > 0 &&
		LB_ERR != m_listWorlds.GetCurSel())
	{
		m_listWorlds.SetFocus();
		m_listWorlds.SetCurSel( 0 );
	}
	else
	{
		m_btnAdd.SetFocus();
	}

	return false;
}


void ChWorldListDlg::OnOK() 
{
	CDialog::OnOK();

	m_worldList.Store();
}


void ChWorldListDlg::OnListCreateShortcut() 
{
	SyncDataFields();
	CreateShortcut();
}


void ChWorldListDlg::OnListConnect() 
{
	SyncDataFields();

	m_worldList.Store();
	EndDialog( IDC_LIST_CONNECT );
}


void ChWorldListDlg::OnListDelete() 
{
	int			iSel;

	if (LB_ERR != (iSel = m_listWorlds.GetCurSel()))
	{
		Delete( iSel );
	}
	
	UpdateButtons();
}


void ChWorldListDlg::OnListAdd()
{
	ChWorldListEdit		editDlg;
	int					iResult;

	iResult = editDlg.DoModal();

	if (IDOK == iResult)
	{
		ASSERT( variableLogin != editDlg.GetLoginType() );

		Add( false, editDlg.GetName(), editDlg.GetDesc(), editDlg.GetHost(),
				editDlg.GetPort(), editDlg.GetType(), editDlg.GetLoginType(),
				editDlg.GetUsername(), editDlg.GetPassword(),
				editDlg.GetHomePage() );
	}
	
	UpdateButtons();
}


void ChWorldListDlg::OnListEdit() 
{
	if (SyncDataFields())
	{
		ChString		strOldName = GetName();
		ChString		strOldUsername = GetUsername();

		ChWorldListEdit		editDlg( strOldName, GetDesc(), GetHost(),
										GetPort(), GetType(), GetLoginType(),
										GetUsername(), GetPassword(),
										GetHomePage() );
		int					iResult;

		iResult = editDlg.DoModal();

		if (IDOK == iResult)
		{
			ChString	strNewName( editDlg.GetName() );
			ChString	strNewUsername( editDlg.GetUsername() );

			ASSERT( variableLogin != editDlg.GetLoginType() );

			if ((strOldName == strNewName) &&
				(strOldUsername == strNewUsername))
			{
				Add( true, strOldName, editDlg.GetDesc(), editDlg.GetHost(),
						editDlg.GetPort(), editDlg.GetType(),
						editDlg.GetLoginType(), editDlg.GetUsername(),
						editDlg.GetPassword(), editDlg.GetHomePage() );
			}
			else
			{
				int		iIndex;
											// Delete the old item

				if (LB_ERR != (iIndex = m_listWorlds.GetCurSel()))
				{
					Delete( iIndex );
				}
											// Add a new item (new name)

				Add( false, strNewName, editDlg.GetDesc(),
						editDlg.GetHost(), editDlg.GetPort(),
						editDlg.GetType(), editDlg.GetLoginType(),
						editDlg.GetUsername(), editDlg.GetPassword(),
						editDlg.GetHomePage() );
			}
		}
	}

	UpdateButtons();
}


void ChWorldListDlg::OnSelchangeWorldList()
{
	if (SyncDataFields())
	{
		UpdateData( false );
	}
}


/*----------------------------------------------------------------------------
	ChWorldListEdit class
----------------------------------------------------------------------------*/

ChWorldListEdit::ChWorldListEdit( CWnd* pParent ) :
					CDialog( ChWorldListEdit::IDD, pParent ),
					m_boolNew( true ),
					m_type( otherType ),
					m_loginType( unamePwLogin )
{
	//{{AFX_DATA_INIT(ChWorldListEdit)
	m_strName = _T("");
	m_strType = _T("");
	m_strDesc = _T("");
	m_strHost = _T("");
	m_sPort = 0;
	m_strUsername = _T("");
	m_strPassword = _T("");
	m_iLoginStyle = 0;
	m_strHomePage = _T("");
	//}}AFX_DATA_INIT
}


ChWorldListEdit::ChWorldListEdit( const ChString& strName, const ChString& strDesc,
									const ChString& strHost, chint16 sPort,
									ChWorldType type, ChLoginType loginType,
									const ChString& strUsername,
									const ChString& strPassword,
									const ChString& strHomePage, CWnd* pParent ) :
					CDialog( ChWorldListEdit::IDD, pParent ),
					m_boolNew( false ),
					m_strName( strName ),
					m_strDesc( strDesc ),
					m_strHost( strHost ),
					m_sPort( sPort ),
					m_type( type ),
					m_loginType( loginType ),
					m_strUsername( strUsername ),
					m_strPassword( strPassword ),
					m_strHomePage( strHomePage )
{
}


void ChWorldListEdit::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChWorldListEdit)
	DDX_Control(pDX, IDC_LIST_WEB_PAGE, m_editHomePage);
	DDX_Control(pDX, IDC_RADIO_LOGIN_MUD, m_radioMudLogin);
	DDX_Control(pDX, IDC_RADIO_LOGIN_CONNECT, m_radioConnectLogin);
	DDX_Control(pDX, IDC_WORLD_LIST_TYPE, m_comboTypes);
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_staticMessage);
	DDX_Control(pDX, IDC_LIST_USERNAME, m_editUsername);
	DDX_Control(pDX, IDC_LIST_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDOK, m_btnOkay);
	DDX_Control(pDX, IDC_LIST_PORT, m_editPort);
	DDX_Control(pDX, IDC_LIST_NAME, m_editName);
	DDX_Control(pDX, IDC_LIST_HOST, m_editHost);
	DDX_Control(pDX, IDC_LIST_DESC, m_editDesc);
	DDX_Text(pDX, IDC_LIST_NAME, m_strName);
	DDX_CBString(pDX, IDC_WORLD_LIST_TYPE, m_strType);
	DDX_Text(pDX, IDC_LIST_DESC, m_strDesc);
	DDX_Text(pDX, IDC_LIST_HOST, m_strHost);
	DDX_Text(pDX, IDC_LIST_PORT, m_sPort);
	DDX_Text(pDX, IDC_LIST_USERNAME, m_strUsername);
	DDX_Text(pDX, IDC_LIST_PASSWORD, m_strPassword);
	DDX_Radio(pDX, IDC_RADIO_LOGIN_MUD, m_iLoginStyle);
	DDX_Text(pDX, IDC_LIST_WEB_PAGE, m_strHomePage);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		if (m_strType.GetLength())
		{
			m_type.Set( m_strType );
		}

		if (0 == m_iLoginStyle)
		{
			m_loginType = unamePwLogin;
		}
		else
		{
			m_loginType = connectLogin;
		}
	}
	else
	{
		switch( m_loginType )
		{
			case connectLogin:
			{
				m_iLoginStyle = 1;
				break;
			}

			case unamePwLogin:
			default:
			{
				m_iLoginStyle = 0;
				break;
			}
		}

		if (m_type == undefinedType)
		{
			m_comboTypes.SetCurSel( -1 );
		}
		else
		{
			m_comboTypes.SelectString( -1, m_type.GetName() );
		}
	}
}


void ChWorldListEdit::UpdateButtons()
{
	bool		boolEnable = false;
	bool		boolEnableLoginType = true;

	UpdateData();

	if (m_strName.GetLength() && m_strHost.GetLength() &&
			m_sPort > 0)
	{
		if (m_comboTypes.GetCurSel() != LB_ERR)
		{
			boolEnable = true;
		}
	}

	m_btnOkay.EnableWindow( boolEnable );

	if ("" == m_strType)
	{
		m_iLoginStyle = 0;
		boolEnableLoginType = false;
	}
	else
	{
		switch( m_type.GetLoginType() )
		{
			case variableLogin:
			{
				boolEnableLoginType = true;
				break;
			}

			case unamePwLogin:
			{
				m_iLoginStyle = 0;
				boolEnableLoginType = false;
				break;
			}

			case connectLogin:
			{
				m_iLoginStyle = 1;
				boolEnableLoginType = false;
				break;
			}
		}
	}

	UpdateData( false );

	m_radioMudLogin.EnableWindow( boolEnableLoginType );
	m_radioConnectLogin.EnableWindow( boolEnableLoginType );
}


void ChWorldListEdit::UpdateWarningMessage()
{
	ChString		strOldMessage;
	ChString		strMessage;
	int			iUsernameLen;
	int			iPasswordLen;

	UpdateData();

	iUsernameLen = m_strUsername.GetLength();
	iPasswordLen = m_strPassword.GetLength();

	if (iUsernameLen || iPasswordLen)
	{
		if (!GetType().IsValidType())
		{
			LOADSTRING( IDS_WARNING_WORLD_TYPE_MISSING, strMessage );
		}
		else
		{
			if (0 == iPasswordLen)
			{
				if (connectLogin == GetLoginType())
				{
					LOADSTRING( IDS_WARNING_MUSH_TYPE, strMessage );
				}
				else
				{
					LOADSTRING( IDS_WARNING_NO_PASSWORD, strMessage );
				}
			}
		}
	}

	m_staticMessage.GetWindowText( strOldMessage );
	if (strOldMessage != strMessage)
	{
		m_staticMessage.SetWindowText( strMessage );
	}
}


void ChWorldListEdit::RemoveIllegalChars( CEdit& edit )
{
	int		iStart;
	int		iEnd;
	ChString	strText;
	int		iLoc;
	bool	boolChanged = false;

	edit.GetSel( iStart, iEnd );
	edit.GetWindowText( strText );

	while (-1 != (iLoc = strText.Find( WORLD_NAME_SEPARATOR )))
	{
		strText = strText.Left( iLoc ) + strText.Mid( iLoc + 1 );
		iStart--;
		boolChanged = true;
	}

	if (boolChanged)
	{
		edit.SetWindowText( strText );
		edit.SetSel( iStart, iStart );
	}
}


BEGIN_MESSAGE_MAP( ChWorldListEdit, CDialog )
	//{{AFX_MSG_MAP(ChWorldListEdit)
	ON_EN_CHANGE(IDC_LIST_NAME, OnChangeListName)
	ON_EN_CHANGE(IDC_LIST_HOST, OnChangeListHost)
	ON_EN_CHANGE(IDC_LIST_PORT, OnChangeListPort)
	ON_CBN_SELCHANGE(IDC_WORLD_LIST_TYPE, OnSelchangeListType)
	ON_EN_CHANGE(IDC_LIST_PASSWORD, OnChangeListPassword)
	ON_EN_CHANGE(IDC_LIST_USERNAME, OnChangeListUsername)
	ON_BN_CLICKED(IDC_RADIO_LOGIN_CONNECT, OnRadioLoginConnect)
	ON_BN_CLICKED(IDC_RADIO_LOGIN_MUD, OnRadioLoginMud)
	ON_EN_UPDATE(IDC_LIST_NAME, OnUpdateListName)
	ON_EN_UPDATE(IDC_LIST_USERNAME, OnUpdateListUsername)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWorldListEdit message handlers
----------------------------------------------------------------------------*/

BOOL ChWorldListEdit::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

	m_editName.LimitText( 30 );
	m_editHost.LimitText( 255 );
	m_editPort.LimitText( 5 );
	m_editHomePage.LimitText( 1024 );
	m_editDesc.LimitText( 1024 );
	m_editUsername.LimitText( 30 );
	m_editPassword.LimitText( 30 );

	ChWorldType::FillTypeList( &m_comboTypes );

	if (IsNew())
	{
		m_editName.SetFocus();
	}
	else
	{
		m_editHost.SetFocus();
		m_editHost.SetSel( 0, -1 );
	}
											/* Update data again since we've
												added the entries to the Types
												combo box */
	UpdateData( false );
	UpdateButtons();

	return false;
}


void ChWorldListEdit::OnUpdateListName() 
{
	RemoveIllegalChars( m_editName );
}


void ChWorldListEdit::OnChangeListName() 
{
	UpdateButtons();
}

void ChWorldListEdit::OnChangeListHost() 
{
	UpdateButtons();
}

void ChWorldListEdit::OnChangeListPort() 
{
	UpdateButtons();
}

void ChWorldListEdit::OnSelchangeListType() 
{
	UpdateButtons();
	UpdateWarningMessage();
}


void ChWorldListEdit::OnUpdateListUsername() 
{
	RemoveIllegalChars( m_editUsername );
}


void ChWorldListEdit::OnChangeListUsername() 
{
	UpdateWarningMessage();
}

void ChWorldListEdit::OnChangeListPassword() 
{
	UpdateWarningMessage();
}


void ChWorldListEdit::OnRadioLoginConnect() 
{
	UpdateWarningMessage();
}


void ChWorldListEdit::OnRadioLoginMud() 
{
	UpdateWarningMessage();
}

// $Log$
