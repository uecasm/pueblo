//-----------------------------------------------------------------------------
//
//           .###.  ###     ###       ,#,          .###.    ,#######,
//        ,####""   ###    .###     ,##'##,     ,####""   .##'    `##.
//       ###        ###########    ,##' `##,   ###       ####      ####
//       ###..      ###'    ###  ,###########, ####..    `###,    ,##'
//         `######  ###     ###  `##'     `##'   `######   `########'
//
//
//	Copyright 1994 Chaco Communications, Inc. All rights reserved.
//
// The contents of this file are subject to the Andromedia Public License Version
// 1.0 (the "License"); you may not use this file except in compliance with the
// License. You may obtain a copy of the License at http://pueblo.sf.net/APL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
// the specific language governing rights and limitations under the License.
//
// The Original Code is Pueblo client code, released November 4, 1998.
// The Initial Developer of the Original Code is Andromedia Incorporated.
// Portions created by Andromedia are Copyright (C) 1998 Andromedia
// Incorporated. All Rights Reserved.
//
// Contributor(s):
//
//   27 April 2002          Gavin Lambert, Ultra Enterprises
//        License notice updated to reflect APL requirements.
//
//------------------------------------------------------------------------------
//
//	ChDibBmp class implementation
//
//------------------------------------------------------------------------------

#include "headers.h"

#include <ChImgUtil.h>
#include <ChDibBmp.h>

#include <lzexpand.h>    
    
#if defined( CH_VRML_VIEWER )
#include "ChGrVw.h"  
#endif

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Win32s works better if you use GlobalAlloc for large memory
// blocks so the CWave and ChDibBmp classes use the ALLOC and FREE
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
// ChDibBmp

IMPLEMENT_SERIAL(ChDibBmp, CObject, 0 /* Schema number */ )

// Create a small DIB here so m_pBMI and m_pBits are always valid.
ChDibBmp::ChDibBmp()
{
    m_pBMI = NULL;
    m_pBits = NULL;
	m_pPalette = NULL;
	m_flOptions = 0;

    m_bMyBits = true;
    Create(16, 16);
}

ChDibBmp::~ChDibBmp()
{
    // Free the memory.
    if (m_pBMI != NULL) FREE(m_pBMI);
    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);
	if ( m_pPalette ) delete m_pPalette;

}

/////////////////////////////////////////////////////////////////////////////
// ChDibBmp serialization

