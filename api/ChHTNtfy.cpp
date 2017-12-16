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

	This file contains the implementation of the ChHTTPInfo class notification
	methods.
----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChTypes.h>
#include <ChConst.h>
#include <ChHtpCon.h>
#include <ChUtil.h>
#include <ChDb.h>
#include <SocketXX.h>
#include "ChHTPriv.h"
#include "ChHttpThreadMgr.h" 

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"	 
#endif

#include <MemDebug.h>

void ChHTTPInfo::UpdateStatusInfo( bool boolActive /* = true */ )
{
	// Update the num active 
	// Serialize HTTP access 
	if ( m_pSocket )
	{ // if we have a socket conn then add to the list 	


		// update the bytes read
		// serialize access to stream state
		LockHTTP();

		if ( boolActive && m_boolUpdateInfo )
		{
			UnlockHTTP();
			return;
		}

		if (  !boolActive && !m_boolUpdateInfo  )
		{
			UnlockHTTP();
			return;
		}


		int 		iActive = 1;
		chint32		lBytes = m_luBodyLen;
		
		if ( !boolActive )
		{
			iActive = -1;
			lBytes *= -1;	
		}

		// Set the state of active info for this connection
		m_boolUpdateInfo = boolActive;

		if ( GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
		{
		 	GetHTTPConn()->m_connState.iTotalActive += iActive;
			if ( m_luBodyLen != 0xFFFFFFFF )
			{
		 		GetHTTPConn()->m_connState.luTotalBytes += lBytes;
			}
		}
		else
		{
		 	GetHTTPConn()->m_connState.iTotalActive += iActive;
		 	GetHTTPConn()->m_connGlobalState.iTotalActive += iActive;

			if ( m_luBodyLen != 0xFFFFFFFF )
			{
		 		GetHTTPConn()->m_connState.luTotalBytes += lBytes;
		 		GetHTTPConn()->m_connGlobalState.luTotalBytes += lBytes;
			}
		}
		// Unlock serialization
		UnlockHTTP();
	}
}

/*----------------------------------------------------------------------------

	FUNCTION		||	AsyncNotifyProgress

	iError 			|| Error code

-------------------------------DESCRIPTION-----------------------------------

Notify the progress for the request.

----------------------------------------------------------------------------*/
void ChHTTPInfo::NotifyProgress(  )
{

	// Serialize HTTP access 	
	LockHTTP();

	// update the bytes read
	if ( GetHTTPConn()->GetConnOptions() & ChHTTPConn::connLocalState)
	{
	 	GetHTTPConn()->m_connState.luTotalReceived += m_luToProcess;
	}
	else
	{
	 	GetHTTPConn()->m_connState.luTotalReceived += m_luToProcess;
	 	GetHTTPConn()->m_connGlobalState.luTotalReceived += m_luToProcess;
	}
	// Unlock serialization
	UnlockHTTP();

	// If progress message is required, notify
	if ( GetHTTPConn()->GetConnOptions() &  ChHTTPConn::connNotifyProgress )
	{
	}

	#if 0
	if ( !(GetHTTPConn()->GetMsgMask() & ChHTTPSocketConn::msgProgress) )
	{
	 	return;
	}

	#if defined( CH_MSW )
	if ( GetHTTPConn()->GetNotificationWnd() )
	{

		switch( iMsgType )
		{
			case ChHTTPInfo::msgBeginProgress :
			{
				break;
			}
			case ChHTTPInfo::msgEndProgress :
			{
				ChHTTPNotification* pInfo = new ChHTTPNotification( 
										ChHTTPNotification::msgEndProgress ); 
				ASSERT( pInfo );

				if ( !::PostMessage( GetHTTPConn()->GetNotificationWnd(), 
								WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo ) )
				{
					TRACE1("Post message failed %ld\n", ::GetLastError() );
					delete pInfo;
				}

				break;
			}

			case ChHTTPInfo::msgProgress :
			{
				ChString strMsg;

				strMsg.Format( "Document: Received %ld of %ld bytes", 
										ChHTTPInfo::GetTotalBytesRead(),
										ChHTTPInfo::GetTotalBytesToRead() );

				if ( m_luTotalBytes == 0 )
				{
					m_luTotalBytes = m_luBytesRead;
				}

				DWORD dwProgress = 100 * 
						ChHTTPInfo::GetTotalBytesRead() / ChHTTPInfo::GetTotalBytesToRead();

				if ( dwProgress > 100 )
				{
					dwProgress = 100;
				}

				ChHTTPNotification* pInfo = new ChHTTPNotification(
										 ChHTTPNotification::msgMakingProgress,
										strMsg, dwProgress, 100 ); 
				ASSERT( pInfo );

				if ( !::PostMessage( GetHTTPConn()->GetNotificationWnd(), 
								WM_CHACO_HTTP_MSG, 0, (LPARAM)pInfo ) )
				{
					TRACE1("Post message failed %ld\n", ::GetLastError() );
					delete pInfo;
				}

				break;
			}
		}

	}
	#endif
	#endif

}  



/*----------------------------------------------------------------------------

	FUNCTION		||	InitStream

	iError 			|| Error code

-------------------------------DESCRIPTION-----------------------------------

Stream and notify the request
----------------------------------------------------------------------------*/

void ChHTTPInfo::InitStream( )
{

	if ( GetThread() )
	{
		GetThread()->LockStream();
	}

	if (  m_iStreamState == streamUnInitialized &&
			GetStreamMgr() )
	{
		m_iStreamState = streamInitialized;


		m_httpStream.m_uStreamOption = GetStreamMgr()->
					NewStream(  GetUserData(), &m_httpStream, false );

	}

	if ( GetThread() )
	{
		GetThread()->UnlockStream();
	}
}





/*----------------------------------------------------------------------------

	FUNCTION		||	StreamClose

	iError 			|| Error code

-------------------------------DESCRIPTION-----------------------------------

Stream and notify the request
----------------------------------------------------------------------------*/

void ChHTTPInfo::CloseStream( )
{  
	// Update the num active 
	// Serialize HTTP access 
	if ( m_pSocket )
	{ // if we have a socket conn then add to the list 	

		LockHTTP();

		if ( m_boolUpdateInfo )
		{
			UpdateStatusInfo( false );
		}

		// Unlock serialization
		UnlockHTTP();
	}

	if ( GetThread() )
	{
		GetThread()->LockStream();
	}

	// if stream is already destroyed, do nothing 
	if (  m_iStreamState != streamDestroyed &&	GetStreamMgr() )
	{
		// Initialize if not initialized
		if (  m_iStreamState == streamUnInitialized )
		{
			m_httpStream.m_uStreamOption = GetStreamMgr()->
						NewStream(  GetUserData(), &m_httpStream, false );
			m_iStreamState = streamInitialized;
		}


		if (  m_iStreamState == streamInitialized )
		{
			if ( 0 == GetError() )
			{	// stream file if everything is OK
				if ( m_httpStream.m_uStreamOption & ChHTTPStreamManager::streamAsFile )
				{
					GetStreamMgr()->StreamAsFile( GetUserData(), 
												&m_httpStream, GetFileName() );	
				}

			}
		}

		GetStreamMgr()->DestroyStream( GetUserData(), &m_httpStream, GetError() );	

		m_iStreamState = streamDestroyed;
	}

	if ( GetThread() )
	{
		GetThread()->UnlockStream();
	}


}

/*----------------------------------------------------------------------------

	FUNCTION		||	StreamURL

	iError 			|| Error code

-------------------------------DESCRIPTION-----------------------------------

Stream and notify the request
----------------------------------------------------------------------------*/
void ChHTTPInfo::StreamURL( )
{
	if ( !GetStreamMgr() )
	{
		return;
	}


	if ( GetThread() )
	{
		GetThread()->LockStream();
	}

	m_httpStream.m_uStreamOption = GetStreamMgr()->
				NewStream(  GetUserData(), &m_httpStream, false );



	if ( 0 == GetError() )
	{
 		if ( m_httpStream.m_uStreamOption & ChHTTPStreamManager::streamNormal )
		{

			m_iStreamState = streamInData;
			// Open the file
			HANDLE hFile = ::CreateFile(
							    GetFileName(),		// address of name of the file 
							    GENERIC_READ,	// access (read-write) mode 
							    FILE_SHARE_READ,// share mode 
							    NULL,			// address of security descriptor 
							    OPEN_EXISTING,	// how to create 
							    0,				// file attributes 
							    NULL			// handle of file with attributes to copy  
						   );
			if ( hFile != INVALID_HANDLE_VALUE )
			{

				DWORD  dwHigh, dwSize;

				dwSize = ::GetFileSize( hFile, &dwHigh );

				if ( dwSize || dwHigh )
				{
					// Map if we have a good file
					HANDLE hFileMap =  ::CreateFileMapping(
								    			hFile,	// handle of file to map 
								      			NULL,	// optional security attributes 
								    			PAGE_READONLY | SEC_COMMIT,	// protection for mapping object 
								    			0,	// high-order 32 bits of object size  
								    			0,	// low-order 32 bits of object size  
								    			NULL 	// name of file-mapping object 
						   );	

					ASSERT( hFileMap );

					char* pMappedView = (char*) MapViewOfFile(
										    hFileMap,		// file-mapping object to map into address space  
										    FILE_MAP_READ,	// access mode 
										    0,	// high-order 32 bits of file offset 
										    0,	// low-order 32 bits of file offset 
										    0 	// number of bytes to map, 0 = Map the whole file
						   );
					ASSERT( pMappedView );
	
					chint32 offset = 0;
					while ( offset < (chint32)dwSize && !IsAborted()  )
					{  // stream all data
						// See if the plugin is ready
						chint32 iReady =  GetStreamMgr()->WriteReady( GetUserData(), &m_httpStream, dwSize );
						if ( (offset + iReady) > (chint32)dwSize )
						{
							iReady = dwSize;
						}

						if ( iReady )
						{
							 GetStreamMgr()->Write( GetUserData(), &m_httpStream,  
							 						offset, iReady,	 pMappedView );
							offset += iReady;
						}
						else
						{
							m_iError = CH_HTTP_ERROR_GENERIC;
							break;
						}

					}
					::UnmapViewOfFile( pMappedView );
					::CloseHandle( hFileMap );
				}
				::CloseHandle( hFile );
			}
			else
			{
				m_iError = CH_HTTP_ERROR_OPEN_FILE;
			}
		}
		// stream as file if requested
		if ( m_httpStream.m_uStreamOption & ChHTTPStreamManager::streamAsFile )
		{
			GetStreamMgr()->StreamAsFile( GetUserData(), 
										&m_httpStream, GetFileName() );	
		}
	}

	// close the stream
	GetStreamMgr()->DestroyStream( GetUserData(), &m_httpStream, GetError() );	

	m_iStreamState = streamDestroyed;
	
	if ( GetThread() )
	{
		GetThread()->UnlockStream();
	}
}


///////////////////////////////////////////////////////////////////////////////
/////////////  ChHTTPStream
////////////
///////////////////////////////////////////////////////////////////////////////

ChHTTPConnStream::ChHTTPConnStream() : ChHTTPStream(),
			m_pUserStreamData(0),
			m_pHTTPInfo(0),
 			m_uStreamOption(0)

{

}
const ChString& ChHTTPConnStream::GetURL()
{
	return m_pHTTPInfo->GetURL();
}

const ChString& ChHTTPConnStream::GetMimeType()
{
	return m_pHTTPInfo->GetMimeType();
}

const ChString& ChHTTPConnStream::GetCacheFilename()
{
	return m_pHTTPInfo->GetFileName();
}

const ChString& ChHTTPConnStream::GetContentEncoding()
{
	return m_pHTTPInfo->GetTargetWindowName();
}

const ChString& ChHTTPConnStream::GetLastModified()
{
	return m_pHTTPInfo->GetLastModified();
}

const ChString& ChHTTPConnStream::GetTargetWindowName()
{
	return m_pHTTPInfo->GetTargetWindowName();
}

long ChHTTPConnStream::GetContentLength()
{
	return m_pHTTPInfo->GetDataLength();
}


void* ChHTTPConnStream::GetStreamPrivateData()
{
	return m_pUserStreamData;					
}
// Attributes
void ChHTTPConnStream::SetStreamPrivateData( void *pData )
{
	m_pUserStreamData = pData;
}

const char* ChHTTPConnStream::GetErrorMsg()
{

	char * pstrBuffer = m_pHTTPInfo->GetBuffer();
	if ( !m_pHTTPInfo->GetBufferSize())
	{
		pstrBuffer = new char[ 250 ];
		m_pHTTPInfo->SetBuffer( pstrBuffer );
		*pstrBuffer	= 0;
	}

	HINSTANCE hInst;
	#if !defined(CH_PUEBLO_PLUGIN)
	hInst = PuebloDLL.hModule;
	#else
	hInst = AfxGetInstanceHandle( );
	#endif


	ASSERT( pstrBuffer );

	switch( m_pHTTPInfo->GetError() )
	{
		case CH_HTTP_ERROR_LOCAL_IN_USE :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_600, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_ABORT :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_601, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_CONNECT :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_602, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_SEND :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_603, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_INVALID_HDR :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_604, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_NOT_CACHED :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_605, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_USER_ABORT :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_606, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_INVALID_DATA :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_607, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_BAD_URL :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_608, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_OPEN_FILE :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_609, pstrBuffer, 250 );
			break;
		}
		case CH_HTTP_ERROR_GENERIC :		
		{
			::LoadString( hInst, IDS_HTTP_ERR_610, pstrBuffer, 250 );
			break;
		}
		default :
		{  // Buffer should contain the error message
			if ( !pstrBuffer[0] )
			{
				::wsprintf( pstrBuffer, "HTTP error : %d", m_pHTTPInfo->GetError() );
			}
		}
	}
	return pstrBuffer;
}
