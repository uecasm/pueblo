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

   ChDibDecoder class implementation

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"


#include <ChImgUtil.h>
#include <ChDibDecoder.h>
#include <ChDibImage.h>

#include <lzexpand.h>    
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
// ChDibDecoder

// Create a small DIB here so m_pBMI and m_pBits are always valid.
ChDibDecoder::ChDibDecoder( ChImageConsumer *pConsumer )  :
					ChImageDecoder( pConsumer )
{
}

ChDibDecoder::~ChDibDecoder()
{
}

// Load a DIB from an open LZ file.
bool ChDibDecoder::Load(LZHANDLE lzHdl)
{
    BOOL bIsPM = false;
    BYTE* pBits = NULL;

    // Get the current file position.
    LONG	lFileStart = 0;
    lFileStart = LZSeek(lzHdl, 0, 1);
	//LZSeek(lzHdl, lFileStart, 0);

    // Read the file header to get the file size and to
    // find where the bits start in the file.
    BITMAPFILEHEADER BmpFileHdr;
    int iBytes;
    iBytes = LZRead(lzHdl, (LPSTR)&BmpFileHdr, sizeof(BmpFileHdr));
    if (iBytes != sizeof(BmpFileHdr)) 
    {
        TRACE("Failed to read file header\n");
        goto d__abort;
    }

    // Check that we have the magic 'BM' at the start.
    if (BmpFileHdr.bfType != 0x4D42) 
    {
        TRACE("Not a bitmap file\n");
        goto d__abort;
    }

    // Make a wild guess that the file is in Windows DIB
    // format and read the BITMAPINFOHEADER. If the file turns
    // out to be a PM DIB file we'll convert it later.
    BITMAPINFOHEADER BmpInfoHdr;
    iBytes = LZRead(lzHdl, (LPSTR)&BmpInfoHdr, sizeof(BmpInfoHdr)); 
    if (iBytes != sizeof(BmpInfoHdr)) 
    {
        TRACE("Failed to read BITMAPINFOHEADER\n");
        goto d__abort;
    }

    // Check that we got a real Windows DIB file.
    if (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)) 
    {
        if (BmpInfoHdr.biSize != sizeof(BITMAPCOREHEADER)) 
        {
            TRACE(" File is not Windows or PM DIB format\n");
            goto d__abort;
        }

        // Set a flag to convert PM file to Win format later.
        bIsPM = true;

        // Back up the file pointer and read the BITMAPCOREHEADER
        // and create the BITMAPINFOHEADER from it.
        LZSeek(lzHdl, lFileStart + sizeof(BITMAPFILEHEADER), 0);
        BITMAPCOREHEADER BmpCoreHdr;
        iBytes = LZRead(lzHdl, (LPSTR)&BmpCoreHdr, sizeof(BmpCoreHdr)); 
        if (iBytes != sizeof(BmpCoreHdr)) 
        {
            TRACE("Failed to read BITMAPCOREHEADER\n");
            goto d__abort;
        }

        BmpInfoHdr.biSize = sizeof(BITMAPINFOHEADER);
        BmpInfoHdr.biWidth = (int) BmpCoreHdr.bcWidth;
        BmpInfoHdr.biHeight = (int) BmpCoreHdr.bcHeight;
        BmpInfoHdr.biPlanes = BmpCoreHdr.bcPlanes;
        BmpInfoHdr.biBitCount = BmpCoreHdr.bcBitCount;
        BmpInfoHdr.biCompression = BI_RGB;
        BmpInfoHdr.biSizeImage = 0;
        BmpInfoHdr.biXPelsPerMeter = 0;
        BmpInfoHdr.biYPelsPerMeter = 0;
        BmpInfoHdr.biClrUsed = 0;
        BmpInfoHdr.biClrImportant = 0;
    }

    // Work out how much memory we need for the BITMAPINFO
    // structure, color table and then for the bits.  
    // Allocate the memory blocks.
    // Copy the BmpInfoHdr we have so far,
    // and then read in the color table from the file.
    int iColors;
    int iColorTableSize;
    iColors = ChImgUtil::NumDIBColorEntries((LPBITMAPINFO) &BmpInfoHdr);
    iColorTableSize = iColors * sizeof(RGBQUAD);
    int iBitsSize;
    int iBISize;
    // Always allocate enough room for 256 entries.
    iBISize = 256 * sizeof(RGBQUAD);

    iBitsSize = (int)(BmpFileHdr.bfSize - 
                BmpFileHdr.bfOffBits);
	long iTmpBitsSize;
    iTmpBitsSize = (((BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount/8 ) + 3) & ~3) 
    						* BmpInfoHdr.biHeight;

	if ( iBitsSize < iTmpBitsSize )
	{ // must be a compressed
        TRACE("Compressed file we don't handle this\n");
        goto d__abort;
	}


	ChImageInfo  imageInfo;
	ChMemClearStruct( &imageInfo );
	imageInfo.iWidth = BmpInfoHdr.biWidth; 
	imageInfo.iHeight = BmpInfoHdr.biHeight;
	GetConsumer()->NewImage( &imageInfo );

	// Create the image 
	ChImageFrameInfo frameInfo;
	ChMemClearStruct( &frameInfo );	
	frameInfo.iFrame = 1;
	frameInfo.iWidth = BmpInfoHdr.biWidth;
	frameInfo.iHeight = BmpInfoHdr.biHeight;

	GetConsumer()->Create( &frameInfo, BmpInfoHdr.biBitCount  );

    // Allocate the memory for the header.
    LPRGBQUAD lpColorTable;
    lpColorTable = (LPRGBQUAD) ALLOC(iBISize);
    if (!lpColorTable) 
    {
        TRACE("Out of memory for color table\n");
        goto d__abort;
    }


    // Now read the color table from the file.
    if (bIsPM == false) 
    {
        // Read the color table from the file.
        iBytes = LZRead(lzHdl, (LPSTR)lpColorTable,
                             iColorTableSize);
        if (iBytes != iColorTableSize) 
        {
            TRACE("Failed to read color table\n");
            goto d__abort;
        }
    } 
    else 
    {
        // Read each PM color table entry in turn and convert it
       // to Win DIB format as we go.
		int i;
		LPRGBQUAD lpRGB = lpColorTable;
		RGBTRIPLE rgbt;
        for (i=0; i<iColors; i++) 
        {
            iBytes = LZRead(lzHdl, (LPSTR)&rgbt, sizeof(RGBTRIPLE));
            if (iBytes != sizeof(RGBTRIPLE)) 
            {
                TRACE("Failed to read RGBTRIPLE\n");
                goto d__abort;
            }
            lpRGB->rgbBlue = rgbt.rgbtBlue;
            lpRGB->rgbGreen = rgbt.rgbtGreen;
            lpRGB->rgbRed = rgbt.rgbtRed;
            lpRGB->rgbReserved = 0;
            lpRGB++;
        }
    }

	// Set the color table
	GetConsumer()->SetColorTable( 1, lpColorTable, iColors );

    // Allocate the memory for the bits in one scan
    // and read the bits from the file.
    //int iScanBitsSize = (((BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount/8 ) + 3) & ~3); 

    pBits = (BYTE*) ALLOC((((BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount/8 ) + 3) & ~3));
    if (!pBits) 
    {
        TRACE("Out of memory for DIB bits\n");
        goto d__abort;
    }

    // Seek to the bits in the file.
    LZSeek(lzHdl, lFileStart + BmpFileHdr.bfOffBits, 0);	

	int iScanLine;
    for ( iScanLine=0; iScanLine < BmpInfoHdr.biHeight; iScanLine++) 
    {
	    // Read the bits.
	    iBytes = LZRead(lzHdl, (LPSTR)pBits, iBitsSize);
	    if (iBytes != iBitsSize) 
	    {
	        TRACE("Failed to read bits\n");
	        goto d__abort;
	    }
		// Set the scan line
		GetConsumer()->SetScanLine( 0, iScanLine, pBits, 
						iBitsSize, ChImageConsumer::format8Bit ); 
	}

	FREE(lpColorTable);
	FREE(pBits);

    return true;
                
