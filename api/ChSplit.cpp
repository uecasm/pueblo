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

	This file consists of interface for the ChSplitter class.  This class
	manages the splitter for the main frame.  There is only one of these
	splitter windows, created by the main application frame.  A pointer
	to the splitter is stored statically and may be obtained from the class.

	This is a MSW-only class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <limits.h>

#include <ChConst.h>
#include <ChCore.h>
#include <ChWnd.h>
#include <ChSplit.h>
#include <ChPane.h>

#if defined( _DEBUG )
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif	// defined( _DEBUG )


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChSplitter constants
----------------------------------------------------------------------------*/

#define CX_BORDER   1
#define CY_BORDER   1


/*----------------------------------------------------------------------------
	ChSplitter static values
----------------------------------------------------------------------------*/

//ChSplitter	*pSplitter = 0;

ChString		ChSplitter::m_strClass;


/*----------------------------------------------------------------------------
	ChSplitter class
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

IMPLEMENT_DYNAMIC( ChSplitter, CSplitterWnd )

#endif	// defined( CH_MSW )



void ChSplitter::RecalcLayout( bool boolHardReset )
{
	if (!m_boolInBannerRecalc)
	{
		RecalcBannerHeights();

		#if defined( CH_MSW )
		{
			if (!IsEmpty())
			{								// Resize the pane(s)
				chint16		sSinglePane;
				HDWP		hDeferWindowPos = 0;
				bool		boolSetPos = false;
				ChRect		rtClient;
				ChRect		rtPanes;

				GetClientRect( rtClient );
				GetPaneRect( rtPanes );

				sSinglePane = GetSinglePane();

				if (sSinglePane != -1)
				{							/* If there is a single pane, then
												fill the client area with this
												pane */
					chint32		lHeight;

					lHeight = rtPanes.Height();
					if (lHeight < 0)
					{
						lHeight = 0;
					}

					hDeferWindowPos =
								::BeginDeferWindowPos( m_sBannerCount + 1 );
					DeferClientPos( hDeferWindowPos, GetPane( sSinglePane ),
									rtPanes.left, rtPanes.top,
									rtPanes.Width(), lHeight );

					ResizeBanners( rtClient, hDeferWindowPos );
					::EndDeferWindowPos( hDeferWindowPos );
				}
				else
				{
					if (boolHardReset)
					{
						InitPaneSizes( rtClient );
					}

					RecalcPaneSizes( rtPanes );
					ResizePanes( rtPanes, rtClient );
				}
			}
		}
		#endif	// defined( CH_MSW )
	}
}


ChSplitterBanner* ChSplitter::CreateBanner( ChPane* pPane,
											const ChString& strName,
											bool boolTop, bool boolFrame,
											chint16 sClientWidth,
											chint16 sClientHeight )
{
	ChSplitterBanner*	pBanner;
	ChPosition			pos;

	pBanner = new ChSplitterBanner( this, pPane, strName, boolTop,
									sClientWidth, sClientHeight );
	ASSERT( pBanner );

	if (!boolTop && boolFrame)
	{										/* Add the bottom/frame banner to
												the end of the list */
		m_bannerList.AddTail( pBanner );
	}
	else if (boolTop && boolFrame)
	{										/* Add the top/frame banner to the
												head of the list */
		m_bannerList.AddHead( pBanner );
	}
	else if (boolTop && !boolFrame)
	{										/* Add this top/non-frame banner
												to the list after all top
												banners */
		if (pos = FindLastTopBanner())
		{
			m_bannerList.InsertAfter( pos, pBanner );
		}
		else
		{									/* No top banners -- add to end
												of list */
			m_bannerList.AddTail( pBanner );
		}
	}
	else	// (!boolTop && !boolFrame)
	{
											/* Add the bottom/non-frame banner
												after all top banners */
		if (pos = FindLastTopBanner())
		{
			m_bannerList.InsertAfter( pos, pBanner );
		}
		else
		{									/* No top banners -- add to head
												of list */
			m_bannerList.AddHead( pBanner );
		}
	}

	return pBanner;
}


void ChSplitter::DestroyBanner( ChWnd* pDestructoBanner )
{
	ChPosition	pos = m_bannerList.GetHeadPosition();
	bool		boolFoundAndDestroyed = false;

	while (pos && !boolFoundAndDestroyed)
	{
		ChSplitterBanner*	pThisBanner = m_bannerList.Get( pos );

		if (pThisBanner == pDestructoBanner)
		{
			if (pThisBanner->GetSafeHwnd())
			{
				pThisBanner->DestroyWindow();
			}

			m_bannerList.Remove( pos );
			boolFoundAndDestroyed = true;
		}
		else
		{
			m_bannerList.GetNext( pos );
		}
	}

	if (boolFoundAndDestroyed)
	{
		RecalcLayout();
	}
}


ChSplitterBanner* ChSplitter::FindBanner( const ChString& strName )
{
	ChPosition			pos;

	return FindBanner( strName, pos );
}


/*----------------------------------------------------------------------------
	ChSplitter protected methods
----------------------------------------------------------------------------*/

ChSplitter::ChSplitter() :
				m_reg( CH_LAYOUT_GROUP ),
				m_boolTracking( false ),
				m_sPaneCount( 0 ),
				m_boolInBannerRecalc( false )
{
	#if defined( CH_MSW )
	{
		chint16				iPane;
		const ChClientInfo*	pClientInfo = ChCore::GetClientInfo();
		OSType				osType = pClientInfo->GetPlatform();
		ChVersion			osVersion = pClientInfo->GetPlatformVersion();
		ChVersion			ntDrawEdgeVer( 3, 51 );

											/* Figure out if we can call the
												'DrawEdge' function */

		m_boolDrawEdgeAvailable = ((osWin95 == osType) ||
									(osWin98 == osType) ||
									(osWinXP == osType) ||
									((osWinNT == osType) &&
										(osVersion >= ntDrawEdgeVer)));

		m_boolWindows95 = ((osWin95 == osType) || (osWin98 == osType) || (osWinXP == osType));

		m_colorBtnFace = ::GetSysColor( COLOR_BTNFACE );
		m_colorBtnShadow = ::GetSysColor( COLOR_BTNSHADOW );
		m_colorBtnHilite = ::GetSysColor( COLOR_BTNHIGHLIGHT );

		m_hArrow = ::LoadCursor( 0, IDC_ARROW );

		m_reg.ReadBool( CH_LAYOUT_SWAPPED, m_boolSwapped, true );
		m_reg.ReadBool( CH_LAYOUT_VERTICAL, m_boolVertical, false );

		for (iPane = 0; iPane < maxPanes; iPane++)
		{
			chint16		sPaneSize;
			char		buffer[80];
											// Get the pane sizes

			sprintf( buffer, CH_LAYOUT_PANE_SIZE, iPane );
			m_reg.Read( buffer, sPaneSize, CH_LAYOUT_PANE_SIZE_DEF );

			SetPaneInUse( iPane, false );
											// All Panes are initially hidden
			SetPaneVisible( iPane, false );
											// Set initial pane standards
		
			SetPaneInfo( iPane, sPaneSize, sPaneSize, 10, sPaneSize, 10 );
		}
											/* Set default splitter box/bar
												sizes (includes borders) */
		if (m_boolDrawEdgeAvailable)
		{
			m_s3dBorderSize = 2;
		}
		else
		{
			m_s3dBorderSize = 1;
		}

		m_sBorderSize = 1;
		m_sSplitterSize = 3 + m_s3dBorderSize + m_s3dBorderSize;
		m_sBorderShareSize = m_sBorderSize * 2;
		m_sSplitterGapSize = m_sSplitterSize;

		if (0 == m_strClass.GetLength())
		{
			m_strClass =
				AfxRegisterWndClass( CS_DBLCLKS, LoadCursor( 0, IDC_ARROW ),
										(HBRUSH)(COLOR_APPWORKSPACE + 1) );		// UE
		}

		#if defined( CH_ARCH_16 )
		{
			m_halftoneBrush = NULL;
		}
		#endif	// defined( CH_ARCH_16 )

		GetSplitterCursor();
	}
	#endif	// defined( CH_MSW )
}


