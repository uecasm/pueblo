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

	This file consists of the implementation of the ChTxtWnd class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#ifdef CH_UNIX
#include <ChTypes.h>
#include <ChRect.h>
#include <ChSize.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#endif // CH_UNIX

#include <ChImgUtil.h>
#include <ChArgList.h>
#include <ChTxtWnd.h>
#include "ChPlgInMgr.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include "resource.h"
#include <MemDebug.h>



/*----------------------------------------------------------------------------
	ChTxtWnd public methods
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::AppendObject

------------------------------------------------------------------------------

	This method appends a control/graphical object to the text view.

----------------------------------------------------------------------------*/

bool ChTxtWnd::AppendObject( ChTxtObject *pTxtObj )
{

	chint32		lStartRun;
	chint32		lStartChar;
	bool		boolOK = false;

	//special case space objects
	// if the last char is a space object then update the last object 
	// with the new values instead of adding a new object

	if ( GetTextCount() && 
			pTxtObj->GetTextObject()->GetType() == ChTextObject::objectSpace )
	{
		// is the last object a space object

		pChStyleInfo 	pStyleUse 	= GetStyleTable();
	 	pChRun 			pRun 		= GetRunTable();

		pRun += GetRunIndex(  GetTextCount() - 1 );	  

		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
		{


			ChTextObject* pSpaceObj = pTxtObj->GetTextObject();

			ChTextObject** ppObject = &(GetObjectTable()[pStyleUse
						[ pRun->lStyleIndex ].style.iObjectIndex]);
			if ( (*ppObject)->GetType() == ChTextObject::objectSpace &&
						(*ppObject)->GetAttrs() == pSpaceObj->GetAttrs() )
			{
	
				// Get the old object info
				ChSize sizeObj;
				ChRect spaceExtra;
				(*ppObject)->GetSize( sizeObj );
				(*ppObject)->GetSpaceExtra( spaceExtra );

				// Get the new object info
				ChSize sizeObjNew;
				ChRect spaceExtraNew;
				pSpaceObj->GetSize( sizeObjNew );
				pSpaceObj->GetSpaceExtra( spaceExtraNew );

				// Update the old object
				sizeObjNew += sizeObj;
				spaceExtraNew.left += spaceExtra.left;
				spaceExtraNew.top += spaceExtra.top;
				spaceExtraNew.right += spaceExtra.right;
				spaceExtraNew.bottom += spaceExtra.bottom;
				(*ppObject)->SetSizeInfo( sizeObjNew, spaceExtraNew );

				delete pSpaceObj;
				return true;
			}
		}

	}


	lStartChar = AppendTextBuffer( "\bObj\b",  GetTextCount(), 5 );
	{
		ChClientDC	dc( this );

		m_pDC = &dc;
		// AppendTextBuffer enforces buffer limit which can change the counts, 
		// update starts 
		lStartRun  = GetRunCount() - 1;
		{
			ChStyle style;
			// Get the style information
			style.iFontIndex = GetFont( pTxtObj->GetFont() );
			style.lColor 	= pTxtObj->GetTextColor();
			style.lBackColor = pTxtObj->GetBackColor();
			//style.iAddWidth = 0;
			style.luStyle   = pTxtObj->GetStyle();

			if ( style.luStyle & textAlwaysUpdate )
			{
				m_boolRedrawAll = true;
			}

			style.iLeftIndent  = pTxtObj->GetLeftIndent( );
			style.userData    = pTxtObj->GetUserData();
			style.iObjectIndex = NewTextObject( pTxtObj );

			if (ChangeStyle( &style, lStartRun, lStartChar, 
									(GetTextCount() - lStartChar) ))
			{
				boolOK = true;
			}
		}

	}

	if ( pTxtObj->GetTextObject()->GetType() == ChTextObject::objectImage )
	{
		ChObjInline* pInline = (ChObjInline*)pTxtObj->GetTextObject();
		ComputeTimerInterval( pInline );
	}

	return boolOK;
}

// This function adjusts the timeout of the animation timer
// to fit nicely with all loaded animations.
void ChTxtWnd::ComputeTimerInterval( ChObjInline *pInline )
{
	if (!pInline->GetImageData()) { return; }

	ChDib *pDib;
	if (!(pDib = pInline->GetImageData()->GetImage())) { return; }

	ComputeTimerInterval(pDib);
}

