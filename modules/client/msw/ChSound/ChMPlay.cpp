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

	Implementation of the ChMediaPlayer class.  This class knows how to
	load and play MIDI music files as well as Wave audio files.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
											// Set Toolvox target...
#if defined( CH_MSW )
	#if defined( WIN32 )
		#define VOXWARE_WIN32
	#else	// defined( WIN32 )
		#define VOXWARE_WIN16
	#endif	// defined( WIN32 )
#else	// defined( CH_MSW )
	#error Voxware platform not defined!
#endif	// defined( CH_MSW )

#if defined( CH_USE_VOXWARE )

	#include <tnt.h>						// Toolvox (Voxware) includes

#endif	// defined( CH_USE_VOXWARE )

#include <ChCore.h>

#include "ChMPlay.h"
#include "ChSoundInfo.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define WAVE_BUFFER_SECS		2

//#define WAVE_DEV_NAME			"mciwave"
//#define MIDI_DEV_NAME			"mciseq"

#define WAVE_DEV_NAME			"audiowave"
#define MIDI_DEV_NAME			"seqencer"


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

#if 0

VOXAPI_CALLBACK VoxwareCallbackFunc( unsigned short wVox,
										unsigned short wMessage,
										LPVOXWARE_DATA lpVoxwareData );

#endif	// defined( CH_USE_VOXWARE )


/*----------------------------------------------------------------------------
	ChMediaPlayer public methods
----------------------------------------------------------------------------*/

ChMediaPlayer::ChMediaPlayer( DeviceType deviceType,
								bool* pboolDeviceInUseFlag ) :
					m_pMainInfo( 0 ),
					m_pboolDeviceInUseFlag( pboolDeviceInUseFlag ),
					m_boolExists( true ),
					m_deviceType( deviceType ),
					m_pMCIObject( 0 ),
					m_boolLoopVolume( false ),
					m_suVolume( 0xffff ),
					m_pInfo( 0 )
{
	ASSERT( pboolDeviceInUseFlag );

	GetNotifyWnd()->Create();
	GetNotifyWnd()->SetPlayer( this );

	m_pInfo = new ChSoundInfo;
	ASSERT( GetSoundInfo() );

	m_pMixer = new ChMixer( deviceType );
	m_boolExists = DeviceExists( deviceType );
}


ChMediaPlayer::~ChMediaPlayer()
{
	if (IsDeviceOpen())
	{
		CloseDevice();
	}

	if (m_pMixer)
	{
		delete m_pMixer;
	}

	if (GetSoundInfo())
	{
		delete m_pInfo;
		m_pInfo = 0;
	}

	GetNotifyWnd()->DestroyWindow();
}


bool ChMediaPlayer::Play( const ChSoundInfo* pInfo )
{
	bool	boolSuccess;

	if (IsDeviceOpen())
	{
		CloseDevice();
	}

	OpenDevice();

	boolSuccess = GetDevice()->OpenFile( pInfo->GetFilename() );
	if (!boolSuccess)
	{
		CloseDevice();
	}
	else
	{
		PrepAndPlay( pInfo );
	}

	return boolSuccess;
}


void ChMediaPlayer::Stop()
{
	if (!IsDeviceOpen())
	{
		return;
	}
											/* Set the event flags to zero
												so that no notification
												occurs (since this object
												is being explicitly stopped) */
	SetEventInfo( 0 );
											// Turn off looping!
	SetLooping( false );
											// Stop the device
	GetDevice()->Stop();
	SetPlaying( false );
}


void ChMediaPlayer::SetVolume( chuint16 suVolume )
{
	m_suVolume = suVolume;

	if (IsPlaying())
	{
		GetMixer()->SetVolume( m_suVolume );
	}
}


/*----------------------------------------------------------------------------
	ChMediaPlayer protected methods
----------------------------------------------------------------------------*/

