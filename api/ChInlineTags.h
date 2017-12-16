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

#if !defined( CHINLINETAGS_H )

#define   CHINLINETAGS_H

#include "ChHtmlTag.h"

#include "ChBodyElement.h"


class  ChImgTag : public  ChHtmlTag
{
	public :
		ChImgTag( ChHtmlParser *pParser );
		virtual ~ChImgTag();
		virtual void ProcessArguments( pChArgList pList, int iArgCount );
		virtual chint32 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );	   
	private :
		char	*m_pstrArgBuffer;
		int		 m_iBufSize;

};

class  ChEmbedTag : public  ChHtmlTag
{
	public :
		ChEmbedTag( ChHtmlParser *pParser );
		virtual ~ChEmbedTag()
			{
			}
		virtual void ProcessArguments( pChArgList pList, int iArgCount );
		virtual chint32 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );

};

class  ChHRTag : public  ChHtmlTag
{
	public :
		ChHRTag( ChHtmlParser *pParser );
		virtual ~ChHRTag()
			{
			}
		virtual chint32 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );
		virtual void ProcessArguments( pChArgList pList, int iArgCount );


};

class  ChBRTag : public  ChHtmlTag
{
	public :
		ChBRTag( ChHtmlParser *pParser )	;
		virtual ~ChBRTag()
			{
			}
		virtual chint32 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );
		virtual void ProcessArguments( pChArgList pList, int iArgCount );


};

class  ChLineItemTag : public  ChBodyElements
{
	public :
		ChLineItemTag( ChHtmlParser *pParser );
		virtual ~ChLineItemTag()
			{
			}
		virtual chint32 ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount );  
		virtual void 	 StartTag()	{}
		virtual void 	 EndTag() {}


};

#endif // CHINLINETAGS_H
