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

	This file contains the implementation of the ChPrefsFontPage class,
	which allows the user to select font preferences.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChReg.h>
#include <ChHtmWnd.h>
#include <ChUtil.h>

#include "ChClCore.h"
#include "ChPrFont.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChPrefsFontPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsFontPage, ChPropertyPage )

ChPrefsFontPage::ChPrefsFontPage() :
					ChPropertyPage( ChPrefsFontPage::IDD, 0, hInstApp ),
					m_regFont( CH_FONT_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsFontPage)
	m_iBackColor = -1;
	//}}AFX_DATA_INIT
}

ChPrefsFontPage::~ChPrefsFontPage()
{
}

void ChPrefsFontPage::DoDataExchange(CDataExchange* pDX)
{
	ChPropertyPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChPrefsFontPage)
	DDX_Control(pDX, IDC_STAT_PROPORTIONAL, m_staticProportionalName);
	DDX_Control(pDX, IDC_STAT_FIXED, m_staticFixedName);
	//}}AFX_DATA_MAP
}


BOOL ChPrefsFontPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		m_regFont.Read( CH_FONT_PROPORTIONAL, m_strProportionalFont,
							CH_FONT_PROPORTIONAL_DEF );
 		m_regFont.Read( CH_FONT_PROPORTIONAL_SIZE, m_lProportionalPointSize,
 						CH_FONT_PROPORTIONAL_SIZE_DEF );

		m_regFont.Read( CH_FONT_FIXED, m_strFixedFont, CH_FONT_FIXED_DEF );
		m_regFont.Read( CH_FONT_FIXED_SIZE, m_lFixedPointSize,
						CH_FONT_FIXED_SIZE_DEF );

											// Set the font names

		m_staticProportionalName.SetWindowText( m_strProportionalFont );
		m_staticFixedName.SetWindowText( m_strFixedFont );

		UpdateData( false );
											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsFontPage::OnCommit()
{
	if (m_boolInitialized)
	{
		UpdateData();

		m_regFont.Write( CH_FONT_PROPORTIONAL, m_strProportionalFont );
 		m_regFont.Write( CH_FONT_PROPORTIONAL_SIZE, m_lProportionalPointSize );

		m_regFont.Write( CH_FONT_FIXED, m_strFixedFont );
		m_regFont.Write( CH_FONT_FIXED_SIZE, m_lFixedPointSize );

											/* Send message to all HTML windows
												to update font */
		CWnd* pWnd = AfxGetApp()->GetMainWnd();
		if ( pWnd )
		{
			pWnd->SendMessageToDescendants( WM_HTML_FONT_CHANGE );
		}
	}
}


