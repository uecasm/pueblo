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

	This file contains the implementation of the world command line processing
	methods

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <fstream>
#include <ChArgList.h>
#include <ChUrlMap.h>
#include "ChWorldCmdline.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define WORLD_SERVER	"worldserver"
#define WORLD_PORT		"worldport"
#define WORLD_TYPE		"worldtype"
#define WORLD_LIST		"worldlist"
#define WORLD_DISCONNECT "worldquit"
#define WORLD_USER		"worldusername"
#define WORLD_PASSWORD	"worldpassword"
#define WORLD_SHORTCUT	"_worldshortcut"


/*----------------------------------------------------------------------------
	ChWorldCmdLine class
----------------------------------------------------------------------------*/

ChWorldCmdLine::ChWorldCmdLine( ChArgumentList* pList )
{
	InitCmdLine();
	
	if (pList)
	{										// Process the arguments

#if !defined( CH_PUEBLO_PLUGIN )

		ChString		strCmdLine;
		if (pList->FindArg( CMD_LINE, strCmdLine ))
		{
			char		cHost[256];
			chint16		sPort;

			strCmdLine.TrimLeft();				// get rid of any leading whitespace that managed to sneak aboard
			strCmdLine.TrimRight();				// likewise with trailing whitespace
			strCmdLine.TrimLeft('"');			// remove a leading quote character, if any
			strCmdLine.TrimRight('"');		// likewise for trailing quote character, if any

			// Check to see if it is a Telnet URL
			ChURLParts url;
			if (url.GetURLParts( strCmdLine ))
			{
				// it looks like an URL
				if (url.GetScheme() == ChURLParts::typeTelnet)
				{
					SetWorldServer( url.GetHostName(), url.GetPortNumber() );
				}
				else
				{
					// Not a telnet URL, so we'll just do something silly.
				  SetWorldServer( strCmdLine, 23 );
				}
			}
			else if (2 == sscanf( strCmdLine, "%s %hd", cHost, &sPort ))
			{
				// Found a host/port combination
			  SetWorldServer( cHost, sPort );
			}
			else
			{
				if (ChUtil::FileExists( strCmdLine ))
				{
					m_strFilepath = strCmdLine;
				}
				else
				{
				  SetWorldServer( cHost, 23 );
				}
			}
		}
#else
		if (pList->FindArg( WORLD_SERVER, m_strWorldServer ))
		{
			ChString strTemp;
  			if ( !pList->FindArg( WORLD_PORT, strTemp ))
			{
				m_sWorldPort = 23;
			}
			else
			{
				m_sWorldPort = (chint16)atol( strTemp );
			}

  			if ( pList->FindArg( WORLD_TYPE, strTemp ))
			{
				ChWorldType worldType( strTemp );

				m_worldType = worldType;
				m_loginType = worldType.GetLoginType();
			}
			else
			{
				m_worldType = otherType;
			}

  			if ( pList->FindArg( WORLD_USER, m_strUserName ))
			{
				pList->FindArg( WORLD_PASSWORD, m_strPassword );
									/* Decrypt the value just in case
										it was encrypted */

				ChUtil::EncryptString( m_strPassword, false );
			}
		}
		else
		{
			pList->FindArg( WORLD_SHORTCUT, m_strFilepath );
		}

		pList->FindArg( WORLD_LIST, m_strHomePage );
		pList->FindArg( WORLD_DISCONNECT, m_strOnDisconnectURL );


#endif
		if ( m_strHomePage.IsEmpty() )
		{
											// Home page setting
			pList->FindArg( "PuebloList", m_strHomePage );
		}
	}

}


void ChWorldCmdLine::InitCmdLine()
{
	m_strFilepath = "";
	m_strWorldServer = "";
	m_sWorldPort = 0;
}


bool ChWorldCmdLine::GetWorldServer( ChString& strWorldName,
										ChString& strWorldServer,
										chint16& sWorldPort,
										ChWorldType& type,
										ChLoginType& loginType,
										ChString& strUsername,
										ChString& strPassword )
{
	bool	boolServerFound = false;

	if (!m_strWorldServer.IsEmpty())
	{										/* This document was created from
												the command line and contains
												only the world host and port.
												Fill in the world settings
												as best we can. */
		boolServerFound = true;

		strWorldName = m_strWorldServer;
		strWorldServer = m_strWorldServer;
		sWorldPort = m_sWorldPort;

		#if !defined( CH_PUEBLO_PLUGIN )

		type = otherType;
		loginType = variableLogin;
		strUsername = "";
		strPassword = "";

		#else
		
		type = m_worldType;
		strUsername = m_strUserName;
		strPassword = m_strPassword;
		loginType = m_loginType;
		
		#endif
	}
	else
	{
		if (m_strFilepath.IsEmpty())
		{
			boolServerFound = false;
		}
		else
		{
		   	std::ifstream ifile( m_strFilepath );

			if ( ifile.is_open() )
			{
				ChWorldInfo worldInfo( ifile );

				if ( worldInfo.IsValid() )
				{
					strWorldName 	= worldInfo.GetName();
					strWorldServer 	= worldInfo.GetHost();
					sWorldPort 		= worldInfo.GetPort();
					type 			= worldInfo.GetType();
					loginType 		= worldInfo.GetLoginType();
					strUsername 	= worldInfo.GetUsername();
					strPassword 	= worldInfo.GetPassword();

		#if defined( CH_PUEBLO_PLUGIN )
					m_strOnDisconnectURL = worldInfo.GetOnDisconnect();
		#endif

					boolServerFound = true;
				}
			}
		}
	}

	return boolServerFound;
}


void ChWorldCmdLine::SetWorldServer( const char* pstrWorldServer,
										chint16 sWorldPort )
{
											/* This method fills in the host
												and port for a world server
												entered on the command line */
	InitCmdLine();

	m_strWorldServer = pstrWorldServer;
	m_sWorldPort = sWorldPort;
}


void ChWorldCmdLine::SetWorld( const ChString& strWorldName,
								const ChWorldType* pType )
{
	m_strWorldName = strWorldName;

	if (pType)
	{
		m_worldType = *pType;
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:32  uecasm
// Import of source tree as at version 2.53 release.
//
