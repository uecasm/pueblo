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

	This file contains the implementation of Pueblo module manager core.  This
	file is only used on the client.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChReg.h>
#include <ChUtil.h>
#include <ChWorld.h>
#include <ChSound.h>
//#include <ModTest.h>

#include "ChPbModuleMgr.h"
#include "ChClCore.h"


CH_DECLARE_MAIN_HANDLER( coreMainHandler )

// There is only one module list and a ID map table for one APP
ChPuebloModuleList			ChPuebloModuleManager::m_moduleList;
ChPuebloModuleIDMap			ChPuebloModuleManager::m_moduleIDMap;
ChString						ChPuebloModuleManager::m_strAppDir;

///////////////////////////////////////////////////////////////////////////////
////////////   ChPuebloModuleManager
////////////
//////////////////////////////////////////////////////////////////////////////

ChPuebloModuleManager::ChPuebloModuleManager( ChClientCore* pCore  ) :
						m_pCore( pCore ),
						m_coreStreamManager( pCore ),
						m_httpConnection( &m_coreStreamManager, 0 /*ChHTTPConn::connLocalState */ ),
						m_idGenerator( 1000 ),
						m_boolInUnloadAll( false )

{
	ASSERT( pCore );
	InitModuleManager();

}
ChPuebloModuleManager::~ChPuebloModuleManager()
{
	
}


ChModuleID ChPuebloModuleManager::GetModuleID( const ChString& strModule )
{
	ChModuleInfo* pInfo = m_moduleList.Find( strModule ); 

	if ( pInfo )
	{
		return pInfo->GetModuleID();
	}

	return( 0 );
	
}
ChString ChPuebloModuleManager::GetModuleName( const ChModuleID idModule )
{
	ChString* pstrModule = m_moduleIDMap.Find( idModule ); 

	if ( pstrModule )
	{
		return *pstrModule;
	}

	return( ChString("") );
}

ChModuleRunInfo* ChPuebloModuleManager::GetRunInfo( const ChModuleID idModule )
{
	ChModuleRunInfo** pRunInfo = m_moduleRunTable.Find( idModule );

	if ( pRunInfo && *pRunInfo )
	{
		return *pRunInfo;
	}
	return  0; 
}

ChDispatcher* ChPuebloModuleManager::GetDispatcher( const ChModuleID idModule )
{
	ChModuleRunInfo** pRunInfo = m_moduleRunTable.Find( idModule );

	if ( pRunInfo && *pRunInfo )
	{
		return (*pRunInfo)->GetDispatcher();
	}

	return 0;
}

void ChPuebloModuleManager::InitModuleManager()
{

	if ( m_strAppDir.IsEmpty() )
	{
		if ( !GetModuleFileName( NULL, m_strAppDir.GetBuffer( 512 ), 512 ) )
		{
			m_strAppDir.ReleaseBuffer();	
			TRACE( "GetModuleFileName function failed !!!!" );
			ASSERT( 0 );
		}

		m_strAppDir.ReleaseBuffer();	
		// path of application, scrpit modules are stored relative to app path
		m_strAppDir = m_strAppDir.Left( m_strAppDir.ReverseFind( TEXT( '\\' ) ) + 1 );

		// Empty the current list and rebuild 
		m_moduleList.Erase();
		// Generate module IDs from 1000
		m_idGenerator = 1000;

		// Add the core module to the list
		ChModuleInfo modInfo( CH_CORE_MODULE_ID, ChString("") );
		m_moduleList.Insert( CH_CLIENT_CORE_NAME, modInfo ); 	
		m_moduleIDMap.Insert( CH_CORE_MODULE_ID, CH_CLIENT_CORE_NAME); 	

		// Build the list of all loadable modules
		BuildModuleList();
	}

}

bool ChPuebloModuleManager::InitCore()
{
	bool			boolSuccess = true;
	ChMainInfo		*pMainInfo;
	ChInitMsg		msg( CH_CORE_MODULE_ID, "" );
	ChString			strModuleName( CH_CLIENT_CORE_NAME );


	#if defined( CH_MSW )
	ChModuleRunInfo*	pModuleInfo = new ChModuleRunInfo( strModuleName,  0 );
	#else
	ChModuleRunInfo*	pModuleInfo = new ChModuleRunInfo( strModuleName );
	#endif

	ASSERT( pModuleInfo ); 
	m_moduleRunTable.Insert( CH_CORE_MODULE_ID, pModuleInfo );


	pMainInfo = (ChMainInfo *)(coreMainHandler( msg, m_pCore, 0, 
						CH_CORE_MODULE_ID, &strModuleName, 0 ));

	pModuleInfo->Set( coreMainHandler, pMainInfo );

	if ( 0 == pMainInfo )
	{

		m_moduleRunTable.Delete( CH_CORE_MODULE_ID );
	  	delete pModuleInfo;
		boolSuccess = false;
	}
	else
	{
		pModuleInfo->GetDispatcher()->Dispatch( msg );
	}

	return boolSuccess;
}


