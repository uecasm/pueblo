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

	Defines the ChWorld module for the Pueblo system.  This module is
	used to connect to external worlds, either old-style MUDs or Pueblo-
	enhanced virtual worlds.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#ifdef CH_UNIX
#include <ChDispat.h>
#include <ChMsgTyp.h>
#include <ChReg.h>
#include <ChDialogs.h>
#include <ChTypes.h>
#include <ChGraphx.h>
#include "../../unix/ChWorld/UnixRes.h"
#include "ChWList.h"
#endif // CH_UNIX

#include <fstream>
#include <ctype.h>
#include <time.h>

#include <ChCore.h>
#include <ChMsgTyp.h>
#include <ChExcept.h>

#if !defined( CH_PUEBLO_PLUGIN )
#include <ChMenu.h>
#else
#include <ChHttp.h>
#endif

#include <ChHtmWnd.h>
											/* Headers for modules referenced
												by this module */
#include <ChSound.h>

#if defined( CH_MSW )

	#include <ChGraphx.h>

#endif	// defined( CH_MSW )


#include <ChWorld.h>

#include "World.h"
#include "ChWConn.h"
#include "ChTextInput.h"
#include "ChWorldStream.h"
#include "ChConnectDlg.h"

#if defined( CH_MSW )

	#include "ChWListD.h"
#if !defined( CH_PUEBLO_PLUGIN )
	#include "ChAbout.h"
	#include "ChPrefsWorld.h"
	#include "ChPrefsNotify.h"
	#include "ChQuickConnect.h"
#endif

#endif	// defined( CH_MSW )


//#define DebugBox(msg)	::MessageBox(0, msg, "Debug", MB_OK|MB_ICONINFORMATION)
#define DebugBox(msg)

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define PERSONAL_URL			"pueblo:personal"
#define WORLD_LIST_URL			"pueblo:world-list"
#define WORLD_LIST_EDIT_URL		"pueblo:world-list-edit"
#define PERSONAL_URL_OLD		"http://~"
#define	PERSONAL_URL_OLD2		"personal:"
#define MAILTO_URL_PREFIX		"mailto:"
#define DISCONNECT_URL			"pueblo:disconnect"
#define RECONNECT_URL				"pueblo:reconnect"

#define SAVE_DLG_FLAGS			(OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY)
#define SAVE_DLG_FILTER			"Text Files (*.txt)|*.txt|"\
								"HTML Files (*.htm)|*.htm|"\
								"All Files (*.*)|*.*||"
#define SAVE_DLG_FILTER_LONG	"Text Files (*.txt)|*.txt|"\
									"HTML Files (*.htm;*.html)|*.htm;*.html|"\
									"All Files (*.*)|*.*||"
#define SAVE_LOG_DIR			"Logs"


/*----------------------------------------------------------------------------
	Type definitions
----------------------------------------------------------------------------*/

typedef enum { invalid, xworld, href, xcmd, xmode } CommandType;


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( CommandType )
GetCommand( const ChString& strCommand, ChString& strValue, bool boolInline );

CH_INTERN_FUNC( bool )
FormatHint( ChString& strHint );

inline bool IsTargetSelfOutput( const ChString& strTarget )
	{
		bool	boolSelf = (strTarget.Compare( "_self" ) == 0);

		return boolSelf;
	}


/*----------------------------------------------------------------------------
	Socket handler declaration
----------------------------------------------------------------------------*/

CH_DECLARE_SOCKET_HANDLER( worldSocketHandler )
CH_DECLARE_SOCKET_ASYNC_HANDLER( worldSocketAsyncHandler )


/*----------------------------------------------------------------------------
	Handler declarations
----------------------------------------------------------------------------*/

CH_DECLARE_MESSAGE_HANDLER( defWorldHandler )
CH_DECLARE_MESSAGE_HANDLER( worldInitHandler )
CH_DECLARE_MESSAGE_HANDLER( worldShowModuleHandler )
CH_DECLARE_MESSAGE_HANDLER( worldLoadCompleteHandler )
CH_DECLARE_MESSAGE_HANDLER( worldLoadErrorHandler )

#if !defined( CH_PUEBLO_PLUGIN )
CH_DECLARE_MESSAGE_HANDLER( worldGetPageCountHandler )
CH_DECLARE_MESSAGE_HANDLER( worldGetPagesHandler )
CH_DECLARE_MESSAGE_HANDLER( worldGetPageDataHandler )
CH_DECLARE_MESSAGE_HANDLER( worldReleasePagesHandler )
#endif // #if !defined( CH_PUEBLO_PLUGIN )

CH_DECLARE_MESSAGE_HANDLER( worldCommandHandler )
CH_DECLARE_MESSAGE_HANDLER( worldInlineHandler )
CH_DECLARE_MESSAGE_HANDLER( worldHintHandler )
CH_DECLARE_MESSAGE_HANDLER( worldInvalidWorldHandler )
CH_DECLARE_MESSAGE_HANDLER( worldSendWorldCmdHandler )

static ChMsgHandlerDesc	worldHandlers[] =
					{	{CH_MSG_INIT, worldInitHandler},
						{CH_MSG_SHOW_MODULE, worldShowModuleHandler},
						{CH_MSG_LOAD_COMPLETE, worldLoadCompleteHandler},
						{CH_MSG_LOAD_ERROR, worldLoadErrorHandler},
#if !defined( CH_PUEBLO_PLUGIN )
						{CH_MSG_GET_PAGE_COUNT, worldGetPageCountHandler},
						{CH_MSG_GET_PAGES, worldGetPagesHandler},
						{CH_MSG_GET_PAGE_DATA, worldGetPageDataHandler},
						{CH_MSG_RELEASE_PAGES, worldReleasePagesHandler},
#endif
						{CH_MSG_CMD, worldCommandHandler},
						{CH_MSG_INLINE, worldInlineHandler},
						{CH_MSG_HINT, worldHintHandler},
						{CH_MSG_INVALID_WORLD, worldInvalidWorldHandler},
						{CH_MSG_SEND_WORLD_CMD, worldSendWorldCmdHandler} };


/*----------------------------------------------------------------------------
	Chaco menu handlers
----------------------------------------------------------------------------*/

#if !defined( CH_PUEBLO_PLUGIN )

CH_DECLARE_MESSAGE_HANDLER( fileMenuHandler )

CH_DECLARE_MESSAGE_HANDLER( editMenuHandler )
CH_DECLARE_MESSAGE_HANDLER( OnStdEditCopy )
CH_DECLARE_MESSAGE_HANDLER( OnStdEditCut )
CH_DECLARE_MESSAGE_HANDLER( OnStdEditPaste )

CH_DECLARE_MESSAGE_HANDLER( viewMenuHandler )
CH_DECLARE_MESSAGE_HANDLER( OnViewPrevCommand )

CH_DECLARE_MESSAGE_HANDLER( worldMenuHandler )
CH_DECLARE_MESSAGE_HANDLER( OnWorldListCommand )
CH_DECLARE_MESSAGE_HANDLER( OnWorldAddCommand )
CH_DECLARE_MESSAGE_HANDLER( OnWorldCreateShortcutCommand )
CH_DECLARE_MESSAGE_HANDLER( OnWorldQuickConnectCommand )
CH_DECLARE_MESSAGE_HANDLER( OnWorldDisconnectCommand )
CH_DECLARE_MESSAGE_HANDLER( OnWorldLoggingCommand )

CH_DECLARE_MESSAGE_HANDLER( windowMenuHandler )
CH_DECLARE_MESSAGE_HANDLER( OnWindowInputCommand )

#endif // !defined( CH_PUEBLO_PLUGIN )


/*----------------------------------------------------------------------------
	ChWorldTinTin class
----------------------------------------------------------------------------*/

ChWorldTinTin::ChWorldTinTin( ChWorldMainInfo* pMainInfo ) :
				TinTin( pMainInfo )
{
}


void ChWorldTinTin::SendToWorld( const ChString& strOutput )
{
	if (IsOnline())
	{										/* This function doesn't apply
												any tintin processing to the
												string that it is sending */
		GetMainInfo()->Send( strOutput );
	}
}


void ChWorldTinTin::Display( const ChString& strOutput,
								bool boolPreformatted ) const
{
	if (IsOnline())
	{										/* This function doesn't apply
												any tintin processing to the
												string that it is sending */

		GetMainInfo()->Display( strOutput, boolPreformatted );
	}
}


/*----------------------------------------------------------------------------
	ChWorldMainInfo class
----------------------------------------------------------------------------*/

ChWorldMainInfo::ChWorldMainInfo( const ChModuleID& idModule, ChCore* pCore,
										ChArgumentList* pList ) :
					ChMainInfo( idModule, pCore ),
					m_iConnectID( 0 ),
					m_pTextInput( 0 ),
					m_pTextOutput( 0 ),
					m_pWorldConn( 0 ),
					m_pTinTin( 0 ),
					m_pWorldStreamMgr( 0 ),
					m_worldDispatcher( pCore, idModule, defWorldHandler ),
					m_boolPuebloEnhancedFound( false ),
					m_focusTarget( focusNone ),
					m_idSoundModule( 0 ),
					m_idGraphicsModule( 0 ),
					m_pWorldInfo( 0 ),
					m_boolShown( false ),
					m_echoState( echoOn ),
	#if !defined( CH_PUEBLO_PLUGIN )
					m_boolMenus( false ),
					m_boolMenusInstalled( false ),
					m_pStdFileMenu( 0 ),
					m_pStdEditMenu( 0 ),
					m_pStdViewMenu( 0 ),
					m_pWorldMenu( 0 ),
					m_pStdWindowMenu( 0 ),
	#endif
					m_boolPersonalList( false ),
					m_boolDisplayChanged( true ),
					m_boolLoadPending( false ),
					m_boolPartialShutdown( false ),
					m_pConnectingDlg( 0 ),
					m_boolWaitingForHostName( false ),
					m_worldCmdLine( pList )
{
											// Create the TinTin object
	m_pTinTin = new ChWorldTinTin( this );
											// Create the stream manager
	m_pWorldStreamMgr =
		new ChWorldStreamManager( this, GetModuleID() );
	ASSERT( m_pWorldStreamMgr );
											// Read the registry settings
	UpdatePreferences();

	RegisterDispatchers();
											// Create critical section object

	InitializeCriticalSection( &m_critsecDisconnect );
}


ChWorldMainInfo::~ChWorldMainInfo()
{
	#if !defined( CH_PUEBLO_PLUGIN )
		DestroyMenus();
	#endif
	
		ShutdownWorld( true, true );

	if (m_pWorldStreamMgr)
	{										// Delete the stream manager
		delete m_pWorldStreamMgr;
		m_pWorldStreamMgr = 0;
	}
											// Free the URL history items
	EmptyURLList();
											// Delete the Text output object
	if (m_pTextOutput)
	{
		delete m_pTextOutput;
		m_pTextOutput = 0;
	}
											// Delete the Text input object
	if (m_pTextInput)
	{
		delete m_pTextInput;
		m_pTextInput = 0;
	}
											// Delete the TinTin object
	if (m_pTinTin)
	{
		delete m_pTinTin;
		m_pTinTin = 0;
	}

	UnloadSoundModule();
	UnloadGraphicsModule();
											// Destroy critical section objects

	DeleteCriticalSection( &m_critsecDisconnect );
}


void ChWorldMainInfo::OnSecondTick( time_t timeCurr )
{
	if (IsConnected())
	{
		GetTinTin()->OnSecondTick( timeCurr );
	}
}


void ChWorldMainInfo::Initialize()
{											/* Create the text input and
												output objects */
	TRACE0("ChWorldMainInfo::Initialize()\n");
	m_pTextInput = new ChTextInput( this );
	m_pTextOutput = new ChTextOutput( this );
											/* Load the sound module as
												'optional' */
	TRACE0(" - init: call LoadSoundModule(true)\n");
	LoadSoundModule( true );
											// Create the menus
#if !defined( CH_PUEBLO_PLUGIN )
	CreateMenus();
#endif
											// Perform startup processing
	OnInitialStartup();
}


