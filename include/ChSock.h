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

	Portions copyright (C) 1992,1993,1994 Gnanasekaran Swaminathan
											<gs4t@virginia.edu>

	Permission is granted to use at your own risk and distribute this software
	in source and binary forms provided the above copyright notice and this
	paragraph are preserved on all copies.  This software is provided "as is"
	with no express or implied warranty.

	Version: 21May94 1.7

------------------------------------------------------------------------------

	This file consists of the interface for the Internet Sockets++ classes.

----------------------------------------------------------------------------*/

// $Header$

#ifndef _CHSOCKET_H
#define	_CHSOCKET_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif


#include <ChTypes.h>
#include <ChSplay.h>
#include <afxext.h>         // MFC extensions
#include <afxsock.h>

#include <iostream>



#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Event constants
----------------------------------------------------------------------------*/

#define CH_SOCK_EVENT_READ		FD_READ		// Socket is ready for reading
#define CH_SOCK_EVENT_OOB_READ	FD_OOB		/* Socket is ready for reading
												out-of-band data */
#define CH_SOCK_EVENT_CONNECT	FD_CONNECT	// Socket connection is complete
#define CH_SOCK_EVENT_CLOSE		FD_CLOSE	// Socket has been closed
#define CH_SOCK_EVENT_ACCEPT	FD_ACCEPT	// Socket ready for accepting connection


#       define _S_NOLIBGXX

#       define _S_USER_BUF		0x0001
#       define _S_UNBUFFERED		0x0002
#       define _S_NO_READS		0x0004
#       define _S_NO_WRITES		0x0008
#       define _S_EOF_SEEN		0x0010
#       define _S_ERR_SEEN		0x0020
#       define _S_DELETE_DONT_CLOSE	0x0040
#       define _S_LINKED		0x0080
#       define _S_LINE_BUF		0x0200

//#       define _G_ssize_t size_t

#       define _S_IOS_SETF(x) (x = 1)
#       define _S_IOS_UNSETF(x) (x=0)

#define CH_DECLARE_SOCKET_ASYNC_HANDLER( name ) \
				CH_EXTERN_CALLBACK( void ) \
				name( sockasyncinfo* pInfo );

#define CH_IMPLEMENT_SOCKET_ASYNCHANDLER( name ) \
				CH_GLOBAL_CALLBACK( void ) \
				name( sockasyncinfo* pInfo  )

#define CH_FRIEND_SOCKET_ASYNCHANDLER( name ) \
				friend CH_GLOBAL_CALLBACK( void ) \
				name( sockasyncinfo* pInfo  );


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/
class sockinetbuf;
class sockbuf;
class sockasyncinfo;

typedef sockinetbuf	*psockinetbuf;

//typedef void (CALLBACK EXPORT *ChSocketHandler)( sockinetbuf& si,
//													chparam luEvent,
//													int iErrorCode );
CH_TYPEDEF_CALLBACK(void, ChSocketHandler)( sockinetbuf& si,
													chparam luEvent,
													int iErrorCode );

//typedef void (CALLBACK EXPORT *ChSocketAsyncHandler)( sockasyncinfo* pInfo );
CH_TYPEDEF_CALLBACK(void, ChSocketAsyncHandler)( sockasyncinfo* pInfo );


class CH_EXPORT_CLASS sockasyncinfo
{
	public :
		sockasyncinfo( int iBufSize, ChSocketAsyncHandler pHandler,
						chparam userData ) :
					m_pprocHandler( pHandler ),
					m_userData( userData ),
					m_boolValid( false ),
					m_pBuf( 0 ),
					m_wParam( 0 ),
					m_lParam( -1 )
				{
					m_pBuf = new char[iBufSize];
					ASSERT( m_pBuf );
				}

		sockasyncinfo( int iBufSize ) :
					m_pprocHandler( 0 ),
					m_userData( 0 ),
					m_pBuf( 0 ),
					m_wParam( 0 ),
					m_lParam( -1 )
				{
					m_pBuf = new char[iBufSize];
					ASSERT( m_pBuf );
				}

		~sockasyncinfo()
				{
					if ( m_pBuf )
					{
						delete m_pBuf;
					}
				}

