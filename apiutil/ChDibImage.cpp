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

		ChDib class implementation

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChImgUtil.h>
#include <ChDibImage.h>
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Win32s works better if you use GlobalAlloc for large memory
// blocks so the CWave and ChDib classes use the ALLOC and FREE
// macros defined here so you can optionally use either
// malloc (for pure 32 bit platforms) or GlobalAlloc if you
// want the app to run on Win32s; for now we are
// using Win16, not 32s

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#define USE_GLOBALALLOC 1
#endif

#ifdef USE_GLOBALALLOC

   #define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))   
                
   #define     GlobalUnlockPtr(lp) 	\
                GlobalUnlock(GlobalPtrHandle(lp))

   #define ALLOC(s)      (GlobalLock(GlobalAlloc((GHND), (s))))
   
   #define FREE(p)       (GlobalUnlockPtr(p), (BOOL)GlobalFree(GlobalPtrHandle(p)))

#else

#if 0
	// Use these to enable MFC's heap checking for debugging or leak plugging
    #define ALLOC(s) ((void*)(new char[(s)]))
    #define FREE(p) (delete [] (char*)(p))
#else
    #define ALLOC(s) malloc(s)
    #define FREE(p) free(p)
#endif
#endif


/////////////////////////////////////////////////////////////////////////////
// ChDib


// Create a small DIB here so m_pBMI and m_pBits are always valid.
ChDib::ChDib()
{
	ChMemClearStruct( &m_imgInfo );
	m_imgInfo.iWidth = m_imgInfo.iHeight = 0;

	m_pFrameList = new ChDibFrame;	
	ASSERT( m_pFrameList );
	ChMemClearStruct( m_pFrameList );

	m_iCurrentFrame = 0;
	m_iNumFrames = 1;
	m_iFrameListSize = 1;

    Create( m_iCurrentFrame, 16, 16, 8 );
}

ChDib::~ChDib()
{
	for ( int i = 0; i < m_iNumFrames; i++ )
	{
	    // Free the memory.
	    if (m_pFrameList[i].m_pBMI != NULL) FREE(m_pFrameList[i].m_pBMI);
	    if (m_pFrameList[i].m_bMyBits && (m_pFrameList[i].m_pBits != NULL)) FREE(m_pFrameList[i].m_pBits);
		if ( m_pFrameList[i].m_pPalette ) delete m_pFrameList[i].m_pPalette;

		delete m_pFrameList[i].m_pTransDib;
	}

	delete []m_pFrameList;
}

void ChDib::AllocateFrame( int iFrame )
{
	m_iNumFrames = iFrame + 1;

	if ( iFrame < m_iFrameListSize )
	{
		return;
	}
	else
	{

		int iOldSize = m_iFrameListSize;
		// bump by 2
		m_iFrameListSize += 2;

		if ( iFrame >= m_iFrameListSize )
		{  // some middle frame being initialized
			m_iFrameListSize = iFrame + 1;	
		} 

		ChDibFrame* pFrameList = new ChDibFrame[m_iFrameListSize];	
		ASSERT( pFrameList );
		ChMemCopy( pFrameList, m_pFrameList, sizeof( ChDibFrame ) * iOldSize );
		// clear the newly allocated frames
		ChMemClear( &pFrameList[iOldSize], (m_iFrameListSize - iOldSize) * sizeof( ChDibFrame ) );

		delete[] m_pFrameList;
		m_pFrameList = pFrameList; 

	}

}

void ChDib::NextFrame()
{
	m_iCurrentFrame++;
	if ( m_iCurrentFrame >= m_iNumFrames )
	{
		m_iCurrentFrame = 0;		
	}
}
void ChDib::SetFrame( int iFrame )
{
	m_iCurrentFrame = iFrame;
	if ( m_iCurrentFrame >= m_iNumFrames )
	{
		m_iCurrentFrame = m_iNumFrames - 1;		
	}
}