void ChWorldMainInfo::ShowModule( bool boolShow )
{
	ChShowModuleMsg	showMsg( boolShow );

	if (boolShow && !IsShown())
	{
		
		#if !defined( CH_PUEBLO_PLUGIN )
			InstallMenus();
		#endif

		SetShown( boolShow );
	}
	else if (!boolShow && IsShown())
	{
		#if !defined( CH_PUEBLO_PLUGIN )
				UninstallMenus();
		#endif

		m_boolShown = false;

		SetShown( boolShow );
	}
											// Hide or show the ChSound module
	if (GetSoundID())
	{
		NotifySound( showMsg );
	}
											// Hide or show the ChGraphx module
	if (GetGraphicsID())
	{
		NotifyGraphics( showMsg );
	}
}


void ChWorldMainInfo::DisplayWorldList()
{
	TRACE0("ChWorldMainInfo::DisplayWorldList()\n");
	if (DisplayChanged())
	{
		SetDisplayChanged( false );
											// Clear the text output window
		GetTextOutput()->Clear();

		if (m_urlList.IsEmpty())
		{									// Display the personal list

			DoJump( PERSONAL_URL, "", true, true, true );
		}
		else
		{									// Display the last page viewed
			ChString*		pstrTail;

			pstrTail = (ChString*)m_urlList.GetTail();
			DoJump( *pstrTail, "", true, false );
		}
	}
}


void ChWorldMainInfo::Send( const ChString& strText, bool boolEcho )
{
	GetWorldConnection()->SendWorldCommand( strText, boolEcho );
}


void ChWorldMainInfo::Send( const ChString& strDefaultCmd,
							const ChString& strMD5,
							const ChString& strOverrideCmd,
							const ChString& strParams,
							bool boolEcho )
{
	ChString		strCmd;

	if (!strMD5.IsEmpty() && !strOverrideCmd.IsEmpty())
	{
		if (VerifyMD5( strMD5 ))
		{
			strCmd = strOverrideCmd;
		}
	}

	if (strCmd.IsEmpty())
	{
		strCmd = strDefaultCmd;
	}

	strCmd += ' ' + strParams;

	Send( strCmd, boolEcho );
}


void ChWorldMainInfo::Display( const ChString& strText, bool boolPreformatted )
{
	ChString		strTextOut( strText );
	ChString		strOut;

	GetWorldConnection()->TurnHtmlOn( strOut );

	if (boolPreformatted)
	{
		strOut += "<pre>";
	}

	// UE: changed default colour from #000080 (which doesn't display very well
	//     against a black background).
	strOut += "<b><font text=\"#C0C000\">";

								/* Strip out HTML from the users'
									text and append it to the
									output buffer */

	ChHtmlWnd::EscapeForHTML( strTextOut );
	strOut += strTextOut;

	if (boolPreformatted)
	{
		strOut += "</pre>";
	}

	strOut += "</font></b><br>";

	GetWorldConnection()->TurnHtmlOff( strOut );
	GetTextOutput()->Add( strOut, false );
}


bool ChWorldMainInfo::DoCommand( const ChString& strCommand, chint32 lX,
									chint32 lY )
{
	bool		boolProcessed = true;
	ChCmdMsg	msg( strCommand, lX, lY );

	boolProcessed = DoHook( msg );

	if (!boolProcessed)
	{
		CommandType	command;
		ChString		strValue;
		ChString		strCoord;

		command = GetCommand( strCommand, strValue, false );

		if ((lX >= 0) && (lY >= 0))
		{										/* Append the coordinates to the
													command */
			char	buffer[80];

			sprintf( buffer, "?%ld,%ld", lX, lY );
			strCoord = buffer;
		}

		switch( command )
		{
			case xworld:
			{
				ChWorldInfo	info( strValue );

				if (info.IsValid())
				{
					Connect( info );
				}
				break;
			}

			case href:
			{
				ChString		strTarget;

				ChHtmlWnd::GetHTMLAttribute( strCommand, ATTR_TARGET, strTarget );

				if (IsTargetSelfOutput( strTarget ))
				{
					boolProcessed = DoJump( strValue, strCommand, false );
				}
				else
				{
					boolProcessed = false;
				}
				break;
			}

			case xcmd:
			{
				strValue += strCoord;
				DoXCmd( strValue );
				break;
			}

			default:
			{
				boolProcessed = false;
				break;
			}
		}
	}

	return boolProcessed;
}


bool ChWorldMainInfo::DoInline( const ChString& strArgs )
{
	bool		boolProcessed;
	ChInlineMsg	msg( strArgs );

	boolProcessed = DoHook( msg );

	if (!boolProcessed)
	{
		CommandType	command = invalid;
		ChString		strValue;

		if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCMD, strValue ))
		{
			command = xcmd;
		}
		else if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XMODE, strValue ))
		{
			command = xmode;
		}

		switch( command )
		{
	#if 0
												// Inline xch_cmd disabled for now
			case xcmd:
			{
				DoXCmd( strValue );
				boolProcessed = true;
				break;
			}
	#endif

			case xmode:
			{
				boolProcessed = DoXMode( strValue );
				break;
			}

			default:
			{
				boolProcessed = false;
				break;
			}
		}
	}

	return boolProcessed;
}


bool ChWorldMainInfo::DoHint( const ChString& strArgs )
{
	ChString	strHint( strArgs );
	bool	boolProcessed;

	if (!strHint.IsEmpty())
	{
		boolProcessed = FormatHint( strHint );
	}
	else
	{
		strHint = "";
		boolProcessed = true;
	}

	if (boolProcessed)
	{
		#if defined( CH_MSW )
		{
			ChPersistentFrame*	pFrame = GetCore()->GetFrameWnd();

			ASSERT( pFrame );
			pFrame->SetMessageText( strHint );
		}
		#elif defined( CH_UNIX )
		{
			TRACE1( "Need code to display hint: %s", (const char*)strHint );
		}
		#else
		{
			#error Platform not defined!
		}
		#endif
	}

	return boolProcessed;
}


void ChWorldMainInfo::DoAlert()
{
	if (GetSoundID())
	{
		ChMsg	doAlert( CH_MSG_SOUND_ALERT );

		NotifySound( doAlert );
	}
}


#if !defined( CH_PUEBLO_PLUGIN )
void ChWorldMainInfo::DoQuickConnect()
{
	ChQuickConnect		quickConnectDlg;
	int					iResult;

	iResult = quickConnectDlg.DoModal();
	if (IDOK == iResult)
	{
		ChWorldInfo	info( quickConnectDlg.GetHost(), "",
							quickConnectDlg.GetHost(),
							quickConnectDlg.GetPort(), otherType,
							quickConnectDlg.GetLoginType(), "", "", "" );

		Connect( info );
	}
}
#endif


bool ChWorldMainInfo::Connect( const ChWorldInfo& info )
{
//	bool		boolSuccess;
	ChString		strStatusFmt;
	ChString		strStatus;
	ChCore*		pCore = GetCore();
											// Reset the 'Pueblo Enhanced' flag
	m_boolPuebloEnhancedFound = false;
	m_iConnectID++;							// Update the connect ID

	#if defined( CH_MSW )
	{										// Put up a status message

		LOADSTRING( IDS_STATUS_CONNECTING_FMT, strStatusFmt );
		strStatus.Format( strStatusFmt, (const char*)info.GetName() );

		pCore->DisplayStatus( strStatus );
	}
	#endif	// defined( CH_MSW )
											// Connect...
	if (0 == m_pWorldConn)
	{
		m_pWorldConn = new ChWorldConn( GetModuleID(), worldSocketHandler,
										this, (chparam)this );
	}
											// Display the 'connecting...' dlg
	DisplayConnectDlg( info );
											// Store the world information
	m_pWorldInfo = new ChWorldInfo( info );
											/* Let the world connection class
												know about the current world */

	GetWorldConnection()->SetWorldInfo( m_pWorldInfo );

	m_boolWaitingForHostName = true;
	GetWorldConnection()->AsyncGetHostByName( info.GetAddr(),
												worldSocketAsyncHandler,
												(chparam)this );
	return true;
}


/*----------------------------------------------------------------------------
	ChWorldMainInfo::OnInitialStartup

	This method is called to initialize the ChWorld module, just after it
	is initialized.
----------------------------------------------------------------------------*/

void ChWorldMainInfo::OnInitialStartup()
{
	ChString			strWorldName;
	ChString			strWorldServer;
	chint16			sWorldPort;
	ChWorldType		type;
	ChLoginType		loginType;
	ChString			strUsername;
	ChString			strPassword;
	ChString			strHomePage;
	ChString			strBuffer;

											// Show the World module
	ShowModule( true );
											// Show the text output window
	GetTextOutput()->Show( true );

	EmptyURLList();

	#if !defined( CH_PUEBLO_PLUGIN )
	if (GetCmdLine().GetWorldServer( strWorldName, strWorldServer, sWorldPort, type,
								loginType, strUsername, strPassword ))
	{
		ChWorldInfo		info( strWorldName, "", strWorldServer, sWorldPort,
								type, loginType, strUsername, strPassword,
								strHomePage );

		if (info.IsValid())
		{
			Connect( info );
		}
	}
	else
	{
		DisplayWorldList();
	}
	#else
	if ( !GetCmdLine().GetHomePage().IsEmpty() )
	{

		DoJump( GetCmdLine().GetHomePage(), "", true, false, true );
	}
	else if (GetCmdLine().GetWorldServer( strWorldName, strWorldServer, sWorldPort, type,
								loginType, strUsername, strPassword ))
	{
		ChWorldInfo		info( strWorldName, "", strWorldServer, sWorldPort,
								type, loginType, strUsername, strPassword,
								strHomePage );

		if (info.IsValid())
		{
			Connect( info );
		}
	}
	#endif
}


void ChWorldMainInfo::OnPuebloEnhanced( const ChVersion& versEnhanced )
{
											/* Check to see if we've already
												seen a Pueblo Enhanced world
												this session */
	if (IsFirstPuebloEnhanced())
	{
		ChShowModuleMsg	showMsg;
		ChVersion		versionMD5( 1, 10 );
		ChString			strPuebloClientCmd;
		ChString			strClientVer;
		ChClientInfo	clientInfo( ChClientInfo::thisMachine );
		ChVersion		clientVer = clientInfo.GetClientVersion();

											/* Load the Sound and Graphics
												modules if they're not already
												loaded */
		if (0 == GetSoundID())
		{
			LoadSoundModule();
		}

		/*	// UE: Don't try to load the graphics module, since it hasn't been
				// 		 imported yet.
		if (0 == GetGraphicsID())
		{
			LoadGraphicsModule();
		}
		*/

		m_boolPuebloEnhancedFound = true;
											// Show the ChSound module
		if (GetSoundID())
		{
			NotifySound( showMsg );
		}
											// Show the ChGraphx module
		if (GetGraphicsID())
		{
			NotifyGraphics( showMsg );
		}
											/* Format the command to tell the
												world that we're a Pueblo
												client */

		strClientVer = clientVer.Format( ChVersion::formatShort );

		if (versEnhanced < versionMD5)
		{
			strPuebloClientCmd = PUEBLO_ENHANCED_COMMAND " " + strClientVer;
		}
		else
		{
			ChString		strPuebloClientFmt;

			CreateMD5Checksum();
											/* Format the new-style command to
												tell the world we're a Pueblo
												client */

			LOADSTRING( IDS_PUEBLO_CLIENT_110_FMT, strPuebloClientFmt );
			strPuebloClientCmd.Format( strPuebloClientFmt,
										(const char*)strClientVer,
										(const char*)GetMD5() );

			if (versEnhanced > versionMD5)
			{
											/* Newest version... add the
												module parameters to the
												PUEBLOCLIENT command */

				if (!m_strPuebloClientParams.IsEmpty())
				{
					strPuebloClientCmd += " " + m_strPuebloClientParams;
				}
			}
		}

		GetWorldConnection()->SendWorldCommand( strPuebloClientCmd, false );

											/* Send a message to the server to
												tell it that this is a Pueblo
												Enhanced world */

		TrackUsage( ChWorldMainInfo::worldEnhanced );
	}
}