// We don't support this yet.
void ChDibBmp::Serialize(CArchive& ar)
{
    ar.Flush();
    CFile* fp = ar.GetFile();

    if (ar.IsStoring()) {
        Save(fp);
    } else {
        Load(fp);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Private functions

static BOOL IsWinDIB(BITMAPINFOHEADER *pBIH)
{
    ASSERT(pBIH);
    if (((BITMAPCOREHEADER*)pBIH)->bcSize == sizeof(BITMAPCOREHEADER)) {
        return false;
    }
    return true;
}

static int NumDIBColorEntries(BITMAPINFO* pBmpInfo) 
{
    BITMAPINFOHEADER* pBIH;
    BITMAPCOREHEADER* pBCH;
    int iColors, iBitCount;

    ASSERT(pBmpInfo);

    pBIH = &(pBmpInfo->bmiHeader);
    pBCH = (BITMAPCOREHEADER*) pBIH;

    // Start off by assuming the color table size from
    // the bit-per-pixel field.
    if (IsWinDIB(pBIH)) {
        iBitCount = pBIH->biBitCount;
    } else {
        iBitCount = pBCH->bcBitCount;
    }

    switch (iBitCount) {
    case 1:
        iColors = 2;
        break;
    case 4:
        iColors = 16;
        break;
    case 8:
        iColors = 256;
        break;
    default:
        iColors = 0;
        break;
    }

    // If this is a Windows DIB, then the color table length
    // is determined by the biClrUsed field if the value in
    // the field is nonzero.
    if (IsWinDIB(pBIH) && (pBIH->biClrUsed != 0)) {
        iColors = (int)pBIH->biClrUsed;
    }

    // BUGFIX 18 Oct 94 NigelT
    // Make sure the value is reasonable since some products
    // will write out more then 256 colors for an 8 bpp DIB!!!
    int iMax = 0;
    switch (iBitCount) {
    case 1:
        iMax = 2;
        break;
    case 4:
        iMax = 16;
        break;
    case 8:
        iMax = 256;
        break;
    default:
        iMax = 0;
        break;
    }
    if (iMax) {
        if (iColors > iMax) {
            TRACE("Invalid color count");
            iColors = iMax;
        }
    }

    return iColors;
}

long ChDibBmp::StorageWidth()
{	
	int iWidth;
    switch ( m_pBMI->bmiHeader.biBitCount ) 
    {
	    case 1:
	        iWidth  = (((m_pBMI->bmiHeader.biWidth/8) + 3) & ~3 );
	        break;
	    case 4:
	        iWidth = ((( m_pBMI->bmiHeader.biWidth >> 1 ) + 3) & ~3 );
	        break;
	    case 8:
	        iWidth  = ((m_pBMI->bmiHeader.biWidth + 3) & ~3 );
	        break;
	    case 24 :
			iWidth  = (((m_pBMI->bmiHeader.biWidth * 3) + 3) & ~3);
			break;
	    default:
	        iWidth = 0;
	        break;
    }
	return ( iWidth == 0 ? 4 : iWidth );
}



/////////////////////////////////////////////////////////////////////////////
// ChDibBmp commands

// Create a new empty 8bpp DIB with a 256 entry color table.
BOOL ChDibBmp::Create(int iWidth, int iHeight, int iBitCount /*= 8 */ )
{
    // Delete any existing stuff.
    if (m_pBMI != NULL) FREE(m_pBMI);
    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);

    // Allocate memory for the header.
    m_pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD));
    if (!m_pBMI) {
        TRACE("Out of memory for DIB header");
        return false;
    }

    // Allocate memory for the bits (DWORD aligned).
    long iBitsSize = (((iWidth * iBitCount/8 ) + 3) & ~3) * iHeight;
	if ( iBitsSize == 0 )
	{
		iBitsSize = 4;
	}
    m_pBits = (BYTE*)ALLOC(iBitsSize);
    if (!m_pBits) {
        TRACE("Out of memory for DIB bits");
        FREE(m_pBMI);
        m_pBMI = NULL;
        return false;
    }
    m_bMyBits = true;

    // Fill in the header info.
    BITMAPINFOHEADER* pBI = (BITMAPINFOHEADER*) m_pBMI;
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
	    RGBQUAD* prgb = GetClrTabAddress();
	    for (int i = 0; i < 256; i++) {
	        prgb->rgbBlue = prgb->rgbGreen = prgb->rgbRed = (BYTE) i;
	        prgb->rgbReserved = 0;
	        prgb++;
	    }
	}

    // Set all the bits to a known state (black).  
    #if defined( CH_ARCH_16 )
    _fmemset(m_pBits, 0, (size_t)iBitsSize);
    #else
    memset(m_pBits, 0, iBitsSize);
    #endif

    return true;
}

// Create a ChDibBmp structure from existing header and bits. The DIB
// won't delete the bits and makes a copy of the header.
BOOL ChDibBmp::Create(BITMAPINFO* pBMI, BYTE* pBits, BOOL boolCopyBits )
{
    ASSERT(pBMI);
    ASSERT(pBits);
    if (m_pBMI != NULL) FREE(m_pBMI);
    m_pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                   + 256 * sizeof(RGBQUAD));
    ASSERT(m_pBMI);
    // Note: This will probably fail for < 256 color headers.
    memcpy(m_pBMI, pBMI, sizeof(BITMAPINFOHEADER)+
             NumDIBColorEntries(pBMI) * sizeof(RGBQUAD));

    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);
	if ( boolCopyBits )
	{
	    long iBitsSize = (((m_pBMI->bmiHeader.biWidth * 
	    			m_pBMI->bmiHeader.biBitCount/8 ) + 3) & ~3) * m_pBMI->bmiHeader.biHeight;
		if ( iBitsSize == 0 )
		{
			iBitsSize = 4;
		}
	    m_pBits = (BYTE*)ALLOC(iBitsSize);
   		memcpy(m_pBits, pBits, iBitsSize );
	    m_bMyBits = true; // We can't delete the bits.
	}
	else
	{
	    m_pBits = pBits;
	    m_bMyBits = false; // We can't delete the bits.
	}
    return true;
}


