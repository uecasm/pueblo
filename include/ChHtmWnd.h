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

	This file consists of the interface for the ChHtmlWnd view class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHHTMLWND_H )
#define _CHHTMLWND_H


#if defined( CH_MSW ) && !defined( NO_TEMPLATES )

	#include <afxtempl.h>

#endif	// defined( CH_MSW )


#include <ChList.h>

#if defined( CH_UNIX ) && !defined( HINSTANCE )

#define HINSTANCE	void*

#endif	// defined( CH_UNIX ) && !defined( HINSTANCE )



/*----------------------------------------------------------------------------
	External classes
----------------------------------------------------------------------------*/

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

//#define HTML_RES_TYPE		"HTML"
#define HTML_RES_TYPE		RT_HTML

#define TOP_WINDOW			TEXT( "_top" )
#define SELF_WINDOW			TEXT( "_self" )
#define CURRENT_WINDOW		TEXT( "_current" )
#define PREVIOUS_WINDOW		TEXT( "_previous" )
#define BLANK_WINDOW		TEXT( "_blank" )

#define WEBTRACKER_WINDOW	TEXT( "_WebTracker" )

#define ATTR_TARGET		"target"
#define ATTR_HREF		"href"
#define ATTR_SRC		"src"
#define ATTR_PANE		"xch_pane"
#define ATTR_PANE_NAME	"xch_pane_name"
#define ATTR_PANE_TITLE	"xch_pane_title"


/*----------------------------------------------------------------------------
	Typedefs
----------------------------------------------------------------------------*/

class  ChHTTPConn;
class  ChHtmlReqInfo;
class  ChHtmlSettings;
class  ChDib;
class  ChHTMLStreamManager;
class  ChHtmlView;
class  ChHtmlViewObj;
class  ChHtmlPane;
class  ChRedirectInfo;
class  ChPaneMgr;

typedef ChPtrList<ChHtmlViewObj>	ChHTMLViewList;
typedef ChPtrList<ChRedirectInfo>	ChHTMLRedirectStack;



/*----------------------------------------------------------------------------
	ChHtmlWnd class
----------------------------------------------------------------------------*/
class ChHtmlViewObj
{
	public :
		ChHtmlViewObj( const char* pstrWndName, ChHtmlView* pView, CFrameWnd* pFrame ); 
		ChHtmlViewObj( const char* pstrWndName, ChHtmlView* pView, ChPaneMgr* pPaneMgr ); 
		~ChHtmlViewObj();
	   CFrameWnd* 		GetFrameWnd()		{ return m_pFrameWnd; } 
	   ChHtmlView* 		GetHtmlView()		{ return m_pHtmlView; } 
	   const ChString& 	GetAltFrameName()	{ return m_strAltName; }
	   void  SetDestroy( bool boolDestroy ) { m_boolDestroy = boolDestroy; }
  	   void  SetHtmlView( ChHtmlView* pView ){ m_pHtmlView = pView; } 

	private	:
		CFrameWnd*		m_pFrameWnd;
		ChPaneMgr*		m_pPaneMgr;
		ChHtmlView*		m_pHtmlView;
		ChString			m_strAltName;
		bool			m_boolDestroy;
};

