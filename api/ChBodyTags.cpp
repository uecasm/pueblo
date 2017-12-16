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

	This file contains the implementation all body Tags.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChHtmWnd.h>

#include "ChHtmlView.h"
#include <ChHtmlSettings.h>
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChBodyTags.h"

#include <MemDebug.h>

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChHeadingTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChHeadingTag::ChHeadingTag( int iToken, ChHtmlParser *pParser ) : ChBodyElements(  pParser )
{

	m_iTokenID = iToken;

	m_luAttrs = attrSaveState |  attrHasArguments |
				attrTerminateLine | attrHasFontName
				| attrHasFontSize | attrAddLineAbove
				| attrAddLineBelow | attrFontProportional 
				| attrCallProcessTag;

	m_iPointSizeFactor = ((HTML_H6 - iToken) * 2) - 2;
	m_iPixelHt = GetHtmlView()->GetSettings()->GetPixelHt();

	if ( iToken < HTML_H4 )
	{
		m_luFontAttrs = fontBold;
		m_luAttrs |= attrCallStartAndEnd;
	}

	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize | restoreLineIndent | restoreLineFmt;
}

chint32	 ChHeadingTag::GetPointSize()
{
	return ( m_iPixelHt * (m_iPointSizeFactor + 
							GetHtmlView()->GetSettings()->GetProportionalFontSize())/ 72);
}	

void  ChHeadingTag::ProcessArguments( 	pChArgList pList, int iArgCount )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	if ( luStyle & ChTxtWnd::textRight || luStyle & ChTxtWnd::textCenter )
	{	// check if we have a center or right tag in effect
		bool	boolFound = false;
		for( int i = m_pParser->HTMLStack().GetTopIndex(); i > 0; i-- )
		{
			ChStackData *pStackInfo = m_pParser->HTMLStack().Peek( i );

			if ( HTML_CENTER == pStackInfo->iType )
			{
				boolFound = true;
			 	break;
			}
		}

		if ( !boolFound )
		{ // reset to left justified
			luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textCenter );
			luStyle |= ChTxtWnd::textLeft;
		}
		
	}

	if ( luStyle & ChTxtWnd::textNoWrap )
	{	// check if we have a nobr tag in effect
		bool	boolFound = false;
		for( int i = m_pParser->HTMLStack().GetTopIndex(); i > 0; i-- )
		{
			ChStackData *pStackInfo = m_pParser->HTMLStack().Peek( i );

			if ( HTML_NOBR == pStackInfo->iType )
			{
				boolFound = true;
			 	break;
			}
		}

		if ( !boolFound )
		{ // reset to wrap
			luStyle &= ~ChTxtWnd::textNoWrap;
		}
		
	}


	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_NOWRAP :
			{
				luStyle |=  ChTxtWnd::textNoWrap;
				break;
			}
			case ARG_CLEAR :
			{
				chuint32 luAttr = 0;

				if ( pList[i].argVal & clearLeft )
				{
					luAttr |= ChTxtWnd::objAttrClearLeft;
				}
				if ( pList[i].argVal & clearRight )
				{
					luAttr |= ChTxtWnd::objAttrClearRight;
				}
				if ( pList[i].argVal & clearAll )
				{
					luAttr |= ( ChTxtWnd::objAttrClearLeft |
									ChTxtWnd::objAttrClearRight );
				}
				if ( luAttr )
				{ // add a object space for clearing the margins
					ChSize size( 1, 1 );
					ChRect spaceExtra( 0, 0, 0, 0 );
					ChObjSpace *pObjSpace = new ChObjSpace( size, spaceExtra, luAttr );
					ChTxtObject	 spaceObject( pObjSpace );
					spaceObject.SetStyle( ChTxtWnd::textObject );
					GetHtmlView()->AppendObject( &spaceObject );

					m_pParser->SetLastChar( TEXT( '\r' ) ); 
				}
				break;
			}
			case ARG_ALIGN :
			{
				if ( pList[i].argVal == VAL_INDENT )
				{

					int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();

					iIndent += m_pParser->GetLeftIndent() * indentFactor;
					m_pParser->GetTextStyle()->SetLeftIndent( iIndent );
					m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| ChTxtWnd::textIndentLeft );
				}
				else
				{
					switch( pList[i].argVal )
					{
						case VAL_CENTER :
						case VAL_ABSMIDDLE :
						{
							luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textLeft );
							luStyle |= 	ChTxtWnd::textCenter;
							break;
						}
						case VAL_RIGHT :
						{
							luStyle &= ~( ChTxtWnd::textCenter | ChTxtWnd::textLeft );
							luStyle |= 	ChTxtWnd::textRight;
							break;
						}
						case VAL_LEFT :
						{
							luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textCenter );
							luStyle |= 	ChTxtWnd::textLeft;
							break;
						}
						default :
						{
							break;
						}

					}
				}
				break;
			}  
		}
	}
	m_pParser->GetTextStyle()->SetStyle( luStyle );

}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChPragraphTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChPragraphTag::ChPragraphTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{

	m_iTokenID = HTML_PARAGRAPH;

	m_luAttrs = attrSaveState |   attrHasArguments 
				| attrTerminateLine | attrStartLine
				| attrHasFontName | attrHasFontSize
				| attrAddLineAbove | attrAddLineBelow
				| attrFontProportional | attrCallProcessTag;
	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize | 	restoreLineIndent | restoreLineFmt;
}

