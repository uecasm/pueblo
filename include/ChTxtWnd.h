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

	This file consists of the interface for the ChTxtWnd class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTXTVW_H )
#define _CHTXTVW_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#include <ChConst.h>
#include <ChScrWnd.h>

#endif	// defined( CH_MSW )

#ifdef CH_UNIX
#include <ChRect.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#include <ChFont.h>
#endif

#include <fstream>
#include <ChTxtObj.h>




/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define CH_DEF_COLOR	0x80000000


/*----------------------------------------------------------------------------
	
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChTextStyle;
class CH_EXPORT_CLASS ChTxtObject;
//class fstream;

/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/



typedef struct tagChLine
{
	chint32		lStartChar;		// line start index
	int			iX;				// X - cordinate
	int			iY;				// Y - coordinate
	int			iMaxHeight;		// max height of the line
//	int			iMaxAscent;		// max ascent of the line
	int			iMaxDescent;		// max ascent of the line
	int			iTotalWidth;	// total width of the line
	int			iMaxLineWidth;	// total width of the line
	chuint32	luLineAttr;		// line attributes
} ChLine, FAR *pChLine;


/*----------------------------------------------------------------------------
	ChRun structure -- Information on a character run of the same style
----------------------------------------------------------------------------*/

typedef struct tagChRun
{
	chint32		lStartChar;		// start of the run
	chint32		lStyleIndex;	// index to style table
} ChRun, FAR *pChRun;


/*----------------------------------------------------------------------------
	ChFontInfo structure -- Information about the font used for a given style
----------------------------------------------------------------------------*/

typedef struct tagChFontInfo
{
	int			iUseCount;		// use count for this style
	ChFont*		pFont;			// Font 
	LOGFONT		fontInfo;		// font information

} ChFontInfo, FAR *pChFontInfo;




/*----------------------------------------------------------------------------
	ChStyle structure -- Describes a style specification for inserting text
							or changing the style of existing text.
----------------------------------------------------------------------------*/

typedef struct tagChStyle
{

	int			iFontIndex;		// Index into the font table
	COLORREF	lColor;			// RGB color for this style
	COLORREF	lBackColor;		// Back ground color of the run
	chuint32	luStyle;		// paragraph style for the run
	int			iLeftIndent;	// Move x by iLeftIndent if this style starts a new line
	chparam		userData;		// user data associated with the style

	int			iObjectIndex;	// index to the object table

} ChStyle, FAR *pChStyle;


/*----------------------------------------------------------------------------
	ChStyleInfo structure -- Describes a style table entry.
----------------------------------------------------------------------------*/

typedef struct tagChStyleInfo
{
	ChStyle		style;			// style information
	int			iFontHeight;	// font height
	int			iFontAscent;	// font ascent
	int			iFontDescent;	// font descent
	chint32		lUseCount;		// number of runs using this style

} ChStyleInfo, FAR *pChStyleInfo;



typedef struct tagLayoutData
{

	int iCurrX;
	int iBreakY;
	int	iCurrPageWidth;

} LayoutData, FAR * pLayoutData;

/*----------------------------------------------------------------------------
	LineData structure -- internal structure used for computing the line
	width during line table update..
----------------------------------------------------------------------------*/

typedef struct tagLineData
{
	pstr			pText;
	pChRun			pRun;
	pChStyleInfo	pStyleUse;
	chint32			lStartChar;
	int				iMaxLineWidth;
	int				iWordHeight;
//	int				iWordAscent;
	int				iWordDescent;
	int				iWordWidth;
	int				iWordWhiteWidth;
	int				iWordCharCount;
	int				iWordRunCount;
	bool			boolObject;
	bool			boolObjectFloat;
	bool			boolStartNewLine;
	chuint32		luLineAttr;
	pLayoutData		pLayoutInfo;
	int				iCurrLayoutIndex;
	int				iLayoutMaxSize;
	int 			iCurrY;

} LineData, FAR *pLineData;