bool ChMediaPlayer::PrepAndPlay( const ChSoundInfo* pInfo )
{
	ASSERT( pInfo );

	*m_pInfo = *pInfo;
	ASSERT( 0 != GetSoundInfo() );

	SetEventInfo( GetSoundInfo()->GetEvents(), GetSoundInfo()->GetAction(),
					GetSoundInfo()->GetMD5() );

	if (GetSoundInfo()->IsVolumeValid())
	{
		SetLooping( GetSoundInfo()->IsLooping(), GetSoundInfo()->GetVolume() );
		GetMixer()->SetVolume( GetSoundInfo()->GetVolume() );
	}
	else
	{
		SetLooping( GetSoundInfo()->IsLooping() );
		GetMixer()->SetVolume( m_suVolume );
	}

	SetPlaying( true );
	return DoPlay();
}


bool ChMediaPlayer::DoPlay()
{
	GetDevice()->Play();

	return true;
}


bool ChMediaPlayer::DeviceExists( DeviceType deviceType )
{
	bool		boolExists;

	switch( deviceType )
	{
		case devMidi:
		{
			boolExists = 0 < midiOutGetNumDevs();
			break;
		}

		case devWave:
		case devSpeech:
		{
			boolExists = 0 < waveOutGetNumDevs();
			break;
		}

		default:
		{
			TRACE( "ChMediaPlayer::DeviceExists:  Unknown device.\n" );
			boolExists = false;
			break;
		}
	}

	return boolExists;
}


void ChMediaPlayer::OpenDevice()
{
	m_pMCIObject = new ChMCIObject( GetDeviceType(), GetNotifyWnd() );
	ASSERT( 0 != m_pMCIObject );
}


void ChMediaPlayer::CloseDevice()
{
	if (m_pMCIObject)
	{
		m_pMCIObject->Close();

		delete m_pMCIObject;
		m_pMCIObject = 0;
	}
}


void ChMediaPlayer::OnPlayComplete()
{
	if (IsLooping())
	{
		Play( GetSoundInfo() );
	}
	else
	{
		CloseDevice();
		SetPlaying( false );

		if (GetEvents() & soundEvtComplete)
		{									// Send completion notification
			
			GetMainInfo()->SendWorldCommand( GetMD5(), GetAction(),
												PUEBLO_SOUND_COMPLETE );
		}

		GetMainInfo()->OnPlayComplete( GetDeviceType() );
	}
}


/*----------------------------------------------------------------------------
	ChMCIObject public methods
----------------------------------------------------------------------------*/

ChMCIObject::ChMCIObject( DeviceType deviceType, CWnd* pwndNotify ) :
				m_deviceType( deviceType ),
				m_pwndNotify( pwndNotify )
{
	ClearDevice();
}


ChMCIObject::~ChMCIObject()
{
    if (IsOpen())
    {
        Close();
    }

    ASSERT( !IsOpen() );
}


bool ChMCIObject::OpenFile( const ChString& strFilename )
{
    chuint32		luResult;
	DWORD			dwOpenOptions = MCI_WAIT |
									MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID |
									MCI_OPEN_ELEMENT;
    if (IsOpen())
    {
        Close();
    }
    ASSERT( !IsOpen() );

	ClearDevice();							// Zero out the open structure

	switch( GetDeviceType() )
	{
		case devMidi:
		{
			SetDeviceType( (const char*)MCI_DEVTYPE_SEQUENCER );
			break;
		}

		case devWave:
		{
			SetDeviceType( (const char*)MCI_DEVTYPE_WAVEFORM_AUDIO );

			dwOpenOptions |= MCI_WAVE_OPEN_BUFFER;
			break;
		}

		default:
		{
			TRACE( "ChMCIObject::OpenFile:  Unknown device.  "
					"Trying to make do.\n" );

			SetDeviceType( 0 );
			dwOpenOptions &= ~((DWORD)MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID );
			break;
		}
	}

	m_openParams.lpstrElementName = strFilename;
	m_openParams.dwBufferSeconds = WAVE_BUFFER_SECS;

    luResult = mciSendCommand( 0, MCI_OPEN, dwOpenOptions,
                              	(chparam)(void*)&m_openParams );
    if (luResult != 0)
    {
        MCIError( luResult );
		ClearDevice();						// Zero out the open structure
        return false;
    }
    										/* Set the time format to
    											milliseconds */
    MCI_SET_PARMS		set;

    set.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	luResult = mciSendCommand( GetDeviceID(), MCI_SET,
                              	MCI_WAIT | MCI_SET_TIME_FORMAT,
                              	(chparam)(void*)&set );
	if (luResult != 0)
	{
		MCIError( luResult );
		ClearDevice();						// Zero out the open structure
		return false;
	}
											/* Attempt to cue the file so it
												will play with no delay */
	MCI_GENERIC_PARMS	generic;

	ChMemClearStruct( &generic );
	luResult = mciSendCommand( GetDeviceID(), MCI_CUE, MCI_WAIT,
								(chparam)(void*)&generic );
	return true;
}


