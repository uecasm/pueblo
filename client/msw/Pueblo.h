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

	This file consists of the Chaco application class definition.

----------------------------------------------------------------------------*/

#if (!defined( _CHACO_H ))
#define _CHACO_H

#include <ChSplay.h>
//#include <ChModule.h>
//#include <ChDispat.h>

#include "ChAbout.h"

#ifndef __AFXWIN_H__
	#error include 'headers.h' before including this file for PCH
#endif
class ChMainFrame;
class ChRMenuMgr;
class ChHTTPDDE;


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define SECONDS_IN_A_DAY			(24 * 60 * 60)
#define REG_PROMPT_DAYS				(25)
#define REG_REQUIRE_DAYS			(30)


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

typedef ChPtrSplay<ChMainFrame>		ChFrameList;


/*----------------------------------------------------------------------------
	ChApp class
----------------------------------------------------------------------------*/

class ChPuebloFrameVisitor 
{
	public:
		virtual bool Visit( const ChMainFrame* pFrame ) = 0;

		virtual void Start() {};
		virtual void Stop() {};
};


/*----------------------------------------------------------------------------
	ChApp class
----------------------------------------------------------------------------*/

class ChApp : public CWinApp
{
	public:
		ChApp();
		virtual ~ChApp();

		void AddToFrameList( ChMainFrame* pFrame ); 
		void RemoveFromFrameList( ChMainFrame* pFrame ); 
		const ChMainFrame* GetNextFrame( ChMainFrame* pCurr );
		const ChMainFrame* FindFrame( const char* pstrFrameName );
		void EnumerateFrames( ChPuebloFrameVisitor* pEnumFrames );
		
		void CreateNewFrame( const ChString& strArgs = "",
								const ChString& strLabel = "" );

		void OnSecondTick( time_t timeCurr );
		void HandleOutdatedClient();

		static ChHTTPDDE* GetDDEConn();

	private:
		ChFrameList				m_pFrameList;
		ChSplashWnd				m_splash;			// Splash window
		ChRMenuMgr*				m_pCurrMenuMgr;
		CFrameWnd*				m_pCurrFrame;
		bool					m_boolInDelete;
		UINT					m_uiTimerID;
		bool					m_clientOutdated;

		static ChHTTPDDE*		m_pddeConn;
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChApp)
		public:
		virtual BOOL InitInstance();
		virtual int  ExitInstance();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
		virtual BOOL OnDDECommand(LPTSTR lpszCommand);
		//}}AFX_VIRTUAL
											// Implementation
	public:
		inline ChSplashWnd* GetSplashWnd() { return( &m_splash ); }

		bool TimeToRegister();
		void SetOutdatedTellLater();

		//{{AFX_MSG(ChApp)
		afx_msg void OnNewPuebloFrame();
		afx_msg void OnAppExit();
	afx_msg void OnUpdateNewPuebloFrame(CCmdUI* pCmdUI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHACO_H )
