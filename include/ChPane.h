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

	This file contains the interface for the ChPaneManager class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHPANE_H )
#define _CHPANE_H


/*----------------------------------------------------------------------------
	Includes
----------------------------------------------------------------------------*/

#include <ChList.h>
#include <ChWnd.h>

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif


/*----------------------------------------------------------------------------
	Forward class definitions
----------------------------------------------------------------------------*/

class ChCore;
class ChPane;
class ChPaneMgr;
class ChSplitter;


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

const chflag32	paneOverlapped	= (1L << 0);
const chflag32	paneFloat		= (1L << 1);
const chflag32	paneInternal	= (1L << 2);
const chflag32	paneSizeable	= (1L << 3);
const chflag32	paneCloseable	= (1L << 4);
const chflag32	paneSmallTitle	= (1L << 5);
const chflag32	paneNoScroll	= (1L << 6);
const chflag32	paneAlignTop	= (1L << 7);
const chflag32	paneAlignBottom	= (1L << 8);
const chflag32	paneSizeToFit	= (1L << 9);
const chflag32	panePersistent	= (1L << 10);
const chflag32	paneForce		= (1L << 11);
const chflag32	paneFrame		= (1L << 30);

const chflag32	defPaneOptions	= (paneOverlapped | paneSizeable |
									paneCloseable);


#if defined( CH_MSW )

/*----------------------------------------------------------------------------
	ChPaneWndMethods class

		These methods must be added (via multiple inheritance) to windows
		that are children of ChPane objects.
----------------------------------------------------------------------------*/

class ChPaneWndMethods
{
	public:
		virtual void GetIdealSize( ChSize& size ) = 0;

											/* The following method should
												return true if the child
												should be automatically sized
												to fit the frame, and false
												otherwise */

		virtual bool OnFrameSize( int iNewWidth, int iNewHeight )
						{
							return true;
						}
											/* The following method is called
												when the frame is closed.  It
												should return TRUE to allow the
												close to continue. */

		virtual bool OnFrameClose() { return true; }

											/* The following method is called
												when the frame is taken over
												by another module */

		virtual void OnFrameDisconnect( const ChModuleID& idNewModule ) {}
};


/*----------------------------------------------------------------------------
	ChPaneFrame class
----------------------------------------------------------------------------*/

class ChPaneFrame : public CWnd
{
	public:
		ChPaneFrame( ChPaneMgr* pPaneMgr, ChPane* pPane );
		virtual ~ChPaneFrame();

		void SizeChild( CRect& rtClient );

	public:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPaneFrame)
		//}}AFX_VIRTUAL

	protected:
		inline ChPane* GetPane() { return m_pPane; }
		inline ChPaneMgr* GetPaneMgr() { return m_pPaneMgr; }

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPaneFrame)
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnClose();
		afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
		afx_msg void OnDestroy();
		afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
		afx_msg BOOL OnQueryNewPalette();
	//}}AFX_MSG
		afx_msg LONG OnCtlBkColor( WPARAM wParam, LPARAM lParam );

		DECLARE_MESSAGE_MAP()

	protected:
		ChPane*		m_pPane;
		ChPaneMgr*	m_pPaneMgr;
		int			m_iFrameWidth;			// Amount frame adds to width
		int			m_iFrameHeight;			// Amount frame adds to height
};


/*----------------------------------------------------------------------------
	ChPaneMiniFrame class
----------------------------------------------------------------------------*/

class ChPaneMiniFrame : public CMiniFrameWnd
{
	public:
		ChPaneMiniFrame( ChPaneMgr* pPaneMgr, ChPane* pPane );
		virtual ~ChPaneMiniFrame();

		void SizeChild( CRect& rtClient );

	public:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPaneMiniFrame)
		//}}AFX_VIRTUAL

	protected:
		inline ChPane* GetPane() { return m_pPane; }
		inline ChPaneMgr* GetPaneMgr() { return m_pPaneMgr; }
		inline HICON GetIcon() { return m_hIcon; }

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPaneMiniFrame)
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnClose();
		afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
		afx_msg void OnDestroy();
		//}}AFX_MSG
		afx_msg LONG OnCtlBkColor( WPARAM wParam, LPARAM lParam );

		DECLARE_MESSAGE_MAP()

	protected:
		ChPane*		m_pPane;
		ChPaneMgr*	m_pPaneMgr;
		int			m_iFrameWidth;			// Amount frame adds to width
		int			m_iFrameHeight;			// Amount frame adds to height
		HICON		m_hIcon;				// Icon for this frame
};

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChPaneWindowClass class
----------------------------------------------------------------------------*/

