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

	Interface for the ChMediaPlayer class.  This class knows how to
	load and play MIDI music files.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHMPLAY_H )
#define _CHMPLAY_H

#include <mmsystem.h>

#include "ChNotify.h"



/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

											// Mixer procs

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerOpen )
					( LPHMIXER phmx, UINT uMxId, DWORD dwCallback, 
    					DWORD dwInstance, DWORD fdwOpen );

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerClose )
								(HMIXER hmx);

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerSetControlDetails )
				(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

CH_TYPEDEF_LIBRARY( UINT, pprocTypeMixerGetNumDevs )
								(VOID);

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerGetDevCaps )
				(UINT uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerGetLineInfo )
		(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);

CH_TYPEDEF_LIBRARY( MMRESULT, pprocTypeMixerGetLineControls )
		(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);




typedef enum { devAll, devMidi, devWave, devSpeech } DeviceType;

#if defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChMixer class
----------------------------------------------------------------------------*/

class ChMixer
{
	public:
		enum tagConstants { maxChannels = 5 };

	public:
		ChMixer( DeviceType deviceType );
		virtual ~ChMixer();

		inline DeviceType GetDeviceType() { return m_deviceType; }
		inline bool IsValid() { return m_boolValid; }

		bool SetVolume( chuint16 suNewVolume );

	#if defined( CH_ARCH_32 )

	protected:
		MMRESULT GetMixerControl();
		bool MixerSetVolume( chuint16 suNewVolume );
		bool WaveSetVolume( chuint16 suNewVolume );


	private:
		DWORD							m_dwTargetComponentType;
		MIXERCONTROL					m_mixerControl;
		MIXERCONTROLDETAILS				m_mixerControlDetails;
		MIXERCONTROLDETAILS_UNSIGNED	m_uMixerControlDetails[maxChannels];
		UINT							m_uiChannels;
		HMIXER							m_hMixer;

	#else	// defined( CH_ARCH_32 )

		bool WaveSetVolume( chuint16 suNewVolume );

	#endif	// defined( CH_ARCH_32 )

	private:
		DeviceType						m_deviceType;
		bool							m_boolValid;

		#if defined( CH_ARCH_32 )

		bool							m_boolUseMixerAPI;

		HINSTANCE						m_hWinMMLib;
		// Mixer procs
		pprocTypeMixerOpen				pprocMixerOpen;
		pprocTypeMixerClose				pprocMixerClose;
		pprocTypeMixerSetControlDetails	pprocMixerSetControlDetails;
		pprocTypeMixerGetNumDevs		pprocMixerGetNumDevs;
		pprocTypeMixerGetDevCaps		pprocMixerGetDevCaps;
		pprocTypeMixerGetLineInfo		pprocMixerGetLineInfo;
		pprocTypeMixerGetLineControls	pprocMixerGetLineControls;

		#endif	// defined( CH_ARCH_32 )

};


/*----------------------------------------------------------------------------
	ChMCIObject class
----------------------------------------------------------------------------*/

class ChMCIObject
{
	public:
		ChMCIObject( DeviceType deviceType, CWnd* pwndNotify );
		virtual ~ChMCIObject();

		inline DeviceType GetDeviceType() { return m_deviceType; }
		inline bool IsOpen() { return (0 != m_openParams.wDeviceID); }
		inline MCIDEVICEID GetDeviceID() { return m_openParams.wDeviceID; }
		inline void SetDeviceType( const char* pstrName )
						{
							m_openParams.lpstrDeviceType = pstrName;
						}
		inline void ClearDevice() { ChMemClearStruct( &m_openParams ); }

		bool OpenFile( const ChString& strFilename );
		bool OpenDevice( const ChString& strDeviceName );
		void Close();
		void Play();
		void Stop();
		chuint32 GetPosition();

	private:
		void MCIError( chuint32 luError );

	private:
		DeviceType			m_deviceType;
		CWnd*				m_pwndNotify;
											/* The following works for both
												midi and wave, but is has some
												extra stuff for wave */
		MCI_WAVE_OPEN_PARMS	m_openParams;
};


/*----------------------------------------------------------------------------
	ChMediaPlayer class
----------------------------------------------------------------------------*/

