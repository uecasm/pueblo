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

	Interface for the ChPrefsDlg class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHPREFSWORLD_H )
#define _CHPREFSWORLD_H

#include <ChReg.h>
#include <ChFileDlg.h>

#include "ChKeyMapType.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif


#if !defined( CH_PUEBLO_PLUGIN )
#include <ChPage.h>
#define ChPropertyBaseClass ChPropertyPage
#else
#define ChPropertyBaseClass CPropertyPage
#endif

/*----------------------------------------------------------------------------
	ChWorldPrefsPage class
----------------------------------------------------------------------------*/

class ChWorldPrefsPage : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE( ChWorldPrefsPage )

	public:
		ChWorldPrefsPage();
		virtual ~ChWorldPrefsPage();

		void UpdateButtons();
											// Accessor functions

		inline bool GetEcho() { return (m_boolEcho != FALSE); }
		inline bool GetBold() { return (m_boolBold != FALSE); }
		inline bool GetItalic() { return (m_boolItalic != FALSE); }
		inline bool GetPause() { return (m_boolPauseOnDisconnect != FALSE); }
		inline bool GetPauseInline() { return (m_boolPauseInline != FALSE); }

		#if defined( CH_PUEBLO_PLUGIN )
		virtual BOOL OnKillActive( );
		#else
		virtual void OnCommit();
		#endif

	protected:
											// Dialog Data
		//{{AFX_DATA(ChWorldPrefsPage)
		enum { IDD = IDD_PREF_PAGE_WORLD };
		BOOL	m_boolBold;
		BOOL	m_boolEcho;
		BOOL	m_boolItalic;
		BOOL	m_boolPauseOnDisconnect;
		BOOL	m_boolPauseInline;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChWorldPrefsPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		// Generated message map functions
		//{{AFX_MSG(ChWorldPrefsPage)
		afx_msg void OnPauseDisconnect();
		afx_msg void OnEchoInput();
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChRegistry		m_reg;
};


/*----------------------------------------------------------------------------
	ChTextInputPrefsPage class
----------------------------------------------------------------------------*/
	
class ChTextInputPrefsPage : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE( ChTextInputPrefsPage )

	public:
		ChTextInputPrefsPage();
		virtual ~ChTextInputPrefsPage();

		inline ChKeyMapType GetKeymap() { return m_keyMap; }
		inline chint16 GetEditLines() { return m_sEditLines; }

		virtual BOOL OnSetActive();			/* Called when this page gets the
												focus */
		virtual BOOL OnKillActive();	    // Perform validation here
		#if !defined( CH_PUEBLO_PLUGIN )
		virtual void OnCommit();   /* Called to commit data (this is
												a good time to save data to the
												registry */

		#endif

		void SetEditLines( chint16 sLines );

											// Dialog Data
		//{{AFX_DATA(ChTextInputPrefsPage)
		enum { IDD = IDD_PREF_PAGE_TEXT_IN };
		CEdit	m_editLineCount;
		int		m_iKeyMap;
		BOOL	m_boolClear;
		CString	m_strLineCount;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChTextInputPrefsPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline bool IsDirty() { return m_boolDirty; }
		inline void SetDirty( bool boolDirty = true )
						{
							m_boolDirty = boolDirty;
						}

		void DisplayTinTinName( const ChString& strName );

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChTextInputPrefsPage)
		afx_msg void OnUpdateEditLineCount();
		afx_msg void OnTintinFile();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChRegistry		m_reg;
		bool			m_boolInitialized;
		bool			m_boolDirty;

		chint16			m_sEditLines;
		ChKeyMapType	m_keyMap;

		ChString			m_strTinTinFile;
};



/*----------------------------------------------------------------------------
	ChProtocolPrefsPage class
----------------------------------------------------------------------------*/
	
class ChProtocolPrefsPage : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE( ChProtocolPrefsPage )

	public:
		ChProtocolPrefsPage();
		virtual ~ChProtocolPrefsPage();

		virtual BOOL OnSetActive();			/* Called when this page gets the
												focus */
		virtual BOOL OnKillActive();	    // Perform validation here
		#if !defined( CH_PUEBLO_PLUGIN )
		virtual void OnCommit();   /* Called to commit data (this is
												a good time to save data to the
												registry */

		#endif

											// Dialog Data
		//{{AFX_DATA(ChProtocolPrefsPage)
		enum { IDD = IDD_PREF_PAGE_PROTOCOL };
		BOOL	m_boolAllowMCCP;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChProtocolPrefsPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline bool IsDirty() { return m_boolDirty; }
		inline void SetDirty( bool boolDirty = true )
						{
							m_boolDirty = boolDirty;
						}

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChProtocolPrefsPage)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChRegistry		m_reg;
		bool			m_boolInitialized;
		bool			m_boolDirty;
};



/*----------------------------------------------------------------------------
	ChTinTinSelectFileDlg class
----------------------------------------------------------------------------*/

class ChTinTinSelectFileDlg : public ChFileDialog
{
	public:
		ChTinTinSelectFileDlg( const ChString& strTitle,
								const ChString& strPath,
								const ChString& strFilter,
								CWnd* pParent = 0 );

											// Dialog Data
		//{{AFX_DATA(ChWebBrowserSelectFileDlg)
		enum { IDD = IDD_SELECT_TINTIN };
		//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChWebBrowserSelectFileDlg)
		public:
			virtual int DoModal();
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChWebBrowserSelectFileDlg)
        afx_msg void OnNoTinTinFile();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChRegistry		m_reg;
		ChString			m_strInitialDir;
};


#endif	// !defined( _CHPREFSWORLD_H )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:10  uecasm
// Import of source tree as at version 2.53 release.
//
