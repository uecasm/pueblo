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

     Ultra Enterprises   Gavin Lambert
     
          Modified to use ChPropertySheet (instead of CPropertySheet),
          which includes modified behaviour to allow using a custom
          font in the property sheet.

------------------------------------------------------------------------------

	This file consists of implementations of the ChAbout class, ChSplashWnd
	class, and ChBigIcon class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <afxcmn.h>			// AFX common controls
#include <time.h>
#include <ChClInfo.h>
#include <ChDibDecoder.h>

// only really to get version numbers, but oh well
#ifndef __BORLANDC__
#define HAVE_BOOLEAN
#endif
#define XMD_H
#include "libmng.h"

#include "ChAbout.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define COLOR_LT_GRAY		RGB( 0x80, 0x80, 0x80 )
#define COLOR_WHITE			RGB( 0xff, 0xff, 0xff )


/*----------------------------------------------------------------------------
	ChLogoBitmap class
----------------------------------------------------------------------------*/

BEGIN_MESSAGE_MAP( ChLogoBitmap, CButton )
	//{{AFX_MSG_MAP(ChLogoBitmap)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------

	FUNCTION	||	ChLogoBitmap::SizeToContent

------------------------------------------------------------------------------

	This function will size a bitmap button to the size of the bitmap.
	This method MUST be called.

----------------------------------------------------------------------------*/

void ChLogoBitmap::SizeToContent( chflag16 fAlignment, WORD resId )
{
	ChDibDecoder	dibDecoder( &m_logoBmp );
	if (dibDecoder.Load( resId, AfxGetApp()->m_hInstance ))
	{
		m_lBmpHeight = m_logoBmp.GetHeight();
		m_lBmpWidth = m_logoBmp.GetWidth();
	}
	else
	{
		m_lBmpHeight = 0;
		m_lBmpWidth = 0;
	}

	if (m_lBmpHeight && m_lBmpWidth)
	{
		CRect	rtParent;
		chint32	lTop = 5;
		chint32	lLeft = 5;
		chint32	lWidth = m_lBmpWidth;
		chint32	lHeight = m_lBmpHeight;

		GetParent()->GetClientRect( &rtParent );

		if (rtParent.Height() < lHeight)
		{
			lHeight = rtParent.Height() - 10;
		}
											// Calculate alignment
		if (fAlignment & right)
		{
			lLeft = rtParent.Width() - (m_lBmpWidth + 5);
		}
		else if (fAlignment & hcenter)
		{
			lLeft = (rtParent.Width() - m_lBmpWidth) / 2;
		}

		if (fAlignment & bottom)
		{
			lTop = rtParent.Height() - (m_lBmpHeight + 5);
		}
		else if (fAlignment & vcenter)
		{
			lTop = (rtParent.Height() - m_lBmpHeight) / 2;
		}
											// Reposition the window

		SetWindowPos( 0, (int)lLeft, (int)lTop, (int)lWidth, (int)lHeight,
						SWP_NOACTIVATE | SWP_NOZORDER );
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChLogoBitmap::DrawItem

------------------------------------------------------------------------------

	This function will draw the contents of the bitmap button.

----------------------------------------------------------------------------*/

void ChLogoBitmap::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (0 == m_logoBmp.GetWidth())
	{
		return;								// Bitmap load unsuccessful
	}

	CDC*		pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	CRect		rect;

	ASSERT( pDC != 0 );

	GetClientRect( rect );

  	m_logoBmp.SetSize( rect.Width(), rect.Height() );
  	m_logoBmp.Draw( pDC, 0, 0, COLOR_WHITE );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChLogoBitmap::OnEraseBkgnd

------------------------------------------------------------------------------

	This function will handle erase processing for the class.

----------------------------------------------------------------------------*/

BOOL ChLogoBitmap::OnEraseBkgnd( CDC* )
{
	return true;    // we don't do any erasing...
}


/*----------------------------------------------------------------------------
				ChSplashWnd class
----------------------------------------------------------------------------*/

BEGIN_MESSAGE_MAP( ChSplashWnd, CDialog )
	//{{AFX_MSG_MAP(ChSplashWnd)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSplashWnd::DoDataExchange

------------------------------------------------------------------------------

	This function is called by the framework to exchange and validate
	dialog data.

----------------------------------------------------------------------------*/

void ChSplashWnd::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChSplashWnd)
	DDX_Control(pDX, IDC_COPYRIGHT2, m_staticCopyright2);
	DDX_Control(pDX, IDC_COPYRIGHT1, m_staticCopyright1);
	DDX_Control(pDX, IDC_VERSION_STRING, m_staticVersionString);
	//}}AFX_DATA_MAP
}


