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

	This file contains the implementation of the ChPrefsTracePage class,
	which manages debugging preferences.

----------------------------------------------------------------------------*/

#include "headers.h"

#include "Pueblo.h"
#include "ChPrefsTrace.h"

#include <ChCore.h>
#include "MemDebug.h"

#if defined( _DEBUG )
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif	// defined( _DEBUG )


/*----------------------------------------------------------------------------
	ChPrefsTracePage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsTracePage, ChPropertyPage )

ChPrefsTracePage::ChPrefsTracePage() :
					ChPropertyPage( ChPrefsTracePage::IDD, 0, hInstApp ),
					m_reg( CH_MISC_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsTracePage)
	m_boolHtml = false;
	m_boolErrors = FALSE;
	m_boolWarnings = FALSE;
	m_boolMiscMsgs = FALSE;
	//}}AFX_DATA_INIT

	m_reg.Read( CH_MISC_TRACE_OPTIONS, m_flOptions,
					ChCore::m_flTraceDefault );
}


ChPrefsTracePage::~ChPrefsTracePage()
{
}


void ChPrefsTracePage::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	if (!pDX->m_bSaveAndValidate)
	{
		m_boolHtml = !!(m_flOptions & ChCore::traceHTML);
		m_boolErrors = !!(m_flOptions & ChCore::traceErrors);
		m_boolWarnings = !!(m_flOptions & ChCore::traceWarnings);
		m_boolMiscMsgs = !!(m_flOptions & ChCore::traceMiscMessages);
	}

	//{{AFX_DATA_MAP(ChPrefsTracePage)
	DDX_Check(pDX, IDC_CHECK_HTML, m_boolHtml);
	DDX_Check(pDX, IDC_CHECK_ERRORS, m_boolErrors);
	DDX_Check(pDX, IDC_CHECK_WARNINGS, m_boolWarnings);
	DDX_Check(pDX, IDC_CHECK_MISC_ERRS, m_boolMiscMsgs);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_flOptions = 0;

		if (m_boolHtml)
		{
			m_flOptions |= ChCore::traceHTML;
		}

		if (m_boolErrors)
		{
			m_flOptions |= ChCore::traceErrors;
		}

		if (m_boolWarnings)
		{
			m_flOptions |= ChCore::traceWarnings;
		}

		if (m_boolMiscMsgs)
		{
			m_flOptions |= ChCore::traceMiscMessages;
		}
	}
}


BOOL ChPrefsTracePage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		m_reg.Read( CH_MISC_TRACE_OPTIONS, m_flOptions,
						ChCore::m_flTraceDefault );
		m_flOldOptions = m_flOptions;
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsTracePage::OnCommit()
{
	if (m_boolInitialized)
	{
		if (m_flOldOptions != m_flOptions)
		{
			m_reg.Write( CH_MISC_TRACE_OPTIONS, m_flOptions );

			ChCore::UpdateTraceOptions();
		}
	}
}


BEGIN_MESSAGE_MAP( ChPrefsTracePage, ChPropertyPage )
	//{{AFX_MSG_MAP(ChPrefsTracePage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsTracePage message handlers
----------------------------------------------------------------------------*/
