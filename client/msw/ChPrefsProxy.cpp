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

	This file contains the implementation of the ChPrefsProxyPage class,
	which allows the user to select font preferences.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChReg.h>
#include <ChHTTP.h>
#include <ChHtpCon.h>
#include <ChSock.h>
#include <ChUtil.h>

#include "ChClCore.h"
#include "ChPrefsProxy.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChPrefsProxyPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsProxyPage, ChPropertyPage )

ChPrefsProxyPage::ChPrefsProxyPage() :
					ChPropertyPage( ChPrefsProxyPage::IDD, 0, hInstApp ),
					m_reg( CH_PROXIES_GROUP ),
					m_boolInitialized( false ),
					m_boolUseProxies( false ),
					m_luHttpPort( 0 ),
					m_luFtpPort( 0 ),
					m_luSocksPort( 0 )
{
	//{{AFX_DATA_INIT(ChPrefsProxyPage)
	m_iRadioProxies = -1;
	m_strHttpProxy = _T("");
	m_strFtpProxy = _T("");
	m_strFtpProxyPort = _T("");
	m_strSocksProxyPort = _T("");
	m_strSocksProxy = _T("");
	m_strHttpProxyPort = _T("");
	//}}AFX_DATA_INIT

	ReadRegistry();
}


ChPrefsProxyPage::~ChPrefsProxyPage()
{
}


void ChPrefsProxyPage::DoDataExchange(CDataExchange* pDX)
{
	ChPropertyPage::DoDataExchange( pDX );

	if (!pDX->m_bSaveAndValidate)
	{
		m_iRadioProxies = m_boolUseProxies ? 1 : 0;
	}

	//{{AFX_DATA_MAP(ChPrefsProxyPage)
	DDX_Control(pDX, IDC_EDIT_SOCKS_PROXY, m_editSocksProxy);
	DDX_Control(pDX, IDC_EDIT_HTTP_PROXY, m_editHttpProxy);
	DDX_Control(pDX, IDC_EDIT_FTP_PROXY, m_editFtpProxy);
	DDX_Control(pDX, IDC_STATIC_SOCKS_PROXY, m_staticSocksProxy);
	DDX_Control(pDX, IDC_STATIC_SOCKS_PORT, m_staticSocksPort);
	DDX_Control(pDX, IDC_STATIC_HTTP_PROXY, m_staticHttpProxy);
	DDX_Control(pDX, IDC_STATIC_HTTP_PORT, m_staticHttpPort);
	DDX_Control(pDX, IDC_STATIC_FTP_PROXY, m_staticFtpProxy);
	DDX_Control(pDX, IDC_STATIC_FTP_PORT, m_staticFtpPort);
	DDX_Control(pDX, IDC_EDIT_SOCKS_PORT, m_editSocksPort);
	DDX_Control(pDX, IDC_EDIT_HTTP_PORT, m_editHttpPort);
	DDX_Control(pDX, IDC_EDIT_FTP_PORT, m_editFtpPort);
	DDX_Radio(pDX, IDC_RADIO_NO_PROXIES, m_iRadioProxies);
	DDX_Text(pDX, IDC_EDIT_HTTP_PROXY, m_strHttpProxy);
	DDX_Text(pDX, IDC_EDIT_FTP_PROXY, m_strFtpProxy);
	DDX_Text(pDX, IDC_EDIT_FTP_PORT, m_strFtpProxyPort);
	DDX_Text(pDX, IDC_EDIT_SOCKS_PORT, m_strSocksProxyPort);
	DDX_Text(pDX, IDC_EDIT_SOCKS_PROXY, m_strSocksProxy);
	DDX_Text(pDX, IDC_EDIT_HTTP_PORT, m_strHttpProxyPort);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_boolUseProxies = (0 == m_iRadioProxies)  ? false : true;

		m_luHttpPort = atoi( m_strHttpProxyPort );
		m_luFtpPort = atoi( m_strFtpProxyPort );
		m_luSocksPort = atoi( m_strSocksProxyPort );
	}
}


BOOL ChPrefsProxyPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		ReadRegistry();

		m_editHttpPort.LimitText( 5 );
		m_editFtpPort.LimitText( 5 );
		m_editSocksPort.LimitText( 5 );

		UpdateData( FALSE );
		UpdateButtons();
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsProxyPage::OnCommit()
{
	if (m_boolInitialized)
	{
		UpdateData();
		WriteRegistry();
											/* Notify HTTP and socket of the
												pref changes */

		ChChacoSocket::UpdateSocketPreferences( CH_PRODUCT_NAME );
		ChHTTPSocketConn::UpdateHTTPPreferences( CH_PRODUCT_NAME );
	}
}


BEGIN_MESSAGE_MAP(ChPrefsProxyPage, ChPropertyPage)
	//{{AFX_MSG_MAP(ChPrefsProxyPage)
	ON_EN_UPDATE(IDC_EDIT_FTP_PORT, OnUpdateEditFtpPort)
	ON_EN_UPDATE(IDC_EDIT_HTTP_PORT, OnUpdateEditHttpPort)
	ON_EN_UPDATE(IDC_EDIT_SOCKS_PORT, OnUpdateEditSocksPort)
	ON_BN_CLICKED(IDC_RADIO_NO_PROXIES, OnRadioNoProxies)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, OnRadioManual)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsProxyPage protected methods
