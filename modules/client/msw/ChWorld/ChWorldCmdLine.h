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

	This file contains the interface for the world command line processing
	class

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHWORLDCMDLINE_H )
#define _CHWORLDCMDLINE_H


/*----------------------------------------------------------------------------
	Forward class declaration
----------------------------------------------------------------------------*/

class ChArgumentList;


/*----------------------------------------------------------------------------
	ChWorldCmdLine class
----------------------------------------------------------------------------*/

class ChWorldCmdLine
{
	public :
		ChWorldCmdLine( ChArgumentList* pArgs );
		~ChWorldCmdLine() {}

		inline const ChString& GetHomePage() { return m_strHomePage; }
		inline const ChWorldType* GetWorldType() const { return &m_worldType; }
		inline const ChString& GetWorldName() const { return m_strWorldName; }

		bool GetWorldServer( ChString& strWorldName, ChString& strWorldServer,
									chint16& sWorldPort, ChWorldType& type,
									ChLoginType& loginType,
									ChString& strUsername, ChString& strPassword );

		void SetWorldServer( const char* pstrWorldServer,
								chint16 sWorldPort = 0 );
		void SetWorld( const ChString& strWorldName,
						const ChWorldType* pType = 0 );


#if defined( CH_PUEBLO_PLUGIN )
		inline const ChString& GetOnDisconnectURL() { return m_strOnDisconnectURL; }
#endif

	private :
		void InitCmdLine();

	private :

		ChString				m_strFilepath;
		ChString				m_strWorldServer;
		chint16				m_sWorldPort;
		ChString				m_strHomePage;
		ChString				m_strWorldName;	// Empty string when not connected
		ChWorldType			m_worldType;	// Empty string when not connected
#if defined( CH_PUEBLO_PLUGIN )
		ChString				m_strUserName;
		ChString				m_strPassword;
		ChString				m_strOnDisconnectURL;
		ChLoginType			m_loginType;
#endif
};


#endif // #if !defined( _CHWORLDCMDLINE_H )

// $Log$
