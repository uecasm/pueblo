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

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/
#define CH_TXTVW_NEWLINE			0x0D	/* LF - line feed  */
#define CH_TXTVW_TAB				0x09	/* HT - tabs  */               


#if (defined( CH_MSW ) && defined( CH_ARCH_16 ) ) || ( defined( CH_UNIX ) )

#define HEAP_ZERO_MEMORY 0x00000001

#endif


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
	Functions :
----------------------------------------------------------------------------*/


#if (defined( CH_MSW ) && defined( CH_ARCH_16 ) ) || ( defined( CH_UNIX ) )



CH_GLOBAL_FUNC( HANDLE )
HeapCreate( int a, int b, int c );

CH_GLOBAL_FUNC( void* )
HeapAlloc( HANDLE hHeap, chuint32 flags, chuint32 luLen );

CH_GLOBAL_FUNC( int )
HeapDestroy( HANDLE hHeap );

CH_GLOBAL_FUNC( void* )
HeapReAlloc( HANDLE hHeap, chuint32 flags, void *oldbuf, chuint32 luNewSize );

CH_GLOBAL_FUNC( void )
HeapFree( HANDLE hHeap, chuint32 flags, void *buf );

CH_GLOBAL_FUNC( chuint32 )
HeapSize( HANDLE hHeap, chuint32 flags, void *buf );

#endif // defined( CH_MSW ) && defined( CH_ARCH_16 )



/*----------------------------------------------------------------------------

	FUNCTION		||	ChTxtWnd::UpdateLayoutInfo


------------------------------------------------------------------------------


----------------------------------------------------------------------------*/

void ChTxtWnd::UpdateLayoutInfo( pLineData pCalc,  pChLine pNewLine, bool boolFloatLeft  )
{
   int iWidth = pNewLine->iTotalWidth;
   int iHeight = pNewLine->iMaxHeight;

	// resize if required
  	if ( (pCalc->iCurrLayoutIndex + 1) >= pCalc->iLayoutMaxSize )
	{	// realloc the layout table
		pCalc->iLayoutMaxSize += 5;
		pCalc->pLayoutInfo =  (pLayoutData)::HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY,
												pCalc->pLayoutInfo,
												sizeof( LayoutData ) 
												* pCalc->iLayoutMaxSize );
		if (0 == pCalc->pLayoutInfo)
		{
			ASSERT( pCalc->pLayoutInfo );
			return;
		}
	}

	pLayoutData pLayout = pCalc->pLayoutInfo;

	LayoutData tmpEntry;

	// copy the previous state
	tmpEntry =  pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex]; 

	
	if ( boolFloatLeft )
	{

		// new left index
		tmpEntry.iCurrX += iWidth;

		// new break Y
		tmpEntry.iBreakY = pCalc->iCurrY + iHeight;

		// new page width
		tmpEntry.iCurrPageWidth -= iWidth; 
	}
	else
	{
		// new break Y
		tmpEntry.iBreakY = pCalc->iCurrY + iHeight;

		// reposition X-cordinate of the line to be floating to the right
		int iOldX = pNewLine->iX; 

		pNewLine->iX += (tmpEntry.iCurrPageWidth - iWidth );

		if ( pNewLine->iX < m_viewIndents.left )
		{
			pNewLine->iX = iOldX;
		}

		// new page width
		tmpEntry.iCurrPageWidth -= iWidth; 
	}
	// recompute if the CurrY is not sorted in decending order
	if (  pCalc->iCurrLayoutIndex == 0 )
	{
		pLayout[ ++pCalc->iCurrLayoutIndex ] = tmpEntry;
	}
	else
	{
	   	if ( tmpEntry.iBreakY <= pLayout[pCalc->iCurrLayoutIndex].iBreakY )
		{
			pLayout[ ++pCalc->iCurrLayoutIndex ] = tmpEntry;
		}
		else
		{ // got to sort the layinfo
			//find the index for the new entry
			int iIndex = 0;

			for( int i = pCalc->iCurrLayoutIndex ; i > 0; i-- )
			{
	   			if ( tmpEntry.iBreakY <= pLayout[i].iBreakY )
				{
					iIndex = i;
					break;
				}
			}
			//location to insert
			iIndex += 1;
	
			pLayout[ ++pCalc->iCurrLayoutIndex ] = tmpEntry;

			// Move all iBreakY upwards
			for ( i = pCalc->iCurrLayoutIndex; i > iIndex; i-- )
			{
				pLayout[i].iBreakY = pLayout[i - 1].iBreakY;
			}
			// insert it to the right loaction
			pLayout[iIndex].iBreakY = tmpEntry.iBreakY;


			// if this is to the right then X is same as the previous
			if ( !boolFloatLeft )
			{
				pLayout[ iIndex].iCurrX = pLayout[iIndex - 1].iCurrX;
			}
			else
			{
				pLayout[ iIndex].iCurrX += iWidth;
			}
		}
	}
}


/*----------------------------------------------------------------------------

	FUNCTION		||	ChTxtWnd::UpdateLineTable

	pboolChanged	||	If non-zero, returns whether or not any line endings
						have changed.

------------------------------------------------------------------------------

	Updates the line table from the given character offset to the end of
	the buffer using the information in the style and run table.

----------------------------------------------------------------------------*/