long ChDib::StorageWidth( int iFrame /*= -1 */ )
{	
	int iWidth;
	if ( iFrame == -1 )
	{
		iFrame = m_iCurrentFrame;	
	}
	ASSERT( iFrame < m_iNumFrames );
	 
	BITMAPINFOHEADER& bmiHeader = m_pFrameList[iFrame].m_pBMI->bmiHeader;
	switch ( bmiHeader.biBitCount ) 
  {
    case 1:
			iWidth = bmiHeader.biWidth / 8;
      break;
    case 4:
			iWidth = bmiHeader.biWidth >> 1;
      break;
    case 8:
			iWidth = bmiHeader.biWidth;
      break;
 	 	case 24 :
			iWidth = bmiHeader.biWidth * 3;
			break;
		case 32 :
			iWidth = bmiHeader.biWidth << 2;
			break;
    default:
        iWidth = 0;
        break;
  }
	iWidth = (iWidth + 3) & ~3;		// round up to DWORD boundary
	return ( (iWidth == 0) ? 4 : iWidth );
}



/////////////////////////////////////////////////////////////////////////////
// ChDib commands

// Create a new empty 8bpp DIB with a 256 entry color table.
bool ChDib::Create( int iFrame, int iWidth, int iHeight, int iBitCount /*= 8 */ )
{
    // Delete any existing stuff.
    if ( m_pFrameList[iFrame].m_pBMI != NULL) FREE( m_pFrameList[iFrame].m_pBMI);
    if ( m_pFrameList[iFrame].m_bMyBits 
    		&& ( m_pFrameList[iFrame].m_pBits != NULL)) 
    		FREE( m_pFrameList[iFrame].m_pBits);

    // Allocate memory for the header.
     m_pFrameList[iFrame].m_pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD));
    if (! m_pFrameList[iFrame].m_pBMI) {
        TRACE("Out of memory for DIB header\n");
        return false;
    }

    // Allocate memory for the bits (DWORD aligned).
    long iBitsSize = (((iWidth * iBitCount/8 ) + 3) & ~3) * iHeight;
	if ( iBitsSize == 0 )
	{
		iBitsSize = 4;
	}
     m_pFrameList[iFrame].m_pBits = (BYTE*)ALLOC(iBitsSize);
    if (! m_pFrameList[iFrame].m_pBits) {
        TRACE("Out of memory for DIB bits\n");
        FREE( m_pFrameList[iFrame].m_pBMI);
         m_pFrameList[iFrame].m_pBMI = NULL;
        return false;
    }
     m_pFrameList[iFrame].m_bMyBits = true;

    // Fill in the header info.
    BITMAPINFOHEADER* pBI = (BITMAPINFOHEADER*) m_pFrameList[iFrame].m_pBMI;
    pBI->biSize = sizeof(BITMAPINFOHEADER);
    pBI->biWidth = iWidth;
    pBI->biHeight = iHeight;
    pBI->biPlanes = 1;
    pBI->biBitCount = iBitCount;
    pBI->biCompression = BI_RGB;
    pBI->biSizeImage = 0;
    pBI->biXPelsPerMeter = 0;
    pBI->biYPelsPerMeter = 0;
    pBI->biClrUsed = 0;
    pBI->biClrImportant = 0;

	if ( iBitCount < 24 )
	{
	    // Create an arbitrary color table (gray scale).
	    RGBQUAD* prgb = GetClrTabAddress( iFrame );
	    for (int i = 0; i < 256; i++) {
	        prgb->rgbBlue = prgb->rgbGreen = prgb->rgbRed = (BYTE) i;
	        prgb->rgbReserved = 0;
	        prgb++;
	    }
	}

    // Set all the bits to a known state (black).  
    memset( m_pFrameList[iFrame].m_pBits, 0, iBitsSize);

	m_pFrameList[iFrame].m_pMask = (void*)0xFFFFFFFF;
    m_pFrameList[iFrame].m_bMyBits = true;

    return true;
}