void ChMCIObject::Close()
{
    MCI_GENERIC_PARMS	generic;
    chuint32			luResult;

    if (!IsOpen())
    {
    	return;								// Already closed.
	}

    Stop();									// Just in case.

    luResult = mciSendCommand( GetDeviceID(), MCI_CLOSE, MCI_WAIT,
								(chparam)(void*)&generic );
    if (luResult != 0)
    {
        MCIError( luResult );
    }

	ClearDevice();							// Zero out the open structure
}


void ChMCIObject::Play()
{
    MCI_PLAY_PARMS	play;
    chuint32		luResult;

    if (!IsOpen())
    {
    	return;								// Not open
	}

    mciSendCommand( GetDeviceID(), MCI_SEEK,
                   	MCI_WAIT | MCI_SEEK_TO_START, 0 );

											// Play music

	play.dwCallback = (DWORD)m_pwndNotify->m_hWnd;
    luResult = mciSendCommand( GetDeviceID(), MCI_PLAY, MCI_NOTIFY,
								(chparam)(void*)&play );
    if (luResult != 0)
    {
        MCIError( luResult );
    }
}


void ChMCIObject::Stop()
{
    chuint32		luResult;

    if (!IsOpen())
	{
    	return;								// Not open
	}

	luResult = mciSendCommand( GetDeviceID(), MCI_STOP, MCI_WAIT, 0 );
    if (luResult != 0)
    {
        MCIError( luResult );
    }
}


chuint32 ChMCIObject::GetPosition()
{
    if (!IsOpen())
	{
    	return 0;							// Not open
	}

    MCI_STATUS_PARMS	status;

    status.dwItem = MCI_STATUS_POSITION;
    if (mciSendCommand( GetDeviceID(), MCI_STATUS,
						MCI_WAIT | MCI_STATUS_ITEM,
                       	(chparam)(void*)&status) != 0)
	{
		return 0;							// Some error
    }

    return status.dwReturn;
}


/*----------------------------------------------------------------------------
	ChMCIObject private methods
----------------------------------------------------------------------------*/

void ChMCIObject::MCIError( chuint32 luError )
{
    char	strBuffer[256];

    strBuffer[0] = 0;
    mciGetErrorString( luError, strBuffer, sizeof( strBuffer ) );
    if (!strlen( strBuffer ))
    {
        strcpy( strBuffer, "Unknown error" );
    }

    TRACE( strBuffer );
    TRACE( "\n" );
}


#if defined( CH_USE_VOXWARE )

/*----------------------------------------------------------------------------
	ChVoxwarePlayer public methods
----------------------------------------------------------------------------*/

ChSpeechPlayer::ChSpeechPlayer( DeviceType deviceType,
								bool* pboolDeviceInUseFlag ) :
					ChMediaPlayer( deviceType, pboolDeviceInUseFlag )
{
	ASSERT( devSpeech == deviceType );
}


ChSpeechPlayer::~ChSpeechPlayer()
{
}


bool ChSpeechPlayer::Play( const ChSoundInfo* pInfo )
{
	return PrepAndPlay( pInfo );
}


void ChSpeechPlayer::Stop()
{
	MSG		msg;
	bool	boolQuitMessage = false;
											/* Set the event flags to zero
												so that no notification
												occurs (since this object
												is being explicitly stopped) */
	SetEventInfo( 0 );
											// Turn off looping!
	SetLooping( false );
	SetStopped();

#if 0
											/* We need to spin a message loop
												to wait until the speech stops
												playing... */
	while (IsPlaying() && !boolQuitMessage)
	{
		if (::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
			if (WM_QUIT == msg.message)
			{
				boolQuitMessage = true;
				::PostQuitMessage( msg.wParam );
			}
			else
			{
				::DispatchMessage( &msg );
			}
		}
	}

#endif	// 0
}