		void GetParams( WPARAM& wParam, LPARAM& lParam )
				{
					wParam = m_wParam;
					lParam = m_lParam;
				}
		void SetParams( WPARAM wParam, LPARAM lParam )
				{
					m_wParam = wParam;
					m_lParam = lParam;
				}

		char* GetBuf()
				{
					return m_pBuf;
				}
		chparam GetUserData()
				{
					return m_userData;
				}

		ChSocketAsyncHandler GetHandler()
				{
					return 	m_pprocHandler;
				}
		bool IsValid() const{ return m_boolValid; }
		void Valid() 		{ m_boolValid = true; } 

	private:
		ChSocketAsyncHandler	m_pprocHandler;
		char*					m_pBuf;
		WPARAM 					m_wParam;
		LPARAM					m_lParam;
		chparam 				m_userData;
		bool					m_boolValid;

	public:
		static bool	IsBlocking()
				{
					return m_boolBlocking;
				}

		static void	StartBlocking()
				{
					m_boolBlocking = true;
				}
		static void	StopBlocking()
				{
					m_boolBlocking = false;
				}

		static void CancelBlocking();

		static ChPtrSplay<sockasyncinfo>	m_reqList;
		static bool							m_boolBlocking;
};


typedef sockasyncinfo *psockasyncinfo;


/*----------------------------------------------------------------------------
	sockAddr class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS sockAddr
{
	public:
		sockAddr()
				{
					memset( &m_sockAddrIn, 0, sizeof( m_sockAddrIn ) );
				}
		virtual ~sockAddr() {}

		operator sockaddr*() const { return GetAddr(); }

		int GetSize() const { return sizeof( m_sockAddrIn ); }
		int GetFamily() const { return m_sockAddrIn.sin_family; }
		sockaddr* GetAddr() const { return (sockaddr *)((sockaddr_in *)&m_sockAddrIn); }

		void error( const char *pstrErr ) const;

		sockaddr_in 	m_sockAddrIn;
};



/*----------------------------------------------------------------------------
	sockinetaddr class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS sockinetaddr: public sockAddr
{
	public:
											// Lots of constructors...
		sockinetaddr();
		sockinetaddr( chuint32 luAddr, chint16 sPort = 0 );
		sockinetaddr( const sockinetaddr& sockINetAddr );

		// synchronous versions
		// If boolAsync is true then we pump messages till the call completes
		sockinetaddr( const char *pstrHostName, chint16 sPort = 0, bool boolAsync = false );
		sockinetaddr( chuint32 luAddr, const char *pstrServiceName,
						const char *pstrProtocolName = "tcp", bool boolAsync = false  );
		sockinetaddr( const char *pstrHostName, const char *pstrServiceName,
						const char *pstrProtocolName = "tcp", bool boolAsync = false  );


											// Public methods
		int	GetPort() const;
		const char *GetHostname() const;

		void SetPort( const char *pstrServiceName,
						const char *pstrProtocolName = "tcp" );
		void AsyncSetPort( const char *pstrServiceName,
						const char *pstrProtocolName = "tcp" );

		void AsyncSetAddr( const char *pstrHostName );
		void SetAddr( const char *pstrHostName );
};


/*----------------------------------------------------------------------------
	ChAsyncChacoSocket class
----------------------------------------------------------------------------*/

class ChAsyncChacoSocket : public CAsyncSocket
{
	DECLARE_DYNAMIC( ChAsyncChacoSocket );

	public:
		enum tagState { stateUnknown = 0, stateWaitingForConnect, 
						stateWaitingForSOCKS, stateConnected,
						stateDisconnected };

		ChAsyncChacoSocket( ChSocketHandler pHandler,
							chparam userData, sockinetbuf* socinet );

	public:
		chparam			m_userData;
		ChSocketHandler	m_pHandler;
		sockinetbuf*	m_sockInet;
		int				m_iState;
		int				m_iSOCKSStatus;
		bool			m_boolCanceled;
		bool			m_boolAsyncCallBlocking;

	public:
		void SetNotificationInfo( sockinetbuf* pInet ) { m_sockInet = pInet; }
		bool IsCanceled()			
				{
					return m_boolCanceled;
				}
		void CancelBlockingCall()
		{
			m_boolCanceled = true;
			m_boolAsyncCallBlocking = false;	
		}

