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

	This file consists of interfaces used by the Pueblo client core.  This
	file is only used on the client.

----------------------------------------------------------------------------*/

// $Header$


#include "headers.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#if defined( CH_MSW )

	#include <ChReg.h>
	#include <ChRMenu.h>
	#include <ChMsgTyp.h>
	#include <ChDDE.h>
	#include <ChUtil.h>
	#include <ChWebTracker.h>
	#include "ChPrApps.h"
	#include "ChLicDlg.h"
	#include "ChAccDlg.h"
#else

	#include <stdio.h>
	#include <ChTypes.h>
	#include <ChModule.h>
	#include <ChModMgr.h>
	#include <ChRMenu.h>
	#include <ChMsgTyp.h>
	#include <ChPerFrm.h>
	#include <ChDialogs.h>

	#define IDCANCEL	1

#endif // defined( CH_MSW )

#include "ChPbModuleMgr.h"
#include "Pueblo.h"
#include "ChMFrame.h"
#include "ChClCore.h"
#include "ChCoreStream.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/



#define MAILTO_URL_PREFIX			"mailto:"


/*----------------------------------------------------------------------------
	Utility function forward declarations
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( void )
PuebloWebPageLoadComplete( chint32 lError, const ChString& strURL,
							const ChString& strFilename );




/*----------------------------------------------------------------------------
	ChClientMainInfo class
----------------------------------------------------------------------------*/

ChClientMainInfo::ChClientMainInfo( ChModuleID idModule, ChCore* pCore ) :
		ChMainInfo( idModule, pCore ),
		coreDispatcher( pCore, idModule, coreDefHandler )
{
}


/*----------------------------------------------------------------------------
	ChClientCore class
----------------------------------------------------------------------------*/


ChClientCore::ChClientCore( ChMainFrame* pFrame ) :
				ChCore(),
				m_modeClient( modeNormal ),
				m_iReqID( 0 ),
				m_boolFlashWindow( 0 ),
				m_boolTrackTime( 0 ),
				m_luSecondsInState( 0 ),
				m_pMenuMgr( 0 ),
				m_pFrame( pFrame )
{
											// Create the module manager

	m_pModuleMgr = new ChPuebloModuleManager( this );
	ASSERT( m_pModuleMgr );

	m_pMenuMgr = new ChRMenuMgr( this );	// One and only one
	ASSERT( m_pMenuMgr );
}


ChClientCore::~ChClientCore()
{
	// Cleanup the menu manager
	delete m_pMenuMgr;

	// Cleanup the module manager
	delete m_pModuleMgr;

}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::StartPueblo
------------------------------------------------------------------------------
	This method loads the initial script for the current frame.
----------------------------------------------------------------------------*/

