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

	Contains the interface for the ChShortcutWizard class, which is
	a Wizard for creating a desktop shortcut file.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSCWIZ_H )
#define _CHSCWIZ_H

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChWizard.h>


class ChShortcutWizard;


/*----------------------------------------------------------------------------
	ChShortcutWizName class
----------------------------------------------------------------------------*/

class ChShortcutWizName : public ChWizardPage
{
	DECLARE_DYNCREATE( ChShortcutWizName )

	public:
		enum { constNameLimit = 30 };

	public:
		ChShortcutWizName();
		~ChShortcutWizName();

		inline const ChString& GetPath() { return m_strPath; }
		inline const ChString& GetName() { return m_strName; }
		inline ChString GetFilePath()
				{
					ChString	strFilePath( GetPath() );

					strFilePath += "\\" + GetFilename();

					return strFilePath;
				}
		inline void SetName( const ChString& strName )
				{
					m_strName = strName;
				}
		inline void SetPath( const ChString& strPath )
				{
					m_strPath = strPath;
				}

		virtual BOOL OnInitPage();			/* Called when this page gets
												the focus */
		virtual BOOL OnNext();

		const ChString& GetFilename();
											// Dialog Data
		//{{AFX_DATA(ChShortcutWizName)
		enum { IDD = IDD_SHORTCUT_WIZ_NAME };
		CButton	m_checkCurrWorld;
		CEdit	m_editName;
		CString	m_strName;
		BOOL	m_boolCurrWorld;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizName)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void GetMangledFilename( ChString& strFilename );
		void StripChars( ChString& strData, const ChString& strChars );

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChShortcutWizName)
	//}}AFX_MSG

	protected:
		ChShortcutWizard*		m_pWiz;
		ChString					m_strPath;
		ChString					m_strFilename;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChShortcutWizServer class
----------------------------------------------------------------------------*/

class ChShortcutWizServer : public ChWizardPage
{
	DECLARE_DYNCREATE( ChShortcutWizServer )

	public:
		enum { constHostLimit = 64, constPortLimit = 4 };

	public:
		ChShortcutWizServer();
		~ChShortcutWizServer();

		virtual BOOL OnInitPage();			/* Called when this page gets
												the focus */
		virtual BOOL OnNext();

		inline void Get( ChString& strHost, chint16& sPort,
									ChWorldType& type ) const
				{
					strHost = m_strHost;
					sPort = (chint16)m_iPort;
					type = m_type;
				}
		inline void Set( const ChString& strHost, chint16 sPort,
							const ChWorldType& type )
				{
					m_strHost = strHost;
					m_iPort = sPort;
					m_type = type;
				}
											// Dialog Data
		//{{AFX_DATA(ChShortcutWizServer)
		enum { IDD = IDD_SHORTCUT_WIZ_SERVER };
		CEdit	m_editPort;
		CEdit	m_editHost;
		CComboBox	m_comboTypes;
		CString	m_strType;
		CString	m_strHost;
		int		m_iPort;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizServer)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChShortcutWizServer)
		//}}AFX_MSG

	protected:
		ChWorldType		m_type;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChShortcutWizUsername class
----------------------------------------------------------------------------*/

class ChShortcutWizUsername : public ChWizardPage
{
	DECLARE_DYNCREATE( ChShortcutWizUsername )

	public:
		enum { constUsernameLimit = 30, constPasswordLimit = 30 };

	public:
		ChShortcutWizUsername();
		~ChShortcutWizUsername();

		virtual BOOL OnInitPage();			/* Called when this page gets
												the focus */

		inline void GetAccount( ChString& strUsername, ChString& strPassword,
								ChLoginType& loginType )
				{
					strUsername = m_strUsername;
					strPassword = m_strPassword;
					loginType = m_loginType;
				}
		inline void Set( const ChString& strUsername, const ChString& strPassword,
							ChLoginType loginType )
				{
					m_strUsername = strUsername;
					m_strPassword = strPassword;
					m_loginType = loginType;
				}
											// Dialog Data
		//{{AFX_DATA(ChShortcutWizUsername)
		enum { IDD = IDD_SHORTCUT_WIZ_USERNAME };
		CButton	m_staticLoginStyle;
		CButton	m_radioMudLogin;
		CButton	m_radioConnectLogin;
		CEdit	m_editPassword;
		CEdit	m_editUsername;
		CString	m_strPassword;
		CString	m_strUsername;
		int		m_iLoginStyle;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizUsername)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChShortcutWizUsername)
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		//}}AFX_MSG

	protected:
		ChWorldType		m_type;
		ChLoginType		m_loginType;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChShortcutWizOptions class