void ChTxtWnd::ComputeTimerInterval(ChDib *pDib)
{
	if (pDib->GetImageInfo()->boolMultiframe)
	{ 	// Set the timer if not present
		
		// UE: we don't have an interface to determine the frame times
		//     for MNG animations; the animation is considerably more
		//     complex than GIF-style animation and we have no way of
		//     determining a concrete frame count (since it may loop
		//     forever).
	

		//UINT uExposture = pDib->GetFrameInfo( 0 )->iExposture;
		//TRACE1("ComputeTimerInterval: initial exposture == %d\n", uExposture);

		//for( int i = 1; i < pDib->GetTotalFrames(); ++i )
		//{
		//	if ( uExposture > (UINT)pDib->GetFrameInfo( i )->iExposture )
		//	{
		//		uExposture = pDib->GetFrameInfo( i )->iExposture;
		//	}
		//}
		//TRACE1("ComputeTimerInterval: final exposture == %d\n", uExposture);

		//if ( uExposture == 0 )
		//{
		//	uExposture = 100;
		//}

		// UE: actually, can't really be stuffed trying to be efficient here, since methinks it
		//     could in fact lead to incorrectly long frame exposition times.  So we'll just use
		//     a simple fixed interval instead.
		UINT uExposture = 100;		// only update at 0.1 second intervals

		if ( m_idTimer )
		{		 
			//TRACE1("ComputeTimerInterval: current interval == %d\n", m_uExpostureTime);
			if ( m_uExpostureTime > uExposture )
			{
			 	KillTimer( m_idTimer );
				m_idTimer = 0;
			}
		}

		if ( 0 == m_idTimer )
		{
			m_idTimer = 10;
			m_uExpostureTime = uExposture; 
			SetTimer( m_idTimer, uExposture, NULL );
			//TRACE1("ComputeTimerInterval: Starting timer at interval == %d\n", uExposture);
		}
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeObjectInfo()

------------------------------------------------------------------------------

	This method computes the location of the object on the canvas.

----------------------------------------------------------------------------*/

void ChTxtWnd::UpdateObject( ChTextObject* pObject, bool boolSizeChanged /* = false */ )
{
	int iID = pObject->GetObjectID();
	//sanity check
	if ( iID == -1 || iID >= GetObjectCount() )
	{
		return;
	}
	ChObjInline* pInline = (ChObjInline*)pObject;
	ComputeTimerInterval( pInline );

	// find the first run using this object
	pChStyleInfo pStyleUse 	= GetStyleTable();
 	pChRun 		 pRun 		= GetRunTable();

	// do we have any runs with hot spot in this line
	bool boolUpdateAll = false;
	chint32 lIndex = 0;

	ChSize size;
	GetTotalCanvasSize( size );

	chint32 lUpdateHeight = size.cy;

	ChPoint ptPos = GetDeviceScrollPosition();

	bool  	boolDisplayAppend = (lUpdateHeight <= ( ptPos.y + GetViewHeight()));


	// skip all the object at the begining of the line
	while( lIndex < GetRunCount() ) 
	{
		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject
				&& pStyleUse[ pRun->lStyleIndex ].style.iObjectIndex == iID )
		{
			ChRect		rtUpdateRect( 0, 0, 
				GetViewWidth(), GetViewHeight() );;
 
 			ptPos = GetDeviceScrollPosition();

			if ( boolSizeChanged )
			{
				ChClientDC	dc( this );
				m_pDC = &dc;

				bool boolChanged;
				UpdateLineTable( pRun->lStartChar, GetTextCount() - pRun->lStartChar, 
										vwUpdate, &boolChanged );
				rtUpdateRect = GetUpdateRect();

				rtUpdateRect.left -=  ptPos.x;
				rtUpdateRect.top -=  ptPos.y;
				rtUpdateRect.right -=  ptPos.x;
				rtUpdateRect.bottom -=  ptPos.y;

				for ( int i = GetObjectCount() - 1; i >= 0; i-- )
				{
					ChTextObject* pObject = GetObjectTable()[i];
					ChSize viewSize;
					if ( ChTextObject::objectControl == pObject->GetType() )
					{
						((ChObjControl*)pObject)->DrawObject( this, -1000, -1000, 0, viewSize );
						boolUpdateAll = true;
					}
				}


				
				if ( boolDisplayAppend && AlwaysDisplayAppend() )
				{
					ChPoint 		ptPos;

					ptPos  = GetDeviceScrollPosition();
					// check if the last line appended is visible, if not scroll the view to update
					if ( m_sizeTotal.cy > (ptPos.y +  GetViewHeight()) )
					{ // The last line is not visible, scroll enough to make it visible
						rtUpdateRect.left = 0;
						rtUpdateRect.right = GetViewWidth();

						ptPos.y = m_sizeTotal.cy;
						ScrollToPosition( ptPos );  // scroll to view
						// updated x,y position
						rtUpdateRect.top =  (int)(GetViewHeight() -
												(m_sizeTotal.cy - rtUpdateRect.top ));
						rtUpdateRect.bottom = GetViewHeight();
					}
					else
					{	// compute the rect to update
						ptPos = GetDeviceScrollPosition();
						rtUpdateRect.top = (int)rtUpdateRect.top  > ptPos.y
														? (int)(rtUpdateRect.top  - ptPos.y) : ptPos.y;
					}
				}
			}
			else
			{
			
				chint32 lUpdateHeight;

				GetLineIndex( pRun->lStartChar, lUpdateHeight );
				rtUpdateRect.top = (int)lUpdateHeight > ptPos.y 
											? (int)(lUpdateHeight - ptPos.y) : ptPos.y;
			}


			if ( rtUpdateRect.top < rtUpdateRect.bottom || boolUpdateAll )
			{
				if ( boolUpdateAll )
				{
					InvalidateRect( 0 );
				}
				else
				{
					InvalidateRect( rtUpdateRect, boolSizeChanged );
				}
			}
			break;
		}
		++pRun;
		++lIndex;
	}


}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeObjectInfo()

------------------------------------------------------------------------------

	This method computes the location of the object on the canvas.

----------------------------------------------------------------------------*/
ChSize ChTxtWnd::ComputeObjectInfo( pLineData pCalc  )
{
	pChStyleInfo	pStyleUse;
	pChRun			pRun;
	ChSize 			size;

	pRun = pCalc->pRun + pCalc->iWordRunCount;
	pStyleUse = &pCalc->pStyleUse[ pRun->lStyleIndex ];

	ASSERT( pStyleUse->style.luStyle & ChTxtWnd::textObject );

	pCalc->boolObject = true;
	pCalc->boolObjectFloat = false;
	pCalc->boolStartNewLine = false;
	pCalc->luLineAttr = 0;;


	if( pStyleUse->style.luStyle & ChTxtWnd::textObject 
						&& pRun->lStartChar < GetTextCount() )
	{

	   	size.cx = pCalc->iMaxLineWidth;
		size.cy = 0;

		GetObjectSizeAndAttr(pStyleUse->style , size, pCalc->luLineAttr );
		// update the location info for the object
		//pChObject pObject = GetObjectTable();

		//pObject += pStyleUse->style.iObjectIndex;

		if ( pCalc->luLineAttr & ChTxtWnd::objAttrLeft  
						 || pCalc->luLineAttr & ChTxtWnd::objAttrRight 
						 || pCalc->luLineAttr & ChTxtWnd::objAttrBreak )
		{
			pCalc->boolStartNewLine = true;
		}
		
		if ( pCalc->luLineAttr & ChTxtWnd::objAttrLeft  
						 || pCalc->luLineAttr & ChTxtWnd::objAttrRight )
		{
			pCalc->boolObjectFloat = true;
		}

		if (  pCalc->luLineAttr & ChTxtWnd::objAttrClearLeft )
		{  
			if ( pCalc->iCurrLayoutIndex )
			{
				int i = pCalc->iCurrLayoutIndex;
				while( i > 0 && pCalc->pLayoutInfo[i].iCurrX != pCalc->pLayoutInfo[0].iCurrX )
				{
					i--;	
				}

				if ( pCalc->iCurrY < 
							pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iBreakY )
				{
					
					pCalc->iCurrY = pCalc->pLayoutInfo[i + 1].iBreakY;
				}
				pCalc->iCurrLayoutIndex = i;   
			}


			pCalc->boolStartNewLine = true;
		}

		if (  pCalc->luLineAttr & ChTxtWnd::objAttrClearRight )
		{
			if ( pCalc->iCurrLayoutIndex )
			{
				int i = 1;
				while( i <= pCalc->iCurrLayoutIndex )
				{
					if ( pCalc->pLayoutInfo[i].iCurrX == 
							pCalc->pLayoutInfo[ i - 1].iCurrX )
					{
					 	break;
					}
					i++;
				}

				if ( i <= pCalc->iCurrLayoutIndex  )
				{
					
					pCalc->iCurrY = pCalc->pLayoutInfo[ i ].iBreakY;
				}
				pCalc->iCurrLayoutIndex = i - 1; 
				  
			}
			pCalc->boolStartNewLine = true;
		}

	}
	return size;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetObjectWidth

------------------------------------------------------------------------------


----------------------------------------------------------------------------*/

void ChTxtWnd::GetObjectSizeAndAttr( ChStyle& style, ChSize& size, chuint32& luAttr )
{

	ChTextObject** ppObject = GetObjectTable();

	ppObject += style.iObjectIndex;

  	// size contains the current max line width and height is set to 0
	(*ppObject)->GetObjectSize( size );

	// get the attriubutes
	luAttr = (*ppObject)->GetAttrs(); 

	return;
}


bool ChTxtWnd::PointInObject( int iObjIndex, ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel /*= 0 */ )
{

	ChTextObject** ppObject = GetObjectTable();

	ppObject += iObjIndex;

  	//  Is the point in rect
  	return((*ppObject)->PointInObject( ptTopLeft, ptLoc, pptRel ));

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::DrawObject

------------------------------------------------------------------------------

	This method sets up an object to be drawn.

----------------------------------------------------------------------------*/
ChSize ChTxtWnd::DrawObject( int x, int y, ChStyle& style, ChSize& size )
{
	ChTextObject** ppObject = GetObjectTable();

	ppObject += style.iObjectIndex;

	ChSize sizeObj = (*ppObject)->DrawObject( this, x, y, style.luStyle, size );


	return sizeObj;
}


void ChTxtWnd::ClearForms( )
{
	// do object table cleanup
	for ( int i = 0; i < (int)GetObjectCount(); i++)
	{
		switch ( m_pObjTbl[i]->GetType() )
		{
			case ChTextObject::objectControl :
			{
				((ChObjControl*)m_pObjTbl[i])->RemoveControl();
				break;
			}
			default :
			{
				break;
			}
		}
	}
}


/*----------------------------------------------------------------------------
	ChTextObject class : This class provides the interface for specifying the
	object information.
----------------------------------------------------------------------------*/


ChTxtObject::ChTxtObject(  ChTextObject *pObj, ChFont *pFont /*= NULL */, 
						chuint32 luTextClr /* = CH_DEF_COLOR */,
						chuint32 luBackColor /* = CH_DEF_COLOR */,
						chuint32 luStyle /* = ChTxtWnd::textLeft */,
						int iLeftIndent /* = 0  */ ) :
						m_ptxtObject(pObj), m_pFont( pFont ), m_luBackClr( luBackColor ),
						m_iLeftIndent( iLeftIndent ), m_luStyle( luStyle ),
						m_userData( 0 )
{

	#ifdef CH_MSW
	m_luTextClr = ( CH_DEF_COLOR & luTextClr ) ?
					COLOR_DEF_TEXT /*::GetSysColor( COLOR_WINDOWTEXT)*/ :
					luTextClr;
	#else
	m_luTextClr = luTextClr;
	// We'll take care of the CH_DEF_COLOR stuff in
	// ChDC instead.
	#endif
}


/*----------------------------------------------------------------------------
	Implementation of ChInlineData object
----------------------------------------------------------------------------*/
ChInlineImageData::~ChInlineImageData()
{
	if ( m_pDIB )
	{
		delete m_pDIB;
	}

}
ChInlinePluginData::~ChInlinePluginData()
{

	if ( m_pluginInst )
	{
		delete m_pluginInst;
	}

	delete m_pArgList;
}



/*----------------------------------------------------------------------------
	Implementation of Space object
----------------------------------------------------------------------------*/


void ChObjSpace::GetObjectSize( ChSize& objSize ) 
{
	if ( m_luAttr & ChTxtWnd::objAttrBreak )
	{
		objSize.cy += ( m_sizeObject.cy + 
					m_spaceExtra.top + m_spaceExtra.bottom );
	}
	else
	{
		objSize = m_sizeObject;
		objSize.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
		objSize.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );
	}
}

ChSize ChObjSpace::DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize )
{   // nothing to draw, we may have to do some thing if we support background color in
	// future
	ChSize objSize;
	if ( m_luAttr & ChTxtWnd::objAttrBreak )
	{
		objSize.cx = viewSize.cx;
		objSize.cy = ( m_sizeObject.cy + 
					m_spaceExtra.top + m_spaceExtra.bottom );
	}
	else
	{
		objSize = m_sizeObject;
		objSize.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
		objSize.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );
	}
	return objSize;
}


	// rest implemented as inline methods

