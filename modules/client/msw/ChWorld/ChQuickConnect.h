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

#if !defined( CHQUICKCONNECT_H )
#define CHQUICKCONNECT_H

#include <ChDlg.h>


/*----------------------------------------------------------------------------
	ChQuickConnect class
----------------------------------------------------------------------------*/

class ChQuickConnect : public ChDialog
{
	public:
		ChQuickConnect( CWnd* pParent = 0 );

		inline const ChString& GetHost() { return m_strHost; }
		inline chint16 GetPort() { return m_sPort; }
		inline ChLoginType GetLoginType() { return m_loginType; }

											// Dialog Data
		//{{AFX_DATA(ChQuickConnect)
		enum { IDD = IDD_QUICKCONNECT };
		CButton	m_btnConnect;
		CEdit	m_editPort;
		CEdit	m_editHost;
		int		m_iLoginStyle;
		CString	m_strHost;
		CString	m_strPort;
		//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChQuickConnect)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void UpdateButtons();

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChQuickConnect)
		virtual BOOL OnInitDialog();
		afx_msg void OnChangeListHost();
		afx_msg void OnChangeListPort();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChLoginType	m_loginType;
		chint16		m_sPort;
};


#endif	// !defined( CHQUICKCONNECT_H )

// $Log$
