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

	This file consists of the Chaco DDE interface to WEB clients.

----------------------------------------------------------------------------*/

// $Header$

//#include "grheader.h"
#include "headers.h"

#include <stdlib.h>
#include <string.h>

#if !defined( CH_VRML_VIEWER )
#include <ChCore.h>
#endif

//#if defined( CH_NOTIFY_MODULE )
//#include <ChMsgTyp.h>
//#endif

#include <ChConst.h>
#include <ChHttpStream.h>
#include <ChUrlMap.h>
#include <ChUtil.h>
#include <ChDDE.h>
#include <ChHtpCon.h>

#include "ChDDEPrv.h"

#include <MemDebug.h>


#ifdef UNICODE
#define STRICMP wcsicmp
#define ITOA(c, sz, b) (itoa(sizeof(szA), szA, b), mbstowcs(sz, szA, b), sz)
#else
#define STRICMP stricmp
#define ITOA itoa
#endif  // UNICODE

#define CH_USE_SOCKET


#define DDE_ERR_MSG		"Unable to establish converstion with a web browser. Please start the browser application and retry the operation."


//	Static initialization.
ChVisitedList				ChHTTPDDE::m_visitedList;
ChConnActiveList			ChHTTPDDE::m_connActiveList;
ChConnWaitList				ChHTTPDDE::m_connWaitList;
unsigned	long			ChHTTPDDE::m_numObjects = 0;
ChString						ChHTTPDDE::m_strStatusMsg;
DWORD						ChHTTPDDE::m_dwProgress   = 0;
DWORD						ChHTTPDDE::m_dwMaxRange   = 0;

CRITICAL_SECTION			ddeSync;

#if defined( CH_USE_SOCKET )
ChHTTPSocketConn*			httpSocketConn = 0;
#endif


#define CH_DDE_NOTIFY_CLASS	"Chaco-DDE-Notification-Class"
LRESULT CALLBACK EXPORT DDENotifyProc( HWND hwnd, UINT  uMsg,
											WPARAM wParam, LPARAM lParam );

class ChDDEConnInfo;

class ChDDEConnStream  : public ChHTTPStream
{
	public :

		ChDDEConnStream() : m_pUserStreamData( 0 ), m_pDDEInfo(0),
							m_uStreamOption(0)
					{
					}
		virtual ~ChDDEConnStream()
					{
					}

	    // methods provided my the base class
	    virtual const ChString& GetURL();
	    virtual const ChString& GetMimeType();
	    virtual const ChString& GetCacheFilename();
		virtual const char* GetErrorMsg()  { return m_strDummy; }
	    virtual const ChString& GetContentEncoding()	{ return m_strDummy; } 
	    virtual const ChString& GetLastModified()		{ return m_strDummy; } 
	    virtual const ChString& GetTargetWindowName() { return m_strDummy; } 
	    virtual long GetContentLength()				{ return 0; }

	    virtual void* GetStreamPrivateData()		{ return m_pUserStreamData; }
	     // Attributes
		virtual void SetStreamPrivateData( void *pData )
							{ m_pUserStreamData = pData; }
	public : 
		void*			m_pUserStreamData;
		ChDDEConnInfo*	m_pDDEInfo;  		
 		UINT			m_uStreamOption;
		ChString			m_strDummy;
 
};



/////////////////////////////////////////////////////////////////////////////
// ChDDEConnInfo 
// This class maintains information about the requested URL. There will be one
// instance of this object per GetURL request. If the request is new then
// this goes into the active list and is removed when ViewDocFile is received.
// If this is a duplicate then it goes into the wait list.
class ChDDEConnInfo	
{
	public :
		enum tagDDEWndMsg { msgGetURL = 1, msgRegisterViewer };
		ChDDEConnInfo(ChHTTPDDE* pDDE, ChString& strURL, chuint32 flOptions, 
										chparam userData ) : m_phttpDDE(pDDE), 
													   m_strURL( strURL ),
													   m_userData( userData ),
													   m_flOptions( flOptions ) {}
		~ChDDEConnInfo() {}
		ChHTTPDDE* GetHTTPDDE()						{ return m_phttpDDE; }
		chparam    GetUserData()					{ return m_userData; }
		const ChString&    GetURL()					{ return m_strURL; }
		const ChString&    GetMimeType()				{ return m_strMimeType; }
		const ChString&    GetFileName()				{ return m_strCacheFile; }
		chuint32   GetOptions()						{ return m_flOptions; }


