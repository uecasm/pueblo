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
     
          Eliminated nonstandard buffering system from sockbuf.

------------------------------------------------------------------------------

	This file consists of the Internet implementation for the Sockets++
	classes.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"						// Precompiled header directive

#include <ChSock.h>
#include <ChExcept.h>
#include <ChUtil.h>
#include <ChReg.h>
#include <ChConst.h>

#if defined( CH_VRML_VIEWER )
	#include "ChGrType.h"
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define _CH_SUPPORTED_ASYNC_EVENTS	(CH_SOCK_EVENT_READ | \
										CH_SOCK_EVENT_OOB_READ | \
										CH_SOCK_EVENT_CONNECT | \
										CH_SOCK_EVENT_CLOSE)


#if defined( CH_MSW ) && defined(  CH_ARCH_32 )

	#define CH_WSA_NOTIF_CLASS	"Chaco-WSA-Notification-Class"
	#define	CH_MSG_WSA_PACKET_RECEIVED		WM_USER + 500
	#define CH_MSG_WSA_NOTIFY_EVENT			WM_USER + 501

	chint32 CALLBACK EXPORT SocketsXXWSAproc( HWND hwnd, UINT  uMsg,
												WPARAM wParam, LPARAM lParam );

	HWND							ChChacoSocket::m_hwndWSA = 0;
	ChPtrSplay<sockasyncinfo>		sockasyncinfo::m_reqList;
	bool							sockasyncinfo::m_boolBlocking = false;



#endif	// defined( WIN32 )


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( void )
WSADisplayError( int iError, const char* pstrContext );


/*----------------------------------------------------------------------------
	Socks related statics
----------------------------------------------------------------------------*/

bool			ChChacoSocket::m_boolSocksAddrInited = false;
sockinetaddr	ChChacoSocket::m_saSOCKS;
ChString 			ChChacoSocket::m_strSocksHostName;
chuint32		ChChacoSocket::m_luSocksPort = 0;


class ZapAsyncRequest : public ChPtrSplayVisitor2<sockasyncinfo >
{
	public:
	 	bool Visit( chparam key,  const sockasyncinfo* pReq )
		{
			if ( pReq->IsValid() )
			{
	 			WSACancelAsyncRequest( (HANDLE) key );
			}
			delete (sockasyncinfo*)pReq;

			return true;
		}
};

void sockasyncinfo::CancelBlocking()
{
	ZapAsyncRequest		zapAsync;

  	m_reqList.Infix( zapAsync );

	m_reqList.Erase();

	sockasyncinfo::StopBlocking();

}


void sock_error (const char* classname, const char* msg)
{
	#if defined( CH_VERBOSE )
    if (errno)
	perror (msg);
    cerr << classname << ' ' << msg << endl;
	#endif
    errno = 0;
}

void sockAddr::error( const char *pstrErr ) const
{
    sock_error( "class sockAddr ", pstrErr );
}



sockinetaddr::sockinetaddr()
{
	m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	m_sockAddrIn.sin_addr.s_addr = htonl( INADDR_ANY );
	m_sockAddrIn.sin_port = 0;
}

sockinetaddr::sockinetaddr( chuint32 luAddr, chint16 sPort )
{
											/* luAddr and sPort are in host
												byte order */
	m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	m_sockAddrIn.sin_addr.s_addr = htonl( luAddr );
	m_sockAddrIn.sin_port = htons( sPort );
}

sockinetaddr::sockinetaddr( chuint32 luAddr, const char *pstrServiceName,
							const char *pstrProtocolName, bool boolAsync )
{
											// addr is in host byte order
	m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	m_sockAddrIn.sin_addr.s_addr = htonl( luAddr );		/* Added by cgay@cs.uoregon.edu
												May 29, 1993 */
	if ( boolAsync )
	{
		AsyncSetPort( pstrServiceName, pstrProtocolName );
	}
	else
	{
		SetPort( pstrServiceName, pstrProtocolName );
	}
}

sockinetaddr::sockinetaddr( const char *pstrHostName, chint16 sPort, bool boolAsync )
{

												// port_no is in host byte order
	if ( boolAsync )
	{
		AsyncSetAddr( pstrHostName );
	}
	else
	{
		SetAddr( pstrHostName );
	}
	m_sockAddrIn.sin_port = htons( sPort );
}

sockinetaddr::sockinetaddr( const char *pstrHostName,
							const char *pstrServiceName,
							const char *pstrProtocolName,
							bool boolAsync )
{
	if ( boolAsync )
	{
		AsyncSetAddr( pstrHostName );
		AsyncSetPort( pstrServiceName, pstrProtocolName );
	}
	else
	{
		SetAddr( pstrHostName );
		SetPort( pstrServiceName, pstrProtocolName );
	}
}

sockinetaddr::sockinetaddr( const sockinetaddr& sockINetAddr )
{
	m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	m_sockAddrIn.sin_addr.s_addr = sockINetAddr.m_sockAddrIn.sin_addr.s_addr;
	m_sockAddrIn.sin_port = sockINetAddr.m_sockAddrIn.sin_port;
}




void sockinetaddr::SetPort( const char *pstrServiceName,
							const char *pstrProtocolName )
{
	servent *pServiceEntry = getservbyname( pstrServiceName,
											pstrProtocolName );

	if (pServiceEntry == 0)
	{
		#if defined( CH_MSW ) && defined( _DEBUG )
		{
			WSADisplayError( WSAGetLastError(), "sockinetaddr::setport" );
		}
		#endif	// defined( CH_MSW ) && defined( _DEBUG )

		#if defined( CH_EXCEPTIONS )
		{
			switch( WSAGetLastError() )
			{
				case WSAETIMEDOUT:
				{							// Throw a timed-out exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{

						THROW( new ChSocketEx( ChEx::socketTimedOut ));
					}
					#else
					throw ChSocketEx( ChEx::socketTimedOut );
					#endif
					//break;
				}

				case WSAEINPROGRESS :
				{

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::inProgress ));
					}
					#else
					throw ChSocketEx( ChEx::inProgress );
					#endif
					//break;
				}
				default:
				{							// Throw a misc. failure exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::hostNotFound ));
					}
					#else
					throw ChSocketEx( ChEx::hostNotFound );
					#endif
					//break;
				}
			}
		}
		#else
		{
			//herror( "sockinetaddr::sockinetaddr -- Bad host name" );
			return;
		}
		#endif
	}

	m_sockAddrIn.sin_port = pServiceEntry->s_port;
}


