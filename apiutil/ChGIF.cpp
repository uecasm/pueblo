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

	Chaco GIF decoder

----------------------------------------------------------------------------*/
//
// $Header$

#include "headers.h"

#ifdef CH_UNIX
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define _STREAM_COMPAT 1
#endif

#include <string.h>
#include <iostream>
#include <fstream>

#include <ChTypes.h>

#ifdef CH_UNIX
#include <ChDC.h>
#endif
#include <ChGifDecoder.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif			




#if defined( CH_MSW )

	#define IOS_BINARY		std::ios::binary

#elif defined( CH_UNIX )

	#define IOS_BINARY		std::ios::bin

#endif	// defined( CH_UNIX )
				    

  
/////////////////////////////////////////////////////////////////////////////////

ChGifDecoder::ChGifDecoder( ChImageConsumer* pConsumer ) : ChImageDecoder( pConsumer )
{
 	curr_size 	= 0;                     /* The current code size */
 	clear 		= 0;                         /* Value for a clear code */
 	ending 		= 0;                        /* Value for a ending code */
	newcodes 	= 0;                      /* First available code */
	top_slot 	= 0;                      /* Highest code for current size */
	slot 		= 0;                          /* Last read code */

	navail_bytes = 0;              /* # bytes left in block */
	nbits_left = 0;                /* # bits left in current byte */
	m_iCurrIndex = 0;				// current read index;
	m_iBufCount = 0;				// Total bytes in the buffer
	m_iFrame = -1;
}

ChGifDecoder::~ChGifDecoder() 
{
	
}
bool ChGifDecoder::Load(WORD wResid, HINSTANCE hInst)
{
	ASSERT( false );
	return false;
}
bool ChGifDecoder::Load(LZHANDLE lzHdl)
{
	ASSERT( false );
	return false;
}

bool ChGifDecoder::Load( const char * pszFileName, chuint32 flOptions /* = ChDIB::loadAuto */ )
{

	m_flOptions	= flOptions;

	bad_code_count  = 0;
	m_pFile = ::new std::fstream( pszFileName, std::ios::in | IOS_BINARY );

	ASSERT( m_pFile );

	if ( !m_pFile->is_open() )
	{
		::delete m_pFile;
		return false;
	}



	// InitDecoder
	InitDecoder();

	// scan the header
	int	  iType;

	if ( (iType = ReadHeader( )) )
	{

		ChImageInfo		 imageInfo;
		ChMemClearStruct( &imageInfo );
		imageInfo.iWidth = m_gifHeader.wScreenWidth; 
		imageInfo.iHeight = m_gifHeader.wScreenHeight;
		 
		// read the color table if present
		if ( m_gifHeader.bPacked & maskColorTable )
		{
			UINT uColors = (m_gifHeader.bPacked & 0x07 );
			ReadColorTable( (int)( 1L << ( uColors + 1 ) ), true );
		}


		// read the image descriptor
		bool boolContinue;
		do 
		{
			ChImageFrameInfo frameInfo;
			ChMemClearStruct( &frameInfo );

		
			if ( (boolContinue = ReadImageDescriptor( iType, &imageInfo, &frameInfo )) )
			{
				if ( m_gifDesc.bPacked & maskColorTable )
				{
					UINT uColors = (m_gifDesc.bPacked & 0x07 );
					ReadColorTable( (int)( 1L << ( uColors + 1 ) ), false );
				}

				// ** BEGIN UE HACK **
				if ( m_iFrame == -1 ) {
					// The incoming GIF wasn't in an expected format; what seems to cause this
					// is a GIF89a that doesn't contain a graphics control block.
					// No idea what output this will result in (since the GCB defines all sorts
					// of image properties), but keeping an image index of -1 is totally
					// unacceptable (and will cause truly crazy behaviour).
					m_iFrame = 0;
					TRACE("WARNING: image didn't decode properly!");
				}
				// ** END UE HACK **

				if ( m_iFrame == 0 )
				{ 
					GetConsumer()->NewImage( &imageInfo );
				}

				frameInfo.iFrame = m_iFrame;
				frameInfo.iLeft = m_gifDesc.wLeft;
				frameInfo.iTop = m_gifDesc.wTop;
				frameInfo.iWidth = m_gifDesc.wWidth;
				frameInfo.iHeight = m_gifDesc.wHeight;

				// Create empty image
				if ( (boolContinue = GetConsumer()->Create( &frameInfo ) ))
				{  // decode and fill the dib 
					pGIFColorTable 	pgifClrTbl;
					int				iEntries;
					// use local table if present
					if ( m_plocalClrTbl )
					{
						UINT uColors = (m_gifDesc.bPacked & 0x07 );
						iEntries   = (int)( 1L << ( uColors + 1 ) );
						pgifClrTbl = (pGIFColorTable)m_plocalClrTbl;
					}
					else
					{
						UINT uColors = (m_gifHeader.bPacked & 0x07 );
						iEntries   = (int)( 1L << ( uColors + 1 ) );
						pgifClrTbl = (pGIFColorTable)m_pgblClrTbl;
					}

					SetDIBColors( pgifClrTbl, iEntries );
					m_iCurrScanLine = 0;		// current scan line
					m_iPass = 1;

					// decode the GIF data and set scan lines
					boolContinue = (0 == decoder( m_gifDesc.wWidth ));

					if ( boolContinue )
					{	// remove the terminator
						boolContinue = chuint8(errorRead) != GetByte();
					}
				}
			}
		} 
		while( iType == typeGIF89a && boolContinue );

	}

	// do cleanup 
	TermDecoder();
	// close file
	::delete m_pFile;

	return true;
}