/*----------------------------------------------------------------------------
	Implementation of line object
----------------------------------------------------------------------------*/

// Get object is aclled with size set to the width of the view and
// height set to any extra height required by the caller

COLORREF	ChObjLine::m_clrBtnHilight= 0,
			ChObjLine::m_clrBtnShadow = 0; // color for line drawing


void ChObjLine::GetObjectSize( ChSize& size )
{
	// width is specified as percentage of the view width   
	chuint32  luWidth =  size.cx; 
	luWidth = (luWidth * m_sizeObject.cx/100);
	size.cx = (int)luWidth;
	// this cause truncation of data on 16 bit
	//size.cx = size.cx * pObject->sizeObject.cx/100;
	size.cy = m_sizeObject.cy;
	size.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
	size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );
}

ChSize ChObjLine::DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize )
{


    COLORREF 	cvSav;

	CDC* pDC = pWnd->GetContext();

	chuint32  luWidth =  viewSize.cx; 

	luWidth = (luWidth * m_sizeObject.cx/100);

    ChRect    	rtRect( x + m_spaceExtra.left, 
    					y + m_spaceExtra.top, 
    					x + (int)luWidth - m_spaceExtra.right, 
    					y + m_spaceExtra.top + m_sizeObject.cy );	  


	if ( GetAttrs() & ChTxtWnd::objAttrNoShade )
	{
		CPen pen;

		pen.CreatePen( PS_SOLID, m_sizeObject.cy, 
						COLOR_DEF_TEXT /*::GetSysColor( COLOR_WINDOWTEXT )*/ );
		CPen *pOld = pDC->SelectObject( &pen );

		pDC->MoveTo( rtRect.left, rtRect.top );
		pDC->LineTo( rtRect.right, rtRect.top );

		pDC->SelectObject( pOld );

	}
	else
	{
		ChRect      rtTemp = rtRect;


	    cvSav = pDC->SetBkColor( ( m_clrBtnShadow ) );

	    // top
	    rtRect.bottom = rtRect.top+1;
	    pDC->ExtTextOut( 0, 0, ETO_OPAQUE, rtRect, 
	            (char FAR *) NULL, 0, (int FAR *) NULL);

	    // left
	    rtRect.bottom = rtTemp.bottom;
	    rtRect.right = rtRect.left+1;
		pDC->ExtTextOut( 0, 0, ETO_OPAQUE, rtRect, 
	            (char FAR *) NULL, 0, (int FAR *) NULL);

	    pDC->SetBkColor( ( m_clrBtnHilight ) );

	    // right
	    rtRect.right = rtTemp.right;
	    rtRect.left = rtRect.right-1;
	    pDC->ExtTextOut( 0, 0, ETO_OPAQUE, rtRect, 
	            (char FAR *) NULL, 0, (int FAR *) NULL);

	    // bot
	    rtRect.left = rtTemp.left;
	    rtRect.top = rtRect.bottom-1;
	    pDC->ExtTextOut( 0, 0, ETO_OPAQUE, rtRect, 
	        (char FAR *) NULL, 0, (int FAR *) NULL);

	    pDC->SetBkColor( cvSav);
	}

	ChSize size;
		
	size.cx = (int)luWidth;
	// this cause truncation of data on 16 bit
	//size.cx = size.cx * pObject->sizeObject.cx/100;
	size.cy = m_sizeObject.cy;
	size.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
	size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );

	return size;
}