void ChWorldMainInfo::DoAutoLogin()
{
	ChWorldInfo*	pWorldInfo = GetWorldInfo();

	if (pWorldInfo)
	{
		ChWorldType		type = pWorldInfo->GetType();
		ChString			strUsername;
		ChString			strPassword;

		strUsername = pWorldInfo->GetUsername();
		strPassword = pWorldInfo->GetPassword();

		if (type.IsValidType() && strUsername.GetLength())
		{
			if (connectLogin == pWorldInfo->GetLoginType())
			{
											/* Login consists of the form
												'connect username password' */
				if (strPassword.GetLength())
				{
					ChString		strLoginFormat;
					ChString		strLogin;
											// Format the login string

					LOADSTRING( IDS_MUSH_LOGIN_FORMAT, strLoginFormat );
					strLogin.Format( strLoginFormat, (const char*)strUsername,
										(const char*)strPassword );

											// Send the login string

					GetWorldConnection()->SendWorldCommand( strLogin, false );
				}
			}
			else
			{								/* Login is on two separate lines.
												First send the username. */

				GetWorldConnection()->SendWorldCommand( strUsername,
															false );

				if (strPassword.GetLength())
				{							// Next send the password

					GetWorldConnection()->SendWorldCommand( strPassword,
															false );
				}
			}

			GetTextInput()->SetLoginRecognized();
		}
	}
}


void ChWorldMainInfo::ShutdownWorld( bool boolShutdownMessage,
										bool boolEntirely )
{
	TRACE2("ChWorldMainInfo::ShutdownWorld(%s, %s)\n",
				 boolShutdownMessage ? "true" : "false",
				 boolEntirely ? "true" : "false");
	EnterCriticalSection( &m_critsecDisconnect );

	if (IsConnected())
	{
		ChString		strOffline;
		ChString		strText;
		ChString		strDisconnectMsg;
	
		#if !defined( CH_PUEBLO_PLUGIN )
				ChMenuItem*	pItem;
		#endif
		
		#if !defined( CH_PUEBLO_PLUGIN )
											// Clear world info
		GetCmdLine().SetWorld( "" );

		LOADSTRING( IDS_OFFLINE, strOffline );
		GetCore()->UpdateSessionPane( strOffline, false );
		
		#endif //  !defined( CH_PUEBLO_PLUGIN )


		if (boolShutdownMessage && m_boolPauseOnDisconnect && IsShown())
		{
			ChWorldInfo*	pWorldInfo = GetWorldInfo();
			ChString			strFormat;
			ChString			strName;

			ASSERT( pWorldInfo );
											// Format the disconnect message
			strName = pWorldInfo->GetName();
			if (0 == strName.GetLength())
			{
				strName = pWorldInfo->GetHost();
			}

			LOADSTRING( IDS_FMT_DISCONNECT_MSG, strFormat );
			strDisconnectMsg.Format( (const char*)strFormat,
										(const char*)strName );
		}
											/* Hide the graphics window if
												it is shown */
		if (GetGraphicsID())
		{
			ChShowModuleMsg		hideMsg( false );
			ChResetMsg			resetMsg;

			NotifyGraphics( hideMsg );
			NotifyGraphics( resetMsg );
		}
											// Stop any sounds that are playing
		if (GetSoundID())
		{
			ChMediaStopMsg	stopMsg;

			NotifySound( stopMsg );
		}

		if (GetWorldConnection()->IsConnected() && GetWorldInfo())
		{
											// Send a statistics event
			TrackUsage( worldDisconnect );
		}

		GetTextOutput()->GetOutputWnd()->UpdateWindow();

		if (!strDisconnectMsg.IsEmpty())
		{									// Display the disconnect message
			if (!m_boolPauseInline) {
				GetCore()->GetFrameWnd()->
								MessageBox( strDisconnectMsg, "Pueblo",
											MB_OK | MB_ICONINFORMATION );
			} else {
				ChString strFormat, strTemp, strHtmlMsg;

				LOADSTRING( IDS_FMT_INLINE_HEAD_MSG, strFormat );
				GetWorldConnection()->TurnHtmlOn(strTemp);
				ChHtmlWnd::EscapeForHTML( strDisconnectMsg );
				strHtmlMsg = strDisconnectMsg;
				strHtmlMsg.Format( (const char*)strFormat,
										(const char*)strDisconnectMsg );
				strTemp += strHtmlMsg;

				LOADSTRING( IDS_INLINE_RETURN_MSG, strHtmlMsg );
				strTemp += strHtmlMsg;
				LOADSTRING( IDS_INLINE_RECONNECT_MSG, strHtmlMsg );
				strTemp += strHtmlMsg;
				LOADSTRING( IDS_INLINE_FOOT_MSG, strHtmlMsg );
				strTemp += strHtmlMsg;
				
				GetTextOutput()->Add(strTemp, false);
				m_boolPartialShutdown = true;
			}
		}
											// Disconnect from the world
		Disconnect();
											/* Set focus back to the text
												output window */
		GetTextOutput()->SetFocus();
											// Reset the Text Output window
		if (GetTextOutput())
		{
			GetTextOutput()->Reset();
		}
											/* Hide and unhook the 'text in'
												module */
		GetTextInput()->Show( false );

#if !defined( CH_PUEBLO_PLUGIN )
		if (m_boolMenus)
		{									// Disable the 'add' command
			LOADSTRING( IDS_MENU_ADD, strText );
			if (pItem = GetWorldMenu()->FindItem( strText ))
			{
				pItem->Enable( false );
			}
											// Disable the disconnect command

			LOADSTRING( IDS_MENU_WORLD_DISCON, strText );
			if (pItem = GetWorldMenu()->FindItem( strText ))
			{
				pItem->Enable( false );
			}
		}
#endif
											// Clear the status message
		GetCore()->DisplayStatus( "" );

		if (!m_boolPartialShutdown) {
			m_boolPartialShutdown = true;
			CompletePartialShutdown(boolEntirely);
		}
	}

	ASSERT( !IsConnected() );

	LeaveCriticalSection( &m_critsecDisconnect );
}


void ChWorldMainInfo::CompletePartialShutdown(bool boolEntirely) {
	TRACE0("ChWorldMainInfo::CompletePartialShutdown()\n");

	if (m_boolPartialShutdown) {
		m_boolPartialShutdown = false;
#if !defined( CH_PUEBLO_PLUGIN )
											// Clear the frame title
		GetCore()->SetFrameTitle( "" );

											// Delete the world info
		if (0 != GetWorldInfo())
		{
			delete m_pWorldInfo;
			m_pWorldInfo = 0;
		}

		if (!boolEntirely)
		{									/* Clear the page history and
												restart the world manager */
			DisplayWorldList();
		}
#else
		if ( !boolEntirely )
		{
			if ( !GetCmdLine().GetOnDisconnectURL().IsEmpty() )
			{
				GetCore()->GetHTTPConn()->GetURL( 
						GetCmdLine().GetOnDisconnectURL(), 
						TEXT( "_current" ),	0 );

			}
			else if ( !GetCmdLine().GetHomePage().IsEmpty() )
			{

												// Clear the text output window
					GetTextOutput()->Clear();

					DoJump( GetCmdLine().GetHomePage(), "", true, false, true );
			}
			else 
			{

				DisplayWorldList();
			}
		}
#endif
	}
}

void ChWorldMainInfo::AddCurrentWorld()
{
	ChCore*			pCore = GetCore();
	ChWorldList		worldList;

	ASSERT( GetWorldInfo() );

	worldList.Add( *GetWorldInfo() );
	worldList.Store();

	#if defined( CH_MSW )
	{										// Put up a status message
		ChString		strFormat;
		ChString		strStatus;

		LOADSTRING( IDS_STATUS_WORLD_ADDED, strFormat );
		strStatus.Format( strFormat, (const char*)GetWorldInfo()->GetName() );

		pCore->DisplayStatus( strStatus );
	}
	#endif	// defined( CH_MSW )
}


void ChWorldMainInfo::CreateShortcut()
{
	ChWorldInfo		info( ChString("") );

	info.CreateShortcut( GetCore() );
}


void ChWorldMainInfo::CreateCurrentWorldShortcut()
{
	ASSERT( GetWorldInfo() );

	GetWorldInfo()->CreateShortcut( GetCore() );
}


bool ChWorldMainInfo::GetPersonalWorldList()
{
	ChWorldList		worldList;
	ChPosition		pos;
	ChString			strText;
	bool			boolSuccess;
	ChString			strFormat;

	SetPersonalList();
											// Hide & clear the window
	GetTextOutput()->Show( false );
	GetTextOutput()->Clear();

	SetDisplayChanged( false );
											// Display the title

	LOADSTRING( IDS_PERSONAL_LIST_TITLE, strText );
	GetTextOutput()->Add( strText );

	/*		// UE: kill this damn prefetch, it causes nothing but trouble.
	if (!GetHomePage().IsEmpty())
	{
		LOADSTRING( IDS_PERSONAL_LIST_PREFETCH, strFormat );
		strText.Format( strFormat, (const char*)GetHomePage() );
		GetTextOutput()->Add( strText );
	}
	*/
											/* Loop through items in world
												list, adding one at a time */
	pos = worldList.GetHead();

	if (pos)
	{
		ChString		strHomePagePrefix;
		ChString		strHomePageSuffix;

		boolSuccess = true;

		LOADSTRING( IDS_PERSONAL_LIST_HDR, strText );
		GetTextOutput()->Add( strText );

		AddChacoListJump();

		GetTextOutput()->Add( "<ul>" );

		LOADSTRING( IDS_PERSONAL_LIST_HOMEPAGE_PREFIX, strHomePagePrefix );
		LOADSTRING( IDS_PERSONAL_LIST_HOMEPAGE_SUFFIX, strHomePageSuffix );

		while( pos )
		{
			ChWorldInfo*	pInfo = worldList.GetData( pos );
			ChString			strWorld;
			ChString			strWorldEscaped("This is the \"Real\" thing!");

			strText = "<li><a " ATTR_XWORLD "=\"";

			pInfo->Stringize( strWorld );
			ChUtil::EncodeAttributeString( strWorld, strWorldEscaped );
			strText += strWorldEscaped;

			strText += "\">";

			if (pInfo->GetName().GetLength() > 0)
			{
				strText += pInfo->GetName();
			}
			else
			{
				strText += pInfo->GetAddr();
			}

			strText += "</a>";

			if (!pInfo->GetUsername().IsEmpty())
			{
				strText += " - " + pInfo->GetUsername();
			}

			if (!pInfo->GetHomePage().IsEmpty())
			{
				strText += " " + strHomePagePrefix + pInfo->GetHomePage() +
									strHomePageSuffix;
			}

			if (pInfo->GetDesc().GetLength() > 0)
			{
				strText += "<p>";
				strText += pInfo->GetDesc();
				strText += "</p>";
			}

			GetTextOutput()->Add( strText );

			worldList.GetNext( pos );
		}

		GetTextOutput()->Add( "</ul>" );
	}
	else
	{
		boolSuccess = false;

		LOADSTRING( IDS_PERSONAL_LIST_EMPTY, strText );
		GetTextOutput()->Add( strText );

		AddChacoListJump();
	}

	GetTextOutput()->Show();

	return boolSuccess;
}