int ChGifDecoder::ReadHeader( )
{
	// read the GIF header
	if ( errorRead != ReadNBytes( m_gifHeader.strSignature, 
					sizeof(m_gifHeader )) )
	{
		char strSignature[7];
	
		strSignature[6] = 0; 
	
		ChMemCopy( strSignature, m_gifHeader.strSignature, 6 );
		// check if we have a valid GIF file
		if ( lstrcmpi( strSignature, "GIF87a" ) == 0  )
		{
			return typeGIF87a;
		}
		else if ( lstrcmpi( strSignature, "GIF89a" ) == 0  )
		{
			return typeGIF89a;
		}
	}	 
	return typeUnknown;
}

bool ChGifDecoder::ReadImageDescriptor( int iType, 
							pChImageInfo pImgInfo, pChImageFrameInfo pFrameInfo )
{
	 
	if ( iType == typeGIF89a )
	{ // read all the extension blocks
		unsigned char bExtn[3];
		// read all the extension blocks
		while ( true )
		{
			if ( errorRead != ReadNBytes( bExtn, sizeof(bExtn)) )
			{
			 	if ( bExtn[0] == 0x21 )
				{ 	// if we have graphics control block
					if ( bExtn[1] == 0xF9 )
					{
						GIFGraphicControlBlock grBlk;
						ReadNBytes( &grBlk.bPacked, bExtn[2] + 1 );

						m_iFrame++;

						if ( grBlk.bPacked & 0x01 )
						{
							if ( m_pgblClrTbl )
							{
								pGIFColorTable pClrTbl =(pGIFColorTable)m_pgblClrTbl; 
								#ifdef CH_MSW

								pFrameInfo->luAttrs |=  ChImageConsumer::imgTransparent;
								pFrameInfo->colorTransparent = RGB( 
														pClrTbl[grBlk.bColorIndex].bRed, 
														pClrTbl[grBlk.bColorIndex].bGreen,
														pClrTbl[grBlk.bColorIndex].bBlue);
								pFrameInfo->iTransparentIndex = grBlk.bColorIndex;

														 
								#else
								cerr << "No implemented: " << __FILE__ << ":" << __LINE__ << endl;
								#endif // CH_MSW
							}
						}
						// Delay time between frames
						pFrameInfo->iExposture = grBlk.wDelayTime * 10;
					}
					else if ( bExtn[1] == 0xFF )
					{  // Netscape looping extension
						unsigned char* pBuffer = new unsigned char[ bExtn[2] + 1];
						ASSERT( pBuffer );
						pBuffer[ bExtn[2] ] = 0;
						ReadNBytes( pBuffer, bExtn[2] );
						if ( strstr( (const char*)pBuffer, "NETSCAPE" ) )
						{
							chuint16 uLoop;
							ReadNBytes( 0, 2 );
							ReadNBytes( (unsigned char*)&uLoop, 2 );

							pImgInfo->iLoopCount = uLoop;
							pImgInfo->boolMultiframe = true;
						}
						delete []pBuffer;

						while( errorRead != ReadNBytes( bExtn, 1 ) && bExtn[0] )
						{
						}
					}
					else
					{
						//skip nExtn[2] bytes
	 					ReadNBytes( 0, bExtn[2] );
						// skip till uo find a terminator
						while( errorRead != ReadNBytes( bExtn, 1 ) && bExtn[0] )
						{
						}
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				return false;
			}
		}
		// already read 3 bytes from the header 
		ChMemCopy( &m_gifDesc.bSeparator, bExtn, sizeof(bExtn) );
		ReadNBytes( (&m_gifDesc.bSeparator) + sizeof( bExtn ), sizeof(m_gifDesc) - sizeof(bExtn));
	}
	else
	{  // 87a
		// read the GIF description
		ReadNBytes( &m_gifDesc.bSeparator, sizeof(m_gifDesc));
		m_iFrame++;
	}	 
	
	if ( 0x2C == m_gifDesc.bSeparator  )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ChGifDecoder::ReadColorTable( int iNumEntries, bool bbGlobalTable )
{
	ASSERT( iNumEntries );
	int iTableSize = sizeof(GIFColorTable) * iNumEntries;
	
	unsigned char *pgifClrTbl = new unsigned char[ iTableSize ];
	ASSERT( pgifClrTbl );

	if ( errorRead != ReadNBytes( (unsigned char*)pgifClrTbl, iTableSize ) )
	{
		if ( bbGlobalTable )
		{
			m_pgblClrTbl = (unsigned char*)pgifClrTbl;
		}
		else
		{
			if ( m_plocalClrTbl )
			{  // This GIF has multiple image descriptors
				delete []m_plocalClrTbl;
			}
			m_plocalClrTbl = (unsigned char*)pgifClrTbl;
		}
		return true;
	}
	return false;
}

#include "ChDibImage.h"

void ChGifDecoder::SetDIBColors( pGIFColorTable pbClrTbl, int iEntries )
{
    // GetDIB color table.
#ifdef CH_MSW
	if ( iEntries > 0 )
	{
	    RGBQUAD* prgb = new RGBQUAD[iEntries];

		ASSERT( prgb );
		ASSERT( pbClrTbl );

		RGBQUAD* prgbTemp = prgb;
 
	    for (int i = 0; i < iEntries; i++) 
	    {
	        prgbTemp->rgbBlue 	= pbClrTbl->bBlue;
	        prgbTemp->rgbGreen 	= pbClrTbl->bGreen;
	        prgbTemp->rgbRed 	= pbClrTbl->bRed;
			prgbTemp->rgbReserved = 0;

			pbClrTbl++;
	        prgbTemp++;
	    }

		ChImageConsumer *consumer = GetConsumer();
		/*
		// UE debugging
		{
			char buffer[1024];
			wsprintf(buffer, "*** Got GIF image consumer = %08Xh.\r\n", consumer);
			OutputDebugString(buffer);
			
			wsprintf(buffer, "*** Current frame = %d\r\n", ((ChDib *)consumer)->GetCurrentFrame());
			OutputDebugString(buffer);
			
			wsprintf(buffer, "*** SetColorTable address = %08Xh\r\n", &consumer->SetColorTable);
			OutputDebugString(buffer);
		}
		// UE debugging end
		*/
		consumer->SetColorTable( m_iFrame, prgb, iEntries );

		delete [] prgb;
	}
#else
    cerr << "ChGifDecoder::SetDIBColors()" << endl;
#endif

}
 
chuint8	ChGifDecoder::GetByte()
{
	if ( m_iCurrIndex >= m_iBufCount )
	{
		m_iCurrIndex = 0;
		//m_BufCount = m_pFile->read( pstrBuffer, ChGifDecoder::bufferSize ).gcount();
		m_iBufCount = Read( m_pstrBuffer, ChGifDecoder::bufferSize );
		if ( 0 == m_iBufCount )
		{
			return errorRead;	
		}
	}
	return ( m_pstrBuffer[m_iCurrIndex++]);
}

int	ChGifDecoder::ReadNBytes( unsigned char* pBuff, int iSize )
{	  
	int iRet = 0;
	while ( iRet < iSize )
	{
		if ( m_iCurrIndex >= m_iBufCount )
		{
			m_iCurrIndex = 0;
			m_iBufCount = Read( m_pstrBuffer, ChGifDecoder::bufferSize );
			if ( 0 == m_iBufCount )
			{
				return errorRead;	
			}
		}
		int iWrite =   ( m_iBufCount - m_iCurrIndex ) < (iSize - iRet) 
						? ( m_iBufCount - m_iCurrIndex ) : (iSize - iRet);
		if ( pBuff && iWrite )
		{
			ChMemCopy( &pBuff[iRet], &m_pstrBuffer[m_iCurrIndex], iWrite );
		}
	
		iRet += iWrite;
		m_iCurrIndex += iWrite;
	}
	return iRet;
}

int	ChGifDecoder::Read( unsigned char* pstrBuffer, int iBufSize )
{
	if ( m_pFile )
	{  // data from file
		return  m_pFile->read( (char *)pstrBuffer, ChGifDecoder::bufferSize ).gcount();
	}
	else
	{  // stream data from socket
		return 0;
	}
}

void ChGifDecoder::InitDecoder()
{
	m_iCurrScanLine = 0;		// current scan line
	m_iFrame  = -1;
	m_iPass = 1;

	m_pstrBuffer = new unsigned char[ChGifDecoder::bufferSize];
	ASSERT( m_pstrBuffer );	  
	m_iCurrIndex = m_iBufCount = 0;

	byte_buff  = new unsigned char[byteBufSize];  // data buffer
	ASSERT(byte_buff );

	stack = new unsigned char[codeSize + 1];    /* Stack for storing pixels */
	ASSERT( stack );

	suffix = new unsigned char[codeSize + 1];    /* Suffix table */
	ASSERT( suffix );

	prefix = new unsigned short[codeSize + 1];    /* Prefix linked list */
	ASSERT(prefix );

	m_pgblClrTbl = m_plocalClrTbl = 0;
}

void ChGifDecoder::TermDecoder()
{
	delete [] byte_buff;
	delete [] stack;
	delete [] suffix;
	delete [] prefix;

	delete []m_pstrBuffer;

	if ( m_pgblClrTbl )
	{
		delete [] m_pgblClrTbl;
	}

	if ( m_plocalClrTbl )
	{
		delete [] m_plocalClrTbl;
	}
}


int ChGifDecoder::out_line( unsigned char* pbPixels, int iLen )
{
	
	if ( m_iCurrScanLine < m_gifDesc.wHeight )
	{
		// point it to the current scan line 
		GetConsumer()->SetScanLine( m_iFrame, m_iCurrScanLine,
									pbPixels, iLen, ChImageConsumer::format8Bit );
										
		// next scan line
		if ( m_gifDesc.bPacked & maskInterlace )
		{
			switch ( m_iPass )
			{
				case 1 :
				{
					m_iCurrScanLine += 8;
					if ( m_iCurrScanLine >= m_gifDesc.wHeight  )
					{
						m_iPass++;
						m_iCurrScanLine = 4;	
					}
					break;
				}
				case 2 :
				{
					m_iCurrScanLine += 8;
					if ( m_iCurrScanLine >= m_gifDesc.wHeight  )
					{
						m_iPass++;
						m_iCurrScanLine = 2;	
					}
					break;
				}
				case 3 :
				{
					m_iCurrScanLine += 4;
					if ( m_iCurrScanLine >= m_gifDesc.wHeight  )
					{
						m_iPass++;
						m_iCurrScanLine = 1;	
					}
					break;
				}
				case 4 :
				{
					m_iCurrScanLine += 2;
					break;
				}
			}
		}
		else
		{
			m_iCurrScanLine++;
		}
		return 0;
	}
	return errorWrite;
}

/* decode.c is Steven Bennett's code with some minor changes */
/* the original copyright from him follows */
/* Wilson MacGyver Liaw */

/* DECODE.C - An LZW decoder for GIF
 * Copyright (C) 1987, by Steven A. Bennett
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 * In accordance with the above, I want to credit Steve Wilhite who wrote
 * the code which this is heavily inspired by...
 * GIF and 'Graphics Interchange Format' are trademarks (tm) of
 * Compuserve, Incorporated, an H&R Block Company.
 * Release Notes: This file contains a decoder routine for GIF images
 * which is similar, structurally, to the original routine by Steve Wilhite.
 * It is, however, somewhat noticably faster in most cases.
 */


// Win32s works better if you use GlobalAlloc for large memory
// blocks so the CWave and ChGIF classes use the ALLOC and FREE
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

    #define ALLOC(s) malloc(s)
    #define FREE(p) free(p)

#endif



/* extern int out_line(pixels, linelen)
 *     unsigned char pixels[];
 *     int linelen;
 * This function takes a full line of pixels (one byte per pixel) and
 * displays them (or does whatever your program wants with them...).  It
 * should return zero, or negative if an error or some other event occurs
 * which would require aborting the decode process...  Note that the length
 * passed will almost always be equal to the line length passed to the
 * decoder function, with the sole exception occurring when an ending code
 * occurs in an odd place in the GIF file...  In any case, linelen will be
 * equal to the number of pixels passed... */


static long code_mask[13] = {
     0,
     0x0001, 0x0003,
     0x0007, 0x000F,
     0x001F, 0x003F,
     0x007F, 0x00FF,
     0x01FF, 0x03FF,
     0x07FF, 0x0FFF
     };
/* This function initializes the decoder for reading a new image. */
short ChGifDecoder::init_exp( short size)
{
   curr_size = size + 1;
   top_slot = 1 << curr_size;
   clear = 1 << size;
   ending = clear + 1;
   slot = newcodes = ending + 1;
   navail_bytes = nbits_left = 0;
   return(0);
}
/* get_next_code() - gets the next code from the GIF file. Returns the code, or
 * else a negative number in case of file errors... */
short ChGifDecoder::get_next_code()
{
   short i, x;
   unsigned long ret;
   if (nbits_left == 0)
   {
      if (navail_bytes <= 0)
      {
         /* Out of bytes in current block, so read next block */
         pbytes = byte_buff;
         if ((navail_bytes = GetByte()) < 0)
            return(navail_bytes);
         else if (navail_bytes)
         {
            for (i = 0; i < navail_bytes; ++i)
            {
               if ((x = GetByte()) < 0)
                  return(x);
               byte_buff[i] = (unsigned char)x;
            }
         }
      }
      b1 = *pbytes++;
      nbits_left = 8;
      --navail_bytes;
   }
   ret = b1 >> (8 - nbits_left);
   while (curr_size > nbits_left)
   {
      if (navail_bytes <= 0)
      {
         /* Out of bytes in current block, so read next block */
         pbytes = byte_buff;
         if ((navail_bytes = GetByte()) < 0)
            return(navail_bytes);
         else if (navail_bytes)
         {
            for (i = 0; i < navail_bytes; ++i)
            {
               if ((x = GetByte()) < 0)
                  return(x);
               byte_buff[i] =(unsigned char) x;
            }
         }
      }
      b1 = *pbytes++;
      ret |= b1 << nbits_left;
      nbits_left += 8;
      --navail_bytes;
   }
   nbits_left -= curr_size;
   ret &= code_mask[curr_size];
   return((short)(ret));
}
/* The reason we have these seperated like this instead of using a structure
 * like the original Wilhite code did, is because this stuff generally produces
 * significantly faster code when compiled. This code is full of similar
 * speedups. (For a good book on writing C for speed or for space optimization,
 * see Efficient C by Tom Plum, published by Plum-Hall Associates.) */
//static unsigned char stack[MAX_CODES + 1];    /* Stack for storing pixels */
//static unsigned char suffix[MAX_CODES + 1];   /* Suffix table */
//static unsigned short prefix[MAX_CODES + 1];  /* Prefix linked list */

/* short decoder(linewidth)
 *    short linewidth;               * Pixels per line of image *
 * - This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to out_line(), which is a user specific function
 * to display a line of pixels.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * seperating them into the proper size codes.  Finally, GetByte() is
 * the global routine to read the next byte from the GIF file.
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 * Returns: 0 if successful, else negative.  (See ERRS defined) */
short ChGifDecoder::decoder( short linewidth)
{
   register unsigned char *sp, *bufptr;
   unsigned char *buf;
   register short code, fc, oc, bufcnt;
   short c, size, ret;
   /* Initialize for decoding a new image... */
   if ((size = GetByte()) < 0)
      return(size);
   if (size < 2 || 9 < size)
      return(errorBadCode);
   init_exp(size);
   /* Initialize in case they forgot to put in a clear code.
    * (This shouldn't happen, but we'll try and decode it anyway...) */
   oc = fc = 0;
   /* Allocate space for the decode buffer */
   if ((buf = (unsigned char *)ALLOC(linewidth + 1)) == NULL)
      return(errorOutOfMemory);
   /* Set up the stack pointer and decode buffer pointer */
   sp = stack;
   bufptr = buf;
   bufcnt = linewidth;
   /* This is the main loop.  For each code we get we pass through the
    * linked list of prefix codes, pushing the corresponding "character" for
    * each code onto the stack.  When the list reaches a single "character"
    * we push that on the stack too, and then start unstacking each character
    * for output in the correct order.  Special handling is included for the
    * clear code, and the whole thing ends when we get an ending code. */
   while ((c = get_next_code()) != ending)
   {
      /* If we had a file error, return without completing the decode */
      if (c < 0)
      {
         FREE(buf);
         return(0);
      }
      /* If the code is a clear code, reinitialize all necessary items. */
      if (c == clear)
      {
         curr_size = size + 1;
         slot = newcodes;
         top_slot = 1 << curr_size;
         /* Continue reading codes until we get a non-clear code
          * (Another unlikely, but possible case...) */
         while ((c = get_next_code()) == clear)
            ;
         /* If we get an ending code immediately after a clear code
          * (Yet another unlikely case), then break out of the loop. */
         if (c == ending)
            break;
         /* Finally, if the code is beyond the range of already set codes, 
          * (this had better NOT happen. I have no idea what will result from
          * this, but I doubt it will look good) then set it to color zero. */
         if (c >= slot)
            c = 0;
         oc = fc = c;
         /* And let us not forget to put the char into the buffer. And if, on
          * the off chance, we were exactly one pixel from the end of the line,
          * we have to send the buffer to the out_line() routine... */
         *bufptr++ = (unsigned char)c;
         if (--bufcnt == 0)
         {
            if ((ret = out_line(buf, linewidth)) < 0)
            {
               FREE(buf);
               return(ret);
            }
            bufptr = buf;
            bufcnt = linewidth;
         }
      }
      else
      {
         /* In this case, it's not a clear code or an ending code, so
          * it must be a code code...  So we can now decode the code into
          * a stack of character codes. (Clear as mud, right?) */
         code = c;
         /* Here we go again with one of those off chances...  If, on the
          * off chance, the code we got is beyond the range of those already
          * set up (Another thing which had better NOT happen...) we trick
          * the decoder into thinking it actually got the last code read.
          * (Hmmn... I'm not sure why this works...  But it does...) */
         if (code >= slot)
         {
            if (code > slot)
               ++bad_code_count;
            code = oc;
            *sp++ = (unsigned char)fc;
         }
         /* Here we scan back along the linked list of prefixes, pushing
          * helpless characters (ie. suffixes) onto the stack as we do so. */
         while ( (code >= newcodes )&& ( sp + 1 < (stack + codeSize) ))
         {
            *sp++ = suffix[code];
            code = prefix[code];
         }
         /* Push the last character on the stack, and set up the new
          * prefix and suffix, and if the required slot number is greater
          * than that allowed by the current bit size, increase the bit
          * size.  (NOTE - If we are all full, we *don't* save the new
          * suffix and prefix...  I'm not certain if this is correct...
          * it might be more proper to overwrite the last code... */
         *sp++ = (unsigned char)code;
         if (slot < top_slot)
         {
		 	fc = code;
            suffix[slot] = (unsigned char)fc;
            prefix[slot++] = oc;
            oc = c;
         }
         if (slot >= top_slot)
            if (curr_size < 12)
            {
               top_slot <<= 1;
               ++curr_size;
            } 
         /* Now that we've pushed the decoded string (in reverse order) onto
          * the stack, lets pop it off and put it into our decode buffer.
          * And when the decode buffer is full, write another line... */
         while (sp > stack)
         {
            *bufptr++ = *(--sp);
            if (--bufcnt == 0)
            {
               if ((ret = out_line(buf, linewidth)) < 0)
               {
                  FREE(buf);
                  return(ret);
               }
               bufptr = buf;
               bufcnt = linewidth;
            }
         }
      }
   }
   ret = 0;
   if (bufcnt != linewidth)
      ret = out_line(buf, (linewidth - bufcnt));
   FREE(buf);
   return(ret);
}

// $Log$
