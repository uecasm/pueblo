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

	Interface for the ChWorldStreamManager class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChHTTP.h>
#include <ChUtil.h>	
#include <ChConst.h>

#include <ChCore.h>

#include "World.h"

#include "ChWorldStream.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	ChWorldConnectHTTPReq class
----------------------------------------------------------------------------*/
void ChWorldConnectHTTPReq::OnConnectRequestComplete( ChWorldMainInfo* pMainInfo, 
											int iReason, const char* pstrFile  )
{
}

/*----------------------------------------------------------------------------
	ChWorldDisconnectHTTPReq class
----------------------------------------------------------------------------*/
void ChWorldDisconnectHTTPReq::OnDisconnectRequestComplete( ChWorldMainInfo* pMainInfo, 
											int iReason, const char* pstrFile  )
{
}

/*----------------------------------------------------------------------------
	ChWorldEnhancedHTTPReq class
----------------------------------------------------------------------------*/
void ChWorldEnhancedHTTPReq::OnEnhancedRequestComplete( ChWorldMainInfo* pMainInfo, 
											int iReason, const char* pstrFile  )
{
	if ( 0 == iReason )
	{
		char * pstrFilename = new char[ lstrlen( pstrFile ) + 1 ];
		ASSERT( pstrFilename );
		lstrcpy( pstrFilename, pstrFile );
		if ( !::PostMessage( GetNotifyWnd(), WM_EXECUTE_PUEBLO_SCRIPT, 0,
								 (LPARAM)pstrFilename ) )
		{  // if successful don't delete, will be deleted by the message handler
			delete [] pstrFilename;
		}
	}
}




/*----------------------------------------------------------------------------
	ChWorldStreamManager class
----------------------------------------------------------------------------*/

ChWorldStreamManager::ChWorldStreamManager( ChWorldMainInfo* pMainInfo, ChModuleID idModule )  :
						m_pMainInfo( pMainInfo ),
						m_idModule( idModule )
{

}


int ChWorldStreamManager::NewStream( chparam requestData,
										pChHTTPStream pStream,
										bool boolSeekable )
{
	return streamAsFile;
}


void ChWorldStreamManager::StreamAsFile( chparam requestData,
											pChHTTPStream pStream,
											const char* pstrFilename )
{
}


void ChWorldStreamManager::DestroyStream( chparam requestData,
											pChHTTPStream pStream,
											int iReason )
{
	ChWorldHTTPReq* pReq = ( ChWorldHTTPReq*)requestData;

	ASSERT( pReq );

	if ( pReq->GetReqID() != m_pMainInfo->GetConnectID() )
	{ 	// Too late to use the request
	 	delete pReq;
		return;
	}

	switch ( pReq->GetRequestType() )
	{
		case ChWorldHTTPReq::typeWorldConnect :
		{
			ChWorldConnectHTTPReq* pInfo = ( ChWorldConnectHTTPReq*)requestData;
			pInfo->OnConnectRequestComplete( m_pMainInfo, iReason, 
								pStream->GetCacheFilename() );
			break;
		}
		case ChWorldHTTPReq::typeWorldDisconnect :
		{
			ChWorldDisconnectHTTPReq* pInfo = ( ChWorldDisconnectHTTPReq*)requestData;
			pInfo->OnDisconnectRequestComplete( m_pMainInfo, iReason, 
								pStream->GetCacheFilename() );
			break;
		}
		case ChWorldHTTPReq::typeWorldEnhanced :
		{
			ChWorldEnhancedHTTPReq* pInfo = ( ChWorldEnhancedHTTPReq*)requestData;
			pInfo->OnEnhancedRequestComplete( m_pMainInfo, iReason, 
								pStream->GetCacheFilename() );
			break;
		}
		case ChWorldHTTPReq::typeFile :
		{

			ChWorldFileHTTPReq* pInfo = ( ChWorldFileHTTPReq*)requestData;
			if ( 0 == iReason )
			{
				pInfo->SetLoadComplete( pStream->GetURL(), pStream->GetCacheFilename(), 
											pStream->GetMimeType() );
			}
			else
			{
				pInfo->SetLoadError( pStream->GetURL(), pStream->GetErrorMsg() );
			}

			if ( ::PostMessage( pInfo->GetNotifyWnd(), WM_MODULE_HTTP_REQ_MSG, iReason,
									 (LPARAM)requestData ) )
			{  // if successful don't delete, will be deleted by the message handler
				pReq = 0;
			}
			
		}
		default :
		{
			break;
		}
	}

	delete pReq;
}

///////////////////////////////////////////////////////////////////////////////////////////////
/////////
/////////	 ChWorldScript
/////////
//////////////////////////////////////////////////////////////////////////////////////////////

ChWorldScript::ChWorldScript( ChWorldMainInfo* pInfo ) :
						ChPuebloScript(),
						m_pMainInfo( pInfo )
{
}


bool ChWorldScript::ExecuteCommand( int iCommand,  ChArgumentList& attrList,
									ChArgumentList& argList )
{
	if (iCommand == cmdMessage)
	{
		ChString		strVal;

		if (attrList.FindArg( "message", strVal ))
		{
			if (0 == strVal.CompareNoCase( TEXT( "WorldDisconnect" ) ))
			{
											// Disconnect from the world

				if (m_pMainInfo->IsConnected())
				{
											// Display the message string

					if (argList.FindArg( TEXT( "reason" ), strVal ))
					{
						if (!strVal.IsEmpty())
						{
							ChString		strErrorCaption;

							LOADSTRING( IDS_APP_NAME, strErrorCaption );
							m_pMainInfo->GetCore()->GetFrameWnd()->
											MessageBox( strVal, strErrorCaption,
														MB_OK | MB_ICONSTOP );
						}
					}

					m_pMainInfo->ShutdownWorld( false );
				}
			}
		}

		return true;
	}

	return false;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:32  uecasm
// Import of source tree as at version 2.53 release.
//
