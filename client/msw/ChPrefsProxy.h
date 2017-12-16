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

	This file contains the interface for the ChPrefsProxyPage class,
	which allows the user to select firewall proxy preferences.

----------------------------------------------------------------------------*/

#if !defined( _CHPREFSPROXY_H )
#define _CHPREFSPROXY_H

#include <ChReg.h>
#include <ChPage.h>


/*----------------------------------------------------------------------------
	ChPrefsProxyPage class
----------------------------------------------------------------------------*/

class ChPrefsProxyPage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsProxyPage )

	public:
		ChPrefsProxyPage();
		~ChPrefsProxyPage();
											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											// Dialog Data
		//{{AFX_DATA(ChPrefsProxyPage)
		enum { IDD = IDD_PREF_PROXIES };
		CEdit	m_editHttpProxy;
		CEdit	m_editHttpPort;
		CEdit	m_editFtpProxy;
		CEdit	m_editFtpPort;
		CEdit	m_editSocksProxy;
		CEdit	m_editSocksPort;
		CStatic	m_staticHttpProxy;
		CStatic	m_staticHttpPort;
		CStatic	m_staticFtpProxy;
		CStatic	m_staticFtpPort;
		CStatic	m_staticSocksProxy;
		CStatic	m_staticSocksPort;
		int		m_iRadioProxies;
		CString	m_strHttpProxy;
		CString	m_strHttpProxyPort;
		CString	m_strFtpProxy;
		CString	m_strFtpProxyPort;
		CString	m_strSocksProxy;
		CString	m_strSocksProxyPort;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsProxyPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPrefsProxyPage)
		afx_msg void OnUpdateEditFtpPort();
		afx_msg void OnUpdateEditHttpPort();
		afx_msg void OnUpdateEditSocksPort();
		afx_msg void OnRadioNoProxies();
		afx_msg void OnRadioManual();
		//}}AFX_MSG

	protected:
		void ReadRegistry();
		void WriteRegistry();
		void UpdateDigitField( CEdit& editField );
		void UpdateButtons();

	protected:
		bool		m_boolInitialized;
		ChRegistry	m_reg;

		bool		m_boolUseProxies;

		chuint32	m_luHttpPort;
		chuint32	m_luFtpPort;
		chuint32	m_luSocksPort;

		DECLARE_MESSAGE_MAP()
};


#endif	// !defined( _CHPREFSPROXY_H )
