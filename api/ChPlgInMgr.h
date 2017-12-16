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

	This file consists of the interface for the PlugIn manager class.

----------------------------------------------------------------------------*/

#include <ChSplay.h>
#include "ChPlgInAPI.h"

#define  USE_NATIVE_HWND 

#define	 WM_PLUGIN_SET_WINDOW		( WM_USER + 200 )
#define  WM_PLUGIN_NEW_STREAM		( WM_USER + 201 )
#define  WM_PLUGIN_STREAM_FILE		( WM_USER + 202 )
#define  WM_PLUGIN_DESTROY_STREAM 	( WM_USER + 203 )
#define	 WM_PLUGIN_INIT_STREAM		( WM_USER + 204 )

class ChPlugInStream;
class ChPlugInInstance;
class ChPlugInMgr;
class ChHtmlView;
class ChArgumentList;

typedef ChPtrSplay<ChPlugInStream>		ChPlgInStrmLst;
typedef ChPtrSplay<ChPlugInInstance>	ChPlgInInstLst;

class ChPluginDllInfo
{
	public :
		ChPluginDllInfo( )  {}
		ChPluginDllInfo( const ChString& strDll,const ChString& strFileExtent,
						const ChString& strFileOpenName, const FILETIME& fileTime,
						bool boolPuebloPlugin ) :
							 m_strDll( strDll ), m_strFileExtent( strFileExtent ),
							 m_strFileOpenName( strFileOpenName ), 
							 m_fileTime( fileTime ),
							 m_boolPuebloPlugin( boolPuebloPlugin ) 
							 {
							 }
		ChPluginDllInfo( const ChPluginDllInfo& dllInfo ) 
				{
					m_strDll 			= dllInfo.m_strDll;	
					m_strFileExtent 	= dllInfo.m_strFileExtent;	
					m_strFileOpenName 	= dllInfo.m_strFileOpenName;	
					m_fileTime			= dllInfo.m_fileTime;	
					m_boolPuebloPlugin	= dllInfo.m_boolPuebloPlugin;	
				}
		ChPluginDllInfo* operator=( ChPluginDllInfo& dllInfo )
				{
					m_strDll 			= dllInfo.m_strDll;	
					m_strFileExtent 	= dllInfo.m_strFileExtent;	
					m_strFileOpenName 	= dllInfo.m_strFileOpenName;
					m_fileTime			= dllInfo.m_fileTime;	
					m_boolPuebloPlugin	= dllInfo.m_boolPuebloPlugin;	
					return this;
				}
		const ChString& GetDllName()		{ return m_strDll; }
		const ChString& GetFileExtent()	{ return m_strFileExtent; }
		const ChString& GetFileOpenName()	{ return m_strFileOpenName; }
		const FILETIME& GetFileTime()	{ return m_fileTime; }
		bool IsPuebloPlugin()			{ return m_boolPuebloPlugin; }

	private :
		ChString 	 	m_strDll;
		ChString 	 	m_strFileExtent;
		ChString 	 	m_strFileOpenName;		 
		FILETIME	m_fileTime;
		bool		m_boolPuebloPlugin;

		
};

typedef ChSplay< ChString, ChPluginDllInfo> 		ChPlugInDllLst;


class ChPluginRunTimeInfo
{
	public :
		ChPluginRunTimeInfo( HINSTANCE hLibrary, NP_ShutdownUPP pShutdown ) :
							m_hLibrary( hLibrary ),
							m_pprocShutdownPlugIn( pShutdown ), 
							m_iUseCount( 0 )
						{
							ChMemClearStruct( &m_plgInEntryFuncs );
							m_plgInEntryFuncs.size = sizeof( NPPluginFuncs );

						}


		~ChPluginRunTimeInfo()
					{  	// if we have use count, somebody did not cleanup properly
						if ( m_iUseCount )
						{
							ShutDown();
						}	
					}


		HINSTANCE	GetLibrary() const		{ return m_hLibrary; }
		void 		ShutDown()				
						{
							m_pprocShutdownPlugIn();
							::FreeLibrary( m_hLibrary );
						}
		int 		GetUseCount() const		{ return m_iUseCount; }
		void 		Use()					{ m_iUseCount++; }
		void 		Release()				{ m_iUseCount--; }


		NPPluginFuncs*  GetPlugInFuncs()    { return &m_plgInEntryFuncs; }