void sockinetaddr::AsyncSetPort( const char *pstrServiceName,
							const char *pstrProtocolName )
{
	bool boolSuccess = false;

	if ( !sockasyncinfo::IsBlocking() )
	{

		sockasyncinfo *pAsyncInfo = new sockasyncinfo( MAXGETHOSTSTRUCT );

		ASSERT( pAsyncInfo );

		sockasyncinfo::StartBlocking();




		HANDLE hWaitOn = WSAAsyncGetServByName (
			    ChChacoSocket::GetWSAWnd(),
			    CH_MSG_WSA_PACKET_RECEIVED,
			    pstrServiceName,
			    pstrProtocolName,
			    pAsyncInfo->GetBuf(),
			    MAXGETHOSTSTRUCT
			   );



		if ( hWaitOn )
		{	
			pAsyncInfo->Valid();
			// wait for the async event to occur
			sockasyncinfo::m_reqList.Insert( (chparam)hWaitOn, pAsyncInfo );

			// wait till the call succeeds or fails, but keep pumping messages
			ChChacoSocket::PumpMessages( hWaitOn );

			WPARAM wParam;
			LPARAM lParam;


			pAsyncInfo->GetParams( wParam, lParam );

			if (WSAGETASYNCERROR( lParam) == 0 )
			{
				servent *pServiceEntry =(servent *)pAsyncInfo->GetBuf();
			  	m_sockAddrIn.sin_port = pServiceEntry->s_port;

				boolSuccess = true;
			}
		}

		sockasyncinfo::StopBlocking();

		delete pAsyncInfo;
	}


	if ( !boolSuccess )
	{
		#if defined( CH_MSW ) && defined( _DEBUG )
		{
			WSADisplayError( WSAGetLastError(), "sockinetaddr::setport" );
		}
		#endif	// defined( CH_MSW ) && defined( _DEBUG )

		#if defined( CH_EXCEPTIONS )
		{
			switch( WSAGetLastError() )
			{
				case WSAETIMEDOUT:
				{							// Throw a timed-out exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{

						THROW( new ChSocketEx( ChEx::socketTimedOut ));
					}
					#else
					throw ChSocketEx( ChEx::socketTimedOut );
					#endif
					//break;
				}

				case WSAEINPROGRESS :
				{

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::inProgress ));
					}
					#else
					throw ChSocketEx( ChEx::inProgress );
					#endif
					//break;
				}
				default:
				{							// Throw a misc. failure exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::hostNotFound ));
					}
					#else
					throw ChSocketEx( ChEx::hostNotFound );
					#endif
					//break;
				}
			}
		}
		#else
		{
			//herror( "sockinetaddr::sockinetaddr -- Bad host name" );
			return;
		}
		#endif
	}
}


int sockinetaddr::GetPort() const
{
	return ntohs( m_sockAddrIn.sin_port );
}

void sockinetaddr::SetAddr( const char *pstrHostName )
{
											/* First try to interpret the host
												name as a *.*.*.* address */

	if ( long(m_sockAddrIn.sin_addr.s_addr = inet_addr( pstrHostName )) == -1)
	{
											/* If that didn't work, then try
												to look up the host by name */


		hostent * pHostEntry = gethostbyname( pstrHostName );

		if (pHostEntry == 0)
		{
			#if defined( CH_MSW ) && defined( _DEBUG )
			{
				WSADisplayError( WSAGetLastError(), "sockinetaddr::setaddr" );
			}
			#endif	// defined( CH_MSW ) && defined( _DEBUG )

			#if defined( CH_EXCEPTIONS )
			{
				switch( WSAGetLastError() )
				{
					case WSAETIMEDOUT:
					{							// Throw a timed-out exception

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{

							THROW( new ChSocketEx( ChEx::socketTimedOut ));
						}
						#else
						throw ChSocketEx( ChEx::socketTimedOut );
						#endif
						//break;
					}

					case WSAEINPROGRESS :
					{

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{
							THROW( new ChSocketEx( ChEx::inProgress ));
						}
						#else
						throw ChSocketEx( ChEx::inProgress );
						#endif
						//break;
					}
					default:
					{							// Throw a misc. failure exception

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{
							THROW( new ChSocketEx( ChEx::hostNotFound ));
						}
						#else
						throw ChSocketEx( ChEx::hostNotFound );
						#endif
						//break;
					}
				}
			}
			#else
			{
				//herror( "sockinetaddr::sockinetaddr -- Bad host name" );
				return;
			}
			#endif
		}
        #if defined( CH_ARCH_32 )
		memcpy( &m_sockAddrIn.sin_addr, pHostEntry->h_addr, pHostEntry->h_length );
		#else
		m_sockAddrIn.sin_addr.s_addr = ((LPIN_ADDR)pHostEntry->h_addr)->s_addr;
		#endif
		m_sockAddrIn.sin_family = pHostEntry->h_addrtype;
	}
	else
	{
		m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	}
}

void sockinetaddr::AsyncSetAddr( const char *pstrHostName )
{
	bool boolSuccess = false;
											/* First try to interpret the host
												name as a *.*.*.* address */

	if ( long(m_sockAddrIn.sin_addr.s_addr = inet_addr( pstrHostName )) == -1)
	{
											/* If that didn't work, then try
												to look up the host by name */


		if ( !sockasyncinfo::IsBlocking() )
		{

			sockasyncinfo *pAsyncInfo = new sockasyncinfo( MAXGETHOSTSTRUCT );

			ASSERT( pAsyncInfo );

			sockasyncinfo::StartBlocking();
			HANDLE hWaitOn = WSAAsyncGetHostByName( ChChacoSocket::GetWSAWnd(),
													CH_MSG_WSA_PACKET_RECEIVED,
													pstrHostName,
													pAsyncInfo->GetBuf(),
													MAXGETHOSTSTRUCT );

			if (hWaitOn)
			{					
		
				pAsyncInfo->Valid();
			
								/* Wait for the async event to
												occur */

				sockasyncinfo::m_reqList.Insert( (chparam)hWaitOn, pAsyncInfo );

											/* Wait till the call succeeds or
												fails, but keep pumping
												messages */

				ChChacoSocket::PumpMessages( hWaitOn );

				WPARAM wParam;
				LPARAM lParam;

				pAsyncInfo->GetParams( wParam, lParam );

				if (WSAGETASYNCERROR( lParam) == 0 )
				{
					hostent * pHostEntry = (hostent *)pAsyncInfo->GetBuf();

			        #if defined( CH_ARCH_32 )
					memcpy( &m_sockAddrIn.sin_addr, pHostEntry->h_addr, pHostEntry->h_length );
					#else
					m_sockAddrIn.sin_addr.s_addr = ((LPIN_ADDR)pHostEntry->h_addr)->s_addr;
					#endif
					m_sockAddrIn.sin_family = pHostEntry->h_addrtype;

					boolSuccess = true;
				}
			}

			sockasyncinfo::StopBlocking();

			delete pAsyncInfo;
		}


		if ( !boolSuccess)
		{
			#if defined( CH_MSW ) && defined( _DEBUG )
			{
				WSADisplayError( WSAGetLastError(), "sockinetaddr::setaddr" );
			}
			#endif	// defined( CH_MSW ) && defined( _DEBUG )

			#if defined( CH_EXCEPTIONS )
			{
				switch( WSAGetLastError() )
				{
					case WSAETIMEDOUT:
					{							// Throw a timed-out exception

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{

							THROW( new ChSocketEx( ChEx::socketTimedOut ));
						}
						#else
						throw ChSocketEx( ChEx::socketTimedOut );
						#endif
						//break;
					}

					case WSAEINPROGRESS :
					{

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{
							THROW( new ChSocketEx( ChEx::inProgress ));
						}
						#else
						throw ChSocketEx( ChEx::inProgress );
						#endif
						//break;
					}
					default:
					{							// Throw a misc. failure exception

						#if defined( CH_MSW) && defined( CH_ARCH_16 )
						{
							THROW( new ChSocketEx( ChEx::hostNotFound ));
						}
						#else
						throw ChSocketEx( ChEx::hostNotFound );
						#endif
						//break;
					}
				}
			}
			#else
			{
				//herror( "sockinetaddr::sockinetaddr -- Bad host name" );
				return;
			}
			#endif
		}
	}
	else
	{
		m_sockAddrIn.sin_family = sockinetbuf::af_inet;
	}
}