void  ChPragraphTag::ProcessArguments( 	pChArgList pList, int iArgCount )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	if ( luStyle & ChTxtWnd::textRight || luStyle & ChTxtWnd::textCenter )
	{	// check if we have a center or right tag in effect
		bool	boolFound = false;
		for( int i = m_pParser->HTMLStack().GetTopIndex(); i > 0; i-- )
		{
			ChStackData *pStackInfo = m_pParser->HTMLStack().Peek( i );

			if ( HTML_CENTER == pStackInfo->iType )
			{
				boolFound = true;
			 	break;
			}
		}

		if ( !boolFound )
		{ // reset to left justified
			luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textCenter );
			luStyle |= ChTxtWnd::textLeft;
		}
		
	}

	if ( luStyle & ChTxtWnd::textNoWrap )
	{	// check if we have a nobr tag in effect
		bool	boolFound = false;
		for( int i = m_pParser->HTMLStack().GetTopIndex(); i > 0; i-- )
		{
			ChStackData *pStackInfo = m_pParser->HTMLStack().Peek( i );

			if ( HTML_NOBR == pStackInfo->iType )
			{
				boolFound = true;
			 	break;
			}
		}

		if ( !boolFound )
		{ // reset to wrap
			luStyle &= ~ChTxtWnd::textNoWrap;
		}
		
	}


	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_NOWRAP :
			{
				luStyle |=  ChTxtWnd::textNoWrap;
				break;
			}
			case ARG_CLEAR :
			{
				chuint32 luAttr = 0;
				if ( pList[i].argVal & clearLeft )
				{
					luAttr |= ChTxtWnd::objAttrClearLeft;
				}
				if ( pList[i].argVal & clearRight )
				{
					luAttr |= ChTxtWnd::objAttrClearRight;
				}
				if ( pList[i].argVal & clearAll )
				{
					luAttr |= ( ChTxtWnd::objAttrClearLeft |
									ChTxtWnd::objAttrClearRight );
				}
				if ( luAttr )
				{ // add a object space for clearing the margins
					ChSize size( 1, 1 );
					ChRect spaceExtra( 0, 0, 0, 0 );
					ChObjSpace *pObjSpace = new ChObjSpace( size, spaceExtra, luAttr );
					ChTxtObject	 spaceObject( pObjSpace );
					spaceObject.SetStyle( ChTxtWnd::textObject );
					GetHtmlView()->AppendObject( &spaceObject );

					m_pParser->SetLastChar( TEXT( '\r' ) ); 

				}
				break;
			}
			case ARG_ALIGN :
			{
				if ( pList[i].argVal == VAL_INDENT )
				{

					int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();

					iIndent += m_pParser->GetLeftIndent() * indentFactor;
					m_pParser->GetTextStyle()->SetLeftIndent( iIndent );
					m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| ChTxtWnd::textIndentLeft );
				}
				else
				{
					switch( pList[i].argVal )
					{
						case VAL_CENTER :
						case VAL_ABSMIDDLE :
						{
							luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textLeft );
							luStyle |= 	ChTxtWnd::textCenter;
							break;
						}
						case VAL_RIGHT :
						{
							luStyle &= ~( ChTxtWnd::textCenter | ChTxtWnd::textLeft );
							luStyle |= 	ChTxtWnd::textRight;
							break;
						}
						case VAL_LEFT :
						{
							luStyle &= ~( ChTxtWnd::textRight | ChTxtWnd::textCenter );
							luStyle |= 	ChTxtWnd::textLeft;
							break;
						}
						default :
						{
							break;
						}

					}
				}
				break;
			}  
		}
	}
	m_pParser->GetTextStyle()->SetStyle( luStyle );

}


///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChPreFormatTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChPreFormatTag::ChPreFormatTag(  ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_PREFORMAT;

	m_luAttrs = attrSaveState | attrHasArguments |
				attrTerminateLine | attrHasFontName
				| attrHasFontSize	| attrFontFixed 
				| attrCallStartAndEnd | attrAddLineAbove
				| attrCallProcessTag | attrParseWhiteSpace;


	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize |  restoreLineFmt;
}