class ChPaneWindowClass : public ChWnd
{
	public:
		ChPaneWindowClass( const ChString& strName, bool boolCloseable,
							chuint16 suIconID );
		virtual ~ChPaneWindowClass() {}

		inline chuint16 GetIconID() { return m_suIconID; }
		inline const ChString& GetName() { return m_strName; }
		inline bool IsCloseable() { return m_boolCloseable; }

	protected:
		ChString		m_strName;
		bool		m_boolCloseable;
		chuint16	m_suIconID;
};


/*----------------------------------------------------------------------------
	ChPane class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPane
{
	friend class ChPaneMgr;
	friend class ChPaneFrame;
	friend class ChPaneMiniFrame;
	friend class ChPaneBannerFrame;
	friend void ChDestructHelper( ChPane* pItem );

	public:
		typedef enum { paneNormal, paneText, paneGraphic } PaneType;
		typedef enum { paneOpen, paneClose } ChPaneCmd;

	public:
		inline ChModuleID GetModuleID() const { return m_idModule; }
		inline chint16 GetMinHeight() const { return m_sMinHeight; }
		inline chint16 GetMinWidth() const { return m_sMinWidth; }
		inline const ChString& GetName() const { return m_strName; }
		inline chflag32 GetOptions() const { return m_flOptions; }
		inline ChPaneMgr* GetPaneMgr() const { return m_pPaneMgr; }
		inline const ChString& GetTitle() const { return m_strTitle; }
		inline PaneType GetType() const { return m_type; }
		inline chparam GetUserData() const { return m_userData; }
		inline ChWnd* GetWindow() const { return m_pChild; }
		inline bool IsNamed() const { return m_boolNamed; }
		inline bool IsShown() const { return m_boolShown; }
		inline bool IsSized() const { return m_boolSized; }
		inline bool IsSizeable() const { return !!(GetOptions() & paneSizeable); }
		inline bool IsNoScroll() const { return !!(GetOptions() & paneNoScroll); }
		inline bool IsSizeToFit() const { return !!(GetOptions() & paneSizeToFit); }
		inline bool IsPersistent() const { return !!(GetOptions() & panePersistent); }

		inline bool OnFrameSize( int iNewWidth, int iNewHeight )
				{
					if (m_pChildMethods)
					{
						return m_pChildMethods->OnFrameSize( iNewWidth,
																iNewHeight );
					}
					else
					{
						return true;
					}
				}

		inline bool OnFrameClose()
				{
					if (m_pChildMethods)
					{
						return m_pChildMethods->OnFrameClose( );
					}
					else
					{
						return true;
					}
				}

		inline void OnFrameDisconnect( const ChModuleID& idNewModule )
				{
					if (m_pChildMethods)
					{
						m_pChildMethods->OnFrameDisconnect( idNewModule );
					}
				}

		bool IsInternal() const
				{
					return !!(GetOptions() & (paneInternal | paneFrame));
				}
		
		ChSplitter* GetSplitter(); 

		ChWnd* GetFrameWnd();
		void GetSize( chint16& sWidth, chint16& sHeight );

		ChPane* SetOwner( const ChModuleID& idModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods,
							chparam userData );
		ChPane* SetOwner( const ChModuleID& idModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods );
		ChPane* SetSize( chint16 sWidth, chint16 sHeight );
		ChPane* SetTitle( const ChString& strTitle );
		ChPane* Show( bool boolShow = true );
		ChPane* SizeToFit();

		void GetSizePrefs( chint16& sIdealWidth, chint16& sIdealHeight,
							chint16& sMinWidth, chint16& sMinHeight );
		void SetSizePrefs( chint16 sIdealWidth, chint16 sIdealHeight,
							chint16 sMinWidth, chint16 sMinHeight );

		#if defined( CH_MSW )

		static bool GetPaneCmdAttr( const char* pstrArgs,
									ChPaneCmd& paneCommand );

		#else	// defined( CH_MSW )

		static bool GetPaneCmdAttr( const char* pstrArgs,
									ChPaneCmd& paneCommand )
						{
							cerr << "GetPaneCmdAttr" << endl;
						}
		
		#endif	// defined( CH_MSW )
		
		static bool GetPaneOptionsAttr( const char* pstrArgs,
										chflag32& flOptions );
		static bool GetPaneNameAttr( const char* pstrArgs,
										ChString& strPaneName );
		static void GetPaneSizeAttrs( const char* pstrArgs, chint16& sWidth,
										chint16& sHeight, chint16& sMinWidth,
										chint16& sMinHeight );
		static void GetPaneBorderAttrs( const char* pstrArgs, chint16& sHSpace,
										chint16& sVSpace );

		#if defined( CH_MSW )

		static bool GetPaneAttrs( const char* pstrArgs, ChPaneCmd& paneCommand,
									ChString& strPaneName, ChString& strPaneTitle,
									chflag32& flOptions );

		#else	// defined( CH_MSW )

		static bool GetPaneAttrs( const char* pstrArgs, ChPaneCmd& paneCommand,
									ChString& strPaneName, ChString& strPaneTitle,
									chflag32& flOptions )
						{
							cerr << "GetPaneAttrs()" << endl;
						}
		
		#endif	// defined( CH_MSW )

		void OnFrameDestroy( const ChRect& rtFrame );
		
	protected:
		ChPane( const ChString& strName, ChPaneMgr* pPaneMgr,
				chint16 sIdealWidth, chint16 sIdealHeight, chparam userData,
				chflag32 flOptions );
		ChPane( PaneType type, const ChString& strName, ChPaneMgr* pPaneMgr );
		~ChPane();

		inline void FrameDestroyed() { m_pFrame = 0; }

		void Construct();
		void OnInitialShow();
		void MakeNewOwner( const ChModuleID& idModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods,
							bool boolChangeUserData = false,
							chparam userData = 0 );
		void CreateNewFrame();
		chuint16 GetPaneIconID();
		bool ProcessClose();

	protected:
		PaneType			m_type;
		ChPaneMgr*			m_pPaneMgr;

		ChString				m_strName;
		ChString				m_strTitle;
		chparam				m_userData;

		chflag32			m_flOptions;

		bool				m_boolHasBeenShown;
		bool				m_boolShown;
		bool				m_boolNamed;
		bool				m_boolSized;	/* Set to true the first time this
												pane is sized */

		ChWnd*				m_pFrame;
		ChWnd*				m_pChild;
		ChPaneWndMethods*	m_pChildMethods;
		ChModuleID			m_idModule;

		chint16				m_sLeft;
		chint16				m_sTop;
		chint16				m_sWidth;
		chint16				m_sHeight;

		chint16				m_sIdealWidth;
		chint16				m_sIdealHeight;
		chint16				m_sMinWidth;
		chint16				m_sMinHeight;

	private:
		static chuint32		m_luPaneNumber;

		static int			m_iScreenWidth;
		static int			m_iScreenHeight;
};


