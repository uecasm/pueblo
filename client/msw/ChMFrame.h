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

	This file consists of the the interface for the ChMainFrame class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHMFRAME_H ))
#define _CHMFRAME_H

#ifdef CH_MSW
#include <ChSplit.h>
#include <ChPerFrm.h>
#include <ChTlBar.h>
#endif

#include <ChTime.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


class ChClientCore;

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define ID_TENTH_SECOND_TIMER		100
#define TENTH_SECOND_TIMER_DURATION	100
#define ID_SECOND_TIMER				1000
#define ID_MINUTE_TIMER				6000

#define CH_TB_IDX_ORIENTATION	6
#define CH_TB_IMAGE_LAST		8			/* This is the the index of the
												last commonly-displayed
												bitmap */
#define CH_TB_IMAGE_HORIZONTAL	4
#define CH_TB_IMAGE_VERTICAL	(CH_TB_IMAGE_LAST + 1)

/*----------------------------------------------------------------------------
	ChMainFrame class
----------------------------------------------------------------------------*/

class ChMainFrame : public ChPersistentFrame
{
	#if defined( CH_MSW )

	DECLARE_DYNCREATE( ChMainFrame )

	#endif	// defined( CH_MSW )
		 
	#if defined( CH_MSW )
	protected:								// create from serialization only
	#else	// defined( CH_MSW )
	public:
	#endif	// defined( CH_MSW )
		ChMainFrame();

	public:
		virtual ~ChMainFrame();

		inline ChClientCore* GetPuebloCore() const { return m_pCore; }
		inline ChSplitter* 	 GetSplitter() { return &m_splitter; }
		inline ChToolBar*	 GetToolBar() { return &m_wndToolBar; }
		inline CStatusBar*	 GetStatusBar() { return &m_wndStatusBar; }

		void OnSecondTick( time_t timeCurr );

		#if defined( CH_MSW )
			#if defined( _DEBUG )
				virtual void AssertValid() const;
				virtual void Dump( CDumpContext& dc ) const;
			#endif	// defined( _DEBUG )
		#endif	// defined( CH_MSW )

		virtual void GetMessageString( UINT nID, ChString& rMessage ) const;

		#if defined( CH_MSW )

		ChFont*		GetAppFont()				{ return m_pAppFont; }

		#endif	// defined( CH_MSW )

	public:
		#if defined( CH_MSW )
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChMainFrame)
		public:
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			virtual void ActivateFrame(int nCmdShow = -1);       
		//}}AFX_VIRTUAL

		virtual BOOL OnCreateClient( LPCREATESTRUCT pCreate,
										CCreateContext* pContext );
		#endif	// defined( CH_MSW )
	
	protected:
		void InitStatusBar();
		void DrawProgressBar();
		void UpdateOrientationUI();
		void ClearStatus( );


	private:
		static ChString	m_strClass;

		ChClientCore*	m_pCore;				// Client core object

		ChTime			m_secondStartTime;

		#if defined( CH_MSW )
											// control bar embedded members
		CStatusBar		m_wndStatusBar;
		ChToolBar	    m_wndToolBar;
		ChSplitter		m_splitter;

		ChFont*			m_pAppFont;

		#endif // defined( CH_ARCH_16 )

		bool			m_boolInShutdown;	// This frame is being destroyed
		bool			m_boolInProgress;
		#if defined( CH_MSW )
		static CBrush	m_brClearProgress;
		static CBrush	m_brShowProgress; 
		#endif

	#if defined( CH_MSW )
											// Generated message map functions
	protected:
		//{{AFX_MSG(ChMainFrame)
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnEditCopy();
		afx_msg void OnEditCut();
		afx_msg void OnEditPaste();
		afx_msg void OnEditClear();
		afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
		afx_msg BOOL OnQueryNewPalette();
		afx_msg void OnClose();
		afx_msg void OnViewPrevious();
		afx_msg void OnViewSwapPanes();
		afx_msg void OnUpdateViewSwapPanes(CCmdUI* pCmdUI);
		afx_msg void OnViewToggleOrientation();
		afx_msg void OnUpdateViewToggleOrientation(CCmdUI* pCmdUI);
		afx_msg void OnMenuHelpPuebloBugReport();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnUpdateViewStopLoading(CCmdUI* pCmdUI);
		afx_msg void OnViewStopLoading();
		afx_msg void OnHelpAboutPueblo();
		afx_msg void OnHelpAboutChaco();
		afx_msg void OnHelpMorePueblo();
		//afx_msg void OnHelpAboutVrscout();
		afx_msg void OnHelpAboutUE();
		afx_msg void OnEditPreferences();
		afx_msg void OnHelpIndex();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnCloseWindow();
		afx_msg void OnHelpRegisterPueblo();
	afx_msg void OnHelpPuebloWorldListEntry();
	afx_msg void OnViewTrace();
	afx_msg void OnUpdateEditPreferences(CCmdUI* pCmdUI);
	//}}AFX_MSG

		afx_msg LONG OnAsyncDispatch( UINT wParam, LONG lParam );

		afx_msg void OnPuebloMenuCommand(UINT uID);
		afx_msg void OnPuebloMenuUpdateCommand(CCmdUI* pCmdUI);
		afx_msg void OnUpdateStatusText(CCmdUI* pCmdUI);

		DECLARE_MESSAGE_MAP()

	#else	// defined( CH_MSW )
  
	public:
	    void OnClose(void);
		void OnTimer(UINT nIDEvent);
		int OnCreate(LPCREATESTRUCT lpCreateStruct);

	#endif	// 	defined( CH_MSW )
};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR 
#endif	// defined( CH_MSW ) && defined( CH_ARCH_16 )


#endif	// !defined( _CHMFRAME_H )


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
