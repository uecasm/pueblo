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

	Header file for the ChDibImage class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHDIBIMAGE_H )
#define _CHDIBIMAGE_H

#include <ChImgConsumer.h>

class ChDib;

typedef struct tagChDibFrame
{

	ChImageFrameInfo 		m_frameInfo;
    BITMAPINFO* 			m_pBMI;         // Pointer to BITMAPINFO struct
    BYTE* 					m_pBits;        // Pointer to the bits
    BOOL  					m_bMyBits;      // true if DIB owns Bits memory
	CPalette*				m_pPalette;		// Palette to use for 8 bit DIBs
	ChDib*					m_pTransDib;
	void*					m_pMask;

} ChDibFrame, FAR* pChDibFrame;


class CH_EXPORT_CLASS ChDib : public ChImageConsumer 
{
	public :
		ChDib( );
		virtual ~ChDib();

		// Overidable methods of ChImageConsumer
		virtual bool NewImage( pChImageInfo pImage );
	    virtual bool Create( int iFrame, BITMAPINFO* pBMI, BYTE* pBits); // Create from existing mem
		virtual bool Create( pChImageFrameInfo pFrameInfo, int iBitCount = 8 );
		virtual bool SetColorTable( int iFrame, RGBQUAD* pColorTbl, int iSize );
		virtual bool SetScanLine( int iFrame, int iScanLine, 
						BYTE* pBits, int iBufferLength, int iFormat );

		// Methods of ChDib  class 
								// Pointer to bitmap info
		bool Create( int iFrame, int iWidth, int iHeight, int iBitCount );
	    
	    BITMAPINFO* GetBitmapInfoAddress()  	{ return m_pFrameList[m_iCurrentFrame].m_pBMI; } 
								// Pointer to the bits
	    void* GetBitsAddress() 			        {return m_pFrameList[m_iCurrentFrame].m_pBits;} 
	    						// Pointer to color table 
	    RGBQUAD* GetClrTabAddress( int iFrame )	 	        
	    					{return (LPRGBQUAD)(((BYTE*)(m_pFrameList[iFrame].m_pBMI)) 
	            							 + sizeof(BITMAPINFOHEADER));} 
	    int GetNumClrEntries();                     // Number of color table entries
		void* GetPixelAddress(int x, int y);

		CPalette*	GetDIBPalette();

		COLORREF GetTransparentColor( ) { return m_pFrameList[m_iCurrentFrame].m_frameInfo.colorTransparent; }
		void  	 SetTransparentColor( COLORREF luColor )	{ }
	   
		bool IsGrayscale() { return m_pFrameList[m_iCurrentFrame].m_frameInfo.luAttrs 
											& ChImageConsumer::grayScale; }

		bool IsTransparent() { return  (m_pFrameList[m_iCurrentFrame].m_frameInfo.luAttrs 
									& ChImageConsumer::imgTransparent) != 0; }

		pChImageInfo GetImageInfo()		{ return &m_imgInfo; }
		pChImageFrameInfo GetFrameInfo( int iFrame )  
									{ return &m_pFrameList[iFrame].m_frameInfo; }


		int  GetCurrentFrame() 		{ return m_iCurrentFrame; }	 
		virtual void NextFrame();
		void SetFrame( int iFrame );
		int	 GetTotalFrames()			{ return m_iNumFrames; }
		long StorageWidth( int iFrame = -1 );

	    virtual void Draw(CDC* pDC, int x, int y);
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, COLORREF  clrMask );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, CBrush*  pbrMask );
		virtual void Draw(CDC* pDC, int x, int y, COLORREF clrTrans, ChDib* pdibMask );
	    virtual BOOL MapColorsToPalette(CPalette* pPal);
		virtual BOOL SetSize( long lWidth, long lHeight );
	    virtual long GetWidth() {return DibWidth();}   // Image width
	    virtual long GetHeight() {return DibHeight();} // Image height
	    virtual void CopyBits(ChDib* pDIB, 
                          int xd, int yd,
                          int w,  int h,
                          int xs, int ys,
                          COLORREF clrTrans = 0xFFFFFFFF);

	private :
		void AllocateFrame( int iFrame );
		void DrawInBuffer(CDC *pDC, int x, int y, HBRUSH hBrMask, ChDib& target);

	protected:
		ChImageInfo			m_imgInfo;
		int					m_iNumFrames;
		int					m_iCurrentFrame;
		int					m_iFrameListSize;
  		pChDibFrame   		m_pFrameList;

	    long DibWidth()
	        {return m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biWidth;}
	    long DibHeight() 
	        {return m_pFrameList[m_iCurrentFrame].m_pBMI->bmiHeader.biHeight;}

		virtual void CreateBackgroundImage(CDC *pDC, int x, int y, COLORREF clrMask);
		virtual void CreateBackgroundImage(CDC *pDC, int x, int y, CBrush *pbrMask);
		virtual void CreateBackgroundImage(CDC *pDC, int x, int y, ChDib *pdibMask);
};

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:36  uecasm
// Import of source tree as at version 2.53 release.
//

#endif //  !defined( _CHDIBIMAGE_H )