// Load a DIB from an open file.
BOOL ChDibBmp::Load(CFile* fp)
{
    BOOL bIsPM = false;
    BITMAPINFO* pBmpInfo = NULL;
    BYTE* pBits = NULL;

    // Get the current file position.
    DWORD dwFileStart = (DWORD)fp->GetPosition();

    // Read the file header to get the file size and to
    // find where the bits start in the file.
    BITMAPFILEHEADER BmpFileHdr;
    int iBytes;
    iBytes = fp->Read(&BmpFileHdr, sizeof(BmpFileHdr));
    if (iBytes != sizeof(BmpFileHdr)) {
        TRACE("Failed to read file header");
        goto d__abort;
    }

    // Check that we have the magic 'BM' at the start.
    if (BmpFileHdr.bfType != 0x4D42) {
        TRACE("Not a bitmap file");
        goto d__abort;
    }

    // Make a wild guess that the file is in Windows DIB
    // format and read the BITMAPINFOHEADER. If the file turns
    // out to be a PM DIB file we'll convert it later.
    BITMAPINFOHEADER BmpInfoHdr;
    iBytes = fp->Read(&BmpInfoHdr, sizeof(BmpInfoHdr)); 
    if (iBytes != sizeof(BmpInfoHdr)) {
        TRACE("Failed to read BITMAPINFOHEADER");
        goto d__abort;
    }

    // Check that we got a real Windows DIB file.
    if (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)) {
        if (BmpInfoHdr.biSize != sizeof(BITMAPCOREHEADER)) {
            TRACE(" File is not Windows or PM DIB format");
            goto d__abort;
        }

        // Set a flag to convert PM file to Win format later.
        bIsPM = true;

        // Back up the file pointer and read the BITMAPCOREHEADER
        // and create the BITMAPINFOHEADER from it.
        fp->Seek(dwFileStart + sizeof(BITMAPFILEHEADER), CFile::begin);
        BITMAPCOREHEADER BmpCoreHdr;
        iBytes = fp->Read(&BmpCoreHdr, sizeof(BmpCoreHdr)); 
        if (iBytes != sizeof(BmpCoreHdr)) {
            TRACE("Failed to read BITMAPCOREHEADER");
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
    iColors = NumDIBColorEntries((LPBITMAPINFO) &BmpInfoHdr);
    iColorTableSize = iColors * sizeof(RGBQUAD);
    int iBitsSize;
    int iBISize;
    // Always allocate enough room for 256 entries.
    iBISize = (int)(sizeof(BITMAPINFOHEADER)    
           + 256 * sizeof(RGBQUAD));


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

   // Allocate the memory for the header.
    pBmpInfo = (LPBITMAPINFO) ALLOC(iBISize);
    if (!pBmpInfo) {
        TRACE("Out of memory for DIB header");
        goto d__abort;
    }

    // Copy the header we already have.
    memcpy(pBmpInfo, &BmpInfoHdr, sizeof(BITMAPINFOHEADER));

    // Now read the color table from the file.
    if (bIsPM == false) {
        // Read the color table from the file.
        iBytes = fp->Read(((LPBYTE) pBmpInfo) + sizeof(BITMAPINFOHEADER),
                             iColorTableSize);
        if (iBytes != iColorTableSize) {
            TRACE("Failed to read color table");
            goto d__abort;
        }
    } else {
        // Read each PM color table entry in turn and convert it
        // to Win DIB format as we go.
        LPRGBQUAD lpRGB;
        lpRGB = (LPRGBQUAD) ((LPBYTE) pBmpInfo + sizeof(BITMAPINFOHEADER));
        int i;
        RGBTRIPLE rgbt;
        for (i=0; i<iColors; i++) {
            iBytes = fp->Read(&rgbt, sizeof(RGBTRIPLE));
            if (iBytes != sizeof(RGBTRIPLE)) {
                TRACE("Failed to read RGBTRIPLE");
                goto d__abort;
            }
            lpRGB->rgbBlue = rgbt.rgbtBlue;
            lpRGB->rgbGreen = rgbt.rgbtGreen;
            lpRGB->rgbRed = rgbt.rgbtRed;
            lpRGB->rgbReserved = 0;
            lpRGB++;
        }
    }

    // Allocate the memory for the bits
    // and read the bits from the file.
    pBits = (BYTE*) ALLOC(iBitsSize);
    if (!pBits) {
        TRACE("Out of memory for DIB bits");
        goto d__abort;
    }

    // Seek to the bits in the file.
    fp->Seek(dwFileStart + BmpFileHdr.bfOffBits, CFile::begin);

    // Read the bits.
    iBytes = fp->Read(pBits, (int)iBitsSize);
    if (iBytes != iBitsSize) {
        TRACE("Failed to read bits");
        goto d__abort;
    }

    // Everything went OK.
    if (m_pBMI != NULL) FREE(m_pBMI);
    m_pBMI = pBmpInfo; 
    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);
    m_pBits = pBits;
    m_bMyBits = true;
    return true;
                
d__abort: // Something went wrong.
    if (pBmpInfo) FREE(pBmpInfo);
    if (pBits) FREE(pBits);
    return false;
}

