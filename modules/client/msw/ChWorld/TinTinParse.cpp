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

	TinTin class main & parsing methods.  Originally modified from TinTin++,
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

#include <ChCore.h>

#include "TinTin.h"
#include "World.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin protected methods
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	TinTin::ParseInput
				Checks for TinTin commands and aliases.
----------------------------------------------------------------------------*/

void TinTin::ParseInput( const ChString& strInput )
{
	ChString			strWorking( strInput );
	const char*		pstrTemp;
	const char*		pstrStart;
	const char*		pstrWorking = strWorking;
	TinTinListNode*	pNode;
	ChString			strCommand;
	const char*		pstrCommand;
	ChString			strArgs;

	if (++m_iRecurseLevel > MAX_RECURSE)
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_RECURSE_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else if (strWorking.IsEmpty())
	{										// Empty line
		Send( strWorking );
	}
	else
	{
		pstrTemp = GetArgStopAtSpaces( strWorking, strCommand );
		pstrStart = strCommand;

		if ((*pstrStart == GetTinTinChar()) &&
				IsAbrev( pstrStart + 1, "verbatim" ))
		{
											// Toggle verbatim status

			pstrTemp = GetArgInBraces( pstrTemp, strArgs, true );

			ToggleVerbatim( strArgs );
		}
		else if (IsVerbatim())
		{
			Send( strWorking );
		}
		else if (*pstrWorking == GetVerbatimChar())
		{									/* Line starts with verbatim
												escape, so send it 'as is'
												to the world */
			strWorking = ++pstrWorking;
			Send( strWorking );
		}
		else
		{
			SubstituteMyVars( strWorking, strWorking );

			pstrWorking = strWorking;
			while (*pstrWorking)
			{
				if (*pstrWorking == ';')
				{
					pstrWorking++;
				}

				pstrWorking = GetArgStopAtSpaces( pstrWorking, strCommand );
				pstrWorking = GetArgAll( pstrWorking, strArgs );

				pstrCommand = strCommand;

				if (*pstrCommand == GetTinTinChar())
				{
					ParseTinTinCommand( pstrCommand + 1, strArgs );
				}
				else if (pNode = GetListAliases()->SearchBegin( strCommand ))
				{
											// Alias found
					int			iLoop;
					const char*	pstrBeginArg;
					const char*	pstrEndArg;
					char		cEnd;
					ChString		strNewCommand;
											// Strip out the arguments
					m_strVars[0] = strArgs;
					for (iLoop = 1, pstrBeginArg = strArgs; iLoop < 10; iLoop++)
					{
						ChString		strTemp;
											// Find the beginning of the arg

						while (*pstrBeginArg == ' ')
						{
							pstrBeginArg++;
						}
											// Find the end of the argument

						cEnd = (*pstrBeginArg == '{') ? '}' : ' ';
						pstrBeginArg = (*pstrBeginArg == '{') ?
											pstrBeginArg + 1 : pstrBeginArg;
						for (pstrEndArg = pstrBeginArg;
								*pstrEndArg && (*pstrEndArg != cEnd); pstrEndArg++)
							;
											// Chop out the argument
						strTemp = pstrBeginArg;
						m_strVars[iLoop] = strTemp.Left( pstrEndArg - pstrBeginArg );

											// Move to the next argument

						pstrBeginArg = (*pstrEndArg) ? pstrEndArg + 1 : pstrEndArg;
					}

					PrepareActionAlias( pNode->GetRight(), strNewCommand );

					if(!strcmp( pNode->GetRight(), strNewCommand ) &&
						!strArgs.IsEmpty())
					{						// Append arguments
						strNewCommand += " ";
						strNewCommand += strArgs;
					}

					ParseInput( strNewCommand );
				}
				else if (IsSpeedwalk() && strArgs.IsEmpty() &&
							IsSpeedwalkDirective( strCommand ))
				{
					DoSpeedwalk( strCommand );
				}
				else
				{
					GetArgWithSpaces( strArgs, strArgs );
					Send( strCommand, strArgs );
				}
			}
		}
	}

	m_iRecurseLevel--;
}