		bool PumpMessages( HANDLE hWaitOn );
											// Overridable callbacks
	protected:
		virtual void OnAccept( int nErrorCode );
		virtual void OnClose( int nErrorCode );
		virtual void OnConnect( int nErrorCode );
		virtual void OnOutOfBandData( int nErrorCode );
		virtual void OnReceive( int nErrorCode );
		BOOL ProcessBlocking( int iError );
	

	public:
		virtual ~ChAsyncChacoSocket();
};


/*----------------------------------------------------------------------------
	ChSyncChacoSocket class
----------------------------------------------------------------------------*/

class ChSyncChacoSocket
{
	public:
		ChSyncChacoSocket();
	
		bool Create( UINT nSocketPort = 0, int nSocketType = SOCK_STREAM,
						LPCTSTR lpszSocketAddress = 0 )
				{
					ASSERT( m_hSocket == INVALID_SOCKET );

					m_hSocket = socket( PF_INET, nSocketType, 0 );
					if (m_hSocket != INVALID_SOCKET)
					{
						return true;
					}
					return false;
				}

	public:
		SOCKET		m_hSocket;
		int			m_iCount;

	public:
		bool Attach( SOCKET hSocket )
				{
					m_hSocket = hSocket;
					return true;
				}
		SOCKET Detach()
				{
					ASSERT( m_hSocket == INVALID_SOCKET );
					
					SOCKET		hTemp = m_hSocket;

					m_hSocket = INVALID_SOCKET;
					return hTemp;
				}

		bool GetPeerName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
			 	{
					return (getpeername( m_hSocket, lpSockAddr,
											lpSockAddrLen ) != SOCKET_ERROR);
				}

		bool GetSockName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
			 	{
					return (getsockname( m_hSocket, lpSockAddr,
											lpSockAddrLen ) != SOCKET_ERROR);
				}

		bool SetSockOpt( int nOptionName, const void* lpOptionValue,
							int nOptionLen, int nLevel = SOL_SOCKET )
				{
					return (setsockopt( m_hSocket, nLevel, nOptionName,
										(char*)lpOptionValue,
										nOptionLen  ) != SOCKET_ERROR);
				}
		bool GetSockOpt( int nOptionName, void* lpOptionValue,
							int* lpOptionLen, int nLevel = SOL_SOCKET )
			 	{
					return (getsockopt( m_hSocket, nLevel, nOptionName,
										(char*)lpOptionValue, lpOptionLen ) !=
											SOCKET_ERROR);
				}


		SOCKET Accept( SOCKADDR* lpAddr = 0, int* lpLen = 0 )
				{
					return accept( m_hSocket, lpAddr, lpLen );
				}
		bool Bind( const SOCKADDR* lpSockAddr, int nSockAddrLen )
				{
					return (bind( m_hSocket, lpSockAddr, nSockAddrLen ) !=
								SOCKET_ERROR);
				}

		virtual void Close()
				{
					if (m_hSocket != INVALID_SOCKET)
					{
						closesocket( m_hSocket );
					}
				}

		bool Connect( const SOCKADDR* lpSockAddr, int nSockAddrLen )
				{
					return (connect( m_hSocket, lpSockAddr, nSockAddrLen ) !=
								SOCKET_ERROR);
				}

		bool IOCtl( long lCommand, DWORD* lpArgument )
				{
					return (ioctlsocket( m_hSocket, lCommand, lpArgument ) !=
								SOCKET_ERROR );
				}

		bool Listen( int nConnectionBacklog = 5 )
				{
					return (listen( m_hSocket, nConnectionBacklog ) !=
								SOCKET_ERROR);
				}

		virtual int Receive( void* lpBuf, int nBufLen, int nFlags = 0 )
			   	{
					return recv( m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags );
				}

		int ReceiveFrom( void* lpBuf, int nBufLen, SOCKADDR* lpSockAddr,
							int* lpSockAddrLen, int nFlags = 0 )
				{
					return recvfrom( m_hSocket, (LPSTR)lpBuf, nBufLen,
										nFlags, lpSockAddr, lpSockAddrLen );
				}

		enum { receives = 0, sends = 1, both = 2 };

		bool ShutDown( int nHow = sends )
				{
					return (shutdown( m_hSocket, nHow ) != SOCKET_ERROR);
				}

