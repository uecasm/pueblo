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

	This file contains the interface for ChHtmlPane.

----------------------------------------------------------------------------*/

// $Header$

#if !defined ( CHHTMLPANE_H )
#define CHHTMLPANE_H


#define REDIRECT 				TEXT( "xch_redirect" )

class ChHtmlWnd;
class ChHtmlView;

class ChRedirectInfo
{	   
	public :
		ChRedirectInfo( const char* pstrTarget, const char* pstrCurr )
							: m_strTarget( pstrTarget ), m_strCurr( pstrCurr )
							{}
		~ChRedirectInfo() 	{}

		const ChString& GetTarget() 		{ return m_strTarget; }
	private :
		ChString m_strTarget;
		ChString m_strCurr;
};


class  ChHtmlPane 
{	   
	public :
		enum tagPaneAction { 	
							actionOpen = 1, actionClose,
							actionRedirect, actionMove
						};

		enum tagPaneScroll { 	
							scrollYes = 1, scrollNo,
							scrollAuto
						};

		enum tagPaneOption { 	
					optionOverlapped = 0x1, 		optionFloating = 0x2,
				 	optionInternal = 0x4,			optionDocking = 0x8,
					optionBrowser = 0x10,			optionNonSizeable = 0x20,
					optionNoClose = 0x40, 			optionSmallTitle = 0x80,
					optionFit = 0x100,				optionExisting = 0x200,
					optionDocked = 0x400, 			optionPersistent =0x800,
					optionForce = 0x1000, 	 		optionWebTracker = 0x2000,
					optionViewBottom = 0x4000,		optionFileAppend = 0x8000
				 };
	
		ChHtmlPane( );
		virtual ~ChHtmlPane()
			{
			}
		int		 GetAction()		{ return m_iAction; }
		int		 GetAlign()			{ return m_iAlign; }
		const ChString& GetURL()		{ return m_strURL; }
		const ChString& GetTitle()	{ return m_strPaneTitle; }
		const ChString& GetName()		{ return m_strTargetName; }
		int		 GetScrolling()		{ return m_iScroll; }
		chuint32 GetOptions()		{ return m_luOption; }
		int		 GetVSpace()		{ return m_iVSpace; }
		int		 GetHSpace()		{ return m_iHSpace; }
		int		 GetWidth()			{ return m_iWidth; }
		int		 GetHeight()		{ return m_iHeight; }
		int		 GetMinWidth()		{ return m_iMinWidth; }
		int		 GetMinHeight()		{ return m_iMinHeight; }

		void SetAction( int iAction )			{ m_iAction = iAction; }
		void SetAlign( int iAlign )			{ m_iAlign = iAlign; }

		void SetURL( const char* pstrURL )	
											{ m_strURL = pstrURL; }
		void SetTitle( const char* pstrTitle )	
											{ m_strPaneTitle = pstrTitle; }
		void SetName( const char* pstrTarget )		
											{ m_strTargetName = pstrTarget; }
		void SetScrolling( int iScroll )	{ m_iScroll = iScroll; }
		void SetOptions( chuint32 luOption )	{ m_luOption = luOption; }
		void SetVSpace( int iSpace )			{ m_iVSpace = iSpace; }
		void SetHSpace( int iSpace )		{ m_iHSpace = iSpace; }
		void SetWidth( int iWidth )			{ m_iWidth = iWidth; }
		void SetHeight( int iHeight )		{ m_iHeight = iHeight; }
		void SetMinWidth( int iWidth )			{ m_iMinWidth = iWidth; }
		void SetMinHeight( int iHeight )		{ m_iMinHeight = iHeight; }


	private :
		int			m_iAction;
		ChString  	m_strPaneTitle;
		ChString  	m_strTargetName;
		ChString  	m_strURL;
		chuint32	m_luOption;
		int			m_iVSpace;
		int 		m_iHSpace;
		int			m_iWidth;
		int 		m_iHeight;
		int			m_iMinWidth;
		int 		m_iMinHeight;
		int 		m_iScroll;
		int			m_iAlign;

};


/////////////////////////////////////////////////////////////////////////////
// ChHtmlFrame frame

class ChHtmlFrame : public CFrameWnd
{
public:
	ChHtmlFrame( ChHtmlWnd* pFrameMgr, const ChString& strName, bool boolSizeToFit );           

// Attributes
public:
	ChHtmlWnd*  GetFrameMgr()	  	{ return m_pFrameMgr; }
	ChHtmlView* GetHtmlView()	  	{ return m_pView; }
	const ChString& GetFrameName()	{ return m_strName; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChHtmlFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ChHtmlFrame();

private :
	ChHtmlWnd*		m_pFrameMgr;
	ChHtmlView*		m_pView;
	ChString 			m_strName;
	bool			m_boolSizeToFit;

protected:
	// Generated message map functions
	//{{AFX_MSG(ChHtmlFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChHtmlMiniFrame frame

class ChHtmlMiniFrame : public CMiniFrameWnd
{
public:
	ChHtmlMiniFrame( ChHtmlWnd* pFrameMgr, const ChString& strName, bool boolSizeToFit );  

// Attributes
public:

	ChHtmlWnd*  GetFrameMgr()	  	{ return m_pFrameMgr; }
	ChHtmlView* GetHtmlView()	  	{ return m_pView; }
	const ChString& GetFrameName()	{ return m_strName; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChHtmlMiniFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ChHtmlMiniFrame();

private :
	ChHtmlWnd*		m_pFrameMgr;
	ChHtmlView*		m_pView;
	ChString 			m_strName;
	bool			m_boolSizeToFit;

protected:

	// Generated message map functions
	//{{AFX_MSG(ChHtmlMiniFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// $Log$

#endif //CHHTMLPANE_H