const char* sockinetaddr::GetHostname() const
{
	if (m_sockAddrIn.sin_addr.s_addr == htonl( INADDR_ANY ))
	{
		static char		strHostname[64];

		if (::gethostname( strHostname, 63 ) == -1)
		{
			#if !defined( CH_ARCH_16 )
  			TRACE( "in sockinetaddr::gethostname\n" );
  			#endif
			return "";
		}

		return strHostname;
	}

	hostent *pHostEntry = gethostbyaddr( (const char*)&m_sockAddrIn.sin_addr,
											sizeof( m_sockAddrIn.sin_addr ),
				    						GetFamily() );
	if (0 == pHostEntry)
	{
		//herror( "sockinetaddr::gethostname" );

		return "";
	}

	if (pHostEntry->h_name)
	{
		return pHostEntry->h_name;
	}

	return "";
}


/*----------------------------------------------------------------------------
	ChChacoSocket class
----------------------------------------------------------------------------*/

ChChacoSocket::ChChacoSocket() :
					m_pAsyncSock( 0 ),
					m_iCount( 1 )
{
	m_pSyncSock = new ChSyncChacoSocket();
	ASSERT( m_pSyncSock );

	m_pSyncSock->m_hSocket = INVALID_SOCKET;
}


ChChacoSocket::ChChacoSocket( ChSocketHandler pHandler,	chparam userData,
								sockbuf* socinet ) :
					m_pSyncSock( 0 ),
					m_iCount( 1 )
{
	m_pAsyncSock = new ChAsyncChacoSocket( pHandler, userData,
											(sockinetbuf*)socinet );
	ASSERT( m_pAsyncSock );

	m_pAsyncSock->m_hSocket = INVALID_SOCKET;
}


ChChacoSocket::~ChChacoSocket()
{
	if (GetSyncSocket())
	{
		delete m_pSyncSock;
	}
	else
	{
		delete m_pAsyncSock;
	}
}

											/* The following method displays
												Winsock error strings */

void ChChacoSocket::GetErrorMsg( int iError, ChString& strOutput )
{
	switch( iError )
	{
		case 0:
		{
			strOutput = "(no error)";
		}

		case WSAEADDRINUSE:
		{
			strOutput = "Specified address is already in use";
			break;
		}

		case WSAEADDRNOTAVAIL:
		{
			strOutput = "Specified address is not available from the "
						"local machine";
			break;
		}

		case WSAECONNREFUSED:
		{
			strOutput = "Attempt to connect was forcibly rejected";
			break;
		}

		case WSAENETDOWN:
		{
			strOutput = "The network subsystem has failed";
			break;
		}

		case WSAENOTSOCK:
		{
			strOutput = "Socket error: The descriptor is not a socket";
			break;
		}

		case WSAHOST_NOT_FOUND:
		{
			strOutput = "Socket error: Authorative Answer Host not found";
			break;
		}

		case WSANOTINITIALISED:
		{
			strOutput = "Socket error: WSAStartup must be called";
			break;
		}

		case WSAEWOULDBLOCK:
		{
			strOutput = "Socket error: Would block";
			break;
		}

		case WSAEINVAL:
		{
			strOutput = "Socket error: EINVAL";
			break;
		}

		case WSAETIMEDOUT:
		{
			strOutput = "Socket error: WSAETIMEDOUT";
			break;
		}

		default:
		{
			strOutput.Format( "Socket error code %d", iError );
			break;
		}
	}
}


bool ChChacoSocket::Create( UINT nSocketPort, int nSocketType,
							LPCTSTR lpszSocketAddress )
{
	if (GetSyncSocket())
	{
		return GetSyncSocket()->Create( nSocketPort, nSocketType,
												lpszSocketAddress );
	}
	else
	{
		BOOL	boolSuccess;

		boolSuccess =
			GetAsyncSocket()->Create( nSocketPort, nSocketType,
										FD_READ | FD_WRITE | FD_OOB |
											FD_ACCEPT | FD_CONNECT |
											FD_CLOSE,
										lpszSocketAddress );
		if (boolSuccess)
		{									// Set to non-blocking
			boolSuccess = GetAsyncSocket()->AsyncSelect();
		}
		return (boolSuccess != FALSE);
	}
}


bool ChChacoSocket::Attach( SOCKET hSocket )
{
	if (GetSyncSocket())
	{
		return GetSyncSocket()->Attach( hSocket );
	}
	else
	{
		if (GetAsyncSocket()->Attach( hSocket ) == FALSE) { return false; }

		// Set to non-blocking
		return (GetAsyncSocket()->AsyncSelect() != FALSE);
	}
}


SOCKET ChChacoSocket::Detach()
{
	if (GetSyncSocket())
	{
		return GetSyncSocket()->Detach();
	}
	else
	{
		return GetAsyncSocket()->Detach();
	}
}


bool ChChacoSocket::Connect( const SOCKADDR* lpSockAddr, int nSockAddrLen )
{
	bool	boolSuccess;

	if (GetSyncSocket())
	{
		boolSuccess = GetSyncSocket()->Connect( lpSockAddr, nSockAddrLen );
	}
	else
	{
		GetAsyncSocket()->m_iState = ChAsyncChacoSocket::stateWaitingForConnect;

		// Enable blocking
		GetAsyncSocket()->m_boolAsyncCallBlocking = true;

		boolSuccess = (GetAsyncSocket()->Connect( lpSockAddr, nSockAddrLen ) != FALSE);

		if ( !boolSuccess )
		{
			if ( WSAGetLastError()	== WSAEWOULDBLOCK )
			{
				// Pump messages to keep UI active
				GetAsyncSocket()->PumpMessages( (HANDLE)GetAsyncSocket()->m_hSocket );
			}
		}
		else
		{
			// Pump messages to keep UI active
			GetAsyncSocket()->PumpMessages( (HANDLE)GetAsyncSocket()->m_hSocket );
		}

		// Just in case
		GetAsyncSocket()->m_boolAsyncCallBlocking = false;

		if ( GetAsyncSocket()->IsCanceled() )
		{
			GetAsyncSocket()->m_iState = ChAsyncChacoSocket::stateDisconnected;	
		}  
		// Check if we are connected
		boolSuccess = GetAsyncSocket()->m_iState == ChAsyncChacoSocket::stateConnected;
		 
	}
	return boolSuccess;
}


#if defined( CH_MSW )

void ChChacoSocket::InitWinsockNotification( HINSTANCE hInstance )
{
	if (m_hwndWSA == 0)
	{
		WNDCLASS	wc;
		ChString		strProduct;

		wc.hInstance = hInstance;
		wc.lpszClassName = CH_WSA_NOTIF_CLASS;
		wc.lpfnWndProc	= SocketsXXWSAproc;
		wc.hCursor = 0;
		wc.hIcon = 0;
		wc.hbrBackground = 0;
		wc.lpszMenuName = 0;
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		if (RegisterClass( &wc ))
		{
			m_hwndWSA =
				CreateWindow( CH_WSA_NOTIF_CLASS, "Chaco WSA Window",
								WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
								CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								0, 0, hInstance, 0 );
			ASSERT( m_hwndWSA );
		}

		#if defined( CH_VRML_VIEWER )
		{
			strProduct = CH_VRML_PRODUCT_NAME;
		}
		#else
		{
			strProduct = CH_PRODUCT_NAME;	// For pueblo
		}
		#endif

		UpdateSocketPreferences( strProduct );
	}
}


