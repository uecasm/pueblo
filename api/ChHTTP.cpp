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

#include <string.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#ifdef CH_UNIX
#include <stdlib.h>
#endif

#include <ChTypes.h>
#include <ChHtpCon.h>
#include <ChDb.h>
#include <ChReg.h>
#include <ChConst.h>
#include <SocketXX.h>
#include <ChClInfo.h>
#include "ChHttpCookie.h"
#include "ChHttpThreadMgr.h"


#include <ChUtil.h>

#include "ChHTPriv.h"

#if defined( CH_VRML_VIEWER )
#include "ChGrType.h"
#endif


#include <MemDebug.h>



#ifdef CH_UNIX
// We have these here because g++ can't handle static members of classes
// whose instances are always new'ed (and never global).  Sheesh
ChHTTPInfoList   ChHTTPInfo::m_httpRequestQueue;
ChHTTPInfoList   ChHTTPInfo::m_httpConnList;
#endif

			// Initialize all ChHTTPSocketConn statics
int 			ChHTTPSocketConn::m_iTotObjects    = 0;
int 			ChHTTPSocketConn::m_iMaxConnection = 0;
long 			ChHTTPSocketConn::m_luhttpOptions  = 0;
ChString 			ChHTTPSocketConn::m_strHTTPProxy;
chuint32		ChHTTPSocketConn::m_luHTTPProxyPort = 0;
ChString 			ChHTTPSocketConn::m_strFTPProxy;
chuint32		ChHTTPSocketConn::m_luFTPProxyPort  = 0;
ConnectionState	ChHTTPSocketConn::m_connGlobalState = { 0, 0, 0, 0 };
ChHttpCookie*   ChHTTPSocketConn::m_phttpCookie = 0;



/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChHTTPSocketConn class
----------------------------------------------------------------------------*/

ChHTTPSocketConn::ChHTTPSocketConn(	ChHTTPStreamManager* pStreamMgr, chuint32 flOptions  ) :
				ChHTTPConn( pStreamMgr, flOptions )
{
	InitHTTP();
}

											// Maximum connections allowed
