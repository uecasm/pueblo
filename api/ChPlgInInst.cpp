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

	This file contains the implementation of the PlugIn instance class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChArgList.h>

#include "ChHtmlView.h"
#include "ChPlgInMgr.h"

#include <MemDebug.h>


ChPluginRunnning	ChPlugInInstance::m_plgRunning;

// Pueblo entry points for Netscape style Plug-Ins
NPNetscapeFuncs	ChPlugInInstance::m_puebloEntryFuncs = 
				{
					sizeof( NPNetscapeFuncs ),
					MAKEWORD( NP_VERSION_MINOR, NP_VERSION_MAJOR ),
					CHPN_GetURL,		
				    CHPN_PostURL,
				    CHPN_RequestRead,
				    CHPN_NewStream,
				    CHPN_Write,
				    CHPN_DestroyStream,
				    CHPN_Status,
				    CHPN_UserAgent,
					CHPN_MemAlloc,
					CHPN_MemFree,
					CHPN_MemFlush,
					CHPN_ReloadPlugins,
					CHPN_getJavaEnv,
					CHPN_getJavaPeer,
					CHPN_GetURLNotify,		
				    CHPN_PostURLNotify,
				};

ChPlugInInstance::ChPlugInInstance( const ChString& strURL, const ChString& strMimeType, uint16 mode )
			: m_strURL( strURL), 
			m_strMimeType( strMimeType ), 
			m_pplgInEntryFuncs(0 )//,
			//m_boolStreamFile( false ),
			//m_pStream( 0 )
			#if defined ( USE_NATIVE_HWND )
			, m_hWnd( 0 )
			#endif
{
	
	m_mode = mode == ChObjInline::embedFull ? NP_FULL: NP_EMBED;

	// initialize the instance handle
	m_instHdl.pdata = 0;
	m_instHdl.ndata = (void*)this;

	ChMemClearStruct( &m_window );

}

ChPlugInInstance::~ChPlugInInstance()
{

	//delete m_pStream;

	ChPluginRunTimeInfo** ppRunInfo = m_plgRunning.Find( m_strModule );

	if ( ppRunInfo && *ppRunInfo )
	{ 

		ChPluginRunTimeInfo* pRunInfo = *ppRunInfo;

		// shut down the instance
		NPSavedData*	pSavedData = 0;


		#if !defined ( USE_NATIVE_HWND )
		if ( m_hWnd.GetSafeHwnd() )
		#else
		if ( m_hWnd )
		#endif
		{
			try 
			{
		 		if ( NPERR_NO_ERROR == GetPlugInFuncs()->destroy( 
											&m_instHdl, &pSavedData ) )
				{  // do something with saved data
				
				}
			}
			catch( ... )
			{ // plugin caused an error, we may be unstable now

			}
			#if !defined ( USE_NATIVE_HWND )
			{
				m_hWnd.DestroyWindow( );
			}
			#else
			if ( m_hWnd )
			{
				::DestroyWindow( m_hWnd );
			}
			#endif
		}

		// Decrement the use count
		pRunInfo->Release();

		if ( pRunInfo->GetUseCount() == 0 )
		{
			// shut down the plugin
			pRunInfo->ShutDown();

			m_plgRunning.Delete( m_strModule );

			delete pRunInfo;
		}

	}


}

