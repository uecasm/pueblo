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

	TinTin class linked list methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChDlg.h>

#include "headers.h"
#include "TinTinInfo.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	TinTinInfo class
----------------------------------------------------------------------------*/

TinTinInfo::TinTinInfo( CWnd* pParent ) :
				ChDialog( 
#if defined( CH_PUEBLO_PLUGIN )
					(chparam)AfxGetInstanceHandle(), 
#else
					(chparam)ChWorldDLL.hModule, 
#endif
					TinTinInfo::IDD,
					pParent ),
				m_iActions( 0 ),
				m_iAliases( 0 ),
				m_iAntiSubs( 0 ),
				m_iHighlights( 0 ),
				m_iSubs( 0 ),
				m_iVars( 0 )
{
	//{{AFX_DATA_INIT(TinTinInfo)
	m_strSpeedwalking = _T("");
	m_strPresubs = _T("");
	m_strDoSubs = _T("");
	m_strIgnoreActions = _T("");
	//}}AFX_DATA_INIT
}


void TinTinInfo::SetCounts( int iActions, int iAliases, int iSubs,
							int iAntiSubs, int iVars, int iHighlights )
{
	m_iActions = iActions;
	m_iAliases = iAliases;
	m_iAntiSubs = iAntiSubs;
	m_iHighlights = iHighlights;
	m_iSubs = iSubs;
	m_iVars = iVars;
}


void TinTinInfo::SetFlags( bool boolActions, bool boolSpeedWalking,
							bool boolSubs, bool boolPresubs )
{
	SetOnOff( m_strSpeedwalking, boolSpeedWalking );

	SetYesNo( m_strIgnoreActions, boolActions );
	SetYesNo( m_strDoSubs, boolSubs );

	SetYesNo( m_strPresubs, boolPresubs );
}


void TinTinInfo::DoDataExchange( CDataExchange* pDX )
{
	ChDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(TinTinInfo)
	DDX_Control(pDX, IDC_STATIC_ANTISUBS, m_staticAntiSubs);
	DDX_Control(pDX, IDC_STATIC_VARS, m_staticVars);
	DDX_Control(pDX, IDC_STATIC_SUBS, m_staticSubs);
	DDX_Control(pDX, IDC_STATIC_HIGHLIGHTS, m_staticHighlights);
	DDX_Control(pDX, IDC_STATIC_ALIASES, m_staticAliases);
	DDX_Control(pDX, IDC_STATIC_ACTIONS, m_staticActions);
	DDX_Text(pDX, IDC_STATIC_SPEEDWALKING, m_strSpeedwalking);
	DDX_Text(pDX, IDC_STATIC_PRESUBS, m_strPresubs);
	DDX_Text(pDX, IDC_STATIC_DO_SUBS, m_strDoSubs);
	DDX_Text(pDX, IDC_STATIC_IGNORE_ACTIONS, m_strIgnoreActions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( TinTinInfo, ChDialog )
	//{{AFX_MSG_MAP(TinTinInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	TinTinInfo protected methods
----------------------------------------------------------------------------*/

void TinTinInfo::SetValue( CStatic& staticControl, int iValue )
{
	ChString		strFormat;
	ChString		strValue;

	staticControl.GetWindowText( strFormat );
	strValue.Format( strFormat, iValue );
	staticControl.SetWindowText( strValue );
}


void TinTinInfo::SetYesNo( ChString& strField, bool boolYes )
{
	if (boolYes)
	{
		strField = "yes";
	}
	else
	{
		strField = "no";
	}
}


void TinTinInfo::SetOnOff( ChString& strField, bool boolOn )
{
	if (boolOn)
	{
		strField = "on";
	}
	else
	{
		strField = "off";
	}
}


void TinTinInfo::SetTrueFalse( ChString& strField, bool boolTrue )
{
	if (boolTrue)
	{
		strField = "true";
	}
	else
	{
		strField = "false";
	}
}


/*----------------------------------------------------------------------------
	TinTinInfo message handlers
----------------------------------------------------------------------------*/


BOOL TinTinInfo::OnInitDialog() 
{
	ChDialog ::OnInitDialog();
	
	SetValue( m_staticActions, m_iActions );
	SetValue( m_staticAliases, m_iAliases );
	SetValue( m_staticSubs, m_iSubs );
	SetValue( m_staticAntiSubs, m_iAntiSubs );
	SetValue( m_staticVars, m_iVars );
	SetValue( m_staticHighlights, m_iHighlights );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// $Log$
