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

#include "headers.h"

#include "ChWorld.h"
#include "ChPrefsNotify.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChNotifyPrefsPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChNotifyPrefsPage, ChPropertyPage )


ChNotifyPrefsPage::ChNotifyPrefsPage() :
		ChPropertyPage( ChNotifyPrefsPage::IDD, 0, 
			#if defined( CH_PUEBLO_PLUGIN )
				AfxGetInstanceHandle()
			#else
				ChWorldDLL.hModule 
			#endif
		)
{
	//{{AFX_DATA_INIT(ChNotifyPrefsPage)
	m_iNotifyOption = -1;
	m_boolOnMatch = FALSE;
	m_strMatch = _T("");
	m_boolFlash = FALSE;
	m_boolAlert = FALSE;
	//}}AFX_DATA_INIT
}


ChNotifyPrefsPage::~ChNotifyPrefsPage()
{
}


void ChNotifyPrefsPage::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChNotifyPrefsPage)
	DDX_Radio(pDX, IDC_NOTIFY_MINIMISED, m_iNotifyOption);
	DDX_Control(pDX, IDC_NOTIFY_ON_MATCH, m_checkOnMatch);
	DDX_Check(pDX, IDC_NOTIFY_ON_MATCH, m_boolOnMatch);
	DDX_Control(pDX, IDC_EDIT_MATCH_TEXT, m_editMatch);
	DDX_Text(pDX, IDC_EDIT_MATCH_TEXT, m_strMatch);
	DDX_Check(pDX, IDC_NOTIFY_FLASH, m_boolFlash);
	DDX_Check(pDX, IDC_NOTIFY_ALERT, m_boolAlert);
	//}}AFX_DATA_MAP

	m_editMatch.EnableWindow(m_boolOnMatch);
}


void ChNotifyPrefsPage::Set( bool boolInactive, bool boolFlash,
								bool boolAlert, const ChString& strMatch )
{
	m_iNotifyOption = (boolInactive) ? radioWhenInactive : radioWhenMinimised;
	m_boolOnMatch = (strMatch.IsEmpty() == false);
	m_strMatch = strMatch;
	m_boolFlash = boolFlash;
	m_boolAlert = boolAlert;
											// Init the data
	if (::IsWindow( m_hWnd ))
	{
		UpdateData( false );
	}
}

BOOL ChNotifyPrefsPage::OnSetActive()
{
	//m_editMatch.EnableWindow(m_boolOnMatch);
	return ChPropertyPage::OnSetActive();
}

void ChNotifyPrefsPage::OnCommit()
{
	ChRegistry		reg( WORLD_PREFS_GROUP );
	ChString			strMatch;

	if (m_boolOnMatch)
	{
		strMatch = m_strMatch;
	}
	else
	{
		strMatch = "";
	}

	reg.WriteBool( WORLD_PREFS_NOTIFY_INACTIVE, GetNotifyInactive() );
	reg.WriteBool( WORLD_PREFS_NOTIFY_FLASH, GetNotifyFlash() );
	reg.WriteBool( WORLD_PREFS_NOTIFY_ALERT, GetNotifyAlert() );
	reg.Write( WORLD_PREFS_NOTIFY_STR, strMatch );
}


BEGIN_MESSAGE_MAP( ChNotifyPrefsPage, ChPropertyPage )
	//{{AFX_MSG_MAP(ChNotifyPrefsPage)
	ON_BN_CLICKED(IDC_NOTIFY_ON_MATCH, OnNotifyOnMatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChNotifyPrefsPage message handlers
----------------------------------------------------------------------------*/

void ChNotifyPrefsPage::OnNotifyOnMatch() 
{
	m_editMatch.EnableWindow(m_checkOnMatch.GetCheck() == BST_CHECKED);
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:09  uecasm
// Import of source tree as at version 2.53 release.
//
