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

	Contains the interface for the ChQuickConnect class, which is
	a dialog allowing users to quickly connect to a world.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include "ChWorld.h"
#include "ChQuickConnect.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChQuickConnect class
----------------------------------------------------------------------------*/

ChQuickConnect::ChQuickConnect( CWnd* pParent ) :
				ChDialog( 
		#if defined( CH_PUEBLO_PLUGIN )
							(chparam)AfxGetInstanceHandle(), 
		#else
							(chparam)ChWorldDLL.hModule, 
		#endif
					ChQuickConnect::IDD,
							pParent ),
				m_loginType( unamePwLogin )
{
	//{{AFX_DATA_INIT(ChQuickConnect)
	m_iLoginStyle = 0;
	m_strHost = _T("");
	m_strPort = _T("");
	//}}AFX_DATA_INIT
}


void ChQuickConnect::DoDataExchange( CDataExchange* pDX )
{
	ChDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChQuickConnect)
	DDX_Control(pDX, IDOK, m_btnConnect);
	DDX_Control(pDX, IDC_LIST_PORT, m_editPort);
	DDX_Control(pDX, IDC_LIST_HOST, m_editHost);
	DDX_Radio(pDX, IDC_RADIO_LOGIN_MUD, m_iLoginStyle);
	DDX_Text(pDX, IDC_LIST_HOST, m_strHost);
	DDX_Text(pDX, IDC_LIST_PORT, m_strPort);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		int		iLoc;

		if (0 == m_iLoginStyle)
		{
			m_loginType = unamePwLogin;
		}
		else
		{
			m_loginType = connectLogin;
		}

		m_sPort = 0;
		if (!m_strPort.IsEmpty())
		{
			m_sPort = atoi( m_strPort );
		}

		if (m_strPort.IsEmpty() || (0 == m_sPort))
		{
			int		iScanned;
			int		iPort;
											/* Attempt to get the port number
												from the host edit field */

			iScanned = sscanf( (const char*)m_strHost,
								"%*[a-zA-z0-9\x2d_.]%*[ \t,:;]%d", &iPort );
			if (1 == iScanned)
			{
				m_sPort = (chint16)iPort;
			}
		}
											/* Now truncate the host name at
												the first illegal character */

		if (-1 != (iLoc = m_strHost.FindOneOf( " \t,:;" )))
		{
			m_strHost = m_strHost.Left( iLoc );
		}
	}
}


BEGIN_MESSAGE_MAP( ChQuickConnect, ChDialog )
	//{{AFX_MSG_MAP(ChQuickConnect)
	ON_EN_CHANGE(IDC_LIST_HOST, OnChangeListHost)
	ON_EN_CHANGE(IDC_LIST_PORT, OnChangeListPort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChQuickConnect protected methods
----------------------------------------------------------------------------*/

void ChQuickConnect::UpdateButtons()
{
	bool		boolEnable = false;

	UpdateData();

	if (m_strHost.GetLength() && m_strHost.GetLength() &&
		(GetPort() > 0))
	{
		boolEnable = true;
	}

	m_btnConnect.EnableWindow( boolEnable );
}


/*----------------------------------------------------------------------------
	ChQuickConnect message handlers
----------------------------------------------------------------------------*/

BOOL ChQuickConnect::OnInitDialog()
{
	ChDialog::OnInitDialog();

	m_editHost.LimitText( 255 );
	m_editPort.LimitText( 5 );

	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ChQuickConnect::OnChangeListHost()
{
	UpdateButtons();
}

void ChQuickConnect::OnChangeListPort()
{
	UpdateButtons();
}

// $Log$
