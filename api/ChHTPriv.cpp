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

	This file contains the implementation of the ChHTTPSocketConn class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#ifdef CH_UNIX
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define _STREAM_COMPAT 1
#endif

#include <string.h>
#include <iostream>
#include <fstream>
#ifdef CH_MSW
#if defined( CH_ARCH_16 )
#include <stdlib.h>
#include <sys/stat.h>
#include <direct.h>
#endif
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <ChTypes.h>
#include <ChHttpStream.h>
#include <ChHtpCon.h>
#include <ChDb.h>
#include <SocketXX.h>

#include <ChReg.h>
#include <ChConst.h>
#include <ChExcept.h>
#include <ChUtil.h>
#include <ChUnzip.h>

#include "ChHttpCookie.h"
#include "ChHTPriv.h"
#include "ChHttpThreadMgr.h"


#if defined( CH_VRML_VIEWER )
#include "ChGrType.h"
#endif

#include <MemDebug.h>

											// Tokens processed
char* pstrHTTPTokens[] =
{
	"HTTP",
	"Content-Type",
	"Last-Modified",
	"Content-Length",
	"Date",
	"Content-Encoding",
	"Location",
	"Set-Cookie",
};

											/* Initialize static member
												variables */

ChDataBase* 		ChHTTPInfo::m_pURLDB = 0;			 // URL cache database object
int 				ChHTTPInfo::m_iActiveConn = 0;		 // Total active connections


bool					ChHTTPInfo::m_boolThreaded = 0;
ChString				ChHTTPInfo::m_strCacheDir;

ChHTTPVisitedList	ChHTTPInfo::m_httpVisited;	 		// Current session visited




#if	defined( CH_MSW ) && defined( CH_ARCH_32 )

CRITICAL_SECTION	ChHTTPInfo::m_httpSync;

#endif	// defined( CH_MSW ) && defined( CH_ARCH_32 )

											// Request queue

#if !defined( CH_UNIX )

ChHTTPInfoList 		ChHTTPInfo::m_httpRequestQueue;

											// Currently active connection list

ChHTTPInfoList 		ChHTTPInfo::m_httpConnList;

ChSocketAddrList 	ChHTTPInfo::m_socketAddrList;
ChSocketDelList* 	ChHTTPInfo::m_psocketDelList = 0;
ChHttpThreadMgr*	ChHTTPInfo::m_pThreadMgr = 0;

#endif	// !defined( CH_UNIX )


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#ifdef CH_MSW
#define	NAME_BASE_LEN	8
#define NAME_EXT_LEN	3
#else
#define	NAME_BASE_LEN	255
#define NAME_EXT_LEN	255
#endif



#define old_url "http://www2.chaco.com/~glenn/pueblo/"
#define new_url "http://www.chaco.com/pueblo/"

/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------
	ChHTTPInfo class  static initializer called by ChHTTPSocketConn
----------------------------------------------------------------------------*/


void ChHTTPInfo::InitHTTPInfo(  )
{
											/* Check if the system we are running under
											  supports multithreading */
	m_boolThreaded = !!(ChUtil::GetSystemProperties() & CH_PROP_MULTITHREADED);

	// open the database
	OpenCacheDataBase();

	#if	defined(CH_MSW ) && defined( WIN32 )
	if (  IsThreaded() )
	{									// Initialize the critcal section

		::InitializeCriticalSection( &m_httpSync );

		m_pThreadMgr = new ChHttpThreadMgr();

		AfxBeginThread( HTTPRequestThread, m_pThreadMgr ); //, 
							//THREAD_PRIORITY_BELOW_NORMAL );

	}
	else
	{

		// Create the deferred delete list for Win16 and Win32s, 
		// Deleting socket connection inside a sockent handler causes an ASSERT
		// if there is a pending connection.

		m_psocketDelList = new ChSocketDelList;
	}
	#endif
}

void ChHTTPInfo::OpenCacheDataBase()
{
	#if defined( CH_VRML_VIEWER )
	ChRegistry reg( CH_COMPANY_NAME, CH_VRML_PRODUCT_NAME, CH_CACHE_GROUP );
	#else
	ChRegistry	reg( CH_CACHE_GROUP );
	#endif

	ChString		strDir;
											/* Get the local cache directory.
												Start with the app path. */
	ChUtil::GetAppDirectory( strDir );

	#if defined( CH_MSW )
	{
		strDir += CACHE_DIR;

		reg.Read( CH_CACHE_DIR, m_strCacheDir, strDir );
	}
	#elif defined( CH_UNIX )
	{
		strDir = ChUtil::GetPuebloDir() + "/" + CACHE_DIR;
		m_strCacheDir = strDir;
	}
	#else	// defined( CH_MSW )
	{
		#error "ChHTTPInfo::OpenCacheDataBase : OS not defined"
	}
	#endif	// defined( CH_MSW )

	if (!ChUtil::FileExists( m_strCacheDir ))
	{
		ChUtil::CreateDirectoryTree( m_strCacheDir );

		if (!ChUtil::FileExists( m_strCacheDir ))
		{
			ASSERT( "Unable to create cache directory" );
		}
	}

	// data base name and location
	ChString strDB = m_strCacheDir;
	strDB += PATH_SEPARATOR_CHAR;
	strDB += URL_DBNAME;
										// open a new data base
	m_pURLDB = new ChDataBase( strDB );

	if (m_pURLDB->IsValidDB())
	{									// open the database
		m_pURLDB->OpenDB( CHDB_READWRITE );
	}
	else
	{
		m_pURLDB->OpenDB( CHDB_CREATE | CHDB_READWRITE );
	}
}

/*----------------------------------------------------------------------------
	ChHTTPInfo class
----------------------------------------------------------------------------*/
ChHTTPInfo::ChHTTPInfo( ChHTTPSocketConn*	pHTTPConn, const ChString& strURL,
						chparam userData, const char* pstrHostName,
						chuint32 flOptions, ChHTTPStreamManager* pStreamMgr )
{
											/* Check the data base if we have
												the module or data locally */
											// Initialize all member variables
	#if	defined(CH_MSW ) && defined( WIN32 )
	m_pThread = NULL;
	#endif
	m_pAltStreamMgr			= pStreamMgr;
	m_pHTTPConn 			= pHTTPConn;
	m_userData				= userData;
	m_pSocket 				= 0;
	m_luTotalLen 			= 0;
	m_pstrBuffer 			= 0;
	m_luBufferSize          = 0;
	m_luToProcess 			= 0;
	m_CurrState 			= 0;
	m_luBodyLen 			= 0;
	m_flOptions 			= flOptions;
	m_pFile					= 0;
	m_boolCache 			= false;
	m_boolCreateByMe 		= false;		// socket create by this object
	m_bbPrefetched			= false; 		// this is set to true if we find this in our
											// cache but has not been visited
	m_bbAbort				= false;		// set this to true to abort operation
	m_bbCompleted			= false;		// set to true when the HTTP request
											// is successful :
	//m_boolNotitfied			= false;		// Not notified yet
	m_nZipType				= ChUnzip::typeUnknown;	 
	m_iError 				= 0;
	m_boolUpdateInfo		= false;

	// save the current pointer in HTTP stream
	m_httpStream.m_pHTTPInfo = this;
	m_iStreamState = streamUnInitialized;

	SetState( HTTP_BEGIN_PROCESS );	// Current state of my HTTP state machine



											// Get the different URL parts
   	if ( !m_urlParts.GetURLParts( strURL, pstrHostName ) )
	{  // bad URL
	 	m_bbAbort = true;
		SetError( CH_HTTP_ERROR_BAD_URL );
		return;
	}

	switch( m_urlParts.GetScheme() )
	{
		case ChURLParts::typeFile :
		{
			HandleFileScheme();
			break;
		}
		default :
		{
			HandleScheme();
			break;
		}
	}

	return;
}

void ChHTTPInfo::HandleFileScheme()
{
	if ( ChURLParts::MapURLToHostFile( m_urlParts.GetURL(), m_strLocalName ) )
	{
		if ( ChUtil::FileExists( m_strLocalName ))
		{
			m_boolCache = true;	
			ChHTTPConn::GetMimeTypeByFileExtn( m_strLocalName, m_strMimeType  );

			return;
		}
	}
	// Failed 
 	m_bbAbort = true;
	SetError( CH_HTTP_ERROR_BAD_URL );
}

