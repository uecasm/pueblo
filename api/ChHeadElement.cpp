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

	This file contains the implementation for the ChHeadTag class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChHeadElement.h"

#include <MemDebug.h>


 // Head 	processed in this file
 // HTML - 
 // HEAD - 
 // BASE -
 // TITLE -
 // ! Comment

 // Treated has unknowns
 // LINK
 // META
 // ISINDEX
 // NEXTID
 // RANGE
 // STYLE



///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChHtmlTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChHtmlElement::ChHtmlElement( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_HTML;
	m_luAttrs = attrCallStart | attrCallProcessTag | attrTrimRight;
}

void ChHtmlElement::StartTag( )
{
	// reset the stack
	m_pParser->ResetStack( HTML_PARAGRAPH );
}

chint32 ChHtmlElement::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{

	// skip all white space after the tag
	while ( lStart < lCount && pstrBuffer[lStart] != TEXT( '<' )  
						&& IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	{
		lStart++;
	}

	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChHeadTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChHeadTag::ChHeadTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser ) 
{
	m_iTokenID = HTML_HEAD;
	m_luAttrs = attrCallStart | attrCallProcessTag | attrTrimRight;
}

void ChHeadTag::StartTag( )
{
	// reset the stack
	m_pParser->ResetStack( HTML_PARAGRAPH );
}

chint32 ChHeadTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{


	// skip all white space after the tag
	while ( lStart < lCount && pstrBuffer[lStart] != TEXT( '<' )  
						&& IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	{
		lStart++;
	}

	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChBaseTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChBaseTag::ChBaseTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_BASE;
	m_luAttrs = attrHasArguments | attrCallProcessTag | attrTrimRight;
}

void ChBaseTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	for( int i = 0; i < iArgCount; i++ )
	{
		if ( pList[i].iArgType == ARG_HREF )
		{
			m_pParser->SetDocBaseURL((char*)pList[i].argVal );
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChTitleTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChTitleTag::ChTitleTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_TITLE;
	m_luAttrs = attrCallStart | attrCallProcessTag | attrTrimRight;

}


void ChTitleTag::StartTag(  )
{
	// reset the stack
	m_pParser->ResetStack( HTML_PARAGRAPH );
}

chint32 ChTitleTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{


	if ( lStart < lCount )
	{ // get the title into our buffer
		while ( lStart < lCount )
		{
			if ( pstrBuffer[lStart] != TEXT( '<' ))
			{
				m_pParser->AppendChar( pstrBuffer[lStart++] );
			}
			else
			{
				bool	boolEnd = false;
				int		iToken;

				iToken = m_pParser->LookUpTag( pstrBuffer, lStart, lCount, boolEnd );

				if (HTML_TITLE == iToken && boolEnd)
				{ // skip till end of token
					
					break;
				}
				else if ( iToken == -1 )
				{
					m_pParser->AppendChar( pstrBuffer[lStart++]  );
				}
				else
				{
					break;
				}
			}
		}
	} 
	
	if ( m_pParser->GetBufferIndex() )
	{
		m_pParser->GetBuffer()[ m_pParser->GetBufferIndex()] = 0;
		// Set the HTML title
		ChString strTitle(m_pParser->GetBuffer() );
		strTitle.TrimLeft(); 
		strTitle.TrimRight(); 
		m_pParser->SetDocumentTitle( strTitle );	
	} 

	m_pParser->SetBufferIndex( 0 );

	return lStart;
}



///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChCommentTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChCommentTag::ChCommentTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID  = HTML_COMMENT;
	m_luAttrs 	= attrCallProcessTag;
}


chint32 ChCommentTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )  
{
	// skip all the newlines 
	while ( lStart < lCount  
				&& ( pstrBuffer[lStart] == TEXT( '\r' )
					 || pstrBuffer[lStart] == TEXT( '\n' ) ) )
	{
		lStart++;
	}

	return lStart;			
}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChPrefetchTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChPrefetchTag::ChPrefetchTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_PREFETCH;
	m_luAttrs = attrHasArguments | attrTrimRight;
}


void ChPrefetchTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
#if !defined( CH_PUEBLO_PLUGIN )
	// For plugin do not perform any prefetch.

	// allocate buffer for the argument
	int iArgSize = m_pParser->GetArgSize();
	if ( iArgSize )
	{

	   ChHTMLPrefetch *pFetch = new ChHTMLPrefetch;
	   ASSERT( pFetch ); 

		for( int i = 0; i < iArgCount; i++ )
		{
			switch ( pList[i].iArgType )
			{
				case ARG_HREF:
				{
					pFetch->SetHREF((char*)pList[i].argVal );
					break;
				}
				case ARG_XPROBABLITY :
				{
					pFetch->SetProbablity( (int)pList[i].argVal );
					break;
				}
				default :
				{
					break;
				}
			}
		}
		// sanity check
		if ( pFetch->GetProbablity() && pFetch->GetHREF() )
		{
			char* pstrBuf = new char[iArgSize+1];
			ASSERT( pstrBuf );
			ChMemCopy( pstrBuf,  m_pParser->GetArgBuffer(), iArgSize );
			pstrBuf[iArgSize] = 0;
			pFetch->SetArg( pstrBuf );
			// insert it into our list
			GetHtmlView()->GetPrefetchList().AddTail( pFetch );
		}
		else
		{ // this is a bogus prefetch tag !!!!
			delete pFetch;
		}
		 
	}
#endif // !defined( CH_PUEBLO_PLUGIN )
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChPageTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChPageTag::ChPageTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_PAGE;
	m_luAttrs = attrHasArguments | attrTrimRight;
}


void ChPageTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	int iType = VAL_CLEAR_LINK;
	chuint32 luColor = m_pParser->GetTextColor();
	chuint32 luClear = 0;

	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_CLEAR :
			{
				luClear = pList[i].argVal;
				break;
			}
			case ARG_TXTCOLOR :
			case ARG_FGCOLOR :
			case ARG_COLOR :
			{
				luColor = BGR( pList[i].argVal );
				break;
			}
			default :
			{
				break;
			}
		}
	}
	if ( luClear & clearText )
	{  	// clear the text
		GetHtmlView()->NewPage();
		// New page puts it into PREFORMAT, reset it back to PARAGRAPH mode
		m_pParser->ResetStack( HTML_PARAGRAPH );
		luClear = 0;  // zap all the other flags
	}

	if ( luClear & clearLinks )
	{
		GetHtmlView()->DisableHotSpots( luColor );
	}
	
	if ( luClear & clearForms )
	{
		GetHtmlView()->ClearForms();
	}

	if ( luClear & clearPlugins )
	{
		GetHtmlView()->UnloadPlugins();
	}

	if ( luClear & clearImages )
	{
		GetHtmlView()->UnloadImages();
	}
}

// $Log$
