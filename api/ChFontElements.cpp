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

	This file contains the implementation for the ChHeadElement class.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChHtmlSettings.h>
#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChFontElements.h"

#include <MemDebug.h>




///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChBaseFont implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChBaseFont::ChBaseFont( ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = HTML_BASEFONT;
	m_luAttrs =  attrHasArguments | attrTrimRight;
}


void ChBaseFont::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	for( int i = 0; i < iArgCount; i++ )
	{
		if ( pList[i].iArgType == ARG_SIZE )
		{
			int iSize = (int )pList[i].argVal;
			iSize = ((iSize > 7) ? 7 : ((iSize < 0 )? 0 : iSize));
			m_pParser->SetBaseFontSize( iSize );

		}
	}

}



///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChCharStyle implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChCharStyle::ChCharStyle( int iIDToken, ChHtmlParser *pParser ) : ChHtmlTag( pParser )
{
	m_iTokenID = iIDToken;
	m_luAttrs =  attrCallStartAndEnd;

	switch( m_iTokenID )
	 {
		// logical styles
		case HTML_BOLD :
		// physical styles
		case HTML_STRONG :
		{
			m_luFontAttrs = fontBold;
			break;
		}
		// physical styles
		case HTML_EMPHASIS :
		case HTML_CITE :
		case HTML_ITALIC :
		{
			m_luFontAttrs = fontItalic;
			break;
		}
		case HTML_UNDERLINE :
		{
			m_luFontAttrs = fontUnderline;
			break;
		}
		case HTML_STRIKETHROUGH :
		{
			m_luFontAttrs = fontStrikethrough;
			break;
		}
		default :
		{
			ASSERT( false );
			break;
		}
	}

}

void  ChCharStyle::StartTag( )
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
}
void  ChCharStyle::EndTag( )
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
}

///////////////////////////////////////////////////////////////////////////////
////////////
///////////// ChTeleTypeTag	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////
ChTeleTypeTag::ChTeleTypeTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_TYPEWRITER;
	m_luAttrs =  attrHasFontName | attrFontFixed | attrSaveState
				| attrCallProcessTag | attrCallStartAndEnd;
}


///////////////////////////////////////////////////////////////////////////////
////////////
///////////// ChFontTag	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChFontTag::ChFontTag(  ChHtmlParser *pParser ) : 
						ChBodyElements( pParser )
{
	m_iTokenID = HTML_FONT;

	m_luAttrs = attrSaveState | attrHasArguments 
				| attrCallProcessTag; //  | attrTrimRight;
	// attrs to restore when the tag terminates
	m_luStateToRestore =  restoreTextColor | restoreFontSizeExtra;
}

void  ChFontTag::ProcessArguments( 	pChArgList pList, int iArgCount )
{
	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_HEIGHT :
			case ARG_SIZE :
			{
				chint32	lPointSize = (int )pList[i].argVal;
				bool	boolAbs = false;
								
				if( lPointSize > 0 )
				{
					if ( lPointSize > 7 )	
					{
						lPointSize = 7;	
					}
					// is it absolute or based on BASEFONT value
					if ( !(strchr( m_pParser->GetArgBuffer(), TEXT( '+' ) )) )
					{
						lPointSize += 2;
						lPointSize *= m_pParser->GetBaseFontSize();
						boolAbs = true;
					}
					else
					{
						//lPointSize += 2;
						//lPointSize *= m_pParser->GetBaseFontSize();
					}
				}
				else
				{ // do nothing if negetive, this should result in a smaller font
				}	
				
				lPointSize = GetHtmlView()->GetSettings()->GetPixelHt() *
								(int)lPointSize / 72;
				if ( !boolAbs )
				{
					lPointSize += m_pParser->GetTextStyle()->GetPointSize();
				}

			   	m_pParser->GetTextStyle()->SetPointSizeExtra( lPointSize );
				break;
			}
			case ARG_COLOR :
			case ARG_FGCOLOR :
			case ARG_TXTCOLOR :
			{
				m_pParser->GetTextStyle()->SetTextColor( BGR(pList[i].argVal) );
				break;
			}
			case ARG_BGCOLOR :
			{
				m_pParser->GetTextStyle()->SetBackColor( BGR(pList[i].argVal) );
				break;
			}
			default :
			{
				break;
			}
		}
	}

}