void TinTin::Send( const ChString& strCmd )
{
	CheckInsertPath( strCmd );
	SendToWorld( strCmd );
}


void TinTin::Send( const ChString& strCmd, const ChString& strArgs )
{
	ChString	strTemp( strCmd );

	if (!strArgs.IsEmpty())
	{
		strTemp += ' ' + strArgs;
	}

	Send( strTemp );
}


/*----------------------------------------------------------------------------
	TinTin::ParseTinTinCommand
				Parses and processes most of the TinTin commands.
----------------------------------------------------------------------------*/

void TinTin::ParseTinTinCommand( const ChString& strCommand, ChString& strArgs )
{
	const char*			pstrCommand = strCommand;

	if (isdigit( *pstrCommand ))
	{
		int		iCount = atoi( pstrCommand );

		if (iCount > 0)
		{
			GetArgInBraces( strArgs, strArgs );

			while (iCount-- > 0)
			{								// Recurse to process the strCommand
				ParseInput( strArgs );
			}
		}
		else
		{
			ErrMessage( "# Yeah right!  Go repeat that yourself dude." );
		}
	}
	else
	{
		bool		boolProcessed = true;
		TTCommand	cmd;

		cmd = ParseTinTinCmd( strCommand );

		switch( cmd )
		{
			case ttAction:
			{
				DoAction( strArgs );
				break;
			}

			case ttAlert:
			{
				DoAlert();
				break;
			}

			case ttAlias:
			{
				DoAlias( strArgs );
				break;
			}

			case ttAll:
			{
				DoAll( strArgs );
				break;
			}

			case ttBell:
			{
				DoBell();
				break;
			}

			case ttChar:
			{
				DoChar( strArgs );
				break;
			}

			case ttCr:
			{
				DoCR();
				break;
			}

			case ttEcho:
			{
				ToggleEcho();
				break;
			}

			case ttEnd:
			{
				DoEnd( strCommand );
				break;
			}

			case ttGag:
			{
				if (strArgs[0] != DEF_OPEN)
				{
					ChString		strTemp( strArgs );

					strArgs = DEF_OPEN + strTemp + DEF_CLOSE;
				}
				
				strArgs += " .";

				DoSubstitute( strArgs );
				break;
			}

			case ttHelp:
			{
				DoHelp( strArgs );
				break;
			}

			case ttIf:
			{
				DoIf( strArgs );
				break;
			}

			case ttIgnore:
			{
				ToggleIgnore();
				break;
			}

			case ttInfo:
			{
				DoInfo();
				break;
			}

			case ttKillall:
			{
				DoKillAll( false );
				break;
			}

			case ttLog:
			{
				DoLog( strArgs );
				break;
			}

			case ttLoop:
			{
				DoLoop( strArgs );
				break;
			}

			case ttMap:
			{
				DoMap( strArgs );
				break;
			}

			case ttMark:
			{
				DoMark();
				break;
			}

			case ttMath:
			{
				DoMath( strArgs );
				break;
			}

			case ttMbox:
			{
				DoMbox( strArgs );
				break;
			}

			case ttMessage:
			{
				DoMessage( strArgs );
				break;
			}

			case ttName:
			{
				DoName( strArgs );
				break;
			}

			case ttNop:
			{
				break;
			}

			case ttPath:
			{
				DoPath();
				break;
			}

			case ttPathdir:
			{
				DoPathdir( strArgs );
				break;
			}

			case ttPlaySound:
			{
				DoPlaySound( strArgs );
				break;
			}

			case ttRandom:
			{
				DoRandom( strArgs );
				break;
			}

			case ttRead:
			{
				DoRead( strArgs, true );
				break;
			}

			case ttReturn:
			{
				DoReturn();
				break;
			}

			case ttSavepath:
			{
				DoSavepath( strArgs );
				break;
			}

			case ttSession:
			{
				DoSession( strArgs );
				break;
			}

			case ttShowme:
			{
				DoShowMe( strArgs );
				break;
			}

			case ttSpeedwalk:
			{
				ToggleSpeedwalk();
				break;
			}

			case ttSplit:
			{
				DoSplit( strArgs );
				break;
			}

			case ttSubstitute:
			{
				DoSubstitute( strArgs );
				break;
			}

			case ttTextin:
			{
				DoRead( strArgs, true );
				break;
			}

			case ttTick:
			{
				DoTick();
				break;
			}

			case ttTickkey:
			{
				DoTickKey( strArgs );
				break;
			}

			case ttTickoff:
			{
				DoTickOff();
				break;
			}

			case ttTickon:
			{
				DoTickOn();
				break;
			}

			case ttTickset:
			{
				DoTickSet();
				break;
			}

			case ttTicksize:
			{
				DoTickSize( strArgs );
				break;
			}

			case ttTogglesub:
			{
				ToggleSub();
				break;
			}

			case ttTolower:
			{
				DoChangeCase( strArgs, false );
				break;
			}

			case ttToupper:
			{
				DoChangeCase( strArgs, true );
				break;
			}

			case ttUnaction:
			{
				DoUnaction( strArgs );
				break;
			}

			case ttUnalias:
			{
				DoUnalias( strArgs );
				break;
			}

			case ttUngag:
			{
				DoUnsubstitute( strArgs, true );
				break;
			}

			case ttUnpath:
			{
				DoUnpath();
				break;
			}

			case ttUnsubstitute:
			{
				DoUnsubstitute( strArgs, false );
				break;
			}

			case ttUnvariable:
			{
				DoUnvariable( strArgs );
				break;
			}

			case ttVariable:
			{
				DoVariable( strArgs );
				break;
			}

			case ttVersion:
			{
				DoVersion();
				break;
			}

			case ttWizlist:
			{
				DoWizList();
				break;
			}

			case ttWrite:
			{
				DoWrite( strArgs );
				break;
			}

			case ttZap:
			{
				DoZap();
				break;
			}

			default:
			{
				boolProcessed = false;
				break;
			}
		}

		if (!boolProcessed)
		{
			ChWorldMainInfo*	pWorldMainInfo;
			ChCore*				pCore = GetMainInfo()->GetCore();

			if (pWorldMainInfo =
					(ChWorldMainInfo*)pCore->GetMainInfo( CH_MODULE_WORLD,
															strCommand ))
			{
				if (strArgs.IsEmpty())
				{							/* If there are no args, then just
												activate the frame */

					pCore->ActivateFrame( strCommand );
				}
				else
				{
					if (pWorldMainInfo->IsConnected())
					{
											/* If there are args, then send
												them to the frame */

						GetArgWithSpaces( strArgs, strArgs );
						pWorldMainInfo->GetTinTin()->ParseInput( strArgs );
					}
					else
					{
						ChString		strFormat;
						ChString		strMessage;

						LOADSTRING( IDS_TINTIN_SESSION_NOT_CONNECTED,
											strFormat );
						strMessage.Format( strFormat,
											(const char*)strCommand );
						ErrMessage( strMessage );
					}
				}
			}
			else
			{
				ChString		strFormat;
				ChString		strMessage;

				LOADSTRING( IDS_TINTIN_UNKNOWN_CMD, strFormat );
				strMessage.Format( strFormat, (const char*)strCommand );
				ErrMessage( strMessage );
			}
		}
	}

	#if 0

//	else if (IsAbrev( strCommand, "antisubstitute" ))
//	{
//		parse_antisub( strArgs );
//	}
//	else if (IsAbrev( strCommand, "boss" ))
//	{
//		boss_strCommand();
//	}
//	else if (IsAbrev( strCommand, "highlight" ))
//	{
//		parse_high( strArgs );
//	}
//	else if (IsAbrev( strCommand, "history" ))
//	{
//		history_strCommand();
//	}
//	else if (IsAbrev( strCommand, "presub" ))
//	{
//		presub_strCommand();
//	}
//	else if (IsAbrev( strCommand, "retab" ))
//	{
//		read_complete();
//	}
//	else if (IsAbrev( strCommand, "tablist" ))
//	{
//		tablist( complete_head );
//	}
//	else if (IsAbrev( strCommand, "tabadd" ))
//	{
//		tab_add( strArgs );
//	}
//	else if (IsAbrev( strCommand, "tabdelete" ))
//	{
//		tab_delete(strArgs);
//	}
//	else if (IsAbrev( strCommand, "togglesubs" ))
//	{
//		togglesubs_strCommand();
//	}
//	else if (IsAbrev( strCommand, "unantisubstitute" ))
//	{
//		unantisubstitute_strCommand( strArgs );
//	}
//	else if (IsAbrev( strCommand, "unhighlight" ))
//	{
//		unhighlight_strCommand( strArgs );
//	}

	#endif
}