void ChChacoSocket::UpdateSocketPreferences( const ChString& strProduct )
{
	bool boolUseProxies;

	ChRegistry regProxy( CH_COMPANY_NAME, strProduct, CH_PROXIES_GROUP );

	regProxy.ReadBool( CH_PROXIES, boolUseProxies, CH_PROXIES_DEF );

	if ( boolUseProxies )
	{

		ChString strHost;
		regProxy.Read( CH_SOCKS_PROXY, strHost );

											// Check if there is any change
		if (strHost != m_strSocksHostName)
		{									/* We need to reinitialize the IP
												addr, this will be done when
												the next connect request
												occurs */
			m_boolSocksAddrInited = false;
		}

		m_strSocksHostName = strHost;
		regProxy.Read( CH_SOCKS_PROXY_PORT, m_luSocksPort );
	}
	else
	{
		m_strSocksHostName.Empty();
		m_luSocksPort = 0;
	}

}


void ChChacoSocket::AsyncGetHostByName( const char *pstrHostName,
										ChSocketAsyncHandler pprocHandler,
										chparam userData )
{
	sockasyncinfo* pAsyncInfo = new sockasyncinfo( MAXGETHOSTSTRUCT,
													pprocHandler, userData );

	ASSERT( pAsyncInfo );

	sockinetaddr sa;	

	if ( long(sa.m_sockAddrIn.sin_addr.s_addr = inet_addr( pstrHostName )) == -1)
	{
		HANDLE	hWaitOn = WSAAsyncGetHostByName( ChChacoSocket::GetWSAWnd(),
													CH_MSG_WSA_PACKET_RECEIVED,
													pstrHostName,
													pAsyncInfo->GetBuf(),
													MAXGETHOSTSTRUCT );

		if (hWaitOn)
		{							
			pAsyncInfo->Valid();
									/* Wait for the async event to
													occur */

			sockasyncinfo::m_reqList.Insert( (chparam)hWaitOn, pAsyncInfo );
		}
		else
		{										/* Call failed notify error
													asyncronously */
			sockasyncinfo::m_reqList.Insert( (chparam)pAsyncInfo, pAsyncInfo );

			::PostMessage( ChChacoSocket::GetWSAWnd(), CH_MSG_WSA_NOTIFY_EVENT,
							0, (LPARAM)(sockasyncinfo*)pAsyncInfo );
		}
	}
	else
	{
		delete pAsyncInfo;
		sa.m_sockAddrIn.sin_family = sockinetbuf::af_inet;
		// This will convert the sockinetaddr to host entry and do a async notification
		ChChacoSocket::AsyncGetHostByName( sa,	pprocHandler,	userData );

	}
}


void ChChacoSocket::AsyncGetHostByName( const sockinetaddr& sa,
										ChSocketAsyncHandler pprocHandler,
										chparam userData )
{
	sockasyncinfo* pAsyncInfo = new sockasyncinfo( MAXGETHOSTSTRUCT,
													pprocHandler, userData );

	ASSERT( pAsyncInfo );

  	hostent * pHostEntry = (hostent *)pAsyncInfo->GetBuf();

	char* pstrAddr = pAsyncInfo->GetBuf() + sizeof( hostent ); 

	pHostEntry->h_addr_list = (char**)(pstrAddr + sizeof( sa.m_sockAddrIn.sin_addr ));

	pHostEntry->h_length = sizeof( sa.m_sockAddrIn.sin_addr );
	memcpy( pstrAddr, &sa.m_sockAddrIn.sin_addr, 
					sizeof( sa.m_sockAddrIn.sin_addr ) );
	pHostEntry->h_addr = pstrAddr;
	pHostEntry->h_addrtype = sa.m_sockAddrIn.sin_family;

	pAsyncInfo->SetParams( 0, 0 );

	sockasyncinfo::m_reqList.Insert( (chparam)pAsyncInfo, pAsyncInfo );
									/* notify asyncronously */
	::PostMessage( ChChacoSocket::GetWSAWnd(), CH_MSG_WSA_NOTIFY_EVENT,
									0, (LPARAM)(sockasyncinfo*)pAsyncInfo );
}



bool ChChacoSocket::PumpMessages( HANDLE hWaitOn )
{
	ASSERT( ChChacoSocket::m_hwndWSA );

	CWinThread*	pThread = AfxGetThread();
	LONG		lLastTick = ::GetMessageTime();

	while (hWaitOn && sockasyncinfo::m_boolBlocking)
	{
		{
			MSG		msg;

			if (::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ))
			{
				if (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
				{
					::DispatchMessage(&msg);
				}
				// allow user-interface updates
				pThread->OnIdle(-1);
			}
			else
			{
				// no work to do -- allow CPU to sleep
				WaitMessage();
			}
		}
		// Don't block for more than a minute
		LONG lCurrTime = ::GetMessageTime();

		if ( lLastTick == 0 || lCurrTime <  lLastTick )
		{ // check for wrap around or if last tick was zero because there was no
		  // messages for this app yet
			lLastTick = lCurrTime;
		}
		else if ( (lCurrTime - lLastTick ) > 60000 )
		{	// wait for atmost 1 minute
			break;
		}

	}

 	return TRUE;
}


void ChChacoSocket::InitSOCKSAddress()
{											// Get the socks server address
	if (IsAsyncSocket())
	{
		ChChacoSocket::m_saSOCKS.AsyncSetAddr( ChChacoSocket::m_strSocksHostName );
	}
	else
	{
		ChChacoSocket::m_saSOCKS.SetAddr( ChChacoSocket::m_strSocksHostName );
	}

	ChChacoSocket::m_saSOCKS.m_sockAddrIn.sin_port = 
			htons( (chuint16)ChChacoSocket::m_luSocksPort );

	ChChacoSocket::m_boolSocksAddrInited = true;
}

#endif

