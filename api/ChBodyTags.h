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

	This file contains the interface for HTML body tags.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CHBODYTAGS_H )
#define  CHBODYTAGS_H

#include "ChBodyElement.h"


class  ChHeadingTag : public  ChBodyElements
{
	public :
		ChHeadingTag( int iToken, ChHtmlParser *pParser );
		virtual ~ChHeadingTag()
			{
			}
		virtual chint32	 GetPointSize();			
		virtual	void  ProcessArguments(	pChArgList pList, int iArgCount );

	private :
		int m_iPointSizeFactor;
		int m_iPixelHt;
};


class  ChPragraphTag : public  ChBodyElements
{
	public :
		ChPragraphTag( ChHtmlParser *pParser );
		virtual ~ChPragraphTag()
			{
			}
		virtual	void  ProcessArguments( 	pChArgList pList, int iArgCount );
};

class  ChPreFormatTag : public  ChBodyElements
{
	public :
		ChPreFormatTag( ChHtmlParser *pParser );
		virtual ~ChPreFormatTag()
			{
			}
		virtual void 	 StartTag();
		virtual void 	 EndTag();
		virtual	void  ProcessArguments( 	pChArgList pList, int iArgCount );
};


class  ChQuoteTag : public  ChBodyElements
{
	public :
		ChQuoteTag( ChHtmlParser *pParser );
		virtual ~ChQuoteTag()
			{
			}
};

class  ChBlockQuoteTag : public  ChBodyElements
{
	public :
		ChBlockQuoteTag( ChHtmlParser *pParser );
		virtual ~ChBlockQuoteTag()
			{
			}
		virtual	void  ProcessArguments( 	pChArgList pList, int iArgCount );
};


class  ChAddressTag : public  ChBodyElements
{
	public :
		ChAddressTag( ChHtmlParser *pParser );
		virtual ~ChAddressTag()
			{
			}
};

class  ChListTags : public  ChBodyElements
{
	public :
		ChListTags( int iToken, ChHtmlParser *pParser );
		virtual ~ChListTags()
			{
			}
		virtual	void  ProcessArguments( pChArgList pList, int iArgCount );
};


class  ChAnchorTag : public  ChBodyElements
{
	public :
		ChAnchorTag(  ChHtmlParser *pParser );
		virtual ~ChAnchorTag()
			{
			}
		virtual	void  ProcessArguments( pChArgList pList, int iArgCount );
};

// samp, code, tt
class  ChSampleTag : public  ChBodyElements
{
	public :
		ChSampleTag( int iToken, ChHtmlParser *pParser );
		virtual ~ChSampleTag()
			{
			}
		virtual void 	 StartTag();
		virtual void 	 EndTag();
};

class  ChCenterTag : public  ChBodyElements
{
	public :
		ChCenterTag( ChHtmlParser *pParser );
		virtual ~ChCenterTag()
			{
			}
		virtual void 	 StartTag();
		virtual void 	 EndTag();
};


class  ChNOBRTag : public  ChBodyElements
{
	public :
		ChNOBRTag( ChHtmlParser *pParser )	;
		virtual ~ChNOBRTag()
			{
			}
		virtual void 	 StartTag();
		virtual void 	 EndTag();

};

// $Log$

#endif //CHBODYTAGS_H