bool ChPlugInInstance::Initialize( const char* pstrModule )
{

	m_strModule =  pstrModule;
	ChPluginRunTimeInfo** ppRunInfo = m_plgRunning.Find( m_strModule );

	ChPluginRunTimeInfo* pRunInfo = 0;

	if ( ppRunInfo && *ppRunInfo )
	{
		pRunInfo = *ppRunInfo;
	} 


	if ( pRunInfo == 0  )
	{

		bool boolSuccess = false;
	
		HINSTANCE 		hLibrary = 0;
		NP_ShutdownUPP  pprocShutdownPlugIn = 0;
		
		hLibrary = ::LoadLibrary( pstrModule );

		if ( hLibrary )
		{
			// sucessfull, now get the entry points 
			NP_GetEntryPointsUPP pprocEntry = ( NP_GetEntryPointsUPP )
							::GetProcAddress( hLibrary, TEXT( "NP_GetEntryPoints" ) ); 
			NP_InitializeUPP pprocInit = ( NP_InitializeUPP )
							::GetProcAddress( hLibrary, TEXT( "NP_Initialize" ) ); 
			if ( 0 == pprocInit )
			{
			 	pprocInit = ( NP_InitializeUPP )
							::GetProcAddress( hLibrary, TEXT( "NP_PluginInit" ) ); 
			}
			pprocShutdownPlugIn = ( NP_ShutdownUPP )
							::GetProcAddress( hLibrary, TEXT( "NP_Shutdown" ) ); 
								
			if ( 0 == pprocShutdownPlugIn )
			{
				pprocShutdownPlugIn = ( NP_ShutdownUPP )
							::GetProcAddress( hLibrary, TEXT( "NP_PluginShutdown" ) ); 
			}

			// we need all the entry points to proceed
			if ( pprocEntry && pprocInit && pprocShutdownPlugIn )
			{	
				pRunInfo = new ChPluginRunTimeInfo( hLibrary, pprocShutdownPlugIn );  
				ASSERT( pRunInfo );
				// Get the entry points from the plugin
				//::MessageBox( NULL, "Get plugin entry points ", "Plugin test", MB_APPLMODAL );

				try
				{
					boolSuccess = ( NPERR_NO_ERROR == 
						pprocEntry( pRunInfo->GetPlugInFuncs() ) );
				}
				catch( ... )
				{ // plugin caused an error, we may be unstable now
					boolSuccess = false;
				}
				

				if ( boolSuccess )
				{ // check if we support this version of the API
					m_plgRunning.Insert( m_strModule, pRunInfo ); 
				}
				
				//::MessageBox( NULL, "Initialize plugin ", "Plugin test", MB_APPLMODAL );
				try
				{
					// initialize the plugin
					if ( !( boolSuccess && NPERR_NO_ERROR == pprocInit( &m_puebloEntryFuncs ) ) )
					{ 
				 		boolSuccess = false;	 
					}
				}
				catch( ... )
				{
					boolSuccess = false;
					m_plgRunning.Delete( m_strModule );
				}

				if ( !boolSuccess )
				{
					delete pRunInfo;
					pRunInfo = 0;	
				}
			}
		}
	}

	// use the dll and get a copy of the entry funcs
	if ( pRunInfo )
	{
		pRunInfo->Use();
		m_pplgInEntryFuncs = pRunInfo->GetPlugInFuncs();
	}
	
	return 	pRunInfo != 0;
}