/*----------------------------------------------------------------------------
	TinTin::IsSpeedwalkDirective
				Returns true if command consists only of the letters
				[n,s,e,w,u,d] or digits followed by those letters.
----------------------------------------------------------------------------*/

bool TinTin::IsSpeedwalkDirective( const char* pstrCmd )
{
	bool	boolSpeedWalk = false;

	while (*pstrCmd)
	{
		if ((*pstrCmd != 'n') && (*pstrCmd != 'e') && (*pstrCmd != 's') &&
			(*pstrCmd != 'w') && (*pstrCmd != 'u') && (*pstrCmd != 'd') &&
			!isdigit( *pstrCmd ))
		{
			return false;
		}

		if (!isdigit( *pstrCmd ))
		{
			boolSpeedWalk = TRUE;
		}

		pstrCmd++;
	}

	return boolSpeedWalk;
}


/*----------------------------------------------------------------------------
	TinTin::DoSpeedwalk
				This function will interpret and perform a speedwalk
				command.
----------------------------------------------------------------------------*/

void TinTin::DoSpeedwalk( const char* pstrCmd )
{
	while (*pstrCmd)
	{
		const char*	pstrStart = pstrCmd;
		bool		boolMultiple = false;
		char		cDirection;
		int			iCount;

		while (isdigit( *pstrCmd ))
		{
			pstrCmd++;
			boolMultiple = true;
		}

		if (boolMultiple && *pstrCmd)
		{
			if (2 == sscanf( pstrStart, "%d%c", &iCount, &cDirection ))
			{
				for (int iLoop = 0; iLoop < iCount; iLoop++)
				{
					Send( cDirection );
				}
			}
		}
		else if (*pstrCmd)
		{
			Send( *pstrCmd );
		}

		if (*pstrCmd)
		{
			pstrCmd++;
		}
	}
}