// Create a ChDib structure from existing header and bits. The DIB
// won't delete the bits and makes a copy of the header.
bool ChDib::Create( int iFrame, BITMAPINFO* pBMI, BYTE* pBits)
{
    ASSERT( pBMI);
    ASSERT(pBits);
 	// Allocate new frame if required
	AllocateFrame( iFrame );


	m_imgInfo.iWidth  = pBMI->bmiHeader.biWidth;
	m_imgInfo.iHeight  = pBMI->bmiHeader.biHeight;

	m_pFrameList[iFrame].m_frameInfo.iWidth  = pBMI->bmiHeader.biWidth;
	m_pFrameList[iFrame].m_frameInfo.iHeight  = pBMI->bmiHeader.biHeight;
    
    if (m_pFrameList[iFrame].m_pBMI != NULL) FREE(m_pFrameList[iFrame].m_pBMI);
    	m_pFrameList[iFrame].m_pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                   + 256 * sizeof(RGBQUAD));
    ASSERT(m_pFrameList[iFrame].m_pBMI);
    // Note: This will probably fail for < 256 color headers.
    memcpy( m_pFrameList[iFrame].m_pBMI, pBMI, sizeof(BITMAPINFOHEADER)+
             ChImgUtil::NumDIBColorEntries(pBMI) * sizeof(RGBQUAD));

    if (m_pFrameList[iFrame].m_bMyBits 
    			&& (m_pFrameList[iFrame].m_pBits != NULL)) 
    	FREE(m_pFrameList[iFrame].m_pBits);

    m_pFrameList[iFrame].m_pBits = pBits;
    m_pFrameList[iFrame].m_bMyBits = false; // We can't delete the bits.
	m_pFrameList[iFrame].m_pMask = (void*)0xFFFFFFFF;
    return true;
}



// Draw the DIB to a given DC.
void ChDib::Draw(CDC* pDC, int x, int y)
{

	CPalette *pOldPal = 0;
	
	if ( GetDIBPalette() )
	{
		pOldPal = pDC->SelectPalette( GetDIBPalette(), TRUE );
		pDC->RealizePalette();
	}  

    ::StretchDIBits(pDC->GetSafeHdc(),
                    x + m_pFrameList[m_iCurrentFrame].
                    m_frameInfo.iLeft,         // Destination x
                    y +  m_pFrameList[m_iCurrentFrame].
                    m_frameInfo.iTop,          // Destination y
                    (int)DibWidth(),          // Destination width
                    (int)DibHeight(),         // Destination height
                    0,                        // Source x
                    0,                        // Source y
                    (int)DibWidth(),          // Source width
                    (int)DibHeight(),         // Source height
                    GetBitsAddress(),         // Pointer to bits
                    GetBitmapInfoAddress(),   // BITMAPINFO
                    DIB_RGB_COLORS,           // Options
                    SRCCOPY);                 // Raster operation code (ROP)

	if ( pOldPal )
	{
		pDC->SelectPalette( pOldPal, FALSE );
	}
}