void ChHTTPInfo::HandleScheme()
{
												/* Do we have the URL locally?
												If we have the data locally
												then get the last modified
												date in GMT */
	ChString strKey = m_urlParts.GetURL();

	strKey.MakeLower();	  
	ChDBKey	dbKey( strKey );

	// Serialize HTTP access 	
	LockHTTP();


	ChDBData dbData = m_pURLDB->GetData( dbKey );

	// Unlock serialization
	UnlockHTTP();

	if ( m_flOptions & ChHTTPConn::returnCache && 0 == dbData.GetDataSize() )
	{ 	// not in our cache
		return;
	}

	chuint32 luFileSize = 0;

	if (dbData.GetDataSize())
	{										/* We have data, the data format is
												"Local file path" "\0"
												"Time in HTTP format" "\0"
												"Size " "\0"*/
		char*   pstrEnd = (char*)dbData.GetData();
		pstrEnd += dbData.GetDataSize();


		char*	pstrDate;

		// database stores only the file name, add the full path of the current database
		m_strLocalName = m_strCacheDir;
		m_strLocalName += PATH_SEPARATOR_CHAR;
		m_strLocalName += (char*)dbData.GetData();
		pstrDate = (char*)dbData.GetData();
		pstrDate += (lstrlen( pstrDate ) + 1);
	  	m_strLastModified = pstrDate;
		pstrDate += (lstrlen( pstrDate ) + 1);
		luFileSize = atol( pstrDate );

		m_boolCache = true;
   		pstrDate += (lstrlen( pstrDate ) + 1);

		m_strMimeType = "";
		if ( pstrDate < pstrEnd && *pstrDate )
		{

			chuint32 luOptions = atol( pstrDate );
			if ( luOptions & ChHTTPConn::PrefetchURL )
			{
				m_bbPrefetched = true;
			}


			if ( !( m_flOptions & ChHTTPConn::PrefetchURL) &&
						( luOptions & ChHTTPConn::PrefetchURL ) )
			{
				char strOptions[25];
				m_flOptions &= ~ChHTTPConn::PrefetchURL;
				#if defined( CH_MSW )
				_ltoa( m_flOptions , strOptions, 10 );
				#else
				ltoa( m_flOptions , strOptions, 10 );
				#endif

				if ( MAX_OPTION_BLK > lstrlen( strOptions ) )
				{
					ChMemCopy( pstrDate, strOptions, lstrlen( strOptions ) );
				}
				else
				{
					ASSERT( false );
				}

				// Serialize HTTP access 	
				LockHTTP();

				m_pURLDB->SetData( dbKey, dbData, CHDB_REPLACE );

				// Unlock serialization
				UnlockHTTP();

			}
			// get the mime type
			pstr pstrMime = pstrDate + MAX_OPTION_BLK;

			if ( pstrMime < pstrEnd && *pstrMime )
			{
		   		m_strMimeType = pstrMime;
				m_strMimeType.TrimLeft();
				m_strMimeType.TrimRight();
			}
		}
	}
	else
	{	
		m_strLocalName.Empty();
	}
	
	if ( m_strLocalName.IsEmpty() )
	{
		m_boolCache = false;
	}
	else
	{										// check if the local path is valid
		if ( !ChUtil::FileExists( GetFileName() ) || luFileSize == 0 )
		{										/* File does not exist; check if
													the directory is valid */
			m_boolCache = false;
			m_strLastModified.Empty();

			char*	pstrDir = strrchr( (char*)(LPCSTR)GetFileName(), PATH_SEPARATOR_CHAR );
			*pstrDir = 0;

			if (!ChUtil::FileExists( GetFileName() ))
			{
				ChUtil::CreateDirectoryTree( GetFileName() );
			}
												// Restore
			*pstrDir = PATH_SEPARATOR_CHAR;
		}
		else
		{ // verify the file size

			#if defined( CH_MSW )

			struct _stat temp_stat;
			_stat( GetFileName(), &temp_stat );

			#else

			struct stat temp_stat;
			stat( GetFileName(), &temp_stat );

			#endif

			if ( luFileSize != (chuint32)temp_stat.st_size )
			{
				m_boolCache = false;
				m_strLastModified.Empty();
			}
		}
	}
}




/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::~ChHTTPInfo( )

------------------------------------------------------------------------------

	Destructor for ChHTTPInfo class.

----------------------------------------------------------------------------*/

ChHTTPInfo::~ChHTTPInfo( )
{

	std::fstream*	pFile;

	if (m_pstrBuffer)
	{
		delete []m_pstrBuffer;
	}


	if (pFile = GetFile())
	{
		::delete pFile; // Cannot use MFC version of delete with MSCRT dll classes
	}


	if (m_pSocket && m_boolCreateByMe)
	{
		if ( m_psocketDelList )
		{	// add it to the delete list, we will clean up this list 
			// when we get a chance to connect again.
			// Some times we come here when we are inside a socket handler, 
			// deleteing inside the handler causes problems.
			m_psocketDelList->AddTail( m_pSocket );

		}
		else
		{
			delete m_pSocket;
		}
	}
}


void ChHTTPInfo::TermHTTPInfo(  )
{

	if (  !IsThreaded() )
	{
		sockasyncinfo::CancelBlocking();
	}

    // close all connections
	CloseAllConnections( false, true );


	if (  IsThreaded() )
	{
		#if	defined(CH_MSW ) && defined( WIN32 )
		::ResetEvent( m_pThreadMgr->GetShutdownAckEvent() );
		m_pThreadMgr->TriggerShutdown();
		#endif // CH_MSW
		// wait till the thread terminates
		DWORD dwStatus = ::WaitForSingleObject( m_pThreadMgr->GetShutdownAckEvent(), INFINITE );
		ASSERT( dwStatus == WAIT_OBJECT_0 );

		TRACE( "Request thread termiantes\n" );

	}


	#if	defined(CH_MSW ) && defined( WIN32 )
	if (  IsThreaded() )
	{									// Terminate the critical section

		::DeleteCriticalSection( &m_httpSync );

		delete m_pThreadMgr;
	}
	#endif

	delete m_pURLDB;
	m_pURLDB = 0;

	m_strCacheDir.Empty();
}


void ChHTTPInfo::CloseAllConnections( bool boolOnlyGlobal, bool boolShutDown /* = false */ )
{

	// Serialize HTTP access 	
	LockHTTP();


	if (ChHTTPInfo::GetConnectionList().GetCount())
	{


		ChPosition pos = ChHTTPInfo::GetConnectionList().GetHeadPosition();

												/* Delete records for all threads
													with the END_PROCESS state */
		while (pos != 0)
		{
			ChHTTPInfo*	pInfo = ChHTTPInfo::GetConnectionList().GetNext( pos );

			if ( !boolShutDown &&  boolOnlyGlobal )
			{
				if ( pInfo->GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState )
				{	// this is a local state connection do not mess with this
					continue;
				}

			}

			// we will put a;; connections to end process state
			// and delete them all in DeleteConnection
			pInfo->SetState( HTTP_END_PROCESS );
			pInfo->AbortRequest();
			pInfo->SetError( CH_HTTP_ERROR_USER_ABORT );
			pInfo->CloseStream();

		}

	}

	if (ChHTTPInfo::GetRequestQueue().GetCount())
	{

		ChPosition pos = ChHTTPInfo::GetRequestQueue().GetHeadPosition();

												/* Delete records for all threads
													with the END_PROCESS state */
		while (pos != 0)
		{
			ChPosition	prevPos = pos;
			ChHTTPInfo*	pInfo = ChHTTPInfo::GetRequestQueue().GetNext( pos );
										// We have a pending connection
			bool boolDelete = false;
			if ( !boolShutDown &&  boolOnlyGlobal )
			{
				if ( !(pInfo->GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState) )
				{	// delete only non local
					boolDelete = true;
				}

			}
			else
			{
				boolDelete = true;
			}

			if ( boolDelete )
			{
				ChHTTPInfo::GetRequestQueue().Remove( prevPos );
				{ 	//Set the abort state and stream
					pInfo->SetState( HTTP_END_PROCESS );
					pInfo->AbortRequest();
					pInfo->SetError( CH_HTTP_ERROR_USER_ABORT );
					pInfo->CloseStream();

				}
				delete pInfo;
				pos = ChHTTPInfo::GetRequestQueue().GetHeadPosition();
			}

		}
	}

	// Unlock serialization
	UnlockHTTP();

	if ( !IsThreaded() )
	{  // for non-threaded we need to delete the connections
		DeleteConnection();
	}

}


void ChHTTPInfo::CloseConnections( ChHTTPSocketConn* phttpConn, bool boolAbortPrefetch,
						ChHTTPStreamManager* pStreamMgr /* = 0 */ )
{

	// Serialize HTTP access 	
	LockHTTP();


	if (ChHTTPInfo::GetConnectionList().GetCount())
	{


		ChPosition pos = ChHTTPInfo::GetConnectionList().GetHeadPosition();

												/* Delete records for all threads
													with the END_PROCESS state */
		while (pos != 0)
		{
			ChHTTPInfo*	pInfo = ChHTTPInfo::GetConnectionList().GetNext( pos );

			if ( pInfo->GetHTTPConn() == phttpConn )
			{
				bool bbDelete = true;
			   	if ( pInfo->GetURLOptions() & ChHTTPConn::PrefetchURL )
				{
					bbDelete = boolAbortPrefetch;
				}

				if ( bbDelete && pStreamMgr ) 
				{	// if there is a stream manager then delete only ig it mathches
					if(	pStreamMgr != pInfo->GetStreamMgr() )
					{
						bbDelete = false;
					}
				} 

				if ( bbDelete )
				{
					TRACE( "Setting state to abort\n" );
					{ 	//Set the abort state and stream
						pInfo->SetState( HTTP_END_PROCESS );
						pInfo->AbortRequest();
						pInfo->SetError( CH_HTTP_ERROR_USER_ABORT );
						pInfo->CloseStream();

					}
				}
			}
												// We have a connection

		}

	}

	if (ChHTTPInfo::GetRequestQueue().GetCount())
	{

		ChPosition pos = ChHTTPInfo::GetRequestQueue().GetHeadPosition();

												/* Delete records for all threads
													with the END_PROCESS state */
		while (pos != 0)
		{
			ChPosition	prevPos = pos;
			ChHTTPInfo*	pInfo = ChHTTPInfo::GetRequestQueue().GetNext( pos );
										// We have a pending connection

			if ( pInfo->GetHTTPConn() == phttpConn )
			{
				bool bbDelete = true;
			   	if ( pInfo->GetURLOptions() & ChHTTPConn::PrefetchURL )
				{
					bbDelete = boolAbortPrefetch;
				}

				if ( bbDelete && pStreamMgr ) 
				{	// if there is a stream manager then delete only ig it mathches
					if(	pStreamMgr != pInfo->GetStreamMgr() )
					{
						bbDelete = false;
					}
				} 

				if ( bbDelete )
				{
					ChHTTPInfo::GetRequestQueue().Remove( prevPos );
					{ 	//Set the abort state and stream
						pInfo->SetState( HTTP_END_PROCESS );
						pInfo->AbortRequest();
						pInfo->SetError( CH_HTTP_ERROR_USER_ABORT );
						pInfo->CloseStream();

					}
					delete pInfo;
					pos = ChHTTPInfo::GetRequestQueue().GetHeadPosition();
				}
			}

		}
	}

	// Unlock serialization
	UnlockHTTP();

	if ( !IsThreaded() )
	{  // for non-threaded we need to delete the connections
		DeleteConnection();
	}

}