LONG CALLBACK EXPORT
SocketsXXWSAproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	#if defined( CH_PUEBLO_PLUGIN )
	{
											// Required to maintain MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	}
	#endif	// defined( CH_PUEBLO_PLUGIN )

	if (CH_MSG_WSA_PACKET_RECEIVED == uMsg)
	{

		sockasyncinfo	**ppInfo;
		sockasyncinfo	*pInfo;

		if (ppInfo = (sockasyncinfo::m_reqList.Find( (chparam) wParam)))
		{
			pInfo = *ppInfo;

			pInfo->SetParams( wParam, lParam );

			::PostMessage( hWnd, CH_MSG_WSA_NOTIFY_EVENT, 0, (LPARAM)wParam );
		}

		return 0;
	}
	else if (CH_MSG_WSA_NOTIFY_EVENT == uMsg)
	{
		sockasyncinfo	**ppInfo;
		sockasyncinfo	*pInfo;

		if (ppInfo = (sockasyncinfo::m_reqList.Find( (chparam)lParam)))
		{
			pInfo = *ppInfo;
			//delete the entry. This is required if they call
			// call cancel inside the handler.
			sockasyncinfo::m_reqList.Delete( (chparam)lParam );
			// we are out of blocking, can  more blocking calls now
	 		sockasyncinfo::StopBlocking();

			if (pInfo->GetHandler())
			{									// Call the event handler
				pInfo->GetHandler()( pInfo );
				delete pInfo;
			}


		}
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


/*----------------------------------------------------------------------------
	ChSyncChacoSocket class
----------------------------------------------------------------------------*/

ChSyncChacoSocket::ChSyncChacoSocket() :  m_iCount( 1 )
{
	m_hSocket = INVALID_SOCKET;
}


ChSyncChacoSocket::~ChSyncChacoSocket()
{
	if (m_hSocket != INVALID_SOCKET)
		Close();
}


/*----------------------------------------------------------------------------
	ChAsyncChacoSocket class
----------------------------------------------------------------------------*/

ChAsyncChacoSocket::ChAsyncChacoSocket( ChSocketHandler pHandler,
										chparam userData,
										sockinetbuf* socinet ) :
						CAsyncSocket(),
						m_userData( userData ),
						m_pHandler( pHandler ),
						m_sockInet( socinet ),
						m_iState( stateUnknown ),
						m_iSOCKSStatus( 0 ),
						m_boolCanceled( false ),
						m_boolAsyncCallBlocking( 0 )
{
}


ChAsyncChacoSocket::~ChAsyncChacoSocket()
{
}


void ChAsyncChacoSocket::OnAccept( int nErrorCode )
{
	CAsyncSocket::OnAccept( nErrorCode );

	if ( ProcessBlocking( nErrorCode ) )
	{
		ASSERT( m_pHandler );
		(m_pHandler)( *m_sockInet, CH_SOCK_EVENT_ACCEPT, nErrorCode );
	}
}


void ChAsyncChacoSocket::OnClose( int nErrorCode )
{
	CAsyncSocket::OnClose( nErrorCode );

	if ( ProcessBlocking( nErrorCode ) )
	{
		ASSERT( m_pHandler );

		// UE: moved this line to before calling the handler; the handler has a high tendency to
		//     destory this object!
		m_iState = stateDisconnected;

		(m_pHandler)( *m_sockInet, CH_SOCK_EVENT_CLOSE, nErrorCode );
		// UE: the handler will most likely delete this object, so do NOT make any references
		//     through 'this' (not even implicitly!) after this point.
	}
}


void ChAsyncChacoSocket::OnConnect( int nErrorCode )
{
	CAsyncSocket::OnConnect( nErrorCode );

	ProcessBlocking( nErrorCode );
}


void ChAsyncChacoSocket::OnOutOfBandData( int nErrorCode )
{
	CAsyncSocket::OnOutOfBandData( nErrorCode );

	if ( ProcessBlocking( nErrorCode ) )
	{
		ASSERT( m_pHandler );
		(m_pHandler)( *m_sockInet, CH_SOCK_EVENT_OOB_READ, nErrorCode );
	}
}


void ChAsyncChacoSocket::OnReceive( int nErrorCode )
{
	CAsyncSocket::OnReceive( nErrorCode );

	if ( ProcessBlocking( nErrorCode ) )
	{
		if ( stateWaitingForSOCKS == m_iState)
		{
			m_iState = stateConnected;

			unsigned char	socksPacket[10];
			int				iRet = Receive( socksPacket, 8 );

			m_iSOCKSStatus = socksPacket[1];
												// Stop blocking
			sockasyncinfo::StopBlocking();
		}
		else
		{
			ASSERT( m_pHandler );
			(m_pHandler)( *m_sockInet, CH_SOCK_EVENT_READ, nErrorCode );
		}
	}
}

#if 0
BOOL ChAsyncChacoSocket::OnMessagePending( )
{
	MSG msg;
	if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		::TranslateMessage (&msg);
		::DispatchMessage(&msg);
		return TRUE;	
	}
	return CSocket::OnMessagePending();
}
#endif

BOOL ChAsyncChacoSocket::ProcessBlocking( int iError )
{
	if ( stateConnected == m_iState || stateWaitingForSOCKS == m_iState )
	{	  
		return true;
	}
	else if ( stateWaitingForConnect == m_iState )
	{
		if ( m_boolCanceled || iError )
		{
			iError = iError ? iError : WSAEINTR;
			m_iState = stateDisconnected;
		}
		else
		{
			m_iState = stateConnected;
		}
		// stop pumping messages
		m_boolAsyncCallBlocking = false;

		ASSERT( m_pHandler );
		(m_pHandler)( *m_sockInet, CH_SOCK_EVENT_CONNECT, iError );

	}
	else if ( stateDisconnected == m_iState )
	{
		return false;
	}

	return true;

}

bool ChAsyncChacoSocket::PumpMessages( HANDLE hWaitOn )
{
	CWinThread*	pThread = AfxGetThread();
	LONG		lLastTick = ::GetMessageTime();

	while (hWaitOn && m_boolAsyncCallBlocking )
	{
		{
			MSG		msg;

			if (::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ))
			{
				if (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
				{
					::DispatchMessage(&msg);
				}
				// allow user-interface updates
				pThread->OnIdle(-1);
			}
			else
			{
				// no work to do -- allow CPU to sleep
				WaitMessage();
			}
		}
		// Don't block for more than a minute
		LONG lCurrTime = ::GetMessageTime();

		if ( lLastTick == 0 || lCurrTime <  lLastTick )
		{ // check for wrap around or if last tick was zero because there was no
		  // messages for this app yet
			lLastTick = lCurrTime;
		}
		else if ( (lCurrTime - lLastTick ) > 60000 )
		{	// wait for atmost 1 minute
			break;
		}

	}

 	return TRUE;
}



IMPLEMENT_DYNAMIC( ChAsyncChacoSocket, CAsyncSocket )


////////////////////////////////////////////////////////////////////////
///////////// sockinet buf


// Copy constructor
sockinetbuf::sockinetbuf( const sockinetbuf& si ) :
				sockbuf( si )

{
}



sockinetbuf::sockinetbuf( sockbuf::type ty, int proto ) :
				sockbuf( af_inet, ty, proto )
{
}


// Create an async socketbuf. Receives read, oob, and close notifcation in the handler callback

sockinetbuf::sockinetbuf( sockbuf::type ty, ChSocketHandler pHandler,
							chparam userData, chint16 sProto ) :
							sockbuf( af_inet, ty, pHandler, userData, sProto )

{
	m_pSocket->SetNotificationInfo( this );
}

sockinetbuf::~sockinetbuf ()
{
}



/*----------------------------------------------------------------------------

	FUNCTION	||	sockinetbuf::SetUserData

------------------------------------------------------------------------------

	This methods sets the user data for the specified socket and for the
	corresponding cached socket, if there is one.

----------------------------------------------------------------------------*/

void sockinetbuf::SetUserData( chparam userData )
{
	m_pSocket->SetUserData( userData );
}


sockinetbuf& sockinetbuf::operator =( const sockinetbuf& inetSocket )
{
											// Copy the member variables
	return *this;
}

sockinetaddr sockinetbuf::localaddr() const
{
	sockinetaddr	sin;
	int				len = sin.GetSize();

	if (!m_pSocket->GetSockName(  sin.GetAddr(), &len) )
	{
		#if !defined( CH_ARCH_16 )
  		perror("sockinetbuf::localaddr()");
  		#endif
  	}
	return sin;
}

int sockinetbuf::localport() const
{
	sockinetaddr	sin;
	int				len = sin.GetSize();

	if (!m_pSocket->GetSockName(  sin.GetAddr(), &len) )
	{
		#if !defined( CH_ARCH_16 )
  		perror("sockinetbuf::localport()");
  		#endif
  	}

	if (sin.GetFamily() != af_inet) return -1;

	return sin.GetPort();
}

const char* sockinetbuf::localhost() const
{
	sockinetaddr sin = localaddr();
	if (sin.GetFamily() != af_inet) return "";
	return sin.GetHostname();
}


sockinetaddr sockinetbuf::peeraddr() const
{
	sockinetaddr sin;
	int len = sin.GetSize();

	if (!m_pSocket->GetPeerName(  sin.GetAddr(), &len) )
	{
		#if !defined( CH_ARCH_16 )
  		perror("sockinetbuf::peeraddr()");
  		#endif
  	}
	return sin;
}

int sockinetbuf::peerport() const
{
	sockinetaddr sin = peeraddr();
	if (sin.GetFamily() != af_inet) return -1;
	return sin.GetPort();
}

const char* sockinetbuf::peerhost() const
{
	sockinetaddr sin = peeraddr();
	if (sin.GetFamily() != af_inet) return "";
	return sin.GetHostname();
}

sockbuf* sockinetbuf::open(sockbuf::type st, int proto)
{
	*this = sockinetbuf(st, proto);
	return this;
}


int sockinetbuf::bind( sockAddr& sa )
{
    return sockbuf::bind( sa );
}

int sockinetbuf::bind()
{
    sockinetaddr	sa;

    return bind( sa );
}

int sockinetbuf::bind( unsigned long addr, int port_no )
{
											/* Address and portno are in host
												byte order */
    sockinetaddr	sa( addr, port_no );

    return bind( sa );
}

int sockinetbuf::bind( const char* host_name, int port_no )
{
    sockinetaddr	sa( host_name, port_no, m_pSocket->IsAsyncSocket() );

    return bind( sa );
}

int sockinetbuf::bind( unsigned long addr, const char* service_name,
						const char* protocol_name )
{
	sockinetaddr	sa( addr, service_name, protocol_name,
						m_pSocket->IsAsyncSocket() );

	return bind( sa );
}

int sockinetbuf::bind( const char* host_name, const char* service_name,
						const char* protocol_name )
{
    sockinetaddr	sa( host_name, service_name, protocol_name,
    						m_pSocket->IsAsyncSocket() );

    return bind( sa );
}

// Note that these are all blocking connects
void sockinetbuf::connect (sockAddr& sa)
{
    sockbuf::connect( sa );
}

void sockinetbuf::connect (unsigned long addr, int port_no)
     // address and portno are in host byte order
{
    sockinetaddr sa (addr, port_no);

    connect (sa);
}

void sockinetbuf::connect (const char* host_name, int port_no)
{
    sockinetaddr sa (host_name, port_no, m_pSocket->IsAsyncSocket());

    connect (sa);
}

void sockinetbuf::connect (unsigned long addr,
			const char* service_name,
			const char* protocol_name)
{
    sockinetaddr sa (addr, service_name, protocol_name,
    								m_pSocket->IsAsyncSocket());

    connect (sa);
}

void sockinetbuf::connect (const char* host_name,
			const char* service_name,
			const char* protocol_name)
{
    sockinetaddr	sa( host_name, service_name, protocol_name,
    										m_pSocket->IsAsyncSocket() );

    connect( sa );
}

int sockinetbuf::is_readready (int wp_sec, int wp_usec)	const
{
	return (sockbuf::is_readready ( wp_sec, wp_usec) != 0);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	sockinetbuf::GetBytesAvailable

	RETURNS		||	Number of bytes available to read on the socket.

------------------------------------------------------------------------------

	This method will return the number of bytes available to be read on
	a socket, -without- doing a select.

----------------------------------------------------------------------------*/

chuint32 sockinetbuf::GetBytesAvailable( void ) const
{
	return(sockbuf::is_readready ( 0, 0 ));
}


/*----------------------------------------------------------------------------
	sockbuf class
----------------------------------------------------------------------------*/

sockbuf::sockbuf( SOCKET soc ) :
			stmo( -1 ),
			rtmo( -1 )
{
	m_pSocket = new ChChacoSocket();
	ASSERT( m_pSocket );

	m_pSocket->Attach( soc );

	#if defined( _S_NOLIBGXX )
	{
		xflags( 0 );
	}
	#endif

    xsetflags( _S_LINE_BUF );
}


sockbuf::sockbuf( int domain, sockbuf::type st, int proto ) :
			stmo( -1 ),
			rtmo( -1 )
{
	m_pSocket = new ChChacoSocket();
	ASSERT( m_pSocket );
											// create a socket
	m_pSocket->Create( 0, st );

	#if defined( _S_NOLIBGXX )
	{
	    xflags( 0 );
	}
	#endif

	xsetflags( _S_LINE_BUF );
}

sockbuf::sockbuf( int domain, type st, ChSocketHandler pHandler,
						chparam userData, chint16 proto ) :
			stmo( -1 ),
			rtmo( -1 )
{
	m_pSocket = new ChChacoSocket( pHandler, userData, this );
	ASSERT( m_pSocket );
											// Create a socket
	m_pSocket->Create( 0, st );

	#if defined( _S_NOLIBGXX )
	{
	    xflags( 0 );
	}
	#endif

    xsetflags( _S_LINE_BUF );
}


sockbuf::sockbuf( const sockbuf& sb ) :
			m_pSocket( sb.m_pSocket ),
			stmo( sb.stmo ),
			rtmo( sb.rtmo )
{
	#if defined( _S_NOLIBGXX )
	{
		xflags( 0 );
	}
	#endif

    m_pSocket->m_iCount++;
    xsetflags( _S_LINE_BUF );
}

sockbuf& sockbuf::operator =( const sockbuf& sb )
{
	if ((this != &sb) && (m_pSocket->GetSocket() != sb.m_pSocket->GetSocket()))
	{
		m_pSocket->Close();
		m_pSocket->Attach( sb.m_pSocket->GetSocket() );
		stmo = sb.stmo;
		rtmo = sb.rtmo;

		#if defined( _S_NOLIBGXX )
		{
			xflags (sb.xflags ());
		}
		#else
		{									/* xflags () is a non-const member
												function in libg++ */
			xflags (((sockbuf&)sb).xflags ());
		}
		#endif
	}

	return *this;
}

sockbuf::~sockbuf ()
{
    overflow (EOF);
    if (m_pSocket->m_iCount <= 1 )
    {
    	delete m_pSocket;
	}
	else
	{
		m_pSocket->m_iCount--;
	}

	// UE: practically certain this is part of the (incompatible) buffering system
	//    delete [] base ();
}

sockbuf* sockbuf::open (type, int)
{
    return 0;
}

sockbuf* sockbuf::close()
{
	if ( m_pSocket->GetSocket() != INVALID_SOCKET )
	{
		m_pSocket->Close();
		m_pSocket->m_iCount = -1;
		return this;
	}

	return 0;
}

/*	// UE: both these routines appear to be related both to a buffering system and
		// 		 to UNIX sockets, which are incompatible with Win32 anyway.
size_t sockbuf::sys_read (char* buf, size_t len)
     // return EOF on eof, 0 on timeout, and # of chars read on success
{
    return read (buf, len);
}

size_t sockbuf::sys_write (const void* buf, long len)
     // return written_length; < len indicates error
{
    return write (buf, (int) len);
}

// UE: Oh, and this one's out for the same reason :)
int sockbuf::flush_output()
     // return 0 when there is nothing to flush or when the flush is a success
     // return EOF when it could not flush
{
    if (pptr () <= pbase ()) return 0;
    if (!(xflags () & _S_NO_WRITES)) {
	int wlen   = pptr () - pbase ();
	int wval   = sys_write (pbase (), wlen);
	int status = (wval == wlen)? 0: EOF;
	if (unbuffered())
		setp (pbase (), pbase ());
	else
		setp (pbase (), pbase () + BUFSIZ);
	return status;
    }
    return EOF;
}
*/

int sockbuf::sync ()
{
	// UE: As we aren't buffering these, sync() has nothing to do.
//    return flush_output ();
	return 0;
}

/*	// UE: This is related to the (defunct) buffering scheme.
int sockbuf::doallocate ()
     // return 1 on allocation and 0 if there is no need
{
    if (!base ()) {
	char*	buf = new char[2*BUFSIZ];
	setb (buf, buf+BUFSIZ, 0);
	setg (buf, buf, buf);

	buf += BUFSIZ;
	setp (buf, buf+BUFSIZ);
	return 1;
    }
    return 0;
}
*/

int sockbuf::underflow ()
{
// UE: Well, this function is supposed to read the next character from the buffer
//     and return it; this would be kinda like a 1-byte receive.  Since we aren't
//     buffering this would just be a pain in the neck, so disable it for now.  If
//     I discover it was necessary I'll have to put something back in, but for now
//     I think it's pretty much redundant.
	TRACE( "in sockbuf::underflow\n" );
/*
    if (xflags () & _S_NO_READS) return EOF;

    if (gptr () < egptr ()) return *(unsigned char*)gptr ();

    if (base () == 0 && doallocate () == 0) return EOF;

    int bufsz = unbuffered () ? 1: BUFSIZ;
    int rval = sys_read (base (), bufsz);
    if (rval == EOF) {
	xsetflags (_S_EOF_SEEN);
	return EOF;
    }else if (rval == 0)
	return EOF;
    setg (eback (), base (), base () + rval);
    return *(unsigned char*)gptr ();
*/
	return EOF;
}

int sockbuf::overflow (int c)
     // if c == EOF, return flush_output();
     // if c == '\n' and linebuffered, insert c and
     // return (flush_output()==EOF)? EOF: c;
     // otherwise insert c into the buffer and return c
{
    if (c == EOF) return 0;	//flush_output ();	// 0 on success

	// UE: As for the rest of this, well, like underflow I can't see the point.
	//     The task at this point devolves to a 1-byte send.  Again, I think
	//     that it's pretty much redundant.
	TRACE( "in sockbuf::overflow\n" );
/*
    if (xflags () & _S_NO_WRITES) return EOF;

    if (pbase () == 0 && doallocate () == 0) return EOF;

    xput_char (c);
    if ((unbuffered () || linebuffered () && c == '\n' || pptr () >= epptr ())
	&& flush_output () == EOF)
	return EOF;

    return c;
*/
	return EOF;
}

/*	// The only significant functionality this added was linebuffering; I think
		// that it can be got along without for the moment :)
int sockbuf::xsputn (const char* s, int n)
{
    if (n <= 0) return 0;
    const unsigned char* p = (const unsigned char*)s;

    for (int i=0; i<n; i++, p++) {
	if (*p == '\n') {
	    if (overflow (*p) == EOF) return i;
	} else
	    if (sputc (*p) == EOF) return i;
    }
    return n;
}
*/

int sockbuf::bind (sockAddr& sa)
{
	int ret;

	ret = m_pSocket->Bind( sa.GetAddr(), sa.GetSize());
	if (!ret)
		error ("sockbuf::bind");
	return ret;
}

void sockbuf::connect( sockAddr& sa )
{


	if ( !m_pSocket->Connect( sa.GetAddr(), sa.GetSize()) )
	{
		#if defined( CH_MSW ) && defined( _DEBUG )
			WSADisplayError( WSAGetLastError(), "sockbuf::connect" );
		#endif	// defined( CH_MSW ) && defined( _DEBUG )

		error ( "sockbuf::connect" );

		#if defined( CH_EXCEPTIONS )
		{
			switch( WSAGetLastError() )
			{
				case WSAETIMEDOUT:
				{							// Throw a timed-out exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{

						THROW( new ChSocketEx( ChEx::socketTimedOut ));
					}
					#else
					throw ChSocketEx( ChEx::socketTimedOut );
					#endif
					//break;
				}

				case WSAEINPROGRESS :
				{

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::inProgress ));
					}
					#else
					throw ChSocketEx( ChEx::inProgress );
					#endif
					//break;
				}
				default:
				{							// Throw a misc. failure exception

					#if defined( CH_MSW) && defined( CH_ARCH_16 )
					{
						THROW( new ChSocketEx( ChEx::connectFailed ));
					}
					#else
					throw ChSocketEx( ChEx::connectFailed );
					#endif
					//break;
				}
			}
		}
		#endif
	}
}

