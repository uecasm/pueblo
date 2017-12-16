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

	This file consists of interfaces of the ChWizardPage class, used to
	define pages for a wizard.

----------------------------------------------------------------------------*/

#if !defined( _CHWIZARD_H )
#define _CHWIZARD_H


#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define IDBACK		IDRETRY
#define IDNEXT		IDOK
#define IDFINISH	IDOK


/*----------------------------------------------------------------------------
	ChWizardPage class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChWizardPage : public CDialog
{
	friend class ChWizard;

	DECLARE_DYNAMIC( ChWizardPage )

	public:
		ChWizardPage( chint32 lIDTemplate, UINT nIDCaption = 0 );
		virtual ~ChWizardPage();

		inline const ChString& GetCaption() { return m_strCaption; }

		virtual chint32 GetID() const;

		void CancelToClose();				/* Called when the Wizard should
												display close instead of
												cancel */
	public:
		virtual BOOL OnInitPage();			/* Called when this page is
												created.  Return true unless
												the function explicitly sets
												the focus.  If you set the
												focus, return false. */

											/* For the OnBack and OnNext
												methods, the code should return
												true if Next or Back should
												proceed */
		virtual BOOL OnBack();
		virtual BOOL OnNext();

		virtual void OnCancel();			// Cancel pressed

											/* The following functions should
												only be overridden carefully */
		virtual BOOL OnSetActive();
		virtual BOOL OnKillActive();

	public:
		virtual BOOL PreTranslateMessage( MSG* pMsg );

		#if defined( CH_DEBUG )
											/* EndDialog is provided to
												generate an assert if it is
												called */
		void EndDialog( int iEndID );

		#endif	// defined( CH_DEBUG )

	protected:
		void CommonConstruct( char* pstrTemplateName, UINT uiIDCaption );
		void UpdateNameValue( ChString& strBuffer, const ChString& strName, const ChString& strValue );
		void UpdateSysColors();

		BOOL PreTranslateKeyDown( MSG* pMsg );
		BOOL ProcessTab( MSG* pMsg );
		bool CreatePage();
		void LoadCaption();

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChWizardPage)
		virtual BOOL OnInitDialog();
		afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
		afx_msg int OnCreate(LPCREATESTRUCT lpcs);
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void OnClose();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		chint32		m_lTemplate;
		ChString		m_strCaption;

		HBRUSH		m_hbrBtnFace;
		COLORREF	m_colorBtnText;
};


/*----------------------------------------------------------------------------
	ChWizard class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChWizard : public CWnd
{
	friend class ChWizardPage;

	DECLARE_DYNAMIC( ChWizard )

	public:
		ChWizard( chparam resources, int iCaptionID, CWnd* pParent );
		~ChWizard();

		void AddPage( ChWizardPage* pPage );
		int DoModal();
		void EndDialog( int iEndID );
											/* OnPageChanging() should return
												true to allow page switch */
		virtual BOOL OnPageChanging();

		virtual BOOL PreTranslateMessage( MSG* pMsg );
		virtual BOOL DestroyWindow();

		virtual chint32 OnBack();
		virtual chint32 OnNext();
		virtual void OnFinish();

	public:
		inline int GetPageCount() const { return m_pages.GetSize(); }
		inline ChWizardPage* GetPage( int iPage ) const
						{
							return (ChWizardPage*)m_pages[iPage];
						}
		inline chint32 GetPageID( int iPage ) const
						{
							return ((ChWizardPage*)m_pages[iPage])->GetID();
						}
		inline chint32 GetActivePageID() const
						{
							return GetActivePage()->GetID();
						}

	protected:
		inline ChWizardPage* GetActivePage() const
						{
							return GetPage( m_iCurPage );
						}
		inline bool IsHelpEnabled() { return false; }

		BOOL Create( CWnd* pParentWnd = 0,
						DWORD dwStyle = WS_SYSMENU | WS_POPUP | WS_CAPTION |
										DS_MODALFRAME | WS_VISIBLE,
						DWORD dwExStyle = WS_EX_DLGMODALFRAME );

		BOOL PumpMessage();
		BOOL ProcessChars( MSG* pMsg );
		void GotoControl( HWND hWnd, TCHAR ch );
		BOOL ProcessTab( MSG* pMsg );
		void CheckDefaultButton( HWND hwndFocusBefore, HWND hwndFocusAfter );
		void CheckFocusChange();

		bool PageBack();
		bool PageNext();
		bool GotoPage( chint32 lPageID );
		bool SetActivePage( int iPage );
		int FindPage( chint32 lPageID );
		void RecalcLayout();

		bool CreateStandardButtons();
		void UpdateButtons();
		void CancelToClose();

	protected:
											/* The following functions are
												legacy from MSVC 2.2 */

		HWND FindNextControl( HWND hWnd, TCHAR ch );
		HWND GetFirstLevelChild( HWND hWndLevel );

	protected:
		// Generated message map functions
		//{{AFX_MSG(ChWizard)
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnPaint();
		afx_msg void OnBackClick();
		afx_msg void OnNextClick();
		afx_msg void OnCancel();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		static UINT	stdButtons[3];

		HINSTANCE	m_hModule;
		ChString		m_strCaption;

		CWnd*		m_pParentWnd;
		int			m_iCurPage;

		bool		m_boolDrawEdgeAvailable;

		HWND		m_hwndFocus;			// Focus when we lost activation
		HWND		m_hwndLastFocus;		// Tracks last window with focus
		HWND		m_hwndDefault;			/* Current default push button if
												there is one */

		HFONT		m_hFont;				/* Sizes below dependent on this
												font */
		CSize		m_sizeButton;
		CSize		m_sizeTabMargin;
		int			m_cxButtonGap;

		CPtrArray	m_pages;				// Array of ChWizardPage pointers
		bool		m_boolParentDisabled;
		int			m_iID;
};


#endif	// !defined( _CHWIZARD_H )
