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

	This file contains the implementation of the ChPrefsCachePage class and
	ChPrefsNetworkPage class, which handle preferences for the main cache
	and the network.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChReg.h>
#include <ChUtil.h>
#include <ChHtpCon.h>

#include "Pueblo.h"
#include "ChPrNet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//CH_EXTERN_VAR ChClientCore	*pClientCore;


/*----------------------------------------------------------------------------
	ChPrefsCachePage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsCachePage, ChPropertyPage )

ChPrefsCachePage::ChPrefsCachePage() :
					ChPropertyPage( ChPrefsCachePage::IDD, 0, hInstApp ),
					m_reg( CH_CACHE_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsCachePage)
	m_cacheDir = _T("");
	m_uCacheSize = 1000;
	//}}AFX_DATA_INIT
}

ChPrefsCachePage::~ChPrefsCachePage()
{
}

void ChPrefsCachePage::DoDataExchange(CDataExchange* pDX)
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChPrefsCachePage)
	DDX_Text(pDX, IDC_CACHE_DIR, m_cacheDir);
	DDV_MaxChars(pDX, m_cacheDir, 256);
	DDX_Text(pDX, IDC_CACHE_SIZE, m_uCacheSize);
	DDV_MinMaxUInt(pDX, m_uCacheSize, 10, 10000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ChPrefsCachePage, ChPropertyPage)
	//{{AFX_MSG_MAP(ChPrefsCachePage)
	ON_BN_CLICKED(IDC_CLR_CACHE, OnClrCache)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#define CACHE_DIR		"urlcache"


BOOL ChPrefsCachePage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		ChString		strDir;
		chuint32	luSize;
		chuint32	luOption;

		#if defined( CH_MSW )
		{
			CWinApp*	pApp = AfxGetApp();
			char		strPath[MAX_PATH];
			int			iLastSeparator;

			::GetModuleFileName( pApp->m_hInstance, strPath, MAX_PATH );
			strDir = strPath;
													// Strip out the app name

			iLastSeparator = strDir.ReverseFind( TEXT( '\\' ) );

			if (iLastSeparator >= 0)
			{
				strDir = strDir.Left( iLastSeparator + 1 );
			}
		}
		#else	// defined( CH_MSW )
		{
			extern ChString strPuebloDirectory;
			strDir = strPuebloDirectory;
		}
		#endif	// defined( CH_MSW )

		m_reg.Read( CH_CACHE_DIR, m_cacheDir, strDir );

		int		iIndex =  m_cacheDir.Find( CACHE_DIR );

		if ( iIndex && iIndex != -1 )
		{  // remove what we added to the path
			m_cacheDir = m_cacheDir.Left( iIndex - 1  );
		}

 		m_reg.Read( CH_CACHE_SIZE, luSize, CH_CACHE_SIZE_DEF );
		m_uCacheSize = (UINT)luSize;

		//CButton * pBtn = (CButton*)GetDlgItem( IDC_CLR_CACHE );
		//pBtn->EnableWindow( false );

 		m_reg.Read( CH_CACHE_OPTION, luOption, CH_CACHE_OPTION_DEF );

		if (luOption & CH_CACHE_VERIFY_PER_SESSION)
		{
			CheckDlgButton( IDC_VRFY_SESSION, true );
		}
		else if (luOption & CH_CACHE_VERIFY_EVERYTIME)
		{
			CheckDlgButton( IDC_VRFY_EVERYTIME, true );
		}
		else
		{
			CheckDlgButton( IDC_VRFY_NEVER, true );
		}
											/* Set the initialized flag so
												that we don't do this again */
		UpdateData( false ); 			// initialize the controls
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsCachePage::OnCommit()
{
	if (m_boolInitialized)
	{
		chuint32	luOption;

		if ( IsDlgButtonChecked( IDC_VRFY_SESSION ) )
		{
			luOption = CH_CACHE_VERIFY_PER_SESSION;
		}
		else if ( IsDlgButtonChecked( IDC_VRFY_EVERYTIME ) )
		{
			luOption = CH_CACHE_VERIFY_EVERYTIME;
		}
		else
		{
			luOption = CH_CACHE_VERIFY_NEVER;
		}

 		m_reg.Write( CH_CACHE_OPTION, luOption );

											// validate the directory
		if (m_cacheDir.IsEmpty())
		{
			ChUtil::GetAppDirectory( m_cacheDir );
			m_cacheDir += CACHE_DIR;
		}

		if (m_cacheDir[m_cacheDir.GetLength() - 1] != TEXT( '\\' ) )
		{
			m_cacheDir += TEXT( "\\" );
		}
		m_cacheDir += CACHE_DIR;

		if (ChHTTPSocketConn::CreateAndValidateCacheDir( m_cacheDir ))
		{
			m_reg.Write( CH_CACHE_DIR, m_cacheDir );
		}
		else
		{
			ChString strMessage;

			LOADSTRING( IDS_INVALID_CACHE_DIR, strMessage );
			char * pMsg = new char[ m_cacheDir.GetLength() + strMessage.GetLength() + 2 ];
			wsprintf( pMsg, strMessage, m_cacheDir );
			AfxMessageBox( pMsg );
			delete []pMsg;
			return;
		}

 		m_reg.Write( CH_CACHE_SIZE, (chuint32)m_uCacheSize );
	}
}


/*----------------------------------------------------------------------------
	ChPrefsCachePage message handlers
----------------------------------------------------------------------------*/

void ChPrefsCachePage::OnClrCache()
{
	ChHTTPSocketConn::ClearCache();
}


/*----------------------------------------------------------------------------
	ChPrefsNetworkPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsNetworkPage, ChPropertyPage )

ChPrefsNetworkPage::ChPrefsNetworkPage() : ChPropertyPage(ChPrefsNetworkPage::IDD),
					m_reg( CH_NETWORK_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsNetworkPage)
	m_uMaxConn = 1;
	//}}AFX_DATA_INIT
}


ChPrefsNetworkPage::~ChPrefsNetworkPage()
{
}


void ChPrefsNetworkPage::DoDataExchange(CDataExchange* pDX)
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChPrefsNetworkPage)
	DDX_Text(pDX, IDC_MAX_CONN, m_uMaxConn);
	DDV_MinMaxUInt(pDX, m_uMaxConn, 1, 10);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChPrefsNetworkPage, ChPropertyPage )
	//{{AFX_MSG_MAP(ChPrefsNetworkPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL ChPrefsNetworkPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		ChString		strSize;

 		m_reg.Read( CH_MAX_CONNECTIONS, strSize,
 						CH_MAX_CONNECTIONS_DEF );

		m_uMaxConn = atoi( strSize );
											/* Set the initialized flag so
												that we don't do this again */
		UpdateData( false ); 			// initialize the controls
		m_boolInitialized = true;
	}


	return boolResult;
}


void ChPrefsNetworkPage::OnCommit()
{
	if (m_boolInitialized)
	{
		if (m_uMaxConn)
		{
 			m_reg.Write( CH_MAX_CONNECTIONS, (chuint32)m_uMaxConn );
		}
	}
}


/*----------------------------------------------------------------------------
	ChPrefsNetworkPage message handlers
----------------------------------------------------------------------------*/

// $Log$