/*----------------------------------------------------------------------------
	ChSplashWnd message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSplashWnd::Create

------------------------------------------------------------------------------

	This function is called to create the splash window dialog.

----------------------------------------------------------------------------*/

bool ChSplashWnd::Create( CWnd* pParent )
{
	//{{AFX_DATA_INIT(ChSplashWnd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	if (!CDialog::Create( ChSplashWnd::IDD, pParent ))
	{
		TRACE0( "Warning: creation of ChSplashWnd dialog failed\n" );
		return( false );
	}

	return( true );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChSplashWnd::OnInitDialog

------------------------------------------------------------------------------

	This function is called when the dialog is initialized.  We use this
	handler to subclass the owner-draw button.

----------------------------------------------------------------------------*/

BOOL ChSplashWnd::OnInitDialog()
{
	ChClientInfo	clientInfo( ChClientInfo::thisMachine );
	ChVersion		clientVer = clientInfo.GetClientVersion();
	ChString			strVerNum( clientVer.Format( ChVersion::formatShort ) );
	CWnd*			pVersionWnd = GetDlgItem( IDC_VERSION_STRING );
	ChString			strFormat;
	ChString			strVersion;

	CDialog::OnInitDialog();
											// Center on the desktop
	CenterWindow( GetDesktopWindow() );
											// Initialize the Chaco logo

	logoBmp.SubclassDlgItem( IDC_CHACO_LOGO, this );
	logoBmp.SizeToContent( ChLogoBitmap::hcenter | ChLogoBitmap::top,
													IDR_CHACO_DIB );

											// Set the version string correctly
	pVersionWnd->GetWindowText( strFormat );
	#if defined( CH_ARCH_16 )
	{
		int		iLen = strFormat.GetLength() + strVerNum.GetLength() + 2;
		char*	pstrVersion = new char[iLen];

		::wsprintf( pstrVersion,  strFormat, (const char*)strVerNum );
		strVersion = pstrVersion;

		delete [] pstrVersion;
	}
	#else
	{
		strVersion.Format( strFormat, (const char*)strVerNum );
	}
	#endif

	pVersionWnd->SetWindowText( strVersion );

	{										/* Set fonts for static fields in
												the page to make it more
												attractive */
		HDC			hDC = ::GetDC( m_hWnd );
		LOGFONT		lf;
		int 		iPixelSize = ::GetDeviceCaps( hDC, LOGPIXELSY );
		CFont		font;

		::ReleaseDC( m_hWnd, hDC );
											// Clear the font structure
		ChMemClearStruct( &lf );

		lf.lfHeight = -1 * (iPixelSize * 14 / 72);
		lf.lfWeight = FW_BOLD;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy( lf.lfFaceName, "Arial" );

		font.CreateFontIndirect( &lf );
		m_staticVersionString.SetFont(&font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 9 / 72);
		lf.lfWeight = FW_BOLD;
		font.CreateFontIndirect( &lf );
		m_staticCopyright1.SetFont(&font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 8 / 72);
		lf.lfWeight = FW_NORMAL;
		font.CreateFontIndirect( &lf );
		m_staticCopyright2.SetFont(&font);
		font.Detach();
	}
  											/* Return true unless you set the
  												focus to a control */
	return true;
}


void ChSplashWnd::OnDestroy()
{
	CDialog ::OnDestroy();
}


/*----------------------------------------------------------------------------
	ChAbout property sheet class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChAbout, ChPropertySheet )

ChAbout::ChAbout( ChCore* pCore, chuint16 suIDCaption, CWnd *pParentWnd,
					chuint16 suSelectPage ) :
			ChPropertySheet( suIDCaption, pParentWnd, suSelectPage ),
			m_pageMgr( pCore, pageAbout )
{
	//UETRACE("ChAbout constructor: ID caption");
}


ChAbout::ChAbout( ChCore* pCore, char *pstrCaption, CWnd *pParentWnd,
					chuint16 suSelectPage ) :
			ChPropertySheet( pstrCaption, pParentWnd, suSelectPage ),
			m_pageMgr( pCore, pageAbout )
{
	//UETRACE("ChAbout constructor: string caption");
}


ChAbout::~ChAbout()
{
	//UETRACE("ChAbout destructor");
}


void ChAbout::AddModulePages()
{
	//UETRACE("ChAbout::AddModulePages");
	m_pageMgr.AddModulePages( this );
}


BEGIN_MESSAGE_MAP( ChAbout, ChPropertySheet )
	//{{AFX_MSG_MAP(ChAbout)
	ON_COMMAND(IDHELP, OnHelp)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_CLOSE()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAbout message handlers
----------------------------------------------------------------------------*/

void ChAbout::OnOK()
{											/* Grab the data for the current
												sheet */
	//UETRACE("ChAbout::OnOK");
	if (GetActivePage()->OnKillActive())
	{										// Get the data from the pages
		m_pageMgr.GetPageData();

		#if ( _MFC_VER > 0x0400	 )
		EndDialog( IDOK );
		#else
		ChPropertySheet::OnOK(  );
		#endif
											/* Release the pages for other
												modules */
		//m_pageMgr.ReleaseModulePages();
	}
}

void ChAbout::OnCancel()
{
	//UETRACE("ChAbout::OnCancel");
	#if ( _MFC_VER > 0x0400	 )
	EndDialog( IDCANCEL );
	#else
	ChPropertySheet::OnOK(  );
	#endif
											/* Release the pages for other
												modules */
	//m_pageMgr.ReleaseModulePages();
}


void ChAbout::OnClose() 
{
	//UETRACE("ChAbout::OnClose");
	ChPropertySheet ::OnClose();
											/* Release the pages for other
												modules */
	//m_pageMgr.ReleaseModulePages();
}


void ChAbout::OnHelp()
{											/* This method gets the help
												button press and passes it on
												to the currently active
												page to process */
	//UETRACE("ChAbout::OnHelp");
	WinHelp( 0x20000 + IDD_ABOUT_BOX );
}

/*
static void RemapWindowUnits(CWnd *wnd, const CPoint& nold, const CPoint& nnew) {
	CRect rWnd;
	wnd->GetWindowRect(rWnd);
	rWnd.left = MulDiv(rWnd.left, nnew.x, nold.x);
	rWnd.top = MulDiv(rWnd.top, nnew.y, nold.y);
	rWnd.right = MulDiv(rWnd.right, nnew.x, nold.x);
	rWnd.bottom = MulDiv(rWnd.bottom, nnew.y, nold.y);
	::MapWindowPoints(0, wnd->GetParent()->GetSafeHwnd(),
					(LPPOINT)(LPRECT)rWnd, 2);
	wnd->SetWindowPos(0, 0, 0, rWnd.Width(), rWnd.Height(),
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}
*/

BOOL ChAbout::OnInitDialog()
{
	//UETRACE("ChAbout::OnInitDialog");
	BOOL bResult = ChPropertySheet::OnInitDialog();

	{
		HDC			hDC = ::GetDC( m_hWnd );
		LOGFONT		lf;
		CPoint	oldBase, newBase;
		CRect		rWnd;
		int 		iPixelSize = ::GetDeviceCaps( hDC, LOGPIXELSY );
		CFont		font;
	
		TRACE2("dc == %lXh, pixelSize == %d\n", hDC, iPixelSize);
		::ReleaseDC( m_hWnd, hDC );
											// Clear the font structure
		ChMemClearStruct( &lf );
	
		lf.lfHeight = iPixelSize * -10 / 72;
		lf.lfWeight = FW_NORMAL;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy( lf.lfFaceName, "Arial" );
	
		font.CreateFontIndirect( &lf );
		CWnd *hTab = GetTabControl();
		//hTab->SetFont(&font);
		GetDlgItem(IDOK)->SetFont(&font);
		font.Detach();
		
		// Scale up the dialog a bit - the system seems a little confused.
		oldBase = ChPoint(8, 8);		// arbitrary base figures
		newBase = ChPoint(9, 9);		// scale up a little
		//RemapWindowUnits(this, oldBase, newBase);
		//RemapWindowUnits(GetTabControl(), oldBase, newBase);
		//RemapWindowUnits(GetDlgItem(0), oldBase, newBase);
		//RemapWindowUnits(GetDlgItem(IDOK), oldBase, newBase);
		//CenterWindow();
	}
	
	//UETRACE("OnInitDialog: hide buttons");
#if 0	//defined(_DEBUG)
	//BOOL oldTrace = afxTraceEnabled;
	//afxTraceEnabled = TRUE;
	afxDump.Flush();
	afxDump << "Beginning dangercode.  Various combination attempts:"
					<< "\n  self.ID_APPLY_NOW == " << GetDlgItem(ID_APPLY_NOW)
					<< "\n  self.IDOK == " << GetDlgItem(IDOK);
	CWnd *pwnd = GetParent();
	if(pwnd) {
		afxDump << "\n  parent.ID_APPLY_NOW == " << pwnd->GetDlgItem(ID_APPLY_NOW)
						<< "\n  parent.IDOK == " << pwnd->GetDlgItem(IDOK);
		pwnd = pwnd->GetParent();
		if(pwnd) {
			afxDump << "\n  gparent.ID_APPLY_NOW == " << pwnd->GetDlgItem(ID_APPLY_NOW)
							<< "\n  gparent.IDOK == " << pwnd->GetDlgItem(IDOK);
		}
	}
	afxDump << "\n";
	afxDump.Flush();
	pwnd = GetTopWindow();
	afxDump << "Children of main property sheet:";
	while(pwnd) {
		CString title;
		pwnd->GetWindowText(title);
		afxDump << "\n  id " << pwnd->GetDlgCtrlID() << " title \"" << title << "\"";
		pwnd = pwnd->GetNextWindow();
	}
	afxDump << "\ndone.\n";			
	afxDump.Flush();
	pwnd = GetParent()->GetTopWindow();
	afxDump << "Children of property sheet parent:";
	while(pwnd) {
		CString title;
		pwnd->GetWindowText(title);
		afxDump << "\n  id " << pwnd->GetDlgCtrlID() << " title \"" << title << "\"";
		pwnd = pwnd->GetNextWindow();
	}
	afxDump << "\ndone.\n";			
	afxDump.Flush();
#endif
	// TODO: Add your specialized creation code here 
	// Remove the Apply button fro the sheet
	GetDlgItem( ID_APPLY_NOW  )->ShowWindow( SW_HIDE ); 
	GetDlgItem( IDCANCEL  )->ShowWindow( SW_HIDE ); 
	GetDlgItem( IDHELP  )->ShowWindow( SW_HIDE ); 
	
	// Reposition the OK, Cancel and Help buttons
	CRect rtFrame, rtBtn;
	GetWindowRect( &rtFrame );

	GetDlgItem( IDOK  )->GetWindowRect( &rtBtn );

	int xPos = (rtFrame.Width() - rtBtn.Width() )/2;

	CPoint ptPos( xPos, rtBtn.top );
	ScreenToClient( &ptPos );
	ptPos.x = xPos;

	//UETRACE("OnInitDialog: reposition");
	GetDlgItem( IDOK  )->SetWindowPos( 0, ptPos.x, ptPos.y, 0, 0, 
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE );

	//UETRACE("OnInitDialog: done");
#if 0		//defined(_DEBUG)
	afxTraceEnabled = oldTrace;
#endif
	return bResult;
}

void ChAbout::OnNcDestroy()
{
	ChPropertySheet::OnNcDestroy();

	m_pageMgr.ReleaseModulePages();
}

/*----------------------------------------------------------------------------
	ChPuebloAbout property page class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPuebloAbout, ChPropertyPage )

ChPuebloAbout::ChPuebloAbout() :
					ChPropertyPage( ChPuebloAbout::IDD, 0, hInstApp )
{
	//UETRACE("ChPuebloAbout constructor");
	//{{AFX_DATA_INIT(ChPuebloAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ChPuebloAbout::~ChPuebloAbout()
{
	//UETRACE("ChPuebloAbout destructor");
}


void ChPuebloAbout::DoDataExchange( CDataExchange* pDX )
{
	//UETRACE("ChPuebloAbout::DoDataExchange");
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChPuebloAbout)
	DDX_Control(pDX, IDC_STATIC_VERSION, m_staticVersion);
	DDX_Control(pDX, IDC_PROD_NAME, m_staticProductName);
	DDX_Control(pDX, IDC_LEGEND, m_staticLegend);
	DDX_Control(pDX, IDC_COPYRIGHT1, m_staticCopyright1);
	DDX_Control(pDX, IDC_COPYRIGHT2, m_staticCopyright2);
	DDX_Control(pDX, IDC_COPYRIGHT3, m_staticCopyright3);
	DDX_Control(pDX, IDC_CLAUSE, m_staticClause);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChPuebloAbout, ChPropertyPage )
	//{{AFX_MSG_MAP(ChPuebloAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPuebloAbout message handlers
----------------------------------------------------------------------------*/

BOOL ChPuebloAbout::OnInitDialog()
{
	//UETRACE("ChPuebloAbout::OnInitDialog");
	ChClientInfo	clientInfo( ChClientInfo::thisMachine );
	ChVersion		clientVer = clientInfo.GetClientVersion();
	ChString			strVerNum( clientVer.Format() );

	ChPropertyPage::OnInitDialog();
											// initialize the Chaco logo

	m_logoBmp.SubclassDlgItem( IDC_CHACO_LOGO, this );
	m_logoBmp.SizeToContent( ChLogoBitmap::left | ChLogoBitmap::vcenter,
														IDR_CHACO_DIB );

	{										/* Set fonts for static fields in
												the page to make it more
												attractive */
		HDC			hDC = ::GetDC( m_hWnd );
		LOGFONT		lf;
		int 		iPixelSize = ::GetDeviceCaps( hDC, LOGPIXELSY );
		CFont		font;

		::ReleaseDC( m_hWnd, hDC );
											// Clear the font structure
		ChMemClearStruct( &lf );

		lf.lfHeight = -1 * (iPixelSize * 14 / 72);
		lf.lfWeight = FW_BOLD;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy( lf.lfFaceName, "Arial" );

		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticProductName, &font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 8 / 72);
		lf.lfWeight = FW_NORMAL;
		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticVersion, &font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 9 / 72);
		lf.lfWeight = FW_BOLD;
		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticCopyright1, &font);
		SetChildCustomFont(m_staticCopyright2, &font);
		SetChildCustomFont(m_staticCopyright3, &font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 8 / 72);
		lf.lfWeight = FW_BOLD;
		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticLegend, &font);
		SetChildCustomFont(m_staticClause, &font);
		font.Detach();

