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

   ChDibPal class implementation

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"

#include <ChDibBmp.h>

#if defined(CH_MSW)
#if !defined(CH_ARCH_16)

//#if defined( CH_VRML_VIEWER )
//#include "ChGrVw.h"
//#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ChDibPal

ChDibPal::ChDibPal()
{
}

ChDibPal::~ChDibPal()
{
}

// Create a palette from the color table in a DIB.
BOOL ChDibPal::Create(ChDibBmp* pDIB)
{
    DWORD dwColors = pDIB->GetNumClrEntries();
    // Check the DIB has a color table.
    if (! dwColors) {
        TRACE("No color table\n");   
        return false;
    }

    // Get a pointer to the RGB quads in the color table.
    RGBQUAD* pRGB = pDIB->GetClrTabAddress();

    // Allocate a log pal and fill it with the color table info.
    LOGPALETTE* pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) 
                     + dwColors * sizeof(PALETTEENTRY));
    if (!pPal) {
        TRACE("Out of memory for logpal\n");
        return false;
    }
    pPal->palVersion = 0x300;              // Windows 3.0
    pPal->palNumEntries = (WORD) dwColors; // Table size
    for (DWORD dw=0; dw<dwColors; dw++) {
        pPal->palPalEntry[dw].peRed = pRGB[dw].rgbRed;
        pPal->palPalEntry[dw].peGreen = pRGB[dw].rgbGreen;
        pPal->palPalEntry[dw].peBlue = pRGB[dw].rgbBlue;
        pPal->palPalEntry[dw].peFlags = 0;
    }
    BOOL bResult = CreatePalette(pPal);
    free(pPal);
    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// ChDibPal commands

int ChDibPal::GetNumColors()
{
    int iColors = 0;
    if (!GetObject(sizeof(iColors), &iColors)) {
        TRACE("Failed to get num pal colors\n");
        return 0;
    }
    return iColors;
}

void ChDibPal::Draw(CDC* pDC, CRect* pRect, BOOL bBkgnd)
{
    int iColors = GetNumColors();
    CPalette* pOldPal = pDC->SelectPalette(this, bBkgnd);
    pDC->RealizePalette();
    int i, j, top, left, bottom, right;
    for (j=0, top=0; j<16 && iColors; j++, top=bottom) {
        bottom = (j+1) * pRect->bottom / 16 + 1;
        for(i=0, left=0; i<16 && iColors; i++, left=right) {
            right = (i+1) * pRect->right / 16 + 1;
            CBrush br (PALETTEINDEX(j * 16 + i));
            CBrush* brold = pDC->SelectObject(&br);
            pDC->Rectangle(left-1, top-1, right, bottom);
            pDC->SelectObject(brold);
            iColors--;
        }
    }
    pDC->SelectPalette(pOldPal, false);
}

