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

	This file consists of the implementation of the ChHtmlParser .

----------------------------------------------------------------------------*/		 

// $Header$

#include "headers.h"

#include <ctype.h>

#ifdef CH_UNIX
	#include <stdlib.h>
	#include <string.h>
	#include <malloc.h>
	#include <ctype.h>

	#include <ChTypes.h>
	#include <ChRect.h>
	#include <ChSize.h>
	#include <ChScrlVw.h>
	#include <ChDC.h>
#else
	#if !defined(CH_PUEBLO_PLUGIN)
	#include "resource.h"
	#else
	#include "vwrres.h"
	#endif
#endif


#include <ChConst.h>
#include <ChHtmWnd.h>
#include "ChHtmlView.h"
#include <ChUtil.h>
#include <ChReg.h>	 
#include <ChHtmlSettings.h>
#include <ChArgList.h>

#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChHtmlTag.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChStack class
----------------------------------------------------------------------------*/

ChStack::ChStack( int iSize /* = 100 */, int iGrowSize /* = 25 */)
{
	ASSERT( iSize );
	ASSERT( iGrowSize );
	m_iSize = iSize;
	m_iGrowSize = iGrowSize;

	m_iTopIndex = 0;
	m_pStack = (ChStackData*)::malloc( sizeof( ChStackData ) * m_iSize );
	ASSERT( m_pStack );
}

ChStack::~ChStack()
{
	free( m_pStack );
}

void ChStack::Push( ChStackData& data )
{
	if ( m_iTopIndex >= m_iSize )
	{
		m_iSize += m_iGrowSize;
		m_pStack = (ChStackData*)::realloc( m_pStack, sizeof( ChStackData ) * m_iSize );
		ASSERT( m_pStack );
		TRACE( "Resizing stack\n" );
	}
	m_pStack[m_iTopIndex++] =  data;
}

ChStackData* ChStack::Pop( )
{
	ASSERT( m_iTopIndex );
	--m_iTopIndex;
	return m_pStack + m_iTopIndex;
}

ChStackData* ChStack::Peek( int iIndex )
{
	ASSERT( iIndex < m_iTopIndex );
	return m_pStack + iIndex;
}


ChStackData* ChStack::Remove( int iIndex )
{
	ASSERT( iIndex < m_iTopIndex );
	// copy the current one to a temp location
	m_pStack[m_iTopIndex] = m_pStack[iIndex];

	--m_iTopIndex;

	ChMemCopy( m_pStack + iIndex, m_pStack + iIndex + 1,
				(sizeof( ChStackData ) * (m_iTopIndex - iIndex) ));

	return m_pStack + m_iTopIndex + 1;
}




/*----------------------------------------------------------------------------
	ChStack class
----------------------------------------------------------------------------*/


ChHtmlParser::ChHtmlParser( ChHtmlView* pHtmlView ) : m_pHtmlView( pHtmlView )
{
	Initialize();
}

ChHtmlParser::~ChHtmlParser()
{
	FreeHTMLTagInstanceList();
	delete [] m_pstrLocalBuffer;
	delete [] m_pstrTagBuffer;
	delete m_pcurrFont;
	delete m_ptxtStyle;
	delete m_pUserArgs;

}

void ChHtmlParser::Initialize()
{

	// internal cache
	m_pstrLocalBuffer = new char[ ChHtmlParser::bufferSize ];
	ASSERT( m_pstrLocalBuffer );
	m_iBufIndex = 0;
	// Max size of tag + args
	m_pstrTagBuffer = new char[ChHtmlParser::tagBufferSize];
	ASSERT( m_pstrTagBuffer );
	m_iTagSize = 0;
	m_pcurrFont = 0;
	m_ptxtStyle = 0;
	m_iNumCharsInLine = 0;

	m_pstrDataBuffer = 0;
	m_lDataBufSize  = 0;
	m_lDataBufAllocSize = 0;	// Allocated size

	m_pUserArgs = 0;

	InitStatics();

}

