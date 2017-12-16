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

	This file consists of implementation of the ChOutOfDateDlg class,
	which displays a dialog indicating that the client is out-of-date.

----------------------------------------------------------------------------*/

#include "headers.h"

#include "Pueblo.h"
#include "ChOODDlg.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChOutOfDateDlg class
----------------------------------------------------------------------------*/

ChOutOfDateDlg::ChOutOfDateDlg( bool boolCritical,
								const ChString& strDescription, CWnd* pParent ) :
					CDialog( ChOutOfDateDlg::IDD, pParent ),
					m_boolCritical( boolCritical ),
					m_strDesc( strDescription )
{
	//{{AFX_DATA_INIT(ChOutOfDateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ChOutOfDateDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChOutOfDateDlg)
	DDX_Control(pDX, IDC_UPDATE_LATER, m_btnUpdateLater);
	DDX_Control(pDX, IDC_EDIT_REASON, m_editReason);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChOutOfDateDlg, CDialog )
	//{{AFX_MSG_MAP(ChOutOfDateDlg)
	ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
	ON_BN_CLICKED(IDC_UPDATE_LATER, OnUpdateLater)
	ON_BN_CLICKED(IDC_UPDATE_NOW, OnUpdateNow)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChOutOfDateDlg message handlers
----------------------------------------------------------------------------*/

BOOL ChOutOfDateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_editReason.SetWindowText( m_strDesc );

	if (m_boolCritical)
	{
		m_btnUpdateLater.EnableWindow( false );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void ChOutOfDateDlg::OnUpdateNow() 
{
	EndDialog( IDC_UPDATE_NOW );
}


void ChOutOfDateDlg::OnUpdateLater() 
{
	EndDialog( IDC_UPDATE_LATER );
}


void ChOutOfDateDlg::OnShutdown() 
{
	EndDialog( IDC_SHUTDOWN );
}

void ChOutOfDateDlg::OnHelp() 
{											/* This method gets the help
												button press and passes it on
												to the currently active
												page to process */
	WinHelp( 0x20000 + IDD_CLIENT_OUTDATED );
}

