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

     Ultra Enterprises:  Gavin Lambert
          Added basic recognition for Windows 98.  This is for output
          strings only; internally the program still calls both 'Win95'.
          Also added recognition for Windows XP (as NT 5 or greater); I
          think this actually catches 2000 as well.  This is stored
          internally as 'WinXP', allowing for some differentiation of
          features; for the most part, though, it'll behave the same as
          Windows 95 (rather than Windows NT).

------------------------------------------------------------------------------

	This file consists of the implementation of the ChClientInfo class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChArch.h>
#include <ChClInfo.h>
#include <ChConst.h>
#include <PbVersion.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChClientInfo static members
----------------------------------------------------------------------------*/

const char*	ChClientInfo::strWin = "Windows";
const char*	ChClientInfo::strWin32s = "Win32s";
const char*	ChClientInfo::strWinNT = "WinNT";
const char*	ChClientInfo::strWin95 = "Win95";
const char* ChClientInfo::strWin98 = "Win98";
const char* ChClientInfo::strWinXP = "Win2K/XP";
const char*	ChClientInfo::strLinux = "Linux";
const char*	ChClientInfo::strSPARCSolaris = "Solaris/SPARC";

const char*	ChClientInfo::strUnknown = "Unknown";
WSADATA		ChClientInfo::m_socketInfo;


/*----------------------------------------------------------------------------
	ChClientInfo constructor and destructor
----------------------------------------------------------------------------*/

ChClientInfo::ChClientInfo()
{
											// For de-serialization
}


ChClientInfo::ChClientInfo( chint16 sMode )
{
	m_clientVersion.Set( VERS_CLIENT_MAJOR, VERS_CLIENT_MINOR,
#ifdef UE_PRERELEASE
							VERS_CLIENT_COMMENT " PRERELEASE" );
#else
							VERS_CLIENT_COMMENT );