// Load a DIB from an open LZ file.
BOOL ChDibBmp::Load(LZHANDLE lzHdl)
{
    BOOL bIsPM = false;
    BITMAPINFO* pBmpInfo = NULL;
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
    if (iBytes != sizeof(BmpFileHdr)) {
        TRACE("Failed to read file header");
        goto d__abort;
    }

    // Check that we have the magic 'BM' at the start.
    if (BmpFileHdr.bfType != 0x4D42) {
        TRACE("Not a bitmap file");
        goto d__abort;
    }

    // Make a wild guess that the file is in Windows DIB
    // format and read the BITMAPINFOHEADER. If the file turns
    // out to be a PM DIB file we'll convert it later.
    BITMAPINFOHEADER BmpInfoHdr;
    iBytes = LZRead(lzHdl, (LPSTR)&BmpInfoHdr, sizeof(BmpInfoHdr)); 
    if (iBytes != sizeof(BmpInfoHdr)) {
        TRACE("Failed to read BITMAPINFOHEADER");
        goto d__abort;
    }

    // Check that we got a real Windows DIB file.
    if (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)) {
        if (BmpInfoHdr.biSize != sizeof(BITMAPCOREHEADER)) {
            TRACE(" File is not Windows or PM DIB format");
            goto d__abort;
        }

        // Set a flag to convert PM file to Win format later.
        bIsPM = true;

        // Back up the file pointer and read the BITMAPCOREHEADER
        // and create the BITMAPINFOHEADER from it.
        LZSeek(lzHdl, lFileStart + sizeof(BITMAPFILEHEADER), 0);
        BITMAPCOREHEADER BmpCoreHdr;
        iBytes = LZRead(lzHdl, (LPSTR)&BmpCoreHdr, sizeof(BmpCoreHdr)); 
        if (iBytes != sizeof(BmpCoreHdr)) {
            TRACE("Failed to read BITMAPCOREHEADER");
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
    iColors = NumDIBColorEntries((LPBITMAPINFO) &BmpInfoHdr);
    iColorTableSize = iColors * sizeof(RGBQUAD);
    int iBitsSize;
    int iBISize;
    // Always allocate enough room for 256 entries.
    iBISize = sizeof(BITMAPINFOHEADER)    
           + 256 * sizeof(RGBQUAD);

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

    // Allocate the memory for the header.
    pBmpInfo = (LPBITMAPINFO) ALLOC(iBISize);
    if (!pBmpInfo) {
        TRACE("Out of memory for DIB header");
        goto d__abort;
    }

    // Copy the header we already have.
    memcpy(pBmpInfo, &BmpInfoHdr, sizeof(BITMAPINFOHEADER));

    // Now read the color table from the file.
    if (bIsPM == false) {
        // Read the color table from the file.
        iBytes = LZRead(lzHdl, (LPSTR)(((LPBYTE) pBmpInfo) + sizeof(BITMAPINFOHEADER)),
                             iColorTableSize);
        if (iBytes != iColorTableSize) {
            TRACE("Failed to read color table");
            goto d__abort;
        }
    } else {
        // Read each PM color table entry in turn and convert it
        // to Win DIB format as we go.
        LPRGBQUAD lpRGB;
        lpRGB = (LPRGBQUAD) ((LPBYTE) pBmpInfo + sizeof(BITMAPINFOHEADER));
        int i;
        RGBTRIPLE rgbt;
        for (i=0; i<iColors; i++) {
            iBytes = LZRead(lzHdl, (LPSTR)&rgbt, sizeof(RGBTRIPLE));
            if (iBytes != sizeof(RGBTRIPLE)) {
                TRACE("Failed to read RGBTRIPLE");
                goto d__abort;
            }
            lpRGB->rgbBlue = rgbt.rgbtBlue;
            lpRGB->rgbGreen = rgbt.rgbtGreen;
            lpRGB->rgbRed = rgbt.rgbtRed;
            lpRGB->rgbReserved = 0;
            lpRGB++;
        }
    }

    // Allocate the memory for the bits
    // and read the bits from the file.
    pBits = (BYTE*) ALLOC(iBitsSize);
    if (!pBits) {
        TRACE("Out of memory for DIB bits");
        goto d__abort;
    }

    // Seek to the bits in the file.
    LZSeek(lzHdl, lFileStart + BmpFileHdr.bfOffBits, 0);

    // Read the bits.
    iBytes = LZRead(lzHdl, (LPSTR)pBits, iBitsSize);
    if (iBytes != iBitsSize) {
        TRACE("Failed to read bits");
        goto d__abort;
    }

    // Everything went OK.
    if (m_pBMI != NULL) FREE(m_pBMI);
    m_pBMI = pBmpInfo; 
    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);
    m_pBits = pBits;
    m_bMyBits = true;
    return true;
                
d__abort: // Something went wrong.
    if (pBmpInfo) FREE(pBmpInfo);
    if (pBits) FREE(pBits);
    return false;
}