BOOL ChDib::SetSize( long lWidth, long lHeight )
{
	if ( lWidth == 0 || lHeight == 0 )
	{
		return FALSE;
	}

	if ( lWidth == GetWidth() && lHeight == GetHeight() )
	{
		return TRUE;
	} 
	// Size the DIB
	LPBITMAPINFO pBmpInfo = GetBitmapInfoAddress();
	int iBitCount = pBmpInfo->bmiHeader.biBitCount == 24 ? 24 : 8;


    // Allocate memory for the header.
    BITMAPINFO * pBMI;

	if ( iBitCount < 24 )
	{
		pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD));	}
	else
	{
		pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER));	
	}
    
    if (!pBMI) 
    {
        TRACE("Out of memory for DIB header\n");
        return false;
    }



    // Allocate memory for the bits (DWORD aligned).
    long iBitsSize = (((lWidth * iBitCount/8 ) + 3) & ~3) * lHeight;
    BYTE *pBits = (BYTE*)ALLOC(iBitsSize);
    if (!pBits) 
    {
        TRACE("Out of memory for DIB bits\n");
        FREE(pBMI);
        return false;
    }

	if ( iBitCount < 24 )
	{
		ChMemCopy( pBMI, m_pFrameList[m_iCurrentFrame].m_pBMI,  sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD) );
	}
	else
	{
		ChMemCopy( pBMI, m_pFrameList[m_iCurrentFrame].m_pBMI,  
									sizeof(BITMAPINFOHEADER) );
	}
	// change the size 
	pBMI->bmiHeader.biWidth = lWidth;
	pBMI->bmiHeader.biHeight = lHeight;


	
	HPALETTE hOldPal  = 0;
	HPALETTE hPalette = 0;
	CPalette *pPal;
	
	// Copy the color table if we are 8 bit
	if ( pBmpInfo->bmiHeader.biBitCount == 8 )
	{
	    pPal = GetDIBPalette();
		hPalette = (HPALETTE)pPal->GetSafeHandle();
	}
	else
	{
		if ( IsGrayscale() )
		{
		    pPal = ChImgUtil::GetGrayScalePalette();
		}
		else
		{
		    pPal = ChImgUtil::GetStdPalette();
		}
		if ( pPal )
		{
			hPalette = (HPALETTE)pPal->GetSafeHandle();
		}
	}


	HWND hWnd = ::GetDesktopWindow();
	HDC  hDC  = ::GetDC( hWnd );
	HDC  hDCTmp	= ::CreateCompatibleDC( hDC );

	if ( hPalette )
	{
		hOldPal = ::SelectPalette( hDCTmp, hPalette, true );
		::RealizePalette( hDCTmp );
	}

	HBITMAP hBmpTmp = ::CreateDIBitmap( hDC, 
									  &pBMI->bmiHeader,
									  CBM_INIT,
									  pBits,
									  pBMI,
									  DIB_RGB_COLORS );

	HBITMAP hOldBmp;

	hOldBmp = (HBITMAP)::SelectObject( hDCTmp, hBmpTmp );

	::SetStretchBltMode( hDCTmp,			// handle of device context  
				    	 COLORONCOLOR	 	// bitmap stretching mode 
				     	);

    ::StretchDIBits( hDCTmp, 0,                  // Destination x
                    0,                        	// Destination y
                    (int)lWidth,               	// Destination width
                    (int)lHeight,              	// Destination height
                    0,                        	// Source x
                    0,                        	// Source y
                    (int)GetWidth(),    // Source width
                    (int)GetHeight(),   // Source height
					GetBitsAddress(),
					GetBitmapInfoAddress(),
					DIB_RGB_COLORS,
					SRCCOPY);



	::GetDIBits( hDCTmp, hBmpTmp, 0, (int)lHeight, 
						pBits,
						pBMI,
						DIB_RGB_COLORS );

	if ( hOldPal )
	{
		::SelectPalette( hDCTmp, hOldPal, false );
	}

	::SelectObject( hDCTmp, hOldBmp );
	::DeleteObject( hBmpTmp );
	::DeleteDC( hDCTmp );
	::ReleaseDC( hWnd, hDC );

    if (m_pFrameList[m_iCurrentFrame].m_pBMI != NULL) FREE(m_pFrameList[m_iCurrentFrame].m_pBMI);
    if (m_pFrameList[m_iCurrentFrame].m_bMyBits 
    			&& (m_pFrameList[m_iCurrentFrame].m_pBits != NULL)) 
    		FREE(m_pFrameList[m_iCurrentFrame].m_pBits);

	m_pFrameList[m_iCurrentFrame].m_pBMI = pBMI;
	m_pFrameList[m_iCurrentFrame].m_pBits = pBits;
	m_pFrameList[m_iCurrentFrame].m_bMyBits = true;

	return true;
}

// Get a pointer to a pixel.
// NOTE: DIB scan lines are DWORD aligned. The scan line 
// storage width may be wider than the scan line image width
// so calc the storage width by rounding the image width 
// to the next highest DWORD value.
void* ChDib::GetPixelAddress(int x, int y)
{
    int iWidth;
    // Note: This version deals only with 8 and 24 bpp DIBs.
    //ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    // Make sure it's in range and if it isn't return zero.
    
    if ((x >= DibWidth()) 
    || (y >= DibHeight())) {
        TRACE("Attempt to get out of range pixel address\n");
        return NULL;
    }

 	iWidth = (int)StorageWidth( m_iCurrentFrame );
   // Calculate the scan line storage width.
   switch( m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount )
   {
   	case 1 :
		{
	    	return m_pFrameList[m_iCurrentFrame].m_pBits 
	    				+ (DibHeight()-y-1) * iWidth + (x/8);
		}
   	case 4 :
		{
	    	return m_pFrameList[m_iCurrentFrame].m_pBits 
	    				+ (DibHeight()-y-1) * iWidth + (x/4);
		}
   	case 8 :
		{
	    	return m_pFrameList[m_iCurrentFrame].m_pBits 
	    					+ (DibHeight()-y-1) * iWidth + x;
		}
   	case 24 :
		{
	    	return m_pFrameList[m_iCurrentFrame].m_pBits 
	    					+ (DibHeight()-y-1) * iWidth + (x * 3);
		}
		case 32 :
		{
	    	return m_pFrameList[m_iCurrentFrame].m_pBits 
	    					+ (DibHeight()-y-1) * iWidth + (x * 4);
		}
   	default :
		{
			return NULL;
		}
	}

}