		void DoLoadCompeteNotification( ChString& strFile,  ChString& strMimeType );
		void ChDDEConnInfo::DoErrorNotification(  int iError  );


	private :
		ChHTTPDDE 		*m_phttpDDE;
		ChDDEConnStream	m_ddeStream;
		chparam 		m_userData;
		ChString 			m_strURL;
		ChString 			m_strMimeType;
		ChString 			m_strCacheFile;
		int				iError;
		chuint32		m_flOptions;
};

/////////////////////////////////////////////////////////////////////////////
// ChVisitedInfo 
// This class maintains the URL's visted during this session
class ChVisitedInfo
{
	public :
		ChVisitedInfo( ChString& strLocalFile, ChString strMimeType ) : 
								m_strLocalFile( strLocalFile ), 
								m_strMimeType( strMimeType )
		{}

		~ChVisitedInfo()  {}

		ChString& GetLocalFileName()
				{
					return m_strLocalFile;
				}
		ChString& GetMimeType()
				{
					return m_strMimeType;
				}
	private :
		ChString m_strLocalFile;
		ChString m_strMimeType;
};



/////////////////////////////////////////////////////////////////////////////
// ChHTTPConnStream 	Implementation



const ChString& ChDDEConnStream::GetURL()
{
	return m_pDDEInfo->GetURL();
}

const ChString& ChDDEConnStream::GetMimeType()
{
	return m_pDDEInfo->GetMimeType();
}

const ChString& ChDDEConnStream::GetCacheFilename()
{
	return m_pDDEInfo->GetFileName();
}


/////////////////////////////////////////////////////////////////////////////
// ChDDEConnInfo 	Implementation


void ChDDEConnInfo::DoLoadCompeteNotification( ChString& strFile, ChString& strMimeType )
{
	if ( !m_phttpDDE->GetStreamMgr() )
	{
		return;
	}

	m_strMimeType = strMimeType;
	m_strCacheFile = strFile;
	m_ddeStream.m_pDDEInfo = this;

	m_ddeStream.m_uStreamOption = m_phttpDDE->GetStreamMgr()->
				NewStream(  GetUserData(), &m_ddeStream, false );

	if ( m_ddeStream.m_uStreamOption & ChHTTPStreamManager::streamAsFile )
	{
		m_phttpDDE->GetStreamMgr()->StreamAsFile( GetUserData(), 
										&m_ddeStream, strFile );	
	}

	
	m_phttpDDE->GetStreamMgr()->DestroyStream( GetUserData(), &m_ddeStream, 0 );	

}

void ChDDEConnInfo::DoErrorNotification( int iError )
{
	if ( !m_phttpDDE->GetStreamMgr() )
	{
		return;
	}

	m_ddeStream.m_pDDEInfo = this;
	m_ddeStream.m_uStreamOption = m_phttpDDE->GetStreamMgr()->
				NewStream(  GetUserData(), &m_ddeStream, false );

	m_phttpDDE->GetStreamMgr()->DestroyStream( GetUserData(), &m_ddeStream, iError );	
}


/////////////////////////////////////////////////////////////////////////////
// ChHTTPDDE implementation 


ChHTTPDDE::ChHTTPDDE( ChHTTPStreamManager* pStreamMgr, chuint32 flOptions  )	
													:  ChHTTPConn( pStreamMgr, flOptions  ),
													   m_pDDE( 0 ),
													   m_boolStopLoading(0),
										   			   m_boolConnectionValid(0),
													   m_boolNotifyDone( 0 ),
													   m_dwTransactionID( 0 ),
													   m_dwWindowID( 0 ),
													   m_hWndDDE(0)
{
	InitDDEConn();
}