BOOL ChDibPal::SetSysPalColors()
{
    BOOL bResult = false;
    int i, iSysColors, iPalEntries;
    HPALETTE hpalOld;

    // Get a screen DC to work with.
    HWND hwndActive = ::GetActiveWindow();
    HDC hdcScreen = ::GetDC(hwndActive);
    ASSERT(hdcScreen);

    // Make sure we are on a palettized device.
    if (!(GetDeviceCaps(hdcScreen, RASTERCAPS) & RC_PALETTE)) {
        TRACE("Not a palettized device\n");
        goto abort;
    }

    // Get the number of system colors and the number of palette
    // entries. Note that on a palletized device the number of
    // colors is the number of guaranteed colors, i.e., the number
    // of reserved system colors.
    iSysColors = GetDeviceCaps(hdcScreen, NUMCOLORS);
    iPalEntries = GetDeviceCaps(hdcScreen, SIZEPALETTE);

    // If there are more than 256 colors we are wasting our time.
    if (iSysColors > 256) goto abort;

    // Now we force the palette manager to reset its tables so that
    // the next palette to be realized will get its colors in the order they are 
    // in the logical palette. This is done by changing the number of
    // reserved colors.
    SetSystemPaletteUse(hdcScreen, SYSPAL_NOSTATIC);
    SetSystemPaletteUse(hdcScreen, SYSPAL_STATIC);

    // Select our palette into the screen DC and realize it so that
    // its colors will be entered into the free slots in the physical palette.
    hpalOld = ::SelectPalette(hdcScreen,
                              (HPALETTE)m_hObject, // Our hpal
                              false);
    ::RealizePalette(hdcScreen);
    // Now replace the old palette (but don't realize it)
    ::SelectPalette(hdcScreen, hpalOld, false);

    // The physical palette now has our colors set in place and its own
    // reserved colors at either end. We can grab the lot now.
    PALETTEENTRY pe[256];
    GetSystemPaletteEntries(hdcScreen, 
                            0,
                            iPalEntries,
                            pe);

    // Set the PC_NOCOLLAPSE flag for each of our colors so that the GDI
    // won't merge them. Be careful not to set PC_NOCOLLAPSE for the 
    // system color entries so that we won't get multiple copies of these
    // colors in the palette when we realize it.
    for (i = 0; i < iSysColors/2; i++) {
        pe[i].peFlags = 0;
    }
    for (; i < iPalEntries-iSysColors/2; i++) {
        pe[i].peFlags = PC_NOCOLLAPSE;
    }
    for (; i < iPalEntries; i++) {
        pe[i].peFlags = 0;
    }

    // Resize the palette in case it was smaller.
    ResizePalette(iPalEntries);

    // Update the palette entries with what is now in the physical palette.
    SetPaletteEntries(0, iPalEntries, pe);
    bResult = true;

abort:
    ::ReleaseDC(hwndActive, hdcScreen);
    return bResult;
}

