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

	This file consists of the implementation of the ChLicenseDlg class,
	used to get user agreement to the Chaco license.

----------------------------------------------------------------------------*/

#include "headers.h"
#include "Pueblo.h"

#include <ChReg.h>

#include "ChLicDlg.h"
#include "PbVersion.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChLicenseDlg class
----------------------------------------------------------------------------*/

ChLicenseDlg::ChLicenseDlg( CWnd* pParent ) :
				CDialog( ChLicenseDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(ChLicenseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ChLicenseDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChLicenseDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


int ChLicenseDlg::DoModal( const ChString& strLicenseFileName )
{
	m_strLicenseFileName = strLicenseFileName;

	return CDialog::DoModal();
}


BEGIN_MESSAGE_MAP( ChLicenseDlg, CDialog )
	//{{AFX_MSG_MAP(ChLicenseDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChLicenseDlg message handlers
----------------------------------------------------------------------------*/


void ChLicenseDlg::OnOK() 
{
	ChRegistry		appReg( CH_MISC_GROUP );
	ChClientInfo	clientInfo( ChClientInfo::thisMachine );
	ChString			strVersion;

	strVersion = clientInfo.GetClientVersion().Format();

	appReg.Write( CH_MISC_LICENSE_ACCEPTED, strVersion );

	CDialog ::OnOK();
}

BOOL ChLicenseDlg::OnInitDialog() 
{
	CRect	rtFrame;

	CDialog::OnInitDialog();

	GetDlgItem( IDC_TEXT_PLACEHOLDER )->GetWindowRect( &rtFrame );
	ScreenToClient( &rtFrame );
	rtFrame.InflateRect( -1, -1 );

	m_htmlWnd.Create( rtFrame, this, WS_VISIBLE | WS_VSCROLL );
#ifdef UE_PRERELEASE
	// The prerelease agreement is retrieved from a resource instead.
	// It's much shorter, and we can't rely on having the external
	// license file present.
	m_htmlWnd.DisplayResource( IDR_PRERELEASE_LICENSE );
#else
	m_htmlWnd.DisplayFile( m_strLicenseFileName );
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
