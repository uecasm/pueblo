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

	This file contains the implementaion of plugin entry points.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChHtmWnd.h>
#include "ChHtmlView.h"

#include "ChPlgInMgr.h"
#include "ChPlgInAPI.h"

#include <MemDebug.h>


 NPError CHPN_GetURL(NPP instance, const char* url, const char* window)
{
	NPError iError =  NPERR_NO_ERROR;

	try
	{
		ChPlugInInstance*	pInst = (ChPlugInInstance*)instance->ndata;
		ChString 				strURL( url );
		ChPlugInStream* 	pStream = 0;

		if ( window == NULL )
		{

			ChString strMimeType;
			pStream = new ChPlugInStream( instance, strURL, strMimeType );
			ASSERT( pStream );

		}

		// Request the URL
		pInst->GetHtmlView()->SpawnPlugInRequest( strURL, pStream, window );
	}
	catch( ... )
	{
		iError = NPERR_GENERIC_ERROR;
	}

	return iError;	
}

 NPError CHPN_PostURL(NPP instance, const char* url, const char* target, 
			uint32 len, const char* buf, NPBool file)
{

	NPError iError =  NPERR_NO_ERROR;

	try
	{
		ChPlugInInstance *pInst = (ChPlugInInstance*)instance->ndata;

		ChString strURL( url );

		ChString strData;
		char* pstrData = strData.GetBuffer( len + 1 );
		ChMemCopy( pstrData, buf, len );
		pstrData[len] = 0;
		strData.ReleaseBuffer();

		pInst->GetHtmlView()->GetFrameMgr()->PostURL( strURL, strData );
	}
	catch( ... )
	{
		iError = NPERR_GENERIC_ERROR;
	}

	return iError;	
}

 NPError CHPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
	return NPERR_GENERIC_ERROR;	
}

NPError CHPN_NewStream( NPP instance, NPMIMEType type,
							  const char* target, NPStream** stream )
{
	return NPERR_GENERIC_ERROR;	
}

 int32   CHPN_Write(NPP instance, NPStream* stream, int32 len, void* buffer)
{
	return len;	
}

 NPError CHPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	return NPERR_GENERIC_ERROR;	
}

 void CHPN_Status(NPP instance, const char* message)
{
	try
	{
		if ( instance )
		{
			ChPlugInInstance*	pInst = (ChPlugInInstance*)instance->ndata;
			CFrameWnd* pFrame = pInst->GetHtmlView()->GetParentFrame( );

			if( pFrame )
			{
				pFrame->SetMessageText( message );
			}
		}
	}
	catch( ... )
	{
	}
 //	TRACE1( "Status message %s\n", message );
}

 const char* CHPN_UserAgent(NPP instance)
{
	return TEXT( "WebTracker" );
}

 void* CHPN_MemAlloc(uint32 size)
{
	return new char[size];	
}

 void CHPN_MemFree(void* ptr)
{
	try 
	{
		delete [] (char*)ptr;
	}
	catch( ... )
	{
	}
}

 uint32  CHPN_MemFlush(uint32 size)
{
	return size;	
}

 void CHPN_ReloadPlugins(NPBool reloadPages)
{
}

 NPError  CHPN_GetURLNotify(NPP instance, const char* url,
								 const char* target, void* notifyData)
{
	NPError iError =  NPERR_NO_ERROR;

	try
	{
		ChPlugInInstance*	pInst = (ChPlugInInstance*)instance->ndata;
		ChString 				strURL( url );
		ChPlugInStream* 	pStream = 0;

		if ( target == NULL )
		{

			ChString strMimeType;
			pStream = new ChPlugInStream( instance, strURL, strMimeType );
			ASSERT( pStream );

		}

		// Request the URL
		pInst->GetHtmlView()->SpawnPlugInRequest( strURL, pStream, target );
	}
	catch( ... )
	{
		iError = NPERR_GENERIC_ERROR;
	}

	return iError;	
}

 NPError  CHPN_PostURLNotify(NPP instance, const char* url,
								  const char* target, uint32 len,
								  const char* buf, NPBool file,
								  void* notifyData)
{
	NPError iError =  NPERR_NO_ERROR;

	try
	{
		ChPlugInInstance *pInst = (ChPlugInInstance*)instance->ndata;

		ChString strURL( url );

		ChString strData;
		char* pstrData = strData.GetBuffer( len + 1 );
		ChMemCopy( pstrData, buf, len );
		pstrData[len] = 0;
		strData.ReleaseBuffer();

		pInst->GetHtmlView()->GetFrameMgr()->PostURL( strURL, strData );
	}
	catch( ... )
	{
		iError = NPERR_GENERIC_ERROR;
	}

	return iError;	
}

 JRIEnv* CHPN_getJavaEnv(void)
{
	return 0;	
}

 jref	CHPN_getJavaPeer(NPP instance)
{
	return 0;	
}

// $Log$