bool ChPlugInInstance::New( ChHtmlView* pHtmlInst, const ChString& strLocalFile, ChSize& sizeWindow,
								ChArgumentList* pArgs )
{
	// Save the HTML instance for use in streams
	m_pHtmlView = pHtmlInst;
	// make argument list 

	// call plugin new
	if ( m_pplgInEntryFuncs == 0 || 
		 0 == m_pplgInEntryFuncs->newp || 
		 0 == m_pplgInEntryFuncs->newstream ||
		 0 == m_pplgInEntryFuncs->destroystream || 
		 0== m_pplgInEntryFuncs->setwindow )
	{
		return false;
	}

	char *pstrName = 0;
	char *pstrValue = 0;

	char** ppstrName;
	char** ppstrVal;
	int iCount = 0;

	ppstrName = &pstrName;
	ppstrVal  = &pstrValue;


	if ( pArgs )
	{
		iCount = pArgs->GetArgCount();

		if ( iCount )
		{
			ppstrName = new char* [ iCount ];
			ppstrVal = new char* [ iCount ];

			for ( int  i = 0; i < iCount; i++ )
			{
				ChString strName, strValue;

				pArgs->GetArg( i, strName, strValue );
				
				ppstrName[i] =	new char[strName.GetLength() + 1 ];
				lstrcpy( ppstrName[i], strName );

				ppstrVal[i] = new char[strValue.GetLength() + 1 ];
				lstrcpy( ppstrVal[i], strValue );
			}
		}

	}

	//::MessageBox( NULL, "Call newp ", "Plugin test", MB_APPLMODAL );

	try
	{
		if ( NPERR_NO_ERROR != m_pplgInEntryFuncs->newp( (char*)LPCSTR(m_strMimeType), &m_instHdl,
								m_mode,
								iCount, ppstrName, ppstrVal, 0 )  )
		{
			return false;
		}
	}
	catch( ... )
	{
		return false;
	}

	if ( iCount )
	{
		for ( int  i = 0; i < iCount; i++ )
		{
			delete [] ppstrName[i];
			delete [] ppstrVal[i];
		}
		delete []ppstrName;
		delete []ppstrVal;
	}
	//::MessageBox( NULL, "Create Window ", "Plugin test", MB_APPLMODAL );

	// create the window and call set window
	#if !defined ( USE_NATIVE_HWND )
	CRect rtFrame( 0, 0, sizeWindow.cx, sizeWindow.cy );
	m_hWnd.Create( NULL, TEXT( "" ), 
							 WS_CHILD ,
							 rtFrame,
							 pHtmlInst, 0 );
	ASSERT( m_hWnd.GetSafeHwnd() );
	#else

	m_hWnd = ::CreateWindow( ChPlugInMgr::GetClassName(),
							 TEXT( "" ),
							 WS_CHILD,
							 0, 0,
							 sizeWindow.cx, sizeWindow.cy,
							 pHtmlInst->GetSafeHwnd(), 
							 NULL,	// menu
							 (HINSTANCE)::GetWindowLong( pHtmlInst->GetSafeHwnd(), 
							 GWL_HINSTANCE ),
							 0 );

	ASSERT( m_hWnd );
	#endif

	#if !defined ( USE_NATIVE_HWND )
	m_window.window = m_hWnd.GetSafeHwnd();
	#else
	m_window.window = m_hWnd;
	#endif
	m_window.width = sizeWindow.cx;
	m_window.height = sizeWindow.cy;

	//::MessageBox( NULL, "Call new stream ", "Plugin test", MB_APPLMODAL );

	//  create a new stream and notify the plugin
	ChPlugInStream* pStream = new ChPlugInStream( &m_instHdl, m_strURL, m_strMimeType );
	ASSERT( pStream );

	pStream->SetLocalFile( strLocalFile );


	::PostMessage( (HWND)m_window.window, WM_PLUGIN_INIT_STREAM, 0, (LPARAM)pStream );

	//m_pStream->New();
	
   	//m_strLocalFile = strLocalFile;
	//m_boolStreamFile = true;
	return true;
}

bool ChPlugInInstance::SetWindow( const ChRect& rtFrame, bool boolAlways /* = false */ )
{
	bool boolReturn = true;

	// call setwindow only if there is a change in size
	if ( boolAlways || 
		 rtFrame.left != (int)m_window.x || 
		 rtFrame.top != (int)m_window.y || 
		 rtFrame.Width() != (int)m_window.width || 
		 rtFrame.Height() != (int)m_window.height )
	{
		if ( !boolAlways )
		{
			m_window.x  = rtFrame.left;
			m_window.y  = rtFrame.top;
			m_window.width  = rtFrame.Width();
			m_window.height  = rtFrame.Height();
		}
		
		try
		{
			boolReturn = ( NPERR_NO_ERROR == GetPlugInFuncs()->
						setwindow( &m_instHdl, &m_window ) );
		}
		catch( ... )
		{ // plugin caused an error, we may be unstable now
			boolReturn = false;
		}
	}

#if 0
	if ( boolReturn && m_boolStreamFile )
	{
		m_boolStreamFile = false;

		boolReturn = m_pStream->StreamFile( m_strLocalFile );

		// Destroy the stream 
		m_pStream->DestroyStream();
	
		delete m_pStream;
		m_pStream = 0;
	}
#endif

	return boolReturn;
}