bool ChHTTPInfo::IsZipped()	  
{ 
	return m_nZipType != ChUnzip::typeUnknown; 
}


/*----------------------------------------------------------------------------
	ChHTTPInfo::Connect
		Connect to the server.
----------------------------------------------------------------------------*/

ChHTTPInfo::ConnectionResult ChHTTPInfo::Connect()
{
	sockinetaddr sa;	

	if (ChHTTPInfo::FindSocketAddr( GetParts().GetHostName(), sa.m_sockAddrIn ) )
	{
		return Connect( sa );
	}
	else
	{

		if ( (long)(sa.m_sockAddrIn.sin_addr.s_addr = inet_addr( GetParts().GetHostName() )) == -1)
		{
												/* If that didn't work, then try
													to look up the host by name */

			hostent * pHostEntry = gethostbyname( GetParts().GetHostName() );

			if (pHostEntry == 0)
			{
				return connFailed;
			}

	        #if defined( CH_ARCH_32 )
			memcpy( &sa.m_sockAddrIn.sin_addr, pHostEntry->h_addr, pHostEntry->h_length );
			#else
			sa.m_sockAddrIn.sin_addr.s_addr = ((LPIN_ADDR)pHostEntry->h_addr)->s_addr;
			#endif
			sa.m_sockAddrIn.sin_family = pHostEntry->h_addrtype;
		}
		else
		{
			sa.m_sockAddrIn.sin_family = sockinetbuf::af_inet;
		}
		ChHTTPInfo::AddToSocketAddrList( GetParts().GetHostName(), 
													sa.m_sockAddrIn );
		return Connect( sa );
	}

}


