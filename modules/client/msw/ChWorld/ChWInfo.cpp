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

	Contains the implementation of the ChWorldInfo class.

----------------------------------------------------------------------------*/

// $Header$


#include "headers.h"

#include <fstream>
#include <ChCore.h>

#include "ChSCWiz.h"
#include "ChWInfo.h"

#if defined( CH_UNIX )

	#include <stdlib.h>

#elif defined( CH_MSW )

	#include <ddeml.h>

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#endif	// defined( CH_MSW )

#include "MemDebug.h"


/*----------------------------------------------------------------------------
	ChWorldInfo constants
----------------------------------------------------------------------------*/

#define NAME_KEY			"Name"
#define HOST_KEY			"Host"
#define ADDR_KEY			"Addr"
#define PORT_KEY			"Port"
#define TYPE_KEY			"Type"
#define LOGIN_KEY			"Login"
#define LOGIN_KEY_CONNECT		"connect"
#define DESC_KEY			"Desc"
#define USERNAME_KEY		"User"
#define PASSWORD_KEY		"Password"
#define HOMEPAGE_KEY		"HomePage"

#define ADDR_LEN			11				// Length of nn.nn.nn.nn

#define PROGRAM_PATH_LEN	256
#define SHORTCUT_ICON_IDX	1


#define COMMENT_PREFIX		'#'
#define TAG_DELIMITER		'='

#define TAG_SERVER			"server"
#define TAG_PORT			"port"
#define TAG_USERNAME		"username"
#define TAG_PASSWORD		"password"
#define TAG_WORLDNAME		"worldname"
#define TAG_WORLDSERVER		"worldserver"
#define TAG_WORLDPORT		"worldport"
#define TAG_WORLDPORT_DEF		4201
#define TAG_WORLDTYPE		"worldtype"
#define TAG_LOGINTYPE		"logintype"
#define TAG_LOGINTYPE_CONNECT	"connect"
#define TAG_WORLDUSERNAME	"worldusername"
#define TAG_WORLDPASSWORD	"worldpassword"

#if defined( CH_PUEBLO_PLUGIN )
#define TAG_WORLD_LIST		"worldlist"
#define TAG_WORLD_DISCONNECT "worldquit"
#endif




/*----------------------------------------------------------------------------
	ChWorldInfo class
----------------------------------------------------------------------------*/

ChWorldInfo::ChWorldInfo( const ChString& strCommand ) :
			m_boolValid( false ),
			m_type( otherType ),
			m_loginType( variableLogin ),
			m_sPort( 0 )
{
	ChString		strTemp = strCommand;

	while (GetKey( strTemp ))
	{
	}
											/* If we still haven't read in an
												explicit login type, then
												try to figure out the login
												type from the server type */
	if (variableLogin == m_loginType)
	{
		m_loginType = m_type.GetLoginType();
	}
											/* If we still don't know, then
												try username/password */
	if (variableLogin == m_loginType)
	{
		m_loginType = unamePwLogin;
	}
}


ChWorldInfo::ChWorldInfo(  const ChString& strName, const ChString& strDesc,
							const ChString& strHost, chint16 sPort,
							const ChWorldType& type, ChLoginType login,
							const ChString& strUsername,
							const ChString& strPassword,
							const ChString& strHomePage ) :
			m_strName( strName ),
			m_strDesc( strDesc ),
			m_type( type ),
			m_loginType( login ),
			m_strHost( strHost ),
			m_sPort( sPort ),
			m_strUsername( strUsername ),
			m_strPassword( strPassword ),
			m_strHomePage( strHomePage )
{
	Validate();
}


ChWorldInfo::ChWorldInfo(  const ChString& strName, const ChString& strDesc,
						const ChString& strHost, const ChString& strAddr,
						chint16 sPort, const ChWorldType& type,
						ChLoginType login, const ChString& strUsername,
						const ChString& strPassword,
						const ChString& strHomePage ) :
			m_strName( strName ),
			m_strDesc( strDesc ),
			m_type( type ),
			m_loginType( login ),
			m_strHost( strHost ),
			m_strAddr( strAddr ),
			m_sPort( sPort ),
			m_strUsername( strUsername ),
			m_strPassword( strPassword ),
			m_strHomePage( strPassword )
{
	Validate();
}