ChSplitter::~ChSplitter()
{
	for (int iPane = 0; iPane < maxPanes; iPane++)
	{
		chint16		sPaneSize;
		char		buffer[80];
											// Save the pane sizes

		sprintf( buffer, CH_LAYOUT_PANE_SIZE, iPane );

		if (IsVertical())
		{
			sPaneSize = (chint16)m_paneInfo[iPane].iIdealWidth;
		}
		else
		{
			sPaneSize = (chint16)m_paneInfo[iPane].iIdealHeight;
		}

		m_reg.Write( buffer, sPaneSize );
	}

}


BOOL ChSplitter::Create( ChWnd* pParentWnd )
{
#if defined( CH_MSW )
	ChRect	rtEmpty;
										/* AFX_IDW_PANE_FIRST is a special
											window ID that indicates to
											the parent frame that this
											window should occupy all
											remaining space after the bars
											are positioned */

	return CWnd::Create( m_strClass, "ChSplitter window",
								WS_CHILD | WS_VISIBLE, rtEmpty,
								pParentWnd, AFX_IDW_PANE_FIRST );
#else	// defined( CH_MSW )
	return FALSE;
#endif	// defined( CH_MSW )
}


void ChSplitter::SetWnd( int iPane, ChWnd* pWnd )
{
	chint16		sOldVisibleCount = m_sPaneCount;
	bool		boolHardReset;

	ASSERT( pWnd->m_hWnd != 0 );
											// Do bookkeeping on our pane count
	SetPaneInUse( iPane );
	CountVisiblePanes();
											// Store the window
	m_pPanes[iPane] = pWnd;

	boolHardReset = (sOldVisibleCount != m_sPaneCount);

											/* If the pane count is changing,
												then recalculate the layout */
	RecalcLayout( boolHardReset );
}


void ChSplitter::EmptyPane( int iPane )
{
	SetPaneInUse( iPane, false );
	m_pPanes[iPane] = 0;
											/* Count remaining visible panes
												and recalc the layout */
	if (CountVisiblePanes())
	{
		RecalcLayout();
	}
}


void ChSplitter::Show( int iPane, bool boolShow )
{
	ASSERT( iPane < maxPanes );
	ASSERT( iPane >= 0 );

	if (m_boolPaneShown[iPane] != boolShow)
	{										// Status is changing

		ChWnd*	pChild = m_pPanes[iPane];

		if (pChild)
		{
			if (boolShow)
			{
				pChild->ShowWindow( SW_SHOW );
			}
			else
			{
				pChild->ShowWindow( SW_HIDE );
			}
		}

		SetPaneVisible( iPane, boolShow );

		if (CountVisiblePanes())
		{
			RecalcLayout();
		}
	}
}


void ChSplitter::GetPaneInfo( int iPane, chint16& sIdealWidth,
								chint16& sMinWidth, chint16& sIdealHeight,
								chint16& sMinHeight )
{
	ChPaneInfo*		pInfo;

	pInfo = &m_paneInfo[iPane];

	sMinWidth = (chint16)pInfo->iMinWidth;
	sIdealWidth = (chint16)pInfo->iIdealWidth;
	sMinHeight = (chint16)pInfo->iMinHeight;
	sIdealHeight = (chint16)pInfo->iIdealHeight;
}


void ChSplitter::SetPaneInfo( int iPane, chint16 sIdealWidth,
								chint16 sMinWidth, chint16 sIdealHeight,
								chint16 sMinHeight )
{
	ChPaneInfo*		pInfo;

	pInfo = &m_paneInfo[iPane];

	pInfo->iMinWidth = sMinWidth;
	pInfo->iMinHeight = sMinHeight;
											/* Resize the pane only if it is
												smaller than the new minimum */
	if (IsVertical())
	{
		pInfo->iIdealHeight = sIdealHeight;

		if (pInfo->iCurSize < pInfo->iMinWidth)
		{
			pInfo->iCurSize = -1;
		}
											/* pInfo->iIdealWidth is used to
												store the current sized width,
												so only screw it up if we
												really need to... */
		if ((-1 == pInfo->iCurSize) ||
			(pInfo->iMinWidth > pInfo->iIdealWidth))
		{
			pInfo->iIdealWidth = sIdealWidth;
		}
	}
	else
	{
		pInfo->iIdealWidth = sIdealWidth;

		if (pInfo->iCurSize < pInfo->iMinHeight)
		{
			pInfo->iCurSize = -1;
		}
											/* pInfo->iIdealHeight is used to
												store the current sized height,
												so only screw it up if we
												really need to... */
		if ((-1 == pInfo->iCurSize) ||
			(pInfo->iMinHeight > pInfo->iIdealHeight))
		{
			pInfo->iIdealHeight = sIdealHeight;
		}
	}

	if (pInfo->iCurSize < 0)
	{
		RecalcLayout();
	}
}


chint16 ChSplitter::GetSinglePane()
{
	chint16		sLoop;

	if (1 != m_sPaneCount)
	{										// Not a single pane
		return -1;
	}
	else
	{
		for (sLoop = 0; sLoop < maxPanes; sLoop++)
		{
			if (IsPaneVisible( sLoop ))
			{
											// Single pane found
				return sLoop;
			}
		}
	}
											// Single pane not found
	return -1;
}


void ChSplitter::SwapPanes()
{
	m_boolSwapped = !m_boolSwapped;
	m_reg.WriteBool( CH_LAYOUT_SWAPPED, m_boolSwapped );

											// Invalidate the window
	RecalcLayout();
}


void ChSplitter::TogglePaneOrientation()
{
	m_boolVertical = !m_boolVertical;
	m_reg.WriteBool( CH_LAYOUT_VERTICAL, m_boolVertical );

											// Update the cursor
	GetSplitterCursor();
											// Invalidate the window
	RecalcLayout();
}


