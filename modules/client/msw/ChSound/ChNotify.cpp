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

#include "headers.h"

#include "ChSound.h"
#include "ChNotify.h"
#include "ChMPlay.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChNotifyWnd class
----------------------------------------------------------------------------*/

ChNotifyWnd::ChNotifyWnd() : m_pMediaPlayer( 0 )
{
}


ChNotifyWnd::~ChNotifyWnd()
{
}


BEGIN_MESSAGE_MAP( ChNotifyWnd, CWnd )
	//{{AFX_MSG_MAP(ChNotifyWnd)
	ON_MESSAGE( MM_MCINOTIFY, OnMciNotify )
	ON_MESSAGE( MSG_ON_VOX_COMPLETE, OnVoxComplete )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL ChNotifyWnd::Create() 
{
	CRect	rtWnd( -100, -100, -100, -100 );

	return CWnd::Create( 0, "ChMidi Notification Sink", WS_OVERLAPPED,
							rtWnd, GetDesktopWindow(), 0 );
}


/*----------------------------------------------------------------------------
	ChNotifyWnd message handlers
----------------------------------------------------------------------------*/

LONG ChNotifyWnd::OnMciNotify( UINT wParam, LONG lParam )
{
	switch( wParam )
	{
		case MCI_NOTIFY_SUCCESSFUL:
		{									/* This notification is send when
												play terminates normally */
			ASSERT( GetPlayer() != 0 );

			GetPlayer()->OnPlayComplete();
			break;
		}

		case MCI_NOTIFY_ABORTED:
		{									/* This notification is sent when
												a play in progress is
												terminated */
			break;
		}

		case MCI_NOTIFY_SUPERSEDED:
		{
			break;
		}

		case MCI_NOTIFY_FAILURE:
		{
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


LONG ChNotifyWnd::OnVoxComplete( UINT wParam, LONG lParam )
{
	TRACE( "Voxware: Got play complete notification.\n" );
	GetPlayer()->OnPlayComplete();

	return 0;
}

// $Log$
