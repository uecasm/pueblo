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

	This file contains the implementation of the ChHTMLStreamManager class notification
	methods.
----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <fstream>
#include <sys/stat.h>

#include <ChReg.h>

#include "Pueblo.h"
#include "ChClCore.h"
#include "ChCoreStream.h"
#include "ChPbModuleMgr.h"
#include "ChOODDlg.h"


/*----------------------------------------------------------------------------
	ChCoreHTTPReq class
----------------------------------------------------------------------------*/
ChCoreHTTPReq::ChCoreHTTPReq( int iReqID ) :
							m_iReqID( iReqID )
{
}	  

/*----------------------------------------------------------------------------
	ChCoreHTTPReq class
----------------------------------------------------------------------------*/
ChCoreModuleReq::ChCoreModuleReq( int iReqID, 
							ChHTTPStreamManager* pDstStream, chparam userData ) :
							ChCoreHTTPReq( iReqID ),
							m_pModuleStream( pDstStream ),
							m_reqUserData( userData )
{
}

typedef ChSplay< ChString, ChString > ChNameValueList;

static bool ParseColonDataList(const char *pstrFile, ChNameValueList& list) {
	list.Erase();
	
	struct stat temp_stat;
	if(stat( pstrFile, &temp_stat ) == 0) {
		int iFileSize = temp_stat.st_size;
		
		if(iFileSize > 0) {
			std::fstream streamIn( pstrFile, std::ios::in );
	
			if (streamIn.is_open()) {  
				ChString strBuffer;
				char *pstrBuffer = strBuffer.GetBuffer( iFileSize );
				ASSERT( pstrBuffer );
	
				streamIn.read( pstrBuffer, iFileSize );
				pstrBuffer[iFileSize] = 0;
				strBuffer.ReleaseBuffer();
	
				// Compact CRLFs to LFs (for easier parsing)
				strBuffer.Replace("\r\n", "\n");
				// Append LF if not terminated
				if(strBuffer[strBuffer.GetLength() - 1] != '\n')
					strBuffer += '\n';
				
				char*	pstrType = strBuffer.LockBuffer(), *pstrNext, *pstrData;
			
				while (ChUtil::GetNextHeaderString( pstrType, pstrData, pstrNext, 2 )) {
					ChString data(pstrData);
					if(data[0] == ' ')
						data.Delete(0);
					data.Replace("~^", "\r\n");		// nice unlikely delimiter
					list.Insert(pstrType, data);
					
					pstrType = pstrNext;
				}
				strBuffer.UnlockBuffer();
				streamIn.close();
				return true;
			}
			streamIn.close();
		}
	}	
	return false;
}


/*----------------------------------------------------------------------------
	ChCoreStartReq class
----------------------------------------------------------------------------*/
ChCoreStartReq::ChCoreStartReq( int iReqID ) :
							ChCoreHTTPReq( iReqID )
{
}

void ChCoreStartReq::OnStartRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile  )
{
#ifdef UE_VERSION_CHECK
	// Only conduct this version check in the official Pueblo/UE release, since the version numbers
	// won't really mesh otherwise, and non-official versions won't be in the online upgrade database
	// in any case.  So if you're compiling up your own version, do *NOT* define the above constant,
	// unless (at the very least) you change IDS_STARTUP_URL.

	if ( 0 == iReason ) {
		// Startup query successful
		ChNameValueList list;
		if (ParseColonDataList(pstrFile, list)) {
			// use parsed data
			ChString *message = list.Find("Message");
			if (message) {
				ChString strCmdLine;

				if (pCore->GetArgList()->FindArg(CMD_LINE, strCmdLine)) {
					if (!strCmdLine.IsEmpty()) {
						// We can't display the dialog box when auto-connecting to a world, it totally
						// screws up the system.  So instead we'll just defer it to a closedown warning.
						((ChApp *)AfxGetApp())->SetOutdatedTellLater();
						return;
					}
				}

				ChString *pstrCritical = list.Find("Critical");
				ChOutOfDateDlg dlg(pstrCritical ? (pstrCritical->CompareNoCase("true") == 0) : false, *message);
				int result = dlg.DoModal();
				switch (result) {
					case IDC_UPDATE_LATER:
						// They want to keep running; that's fine, but we need to remember
						// that we are out of date, so we can warn them again on shutdown.
						((ChApp *)AfxGetApp())->SetOutdatedTellLater();
						break;
					case IDC_UPDATE_NOW:
						// They want to update now; load up the update website.
						{
							ChString strFormat, strURL;
							LOADSTRING( IDS_UPDATE_URL, strFormat );
							ChString strClientVersion = pCore->GetClientInfo()->
													GetClientVersion().Format( ChVersion::formatShort );
							strURL.Format( strFormat, LPCSTR(strClientVersion) );
							pCore->DisplayWebPage(strURL, ChCore::browserExternal);
						}
						
						// Fall through to shutdown the client
					case IDC_SHUTDOWN:
						// They just want to quit.
						pCore->GetFrameWnd()->PostMessage(WM_CLOSE);
						break;
				}
			}
		}
	}
#endif		// UE_VERSION_CHECK defined
}
/*----------------------------------------------------------------------------
	ChCoreQuitReq class
----------------------------------------------------------------------------*/
ChCoreQuitReq::ChCoreQuitReq( int iReqID ) :
							ChCoreHTTPReq( iReqID )
{
}