int ChSplitter::GetIdFromPane( int iPane ) const
{
	ASSERT( 0 != this );

	ASSERT( iPane >= 0 );
	ASSERT( iPane < maxPanes );

	return AFX_IDW_PANE_FIRST + iPane;
}


ChPosition ChSplitter::FindLastTopBanner() const
{
	ChPosition		pos = m_bannerList.GetHeadPosition();
	ChPosition		lastPos = 0;
	bool			boolBottomPane = false;

	while (pos && !boolBottomPane)
	{
		const ChSplitterBanner*	pThisBanner = m_bannerList.Get( pos );

		if (!(boolBottomPane = !pThisBanner->IsTop()))
		{
			lastPos = pos;
			m_bannerList.GetNext( pos );
		}
	}

	return lastPos;
}


#if defined( CH_MSW )

BEGIN_MESSAGE_MAP( ChSplitter, CWnd )
	//{{AFX_MSG_MAP(ChSplitter)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CTLCOLORDLG, OnCtlColorStatic )
#if defined(CH_PUEBLO_PLUGIN)
	ON_WM_ERASEBKGND()
#endif
END_MESSAGE_MAP()

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChSplitter protected methods
----------------------------------------------------------------------------*/

void ChSplitter::SetPaneInfo( int iPane, chint16 sCurSize,
								chint16 sIdealWidth, chint16 sMinWidth,
								chint16 sIdealHeight, chint16 sMinHeight )
{
	ChPaneInfo*		pInfo;

	pInfo = &m_paneInfo[iPane];

	pInfo->iCurSize = sCurSize;

	SetPaneInfo( iPane, sIdealWidth, sMinWidth, sIdealHeight, sMinHeight );
}


bool ChSplitter::CountVisiblePanes()
{											/* This method returns true if
												the number of visible panes
												is changing */
	chint16		iPane;
	chint16		sOldCount = m_sPaneCount;

	m_sPaneCount = 0;

	for (iPane = 0; iPane < maxPanes; iPane++)
	{
		if (IsPaneVisible( iPane ))
		{
			m_sPaneCount++;
		}
	}

	return sOldCount != m_sPaneCount;
}


bool ChSplitter::HitTest( ChPoint pt, int* piPaneSplitter ) const
{
	if (GetPaneCount() <= 1)
	{
		return false;
	}

	ChRect		rtPanes;

	#if defined( CH_MSW )
	{
		ASSERT_VALID( this );
	}
	#endif	// defined( CH_MSW )

	GetPaneRect( rtPanes );
											/* For hit detect, include the
												border of splitters */
	ChRect	rtTest( rtPanes );
	bool	boolFound( false );
	int		iPane;
	int		iPaneFound;

	for (iPane = 0; iPane < maxPanes - 1; iPane++)
	{
		int		iPaneIndex;

		iPaneIndex = GetPaneIndex( iPane );

		if (IsVertical())
		{
			rtTest.left += m_paneInfo[iPaneIndex].iCurSize;
			rtTest.right = rtTest.left + m_sSplitterGapSize;
		}
		else
		{
			rtTest.top += m_paneInfo[iPaneIndex].iCurSize;
			rtTest.bottom = rtTest.top + m_sSplitterGapSize;
		}

		if (rtTest.PtInRect( pt ))
		{
			boolFound = true;
			iPaneFound = iPane;
		}
		else
		{
			if (IsVertical())
			{
				rtTest.left = rtTest.right;
			}
			else
			{
				rtTest.top = rtTest.bottom;
			}
		}
	}

	if (boolFound)
	{
		if (piPaneSplitter)
		{
			*piPaneSplitter = iPaneFound;
		}
	}

	return boolFound;
}

#if defined( CH_MSW )

void ChSplitter::DeferClientPos( HDWP& hDeferWindowPos, CWnd* pWnd,
									int x, int y, int cx, int cy )
{
	ASSERT( pWnd != 0 );
	ASSERT( pWnd->m_hWnd != 0 );

	CRect	rtNew( x, y, x + cx, y + cy );
	CRect	rtOld;
											/* Adjust for border size
												(even if zero client size) */
	pWnd->CalcWindowRect( &rtNew );
											/* First check if the new rectangle
												is the same as the current */

	pWnd->GetWindowRect( rtOld );
	pWnd->GetParent()->ScreenToClient( &rtOld );

	if (rtNew != rtOld)
	{
		if (hDeferWindowPos)
		{
			hDeferWindowPos = ::DeferWindowPos( hDeferWindowPos, pWnd->m_hWnd,
												0, rtNew.left, rtNew.top,
												rtNew.Width(), rtNew.Height(),
												SWP_NOACTIVATE | SWP_NOZORDER );
		}
		else
		{
			::SetWindowPos( pWnd->m_hWnd, 0, rtNew.left, rtNew.top,
							rtNew.Width(), rtNew.Height(),
							SWP_NOACTIVATE | SWP_NOZORDER );
		}
	}
}

#endif	// defined( CH_MSW )

#if defined( CH_MSW )

void ChSplitter::InvalidateSplitBar()
{
	ChRect	rtSplitter;

	GetPaneRect( rtSplitter );

	if (IsVertical())
	{
		rtSplitter.bottom++;
	}
	else
	{
		rtSplitter.right++;
	}

	for (int iPane = 0; iPane < maxPanes - 1; iPane++)
	{
		int		iPaneIndex = GetPaneIndex( iPane );

		if (IsVertical())
		{
			rtSplitter.left += m_paneInfo[iPaneIndex].iCurSize +
								m_sBorderSize;
			rtSplitter.right = rtSplitter.left + m_sSplitterSize;
		}
		else
		{
			rtSplitter.top += m_paneInfo[iPaneIndex].iCurSize +
								m_sBorderSize;
			rtSplitter.bottom = rtSplitter.top + m_sSplitterSize;
		}

		InvalidateRect( rtSplitter, false );
	}
}


void ChSplitter::DrawAllSplitBars( CDC* pDC, int iInsideSpace )
{
	ASSERT_VALID( this );

	if (m_sPaneCount > 1)
	{										// Draw column split bars
		ChRect	rtSplitter;
		int		iAdjust = IsWindows95() ? 0 : 1;

		GetPaneRect( rtSplitter );

		if (IsVertical())
		{
			rtSplitter.bottom += iAdjust;
		}
		else
		{
			rtSplitter.right += iAdjust;
		}

		for (int iPane = 0; iPane < maxPanes - 1; iPane++)
		{
			int		iPaneIndex = GetPaneIndex( iPane );

			if (IsVertical())
			{
				rtSplitter.left += m_paneInfo[iPaneIndex].iCurSize;
				rtSplitter.right = rtSplitter.left + m_sSplitterSize;

				if (rtSplitter.right > iInsideSpace)
				{
					break;						// Stop if not fully visible
				}
			}
			else
			{
				rtSplitter.top += m_paneInfo[iPaneIndex].iCurSize;
				rtSplitter.bottom = rtSplitter.top + m_sSplitterSize;

				if (rtSplitter.bottom > iInsideSpace)
				{
					break;						// Stop if not fully visible
				}
			}

			OnDrawSplitter( pDC, rtSplitter );

			if (IsVertical())
			{
				rtSplitter.left = rtSplitter.right + m_sBorderShareSize;
			}
			else
			{
				rtSplitter.top = rtSplitter.bottom + m_sBorderShareSize;
			}
		}
	}
}

