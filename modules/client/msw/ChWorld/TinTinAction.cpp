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

	TinTin class linked list methods.  Originally modified from TinTin++,
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
	Utility functions declarations
----------------------------------------------------------------------------*/

int MatchString( const char* pstrLine, const char* pstrMask );


/*----------------------------------------------------------------------------
	TinTin public methods
----------------------------------------------------------------------------*/

void TinTin::CheckActions( const ChString& strLine )
{
	if (!IsIgnore())
	{
		TinTinListNode*	pNode = GetListActions()->GetTop();

		while (pNode)
		{
			if (CheckOneAction( strLine, pNode->GetLeft() ))
			{
				ChString		strResult;

				PrepareActionAlias( pNode->GetRight(), strResult );
			
				if (IsEcho())
				{
					ChString		strFormat;
					ChString		strMessage;

					LOADSTRING( IDS_TINTIN_ACTION_ECHO_FMT, strFormat );
					strMessage.Format( strFormat, (const char*)strResult );
					Message( strMessage );
				}

				ParseInput( strResult );
				return;
			}

			pNode = pNode->GetNext();
		}
	}
}


/*----------------------------------------------------------------------------
	TinTin protected methods
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	TinTin::PrepareActionAlias
				Run throught each of the commands on the right side of an
				alias/action expression, call substitute_text() for all
				commands but #alias/#action.
----------------------------------------------------------------------------*/

void TinTin::PrepareActionAlias( const ChString& strText, ChString& strResult )
{
	SubstituteVars( strText, strResult );
	SubstituteMyVars( strResult, strResult );
}


void TinTin::DoAction( const ChString& strArgs )
{
	const char*	pstrArgs = strArgs;
	ChString		strLeft;
	ChString		strRight;
	ChString		strPriority;
	ChString		strMessage;
	ChString		strFormat;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );
	pstrArgs = GetArgInBraces( pstrArgs, strPriority, true );

	if (strPriority.IsEmpty())
	{										/* Default priority to 5 if no
												value given */
		strPriority = "5";
	}

	if (strLeft.IsEmpty())
	{
		if (GetListActions()->GetTop())
		{
			LOADSTRING( IDS_TINTIN_ACTION_LIST_HDR, strMessage );
			Message( strMessage );

			GetListActions()->ShowList( this );
		}
		else
		{
			LOADSTRING( IDS_TINTIN_ACTION_LIST_EMPTY, strMessage );
			Message( strMessage );
		}
	}
	else if (!strLeft.IsEmpty() && strRight.IsEmpty())
	{
		TinTinListNode*		pNode;

		if (pNode = GetListActions()->SearchWithWildchars( strLeft ))
		{
			LOADSTRING( IDS_TINTIN_ACTION_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );

			do {
				GetListActions()->ShowNode( this, pNode );
				pNode = pNode->GetNext();

			} while (pNode = GetListActions()->SearchWithWildchars( strLeft,
																	pNode ));
		}
		else if (IsDisplayingMsg( msgIndexActions ))
		{
			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListActions()->Search( strLeft ))
		{
			GetListActions()->DeleteNode( pNode );
		}

		GetListActions()->InsertNode( strLeft, strRight, strPriority );

		if (IsDisplayingMsg( msgIndexActions ))
		{
			LOADSTRING( IDS_TINTIN_NEW_ACTION, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft,
											(const char*)strRight,
											(const char*)strPriority );
			Message( strMessage );
		}

		m_iActionCounter++;
	}
}