void ChHTTPSocketConn::InitHTTP()
{
	ChMemClearStruct( &m_connState ); 

	// Initialize the default user agent string
	ChClientInfo	clientInfo( ChClientInfo::thisMachine );

	// UE: removed the crap about Winsock.
/*
	m_strUserAgent.Format( "Pueblo %s, %s %s, %s, Winsock Version %s, Description : %s",
							 LPCSTR(clientInfo.GetClientVersion().Format()),
							 LPCSTR(clientInfo.GetPlatformName()),
							 LPCSTR(clientInfo.GetPlatformVersion().Format()),
							 LPCSTR(clientInfo.GetProcessor()),
							 LPCSTR(clientInfo.GetSocketsBestVersion().Format()),
							 LPCSTR(clientInfo.GetSocketsDescription()) );
*/
	m_strUserAgent.Format( "Pueblo/UE %s, %s %s, %s",
							 LPCSTR(clientInfo.GetClientVersion().Format()),
							 LPCSTR(clientInfo.GetPlatformName()),
							 LPCSTR(clientInfo.GetPlatformVersion().Format()),
							 LPCSTR(clientInfo.GetProcessor()) );

	if (m_iTotObjects == 0)
	{
											// Read all the HTTP preferences
		ChString strCompany;

		#if defined( CH_VRML_VIEWER )
		{
			strCompany = CH_VRML_PRODUCT_NAME;
		}
		#else
		{
			strCompany = CH_PRODUCT_NAME;	// for pueblo
		}
		#endif

		UpdateHTTPPreferences( strCompany );
	
		// Initialize our internal HTTP engine
		ChHTTPInfo::InitHTTPInfo();

		// On Win32s and Win16, limit the number of connections to a max of 2
		if ( !ChHTTPInfo::IsThreaded() )
		{
			m_iMaxConnection = 4;
		}

		// Read the cookie file 
		m_phttpCookie = new ChHttpCookie;
		ASSERT( m_phttpCookie );
		m_phttpCookie->ReadCookieFile();
	}

	m_iTotObjects++; // Total number of objects created so far
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::~ChHTTPSocketConn

------------------------------------------------------------------------------

	Destructor for ChHTTPSocketConn class.

----------------------------------------------------------------------------*/

ChHTTPSocketConn::~ChHTTPSocketConn()
{
	--m_iTotObjects;

	if ( 0 == m_iTotObjects )
	{


		m_strHTTPProxy.Empty();
		m_strFTPProxy.Empty();

		ChHTTPConn::TermMimeManager( );
		ChHTTPInfo::TermHTTPInfo();	

		// Save the cookie file
		if ( !m_phttpCookie->IsSaved() )
		{
			m_phttpCookie->WriteCookieFile();
		}
		delete m_phttpCookie;
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::UpdateHTTPPreferences

------------------------------------------------------------------------------

	Updates all HTTP related preferences.

----------------------------------------------------------------------------*/


void ChHTTPSocketConn::UpdateHTTPPreferences( const ChString& strProduct )
{

	ChRegistry  regCache( CH_COMPANY_NAME, strProduct, CH_CACHE_GROUP );
	regCache.Read( CH_CACHE_OPTION, m_luhttpOptions, CH_CACHE_OPTION_DEF );

	ChRegistry reg( CH_COMPANY_NAME, strProduct, CH_NETWORK_GROUP );
	ChString		strSize;

	reg.Read( CH_MAX_CONNECTIONS, strSize, 
					CH_MAX_CONNECTIONS_DEF );

	m_iMaxConnection = atoi( strSize );

	if (  m_iMaxConnection <= 0 )
	{
		m_iMaxConnection = atoi( CH_MAX_CONNECTIONS_DEF );
	}
	
	// Read Proxy settings
	bool 	boolUseProxies;
	ChRegistry regProxy( CH_COMPANY_NAME, strProduct, CH_PROXIES_GROUP );
	regProxy.ReadBool( CH_PROXIES, boolUseProxies, CH_PROXIES_DEF );

	if ( boolUseProxies )
	{
		// Read all the host names for proxies
		regProxy.Read( CH_HTTP_PROXY, m_strHTTPProxy );
		regProxy.Read( CH_FTP_PROXY, m_strFTPProxy );

		// Read all port numbers if we have host names
		if ( !m_strHTTPProxy.IsEmpty() )
		{
			regProxy.Read( CH_HTTP_PROXY_PORT,  m_luHTTPProxyPort );
		}
		if ( !m_strFTPProxy.IsEmpty() )
		{
			regProxy.Read( CH_FTP_PROXY_PORT,   m_luFTPProxyPort );
		}
	}
	else
	{
		// Reset all proxy settings
		m_strHTTPProxy.Empty();
		m_luHTTPProxyPort = 0;
		m_strFTPProxy.Empty();
		m_luFTPProxyPort = 0;
	}

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::GetURL

------------------------------------------------------------------------------

	Get the URL requested from the local cache or from the remote server.

----------------------------------------------------------------------------*/


bool ChHTTPSocketConn::GetURL( const ChString& strURL, chparam userData,
							const char* pstrHostName, /* = 0  */
							chuint32 flOptions /* = CH_OPT_NOTIFY_ON_COMPLETE */,
							ChHTTPStreamManager* pStreamMgr /*= 0 */ )
{
	bool 			boolReturn = true;
	ChHTTPInfo*		pHTTPInfo;

	pHTTPInfo = new ChHTTPInfo( this, strURL, userData, pstrHostName,
								flOptions, pStreamMgr );
	ASSERT( pHTTPInfo );

	if ( pHTTPInfo->IsAborted() )
	{	// bad request
		pHTTPInfo->SetState( HTTP_NOTIFY );
	}
	else if ( pHTTPInfo->GetParts().GetScheme() == ChURLParts::typeFile )
	{  // if it is a local file notify
		if ( !(flOptions & ChHTTPConn::PrefetchURL) )
		{
			pHTTPInfo->SetState( HTTP_NOTIFY );
		}
		else
		{
			delete pHTTPInfo;
			return true;
		}
	}
	else if (flOptions & ChHTTPConn::returnCache )
	{

		if (pHTTPInfo->IsCached() )
		{									// Notify
			pHTTPInfo->SetError( 0 );
			pHTTPInfo->SetState( HTTP_NOTIFY );

		}
		else
		{
			pHTTPInfo->SetState( HTTP_NOTIFY );
			pHTTPInfo->SetError( CH_HTTP_ERROR_NOT_CACHED );

			boolReturn = false;
		}
											// Delete the info
	}
	else if ( (flOptions & ChHTTPConn::PrefetchURL)  && pHTTPInfo->IsCached() 
				&& pHTTPInfo->Visited() ) 
	{	// we have it prefetched already

		delete pHTTPInfo;
		return true;
	}
	else if ( !(flOptions & ChHTTPConn::ReloadURL) )
	{
		bool boolCached = false;

   		if (pHTTPInfo->IsCached() && m_luhttpOptions & CH_CACHE_VERIFY_NEVER )
		{
			boolCached = true;
		}
   		if (pHTTPInfo->IsCached() && ( m_luhttpOptions & CH_CACHE_VERIFY_PER_SESSION )
   				&& pHTTPInfo->Visited() )
		{
			boolCached = true;
		}

		if ( boolCached )
		{									
			pHTTPInfo->SetError( 0 );
			pHTTPInfo->SetState( HTTP_NOTIFY );
		}
	}
											/* Add this to the request queue
												and wake up the thread */

	#if	defined(CH_MSW ) && defined( WIN32 )

	if (  ChHTTPInfo::IsThreaded() ) 
	{
		// Serialize HTTP access 	
		ChHTTPInfo::LockHTTP();

											/* Add the request to my queue,
												this queue is processed by
												the request processing thread
												HTTPRequestThread */

		ChHTTPInfo::GetRequestQueue().AddTail( pHTTPInfo );


		// Unlock serialization
		ChHTTPInfo::UnlockHTTP();

											/* Wake up the thread to process
												new connection */
		ChHTTPInfo::GetThreadMgr()->TriggerRequestEvent();
	}
	else
	#endif // CH_MSW
	{
		ChHTTPInfo::GetRequestQueue().AddTail( pHTTPInfo );

											/* If we have less than max
												connections, connect and send
												the request */
		ProcessNonThreadedRequest();
	}

	return boolReturn;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::PostURL

------------------------------------------------------------------------------

	Get the URL requested from the remote server.

----------------------------------------------------------------------------*/


bool ChHTTPSocketConn::PostURL( const ChString& strURL, const char* pstrBody,
							chint32 lLen /* = -1 */, chparam userData /* = 0 */,
							const char* pstrDefURL /* = 0 */, 
							chuint32 flOptions /* = CH_OPT_NOTIFY_ON_COMPLETE */,
							ChHTTPStreamManager* pStreamMgr /* = 0 */ )
{
	ChHTTPInfo*		pHTTPInfo;

	ASSERT( pstrBody );

	pHTTPInfo = new ChHTTPInfo( this, strURL, userData, pstrDefURL,
								flOptions | ChHTTPConn::UsePostMethod, pStreamMgr );
	ASSERT( pHTTPInfo );

	if ( pHTTPInfo->IsAborted() )
	{
		pHTTPInfo->SetState( HTTP_NOTIFY );
	}
	else
	{
		// Set up the post data and data length
		pHTTPInfo->SetPostData( pstrBody, lLen );
	}

											/* Add this to the request queue
												and wake up the thread */

	#if	defined(CH_MSW ) && defined( WIN32 )

	if (  ChHTTPInfo::IsThreaded() ) 
	{
		// Serialize HTTP access 	
		ChHTTPInfo::LockHTTP();

											/* Add the request to my queue,
												this queue is processed by
												the request processing thread
												HTTPRequestThread */

		ChHTTPInfo::GetRequestQueue().AddTail( pHTTPInfo );

		// Unlock serialization
		ChHTTPInfo::UnlockHTTP();

											/* Wake up the thread to process
												new connection */

		ChHTTPInfo::GetThreadMgr()->TriggerRequestEvent();
	}
	else
	#endif // CH_MSW
	{
		ChHTTPInfo::GetRequestQueue().AddTail( pHTTPInfo );

											/* If we have less than max
												connections, connect and send
												the request */
		ProcessNonThreadedRequest();
	}

	return true;
}

 /*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::ShutDownHTTP

------------------------------------------------------------------------------

	cleanup all connections.

----------------------------------------------------------------------------*/
void ChHTTPSocketConn::ShutDownHTTP()
{
	if ( m_iTotObjects  > 0 )
	{
		m_iTotObjects = -1;

		m_strHTTPProxy.Empty();
		m_strFTPProxy.Empty();

		ChHTTPInfo::TermHTTPInfo();
		ChHTTPConn::TermMimeManager( );

		// Save the cookie file
		if ( m_phttpCookie && !m_phttpCookie->IsSaved() )
		{
			m_phttpCookie->WriteCookieFile();
		}
		delete m_phttpCookie;
		m_phttpCookie = 0;

	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::AbortRequests

------------------------------------------------------------------------------

	Check if  the URL  is in the local cache.

----------------------------------------------------------------------------*/
void ChHTTPSocketConn::AbortRequests( bool boolAbortPrefetch /* = false */,
						ChHTTPStreamManager* pStreamMgr /*= 0 */)
{
	ChHTTPInfo::CloseConnections( this, boolAbortPrefetch, pStreamMgr ); 
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::IsURLCached

------------------------------------------------------------------------------

	Check if  the URL  is in the local cache.

----------------------------------------------------------------------------*/

bool ChHTTPSocketConn::IsURLCached( const ChString& strURL, const char* pstrDefURL /* = 0  */ )
{
	bool 			boolReturn = true;
	ChHTTPInfo*		pHTTPInfo;

	pHTTPInfo = new ChHTTPInfo( this, strURL, 0, pstrDefURL, ChHTTPConn::returnCache, 0 );
	ASSERT( pHTTPInfo );

	if (!pHTTPInfo->IsCached())
	{
		boolReturn = false;
	}
	delete pHTTPInfo;						// delete the info

	return boolReturn;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::IsURLCached

------------------------------------------------------------------------------

	Check if  the URL  is in the local cache.

----------------------------------------------------------------------------*/

bool ChHTTPSocketConn::IsVisitedURL( const ChString& strURL, const char* pstrDefURL /* = 0  */ )
{
	bool 			boolReturn = true;
	ChHTTPInfo*		pHTTPInfo;

	pHTTPInfo = new ChHTTPInfo( this, strURL, 0, pstrDefURL, ChHTTPConn::returnCache, 0 );
	ASSERT( pHTTPInfo );

	if (!pHTTPInfo->IsCached() || pHTTPInfo->IsPrefetched() )
	{
		boolReturn = false;
	}
	delete pHTTPInfo;						// delete the info

	return boolReturn;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::GetCachedURL

------------------------------------------------------------------------------

	Get the URL  from the local cache.

----------------------------------------------------------------------------*/

bool ChHTTPSocketConn::GetCachedURL( const ChString& strURL, ChString& strName,
										ChString& strMimeType,
										const char* pstrDefURL /* = 0 */, 
										chuint32 flOption /* = CH_OPT_RETURN_VISITED */ )
{
	bool		 	boolReturn = true;
	ChHTTPInfo*		pHTTPInfo;

	pHTTPInfo = new ChHTTPInfo( this, strURL, 0, pstrDefURL, ChHTTPConn::returnCache, 0 );
	ASSERT( pHTTPInfo );

	if (!pHTTPInfo->IsCached())
	{
		boolReturn = false;
		strName.Empty();
	}
	else
	{
		strName = pHTTPInfo->GetFileName();
		strMimeType = pHTTPInfo->GetMimeType();
		if ( flOption & ChHTTPConn::returnVisited )
		{
			if ( !pHTTPInfo->Visited() )
			{
				boolReturn = false;
			}
		}
	}
	delete pHTTPInfo;						// Delete the info

	return boolReturn;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::DeleteCachedURL

------------------------------------------------------------------------------

	Delete cached URL.

----------------------------------------------------------------------------*/

bool ChHTTPSocketConn::DeleteCachedURL( const ChString& strURL, const char* pstrDefURL /* = 0 */ )
{
   	bool		 	boolReturn = true;
	ChHTTPInfo*		pHTTPInfo;

	pHTTPInfo = new ChHTTPInfo( this, strURL, 0, pstrDefURL, ChHTTPConn::returnCache, 0 );
	ASSERT( pHTTPInfo );

	if (!pHTTPInfo->IsCached())
	{
		boolReturn = false;
	}
	else
	{
		boolReturn = pHTTPInfo->DeleteCache( pHTTPInfo->GetParts().GetURL() );
	}
	delete pHTTPInfo;						// Delete the info

	return boolReturn;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::NumActive

------------------------------------------------------------------------------

	Gets the total active connections.

----------------------------------------------------------------------------*/
int ChHTTPSocketConn::NumActive()
{
	if ( GetConnOptions() & connLocalState )
	{
		return m_connState.iTotalActive;
	}
	else
	{
		return m_connGlobalState.iTotalActive;
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::NumActive

------------------------------------------------------------------------------

	Gets the total active connections.

----------------------------------------------------------------------------*/
bool ChHTTPSocketConn::IsActive()
{
	if ( GetConnOptions() & connLocalState )
	{
		return ( (m_connState.iTotalActive + m_connState.iTotalWaiting) > 0 );
	}
	else
	{
		return ((m_connGlobalState.iTotalActive + m_connGlobalState.iTotalWaiting) > 0);
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::GetHTTPConnState

------------------------------------------------------------------------------

	Gets the current state of all active HTTP connections.

----------------------------------------------------------------------------*/
void ChHTTPSocketConn::GetProgressMsg( ChString& strProgressMessage, int& iPercentComplete )
{
	int 		iTotWait;
	int 		iTotalActive;
	chuint32 	luCompleted;
	chuint32 	luTotal;

	iPercentComplete = 0;
	strProgressMessage.Empty();
	
	if ( GetConnOptions() & connLocalState )
	{
		iTotalActive =  m_connState.iTotalActive;
		iTotWait 	 = m_connState.iTotalWaiting;
		luCompleted  = m_connState.luTotalReceived;
		luTotal 	 = m_connState.luTotalBytes;
	}
	else
	{
		iTotalActive = m_connGlobalState.iTotalActive;
		iTotWait 	 = m_connGlobalState.iTotalWaiting;
		luCompleted  = m_connGlobalState.luTotalReceived;
		luTotal 	 = m_connGlobalState.luTotalBytes;
	}

	if ( iTotWait > 0 || iTotalActive > 0 )
	{
		if ( 0 == iTotalActive )
		{
			if ( iTotWait == 1 )
			{
				strProgressMessage = "[ 1 Connection: Waiting for reply. ]" ;
			}
			else
			{
				strProgressMessage.Format( "[ %d Connections: Waiting for reply. ]", 
														iTotWait );
			}
		}
		
		if ( iTotalActive > 0 )
		{
		   	ChString 	strProgress;
			if ( luCompleted > luTotal )
			{
				luCompleted = luTotal;
			}

			if ( luTotal )
			{
				iPercentComplete =  (int)((luCompleted * 100 )/ luTotal);
			}

			if ( iTotalActive  == 1 )
			{
				strProgress.Format( " [ 1 Connection: Received %ld of %ld bytes (%ld%% Completed). ]",
								 			luCompleted, luTotal, iPercentComplete );
			}
			else
			{
				strProgress.Format( " [ %d Connections: Received %ld of %ld bytes (%ld%% Completed). ]",
								 			iTotalActive , luCompleted, luTotal, iPercentComplete );
			}

			strProgressMessage += strProgress;
		}
	}
}

#if 0
void ChHTTPSocketConn::GetHTTPConnState( int& iTotalConnections, int& iTotInWait,
									chuint32& luBytesRead,
									chuint32& luTotalBytes )
{
	iTotalConnections 	= ChHTTPInfo::GetTotalConn();
	if ( iTotalConnections )
	{
		iTotInWait		= ChHTTPInfo::GetTotalConnInWait();
		luBytesRead 	= ChHTTPInfo::GetTotalBytesRead();
		luTotalBytes 	= ChHTTPInfo::GetTotalBytesToRead();
	}
	else
	{
		iTotInWait	= 0;
	 	luBytesRead = 0;
		luTotalBytes = 0;
	}
}
#endif
/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::ClearCache

------------------------------------------------------------------------------

Clears the cache directory

----------------------------------------------------------------------------*/
bool ChHTTPSocketConn::ClearCache( )
{
	return ChHTTPInfo::ClearCache( );	
}
/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::EnforceCacheLimit

------------------------------------------------------------------------------

	Limis the disk cache to user specified size.

----------------------------------------------------------------------------*/
bool ChHTTPSocketConn::EnforceCacheLimit( )
{ 
	return ChHTTPInfo::EnforceCacheLimit( );
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPSocketConn::CreateAndValidateCacheDir

------------------------------------------------------------------------------
This method creates the cache directory
----------------------------------------------------------------------------*/

bool ChHTTPSocketConn::CreateAndValidateCacheDir( ChString& strDir )
{

	#if defined( CH_VRML_VIEWER )
	ChRegistry regCache( CH_COMPANY_NAME, CH_VRML_PRODUCT_NAME, CH_CACHE_GROUP );
	#else
	ChRegistry	regCache( CH_CACHE_GROUP );
	#endif

	regCache.Read( CH_CACHE_OPTION, m_luhttpOptions, CH_CACHE_OPTION_DEF );
	
	return ChHTTPInfo::CreateAndValidateCacheDir( strDir );
}  


bool ChHTTPSocketConn::CloseAllConnections( bool boolOnlyGlobal /* = true */ )
{
	ChHTTPInfo::CloseAllConnections( boolOnlyGlobal ); 
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
///////////	  ChHttpUrlParts
//////////
///////////////////////////////////////////////////////////////////////////////////
 
bool ChHttpUrlParts::UsingProxy()
{
	switch( GetScheme() )
	{
		case typeHTTP :
		{
			if ( !ChHTTPSocketConn::GetHTTPProxy().IsEmpty() )
			{
				return true;
			}
		}
		case typeFTP :
		{
			if ( !ChHTTPSocketConn::GetFTPProxy().IsEmpty() )
			{
				return true;
			}
		}
		default :
		{
			break;
		}
	}
	return  false; 
}



const char*	ChHttpUrlParts::GetHostName()		
{ 	// if we are using proxy server then use that as the host name
	switch( GetScheme() )
	{
		case typeHTTP :
		{
			if ( !ChHTTPSocketConn::GetHTTPProxy().IsEmpty() )
			{
				return ChHTTPSocketConn::GetHTTPProxy();
			}
		}
		case typeFTP :
		{
			if ( !ChHTTPSocketConn::GetFTPProxy().IsEmpty() )
			{
				return ChHTTPSocketConn::GetFTPProxy();
			}
		}
		default :
		{
			break;
		}
	}
	return  ChURLParts::GetHostName(); 
}

const char* ChHttpUrlParts::GetAbsPath()		
{ 
	switch( GetScheme() )
	{
		case typeHTTP :
		{
			if ( !ChHTTPSocketConn::GetHTTPProxy().IsEmpty() )
			{
				return GetURL();
			}
		}
		case typeFTP :
		{
			if ( !ChHTTPSocketConn::GetFTPProxy().IsEmpty() )
			{
				return GetURL();
			}
		}
		default :
		{
			break;
		}
	}
	return  ChURLParts::GetAbsPath(); 
}

const char* ChHttpUrlParts::GetRelPath()		
{ 
	switch( GetScheme() )
	{
		case typeHTTP :
		{
			if ( !ChHTTPSocketConn::GetHTTPProxy().IsEmpty() )
			{
				return NULL;
			}
		}
		case typeFTP :
		{
			if ( !ChHTTPSocketConn::GetFTPProxy().IsEmpty() )
			{
				return NULL;
			}
		}
		default :
		{
			break;
		}
	}
	return  ChURLParts::GetRelPath(); 
}

int	ChHttpUrlParts::GetPortNumber()		
{ 
	switch( GetScheme() )
	{
		case typeHTTP :
		{
			if ( !ChHTTPSocketConn::GetHTTPProxy().IsEmpty() )
			{
				return ChHTTPSocketConn::GetHTTPProxyPort();
			}
		}
		case typeFTP :
		{
			if ( !ChHTTPSocketConn::GetFTPProxy().IsEmpty() )
			{
				return ChHTTPSocketConn::GetFTPProxyPort();
			}
		}
		default :
		{
			break;
		}
	}
	return  ChURLParts::GetPortNumber(); 
}

// $Log$

// Local Variables: ***
// tab-width:4 ***
// End: ***
