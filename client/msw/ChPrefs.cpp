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

	This file contains the implementation of the ChPrefs class, which
	manages preferences.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "Pueblo.h"
#include "ChPrefs.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChPrefs class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChPrefs, CPropertySheet )


ChPrefs::ChPrefs( ChCore* pCore, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage ) :
			CPropertySheet( nIDCaption, pParentWnd, iSelectPage ),
			m_pageMgr( pCore, pagePreferences )
{
}

ChPrefs::ChPrefs( ChCore* pCore, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage ) :
			CPropertySheet( pszCaption, pParentWnd, iSelectPage ),
			m_pageMgr( pCore, pagePreferences )
{
}

ChPrefs::~ChPrefs()
{
}


void ChPrefs::AddModulePages()
{
	m_pageMgr.AddModulePages( this );
}


BEGIN_MESSAGE_MAP( ChPrefs, CPropertySheet )
	//{{AFX_MSG_MAP(ChPrefs)
		ON_BN_CLICKED(IDOK, OnOK)
		ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefs message handlers
----------------------------------------------------------------------------*/

void ChPrefs::OnOK() 
{											/* Grab the data for the current
												sheet */
	if (GetActivePage()->OnKillActive())
	{
											// Get the data from the pages
		m_pageMgr.GetPageData();

		#if ( _MFC_VER > 0x0400	 )
		EndDialog( IDOK );
		#else
		CPropertySheet::OnOK(  );
		#endif
											/* Release the pages for other
												modules */
		m_pageMgr.ReleaseModulePages();
	}
}

void ChPrefs::OnCancel()
{
	#if ( _MFC_VER > 0x0400	 )
	EndDialog( IDCANCEL );
	#else
	CPropertySheet::OnOK(  );
	#endif
											/* Release the pages for other
												modules */
	m_pageMgr.ReleaseModulePages();
}

void ChPrefs::OnClose() 
{
	CPropertySheet::OnClose();
											/* Release the pages for other
												modules */
	m_pageMgr.ReleaseModulePages();
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:32  uecasm
// Import of source tree as at version 2.53 release.
//
