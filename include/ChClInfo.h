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

	This file consists of the interface for the ChClientInfo class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHCLINFO_H ))
#define _CHCLINFO_H        

#if defined( CH_MSW )

	#include <afxsock.h>

#endif	// defined( CH_MSW )

#include <ChTypes.h>
#include <ChData.h>
#include <ChStrmbl.h>
#include <ChVers.h>

#if defined( CH_MSW )
	#if !defined( VER_PLATFORM_WIN32_WINDOWS )
		#define VER_PLATFORM_WIN32_WINDOWS		1
	#endif	// !defined( VER_PLATFORM_WIN32_WINDOWS )
#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChClientInfo class
----------------------------------------------------------------------------*/

typedef enum { osUnknown = 0, osWin16, osWin32s, osWinNT, osWin95, osWinXP,
				osLinux, osSPARCSolaris } OSType;


/*----------------------------------------------------------------------------
	ChClientInfo class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChClientInfo : public ChStreamable
{
	public:
		enum tagModes { thisMachine };

	protected:
		enum tagInfoVersion { major = 1, minor = 0 };

	public:
		static const char*		strWin;
		static const char*		strWin32s;
		static const char*		strWinNT;
		static const char*		strWin95;
		static const char*		strWin98;
		static const char*		strWinXP;
		static const char*		strLinux;
		static const char*		strSPARCSolaris;
		static const char*		strUnknown;

	public:
		ChClientInfo();
		ChClientInfo( chint16 sMode );
		virtual ~ChClientInfo();

		inline const ChVersion& GetClientVersion() const
							{ return m_clientVersion; }
		inline const OSType GetPlatform() const { return m_platform; }
		inline const ChString& GetPlatformName() const { return m_strPlatform; }
		inline const ChVersion& GetPlatformVersion() const
							{ return m_platformVersion; }
		inline const ChString& GetProcessor() const { return m_strProcessor; }
		inline chint32 GetProcessorCount() const { return m_lProcessorCount; }
		inline const ChVersion& GetSocketsBestVersion() const
							{ return m_socketsBestVersion; }
		inline const ChVersion& GetSocketsHighVersion() const
							{ return m_socketsHighVersion; }
		inline const ChString& GetSocketsDescription() const
							{ return m_strSocketsDescription; }

											// Overrides

		virtual void Serialize( ChArchive& ar );

	private:
		ChVersion			m_clientVersion;
		ChString				m_strPlatform;
		OSType				m_platform;
		ChVersion			m_platformVersion;
		ChString				m_strProcessor;
		chint32				m_lProcessorCount;
		ChVersion			m_socketsBestVersion;
		ChVersion			m_socketsHighVersion;
		ChString				m_strSocketsDescription;
		static WSADATA		m_socketInfo;
};

// $Log$

#endif	// !defined( _CHCLINFO_H )
