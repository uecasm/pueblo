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

	TinTin class utility methods.  Originally modified from TinTin++,
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
	Utility functions
----------------------------------------------------------------------------*/

inline bool IsVarNameChar( char cTest )
{
	return (isalnum( cTest ) || ('_' == cTest));
}


inline bool IsNumeric( const ChString& strVal )
{
	if (strVal.IsEmpty())
	{										// Empty strings are non-numeric
		return false;
	}

	const char*		pstrVal = strVal;

	if ((*pstrVal == '-') && (strVal.GetLength() > 1))
	{
											/* Forgive a '-' sign if it is the
												first character in the string
												and there is more than just the
												'-' */
		pstrVal++;
	}

	while (*pstrVal)
	{
		if (!isdigit( *pstrVal ))
		{
			return false;
		}

		pstrVal++;
	}
											// All digits
	return true;
}


CH_INTERN_FUNC( void )
AppendQuotedArg( ChString& strDest, const ChString& strAppend )
{
	ChString		strTemp;
	const char*	pstrAppend = strAppend;
	bool		boolNumeric = IsNumeric( strAppend );

	if (!boolNumeric)
	{
		strTemp += '"';
	}

	while (*pstrAppend)
	{
		if (('"' == *pstrAppend) || ('\\' == *pstrAppend))
		{
			strTemp += '\\';
		}

		strTemp += *pstrAppend;
		pstrAppend++;
	}

	if (!boolNumeric)
	{
		strTemp += '"';
	}

	strDest += strTemp;
}


/*----------------------------------------------------------------------------
	TinTin class protected methods
----------------------------------------------------------------------------*/

void TinTin::DoChangeCase( const ChString& strArgs, bool boolToUpper )
{
	const char*	pstrArgs = strArgs;
	ChString		strLeft;
	ChString		strRight;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strLeft.IsEmpty() || strRight.IsEmpty())
	{
		const char*	pstrCmd = boolToUpper ? "toupper" : "tolower";
		ChString		strFormat;
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_CASE_PARAM_ERR, strFormat );
		strMessage.Format( strFormat, pstrCmd );
		Message( strMessage );
	}
	else if (!IsVarNameValid( strLeft ))
	{
		ChString		strFormat;
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_VAR_NAME_ERR, strFormat );
		strMessage.Format( strFormat, (const char*)strLeft );
		ErrMessage( strMessage );
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListVars()->Search( strLeft ))
		{
			GetListVars()->DeleteNode( pNode );
		}

		if (boolToUpper)
		{
			strRight.MakeUpper();
		}
		else
		{
			strRight.MakeLower();
		}

		GetListVars()->InsertNode( strLeft, strRight, "0" );

		if (IsDisplayingMsg( msgIndexVariable ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NEW_VAR, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft,
											(const char*)strRight );
			Message( strMessage );
		}

		m_iVarCounter++;
	}
}


void TinTin::DoVariable( const ChString& strArgs )
{
	const char*	pstrArgs = strArgs;
	ChString		strLeft;
	ChString		strRight;
	ChString		strFormat;
	ChString		strMessage;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strLeft.IsEmpty())
	{
		if (GetListVars()->GetTop())
		{
			LOADSTRING( IDS_TINTIN_VAR_LIST_HDR, strMessage );
			Message( strMessage );

			GetListVars()->ShowList( this );
		}
		else
		{
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_VAR_LIST_EMPTY, strMessage );
			Message( strMessage );
		}
	}
	else if (!strLeft.IsEmpty() && !IsVarNameValid( strLeft ))
	{
		ChString		strFormat;
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_VAR_NAME_ERR, strFormat );
		strMessage.Format( strFormat, (const char*)strLeft );
		ErrMessage( strMessage );
	}
	else if (!strLeft.IsEmpty() && strRight.IsEmpty())
	{
		TinTinListNode*		pNode;

		if (pNode = GetListVars()->SearchWithWildchars( strLeft ))
		{
			LOADSTRING( IDS_TINTIN_VAR_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );

			do {
				GetListVars()->ShowNode( this, pNode );
				pNode = pNode->GetNext();

			} while (pNode = GetListVars()->SearchWithWildchars( strLeft,
																	pNode ));
		}
		else if (IsDisplayingMsg( msgIndexVariable ))
		{
			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			ErrMessage( strMessage );
		}
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListVars()->Search( strLeft ))
		{
			GetListVars()->DeleteNode( pNode );
		}

		GetListVars()->InsertNode( strLeft, strRight, "0" );

		if (IsDisplayingMsg( msgIndexVariable ))
		{
			LOADSTRING( IDS_TINTIN_NEW_VAR, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft,
											(const char*)strRight );
			Message( strMessage );
		}

		m_iVarCounter++;
	}
}