// Load a DIB from a disk file. If no file name is given, show
// an Open File dialog to get one.
BOOL ChDibBmp::Load(const char* pszFileName, chuint32 flOption )
{
    CString strFile;    

    if ((pszFileName == NULL) 
    ||  (strlen(pszFileName) == 0)) {

        // Show an Open File dialog to get the name.
        CFileDialog dlg   (true,    // Open
                           NULL,    // No default extension
                           NULL,    // No initial file name
                           OFN_FILEMUSTEXIST
                             | OFN_HIDEREADONLY,
                           "Image files (*.DIB, *.BMP)|*.DIB;*.BMP|All files (*.*)|*.*||");
        if (dlg.DoModal() == IDOK) {
            strFile = dlg.GetPathName();
        } else {
            return false;
        }
    } else {
        // Copy the supplied file path.
        strFile = pszFileName;                    
    }

    // Try to open the file for read access.
    LZHANDLE lzHdl; 
    
    #if defined( CH_ARCH_16 )
	OFSTRUCT ofs;   
	HFILE hFile;
	
    if ((hFile = OpenFile((LPTSTR)((LPCTSTR)(strFile)), &ofs,
                    OF_READ | OF_SHARE_DENY_WRITE	)) == HFILE_ERROR )  
	#else        
	INT	hFile;
    if (! (hFile = (INT)::CreateFile(	(LPTSTR)((LPCTSTR)(strFile)),
					                    GENERIC_READ,
					                    FILE_SHARE_READ,
										0,
										OPEN_EXISTING,
										0,
										0)))       
	#endif										
	{  //
        TRACE("Failed to open file");
        return false;
    }
	lzHdl = ::LZInit( hFile );
    BOOL bResult = Load(lzHdl);
    ::LZClose(lzHdl);        
         
    #if defined( CH_ARCH_16 )
    _lclose( hFile );
    #else
	::CloseHandle((HANDLE)hFile);
	#endif


	#if 0
    CFile file;
    if (! file.Open(strFile,
                    CFile::modeRead | CFile::shareDenyWrite)) {
        TRACE("Failed to open file");
        return false;
    }

    BOOL bResult = Load(&file);
    file.Close();
	#endif
    return bResult;
}

