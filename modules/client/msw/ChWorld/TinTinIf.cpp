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


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define MATH_STACK_SIZE		100


/*----------------------------------------------------------------------------
	Static variables
----------------------------------------------------------------------------*/

CH_INTERN_VAR MathOps	mathStack[MATH_STACK_SIZE];


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

inline void MakeString( int iVal, ChString& strVal )
{
	strVal.Format( "%d", iVal );
}


/*----------------------------------------------------------------------------
	TinTin class protected methods
----------------------------------------------------------------------------*/

void TinTin::DoIf( const ChString& strArgs )
{
	const char*		pstrArgs = strArgs;
	ChString			strCondition;
	ChString			strThen;
	ChString			strElse;
	ChString			strTemp;

	pstrArgs = GetArgInBraces( pstrArgs, strCondition, false );
	pstrArgs = GetArgInBraces( pstrArgs, strThen, true );
	pstrArgs = GetArgInBraces( pstrArgs, strElse, true );

	SubstituteVars( strCondition, strTemp, true );
	SubstituteMyVars( strTemp, strCondition, true );

	EvalExpression( strCondition, strTemp );
	if (strTemp.IsEmpty() || strTemp == m_strFalse)
	{
		SubstituteVars( strElse, strTemp );
		SubstituteMyVars( strTemp, strElse );

		ParseInput( strElse );
	}
	else
	{
		SubstituteVars( strThen, strTemp );
		SubstituteMyVars( strTemp, strThen );

		ParseInput( strThen );
	}
}


void TinTin::DoMath( const ChString& strArgs )
{
	const char*		pstrArgs = strArgs;
	ChString			strVar;
	ChString			strRight;
	ChString			strTemp;
	TinTinListNode*	pNode;

	pstrArgs = GetArgInBraces( pstrArgs, strVar, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strVar.IsEmpty() || strRight.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_MATH_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else if (!IsVarNameValid( strVar ))
	{
		ChString		strFormat;
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_VAR_NAME_ERR, strFormat );
		strMessage.Format( strFormat, (const char*)strVar );
		ErrMessage( strMessage );
	}
	else
	{
		SubstituteVars( strRight, strTemp, true );
		SubstituteMyVars( strTemp, strRight, true );

		EvalExpression( strRight, strTemp );

		if (pNode = GetListVars()->Search( strVar ))
		{
			GetListVars()->DeleteNode( pNode );
		}

		GetListVars()->InsertNode( strVar, strTemp, "0" );
	}
}


void TinTin::EvalExpression( const char* pstrArgs, ChString& strResult )
{
	::MakeString( true, strResult );
											// Parse the expression onto the stack
	if (CompileExpression( pstrArgs ))
	{
		bool	boolDone = false;

		while (!boolDone)
		{
			int		iIndex = 0;
			int		iBegin = -1;
			int		iEnd = -1;
			int		iPrev = -1;
			bool	boolCloseParenFound = false;

			while (mathStack[iIndex].GetNext() && !boolCloseParenFound)
			{
				if (mathStack[iIndex].GetOp() == opOpenParen)
				{
					iBegin = iIndex;
				}
				else if (mathStack[iIndex].GetOp() == opCloseParen)
				{
					iEnd = iIndex;
					boolCloseParenFound = true;
				}

				iPrev = iIndex;
				iIndex = mathStack[iIndex].GetNext();
			}

			if ((!boolCloseParenFound && (iBegin != -1)) ||
					(boolCloseParenFound && (iBegin == -1)))
			{
				ErrMessage( "#Error - Unmatched parentheses." );
				boolDone = true;
				::MakeString( false, strResult );
			}
			else if (!boolCloseParenFound)
			{
				if (iPrev == -1)
				{
					boolDone = true;

					if (mathStack[0].IsString())
					{
						strResult = *(mathStack[0].GetString());
					}
					else
					{
						::MakeString( mathStack[0].GetInt(), strResult );
					}
				}
				else
				{
					iBegin = -1;
					iEnd = iIndex;
				}
			}
			
			if (!boolDone && !DoOneInside( iBegin, iEnd ))
			{
				ChString		strMessage;

				strMessage.Format( "# Error - Invalid expression to evaluate in {%s}",
										pstrArgs );
				ErrMessage( strMessage );
				::MakeString( false, strResult );
				boolDone = true;
			}
		}
	}
	else
	{
		::MakeString( false, strResult );
	}

	ResetExpression();
}


