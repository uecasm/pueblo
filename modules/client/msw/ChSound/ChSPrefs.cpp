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

	Implementation of the ChSoundPrefs property page.  This page handles
	preferences for the Sound module.

----------------------------------------------------------------------------*/

// $Header$

#include "stdlib.h"

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChUtil.h>

#include "ChSOpen.h"
#include "ChSPrefs.h"
#include "ChSoundInfo.h"
#include "ChSoundUtils.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define MIDI_TEST_FILE_NAME			"ChTest.mid"
#define SPEECH_TEST_FILE_NAME		"ChTest.vox"


/*----------------------------------------------------------------------------
	ChAlertTime class
----------------------------------------------------------------------------*/

ChAlertTime::ChAlertTime( ChRegistry* pSoundPrefs ) :
				 m_pSoundPrefs( pSoundPrefs ),
				 m_boolDirty( false )
{
	ReadAlertFreq();
}


void ChAlertTime::OnCommit()
{
	if (m_boolDirty)
	{
		WriteAlertFreq();
	}
}


void ChAlertTime::SetAlertCount( chint32 lCount )
{
	if (lCount < 0)
	{
		lCount = 0;
	}
	else if (lCount > 60)
	{
		lCount = 60;
	}

	if (lCount != m_lAlertCount)
	{
		m_boolDirty = true;
	}

	m_lAlertCount = lCount;
}


void ChAlertTime::SetAlertUnits( ChString strUnits )
{
	AlertUnitType	newUnits;

	newUnits = TranslateUnitsName( strUnits );

	SetAlertUnits( newUnits );
}


void ChAlertTime::SetAlertUnits( AlertUnitType newUnits )
{
	if (newUnits != m_timeUnits)
	{
		m_boolDirty = true;
	}

	m_timeUnits = newUnits;

	switch( newUnits )
	{
		case minutes:
		{
			m_lUnitsMultiplier = 60;
			break;
		}

		case hours:
		{
			m_lUnitsMultiplier = 3600;
			break;
		}

		default:
		{
			m_lUnitsMultiplier = 1;
			break;
		}
	}
}


ChString ChAlertTime::GetUnitsName()
{
	ChString	strUnits;

	switch( m_timeUnits )
	{
		case minutes:
		{
			strUnits = SOUND_PREFS_ALERT_FREQ_UNITS_MIN;
			break;
		}

		case hours:
		{
			strUnits = SOUND_PREFS_ALERT_FREQ_UNITS_HOUR;
			break;
		}

		default:
		{
			strUnits = SOUND_PREFS_ALERT_FREQ_UNITS_SEC;
			break;
		}
	}

	return strUnits;
}


ChAlertTime::AlertUnitType
ChAlertTime::TranslateUnitsName( const ChString& strUnitsName )
{
	AlertUnitType	units;
	ChString			strUnits( strUnitsName );

	strUnits.MakeLower();
	if (strUnits == SOUND_PREFS_ALERT_FREQ_UNITS_MIN)
	{
		units = minutes;
	}
	else if (strUnits == SOUND_PREFS_ALERT_FREQ_UNITS_HOUR)
	{
		units = hours;
	}
	else
	{										// Assume seconds
		units = seconds;
	}

	return units;
}


void ChAlertTime::ReadAlertFreq()
{
	ChString		strTimeUnits;

	m_pSoundPrefs->Read( SOUND_PREFS_ALERT_FREQ_COUNT, m_lAlertCount,
							SOUND_PREFS_ALERT_FREQ_COUNT_DEF );

	m_pSoundPrefs->Read( SOUND_PREFS_ALERT_FREQ_UNITS, strTimeUnits,
							SOUND_PREFS_ALERT_FREQ_UNITS_DEF );

	SetAlertUnits( TranslateUnitsName( strTimeUnits ) );

	m_boolDirty = false;
}


