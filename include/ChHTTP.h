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

#if !defined( _CHHTTP_H )
#define _CHHTTP_H

#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#include <ChSplay.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	Constants:
----------------------------------------------------------------------------*/

#define CH_HTTP_ERROR_NOT_FOUND		404

#define CH_HTTP_CLIENT_ERROR_BASE	600

#define CH_HTTP_ERROR_LOCAL_IN_USE	600
#define CH_HTTP_ERROR_ABORT			601
#define CH_HTTP_ERROR_CONNECT		602
#define CH_HTTP_ERROR_SEND			603
#define CH_HTTP_ERROR_INVALID_HDR	604
#define CH_HTTP_ERROR_NOT_CACHED	605
#define CH_HTTP_ERROR_USER_ABORT	606
#define CH_HTTP_ERROR_INVALID_DATA	607
#define CH_HTTP_ERROR_BAD_URL		608
#define CH_HTTP_ERROR_OPEN_FILE		609
#define CH_HTTP_ERROR_GENERIC		610

#define MIME_TEXT					"text/plain"
#define MIME_HTML					"text/html"
#define MIME_HTML2					"text/htm"
#define MIME_GIF					"image/gif"
#define MIME_BMP					"image/x-bmp"
#define MIME_JPEG					"image/jpeg"
#define MIME_MIDI					"audio/x-midi"
#define MIME_MIDI1					"audio/x-mid"
#define MIME_MIDI2					"audio/midi"
#define MIME_WAVE					"audio/x-wav"
#define MIME_VOX					"audio/voxware"
#define MIME_VRML					"x-world/x-vrml"
#define MIME_WORLD					"application/x-pueblo-world"

//#define MIME_AVI		"video/x-msvideo"
//#define MIME_REALAUDIO	"audio/x-pn-realaudio-plugin"


/*----------------------------------------------------------------------------
	ChHTTPNotification class
----------------------------------------------------------------------------*/

class ChHTTPConn;
class ChHTTPStreamManager;

class CH_EXPORT_CLASS ChHTTPNotification
{
	public:
		enum tagNotification { msgLoadComplete = 1, msgLoadError,
							   msgBeginProcess, msgMakingProgress,
							   msgEndProgress, msgAlert,
							   msgDataNotify } NotifyMsg;

		enum tagRequest { msgGetURL = 1, msgRegisterViewer };

	
		ChHTTPNotification( const ChString& strURL, const ChString& strFile, 
							const ChString& strMimeType, chparam userData ) :
							m_iType(msgLoadComplete), m_iError(0), 
							m_strURL( strURL ), m_strLocalFilename( strFile ),
							m_strData( strMimeType ), m_userData( userData ),
							m_luProgress(0), m_luMaxRange(0), m_pConn(0)
							{
							};

								
		ChHTTPNotification( int iError, const ChString& strErrMsg, const ChString& strURL,  chparam userData) :
							m_iType(msgLoadError), m_iError(iError), 
							m_strURL( strURL ), m_userData( userData ),
							m_luProgress(0), m_luMaxRange(0), m_pConn(0),
							m_strData( strErrMsg )
							{
							};

		ChHTTPNotification( int iType, const ChString& strMsg ) :
							m_iType(iType), m_iError(0), 
							m_strURL( strMsg ), m_userData( 0 ),
							m_luProgress(0), m_luMaxRange(0), m_pConn(0)
							{
							};

		ChHTTPNotification( int iType ) :
							m_iType(iType), m_iError(0), 
							m_userData( 0 ),
							m_luProgress(0), m_luMaxRange(0), m_pConn(0)
							{
							};

		ChHTTPNotification( int iType, const ChString& strMsg, chuint32 luProgress, chuint32 luRange ) :
							m_iType(iType ), m_iError(0), 
							m_strURL( strMsg ),  m_userData( 0 ),
							m_luProgress(luProgress), m_luMaxRange(luRange), m_pConn(0)
							{
							};

		~ChHTTPNotification() {}
		// Accessor methods
		int	GetMsgType()		{ return m_iType; }

		void  GetLoadCompleteInfo( ChString& strURL, ChString& strFile, 
							ChString& strMimeType, chparam& userData )
							{
								strURL = m_strURL;
								strFile = m_strLocalFilename;
								strMimeType = m_strData;
								userData = m_userData;
							}

