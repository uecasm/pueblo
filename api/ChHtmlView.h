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

	This file consists of the interface for the ChHtmlView view class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHHTMLVW_H )
#define _CHHTMLVW_H


#if defined( CH_MSW ) && !defined( NO_TEMPLATES )

	#include <afxtempl.h>

#endif	// defined( CH_MSW )

#if defined( CH_UNIX ) && !defined( HINSTANCE )

#define HINSTANCE	void*

#endif	// defined( CH_UNIX ) && !defined( HINSTANCE )

#include <ChTxtWnd.h>
#include <ChSplay.h>
#include <ChList.h>
#include <ChPane.h>

#include "ChHtmFrm.h"


/*----------------------------------------------------------------------------
	External classes
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif

class ChHtmlWnd;
class ChHtmlStyle;
class ChHTMLPrefetch;
class ChPlugInMgr;
class ChHtmlSettings;
class ChHTTPConn;
class ChHTMLStreamManager;
class ChPlugInStream;
class ChHtmlPane;

#if !defined( NO_TEMPLATES )

typedef ChPtrList<char>	ChHTMLCmdString;
typedef ChPtrList<ChHTMLForm>	ChHTMLFormLst;
typedef ChPtrList<ChHTMLPrefetch>	ChHTMLPrefetchLst;
typedef ChSplay< ChString, ChInlineImageData*> ChHTMLImageList;

#else
#include <TemplCls\ChPstrLs.h>
#include <TemplCls\ChHtFmLs.h>
#include <TemplCls\ChHtFmLs.inl>
#include <TemplCls\ChFechLs.h>
#include <TemplCls\ChFechLs.inl>
#include <TemplCls\ChHtmSpl.h>
typedef ChPtrStringList	ChHTMLCmdString;
typedef ChPtrHtmlFormList	ChHTMLFormLst;
typedef ChPtrHtmlPrefetchList	ChHTMLPrefetchLst;
#endif


#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	Typedefs
----------------------------------------------------------------------------*/

class 	CH_EXPORT_CLASS ChHTMLPrefetch
{
	public :
		ChHTMLPrefetch();
		~ChHTMLPrefetch();
		const char * GetArgs()			{ return 	m_pstrArg; }
		const char * GetHREF()			{ return 	m_pstrhRef; }
		int   GetProbablity() 			{ return  	m_iProbablity; }
		void  SetArg( char * pstrArg )	{ m_pstrArg = pstrArg; }
		void  SetHREF( char * pstrRef )	{ m_pstrhRef = pstrRef; }
		void  SetProbablity( int iProb ){ m_iProbablity = iProb; }
	private :
		char* 	m_pstrArg;
		char* 	m_pstrhRef;
		int 	m_iProbablity;
};