/*----------------------------------------------------------------------------
	ChSpeechPlayer protected methods
----------------------------------------------------------------------------*/

bool ChSpeechPlayer::DoPlay()
{
	const char*		pstrFilename = GetSoundInfo()->GetFilename();

	SetStopped( false );

	ChTNT::PlayVoxFile( pstrFilename );
	return true;
}


#if 0

/*----------------------------------------------------------------------------
	Voxware callback function
----------------------------------------------------------------------------*/

VOXAPI_CALLBACK VoxwareCallbackFunc( unsigned short wVox,
										unsigned short wMessage,
										LPVOXWARE_DATA lpVoxwareData )
{
	ChSpeechPlayer*	pPlayer = (ChSpeechPlayer*)lpVoxwareData->dwUserData;
	bool			boolStopped = false;

	switch( wMessage )
	{
		case VOXWARE_STARTPLAY:
		{
			TRACE( "ToolVox:  Start play!\n" );
			break;
		}

		case VOXWARE_ENDPLAY:
		{
			TRACE( "ToolVox:  End play!\n" );
			pPlayer->GetNotifyWnd()->PostMessage( MSG_ON_VOX_COMPLETE );
			break;
		}

		case VOXWARE_GETINFO:
		{
			break;
		}

		case VOXWARE_PLAYBACKERROR:
		{
			TRACE( "ToolVox:  Playback error\n" );
			break;
		}

		default:
		{
			break;
		}
	}

	#if defined( CH_DEBUG )
	{
		if (pPlayer->IsStopped())
		{
			TRACE( "ToolVox:  STOPPED!\n" );
		}
	}
	#endif	// defined( CH_DEBUG )

	return pPlayer->IsStopped();
}

#endif	// 0

#endif	// defined( CH_USE_VOXWARE )


/*----------------------------------------------------------------------------
	ChMixer class
----------------------------------------------------------------------------*/

ChMixer::ChMixer( DeviceType deviceType ) :
					m_deviceType( deviceType )
{

	#if defined( CH_ARCH_32 )
	{
		m_boolUseMixerAPI = false;

		m_hWinMMLib = LoadLibrary( TEXT( "winmm.dll" ) );

		if (m_hWinMMLib)
		{									/* Get the address of all the
												procs used but not available
												under Win32s */
			pprocMixerOpen = (pprocTypeMixerOpen)
									GetProcAddress( m_hWinMMLib, "mixerOpen" );
			pprocMixerClose = (pprocTypeMixerClose)
									GetProcAddress( m_hWinMMLib, "mixerClose" );
			pprocMixerSetControlDetails = (pprocTypeMixerSetControlDetails)
									GetProcAddress( m_hWinMMLib, "mixerSetControlDetails" );
			pprocMixerGetNumDevs = (pprocTypeMixerGetNumDevs)
									GetProcAddress( m_hWinMMLib, "mixerGetNumDevs" );
			pprocMixerGetDevCaps = (pprocTypeMixerGetDevCaps)
									GetProcAddress( m_hWinMMLib, "mixerGetDevCaps" );
			pprocMixerGetLineInfo = (pprocTypeMixerGetLineInfo)
									GetProcAddress( m_hWinMMLib, "mixerGetLineInfo" );
			pprocMixerGetLineControls = (pprocTypeMixerGetLineControls)
									GetProcAddress( m_hWinMMLib, "mixerGetLineControls" );	  

			if (pprocMixerOpen && pprocMixerClose &&
				pprocMixerSetControlDetails && pprocMixerGetNumDevs &&
				pprocMixerGetLineInfo && pprocMixerGetLineControls)
			{
				m_boolUseMixerAPI = true;	
			}
		}

		if (m_boolUseMixerAPI)
		{
			MMRESULT	mmr;

			switch( deviceType )
			{
				case devMidi:
				{
					m_dwTargetComponentType = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER;
					break;
				}

				case devWave:
				case devSpeech:
				{
					m_dwTargetComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
					break;
				}

				default:
				{
					ASSERT( false );
					break;
				}
			}

			mmr = GetMixerControl();
			if (MMSYSERR_NOERROR == mmr)
			{
				m_boolValid = true;
											// Init the details structures

			    ChMemClearStruct( &m_mixerControlDetails );
		    	memset( &m_uMixerControlDetails, 0x00,
		    			sizeof( m_uMixerControlDetails ) );

			    m_mixerControlDetails.cbStruct = sizeof( m_mixerControlDetails );
			    m_mixerControlDetails.dwControlID = m_mixerControl.dwControlID;
			    m_mixerControlDetails.cChannels = m_uiChannels;
			    m_mixerControlDetails.cbDetails = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
			    m_mixerControlDetails.paDetails = &m_uMixerControlDetails[0];
			}
			else
			{
				m_boolValid = false;
			}
		}
	}
	#endif // CH_ARCH_32 
}


