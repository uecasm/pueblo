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

	All rendering metods are in this file

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

#include <ChConst.h>
#include <ChTxtWnd.h>
#include <ChUtil.h>
#include <ChImgUtil.h>

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::UpdateAnimation

------------------------------------------------------------------------------

	This method draws the text in the view.

----------------------------------------------------------------------------*/

void ChTxtWnd::UpdateAnimation()
{
	pChRun			pRun;
	pstr			pText;
	pChLine			pLine;
	pChStyleInfo	pStyleUse;
	pChStyleInfo	pCurrStyleUse;
	pChFontInfo		pFontTbl;
	int				sDrawCount;
	chint32			lStartChar, lLastChar;
	chint32			lIndex;
	int				x; 
	bool			boolNewStyle;
	ChRect			rtClipBox( 0, 0, GetViewWidth(), GetViewHeight() );
	DWORD 			uUpdateTime = ::GetTickCount();


	if ( !GetTextCount() )
	{
		return;
	}


	ChPoint 		ptOrigin;

	// The current view position
	ptOrigin = GetDeviceScrollPosition();	 

	// set up the bottom indent
	rtClipBox.bottom = ptOrigin.y + GetViewHeight();
	// set up the top indent
	rtClipBox.top = ptOrigin.y ;

	ChClientDC	dc( this );

	m_pDC = &dc;

	OnPrepareDC( m_pDC );

	pRun 		= GetRunTable();
	pText 		= GetTextBuffer();
	pLine 		= GetLineTable();
	pStyleUse 	= GetStyleTable();
	pFontTbl 	= GetFontTable();



	lStartChar 	= 0;
	lLastChar 	= GetTextCount();
	lIndex = 0;

	while ((lIndex < GetLineCount())
		&& ( ( pLine->iY + pLine->iMaxHeight ) < rtClipBox.top ))
	{
		lIndex += 1;
		pLine += 1;
	}

	// This is our new top line
	lStartChar = pLine->lStartChar;
	lIndex = GetRunIndex(  lStartChar );
	pRun += lIndex;
	pText += lStartChar;
	boolNewStyle = true;


   	// Draw all the characters till we hit the bottom of the view
	while ((lStartChar < lLastChar) && ( pLine->iY <  
							( rtClipBox.bottom + pLine->iMaxHeight ) ))
	{
		x = pLine->iX;	// setup the left margin


		if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textCenter )
		{ // justify center
			x += ( (pLine->iMaxLineWidth - pLine->iTotalWidth)/2);
		}
  		else if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textRight )
		{   // justify right
			x += pLine->iMaxLineWidth - pLine->iTotalWidth;
		}

		bool boolObjectFound = false;
		int  iRunIndex = 0;

		while(  pRun[iRunIndex].lStartChar < pLine[1].lStartChar )
		{
			if ( pStyleUse[ pRun[iRunIndex].lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
			{
				boolObjectFound = true;
				break;	
			}
			iRunIndex++;
		}


		// Draw all the charcters in this line
		while ( (lStartChar < lLastChar) && (lStartChar < pLine[1].lStartChar))
		{

			// Number of charcters to draw
			if (pLine[1].lStartChar < pRun[1].lStartChar)
			{  // multiline run
				sDrawCount = (int)(pLine[1].lStartChar - lStartChar);
			}
			else
			{  // multiple runs in the current line
				sDrawCount = (int)(pRun[1].lStartChar - lStartChar);
			}
			// sanity check
			if (sDrawCount > (int)(lLastChar - lStartChar))
			{
				sDrawCount = (int)(lLastChar - lStartChar);
			}
		
			if ( boolObjectFound )
			{

				if (boolNewStyle)
				{	// Set up the DC for the new style
					pCurrStyleUse = &pStyleUse[ pRun->lStyleIndex ];
					::SelectObject(  GetContext()->m_hDC, 
							pFontTbl[pCurrStyleUse->style.iFontIndex].pFont->m_hObject );
					boolNewStyle = false;
				}
				// splat it on the view
				ChSize txtOut;
				if ( pStyleUse[ pRun->lStyleIndex ].style.luStyle & ChTxtWnd::textObject )
				{	// Draw the line

				  	bool	boolUpdate = false;
					ChTextObject** ppObject = GetObjectTable();

					ppObject += pStyleUse[ pRun->lStyleIndex ].style.iObjectIndex;

					if ( ChTextObject::objectImage == (*ppObject)->GetType() )
					{
						ChObjInline* pInline = (ChObjInline*)(*ppObject);
						ChInlineImageData*	pImgData = pInline->GetImageData();
						if (pImgData != NULL)
						{
							ChDib* pDib = pImgData->GetImage();
							pChImageInfo pImageInfo;
	
							if ( pDib && (pImageInfo = pDib->GetImageInfo() )
										&& pImageInfo->boolMultiframe )
							{
								if ( pImageInfo->iLoopCount== 0 || 
											pInline->GetImageData()->m_iLoopCount < pImageInfo->iLoopCount )
								{
									pChImageFrameInfo pFrame = pDib->GetFrameInfo( pDib->GetCurrentFrame() );
	
									if ( pFrame->iExposture )
									{
										// UE: okay, this might be a little inefficient, but unfortunately when we have
										//     shared images (multiple images in the document with the same URL and therefore
										//     also the same pDib pointer) the update times don't always mesh nicely -- besides,
										//     we don't want to call NextFrame too often on the underlying image....
										boolUpdate = true;

										//TRACE("Animation(%08X, %08X): uUpdateTime == %d, uLastUpdate == %d, iExposture == %d\n",
										//	pInline, pDib, uUpdateTime, pImgData->m_uLastUpdate, pFrame->iExposture);
										if ( uUpdateTime > pImgData->m_uLastUpdate +  pFrame->iExposture )
										{
											//TRACE0("Animation: NextFrame\n");
											pImgData->m_uLastUpdate = uUpdateTime;
											pDib->NextFrame();
											boolUpdate = true;
										}
	
									}
									else
									{
										//TRACE0("Animation: no exposture\n");
										pDib->NextFrame();
										boolUpdate = true;
									}
	
									if (pImageInfo->boolMultiframe && (pDib->GetTotalFrames() == 1))
									{
										//// UE: total frames == 1 (but still multiframe) means it's dynamic framed, so we
										////     need to re-evaluate the timer interval
										//if (boolUpdate) {
										//	ComputeTimerInterval(pDib);
										//}
									} 
									else if ( pDib->GetCurrentFrame() == 0 )
									{	// We have looped through all the frames
										pImgData->m_iLoopCount++;  
									}
								}
							}
						}
					}

					if ( boolUpdate )
					{
						// width of the line from current X to the end of the line
						ChSize lineSize( GetViewWidth() - x - m_viewIndents.right, pLine->iMaxHeight );

				
						txtOut = DrawObject(  x, pLine->iY, 
										pStyleUse[ pRun->lStyleIndex ].style, lineSize );
					}
					else
					{
						(*ppObject)->GetObjectSize( txtOut );
					}

					sDrawCount = (int)(pRun[1].lStartChar - pRun[0].lStartChar);

				}
				else
				{	
				 
					int iCount = GetDrawCharCount( pText, sDrawCount );
					txtOut = GetContext()->GetTextExtent( pText, iCount);

				}

				// Set up for the next run
				x += txtOut.cx;
			}

			pText += sDrawCount;
			lStartChar += sDrawCount;
			if (lStartChar == pRun[1].lStartChar)
			{
				pRun += 1;
				boolNewStyle = true;
			}
		}
		
		// Setup for the next line
		pLine += 1;
	}

	return;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:00  uecasm
// Import of source tree as at version 2.53 release.
//