bool ChTxtWnd::UpdateLineTable(  chint32 lStartChar, chint32 lCharCount,
									UINT fsOptions, bool* pboolChanged )
{
	pChLine			pNewLine, pNewLineStart;
	chint32			lNewLineCount;
	pChLine			pOldLine;
	chint32			lOldStartLine;
	chint32			lOldLineIndex;
	chint32			lOldLineCount;
	chint32			lAboveHeight;
	chint32			lCalcOrigLine;
	chint32			lRunIndex;
	bool			boolOK;
	LineData		calc;
	bool			boolDummyChanged; 

	// Set this to empty rect for now
  	m_updateRect.SetRectEmpty();

	if (0 == pboolChanged)
	{										/* Set a pointer to avoid testing
												later */
		pboolChanged = &boolDummyChanged;
	}

	*pboolChanged = true;					// Set it to true now, we fill fix this later
			// Assume not changed

	if (GetTextCount() == 0)
	{
		return true;
	}

	ChMemClearStruct( &calc );



											// Allocate a new line table
	lNewLineCount = 0;
	lCalcOrigLine = 1;

	calc.pText = GetTextBuffer();
	pOldLine = GetLineTable();

	// allocate the layout info table
	calc.iLayoutMaxSize = 5;
	calc.pLayoutInfo = (pLayoutData)::HeapAlloc( GetHeap(), HEAP_ZERO_MEMORY,
									(sizeof( LayoutData ) * calc.iLayoutMaxSize) );

	ASSERT( calc.pLayoutInfo );
	// set upinfo in the layout
	calc.pLayoutInfo->iCurrX = m_viewIndents.left;
	calc.iCurrY 			= m_viewIndents.top;
	calc.pLayoutInfo->iBreakY = -1;	 // no floating objects
	calc.pLayoutInfo->iCurrPageWidth = GetPageWidth();
	calc.iMaxLineWidth = GetPageWidth();   


	lOldStartLine = 0;
	lOldLineIndex = 0;
	lOldLineCount = 0;

	if (fsOptions & vwDelete)
	{
		if (lStartChar == GetTextCount())
		{
			lStartChar -= 1;

			lOldLineIndex = GetLineIndex( lStartChar, lAboveHeight );
			UpdateLineStarts( lOldLineIndex, -lCharCount );

			lCharCount = 1;
		}
		else
		{
			lOldLineIndex = GetLineIndex( lStartChar, lAboveHeight );
			UpdateLineStarts( lOldLineIndex, -lCharCount );
		}
	}
	else /* (fsOptions & vwUpdate) */
	{
		lOldLineIndex = GetLineIndex( lStartChar, lAboveHeight );
	}

	if (lOldLineIndex < 0)
	{
		lOldLineIndex = 0;
	}
	else if (lOldLineIndex > 0)
	{
		while ( lOldLineIndex && (pOldLine[lOldLineIndex].luLineAttr & objAttrFloat) )
		{   // find the first non-floating line
			--lOldLineIndex;
			lCalcOrigLine++;	
		}
	
		if ( lOldLineIndex )
		{
			pOldLine += lOldLineIndex;

			while ( lOldLineIndex && pOldLine->lStartChar > 0 ) 
			{

				if ( ( calc.pText[ pOldLine->lStartChar - 1 ] == CH_TXTVW_NEWLINE 
						|| pOldLine->luLineAttr & ChTxtWnd::objAttrBreak  )
						&& 	pOldLine->iX ==  m_viewIndents.left )
				{
					break;
				}
				else
				{
					pOldLine -= 1;
					lOldLineIndex -= 1;
					lAboveHeight -= pOldLine->iMaxHeight;
					lCalcOrigLine++;
				}
			}
			calc.lStartChar = pOldLine->lStartChar;
			lOldStartLine = lOldLineIndex;
			calc.iCurrY = pOldLine->iY;

		}
		else
		{
			calc.lStartChar = pOldLine->lStartChar;
			lOldStartLine = lOldLineIndex;
			calc.iCurrY = pOldLine->iY;
		}
	}
	calc.pText += calc.lStartChar;

	calc.pStyleUse = GetStyleTable();
	calc.pRun = GetRunTable();
	lRunIndex = GetRunIndex(  calc.lStartChar );
	calc.pRun += lRunIndex;


	int 	iLineAllocCount = 20;

	pNewLineStart =
			(pChLine)::HeapAlloc( GetHeap(), HEAP_ZERO_MEMORY,
									(sizeof( ChLine ) * iLineAllocCount) );
	pNewLine = pNewLineStart;
	pNewLine->lStartChar = calc.lStartChar;

	boolOK = true;

	m_updateRect.SetRect( m_viewIndents.left, calc.iCurrY, 
								GetViewWidth(), calc.iCurrY );

	while (true )
	{
		chint32 lTemp = calc.lStartChar;

		ComputeLineInfo( &calc, pNewLine ); 
		
		if ( (pNewLine->iY + pNewLine->iMaxHeight) > m_updateRect.bottom   )
		{
			m_updateRect.bottom = pNewLine->iY + pNewLine->iMaxHeight ;
		}
		
		if ( lTemp == calc.lStartChar )
		{
			if (  calc.lStartChar >= GetTextCount() )
			{
				break;
			}
			else
			{
				ASSERT( false );
			}
		}
	
		lNewLineCount += 1;

		if (iLineAllocCount < (lNewLineCount + 1))
		{
			while ( iLineAllocCount < 	(lNewLineCount + 1 ) )
			{	// allocate in blocks of 20 lines
				iLineAllocCount += 20;
			}

			pNewLineStart =  (pChLine)::HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY,
													pNewLineStart,
													sizeof( ChLine ) * iLineAllocCount );
			if (0 == pNewLineStart)
			{
				ASSERT( pNewLineStart );
				break;
			}
			pNewLine = pNewLineStart;
			pNewLine += lNewLineCount;
		}
		else
		{
			pNewLine += 1;
		}

		pNewLine->lStartChar = calc.lStartChar;

		while ((lOldLineIndex < GetLineCount()) &&
				(pOldLine->lStartChar < pNewLine->lStartChar))
		{
			lOldLineCount += 1;
			lOldLineIndex += 1;
			pOldLine += 1;
		}

		if (calc.lStartChar >= GetTextCount())
		{
			break;
		}
		if (calc.lStartChar < lStartChar + lCharCount)
		{
			continue;
		}
		if ((lNewLineCount > lCalcOrigLine)
			&& (lOldLineIndex < GetLineCount())
			&& (pOldLine->iX == pNewLine->iX)
			&& (pOldLine->iY == pNewLine->iY)
			&& (pOldLine->lStartChar == pNewLine->lStartChar))
		{
			break;
		}
	}

	// Free the layout stack
	if ( calc.pLayoutInfo )
	{
		HeapFree( GetHeap(), 0, calc.pLayoutInfo );
	}

	if (! boolOK)
	{
		HeapFree( GetHeap(), 0, pNewLine);
		return (false);
	}


	if ((lOldStartLine == 0) && (lOldLineCount == GetLineCount()))
	{
		HeapFree( GetHeap(), 0, GetLineTable());
		m_pLinesTbl = pNewLineStart;
		m_lLineCount = lNewLineCount;

		ChSize sizeOld( m_sizeTotal );

		// Set the new canvas size
		SetViewSize();

		// Check if the document shrunk since the last update
		if ( m_sizeTotal.cy < sizeOld.cy )
		{
			m_updateRect.bottom += ( sizeOld.cy - m_sizeTotal.cy );
		}

		if (  GetDocumentAttrs() & docVCenter )
		{
			ChSize sizeDoc;
			GetDocumentSize( sizeDoc );

			if ( sizeDoc.cy < GetPageHeight() )
			{  // do vertival center of the document
				int iOffset = (GetPageHeight() - sizeDoc.cy)  >> 1;

				pChLine 	pLine  = GetLineTable();

				for ( int lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex++ )
				{		
					pLine->iY += iOffset;
					pLine += 1;
				}

				m_updateRect.bottom += iOffset;
			}
		}

		return (true);
	}

	// if we come here the line table has changed
	*pboolChanged = true;

	chint32			lDiffLineCount;

	if (lOldLineCount < lNewLineCount)
	{
		lDiffLineCount = lNewLineCount - lOldLineCount;
		m_pLinesTbl = (pChLine)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY,
										GetLineTable(), sizeof( ChLine )
							* (GetLineCount() + lDiffLineCount + 1));

		pNewLine = pNewLineStart;
		pOldLine = GetLineTable();

		ChMemMove(
			pOldLine + (lOldStartLine + lOldLineCount + lDiffLineCount),
			pOldLine + (lOldStartLine + lOldLineCount),
						(chuint16)(sizeof( ChLine ) * ((GetLineCount() + 1)
					- (lOldStartLine + lOldLineCount )) ));
	}
	else if (lOldLineCount > lNewLineCount)
	{
		pNewLine = pNewLineStart;
		pOldLine = GetLineTable();

		ChMemMove(
			(pOldLine + (lOldStartLine + lNewLineCount)),
			(pOldLine + (lOldStartLine + lOldLineCount)),
			(chuint16)(sizeof( ChLine ) * ((GetLineCount() + 1)
			- (lOldStartLine + lOldLineCount))) );


		lDiffLineCount = lOldLineCount - lNewLineCount;
		m_pLinesTbl = (pChLine)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetLineTable(),  sizeof( ChLine )
									* (GetLineCount() - lDiffLineCount + 1));

		pNewLine = pNewLineStart;
		pOldLine = GetLineTable();
	}
	else /* (lOldLineCount == lNewLineCount) */
	{
		pNewLine = pNewLineStart;
		pOldLine = GetLineTable();
	}
	m_lLineCount += lNewLineCount - lOldLineCount;

	ChMemCopy( pOldLine + lOldStartLine, pNewLine,
		(chuint16)(sizeof( ChLine ) * lNewLineCount) );
	pOldLine[ GetLineCount() ].lStartChar = GetTextCount(); 
	
	HeapFree( GetHeap(), 0, pNewLine );

	ChSize sizeOld( m_sizeTotal );

	// Set the new canvas size
	SetViewSize();

	// Check if the document shrunk since the last update
	if ( m_sizeTotal.cy < sizeOld.cy )
	{
		m_updateRect.bottom += ( sizeOld.cy - m_sizeTotal.cy );
	}

	if ( GetDocumentAttrs() & docVCenter )
	{
		ChSize sizeDoc;
		GetDocumentSize( sizeDoc );

		if ( sizeDoc.cy < GetPageHeight() )
		{  // do vertival center of the document
			int iOffset = (GetPageHeight() - sizeDoc.cy) >> 1;

			pChLine 	pLine  = GetLineTable();

			for ( int lLineIndex = 0; lLineIndex < GetLineCount(); lLineIndex++ )
			{		
				pLine->iY += iOffset;
				pLine += 1;
			}
			m_updateRect.bottom += iOffset;
		}
	}


	return (true);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeLineInfo()

