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

	Interface for the ChTextInput class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTEXTINPUT_H )
#define _CHTEXTINPUT_H

#include "ChHist.h"
#include "ChKeyMapType.h"


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

CH_TYPEDEF_LIBRARY( bool, SubclassCtl3dProc )( HWND hwndCtl );

typedef enum { tabModeReset, tabModeStart, tabModeHistory,
				tabModeKeywords, tabModeEnd } TabCompletionMode;


/*----------------------------------------------------------------------------
	ChTextInputEdit class
----------------------------------------------------------------------------*/

class ChSplitterBanner;

class ChTextInputEdit : public CEdit
{
	public:
		enum tagConstants { maxHistory = 100,
							maxMenuHistory = 20,
							popupMenuHistoryBase = 10000 };

	public:
		ChTextInputEdit( bool boolPassword );

		inline ChWorldMainInfo* GetMainInfo() { return m_pMainInfo; }

		inline bool IsValid() { return (0 != m_hWnd); }
		inline bool IsControlKeyDown()
					{
						return !!(0x8000 & GetKeyState( VK_CONTROL ));
					}
		inline bool IsBrowsingHistory() { return m_boolBrowsingHistory; }
		inline bool IsPassword() { return m_boolPassword; }

		inline void SetSelectedText( ChString& strText )
					{
						SetWindowText( strText );
						SetSel( 0, -1 );
					}
		inline void SetHistoryText( ChString& strText )
					{
						int		iLen = strText.GetLength();

						SetWindowText( strText );
						SetSel( iLen, iLen );
					}

		BOOL Create( ChWnd* pParent, ChWorldMainInfo* pMainInfo );

		static void GetSize( int iLines, CSize& size );

		virtual BOOL OnChildNotify( UINT message, WPARAM wParam,
									LPARAM lParam, LRESULT* pResult );

		void Send( const ChString& strText );
		void Display( const ChString& strText );
		
		void EraseText();

		void UpdatePreferences();

	public:
#if !defined( CH_PUEBLO_PLUGIN )
		virtual BOOL PreTranslateMessage( MSG* pMsg );
#else
		virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
#endif

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChTextInputEdit)
		//}}AFX_VIRTUAL

											// Implementation
	public:
		virtual ~ChTextInputEdit();

	protected:
		void OnSendKey();
		void MoveInHistory( bool boolUp );
		void SetBrowsingHistory( bool boolBrowsing );
		void DoTabCompletion();
		void DoRightButtonMenu( CPoint ptMouse );
		void ConstructRightButtonMenu( CMenu& menu );

		int GetEndOfLineIndex( int iLine ) const;
		void SendKeyDown( UINT uiKey, LPARAM lParam, chflag32	flMods );
		bool ProcessKey( UINT& uiChar, LPARAM lParam );

	protected:
		ChWorldMainInfo*	m_pMainInfo;
		bool				m_boolPassword;

		bool				m_boolWindows95;
		HINSTANCE			m_h3dLib;
		SubclassCtl3dProc	m_pprocSubclassCtl3d;

		bool				m_boolClearOnSend;
		bool				m_boolBrowsingHistory;
		ChHistory			m_history;
		ChString				m_strEndText;

		ChKeyMapType		m_keyMapType;
		ChKeyMap			m_keyMap;

		TabCompletionMode	m_tabCompletionMode;
		ChPosition			m_posTabCompletion;
		ChString				m_strTabCompletion;

	protected:
		//{{AFX_MSG(ChTextInputEdit)
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
		#if defined( CH_PUEBLO_PLUGIN )
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		#endif

		afx_msg LONG OnGrabFocus( UINT wParam, LONG lParam );
		virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChTextInput class
----------------------------------------------------------------------------*/

class ChTextInput
{
	public:
		ChTextInput( ChWorldMainInfo* pMainInfo );
		~ChTextInput();

		inline bool IsShown() { return m_boolShown; }
		inline void SetLoginRecognized() { m_boolLoginRecognized = true; }

		inline void Paste() { GetEdit()->Paste(); }

		void Show( bool boolShow = true );
		void SetFocus();
		void Reset();
		void Clear();
		void SetEcho( bool boolEcho, bool boolPreserve );
		void SetInputLines( int iCount );

		void UpdatePreferences();

#if !defined( CH_PUEBLO_PLUGIN )
		bool CheckEditMenuItem( EditMenuItem item );
		void DoEditMenuItem( EditMenuItem item );
#endif

		void CheckForPasswordProtection();

	protected:
		inline ChWorldMainInfo* GetMainInfo() { return m_pMainInfo; }
		inline chint16 GetEditLines() const { return m_sEditLines; }
		inline ChTextInputEdit* GetEdit()
					{
						EchoState	echo = GetMainInfo()->GetEchoState();

						return ((echoOn == echo) ? m_pEdit : m_pPasswordEdit);
					}

		void CreateEditField();
		void SizeEditField();

		bool MatchMushLogin( const ChString& strText );

	protected:
		ChWorldMainInfo*	m_pMainInfo;

		ChSplitterBanner*	m_pBanner;
		ChTextInputEdit*	m_pEdit;
		ChTextInputEdit*	m_pPasswordEdit;

		bool				m_boolShown;
		bool				m_boolLoginRecognized;

		chint16				m_sEditLines;
};


#endif	// !defined( _CHTEXTINPUT_H )

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:53:15  uecasm
// Import of source tree as at version 2.53 release.
//
