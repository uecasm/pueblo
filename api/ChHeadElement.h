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

	This file contains the interface for HTML head tags.

----------------------------------------------------------------------------*/

// $Header$

#include "ChHtmlTag.h"


class  ChHtmlElement : public  ChHtmlTag
{
	public :
		ChHtmlElement( ChHtmlParser *pParser );
		virtual ~ChHtmlElement()
			{
			}
		virtual	chint32  ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );
		virtual void 	 StartTag();
};



class  ChHeadTag : public  ChHtmlTag
{
	public :
		ChHeadTag( ChHtmlParser *pParser );
		virtual ~ChHeadTag()
			{
			}
		virtual	chint32	 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount  );
		virtual void 	 StartTag();
};


class  ChBaseTag : public  ChHtmlTag
{
	public :
		ChBaseTag( ChHtmlParser *pParser );
		virtual ~ChBaseTag()
			{
			}
		virtual	void ProcessArguments( 	pChArgList pList, int iArgCount ); 
};

class  ChTitleTag : public  ChHtmlTag
{
	public :
		ChTitleTag( ChHtmlParser *pParser );
		virtual ~ChTitleTag()
			{
			}
		virtual	chint32  ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount  );
		virtual void 	 StartTag();
};

class  ChCommentTag : public  ChHtmlTag
{
	public :
		ChCommentTag( ChHtmlParser *pParser );
		virtual ~ChCommentTag()
			{
			}
		virtual	chint32  ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount  );
};


class  ChPrefetchTag : public  ChHtmlTag
{
	public :
		ChPrefetchTag( ChHtmlParser *pParser );
		virtual ~ChPrefetchTag()
			{
			}
		virtual	void ProcessArguments( 	pChArgList pList, int iArgCount ); 
};

class  ChPageTag : public  ChHtmlTag
{
	public :
		ChPageTag( ChHtmlParser *pParser );
		virtual ~ChPageTag()
			{
			}
		virtual	void ProcessArguments( 	pChArgList pList, int iArgCount ); 
};

// $Log$