// Load a DIB from a resource id.
BOOL ChDibBmp::Load(WORD wResid, HINSTANCE hInstance)
{
    ASSERT(wResid);
    HINSTANCE hInst = (hInstance ? hInstance : AfxGetResourceHandle());
    HRSRC hrsrc = ::FindResource(hInst, MAKEINTRESOURCE(wResid), "DIB");
    if (!hrsrc) {
        TRACE("DIB resource not found");
        return false;
    }
    HGLOBAL hg = LoadResource(hInst, hrsrc);
    if (!hg) {
        TRACE("Failed to load DIB resource");
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

    // Now create the ChDibBmp object. We will create a new header from the 
    // data in the resource image and copy the bits from the resource
    // to a new block of memory.  We can't use the resource image as-is 
    // because we might want to map the DIB colors and the resource memory
    // is write protected in Win32.

    BITMAPFILEHEADER* pFileHdr = (BITMAPFILEHEADER*)pRes;
    ASSERT(pFileHdr->bfType == 0x4D42); // BM file
    BITMAPINFOHEADER* pInfoHdr = (BITMAPINFOHEADER*) (pRes + sizeof(BITMAPFILEHEADER));
    ASSERT(pInfoHdr->biSize == sizeof(BITMAPINFOHEADER));  // must be a Win DIB
    BYTE* pBits = pRes + pFileHdr->bfOffBits;
    return Create((BITMAPINFO*)pInfoHdr, pBits);
    // Note: not required to unlock or free the resource in Win32
}

// Draw the DIB to a given DC.
void ChDibBmp::Draw(CDC* pDC, int x, int y)
{

	CPalette *pOldPal = 0;
	
	if ( GetDIBPalette() )
	{
		pOldPal = pDC->SelectPalette( GetDIBPalette(), TRUE );
		pDC->RealizePalette();
	}  

    ::StretchDIBits(pDC->GetSafeHdc(),
                    x,                        // Destination x
                    y,                        // Destination y
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


BOOL ChDibBmp::SetSize( long lWidth, long lHeight )
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
    BITMAPINFO * pBMI = (BITMAPINFO*) ALLOC(sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD));
    if (!pBMI) 
    {
        TRACE("Out of memory for DIB header");
        return false;
    }



    // Allocate memory for the bits (DWORD aligned).
    long iBitsSize = (((lWidth * iBitCount/8 ) + 3) & ~3) * lHeight;
    BYTE *pBits = (BYTE*)ALLOC(iBitsSize);
    if (!pBits) 
    {
        TRACE("Out of memory for DIB bits");
        FREE(pBMI);
        return false;
    }

	ChMemCopy( pBMI, m_pBMI,  sizeof(BITMAPINFOHEADER)
                                  + 256 * sizeof(RGBQUAD) );
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
		if ( m_flOptions & grayScale )
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

    if (m_pBMI != NULL) FREE(m_pBMI);
    if (m_bMyBits && (m_pBits != NULL)) FREE(m_pBits);

	m_pBMI = pBMI;
	m_pBits = pBits;
	m_bMyBits = true;

	return true;
}



// Get the number of color table entries.
int ChDibBmp::GetNumClrEntries()
{
    return NumDIBColorEntries(m_pBMI);
}

// NOTE: This assumes all ChDibBmp objects have 256 color table entries.
BOOL ChDibBmp::MapColorsToPalette(CPalette *pPal)
{
    if (!pPal) {
        TRACE("No palette to map to");
        return false;
    }
    ASSERT(m_pBMI);
    ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    ASSERT(m_pBits);
    LPRGBQUAD pctThis = GetClrTabAddress();
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
    DWORD iSize = StorageWidth() * DibHeight();
    while (iSize--) {
        *pBits = imap[*pBits];
        pBits++;
    }
    // Now reset the DIB color table so that its RGB values match
    // those in the palette.
    PALETTEENTRY pe[256];
    pPal->GetPaletteEntries(0, 256, pe);
    pctThis = GetClrTabAddress();
    for (i = 0; i < 256; i++) {
        pctThis->rgbRed = pe[i].peRed;    
        pctThis->rgbGreen = pe[i].peGreen;    
        pctThis->rgbBlue = pe[i].peBlue;
        pctThis++;    
    }
    // Now say all the colors are in use
    m_pBMI->bmiHeader.biClrUsed = 256;
    return true;
}

// Get a pointer to a pixel.
// NOTE: DIB scan lines are DWORD aligned. The scan line 
// storage width may be wider than the scan line image width
// so calc the storage width by rounding the image width 
// to the next highest DWORD value.
void* ChDibBmp::GetPixelAddress(int x, int y)
{
    int iWidth;
    // Note: This version deals only with 8 and 24 bpp DIBs.
    //ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    // Make sure it's in range and if it isn't return zero.
    
    if ((x >= DibWidth()) 
    || (y >= DibHeight())) {
        TRACE("Attempt to get out of range pixel address");
        return NULL;
    }

 	iWidth = (int)StorageWidth();
   // Calculate the scan line storage width.
   switch( m_pBMI->bmiHeader.biBitCount )
   {
   		case 1 :
		{
	    	return m_pBits + (DibHeight()-y-1) * iWidth + (x/8);
		}
   		case 4 :
		{
	    	return m_pBits + (DibHeight()-y-1) * iWidth + (x/4);
		}
   		case 8 :
		{
	    	return m_pBits + (DibHeight()-y-1) * iWidth + x;
		}
   		case 24 :
		{
	    	return m_pBits + (DibHeight()-y-1) * iWidth + (x * 3);
		}
   		default :
		{
			return NULL;
		}
	}

}

// Get the bounding rectangle.
void ChDibBmp::GetRect(CRect* pRect)
{
    pRect->top = 0;
    pRect->left = 0;
    pRect->bottom = (int)DibHeight();
    pRect->right = (int)DibWidth();
}

CPalette*	ChDibBmp::GetDIBPalette()
{
	if ( m_pBMI->bmiHeader.biBitCount < 16 && m_pPalette == 0 )
	{ // Create a palette based on the DIB color table
		RGBQUAD* pClrIn = GetClrTabAddress();

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
		m_pPalette = new CPalette;
		ASSERT( m_pPalette );
		m_pPalette->CreatePalette( lpPalette );
		delete [] lpPalette;
	}

	return m_pPalette;
}


// Copy a rectangle of the DIB to another DIB.
// Note: We only support 8bpp DIBs here.
void ChDibBmp::CopyBits(ChDibBmp* pdibDest, 
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
    int iScanS = (int)StorageWidth();
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
		int iIndex = -1;
	    RGBQUAD*  prgb = GetClrTabAddress();
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
			RGBQUAD * pSrcClrTbl = GetClrTabAddress();
			RGBQUAD * pDstClrTbl = pdibDest->GetClrTabAddress();
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

			delete pdibDest->m_pPalette;
			pdibDest->m_pPalette = 0;

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
		int iIndex = -1;
	    RGBQUAD*  prgb = GetClrTabAddress();
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

	    prgb = GetClrTabAddress();
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

         

// Copy a rectangle of the DIB to a native DC.
void ChDibBmp::CopyBits(CDC* pDC, 
                    int xd, int yd,
                    int w,  int h,
                    int xs, int ys,
                    COLORREF clrTrans)
{
    ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    // Test for silly cases.
    if (w == 0 || h == 0) return;

	ChDibBmp osb;		// buffer to do our work
	osb.Create(w, h);
	osb.CopyDCBits(pDC, 0, 0, w, h, xd, yd); // get bkgd bits of dc
	CopyBits(&osb, 0, 0, w, h, xs, ys, clrTrans);
	osb.Draw(pDC, xd, yd);  

}          

// Copy a rectangle of a native DC to the DIB.
void ChDibBmp::CopyDCBits(CDC* pDC, 
                    int xd, int yd,
                    int w,  int h,
                    int xs, int ys)
{
    ASSERT(m_pBMI->bmiHeader.biBitCount == 8);
    // Test for silly cases.
    if (w == 0 || h == 0) return;

	CDC tmpDC; 
	tmpDC.CreateCompatibleDC(pDC);
	CBitmap bmp, *pOldBmp;

	bmp.CreateCompatibleBitmap(pDC, (int)DibWidth(), (int)DibHeight());

	pOldBmp = tmpDC.SelectObject(&bmp);

    tmpDC.StretchBlt(xd,                        // Destination x
                    yd,                        // Destination y
                    w,               // Destination width
                    h,              // Destination height
                    pDC,
                    xs,                        // Source x
                    ys,                        // Source y
                    w,               // Source width
                    h,              // Source height
					SRCCOPY);


	tmpDC.SelectObject(pOldBmp);

	#if 0
	::GetDIBits( tmpDC.GetSafeHendle(), 
						bmp.GetSafeHandle(), 	// Bitmap
						yd, 					// Start scan line
						h, 						// Number of scan line
        	            GetBitsAddress(),       // Pointer to bits
    	                GetBitmapInfoAddress(), // BITMAPINFO
	                    DIB_RGB_COLORS );       // Options
	#endif

}          

// Save a DIB to a disk file.
// This is somewhat simplistic because we only deal with 256 color DIBs
// and we always write a 256 color table.
BOOL ChDibBmp::Save(CFile* fp)
{
    BITMAPFILEHEADER bfh;

    // Construct the file header.
    bfh.bfType = 0x4D42; // 'BM'
    bfh.bfSize = 
        sizeof(BITMAPFILEHEADER) +
        sizeof(BITMAPINFOHEADER) +
        256 * sizeof(RGBQUAD) +
        StorageWidth() * DibHeight();
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits =
        sizeof(BITMAPFILEHEADER) +
        sizeof(BITMAPINFOHEADER) +
        256 * sizeof(RGBQUAD);

    // Write the file header.
    int iSize = sizeof(bfh);
    TRY {
        fp->Write(&bfh, iSize);
    } CATCH(CFileException, e) {
        TRACE("Failed to write file header");
        return false;
    } END_CATCH

    // Write the BITMAPINFO structure.
    // Note: we assume that there are always 256 colors in the
    // color table.
    ASSERT(m_pBMI);
    iSize = 
        sizeof(BITMAPINFOHEADER) +
        256 * sizeof(RGBQUAD);
    TRY {
        fp->Write(m_pBMI, iSize);
    } CATCH(CFileException, e) {
        TRACE("Failed to write BITMAPINFO");
        return false;
    } END_CATCH

    // Write the bits.
    iSize = (int)(StorageWidth() * DibHeight());
    TRY {
        fp->Write(m_pBits, iSize);
    } CATCH(CFileException, e) {
        TRACE("Failed to write bits");
        return false;
    } END_CATCH

    return true;
}

// Save a DIB to a disk file. If no file name is given, show
// a File Save dialog to get one.
BOOL ChDibBmp::Save(LPSTR pszFileName)
{
    CString strFile;    

    if ((pszFileName == NULL) 
    ||  (strlen(pszFileName) == 0)) {

        // Show a File Save dialog to get the name.
        CFileDialog dlg   (false,   // Save
                           NULL,    // No default extension
                           NULL,    // No initial file name
                           OFN_OVERWRITEPROMPT
                             | OFN_HIDEREADONLY,
                           "Image files (*.DIB, *.BMP)|*.DIB;*.BMP|All files (*.*)|*.*||");
        if (dlg.DoModal() == IDOK) {
            strFile = dlg.GetPathName();
        } else {
            return false;
        }
    } else {
    
        // Copy the supplied file path.
        strFile = pszFileName;                    
    }

    // Try to open the file for write access.
    CFile file;
    if (!file.Open(strFile,
                    CFile::modeReadWrite
                     | CFile::modeCreate
                     | CFile::shareExclusive)) {
        AfxMessageBox("Failed to open file");
        return false;
    }

    BOOL bResult = Save(&file);
    file.Close();
    if (!bResult) AfxMessageBox("Failed to save file");
    return bResult;
}

// $Log$
