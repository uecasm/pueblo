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

	This file contains the definition of the ChHTTPInfo class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHHTPRIV_H ))
#define _CHHTPRIV_H

#include <ChSplay.h>
#include <ChList.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_MSW )  

	#if defined( WIN32 )
		#include <afxtempl.h>
	#endif
	
	#include <fstream>

#endif	// defined( CH_MSW )

#include <ChUrlMap.h>
#include <ChHttpStream.h>


/*----------------------------------------------------------------------------
	Constants:
----------------------------------------------------------------------------*/

#define HTTP_VERSION			"HTTP/1.0"

#define BROWSER_NAME			"WebTracker"
#define BROWSER_VERSION			"0.92"

#if !defined( CR )
#define CR						('\015')	// Must be converted to ^M for transmission
#endif
#if !defined( LF )
#define LF						('\012')	// Must be converted to ^J for transmission
#endif

#define MAX_OPTION_BLK			30

#define DEF_FILE_NAME			"index.htm"
#define URL_DBNAME				"pburldb"

#define HTTP					0
#define CONTENT_TYPE			1
#define LAST_MODIFIED			2
#define CONTENT_LENGTH			3 
#define	DATE					4
#define CONTENT_ENCODING		5
#define LOCATION				6
#define SET_COOKIE				7

											// state machine 
#define HTTP_BEGIN_PROCESS		1
#define HTTP_IN_BODY			2
#define HTTP_IN_HEADER			3
#define HTTP_USE_LOCAL			4
#define HTTP_ERROR				5
#define HTTP_DATA_COMPLETE		6
#define HTTP_END_PROCESS		7
#define HTTP_RETRY				8
#define HTTP_NOTIFY				9
										// A request is put to HTTP_WAIT if the requested URL is 
										// currently being downloaded by a pervious request, we wait till
										// the old request is done and then notify the module. 
#define HTTP_WAIT				9        




											/* The binary flag is different
												between UNIX and MSW */
#if defined( CH_MSW )

	#define IOS_BINARY		std::ios::binary

#elif defined( CH_UNIX )

	#define IOS_BINARY		std::ios::bin

#endif	// defined( CH_UNIX )



#if defined( CH_UNIX )

class ChThreadStringBlock
{
	public:
		ChThreadStringBlock() {}
		~ChThreadStringBlock() {}

		bool Check( const ChString& strName ) { return false; }
		void Release() {}
};

#endif	// defined( CH_UNIX )



/*----------------------------------------------------------------------------
	ChHTTPInfo class
----------------------------------------------------------------------------*/

class ChHTTPInfo;    
class ChDataBase;
class ChHttpThreadMgr;
class ChHttpThread;

#if !defined( NO_TEMPLATES )
											/* Templete definition for request
												queue and connection list */
typedef ChPtrList<ChHTTPInfo>	ChHTTPInfoList; 
typedef ChPtrList<sockinetbuf>	ChSocketDelList; 

typedef ChSplay<ChString, int>	ChHTTPVisitedList;            
typedef ChSplay<ChString, sockaddr_in>	ChSocketAddrList;            
#else 
#include <TemplCls\ChHtpSpl.h>
#include <TemplCls/ChHTPLst.h>
typedef ChPtrHTTPList			ChHTTPInfoList;            
typedef ChSplayHTTPVisited		ChHTTPVisitedList;            
#endif


class ChHTTPConnStream  : public ChHTTPStream
{
	public :
    	enum tagStreamType { streamNormal = 0x1, streamAsFile = 0x02 };

		ChHTTPConnStream();
		virtual ~ChHTTPConnStream()
					{
					}

	    // methods provided my the base class
	    virtual const ChString& GetURL();
	    virtual const ChString& GetMimeType();
	    virtual const ChString& GetCacheFilename();
		virtual const char*   GetErrorMsg();
	    virtual const ChString& GetContentEncoding();
	    virtual const ChString& GetLastModified();
	    virtual const ChString& GetTargetWindowName();
	    virtual long GetContentLength();

	    virtual void* GetStreamPrivateData();
	     // Attributes
		virtual void SetStreamPrivateData( void *pData );
	public  :
		void*			m_pUserStreamData;
		ChHTTPInfo*		m_pHTTPInfo;  		
 		UINT			m_uStreamOption;
 
};



class ChHTTPInfo
{
	public:
		ChHTTPInfo( ChHTTPSocketConn* pHTTPConn,  const ChString& strURL,
					chparam userData, const char* pstrHostName,
					chuint32 flOptions, ChHTTPStreamManager* pStreamMgr );
		~ChHTTPInfo();

		enum ConnectionResult {
			connFailed, connSucceeded, connRetry
		};

