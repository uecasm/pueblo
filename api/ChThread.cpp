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

#if defined( CH_MSW )

	#include "PbError.h"
	#include <io.h>

#endif

#if defined( CH_UNIX )

	#define _STREAM_COMPAT 1
	#include <iostream>
	#include <fstream>
	#include <unistd.h>

#endif // CH_UNIX

#include <string.h>

#include <ChTypes.h>
#include <ChHtpCon.h>
#include <ChDb.h>
#include <SocketXX.h>
#include <ChSock.h>
#include <ChUtil.h>

#include "ChHTPriv.h"
#include "ChHttpThreadMgr.h" 

CH_INTERN_FUNC( ChHTTPInfo * )
GetNextRequest( ChHttpThread* pThread );

CH_INTERN_FUNC( bool )
DoHTTPConnect( ChHTTPInfo *pHTTPInfo );


CH_INTERN_FUNC( void )
ProcessWaitRequest( ChHTTPInfo *pHTTPInfo  );

CH_INTERN_FUNC( void )
DoNotification( ChHTTPInfo *pHTTPInfo, ChHTTPInfo *pConnection  );

CH_DECLARE_SOCKET_ASYNC_HANDLER( httpAsyncSocketHandler )


#include <MemDebug.h>


/*----------------------------------------------------------------------------

	FUNCTION	||	DoHTTPConnect

------------------------------------------------------------------------------

Connect to a HTTP server and send the request to download a URL.
----------------------------------------------------------------------------*/
CH_INTERN_FUNC( bool )
DoHTTPConnect( ChHTTPInfo *pHTTPInfo )
{

	if ( pHTTPInfo->IsThreaded() )
	{
		ASSERT( false );
		#if	defined(CH_MSW ) && defined( WIN32 )
						// Read the data
		//AfxBeginThread( HTTPReadThread, (ptr)pHTTPInfo, THREAD_PRIORITY_BELOW_NORMAL );
		#endif
	}
	else
	{ // non-threaded request
		#if !defined( CH_MSW )

		if ( !pHTTPInfo->Connect() )
		{ // Notify error and cleanup connection

			pHTTPInfo->SetState( HTTP_ERROR );
			pHTTPInfo->SetError( CH_HTTP_ERROR_CONNECT );
			
			pHTTPInfo->CloseStream();

			pHTTPInfo->SetState( HTTP_END_PROCESS );

			// Update the global state and local connection state
			if ( pHTTPInfo->GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
			{
			 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalActive--;
			 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalWaiting--;
			}
			else
			{
			 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalActive--;
			 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalWaiting--;
			 	pHTTPInfo->GetHTTPConn()->m_connGlobalState.iTotalActive--;
			 	pHTTPInfo->GetHTTPConn()->m_connGlobalState.iTotalWaiting--;
			}


			DeleteConnection();
			return ( false );
		}
		// Update the global state and local connection state
		if ( pHTTPInfo->GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
		{
		 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalWaiting--;
		}
		else
		{
		 	pHTTPInfo->GetHTTPConn()->m_connState.iTotalWaiting--;
		 	pHTTPInfo->GetHTTPConn()->m_connGlobalState.iTotalWaiting--;
		}
											// Send the request
		if (!pHTTPInfo->SendRequest())
		{									// Request failed, notify module
											// Cleanup
			pHTTPInfo->SetState( HTTP_ERROR );

			pHTTPInfo->SetError( CH_HTTP_ERROR_CONNECT );

			pHTTPInfo->CloseStream();

			pHTTPInfo->SetState( HTTP_END_PROCESS );

			DeleteConnection();
			return false;
		}
		#else
		
		sockinetaddr sa;	
		
		if (!ChHTTPInfo::FindSocketAddr( pHTTPInfo->GetParts().GetHostName(),
											sa.m_sockAddrIn ))
		{
											/* On win16 and win32s we need to
												get the host name
												asynchronously */

			ChChacoSocket::AsyncGetHostByName( pHTTPInfo->GetParts().GetHostName(),
												httpAsyncSocketHandler,
												(chparam)pHTTPInfo );
			return true;
		}
		else
		{									/* I have the address but to make
												the connection asynchronous we
												will call this special
												ChChacoSocket call which will
												call our async handler
												asynchronously by posting a
												window message */

			ChChacoSocket::AsyncGetHostByName( sa,	httpAsyncSocketHandler,
												(chparam)pHTTPInfo );
			return true;
		}

		#endif
	}

	return true;

}

/*----------------------------------------------------------------------------

	FUNCTION	||	SendRequestIfAlive

------------------------------------------------------------------------------

	This function checks if the request is to an existing connection, if it
	is than the request is sent else it returns false

----------------------------------------------------------------------------*/

CH_INTERN_FUNC( bool )
SendRequestIfAlive( ChHTTPInfo *pNewConn )
{
											/* We need to resolve how this
												works, for now create separate
												connections */
	#if 0
	{
		ChPosition		pos;

		if (!ChHTTPInfo::GetConnectionList(), GetCount())
		{
			return false;
		}
											/* Check if we have a connection
												to the requested site */
		pos = GetConnectionList().GetHeadPosition();

		while (0 != pos)
		{
			ChHTTPInfo*		pConn = GetNextChPosition( &pos );

			if ((pConn.GetParts().iSocket == pNewConn.GetParts().iSocket) &&
				(lstrcmpi( pConn.GetParts().pHostName, pNewConn.GetParts().pHostName) == 0))
			{
											// We have a connection
				return ( true );
			}
		}
	}
	#endif  // 0

	return false;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	GetNextRequest

------------------------------------------------------------------------------

	This function gets the next request to be processed and puts it into
	the process list, if the request is already being processed it is put in to the
	Process queue and its state is set to HTTP_WAIT.

----------------------------------------------------------------------------*/

CH_INTERN_FUNC( ChHTTPInfo* )
GetNextRequest( ChHttpThread* pThread )
{
	ChHTTPInfo	*pHTTPPreFetchReq = 0;
	ChPosition	posPrefetch = 0;
	ChHTTPInfo	*pHTTPReq = 0;
	ChPosition	posReq;

	// Serialize HTTP access 	
	ChHTTPInfo::LockHTTP();

	// Delete all socket connection in the del list, By now we should be out of
	// the handler !
	if ( ChHTTPInfo::GetSocketDelList() && 
				ChHTTPInfo::GetSocketDelList()->GetCount() )
	{
		ChPosition posSocket = ChHTTPInfo::GetSocketDelList()->GetHeadPosition();

		while (0 != posSocket )
		{									/* Make sure that the new request
												is not currently being
												processed */
			sockinetbuf*		pSocket;

			pSocket = ChHTTPInfo::GetSocketDelList()->GetNext( posSocket );

			delete pSocket;
		}
		// empty the list
		ChHTTPInfo::GetSocketDelList()->Empty();

	}

	posReq = ChHTTPInfo::GetRequestQueue().GetHeadPosition();

	while (0 != posReq)
	{
		ChPosition	posCurrent = posReq;
											// get the oldest pending request

		pHTTPReq = ChHTTPInfo::GetRequestQueue().GetNext( posReq );

		
		if ( pHTTPReq->GetState() == HTTP_NOTIFY )
		{
			// for threaded we need the thread object to stream
			pHTTPReq->SetThread( pThread );

			if ( !(pHTTPReq->GetURLOptions() &  ChHTTPConn::PrefetchURL ) )
			{
				if ( 0 == pHTTPReq->GetError() )
				{
					pHTTPReq->UpdateStatusInfo();
					pHTTPReq->StreamURL();
					pHTTPReq->UpdateStatusInfo( false );
				}
				else
				{
					pHTTPReq->CloseStream();
				}
			}
			// we have a new connection, add it to our process list
			ChHTTPInfo::GetRequestQueue().Remove( posCurrent );
			// get the next request
			posReq = ChHTTPInfo::GetRequestQueue().GetHeadPosition();

			delete pHTTPReq;
			pHTTPReq = NULL;	// zap it, we have already processed this

		}
		else
		{
												// Do we have any connections ?

			ChPosition	posConn = ChHTTPInfo::GetConnectionList().GetHeadPosition();

			while (0 != posConn)
			{									/* Make sure that the new request
													is not currently being
													processed */
				ChHTTPInfo*		pHTTPConn;

				pHTTPConn = ChHTTPInfo::GetConnectionList().GetNext( posConn );

				if (pHTTPReq->GetURL() == pHTTPConn->GetURL())
				{
					pHTTPReq->SetState( HTTP_WAIT );

					TRACE1( "Duplicate request %s\n",
							(const char *)pHTTPReq->GetURL() );	

					// for threaded we need the thread object to stream
					pHTTPReq->SetThread( pHTTPConn->GetThread() );


						// We have a connection	which is doing the job
					pHTTPReq->AddToConnList();

								// Remove  the  request
					ChHTTPInfo::GetRequestQueue().Remove( posCurrent );

					// get the next request
					posReq = ChHTTPInfo::GetRequestQueue().GetHeadPosition();


					pHTTPReq = NULL;	// zap it, we have already processed this

					break; // break the loop and get the next request
				}
			}

			if ( pHTTPReq )
			{  
				if ( pHTTPReq->GetURLOptions() &  ChHTTPConn::PrefetchURL )
				{
					chint32 lMaxPrefetch = (ChHTTPSocketConn::GetMaxConnections() >> 1); // 50%

					if ( ChHTTPInfo::NumActive() > lMaxPrefetch )
					{	// leave some connections open for critical stuff
						pHTTPPreFetchReq = pHTTPReq;
						posPrefetch      = posCurrent; 
						pHTTPReq = 0; // zap it otherwise this will result in a connect
						continue;	
					}
				}

				// we have a new connection, add it to our process list
				ChHTTPInfo::GetRequestQueue().Remove( posCurrent );

				pHTTPReq->SetThread( pThread );
				pHTTPReq->AddToConnList();
				break;
			}
		}
	}

	if (  pHTTPReq == 0 && 	pHTTPPreFetchReq )
	{
		// we have only prefetch request in our list, process the request
		ChHTTPInfo::GetRequestQueue().Remove( posPrefetch );

		pHTTPPreFetchReq->SetThread( pThread );
		pHTTPPreFetchReq->AddToConnList();
		pHTTPReq = pHTTPPreFetchReq;
	}

	// Unlock serialization
	ChHTTPInfo::UnlockHTTP();

	return( pHTTPReq );

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ProcessWaitRequest

------------------------------------------------------------------------------
	This function is called after a URL has been downloaded to check if
	there is any request with HTTP_WAIT state. If there is one and if it
	corresponds to the current processed URL then we do the notification for
	this request.

----------------------------------------------------------------------------*/

CH_INTERN_FUNC( void )
ProcessWaitRequest( ChHTTPInfo *pHTTPInfo  )
{
	ChPosition	posConn;

	// Serialize HTTP access 	
	ChHTTPInfo::LockHTTP();
											// current active list

	posConn = ChHTTPInfo::GetConnectionList().GetHeadPosition();

	while (0 != posConn)
	{										/* Make sure that the new request
												is not currently being
												processed */
		ChHTTPInfo*		pHTTPConn;

		ChPosition	prevPos = posConn;
		pHTTPConn = ChHTTPInfo::GetConnectionList().GetNext( posConn );

		if (pHTTPConn->GetState() == HTTP_WAIT)
		{

			if (pHTTPInfo->GetURL() == pHTTPConn->GetURL())
			{
 				TRACE1( "Coming out of HTTP_WAIT %s\n",
 							(const char *)pHTTPConn->GetURL() );
				// remove the connection we are done with this
				pHTTPConn->RemoveFromConnList( prevPos );


				DoNotification( pHTTPInfo, pHTTPConn );

				delete pHTTPConn;

				posConn = ChHTTPInfo::GetConnectionList().GetHeadPosition();

			}
		}
	}

	// Unlock serialization
	ChHTTPInfo::UnlockHTTP();
}

/*----------------------------------------------------------------------------

	FUNCTION	||	UpdateWaitRequest

------------------------------------------------------------------------------
	This function is called if there is a request which fails
	and we attempt a retry. In this case we need to update all the wait state
	requests for the retry URL to the new url

----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( void )
UpdateWaitRequest( ChHTTPInfo *pHTTPInfo, const ChString strNewURL )
{
	ChPosition	posConn;

	// Serialize HTTP access 	
	ChHTTPInfo::LockHTTP();
											// current active list

	posConn = ChHTTPInfo::GetConnectionList().GetHeadPosition();

	while (0 != posConn)
	{										/* Make sure that the new request
												is not currently being
												processed */
		ChHTTPInfo*		pHTTPConn;

		pHTTPConn = ChHTTPInfo::GetConnectionList().GetNext( posConn );

		if (pHTTPConn->GetState() == HTTP_WAIT)
		{

			if (pHTTPInfo->GetURL() == pHTTPConn->GetURL())
			{
				pHTTPConn->GetParts().GetURLParts( strNewURL );
			}
		}
	}

	// Unlock serialization
	ChHTTPInfo::UnlockHTTP();


}

/*----------------------------------------------------------------------------

	FUNCTION	||	DoNotification

------------------------------------------------------------------------------
This performas all the module notification for requests in HTTP_WAIT state. After
the notification the request is set to HTTP_END_PROCESS.

----------------------------------------------------------------------------*/

CH_INTERN_FUNC( void )
DoNotification( ChHTTPInfo *pHTTPInfo, ChHTTPInfo *pConnection  )
{


	if ( pHTTPInfo->GetState() == HTTP_RETRY ) 
	{
		return;		// Another request has been issued.
	}
	// pConnection is a duplicate request, sync all info of the real
	// connection .
  	
  	pConnection->SyncInfo( pHTTPInfo );

 	if ( pHTTPInfo->GetState() == HTTP_ERROR || 
   		!pHTTPInfo->CompletedRequest()  || 
   		pHTTPInfo->IsAborted() ) 
	{  // if the previous request was unsuccessful, send the error
	   // message for this request also
	   	if ( 0 == pConnection->GetError() )
		{

		   	if ( pHTTPInfo->IsAborted()   )
			{
	   			pConnection->SetError( CH_HTTP_ERROR_USER_ABORT );
			}
			else
			{
	   			pConnection->SetError( CH_HTTP_ERROR_ABORT );
			}
		}

		// Create a new stream and notify the error
		pConnection->CloseStream();
		// Notify any request for the same url in our wait queue


		pConnection->SetState( HTTP_ERROR ); // this will set the completed flag
											 // so we will not have double notification
		pConnection->SetState( HTTP_END_PROCESS );
		return;
	}


	pConnection->SetError( 0 );

	pConnection->SetState( HTTP_DATA_COMPLETE );  // this will set the completed flag
											 	// so we will not have double notification

	// Stream this connection
	pConnection->UpdateStatusInfo();
	pConnection->StreamURL();
	pConnection->UpdateStatusInfo( false );

	pConnection->SetState( HTTP_END_PROCESS );

	return;
}




/*----------------------------------------------------------------------------

	FUNCTION	||	DeleteConnection

------------------------------------------------------------------------------

	This function delete the ChHTTPInfo object when the request has been
	satisfied or an error has occured.

----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( void )
DeleteConnection( void )
{

	// This function should be called only by non-threaded HTTP request	  
	if ( ChHTTPInfo::IsThreaded() )
	{
		TRACE( "Function is not thread safe\n" );
		ASSERT( false );
	}


	if (ChHTTPInfo::GetConnectionList().GetCount())
	{
		ChPosition		pos;

		pos = ChHTTPInfo::GetConnectionList().GetHeadPosition();
												/* Delete records for all threads
													with the END_PROCESS state */
		while (0 != pos)
		{
			ChPosition	prevPos = pos;
			ChHTTPInfo*	pInfo = ChHTTPInfo::GetConnectionList().GetNext( pos );

			if (pInfo->GetState() == HTTP_END_PROCESS)
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
				delete pInfo;
				pos = ChHTTPInfo::GetConnectionList().GetHeadPosition();
			}
		}
	}
										/* Process if there is any pending
												request if we are not using
												threads */

	if ( !ChHTTPInfo::IsThreaded() )
	{
		if (ChHTTPInfo::GetRequestQueue().GetCount())
		{
			ProcessNonThreadedRequest();
		}
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ProcessNonThreadedRequest

------------------------------------------------------------------------------

This function processes the HTTP requests for non-threaded systems.

----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( void )
ProcessNonThreadedRequest()
{

	while (ChHTTPInfo::NumActive() < ChHTTPSocketConn::GetMaxConnections() &&
			ChHTTPInfo::GetRequestQueue().GetCount() )
	{
		ChHTTPInfo*		pHTTPInfo;

		TRACE( "Processing request\n" );

		// Get the next connection
		pHTTPInfo = GetNextRequest( 0 );

		if ( NULL == pHTTPInfo )
		{
			if ( ChHTTPInfo::GetRequestQueue().GetCount() )
			{	// we have prefetch requests, we will process them when
				// one of the connections currently active closes.
				break;
			}
			else
			{
				continue;
			}
		}
		
		#if 0								// We need to connect to the server
		if (pHTTPInfo->GetConnOptions() &  CH_OPT_KEEP_ALIVE)
		{
											/* Check if we have any connections
												already */
			if (!SendRequestIfAlive( pHTTPInfo ))
			{
				if( !DoHTTPConnect( pHTTPInfo ) )
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}
		else
		#endif
		{
			if( !DoHTTPConnect( pHTTPInfo ) )
			{
				continue;
			}
		}
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	httpAsyncSocketHandler

------------------------------------------------------------------------------

	Handler for asyncronous socket events.  Passes the following parameters:

		socket
		luEvent

----------------------------------------------------------------------------*/

CH_IMPLEMENT_SOCKET_ASYNCHANDLER( httpAsyncSocketHandler )
{
	ChHTTPInfo	*pHTTPInfo;

	pHTTPInfo = (ChHTTPInfo *)pInfo->GetUserData();

	if ( pHTTPInfo )
	{
		// get the address info
		sockinetaddr sa;	
		WPARAM wParam;
		LPARAM lParam;	


		pInfo->GetParams( wParam, lParam );

		if (WSAGETASYNCERROR( lParam) == 0 )
		{
		  	hostent * pHostEntry = (hostent *)pInfo->GetBuf();
	        #if defined( CH_ARCH_32 )
			int iSize = sizeof( sa.m_sockAddrIn.sin_addr );
			if ( pHostEntry->h_length < iSize && pHostEntry->h_length > 0 )
			{
				iSize = pHostEntry->h_length;
			}
			memcpy( &sa.m_sockAddrIn.sin_addr, pHostEntry->h_addr, iSize );
			#else
			sa.m_sockAddrIn.sin_addr.s_addr = ((LPIN_ADDR)pHostEntry->h_addr)->s_addr;
			#endif
			sa.m_sockAddrIn.sin_family = pHostEntry->h_addrtype;

			ChHTTPInfo::AddToSocketAddrList( pHTTPInfo->GetParts().GetHostName(), 
													sa.m_sockAddrIn );
		}
		else
		{ // Notify error and cleanup connection

			pHTTPInfo->SetState( HTTP_ERROR );

			pHTTPInfo->SetError( CH_HTTP_ERROR_CONNECT );

			// Create a new stream and notify the error
			pHTTPInfo->CloseStream();
			// Notify any request for the same url in our wait queue

			ProcessWaitRequest( pHTTPInfo );
	
			pHTTPInfo->SetState( HTTP_END_PROCESS );
			DeleteConnection();
			return;
		}


		ChHTTPInfo::ConnectionResult result = pHTTPInfo->Connect( sa );

		if ( result == ChHTTPInfo::connRetry )
		{ // reconnect with new address
			DoHTTPConnect( pHTTPInfo );
			return;
		}

		if ( result == ChHTTPInfo::connFailed )
		{ // Notify error and cleanup connection

			pHTTPInfo->SetState( HTTP_ERROR );
			pHTTPInfo->SetError( CH_HTTP_ERROR_CONNECT );

			// Create a new stream and notify the error
			pHTTPInfo->CloseStream();
			// Notify any request for the same url in our wait queue
			ProcessWaitRequest( pHTTPInfo );

			pHTTPInfo->SetState( HTTP_END_PROCESS );
			DeleteConnection();
			return;
		}
											// Send the request
		if (!pHTTPInfo->SendRequest())
		{									// Request failed, notify module
											// Cleanup
			pHTTPInfo->SetState( HTTP_ERROR );
			pHTTPInfo->SetError( CH_HTTP_ERROR_SEND );

			// Create a new stream and notify the error
			pHTTPInfo->CloseStream();
			// Notify any request for the same url in our wait queue

			ProcessWaitRequest( pHTTPInfo );

			pHTTPInfo->SetState( HTTP_END_PROCESS );
			DeleteConnection();
			return;
		}
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	coreSocketHandler

------------------------------------------------------------------------------

	Handler for asyncronous socket events.  Passes the following parameters:

		socket
		luEvent

----------------------------------------------------------------------------*/

CH_IMPLEMENT_SOCKET_HANDLER( httpSocketHandler )
{
	ChHTTPInfo	*pHTTPConn;

	if ( CH_SOCK_EVENT_CONNECT == luEvent )
	{
		return;
	}

	pHTTPConn = (ChHTTPInfo *)socket.GetUserData();

	if ( pHTTPConn )
	{

		// Process the incoming data from the socket    
		if ( CH_SOCK_EVENT_READ == luEvent )
		{
			pHTTPConn->ProcessInput();
		}

		if ( !pHTTPConn->IsAborted() && pHTTPConn->GetState() == HTTP_IN_BODY 
				&& CH_SOCK_EVENT_CLOSE == luEvent )
		{ 
			// do the notification
		   pHTTPConn->OnDataComplete();

		}



		// if we are done reading all the data then close the connection
		if ( pHTTPConn->GetState() == HTTP_DATA_COMPLETE ||
			 pHTTPConn->GetState() == HTTP_ERROR ||
			 pHTTPConn->GetState() == HTTP_USE_LOCAL ||
			 pHTTPConn->GetState() == HTTP_RETRY ||
			 pHTTPConn->GetState() == HTTP_END_PROCESS ||
			 pHTTPConn->IsAborted() ||
			 CH_SOCK_EVENT_CLOSE == luEvent	 )
		{
			#if defined( CH_ARCH_16 )   
			
			if ( pHTTPConn->GetState() == HTTP_ERROR ) 
			{
				TRACE( "Error processing request\n" );
			}
			#endif 

			if ( pHTTPConn->IsAborted() || !pHTTPConn->CompletedRequest() ) 
			{
				pHTTPConn->SetError( pHTTPConn->IsAborted() 
										? CH_HTTP_ERROR_USER_ABORT 
										: CH_HTTP_ERROR_ABORT );   
			}


	
			if ( pHTTPConn->GetState() != HTTP_RETRY )
			{
				pHTTPConn->CloseStream();
				ProcessWaitRequest( pHTTPConn );
			}

			socket.SetUserData( (chparam) NULL );
			// we are done
			pHTTPConn->SetState( HTTP_END_PROCESS );

			DeleteConnection();   
			
		}
	} 
	else 
	{
		// The HTTP connection is closed from our perspective,
		// but data came down it anyways.  read & discard the data.
		long		lLen;
		char 		*buf;

		lLen = socket.GetBytesAvailable();

		if ( lLen )
		{
			buf = new char[ lLen ];
			while( lLen ) 
			{
				lLen -= (int)socket.read( buf, (int)lLen );
			}
			delete buf;
		}
	}
}

#if defined( CH_MSW )  && defined( CH_ARCH_32 )

/*----------------------------------------------------------------------------
	HTTPRequestThread
------------------------------------------------------------------------------

	Thread for processing user request for data.

----------------------------------------------------------------------------*/
UINT HTTPRequestThread( LPVOID pData )
{
	DWORD	dwStatus;
	bool   	boolShutdown = false;

	ChHttpThreadMgr* pMgr = (ChHttpThreadMgr*)pData;

#ifdef PBERROR_API
	PbError::SetThreadName("HTTP Request Thread");
#endif
	//TRACE( "Start request thread\n" );

	while (WAIT_FAILED !=
			(dwStatus =
				::WaitForMultipleObjects( pMgr->GetTotalEvents(),
											pMgr->GetEvents(), false,
							   				INFINITE )))
	{
		chint32				lEvent = dwStatus - WAIT_OBJECT_0;

 		if ( pMgr->IsShutdownEvent( lEvent ) )
		{

			if ( pMgr->ProcessShutdownEvent() )
			{
				// all worker threads have terminated, time to quit
				AfxEndThread( 0 );
				TRACE( "Terminating Request thread\n" );
				return ( 0 );
			}
		}
  		else if ( pMgr->IsWorkerDieEvent( lEvent ) )
		{
		
 			if ( pMgr->ProcessWorkerDieEvent() )
			{
				// all worker threads have terminated, time to quit
				AfxEndThread( 0 );
				TRACE( "Terminating Request thread\n" );
				return ( 0 );
			}
 			TRACE( "Event: End Thread\n" );
		}
		else if ( pMgr->IsWorkerIdleEvent( lEvent) )
		{
			pMgr->ProcessIdleEvent();
		}
		else
		{
			pMgr->ProcessRequestEvent();
		}
	}

	TRACE( "End request thread : Terminate on failure\n" );

	return ( 0 );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	HTTPReadThread

------------------------------------------------------------------------------

	Thread for processing data from the socket

----------------------------------------------------------------------------*/
void StartDownload( ChHTTPInfo* pHTTPInfo )
{
	//TRACE( "Start read thread\n" );
	// connect to the server
	if ( !pHTTPInfo->Connect() )
	{ // Notify error and cleanup connection

		pHTTPInfo->SetState( HTTP_ERROR );
		pHTTPInfo->SetError( CH_HTTP_ERROR_CONNECT );
		
		// Create a new stream and notify the error
		pHTTPInfo->CloseStream();
		// Notify any request for the same url in our wait queue
		ProcessWaitRequest( pHTTPInfo );
	}
	else if (!pHTTPInfo->SendRequest())	  // Send the request
	{									// Request failed, notify module
										// Cleanup
		pHTTPInfo->SetState( HTTP_ERROR );
		pHTTPInfo->SetError( CH_HTTP_ERROR_SEND );

		// Create a new stream and notify the error
		pHTTPInfo->CloseStream();
		// Notify any request for the same url in our wait queue
		ProcessWaitRequest( pHTTPInfo );
	}
	else
	{


		if (!pHTTPInfo->IsAborted() )
		{
												// Process the HTTP request
	   		pHTTPInfo->ProcessInput();
		}

		if ( !pHTTPInfo->IsAborted() && pHTTPInfo->GetState() == HTTP_IN_BODY )
		{ 
			// do the notification
		   pHTTPInfo->OnDataComplete();

		}

	
		if ( pHTTPInfo->GetState() != HTTP_RETRY )
		{
			// done with this stream
			pHTTPInfo->CloseStream();
			// Notify if there is a request in our process list in HTTP_WAIT state
			// for the current URL
			ProcessWaitRequest( pHTTPInfo );
		}
													/* Thread is going to terminate,
													notify the request thread to
													cleanup and to process other
													pending requests */

	}

	pHTTPInfo->SetState( HTTP_END_PROCESS );
	// we have to remove this from here if we implement persistant
	// Remove it from the connection list
	pHTTPInfo->RemoveConnection();

	delete pHTTPInfo;

	return;
}

/*----------------------------------------------------------------------------
	HTTPProcessDownload
------------------------------------------------------------------------------

	Thread for processing user request for data.

----------------------------------------------------------------------------*/
UINT HTTPProcessDownload( LPVOID pData )
{
	DWORD			dwStatus;
	ChHttpThread*	pThread = (ChHttpThread*)pData;
	
	TRACE( "Starting new Read thread\n" );
#ifdef PBERROR_API
	PbError::SetThreadName("HTTP Process Download");
#endif

	while ( true )
	{
	
		pThread->SetWorking( true );

		ChHTTPInfo* pJob = GetNextRequest( pThread );


		if ( pJob )
		{
			StartDownload( pJob );

		}
		else
		{
			// send an event to the manager that I am idle
			{  // if I am not critical then trigger idle event
				pThread->SetWorking( false );
				pThread->GetThreadMgr()->TriggerIdle();
			}

			while (WAIT_FAILED !=
					(dwStatus =
						::WaitForMultipleObjects( pThread->GetTotalThreadEvents(),
													pThread->GetHttpThreadEvents(), false,
									   				INFINITE )))
			{
				chint32				lEvent = dwStatus - WAIT_OBJECT_0;

				if ( lEvent == ChHttpThread::eventTerminate )
				{	 
					pThread->SetDead( true );
					pThread->GetThreadMgr()->TriggerTerminate();

					TRACE( "End read thread \n" );
					AfxEndThread( 0 );
					return ( 0 );
				}
				else
				{
					break;
				}
			}
		}
	}

#if 0			// no way to get here
	TRACE( "End request thread : Terminate on failure\n" );

	return ( 0 );
#endif
}

#endif	// defined( CH_MSW )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:54:50  uecasm
// Import of source tree as at version 2.53 release.
//
