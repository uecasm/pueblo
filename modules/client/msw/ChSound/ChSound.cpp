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
					
		 Ultra Enterprises:  Gavin Lambert
		 
		 			Modified to look for WinXP sound files in the
		 			Media subdirectory as well.

------------------------------------------------------------------------------

	Defines the ChSound module for the Pueblo system.  This module is
	used to play MIDI music.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include "ChMPlay.h"

#ifdef CH_UNIX
#include <ChDispat.h>
#include <ChMsgTyp.h>
#include <ChReg.h>
#include "../../unix/ChTxtMud/UnixRes.h"
#endif // CH_UNIX

#include <fstream>

#include <ChMsgTyp.h>
#include <ChExcept.h>
#include <ChMenu.h>
#include <ChHtmWnd.h>
#include <ChCore.h>
#include <ChHtpCon.h>

#include <ChSound.h>
#include <ChWorld.h>

#include "ChSoundInfo.h"
#include "ChSoundStream.h"
#include "ChTNT.h"
#include "ChSpeechPrefs.h"
#include "ChSpeechStatus.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define TNT_PUEBLOSOUND_PARAM	"speech_port"

											/* Note that the following
												strings should be entirely
												in lower case */
#define ATTR_XCH_SOUND			"xch_sound"
#define ATTR_XCH_ALERT			"xch_alert"
#define ATTR_XCH_SPEECH			"xch_speech"
#define ATTR_XCH_DEVICE			"xch_device"
#define ATTR_XCH_VOLUME			"xch_volume"
#define ATTR_EVENT				"event"
#define ATTR_OPTIONS			"options"
#define ATTR_REMOTE_OPTIONS		"remote_options"
#define ATTR_ACTION				"action"
#define ATTR_MD5				"md5"
#define ATTR_HOST				"host"
#define ATTR_ID					"id"

#define SND_CMD_PLAY			"play"
#define SND_CMD_LOOP			"loop"
#define SND_CMD_STOP			"stop"
#define SND_CMD_STOPLOOP		"stoploop"

#if defined( CH_USE_VOXWARE )

#define SPEECH_CMD_CALL			"call"
#define SPEECH_CMD_CLOSE		"close"

#endif	// defined( CH_USE_VOXWARE )

#define MAX_VOLUME_SETTING		100

#define SOUND_HTML_DELIM		" ,;\t"


/*----------------------------------------------------------------------------
	Type definitions
----------------------------------------------------------------------------*/

typedef enum { invalid, xch_sound, xch_volume, xch_alert, href,
				xch_speech } CommandType;

											/* The following structure
												associates a pane option
												name with a flag bit */
typedef struct
{
	char*		pstrName;
	chflag32	flOption;

} OptionsNameType;


/*----------------------------------------------------------------------------
	Handler declarations
----------------------------------------------------------------------------*/

CH_DECLARE_MESSAGE_HANDLER( defSoundHandler )
CH_DECLARE_MESSAGE_HANDLER( soundInitHandler )
CH_DECLARE_MESSAGE_HANDLER( soundLoadCompleteHandler )
CH_DECLARE_MESSAGE_HANDLER( soundShowModuleHandler )
#if !defined(CH_PUEBLO_PLUGIN )
CH_DECLARE_MESSAGE_HANDLER( soundGetPageCountHandler )
CH_DECLARE_MESSAGE_HANDLER( soundGetPagesHandler )
CH_DECLARE_MESSAGE_HANDLER( soundGetPageDataHandler )
CH_DECLARE_MESSAGE_HANDLER( soundReleasePagesHandler )
#endif
CH_DECLARE_MESSAGE_HANDLER( soundCommandHandler )
CH_DECLARE_MESSAGE_HANDLER( soundInlineHandler )
CH_DECLARE_MESSAGE_HANDLER( soundAlertHandler )
CH_DECLARE_MESSAGE_HANDLER( soundMediaPlayHandler )
CH_DECLARE_MESSAGE_HANDLER( soundMediaStopHandler )
CH_DECLARE_MESSAGE_HANDLER( soundConnectedHandler )

static ChMsgHandlerDesc	soundHandlers[] =
					{	{CH_MSG_INIT, soundInitHandler},
						{CH_MSG_LOAD_COMPLETE, soundLoadCompleteHandler},
						{CH_MSG_SHOW_MODULE, soundShowModuleHandler},
#if !defined(CH_PUEBLO_PLUGIN )
						{CH_MSG_GET_PAGE_COUNT, soundGetPageCountHandler},
						{CH_MSG_GET_PAGES, soundGetPagesHandler},
						{CH_MSG_GET_PAGE_DATA, soundGetPageDataHandler},
						{CH_MSG_RELEASE_PAGES, soundReleasePagesHandler},
#endif
						{CH_MSG_CMD, soundCommandHandler},
						{CH_MSG_INLINE, soundInlineHandler},
						{CH_MSG_SOUND_ALERT, soundAlertHandler},
						{CH_MSG_MEDIA_PLAY, soundMediaPlayHandler},
						{CH_MSG_MEDIA_STOP, soundMediaStopHandler},
						{CH_MSG_CONNECTED, soundConnectedHandler} };


/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

const OptionsNameType	soundEventsList[] =
									{	{ "complete", soundEvtComplete } };


const OptionsNameType	soundOptionsList[] =
									{	{ "queue", soundOptQueue } };
const OptionsNameType	speechOptionsList[] =
									{	{ "status", speechOptStatus },
										{ "sounds", speechOptSounds },
										{ "query", speechOptQuery } };