void ChHTTPDDE::InitDDEConn()
{

    /*
     * Here we tell DDEML what we will be doing.
     *
     * 1) We let it know our callback proc address - MakeProcInstance
     *      is called just to be more portable.
     */

	if ( 0 == m_numObjects )
	{

		WNDCLASS	wc;
		wc.hInstance = AfxGetInstanceHandle( );
		wc.lpszClassName = CH_DDE_NOTIFY_CLASS;
		wc.lpfnWndProc	= DDENotifyProc;
		wc.hCursor = 0;
		wc.hIcon = 0;
		wc.hbrBackground = 0;
		wc.lpszMenuName = 0;
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		RegisterClass( &wc );

		::InitializeCriticalSection( &ddeSync );

	    if (DdeInitialize(&CDDEObject::m_dwidInst,
	            (PFNCALLBACK)MakeProcInstance((FARPROC)ChacoVRMLDdeCallback, hInstance),
	            APPCLASS_STANDARD ,	 0))
		{
			CDDEObject::m_dwidInst = 0ul;
	    }

		#if !defined( CH_VRML_VIEWER )
		ChCore::SetDDEInstance( CDDEObject::m_dwidInst );
		#endif


	}

	// Incerement the usage count
	m_numObjects++;


	if (  CDDEObject::m_dwidInst )
	{

		//	new should THROW if unable to allocate.
		m_pDDE = new CDDEObject( CDDEObject::srvMosaic, this);
		ASSERT( m_pDDE );

		EstablishConversation();
	}
}

bool ChHTTPDDE::EstablishConversation()
{
	// Try connecting to a Web Browser
	for ( int i = 0;  i < CDDEObject::maxServers; i++ )
	{
		m_pDDE->SetServer( i );

		if ( RegisterViewers() )
		{
			m_boolConnectionValid = true;
			return true;
		} 
	}
	m_pDDE->SetServer( CDDEObject::srvMosaic );
	return false;
}  


class ZapVisitedConn : public ChVisitor2<ChString, pChVisitedInfo>  
{
	public:
	 	bool Visit( const ChString& key,  const pChVisitedInfo& pVisited )
		{
			delete pVisited;
			return true;
		}
};



ChHTTPDDE::~ChHTTPDDE( )
{

	// Decerement the usage count
	m_numObjects--;

	if (  CDDEObject::m_dwidInst )
	{
		UnregisterViewers();
	}

	if ( m_pDDE )
	{
		delete m_pDDE;
	}
	
	if ( m_hWndDDE )
	{
		::DestroyWindow( m_hWndDDE );
	}

	if ( 0 == m_numObjects )
	{
		ShutDownHTTP();	
	}

}

void ChHTTPDDE::ShutDownHTTP()
{
	if ( m_numObjects > 0 )
	{   // Shutdown when there is a object still active
		m_numObjects = 0;
	}

	if ( 0 == m_numObjects )
	{
		
		#if defined( CH_USE_SOCKET )
		if ( httpSocketConn )
		{
			httpSocketConn->ShutDownHTTP();
			// to cache management
			ChHTTPSocketConn::EnforceCacheLimit();       
			delete httpSocketConn;
		}
		#endif

		::EnterCriticalSection( &ddeSync );

		ChPosition posConn = GetConnectionList().GetHeadPosition();
		while (0 != posConn)
		{
			ChDDEConnInfo *pConn = ChHTTPDDE::GetConnectionList().GetHead();
			ChHTTPDDE::GetConnectionList().RemoveHead();
			delete pConn;

			posConn = ChHTTPDDE::GetConnectionList().GetHeadPosition();
		}


		// zap all visited entries
		//ZapVisitedConn		zapVisited;
		//GetVisitedList().Infix( zapVisited );

		posConn = GetWaitList().GetHeadPosition();

		while (0 != posConn)
		{
			ChDDEConnInfo *pConn = ChHTTPDDE::GetWaitList().GetHead();
			ChHTTPDDE::GetWaitList().RemoveHead();
			delete pConn;

			posConn = ChHTTPDDE::GetWaitList().GetHeadPosition();
		}
	
		::LeaveCriticalSection( &ddeSync );


		::DeleteCriticalSection( &ddeSync );

		if (  CDDEObject::m_dwidInst )
		{
			DdeNameService(CDDEObject::m_dwidInst, NULL, NULL, DNS_UNREGISTER);
			// Uninitialize DDE
	    	DdeUninitialize( CDDEObject::m_dwidInst );
		}
	}
}


