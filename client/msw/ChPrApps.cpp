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

		 Ultra Enterprises:  Gavin Lambert
		 
		 			Edited quite a bit, since MFC 4.2 doesn't like hook procs.

------------------------------------------------------------------------------

	This file contains the implementation of the ChPrefsApps class,
	which manages helper application preferences.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <cderr.h>

#include <ChReg.h>
#include <ChUtil.h>
#include "pueblo.h"
#include "ChPrApps.h"
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define OPEN_DLG_FLAGS		(OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |\
								OFN_HIDEREADONLY)


/*----------------------------------------------------------------------------
	ChPrefsApps class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsApps, ChPropertyPage )


ChPrefsApps::ChPrefsApps() :
				ChPropertyPage( ChPrefsApps::IDD, 0, hInstApp ),
				m_reg( CH_APPS_GROUP ),
				m_boolInitialized( false ),
				m_boolInternal( false ),
				m_strBrowser(_T(""))
				{
	//{{AFX_DATA_INIT(ChPrefsApps)
	m_iRadioBrowsers = -1;
	//}}AFX_DATA_INIT
}


ChPrefsApps::~ChPrefsApps()
{
}


void ChPrefsApps::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	if (!pDX->m_bSaveAndValidate)
	{
		m_iRadioBrowsers = m_boolUseDefaultBrowser ? 0 : 1;
	}

	//{{AFX_DATA_MAP(ChPrefsApps)
	DDX_Control(pDX, IDC_BROWSE, m_buttonBrowse);
	DDX_Control(pDX, IDC_STATIC_BROWSER, m_staticBrowser);
	DDX_Radio(pDX, IDC_USEDEFAULTBROWSER, m_iRadioBrowsers);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_boolUseDefaultBrowser = (0 == m_iRadioBrowsers);
	}
}


BOOL ChPrefsApps::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		ReadRegistry();
		UpdateData( FALSE );
		UpdateButtons();
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsApps::OnCommit()
{
	if (m_boolInitialized)
	{
		m_reg.WriteBool( CH_APP_DEFAULTBROWSER, m_boolUseDefaultBrowser );
		m_reg.WriteBool( CH_APP_WEBTRACKER, m_boolInternal );
		m_reg.Write( CH_APP_WEBBROWSER, m_strBrowser );
	}     
}


BEGIN_MESSAGE_MAP( ChPrefsApps, ChPropertyPage )
	//{{AFX_MSG_MAP(ChPrefsApps)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_USEDEFAULTBROWSER, OnRadioDefaultBrowser)
	ON_BN_CLICKED(IDC_USESPECIFICBROWSER2, OnRadioSpecifiedBrowser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsApps protected methods
----------------------------------------------------------------------------*/

void ChPrefsApps::ReadRegistry()
{
	m_reg.Read( CH_APP_WEBBROWSER, m_strBrowser, CH_APP_WEBBROWSER_DEF );
	m_reg.ReadBool( CH_APP_WEBTRACKER, m_boolInternal, CH_APP_WEBTRACKER_DEF );
	m_reg.ReadBool( CH_APP_DEFAULTBROWSER, m_boolUseDefaultBrowser, m_strBrowser.IsEmpty() );

	DisplayBrowserName( m_strBrowser );
	
	if (m_strBrowser.IsEmpty())
	{
		m_boolInternal = true;	
	}
}

void ChPrefsApps::DisplayBrowserName( const ChString& strName )
{
	if (strName.GetLength())
	{
		ChString		strBrowserName( strName );
		int			iIndex = strBrowserName.ReverseFind( '\\' );

		if (-1 != iIndex)
		{
			strBrowserName = strBrowserName.Mid( iIndex + 1 );
		}

		m_staticBrowser.SetWindowText( strBrowserName );
	}
	else
	{	
		ChString		strTag;

		LOADSTRING( IDS_WEB_BROWSER_INTERNAL, strTag );
		m_staticBrowser.SetWindowText( strTag );
	}
}

void ChPrefsApps::UpdateButtons()
{
	UpdateData();

	m_buttonBrowse.EnableWindow(!m_boolUseDefaultBrowser);
	m_staticBrowser.EnableWindow(!m_boolUseDefaultBrowser);
}

