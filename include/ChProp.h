/*----------------------------------------------------------------------------
  _   _ _  _                 ____        _                     _
 | | | | || |               |  __|      | |                   (_)
 | | | | || |_ _ __  __ _   | |_  _ __ _| |_ ___ _ ___ __ _ __ _ ___  ___ ___
 | | | | |_  _| '__|/ _` |  |  _|| '_ \_   _| _ \ '__|'_ \ '__| | __|/ _ \ __|
 | '-' | || | | |  | (_| |  | |__| | | || ||  __/ | | |_) ||  | |__ |  __/__ |
  \___/|_||_| |_|   \__,_|  |____|_| |_||_| \___|_| | .__/_|  |_|___|\___|___|
                                                    | |     
                                                    |_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo/UE client code, first released April 1, 2002.

    The Initial Developer of the Original Code is Ultra Enterprises.

    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Wrote and designed this class; it was not originally part of
					the Pueblo client.

------------------------------------------------------------------------------

	This file consists of interfaces of the ChPropertySheet class, used to
	allow a CPropertySheet to have custom fonts.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHPROP_H )
#define _CHPROP_H


#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	ChPropertySheet class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPropertySheet : public CPropertySheet {
	DECLARE_DYNAMIC(ChPropertySheet)

	public:
		ChPropertySheet(UINT nIDCaption, CWnd *pParentWnd=NULL, UINT iSelectPage=0);
		ChPropertySheet(LPCTSTR pszCaption, CWnd *pParentWnd=NULL, UINT iSelectPage=0);
		virtual ~ChPropertySheet();

		virtual BOOL OnInitDialog();
	protected:
	
		virtual void BuildPropPageArray ();
										// Generated message map functions
		//{{AFX_MSG(ChWizardPage)
		virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg LONG OnResizePage (UINT, LONG);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		CRect		m_rctPage;
		CFont		m_fntPage;
};

#endif	// !defined( _CHPROP_H )

// $Log$