BOOL ChHTTPDDE::RegisterViewers( )
{

	CString strServer( m_pDDE->GetServiceName() );
	CString strMimeType;

	// See if we have a browser that can talk to us
	if( !m_pDDE->WWW_Version( ) )
	{
//		return false;
	}

	#if defined( CH_VRML_VIEWER )
	strMimeType = MIME_VRML;

	if( !m_pDDE->WWW_RegisterViewer( strServer, strMimeType,	 0x04 ) )
	{
		return false;
	}

	strMimeType = MIME_GIF;

	if( !m_pDDE->WWW_RegisterViewer( strServer, strMimeType,	 0x04 ) )
 	{
		return false;
	}

	strMimeType = MIME_JPEG;

	if ( !m_pDDE->WWW_RegisterViewer( strServer, strMimeType,	 0x04 ) )
	{
		return false;
	}

	strMimeType = MIME_BMP;

	if ( !m_pDDE->WWW_RegisterViewer( strServer, strMimeType,	 0x04 ) )
	{
		return false;
	}
	#else
	// See if we have a browser that can talk to us
	if( !m_pDDE->WWW_Version( ) )
	{
		return false;
	}

	#endif

	return TRUE;
}

BOOL ChHTTPDDE::UnregisterViewers( )
{

	if ( !m_boolConnectionValid )
	{
		return false;
	}


	#if defined( CH_VRML_VIEWER )
	CString strServer( m_pDDE->GetServiceName() );
	CString strMimeType;

	strMimeType = MIME_VRML;

	m_pDDE->WWW_UnRegisterViewer( strServer, strMimeType );

	strMimeType = MIME_GIF;

	m_pDDE->WWW_UnRegisterViewer( strServer, strMimeType);

	strMimeType = MIME_JPEG;

	m_pDDE->WWW_UnRegisterViewer( strServer, strMimeType );

	strMimeType = MIME_BMP;

	m_pDDE->WWW_UnRegisterViewer( strServer, strMimeType );

	#endif

	return TRUE;
}

void ChHTTPDDE::GetProgressMsg( ChString& strMsg, int& iPercentComplete )
{
	int iConnections = m_connActiveList.GetCount() +
	 		m_connWaitList.GetCount();

	if ( httpSocketConn && iConnections == 0 )
	{	// our connections
		httpSocketConn->GetProgressMsg( strMsg, iPercentComplete );
	}
	else
	{

	}
}


bool ChHTTPDDE::IsActive()		
{
	int iConnections = m_connActiveList.GetCount() +
	 		m_connWaitList.GetCount();

	if ( httpSocketConn  )
	{	// our connections
		iConnections += httpSocketConn->NumActive();	
	}


		
	 return iConnections > 0;
}

int ChHTTPDDE::NumActive()		
{
	int iConnections = m_connActiveList.GetCount() +
	 		m_connWaitList.GetCount();

	if ( httpSocketConn  )
	{	// our connections
		iConnections += httpSocketConn->NumActive();	
	}


		
	 return iConnections;
}

	