/*----------------------------------------------------------------------------
	Implementation of image object
----------------------------------------------------------------------------*/

ChObjInline::~ChObjInline() 
{
	delete m_pPlugin;
	m_pPlugin = 0;
}

void ChObjInline::ShutdownPlugin()
{
	delete m_pPlugin;
	m_pPlugin = 0;
}

void ChObjInline::CreateImagePlaceholder()
{
	CreatePlaceholder(IDI_IMAGE);
}

void ChObjInline::CreateBrokenImagePlaceholder()
{
	CreatePlaceholder(IDI_IMAGE_BROKEN);
}

void ChObjInline::CreatePlaceholder(int iIconId)
{
	// UE: create a placeholder image for this object, to be replaced later
	//     by the real thing.
	ChSize size;
	GetImageSize(size);
	if (size.cx == 0 || size.cy == 0) { return; }

	CDC *dcDesktop = CWnd::GetDesktopWindow()->GetDC();
	CDC dcMem;
	CBitmap bitmap;
	dcMem.CreateCompatibleDC(dcDesktop);
	bitmap.CreateCompatibleBitmap(dcDesktop, size.cx, size.cy);
	CWnd::GetDesktopWindow()->ReleaseDC(dcDesktop);
	CBitmap *oldBitmap = dcMem.SelectObject(&bitmap);

	// make a budget frame rectangle
	CRect imageRect(ChPoint(0, 0), size);
	dcMem.FillSolidRect(&imageRect, RGB(128,128,128));
	HICON iconImage = (HICON)::LoadImage(PuebloDLL.hModule, MAKEINTRESOURCE(iIconId), IMAGE_ICON, 16, 16, 0);
	ASSERT(iconImage);
	::DrawIconEx(dcMem, 3, 3, iconImage, 0, 0, 0, NULL, DI_NORMAL);
	::DestroyIcon(iconImage);
	dcMem.Draw3dRect(&imageRect, RGB(64,64,64), RGB(255,255,255));

	ChDib *image = new ChDib();
	image->Create(0, size.cx, size.cy, 24);

	::GetDIBits(dcMem, bitmap, 0, size.cy, image->GetBitsAddress(), image->GetBitmapInfoAddress(), DIB_RGB_COLORS);
	dcMem.SelectObject(oldBitmap);
	bitmap.DeleteObject();
	dcMem.DeleteDC();

	if (GetImageData() == NULL)
	{
		SetImageData(new ChInlineImageData);
	}
	ASSERT( GetImageData() );
	GetImageData()->SetImage(image);
}