/*----------------------------------------------------------------------------
	TinTin::GetArgStopAtSpaces
				Get all arguments - don't remove double-quotes or
				back-slashes.
----------------------------------------------------------------------------*/

const char* TinTin::GetArgAll( const char* pstrText, ChString& strArg )
{
	int		iNestLevel = 0;

	strArg = "";
	pstrText = SpaceOut( pstrText );

	while (*pstrText)
	{
		if (*pstrText == '\\')
		{									/* Next character is quoted,
												so leave it in to be
												processed later */
			strArg += *pstrText++;
			if (*pstrText)
			{
				strArg += *pstrText++;
			}
		}
		else if ((*pstrText == ';') && (0 == iNestLevel))
		{
											// Semicolon ends the command...
			break;
		}
		else if (*pstrText == DEF_OPEN)
		{									// Open argument
			iNestLevel++;
			strArg += *pstrText++;
		}
		else if (*pstrText == DEF_CLOSE)
		{									// Close argument
			if (0 > --iNestLevel)
			{
				iNestLevel = 0;
			}
			strArg += *pstrText++;
		}
		else
		{									// Copy the character
			strArg += *pstrText++;
		}
	}

	return pstrText;
}


/*----------------------------------------------------------------------------
	TinTin::GetArgInBraces
				Get the argument.  If the argument is bracketed with
				curly braces, just get that part.
----------------------------------------------------------------------------*/