void ChCoreQuitReq::OnQuitRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile  )
{

}

/*----------------------------------------------------------------------------
	ChCoreQuitReq class
----------------------------------------------------------------------------*/
ChCoreRegisterReq::ChCoreRegisterReq( int iReqID, const ChString& strRegInfo ) :
							ChCoreHTTPReq( iReqID ),
							m_strRegInfo( strRegInfo )
{
}

void ChCoreRegisterReq::OnRegisterRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile  )
{
	if ( 0 == iReason )
	{ // Registration sucessfull
		ChRegistry	regInfo( CH_GENERAL_GROUP );
		regInfo.Write( CH_REGISTERED, CH_REGISTRATION_NOTIFIED );
		// Remove the saved file
		ChString strName;
		LOADSTRING( IDS_REGINFO_FILE, strName );
	  	ChString strRegInfoFile( pCore->GetModuleMgr()->GetAppDirectory() + strName );
		::SetFileAttributes( strRegInfoFile, FILE_ATTRIBUTE_NORMAL );
		::DeleteFile( strRegInfoFile );
	}
}




/*----------------------------------------------------------------------------
	ChCoreStreamManager class
----------------------------------------------------------------------------*/

int ChCoreStreamManager::NewStream( chparam requestData, pChHTTPStream pStream, bool boolSeekable  )
{
	int iType = ChCoreHTTPReq::coreReq;

	ChCoreHTTPReq* pReq = (ChCoreHTTPReq*)requestData;

	if ( pReq )
	{
		iType = pReq->GetType();

		if ( pReq->GetReqID() != m_pCore->GetReqID() 
					|| m_pCore->GetClientMode() != ChClientCore::modeNormal )
		{
			return streamAsFile;
		}
	}

	switch( iType )
	{
		case ChCoreHTTPReq::moduleReq :
		{
			ChCoreModuleReq* pModReq = (ChCoreModuleReq*)requestData;

			if ( pModReq->GetModuleStream() )
			{  
			 	return pModReq->GetModuleStream()->NewStream( pModReq->GetModuleUserData(),
			 											pStream, boolSeekable );
			}
			else
			{
				return streamAsFile;
			}
		}
		case ChCoreHTTPReq::coreReq :
		case ChCoreHTTPReq::startReq :
		case ChCoreHTTPReq::quitReq :
		case ChCoreHTTPReq::registerReq :
		default :
		{
			return streamAsFile;
		}
	}


	//return streamAsFile;
}

chint32 ChCoreStreamManager::WriteReady( chparam requestData, pChHTTPStream pStream, chint32 iBytes )
{
	int iType = ChCoreHTTPReq::coreReq;

	ChCoreHTTPReq* pReq = (ChCoreHTTPReq*)requestData;

	if ( pReq )
	{
		iType = pReq->GetType();

		if ( pReq->GetReqID() != m_pCore->GetReqID()
					|| m_pCore->GetClientMode() != ChClientCore::modeNormal )
		{
			return iBytes;
		}

	}

	switch( iType )
	{
		case ChCoreHTTPReq::moduleReq :
		{
			ChCoreModuleReq* pModReq = (ChCoreModuleReq*)requestData;

			if ( pModReq->GetModuleStream() )
			{  
			 	return pModReq->GetModuleStream()->WriteReady( pModReq->GetModuleUserData(),
			 											pStream, iBytes );
			}
			else
			{
				return iBytes;
			}
		}
		case ChCoreHTTPReq::coreReq :
		default :
		{
			return iBytes;
		}
	}

	//return iBytes;

}
chint32 ChCoreStreamManager::Write( chparam requestData, pChHTTPStream pStream, 
	    								chint32 lOffset, chint32 lLen, const char* pBuffer )
{
	int iType = ChCoreHTTPReq::coreReq;

	ChCoreHTTPReq* pReq = (ChCoreHTTPReq*)requestData;

	if ( pReq )
	{
		iType = pReq->GetType();

		if ( pReq->GetReqID() != m_pCore->GetReqID()
					|| m_pCore->GetClientMode() != ChClientCore::modeNormal )
		{
			return lLen;
		}
	}

	switch( iType )
	{
		case ChCoreHTTPReq::moduleReq :
		{
			ChCoreModuleReq* pModReq = (ChCoreModuleReq*)requestData;

			if ( pModReq->GetModuleStream() )
			{  
			 	return pModReq->GetModuleStream()->Write( pModReq->GetModuleUserData(),
			 											pStream, lOffset,
			 											lLen, pBuffer );
			}
			else
			{
				return lLen;
			}
		}
		case ChCoreHTTPReq::coreReq :
		default :
		{
			return lLen;
		}
	}

	//return lLen;

}