void ChObjInline::GetObjectSize( ChSize& size )
{
	// if we have a image than we will get the size from ChGIF
	if ( m_pImage && m_pImage->GetImage() )
	{
		m_sizeObject.cx = (int)m_pImage->GetImage()->GetWidth();
		m_sizeObject.cy = (int)m_pImage->GetImage()->GetHeight();
		
	}
	size = m_sizeObject;   
	size.cx += ( m_spaceExtra.left + m_spaceExtra.right ) + (2 * m_iBorder);	
	size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom ) + (2 * m_iBorder);

}

void ChObjInline::GetImageSize( ChSize& size )
{
	// if we have a image than we will get the size from ChGIF
	if ( m_pImage && m_pImage->GetImage() )
	{
		size.cx = (int)m_pImage->GetImage()->GetWidth();
		size.cy = (int)m_pImage->GetImage()->GetHeight();
	}
	else
	{
		size = m_sizeObject;  
	} 
}

bool   ChObjInline::PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel /*= 0 */ )
{
	if ( ptLoc.x >= (ptTopLeft.x  + m_spaceExtra.left + m_iBorder )  
				&&	ptLoc.x < ( ptTopLeft.x + m_spaceExtra.left 
									+ m_sizeObject.cx - m_spaceExtra.right - ( 2 * m_iBorder) ) 
				&&	(ptTopLeft.y + m_spaceExtra.top + m_iBorder) <=  ptLoc.y 
				&& 	(ptTopLeft.y + m_spaceExtra.top -  m_iBorder +
								m_sizeObject.cy - m_spaceExtra.bottom)  > ptLoc.y )
	{
		if (pptRel )
		{
			pptRel->x = ptLoc.x -( m_spaceExtra.left + ptTopLeft.x );
			pptRel->y = ptLoc.y - ( m_spaceExtra.top + ptTopLeft.y );
		}
		return true;
	}

	return false;
	
} 

