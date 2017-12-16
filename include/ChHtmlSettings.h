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

	This file consists of the interface for the ChHtmlSettings.

----------------------------------------------------------------------------*/

#if !defined( _CHHTMLSETTINGS_H )
#define _CHHTMLSETTINGS_H

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


#if defined ( CH_WEBTRACKER )

	#define CH_PRODUCT_NAME  "WebTracker"

#elif  defined( CH_PUEBLO_PLUGIN )

	#define	CH_PRODUCT_NAME	"Pueblo Plug-In"

#else
	// Pueblo standalone
	#define CH_PRODUCT_NAME		"Pueblo"

#endif

class CH_EXPORT_CLASS ChHtmlSettings
{
	public :
		ChHtmlSettings( )	{}
		virtual  ~ChHtmlSettings()	{}
		
		void SetProductName( const ChString& strProduct );
		void ReadPreferences( );

	// Accessor methods
		inline int GetPixelHt()								{ return m_iPixelHeight; }
		inline int GetTopIndent()							{ return m_iTopIndent; }
		inline int GetLeftIndent()							{ return m_iLeftIndent; }
		inline int GetCharWidth()							{ return m_iCharWidth; }

		inline const ChString& 	GetFixedFontName()			{ return m_strFixedFontName; }
		inline const ChString& 	GetProportionalFontName()	{ return m_strProportionalFontName; }
		inline const ChString& 	GetSymbolFontName()			{ return m_strSymbolFontName; }
		inline chint32	   		GetFixedFontSize()			{ return m_lFixedPointSize; }
		inline chint32	   		GetProportionalFontSize()	{ return m_lProportionalPointSize; }


		inline chuint32	GetTextColor()			{ return m_luTextColor; }
		inline chuint32	GetLinkColor()			{ return m_luLinkColor; }
		inline chuint32	GetVistedLinkColor()	{ return m_luVLinkColor; }
		inline chuint32	GetActiveLinkColor()	{ return m_luALinkColor; }
		inline chuint32	GetPrefetchedLinkColor(){ return m_luPLinkColor; }
		inline chuint32	GetBackColor()			{ return m_luBackColor; }

	private :
		static 	int		m_iPixelHeight;
		static  int		m_iCharWidth;
		static  int		m_iCharHeight;
		int				m_iTopIndent;
		int				m_iLeftIndent;
		ChString  		m_strFixedFontName;
		ChString  		m_strProportionalFontName;
		ChString			m_strSymbolFontName;

		chint32			m_lFixedPointSize;
		chint32	 		m_lProportionalPointSize;
		// color for regular text, visited links, active links, prefetched links
		chuint32 		m_luBackColor;			// regular text color
		chuint32 		m_luTextColor;			// regular text color
		chuint32 		m_luLinkColor;			// link color
		chuint32 		m_luVLinkColor;			// visited link
		chuint32 		m_luALinkColor;			// active link
		chuint32 		m_luPLinkColor;			// prefetched link

		ChString			m_strProduct;
};

#endif // _CHHTMLSETTINGS_H