		inline bool IsCached()
							{
								return m_boolCache;
							}
		inline const ChString& GetLocalName()
							{
								return m_strLocalName;
							}
		inline bool	IsPrefetched()
						{
							return m_bbPrefetched;
						}

		inline	bool	IsAborted()
							{
								return m_bbAbort;
							}
	
		inline 	void	AbortRequest()		
						{
							m_bbAbort = true;
							Disconnect();
						}

		inline bool		CompletedRequest()
						{
							return m_bbCompleted;
						}
		inline  ChHTTPSocketConn* GetHTTPConn() const
						{
							return m_pHTTPConn;
						}
		
		ConnectionResult			Connect();
		ConnectionResult 			Connect( sockinetaddr& sa );

		void			Disconnect()		
							{ 
								if ( m_pSocket )
								{
									m_pSocket->close(); 
			
									if ( !IsThreaded() )
									{
										m_pSocket->SetUserData( 0 );
									}
								}
							}

 		bool 			SendRequest();
		void	 		ProcessInput();
		bool			DeleteCache( const ChString& strURL );
		chuint32		GetURLOptions()		{  return m_flOptions; }
		ChHttpUrlParts&	GetParts()			{ return m_urlParts; }
		const ChString&	GetURL()			{ return m_urlParts.GetURL(); }
		int				GetState()			{ return (int)m_CurrState; }
		std::fstream*		GetFile()			{ return m_pFile; }
		const ChString&	GetMimeType()		{ return m_strMimeType; }
		char*			GetBuffer()			{ return m_pstrBuffer; }
		chuint32		GetBufferSize()		{ return m_luBufferSize; }
		chparam			GetUserData() 		{ return m_userData; }
		const ChString&	GetFileName()		{ return m_strLocalName; }
		chint32			GetDataLength()		{ return m_luBodyLen; }
		const ChString&   GetTargetWindowName(){ return m_strTargetWndName; }
		const ChString&   GetLastModified()	{ return m_strLastModified; }

		void			SetBuffer( char* pstrBuf )	{ m_pstrBuffer = pstrBuf; }


		#if	defined(CH_MSW ) && defined( WIN32 )
		ChHttpThread*	GetThread()		{ return m_pThread; }
		void			SetThread( ChHttpThread * pThread ) { m_pThread = pThread; }
		static 	ChHttpThreadMgr* GetThreadMgr()	{ return m_pThreadMgr; }
		#endif // CH_MSW

		void			SetState( int iState ) 
							{ 
								m_CurrState = iState;
								if ( iState == HTTP_DATA_COMPLETE ||
									 iState == HTTP_ERROR ||
									 iState == HTTP_END_PROCESS ||
									 iState == HTTP_USE_LOCAL  ||
									 iState == HTTP_NOTIFY ||
									 iState == HTTP_RETRY )
								{
									m_bbCompleted = true;
								}
							}

		void SetFile( std::fstream* pFile )
							{
								m_pFile = pFile;
							}
		void			SetThreaded() { m_boolThreaded = true; }

		bool			IsZipped();
		const ChString&	GetZipFileName()	{ return m_strZipName; }

		bool			Visited();

		void			SetPostData( const char *pstrData, chint32 lLen )		
								{
									if ( lLen == -1 )
									{
										m_strPostData = pstrData;
									}
									else
									{
										ChString strTmp( pstrData, (int)lLen );
										m_strPostData = strTmp;
									}
								}

		void 	OnDataComplete();
	
		int		GetError()			{ return m_iError; }
		void	SetError( int iErr ){ m_iError = iErr; }

		void 	SyncInfo( ChHTTPInfo* pInfo );

		void 	InitStream( );
		void 	CloseStream( );
		void 	StreamURL( );
		void    NotifyProgress();

		ChHTTPStreamManager*   GetStreamMgr()						
								{ return  m_pAltStreamMgr ? m_pAltStreamMgr : m_pHTTPConn->GetStreamMgr(); }

		void 	AddToConnList( );
		void 	RemoveFromConnList( ChPosition posConn );
		void 	RemoveConnection( );
		void 	UpdateStatusInfo( bool boolActive = true );

		static void 	InitHTTPInfo();
		static void 	TermHTTPInfo();
		static void 	OpenCacheDataBase();
		static bool 	ClearCache( );
		static bool 	EnforceCacheLimit( );
		static bool 	CreateAndValidateCacheDir( ChString& strDir );

		static bool 	IsThreaded() 			{ return m_boolThreaded;}

		static int				NumActive()		{ return m_iActiveConn; }


		static void		LockHTTP()
							{
								#if	defined(CH_MSW ) && defined( WIN32 )
								if ( m_boolThreaded )
								{
									::EnterCriticalSection( &m_httpSync );
								}
								#endif
							}
		static void		UnlockHTTP()
							{
							 	#if	defined(CH_MSW ) && defined( WIN32 )
								if ( m_boolThreaded )
								{
									::LeaveCriticalSection( &m_httpSync );
								}
								#endif
							}


