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

#include "headers.h"

#include <ChUrlMap.h>
#include <ChHtmlSettings.h>
#include <ChArgList.h>
#include <ChHtmWnd.h>
#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChInlineTags.h"


#include <MemDebug.h>



///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChImgTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChImgTag::ChImgTag(  ChHtmlParser *pParser ) :	ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_IMAGE;
	m_luAttrs =  attrHasArguments | attrCallProcessTag | attrParseWhiteSpace;
	m_iBufSize = 512;
	m_pstrArgBuffer = new char[ m_iBufSize ];
	ASSERT( m_pstrArgBuffer );
}

ChImgTag::~ChImgTag( )
{
	delete [] m_pstrArgBuffer;
	m_iBufSize = 0;
}

void ChImgTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	bool boolProcessed = false;	
	
	int iArgSize = m_pParser->GetArgSize();
									// Allocate buffer for the argument
	if (iArgSize)
	{

		if ( m_iBufSize < (iArgSize + 1) )
		{
			delete [] m_pstrArgBuffer;
			m_iBufSize = iArgSize + 1;	
			m_pstrArgBuffer = new char[ m_iBufSize ];
		}
	
		ASSERT( m_pstrArgBuffer );

		ChMemCopy( m_pstrArgBuffer, m_pParser->GetArgBuffer(), iArgSize );
		m_pstrArgBuffer[iArgSize] = 0;

		boolProcessed = m_pParser->NotifyInline( m_pstrArgBuffer );
		if ( boolProcessed )
		{  // check if we have mode = html
			ChString strValue;
			if ( ChHtmlWnd::GetHTMLAttribute( m_pstrArgBuffer, "xch_mode", strValue ) &&
						0 == strValue.CompareNoCase( "html" ) )
			{
				// reset the stack
				m_pParser->ResetStack( HTML_PARAGRAPH );
			}
		}
	} 

	if (!boolProcessed)
	{
		int			iBorder = 0;
		ChSize		size( 8, 8 );
		chuint32	luAttr = ChTxtWnd::objAttrMiddle; 
		chuint32	luStyle = ChTxtWnd::textObject; 
		ChRect		spaceExtra( 0, 0, 0 , 0 );

		if ( m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textHotSpot )
		{	// Draw a pixel wide border to show this is a hotspot
			iBorder = 2;	
		}

		int iHREF = -1;
		for (int i = 0; i < iArgCount; i++)
		{
			switch (pList[i].iArgType)
			{
				case ARG_SRC :
				case ARG_HREF :
				{
					iHREF = i;
					break;
				}
				case ARG_BORDER :
				{
					iBorder = (int)pList[i].argVal;
					break;
				}
				case ARG_WIDTH:
				{
					size.cx = (int)pList[i].argVal;
					break;
				}

				case ARG_HEIGHT:
				{
					size.cy = (int)pList[i].argVal;
					break;
				}

				case ARG_VSPACE:
				{
					spaceExtra.top = spaceExtra.bottom =  
									(int)pList[i].argVal;
					break;
				}

				case ARG_HSPACE:
				{
					spaceExtra.left = spaceExtra.right =  
									(int)pList[i].argVal;
					break;
				}

				case ARG_ALIGN:
				{
					if ( VAL_LEFT == pList[i].argVal)
					{
						luAttr = ChTxtWnd::objAttrLeft;
					}
					else if (VAL_RIGHT == pList[i].argVal)
					{
						luAttr = ChTxtWnd::objAttrRight;
					}
					else if ( VAL_CENTER == pList[i].argVal || VAL_ABSMIDDLE == pList[i].argVal)
					{
						luAttr = ChTxtWnd::objAttrMiddle;
					}
					break;
				}
				case ARG_ISMAP:
				{
					luStyle |= ChTxtWnd::textISMAP;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		if ( iHREF != -1 )
		{

			ChURLParts  urlParts;
			ChString		strURL( (const char*)pList[iHREF].argVal );

			if ( urlParts.GetURLParts( strURL, GetHtmlView()->GetDocBaseURL() ) )
			{
				// Find if this is the first time we are getting this image
				ChInlineImageData **ppData = GetHtmlView()->GetImageList().Find( urlParts.GetURL() );
				ChInlineImageData *pImage;

				if ( ppData && *ppData )
				{
					pImage = *ppData;	
				}
				else
				{
					pImage = new ChInlineImageData;
					ASSERT( pImage );

					// Insert it to our list
					GetHtmlView()->GetImageList().Insert( urlParts.GetURL(), pImage );
				}

				ChObjInline*		pObjImg = new ChObjInline( size, spaceExtra, luAttr, 
															iBorder, 
															m_pParser->GetTextStyle()->
															GetTextColor(), pImage );
				ChTxtObject		ImgObject( pObjImg );

				ASSERT( 0 != pObjImg );

				ImgObject.SetStyle( luStyle |  
									m_pParser->GetTextStyle()->GetStyle() );
				ImgObject.SetLeftIndent( m_pParser->GetTextStyle()->GetLeftIndent() );
				ImgObject.SetUserData( m_pParser->GetTextStyle()->GetUserData() );
				ImgObject.SetTextColor( m_pParser->GetTextStyle()->GetTextColor() );

				GetHtmlView()->AppendObject( &ImgObject );	

				if ( luAttr & ChTxtWnd::objAttrMiddle )
				{
					m_pParser->SetLastChar( TEXT( 'C' ) ); // assume control as some printable char
				}


				// Load it only if it is the first request
				if ( ppData == 0 )
				{
					m_pParser->LoadInline( urlParts.GetURL(), pObjImg );
				}
			}
		}
	}
} 

chint32 ChImgTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	bool boolAddSpace = false;	
	// remove all white space
	if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ) )
		{
			++lStart;
			boolAddSpace = true;
		}
	}

	if ( boolAddSpace )
	{
		// add space only if it is a non floating object

		int  		idObjType;
		chuint32 	luAttr;
		ChSize		objSize;
		ChRect 		objSpaceExtra;

		m_pParser->GetObjectAttrs( idObjType, luAttr, objSize, objSpaceExtra );

		if ( ChTxtWnd::objAttrMiddle == luAttr )
		{

			// add a space after the embedded object
			m_pParser->AppendChar( CHAR_SPACE  );
			// make sure we dont'have hotspot and userdata
			chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();
			chparam  userData = m_pParser->GetTextStyle()->GetUserData();
			m_pParser->GetTextStyle()->SetStyle( ( luStyle & ~(ChTxtWnd::textHotSpot | ChTxtWnd::textJumpMark )) );
			m_pParser->GetTextStyle()->SetUserData( 0 );

			m_pParser->CommitBuffer();
			//restore style
			m_pParser->GetTextStyle()->SetStyle( luStyle );
			m_pParser->GetTextStyle()->SetUserData( userData );
		}
	}

	return lStart;
}



