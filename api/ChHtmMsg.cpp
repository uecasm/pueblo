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

	This file consists of the implementation of the ChHtmlView class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ctype.h>

#ifdef CH_UNIX
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#include <ChTypes.h>
#include <ChRect.h>
#include <ChSize.h>
#include <ChScrlVw.h>
#else
	#if !defined(CH_PUEBLO_PLUGIN)
	#include "resource.h"
	#else
	#include "vwrres.h"
	#endif
#endif
#include <ChHtmWnd.h>

#include "ChHtmlView.h"
#include <ChConst.h>
#include <ChReg.h>
#include <ChUtil.h>

#include <ChHtpCon.h>

#include "ChPlgInMgr.h"
#include <ChHtmlSettings.h>

#include "ChHtmSym.h"
#include "ChHtmlParser.h"

#include "ChHTMLStream.h"


#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>

////////////////////////////////////////////////////////////////////////////////////


// HTML Wnd native message handlers

#ifdef CH_MSW
BEGIN_MESSAGE_MAP(ChHtmlView, ChTxtWnd)
	//{{AFX_MSG_MAP(ChHtmlView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
//	ON_MESSAGE( WM_CHACO_HTTP_MSG, OnHTTPNotificaton )
END_MESSAGE_MAP()
#endif 




#ifdef CH_UNIX
void ChHtmlView::OnLButtonUp( chuint32 nFlags, ChPoint& point )
#else
void ChHtmlView::OnLButtonUp( UINT nFlags, ChPoint point )
#endif
{
	chparam		userData;

	#if defined( CH_VERBOSE )
	{
		cerr << "XXX ChHtmlView::OnLButtonUp" << endl;
	}
	#endif	// defined( CH_VERBOSE )
	ChPoint ptRel;

	chint32 lStart = 0, lEnd = 0;
	if ( IsSelectionEnabled() )
	{
		GetSel( lStart, lEnd );
	}

	ChTxtWnd::OnLButtonUp( nFlags, point );

	// UE: moved below call to base (otherwise things screw up if the hotspot opens
	//     another window, especially a dialog box)
	if ( ( (lEnd - lStart) <= 1 ) && 
			ChTxtWnd::PointOn( point.x, point.y, userData, &ptRel ) == ChTxtWnd::locHotspot )
	{
		GetFrameMgr()->OnViewSelectHotSpot( this, point.x, point.y, ptRel, userData );
	}
}

#if defined( CH_MSW )

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::OnMouseMove

------------------------------------------------------------------------------
OnMouseMove calls OnHotSpot if the mouse enters a new hotspot. This is called only
once. When the mouse moves out of the hotspot, OnHotSpot is called with 0 userData,

----------------------------------------------------------------------------*/

void ChHtmlView::OnMouseMove(UINT nFlags, CPoint point)
{
	chparam		userData;
	static		chparam lastHotSpot = 0;
	int 		iMouseOn = ChTxtWnd::locUnknown;

	if ( !( nFlags & MK_LBUTTON ) && 
			(iMouseOn = ChTxtWnd::PointOn( point.x, point.y, userData )) == ChTxtWnd::locHotspot )
	{
		ChangeCursor(cursorHotspot);		// UE
		if ( lastHotSpot != userData )
		{
			lastHotSpot = userData;
			GetFrameMgr()->OnViewHotSpot( this, userData );
		}
	}
	else
	{
		if ( lastHotSpot )
		{
			lastHotSpot = 0;
			GetFrameMgr()->OnViewHotSpot( this, lastHotSpot );
		}
		if ( !IsSelectionEnabled() || ChTxtWnd::locUnknown == iMouseOn 
					|| ChTxtWnd::locObject == iMouseOn )
		{
			ChangeCursor(cursorNormal);		// UE
		}
		else if ( iMouseOn == ChTxtWnd::locText )
		{
			ChangeCursor(cursorText);		// UE
		}
		else
		{
			m_hCursor = NULL;
		}
	}

	ChTxtWnd::OnMouseMove( nFlags, point );
}

BOOL ChHtmlView::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	if (m_hCursor)		// UE
	{
		::SetCursor( m_hCursor );
		return true;
	}
	else
	{
		return ChTxtWnd::OnSetCursor( pWnd, nHitTest, message );
	}
}



void ChHtmlView::OnSetFocus(CWnd* pOldWnd) 
{
	ChTxtWnd::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
	// Notify my parent of focus change
	if ( m_iEmbedMode == embedInternal )
	{
		GetFrameMgr()->SendMessage( WM_SETFOCUS, 
						(WPARAM)(pOldWnd ? pOldWnd->GetSafeHwnd() : 0) );
	}
	
	
}