// Get the number of color table entries.
int ChDib::GetNumClrEntries()
{
    return ChImgUtil::NumDIBColorEntries(m_pFrameList[m_iCurrentFrame].m_pBMI);
}

// NOTE: This assumes all ChDib objects have 256 color table entries.
BOOL ChDib::MapColorsToPalette(CPalette *pPal)
{
    if (!pPal) {
        TRACE("No palette to map to\n");
        return false;
    }
    ASSERT(m_pFrameList[m_iCurrentFrame].m_pBMI);
    ASSERT(m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biBitCount == 8);
    ASSERT(m_pFrameList[m_iCurrentFrame].m_pBits);
    LPRGBQUAD pctThis = GetClrTabAddress( m_iCurrentFrame );
    ASSERT(pctThis);
    // Build an index translation table to map this DIBs colors
    // to those of the reference DIB.
    BYTE imap[256];
    int iChanged = 0; // For debugging only
    for (int i = 0; i < 256; i++) {
        imap[i] = (BYTE) pPal->GetNearestPaletteIndex(
                            RGB(pctThis->rgbRed,
                                pctThis->rgbGreen,
                                pctThis->rgbBlue));
        pctThis++;
        if (imap[i] != i) iChanged++; // For debugging
    }
    // Now map the DIB bits.
    BYTE* pBits = (BYTE*)GetBitsAddress();
    DWORD iSize = StorageWidth( m_iCurrentFrame ) * DibHeight();
    while (iSize--) {
        *pBits = imap[*pBits];
        pBits++;
    }
    // Now reset the DIB color table so that its RGB values match
    // those in the palette.
    PALETTEENTRY pe[256];
    pPal->GetPaletteEntries(0, 256, pe);
    pctThis = GetClrTabAddress( m_iCurrentFrame );
    for (i = 0; i < 256; i++) {
        pctThis->rgbRed = pe[i].peRed;    
        pctThis->rgbGreen = pe[i].peGreen;    
        pctThis->rgbBlue = pe[i].peBlue;
        pctThis++;    
    }
    // Now say all the colors are in use
    m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biClrUsed = 256;
    return true;
}


CPalette*	ChDib::GetDIBPalette()
{
	if ( m_pFrameList[m_iCurrentFrame].
				m_pBMI->bmiHeader.biBitCount < 16 
				&& m_pFrameList[m_iCurrentFrame].m_pPalette == 0 )
	{ // Create a palette based on the DIB color table
		RGBQUAD* pClrIn = GetClrTabAddress( m_iCurrentFrame );

		LPLOGPALETTE lpPalette =(LPLOGPALETTE)new char[ sizeof( LOGPALETTE ) + 
									( sizeof( PALETTEENTRY ) *  GetNumClrEntries() )];

		lpPalette->palVersion = 0x300;
		lpPalette->palNumEntries = GetNumClrEntries();

		for ( int i = 0;  i < GetNumClrEntries(); i++ )
		{
			lpPalette->palPalEntry[i].peRed 	=  pClrIn[i].rgbRed;
			lpPalette->palPalEntry[i].peGreen 	=  pClrIn[i].rgbGreen;
			lpPalette->palPalEntry[i].peBlue 	=  pClrIn[i].rgbBlue;
			lpPalette->palPalEntry[i].peFlags = 0;
		}
		m_pFrameList[m_iCurrentFrame].m_pPalette = new CPalette;
		ASSERT( m_pFrameList[m_iCurrentFrame].m_pPalette );
		m_pFrameList[m_iCurrentFrame].m_pPalette->CreatePalette( lpPalette );
		delete [] lpPalette;
	}

	return m_pFrameList[m_iCurrentFrame].m_pPalette;
}