void ChAlertTime::WriteAlertFreq()
{
	m_pSoundPrefs->Write( SOUND_PREFS_ALERT_FREQ_COUNT, m_lAlertCount );
	m_pSoundPrefs->Write( SOUND_PREFS_ALERT_FREQ_UNITS, GetUnitsName() );
}


/*----------------------------------------------------------------------------
	ChSoundPrefs class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChSoundPrefs, ChPropertyBaseClass )

ChSoundPrefs::ChSoundPrefs() :
				ChPropertyBaseClass( ChSoundPrefs::IDD, 0 
#if !defined( CH_PUEBLO_PLUGIN )
					, ChSoundDLL.hModule 
#endif					
					),
				m_pMainInfo( 0 ),
				m_reg( SOUND_PREFS_GROUP ),
				m_boolInitialized( false ),
				m_alertTime( &m_reg )
{
	ChString		strDefAlertSound;

	//{{AFX_DATA_INIT(ChSoundPrefs)
	m_boolDisableSound = FALSE;
	//}}AFX_DATA_INIT

	ChUtil::GetAppDirectory( m_strMidiFilePath );
	m_strMidiFilePath += MIDI_TEST_FILE_NAME;

	if (!ChUtil::FileExists( m_strMidiFilePath ))
	{
		m_strMidiFilePath = "";
	}
											/* Pick a reasonable default alert
												sound */

	strDefAlertSound = GetSysSoundFilesPath() + "Tada.wav";
	m_reg.Read( SOUND_PREFS_ALERT_SOUND, m_strAlertSoundPath,
				strDefAlertSound );
											// Read whether we're disabled

	m_reg.ReadBool( SOUND_PREFS_DISABLED, m_boolDisableSound,
					SOUND_PREFS_DISABLED_DEF );
}


ChSoundPrefs::~ChSoundPrefs()
{
}

void ChSoundPrefs::SetMainInfo( ChSoundMainInfo* pMainInfo )
{
	m_pMainInfo = pMainInfo;
	m_boolSoundDeviceFound = pMainInfo->SoundDeviceFound();
}
											// Overrides


void ChSoundPrefs::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyBaseClass::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChSoundPrefs)
	DDX_Control(pDX, IDC_CHECK_DISABLE, m_btnDisable);
	DDX_Control(pDX, IDC_ALERT_SOUND, m_btnAlertSound);
	DDX_Control(pDX, IDC_STATIC_OFF, m_staticOff);
	DDX_Control(pDX, IDC_STATIC_MUSIC, m_staticMusic);
	DDX_Control(pDX, IDC_STATIC_MSG, m_staticMsg);
	DDX_Control(pDX, IDC_STATIC_MAX, m_staticMax);
	DDX_Control(pDX, IDC_STATIC_EFFECTS, m_staticEffects);
	DDX_Control(pDX, IDC_STATIC_DISABLE_MSG, m_staticDisableMsg);
	DDX_Control(pDX, IDC_STATIC_BOX, m_staticBox);
	DDX_Control(pDX, IDC_STATIC_ALERTS, m_staticAlerts);
	DDX_Control(pDX, IDC_ALERT_SOUND_NAME, m_staticAlertSoundName);
	DDX_Control(pDX, IDC_TIME_UNITS, m_comboTimeUnits);
	DDX_Control(pDX, IDC_TIME_COUNT, m_comboTimeCount);
	DDX_Control(pDX, IDC_SLIDER_VOLUME_MIDI, m_midiVolume);
	DDX_Control(pDX, IDC_SLIDER_VOLUME_WAVE, m_waveVolume);
	DDX_Control(pDX, IDC_SLIDER_VOLUME_ALERT, m_alertVolume);
	DDX_Check(pDX, IDC_CHECK_DISABLE, m_boolDisableSound);
	//}}AFX_DATA_MAP
}