void ChWorldMainInfo::EditPersonalWorldList()
{
	#if defined( CH_MSW )
	{										// Load and execute the dialog

		ChWorldListDlg	worldListDlg( GetCore(), IsConnected());
		chint16			sResult;

		m_pTextOutput->GetOutputWnd()->EnableWindow(false);
		sResult = worldListDlg.DoModal();
		m_pTextOutput->GetOutputWnd()->EnableWindow(true);
		switch( sResult )
		{
			case IDOK:
			{
				break;
			}

			case IDC_LIST_CONNECT:
			{
				ChWorldInfo	info( worldListDlg.GetName(),
									worldListDlg.GetDesc(),
									worldListDlg.GetHost(),
									worldListDlg.GetPort(),
									worldListDlg.GetType(),
									worldListDlg.GetLoginType(),
									worldListDlg.GetUsername(),
									worldListDlg.GetPassword(),
									worldListDlg.GetHomePage() );

				Connect( info );
				break;
			}

			case IDCANCEL:
			{								// Do nothing
				break;
			}
		}

		if (IsPersonalList() && (sResult != IDCANCEL) && !IsConnected())
		{
											/* Redisplay the personal list as
												it may have changed */
			GetPersonalWorldList();
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "XXX: " << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )
}


void ChWorldMainInfo::DoPreviousURL()
{
	ChString*		pstrTail;

	ASSERT( m_urlList.GetCount() > 0 );
											/* Remove the tail URL (which is
												the current one) */
	pstrTail = (ChString*)m_urlList.GetTail();
	m_urlList.RemoveTail();
											// Free the item
	delete pstrTail;

	ASSERT( m_urlList.GetCount() > 0 );
											/* Make the last URL the new bottom
												most page */
	pstrTail = (ChString*)m_urlList.GetTail();
	DoJump( *pstrTail, "", true, false );
}


void ChWorldMainInfo::SetEchoPrefs( bool boolEcho, bool boolBold,
									bool boolItalic )
{
	ChWorldConn*	pConn = GetWorldConnection();

	if (pConn)
	{
		pConn->SetEchoPrefs( boolEcho, boolBold, boolItalic );
	}
}


bool ChWorldMainInfo::LookForPuebloEnhanced( const ChString& strLine,
												ChVersion& versEnhanced )
{
											/* We want to find the first string
												followed by a version and then
												the second string */

	const char*		pstrFirstPart = PUEBLO_ENHANCED_PART_1;
	const char*		pstrSecondPart = PUEBLO_ENHANCED_PART_2;

	ChString			strSearch( strLine );
	int				iFirst;
	bool			boolFound = false;

											// Search case-insensitive
	strSearch.MakeLower();
	do
	{										// Look for the first string

		iFirst = strSearch.Find( pstrFirstPart );
		if (-1 != iFirst)
		{
			int		iSecond;
			ChString	strRest;
											/* First part found, so look for
												the second part */

			strRest = strSearch.Mid( iFirst + strlen( pstrFirstPart ) );
			iSecond = strRest.Find( pstrSecondPart );

			if (-1 != iSecond)
			{								/* Second part found, look between
												them for the version number */
				const char*	pstrScan;
				chint16		sMajor = 0;
				chint16		sMinor = 0;
				int			iDigits = 0;

				strRest = strRest.Left( iSecond );

                #if defined( CH_ARCH_16 )
				{
					TrimLeft( strRest );
					TrimRight( strRest );
				}
				#else	// defined( CH_ARCH_16 )
				{
					strRest.TrimLeft();
					strRest.TrimRight();
				}
				#endif	// defined( CH_ARCH_16 )

											// Look for digits in form #.#
				pstrScan = strRest;

				while (isdigit( *pstrScan ))
				{
					sMajor = (sMajor * 10) + (*pstrScan - '0');
					pstrScan++;
					iDigits++;
				}

				if (iDigits)
				{							/* There must be at least one
												digit in the major version
												number */
					if ('.' == *pstrScan)
					{						/* Found the decimal... Now get
												the second part of the
												version */
						pstrScan++;
						while (isdigit( *pstrScan ))
						{
							sMinor = (sMinor * 10) + (*pstrScan - '0');
							pstrScan++;
						}
											/* Purity check -- make sure these
												minor digits are the last
												things we find */
						if (0 == *pstrScan)
						{					// Yes!

							versEnhanced = ChVersion( sMajor, sMinor );
							boolFound = true;
						}
					}
					else if (0 == *pstrScan)
					{						// Only major version number found

						versEnhanced = ChVersion( sMajor, 0 );
						boolFound = true;
					}
											// Illegal version format... ignore
				}
			}

			if ((-1 != iFirst) && !boolFound)
			{								/* We found the wrong first string.
												Start again after this
												string. */

				strSearch = strSearch.Mid( iFirst + strlen( pstrFirstPart ) );
			}
		}
	}
	while ((-1 != iFirst) && !boolFound);

	if (boolFound)
	{										/* Perform out Pueblo Enhanced
												processing */
		OnPuebloEnhanced( versEnhanced );
	}

	return boolFound;
}


void ChWorldMainInfo::UpdatePreferences()
{
	ChRegistry		worldPrefsReg( WORLD_PREFS_GROUP );

											// Read values from registry

	worldPrefsReg.ReadBool( WORLD_PREFS_PAUSE_DISCONNECT,
							m_boolPauseOnDisconnect, true );
	worldPrefsReg.ReadBool( WORLD_PREFS_PAUSE_INLINE,
							m_boolPauseInline, true );
	worldPrefsReg.ReadBool( WORLD_PREFS_NOTIFY, m_boolNotify, true );
	worldPrefsReg.ReadBool( WORLD_PREFS_NOTIFY_ALERT, m_boolNotifyAlert,
							false );
	worldPrefsReg.Read( WORLD_PREFS_NOTIFY_STR, m_strNotifyMatch, "" );

	if (GetTextInput())
	{
		GetTextInput()->UpdatePreferences();
	}

	if (GetWorldConnection())
	{
		GetWorldConnection()->UpdatePreferences();
	}
}


void ChWorldMainInfo::SetFocusTarget( FocusTarget target,
										bool boolGainingFocus )
{
	if ((GetFocusTarget() == target) && !boolGainingFocus)
	{
											// We're losing the focus
		m_focusTarget = focusNone;

#if !defined( CH_PUEBLO_PLUGIN )
		if (m_boolMenusInstalled)
		{									// Demote ourselves
			GetEditMenu()->Promote( false );
		}
#endif
	}
	else if (boolGainingFocus)
	{										// New target has the focus
		ASSERT( focusNone != target );

		m_focusTarget = target;

#if !defined( CH_PUEBLO_PLUGIN )
		if (m_boolMenusInstalled)
		{									/* Promote ourselves (we want to
												get the Cut/Copy/Paste menu
												messages first) */
			GetEditMenu()->Promote();
		}
#endif
	}
}


bool ChWorldMainInfo::WantTextLines()
{
	bool	boolWantLines;

	boolWantLines = GetCore()->GetFrameWnd()->IsIconic() &&
					(GetNotify() || GetNotifyAlert());

	boolWantLines = boolWantLines ||
					(GetTinTin()->IsActionsDefined() &&
						!GetTinTin()->IsIgnore());

	return boolWantLines;
}


void ChWorldMainInfo::OnTextLine( const ChString& strLine )
{
											/* If the user wants notification,
												then check to see if it's
												appropriate */
	if (GetNotify() || GetNotifyAlert())
	{
		LookForNotify( strLine );
	}

	if (!GetTinTin()->IsIgnore())
	{
		GetTinTin()->CheckActions( strLine );
	}
}


void ChWorldMainInfo::LookForNotify( const ChString& strLine ) const
{
	ChCore*		pCore = GetCore();
	bool		boolIconic;

	#if defined( CH_MSW )
	{
		boolIconic = (pCore->GetFrameWnd()->IsIconic() != FALSE);
	}
	#else	// defined( CH_MSW )
	{
		TRACE( "ChWorldMainInfo::LookForNotify : "
				"Need to confirm if app is iconic" );
		boolIconic = false;
	}
	#endif	// defined( CH_MSW )

	if (boolIconic && !pCore->IsFlashWindow())
	{
											/* Do this only if we haven't
												already notified the user */
		bool		boolMatch;
		ChString		strMatch = GetNotifyMatch();

		if (strMatch.IsEmpty())
		{
			boolMatch = true;
		}
		else
		{
			ChString		strTest( strLine );

			strTest.MakeLower();
			boolMatch = (strTest.Find( strMatch ) >= 0);
		}

		if (boolMatch)
		{
			if (GetNotify())
			{
				pCore->EnableFlashWindow();
			}

			if (GetNotifyAlert())
			{
				if (GetSoundID())
				{
					ChMsg	doAlert( CH_MSG_SOUND_ALERT );

					NotifySound( doAlert );
				}
			}
		}
	}
}


void ChWorldMainInfo::SetEchoState( EchoState newState, bool boolPreserve )
{
	EchoState	oldEchoState = GetEchoState();

	if (oldEchoState != newState)
	{
		m_echoState = newState;

		GetTextInput()->SetEcho( (echoOn == m_echoState), boolPreserve );
	}
}


/*----------------------------------------------------------------------------
	ChWorldMainInfo protected methods
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	ChWorldMainInfo::OnWorldConnect

	This method is called after the first data is recieved from the
	destination world, confirming the connection is two-way.
----------------------------------------------------------------------------*/

void ChWorldMainInfo::OnWorldConnect()
{
	ChString			strText;
	ChString			strStatusFmt;
	ChString			strStatus;
	ChString			strOnLine;
	ChWorldInfo*	pWorldInfo = GetWorldInfo();

#if !defined( CH_PUEBLO_PLUGIN )
	ChMenuItem*		pItem;
#endif

	ASSERT( 0 != pWorldInfo );
											// Turn on echo
	SetEchoState( echoOn );
											// Show the 'text in' window
	GetTextInput()->Show();
	GetTextInput()->Clear();
	TRACE("(input window cleared)\n");
											/* Clear the text out window
												and set it to auto-scroll.
												Also set the default buffer
												limit. */
	GetTextOutput()->Clear();
	GetTextOutput()->SetAutoScroll();
	GetTextOutput()->SetBufferLimit();

	SetDisplayChanged();
											// Enable the disconnect command

#if !defined( CH_PUEBLO_PLUGIN )
	LOADSTRING( IDS_MENU_WORLD_DISCON, strText );
	if (pItem = GetWorldMenu()->FindItem( strText ))
	{
		pItem->Enable();
	}
											// Set the frame title

	GetCore()->SetFrameTitle( pWorldInfo->GetName() );
 #endif

											/* Store the world name & type in
												the cmd line info */

	GetCmdLine().SetWorld( pWorldInfo->GetName(), &pWorldInfo->GetType() );

											// Reset the TinTin processor
	GetTinTin()->Reset();
											// Start on line time tracking
#if !defined( CH_PUEBLO_PLUGIN )
	LOADSTRING( IDS_ONLINE, strOnLine );
	GetCore()->UpdateSessionPane( strOnLine, true );
#endif

											// Init the ChWorldConn
	GetWorldConnection()->InitConnection();
											/* Send connect information to
												the server */
	TrackUsage( worldConnect );

#if !defined( CH_PUEBLO_PLUGIN )
											// Enable the 'add' command
	LOADSTRING( IDS_MENU_ADD, strText );
	if (pItem = GetWorldMenu()->FindItem( strText ))
	{
		pItem->Enable();
	}
#endif

	ASSERT( 0 != GetWorldInfo() );

	#if defined( CH_MSW )
	{										/* Put up the connection
												confirmation status message */

		LOADSTRING( IDS_STATUS_CONNECTED_FMT, strStatusFmt );
		strStatus.Format( strStatusFmt, (const char*)GetWorldInfo()->GetName() );

		GetCore()->DisplayStatus( strStatus );
	}
	#endif	// defined( CH_MSW )
											/* Gather information from each
												of our controlled modules
												about what they wish to add to
												the PUEBLOCLIENT command
												line... */
	SendConnectedMsg();
}


void ChWorldMainInfo::OnInvalidWorld( const ChString& strReason )
{
											// Shutdown the world
	ShutdownWorld( false );
											/* Display the reason for the
												disconnect */
	#if defined( CH_MSW )
	{
		GetCore()->GetFrameWnd()->MessageBox( (const char*)strReason,  0,
						MB_OK | MB_ICONINFORMATION );
	}
	#elif defined( CH_UNIX )
	{
		TRACE( "ChWorldMainInfo::OnInvalidWorld : Need message box" );
	}
	#else
	{
		#error "Undefined platform";
	}
	#endif
}


void ChWorldMainInfo::OnAsyncSocketAddress( int iError, chuint32 luAddress )
{
	m_boolWaitingForHostName = false;
											/* (The address is in host byte
												order) */
	if (0 == iError)
	{
		ChString		strFormat;
		ChString		strMessage;
		ChString		strName;

		ASSERT( GetWorldInfo() );
		strName = GetWorldInfo()->GetName();
		if (0 == strName.GetLength())
		{
			strName = GetWorldInfo()->GetHost();
		}
											// Format and change the message

		LOADSTRING( IDS_CONNECT_STATUS_MSG, strFormat );
		strMessage.Format( strFormat, (const char*)strName,
							GetWorldInfo()->GetPort() );

		if (m_pConnectingDlg)
		{
			m_pConnectingDlg->ChangeMessage( strMessage );
		}
											// Perform the async connection
		try
		{
			GetWorldConnection()->Connect( luAddress,
											GetWorldInfo()->GetPort() );
		}
		catch( ChSocketEx socketEx )
		{
			ShutdownWorld( false );
		}
	}
	else
	{
		ChString		strFormat;
		ChString		strErrMessage;

		switch( iError )
		{
			case WSAENETDOWN:
			{
				LOADSTRING( IDS_ERR_DNS_NETDOWN, strFormat );
				break;
			}

			case WSAHOST_NOT_FOUND:
			default:
			{
				LOADSTRING( IDS_ERR_DNS_LOOKUP, strFormat );
				break;
			}
		}

		if (!strFormat.IsEmpty())
		{
			strErrMessage.Format( strFormat,
									(const char*)GetWorldInfo()->GetHost() );
		}

		CloseConnectDlg();

		if (!strErrMessage.IsEmpty())
		{
			#if defined( CH_MSW )
			{
				GetCore()->GetFrameWnd()->
								MessageBox( strErrMessage, 0,
											MB_ICONEXCLAMATION | MB_OK );
			}
			#elif defined( CH_UNIX )
			{
				TRACE( strErrMessage );
			}
			#else
			{
				#error "Undefined platform";
			}
			#endif
		}

		ShutdownWorld( false );
	}
}


void ChWorldMainInfo::OnConnectComplete( int iErrorCode )
{
	ChString		strErrMessage;

	GetCore()->DisplayStatus( "" );

	CloseConnectDlg();

	ASSERT( 0 != GetWorldInfo() );

	if (0 == iErrorCode)
	{
		//ASSERT( 0 != GetWorldInfo() );

		OnWorldConnect();
	}
	else
	{
		ChString		strName( GetWorldInfo()->GetName() );
		bool		boolShutdownWorld = true;

		if (m_boolWaitingForHostName)
		{
			m_boolWaitingForHostName = false;
		}
		else
		{									/* We are in the connect phase, connect
												 will throw an exception on failure 
												 and we will shutdown the world when 
												 we catch the exception.*/
			boolShutdownWorld = false;
		}

		if (strName.IsEmpty())
		{
			strName = GetWorldInfo()->GetHost();
		}

		switch ( iErrorCode )
		{
			case WSAEDISCON:
			{								/* Cancel the blocking request and
												terminate the connection */
				m_pWorldConn->cancelblocking();
				break;
			}
			case WSAEINTR :
			{
				break;
			}
			case WSAEADDRNOTAVAIL:
			{
				ChString		strFormat;

				LOADSTRING( IDS_HOST_NAME_ERROR, strFormat );
				strErrMessage.Format( strFormat, (const char*)strName );
				break;
			}

			case WSAECONNREFUSED:
			case WSAETIMEDOUT:
			default:
			{
				ChString		strFormat;

				LOADSTRING( IDS_CONNECT_ERROR, strFormat );
				strErrMessage.Format( strFormat, (const char*)strName );
				break;
			}
		}

		#if defined( CH_MSW )
		{
			if (!strErrMessage.IsEmpty())
			{
				GetCore()->Trace( strErrMessage, ChCore::traceErrors );
				GetCore()->GetFrameWnd()->MessageBox( strErrMessage );
			}
		}
		#endif	// defined( CH_MSW )

		if (boolShutdownWorld)
		{
			ShutdownWorld( false );
		}
	}
}


bool ChWorldMainInfo::DoJump( const ChString& strURL, const ChString& strHTML,
								bool boolForceLoad, bool boolAddToHistory,
								bool boolCritical )
{
	bool	boolProcessed;

	TRACE2("ChWorldMainInfo::DoJump(\"%s\", \"%s\", ...)\n", (LPCSTR)strURL,
				(LPCSTR)strHTML);

	if (boolAddToHistory)
	{										// Save the URL in the list
		AddURLToList( strURL );
	}
											// Load the page
	if ((0 == strURL.Compare( DISCONNECT_URL )) || (0 == strURL.Compare( RECONNECT_URL ))) {
		m_boolLoadPending = false;
		boolProcessed = true;

		if (m_boolPartialShutdown) {
			if (GetWorldInfo() && (0 == strURL.Compare( RECONNECT_URL ))) {
				// Save local copy of the world info
				ChWorldInfo info(*GetWorldInfo());
				CompletePartialShutdown();
				// Reconnect using the saved info
				Connect(info);
			} else {
				CompletePartialShutdown();
			}
		} else {
			TRACE0("World attempted to re/disconnect when not allowed.\n");
		}
	} else if ((0 == strURL.Compare( PERSONAL_URL )) ||
		(0 == strURL.Compare( PERSONAL_URL_OLD )) ||
		(0 == strURL.Compare( PERSONAL_URL_OLD2 )))
	{
		m_boolLoadPending = false;
		boolProcessed = true;

		GetPersonalWorldList();
	}
	else if (0 == strURL.Compare( WORLD_LIST_EDIT_URL ))
	{
		m_boolLoadPending = false;
		boolProcessed = true;

		EditPersonalWorldList();
	}
	else
	{
		ChString	strWorkingURL( strURL );

		if (0 == strWorkingURL.Compare( WORLD_LIST_URL ))
		{
			strWorkingURL = GetHomePage();
			boolForceLoad = true;
		}

		if (boolForceLoad)
		{
			SetPersonalList( false );
			m_boolLoadPending = true;
			GetTextOutput()->Clear();

			ChURLParts urlParts;

			urlParts.GetURLParts( strWorkingURL, GetCurrentURL() );

			GetTextOutput()->LoadTextOutURL( urlParts.GetURL() );

			boolProcessed = true;
		}
		else
		{
			m_boolLoadPending = false;

			boolProcessed = false;
		}
	}

	return boolProcessed;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChWorldMainInfo::DoXCmd

	strCommand	||	Arguments to the 'xch_cmd=' HTML attribute.

------------------------------------------------------------------------------

	This method will process inline HTML for the 'xch_cmd=' attribute.

----------------------------------------------------------------------------*/

void ChWorldMainInfo::DoXCmd( const ChString& strCommand )
{
	if (!strCommand.IsEmpty())
	{
						/* Run this command by TinTin to
							figure out if it is a direction
							for the path */
		GetTinTin()->CheckInsertPath( strCommand );

						// Now send it directly to the world
		if(GetWorldConnection() == NULL)
		{
			ChString strErrMessage;
			LOADSTRING( IDS_CMD_NOT_CONNECTED, strErrMessage );

			GetCore()->GetFrameWnd()->MessageBox( strErrMessage );
		}
		else
		{
			GetWorldConnection()->SendWorldCommand( strCommand, false );
		}
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChWorldMainInfo::DoXMode

	strArgs		||	Arguments to the 'xmode=' HTML attribute.

------------------------------------------------------------------------------

	This method will process the xmode inline command.  The valid values are
	either 'HTML' or 'TEXT'.

----------------------------------------------------------------------------*/

bool ChWorldMainInfo::DoXMode( ChString& strArgs )
{
	bool	boolProcessed = true;

	strArgs.MakeLower();

	if (strArgs == ATTR_XMODE_HTML)
	{
		GetWorldConnection()->SetMode( modeHtml );
	}
	else if (strArgs == ATTR_XMODE_PURE_HTML)
	{
		GetWorldConnection()->SetMode( modePureHtml );
	}
	else if (strArgs == ATTR_XMODE_TEXT)
	{
		GetWorldConnection()->SetMode( modeText );
	}
	else
	{
		boolProcessed = false;
	}

	return boolProcessed;
}


void ChWorldMainInfo::NotifyCore( ChMsg& msg ) const
{
	GetCore()->DispatchMsg( CH_CORE_MODULE_ID, msg );
}

void ChWorldMainInfo::NotifySound( ChMsg& msg ) const
{
	if ( m_idSoundModule )
	{
		GetCore()->DispatchMsg( m_idSoundModule, msg );
	}
}
void ChWorldMainInfo::NotifyGraphics( ChMsg& msg )  const
{
	if ( m_idGraphicsModule )
	{
		GetCore()->DispatchMsg( m_idGraphicsModule, msg );
	}
}

void ChWorldMainInfo::SetSoundID( const ChModuleID& idModule )
{
	m_idSoundModule = idModule;

	if (GetSoundID())
	{
		ChShowModuleMsg		showMsg;
											// Show the ChSound module
		NotifySound( showMsg );
	}
}


void ChWorldMainInfo::SetGraphicsID( const ChModuleID& idModule )
{
	m_idGraphicsModule = idModule;

	if (GetGraphicsID())
	{
		ChShowModuleMsg		showMsg;
		ChInstallHookMsg	hookCmdMsg( GetModuleID(), CH_MSG_CMD );
		ChInstallHookMsg	hookHintMsg( GetModuleID(), CH_MSG_HINT );
		ChInstallHookMsg	hookInlineMsg( GetModuleID(), CH_MSG_INLINE );

											// Show the ChGraphx module
		NotifyGraphics( showMsg );
											/* Hook the graphics module
												so we get hot spot
												notifications */
		NotifyGraphics( hookCmdMsg );
		NotifyGraphics( hookHintMsg );
		NotifyGraphics( hookInlineMsg );
	}
}


#if !defined( CH_PUEBLO_PLUGIN )
bool ChWorldMainInfo::CheckEditMenuItem( EditMenuItem item )
{
	bool	boolEnable = true;

	switch( GetFocusTarget() )
	{
		case focusTextInput:
		{
			boolEnable = GetTextInput()->CheckEditMenuItem( item );
			break;
		}

		case focusTextOutput:
		{
			boolEnable = GetTextOutput()->CheckEditMenuItem( item );
			break;
		}

		case focusNone:
		default:
		{
			boolEnable = false;
			break;
		}
	}

	return boolEnable;
}


void ChWorldMainInfo::DoEditMenuItem( EditMenuItem item )
{
	switch( GetFocusTarget() )
	{
		case focusTextInput:
		{
			GetTextInput()->DoEditMenuItem( item );
			break;
		}

		case focusTextOutput:
		{
			GetTextOutput()->DoEditMenuItem( item );
			break;
		}

		case focusNone:
		default:
		{
			break;
		}
	}
}

#endif //  !defined( CH_PUEBLO_PLUGIN )


void ChWorldMainInfo::AddChacoListJump()
{
	ChString		strText;
	ChString		strTemp;
	ChString		strFormat;
	ChString		strAppPath;
	ChString		strAppPathURL;

	strText = "<hr><dl>";
											// Get the file:// path of the dir
	ChUtil::GetAppDirectory( strAppPath );
	strAppPath += "img";

	ChURLParts::MapHostFileToURL( strAppPath, strAppPathURL );

											// Add the options

	LOADSTRING( IDS_PERSONAL_LIST_JUMP1, strFormat );
	strTemp.Format( strFormat, (const char*)strAppPathURL );
	strText += strTemp;

	LOADSTRING( IDS_PERSONAL_LIST_JUMP2, strFormat );
	strTemp.Format( strFormat, (const char*)strAppPathURL );
	strText += strTemp;

	strText += "</dl><hr>";

	GetTextOutput()->Add( strText );
}


void ChWorldMainInfo::DisplayConnectDlg( const ChWorldInfo& info )
{
	ChString		strMessage;
	ChString		strFormat;
	ChString		strName( info.GetName() );

	if (strName.IsEmpty())
	{
		strName = info.GetHost();
	}

	LOADSTRING( IDS_LOOKUP_STATUS_MSG, strFormat );
	strMessage.Format( strFormat, (const char*)strName );

	m_pConnectingDlg = new ChConnectingDlg( this );
	ASSERT( m_pConnectingDlg );

	m_pConnectingDlg->Create( strMessage );
	m_pConnectingDlg->CenterWindow( GetCore()->GetFrameWnd() );
	m_pConnectingDlg->ShowAfterASec();

	GetCore()->GetFrameWnd()->EnableWindow( false );
}


void ChWorldMainInfo::CloseConnectDlg()
{
	GetCore()->GetFrameWnd()->EnableWindow();

	if (m_pConnectingDlg)
	{										// Destroy the 'connecting...' dlg
		m_pConnectingDlg->ShowWindow( SW_HIDE );

		m_pConnectingDlg->DestroyWindow();
		delete m_pConnectingDlg;
		m_pConnectingDlg = 0;
	}

	GetCore()->GetFrameWnd()->ActivateFrame();
}


/*----------------------------------------------------------------------------
	ChWorldMainInfo private methods
----------------------------------------------------------------------------*/

void ChWorldMainInfo::RegisterDispatchers()
{
	chint16		sHandlerCount = sizeof( worldHandlers ) /
								sizeof( ChMsgHandlerDesc );

	m_worldDispatcher.AddHandler( worldHandlers, sHandlerCount );
}


#if !defined( CH_PUEBLO_PLUGIN )

void ChWorldMainInfo::CreateMenus()
{
	ChRMenuMgr*		pMgr  = GetCore()->GetMenuMgr();
	ChMenuItem*		pItem;
	ChString			strText;

	ASSERT( pMgr );

	LOADSTRING( IDS_MENU_WORLD, strText );

	m_pStdFileMenu = new ChFileMenu( pMgr, fileMenuHandler );
	m_pStdEditMenu = new ChEditMenu( pMgr, editMenuHandler );
	m_pStdViewMenu = new ChViewMenu( pMgr, viewMenuHandler );
	m_pWorldMenu = new ChMenu( pMgr, strText, worldMenuHandler );
	m_pStdWindowMenu = new ChWindowMenu( pMgr, windowMenuHandler );

	m_boolMenus = true;

	ASSERT( 0 != GetFileMenu() );
	ASSERT( 0 != GetEditMenu() );
	ASSERT( 0 != GetViewMenu() );
	ASSERT( 0 != GetWorldMenu() );
	ASSERT( 0 != GetWindowMenu() );
											// Add our items to the File menu
	LOADSTRING( IDS_MENU_WORLD_LOGGING, strText );
	pItem = GetFileMenu()->InsertItem( strText, OnWorldLoggingCommand );
	LOADSTRING( IDS_MENU_WORLD_LOGGING_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->Enable( false );
	pItem->SetData( (chparam)this );

	GetFileMenu()->InsertSeparator();
											/* Add our menu hooks to the
												standard Edit menu */
	GetEditMenu()->GetCutItem()->
					SetHandler( OnStdEditCut )->SetData( (chparam)this );
	GetEditMenu()->GetCopyItem()->
					SetHandler( OnStdEditCopy )->SetData( (chparam)this );
	GetEditMenu()->GetPasteItem()->
					SetHandler( OnStdEditPaste )->SetData( (chparam)this );

											// Hook into 'View/Previous'
	LOADSTRING( IDS_MENU_VIEW_PREV, strText );
	pItem = GetViewMenu()->InsertItem( strText, OnViewPrevCommand );
	LOADSTRING( IDS_MENU_VIEW_PREV_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->SetData( (chparam)this );
											// Add our items to the World menu

	LOADSTRING( IDS_MENU_QUICK_CONNECT, strText );
	pItem = GetWorldMenu()->InsertItem( strText, OnWorldQuickConnectCommand );
	LOADSTRING( IDS_MENU_QUICK_CONNECT_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->Enable( false );
	pItem->SetData( (chparam)this );

	GetWorldMenu()->InsertSeparator();

	LOADSTRING( IDS_MENU_WORLD_LIST, strText );
	pItem = GetWorldMenu()->InsertItem( strText, OnWorldListCommand );
	LOADSTRING( IDS_MENU_WORLD_LIST_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->SetData( (chparam)this );

	LOADSTRING( IDS_MENU_ADD, strText );
	pItem = GetWorldMenu()->InsertItem( strText, OnWorldAddCommand );
	LOADSTRING( IDS_MENU_ADD_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->Enable( false );
	pItem->SetData( (chparam)this );

	LOADSTRING( IDS_MENU_CREATE_SHORTCUT, strText );
	pItem = GetWorldMenu()->InsertItem( strText,
										OnWorldCreateShortcutCommand );
	LOADSTRING( IDS_MENU_CREATE_SHORTCUT_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->Enable( false );
	pItem->SetData( (chparam)this );

	GetWorldMenu()->InsertSeparator();

	LOADSTRING( IDS_MENU_WORLD_DISCON, strText );
	pItem = GetWorldMenu()->InsertItem( strText, OnWorldDisconnectCommand );
	LOADSTRING( IDS_MENU_WORLD_DISCON_TXT, strText );
	pItem->SetHelpText( strText );
	pItem->Enable( false );
	pItem->SetData( (chparam)this );
											/* Add our item to the standard
												Window menu */

	LOADSTRING( IDS_WND_MENU_INPUT, strText );
	pItem = GetWindowMenu()->InsertItem( strText, OnWindowInputCommand );
	LOADSTRING( IDS_WND_MENU_INPUT_TXT, strText );
	pItem->SetHelpText( strText );
	LOADSTRING( IDS_WND_MENU_INPUT_ACC, strText );
	pItem->SetAccelerator( strText[0], CH_ACC_CONTROL | CH_ACC_VIRTKEY );
	pItem->SetData( (chparam)this );
}


void ChWorldMainInfo::InstallMenus()
{
	ASSERT( 0 != GetFileMenu() );
	ASSERT( 0 != GetEditMenu() );
	ASSERT( 0 != GetViewMenu() );
	ASSERT( 0 != GetWorldMenu() );
	ASSERT( 0 != GetWindowMenu() );

	GetFileMenu()->Install( CH_MODULE_WORLD );
	GetEditMenu()->Install( CH_MODULE_WORLD );
	GetViewMenu()->Install( CH_MODULE_WORLD );
	GetWorldMenu()->Install( CH_MODULE_WORLD );
	GetWindowMenu()->Install( CH_MODULE_WORLD );

	m_boolMenusInstalled = true;
}


void ChWorldMainInfo::UninstallMenus()
{
	ASSERT( 0 != GetFileMenu() );
	ASSERT( 0 != GetEditMenu() );
	ASSERT( 0 != GetViewMenu() );
	ASSERT( 0 != GetWorldMenu() );
	ASSERT( 0 != GetWindowMenu() );

	GetFileMenu()->Uninstall();
	GetEditMenu()->Uninstall();
	GetViewMenu()->Uninstall();
	GetWorldMenu()->Uninstall();
	GetWindowMenu()->Uninstall();

	m_boolMenusInstalled = false;
}


void ChWorldMainInfo::DestroyMenus()
{
	ASSERT( 0 != GetFileMenu() );
	ASSERT( 0 != GetEditMenu() );
	ASSERT( 0 != GetViewMenu() );
	ASSERT( 0 != GetWorldMenu() );
	ASSERT( 0 != GetWindowMenu() );
											// Delete all of the menu objects
	delete m_pStdFileMenu;
	m_pStdFileMenu = 0;

	delete m_pStdEditMenu;
	m_pStdEditMenu = 0;

	delete m_pStdViewMenu;
	m_pStdViewMenu = 0;

	delete m_pWorldMenu;
	m_pWorldMenu = 0;

	delete m_pStdWindowMenu;
	m_pStdWindowMenu = 0;

	m_boolMenus = false;
}

#endif // !defined( CH_PUEBLO_PLUGIN )


void ChWorldMainInfo::Disconnect()
{
	m_iConnectID++;							// Update the connect ID

	if (0 != m_pWorldConn)
	{
		delete m_pWorldConn;
		m_pWorldConn = 0;
	}

	if (GetTextOutput()->IsLogging())
	{										// Turn off logging if it's on
		GetTextOutput()->ToggleLogging();
	}
}


void ChWorldMainInfo::AddURLToList( const ChString& strURL )
{
	ChString		strAbsURL;
	ChString*		pstrCopy;

	if ((0 == strURL.Compare( WORLD_LIST_EDIT_URL )) ||
				(0 == strURL.Compare( DISCONNECT_URL )) ||
				(0 == strURL.Compare( RECONNECT_URL )))
	{
											/* Don't add actions to the URL
												list */
		return;
	}
	else if ((0 == strURL.Compare( PERSONAL_URL )) ||
				(0 == strURL.Compare( WORLD_LIST_URL )) ||
				(0 == strURL.Compare( PERSONAL_URL_OLD )))
	{
		strAbsURL = strURL;
	}
	else
	{
		ChURLParts	parts;
		ChString		strPrevURL;
		const char*	pstrPrevURL;

		if (m_urlList.IsEmpty())
		{
			pstrPrevURL = 0;
		}
		else
		{
			strPrevURL = *(ChString*)m_urlList.GetTail();

			if ((0 == strPrevURL.Compare( PERSONAL_URL )) ||
				(0 == strPrevURL.Compare( PERSONAL_URL_OLD )))
			{
											// No previous relative URL
				pstrPrevURL = 0;
			}
			else if (0 == strPrevURL.Compare( WORLD_LIST_URL ))
			{
											/* The relative URL is from the
												home page */
				strPrevURL = GetHomePage();
				pstrPrevURL = strPrevURL;
			}
			else
			{
				pstrPrevURL = strPrevURL;
			}
		}
											// Calculate the absolute URL
		parts.GetURLParts( strURL, pstrPrevURL );
		strAbsURL = parts.GetURL();
	}

	ASSERT( !strAbsURL.IsEmpty() );

	pstrCopy = new ChString( strAbsURL );
	m_urlList.AddTail( (chparam)pstrCopy );
}

void ChWorldMainInfo::EmptyURLList()
{
	while (!m_urlList.IsEmpty())
	{
		ChString*		pstrTail;

		pstrTail = (ChString*)m_urlList.GetTail();
		m_urlList.RemoveTail();
											// Free the item
		delete pstrTail;
	}
}


void ChWorldMainInfo::LoadSoundModule( bool boolOptional )
{
	WorldLoadInfo*	pInfo;

	pInfo = new WorldLoadInfo( CH_MODULE_SOUND );
	LoadClientModule( CH_MODULE_SOUND, CH_MODULE_SOUND_BASE,
						GetModuleID(), (chparam)pInfo, boolOptional );
}


void ChWorldMainInfo::UnloadSoundModule()
{
	if (m_idSoundModule)
	{
		UnloadClientModule( m_idSoundModule );
		m_idSoundModule = 0;
	}
}


void ChWorldMainInfo::LoadGraphicsModule( bool boolOptional )
{
	TRACE0("Graphics module loading disabled (not imported)\n");
	/*
	WorldLoadInfo*	pInfo;

	TRACE0("ChWorldMainInfo::LoadGraphicsModule\n");
	pInfo = new WorldLoadInfo( CH_MODULE_GRAPHICS_PANE );
	ASSERT( pInfo );

	TRACE0(" - Loading module...\n");
	LoadClientModule( CH_MODULE_GRAPHICS_PANE, CH_MODULE_GRAPHICS_BASE,
						GetModuleID(), (chparam)pInfo, boolOptional );
	TRACE0(" - Done.\n");
	*/
}


void ChWorldMainInfo::UnloadGraphicsModule()
{
	if (GetGraphicsID())
	{										// Unhook the graphics module

		ChUninstallHookMsg	unhookCmdMsg( GetModuleID(), CH_MSG_CMD );
		ChUninstallHookMsg	unhookHintMsg( GetModuleID(), CH_MSG_HINT );
		ChUninstallHookMsg	unhookInlineMsg( GetModuleID(), CH_MSG_INLINE );

		NotifyGraphics( unhookCmdMsg );
		NotifyGraphics( unhookHintMsg );
		NotifyGraphics( unhookInlineMsg );
	}

	if (m_idGraphicsModule)
	{
		UnloadClientModule( m_idGraphicsModule );
	}
}


void ChWorldMainInfo::SendConnectedMsg()
{
	ChConnectedMsg		connectedMsg;

	m_strPuebloClientParams = "";

	if (GetSoundID())
	{
		NotifySound( connectedMsg );

		if (!m_strPuebloClientParams.IsEmpty())
		{
			m_strPuebloClientParams += ' ';
		}
		m_strPuebloClientParams += connectedMsg.GetPuebloClientParams();
		connectedMsg.ClearPuebloClientParams();
	}

	if (GetGraphicsID())
	{
		NotifyGraphics( connectedMsg );

		if (!m_strPuebloClientParams.IsEmpty())
		{
			m_strPuebloClientParams += ' ';
		}
		m_strPuebloClientParams += connectedMsg.GetPuebloClientParams();
		connectedMsg.ClearPuebloClientParams();
	}
}


void ChWorldMainInfo::TrackUsage( int iType )
{

	ChString 				strFormat;
	ChWorldHTTPReq* 	pReq = 0;

	TRACE1("ChWorldMainInfo::TrackUsage(%d)\n", iType);

	switch( iType )
	{
		case worldConnect :
		{
			LOADSTRING( IDS_ONCONNECT_URL, strFormat  );
			pReq = new ChWorldConnectHTTPReq( GetConnectID() );
			break;
		}
		case worldDisconnect :
		{
			LOADSTRING( IDS_ONDISCONNECT_URL, strFormat  );
			pReq = new ChWorldDisconnectHTTPReq( GetConnectID() );
			break;
		}
		case worldEnhanced :
		{
			LOADSTRING( IDS_PUEBLOENHANCED_URL, strFormat  );
			pReq = new ChWorldEnhancedHTTPReq( GetConnectID(),
					GetTextOutput()->GetOutputWnd()->GetSafeHwnd() );
			break;
		}
	}

	if ( pReq )
	{


		ChString 				strURL, strData, strClientVersion;

		// format time in RFC 870 format
		time_t currTime;
		time( &currTime );
		struct tm* pTheTime = gmtime( &currTime );
		if (pTheTime )
		{
			ChString strTime;

			if (strftime(strTime.GetBuffer( 256 ), 256, "%A, %d-%b-%y %H:%M:%S GMT", pTheTime ))
			{
				strTime.ReleaseBuffer();
				ChUtil::HtmlAddNameValuePair( strData, "Time", strTime );
			}
			else
			{
				strTime.ReleaseBuffer();
			}
		}

		ASSERT( GetWorldInfo() );

		ChUtil::HtmlAddNameValuePair( strData, "Host", GetWorldInfo()->GetHost() );
		ChUtil::HtmlAddNameValuePair( strData, "Port", GetWorldInfo()->GetPort() );

											// Current client version

		strClientVersion = GetCore()->GetClientInfo()->
							GetClientVersion().Format( ChVersion::formatShort );
		strURL.Format( strFormat, LPCSTR(strClientVersion) );

		GetCore()->PostURL( strURL, strData, strData.GetLength(),
								0, GetStream(),
								(chparam)pReq  );
	}
}


void ChWorldMainInfo::CreateMD5Checksum()
{
	chuint32		luTime = 0;
	chuint32		luTicks = 0;
	chuint32		luAddress = 0;
	char			cTemp[65];
	const char*		pstrTemp = cTemp;
	ChString			strMD5;
	char			cLocalHostName[255];
	ChString			strWorldName = GetWorldInfo()->GetName();
	const char*		pstrWorldName;

	luTime = time(0);
	if (0 == gethostname( cLocalHostName, sizeof( cLocalHostName ) ))
	{
		hostent*	pLocalHost;

		pLocalHost = gethostbyname( cLocalHostName );
		if (pLocalHost)
		{
			LPIN_ADDR	pAddress = (LPIN_ADDR)(pLocalHost->h_addr_list[0]);

			luAddress = pAddress->s_addr;
		}
	}

	luTicks = GetTickCount();
											/* Make sure strWorldName has enough
												characters to cast... */

	strWorldName += "xXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxX";
	pstrWorldName = strWorldName;
											/* XOR the world name through the
												values */
	luTime ^= (chuint32)pstrWorldName;
	pstrWorldName += 4;
	luAddress ^= (chuint32)pstrWorldName;
	pstrWorldName += 4;
	luTicks ^= (chuint32)pstrWorldName;
	pstrWorldName += 4;

	strMD5.Format( "%010lx%011lx%011lx", luTime, luAddress, luTicks );
	SetMD5( strMD5 );
}


/*----------------------------------------------------------------------------
	Chaco module library entry point
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
CH_EXTERN_LIBRARY( void )
InitChWorldDLL();
#endif

#ifdef __linux__
CH_IMPLEMENT_MAIN_HANDLER( ChMainEntryWorld )
#else
#if defined( CH_PUEBLO_PLUGIN )
STDAPI_(int) 				
ChMainEntryWorld( ChMsg &msg, ChCore *pCore, ChMainInfo *pMainInfo, 
								ChModuleID idModule, const ChString *pstrModule, 
								ChArgumentList *pArgList )
#else
ChMain
#endif
#endif
{
	chparam		retVal = 0;

	switch( msg.GetMessage() )
	{
		case CH_MSG_INIT:
		{
			ChInitMsg	*pMsg = (ChInitMsg *)&msg;
			ChString		strLoadParam;
			ChModuleID	idServerModule;

			#if defined( CH_MSW ) && defined( CH_ARCH_16 )
			{
											// Initialize MFC
				InitChWorldDLL();
			}
			#endif	// defined( CH_MSW ) && defined( CH_ARCH_16 )

			pMsg->GetParams( idModule, strLoadParam, idServerModule );

			if (*pstrModule == CH_MODULE_WORLD)
			{
				ChWorldMainInfo	*pMainInfo;

				pMainInfo = new ChWorldMainInfo( idModule, pCore, pArgList );

				retVal = (chparam)pMainInfo;
			}

			break;
		}

		case CH_MSG_TERM:
		{
			DebugBox("WORLD: Entering TERM");
			delete pMainInfo;
			DebugBox("WORLD: Deleted info");
			break;
		}
	}

	return retVal;
}


/*----------------------------------------------------------------------------
	Chaco socket handler
----------------------------------------------------------------------------*/

#ifdef _DEBUG
extern "C" __declspec(dllimport) int __cdecl _CrtCheckMemory();
#endif

CH_IMPLEMENT_SOCKET_HANDLER( worldSocketHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)socket.GetUserData();

#ifdef _DEBUG
	_CrtCheckMemory();
#endif

	switch( luEvent )
	{
		case CH_SOCK_EVENT_CONNECT:
		{
			pInfo->OnConnectComplete( iErrorCode );
			break;
		}

		case CH_SOCK_EVENT_CLOSE:
		{
			pInfo->ShutdownWorld();
			break;
		}

		case CH_SOCK_EVENT_READ:
		{
			pInfo->GetWorldConnection()->ProcessOutput();
			break;
		}

		default:
		{
			TRACE1( "ChWorld: Unhandled socket event: %ld\n", luEvent );
		}
	}
}


CH_IMPLEMENT_SOCKET_ASYNCHANDLER( worldSocketAsyncHandler )
{
	ChWorldMainInfo*	pMainInfo = (ChWorldMainInfo*)pInfo->GetUserData();
	WPARAM				wParam;
	LPARAM				lParam;
	int					iError;
	chuint32			luAddress;

	pInfo->GetParams( wParam, lParam );
	iError = WSAGETASYNCERROR( lParam );

	if (0 == iError)
	{
		hostent*		pHostEntry = (hostent*)pInfo->GetBuf();
		sockaddr_in		socket_address;

		memcpy( &socket_address.sin_addr, pHostEntry->h_addr,
					pHostEntry->h_length );
		luAddress = ntohl( socket_address.sin_addr.s_addr );
	}

	pMainInfo->OnAsyncSocketAddress( iError, luAddress );
}


/*----------------------------------------------------------------------------
	Chaco message handlers
----------------------------------------------------------------------------*/

CH_IMPLEMENT_MESSAGE_HANDLER( defWorldHandler )
{
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldInitHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;

#if !defined( CH_PUEBLO_PLUGIN )
	ChString strOffline;
	LOADSTRING( IDS_OFFLINE, strOffline );
	pMainInfo->GetCore()->UpdateSessionPane( strOffline, false );
#endif

	pInfo->Initialize();
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldShowModuleHandler )
{
	ChShowModuleMsg*	pMsg = (ChShowModuleMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;

	pInfo->ShowModule( pMsg->IsShowing() );
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldLoadCompleteHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChLoadCompleteMsg*	pMsg = (ChLoadCompleteMsg*)&msg;
	ChString				strModuleName;
	ChModuleID			idModule;
	ChString				strFilename;
	chparam				userData;
	WorldLoadInfo*		pLoadInfo;

	pMsg->GetParams( strModuleName, idModule, strFilename, userData );

	pLoadInfo = (WorldLoadInfo*)userData;
	ASSERT( pLoadInfo );

#ifdef __BORLANDC__
 #pragma warn -ccc
#endif
	if (pLoadInfo->GetType() == WorldLoadInfo::typeModule)
	{
#ifdef __BORLANDC__
 #pragma warn .ccc
#endif
		if (CH_MODULE_GRAPHICS_PANE == strModuleName)
		{
			TRACE1("worldLoadCompleteHandler(GraphicsModule:%d)\n", idModule);
			pInfo->SetGraphicsID( idModule );
		}
		else if (CH_MODULE_SOUND == strModuleName)
		{
			pInfo->SetSoundID( idModule );
		}
	}
	delete pLoadInfo;
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldLoadErrorHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChLoadErrorMsg*		pMsg = (ChLoadErrorMsg*)&msg;
	ChString				strModuleName;
	ChModuleID			idModule;
	ChString				strURL;
	chint32				lError;
	chparam				userData;
	WorldLoadInfo*		pLoadInfo;
											// Get message params

	pMsg->GetParams( strModuleName, idModule, strURL, lError, userData );

	pLoadInfo = (WorldLoadInfo*)userData;
	ASSERT( pLoadInfo );
											/* As long as the error isn't
												'errOptionalNotCached', then
												process the error.  Otherwise,
												just delete the user data */

	if (lError != ChLoadErrorMsg::errOptionalNotCached)
	{
#ifdef __BORLANDC__
 #pragma warn -ccc
#endif
		if (pLoadInfo->GetType() != WorldLoadInfo::typeModule)
		{
#ifdef __BORLANDC__
 #pragma warn .ccc
#endif
											// Not a module, must be data
		}
		else
		{
			ChString		strModuleName = pLoadInfo->GetModuleName();
			ChString		strErrorCaption;

			LOADSTRING( IDS_LOAD_ERROR_CAPTION, strErrorCaption );

			if (CH_MODULE_GRAPHICS_PANE == strModuleName)
			{
				ChString		strErrorMsg;

				LOADSTRING( IDS_GRAPHICS_LOAD_ERROR, strErrorMsg );

				#if defined( CH_MSW )
				{

					pMainInfo->GetCore()->GetFrameWnd()->MessageBox( strErrorMsg,
														strErrorCaption );
				}
				#elif defined( CH_UNIX )
				{
					TRACE( strErrorMsg );
				}
				#endif
			}
			else if (CH_MODULE_SOUND == strModuleName)
			{
				ChString		strErrorMsg;

				LOADSTRING( IDS_SOUND_LOAD_ERROR, strErrorMsg );

				#if defined( CH_MSW )
				{

					pMainInfo->GetCore()->GetFrameWnd()->MessageBox( strErrorMsg,
														strErrorCaption );
				}
				#elif defined( CH_UNIX )
				{
					TRACE( strErrorMsg );
				}
				#endif
			}
		}
	}

	delete pLoadInfo;

	return 0;
}


#if !defined( CH_PUEBLO_PLUGIN )

CH_IMPLEMENT_MESSAGE_HANDLER( worldGetPageCountHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChGetPageCountMsg*	pMsg = (ChGetPageCountMsg*)&msg;
	ChPageType			type;
	int					iPageCount;

	pMsg->GetParams( type );

	if (pInfo->IsShown())
	{
		switch( type )
		{
			case pagePreferences:
			{
				iPageCount = 3;
				break;
			}

			case pageAbout:
			{
				iPageCount = 1;
				break;
			}

			default:
			{
				iPageCount = 0;
				break;
			}
		}
	}
	else
	{
		iPageCount = 0;
	}

	return iPageCount;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldGetPagesHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChGetPagesMsg*		pMsg = (ChGetPagesMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			ASSERT( 3 == sCount );

			#if defined( CH_MSW )
			{
				ChWorldPrefsPage*		pWorldPage;
				ChTextInputPrefsPage*	pTextInputPage;
				ChNotifyPrefsPage*		pNotifyPage;

											// Create the pages

				pWorldPage = new ChWorldPrefsPage;
				pTextInputPage = new ChTextInputPrefsPage;
				pNotifyPage = new ChNotifyPrefsPage;

											// Set initial data

				pNotifyPage->Set( pInfo->GetNotify(), pInfo->GetNotifyAlert(),
									pInfo->GetNotifyMatch() );

											// Return the pages
				pPages[0] = (chparam)pWorldPage;
				pPages[1] = (chparam)pTextInputPage;
				pPages[2] = (chparam)pNotifyPage;
			}
			#endif	// defined( CH_MSW )
			break;
		}

		case pageAbout:
		{
			ASSERT( 1 == sCount );

			#if defined( CH_MSW )
			{
				ChTinTinAbout*		pTinTinAbout;

											// Create the page

				pTinTinAbout = new ChTinTinAbout;

											// Return the page

				pPages[0] = (chparam)pTinTinAbout;
			}
			#endif	// defined( CH_MSW )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldGetPageDataHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChGetPageDataMsg*	pMsg = (ChGetPageDataMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			ASSERT( 3 == sCount );

			#if defined( CH_MSW )
			{
				if (pPages[0])
				{
					ChWorldPrefsPage*	pPage = (ChWorldPrefsPage*)pPages[0];

					pPage->OnCommit();
				}

				if (pPages[1])
				{
					ChTextInputPrefsPage*	pPage;

					pPage = (ChTextInputPrefsPage*)pPages[1];
					pPage->OnCommit();
				}

				if (pPages[2])
				{
					ChNotifyPrefsPage*	pPage = (ChNotifyPrefsPage*)pPages[2];

					pPage->OnCommit();
				}

				pInfo->UpdatePreferences();
			}
			#endif	// defined( CH_MSW )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldReleasePagesHandler )
{
	ChReleasePagesMsg*	pMsg = (ChReleasePagesMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			ASSERT( 3 == sCount );

			#if defined( CH_MSW )
			{
				if (pPages[0])
				{
					ChWorldPrefsPage*	pPage = (ChWorldPrefsPage*)pPages[0];

					delete pPage;
				}

				if (pPages[1])
				{
					ChTextInputPrefsPage*	pPage;

					pPage = (ChTextInputPrefsPage*)pPages[1];
					delete pPage;
				}

				if (pPages[2])
				{
					ChNotifyPrefsPage*	pPage = (ChNotifyPrefsPage*)pPages[2];

					delete pPage;
				}
			}
			#endif	// defined( CH_MSW )
			break;
		}

		case pageAbout:
		{
			ASSERT( 1 == sCount );

			#if defined( CH_MSW )
			{
				if (pPages[0])
				{
					ChTinTinAbout*	pPage = (ChTinTinAbout*)pPages[0];

					delete pPage;
				}
			}
			#endif	// defined( CH_MSW )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}
#endif


CH_IMPLEMENT_MESSAGE_HANDLER( worldCommandHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChCmdMsg*			pMsg = (ChCmdMsg*)&msg;
	ChString				strArgs;
	chint32				lXCoord;
	chint32				lYCoord;

	pMsg->GetParams( strArgs, lXCoord, lYCoord );
	if (!strArgs.IsEmpty())
	{
		pMsg->SetProcessed( pInfo->DoCommand( strArgs, lXCoord, lYCoord ) );
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldInlineHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChInlineMsg*		pMsg = (ChInlineMsg*)&msg;
	ChString				strArgs;

	pMsg->GetParams( strArgs );
	pMsg->SetProcessed( pInfo->DoInline( strArgs ) );

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldHintHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChHintMsg*			pMsg = (ChHintMsg*)&msg;
	ChString				strHint;

	pMsg->GetParams( strHint );
	pMsg->SetProcessed( pInfo->DoHint( strHint ) );

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldInvalidWorldHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChInvalidWorldMsg*	pMsg = (ChInvalidWorldMsg*)&msg;
	ChString				strReason;

	pMsg->GetParams( strReason );
	pInfo->OnInvalidWorld( strReason );

	msg.SetProcessed( true );
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldSendWorldCmdHandler )
{
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMainInfo;
	ChSendWorldCmdMsg*	pMsg = (ChSendWorldCmdMsg*)&msg;

	pInfo->Send( pMsg->GetDefaultCmd(), pMsg->GetMD5(),
					pMsg->GetOverrideCmd(), pMsg->GetParams(),
					pMsg->GetEcho() );

	msg.SetProcessed( true );
	return 0;
}


#if !defined( CH_PUEBLO_PLUGIN )
/*----------------------------------------------------------------------------
	Chaco menu handlers
----------------------------------------------------------------------------*/

CH_IMPLEMENT_MESSAGE_HANDLER( fileMenuHandler )
{
	chparam		retVal = 0;

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( editMenuHandler )
{
	chparam		retVal = 0;

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnStdEditCut )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	pMsg->SetProcessed();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SHOW:
		{
			bool	boolEnable;

			boolEnable = pInfo->CheckEditMenuItem( editMenuCut );
			pMsg->GetItem()->Enable( boolEnable );
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SELECT:
		{
			pInfo->DoEditMenuItem( editMenuCut );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}
	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnStdEditCopy )
{
	chparam		retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	pMsg->SetProcessed();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SHOW:
		{
			bool	boolEnable;

			boolEnable = pInfo->CheckEditMenuItem( editMenuCopy );
			pMsg->GetItem()->Enable( boolEnable );
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SELECT:
		{
			pInfo->DoEditMenuItem( editMenuCopy );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnStdEditPaste )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	pMsg->SetProcessed();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SHOW:
		{
			bool	boolEnable;

			boolEnable = pInfo->CheckEditMenuItem( editMenuPaste );
			pMsg->GetItem()->Enable( boolEnable );
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SELECT:
		{
			pInfo->DoEditMenuItem( editMenuPaste );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( viewMenuHandler )
{
	chparam		retVal = 0;

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnViewPrevCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pInfo->DoPreviousURL();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			bool	boolEnable = !pInfo->IsLoadPending() &&
									!pInfo->IsTopLevelWorldList() &&
									!pInfo->IsConnected();

			pMsg->GetItem()->Enable( boolEnable );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( worldMenuHandler )
{
	chparam		retVal = 0;

	return retVal;
}

CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldListCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pInfo->EditPersonalWorldList();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			pMsg->GetItem()->Enable();
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldAddCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pInfo->AddCurrentWorld();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			pMsg->GetItem()->Enable( pInfo->IsConnected() );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldCreateShortcutCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			if (pInfo->IsConnected())
			{
				pInfo->CreateCurrentWorldShortcut();
			}
			else
			{
				pInfo->CreateShortcut();
			}
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			static bool		boolWasConnected = false;
			bool			boolIsConnected = pInfo->IsConnected();

			if (boolWasConnected != boolIsConnected)
			{
				ChString		strText;

				if (boolIsConnected)
				{
					LOADSTRING( IDS_MENU_CREATE_SHORTCUT_CONNECTED_TXT,
								strText );
				}
				else
				{
					LOADSTRING( IDS_MENU_CREATE_SHORTCUT_TXT, strText );
				}

				pMsg->GetItem()->SetHelpText( strText );
				boolWasConnected = boolIsConnected;
			}

			pMsg->GetItem()->Enable();
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldQuickConnectCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pInfo->DoQuickConnect();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			pMsg->GetItem()->Enable( !pInfo->IsConnected() );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldDisconnectCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pInfo->ShutdownWorld();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			pMsg->GetItem()->Enable( pInfo->IsConnected() );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWorldLoggingCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();
	ChTextOutput*		pTextOutput = pInfo->GetTextOutput();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			pTextOutput->ToggleLogging();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			static bool		boolLogging = false;

			if (boolLogging != pTextOutput->IsLogging())
			{
				ChString		strHelpText;

				boolLogging = !boolLogging;

				if (boolLogging)
				{
					LOADSTRING( IDS_MENU_WORLD_LOGGING_OFF_TXT, strHelpText );
				}
				else
				{
					LOADSTRING( IDS_MENU_WORLD_LOGGING_TXT, strHelpText );
				}

				pMsg->GetItem()->Check( boolLogging );
				pMsg->GetItem()->SetHelpText( strHelpText );
			}

			pMsg->GetItem()->Enable( pInfo->IsConnected() );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( windowMenuHandler )
{
	chparam		retVal = 0;
	ChMenuMsg*	pMsg = (ChMenuMsg*)&msg;

	pMsg->SetProcessed();

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( OnWindowInputCommand )
{
	chparam				retVal = false;
	ChMenuMsg*			pMsg = (ChMenuMsg*)&msg;
	ChWorldMainInfo*	pInfo = (ChWorldMainInfo*)pMsg->GetItem()->GetData();

	pMsg->SetProcessed();

	switch( pMsg->GetMessage() )
	{
		case CH_MSG_MENU_SELECT:
		{
			ChTextInput*	pTextInput = pInfo->GetTextInput();

			pTextInput->SetFocus();
			pMsg->SetProcessed();
			break;
		}

		case CH_MSG_MENU_SHOW:
		{
			pMsg->GetItem()->Enable( pInfo->IsConnected() );
			pMsg->SetProcessed();
			break;
		}

		default:
		{
			break;
		}
	}

	return retVal;
}

#endif //  !defined( CH_PUEBLO_PLUGIN )


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( CommandType )
GetCommand( const ChString& strCommand, ChString& strValue, bool boolInline )
{
	CommandType		command = invalid;

	if (ChHtmlWnd::GetHTMLHref( strCommand, boolInline, strValue ))
	{
		command = href;
	}
	else if (ChHtmlWnd::GetHTMLAttribute( strCommand, ATTR_XWORLD, strValue ))
	{
		command = xworld;
	}
	else if (ChHtmlWnd::GetHTMLAttribute( strCommand, ATTR_XCMD, strValue ))
	{
		command = xcmd;
	}

	return command;
}


CH_INTERN_FUNC( bool )
FormatHint( ChString& strHint )
{
	bool			boolProcessed = false;
	ChString			strArgs;
	ChString			strHintArg;
	CommandType		command;

	command = GetCommand( strHint, strArgs, false );

	if (ChHtmlWnd::GetHTMLAttribute( strHint, ATTR_XHINT, strHintArg ))
	{
		if (xcmd == command)
		{
			strHint = strArgs + " (";
			strHint += strHintArg + ")";
		}
		else
		{
			strHint = strHintArg;
		}
		boolProcessed = true;
	}
	else
	{
		bool	boolSelf;
		ChString	strTarget;

		if (ChHtmlWnd::GetHTMLAttribute( strHint, ATTR_TARGET, strTarget ))
		{
			boolSelf = IsTargetSelfOutput( strTarget );
		}
		else
		{
			boolSelf = false;
		}

		switch( command )
		{
			case href:
			{
				if (boolSelf)
				{
					LOADSTRING( IDS_HINT_JUMP_SELF, strHint );
				}
				else
				{
					int		iIndex = strArgs.Find( MAILTO_URL_PREFIX );

					if (0 == iIndex)
					{
						ChString		strEmailAddress;
						ChString		strFormat;

						strEmailAddress =
								strArgs.Mid( strlen( MAILTO_URL_PREFIX ) );
						strEmailAddress.TrimLeft();
						strEmailAddress.TrimRight();
						LOADSTRING( IDS_HINT_SEND_MAIL, strFormat );
						strHint.Format( strFormat,
										(const char*)strEmailAddress );
					}
					else
					{
						ChString strFormat;

						LOADSTRING( IDS_HINT_JUMP, strFormat );
						strHint.Format( strFormat, (const char*)strArgs );
					}
				}

				boolProcessed = true;
				break;
			}

			case xworld:
			{
				ChWorldInfo	info( strArgs );
				ChString		strText;

				strText = info.GetHost();
				if (!strText.IsEmpty())
				{
					chint16		sPort = info.GetPort();

					LOADSTRING( IDS_HINT_CONNECT_PREFIX, strHint );

					strHint += strText;

					if (sPort > 0)
					{
						strHint += ":";
						#if defined( CH_ARCH_16 )
						char * pstrText = strText.GetBufferSetLength( 20 );
						::wsprintf( pstrText, "%d", (int)sPort );
						strText.ReleaseBuffer();
						#else
						strText.Format( "%d", (int)sPort );
						#endif

						strHint += strText;
					}

					if (otherType != info.GetType())
					{
						ChString		strType( info.GetType().GetName() );

						strType.MakeLower();

						strHint += " (";
						strHint += strType +")";
					}
				}
				else
				{
					strHint = "";
				}

				boolProcessed = true;
				break;
			}

			case xcmd:
			{
				strHint = strArgs;
				boolProcessed = true;
				break;
			}

			default:
			{
				boolProcessed = false;
				break;
			}
		}
	}

	return boolProcessed;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
