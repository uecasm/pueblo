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

#include <ChArgList.h>

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::GetArguments

------------------------------------------------------------------------------
This method translates all arguments we understand and fill up the argument list,
currently a max of 10 arguments can be specified.
----------------------------------------------------------------------------*/
int ChHtmlParser::GetArguments( chint32 lStart,	pChArgList pArg, int iMax)
{
	ClearUserArgs(); // empty the user args for this tag

	int iArgCount = 0;

	if ( m_pstrTagBuffer[lStart] == TEXT( '>' ))
	{
		return ( iArgCount );
	}

	// remove the tag
	while( lStart < m_iTagSize && m_pstrTagBuffer[lStart] != TEXT( '>' )
					&& !IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ) )
	{
		lStart++;
	}

	ChString strAttr;
	ChString strValue;



	// Get all the attributes
	while( true )
	{
		// remove any leading white  space
		while( lStart < m_iTagSize &&  IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ) )
		{
			++lStart;
		}
		if ( lStart >= m_iTagSize || m_pstrTagBuffer[lStart] == TEXT( '>' ) )
		{
			break;
		}

		// Get the argument
		// force padding to concat faster
		strAttr.GetBuffer( 64 );
		strAttr.ReleaseBuffer( 0 );   

		while( lStart < m_iTagSize  && m_pstrTagBuffer[lStart] != TEXT( '=' )
								 && m_pstrTagBuffer[lStart] != CHAR_SPACE
								&&  m_pstrTagBuffer[lStart] != TEXT( '>' ) )
		{
			strAttr += m_pstrTagBuffer[lStart++];
		}

		while( lStart < m_iTagSize &&  IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ) )
		{
			++lStart;
		}


		if ( m_pstrTagBuffer[lStart] == TEXT( '=' ) )
		{// we have a attribute
			// remove the equal sign
			lStart++;
			while( lStart < m_iTagSize &&  IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ))
			{
				++lStart;
			}

			// Get the value

			strValue.GetBuffer( 512 );
			strValue.ReleaseBuffer( 0 );   

			if ( CHAR_DBL_QUOTE == m_pstrTagBuffer[lStart] 
						|| CHAR_SGL_QUOTE == m_pstrTagBuffer[lStart]  )
			{
				char strTerm = m_pstrTagBuffer[lStart] ;

				lStart++;
				while( lStart < m_iTagSize  && m_pstrTagBuffer[lStart] != strTerm
										&&  m_pstrTagBuffer[lStart] != TEXT( '>' ) )
				{		    
					char strChar = m_pstrTagBuffer[lStart++];
					if ( strChar == TEXT( '&' ) )
					{
						--lStart;
						strChar = ChHtmlParser::MapEntity( m_pstrTagBuffer, lStart, m_iTagSize );
					}
					else if ( IS_WHITE_SPACE( strChar ) )
					{
						strChar = TEXT( ' ' );
						while( lStart < m_iTagSize &&  
								IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ) )
						{
							++lStart;
						}
					}
					strValue += strChar;
				}
			}
			else
			{
				while( lStart < m_iTagSize  && !IS_WHITE_SPACE( m_pstrTagBuffer[lStart] )
										&&  m_pstrTagBuffer[lStart] != TEXT( '>' ) )
				{		    
					char strChar = m_pstrTagBuffer[lStart++];
					if ( strChar == TEXT( '&' ) )
					{
						--lStart;
						strChar = ChHtmlParser::MapEntity( m_pstrTagBuffer, lStart, m_iTagSize );
					}
					strValue += strChar;
				}
			}
			lStart++;

			if ( iArgCount < iMax )
			{
				// update our style
				UpdateAttributes( iArgCount, pArg, strAttr, strValue );
			}


		}
		else
		{
			// remove any leading white  space
			while( lStart < m_iTagSize && IS_WHITE_SPACE( m_pstrTagBuffer[lStart] ))
			{
				++lStart;
			}

			if ( iArgCount < iMax )
			{
				// update our style
				strValue.GetBuffer( 512 );
				strValue.ReleaseBuffer( 0 );   

											 //if strVal is empty
				UpdateAttributes( iArgCount, pArg, strAttr, strValue );
			}


		}

	}

	return iArgCount;

}