		virtual int Send( const void* lpBuf, int nBufLen, int nFlags = 0 )
				{
					return send( m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags );
				}

		int SendTo( const void* lpBuf, int nBufLen, const SOCKADDR* lpSockAddr,
					int nSockAddrLen, int nFlags = 0 )
				{
					return sendto( m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags,
									lpSockAddr, nSockAddrLen );
				}

	public:
		virtual ~ChSyncChacoSocket();
};


/*----------------------------------------------------------------------------
	ChChacoSocket class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChChacoSocket
{
	public:
		ChChacoSocket();
		ChChacoSocket( ChSocketHandler pHandler, chparam userData,
						sockbuf* socinet );

		static void GetErrorMsg( int iError, ChString& strOutput );

	public:
		int		m_iCount;

	public:
		operator SOCKET() const
				{
					if (GetSyncSocket())
					{
						return( GetSyncSocket()->m_hSocket );
					}
					else
					{
						return( GetAsyncSocket()->m_hSocket );
					}
				}

		SOCKET GetSocket()
				{
					if (GetSyncSocket())
					{
						return( GetSyncSocket()->m_hSocket );
					}
					else
					{
						return( GetAsyncSocket()->m_hSocket );
					}
				}

		chparam	GetUserData()
				{
					if (GetSyncSocket())
					{
						return( 0 );
					}
					else
					{
						return( GetAsyncSocket()->m_userData );
					}
				}

		static void AsyncGetHostByName( const char *pstrHostName,
										ChSocketAsyncHandler pprocHandler,
										chparam userData );
		static void AsyncGetHostByName( const sockinetaddr& sa,
										ChSocketAsyncHandler pprocHandler,
										chparam userData );

		int	IsOpen()
				{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->m_hSocket != INVALID_SOCKET;
					}
					else
					{
						return GetAsyncSocket()->m_hSocket != INVALID_SOCKET;
					}
				}

		void SetUserData( chparam userData )
				{
					if (GetSyncSocket())
					{
						return;
					}
					else
					{
						GetAsyncSocket()->m_userData = userData;
					}
				}

	public:
		bool Create( UINT nSocketPort = 0, int nSocketType=SOCK_STREAM,
						LPCTSTR lpszSocketAddress = 0 );

		bool Attach( SOCKET hSocket );
		SOCKET Detach();

		SOCKET Accept( SOCKADDR *lpAddr = 0, int *lpLen = 0 )
				{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->Accept( lpAddr, lpLen );
					}
					else
					{
						CSocket		soc;

						GetAsyncSocket()->Accept( soc, lpAddr, lpLen );
						return soc.Detach();
					}
				}

		bool GetPeerName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
				{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->GetPeerName( lpSockAddr,
																lpSockAddrLen );
					}
					else
					{
						return (GetAsyncSocket()->GetPeerName( lpSockAddr,
																lpSockAddrLen ) != FALSE);
					}
				}

		bool GetSockName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
		 		{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->GetSockName( lpSockAddr,
																lpSockAddrLen );
					}
					else
					{
						return (GetAsyncSocket()->GetSockName( lpSockAddr,
																lpSockAddrLen ) != FALSE);
					}
				}

		bool SetSockOpt( int nOptionName, const void* lpOptionValue,
							int nOptionLen, int nLevel = SOL_SOCKET )
		 		{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->SetSockOpt( nOptionName,
														lpOptionValue,
														nOptionLen, nLevel );
					}
					else
					{
						return (GetAsyncSocket()->SetSockOpt( nOptionName,
															lpOptionValue,
															nOptionLen,
															nLevel ) != FALSE);
					}
				}

		bool GetSockOpt( int nOptionName, void* lpOptionValue,
							int* lpOptionLen, int nLevel = SOL_SOCKET )
		    	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->GetSockOpt( nOptionName,
														lpOptionValue,
														lpOptionLen, nLevel);
					}
					else
					{
						return (GetAsyncSocket()->GetSockOpt( nOptionName,
															lpOptionValue,
															lpOptionLen,
															nLevel ) != FALSE);
					}
				}

		bool Bind ( const SOCKADDR* lpSockAddr, int nSockAddrLen )
		     	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->Bind( lpSockAddr,
														nSockAddrLen );
					}
					else
					{
						return (GetAsyncSocket()->Bind( lpSockAddr,
														nSockAddrLen ) != FALSE);
					}
				}

		virtual void Close()
		     	{
					if (GetSyncSocket())
					{
						GetSyncSocket()->Close();
					}
					else
					{
						GetAsyncSocket()->Close();
					}
					return;
				}

		bool Connect( const SOCKADDR* lpSockAddr, int nSockAddrLen );

		bool IOCtl( long lCommand, DWORD* lpArgument )
		      	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->IOCtl( lCommand, lpArgument );
					}
					else
					{
						return (GetAsyncSocket()->IOCtl( lCommand, lpArgument ) != FALSE);
					}
				}

		bool Listen( int nConnectionBacklog = 5 )
		      	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->Listen( nConnectionBacklog );
					}
					else
					{
						return (GetAsyncSocket()->Listen( nConnectionBacklog ) != FALSE);
					}
				}

		virtual int Receive( void* lpBuf, int nBufLen, int nFlags = 0 )
		       	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->Receive( lpBuf, nBufLen,
															nFlags );
					}
					else
					{
						return GetAsyncSocket()->Receive( lpBuf, nBufLen,
															nFlags );
					}
				}

		int ReceiveFrom( void* lpBuf, int nBufLen, SOCKADDR* lpSockAddr,
							int* lpSockAddrLen, int nFlags = 0 )
		       	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->ReceiveFrom( lpBuf, nBufLen,
																lpSockAddr,
																lpSockAddrLen,
																nFlags );
					}
					else
					{
						return GetAsyncSocket()->ReceiveFrom( lpBuf, nBufLen,
																lpSockAddr,
																lpSockAddrLen,
																nFlags );
					}
				}

		enum { receives = 0, sends = 1, both = 2 };

		bool ShutDown( int nHow = sends )
		       	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->ShutDown( nHow );
					}
					else
					{
						return (GetAsyncSocket()->ShutDown( nHow ) != FALSE);
					}
				}

		virtual int Send( const void* lpBuf, int nBufLen, int nFlags = 0 )
		      	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->Send( lpBuf, nBufLen, nFlags );
					}
					else
					{
						return GetAsyncSocket()->Send( lpBuf, nBufLen,
														nFlags );
					}
				}

		int SendTo( const void* lpBuf, int nBufLen, const SOCKADDR* lpSockAddr,
					int nSockAddrLen, int nFlags = 0 )
		       	{
					if (GetSyncSocket())
					{
						return GetSyncSocket()->SendTo( lpBuf, nBufLen,
														lpSockAddr,
														nSockAddrLen, nFlags );
					}
					else
					{
						return GetAsyncSocket()->SendTo( lpBuf, nBufLen,
															lpSockAddr,
															nSockAddrLen,
															nFlags );
					}
				}

	public:
		void SetNotificationInfo( sockinetbuf* pInet )
				{
					GetAsyncSocket()->SetNotificationInfo( pInet );
				}

		void CancelBlockingCall()
				{
					if ( GetAsyncSocket() )
					{
						GetAsyncSocket()->CancelBlockingCall();
					}
				}

		bool IsAsyncSocket() { return GetAsyncSocket() != 0; }
		
		ChAsyncChacoSocket* GetAsyncSocket() const { return m_pAsyncSock; }
		ChSyncChacoSocket* GetSyncSocket() const { return m_pSyncSock; }

	public:
		virtual ~ChChacoSocket();

	private :
		ChSyncChacoSocket*	 m_pSyncSock;
		ChAsyncChacoSocket*  m_pAsyncSock;

	#if defined( CH_MSW )

	public :
		static void InitWinsockNotification( HINSTANCE hInstance );
		static void UpdateSocketPreferences( const ChString& strProduct );
		static bool PumpMessages( HANDLE hWaitOn );

		static HWND GetWSAWnd() { return m_hwndWSA; }

		static const ChString&  GetSOCKSHost() { return m_strSocksHostName; }
   		static chuint32 GetSOCKSPort() { return m_luSocksPort; }
   		static bool IsUsingSOCKS() { return !m_strSocksHostName.IsEmpty(); }
		static bool IsSOCKSInited() { return m_boolSocksAddrInited; }
		
		void InitSOCKSAddress();

	private :
		static HWND			m_hwndWSA;

	#endif

	public:
		static bool			m_boolSocksAddrInited;
		static sockinetaddr	m_saSOCKS;
		static ChString 		m_strSocksHostName;
		static chuint32		m_luSocksPort;
};


/*----------------------------------------------------------------------------
	sockbuf class
----------------------------------------------------------------------------*/
// UE:  This class has undergone extensive modification (mostly eliminating
//      the buffering scheme) due to nonstandard stuff in MSVC.  This version
//      is compatible with RW, and therefore should be compatible with
//			the new ANSI C++ 
class CH_EXPORT_CLASS sockbuf: public std::streambuf
{
	public:
    	enum type { sock_stream	= SOCK_STREAM,
					sock_dgram	= SOCK_DGRAM,
					sock_raw	= SOCK_RAW,
					sock_rdm	= SOCK_RDM,
					sock_seqpacket  = SOCK_SEQPACKET };
	    enum option {	so_debug		= SO_DEBUG,
						so_acceptconn	= SO_ACCEPTCONN,
						so_reuseaddr	= SO_REUSEADDR,
						so_keepalive	= SO_KEEPALIVE,
						so_dontroute	= SO_DONTROUTE,
						so_broadcast	= SO_BROADCAST,
						so_useloopback	= SO_USELOOPBACK,
						so_linger		= SO_LINGER,
						so_oobinline	= SO_OOBINLINE,
						so_sndbuf		= SO_SNDBUF,
						so_rcvbuf		= SO_RCVBUF,
						so_sndlowat		= SO_SNDLOWAT,
						so_rcvlowat		= SO_RCVLOWAT,
						so_sndtimeo		= SO_SNDTIMEO,
						so_rcvtimeo		= SO_RCVTIMEO,
						so_error		= SO_ERROR,
						so_type			= SO_TYPE };
	    enum shuthow {	shut_read,
						shut_write,
						shut_readwrite };
	    enum { somaxconn	= 5 };

