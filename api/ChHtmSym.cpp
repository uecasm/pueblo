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

	This file consists of the implementation of the ChHtmWnd  class.
	Formas implementation

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ctype.h>

#ifdef CH_UNIX
#include <stdlib.h>
#include <ctype.h>
#include <ChTypes.h>
#include <ChRect.h>
#include <ChSize.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#endif

#include "ChHtmlView.h"

#include "ChHtmSym.h"
#include "ChHtmlParser.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/ 



CH_GLOBAL_VAR  SymbolMap aHtmlTokens[] =
{
	{ "!unknown", HTML_UNKNOWN 	},	// style for unknown tags

	{ "h1",	HTML_H1	},				// heading 1
	{ "h2",	HTML_H2 },		// heading 2
	{ "h3",	HTML_H3	},				// heading 3
	{ "h4", HTML_H4	},		// heading 4
	{ "h5",	HTML_H5	},				// heading 5
	{ "h6",	HTML_H6	},		// heading 6

	{ "blockquote",	HTML_BLKQUOTE	}, // blockquote
	{ "address", HTML_ADDRESS			},	// address
	{ "p",	HTML_PARAGRAPH			},	// paragraph
	{ "!",	HTML_COMMENT			}, // comments},	
	{ "pre", HTML_PREFORMAT				},	// preformat text
	
	{ "ul",	HTML_BULLETLINE			},	// bulleted list
	{ "ol",	HTML_NUMLIST			},	// numbered list
	{ "dl",	HTML_DEFNLIST			},	// Definition list
	{ "dir", HTML_DIR				},	// directory list of items

	{ "menu",	HTML_MENU			},	// menu : we render it as dir
	
	{ "quote",	HTML_QUOTE		},	// quoted text

	{ "strike",	HTML_STRIKETHROUGH		},	// strikethrough
 	{ "b",	HTML_BOLD			},	// bold
	{ "i", 	HTML_ITALIC			},	// italic
	{ "u",	HTML_UNDERLINE			},	// underline

	{ "tt",	HTML_TYPEWRITER			},	// typewriter font

	{ "em",	HTML_EMPHASIS			},	// emphasis
	{ "cite", HTML_CITE				},	// citation
	{ "strong",	HTML_STRONG		},	// stronger emphasis

	{ "code",   HTML_CODE		},	// HTML directive
	{ "samp",	HTML_SAMP			},	// sample output

	{ "hr",	HTML_HORZRULE			},	// embedded line

	{ "br",	HTML_LINEBREAK			},	// Line break 

	{ "li",	HTML_LINEITEM			},	// non-wraped line

	{ "a",	HTML_LINK			},	// Link

	{ "dd",	HTML_DEFNTEXT			},	// definition text
	{ "dt",	HTML_DEFNTERM			},	// definiton term

	{ "center",	HTML_CENTER		},	// center text

	{ "html",	HTML_HTML			},	// html
	{ "head",	HTML_HEAD			},// head
 	{ "body",HTML_BODY				},	// body
 	{ "title", HTML_TITLE 			},	// title
	{ "xch_page",	HTML_PAGE		},	// reformat page
	{ "xch_prefetch", HTML_PREFETCH	},	// prefetch tag
 	{ "img",	HTML_IMAGE			},	// Image

	{ "basefont",	HTML_BASEFONT		},	// basefont : Netscape additions
	{ "font",	HTML_FONT			},	// font

	{ "form",	HTML_FORM			},	// form
	{ "input",	HTML_INPUT		},	// input control
	{ "option",	HTML_OPTION		},	// options for form items
	{ "select",	HTML_SELECT		},	// multiple choice items
	{ "textarea",	HTML_TEXTAREA		},	// multiline text control	

	{ "xch_mudtext",	HTML_MUDTEXT	},	// mud text extension		

	{ "xmp",	HTML_XMP			},	// alternate to plaintext

	{ "listing",	HTML_LISTING		},	// alternate to plaintext
	{ "plaintext",	HTML_PLAINTEXT	},	// unformatted text
	{ "nobr",	HTML_NOBR	},	// unformatted text
	{ "base",	HTML_BASE	},	// unformatted text
	{ "xch_pane",	HTML_XCHPANE	},	// chaco pane tag
	{ "embed",	HTML_EMBED	},	// chaco pane tag
};



											/* Argument map table, this table
												shows all the arguments that
												are processed for HTML tags.
												Any arguments not in this list
												are ignored. */
