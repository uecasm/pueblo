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

	Interface for the ChSpeechPrefs property page.  This page handles
	preferences for the Sound module involving speech.

----------------------------------------------------------------------------*/

// $Header$

#if defined( CH_USE_VOXWARE )

#if !defined( _CHSPEECHPREFS_H )
#define _CHSPEECHPREFS_H

#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChReg.h>

#if !defined( CH_PUEBLO_PLUGIN )
#include <ChPage.h>
#define ChPropertyBaseClass ChPropertyPage
#else
#define ChPropertyBaseClass CPropertyPage
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define SLIDER_TICKS				8
#define SLIDER_PAGES				4
#define SLIDER_LINES				16
#define MIKE_VOLUME_MAX				100
#define SPEECH_OUT_VOLUME_MAX		100

#define SPEECH_TEST_FILE_NAME		"ChTest.vox"


/*----------------------------------------------------------------------------
	ChSpeechPrefs class
----------------------------------------------------------------------------*/

class ChSoundMainInfo;

class ChSpeechPrefs : public ChPropertyBaseClass
{
	DECLARE_DYNCREATE( ChSpeechPrefs )

	public:
		ChSpeechPrefs();
		~ChSpeechPrefs();

		void SetMainInfo( ChSoundMainInfo* pMainInfo );
											// Overrides
		virtual bool OnSetActive();
		virtual void OnCommit();

		#if defined( CH_PUEBLO_PLUGIN )
		virtual BOOL OnKillActive( );
		#endif
											// Dialog Data
		//{{AFX_DATA(ChSpeechPrefs)
		enum { IDD = IDD_PREF_PAGE_SPEECH };
		CSliderCtrl	m_speechVolume;
		CSliderCtrl	m_sliderMikeVolume;
		bool	m_boolRejectCalls;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChSpeechPrefs)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline ChSoundMainInfo* GetMainInfo() { return m_pMainInfo; }

		void InitSlider( CSliderCtrl& slider, int iMax );
		void ReadSliderPos( CSliderCtrl& slider, const char* strRegKey,
								int iDefault, int iMax );
		void WriteSliderPos( CSliderCtrl& slider, const char* strRegKey,
								int iMax );
		int InvertSliderPos( int iPos, int iMax );
		int GetSliderPos( CSliderCtrl& slider, int iMax );

		void TestSpeechSound();

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChSpeechPrefs)
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnDestroy();
		//}}AFX_MSG
	
		DECLARE_MESSAGE_MAP()

	protected:
		ChSoundMainInfo*	m_pMainInfo;

		bool				m_boolInitialized;
		ChRegistry			m_reg;

		ChString				m_strSpeechFilePath;	// Includes file name
};

#endif	// defined( CH_USE_VOXWARE )

#endif	// !defined( _CHSPEECHPREFS_H )

// $Log$
