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

	This file contains the interface for the ChPrefsApps preferences page,
	which manages helper application preferences.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CH_PRAPPS_H_ )
#define CH_PRAPPS_H_

#include <ChReg.h>
#include <ChPage.h>
#include <ChFileDlg.h>

/*----------------------------------------------------------------------------
	ChPrefsApps class
----------------------------------------------------------------------------*/

class ChPrefsApps : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsApps )

	public:
		ChPrefsApps();
		~ChPrefsApps();
											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											// Dialog Data
		//{{AFX_DATA(ChPrefsApps)
		enum { IDD = IDD_PREF_APPS };
		int		m_iRadioBrowsers;
		CButton m_buttonBrowse;
		CStatic m_staticBrowser;
		//}}AFX_DATA

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsApps)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void ReadRegistry();
		void DisplayBrowserName( const ChString& strName );
		void UpdateButtons();

	protected:
		bool		m_boolInitialized;
		CString	m_strBrowser;
		bool		m_boolInternal;
		bool		m_boolUseDefaultBrowser;
		ChRegistry	m_reg;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPrefsApps)
		afx_msg void OnRadioDefaultBrowser();
		afx_msg void OnRadioSpecifiedBrowser();
		afx_msg void OnBrowse();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChWebBrowserSelectFileDlg class
----------------------------------------------------------------------------*/

class ChWebBrowserSelectFileDlg : public ChFileDialog
{
	public:
		ChWebBrowserSelectFileDlg( const ChString& strTitle,
									const ChString& strPath,
									const ChString& strFilter,
									CWnd* pParent = 0 );

		inline bool IsInternal() { return m_boolInternal; }
		inline void UseInternal() { OnInternal(); }

											// Dialog Data
		//{{AFX_DATA(ChWebBrowserSelectFileDlg)
		enum { IDD = IDD_SELECT_WEB_BROWSER };
		//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChWebBrowserSelectFileDlg)
		public:
			virtual int DoModal();
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual void OnInitDone();
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChWebBrowserSelectFileDlg)
        afx_msg void OnInternal();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		bool			m_boolInternal;
		ChRegistry		m_reg;
		ChString			m_strInitialDir;
};


#endif // CH_PRAPPS_H_

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:31  uecasm
// Import of source tree as at version 2.53 release.
//
