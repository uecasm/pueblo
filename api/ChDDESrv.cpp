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

	This file consists of the Chaco DDE server interface to WEB clients.

----------------------------------------------------------------------------*/

// $Header$

//#include "grheader.h"
#include "headers.h"

#include <stdarg.h>
#include <ChHTTP.h>
#include <ChUtil.h>
#include <ChDDE.h>
#include "ChDDEPrv.h"

//	Serialization, runtime info, dynamic creation.
IMPLEMENT_SERIAL( CDDEConversation, CObject, 1 )

#include <MemDebug.h>

//	Static initialization.
CObList *CDDEConversation::m_pcolConversations = 0;


/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
//	Construction
CDDEConversation::CDDEConversation()	
{
	m_hConv = NULL;
	CommonConstruction();
}

CDDEConversation::CDDEConversation(HCONV hConv)	
{
	m_hConv = hConv;
	CommonConstruction();
}

/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
void CDDEConversation::CommonConstruction()	
{
	//	Initialize everything but the conversation handle.
	if ( m_pcolConversations == 0 )
	{
		m_pcolConversations = new CObList( 20 );
		ASSERT( m_pcolConversations );
	}
	m_rIndex = m_pcolConversations->AddTail(this);
	
	//TRACE("Creating Conversation %p\n", this);
}

/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
//	Destruction.
CDDEConversation::~CDDEConversation()	
{
	//	Remove ourselves from the list of running conversations.
	m_pcolConversations->RemoveAt(m_rIndex);

	if ( m_pcolConversations->GetCount() == 0 )
	{
		delete m_pcolConversations;
		m_pcolConversations = 0;
	}
	
	//TRACE("Destroying Conversation %p\n", this);
}

/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
ChHTTPDDE *CDDEConversation::GetDDEConn()	
{
	CDDEObject *pDDE = CDDEObject::ResolveConversation(this);
	if(pDDE == NULL)	
	{
		return(NULL);
	}
	return pDDE->GetDDEConn();
}