void ChHTTPDDE::AbortRequests( bool boolAbortPrefetch /*= false */,
						ChHTTPStreamManager* pStreamMgr /*= 0 */ )
{
	m_boolStopLoading = true;

	::EnterCriticalSection( &ddeSync );

	if ( GetConnectionList().GetCount() )
	{
		if ( m_dwTransactionID )
		{
			m_pDDE->WWW_CancelProgress( m_dwTransactionID );
		}

		ChPosition posConn = GetConnectionList().GetHeadPosition();
		
		while (0 != posConn)
		{
			ChDDEConnInfo *pConn = ChHTTPDDE::GetConnectionList().GetHead();
			pConn->DoErrorNotification( CH_HTTP_ERROR_USER_ABORT);
			ChHTTPDDE::GetConnectionList().RemoveHead();
			delete pConn;
			posConn = ChHTTPDDE::GetConnectionList().GetHeadPosition();
		}
	}


	ChPosition posConn = GetWaitList().GetHeadPosition();

	while (0 != posConn)
	{
		ChDDEConnInfo *pConn = GetWaitList().GetHead();
		GetWaitList().RemoveHead();
		delete pConn;

		posConn = ChHTTPDDE::GetWaitList().GetHeadPosition();
	}

	::LeaveCriticalSection( &ddeSync );

	// check if we have our own http requests
	if ( httpSocketConn  )
	{
		httpSocketConn->AbortRequests();	
	}



}	 

bool ChHTTPDDE::ViewFile( const ChString strURL, const ChString& strFile ) 
{	
	ChString strEmpty;

	// To prevent recursive calls between client and server
	UnregisterViewers( );


	DWORD dwResult = m_pDDE->WWW_OpenURL( strURL, strEmpty, 0xFFFFFFFF,
					0x04, strEmpty, strEmpty,
					m_pDDE->GetServiceName() );

	if ( dwResult == 0 )
	{  // connection failed, try shell execute
		char strBuffer[MAX_PATH];
		::GetCurrentDirectory( MAX_PATH, strBuffer );
		ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), 
					"open", strFile, NULL, strBuffer, 0);
	}

	RegisterViewers( );


	return true;
}

bool ChHTTPDDE::ProcessPendingRequest( UINT uType )
{
	switch( uType )
	{
		case ChDDEConnInfo::msgGetURL :
		{
			if ( ChHTTPDDE::GetWaitList().GetCount() )
			{
				ProcessConnectionRequest();
			}
			break;
		}
		case ChDDEConnInfo::msgRegisterViewer :
		{

			static int iTryCount = 0;
			
			if ( !IsConnectionValid() )
			{
				EstablishConversation();
				// Send connection done if we successfully connected
			}

			if( IsConnectionValid() )
			{
				iTryCount = 0;
				m_pDDE->WWW_RegisterDone();
			}
			else if ( iTryCount++ < 100 )
			{  // We are not up yet, post a message to try again
				RegisterNow();
			}
			break;
		}
		default :
		{
			break;
		}
	}
	return true;
}

bool ChHTTPDDE::ProcessConnectionRequest( ) 
{	   
	bool boolSuccess = true;

	// check to see if we have a vaild connection established with the server
	if ( !IsConnectionValid() )
	{	// Not valid try connections
		if ( !EstablishConversation() )
		{ // have I notified the user of this

			if ( !m_boolNotifyDone )
			{
				#if !defined( CH_USE_SOCKET )
				m_boolNotifyDone = true;
				AfxMessageBox( DDE_ERR_MSG, MB_OK | MB_ICONEXCLAMATION );  
				#endif

			}
			#if !defined( CH_USE_SOCKET )
			return false;
			#endif
		}
		
	}


	#if defined( CH_USE_SOCKET )
retry :

	if ( !IsConnectionValid()  )
	{
		ChDDEConnInfo *pConn = GetWaitList().GetHead();
		GetWaitList().RemoveHead();
		// Don't try any other method if user wants URL by DDE only
		if ( ChHTTPConn::UseDDEOnly & pConn->GetOptions() )
		{
			delete pConn;
			return false;
		}

		if ( httpSocketConn == 0 )
		{
			TRACE( "Unable to establish conversation with a DDE server, using Chaco's HTTP engine\n" );
			httpSocketConn = new ChHTTPSocketConn( GetStreamMgr() );
		}

		// we will do our on notification
		httpSocketConn->GetURL( pConn->GetURL(), 
					pConn->GetUserData(), 0, pConn->GetOptions());

		delete pConn;
		return true;
	}
	#endif

	::EnterCriticalSection( &ddeSync );

	TRACE2( "ProcessConnectionRequest : Num pending %d, Num in wait %d\n", GetConnectionList().GetCount(), 
					GetWaitList().GetCount( ) );
	while( GetConnectionList().GetCount() == 0 && GetWaitList().GetCount( ) )
	{  // start the next request
		ChDDEConnInfo *pConn = GetWaitList().GetHead();
		GetWaitList().RemoveHead();

		GetConnectionList().AddTail( pConn );

		// Send the request
		CString strEmpty, strFileName;

		strEmpty.Empty();
	
		
		//ChUtil::GetTempFileName( strFileName, 0, 0, 0 );

		m_dwTransactionID = m_pDDE->WWW_OpenURL( pConn->GetURL(), strFileName, 0xFFFFFFFF,
						0x04, strEmpty, strEmpty,
						m_pDDE->GetServiceName() );

		if ( m_dwTransactionID == 0 )
		{  // connection failed
			

			UnregisterViewers();
			m_boolConnectionValid = false;

			#if !defined( CH_USE_SOCKET )
			pConn->DoErrorNotification( CH_HTTP_ERROR_CONNECT );
			delete pConn;
			ChHTTPDDE::GetConnectionList().RemoveTail();
			boolSuccess = false;
			#else
			GetWaitList().AddTail( pConn );
			ChHTTPDDE::GetConnectionList().RemoveTail();
			::LeaveCriticalSection( &ddeSync );
			goto retry;
			#endif
		}
	
	}
	::LeaveCriticalSection( &ddeSync );
	return boolSuccess;

}