// Copy a rectangle of the DIB to another DIB.
// Note: We only support 8bpp DIBs here.
void ChDib::CopyBits(ChDib* pdibDest, 
                    int xd, int yd,
                    int w,  int h,
                    int xs, int ys,
                    COLORREF clrTrans)
{
    //ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    ASSERT(pdibDest);
    // Test for silly cases.
    if (w == 0 || h == 0) return;

    // Get pointers to the start points in the source and destination
    // DIBs. Note that the start points will be the bottom-left
    // corner of the DIBs because the scan lines are reversed in memory.
    BYTE* pSrc = (BYTE*)GetPixelAddress(xs, ys + h - 1);
    ASSERT(pSrc);
    BYTE* pDest = (BYTE*)pdibDest->GetPixelAddress(xd, yd + h - 1);
    ASSERT(pDest);

    // Get the scan line widths of each DIB.
    int iScanS = (int)StorageWidth( );
    int iScanD = (int)pdibDest->StorageWidth();

    if((clrTrans & 0xFF000000) == 0x01000000)
    {
        // Copy lines with transparency.
        // Note: We accept only a PALETTEINDEX description
        // for the color definition.
        //ASSERT((clrTrans & 0xFF000000) == 0x01000000);
        BYTE bTransClr = LOBYTE(LOWORD(clrTrans));
        int iSinc = iScanS - w; // Source increment value
        int iDinc = iScanD - w; // Destination increment value
        int iCount;
        BYTE pixel;
        while (h--) {
            iCount = w;    // Number of pixels to scan.
            while (iCount--) {
                pixel = *pSrc++;
                // Copy pixel only if it isn't transparent.
                if (pixel != bTransClr) {
                    *pDest++ = pixel;
                } else {
                    pDest++;
                }
            }
            // Move on to the next line.
            pSrc += iSinc;
            pDest += iDinc;
        }
    }
	else if ( pdibDest->GetBitmapInfoAddress()->bmiHeader.biBitCount == 8 &&
						GetBitmapInfoAddress()->bmiHeader.biBitCount == 8)
	{  // copy

		
		// find the index which is transperent
		int iIndex = m_pFrameList[m_iCurrentFrame].m_frameInfo.iTransparentIndex;
	    RGBQUAD*  prgb = GetClrTabAddress( m_iCurrentFrame );

		
        if ( !( prgb[iIndex].rgbBlue == GetBValue( clrTrans )
        	&& prgb[iIndex].rgbGreen == GetGValue( clrTrans )
        	&& prgb[iIndex].rgbRed ==  GetRValue( clrTrans ) ) )
		{
			iIndex = -1;
			for ( int i = 0; i < 256; i++ )
			{
		        if ( prgb->rgbBlue == GetBValue( clrTrans )
		        	&& prgb->rgbGreen == GetGValue( clrTrans )
		        	&& prgb->rgbRed ==  GetRValue( clrTrans ) )
				{
					iIndex = i;
					break;
				}
		        prgb++;
			}
		}

		if ( iIndex == -1 )
		{
		    while (h--) 
	        {
	            ChMemCopy(pDest, pSrc, w);
	            pSrc += iScanS;
	            pDest += iScanD;
	        }

		}
		else
		{

			BYTE imapSrc[256];
			BYTE imapBits[256];
			BYTE imapDst[256];
			ChMemClear( imapSrc, sizeof( imapSrc ) );
			ChMemClear( imapDst, sizeof( imapDst ) );
			ChMemClear( imapBits, sizeof( imapBits ) );
			{  	// Find number of colors used and mark the entries which are used
			    BYTE* pBits = (BYTE*)GetBitsAddress();
			    DWORD iSize = StorageWidth() * DibHeight();
			    while (iSize--)
			    {
			        imapSrc[*pBits] = true;
					imapBits[*pBits] = *pBits;
			        pBits++;
			    }
				imapSrc[iIndex] = 0; // do not map the transparent bit
			}
			{  // find the entries used in the destination
			    BYTE* pBits = (BYTE*)pdibDest->GetBitsAddress();
			    DWORD iSize = pdibDest->StorageWidth() * pdibDest->DibHeight();
			    while (iSize--)
			    {
			        imapDst[*pBits] = true;
			        pBits++;
			    }
			}
			// Map the colors of the source to destination and
			// remap if entry is used by the destination
			RGBQUAD * pSrcClrTbl = GetClrTabAddress( m_iCurrentFrame );
			RGBQUAD * pDstClrTbl = pdibDest->GetClrTabAddress( pdibDest->GetCurrentFrame() );
			for ( int i = 0; i < 256; i++ )
			{
				if ( imapSrc[i] )
				{  // source uses this color
	
					if ( imapDst[i] == 0 )
					{ // not used, remap, destination is not using
						pDstClrTbl[i] =  pSrcClrTbl[i];
					}
					else if (  pDstClrTbl[i].rgbRed != pSrcClrTbl[i].rgbRed 
                               || pDstClrTbl[i].rgbGreen != pSrcClrTbl[i].rgbGreen 
                               || pDstClrTbl[i].rgbBlue != pSrcClrTbl[i].rgbBlue )
					{ // source != dst, find a free entry   
						int j = 16;
						while ( j < 256 && imapDst[j] )
						{
							j++;
						}

						if ( j < 256 )
						{
							pDstClrTbl[j] =  pSrcClrTbl[i];
							imapBits[i] = j;
							imapDst[j] = true; // cannot use this any more
						}
					}
				}
			}

			delete pdibDest->m_pFrameList[pdibDest->GetCurrentFrame()].m_pPalette;
			pdibDest->m_pFrameList[pdibDest->GetCurrentFrame()].m_pPalette = 0;

	        BYTE bTransClr = (BYTE) iIndex;
	        int iSinc = iScanS - w; // Source increment value
	        int iDinc = iScanD - w; // Destination increment value
	        int iCount;
	        BYTE pixel;

	        while (h--) 
	        {
	            iCount = w;    // Number of pixels to scan.
	            while (iCount--) 
	            {
	                pixel = *pSrc++;

	                // Copy pixel only if it isn't transparent.
	                if (pixel != bTransClr) 
	                {
	                    *pDest++ = imapBits[pixel];
	                } 
	                else 
	                {
	                    pDest++;
	                }
	            }
	            // Move on to the next line.
	            pSrc += iSinc;
	            pDest += iDinc;
	        }
		}
	}
	// UE TODO: 32-bit images?
	else if ( pdibDest->GetBitmapInfoAddress()->bmiHeader.biBitCount == 24 &&
						GetBitmapInfoAddress()->bmiHeader.biBitCount == 24 )
	{  // copy

        int iSinc = iScanS - ( w * 3); // Source increment value
        int iDinc = iScanD - ( w * 3); // Destination increment value
        int iCount;

        while (h--) 
        {
            iCount = w;    // Number of pixels to scan.
            while (iCount--) 
            {
                // Copy pixel only if it isn't transparent.
		        if ( pSrc[0] != GetRValue( clrTrans )
		        	|| pSrc[1] != GetGValue( clrTrans )
		        	|| pSrc[2] !=  GetBValue( clrTrans ) )
				{
                    *pDest++ = *pSrc++;
                    *pDest++ = *pSrc++;
                    *pDest++ = *pSrc++;
				}
				else
				{
                    pDest += 3;
                    pSrc  += 3;
				}
            }
            // Move on to the next line.
            pSrc += iSinc;
            pDest += iDinc;
        }

	}
	else if ( pdibDest->GetBitmapInfoAddress()->bmiHeader.biBitCount == 24 &&
						GetBitmapInfoAddress()->bmiHeader.biBitCount == 8 )
	{

		// find the index which is transperent

		int iIndex = m_pFrameList[m_iCurrentFrame].m_frameInfo.iTransparentIndex;
	    RGBQUAD*  prgb = GetClrTabAddress( m_iCurrentFrame );

		
        if ( !( prgb[iIndex].rgbBlue == GetBValue( clrTrans )
        	&& prgb[iIndex].rgbGreen == GetGValue( clrTrans )
        	&& prgb[iIndex].rgbRed ==  GetRValue( clrTrans ) ) )
		{
			iIndex = -1;
			for ( int i = 0; i < 256; i++ )
			{
		        if ( prgb->rgbBlue == GetBValue( clrTrans )
		        	&& prgb->rgbGreen == GetGValue( clrTrans )
		        	&& prgb->rgbRed ==  GetRValue( clrTrans ) )
				{
					iIndex = i;
					break;
				}
		        prgb++;
			}
		}

	    prgb = GetClrTabAddress( m_iCurrentFrame );
        int iSinc = iScanS - w; // Source increment value
        int iDinc = iScanD - ( w * 3); // Destination increment value
		if ( iIndex == -1 )
		{
	        while (h--) 
	        {
	            int iCount = w;    // Number of pixels to scan.
	            while (iCount--) 
	            {
	                // Copy pixel only if it isn't transparent.
                    *pDest++ = prgb[*pSrc].rgbBlue;
                    *pDest++ = prgb[*pSrc].rgbGreen;
                    *pDest++ = prgb[*pSrc].rgbRed;
					*pSrc++;
	            }
	            // Move on to the next line.
	            pSrc += iSinc;
	            pDest += iDinc;
	        }

		}
		else
		{
	        BYTE bTransClr = (BYTE) iIndex;
	        BYTE pixel;
	        int iCount;

	        while (h--) 
	        {
	            iCount = w;    // Number of pixels to scan.
	            while (iCount--) 
	            {
	                // Copy pixel only if it isn't transparent.
	                pixel = *pSrc++;
	                if (pixel != bTransClr) 
					{
	                    *pDest++ = prgb[pixel].rgbBlue;
	                    *pDest++ = prgb[pixel].rgbGreen;
	                    *pDest++ = prgb[pixel].rgbRed;
					}
					else
					{
	                    pDest += 3;
					}
	            }
	            // Move on to the next line.
	            pSrc += iSinc;
	            pDest += iDinc;
	        }
		}
	}
} 