ChMixer::~ChMixer()
{
											/* The following 'if' statement may
												not be necessary, but we were
												getting some hangs on
												mixerClose on NT 3.51 */
	#if defined( CH_ARCH_32 )
	{
		if (m_boolUseMixerAPI)
		{
			if (ChCore::GetClientInfo()->GetPlatform() != osWinNT)
			{
				if (m_boolValid && (0 != m_hMixer) )
				{
					pprocMixerClose( m_hMixer );
				}
			}
			FreeLibrary( m_hWinMMLib );
		}
	}
	#endif // CH_ARCH_32
}  


bool ChMixer::SetVolume( chuint16 suNewVolume )
{
	#if defined( CH_ARCH_32 )

	if ( m_boolUseMixerAPI )
	{
	 	return MixerSetVolume( suNewVolume );
	}
	else

	#endif

	{
	 	return WaveSetVolume( suNewVolume );
	}
}


#if defined( CH_ARCH_32 )

bool ChMixer::MixerSetVolume( chuint16 suNewVolume )
{
	MMRESULT	mmr;
	int			iChannel;
	DWORD		dwNewVolume( suNewVolume );
											/* Make sure the mixer could be
												opened */
	if (!m_boolValid)
	{
		return false;
	}
											// Make sure the mixer is open
	ASSERT( 0 != m_hMixer );
											// Pin the volume

	if (dwNewVolume < m_mixerControl.Bounds.dwMinimum)
	{
		dwNewVolume = m_mixerControl.Bounds.dwMinimum;
	}
	else if (dwNewVolume > m_mixerControl.Bounds.dwMaximum)
	{
		dwNewVolume = m_mixerControl.Bounds.dwMaximum;
	}
											// Set the volume for all channels

	for (iChannel = 0; iChannel < (int)m_uiChannels; iChannel++)
	{
		m_uMixerControlDetails[iChannel].dwValue = dwNewVolume;
	}

	mmr = pprocMixerSetControlDetails( (HMIXEROBJ)m_hMixer, &m_mixerControlDetails,
									MIXER_SETCONTROLDETAILSF_VALUE );

	return MMSYSERR_NOERROR == mmr;
}

#endif	// defined( CH_ARCH_32 )


/*----------------------------------------------------------------------------
	ChMixer protected methods
----------------------------------------------------------------------------*/

#if defined( CH_ARCH_32 )

