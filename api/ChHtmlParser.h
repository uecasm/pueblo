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

	This file contains the  interface for ChHtmlParser class.

----------------------------------------------------------------------------*/

#if (!defined( _CHHTMLPARSER_H ))
#define _CHHTMLPARSER_H

#include <ChSplay.h>




/*----------------------------------------------------------------------------
Constants :
----------------------------------------------------------------------------*/

#define CHAR_DBL_QUOTE		TEXT( '"' )
#define CHAR_SGL_QUOTE		TEXT( '\'' )
#define CHAR_BACKSLASH		TEXT( '\\' )
#define CHAR_SPACE			TEXT( ' ' )


/*----------------------------------------------------------------------------
Macros :
----------------------------------------------------------------------------*/



#define IS_WHITE_SPACE( c )	  ( isspace( (c) ) || ((c) == 0x0A ) )

#define IS_NEW_TAG( c )	  ( (c) == TEXT('<') || (c) == TEXT( 'h') || (c) == TEXT( 'H') )


/*----------------------------------------------------------------------------
	Typedefs
----------------------------------------------------------------------------*/
class ChHtmlView;
class ChHtmlTag;
class ChTextStyle;
class ChObjInline;
class ChArgumentList;

typedef ChSplay< int, ChHtmlTag*> ChHtmlTagTbl;		  

enum tagClearArgOptions 
			{ clearLeft = 0x1, clearRight = 0x2, clearAll = 0x4,
			  clearLinks =0x8, clearText = 0x10, clearPlugins = 0x20,
			  clearForms = 0x40, clearImages = 0x80 };


// This defines the argument list for html tags
typedef struct tagChacoArgList
{
	int			iArgType; 		// type
	chparam		argVal;			// value

} ChArgList, FAR* pChArgList;

// HTML stack data

typedef struct tagChStackData
{
	// State information
	int			iType;					// tag type
	int			iLeftIndent;			// indents
	int			iPointSize;				// pointsize
	chuint32	luStyle;				// text view styles
	chuint32	luForeColor;			// fore color
	chuint32	luBackColor;			// back color
	int 		iPointSizeExtra;		// Add extar Height to the point size
	chparam		userData;				// user data
	// Data stored by certain tags during parsing
	int			iLineWidth;				// line width
	int			iLineHeight;			// line height
	chuint32	luModifier;			 	// argument modifiers
	int			iLineNumber;			// line number

} ChStackData, FAR* pChStackData;



/*----------------------------------------------------------------------------
	ChStack class
----------------------------------------------------------------------------*/

// This class defines the interface for the HTML stack

class ChStack
{
	public:
		ChStack( int iSize = 100, int iGrowSize = 25 );
		~ChStack();

		void Push( ChStackData& data );
		ChStackData* Pop();
		ChStackData* Peek( int iIndex );
		int GetTopIndex()
				{
					return m_iTopIndex - 1;
				}
		ChStackData* Remove( int iIndex );

	private:
		int 	m_iTopIndex;
		int 	m_iSize;
		int		m_iGrowSize;
		ChStackData* m_pStack;
};


/*----------------------------------------------------------------------------
	ChHtmlParser class
----------------------------------------------------------------------------*/
class  ChHtmlParser
{
	public :

		enum tagHTMLParserOptions { tagBufferSize = 4096, bufferSize = 4096,
							  		tagMaxArgs = 10, baseFontSize = 3,
							  		indentFactor = 4 }; // internal constants

		ChHtmlParser( ChHtmlView* pHtmlWnd );
		~ChHtmlParser();

		// Accessor methods
		// Attributes
		ChHtmlTag* 	GetHTMLTagInstance( int iToken );
		ChHtmlTag* 	GetHTMLTagInstance( const ChString& strToken );

		int  		GetArgType(  ChString& strArg );
		int 		GetCurrentIndent();

		inline ChStack& HTMLStack() 					{ return m_htmlStack; }
		inline const char* 	GetArgBuffer() 				{ return &m_pstrTagBuffer[m_iArgStart]; }
		inline int 			GetArgSize() 				{ return (m_iTagSize - m_iArgStart) > 0 
															   ?(m_iTagSize - m_iArgStart) : 0; }
		inline char* GetBuffer() 						{ return m_pstrLocalBuffer; }
		inline int GetBufferIndex() 					{ return m_iBufIndex; }			   
		inline ChHtmlView*	GetHtmlView()				{ return m_pHtmlView; }
 		inline ChHtmlTagTbl& GetHTMLTagInstance() 		{ return m_htmlInstTbl; }
		inline ChFont* GetHTMLFont() 					{ return m_pcurrFont; }
		inline ChTextStyle* GetTextStyle() 				{ return m_ptxtStyle; }
		inline	int GetBaseFontSize()					{ return m_iBaseFontSize; }
		inline  int GetLeftIndent()						{ return m_iLeftIndent; }
		inline	int  GetLineCharCount()					{ return m_iNumCharsInLine; }
		inline	void SetLineCharCount( int iCount)		{ m_iNumCharsInLine = iCount; }