----------------------------------------------------------------------------*/

class ChShortcutWizOptions : public ChWizardPage
{
	DECLARE_DYNCREATE( ChShortcutWizOptions )

	public:
		ChShortcutWizOptions();
		~ChShortcutWizOptions();

		inline const ChString& GetGroupName() { return m_strGroup; }

		virtual BOOL OnInitPage();			/* Called when this page gets
												the focus */

											// Dialog Data
		//{{AFX_DATA(ChShortcutWizOptions)
		enum { IDD = IDD_SHORTCUT_WIZ_OPTIONS };
		CComboBox	m_comboShortcutFolder;
		CStatic	m_staticShortcutMsg;
		CString	m_strGroup;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizOptions)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void SetupForProgman();

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChShortcutWizOptions)
			// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChShortcutWizFinish class
----------------------------------------------------------------------------*/

class ChShortcutWizFinish : public ChWizardPage
{
	DECLARE_DYNCREATE( ChShortcutWizFinish )

	public:
		ChShortcutWizFinish();
		~ChShortcutWizFinish();

		virtual BOOL OnInitPage();			/* Called when this page gets
												the focus */

											// Dialog Data
		//{{AFX_DATA(ChShortcutWizFinish)
		enum { IDD = IDD_SHORTCUT_WIZ_FINISH };
		CStatic	m_staticFinishSysMsg;
		CStatic	m_staticFinishMsg;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizFinish)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChShortcutWizFinish)
			// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChShortcutWizard class
----------------------------------------------------------------------------*/

class ChShortcutWizard : public ChWizard
{
	DECLARE_DYNAMIC( ChShortcutWizard )

	public:
		ChShortcutWizard( CWnd* pParentWnd = 0 );
		virtual ~ChShortcutWizard();

		inline void GetAccount( ChString& strUsername, ChString& strPassword,
								ChLoginType& loginType )
				{
					m_usernamePage.GetAccount( strUsername, strPassword,
												loginType );
				}
		inline void GetData( ChString& strGroupName, ChString& strName )
				{
					strGroupName = m_optionsPage.GetGroupName();
					strName = m_namePage.GetName();
				}
		inline void GetData( ChString& strPath, ChString& strFilepath,
								ChString& strGroupName, ChString& strName )
				{
					strPath = m_namePage.GetPath();
					strFilepath = m_namePage.GetFilePath();
					strGroupName = m_optionsPage.GetGroupName();
					strName = m_namePage.GetName();
				}
		inline void GetServer( ChString& strHost, chint16& sPort,
									ChWorldType& type )
				{
					m_serverPage.Get( strHost, sPort, type );
				}

		inline void SetAccount( const ChString& strUsername,
								const ChString& strPassword,
								ChLoginType loginType )
				{
					m_usernamePage.Set( strUsername, strPassword, loginType );
				}
		inline void SetName( const ChString& strName )
				{
					m_namePage.SetName( strName );
				}
		inline void SetPath( const ChString& strPath )
				{
					m_namePage.SetPath( strPath );
				}
		inline void SetServer( const ChString& strHost, chint16 sPort,
								const ChWorldType& type )
				{
					m_serverPage.Set( strHost, sPort, type );
				}
		inline bool UseCurrWorld() { return m_boolUseCurrWorld; }
		inline bool EnableUseCurrWorld() { return m_boolEnableCurrWorld; }
		inline void SetUseCurrWorld( bool boolUseCurrWorld = true )
				{
					m_boolEnableCurrWorld =
						m_boolUseCurrWorld = boolUseCurrWorld;
				}

		virtual chint32 OnBack();
		virtual chint32 OnNext();
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChShortcutWizard)
		//}}AFX_VIRTUAL

	protected:
		void AddPages();

	protected:
		//{{AFX_MSG(ChShortcutWizard)
		//}}AFX_MSG

	protected:
		ChShortcutWizName		m_namePage;
		ChShortcutWizServer		m_serverPage;
		ChShortcutWizUsername	m_usernamePage;
		ChShortcutWizOptions	m_optionsPage;
		ChShortcutWizFinish		m_finishPage;

		bool					m_boolUseCurrWorld;
		bool					m_boolEnableCurrWorld;

		DECLARE_MESSAGE_MAP()
};


#endif	// !defined( _CHSCWIZ_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
