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

#if !defined( _CHCONNECTDLG_H_ )
#define _CHCONNECTDLG_H_


/*----------------------------------------------------------------------------
	Includes
----------------------------------------------------------------------------*/

#include <ChDlg.h>

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif


/*----------------------------------------------------------------------------
	Forward class declarations
----------------------------------------------------------------------------*/

class ChWorldMainInfo;


/*----------------------------------------------------------------------------
	ChConnectingDlg class
----------------------------------------------------------------------------*/

class ChConnectingDlg : public ChDialog
{
	public:
		ChConnectingDlg( ChWorldMainInfo* pMainInfo, CWnd* pParent = 0 );

		inline ChWorldMainInfo* GetMainInfo() { return m_pMainInfo; }

		BOOL Create( const ChString& strMessage, CWnd* pParentWnd = 0 );
		void ShowAfterASec();
		void ChangeMessage( const ChString& strMessage );

											// Dialog Data
		//{{AFX_DATA(ChConnectingDlg)
		enum { IDD = IDD_CONNECTING_DLG };
		CString	m_strConnectMsg;
		//}}AFX_DATA

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChConnectingDlg)
		public:
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChConnectingDlg)
		virtual void OnCancel();
		afx_msg void OnTimer(UINT nIDEvent);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		enum { timerShowID = 12345 };

		ChWorldMainInfo*		m_pMainInfo;
};

#endif	// !defined( _CHCONNECTDLG_H_ )

// $Log$