ChWorldInfo::ChWorldInfo( std::ifstream& ifile ) :

			m_boolValid( false ),
			m_type( otherType ),
			m_loginType( variableLogin ),
			m_sPort( 0 )
{

   	// Read the file and set all the variables
	if (ifile.is_open())
	{

		if ( FindTag( ifile, TAG_WORLDSERVER, m_strHost ) && !m_strHost.IsEmpty() )
		{
			if (FindTag( ifile, TAG_WORLDPORT, m_sPort ) && (0 == m_sPort))
			{
				m_sPort = TAG_WORLDPORT_DEF;
			}

			FindTag( ifile, TAG_WORLDNAME, m_strName );
			FindTag( ifile, TAG_WORLDUSERNAME, m_strUsername );
			FindTag( ifile, TAG_WORLDPASSWORD, m_strPassword );

#if defined( CH_PUEBLO_PLUGIN )
			FindTag( ifile, TAG_WORLD_LIST, m_strHomePage );
			FindTag( ifile, TAG_WORLD_DISCONNECT, m_strOnDisconnect );
#endif

			ChString	strType;
			if (FindTag( ifile, TAG_WORLDTYPE, strType ))
			{
				m_type.Set( strType );
			}
			else
			{
				m_type.Set( otherType );
			}

			ChString		strLoginType;
			m_loginType = variableLogin;
			if (FindTag( ifile, TAG_LOGINTYPE, strLoginType ))
			{
				if (strLoginType.CompareNoCase( TAG_LOGINTYPE_CONNECT ))
				{
					m_loginType = m_type.GetLoginType();
				}
				else
				{
					m_loginType = connectLogin;
				}
			}

			if (variableLogin == m_loginType)
			{							// Default to un/pw login

				m_loginType = unamePwLogin;
			}
		}
	}

	if (variableLogin == m_loginType)
	{
		m_loginType = m_type.GetLoginType();
	}
											/* If we still don't know, then
												try username/password */
	if (variableLogin == m_loginType)
	{
		m_loginType = unamePwLogin;
	}

	Validate();
}