bool ChPuebloModuleManager::OnStartup()
{
	TRACE0("ChPuebloModuleManager::OnStartup()\n");

	// Initialize core for this instance
	InitCore();

	ChArgumentList* pArgList = m_pCore->GetArgList();

	ChString			strLoadParam( "http://pueblo.sf.net/worldlist.php" );
	// Note: in a future version we might add "?mode=kids" to the above to get
	//       a kid-safe list (one without worlds flagged 'adult').
	pArgList->AddArg( "PuebloList",  strLoadParam );

	// This will be lanuched by the script handler
	// UE: I've replaced the exact string here with the definition from the
	//     header file, making one less place that it needs to be altered in the
	//     event that it gets changed.
	OnLoadModule( ChString(CH_MODULE_WORLD), pArgList );
	//OnLoadModule( ChString(CH_MODULE_TEST), m_pCore->GetArgList() );

	//pArgList->Empty();

	#if 0
	ChRegistry		reg( CH_LOGIN_GROUP );

	if (reg.Read( CH_STARTUP_SCRIPT_URL, strURL ))
	{
					
	}
	else
	{ // No script specified load the default script
	  // Get the local default script file name
	  	ChString strScriptFile( GetAppDirectory() + DEFAULT_SCRIPT );
		ParseScript( strScriptFile );
	}
	#endif

	return true;
}


bool ChPuebloModuleManager::OnShutDown()
{

	m_httpConnection.AbortRequests( true );
	return true;
}


bool  ChPuebloModuleManager::OnLoadModule( const ChString& strModuleName, ChArgumentList *pArgList)
{

	ChString 	strLibrary;
	bool	boolSuccess = true;

	TRACE1("ChPuebloModuleManager::OnLoadModule(\"%s\", args)\n", (LPCSTR)strModuleName);

	if ( FindModule( strModuleName, strLibrary ) )
	{	// We have the module, load it
		TRACE0("  (module found");
		ChModuleRunInfo** pRunInfo =  m_moduleRunTable.Find( GetModuleID( strModuleName ) );

		if ( pRunInfo && *pRunInfo )
		{ // Module is already loaded, up the use count
			TRACE0(", preloaded)\n");
			(*pRunInfo)->Use();
		}
		else 
		{  // Load the module, if successful add it to the run table
			TRACE0(", now loading...)\n");
			boolSuccess = LoadPuebloModule( strModuleName, strLibrary, pArgList );

		}
	}
	else
	{	 // not in our list, request server to download
			TRACE0("  (don't know it - download?)\n");
			boolSuccess = false;
	}

	return boolSuccess;

}

bool ChPuebloModuleManager::FindModule( const ChString& strModuleName, ChString& strLibrary )
{
	ChModuleInfo* pInfo = m_moduleList.Find( strModuleName ); 

	strLibrary.Empty();

	if ( pInfo && !pInfo->GetLibraryName().IsEmpty() )
	{
		strLibrary = pInfo->GetLibraryName(); 	
	}

	return 	!strLibrary.IsEmpty();
}

