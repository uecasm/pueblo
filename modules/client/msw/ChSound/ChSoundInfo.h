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

	Main header file for the ChSound module of the Pueblo system.  This
	module handles playing MIDI & sampled sounds.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSOUNDINFO_H )
#define _CHSOUNDINFO_H

#include <ChReg.h>
#include <ChMsg.h>

#include "ChSPrefs.h"

#if defined( CH_USE_VOXWARE )

	#include "ChTNT.h"

#endif	// defined( CH_USE_VOXWARE )


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

enum { sndLoadModule = -1, sndPlay = 1, sndLoop, sndStop, sndStopLoop,
		speechCall, speechClose };


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

class ChSoundStreamManager;


/*----------------------------------------------------------------------------
	ChSoundInfo class
----------------------------------------------------------------------------*/

class ChSoundInfo
{
	public:
		ChSoundInfo() :
				m_boolVolumeValid( false ),
				m_suVolume( 0 ),
				m_flOptions( 0 ),
				m_flEvents( 0 ),
				m_boolLooping( false ),
				m_deviceType( devAll )
				{
				}
		ChSoundInfo( bool boolLooping, bool boolVolumeValid = false,
						chuint16 suVolume = 0, chflag32 flOptions = 0,
						chflag32 flEvents = 0, const ChString& strAction = "",
						const ChString& strMD5 = "" ) :
				m_boolLooping( boolLooping ),
				m_boolVolumeValid( boolVolumeValid ),
				m_suVolume( suVolume ),
				m_flOptions( flOptions ),
				m_flEvents( flEvents ),
				m_strAction( strAction ),
				m_strMD5( strMD5 ),
				m_deviceType( devAll )
				{
				}
		virtual ~ChSoundInfo() {}

		inline const ChString& GetAction() const { return m_strAction; }
		inline const ChString& GetMD5() const { return m_strMD5; }
		inline bool IsVolumeValid() const { return m_boolVolumeValid; }
		inline chuint16 GetVolume() const { return m_suVolume; }
		inline chflag32 GetOptions() const { return m_flOptions; }
		inline chflag32 GetEvents() const { return m_flEvents; }
		inline const ChString& GetFilename() const { return m_strFilename; }
		inline DeviceType GetDeviceType() const { return m_deviceType; }

		inline bool IsLooping() const { return m_boolLooping; }

		inline void SetLooping( bool boolLooping )
					{
						m_boolLooping = boolLooping;
					}
		inline void SetVolume( chuint16 suVolume )
					{
						m_boolVolumeValid = true;
						m_suVolume = suVolume;
					}
		inline void SetFilename( const ChString& strFilename )
					{
						m_strFilename = strFilename;
					}
		inline void SetDeviceType( DeviceType deviceType )
					{
						m_deviceType = deviceType;
					}

	protected:
		bool		m_boolVolumeValid;
		chuint16	m_suVolume;
		chflag32	m_flOptions;
		chflag32	m_flEvents;
		ChString		m_strAction;
		ChString		m_strMD5;
		bool		m_boolLooping;
		ChString		m_strFilename;
		DeviceType	m_deviceType;
};


/*----------------------------------------------------------------------------
	ChSoundQueue class
----------------------------------------------------------------------------*/

class ChSoundQueue
{
	public:
		ChSoundQueue() {}
		virtual ~ChSoundQueue() {}

		bool IsEmpty( DeviceType deviceType )
					{
						return GetDeviceQueue( deviceType )->IsEmpty();
					}

		void AddItem( DeviceType deviceType, const ChSoundInfo& info );
		bool GetNextItem( DeviceType deviceType, ChSoundInfo& info );

	protected:
		inline ChList<ChSoundInfo>* GetDeviceQueue( DeviceType deviceType )
					{
						if (devMidi == deviceType)
						{
							return &m_midiQueue;
						}
						else
						{
							return &m_waveQueue;
						}
					}

	protected:
		ChList<ChSoundInfo>		m_waveQueue;
		ChList<ChSoundInfo>		m_midiQueue;
};


/*----------------------------------------------------------------------------
	ChSoundMainInfo class
----------------------------------------------------------------------------*/

class ChSpeechStatus;

class ChSoundMainInfo : public ChMainInfo
{
	CH_FRIEND_MESSAGE_HANDLER( soundGetPagesHandler )
	CH_FRIEND_MESSAGE_HANDLER( soundGetPageDataHandler )
	CH_FRIEND_MESSAGE_HANDLER( soundLoadCompleteHandler )

	friend class ChSoundStreamManager;
	friend class ChSoundPrefs;
	friend class ChSpeechPrefs;
	friend class ChSoundOpenFileDlg;

	public:
		ChSoundMainInfo(  const ChModuleID& idModule, ChCore* pCore,
							const ChString& strLoadParam );
		virtual ~ChSoundMainInfo();

		inline bool SoundDeviceFound()
						{
							return m_boolSoundDeviceFound;
						}
		inline bool UseSound()
						{
							return SoundDeviceFound() && !IsDisabled();
						}
		inline bool IsDisabled()
						{
							return m_boolDisableSound;
						}

		inline bool IsShown() { return m_boolShown; }
		inline const ChString& GetAlertSoundPath() { return m_strAlertSoundPath; }
		inline void UpdateAlertTimes()
						{
							m_alertTime.Update();
						}

		#if defined( CH_USE_VOXWARE )

		inline void UpdateSpeechPrefs()
						{
							UpdateSpeechVolume();

							m_pTNT->UpdatePrefs();
						}

		inline int GetSpeechPort()
						{
							return m_pTNT->GetTNTPort();
						}