CH_GLOBAL_VAR  SymbolMap aArguments[] =
						{
							{ "align",		ARG_ALIGN },
							{ "size",		ARG_SIZE },
							{ "width",  	ARG_WIDTH},
							{ "height", 	ARG_HEIGHT},
							{ "href",		ARG_HREF},
							{ "src",		ARG_SRC},
							{ "clear",		ARG_CLEAR},
							{ "xch_cmd",	ARG_XCMD},
							{ "type",		ARG_TYPE },
							{ "text",   	ARG_TXTCOLOR },
							{ "fgcolor", 	ARG_FGCOLOR },
							{ "bgcolor", 	ARG_BGCOLOR },
							{ "link",    	ARG_LINK },
							{ "vlink", 		ARG_VLINK },
							{ "action",  	ARG_ACTION },
							{ "method",  	ARG_METHOD },
							{ "enctype", 	ARG_ENCTYPE },
							{ "disabled", 	ARG_DISABLED },
							{ "selected", 	ARG_SELECTED },
							{ "value", 	  	ARG_VALUE },
							{ "checked", 	ARG_CHECKED },
							{ "maxlength", 	ARG_MAXLENGTH },
							{ "name", 	  	ARG_NAME },
							{ "multiple", 	ARG_MULTIPLE },
							{ "rows", 	  	ARG_ROWS },
							{ "cols", 	  	ARG_COLS },
							{ "start", 	  	ARG_START},
							{ "xch_world", 	ARG_XWORLD},
							{ "xch_prob", 	ARG_XPROBABLITY },
							{ "alink",		ARG_ALINK },
							{ "xplink",		ARG_PLINK },
							{ "hspace",		ARG_HSPACE },
							{ "vspace",		ARG_VSPACE },
							{ "ismap",		ARG_ISMAP },
							{ "background",	ARG_BACKGROUND },
							{ "border",		ARG_BORDER },
							{ "noshade",	ARG_NOSHADE },
							{ "nowrap",		ARG_NOWRAP },
							{ "target",  	ARG_TARGET	},
							{ "panetitle", 	ARG_PANETITLE	},
							{ "minwidth",  	ARG_MINWIDTH	},
							{ "minheight", 	ARG_MINHEIGHT	},
							{ "options",  	ARG_OPTIONS	},
							{ "alignto",  	ARG_ALIGNTO	},
							{ "scrolling",  ARG_SCROLLING	},
							{ "xch_graph",  ARG_XGRAPH	},
							{ "md5",		ARG_MD5 },
							{ "color",		ARG_COLOR },


						};


// Possible pre-defined values attributes

CH_GLOBAL_VAR SymbolMap aAttrValueMap[] =
{
	{ "1",  		VAL_NUM },			// default numbers	 1, 2, 3
	{ "A",			VAL_CHARUPPER },	// capital letters
	{ "a",			VAL_CHARLOWER },	// small letters
	{ "I",			VAL_ROMANBIG },		// large roman
	{ "i", 			VAL_ROMANSMALL },	// small roman
	{ "disc",		VAL_DISC },			// disc
	{ "circle",		VAL_CIRCLE },		// circle
	{ "square",		VAL_SQUARE },		// square
	{ "checkbox",	VAL_CHECKBOX },		// checkbox
	{ "hidden",		VAL_HIDDEN },		// hidden control
	{ "image",		VAL_IMAGE },		// image
	{ "radio",		VAL_RADIO },		// radio button
	{ "reset",		VAL_RESET },		// reset
	{ "submit",		VAL_SUBMIT },		// submit
	{ "password",	VAL_PASSWORD },		// password
	{ "text",		VAL_TEXT },			// edittext
	{ "left",		VAL_LEFT },			// left
	{ "right", 		VAL_RIGHT },		// right
	{ "center", 	VAL_CENTER },		// center
	{ "all",		VAL_CLEAR_ALL },	// clear all
	{ "links",		VAL_CLEAR_LINK },	// clear links
	{ "indent",		VAL_INDENT },		// indent
	{ "middle",		VAL_MIDDLE },		// middle
	{ "absmiddle",	VAL_ABSMIDDLE },	// abs middle
	{ "post",		VAL_POST },			// post method
	{ "get",		VAL_GET },	 		// get method
	{ "xch_cmd",	VAL_XCHCMD },		// xch_cmd method
	{ "forms",		VAL_FORMS },		// clear forms
	{ "plugins",	VAL_PLUGINS },		// clear plugins
	{ "top",		VAL_TOP },    		// align top
	{ "bottom",		VAL_BOTTOM },    	// align bottom
	{ "above",		VAL_ABOVE },    	// align above
	{ "below",		VAL_BELOW },    	// align below
	{ "leftof",		VAL_LEFTOF },    	// align leftof
	{ "rightof",	VAL_RIGHTOF },    	// align rightof
	{ "images",		VAL_IMAGES },    	// align rightof

	{ "black", 		0x000000 },
	{ "white", 		0xffffff },
	{ "green", 		0x00ff00 },
	{ "maroon", 	0x800000 },
	{ "olive", 		0x008000 },
	{ "navy", 		0x000080 },
	{ "purple", 	0x800080 },
	{ "gray", 		0x808080 },
	{ "red", 		0xff0000 },
	{ "yellow", 	0xffff00 },
	{ "blue", 		0x0000ff },
	{ "teal", 		0x008080 },
	{ "lime", 		0x808000 },
	{ "aqua", 		0x00ffff },
	{ "fuchsia", 	0xff00ff },
	{ "silver", 	0xc0c0c0 },

};




