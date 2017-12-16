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

#if !defined( _CHOODDLG_H )
#define _CHOODDLG_H


/*----------------------------------------------------------------------------
	ChOutOfDateDlg class
----------------------------------------------------------------------------*/

class ChOutOfDateDlg : public CDialog
{
	public:
		ChOutOfDateDlg( bool boolCritical, const ChString& strDescription,
						CWnd* pParent = 0 );

											// Dialog Data
		//{{AFX_DATA(ChOutOfDateDlg)
		enum { IDD = IDD_CLIENT_OUTDATED };
		CButton	m_btnUpdateLater;
		CEdit	m_editReason;
		//}}AFX_DATA

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChOutOfDateDlg)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChOutOfDateDlg)
		virtual BOOL OnInitDialog();
	afx_msg void OnShutdown();
	afx_msg void OnUpdateLater();
	afx_msg void OnUpdateNow();
	afx_msg void OnHelp();
	//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		bool		m_boolCritical;
		ChString		m_strDesc;
};

#endif	// !defined( _CHOODDLG_H )