/*----------------------------------------------------------------------------
	ChTxtWnd class	: Text view class is a rich text view. The view supports
	line justification, indents, characters in any font, foreground and background
	color. A sequence of charcters with a same style is called a run, the style for a
	run is specified using ChTextStyle class.
----------------------------------------------------------------------------*/

#ifdef CH_MSW
class CH_EXPORT_CLASS ChTxtWnd : public ChScrollWnd
#else
class CH_EXPORT_CLASS ChTxtWnd : public ChScrollView
#endif
{
	friend class ChObjControl;
	friend class ChObjInline;
	public:
		enum tagDocAttrs { docVCenter = 0x1, docDisableSelection = 0x2, docViewAppend = 0x4,
						   docNoVertScroll = 0x8, docNoHorzScroll = 0x10 };

		enum tagStyles { textLeft = 0x1, textCenter = 0x2, textRight = 0x4,
						 textFull = 0x01, // Full justfication is  left justfied	 for now
						 textNoWrap = 0x10,  textIndentLeft = 0x20,	 textVCenter = 0x40,
						 textPreFormat = 0x80,
						 textBullet = 0x100,
						 textGraphic = 0x200,   textJumpMark = 0x400, textAlwaysUpdate = 0x800,
						 textHotSpot = 0x1000,
						 textDeleteUserData = 0x2000,
						 textObject = 0x4000,
						 textISMAP  = 0x8000,
						 textResetAnimation = 0x10000 };

		enum tagObjAttrs { objAttrLeft = 0x1, objAttrRight = 0x2, objAttrTop = 0x4,
							  objAttrMiddle = 0x8, objAttrBreak = 0x10,
							  objAttrClearLeft = 0x20, objAttrClearRight = 0x40,
							  objAttrFloat = 0x80, /* if the line is around a floating object*/
							  objAttrBackground   = 0x100,
							  objAttrNoShade = 0x200 };

		enum tagPointOn { locUnknown = 0, locText, locObject, locHotspot };

	public:
		ChTxtWnd();
		virtual ~ChTxtWnd();
		
 		inline chint32 GetBufferLimit() { return m_lTextLimit; }
		inline bool GetAppend() { return (m_luDocAttrs & docViewAppend) != 0; }

 		void SetBufferLimit( chint32 lLimit )
 						{
							#if defined( CH_MSW ) && defined( CH_ARCH_16 )
							{
								if (lLimit > 64500)
								{
									lLimit = 64500;
								}
							}
							#endif	// defined( CH_MSW ) && defined( CH_ARCH_16 )

 							m_lTextLimit = lLimit;
 						}

		inline void ShowAppend( bool boolAppend = true )
						{
							m_luDocAttrs &= ~(docViewAppend);
							m_luDocAttrs  |= (boolAppend ? docViewAppend : 0);
						}

		inline void GetScrollState( bool& boolHorizontal, bool& boolVertical )
						{
							boolHorizontal = !(m_luDocAttrs & docNoHorzScroll);
							boolVertical   = !(m_luDocAttrs & docNoVertScroll);
						}

		void AllowScroll( bool boolHorizontal, bool boolVertical );
		void GetDocumentSize( ChSize& docSize );
		inline void GetViewIndents( ChRect& viewIndents )
						{
							viewIndents = m_viewIndents;
						}
		void SetViewIndents( const ChRect& viewIndent );

		inline chuint32 GetDocumentAttrs() { return m_luDocAttrs; }
		inline void SetDocumentAttrs( const chuint32 luAttrs )
						{
							m_luDocAttrs = luAttrs;
						}

		bool WriteFile( const char* pstrFilePath, chflag32 flOptions = 0 );
		bool CloseFile();
		void ClearForms();
											/* Method called when mouse is
												released, either in the client
												or non-client area of this
												window */
		virtual void OnMouseUp() = 0;

	public:
											/* ClassWizard generated virtual
												function overrides */
		#if defined( CH_MSW )

		//{{AFX_VIRTUAL(ChTxtWnd)
		protected:
		virtual void OnDraw(CDC* pDC);      // overridden to draw this view
		virtual void OnInitialUpdate();     // first time after construct
		//}}AFX_VIRTUAL

		#else	// defined( CH_MSW )

		virtual void OnInitialUpdate();

		virtual XtCallbackProc GetOnDraw(void) { return (XtCallbackProc)OnDraw; };
		virtual XtCallbackProc GetOnInput(void) { return (XtCallbackProc)OnInput; };

		virtual void OnDraw( Widget widget, XtPointer client_data,
							XtPointer call_data );
		virtual void OnInput( Widget widget, XtPointer client_data,
							 XtPointer call_data );
		void OnLButtonUp(chuint32 nFlags, ChPoint& point);

		#endif	// defined( CH_MSW )

	public:
		void ClearPage();
		inline bool AppendText( const ChString& strNewText,
								chint32 lNewCount = -1,
								ChTextStyle * pNewStyle = 0 )
						{
							return AppendText( (const char*)strNewText,
												lNewCount, pNewStyle );
						}

		bool AppendText( const char* pstrNewText, chint32 lNewCount = -1,
							ChTextStyle * pNewStyle = 0 );
		bool AppendObject( ChTxtObject *pObject );

		bool SetStyle(  chint32 lStartChar, chint32 lCharCount,
							ChTextStyle *pNewStyle );
		int PointOn( int x, int y, chparam& userData, ChPoint *ptRel = 0 );
		char GetChar( chint32 lIndex )
					{
						ASSERT( lIndex < m_lTextCount );
						return( m_pstrText[lIndex] );
					}

		void GetSel( chint32& lStart, chint32& lEnd );
		void SetSel( chint32 lStart, chint32 lEnd );

		inline void EnableSelection( bool boolSel )
					{
						m_luDocAttrs &= ~(docDisableSelection);
						m_luDocAttrs |= (boolSel ? 0 : docDisableSelection );
					}

		inline bool IsSelectionEnabled()
					{
						return !(m_luDocAttrs & docDisableSelection);
					}

		#if defined( CH_MSW )
		void CopyToClipboard( );
		#else
		void CopyToClipboard( ) { cerr << "XXX Not copying to clipboard." << endl; };
		#endif

		void UpdateObject( ChTextObject* pObject, bool boolSizeChanged = false );

		// background color specifications
		void SetBackColor( chuint32 luColor );
		void SetBackPattern( ChDib* pDib );
		void DrawBackground( CDC* pDC );

		#if defined( CH_MSW )
		CBrush * GetCurrentBkBrush();

		ChDib* 	 GetCurrentBkImage()		{ return  m_pbackGround; }

		#endif
		void DisableHotSpots( chuint32 luColor );

		CDC *		GetContext()			{ return m_pDC; }
	
	protected:
		#ifdef CH_MSW
			virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
		#endif

	private :
		HANDLE				m_hHeap;			// View heap
		CDC					*m_pDC;				// Device context used by view
		int					m_iExtraHeight;		// Extra height for lines
		chint32				m_lTextCount;		// View character count
		pstr				m_pstrText;			// text buffer
		chint32				m_lRunCount;		// total runs
		pChRun				m_pRunsTbl;			// run table
		chint32				m_lLineCount;		// total lines
		pChLine				m_pLinesTbl;		// line table
		chint32				m_lStyleCount;		// total styles
		pChStyleInfo		m_pStyleTbl;		// style table
		ChStyle				m_defStyle;			// Default style
		pChFontInfo			m_pFontTbl;			// Font table
		chint32				m_lFontCount;
		chint32				m_lTextLimit;		// Max number of characters in the view	, 0 for nolimit
		chint32				m_lTextBlkSize;		// current size of text buffer
		ChTextObject**		m_pObjTbl;			// Object table
		chint32				m_lObjCount;		// total object;
		chint32				m_lObjTblSize;		// current table size
		ChSize				m_sizeTotal;		// Size of the canvas
		ChSize				m_viewSize;			// size of the visible area
		ChRect				m_viewIndents;		// indents for the view
		ChRect				m_updateRect;		// This rect will have the update rgn after call
												// to UpdateLineTable
		chint32				m_lStartUpdate;		// Update line table from
		chint32				m_lSelStart;		// Anchor for selection
		chint32				m_lSelEnd;			// end of selection
		bool				m_boolInSelMode;	// true when user is selecting text
		bool				m_boolDeletedText;	// set to true when the text is deleted
		bool				m_boolRedrawAll;	// View has a object that requires to be updated 
												// irrespective of the clip rect
		chuint32			m_luSelTextColor;   // Selection text color
		chuint32			m_luDocAttrs;		// Attributes for the current document

		std::fstream*			m_plogStream;
		bool				m_boolLog;
		chuint32			m_luLogOptions;
		char*				m_pstrLogBuffer;
		chuint16			m_iLogIndex;


		#if defined( CH_MSW )

		CBrush				m_brSelBackColor;

		#endif	// defined( CH_MSW )

	protected :

		#if defined( CH_MSW )

		ChDib*				m_pbkImage;				// background pattern DIB
		ChDib*				m_pbackGround;			// Dib for the background
		CBrush				m_bodyBkColor;			// back color if any for the document
		static CBrush		m_defBkColor;			// default background color
		int					m_idTimer;				// Timer ID
		UINT				m_uExpostureTime;		// Timer resolution

		#endif	// defined( CH_MSW )

	private :

		#if defined( CH_UNIX )

		bool				m_boolInitialUpdate;	// Is this the first OnDraw()?

		#endif	// defined( CH_UNIX )

	protected :
			enum tagConst { bufferGrowSize = 4092, minWidth = 50, objTblGrowSize = 15, 
							fontTblGrowSize = 10, logBufferSize = 80 };
			enum tagCommmands { vwInsert = 0x1, vwDelete = 0x2, vwUpdate = 0x4 };

			enum tagWriteOptions { writeAll = 0x01, writeHTML = 0x02, writeNew = 0x4 };

protected:
	// accessor functions
	HANDLE		GetHeap()				{ return m_hHeap; }
	void		SetContext( CDC* pDC )	{ m_pDC = pDC; }
	int			GetExtraHeight()		{ return m_iExtraHeight; }
	chint32		GetTextCount()			{ return m_lTextCount; }
	pstr		GetTextBuffer()			{ return m_pstrText; }
	chint32		GetRunCount()			{ return m_lRunCount; }
	pChRun		GetRunTable()			{ return m_pRunsTbl; }
	chint32		GetStyleCount()			{ return m_lStyleCount; }
   	pChStyleInfo	GetStyleTable()		{ return m_pStyleTbl; }
	pChStyle	GetDefaultStyle()		{ return &m_defStyle; }
	chint32		GetTextBufferSize()		{ return m_lTextBlkSize; }
	chint32		GetLineCount()			{ return m_lLineCount; }
   	pChLine		GetLineTable()			{ return m_pLinesTbl; }
	chint32		GetFontCount()			{ return m_lFontCount; }
   	pChFontInfo	GetFontTable()			{ return m_pFontTbl; }
	chint32		GetObjectCount()		{ return m_lObjCount; }
   	ChTextObject	**GetObjectTable()	{ return m_pObjTbl; }
	int			GetViewHeight()			{ return m_viewSize.cy; }
	int			GetViewWidth()			{ return m_viewSize.cx; }
	int			GetPageHeight()
						{
							return (m_viewSize.cy -
								(m_viewIndents.top + m_viewIndents.bottom ));
						}
	int			GetPageWidth()
						{
							return (m_viewSize.cx -
								(m_viewIndents.left + m_viewIndents.right ));
						}
	bool		RedrawAll()				{ return m_boolRedrawAll; }
	ChRect&		GetUpdateRect()			{ return m_updateRect; }
	int 		GetCanvasHeight()		{ return m_sizeTotal.cy; }
	bool		AlwaysDisplayAppend()	{ return (m_luDocAttrs & docViewAppend) != 0; }
	chint32 	GetLocToIndex( int x, int y );
	void 		GetIndexToLoc( chint32 lIndex, int& x, int& y, int& iLineMaxHt );
	void		GetBoundingRect( chint32 lStartIndex, chint32 lEndIndex, ChRect& rtBound );
	#if defined( CH_MSW )
	void 		GetBoundingRgn( chint32 lStartIndex, chint32 lEndIndex, CRgn& rtBound );
	#endif

  	// member functions
	void InitView();
	void TermView();
	bool AppendTextRun( const char* pstrNewText, chint32 lNewCount,
								ChTextStyle *pNewStyle = NULL );
	bool UpdateAppendText( chint32 lStartChar, bool boolDisplayAppend );
	chint32 GetUpdateIndex(  )  			   	{ return m_lStartUpdate; }
	void SetUpdateIndex( chint32 lIndex )  		{ m_lStartUpdate = lIndex; }


	bool UpdateLineTable( chint32 lStartChar, chint32 lCharCount,
							UINT fsOptions, bool* pboolChanged = 0 );
	void UpdateLayoutInfo( pLineData pCalc,  pChLine pNewLine, bool boolFloatLeft  );
	void ComputeTimerInterval( ChObjInline *pInline );
	void ComputeTimerInterval( ChDib *pDib );
	void ComputeLineInfo(  pLineData pCalc, pChLine pNewLine );
	void ComputeBigWord(  pLineData pCalc );
	void ComputeLastWordInALine( pLineData pCalc, int sCurrLineWidth );
	bool ComputeNextWordInfo( pLineData pCalc );
	int  GetWordCharCount(  pLineData pCalc );
	void ComputeWordInfo(  pLineData pCalc, int iWordCharCount );
	bool InsertText( const char* pstrNewText, chint32 lStartChar,
						chint32 lNewCount );
	chint32 AppendTextBuffer( const char* pstrNewText, chint32 lStartChar,
						chint32 lNewCount );
	bool DeleteText( chint32 lStartChar, chint32 lCharCount );
	bool RemoveText( chint32 lStartChar, chint32 lCharCount );
 	void UpdateLineStarts( chint32 lStartLine, chint32 lCharCount );
	void UpdateRunStarts( chint32 lStartRun, chint32 lCharCount );
	bool ChangeStyle( pChStyle pNewStyle, chint32 lStartRun,
			chint32 lStartChar, chint32 lCharCount );
	pChRun InsertRuns(  pChRun pRun, chint32 lStartRun, int sNewCount );
	pChRun DeleteRuns(  pChRun pRun, chint32 lStartRun, chint32 lLastChar );
	// Style table methods
	pChStyleInfo GetStyle(  pChStyleInfo pStyleUse, pChStyle pNewStyle,
					chint32& lIndex );
	void NewStyle(  pChStyleInfo pStyleUse, pChStyle pStyle );
	void ReleaseStyle( pChStyleInfo pStyleUse );
	bool EqualStyle( pChStyleInfo pStyleUse, pChStyle pStyle );
	// Font table methods
	int  GetFont( ChFont* pFont );
	void NewFont( pChFontInfo pFontElem );
	bool EqualFont( LOGFONT& pFont1, LOGFONT& pFont2 );

	bool ClearAll( );
	int GetRunIndex( chint32 lStartChar );
	int GetLineIndex( chint32 lStartChar, chint32& lTotalHeight );
	void GetTotalCanvasSize( ChSize& sizeTotal );
	int GetDrawCharCount( pstr pstrText, int sCharCount );
	int GetWhiteSpaceCount( pstr pstrText, int sCharCount );
	// Redering methods
	bool DrawRange();
	inline void DrawTextBackground( int x, int y, const char* pText, 
											int iCount, chuint32 luColor );
	inline void DrawTextBackground( int x, int y, const char* pText, 
											int iCount, CBrush* pBkBrush );
	void CreateBackground( ChDib* pDib );
	void CreateBackground( COLORREF clrBack );



	ChSize DrawObject(  int x, int y, ChStyle& style, ChSize& size );
	ChSize DrawLine(  int x, int y, ChStyle& style );

	void GetObjectSizeAndAttr( ChStyle& style, ChSize& size, chuint32& luAttr );
	bool PointInObject( int iObjIndex, ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0 );
	ChSize ComputeObjectInfo( pLineData pCalc );
	int  NewTextObject( ChTxtObject *pTxtObj );
	bool	SetViewSize();	 

	// File logging methods
	bool	 LogHTML()				{ return ( m_boolLog && ( m_luLogOptions & writeHTML ) ); }
	bool	 LogPlainText()			{ return ( m_boolLog && !(m_luLogOptions & writeHTML ) ); }
	std::fstream* GetLogFile()			{ return m_plogStream; }
	chint32	 WriteLogFile( chint32 lCount, bool boolAddNewline = false );
	bool 	 LogToFile( const char* pstrBuffer, chint32 lCount );

	// Animation methods
	void UpdateAnimation();

	#if defined( CH_MSW )
											// Generated message map functions
	//{{AFX_MSG(ChTxtWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
//	afx_msg BOOL OnQueryNewPalette();
//	afx_msg LONG OnRecomputeView(UINT wParam, LONG lParam );

	DECLARE_MESSAGE_MAP()

	#else
		 virtual void OnSize(UINT nType, int cx, int cy);
	#endif	// defined( CH_MSW )
};