d__abort: // Something went wrong.
    if (lpColorTable) FREE(lpColorTable);
    if (pBits) FREE(pBits);
    return false;
}

// Load a DIB from a disk file. If no file name is given, show
// an Open File dialog to get one.
bool ChDibDecoder::Load(const char* pszFileName, chuint32 flOption )
{
    if ((pszFileName == NULL) ||  (strlen(pszFileName) == 0)) 
    {
		return false;
    } 

    // Try to open the file for read access.
    LZHANDLE lzHdl; 
	INT	hFile;
    if (! (hFile = (INT)::CreateFile(	(LPTSTR)((LPCTSTR)(pszFileName)),
					                    GENERIC_READ,
					                    FILE_SHARE_READ,
										0,
										OPEN_EXISTING,
										0,
										0)))       
	{  //
        TRACE("Failed to open file\n");
        return false;
    }
	lzHdl = ::LZInit( hFile );
    bool bResult = Load(lzHdl);
    ::LZClose(lzHdl);        
         
	::CloseHandle((HANDLE)hFile);

    return bResult;
}

// Load a DIB from a resource id.
bool ChDibDecoder::Load(WORD wResid, HINSTANCE hInstance)
{
    ASSERT(wResid);
    HINSTANCE hInst = (hInstance ? hInstance : AfxGetResourceHandle());
    HRSRC hrsrc = ::FindResource(hInst, MAKEINTRESOURCE(wResid), "DIB");
    if (!hrsrc) 
    {
        TRACE("DIB resource not found\n");
        return false;
    }
    HGLOBAL hg = LoadResource(hInst, hrsrc);
    if (!hg) 
    {
        TRACE("Failed to load DIB resource\n");
        return false;
    }
    BYTE* pRes = (BYTE*) LockResource(hg);
    ASSERT(pRes);
    DWORD iSize = ::SizeofResource(hInst, hrsrc);

	#if !defined( CH_ARCH_16 )
    // Mark the resource pages as read/write so the mmioOpen
    // won't fail
    DWORD dwOldProt;
    BOOL b = ::VirtualProtect(pRes,
                              iSize,
                              PAGE_READWRITE,
                              &dwOldProt);
    ASSERT(b);
	#endif

    // Now create the ChDib object. We will create a new header from the 
    // data in the resource image and copy the bits from the resource
    // to a new block of memory.  We can't use the resource image as-is 
    // because we might want to map the DIB colors and the resource memory
    // is write protected in Win32.

    BITMAPFILEHEADER* pFileHdr = (BITMAPFILEHEADER*)pRes;
    ASSERT(pFileHdr->bfType == 0x4D42); // BM file
    BITMAPINFOHEADER* pInfoHdr = (BITMAPINFOHEADER*) (pRes + sizeof(BITMAPFILEHEADER));
    ASSERT(pInfoHdr->biSize == sizeof(BITMAPINFOHEADER));  // must be a Win DIB
    BYTE* pBits = pRes + pFileHdr->bfOffBits;

    return GetConsumer()->Create( 0, (BITMAPINFO*)pInfoHdr, pBits);
    // Note: not required to unlock or free the resource in Win32
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:12  uecasm
// Import of source tree as at version 2.53 release.
//