BOOL ChSoundPrefs::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyBaseClass::OnSetActive();

	if (!m_boolInitialized)
	{
		InitVolumeSlider( m_midiVolume );
		InitVolumeSlider( m_waveVolume );
		InitVolumeSlider( m_alertVolume );

		ReadVolumeSliderPos( m_midiVolume, m_reg, SOUND_PREFS_MUSIC_VOLUME );
		ReadVolumeSliderPos( m_waveVolume, m_reg, SOUND_PREFS_EFFECTS_VOLUME );
		ReadVolumeSliderPos( m_alertVolume, m_reg, SOUND_PREFS_ALERT_VOLUME );

		UpdateAlertSoundName();
		InitAlertFreq();

		if (!GetMainInfo()->GetMidiPlayer()->DeviceExists())
		{
			m_midiVolume.EnableWindow( FALSE );
		}

		if (!GetMainInfo()->GetWavePlayer()->DeviceExists())
		{
			m_waveVolume.EnableWindow( FALSE );
			m_alertVolume.EnableWindow( FALSE );
		}
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = TRUE;
	}

	return boolResult;
}


void ChSoundPrefs::OnCommit()
{
	if (m_boolInitialized)
	{
		WriteVolumeSliderPos( m_midiVolume, m_reg, SOUND_PREFS_MUSIC_VOLUME );
		WriteVolumeSliderPos( m_waveVolume, m_reg, SOUND_PREFS_EFFECTS_VOLUME );
		WriteVolumeSliderPos( m_alertVolume, m_reg, SOUND_PREFS_ALERT_VOLUME );

		WriteAlertSound();

		m_alertTime.OnCommit();
											/* Perform updates in
												ChSoundMainInfo */
		GetMainInfo()->UpdateVolume();
											// Update our cached data

		GetMainInfo()->SetAlertSoundPath( GetAlertSoundPath() );
		GetMainInfo()->SetDisabled( IsDisabled() );
		GetMainInfo()->UpdateAlertTimes();
	}
}

#if defined( CH_PUEBLO_PLUGIN )

BOOL ChSoundPrefs::OnKillActive()
{
	UpdateData();
	OnCommit();
	return true;
}

#endif


void ChSoundPrefs::OnDestroy()
{
	GetMainInfo()->GetWavePlayer()->Stop();
	GetMainInfo()->GetMidiPlayer()->Stop();

	ChPropertyBaseClass::OnDestroy();
}


BEGIN_MESSAGE_MAP( ChSoundPrefs, ChPropertyBaseClass )
	//{{AFX_MSG_MAP(ChSoundPrefs)
	ON_BN_CLICKED(IDC_ALERT_SOUND, OnAlertSound)
	ON_WM_VSCROLL()
	ON_CBN_EDITCHANGE(IDC_TIME_COUNT, OnEditchangeTimeCount)
	ON_CBN_CLOSEUP(IDC_TIME_COUNT, OnCloseupTimeCount)
	ON_CBN_SELCHANGE(IDC_TIME_UNITS, OnSelchangeTimeUnits)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_DISABLE, OnCheckDisable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChSoundPrefs protected methods
----------------------------------------------------------------------------*/

void ChSoundPrefs::TestAlertSound()
{
	chuint16		suVolume;
	ChString			strAlertPath;

	ASSERT( GetMainInfo() );

	suVolume = GetSliderVolume( &m_alertVolume );

	if (!m_strAlertSoundPath.IsEmpty() &&
		ChUtil::FileExists( m_strAlertSoundPath ))
	{
		strAlertPath = m_strAlertSoundPath;
	}
	else
	{
		strAlertPath = GetSysSoundFilesPath() + "Tada.wav";
	}

	ChSoundInfo		info( false, true, suVolume );

	info.SetFilename( strAlertPath );
	info.SetDeviceType( devWave );

	GetMainInfo()->DoPlay( &info );
}


void ChSoundPrefs::TestWaveSound()
{
	chuint16	suVolume;
	ChString		strPath;

	suVolume = GetSliderVolume( &m_waveVolume );

	strPath = GetSysSoundFilesPath() + "Chimes.wav";

	ChSoundInfo		info( false, true, suVolume );

	info.SetFilename( strPath );
	info.SetDeviceType( devWave );

	ASSERT( GetMainInfo() );
	GetMainInfo()->DoPlay( &info );
}


