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

	This file contains the interface for the ChHtmlTag class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CHHTMLTAG_H )

#define   CHHTMLTAG_H

/*----------------------------------------------------------------------------
Class :
----------------------------------------------------------------------------*/

class 	ChHtmlParser;
class 	ChHtmlTag;
typedef ChHtmlTag *pChHtmlTag;



class  ChHtmlTag
{
	public :
		enum tagModifiers {  typeListNum = 0x01, typeListAlphaLower = 0x02, 
						  typeListAlphaUpper = 0x08, typeListRomanLower = 0x10,
						  typeListRomanUpper = 0x20, typeListCircleBullet = 0x40,
						  typeListSquareBullet = 0x80 };


		enum tagAttrs { attrSaveState = 0x01, attrHasFontName = 0x02, attrHasFontSize = 0x04,  
						attrCallStart = 0x08, 
						attrHasArguments = 0x10, attrStartLine = 0x20,
						attrTerminateLine = 0x40, attrTrimRight = 0x80, 
						attrAddLineAbove = 0x100, attrAddLineBelow = 0x200, 
						attrFontProportional = 0x400, attrFontFixed = 0x800, 
						attrParseWhiteSpace = 0x1000,	attrFontSizeAbsolute = 0x2000, 
						attrCallEnd = 0x4000, attrCallProcessTag = 0x8000, 
						attrCallStartAndEnd = 0x4008
							}; 

		enum tagFontAttrs { fontItalic = 0x1, fontBold = 0x2, fontUnderline = 0x4,
							fontStrikethrough = 0x10 };


		enum tagRestore { restoreLineFmt = 0x1, restoreFontAttrs = 0x2, restoreFontSize = 0x4,
						  restoreTextColor = 0x8, restoreBkColor = 0x10, restoreLineIndent = 0x20,
						  restoreFontName = 0x40, restoreUserData = 0x80, restoreFontSizeExtra = 0x100 };

		enum tagConstants { indentFactor = 3 };

		ChHtmlTag( ChHtmlParser *pParser );
		virtual ~ChHtmlTag();

		int GetHTMLId() 					{ return  m_iTokenID; }
		chuint32 GetAttributes()			{ return  m_luAttrs; }
		chuint32 GetFontAttrs()				{ return  m_luFontAttrs; }
		chuint32 GetRestoreFlags()			{ return  m_luStateToRestore; }

		// Overide these methods for specific tags
		virtual chint32	 GetPointSize();			

		virtual	void  	 ProcessArguments( 	pChArgList pList, int iArgCount ) 
											{
											}
		virtual void 	 StartTag()			{ return; }
		virtual void 	 EndTag()			{ return; }
		virtual	chint32	 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount ); 

	protected  :
		inline ChHtmlView* 		GetHtmlView()		{ return m_pParser->GetHtmlView(); }

  		int				m_iTokenID;					// the HTML_XXX id
		chuint32		m_luAttrs;					// style for this tag
		chuint32		m_luFontAttrs;				// font attrs to change and restore
		chuint32		m_luStateToRestore;			// Attributes to restore when the tag terminates
		ChHtmlParser*	m_pParser;

};

// $Log$

#endif // CHHTMLTAG_H