void ChWorldInfo::Stringize( ChString& strWorld )
{
	ChString		strTemp;

	if (GetName().GetLength())
	{
		strTemp = GetName();
		Escape( strTemp );

		strWorld = "/" NAME_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetHost().GetLength())
	{
		strTemp = GetHost();
		Escape( strTemp );

		strWorld += "/" HOST_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (m_strAddr.GetLength())
	{
		strTemp = m_strAddr;
		Escape( strTemp );

		strWorld += "/" ADDR_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetPort() > 0)
	{
		char	buffer[10];

		sprintf( buffer, "%d", (int)GetPort() );
		strTemp = buffer;
		Escape( strTemp );

		strWorld += "/" PORT_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetType() != otherType)
	{
		strTemp = GetType().GetName();
		Escape( strTemp );

		strWorld += "/" TYPE_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetLoginType() == connectLogin)
	{
		strTemp = LOGIN_KEY_CONNECT;
		Escape( strTemp );

		strWorld += "/" LOGIN_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetUsername().GetLength())
	{
		strTemp = GetUsername();
		Escape( strTemp );

		strWorld += "/" USERNAME_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetPassword().GetLength())
	{
		ChString		strPassword = GetPassword();

		ChUtil::EncryptString( strPassword, true, '/' );
		strTemp = strPassword;
		Escape( strTemp );

		strWorld += "/" PASSWORD_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetHomePage().GetLength())
	{
		strTemp = GetHomePage();
		Escape( strTemp );

		strWorld += "/" HOMEPAGE_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}

	if (GetDesc().GetLength())
	{
		strTemp = GetDesc();
		Escape( strTemp );

		strWorld += "/" DESC_KEY "={";
		strWorld += strTemp;
		strWorld += "}";
	}
}


void ChWorldInfo::CreateShortcut( ChCore* pCore  )
{
	#if defined( CH_MSW )
	{
		CreateWindowsShortcut( pCore );
	}
	#else
	{
		TRACE( "Don't know how to create shortcut for this platform!\n" );
	}
	#endif	// defined( CH_MSW )
}


void ChWorldInfo::Set( const ChString& strDesc, const ChString& strHost,
						chint16 sPort, const ChWorldType& type,
						ChLoginType login, const ChString& strUsername,
						const ChString& strPassword, const ChString& strHomePage )
{
	ASSERT( sPort != 0 );

	m_strDesc = strDesc;
	m_strHost = strHost;
	m_sPort = sPort;
	m_type = type;
	m_loginType = login;
	m_strUsername = strUsername;
	m_strPassword = strPassword;
	m_strHomePage = strHomePage;
}


void ChWorldInfo::Set( const ChString& strDesc, const ChString& strHost,
						const ChString& strAddr, chint16 sPort,
						const ChWorldType& type, ChLoginType login,
						const ChString& strUsername, const ChString& strPassword,
						const ChString& strHomePage )
{
	Set( strDesc, strHost, sPort, type, login, strUsername, strPassword,
			strHomePage );

	if (strAddr.GetLength() == ADDR_LEN)
	{
		m_strAddr = strAddr;
	}
}


/*----------------------------------------------------------------------------
	ChWorldInfo protected methods
----------------------------------------------------------------------------*/

bool ChWorldInfo::GetKey( ChString& strCommand )
{
	if (0 == strCommand.GetLength())
	{
		return false;
	}

	const char*	pstrCommand = strCommand;
	ChString		strKey;
	ChString		strValue;

	while (*pstrCommand && (*pstrCommand != '/'))
	{
		pstrCommand++;
	}

	if (*pstrCommand == '/')
	{
		pstrCommand++;
											/* We found a slash, now find the
												equal sign */

		while (*pstrCommand && (*pstrCommand != '='))
		{
			strKey += *pstrCommand;
			pstrCommand++;
		}

		if (*pstrCommand == '=')
		{
			bool	boolQuote;
			bool	boolFoundEnd = false;

			pstrCommand++;
											/* We found the open brace, now
												find the end of the value */

			if (boolQuote = (*pstrCommand == '{'))
			{
											// Value is bracketted by quotes
				pstrCommand++;
			}

			while (*pstrCommand && !boolFoundEnd)
			{
				if (boolQuote && (*pstrCommand == '}'))
				{
					pstrCommand++;
					boolFoundEnd = true;
				}
				else if (!boolQuote && (*pstrCommand == '/'))
				{
					boolFoundEnd = true;
				}
				else
				{
					if (boolQuote && (*pstrCommand == '\\'))
					{
											// Unescape next character
						pstrCommand++;
					}

					if (*pstrCommand)
					{
						strValue += *pstrCommand;
						pstrCommand++;
					}
				}
			}

			if (strValue.GetLength())
			{								// Process the tag and value
				ProcessKey( strKey, strValue );
			}
		}
	}
											/* Strip what we just processed
												off of this string */
	if (*pstrCommand)
	{
		strCommand = strCommand.Mid( pstrCommand -
										(const char*)strCommand );
	}
	else
	{
		strCommand = "";
	}

	return true;
}

void ChWorldInfo::ProcessKey( const ChString& strKey, const ChString& strValue )
{
	if (0 == strKey.CompareNoCase( HOST_KEY ))
	{										// Host key found
		if (strValue.GetLength())
		{
			m_strHost = strValue;
		}
	}
	else if (0 == strKey.CompareNoCase( ADDR_KEY ))
	{
											// Address key found

		if (strValue.GetLength() == ADDR_LEN)
		{
			m_strAddr = strValue;
		}
	}
	else if (0 == strKey.CompareNoCase( PORT_KEY ))
	{
											// Port key found
		m_sPort = atoi( strValue );
	}
	else if (0 == strKey.CompareNoCase( TYPE_KEY ))
	{
											// Type key found
		m_type.Set( strValue );
	}
	else if (0 == strKey.CompareNoCase( LOGIN_KEY ))
	{
		if (strValue == LOGIN_KEY_CONNECT)
		{
			m_loginType = connectLogin;
		}
	}
	else if (0 == strKey.CompareNoCase( NAME_KEY ))
	{
											// Name key found
		m_strName = strValue;
		RemoveIllegalChars( m_strName );
	}
	else if (0 == strKey.CompareNoCase( DESC_KEY ))
	{
											// Type key found
		m_strDesc = strValue;
	}
	else if (0 == strKey.CompareNoCase( USERNAME_KEY ))
	{
											// Username key found
		m_strUsername = strValue;
		RemoveIllegalChars( m_strUsername );
	}
	else if (0 == strKey.CompareNoCase( PASSWORD_KEY ))
	{
		ChString	strTemp( strValue );
											// Password key found

		ChUtil::EncryptString( strTemp, false, '/' );
		m_strPassword = strTemp;
	}
	else if (0 == strKey.CompareNoCase( HOMEPAGE_KEY ))
	{
											// Username key found
		m_strHomePage = strValue;
	}

	Validate();
}


void ChWorldInfo::Validate()
{
	if (m_strHost.GetLength() && m_sPort != 0)
	{
		m_boolValid = true;
	}
	else
	{
		m_boolValid = false;
	}
}


void ChWorldInfo::Escape( ChString& strValue )
{
	ChString		strTemp;
	const char*	pstrValue = strValue;

	while (*pstrValue)
	{
		if ((*pstrValue == '{') || (*pstrValue == '}') ||
			(*pstrValue == '\\') || (*pstrValue == '"'))
		{
			strTemp += '\\';
		}

		strTemp += *pstrValue;
		pstrValue++;
	}

	strValue = strTemp;
}


void ChWorldInfo::RemoveIllegalChars( ChString& strValue )
{
	ChString	strText;
	int		iLoc;

	while (-1 != (iLoc = strValue.Find( WORLD_NAME_SEPARATOR )))
	{
		strValue = strValue.Left( iLoc ) + strValue.Mid( iLoc + 1 );
	}
}


#if defined( CH_MSW )

void ChWorldInfo::CreateWindowsShortcut( ChCore* pCore )
{
											/* First save the information to
												a file */
	ChString				strPath;
	ChString				strShortcutDir;
	ChString				strFilename;
	bool				boolDone = false;
	ChShortcutWizard	shortcutWizard;
	int					iResult;
	bool				boolSuccess;

	LOADSTRING( IDS_SHORTCUT_DIR, strShortcutDir );
	ChUtil::GetAppDirectory( strPath );
	strPath += strShortcutDir;

	shortcutWizard.SetPath( strPath );
	shortcutWizard.SetName( GetName() );
	shortcutWizard.SetAccount( GetUsername(), GetPassword(), GetLoginType() );
	shortcutWizard.SetServer( GetHost(), GetPort(), GetType() );

	if (!GetHost().IsEmpty() && (GetPort() != 0) &&
		(GetType() != undefinedType))
	{										/* We have all the server
												information we need */
		shortcutWizard.SetUseCurrWorld();
	}

											// Present the dialog
	iResult = shortcutWizard.DoModal();
	switch( iResult )
	{
		case IDFINISH:
		{
			boolSuccess = true;
			break;
		}

		case IDCANCEL:
		case IDABORT:
		{
			boolSuccess = false;
			break;
		}
	}

	if (boolSuccess)
	{
		ChString		strPath;
		ChString		strFilePath;
		ChString		strGroupName;
		ChString		strName;
		ChString		strUsername;
		ChString		strPassword;
		ChString		strHost;
		chint16		sPort;
		ChWorldType	type;
		ChLoginType	loginType;

		shortcutWizard.GetData( strPath, strFilePath, strGroupName, strName );
		shortcutWizard.GetAccount( strUsername, strPassword, loginType );
		shortcutWizard.GetServer( strHost, sPort, type );

		boolSuccess = WriteWindowsShortcutFile( pCore, strPath, strFilePath,
												strGroupName, strName,
												strUsername, strPassword,
												strHost, "", sPort, type,
												loginType );
	}
}


bool ChWorldInfo::WriteWindowsShortcutFile( ChCore* pCore, const ChString& strPath,
											const ChString& strFilePath,
											const ChString& strGroupName,
											const ChString& strName,
											const ChString& strUsername,
											const ChString& strPassword,
											const ChString& strHost,
											const ChString& strHomePage,
											chint16 sPort,
											const ChWorldType& type,
											ChLoginType loginType )
{
	bool	boolSuccess = true;

	ChUtil::CreateDirectoryTree( strPath );

	try
	{
		ChString		strTemp( "Pueblo user" );
		bool		boolWritten = false;
		CStdioFile	shortcut( strFilePath, CFile::modeCreate |
											CFile::modeWrite |
											CFile::shareExclusive );

	//	strTemp = pCore->GetPuebloUsername();
		if (!strTemp.IsEmpty())
		{
			shortcut.WriteString( "username            = " );
			shortcut.WriteString( strTemp );
			shortcut.WriteString( "\n" );

			boolWritten = true;
		}

		if (boolWritten)
		{
			shortcut.WriteString( "\n" );
		}

		if (!GetName().IsEmpty())
		{
			shortcut.WriteString( "worldname           = " );
			shortcut.WriteString( GetName() );
			shortcut.WriteString( "\n" );
		}

		if (!strHost.IsEmpty())
		{
			shortcut.WriteString( "worldserver         = " );
			shortcut.WriteString( strHost );
			shortcut.WriteString( "\n" );
		}

		if (0 != sPort)
		{
			char	strPort[10];

			sprintf( strPort, "%d", (int)sPort );

			shortcut.WriteString( "worldport           = " );
			shortcut.WriteString( strPort );
			shortcut.WriteString( "\n" );
		}

		if (undefinedType != type)
		{
			strTemp = type.GetName();

			shortcut.WriteString( "worldtype           = " );
			shortcut.WriteString( strTemp );
			shortcut.WriteString( "\n" );
		}

		if (connectLogin == loginType)
		{
			shortcut.WriteString( "logintype           = connect\n" );
		}

		if (!strUsername.IsEmpty())
		{
			shortcut.WriteString( "worldusername       = " );
			shortcut.WriteString( strUsername );
			shortcut.WriteString( "\n" );
		}

		if (!strPassword.IsEmpty())
		{
			strTemp = strPassword;
			ChUtil::EncryptString( strTemp, true );

			shortcut.WriteString( "worldpassword       = " );
			shortcut.WriteString( strTemp );
			shortcut.WriteString( "\n" );
		}

		if (!strHomePage.IsEmpty())
		{
			shortcut.WriteString( "worldhomepage       = " );
			shortcut.WriteString( strHomePage );
			shortcut.WriteString( "\n" );
		}
	}
	catch( CFileException* pFileExcept )
	{
		CWnd*	pFrame = pCore->GetFrameWnd();
		ChString	strErr;
		ChString	strCaption;

		LOADSTRING( IDS_FILEERR_CAPTION, strCaption );

		switch( pFileExcept->m_cause )
		{
			case CFileException::sharingViolation:
			{
				LOADSTRING( IDS_FILEERR_SHARING, strErr );
				break;
			}

			case CFileException::diskFull:
			{
				LOADSTRING( IDS_FILEERR_DISK_FULL, strErr );
				break;
			}

			default:
			{
				ChString		strFormat;

				LOADSTRING( IDS_FILEERR_DISK, strFormat );
				strErr.Format( strFormat, (int)pFileExcept->m_cause,
											(const char*)strFilePath );
				break;
			}
		}

		pFrame->MessageBox( strErr, strCaption, MB_OK | MB_ICONSTOP );

											// Delete the exception
		pFileExcept->Delete();
		boolSuccess = false;
	}

	if (boolSuccess)
	{
		CreateProgmanIcon( strFilePath, strGroupName, strName );
	}

	return boolSuccess;
}


void ChWorldInfo::CreateProgmanIcon( const ChString& strFilePath,
										const ChString& strGroupName,
										const ChString& strName )
{
	DWORD		idDDEInst = ChCore::GetDDEInstance();
	HCONV		hDDEConversation;
	HSZ			hszShell;
	ChString		strRequest;
	bool		boolSuccess;
    DWORD		dwResult;
	char*		pstrBuffer = new char[1000];
	char*		pstrProgram = new char[PROGRAM_PATH_LEN];
	int			iLen;

	ASSERT( pstrBuffer );
	ASSERT( pstrProgram );

	GetModuleFileName( 0, pstrProgram, PROGRAM_PATH_LEN );

											/* Get a list of program manager
												groups (via DDE) and add them
												to the combo box */

	hszShell = DdeCreateStringHandle( idDDEInst, PROGMAN_SERVICE, CP_WINANSI );

											// Initiate a conversation

	hDDEConversation = DdeConnect( idDDEInst, hszShell, hszShell, 0 );

											/* Send the request to the server
												to create (and/or activate)
												the group */

	sprintf( pstrBuffer, "[CreateGroup(%s)]", (const char*)strGroupName );
	iLen = strlen( pstrBuffer );
	boolSuccess = !!DdeClientTransaction( (LPBYTE)pstrBuffer, iLen + 1,
											hDDEConversation, 0, CF_TEXT,
											XTYP_EXECUTE, 1000, &dwResult );

	if (boolSuccess)
	{										/* Send the request to the server
												to add the item */

		sprintf( pstrBuffer, "[AddItem(\"%s\",%s,%s,%d)]",
								(const char*)strFilePath, (const char*)strName,
								pstrProgram, SHORTCUT_ICON_IDX );
		iLen = strlen( pstrBuffer );
		boolSuccess = !!DdeClientTransaction( (LPBYTE)pstrBuffer, iLen + 1,
												hDDEConversation, 0, CF_TEXT,
												XTYP_EXECUTE, 1000, &dwResult );
	}
											// Clean up from DDE
	DdeFreeStringHandle( idDDEInst, hszShell );
	delete [] pstrBuffer;
	delete [] pstrProgram;
}

/////////////////////////////////////////
///  Methodfs to read the shortcut file



bool ChWorldInfo::FindTag( std::ifstream& ifile, const char* pstrTag, ChString& strVal )
{
	bool		boolFound = false;
	char		buffer[256];

	ifile.clear(std::ios::goodbit);
	ifile.seekg( 0 ); // start from the top of the file

	while (!ifile.eof() && !boolFound)
	{
		char*	pTagStart;
											// Read the next line...
		ifile.getline( buffer, sizeof( buffer ) );

											// Eliminate white space...
		pTagStart = buffer;
		while (*pTagStart && isspace( *pTagStart ))
		{
			pTagStart++;
		}

		if ((0 != *pTagStart) && (COMMENT_PREFIX != *pTagStart))
		{
											// Look for an delimiter...
			char*	pDelim;

			if (pDelim = strchr( pTagStart, TAG_DELIMITER ))
			{
				char*	pLastChar;
											/* Separate the tag string from
												the value */
				*pDelim = 0;
				pLastChar = pDelim - 1;
				while ((pLastChar != pTagStart) && isspace( *pLastChar ))
				{
					*pLastChar = 0;
					--pLastChar;
				}

				if (0 == stricmp( pstrTag, pTagStart ))
				{
											// Yay!  We found it!
					strVal = ++pDelim;

					strVal.TrimLeft();
					strVal.TrimRight();
											/* Decrypt the value just in case
												it was encrypted */

					ChUtil::EncryptString( strVal, false );

					boolFound = true;
				}
			}
		}
	}

	return boolFound;
}



bool ChWorldInfo::FindTag( std::ifstream& ifile, const char* pstrTag, chint16& sVal )
{
	bool		boolFound;
	ChString		strVal;

	if (boolFound = FindTag( ifile, pstrTag, strVal ))
	{
		sVal = atoi( strVal );
	}

	return boolFound;
}


bool ChWorldInfo::FindTag( std::ifstream& ifile, const char* pstrTag, chuint16& suVal )
{
	bool		boolFound;
	chint16		sVal;

	if (boolFound = FindTag( ifile, pstrTag, sVal ))
	{
		suVal = (chuint16)sVal;
	}

	return boolFound;
}


bool ChWorldInfo::FindTag( std::ifstream& ifile, const char* pstrTag, chint32& lVal )
{
	bool		boolFound;
	ChString		strVal;

	if (boolFound = FindTag( ifile, pstrTag, strVal ))
	{
		lVal = atol( strVal );
	}

	return boolFound;
}


bool ChWorldInfo::FindTag( std::ifstream& ifile, const char* pstrTag, chuint32& luVal )
{
	bool		boolFound;
	chint32		lVal;

	if (boolFound = FindTag( ifile, pstrTag, lVal ))
	{
		luVal = (chuint32)lVal;
	}

	return boolFound;
}



#endif	// defined( CH_MSW )


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:21  uecasm
// Import of source tree as at version 2.53 release.
//
