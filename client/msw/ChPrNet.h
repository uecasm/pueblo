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

	This file contains the interface for the ChPrefsCachePage class and
	ChPrefsNetworkPage class, which handle preferences for the main cache
	and the network.

----------------------------------------------------------------------------*/

#if !defined( _CHPRNET_H )
#define _CHPRNET_H

#include <ChPage.h>


/*----------------------------------------------------------------------------
	ChPrefsCachePage class
----------------------------------------------------------------------------*/

class ChPrefsCachePage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsCachePage )

	public:
		ChPrefsCachePage();
		~ChPrefsCachePage();

											// Dialog Data
		//{{AFX_DATA(ChPrefsCachePage)
		enum { IDD = IDD_PREF_CACHE };
		CString		m_cacheDir;
		UINT 		m_uCacheSize;
		//}}AFX_DATA

											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsCachePage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPrefsCachePage)
		afx_msg void OnClrCache();
		//}}AFX_MSG

	protected:
		bool		m_boolInitialized;
		ChRegistry	m_reg;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChPrefsNetworkPage class
----------------------------------------------------------------------------*/

class ChPrefsNetworkPage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsNetworkPage )

	public:
		ChPrefsNetworkPage();
		~ChPrefsNetworkPage();

											// Dialog Data
		//{{AFX_DATA(ChPrefsNetworkPage)
		enum { IDD = IDD_NETWORK };
		UINT	m_uMaxConn;
		//}}AFX_DATA

											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsNetworkPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPrefsNetworkPage)
			// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

	protected:
		bool		m_boolInitialized;
		ChRegistry	m_reg;

	DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHPRNET_H )