/// Implementation of streams class
ChPlugInStream::ChPlugInStream( NPP pInst, const ChString& strURL, const ChString& strMimeType ) 
									: m_pPlgInInst( pInst ), m_uMode( 1 ), 
									m_strURL( strURL ), m_strMimeType( strMimeType ),
									m_iState( newStream ),
									m_iError( NPERR_NO_ERROR ),
									m_pURLNotifyData( 0 )
{
	ChMemClearStruct( &m_streamHdl );
	m_streamHdl.pdata = 0;
	m_streamHdl.ndata = pInst->ndata;
	m_streamHdl.url   = m_strURL;
}
ChPlugInStream::ChPlugInStream( NPP pInst, const ChString& strURL, const ChString& strMimeType, void* pData ) 
									: m_pPlgInInst( pInst ), m_uMode( 1 ), 
									m_strURL( strURL ), m_strMimeType( strMimeType ),
									m_iState( newStream ),
									m_iError( NPERR_NO_ERROR ),
									m_pURLNotifyData( pData )
{
	ChMemClearStruct( &m_streamHdl );
	m_streamHdl.pdata = 0;
	m_streamHdl.ndata = pInst->ndata;
	m_streamHdl.url   = m_strURL;
}

									 
ChPlugInStream::~ChPlugInStream()
{
}

 // Methods
bool ChPlugInStream::New()
{


	m_streamHdl.lastmodified = time( 0 );

	try
	{
		m_iError = ((ChPlugInInstance*)m_pPlgInInst->ndata)->GetPlugInFuncs()->newstream(
										m_pPlgInInst, (char*)LPCSTR(m_strMimeType), 
										&m_streamHdl, true, &m_uMode );
	}
	catch( ... )
	{ // plugin caused an error, we may be unstable now
		m_iError = NPERR_GENERIC_ERROR;
	}

	// For some reason netscape ignores the error returned by netstream and some
	// plugins like Voxware depend on this, let us ignore all error <= generic error
	if ( m_iError <= NPERR_GENERIC_ERROR )
	{
		m_iError = NPERR_NO_ERROR;
	}
	return m_iError == NPERR_NO_ERROR;
}