class ChSoundMainInfo;
class ChSoundInfo;

class ChMediaPlayer
{
	friend class ChNotifyWnd;

	public:
		ChMediaPlayer( DeviceType deviceType, bool* pboolDeviceInUseFlag );
		virtual ~ChMediaPlayer();

		inline ChSoundMainInfo* GetMainInfo() { return m_pMainInfo; }
		inline chuint16 GetVolume() { return m_suVolume; }
		inline bool IsPlaying() { return *m_pboolDeviceInUseFlag; }
		inline bool IsLooping() { return m_boolLooping; }
		inline void StopLoop() { m_boolLooping = false; }
		inline bool DeviceExists() { return m_boolExists; }

		inline ChNotifyWnd* GetNotifyWnd() { return &m_notifyWnd; }

		inline void SetMainInfo( ChSoundMainInfo* pMainInfo )
				{
					m_pMainInfo = pMainInfo;
				}
		inline void SetEventInfo( chflag32 flEvents,
									const ChString& strAction = "",
									const ChString& strMD5 = "" )
				{
					m_flEvents = flEvents;
					m_strAction = strAction;
					m_strMD5 = strMD5;
				}
		inline chflag32 GetEvents() { return m_flEvents; }
		inline const ChString& GetAction() { return m_strAction; }
		inline const ChString& GetMD5() { return m_strMD5; }

		virtual bool Play( const ChSoundInfo* pInfo );
		virtual void Stop();

		virtual void SetVolume( chuint16 suVolume );

	protected:
		inline bool IsDeviceOpen() const { return (m_pMCIObject != 0); }
		inline DeviceType GetDeviceType() const { return m_deviceType; }
		inline ChMCIObject* GetDevice() const { return m_pMCIObject; }
		inline ChMixer* GetMixer() const { return m_pMixer; }
		inline const ChSoundInfo* GetSoundInfo() const { return m_pInfo; }

		inline void SetPlaying( bool boolPlaying )
				{
					*m_pboolDeviceInUseFlag = boolPlaying;
				}
		inline void SetLooping( bool boolLooping )
				{
					m_boolLooping = boolLooping;
					m_boolLoopVolume = false;
				}
		inline void SetLooping( bool boolLooping, chuint16 suVolume )
				{
					m_boolLooping = boolLooping;
					m_boolLoopVolume = true;
					m_suLoopVolume = suVolume;
				}

		virtual bool PrepAndPlay( const ChSoundInfo* pInfo );
		virtual bool DoPlay();
		virtual void OnPlayComplete();

		bool DeviceExists( DeviceType deviceType );
		void OpenDevice();
		void CloseDevice();

	private:
		ChSoundMainInfo*	m_pMainInfo;
		bool*				m_pboolDeviceInUseFlag;
		bool				m_boolExists;

		DeviceType			m_deviceType;
		ChMixer*			m_pMixer;
		ChMCIObject*		m_pMCIObject;
		ChNotifyWnd			m_notifyWnd;
		chuint16			m_suVolume;

		chflag32			m_flEvents;
		ChString				m_strAction;
		ChString				m_strMD5;

		bool				m_boolLooping;
		bool				m_boolLoopVolume;
		chuint16			m_suLoopVolume;

		ChSoundInfo*		m_pInfo;		// Info on currently playing sound
};


#if defined( CH_USE_VOXWARE )

/*----------------------------------------------------------------------------
	ChVoxwarePlayer class
----------------------------------------------------------------------------*/

struct tagVOXWARE_DATA;

class ChSpeechPlayer : public ChMediaPlayer
{
	public:
		ChSpeechPlayer( DeviceType deviceType, bool* pboolDeviceInUseFlag );
		virtual ~ChSpeechPlayer();

		inline bool IsStopped() { return m_boolStopped; }

		virtual bool Play( const ChSoundInfo* pInfo );
		virtual void Stop();

	protected:
		inline void SetStopped( bool boolStopped = true )
						{
							m_boolStopped = boolStopped;
						}

		virtual bool DoPlay();

	private:
		bool		m_boolStopped;
};

#endif	// defined( CH_USE_VOXWARE )

#endif	// defined( CH_MSW )
#endif	// !defined( _CHMPLAY_H )

// $Log$
