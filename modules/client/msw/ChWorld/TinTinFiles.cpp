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

#include <fstream>

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define BUF_SIZE		2048


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	TinTin::DoRead
				Read a file of tintin commands.
----------------------------------------------------------------------------*/

void TinTin::DoRead( const ChString& strFilename, bool boolOnline )
{
	ChString		strFilePath;
	char		buffer[BUF_SIZE];
	std::ifstream	file;
	bool		boolFlag = true;
	bool		boolSaveOnline = IsOnline();

											// Set the 'online' state
	if (IsOnline() && !boolOnline)
	{										/* Only turn 'online' in 'off'
												direction */
		m_boolOnline = boolOnline;
	}

	GetArgInBraces( strFilename, strFilePath, true );
	if (strFilePath.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_NO_FILE_NAME, strMessage );
		Message( strMessage );
	}
	else
	{
		CompleteFilePath( strFilePath );

		file.open( strFilePath, std::ios::in );
		if (!file.is_open())
		{
			if (boolOnline)
			{
				ChString		strFormat;
				ChString		strMessage;

				LOADSTRING( IDS_TINTIN_FILE_OPEN_ERR, strFormat );
				strMessage.Format( strFormat, (const char*)strFilePath );
				ErrMessage( strMessage );
			}
		}
		else
		{
			int		iFirstChar;
			bool	boolDone;

			m_iAliasCounter = 0;
			m_iActionCounter = 0;
			m_iSubCounter = 0;
			m_iAntiSubCounter = 0;
			m_iVarCounter = 0;
			m_iHighlightCounter = 0;
			m_iPathdirCounter = 0;
											/* Set so only error messages will
												be displayed */
			SetMessages( false );

			if ((iFirstChar = file.peek()) != EOF)
			{
				ChString		strChar = (TCHAR)iFirstChar;

											// Set the first character
				DoChar( strChar );
			}

			boolDone = false;
			do {
				file.getline( buffer, BUF_SIZE );
				boolDone = file.eof();

				if (!boolDone)
				{
					ParseInput( buffer );
				}

			} while (!boolDone);

			SetMessages( true );

			if (boolOnline)
			{
				ChString		strTemp;

				if (m_iAliasCounter)
				{
					strTemp.Format( "# OK.  %d aliases loaded.",
										m_iAliasCounter );
					Message( strTemp );
				}

				if (m_iActionCounter)
				{
					strTemp.Format( "# OK.  %d actions loaded.",
										m_iActionCounter );
					Message( strTemp );
				}

				if (m_iAntiSubCounter)
				{
					strTemp.Format( "# OK.  %d antisubs loaded.",
										m_iAntiSubCounter );
					Message( strTemp );
				}

				if (m_iSubCounter)
				{
					strTemp.Format( "# OK.  %d substitutes loaded.",
										m_iSubCounter );
					Message( strTemp );
				}

				if (m_iVarCounter)
				{
					strTemp.Format( "# OK.  %d variables loaded.",
										m_iVarCounter );
					Message( strTemp );
				}

				if (m_iHighlightCounter)
				{
					strTemp.Format( "# OK.  %d highlights loaded.",
										m_iHighlightCounter );
					Message( strTemp );
				}

				if (m_iPathdirCounter)
				{
					strTemp.Format( "# OK.  %d path directions loaded.",
										m_iPathdirCounter );
					Message( strTemp );
				}
			}

			file.close();
		}
	}
											// Reset the 'online' state
	m_boolOnline = boolSaveOnline;
}


/*----------------------------------------------------------------------------
	TinTin::DoWrite
				Read a file of tintin commands.
----------------------------------------------------------------------------*/

void TinTin::DoWrite( const ChString& strFilename )
{
	ChString		strFilePath;
	std::ofstream	file;

	GetArgInBraces( strFilename, strFilePath, true );
	if (strFilePath.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_NO_FILE_NAME, strMessage );
		Message( strMessage );
	}
	else
	{
		CompleteFilePath( strFilePath );

		file.open( strFilePath );
		if (!file.is_open())
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_FILE_OPEN_ERR, strFormat );
			strMessage.Format( strFormat, (const char*)strFilePath );
			ErrMessage( strMessage );
		}
		else
		{
			ChString			strCommandLine;
			TinTinListNode*	pNode;
			ChString			strFormat;
			ChString			strMessage;
											// Start by turning off verbatim

			FormatCommand( "verbatim", "off", "", "", strCommandLine );
			file.write( strCommandLine, strCommandLine.GetLength() );

											// Now write out all commands
			pNode = GetListAliases()->GetTop();
			while (pNode)
			{
				FormatCommand( "alias", pNode->GetLeft(), pNode->GetRight(),
								"", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListActions()->GetTop();
			while (pNode)
			{
				FormatCommand( "action", pNode->GetLeft(), pNode->GetRight(),
								pNode->GetPriority(), strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListAntiSubs()->GetTop();
			while (pNode)
			{
				FormatCommand( "antisubstitute", pNode->GetLeft(),
								pNode->GetRight(), "", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListSubs()->GetTop();
			while (pNode)
			{
				FormatCommand( "substitute", pNode->GetLeft(),
								pNode->GetRight(), "", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListVars()->GetTop();
			while (pNode)
			{
				FormatCommand( "variable", pNode->GetLeft(),
								pNode->GetRight(), "", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListHighlights()->GetTop();
			while (pNode)
			{
				FormatCommand( "highlight", pNode->GetLeft(),
								pNode->GetRight(), "", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			pNode = GetListPathDirs()->GetTop();
			while (pNode)
			{
				FormatCommand( "pathdir", pNode->GetLeft(),
								pNode->GetRight(), "", strCommandLine );
				file.write( strCommandLine, strCommandLine.GetLength() );

				pNode = pNode->GetNext();
			}

			file.close();

			LOADSTRING( IDS_TINTIN_WRITE_OK, strFormat );
			strMessage.Format( strFormat, (const char*)strFilePath );
			Message( strMessage );
		}
	}
}


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

void TinTin::CompleteFilePath( ChString& strFilePath )
{
	int		iLastPart;
	ChString	strLastPart;

	if (-1 == strFilePath.Find( '\\' ))
	{										/* Get the file from the Pueblo
												directory by default */
		ChString		strAppDir;

		ChUtil::GetAppDirectory( strAppDir );
		strFilePath = strAppDir + strFilePath;
	}

	iLastPart = strFilePath.ReverseFind( '\\' );
	strLastPart = strFilePath.Mid( iLastPart + 1 );

	if (-1 == strLastPart.Find( '.' ))
	{										/* Add the '.txt' extension by
												default */
		strFilePath += ".txt";
	}
}


void TinTin::FormatCommand( const ChString& strCommand, const ChString& strLeft,
							const ChString& strRight, const ChString& strPriority,
							ChString& strResult )
{
	strResult = GetTinTinChar();
	strResult += strCommand;
	strResult += " {";
	strResult += strLeft;
	strResult += "}";

	if (!strRight.IsEmpty())
	{
		strResult += " {";
		strResult += strRight;
		strResult += "}";
	}

	if (!strPriority.IsEmpty())
	{
		strResult += " {";
		strResult += strPriority;
		strResult += "}";
	}

	strResult += "\n";
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:38  uecasm
// Import of source tree as at version 2.53 release.
//
