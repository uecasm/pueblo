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

	This file contains the interface for the ChHtmlTag class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChPlainTag.h"

#include <MemDebug.h>


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChPlainTextElement implementation 
////////////
///////////////////////////////////////////////////////////////////////////////



ChPlainTextElement::ChPlainTextElement( int iTokenID, ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = iTokenID;
	m_luAttrs = attrSaveState | 
				attrTerminateLine | attrStartLine |
				attrHasFontName | attrHasFontSize |
				attrAddLineAbove | attrAddLineBelow |
				attrCallStartAndEnd | attrFontFixed
				| attrCallProcessTag;

	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize | restoreLineFmt;
}

void  ChPlainTextElement::StartTag(  )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle |=  ( ChTxtWnd::textNoWrap | ChTxtWnd::textPreFormat );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->IncrementFixed();



}
void  ChPlainTextElement::EndTag( )
{
	//chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	//luStyle &=  ~( ChTxtWnd::textNoWrap | ChTxtWnd::textPreFormat );

	//m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->DecrementFixed();

}

chint32  ChPlainTextElement::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{


	if (lStart >= lCount)
	{
		return lStart;
	}

	
											/* Get the text into our
												buffer */
	while ( lStart < lCount )
	{
		if ( !IS_NEW_TAG( pstrBuffer[lStart] ) )
		{
			char strChar = pstrBuffer[lStart++];

			if ( strChar >= TEXT( ' ' ) )
			{
				m_pParser->AppendChar( strChar  );
			}
			else
			{
				switch ( strChar )
				{
					case  TEXT( '\n' ) :
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
			
				bool	boolEnd;
				int iToken = m_pParser->LookUpTag( pstrBuffer, lStart, lCount, boolEnd );
				if ( m_iTokenID == iToken  && boolEnd )
				{ 
					m_pParser->CommitBuffer();
					EndTag();
					m_pParser->RestoreTextStyle( iToken );
					// remove the token
					while( lStart < lCount &&  pstrBuffer[lStart++] != TEXT( '>' ));
				
					// remove any trailing new lines
					while( lStart < lCount &&  (  pstrBuffer[lStart] == TEXT( '\r' )
												|| pstrBuffer[lStart] == TEXT( '\n' )) )
					{
						++lStart;
					}
					break;
				}
				else
				{
					m_pParser->AppendChar( pstrBuffer[lStart++]  );
				}
			}

		}
	} 
	
	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChMudTextTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChMudTextTag::ChMudTextTag(  ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_MUDTEXT;
	m_luAttrs = attrSaveState | attrParseWhiteSpace   |
				attrHasFontName | attrHasFontSize |
				attrFontFixed | attrCallProcessTag |
				attrCallStartAndEnd;
	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize | restoreLineFmt;
}

void  ChMudTextTag::StartTag(  )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle |=  ( ChTxtWnd::textPreFormat );

 	luStyle &=  ~( ChTxtWnd::textNoWrap );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->IncrementFixed();

}
void  ChMudTextTag::EndTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle &=  ~( ChTxtWnd::textPreFormat );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->DecrementFixed();


}


chint32  ChMudTextTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{

	if (lStart >= lCount)
	{
		return lStart;
	}

	
											/* Get the text into our
												buffer */
	while ( lStart < lCount )
	{
		if ( !IS_NEW_TAG( pstrBuffer[lStart] ) )
		{
			char strChar = pstrBuffer[lStart++];

			if ( strChar >= TEXT( ' ' ) )
			{
				m_pParser->AppendChar( strChar  );
			}
			else
			{
				switch ( strChar )
				{
					case  TEXT( '\n' ) :
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
			
				bool	boolEnd;
				int iToken = m_pParser->LookUpTag( pstrBuffer, lStart, lCount, boolEnd );
				if ( m_iTokenID == iToken  && boolEnd )
				{ 
					m_pParser->CommitBuffer();
					EndTag();
					m_pParser->RestoreTextStyle( iToken );
					// remove the token
					while( lStart < lCount &&  pstrBuffer[lStart++] != TEXT( '>' ));
					break;
				}
				else
				{
					m_pParser->AppendChar( pstrBuffer[lStart++]  );
				}
			}

		}
	} 
	

	return lStart;
}

// $Log$