/*----------------------------------------------------------------------------

	FUNCTION	||	iArgCount::UpdateAttributes

------------------------------------------------------------------------------
This method translates the string value of the argument to a known quantity.
----------------------------------------------------------------------------*/
void ChHtmlParser::UpdateAttributes(  int& iIndex, pChArgList pList,
			ChString& strAttr, ChString& strVal )
{
	if ( strAttr.IsEmpty() )
	{
		return;
	}


	UpdateUserArgs( strAttr, strVal );

	int iType;

	switch( (iType = ChHtmlParser::GetArgType( strAttr )) )
	{
		case ARG_ALIGN :
		{
			pList[iIndex].iArgType = ARG_ALIGN;

			strVal.MakeLower();

			int iVal = ChHtmlParser::MapAttributeValue( strVal );

			if ( iVal != - 1)
			{
				pList[iIndex].argVal = iVal;
			}
			else
			{
				pList[iIndex].argVal = 0;
			}
			iIndex++;
			break;
		}
		case ARG_HEIGHT :
		case ARG_SIZE :
		case ARG_WIDTH :
		case ARG_MAXLENGTH :
		case ARG_ROWS :
		case ARG_COLS :
		case ARG_START :
		case ARG_VSPACE :
		case ARG_HSPACE :
		case ARG_BORDER :
		case ARG_MINWIDTH :
		case ARG_MINHEIGHT :
		{
			pList[iIndex].iArgType = iType;
			pList[iIndex].argVal = atol( strVal );
			iIndex++;
			break;
		}
		case ARG_TYPE :
		{
			pList[iIndex].iArgType = iType;
			strVal.MakeLower();

			int iVal = ChHtmlParser::MapAttributeValue( strVal );

			if ( iVal != -1 )
			{
				pList[iIndex].argVal = iVal;
				iIndex++;
			}
			break;
		}
		case ARG_CLEAR :
		{
			pList[iIndex].iArgType = iType;
			pList[iIndex].argVal = 0;

			strVal.MakeLower();
			strVal.TrimLeft();
			strVal.TrimRight();

			int iSpace;
			ChString strTmp;
			
			do 
			{ 
				iSpace = strVal.Find( TEXT( ' ' ) );

				if ( iSpace == - 1 )
				{
					strTmp = strVal;
				}
				else
				{
					strTmp = strVal.Left( iSpace );
					strVal = strVal.Right( strVal.GetLength() - iSpace - 1 );
					strVal.TrimLeft();
				}

				switch ( ChHtmlParser::MapAttributeValue( strTmp ) )
				{
					case VAL_LEFT :
					{
						pList[iIndex].argVal |= clearLeft;
						break;
					}
					case VAL_RIGHT :
					{
						pList[iIndex].argVal |= clearRight;
						break;
					}
					case VAL_CLEAR_ALL :
					{
						pList[iIndex].argVal |= clearAll;
						break;
					}
					case VAL_CLEAR_LINK :
					{
						pList[iIndex].argVal |= clearLinks;
						break;
					}
					case VAL_TEXT :
					{
						pList[iIndex].argVal |= clearText;
						break;
					}
					case VAL_PLUGINS :
					{
						pList[iIndex].argVal |= clearPlugins;
						break;
					} 
					case VAL_FORMS :
					{
						pList[iIndex].argVal |= clearForms;
						break;
					} 
					case VAL_IMAGES :
					{
						pList[iIndex].argVal |= clearImages;
						break;
					} 
					default :
					{
						break;
					}
				}
			}
			while ( iSpace != -1 );
			iIndex++;
			break;
		}

		case ARG_HREF:
		case ARG_XCMD:
		case ARG_XWORLD:
		case ARG_ACTION:
		case ARG_VALUE:
		case ARG_SRC:
		case ARG_NAME:
		case ARG_BACKGROUND :
		case ARG_OPTIONS :
		case ARG_TARGET :
		case ARG_ALIGNTO :
		case ARG_PANETITLE :
		case ARG_SCROLLING :
		case ARG_MD5 :
		{

			char *pstrCmd = new char[ strVal.GetLength() + 1];
			lstrcpy( pstrCmd, strVal );
			pList[iIndex].iArgType = iType;
			pList[iIndex].argVal = (chparam)pstrCmd;
			iIndex++;
											/* Add all the mem allocation, we
												will free this on new page or
												when the window is destroyed */
			GetHtmlView()->GetAllocList().AddTail( pstrCmd );
			break;
		}

		case ARG_FGCOLOR :
		case ARG_COLOR :
		case ARG_TXTCOLOR :
		case ARG_BGCOLOR :
		case ARG_LINK :
		case ARG_VLINK :
		{
			char *pstrEnd = 0;
			const char* pstrVal = strVal;

			pList[iIndex].iArgType = iType;

			if ( *pstrVal == TEXT( '#' ) )
			{
				++pstrVal;
				pList[iIndex].argVal = strtol( pstrVal, &pstrEnd, 16 );


			}
			else
			{
				strVal.MakeLower();
				int iColor = ChHtmlParser::MapAttributeValue( strVal );
				if ( iColor != - 1 )
				{
					pList[iIndex].argVal = iColor;
				}
				else
				{
					pList[iIndex].argVal = strtol( pstrVal, &pstrEnd, 16 );
				}
			}
			// if there was no conversion then ignore the value
			if ( !(0 == pList[iIndex].argVal && pstrVal == pstrEnd ))
			{
				iIndex++;
			}
			break;
		}
		case ARG_METHOD :
		{
			pList[iIndex].iArgType = iType;

			strVal.MakeLower();

 			pList[iIndex].argVal = ChHtmlParser::MapAttributeValue( strVal );

			iIndex++;
			break;
		}
		case ARG_ENCTYPE :
		{	// ignore for now
			break;
		}
		case ARG_XPROBABLITY :
		{
			
			char * pstrEnd;
			const char* pstrVal = strVal;

			pList[iIndex].iArgType = iType;
			pList[iIndex].argVal = strtol( pstrVal, &pstrEnd, 10 );

			if ( 0 == pList[iIndex].argVal && *pstrEnd == TEXT( '.' ) )
			{
				pstrVal = pstrEnd + 1;
				pList[iIndex].argVal = strtol( pstrVal, &pstrEnd, 10 );
			}
			else
			{
				pList[iIndex].argVal = 10;
			}
			iIndex++;
			break;
		}
		case ARG_DISABLED :
		case ARG_SELECTED :
		case ARG_CHECKED :
		case ARG_MULTIPLE :
		case ARG_ISMAP :
		case ARG_NOSHADE :
		case ARG_XGRAPH :
		{
			pList[iIndex].iArgType = iType;
			pList[iIndex].argVal = 0;
			iIndex++;
			break;
		}
		default :
		{
		//	UpdateUserArgs( strAttr, strVal );
			break;
		}
	}

}

void ChHtmlParser::ClearUserArgs()
{
	if ( m_pUserArgs )
	{
		m_pUserArgs->Empty();	
	}
	
}

void ChHtmlParser::UpdateUserArgs( const ChString& strName, const ChString& strValue )
{
	if ( m_pUserArgs == 0 )
	{
	 	m_pUserArgs = new ChArgumentList;
		ASSERT( m_pUserArgs  );
	}
	m_pUserArgs->AddArg( strName, strValue ); // empty the user args for this tag
}

// $Log$
