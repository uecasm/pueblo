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

	Chaco GIF decoder interface

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CHGIFDECODER_H_ )
#define   CHGIFDECODER_H_

#include <fstream>
#include <ChImgConsumer.h>
#include <ChImageDecoder.h>

#if defined( CH_ARCH_16 )
#pragma pack( 1 )		// set the packing to byte boundry
#else
#pragma pack( push, 1 )		// set the packing to byte boundry
#endif

typedef struct tagGIFHeader
{
	BYTE 	strSignature[3];
	BYTE 	strVersion[3];
	WORD 	wScreenWidth;
	WORD	wScreenHeight;
	BYTE	bPacked;
	BYTE	bBackgroundColor;
	BYTE	bAspectRatio;
} GIFHeader, FAR* pGIFHeader;

typedef struct tagGIFColorTable
{
	BYTE	bRed;
	BYTE	bGreen;
	BYTE	bBlue;
} GIFColorTable , FAR* pGIFColorTable;

typedef struct tagGIFImageDescriptor
{
	BYTE  	bSeparator;
	WORD	wLeft;
	WORD	wTop;
	WORD	wWidth;
	WORD	wHeight;
	BYTE	bPacked;

} GIFImageDescriptor, FAR* pGIFImageDescriptor;

typedef struct tagGIFGraphicControlBloack
{
	BYTE 	bPacked;
	WORD	wDelayTime;
	BYTE	bColorIndex;
	BYTE	bTerminator;

} GIFGraphicControlBlock, *pGIFGraphicControlBloack;
          
#if defined( CH_ARCH_16 )
#pragma pack()			//  restore the structure packing
#else
#pragma pack( pop )		// restore the structure packing
#endif
          

//class fstream;

class CH_EXPORT_CLASS ChGifDecoder : public ChImageDecoder
{
	public:
	    ChGifDecoder( ChImageConsumer* pConsumer );
	    virtual ~ChGifDecoder();
	    virtual bool Load( const char* pszFileName = NULL, 
	    					chuint32 flOption = loadAuto );// Load GIF from disk file

			// the following two methods are not implemented and will always fail:
	    virtual bool Load(WORD wResid, HINSTANCE hInst = 0); // Load GIF from resource
	    virtual bool Load(LZHANDLE lzHdl);             // Load GIF from LZ File	


		enum tagErrors { errorOutOfMemory = -10, errorBadCode = -20, errorRead = -1,
			   errorWrite = -3,  errorOpen = -3, errorCreate = -4 };

		enum  tagSizes { codeSize = 4095, byteBufSize = 257, bufferSize = 1024 };
		enum  tagMasks { maskColorTable = 0x80 , maskInterlace = 0x40 }; 
		enum  tagTypes { typeUnknown = 0, typeGIF87a = 1, typeGIF89a = 2 }; 


	private :
		void 	InitDecoder();
		void 	TermDecoder();	   
		chuint8	GetByte();
		int		ReadNBytes( unsigned char* pBuff, int iSize );
		int		Read( unsigned char* pstrBuffer, int iBufSize );
		int		ReadHeader( );
		bool 	ReadImageDescriptor( int iType, pChImageInfo pImgInfo, pChImageFrameInfo pFrameInfo  );
		void 	SetDIBColors( pGIFColorTable pbClrTbl, int iEntries );
		bool 	ReadColorTable( int iNumEntries, bool bbGlobalTable );


		short init_exp( short size );		// This function initializes the decoder for reading a new image. 
		short get_next_code();				//  gets the next code from the GIF file. 
											// Returns the code, or
	 										//  else a negative number in case of file errors... 
		short decoder( short linewidth);

		int out_line( unsigned char* line, int iLen );


		int   bad_code_count;				//  This is incremented each time an out of range code is 
											// read by the decoder. When this value is non-zero 
											// after a decode, your GIF 
											// file is probably corrupt in some way... 
	 	short 				curr_size;                     /* The current code size */
	 	short 				clear;                         /* Value for a clear code */
	 	short 				ending;                        /* Value for a ending code */
		short 				newcodes;                      /* First available code */
		short 				top_slot;                      /* Highest code for current size */
		short 				slot;                          /* Last read code */

		short 				navail_bytes;              /* # bytes left in block */
		short 				nbits_left;                /* # bits left in current byte */
		unsigned char 		b1;                    /* Current byte */
		unsigned char 		*byte_buff;        	 /* Current block */
		unsigned char 		*pbytes;               /* Pointer to next byte in block */
		unsigned char 		*stack;    			 /* Stack for storing pixels */
		unsigned char 		*suffix;   			/* Suffix table */
		unsigned short		*prefix;  			/* Prefix linked list */

		std::fstream*			m_pFile;			// GIF file
		unsigned char*		m_pstrBuffer;		// internal buffer
		int					m_iCurrIndex;		// current read index;
		int					m_iBufCount;		// Total bytes in the buffer
		unsigned char  		*m_plocalClrTbl;		// local color table
		unsigned char  		*m_pgblClrTbl;		// global color table
		int					m_iCurrScanLine;	// Current scan line 
		int					m_iFrame;			// current frame being decoded
		int					m_iPass;			// current pass for interlaced GIF's
		GIFHeader			m_gifHeader;
		GIFImageDescriptor	m_gifDesc;

};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:38  uecasm
// Import of source tree as at version 2.53 release.
//

#endif // CHGIFDECODER_H_