/*----------------------------------------------------------------------------
	ChHtmlWnd class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHtmlWnd : public CWnd
{
	public:
		friend ChHtmlView;
											// Options for HTML

		enum tagConst { fileReplace = 0x01, fileAppend = 0x02, modeUntranslated = 0x04,
						traceError = 0x08, traceWarning = 0x10 };

											// Options for WriteFile method

		enum tagWriteOptions { writeAll = 0x01, writeHTML = 0x02, writeNew = 0x4 };

	public:
		
	#if !defined( CH_PUEBLO_PLUGIN )
		ChHtmlWnd();
	#endif

		ChHtmlWnd( ChHTTPConn *pConn );
		virtual ~ChHtmlWnd();

		#if defined( CH_MSW )

		virtual BOOL Create( const CRect& rtView, CWnd* pParent,
								DWORD dwStyle = WS_VISIBLE | WS_BORDER,
								UINT uiID = 0 );
		virtual BOOL CreateEx( const CRect& rtView, CWnd* pParent,
								DWORD dwStyle = WS_VISIBLE | WS_BORDER,
								DWORD dwStyleEx = 0, UINT uiID = 0 );

		#endif	// defined( CH_MSW )

	public:
		virtual void OnMouseUp( ) {}
											/* Clears the contents of the
												HTML view */

		bool NewPage( const char* pstrWindowName = 0 );
		void AppendText( const char* pstrNewText, chint32 lNewCount = -1,
							chflag32 flOptions = 0,
							const char* pstrWindowName = 0 );
		bool DisplayFile( const char* pstrFile,	const char * pstrMimeType = 0,
							chuint32 flOptions = ChHtmlWnd::fileReplace,
							ChString	 strURL = "", const char* pstrWindowName = 0  );
		void DisplayResource( int iResID,
								chuint32 flOptions = ChHtmlWnd::fileReplace,
								HINSTANCE hModule = 0, const char* pstrWindowName = 0  );

		ChString GetAnchorTarget( const char* pstrWindowName = 0 );
		ChString GetDocURL( const char* pstrWindowName = 0 );
		ChString GetDocBaseURL( const char* pstrWindowName = 0 );
		ChString GetDocTitle( const char* pstrWindowName = 0 );

		void PageUp( const char* pstrWindowName = 0 );
		void PageDown( const char* pstrWindowName = 0 );
		void Home( const char* pstrWindowName = 0 );
		void End( const char* pstrWindowName = 0 );

		bool IsSelectionEnabled( const char* pstrWindowName = 0 );
		void EnableSelection( bool boolSel, const char* pstrWindowName = 0 );
		void GetSel( chint32& lStart, chint32& lEnd, const char* pstrWindowName = 0 );
		void SetSel( chint32 lStart, chint32 lEnd, const char* pstrWindowName = 0 );
		void CopyToClipboard( const char* pstrWindowName = 0 );

		inline chuint32 GetPageID() const { return m_uPageID; }

		void SetAnchorTarget( const ChString& strTarget, const char* pstrWindowName = 0 );

		void SetFocus( const char* pstrWindowName = 0 );

		ChPaneMgr* GetPaneMgr( )					{ return m_pPaneMgr; }
		void SetPaneMgr( ChPaneMgr* pMgr )			{ m_pPaneMgr = pMgr; }

		ChHtmlSettings*	GetSettings() { return m_pSettings; }

		void AbortFormatting();

		bool VisitedURL( const ChString &strURL, const char * pstrDefault = 0 );
		void LoadURL( const ChString& strURL, const char* pstrWindowName = 0 );
		void PostURL( const ChString& strURL, const ChString& strData,
						const char* pstrWindowName = 0 );

		ChDib* LoadDIB( const ChString& strMimeType, const char* strFilename );

		ChHtmlView* CreateHtmlView( const char* pstrName,
									CWnd* pFrame = 0 );

		void CreatePane( ChHtmlView* pView, ChHtmlPane* pPane );
  		void ClosePane( const ChString& strName, bool boolDestroy = true );
		void CloseAllSubPanes();

											// Static utility functions

		static void EscapeForHTML( ChString& strTextOut );
		static bool GetHTMLAttribute( const char* pstrArgs,
										const char* pstrType,
										ChString& strVal );
		static bool GetHTMLHref( const char* pstrArgs, bool boolImage,
									ChString& strVal );



	public:									// ChTxtWnd related

 		chint32 GetBufferLimit( const char* pstrWindowName = 0);
 		bool GetAppend( const char* pstrWindowName = 0);

 		void SetBufferLimit( chint32 lLimit, const char* pstrWindowName = 0 );
		void ShowAppend( bool boolAppend = true, const char* pstrWindowName = 0 );
		
		void GetScrollState( bool& boolHorizontal, bool& boolVertical,
								const char* pstrWindowName = 0 );
		void AllowScroll( bool boolHorizontal, bool boolVertical,
							const char* pstrWindowName = 0 );

		void GetDocumentSize( ChSize& docSize, const char* pstrWindowName = 0 );

		static void RemoveEntities( const char* pstrBuffer,
									ChString& strResult );

		void GetViewIndents( ChRect& viewIndents,
								const char* pstrWindowName = 0 );
		void SetViewIndents( const ChRect& viewIndent,
								const char* pstrWindowName = 0 );

		chuint32 GetDocumentAttrs( const char* pstrWindowName = 0 );
		void SetDocumentAttrs( const chuint32 luAttrs,
								const char* pstrWindowName = 0 );

		bool WriteFile( const char* pstrFilePath, chflag32 flOptions = 0,
							const char* pstrWindowName = 0 );
		bool CloseFile( const char* pstrWindowName = 0 );

		void RemapColors( int iNumColors, 
						chuint32* pluOldColor, chuint32* pluNewColors,
						const char* pstrWindowName = 0  );

	protected:
		//{{AFX_VIRTUAL(ChHtmlWnd)
		protected:
		//}}AFX_VIRTUAL

		// Override this virtual member function if you need
		// notification when user clicks on a hotspot.

		virtual void OnHotSpot( chparam userData, const ChString& strDocURL );
		virtual bool OnSelectHotSpot( int x, int y, ChPoint& ptRel, 
										chparam userData, 
										const ChString& strDocURL );

		// this method is called when the HTML parser detects an inline image
		// in the the document, the data passed to the method is the arguments associated with
		// with the tag buffer

		virtual bool OnInline( const char* pstrArgs, 
										const ChString& strDocURL  );    

		virtual bool OnRedirect( const ChString& strURL, bool boolWebTracker  );    

		// called when a requested URL has been sucessfully loaded
		virtual void OnLoadComplete( const ChString& strFile, const ChString& strURL, 
											const ChString& strMimeType,
											chparam userData );
		// called when load error occurs for user requested URLs only.
		virtual void OnLoadError( chint32 lError, const ChString& strErrMsg, 
								const ChString& strURL,	chparam userData );

		// called when user submits a form with a non-standard method type.
		virtual void OnSubmitForm( const ChString& strCommand, const ChString& strMD5,
												const ChString& strFormData );

		virtual void OnRightMouseUp( const CPoint& point, const ChString& strWindowName )
		{
		}
		virtual void OnRightMouseDown( const CPoint& point, const ChString& strWindowName )
		{
		}

	public :

		virtual void OnTrace( const ChString& strMsg, int iType );
		void PostMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	protected :

 		ChHTTPConn* GetHTTPConn();	 

	protected:
		ChHtmlView* GetDefaultView( );
		ChHtmlView* GetHtmlViewByName( const ChString& strName );
		ChHtmlView* ChHtmlWnd::CreateView( ChHtmlPane* pInfo );



		inline bool IsAborted()					{ return m_boolAbortFormatting; }
		void PrefetchURL( const ChString& strURL, const char * pstrDef = 0 );
		void LoadURL( const ChString& strURL, ChHtmlReqInfo* pReqInfo );
		void PostURL( const ChString& strURL, const ChString& strData, ChHtmlReqInfo* pReqInfo );

		void OnViewHotSpot( ChHtmlView* pView, chparam userData );
		void OnViewSelectHotSpot( ChHtmlView* pView, int x, int y, ChPoint& ptRel, 
										chparam userData );

		// this method is called when the HTML parser detects an inline image
		// in the the document, the data passed to the method is the arguments associated with
		// with the tag buffer

		bool OnViewInline( ChHtmlView* pView, const char* pstrArgs );    

		// called when user submits a form with a non-standard method type.
		void OnViewSubmitForm( ChHtmlView* pView, const ChString& strAction, 
							const ChString& strMD5, const ChString& strFormData );

		bool IsRedirectStream( const ChString& strCurrStream  );
		bool RedirectStream( const ChString& strCurrStream, const ChString& strTargetStream  );
		bool RedirectURL( const ChString& strTarget, const ChString& strURL, chuint32 flOptions );

	private:
		void InitHtmlWnd();

		bool StreamFile( const char* pstrFile, const char* pstrMimeType,
						chuint32 flOptions , const ChString& strURL, ChHtmlView* pView );
		bool StreamText(	const char* pstrNewText, chint32 lNewCount, 
										chuint32 flOptions, ChHtmlView* pView );
		void RecalcLayout();

	private:
		chuint32 						m_uPageID;				// page number
		bool							m_boolAbortFormatting;	// Stop processing current file
		bool							m_boolMyConn;		// true if connection created by me 
		ChHTTPConn*						m_pHttpConnection;		// HTTP connection
		ChHTMLStreamManager*			m_phtmlStreamMgr;		// Stream manager
		ChHtmlSettings*					m_pSettings;			// HTML settings
		ChHTMLViewList					m_htmlViewList;			// List of HTML views
		ChHTMLRedirectStack*			m_pRedirectStack;		// Redirect stack
		ChPaneMgr*						m_pPaneMgr;


	protected :
		#if defined( CH_MSW )

		//{{AFX_MSG(ChHtmlWnd)
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnDestroy();
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

		afx_msg LONG OnFontChange( UINT wParam, LONG lParam );
		afx_msg LONG OnColorChange( UINT wParam, LONG lParam );
		afx_msg LONG OnHTTPNotificaton(UINT wParam, LONG lParam );
		afx_msg LONG OnCtlBkColor( WPARAM wParam, LPARAM lParam );

		DECLARE_MESSAGE_MAP()

		#elif defined( CH_UNIX )

		chuint32 OnFontChange( chuint32 wParam, chuint32 lParam );
		chuint32 OnColorChange( chuint32 wParam, chuint32 lParam );

		#endif	// defined( CH_UNIX )
};


#endif // _CHHTMLWND_H
// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:40  uecasm
// Import of source tree as at version 2.53 release.
//