bool ChPuebloModuleManager::LoadPuebloModule( const ChString& strModule, const ChString& strLibrary,
									ChArgumentList *pArgList )
{
	TRACE2("ChPuebloModuleManager::LoadPuebloModule(\"%s\", \"%s\", args)\n",
						(LPCSTR)strModule, (LPCSTR)strLibrary);

	bool			boolSuccess = true;
	ChMainHandler	handler;

	#if defined( CH_MSW )

	HINSTANCE		hLibrary;

	{       
		TRACE2( "LOAD: Library %s (%s)\n", (const char *)strLibrary,
											(const char *)strModule );

		#pragma message( __FILE__ ": Reality Lab workaround : Remove this when we get a fix " )
		#pragma message( "UE note: no longer linking to RL, but fix is still present." )
		// add a work around for the reality lab bug.
		// Reality lab cannot be used from more than one dll, if they are
		// then the dll should never be unloaded till the app terminates
		{
			ChString 	strMod( strLibrary );
			strMod.MakeLower();

			if ( strMod.Find( TEXT( "chgraphx.dll" ) ) != -1 )
			{
				ChUtil::AddModuleToLoadList( strLibrary );
			}
		}
											/* Load the library and get the ChMain
												entrypoint address */
		hLibrary = LoadLibrary( strLibrary );

        #if defined( WIN32 )
		if (0 == hLibrary)
		{
			boolSuccess = false;
			TRACE0("  (LoadLibrary failed)\n");
		}
		#else
		if ( hLibrary <= HINSTANCE_ERROR )  
		{
			boolSuccess = false;
		}
		#endif
		else
		{
			handler = (ChMainHandler)GetProcAddress( hLibrary, CH_MAIN_NAME );
            
 			if (0 == handler)
			{
           		#if defined( WIN32 )
				DWORD		dwError;

				dwError = GetLastError(); 
				#endif

				TRACE0("  (couldn't get entry point)\n");
				boolSuccess = false;
			} 
		}
	}
	#else	// defined( CH_MSW )
	{
		#if defined( linux )
			CH_DECLARE_MAIN_HANDLER( ChMainEntry );
#ifdef CH_CLIENT
			CH_DECLARE_MAIN_HANDLER( ChMainEntryWorld );

			if (strLibrary == CH_MODULE_WORLD) {
				handler = ChMainEntryWorld;
			} else
#endif // CH_CLIENT
			{
				handler = ChMainEntry;
			}
		#else
			void *lib = dlopen( strLibrary, RTLD_NOW );
			if (lib) {
				handler = (ChMainHandler)dlsym( lib, CH_MAIN_NAME );
				if (handler == 0) {
					cerr << dlerror() << endl;
					boolSuccess = false;
				}
			} else {
				cerr << dlerror() << endl;
				boolSuccess = false;
			}
		#endif // !linux
	}
	#endif	// defined( CH_MSW )

	if (boolSuccess)
	{	 
		ChModuleID		idModule = GetModuleID( strModule );
		ChModuleID		idServerModule = 3;
		ChString			strLoadParam;

		ChInitMsg		initMsg( idModule, strLoadParam, idServerModule );
		ChMainInfo*		pMainInfo;

		#if defined( CH_MSW )
		ChModuleRunInfo*	pModuleInfo = new ChModuleRunInfo( strModule,  hLibrary );
		#else
		ChModuleRunInfo*	pModuleInfo = new ChModuleRunInfo( strModule );
		#endif

		ASSERT( pModuleInfo ); 
		m_moduleRunTable.Insert( idModule, pModuleInfo );

		//TRACE0("  (negotiating with module)\n");

		pMainInfo = (ChMainInfo *)(handler( initMsg, m_pCore, 0, idModule,
											&strModule, pArgList ));

		if (pMainInfo)
		{


			pModuleInfo->Set( handler, pMainInfo );
			//TRACE0("  (initialising module)\n");
			pModuleInfo->GetDispatcher()->Dispatch( initMsg );
		}
		else
		{
			::FreeLibrary( hLibrary );
			boolSuccess = false;

			TRACE0("  (negotiation failed)\n");
			m_moduleRunTable.Delete( CH_CORE_MODULE_ID );
		  	delete pModuleInfo;
			boolSuccess = false;
		}
	}

	return boolSuccess;
}


void ChPuebloModuleManager::BuildModuleList()
{
	ChString strDir;

	strDir = GetAppDirectory() + TEXT( "modules" );

	void * pDir = ChUtil::OpenDirectory( strDir, TEXT( "*.*" ), 0xFFFFFFFF );

	if ( !pDir )
	{
		TRACE( "No modules installed !!!" );
		return;
	}

	ChFileAttrs attrs;

	while( ChUtil::ReadDirectory( pDir,  &attrs, ChUtil::reqPath ) )
	{
		if ( attrs.astrName[0] )
		{
			DWORD  dwHandle;
			DWORD dwLen = GetFileVersionInfoSize( 
							attrs.astrName,	// pointer to filename string
				    		&dwHandle 		// pointer to variable to receive zero
			   				);

			if ( dwLen )
			{
				char * pstrBuffer = new char[dwLen];
				ASSERT( pstrBuffer );
				char *pstrValue;
				if ( GetFileVersionInfo(
							attrs.astrName,	// pointer to filename string
						    dwHandle,	// ignored 
						    dwLen,		// size of buffer
						    pstrBuffer 	// pointer to buffer to receive file-version info.
									   ) )
				{
					UINT dwBytes = 0;

					if ( VerQueryValue(pstrBuffer,
				              TEXT("\\StringFileInfo\\140904b0\\PuebloModule"),
				              (LPVOID  *)&pstrValue,
				              &dwBytes) )
					{  // add it to the list if not present
						ChString strModule( pstrValue );

						if ( !strModule.IsEmpty() )
						{
							while( true )
							{
								int iIndex = strModule.Find( TEXT( ';' ));

								if ( iIndex == -1 )
								{
									ChModuleInfo* pInfo = m_moduleList.Find( strModule ); 

									if ( pInfo == 0 )
									{
										ChModuleInfo modInfo( m_idGenerator, ChString(attrs.astrName) );
										m_moduleList.Insert( strModule, modInfo ); 	
										m_moduleIDMap.Insert( m_idGenerator++, strModule); 	
									}
									break;
								}
								else
								{
									ChString strTemp;
									strTemp = strModule.Left( iIndex );

									ChModuleInfo* pInfo = m_moduleList.Find( strTemp ); 

									if ( pInfo == 0 )
									{
										ChModuleInfo modInfo( m_idGenerator, ChString(attrs.astrName) );
										m_moduleList.Insert( strTemp, modInfo ); 	
										m_moduleIDMap.Insert( m_idGenerator++, strTemp); 	
									}

									strModule = strModule.Right( 
													strModule.GetLength() - ( iIndex + 1 ) );
								}
							}
						}

					}

				}
				delete [] pstrBuffer;	
			}

		}
	}
	ChUtil::CloseDirectory( pDir );
}

// $Log$