///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChEmbedTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChEmbedTag::ChEmbedTag(  ChHtmlParser *pParser ) :	ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_EMBED;
	m_luAttrs =  attrHasArguments | attrCallProcessTag | attrParseWhiteSpace;
}

void ChEmbedTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	ChSize		size( 8, 8 );
	chuint32	luAttr = ChTxtWnd::objAttrMiddle; 
	chuint32	luStyle = ChTxtWnd::textObject; 
	ChRect		spaceExtra( 0, 0, 0 , 0 );
	int			i;

	for (i = 0; i < iArgCount; i++)
	{
		switch (pList[i].iArgType)
		{
			case ARG_WIDTH:
			{
				size.cx = (int)pList[i].argVal;
				break;
			}

			case ARG_HEIGHT:
			{
				size.cy = (int)pList[i].argVal;
				break;
			}

			case ARG_VSPACE:
			{
				spaceExtra.top = spaceExtra.bottom =  
								(int)pList[i].argVal;
				break;
			}

			case ARG_HSPACE:
			{
				spaceExtra.left = spaceExtra.right =  
								(int)pList[i].argVal;
				break;
			}

			case ARG_ALIGN:
			{
				if (VAL_LEFT == pList[i].argVal)
				{
					luAttr = ChTxtWnd::objAttrLeft;
				}
				else if (VAL_RIGHT == pList[i].argVal)
				{
					luAttr = ChTxtWnd::objAttrRight;
				}
				else if (VAL_CENTER == pList[i].argVal || VAL_ABSMIDDLE == pList[i].argVal )
				{
					luAttr = ChTxtWnd::objAttrMiddle;
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}

	for (i = 0; i < iArgCount; i++)
	{
		if (ARG_SRC == pList[i].iArgType
				|| ARG_HREF == pList[i].iArgType )
		{

			ChURLParts  urlParts;
			ChString		strURL( (const char*)pList[i].argVal );

			if ( urlParts.GetURLParts( strURL, GetHtmlView()->GetDocBaseURL() ) )
			{
				ChInlinePluginData *pPlugin;

				ChArgumentList* pArgs = 0;

				if ( m_pParser->GetUserArgs() )
				{
					pArgs = new ChArgumentList( *(m_pParser->GetUserArgs()) );
					ASSERT( pArgs );
				}

				pPlugin = new ChInlinePluginData( pArgs );
				ASSERT( pPlugin );

				ChObjInline*		pObjInline = new ChObjInline( size, spaceExtra, luAttr, 
															0, 
															m_pParser->GetTextStyle()->
															GetTextColor(), pPlugin );
				ChTxtObject		ImgObject( pObjInline );

				ASSERT( 0 != pObjInline );

				ImgObject.SetStyle( luStyle |  
									m_pParser->GetTextStyle()->GetStyle() );
				ImgObject.SetLeftIndent( m_pParser->GetTextStyle()->GetLeftIndent() );
				ImgObject.SetUserData( m_pParser->GetTextStyle()->GetUserData() );
				ImgObject.SetTextColor( m_pParser->GetTextStyle()->GetTextColor() );

				GetHtmlView()->AppendObject( &ImgObject );

				if ( luAttr & ChTxtWnd::objAttrMiddle )
				{
					m_pParser->SetLastChar( TEXT( 'C' ) ); // Treat it as a character
				}
		
				// Always request for embeds
				m_pParser->LoadInline( urlParts.GetURL(),	pObjInline );
			}
			break;
		}
	}
} 

chint32 ChEmbedTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	
	bool boolAddSpace = false;	
	// remove all white space
	if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ) )
		{
			++lStart;
			boolAddSpace = true;
		}
	}

	if ( boolAddSpace )
	{
		// add space only if it is a non floating object

		int  		idObjType;
		chuint32 	luAttr;
		ChSize		objSize;
		ChRect 		objSpaceExtra;

		m_pParser->GetObjectAttrs( idObjType, luAttr, objSize, objSpaceExtra );

		if ( ChTxtWnd::objAttrMiddle == luAttr )
		{

			// add a space after the embedded object
			m_pParser->AppendChar( CHAR_SPACE  );
			// make sure we dont'have hotspot and userdata
			chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();
			chparam  userData = m_pParser->GetTextStyle()->GetUserData();
			m_pParser->GetTextStyle()->SetStyle( ( luStyle & ~(ChTxtWnd::textHotSpot | ChTxtWnd::textJumpMark )) );
			m_pParser->GetTextStyle()->SetUserData( 0 );

			m_pParser->CommitBuffer();
			//restore style
			m_pParser->GetTextStyle()->SetStyle( luStyle );
			m_pParser->GetTextStyle()->SetUserData( userData );
		}
	}

	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChHRTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChHRTag::ChHRTag(  ChHtmlParser *pParser ) : 
						ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_HORZRULE;
	m_luAttrs = attrHasArguments | attrSaveState
				| attrStartLine | attrCallProcessTag;
	m_luStateToRestore =  restoreLineFmt;
}


void ChHRTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	ChStackData* pdata;

	pdata = m_pParser->HTMLStack().Peek( m_pParser->HTMLStack().GetTopIndex() );

	pdata->luModifier = 0;
	pdata->iPointSize = (GetHtmlView()->GetSettings()->GetPixelHt() * 8 / 72);
	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_HEIGHT :
			case ARG_SIZE :
			{
				m_pParser->GetTextStyle()->SetLineHeight( (int)pList[i].argVal );
				pdata->iPointSize =(int) (GetHtmlView()->GetSettings()->GetPixelHt() 
							* ( 4 * pList[i].argVal) / 72);
				break;
			}
			case ARG_WIDTH :
			{
				m_pParser->GetTextStyle()->SetLineWidth( (int)pList[i].argVal );
				break;
			}
			case ARG_ALIGN :
			{
				if ( pList[i].argVal == VAL_INDENT )
				{

					int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();

					iIndent += m_pParser->GetLeftIndent() * 3;
					m_pParser->GetTextStyle()->SetLeftIndent(iIndent);
					m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| ChTxtWnd::textIndentLeft );
				}
				else
				{
					chuint32 luAlign = 0;
					switch( pList[i].argVal )
					{
						case VAL_CENTER :
						case VAL_ABSMIDDLE :
						{
							luAlign = 	ChTxtWnd::textCenter;
							break;
						}
						case VAL_RIGHT :
						{
							luAlign = 	ChTxtWnd::textRight;
							break;
						}
						case VAL_LEFT :
						{
							luAlign = 	ChTxtWnd::textLeft;
							break;
						}
						default :
						{
							break;
						}

					}

					m_pParser->GetTextStyle()->SetStyle( m_pParser->GetTextStyle()->GetStyle()  
										| luAlign );
				}
				break;
			}
			case ARG_NOSHADE :
			{
				pdata->luModifier = ChTxtWnd::objAttrNoShade;
				break;
			}  
			default :
			{
				break;
			}
		}
	}
}
chint32 ChHRTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	if ( m_pParser->GetTextStyle()->GetLineWidth() <= 0 )
	{
		
		m_pParser->GetTextStyle()->SetLineWidth( 100 );
	}
	if ( m_pParser->GetTextStyle()->GetLineHeight() <= 0 )
	{
		m_pParser->GetTextStyle()->SetLineHeight( 2 );
	}

	ChSize size;
	size.cx =  m_pParser->GetTextStyle()->GetLineWidth();
	size.cy = m_pParser->GetTextStyle()->GetLineHeight();

	chint32  lHeight = (m_pParser->GetTextStyle()->GetPointSize() * 
								GetHtmlView()->GetSettings()->GetPixelHt()) / 72;
	lHeight =  (lHeight * 30)/100; // 30 %
	if ( lHeight < 0 )
	{
		lHeight *= -1;
	}
	
	int 		iSpaceTop = 0; 
	int 		iSpaceExtra = 0;
	int  		idObjType;
	chuint32 	luAttr;
	ChSize		objSize;
	ChRect 		objSpaceExtra;

	m_pParser->GetObjectAttrs( idObjType, luAttr, objSize, objSpaceExtra );

	if ( idObjType )
	{
		if ( !(luAttr & ChTxtWnd::objAttrBreak) )
		{
			iSpaceTop = (int)lHeight;
		}
		else
		{
			if ( idObjType == ChTextObject::objectSpace )
			{
				iSpaceExtra = objSpaceExtra.top + objSpaceExtra.bottom + objSize.cy;
			}
			else
			{
				iSpaceExtra = objSpaceExtra.bottom;
			}
		}
	}
	else
	{
		iSpaceTop = (int)lHeight;
	}


	// Get the attributes of the last added HR
	if( m_pParser->GetLastObjectAttrs( ChTextObject::objectLine, luAttr,
								 			objSize, objSpaceExtra ) )
	{
		if ( 0 == idObjType )
		{
			iSpaceTop = objSpaceExtra.bottom;
		}
		else
		{
			if ( objSpaceExtra.bottom >= iSpaceExtra )
			{
				iSpaceTop = objSpaceExtra.bottom - iSpaceExtra;
			} 
			else
			{
				iSpaceTop = (iSpaceExtra - objSpaceExtra.bottom);
			}
		}
	}

	ChStackData* pdataTerm;
	pdataTerm = m_pParser->HTMLStack().Peek( m_pParser->HTMLStack().GetTopIndex() );

	ChRect spaceExtra( 0, iSpaceTop, 0, (int)lHeight );

	ChObjLine *pobjLine = new ChObjLine( size,  spaceExtra, 
							pdataTerm->luModifier | ChTxtWnd::objAttrBreak );

	ChTxtObject	 lineObject( pobjLine );

	lineObject.SetStyle( ChTxtWnd::textObject | 
							m_pParser->GetTextStyle()->GetStyle() );



	GetHtmlView()->AppendObject( &lineObject );

	m_pParser->SetLastChar( TEXT( '\r' ) ); 


	m_pParser->RestoreTextStyle( HTML_HORZRULE );
	// restore style
	ChStackData* pdataTop = m_pParser->HTMLStack().
						Peek( m_pParser->HTMLStack().GetTopIndex() );

	ChHtmlTag *pCurrStyle = m_pParser->GetHTMLTagInstance( pdataTop->iType );

	m_pParser->CreateStyle( pCurrStyle ); 

	return lStart;			

}