	private :
		HINSTANCE				m_hLibrary;				// library 
		int						m_iUseCount;
		NP_ShutdownUPP			m_pprocShutdownPlugIn;	// term entry
		NPPluginFuncs			m_plgInEntryFuncs;		// Plugin entry points
};


typedef ChSplay< ChString, ChPluginRunTimeInfo*> 		ChPluginRunnning;



class ChPlugInStream
{
	public :
		enum tagState { newStream = 1, streamFile, destroyStream };

		ChPlugInStream( NPP pPlgInInst, const ChString& strURL, const ChString& strMimeType );
		ChPlugInStream( NPP pPlgInInst, const ChString& strURL, const ChString& strMimeType, void* pData );
		~ChPlugInStream();

		bool New();
		bool StreamFile( const char* pstrFile );
		bool DestroyStream();

		// called by HTTP after completion
		void OnLoadComplete( const ChString& strFile, const ChString& strURL,
									const ChString& strMimeType );
		void OnLoadError(  chint32 lError, const ChString& strURL );

		
		const char* GetLocalFile()		{ return m_strLocalFile; }
		void  SetLocalFile( const char* pstrLocalFile ) {  m_strLocalFile = pstrLocalFile; }
		void  SetState( int iState )	{ m_iState = iState; } 
		NPP   GetNetscapeInstance()		{ return m_pPlgInInst; }
	private :
		ChString		m_strURL;
		ChString		m_strMimeType;
		ChString		m_strLocalFile;
		NPStream	m_streamHdl;
		NPP 		m_pPlgInInst;
		uint16 		m_uMode;
		NPError		m_iError;
		int			m_iState;
		void*		m_pURLNotifyData;
};


class ChPlugInInstance
{
	public :
		ChPlugInInstance( const ChString& strURL, const ChString& strMimeType, uint16 mode );
		~ChPlugInInstance();

		// attributes
		NPP					GetInstanceHandle()		{ return &m_instHdl; }
		NPPluginFuncs*  	GetPlugInFuncs()		{ return m_pplgInEntryFuncs; }
		ChPluginRunnning*	GetRunList()			{ return &m_plgRunning; }
		ChHtmlView*			GetHtmlView()			{ return m_pHtmlView; }
		#if defined ( USE_NATIVE_HWND )
		HWND				GetPluginWindow()		{ return m_hWnd; }
		#else
		CWnd*				GetPluginWindow()		{ return &m_hWnd; }
		#endif

		// methods
		bool Initialize( const char* pstrModule  );
		bool New( ChHtmlView* pHtmlInst, const ChString& strLocalFile, ChSize& sizeWindow,
								ChArgumentList* pArgs );
		bool SetWindow( const ChRect& rtFrame, bool boolAlways = false  );


	private :
		NPP_t					m_instHdl;  			// instance handle
		NPWindow  				m_window;				// Netscape window handle

		NPPluginFuncs*			m_pplgInEntryFuncs;		// Plugin entry points
		//ChPlugInStream*  		m_pStream;
		//bool					m_boolStreamFile;
		//ChString					m_strLocalFile;
		static NPNetscapeFuncs		m_puebloEntryFuncs;		// Pueblo entry points
		static ChPluginRunnning		m_plgRunning;


		ChHtmlView*				m_pHtmlView;
		#if defined ( USE_NATIVE_HWND )
		HWND					m_hWnd;					// native window handle
		#else
		CWnd					m_hWnd;
		#endif
		ChString					m_strURL;
		ChString 					m_strMimeType;
		ChString					m_strModule;
		uint16					m_mode;
};

class ChPlugInMgr
{
	public :
		ChPlugInMgr();
		~ChPlugInMgr();
		static void Initialize();

		bool LoadPlugInModule( ChHtmlView*	pHtmlView, const ChString& strLocalFile, 
								const ChString& strURL, const ChString& strMimeType, 
								ChObjInline* pPlugIn );

		bool HandleMimeType( const ChString& strMimeType );

		static const char* GetClassName();
	private :

		static void LocatePlugIn( const ChString& strDir, bool boolPuebloPlugin );
 		static bool FindPlugIn( const ChString& strMimeType, ChString& strPlgInModule );
		static bool AddToList( const ChString strPlgInMime, 
							ChPluginDllInfo& info, bool boolPuebloPlugin );

	private : 
		static ChPlugInDllLst		m_plgInDllList;
		static ChString				m_strPluginWndClass;
		static ChString 				m_strPuebloPluginDir;
		static ChString 				m_strNetscapePluginDir;

};
