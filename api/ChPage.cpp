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

		 Ultra Enterprises:  Gavin Lambert
		 
		 			Added font customisation handling.

------------------------------------------------------------------------------

	This file consists of the implementation of the ChPropertyPage class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChPage.h>

#include <MemDebug.h>

// Note :: All header file includes should be above this line
 
#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	ChPropertyPage constructor and destructor
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChPropertyPage, CPropertyPage )

ChPropertyPage::ChPropertyPage( unsigned int idTemplate,
								unsigned int idCaption, HINSTANCE hModule ) :
		CPropertyPage( 1 ),
		m_hModule( hModule )
{
	if (0 == m_hModule)
	{
		CWinApp*	pApp = AfxGetApp();

		m_hModule = pApp->m_hInstance;
	}
											/* Reconstruct using the correct
												resource file */
	m_hOldModule = AfxGetResourceHandle();
	AfxSetResourceHandle( m_hModule );

	CommonConstruct( MAKEINTRESOURCE( idTemplate ), idCaption );

	AfxSetResourceHandle( m_hOldModule );
	
	// UE: don't provide the help button unless the page specifically indicates
	//     that it has help.
	m_psp.dwFlags &= ~(PSP_HASHELP);
}


ChPropertyPage::~ChPropertyPage()
{
	if (m_hWnd != NULL)
	{
		DestroyWindow();
	}
}


/*----------------------------------------------------------------------------
	ChPropertyPage public methods
----------------------------------------------------------------------------*/

BOOL ChPropertyPage::OnSetActive()
{
	BOOL	boolSuccess;
										// Override the resource chain
	m_hOldModule = AfxGetResourceHandle();
	AfxSetResourceHandle( m_hModule );

										// Call the default proc

	boolSuccess = CPropertyPage::OnSetActive();

										// Restore the old resource chain
	AfxSetResourceHandle( m_hOldModule );

	return boolSuccess;
}


BOOL ChPropertyPage::OnKillActive()
{
	BOOL	boolSuccess;
										// Call the default proc

	boolSuccess = CPropertyPage::OnKillActive();

	return boolSuccess;
}

/*----------------------------------------------------------------------------
	ChPropertyPage protected methods
----------------------------------------------------------------------------*/

void ChPropertyPage::SetChildCustomFont(CWnd& childWnd, CFont *pFont)
{
	childWnd.SetFont(pFont);
	::SetProp(childWnd.GetSafeHwnd(), PROP_UECUSTOMFONT, (HANDLE)true);
}

afx_msg void ChPropertyPage::OnDestroy()
{
	// remove the property from any children that have it
	CWnd* pChildWnd = GetWindow(GW_CHILD);

	while (pChildWnd) {
		if(::GetProp(pChildWnd->GetSafeHwnd(), PROP_UECUSTOMFONT)) {
			::RemoveProp(pChildWnd->GetSafeHwnd(), PROP_UECUSTOMFONT);
		}
		
		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}
	
	CPropertyPage::OnDestroy();
}

// $Log$