// Entity  map table. 
// In addition to the entitiies specified here
// all the entities of the format &[X];
// where X is any value from 1-255 are handled.

CH_GLOBAL_VAR  SymbolMap aEntityTbl[] =
						{
							{"LT", (unsigned char) '<'},
							{"GT", (unsigned char) '>'},
							{"AMP", (unsigned char)'&'},
							{"QUOT", (unsigned char)'\"'},
							{"lt", (unsigned char)'<'},
							{"gt", (unsigned char)'>'},
							{"amp", (unsigned char)'&'},
							{"quot", (unsigned char)'\"'},
							{"Aacute", 193},
							{"Agrave", 192},
							{"Acirc", 194},
							{"Atilde", 195},
							{"Aring", 197},
							{"Auml", 196},
							{"AElig", 198},
							{"Ccedil", 199},
							{"Eacute", 201},
							{"Egrave", 200},
							{"Ecirc", 202},
							{"Euml", 203},
							{"Iacute", 205},
							{"Igrave", 204},
							{"Icirc", 206},
							{"Iuml", 207},
							{"ETH", 208},
							{"Ntilde", 209},
							{"Oacute", 211},
							{"Ograve", 210},
							{"Ocirc", 212},
							{"Otilde", 213},
							{"Ouml", 214},
							{"Oslash", 216},
							{"Uacute", 218},
							{"Ugrave", 217},
							{"Ucirc", 219},
							{"Uuml", 220},
							{"Yacute", 221},
							{"THORN", 222},
							{"szlig", 223},
							{"aacute", 225},
							{"agrave", 224},
							{"acirc", 226},
							{"atilde", 227},
							{"aring", 229},
							{"auml", 228},
							{"aelig", 230},
							{"ccedil", 231},
							{"eacute", 233},
							{"egrave", 232},
							{"ecirc", 234},
							{"euml", 235},
							{"iacute", 237},
							{"igrave", 236},
							{"icirc", 238},
							{"iuml", 239},
							{"eth", 240},
							{"ntilde", 241},
							{"oacute", 243},
							{"ograve", 242},
							{"ocirc", 244},
							{"otilde", 245},
							{"ouml", 246},
							{"oslash", 248},
							{"uacute", 250},
							{"ugrave", 249},
							{"ucirc", 251},
							{"uuml", 252},
							{"yacute", 253},
							{"thorn", 254},
							{"yuml", 255},
												// Netscape additions
							{ "copy",'\xA9'},
							{ "reg", '\xAE' },
							{ "trade", '\x99' },
							{ "nbsp", '\xA0' },	// UE: was regular space
							{ "tmark", '\x99' },  // Chaco addition

						};


 // Table used to map special characters 
CH_GLOBAL_VAR CharType aSpecialChar[] =
				{
					CHAR_BULLET, 	 183,
					CHAR_SQUARE,     0xa8,
					//CHAR_REGISTER,   0xe2,
					//CHAR_COPYRIGHT,	 0xe3,
					//CHAR_TRADEMARK,	 0xe4,
				};






/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::InitHTMLParser()

------------------------------------------------------------------------------
	Get the pixel width
	Read the registry and initialize the font name and size for HTML 
	Sort all tables
----------------------------------------------------------------------------*/