void sockbuf::listen (int num)
{
    if ( !m_pSocket->Listen( num) )
		error ("sockbuf::listen");
}

sockbuf	sockbuf::accept (sockAddr& sa)
{
    int len = sa.GetSize();
	SOCKET soc;
	if ( (soc = m_pSocket->Accept( sa.GetAddr(), &len )) == INVALID_SOCKET )
	{
		error ("sockbuf::accept");
	}
	return soc;
}

sockbuf	sockbuf::accept ()
{

	SOCKET soc;
	if ( (soc = m_pSocket->Accept( )) != INVALID_SOCKET )
	{
		error ("sockbuf::accept");
	}
	return soc;
}

int sockbuf::read (void* buf, int len)
{
    if (rtmo != -1 && is_readready (rtmo)==0) return 0;

    int	rval;

	if ((rval = m_pSocket->Receive( buf, len ) ) )
		error("sockbuf::read");

    return ((rval==0) ? EOF: rval);
}

int sockbuf::recv (void* buf, int len, int msgf)
{
    if (rtmo != -1 && is_readready (rtmo)==0) return 0;

    int	rval;
	if ((rval = m_pSocket->Receive( buf, len, msgf ) ) )
		error ("sockbuf::recv");

    return (rval==0) ? EOF: rval;
}