/*
		lf.lfHeight = -1 * (iPixelSize * 6 / 72);
		lf.lfWeight = FW_MEDIUM;
		font.CreateFontIndirect( &lf );
		m_staticClause.SendMessage( WM_SETFONT,
										(WPARAM)font.GetSafeHandle() );
		font.Detach();
*/
	}

	m_staticVersion.SetWindowText( strVerNum );

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL ChPuebloAbout::OnSetActive()
{
	//UETRACE("ChPuebloAbout::OnSetActive");
	return ChPropertyPage::OnSetActive();
}



/*----------------------------------------------------------------------------
	ChDisclaimerAbout property page class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChDisclaimerAbout, ChPropertyPage )

ChDisclaimerAbout::ChDisclaimerAbout() :
					ChPropertyPage( ChDisclaimerAbout::IDD, 0, hInstApp )
{
	//{{AFX_DATA_INIT(ChDisclaimerAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ChDisclaimerAbout::~ChDisclaimerAbout()
{
}

void ChDisclaimerAbout::CreateTextWindow()
{
	CRect		rtView;
	ChWnd*		pPlaceholder = GetDlgItem( IDC_STATIC_PLACEHOLDER );

	pPlaceholder->GetWindowRect( &rtView );
	ScreenToClient( &rtView );
	pPlaceholder->ShowWindow(SW_HIDE);

	m_htmlWnd.CreateEx( rtView, this, WS_CHILD | WS_VISIBLE | WS_BORDER,
						WS_EX_CLIENTEDGE );
	m_htmlWnd.EnableSelection( false );		// No se lection allowed

	m_htmlWnd.DisplayResource( IDR_DISCLAIMER );
}

void ChDisclaimerAbout::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChDisclaimerAbout)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( ChDisclaimerAbout, ChPropertyPage )
	//{{AFX_MSG_MAP(ChDisclaimerAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChDisclaimerAbout message handlers
----------------------------------------------------------------------------*/