bool ChHTTPDDE::GetURL( const ChString& strURL, chparam userData,
						const char* pstrDefURL /* = 0 */, 
						chuint32 flOptions /*= 0 */,
						ChHTTPStreamManager* pStreamMgr /*= 0 */)
{
	// resolve the URL
	ChString strRequest;

	ChURLParts	urlParts;

	if ( urlParts.GetURLParts( strURL, pstrDefURL ) )
	{
		if(  urlParts.GetScheme() == ChURLParts::typeFile )
		{
			// Map URL to localfile
			strRequest = urlParts.GetURL();		 
			ChString	strFile, strMimeType;
			ChURLParts::MapURLToHostFile( strRequest, strFile );
			ChHTTPConn::GetMimeTypeByFileExtn( strFile, strMimeType  );


		 	ChDDEConnInfo	*pNewConn = new ChDDEConnInfo( this, strRequest, flOptions, userData );
			ASSERT( pNewConn );

			if ( ChUtil::FileExists( strFile ) )
			{
				pNewConn->DoLoadCompeteNotification( strFile, strMimeType ); 
			}
			else
			{
				pNewConn->DoErrorNotification( CH_HTTP_ERROR_NOT_FOUND );
			}

			delete pNewConn;

			return TRUE;

		}
		
	}
	
	strRequest = urlParts.GetURL();		 
	// check if we have a file scheme for GetURL, if we do
	// then resolve path and do a load complete


	m_boolStopLoading = false;

	#if 0
	if ( pstrDefURL && !strURL.IsEmpty() )
	{
		// Parse anchor
		m_pDDE->WWW_ParseAnchor( CString( pstrDefURL ), strURL, strRequest );
	}
	else
	{
		strRequest = strURL;
	}
	#endif

	if ( strRequest.IsEmpty() )
	{
		TRACE( "Error : URL is empty \n" );
	 	ChDDEConnInfo	*pNewConn = new ChDDEConnInfo( this, strRequest, flOptions, userData );
		ASSERT( pNewConn );
		pNewConn->DoErrorNotification( CH_HTTP_ERROR_BAD_URL );
		delete pNewConn;
		return FALSE;
	}


	TRACE1( "Getting URL %s\n", LPCSTR( strRequest ) );
	// Check if we have already visted the location
	ChVisitedInfo ** ppVisted = GetVisitedList().Find( strRequest );

	if ( ppVisted && *ppVisted )
	{  // return what we have

	 	ChDDEConnInfo	*pNewConn = new ChDDEConnInfo( this, strRequest, flOptions, userData );
		ASSERT( pNewConn );
		
		pNewConn->DoLoadCompeteNotification( (*ppVisted)->GetLocalFileName(), 
								(*ppVisted)->GetMimeType() ); 

		delete pNewConn;

		return TRUE;
	}




	// Find if this is a duplicate request, if it is put
	// it in our wait list, else add it to our conn list
 	ChDDEConnInfo	*pNewConn = new ChDDEConnInfo( this, strRequest, flOptions, userData );
	ASSERT( pNewConn );

	::EnterCriticalSection( &ddeSync );

	GetWaitList().AddTail( pNewConn );

	::LeaveCriticalSection( &ddeSync );
 
	return ProcessConnectionRequest();
}		 