void TinTin::DoUnaction( const ChString& strArgs )
{
	bool			boolFound = false;
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	TinTinListNode*	pNode;
	TinTinListNode*	pStart = GetListActions()->GetTop();

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, true );

	if (strLeft.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_UNACTION_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		while (pNode = GetListActions()->SearchWithWildchars( strLeft, pStart ))
		{
			boolFound = true;

			if (IsDisplayingMsg( msgIndexActions ))
			{
				ChString		strFormat;
				ChString		strMessage;

				LOADSTRING( IDS_TINTIN_UNACTION, strFormat );
				strMessage.Format( strFormat, (const char*)pNode->GetLeft() );
				Message( strMessage );
			}

			pStart = pNode->GetNext();
			GetListActions()->DeleteNode( pNode );
		}

		if (!boolFound && IsDisplayingMsg( msgIndexActions ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
}


bool TinTin::CheckOneAction( const ChString& strLine, const ChString& strAction )
{
	bool		boolSuccess;
	VarLenType	iVarLen;
	VarType		strVar;


	if (boolSuccess = CompareAction( strLine, strAction, iVarLen, strVar ))
	{
		int		iLoop;
											// Copy variables
		for (iLoop = 0; iLoop < 10; iLoop++)
		{
			if (iVarLen[iLoop] != -1)
			{
				m_strVars[iLoop] = strVar[iLoop];
			}
		}
	}

	return boolSuccess;
}


bool TinTin::CompareAction( const ChString& strLine, const ChString& strAction,
							VarLenType& iVarLen, VarType& strVar )
{
	int				iLoop;
	bool			boolAnchor = false;
	const char*		pstrLine = strLine;
	const char*		pstrTrigger;
	ChString			strResult;

	for (iLoop = 0; iLoop < 10; iLoop++)
	{
		iVarLen[iLoop] = -1;
	}

	SubstituteMyVars( strAction, strResult );
	pstrTrigger = strResult;

	if(*pstrTrigger=='^')
	{
		pstrTrigger++;
		boolAnchor = true;
	}

	if (boolAnchor)
	{
		int		iLen;

		if ((iLen = MatchString( pstrLine, pstrTrigger )) == -1)
		{
			return false;
		}

		pstrLine += iLen;
		pstrTrigger += iLen;
	}
	else
	{
		bool	boolFound = false;
		int		iLen = -1;

		while (*pstrLine && !boolFound)
		{
			if ((iLen = MatchString( pstrLine, pstrTrigger )) != -1)
			{
				boolFound = true;
			}
			else
			{
				pstrLine++;
			}
		}

		if (iLen != -1)
		{
			pstrLine += iLen;
			pstrTrigger += iLen;
		}
		else
		{
			return false;
		}
	}

	while (*pstrLine && *pstrTrigger)
	{
		bool			boolMatch = true;
		int				iLen = -1;
		const char*		pstrTemp2 = pstrTrigger + 2;
		const char*		pstrLine2;

		if (!*pstrTemp2)
		{
			int		iIndex = *(pstrTrigger + 1) - '0';

			if ((iIndex >= 0) && (iIndex <= 9))
			{
				strVar[iIndex] = pstrLine;
				iVarLen[iIndex] = strVar[iIndex].GetLength();
			}

			return true;
		}

		pstrLine2 = pstrLine;
		while (*pstrLine2 && boolMatch)
		{
			if ((iLen = MatchString( pstrLine2, pstrTemp2 )) != -1)
			{
				boolMatch = false;
			}
			else 
			{
				pstrLine2++;
			}
		}

		if (iLen != -1)
		{
			int		iIndex = *(pstrTrigger + 1) - '0';
			int		iValueLen = pstrLine2 - pstrLine;
			ChString	strTemp( pstrLine, iValueLen );

			iVarLen[iIndex] = iValueLen;
			strVar[iIndex] = strTemp;

			pstrLine = pstrLine2 + iLen;
			pstrTrigger = pstrTemp2 + iLen;
		}
		else
		{
			return false;
		}
	}

	if (*pstrTrigger)
	{
		return false;
	}
	else
	{
		return true;
	}
}


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

int MatchString( const char* pstrLine, const char* pstrMask )
{
	const char*		pstrStart = pstrLine;

	while (*pstrLine && *pstrMask &&
			!((*pstrMask == '%') && isdigit( *(pstrMask + 1) )))
	{
		if (*pstrLine++ != *pstrMask++)
		{
			return -1;
		}
	}

	if (!*pstrMask ||
		((*pstrMask == '%') && isdigit( *(pstrMask + 1) )))
	{
		return (int)(pstrLine - (const char*)pstrStart);
	}

	return -1;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:35  uecasm
// Import of source tree as at version 2.53 release.
//