bool ChPlugInStream::StreamFile( const char* pstrFile )
{

	ChPlugInInstance* pInst = ((ChPlugInInstance*)m_pPlgInInst->ndata);

	// Open the file
	HANDLE hFile = ::CreateFile(
					    pstrFile,		// address of name of the file 
					    GENERIC_READ,	// access (read-write) mode 
					    FILE_SHARE_READ,// share mode 
					    NULL,			// address of security descriptor 
					    OPEN_EXISTING,	// how to create 
					    0,				// file attributes 
					    NULL			// handle of file with attributes to copy  
				   );
	if ( hFile != INVALID_HANDLE_VALUE )
	{

		DWORD  dwHigh, dwSize;
		int32  lWrite;

		dwSize = ::GetFileSize( hFile, &dwHigh );

		m_streamHdl.end = dwSize;

		HANDLE hFileMap;
		char* pMappedView;

		if ( dwSize || dwHigh )
		{
			// Map if we have a good file
			hFileMap =  ::CreateFileMapping(
						    			hFile,	// handle of file to map 
						      			NULL,	// optional security attributes 
						    			PAGE_READONLY | SEC_COMMIT,	// protection for mapping object 
						    			0,	// high-order 32 bits of object size  
						    			0,	// low-order 32 bits of object size  
						    			NULL 	// name of file-mapping object 
				   );	

			ASSERT( hFileMap );

			pMappedView = (char*) MapViewOfFile(
								    hFileMap,		// file-mapping object to map into address space  
								    FILE_MAP_READ,	// access mode 
								    0,	// high-order 32 bits of file offset 
								    0,	// low-order 32 bits of file offset 
								    0 	// number of bytes to map, 0 = Map the whole file
				   );
			ASSERT( pMappedView );
		
			int32 offset = 0;
			while ( offset < (int32)dwSize )
			{  // stream all data
				int32 iReady;
				try
				{
					// See if the plugin is ready
					iReady =  pInst->GetPlugInFuncs()->writeready( m_pPlgInInst, &m_streamHdl );
				}
				catch( ... )
				{ // plugin caused an error, we may be unstable now
	 				m_iError = NPERR_GENERIC_ERROR;
					break;
				}

				if ( iReady == -1 )
				{
				 	iReady = dwSize - offset;
				}
				else
				{
					if ( (offset + iReady) > (int32)dwSize )
					{
						iReady = dwSize - offset;
					}
				}

				if ( iReady )
				{
					try
					{
						// Some plugins don't work if offset is not equal to zero - wierd
						lWrite = pInst->GetPlugInFuncs()->write( m_pPlgInInst, &m_streamHdl, 
					 							0, iReady,	 pMappedView + offset );
						offset += iReady;
					}
					catch( ... )
					{
	 					m_iError = NPERR_GENERIC_ERROR;
						break;
					}
				}
				else
				{
					m_iError = NPERR_GENERIC_ERROR;
					break;
				}

			}
		}

 		::UnmapViewOfFile( pMappedView );
		::CloseHandle( hFileMap );
		::CloseHandle( hFile );

		// stream as file if requested
		if ( m_uMode == NP_ASFILE )
		{
			try
			{
				pInst->GetPlugInFuncs()->writeready( m_pPlgInInst, &m_streamHdl );
					// See if the plugin is ready
				m_iError = NPERR_NO_ERROR;
				pInst->GetPlugInFuncs()->asfile( m_pPlgInInst, &m_streamHdl, pstrFile );
			}
			catch( ... )
			{
				m_iError = NPERR_GENERIC_ERROR;
			}
		}

	}
	else
	{
		m_iError = NPERR_GENERIC_ERROR;
	}

	return m_iError == NPERR_NO_ERROR;
}

bool ChPlugInStream::DestroyStream()
{
	ChPlugInInstance* pInst = ((ChPlugInInstance*)m_pPlgInInst->ndata);

	bool boolReturn;

	try
	{
		boolReturn = ( NPERR_NO_ERROR == pInst->GetPlugInFuncs()->destroystream( 
									m_pPlgInInst, &m_streamHdl, m_iError ));

		if ( m_pURLNotifyData &&  pInst->GetPlugInFuncs()->urlnotify )
		{
			pInst->GetPlugInFuncs()->urlnotify( m_pPlgInInst, m_strLocalFile, 
				m_iError, m_pURLNotifyData );
		}
	}
	catch( ... )
	{
		boolReturn = false;
	}
	return 	boolReturn;
}

void ChPlugInStream::OnLoadComplete( const ChString& strFile, const ChString& strURL, 
									const ChString& strMimeType )
{	
	m_strURL = strURL;
	m_strMimeType = strMimeType;
	SetLocalFile( strFile );

	if ( !New() )
	{
		DestroyStream();
		return;
	}

	// stream the data

	if ( !StreamFile( strFile ) )
	{
		DestroyStream();
		return;
	}

	DestroyStream();

	delete this;
}



void ChPlugInStream::OnLoadError( chint32 lError, const ChString& strURL )
{	
	
	m_iError = (int)lError;
	m_strURL = strURL;

	New();
	DestroyStream();
}

// $Log$