/*----------------------------------------------------------------------------
	ChPaneMgr list destruct helpers
----------------------------------------------------------------------------*/

static void ChDestructHelper( ChPane* pItem )
{
	delete pItem;
}


static void ChDestructHelper( ChPaneWindowClass* pItem )
{
	delete pItem;
}


/*----------------------------------------------------------------------------
	ChPaneMgr class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPaneMgr
{
	friend class ChPane;

	public:
		static const char *strTextName;
		static const char *strGraphicName;

	public:
		ChPaneMgr( ChCore* pCore );
		~ChPaneMgr();

		inline ChCore* GetCore() const { return m_pCore; }

		ChPane* FindPane( const ChString& strName );
		ChPane* CreatePane( const ChString& strName, chparam userData,
							chint16 sIdealWidth, chint16 sIdealHeight,
							chflag32 flOptions );
		void DestroyPane( const ChString& strName );
		void ShowAllPanes( const ChModuleID& idModule, bool boolShow );
		void DestroyAllPanes( const ChModuleID& idModule,
								bool boolDestroyFramePanes = true );
		void RecalcLayout( ChPane* pChangingPane );

	protected:
		ChString FindClassName( bool boolCloseable, chuint16 suIconID );
		ChString RegisterPaneClass( bool boolCloseable, chuint16 suIconID );

	protected:
		ChPtrList<ChPane>				m_list;
		ChPtrList<ChPaneWindowClass>	m_wndClassList;

	private:
		ChCore*							m_pCore;
		static chuint32					m_luClassNumber;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR
#endif

// $Log$

#endif	// !defined( _CHPANE_H )