#endif	// defined( CH_MSW )

#if defined( CH_MSW )

void ChSplitter::OnDrawSplitter( CDC* pDC, const CRect& rtSplitBar )
{
	if (0 == pDC)
	{
		RedrawWindow( rtSplitBar, 0, RDW_INVALIDATE | RDW_NOCHILDREN );
		return;
	}

	ASSERT_VALID( pDC );

	CRect	rect( rtSplitBar );

	#if defined( CH_ARCH_16 )
	{
		Draw3dRect( pDC, rect, m_colorBtnHilite, m_colorBtnShadow );
	}
	#else	// defined( CH_ARCH_16 )
	{
		if (m_boolDrawEdgeAvailable)
		{
			HDC		hDC = pDC->GetSafeHdc();

			DrawEdge( hDC, &rect, EDGE_RAISED, BF_RECT );
		}
		else
		{
			pDC->Draw3dRect( rect, m_colorBtnHilite, m_colorBtnShadow );
		}
	}
	#endif	// defined( CH_ARCH_16 )

	rect.InflateRect( -m_s3dBorderSize, -m_s3dBorderSize );

											// Fill the middle
	#if defined( CH_ARCH_16 )
	{
		FillSolidRect( pDC, rect.left, rect.top, rect.right - rect.left,
						rect.bottom - rect.top, m_colorBtnFace );
	}
	#else	// defined( CH_ARCH_16 )
	{
		pDC->FillSolidRect( rect, m_colorBtnFace );
	}
	#endif	// defined( CH_ARCH_16 )
}

#endif	// defined( CH_MSW )

#if defined( CH_MSW )

void ChSplitter::OnInvertTracker( const ChRect& rtTracker )
{
	ASSERT( !rtTracker.IsRectEmpty() );
	ASSERT( (GetStyle() & WS_CLIPCHILDREN) == 0 );

											// Pat-blt without clip children on
	CDC*		pDC = GetDC();
	#if defined( CH_ARCH_16 )
	CBrush*		pBrush = GetHalftoneBrush();
	#else
	CBrush*		pBrush = CDC::GetHalftoneBrush();
	#endif
	HBRUSH		hOldBrush = 0;
											/* Inverting the brush pattern looks
												just like frame window sizing */
	if (pBrush != 0)
	{
		hOldBrush = (HBRUSH)SelectObject( pDC->m_hDC, pBrush->m_hObject );
	}

	pDC->PatBlt( rtTracker.left, rtTracker.top, rtTracker.Width(),
					rtTracker.Height(), PATINVERT );

	if (hOldBrush != 0)
	{
		SelectObject( pDC->m_hDC, hOldBrush );
	}

	ReleaseDC( pDC );
}

#endif	// defined( CH_MSW )

#if defined( CH_MSW )

void ChSplitter::TrackPaneSize( int iPane, int iPos )
{
	ASSERT( m_sPaneCount > 1 );

	if (IsVertical())
	{
		CPoint		pt( iPos, 0 );

		ClientToScreen( &pt );
		GetPane( iPane )->ScreenToClient( &pt );

		m_paneInfo[iPane].iIdealWidth = pt.x;// New size
		if (pt.x < m_paneInfo[iPane].iMinWidth)
		{									/* Resized too small... Make it
												go away */
			m_paneInfo[iPane].iIdealWidth = 0;
		}
	}
	else
	{
		CPoint		pt( 0, iPos );

		ClientToScreen( &pt );
		GetPane( iPane )->ScreenToClient( &pt );

		m_paneInfo[iPane].iIdealHeight = pt.y;// New size
		if (pt.y < m_paneInfo[iPane].iMinHeight)
		{									/* Resized too small... Make it
												go away */
			m_paneInfo[iPane].iIdealHeight = 0;
		}
	}
}

void ChSplitter::StartTracking( int iPaneSplitter )
{
											/* GetHitRect will restrict
												'm_rectLimit' as appropriate */
	GetClientRect( m_rtTrackerLimit );

	GetHitRect( iPaneSplitter, m_rtTracker );

											// Steal focus and capture
	SetCapture();
	SetFocus();
											// Make sure no updates are pending

	RedrawWindow( 0, 0, RDW_ALLCHILDREN | RDW_UPDATENOW );

											/* Set tracking state and
												appropriate cursor */
	m_boolTracking = true;
	OnInvertTracker( m_rtTracker );

	SetCursor( m_hcurSplitter );
}


void ChSplitter::StopTracking( bool boolAccept )
{
	if (!m_boolTracking)
	{
		return;
	}

	ReleaseCapture();
											// Erase tracker rectangle
	OnInvertTracker( m_rtTracker );
	m_boolTracking = false;

	// m_rtTracker is set to the new splitter position (without border)
	// (so, adjust relative to where the border will be)

	m_rtTracker.OffsetRect( -CX_BORDER , -CY_BORDER );

	if (boolAccept)
	{										// Set column width
		int		iPane = m_boolSwapped ? 1 : 0;
		int		iPos = IsVertical() ? m_rtTracker.left : m_rtTracker.top;

		TrackPaneSize( iPane, iPos );
		RecalcLayout();
	}
}

#endif	// defined( CH_MSW )


void ChSplitter::GetHitRect( int iPaneSplitter, ChRect& rtHit )
{
	ChRect		rtPanes;
	int			iX;
	int			iY;
	int			iWidth;
	int			iHeight;
	int			iPane;
	int			iPaneIndex;

	GetPaneRect( rtPanes );

	iWidth = rtPanes.Width();
	iHeight = rtPanes.Height();
	iX = rtPanes.left;
	iY = rtPanes.top;
											/* Hit rectangle does not include
												border.  m_rtTrackerLimit will
												be limited to valid tracking
												rect */
	m_ptTrackerOffset.x = 0;
	m_ptTrackerOffset.y = 0;

	if (IsVertical())
	{
		iWidth = m_sSplitterSize - (2 * m_sBorderSize );

		m_ptTrackerOffset.x = -(iWidth / 2);
		for (iPane = 0; iPane < iPaneSplitter; iPane++)
		{
			iPaneIndex = GetPaneIndex( iPane );

			iX += m_paneInfo[iPaneIndex].iCurSize + m_sSplitterGapSize;
		}

		m_rtTrackerLimit.left = iX;
		iPaneIndex = GetPaneIndex( iPane );
		iX += m_paneInfo[iPaneIndex].iCurSize + m_sBorderShareSize;
		m_rtTrackerLimit.right -= iWidth;
	}
	else
	{
		iHeight = m_sSplitterSize - (2 * m_sBorderSize );

		m_ptTrackerOffset.y = -(iHeight / 2);
		for (iPane = 0; iPane < iPaneSplitter; iPane++)
		{
			iPaneIndex = GetPaneIndex( iPane );

			iY += m_paneInfo[iPaneIndex].iCurSize + m_sSplitterGapSize;
		}

		m_rtTrackerLimit.top = iY;
		iPaneIndex = GetPaneIndex( iPane );
		iY += m_paneInfo[iPaneIndex].iCurSize + m_sBorderShareSize;
		m_rtTrackerLimit.bottom -= iHeight;
	}

	rtHit.right = (rtHit.left = iX) + iWidth;
	rtHit.bottom = (rtHit.top = iY) + iHeight;
}


