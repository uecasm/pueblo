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

// $Header$

#include "headers.h"

#include <ChHtmlSettings.h>
#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChDefnTag.h"

#include <MemDebug.h>




///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChDefnTermTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChDefnTermTag::ChDefnTermTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_DEFNTERM;
	m_luAttrs =  attrStartLine | attrCallProcessTag ;
}

chint32	 ChDefnTermTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	m_pParser->GetTextStyle()->SetLeftIndent(m_pParser->GetCurrentIndent());

	return ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );
}

///////////////////////////////////////////////////////////////////////////////
////////////
/////////////	ChDefnTermTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChDefnTextTag::ChDefnTextTag( ChHtmlParser *pParser ) : ChBodyElements( pParser )
{
	m_iTokenID = HTML_DEFNTEXT;
	m_luAttrs =  attrStartLine | attrCallProcessTag ;
}

chint32	 ChDefnTextTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	// Find the indent and set it
	m_pParser->GetTextStyle()->SetLeftIndent( 
					m_pParser->GetCurrentIndent() + 
					m_pParser->GetLeftIndent() * indentFactor );


	return ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );

}

// $Log$