void ChSoundPrefs::TestMidiSound()
{
	if (!m_strMidiFilePath.IsEmpty())
	{
		chuint16	suVolume;

		suVolume = GetSliderVolume( &m_midiVolume );

		ChSoundInfo		info( false, true, suVolume );

		info.SetFilename( m_strMidiFilePath );
		info.SetDeviceType( devMidi );

		ASSERT( GetMainInfo() );
		GetMainInfo()->DoPlay( &info );
	}
}


void ChSoundPrefs::InitAlertFreq()
{
	char	buffer[32];

	ltoa( m_alertTime.GetAlertCount(), buffer, 10 );
	m_comboTimeCount.SetWindowText( buffer );
	m_comboTimeUnits.SelectString( -1, m_alertTime.GetAlertUnitsName() );
}


void ChSoundPrefs::WriteAlertSound()
{
	m_reg.Write( SOUND_PREFS_ALERT_SOUND, m_strAlertSoundPath );
}


void ChSoundPrefs::UpdateAlertSoundName()
{
	if (m_strAlertSoundPath.IsEmpty())
	{
		ChString		strNoSound;

		LOADSTRING( IDS_NO_SOUND, strNoSound );
		m_staticAlertSoundName.SetWindowText( strNoSound );
	}
	else
	{
		m_staticAlertSoundName.SetWindowText( ExtractAlertSoundName() );
	}
}


ChString ChSoundPrefs::ExtractAlertSoundName()
{
	if (m_strAlertSoundPath.IsEmpty())
	{
		return "";
	}
	else
	{
		char	name[_MAX_FNAME];

		_splitpath( m_strAlertSoundPath, 0, 0, name, 0 );

		return ChString(name);
	}
}


ChString ChSoundPrefs::ExtractAlertSoundTitle()
{
	if (m_strAlertSoundPath.IsEmpty())
	{
		return "*.wav";
	}
	else
	{
		char	name[_MAX_FNAME];
		char	ext[_MAX_EXT];

		_splitpath( m_strAlertSoundPath, 0, 0, name, ext );

		ChString	strTitle( name );

		strTitle += ext;

		return strTitle;
	}
}


ChString ChSoundPrefs::ExtractAlertSoundPath()
{
	if (m_strAlertSoundPath.IsEmpty())
	{
		return GetSysSoundFilesPath();
	}
	else
	{
		char	drive[_MAX_DRIVE];
		char	dir[_MAX_DIR];

		_splitpath( m_strAlertSoundPath, drive, dir, 0, 0 );

		ChString	strPath( drive );

		strPath += dir;

		return strPath;
	}
}


void ChSoundPrefs::AdjustUI()
{
	if (!SoundDeviceFound() || IsDisabled())
	{										// Disable everything

		m_staticAlertSoundName.EnableWindow( false );
		m_staticOff.EnableWindow( false );
		m_staticMusic.EnableWindow( false );
		m_staticMsg.EnableWindow( false );
		m_staticMax.EnableWindow( false );
		m_staticEffects.EnableWindow( false );
		m_staticBox.EnableWindow( false );
		m_staticAlerts.EnableWindow( false );
		m_staticAlertSoundName.EnableWindow( false );
		m_comboTimeUnits.EnableWindow( false );
		m_comboTimeCount.EnableWindow( false );
		m_midiVolume.EnableWindow( false );
		m_waveVolume.EnableWindow( false );
		m_alertVolume.EnableWindow( false );
		m_btnAlertSound.EnableWindow( false );

		if (!SoundDeviceFound())
		{
			m_btnDisable.EnableWindow( false );
		}
	}
	else
	{										// Enable everything

		m_staticAlertSoundName.EnableWindow();
		m_staticOff.EnableWindow();
		m_staticMusic.EnableWindow();
		m_staticMsg.EnableWindow();
		m_staticMax.EnableWindow();
		m_staticEffects.EnableWindow();
		m_staticBox.EnableWindow();
		m_staticAlerts.EnableWindow();
		m_staticAlertSoundName.EnableWindow();
		m_comboTimeUnits.EnableWindow();
		m_comboTimeCount.EnableWindow();
		m_midiVolume.EnableWindow();
		m_waveVolume.EnableWindow();
		m_alertVolume.EnableWindow();
		m_btnAlertSound.EnableWindow();
	}

	if (SoundDeviceFound())
	{
		m_staticDisableMsg.SetWindowText( "" );
	}
	else
	{
		ChString		strExplanation;

#if defined( CH_PUEBLO_PLUGIN )
		if (ChUtil::Load( (chparam)AfxGetInstanceHandle(), IDS_PREF_NO_DEVICES,
							strExplanation ))
#else
					
		if (ChUtil::Load( (chparam)ChSoundDLL.hModule, IDS_PREF_NO_DEVICES,
							strExplanation ))
#endif					
		{
			m_staticDisableMsg.SetWindowText( strExplanation );
		}
	}
}