#if defined( CH_ARCH_16 )

CBrush*  ChSplitter::GetHalftoneBrush()
{
	if (m_halftoneBrush == NULL)
	{
		WORD grayPattern[8];
		for (int i = 0; i < 8; i++)
			grayPattern[i] = (WORD)(0x5555 << (i & 1));
		HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
		if (grayBitmap != NULL)
		{
			m_halftoneBrush = ::CreatePatternBrush(grayBitmap);
			DeleteObject(grayBitmap);
		}
	}

	return CBrush::FromHandle( m_halftoneBrush );
}

void ChSplitter::FillSolidRect( CDC *pDC, int x, int y, int cx, int cy, COLORREF clr)
{
	ASSERT_VALID(this);
	ASSERT(pDC->m_hDC != NULL);

	::SetBkColor( pDC->m_hDC, clr);
	CRect rect(x, y, x + cx, y + cy);
	::ExtTextOut( pDC->m_hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

void ChSplitter::Draw3dRect( CDC *pDC, LPCRECT lpRect,
	COLORREF clrTopLeft, COLORREF clrBottomRight)
{
	FillSolidRect( pDC, lpRect->left, lpRect->top, (lpRect->right - lpRect->left) - 1, 1, clrTopLeft);
	FillSolidRect( pDC, lpRect->left, lpRect->top, 1,
						(lpRect->bottom - lpRect->top) - 1, clrTopLeft);

	FillSolidRect( pDC, lpRect->left + (lpRect->right - lpRect->left), lpRect->top, -1,
								(lpRect->bottom - lpRect->top),  clrBottomRight);
	FillSolidRect( pDC, lpRect->left, lpRect->top + (lpRect->bottom - lpRect->top),
								(lpRect->right - lpRect->left), -1, clrBottomRight);

}

#endif	// defined( CH_ARCH_16 )


void ChSplitter::GetSplitterCursor()
{
	#if defined( CH_MSW )
	{
		if (IsVertical())
		{
			#if defined( CH_PUEBLO_PLUGIN )
				m_hcurSplitter =
					::LoadCursor( AfxGetInstanceHandle(),
									MAKEINTRESOURCE( IDC_COL_SPLITTER ) );
			#else
				m_hcurSplitter =
					::LoadCursor( PuebloDLL.hModule,
									MAKEINTRESOURCE( IDC_COL_SPLITTER ) );
			#endif
		}
		else
		{
			#if defined( CH_PUEBLO_PLUGIN )
				m_hcurSplitter =
					::LoadCursor( AfxGetInstanceHandle(),
									MAKEINTRESOURCE( IDC_ROW_SPLITTER ) );
			#else
				m_hcurSplitter =
					::LoadCursor( PuebloDLL.hModule,
									MAKEINTRESOURCE( IDC_ROW_SPLITTER ) );
			#endif
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )
}


void ChSplitter::InitPaneSizes( const ChRect& rtClient )
{
	int				iPane;
	ChPaneInfo* 	pInfo;
	int				iPaneIndex;

	for (iPane = 0; iPane < maxPanes - 1; iPane++)
	{
		iPaneIndex = GetPaneIndex( iPane );
		pInfo = &m_paneInfo[iPaneIndex];

		if (IsVertical())
		{
			if (pInfo->iIdealWidth < pInfo->iMinWidth)
			{
				pInfo->iIdealWidth = 0;		// Too small to see
			}

			pInfo->iCurSize = pInfo->iIdealWidth;
		}
		else
		{
			if (pInfo->iIdealHeight < pInfo->iMinHeight)
			{
				pInfo->iIdealHeight = 0;	// Too small to see
			}

			pInfo->iCurSize = pInfo->iIdealHeight;
		}
	}
											// Last row/column takes the rest

	iPaneIndex = m_boolSwapped ? 0 : 1;
	m_paneInfo[iPaneIndex].iCurSize = INT_MAX;
}


void ChSplitter::RecalcPaneSizes( const ChRect& rtPanes )
{
	int				iPane;
	int				iSpaceRemaining;
	ChPaneInfo* 	pInfo;
	ChPaneInfo* 	pPrevInfo;
	int				iPaneIndex;

	if (IsVertical())
	{
		iSpaceRemaining = rtPanes.Width();
	}
	else
	{
		iSpaceRemaining = rtPanes.Height();
	}

	if (iSpaceRemaining < 0)
	{
		iSpaceRemaining = 0;
	}

	pInfo = 0;
	for (iPane = 0; iPane < maxPanes; iPane++)
	{
		int		iMinSize;

		iPaneIndex = GetPaneIndex( iPane );
		pPrevInfo = pInfo;
		pInfo = &m_paneInfo[iPaneIndex];
		iMinSize = IsVertical() ? pInfo->iMinWidth : pInfo->iMinHeight;

		if (IsPaneVisible( iPaneIndex ))
		{
			ASSERT( iSpaceRemaining >= 0 );

			if (iSpaceRemaining == 0)
			{								/* No more room (set pane to be
												zero width) */
				pInfo->iCurSize = 0;
			}
			else if ((iSpaceRemaining < iMinSize) && (iPane != 0))
			{
											/* Additional panes below the
												recommended minimum size aren't
												shown and the size goes to the
												previous pane */
				pInfo->iCurSize = 0;
											/* Previous pane already has room
												for splitter + border add
												remaining size and remove the
												extra border */

				pPrevInfo->iCurSize += iSpaceRemaining + m_sBorderSize;
				iSpaceRemaining = 0;
			}
			else
			{								// We can add the pane

				ASSERT( iSpaceRemaining > 0 );

				if (0 == pInfo->iCurSize)
				{
											// Too small to see
					if (0 != iPane)
					{
						pInfo->iCurSize = 0;
					}
				}
				else if (iSpaceRemaining < pInfo->iCurSize)
				{
											/* This row/col won't fit
												completely - make as small
												as possible */

					pInfo->iCurSize = iSpaceRemaining;
					iSpaceRemaining = 0;
				}
				else
				{					// Can fit everything

					iSpaceRemaining -= pInfo->iCurSize;
				}
			}
		}
		else
		{									// Pane is invisible
			pInfo->iCurSize = 0;
		}
											// See if we should add a splitter
		ASSERT( iSpaceRemaining >= 0 );

		if (iPane != maxPanes - 1)
		{									// Should have a splitter

			if (iSpaceRemaining > m_sSplitterGapSize)
			{
				 							// Leave room for splitter & border

				iSpaceRemaining -= m_sSplitterGapSize;
				ASSERT( iSpaceRemaining > 0 );
			}
			else
			{								/* Not enough room - add left over
												less splitter size */

				int		iLastSplitterSize;

				pInfo->iCurSize += iSpaceRemaining;

				iLastSplitterSize = m_sSplitterGapSize;

				if (pInfo->iCurSize > iLastSplitterSize)
				{
					pInfo->iCurSize -= iLastSplitterSize;
				}

				iSpaceRemaining = 0;
			}
		}
	}
}


void ChSplitter::ResizePanes( const ChRect& rtPanes, const ChRect& rtClient )
{
	#if defined( CH_MSW )
	{
		HDWP	hDeferWindowPos = 0;
		int		iX = rtPanes.left;
		int		iY = rtPanes.top;
		int		iPane;

		hDeferWindowPos = ::BeginDeferWindowPos( maxPanes );

											// Resize the banner panes

		ResizeBanners( rtClient, hDeferWindowPos );

		for (iPane = 0; iPane < maxPanes; iPane++)
		{
			int		iPaneIndex = GetPaneIndex( iPane );

			if (IsPaneVisible( iPane ))
			{
				if (IsVertical())
				{
					int		iWidth = m_paneInfo[iPaneIndex].iCurSize;

					DeferClientPos( hDeferWindowPos, GetPane( iPaneIndex ),
									iX, iY, iWidth, rtPanes.Height() );

					iX += iWidth + m_sSplitterGapSize;
				}
				else
				{
					int		iHeight = m_paneInfo[iPaneIndex].iCurSize;

					DeferClientPos( hDeferWindowPos, GetPane( iPaneIndex ),
									iX, iY, rtPanes.Width(), iHeight );

					iY += iHeight + m_sSplitterGapSize;
				}
			}
		}

		if (0 != hDeferWindowPos)
		{
			::EndDeferWindowPos( hDeferWindowPos );
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )
}


void ChSplitter::RecalcFitBanners()
{
	ChPosition		pos;
	bool			boolOldBannerRecalc = m_boolInBannerRecalc;

	m_boolInBannerRecalc = true;

	pos = m_bannerList.GetHeadPosition();
	while (pos)
	{
		ChSplitterBanner*	pBanner = m_bannerList.Get( pos );
		ChPane*				pPane = pBanner->GetPane();

		if (pPane && pPane->IsSizeToFit())
		{
			pPane->SizeToFit();
		}

		m_bannerList.GetNext( pos );
	}

	m_boolInBannerRecalc = boolOldBannerRecalc;
}


void ChSplitter::ResizeBanners( const ChRect& rtClient, HDWP& hDeferWindowPos )
{
	chint32			lTopBannerY;
	chint32			lBottomBannerY;
	ChPosition		pos;

	if (m_sBannerCount)
	{
		lTopBannerY = rtClient.top;
		lBottomBannerY = rtClient.bottom - m_lBannerBottom;
		if (lTopBannerY + m_lBannerTop > lBottomBannerY)
		{
			lBottomBannerY = lTopBannerY + m_lBannerTop;
		}

		pos = m_bannerList.GetHeadPosition();
		while (pos)
		{
			ChSplitterBanner*	pBanner = m_bannerList.Get( pos );

			if (!pBanner->IsHidden())
			{
				CSize		size;
				chint32		lTop;

				pBanner->GetSize( size );

				if (pBanner->IsTop())
				{
					lTop = lTopBannerY;
					lTopBannerY += size.cy;
				}
				else
				{
					lTop = lBottomBannerY;
					lBottomBannerY += size.cy;
				}

				#if defined( CH_MSW )
				{
					int		iAdjust = IsWindows95() ? 0 : 1;

					DeferClientPos( hDeferWindowPos, pBanner,
									rtClient.left, lTop,
									rtClient.Width() + iAdjust,
									size.cy );
				}
				#else	// defined( CH_MSW )
				{
					cerr << "Not implemented: " << __FILE__ << ":" <<
													__LINE__ << endl;
				}
				#endif	// defined( CH_MSW )
			}

			m_bannerList.GetNext( pos );
		}
	}
}


ChSplitterBanner* ChSplitter::FindBanner( const ChString& strName,
											ChPosition& pos )
{
	ChSplitterBanner*	pBanner = 0;

	pos = m_bannerList.GetHeadPosition();
	while ((0 == pBanner) && pos)
	{
		ChSplitterBanner*	pThisBanner = m_bannerList.Get( pos );
		ChString				strThisName;

		pThisBanner->GetWindowText( strThisName );
		if (strThisName == strName)
		{
			pBanner = pThisBanner;
		}
		else
		{
			m_bannerList.GetNext( pos );
		}
	}

	return pBanner;
}


/*----------------------------------------------------------------------------
	ChSplitter::RecalcBannerHeights

	This function will recalculate the heights of banner panes.  (It does
	not resize the panes.)
----------------------------------------------------------------------------*/

void ChSplitter::RecalcBannerHeights()
{
	ChPosition		pos;

	m_lBannerTop = m_lBannerBottom = 0;
	m_sBannerCount = 0;

	pos = m_bannerList.GetHeadPosition();
	while (pos)
	{
		ChSplitterBanner*	pBanner = m_bannerList.Get( pos );

		if (!pBanner->IsHidden())
		{
			CSize	size;

			pBanner->GetSize( size );

			if (pBanner->IsTop())
			{
				m_lBannerTop += size.cy;
			}
			else
			{
				m_lBannerBottom += size.cy;
			}

			m_sBannerCount++;
		}

		m_bannerList.GetNext( pos );
	}
}


void ChSplitter::GetPaneRect( ChRect& rtPane ) const
{
	#if defined( CH_MSW )
	{
		GetClientRect( &rtPane );
	}
	#else	// defined( CH_MSW )
	{
		cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )

	rtPane.top += m_lBannerTop;
	rtPane.bottom -= m_lBannerBottom;
}


/*----------------------------------------------------------------------------
	ChSplitter message handlers
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

void ChSplitter::OnSize( UINT nType, int cx, int cy )
{
	if (nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		RecalcFitBanners();
		RecalcLayout();
		InvalidateSplitBar();
	}

	CWnd::OnSize( nType, cx, cy );
}

void ChSplitter::OnPaint()
{
	CPaintDC	dc( this );					// Device context for painting
	CRect		rtClient;
	int			iInsideSpace;

	GetClientRect( &rtClient );
	rtClient.InflateRect( -m_sBorderSize, -m_sBorderSize );

											/* Draw the splitter boxes & the
												space between them */

	iInsideSpace = IsVertical() ? rtClient.right : rtClient.bottom;
	DrawAllSplitBars( &dc, iInsideSpace );
}


void ChSplitter::OnMouseMove( UINT nFlags, CPoint ptMouse )
{
	if (GetCapture() != this)
	{
		StopTracking( false );
	}

	if (m_boolTracking)
	{
		bool	boolChanged;
											/* Move tracker to current cursor
												position */

											/* Make ptMouse the upper right of
												hit detect */
		ptMouse.Offset( m_ptTrackerOffset );
											/* Limit the point to the valid
												split range */
		if (ptMouse.y < m_rtTrackerLimit.top)
		{
			ptMouse.y = m_rtTrackerLimit.top;
		}
		else if (ptMouse.y > m_rtTrackerLimit.bottom)
		{
			ptMouse.y = m_rtTrackerLimit.bottom;
		}

		if (ptMouse.x < m_rtTrackerLimit.left)
		{
			ptMouse.x = m_rtTrackerLimit.left;
		}
		else if (ptMouse.x > m_rtTrackerLimit.right)
		{
			ptMouse.x = m_rtTrackerLimit.right;
		}

		boolChanged = IsVertical() ? (m_rtTracker.left != ptMouse.x) :
										(m_rtTracker.top != ptMouse.y);

		if (boolChanged)
		{
			OnInvertTracker( m_rtTracker );

			if (IsVertical())
			{
				m_rtTracker.OffsetRect( ptMouse.x - m_rtTracker.left, 0 );
			}
			else
			{
				m_rtTracker.OffsetRect( 0, ptMouse.y - m_rtTracker.top );
			}

			OnInvertTracker( m_rtTracker );
		}
	}
	else
	{										/* Simply hit-test and set
												appropriate cursor */
		int		iSplitterCol;

		if (HitTest( ptMouse, &iSplitterCol ))
		{
			SetCursor( m_hcurSplitter );
		}
		else
		{
			SetCursor( m_hArrow );
		}
	}
}


void ChSplitter::OnLButtonDown( UINT nFlags, CPoint ptMouse )
{
	if (!m_boolTracking)
	{
		int		iPaneSplitter;

		if (HitTest( ptMouse, &iPaneSplitter ))
		{
			StartTracking( iPaneSplitter );
		}
	}
}


void ChSplitter::OnLButtonUp( UINT nFlags, CPoint ptMouse )
{
	StopTracking( true );
}


void ChSplitter::OnCancelMode()
{
	CWnd::OnCancelMode();

	StopTracking( false );
}


HBRUSH ChSplitter::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH	hbr = CWnd::OnCtlColor( pDC, pWnd, nCtlColor );

	if (CTLCOLOR_STATIC == nCtlColor)
	{
		hbr = 0;
	}

	return hbr;
}


LONG ChSplitter::OnCtlColorStatic( WPARAM wParam, LPARAM lParam )
{
	CWnd*	pWnd = CWnd::FromHandle( (HWND)lParam );
	CDC*	pDC = CDC::FromHandle( (HDC)wParam );

	return (LONG)OnCtlColor( pDC, pWnd, CTLCOLOR_STATIC );
}

#if defined( CH_PUEBLO_PLUGIN )
BOOL ChSplitter::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return true;
}
#endif



/*----------------------------------------------------------------------------
	ChSplitterBanner constants
----------------------------------------------------------------------------*/

//#define WM_SPLITTER_BANNER_RECALC		(WM_USER + 1)


/*----------------------------------------------------------------------------
	ChSplitter static values
----------------------------------------------------------------------------*/

ChString		ChSplitterBanner::m_strClass;


/*----------------------------------------------------------------------------
	ChSplitterBanner class
----------------------------------------------------------------------------*/

ChSplitterBanner::~ChSplitterBanner()
{				
	OSType				osType = ChCore::GetClientInfo()->GetPlatform();
	if ( osWin32s == osType || osWin16 == osType )
	{
		::DeleteObject( m_hbrBackColor );
	}

	m_hbrBackColor = 0;
}


BOOL ChSplitterBanner::PreTranslateMessage( MSG* pMsg )
{
	ASSERT( m_hWnd != NULL );
											/* Don't translate dialog messages
												when in Shift+F1 help mode */
	CFrameWnd*	pFrameWnd = GetTopLevelFrame();

	if ((pFrameWnd != 0) && (pFrameWnd->m_bHelpMode))
	{
		return false;
	}
											/* Since 'IsDialogMessage' will eat
												frame window accelerators, we
												call all frame windows'
												PreTranslateMessage first */

	CWnd*	pOwner = GetOwner();			// Always use owner first

	while (pOwner != 0)
	{										/* Allow owner & frames to translate
												before IsDialogMessage does */
		if (pOwner->PreTranslateMessage( pMsg ))
		{
			return true;
		}
											/* Try parent frames until there are
												no parent frames */
		pOwner = pOwner->GetParentFrame();
	}
											/* Filter both messages to dialog and
												from children */
	return PreTranslateInput( pMsg );
}


BEGIN_MESSAGE_MAP( ChSplitterBanner, CWnd )
	//{{AFX_MSG_MAP(ChSplitterBanner)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CTLCOLORDLG, OnCtlColorStatic )
//	ON_MESSAGE( WM_SPLITTER_BANNER_RECALC, OnSplitterBannerRecalc )
END_MESSAGE_MAP()


void ChSplitterBanner::GetSize( CSize& size ) const
{
	size.cx = m_sClientWidth + m_rtBorders.left + m_rtBorders.right +
				(GetEdgeHeight() * 2);
	size.cy = m_sClientHeight + m_rtBorders.top + m_rtBorders.bottom +
				(GetEdgeHeight() * 2);
}


void ChSplitterBanner::GetDefaultBorders( CRect& rtBorders ) const
{
	int		iWidthMetric;
	int		iHeightMetric;

	if (IsWindows95())
	{
		iWidthMetric = SM_CXFIXEDFRAME;
		iHeightMetric = SM_CYFIXEDFRAME;
	}
	else
	{
		iWidthMetric = SM_CXDLGFRAME;
		iHeightMetric = SM_CYDLGFRAME;
	}

	rtBorders.left = rtBorders.right =
		GetSystemMetrics( iWidthMetric ) * 2;
	rtBorders.top = rtBorders.bottom =
		(int)((double)GetSystemMetrics( iHeightMetric ) * 1.5);
}


void ChSplitterBanner::SetChild( ChWnd* pChild )
{
	ChRect		rtWindow;

	m_pChild = pChild;

	GetClientRect( rtWindow );
	SizeChild( rtWindow.Width(), rtWindow.Height() );
}


void ChSplitterBanner::SetChildSize( chint16 sWidth, chint16 sHeight )
{
											/* The width doesn't matter
												right now */
	m_sClientWidth = sWidth;

	if (m_sClientHeight != sHeight)
	{
		m_sClientHeight = sHeight;
		GetSplitter()->RecalcLayout();
	}
}


/*----------------------------------------------------------------------------
	ChSplitterBanner protected methods
----------------------------------------------------------------------------*/

ChSplitterBanner::ChSplitterBanner( ChSplitter* pSplitter, ChPane* pPane,
									const ChString& strName, bool boolTop,
									chint16 sClientWidth,
									chint16 sClientHeight ) :
						m_pSplitter( pSplitter ),
						m_pPane( pPane ),
						m_boolTop( boolTop ),
						m_sClientWidth( sClientWidth ),
						m_sClientHeight( sClientHeight ),
						m_pChild( 0 ),
						m_boolHidden( true ),
						m_hbrBackColor( 0 )
{
	const ChClientInfo*	pClientInfo = ChCore::GetClientInfo();
	OSType				osType = pClientInfo->GetPlatform();
	ChVersion			osVersion = pClientInfo->GetPlatformVersion();
	ChVersion			ntDrawEdgeVer( 3, 51 );
	CRect				rtPanes;
	CRect				rtWindow( 0, 0, 100, 100 );

											/* Start by using the client rect
												of the parent, since panes are
												all horizontally stretch-to-fit
												right now */
	pSplitter->GetClientRect( rtPanes );
	rtWindow.right = rtPanes.Width();
											/* Figure out if we can call the
												'DrawEdge' function */

	m_boolWindows95 = ((osWin95 == osType) || (osWin98 == osType) || (osWinXP == osType));
	m_boolDrawEdgeAvailable = IsWindows95() ||
								((osWinNT == osType) &&
									(osVersion >= ntDrawEdgeVer));

											/* Calculate the amount of space
												necessary to draw the 3d edge
												of this pane */

	m_sEdgeHeight = GetSystemMetrics( SM_CYBORDER ) * 2;

											/* Get the borders for all sides
												of the child window */
	GetDefaultBorders( m_rtBorders );
											// Define the brush
	if (IsWindows95())
	{
		m_hbrBackColor = (HBRUSH)(COLOR_3DFACE + 1);
	}
	else if ( osWinNT == osType )
	{
		m_hbrBackColor = (HBRUSH)(COLOR_BTNFACE + 1);
	}
	else
	{
		m_hbrBackColor = ::CreateSolidBrush( ::GetSysColor( COLOR_BTNFACE ) );
	}
											/* Make sure we have a class
												registered */
	if (0 == m_strClass.GetLength())
	{
		m_strClass = AfxRegisterWndClass( CS_DBLCLKS,
											LoadCursor( 0, IDC_ARROW ),
											m_hbrBackColor );
	}
											// Create the frame

	Create( m_strClass, strName, WS_CHILD, rtWindow, pSplitter, 0 );
}


void ChSplitterBanner::SizeChild( chint32 lWidth, chint32 lHeight )
{
	ChWnd*	pChild;

	if (pChild = GetChild())
	{										/* Set the size of the child
												window to fill this window */
		CRect	rtBorders = GetBorders();

		pChild->SetWindowPos( 0, rtBorders.left, rtBorders.top,
								lWidth - (rtBorders.left + rtBorders.right),
								lHeight - (rtBorders.top + rtBorders.bottom),
								SWP_NOACTIVATE | SWP_NOZORDER );
	}
	else
	{
		Invalidate( false );
	}
}


/*----------------------------------------------------------------------------
	ChSplitterBanner message handlers
----------------------------------------------------------------------------*/

void ChSplitterBanner::OnShowWindow( BOOL boolShow, UINT nStatus )
{
	m_boolHidden = !boolShow;
	GetSplitter()->RecalcLayout();

	CWnd::OnShowWindow( boolShow, nStatus );

//	PostMessage( WM_SPLITTER_BANNER_RECALC );
}


void ChSplitterBanner::OnSize( UINT nType, int cx, int cy )
{
	CWnd::OnSize( nType, cx, cy );

	SizeChild( cx, cy );
}


#if 0
LONG ChSplitterBanner::OnSplitterBannerRecalc( UINT wParam, LONG lParam )
{
	GetSplitter()->RecalcLayout();
	return 0;
}
#endif


HBRUSH ChSplitterBanner::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH	hbr = CWnd::OnCtlColor( pDC, pWnd, nCtlColor );

	if (CTLCOLOR_STATIC == nCtlColor)
	{
		if (m_hbrBackColor)
		{
			hbr = m_hbrBackColor;
		}
	}

	return hbr;
}