///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChBRTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChBRTag::ChBRTag(  ChHtmlParser *pParser ) : ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_LINEBREAK; 
	m_luAttrs = attrHasArguments | attrCallProcessTag | attrTrimRight;
}


void ChBRTag::ProcessArguments( pChArgList pList, int iArgCount ) 
{
	chuint32 luAttr = 0;
	for( int i = 0; i < iArgCount; i++ )
	{
		if ( pList[i].iArgType == ARG_CLEAR)
		{
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

		}
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
	else
	{
		m_pParser->AppendChar( TEXT( '\r' )  );
	}
}

chint32 ChBRTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{  

	// skip till the next tag
	if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ))
		{
			++lStart;
		}
	}
	return lStart;			

}

///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChLineItemTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////
ChLineItemTag::ChLineItemTag(  ChHtmlParser *pParser ) : ChBodyElements(  pParser )
{
	m_iTokenID = HTML_LINEITEM; 
	m_luAttrs |= attrStartLine | attrCallProcessTag;
}


chint32 ChLineItemTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
 	// Find the indent and set it
	m_pParser->GetTextStyle()->SetLeftIndent( 
					m_pParser->GetCurrentIndent() + m_pParser->GetLeftIndent() * indentFactor );


	if ( lStart >= lCount )
	{
		return lStart;
	}

	// This text should be rendered with the same style as the previous run, 
	// at the end of the style add a new line	  

	int 	iIndex = m_pParser->HTMLStack().GetTopIndex(); 
	ChStackData* pdataTerm = 0;

	while( iIndex > 0 )
	{ 
		pdataTerm = m_pParser->HTMLStack().Peek( iIndex );

		if ( HTML_NUMLIST == pdataTerm->iType || HTML_BULLETLINE == pdataTerm->iType )
		{
			break;
		}
		--iIndex;
	}

	if ( pdataTerm && pdataTerm->iType == HTML_NUMLIST )
	{  // Number the list

		
		char	  strNum[30];

		if ( pdataTerm->luModifier & typeListAlphaLower  )
		{
			wsprintf( strNum, "%c.   ", (char)( 'a' + pdataTerm->iLineNumber++ - 1));
		}
		else if ( pdataTerm->luModifier & typeListAlphaUpper )
		{
			wsprintf( strNum, "%c.   ", (char)( 'A' + pdataTerm->iLineNumber++ - 1 ) );
		}
		else if ( pdataTerm->luModifier & typeListNum )
		{
			wsprintf( strNum, "%d.   ", pdataTerm->iLineNumber++ );
		}
		else
		{  // need to process roman numerals here
			wsprintf( strNum, "%d.   ", pdataTerm->iLineNumber++ );
		}

		for ( int i = 0; i < lstrlen( strNum ); i++ )
		{
			m_pParser->AppendChar( strNum[i]  );
		}

		// get the with of the text
		int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();	
		// we need the width of the decoration for indents
		ChSize size;
		{
			ChClientDC dc( GetHtmlView() );

			ChFont* pOld = dc.SelectObject( m_pParser->GetTextStyle()->GetFont() ); 

			size = dc.GetTextExtent( m_pParser->GetBuffer(), m_pParser->GetBufferIndex() );
			dc.SelectObject( pOld );
		}

		iIndent -= size.cx;

		m_pParser->GetTextStyle()->SetLeftIndent( iIndent );


		m_pParser->CommitBuffer( );
		// restore it back
		m_pParser->GetTextStyle()->SetLeftIndent( iIndent + size.cx );

	
	}
	else if ( pdataTerm && pdataTerm->iType == HTML_BULLETLINE )
	{
		// append the bullet and adjust the indent
		if ( pdataTerm->luModifier & typeListSquareBullet )
		{
			m_pParser->AppendChar( m_pParser->GetSpecialCharacter( CHAR_SQUARE )  );
		}
		else
		{
			m_pParser->AppendChar( m_pParser->GetSpecialCharacter( CHAR_BULLET )  );
		}
		m_pParser->AppendChar( TEXT(' ' )  );

		int iIndent = m_pParser->GetTextStyle()->GetLeftIndent();	

		ChFont*	 pOldFont   = 	m_pParser->GetTextStyle()->GetFont();
		m_pParser->GetTextStyle()->SetFont( m_pParser->GetSymbolFont( pOldFont ) );

		ChSize size;
		{
			ChClientDC dc( GetHtmlView() );

			ChFont* pOld = dc.SelectObject( m_pParser->GetTextStyle()->GetFont() ); 

			size = dc.GetTextExtent( m_pParser->GetBuffer(), m_pParser->GetBufferIndex() );
			dc.SelectObject( pOld );
		}


	
		m_pParser->GetTextStyle()->SetLeftIndent( iIndent - size.cx );

		// append the text to view
		m_pParser->CommitBuffer();

		// restore the font and indent

		m_pParser->GetTextStyle()->SetLeftIndent( iIndent );

		m_pParser->GetTextStyle()->SetFont( pOldFont );


	}

	m_pParser->LineBreak( false );

	return ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );

	//#if 0

	//// get the text into our buffer
	//while ( lStart < lCount )
	//{
	//	if ( !IS_NEW_TAG( pstrBuffer[lStart] ) )
	//	{
	//		char strChar = pstrBuffer[lStart++];
	//		if ( !(strChar== TEXT( '\r' ) || strChar == TEXT( '\n' )) )
	//		{ 

	//			if ( strChar == TEXT( '&' ) )
	//			{
	//				if ( m_pParser->MapEntityToChar( pstrBuffer, 
	//												--lStart, lCount, strChar ) )
	//				{
	//					m_pParser->AppendChar( strChar  );
	//				}
	//			}
	//			else
	//			{
	//				m_pParser->AppendChar( strChar  );
	//			}

	//		}
	//		else if ( ( strChar == TEXT( '\r' ) ||  strChar == TEXT( '\n' ) )
	//								&& m_pParser->GetLastChar() != TEXT( ' ' )  )
	//		{
	//			m_pParser->AppendChar( TEXT( ' ' ) );
	//			// skip till the next non-white space
	//			if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	//			{
	//				while ( lStart < lCount && IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	//				{
	//					lStart++;
	//				}
	//			}
	//		}
	//	}
	//	else
	//	{
	//		if ( m_pParser->BreakTag( pstrBuffer, lStart, lCount ) )
	//		{ 
	//			break;
	//		}
	//	}
	//}


	//return lStart;			
	//#endif

}
