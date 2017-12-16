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

	This file contains the definition of the ChHTTPInfo class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

#if (!defined( CHTTPSTREAM_H ))
#define CHTTPSTREAM_H

class  ChHTTPStreamManager;
class  ChHTTPStream ;

typedef ChHTTPStreamManager *pChHTTPStreamManager;
typedef ChHTTPStream 		*pChHTTPStream;



class CH_EXPORT_CLASS ChHTTPStream 
{
	public :

		ChHTTPStream()
					{
					}
		virtual ~ChHTTPStream()
					{
					}

	    virtual const ChString& GetURL() = 0;
	    virtual const ChString& GetMimeType() = 0;
	    virtual const ChString& GetCacheFilename() = 0;
		virtual const char* GetErrorMsg() = 0;
	    virtual const ChString& GetContentEncoding() = 0;
	    virtual const ChString& GetLastModified() = 0;
	    virtual const ChString& GetTargetWindowName() = 0;
	    virtual long  GetContentLength() = 0;
	    virtual void* GetStreamPrivateData() = 0;
	 	virtual void SetStreamPrivateData( void *pData ) = 0;
};



class CH_EXPORT_CLASS ChHTTPStreamManager
{
	public :
	   	enum StreamType { streamNormal = 0x1, streamAsFile = 0x02 } ;

		ChHTTPStreamManager()				{}
		virtual ~ChHTTPStreamManager() 		{}

	    virtual int NewStream( chparam requestData, pChHTTPStream pStream, bool boolSeekable ) = 0;
	    virtual void DestroyStream( chparam requestData, pChHTTPStream pStream, int iReason ) = 0;
	    
	    virtual chint32 WriteReady( chparam requestData, pChHTTPStream pStream, chint32 iBytes );
	    virtual chint32 Write( chparam requestData, pChHTTPStream pStream, 
	    								chint32 lOffset, chint32 lLen, const char* pBuffer );
	 	virtual void StreamAsFile(chparam requestData, pChHTTPStream pStream, const char* fname);

	 	virtual void OnUpdateProgress( const char* pstrMsg, int iPercentComplete );
};

#endif // (!defined( CHTTPSTREAM_H ))
