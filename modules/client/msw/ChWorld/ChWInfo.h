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

	Contains the interface for the ChWorldInfo class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHWINFO_H )
#define _CHWINFO_H

#include <ChWType.h>

class ChCore;
//class ifstream;


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define WORLD_NAME_SEPARATOR		'/'


/*----------------------------------------------------------------------------
	ChWorldInfo class
----------------------------------------------------------------------------*/

class ChWorldInfo
{
	public:
		ChWorldInfo( const ChString& strCommand );
		ChWorldInfo( const ChString& strName, const ChString& strDesc,
						const ChString& strHost, chint16 sPort,
						const ChWorldType& type, ChLoginType login,
						const ChString& strUsername, const ChString& strPassword,
						const ChString& strHomePage );
		ChWorldInfo( const ChString& strName, const ChString& strDesc,
						const ChString& strHost, const ChString& strAddr,
						chint16 sPort, const ChWorldType& type,
						ChLoginType login, const ChString& strUsername,
						const ChString& strPassword, const ChString& strHomePage );
		
		ChWorldInfo( std::ifstream& ifile  );

		virtual ~ChWorldInfo() {}

		inline bool IsValid() { return m_boolValid; }

		inline const ChString& GetName() const { return m_strName; }
		inline const ChString& GetDesc() const { return m_strDesc; }
		inline const ChString& GetHost() const { return m_strHost; }
		inline const ChString& GetAddr() const
								{
									if (m_strAddr.GetLength() > 0)
									{
										return m_strAddr;
									}
									else
									{
										return m_strHost;
									}
								}
		inline chint16 GetPort() const { return m_sPort; }
		inline const ChWorldType& GetType() const { return m_type; }
		inline ChLoginType GetLoginType() const { return m_loginType; }
		inline const ChString& GetUsername() const { return m_strUsername; }
		inline const ChString& GetPassword() const { return m_strPassword; }
		inline const ChString& GetHomePage() const { return m_strHomePage; }
	
	#if defined( CH_PUEBLO_PLUGIN )
		inline const ChString& GetOnDisconnect() const { return m_strOnDisconnect; }
	#endif

		inline void SetAddr( const ChString& strAddr ) { m_strAddr = strAddr; }

		void Stringize( ChString& strWorld );
		void CreateShortcut( ChCore* pCore );

		void Set( const ChString& strDesc, const ChString& strHost,
					chint16 sPort, const ChWorldType& type, ChLoginType login,
					const ChString& strUsername = "",
					const ChString& strPassword = "",
					const ChString& strHomePage = "" );
		void Set( const ChString& strDesc, const ChString& strHost,
					const ChString& strAddr, chint16 sPort,
					const ChWorldType& type, ChLoginType login,
					const ChString& strUsername = "",
					const ChString& strPassword = "",
					const ChString& strHomePage = "" );

	protected:
		bool GetKey( ChString& strCommand );
		void ProcessKey( const ChString& strKey, const ChString& strValue );
		void Validate();
		void Escape( ChString& strValue );

		void RemoveIllegalChars( ChString& strValue );

		#if defined( CH_MSW )

		void CreateWindowsShortcut( ChCore* pCore );
		bool WriteWindowsShortcutFile( ChCore* pCore, const ChString& strPath,
										const ChString& strFilePath,
										const ChString& strGroupName,
										const ChString& strName,
										const ChString& strUsername,
										const ChString& strPassword,
										const ChString& strHost,
										const ChString& strHomePage,
										chint16 sPort,
										const ChWorldType& type,
										ChLoginType loginType );
		void CreateProgmanIcon( const ChString& strFilePath,
									const ChString& strGroupName,
									const ChString& strName  );

		#endif	// defined( CH_MSW )

	private :
		bool FindTag( std::ifstream& ifile, const char* pstrTag, ChString& strVal );
		bool FindTag( std::ifstream& ifile, const char* pstrTag, chint16& sVal );
		bool FindTag( std::ifstream& ifile, const char* pstrTag, chuint16& suVal );
		bool FindTag( std::ifstream& ifile, const char* pstrTag, chint32& lVal );
		bool FindTag( std::ifstream& ifile, const char* pstrTag, chuint32& luVal );


	protected:
		bool		m_boolValid;
		ChString		m_strName;
		ChString		m_strDesc;
		ChWorldType	m_type;
		ChLoginType	m_loginType;
		ChString		m_strHost;
		ChString		m_strAddr;
		chint16		m_sPort;
		ChString		m_strUsername;
		ChString		m_strPassword;
		ChString		m_strHomePage;
#if defined( CH_PUEBLO_PLUGIN )
		ChString		 m_strOnDisconnect;
#endif

};


#endif	// !defined( _CHWINFO_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