// Load a palette from a named file.
BOOL ChDibPal::Load(char* pszFileName)
{
    CString strFile;    

    if ((pszFileName == NULL) 
    ||  (strlen(pszFileName) == 0)) {

        // Show an File Open dialog to get the name.
        CFileDialog dlg   (true,    // Open
                           NULL,    // No default extension
                           NULL,    // No initial file name
                           OFN_FILEMUSTEXIST
                             | OFN_HIDEREADONLY,
                           "Palette files (*.PAL)|*.PAL|All files (*.*)|*.*||");
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
    CFile file;
    if (!file.Open(strFile,
                    CFile::modeRead | CFile::shareDenyWrite)) {
        AfxMessageBox("Failed to open file");
        return false;
    }

    BOOL bResult = Load(&file);
    file.Close();
    if (!bResult) AfxMessageBox("Failed to load file");
    return bResult;
}

// Load a palette from an open CFile object.
BOOL ChDibPal::Load(CFile* fp)
{
    return Load((HANDLE)fp->m_hFile);
}

// Load a palette from an open file handle.
BOOL ChDibPal::Load(HANDLE hFile)
{
    HMMIO hmmio;
    MMIOINFO info;
    memset(&info, 0, sizeof(info));
    info.adwInfo[0] = (DWORD)hFile;
    hmmio = mmioOpen(NULL,
                     &info,
                     MMIO_READ | MMIO_ALLOCBUF);
    if (!hmmio) {
        TRACE("mmioOpen failed\n");
        return false;
    }
    BOOL bResult = Load(hmmio);
    mmioClose(hmmio, MMIO_FHOPEN);
    return bResult;
}

// Load a palette from an open MMIO handle.
BOOL ChDibPal::Load(HMMIO hmmio)
{
    // Check whether it's a RIFF PAL file.
    MMCKINFO ckFile;
    ckFile.fccType = mmioFOURCC('P','A','L',' ');
    if (mmioDescend(hmmio,
                    &ckFile,
                    NULL,
                    MMIO_FINDRIFF) != 0) {
        TRACE("Not a RIFF or PAL file\n");
        return false;
    }
    // Find the 'data' chunk.
    MMCKINFO ckChunk;
    ckChunk.ckid = mmioFOURCC('d','a','t','a');
    if (mmioDescend(hmmio,
                    &ckChunk,
                    &ckFile,
                    MMIO_FINDCHUNK) != 0) {
        TRACE("No data chunk in file\n");
        return false;
    }
    // Allocate some memory for the data chunk.
    int iSize = ckChunk.cksize;
    void* pdata = malloc(iSize);
    if (!pdata) {
        TRACE("No mem for data\n");
        return false;
    }
    // Read the data chunk.
    if (mmioRead(hmmio,
                 (char*)pdata,
                 iSize) != iSize) {
        TRACE("Failed to read data chunk\n");
        free(pdata);
        return false;
    }
    // The data chunk should be a LOGPALETTE structure
    // that we can create a palette from.
   LOGPALETTE* pLogPal = (LOGPALETTE*)pdata;
   if (pLogPal->palVersion != 0x300) {
      TRACE("Invalid version number\n");
        free(pdata);
        return false;
   }
   // Get the number of entries.
   int iColors = pLogPal->palNumEntries;
   if (iColors <= 0) {
      TRACE("No colors in palette\n");
        free(pdata);
        return false;
   }
   return CreatePalette(pLogPal);
}

// Save a palette to an open CFile object.
BOOL ChDibPal::Save(CFile* fp)
{
    return Save((HANDLE)fp->m_hFile);
}

// Save a palette to an open file handle.
BOOL ChDibPal::Save(HANDLE hFile)
{
    HMMIO hmmio;
    MMIOINFO info;
    memset(&info, 0, sizeof(info));
    info.adwInfo[0] = (DWORD)hFile;
    hmmio = mmioOpen(NULL,
                     &info,
                     MMIO_WRITE | MMIO_CREATE | MMIO_ALLOCBUF);
    if (!hmmio) {
        TRACE("mmioOpen failed\n");
        return false;
    }
    BOOL bResult = Save(hmmio);
    mmioClose(hmmio, MMIO_FHOPEN);
    return bResult;
}

// Save a palette to an open MMIO handle.
BOOL ChDibPal::Save(HMMIO hmmio)
{
   // Create a RIFF chunk for a PAL file.
    MMCKINFO ckFile;
   ckFile.cksize = 0; // Corrected later
    ckFile.fccType = mmioFOURCC('P','A','L',' ');
    if (mmioCreateChunk(hmmio,
                        &ckFile,
                        MMIO_CREATERIFF) != 0) {
        TRACE("Failed to create RIFF-PAL chunk\n");
        return false;
    }
   // Create the LOGPALETTE data which will become
   // the data chunk.
   int iColors = GetNumColors();
   ASSERT(iColors > 0);
   int iSize = sizeof(LOGPALETTE)
            + (iColors-1) * sizeof(PALETTEENTRY);
   LOGPALETTE* plp = (LOGPALETTE*) malloc(iSize);
   ASSERT(plp);
   plp->palVersion = 0x300;
   plp->palNumEntries = iColors;
   GetPaletteEntries(0, iColors, plp->palPalEntry);
   // create the data chunk.
    MMCKINFO ckData;
   ckData.cksize = iSize; 
    ckData.ckid = mmioFOURCC('d','a','t','a');
    if (mmioCreateChunk(hmmio,
                        &ckData,
                        0) != 0) {
        TRACE("Failed to create data chunk\n");
        return false;
    }
   // Write the data chunk.
    if (mmioWrite(hmmio,
                 (char*)plp,
                 iSize) != iSize) {
        TRACE("Failed to write data chunk\n");
        free(plp);
        return false;
    }
   free(plp);
   // Ascend from the data chunk which will correct the length.
   mmioAscend(hmmio, &ckData, 0);
        // Ascend from the RIFF-PAL chunk.
   mmioAscend(hmmio, &ckFile, 0);

   return true;
}
#endif //!defined(CH_ARCH_16)
#endif // CH_MSW

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:14  uecasm
// Import of source tree as at version 2.53 release.
//