void ChHTTPDDE::QueryURL( const ChString& strFile, ChString& strURL, ChString& strMimeType )
{
	//Unfortunately we cannot find the mime type of the document
	strMimeType.Empty();

	m_pDDE->WWW_QueryURLFile( strFile, strURL );
}




void ChHTTPDDE::ViewDocFile( CString& strFile, CString& strURL, CString& strMimeType )
{

	TRACE1( "ViewDocFile : Num pending %d\n", GetConnectionList().GetCount() - 1 );

	ChDDEConnInfo * pConn = 0;

	if ( GetConnectionList().GetCount() != 0 )
	{
		pConn = GetConnectionList().GetHead();
	}

	if ( pConn )
	{ 

		GetConnectionList().RemoveHead();

		if ( strFile.IsEmpty() || strURL.IsEmpty() || strMimeType.IsEmpty() )
		{
			pConn->DoErrorNotification( CH_HTTP_ERROR_NOT_FOUND );
		}
		else
		{
			pConn->DoLoadCompeteNotification( strFile, strMimeType );

			// Add this to our visted list

			ChVisitedInfo *pVisted = new ChVisitedInfo( strFile, strMimeType );
			ASSERT( pVisted );
			GetVisitedList().Insert( strURL, pVisted );

		}
	}
	else
	{

  		// we have a request from the server to view a file that was not requested
		// by me 	 	
		
		ChDDEConnInfo	*pConn = new ChDDEConnInfo( this, strURL, 
										0, 0 );
		ASSERT( pConn );
	
		pConn->DoLoadCompeteNotification( strFile, strMimeType ); 

		delete pConn;
		// Add this to our visted list

		ChVisitedInfo *pVisted = new ChVisitedInfo( strFile, strMimeType );
		ASSERT( pVisted );
		GetVisitedList().Insert( strURL, pVisted );
	}


	if ( ChHTTPDDE::GetWaitList().GetCount() )
	{
		#if defined( CH_USE_DDE )
		// Process wait request
		if ( GetDDEPostingWindow())
		{
			::PostMessage(  GetDDEPostingWindow(), WM_CHACO_HTTP_MSG, 
						ChDDEConnInfo::msgGetURL, (LPARAM)this );	
		}
		#endif
	}

//	if ( GetNotificationWnd() )
	{
//		::SetFocus( GetNotificationWnd() );
	}

	return;
}

DWORD ChHTTPDDE::Alert( const CString& csMessage, DWORD dwType, DWORD dwButtons )
{

	#if 0
	if ( GetNotificationWnd() )
	{
		#if defined( CH_MSW )
		ChHTTPNotification* pInfo = new ChHTTPNotification( ChHTTPNotification::msgAlert,
								csMessage, dwType, dwButtons ); 
		ASSERT( pInfo );

		::PostMessage( GetNotificationWnd(), WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo );	
		#endif
	}
	#endif

	return 0;

}

void ChHTTPDDE::SetProgressRange( DWORD dwID, DWORD dwRange )
{
	m_dwMaxRange = dwRange;
}