bool TinTin::CompileExpression( const char* pstrArgs )
{
	int			iIndex = 0;

	while (*pstrArgs)
	{
		if (isdigit( *pstrArgs ))
		{
			const char*		pstrStart;
			int				iVal;

			mathStack[iIndex].SetOp( opInt );

			pstrStart = pstrArgs;
			while (isdigit( *pstrArgs ))
			{
				pstrArgs++;
			}

			sscanf( pstrStart, "%d", &iVal );
			mathStack[iIndex].Set( iVal );
			pstrArgs--;
		}
		else
		{
			switch( *pstrArgs )
			{
				case ' ':
				{
					break;
				}

				case '(':
				{
					mathStack[iIndex].SetOp( opOpenParen );
					break;
				}

				case ')':
				{
					mathStack[iIndex].SetOp( opCloseParen );
					break;
				}

				case '!':
				{
					if (*(pstrArgs + 1) == '=')
					{
						mathStack[iIndex].SetOp( opCompNotEqual );
						pstrArgs++;
					}
					else
					{
						mathStack[iIndex].SetOp( opBooleanNot );
					}
					break;
				}

				case '_':
				{
					mathStack[iIndex].SetOp( opLowercase );
					break;
				}

				case '*':
				{
					mathStack[iIndex].SetOp( opMult );
					break;
				}

				case '/':
				{
					mathStack[iIndex].SetOp( opDiv );
					break;
				}

				case '+':
				{
					mathStack[iIndex].SetOp( opAdd );
					break;
				}

				case '-':
				{
					TTOperator		oper = opInvalid;

					if (iIndex > 0)
					{
						oper = mathStack[iIndex-1].GetOp();
					}

					if (oper == opInt)
					{
						mathStack[iIndex].SetOp( opSubtract );
					}
					else
					{
						const char*		pstrStart;
						int				iVal;

						pstrStart = pstrArgs;
						pstrArgs++;
						while (isdigit(*pstrArgs))
						{
							pstrArgs++;
						}

						sscanf( pstrStart, "%d", &iVal );
						mathStack[iIndex].Set( iVal );
						mathStack[iIndex].SetOp( opInt );
						pstrArgs--;
					}
					break;
				}

				case '>':
				{
					if (*(pstrArgs + 1) == '=')
					{
						mathStack[iIndex].SetOp( opCompGreaterEqual );
						pstrArgs++;
					}
					else
					{
						mathStack[iIndex].SetOp( opCompGreater );
					}
					break;
				}

				case '<':
				{
					if (*(pstrArgs+1) == '=')
					{
						pstrArgs++;
						mathStack[iIndex].SetOp( opCompLessEqual );
					}
					else
					{
						mathStack[iIndex].SetOp( opCompLess );
					}
					break;
				}

				case '=':
				{
					mathStack[iIndex].SetOp( opCompEqual );

					if (*(pstrArgs + 1) == '=')
					{
						pstrArgs++;
					}
					break;
				}

				case '&':
				{
					mathStack[iIndex].SetOp( opBoolAnd );

					if (*(pstrArgs + 1) == '&')
					{
						pstrArgs++;
					}
					break;
				}

				case '|':
				{
					mathStack[iIndex].SetOp( opBoolOr );

					if (*(pstrArgs+1) == '|')
					{
						pstrArgs++;
					}
					break;
				}
											/* Strings can start with a single
												or double quote */
				case '\'':
				case '"':
				{
					char	cQuote = *pstrArgs;
					ChString	strTemp;

					mathStack[iIndex].SetOp( opInt );

					pstrArgs++;
					while (*pstrArgs && (*pstrArgs != cQuote))
					{
						if ('\\' == *pstrArgs)
						{
							pstrArgs++;
						}

						if (*pstrArgs)
						{
							strTemp += *pstrArgs;
							pstrArgs++;
						}
					}

					mathStack[iIndex].SetOp( opString );
					mathStack[iIndex].Set( strTemp );
					break;
				}

				case 'T':
				{
					mathStack[iIndex].SetOp( opInt );
					mathStack[iIndex].Set( true );
					break;
				}

				case 'F':
				{
					mathStack[iIndex].SetOp( opInt );
					mathStack[iIndex].Set( false );
					break;
				}

				default:
				{
					Message( "# Error - Invalid expression in #if or #math." );
					return false;
				}
			}
		}

		if (*pstrArgs != ' ')
		{
			mathStack[iIndex].SetNext( iIndex + 1 );
			iIndex++;
		}

		pstrArgs++;
	}

	if (iIndex > 0)
	{
		mathStack[iIndex].SetNext( 0 );
	}

	return true;
}


