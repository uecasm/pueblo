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

#if !defined( _TINTININFO_H )
#define _TINTININFO_H

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif


/*----------------------------------------------------------------------------
	TinTinInfo class
----------------------------------------------------------------------------*/

class TinTinInfo : public ChDialog
{
	public:
		TinTinInfo( CWnd* pParent = 0 );

		void SetCounts( int iActions, int iAliases, int iSubs,
						int iAntiSubs, int iVars, int iHighlights );
		void SetFlags( bool boolIgnoreActions, bool boolSpeedWalking,
						bool boolSubs, bool boolPresubs );

											// Dialog Data
		//{{AFX_DATA(TinTinInfo)
		enum { IDD = IDD_TINTIN_INFO };
		CStatic	m_staticAntiSubs;
		CStatic	m_staticVars;
		CStatic	m_staticSubs;
		CStatic	m_staticHighlights;
		CStatic	m_staticAliases;
		CStatic	m_staticActions;
		CString	m_strSpeedwalking;
		CString	m_strPresubs;
		CString	m_strDoSubs;
		CString	m_strIgnoreActions;
		//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(TinTinInfo)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void SetValue( CStatic& staticControl, int iValue );
		void SetYesNo( ChString& strField, bool boolYes );
		void SetOnOff( ChString& strField, bool boolOn );
		void SetTrueFalse( ChString& strField, bool boolTrue );

	protected:
											// Generated message map functions
		//{{AFX_MSG(TinTinInfo)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		int		m_iActions;
		int		m_iAliases;
		int		m_iAntiSubs;
		int		m_iHighlights;
		int		m_iSubs;
		int		m_iVars;
};

#endif	// !defined( _TINTININFO_H )

// $Log$
