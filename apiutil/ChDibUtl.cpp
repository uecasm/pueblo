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

	ChImgUtil class implementation

----------------------------------------------------------------------------*/

#include "headers.h"
#if !defined(CH_STATIC_LINK)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChImgUtil.h>
#include "MemDebug.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*----------------------------------------------------------------------------
	ChImgUtil class static values
----------------------------------------------------------------------------*/
#if defined( CH_MSW )

RGBQUAD			ChImgUtil::m_grayscaleClrTbl[256];
int				ChImgUtil::m_iGrayscaleClrTblNumEntries;
RGBQUAD			ChImgUtil::m_clrTbl[256];
int				ChImgUtil::m_iClrTblNumEntries;
CPalette		ChImgUtil::m_stdPal;			// for standard palette
CPalette		ChImgUtil::m_grayScalePal;			// for standard palette



/*----------------------------------------------------------------------------
	ChImgUtil class utility functions
----------------------------------------------------------------------------*/
int   ChImgUtil::GetGrayScaleClrTblNumEntries()			
{ 

	if ( 0 == m_iGrayscaleClrTblNumEntries )
	{
		ChImgUtil::GetGrayScaleColorTable();
	}

	return m_iGrayscaleClrTblNumEntries;

}


RGBQUAD*   ChImgUtil::GetGrayScaleColorTable()			
{ 
	if ( 0 == m_iGrayscaleClrTblNumEntries )
	{
		GetGrayScalePalette();
	}

	return m_grayscaleClrTbl;

} 

CPalette*   ChImgUtil::GetGrayScalePalette()			
{ 
	if ( !m_grayScalePal.GetSafeHandle() )
	{
		LoadPalette( IDR_GRAYSCALE_PAL, &m_grayScalePal,  
					m_grayscaleClrTbl, m_iGrayscaleClrTblNumEntries );
	}

	return &m_grayScalePal;

}




RGBQUAD*   ChImgUtil::GetStdColorTable()			
{ 
	if ( 0 == m_iClrTblNumEntries )
	{
		GetStdPalette();
	}
	return m_clrTbl;

}




int   ChImgUtil::GetStdClrTblNumEntries()			
{ 

	if ( 0 == m_iClrTblNumEntries )
	{
		GetStdPalette();
	}

	return m_iClrTblNumEntries;

}

CPalette * ChImgUtil::GetStdPalette()
{ 
	if ( !m_stdPal.GetSafeHandle() )
	{
		LoadPalette( IDR_STDDIB_PAL, &m_stdPal,  
					m_clrTbl, m_iClrTblNumEntries );
	}

	return &m_stdPal; 
}

void ChImgUtil::LoadPalette( UINT uResID, CPalette* pPal,  RGBQUAD* pClrTbl, int& iEntries )
{
	if ( !pPal->GetSafeHandle() )
	{

	   	HINSTANCE hInst;

		#if !defined(CH_STATIC_LINK)
		hInst = PbUtil32DLL.hModule;
		#else
		hInst = AfxGetInstanceHandle( );
		#endif

	    HRSRC hrsrc = ::FindResource(hInst, MAKEINTRESOURCE(uResID), "PAL");

	    if (!hrsrc) 
	    {
	        TRACE("PAL resource not found\n");
	        return;
	    }
	    HGLOBAL hg = LoadResource(hInst, hrsrc);
	    if (!hg) 
	    {
	        TRACE("Failed to load PAL resource\n");
	        return;
	    }
	    BYTE* pRes = (BYTE*) LockResource(hg);
	    ASSERT(pRes);		

		// Add the header size
		pRes +=  0x14;

 
	    LOGPALETTE* pLogPal = (LOGPALETTE*)pRes;
	    if (pLogPal->palVersion != 0x300) 
	    {
	        TRACE("Invalid palette version number\n");
	        return ;
	    }
	    // Get the number of entries.
	    int iColors = pLogPal->palNumEntries;
    
	    if (iColors <= 0) 
	    {
	        TRACE("No colors in palette\n");
	        return ;
	    }


	    pPal->CreatePalette(pLogPal);


		for (  int i = 0; i < pLogPal->palNumEntries && i < 256; i++ )
		{
	        pClrTbl[i].rgbRed = pLogPal->palPalEntry[i].peRed;
	        pClrTbl[i].rgbGreen = pLogPal->palPalEntry[i].peGreen;
	        pClrTbl[i].rgbBlue = pLogPal->palPalEntry[i].peBlue;
	        pClrTbl[i].rgbReserved = 0;
		}

		iEntries = pLogPal->palNumEntries;


		UnlockResource( hg );
		FreeResource( hrsrc );

	}

}

#endif

int ChImgUtil::MaxDeviceColors()
{
	static int iMaxColor = 0;

	if ( !iMaxColor )
	{
		#ifdef CH_MSW
			  HWND hWnd = ::GetDesktopWindow();
			  HDC hDC   = ::GetDC( hWnd );
			  iMaxColor = ::GetDeviceCaps( hDC, NUMCOLORS);
			  ::ReleaseDC( hWnd, hDC );
		#else
			cerr << "Not implemented: " << __FILE__ << ":" << __LINE__ << endl;
		#endif
	}

 	return iMaxColor;
}


bool ChImgUtil::IsWinDIB(BITMAPINFOHEADER *pBIH)
{
    ASSERT(pBIH);
    if (((BITMAPCOREHEADER*)pBIH)->bcSize == sizeof(BITMAPCOREHEADER))
    {
        return false;
    }
    return true;
}


int ChImgUtil::NumDIBColorEntries(BITMAPINFO* pBmpInfo) 
{
    BITMAPINFOHEADER* pBIH;
    BITMAPCOREHEADER* pBCH;
    int iColors, iBitCount;

    ASSERT(pBmpInfo);

    pBIH = &(pBmpInfo->bmiHeader);
    pBCH = (BITMAPCOREHEADER*) pBIH;

    // Start off by assuming the color table size from
    // the bit-per-pixel field.
    if (IsWinDIB(pBIH)) 
    {
        iBitCount = pBIH->biBitCount;
    } 
    else 
    {
        iBitCount = pBCH->bcBitCount;
    }

    switch (iBitCount) 
    {
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
    if (IsWinDIB(pBIH) && (pBIH->biClrUsed != 0)) 
    {
        iColors = (int)pBIH->biClrUsed;
    }

    // BUGFIX 18 Oct 94 NigelT
    // Make sure the value is reasonable since some products
    // will write out more then 256 colors for an 8 bpp DIB!!!
    int iMax = 0;
    switch (iBitCount) 
    {
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
    if (iMax) 
    {
        if (iColors > iMax) 
        {
            TRACE("Invalid color count\n");
            iColors = iMax;
        }
    }

    return iColors;
}