int sockbuf::recvfrom (sockAddr& sa, void* buf, int len, int msgf)
{
    if (rtmo != -1 && is_readready (rtmo)==0) return 0;

    int	rval;
    int	sa_len = sa.GetSize();

    if ((rval = m_pSocket->ReceiveFrom( buf, len,
	                    sa.GetAddr(), &sa_len, msgf)) )
	error ("sockbuf::recvfrom");
    return (rval==0) ? EOF: rval;
}

int sockbuf::write(const void* buf, int len) const
{
    if (stmo != -1 && is_writeready (stmo)==0) return 0;

    int	wlen=0;

    while(len>0)
    {
		int	wval;

		wval = m_pSocket->Send(((char*)buf + wlen), len);
		if (wval == SOCKET_ERROR )
		{
			int error_val = WSAGetLastError();
			error ("sockbuf::write");
			#if defined( CH_VERBOSE )
			cerr << "WSAGetLastError() == " << error_val << endl;
			#endif
			return wval;
		}
		len -= wval;
		wlen += wval;
    }
    return wlen; // == len if every thing is all right
}

int sockbuf::send (const void* buf, int len, int msgf)
{
    if (stmo != -1 && is_writeready (stmo)==0) return 0;

    int	wlen=0;

    while(len>0)
    {
		int	wval;

		wval = m_pSocket->Send( ((char*)buf + wlen), len, msgf );
		if (wval == SOCKET_ERROR )
		{
			int error_val = WSAGetLastError();
			error ("sockbuf::write");
			#if defined( CH_VERBOSE )
			cerr << "WSAGetLastError() == " << error_val << endl;
			#endif
			return wval;
		}
		len -= wval;
		wlen += wval;
    }
    return wlen; // == len if every thing is all right
}