		#endif	// defined( CH_USE_VOXWARE )

		void Initialize();
		void ShowModule( bool boolShow );

		bool DoCommand( const ChString& strArgs, bool boolInline );

		void Play( ChString strURL, bool boolLooping );
		void StopAll();

		bool DoAlertCommand();

		void SendWorldCommand( const ChString& strMD5, const ChString& strAction,
								const ChString& strParams );

		void OnPlayComplete( DeviceType deviceType );

	protected:
		inline ChModuleID GetWorldID() { return m_idWorldModule; }
		inline chuint16 GetAlertVolume() { return m_suAlertVolume; }
		inline chuint16 GetMaxMidi() { return m_suMaxMidi; }
		inline chuint16 GetMaxWave() { return m_suMaxWave; }
		inline ChSoundQueue* GetDeviceQueue() { return &m_queue; }
		inline ChMediaPlayer* GetMidiPlayer() { return &m_midiPlayer; }
		inline ChMediaPlayer* GetWavePlayer() { return &m_wavePlayer; }

		#if defined( CH_USE_VOXWARE )

		inline ChSpeechPlayer* GetSpeechPlayer() { return &m_speechPlayer; }
		inline chuint16 GetMaxSpeech() { return m_suMaxSpeech; }

		#endif	// defined( CH_USE_VOXWARE )

		inline chuint16 GetScaledVolume( chuint16 suVolume,
											chuint16 suMaxVolume )
						{
							chuint32		luValue;

											// suVolume can be from 0 to 100...

							luValue = (suMaxVolume * suVolume) / 100;
							if ( luValue > (chuint32)suMaxVolume)
							{
								luValue = suMaxVolume;
							}
						
							return (chuint16)luValue;
						}
		inline void SetShown( bool boolShown ) { m_boolShown = boolShown; }
		inline void SetDisabled( bool boolDisabled )
						{
							m_boolDisableSound = boolDisabled;
						}
		inline void SetAlertSoundPath( const ChString& strPath )
						{
							m_strAlertSoundPath = strPath;
						}

		void SetWorldID( const ChModuleID& idModule );

		void HandleSoundFile( const ChString& strFile, int iMimeType,
								ChSoundInfo* pSoundInfo );
		void PlayFromQueue( DeviceType queueDevType );
		bool DoPlay( const ChSoundInfo* pInfo );

		void InstallHooks( bool boolInstall = true );

		bool DoSoundCommand( const ChString& strCommand,
								const ChString& strURL, const ChString& strDevice,
								bool boolVolume, chuint16 suVolume,
								chflag32 flOptions, chflag32 flEvents,
								const ChString& strAction,
								const ChString& strMD5 );
		bool DoVolumeCommand( DeviceType device, chuint16 suVolume );
		bool GetVolume( const ChString& strVolume, chuint16& suVolume );
		void UpdateVolume();

		#if defined( CH_USE_VOXWARE )

		void UpdateSpeechVolume();

		#endif	// defined( CH_USE_VOXWARE )

		#if defined( CH_USE_VOXWARE )

		bool DoSpeechCommand( const ChString& strCommand, const ChString& strHost,
								chuint16 suPort, const ChString& strCallId,
								chflag32 flOptions, chflag32 flRemoteOptions );

		#endif	// defined( CH_USE_VOXWARE )

	private:
		void RegisterDispatchers();

		void CreateMenus();
		void InstallMenus();
		void UninstallMenus();
		void DestroyMenus();

		void LoadWorldModule();
		void UnloadWorldModule();

		int ParseSoundCmd( const ChString& strCommand );

		#if defined( CH_USE_VOXWARE )

		int ParseSpeechCmd( const ChString& strCommand );

		#endif	// defined( CH_USE_VOXWARE )

		DeviceType ParseDevice( const ChString& strDevice );
		chflag32 ParseEvents( const ChString& strEvents );
		chflag32 ParseOptions( const ChString& strOptions );
		chflag32 ParseSpeechOptions( const ChString& strOptions );

	private:
		ChRegistry				m_reg;

		ChModuleID				m_idWorldModule;

		bool					m_boolDisableSound;
		bool					m_boolSoundDeviceFound;

		ChDispatcher			m_soundDispatcher;
		ChString					m_strInitialCommand;

		chuint16				m_suMidiVolume;
		chuint16				m_suWaveVolume;
		chuint16				m_suAlertVolume;

		chuint16				m_suMaxMidi;
		chuint16				m_suMaxWave;
						  	
		ChSoundStreamManager* 	m_pSoundStream; 

		bool					m_boolShown;
		bool					m_boolMenus;
		bool					m_boolHooksInstalled;

		ChSoundQueue			m_queue;
		bool					m_boolMidiDeviceInUse;
		bool					m_boolWaveDeviceInUse;

		ChMediaPlayer			m_midiPlayer;
		ChMediaPlayer			m_wavePlayer;

		#if defined( CH_USE_VOXWARE )

		ChSpeechPlayer			m_speechPlayer;
		chuint16				m_suSpeechVolume;
		chuint16				m_suMaxSpeech;

		#endif	// defined( CH_USE_VOXWARE )

		ChString					m_strAlertSoundPath;
		ChTime					m_lastAlertTime;
		ChAlertTime				m_alertTime;

		#if defined( CH_USE_VOXWARE )

		ChTNT*					m_pTNT;
		ChSpeechStatus*			m_pSpeechStatus;

		#endif	// defined( CH_USE_VOXWARE )
};

#endif	// !defined( _CHSOUNDINFO_H )

// $Log$
