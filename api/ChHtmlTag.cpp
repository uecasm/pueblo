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

	This file contains the implementation for the ChHtmlTag class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ctype.h>



#include "ChHtmlView.h"
#include <ChHtmlSettings.h>
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChHtmlTag.h"

#include <MemDebug.h>

ChHtmlTag::ChHtmlTag( ChHtmlParser *pParser ) :
				 m_pParser( pParser ),
				 m_iTokenID( HTML_UNKNOWN ),
				 m_luAttrs( attrCallProcessTag ), 
				 m_luFontAttrs(0), 
				 m_luStateToRestore(0)
{
 	ASSERT( pParser );
}	


ChHtmlTag::~ChHtmlTag()
{

}

chint32	 ChHtmlTag::GetPointSize()
{
	ChHtmlSettings *pSetting = GetHtmlView()->GetSettings();

	if ( m_luAttrs & attrFontProportional )
	{
		return ( pSetting->GetPixelHt() * (pSetting->GetProportionalFontSize())/ 72);
	}
	else
	{
		return (pSetting->GetPixelHt() * (	pSetting->GetFixedFontSize())/ 72);
	}
}	


chint32	 ChHtmlTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ) 
{
	// skip till the next tag
	if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		while ( lStart < lCount && pstrBuffer[lStart] != TEXT( '<' )  
							&& IS_WHITE_SPACE( pstrBuffer[lStart] ) )
		{
			lStart++;
		}
	}

	return lStart;
}

// $Log$
