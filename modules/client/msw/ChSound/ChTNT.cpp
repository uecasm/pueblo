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

	Implementation of the ChTNT object, which manages the interface between
	TNT and the Sound Module.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#if defined( CH_USE_VOXWARE )

#include <ChReg.h>

#include "ChSpeechPrefs.h"
#include "ChSpeechStatus.h"
#include "ChTNT.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define DEF_TNT_PORT			12380
											/* This should be set to zero
												if Voxware ever fixes the
												problem of returning the
												correct port number */
#define TNT_PORT				DEF_TNT_PORT
											/* The following is the number
												of milliseconds that TNT
												will continue to transmit
												if it encounters silence.
												Probably best not to change
												this. */
#define TNT_SILENT_WINDOW		500
											/* The following is the number
												of milliseconds until playback
												occurs. */
#define TNT_PLAYBACK_DELAY		300
											/* The following is the activation
												level */
#define TNT_ACTIVATION_LEVEL	160


/*----------------------------------------------------------------------------
	ChTNT static member variables
----------------------------------------------------------------------------*/

ChTNT*				ChTNT::m_this = 0;
int					ChTNT::m_iUsage = 0;
TNT_HANDLE			ChTNT::m_hTNT = 0;

ChTNTSessionMgr		ChTNT::m_tntSessions;
chuint16			ChTNT::m_suTNTPort = 0;
ChString				ChTNT::m_strTNTLocalHost;

TNT_USER_INFO		ChTNT::m_userInfo;

chuint32			ChTNT::m_luMikeVolume;
chuint32			ChTNT::m_luMikeSensitivity = TNT_ACTIVATION_LEVEL;
bool				ChTNT::m_boolRejectCalls = false;
ChSpeechStatus*		ChTNT::m_pStatus = 0;


/*----------------------------------------------------------------------------
	ChTNT public methods
----------------------------------------------------------------------------*/

ChTNT::ChTNT( ChSpeechStatus* pSpeechStatus )
{
	ASSERT( 0 != pSpeechStatus );
	m_pStatus = pSpeechStatus;

	pSpeechStatus->SetTNT( this );

	if (0 == m_iUsage)
	{
		TNT_INITIALIZE			tntInit;
		TNT_TRANSACTION_HANDLE	hTransaction;
		TNT_RETCODE				retCode;

		m_this = this;

		ChMemClearStruct( &m_userInfo );
		m_userInfo.wSizeOfUserInfo = sizeof( TNT_USER_INFO );

		ChMemClearStruct( &tntInit );
		tntInit.wSizeOfInitialize = sizeof( TNT_INITIALIZE );
		tntInit.usListenPort = TNT_PORT;
		tntInit.lpNotifyProc = NotifyProc;

		#if defined( CH_DEBUG )
		{
			tntInit.dwDebugLevel = TNT_DEBUG_WARNING | TNT_DEBUG_ERROR;
		}
		#endif	// defined( CH_DEBUG )

		retCode = tntInitialize( &m_hTNT, &hTransaction, 0, &tntInit );
		if (TNT_NO_ERROR != retCode)
		{
			#if defined( CH_DEBUG )
			{
				char		cBuffer[160];

				sprintf( cBuffer, "TNT: Error calling tntInitialize (%hu)\n",
										retCode );
				TRACE( cBuffer );
			}
			#endif	// defined( CH_DEBUG )

			m_hTNT = 0;
		}
		else
		{
			chuint16	suFullDuplex = false;

			retCode = tntSoundQueryDuplex( GetTNTHandle(), 0, 0,
											&suFullDuplex );
			retCode = tntSoundSetDevices( GetTNTHandle(), 0, 0,
											!!suFullDuplex );
			if (TNT_NO_ERROR != retCode)
			{
				#if defined( CH_DEBUG )
				{
					char		cBuffer[160];

					sprintf( cBuffer,
								"TNT: Error calling "
								"tntSoundSetDevices (%hu)\n",
								retCode );
					TRACE( cBuffer );
				}
				#endif	// defined( CH_DEBUG )
			}

			retCode = tntSoundHandsOffMode( GetTNTHandle(),
											TNT_HANDS_OFF_MODE );
			if (TNT_NO_ERROR != retCode)
			{
				#if defined( CH_DEBUG )
				{
					char		cBuffer[160];

					sprintf( cBuffer,
								"TNT: Error calling "
								"tntSoundHandsOffMode (%hu)\n",
								retCode );
					TRACE( cBuffer );
				}
				#endif	// defined( CH_DEBUG )
			}
											// Set the user information

			ChMemClearStruct( &m_userInfo );

			m_userInfo.wSizeOfUserInfo = sizeof( TNT_USER_INFO );
			strcpy( m_userInfo.szFirstName, "Pueblo" );
			strcpy( m_userInfo.szLastName, "User" );

			retCode = tntSetUserInfo( GetTNTHandle(), &m_userInfo );
			if (TNT_NO_ERROR != retCode)
			{
				#if defined( CH_DEBUG )
				{
					char		cBuffer[160];

					sprintf( cBuffer,
								"TNT: Error calling tntSetUserInfo (%hu)\n",
										retCode );
					TRACE( cBuffer );
				}
				#endif	// defined( CH_DEBUG )
			}

			UpdatePrefs();
		}
	}

	m_iUsage++;
}