/*----------------------------------------------------------------------------
	ChPrefsApps message handlers
----------------------------------------------------------------------------*/

void ChPrefsApps::OnRadioDefaultBrowser()
{
	UpdateButtons();
}

void ChPrefsApps::OnRadioSpecifiedBrowser()
{
	UpdateButtons();
}

void ChPrefsApps::OnBrowse() 
{
	ChString			strFilter;
	ChString			strTitle;
	int				iResult;

	LOADSTRING( IDS_OPEN_WEB_BROWSER_FILTER, strFilter );
	LOADSTRING( IDS_OPEN_WEB_BROWSER_TITLE, strTitle );

	//TRACE0("ChPrefsApps::OnBrowse()\n");

	ChWebBrowserSelectFileDlg	browseDlg( strTitle, m_strBrowser, strFilter,
											AfxGetMainWnd() );

	iResult = browseDlg.DoModal();
	if (IDOK == iResult)
	{										// Read the registry again
		ReadRegistry();
	}
	else if (IDC_INTERNAL == iResult)
	{
		m_boolInternal = true;
		m_strBrowser = "";

		DisplayBrowserName( m_strBrowser );
	}
}


/*----------------------------------------------------------------------------
	ChWebBrowserSelectFileDlg class
----------------------------------------------------------------------------*/

ChWebBrowserSelectFileDlg::ChWebBrowserSelectFileDlg( const ChString& strTitle,
														const ChString& strBrowser,
														const ChString& strFilter,
														CWnd* pParent ) :
		ChFileDialog( true, "exe", 0, OPEN_DLG_FLAGS, strFilter ),
		m_reg( CH_APPS_GROUP ),
		m_boolInternal( false )
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

	SetTemplate( 0, IDD_SELECT_WEB_BROWSER, IDD_SELECT_WEB_BROWSER_95 );

	//{{AFX_DATA_INIT(ChWebBrowserSelectFileDlg)
	//}}AFX_DATA_INIT
}


int ChWebBrowserSelectFileDlg::DoModal() 
{
	int		iResult;

	//TRACE0("ChWebBrowserSelectFileDlg::DoModal()\n");

	iResult = ChFileDialog::DoModal();

	//TRACE1("  (result code was %d)\n", iResult);

	if (IDOK == iResult)
	{
		m_reg.Write( CH_APP_WEBBROWSER, GetPathName( ) );
		m_boolInternal = false;
	}
	else if (IDC_INTERNAL == iResult)
	{
		// UE: In this case, we don't want to overwrite the browser, so that
		//     it will come up as the default next time.  This is only possible
		//     because we are storing whether to use the internal browser or
		//     not separately.
		//m_reg.Write( CH_APP_WEBBROWSER, "" );
		m_boolInternal = true;
	}

	m_reg.WriteBool( CH_APP_WEBTRACKER, m_boolInternal );

	return iResult;
}

void ChWebBrowserSelectFileDlg::OnInitDone() {
	ChFileDialog::OnInitDone();
	
	//TRACE0("ChWebBrowserSelectFileDlg::OnInitDone()\n");

#ifdef CH_NO_WEBTRACKER
	// The internal WebTracker has been disabled...
	GetDlgItem(IDC_INTERNAL)->EnableWindow(FALSE);
#endif
}

void ChWebBrowserSelectFileDlg::DoDataExchange( CDataExchange* pDX )
{
	ChFileDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(ChWebBrowserSelectFileDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChWebBrowserSelectFileDlg, ChFileDialog )
	//{{AFX_MSG_MAP(ChWebBrowserSelectFileDlg)
    ON_BN_CLICKED(IDC_INTERNAL, OnInternal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChWebBrowserSelectFileDlg message handlers
----------------------------------------------------------------------------*/

void ChWebBrowserSelectFileDlg::OnInternal()
{
	//TRACE0("ChWebBrowserSelectFileDlg::OnInternal()\n");

  m_boolInternal = true;
  EndDialog( IDC_INTERNAL );
}

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:52:31  uecasm
// Import of source tree as at version 2.53 release.
//