	    struct socklinger {
			int	l_onoff;	// option on/off
			int	l_linger;	// linger time

			socklinger (int a, int b): l_onoff (a), l_linger (b) {}
	    };

	public:
		ChChacoSocket* GetChacoSocket() { return m_pSocket; }

	protected:
	    ChChacoSocket*	m_pSocket;			// counts the # refs to sock
	    int				stmo;				// -1==block, 0==poll, >0 == waiting time in secs
	    int				rtmo;				// -1==block, 0==poll, >0 == waiting time in secs

	    virtual int	underflow ();
	    virtual int	overflow( int c = EOF );
//	    virtual int	doallocate ();
//	    int			flush_output ();

#ifdef _S_NOLIBGXX
    int         		x_flags; // port to USL iostream
    int                 xflags () const { return x_flags; }
    int                 xsetflags (int f) { return x_flags |= f; }
    int                 xflags (int f) { int ret = x_flags;
    															x_flags = f | _S_UNBUFFERED; return ret; }
    //void                xput_char (char c) { *pptr() = c; pbump (1); }
    int                 linebuffered () const { return x_flags & _S_LINE_BUF; }
    
    int									unbuffered () const { return 1; }
#endif // _S_NOLIBGXX

public:
				sockbuf (SOCKET soc = -1);
    			sockbuf (int, type, int proto=0);
				sockbuf( int domain, type ty, ChSocketHandler pHandler,
						chparam userData = 0, chint16 sProto = 0 );

