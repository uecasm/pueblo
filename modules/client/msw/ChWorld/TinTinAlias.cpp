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

	TinTin alias-related methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ctype.h>
#include <signal.h>

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

void TinTin::DoAlias( const ChString& strArgs )
{
	const char*	pstrArgs = strArgs;
	ChString		strLeft;
	ChString		strRight;
	ChString		strMessage;
	ChString		strFormat;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strLeft.IsEmpty())
	{
		if (GetListAliases()->GetTop())
		{
			LOADSTRING( IDS_TINTIN_ALIAS_LIST_HDR, strMessage );
			Message( strMessage );

			GetListAliases()->ShowList( this );
		}
		else
		{
			LOADSTRING( IDS_TINTIN_ALIAS_LIST_EMPTY, strMessage );
			Message( strMessage );
		}
	}
	else if (!strLeft.IsEmpty() && strRight.IsEmpty())
	{
		TinTinListNode*		pNode;

		if (pNode = GetListAliases()->SearchWithWildchars( strLeft ))
		{
			LOADSTRING( IDS_TINTIN_ALIAS_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );

			do {
				GetListAliases()->ShowNode( this, pNode );
				pNode = pNode->GetNext();

			} while (pNode = GetListAliases()->SearchWithWildchars( strLeft,
																	pNode ));
		}
		else if (IsDisplayingMsg( msgIndexAliases ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListAliases()->Search( strLeft ))
		{
			GetListAliases()->DeleteNode( pNode );
		}

		GetListAliases()->InsertNode( strLeft, strRight, "0" );

		if (IsDisplayingMsg( msgIndexAliases ))
		{
			ChString		strTemp;

			LOADSTRING( IDS_TINTIN_NEW_ALIAS, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft,
											(const char*)strRight );
			Message( strMessage );
		}

		m_iAliasCounter++;
	}
}


void TinTin::DoUnalias( const ChString& strArgs )
{
	bool			boolFound = false;
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	TinTinListNode*	pNode;
	TinTinListNode*	pStart = GetListAliases()->GetTop();

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, true );

	if (strLeft.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_UNALIAS_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		while (pNode = GetListAliases()->SearchWithWildchars( strLeft, pStart ))
		{
			boolFound = true;

			if (IsDisplayingMsg( msgIndexAliases ))
			{
				ChString		strTemp;

				strTemp.Format( "# Ok.  {%s} is no longer an alias.",
									(const char*)pNode->GetLeft() );
				Message( strTemp );
			}

			pStart = pNode->GetNext();
			GetListAliases()->DeleteNode( pNode );
		}


		if (!boolFound && IsDisplayingMsg( msgIndexAliases ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:38  uecasm
// Import of source tree as at version 2.53 release.
//