void ChCoreStreamManager::StreamAsFile(chparam requestData, pChHTTPStream pStream, const char* fname)
{
	int iType = ChCoreHTTPReq::coreReq;

	ChCoreHTTPReq* pReq = (ChCoreHTTPReq*)requestData;

	if ( pReq )
	{
		iType = pReq->GetType();

		if ( pReq->GetReqID() != m_pCore->GetReqID()
					|| m_pCore->GetClientMode() != ChClientCore::modeNormal )
		{
			return;
		}
	}

	switch( iType )
	{
		case ChCoreHTTPReq::moduleReq :
		{
			ChCoreModuleReq* pModReq = (ChCoreModuleReq*)requestData;

			if ( pModReq->GetModuleStream() )
			{  
			 	pModReq->GetModuleStream()->StreamAsFile( pModReq->GetModuleUserData(),
			 											pStream, fname  );
			}
			break;
		}

		case ChCoreHTTPReq::coreReq :
		default :
		{
			break;
		}
	}
}

void ChCoreStreamManager::DestroyStream( chparam requestData, pChHTTPStream pStream, int iReason )
{
	int iType = ChCoreHTTPReq::coreReq;

	ChCoreHTTPReq* pReq = (ChCoreHTTPReq*)requestData;

	if ( pReq )
	{
		iType = pReq->GetType();

		if ( pReq->GetReqID() != m_pCore->GetReqID()
					|| m_pCore->GetClientMode() != ChClientCore::modeNormal )
		{
			// Too late to use the request abort
			TRACE( "Aborting requests, too late to use\n" );
			delete 	pReq;
			return;
		}

	}

	#if 0
	{	// This function is called from different threads and Trace method is not 
	   // Thread safe. We need to make Trace automatically handle calls from different threads
	    // and call method of ChHtmlWnd from the thread which it was created.
		if (iReason && m_pCore->GetTraceOptions() & ChCore::traceErrors )
		{						
			ChString strMsg( pStream->GetURL() );
			strMsg += TEXT( " : " );
			strMsg += pStream->GetErrorMsg();
			m_pCore->Trace( strMsg, ChCore::traceErrors, true );
		}
	}
	#endif

	switch( iType )
	{
		case ChCoreHTTPReq::moduleReq :
		{
			ChCoreModuleReq* pModReq = (ChCoreModuleReq*)requestData;

			if ( pModReq->GetModuleStream() )
			{  
			 	pModReq->GetModuleStream()->DestroyStream( pModReq->GetModuleUserData(),
			 											pStream, iReason  );
			}
			break;
		}

		case ChCoreHTTPReq::startReq :
		{
			ChCoreStartReq* pStartReq = (ChCoreStartReq*)requestData;
			pStartReq->OnStartRequestComplete( m_pCore, iReason, pStream->GetCacheFilename() );
			break;
		}
		case ChCoreHTTPReq::quitReq :
		{
			ChCoreQuitReq* pQuitReq = (ChCoreQuitReq*)requestData;
			pQuitReq->OnQuitRequestComplete( m_pCore, iReason, pStream->GetCacheFilename() );
			break;
		}
		case ChCoreHTTPReq::registerReq :
		{
			ChCoreRegisterReq* pRegReq = (ChCoreRegisterReq*)requestData;
			pRegReq->OnRegisterRequestComplete( m_pCore, iReason, pStream->GetCacheFilename() );
			break;
		}
	case ChCoreHTTPReq::coreReq :
		default :
		{
			break;
		}
	}
	// Done with the user data
	delete 	pReq;
}

// $Log$