/*----------------------------------------------------------------------------
	ChSoundPrefs message handlers
----------------------------------------------------------------------------*/

BOOL ChSoundPrefs::OnInitDialog() 
{
	ChPropertyBaseClass ::OnInitDialog();

	AdjustUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void ChSoundPrefs::OnAlertSound()
{
 	ChSoundOpenFileDlg	fileDlg( GetMainInfo(),
 									GetSliderVolume( &m_alertVolume ),
 									ExtractAlertSoundPath(),
 									ExtractAlertSoundTitle() );
	int					iResult;

	iResult = fileDlg.DoModal();

	switch( iResult )
	{
		case IDOK:
		{
			m_strAlertSoundPath = fileDlg.GetPathName();

			WriteAlertSound();
			UpdateAlertSoundName();
			break;
		}

		case IDCANCEL:
		{
			break;
		}
	}
}


void ChSoundPrefs::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	ChPropertyBaseClass::OnVScroll( nSBCode, nPos, pScrollBar );

	ASSERT( pScrollBar );

	switch( nSBCode )
	{
		case SB_ENDSCROLL:
		{
			switch( pScrollBar->GetDlgCtrlID() )
			{
				case IDC_SLIDER_VOLUME_MIDI:
				{
					TestMidiSound();
					break;
				}

				case IDC_SLIDER_VOLUME_WAVE:
				{
					TestWaveSound();
					break;
				}

				case IDC_SLIDER_VOLUME_ALERT:
				{
					TestAlertSound();
					break;
				}

				default:
				{
					break;
				}
			}
			break;
		}
	}
}


void ChSoundPrefs::OnEditchangeTimeCount()
{
	ChString		strText;
	chint32		lUnits;
	char		buffer[32];

	m_comboTimeCount.GetWindowText( strText );

	lUnits = atol( strText );
	ltoa( lUnits, buffer, 10 );

	if (strText != buffer)
	{
		m_comboTimeCount.SetWindowText( buffer );
		m_comboTimeCount.SetEditSel( -1, -1 );
	}

	m_alertTime.SetAlertCount( lUnits );
}


void ChSoundPrefs::OnCloseupTimeCount()
{
	PostMessage( WM_COMMAND,
					MAKELONG( m_comboTimeCount.GetDlgCtrlID(),
								CBN_EDITCHANGE ),
					(LONG)m_hWnd );
}

void ChSoundPrefs::OnSelchangeTimeUnits()
{
	ChString		strText;
	int			iSel;

	iSel = m_comboTimeUnits.GetCurSel();
	if (iSel < 0)
	{
		iSel = 0;
	}
	m_comboTimeUnits.GetLBText( iSel, strText );

	m_alertTime.SetAlertUnits( strText );
}


void ChSoundPrefs::OnCheckDisable() 
{
	UpdateData();
	AdjustUI();
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:05  uecasm
// Import of source tree as at version 2.53 release.
//