void ChHtmlView::OnKillFocus(CWnd* pNewWnd) 
{
	ChTxtWnd::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	// Notify my parent of focus change
	GetFrameMgr()->SendMessage( WM_KILLFOCUS, 
					(WPARAM)(pNewWnd ? pNewWnd->GetSafeHwnd() : 0) );
}



void ChHtmlView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	TRACE("ChHtmlView::OnRButtonDown(%08Xh, %d, {%d, %d}) -- (%08Xh, \"%s\")\n",
						this, nFlags, point.x, point.y, GetFrameMgr(), (LPCSTR)GetFrameName());	// UE DEBUG
	GetFrameMgr()->OnRightMouseDown( point, GetFrameName() );

	ChTxtWnd::OnRButtonDown(nFlags, point);
}

void ChHtmlView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	GetFrameMgr()->OnRightMouseUp( point, GetFrameName() );
	
	ChTxtWnd::OnRButtonUp(nFlags, point);
}


#endif	// defined( CH_MSW )





/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when the
	HTML parser detects a prefetch tag in the document.
----------------------------------------------------------------------------*/

void ChHtmlView::OnPrefetch( ChHTMLPrefetch* pFetch, const ChString& strDocURL )
{
											/* Call base class to perform the
												prefetching */

	if (ChUtil::GetSystemProperties() & CH_PROP_MULTITHREADED)
	{
		GetFrameMgr()->PrefetchURL( pFetch->GetHREF(),  strDocURL );
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::UpdateTextColor()

------------------------------------------------------------------------------

	This method computes the location of the object on the canvas.

----------------------------------------------------------------------------*/

void ChHtmlView::UpdateTextColor( chuint32 luOldTextColor,
								 chuint32 luOldLinkColor,
								 chuint32 luOldVLinkColor,
								 chuint32 luOldPrefetchColor  )
{
		


	// Get the new colors
	chuint32 luTextColor = GetSettings()->GetTextColor();
	chuint32 luLinkColor = GetSettings()->GetLinkColor();
	chuint32 luVLinkColor = GetSettings()->GetVistedLinkColor();
	chuint32 luPrefetchColor = GetSettings()->GetPrefetchedLinkColor();
	chuint32 luBackColor = GetSettings()->GetBackColor();	


	// find the first run using this object
	pChStyleInfo pStyleUse 	= GetStyleTable();
 	pChRun 		 pRun 		= GetRunTable();

	// do we have any runs with hot spot in this line
	chint32 lIndex = 0;

	// skip all the object at the begining of the line
	while( lIndex < GetRunCount() ) 
	{
		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textHotSpot )
		{
			if ( luOldLinkColor == pStyleUse[ pRun->lStyleIndex ].style.lColor )
			{
				pStyleUse[ pRun->lStyleIndex ].style.lColor = luLinkColor;
			}
			else if ( luOldVLinkColor == pStyleUse[ pRun->lStyleIndex ].style.lColor )
			{
				pStyleUse[ pRun->lStyleIndex ].style.lColor = luVLinkColor;
			}
			else if ( luOldPrefetchColor == pStyleUse[ pRun->lStyleIndex ].style.lColor )
			{
				pStyleUse[ pRun->lStyleIndex ].style.lColor = luPrefetchColor;
			}
		}
		else if ( luOldTextColor == pStyleUse[ pRun->lStyleIndex ].style.lColor )
		{
			pStyleUse[ pRun->lStyleIndex ].style.lColor = luTextColor;
		}

		++pRun;
		++lIndex;
	}
}

void ChHtmlView::UpdateFontChange( const ChString& strOldProportional, int iOldProportionalSize,
						const ChString& strOldFixed, int iOldFixedSize )
{
	ChString		strNewFixed =  GetSettings()->GetFixedFontName();
	ChString		strNewProportional = GetSettings()->GetProportionalFontName();
	int			iNewFixedSize = GetSettings()->GetFixedFontSize();
	int			iNewProportionalSize = GetSettings()->GetProportionalFontSize();

	bool		boolUpdateFixed = false;
	bool		boolUpdateProportional = false;

	if ( iOldFixedSize != iNewFixedSize 
		 ||	strOldFixed !=  strNewFixed )
	{
	 	boolUpdateFixed = true;
	}
	if ( iOldProportionalSize != iNewProportionalSize 
		 || strOldProportional != strNewProportional )
	{
	 	boolUpdateProportional = true;
	}

	if ( !boolUpdateProportional && !boolUpdateFixed )  
	{
		return;
	}


											// New font name and size



	ChClientDC	dc( this );

	SetContext( &dc );

	#if defined( CH_MSW )

	pChFontInfo	pFontTbl	= ChTxtWnd::GetFontTable();

	#endif	// defined( CH_MSW )
											/* Remap all the font names to new
												font selected */


	for (int i = 1; i < ChTxtWnd::GetFontCount(); i++)
	{
		#if defined( CH_MSW )
		{

			bool		boolUpdate = false;
		
			if ( pFontTbl[i].iUseCount <= 0 || pFontTbl[i].pFont == 0 )
			{
				continue;
			}

			if (boolUpdateProportional &&
					!lstrcmpi( pFontTbl[i].fontInfo.lfFaceName, strOldProportional ))
			{
				int		iSizeExtra;

				lstrcpy( pFontTbl[i].fontInfo.lfFaceName, strNewProportional );

				iSizeExtra = (-1 * pFontTbl[i].fontInfo.lfHeight) -
								iOldProportionalSize;
				pFontTbl[i].fontInfo.lfHeight = -1 * (iNewProportionalSize +
														iSizeExtra);
				boolUpdate = true;
			}
			else if (boolUpdateFixed &&
						!lstrcmpi( pFontTbl[i].fontInfo.lfFaceName, strOldFixed ))
			{
				int		iSizeExtra;

				lstrcpy( pFontTbl[i].fontInfo.lfFaceName, strNewFixed );
				iSizeExtra = (-1 * pFontTbl[i].fontInfo.lfHeight) -
								iOldFixedSize;
				pFontTbl[i].fontInfo.lfHeight = -1 * (iNewFixedSize + iSizeExtra);
				boolUpdate = true;
			}

			if (boolUpdate)
			{
				CFont*		pOldFont;
				TEXTMETRIC	textMetric;
											// Delete the old font
				delete pFontTbl[i].pFont;

				pFontTbl[i].pFont = new ChFont;

				pFontTbl[i].pFont->CreateFontIndirect( &pFontTbl[i].fontInfo );

				pOldFont = GetContext()->SelectObject( pFontTbl[i].pFont );

				GetContext()->GetTextMetrics( &textMetric );
				GetContext()->SelectObject( pOldFont );

				pChStyleInfo pStyle = GetStyleTable();

				for ( int j = 0; j < GetStyleCount(); j++ )
				{
					if ( pStyle[j].style.iFontIndex == i ) 
					{
						pStyle[j].iFontHeight = textMetric.tmHeight +
												textMetric.tmExternalLeading;
						pStyle[j].iFontAscent = textMetric.tmAscent;
						pStyle[j].iFontDescent = textMetric.tmDescent;
					}
				}
			}
		}
		#else	// defined( CH_MSW )
		{
		}
		#endif	// defined( CH_MSW )
	}

	if ( boolUpdateProportional || boolUpdateFixed )
	{
											/* Recompute the line table and
												redraw */
		ChTxtWnd::UpdateLineTable( 0, ChTxtWnd::GetTextCount(), vwUpdate );
		InvalidateRect( 0 );
	}
}
void ChHtmlView::UpdateColorChange( chuint32   luOldTextColor, chuint32 luOldLinkColor, 
						chuint32 luOldVLinkColor, chuint32 luOldPrefetchColor, 
						chuint32 luBackColor )
{


	if (ChTxtWnd::m_defBkColor.GetSafeHandle())
	{
	 	ChTxtWnd::m_defBkColor.DeleteObject();
	}
 
 	ChTxtWnd::m_defBkColor.CreateSolidBrush( GetSettings()->GetBackColor() );

											// Map all the new colors

	UpdateTextColor( luOldTextColor, luOldLinkColor, luOldVLinkColor, 
						luOldPrefetchColor );

	if ( m_pParser )
	{
		m_pParser->UpdateColors( luOldTextColor, luBackColor );
	}

	if ( GetSafeHwnd() )
	{
		// Update all the text colors
		InvalidateRect( NULL, true );
	}

}

void ChHtmlView::RemapColors( int iNumColors, 
						chuint32* pluOldColor, chuint32* pluNewMap )
{
	// find the first run using this object
	pChStyleInfo pStyleUse 	= GetStyleTable();
 	pChRun 		 pRun 		= GetRunTable();

	// do we have any runs with hot spot in this line
	chint32 lIndex = 0;

	// skip all the object at the begining of the line
	bool boolUpdate = false;
	while( lIndex < GetRunCount() ) 
	{
		for ( int i = 0; i < iNumColors; i++ )
		{
			if ( pluOldColor[i] == (chuint32)pStyleUse[ pRun->lStyleIndex ].style.lColor )
			{
				pStyleUse[ pRun->lStyleIndex ].style.lColor = pluNewMap[i];
				boolUpdate = true;
				break;
			}
		}

		++pRun;
		++lIndex;
	}

	if ( boolUpdate && GetSafeHwnd() )
	{
		// Update all the text colors
		InvalidateRect( NULL, true );
	}

}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:53  uecasm
// Import of source tree as at version 2.53 release.
//