    			sockbuf (const sockbuf&);
    sockbuf&		operator = (const sockbuf&);
    virtual 		~sockbuf ();
    			operator SOCKET() const { return m_pSocket->GetSocket(); }


    virtual sockbuf*	open	(type, int proto=0);
    virtual sockbuf*	close	();
    virtual int		sync	();
//    virtual size_t	sys_read (char* buf, size_t len);
//    virtual size_t	sys_write (const void* buf, long len);
//    virtual int		xsputn (const char* s, int n);
    int			is_open () const { return m_pSocket->IsOpen(); }
    //int			is_eof  ()       { return xflags() & _S_EOF_SEEN; }


    virtual int bind( sockAddr& );
    virtual void connect( sockAddr& );

    void listen( int num = somaxconn );
    virtual sockbuf	accept();
    virtual sockbuf	accept( sockAddr& sa );

    int			read	(void* buf, int len);
    int			recv	(void* buf, int len, int msgf=0);
    int			recvfrom(sockAddr& sa,
				 void* buf, int len, int msgf=0);


    int			write	(const void* buf, int len) const;
    int			send	(const void* buf, int len, int msgf=0);
    int			sendto	(sockAddr& sa,
				 const void* buf, int len, int msgf=0);

    int			sendtimeout (int wp=-1);
    int			recvtimeout (int wp=-1);
    int			is_readready (int wp_sec, int wp_usec=0) const;
    int			is_writeready (int wp_sec, int wp_usec=0) const;
    int			is_exceptionpending (int wp_sec, int wp_usec=0) const;

