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

#include <ChTypes.h>

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class protected methods
----------------------------------------------------------------------------*/

											/* If you change this array must
												also change the TTCommand
												enumeration to correspond
												with the indices in this
												array */
                      
const char* TinTin::m_pstrCommands[] =
						{	"action",
							"alert",
							"alias",
							"all",
							"antisubstitute",
							"bell",
							"boss",
							"char",
							"cr",
							"echo",
							"end",
							"gag",
							"help",
							"highlight",
							"history",
							"if",
							"ignore",
							"info",
							"killall",
							"log",
							"loop",
							"map",
							"math",
							"mark",
							"mbox",
							"message",
							"name",
							"nop",
							"path",
							"pathdir",
							"playsound",
							"presub",
							"random",
							"redraw",
							"retab",
							"return",
							"read",
							"savepath",
							"session",
							"showme",
							"speedwalk",
							"split",
							"substitute",
							"textin",
							"tick",
							"tickkey",
							"tickoff",
							"tickon",
							"tickset",
							"ticksize",
							"tolower",
							"togglesub",
							"toupper",
							"unaction",
							"unalias",
							"unantisubstitute",
							"unhighlight",
							"unsubstitute",
							"ungag",
							"unpath",
							"unvariable",
							"variable",
							"version",
							"wizlist",
							"write",
							"writesession",
							"zap",
											/* The following are at the end
												for a reason -- They are only
												here for help file access. */
							"verbatim",
							"enhancements",
							"expressions",
							0 };


/*----------------------------------------------------------------------------
	TinTin::ParseTinTinCmd
				Returns the TTCommand value corresponding to the passed
				string.  Abbreviations are matched.
----------------------------------------------------------------------------*/

TTCommand TinTin::ParseTinTinCmd( const char* strCommand )
{
	const char**	ppstrThisCommand = m_pstrCommands;
	TTCommand		foundCommand = ttUnknown;
	int				iLoop = 0;

	while (*ppstrThisCommand && (ttUnknown == foundCommand))
	{
		if (IsAbrev( strCommand, *ppstrThisCommand ))
		{
			foundCommand = (TTCommand)iLoop;
		}

		iLoop++;
		ppstrThisCommand++;
	}

	return foundCommand;
}


/*----------------------------------------------------------------------------
	TinTin::IsAbrev
				Returns true if pstr1 is an abbreviation of pstr2.
----------------------------------------------------------------------------*/

bool TinTin::IsAbrev( const char* pstr1, const char* pstr2 )
{
	return( !strncmp( pstr2, pstr1, strlen( pstr1 ) ) );
}


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	::Match
				Lightweight regular expression matching.  Only matches '*'
				and understands backslashes for escaping.
----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( bool )
Match( const char* pstrRegex, const char* pstrTest )
{
	while (*pstrRegex != 0)
	{
		char		cRegex;

		switch (cRegex = *pstrRegex++)
		{
			case '*':
			{
				const char*	pstrSave;
											/* Match empty strTest at end
												of `strTest' */
				if (0 == *pstrTest)
				{							/* ... but only if we're done
													with the pattern */
					return (0 == *pstrRegex);
				}
											/* Greedy algorithm: save starting
												location, then find end of
												strTest */
				pstrSave = pstrTest;
				pstrTest += strlen( pstrTest );

				do
				{
					if (Match( pstrRegex, pstrTest ))
					{
											// Return success if we match here
						return true;
					}
											// Otherwise back up and try again

				} while (--pstrTest >= pstrSave);

											/* Backed up all the way to the
												starting location (i.e. `*'
												matches empty strTest) and
												we _still_ can't match here.
												Give up. */
				return false;
			}

			case '\\':
			{
				if ((cRegex = *pstrRegex++) != '\0')
				{
											/* If not end of pattern, match
												next char explicitly */
					if (cRegex != *pstrTest++)
					{
						return false;
					}
					break;
				}
				
				/* else FALL THROUGH to match a backslash */
			}

			default:
			{								// Normal character
				if (cRegex != *pstrTest++)
				{
					return false;
				}
				break;
			}
		}
	}
											/* OK, we successfully matched the
												pattern if we got here.  Now
												return a match if we also
												reached end of strTest,
												otherwise failure */
	return (0 == *pstrTest);
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:42  uecasm
// Import of source tree as at version 2.53 release.
//