void TinTin::ResetExpression()
{
	int		iLoop;

	for (iLoop = 0; iLoop < MATH_STACK_SIZE; iLoop++)
	{
		mathStack[iLoop].Reset();
	}
}


bool TinTin::DoOneInside( int iBegin, int iEnd )
{
	while (true)
	{
		TTOperator	highestOp = opMaximum;
		int			iSearchPrevIndex = -1;
		int			iSearchCurrIndex = 0;
		int			iOpIndex = -1;
		int			iOpLeftIndex = -1;
		int			iOpRightIndex;

		if (iBegin > -1)
		{									/* If we have an explicit start
												point, then start searching
												for the active operator just
												after this point */

			iSearchCurrIndex = mathStack[iBegin].GetNext();
		}
											/* Search for the highest operator
												with the highest precedence */
		while (iSearchCurrIndex < iEnd)
		{
			if (mathStack[iSearchCurrIndex].GetOp() < highestOp)
			{
											/* We found an operator of higher
												precedence.  Store pointers to
												this operator. */

				highestOp = mathStack[iSearchCurrIndex].GetOp();
				iOpIndex = iSearchCurrIndex;
				iOpLeftIndex = iSearchPrevIndex;
			}

			iSearchPrevIndex = iSearchCurrIndex;
			iSearchCurrIndex = mathStack[iSearchCurrIndex].GetNext();
		}

		if (highestOp == opInt)
		{									/* The only thing between iBegin
												and iEnd was an integer */
			if (iBegin > -1)
			{
				mathStack[iBegin].SetNext( mathStack[iEnd].GetNext() );
				mathStack[iBegin].SetOp( opInt );
				mathStack[iBegin].Set( mathStack[iOpIndex].GetInt() );

				return true;
			}
			else
			{
				mathStack[0].SetNext( mathStack[iEnd].GetNext() );
				mathStack[0].SetOp( opInt );
				mathStack[0].Set( mathStack[iOpIndex].GetInt() );

				return true;
			}
		}
		else if (highestOp == opString)
		{									/* The only thing between iBegin
												and iEnd was a string */
			if (iBegin > -1)
			{
				mathStack[iBegin].SetNext( mathStack[iEnd].GetNext() );
				mathStack[iBegin].SetOp( opString );
				mathStack[iBegin].Set( mathStack[iOpIndex].GetString() );

				return true;
			}
			else
			{
				mathStack[0].SetNext( mathStack[iEnd].GetNext() );
				mathStack[0].SetOp( opString );
				mathStack[0].Set( mathStack[iOpIndex].GetString() );

				return true;
			}
		}
		else if (highestOp == opBooleanNot)
		{									// We found a boolean NOT operator

			iOpRightIndex = mathStack[iOpIndex].GetNext();

			if (!mathStack[iOpRightIndex].IsValue() ||
				(mathStack[iOpRightIndex].GetNext() == 0))
			{
											/* It's an error if nothing follows
												the NOT operator, or if it's
												not followed by a value */
				return false;
			}

			mathStack[iOpIndex].SetNext( mathStack[iOpRightIndex].GetNext() );

			if (mathStack[iOpRightIndex].IsString())
			{
				ChString		strRight( *(mathStack[iOpRightIndex].GetString()) );

											/* This operator makes a string
												value into an integer.  If the
												string is empty or "0", then
												the value becomes true.
												Otherwise the value becomes
												false. */

				mathStack[iOpIndex].SetOp( opInt );

				if (strRight.IsEmpty() || (strRight == m_strFalse))
				{
					mathStack[iOpIndex].Set( true );
				}
				else
				{
					mathStack[iOpIndex].Set( false );
				}
			}
			else
			{
				mathStack[iOpIndex].SetOp( opInt );
				mathStack[iOpIndex].Set( !mathStack[iOpRightIndex].GetInt() );
			}
		}
		else if (highestOp == opLowercase)
		{									/* We found a lowercase operator
												for strings */

			iOpRightIndex = mathStack[iOpIndex].GetNext();

			if (!mathStack[iOpRightIndex].IsValue() ||
				(mathStack[iOpRightIndex].GetNext() == 0))
			{
											/* It's an error if nothing follows
												the lowercase operator, or if
												it's not followed by a value */
				return false;
			}

			mathStack[iOpIndex].SetNext( mathStack[iOpRightIndex].GetNext() );

			if (mathStack[iOpRightIndex].IsString())
			{
											// Lowercase the affected string

				mathStack[iOpIndex].SetOp( opString );
				mathStack[iOpIndex].Set( mathStack[iOpRightIndex].GetString() );
				mathStack[iOpIndex].GetString()->MakeLower();
			}
			else
			{								// Copy the integer value unchanged

				mathStack[iOpIndex].SetOp( opInt );
				mathStack[iOpIndex].Set( mathStack[iOpRightIndex].GetInt() );
			}
		}
		else
		{
			iOpRightIndex = mathStack[iOpIndex].GetNext();

											/* Make sure that the left and right
												of this operation are values */

			if ((iOpLeftIndex == -1) || (mathStack[iOpRightIndex].GetNext() == 0) ||
				!mathStack[iOpRightIndex].IsValue())
			{
				return false;
			}

			if (!mathStack[iOpLeftIndex].IsValue())
			{
				return false;
			}
											// Perform the operation
			switch (highestOp)
			{
				case opMult:
				{
					if (!mathStack[iOpLeftIndex].IsInt() ||
						!mathStack[iOpRightIndex].IsInt())
					{
						return false;
					}

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					mathStack[iOpLeftIndex].GetInt() *=
									mathStack[iOpRightIndex].GetInt();
					break;
				}

				case opDiv:
				{
					if (!mathStack[iOpLeftIndex].IsInt() ||
						!mathStack[iOpRightIndex].IsInt())
					{
						return false;
					}

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					mathStack[iOpLeftIndex].GetInt() /=
									mathStack[iOpRightIndex].GetInt();
					break;
				}

				case opAdd:
				{
					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );

					if (mathStack[iOpLeftIndex].IsString() ||
						mathStack[iOpRightIndex].IsString())
					{
											// String concatenation
						ChString		strLeft;
						ChString		strRight;
						ChString		strTemp;

						MakeString( mathStack[iOpLeftIndex], strLeft );
						MakeString( mathStack[iOpRightIndex], strRight );
						strTemp = strLeft + strRight;

						mathStack[iOpLeftIndex].SetOp( opString );
						mathStack[iOpLeftIndex].Set( strTemp );
					}
					else
					{
						mathStack[iOpLeftIndex].GetInt() +=
										mathStack[iOpRightIndex].GetInt();
					}
					break;
				}

				case opSubtract:
				{
					if (!mathStack[iOpLeftIndex].IsInt() ||
						!mathStack[iOpRightIndex].IsInt())
					{
						return false;
					}

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					mathStack[iOpLeftIndex].GetInt() -=
									mathStack[iOpRightIndex].GetInt();
					break;
				}

				case opCompGreater:
				case opCompGreaterEqual:
				case opCompLess:
				case opCompLessEqual:
				case opCompEqual:
				case opCompNotEqual:
				{
					int		iResult;
											// Compare the left and right

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					iResult = CompareValues( mathStack[iOpLeftIndex],
												mathStack[iOpRightIndex],
												highestOp );

											/* The result of a comparison is
												always an int value */

					mathStack[iOpLeftIndex].SetOp( opInt );
					mathStack[iOpLeftIndex].GetInt() = iResult;
					break;
				}

				case opBoolAnd:
				{
					if (!mathStack[iOpLeftIndex].IsInt() ||
						!mathStack[iOpRightIndex].IsInt())
					{
						return false;
					}

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					mathStack[iOpLeftIndex].GetInt() =
									(mathStack[iOpLeftIndex].GetInt() &&
										mathStack[iOpRightIndex].GetInt());
					break;
				}

				case opBoolOr:
				{
					if (!mathStack[iOpLeftIndex].IsInt() ||
						!mathStack[iOpRightIndex].IsInt())
					{
						return false;
					}

					mathStack[iOpLeftIndex].SetNext( mathStack[iOpRightIndex].GetNext() );
					mathStack[iOpLeftIndex].GetInt() =
									(mathStack[iOpLeftIndex].GetInt() ||
										mathStack[iOpRightIndex].GetInt());
					break;
				}

				default:
				{
					ErrMessage( "# Programming error!   *slap Bill or Coyote*" );
					return false;
				}
			}
		}
	}
}