ChTNT::~ChTNT()
{
	if (0 == --m_iUsage)
	{
		if (GetTNTHandle())
		{
			TNT_RETCODE			retCode;

			retCode = tntCleanup( GetTNTHandle() );
			if (TNT_NO_ERROR != retCode)
			{
				#if defined( CH_DEBUG )
				{
					char		cBuffer[160];

					sprintf( cBuffer, "TNT: Error calling tntCleanup (%hu)\n",
										retCode );
					TRACE( cBuffer );
				}
				#endif	// defined( CH_DEBUG )
			}
		}

		m_hTNT = 0;
	}
}


/*----------------------------------------------------------------------------
	ChTNT public methods
----------------------------------------------------------------------------*/

bool ChTNT::MakeCall( const ChString& strCallId, const ChString& strHost,
						chuint16 suPort, chflag32 flOptions,
						chflag32 flRemoteOptions )
{
	TNT_USER_INFO			destUserInfo;
	TNT_RETCODE				retCode;
	TNT_TRANSACTION_HANDLE	hTransaction;
	bool					boolSuccess;
											// Copy information to be shared

	ASSERT( TNT_MAX_LASTNAME_LENGTH > sizeof( chflag32 ) );

	sprintf( m_userInfo.szLastName, "%d", flRemoteOptions );
	strncpy( m_userInfo.szFirstName, (const char*)strCallId, TNT_MAX_LASTNAME_LENGTH );
	m_userInfo.szFirstName[TNT_MAX_LASTNAME_LENGTH - 1] = '\0';

	retCode = tntSetUserInfo( GetTNTHandle(), &m_userInfo );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntSetUserInfo (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}

	ChMemClearStruct( &destUserInfo );
	destUserInfo.wSizeOfUserInfo = sizeof( TNT_USER_INFO );
	strcpy( destUserInfo.szNetAddress, (const char*)strHost );
	destUserInfo.usPort = suPort;

	retCode = tntSessionEstablish( GetTNTHandle(), &hTransaction,
										0, &destUserInfo, USE_UDP, 0 );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntEstablishSession (%hu)\n",
						retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )

		boolSuccess = false;
	}

	return boolSuccess;
}


bool ChTNT::Hangup( const ChString& strCallId )
{
	bool	boolSuccess;

	boolSuccess = m_tntSessions.Hangup( GetTNTHandle(), strCallId );

	return boolSuccess;
}


void ChTNT::ForceTalk( bool boolTalk )
{
	unsigned short		suState;

	suState = boolTalk ? TNT_START_TALK : TNT_STOP_TALK;
	tntSoundSetTalkState( GetTNTHandle(), suState );
}


