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

	Implementation of the ChNotifyPrefsPage class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHPREFSNOTIFY_H )
#define _CHPREFSNOTIFY_H

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChReg.h>
#if !defined( CH_PUEBLO_PLUGIN )
#include <ChPage.h>
#define ChPropertyBaseClass ChPropertyPage
#else
#define ChPropertyBaseClass CPropertyPage
#endif


/*----------------------------------------------------------------------------
	ChNotifyPrefsPage class
----------------------------------------------------------------------------*/

class ChNotifyPrefsPage : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE(ChNotifyPrefsPage)

	public:
		ChNotifyPrefsPage();
		~ChNotifyPrefsPage();

		inline bool GetNotifyInactive() { return (m_iNotifyOption == radioWhenInactive); }
		inline bool GetNotifyFlash() { return (m_boolFlash != FALSE); }
		inline bool GetNotifyAlert() { return (m_boolAlert != FALSE); }
		inline const ChString& GetNotifyMatch() { return m_strMatch; }

		void Set( bool boolInactive, bool boolFlash, bool boolAlert, const ChString& strMatch );

		virtual BOOL OnSetActive();			/* Called when this page gets the
												focus */
		#if defined( CH_PUEBLO_PLUGIN )
		virtual BOOL OnKillActive();	    // Perform validation here
		#else
		virtual void OnCommit();   /* Called to commit data (this is
												a good time to save data to the
												registry */

		#endif
											// Dialog Data
		//{{AFX_DATA(ChNotifyPrefsPage)
		enum { IDD = IDD_PREF_PAGE_NOTIFY };
		int		m_iNotifyOption;
		CButton m_checkOnMatch;
		BOOL	m_boolOnMatch;
		CEdit	m_editMatch;
		CString	m_strMatch;
		BOOL	m_boolFlash;
		BOOL	m_boolAlert;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChNotifyPrefsPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChNotifyPrefsPage)
		afx_msg void OnNotifyOnMatch();
		//}}AFX_MSG
	
		DECLARE_MESSAGE_MAP()

	protected:
		enum tagRadioButtons { radioWhenMinimised, radioWhenInactive };
};

#endif	// !defined( _CHPREFSNOTIFY_H )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:09  uecasm
// Import of source tree as at version 2.53 release.
//