void ChHtmlParser::InitParser()
{
	// Reset all our state variables
	m_iFontItalic = m_iFontBold = m_iFontFixed =
	m_iFontUnderline = m_iFontStrikethrough = 0;
	m_iBaseFontSize = baseFontSize;
	m_iArgStart = 0;
	m_iTagSize = 0;
	m_iLeftIndent = m_pHtmlView->GetSettings()->GetLeftIndent();

	m_boolLastAddSpaceAbove = m_boolLastAddSpaceBelow = false;
	m_boolLineBreak = true;

	SetLineCharCount( 0 );

	// Set all colors to default
	SetTextColor( m_pHtmlView->GetSettings()->GetTextColor() );			
	SetLinkColor( m_pHtmlView->GetSettings()->GetLinkColor() );			
	SetVisitedLinkColor( m_pHtmlView->GetSettings()->GetVistedLinkColor() )	;
	SetActiveLinkColor( m_pHtmlView->GetSettings()->GetActiveLinkColor() );	
	SetPrefetchedLinkColor( m_pHtmlView->GetSettings()->GetPrefetchedLinkColor() );

	ResetStack( HTML_PREFORMAT );
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::ParseText

------------------------------------------------------------------------------
	This method is the main entry point for our HTML parser. The state
	of the parser is maintained using our own stack. On entry there should be
	atleast one entry in our stack

----------------------------------------------------------------------------*/

void ChHtmlParser::ParseText( const char* pstrInput, chint32 lLength )
{
	chint32 lStart 		= 0;
	const char *	pstrText = pstrInput;
	char*			pstrBuffer = 0;

	if ( lLength == -1 )
	{
		lLength = lstrlen( pstrInput );
	}

	if ( m_lDataBufSize )
	{ 	// we have buffered data from the last read
		pstrBuffer = new char[ m_lDataBufSize + lLength ];
		ASSERT( pstrBuffer );
		ChMemCopy( pstrBuffer, m_pstrDataBuffer, m_lDataBufSize );
		ChMemCopy( &pstrBuffer[m_lDataBufSize], pstrInput, lLength ); 
		// delete the old buffer
		delete [] m_pstrDataBuffer;
		m_pstrDataBuffer = 0;
		m_lDataBufAllocSize = m_lDataBufSize = 0;

		pstrText = pstrBuffer;
	}

	// there should atleast be one entry in our stack
	ASSERT( HTMLStack().GetTopIndex() >= 0 );

	if ( m_iTagSize )
	{ // we have a incomplete tag
		int iType = LookUpTag( pstrText, lStart, lLength );
	 	// style of the current state
		ChHtmlTag *pCurrStyle = GetHTMLTagInstance( iType );

		if ( pCurrStyle->GetAttributes() & ChHtmlTag::attrCallProcessTag )
		{ // this tag has a proc to process data
			CommitBuffer();
			lStart = pCurrStyle->ProcessTag( pstrText, lStart, lLength );
			CommitBuffer();
		}
	}

	/*	// UE: Don't skip whitespace at the beginning of the stream, it's annoying!
	if ( GetHtmlView()->GetTextCount() == 0 )
	{ // skip all the white space at the begining of stream.
		while( lStart < lLength && 	IS_WHITE_SPACE( pstrText[lStart] ) )
		{
			lStart++;
		}
	}
	*/

	// process all the characters in the buffer
	while ( lStart < lLength )
	{
		switch( pstrText[lStart] )
		{
			case TEXT( '<' ) :
			{	// New tag ?
				CommitBuffer();
				// look up tag will process the the tag and any arguments, creates fonts
				// and updates the stack if necessary, lStart is updated by the function
			  	int iType = LookUpTag( pstrText, lStart, lLength );
			 	// style of the current state
				ChHtmlTag *pCurrStyle = GetHTMLTagInstance( iType );

				if ( pCurrStyle->GetAttributes() & ChHtmlTag::attrCallProcessTag )
				{ // this tag has a proc to process data
					CommitBuffer();
					lStart = pCurrStyle->ProcessTag( pstrText, lStart, lLength );
					CommitBuffer();
				}
				break;
			}
			default :
			{   // we are continuing the previous style
				ChStackData* pdataTop = HTMLStack().Peek( HTMLStack().GetTopIndex() );

				ChHtmlTag *pCurrStyle = GetHTMLTagInstance( pdataTop->iType );
				ASSERT( pCurrStyle->GetAttributes() & ChHtmlTag::attrCallProcessTag );
				CommitBuffer();
				lStart = pCurrStyle->ProcessTag( pstrText, lStart, lLength );
				CommitBuffer();
				break;
			}
		}
	}

	CommitBuffer();

	if ( pstrBuffer )
	{
		delete []pstrBuffer;
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
//////  Private member fuctions of HTML view class

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::FreeHTMLStyleTable

------------------------------------------------------------------------------
	cleanup code.
----------------------------------------------------------------------------*/

class ZapHtmlTagInstance : public ChVisitor2< int, pChHtmlTag >  
{
	public:
	 	bool Visit( const int& key,  const pChHtmlTag& pInst )
		{
			delete pInst;
			return true;
		}
};


void ChHtmlParser::FreeHTMLTagInstanceList()
{
	// cleanup all the allocated resources
	ZapHtmlTagInstance		zapInst;

  	m_htmlInstTbl.Infix( zapInst );

	m_htmlInstTbl.Erase();
}

void ChHtmlParser::AddToBuffer( const char* pstrBuffer, chint32 lCount )
{
	if ( 0 == m_pstrDataBuffer )
	{
		m_pstrDataBuffer = new char[lCount];
		ASSERT( m_pstrDataBuffer );
		m_lDataBufAllocSize = m_lDataBufSize = lCount;
	}
	else if ( lCount > m_lDataBufAllocSize )
	{
		delete []m_pstrDataBuffer;
		m_pstrDataBuffer = new char[lCount];
		ASSERT( m_pstrDataBuffer );
		m_lDataBufAllocSize = m_lDataBufSize = lCount;
	}

	ChMemCopy( m_pstrDataBuffer, pstrBuffer, lCount );
}


int ChHtmlParser::GetCurrentIndent()
{
	int iIndent = GetTextStyle()->GetLeftIndent();

	for( int i = HTMLStack().GetTopIndex(); i > 0; i-- )
	{
		ChStackData* pStack;
		pStack = HTMLStack().Peek( i );

		if ( pStack->iType == HTML_DEFNLIST || 
			 pStack->iType == HTML_BULLETLINE ||
			 pStack->iType == HTML_NUMLIST ||
			 pStack->iType == HTML_DIR  )
		{
			iIndent = pStack->iLeftIndent;
			break;
		}
	}

	return iIndent;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::GetHTMLTagInstance

------------------------------------------------------------------------------
	This method returns the pointer to ChHtmlParser for a given tag ID, tag IDs are
	listed in ChHTMLVw.h ( HTML_XXXX).
----------------------------------------------------------------------------*/


ChHtmlTag* ChHtmlParser::GetHTMLTagInstance( int iToken )
{
	ChHtmlTag ** ppTag = m_htmlInstTbl.Find( iToken );

	if ( ppTag && *ppTag )
	{
		return( *ppTag );
	}
	else
	{
		ChHtmlTag* pInst = CreateInstanceFromID( iToken );
		ASSERT( pInst );
		m_htmlInstTbl.Insert( iToken, pInst );
		return ( pInst );
	}
}


ChHtmlTag* ChHtmlParser::GetHTMLTagInstance( const ChString& strToken )
{
	return GetHTMLTagInstance( GetTokenID( strToken ) ); 
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::LookUpTag

------------------------------------------------------------------------------
This method looks up the HTML table for a given tag returns the HTML tag code.
if found else HTML_UNKNOWN. This method also updates lStart to point to
the charcter after the tag.
If the tag is valid and a start tag it pre-processes the tag, setting up
the styles, font, stack etc. for termination tags it restores the state
if for the tag.

----------------------------------------------------------------------------*/


int ChHtmlParser::LookUpTag( const char* pstrBuffer, chint32& lStart,
							chint32 lCount )
{
	bool boolEnd = false;

	int  iType = HTML_UNKNOWN;


	ChStackData* pdataTop = HTMLStack().Peek( HTMLStack().GetTopIndex() );

	// if we are in plaintext mode ignore all HTML tags
	if ( HTML_PLAINTEXT == pdataTop->iType
					|| HTML_MUDTEXT == pdataTop->iType  )
	{
		return pdataTop->iType;
	}

	// get the HTML tag to our local buffer
	chint32 lDelimTag = -1;
	chint32 lDelimBuf = -1;

	if ( m_iTagSize )
	{ 			// there is a partial tag from the previous read
				// check if we were inside a delimited quote
		chint32 lScanQuote = 0;
		char 	strTerm;
		bool	boolInQuote = false;
		while ( lScanQuote < m_iTagSize )
		{  
			if ( m_pstrTagBuffer[lScanQuote] == CHAR_DBL_QUOTE  
					|| m_pstrTagBuffer[lScanQuote] == CHAR_SGL_QUOTE )
			{
				strTerm = m_pstrTagBuffer[lScanQuote++];
			 	boolInQuote = true;
				while ( lScanQuote < m_iTagSize )
				{
					if ( m_pstrTagBuffer[lScanQuote++] == strTerm )
					{
					 	boolInQuote = false;
						break;
					}
				}
			}
			else
			{
				lScanQuote++;	
			}
		}
		if ( boolInQuote )
		{
			while( lStart < lCount &&
				 pstrBuffer[lStart] != strTerm 
				 && m_iTagSize < tagBufferSize )
			{
				if ( pstrBuffer[lStart] == TEXT( '>' ) )
				{	// convert all > to entity so that our argument processor 
					// does not break
					if ( lDelimTag == - 1 )	
					{
					 	lDelimTag = m_iTagSize;
					 	lDelimBuf = lStart;
					}
					lStart++;  // ignore the >
					m_pstrTagBuffer[m_iTagSize++] = TEXT( '&' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( 'g' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( 't' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( ';' );		
				}
				else
				{
					m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];
				}
			}
			if ( lStart < lCount && m_iTagSize < tagBufferSize )
			{
				// copy the delimitor
				m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];
			}
		}
	}

	while( lStart < lCount &&
				pstrBuffer[lStart] != TEXT( '>' ) && m_iTagSize < tagBufferSize )
	{
		if ( pstrBuffer[lStart] == CHAR_DBL_QUOTE  
					|| pstrBuffer[lStart] == CHAR_SGL_QUOTE )
		{
			char strTerm = pstrBuffer[lStart];

			// copy the delimitor
			m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];

			// copy the string within the quotes ignoring the > char
			while( lStart < lCount &&
				pstrBuffer[lStart] != strTerm && m_iTagSize < tagBufferSize )
			{
				if ( pstrBuffer[lStart] == TEXT( '>' ) )
				{	// convert all > to entity so that our argument processor 
					// does not break
					if ( lDelimTag == - 1 )	
					{
					 	lDelimTag = m_iTagSize;
					 	lDelimBuf = lStart;
					}
					lStart++;
					m_pstrTagBuffer[m_iTagSize++] = TEXT( '&' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( 'g' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( 't' );		
					m_pstrTagBuffer[m_iTagSize++] = TEXT( ';' );		
				}
				else
				{
					m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];
				}
			}
			if ( lStart < lCount && m_iTagSize < tagBufferSize )
			{
				// copy the delimitor
				m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];
			}
		}
		else
		{
			m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];
		}
	}

	if ( ( lStart >= lCount ||  m_iTagSize >= tagBufferSize )
				&& pstrBuffer[lStart] != TEXT( '>' ) )
	{
		if ( lDelimTag != - 1 )	
		{
			m_iTagSize 	= lDelimTag;
			lStart 		= lDelimBuf;
		}
	}


	// is the tag terminated ?
	if ( lStart >= lCount || pstrBuffer[lStart] != TEXT( '>' ) )
	{ // partial tag
		//save this and return

		if ( m_iTagSize >= tagBufferSize )
		{ // token is too long, this should be a bogus tag
		  // skip it completly
			while( lStart < lCount &&
					pstrBuffer[lStart++] != TEXT( '>' ) )

			// remove all new lines characters
			while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ) )
			{
				++lStart;
			}
		}

		return iType;

	}
	// the '>' char
	m_pstrTagBuffer[m_iTagSize++] = pstrBuffer[lStart++];


	ASSERT( m_pstrTagBuffer[0] == TEXT( '<' ) );

	if ( HTML_PARAGRAPH != pdataTop->iType &&  HTMLStack().GetTopIndex() == 0 &&
					GetHtmlView()->GetTextCount() == 0 )
	{ // if there is no text in our buffer and we found a tag the reset it
	  // to paragraph mode
		ResetStack( HTML_PARAGRAPH );
	}

 	// Process the tag
	m_iArgStart = 1;	//skip '<'

	if ( m_pstrTagBuffer[m_iArgStart] == TEXT( '/' ))
	{	// termination tag
		boolEnd = true;
		m_iArgStart++;
	}

	// special processing for comment
	if ( m_pstrTagBuffer[m_iArgStart] == TEXT( '!' ))
	{
		// remove all new lines characters
		while( lStart < lCount &&  (  pstrBuffer[lStart] == TEXT( '\r' )
									|| pstrBuffer[lStart] == TEXT( '\n' )) )
		{
			++lStart;
		}
		m_iTagSize = 0;
		return ( HTML_COMMENT );
	}

	// get the tag name
	ChString strToken( "" );
	strToken.GetBuffer( 10 );

	while( m_iArgStart < lCount  &&  m_pstrTagBuffer[m_iArgStart] != CHAR_SPACE
							&&  m_pstrTagBuffer[m_iArgStart] != TEXT( '\r' )
							&&  m_pstrTagBuffer[m_iArgStart] != TEXT( '\n' )
							&&  m_pstrTagBuffer[m_iArgStart] != TEXT( '>' ) )
	{
		strToken += m_pstrTagBuffer[m_iArgStart++];
	}

	strToken.MakeLower();  
	strToken.TrimLeft();  
	strToken.TrimRight();  
	ChHtmlTag *pStyle = GetHTMLTagInstance( strToken );

	iType = pStyle->GetHTMLId();

	if ( iType == HTML_UNKNOWN )
	{
		if ( m_iTagSize )
		{
			//Report Unknown Tag
			m_pstrTagBuffer[ m_iTagSize - 1] = 0;
			ChString strMsg( TEXT( "Invalid HTML tag : " ));
			strMsg += &m_pstrTagBuffer[1];
			m_pHtmlView->GetFrameMgr()->OnTrace( strMsg, ChHtmlWnd::traceWarning );
		}
		m_iTagSize = 0;
		
		return iType;
	}

	if ( !(pStyle->GetAttributes()  & ChHtmlTag::attrParseWhiteSpace ) && 
			!(GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		chint32 lTemp = lStart;
		// remove all new lines characters at the end of the tag
		while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ) )
		{
			++lStart;
		}

		if ( (lStart - lTemp) && !(pStyle->GetAttributes()  & ChHtmlTag::attrStartLine
				|| pStyle->GetAttributes()  & ChHtmlTag::attrTerminateLine
				|| pStyle->GetAttributes()  & ChHtmlTag::attrTrimRight
				|| pStyle->GetAttributes()  & ChHtmlTag::attrAddLineBelow  ))
				//&& lStart < lCount  &&  pstrBuffer[lStart] != TEXT( '<' ) )
		{
			// leave atleast one white space 
			lStart -= 1;
		}
	}

	if ( !boolEnd )
	{ // process the tag
		 
		m_pstrTagBuffer[ m_iTagSize ] = 0;
		PreProcessTag( pStyle );
	}
	else
	{
	   	// do all cleanup for the tag
		iType = ProcessTerminationTag(  pStyle );
	}
	// finished processing the tag
	m_iTagSize = 0;
	m_iArgStart = 0;
	return iType;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::RestoreTextStyle

------------------------------------------------------------------------------
This method pops the stack and restores the previos state

----------------------------------------------------------------------------*/

void ChHtmlParser::RestoreTextStyle( int iType )
{
	int 	iIndex = HTMLStack().GetTopIndex();
	ChStackData* pdataTerm = 0;

	while( iIndex > 0 )
	{
		pdataTerm = HTMLStack().Peek( iIndex );

		if ( pdataTerm->iType == iType )
		{
			break;
		}
		else
		{
		 	if( pdataTerm->iType == HTML_PARAGRAPH )
			{   // for paragragh the termination is optional
				// pop the stack
				HTMLStack().Pop();
			}
		}
		--iIndex;
	}


	if ( iIndex && iIndex == HTMLStack().GetTopIndex() )
	{
		GetTextStyle()->SetLeftIndent( pdataTerm->iLeftIndent );
		GetTextStyle()->SetStyle( pdataTerm->luStyle );
		GetTextStyle()->SetPointSize( pdataTerm->iPointSize );
		GetTextStyle()->SetPointSizeExtra( pdataTerm->iPointSizeExtra );
		GetTextStyle()->SetTextColor( pdataTerm->luForeColor );
		GetTextStyle()->SetBackColor( pdataTerm->luBackColor );
		GetTextStyle()->SetLineWidth( pdataTerm->iLineWidth );
		GetTextStyle()->SetLineHeight( pdataTerm->iLineHeight );
		GetTextStyle()->SetUserData( pdataTerm->userData );
		// pop the stack
		HTMLStack().Pop();
	}
	else if ( iIndex )
	{
		// currently the token terminating which is not on the top of the stack, we should
		// remove all the styles which was added to iTop + 1 and above

		
		ChHtmlTag *pStyle = GetHTMLTagInstance( pdataTerm->iType );
		ChHtmlTag *pTmpStyle;

		int iCurrIndex;

		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreTextColor )
		{
			GetTextStyle()->SetTextColor( pdataTerm->luForeColor );

			iCurrIndex = iIndex + 1; 


			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreTextColor )
				{
					pStk->luForeColor = pdataTerm->luForeColor;	
					break;
				}

				pStk->luForeColor = pdataTerm->luForeColor;	

				iCurrIndex++;
			}
			
		}

		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreBkColor )
		{
			GetTextStyle()->SetBackColor( pdataTerm->luBackColor );

			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreBkColor )
				{
					pStk->luBackColor = pdataTerm->luBackColor;	
					break;
				}

				pStk->luBackColor = pdataTerm->luBackColor;	

				iCurrIndex++;
			}
		}
	
		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreLineIndent )
		{
			GetTextStyle()->SetLeftIndent( pdataTerm->iLeftIndent );

			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreLineIndent )
				{
					pStk->iLeftIndent = pdataTerm->iLeftIndent;	
					break;
				}

				pStk->iLeftIndent = pdataTerm->iLeftIndent;	

				iCurrIndex++;
			}
		}

		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreLineFmt )
		{
			GetTextStyle()->SetStyle( pdataTerm->luStyle );
		
			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreLineFmt )
				{
					pStk->luStyle = pdataTerm->luStyle;	
					break;
				}

				pStk->luStyle = pdataTerm->luStyle;	

				iCurrIndex++;
			}
		}

		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreUserData )
		{
			GetTextStyle()->SetUserData( pdataTerm->userData );

			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreUserData )
				{
					pStk->userData = pdataTerm->userData;	
					break;
				}

				pStk->userData = pdataTerm->userData;	

				iCurrIndex++;
			}
		}

		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreFontSize )
		{
			GetTextStyle()->SetPointSize( pdataTerm->iPointSize );

			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreFontSize )
				{
					pStk->iPointSize = pdataTerm->iPointSize;	
					break;
				}

				pStk->iPointSize = pdataTerm->iPointSize;	

				iCurrIndex++;
			}
		}
		if ( pStyle->GetRestoreFlags() & ChHtmlTag::restoreFontSizeExtra )
		{
			GetTextStyle()->SetPointSizeExtra( pdataTerm->iPointSizeExtra );

			iCurrIndex = iIndex + 1; 
			while( iCurrIndex <= HTMLStack().GetTopIndex() )
			{
				ChStackData* pStk = HTMLStack().Peek( iCurrIndex );

				pTmpStyle = GetHTMLTagInstance( pStk->iType );
				if ( pTmpStyle->GetRestoreFlags() & ChHtmlTag::restoreFontSizeExtra )
				{
					pStk->iPointSizeExtra = pdataTerm->iPointSizeExtra;	
					break;
				}

				pStk->iPointSizeExtra = pdataTerm->iPointSizeExtra;	

				iCurrIndex++;
			}
		}

		// remove the terminated tag
		HTMLStack().Remove( iIndex );

	}
	else
	{
		ASSERT( "invalid end tag " );
	}

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::PreProcessTag