----------------------------------------------------------------------------*/
	
void ChPrefsProxyPage::ReadRegistry()
{
	m_reg.ReadBool( CH_PROXIES, m_boolUseProxies, CH_PROXIES_DEF );

	m_reg.Read( CH_HTTP_PROXY, m_strHttpProxy, CH_HTTP_PROXY_DEF );
	m_reg.Read( CH_HTTP_PROXY_PORT, m_luHttpPort, CH_HTTP_PROXY_PORT_DEF );
	m_reg.Read( CH_FTP_PROXY, m_strFtpProxy, CH_FTP_PROXY_DEF );
	m_reg.Read( CH_FTP_PROXY_PORT, m_luFtpPort, CH_FTP_PROXY_PORT_DEF );
	m_reg.Read( CH_SOCKS_PROXY, m_strSocksProxy, CH_SOCKS_PROXY_DEF );
	m_reg.Read( CH_SOCKS_PROXY_PORT, m_luSocksPort, CH_SOCKS_PROXY_PORT_DEF );

											// Format the strings

	m_strHttpProxyPort.Format( "%lu", m_luHttpPort );
	m_strFtpProxyPort.Format( "%lu", m_luFtpPort );
	m_strSocksProxyPort.Format( "%lu", m_luSocksPort );
}


void ChPrefsProxyPage::WriteRegistry()
{
	m_reg.WriteBool( CH_PROXIES, m_boolUseProxies );

	m_reg.Write( CH_HTTP_PROXY, m_strHttpProxy );
	m_reg.Write( CH_FTP_PROXY, m_strFtpProxy );
	m_reg.Write( CH_SOCKS_PROXY, m_strSocksProxy );

	if (m_luHttpPort)
	{
		m_reg.Write( CH_HTTP_PROXY_PORT, m_luHttpPort );
	}
	else
	{
		m_reg.Erase( CH_HTTP_PROXY_PORT );
	}

	if (m_luFtpPort)
	{
		m_reg.Write( CH_FTP_PROXY_PORT, m_luFtpPort );
	}
	else
	{
		m_reg.Erase( CH_FTP_PROXY_PORT );
	}

	if (m_luSocksPort)
	{
		m_reg.Write( CH_SOCKS_PROXY_PORT, m_luSocksPort );
	}
	else
	{
		m_reg.Erase( CH_SOCKS_PROXY_PORT );
	}
}


void ChPrefsProxyPage::UpdateDigitField( CEdit& editField )
{
	ChString		strOldText;
	ChString		strNewText;
	const char*	pstrOld;

	editField.GetWindowText( strOldText );
	pstrOld = strOldText;

	while (*pstrOld)
	{
		if (isdigit( *pstrOld ))
		{
			strNewText += *pstrOld;
		}

		pstrOld++;
	}

	if (strNewText != strOldText)
	{
		int		iStart;
		int		iEnd;

		editField.GetSel( iStart, iEnd );

		editField.SetWindowText( strNewText );

											/* Subtract one since we're
												removing a character from
												the edit field */
		if (iStart > 0)	iStart--;
		if (iEnd > 0)	iEnd--;

		editField.SetSel( iStart, iEnd );
		MessageBeep( MB_OK );
	}
}


void ChPrefsProxyPage::UpdateButtons()
{
	UpdateData();

	m_staticHttpProxy.EnableWindow( m_boolUseProxies );
	m_editHttpProxy.EnableWindow( m_boolUseProxies );
	m_staticHttpPort.EnableWindow( m_boolUseProxies );
	m_editHttpPort.EnableWindow( m_boolUseProxies );
	m_staticFtpProxy.EnableWindow( m_boolUseProxies );
	m_editFtpProxy.EnableWindow( m_boolUseProxies );
	m_staticFtpPort.EnableWindow( m_boolUseProxies );
	m_editFtpPort.EnableWindow( m_boolUseProxies );
	m_staticSocksProxy.EnableWindow( m_boolUseProxies );
	m_editSocksProxy.EnableWindow( m_boolUseProxies );
	m_staticSocksPort.EnableWindow( m_boolUseProxies );
	m_editSocksPort.EnableWindow( m_boolUseProxies );
}


/*----------------------------------------------------------------------------
	ChPrefsProxyPage message handlers
----------------------------------------------------------------------------*/

void ChPrefsProxyPage::OnUpdateEditHttpPort() 
{
	UpdateDigitField( m_editHttpPort );
}


void ChPrefsProxyPage::OnUpdateEditFtpPort() 
{
	UpdateDigitField( m_editFtpPort );
}


void ChPrefsProxyPage::OnUpdateEditSocksPort() 
{
	UpdateDigitField( m_editSocksPort );
}


void ChPrefsProxyPage::OnRadioNoProxies() 
{
	UpdateButtons();
}


void ChPrefsProxyPage::OnRadioManual() 
{
	UpdateButtons();
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:32  uecasm
// Import of source tree as at version 2.53 release.
//