/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
//	Proactive disconnection.
void CDDEConversation::DoDisconnect()	
{
	//	Just disconnect from the current HCONV.
	DdeDisconnect(m_hConv);	
	GetDDEConn()->Display(ExplainError());
	CDDEObject::DDEDisconnect(this);
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CDDEConversation 
//	Informaitonal Lookup.
CDDEConversation *CDDEConversation::ResolveConversation(HCONV hConv)	
{
	//	Loop through all conversations, and return the obect handling the
	//		conversation.
	POSITION rTraverse = m_pcolConversations->GetHeadPosition();
	CDDEConversation *pRetVal = NULL;
	while(rTraverse != NULL)	
	{
		pRetVal = (CDDEConversation *)m_pcolConversations->GetNext(rTraverse);
		if(pRetVal->m_hConv == hConv)	
		{
			break;
		}
		else	
		{
			pRetVal = NULL;
		}
	}
	
	return(pRetVal);
}

//	Utility
void CDDEConversation::ClientPassed(HSZ hszArgs, const char *pFormat, ...)	
{
	//	Initialize variable number of argumetns.
	va_list VarList;
	va_start(VarList, pFormat);	
	
	int i_ArgNum = 0;
	char *pScan = (char *)pFormat;
	char *pExtract;
	
	//	Loop through the arguments we are going to parse.
	while(pScan && *pScan)	
	{
		//	What argument are we currently looking for?
		i_ArgNum++;
		pExtract = ExtractArgument(hszArgs, i_ArgNum);
	
		if(0 == strncmp(pScan, "DW", 2))	
		{
			//	DWORD.
			DWORD *pWord;
			pWord = va_arg(VarList, DWORD *);
			
			if(pExtract == NULL)	
			{
				*pWord = 0x0;
			}
			else	{
				sscanf(pExtract, "%lu", pWord);
			}
		}
		else if(0 == strncmp(pScan, "QCS", 3))	
		{
			//	A quoted CString
			CString *pCS = va_arg(VarList, CString *);
			
			if(pExtract == NULL)	
			{
				pCS->Empty();
			}
			else	
			{
				//	Extractions took off the leading and ending quotes.
				char *pCopy = pExtract;
				while(*pCopy)	{
					if(*pCopy == '\\' && *(pCopy + 1) == '\"')	
					{
						pCopy++;
					}
					
					*pCS += *pCopy;
					pCopy++;
				}
			}
		}
		else if(0 == strncmp(pScan, "CS", 2))	
		{
			//	A CString
			CString *pCS = va_arg(VarList, CString *);

			if(pExtract == NULL)	
			{
				pCS->Empty();
			}
			else	{
				*pCS = pExtract;
			}
		}
		else if(0 == strncmp(pScan, "BL", 2))	
		{
			//	A boolean
			TwoByteBool *pBool = va_arg(VarList, TwoByteBool *);
			
			if(pExtract == NULL)	
			{
				*pBool = FALSE;
			}
			else	
			{
				//	Compare for a TRUE or a FALSE
				if(0 == stricmp(pExtract, "TRUE"))	
				{
					*pBool = TRUE;
				}
				else	{
					*pBool = FALSE;
				}
			}
		}
		
		//	Go on to the next argument in our format string.
		pScan = SkipToNextArgument(pScan);
		
		//	Free the memory that was used during extraction.
		if(pExtract != NULL)	
		{
			delete pExtract;
		}
	}
	
	//	Done with variable number of arguments
	va_end(VarList);
}

HDDEDATA CDDEConversation::ServerReturns( HSZ hszItem, const char *pFormat, ...)	
{
	va_list VarList;
	va_start(VarList, pFormat);

	char *pTraverse = (char *)pFormat;
	char caNumpad[64];
	CString csBuffer;
	CString csRetval;
	
	while(*pTraverse)	
	{
		//	Erase temp data from our last pass.
		caNumpad[0] = '\0';
		csBuffer.Empty();
	
		//	Compare our current format to the known formats
		if(0 == strncmp(pTraverse, "DW", 2))	
		{
			//	A DWORD.
			DWORD *pWord = va_arg(VarList, DWORD *);
			
			if(pWord != NULL)	
			{
        //  See if we're to use hex or not.
        #if (FALSE)  
          sprintf(caNumpad, "%#lx", *pWord);
        #else
			    sprintf(caNumpad, "%lu", *pWord);
        #endif
				csRetval += caNumpad;
			}
		}
		else if(0 == strncmp(pTraverse, "CS", 2))	
		{
			//	A CString, not quoted
			CString *pCS = va_arg(VarList, CString *);
			
			if(pCS != NULL)	{
				csRetval += *pCS;
			}
		}
		else if(0 == strncmp(pTraverse, "QCS", 3))	
		{
			//	A quoted CString
			CString *pQCS = va_arg(VarList, CString *);
			
			if(pQCS != NULL)	
			{
				csRetval += '\"';
				
				//	Need to escape any '"' to '\"', literally.
				char *pConvert = (char *)(const char *)*pQCS;
				while(*pConvert != '\0')	
				{
					if(*pConvert == '\"')	
					{
						csRetval += '\\';
					}
					csRetval += *pConvert;
					pConvert++;
				}
				csRetval += '\"';
			}
		}
		else if(0 == strncmp(pTraverse, "BL", 2))	
		{
			//	A boolean
			TwoByteBool *pBool = va_arg(VarList, TwoByteBool *);
			
			if(pBool != NULL)	{
				if(*pBool != FALSE)	{
					csRetval += "TRUE";
				}
				else	{
					csRetval += "FALSE";
				}
			}
		}

		
		pTraverse = SkipToNextArgument(pTraverse);
		
		if(*pTraverse != '\0')	
		{
			csRetval += ',';
		}
	}

	//	Done with varargs.
	va_end(VarList);
	
	if(csRetval.IsEmpty())	
	{
		return(NULL);
	}

	HDDEDATA Final;
	Final = DdeCreateDataHandle(CDDEObject::m_dwidInst,
		#ifdef _WIN32
        (unsigned char *)(const char *)csRetval,
		#else
		(void *)(const char *)csRetval, 
		#endif // _WIN32
 		csRetval.GetLength() + 1, 0,
 		hszItem,
		CF_TEXT, 0);
	GetDDEConn()->Display(ExplainError());

	//	THIS IS A HACK.
	if(strchr(pFormat, ',') == NULL)	
	{
		if(strcmp(pFormat, "BL") == 0)	
		{
			TwoByteBool bData = FALSE;
			if(csRetval == "TRUE")	
			{
				bData = TRUE;
			}
			DdeFreeDataHandle(Final);
			GetDDEConn()->Display(ExplainError());
			
			Final = DdeCreateDataHandle(CDDEObject::m_dwidInst,
				(unsigned char *)&bData, sizeof(TwoByteBool), 0,
				hszItem, CF_TEXT, 0);
			GetDDEConn()->Display(ExplainError());
		}
		else if(strcmp(pFormat, "DW") == 0)	
		{
			DWORD dwData;
			dwData = strtoul(csRetval, NULL, 0);
			DdeFreeDataHandle(Final);
			GetDDEConn()->Display(ExplainError());
			
			Final = DdeCreateDataHandle(CDDEObject::m_dwidInst,
				(unsigned char *)&dwData, sizeof(DWORD), 0,
				hszItem, CF_TEXT, 0);
			GetDDEConn()->Display(ExplainError());
		}
	}

	return(Final);
}

//	Handle DDE Pokes
HDDEDATA CDDEConversation::ServerPoke(HSZ hszTopic, HSZ hszItem, HDDEDATA hDataPoke)	
{
	//	Switch on the topic we're handling.
	int iTopic = CDDEObject::EnumTopic(hszTopic);
	switch(iTopic)	
	{
		case CDDEObject::m_EndProgress:
			return(WWW_EndProgress(hszItem));
		case CDDEObject::m_SetProgressRange:
			return(WWW_SetProgressRange(hszItem));
//		case CDDEObject::m_URLEcho:
//			return(WWW_URLEcho(hszItem));
		case CDDEObject::m_ViewDocFile:
			return(WWW_ViewDocFile(hszItem));
//		case CDDEObject::m_WindowChange:
//			return(WWW_WindowChange(hszItem));
		case CDDEObject::m_OpenURLResult:
			return(WWW_OpenURLResult(hszItem));
		case CDDEObject::m_BeginProgress:
			return(WWW_BeginProgress(hszItem));

	}

	TRACE("Unknown poke received\n");
	return(DDE_FNOTPROCESSED);
}

HDDEDATA CDDEConversation::WWW_EndProgress(HSZ hszItem)	
{
	DWORD dwTransactionID;
	
	ClientPassed(hszItem, "DW", &dwTransactionID);
	
	GetDDEConn()->EndProgress( dwTransactionID );

	return(DDEFACK);
}

HDDEDATA CDDEConversation::WWW_SetProgressRange(HSZ hszItem)	
{
	DWORD dwTransactionID;
	DWORD dwMaximum;
	
	ClientPassed(hszItem, "DW,DW", &dwTransactionID, &dwMaximum);
	
	GetDDEConn()->SetProgressRange( dwTransactionID, dwMaximum );
	
	return(DDEFACK);
}

#if 0
HDDEDATA CDDEConversation::WWW_URLEcho(HSZ hszItem)	
{
	CString csURL;
	CString csMIMEType;
	DWORD dwWindowID;
	CString csReferrer;
	
	ClientPassed(hszItem, "QCS,QCS,DW,QCS", &csURL, &csMIMEType, &dwWindowID,
		&csReferrer);
		
	char aBuf[1024];
	sprintf(aBuf, "%s WWW_URLEcho(%s,%s,%lu,%s);", (const char *)ExplainPoke(DDEFACK),
		(const char *)csURL, (const char *)csMIMEType, dwWindowID, (const char *)csReferrer);
	GetDDEConn()->Display(aBuf);
		
	return(DDEFACK);
}
#endif

HDDEDATA CDDEConversation::WWW_ViewDocFile(HSZ hszItem)	
{
	CString csFileName;
	CString csURL;
	CString csMIMEType;
	DWORD dwWindowID;
	
	ClientPassed(hszItem, "QCS,QCS,QCS,DW", &csFileName, &csURL, &csMIMEType, &dwWindowID);
	
	#ifdef _DEBUG
	CString strMsg;
	strMsg.Format( "%s WWW_ViewDocFile(%s,%s,%s,%lu);\n", (const char *)ExplainPoke(DDEFACK),
		(const char *)csFileName, (const char *)csURL, (const char *)csMIMEType,
		dwWindowID);

	TRACE( strMsg ); 
	#endif

	GetDDEConn()->ViewDocFile( csFileName, csURL, csMIMEType );
	
	return(DDEFACK);
}

#if 0
HDDEDATA CDDEConversation::WWW_WindowChange(HSZ hszItem)	
{
	DWORD dwWindowID;
	DWORD dwWindowFlags;
	DWORD dwX;
	DWORD dwY;
	DWORD dwCX;
	DWORD dwCY;
	
	ClientPassed(hszItem, "DW,DW,DW,DW,DW,DW", &dwWindowID, &dwWindowFlags, &dwX,
		&dwY, &dwCX, &dwCY);
		
	char aBuf[512];
	sprintf(aBuf, "%s WWW_WindowChange(%lu,%lu,%lu,%lu,%lu,%lu);", (const char *)ExplainPoke(DDEFACK),
		dwWindowID, dwWindowFlags, dwX, dwY, dwCX, dwCY);
	GetDDEConn()->Display(aBuf);
		
	return(DDEFACK);
}
#endif

HDDEDATA CDDEConversation::WWW_OpenURLResult(HSZ hszItem)	
{
	DWORD dwTransID;
	DWORD dwWindowID;
	
	ClientPassed(hszItem, "DW,DW", &dwTransID, &dwWindowID );
		
	GetDDEConn()->OpenURLResult( dwTransID, dwWindowID );
		
	return(DDEFACK);
}


//	Handle DDE Requests
HDDEDATA CDDEConversation::ServerRequest(HSZ hszTopic, HSZ hszItem)	
{
	//	Switch on the topic we're handling.
	int iTopic = CDDEObject::EnumTopic(hszTopic);
	switch(iTopic)	
	{
		case CDDEObject::m_Alert:
			return(WWW_Alert(hszItem));
		case CDDEObject::m_BeginProgress:
			return(WWW_BeginProgress(hszItem));
		case CDDEObject::m_MakingProgress:
			return(WWW_MakingProgress(hszItem));
		case CDDEObject::m_OpenURL:
			return(WWW_OpenURL(hszItem));
		case CDDEObject::m_QueryViewer:
			return(WWW_QueryViewer(hszItem));
		//case CDDEObject::m_ViewDocCache:
		//	return( WWW_ViewDocCache(hszItem));
		case CDDEObject::m_RegisterNow:
			return( WWW_RegisterNow(hszItem));
	}

	TRACE("Unknown request received\n");
	return(NULL);
}

HDDEDATA CDDEConversation::WWW_Alert(HSZ hszItem)	
{
	CString csMessage;
	DWORD dwType;
	DWORD dwButtons;
	DWORD dwAnswer = 0;
		
	ClientPassed(hszItem, "QCS,DW,DW", &csMessage, &dwType, &dwButtons);
	
	dwAnswer = GetDDEConn()->Alert( csMessage, dwType, dwButtons );
	
	return(ServerReturns( hszItem, "DW", &dwAnswer));

}
HDDEDATA CDDEConversation::WWW_BeginProgress(HSZ hszItem)	
{
	DWORD dwWindowID;
	CString csInitialMessage;
	DWORD dwTransactionID = (DWORD)time(NULL);	
	
	ClientPassed(hszItem, "DW,QCS", &dwWindowID, &csInitialMessage);
	
	GetDDEConn()->BeginProcess( dwTransactionID,  csInitialMessage );

	CDDEObject *pDDE = CDDEObject::ResolveConversation(this);

	if ( pDDE->GetServerType() == CDDEObject::srvMosaic )
	{
		TwoByteBool bTrue = TRUE;
		return(ServerReturns( hszItem, "BL", &bTrue));
	}
	else
	{
		return(ServerReturns( hszItem, "DW", &dwTransactionID));
	}
}
HDDEDATA CDDEConversation::WWW_MakingProgress(HSZ hszItem)	
{
	DWORD dwTransactionID;
	CString csMessage;
	DWORD dwProgress;
	TwoByteBool bStop;
	
	ClientPassed(hszItem, "DW,QCS,DW", &dwTransactionID, &csMessage, &dwProgress);
	
	bStop = GetDDEConn()->MakingProgress( dwTransactionID, csMessage, dwProgress );
	
	return(ServerReturns( hszItem, "BL", &bStop));
}

HDDEDATA CDDEConversation::WWW_OpenURL(HSZ hszItem)	
{
	CString csURL;
	CString csSaveAs;
	DWORD dwWindowID;
	DWORD dwFlags;
	CString csPostFormData;
	CString csPostMIMEType;
	CString csProgressServer;
	DWORD dwServicingID = (DWORD)time(NULL);
	
	ClientPassed(hszItem, "QCS,QCS,DW,DW,QCS,QCS,CS", &csURL, &csSaveAs,
		&dwWindowID, &dwFlags, &csPostFormData, &csPostMIMEType, &csProgressServer);
	
	#if 0	
	char aBuf[2048];
	sprintf(aBuf, "%lu WWW_OpenURL(%s,%s,%lu,%lu,%s,%s,%s);", dwServicingID,
		(const char *)csURL, (const char *)csSaveAs, dwWindowID, dwFlags,
		(const char *)csPostFormData, (const char *)csPostMIMEType,
		(const char *)csProgressServer);
	GetDDEConn()->Display(aBuf);
	#endif

	if ( !csSaveAs.IsEmpty() )
	{
		ChString csMIMEType( "x-world/x-vrml");
		GetDDEConn()->ViewDocFile( csSaveAs, csURL,  csMIMEType );
	}
	else
	{
		GetDDEConn()->GetURL( csURL, 0 );
	}

		
	return(ServerReturns( hszItem, "DW", &dwServicingID));
}

#if 0
HDDEDATA CDDEConversation::WWW_ViewDocCache(HSZ hszItem)	
{
	TwoByteBool bNotInCache = FALSE;
	return(ServerReturns( hszItem, "BL", &bNotInCache));
}
#endif

HDDEDATA CDDEConversation::WWW_RegisterNow(HSZ hszItem)	
{
	DWORD dwTimeOut = 0xFFFFF;
	CDDEObject *pDDE = CDDEObject::ResolveConversation(this);
	CString csBrowser;
	DWORD   dwID;
	ClientPassed(hszItem, "QCS,DW", &csBrowser, &dwID);
	pDDE->SetRegNowID( dwID);
	// Call to register viewer
	GetDDEConn()->RegisterNow();
	return(ServerReturns( hszItem, "DW", &dwTimeOut));
}

HDDEDATA CDDEConversation::WWW_QueryViewer(HSZ hszItem)	
{
	CString csSaveIn;
	ChUtil::GetTempFileName( csSaveIn, 0, 0, 0 );
	ChUtil::AddFileToTempList( csSaveIn );

	#if 0
	CString csMIMEType;
	char *pTemp = _tempnam("c:\\temp", "nstest");
	CString csSaveIn = (pTemp == NULL) ? "\\delme.nst" : pTemp;
	
	ClientPassed(hszItem, "QCS,QCS", &csURL, &csMIMEType);
	
	char aBuf[2048];
	sprintf(aBuf, "%s WWW_QueryViewer(%s,%s);", (const char *)csSaveIn,
		(const char *)csURL, (const char *)csMIMEType);
	GetDDEConn()->Display(aBuf);
	#endif
	
	return(ServerReturns( hszItem, "QCS", &csSaveIn ));
}


//	Client Requests and pokes.
HDDEDATA CDDEConversation::ClientRequest(HSZ hszItem)	
{
	//	Send along the item to the server, as an XTYP_REQUEST.
	//	Return the data received from the server.
	//	Proactively delete the hszItem string.
	HDDEDATA hRetVal = DdeClientTransaction(NULL, 0ul, m_hConv, hszItem, 
						CF_TEXT, XTYP_REQUEST, DDETIMEOUT, NULL);
	GetDDEConn()->Display(ExplainError());	
	if(hszItem != NULL)	
	{
		DdeFreeStringHandle(CDDEObject::m_dwidInst, hszItem);
		GetDDEConn()->Display(ExplainError());
	}
	return(hRetVal);
}
HDDEDATA CDDEConversation::ClientPoke(HSZ hszItem)	
{
	//	Send along the item to the server, as an XTYP_POKE.
	//	Return the data received from the server.
	//	Proactively delete the hszItem string.
	HDDEDATA hRetVal = DdeClientTransaction(NULL, 0ul, m_hConv, hszItem, 
					CF_TEXT, XTYP_POKE, DDETIMEOUT, NULL);
	GetDDEConn()->Display(ExplainError());
	if(hszItem != NULL)	
	{
		DdeFreeStringHandle(CDDEObject::m_dwidInst, hszItem);
		GetDDEConn()->Display(ExplainError());
	}
	return(hRetVal);
}

//	Persistance
void CDDEConversation::Serialize(CArchive& ar)
{
	if(ar.IsStoring())	
	{
		//	Add storing code
	}
	else	
	{
		//	Add loading code
	}
	
	//	Serialize the base.
	CObject::Serialize(ar);
}

#ifdef _DEBUG

void CDDEConversation::AssertValid() const	
{
	//	Call base to assert valid.
	CObject::AssertValid();
	
	//	Check our instance specific members.
	ASSERT(m_hConv);	//	Better have a conversation!
	ASSERT(m_rIndex);	//	Position in list.
}

void CDDEConversation::Dump(CDumpContext& dc) const	
{
	//	Have the base dump
	CObject::Dump(dc);
	
	//	Now we dump any relevant information....
}

#endif // _DEBUG

// $Log$