		static ChHTTPInfoList&	GetRequestQueue() 	{ return m_httpRequestQueue; }
		static ChHTTPInfoList&	GetConnectionList() { return m_httpConnList; }
		static ChSocketDelList*	GetSocketDelList()	{ return m_psocketDelList; }
		static void 			AddToSocketAddrList( const ChString& strHost, const sockaddr_in& addrIn );
		static bool 			FindSocketAddr( const ChString& strHost, sockaddr_in& addrIn );
		static void				CloseAllConnections( bool boolOnlyGlobal, bool boolShutDown = false  );
		static void 			CloseConnections( ChHTTPSocketConn* phttpConn, bool boolAbortPrefetch,
									ChHTTPStreamManager* pStreamMgr = 0 );

	private :
		void 	BuildRequestLine( );
		void	BuildGeneralHeader( ) { }
		void	BuildRequestHeader( );
		void	BuildObjectHeader();
		void	TerminateCommand() { m_strRequestBuffer += TEXT( "\r\n" ); }
		bool 	IsStatusLine( char* pstrBuffer, chuint iLen );
		int 	GetHdrType( char* pstrHdr  );
		void 	ProcessBody();
		void 	ProcessStatusLine();
		void 	ProcessHeaders();
		void 	ProcessNoHeaderReply();
		void 	ProcessBuffer();
		void	MapURLToLocalName( int iMimeType );
		void 	HandleFileScheme();
		void 	HandleScheme();
		void 	HandleError( int iError );


		#if defined( _DEBUG )
		bool DisplayAllDBKeys( );
		#endif						 

	private:

		enum tagConstants{ minBufferSize = 2048, midBufferSize = 4096, maxBufferSize = 10240 };

		enum tagProgressMsg{ msgBeginProgress = 1, msgEndProgress, msgProgress };

		enum tagStreamState{ streamUnInitialized = 1, streamInitialized, streamInData, streamDestroyed };

		ChHTTPSocketConn*		m_pHTTPConn;
		chparam					m_userData;
		sockinetbuf*			m_pSocket;
		bool					m_boolCreateByMe;
		ChHttpUrlParts			m_urlParts;
		chuint32				m_luTotalLen;
		char*					m_pstrBuffer;
		chuint32				m_luBufferSize;
		chuint32				m_luToProcess;
		chuint32				m_CurrState;
		chuint32				m_luBodyLen;
		chuint32				m_flOptions;
		ChString					m_strLastModified;
		ChString					m_strLocalName;
		ChString					m_strMimeType;
		ChString					m_strRequestBuffer;
		std::fstream*				m_pFile;
		bool					m_boolCache;
		bool					m_bbPrefetched;
		bool					m_bbAbort;
		bool					m_bbCompleted;
		bool					m_boolUpdateInfo;
		int						m_nZipType;
		ChString					m_strZipName;
		ChString					m_strPostData;
		ChString					m_strTargetWndName;
		int						m_iError;
		ChHTTPStreamManager* 	m_pAltStreamMgr;
		ChHTTPConnStream		m_httpStream;
		int						m_iStreamState;
		#if	defined(CH_MSW ) && defined( WIN32 )
		static CRITICAL_SECTION	ChHTTPInfo::m_httpSync;
		static ChHttpThreadMgr*	m_pThreadMgr;
		ChHttpThread*			m_pThread;
		#endif


	private:
		static ChDataBase*		m_pURLDB;
		static bool				m_boolThreaded;
		static int				m_iActiveConn;
		static ChString			m_strCacheDir;

											// Request queue

		static ChHTTPInfoList	m_httpRequestQueue;

											// Currently active connection list

		static ChHTTPInfoList		m_httpConnList;
		static ChHTTPVisitedList 	m_httpVisited;	 		// Current session visited

		static ChSocketDelList*		m_psocketDelList;

		static ChSocketAddrList		m_socketAddrList;

};


/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/

CH_DECLARE_SOCKET_HANDLER( httpSocketHandler )


CH_EXTERN_FUNC( void )
ProcessNonThreadedRequest();

CH_GLOBAL_FUNC( void )
NotifyErrorMsg( ChHTTPInfo * pHTTPInfo, int iError );

CH_EXTERN_FUNC( void )
DeleteConnection( void );


CH_EXTERN_FUNC( void )
CleanupHTTPConn();

CH_GLOBAL_FUNC( void )
UpdateWaitRequest( ChHTTPInfo *pHTTPInfo, const ChString strNewURL );



#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif 	// _CHHTPRIV_H

// $Log$