/*----------------------------------------------------------------------------
	ChSoundQueue public methods
----------------------------------------------------------------------------*/

void ChSoundQueue::AddItem( DeviceType deviceType, const ChSoundInfo& info )
{
	ChList<ChSoundInfo>*	pDevQueue = GetDeviceQueue( deviceType );

	pDevQueue->AddTail( info );
}


bool ChSoundQueue::GetNextItem( DeviceType deviceType, ChSoundInfo& info )
{
	ChList<ChSoundInfo>*	pDevQueue = GetDeviceQueue( deviceType );
	bool					boolFound;

	if (boolFound = !pDevQueue->IsEmpty())
	{
		info = pDevQueue->RemoveHead();
	}

	return boolFound;
}


/*----------------------------------------------------------------------------
	ChSoundMainInfo public methods
----------------------------------------------------------------------------*/

ChSoundMainInfo::ChSoundMainInfo( const ChModuleID& idModule, ChCore* pCore,
										const ChString& strLoadParam ) :
					ChMainInfo(  idModule,  pCore),
					m_reg( SOUND_PREFS_GROUP ),
					m_idWorldModule( 0 ),
					m_soundDispatcher( pCore, idModule, defSoundHandler ),
					m_strInitialCommand( strLoadParam ),
					m_suMidiVolume( 100 ),
					m_suWaveVolume( 100 ),
					m_boolShown( false ),
					m_boolMenus( false ),
					m_boolHooksInstalled( false ),
					m_boolMidiDeviceInUse( false ),
					m_boolWaveDeviceInUse( false ),
					m_midiPlayer( devMidi, &m_boolMidiDeviceInUse ),
					m_wavePlayer( devWave, &m_boolWaveDeviceInUse ),
					#if defined( CH_USE_VOXWARE )
					m_speechPlayer( devSpeech, &m_boolWaveDeviceInUse ),
					m_suSpeechVolume( 100 ),
					m_pSpeechStatus( 0 ),
					#endif	// defined( CH_USE_VOXWARE )
					m_lastAlertTime(),
					m_alertTime( &m_reg )
{
	chint32		lVolume;
	ChString		strDefAlertSound;
											// Set the sound stream

	m_pSoundStream = new ChSoundStreamManager( this ); 
	ASSERT( m_pSoundStream );

	RegisterDispatchers();

	m_midiPlayer.SetMainInfo( this );
	m_wavePlayer.SetMainInfo( this );

	#if defined( CH_USE_VOXWARE )
	{
		m_speechPlayer.SetMainInfo( this );
		m_pSpeechStatus = new ChSpeechStatus( this );
		m_pTNT = new ChTNT( m_pSpeechStatus );
	}
	#endif	// defined( CH_USE_VOXWARE )

	m_reg.Read( SOUND_PREFS_MUSIC_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suMaxMidi = (chuint16)lVolume;

	m_reg.Read( SOUND_PREFS_EFFECTS_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suMaxWave = (chuint16)lVolume;

	m_reg.Read( SOUND_PREFS_ALERT_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suAlertVolume = (chuint16)lVolume;

	GetMidiPlayer()->SetVolume( m_suMaxMidi );
	GetWavePlayer()->SetVolume( m_suMaxWave );

	#if defined( CH_USE_VOXWARE )
	{
		m_reg.Read( SOUND_PREFS_SPEECH_VOLUME, lVolume, VOLUME_MAX_RANGE );
		m_suMaxSpeech = (chuint16)lVolume;

		GetSpeechPlayer()->SetVolume( m_suMaxSpeech );
	}
	#endif	// defined( CH_USE_VOXWARE )
											/* Pick a reasonable default alert
												sound */

	strDefAlertSound = GetSysSoundFilesPath() + "Tada.wav";
	m_reg.Read( SOUND_PREFS_ALERT_SOUND, m_strAlertSoundPath,
				strDefAlertSound );
											// Read whether we're disabled

	m_reg.ReadBool( SOUND_PREFS_DISABLED, m_boolDisableSound,
					SOUND_PREFS_DISABLED_DEF );
											// See if we have any sound devices

	m_boolSoundDeviceFound = (midiOutGetNumDevs() || waveOutGetNumDevs());

											// Get initial volume settings
	UpdateVolume();

	#if defined( CH_USE_VOXWARE )
	{
		UpdateSpeechVolume();
	}
	#endif	// defined( CH_USE_VOXWARE )
}


ChSoundMainInfo::~ChSoundMainInfo()
{
											// Stop any playing sounds
	GetMidiPlayer()->Stop();
	GetWavePlayer()->Stop();

	#if defined( CH_USE_VOXWARE )
	{
		GetSpeechPlayer()->Stop();
		
		delete m_pTNT;
		m_pTNT = 0;
	}
	#endif	// defined( CH_USE_VOXWARE )

	if (m_pSoundStream)
	{
		delete m_pSoundStream;
		m_pSoundStream = 0;
	}

	DestroyMenus();
											// Let go of the world module
	UnloadWorldModule();
}


void ChSoundMainInfo::Initialize()
{
	CreateMenus();							// Create & Install menus

											// Load the ChWorld module
	LoadWorldModule();
}


void ChSoundMainInfo::ShowModule( bool boolShow )
{
	if (boolShow && !IsShown())
	{
		InstallMenus();
		InstallHooks();

#if defined( CH_USE_VOXWARE )
		m_pSpeechStatus->Show( boolShow );
		SetShown( boolShow );
#endif
	}
	else if (!boolShow && IsShown())
	{
											// Stop any playing music
		GetMidiPlayer()->Stop();

		UninstallMenus();
		InstallHooks( false );

#if defined( CH_USE_VOXWARE )
		m_pSpeechStatus->Show( boolShow );
		SetShown( boolShow );
#endif
	}
}


bool ChSoundMainInfo::DoCommand( const ChString& strArgs, bool boolInline )
{
	bool		boolProcessed;
	CommandType	command = invalid;
	ChString		strCommand;
	ChString		strDevice;

	if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCH_SOUND, strCommand ))
	{
		command = xch_sound;
		strCommand.MakeLower();
	}
	else if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCH_VOLUME, strCommand ))
	{
		command = xch_volume;
	}
	else if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCH_ALERT, strCommand ))
	{
		command = xch_alert;
	}

	#if defined( CH_USE_VOXWARE )

	else if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCH_SPEECH, strCommand ))
	{
		command = xch_speech;
		strCommand.MakeLower();
	}

	#endif	// defined( CH_USE_VOXWARE )
											/* Device is pretty common, so look
												for it here */

	if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_XCH_DEVICE, strDevice ))
	{
		strDevice.MakeLower();
	}

	switch( command )
	{
		case xch_sound:
		{
			ChString		strURL;
			ChString		strVolume;
			bool		boolVolume;
			chuint16	suVolume;
			ChString		strEvents;
			chflag32	flEvents = 0;
			ChString		strOptions;
			chflag32	flOptions = 0;
			ChString		strAction;
			ChString		strMD5;

			ChHtmlWnd::GetHTMLHref( strArgs, boolInline, strURL );
			if (boolVolume = ChHtmlWnd::GetHTMLAttribute( strArgs,
															ATTR_XCH_VOLUME,
															strVolume ))
			{
				boolVolume = GetVolume( strVolume, suVolume );
			}

			if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_OPTIONS, strOptions ))
			{
				flOptions = ParseOptions( strOptions );
			}

			if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_EVENT, strEvents ))
			{
				if (flEvents = ParseEvents( strEvents ))
				{
					if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_MD5,
														strMD5 ))
					{
						if (!ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_ACTION,
															strAction ))
						{
											// No action, so ignore MD5 attrib
							strMD5 = "";
						}
					}
				}
			}

			boolProcessed = DoSoundCommand( strCommand, strURL, strDevice,
											boolVolume, suVolume, flOptions,
											flEvents, strAction, strMD5 );
			break;
		}

		case xch_volume:
		{
			ChString		strURL;
			chuint16	suVolume;
			DeviceType	device = ParseDevice( strDevice );

			if (GetVolume( strCommand, suVolume ))
			{
				boolProcessed = DoVolumeCommand( device, suVolume );
			}
			else
			{
				boolProcessed = false;
			}
			break;
		}

		case xch_alert:
		{
			boolProcessed = DoAlertCommand();
			break;
		}

		#if defined( CH_USE_VOXWARE )

		case xch_speech:
		{
			ChString		strHost;
			chuint16	suPort;
			ChString		strCallId;
			chflag32	flOptions = 0;
			chflag32	flRemoteOptions = 0;
			int			iIndex;
			ChString		strOptions;
			ChString		strRemoteOptions;

			ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_ID, strCallId );
			ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_HOST, strHost );
			iIndex = strHost.Find( ':' );
			if (-1 == iIndex)
			{
				suPort = 0;
			}
			else
			{
				const char*	pstrPort;

				pstrPort = ((const char*)strHost) + iIndex + 1;
				suPort = atoi( pstrPort );

				strHost = strHost.Left( iIndex );
			}

			if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_OPTIONS,
												strOptions ))
			{
				flOptions = ParseSpeechOptions( strOptions );
			}

			if (ChHtmlWnd::GetHTMLAttribute( strArgs, ATTR_REMOTE_OPTIONS,
												strRemoteOptions ))
			{
				flRemoteOptions = ParseSpeechOptions( strRemoteOptions );
			}

			boolProcessed = DoSpeechCommand( strCommand, strHost,
											suPort, strCallId, flOptions,
											flRemoteOptions );
			break;
		}

		#endif	// defined( CH_USE_VOXWARE )

		default:
		{
			boolProcessed = false;
			break;
		}
	}

	return boolProcessed;
}