void ChClientCore::StartPueblo( const ChString& strArg, chuint uOptions )
{
	m_iReqID++;								// ID for this session

	if (uOptions & doLicense)
	{
		if (!DoLicenseDialog())
		{
			PostQuitMessage( 0 );
			return;
		}
	}

   	m_ArgList.Empty();
											// Initialize the command line
	if (!strArg.IsEmpty())
	{
		m_ArgList.AddArg( CMD_LINE, strArg );
	}
											// Notify that we have stared
	if (uOptions & doLoginNotify)
	{
		ChString		strStartURL, strURL;
		ChString		strClientVersion;
		ChString		strData( "\r\n"  );

		strClientVersion = GetClientInfo()->
							GetClientVersion().Format( ChVersion::formatShort );

		LOADSTRING( IDS_STARTUP_URL, strStartURL );		
		strURL.Format( strStartURL, LPCSTR(strClientVersion) );
		
		ChCoreStartReq* pReq = new ChCoreStartReq( GetReqID());
		ASSERT( pReq );
	
		GetHTTPConn()->PostURL( strURL, strData, strData.GetLength(),
								(chparam)pReq );
	}

											// Do we need to register?
	if (uOptions & doRegistration)
	{
		if (!DoRegistration())
		{
			return;
		}
	}

											/* This should be called after the
												main frame has been created */

	m_pMenuMgr->Init( m_pFrame->GetMenu(), 0, menuMsgDefHandler,
						m_pFrame->m_hAccelTable );

	m_pModuleMgr->OnStartup();
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::StartShutdown
------------------------------------------------------------------------------
	This method starts cleanup of all modules for the current frame.
----------------------------------------------------------------------------*/

void ChClientCore::StartShutdown()
{
	// Abort all HTTP requests
	GetHTTPConn()->AbortRequests( true );

	m_modeClient = modeShutdown;

	m_pModuleMgr->HideAll();
	m_pModuleMgr->UnloadAll();
	m_pModuleMgr->UnloadModule( CH_CORE_MODULE_ID );

 	m_pModuleMgr->OnShutDown();

	m_iReqID++;  // ID for this session
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::NewFrameWnd
------------------------------------------------------------------------------
	Creates a new Pueblo Frame.
----------------------------------------------------------------------------*/

void ChClientCore::NewFrameWnd( const ChString& strArgs,
								const ChString& strLabel ) const
{
	((ChApp*)AfxGetApp())->CreateNewFrame( strArgs, strLabel );
}


/*----------------------------------------------------------------------------
	ChClientCore::ActivateFrame
			Activate the named frame.
----------------------------------------------------------------------------*/

bool ChClientCore::ActivateFrame( const ChString& strFrameName )
{
	ChApp*			pApp = (ChApp*)AfxGetApp();
	ChMainFrame*	pFrame;
											/* Get ChMainFrame, casting to
												a non-const value */

	if (pFrame = (ChMainFrame*)pApp->FindFrame( strFrameName ))
	{
		pFrame->ActivateFrame();
	}

	return false;
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetFrameWnd
------------------------------------------------------------------------------
	Get the Frame associated with this frame
----------------------------------------------------------------------------*/

ChPersistentFrame* ChClientCore::GetFrameWnd() 	const
{
	return m_pFrame;
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::SetFrameTitle
------------------------------------------------------------------------------
	Set the title for frame associated with this core
----------------------------------------------------------------------------*/

void ChClientCore::SetFrameTitle( const ChString& strDocName )
{
	ChString				strAppName;
	ChString				strTitle;

	LOADSTRING( AFX_IDS_APP_TITLE, strAppName );

	if (!strDocName.IsEmpty())
	{									// The title is the world name

		if (ChUtil::GetSystemProperties() & CH_PROP_WIN95)
		{
			strTitle = strDocName + " - " + strAppName;
		}
		else
		{
			strTitle = strAppName + " - " + strDocName;
		}
	}
	else
	{									// The title is the app name
		strTitle = strAppName;
	}
										// Set the new text
	m_pFrame->SetWindowText( strTitle );
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetFrameName
------------------------------------------------------------------------------
	Get the name of frame associated with this core
----------------------------------------------------------------------------*/

ChString ChClientCore::GetFrameName() const
{
	return m_pFrame->GetLabel();
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::SetFrameName
------------------------------------------------------------------------------
	Rename the frame associated with this core
----------------------------------------------------------------------------*/

void ChClientCore::SetFrameName( const ChString& strFrameName )
{
	m_pFrame->SetLabel( strFrameName );
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::EnumerateFrames
------------------------------------------------------------------------------

	Enumerate all the frames currently up and running
----------------------------------------------------------------------------*/

class ChEnumPuebloFrames : public ChPuebloFrameVisitor
{
	public:
		ChEnumPuebloFrames( ChFrameVisitor* pEnumFrame ) :
						m_pEnumFrame( pEnumFrame )
				 {
				 }

		virtual bool Visit( const ChMainFrame* pFrame );

	private :
		 ChFrameVisitor*	m_pEnumFrame;
};


bool ChEnumPuebloFrames::Visit( const ChMainFrame* pFrame )
{
	return m_pEnumFrame->Visit( pFrame->GetLabel(),
								pFrame->GetPuebloCore() );
}


void ChClientCore::EnumerateFrames( ChFrameVisitor& enumFrame )
{
	ChEnumPuebloFrames	enumPuebloFrames( &enumFrame );
	ChApp*				pApp = (ChApp*)AfxGetApp();

											// Enum all frames and notify user
 	pApp->EnumerateFrames( &enumPuebloFrames );
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetSplitter
------------------------------------------------------------------------------

	Get the splitter associated with the current frame
----------------------------------------------------------------------------*/

ChSplitter* ChClientCore::GetSplitter()
{
	return m_pFrame->GetSplitter();
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetDDEConn
------------------------------------------------------------------------------

	Get the DDE conn object associated with the current frame
----------------------------------------------------------------------------*/

ChHTTPDDE* ChClientCore::GetDDEConn()
{
	return ChApp::GetDDEConn();
}


/*----------------------------------------------------------------------------
	ChClientCore::GetDDEConn

		Get the HTTP socket conn object associated with the current frame.
----------------------------------------------------------------------------*/

ChHTTPConn* ChClientCore::GetHTTPConn()
{
	return m_pModuleMgr->GetHTTPConnection();
}


/*----------------------------------------------------------------------------
	ChClientCore::GetDDEConn

		Map module ID to a string name
----------------------------------------------------------------------------*/

ChString ChClientCore::GetModuleName( const ChModuleID& idModule )
{
	return m_pModuleMgr->GetModuleName( idModule );
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetMainInfo
------------------------------------------------------------------------------

	Get the maininfo object for idModule
----------------------------------------------------------------------------*/

ChMainInfo* ChClientCore::GetMainInfo( const ChModuleID& idModule,
									const char* pstrFrameName/* = 0 */ ) const
{

	if ( 0 == pstrFrameName )
	{
		ChModuleRunInfo*	pRunInfo = m_pModuleMgr->GetRunInfo( idModule );

		if (pRunInfo)
		{
			return pRunInfo->GetMainInfo();
		}
	}
	else
	{
		ChApp*				pApp = (ChApp*)AfxGetApp();
		const ChMainFrame*	pFrame = pApp->FindFrame( pstrFrameName );

		if (pFrame)
		{									// There is a frame by this name
			ChModuleRunInfo*	pRunInfo;

			pRunInfo = pFrame->GetPuebloCore()->GetModuleMgr()->
														GetRunInfo( idModule );
			if (pRunInfo)
			{
				return pRunInfo->GetMainInfo();
			}
		}

	}

	return 0;
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetMainInfo
------------------------------------------------------------------------------

	Get the maininfo object for idModule
----------------------------------------------------------------------------*/
ChMainInfo* ChClientCore::GetMainInfo( const ChString& strModule,
									const char* pstrFrameName/* = 0 */ ) const
{
	ChModuleID idModule = m_pModuleMgr->GetModuleID( strModule );

	return GetMainInfo(	idModule, pstrFrameName );
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::RegisterDispatcher
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::RegisterDispatcher( const ChModuleID idModule,
										ChDispatcher *pDispatcher )
{
	ChModuleRunInfo* pRunInfo =  m_pModuleMgr->GetRunInfo( idModule );
	ASSERT( pRunInfo );
	pRunInfo->SetDispatcher( pDispatcher );

}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::UnregisterDispatcher
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::UnregisterDispatcher( const ChModuleID idModule )
{
	ChModuleRunInfo* pRunInfo =  m_pModuleMgr->GetRunInfo( idModule );
	ASSERT( pRunInfo );
	pRunInfo->SetDispatcher( 0 );
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::DispatchMsg
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/


chparam ChClientCore::DispatchMsg( const ChModuleID idModule, ChMsg& msg )
{
	ChDispatcher*	pDispatcher;
	chparam			returnVal;

	if (pDispatcher = m_pModuleMgr->GetDispatcher( idModule ))
	{
		returnVal = pDispatcher->Dispatch( msg );
	}
	else
	{
		TRACE( "DISPATCHER NOT FOUND in ChCore::DispatchMsg\n" );
		returnVal = 0;
	}

	return returnVal;
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::AsyncDispatchMsg
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

bool ChClientCore::AsyncDispatchMsg( const ChModuleID idModule, ChMsg* pMsg )
{    // Post a message to the core frame which will do the dispatch
	return (m_pFrame->PostMessage( WM_CHACO_ASYNC_DISPATCH, idModule, (LPARAM)pMsg ) != FALSE);
}




/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::LoadClientModule
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::LoadClientModule( const ChString& strModuleName,
						const ChString& strModuleBase,
						const ChModuleID& idNotifyModule /*= 0 */,
						chparam userData/* = 0 */, bool boolOptional/* = false */,
						bool boolUseExisting /*= true*/ )
{
	// Clear hte argument list
	//GetArgList()->Empty();

	if ( m_pModuleMgr->OnLoadModule( strModuleName, GetArgList() ) )
	{
		ChLoadCompleteMsg	msg( strModuleName, m_pModuleMgr->GetModuleID( strModuleName ),
									ChString( "" ),
									userData );

											/* Send the load complete message
												to the requesting module */
		DispatchMsg( idNotifyModule, msg );

	} else {
		ChLoadErrorMsg	msg( strModuleName, 0, ChString(""), 404,
								userData );

										/* Send the load error message
											to the requesting module */

		DispatchMsg( idNotifyModule, msg );
	}

}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::UnloadModule
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::UnloadModule( const ChModuleID& idModule ) const
{
	m_pModuleMgr->UnloadModule( idModule );
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::UnloadModule
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/
void ChClientCore::UnloadModule( const ChString& strModule ) const
{
	m_pModuleMgr->UnloadModule( m_pModuleMgr->GetModuleID( strModule ) );
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetModuleCount
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/
int	 ChClientCore::GetModuleCount()
{
 	return m_pModuleMgr->GetModuleCount();
}
/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetModuleIDs
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/
int ChClientCore::GetModuleIDs( int iModuleCount, ChModuleID* pModules )
{
 	return m_pModuleMgr->GetModuleIDs( iModuleCount, pModules );
}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::DisplayStatus
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::DisplayStatus( const ChString& strStatus )
{
											/* Display a status message */
	#if defined( CH_MSW )
	{
		GetFrameWnd()->SetMessageText( strStatus );
	}
	#else	// defined( CH_MSW )
	{
		TRACE1( "Status message: %s\n", (const char*)strStatus );
	}
	#endif	// defined( CH_MSW )
}

void ChClientCore::SetStatusPaneText( int iIndex, const ChString& strStatus )
{
	if ( iIndex < paneMax )
	{
		m_pFrame->GetStatusBar()->SetPaneText( iIndex, strStatus );
	}
}


void ChClientCore::AbortRequests( bool boolAbortPrefetch /*= false */, 
				ChHTTPStreamManager* pStreamMgr /*= 0*/ )
{
	GetHTTPConn()->AbortRequests( boolAbortPrefetch, pStreamMgr );	
}

/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::GetURL
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/
bool ChClientCore::GetURL( const ChString strURL, chuint32 flOptions /*= 0 */, 
									ChHTTPStreamManager* pStream /* = 0 */,
									chparam userData /* = 0 */)
{
	ChCoreModuleReq* pReq = 0;

	if ( !(flOptions & ChHTTPConn::PrefetchURL) )
	{ 	// For prefetch do not allocate any user data
	 	pReq = new ChCoreModuleReq( GetReqID(), pStream, userData );
		ASSERT( pReq );
	}

	return GetHTTPConn()->GetURL( strURL, (chparam)pReq, 0, flOptions );	
}


/*----------------------------------------------------------------------------
	ChClientCore::GetURL
----------------------------------------------------------------------------*/

bool ChClientCore::PostURL( const ChString strURL, const char* pData, chint32 lLen, 
									chuint32 flOptions /*= 0 */, 
									ChHTTPStreamManager* pStream /* = 0 */,
									chparam userData /* = 0 */ )
{
	ChCoreModuleReq* pReq = 0;

 	pReq = new ChCoreModuleReq( GetReqID(), pStream, userData );
	ASSERT( pReq );

	return	GetHTTPConn()->PostURL( strURL, pData, lLen, 
										(chparam)pReq, 0, flOptions );	

}


/*----------------------------------------------------------------------------
	FUNCTION	||	ChClientCore::DisplayWebPage
------------------------------------------------------------------------------

----------------------------------------------------------------------------*/

void ChClientCore::DisplayWebPage( const ChString& strURL, int iBrowser )
{
	ChRegistry			regApps( CH_APPS_GROUP );
	ChString				strBrowser;
	bool				boolInternal;

	regApps.Read( CH_APP_WEBBROWSER, strBrowser, CH_APP_WEBBROWSER_DEF );
	regApps.ReadBool( CH_APP_WEBTRACKER, boolInternal, CH_APP_WEBTRACKER_DEF );

#ifdef _DEBUG
	afxDump << "ChClientCore::DisplayWebPage(\"" << strURL << "\", " << iBrowser
					<< ")\n  boolInternal == " << (BOOL)boolInternal
					<< "\n  strBrowser == \"" << strBrowser << "\"\n";
#endif

	if (!boolInternal && strBrowser.IsEmpty())
	{										/* Ask user how he wants the URL
												to be displayed */
		ChString		strFilter;
		ChString		strTitle;

		LOADSTRING( IDS_OPEN_WEB_BROWSER_FILTER, strFilter );
		LOADSTRING( IDS_OPEN_WEB_BROWSER_TITLE, strTitle );

		TRACE0("(Prompting for browser application)\n");

		ChWebBrowserSelectFileDlg	browseDlg( strTitle, strBrowser,
												strFilter, GetFrameWnd() );

		browseDlg.DoModal();
											// Read the registy again

		regApps.Read( CH_APP_WEBBROWSER, strBrowser, CH_APP_WEBBROWSER_DEF );
		regApps.ReadBool( CH_APP_WEBTRACKER, boolInternal,
							CH_APP_WEBTRACKER_DEF );
	}

	if ( boolInternal && iBrowser == ChCore::browserExternal && !strBrowser.IsEmpty() )
	{
	 	boolInternal = false;
	}

	if (boolInternal || iBrowser == ChCore::browserWebTracker || strBrowser.IsEmpty() )
	{
#ifndef CH_NO_WEBTRACKER
		if (0 == strURL.Find( MAILTO_URL_PREFIX ))
		{
			AfxMessageBox( IDS_MAILTO_NOT_SUPPORTED,
						   MB_OK | MB_ICONEXCLAMATION );
		}
		else
		{

			if ( !ChWebTracker::LoadWebTracker( strURL, 0, m_pFrame ) )
			{										  
				AfxMessageBox( "Failed to start WebTracker",
						   MB_OK | MB_ICONEXCLAMATION );
			}

		}
#else
		AfxMessageBox( "WebTracker is disabled in this version of Pueblo.  Please "
						"select an external browser instead.", MB_OK | MB_ICONEXCLAMATION );
#endif
	}
	else 
	{
		GetDDEConn()->AbortRequests();
		if (!GetDDEConn()->GetURL( strURL, 0, 0, ChHTTPConn::UseDDEOnly ) )
		{
			#if defined( CH_MSW )
			{
				ChString strProgram( strBrowser );

				strProgram = TEXT( '"' ) + strProgram + TEXT( '"' );
				strProgram += TEXT( ' ' );
				strProgram +=  strURL;

				WinExec( strProgram, SW_SHOW );
			}
			#elif defined( CH_UNIX )
			{
				TRACE( "ChClientCore::OnUpdateLoadComplete : "
						"We need to do an exec here!" );
			}
			#else
			{
				#error "Platform not defined!"
			}
			#endif	// defined( CH_UNIX )
		}
	}
}


/*----------------------------------------------------------------------------
	ChClientCore::OnSecondTick

		This function will be called by the application approximately once
		per second.  It is used to perform time-sensitive and periodic
		tasks.
----------------------------------------------------------------------------*/

void ChClientCore::OnSecondTick( time_t timeCurr )
{
	GetModuleMgr()->OnSecondTick( timeCurr );
											/* Increment the counter of the
												number of seconds we've been
												in the current state */
	if (m_boolTrackTime)
	{
		m_luSecondsInState++;
		UpdateSessionTime();
	}
}


void ChClientCore::UpdateSessionPane( const ChString& strPaneText,
										bool boolStartTracking )
{
	m_strSessionText = strPaneText;
	m_luSecondsInState = 0;
	m_boolTrackTime = boolStartTracking;

	if (!boolStartTracking)
	{										/* Display the text when not
												tracking time */

		m_pFrame->GetStatusBar()->SetPaneText( paneSessionTime,	m_strSessionText );
	}
}

void ChClientCore::UpdateSessionTime()
{
	ChString strText;
	chint32	iTime;
	ChString	strTime;
	ChTimeSpan		timePassed( m_luSecondsInState );

	if (iTime = timePassed.GetDays())
	{
		strTime.Format( " %dd %dh ", iTime, timePassed.GetHours() );
	}
	else if (iTime = timePassed.GetHours())
	{
		strTime.Format( " %d:%02dh ", iTime, timePassed.GetMinutes() );
	}
	else
	{
		strTime.Format( " %d min ", timePassed.GetMinutes() );
	}

	strText = " " + m_strSessionText + strTime;
	m_pFrame->GetStatusBar()->SetPaneText( paneSessionTime,	strText );
}



/*----------------------------------------------------------------------------
	ChClientCore protected methods
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
    ChClientCore::DoLicenseDialog
------------------------------------------------------------------------------
    This method displays the beta license agreement and requests that the
    user agree to it.  If the user doesn't agree, the application is shut
    down.
----------------------------------------------------------------------------*/

bool ChClientCore::DoLicenseDialog()
{
    bool    boolAccepted = true;
	ChString  strFilename;

	// make the file name
	if ( !GetModuleFileName( NULL, strFilename.GetBuffer( 512 ), 512 ) )
	{
		strFilename.ReleaseBuffer();
		TRACE( "GetModuleFileName function failed !!!!" );
		ASSERT( 0 );
	}

	strFilename.ReleaseBuffer();
	// License file is stored where Pueblo.exe is located
	strFilename = strFilename.Left( strFilename.ReverseFind( TEXT( '\\' ) ) + 1 );
	strFilename += LICENSE_FILE;



    #if defined( CH_MSW )
    {
        ChLicenseDlg    licenseDlg;
        int             iResult;

        iResult = licenseDlg.DoModal( strFilename );

        if (IDCANCEL == iResult)
        {
            boolAccepted = false;
        }
    }
    #elif defined( CH_UNIX )
    {
        cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;
    }
    #endif  // defined( CH_UNIX )

	return boolAccepted;

}


bool ChClientCore::DoRegistration()
{
	ChRegistry	regInfo( CH_GENERAL_GROUP );
	bool		boolRegistered = false;
   	ChString		strRegInfo;
	chint32		lRegistered;

	regInfo.Read( CH_REGISTERED, lRegistered );

	if (CH_REGISTERED_USER == lRegistered)
	{										/* User has registered but we were
												unable to send the info to the
												server, so lets try sending it
												again. Read the saved file into
												strRegInfo. */
		ChString		strName;
		ChString		strRegInfoFile;

		LOADSTRING( IDS_REGINFO_FILE, strName );

	  	strRegInfoFile = GetModuleMgr()->GetAppDirectory() + strName;

		if (ChUtil::FileExists( strRegInfoFile )) 
		{
			#if defined( CH_MSW )

			struct _stat temp_stat;
			_stat( strRegInfoFile, &temp_stat );

			#else

			struct stat temp_stat;
			stat( strRegInfoFile, &temp_stat );

			#endif

			int			iFileSize = temp_stat.st_size;
			std::fstream		streamIn( strRegInfoFile, std::ios::in );

			if (streamIn.is_open())
			{  
				char*		pstrBuffer = strRegInfo.GetBuffer( iFileSize );

				ASSERT( pstrBuffer );

				streamIn.read( pstrBuffer, iFileSize );
				strRegInfo.ReleaseBuffer();
			}
			streamIn.close();
		}
		boolRegistered = true;
	}
	else if (CH_REGISTRATION_NOTIFIED == lRegistered)
	{
											// User trying to register again

		if(IDNO == AfxMessageBox( IDS_REGISTER_AGAIN,
									MB_YESNO | MB_ICONQUESTION ))
		{ 
			return true;
		}
	}

	#if defined( CH_MSW )
	{
		if ((CH_UNREGISTERED_USER == lRegistered) ||
			(CH_REGISTRATION_NOTIFIED == lRegistered))
		{
			int				iResult;
			ChAccountDlg	accountWizard;

											// Present the dialog

			switch (iResult = accountWizard.DoModal())
			{
				case IDFINISH:
				{
					accountWizard.GetWizardData( strRegInfo );
											/* Send the information to the
												server */
					boolRegistered = true;
					break;
				}

				case IDCANCEL:
				case IDABORT:
				{
					break;
				}
			}
		}
	}
	#else	// defined( CH_MSW )
	{
	}
	#endif	// defined( CH_MSW )

	if (boolRegistered)
	{										/* Send the registration Info user
												has registered, so we will not
												bug him/her any more */
		ChString	strRegURL, strURL;
		ChString	strClientVersion;

		regInfo.Write( CH_REGISTERED, CH_REGISTERED_USER );

		strClientVersion = GetClientInfo()->
							GetClientVersion().Format( ChVersion::formatShort );

		LOADSTRING( IDS_REGISTER_URL, strRegURL );
		strURL.Format( strRegURL, (const char*)strClientVersion );
		
		ChCoreRegisterReq* pReq = new ChCoreRegisterReq( GetReqID(),
															strRegInfo );
		ASSERT( pReq );

		{  // save it just in case we cannot register with the server
			ChString strName;
			LOADSTRING( IDS_REGINFO_FILE, strName );
		  	ChString strRegInfoFile( GetModuleMgr()->GetAppDirectory() + strName );

			std::fstream streamOut( strRegInfoFile, std::ios::out );

			if ( streamOut.is_open() )
			{  
				streamOut.write( strRegInfo, strRegInfo.GetLength() );
			}
			streamOut.close();

			::SetFileAttributes( strRegInfoFile, FILE_ATTRIBUTE_HIDDEN );
		}

		GetHTTPConn()->PostURL( strURL, strRegInfo, strRegInfo.GetLength(),
								(chparam)pReq );
	}
	else if (CH_UNREGISTERED_USER == lRegistered)
	{
											/* If user cancels registration
												then check if we are past 30
												days.  If true then tell the
												user client is not useable
												without registering. */
		chuint32	luTime;
		time_t		currTime = time( 0 );

		if (regInfo.Read( CH_PRODUCT_ID, luTime ) && luTime)
		{
			luTime += (REG_REQUIRE_DAYS - REG_PROMPT_DAYS) *
						SECONDS_IN_A_DAY;

			if ((chuint32)currTime > luTime)
			{								// disable the client
				m_modeClient = modeDisabled;
				AfxMessageBox( IDS_REGISTRATION_WARNING, MB_OK | MB_ICONSTOP );

				return false;
			}
		}
	}

	return true;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
