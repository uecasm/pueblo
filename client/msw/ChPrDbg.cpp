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

	This file contains the implementation of the ChPrefsDebugPage class,
	which manages debugging preferences.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "Pueblo.h"
#include "ChPrDbg.h"
#include "MemDebug.h"

#if defined( _DEBUG )
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;


/*----------------------------------------------------------------------------
	ChPrefsDebugPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsDebugPage, ChPropertyPage )

ChPrefsDebugPage::ChPrefsDebugPage() :
					ChPropertyPage( ChPrefsDebugPage::IDD, 0, hInstApp ),
					m_reg( CH_COMPANY_NAME, CH_PRODUCT_NAME,
							CH_DEBUG_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsDebugPage)
	m_boolUseLocal = false;
	//}}AFX_DATA_INIT
}


ChPrefsDebugPage::~ChPrefsDebugPage()
{
}


void ChPrefsDebugPage::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChPrefsDebugPage)
	DDX_Check(pDX, IDC_USE_LOCAL_MODULES, m_boolUseLocal);
	//}}AFX_DATA_MAP
}


BOOL ChPrefsDebugPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		bool bTemp;
		m_reg.ReadBool( CH_DEBUG_USE_LOCAL, bTemp, false );
		m_boolUseLocal = bTemp;
		m_boolOldUseLocal = m_boolUseLocal;

		CheckDlgButton( IDC_USE_LOCAL_MODULES, m_boolUseLocal );

											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsDebugPage::OnCommit()
{
	if (m_boolInitialized)
	{
		if (m_boolUseLocal != m_boolOldUseLocal)
		{
			m_reg.WriteBool( CH_DEBUG_USE_LOCAL, m_boolUseLocal );

											// Force a reload of debug info

//			ChClientCore::GetCore()->LoadDebugInfo();
		}
	}
}


BEGIN_MESSAGE_MAP( ChPrefsDebugPage, ChPropertyPage )
	//{{AFX_MSG_MAP(ChPrefsDebugPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsDebugPage message handlers
----------------------------------------------------------------------------*/

#endif	// defined( _DEBUG )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:31  uecasm
// Import of source tree as at version 2.53 release.
//