ChSize ChObjInline::DrawObject( ChTxtWnd *pTxtWnd, int x, int y, chuint32 luStyle, ChSize& viewSize )
{

   	
	ChSize size = m_sizeObject;   
	size.cx += ( m_spaceExtra.left + m_spaceExtra.right + m_iBorder );	
	size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom + m_iBorder );

	if ( m_pImage && m_pImage->GetImage() )
	{
		CDC* pDC = pTxtWnd->GetContext();

		if ( luStyle &  ChTxtWnd::textVCenter )
		{
			if ( size.cy < viewSize.cy  )
			{  // center the text to the height of the control
				y = y + ((viewSize.cy  >> 1) - ( size.cy >> 1));
			}
		}
		if ( m_iBorder )
		{										    
			// Set the attributes
			CPen pen( PS_SOLID, m_iBorder, (COLORREF)m_luBorderColor );
			CPen *pOldPen = pDC->SelectObject( &pen );
			POINT ptPos[5];

			ptPos[0].x = x +  m_spaceExtra.left + ( m_iBorder >> 1);
			ptPos[0].y = y +  m_spaceExtra.top + ( m_iBorder >> 1);
			ptPos[1].x = ptPos[0].x + m_sizeObject.cx + m_iBorder;
			ptPos[1].y = ptPos[0].y;
			ptPos[2].x = ptPos[1].x;
			ptPos[2].y = ptPos[1].y	+ m_sizeObject.cy + m_iBorder;
			ptPos[3].x = ptPos[0].x;
			ptPos[3].y = ptPos[2].y;
			ptPos[4] = ptPos[0];
		
			pDC->Polyline( ptPos, 5 );
			// Restore the attributes changed
			pDC->SelectObject( pOldPen );
		}

		if ( luStyle & ChTxtWnd::textResetAnimation )
		{
			m_pImage->GetImage()->SetFrame( 0 );
		}

		if ( m_pImage->GetImage()->IsTransparent() )
		{
			if ( pTxtWnd->GetCurrentBkImage() )
			{
				m_pImage->GetImage()->Draw( pDC, x +  m_spaceExtra.left + m_iBorder, 
											 y +  m_spaceExtra.top + m_iBorder,
											 m_pImage->GetImage()->GetTransparentColor(),
											 pTxtWnd->GetCurrentBkImage() );
			}
			else
			{

				CBrush * pBrush = pTxtWnd->GetCurrentBkBrush();

				if ( pBrush )
				{

					m_pImage->GetImage()->Draw( pDC, x +  m_spaceExtra.left + m_iBorder, 
												 y +  m_spaceExtra.top + m_iBorder,
												 m_pImage->GetImage()->GetTransparentColor(),
												 pBrush );
				}
				else
				{
					m_pImage->GetImage()->Draw( pDC, x +  m_spaceExtra.left + m_iBorder, 
												 y +  m_spaceExtra.top + m_iBorder,
												 m_pImage->GetImage()->GetTransparentColor(),
												 ::GetSysColor( COLOR_BTNFACE ) );
				}
			}
		}
		else
		{

			m_pImage->GetImage()->Draw( pDC, x +  m_spaceExtra.left + m_iBorder, 
											 y +  m_spaceExtra.top + m_iBorder );
		}												
	}
	else if ( m_pPlugin && m_pPlugin->GetPluginInstance() )
	{
		#if defined ( USE_NATIVE_HWND )
		if ( m_pPlugin->GetPluginInstance()->GetPluginWindow() )
		#else
		if ( m_pPlugin->GetPluginInstance()->GetPluginWindow()->GetSafeHwnd() )
		#endif
		{
			#if defined ( USE_NATIVE_HWND )
			HWND hWnd = m_pPlugin->GetPluginInstance()->GetPluginWindow();
			#else
			CWnd* pWnd = m_pPlugin->GetPluginInstance()->GetPluginWindow();
			#endif

			CRect rtClient;
			if ( GetMode() == embedEmbed )
			{
				if ( luStyle  & ChTxtWnd::textVCenter )
				{
					if ( size.cy < viewSize.cy  )
					{  // center the text to the height of the control
						y = y + ((viewSize.cy  >> 1) - ( size.cy >> 1));
					}
				}

				ChPoint ptPos = pTxtWnd->GetDeviceScrollPosition();
				// Move the control to the right position
				ChRect  rtEmbedRect;
				#if defined ( USE_NATIVE_HWND )
				::GetClientRect( hWnd, &rtEmbedRect );
				#else
				pWnd->GetClientRect( rtEmbedRect );
				#endif
				rtClient.left = (x - ptPos.x) + m_spaceExtra.left +  m_iBorder;
				rtClient.top = (y - ptPos.y) + m_spaceExtra.top + m_iBorder;
				rtClient.right = rtClient.left +  rtEmbedRect.Width();
				rtClient.bottom = rtClient.top +  rtEmbedRect.Height();

				#if defined ( USE_NATIVE_HWND )
				::SetWindowPos( hWnd, 0, (x - ptPos.x) + m_spaceExtra.left +  m_iBorder, 
										 (y - ptPos.y) + m_spaceExtra.top + m_iBorder,
										0, 0, SWP_NOACTIVATE |  SWP_SHOWWINDOW | SWP_NOSIZE
												| SWP_NOZORDER );
				#else
				pWnd->SetWindowPos( 0, (x - ptPos.x) + m_spaceExtra.left +  m_iBorder, 
										 (y - ptPos.y) + m_spaceExtra.top + m_iBorder,
										0, 0, SWP_NOACTIVATE |  SWP_SHOWWINDOW | SWP_NOSIZE
												| SWP_NOZORDER );
				#endif
			}
			else
			{
				pTxtWnd->GetClientRect( rtClient );
				#if defined ( USE_NATIVE_HWND )
				::SetWindowPos( hWnd, 0, 0, 0, rtClient.Width(), rtClient.Height(),
										 SWP_NOACTIVATE |  SWP_SHOWWINDOW 
												| SWP_NOZORDER );
				#else
				pWnd->SetWindowPos( 0, 0, 0, rtClient.Width(), rtClient.Height(),
										 SWP_NOACTIVATE |  SWP_SHOWWINDOW 
												| SWP_NOZORDER );
				#endif
			}
			// call plugin setwindow
			m_pPlugin->GetPluginInstance()->SetWindow( rtClient );
		}
	}
	else
	{ // draw some bitmap
		
	}
	
	return size;
}


/*----------------------------------------------------------------------------
	Implementation of control object
----------------------------------------------------------------------------*/


ChSize ChObjControl::DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize )
{
	ChSize size = m_sizeObject;   
	size.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
	size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );

	#if defined( CH_MSW )

	if ( m_pWnd )
	{

		if ( luStyle  & ChTxtWnd::textVCenter )
		{
			if ( size.cy < viewSize.cy  )
			{  // center the text to the height of the control
				y = y + ((viewSize.cy  >> 1) - ( size.cy >> 1));
			}
		}

		ChPoint ptPos = pWnd->GetDeviceScrollPosition();
		// Move the control to the right position
		m_pWnd->SetWindowPos( 0, x - ptPos.x + m_spaceExtra.left, 
								 y - ptPos.y + m_spaceExtra.top,
								0, 0, SWP_NOACTIVATE |  SWP_SHOWWINDOW | SWP_NOSIZE
										| SWP_NOZORDER );
	}
	#else
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
	#endif

	return size;
}

// $Log$
// Revision 1.2  2003/07/04 11:26:41  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:55:00  uecasm
// Import of source tree as at version 2.53 release.
//