MMRESULT ChMixer::GetMixerControl()
{
	MMRESULT			mmr;
	UINT				uiSource;
	UINT				uiDest;
	UINT				uiMixerID;
	MIXERCAPS			mixerCaps;
	UINT				uiSrcLines;
	MIXERLINE			mixerLine;
	BOOL				boolFoundLine;
	MIXERLINECONTROLS	mixerLineControls;

    if ((MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER != m_dwTargetComponentType) &&
    	(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT != m_dwTargetComponentType))
    {
        return MMSYSERR_INVALPARAM;
    }
											/* Check to ensure that we have
												a mixer */
    if (0 == pprocMixerGetNumDevs())
    {
        return MMSYSERR_NODRIVER;
    }
											// Take the first mixer.
    uiMixerID = 0;
	mmr = pprocMixerOpen( &m_hMixer, uiMixerID, 0, 0, 0 );
    if (MMSYSERR_NOERROR != mmr)
    {
        return mmr;
    }

	mmr = pprocMixerGetDevCaps( (UINT)m_hMixer, &mixerCaps, sizeof( mixerCaps ) );
    if (MMSYSERR_NOERROR != mmr)
    {
		pprocMixerClose( m_hMixer );
		m_hMixer = 0;
        return mmr;
    }

	mixerLine.dwComponentType = 0;
	boolFoundLine = false;
											/* Search the mixer device lines
												for the correct line */

    for (uiDest = 0; (uiDest < mixerCaps.cDestinations) && !boolFoundLine;
    		uiDest++)
    {
		ChMemClearStruct( &mixerLine );
		mixerLine.cbStruct = sizeof( mixerLine );
		mixerLine.dwDestination = uiDest;

		mmr = pprocMixerGetLineInfo( (HMIXEROBJ)m_hMixer, &mixerLine,
								MIXER_GETLINEINFOF_DESTINATION );
        if (MMSYSERR_NOERROR != mmr)
        {
            pprocMixerClose( m_hMixer );
			m_hMixer = 0;
            return mmr;
        }

        if (MIXERLINE_COMPONENTTYPE_DST_SPEAKERS == mixerLine.dwComponentType)
        {
											/* This is an audio line intended
												to drive speakers.  This is
												usually the output line of an
												audio card. */

            if (mixerLine.cChannels > maxChannels)
            {
											/* There are too many channels
												to handle.  More than 5?
												Argh! */
                pprocMixerClose( m_hMixer );
				m_hMixer = 0;

                return MMSYSERR_NOMEM;
            }
											// Save the number of channels

			m_uiChannels = mixerLine.cChannels;
			if (m_uiChannels > maxChannels)
			{								// Pin the number of channels
				m_uiChannels = maxChannels;
			}
			uiSrcLines = mixerLine.cConnections;

											/* Search for the line that matches
												the component we want to control */

            for (uiSource = 0; uiSource < uiSrcLines; uiSource++)
            {
				ChMemClearStruct( &mixerLine );
				mixerLine.cbStruct = sizeof( mixerLine );
				mixerLine.dwDestination = uiDest;
				mixerLine.dwSource = uiSource;

				mmr = pprocMixerGetLineInfo( (HMIXEROBJ)m_hMixer, &mixerLine,
										MIXER_GETLINEINFOF_SOURCE );
                if (MMSYSERR_NOERROR != mmr)
                {
                    pprocMixerClose( m_hMixer );
					m_hMixer = 0;
                    return mmr;
                }

                if (mixerLine.dwComponentType == m_dwTargetComponentType)
                {
											/* Yay!  This is the line
												connecting our device to the
												speakers. */
                    boolFoundLine = TRUE;
                    break;
                }
            }
        }
    }

    if (!boolFoundLine)
    {
        pprocMixerClose( m_hMixer );
		m_hMixer = 0;
        return MMSYSERR_NOTSUPPORTED;
    }
											// Init the MIXERCONTROL struct
    ChMemClearStruct( &mixerLineControls );
    mixerLineControls.cbStruct = sizeof( mixerLineControls );
    mixerLineControls.dwLineID = mixerLine.dwLineID;
    mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mixerLineControls.cbmxctrl = sizeof( MIXERCONTROL );
    mixerLineControls.pamxctrl = &m_mixerControl;

	mmr = pprocMixerGetLineControls( (HMIXEROBJ)m_hMixer, &mixerLineControls,
								MIXER_GETLINECONTROLSF_ONEBYTYPE );
    if (MMSYSERR_NOERROR != mmr)
    {
        pprocMixerClose( m_hMixer );
		m_hMixer = 0;
        return mmr;
    }

    return MMSYSERR_NOERROR;
}

#endif	// defined( CH_ARCH_32 )


bool ChMixer::WaveSetVolume( chuint16 suNewVolume )
{
	int		iCount;

	switch( m_deviceType )
	{
		case devMidi:
		{
			if (iCount = midiOutGetNumDevs())
			{
				for (int iDev = 0; iDev < iCount; iDev++)
				{
					midiOutSetVolume( (HMIDIOUT)iDev, MAKELONG( suNewVolume, suNewVolume ));
				}
			}
			break;
		}

		case devWave:
		case devSpeech:
		{
			if (iCount = waveOutGetNumDevs())
			{
				for (int iDev = 0; iDev < iCount; iDev++)
				{
					waveOutSetVolume( (HWAVEOUT)iDev, MAKELONG( suNewVolume, suNewVolume ));
				}
			}
			break;
		}

		default:
		{
			break;
		}
	}

	return true;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
