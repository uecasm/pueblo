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

	This file contains the implementation of the ChPrefsDebugPage
	preferences page, which manages debugging preferences.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHDBGPRF_H )
#define _CHDBGPRF_H

#if defined( _DEBUG )

#include <ChReg.h>
#include <ChPage.h>


/*----------------------------------------------------------------------------
	ChPrefsDebugPage class
----------------------------------------------------------------------------*/

class ChPrefsDebugPage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsDebugPage )

	public:
		ChPrefsDebugPage();
		~ChPrefsDebugPage();

		inline bool GetUseLocal() { return (m_boolUseLocal != FALSE); }

		// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											// Dialog Data
		//{{AFX_DATA(ChPrefsDebugPage)
		enum { IDD = IDD_PREF_DEBUG };
		BOOL	m_boolUseLocal;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsDebugPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPrefsDebugPage)
		// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	protected:
		bool		m_boolInitialized;
		BOOL		m_boolOldUseLocal;
		ChRegistry	m_reg;
};

#endif	// defined( _DEBUG )
#endif	// !defined( _CHDBGPRF_H )

// $Log$
