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

	Implementation of the ChSpeechStatus class.  This class
	is used to display status information about Voxware TNT streaming
	speech.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"


#if defined( CH_USE_VOXWARE )

#include <ChCore.h>

#include "ChSound.h"
#include "ChSoundInfo.h"
#include "ChSpeechStatus.h"

#if defined( _DEBUG )
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

/*----------------------------------------------------------------------------
	ChSpeechStatus class
----------------------------------------------------------------------------*/

ChSpeechStatus::ChSpeechStatus( ChSoundMainInfo* pMainInfo ) :
	m_pMainInfo( pMainInfo ),
	m_pWndStatus( 0 ),
	m_boolFullDuplex( false ),
	m_boolConnected( false ),
	m_boolTalk( true ),
	m_pTNT( 0 )
{
	OpenWnd();
}


ChSpeechStatus::~ChSpeechStatus()
{
	CloseWnd();
}


void ChSpeechStatus::ForceTalk( bool boolTalk )
{
	if (!IsFullDuplex())
	{										/* For full duplex, you can always
												talk... */
		m_pTNT->ForceTalk( boolTalk );
	}
}


void ChSpeechStatus::DisplayTalk( bool boolTalk )
{
	if (IsFullDuplex())
	{										/* For full duplex, you can always
												talk... */
		boolTalk = true;
	}

	if (IsConnected())
	{
		ChString		strMessage;
		int			iMessage;

		if (!IsFullDuplex())
		{
			if (boolTalk)
			{
				iMessage = IDS_SPEECH_MODE_TALK;
			}
			else
			{
				iMessage = IDS_SPEECH_MODE_LISTEN;
			}

			strMessage.LoadString( iMessage );
			GetMainInfo()->GetCore()->DisplayStatus( strMessage );

			if (m_pWndStatus)
			{
				m_pWndStatus->Update( m_pWndStatus->button );
			}
		}
	}

	m_boolTalk = boolTalk;
}


void ChSpeechStatus::SetRecordLevel( int iLevel )
{
	if (iLevel < MIN_RECORD_LEVEL)
	{
		iLevel = MIN_RECORD_LEVEL;
	}
	else if (iLevel > MAX_RECORD_LEVEL)
	{
		iLevel = MAX_RECORD_LEVEL;
	}

	iLevel -= MIN_RECORD_LEVEL;

	if (iLevel != GetRecordLevel())
	{
		m_iRecordLevel = iLevel;
		m_pWndStatus->Update( m_pWndStatus->recLevel );
	}
}


void ChSpeechStatus::Show( bool boolShow )
{
	if (boolShow)
	{
		OpenWnd();
	}
	else
	{
		CloseWnd();
	}
}


void ChSpeechStatus::OpenWnd()
{
	if (0 == m_pWndStatus)
	{
		m_pWndStatus = new ChSpeechStatusWnd( this );
		m_pWndStatus->Create( "Speech", GetMainInfo()->GetCore()->GetFrameWnd() );
	}
}


void ChSpeechStatus::CloseWnd()
{
	if (m_pWndStatus)
	{
		m_pWndStatus->DestroyWindow();
		m_pWndStatus = 0;
	}
}


/*----------------------------------------------------------------------------
	ChSpeechStatusWnd class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChSpeechStatusWnd, CMiniFrameWnd )

CRect		ChSpeechStatusWnd::m_rtInactive( 0, 0, 87, 31 );
CRect		ChSpeechStatusWnd::m_rtPhone( 0, 0, 39, 31 );
CRect		ChSpeechStatusWnd::m_rtButton( 39, 0, 87, 31 );
CRect		ChSpeechStatusWnd::m_rtButtonPress( 43, 8, 83, 25 );
CRect		ChSpeechStatusWnd::m_rtRecordLevel( 0, 31, 87, 40 );

ChSpeechStatusWnd::ChSpeechStatusWnd( ChSpeechStatus* pStatus ) :
						m_boolCaptured( false )
{
	m_pStatus = pStatus;
}

ChSpeechStatusWnd::ChSpeechStatusWnd() :
						m_boolCaptured( false )
{
	m_pStatus = 0;
}

ChSpeechStatusWnd::~ChSpeechStatusWnd()
{
}

bool ChSpeechStatusWnd::Create( char* pstrWindowName, CWnd* pParentWnd )
{
	CWnd*	pNextParentWnd;
	CRect	rtFrame( 10, 10, 97, 49 );
											/* Find topmost parent for a
												good placement */

	while (pNextParentWnd = pParentWnd->GetParent())
	{
		pParentWnd = pNextParentWnd;
	}
											// Store temporarily
	m_pParentWnd = pParentWnd;

	m_bmpInactive.LoadBitmap( IDB_SPEECH_INACTIVE );
	m_bmpPhone.LoadBitmap( IDB_SPEECH_PHONE );
	m_bmpTalk.LoadBitmap( IDB_SPEECH_TALK );
	m_bmpListen.LoadBitmap( IDB_SPEECH_LISTEN );
	m_bmpRecordLevel.LoadBitmap( IDB_SPEECH_RECORD_LEVEL );

	return CMiniFrameWnd::Create( 0, pstrWindowName,
									WS_CAPTION | WS_SYSMENU |
										MFS_SYNCACTIVE | MFS_MOVEFRAME |
										MFS_4THICKFRAME,
									rtFrame, pParentWnd );
}