const char* TinTin::GetArgInBraces( const char* pstrText, ChString& strArg,
									bool boolIncludeSpaces )
{
	int			iNestLevel = 0;
	const char*	pstrTemp;
	ChString		strResult;

	pstrText = SpaceOut( pstrText );

	pstrTemp = pstrText;

	if (*pstrText != DEF_OPEN)
	{										// This isn't bracketed in braces
		if (boolIncludeSpaces)
		{
			pstrText = GetArgWithSpaces( pstrText, strArg );
		}
		else
		{
			pstrText = GetArgStopAtSpaces( pstrText, strArg );
		}

		return pstrText;
	}

	pstrText++;

	while (*pstrText && !((*pstrText == DEF_CLOSE) && (iNestLevel == 0)))
	{
		if (*pstrText == DEF_OPEN)
		{
			iNestLevel++;
		}
		else if (*pstrText == DEF_CLOSE)
		{
			iNestLevel--;
		}

		strResult += *pstrText++;
	}

	strArg = strResult;

	if (!*pstrText)
	{										/* There should be a '}' still
												in the buffer */

		ErrMessage( "# Unmatched braces error!" );
	}
	else
	{										// Skip the close brace
		pstrText++;
	}

	return pstrText;
}


/*----------------------------------------------------------------------------
	TinTin::GetArgStopAtSpaces
				Gets a single argument, stopping at spaces and removing
				quotes.
----------------------------------------------------------------------------*/

const char* TinTin::GetArgStopAtSpaces( const char* pstrText, ChString& strArg )
{
	strArg = "";
	pstrText = SpaceOut( pstrText );

	while (*pstrText)
	{
		if (*pstrText == '\\')
		{									// Next character is quoted
			if (*++pstrText)
			{
				strArg += *pstrText++;
			}
		}
		else if (*pstrText == ';')
		{
											// Semicolon ends the command...
			break;
		}
		else if (*pstrText == ' ')
		{									// End at a space...
			break;
		}
		else
		{									// Copy the character
			strArg += *pstrText++;
		}
	}

	return pstrText;
}


/*----------------------------------------------------------------------------
	TinTin::GetArgWithSpaces
				Gets the entire argument, including spaces,
				removing quotes.

				For example:
					In:		"this is it" way way hmmm;
					Out:	this is it way way hmmm
----------------------------------------------------------------------------*/

const char* TinTin::GetArgWithSpaces( const char* pstrText, ChString& strArg )
{
	int			iNestLevel = 0;
	ChString		strTemp;

	pstrText = SpaceOut( pstrText );

	while (*pstrText)
	{
		if (*pstrText == '\\')
		{
			if (*++pstrText)
			{
				strTemp += *pstrText++;
			}
			else
			{
				strTemp += '\\';
			}
		}
		else if ((*pstrText == ';') && (0 == iNestLevel))
		{
			break;
		}
		else if (*pstrText == DEF_OPEN)
		{
			iNestLevel++;
			strTemp += *pstrText++;
		}
		else if(*pstrText == DEF_CLOSE)
		{
			strTemp += *pstrText++;
			iNestLevel--;
		}
		else
		{
			strTemp += *pstrText++;
		}
	}

	strArg = strTemp;
	return pstrText;
}


/*----------------------------------------------------------------------------
	TinTin::SpaceOut

			Advances point to next non-space.
----------------------------------------------------------------------------*/

const char* TinTin::SpaceOut( const char* pstrText )
{
	while (isspace( *pstrText ))
	{
		pstrText++;
	}

	return pstrText;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:40  uecasm
// Import of source tree as at version 2.53 release.
//