void ChHtmlParser::InitStatics()
{

	static boolInitialized = false;

	if ( !boolInitialized )
	{
		boolInitialized = true;
		// Sort all tables
		SortSymbolMap( aHtmlTokens, (sizeof(aHtmlTokens)/sizeof(SymbolMap)) );

		SortSymbolMap( aEntityTbl, (sizeof(aEntityTbl)/sizeof(SymbolMap)) );
		// Sort attribute type list
		SortSymbolMap( aArguments, (sizeof(aArguments)/sizeof(SymbolMap)) );

		// Sort the attribute value table
		SortSymbolMap( aAttrValueMap, (sizeof(aAttrValueMap)/sizeof(SymbolMap)) );
	}

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::GetToken()

------------------------------------------------------------------------------
	Return the HTML token
----------------------------------------------------------------------------*/
int  ChHtmlParser::GetTokenID( const char* pstrToken )
{ 
	int iToken = FindSymbol( pstrToken, aHtmlTokens,
				 sizeof( aHtmlTokens)/sizeof( SymbolMap ) );

	return ( iToken >= 0 ? iToken : 0 );
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::LookUpTag

------------------------------------------------------------------------------
This method looks up the HTML table for a given tag treturns the HTML tag code.
if found else -1. if the tag is terminating HTML tag then boolEnd is set to true.

----------------------------------------------------------------------------*/


int ChHtmlParser::LookUpTag( const char* pstrBuffer, chint32 lStart,
							chint32 lCount, bool& boolEnd )
{
	boolEnd = false;
	int iRet = -1;

	if ( pstrBuffer[lStart] == TEXT( '<' ) )
	{
		lStart++;
	}
	if ( pstrBuffer[lStart] == TEXT( '/' ))
	{
		boolEnd = true;
		lStart++;
	}

	if ( pstrBuffer[lStart] == TEXT( '!' ))
	{
		return ( HTML_COMMENT );
	}

	ChString strToken = TEXT( "" );
	int i = 0;
	while( lStart < lCount  && !IS_WHITE_SPACE( pstrBuffer[lStart] )
							&&  pstrBuffer[lStart] != TEXT( '>' ) )
	{
		strToken += pstrBuffer[lStart++];
	}

	strToken.MakeLower();
	strToken.TrimLeft();
	strToken.TrimRight();

	return GetTokenID( strToken ); 
} 




/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::GetSpecialCharacter

------------------------------------------------------------------------------
This method maps the given character to a symbol character.
----------------------------------------------------------------------------*/

char ChHtmlParser::GetSpecialCharacter( UINT uCharType  )
{
	// get the index to the symbol font  

	char chChar;
	if ( uCharType < sizeof(aSpecialChar)/sizeof(CharType))
	{
		chChar = aSpecialChar[uCharType].chCharMap;
	}
	else
	{
		chChar	= (char)uCharType;		
	}

	return chChar;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::MapEntityToChar

------------------------------------------------------------------------------
This method maps a given entity to its charcter equivialent..

----------------------------------------------------------------------------*/


bool ChHtmlParser::MapEntityToChar(  const char* pstrBuffer, chint32& lStart, chint32 lCount, char& strChar )
{
	bool		boolTerminated = true;
	chint32		lTemp = lStart;

	ASSERT( pstrBuffer[lStart] == TEXT( '&' ) );

	lStart++;  // Skip the '&'

	ChString strEntity;

	// For faster concat
	strEntity.GetBuffer( 25 );
	strEntity.ReleaseBuffer( 0 );


	while( lStart < lCount && !IS_WHITE_SPACE( pstrBuffer[lStart] ) 
								&& pstrBuffer[lStart] != TEXT( ';' ))
	{
		strEntity += pstrBuffer[lStart++];
	}
	
	if ( pstrBuffer[lStart] == TEXT( ';' ) )
	{
		lStart++;
	}
	else
	{ 
		if (  lStart >= lCount )
		{
			// buffer '&' and return
			AddToBuffer( &pstrBuffer[lTemp], lStart - lTemp );
			return false;
		}
		boolTerminated = false;
	}

	if ( !strEntity.IsEmpty() )
	{
		int chChar;
		if ( strEntity[0]=='#') 
		{
			const char* pstrNum = strEntity;

			strChar = (char)strtol(++pstrNum, (char **)NULL, 10);
		
			return true;
		} 
		else 
		{	// Find the map in the entity table
			chChar = FindSymbol( strEntity, aEntityTbl,
					 sizeof( aEntityTbl)/sizeof( SymbolMap ) );
			if (chChar != -1 )
			{
				strChar = chChar;
				return true;
			}
			else if ( boolTerminated && strEntity.GetLength() == 1 )
			{
				strChar = strEntity[0];
				return true;
			} 
		}
	}

	// not an entity
	strChar = TEXT( '&' );
	lStart = lTemp + 1;
	return true;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::MapEntity

------------------------------------------------------------------------------
This method maps a given entity to its charcter equivialent..

----------------------------------------------------------------------------*/


char ChHtmlParser::MapEntity(  const char* pstrBuffer, chint32& lStart, chint32 lCount )
{

	ASSERT( pstrBuffer[lStart] == TEXT( '&' ) );

	bool boolTerminated = true;
	
	chint32 lTemp = lStart;	 

	lStart++;  // Skip the '&'

	ChString strEntity;

	// For faster concat
	strEntity.GetBuffer( 25 );
	strEntity.ReleaseBuffer( 0 );


	while( lStart < lCount && !IS_WHITE_SPACE( pstrBuffer[lStart] ) 
								&& pstrBuffer[lStart] != TEXT( ';' ))
	{
		strEntity += pstrBuffer[lStart++];
	}
	
	if ( pstrBuffer[lStart] == TEXT( ';' ) )
	{
		lStart++;
	}
	else
	{	// not an entity, return unmodified
		if (  lStart >= lCount )
		{
			lStart = lTemp + 1;
			return TEXT( '&' );
		}
		boolTerminated = false;
	}
	
	if ( !strEntity.IsEmpty() )
	{
		int chChar;
		if ( strEntity[0]=='#') 
		{
			const char* pstrNum = strEntity;

			chChar = strtol(++pstrNum, (char **)NULL, 10);
		
			return ((char)chChar);
		} 
		else 
		{	// Find the map in the entity table
			chChar = FindSymbol( strEntity, aEntityTbl,
					 sizeof( aEntityTbl)/sizeof( SymbolMap ) );
			if (chChar != -1 )
			{
				return ((char)chChar);
			}
			else if ( boolTerminated && strEntity.GetLength() == 1 )
			{
				return ( strEntity[0] );
			} 
		}
	}
	// restore the index
	lStart = lTemp + 1;

	return('&');
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::SortSymbolMap()

----------------------------------------------------------------------------*/


void ChHtmlParser::SortSymbolMap( pSymbolMap pmapTbl, int iTblSize )
{
	SymbolMap			tmp;
	register SymbolMap	*cp, *np, *lp;
	register bool		boolDone = 0;

	for (lp = &(pmapTbl[iTblSize - 1]); !boolDone && lp >= pmapTbl; lp--) 
	{
		boolDone = 1;
		for (cp = pmapTbl; cp < lp; cp++) 
		{
			np = cp + 1;
			if (lstrcmp(cp->pstrSymbol, np->pstrSymbol) > 0) 
			{
				tmp = *np;
				*np = *cp;
				*cp = tmp;
				boolDone = 0;
			}
		}
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlStyle::FindSymbol()

------------------------------------------------------------------------------
----------------------------------------------------------------------------*/


int ChHtmlParser::FindSymbol(const char *pstrSymbol, pSymbolMap pmapTbl,
													int iTblSize )
{
	register int	low, high, mid, r;

	low = 0;
	high = iTblSize-1;

	while ( low <= high ) 
	{
		mid = (low+high+1)/2;

		r = lstrcmp( pmapTbl[mid].pstrSymbol, pstrSymbol );
		if (r < 0) 
		{
			low = mid+1;
		} 
		else if ( r > 0 ) 
		{
			high = mid-1;
		} 
		else
		{
			return pmapTbl[mid].iMap;
		}
	}

	return -1;
}





/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::GetArgType

------------------------------------------------------------------------------
This method returns a integer id for a given argument type, it
the arg type is not processed by us then it returns 0.

----------------------------------------------------------------------------*/
int  ChHtmlParser::GetArgType(  ChString& strArg )
{
	strArg.MakeLower();

	int iType = FindSymbol( strArg, aArguments,
				 sizeof( aArguments)/sizeof( SymbolMap ) );

	return iType != -1 ? iType : 0;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::MapAttributeValue

------------------------------------------------------------------------------
This method maps the given attribute to a integer value, returns -1 if there 
is no map.
----------------------------------------------------------------------------*/

int  ChHtmlParser::MapAttributeValue( const char* pstrValue  )
{
	return 	FindSymbol( pstrValue, aAttrValueMap,
				 sizeof( aAttrValueMap)/sizeof( SymbolMap ) );
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlParser::BreakTag

------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

bool ChHtmlParser::BreakTag( const char* pstrBuffer, chint32& lStart, 	chint32  lCount  )
{

	if ( pstrBuffer[lStart] == TEXT( '<' ))
	{
		return true;
	}
	else if ( m_pHtmlView->GetAnchorTarget().GetLength() )
	{ // check if we need to start a new sub-style  for the current tag
		bool boolContinue = IS_WHITE_SPACE( GetLastChar() );

		if ( !boolContinue )
		{
			switch( GetLastChar() )
			{
				case '"' :
				case ':' :
				case '(' :
				case ')' :
				case ',' :
				case '!' :
				case ';' :
				case '{' :
				case '}' :
				case '[' :
				case ']' :
				case '`' :
				case '\'' :
				{
					boolContinue = true;
					break;
				}
				default :
					break;
			}
		}
		if ( ( ( lCount - lStart ) > 7 ) 
				&& boolContinue 
				&& !(GetTextStyle()->GetStyle() & ChTxtWnd::textHotSpot) )
		{	// we currently look for a URL 
			ChString strURL;
			chint32 lOldStart = lStart;		//UE
			while ( lStart < lCount && !IS_WHITE_SPACE( pstrBuffer[lStart] ) 
							&& !( pstrBuffer[lStart] == TEXT( '<' )
								  || pstrBuffer[lStart] == TEXT( ')' )
								  || pstrBuffer[lStart] == TEXT( '"' )
								  || pstrBuffer[lStart] == TEXT( '\'' )
								  || pstrBuffer[lStart] == TEXT( '}' )
									|| pstrBuffer[lStart] == TEXT( '>' )				//UE: disallow '>' in links
								 /*|| pstrBuffer[lStart] == TEXT( '&' )*/ ) )	//UE: allow '&' in links
			{  	
				strURL += pstrBuffer[lStart++];
			}

			// strip period and ,
			char chLast = strURL[strURL.GetLength() - 1];
			if ( chLast == '.' || chLast == ',' )
			{
				lStart--;
				strURL = strURL.Left( strURL.GetLength() - 1 );
			}

			ChString strTemp( strURL);
			strTemp.MakeLower();

			if ( strTemp.Find( "http://" ) == 0 )
			{
				// commit the old buffer
				CommitBuffer();

				// set the new style
				chuint32  luOldStyle = GetTextStyle()->GetStyle();
				chuint32  luTextColor = GetTextStyle()->GetTextColor(); 
			
				GetTextStyle()->SetTextColor(	GetLinkTextColor() );
				GetTextStyle()->SetStyle( luOldStyle | ChTxtWnd::textHotSpot );

				// create the user data for this link
				// href=url target=_webtracker
				char* pstrBuf = new char[30 + strURL.GetLength() + 
									m_pHtmlView->GetAnchorTarget().GetLength() ];
				ASSERT( pstrBuf );
				lstrcpy( pstrBuf, "href=\"" );
				lstrcat( pstrBuf, strURL );
				lstrcat( pstrBuf, "\" target=" );
				lstrcat( pstrBuf, m_pHtmlView->GetAnchorTarget() );
				GetTextStyle()->SetUserData( (chparam)pstrBuf ); 
				// add all the mem allocation, we will free this on 
				//new page or when the window is destroyed
				m_pHtmlView->GetAllocList().AddTail( pstrBuf );

			  for( int i = 0; i < strURL.GetLength(); i++ )
				{
					AppendChar( strURL[i] );
				}

				CommitBuffer();

				// restore the style
				GetTextStyle()->SetTextColor(	luTextColor );
				GetTextStyle()->SetStyle( luOldStyle );
				GetTextStyle()->SetUserData( 0 ); 
				
			}
			else
			{
			 // for( int i = 0; i < strURL.GetLength(); i++ )
				//{
				//	AppendChar( strURL[i] );
				//}
				
				// UE: if you append in the above fashion, it screws up entities (now that we're considering '&' to
				//     be a valid link character), so we'll just declare that nothing happened and return to sender.
				lStart = lOldStart;
				AppendChar( pstrBuffer[lStart++] );		// consume one character, so we don't cause an infinite loop
			}
		}
		else
		{
			AppendChar( pstrBuffer[lStart++]  );
		}
	}
	else
	{
		AppendChar( pstrBuffer[lStart++]  );
	}
	return false;
}

// $Log$
// Revision 1.2  2003/07/04 11:26:41  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:54:18  uecasm
// Import of source tree as at version 2.53 release.
//
