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

	Interface for the ChSoundPrefs property page.  This page handles
	preferences for the Sound module.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSPREFS_H )
#define _CHSPREFS_H

#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChReg.h>

#include "ChMPlay.h"

#if !defined( CH_PUEBLO_PLUGIN )
#include <ChPage.h>
#define ChPropertyBaseClass ChPropertyPage
#else
#define ChPropertyBaseClass CPropertyPage
#endif


/*----------------------------------------------------------------------------
	ChAlertTime class
----------------------------------------------------------------------------*/

class ChAlertTime
{
	public:
		typedef enum { seconds, minutes, hours } AlertUnitType;

	public:
		ChAlertTime( ChRegistry* pSoundPrefs );
		
		void OnCommit();

		inline void Update()
					{
						ReadAlertFreq();
					}
		inline chint32 GetAlertCount() { return m_lAlertCount; }
		inline chint32 GetAlertSeconds()
					{
						return m_lUnitsMultiplier * m_lAlertCount;
					}
		inline AlertUnitType GetAlertUnits() { return m_timeUnits; }
		inline ChString GetAlertUnitsName() { return GetUnitsName(); }

		void SetAlertCount( chint32 lCount );
		void SetAlertUnits( ChString strUnits );
		void SetAlertUnits( AlertUnitType newUnits );

	protected:
		ChString GetUnitsName();
		AlertUnitType TranslateUnitsName( const ChString& strUnitsName );

		void ReadAlertFreq();
		void WriteAlertFreq();

	protected:
		ChRegistry*		m_pSoundPrefs;
		bool			m_boolDirty;
		chint32			m_lAlertCount;
		AlertUnitType	m_timeUnits;
		chint32			m_lUnitsMultiplier;
};


/*----------------------------------------------------------------------------
	ChSoundPrefs class
----------------------------------------------------------------------------*/

class ChSoundPrefs : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE( ChSoundPrefs )

	public:
		enum { IDD = IDD_PREF_PAGE_SOUND };

	public:
		ChSoundPrefs();
		~ChSoundPrefs();

		inline const ChString& GetAlertSoundPath() const
					{
						return m_strAlertSoundPath;
					}
		inline bool IsDisabled() const
					{
						return (m_boolDisableSound != FALSE);
					}
		inline bool SoundDeviceFound() const
					{
						return m_boolSoundDeviceFound;
					}
		//inline void SetDeviceFound( bool boolSoundDeviceFound )
		//			{
		//				m_boolSoundDeviceFound = boolSoundDeviceFound;
		//			}

		void SetMainInfo( ChSoundMainInfo* pMainInfo );
											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();

		#if defined( CH_PUEBLO_PLUGIN )
		virtual BOOL OnKillActive( );
		#endif
											// Dialog Data
		//{{AFX_DATA(ChSoundPrefs)
		CButton	m_btnDisable;
		CButton	m_btnAlertSound;
		CStatic	m_staticOff;
		CStatic	m_staticMusic;
		CStatic	m_staticMsg;
		CStatic	m_staticMax;
		CStatic	m_staticEffects;
		CStatic	m_staticDisableMsg;
		CButton	m_staticBox;
		CStatic	m_staticAlerts;
		CStatic	m_staticAlertSoundName;
		CComboBox	m_comboTimeUnits;
		CComboBox	m_comboTimeCount;
		BOOL	m_boolDisableSound;
		//}}AFX_DATA

		CSliderCtrl		m_midiVolume;
		CSliderCtrl		m_waveVolume;
		CSliderCtrl		m_alertVolume;

		#if defined( CH_USE_VOXWARE )
		CSliderCtrl		m_speechVolume;
		#endif	// defined( CH_USE_VOXWARE )
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChSoundPrefs)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline ChSoundMainInfo* GetMainInfo() { return m_pMainInfo; }

		void TestAlertSound();
		void TestWaveSound();
		void TestMidiSound();

		#if defined( CH_USE_VOXWARE )

		void TestSpeechSound();
		
		#endif	// defined( CH_USE_VOXWARE )

		void InitAlertFreq();

		void WriteAlertSound();
		void UpdateAlertSoundName();

		ChString ExtractAlertSoundName();
		ChString ExtractAlertSoundTitle();
		ChString ExtractAlertSoundPath();

		void AdjustUI();

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChSoundPrefs)
		afx_msg void OnAlertSound();
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnEditchangeTimeCount();
		afx_msg void OnCloseupTimeCount();
		afx_msg void OnSelchangeTimeUnits();
		afx_msg void OnDestroy();
		virtual BOOL OnInitDialog();
		afx_msg void OnCheckDisable();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChSoundMainInfo*	m_pMainInfo;

		ChRegistry			m_reg;
		bool				m_boolInitialized;
		bool				m_boolSoundDeviceFound;

		ChString				m_strAlertSoundPath;	// includes name
		ChString				m_strMidiFilePath;		// includes name
		ChAlertTime			m_alertTime;
};

#endif	// !defined( _CHSPREFS_H )

// $Log$