------------------------------------------------------------------------------
 This method process all the arguments for a given tag and returns the
 new index where the processing stopped.
				 


----------------------------------------------------------------------------*/
void ChHtmlParser::PreProcessTag( ChHtmlTag* pStyle )
{

	bool 		boolRestore = false;
	chuint32	luTagAttrs  = pStyle->GetAttributes();


	if ( luTagAttrs & ChHtmlTag::attrSaveState )
	{ 	// save the state before we process this tag
		ChStackData data;

		ChMemClearStruct( &data );


		data.iType = pStyle->GetHTMLId();


		ChStackData *pdataTop = HTMLStack().Peek( HTMLStack().GetTopIndex()  );

		if ( HTMLStack().GetTopIndex() && pdataTop->iType == HTML_PARAGRAPH &&
					luTagAttrs & ChHtmlTag::attrAddLineAbove )
		{ 	// HTML_PARAGRAPH has a optional </p> tag if we are starting a new style
		    // and top of the stack is <p> then this is as good as a </p>
			RestoreTextStyle( pdataTop->iType );
		}
		else if ( data.iType == HTML_PARAGRAPH && HTMLStack().GetTopIndex() &&
				HTMLStack().Peek( HTMLStack().GetTopIndex())->iType == HTML_PARAGRAPH )
		{	// if we get another <p> before </p> the consider this <p> as </p> and <p>
			HTMLStack().Pop();
		}

		data.iLeftIndent = GetTextStyle()->GetLeftIndent();
		data.luStyle = GetTextStyle()->GetStyle( );
		data.luForeColor = GetTextStyle()->GetTextColor( );
		data.luBackColor = GetTextStyle()->GetBackColor(  );
		data.iLineWidth = GetTextStyle()->GetLineWidth( );
		data.iLineHeight = GetTextStyle()->GetLineHeight( );
		data.userData = GetTextStyle()->GetUserData(  );
		data.iPointSize = GetTextStyle()->GetPointSize(  );
		data.iPointSizeExtra = GetTextStyle()->GetPointSizeExtra(  );

		// We use <p> only as a line break.
		if ( data.iType == HTML_PARAGRAPH && 
				GetTextStyle()->GetStyle( ) & ChTxtWnd::textNoWrap )
		{
			boolRestore = true;
		}

		HTMLStack().Push( data );
	}


	if ( luTagAttrs & ChHtmlTag::attrCallStart )
	{  // Call Start of tag 
		pStyle->StartTag();
	}

	if (luTagAttrs & ChHtmlTag::attrHasArguments)
	{
											// This tag can have arguments

		ChArgList	argList[tagMaxArgs];
		int			iArgCount;

		iArgCount = GetArguments( m_iArgStart, argList,	sizeof( argList )/sizeof( ChArgList ));

											/* Even if there is no argument
												call this method, some tags
												set up text style modifiers */

		pStyle->ProcessArguments( argList, iArgCount );
	}
											// Ready to create the font
	CreateStyle( pStyle );

	if ( m_boolLineBreak && ((luTagAttrs & ChHtmlTag::attrStartLine) ||
		(luTagAttrs & ChHtmlTag::attrAddLineAbove) ) )
	{
		int   	iNewLineCount = 0;
		bool	boolSpaceAbove = false;

		if ( m_pHtmlView->GetTextCount() )
		{

			int  		idObjType;
			chuint32 	luAttr;
			ChSize		objSize;
			ChRect 		objSpaceExtra;
			GetObjectAttrs( idObjType, luAttr, objSize, objSpaceExtra );

			if (idObjType )
			{
				if ( luAttr & ChTxtWnd::objAttrBreak )
				{
					boolSpaceAbove = ( idObjType == ChTextObject::objectSpace ||
								 objSpaceExtra.bottom != 0);
				}
			}
	
			if ( !boolSpaceAbove )
			{
				if ( m_pHtmlView->GetTextCount() > 0L &&
								 m_pHtmlView->GetChar( m_pHtmlView->GetTextCount() - 1 ) == TEXT( '\r' ) )
				{
					 iNewLineCount++;
					if ( m_pHtmlView->GetTextCount() > 1
							&& m_pHtmlView->GetChar( m_pHtmlView->GetTextCount() - 2 ) == TEXT( '\r' ) )
					{
						iNewLineCount++;
					}
				}
			}
		}
		else
		{
			iNewLineCount++;
		}


		if ( luTagAttrs & ChHtmlTag::attrStartLine &&
			 !( luTagAttrs & ChHtmlTag::attrAddLineAbove ) &&
									iNewLineCount == 0  && (!boolSpaceAbove) )
		{ 	// if this tag starts a newline then do so if the previous
		    // line was not terminated
			iNewLineCount++;
			m_pstrLocalBuffer[m_iBufIndex++] = TEXT( '\r' );
			m_pHtmlView->AppendTextRun( GetBuffer(), GetBufferIndex(), NULL );
			SetBufferIndex( 0 );
		}

		if ( luTagAttrs & ChHtmlTag::attrAddLineAbove
						&& iNewLineCount < 2  )
		{

			if ( boolSpaceAbove && m_boolLastAddSpaceAbove )
			{  // don't add if it was done before
				boolSpaceAbove = false;
			}
			else
			{
				boolSpaceAbove = true;
			}

			if ( boolSpaceAbove )
			{

				chint32  iHeight = (pStyle->GetPointSize() * m_pHtmlView->GetSettings()->GetPixelHt())/72;
				iHeight =  (iHeight * 30)/100; // 30 %
				if ( iHeight < 0 )
				{
					iHeight *= -1;
				}
				ChSize size( 100, (int)iHeight );
				ChRect spaceExtra( 0, 0, 0, 0 );
				ChObjSpace *pObjSpace = new ChObjSpace( size, spaceExtra, ChTxtWnd::objAttrBreak );
				ChTxtObject	 spaceObject( pObjSpace );
				spaceObject.SetStyle( ChTxtWnd::textObject | GetTextStyle()->GetStyle() );
				m_pHtmlView->AppendObject( &spaceObject );

				m_strLastChar = TEXT( '\r' );

				m_boolLastAddSpaceAbove = true;
				m_boolLastAddSpaceBelow = false;
			}

		}
	}
	else if ( (luTagAttrs & ChHtmlTag::attrStartLine) ||
		(luTagAttrs & ChHtmlTag::attrAddLineAbove )	)
	{	// <LI> tag disable line break temporarily if a new tag follows <LI> tag 
		// without and text
		LineBreak();
	}

	if ( boolRestore )
	{
		ProcessTerminationTag( pStyle );
	}

	return;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::ProcessTerminationTag

------------------------------------------------------------------------------
 This method process all the arguments for a given tag and returns the
 new index where the processing stopped.

 Returns the new state of the HTML viewer

----------------------------------------------------------------------------*/
int ChHtmlParser::ProcessTerminationTag( ChHtmlTag* pStyle )
{

	chuint32	luTagAttrs  = pStyle->GetAttributes();

	if ( luTagAttrs & ChHtmlTag::attrSaveState )
	{  	// Check for bad HTML tag
		int 	iIndex = HTMLStack().GetTopIndex();
		ChStackData* pdataTerm = 0;
		bool		boolFound = iIndex == 0;

		while( iIndex > 0 )
		{
			pdataTerm = HTMLStack().Peek( iIndex );

			if ( pdataTerm->iType == pStyle->GetHTMLId() )
			{		  
				boolFound = true;
				break;
			}
			iIndex--;
		}

		if ( !boolFound )
		{
			pdataTerm = HTMLStack().Peek( HTMLStack().GetTopIndex() );
			return pdataTerm->iType;
		}
	}

	if ( GetBufferIndex() )
	{
		m_pHtmlView->AppendTextRun( GetBuffer(), GetBufferIndex(), NULL );
		SetBufferIndex( 0 );
	}


	if ( luTagAttrs & ChHtmlTag::attrTerminateLine
			||  luTagAttrs & ChHtmlTag::attrAddLineBelow )
	{

		int 	iNewLineCount = 0;
		bool 	boolSpacebelow = false;

		if ( m_pHtmlView->GetTextCount() )
		{

			int  		idObjType;
			chuint32 	luAttr;
			ChSize		objSize;
			ChRect 		objSpaceExtra;

			GetObjectAttrs( idObjType, luAttr, objSize, objSpaceExtra );

			if ( idObjType )
			{
				if ( luAttr & ChTxtWnd::objAttrBreak )
				{
					boolSpacebelow = ( idObjType == ChTextObject::objectSpace ||
								 objSpaceExtra.bottom != 0);
				}
			}
		}

		if ( m_pHtmlView->GetTextCount() > 0 &&	!boolSpacebelow  &&
						 m_pHtmlView->GetChar( m_pHtmlView->GetTextCount() - 1 ) == TEXT( '\r' ) )
		{	 
			iNewLineCount++;
			if ( m_pHtmlView->GetTextCount() > 1
					&& m_pHtmlView->GetChar( m_pHtmlView->GetTextCount() - 2 ) == TEXT( '\r' ) )
			{
					iNewLineCount++;
			}
		}
		else if ( !m_pHtmlView->GetTextCount() )
		{
			iNewLineCount++;
		}


		// terminate the line
		if ( luTagAttrs & ChHtmlTag::attrTerminateLine	&& !boolSpacebelow 
				&& !( luTagAttrs & ChHtmlTag::attrAddLineBelow )
					&& iNewLineCount == 0)
		{
			m_pstrLocalBuffer[m_iBufIndex++] = TEXT( '\r' );
			m_pHtmlView->AppendTextRun( GetBuffer(), GetBufferIndex(), NULL );
			SetBufferIndex( 0 );
		}

		if ( luTagAttrs & ChHtmlTag::attrAddLineBelow && iNewLineCount < 2 )
		{
			if ( boolSpacebelow && m_boolLastAddSpaceBelow )
			{
				boolSpacebelow = false;
			}
			else
			{
				boolSpacebelow = true;
			}

			if ( boolSpacebelow )
			{
				m_boolLastAddSpaceBelow = true;
				m_boolLastAddSpaceAbove = false;

				chint32  iHeight = (pStyle->GetPointSize() * m_pHtmlView->GetSettings()->GetPixelHt())/72;
				iHeight =  (iHeight * 30)/100; // 30 %
				if ( iHeight < 0 )
				{
					iHeight *= -1;
				}
				ChSize size( 100, (int)iHeight );
				ChRect spaceExtra( 0, 0, 0, 0 );
				ChObjSpace *pObjSpace = new ChObjSpace( size, spaceExtra, ChTxtWnd::objAttrBreak );
				ChTxtObject	 spaceObject( pObjSpace );
				spaceObject.SetStyle( ChTxtWnd::textObject | GetTextStyle()->GetStyle() );
				m_pHtmlView->AppendObject( &spaceObject );

				m_strLastChar = TEXT( '\r' );
			}
		}
	}



	if ( luTagAttrs & ChHtmlTag::attrSaveState )
	{  	// restore state if any
		if ( HTMLStack().GetTopIndex() )
		{
			RestoreTextStyle( pStyle->GetHTMLId() );
		}
		else
		{
			ChStackData* pdataTerm;

			pdataTerm = HTMLStack().Peek( 0 );
			GetTextStyle()->SetLeftIndent( pdataTerm->iLeftIndent );
			GetTextStyle()->SetStyle( pdataTerm->luStyle );
			GetTextStyle()->SetTextColor( GetTextColor() );
			GetTextStyle()->SetBackColor( GetTextStyle()->GetBackColor() );
			GetTextStyle()->SetLineWidth( pdataTerm->iLineWidth );
			GetTextStyle()->SetLineHeight( pdataTerm->iLineHeight );
			GetTextStyle()->SetUserData( pdataTerm->userData );
			GetTextStyle()->SetPointSize( pdataTerm->iPointSize );
		}
	}


	if ( luTagAttrs & ChHtmlTag::attrCallEnd )
	{  // Call End of tag 
		pStyle->EndTag();
	}

	// restore style
	ChStackData* pdataTop = HTMLStack().Peek( HTMLStack().GetTopIndex() );

	ChHtmlTag *pCurrStyle = GetHTMLTagInstance( pdataTop->iType );

	CreateStyle( pCurrStyle );

	return pdataTop->iType;
}




/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::CreateStyle

------------------------------------------------------------------------------
This method creates a new font if necessary based on the current state.

----------------------------------------------------------------------------*/
void  ChHtmlParser::CreateStyle( ChHtmlTag *pStyle )
{
#ifdef CH_MSW
	LOGFONT		logFont, logTemp;
	chuint32 	luAttrs = pStyle->GetAttributes();

	GetHTMLFont()->GetObject( sizeof( LOGFONT ), &logFont );

	logTemp = logFont;  // save the current attrs of the font

	if ( luAttrs & ChHtmlTag::attrHasFontName )
	{
		if ( luAttrs & ChHtmlTag::attrFontProportional )
		{ 
			lstrcpy( logFont.lfFaceName, 
					m_pHtmlView->GetSettings()->GetProportionalFontName() );
		}
		else
		{
			lstrcpy( logFont.lfFaceName, 
					m_pHtmlView->GetSettings()->GetFixedFontName() );
		}
	}
	else
	{
		if ( m_iFontFixed )
		{
			lstrcpy( logFont.lfFaceName, 
					m_pHtmlView->GetSettings()->GetFixedFontName() );
		}
		else
		{
			lstrcpy( logFont.lfFaceName, 
					m_pHtmlView->GetSettings()->GetProportionalFontName() );
		}

	}


	if ( GetTextStyle()->GetPointSizeExtra() )
	{
		logFont.lfHeight = GetTextStyle()->GetPointSizeExtra();
	}
	else
	{
		if ( luAttrs & ChHtmlTag::attrHasFontSize )
		{
			logFont.lfHeight = pStyle->GetPointSize();
			GetTextStyle()->SetPointSize( pStyle->GetPointSize() );

		}
		else
		{
			logFont.lfHeight = GetTextStyle()->GetPointSize();
		}
	}

	if ( lstrcmp( m_pHtmlView->GetSettings()->GetFixedFontName(), 
							logFont.lfFaceName ) == 0 )
	{
		logFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		logFont.lfWeight = FW_LIGHT;
	}
	else
	{
		logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
		logFont.lfWeight = FW_MEDIUM;
	}


	if ( m_iFontItalic )
	{
		logFont.lfItalic = true;
	}
	else
	{
		logFont.lfItalic = false;
	}

	if ( m_iFontBold )
	{
		logFont.lfWeight = FW_BOLD;
	}

	if ( m_iFontUnderline )
	{
		logFont.lfUnderline = true;
	}
	else
	{
		logFont.lfUnderline = false;
	}

	if ( m_iFontStrikethrough )
	{
		logFont.lfStrikeOut = true;
	}
	else
	{
		logFont.lfStrikeOut = false;
	}
	// create only if things have changed
	if ( ChMemCmp( &logFont, &logTemp, sizeof( logFont))  )
	{

		HFONT hFont = (HFONT)GetHTMLFont()->Detach();
		::DeleteObject( hFont );
		hFont = ::CreateFontIndirect( &logFont );
		GetHTMLFont()->Attach( hFont );
	}

#else
	char *lfFaceName;
	char *lfWeight;
	char *lfSlant;
	int lfHeight;

	if ( pStyle->GetAttributes() & ChHtmlParser::attrHasFontName ) {
		lfFaceName = pStyle->GetFontName();
	} else {
		lfFaceName = "*";
	}

	if ( pStyle->GetAttributes() & ChHtmlParser::attrHasFontSize ) {
		lfHeight = pStyle->GetPointSize();
	} else {
		lfHeight = GetTextStyle()->GetPointSize();
	}

	if ( lstrcmp( ChHtmlParser::GetFixedFontName(), lfFaceName ) == 0 ) {
		lfFaceName = "courier";
		lfWeight = "medium";
	} else {
		lfFaceName = "times";
		lfWeight = "medium";
	}

	if ( m_iFontItalic )
	{
		lfSlant = "i";
	}
	else
	{
		lfSlant = "r";
	}

	if ( m_iFontBold )
	{
		lfWeight = "bold";
	}

// XXX underline not supported.
// XXX strikethrough not supported.

	char font_specifier[1024];

	sprintf( font_specifier, "-*-%s-%s-%s-*-*-*-%d-*-*-*-*-iso8859-*",
			 lfFaceName, lfWeight, lfSlant, lfHeight);

	GetHTMLFont()->CreateFontIndirect( font_specifier );
#endif

}

void ChHtmlParser::ResetStack( int iToken )
{
	ChHtmlTag *pStyle = GetHTMLTagInstance( iToken );
	ASSERT( pStyle );


	m_iFontItalic = m_iFontBold = m_iFontFixed =
	m_iFontUnderline = m_iFontStrikethrough = 0;
	m_iBaseFontSize = baseFontSize;

	m_boolLastAddSpaceAbove = m_boolLastAddSpaceBelow = false;
	m_boolLineBreak = true;
	m_strLastChar = TEXT( ' ' );

	SetLineCharCount( 0 );



	int 		nFontSize = pStyle->GetPointSize();
	chuint32 	luAttrs = pStyle->GetAttributes();


	if (!m_pcurrFont)
	{
		// set up the default font
		m_pcurrFont = new ChFont();
		ASSERT( m_pcurrFont );


		#ifdef CH_MSW
			LOGFONT	lf;
			ChMemClearStruct( &lf );

			lf.lfHeight 		= nFontSize;
			lf.lfWeight 		= FW_LIGHT;
			lf.lfCharSet 		= ANSI_CHARSET;
			lf.lfOutPrecision 	= OUT_STROKE_PRECIS;
			lf.lfClipPrecision 	= CLIP_STROKE_PRECIS;
			lf.lfQuality 		= DEFAULT_QUALITY;
			lf.lfPitchAndFamily = FF_DONTCARE;
	
			if ( luAttrs & ChHtmlTag::attrFontProportional )
			{ 
				lstrcpy( lf.lfFaceName, 
						m_pHtmlView->GetSettings()->GetProportionalFontName() );
			}
			else
			{
				lstrcpy( lf.lfFaceName, 
						m_pHtmlView->GetSettings()->GetFixedFontName() );
			}

			m_pcurrFont->CreateFontIndirect( &lf );

		#else
			m_pcurrFont->CreateFontIndirect( "fixed" );
			// XXX Should we use something other than "fixed"?
		#endif
	}

	// set up the run style
	if ( m_ptxtStyle )
	{
		delete m_ptxtStyle;
	}

	m_ptxtStyle = new ChTextStyle( m_pcurrFont );
	ASSERT( m_ptxtStyle );
	m_ptxtStyle->SetStyle( ChTxtWnd::textLeft );
	m_ptxtStyle->SetTextColor( GetTextColor() );
	m_ptxtStyle->SetLeftIndent( 0 );

	m_ptxtStyle->SetPointSize( nFontSize );

	CreateStyle( pStyle );

	if ( luAttrs & ChHtmlTag::attrCallStart )
	{  // modifies the current font	 ?

		pStyle->StartTag( );

	}

	for( int i = HTMLStack().GetTopIndex(); i >= 0; i-- )
	{
		HTMLStack().Pop();
	}

	// setup the stack
	ChStackData data;

	ChMemClearStruct( &data );
	data.iType = iToken;

	data.iLeftIndent	= GetTextStyle()->GetLeftIndent();
	data.luStyle 		= GetTextStyle()->GetStyle( );
	data.luForeColor 	= GetTextColor();
	data.luBackColor 	= GetTextStyle()->GetBackColor(  );
	data.iLineWidth 	= GetTextStyle()->GetLineWidth( );
	data.iLineHeight	= GetTextStyle()->GetLineHeight( );
	data.userData 		= GetTextStyle()->GetUserData(  );
	data.iPointSize 	= pStyle->GetPointSize();

	HTMLStack().Push( data );


}

void ChHtmlParser::UpdateColors(  chuint32 luOldTextColor,
								 chuint32 luOldBkColor )
{ 

	// Set all colors to default
	SetTextColor( m_pHtmlView->GetSettings()->GetTextColor() );			
	SetLinkColor( m_pHtmlView->GetSettings()->GetLinkColor() );			
	SetVisitedLinkColor( m_pHtmlView->GetSettings()->GetVistedLinkColor() )	;
	SetActiveLinkColor( m_pHtmlView->GetSettings()->GetActiveLinkColor() );	
	SetPrefetchedLinkColor( m_pHtmlView->GetSettings()->GetPrefetchedLinkColor() );

	for( int i = HTMLStack().GetTopIndex(); i >= 0; i-- )
	{
		ChStackData*pdata = HTMLStack().Peek( i );

		if ( !(pdata->luForeColor & CH_COLOR_DEFAULT)
				&& pdata->luForeColor == luOldTextColor )
		{
		 	pdata->luForeColor = GetTextColor();
		}
		if ( !(pdata->luBackColor & /*luOldBkColor &*/ CH_COLOR_DEFAULT)
				&& pdata->luBackColor == luOldBkColor )
		{
			pdata->luBackColor = m_pHtmlView->GetSettings()->GetBackColor();
		}

	}

	if ( m_ptxtStyle )
	{
		m_ptxtStyle->SetTextColor( GetTextColor() );
	}
}



void ChHtmlParser::GetObjectAttrs( int& idObj, chuint32& luAttr,
									ChSize& sizeObj, ChRect& spaceExtra, int lIndex /* = -1 */ )
{
	idObj = 0;
	luAttr  = 0;

	if ( lIndex == - 1)
	{
		lIndex = m_pHtmlView->GetTextCount() - 1	;
	}

	if ( lIndex >= 0 )
	{

		pChRun			pRun;
	 	pChStyleInfo	pStyleUse;

		pStyleUse 	= m_pHtmlView->GetStyleTable();
	 	pRun 		= m_pHtmlView->GetRunTable();

		pRun += m_pHtmlView->GetRunIndex(  lIndex );	  
		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{
			ChTextObject** ppObject = &(m_pHtmlView->GetObjectTable()[pStyleUse
						[ pRun->lStyleIndex ].style.iObjectIndex]);
			luAttr = (*ppObject)->GetAttrs();
			(*ppObject)->GetSize( sizeObj );
			(*ppObject)->GetSpaceExtra( spaceExtra );
			idObj = (*ppObject)->GetType();
		}
	}

}

bool ChHtmlParser::GetLastObjectAttrs( int idObjType, chuint32& luAttr,
									ChSize& sizeObj, ChRect& spaceExtra )
{
	luAttr  = 0;
	bool	boolFound = false;
	int 	iNextIndex;

	pChRun			pRun;
 	pChStyleInfo	pStyleUse;

	pStyleUse 	= m_pHtmlView->GetStyleTable();
 	pRun 		= m_pHtmlView->GetRunTable();

	for ( int i = m_pHtmlView->GetRunCount() - 1; i >= 0; i-- )
	{
		if ( pStyleUse[ pRun[i].lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{
			ChTextObject** ppObject = &(m_pHtmlView->GetObjectTable()[pStyleUse
						[ pRun[i].lStyleIndex ].style.iObjectIndex]);
			if ( idObjType == (*ppObject)->GetType() )
			{
				luAttr = (*ppObject)->GetAttrs();
				(*ppObject)->GetSize( sizeObj );
				(*ppObject)->GetSpaceExtra( spaceExtra );
				iNextIndex = i + 1;
				boolFound = true;
				break;
			}
		}
		
	}

	if ( boolFound && iNextIndex <  m_pHtmlView->GetRunCount() )
	{
		if ( pStyleUse[ pRun[iNextIndex].lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{
			ChTextObject** ppObject = &(m_pHtmlView->GetObjectTable()[pStyleUse
						[ pRun[iNextIndex].lStyleIndex ].style.iObjectIndex]);

			if ( (*ppObject)->GetAttrs() &  ChTxtWnd::objAttrBreak)
			{
				if ( ChTextObject::objectSpace == (*ppObject)->GetType() )
				{
					ChSize sizeObjNext;
					ChRect spaceExtraNext;
					(*ppObject)->GetSize( sizeObjNext );
					(*ppObject)->GetSpaceExtra( spaceExtraNext );

					spaceExtra.bottom += (spaceExtraNext.top + 
								spaceExtraNext.bottom  + sizeObjNext.cy); 

				}
				else
				{
					ChRect spaceExtraNext;
					(*ppObject)->GetSpaceExtra( spaceExtraNext );
					spaceExtra.top += spaceExtraNext.top;
				}
			}
		}

	}

	return boolFound;

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::GetSymbolFont()

------------------------------------------------------------------------------
	Create a new symbol font and return
----------------------------------------------------------------------------*/

ChFont* ChHtmlParser::GetSymbolFont( ChFont* pFont )
{
	#ifdef CH_MSW
	LOGFONT		logFont;

	pFont->GetObject( sizeof( LOGFONT ), &logFont );

	lstrcpy( logFont.lfFaceName, m_pHtmlView->GetSettings()->GetSymbolFontName() );
	// create the font for this style
	logFont.lfCharSet = SYMBOL_CHARSET;
	logFont.lfWeight = 	FW_HEAVY;
	logFont.lfItalic = false;
	logFont.lfUnderline = false;
	logFont.lfStrikeOut = false;

	if ( m_symbolFont.GetSafeHandle() )
	{

		HFONT hFont = (HFONT)m_symbolFont.Detach();
		::DeleteObject( hFont );
	}
	HFONT hFont = ::CreateFontIndirect( &logFont );
	m_symbolFont.Attach( hFont );
	#else
	m_symbolFont.CreateFontIndirect( "*symbol*" );
	#endif

	return( &m_symbolFont );
}

// $Log$