LONG ChSplitterBanner::OnCtlColorStatic( WPARAM wParam, LPARAM lParam )
{
	CWnd*	pWnd = CWnd::FromHandle( (HWND)lParam );
	CDC*	pDC = CDC::FromHandle( (HDC)wParam );

	return (LONG)OnCtlColor( pDC, pWnd, CTLCOLOR_STATIC );
}


void ChSplitterBanner::OnNcCalcSize( BOOL boolCalcValidRects,
										NCCALCSIZE_PARAMS FAR* lpncsp )
{
	::InflateRect( &(lpncsp->rgrc[0]), -m_sEdgeHeight, -m_sEdgeHeight );

//	CWnd::OnNcCalcSize( boolCalcValidRects, lpncsp );
}


void ChSplitterBanner::OnNcPaint()
{
	CWindowDC	dc( this );					// Device context for painting
	HDC			hDC = dc.GetSafeHdc();
	CRect		rtFrame;

	GetWindowRect( rtFrame );
	rtFrame.OffsetRect( -rtFrame.left, -rtFrame.top );

	if (m_boolDrawEdgeAvailable)
	{
		UINT	uiBorderType;

		if (IsTop())
		{
			uiBorderType = BF_BOTTOM;
		}
		else
		{
			uiBorderType = BF_TOP;
		}

		DrawEdge( hDC, &rtFrame, EDGE_RAISED, BF_RECT );
	}
	else
	{
		int		iY;
		HGDIOBJ	hOldPen = ::SelectObject( hDC, GetStockObject( BLACK_PEN ) );

		if (IsTop())
		{
			iY = rtFrame.bottom;
		}
		else
		{
			iY = rtFrame.top;
		}
											/* We probably should draw a nice
												3d border here */
		dc.MoveTo( rtFrame.left, iY );
		dc.LineTo( rtFrame.right, iY );
		::SelectObject( hDC, hOldPen );
	}
}


void ChSplitterBanner::OnDestroy() 
{
	ChPane*		pPane = GetPane();

	if (pPane)
	{
		ChRect		rtFrame;

		GetWindowRect( &rtFrame );
		pPane->OnFrameDestroy( rtFrame );
	}

	ChWnd::OnDestroy();
}


void ChSplitterBanner::OnActivate( UINT nState, CWnd* pWndOther,
									BOOL boolMinimized )
{
	CWnd::OnActivate( nState, pWndOther, boolMinimized );

	if ((WA_INACTIVE != nState) && GetChild())
	{
		GetChild()->SetFocus();
	}
}


#endif	// defined( CH_MSW )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:54:46  uecasm
// Import of source tree as at version 2.53 release.
//