void ChHTTPDDE::BeginProcess( DWORD dwTransactionID,  const CString& csInitialMessage )
{
	m_strStatusMsg = csInitialMessage;

	m_dwTransactionID = dwTransactionID;

	#if 0
	if ( GetNotificationWnd() )
	{
		#if defined( CH_MSW )
		ChHTTPNotification* pInfo = new ChHTTPNotification( 
					ChHTTPNotification::msgBeginProcess, csInitialMessage ); 
		ASSERT( pInfo );

		::PostMessage( GetNotificationWnd(), WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo );	
		#endif
	}
	#endif

}

bool ChHTTPDDE::MakingProgress( DWORD dwTransactionID, const CString& csMessage, DWORD dwProgress )
{

	#if 0
	if ( GetNotificationWnd() )
	{
		#if defined( CH_MSW )
		ChHTTPNotification* pInfo = new ChHTTPNotification( ChHTTPNotification::msgMakingProgress,
								csMessage, dwProgress, m_dwMaxRange ); 
		ASSERT( pInfo );

		::PostMessage( GetNotificationWnd(), WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo );	
		#endif
	}
	#endif

	if ( !csMessage.IsEmpty() )
	{
		m_strStatusMsg = csMessage;
		m_dwProgress   = dwProgress;
	}
	
	if ( m_dwProgress == m_dwMaxRange )
	{
		m_dwProgress = m_dwMaxRange = 0;
		m_strStatusMsg.Empty();
	}


	return ( m_boolStopLoading );
}

void ChHTTPDDE::OpenURLResult( DWORD dwTrans, DWORD dwWindow )
{
	if ( GetDDEPostingWindow())
	{
		::PostMessage(  GetDDEPostingWindow(), WM_CHACO_HTTP_MSG, 
					ChDDEConnInfo::msgGetURL, (LPARAM)this );	
	}
}


void ChHTTPDDE::EndProgress( DWORD dwProgress )
{

	TRACE( "End Progress\n" );

	m_dwTransactionID = 0;

	m_strStatusMsg.Empty();
	m_dwProgress   = 0;
	m_dwMaxRange   = 0;

	#if 0
	// if we still have the connection in the active list because
	// we did not get a ViewDocFile, remove it
	::EnterCriticalSection( &ddeSync );

	ChPosition posConn = GetConnectionList().GetHeadPosition();
	while (0 != posConn)
	{
		ChDDEConnInfo *pConn = GetConnectionList().GetHead();
		GetConnectionList().RemoveHead();
		delete pConn;
		posConn = ChHTTPDDE::GetConnectionList().GetHeadPosition();
	}

	::LeaveCriticalSection( &ddeSync );
	#endif

	#if 0
	if ( GetNotificationWnd() )
	{
		#if defined( CH_MSW )
		ChHTTPNotification* pInfo = new ChHTTPNotification( ChHTTPNotification::msgEndProgress ); 
		ASSERT( pInfo );

		::PostMessage( GetNotificationWnd(), WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo );	

			#if 0
				#if defined( CH_USE_DDE )
				// Process wait request
				pInfo = new ChHTTPNotification(  this ); 
				ASSERT( pInfo );
				::PostMessage( GetNotificationWnd(), WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo );	
				#endif
			#endif
		#endif
	}
	#endif

}

void ChHTTPDDE::RegisterNow()
{
//	::DebugBreak();

	// Create the posting window

	if ( GetDDEPostingWindow())
	{
		::PostMessage(  GetDDEPostingWindow(), WM_CHACO_HTTP_MSG, 
					ChDDEConnInfo::msgRegisterViewer, (LPARAM)this );	
	}
}

HWND ChHTTPDDE::GetDDEPostingWindow( )
{
	 
	if ( !m_hWndDDE )
	{
		m_hWndDDE = ::CreateWindow( CH_DDE_NOTIFY_CLASS, "Chaco DDE Window",
							WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							0, 0, AfxGetInstanceHandle( ), 0 );
	}

 	return m_hWndDDE;
}

LRESULT CALLBACK EXPORT
DDENotifyProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (WM_CHACO_HTTP_MSG == uMsg && lParam )
	{
		ChHTTPDDE* pDDE = (ChHTTPDDE*)lParam;
	
		pDDE->ProcessPendingRequest( wParam );
		return 0;
	}
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

// $Log$
