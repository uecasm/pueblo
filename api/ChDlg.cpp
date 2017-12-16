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

	This file contains the implementation of the ChDialog class.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChTypes.h>
#include <ChDlg.h>


#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChDialog class
----------------------------------------------------------------------------*/

ChDialog::ChDialog( chparam resources, LPCTSTR lpszTemplateName,
					CWnd* pParentWnd ) :
			CDialog( lpszTemplateName, pParentWnd ),
			m_res( resources ),
			m_hwndLast( 0 )
{
}


ChDialog::ChDialog( chparam resources, UINT nIDTemplate,
					CWnd* pParentWnd ) :
			CDialog( nIDTemplate, pParentWnd ),
			m_res( resources )
{
}


BOOL ChDialog::Create( LPCTSTR lpszTemplateName, CWnd* pParentWnd )
{
	HINSTANCE	hModule = (HINSTANCE)m_res;
	HINSTANCE	hInstOld = AfxGetResourceHandle();
	BOOL		boolResult;

	AfxSetResourceHandle( hModule );
											// Create the window

	boolResult = CDialog::Create( lpszTemplateName, pParentWnd );

											// Restore the old resource chain
	AfxSetResourceHandle( hInstOld );

	return boolResult;
}


BOOL ChDialog::Create( UINT nIDTemplate, CWnd* pParentWnd )
{
	HINSTANCE	hModule = (HINSTANCE)m_res;
	HINSTANCE	hInstOld = AfxGetResourceHandle();
	BOOL		boolResult;

	AfxSetResourceHandle( hModule );
											// Create the window

	boolResult = CDialog::Create( nIDTemplate, pParentWnd );

											// Restore the old resource chain
	AfxSetResourceHandle( hInstOld );

	return boolResult;
}

int ChDialog::DoModal()
{
	HINSTANCE	hModule = (HINSTANCE)m_res;
	HINSTANCE	hInstOld = AfxGetResourceHandle();
	int			iResult;

	AfxSetResourceHandle( hModule );
											// Save the current focus window
	m_hwndLast = ::GetFocus();
											// Do the dialog
	iResult = CDialog::DoModal();
											// Restore the old resource chain
	AfxSetResourceHandle( hInstOld );
											// Restore the last focus window
	if (::IsWindow( m_hwndLast ))
	{
		::SetFocus( m_hwndLast );
	}

	return iResult;
}