BOOL ChDisclaimerAbout::OnInitDialog()
{
	ChPropertyPage::OnInitDialog();

	CreateTextWindow();

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL ChDisclaimerAbout::OnSetActive()
{
	return ChPropertyPage::OnSetActive();
}


/*----------------------------------------------------------------------------
	ChTeamAbout property sheet class
----------------------------------------------------------------------------*/

chint32	ChTeamAbout::iTeamNames[namesCount] = {	    IDS_TEAM_GLENN,
												IDS_TEAM_PRITHAM,
												IDS_TEAM_RON,
												IDS_TEAM_JIM,
												IDS_TEAM_DAN };

IMPLEMENT_DYNCREATE( ChTeamAbout, ChPropertyPage )

ChTeamAbout::ChTeamAbout() : ChPropertyPage( ChTeamAbout::IDD, 0, hInstApp )
{
	//{{AFX_DATA_INIT(ChTeamAbout)
	//}}AFX_DATA_INIT
}

ChTeamAbout::~ChTeamAbout()
{
}

void ChTeamAbout::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChTeamAbout)
	DDX_Control(pDX, IDC_NAME_5, m_staticName5);
	DDX_Control(pDX, IDC_NAME_4, m_staticName4);
	DDX_Control(pDX, IDC_NAME_3, m_staticName3);
	DDX_Control(pDX, IDC_NAME_2, m_staticName2);
	DDX_Control(pDX, IDC_NAME_1, m_staticName1);
	DDX_Control(pDX, IDC_TEAM_TITLE, m_staticTeamTitle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( ChTeamAbout, ChPropertyPage )
	//{{AFX_MSG_MAP(ChTeamAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTeamAbout protected methods
----------------------------------------------------------------------------*/

void ChTeamAbout::RandomizeNames()
{
	int		iLoop;
											/* Initialize the random number
												generator with the current
												time */
	srand( (unsigned int)time( 0 ) );

	for (iLoop = 0; iLoop < 30; iLoop++)
	{
		int		iFirst = GetRandomInt( namesCount - 1 );
		int		iSecond = GetRandomInt( namesCount - 1 );

		if (iFirst != iSecond)
		{
			chint32		iTemp;
											// Swap the two
			iTemp = iTeamNames[iFirst];
			iTeamNames[iFirst] = iTeamNames[iSecond];
			iTeamNames[iSecond] = iTemp;
		}
	}
}


int ChTeamAbout::GetRandomInt( int iMax )
{											/* Generates a random int between
												zero and iMax, inclusive */
	int		iResult;
	double	dRan = (double)rand();
	double	fVal = dRan / (double)RAND_MAX;

	iResult = (int)((fVal * (double)iMax) + 0.5);
	ASSERT( iResult >= 0 && iResult <= iMax );

	return iResult;
}


void ChTeamAbout::SetNames()
{
	static int	iFields[namesCount] = { IDC_NAME_1, IDC_NAME_2, IDC_NAME_3,
										IDC_NAME_4, IDC_NAME_5 };
	int			iLoop;

	for (iLoop = 0; iLoop < namesCount; iLoop++)
	{
		CStatic*	pStatic;

		if (pStatic = (CStatic*)GetDlgItem( iFields[iLoop] ))
		{
			ChString	strName;

			LOADSTRING( (int)iTeamNames[iLoop], strName );
			pStatic->SetWindowText( strName );
		}
	}
}


/*----------------------------------------------------------------------------
	ChTeamAbout message handlers
----------------------------------------------------------------------------*/

BOOL ChTeamAbout::OnInitDialog()
{
	ChPropertyPage::OnInitDialog();
											// initialize the Chaco logo

	m_logoBmp.SubclassDlgItem( IDC_CHACO_LOGO, this );
	m_logoBmp.SizeToContent( ChLogoBitmap::left | ChLogoBitmap::vcenter,
														IDR_CHACO_DIB );

	RandomizeNames();
	SetNames();

	{										/* Set fonts for static fields in
												the page to make it more
												attractive */
		HDC			hDC = ::GetDC( m_hWnd );
		LOGFONT		lf;
		int 		iPixelSize = ::GetDeviceCaps( hDC, LOGPIXELSY );
		CFont		font;

		::ReleaseDC( m_hWnd, hDC );
											// Clear the font structure
		ChMemClearStruct( &lf );

		lf.lfHeight = -1 * (iPixelSize * 12/ 72);
		lf.lfWeight = FW_BOLD;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy( lf.lfFaceName, "Arial" );

		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticTeamTitle, &font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 10 / 72);
		lf.lfItalic = TRUE;
		lf.lfWeight = FW_NORMAL;
		font.CreateFontIndirect( &lf ); 
		SetChildCustomFont(m_staticName1, &font);
		SetChildCustomFont(m_staticName2, &font);
		SetChildCustomFont(m_staticName3, &font);
		SetChildCustomFont(m_staticName4, &font);
		SetChildCustomFont(m_staticName5, &font);
		font.Detach();
	}

	return true;							/* Return TRUE unless you set the
												focus to a control */
}