void ChSoundMainInfo::Play( ChString strURL, bool boolLooping )
{
	if (UseSound())
	{
		ChSoundInfo*	pSoundInfo;
		pSoundInfo = new ChSoundInfo( boolLooping );

		GetCore()->GetURL( strURL, 0, m_pSoundStream,  (chparam)pSoundInfo );
	}
}


void ChSoundMainInfo::StopAll()
{
	GetMidiPlayer()->Stop();
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSoundMainInfo::DoAlertCommand

	RETURNS		||  true if the command was processed.

------------------------------------------------------------------------------

	This method will perform a sound command.

----------------------------------------------------------------------------*/

bool ChSoundMainInfo::DoAlertCommand()
{
	if (UseSound())
	{
		ChTime		timeNow( ChTime::GetCurrentTime() );
		ChTimeSpan	span;

		span = timeNow - m_lastAlertTime;

		if (span.GetTotalSeconds() > m_alertTime.GetAlertSeconds())
		{
			m_lastAlertTime = timeNow;
											/* Make sure we're not getting
												alerts too frequently */
			if (!GetWavePlayer()->IsPlaying())
			{
				if (!m_strAlertSoundPath.IsEmpty())
				{
					ChSoundInfo		info( false, true, GetAlertVolume() );

					info.SetFilename( m_strAlertSoundPath );

					GetWavePlayer()->Play( &info );
				}
			}
		}
	}

	return true;
}


void ChSoundMainInfo::SendWorldCommand( const ChString& strMD5,
										const ChString& strAction,
										const ChString& strParams )
{
	ChSendWorldCmdMsg*	pMsg;

	pMsg = new ChSendWorldCmdMsg( PUEBLO_SOUND_CMD, strMD5, strAction,
									strParams );
	ASSERT( pMsg );

	GetCore()->DispatchMsg( GetWorldID(), *pMsg );
	delete pMsg;
}


void ChSoundMainInfo::OnPlayComplete( DeviceType deviceType )
{
	ASSERT( devAll != deviceType );
											/* Play the next sound in the
												queue */
	PlayFromQueue( deviceType );
}


/*----------------------------------------------------------------------------
	ChSoundMainInfo protected methods
----------------------------------------------------------------------------*/


void ChSoundMainInfo::SetWorldID( const ChModuleID& idModule )
{
	m_idWorldModule = idModule;
}


void ChSoundMainInfo::HandleSoundFile( const ChString& strFile, int iMimeType,
										ChSoundInfo* pSoundInfo )
{
	bool		boolVolume;
	chuint16	suVolume;
	bool		boolPlaying;

	ASSERT( pSoundInfo );

	boolVolume = pSoundInfo->IsVolumeValid();
	suVolume = pSoundInfo->GetVolume();

	pSoundInfo->SetFilename( strFile );

	switch( iMimeType )
	{
		case ChHTTPConn::typeMidi:
		{
			if (boolVolume)
			{								/* Adjust the volume for this
												device */

				suVolume = GetScaledVolume( suVolume, GetMaxMidi() );
				pSoundInfo->SetVolume( suVolume );
			}
											/* Set the device into the sound
												info object */

			pSoundInfo->SetDeviceType( devMidi );
			boolPlaying = m_boolMidiDeviceInUse;
			break;
		}

		case ChHTTPConn::typeWave:
		{
			if (boolVolume)
			{								/* Adjust the volume for this
												device */

				suVolume = GetScaledVolume( suVolume, GetMaxWave() );
				pSoundInfo->SetVolume( suVolume );
			}
											/* Set the device into the sound
												info object */

			pSoundInfo->SetDeviceType( devWave );
			boolPlaying = m_boolWaveDeviceInUse;
			break;
		}

		#if defined( CH_USE_VOXWARE )

		case ChHTTPConn::typeVox:
		{
			if (boolVolume)
			{								/* Adjust the volume for this
												device */

				suVolume = GetScaledVolume( suVolume, GetMaxSpeech() );
				pSoundInfo->SetVolume( suVolume );
			}
											/* Set the device into the sound
												info object */

			pSoundInfo->SetDeviceType( devSpeech );
			boolPlaying = m_boolWaveDeviceInUse;
			break;
		}

		#endif	// defined( CH_USE_VOXWARE )

		default:
		{
			break;
		}
	}

	if (boolPlaying && (pSoundInfo->GetOptions() & soundOptQueue))
	{
											/* Add the item to the correct
												device queue */

		GetDeviceQueue()->AddItem( pSoundInfo->GetDeviceType(), *pSoundInfo );
	}
	else
	{										// Play the sound immediately
		DoPlay( pSoundInfo );
	}
}


void ChSoundMainInfo::PlayFromQueue( DeviceType queueDevType )
{
	ChSoundInfo		info;

	ASSERT( devAll != queueDevType );

	if (!GetDeviceQueue()->IsEmpty( queueDevType ))
	{
		GetDeviceQueue()->GetNextItem( queueDevType, info );
		ASSERT( devAll != info.GetDeviceType() );

		DoPlay( &info );
	}
}


bool ChSoundMainInfo::DoPlay( const ChSoundInfo* pInfo )
{
	bool	boolSuccess;
											// Stop any playing sounds

	if (devMidi == pInfo->GetDeviceType())
	{
		GetMidiPlayer()->Stop();
	}
	else
	{
		GetWavePlayer()->Stop();

		#if defined( CH_USE_VOXWARE )
		{
			GetSpeechPlayer()->Stop();
		}
		#endif	// defined( CH_USE_VOXWARE )
	}
											// Play the requested sound
	switch( pInfo->GetDeviceType() )
	{
		case devMidi:
		{
			boolSuccess = GetMidiPlayer()->Play( pInfo );
			break;
		}

		case devWave:
		{
			boolSuccess = GetWavePlayer()->Play( pInfo );
			break;
		}

		#if defined( CH_USE_VOXWARE )

		case devSpeech:
		{
			boolSuccess = GetSpeechPlayer()->Play( pInfo );
			break;
		}

		#endif	// defined( CH_USE_VOXWARE )

		default:
		{
			break;
		}
	}

	return boolSuccess;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSoundMainInfo::InstallHooks

	boolInstall	||	'true' to install the hooks, and false to uninstall them.

------------------------------------------------------------------------------

	This method will install or uninstall the hooks into other modules.

----------------------------------------------------------------------------*/

void ChSoundMainInfo::InstallHooks( bool boolInstall )
{
	if (boolInstall != m_boolHooksInstalled)
	{										// We're changing state
		if (boolInstall)
		{
			InstallHook( CH_MSG_CMD, GetModuleID() );
			InstallHook( CH_MSG_INLINE, GetModuleID() );

			m_boolHooksInstalled = true;
		}
		else
		{
			UninstallHook( CH_MSG_CMD, GetModuleID() );
			UninstallHook( CH_MSG_INLINE, GetModuleID() );

			m_boolHooksInstalled = false;
		}
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSoundMainInfo::DoSoundCommand

	strCommand	||	Sound command to perform.

	strURL		||	URL parameter to sound command.  (May be empty.)

	strDevice	||	Indicates the affected device.  May be empty, or one
					of the following values:
						midi
						wave

	boolVolume	||	true if the volume is valid.

	suVolume	||	Volume setting.

	RETURNS		||  true if the command was processed.

------------------------------------------------------------------------------

	This method will perform a sound command.

----------------------------------------------------------------------------*/

bool ChSoundMainInfo::DoSoundCommand( const ChString& strCommand,
										const ChString& strURL,
										const ChString& strDevice,
										bool boolVolume, chuint16 suVolume,
										chflag32 flOptions, chflag32 flEvents,
										const ChString& strAction,
										const ChString& strMD5 )
{
	bool		boolProcessed = true;
	DeviceType	device = ParseDevice( strDevice );
	int			iCommand = ParseSoundCmd( strCommand );

	switch( iCommand )
	{
		case sndPlay:
		case sndLoop:
		{
			if (UseSound())
			{
				if (!strURL.IsEmpty())
				{
					ChSoundInfo*	pSoundInfo;
					bool			boolSuccess;

					pSoundInfo = new ChSoundInfo( (sndLoop == iCommand),
													boolVolume, suVolume,
													flOptions, flEvents,
													strAction, strMD5 );

					boolSuccess = GetCore()->GetURL( strURL, 0, m_pSoundStream,  
												(chparam)pSoundInfo );

					boolProcessed = true;
				}
				else
				{
					TRACE( "Sound played (or looped) without an 'href' field.\n" );
				}
			}
			break;
		}

		case sndStop:
		{
			if ((devAll == device) || (devMidi == device))
			{
				GetMidiPlayer()->Stop();
			}

			if ((devAll == device) || (devWave == device))
			{
				GetWavePlayer()->Stop();
			}
			break;
		}

		case sndStopLoop:
		{
			if ((devAll == device) || (devMidi == device))
			{
				GetMidiPlayer()->StopLoop();
			}

			if ((devAll == device) || (devWave == device))
			{
				GetWavePlayer()->StopLoop();
			}
			break;
		}

		default:
		{
			boolProcessed = false;
			break;
		}
	}

	return boolProcessed;
}


bool ChSoundMainInfo::DoVolumeCommand( DeviceType device,
										chuint16 suVolume )
{
	if (UseSound())
	{										// Set the volume

		if ((devAll == device) || (devMidi == device))
		{
											// Save the unscaled volume
			m_suMidiVolume = suVolume;

			suVolume = GetScaledVolume( m_suMidiVolume, GetMaxMidi() );

			GetMidiPlayer()->SetVolume( suVolume );
		}

		if ((devAll == device) || (devWave == device))
		{
											// Save the unscaled volume
			m_suWaveVolume = suVolume;

			suVolume = GetScaledVolume( m_suWaveVolume, GetMaxWave() );

			GetWavePlayer()->SetVolume( suVolume );
		}

		#if defined( CH_USE_VOXWARE )
		{
			if ((devAll == device) || (devSpeech == device))
			{
											// Save the unscaled volume
				m_suSpeechVolume = suVolume;

				suVolume = GetScaledVolume( m_suSpeechVolume, GetMaxSpeech() );

				GetSpeechPlayer()->SetVolume( suVolume );
			}
		}
		#endif	// defined( CH_USE_VOXWARE )
	}

	return true;
}


bool ChSoundMainInfo::GetVolume( const ChString& strVolume,
									chuint16& suVolume )
{
	bool	boolTranslated;

	if (!strVolume.IsEmpty() && isdigit( strVolume[0] ))
	{
		int			iValue = atoi( strVolume );

											// Range check
		if (iValue <= 0)
		{
			iValue = 0;
		}
		else if (iValue > MAX_VOLUME_SETTING)
		{
			iValue = MAX_VOLUME_SETTING;
		}

		boolTranslated = true;
		suVolume = (chuint16)iValue;
	}
	else
	{
		boolTranslated = false;
	}

	return boolTranslated;
}


void ChSoundMainInfo::UpdateVolume()
{
	chint32		lVolume;
	chuint16	suVolume;

	m_reg.Read( SOUND_PREFS_MUSIC_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suMaxMidi = (chuint16)lVolume;

	m_reg.Read( SOUND_PREFS_EFFECTS_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suMaxWave = (chuint16)lVolume;

	m_reg.Read( SOUND_PREFS_ALERT_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suAlertVolume = (chuint16)lVolume;
											// Now rescale the volume

	suVolume = GetScaledVolume( m_suMidiVolume, GetMaxMidi() );
	GetMidiPlayer()->SetVolume( suVolume );

	suVolume = GetScaledVolume( m_suWaveVolume, GetMaxWave() );
	GetWavePlayer()->SetVolume( suVolume );
}


#if defined( CH_USE_VOXWARE )

void ChSoundMainInfo::UpdateSpeechVolume()
{
	chint32		lVolume;
	chuint16	suVolume;

	m_reg.Read( SOUND_PREFS_SPEECH_VOLUME, lVolume, VOLUME_MAX_RANGE );
	m_suSpeechVolume = (chuint16)lVolume;
											// Now rescale the volume

	suVolume = GetScaledVolume( m_suWaveVolume, GetMaxSpeech() );
	GetSpeechPlayer()->SetVolume( suVolume );
}

#endif	// defined( CH_USE_VOXWARE )

#if defined( CH_USE_VOXWARE )

bool ChSoundMainInfo::DoSpeechCommand( const ChString& strCommand,
										const ChString& strHost,
										chuint16 suPort,
										const ChString& strCallId,
										chflag32 flOptions,
										chflag32 flRemoteOptions )
{
	int			iCommand = ParseSpeechCmd( strCommand );
	bool		boolProcessed = true;

	switch( iCommand )
	{
		case speechCall:
		{
			if (strHost.IsEmpty())
			{
				boolProcessed = false;
			}
			else
			{
				m_pTNT->MakeCall( strCallId, strHost, suPort, flOptions,
									flRemoteOptions );
			}
			break;
		}

		case speechClose:
		{
			m_pTNT->Hangup( strCallId );
			break;
		}

		default:
		{
			boolProcessed = false;
			break;
		}
	}

	return boolProcessed;
}

#endif	// defined( CH_USE_VOXWARE )


/*----------------------------------------------------------------------------
	ChSoundMainInfo private methods
----------------------------------------------------------------------------*/

void ChSoundMainInfo::RegisterDispatchers()
{
	chint16		sHandlerCount = sizeof( soundHandlers ) /
								sizeof( ChMsgHandlerDesc );

	m_soundDispatcher.AddHandler( soundHandlers, sHandlerCount );
}


void ChSoundMainInfo::CreateMenus()
{
	m_boolMenus = true;
}


void ChSoundMainInfo::InstallMenus()
{
}


void ChSoundMainInfo::UninstallMenus()
{
}


void ChSoundMainInfo::DestroyMenus()
{
	m_boolMenus = false;
}


void ChSoundMainInfo::LoadWorldModule()
{
	LoadClientModule( CH_MODULE_WORLD, CH_MODULE_WORLD_BASE,
						GetModuleID() );
}


void ChSoundMainInfo::UnloadWorldModule()
{
	if (m_idWorldModule)
	{
		UnloadClientModule( m_idWorldModule );
		m_idWorldModule = 0;
	}
}


int ChSoundMainInfo::ParseSoundCmd( const ChString& strCommand )
{
	ChString	strCmd( strCommand );

	strCmd.MakeLower();

	if (strCmd == SND_CMD_PLAY)
	{
		return sndPlay;
	}
	else if (strCmd == SND_CMD_LOOP)
	{
		return sndLoop;
	}
	else if (strCmd == SND_CMD_STOP)
	{
		return sndStop;
	}
	else if (strCmd == SND_CMD_STOPLOOP)
	{
		return sndStopLoop;
	}

	return 0;
}


#if defined( CH_USE_VOXWARE )

int ChSoundMainInfo::ParseSpeechCmd( const ChString& strCommand )
{
	ChString	strCmd( strCommand );

	strCmd.MakeLower();

	if (strCmd == SPEECH_CMD_CALL)
	{
		return speechCall;
	}
	else if (strCmd == SPEECH_CMD_CLOSE)
	{
		return speechClose;
	}

	return 0;
}

#endif	// defined( CH_USE_VOXWARE )


DeviceType ChSoundMainInfo::ParseDevice( const ChString& strDevice )
{
	ChString	strDev( strDevice );

	strDev.MakeLower();

	if (strDev == "wave")
	{
		return devWave;
	}
	else if (strDev == "midi")
	{
		return devMidi;
	}

	#if defined( CH_USE_VOXWARE )

	else if (strDev == "speech")
	{
		return devSpeech;
	}

	#endif	// defined( CH_USE_VOXWARE )

	return devAll;
}


chflag32 ChSoundMainInfo::ParseEvents( const ChString& strEvents )
{
	chflag32	flOptions = 0;
	ChString		strWorking( strEvents );

	if (!strEvents.IsEmpty())
	{
		int		iOptionsCount = sizeof( soundEventsList ) /
									sizeof( OptionsNameType );
		char*	pstrCopy;
		char*	pstrToken;

		strWorking.MakeLower();

		pstrCopy = new char[strWorking.GetLength() + 2];
		strcpy( pstrCopy, strWorking );

		pstrToken = strtok( pstrCopy, SOUND_HTML_DELIM );
		while (pstrToken)
		{
			bool	boolFound = false;
			int		iLoop;
			ChString	strTok( pstrToken );

			for (iLoop = 0; (iLoop < iOptionsCount) && !boolFound;
					iLoop++)
			{
				if (strTok == soundEventsList[iLoop].pstrName)
				{
					flOptions |= soundEventsList[iLoop].flOption;
					boolFound = true;
				}
			}

			pstrToken = strtok( 0, SOUND_HTML_DELIM );
		}
		delete [] pstrCopy;
	}

	return flOptions;
}


chflag32 ChSoundMainInfo::ParseOptions( const ChString& strOptions )
{
	chflag32	flOptions = 0;
	ChString		strWorking( strOptions );

	if (!strOptions.IsEmpty())
	{
		int		iOptionsCount = sizeof( soundOptionsList ) /
									sizeof( OptionsNameType );
		char*	pstrCopy;
		char*	pstrToken;

		strWorking.MakeLower();

		pstrCopy = new char[strWorking.GetLength() + 2];
		strcpy( pstrCopy, strWorking );

		pstrToken = strtok( pstrCopy, SOUND_HTML_DELIM );
		while (pstrToken)
		{
			bool	boolFound = false;
			int		iLoop;
			ChString	strTok( pstrToken );

			for (iLoop = 0; (iLoop < iOptionsCount) && !boolFound;
					iLoop++)
			{
				if (strTok == soundOptionsList[iLoop].pstrName)
				{
					flOptions |= soundOptionsList[iLoop].flOption;
					boolFound = true;
				}
			}

			pstrToken = strtok( 0, SOUND_HTML_DELIM );
		}
		delete [] pstrCopy;
	}

	return flOptions;
}


chflag32 ChSoundMainInfo::ParseSpeechOptions( const ChString& strOptions )
{
	chflag32	flOptions = 0;
	ChString		strWorking( strOptions );

	if (!strOptions.IsEmpty())
	{
		int		iOptionsCount = sizeof( speechOptionsList ) /
									sizeof( OptionsNameType );
		char*	pstrCopy;
		char*	pstrToken;

		strWorking.MakeLower();

		pstrCopy = new char[strWorking.GetLength() + 2];
		strcpy( pstrCopy, strWorking );

		pstrToken = strtok( pstrCopy, SOUND_HTML_DELIM );
		while (pstrToken)
		{
			bool	boolFound = false;
			int		iLoop;
			ChString	strTok( pstrToken );

			for (iLoop = 0; (iLoop < iOptionsCount) && !boolFound;
					iLoop++)
			{
				if (strTok == speechOptionsList[iLoop].pstrName)
				{
					flOptions |= speechOptionsList[iLoop].flOption;
					boolFound = true;
				}
			}

			pstrToken = strtok( 0, SOUND_HTML_DELIM );
		}
		delete [] pstrCopy;
	}

	return flOptions;
}


/*----------------------------------------------------------------------------
	Chaco module library entry point
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
CH_EXTERN_LIBRARY( void )
InitChSoundDLL();
#endif

#ifdef __linux__
CH_IMPLEMENT_MAIN_HANDLER( ChMainEntryMidi )
#else
#if defined( CH_PUEBLO_PLUGIN )
STDAPI_(int) 				
ChMainEntrySound( ChMsg &msg, ChCore *pCore, ChMainInfo *pMainInfo, 
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
				InitChSoundDLL();
			}
			#endif	// defined( CH_MSW ) && defined( CH_ARCH_16 )

			pMsg->GetParams( idModule, strLoadParam, idServerModule );

			if (*pstrModule == CH_MODULE_SOUND)
			{
				ChSoundMainInfo	*pMainInfo;

				pMainInfo = new ChSoundMainInfo( idModule, pCore, strLoadParam );

				retVal = (chparam)pMainInfo;
			}
			break;
		}

		case CH_MSG_TERM:
		{
			TRACE("SOUND: About to delete MainInfo... ");
			if(pMainInfo) {
				delete pMainInfo;
				TRACE("deleted MainInfo\n");
			} else
				TRACE("there wasn't any MainInfo!\n");
			break;
		}
	}

	return retVal;
}


/*----------------------------------------------------------------------------
	Chaco message handlers
----------------------------------------------------------------------------*/

CH_IMPLEMENT_MESSAGE_HANDLER( defSoundHandler )
{
	chparam		retVal = 0;

	return retVal;
}

CH_IMPLEMENT_MESSAGE_HANDLER( soundInitHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;

	pInfo->Initialize();
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundLoadCompleteHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChLoadCompleteMsg*	pMsg = (ChLoadCompleteMsg*)&msg;
	ChString				strModuleName;
	ChModuleID			idModule;
	ChString				strFilename;
	chparam				userData;

	pMsg->GetParams( strModuleName, idModule, strFilename, userData );

	if (0 == idModule)
	{
		ChSoundInfo*	pSoundInfo;
		int				iMimeType;

		pSoundInfo = (ChSoundInfo*)userData;
		ASSERT( pSoundInfo );

		iMimeType = ChHTTPConn::GetMimeType( pMsg->GetType() );
		pInfo->HandleSoundFile( strFilename, iMimeType, pSoundInfo );

		delete pSoundInfo;
	}
	else
	{										// Module load complete...

		if (CH_MODULE_WORLD == strModuleName)
		{
			pInfo->SetWorldID( idModule );
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundShowModuleHandler )
{
	ChShowModuleMsg*	pMsg = (ChShowModuleMsg*)&msg;
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;

	pInfo->ShowModule( pMsg->IsShowing() );
	return 0;
}

#if !defined( CH_PUEBLO_PLUGIN )

CH_IMPLEMENT_MESSAGE_HANDLER( soundGetPageCountHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChGetPageCountMsg*	pMsg = (ChGetPageCountMsg*)&msg;
	ChPageType			type;
	int					iPageCount;

	pMsg->GetParams( type );

//	if (pInfo->IsShown())
//	{
		switch( type )
		{
			case pagePreferences:
			{
				#if defined( CH_USE_VOXWARE )
				{
					iPageCount = 2;
				}
				#else	// defined( CH_USE_VOXWARE )
				{
					iPageCount = 1;
				}
				#endif	// defined( CH_USE_VOXWARE )
				break;
			}

			default:
			{
				iPageCount = 0;
				break;
			}
		}
//	}
//	else
//	{
//		iPageCount = 0;
//	}

	return iPageCount;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundGetPagesHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChGetPagesMsg*		pMsg = (ChGetPagesMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			#if defined( CH_USE_VOXWARE )
			{
				ASSERT( 2 == sCount );
			}
			#else	// defined( CH_USE_VOXWARE )
			{
				ASSERT( 1 == sCount );
			}
			#endif	// defined( CH_USE_VOXWARE )

			#if defined( CH_MSW )
			{
				ChSoundPrefs*	pPage;

				pPage = new ChSoundPrefs();

											// Set data into dialog
				pPage->SetMainInfo( pInfo );
				//pPage->SetDeviceFound( pInfo->SoundDeviceFound() );

				pPages[0] = (chparam)pPage;
			}
			#endif

			#if defined( CH_USE_VOXWARE )
			{
				if (pPages[1])
				{
					ChSpeechPrefs*	pPage;

					pPage = new ChSpeechPrefs();

											// Set data into dialog
					pPage->SetMainInfo( pInfo );

					pPages[1] = (chparam)pPage;
				}
			}
			#endif	// defined( CH_USE_VOXWARE )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundGetPageDataHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChGetPageDataMsg*	pMsg = (ChGetPageDataMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			#if defined( CH_USE_VOXWARE )
			{
				ASSERT( 2 == sCount );
			}
			#else	// defined( CH_USE_VOXWARE )
			{
				ASSERT( 1 == sCount );
			}
			#endif	// defined( CH_USE_VOXWARE )

			#if defined( CH_MSW )
			{
				if (pPages[0])
				{
					ChSoundPrefs*	pPage = (ChSoundPrefs*)pPages[0];

					pPage->OnCommit();
				}

				#if defined( CH_USE_VOXWARE )
				{
					if (pPages[1])
					{
						ChSpeechPrefs*	pPage = (ChSpeechPrefs*)pPages[1];

						pPage->OnCommit();
					}
				}
				#endif	// defined( CH_USE_VOXWARE )
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


CH_IMPLEMENT_MESSAGE_HANDLER( soundReleasePagesHandler )
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
			#if defined( CH_USE_VOXWARE )
			{
				ASSERT( 2 == sCount );
			}
			#else	// defined( CH_USE_VOXWARE )
			{
				ASSERT( 1 == sCount );
			}
			#endif	// defined( CH_USE_VOXWARE )

			#if defined( CH_MSW )
			{
				if (pPages[0])
				{
					ChSoundPrefs*	pPage = (ChSoundPrefs*)pPages[0];

					delete pPage;
				}

				#if defined( CH_USE_VOXWARE )
				{
					if (pPages[1])
					{
						ChSpeechPrefs*	pPage = (ChSpeechPrefs*)pPages[1];

						delete pPage;
					}
				}
				#endif	// defined( CH_USE_VOXWARE )
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

#endif // #if !defined(CH_PUEBLO_PLUGIN )


CH_IMPLEMENT_MESSAGE_HANDLER( soundCommandHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChCmdMsg*			pMsg = (ChCmdMsg*)&msg;
	ChString				strArgs;

	pMsg->GetParams( strArgs );
	if (!strArgs.IsEmpty())
	{
		bool	boolProcessed;

		boolProcessed = pInfo->DoCommand( strArgs, false );
		pMsg->SetProcessed( boolProcessed );
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundInlineHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChInlineMsg*		pMsg = (ChInlineMsg*)&msg;
	ChString				strArgs;

	pMsg->GetParams( strArgs );
	if (!strArgs.IsEmpty())
	{
		bool	boolProcessed;

		boolProcessed = pInfo->DoCommand( strArgs, true );
		pMsg->SetProcessed( boolProcessed );
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundAlertHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;

	pInfo->DoAlertCommand();
	msg.SetProcessed();

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundMediaPlayHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
	ChMediaPlayMsg*		pMsg = (ChMediaPlayMsg*)&msg;
	ChString				strURL;
	bool				boolLooping;

	pMsg->GetParams( strURL, boolLooping );
	pInfo->Play( strURL, boolLooping );
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundMediaStopHandler )
{
	ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;

	pInfo->StopAll();

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( soundConnectedHandler )
{
	#if defined( CH_USE_VOXWARE )
	{
		ChSoundMainInfo*	pInfo = (ChSoundMainInfo*)pMainInfo;
		ChConnectedMsg*		pMsg = (ChConnectedMsg*)&msg;
		ChString				strPort;

		strPort.Format( "%d", (int)pInfo->GetSpeechPort() );

		pMsg->AppendPuebloClientParam( TNT_PUEBLOSOUND_PARAM, strPort );
	}
	#endif	// defined( CH_USE_VOXWARE )

	return 0;
}


/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_GLOBAL_FUNC( ChString )
GetSysSoundFilesPath()
{
	char		dir[MAX_PATH + 100];
	ChString		strPath;

	GetWindowsDirectory( dir, sizeof( dir ) );
	strPath = dir;
	strPath += "\\";

	OSType osType = ChCore::GetClientInfo()->GetPlatform();
	if ((osWin95 == osType) || (osWin98 == osType) || (osWinXP == osType))
	{
		/* Win95 sounds are in the Media
			directory, as are WinXP's */
		strPath += "Media\\";
	}

	return strPath;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:01  uecasm
// Import of source tree as at version 2.53 release.
//