void ChTNT::PlayVoxFile( const ChString& strFilepath )
{
	TNT_RETCODE				retCode = TNT_NO_ERROR;
//	TNT_TRANSACTION_HANDLE	hTransaction;

//	retCode = tntPlaySound( GetTNTHandle(), &hTransaction, 0,
//							(const char*)strFilepath, TNT_SOUND_VOX_FILE );

	#if defined( CH_DEBUG )
	{
		if (TNT_NO_ERROR != retCode)
		{
			char		cBuffer[160];

			sprintf( cBuffer, "TNT: Error calling tntPlaySound (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
	}
	#endif	// defined( CH_DEBUG )
}


void ChTNT::UpdatePrefs()
{
	ChRegistry	reg( SOUND_PREFS_GROUP );
	chint32		lVolume;
	TNT_RETCODE	retCode = TNT_NO_ERROR;

	reg.ReadBool( SOUND_PREFS_REJECT_CALLS, m_boolRejectCalls,
					SOUND_PREFS_REJECT_CALLS_DEF );
	reg.Read( SOUND_PREFS_MIKE_VOLUME, m_luMikeVolume,
				SOUND_PREFS_MIKE_VOLUME_DEF );

	retCode = tntSoundSetRecordLevel( GetTNTHandle(), GetMikeVolume() );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntSoundSetRecordLevel (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}

	reg.Read( SOUND_PREFS_SPEECH_VOLUME, lVolume, SPEECH_OUT_VOLUME_MAX );
	if (lVolume < 0)
	{
		lVolume = 0;
	}
	else if (lVolume > SPEECH_OUT_VOLUME_MAX)
	{
		lVolume = SPEECH_OUT_VOLUME_MAX;
	}
	retCode = tntSoundSetSpeakerVolume( GetTNTHandle(), lVolume );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntSoundSetSpeakerVolume (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}

	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntSoundMuteMicrophone (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}

	retCode = tntVoxSetGainControl( GetTNTHandle(), true, 5 );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntVoxGetGainControl (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}

	retCode = tntVoxSetActivationLevel( GetTNTHandle(),
										(chuint16)GetMikeSensitivity() );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer,
						"TNT: Error calling tntVoxSetActivationLevel (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )
	}
}


/*----------------------------------------------------------------------------
	ChTNT notification callback
----------------------------------------------------------------------------*/

TNT_RETCODE TNT_FAR TNT_PASCAL TNT_LOADDS 
ChTNT::NotifyProc( TNT_HANDLE hTNT, unsigned short wMessage,
					TNT_TRANSACTION_HANDLE hTransaction,
    				unsigned long dwParam1, unsigned long dwParam2,
    				TNT_USER_HANDLE hUser )
{
	#if defined( CH_DEBUG )
	char		cBuffer[160];
	#endif	// defined( CH_DEBUG )
											// Get the 'this' pointer
	switch (wMessage)
	{
		case TNT_NOTIFY_INIT_STATUS:
		{
			break;
		}

		case TNT_NOTIFY_LOCAL_IP_PORT:
		{
			char*		pstrAddress = (char*)dwParam1;
			chuint16	suPort = (chuint16)dwParam2;

			SetTNTPort( suPort );

			#if defined( CH_DEBUG )
			{
				sprintf( cBuffer, "TNT: Local port is %hu\n", suPort );
				TRACE( cBuffer );
			}
			#endif	// defined( CH_DEBUG )
			break;
		}

		case TNT_NOTIFY_MIC_VOLUME_CHANGE:
		{
			return TNT_NO_ERROR;
		}

		case TNT_NOTIFY_SPEAKER_VOLUME_CHANGE:
		{
			return TNT_NO_ERROR;
		}

		case TNT_NOTIFY_BUFFER_LEVEL_CHANGE:
		{
			return TNT_NO_ERROR;
		}

		case TNT_NOTIFY_MIC_GAINMETER_CHANGE:
		{
			GetStatus()->SetRecordLevel( (int)dwParam1 );
			return TNT_NO_ERROR;
		}

		case TNT_NOTIFY_SESSION_STATUS:
		{
			TNT_SESSION_INFO*	pSessionInfo = (TNT_SESSION_INFO*)dwParam2;

			switch( dwParam1 )
			{
				case TNT_SESSION_ESTABLISHED:
				{
					TNT_RETCODE			retCode;
					TNT_SESSION_HANDLE	hSession;
					ChString				strCallId;
					chflag32			flOptions;

					hSession = pSessionInfo->hSession;

					strCallId = pSessionInfo->RemoteUserInfo.szFirstName;
					flOptions =
						atol( pSessionInfo->RemoteUserInfo.szLastName );

					TRACE( "TNT: Call is established.\n" );

					m_tntSessions.Set( strCallId, hSession, flOptions );

					retCode = tntSessionSetDelayBuffer( hTNT, hSession,
														TNT_PLAYBACK_DELAY );
					if (TNT_NO_ERROR != retCode)
					{
						#if defined( CH_DEBUG )
						{
							sprintf( cBuffer,
										"TNT: Error calling "
										"tntSessionSetDelayBuffer (%hu)\n",
										retCode );
							TRACE( cBuffer );
						}
						#endif	// defined( CH_DEBUG )
					}

					retCode = tntVoxSetSilentWindow( hTNT, TNT_SILENT_WINDOW );
					if (TNT_NO_ERROR != retCode)
					{
						#if defined( CH_DEBUG )
						{
							sprintf( cBuffer,
										"TNT: Error calling "
										"tntVoxSetSilentWindow (%hu)\n",
										retCode );
							TRACE( cBuffer );
						}
						#endif	// defined( CH_DEBUG )
					}

					GetStatus()->SetConnected( true );
					UpdatePrefs();
					break;
				}

				case TNT_SESSION_RINGING:
				{
					TRACE( "TNT: Ring!\n" );
					GetStatus()->SetConnected( true );
					break;
				}

				case TNT_SESSION_INCOMING_CALL:
				{
					TNT_RETCODE				retCode;
					unsigned long			dwResponse;
					TNT_SESSION_HANDLE		hSession;

					TRACE( "TNT: Incoming call.\n" );

					hSession = pSessionInfo->hSession;

					if (m_boolRejectCalls ||
						m_tntSessions.AllSessionsInUse())
					{
						dwResponse = TNT_SESSION_RESPONSE_BUSY;
					}
					else
					{
						dwResponse = TNT_SESSION_RESPONSE_ACCEPT;
					}

					retCode = tntSessionResponse( hTNT, 0, 0, hSession,
													dwResponse );
					if (TNT_NO_ERROR != retCode)
					{
						#if defined( CH_DEBUG )
						{
							sprintf( cBuffer,
										"TNT: Error calling "
										"tntSessionResponse (%hu)\n",
										retCode );
							TRACE( cBuffer );
						}
						#endif	// defined( CH_DEBUG )
					}
					break;
				}

				case TNT_SESSION_HANGUP:
				{
											// Clear the active session handle

					m_tntSessions.ClearSessionHdl( pSessionInfo->hSession );
					GetStatus()->SetConnected( false );

					TRACE( "TNT: Hung up!\n" );
					break;
				}

				case TNT_SESSION_XMIT:
				{
					return TNT_NO_ERROR;
				}

				case TNT_SESSION_RECV:
				{
					return TNT_NO_ERROR;
				}

				case TNT_SESSION_UNKNOWN_STATUS:
				{
					break;
				}

				default:
				{
					#if defined( CH_DEBUG )
					{
						//wsprintf(cBuffer, "Unknown response from remote user (%ld).", dwParam1);
						//SendMessage(GetDlgItem(hNotifyDlg, IDC_NOTIFY_LIST), LB_INSERTSTRING, (WPARAM)-1, (long)cBuffer);
					}
					#endif	// defined( CH_DEBUG )
					break;
				}
			}
			break;
		}	

		case TNT_NOTIFY_SOUND_ENGINE_MODE:
		{
			switch( dwParam1 )
			{
				case TNT_SOUND_ENGINE_LISTEN_MODE:
				{
					GetStatus()->DisplayTalk( !dwParam2 );
					break;
				}

				case TNT_SOUND_ENGINE_TALK_MODE:
				{
					GetStatus()->DisplayTalk( !!dwParam2 );
					break;
				}
			}
			break;
		}

		case TNT_NOTIFY_SESSION_RESPONSE:
		{
			if (TNT_SESSION_RESPONSE_ACCEPT == dwParam2)
			{
				#if defined( CH_DEBUG )
				{
											// Call accepted!

					strcpy( cBuffer, "TNT: Call has been accepted." );
				}
				#endif	// defined( CH_DEBUG )
			}
			else
			{
				switch (dwParam2)
				{
					case TNT_SESSION_RESPONSE_REJECT:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: Call has not been accepted." );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}

					case TNT_SESSION_RESPONSE_BUSY:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: Caller is busy." );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}

					case TNT_SESSION_RESPONSE_NOANSWER:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: Caller did not answer." );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}

					case TNT_SESSION_RESPONSE_BLOCK:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: Caller is blocking calls." );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}

					case TNT_SESSION_RESPONSE_VOICEMAIL:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: Speech mail system." );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}

					default:
					{
						#if defined( CH_DEBUG )
						{
							strcpy( cBuffer, "TNT: UNKNOWN CALL RESPONSE\n" );
						}
						#endif	// defined( CH_DEBUG )
						break;
					}
				}
			}

			#if defined( CH_DEBUG )
			{
				TRACE( cBuffer );
			}
			#endif	// defined( CH_DEBUG )
			break;
		}

		case TNT_NOTIFY_PLAYSOUND_DONE:
		{
			break;
		}

		case TNT_NOTIFY_SPEAKER_GAINMETER_CHANGE:
		case TNT_NOTIFY_ENCODE_LOAD:
		case TNT_NOTIFY_DECODE_LOAD:
		{
			break;
		}

		#if defined( CH_DEBUG )

		case TNT_NOTIFY_DEBUG_MESSAGE:
		{
			char*	pstrError = (char*)dwParam2;

			TRACE( pstrError );
			TRACE( "\n" );
			break;
		}

		#endif	// defined( CH_DEBUG )

		default:
		{
			#if defined( CH_DEBUG )
			{
				sprintf( cBuffer, "TNT: Unknown notification message (%ld).\n",
									wMessage );
				TRACE( cBuffer );
			}
			#endif	// defined( CH_DEBUG )
			break;
		}
	}

	return TNT_NO_ERROR;
}


/*----------------------------------------------------------------------------
	ChTNTSessionMgr public methods
----------------------------------------------------------------------------*/

ChTNTSessionMgr::ChTNTSessionMgr() :
						m_iSessions( 0 )
{
	for (int iLoop = 0; iLoop < SESSION_LIMIT; iLoop++)
	{
		SetInUse( iLoop, false );
	}
}


ChTNTSessionMgr::~ChTNTSessionMgr()
{
}


bool ChTNTSessionMgr::Set( const ChString& strCallId,
							TNT_SESSION_HANDLE hSession,
							chflag32 flOptions )
{
	bool			boolSuccess = false;
	ChTNTSession*	pFoundSession = 0;
	int				iLoc;

	if (0 == (pFoundSession = Find( strCallId, &iLoc )))
	{
		if (SESSION_LIMIT > GetSessionCount())
		{
			iLoc = 0;

			while ((0 == pFoundSession) && (iLoc < SESSION_LIMIT))
			{
				if (!IsInUse( iLoc ))
				{
					pFoundSession = GetSession( iLoc );
				}
				else
				{
					iLoc++;
				}
			}
		}
	}

	if (pFoundSession)
	{
		SetInUse( iLoc );

		pFoundSession->Set( hSession, strCallId, flOptions );

		boolSuccess = true;
	}

	return boolSuccess;
}


TNT_SESSION_HANDLE ChTNTSessionMgr::
						GetSessionHandle( const ChString& strCallId,
											chflag32* pflOptions )
{
	TNT_SESSION_HANDLE		hSession = 0;
	ChTNTSession*			pSession;

	if (pSession = Find( strCallId ))
	{
		hSession = pSession->GetSessionHdl();

		if (pflOptions)
		{
			*pflOptions = pSession->GetOptions();
		}
	}

	return hSession;
}


bool ChTNTSessionMgr::Hangup( TNT_HANDLE hTNT, const ChString& strCallId )
{
	TNT_SESSION_HANDLE		hSession = 0;
	bool					boolSuccess = true;

	if (!strCallId.IsEmpty())
	{										// Look up the session by call id

		hSession = GetSessionHandle( strCallId );
	}

	if (hSession)
	{
		boolSuccess = Hangup( hTNT, hSession );
	}
	else
	{
		for (int iLoop = 0; iLoop < SESSION_LIMIT; iLoop++)
		{
			if (IsInUse( iLoop ))
			{
				boolSuccess = boolSuccess &&
								Hangup( hTNT,
										GetSession( iLoop )->GetSessionHdl() );
			}
		}
	}

	return boolSuccess;
}


bool ChTNTSessionMgr::ClearSession( const ChString& strCallId )
{
	int		iLoc;
	bool	boolFound;

	if (boolFound = (0 != Find( strCallId, &iLoc )))
	{
		SetInUse( iLoc, false );
	}

	return boolFound;
}


bool ChTNTSessionMgr::ClearSessionHdl( TNT_SESSION_HANDLE hSession )
{
	bool		boolFound = false;
	int			iLoop;

	for (iLoop = 0; iLoop < SESSION_LIMIT; iLoop++)
	{
		if (IsInUse( iLoop ) &&
			(GetSession( iLoop )->GetSessionHdl() == hSession))
		{
			SetInUse( iLoop, false );

			boolFound = true;
		}
	}

	return boolFound;
}


/*----------------------------------------------------------------------------
	ChTNTSessionMgr protected methods
----------------------------------------------------------------------------*/

ChTNTSession* ChTNTSessionMgr::Find( const ChString& strCallId, int* piLoc )
{
	ChTNTSession*	pFoundSession = 0;

	if (0 != GetSessionCount())
	{
		int		iLoop = 0;

		while ((0 == pFoundSession) && (iLoop < SESSION_LIMIT))
		{
			if (IsInUse( iLoop ))
			{
				if (GetSession( iLoop )->GetId() == strCallId)
				{
					pFoundSession = GetSession( iLoop );

					if (piLoc)
					{
						*piLoc = iLoop;
					}
				}
			}

			iLoop++;
		}
	}

	return pFoundSession;
}


ChTNTSession* ChTNTSessionMgr::Find( TNT_SESSION_HANDLE hSession,
										int* piLoc )
{
	ChTNTSession*	pFoundSession = 0;

	if (0 != GetSessionCount())
	{
		int		iLoop = 0;

		while ((0 == pFoundSession) && (iLoop < SESSION_LIMIT))
		{
			if (IsInUse( iLoop ))
			{
				if (GetSession( iLoop )->GetSessionHdl() == hSession)
				{
					pFoundSession = GetSession( iLoop );

					if (piLoc)
					{
						*piLoc = iLoop;
					}
				}
			}

			iLoop++;
		}
	}

	return pFoundSession;
}


bool ChTNTSessionMgr::Hangup( TNT_HANDLE hTNT, TNT_SESSION_HANDLE hSession )
{
	TNT_RETCODE			retCode;
	TNT_TRANSACTION_HANDLE	hTransaction;
	bool					boolSuccess = true;

	retCode = tntSessionHangup( hTNT, &hTransaction, 0, hSession );
	if (TNT_NO_ERROR != retCode)
	{
		#if defined( CH_DEBUG )
		{
			char		cBuffer[160];

			sprintf( cBuffer, "TNT: Error calling tntHangupSession (%hu)\n",
								retCode );
			TRACE( cBuffer );
		}
		#endif	// defined( CH_DEBUG )

		boolSuccess = false;
	}

	return boolSuccess;
}


/*----------------------------------------------------------------------------
	ChTNTSession public methods
----------------------------------------------------------------------------*/

ChTNTSession::ChTNTSession()
{
}


ChTNTSession::~ChTNTSession()
{
}


#endif	// defined( CH_USE_VOXWARE )


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:07  uecasm
// Import of source tree as at version 2.53 release.
//
