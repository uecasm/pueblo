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

	This file contains the implementation all body elements.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChBodyElement.h"

#include <MemDebug.h>

 // 	processed in this file
 // BODY - 
 // H1-H6 - 
 // P -
 // PRE -

 // Treated has unknowns



///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChBodyElement implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChBodyTag::ChBodyTag( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	 m_iTokenID = HTML_BODY;
	 m_luAttrs =  attrHasArguments | attrCallProcessTag | attrTrimRight;
}


void ChBodyTag::ProcessArguments( 	pChArgList pList, int iArgCount ) 
{

	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_LINK :
			{
				m_pParser->SetLinkColor( BGR( pList[i].argVal ) );
				break;
			}
			case ARG_VLINK :
			{
				m_pParser->SetVisitedLinkColor( BGR( pList[i].argVal ) );
				break;
			}
			case ARG_ALINK :
			{
				m_pParser->SetActiveLinkColor( BGR(pList[i].argVal) );
				break;
			}
			case ARG_PLINK :
			{
				m_pParser->SetPrefetchedLinkColor( BGR( pList[i].argVal ) );
				break;
			}
			case ARG_COLOR :
			case ARG_FGCOLOR :
			case ARG_TXTCOLOR :
			{
				m_pParser->SetTextColor( BGR(pList[i].argVal) );
				m_pParser->GetTextStyle()->SetTextColor( BGR(pList[i].argVal) );

				break;
			}
			case ARG_BGCOLOR :
			{
				m_pParser->SetBackColor( BGR(pList[i].argVal) );
				break;
			}
			case ARG_BACKGROUND :
			{
				ChString		strURL( (const char*)pList[i].argVal );
				// turn off draw till we will turn it on once we get the
				// background pattern or if the pattern load fails
				//m_pParser->SetRedraw( false );
				//Load the pattern
				m_pParser->LoadBkPattern( strURL );
				break;
			}
			case ARG_ALIGN :
			{
				if ( pList[i].argVal == VAL_MIDDLE )
				{	// Center the document vertically
					GetHtmlView()->SetDocumentAttrs(
							GetHtmlView()->GetDocumentAttrs() | ChTxtWnd::docVCenter ); 	
				}
				break;
			}
			default :
			{
				break;
			}
		}
	}
	return;
}


chint32 ChBodyTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{
	#if 0
	if ( !(GetHtmlView()->GetDocumentAttrs() & ChTxtWnd::docVCenter) )
	{
		// Add some space at the top of the document
		ChSize size( 100, (int)m_pParser->GetLeftIndent() );
		ChRect spaceExtra( 0, 0, 0, 0 );
		ChObjSpace *pObjSpace = new ChObjSpace( size, spaceExtra, ChTxtWnd::objAttrBreak );
		ChTxtObject	 spaceObject( pObjSpace );
		spaceObject.SetStyle( ChTxtWnd::textObject | ChTxtWnd::textLeft );
		GetHtmlView()->AppendObject( &spaceObject );
	}
	#endif

	m_pParser->ResetStack( HTML_PARAGRAPH );


	return ChHtmlTag::ProcessTag( pstrBuffer, lStart, lCount );
}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChBodyElements : generic body element 
////////////
///////////////////////////////////////////////////////////////////////////////