void  ChPreFormatTag::StartTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle |=  ( ChTxtWnd::textNoWrap | ChTxtWnd::textPreFormat );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->IncrementFixed();

}
void  ChPreFormatTag::EndTag(  )
{
//	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

//	luStyle &=  ~( ChTxtWnd::textNoWrap | ChTxtWnd::textPreFormat );

//	m_pParser->GetTextStyle()->SetStyle( luStyle );
	m_pParser->DecrementFixed();

}

void  ChPreFormatTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_WIDTH :
			{
				m_pParser->HTMLStack().Peek( m_pParser->HTMLStack().GetTopIndex() )
								->iLineWidth = (int)pList[i].argVal;
				break;
			}
			default :
			{
				break;
			}
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
////////////
///////////// ChQuoteTag	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChQuoteTag::ChQuoteTag( ChHtmlParser *pParser ) : ChBodyElements(  pParser )
{
	m_iTokenID = HTML_QUOTE;
	m_luAttrs = attrSaveState |	attrTerminateLine | attrCallProcessTag
					| attrCallStartAndEnd;	

	m_luFontAttrs = fontItalic;

	m_luStateToRestore =  restoreFontAttrs;
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChBlockQuoteTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChBlockQuoteTag::ChBlockQuoteTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_BLKQUOTE;
	m_luAttrs = attrSaveState | attrStartLine |
				attrTerminateLine |	attrAddLineAbove  |  
				attrAddLineBelow  |	attrHasArguments  |	attrCallProcessTag
					| attrCallStartAndEnd;

	m_luFontAttrs = fontItalic;
	// attrs to restore when the tag terminates
	m_luStateToRestore =  restoreLineIndent | restoreFontAttrs;
}

void  ChBlockQuoteTag::ProcessArguments( 	pChArgList pList, int iArgCount )
{
	int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();

	iIndent += m_pParser->GetLeftIndent() * indentFactor;
	m_pParser->GetTextStyle()->SetLeftIndent( iIndent );
	m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| ChTxtWnd::textIndentLeft );
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChAddressTag::ChAddressTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_ADDRESS;
	m_luAttrs = attrSaveState |	attrTerminateLine | attrCallProcessTag 
					| attrCallStartAndEnd;
	m_luFontAttrs = fontItalic;
	// attrs to restore when the tag terminates
	m_luStateToRestore =  restoreFontAttrs;
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChListTags::ChListTags( int iToken, ChHtmlParser *pParser ) : ChBodyElements(  pParser )
{
	m_iTokenID = iToken;

	m_luAttrs = attrSaveState | attrHasArguments | attrTerminateLine |  
				attrStartLine |	attrAddLineAbove |  attrAddLineBelow |
				attrCallProcessTag;

	// attrs to restore when the tag terminates
	m_luStateToRestore =  restoreLineIndent;
}

void  ChListTags::ProcessArguments( 	pChArgList pList, int iArgCount )
{

	ChStackData* pdata;

	pdata = m_pParser->HTMLStack().Peek( m_pParser->HTMLStack().GetTopIndex() );

	pdata->luModifier = 0;
	pdata->iLineNumber = 1;

	//int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();

	//iIndent += GetLeftIndent() * indentFactor;
	//m_pParser->GetTextStyle()->SetLeftIndent( iIndent );
	m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| ChTxtWnd::textIndentLeft 
										| ChTxtWnd::textVCenter );


	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_TYPE :
			{
				switch ( pList[i].argVal )
				{
					case VAL_CHARLOWER :
					{
						pdata->luModifier |= typeListAlphaLower;
						break;
					}
					case VAL_CHARUPPER :
					{
						pdata->luModifier |= typeListAlphaUpper;
						break;
					}
					case VAL_ROMANSMALL :
					{
						pdata->luModifier |= typeListRomanLower;
						break;
					}
					case VAL_ROMANBIG :
					{
						pdata->luModifier |= typeListRomanUpper;
						break;
					}
					case VAL_DISC :
					case VAL_CIRCLE :
					{
						pdata->luModifier |= typeListCircleBullet;
						break;
					}
					case VAL_SQUARE :
					{
						pdata->luModifier |= typeListSquareBullet;
						break;
					}
					case VAL_NUM :
					{
						pdata->luModifier |= typeListNum;
						break;
					}
					default :
					{
						break;
					}
				}
				break;
			}
			case ARG_START :
			{
				pdata->iLineNumber = (int)pList[i].argVal;	
				break;
			}
			default :
				break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChAnchorTag::ChAnchorTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_LINK;

	m_luAttrs =  attrSaveState | attrHasArguments | attrCallProcessTag;
				
	// attrs to restore when the tag terminates
	m_luStateToRestore =  restoreTextColor | restoreLineFmt | restoreUserData;
}