		void  GetLoadErrorInfo( int& iError, ChString& strMsg, ChString& strURL, chparam& userData )
							{
								strURL = m_strURL;
								strMsg = m_strData;
								iError = m_iError;
								userData = m_userData;
							}
		void  GetProgressInfo( ChString& strMsg, chuint32& luProgress, chuint32& luMax )
							{
								strMsg = m_strURL;
								luProgress = m_luProgress;
								luMax = m_luMaxRange;
							}
		void  GetAlertInfo( ChString& strMsg, chuint32& luType, chuint32& luButton )
							{
								strMsg = m_strURL;
								luType = m_luProgress;
								luButton = m_luMaxRange;
							}
		void  GetBeginInfo( ChString& strMsg )
							{
								strMsg = m_strURL;
							}
		ChHTTPConn*  GetConnectionInfo()
							{
								return m_pConn;
							}
	private :
		int			m_iType;
		int			m_iError;
		ChString 		m_strURL;
		ChString		m_strLocalFilename;
		ChString		m_strData;	
		chparam		m_userData;
		chuint32	m_luProgress;
		chuint32	m_luMaxRange;
		ChHTTPConn*	m_pConn;
};



/*----------------------------------------------------------------------------
	ChHTTPConn class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHTTPConn
{
	public:
		enum tagConnectionType { usingSockets, usingDDE, usingOLE, usingPlugIn };
		enum tagMimeTypes { typeText, typeHTML, typeGIF, typeMidi, typeWave,
							typeJPEG, typeVRML, typeBMP, typeVox, typeWorld };

		enum tagConnOptions { connLocalState = 0x1, // All connections requested through
														// this conn object will not be visible
														// globally
							  connNotifyProgress = 0x2
							};
		enum tagGetOptions { returnCache = 0x1, returnVisited = 0x02, 
							 UsePostMethod = 0x04, DoNotCache = 0x08, ReloadURL = 0x10,
							 PrefetchURL = 0x40,  UseDDEOnly = 0x80,
							 streamDirect = 0x100 };


	public:

		ChHTTPConn( ChHTTPStreamManager* pStreamMgr, chuint32 flOptions = 0 ) :
						m_pStreamMgr( pStreamMgr ),
						m_flOptions( flOptions )
						{
						}


		virtual ~ChHTTPConn() {}


		ChHTTPStreamManager*   GetStreamMgr()						{ return m_pStreamMgr; }
		void	   			   SetStreamMgr( ChHTTPStreamManager* pStreamMgr )	
																	{ m_pStreamMgr = pStreamMgr; }
		chuint32			   GetConnOptions()						{ return m_flOptions; }
		void	   			   SetConnOptions( chuint32 flOptions )	
																	{ m_flOptions = flOptions; }

		virtual int	 GetConnectionType() = 0;
		virtual void    GetUserAgent( ChString& strAgent )	{}
		virtual void    SetUserAgent( const ChString& strAgent )	{}

		virtual bool GetURL( const ChString& strURL, chparam userData,
						const char* pstrDefURL = 0,
						chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 ) = 0;

		virtual bool GetURL( const ChString& strURL, 
						const char* pstrTargetWindow,
						chparam userData,
						const char* pstrDefURL = 0,
						chuint32 flOptions = 0, 
						ChHTTPStreamManager* pStreamMgr = 0 )
		{
			return false;
		}
		
		virtual bool PostURL( const ChString& strURL, const char* pstrBody,
							chint32 lLen = -1, chparam userData = 0,
							const char* pstrDefURL = 0, 
							chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 ) = 0;
		virtual void QueryURL( const ChString& strFile, ChString& strURL, ChString& strMimeType ) = 0;

		virtual bool IsURLCached( const ChString& strURL, const char* pstrDefURL = 0 ) = 0;
		virtual bool IsVisitedURL( const ChString& strURL, const char* pstrDefURL = 0 ) = 0;
		virtual bool GetCachedURL( const ChString& strURL, ChString& strName, 
								ChString& strMimeType, const char* pstrDefURL  = 0 , 
								chuint32 flOption  = 0 ) = 0;
		virtual bool DeleteCachedURL( const ChString& pstrURL, const char* pstrDefURL = 0 ) = 0;
		virtual void AbortRequests( bool boolAbortPrefetch = false, ChHTTPStreamManager* pStreamMgr = 0 ) = 0;

		virtual bool IsActive() = 0;
		virtual int  NumActive() = 0;
		virtual void GetProgressMsg( ChString& strMsg, int& iPercentComplete ) = 0; 

		virtual void ShutDownHTTP() = 0;


		virtual bool ViewFile( const ChString strURL, const ChString& strFile )
						{
							return false;
						}
		static int GetMimeType( const ChString& strType );
		static void GetMimeTypeByFileExtn( const char* pstrFile, ChString& strMimeType  );
		static void GetFileExtnByMimeType( int iMimeType, ChString& strExtn  );
		static void	AddMimeType( const ChString& strMime, const ChString& strFileExtn, 
							const ChString& strDesc );
 		static int GetFileOpenFilter( ChString& strFilter );

		static void TermMimeManager();

	private:

		ChHTTPStreamManager* 	m_pStreamMgr;
		chuint32			 	m_flOptions;
};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif	// !defined( _CHHTTP_H )
