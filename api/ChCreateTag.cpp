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

#include "headers.h"


#include "ChHtmlView.h"

#include "ChHtmSym.h"
#include "ChHtmlParser.h"

#include "ChHeadElement.h"
#include "ChBodyElement.h"
#include "ChBodyTags.h"
#include "ChFontElements.h"
#include "ChFormTags.h"
#include "ChInlineTags.h"
#include "ChPlainTag.h"
#include "ChDefnTag.h"
#include "ChPaneTag.h"

#include <MemDebug.h>


ChHtmlTag* ChHtmlParser::CreateInstanceFromName( const ChString& strName )
{
	return CreateInstanceFromID( GetTokenID( strName ) );
}

ChHtmlTag* ChHtmlParser::CreateInstanceFromID( int iToken )
{
	ChHtmlTag* pTag = 0;

	switch( iToken )
	{
		case HTML_TITLE :
		{
			pTag = new ChTitleTag( this );	
			break;
		}
		case HTML_HTML :
		{
			pTag = new ChHtmlElement( this );	
			break;
		}
		case HTML_HEAD :
		{
			pTag = new ChHeadTag( this );	
			break;
		}
		case HTML_BASE :
		{
			pTag = new ChBaseTag( this );
			break;
		}
		case HTML_COMMENT :
		{
			pTag = new ChCommentTag( this );	
			break;
		}
		case HTML_PREFETCH :
		{
			pTag = new ChPrefetchTag( this );	
			break;
		}
		case HTML_PAGE :
		{
			pTag = new ChPageTag( this );	
			break;
		}
		case HTML_BODY :
		{
			pTag = new ChBodyTag( this );	
			break;
		}
		case HTML_H1 :
		case HTML_H2 :
		case HTML_H3 :
		case HTML_H4 :
		case HTML_H5 :
		case HTML_H6 :
		{	  
			pTag = new ChHeadingTag( iToken, this );
			break;
		}

		case HTML_PARAGRAPH :
		{
			pTag = new ChPragraphTag( this );
			break;
		}
		case HTML_PREFORMAT :
		{
			pTag = new ChPreFormatTag( this );
			break;
		}
		case HTML_QUOTE :
		{
			pTag = new ChQuoteTag( this );
			break;
		}
		case HTML_BLKQUOTE :
		{
			pTag = new ChBlockQuoteTag( this );
			break;
		}

		case HTML_ADDRESS :
		{
			pTag = new ChAddressTag( this );
			break;
		}

		case HTML_MENU :
		case HTML_DEFNLIST :
		case HTML_BULLETLINE :
		case HTML_NUMLIST :
		case HTML_DIR :
		{
			pTag = new ChListTags( iToken, this );
			break;
		}

		case HTML_LINK :
		{
			pTag = new ChAnchorTag( this );
			break;
		}

		case HTML_CODE :
		case HTML_SAMP :
		{
			pTag = new ChSampleTag( iToken, this );
			break;
		}
		case  HTML_CENTER :
		{
			pTag = new ChCenterTag( this );
			break;
		}
		case  HTML_NOBR :
		{
			pTag = new ChNOBRTag( this );
			break;
		}

		case HTML_BASEFONT :
		{
			pTag = new ChBaseFont( this );
			break;
		}

		// logical styles
		case HTML_BOLD :
		case HTML_ITALIC :
		case HTML_UNDERLINE :
		case HTML_STRIKETHROUGH :
		// physical styles
		case HTML_EMPHASIS :
		case HTML_CITE :
		case HTML_STRONG :
		{
			pTag = new ChCharStyle( iToken, this );
			break;
		}

		case HTML_FONT :
		{
			pTag = new ChFontTag( this );
			break;
		}

		case HTML_TYPEWRITER :
		{
			pTag = new ChTeleTypeTag( this );
			break;
		}

		case HTML_XMP :
		case HTML_LISTING :
		case HTML_PLAINTEXT :
		{
			pTag = new ChPlainTextElement( iToken, this );
			break;
		}

		case HTML_MUDTEXT :
		{
			pTag = new ChMudTextTag( this );
			break;
		}

		case HTML_LINEITEM :
		{
			pTag = new ChLineItemTag( this );
			break;
		}
		case HTML_IMAGE :
		{
			pTag = new ChImgTag( this );
			break;
		}
		case HTML_EMBED :
		{
			pTag = new ChEmbedTag( this );
			break;
		}

		case HTML_HORZRULE :
		{
			pTag = new ChHRTag( this );
			break;
		}

		case HTML_LINEBREAK :
		{
			pTag = new ChBRTag( this );
			break;
		}

		case  HTML_DEFNTERM :
		{
			pTag = new ChDefnTermTag( this );
			break;
		}
		case  HTML_DEFNTEXT :
		{
			pTag = new ChDefnTextTag( this );
			break;
		}
		case  HTML_FORM :
		{
			pTag = new ChFormTag( this );
			break;
		}
		case  HTML_INPUT :
		{
			pTag = new ChFormInputTag( this );
			break;
		}
		case HTML_TEXTAREA :
		{
			pTag = new ChFormTextAreaTag( this );
			break;
		}
		case HTML_SELECT :
		{
			pTag = new ChFormSelectTag( this );
			break;
		}
		case  HTML_OPTION :
		{
			pTag = new ChFormOptionTag( this );
			break;
		}
		case HTML_XCHPANE :
		{
			pTag = new ChPaneTag( this );
			break;
		}
		case HTML_UNKNOWN :
		default : 
		{
			pTag = new ChHtmlTag( this );
			break;
		}
	} 
	ASSERT( pTag );
	return pTag;
}