------------------------------------------------------------------------------

	Calculates the line height, width and char count of the line.

----------------------------------------------------------------------------*/

void ChTxtWnd::ComputeLineInfo(  pLineData pCalc, pChLine pNewLine )
{
	bool			boolEndOfLine;
	pChStyleInfo	pStyleUse;
	pChRun			pRun;
 
 	pNewLine->iTotalWidth = 0;
//	pNewLine->iMaxAscent = 0;
	pNewLine->iMaxDescent = 0;
	pNewLine->iMaxHeight = 0;
	pNewLine->luLineAttr = 0;

	boolEndOfLine = false;	 

	pRun = pCalc->pRun;
	pStyleUse = &pCalc->pStyleUse[ pRun[pCalc->iWordRunCount].lStyleIndex ];

	if ( pCalc->iCurrLayoutIndex )
	{ // do we need to change the layout for the page
		while( pCalc->iCurrLayoutIndex && 
						pCalc->iCurrY > 
						pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iBreakY	)
		{
			pCalc->iCurrLayoutIndex--;	
		}
	}

	pCalc->iMaxLineWidth = pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iCurrPageWidth;
	

  	if ( pStyleUse->style.luStyle  & ChTxtWnd::textIndentLeft )
	{// we have left  indentation
		pCalc->iMaxLineWidth -=  pStyleUse->style.iLeftIndent;

	}
	
	// top of current line
  	if ( pStyleUse->style.luStyle  & ChTxtWnd::textIndentLeft )
	{
		pNewLine->iX = pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iCurrX +
							pStyleUse->style.iLeftIndent;
	}
	else
	{
		pNewLine->iX = pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iCurrX;
	}
	pNewLine->iY = pCalc->iCurrY;
	pNewLine->iMaxLineWidth = pCalc->iMaxLineWidth;

	if ( pCalc->iCurrLayoutIndex )
	{
		pNewLine->luLineAttr = objAttrFloat;
	}
	
	//cannot do no wrap when there is a floating object
	if ( pCalc->iCurrLayoutIndex == 0 && 
			pStyleUse->style.luStyle  & ChTxtWnd::textNoWrap )
	{// line will not wrap for this style

	   	chint32 lStartChar = pCalc->lStartChar;

		while ((pCalc->lStartChar < GetTextCount())  
					&& (! boolEndOfLine) && ( !pCalc->boolStartNewLine ) )
		{

			if ( ((pNewLine->iTotalWidth + pCalc->iWordWidth + pCalc->iWordWhiteWidth )
				 > pCalc->iMaxLineWidth )
				 && !(pCalc->pStyleUse[ pCalc->pRun[pCalc->iWordRunCount].lStyleIndex ]
					.style.luStyle & ChTxtWnd::textNoWrap) )
			{  // if we have a no wrap style now which is causes the line to exceed
			   // max line width, stop processing this line
				break;
			}

			if (pNewLine->iMaxHeight < pCalc->iWordHeight)
			{
				pNewLine->iMaxHeight = pCalc->iWordHeight;
			}
//			if (pNewLine->iMaxAscent < pCalc->iWordAscent)
			{
//				pNewLine->iMaxAscent = pCalc->iWordAscent;
			}
			if (pNewLine->iMaxDescent < pCalc->iWordDescent)
			{
				pNewLine->iMaxDescent = pCalc->iWordDescent;
			}
			pNewLine->iTotalWidth += (pCalc->iWordWidth + pCalc->iWordWhiteWidth);
			pNewLine->luLineAttr = pCalc->luLineAttr;

			boolEndOfLine = ComputeNextWordInfo( pCalc );
			ComputeWordInfo( pCalc, GetWordCharCount( pCalc ));
		}

		if ( lStartChar != pCalc->lStartChar )
		{
			pCalc->iCurrY += pNewLine->iMaxHeight;
			return;  // we have the line info for nowrap no more processing
		}
		// fall through
	}



	while (((pNewLine->iTotalWidth + pCalc->iWordWidth + pCalc->iWordWhiteWidth )
				 <= pCalc->iMaxLineWidth )
				&& (pCalc->lStartChar < GetTextCount()) && (! boolEndOfLine) 
				&& ( !pCalc->boolStartNewLine  ) )
	{
		if (pNewLine->iMaxHeight < pCalc->iWordHeight)
		{
			pNewLine->iMaxHeight = pCalc->iWordHeight;
		}
//		if (pNewLine->iMaxAscent < pCalc->iWordAscent)
		{
//			pNewLine->iMaxAscent = pCalc->iWordAscent;
		}
		if (pNewLine->iMaxDescent < pCalc->iWordDescent)
		{
			pNewLine->iMaxDescent = pCalc->iWordDescent;
		}
		pNewLine->iTotalWidth += (pCalc->iWordWidth + pCalc->iWordWhiteWidth);
		pNewLine->luLineAttr = pCalc->luLineAttr;

		boolEndOfLine = ComputeNextWordInfo( pCalc );
		ComputeWordInfo( pCalc, GetWordCharCount( pCalc ));
	}

	if ( pCalc->boolObject && !boolEndOfLine )
	{
		if (  pNewLine->iTotalWidth == 0  )
		{
			pNewLine->iMaxHeight = pCalc->iWordHeight;
//			pNewLine->iMaxAscent = pCalc->iWordAscent;
			pNewLine->iMaxDescent = pCalc->iWordDescent;
			pNewLine->iTotalWidth = pCalc->iWordWidth + pCalc->iWordWhiteWidth;
			pNewLine->luLineAttr |= pCalc->luLineAttr; 

			if ( pCalc->boolObjectFloat )
			{
				pNewLine->luLineAttr |=  ChTxtWnd::objAttrFloat;

				if ( pNewLine->luLineAttr & ChTxtWnd::objAttrLeft )
				{
					UpdateLayoutInfo( pCalc, pNewLine, true );

				}
				else if ( pNewLine->luLineAttr & objAttrRight )
				{
					UpdateLayoutInfo( pCalc, pNewLine, false );
				}
				// check the new width and of our page
				if ( pCalc->iCurrLayoutIndex &&
							pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iCurrPageWidth < minWidth )
				{ // discard this layout and pop the stack
					pCalc->iCurrY = pCalc->pLayoutInfo[pCalc->iCurrLayoutIndex].iBreakY;
					pCalc->iCurrLayoutIndex--;
				}
			}
			else
			{	// this line contains only an object
				pCalc->iCurrY += 	pNewLine->iMaxHeight; // top of next line
			}

			ComputeNextWordInfo( pCalc );
			ComputeWordInfo(pCalc, GetWordCharCount( pCalc ));
		}
		else  
		{ // width != 0, I have an object and it wants to start a new line
		  // so break the old line
			pCalc->iCurrY  += pNewLine->iMaxHeight; // top of next line
		}
	}
	else
	{
		pCalc->iCurrY  += pNewLine->iMaxHeight; // top of next line

	    if ((pNewLine->iTotalWidth > 0 || pCalc->iWordWhiteWidth ) &&
			(pNewLine->iTotalWidth + pCalc->iWordWidth + pCalc->iWordWhiteWidth >
			 pCalc->iMaxLineWidth )     &&
			(pNewLine->iTotalWidth + pCalc->iWordWidth < pCalc->iMaxLineWidth ) &&
			(pCalc->lStartChar < GetTextCount()) && (! boolEndOfLine) )
		{
			/* last word with its whole white space can not be fit in the current
			line. Try to split the white space and fit whatever possible.
			Note: The current line already has some text not like the
			next case. */

			ComputeLastWordInALine(  pCalc, pNewLine->iTotalWidth );

			if ( pCalc->iWordWhiteWidth > 0 )
			{
				/*	Accept this word if there is non zero length of white space
				 *	following the word
				 */
				if (pNewLine->iMaxHeight < pCalc->iWordHeight)
				{
					pNewLine->iMaxHeight = pCalc->iWordHeight;
				}
//				if (pNewLine->iMaxAscent < pCalc->iWordAscent)
				{
//					pNewLine->iMaxAscent = pCalc->iWordAscent;
				}
				if (pNewLine->iMaxDescent < pCalc->iWordDescent)
				{
					pNewLine->iMaxDescent = pCalc->iWordDescent;
				}

				pNewLine->iTotalWidth += (pCalc->iWordWidth + pCalc->iWordWhiteWidth);

				ComputeNextWordInfo( pCalc );
				ComputeWordInfo(  pCalc, GetWordCharCount( pCalc ));
			}
	    }

		if ((pNewLine->iTotalWidth == 0) && (! boolEndOfLine) )
		{
			// we will not split big words, this feature is not required
			#if 0
			/* next word is bigger than the whole line */
			ComputeBigWord( pCalc );
			#endif

			pNewLine->iMaxHeight = pCalc->iWordHeight;
//			pNewLine->iMaxAscent = pCalc->iWordAscent;
			pNewLine->iMaxDescent = pCalc->iWordDescent;
			pNewLine->iTotalWidth = pCalc->iWordWidth + pCalc->iWordWhiteWidth;
			// The one added above had rthe max height = 0, update to reflect the
			// new height
			pCalc->iCurrY  += pNewLine->iMaxHeight; // top of next line


			ComputeNextWordInfo( pCalc );
			ComputeWordInfo(pCalc, GetWordCharCount( pCalc ));
		}
	}
	return;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeBigWord()

------------------------------------------------------------------------------

	This method is called when the word is too big to fit in one line,
	this breaks up the word to fit the line. method uses divide and conquer
	approch to fit the word.

----------------------------------------------------------------------------*/

void ChTxtWnd::ComputeBigWord(  pLineData pCalc )
{
	int		iBigCount;
	int		iBigWidth;
	int		iSmallCount;
	int		iSmallWidth;
	int		iTryCount;

    iBigCount   = pCalc->iWordCharCount;
	iSmallCount = 0;
	iSmallWidth = 0;

    while (((pCalc->iWordWidth > pCalc->iMaxLineWidth)	||
			(iBigCount - iSmallCount > 1))				&&
		   (pCalc->iWordCharCount > 1))
	{
		if (pCalc->iWordWidth > pCalc->iMaxLineWidth)
		{
			iBigCount = pCalc->iWordCharCount;
			iBigWidth = pCalc->iWordWidth;
		}
		else
		{
			iSmallCount = pCalc->iWordCharCount;
			iSmallWidth = pCalc->iWordWidth;
		}
		iTryCount = iSmallCount + ((iBigCount - iSmallCount) / 2);
		ComputeWordInfo( pCalc, iTryCount );
	}
	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeLastWordInALine()

------------------------------------------------------------------------------

	This method is called when the word is too big to fit in one line,
	this breaks up the word to fit the line. method uses divide and conquer
	approch to fit the word.
	.
	.

----------------------------------------------------------------------------*/

void ChTxtWnd::ComputeLastWordInALine( pLineData pCalc, int sCurrLineWidth )
{
    int	iBigCount;
    int	iSmallCount;
    int	iTryCount;

   iBigCount   = pCalc->iWordCharCount;
    iSmallCount = 0;

    while ( (sCurrLineWidth + pCalc->iWordWidth + pCalc->iWordWhiteWidth >
			 pCalc->iMaxLineWidth)
			|| (iBigCount - iSmallCount > 1))
    {
		if (sCurrLineWidth + pCalc->iWordWidth + pCalc->iWordWhiteWidth >
			pCalc->iMaxLineWidth)
		{
			iBigCount = pCalc->iWordCharCount;
		}
		else
		{
			iSmallCount = pCalc->iWordCharCount;
		}
		iTryCount = iSmallCount + ((iBigCount - iSmallCount) / 2);
		ComputeWordInfo( pCalc, iTryCount );
    }

    return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeNextWordInfo()

------------------------------------------------------------------------------

	Update the LineData to point to the next word.

----------------------------------------------------------------------------*/

bool ChTxtWnd::ComputeNextWordInfo( pLineData pCalc )
{
	pCalc->lStartChar += pCalc->iWordCharCount;
	pCalc->pText += pCalc->iWordCharCount;
	pCalc->pRun += pCalc->iWordRunCount;

	return( (pCalc->iWordCharCount > 0)
			&& (pCalc->pText[ -1 ] == (char)CH_TXTVW_NEWLINE) );
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetWordCharCount()

------------------------------------------------------------------------------

	Method returns the number of characters in the given word.

----------------------------------------------------------------------------*/

int ChTxtWnd::GetWordCharCount( pLineData pCalc )
{							 
	bool		boolWhiteSpace, boolObject;
	char		strChar;
	pstr		pstrText;
	int			iWidth;

	pstrText = pCalc->pText;
	boolObject = boolWhiteSpace = false;

	for (iWidth = 0; pCalc->lStartChar + iWidth < GetTextCount();)
	{
		strChar = *pstrText++;
		if (strChar == TEXT(' ')) /* found white space */
		{
			iWidth += 1;
			boolWhiteSpace = true;
		}
		else if (strChar == CH_TXTVW_TAB) /* found white space */
		{
			iWidth += 1;
			boolWhiteSpace = true;
			//pstrText[-1] = TEXT(' ');
		}
		else if (strChar == CH_TXTVW_NEWLINE)	/* count the newline and stop */
		{
			iWidth += 1;
			break;
		}
		else if ( strChar == '\b' )
		{
			if ( iWidth )
			{  // we have a word
				if ( boolObject )
				{
					iWidth += 1;
				}
				break;
			}
			else
			{ // we have an object
				iWidth += 1;
				boolObject = true;
			}
		}
		else if (boolWhiteSpace) /* end of white space, don't count char */
		{
			break;
		}
		else /* non-white char and no white space yet found */
		{
			iWidth += 1;
		}
	}
	return (iWidth);
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ComputeWordInfo()

------------------------------------------------------------------------------

	This method computes the width of the word and updates the LineData
	structure.

----------------------------------------------------------------------------*/

void ChTxtWnd::ComputeWordInfo(  pLineData pCalc, int iWordCharCount )
{
	int				iWhiteCharCount;
	int				iDrawCharCount;
	int				iRunCharCount;
	int				sCharIndex;
	int				iHeight;
	pChStyleInfo	pStyleUse;
	pChFontInfo		pFontTbl;
	pChRun			pRun;


	pCalc->iWordWidth = 0;
	pCalc->iWordWhiteWidth = 0;
	pCalc->iWordHeight = 0;
//	pCalc->iWordAscent = 0;
	pCalc->iWordDescent = 0;
	pCalc->iWordRunCount = 0;
	pCalc->iWordCharCount = iWordCharCount;
	pCalc->boolObject = false;
	pCalc->boolObjectFloat = false;
	pCalc->boolStartNewLine = false;
	pCalc->luLineAttr = 0;;
	pRun = pCalc->pRun;
	pStyleUse = &pCalc->pStyleUse[ pRun->lStyleIndex ];
	pFontTbl  = GetFontTable();


	if (pCalc->lStartChar >= GetTextCount()) 
	{
		return;
	}


	HFONT			hOldFont;
	//int				sOldExtra;

	hOldFont = (HFONT)::SelectObject( GetContext()->m_hDC, 
								::GetStockObject( SYSTEM_FONT ));
	//sOldExtra = GetContext()->GetTextCharacterExtra( );

	for (sCharIndex = 0; sCharIndex < pCalc->iWordCharCount;
		sCharIndex += iRunCharCount)
	{
		if (pCalc->lStartChar + sCharIndex < pRun[1].lStartChar)
		{
			iRunCharCount = (int)(pRun[1].lStartChar
				- (pCalc->lStartChar + sCharIndex));
		}
		else
		{
			pRun += 1;
			iRunCharCount =(int)(pRun[1].lStartChar - pRun[0].lStartChar);
			pCalc->iWordRunCount += 1;
			pStyleUse = &pCalc->pStyleUse[ pRun->lStyleIndex ];
		}

		if (iRunCharCount > pCalc->iWordCharCount - sCharIndex)
		{
			iRunCharCount = pCalc->iWordCharCount - sCharIndex;
		}

		if ( pStyleUse->style.luStyle & ChTxtWnd::textObject )
		{	  
			// now this will point to the object info
			ChSize objSize = ComputeObjectInfo( pCalc );
			pCalc->iWordWidth = objSize.cx;
			iHeight = objSize.cy;
		} 
		else
		{

			::SelectObject( GetContext()->m_hDC, 
						pFontTbl[pStyleUse->style.iFontIndex].pFont->m_hObject	);
			//GetContext()->SetTextCharacterExtra( pStyleUse->style.iAddWidth );

			iDrawCharCount = GetDrawCharCount( &pCalc->pText[ sCharIndex ],
				iRunCharCount );
			iWhiteCharCount = GetWhiteSpaceCount( &pCalc->pText[ sCharIndex ],
				iDrawCharCount );

			if (iDrawCharCount > iWhiteCharCount)
			{
				ChSize textSize;
				{
					//#if defined( CH_MSW )
					//textSize = GetContext()->GetTabbedTextExtent( &pCalc->pText[ sCharIndex ],
					//	iDrawCharCount - iWhiteCharCount, 0, 0 );
					//#else
					textSize = GetContext()->GetTextExtent( &pCalc->pText[ sCharIndex ],
						iDrawCharCount - iWhiteCharCount );
					//#endif
				}
				pCalc->iWordWidth += textSize.cx;

			}
			if (iWhiteCharCount > 0)
			{
				ChSize textSize;

				//#if defined( CH_MSW )
				//textSize = GetContext()->GetTabbedTextExtent(
				//			&pCalc->pText[ sCharIndex +
				//			iDrawCharCount - iWhiteCharCount],
				//			iWhiteCharCount, 0, 0 );
				//#else
				textSize = GetContext()->GetTextExtent(
							&pCalc->pText[ sCharIndex +
							iDrawCharCount - iWhiteCharCount],
							iWhiteCharCount );
				//#endif
				pCalc->iWordWhiteWidth += textSize.cx;
			}
			iHeight = GetExtraHeight() + pStyleUse->iFontHeight;
		
//			if (pCalc->iWordAscent < pStyleUse->iFontAscent)
			{
//				pCalc->iWordAscent = pStyleUse->iFontAscent;
			}
			if (pCalc->iWordDescent < pStyleUse->iFontDescent)
			{
				pCalc->iWordDescent = pStyleUse->iFontDescent;
			}
		}

		if (pCalc->iWordHeight < iHeight)
		{
			pCalc->iWordHeight = iHeight;
		}
	}

	::SelectObject( GetContext()->m_hDC, hOldFont) ;
	//GetContext()->SetTextCharacterExtra( sOldExtra );

	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::UpdateLineStarts()

------------------------------------------------------------------------------

	Updates line offset.
	.
----------------------------------------------------------------------------*/

void ChTxtWnd::UpdateLineStarts( chint32 lStartLine, chint32 lCharCount )
{
	pChLine		pLine;

	pLine = GetLineTable();
	pLine += lStartLine + 1;

	while (lStartLine < GetLineCount())
	{
		pLine->lStartChar += lCharCount;
		lStartLine += 1;
		pLine += 1;
	}

	return;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::UpdateRunStarts()

------------------------------------------------------------------------------

	Updates run offset.
	.
----------------------------------------------------------------------------*/

void ChTxtWnd::UpdateRunStarts( chint32 lStartRun, chint32 lCharCount )
{
	pChRun		pRun;

	pRun = GetRunTable();
	pRun += lStartRun + 1;

	while (lStartRun < GetRunCount())
	{
		pRun->lStartChar += lCharCount;
		lStartRun += 1;
		pRun += 1;
	}

	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ChangeStyle()

------------------------------------------------------------------------------

	Updates style and run information for a text block..
	.
----------------------------------------------------------------------------*/

bool ChTxtWnd::ChangeStyle( pChStyle pNewStyle, chint32 lStartRun,
	chint32 lStartChar, chint32 lCharCount )
{
	chint32			lNextStartChar;
	pChStyleInfo	pStyleUse;
	pChRun			pRunBase;
	pChRun			pRun;


	pRunBase =  GetRunTable();
	pStyleUse = GetStyleTable();

	pRun = pRunBase + lStartRun;
	lNextStartChar = lStartChar + lCharCount;

	if (lStartRun < 0)
	{
		/* everything is empty, so make the first run */

		pRunBase = InsertRuns( pRunBase, ( chint32 )0, 1 );
		pRun = pRunBase;

		pRun[1].lStartChar = lNextStartChar;
		pRun[0].lStartChar = lStartChar;
		pStyleUse = GetStyle(  pStyleUse, pNewStyle,
			pRun[0].lStyleIndex );
	}
	else if ((pRun[1].lStartChar >= lNextStartChar)
		&& EqualStyle( &pStyleUse[ pRun[0].lStyleIndex ], pNewStyle ))
	{
		/* "[<0>]" change area is enclosed in run 0 and has same style */
		/* NOP case */
	}
	else if ((pRun[1].lStartChar > lNextStartChar)
		&& (pRun[0].lStartChar == lStartChar) && (lStartRun > 0)
		&& EqualStyle( &pStyleUse[ pRun[-1].lStyleIndex ], pNewStyle ))
	{
		pRun[0].lStartChar = lNextStartChar;
	}
	else if ((pRun[0].lStartChar == lStartChar)
		&& (pRun[1].lStartChar <= lNextStartChar))
	{

		pRunBase = DeleteRuns( pRunBase, lStartRun + 1, lNextStartChar );
		pRun = pRunBase + lStartRun;

		ReleaseStyle( &pStyleUse[ pRun[0].lStyleIndex ] );
		pStyleUse = GetStyle( pStyleUse, pNewStyle,
			pRun[0].lStyleIndex );
	}
	else if ((lStartRun < GetRunCount() - 1)
		&& (pRun[2].lStartChar <= lNextStartChar))
	{

		pRunBase = DeleteRuns( pRunBase, lStartRun + 2, lNextStartChar );
		pRun = pRunBase + lStartRun;

		ReleaseStyle(  &pStyleUse[ pRun[1].lStyleIndex ] );
		pStyleUse = GetStyle( pStyleUse, pNewStyle,
			pRun[1].lStyleIndex );
		pRun[1].lStartChar = lStartChar;
	}
	else if (pRun[0].lStartChar == lStartChar)
	{

		pRunBase = InsertRuns(  pRunBase, lStartRun, 1 );
		pRun = pRunBase + lStartRun;

		pRun[1].lStartChar = lNextStartChar;
		pStyleUse = GetStyle(  pStyleUse, pNewStyle,
			pRun[0].lStyleIndex );
	}
	else if (pRun[1].lStartChar == lNextStartChar)
	{

		pRunBase = InsertRuns(  pRunBase, lStartRun + 1, 1 );
		pRun = pRunBase + lStartRun;

		pRun[1].lStartChar = lStartChar;
		pStyleUse = GetStyle(  pStyleUse, pNewStyle,
			pRun[1].lStyleIndex );
	}
	else if (pRun[1].lStartChar < lNextStartChar)
	{

		pRunBase = InsertRuns(  pRunBase, lStartRun + 1, 1 );
		pRun = pRunBase + lStartRun;

		pRun[2].lStartChar = lNextStartChar;
		pRun[1].lStartChar = lStartChar;
		pStyleUse = GetStyle( pStyleUse, pNewStyle,
			pRun[1].lStyleIndex );
	}
	else
	{

		pRunBase = InsertRuns( pRunBase, lStartRun + 1, 2 );
		pRun = pRunBase + lStartRun;

		pStyleUse[ pRun[0].lStyleIndex ].lUseCount += 1;

		pRun[2].lStyleIndex = pRun[0].lStyleIndex;
		pRun[2].lStartChar = lNextStartChar;
		pRun[1].lStartChar = lStartChar;
		pStyleUse = GetStyle( pStyleUse, pNewStyle,
			pRun[1].lStyleIndex );
	}

	return (true);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::NewTextObject()

------------------------------------------------------------------------------

	Add a new object
----------------------------------------------------------------------------*/
int ChTxtWnd::NewTextObject( ChTxtObject *pTxtObj )
{
	if ( m_lObjCount >= m_lObjTblSize )
	{
		m_lObjTblSize += objTblGrowSize;
		m_pObjTbl = (ChTextObject**)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY, GetObjectTable(),
							( m_lObjTblSize * sizeof( ChTextObject** )));
	}

	if (!GetObjectTable())
	{
		return 0;
	}

	ChTextObject **ppObject = GetObjectTable() + m_lObjCount;
	*ppObject = pTxtObj->GetTextObject();
	// set the object ID
	(*ppObject)->SetObjectID( (int)m_lObjCount ); 


	m_lObjCount += 1;
	// return index to the object
	return ((int)(m_lObjCount - 1) );

}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::InsertRuns()

------------------------------------------------------------------------------

	Inserts a new run in the run table.
	.
----------------------------------------------------------------------------*/

pChRun ChTxtWnd::InsertRuns(  pChRun pRun, chint32 lStartRun, int sNewCount )
{
	pChRun		pRunBase;


	m_pRunsTbl = (pChRun)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY, GetRunTable(),
						((GetRunCount() + 1 + sNewCount) * sizeof( ChRun )));
	if (!GetRunTable())
	{
		return 0;
	}
	pRunBase = GetRunTable();
	pRun = pRunBase + lStartRun;

	ChMemMove( pRun + sNewCount, pRun,
		(chuint16)((GetRunCount() + 1 - lStartRun) * sizeof( ChRun )));

	m_lRunCount += sNewCount;

	return (pRunBase);
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::DeleteRuns()

------------------------------------------------------------------------------

	Deletes a range of .runs and updates the run table.
	.
----------------------------------------------------------------------------*/

pChRun ChTxtWnd::DeleteRuns(  pChRun pRun, chint32 lStartRun, chint32 lLastChar )
{
	pChStyleInfo	pStyleUse;
	int				iCount;

	pStyleUse = GetStyleTable();
	pRun += lStartRun;

	for (iCount = 0; (lStartRun + iCount < GetRunCount())
		&& (pRun[ iCount + 1 ].lStartChar <= lLastChar);
		iCount += 1)
	{
		ReleaseStyle(  &pStyleUse[ pRun[ iCount ].lStyleIndex ] );
	}
	pRun[ iCount ].lStartChar = lLastChar;

	ChMemCopy( pRun, pRun + iCount,
		(chuint16)(GetRunCount() + 1 - lStartRun - iCount) * sizeof( ChRun ));

	m_lRunCount -= iCount;

	m_pRunsTbl = (pChRun)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetRunTable(),
					(GetRunCount() + 1) * sizeof( ChRun ));

	return (GetRunTable());
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetStyle()

------------------------------------------------------------------------------

	Adds a new entry if the given style is not found, else returns
	the style from the style table.
	.
----------------------------------------------------------------------------*/


pChStyleInfo ChTxtWnd::GetStyle(  pChStyleInfo pStyleUse, pChStyle pNewStyle,
	chint32& lStyleIndex )
{
	chint32			lIndex;
	chint32			lEmptyStyle;


	lEmptyStyle = -1;

	for (lIndex = 0; lIndex < GetStyleCount(); lIndex += 1)
	{
		if (pStyleUse[ lIndex ].lUseCount > 0)
		{
			if (EqualStyle( &pStyleUse[ lIndex ], pNewStyle )) break;
		}
		else if (lEmptyStyle < 0)
		{
			lEmptyStyle = lIndex;
		}
	}
	if (lIndex < GetStyleCount())
	{

		pStyleUse[ lIndex ].lUseCount += 1;
	}
	else if (lEmptyStyle >= 0)
	{
		lIndex = lEmptyStyle;
		NewStyle(  &pStyleUse[ lIndex ], pNewStyle );
	}
	else
	{
		m_lStyleCount += 1;

		m_pStyleTbl = (pChStyleInfo)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY, GetStyleTable(),
				GetStyleCount() * sizeof( ChStyleInfo ));

		m_pStyleTbl = GetStyleTable();

		lIndex = GetStyleCount() - 1;
		NewStyle( &m_pStyleTbl[ lIndex ], pNewStyle );
	}
	lStyleIndex = lIndex;

	return (m_pStyleTbl);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::NewStyle()

------------------------------------------------------------------------------

	Adds a new entry to the style table.
----------------------------------------------------------------------------*/

void ChTxtWnd::NewStyle(  pChStyleInfo pStyleUse, pChStyle pStyle )
{
	ChFont		*pOldFont;
	pChFontInfo	 pFontElem;

	pFontElem = GetFontTable();

	pFontElem += pStyle->iFontIndex;

	pStyleUse->lUseCount = 1;


	pStyleUse->style = *pStyle;

	pOldFont = GetContext()->SelectObject( pFontElem->pFont );

	TEXTMETRIC	textMetric;
	
	GetContext()->GetTextMetrics( &textMetric );
	GetContext()->SelectObject( pOldFont );

	pStyleUse->iFontHeight = textMetric.tmHeight
			+ textMetric.tmExternalLeading;
	pStyleUse->iFontAscent = textMetric.tmAscent;
	pStyleUse->iFontDescent =  textMetric.tmDescent;

	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ReleaseStyle()

------------------------------------------------------------------------------

	Decrement the use count of the font.
----------------------------------------------------------------------------*/

void ChTxtWnd::ReleaseStyle( pChStyleInfo pStyleUse )
{

	if (pStyleUse->lUseCount > 0)
	{
		pStyleUse->lUseCount -= 1;
	}

	if (pStyleUse->lUseCount == 0)
	{
		if ( pStyleUse->style.iFontIndex )
		{  // Release only non-default fonts
			pChFontInfo pFontElem = GetFontTable();

			pFontElem += pStyleUse->style.iFontIndex ;
			ASSERT( pFontElem->iUseCount > 0 );	

		   	pFontElem->iUseCount--;
			if ( pFontElem->iUseCount == 0 && pFontElem->pFont )
			{
				delete pFontElem->pFont;
				pFontElem->pFont = 0;
			}
		}

		if ( pStyleUse->style.luStyle & ChTxtWnd::textDeleteUserData 
					&& 	pStyleUse->style.userData )
		{
			delete[](ptr)pStyleUse->style.userData;
		}
	}

	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::EqualStyle()

------------------------------------------------------------------------------

	Compare two styles and return true if we have a match else false..
----------------------------------------------------------------------------*/

bool ChTxtWnd::EqualStyle( pChStyleInfo pStyleUse, pChStyle pStyle )
{
	if (   ( pStyleUse->style.iFontIndex == pStyle->iFontIndex ) 
		&& ( pStyleUse->style.luStyle  == pStyle->luStyle )
		&& (pStyleUse->style.lColor == pStyle->lColor)
		&& ( pStyleUse->style.iLeftIndent == pStyle->iLeftIndent )
		&& (pStyleUse->style.lBackColor == pStyle->lBackColor)
		&& ( pStyleUse->style.iObjectIndex == pStyle->iObjectIndex )
		&& ( pStyleUse->style.userData == pStyle->userData )
	   )
	{
		return (true);
	}
	else
	{
		return (false);
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::EqualStyle()

------------------------------------------------------------------------------

	Compare two fonts and return true if we have a match else false..

----------------------------------------------------------------------------*/

bool ChTxtWnd::EqualFont( LOGFONT& logFontCache, LOGFONT& logFontNew  )
{
#ifdef CH_MSW

	#if 0
	if ( !ChMemCmp( &logFontCache, &logFontNew, sizeof(LOGFONT) ) )
	{
	 	return true;
	}
	else
	{
		return false;
	}
	#else
	if ( logFontCache.lfHeight != logFontNew.lfHeight ||
		 logFontCache.lfWeight != logFontNew.lfWeight ||
		 logFontCache.lfItalic != logFontNew.lfItalic ||
		 logFontCache.lfOrientation != logFontNew.lfOrientation ||
		 logFontCache.lfUnderline != logFontNew.lfUnderline ||
		 logFontCache.lfStrikeOut != logFontNew.lfStrikeOut ||
		 logFontCache.lfWidth != logFontNew.lfWidth ||
		 lstrcmpi( logFontCache.lfFaceName, logFontNew.lfFaceName) )
	{
		return false;
	}
	else
	{
		return true;
	}

	#endif

#else
//	cerr << "XXX Not really comparing fonts: " << __FILE__ << ":" << __LINE__ << endl;
	return true;
#endif			
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::NewFont()

------------------------------------------------------------------------------

	Adds a new entry to the font table.
----------------------------------------------------------------------------*/

void ChTxtWnd::NewFont(  pChFontInfo pFontElem )
{

	static int iCount  = 0;

	pFontElem->iUseCount = 1;

	pFontElem->pFont = new ChFont;

	ASSERT( pFontElem->pFont );

	pFontElem->pFont->CreateFontIndirect( &pFontElem->fontInfo );

	return;
}


int ChTxtWnd::GetFont( ChFont* pFont )
{
	if ( !pFont )
	{ // no font use the default
		// Index of default font is 0
		return 0;
	}
	int			iEmptyFont = -1;

	// get the font information
	LOGFONT 	logFont;
	pFont->GetObject( sizeof(logFont), &logFont );


	for ( int iIndex = 0; iIndex < GetFontCount(); iIndex += 1)
	{
		if (m_pFontTbl[ iIndex ].iUseCount > 0 )
		{
			if (EqualFont( m_pFontTbl[ iIndex ].fontInfo, logFont )) break;
		}
		else if (iEmptyFont < 0)
		{
			iEmptyFont = iIndex;
		}
	}

	if (iIndex < GetFontCount())
	{

		m_pFontTbl[ iIndex ].iUseCount += 1;
	}
	else if (iEmptyFont > 0)
	{
		iIndex = iEmptyFont;
		m_pFontTbl[iIndex].fontInfo = logFont;
		NewFont(  &m_pFontTbl[iIndex] );
	}
	else
	{
	
		iIndex = m_lFontCount;

		m_lFontCount += fontTblGrowSize;

		m_pFontTbl = (pChFontInfo)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY, m_pFontTbl,
				m_lFontCount * sizeof( ChFontInfo ) );

		ASSERT( m_pFontTbl );

		m_pFontTbl[iIndex].fontInfo = logFont;
		NewFont(  &m_pFontTbl[iIndex] );
	}

	// Just in case we failed to create a new font then 
	// lets default to the system font
	if ( !m_pFontTbl[iIndex].pFont->GetSafeHandle() )
	{
		iIndex = 0;
	}
	return iIndex;
}



/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::ClearAll()

------------------------------------------------------------------------------

	Cleanup all the tables.
----------------------------------------------------------------------------*/

bool ChTxtWnd::ClearAll( )
{
	int 			i;

	// clear the selection if any
	m_lSelStart = m_lSelEnd = 0;
	m_boolInSelMode = false;
	m_boolRedrawAll = false;



	m_lTextCount = 0;
	m_lStartUpdate = 0;
	m_lTextBlkSize = bufferGrowSize;
	m_pstrText = (pstr)HeapReAlloc( GetHeap(), HEAP_ZERO_MEMORY, GetTextBuffer(),
							GetTextBufferSize());

	if (! GetTextBuffer())
		return (false);

	ChMemClear( GetTextBuffer(),  GetTextBufferSize());

	m_lRunCount = 0;
	m_pRunsTbl = (pChRun)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetRunTable(),
						(chint32) (2 * sizeof( ChRun )));

	if (! GetRunTable())
		return (false);
	ChMemClear( GetRunTable(),  HeapSize(GetHeap(), 0, GetRunTable()));

	m_lLineCount = 0;
	m_pLinesTbl = (pChLine)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetLineTable(),
					(chint32)(2 * sizeof( ChLine )));
	if (! GetLineTable())
		return (false);
	ChMemClear( GetRunTable(), HeapSize(GetHeap(), 0, GetRunTable()));

	// do object table cleanup
	for (i = 0; i < (int)GetObjectCount(); i++)
	{
		delete m_pObjTbl[i];
	}


	m_lObjCount = 0;
	m_lObjTblSize = objTblGrowSize;
	m_pObjTbl = (ChTextObject**)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetObjectTable(),
					(chint32)(m_lObjTblSize * sizeof( ChTextObject* )));
	
	if (!GetObjectTable())
		return (false);
	
	ChMemClear( GetObjectTable(), HeapSize(GetHeap(), 0, GetObjectTable()));



	/* Delete all the fonts except the default which is at index 0 */
	pChFontInfo pFontTbl = GetFontTable();
	// Clear the font table
	for (i = 1; i < (int)GetFontCount(); i++)
	{  // delete only if it is in use, other wise we have
	   // already deleted the style in ReleaseStyle
		if ( pFontTbl[i].iUseCount > 0 )
		{
			delete pFontTbl[i].pFont;
			pFontTbl[i].iUseCount = 0;
			pFontTbl[i].pFont = 0;
		}
	}

	m_lFontCount = fontTblGrowSize;  // default faont is always at zero index, do not delete it
	m_pFontTbl = (pChFontInfo)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetFontTable(),
						(chint32) sizeof( ChFontInfo ) * m_lFontCount );
	if (! m_pFontTbl )
		return (false);


	/* Delete all the styles */
	pChStyleInfo 	pStyle;
	pStyle = GetStyleTable();

	for (i = 0; i < (int)GetStyleCount(); i++)
	{  // delete only if it is in use, other wise we have
	   // already deleted the style in ReleaseStyle
		if ( pStyle->lUseCount > 0 )
		{
			// delete any user data associated
			if ( pStyle->style.luStyle & ChTxtWnd::textDeleteUserData 
						&& 	pStyle->style.userData )
			{
				delete[](ptr)pStyle->style.userData;
			}
		}
		pStyle++;
	}

	m_lStyleCount = 1;
	m_pStyleTbl = (pChStyleInfo)HeapReAlloc(GetHeap(), HEAP_ZERO_MEMORY, GetStyleTable(),
						(chint32) sizeof( ChStyleInfo ));
	if (! GetStyleTable())
		return (false);
	ChMemClear( GetStyleTable(), HeapSize(GetHeap(), 0, GetStyleTable()));

	return (true);
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetDrawCharCount()

------------------------------------------------------------------------------

Returns total number of drawable characters.
----------------------------------------------------------------------------*/


int ChTxtWnd::GetDrawCharCount( pstr pstrText, int sCharCount )
{
	if ((sCharCount > 0) && (pstrText[ sCharCount - 1 ] == CH_TXTVW_NEWLINE))
	{
		sCharCount -= 1;
	}
	return (sCharCount);
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChTxtWnd::GetWhiteSpaceCount()

------------------------------------------------------------------------------

Returns total number whitespaces.
----------------------------------------------------------------------------*/

int ChTxtWnd::GetWhiteSpaceCount( pstr pstrText, int sCharCount )
{
	int		iWhiteCharCount;

	iWhiteCharCount = 0;
	pstrText += sCharCount - 1;

	while ((iWhiteCharCount < sCharCount) && ((*pstrText == TEXT(' '))
		|| (*pstrText == CH_TXTVW_TAB)))
	{
		iWhiteCharCount += 1;
		pstrText -= 1;
	}
	return (iWhiteCharCount);
}


#if ( defined( CH_MSW ) && defined( CH_ARCH_16 ) ) || ( defined( CH_UNIX ) )

CH_GLOBAL_FUNC( HANDLE )
HeapCreate( int a, int b, int c )
{
    return (HANDLE)1;
}

CH_GLOBAL_FUNC(void *)
HeapAlloc( HANDLE hHeap, chuint32 flags, chuint32 luLen )
{   
	void *pBlk = new char[luLen + sizeof( chuint32 )];     
	ASSERT( pBlk );
	// store the size in our header
	chuint32 *pSize = (chuint32 *)pBlk;
	*pSize = luLen;
	pBlk = pSize + 1;      
	
	if ( flags & HEAP_ZERO_MEMORY )
	{     
		ChMemClear(  pBlk, luLen );
	}
	
    return ( pBlk );
}

CH_GLOBAL_FUNC( int )
HeapDestroy( HANDLE hHeap )
{
    return 0;
}

CH_GLOBAL_FUNC(void *)
HeapReAlloc( HANDLE hHeap, chuint32 flags, void *oldbuf, chuint32 luNewSize )
{

 	chuint32 *pOldSize = (chuint32 *)oldbuf;   
 	chuint32 luOldSize;
 	
 	pOldSize--;
	luOldSize = *pOldSize;

 	void *pBlk = new char[luNewSize + sizeof( chuint32 )];     
	ASSERT( pBlk );
	// store the size in 
	// our header
	chuint32 *pNewSize = (chuint32 *)pBlk;
	*pNewSize = luNewSize;
	pBlk = pNewSize + 1;
	// If we're HeapRealloc()ing a NULL pointer, don't do the copy.
	if (oldbuf != NULL) {
		ChMemCopy( pBlk, pOldSize + 1, (( luOldSize < luNewSize) ? luOldSize : luNewSize)); 

		oldbuf = pOldSize;  
		delete []oldbuf;
		
		if ( flags & HEAP_ZERO_MEMORY && luNewSize > luOldSize )
		{     
			ChMemClear(  (char*)pBlk + luOldSize, (luNewSize - luOldSize) );
		}
		
	}

    return ( pBlk );
}

CH_GLOBAL_FUNC(void )
HeapFree( HANDLE hHeap, chuint32 flags, void *buf )
{
	chuint32 *pSize = (chuint32 *)buf;
	buf = pSize - 1;

	if (pSize != NULL)
		delete[] buf;
}

CH_GLOBAL_FUNC(chuint32 )
HeapSize( HANDLE hHeap, chuint32 flags, void *buf )
{
	chuint32 *pSize = (chuint32 *)buf;
    return *(pSize - 1);
}

#endif
