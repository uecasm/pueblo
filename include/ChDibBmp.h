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

	ChDibBmp class definition

----------------------------------------------------------------------------*/

// $Header$


#if !defined( _CHDIBBMP_H )
#define _CHDIBBMP_H

#ifdef CH_MSW
#include<mmsystem.h>
#endif

#ifdef CH_UNIX
#include <ChDC.h>

class RGBQUAD {
	public:
		chuint8 rgbRed;
		chuint8 rgbGreen;
		chuint8 rgbBlue;
};
typedef RGBQUAD *LPRGBQUAD;

class BITMAPINFO {

};
#endif

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#if defined(CH_MSW)
//#if !defined(CH_ARCH_16)


/*----------------------------------------------------------------------------
	DIB class
----------------------------------------------------------------------------*/

#if !defined(CH_ARCH_16)

// Define the CreateDIBSection function.
typedef HBITMAP (APIENTRY CDSPROC)
                (HDC hDC, 
                 BITMAPINFO* pbmi,
                 UINT iUsage, 
                 VOID** ppvBits,
                 HANDLE hSection,
                 DWORD dwOffset);
#endif

typedef int LZHANDLE;

class CH_EXPORT_CLASS ChDibBmp : public CObject
{
    DECLARE_SERIAL(ChDibBmp)
public:
    ChDibBmp();
    ~ChDibBmp();

	enum tagOptions { loadAuto = 0x01, load8Bit = 0x02, load24Bit = 0x04, grayScale = 0x08,
					  dibTransperent = 0x10, dibTransperentCached = 0x20 };

    BITMAPINFO* GetBitmapInfoAddress()
        {return m_pBMI;}                        // Pointer to bitmap info
    void* GetBitsAddress()
        {return m_pBits;}                       // Pointer to the bits
    RGBQUAD* GetClrTabAddress()
        {return (LPRGBQUAD)(((BYTE*)(m_pBMI)) 
            + sizeof(BITMAPINFOHEADER));}       // Pointer to color table
    int GetNumClrEntries();                     // Number of color table entries
    BOOL Create(int width, int height, int iBitCount = 8 );         // Create a new DIB
    BOOL Create(BITMAPINFO* pBMI, BYTE* pBits, BOOL boolCopyBits = false ); // Create from existing mem
    void* GetPixelAddress(int x, int y);
    virtual BOOL Load(CFile* fp);               // Load from file
    virtual BOOL Load(const char* pszFileName = NULL, chuint32 flOption = loadAuto );// Load DIB from disk file
    virtual BOOL Load(WORD wResid, HINSTANCE hInst = 0); // Load DIB from resource
    virtual BOOL Load(LZHANDLE lzHdl);             // Load DIB from LZ File
    virtual BOOL Save(char* pszFileName = NULL);// Save DIB to disk file
    virtual BOOL Save(CFile* fp);               // Save to file
    virtual void Serialize(CArchive& ar);
    virtual void Draw(CDC* pDC, int x, int y);
	virtual BOOL SetSize( long lWidth, long lHeight );
    virtual long GetWidth() {return DibWidth();}   // Image width
    virtual long GetHeight() {return DibHeight();} // Image height
    virtual BOOL MapColorsToPalette(CPalette* pPal);
    virtual void GetRect(CRect* pRect);
    virtual void CopyBits(ChDibBmp* pDIB, 
                          int xd, int yd,
                          int w,  int h,
                          int xs, int ys,
                          COLORREF clrTrans = 0xFFFFFFFF);
  	// Not implemented yet - jwd - support for tranparent blt to dc
  	virtual void CopyBits(CDC* pDC, 
                          int xd, int yd,
                          int w,  int h,
                          int xs, int ys,
                          COLORREF clrTrans = 0xFFFFFFFF);
    virtual void CopyDCBits(CDC* pDC, 
                          int xd, int yd,
                          int w,  int h,
                          int xs, int ys);
  	// End: Not implemented yet


	CPalette*	GetDIBPalette();
	void		ZapData()
					{
					    m_pBMI = NULL;
					    m_pBits = NULL;
					}

protected:
    BITMAPINFO* 		m_pBMI;         // Pointer to BITMAPINFO struct
    BYTE* 				m_pBits;        // Pointer to the bits
    BOOL  				m_bMyBits;      // true if DIB owns Bits memory
	CPalette*			m_pPalette;		// Palette to use for 8 bit DIBs
 	chuint32			m_flOptions;		// option for load


private:
    long DibWidth()
        {return m_pBMI->bmiHeader.biWidth;}
    long DibHeight() 
        {return m_pBMI->bmiHeader.biHeight;}
    long StorageWidth();
};

/*----------------------------------------------------------------------------
	ChDibPal class
----------------------------------------------------------------------------*/


class CH_EXPORT_CLASS ChDibPal : public CPalette
{
public:
    ChDibPal();
    ~ChDibPal();
    BOOL Create(ChDibBmp *pDIB);            // Create from a DIB
    int GetNumColors();                 // Get the number of colors
                                        // in the palette.
    void Draw(CDC* pDC, CRect* pRect, BOOL bBkgnd = false); 
    BOOL SetSysPalColors();
    BOOL Load(char* pszFileName = NULL);
    BOOL Load(CFile* fp);  
    BOOL Load(HANDLE hFile);
    BOOL Load(HMMIO hmmio);
    BOOL Save(CFile* fp);  
    BOOL Save(HANDLE hFile);
    BOOL Save(HMMIO hmmio);
};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif
//#endif //!defined(CH_ARCH_16)
#endif // CH_MSW

#ifdef CH_UNIX
class ChDibBmp
{
  public:
    GetWidth() { return 1; };
    GetHeight() { return 1; };
    Draw(ChClientDC *pDC, int x, int y) {
	cerr << "XXX ChDib::Draw( x=" << x << ", y=" << y << " )" << endl;
    };
    BOOL Create(int width, int height, int iBitCount = 8 ) {        // Create a new DIB
	cerr << "XXX ChDib::Create( width=" << width << ", height=" << height << " )" << endl;
    }
    BOOL Create(BITMAPINFO* pBMI, BYTE* pBits) {
	cerr << "XXX ChDib::Create()" << endl;
    }
    unsigned char *GetBitsAddress() { return m_bits; };
    enum { loadAuto, load8Bit, load24Bit };
    RGBQUAD* GetClrTabAddress()
        {return (LPRGBQUAD)(&m_BMI);};  // XXX Junk.

  private:
    unsigned char m_bits[1];
    unsigned char m_BMI[1024];  // XXX This is just a placeholder
};
#endif // CH_UNIX

// $Log$

#endif	// !defined( _CHGRAPHX_H )