ChBodyElements::ChBodyElements( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{

}


void  ChBodyElements::StartTag( )
{
	if ( m_luFontAttrs & fontItalic )
	{
		m_pParser->IncrementItalic();
	}
	
	if ( m_luFontAttrs & fontBold )
	{
		m_pParser->IncrementBold();
	}
	
	if ( m_luFontAttrs & fontUnderline )
	{
		m_pParser->IncrementUnderline();
	}
	
	if ( m_luFontAttrs & fontStrikethrough )
	{
		m_pParser->IncrementStrikethrough();
	}
	if ( m_luAttrs & attrFontFixed )
	{
		m_pParser->IncrementFixed();
	}
}

void  ChBodyElements::EndTag(  )
{
	if ( m_luFontAttrs & fontItalic )
	{
		m_pParser->DecrementItalic();
	}
	
	if ( m_luFontAttrs & fontBold )
	{
		m_pParser->DecrementBold();
	}
	
	if ( m_luFontAttrs & fontUnderline )
	{
		m_pParser->DecrementUnderline();
	}
	
	if ( m_luFontAttrs & fontStrikethrough )
	{
		m_pParser->DecrementStrikethrough();
	}

	if ( m_luAttrs & attrFontFixed )
	{
		m_pParser->DecrementFixed();
	}

}


chint32 ChBodyElements::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{

	// The text is pre-formatted
	if ( m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat )
	{
		return ProcessPreformatText( pstrBuffer, lStart, lCount );
	}

	if ( lStart >= lCount )
	{
		return lStart;
	}

	
	// get the text into our buffer
	while ( lStart < lCount )
	{
		if ( !IS_NEW_TAG( pstrBuffer[lStart] ) )
		{
			char	strChar = pstrBuffer[lStart++];

			if ( !IS_WHITE_SPACE( strChar )	)
			{ 
				if ( strChar == TEXT( '&' ) )
				{
					if ( m_pParser->MapEntityToChar( pstrBuffer, 
													--lStart, lCount, strChar ) )
					{
						m_pParser->AppendChar( strChar  );
					}
				}
				else
				{
					m_pParser->AppendChar( strChar  );
				}
			}
			else if ( IS_WHITE_SPACE( strChar ) )
									//&& m_pParser->GetLastChar() != TEXT( ' ' )  )
			{
				if ( !IS_WHITE_SPACE( m_pParser->GetLastChar() ) )
				{
					m_pParser->AppendChar( TEXT( ' ' ) );
				}
				// skip till the next non-white space
				while ( lStart < lCount && IS_WHITE_SPACE( pstrBuffer[lStart] ) )
				{
					lStart++;
				}
			}
		}
		else
		{
			if ( m_pParser->BreakTag( pstrBuffer, lStart, lCount ) )
			{ 
				break;
			}
		}
	}

	return lStart;
}

chint32 ChBodyElements::ProcessPreformatText( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{

	if ( lStart >= lCount )
	{
		return lStart;
	}
	
	
	// get the header text into our buffer
	while ( lStart < lCount )
	{
		if ( !IS_NEW_TAG( pstrBuffer[lStart] ) )
		{
			char strChar = pstrBuffer[lStart++];

			if ( strChar >= TEXT( ' ' ) )
			{
				if ( strChar == TEXT( '&' ) )
				{
					if ( m_pParser->MapEntityToChar( pstrBuffer, 
													--lStart, lCount, strChar ) )
					{
						m_pParser->AppendChar( strChar  );
					}
				}
				else
				{
					m_pParser->AppendChar( strChar  );
				}
			}
			else
			{
				switch ( strChar )
				{
					case  TEXT( '\n' ) :
					{
						if ( m_pParser->GetLastChar() != TEXT( '\r' ) )
						{
							m_pParser->AppendChar( TEXT( '\r' )  );
							m_pParser->SetLineCharCount( 0 );
						}
						break;
					}
					case  TEXT( '\r' ) :
					{
						m_pParser->AppendChar( TEXT( '\r' )  );
						m_pParser->SetLineCharCount( 0 );
						break;
					}
					case  TEXT( '\t' ) :
					{
						int iAddSpace = 8 - ( m_pParser->GetLineCharCount() % 8 );
						for ( int i = 0; i < iAddSpace; i++ )
						{
							m_pParser->AppendChar( TEXT( ' ' )  );
						}
						break;
					}
					default :
					{
						m_pParser->AppendChar( strChar  );
						break;
					}
				}
			}
		}
		else
		{
			if ( m_pParser->BreakTag( pstrBuffer, lStart, lCount ) )
			{ 
				break;
			}
		}
	} 
	

	return lStart;
}

// $Log$