		inline chuint32	GetTextColor()					{ return m_luTextColor; }
		inline chuint32	GetLinkTextColor()				{ return m_luLinkColor; }
		inline chuint32	GetVisitedLinkColor()			{ return m_luVLinkColor; }
		inline chuint32	GetActiveLinkColor()			{ return m_luALinkColor; }
		inline chuint32	GetPrefetchedLinkColor()		{ return m_luPLinkColor; }

		inline void SetBufferIndex( int iIndex ) 		{ m_iBufIndex = iIndex; }
		inline void SetBaseFontSize( int iSize )		{ m_iBaseFontSize = iSize; }
 		inline void	SetTextColor( chuint32 luColor )			
 														{ m_luTextColor = luColor; }
		inline void	SetLinkColor( chuint32 luColor )			
														{ m_luLinkColor = luColor; }
		inline void	SetVisitedLinkColor( chuint32 luColor )	
														{ m_luVLinkColor = luColor; }
		inline void	SetActiveLinkColor( chuint32 luColor )	
														{ m_luALinkColor = luColor; }
		inline void	SetPrefetchedLinkColor( chuint32 luColor )
														{ m_luPLinkColor = luColor; }

 		inline void	SetBackColor( chuint32 luColor )
 														{ m_pHtmlView->SetBackColor( luColor ); }			

		void			SetDocumentTitle( const ChString& strTitle )			 
										{m_pHtmlView->SetDocumentTitle( strTitle ); }
		void			SetDocBaseURL( const char* pstrURL )			 
										{ m_pHtmlView->SetDocBaseURL( pstrURL ); }


		inline void AppendChar( char strChar )
		{
			if ( m_iBufIndex >= bufferSize )
			{ // append this to the previous style
				m_pHtmlView->AppendTextRun( m_pstrLocalBuffer, m_iBufIndex, GetTextStyle() );
				m_iBufIndex = 0;
			}
			m_strLastChar = strChar;
			m_pstrLocalBuffer[ m_iBufIndex++ ] = strChar;
			m_iNumCharsInLine++;
		}

		inline 	void SetLastChar( char strChar )	{ m_strLastChar = strChar; }

		inline char GetLastChar()
			{
				return m_strLastChar;
				#if 0
				if ( m_iBufIndex )
				{ // we have text in our cache
					return m_pstrLocalBuffer[ m_iBufIndex - 1];
				}
				else
				{  // get the last char from the text wnd
					return m_pHtmlView->GetLastChar();
					//char strChar = m_pHtmlView->GetLastChar();
					//return (strChar == 0 || strChar == '\b' ) ? TEXT( ' ' ) : strChar;  
				}
				#endif
			}

		void CommitBuffer()
		{
			if ( m_iBufIndex )
			{  
				m_pHtmlView->AppendTextRun( m_pstrLocalBuffer,	m_iBufIndex, m_ptxtStyle );	
				// Reset the bufferindex of the style we modified
				m_iBufIndex = 0;
				m_boolLineBreak = true;
			} 
		}

		inline void IncrementUnderline()		{ m_iFontUnderline++; };
		inline void IncrementStrikethrough()	{ m_iFontStrikethrough++; };
		inline void IncrementItalic()			{ m_iFontItalic++; };
		inline void IncrementBold()				{ m_iFontBold++; };
		inline void IncrementFixed()			{ m_iFontFixed++; };

		inline void DecrementUnderline()		{ m_iFontUnderline > 0 ? m_iFontUnderline-- : 0; };
		inline void DecrementStrikethrough()	{ m_iFontStrikethrough > 0  ? m_iFontStrikethrough-- : 0; };
		inline void DecrementItalic()			{ m_iFontItalic > 0 ? m_iFontItalic-- : 0;};
		inline void DecrementBold()				{ m_iFontBold > 0 ? m_iFontBold-- : 0; };
		inline void DecrementFixed()			{ m_iFontFixed > 0 ? m_iFontFixed-- : 0; };

		inline void LineBreak( bool boolEnable = true )	
												{ m_boolLineBreak = boolEnable; }


		void ClearUserArgs();
		void UpdateUserArgs( const ChString& strName, const ChString& strValue );
		ChArgumentList*		GetUserArgs()		{ return m_pUserArgs; }


		int  MapAttributeValue( const char* pstrValue  );
		void ResetStack( int iToken );
	 	int LookUpTag( const char* pstrBuffer, chint32 lStart, 
								chint32 lCount, bool& boolEnd );   

