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
#include "World.h"
#include "ChConnectDlg.h"


#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChConnectingDlg class
----------------------------------------------------------------------------*/

ChConnectingDlg::ChConnectingDlg( ChWorldMainInfo* pMainInfo, CWnd* pParent ) :
					ChDialog( (chparam)
					#if defined( CH_PUEBLO_PLUGIN )
						AfxGetInstanceHandle(),
					#else
						ChWorldDLL.hModule, 
					#endif
								ChConnectingDlg::IDD, pParent ),
					m_pMainInfo( pMainInfo )
{
	//{{AFX_DATA_INIT(ChConnectingDlg)
	m_strConnectMsg = _T("");
	//}}AFX_DATA_INIT
}


void ChConnectingDlg::DoDataExchange( CDataExchange* pDX )
{
	ChDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChConnectingDlg)
	DDX_Text(pDX, IDC_CONNECT_MESSAGE, m_strConnectMsg);
	//}}AFX_DATA_MAP
}


BOOL ChConnectingDlg::Create( const ChString& strMessage, CWnd* pParentWnd )
{
	m_strConnectMsg = strMessage;

	return ChDialog::Create( IDD, pParentWnd );
}


void ChConnectingDlg::ShowAfterASec()
{
											// Set a 1-second timer...
	SetTimer( timerShowID, 1000, 0 );
}


void ChConnectingDlg::ChangeMessage( const ChString& strMessage )
{
											// Kill the old timer...
	KillTimer( timerShowID );

	m_strConnectMsg = strMessage;

	UpdateData( false );
	UpdateWindow();

	if (!IsWindowVisible())
	{
											// Do a new deferred show.
		ShowAfterASec();
	}
}


BEGIN_MESSAGE_MAP( ChConnectingDlg, ChDialog )
	//{{AFX_MSG_MAP(ChConnectingDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChConnectingDlg message handlers
----------------------------------------------------------------------------*/

void ChConnectingDlg::OnCancel()
{
	KillTimer( timerShowID );
	GetMainInfo()->OnConnectComplete( WSAEDISCON );
}

void ChConnectingDlg::OnTimer( UINT nIDEvent )
{
	if (timerShowID == nIDEvent)
	{
		ShowWindow( SW_SHOW );
		KillTimer( timerShowID );
	}
	else
	{
		ChDialog::OnTimer( nIDEvent );
	}
}

// $Log$