bool ChDib::NewImage( pChImageInfo pImage )
{
	m_imgInfo = *pImage;
	return true;
}

bool ChDib::Create( pChImageFrameInfo pFrameInfo, int iBitCount /*= 8 */)
{
	AllocateFrame( pFrameInfo->iFrame );

	m_pFrameList[pFrameInfo->iFrame].m_frameInfo = *pFrameInfo;

 	if ( Create( pFrameInfo->iFrame, pFrameInfo->iWidth, 
 							pFrameInfo->iHeight, iBitCount ) )
	{

	 	return true;
	}
	return false;
}

bool ChDib::SetColorTable( int iFrame, RGBQUAD* pColorTbl, int iColors ) 
{
	ASSERT( iFrame < m_iNumFrames );
	
	OutputDebugString("*** Entered ChDib::SetColorTable\r\n");
	
    // Note: This will probably fail for < 256 color headers.
    ChMemCopy( GetClrTabAddress( iFrame ), pColorTbl, iColors * sizeof(RGBQUAD));

	OutputDebugString("*** Exited ChDib::SetColorTable\r\n");

	return true;
} 

bool ChDib::SetScanLine( int iFrame, int iScanLine, 
					BYTE* pPixels, int iBufferLength, int iFormat )
{
	ASSERT( iFrame < m_iNumFrames ); 


    LPBYTE pBits = (LPBYTE)m_pFrameList[iFrame].m_pBits;
    pBits += (StorageWidth( iFrame ) * 
    		(m_pFrameList[iFrame].m_pBMI->
    				bmiHeader.biHeight - iScanLine - 1 ) );
	
	if ( iFormat == format8Bit || iFormat == format24RGB )
	{
    	ChMemCopy( pBits, pPixels, iBufferLength );
	}
	else
	{
		for (int col = m_pFrameList[iFrame].m_pBMI->
    				bmiHeader.biWidth; col > 0; col--) 
		{
			*pBits++ = pPixels[2];	
			*pBits++ = pPixels[1];
			*pBits++ = pPixels[0];
			pPixels += 3;
		}
	}
	return true;
}