BOOL ChTeamAbout::OnSetActive()
{
	return ChPropertyPage::OnSetActive();
}



/*----------------------------------------------------------------------------
	ChUEAbout property page class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChUEAbout, ChPropertyPage )

ChUEAbout::ChUEAbout() :
					ChPropertyPage( ChUEAbout::IDD, 0, hInstApp )
{
	//{{AFX_DATA_INIT(ChUEAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ChUEAbout::~ChUEAbout()
{
}


void ChUEAbout::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChUEAbout)
	DDX_Control(pDX, IDC_PROD_NAME, m_staticCompanyName);
	DDX_Control(pDX, IDC_UE_WEBSITE, m_staticWebsite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChUEAbout, ChPropertyPage )
	//{{AFX_MSG_MAP(ChUEAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChUEAbout message handlers
----------------------------------------------------------------------------*/

BOOL ChUEAbout::OnInitDialog()
{
	ChPropertyPage::OnInitDialog();
											// initialize the UE logo

	m_logoBmp.SubclassDlgItem( IDC_UE_LOGO, this );
	m_logoBmp.SizeToContent( ChLogoBitmap::left | ChLogoBitmap::top,
														IDR_UE_DIB );

	{										/* Set fonts for static fields in
												the page to make it more
												attractive */
		HDC			hDC = ::GetDC( m_hWnd );
		LOGFONT		lf;
		int 		iPixelSize = ::GetDeviceCaps( hDC, LOGPIXELSY );
		CFont		font;

		::ReleaseDC( m_hWnd, hDC );
											// Clear the font structure
		ChMemClearStruct( &lf );

		lf.lfHeight = -1 * (iPixelSize * 14 / 72);
		lf.lfWeight = FW_BOLD;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE;
		lstrcpy( lf.lfFaceName, "Arial" );

		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticCompanyName, &font);
		font.Detach();

		lf.lfHeight = -1 * (iPixelSize * 9 / 72);
		lf.lfWeight = FW_NORMAL;
		lstrcpy( lf.lfFaceName, "Courier New" );
		font.CreateFontIndirect( &lf );
		SetChildCustomFont(m_staticWebsite, &font);
		font.Detach();
	}

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL ChUEAbout::OnSetActive()
{
	return ChPropertyPage::OnSetActive();
}

