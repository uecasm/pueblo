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

	This file contains the implementation of the ChHTTPConn class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHHTPCON_H )
#define _CHHTPCON_H

#include <ChHTTP.h>
#include <ChUrlMap.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif



typedef struct tagConnectionState
{
	int			iTotalActive;
	int			iTotalWaiting;
	chuint32	luTotalBytes;
	chuint32	luTotalReceived;

} ConnectionState;

class ChHttpCookie;


/*----------------------------------------------------------------------------
	ChHTTPSocketConn class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHTTPSocketConn : public ChHTTPConn
{

	public:
		friend class ChHTTPInfo;

		ChHTTPSocketConn( ChHTTPStreamManager* pStreamMgr, chuint32 flOptions = 0 );


		virtual ~ChHTTPSocketConn();

		virtual int	GetConnectionType()	
						{ 
							return  usingSockets; 
						}

		virtual void    GetUserAgent( ChString& strAgent )		{ strAgent =  m_strUserAgent; }
		virtual void    SetUserAgent( const ChString& strAgent )	{ m_strUserAgent = strAgent; }
		
		virtual bool GetURL( const ChString& strURL, chparam userData,
						const char* pstrDefURL = 0,
						chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 );
		virtual bool PostURL( const ChString& strURL, const char* pstrBody,
							chint32 lLen = -1, chparam userData = 0,
							const char* pstrDefURL = 0, 
							chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 );
		virtual void QueryURL( const ChString& strFile, ChString& strURL, ChString& strMimeType )
				{}
		virtual bool IsURLCached( const ChString& strURL, const char* pstrDefURL = 0 );
		virtual bool IsVisitedURL( const ChString& strURL, const char* pstrDefURL = 0 );
		virtual bool GetCachedURL( const ChString& strURL, ChString& strName, 
								ChString& strMimeType, const char* pstrDefURL  = 0 , 
								chuint32 flOption  = 0 );
		virtual bool DeleteCachedURL( const ChString& pstrURL, const char* pstrDefURL = 0 );
		virtual void AbortRequests( bool boolAbortPrefetch = false, ChHTTPStreamManager* pStreamMgr = 0 );

		virtual bool IsActive();
		virtual int  NumActive();
		virtual void GetProgressMsg( ChString& strMsg, int& iPercentComplete ); 
		virtual void ShutDownHTTP();

	private :
		void InitHTTP();

	public:
		static void     UpdateHTTPPreferences( const ChString& strCompany );
		static bool 	ClearCache( );
		static bool 	EnforceCacheLimit( );
		static bool 	CreateAndValidateCacheDir( ChString& strDir );
		static bool 	CloseAllConnections( bool boolOnlyGlobal = true );

		static int  GetMaxConnections()					{ return  m_iMaxConnection; }
		static void SetMaxConnections( int iMaxConn )  	{ m_iMaxConnection = iMaxConn; }
		// Proxy access methods
		static const ChString&  GetHTTPProxy()	{ return m_strHTTPProxy; }
		static const ChString&  GetFTPProxy()		{ return m_strFTPProxy; }

   		static chuint32  GetHTTPProxyPort()		{ return m_luHTTPProxyPort; }
   		static chuint32  GetFTPProxyPort()		{ return m_luFTPProxyPort; }
		static ChHttpCookie* GetHTTPCookie()	{ return m_phttpCookie; }


	protected :
		ConnectionState			m_connState;
		static ConnectionState	m_connGlobalState;
		static int				m_iMaxConnection;	// Maximum simultaneous connections allowed
		static long				m_luhttpOptions;
		static ChString 			m_strHTTPProxy;
		static chuint32 		m_luHTTPProxyPort;
		static ChString 			m_strFTPProxy;
		static chuint32 		m_luFTPProxyPort;
		static ChHttpCookie*	m_phttpCookie;


	private:
		static int			m_iTotObjects;
		ChString				m_strUserAgent;
};


/*----------------------------------------------------------------------------
	ChHttpUrlParts class
----------------------------------------------------------------------------*/

class ChHttpUrlParts : public ChURLParts
{
	public :
		ChHttpUrlParts() {}
		virtual ~ChHttpUrlParts() {}

		bool  UsingProxy();
		virtual const char*	GetHostName();
		virtual const char* GetAbsPath();
		virtual const char* GetRelPath();
		virtual int			GetPortNumber();

};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif	// !defined( _CHHTPCON_H )
