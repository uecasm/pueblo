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

	This file contains the implementation of ChHtmlSettings.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChConst.h>
#include <ChReg.h>
#include <ChHtmlSettings.h>

#include <MemDebug.h>

//#define CH_COMPANY_NAME		"Andromedia Incorporated"


int ChHtmlSettings::m_iPixelHeight  = 0;
int ChHtmlSettings::m_iCharWidth 	= 0;
int ChHtmlSettings::m_iCharHeight   = 0;

void ChHtmlSettings::SetProductName( const ChString& strProduct )
{
	if ( m_strProduct != strProduct )
	{
		m_strProduct = strProduct;
		ReadPreferences();
	}
	
}


void ChHtmlSettings::ReadPreferences(  )
{
	// get the logical pixel width;
	if ( m_iPixelHeight == 0 )
	{
		#ifdef CH_MSW

		HDC			hDC = ::GetDC( ::GetDesktopWindow() );
		m_iPixelHeight = -::GetDeviceCaps(hDC, LOGPIXELSY);

		SIZE size;
		::GetTextExtentPoint( hDC, TEXT( "M" ), 1, &size );

		m_iCharWidth = size.cx;
		m_iCharHeight = size.cy;
		::ReleaseDC(::GetDesktopWindow(), hDC );

		#else

		m_iPixelY = 1;
		m_iCharWidth = 10;
		m_iCharHt = 10;

				cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
		#endif
	}

	m_iLeftIndent = m_iCharWidth;
	m_iTopIndent  = 0;

	#ifdef CH_MSW
		ChRegistry reg = ChRegistry( CH_COMPANY_NAME, m_strProduct, CH_FONT_GROUP );

	   	// set up proportional font 				    
		reg.Read( CH_FONT_PROPORTIONAL, m_strProportionalFontName, 
							CH_FONT_PROPORTIONAL_DEF );
		reg.Read( CH_FONT_PROPORTIONAL_SIZE, m_lProportionalPointSize, 
						CH_FONT_PROPORTIONAL_SIZE_DEF );

		if (m_lProportionalPointSize <= 0 || m_lProportionalPointSize > 18)
		{
			m_lProportionalPointSize = CH_FONT_PROPORTIONAL_SIZE_DEF;
		}

	   	// set up fixed font 

		reg.Read( CH_FONT_FIXED, m_strFixedFontName, CH_FONT_FIXED_DEF );
		reg.Read( CH_FONT_FIXED_SIZE, m_lFixedPointSize,
					CH_FONT_FIXED_SIZE_DEF );

		if (m_lFixedPointSize <= 0 || m_lFixedPointSize > 18)
		{
			m_lFixedPointSize = CH_FONT_FIXED_SIZE_DEF;
		}

	#else
		m_strProportionalFontName = "times";
		m_lProportionalPointSize = CH_FONT_PROPORTIONAL_SIZE_DEF;
		m_strFixedFontName = "fixed";
		m_lFixedPointSize = CH_FONT_FIXED_SIZE_DEF;
	#endif

	// Symbol font name
	m_strSymbolFontName = TEXT( "Symbol" );

	ChRegistry htmlColor( CH_COMPANY_NAME, m_strProduct, CH_COLOR_GROUP );

	// load all the colors

	htmlColor.Read( CH_COLOR_TEXT, m_luTextColor, CH_COLOR_DEFAULT );
	
	if ( CH_COLOR_DEFAULT & m_luTextColor )
	{
		m_luTextColor = COLOR_DEF_TEXT;	//::GetSysColor( COLOR_BTNTEXT );
	}

	htmlColor.Read( CH_COLOR_LINK, m_luLinkColor, CH_COLOR_DEFAULT );
	if ( CH_COLOR_DEFAULT & m_luLinkColor )
	{
		m_luLinkColor = COLOR_DEF_LINK;
	}

	htmlColor.Read( CH_COLOR_FLINK, m_luVLinkColor, CH_COLOR_DEFAULT );
	if ( CH_COLOR_DEFAULT & m_luVLinkColor )
	{
		m_luVLinkColor = COLOR_DEF_VIST_LINK;
	}

	htmlColor.Read( CH_COLOR_PLINK, m_luPLinkColor, CH_COLOR_DEFAULT );
	if ( CH_COLOR_DEFAULT & m_luPLinkColor )
	{
		m_luPLinkColor = COLOR_DEF_PREFETCH_LINK;
	}

	m_luALinkColor	= COLOR_DEF_ACTIVE_LINK;

	htmlColor.Read( CH_COLOR_BACK, m_luBackColor, CH_COLOR_DEFAULT );

	if ( CH_COLOR_DEFAULT & m_luBackColor )
	{
		m_luBackColor = COLOR_DEF_BACK;	//::GetSysColor( COLOR_WINDOW );
	}
}

// $Log$