		void CreateStyle( ChHtmlTag *pStyle );
		void RestoreTextStyle( int iType );



		// Methods
		void InitParser();
		void ParseText( const char* pstrNewText, chint32 lCount );

		void UpdateColors(  chuint32 luOldTextColor,
								 chuint32 luOldBkColor  );

		bool NotifyInline( const char *pstrBuf )
				{
					return m_pHtmlView->NotifyInline( pstrBuf );
				}
		void LoadInline( const char* pstrURL, ChObjInline *pImage )
				{
					m_pHtmlView->LoadInlineImage( ChString(pstrURL), pImage );
				}
		void LoadBkPattern( const ChString& strURL )
				{
					m_pHtmlView->LoadBkPattern( strURL  );
				}

		void GetObjectAttrs( int& idObj, chuint32& luAttr, ChSize& sizeObj, 
										ChRect& sizeExtra,  int lIndex  = -1 );
		bool GetLastObjectAttrs( int idObjType, chuint32& luAttr,
									ChSize& sizeObj, ChRect& spaceExtra );

		bool BreakTag( const char* pstrBuffer, chint32& lStart, chint32  lCount  );
		ChFont*		GetSymbolFont( ChFont* pFont );
	
	public :
		bool MapEntityToChar(  const char* pstrBuffer, chint32& lStart, chint32 lCount, char& strChar );
		static char MapEntity(  const char* pstrBuffer, chint32& lStart, chint32 lCount );
		static char GetSpecialCharacter( UINT uCharType  );


	private : // core Parser methods

		void Initialize();
		ChHtmlTag* CreateInstanceFromName( const ChString& strName );
		ChHtmlTag* CreateInstanceFromID( int iHtmlID );

		inline chint32 GetBufferSize() 			{ return bufferSize; }

		void FreeHTMLTagInstanceList();
		void PreProcessTag(  ChHtmlTag* pStyle );
		int  ProcessTerminationTag(  ChHtmlTag* pStyle );

		int  GetArguments(  chint32 lStart,	pChArgList pArg, int iMax );
		void UpdateAttributes(  int& iIndex, pChArgList pList,
									ChString& strAttr, ChString& strVal );
		int  LookUpTag( const char* pstrBuffer, chint32& lStart, chint32 lCount );

		void AddToBuffer( const char* pstrBuffer, chint32 lCount ); 

	private :
		void InitStatics();
		static   int  		GetTokenID( const char* pstrToken );
		static void 		SortSymbolMap( pSymbolMap pmapTbl, int iTblSize );
		static int 			FindSymbol(const char *pstrSymbol, pSymbolMap pmapTbl,
													int iTblSize );

	private :

		char*							m_pstrLocalBuffer;		// internal buffer
		char*							m_pstrTagBuffer;		// internal tag buffer
		int								m_iBufIndex;			// current index
		int								m_iArgStart;			// Start of argument buffer
		int								m_iTagSize;				// sizeof tag
		int								m_iNumCharsInLine;		// Num chars in the current line

		char*							m_pstrDataBuffer;		// Data buffer
		chint32							m_lDataBufSize;			// Current buffer content size
		chint32							m_lDataBufAllocSize;	// Allocated size

		int								m_iFontItalic;			// use italic if > 0
		int								m_iFontBold;			// use bold if > 0
		int 							m_iFontUnderline;		// use underline if > 0
		int								m_iFontStrikethrough;	// use strikeout if > 0		
		int								m_iFontFixed;			// fixed font in effect

		bool							m_boolLastAddSpaceAbove;
		bool							m_boolLastAddSpaceBelow;
		bool							m_boolLineBreak;
		char							m_strLastChar;

		#if !defined( NO_TEMPLATES )
		ChHtmlTagTbl					m_htmlInstTbl;	 		// HTML tag table
		#endif
		ChStack							m_htmlStack;			// HTML state stack

		ChTextStyle*					m_ptxtStyle;		  	// current text style
		ChFont*							m_pcurrFont; 			// current font
		CFont							m_symbolFont;
		ChHtmlView*						m_pHtmlView;				// HTML window
		ChArgumentList*					m_pUserArgs;

		// current document overrides
		int								m_iBaseFontSize;		// Used by font tag
		int								m_iLeftIndent;			// Left indent of the view
		// color for regular text, visited links, active links, prefetched links
		chuint32 						m_luTextColor;			// regular text color
		chuint32 						m_luLinkColor;			// link color
		chuint32 						m_luVLinkColor;			// visited link
		chuint32 						m_luALinkColor;			// active link
		chuint32 						m_luPLinkColor;			// prefetched link



};

#endif // !define _CHHTMLPARSER_H