void TinTin::DoUnvariable( const ChString& strArgs )
{
	bool			boolFound = false;
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	TinTinListNode*	pNode;
	TinTinListNode*	pStart = GetListVars()->GetTop();

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, true );

	if (strLeft.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_UNVAR_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else if (!IsVarNameValid( strLeft ))
	{
		ChString		strFormat;
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_VAR_NAME_ERR, strFormat );
		strMessage.Format( strFormat, (const char*)strLeft );
		ErrMessage( strMessage );
	}
	else
	{
		while (pNode = GetListVars()->SearchWithWildchars( strLeft, pStart ))
		{
			boolFound = true;

			if (IsDisplayingMsg( msgIndexVariable ))
			{
				ChString		strFormat;
				ChString		strMessage;

				LOADSTRING( IDS_TINTIN_UNVAR_OK, strFormat );
				strMessage.Format( strFormat, (const char*)strLeft );
				Message( strMessage );
			}

			pStart = pNode->GetNext();
			GetListVars()->DeleteNode( pNode );
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


/*----------------------------------------------------------------------------
	TinTin::SubstituteVars
					Copy 'pstrArg' into 'strOut', but substitute
					the variables %0..%9 with the real variables.
----------------------------------------------------------------------------*/

void TinTin::SubstituteVars( const char* pstrArg, ChString& strOut,
								bool boolQuoted )
{
	ChString		strResult;
	int			iPercentCount;
	int			iNestLevel = 0;
	int			iArgNumber;

	while (*pstrArg)
	{
		if (*pstrArg == '%')
		{									// Substitute variable
			iPercentCount = 0;

			while (*(pstrArg + iPercentCount) == '%') 
			{
				iPercentCount++;
			}

			if (isdigit( *(pstrArg + iPercentCount) ) &&
				(iPercentCount == (iNestLevel + 1)))
			{
				iArgNumber = *(pstrArg + iPercentCount) - '0';

											// Add the value
				if (boolQuoted)
				{
					AppendQuotedArg( strResult, m_strVars[iArgNumber] );
				}
				else
				{
					strResult += m_strVars[iArgNumber];
				}

				pstrArg += iPercentCount + 1;
			}
			else
			{
				ChString		strTemp( pstrArg );

				strResult += strTemp.Left( iPercentCount + 1 );
				pstrArg += iPercentCount + 1;
			}
		}

		if (*pstrArg=='$')
		{									/* Substitute variable without
												semicolons */
			iPercentCount = 0;

			while (*(pstrArg + iPercentCount) == '$')
			{
				iPercentCount++;
			}

			if (isdigit( *(pstrArg + iPercentCount) ) &&
				(iPercentCount == (iNestLevel + 1)))
			{
				const char*		pstrText;
				ChString			strTemp;

				iArgNumber = *(pstrArg + iPercentCount) - '0';

				pstrText = m_strVars[iArgNumber];
				while (*pstrText)
				{
					if (*pstrText == ';')
					{
						pstrText++;
					}
					else
					{
						strTemp += *pstrText++;
					}
											/* Add the value with semicolons
												stripped out */
					if (boolQuoted)
					{
						AppendQuotedArg( strResult, strTemp );
					}
					else
					{
						strResult += strTemp;
					}
				}

				pstrArg += iPercentCount + 1;
			}
			else
			{
				ChString		strTemp( pstrArg );

				strResult += strTemp.Left( iPercentCount + 1 );
				pstrArg += iPercentCount + 1;
			}
		}
		else if (*pstrArg == DEF_OPEN)
		{
			iNestLevel++;
			strResult += *pstrArg++;
		}
		else if (*pstrArg==DEF_CLOSE)
		{
			iNestLevel--;
			strResult += *pstrArg++;
		}
		else if ((*pstrArg == '\\') && (iNestLevel == 0))
		{
			while (*pstrArg == '\\')
			{
				strResult += *pstrArg++;
			}

			if (*pstrArg == '%')
			{
				strResult += '\\';
				strResult += *pstrArg++;
			}
		}
		else
		{
			strResult += *pstrArg++;
		}
	}

	strOut = strResult;
}


/*----------------------------------------------------------------------------
	TinTin::SubstituteMyVars
					Substitute the variables $<string> with the values
					they stand for.
----------------------------------------------------------------------------*/

void TinTin::SubstituteMyVars( const char* pstrArg, ChString& strOut,
								bool boolQuoted )
{
	ChString			strResult;
	int				iNestLevel = 0;

	while (*pstrArg)
	{
		if (*pstrArg == '$')
		{									// Substitute variable
			int		iCounter = 0;
			int		iVarLen = 0;
			ChString	strVarName;

			while (*(pstrArg + iCounter) == '$')
			{
				iCounter++;
			}

			while (::IsVarNameChar( *(pstrArg + iVarLen + iCounter) ))
			{
				iVarLen++;
			}

			if (iVarLen > 0)
			{
				strVarName = (const char*)(pstrArg + iCounter);
				strVarName = strVarName.Left( iVarLen );
			}

			if ((iCounter == iNestLevel + 1) &&
				!isdigit( *(pstrArg + iCounter + 1) ))
			{
				TinTinListNode*		pNode;

				pNode = GetListVars()->Search( strVarName );

				if (pNode)
				{							// Add the value
					if (boolQuoted)
					{
						AppendQuotedArg( strResult, pNode->GetRight() );
					}
					else
					{
						strResult += pNode->GetRight();
					}

					pstrArg += iCounter + iVarLen;
				}
				else
				{
					ChString		strTemp( pstrArg );

					strResult += strTemp.Left( iCounter + iVarLen );
					pstrArg += iCounter + iVarLen;
				}
			}
			else
			{
				ChString		strTemp( pstrArg );

				strResult += strTemp.Left( iCounter + iVarLen );
				pstrArg += iVarLen + iCounter;
			}
		}
		else if (*pstrArg == DEF_OPEN)
		{
			iNestLevel++;
			strResult += *pstrArg++;
		}
		else if (*pstrArg == DEF_CLOSE)
		{
			iNestLevel--;
			strResult += *pstrArg++;
		}
		else if ((*pstrArg == '\\') && (*(pstrArg + 1) == '$') &&
					(iNestLevel == 0))
		{
			pstrArg++;
			strResult += *pstrArg++;
		}
		else
		{
			strResult += *pstrArg++;
		}
	}

	strOut = strResult;
}


bool TinTin::IsVarNameValid( const ChString& strName )
{
	const char*		pstrName = strName;

	while (*pstrName)
	{
		if (!IsVarNameChar( *pstrName ))
		{
			return false;
		}

		pstrName++;
	}

	return true;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:43  uecasm
// Import of source tree as at version 2.53 release.
//
