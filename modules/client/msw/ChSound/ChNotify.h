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

	Implementation of the ChNotify window.  This window is used to recieve
	notification messages.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHNOTIFY_H )
#define _CHNOTIFY_H

class ChMediaPlayer;


/*----------------------------------------------------------------------------
	User-defined messages
----------------------------------------------------------------------------*/

											/* The following message is sent
												when a Vox file finishes
												playing */

#define MSG_ON_VOX_COMPLETE			(WM_USER + 1000)


/*----------------------------------------------------------------------------
	ChNotifyWnd class
----------------------------------------------------------------------------*/

class ChNotifyWnd : public CWnd
{
	public:
		ChNotifyWnd();

		inline ChMediaPlayer* GetPlayer() { return m_pMediaPlayer; }
		inline void SetPlayer( ChMediaPlayer* pMediaPlayer )
				{
					m_pMediaPlayer = pMediaPlayer;
				}

		BOOL Create();
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChNotifyWnd)
		//}}AFX_VIRTUAL

	public:
		virtual ~ChNotifyWnd();

	protected:
		//{{AFX_MSG(ChNotifyWnd)
		afx_msg LONG OnMciNotify( UINT wParam, LONG lParam );
		afx_msg LONG OnVoxComplete( UINT wParam, LONG lParam );
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChMediaPlayer*	m_pMediaPlayer;
};

#endif	// !defined( _CHNOTIFY_H )

// $Log$