/*----------------------------------------------------------------------------
	ChTextStyle class : This class provides the interface for specifying the
	style for a run.
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChTextStyle
{
	public:
		ChTextStyle(    ChFont *pFont,
						chuint32 luTextClr = CH_DEF_COLOR,
						chuint32 luBackColor = CH_DEF_COLOR,
						chuint32 luStyle = ChTxtWnd::textLeft,
						int  iLeftIndent = 0 ) :
						m_pFont( pFont ), m_luBackClr( luBackColor ),
						m_iLeftIndent( iLeftIndent ),
 				 		m_luStyle( luStyle ),
						m_lLineWidth( 0 ), m_lLineHeight( 0 ),
					    m_lPointSize( 0 ), m_userData( 0 ),
						m_lPointSizeExtra(0)

				{

				#ifdef CH_MSW
					m_luTextClr = ( CH_DEF_COLOR & luTextClr ) ?
									COLOR_DEF_TEXT /*::GetSysColor( COLOR_WINDOWTEXT )*/ :
									luTextClr;
				#else
					m_luTextClr = luTextClr;
					// We'll take care of the CH_DEF_COLOR stuff in
					// ChDC instead.
				#endif

				}

		// accessor functions
		chuint32 GetTextColor()					{ return m_luTextClr; }
		chuint32 GetBackColor()					{ return m_luBackClr; }
		ChFont *  GetFont() const				{ return m_pFont; }
		chuint32 GetStyle()						{ return m_luStyle; }
		int	 	 GetLeftIndent( ) 				{ return m_iLeftIndent; }
		chint32	 GetLineWidth()					{ return m_lLineWidth; }

		chint32	 GetLineHeight()				{ return m_lLineHeight; }
		chint32	 GetPointSize()					{ return m_lPointSize; }
		chint32	 GetPointSizeExtra()			{ return m_lPointSizeExtra; }
		chparam	 GetUserData()					{ return m_userData; }

		void SetFont( ChFont *pFont )			{ m_pFont = pFont; }
		void SetTextColor( chuint32 luColor)
				{
					#ifdef CH_MSW
					m_luTextClr = (CH_DEF_COLOR & luColor) ?
									COLOR_DEF_TEXT /*::GetSysColor( COLOR_WINDOWTEXT)*/ : luColor;
					#else
					m_luTextClr = luColor;
					// We'll take care of the CH_DEF_COLOR stuff in
					// ChDC instead.
					#endif
				}

		void SetBackColor( chuint32 luColor) 	{ m_luBackClr = luColor; }
		void SetStyle( chuint32 luStyle )		{ m_luStyle   = luStyle; }
		void SetLeftIndent( int iLeftIndent )	{ m_iLeftIndent = iLeftIndent; }
		void SetLineWidth( chint32 lWidth )		{ m_lLineWidth = lWidth; }
		void SetLineHeight( chint32 lHeight )	{ m_lLineHeight = lHeight; }
		void SetPointSize( chint32 lHeight )	{ m_lPointSize = lHeight; }
		void SetPointSizeExtra( chint32 lExtra ){ m_lPointSizeExtra = lExtra; }
		void SetUserData( chparam userData )	{ m_userData = userData; }

	private :
		ChFont 		*m_pFont;
		chuint32	m_luTextClr;
		chuint32	m_luBackClr;
		chuint32	m_luStyle;		// paragraph style for the run
		int			m_iLeftIndent;	// specifies the indentation
		chint32		m_lLineWidth, m_lLineHeight, m_lPointSize, m_lPointSizeExtra;
		chparam		m_userData;

	public :
		virtual ~ChTextStyle()		{};

};



