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

	Header file for the image consumer class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHIMGCONSUMER_H )
#define _CHIMGCONSUMER_H


typedef struct tagChImageInfo
{
	bool		boolMultiframe;
	int			iWidth;
	int			iHeight;
	int			iLoopCount;
	COLORREF	colorBack;

} ChImageInfo, FAR* pChImageInfo;

typedef struct tagChImageFrameInfo
{

	int 		iFrame;
	int			iLeft;
	int			iTop;
	int			iWidth;
	int			iHeight;
	COLORREF	colorTransparent;
	int			iTransparentIndex;
	int			iExposture;
	chuint32	luAttrs;

} ChImageFrameInfo, FAR* pChImageFrameInfo;


class CH_EXPORT_CLASS ChImageConsumer 
{
	public :
		enum  tagTypes {  format8Bit = 1, format24RGB = 2, format24BGR = 3 }; 
		enum  tagAttrs {  grayScale = 0x1, imgTransparent = 0x2,
						  userInput = 0x04 }; 

		ChImageConsumer() 
			{
			}
		virtual ~ChImageConsumer()  {}

		// method that should be provided by the consumer of the decoded image
		virtual bool NewImage( pChImageInfo pImage ) = 0;
		virtual bool Create( int iFrame, BITMAPINFO* pBMI, BYTE* pBits ) = 0;
		virtual bool Create( pChImageFrameInfo pFrameInfo, int iBitCount = 8 ) = 0;
		virtual bool SetColorTable( int iFrame, RGBQUAD* pColorTbl, int iColors ) = 0;
		virtual bool SetScanLine( int iFrame, int iScanLine,
					BYTE* pBits, int iBufferLength, int iFormat ) = 0;
	protected :
		
};

// $Log$

#endif //  !defined( _CHIMGCONSUMER_H )

