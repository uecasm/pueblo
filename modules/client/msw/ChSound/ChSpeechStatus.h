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

	Interface for the ChSpeechStatus class.  This class
	is used to display status information about Voxware TNT streaming
	speech.

----------------------------------------------------------------------------*/

// $Header$

#if defined( CH_USE_VOXWARE )
#if !defined( _CHSPEECHSTATUSWND_H_ )
#define _CHSPEECHSTATUSWND_H_


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define MIN_RECORD_LEVEL				95
#define MAX_RECORD_LEVEL				255
#define RECORD_LEVEL_BMP_OFFSET			3
#define RECORD_LEVEL_BMP_TICK_WIDTH		4


/*----------------------------------------------------------------------------
	ChSpeechStatusWnd class
----------------------------------------------------------------------------*/

class ChSpeechStatus;

class ChSpeechStatusWnd : public CMiniFrameWnd
{
	DECLARE_DYNCREATE( ChSpeechStatusWnd )

	public:
		enum { phone, button, recLevel, all };

	public:
		ChSpeechStatusWnd( ChSpeechStatus* pStatus );
		ChSpeechStatusWnd();
		virtual ~ChSpeechStatusWnd();

		bool Create( char* pstrWindowName, CWnd* pParentWnd );
		void Update( int iWhat );

		inline ChSpeechStatus* GetStatus() { return m_pStatus; }

	public:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChSpeechStatusWnd)
		//}}AFX_VIRTUAL

	protected:
		inline BOOL IsCaptured() { return m_boolCaptured; }

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChSpeechStatusWnd)
		afx_msg void OnPaint();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnClose();
		afx_msg UINT OnNcHitTest( CPoint point );
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		static CRect	m_rtInactive;
		static CRect	m_rtPhone;
		static CRect	m_rtButton;
		static CRect	m_rtRecordLevel;

		static CRect	m_rtButtonPress;

		CBitmap			m_bmpInactive;
		CBitmap			m_bmpPhone;
		CBitmap			m_bmpTalk;
		CBitmap			m_bmpListen;
		CBitmap			m_bmpRecordLevel;

		ChSpeechStatus*	m_pStatus;
		
		bool			m_boolCaptured;

		CWnd*			m_pParentWnd;
};



/*----------------------------------------------------------------------------
	ChSpeechStatus class
----------------------------------------------------------------------------*/

class ChSoundMainInfo;
class ChTNT;

class ChSpeechStatus
{
	public:
		ChSpeechStatus( ChSoundMainInfo* pMainInfo );
		virtual ~ChSpeechStatus();

		inline ChSoundMainInfo* GetMainInfo() { return m_pMainInfo; }

		inline bool IsConnected()
				{
					return m_boolConnected;
				}
		inline void SetConnected( bool boolConnected )
				{
					m_boolConnected = boolConnected;
					m_pWndStatus->Update( m_pWndStatus->all );
				}
		inline bool IsFullDuplex()
				{
					return m_boolFullDuplex;
				}
		inline void SetFullDuplex( bool boolFullDuplex )
				{
					m_boolFullDuplex = boolFullDuplex;
				}

		void ForceTalk( bool boolTalk );
		void DisplayTalk( bool boolTalk );
		void Show( bool boolShow );

		inline bool IsTalk()
				{
					if (IsFullDuplex())
					{
						return true;
					}
					else
					{
						return m_boolTalk;
					}
				}

		inline bool GetRecordLevel()
				{
					return m_iRecordLevel;
				}
		void SetRecordLevel( int iLevel );

		void OpenWnd();
		void CloseWnd();

		inline void SetTNT( ChTNT* pTNT )
				{
					m_pTNT = pTNT;
				}


	protected:
		ChSoundMainInfo*	m_pMainInfo;
		ChSpeechStatusWnd*	m_pWndStatus;
		bool				m_boolFullDuplex;
		bool				m_boolConnected;
		bool				m_boolTalk;
		int					m_iRecordLevel;
		ChTNT*				m_pTNT;
};

#endif	// !defined( _CHSPEECHSTATUSWND_H_ )
#endif	// defined( CH_USE_VOXWARE )

// $Log$