bool TinTin::CompareValues( const MathOps& left, const MathOps& right,
							TTOperator op )
{
	bool	boolResult;

	if (left.IsString() || right.IsString())
	{
		ChString		strLeft;
		ChString		strRight;

		MakeString( left, strLeft );
		MakeString( right, strRight );

		switch( op )
		{
			case opCompGreater:
			{
				boolResult = (strLeft > strRight);
				break;
			}

			case opCompGreaterEqual:
			{
				boolResult = (strLeft >= strRight);
				break;
			}

			case opCompLess:
			{
				boolResult = (strLeft < strRight);
				break;
			}

			case opCompLessEqual:
			{
				boolResult = (strLeft <= strRight);
				break;
			}

			case opCompEqual:
			{
				boolResult = (strLeft == strRight);
				break;
			}

			case opCompNotEqual:
			{
				boolResult = (strLeft != strRight);
				break;
			}

			default:
			{
				ErrMessage( "# Programming error!   *slap Bill or Coyote*" );
				return false;
			}
		}
	}
	else
	{
		switch( op )
		{
			case opCompGreater:
			{
				boolResult = (left.GetInt() > right.GetInt());
				break;
			}

			case opCompGreaterEqual:
			{
				boolResult = (left.GetInt() >= right.GetInt());
				break;
			}

			case opCompLess:
			{
				boolResult = (left.GetInt() < right.GetInt());
				break;
			}

			case opCompLessEqual:
			{
				boolResult = (left.GetInt() <= right.GetInt());
				break;
			}

			case opCompEqual:
			{
				boolResult = (left.GetInt() == right.GetInt());
				break;
			}

			case opCompNotEqual:
			{
				boolResult = (left.GetInt() != right.GetInt());
				break;
			}

			default:
			{
				ErrMessage( "# Programming error!   *slap Bill or Coyote*" );
				return false;
			}
		}

	}

	return boolResult;
}


void TinTin::MakeString( const MathOps& val, ChString& strVal )
{
	if (val.IsString())
	{
		strVal = *(val.GetString());
	}
	else
	{
		strVal.Format( "%d", val.GetInt() );
	}
}

// $Log$