ChHTTPInfo::ConnectionResult ChHTTPInfo::Connect( sockinetaddr& sa )
{
	ChHTTPInfo::ConnectionResult result;

	#if defined( CH_DEBUG )
	{										/* Truncate the URL so that the
												trace statement works */

		ChString		strAbbrevURL = m_urlParts.GetURL();
		int			iLen = strAbbrevURL.GetLength();

		strAbbrevURL = strAbbrevURL.Left( 80 );
		if (iLen > 80)
		{
			strAbbrevURL += "[...]";
		}
		TRACE1( "Connecting ... %s\n", (const char*)strAbbrevURL );
	}
	#endif	// defined( CH_DEBUG )


	if ( !IsThreaded() )
	{
		m_pSocket = new sockinetbuf( sockbuf::sock_stream, httpSocketHandler );
		ASSERT( m_pSocket );
		m_pSocket->SetUserData( (chparam)this );
	}
	else
	{
		m_pSocket = new sockinetbuf( sockbuf::sock_stream );
   		ASSERT( m_pSocket );
	}

	m_boolCreateByMe = true;

	// Increment the total connect state connections
	if ( !(GetURLOptions() &  ChHTTPConn::PrefetchURL ) )
	{

		LockHTTP();
		// Update the global state and local connection state
		if ( GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
		{
		 	GetHTTPConn()->m_connState.iTotalWaiting++;
		}
		else
		{
		 	GetHTTPConn()->m_connState.iTotalWaiting++;
		 	GetHTTPConn()->m_connGlobalState.iTotalWaiting++;
		}
		UnlockHTTP();
	}

	{ 	// try to connect to the current host, if we fail
	   // see if there is a mirror site. try again

		#if defined( CH_MSW ) && defined( CH_ARCH_16 )
		TRY
		#else
		try
		#endif
		{
			// do the connect to remote server
			// connect to the server
			// Set the port number in to the address struct 

			sa.m_sockAddrIn.sin_port = htons( m_urlParts.GetPortNumber() );

			if ( m_urlParts.UsingProxy() )
			{  // connect to socks server
				m_pSocket->connect( sa );
			}
			else
			{
				m_pSocket->SOCKSconnect( sa );
			}
			result = connSucceeded;
		}
		#if defined( CH_MSW ) && defined( CH_ARCH_16 )
		CATCH( ChSocketEx, socketEx )
		#else
		catch( ChSocketEx socketEx )
		#endif
		{
			#if defined( CH_EXCEPTIONS )
			result = connFailed;

			if ( !IsAborted() && socketEx.GetCause() == ChEx::connectFailed)
			{
											// Connection failed!
				// try the mirror site if any
				if (m_urlParts.GetURL().Left(strlen(old_url)) == old_url) 
				{
					ChString strNewURL = ((ChString) new_url) +
						m_urlParts.GetURL().Right(strlen(old_url) - 1);	

					TRACE( "Connection failed, trying different host\n" );

					// Update any waits to the new URL
					UpdateWaitRequest( this, strNewURL );


				  if ( m_urlParts.GetURLParts( strNewURL ) )
					{	// Get the address  of the new domain
						result = connRetry; // Retry	
					}
				}

			}
			#endif
		}
		#if defined( CH_MSW ) && defined( CH_ARCH_16 )
		END_CATCH
		#endif
	}
	// coming out of wait state
	if ( !(GetURLOptions() &  ChHTTPConn::PrefetchURL ) )
	{
		LockHTTP();
		// Update the global state and local connection state
		if ( GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
		{
		 	GetHTTPConn()->m_connState.iTotalWaiting--;
		}
		else
		{
		 	GetHTTPConn()->m_connState.iTotalWaiting--;
		 	GetHTTPConn()->m_connGlobalState.iTotalWaiting--;
		}
		UnlockHTTP();
	}

	if ( IsAborted() )
	{
		result = connFailed;	
	}

	return result;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::SendRequest( )

------------------------------------------------------------------------------

	Construct the HTTP request command and send it to the server.

----------------------------------------------------------------------------*/

bool ChHTTPInfo::SendRequest( )
{
	if ( IsAborted() )
	{
		return false;
	}

	if (0 == m_pstrBuffer)
	{
		m_luBufferSize   = minBufferSize; // should be enough for headers, realloc on content length
		m_pstrBuffer = new char[m_luBufferSize + 1];
		ASSERT( m_pstrBuffer );
		m_luToProcess = 0;
		m_luBodyLen   = 0;
		m_luTotalLen  = 0;
		SetState( HTTP_BEGIN_PROCESS );	// Current state of my HTTP state machine
	}

	// Build  the Requst command
	BuildRequestLine( );
	BuildGeneralHeader( );
	BuildRequestHeader( );
	BuildObjectHeader( );

	TerminateCommand( );

	// send the command
	if ( (GetURLOptions() & ChHTTPConn::UsePostMethod ) && m_strPostData.GetLength() )
	{
		m_strRequestBuffer += m_strPostData;	
	}

	m_strRequestBuffer += TEXT( "\r\n" );
	int iLen =  m_strRequestBuffer.GetLength();
	int iCount = 0;
	const char* pstrBuffer =  m_strRequestBuffer;

//#ifdef _DEBUG
//	afxDump << "----- HTTP Request:\n" << m_strRequestBuffer << "-----\n";
//#endif
	while (iLen != 0)
	{
		int wcnt = m_pSocket->write( pstrBuffer + iCount, iLen );

		if (wcnt == -1)
		{
			TRACE( "Post : Write to socket failed\n" );
			return false;
		}

		iLen -= wcnt;
		iCount += wcnt;
	}

	m_strRequestBuffer.Empty();

	return true;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::MapURLToLocalName()

------------------------------------------------------------------------------

	Map a given URL to a local file name relative to the URL cache directory.

----------------------------------------------------------------------------*/

#if 0

void ChHTTPInfo::MapURLToLocalName( const char* pstrRemotePath )
{
	m_strLocalName = m_strCacheDir;

	if ( 0 == pstrRemotePath || 0 == *pstrRemotePath )
	{
		m_strLocalName += PATH_SEPARATOR_CHAR;
		m_strLocalName += DEF_FILE_NAME;
	}
	else
	{

		char *pstrStart = strrchr( pstrRemotePath, TEXT( '/' ));
												/* Append the remote path.
												Truncate any file name which
												exceeds 8.3 limitations */

											// Loop through the source path
		//while (*pstrStart)
		if ( !pstrStart )
		{
			#if defined( CH_MSW )
			char   strPath[MAX_PATH];
				#if defined(CH_ARCH_16 )
					GetTempFileName( GetTempDrive( 0 ), "Ch", 0, strPath );
					char *pstrFileName = _fstrrchr( strPath, PATH_SEPARATOR_CHAR );
					ASSERT( pstrFileName );
					m_strLocalName = m_strCacheDir;
					m_strLocalName += pstrFileName;

				#else
					GetTempFileName( m_strCacheDir, "Ch", 0, strPath );
					m_strLocalName = strPath;
				#endif
			#else
			char* pstrTemp;

			pstrTemp = mktemp( ChUtil::GetPuebloDir() + "/" + CACHE_DIR +
								"/ChXXXXXX" );
			ASSERT( pstrTemp );
			m_strLocalName = pstrTemp;
			#endif

		}
		else
		{
			int		iBaseCount;
			bool	boolDone;

			m_strLocalName += PATH_SEPARATOR_CHAR;

											/* Strip leading slashes and
												backslashes */

			while ((*pstrStart == TEXT( '/' )) || (*pstrStart == TEXT( '\\' )))
			{
				pstrStart++;
			}
											/* Copy the directory or file
												base name */
			iBaseCount = 0;
			boolDone = false;
			while (!boolDone)
			{
				if ((*pstrStart == 0) || (*pstrStart == TEXT( '.' )) ||
					(*pstrStart == TEXT( '/' )) || (*pstrStart == TEXT( '\\' )))
				{
					boolDone = true;
				}
				else
				{							/* Copy another char of the
												base name */
					if ( isalnum( *pstrStart ) )
					{
						m_strLocalName += *pstrStart++;
					}
					else
					{
						m_strLocalName += '_';
						pstrStart++;
					}
					iBaseCount++;

					if (iBaseCount == NAME_BASE_LEN)
					{
						boolDone = true;
					}
				}
			}
											// Skip until we hit a delimiter

			while (*pstrStart && (*pstrStart != TEXT( '.' )) &&
					(*pstrStart != TEXT( '/' )) && (*pstrStart != TEXT( '\\' )))
			{
			 	++pstrStart;
			}
											// Copy the extension if any

			if (*pstrStart && *pstrStart == TEXT( '.' ))
			{
				int		iExtCount;
				bool	boolDone;
											// Copy the extension delimiter
				m_strLocalName += *pstrStart++;

				iExtCount = 0;
				boolDone = false;
				while (!boolDone)
				{
					if ((*pstrStart == 0) || (*pstrStart == TEXT( '.' )) ||
						(*pstrStart == TEXT( '/' )) || (*pstrStart == TEXT( '\\' )))
					{
						boolDone = true;
					}
					else
					{							/* Copy another char of the
													extension */

						if ( isalnum( *pstrStart ) )
						{
							m_strLocalName += *pstrStart++;
						}
						else
						{
							m_strLocalName += '_';
							pstrStart++;
						}
						iExtCount++;

						if (iExtCount == NAME_EXT_LEN)
						{
							boolDone = true;
						}
					}
				}
			}

		}
	}
}

#else

void ChHTTPInfo::MapURLToLocalName( int iMimeType )
{
	m_strLocalName = m_strCacheDir;	
	
	ChString strTmp;
	ChHTTPConn::GetFileExtnByMimeType( iMimeType, strTmp );

	if ( strTmp.CompareNoCase(  TEXT( "tmp" )) == 0 )
	{ // map it based on the url extension
		int iIndex = m_urlParts.GetURL().ReverseFind( TEXT( '.' ) );
		if ( iIndex != -1 )
		{
			strTmp = m_urlParts.GetURL().Right( 
						m_urlParts.GetURL().GetLength() - iIndex - 1 );
			if ( strTmp.GetLength() > 3 )
			{
			  	strTmp = strTmp.Left( 3 );
			}								
		}
	}

	ChString strExtn( TEXT( "." ) );
	strExtn += 	strTmp;
		

	// check if someone has deleted our cache directory, create it
	if (!ChUtil::FileExists( m_strLocalName ))
	{
		ChUtil::CreateDirectoryTree( m_strLocalName );
	}

	// Serialize HTTP access 	
	LockHTTP();


	#if defined( CH_MSW )
	char   strPath[MAX_PATH];
		#if defined(CH_ARCH_16 )
			GetTempFileName( GetTempDrive( 0 ), "Ch", 0, strPath );

			::DeleteFile( strPath );

			char *pstrFileName = _fstrrchr( strPath, PATH_SEPARATOR_CHAR );
			ASSERT( pstrFileName );
			m_strLocalName = m_strCacheDir;
			char *pstrTmp = _fstrrchr( pstrFileName, TEXT( '.' ) );
			if ( pstrTmp )
			{
				*pstrTmp = 0;
			}
			m_strLocalName += pstrFileName;

		#else
			GetTempFileName( m_strCacheDir, "Ch", 0, strPath );
			::DeleteFile( strPath );
	
			char *pstrTmp = strrchr( strPath, TEXT( '.' ) );
			if ( pstrTmp )
			{
				*pstrTmp = 0;
			}
			m_strLocalName = strPath;
			m_strLocalName += strExtn;

			if ( ChUtil::FileExists( m_strLocalName ) )
			{
				m_strLocalName = strPath;
				m_strLocalName += TEXT( ".tmp" );
			}

		#endif
	#else
	char* pstrTemp;
	char* pstrTempIn;

	pstrTempIn = new char[MAX_PATH];
	sprintf( pstrTempIn, "%s/%s/ChXXXXXX", 
				(const char *)ChUtil::GetPuebloDir(), CACHE_DIR);
	pstrTemp = mktemp( pstrTempIn );
	ASSERT( pstrTemp );
	m_strLocalName = pstrTemp;
	::DeleteFile( m_strLocalName );

	delete []pstrTempIn;  

	m_strLocalName += strExtn;
	#endif

	// Unlock serialization
	UnlockHTTP();

}
#endif





/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::BuildRequestLine( )

------------------------------------------------------------------------------

Construct HTTP request line. Version 1.0 >
UE: updated to 1.1 because we need to support virtual hosting.
----------------------------------------------------------------------------*/

inline void ChHTTPInfo::BuildRequestLine(  )
{

	if ( IsAborted() )
	{
		return;
	}

	ChString	strPath =  m_urlParts.GetAbsPath() ?  m_urlParts.GetAbsPath() : TEXT( "" );

	if ( m_urlParts.UsingProxy() )
	{
		strPath	= m_urlParts.GetAbsPath();
	}
	else
	{
	 	strPath = TEXT( "/"  );
		strPath +=  (m_urlParts.GetAbsPath() ?  m_urlParts.GetAbsPath() : TEXT( "" ));
	}

	if ( m_urlParts.GetRelPath() )
	{
		if ( strPath.GetLength() &&
				TEXT( '/' ) != strPath[strPath.GetLength()-1] )
		{
			strPath += TEXT( '/' );
		}

		strPath += m_urlParts.GetRelPath();
	}

	if ( (GetURLOptions() & ChHTTPConn::UsePostMethod) && m_strPostData.GetLength() )
	{
		m_strRequestBuffer = "POST " ;
	}
	else
	{
		m_strRequestBuffer = "GET ";
	}
 	char	strVersion[25];
	wsprintf( strVersion, " %s%c%c",  HTTP_VERSION, CR, LF );
	
	ChURLParts::EscapeSpecialChars( strPath );

	m_strRequestBuffer += strPath;
	m_strRequestBuffer += strVersion;

}


void ChHTTPInfo::BuildRequestHeader()
{
	ChString		strUserAgent;

	if ( IsAborted() )
	{
		return;
	}

	// UE: Send Host header for HTTP/1.1, to support virtual hosting.
	m_strRequestBuffer += TEXT( "Host: " );
	m_strRequestBuffer += m_urlParts.GetHostName();
	TerminateCommand();
	
	// UE: Send Connection header for HTTP/1.1 to ensure correct operation.
	m_strRequestBuffer += TEXT( "Connection: close" );
	TerminateCommand();

	m_strRequestBuffer += TEXT( "User-Agent: " );
	GetHTTPConn()->GetUserAgent( strUserAgent );
	m_strRequestBuffer += strUserAgent;

	TerminateCommand();

	if (IsCached() && !m_strLastModified.IsEmpty())
	{
											/* if we have a date, check if we
												need to update our local
												cache */

		m_strRequestBuffer += TEXT( "If-Modified-Since: " );
		m_strRequestBuffer += m_strLastModified;
		TerminateCommand();
	}

	ChString		strCookie;
	bool		boolSecure = false;

	if (ChHTTPSocketConn::GetHTTPCookie())
	{										/* Cookie access needs to be
												serialized */
		LockHTTP();
		if (ChHTTPSocketConn::GetHTTPCookie()->
					GetCookie( m_urlParts.GetURL(), strCookie, boolSecure ))
		{
			m_strRequestBuffer += TEXT( "Cookie: " );
			m_strRequestBuffer += strCookie;
			TerminateCommand();
		}

		UnlockHTTP();
	}

	// UE: Reworded these considerably, since the original was just plain weird.
	m_strRequestBuffer += TEXT( "Accept: text/html, text/plain, image/*, */*" );
	TerminateCommand();
	
	if (!(GetURLOptions() & ChHTTPConn::UsePostMethod))
	{
		#if defined( CH_MSW )
		{
			m_strRequestBuffer +=
					TEXT( "Accept-Encoding: x-gzip, gzip, x-zip" );
			TerminateCommand();
		}
		#endif
	}
}


void ChHTTPInfo::BuildObjectHeader()
{
	if ( IsAborted() )
	{
		return;
	}

	if ((GetURLOptions() & ChHTTPConn::UsePostMethod) &&
			m_strPostData.GetLength())
	{
											/* If we have a date, check if we
												need to update our local
												cache */

		m_strRequestBuffer	+= TEXT( "Content-Type: " );
		m_strRequestBuffer += TEXT( "application/x-www-form-urlencoded" );
		TerminateCommand( );
		m_strRequestBuffer += TEXT( "Content-Length: " );
		char strTmp[25];
		#if defined( CH_MSW )
		_ltoa( m_strPostData.GetLength(), strTmp, 10 );
		#else
		ltoa( m_strPostData.GetLength(), strTmp, 10 );
		#endif
		m_strRequestBuffer += strTmp;
		TerminateCommand();
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ProcessInput

------------------------------------------------------------------------------

	Reads the contents of the socket.

----------------------------------------------------------------------------*/

void ChHTTPInfo::ProcessInput()
{

	if ( !IsThreaded() )
	{
		chuint32	luLen;

		luLen = m_pSocket->GetBytesAvailable();

											// process in chunks if required
		while ( luLen )
		{
			chuint32 luToRead = (m_luToProcess + luLen > m_luBufferSize) ?
									m_luBufferSize - m_luToProcess : luLen;

											/* Read the contents of the socket
												to the end of 'inputbuf' */

			m_pSocket->read( m_pstrBuffer + m_luToProcess, (int)luToRead );
			m_luToProcess += luToRead;		// Total data still to be processed
			luLen -= luToRead; 				// number of bytes to read

											/* Process the contents of the
												internal buffer  and update
												the m_ToProcess*/
			ProcessBuffer();
		}
	}
	else
	{
		chuint32	luMaxRead = m_luBufferSize - m_luToProcess;
		chint32		lRead;

		while ((lRead = m_pSocket->read( m_pstrBuffer + m_luToProcess, (int)luMaxRead )) > 0)
		{
			m_luToProcess += lRead;
			ProcessBuffer();
			luMaxRead = m_luBufferSize	- m_luToProcess;
		}
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ProcessInput

------------------------------------------------------------------------------

	Interpret the data read from the socket.

----------------------------------------------------------------------------*/
void ChHTTPInfo::ProcessBuffer()
{

	m_pstrBuffer[m_luToProcess] = 0;
	{
		switch ( GetState() )
		{
			case HTTP_IN_BODY :
			{
				//TRACE( "In Body:" );
				//TRACE( 	GetFileName() );
				//TRACE( "\n" );
				ProcessBody();
				break;
			}
			case HTTP_BEGIN_PROCESS :
			{
				//TRACE( "In Status line:" );
				//TRACE( 	GetFileName() );
				//TRACE( "\n" );
				ProcessStatusLine();
				break;
			}
			case HTTP_IN_HEADER :
			{
				//TRACE( "In Header:" );
				//TRACE( 	GetFileName() );
				//TRACE( "\n" );
				ProcessHeaders();
				break;
			}
			case HTTP_USE_LOCAL :
			{
				break;
			}
			default :
			{ // error occured or
				m_luToProcess  = 0;
			}
		}

	}
}

void ChHTTPInfo::ProcessBody()
{
	// if we are reading the body of the message, there is
	// nothing to process, just write to the file, or notify user of
	// the new data that is available


	if ( IsAborted() )
	{
		m_luToProcess = 0;
		return;
	}

	m_luTotalLen +=  m_luToProcess;


	// Notify progress
	NotifyProgress( );


	// we keep the file open for all systems except win16 and win32s
	// due to open file handle limitations
											// Write to file
	if (  !IsThreaded() )
	{
		const char* pstrFile;

		if ( m_nZipType != ChUnzip::typeUnknown )
		{
			pstrFile = GetZipFileName();
		}
		else
		{
			pstrFile = GetFileName();
		}

		GetFile()->open( pstrFile, std::ios::app | IOS_BINARY );
	}

	GetFile()->write( m_pstrBuffer, (int)m_luToProcess );

	if ( m_httpStream.m_uStreamOption & ChHTTPStreamManager::streamNormal 
			&& GetStreamMgr() )
	{
		m_iStreamState = streamInData;

		chint32 lBytes = GetStreamMgr()->WriteReady( GetUserData(), 
								&m_httpStream, m_luToProcess );
		if ( lBytes > 0 )
		{  
			GetStreamMgr()->Write( GetUserData(), &m_httpStream, 
										0, m_luToProcess, m_pstrBuffer );
		}
	}

	if (  !IsThreaded() )
	{
		GetFile()->close();
	}

	m_luToProcess = 0;
	return;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::OnDataComplete

------------------------------------------------------------------------------

----------------------------------------------------------------------------*/


void ChHTTPInfo::OnDataComplete()
{
	// Notify we are done 
	//AsyncNotifyProgress( ChHTTPInfo::msgEndProgress );

		// close the local file 
	if ( IsThreaded() )
	{
		GetFile()->close();
	}


	#if defined( CH_MSW )	 
	if ( m_nZipType != ChUnzip::typeUnknown )
	{
		ChUnzip unzip;

		if ( unzip.UnzipFileTo( m_strZipName, 
						m_nZipType == ChUnzip::typeGZIP ? UNZIP_OPT_GZIP_FILE : UNZIP_DEF_OPTION,
						0, GetFileName() ) )
		{ // get the size of uncomppressed file
			TRACE2( "Unzip successful %s -> \n", LPCSTR(m_strZipName), LPCSTR(GetFileName()) );

			#if defined( CH_MSW )

			struct _stat temp_stat;
			_stat( GetFileName(), &temp_stat );

			#else

			struct stat temp_stat;
			stat( GetFileName(), &temp_stat );

			#endif
			m_luTotalLen = temp_stat.st_size;

		  	//  delete the zip file
		  	::DeleteFile( m_strZipName );
		}
		else
		{
			TRACE1( "Unzip failed %s\n", LPCSTR(m_strZipName) );
			m_luToProcess = 0;
		  	::DeleteFile( m_strZipName );
			SetState( HTTP_ERROR );
			SetError( CH_HTTP_ERROR_INVALID_DATA );
			return;
		}

	}
	#endif


	if ( !(GetURLOptions() & ChHTTPConn::DoNotCache) )
	{
		char *		pstrFile = strrchr( (char*)(LPCSTR)GetFileName(), PATH_SEPARATOR_CHAR );
		ASSERT( pstrFile );
		// point to the file name
		pstrFile++;

		const char * pstrMimeType = m_strMimeType;

		int 	iPathLen = lstrlen( pstrFile ) + 1;
		int 	iDateLen = m_strLastModified.GetLength() + 1;
		int		iMimeTypeLen = lstrlen( pstrMimeType ) + 1;
		char	strSize[25], strOptions[25];

		#if defined( CH_MSW )
		_ltoa( m_luTotalLen, strSize, 10 );
		#else
		ltoa( m_luTotalLen, strSize, 10 );
		#endif

		int  iSizeLen = lstrlen( strSize ) + 1;

		#if defined( CH_MSW )
		_ltoa( GetURLOptions() , strOptions, 10 );
		#else
		ltoa( GetURLOptions() , strOptions, 10 );
		#endif

		int  iOptionLen = lstrlen( strOptions ) + 1;

		int iTotalLen =  iPathLen + iDateLen + iSizeLen + MAX_OPTION_BLK + iMimeTypeLen + 1;
		ptr pDBData = new char[ iTotalLen ];

		ChMemClear( pDBData, iTotalLen );

		ChMemCopy( pDBData, pstrFile, iPathLen );
		ChMemCopy( (char*)pDBData + iPathLen, LPCSTR(m_strLastModified), iDateLen );
		ChMemCopy( (char*)pDBData + (iPathLen + iDateLen), strSize, iSizeLen );
		ChMemCopy( (char*)pDBData + (iPathLen + iDateLen + iSizeLen ),
						strOptions, iOptionLen );
		ChMemCopy( (char*)pDBData +
						(iPathLen + iDateLen + iSizeLen + MAX_OPTION_BLK ),
							pstrMimeType, iMimeTypeLen );

		ChDBData dbData( pDBData, iTotalLen );

		ChString strKey = m_urlParts.GetURL();
		strKey.MakeLower();	  

		ChDBKey  dbKey( strKey );

		// Serialize HTTP access 	
		LockHTTP();

		ChDBData dbCacheData = m_pURLDB->GetData( dbKey );
							// Set the data
		m_pURLDB->SetData( dbKey, dbData,
						dbCacheData.GetDataSize() ? CHDB_REPLACE : CHDB_INSERT );

		if ( 1 ||  !m_strLastModified.IsEmpty() )
		{ // store it only if it is persistant on the server
			// add it to the current session
	       int iTemp = 1; // we need to do this for Win16, the compiler cannot
	        // convert const int to const int&
			ChHTTPInfo::m_httpVisited.Insert( m_urlParts.GetURL(), iTemp );
		}

		// Unlock serialization
		UnlockHTTP();

		delete []pDBData;
	}


	// We are done with processing data
	SetState( HTTP_DATA_COMPLETE );

	// Do the load complete notification
	SetError( 0 );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ProcessStatusLine

------------------------------------------------------------------------------
Process the HTTP status line.
----------------------------------------------------------------------------*/


void ChHTTPInfo::ProcessStatusLine()
{											// Our HTTP response state machine

											// Terminate the buffer
	m_pstrBuffer[m_luToProcess] = TEXT( '\0' );

	char* pstrNext = strchr( m_pstrBuffer, LF );

	if (pstrNext)
	{										// Looks like we have a status line
											// Do we have the version
		chuint32  i = 0;

		while( i < m_luToProcess && m_pstrBuffer[i] != TEXT( '/' ))  
								 //&&	m_pstrBuffer[i] != TEXT( '<' ))
		{
			i++;
		}


		pstrNext = &m_pstrBuffer[i + 1];
		*(pstrNext - 1) = TEXT ('\0' );

		if ( lstrcmpi( pstrHTTPTokens[HTTP], m_pstrBuffer ))
		{ // invalid status line
			ProcessNoHeaderReply();
			return;
		}
		else
		{ // get the status code
			while( pstrNext < &m_pstrBuffer[m_luToProcess]   && *pstrNext++ != TEXT( ' ' ) );

			int iStatus = (int)atol( pstrNext );
			switch ( iStatus )
			{
				case 200 :
				{  // successful
					pstrNext = strchr( pstrNext, LF ) + 1;
					// now lets get to process header mode
					SetState( HTTP_IN_HEADER );

					m_luToProcess -= (chuint32)( pstrNext - m_pstrBuffer );
					// copy the rest to be processed
					ChMemCopy( m_pstrBuffer, pstrNext, m_luToProcess );
					if ( m_luToProcess )
					{
						ProcessHeaders();
					}
					return;
				}
				case 304 :
				{
					SetState( HTTP_USE_LOCAL );
					m_luToProcess = 0;
					TRACE( "Use local :" );
					TRACE( 	GetFileName() );
					TRACE( "\n" );
			 		// notify the module of the new data

					// Serialize HTTP access 	
					LockHTTP();

                    int iTemp = 1; // we need to do this for Win16, the compiler cannot
                    // convert const int to const int&
					ChHTTPInfo::m_httpVisited.Insert( m_urlParts.GetURL(), iTemp );

					// Unlock serialization
					UnlockHTTP();

					if ( IsAborted() )
					{
						m_luToProcess = 0;
						return;
					}

					// Set the time of the file to current time for LRU implementation
					ChUtil::SetFileToCurrentTime( GetFileName() );

					// What we have is no different from the one on the server
					UpdateStatusInfo();
					StreamURL();
					UpdateStatusInfo( false );
					return;
				}
				default :
				{
					pstrNext = strchr( pstrNext, LF ) + 1;
					if ( pstrNext )
					{
						m_luToProcess -= (chuint32)( pstrNext - m_pstrBuffer );
						// copy the rest to be processed
						ChMemCopy( m_pstrBuffer, pstrNext, m_luToProcess );
					}
					else
					{
					 	m_pstrBuffer[0] = 0;
						m_luToProcess = 0;
					}
					HandleError( iStatus );
					return;
				}

			}

		}
	}
	else
	{
		ProcessNoHeaderReply();
		return;
	}
}


#define STANDARD		1  			//  with \r\n delimiter
#define NON_STANDARD	2			// with  \n delimiter

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ProcessHeaders

------------------------------------------------------------------------------
Process HTTP headers
----------------------------------------------------------------------------*/
void ChHTTPInfo::ProcessHeaders()
{

	int		iDelimType = NON_STANDARD;
											// Do we have the complete header
											// this is the right delimitor

	char*	pstrBody = strstr( m_pstrBuffer, "\r\n\r\n" );

	if (pstrBody)
	{
		iDelimType = STANDARD;
	}
	else
	{
		pstrBody = strstr( m_pstrBuffer, "\n\n" );
		if (pstrBody)
		{
			iDelimType = NON_STANDARD;
		}
	}

	if (!pstrBody)
	{										/* We have a partial body, go back
												and read the rest */
		return;
	}

	m_strMimeType.Empty();
											// process the header;

	char*	pstrType = m_pstrBuffer, *pstrNext, *pstrData;

	while (ChUtil::GetNextHeaderString( pstrType, pstrData, pstrNext, iDelimType ))
	{
		int		iType = GetHdrType( pstrType );

		switch( iType )
		{
			case CONTENT_TYPE :
			{
				m_strMimeType = pstrData;
				m_strMimeType.TrimLeft();
				m_strMimeType.TrimRight();
				break;
			}

			case LAST_MODIFIED :
			{
				m_strLastModified = pstrData;
				break;
			}

			case CONTENT_LENGTH :
			{
				m_luBodyLen = atol( pstrData );
				break;
			}
			case  CONTENT_ENCODING :
			{
				ChString strData = pstrData;
				strData.MakeLower();

				if (  strData.Find( "x-gzip" ) != -1 )
				{
					m_nZipType = ChUnzip::typeGZIP;
				}
				else if (  strData.Find( "gzip" ) != -1 )
				{
					m_nZipType = ChUnzip::typeGZIP;
				}
				else if	( strData.Find( "x-zip" ) != -1 ) 
				{
					m_nZipType = ChUnzip::typePKZIP;
				}
				break;
			}
			case SET_COOKIE :
			{
				if ( ChHTTPSocketConn::GetHTTPCookie() )
				{	// Serialize access to cookie list
					LockHTTP();

					ChHTTPSocketConn::GetHTTPCookie()->
							SetCookie( m_urlParts.GetURL(), pstrData);
					// if this was a persistant cookie then save it
					if ( !ChHTTPSocketConn::GetHTTPCookie()->IsSaved() )
					{
						ChHTTPSocketConn::GetHTTPCookie()->WriteCookieFile();
					}
					
					UnlockHTTP();

				}
				break;
			}
			default:
			{								// ignore for now
				break;
			}
		}
		pstrType = pstrNext;
	}

	if ( m_strMimeType.IsEmpty() )
	{
		pstr pstrData = pstrBody;
		int i = 0;

		if (iDelimType == STANDARD)
		{
			pstrData += 4;						/* Points to the begining of
													the data */
		}
		else
		{
			pstrData += 2;						/* Points to the begining of
													the data */

		}

		if ( pstrData[i] == TEXT( '<' ) )
		{ 
			m_strMimeType = "text/html";
		}
		else if ( pstrData[i] ==   TEXT( 'G' )  && 
				  pstrData[i+1] == TEXT( 'I' ) &&
				  pstrData[i+2] == TEXT( 'F' ) )
		{
			m_strMimeType = "image/gif";
		}
		else if ( pstrData[i + 6] == TEXT( 'J' )  && 
				  pstrData[i + 7] == TEXT( 'F' ) &&
				  pstrData[i + 8] == TEXT( 'I' ) &&
				  pstrData[i + 9] == TEXT( 'F' ) )
		{
			m_strMimeType = "image/jpeg";
		}
		else if ( pstrData[i] == TEXT( '#' )  && 
				  pstrData[i+1] == TEXT( 'V' ) &&
				  pstrData[i+2] == TEXT( 'R' ) &&
				  pstrData[i+3] == TEXT( 'M' ) &&
				  pstrData[i+4] == TEXT( 'L' ) )
		{
			m_strMimeType = "x-world/x-vrml";
		}
		else
		{  // what is it ???
			m_strMimeType = "text/html";
		}
	}

	if ( !m_strMimeType.IsEmpty() )
	{
		std::fstream*	pFile;

		SetState( HTTP_IN_BODY );

		if (0 == m_luBodyLen)
		{									// max we can get
			m_luBodyLen = 0xFFFFFFFF;
			m_bbCompleted = true;	// We don't know how much data we have,
									//  what ever we get till the socket is closed
									//  is our data
		}

		if ( m_strLocalName.IsEmpty() )
		{ // get the local file name
			MapURLToLocalName( ChHTTPSocketConn::GetMimeType( m_strMimeType ) );
		}

		const char* pstrOutFile;
		if ( m_nZipType != ChUnzip::typeUnknown )
		{
			// download the zipped file under a temp name, after unzipping the file
			// to its original name, delete the tempp file
			#if defined( CH_MSW )
			char   strPath[MAX_PATH];
				#if defined(CH_ARCH_16 )
					GetTempFileName( GetTempDrive( 0 ), "Ch", 0, strPath );
					char *pstrFileName = _fstrrchr( strPath, PATH_SEPARATOR_CHAR );
					ASSERT( pstrFileName );
					m_strZipName = m_strCacheDir;
					m_strZipName += pstrFileName;

				#else
					GetTempFileName( m_strCacheDir, "Ch", 0, strPath );
					m_strZipName = strPath;
				#endif
			#else
			char* pstrTemp;

			pstrTemp = mktemp( ChUtil::GetPuebloDir() + "/" + CACHE_DIR +
								"/ChXXXXXX" );
			ASSERT( pstrTemp );
			m_strZipName = pstrTemp;
			#endif

			// if the OS creates a temp file delete it
			if ( ChUtil::FileExists( m_strZipName ) )
			{
				::DeleteFile( m_strZipName );
			}

			pstrOutFile = m_strZipName;

		}
		else
		{
			pstrOutFile = GetFileName();
		}
											// create the file locally

//				pFile = ::new fstream( pstrOutFile, ios::out | IOS_BINARY, filebuf::sh_none );
		pFile = ::new std::fstream( pstrOutFile, std::ios::out | IOS_BINARY );
		SetFile( pFile );

		ASSERT( GetFile() );

		if ( !GetFile()->is_open() )
		{
			SetState( HTTP_ERROR );
			SetError( CH_HTTP_ERROR_LOCAL_IN_USE );
			CloseStream();


		}
		else if ( !IsThreaded() )
		{
											/* The file is opened to write
												each block */
			GetFile()->close();
		}

	}
	else
	{
		SetState( HTTP_ERROR );
		SetError( CH_HTTP_ERROR_INVALID_HDR );
		CloseStream();
		m_luToProcess = 0;

		return;
	}

	if (iDelimType == STANDARD)
	{
		pstrBody += 4;						/* Points to the begining of
												the data */
	}
	else
	{
		pstrBody += 2;						/* Points to the begining of
												the data */

	}

	m_luToProcess -= (pstrBody - m_pstrBuffer);

	if ( m_luBodyLen > maxBufferSize )
	{
		m_luBufferSize =  maxBufferSize;
		char * pstrTemp = new char[ m_luBufferSize + 1];
		ASSERT( pstrTemp );
		ChMemCopy( pstrTemp, pstrBody, m_luToProcess );
		delete [] m_pstrBuffer;
		m_pstrBuffer = pstrTemp;
	}
	else if ( m_luBodyLen > midBufferSize )
	{
		m_luBufferSize =  midBufferSize;
		char * pstrTemp = new char[ m_luBufferSize + 1];
		ASSERT( pstrTemp );
		ChMemCopy( pstrTemp, pstrBody, m_luToProcess );
		delete [] m_pstrBuffer;
		m_pstrBuffer = pstrTemp;
	}
	else
	{
		ChMemCopy( m_pstrBuffer, pstrBody, m_luToProcess );
	}

	// Update the active conn status
	UpdateStatusInfo();

	// Initalize the stream
	InitStream();

	if ( m_luToProcess )
	{
		ProcessBody();
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::HandleError

------------------------------------------------------------------------------
Process HTTP headers on error
----------------------------------------------------------------------------*/
void ChHTTPInfo::HandleError( int iError )
{

	int		iDelimType = NON_STANDARD;
											// Do we have the complete header
											// this is the right delimitor

	char*	pstrBody = strstr( m_pstrBuffer, "\r\n\r\n" );

	if (pstrBody)
	{
		iDelimType = STANDARD;
	}
	else
	{
		pstrBody = strstr( m_pstrBuffer, "\n\n" );
		if (pstrBody)
		{
			iDelimType = NON_STANDARD;
		}
	}


	ChString strNewLocation;

										// process the header;
	if ( pstrBody )
	{										/* We have a partial body, go back
												and read the rest */
		char*	pstrType = m_pstrBuffer, *pstrNext, *pstrData;

		while (ChUtil::GetNextHeaderString( pstrType, pstrData, pstrNext, iDelimType ))
		{
			int		iType = GetHdrType( pstrType );

			switch( iType )
			{
				case LOCATION :
				{
					strNewLocation =  pstrData;
					strNewLocation.TrimLeft();
					strNewLocation.TrimRight();
					break;
				}
				default:
				{								// ignore for now
					break;
				}
			}
			pstrType = pstrNext;
		}


		if (iDelimType == STANDARD)
		{
			pstrBody += 4;						/* Points to the begining of
													the data */
		}
		else
		{
			pstrBody += 2;						/* Points to the begining of
													the data */
		}

		m_luToProcess -= (pstrBody - m_pstrBuffer);

		ChMemCopy( m_pstrBuffer, pstrBody, m_luToProcess );
		m_pstrBuffer[ m_luToProcess ] = 0;
	}
	else
	{
		m_pstrBuffer[0] = 0;	
	}

	switch ( iError )
	{
		case 302 :
		case 304 :
		{
			if ( !strNewLocation.IsEmpty() )
			{
				break;
			}
		}
		case 404 :	// not found
		case 408 :	// request timeout
		case 500 :  // internal server error
		case 502 :  // bad gateway
		case 504 :  // gateway timeout
		{

			if ( !(m_flOptions & ChHTTPConn::PrefetchURL ) &&
					 m_urlParts.GetURL().Left(strlen(old_url)) == old_url) 
			{
				ChString	strTempURL( m_urlParts.GetURL() );

				strNewLocation = ((ChString)new_url) +
							strTempURL.Right( strlen( old_url ) - 1 );
				#if !defined( CH_MSW )
				cerr << "Replacing " << strTempURL << " with "
					<< strNewLocation << endl;
				#endif
			}
		}
		default :
		{
			break;
		}
	}

	SetState( HTTP_ERROR );
	SetError( iError );

	if ( !strNewLocation.IsEmpty() )
	{

		SetState( HTTP_RETRY );

		// if there is any requests waiting for this URL
		// update them to the new URL
		UpdateWaitRequest( this, strNewLocation );

		ChHTTPInfo* pHTTPInfo;
		pHTTPInfo = new ChHTTPInfo( GetHTTPConn(), 
									strNewLocation,
									m_userData, 0, m_flOptions , m_pAltStreamMgr );
		// UE: copy the POST data across too, otherwise it'll be lost
		pHTTPInfo->SetPostData(m_strPostData, m_strPostData.GetLength());

		if ( !pHTTPInfo->GetError() )
		{	// if the url is bad then abort flag is set


			// Serialize HTTP access 	
			LockHTTP();

			ChHTTPInfo::GetRequestQueue().AddTail( pHTTPInfo );

			// Unlock serialization
			UnlockHTTP();

			if (  ChHTTPInfo::IsThreaded() ) 
			{
											/* Wake up the thread to process
												new connection */
				ChHTTPInfo::GetThreadMgr()->TriggerRequestEvent();
			}
			else
			{



									/* If we have less than max
									   connections, connect and send
									   the request */
				ProcessNonThreadedRequest();
			}
		}
	}

	if ( GetState() == HTTP_ERROR )
	{
		// close stream
		CloseStream();
	}
	m_luToProcess = 0;


}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::GetHdrType

------------------------------------------------------------------------------
Get the  HTTP header type. returns > 0 if we are interested in the header else -1
----------------------------------------------------------------------------*/

int ChHTTPInfo::GetHdrType( char* pstrHdr  )
{
	for ( int i = 0; i < sizeof( pstrHTTPTokens )/sizeof( char* ); i++ )
	{
		if ( !lstrcmpi( pstrHdr, pstrHTTPTokens[i] ))
		{
			return( i );
		}
	}
	return ( - 1 );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ProcessNoHeaderReply

------------------------------------------------------------------------------
Process HTTP headers
----------------------------------------------------------------------------*/
void ChHTTPInfo::ProcessNoHeaderReply()
{
	// since we dont have the content-encoding lets make
	// a guess based on data we have

	chuint32  i = 0;

	while( i < m_luToProcess && ( m_pstrBuffer[i] == TEXT( '\n' )
							 ||	m_pstrBuffer[i] == TEXT( '\r' ) ))
	{
		i++;
	}

	if ( m_pstrBuffer[i] == TEXT( '<' ) )
	{ 
		m_strMimeType = "text/html";
	}
	else if ( m_pstrBuffer[i] ==   TEXT( 'G' )  && 
			  m_pstrBuffer[i+1] == TEXT( 'I' ) &&
			  m_pstrBuffer[i+2] == TEXT( 'F' ) )
	{
		m_strMimeType = "image/gif";
	}
	else if ( m_pstrBuffer[i + 6] == TEXT( 'J' )  && 
			  m_pstrBuffer[i + 7] == TEXT( 'F' ) &&
			  m_pstrBuffer[i + 8] == TEXT( 'I' ) &&
			  m_pstrBuffer[i + 9] == TEXT( 'F' ) )
	{
		m_strMimeType = "image/jpeg";
	}
	else if ( m_pstrBuffer[i] == TEXT( '#' )  && 
			  m_pstrBuffer[i+1] == TEXT( 'V' ) &&
			  m_pstrBuffer[i+2] == TEXT( 'R' ) &&
			  m_pstrBuffer[i+3] == TEXT( 'M' ) &&
			  m_pstrBuffer[i+4] == TEXT( 'L' ) )
	{
		m_strMimeType = "x-world/x-vrml";
	}
	else
	{  // what is it ???
		m_strMimeType = "text/html";
	}

	m_strLastModified.Empty();
	m_luBodyLen = 0xFFFFFFFF;
	m_bbCompleted = true;	// We don't know how much data we have,

	std::fstream*	pFile;

	SetState( HTTP_IN_BODY );


	if ( m_strLocalName.IsEmpty() )
	{ // get the local file name
		MapURLToLocalName( ChHTTPSocketConn::GetMimeType( m_strMimeType ) );
	}

	// create the local file 

	//#ifdef CH_MSW
//		pFile = ::new fstream( GetFileName(), ios::out | IOS_BINARY, filebuf::sh_none );
		pFile = ::new std::fstream( GetFileName(), std::ios::out | IOS_BINARY );
	//#else
//		pFile = ::new fstream( GetFileName(), ios::out | IOS_BINARY );
	//#endif

	SetFile( pFile );

	ASSERT( GetFile() );

	if ( !GetFile()->is_open() )
	{
		SetError( CH_HTTP_ERROR_LOCAL_IN_USE );
		SetState( HTTP_ERROR );
		CloseStream();
		m_luToProcess = 0;
		return;
	}
	else if ( !IsThreaded() )
	{
										/* The file is opened to write
											each block */
		GetFile()->close();
	}

	// Update the active conn status
	UpdateStatusInfo();

	// Initalize the stream
	InitStream();


	if ( m_luToProcess )
	{
		ProcessBody();
	}

	// Allocate a bigger buffer
	m_luBufferSize =  midBufferSize;
	char * pstrTemp = new char[ m_luBufferSize + 1];
	ASSERT( pstrTemp );
	delete [] m_pstrBuffer;
	m_pstrBuffer = pstrTemp;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::IsStatusLine

------------------------------------------------------------------------------
Determines if the given buffer is a HTTP status line.
----------------------------------------------------------------------------*/

bool ChHTTPInfo::IsStatusLine( char* pstrBuffer, chuint iLen )
{

	char* pstrNext = strchr( pstrBuffer, LF );

	if (  pstrNext )
	{ // Looks like we have a status line
			// do we have the version
		chuint32  i = 0;
		while( i < iLen && pstrBuffer[i++] != TEXT( '/' ) );

		pstrNext = &pstrBuffer[i];
		if ( *(pstrNext - 1) == TEXT( '/' ) )
		{
			*(pstrNext - 1) = TEXT ('\0' );

			if ( lstrcmpi( pstrHTTPTokens[HTTP], m_pstrBuffer ))
			{ // invalid status line
				*(pstrNext - 1) = TEXT( '/' );

				return false;
			}
		}
		else
		{  // invalid
			return false;
		}
	}
	return ( false );
}


void ChHTTPInfo::SyncInfo( ChHTTPInfo* pInfo )
{
	m_strLocalName = pInfo->GetFileName();
	m_strMimeType  = pInfo->GetMimeType();
	m_luBodyLen    = pInfo->GetDataLength();
	m_iError	   = pInfo->GetError();	 
}


void ChHTTPInfo::AddToConnList( )
{
	if ( GetState() != HTTP_WAIT )
	{
	 	m_iActiveConn++;
	}
	m_httpConnList.AddHead( this );
}

void ChHTTPInfo::RemoveFromConnList( ChPosition posConn )
{
	if ( GetState() != HTTP_WAIT )
	{
	 	m_iActiveConn--;
	}
	m_httpConnList.Remove( posConn );
}


void ChHTTPInfo::RemoveConnection( )
{
	// Serialize HTTP access 	
	ChHTTPInfo::LockHTTP();

	ChPosition pos = ChHTTPInfo::GetConnectionList().GetHeadPosition();
											/* Delete records for all threads
												with the END_PROCESS state */
	while (0 != pos)
	{
		ChPosition	prevPos = pos;
		ChHTTPInfo*	pInfo = ChHTTPInfo::GetConnectionList().GetNext( pos );

		if (pInfo == this )
		{
			
			if ( pInfo->IsAborted() || !pInfo->CompletedRequest() ) 
		  	{
				TRACE1( "Aborting connection %s\n",  (const char *)pInfo->GetURL() );

				if (!pInfo->GetFileName().IsEmpty())
				{ 
					if (pInfo->GetFile() && pInfo->GetFile()->is_open())
					{
						pInfo->GetFile()->close();
					}

					if (pInfo->IsZipped() && ChUtil::FileExists( pInfo->GetZipFileName() ))
					{
						::DeleteFile( pInfo->GetZipFileName() );
					}

					if (ChUtil::FileExists( pInfo->GetFileName() ))
					{
						::DeleteFile( pInfo->GetFileName() );
					}  
				}
			}
									// We have a connection
			pInfo->RemoveFromConnList( prevPos );

		}
	}
	// Unlock serialization
	ChHTTPInfo::UnlockHTTP();
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::AddToSocketAddrList

------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

void ChHTTPInfo::AddToSocketAddrList( const ChString& strHost, 
									const sockaddr_in& addrIn )
{
	LockHTTP();

	sockaddr_in* pAddr = m_socketAddrList.Find( strHost );

	if ( 0 == pAddr )
	{
		m_socketAddrList.Insert( strHost, addrIn );			
	}

	UnlockHTTP();
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::FindSocketAddr

------------------------------------------------------------------------------
----------------------------------------------------------------------------*/
bool ChHTTPInfo::FindSocketAddr( const ChString& strHost, sockaddr_in& addrIn )
{
	bool boolFound = false;

	LockHTTP();		 

	sockaddr_in* pAddr = m_socketAddrList.Find( strHost );

	if ( pAddr )
	{
		boolFound = true;		
		addrIn = *pAddr;
	}

	UnlockHTTP(); 

	return boolFound;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::DeleteCache

------------------------------------------------------------------------------
Delete the local cached URL.
----------------------------------------------------------------------------*/

bool ChHTTPInfo::DeleteCache( const ChString& strURL )
{
	bool	 	boolReturn;
	ChString strKey = strURL;
	strKey.MakeLower();	  
	ChDBKey		dbKey( strKey );

	// Serialize HTTP access 	
	LockHTTP();

											// Delete the cache
	boolReturn = m_pURLDB->Delete( dbKey );

	if ( boolReturn )
	{
		if ( ChUtil::FileExists( GetFileName() ) )
		{
											// file does not exists
			::DeleteFile( GetFileName() );
		}
	}

	// Unlock serialization
	UnlockHTTP();

	return boolReturn;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::ClearCache

------------------------------------------------------------------------------
Delete the local cached URL.
----------------------------------------------------------------------------*/

bool ChHTTPInfo::ClearCache( )
{
	bool		boolDBOpen = false;

	{

		if ( !m_pURLDB )
		{
			OpenCacheDataBase();
			boolDBOpen = true;
		}

		if ( !m_pURLDB )
		{
			// close the database
			delete m_pURLDB;
		}

		// delete all the files in the cache directory
		void * pDir = ChUtil::OpenDirectory( m_strCacheDir, TEXT( "*.*" ), 0XFFFFFFFF );

		if ( pDir )
		{
			ChFileAttrs attrs;

			while( ChUtil::ReadDirectory( pDir,  &attrs, ChUtil::reqPath ) )
			{
				if ( attrs.uFileType == ChUtil::typeFile )
				{
					TRACE1( "Deleteing file %s\n", attrs.astrName );
					::DeleteFile( attrs.astrName );
				}
			}

			ChUtil::CloseDirectory( pDir );
		}

		if ( boolDBOpen )
		{
			delete m_pURLDB;
			m_pURLDB = NULL;
		}
		else
		{  // create a new database
			OpenCacheDataBase();
		}
	}

	return true;
}

#if defined( _DEBUG )
bool ChHTTPInfo::DisplayAllDBKeys( )
{

	{
		if ( !m_pURLDB )
		{
			return false;
		}

		TRACE( "Display all DB keys\n\n" );

		ChDBKey dbKey = m_pURLDB->GetFirstKey();

		while( dbKey.GetKeySize() )
		{
			ChString strURL( dbKey.GetKey(), (int)dbKey.GetKeySize() );
			ChDBData dbData = m_pURLDB->GetData( dbKey );

			ChString strFile = (char*)dbData.GetData() ;
			strFile = (char*)dbData.GetData() ;
			TRACE2( "URL %s, Local file %s\n", 
						((const char*)strURL), ((const char*)strFile) );

			dbKey = m_pURLDB->GetNextKey( dbKey );
		}

		TRACE( "End of all DB keys\n\n" );


	}

	return true;
}

#endif


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::EnforceCacheLimit

------------------------------------------------------------------------------
This method prunes the cache to the user requested size
----------------------------------------------------------------------------*/

bool ChHTTPInfo::EnforceCacheLimit( )
{
	bool		boolDBOpen = false;

	#if defined( CH_VRML_VIEWER )
	ChRegistry  reg( CH_COMPANY_NAME, CH_VRML_PRODUCT_NAME, CH_CACHE_GROUP );
	#else
	ChRegistry	reg( CH_CACHE_GROUP );
	#endif

	{
		if ( !m_pURLDB )
		{
			OpenCacheDataBase();
			boolDBOpen = true;
		}
 
 		chuint32 luTotalSize = 0;

		void * pDir = ChUtil::OpenDirectory( m_strCacheDir, TEXT( "*.*" ), 0XFFFFFFFF );

		if ( pDir )
		{
			ChFileAttrs attrs;

			while( ChUtil::ReadDirectory( pDir,  &attrs, ChUtil::reqSize) )
			{
				if ( attrs.uFileType == ChUtil::typeFile )
				{
					luTotalSize += attrs.luSize;
				}
			}

			ChUtil::CloseDirectory( pDir );
		}

		if ( luTotalSize )
		{
			chuint32	luCacheSize;

	 		reg.Read( CH_CACHE_SIZE, luCacheSize, CH_CACHE_SIZE_DEF );
			luCacheSize *= 1024;  // size specified in kilobytes

			if ( luTotalSize > luCacheSize  )
			{ // build a list of files and delete the oldest files first till 
			  //we go under the cache limit
			   	ChParamList fileList;
				ChPosition   pPos;

				ChString strDir = m_strCacheDir;
				strDir += PATH_SEPARATOR_CHAR;
				strDir+= URL_DBNAME;

				ChString strPag = strDir;

				strPag += ".pag";
				strDir += ".dir";
				bool boolCheckPag = true, boolCheckDir = true;



				void * pDir = ChUtil::OpenDirectory( m_strCacheDir, TEXT( "*.*" ), 0XFFFFFFFF );

				if ( pDir )
				{
					ChFileAttrs attrs;

					while( ChUtil::ReadDirectory( pDir,  &attrs, 
									ChUtil::reqSize | ChUtil::reqTime | ChUtil::reqPath ) )
					{
						if ( attrs.uFileType == ChUtil::typeFile )
						{
							// if the file is part of database don't add the file
							if ( boolCheckDir && strDir.CompareNoCase( attrs.astrName ) == 0 )
							{
								boolCheckDir = false;
								continue;
							}

							if ( boolCheckPag && strPag.CompareNoCase( attrs.astrName ) == 0 )
							{
								boolCheckPag = false;
								continue;
							}

							pChFileAttrs pAttrs;
							pChFileAttrs pElem;
							pAttrs = new ChFileAttrs;
							*pAttrs = attrs;

							if ( fileList.IsEmpty() )
							{
								fileList.AddHead( (chparam)pAttrs );
							}
							else
							{
								
								pPos = fileList.GetHeadPosition(); 

								while( pPos )
								{
									pElem = (pChFileAttrs)fileList.Get( pPos );	

									if ( ChUtil::CompareFileTime( pAttrs->mtime, pElem->mtime )  <= 0 )
									{
										break;
									}
									fileList.GetNext( pPos );	
								}
								// add it to the list
								if ( pPos )
								{
									fileList.InsertBefore( pPos, (chparam)pAttrs );	
								}
								else
								{
									fileList.AddTail( (chparam)pAttrs );
								}

							}
						}
					}
					ChUtil::CloseDirectory( pDir );
				}
				// we have a list of files sorted by time in ascending order, we will start deleting file
				// in ascending order till we go below the limit
				
				pPos = fileList.GetHeadPosition(); 

				while( pPos && luTotalSize > luCacheSize )
				{
					pChFileAttrs pElem;
					pElem = (pChFileAttrs)fileList.GetNext( pPos );	

					if ( ::DeleteFile( pElem->astrName ) )
					{
						luTotalSize -= pElem->luSize;	
					}
				}
				// Delete all the attrs structs allocated
				pPos = fileList.GetHeadPosition(); 

				while( pPos && luTotalSize > luCacheSize )
				{
					pChFileAttrs pElem;
					pElem = (pChFileAttrs)fileList.GetNext( pPos );	
					delete pElem;
				}
				fileList.Empty();

				// update the database
				ChDBKey dbKey = m_pURLDB->GetFirstKey();

				while (  dbKey.GetKeySize() )
				{

					ChDBData dbData = m_pURLDB->GetData( dbKey );

					ChString strFile = m_strCacheDir;
					strFile += PATH_SEPARATOR_CHAR;
					strFile += (char*)dbData.GetData() ;


					if (  !ChUtil::FileExists( strFile ) )
					{
						ChDBKey dbOldKey = dbKey;

						dbKey = m_pURLDB->GetNextKey( dbKey );

						m_pURLDB->Delete( dbOldKey );
					}
					else
					{
						dbKey = m_pURLDB->GetNextKey( dbKey );
					}
				}
			}
		}

		if ( boolDBOpen )
		{
			delete m_pURLDB;
			m_pURLDB = NULL;
		}
	}

	return true;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::Visited

------------------------------------------------------------------------------
This method returns true if the URL has been visted during the current session
----------------------------------------------------------------------------*/
bool ChHTTPInfo::Visited()
{
	return (ChHTTPInfo::m_httpVisited.Find( m_urlParts.GetURL() ) != NULL);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::CreateAndValidateCacheDir

------------------------------------------------------------------------------
This method creates the cache directory
----------------------------------------------------------------------------*/

bool ChHTTPInfo::CreateAndValidateCacheDir( ChString& strDir )
{
	if (!ChUtil::FileExists( strDir ))
	{
		ChUtil::CreateDirectoryTree( strDir );
		if (!ChUtil::FileExists( strDir ))
		{
			return false;
		}

	}
	return true;
}

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:13  uecasm
// Import of source tree as at version 2.53 release.
//