BEGIN_MESSAGE_MAP(ChPrefsFontPage, ChPropertyPage)
	//{{AFX_MSG_MAP(ChPrefsFontPage)
	ON_BN_CLICKED(IDC_CHOOSE_FIXED, OnChooseFixedFont)
	ON_BN_CLICKED(IDC_CHOOSE_PROPORTIONAL, OnChooseProportionalFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsFontPage message handlers
----------------------------------------------------------------------------*/
	
void ChPrefsFontPage::OnChooseFixedFont()
{
	#if defined( CH_MSW )
	{
		LOGFONT		lf;
		HDC			hDC = ::GetDC( ::GetDesktopWindow() );
		int			iPixelY = -1 * ::GetDeviceCaps( hDC, LOGPIXELSY );
		const DWORD	dwFlags = (CF_FIXEDPITCHONLY | CF_SCREENFONTS |
								  CF_LIMITSIZE | CF_INITTOLOGFONTSTRUCT);

		::ReleaseDC(::GetDesktopWindow(), hDC );

		ChMemClearStruct( &lf );

		lf.lfHeight			= (iPixelY * (int)m_lFixedPointSize) / 72;
		lf.lfWeight 		= FW_LIGHT;
		lf.lfCharSet 		= ANSI_CHARSET;
		lf.lfOutPrecision 	= OUT_STROKE_PRECIS;
		lf.lfClipPrecision 	= CLIP_STROKE_PRECIS;
		lf.lfQuality 		= DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		lstrcpy( lf.lfFaceName, m_strFixedFont );	/* Should we get the name
														from the preference
														file ??? */

		CFontDialog		fixedFont( &lf, dwFlags, 0, this );

		fixedFont.m_cf.Flags &= ~CF_SHOWHELP;
		fixedFont.m_cf.nSizeMin = 6;
		fixedFont.m_cf.nSizeMax = 16;
		fixedFont.m_cf.iPointSize = (int)m_lFixedPointSize;
		if (fixedFont.DoModal() == IDOK)
		{
			CStatic		*pName;
			m_strFixedFont = lf.lfFaceName;
			m_lFixedPointSize = fixedFont.m_cf.iPointSize / 10;

													// Set the fixed font name

			pName = (CStatic *)GetDlgItem( IDC_STAT_FIXED );
			pName->SetWindowText( m_strFixedFont );
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )
}

void ChPrefsFontPage::OnChooseProportionalFont()
{
	#if defined( CH_MSW )
	{
		LOGFONT		lf;
		HDC			hDC = ::GetDC( ::GetDesktopWindow() );
		int			iPixelY = -1 * ::GetDeviceCaps( hDC, LOGPIXELSY );
		const DWORD	dwFlags = (CF_SCALABLEONLY | CF_SCREENFONTS |
								  CF_LIMITSIZE | CF_INITTOLOGFONTSTRUCT);

		::ReleaseDC( ::GetDesktopWindow(), hDC );

		ChMemClearStruct( &lf );

		lf.lfHeight			= (iPixelY * (int)m_lProportionalPointSize) / 72;
		lf.lfWeight 		= FW_LIGHT;
		lf.lfCharSet 		= ANSI_CHARSET;
		lf.lfOutPrecision 	= OUT_STROKE_PRECIS;
		lf.lfClipPrecision 	= CLIP_STROKE_PRECIS;
		lf.lfQuality 		= DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		lstrcpy( lf.lfFaceName, m_strProportionalFont );/* Should we get the name
														from the preference
														file ??? */

		CFontDialog		propFont( &lf, dwFlags, 0, this );

		propFont.m_cf.Flags &= ~CF_SHOWHELP;
		propFont.m_cf.nSizeMin = 6;
		propFont.m_cf.nSizeMax = 16;
		propFont.m_cf.iPointSize = (int)m_lProportionalPointSize;

		if (propFont.DoModal() == IDOK)
		{
			m_strProportionalFont = lf.lfFaceName;
			m_lProportionalPointSize = propFont.m_cf.iPointSize / 10;

													// Set the proportional font name
			CStatic		*pName;
			pName = (CStatic *)GetDlgItem( IDC_STAT_PROPORTIONAL );
			pName->SetWindowText( m_strProportionalFont );
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )
}



/*----------------------------------------------------------------------------
	ChPrefsColorPage class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPrefsColorPage, ChPropertyPage )

ChPrefsColorPage::ChPrefsColorPage() :
					ChPropertyPage( ChPrefsColorPage::IDD, 0, hInstApp ),
					m_regColor( CH_COLOR_GROUP ),
					m_boolInitialized( false )
{
	//{{AFX_DATA_INIT(ChPrefsColorPage)
	m_iBackColor = -1;
	m_iTextColor = -1;
	m_iLinkColor = -1;
	m_iFLinkColor = -1;
	m_iPLinkColor = -1;
	//}}AFX_DATA_INIT
}

ChPrefsColorPage::~ChPrefsColorPage()
{
}

void ChPrefsColorPage::DoDataExchange(CDataExchange* pDX)
{
	ChPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChPrefsColorPage)
	DDX_Control(pDX, IDC_SAMPLE_BACK, m_btnSampleBack);
	DDX_Control(pDX, IDC_SAMPLE_TEXT, m_btnSampleText);
	DDX_Control(pDX, IDC_SAMPLE_LINK, m_btnSampleLink);
	DDX_Control(pDX, IDC_SAMPLE_FLINK, m_btnSampleFLink);
	DDX_Control(pDX, IDC_SAMPLE_PLINK, m_btnSamplePLink);
	DDX_Control(pDX, IDC_COMBO_BACK_COLOR, m_comboBackColor);
	DDX_Control(pDX, IDC_COMBO_TEXT_COLOR, m_comboTextColor);
	DDX_Control(pDX, IDC_COMBO_LINK_COLOR, m_comboLinkColor);
	DDX_Control(pDX, IDC_COMBO_FLINK_COLOR, m_comboFLinkColor);
	DDX_Control(pDX, IDC_COMBO_PLINK_COLOR, m_comboPLinkColor);
	DDX_CBIndex(pDX, IDC_COMBO_BACK_COLOR, m_iBackColor);
	DDX_CBIndex(pDX, IDC_COMBO_TEXT_COLOR, m_iTextColor);
	DDX_CBIndex(pDX, IDC_COMBO_LINK_COLOR, m_iLinkColor);
	DDX_CBIndex(pDX, IDC_COMBO_FLINK_COLOR, m_iFLinkColor);
	DDX_CBIndex(pDX, IDC_COMBO_PLINK_COLOR, m_iPLinkColor);
	//}}AFX_DATA_MAP
}


BOOL ChPrefsColorPage::OnSetActive()
{
	BOOL	boolResult;

	boolResult = ChPropertyPage::OnSetActive();

	if (!m_boolInitialized)
	{
		chuint32		luColor;
		DWORD			dwStyle;
											// Initialize the color lists

		InitColorList( m_comboBackColor, IDS_TEXT_COLOR );
		InitColorList( m_comboTextColor, IDS_TEXT_COLOR );
		InitColorList( m_comboLinkColor, IDS_TEXT_COLOR );
		InitColorList( m_comboFLinkColor, IDS_TEXT_COLOR );
		InitColorList( m_comboPLinkColor, IDS_TEXT_COLOR );

											// Read the registry colors

		m_regColor.Read( CH_COLOR_BACK, luColor, CH_COLOR_DEFAULT );
		m_iBackColor = GetColorIndex( luColor );
		m_regColor.Read( CH_COLOR_TEXT, luColor, CH_COLOR_DEFAULT );
		m_iTextColor = GetColorIndex( luColor );
		m_regColor.Read( CH_COLOR_LINK, luColor, CH_COLOR_DEFAULT );
		m_iLinkColor = GetColorIndex( luColor );
		m_regColor.Read( CH_COLOR_FLINK, luColor, CH_COLOR_DEFAULT );
		m_iFLinkColor = GetColorIndex( luColor );
		m_regColor.Read( CH_COLOR_PLINK, luColor, CH_COLOR_DEFAULT );
		m_iPLinkColor = GetColorIndex( luColor );

		UpdateData( false );
											/* Set the background button to
												clip siblings */
		dwStyle = m_btnSampleBack.GetStyle();
		dwStyle |= WS_CLIPSIBLINGS;
		SetWindowLong( m_btnSampleBack.m_hWnd, GWL_STYLE, dwStyle );

											/* Set the initialized flag so
												that we don't do this again */
		m_boolInitialized = true;
	}

	return boolResult;
}


void ChPrefsColorPage::OnCommit()
{
	if (m_boolInitialized)
	{
		chint32		luColor;

		UpdateData();

		luColor = GetColorValue( m_iBackColor );
		m_regColor.Write( CH_COLOR_BACK, luColor );
		luColor = GetColorValue(  m_iTextColor );
		m_regColor.Write( CH_COLOR_TEXT, luColor );
		luColor = GetColorValue( m_iLinkColor );
		m_regColor.Write( CH_COLOR_LINK, luColor );
		luColor = GetColorValue( m_iFLinkColor );
		m_regColor.Write( CH_COLOR_FLINK, luColor );
		luColor = GetColorValue( m_iPLinkColor );
		m_regColor.Write( CH_COLOR_PLINK, luColor );

											/* Send message to all HTML windows
												to update font */
		CWnd* pWnd = AfxGetApp()->GetMainWnd();
		if ( pWnd )
		{
			pWnd->SendMessageToDescendants( WM_HTML_COLOR_CHANGE );
		}
	}
}


BEGIN_MESSAGE_MAP(ChPrefsColorPage, ChPropertyPage)
	//{{AFX_MSG_MAP(ChPrefsColorPage)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(IDC_COMBO_BACK_COLOR, OnSelchangeComboBackColor)
	ON_CBN_SELCHANGE(IDC_COMBO_FLINK_COLOR, OnSelchangeComboFlinkColor)
	ON_CBN_SELCHANGE(IDC_COMBO_LINK_COLOR, OnSelchangeComboLinkColor)
	ON_CBN_SELCHANGE(IDC_COMBO_PLINK_COLOR, OnSelchangeComboPlinkColor)
	ON_CBN_SELCHANGE(IDC_COMBO_TEXT_COLOR, OnSelchangeComboTextColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPrefsColorPage protected members
----------------------------------------------------------------------------*/
	
const chuint32	ChPrefsColorPage::m_luBackColors[] = {
									RGB( 255, 255, 255 ),	// white
									RGB( 192, 192, 192 ),	// lt grey
									RGB( 128, 128, 128 ),	// dk grey
									RGB( 0, 0, 0 ),			// black
									RGB( 255, 0, 0 ),		// red
									RGB( 255, 0, 255 ),		// magenta
									RGB( 0, 0, 255 ),		// blue
									RGB( 0, 255, 255 ),		// cyan
									RGB( 0, 255, 0 ),		// green
									RGB( 255, 255, 0 ),		// yellow
									RGB( 128, 0, 0 ),		// dk red
									RGB( 128, 0, 128 ),		// dk magenta
									RGB( 0, 0, 128 ),		// dk blue
									RGB( 0, 128, 128 ),		// dk cyan
									RGB( 0, 128, 0 ),		// dk green
									RGB( 128, 128, 0 ),		// dk yellow
									};


void ChPrefsColorPage::InitColorList( CComboBox& comboColor,
										chuint16 suExtraColorText )
{
	int					iLoop;
	const int			iCount = sizeof( m_luBackColors ) /
									sizeof( m_luBackColors[0] );

	if (suExtraColorText)
	{										// Add the extra color
		chuint32		luData;

		luData = CH_COLOR_DEFAULT | suExtraColorText;
		comboColor.AddString( (LPCSTR)luData );
	}

	for (iLoop = 0; iLoop < iCount; iLoop++)
	{
		comboColor.AddString( (LPCSTR)m_luBackColors[iLoop] );
	}
}


chint16 ChPrefsColorPage::GetColorIndex( chuint32 luColor,
											bool boolDefaultUsed )
{
											/* This function will return the
												color index corresponding to
												a given color.  If no colors
												match, the default color will
												be returned */

	int					iFound = -1;
	int					iLoop;
	const int			iCount = sizeof( m_luBackColors ) /
									sizeof( m_luBackColors[0] );

	for (iLoop = 0; (-1 == iFound) && (iLoop < iCount); iLoop++)
	{
		if (m_luBackColors[iLoop] == luColor)
		{
			iFound = iLoop;

			if (boolDefaultUsed)
			{
				iFound++;
			}
		}
	}

	if (-1 == iFound && boolDefaultUsed )
	{
		iFound = 0;
	}

	return iFound;
}


chuint32 ChPrefsColorPage::GetColorValue( chint16 sIndex,
											bool boolDefaultUsed )
{
	if (boolDefaultUsed)
	{
		sIndex--;
	}

	if (-1 == sIndex)
	{
		return CH_COLOR_DEFAULT;
	}
	else
	{
		ASSERT( sIndex >= 0 );
		ASSERT( sIndex < (sizeof( m_luBackColors ) /
							sizeof( m_luBackColors[0] )) );

		return m_luBackColors[sIndex];
	}
}


void ChPrefsColorPage::DrawColorItem( LPDRAWITEMSTRUCT lpDrawItemStruct,
										CDC* pDC, const CRect& rtItem )
{
	int			iIndex = lpDrawItemStruct->itemID - 1;
	chuint32	luItemData = lpDrawItemStruct->itemData;
	COLORREF	itemColor = (COLORREF)luItemData;
	DWORD		itemState = lpDrawItemStruct->itemState;

											// Fill the item rect with the color

	if ((-1 == iIndex) || IsDefColor( luItemData ))
	{
											/* Empty item or default window
												color */

		COLORREF	windowColor = GetSysColor( COLOR_WINDOW );
		CBrush		fillBrush;
		COLORREF	textColor;
		ChString		strText;
		CFont		textFont;
		CFont*		pOldFont;
		CSize		sizeText;
		int			iLeft;
		int			iTop;

		// UE: make the appearance of the defaults match their actual colour.
		switch( lpDrawItemStruct->CtlID ) {
			case IDC_COMBO_BACK_COLOR:
				windowColor = COLOR_DEF_BACK;
				break;
			case IDC_COMBO_TEXT_COLOR:
				windowColor = COLOR_DEF_TEXT;
				break;
			case IDC_COMBO_LINK_COLOR:
				windowColor = COLOR_DEF_LINK;
				break;
			case IDC_COMBO_FLINK_COLOR :
				windowColor = COLOR_DEF_VIST_LINK;
				break;
			case IDC_COMBO_PLINK_COLOR :
				windowColor = COLOR_DEF_PREFETCH_LINK;
				break;
		}
		fillBrush.CreateSolidBrush(windowColor);
		pDC->FillRect( &rtItem, &fillBrush );
											// Default color

		if (LOADSTRING( IDS_TEXT_COLOR, strText ))
		{
			// UE: select either black or white as text colour, depending on the
			//     relative intensity of the background colour.
			int bright = ((222*GetRValue(windowColor)) + (707*GetGValue(windowColor)) +
										( 71*GetBValue(windowColor))) / 1000;
			// This, incidentally, is the grayscale summing formula according to the ITU
			// standard, at least according to my notes :)
			textColor = (bright >= 128) ? 0x000000 : 0xFFFFFF;
			//textColor = GetSysColor( COLOR_BTNTEXT );

			pOldFont = pDC->SelectObject( m_comboBackColor.GetFont() );	  

/*
			switch( lpDrawItemStruct->CtlID )
			{
				case IDC_COMBO_LINK_COLOR:
				{
					textColor = COLOR_DEF_LINK;
					break;
				}

				case IDC_COMBO_FLINK_COLOR :
				{
					textColor = COLOR_DEF_VIST_LINK;
					break;
				}

				case IDC_COMBO_PLINK_COLOR :
				{
					textColor = COLOR_DEF_PREFETCH_LINK;
					break;
				}
			}
*/

			pDC->SetTextColor( textColor );
											// Center the text if possible

			sizeText = pDC->GetTextExtent( strText, strText.GetLength() );
			iTop = rtItem.top;
			if (rtItem.Height() > sizeText.cy)
			{
				iTop += (rtItem.Height() - sizeText.cy) / 2;
			}

			iLeft = rtItem.left;
			if (rtItem.Width() > sizeText.cx)
			{
				iLeft += (rtItem.Width() - sizeText.cx) / 2;
			}
			pDC->SetBkColor( windowColor );
											// Draw the text

			pDC->ExtTextOut( iLeft, iTop, ETO_OPAQUE, &rtItem, strText,
								strText.GetLength(), 0 );

			pDC->SelectObject( pOldFont );
		}
	}
	else
	{										// Specific color
		CBrush		fillBrush( itemColor );

		pDC->FillRect( &rtItem, &fillBrush );
	}

	if (itemState & ODS_FOCUS)
	{										/* Set the initial state of the
												focus */
		CRect	rtFocus( rtItem );

		rtFocus.InflateRect( -1, -1 );
		pDC->DrawFocusRect( &rtFocus );
	}
	else if (itemState & ODS_SELECTED)
	{										/* Set the initial state of the
												selection */

		DrawColorSelect( lpDrawItemStruct, pDC, rtItem, true );
	}
}


void ChPrefsColorPage::DrawColorSelect( LPDRAWITEMSTRUCT lpDrawItemStruct,
										CDC* pDC, const CRect& rtItem,
										bool boolSelected )
{
	if (boolSelected)
	{
		CPen	whitePen( PS_SOLID, 0, RGB( 255, 255, 255 ) );
		CPen	blackPen( PS_SOLID, 0, RGB( 0, 0, 0 ) );
		CBrush*	pNullBrush;
		CPen*	pOldPen;
		CBrush*	pOldBrush;
		CRect	rtOutline( rtItem );

		pNullBrush = CBrush::FromHandle( (HBRUSH)GetStockObject( NULL_BRUSH ) );
		pOldBrush = pDC->SelectObject( pNullBrush );
		pOldPen = pDC->SelectObject( &whitePen );

		pDC->Rectangle( rtOutline );
		rtOutline.InflateRect( -1, -1 );
		pOldPen = pDC->SelectObject( &blackPen );
		pDC->Rectangle( rtOutline );
		rtOutline.InflateRect( -1, -1 );
		pOldPen = pDC->SelectObject( &whitePen );
		pDC->Rectangle( rtOutline );

		pDC->SelectObject( pOldPen );
		pDC->SelectObject( pOldBrush );
	}
	else
	{
		DrawColorItem( lpDrawItemStruct, pDC, rtItem );
	}
}


void ChPrefsColorPage::DrawColorSample( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC*		pDC;
	CRect		rtItem;
	COLORREF	colorBack;
	COLORREF	colorFore;
	chuint32	luBackColor;
	chuint32	luForeColor;
	COLORREF	defForeColor;
	ChString		strWindowText;
	CRect		rtWindow;
	int			iTop = 0;
	int			iLeft = 0;

	ASSERT( ODT_BUTTON == lpDrawItemStruct->CtlType );

	UpdateData();
											// Calculate the background color
	luBackColor = GetColorValue( m_iBackColor );
	if (IsDefColor( luBackColor ))
	{
		colorBack = COLOR_DEF_BACK;		//GetSysColor( COLOR_BTNFACE );
	}
	else
	{
		colorBack = (COLORREF)luBackColor;
	}
											/* Grab a copy of the things that
												are needed over and over
												again */

	pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	rtItem = lpDrawItemStruct->rcItem;

	switch( lpDrawItemStruct->CtlID )
	{
		case IDC_SAMPLE_BACK:
		{
			strWindowText = "";
			break;
		}

		case IDC_SAMPLE_TEXT:
		{
			luForeColor = GetColorValue( m_iTextColor );
			defForeColor = COLOR_DEF_TEXT;	//GetSysColor( COLOR_BTNTEXT );
			m_btnSampleText.GetWindowText( strWindowText );
			break;
		}

		case IDC_SAMPLE_LINK:
		{
			luForeColor = GetColorValue( m_iLinkColor );
			defForeColor = COLOR_DEF_LINK;
			m_btnSampleLink.GetWindowText( strWindowText );
			break;
		}

		case IDC_SAMPLE_FLINK:
		{
			luForeColor = GetColorValue( m_iFLinkColor );
			defForeColor = COLOR_DEF_VIST_LINK;
			m_btnSampleFLink.GetWindowText( strWindowText );
			break;
		}

		case IDC_SAMPLE_PLINK:
		{
			luForeColor = GetColorValue( m_iPLinkColor );
			defForeColor = COLOR_DEF_PREFETCH_LINK;
			m_btnSamplePLink.GetWindowText( strWindowText );
			break;
		}
	}

	if (!strWindowText.IsEmpty())
	{
		CSize	extents;

		if (IsDefColor( luForeColor ))
		{
			colorFore = defForeColor;
		}
		else
		{
			colorFore = (COLORREF)luForeColor;
		}
									// Measure the text

		extents = pDC->GetTextExtent( strWindowText,
										strWindowText.GetLength() );

		if (extents.cy < rtItem.Height())
		{
			iTop = rtItem.top + ((rtItem.Height() - extents.cy) / 2);
		}

		if (extents.cx < rtItem.Width())
		{
			iLeft = rtItem.left + ((rtItem.Width() - extents.cx) / 2);
		}
	}
									// Fill the rect with the color
	pDC->SetBkColor( colorBack );
	pDC->SetTextColor( colorFore );
	pDC->ExtTextOut( iLeft, iTop, ETO_OPAQUE, &rtItem, strWindowText,
						strWindowText.GetLength(), 0 );
}


/*----------------------------------------------------------------------------
	ChPrefsColorPage message handlers
----------------------------------------------------------------------------*/
	
void ChPrefsColorPage::OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	ChPropertyPage ::OnMeasureItem( nIDCtl, lpMeasureItemStruct );
}


void ChPrefsColorPage::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	switch( nIDCtl )
	{
		case IDC_COMBO_BACK_COLOR:
		case IDC_COMBO_TEXT_COLOR:
		case IDC_COMBO_LINK_COLOR:
		case IDC_COMBO_FLINK_COLOR:
		case IDC_COMBO_PLINK_COLOR:
		{
			CDC*	pDC;
			CRect	rtItem;
			int		iSavedDC;

			ASSERT( ODT_COMBOBOX == lpDrawItemStruct->CtlType );

											/* Grab a copy of the things that
												are needed over and over
												again */

			pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
			rtItem = lpDrawItemStruct->rcItem;
											/* Save the state of the passed-in
												DC */
			iSavedDC = pDC->SaveDC();
											// Draw the contents of the item

			if (lpDrawItemStruct->itemAction & ODA_DRAWENTIRE)
			{
				DrawColorItem( lpDrawItemStruct, pDC, rtItem );
			}
			else if (lpDrawItemStruct->itemAction & ODA_FOCUS)
			{
											// Toggle the focus rect
				CRect	rtFocus( rtItem );

				rtFocus.InflateRect( -1, -1 );
				pDC->DrawFocusRect( &rtFocus );
			}
			else if (lpDrawItemStruct->itemAction & ODA_SELECT)
			{
											// Toggle the selection rect
				bool	boolSelected;

				boolSelected = !!(lpDrawItemStruct->itemState & ODS_SELECTED);
				DrawColorSelect( lpDrawItemStruct, pDC, rtItem, boolSelected );
			}
											// Restore the original DC state
			pDC->RestoreDC( iSavedDC );
			break;
		}

		case IDC_SAMPLE_BACK:
		case IDC_SAMPLE_TEXT:
		case IDC_SAMPLE_LINK:
		case IDC_SAMPLE_FLINK:
		case IDC_SAMPLE_PLINK:
		{
			DrawColorSample( lpDrawItemStruct );
			break;
		}

		default:
		{
			ChPropertyPage::OnDrawItem( nIDCtl, lpDrawItemStruct );
			break;
		}
	}
}


void ChPrefsColorPage::OnSelchangeComboBackColor() 
{
	m_btnSampleBack.Invalidate( false );
	m_btnSampleText.Invalidate( false );
	m_btnSampleLink.Invalidate( false );
	m_btnSampleFLink.Invalidate( false );
	m_btnSamplePLink.Invalidate( false );

	// Invalidate the color combo boxes 
	m_comboBackColor.Invalidate( );
	m_comboTextColor.Invalidate( );
	m_comboLinkColor.Invalidate( );
	m_comboFLinkColor.Invalidate( );
	m_comboPLinkColor.Invalidate( );

}

void ChPrefsColorPage::OnSelchangeComboTextColor() 
{
	m_btnSampleText.Invalidate( false );
}

void ChPrefsColorPage::OnSelchangeComboLinkColor() 
{
	m_btnSampleLink.Invalidate( false );
}

void ChPrefsColorPage::OnSelchangeComboFlinkColor() 
{
	m_btnSampleFLink.Invalidate( false );
}

void ChPrefsColorPage::OnSelchangeComboPlinkColor() 
{
	m_btnSamplePLink.Invalidate( false );
}

// $Log$