void  ChAnchorTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	// allocate buffer for the argument and set the user data  
	int iArgSize = m_pParser->GetArgSize();
	if ( iArgSize )
	{

		char* pstrBuf = new char[iArgSize+1];
		ASSERT( pstrBuf );
		ChMemCopy( pstrBuf, m_pParser->GetArgBuffer(), iArgSize );
		pstrBuf[iArgSize] = 0;
		m_pParser->GetTextStyle()->SetUserData( (chparam)pstrBuf ); 
		// add all the mem allocation, we will free this on 
		//new page or when the window is destroyed
		GetHtmlView()->GetAllocList().AddTail( pstrBuf );
	} 
	else
	{
		m_pParser->GetTextStyle()->SetUserData( 0 ); 
	}

	if ( iArgCount )
	{		

		chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

		for( int i = 0; i < iArgCount; i++ )
		{
			switch ( pList[i].iArgType )
			{
				case ARG_XWORLD:
				case ARG_XCMD:
				{
					m_pParser->GetTextStyle()->SetTextColor( 
													m_pParser->GetLinkTextColor() );
					luStyle |= ChTxtWnd::textHotSpot;
					break;
				}
				case ARG_HREF:
				{
					// Set it to default color, change it if it is a HREF and
					// ahs been visited
					m_pParser->GetTextStyle()->SetTextColor( 
													m_pParser->GetLinkTextColor() );

					char *pstrLink = (char*)pList[i].argVal;
					if ( pstrLink )
					{
						ChString strLink = pstrLink;
						if ( GetHtmlView()->GetFrameMgr()->VisitedURL( strLink, GetHtmlView()->GetDocURL() ) )
						{
							m_pParser->GetTextStyle()->SetTextColor( 
											m_pParser->GetVisitedLinkColor() );
						}
						else if ( GetHtmlView()->PrefetchedLink( strLink ) )
						{  // is it prefetched 
							m_pParser->GetTextStyle()->SetTextColor( 
											m_pParser->GetPrefetchedLinkColor() );
						}
					}
	
					luStyle |= ChTxtWnd::textHotSpot;
					break;
				} 
				case ARG_NAME :
				{
					luStyle &= ~ChTxtWnd::textHotSpot;
					luStyle |= ChTxtWnd::textJumpMark;
					break;
				}
				default :
				{
					break;
				}
			}
		}
		m_pParser->GetTextStyle()->SetStyle( luStyle );
	}

}




///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChSampleTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChSampleTag::ChSampleTag( int iToken, ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = iToken;

	m_luAttrs = attrSaveState |   attrFontFixed | attrHasFontName |
				attrCallProcessTag | attrCallStartAndEnd | attrParseWhiteSpace;
	// attrs to restore when the tag terminates
	m_luStateToRestore = restoreFontSize |  restoreLineFmt;
}

void  ChSampleTag::StartTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle |=  ( ChTxtWnd::textPreFormat );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

	m_pParser->IncrementFixed();

}
void  ChSampleTag::EndTag(  )
{
	m_pParser->DecrementFixed();
}

///////////////////////////////////////////////////////////////////////////////
////////////
///////////// ChCenterTag	 implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChCenterTag::ChCenterTag(  ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_CENTER;

	m_luAttrs = attrCallStart |  attrSaveState | attrCallProcessTag |
				attrTrimRight;
	m_luStateToRestore =  restoreLineFmt;
}

void  ChCenterTag::StartTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle &= ~(ChTxtWnd::textRight | ChTxtWnd::textCenter |
							ChTxtWnd::textLeft);
	luStyle |= ChTxtWnd::textCenter;
	m_pParser->GetTextStyle()->SetStyle( luStyle );

}

void  ChCenterTag::EndTag( )
{
	//chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	//luStyle &= ~(ChTxtWnd::textRight | ChTxtWnd::textCenter |
	//						ChTxtWnd::textLeft);
	//luStyle |= ChTxtWnd::textLeft;

	//m_pParser->GetTextStyle()->SetStyle( luStyle );

}



ChNOBRTag::ChNOBRTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_NOBR;
	m_luAttrs = attrSaveState  | attrCallStartAndEnd | attrCallProcessTag | attrTrimRight;
	m_luStateToRestore = restoreLineFmt;
}

void  ChNOBRTag::StartTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle |=  ( ChTxtWnd::textNoWrap );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

}

void  ChNOBRTag::EndTag( )
{
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	luStyle &=  ~( ChTxtWnd::textNoWrap  );

	m_pParser->GetTextStyle()->SetStyle( luStyle );

}
