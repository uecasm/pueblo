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

	Implementation of the ChSpeechPrefs property page.  This page handles
	preferences for the Sound module involving speech.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"


#if defined( CH_USE_VOXWARE )

#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChUtil.h>

#include "ChSpeechPrefs.h"
#include "ChSoundInfo.h"
#include "ChSoundUtils.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChSpeechPrefs class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChSpeechPrefs, ChPropertyBaseClass )

ChSpeechPrefs::ChSpeechPrefs() :
				ChPropertyBaseClass( ChSpeechPrefs::IDD, 0 
#if !defined( CH_PUEBLO_PLUGIN )
					, ChSoundDLL.hModule 
#endif					
				 ),
				m_reg( SOUND_PREFS_GROUP ),
				m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChSpeechPrefs)
	m_boolRejectCalls = FALSE;
	//}}AFX_DATA_INIT
											/* Read whether we're rejecting
												incoming calls */

	m_reg.ReadBool( SOUND_PREFS_REJECT_CALLS, m_boolRejectCalls,
					SOUND_PREFS_REJECT_CALLS_DEF );
}


ChSpeechPrefs::~ChSpeechPrefs()
{
}

void ChSpeechPrefs::SetMainInfo( ChSoundMainInfo* pMainInfo )
{
	m_pMainInfo = pMainInfo;
}


void ChSpeechPrefs::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyBaseClass::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChSpeechPrefs)
	DDX_Control(pDX, IDC_SLIDER_VOLUME_SPEECH, m_speechVolume);
	DDX_Control(pDX, IDC_SLIDER_MIKE_VOLUME, m_sliderMikeVolume);
	DDX_Check(pDX, IDC_CHECK_REJECT_CALLS, m_boolRejectCalls);
	//}}AFX_DATA_MAP
}


bool ChSpeechPrefs::OnSetActive()
{
	bool	boolResult;

	boolResult = ChPropertyBaseClass::OnSetActive();

	if (!m_boolInitialized)
	{
		ChUtil::GetAppDirectory( m_strSpeechFilePath );
		m_strSpeechFilePath += SPEECH_TEST_FILE_NAME;

		if (!ChUtil::FileExists( m_strSpeechFilePath ))
		{
			m_strSpeechFilePath = "";
		}

		InitSlider( m_sliderMikeVolume, MIKE_VOLUME_MAX );
		InitSlider( m_speechVolume, SPEECH_OUT_VOLUME_MAX );

		ReadSliderPos( m_sliderMikeVolume, SOUND_PREFS_MIKE_VOLUME,
						SOUND_PREFS_MIKE_VOLUME_DEF, MIKE_VOLUME_MAX );
		ReadSliderPos( m_speechVolume, SOUND_PREFS_SPEECH_VOLUME,
						SOUND_PREFS_SPEECH_VOLUME_DEF,
						SPEECH_OUT_VOLUME_MAX );

		if (!GetMainInfo()->GetSpeechPlayer()->DeviceExists())
		{
			m_speechVolume.EnableWindow( false );
		}
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChSpeechPrefs::OnCommit()
{
	if (m_boolInitialized)
	{
		WriteSliderPos( m_sliderMikeVolume, SOUND_PREFS_MIKE_VOLUME,
						MIKE_VOLUME_MAX);
		WriteSliderPos( m_speechVolume, SOUND_PREFS_SPEECH_VOLUME,
								SPEECH_OUT_VOLUME_MAX );

		m_reg.WriteBool( SOUND_PREFS_REJECT_CALLS, m_boolRejectCalls );

											// Perform updates
		GetMainInfo()->UpdateSpeechPrefs();
	}
}


#if defined( CH_PUEBLO_PLUGIN )

BOOL ChSpeechPrefs::OnKillActive()
{
	UpdateData();
	OnCommit();
	return true;
}

#endif	// !defined( CH_PUEBLO_PLUGIN )


BEGIN_MESSAGE_MAP( ChSpeechPrefs, ChPropertyBaseClass )
	//{{AFX_MSG_MAP(ChSpeechPrefs)
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChSpeechPrefs protected methods
----------------------------------------------------------------------------*/

void ChSpeechPrefs::InitSlider( CSliderCtrl& slider, int iMax )
{
	slider.SetRange( 0, iMax );
	slider.SetLineSize( (iMax + 1) / SLIDER_LINES );
	slider.SetPageSize( (iMax + 1) / SLIDER_PAGES );
	slider.SetTicFreq( (iMax + 1) / SLIDER_TICKS );
}


void ChSpeechPrefs::ReadSliderPos( CSliderCtrl& slider, const char* strRegKey,
									int iDefault, int iMax )
{
	chint32		lVolume;
	chuint16	suPos;

	m_reg.Read( strRegKey, lVolume, iDefault );
	if (lVolume > iMax)
	{
		lVolume = iMax;
	}
	if (lVolume < 0)
	{
		lVolume = 0;
	}
	suPos = (chuint16)lVolume;

	suPos = InvertSliderPos( suPos, iMax );
	slider.SetPos( suPos );
}


void ChSpeechPrefs::WriteSliderPos( CSliderCtrl& slider, const char* strRegKey,
									int iMax )
{
	chint32		lVolume;
	int			iPos;

	iPos = slider.GetPos();
	iPos = InvertSliderPos( (chuint16)iPos, iMax );
	lVolume = iPos;

	ASSERT( lVolume <= iMax );

	m_reg.Write( strRegKey, lVolume );
}


int ChSpeechPrefs::InvertSliderPos( int iPos, int iMax )
{
	iPos -= iMax;
	iPos = abs( iPos );
											// Range check
	if (iPos > iMax)
	{
		iPos = iMax;
	}

	return iPos;
}


int ChSpeechPrefs::GetSliderPos( CSliderCtrl& slider, int iMax )
{
	int		iPos;

	iPos = m_speechVolume.GetPos();
	iPos = InvertSliderPos( (chuint16)iPos, iMax );

	return iPos;
}


void ChSpeechPrefs::TestSpeechSound()
{
	if (!m_strSpeechFilePath.IsEmpty())
	{
		int		iPos = GetSliderPos( m_speechVolume, SPEECH_OUT_VOLUME_MAX );

		ChSoundInfo		info( false, true, iPos );

		info.SetFilename( m_strSpeechFilePath );
		info.SetDeviceType( devSpeech );

		ASSERT( GetMainInfo() );
		GetMainInfo()->DoPlay( &info );
	}
}


/*----------------------------------------------------------------------------
	ChSpeechPrefs message handlers
----------------------------------------------------------------------------*/

void ChSpeechPrefs::OnVScroll( UINT nSBCode, UINT nPos,
								CScrollBar* pScrollBar )
{
	ChPropertyBaseClass::OnVScroll( nSBCode, nPos, pScrollBar );

	ASSERT( pScrollBar );

	switch( nSBCode )
	{
		case SB_ENDSCROLL:
		{
			switch( pScrollBar->GetDlgCtrlID() )
			{
				case IDC_SLIDER_VOLUME_SPEECH:
				{
					TestSpeechSound();
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


void ChSpeechPrefs::OnDestroy() 
{
	ChPropertyBaseClass::OnDestroy();

	GetMainInfo()->GetSpeechPlayer()->Stop();
}


#endif	// defined( CH_USE_VOXWARE )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:03  uecasm
// Import of source tree as at version 2.53 release.
//