#endif

	#if defined( CH_MSW )
	{
											// Get platform information
		#if defined( CH_ARCH_32 )
		{
			OSVERSIONINFO	versionInfo;

			versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );
			GetVersionEx( &versionInfo );

			m_platformVersion.Set( (chint16)versionInfo.dwMajorVersion,
									(chint16)versionInfo.dwMinorVersion );

			switch( versionInfo.dwPlatformId )
			{
				case VER_PLATFORM_WIN32s:
				{
					m_strPlatform = strWin32s;
					m_platform = osWin32s;
					break;
				}
				case VER_PLATFORM_WIN32_WINDOWS:
				{
					if(m_platformVersion >= 0x0004000A)		// Win98/ME
					{
						m_strPlatform = strWin98;
						m_platform = osWin98;
					}
					else
					{
						m_strPlatform = strWin95;
						m_platform = osWin95;
					}
					break;
				}

				case VER_PLATFORM_WIN32_NT:
				{
					if(m_platformVersion >= 0x00050000) {		// Win2000/XP/2003
						m_strPlatform = strWinXP;
						m_platform = osWinXP;
					} else {
						m_strPlatform = strWinNT;
						m_platform = osWinNT;
					}
					break;
				}

				default:
				{
					m_strPlatform = strUnknown;
					m_platform = osUnknown;
					break;
				}
			}
		}
		#else	// defined( CH_ARCH_32 )
		{
			DWORD	dwVersion = GetVersion();
			WORD	wVersion = LOWORD( dwVersion );
 			m_strPlatform = strWin;
			m_platform = osWin16;

			m_platformVersion.Set( (chint16)LOBYTE( wVersion ),
									(chint16)HIBYTE( wVersion ) );
		}
		#endif	// defined( CH_ARCH_32 )
	    
	    #if defined( CH_ARCH_32 )
												// Get hardware information
		SYSTEM_INFO		sysInfo;
		GetSystemInfo( &sysInfo );

		switch( sysInfo.dwProcessorType )
		{
			case PROCESSOR_INTEL_386:
			{
				m_strProcessor = "Intel 386";
				break;
			}

			case PROCESSOR_INTEL_486:
			{
				m_strProcessor = "Intel 486";
				break;
			}

			case PROCESSOR_INTEL_PENTIUM:
			{
				m_strProcessor = "Intel Pentium";
				break;
			}

			default:
			{
				m_strProcessor = strUnknown;
				break;
			}
		}

		m_lProcessorCount = (chint32)sysInfo.dwNumberOfProcessors;    
		#else    
		m_lProcessorCount = 1;
		DWORD dwFlags = ::GetWinFlags();
		
		m_strProcessor = (dwFlags & WF_CPU286) ? "Intel 286" :
		    			 (dwFlags & WF_CPU386) ? "Intel 386" :
		    			 (dwFlags & WF_CPU486) ? "Intel 486" : strUnknown;
		#endif

											// Socket information 
											// Should call WSAStartUp only once per
											// app or DLL, hte function fails if called
											// more than once  
		if ( 0 == m_socketInfo.wVersion )
		{										// Initialize Winsock
			#if defined( CH_ARCH_32 )	
			if (0 != WSAStartup( MAKEWORD( 2, 0 ), &m_socketInfo ))
			#else
			if (0 != WSAStartup( MAKEWORD(1, 1), &m_socketInfo ))
			#endif
			{
												// Use the version they want
	
				WSAStartup( m_socketInfo.wHighVersion, &m_socketInfo );
			}
		}
		
		if ( m_socketInfo.wVersion )
		{									
			m_strSocketsDescription = m_socketInfo.szDescription; 

			m_socketsBestVersion.Set( LOBYTE( m_socketInfo.wVersion ),
										HIBYTE( m_socketInfo.wVersion ) );
			m_socketsHighVersion.Set( LOBYTE( m_socketInfo.wHighVersion ),
										HIBYTE( m_socketInfo.wHighVersion ) );
		}
		else
		{
			m_strSocketsDescription = TEXT( "Unknown" ); 
		}
	}
	#elif defined( CH_UNIX )
	{
		#ifdef __linux__
		{
			m_strPlatform = strLinux;
			m_platform = osLinux;
		}
		#else
		{
			m_strPlatform = strSPARCSolaris;
			m_platform = osSPARCSolaris;
		}
		#endif

		m_lProcessorCount = 1;
		m_strSocketsDescription = "Unix sockets";
		m_socketsBestVersion.Set( 0, 0 );
		m_socketsHighVersion.Set( 0, 0 );
	}
	#endif	// defined( CH_UNIX )
}

ChClientInfo::~ChClientInfo()
{
}


/*----------------------------------------------------------------------------
	ChClientInfo public methods
----------------------------------------------------------------------------*/

void ChClientInfo::Serialize( ChArchive& archive )
{
	ChStreamable::Serialize( archive );

											// Now serialize ourself
	if (modeWrite & archive.GetMode())
	{
		ChVersion	infoVersion( major, minor );
		chint16		sPlatform = (chint16)m_platform;

											// Write out data
		archive << infoVersion
				<< m_clientVersion
				<< m_strPlatform << m_platformVersion
				<< m_strProcessor << m_lProcessorCount
				<< m_socketsBestVersion << m_socketsHighVersion
				<< m_strSocketsDescription
				<< sPlatform;
	}
	else
	{
		ChVersion	infoVersion;
		chint16		sPlatform;
											// Read in data
		archive << infoVersion
				>> m_clientVersion
				>> m_strPlatform >> m_platformVersion
				>> m_strProcessor >> m_lProcessorCount
				>> m_socketsBestVersion >> m_socketsHighVersion
				>> m_strSocketsDescription
				>> sPlatform;

		m_platform = (OSType)sPlatform;
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:14  uecasm
// Import of source tree as at version 2.53 release.
//
