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

	TinTin class miscellaneous methods.  Originally modified from TinTin++,
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

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

void TinTin::DoSubstitute( const ChString& strArgs )
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
		if (GetListSubs()->GetTop())
		{
			LOADSTRING( IDS_TINTIN_SUBS_LIST_HDR, strMessage );
			Message( strMessage );

			GetListSubs()->ShowList( this );
		}
		else
		{
			LOADSTRING( IDS_TINTIN_SUBS_LIST_EMPTY, strMessage );
			Message( strMessage );
		}
	}
	else if (!strLeft.IsEmpty() && strRight.IsEmpty())
	{
		TinTinListNode*		pNode;

		if (pNode = GetListSubs()->SearchWithWildchars( strLeft ))
		{
			LOADSTRING( IDS_TINTIN_SUBS_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );

			do {
				GetListSubs()->ShowNode( this, pNode );
				pNode = pNode->GetNext();

			} while (pNode = GetListSubs()->SearchWithWildchars( strLeft,
																	pNode ));
		}
		else if (IsDisplayingMsg( msgIndexSubstitutions ))
		{
			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListSubs()->Search( strLeft ))
		{
			GetListSubs()->DeleteNode( pNode );
		}

		GetListSubs()->InsertNode( strLeft, strRight, "0" );

		if (IsDisplayingMsg( msgIndexSubstitutions ))
		{
			ChString		strTemp;

			if (0 == strRight.Compare( TINTIN_DEF_GAG_STRING ))
			{
				LOADSTRING( IDS_TINTIN_GAG_OK, strFormat );
				strTemp.Format( strFormat, (const char*)strLeft );
			}
			else
			{
				LOADSTRING( IDS_TINTIN_SUBS_OK, strFormat );
				strTemp.Format( strFormat, (const char*)strRight,
											(const char*)strLeft );
			}
			Message( strTemp );
		}

		m_iSubCounter++;
	}
}


void TinTin::DoUnsubstitute( const ChString& strArgs, bool boolUngag )
{
	bool			boolFound = false;
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	TinTinListNode*	pNode;
	TinTinListNode*	pStart = GetListSubs()->GetTop();
	ChString			strMessage;
	ChString			strFormat;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, true );

	if (strLeft.IsEmpty())
	{
		LOADSTRING( IDS_TINTIN_UNSUB_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		while (pNode = GetListSubs()->SearchWithWildchars( strLeft, pStart ))
		{
			bool	boolMatch = false;

			if (boolUngag)
			{
				if (0 == pNode->GetRight().Compare( TINTIN_DEF_GAG_STRING ))
				{
					boolMatch = true;
				}
			}
			else
			{
				boolMatch = true;
			}

			if (boolMatch)
			{
				boolFound = true;

				if (IsDisplayingMsg( msgIndexSubstitutions ))
				{
					ChString		strTemp;

					if (boolUngag)
					{
						LOADSTRING( IDS_TINTIN_UNGAG_OK, strFormat );
					}
					else
					{
						LOADSTRING( IDS_TINTIN_UNSUB_OK, strFormat );
					}

					strMessage.Format( strFormat,
										(const char*)pNode->GetLeft() );
					Message( strMessage );
				}

				pStart = pNode->GetNext();
				GetListSubs()->DeleteNode( pNode );
			}
		}

		if (!boolFound && IsDisplayingMsg( msgIndexSubstitutions ))
		{
			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
}


#if 0

/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: substitute.c - functions related to the substitute command  */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#include "tintin.h"

extern char *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

extern struct listnode *common_subs;
extern char vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */
extern int subnum;
extern int mesvar[7];


void do_one_sub(line, ses)
     char *line;
     struct session *ses;
{
  struct listnode *ln;
  ln=ses->subs;

  while((ln=ln->next)) 
      if(check_one_action(line,ln->left, ses))
      prepare_actionalias(ln->right, line, ses);
}

#endif	// 0

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:42  uecasm
// Import of source tree as at version 2.53 release.
//