/*----------------------------------------------------------------------------
	ChComponentsAbout property page class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChComponentsAbout, ChPropertyPage )

ChComponentsAbout::ChComponentsAbout() :
					ChPropertyPage( ChComponentsAbout::IDD, 0, hInstApp )
{
	//{{AFX_DATA_INIT(ChComponentsAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ChComponentsAbout::~ChComponentsAbout()
{
}

void ChComponentsAbout::CreateTextWindow()
{
	CRect		rtView;
	ChWnd*		pPlaceholder = GetDlgItem( IDC_STATIC_PLACEHOLDER );

	pPlaceholder->GetWindowRect( &rtView );
	ScreenToClient( &rtView );
	pPlaceholder->ShowWindow(SW_HIDE);

	m_htmlWnd.CreateEx( rtView, this, WS_CHILD | WS_VISIBLE | WS_BORDER,
						WS_EX_CLIENTEDGE );
	m_htmlWnd.EnableSelection( false );		// No se lection allowed

	//m_htmlWnd.DisplayResource( IDR_DISCLAIMER );
	m_htmlWnd.NewPage();
	
	DisplayStringResource(IDS_COMPONENTS_HEADER);
	m_htmlWnd.AppendText("  <ul>\r\n");

	DisplayMFCVersion();
	DisplayMNGComponents();
	m_htmlWnd.AppendText("    <li>mcclient version 0.4</li>\r\n");

	m_htmlWnd.AppendText("  </ul>\r\n");
	DisplayStringResource(IDS_COMPONENTS_FOOTER);
}

void ChComponentsAbout::DisplayStringResource(int id)
{
	ChString text;
	LOADSTRING(id, text);

	m_htmlWnd.AppendText(text);
}

void ChComponentsAbout::DisplayMFCVersion()
{
	ChString text, version;
	text = "    <li>Microsoft Foundation Classes ";
#if defined(__BORLANDC__)
	text += "(for Borland C++) ";
#endif
	version.Format("version %d.%d</li>\r\n", HIBYTE(_MFC_VER), LOBYTE(_MFC_VER));
	text += version;
	m_htmlWnd.AppendText(text);
}

void ChComponentsAbout::DisplayMNGComponents()
{
	ChString text, temp;

	text.Format("    <li>libmng version %s\r\n", MNG_VERSION_TEXT);
	text += "      <ul>\r\n";
	temp.Format("      <li>zlib version %s</li>\r\n", ZLIB_VERSION);
	text += temp;
	temp.Format("      <li>jpeglib version %d.%d</li>\r\n", JPEG_LIB_VERSION / 10, JPEG_LIB_VERSION % 10);
	text += temp;
	text += "      <li>lcms version 1.09d</li>\r\n";
	text += "    </ul></li>\r\n";
	
	m_htmlWnd.AppendText(text);
}

void ChComponentsAbout::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChComponentsAbout)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( ChComponentsAbout, ChPropertyPage )
	//{{AFX_MSG_MAP(ChComponentsAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChComponentsAbout message handlers
----------------------------------------------------------------------------*/

BOOL ChComponentsAbout::OnInitDialog()
{
	ChPropertyPage::OnInitDialog();

	CreateTextWindow();

	return TRUE;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:20  uecasm
// Import of source tree as at version 2.53 release.
//