    void		shutdown (shuthow sh);

    int			getopt(option op, void* buf,int len,
			       int l=SOL_SOCKET) const;
    void		setopt(option op, void* buf,int len,
			       int l=SOL_SOCKET) const;

    type		gettype () const;
    int			clearerror () const;
    int			debug	  (int opt= -1) const;
    int			reuseaddr (int opt= -1) const;
    int			keepalive (int opt= -1) const;
    int			dontroute (int opt= -1) const;
    int			broadcast (int opt= -1) const;
    int			oobinline (int opt= -1) const;
    int			linger    (int tim= -1) const;
    int			sendbufsz (int sz=-1)   const;
    int			recvbufsz (int sz=-1)   const;
	void		cancelblocking()
					{
						if ( m_pSocket )
						{
							m_pSocket->CancelBlockingCall();
						}
						// cancel all outstanding blocking calls
						sockasyncinfo::CancelBlocking();
					}
    void		error (const char* errmsg) const;
};


/*----------------------------------------------------------------------------
	sockinetbuf class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS sockinetbuf: public sockbuf
{

	public:
		enum domain { af_inet = AF_INET };

		sockinetbuf( const sockbuf& si ) : sockbuf( si ) {};
		sockinetbuf( const sockinetbuf& si );
		sockinetbuf( sockbuf::type ty, int proto=0 );
		sockinetbuf( int soc );

		chparam GetUserData() { return m_pSocket->GetUserData(); }
		void SetUserData( chparam userData );

		chuint32 GetBytesAvailable() const;

		sockinetbuf( sockbuf::type ty, ChSocketHandler pHandler,
						chparam userData = 0, chint16 sProto = 0 );

		virtual ~sockinetbuf();

		sockbuf*		open (sockbuf::type, int proto=0);

		sockinetaddr	localaddr() const;
		int				localport() const;
		const char*		localhost() const;

		sockinetaddr	peeraddr() const;
		int				peerport() const;
		const char*		peerhost() const;

		virtual int		bind( sockAddr& sa );
		int				bind();
		int				bind( unsigned long addr, int port_no=0 );
		int				bind( const char* host_name, int port_no=0 );
		int				bind( unsigned long addr, const char* service_name,
								const char* protocol_name="tcp" );
		int				bind( const char* host_name, const char* service_name,
								const char* protocol_name="tcp");

		virtual void	connect( sockAddr& sa );
		void			connect( unsigned long addr, int port_no=0 );
		void			connect( const char* host_name, int port_no=0 );
		void			connect( unsigned long addr, const char* service_name,
								const char* protocol_name="tcp" );
		void			connect( const char* host_name, const char* service_name,
								const char* protocol_name="tcp" );

		virtual void	SOCKSconnect( sockAddr& sa );
		void			SOCKSconnect( unsigned long addr, int port_no=0 );
		void			SOCKSconnect( const char* host_name, int port_no=0 );
		void			SOCKSconnect( unsigned long addr, const char* service_name,
								const char* protocol_name="tcp" );
		void			SOCKSconnect( const char* host_name, const char* service_name,
								const char* protocol_name="tcp" );

		int				is_readready ( int wp_sec, int wp_usec=0 ) const;
	protected:
    	sockinetbuf& operator=( const sockbuf& si );
		sockinetbuf& operator=( const sockinetbuf& inetSocket );

		void EstablishSOCKSconnect ( sockAddr& sa);
};



#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR
#endif

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:54  uecasm
// Import of source tree as at version 2.53 release.
//

#endif	// _CHSOCKET_H