/*----------------------------------------------------------------------------
	ChHtmlView class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHtmlView : public ChTxtWnd, public ChPaneWndMethods
{
	friend class   ChHtmlParser;			// To access the HTML tag table
	friend class   ChHtmlTag;	   			// To access the HTML tag table
	friend class   ChHTMLForm;	   			// To access the HTML tag table
	friend class   ChHtmlWnd;

	public:
											// Options for HTML

		enum tagCursors { cursorNormal = 0, cursorText, cursorHotspot,
							maxCursors };

	public:
		ChHtmlView( const char* pstrName, ChHtmlWnd* pFrameMgr );
		virtual ~ChHtmlView();

		#if defined( CH_MSW )

		virtual BOOL Create( const CRect& rtView, CWnd* pParent,
								DWORD dwStyle = WS_VISIBLE | WS_BORDER | WS_VSCROLL,
								UINT uiID = 0 );
		virtual BOOL CreateEx( const CRect& rtView, CWnd* pParent,
								DWORD dwStyle = WS_VISIBLE | WS_BORDER | WS_VSCROLL,
								DWORD dwStyleEx = 0, UINT uiID = 0 );

		#endif	// defined( CH_MSW )
											// ChPaneWndMethods overides

		virtual void GetIdealSize( ChSize& size );
		virtual void OnFrameDisconnect( const ChModuleID& idNewModule );

	public:
		bool NewPage();						/* Clears the contents of the
												HTML view */

		void AppendText( const char* pstrNewText, chint32 lNewCount, chuint32 flOptions );

		bool DisplayFile( const char* pstrFile,	const char * pstrMimeType,
											chuint32 flOptions,
											const ChString& strURL );
		void DisplayResource( int iResID,	chuint32 flOptions,	HINSTANCE hModule );

		bool PrefetchedLink( ChString &strLink );

		ChHTMLForm* GetCurrentForm();
		ChHTMLFormLst* GetFormList();

		void CreateControl( ChCtrlList& ctrlObj,
							chuint32 luAttr = objAttrMiddle,
							bool bbChecked = false );

		inline ChHTMLCmdString& GetAllocList() { return m_cmdList; }
		inline ChHTMLPrefetchLst& GetPrefetchList() { return m_prefetchList; }
		inline ChHTMLImageList& GetImageList() { return m_htmlImageList; }
		inline const ChString& GetAnchorTarget() { return m_strTargetWindowName; }
		inline void SetAnchorTarget( const ChString& strTarget )
						{
							m_strTargetWindowName = strTarget;
						}

		inline const ChString& GetFrameName() { return m_strFrameName; }
		inline ChHtmlWnd* GetFrameMgr() { return m_pFrameMgr; }

		inline const ChString& GetDocURL() { return m_strDocURL; }

		inline const ChString& GetDocBaseURL()
						{
							return m_strDocBaseURL.IsEmpty() ? m_strDocURL :
																m_strDocBaseURL;
						}
		inline const ChString& GetDocTitle() { return m_strDocTitle; }

		inline chuint32 GetPageNumber() { return m_uPageNumber; }

		int GetCurrentIndent();
		void SetPageIndents( const ChRect& viewIndent )
						{
							m_pageIndent = viewIndent;
							SetViewIndents( viewIndent     ); 
						}
		ChHtmlSettings*	GetSettings();
		void ClearForms();
		void AbortFormatting();

		void SpawnPlugInRequest( const ChString& strURL, 
						ChPlugInStream* pStream, const char* pstrWindow );
		void CreatePane( ChHtmlPane* pPane );

		chint32 RedirectStream( const char* pstrBuffer, chint32 lStart, chint32 lCount );

		void UnloadPlugins();
		void UnloadImages();

		void RemapColors( int iNumColors, 
						chuint32* pluOldColor, chuint32* pluNewColors );

		virtual void OnMouseUp();

		void ChangeCursor(tagCursors newCursor);		// UE

	protected :

		void			SetDocURL( const ChString& strURL )			 
										{ m_strDocURL = strURL; }
		void			SetDocBaseURL( const char* pstrURL )			 
										{ m_strDocBaseURL = pstrURL; }

		void			SetDocumentTitle( const ChString& strTitle )			 
													{ m_strDocTitle = strTitle; }
		void AppendUntranslated( const char* pstrNewText, chint32 lNewCount );
		bool DisplayImage( ChDib *pDib,
							chuint32 flOptions,
							const ChString& strURL);

		bool DisplayUnknown( const char* pstrFile,	chuint32 flOptions,
								const ChString& strURL, const ChString& strMimeType );


	protected:
		//{{AFX_VIRTUAL(ChHtmlView)
		protected:
		//}}AFX_VIRTUAL		 

		enum tagEmbedMode { embedInternal = 1, embedFloat, embedDetached };

		ChPlugInMgr*	GetPlugInMgr();

		void SetEmbedMode( int iMode )				{ m_iEmbedMode = iMode; }

		void LoadInlineImage( const ChString& strURL, ChObjInline *pInLine );
		void LoadBkPattern( const ChString& strURL );

		bool NotifyInline( const char *pstrBuf );

		void NotifySubmitForm( const ChString& strAction, const ChString& strMD5,
														const ChString& strData );


		// This method is called by the HTML parser when it detects the prefetch tag
		virtual void OnPrefetch( ChHTMLPrefetch* pPreFetch, 
											const ChString& strDocURL   );


	protected:
		
		inline char GetLastChar()
			{
				if ( ChTxtWnd::GetTextCount() )
				{
					return ChTxtWnd::GetTextBuffer()[ChTxtWnd::GetTextCount() - 1];
				}
				return 0;  // return NULL char
			}


		inline bool IsAborted()					{ return m_boolAbortFormatting; }


	private:

		void InitHtmlView();
											// core methods for HTML view
		bool GetRedirectInfo( chint32& lRedirect )		
								{ 
									lRedirect = m_lPosRedirect;
									return 	m_boolRedirect;
								}
		void SetRedirectInfo( bool boolRedirect, chint32& lRedirect )		
								{ 
									m_lPosRedirect = lRedirect;
									m_boolRedirect = boolRedirect;
								}

		bool CalcDisplayAppend();
											// Accessor functions
		void UpdateTextColor(  chuint32 luOldTextColor,		chuint32 luOldLinkColor,
								chuint32 luOldVLinkColor, chuint32 luOldPrefetchColor  );
		void UpdateFontChange(  const ChString& strOldProportional, int iOldProportionalSize,
								const ChString& strOldFixed, int iOldFixedSize );
		void UpdateColorChange( chuint32   luOldTextColor, chuint32 luOldLinkColor, 
								chuint32 luOldVLinkColor, chuint32 luOldPrefetchColor, 
								chuint32 luBackColor );



	private:
		ChHtmlWnd*						m_pFrameMgr;	 		// Frame manager
		ChHTMLCmdString					m_cmdList;				// Allocation list
		ChHTMLImageList					m_htmlImageList;		// All unique inline images
		HCURSOR							m_hCursor; 				// current cursor
		ChHTMLFormLst*					m_pformLst;				// HTML forms
		ChHTMLPrefetchLst				m_prefetchList;			// HTML prefetch list
		ChString							m_strDocURL;			// current document URL 
		ChString							m_strDocBaseURL;		// Base URL for current document URL 
		ChString							m_strDocTitle;			// Title of the document
		ChString							m_strTargetWindowName;  // name of the target window for anchors
		ChString							m_strFrameName;			// Name of this window
		chuint32 						m_uPageNumber;			// page number
		bool							m_boolAbortFormatting;	// Stop processing current file
		bool							m_boolFormatting;		// formatting in progress ?
		bool							m_boolDeleteParser;		// true if parser created by me

		ChPlugInMgr*					m_pluginMgr;				// Netscape plugin manager
		static	HCURSOR					m_hHTMLCursors[maxCursors];	// HTML cursor
		ChHtmlParser*					m_pParser;					// Parser
		bool							m_boolRedirect;
		chint32							m_lPosRedirect;
		int								m_iEmbedMode;  
		ChRect							m_pageIndent;


	protected :

		#if defined( CH_MSW )

		//{{AFX_MSG(ChHtmlView)
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, ChPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

		#elif defined( CH_UNIX )

		void OnLButtonUp( chuint32 nFlags, ChPoint& point );

		#endif	// defined( CH_UNIX )



};
#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR
#endif

#endif // _CHHTMLVW_H
// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