int sockbuf::sendto (sockAddr& sa, const void* buf, int len, int msgf)
{
    if (stmo != -1 && is_writeready (stmo)==0) return 0;

    int	wlen=0;

    while(len>0)
    {
		int	wval;

		wval = m_pSocket->SendTo( ((char*)buf + wlen), len,
							sa.GetAddr(), sa.GetSize(), msgf);
		if (wval == SOCKET_ERROR )
		{
			int error_val = WSAGetLastError();
			error ("sockbuf::write");
			#if defined( CH_VERBOSE )
			cerr << "WSAGetLastError() == " << error_val << endl;
			#endif
			return wval;
		}
		len -= wval;
		wlen += wval;
    }
    return wlen; // == len if every thing is all right
}


int sockbuf::sendtimeout (int wp)
{
    int oldstmo = stmo;
    stmo = (wp < 0) ? -1: wp;
    return oldstmo;
}

int sockbuf::recvtimeout (int wp)
{
    int oldrtmo = rtmo;
    rtmo = (wp < 0) ? -1: wp;
    return oldrtmo;
}

int sockbuf::is_readready (int wp_sec, int wp_usec) const
{
  	DWORD luBytes;
	if ( m_pSocket->IOCtl(  FIONREAD, &luBytes ) )
	{
		return (int)luBytes;
	}
	return( 0 );
}

int sockbuf::is_writeready (int wp_sec, int wp_usec) const
{
  	DWORD luBytes;
	if ( m_pSocket->IOCtl(  FIONREAD, &luBytes ) )
	{
		return (int)luBytes;
	}
	return( 0 );
}

int sockbuf::is_exceptionpending (int wp_sec, int wp_usec) const
{
    return 0;
}

void sockbuf::shutdown (shuthow sh)
{
    switch (sh) {
    case shut_read:
	xsetflags(_S_NO_READS);
	break;
    case shut_write:
	xsetflags(_S_NO_WRITES);
	break;
    case shut_readwrite:
	xsetflags(_S_NO_READS|_S_NO_WRITES);
	break;
    }

    if (!m_pSocket->ShutDown( sh))
		error("sockbuf::shutdown");
}

int sockbuf::getopt (option op, void* buf, int len, int l) const
{
    int	rlen = len;
    if ( !m_pSocket->GetSockOpt ( op, buf, &rlen, l))
    {
		#if !defined( CH_ARCH_16 )
  		perror ("sockbuf::getopt");
  		#endif
	}
    return rlen;
}

void sockbuf::setopt (option op, void* buf, int len, int l) const
{
    if ( !m_pSocket->SetSockOpt ( op, buf, len, l))
    {
		#if !defined( CH_ARCH_16 )
	  	perror ("sockbuf::setopt");
	  	#endif
	}
}

sockbuf::type sockbuf::gettype () const
{
    int	ty=0;
    int	rlen = sizeof( ty );
    m_pSocket->GetSockOpt ( SO_TYPE, &ty, &rlen);
    return sockbuf::type(ty);
}

int sockbuf::clearerror () const
{
    int 	err=0;
    m_pSocket->SetSockOpt ( SO_ERROR, &err, sizeof (err) );
    return err;
}

int sockbuf::debug (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );

    m_pSocket->GetSockOpt ( SO_DEBUG, (void*)old, &rlen );
    if (old != -1)
	m_pSocket->SetSockOpt (SO_DEBUG, &opt, sizeof (opt));

    return old;
}

int sockbuf::reuseaddr (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );
	 m_pSocket->GetSockOpt( SO_REUSEADDR, &old, &rlen);
    if (opt != -1)
		m_pSocket->SetSockOpt (SO_REUSEADDR, &opt, sizeof (opt));
    return old;
}

int sockbuf::keepalive (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt(SO_KEEPALIVE, &old, &rlen);
    if (opt != -1)
	 m_pSocket->SetSockOpt( SO_KEEPALIVE, &opt, sizeof (opt));
    return old;
}

int sockbuf::dontroute (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt (SO_DONTROUTE, &old, &rlen);
    if (opt != -1)
 		m_pSocket->SetSockOpt (SO_DONTROUTE, &opt, sizeof (opt));
   return old;
}

int sockbuf::broadcast (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt (SO_BROADCAST, &old, &rlen);
    if (opt != -1)
		m_pSocket->SetSockOpt (SO_BROADCAST, &opt, sizeof (opt));
    return old;
}

int sockbuf::oobinline (int opt) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt (SO_OOBINLINE, &old, &rlen);
    if (opt != -1)
	m_pSocket->SetSockOpt (SO_OOBINLINE, &opt, sizeof (opt));
    return old;
}

int sockbuf::linger (int opt) const
{
    socklinger	old (0, 0);
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt (SO_LINGER, &old, &rlen);
    if (opt > 0)
    {
		socklinger nw (1, opt);
		m_pSocket->SetSockOpt (SO_LINGER, &nw, sizeof (nw));
    }
    else if (opt == 0)
    {
		socklinger nw (0, old.l_linger);
		m_pSocket->SetSockOpt  (SO_LINGER, &nw, sizeof (nw));
    }
    return old.l_onoff ? old.l_linger: -1;
}

int sockbuf::sendbufsz (int  sz) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt  (SO_SNDBUF, &old, &rlen);
    if (sz >= 0)
	m_pSocket->SetSockOpt (SO_SNDBUF, &sz, sizeof (sz));
    return old;
}

int sockbuf::recvbufsz (int sz) const
{
    int old=0;
    int	rlen = sizeof( old );
    m_pSocket->GetSockOpt (SO_RCVBUF, &old, &rlen);
    if (sz >= 0)
	m_pSocket->SetSockOpt  (SO_RCVBUF, &sz, sizeof (sz));
    return old;
}

void sockbuf::error (const char* msg) const
{
    sock_error ("class sockbuf: ", msg);
}

#if 0
void isockstream::error (const char* msg) const
{
    sock_error ("class isockstream: ", msg);
}

void osockstream::error (const char* msg) const
{
    sock_error ("class osockstream: ", msg);
}

void iosockstream::error( const char* msg ) const
{
    sock_error ("class iosockstream: ", msg);
}
#endif


CH_INTERN_FUNC( void )
WSADisplayError( int iError, const char* pstrContext )
{
	ChString		strErrorMsg;

	ChChacoSocket::GetErrorMsg( iError, strErrorMsg );
	TRACE2( "%s: %s", (const char*)strErrorMsg, pstrContext );
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:54:41  uecasm
// Import of source tree as at version 2.53 release.
//