/*----------------------------------------------------------------------------
	ChTextObject class : This class provides the interface for specifying the
	object information.
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChTxtObject
{
	public:

	ChTxtObject::ChTxtObject(  ChTextObject* pObj, ChFont *pFont = NULL,
						chuint32 luTextClr = CH_DEF_COLOR,
						chuint32 luBackColor = CH_DEF_COLOR,
						chuint32 luStyle = ChTxtWnd::textLeft,
						int 	 iLeftIndent = 0 );
		// accessor functions
		chuint32 GetTextColor()					{ return m_luTextClr; }
		chuint32 GetBackColor()					{ return m_luBackClr; }
		ChFont *  GetFont() const				{ return m_pFont; }
		chuint32 GetStyle()						{ return m_luStyle; }
		int	 GetLeftIndent( ) 					{ return m_iLeftIndent; }
		chparam	 GetUserData()					{ return m_userData; }

		void SetFont( ChFont *pFont )			{ m_pFont = pFont; }
		void SetTextColor( chuint32 luColor)
				{
					#ifdef CH_MSW
					m_luTextClr = (CH_DEF_COLOR & luColor) ?
									COLOR_DEF_TEXT /*::GetSysColor( COLOR_WINDOWTEXT)*/ : luColor;
					#else
					m_luTextClr = luColor;
					// We'll take care of the CH_DEF_COLOR stuff in
					// ChDC instead.
					#endif
				}

		void SetBackColor( chuint32 luColor) 	{ m_luBackClr = luColor; }
		void SetStyle( chuint32 luStyle )		{ m_luStyle   = luStyle; }
		void SetLeftIndent( int iLeft = 0 )		{ m_iLeftIndent = iLeft; }
		void SetUserData( chparam userData )	{ m_userData = userData; }
		ChTextObject* GetTextObject()			{ return m_ptxtObject; }


	private :
		ChFont 		*m_pFont;
		chuint32	m_luTextClr;
		chuint32	m_luBackClr;
		chuint32	m_luStyle;		// paragraph style for the run
		int			m_iLeftIndent;		// specifies the indentation
		chparam		m_userData;
		ChTextObject* m_ptxtObject;
	public :
		virtual ~ChTxtObject()		{};

};


/*----------------------------------------------------------------------------
MACROS :
----------------------------------------------------------------------------*/

#if defined( CH_MSW )
#define BGR( c )		\
				( (DWORD) (LOBYTE(HIWORD( c) )) | ((HIBYTE(LOWORD( c) )) << 8 )\
				 | ((DWORD)(LOBYTE(LOWORD( c) )) << 16 ) )
#endif


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR
#endif

#endif	// !defined( _CHTXTVW_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:00  uecasm
// Import of source tree as at version 2.53 release.
//