void ChSpeechStatusWnd::Update( int iWhat )
{
	switch( iWhat )
	{
		case phone:
		{
			InvalidateRect( &m_rtPhone, false );
		}

		case button:
		{
			InvalidateRect( &m_rtButton, false );
		}

		case recLevel:
		{
			InvalidateRect( &m_rtRecordLevel, false );
		}

		case all:
		default:
		{
			InvalidateRect( NULL, false );
		}
	}
}


BEGIN_MESSAGE_MAP( ChSpeechStatusWnd, CMiniFrameWnd )
	//{{AFX_MSG_MAP(ChSpeechStatusWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_NCHITTEST()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChSpeechStatusWnd message handlers
----------------------------------------------------------------------------*/

void ChSpeechStatusWnd::OnPaint() 
{
	CPaintDC	dc( this );					// Device context for painting
	CRect		rtPaint( dc.m_ps.rcPaint );
	ChString		strMessage( "Hello, world!" );
	CDC			dcSrc;
	CBitmap*	pOldBitmap;
	CBitmap*	pSrcBitmap;
	int			iLevel;
	CRect		rtLevel = m_rtRecordLevel;
	int			iWidth;

	dcSrc.CreateCompatibleDC( &dc );

	if (GetStatus()->IsConnected())
	{
											// Paint the phone...

		pOldBitmap = (CBitmap*)dcSrc.SelectObject( &m_bmpPhone );
		dc.BitBlt( m_rtPhone.left, m_rtPhone.top,
					m_rtPhone.Width(), m_rtPhone.Height(),
					&dcSrc, 0, 0, SRCCOPY );

											// Paint the button...

		pSrcBitmap = GetStatus()->IsTalk() ? &m_bmpTalk : &m_bmpListen;

		dcSrc.SelectObject( pSrcBitmap );
		dc.BitBlt( m_rtButton.left, m_rtButton.top,
					m_rtButton.Width(), m_rtButton.Height(),
					&dcSrc, 0, 0, SRCCOPY );

		iLevel = GetStatus()->GetRecordLevel();
	}
	else
	{
		pOldBitmap = (CBitmap*)dcSrc.SelectObject( &m_bmpInactive );
		dc.BitBlt( m_rtInactive.left, m_rtInactive.top,
					m_rtInactive.Width(), m_rtInactive.Height(),
					&dcSrc, m_rtInactive.left, m_rtInactive.top, SRCCOPY );

		iLevel = 0;
	}
											// Paint the gain level...

	iWidth = RECORD_LEVEL_BMP_OFFSET +
				((iLevel / 8) * RECORD_LEVEL_BMP_TICK_WIDTH);
	if (iLevel > 0)
	{

		dcSrc.SelectObject( &m_bmpRecordLevel );

		dc.BitBlt( m_rtRecordLevel.left, m_rtRecordLevel.top,
					iWidth, m_rtRecordLevel.Height(),
					&dcSrc, 0, 0, SRCCOPY );
	}

	rtLevel.left = iWidth;
	dc.SetBkColor( RGB( 192, 192, 192 ) );
	dc.ExtTextOut( 0, 0, ETO_OPAQUE, rtLevel, "", 0, NULL );

											// Clean up
	dcSrc.SelectObject( pOldBitmap );
}


int ChSpeechStatusWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	CRect	rtFrameSize( 0, 0, 87, 39 );
	CRect	rtParent;

	if (CMiniFrameWnd::OnCreate( lpCreateStruct ) == -1)
	{
		return -1;
	}

	CalcWindowRect( rtFrameSize );

	ASSERT( 0 != m_pParentWnd );
	m_pParentWnd->GetWindowRect( &rtParent );
	rtParent.left = rtParent.right -
					(rtFrameSize.Width() +
						(3 * GetSystemMetrics( SM_CXMENUSIZE )) +
						(2 * GetSystemMetrics( SM_CXSIZEFRAME )));

	SetWindowPos( 0, rtParent.left, rtParent.top,
					rtFrameSize.Width(), rtFrameSize.Height(),
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW );
	return 0;
}


BOOL ChSpeechStatusWnd::OnEraseBkgnd( CDC* pDC ) 
{
	return true;
}


void ChSpeechStatusWnd::OnClose() 
{
	GetStatus()->CloseWnd();
}


UINT ChSpeechStatusWnd::OnNcHitTest( CPoint point )
{
	int		iLoc;
	CPoint	ptClient;

	ptClient = point;
	ScreenToClient( &ptClient );

	iLoc = CMiniFrameWnd::OnNcHitTest( point );
	if ((HTCLIENT == iLoc) && !m_rtButtonPress.PtInRect( ptClient ))
	{
										/* Make the window draggable
											(except for the button) */
		iLoc = HTCAPTION;
	}

	return iLoc;
}

void ChSpeechStatusWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	if (GetStatus()->IsConnected() && m_rtButtonPress.PtInRect( point ))
	{
		m_boolCaptured = true;
		SetCapture();

		GetStatus()->ForceTalk( true );
	}
	else
	{
		CMiniFrameWnd::OnLButtonDown(nFlags, point);
	}
}

void ChSpeechStatusWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	if (IsCaptured())
	{
		ReleaseCapture();
		m_boolCaptured = false;

		GetStatus()->ForceTalk( false );
	}
	else
	{
		CMiniFrameWnd::OnLButtonUp( nFlags, point );
	}
}

#endif	// defined( CH_USE_VOXWARE )

// $Log$
